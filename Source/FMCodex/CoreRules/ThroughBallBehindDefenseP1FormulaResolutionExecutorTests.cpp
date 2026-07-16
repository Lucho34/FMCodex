#if WITH_DEV_AUTOMATION_TESTS

#include "ThroughBallBehindDefenseP1FormulaResolutionExecutor.h"

#include "Algo/Reverse.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace ThroughBallBehindDefenseP1FormulaResolutionExecutorTests
{
	const FName CarrierId(TEXT("ThroughBall.Carrier"));
	const FName RunnerId(TEXT("ThroughBall.Runner"));
	const FName MarkerId(TEXT("ThroughBall.Marker"));
	const FName HelperId(TEXT("ThroughBall.Helper"));
	const FName AttackingOwnerId(TEXT("Player.Attacking"));
	const FName DefendingOwnerId(TEXT("Player.Defending"));
	const FGuid LogId(741, 742, 743, 744);

	FThroughBallBehindDefenseP1FormulaPlan MakePlan(
		const bool bHasHelper = false)
	{
		FThroughBallBehindDefenseP1FormulaPlan Plan;
		Plan.FormulaType = EFormulaType::Transition;
		Plan.CarrierId = CarrierId;
		Plan.CarrierPassing = 7;
		Plan.CarrierStamina = 5;
		Plan.RunnerId = RunnerId;
		Plan.RunnerSpeed = 5;
		Plan.RunnerStamina = 4;
		Plan.AttackD6 = 4;
		Plan.AttackBaseValue = 6.0f;
		Plan.AttackExternalModifier = 0.0f;
		Plan.AttackParticipatingStamina = {5, 4};

		Plan.MarkerId = MarkerId;
		Plan.MarkerMarking = 5;
		Plan.MarkerStamina = 3;
		Plan.bHasHelper = bHasHelper;
		if (bHasHelper)
		{
			Plan.HelperId = HelperId;
			Plan.HelperSpeed = 3;
			Plan.HelperStamina = 2;
		}

		Plan.DefenseD6 = 3;
		Plan.DefenseBaseValue = bHasHelper ? 4.0f : 2.5f;
		Plan.DefenseExternalModifier = 1.0f;
		Plan.DefenseParticipatingStamina.Add(Plan.MarkerStamina);
		if (bHasHelper)
		{
			Plan.DefenseParticipatingStamina.Add(Plan.HelperStamina);
		}

		Plan.LogId = LogId;
		Plan.TurnIndex = 9;
		Plan.AttackingOwnerId = AttackingOwnerId;
		Plan.DefendingOwnerId = DefendingOwnerId;
		Plan.InvolvedCardIds = {CarrierId, RunnerId, MarkerId};
		if (bHasHelper)
		{
			Plan.InvolvedCardIds.Add(HelperId);
		}
		Plan.bAttackerVictoryRequiresP2 = true;
		Plan.bDefenderVictoryEndsAttack = true;
		return Plan;
	}

	FThroughBallBehindDefenseP1PlanQueryResult MakePlanQueryResult(
		const FThroughBallBehindDefenseP1FormulaPlan& Plan = MakePlan())
	{
		FThroughBallBehindDefenseP1PlanQueryResult Result;
		Result.bSuccess = true;
		Result.Decision =
			EThroughBallBehindDefenseP1PlanQueryDecision
				::FormulaResolutionRequired;
		Result.Input.SelectedBranch =
			EThroughBallSelectedBranch::BehindDefense;
		Result.Input.bHasAttackD6 = true;
		Result.Input.AttackD6 = Plan.AttackD6;
		Result.Input.bHasDefenseD6 = true;
		Result.Input.DefenseD6 = Plan.DefenseD6;
		Result.Input.LogId = Plan.LogId;
		Result.Input.TurnIndex = Plan.TurnIndex;
		Result.bHasFormulaPlan = true;
		Result.FormulaPlan = Plan;
		return Result;
	}

	FThroughBallBehindDefenseP1FormulaResolutionExecutionInput
	MakeExecutionInput(
		const FThroughBallBehindDefenseP1FormulaPlan& Plan = MakePlan())
	{
		FThroughBallBehindDefenseP1FormulaResolverInputAssemblyInput AssemblyInput;
		AssemblyInput.PlanQueryResult = MakePlanQueryResult(Plan);

		FThroughBallBehindDefenseP1FormulaResolutionExecutionInput Input;
		Input.ResolverInputAssemblyResult =
			FThroughBallBehindDefenseP1FormulaResolverInputAssembler::Assemble(
				AssemblyInput);
		return Input;
	}

	bool ArePlansEqual(
		const FThroughBallBehindDefenseP1FormulaPlan& Left,
		const FThroughBallBehindDefenseP1FormulaPlan& Right)
	{
		return Left.FormulaType == Right.FormulaType
			&& Left.CarrierId == Right.CarrierId
			&& Left.CarrierPassing == Right.CarrierPassing
			&& Left.CarrierStamina == Right.CarrierStamina
			&& Left.RunnerId == Right.RunnerId
			&& Left.RunnerSpeed == Right.RunnerSpeed
			&& Left.RunnerStamina == Right.RunnerStamina
			&& Left.AttackD6 == Right.AttackD6
			&& Left.AttackBaseValue == Right.AttackBaseValue
			&& Left.AttackExternalModifier == Right.AttackExternalModifier
			&& Left.AttackParticipatingStamina
				== Right.AttackParticipatingStamina
			&& Left.MarkerId == Right.MarkerId
			&& Left.MarkerMarking == Right.MarkerMarking
			&& Left.MarkerStamina == Right.MarkerStamina
			&& Left.bHasHelper == Right.bHasHelper
			&& Left.HelperId == Right.HelperId
			&& Left.HelperSpeed == Right.HelperSpeed
			&& Left.HelperStamina == Right.HelperStamina
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
			&& Left.bAttackerVictoryRequiresP2
				== Right.bAttackerVictoryRequiresP2
			&& Left.bDefenderVictoryEndsAttack
				== Right.bDefenderVictoryEndsAttack;
	}

	bool ArePlanQueryResultsEqual(
		const FThroughBallBehindDefenseP1PlanQueryResult& Left,
		const FThroughBallBehindDefenseP1PlanQueryResult& Right)
	{
		return Left.bSuccess == Right.bSuccess
			&& Left.Decision == Right.Decision
			&& Left.ErrorCode == Right.ErrorCode
			&& Left.ErrorMessage == Right.ErrorMessage
			&& Left.InvalidField == Right.InvalidField
			&& Left.Input.SelectedBranch == Right.Input.SelectedBranch
			&& Left.Input.bHasAttackD6 == Right.Input.bHasAttackD6
			&& Left.Input.AttackD6 == Right.Input.AttackD6
			&& Left.Input.bHasDefenseD6 == Right.Input.bHasDefenseD6
			&& Left.Input.DefenseD6 == Right.Input.DefenseD6
			&& Left.Input.LogId == Right.Input.LogId
			&& Left.Input.TurnIndex == Right.Input.TurnIndex
			&& Left.bHasFormulaPlan == Right.bHasFormulaPlan
			&& ArePlansEqual(Left.FormulaPlan, Right.FormulaPlan)
			&& Left.bAttackEnded == Right.bAttackEnded
			&& Left.bContinueResolution == Right.bContinueResolution
			&& Left.bRequiresP2 == Right.bRequiresP2;
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
		const FThroughBallBehindDefenseP1FormulaResolverInputAssemblyResult& Left,
		const FThroughBallBehindDefenseP1FormulaResolverInputAssemblyResult& Right)
	{
		return Left.bSuccess == Right.bSuccess
			&& Left.ErrorCode == Right.ErrorCode
			&& Left.ErrorMessage == Right.ErrorMessage
			&& Left.InvalidField == Right.InvalidField
			&& ArePlanQueryResultsEqual(
				Left.Input.PlanQueryResult,
				Right.Input.PlanQueryResult)
			&& Left.bHasResolverInput == Right.bHasResolverInput
			&& AreResolverInputsEqual(Left.ResolverInput, Right.ResolverInput);
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

	bool AreExecutionResultsEqual(
		const FThroughBallBehindDefenseP1FormulaResolutionExecutionResult& Left,
		const FThroughBallBehindDefenseP1FormulaResolutionExecutionResult& Right)
	{
		return Left.bSuccess == Right.bSuccess
			&& Left.ErrorCode == Right.ErrorCode
			&& Left.ErrorMessage == Right.ErrorMessage
			&& Left.InvalidField == Right.InvalidField
			&& AreAssemblyResultsEqual(
				Left.Input.ResolverInputAssemblyResult,
				Right.Input.ResolverInputAssemblyResult)
			&& Left.bHasFormulaResolution == Right.bHasFormulaResolution
			&& AreFormulaResolutionsEqual(
				Left.FormulaResolutionResult,
				Right.FormulaResolutionResult)
			&& Left.Decision == Right.Decision
			&& Left.bAttackEnded == Right.bAttackEnded
			&& Left.bContinueResolution == Right.bContinueResolution
			&& Left.bRequiresP2 == Right.bRequiresP2
			&& Left.RunnerId == Right.RunnerId;
	}

	bool IsDefaultResolution(const FFormulaResolutionResult& Result)
	{
		return AreFormulaResolutionsEqual(Result, FFormulaResolutionResult{});
	}

	FThroughBallBehindDefenseP1FormulaResolutionExecutionResult ExecuteSuccess(
		FAutomationTestBase& Test,
		const FThroughBallBehindDefenseP1FormulaResolutionExecutionInput& Input)
	{
		const auto Result =
			FThroughBallBehindDefenseP1FormulaResolutionExecutor::Execute(Input);
		Test.TestTrue(TEXT("Execution succeeds"), Result.bSuccess);
		Test.TestEqual(
			TEXT("Success has no error"),
			Result.ErrorCode,
			EThroughBallBehindDefenseP1FormulaResolutionExecutionErrorCode::None);
		Test.TestTrue(TEXT("Success message is empty"), Result.ErrorMessage.IsEmpty());
		Test.TestTrue(TEXT("Success field is empty"), Result.InvalidField.IsNone());
		Test.TestTrue(TEXT("Success has Formula Resolution"), Result.bHasFormulaResolution);
		return Result;
	}

	FThroughBallBehindDefenseP1FormulaResolutionExecutionResult ExecuteFailure(
		FAutomationTestBase& Test,
		const FThroughBallBehindDefenseP1FormulaResolutionExecutionInput& Input,
		const EThroughBallBehindDefenseP1FormulaResolutionExecutionErrorCode ErrorCode,
		const FName InvalidField)
	{
		const auto Result =
			FThroughBallBehindDefenseP1FormulaResolutionExecutor::Execute(Input);
		Test.TestFalse(TEXT("Execution fails"), Result.bSuccess);
		Test.TestFalse(TEXT("Pre-call failure has no resolution"), Result.bHasFormulaResolution);
		Test.TestEqual(TEXT("Expected error"), Result.ErrorCode, ErrorCode);
		Test.TestEqual(TEXT("Expected invalid field"), Result.InvalidField, InvalidField);
		Test.TestFalse(TEXT("Failure message is non-empty"), Result.ErrorMessage.IsEmpty());
		Test.TestTrue(TEXT("Failure resolution is default"), IsDefaultResolution(Result.FormulaResolutionResult));
		Test.TestEqual(TEXT("Failure decision is None"), Result.Decision, EThroughBallBehindDefenseP1FormulaResolutionExecutionDecision::None);
		Test.TestFalse(TEXT("Failure does not end attack"), Result.bAttackEnded);
		Test.TestFalse(TEXT("Failure does not continue"), Result.bContinueResolution);
		Test.TestFalse(TEXT("Failure does not require P2"), Result.bRequiresP2);
		Test.TestTrue(TEXT("Failure RunnerId is None"), Result.RunnerId.IsNone());
		Test.TestTrue(TEXT("Failure preserves execution input"), AreAssemblyResultsEqual(Result.Input.ResolverInputAssemblyResult, Input.ResolverInputAssemblyResult));
		return Result;
	}

	void ExpectInvalidAssembly(
		FAutomationTestBase& Test,
		const FThroughBallBehindDefenseP1FormulaResolutionExecutionInput& Input,
		const FName Field)
	{
		ExecuteFailure(
			Test,
			Input,
			EThroughBallBehindDefenseP1FormulaResolutionExecutionErrorCode
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
				TEXT("ThroughBallBehindDefenseP1FormulaResolutionExecutor.h")));
		const bool bImplementationLoaded = FFileHelper::LoadFileToString(
			OutImplementation,
			*FPaths::Combine(
				Root,
				TEXT("ThroughBallBehindDefenseP1FormulaResolutionExecutor.cpp")));
		Test.TestTrue(TEXT("Executor header loaded"), bHeaderLoaded);
		Test.TestTrue(TEXT("Executor implementation loaded"), bImplementationLoaded);
		return bHeaderLoaded && bImplementationLoaded;
	}

	bool RunCase(FAutomationTestBase& Test, const int32 Case)
	{
		FThroughBallBehindDefenseP1FormulaPlan Plan;
		FThroughBallBehindDefenseP1FormulaResolutionExecutionInput Input;
		FThroughBallBehindDefenseP1FormulaResolutionExecutionResult Result;
		switch (Case)
		{
		case 1:
			Test.TestFalse(TEXT("Default Result fails safely"), Result.bSuccess);
			Test.TestFalse(TEXT("Default Result has no resolution"), Result.bHasFormulaResolution);
			Test.TestEqual(TEXT("Default decision"), Result.Decision, EThroughBallBehindDefenseP1FormulaResolutionExecutionDecision::None);
			Test.TestTrue(TEXT("Default RunnerId"), Result.RunnerId.IsNone());
			ExecuteFailure(Test, Input, EThroughBallBehindDefenseP1FormulaResolutionExecutionErrorCode::ResolverInputAssemblyFailed, TEXT("ResolverInputAssemblyResult"));
			break;
		case 2:
			Plan = MakePlan(); Plan.AttackBaseValue = 1.0f; Plan.DefenseBaseValue = 8.0f;
			Result = ExecuteSuccess(Test, MakeExecutionInput(Plan));
			Test.TestEqual(TEXT("Defender decision"), Result.Decision, EThroughBallBehindDefenseP1FormulaResolutionExecutionDecision::DefenderStoppedAttack);
			Test.TestTrue(TEXT("Defender ends attack"), Result.bAttackEnded);
			Test.TestFalse(TEXT("Defender does not continue"), Result.bContinueResolution);
			Test.TestFalse(TEXT("Defender does not require P2"), Result.bRequiresP2);
			Test.TestTrue(TEXT("Defender has no Runner continuation"), Result.RunnerId.IsNone());
			break;
		case 3:
			Plan = MakePlan(); Plan.AttackBaseValue = 10.0f; Plan.DefenseBaseValue = 1.0f;
			Result = ExecuteSuccess(Test, MakeExecutionInput(Plan));
			Test.TestEqual(TEXT("Attacker decision"), Result.Decision, EThroughBallBehindDefenseP1FormulaResolutionExecutionDecision::P2Required);
			Test.TestFalse(TEXT("Attacker does not end attack"), Result.bAttackEnded);
			Test.TestTrue(TEXT("Attacker continues"), Result.bContinueResolution);
			Test.TestTrue(TEXT("Attacker requires P2"), Result.bRequiresP2);
			Test.TestEqual(TEXT("Runner continuation"), Result.RunnerId, RunnerId);
			break;
		case 4:
			Input = MakeExecutionInput();
			{
				const FFormulaResolutionResult Expected = UFormulaResolver::ResolveFormula(Input.ResolverInputAssemblyResult.ResolverInput);
				Result = ExecuteSuccess(Test, Input);
				Test.TestTrue(TEXT("Complete execution input preserved"), AreAssemblyResultsEqual(Result.Input.ResolverInputAssemblyResult, Input.ResolverInputAssemblyResult));
				Test.TestTrue(TEXT("Actual Resolver result preserved"), AreFormulaResolutionsEqual(Result.FormulaResolutionResult, Expected));
			}
			break;
		case 5:
			Input = MakeExecutionInput(); Input.ResolverInputAssemblyResult.bSuccess = false;
			ExecuteFailure(Test, Input, EThroughBallBehindDefenseP1FormulaResolutionExecutionErrorCode::ResolverInputAssemblyFailed, TEXT("ResolverInputAssemblyResult"));
			break;
		case 6:
			Input = MakeExecutionInput(); Input.ResolverInputAssemblyResult.ErrorCode = EThroughBallBehindDefenseP1FormulaResolverInputAssemblyErrorCode::PlanQueryFailed;
			ExpectInvalidAssembly(Test, Input, TEXT("ErrorCode"));
			break;
		case 7:
			Input = MakeExecutionInput(); Input.ResolverInputAssemblyResult.ErrorMessage = TEXT("Injected");
			ExpectInvalidAssembly(Test, Input, TEXT("ErrorMessage"));
			break;
		case 8:
			Input = MakeExecutionInput(); Input.ResolverInputAssemblyResult.InvalidField = TEXT("Injected");
			ExpectInvalidAssembly(Test, Input, TEXT("InvalidField"));
			break;
		case 9:
			Input = MakeExecutionInput(); Input.ResolverInputAssemblyResult.bHasResolverInput = false;
			ExpectInvalidAssembly(Test, Input, TEXT("bHasResolverInput"));
			break;
		case 10:
			Input = MakeExecutionInput(); Input.ResolverInputAssemblyResult.Input.PlanQueryResult.bSuccess = false;
			ExpectInvalidAssembly(Test, Input, TEXT("PlanQueryResult.bSuccess"));
			break;
		case 11:
			Input = MakeExecutionInput(); Input.ResolverInputAssemblyResult.Input.PlanQueryResult.ErrorCode = EThroughBallBehindDefenseP1PlanQueryErrorCode::InvalidAttackD6;
			ExpectInvalidAssembly(Test, Input, TEXT("PlanQueryResult.ErrorCode"));
			break;
		case 12:
			Input = MakeExecutionInput(); Input.ResolverInputAssemblyResult.Input.PlanQueryResult.ErrorMessage = TEXT("Injected");
			ExpectInvalidAssembly(Test, Input, TEXT("PlanQueryResult.ErrorMessage"));
			break;
		case 13:
			Input = MakeExecutionInput(); Input.ResolverInputAssemblyResult.Input.PlanQueryResult.InvalidField = TEXT("Injected");
			ExpectInvalidAssembly(Test, Input, TEXT("PlanQueryResult.InvalidField"));
			break;
		case 14:
			Input = MakeExecutionInput(); Input.ResolverInputAssemblyResult.Input.PlanQueryResult.Decision = EThroughBallBehindDefenseP1PlanQueryDecision::None;
			ExpectInvalidAssembly(Test, Input, TEXT("PlanQueryResult.Decision"));
			break;
		case 15:
			Input = MakeExecutionInput(); Input.ResolverInputAssemblyResult.Input.PlanQueryResult.Decision = EThroughBallBehindDefenseP1PlanQueryDecision::OutOfPlay;
			ExpectInvalidAssembly(Test, Input, TEXT("PlanQueryResult.Decision"));
			break;
		case 16:
			Input = MakeExecutionInput(); Input.ResolverInputAssemblyResult.Input.PlanQueryResult.bHasFormulaPlan = false;
			ExpectInvalidAssembly(Test, Input, TEXT("PlanQueryResult.bHasFormulaPlan"));
			break;
		case 17:
			Input = MakeExecutionInput(); Input.ResolverInputAssemblyResult.Input.PlanQueryResult.bAttackEnded = true;
			ExpectInvalidAssembly(Test, Input, TEXT("PlanQueryResult.bAttackEnded"));
			break;
		case 18:
			Input = MakeExecutionInput(); Input.ResolverInputAssemblyResult.Input.PlanQueryResult.bContinueResolution = true;
			ExpectInvalidAssembly(Test, Input, TEXT("PlanQueryResult.bContinueResolution"));
			break;
		case 19:
			Input = MakeExecutionInput(); Input.ResolverInputAssemblyResult.Input.PlanQueryResult.bRequiresP2 = true;
			ExpectInvalidAssembly(Test, Input, TEXT("PlanQueryResult.bRequiresP2"));
			break;
		case 20:
			Input = MakeExecutionInput(); Input.ResolverInputAssemblyResult.Input.PlanQueryResult.FormulaPlan.FormulaType = EFormulaType::Finishing;
			ExpectInvalidAssembly(Test, Input, TEXT("FormulaPlan.FormulaType"));
			break;
		case 21:
			Input = MakeExecutionInput(); Input.ResolverInputAssemblyResult.Input.PlanQueryResult.FormulaPlan.RunnerId = NAME_None;
			ExpectInvalidAssembly(Test, Input, TEXT("FormulaPlan.RunnerId"));
			break;
		case 22:
			Input = MakeExecutionInput(); Input.ResolverInputAssemblyResult.Input.PlanQueryResult.FormulaPlan.bAttackerVictoryRequiresP2 = false;
			ExpectInvalidAssembly(Test, Input, TEXT("FormulaPlan.bAttackerVictoryRequiresP2"));
			break;
		case 23:
			Input = MakeExecutionInput(); Input.ResolverInputAssemblyResult.Input.PlanQueryResult.FormulaPlan.bDefenderVictoryEndsAttack = false;
			ExpectInvalidAssembly(Test, Input, TEXT("FormulaPlan.bDefenderVictoryEndsAttack"));
			break;
		case 24:
			Input = MakeExecutionInput(); Input.ResolverInputAssemblyResult.ResolverInput.FormulaType = EFormulaType::Finishing;
			ExpectInvalidAssembly(Test, Input, TEXT("ResolverInput.FormulaType"));
			break;
		case 25:
			Input = MakeExecutionInput(); Input.ResolverInputAssemblyResult.ResolverInput.Attacker.BaseValue += 1.0f;
			ExpectInvalidAssembly(Test, Input, TEXT("ResolverInput.Attacker.BaseValue"));
			break;
		case 26:
			Input = MakeExecutionInput(); Input.ResolverInputAssemblyResult.ResolverInput.Attacker.Modifier += 1.0f;
			ExpectInvalidAssembly(Test, Input, TEXT("ResolverInput.Attacker.Modifier"));
			break;
		case 27:
			Input = MakeExecutionInput(); ++Input.ResolverInputAssemblyResult.ResolverInput.Attacker.ComparePoint;
			ExpectInvalidAssembly(Test, Input, TEXT("ResolverInput.Attacker.ComparePoint"));
			break;
		case 28:
			Input = MakeExecutionInput(); Input.ResolverInputAssemblyResult.ResolverInput.Attacker.bComparePointWasRolledOnD6 = false;
			ExpectInvalidAssembly(Test, Input, TEXT("ResolverInput.Attacker.bComparePointWasRolledOnD6"));
			break;
		case 29:
			Input = MakeExecutionInput(); Algo::Reverse(Input.ResolverInputAssemblyResult.ResolverInput.Attacker.ParticipatingStamina);
			ExpectInvalidAssembly(Test, Input, TEXT("ResolverInput.Attacker.ParticipatingStamina"));
			break;
		case 30:
			Input = MakeExecutionInput(); Input.ResolverInputAssemblyResult.ResolverInput.Defender.BaseValue += 1.0f;
			ExpectInvalidAssembly(Test, Input, TEXT("ResolverInput.Defender.BaseValue"));
			break;
		case 31:
			Input = MakeExecutionInput(); Input.ResolverInputAssemblyResult.ResolverInput.Defender.Modifier += 1.0f;
			ExpectInvalidAssembly(Test, Input, TEXT("ResolverInput.Defender.Modifier"));
			break;
		case 32:
			Input = MakeExecutionInput(); ++Input.ResolverInputAssemblyResult.ResolverInput.Defender.ComparePoint;
			ExpectInvalidAssembly(Test, Input, TEXT("ResolverInput.Defender.ComparePoint"));
			break;
		case 33:
			Input = MakeExecutionInput(); Input.ResolverInputAssemblyResult.ResolverInput.Defender.bComparePointWasRolledOnD6 = false;
			ExpectInvalidAssembly(Test, Input, TEXT("ResolverInput.Defender.bComparePointWasRolledOnD6"));
			break;
		case 34:
			Input = MakeExecutionInput(MakePlan(true)); Algo::Reverse(Input.ResolverInputAssemblyResult.ResolverInput.Defender.ParticipatingStamina);
			ExpectInvalidAssembly(Test, Input, TEXT("ResolverInput.Defender.ParticipatingStamina"));
			break;
		case 35:
			Input = MakeExecutionInput(); Input.ResolverInputAssemblyResult.ResolverInput.bGoalkeeperParticipated = true;
			ExpectInvalidAssembly(Test, Input, TEXT("ResolverInput.bGoalkeeperParticipated"));
			break;
		case 36:
			Input = MakeExecutionInput(); Input.ResolverInputAssemblyResult.ResolverInput.LogId.Invalidate();
			ExpectInvalidAssembly(Test, Input, TEXT("ResolverInput.LogId"));
			Input = MakeExecutionInput(); ++Input.ResolverInputAssemblyResult.ResolverInput.TurnIndex;
			ExpectInvalidAssembly(Test, Input, TEXT("ResolverInput.TurnIndex"));
			break;
		case 37:
			Input = MakeExecutionInput(); Input.ResolverInputAssemblyResult.ResolverInput.AttackerPlayerId = FName(TEXT("Other"));
			ExpectInvalidAssembly(Test, Input, TEXT("ResolverInput.AttackerPlayerId"));
			Input = MakeExecutionInput(); Input.ResolverInputAssemblyResult.ResolverInput.DefenderPlayerId = FName(TEXT("Other"));
			ExpectInvalidAssembly(Test, Input, TEXT("ResolverInput.DefenderPlayerId"));
			break;
		case 38:
			Input = MakeExecutionInput(); Input.ResolverInputAssemblyResult.ResolverInput.InvolvedCardIds.Pop();
			ExpectInvalidAssembly(Test, Input, TEXT("ResolverInput.InvolvedCardIds"));
			Input = MakeExecutionInput(); Swap(Input.ResolverInputAssemblyResult.ResolverInput.InvolvedCardIds[0], Input.ResolverInputAssemblyResult.ResolverInput.InvolvedCardIds[1]);
			ExpectInvalidAssembly(Test, Input, TEXT("ResolverInput.InvolvedCardIds"));
			Input = MakeExecutionInput(); Input.ResolverInputAssemblyResult.ResolverInput.InvolvedCardIds[1] = FName(TEXT("Other"));
			ExpectInvalidAssembly(Test, Input, TEXT("ResolverInput.InvolvedCardIds"));
			break;
		case 39:
			Input = MakeExecutionInput();
			Input.ResolverInputAssemblyResult.Input.PlanQueryResult.FormulaPlan.AttackBaseValue = 123.4f;
			Input.ResolverInputAssemblyResult.ResolverInput.Attacker.BaseValue = 123.4f;
			Input.ResolverInputAssemblyResult.Input.PlanQueryResult.FormulaPlan.DefenseBaseValue = -12.5f;
			Input.ResolverInputAssemblyResult.ResolverInput.Defender.BaseValue = -12.5f;
			Result = ExecuteSuccess(Test, Input);
			Test.TestEqual(TEXT("Attack Base is not recalculated"), Result.FormulaResolutionResult.AttackerFinalValue, 127.4f);
			Test.TestEqual(TEXT("Defense Base is not recalculated"), Result.FormulaResolutionResult.DefenderFinalValue, -8.5f);
			break;
		case 40:
			Plan = MakePlan(); Plan.AttackBaseValue = 1.0f; Plan.AttackD6 = 3; Plan.DefenseBaseValue = 0.0f; Plan.DefenseD6 = 3;
			Result = ExecuteSuccess(Test, MakeExecutionInput(Plan));
			Test.TestEqual(TEXT("Attacker stamina wins tie"), Result.FormulaResolutionResult.Winner, EFormulaWinner::Attacker);
			Test.TestEqual(TEXT("Real tie-break reason"), Result.FormulaResolutionResult.WinReason, EFormulaWinReason::StaminaTieBreaker);
			Test.TestFalse(TEXT("Transition is not Goal"), Result.FormulaResolutionResult.bIsGoal);
			break;
		case 41:
			Plan = MakePlan(); Plan.MarkerId = RunnerId; Plan.InvolvedCardIds = {CarrierId, RunnerId, RunnerId};
			Result = ExecuteSuccess(Test, MakeExecutionInput(Plan));
			Test.TestEqual(TEXT("Duplicate CardIds preserved"), Result.FormulaResolutionResult.MatchLogEntry.InvolvedCardIds, Plan.InvolvedCardIds);
			break;
		case 42:
			Input = MakeExecutionInput(MakePlan(true));
			{
				const auto First = FThroughBallBehindDefenseP1FormulaResolutionExecutor::Execute(Input);
				const auto Second = FThroughBallBehindDefenseP1FormulaResolutionExecutor::Execute(Input);
				Test.TestTrue(TEXT("Complete execution contract observation is deterministic"), AreExecutionResultsEqual(First, Second));
			}
			break;
		case 43:
			{
				FString Header;
				FString Implementation;
				if (LoadProductionSource(Test, Header, Implementation))
				{
					const FString ResolverCall(TEXT("UFormulaResolver::ResolveFormula("));
					int32 ResolverCallCount = 0;
					FString Remaining = Implementation;
					while (Remaining.Split(ResolverCall, nullptr, &Remaining))
					{
						++ResolverCallCount;
					}
					Test.TestEqual(TEXT("Production has one FormulaResolver call point"), ResolverCallCount, 1);
					Test.TestFalse(TEXT("Does not run P1 Plan Query"), Implementation.Contains(TEXT("FThroughBallBehindDefenseP1PlanQuery::Evaluate")));
					Test.TestFalse(TEXT("Does not run P1 Assembler"), Implementation.Contains(TEXT("FThroughBallBehindDefenseP1FormulaResolverInputAssembler::Assemble")));
					Test.TestFalse(TEXT("Does not run Eligibility"), Implementation.Contains(TEXT("EligibilityQuery::")));
					Test.TestFalse(TEXT("Does not run Branch Selection"), Implementation.Contains(TEXT("BranchSelectionQuery::")));
					Test.TestFalse(TEXT("Does not call Feet or SingleCard Executor"), Implementation.Contains(TEXT("ThroughBallFeetFormulaResolutionExecutor::")) || Implementation.Contains(TEXT("SingleCardFormulaResolutionExecutor::")));
					Test.TestFalse(TEXT("Does not call FormulaAttackFlow"), Implementation.Contains(TEXT("FormulaAttackFlow")));
					Test.TestFalse(TEXT("Does not call MatchPlay"), Implementation.Contains(TEXT("MatchPlay")));
					Test.TestFalse(TEXT("Does not access Match State"), Implementation.Contains(TEXT("MatchState")));
					Test.TestFalse(TEXT("Does not use RNG"), Implementation.Contains(TEXT("FMath::Rand")) || Implementation.Contains(TEXT("RandomStream")));
					Test.TestFalse(TEXT("No continuation struct"), (Header + Implementation).Contains(TEXT("ContinuationContext")));
				}
			}
			break;
		default:
			Test.AddError(TEXT("Unknown P1 Formula Resolution Executor test case."));
			break;
		}
		return !Test.HasAnyErrors();
	}
}

