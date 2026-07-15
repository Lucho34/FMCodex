#if WITH_DEV_AUTOMATION_TESTS

#include "ThroughBallFeetFormulaResolutionExecutor.h"

#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace ThroughBallFeetFormulaResolutionExecutorTests
{
	const FName CarrierId(TEXT("ThroughBall.Carrier"));
	const FName RunnerId(TEXT("ThroughBall.Runner"));
	const FName MarkerId(TEXT("ThroughBall.Marker"));
	const FName HelperId(TEXT("ThroughBall.Helper"));
	const FName GoalkeeperId(TEXT("ThroughBall.Goalkeeper"));
	const FName AttackingOwnerId(TEXT("Player.Attacking"));
	const FName DefendingOwnerId(TEXT("Player.Defending"));
	const FGuid LogId(321, 322, 323, 324);

	FThroughBallFeetFormulaPlan MakePlan(
		const bool bHasHelper = false,
		const bool bHasGoalkeeper = false)
	{
		FThroughBallFeetFormulaPlan Plan;
		Plan.FormulaType = EFormulaType::Finishing;
		Plan.CarrierId = CarrierId;
		Plan.CarrierPassing = 6;
		Plan.CarrierStamina = 5;
		Plan.RunnerId = RunnerId;
		Plan.RunnerOffBall = 6;
		Plan.RunnerStamina = 4;
		Plan.AttackD6 = 3;
		Plan.AttackBaseValue = 6.0f;
		Plan.AttackExternalModifier = 0.0f;
		Plan.AttackParticipatingStamina = {5, 4};

		Plan.MarkerId = MarkerId;
		Plan.MarkerTackling = 4;
		Plan.MarkerStamina = 3;
		Plan.bHasHelper = bHasHelper;
		if (bHasHelper)
		{
			Plan.HelperId = HelperId;
			Plan.HelperMarking = 4;
			Plan.HelperStamina = 2;
		}

		Plan.bHasActiveGoalkeeper = bHasGoalkeeper;
		if (bHasGoalkeeper)
		{
			Plan.ActiveGoalkeeperId = GoalkeeperId;
			Plan.GoalkeeperOneOnOne = 5;
			Plan.GoalkeeperStamina = 6;
		}

		Plan.DefenseD6 = 4;
		Plan.DefenseBaseValue = 3.0f;
		Plan.DefenseExternalModifier = 1.0f;
		Plan.DefenseParticipatingStamina.Add(Plan.MarkerStamina);
		if (bHasHelper)
		{
			Plan.DefenseParticipatingStamina.Add(Plan.HelperStamina);
		}
		if (bHasGoalkeeper)
		{
			Plan.DefenseParticipatingStamina.Add(Plan.GoalkeeperStamina);
		}

		Plan.LogId = LogId;
		Plan.TurnIndex = 8;
		Plan.AttackingOwnerId = AttackingOwnerId;
		Plan.DefendingOwnerId = DefendingOwnerId;
		Plan.InvolvedCardIds = {CarrierId, RunnerId, MarkerId};
		if (bHasHelper)
		{
			Plan.InvolvedCardIds.Add(HelperId);
		}
		if (bHasGoalkeeper)
		{
			Plan.InvolvedCardIds.Add(GoalkeeperId);
		}

		Plan.GoalScorerCardId = RunnerId;
		Plan.bAttackVictoryIsGoal = true;
		Plan.bDefenderVictoryIsMiss = true;
		Plan.bAttackEndsAfterResolution = true;
		Plan.bContinueResolution = false;
		return Plan;
	}

	FThroughBallFeetFormulaResolutionExecutionInput MakeExecutionInput(
		const FThroughBallFeetFormulaPlan& Plan = MakePlan())
	{
		FThroughBallFeetFormulaResolverInputAssemblyInput AssemblyInput;
		AssemblyInput.FormulaPlan = Plan;

		FThroughBallFeetFormulaResolutionExecutionInput Input;
		Input.ResolverInputAssemblyResult =
			FThroughBallFeetFormulaResolverInputAssembler::Assemble(
				AssemblyInput);
		return Input;
	}

	bool ArePlansEqual(
		const FThroughBallFeetFormulaPlan& Left,
		const FThroughBallFeetFormulaPlan& Right)
	{
		return Left.FormulaType == Right.FormulaType
			&& Left.CarrierId == Right.CarrierId
			&& Left.CarrierPassing == Right.CarrierPassing
			&& Left.CarrierStamina == Right.CarrierStamina
			&& Left.RunnerId == Right.RunnerId
			&& Left.RunnerOffBall == Right.RunnerOffBall
			&& Left.RunnerStamina == Right.RunnerStamina
			&& Left.AttackD6 == Right.AttackD6
			&& Left.AttackBaseValue == Right.AttackBaseValue
			&& Left.AttackExternalModifier == Right.AttackExternalModifier
			&& Left.AttackParticipatingStamina
				== Right.AttackParticipatingStamina
			&& Left.MarkerId == Right.MarkerId
			&& Left.MarkerTackling == Right.MarkerTackling
			&& Left.MarkerStamina == Right.MarkerStamina
			&& Left.bHasHelper == Right.bHasHelper
			&& Left.HelperId == Right.HelperId
			&& Left.HelperMarking == Right.HelperMarking
			&& Left.HelperStamina == Right.HelperStamina
			&& Left.bHasActiveGoalkeeper == Right.bHasActiveGoalkeeper
			&& Left.ActiveGoalkeeperId == Right.ActiveGoalkeeperId
			&& Left.GoalkeeperOneOnOne == Right.GoalkeeperOneOnOne
			&& Left.GoalkeeperStamina == Right.GoalkeeperStamina
			&& Left.DefenseD6 == Right.DefenseD6
			&& Left.DefenseBaseValue == Right.DefenseBaseValue
			&& Left.DefenseExternalModifier == Right.DefenseExternalModifier
			&& Left.DefenseParticipatingStamina
				== Right.DefenseParticipatingStamina
			&& Left.LogId == Right.LogId
			&& Left.TurnIndex == Right.TurnIndex
			&& Left.AttackingOwnerId == Right.AttackingOwnerId
			&& Left.DefendingOwnerId == Right.DefendingOwnerId
			&& Left.InvolvedCardIds == Right.InvolvedCardIds
			&& Left.GoalScorerCardId == Right.GoalScorerCardId
			&& Left.bAttackVictoryIsGoal == Right.bAttackVictoryIsGoal
			&& Left.bDefenderVictoryIsMiss == Right.bDefenderVictoryIsMiss
			&& Left.bAttackEndsAfterResolution
				== Right.bAttackEndsAfterResolution
			&& Left.bContinueResolution == Right.bContinueResolution;
	}

	bool AreSideInputsEqual(
		const FFormulaSideInput& Left,
		const FFormulaSideInput& Right)
	{
		return Left.BaseValue == Right.BaseValue
			&& Left.Modifier == Right.Modifier
			&& Left.ComparePoint == Right.ComparePoint
			&& Left.bComparePointWasRolledOnD6
				== Right.bComparePointWasRolledOnD6
			&& Left.ParticipatingStamina == Right.ParticipatingStamina;
	}

	bool AreResolverInputsEqual(
		const FFormulaResolverInput& Left,
		const FFormulaResolverInput& Right)
	{
		return Left.FormulaType == Right.FormulaType
			&& AreSideInputsEqual(Left.Attacker, Right.Attacker)
			&& AreSideInputsEqual(Left.Defender, Right.Defender)
			&& Left.bGoalkeeperParticipated
				== Right.bGoalkeeperParticipated
			&& Left.LogId == Right.LogId
			&& Left.TurnIndex == Right.TurnIndex
			&& Left.AttackerPlayerId == Right.AttackerPlayerId
			&& Left.DefenderPlayerId == Right.DefenderPlayerId
			&& Left.InvolvedCardIds == Right.InvolvedCardIds;
	}

	bool AreAssemblyResultsEqual(
		const FThroughBallFeetFormulaResolverInputAssemblyResult& Left,
		const FThroughBallFeetFormulaResolverInputAssemblyResult& Right)
	{
		return Left.bSuccess == Right.bSuccess
			&& Left.ErrorCode == Right.ErrorCode
			&& Left.ErrorMessage == Right.ErrorMessage
			&& Left.InvalidField == Right.InvalidField
			&& ArePlansEqual(
				Left.Input.FormulaPlan,
				Right.Input.FormulaPlan)
			&& Left.bHasResolverInput == Right.bHasResolverInput
			&& AreResolverInputsEqual(
				Left.ResolverInput,
				Right.ResolverInput);
	}

	bool AreMatchLogsEqual(
		const FMatchLogEntry& Left,
		const FMatchLogEntry& Right)
	{
		return Left.LogId == Right.LogId
			&& Left.TurnIndex == Right.TurnIndex
			&& Left.EventType == Right.EventType
			&& Left.ActingPlayerId == Right.ActingPlayerId
			&& Left.InvolvedCardIds == Right.InvolvedCardIds
			&& Left.DiceResults == Right.DiceResults
			&& Left.DiceOrder == Right.DiceOrder
			&& Left.FormulaType == Right.FormulaType
			&& Left.FormulaInputs == Right.FormulaInputs
			&& Left.FormulaResult == Right.FormulaResult
			&& Left.ScoreAfterEvent.OrderIndependentCompareEqual(
				Right.ScoreAfterEvent)
			&& Left.ZonesAfterEvent.OrderIndependentCompareEqual(
				Right.ZonesAfterEvent);
	}

	bool AreFormulaResolutionsEqual(
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
			&& AreMatchLogsEqual(Left.MatchLogEntry, Right.MatchLogEntry);
	}

	bool IsDefaultFormulaResolution(const FFormulaResolutionResult& Result)
	{
		return AreFormulaResolutionsEqual(Result, FFormulaResolutionResult{});
	}

	FThroughBallFeetFormulaResolutionExecutionResult ExecuteSuccess(
		FAutomationTestBase& Test,
		const FThroughBallFeetFormulaResolutionExecutionInput& Input)
	{
		const FThroughBallFeetFormulaResolutionExecutionResult Result =
			FThroughBallFeetFormulaResolutionExecutor::Execute(Input);
		Test.TestTrue(TEXT("Execution succeeds"), Result.bSuccess);
		Test.TestEqual(
			TEXT("Success has no error"),
			Result.ErrorCode,
			EThroughBallFeetFormulaResolutionExecutionErrorCode::None);
		Test.TestTrue(
			TEXT("Success message is empty"),
			Result.ErrorMessage.IsEmpty());
		Test.TestTrue(
			TEXT("Success field is empty"),
			Result.InvalidField.IsNone());
		Test.TestTrue(
			TEXT("Success has Formula Resolution"),
			Result.bHasFormulaResolution);
		return Result;
	}

	FThroughBallFeetFormulaResolutionExecutionResult ExecuteFailure(
		FAutomationTestBase& Test,
		const FThroughBallFeetFormulaResolutionExecutionInput& Input,
		const EThroughBallFeetFormulaResolutionExecutionErrorCode ErrorCode,
		const FName InvalidField)
	{
		const FThroughBallFeetFormulaResolutionExecutionResult Result =
			FThroughBallFeetFormulaResolutionExecutor::Execute(Input);
		Test.TestFalse(TEXT("Execution fails"), Result.bSuccess);
		Test.TestFalse(
			TEXT("Failure has no Formula Resolution"),
			Result.bHasFormulaResolution);
		Test.TestEqual(TEXT("Expected error"), Result.ErrorCode, ErrorCode);
		Test.TestEqual(
			TEXT("Expected invalid field"),
			Result.InvalidField,
			InvalidField);
		Test.TestFalse(
			TEXT("Failure message is non-empty"),
			Result.ErrorMessage.IsEmpty());
		Test.TestTrue(
			TEXT("Failure Resolution is default"),
			IsDefaultFormulaResolution(Result.FormulaResolutionResult));
		return Result;
	}

	void ExpectInvalidAssemblyResult(
		FAutomationTestBase& Test,
		const FThroughBallFeetFormulaResolutionExecutionInput& Input,
		const FName Field)
	{
		ExecuteFailure(
			Test,
			Input,
			EThroughBallFeetFormulaResolutionExecutionErrorCode
				::InvalidResolverInputAssemblyResult,
			Field);
	}

	bool LoadProductionSource(
		FAutomationTestBase& Test,
		FString& OutHeader,
		FString& OutImplementation)
	{
		const FString Root = FPaths::Combine(
			FPaths::ProjectDir(),
			TEXT("Source/FMCodex/CoreRules"));
		const bool bHeaderLoaded = FFileHelper::LoadFileToString(
			OutHeader,
			*FPaths::Combine(
				Root,
				TEXT("ThroughBallFeetFormulaResolutionExecutor.h")));
		const bool bImplementationLoaded = FFileHelper::LoadFileToString(
			OutImplementation,
			*FPaths::Combine(
				Root,
				TEXT("ThroughBallFeetFormulaResolutionExecutor.cpp")));
		Test.TestTrue(TEXT("Executor header loaded"), bHeaderLoaded);
		Test.TestTrue(TEXT("Executor implementation loaded"), bImplementationLoaded);
		return bHeaderLoaded && bImplementationLoaded;
	}

	bool RunCase(FAutomationTestBase& Test, const int32 Case)
	{
		FThroughBallFeetFormulaPlan Plan;
		FThroughBallFeetFormulaResolutionExecutionInput Input;
		FThroughBallFeetFormulaResolutionExecutionResult Result;
		switch (Case)
		{
		case 1:
			Input = MakeExecutionInput();
			Input.ResolverInputAssemblyResult.bSuccess = false;
			Input.ResolverInputAssemblyResult.ErrorCode =
				EThroughBallFeetFormulaResolverInputAssemblyErrorCode
					::InvalidAttackFormulaData;
			Input.ResolverInputAssemblyResult.ErrorMessage =
				TEXT("Upstream failure.");
			Input.ResolverInputAssemblyResult.InvalidField = TEXT("AttackD6");
			Input.ResolverInputAssemblyResult.bHasResolverInput = false;
			Result = ExecuteFailure(
				Test,
				Input,
				EThroughBallFeetFormulaResolutionExecutionErrorCode
					::ResolverInputAssemblyFailed,
				TEXT("ResolverInputAssemblyResult"));
			Test.TestEqual(
				TEXT("Upstream error is retained in Input"),
				Result.Input.ResolverInputAssemblyResult.ErrorCode,
				EThroughBallFeetFormulaResolverInputAssemblyErrorCode
					::InvalidAttackFormulaData);
			break;
		case 2:
			Input = MakeExecutionInput();
			Input.ResolverInputAssemblyResult.ErrorCode =
				EThroughBallFeetFormulaResolverInputAssemblyErrorCode
					::InvalidLogContext;
			ExpectInvalidAssemblyResult(Test, Input, TEXT("ErrorCode"));
			break;
		case 3:
			Input = MakeExecutionInput();
			Input.ResolverInputAssemblyResult.ErrorMessage = TEXT("Unexpected");
			ExpectInvalidAssemblyResult(Test, Input, TEXT("ErrorMessage"));
			break;
		case 4:
			Input = MakeExecutionInput();
			Input.ResolverInputAssemblyResult.InvalidField = TEXT("Unexpected");
			ExpectInvalidAssemblyResult(Test, Input, TEXT("InvalidField"));
			break;
		case 5:
			Input = MakeExecutionInput();
			Input.ResolverInputAssemblyResult.bHasResolverInput = false;
			ExpectInvalidAssemblyResult(Test, Input, TEXT("bHasResolverInput"));
			break;
		case 6:
			Input = MakeExecutionInput();
			Input.ResolverInputAssemblyResult.Input.FormulaPlan.FormulaType =
				EFormulaType::Transition;
			ExpectInvalidAssemblyResult(Test, Input, TEXT("FormulaPlan.FormulaType"));
			break;
		case 7:
			Input = MakeExecutionInput();
			Input.ResolverInputAssemblyResult.ResolverInput.FormulaType =
				EFormulaType::Transition;
			ExpectInvalidAssemblyResult(Test, Input, TEXT("ResolverInput.FormulaType"));
			break;
		case 8:
			Input = MakeExecutionInput();
			Input.ResolverInputAssemblyResult.Input.FormulaPlan.LogId.Invalidate();
			Input.ResolverInputAssemblyResult.ResolverInput.LogId.Invalidate();
			ExpectInvalidAssemblyResult(Test, Input, TEXT("ResolverInput.LogId"));
			Input = MakeExecutionInput();
			Input.ResolverInputAssemblyResult.Input.FormulaPlan.TurnIndex = -1;
			Input.ResolverInputAssemblyResult.ResolverInput.TurnIndex = -1;
			ExpectInvalidAssemblyResult(Test, Input, TEXT("ResolverInput.TurnIndex"));
			Input = MakeExecutionInput();
			Input.ResolverInputAssemblyResult.Input.FormulaPlan.AttackingOwnerId =
				NAME_None;
			Input.ResolverInputAssemblyResult.ResolverInput.AttackerPlayerId =
				NAME_None;
			ExpectInvalidAssemblyResult(
				Test,
				Input,
				TEXT("ResolverInput.AttackerPlayerId"));
			Input = MakeExecutionInput();
			Input.ResolverInputAssemblyResult.Input.FormulaPlan.DefendingOwnerId =
				NAME_None;
			Input.ResolverInputAssemblyResult.ResolverInput.DefenderPlayerId =
				NAME_None;
			ExpectInvalidAssemblyResult(
				Test,
				Input,
				TEXT("ResolverInput.DefenderPlayerId"));
			Input = MakeExecutionInput();
			Input.ResolverInputAssemblyResult.Input.FormulaPlan.DefendingOwnerId =
				AttackingOwnerId;
			Input.ResolverInputAssemblyResult.ResolverInput.DefenderPlayerId =
				AttackingOwnerId;
			ExpectInvalidAssemblyResult(
				Test,
				Input,
				TEXT("ResolverInput.DefenderPlayerId"));
			break;
		case 9:
			Input = MakeExecutionInput();
			Input.ResolverInputAssemblyResult.ErrorCode =
				EThroughBallFeetFormulaResolverInputAssemblyErrorCode
					::InvalidLogContext;
			Input.ResolverInputAssemblyResult.ErrorMessage = TEXT("Also invalid");
			Input.ResolverInputAssemblyResult.bHasResolverInput = false;
			ExpectInvalidAssemblyResult(Test, Input, TEXT("ErrorCode"));
			break;
		case 10:
			Input = MakeExecutionInput();
			Input.ResolverInputAssemblyResult.ResolverInput.Attacker.BaseValue += 1.0f;
			ExpectInvalidAssemblyResult(
				Test, Input, TEXT("ResolverInput.Attacker.BaseValue"));
			Input = MakeExecutionInput();
			Input.ResolverInputAssemblyResult.ResolverInput.Attacker.Modifier += 1.0f;
			ExpectInvalidAssemblyResult(
				Test, Input, TEXT("ResolverInput.Attacker.Modifier"));
			Input = MakeExecutionInput();
			++Input.ResolverInputAssemblyResult.ResolverInput.Attacker.ComparePoint;
			ExpectInvalidAssemblyResult(
				Test, Input, TEXT("ResolverInput.Attacker.ComparePoint"));
			break;
		case 11:
			Input = MakeExecutionInput();
			Input.ResolverInputAssemblyResult.ResolverInput.Attacker
				.bComparePointWasRolledOnD6 = false;
			ExpectInvalidAssemblyResult(
				Test,
				Input,
				TEXT("ResolverInput.Attacker.bComparePointWasRolledOnD6"));
			break;
		case 12:
			Input = MakeExecutionInput();
			Input.ResolverInputAssemblyResult.ResolverInput.Attacker
				.ParticipatingStamina = {5};
			ExpectInvalidAssemblyResult(
				Test,
				Input,
				TEXT("ResolverInput.Attacker.ParticipatingStamina"));
			break;
		case 13:
			Input = MakeExecutionInput();
			Input.ResolverInputAssemblyResult.ResolverInput.Defender.BaseValue += 1.0f;
			ExpectInvalidAssemblyResult(
				Test, Input, TEXT("ResolverInput.Defender.BaseValue"));
			Input = MakeExecutionInput();
			Input.ResolverInputAssemblyResult.ResolverInput.Defender.Modifier += 1.0f;
			ExpectInvalidAssemblyResult(
				Test, Input, TEXT("ResolverInput.Defender.Modifier"));
			Input = MakeExecutionInput();
			++Input.ResolverInputAssemblyResult.ResolverInput.Defender.ComparePoint;
			ExpectInvalidAssemblyResult(
				Test, Input, TEXT("ResolverInput.Defender.ComparePoint"));
			break;
		case 14:
			Input = MakeExecutionInput();
			Input.ResolverInputAssemblyResult.ResolverInput.Defender
				.bComparePointWasRolledOnD6 = false;
			ExpectInvalidAssemblyResult(
				Test,
				Input,
				TEXT("ResolverInput.Defender.bComparePointWasRolledOnD6"));
			break;
		case 15:
			Input = MakeExecutionInput();
			Input.ResolverInputAssemblyResult.ResolverInput.Defender
				.ParticipatingStamina = {3, 2};
			ExpectInvalidAssemblyResult(
				Test,
				Input,
				TEXT("ResolverInput.Defender.ParticipatingStamina"));
			Input = MakeExecutionInput();
			Input.ResolverInputAssemblyResult.ResolverInput
				.bGoalkeeperParticipated = true;
			ExpectInvalidAssemblyResult(
				Test,
				Input,
				TEXT("ResolverInput.bGoalkeeperParticipated"));
			break;
		case 16:
			Input = MakeExecutionInput();
			Input.ResolverInputAssemblyResult.ResolverInput.LogId =
				FGuid(1, 1, 1, 1);
			ExpectInvalidAssemblyResult(Test, Input, TEXT("ResolverInput.LogId"));
			Input = MakeExecutionInput();
			++Input.ResolverInputAssemblyResult.ResolverInput.TurnIndex;
			ExpectInvalidAssemblyResult(Test, Input, TEXT("ResolverInput.TurnIndex"));
			Input = MakeExecutionInput();
			Input.ResolverInputAssemblyResult.ResolverInput.AttackerPlayerId =
				TEXT("Other.Attacker");
			ExpectInvalidAssemblyResult(
				Test, Input, TEXT("ResolverInput.AttackerPlayerId"));
			Input = MakeExecutionInput();
			Input.ResolverInputAssemblyResult.ResolverInput.DefenderPlayerId =
				TEXT("Other.Defender");
			ExpectInvalidAssemblyResult(
				Test, Input, TEXT("ResolverInput.DefenderPlayerId"));
			Input = MakeExecutionInput();
			Input.ResolverInputAssemblyResult.ResolverInput.InvolvedCardIds.Swap(0, 1);
			ExpectInvalidAssemblyResult(
				Test, Input, TEXT("ResolverInput.InvolvedCardIds"));
			break;
		case 17:
			Plan = MakePlan();
			Plan.AttackBaseValue = 8.0f;
			Plan.DefenseBaseValue = 1.0f;
			Result = ExecuteSuccess(Test, MakeExecutionInput(Plan));
			Test.TestEqual(TEXT("Attacker wins"), Result.FormulaResolutionResult.Winner, EFormulaWinner::Attacker);
			Test.TestTrue(TEXT("Attacker victory is Goal"), Result.FormulaResolutionResult.bIsGoal);
			Test.TestTrue(TEXT("Goal ends attack"), Result.FormulaResolutionResult.bAttackEnded);
			Test.TestFalse(TEXT("Finishing does not continue"), Result.FormulaResolutionResult.bContinueResolution);
			break;
		case 18:
			Plan = MakePlan();
			Plan.AttackBaseValue = 1.0f;
			Plan.DefenseBaseValue = 8.0f;
			Result = ExecuteSuccess(Test, MakeExecutionInput(Plan));
			Test.TestEqual(TEXT("Defender wins"), Result.FormulaResolutionResult.Winner, EFormulaWinner::Defender);
			Test.TestFalse(TEXT("Defender victory is not Goal"), Result.FormulaResolutionResult.bIsGoal);
			Test.TestTrue(TEXT("Miss semantic ends attack"), Result.FormulaResolutionResult.bAttackEnded);
			Test.TestFalse(TEXT("Miss does not continue"), Result.FormulaResolutionResult.bContinueResolution);
			Test.TestTrue(TEXT("Input retains Miss policy"), Result.Input.ResolverInputAssemblyResult.Input.FormulaPlan.bDefenderVictoryIsMiss);
			break;
		case 19:
			Plan = MakePlan(false, true);
			Plan.AttackBaseValue = 4.0f;
			Plan.AttackExternalModifier = 0.0f;
			Plan.AttackD6 = 3;
			Plan.DefenseBaseValue = 1.0f;
			Plan.DefenseExternalModifier = 2.0f;
			Plan.DefenseD6 = 4;
			Result = ExecuteSuccess(Test, MakeExecutionInput(Plan));
			Test.TestEqual(TEXT("GK wins numeric tie"), Result.FormulaResolutionResult.Winner, EFormulaWinner::Defender);
			Test.TestEqual(TEXT("GK tie reason"), Result.FormulaResolutionResult.WinReason, EFormulaWinReason::DefenderWinsGoalkeeperTie);
			break;
		case 20:
			Plan = MakePlan();
			Plan.AttackBaseValue = 4.0f;
			Plan.AttackD6 = 3;
			Plan.DefenseBaseValue = 2.0f;
			Plan.DefenseExternalModifier = 1.0f;
			Plan.DefenseD6 = 4;
			Plan.CarrierStamina = 8;
			Plan.RunnerStamina = 7;
			Plan.AttackParticipatingStamina = {8, 7};
			Plan.MarkerStamina = 3;
			Plan.DefenseParticipatingStamina = {3};
			Result = ExecuteSuccess(Test, MakeExecutionInput(Plan));
			Test.TestEqual(TEXT("Attack stamina wins"), Result.FormulaResolutionResult.Winner, EFormulaWinner::Attacker);
			Test.TestEqual(TEXT("Stamina tie-break reason"), Result.FormulaResolutionResult.WinReason, EFormulaWinReason::StaminaTieBreaker);
			break;
		case 21:
			Plan = MakePlan();
			Plan.AttackBaseValue = 4.0f;
			Plan.AttackD6 = 3;
			Plan.DefenseBaseValue = 2.0f;
			Plan.DefenseExternalModifier = 1.0f;
			Plan.DefenseD6 = 4;
			Plan.CarrierStamina = 1;
			Plan.RunnerStamina = 1;
			Plan.AttackParticipatingStamina = {1, 1};
			Plan.MarkerStamina = 9;
			Plan.DefenseParticipatingStamina = {9};
			Result = ExecuteSuccess(Test, MakeExecutionInput(Plan));
			Test.TestEqual(TEXT("Defense stamina wins"), Result.FormulaResolutionResult.Winner, EFormulaWinner::Defender);
			Test.TestEqual(TEXT("Stamina tie-break reason"), Result.FormulaResolutionResult.WinReason, EFormulaWinReason::StaminaTieBreaker);
			break;
		case 22:
			Plan = MakePlan();
			Plan.AttackBaseValue = 4.0f;
			Plan.AttackD6 = 3;
			Plan.DefenseBaseValue = 2.0f;
			Plan.DefenseExternalModifier = 1.0f;
			Plan.DefenseD6 = 4;
			Plan.CarrierStamina = 5;
			Plan.RunnerStamina = 4;
			Plan.AttackParticipatingStamina = {5, 4};
			Plan.MarkerStamina = 9;
			Plan.DefenseParticipatingStamina = {9};
			Result = ExecuteSuccess(Test, MakeExecutionInput(Plan));
			Test.TestEqual(TEXT("Equal stamina favors Defender"), Result.FormulaResolutionResult.Winner, EFormulaWinner::Defender);
			Test.TestEqual(TEXT("Equal stamina reason"), Result.FormulaResolutionResult.WinReason, EFormulaWinReason::DefenderWinsEqualStamina);
			break;
		case 23:
			Plan = MakePlan();
			Plan.AttackBaseValue = 0.0f;
			Plan.AttackD6 = 6;
			Plan.DefenseBaseValue = 20.0f;
			Plan.DefenseExternalModifier = 0.0f;
			Plan.DefenseD6 = 2;
			Result = ExecuteSuccess(Test, MakeExecutionInput(Plan));
			Test.TestTrue(TEXT("Attacker fast suppression triggers"), Result.FormulaResolutionResult.bFastSuppressionTriggered);
			Test.TestEqual(TEXT("Attacker wins suppression"), Result.FormulaResolutionResult.Winner, EFormulaWinner::Attacker);
			Test.TestEqual(TEXT("Suppression reason"), Result.FormulaResolutionResult.WinReason, EFormulaWinReason::FastSuppression);
			break;
		case 24:
			Plan = MakePlan();
			Plan.AttackBaseValue = 20.0f;
			Plan.AttackD6 = 1;
			Plan.DefenseBaseValue = 0.0f;
			Plan.DefenseExternalModifier = 0.0f;
			Plan.DefenseD6 = 6;
			Result = ExecuteSuccess(Test, MakeExecutionInput(Plan));
			Test.TestTrue(TEXT("Defender fast suppression triggers"), Result.FormulaResolutionResult.bFastSuppressionTriggered);
			Test.TestEqual(TEXT("Defender wins suppression"), Result.FormulaResolutionResult.Winner, EFormulaWinner::Defender);
			Test.TestEqual(TEXT("Suppression reason"), Result.FormulaResolutionResult.WinReason, EFormulaWinReason::FastSuppression);
			break;
		case 25:
			Plan = MakePlan(true, true);
			Plan.AttackBaseValue = 4.5f;
			Plan.AttackD6 = 3;
			Plan.DefenseBaseValue = 2.5f;
			Plan.DefenseExternalModifier = 1.0f;
			Plan.DefenseD6 = 4;
			Result = ExecuteSuccess(Test, MakeExecutionInput(Plan));
			Test.TestEqual(TEXT("FormulaType retained"), Result.FormulaResolutionResult.FormulaType, EFormulaType::Finishing);
			Test.TestEqual(TEXT("Attacker decimal final retained"), Result.FormulaResolutionResult.AttackerFinalValue, 7.5f);
			Test.TestEqual(TEXT("Defender decimal final retained"), Result.FormulaResolutionResult.DefenderFinalValue, 7.5f);
			Test.TestEqual(TEXT("LogId retained"), Result.FormulaResolutionResult.MatchLogEntry.LogId, LogId);
			Test.TestEqual(TEXT("Turn retained"), Result.FormulaResolutionResult.MatchLogEntry.TurnIndex, 8);
			Test.TestEqual(TEXT("Cards retained"), Result.FormulaResolutionResult.MatchLogEntry.InvolvedCardIds, Plan.InvolvedCardIds);
			break;
		case 26:
			Input = MakeExecutionInput();
			Input.ResolverInputAssemblyResult.bSuccess = false;
			Result = ExecuteFailure(
				Test,
				Input,
				EThroughBallFeetFormulaResolutionExecutionErrorCode
					::ResolverInputAssemblyFailed,
				TEXT("ResolverInputAssemblyResult"));
			Test.TestTrue(TEXT("Every Resolution field remains default"), IsDefaultFormulaResolution(Result.FormulaResolutionResult));
			break;
		case 27:
			Input = MakeExecutionInput(MakePlan(true, true));
			{
				const FFormulaResolutionResult Expected = UFormulaResolver::ResolveFormula(
					Input.ResolverInputAssemblyResult.ResolverInput);
				Result = ExecuteSuccess(Test, Input);
				Test.TestTrue(TEXT("Result retains complete Input"), AreAssemblyResultsEqual(Result.Input.ResolverInputAssemblyResult, Input.ResolverInputAssemblyResult));
				Test.TestTrue(TEXT("Result retains complete Resolver output"), AreFormulaResolutionsEqual(Result.FormulaResolutionResult, Expected));
			}
			break;
		case 28:
			Input = MakeExecutionInput(MakePlan(true, true));
			{
				const FThroughBallFeetFormulaResolutionExecutionInput Before = Input;
				ExecuteSuccess(Test, Input);
				Test.TestTrue(TEXT("Execution Input unchanged"), AreAssemblyResultsEqual(Input.ResolverInputAssemblyResult, Before.ResolverInputAssemblyResult));
			}
			break;
		case 29:
			Input = MakeExecutionInput(MakePlan(true, true));
			{
				const auto First = ExecuteSuccess(Test, Input);
				const auto Second = ExecuteSuccess(Test, Input);
				Test.TestEqual(TEXT("Deterministic success"), First.bSuccess, Second.bSuccess);
				Test.TestEqual(TEXT("Deterministic Resolution presence"), First.bHasFormulaResolution, Second.bHasFormulaResolution);
				Test.TestEqual(TEXT("Deterministic error"), First.ErrorCode, Second.ErrorCode);
				Test.TestEqual(TEXT("Deterministic error message"), First.ErrorMessage, Second.ErrorMessage);
				Test.TestEqual(TEXT("Deterministic invalid field"), First.InvalidField, Second.InvalidField);
				Test.TestTrue(TEXT("Deterministic Input"), AreAssemblyResultsEqual(First.Input.ResolverInputAssemblyResult, Second.Input.ResolverInputAssemblyResult));
				Test.TestTrue(TEXT("Deterministic Resolution"), AreFormulaResolutionsEqual(First.FormulaResolutionResult, Second.FormulaResolutionResult));
			}
			break;
		case 30:
			{
				FString Header;
				FString Implementation;
				if (LoadProductionSource(Test, Header, Implementation))
				{
					const FString ResolverCall(
						TEXT("UFormulaResolver::ResolveFormula("));
					int32 ResolverCallCount = 0;
					FString Remaining = Implementation;
					while (Remaining.Split(
						ResolverCall,
						nullptr,
						&Remaining))
					{
						++ResolverCallCount;
					}
					Test.TestEqual(TEXT("Production calls FormulaResolver exactly once"), ResolverCallCount, 1);
					Test.TestFalse(TEXT("Does not call Feet Assembler"), Implementation.Contains(TEXT("FThroughBallFeetFormulaResolverInputAssembler::Assemble")));
					Test.TestFalse(TEXT("Does not call SingleCard path"), Implementation.Contains(TEXT("SingleCard")));
					Test.TestFalse(TEXT("Does not call FormulaAttackFlow"), Implementation.Contains(TEXT("FormulaAttackFlow")));
					Test.TestFalse(TEXT("Does not call MatchPlay"), Implementation.Contains(TEXT("MatchPlay")));
					Test.TestFalse(TEXT("Does not access Match State"), Implementation.Contains(TEXT("MatchState")));
					Test.TestFalse(TEXT("Does not use RNG"), Implementation.Contains(TEXT("FMath::Rand")) || Implementation.Contains(TEXT("RandomStream")));
					Test.TestFalse(TEXT("Does not use Provider or DataTable"), Implementation.Contains(TEXT("Provider")) || Implementation.Contains(TEXT("DataTable")));
					Test.TestFalse(TEXT("Does not add generic framework"), (Header + Implementation).Contains(TEXT("GenericFormulaExecutor")));
				}
			}
			break;
		default:
			Test.AddError(TEXT("Unknown Through Ball Feet Formula Resolution Executor test case."));
			break;
		}
		return !Test.HasAnyErrors();
	}
}

