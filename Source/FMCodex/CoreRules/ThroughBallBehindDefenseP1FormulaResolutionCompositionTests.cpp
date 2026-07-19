#if WITH_DEV_AUTOMATION_TESTS

#include "ThroughBallBehindDefenseP1FormulaResolutionExecutor.h"

#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace FMCodex::Tests::ThroughBallBehindDefenseP1FormulaResolutionComposition
{
namespace
{
	const FName SkillId(TEXT("Skill.ThroughBall.BehindDefenseP1.Composition"));
	const FName CarrierId(TEXT("ThroughBall.Composition.Carrier"));
	const FName RunnerId(TEXT("ThroughBall.Composition.Runner"));
	const FName MarkerId(TEXT("ThroughBall.Composition.Marker"));
	const FName HelperId(TEXT("ThroughBall.Composition.Helper"));
	const FName AttackingOwnerId(TEXT("Player.Composition.Attacking"));
	const FName DefendingOwnerId(TEXT("Player.Composition.Defending"));
	const FGuid LogId(747, 748, 749, 750);

	enum class EComposedP1Outcome : uint8
	{
		None,
		OutOfPlay,
		DefenderStoppedAttack,
		P2Required
	};

	struct FComposedP1Observation
	{
		FThroughBallBehindDefenseP1PlanQueryResult PlanResult;

		bool bAssemblerCalled = false;
		FThroughBallBehindDefenseP1FormulaResolverInputAssemblyResult
			AssemblyResult;

		bool bExecutorCalled = false;
		FThroughBallBehindDefenseP1FormulaResolutionExecutionResult
			ExecutionResult;

		EComposedP1Outcome Outcome = EComposedP1Outcome::None;
		bool bAttackEnded = false;
		bool bContinueResolution = false;
		bool bRequiresP2 = false;
		FName RunnerId = NAME_None;
	};

	FPlayerCardRuleSnapshot MakeOutfield(
		const FName CardId,
		const EPlayerPositionType Position,
		const int32 Passing,
		const int32 Speed,
		const int32 Marking,
		const int32 Stamina)
	{
		FPlayerCardRuleSnapshot Snapshot;
		Snapshot.CardId = CardId;
		Snapshot.PositionTypes = {Position};
		Snapshot.Attributes.Passing = Passing;
		Snapshot.Attributes.Speed = Speed;
		Snapshot.Attributes.Marking = Marking;
		Snapshot.Attributes.Stamina = Stamina;
		return Snapshot;
	}

	FSkillRuleSnapshotSet MakeSkillRules()
	{
		FSkillRuleSnapshot Rule;
		Rule.SkillId = SkillId;
		Rule.SkillType = ESkillRuleType::ThroughBall;
		Rule.MinTriggerActionPoint = 2;
		Rule.MaxTriggerActionPoint = 8;

		FSkillRuleSnapshotSet Rules;
		Rules.SkillRules = {Rule};
		return Rules;
	}

	FThroughBallParticipantEligibilityQueryResult MakeFormalEligibility(
		const bool bHasHelper = false,
		const bool bUseCrossSideDuplicateCardId = false)
	{
		FThroughBallParticipantEligibilityQueryInput Input;
		Input.SelectedSkillId = SkillId;
		Input.CurrentActionPoint = 4;
		Input.AttackingOwnerId = AttackingOwnerId;
		Input.DefendingOwnerId = DefendingOwnerId;
		Input.CarrierSnapshot = MakeOutfield(
			CarrierId, EPlayerPositionType::Midfield, 5, 2, 1, 6);
		Input.CarrierSnapshot.SkillIds = {SkillId};
		Input.RunnerSnapshot = MakeOutfield(
			RunnerId, EPlayerPositionType::Attack, 1, 4, 1, 5);
		Input.MarkerSnapshot = MakeOutfield(
			bUseCrossSideDuplicateCardId ? CarrierId : MarkerId,
			EPlayerPositionType::Defense,
			1,
			1,
			5,
			4);
		Input.bHasHelper = bHasHelper;
		if (bHasHelper)
		{
			Input.HelperSnapshot = MakeOutfield(
				HelperId, EPlayerPositionType::Midfield, 1, 4, 1, 3);
		}
		Input.bIsRunnerInAttackingForwardArea = true;

		return FThroughBallParticipantEligibilityQuery::Evaluate(
			MakeSkillRules(),
			Input);
	}

	FThroughBallBehindDefenseP1PlanQueryInput MakePlanInput(
		const bool bHasHelper = false,
		const bool bUseCrossSideDuplicateCardId = false)
	{
		FThroughBallBehindDefenseP1PlanQueryInput Input;
		Input.ParticipantEligibilityResult = MakeFormalEligibility(
			bHasHelper,
			bUseCrossSideDuplicateCardId);
		Input.SelectedBranch = EThroughBallSelectedBranch::BehindDefense;
		Input.bHasAttackD6 = true;
		Input.AttackD6 = 3;
		Input.bHasDefenseD6 = true;
		Input.DefenseD6 = 4;
		Input.LogId = LogId;
		Input.TurnIndex = 10;
		return Input;
	}

	// Composition helper begin.
	FComposedP1Observation Compose(
		const FThroughBallBehindDefenseP1PlanQueryInput& Input)
	{
		FComposedP1Observation Observation;
		Observation.PlanResult =
			FThroughBallBehindDefenseP1PlanQuery::Evaluate(Input);
		if (!Observation.PlanResult.bSuccess)
		{
			return Observation;
		}

		if (Observation.PlanResult.Decision
			== EThroughBallBehindDefenseP1PlanQueryDecision::OutOfPlay)
		{
			Observation.Outcome = EComposedP1Outcome::OutOfPlay;
			Observation.bAttackEnded = Observation.PlanResult.bAttackEnded;
			Observation.bContinueResolution =
				Observation.PlanResult.bContinueResolution;
			Observation.bRequiresP2 = Observation.PlanResult.bRequiresP2;
			return Observation;
		}

		if (Observation.PlanResult.Decision
			!= EThroughBallBehindDefenseP1PlanQueryDecision
				::FormulaResolutionRequired)
		{
			return Observation;
		}

		FThroughBallBehindDefenseP1FormulaResolverInputAssemblyInput
			AssemblyInput;
		AssemblyInput.PlanQueryResult = Observation.PlanResult;
		Observation.bAssemblerCalled = true;
		Observation.AssemblyResult =
			FThroughBallBehindDefenseP1FormulaResolverInputAssembler::Assemble(
				AssemblyInput);
		if (!Observation.AssemblyResult.bSuccess)
		{
			return Observation;
		}

		FThroughBallBehindDefenseP1FormulaResolutionExecutionInput
			ExecutionInput;
		ExecutionInput.ResolverInputAssemblyResult = Observation.AssemblyResult;
		Observation.bExecutorCalled = true;
		Observation.ExecutionResult =
			FThroughBallBehindDefenseP1FormulaResolutionExecutor::Execute(
				ExecutionInput);
		if (!Observation.ExecutionResult.bSuccess)
		{
			return Observation;
		}

		Observation.bAttackEnded = Observation.ExecutionResult.bAttackEnded;
		Observation.bContinueResolution =
			Observation.ExecutionResult.bContinueResolution;
		Observation.bRequiresP2 = Observation.ExecutionResult.bRequiresP2;
		Observation.RunnerId = Observation.ExecutionResult.RunnerId;

		if (Observation.ExecutionResult.Decision
			== EThroughBallBehindDefenseP1FormulaResolutionExecutionDecision
				::DefenderStoppedAttack)
		{
			Observation.Outcome = EComposedP1Outcome::DefenderStoppedAttack;
		}
		else if (Observation.ExecutionResult.Decision
			== EThroughBallBehindDefenseP1FormulaResolutionExecutionDecision
				::P2Required)
		{
			Observation.Outcome = EComposedP1Outcome::P2Required;
		}

		return Observation;
	}
	// Composition helper end.

	void ExpectOutOfPlay(
		FAutomationTestBase& Test,
		const FComposedP1Observation& Observation)
	{
		Test.TestTrue(TEXT("Plan succeeds"), Observation.PlanResult.bSuccess);
		Test.TestEqual(
			TEXT("Plan returns OutOfPlay"),
			Observation.PlanResult.Decision,
			EThroughBallBehindDefenseP1PlanQueryDecision::OutOfPlay);
		Test.TestFalse(
			TEXT("OutOfPlay has no Formula Plan"),
			Observation.PlanResult.bHasFormulaPlan);
		Test.TestFalse(
			TEXT("OutOfPlay does not call Assembler"),
			Observation.bAssemblerCalled);
		Test.TestFalse(
			TEXT("OutOfPlay does not call Executor"),
			Observation.bExecutorCalled);
		Test.TestEqual(
			TEXT("Observed OutOfPlay"),
			Observation.Outcome,
			EComposedP1Outcome::OutOfPlay);
	}

	void ExpectFormulaSuccess(
		FAutomationTestBase& Test,
		const FComposedP1Observation& Observation,
		const EComposedP1Outcome ExpectedOutcome)
	{
		Test.TestTrue(TEXT("Plan succeeds"), Observation.PlanResult.bSuccess);
		Test.TestEqual(
			TEXT("Plan requires formula resolution"),
			Observation.PlanResult.Decision,
			EThroughBallBehindDefenseP1PlanQueryDecision
				::FormulaResolutionRequired);
		Test.TestTrue(
			TEXT("Plan has Formula Plan"),
			Observation.PlanResult.bHasFormulaPlan);
		Test.TestTrue(TEXT("Assembler was called"), Observation.bAssemblerCalled);
		Test.TestTrue(
			TEXT("Assembly succeeds"),
			Observation.AssemblyResult.bSuccess);
		Test.TestTrue(TEXT("Executor was called"), Observation.bExecutorCalled);
		Test.TestTrue(
			TEXT("Execution succeeds"),
			Observation.ExecutionResult.bSuccess);
		Test.TestTrue(
			TEXT("Execution has Formula Resolution"),
			Observation.ExecutionResult.bHasFormulaResolution);
		Test.TestEqual(
			TEXT("Expected composed outcome"),
			Observation.Outcome,
			ExpectedOutcome);
	}

	int32 CountOccurrences(const FString& Source, const FString& Needle)
	{
		if (Needle.IsEmpty())
		{
			return 0;
		}

		int32 Count = 0;
		int32 SearchFrom = 0;
		while (true)
		{
			const int32 FoundAt = Source.Find(
				Needle,
				ESearchCase::CaseSensitive,
				ESearchDir::FromStart,
				SearchFrom);
			if (FoundAt == INDEX_NONE)
			{
				return Count;
			}
			++Count;
			SearchFrom = FoundAt + Needle.Len();
		}
	}

	void AssertSelectedInputFieldsUnchanged(
		FAutomationTestBase& Test,
		const FThroughBallBehindDefenseP1PlanQueryInput& Before,
		const FThroughBallBehindDefenseP1PlanQueryInput& After)
	{
		Test.TestEqual(TEXT("SelectedBranch unchanged"), After.SelectedBranch, Before.SelectedBranch);
		Test.TestEqual(TEXT("Attack D6 presence unchanged"), After.bHasAttackD6, Before.bHasAttackD6);
		Test.TestEqual(TEXT("Attack D6 unchanged"), After.AttackD6, Before.AttackD6);
		Test.TestEqual(TEXT("Defense D6 presence unchanged"), After.bHasDefenseD6, Before.bHasDefenseD6);
		Test.TestEqual(TEXT("Defense D6 unchanged"), After.DefenseD6, Before.DefenseD6);
		Test.TestEqual(TEXT("LogId unchanged"), After.LogId, Before.LogId);
		Test.TestEqual(TEXT("TurnIndex unchanged"), After.TurnIndex, Before.TurnIndex);

		const auto& Left = After.ParticipantEligibilityResult;
		const auto& Right = Before.ParticipantEligibilityResult;
		Test.TestEqual(TEXT("Eligibility success unchanged"), Left.bSuccess, Right.bSuccess);
		Test.TestEqual(TEXT("Eligibility error unchanged"), Left.ErrorCode, Right.ErrorCode);
		Test.TestEqual(TEXT("Eligibility message unchanged"), Left.ErrorMessage, Right.ErrorMessage);
		Test.TestEqual(TEXT("Eligibility invalid field unchanged"), Left.InvalidField, Right.InvalidField);
		Test.TestEqual(TEXT("Eligibility helper state unchanged"), Left.bHasHelper, Right.bHasHelper);
		Test.TestEqual(TEXT("Selected SkillId unchanged"), Left.Input.SelectedSkillId, Right.Input.SelectedSkillId);
		Test.TestEqual(TEXT("Action Point unchanged"), Left.Input.CurrentActionPoint, Right.Input.CurrentActionPoint);
		Test.TestEqual(TEXT("Attacking owner unchanged"), Left.Input.AttackingOwnerId, Right.Input.AttackingOwnerId);
		Test.TestEqual(TEXT("Defending owner unchanged"), Left.Input.DefendingOwnerId, Right.Input.DefendingOwnerId);
		Test.TestEqual(TEXT("Eligibility input helper state unchanged"), Left.Input.bHasHelper, Right.Input.bHasHelper);
		Test.TestEqual(TEXT("Runner deployment flag unchanged"), Left.Input.bIsRunnerInAttackingForwardArea, Right.Input.bIsRunnerInAttackingForwardArea);

		const FPlayerCardRuleSnapshot* LeftSnapshots[] = {
			&Left.Input.CarrierSnapshot,
			&Left.Input.RunnerSnapshot,
			&Left.Input.MarkerSnapshot,
			&Left.Input.HelperSnapshot
		};
		const FPlayerCardRuleSnapshot* RightSnapshots[] = {
			&Right.Input.CarrierSnapshot,
			&Right.Input.RunnerSnapshot,
			&Right.Input.MarkerSnapshot,
			&Right.Input.HelperSnapshot
		};
		const TCHAR* Labels[] = {TEXT("Carrier"), TEXT("Runner"), TEXT("Marker"), TEXT("Helper")};
		for (int32 Index = 0; Index < 4; ++Index)
		{
			const auto& L = *LeftSnapshots[Index];
			const auto& R = *RightSnapshots[Index];
			Test.TestEqual(FString::Printf(TEXT("%s identity unchanged"), Labels[Index]), L.CardId, R.CardId);
			Test.TestEqual(FString::Printf(TEXT("%s Passing unchanged"), Labels[Index]), L.Attributes.Passing, R.Attributes.Passing);
			Test.TestEqual(FString::Printf(TEXT("%s Speed unchanged"), Labels[Index]), L.Attributes.Speed, R.Attributes.Speed);
			Test.TestEqual(FString::Printf(TEXT("%s Marking unchanged"), Labels[Index]), L.Attributes.Marking, R.Attributes.Marking);
			Test.TestEqual(FString::Printf(TEXT("%s Stamina unchanged"), Labels[Index]), L.Attributes.Stamina, R.Attributes.Stamina);
		}
	}

	void AssertSelectedDeterministicFields(
		FAutomationTestBase& Test,
		const FComposedP1Observation& First,
		const FComposedP1Observation& Second)
	{
		Test.TestEqual(TEXT("Plan success deterministic"), First.PlanResult.bSuccess, Second.PlanResult.bSuccess);
		Test.TestEqual(TEXT("Plan Decision deterministic"), First.PlanResult.Decision, Second.PlanResult.Decision);
		Test.TestEqual(TEXT("Plan presence deterministic"), First.PlanResult.bHasFormulaPlan, Second.PlanResult.bHasFormulaPlan);
		Test.TestEqual(TEXT("Assembler call flag deterministic"), First.bAssemblerCalled, Second.bAssemblerCalled);
		Test.TestEqual(TEXT("Assembly success deterministic"), First.AssemblyResult.bSuccess, Second.AssemblyResult.bSuccess);
		Test.TestEqual(TEXT("Resolver Input presence deterministic"), First.AssemblyResult.bHasResolverInput, Second.AssemblyResult.bHasResolverInput);
		Test.TestEqual(TEXT("Executor call flag deterministic"), First.bExecutorCalled, Second.bExecutorCalled);
		Test.TestEqual(TEXT("Execution success deterministic"), First.ExecutionResult.bSuccess, Second.ExecutionResult.bSuccess);
		Test.TestEqual(TEXT("Resolution presence deterministic"), First.ExecutionResult.bHasFormulaResolution, Second.ExecutionResult.bHasFormulaResolution);

		const auto& LeftResolution = First.ExecutionResult.FormulaResolutionResult;
		const auto& RightResolution = Second.ExecutionResult.FormulaResolutionResult;
		Test.TestEqual(TEXT("Winner deterministic"), LeftResolution.Winner, RightResolution.Winner);
		Test.TestEqual(TEXT("Attacker final value deterministic"), LeftResolution.AttackerFinalValue, RightResolution.AttackerFinalValue);
		Test.TestEqual(TEXT("Defender final value deterministic"), LeftResolution.DefenderFinalValue, RightResolution.DefenderFinalValue);
		Test.TestEqual(TEXT("Outcome deterministic"), First.Outcome, Second.Outcome);
		Test.TestEqual(TEXT("Attack-ended flag deterministic"), First.bAttackEnded, Second.bAttackEnded);
		Test.TestEqual(TEXT("Continue flag deterministic"), First.bContinueResolution, Second.bContinueResolution);
		Test.TestEqual(TEXT("P2 flag deterministic"), First.bRequiresP2, Second.bRequiresP2);
		Test.TestEqual(TEXT("RunnerId deterministic"), First.RunnerId, Second.RunnerId);
		Test.TestEqual(TEXT("MatchLog LogId deterministic"), LeftResolution.MatchLogEntry.LogId, RightResolution.MatchLogEntry.LogId);
		Test.TestEqual(TEXT("MatchLog TurnIndex deterministic"), LeftResolution.MatchLogEntry.TurnIndex, RightResolution.MatchLogEntry.TurnIndex);
		Test.TestEqual(TEXT("MatchLog actor deterministic"), LeftResolution.MatchLogEntry.ActingPlayerId, RightResolution.MatchLogEntry.ActingPlayerId);
		Test.TestTrue(TEXT("MatchLog CardIds deterministic"), LeftResolution.MatchLogEntry.InvolvedCardIds == RightResolution.MatchLogEntry.InvolvedCardIds);
	}

	bool RunCase(FAutomationTestBase& Test, const int32 Case)
	{
		switch (Case)
		{
		case 1:
		{
			auto Input = MakePlanInput();
			Input.AttackD6 = 1;
			Input.bHasDefenseD6 = false;
			ExpectOutOfPlay(Test, Compose(Input));
			break;
		}
		case 2:
		{
			auto Input = MakePlanInput();
			Input.AttackD6 = 2;
			Input.bHasDefenseD6 = true;
			Input.DefenseD6 = 99;
			const auto Observation = Compose(Input);
			ExpectOutOfPlay(Test, Observation);
			Test.TestEqual(TEXT("Garbage DefenseD6 remains only in saved Plan input"), Observation.PlanResult.Input.DefenseD6, 99);
			break;
		}
		case 3:
		{
			auto Input = MakePlanInput();
			Input.AttackD6 = 1;
			const auto Observation = Compose(Input);
			ExpectOutOfPlay(Test, Observation);
			Test.TestTrue(TEXT("OutOfPlay ends attack"), Observation.bAttackEnded);
			Test.TestFalse(TEXT("OutOfPlay does not continue"), Observation.bContinueResolution);
			Test.TestFalse(TEXT("OutOfPlay does not require P2"), Observation.bRequiresP2);
			Test.TestTrue(TEXT("OutOfPlay RunnerId is None"), Observation.RunnerId.IsNone());
			break;
		}
		case 4:
		{
			auto Input = MakePlanInput(false);
			Input.DefenseD6 = 6;
			const auto Observation = Compose(Input);
			ExpectFormulaSuccess(Test, Observation, EComposedP1Outcome::DefenderStoppedAttack);
			Test.TestTrue(TEXT("Defender outcome ends attack"), Observation.bAttackEnded);
			Test.TestTrue(TEXT("Defender outcome has no RunnerId"), Observation.RunnerId.IsNone());
			break;
		}
		case 5:
		{
			const auto Observation = Compose(MakePlanInput(true));
			ExpectFormulaSuccess(Test, Observation, EComposedP1Outcome::DefenderStoppedAttack);
			Test.TestTrue(TEXT("Helper is present in Plan"), Observation.PlanResult.FormulaPlan.bHasHelper);
			break;
		}
		case 6:
		{
			const auto Observation = Compose(MakePlanInput(false));
			ExpectFormulaSuccess(Test, Observation, EComposedP1Outcome::P2Required);
			Test.TestTrue(TEXT("Attacker outcome requires P2"), Observation.bRequiresP2);
			break;
		}
		case 7:
		{
			auto Input = MakePlanInput(true);
			Input.AttackD6 = 6;
			Input.DefenseD6 = 1;
			const auto Observation = Compose(Input);
			ExpectFormulaSuccess(Test, Observation, EComposedP1Outcome::P2Required);
			Test.TestEqual(TEXT("P2 carries real Plan RunnerId"), Observation.RunnerId, Observation.PlanResult.FormulaPlan.RunnerId);
			Test.TestEqual(TEXT("P2 carries fixture RunnerId"), Observation.RunnerId, RunnerId);
			break;
		}
		case 8:
		{
			const auto Observation = Compose(MakePlanInput(false));
			ExpectFormulaSuccess(Test, Observation, EComposedP1Outcome::P2Required);
			const auto& Plan = Observation.PlanResult.FormulaPlan;
			const auto& Resolver = Observation.AssemblyResult.ResolverInput;
			Test.TestEqual(TEXT("Fixture produces attack Base"), Plan.AttackBaseValue, 4.5f);
			Test.TestEqual(TEXT("Attack Base reaches Assembly"), Resolver.Attacker.BaseValue, Plan.AttackBaseValue);
			Test.TestEqual(TEXT("Attack Modifier reaches Assembly"), Resolver.Attacker.Modifier, Plan.AttackExternalModifier);
			Test.TestEqual(TEXT("Fixture produces defense Base"), Plan.DefenseBaseValue, 2.5f);
			Test.TestEqual(TEXT("Defense Base reaches Assembly"), Resolver.Defender.BaseValue, Plan.DefenseBaseValue);
			Test.TestEqual(TEXT("Defense Modifier reaches Assembly"), Resolver.Defender.Modifier, Plan.DefenseExternalModifier);
			break;
		}
		case 9:
		{
			constexpr int32 BranchSelectionD6Context = 3;
			auto Input = MakePlanInput(false);
			Input.AttackD6 = 6;
			Input.DefenseD6 = 2;
			const auto Observation = Compose(Input);
			ExpectFormulaSuccess(Test, Observation, EComposedP1Outcome::P2Required);
			Test.TestEqual(TEXT("Selected branch is explicit"), Observation.PlanResult.Input.SelectedBranch, EThroughBallSelectedBranch::BehindDefense);
			Test.TestNotEqual(TEXT("P1 AttackD6 is distinct fixture data from branch D6 context"), Input.AttackD6, BranchSelectionD6Context);
			Test.TestNotEqual(TEXT("P1 DefenseD6 is distinct fixture data from branch D6 context"), Input.DefenseD6, BranchSelectionD6Context);
			Test.TestEqual(TEXT("P1 AttackD6 reaches Plan"), Observation.PlanResult.FormulaPlan.AttackD6, Input.AttackD6);
			Test.TestEqual(TEXT("P1 DefenseD6 reaches Plan"), Observation.PlanResult.FormulaPlan.DefenseD6, Input.DefenseD6);
			Test.TestEqual(TEXT("P1 AttackD6 reaches Resolver"), Observation.AssemblyResult.ResolverInput.Attacker.ComparePoint, Input.AttackD6);
			Test.TestEqual(TEXT("P1 DefenseD6 reaches Resolver"), Observation.AssemblyResult.ResolverInput.Defender.ComparePoint, Input.DefenseD6);
			break;
		}
		case 10:
		{
			const auto Observation = Compose(MakePlanInput(false));
			ExpectFormulaSuccess(Test, Observation, EComposedP1Outcome::P2Required);
			const TArray<int32> ExpectedAttackStamina{6, 5};
			const TArray<int32> ExpectedDefenseStamina{4};
			const TArray<FName> ExpectedCards{CarrierId, RunnerId, MarkerId};
			Test.TestTrue(TEXT("No-helper attack stamina order"), Observation.AssemblyResult.ResolverInput.Attacker.ParticipatingStamina == ExpectedAttackStamina);
			Test.TestTrue(TEXT("No-helper defense stamina order"), Observation.AssemblyResult.ResolverInput.Defender.ParticipatingStamina == ExpectedDefenseStamina);
			Test.TestTrue(TEXT("No-helper CardId order"), Observation.AssemblyResult.ResolverInput.InvolvedCardIds == ExpectedCards);
			break;
		}
		case 11:
		{
			auto Input = MakePlanInput(true);
			Input.AttackD6 = 6;
			Input.DefenseD6 = 1;
			const auto Observation = Compose(Input);
			ExpectFormulaSuccess(Test, Observation, EComposedP1Outcome::P2Required);
			const TArray<int32> ExpectedAttackStamina{6, 5};
			const TArray<int32> ExpectedDefenseStamina{4, 3};
			const TArray<FName> ExpectedCards{CarrierId, RunnerId, MarkerId, HelperId};
			Test.TestTrue(TEXT("Helper attack stamina order"), Observation.AssemblyResult.ResolverInput.Attacker.ParticipatingStamina == ExpectedAttackStamina);
			Test.TestTrue(TEXT("Helper defense stamina order"), Observation.AssemblyResult.ResolverInput.Defender.ParticipatingStamina == ExpectedDefenseStamina);
			Test.TestTrue(TEXT("Helper CardId order"), Observation.AssemblyResult.ResolverInput.InvolvedCardIds == ExpectedCards);
			break;
		}
		case 12:
		{
			const auto Input = MakePlanInput(false, true);
			Test.TestTrue(TEXT("Real Eligibility accepts cross-side duplicate CardId"), Input.ParticipantEligibilityResult.bSuccess);
			const auto Observation = Compose(Input);
			ExpectFormulaSuccess(Test, Observation, EComposedP1Outcome::P2Required);
			const TArray<FName> ExpectedCards{CarrierId, RunnerId, CarrierId};
			Test.TestTrue(TEXT("Plan preserves allowed duplicate CardId"), Observation.PlanResult.FormulaPlan.InvolvedCardIds == ExpectedCards);
			Test.TestTrue(TEXT("Assembly preserves allowed duplicate CardId"), Observation.AssemblyResult.ResolverInput.InvolvedCardIds == ExpectedCards);
			Test.TestTrue(TEXT("Resolution log preserves allowed duplicate CardId"), Observation.ExecutionResult.FormulaResolutionResult.MatchLogEntry.InvolvedCardIds == ExpectedCards);
			break;
		}
		case 13:
		{
			auto Input = MakePlanInput();
			Input.ParticipantEligibilityResult.bSuccess = false;
			const auto Observation = Compose(Input);
			Test.TestFalse(TEXT("Plan fails"), Observation.PlanResult.bSuccess);
			Test.TestEqual(TEXT("Plan exposes Eligibility failure"), Observation.PlanResult.ErrorCode, EThroughBallBehindDefenseP1PlanQueryErrorCode::ParticipantEligibilityFailed);
			Test.TestFalse(TEXT("Eligibility failure skips Assembler"), Observation.bAssemblerCalled);
			Test.TestFalse(TEXT("Eligibility failure skips Executor"), Observation.bExecutorCalled);
			Test.TestEqual(TEXT("Failure has no composed outcome"), Observation.Outcome, EComposedP1Outcome::None);
			break;
		}
		case 14:
		{
			auto Input = MakePlanInput();
			Input.SelectedBranch = EThroughBallSelectedBranch::Feet;
			const auto Observation = Compose(Input);
			Test.TestFalse(TEXT("Unsupported branch fails Plan"), Observation.PlanResult.bSuccess);
			Test.TestEqual(TEXT("Plan exposes unsupported branch"), Observation.PlanResult.ErrorCode, EThroughBallBehindDefenseP1PlanQueryErrorCode::UnsupportedBranch);
			Test.TestFalse(TEXT("Unsupported branch skips Assembler"), Observation.bAssemblerCalled);
			Test.TestFalse(TEXT("Unsupported branch skips Executor"), Observation.bExecutorCalled);
			break;
		}
		case 15:
		{
			auto Input = MakePlanInput(true);
			Input.AttackD6 = 6;
			Input.DefenseD6 = 1;
			const auto Observation = Compose(Input);
			ExpectFormulaSuccess(Test, Observation, EComposedP1Outcome::P2Required);
			Test.TestEqual(TEXT("Assembly receives Plan Decision"), Observation.AssemblyResult.Input.PlanQueryResult.Decision, Observation.PlanResult.Decision);
			Test.TestTrue(TEXT("Assembly receives Plan CardIds"), Observation.AssemblyResult.Input.PlanQueryResult.FormulaPlan.InvolvedCardIds == Observation.PlanResult.FormulaPlan.InvolvedCardIds);
			Test.TestTrue(TEXT("Execution receives Resolver CardIds"), Observation.ExecutionResult.Input.ResolverInputAssemblyResult.ResolverInput.InvolvedCardIds == Observation.AssemblyResult.ResolverInput.InvolvedCardIds);
			const auto& Resolution = Observation.ExecutionResult.FormulaResolutionResult;
			Test.TestEqual(TEXT("Resolution uses Transition"), Resolution.FormulaType, EFormulaType::Transition);
			Test.TestEqual(TEXT("Resolution winner matches P2 outcome"), Resolution.Winner, EFormulaWinner::Attacker);
			Test.TestEqual(TEXT("MatchLog LogId bridged"), Resolution.MatchLogEntry.LogId, Input.LogId);
			Test.TestEqual(TEXT("MatchLog TurnIndex bridged"), Resolution.MatchLogEntry.TurnIndex, Input.TurnIndex);
			Test.TestEqual(TEXT("MatchLog actor is attacking owner"), Resolution.MatchLogEntry.ActingPlayerId, AttackingOwnerId);
			Test.TestTrue(TEXT("MatchLog CardIds bridged"), Resolution.MatchLogEntry.InvolvedCardIds == Observation.PlanResult.FormulaPlan.InvolvedCardIds);
			break;
		}
		case 16:
		{
			auto Input = MakePlanInput(true);
			Input.AttackD6 = 6;
			Input.DefenseD6 = 1;
			const auto Before = Input;
			const auto Observation = Compose(Input);
			ExpectFormulaSuccess(Test, Observation, EComposedP1Outcome::P2Required);
			AssertSelectedInputFieldsUnchanged(Test, Before, Input);
			break;
		}
		case 17:
		{
			auto Input = MakePlanInput(true);
			Input.AttackD6 = 6;
			Input.DefenseD6 = 1;
			const auto First = Compose(Input);
			const auto Second = Compose(Input);
			ExpectFormulaSuccess(Test, First, EComposedP1Outcome::P2Required);
			ExpectFormulaSuccess(Test, Second, EComposedP1Outcome::P2Required);
			AssertSelectedDeterministicFields(Test, First, Second);
			break;
		}
		case 18:
		{
			auto OutOfPlayInput = MakePlanInput();
			OutOfPlayInput.AttackD6 = 1;
			const auto OutOfPlayObservation = Compose(OutOfPlayInput);
			ExpectOutOfPlay(Test, OutOfPlayObservation);

			auto FormulaInput = MakePlanInput(true);
			FormulaInput.AttackD6 = 6;
			FormulaInput.DefenseD6 = 1;
			const auto FormulaObservation = Compose(FormulaInput);
			ExpectFormulaSuccess(Test, FormulaObservation, EComposedP1Outcome::P2Required);

			FString Source;
			const FString Path = FPaths::Combine(
				FPaths::ProjectDir(),
				TEXT("Source/FMCodex/CoreRules/ThroughBallBehindDefenseP1FormulaResolutionCompositionTests.cpp"));
			Test.TestTrue(TEXT("Composition source is readable"), FFileHelper::LoadFileToString(Source, *Path));
			const int32 HelperStart = Source.Find(TEXT("// Composition helper begin."));
			const int32 HelperEnd = Source.Find(TEXT("// Composition helper end."));
			Test.TestTrue(TEXT("Composition helper region is found"), HelperStart != INDEX_NONE && HelperEnd > HelperStart);
			const FString HelperSource = Source.Mid(HelperStart, HelperEnd - HelperStart);
			Test.TestEqual(TEXT("Plan Query has one helper call point"), CountOccurrences(HelperSource, TEXT("FThroughBallBehindDefenseP1PlanQuery::Evaluate")), 1);
			Test.TestEqual(TEXT("Assembler has one helper call point"), CountOccurrences(HelperSource, TEXT("FThroughBallBehindDefenseP1FormulaResolverInputAssembler::Assemble")), 1);
			Test.TestEqual(TEXT("Executor has one helper call point"), CountOccurrences(HelperSource, TEXT("FThroughBallBehindDefenseP1FormulaResolutionExecutor::Execute")), 1);
			Test.TestEqual(TEXT("Eligibility has no helper call point"), CountOccurrences(HelperSource, TEXT("FThroughBallParticipantEligibilityQuery::Evaluate")), 0);
			Test.TestEqual(TEXT("Branch Selection has no helper call point"), CountOccurrences(HelperSource, TEXT("FThroughBallBranchSelectionQuery::Select")), 0);
			Test.TestEqual(TEXT("FormulaResolver has no direct helper call point"), CountOccurrences(HelperSource, TEXT("UFormulaResolver::ResolveFormula")), 0);
			for (const TCHAR* Forbidden : {
				TEXT("FormulaAttackFlow"),
				TEXT("MatchPlay"),
				TEXT("MatchState"),
				TEXT("FMath::Rand"),
				TEXT("RandomStream")})
			{
				Test.TestFalse(Forbidden, HelperSource.Contains(Forbidden));
			}
			break;
		}
		default:
			Test.AddError(FString::Printf(TEXT("Unknown Composition case %d"), Case));
			return false;
		}

		return true;
	}
}

#define THROUGH_BALL_BEHIND_DEFENSE_P1_COMPOSITION_TEST(ClassName, TestName, CaseNumber) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
		ClassName, \
		"FMCodex.CoreRules.ThroughBallBehindDefenseP1FormulaResolutionComposition." TestName, \
		EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter) \
	bool ClassName::RunTest(const FString& Parameters) \
	{ \
		return RunCase(*this, CaseNumber); \
	}

