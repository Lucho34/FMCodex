#if WITH_DEV_AUTOMATION_TESTS

#include "ThroughBallFeetFormulaResolutionExecutor.h"
#include "ThroughBallParticipantEligibilityQuery.h"

#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace
{
	const FName SelectedSkillId(TEXT("Skill.ThroughBall.Composition"));
	const FName FeetCompositionTestAttackingOwnerId(TEXT("Player.Attacking"));
	const FName FeetCompositionTestDefendingOwnerId(TEXT("Player.Defending"));
	const FName CarrierId(TEXT("ThroughBall.Carrier"));
	const FName RunnerId(TEXT("ThroughBall.Runner"));
	const FName MarkerId(TEXT("ThroughBall.Marker"));
	const FName HelperId(TEXT("ThroughBall.Helper"));
	const FName GoalkeeperId(TEXT("ThroughBall.Goalkeeper"));
	const FGuid LogId(731, 732, 733, 734);

	enum class EThroughBallFeetFormulaResolutionCompositionErrorCode : uint8
	{
		None,
		PlanQueryFailed,
		InvalidPlanQueryResult,
		ResolverInputAssemblyFailed,
		InvalidResolverInputAssemblyResult,
		FormulaResolutionExecutionFailed,
		InvalidFormulaResolutionExecutionResult
	};

	enum class EThroughBallFeetResolvedTestOutcome : uint8
	{
		None,
		Goal,
		Miss
	};

	struct FThroughBallFeetFormulaResolutionCompositionInput
	{
		FThroughBallFeetPlanQueryInput PlanQueryInput;
	};

	struct FThroughBallFeetResolvedTestView
	{
		EThroughBallFeetResolvedTestOutcome Outcome =
			EThroughBallFeetResolvedTestOutcome::None;
		FName PlannedGoalScorerCardId = NAME_None;
		TArray<FName> InvolvedCardIds;
		FFormulaResolutionResult FormulaResolutionResult;
	};

	struct FThroughBallFeetFormulaResolutionCompositionResult
	{
		bool bSuccess = false;
		EThroughBallFeetFormulaResolutionCompositionErrorCode ErrorCode =
			EThroughBallFeetFormulaResolutionCompositionErrorCode::None;
		FThroughBallFeetFormulaResolutionCompositionInput Input;
		FThroughBallFeetPlanQueryResult PlanResult;
		FThroughBallFeetFormulaResolverInputAssemblyResult AssemblyResult;
		FThroughBallFeetFormulaResolutionExecutionResult ExecutionResult;
		bool bHasResolvedView = false;
		FThroughBallFeetResolvedTestView ResolvedView;
	};

	FPlayerCardRuleSnapshot MakeOutfield(
		const FName CardId,
		const EPlayerPositionType Position,
		const int32 Passing,
		const int32 OffBall,
		const int32 Marking,
		const int32 Tackling,
		const int32 Stamina)
	{
		FPlayerCardRuleSnapshot Snapshot;
		Snapshot.CardId = CardId;
		Snapshot.PositionTypes = {Position};
		Snapshot.Attributes.Passing = Passing;
		Snapshot.Attributes.OffBall = OffBall;
		Snapshot.Attributes.Marking = Marking;
		Snapshot.Attributes.Tackling = Tackling;
		Snapshot.Attributes.Stamina = Stamina;
		return Snapshot;
	}

	FPlayerCardRuleSnapshot MakeGoalkeeper(
		const int32 OneOnOne = 5,
		const int32 Stamina = 6)
	{
		FPlayerCardRuleSnapshot Snapshot;
		Snapshot.CardId = GoalkeeperId;
		Snapshot.PositionTypes = {EPlayerPositionType::Goalkeeper};
		Snapshot.Attributes.Stamina = Stamina;
		Snapshot.bIsGoalkeeper = true;
		Snapshot.bHasGoalkeeperAttributes = true;
		Snapshot.GoalkeeperAttributes.OneOnOne = OneOnOne;
		return Snapshot;
	}

	FSkillRuleSnapshotSet MakeSkillRules()
	{
		FSkillRuleSnapshot Rule;
		Rule.SkillId = SelectedSkillId;
		Rule.SkillType = ESkillRuleType::ThroughBall;
		Rule.MinTriggerActionPoint = 3;
		Rule.MaxTriggerActionPoint = 6;
		FSkillRuleSnapshotSet Rules;
		Rules.SkillRules = {Rule};
		return Rules;
	}

	FThroughBallParticipantEligibilityQueryInput MakeEligibilityInput(
		const bool bHasHelper)
	{
		FThroughBallParticipantEligibilityQueryInput Input;
		Input.SelectedSkillId = SelectedSkillId;
		Input.CurrentActionPoint = 4;
		Input.AttackingOwnerId = FeetCompositionTestAttackingOwnerId;
		Input.DefendingOwnerId = FeetCompositionTestDefendingOwnerId;
		Input.CarrierSnapshot = MakeOutfield(
			CarrierId, EPlayerPositionType::Midfield, 4, 2, 1, 1, 5);
		Input.CarrierSnapshot.SkillIds = {SelectedSkillId};
		Input.RunnerSnapshot = MakeOutfield(
			RunnerId, EPlayerPositionType::Attack, 1, 5, 1, 1, 4);
		Input.MarkerSnapshot = MakeOutfield(
			MarkerId, EPlayerPositionType::Defense, 1, 1, 2, 5, 3);
		Input.bHasHelper = bHasHelper;
		if (bHasHelper)
		{
			Input.HelperSnapshot = MakeOutfield(
				HelperId, EPlayerPositionType::Midfield, 1, 1, 4, 1, 2);
		}
		Input.bIsRunnerInAttackingForwardArea = true;
		return Input;
	}

	FThroughBallFeetPlanQueryInput MakePlanInput(
		const bool bHasHelper = false,
		const bool bHasGoalkeeper = false)
	{
		FThroughBallFeetPlanQueryInput Input;
		Input.ParticipantEligibilityResult =
			FThroughBallParticipantEligibilityQuery::Evaluate(
				MakeSkillRules(), MakeEligibilityInput(bHasHelper));
		Input.AttackD6 = 3;
		Input.DefenseD6 = 4;
		Input.bHasActiveGoalkeeper = bHasGoalkeeper;
		if (bHasGoalkeeper)
		{
			Input.ActiveGoalkeeperSnapshot = MakeGoalkeeper();
		}
		Input.LogId = LogId;
		Input.TurnIndex = 9;
		return Input;
	}

	bool IsPlanSuccessConsistent(const FThroughBallFeetPlanQueryResult& Result)
	{
		return Result.ErrorCode == EThroughBallFeetPlanQueryErrorCode::None
			&& Result.ErrorMessage.IsEmpty()
			&& Result.InvalidField.IsNone()
			&& Result.Decision
				== EThroughBallFeetPlanQueryDecision::FormulaResolutionRequired
			&& Result.bHasFormulaPlan;
	}

	bool IsAssemblySuccessConsistent(
		const FThroughBallFeetFormulaResolverInputAssemblyResult& Result)
	{
		return Result.ErrorCode
				== EThroughBallFeetFormulaResolverInputAssemblyErrorCode::None
			&& Result.ErrorMessage.IsEmpty()
			&& Result.InvalidField.IsNone()
			&& Result.bHasResolverInput;
	}

	bool IsExecutionSuccessConsistent(
		const FThroughBallFeetFormulaResolutionExecutionResult& Result)
	{
		if (Result.ErrorCode
				!= EThroughBallFeetFormulaResolutionExecutionErrorCode::None
			|| !Result.ErrorMessage.IsEmpty()
			|| !Result.InvalidField.IsNone()
			|| !Result.bHasFormulaResolution)
		{
			return false;
		}

		const FFormulaResolutionResult& Resolution =
			Result.FormulaResolutionResult;
		return (Resolution.Winner == EFormulaWinner::Attacker
				|| Resolution.Winner == EFormulaWinner::Defender)
			&& Resolution.bAttackEnded
			&& !Resolution.bContinueResolution
			&& Resolution.bIsGoal
				== (Resolution.Winner == EFormulaWinner::Attacker);
	}

	FThroughBallFeetFormulaResolutionCompositionResult Compose(
		const FThroughBallFeetFormulaResolutionCompositionInput& Input)
	{
		FThroughBallFeetFormulaResolutionCompositionResult Result;
		Result.Input = Input;

		Result.PlanResult = FThroughBallFeetPlanQuery::Evaluate(
			Input.PlanQueryInput);
		if (!Result.PlanResult.bSuccess)
		{
			Result.ErrorCode =
				EThroughBallFeetFormulaResolutionCompositionErrorCode
					::PlanQueryFailed;
			return Result;
		}
		if (!IsPlanSuccessConsistent(Result.PlanResult))
		{
			Result.ErrorCode =
				EThroughBallFeetFormulaResolutionCompositionErrorCode
					::InvalidPlanQueryResult;
			return Result;
		}

		FThroughBallFeetFormulaResolverInputAssemblyInput AssemblyInput;
		AssemblyInput.FormulaPlan = Result.PlanResult.FormulaPlan;
		Result.AssemblyResult =
			FThroughBallFeetFormulaResolverInputAssembler::Assemble(
				AssemblyInput);
		if (!Result.AssemblyResult.bSuccess)
		{
			Result.ErrorCode =
				EThroughBallFeetFormulaResolutionCompositionErrorCode
					::ResolverInputAssemblyFailed;
			return Result;
		}
		if (!IsAssemblySuccessConsistent(Result.AssemblyResult))
		{
			Result.ErrorCode =
				EThroughBallFeetFormulaResolutionCompositionErrorCode
					::InvalidResolverInputAssemblyResult;
			return Result;
		}

		FThroughBallFeetFormulaResolutionExecutionInput ExecutionInput;
		ExecutionInput.ResolverInputAssemblyResult = Result.AssemblyResult;
		Result.ExecutionResult =
			FThroughBallFeetFormulaResolutionExecutor::Execute(ExecutionInput);
		if (!Result.ExecutionResult.bSuccess)
		{
			Result.ErrorCode =
				EThroughBallFeetFormulaResolutionCompositionErrorCode
					::FormulaResolutionExecutionFailed;
			return Result;
		}
		if (!IsExecutionSuccessConsistent(Result.ExecutionResult))
		{
			Result.ErrorCode =
				EThroughBallFeetFormulaResolutionCompositionErrorCode
					::InvalidFormulaResolutionExecutionResult;
			return Result;
		}

		const FFormulaResolutionResult& Resolution =
			Result.ExecutionResult.FormulaResolutionResult;
		Result.ResolvedView.Outcome = Resolution.Winner
				== EFormulaWinner::Attacker
			? EThroughBallFeetResolvedTestOutcome::Goal
			: EThroughBallFeetResolvedTestOutcome::Miss;
		Result.ResolvedView.PlannedGoalScorerCardId =
			Result.PlanResult.FormulaPlan.GoalScorerCardId;
		Result.ResolvedView.InvolvedCardIds =
			Resolution.MatchLogEntry.InvolvedCardIds;
		Result.ResolvedView.FormulaResolutionResult = Resolution;
		Result.bHasResolvedView = true;
		Result.bSuccess = true;
		return Result;
	}

	FThroughBallFeetFormulaResolutionCompositionResult Compose(
		const FThroughBallFeetPlanQueryInput& PlanInput)
	{
		FThroughBallFeetFormulaResolutionCompositionInput Input;
		Input.PlanQueryInput = PlanInput;
		return Compose(Input);
	}

	bool AreFormulaResultsEqual(
		const FFormulaResolutionResult& Left,
		const FFormulaResolutionResult& Right)
	{
		return Left.FormulaType == Right.FormulaType
			&& Left.AttackerFinalValue == Right.AttackerFinalValue
			&& Left.DefenderFinalValue == Right.DefenderFinalValue
			&& Left.Winner == Right.Winner
			&& Left.WinReason == Right.WinReason
			&& Left.bIsGoal == Right.bIsGoal
			&& Left.bAttackEnded == Right.bAttackEnded
			&& Left.bContinueResolution == Right.bContinueResolution
			&& Left.bFastSuppressionTriggered
				== Right.bFastSuppressionTriggered
			&& Left.MatchLogEntry.LogId == Right.MatchLogEntry.LogId
			&& Left.MatchLogEntry.TurnIndex == Right.MatchLogEntry.TurnIndex
			&& Left.MatchLogEntry.ActingPlayerId
				== Right.MatchLogEntry.ActingPlayerId
			&& Left.MatchLogEntry.InvolvedCardIds
				== Right.MatchLogEntry.InvolvedCardIds
			&& Left.MatchLogEntry.DiceResults
				== Right.MatchLogEntry.DiceResults
			&& Left.MatchLogEntry.DiceOrder == Right.MatchLogEntry.DiceOrder
			&& Left.MatchLogEntry.FormulaType
				== Right.MatchLogEntry.FormulaType
			&& Left.MatchLogEntry.FormulaInputs
				== Right.MatchLogEntry.FormulaInputs
			&& Left.MatchLogEntry.FormulaResult
				== Right.MatchLogEntry.FormulaResult;
	}

	void MakeAttackerStrong(FThroughBallFeetPlanQueryInput& Input)
	{
		Input.ParticipantEligibilityResult.Input.CarrierSnapshot.Attributes.Passing = 9;
		Input.ParticipantEligibilityResult.Input.RunnerSnapshot.Attributes.OffBall = 9;
		Input.ParticipantEligibilityResult.Input.MarkerSnapshot.Attributes.Tackling = 1;
		if (Input.ParticipantEligibilityResult.bHasHelper)
		{
			Input.ParticipantEligibilityResult.Input.HelperSnapshot.Attributes.Marking = 1;
		}
		Input.AttackD6 = 5;
		Input.DefenseD6 = 3;
	}

	void MakeNumericTie(FThroughBallFeetPlanQueryInput& Input)
	{
		Input.ParticipantEligibilityResult.Input.CarrierSnapshot.Attributes.Passing = 4;
		Input.ParticipantEligibilityResult.Input.RunnerSnapshot.Attributes.OffBall = 4;
		Input.ParticipantEligibilityResult.Input.MarkerSnapshot.Attributes.Tackling = 2;
		if (Input.ParticipantEligibilityResult.bHasHelper)
		{
			Input.ParticipantEligibilityResult.Input.HelperSnapshot.Attributes.Marking = 2;
		}
		Input.AttackD6 = 4;
		Input.DefenseD6 = 5;
	}

	bool ExpectSuccess(
		FAutomationTestBase& Test,
		const FThroughBallFeetFormulaResolutionCompositionResult& Result)
	{
		Test.TestTrue(TEXT("Composition succeeds"), Result.bSuccess);
		Test.TestEqual(
			TEXT("Composition has no error"), Result.ErrorCode,
			EThroughBallFeetFormulaResolutionCompositionErrorCode::None);
		Test.TestTrue(TEXT("Plan succeeds"), Result.PlanResult.bSuccess);
		Test.TestTrue(TEXT("Assembly succeeds"), Result.AssemblyResult.bSuccess);
		Test.TestTrue(TEXT("Execution succeeds"), Result.ExecutionResult.bSuccess);
		Test.TestTrue(TEXT("Resolved view exists"), Result.bHasResolvedView);
		return Result.bSuccess;
	}

	int32 CountOccurrences(const FString& Source, const FString& Needle)
	{
		int32 Count = 0;
		int32 Offset = 0;
		while ((Offset = Source.Find(Needle, ESearchCase::CaseSensitive,
			ESearchDir::FromStart, Offset)) != INDEX_NONE)
		{
			++Count;
			Offset += Needle.Len();
		}
		return Count;
	}

	bool RunCase(FAutomationTestBase& Test, const int32 Case)
	{
		FThroughBallFeetPlanQueryInput Input;
		FThroughBallFeetFormulaResolutionCompositionResult Result;
		switch (Case)
		{
		case 1:
			Input = MakePlanInput(true, true);
			MakeAttackerStrong(Input);
			Result = Compose(Input);
			ExpectSuccess(Test, Result);
			Test.TestEqual(TEXT("Attacker maps to Goal"), Result.ResolvedView.Outcome,
				EThroughBallFeetResolvedTestOutcome::Goal);
			Test.TestEqual(TEXT("Formula is Finishing"),
				Result.ResolvedView.FormulaResolutionResult.FormulaType,
				EFormulaType::Finishing);
			break;
		case 2:
			Result = Compose(MakePlanInput(true, true));
			ExpectSuccess(Test, Result);
			Test.TestEqual(TEXT("Defender maps to Miss"), Result.ResolvedView.Outcome,
				EThroughBallFeetResolvedTestOutcome::Miss);
			Test.TestEqual(TEXT("Miss retains planned Runner"),
				Result.ResolvedView.PlannedGoalScorerCardId, RunnerId);
			Test.TestFalse(TEXT("Miss is not Goal"),
				Result.ResolvedView.FormulaResolutionResult.bIsGoal);
			break;
		case 3:
			Input = MakePlanInput(false, false);
			MakeAttackerStrong(Input);
			Result = Compose(Input);
			ExpectSuccess(Test, Result);
			Test.TestEqual(TEXT("Only required defenders contribute stamina"),
				Result.PlanResult.FormulaPlan.DefenseParticipatingStamina,
				TArray<int32>({3}));
			Test.TestEqual(TEXT("Only required cards involved"),
				Result.ResolvedView.InvolvedCardIds,
				TArray<FName>({CarrierId, RunnerId, MarkerId}));
			break;
		case 4:
			Input = MakePlanInput(true, false);
			MakeAttackerStrong(Input);
			Result = Compose(Input);
			ExpectSuccess(Test, Result);
			Test.TestEqual(TEXT("Helper is involved"),
				Result.ResolvedView.InvolvedCardIds,
				TArray<FName>({CarrierId, RunnerId, MarkerId, HelperId}));
			Test.TestEqual(TEXT("Helper contributes stamina"),
				Result.PlanResult.FormulaPlan.DefenseParticipatingStamina,
				TArray<int32>({3, 2}));
			break;
		case 5:
			Input = MakePlanInput(false, true);
			MakeAttackerStrong(Input);
			Result = Compose(Input);
			ExpectSuccess(Test, Result);
			Test.TestEqual(TEXT("GK is involved"),
				Result.ResolvedView.InvolvedCardIds,
				TArray<FName>({CarrierId, RunnerId, MarkerId, GoalkeeperId}));
			Test.TestEqual(TEXT("GK contributes stamina"),
				Result.PlanResult.FormulaPlan.DefenseParticipatingStamina,
				TArray<int32>({3, 6}));
			break;
		case 6:
			Input = MakePlanInput(false, true);
			Input.ActiveGoalkeeperSnapshot = MakeGoalkeeper(4, 6);
			MakeNumericTie(Input);
			Input.DefenseD6 = 3;
			Result = Compose(Input);
			ExpectSuccess(Test, Result);
			Test.TestEqual(TEXT("GK numeric tie favors Defender"),
				Result.ExecutionResult.FormulaResolutionResult.Winner,
				EFormulaWinner::Defender);
			Test.TestEqual(TEXT("GK tie reason retained"),
				Result.ExecutionResult.FormulaResolutionResult.WinReason,
				EFormulaWinReason::DefenderWinsGoalkeeperTie);
			break;
		case 7:
			Input = MakePlanInput(false, false);
			MakeNumericTie(Input);
			Input.ParticipantEligibilityResult.Input.CarrierSnapshot.Attributes.Stamina = 8;
			Input.ParticipantEligibilityResult.Input.RunnerSnapshot.Attributes.Stamina = 7;
			Input.ParticipantEligibilityResult.Input.MarkerSnapshot.Attributes.Stamina = 2;
			Result = Compose(Input);
			ExpectSuccess(Test, Result);
			Test.TestEqual(TEXT("Attacker stamina wins"),
				Result.ExecutionResult.FormulaResolutionResult.Winner,
				EFormulaWinner::Attacker);
			break;
		case 8:
			Input = MakePlanInput(false, false);
			MakeNumericTie(Input);
			Input.ParticipantEligibilityResult.Input.CarrierSnapshot.Attributes.Stamina = 1;
			Input.ParticipantEligibilityResult.Input.RunnerSnapshot.Attributes.Stamina = 1;
			Input.ParticipantEligibilityResult.Input.MarkerSnapshot.Attributes.Stamina = 9;
			Result = Compose(Input);
			ExpectSuccess(Test, Result);
			Test.TestEqual(TEXT("Defender stamina wins"),
				Result.ExecutionResult.FormulaResolutionResult.Winner,
				EFormulaWinner::Defender);
			Test.TestEqual(TEXT("Stamina tie-break retained"),
				Result.ExecutionResult.FormulaResolutionResult.WinReason,
				EFormulaWinReason::StaminaTieBreaker);
			break;
		case 9:
			Input = MakePlanInput(false, false);
			MakeNumericTie(Input);
			Input.ParticipantEligibilityResult.Input.CarrierSnapshot.Attributes.Stamina = 5;
			Input.ParticipantEligibilityResult.Input.RunnerSnapshot.Attributes.Stamina = 4;
			Input.ParticipantEligibilityResult.Input.MarkerSnapshot.Attributes.Stamina = 9;
			Result = Compose(Input);
			ExpectSuccess(Test, Result);
			Test.TestEqual(TEXT("Equal stamina favors Defender"),
				Result.ExecutionResult.FormulaResolutionResult.WinReason,
				EFormulaWinReason::DefenderWinsEqualStamina);
			break;
		case 10:
			Input = MakePlanInput(false, false);
			Input.AttackD6 = 6;
			Input.DefenseD6 = 2;
			Result = Compose(Input);
			ExpectSuccess(Test, Result);
			Test.TestTrue(TEXT("Attacker fast suppression retained"),
				Result.ExecutionResult.FormulaResolutionResult.bFastSuppressionTriggered);
			Test.TestEqual(TEXT("Attacker wins fast suppression"),
				Result.ExecutionResult.FormulaResolutionResult.Winner,
				EFormulaWinner::Attacker);
			break;
		case 11:
			Input = MakePlanInput(false, false);
			Input.AttackD6 = 1;
			Input.DefenseD6 = 6;
			Result = Compose(Input);
			ExpectSuccess(Test, Result);
			Test.TestTrue(TEXT("Defender fast suppression retained"),
				Result.ExecutionResult.FormulaResolutionResult.bFastSuppressionTriggered);
			Test.TestEqual(TEXT("Defender wins fast suppression"),
				Result.ExecutionResult.FormulaResolutionResult.Winner,
				EFormulaWinner::Defender);
			break;
		case 12:
			Input = MakePlanInput(true, true);
			Result = Compose(Input);
			ExpectSuccess(Test, Result);
			Test.TestEqual(TEXT("Attack half retained"),
				Result.PlanResult.FormulaPlan.AttackBaseValue, 4.5f);
			Test.TestEqual(TEXT("Defense half retained"),
				Result.PlanResult.FormulaPlan.DefenseBaseValue, 4.5f);
			Test.TestEqual(TEXT("GK half plus two retained"),
				Result.PlanResult.FormulaPlan.DefenseExternalModifier, 4.5f);
			Test.TestEqual(TEXT("Attack D6 retained"),
				Result.AssemblyResult.ResolverInput.Attacker.ComparePoint, 3);
			Test.TestEqual(TEXT("Defense D6 retained"),
				Result.AssemblyResult.ResolverInput.Defender.ComparePoint, 4);
			break;
		case 13:
			Input = MakePlanInput(true, true);
			Input.TurnIndex = 27;
			Result = Compose(Input);
			ExpectSuccess(Test, Result);
			Test.TestEqual(TEXT("LogId retained"),
				Result.ResolvedView.FormulaResolutionResult.MatchLogEntry.LogId,
				LogId);
			Test.TestEqual(TEXT("Turn retained"),
				Result.ResolvedView.FormulaResolutionResult.MatchLogEntry.TurnIndex,
				27);
			Test.TestEqual(TEXT("Attacking owner retained"),
				Result.AssemblyResult.ResolverInput.AttackerPlayerId,
				FeetCompositionTestAttackingOwnerId);
			Test.TestEqual(TEXT("Defending owner retained"),
				Result.AssemblyResult.ResolverInput.DefenderPlayerId,
				FeetCompositionTestDefendingOwnerId);
			Test.TestTrue(TEXT("Complete resolution retained"),
				AreFormulaResultsEqual(Result.ResolvedView.FormulaResolutionResult,
					Result.ExecutionResult.FormulaResolutionResult));
			break;
		case 14:
			Input = MakePlanInput(false, false);
			Input.ParticipantEligibilityResult.Input.MarkerSnapshot.CardId = CarrierId;
			MakeAttackerStrong(Input);
			Result = Compose(Input);
			ExpectSuccess(Test, Result);
			Test.TestEqual(TEXT("Owner-aware same CardId survives"),
				Result.ResolvedView.InvolvedCardIds,
				TArray<FName>({CarrierId, RunnerId, CarrierId}));
			break;
		case 15:
		{
			FThroughBallParticipantEligibilityQueryInput EligibilityInput =
				MakeEligibilityInput(false);
			EligibilityInput.RunnerSnapshot.CardId = CarrierId;
			Input = MakePlanInput(false, false);
			Input.ParticipantEligibilityResult =
				FThroughBallParticipantEligibilityQuery::Evaluate(
					MakeSkillRules(), EligibilityInput);
			Result = Compose(Input);
			Test.TestFalse(TEXT("Duplicate attackers fail composition"), Result.bSuccess);
			Test.TestEqual(TEXT("Plan failure classified"), Result.ErrorCode,
				EThroughBallFeetFormulaResolutionCompositionErrorCode::PlanQueryFailed);
			Test.TestFalse(TEXT("Assembler is not reached"), Result.AssemblyResult.bSuccess);
			Test.TestFalse(TEXT("Executor is not reached"), Result.ExecutionResult.bSuccess);
			break;
		}
		case 16:
		{
			FThroughBallParticipantEligibilityQueryInput EligibilityInput =
				MakeEligibilityInput(true);
			EligibilityInput.HelperSnapshot.CardId = MarkerId;
			Input = MakePlanInput(true, false);
			Input.ParticipantEligibilityResult =
				FThroughBallParticipantEligibilityQuery::Evaluate(
					MakeSkillRules(), EligibilityInput);
			Result = Compose(Input);
			Test.TestFalse(TEXT("Duplicate defenders fail composition"), Result.bSuccess);
			Test.TestEqual(TEXT("Plan failure classified"), Result.ErrorCode,
				EThroughBallFeetFormulaResolutionCompositionErrorCode::PlanQueryFailed);
			Test.TestFalse(TEXT("No resolved view"), Result.bHasResolvedView);
			break;
		}
		case 17:
			Input = MakePlanInput();
			Input.ParticipantEligibilityResult.bSuccess = false;
			Result = Compose(Input);
			Test.TestEqual(TEXT("Formal Eligibility failure maps to Plan failure"),
				Result.ErrorCode,
				EThroughBallFeetFormulaResolutionCompositionErrorCode::PlanQueryFailed);
			Test.TestFalse(TEXT("Assembly remains default"), Result.AssemblyResult.bSuccess);
			Test.TestFalse(TEXT("Execution remains default"), Result.ExecutionResult.bSuccess);
			break;
		case 18:
			Input = MakePlanInput();
			Input.ParticipantEligibilityResult.ErrorMessage = TEXT("stale");
			Result = Compose(Input);
			Test.TestEqual(TEXT("Inconsistent Eligibility maps to Plan failure"),
				Result.ErrorCode,
				EThroughBallFeetFormulaResolutionCompositionErrorCode::PlanQueryFailed);
			Test.TestEqual(TEXT("Production Plan diagnostic preserved"),
				Result.PlanResult.ErrorCode,
				EThroughBallFeetPlanQueryErrorCode::InvalidParticipantEligibilityResult);
			break;
		case 19:
			Input = MakePlanInput();
			Input.AttackD6 = 0;
			Result = Compose(Input);
			Test.TestEqual(TEXT("Invalid D6 maps to Plan failure"), Result.ErrorCode,
				EThroughBallFeetFormulaResolutionCompositionErrorCode::PlanQueryFailed);
			Test.TestEqual(TEXT("Production D6 diagnostic preserved"),
				Result.PlanResult.ErrorCode,
				EThroughBallFeetPlanQueryErrorCode::InvalidAttackD6);
			Test.TestFalse(TEXT("Assembler short-circuited"), Result.AssemblyResult.bSuccess);
			Test.TestFalse(TEXT("Executor short-circuited"), Result.ExecutionResult.bSuccess);
			break;
		case 20:
		{
			Input = MakePlanInput(true, true);
			const FThroughBallFeetPlanQueryInput Before = Input;
			const auto First = Compose(Input);
			const auto Second = Compose(Input);
			Test.TestEqual(TEXT("Caller D6 unchanged"), Input.AttackD6, Before.AttackD6);
			Test.TestEqual(TEXT("Caller eligibility unchanged"),
				Input.ParticipantEligibilityResult.Input.CarrierSnapshot.CardId,
				Before.ParticipantEligibilityResult.Input.CarrierSnapshot.CardId);
			Test.TestEqual(TEXT("Deterministic success"), First.bSuccess, Second.bSuccess);
			Test.TestEqual(TEXT("Deterministic outcome"),
				First.ResolvedView.Outcome, Second.ResolvedView.Outcome);
			Test.TestTrue(TEXT("Deterministic full resolution"),
				AreFormulaResultsEqual(First.ResolvedView.FormulaResolutionResult,
					Second.ResolvedView.FormulaResolutionResult));
			Test.TestEqual(TEXT("Plan preserved through Assembler"),
				First.AssemblyResult.Input.FormulaPlan.InvolvedCardIds,
				First.PlanResult.FormulaPlan.InvolvedCardIds);
			Test.TestEqual(TEXT("Assembly preserved through Executor"),
				First.ExecutionResult.Input.ResolverInputAssemblyResult.ResolverInput.InvolvedCardIds,
				First.AssemblyResult.ResolverInput.InvolvedCardIds);
			break;
		}
		case 21:
		{
			FString Source;
			const FString Path = FPaths::Combine(
				FPaths::ProjectDir(),
				TEXT("Source/FMCodex/CoreRules/ThroughBallFeetFormulaResolutionCompositionTests.cpp"));
			Test.TestTrue(TEXT("Composition source is readable"),
				FFileHelper::LoadFileToString(Source, *Path));
			const int32 ComposeStart = Source.Find(
				TEXT("FThroughBallFeetFormulaResolutionCompositionResult Compose(\n\t\tconst FThroughBallFeetFormulaResolutionCompositionInput& Input)"));
			const int32 ComposeEnd = Source.Find(
				TEXT("FThroughBallFeetFormulaResolutionCompositionResult Compose(\n\t\tconst FThroughBallFeetPlanQueryInput& PlanInput)"));
			Test.TestTrue(TEXT("Composition helper region found"),
				ComposeStart != INDEX_NONE && ComposeEnd > ComposeStart);
			const FString ComposeSource =
				Source.Mid(ComposeStart, ComposeEnd - ComposeStart);
			Test.TestEqual(TEXT("Plan Query called exactly once"),
				CountOccurrences(ComposeSource,
					TEXT("FThroughBallFeetPlanQuery::Evaluate")), 1);
			Test.TestEqual(TEXT("Assembler called exactly once"),
				CountOccurrences(ComposeSource,
					TEXT("FThroughBallFeetFormulaResolverInputAssembler::Assemble")), 1);
			Test.TestEqual(TEXT("Executor called exactly once"),
				CountOccurrences(ComposeSource,
					TEXT("FThroughBallFeetFormulaResolutionExecutor::Execute")), 1);
			Test.TestEqual(TEXT("Eligibility is not called by Composition"),
				CountOccurrences(ComposeSource,
					TEXT("FThroughBallParticipantEligibilityQuery::Evaluate")), 0);
			Test.TestEqual(TEXT("FormulaResolver is not called directly"),
				CountOccurrences(ComposeSource,
					TEXT("UFormulaResolver::ResolveFormula")), 0);
			for (const TCHAR* Forbidden : {
				TEXT("SingleCard"), TEXT("FormulaAttackFlow"), TEXT("MatchPlay"),
				TEXT("DataTable"), TEXT("Provider"), TEXT("FMath::Rand")})
			{
				Test.TestFalse(Forbidden, ComposeSource.Contains(Forbidden));
			}
			break;
		}
		default:
			Test.AddError(FString::Printf(TEXT("Unknown case %d"), Case));
			return false;
		}
		return true;
	}
}