#define THROUGH_BALL_FEET_EXECUTOR_TEST(Number, Name) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
		FThroughBallFeetExecutorTest##Number, \
		"FMCodex.CoreRules.ThroughBallFeetFormulaResolutionExecutor." Name, \
		EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter) \
	bool FThroughBallFeetExecutorTest##Number::RunTest(const FString& Parameters) \
	{ \
		return ThroughBallFeetFormulaResolutionExecutorTests::RunCase(*this, Number); \
	}

THROUGH_BALL_FEET_EXECUTOR_TEST(1, "01RejectsAssemblyFailure")
THROUGH_BALL_FEET_EXECUTOR_TEST(2, "02RejectsSuccessWithErrorCode")
THROUGH_BALL_FEET_EXECUTOR_TEST(3, "03RejectsSuccessWithErrorMessage")
THROUGH_BALL_FEET_EXECUTOR_TEST(4, "04RejectsSuccessWithInvalidField")
THROUGH_BALL_FEET_EXECUTOR_TEST(5, "05RejectsSuccessWithoutResolverInput")
THROUGH_BALL_FEET_EXECUTOR_TEST(6, "06RejectsNonFinishingPlan")
THROUGH_BALL_FEET_EXECUTOR_TEST(7, "07RejectsNonFinishingResolverInput")
THROUGH_BALL_FEET_EXECUTOR_TEST(8, "08RejectsInvalidResolverContext")
THROUGH_BALL_FEET_EXECUTOR_TEST(9, "09UsesFrozenSuccessStatePriority")
THROUGH_BALL_FEET_EXECUTOR_TEST(10, "10RejectsAttackValueMappingMismatch")
THROUGH_BALL_FEET_EXECUTOR_TEST(11, "11RejectsAttackRolledFlagMismatch")
THROUGH_BALL_FEET_EXECUTOR_TEST(12, "12RejectsAttackStaminaMismatch")
THROUGH_BALL_FEET_EXECUTOR_TEST(13, "13RejectsDefenseValueMappingMismatch")
THROUGH_BALL_FEET_EXECUTOR_TEST(14, "14RejectsDefenseRolledFlagMismatch")
THROUGH_BALL_FEET_EXECUTOR_TEST(15, "15RejectsDefenseStaminaOrGoalkeeperMismatch")
THROUGH_BALL_FEET_EXECUTOR_TEST(16, "16RejectsContextMappingMismatch")
THROUGH_BALL_FEET_EXECUTOR_TEST(17, "17ResolvesAttackerGoal")
THROUGH_BALL_FEET_EXECUTOR_TEST(18, "18ResolvesDefenderMissSemantic")
THROUGH_BALL_FEET_EXECUTOR_TEST(19, "19ResolvesGoalkeeperTie")
THROUGH_BALL_FEET_EXECUTOR_TEST(20, "20ResolvesAttackStaminaTieBreaker")
THROUGH_BALL_FEET_EXECUTOR_TEST(21, "21ResolvesDefenseStaminaTieBreaker")
THROUGH_BALL_FEET_EXECUTOR_TEST(22, "22ResolvesEqualStaminaForDefender")
THROUGH_BALL_FEET_EXECUTOR_TEST(23, "23ResolvesAttackerFastSuppression")
THROUGH_BALL_FEET_EXECUTOR_TEST(24, "24ResolvesDefenderFastSuppression")
THROUGH_BALL_FEET_EXECUTOR_TEST(25, "25PreservesDecimalLogAndCards")
THROUGH_BALL_FEET_EXECUTOR_TEST(26, "26FailureKeepsDefaultResolution")
THROUGH_BALL_FEET_EXECUTOR_TEST(27, "27PreservesInputAndResolverResult")
THROUGH_BALL_FEET_EXECUTOR_TEST(28, "28PreservesCallerInput")
THROUGH_BALL_FEET_EXECUTOR_TEST(29, "29IsDeterministic")
THROUGH_BALL_FEET_EXECUTOR_TEST(30, "30UsesOnlyAllowedDependencies")

#undef THROUGH_BALL_FEET_EXECUTOR_TEST

#endif