THROUGH_BALL_BEHIND_DEFENSE_P1_COMPOSITION_TEST(FThroughBallBehindDefenseP1Composition01, "01OutOfPlayAttackOneWithoutDefenseD6", 1)
THROUGH_BALL_BEHIND_DEFENSE_P1_COMPOSITION_TEST(FThroughBallBehindDefenseP1Composition02, "02OutOfPlayAttackTwoIgnoresGarbageDefenseD6", 2)
THROUGH_BALL_BEHIND_DEFENSE_P1_COMPOSITION_TEST(FThroughBallBehindDefenseP1Composition03, "03OutOfPlayTerminalMetadata", 3)
THROUGH_BALL_BEHIND_DEFENSE_P1_COMPOSITION_TEST(FThroughBallBehindDefenseP1Composition04, "04NoHelperDefenderStoppedAttack", 4)
THROUGH_BALL_BEHIND_DEFENSE_P1_COMPOSITION_TEST(FThroughBallBehindDefenseP1Composition05, "05HelperDefenderStoppedAttack", 5)
THROUGH_BALL_BEHIND_DEFENSE_P1_COMPOSITION_TEST(FThroughBallBehindDefenseP1Composition06, "06NoHelperP2Required", 6)
THROUGH_BALL_BEHIND_DEFENSE_P1_COMPOSITION_TEST(FThroughBallBehindDefenseP1Composition07, "07HelperP2RequiredWithRunner", 7)
THROUGH_BALL_BEHIND_DEFENSE_P1_COMPOSITION_TEST(FThroughBallBehindDefenseP1Composition08, "08BaseAndModifierBridge", 8)
THROUGH_BALL_BEHIND_DEFENSE_P1_COMPOSITION_TEST(FThroughBallBehindDefenseP1Composition09, "09P1DiceBridgeAndBranchDiceIsolation", 9)
THROUGH_BALL_BEHIND_DEFENSE_P1_COMPOSITION_TEST(FThroughBallBehindDefenseP1Composition10, "10NoHelperStaminaAndCardOrder", 10)
THROUGH_BALL_BEHIND_DEFENSE_P1_COMPOSITION_TEST(FThroughBallBehindDefenseP1Composition11, "11HelperStaminaAndCardOrder", 11)
THROUGH_BALL_BEHIND_DEFENSE_P1_COMPOSITION_TEST(FThroughBallBehindDefenseP1Composition12, "12OwnerAwareDuplicateCardAcceptanceAndPreservation", 12)
THROUGH_BALL_BEHIND_DEFENSE_P1_COMPOSITION_TEST(FThroughBallBehindDefenseP1Composition13, "13EligibilityFailureShortCircuit", 13)
THROUGH_BALL_BEHIND_DEFENSE_P1_COMPOSITION_TEST(FThroughBallBehindDefenseP1Composition14, "14UnsupportedBranchShortCircuit", 14)
THROUGH_BALL_BEHIND_DEFENSE_P1_COMPOSITION_TEST(FThroughBallBehindDefenseP1Composition15, "15ProductionResultAndMatchLogBridge", 15)
THROUGH_BALL_BEHIND_DEFENSE_P1_COMPOSITION_TEST(FThroughBallBehindDefenseP1Composition16, "16SelectedInputFieldsRemainUnchanged", 16)
THROUGH_BALL_BEHIND_DEFENSE_P1_COMPOSITION_TEST(FThroughBallBehindDefenseP1Composition17, "17SelectedObservedFieldsRepeatDeterministically", 17)
THROUGH_BALL_BEHIND_DEFENSE_P1_COMPOSITION_TEST(FThroughBallBehindDefenseP1Composition18, "18DependencyAndStateBoundary", 18)

#undef THROUGH_BALL_BEHIND_DEFENSE_P1_COMPOSITION_TEST

}

#endif