#define THROUGH_BALL_FEET_COMPOSITION_TEST(ClassName, TestName, CaseNumber) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
		ClassName, \
		"FMCodex.CoreRules.ThroughBall.FeetFormulaResolutionComposition." TestName, \
		EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter) \
	bool ClassName::RunTest(const FString& Parameters) \
	{ \
		return RunCase(*this, CaseNumber); \
	}

THROUGH_BALL_FEET_COMPOSITION_TEST(FThroughBallFeetComposition01, "01AttackerWinnerGoalWithHelperAndGoalkeeper", 1)
THROUGH_BALL_FEET_COMPOSITION_TEST(FThroughBallFeetComposition02, "02DefenderWinnerMissRetainsRunner", 2)
THROUGH_BALL_FEET_COMPOSITION_TEST(FThroughBallFeetComposition03, "03NoHelperNoGoalkeeper", 3)
THROUGH_BALL_FEET_COMPOSITION_TEST(FThroughBallFeetComposition04, "04HelperWithoutGoalkeeper", 4)
THROUGH_BALL_FEET_COMPOSITION_TEST(FThroughBallFeetComposition05, "05GoalkeeperWithoutHelper", 5)
THROUGH_BALL_FEET_COMPOSITION_TEST(FThroughBallFeetComposition06, "06GoalkeeperNumericTie", 6)
THROUGH_BALL_FEET_COMPOSITION_TEST(FThroughBallFeetComposition07, "07AttackerStaminaTieBreak", 7)
THROUGH_BALL_FEET_COMPOSITION_TEST(FThroughBallFeetComposition08, "08DefenderStaminaTieBreak", 8)
THROUGH_BALL_FEET_COMPOSITION_TEST(FThroughBallFeetComposition09, "09EqualStaminaDefenderWins", 9)
THROUGH_BALL_FEET_COMPOSITION_TEST(FThroughBallFeetComposition10, "10AttackerFastSuppression", 10)
THROUGH_BALL_FEET_COMPOSITION_TEST(FThroughBallFeetComposition11, "11DefenderFastSuppression", 11)
THROUGH_BALL_FEET_COMPOSITION_TEST(FThroughBallFeetComposition12, "12DecimalAndDicePreservation", 12)
THROUGH_BALL_FEET_COMPOSITION_TEST(FThroughBallFeetComposition13, "13ContextOwnerAndResolutionPreservation", 13)
THROUGH_BALL_FEET_COMPOSITION_TEST(FThroughBallFeetComposition14, "14OwnerAwareSameCardId", 14)
THROUGH_BALL_FEET_COMPOSITION_TEST(FThroughBallFeetComposition15, "15DuplicateAttackerShortCircuit", 15)
THROUGH_BALL_FEET_COMPOSITION_TEST(FThroughBallFeetComposition16, "16DuplicateDefenderShortCircuit", 16)
THROUGH_BALL_FEET_COMPOSITION_TEST(FThroughBallFeetComposition17, "17EligibilityFailureShortCircuit", 17)
THROUGH_BALL_FEET_COMPOSITION_TEST(FThroughBallFeetComposition18, "18InconsistentEligibilityShortCircuit", 18)
THROUGH_BALL_FEET_COMPOSITION_TEST(FThroughBallFeetComposition19, "19InvalidD6ShortCircuit", 19)
THROUGH_BALL_FEET_COMPOSITION_TEST(FThroughBallFeetComposition20, "20DeterminismAndImmutability", 20)
THROUGH_BALL_FEET_COMPOSITION_TEST(FThroughBallFeetComposition21, "21DependencyBoundary", 21)

#undef THROUGH_BALL_FEET_COMPOSITION_TEST

#endif