#define THROUGH_BALL_BEHIND_DEFENSE_P1_EXECUTOR_TEST(Number, Name) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
		FThroughBallBehindDefenseP1ExecutorTest##Number, \
		"FMCodex.CoreRules.ThroughBallBehindDefenseP1FormulaResolutionExecutor." Name, \
		EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter) \
	bool FThroughBallBehindDefenseP1ExecutorTest##Number::RunTest( \
		const FString& Parameters) \
	{ \
		return ThroughBallBehindDefenseP1FormulaResolutionExecutorTests::RunCase( \
			*this, \
			Number); \
	}

THROUGH_BALL_BEHIND_DEFENSE_P1_EXECUTOR_TEST(1, "01DefaultValuesAreFailureSafe")
THROUGH_BALL_BEHIND_DEFENSE_P1_EXECUTOR_TEST(2, "02MapsDefenderWinnerToStoppedAttack")
THROUGH_BALL_BEHIND_DEFENSE_P1_EXECUTOR_TEST(3, "03MapsAttackerWinnerToP2Required")
THROUGH_BALL_BEHIND_DEFENSE_P1_EXECUTOR_TEST(4, "04PreservesExecutionInputAndResolverResult")
THROUGH_BALL_BEHIND_DEFENSE_P1_EXECUTOR_TEST(5, "05RejectsAssemblyFailure")
THROUGH_BALL_BEHIND_DEFENSE_P1_EXECUTOR_TEST(6, "06RejectsAssemblySuccessWithErrorCode")
THROUGH_BALL_BEHIND_DEFENSE_P1_EXECUTOR_TEST(7, "07RejectsAssemblySuccessWithErrorMessage")
THROUGH_BALL_BEHIND_DEFENSE_P1_EXECUTOR_TEST(8, "08RejectsAssemblySuccessWithInvalidField")
THROUGH_BALL_BEHIND_DEFENSE_P1_EXECUTOR_TEST(9, "09RejectsMissingResolverInput")
THROUGH_BALL_BEHIND_DEFENSE_P1_EXECUTOR_TEST(10, "10RejectsNestedPlanQueryFailure")
THROUGH_BALL_BEHIND_DEFENSE_P1_EXECUTOR_TEST(11, "11RejectsNestedPlanQueryErrorCode")
THROUGH_BALL_BEHIND_DEFENSE_P1_EXECUTOR_TEST(12, "12RejectsNestedPlanQueryErrorMessage")
THROUGH_BALL_BEHIND_DEFENSE_P1_EXECUTOR_TEST(13, "13RejectsNestedPlanQueryInvalidField")
THROUGH_BALL_BEHIND_DEFENSE_P1_EXECUTOR_TEST(14, "14RejectsNoneDecision")
THROUGH_BALL_BEHIND_DEFENSE_P1_EXECUTOR_TEST(15, "15RejectsOutOfPlayDecision")
THROUGH_BALL_BEHIND_DEFENSE_P1_EXECUTOR_TEST(16, "16RejectsMissingFormulaPlan")
THROUGH_BALL_BEHIND_DEFENSE_P1_EXECUTOR_TEST(17, "17RejectsPreExecutedAttackEnd")
THROUGH_BALL_BEHIND_DEFENSE_P1_EXECUTOR_TEST(18, "18RejectsPreExecutedContinuation")
THROUGH_BALL_BEHIND_DEFENSE_P1_EXECUTOR_TEST(19, "19RejectsPreExecutedP2Requirement")
THROUGH_BALL_BEHIND_DEFENSE_P1_EXECUTOR_TEST(20, "20RejectsNonTransitionPlan")
THROUGH_BALL_BEHIND_DEFENSE_P1_EXECUTOR_TEST(21, "21RejectsMissingRunnerId")
THROUGH_BALL_BEHIND_DEFENSE_P1_EXECUTOR_TEST(22, "22RejectsAttackerWinnerPolicyMismatch")
THROUGH_BALL_BEHIND_DEFENSE_P1_EXECUTOR_TEST(23, "23RejectsDefenderWinnerPolicyMismatch")
THROUGH_BALL_BEHIND_DEFENSE_P1_EXECUTOR_TEST(24, "24RejectsFormulaTypeMappingMismatch")
THROUGH_BALL_BEHIND_DEFENSE_P1_EXECUTOR_TEST(25, "25RejectsAttackBaseMappingMismatch")
THROUGH_BALL_BEHIND_DEFENSE_P1_EXECUTOR_TEST(26, "26RejectsAttackModifierMappingMismatch")
THROUGH_BALL_BEHIND_DEFENSE_P1_EXECUTOR_TEST(27, "27RejectsAttackD6MappingMismatch")
THROUGH_BALL_BEHIND_DEFENSE_P1_EXECUTOR_TEST(28, "28RejectsAttackRolledFlagMismatch")
THROUGH_BALL_BEHIND_DEFENSE_P1_EXECUTOR_TEST(29, "29RejectsAttackStaminaOrderMismatch")
THROUGH_BALL_BEHIND_DEFENSE_P1_EXECUTOR_TEST(30, "30RejectsDefenseBaseMappingMismatch")
THROUGH_BALL_BEHIND_DEFENSE_P1_EXECUTOR_TEST(31, "31RejectsDefenseModifierMappingMismatch")
THROUGH_BALL_BEHIND_DEFENSE_P1_EXECUTOR_TEST(32, "32RejectsDefenseD6MappingMismatch")
THROUGH_BALL_BEHIND_DEFENSE_P1_EXECUTOR_TEST(33, "33RejectsDefenseRolledFlagMismatch")
THROUGH_BALL_BEHIND_DEFENSE_P1_EXECUTOR_TEST(34, "34RejectsDefenseStaminaOrderMismatch")
THROUGH_BALL_BEHIND_DEFENSE_P1_EXECUTOR_TEST(35, "35RejectsGoalkeeperParticipation")
THROUGH_BALL_BEHIND_DEFENSE_P1_EXECUTOR_TEST(36, "36RejectsLogContextMappingMismatch")
THROUGH_BALL_BEHIND_DEFENSE_P1_EXECUTOR_TEST(37, "37RejectsOwnerMappingMismatch")
THROUGH_BALL_BEHIND_DEFENSE_P1_EXECUTOR_TEST(38, "38RejectsCardCountOrderAndValueMismatch")
THROUGH_BALL_BEHIND_DEFENSE_P1_EXECUTOR_TEST(39, "39DoesNotRecalculateFormulaBase")
THROUGH_BALL_BEHIND_DEFENSE_P1_EXECUTOR_TEST(40, "40UsesRealTransitionStaminaTieBreaker")
THROUGH_BALL_BEHIND_DEFENSE_P1_EXECUTOR_TEST(41, "41PreservesCrossSideDuplicateCardId")
THROUGH_BALL_BEHIND_DEFENSE_P1_EXECUTOR_TEST(42, "42CompleteContractObservationIsDeterministic")
THROUGH_BALL_BEHIND_DEFENSE_P1_EXECUTOR_TEST(43, "43UsesOnlyAllowedDependencies")

#undef THROUGH_BALL_BEHIND_DEFENSE_P1_EXECUTOR_TEST

#endif
