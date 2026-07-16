#if WITH_DEV_AUTOMATION_TESTS

#include "ThroughBallBehindDefenseP1FormulaResolverInputAssembler.h"

#include "Misc/AutomationTest.h"

#include <limits>

namespace ThroughBallBehindDefenseP1FormulaResolverInputAssemblerTests
{
	const FName CarrierId(TEXT("P1_Carrier"));
	const FName RunnerId(TEXT("P1_Runner"));
	const FName MarkerId(TEXT("P1_Marker"));
	const FName HelperId(TEXT("P1_Helper"));
	const FName AttackingOwnerId(TEXT("P1_AttackingOwner"));
	const FName DefendingOwnerId(TEXT("P1_DefendingOwner"));

	using FAssemblyInput =
		FThroughBallBehindDefenseP1FormulaResolverInputAssemblyInput;
	using FAssemblyResult =
		FThroughBallBehindDefenseP1FormulaResolverInputAssemblyResult;
	using EAssemblyError =
		EThroughBallBehindDefenseP1FormulaResolverInputAssemblyErrorCode;

	FAssemblyInput MakeInput(const bool bHasHelper = false)
	{
		FAssemblyInput Input;
		FThroughBallBehindDefenseP1PlanQueryResult& QueryResult =
			Input.PlanQueryResult;
		QueryResult.bSuccess = true;
		QueryResult.Decision =
			EThroughBallBehindDefenseP1PlanQueryDecision
				::FormulaResolutionRequired;
		QueryResult.bHasFormulaPlan = true;

		FThroughBallBehindDefenseP1FormulaPlan& Plan = QueryResult.FormulaPlan;
		Plan.FormulaType = EFormulaType::Transition;
		Plan.CarrierId = CarrierId;
		Plan.CarrierPassing = 8;
		Plan.CarrierStamina = 4;
		Plan.RunnerId = RunnerId;
		Plan.RunnerSpeed = 3;
		Plan.RunnerStamina = 5;
		Plan.AttackD6 = 3;
		Plan.AttackBaseValue = 5.5f;
		Plan.AttackExternalModifier = 0.0f;
		Plan.AttackParticipatingStamina = {4, 5};
		Plan.MarkerId = MarkerId;
		Plan.MarkerMarking = 6;
		Plan.MarkerStamina = 6;
		Plan.bHasHelper = bHasHelper;
		if (bHasHelper)
		{
			Plan.HelperId = HelperId;
			Plan.HelperSpeed = 4;
			Plan.HelperStamina = 7;
		}
		Plan.DefenseD6 = 1;
		Plan.DefenseBaseValue = bHasHelper ? 5.0f : 3.0f;
		Plan.DefenseExternalModifier = 1.0f;
		Plan.DefenseParticipatingStamina = bHasHelper
			? TArray<int32>{6, 7}
			: TArray<int32>{6};
		Plan.LogId = FGuid::NewGuid();
		Plan.TurnIndex = 2;
		Plan.AttackingOwnerId = AttackingOwnerId;
		Plan.DefendingOwnerId = DefendingOwnerId;
		Plan.InvolvedCardIds = {CarrierId, RunnerId, MarkerId};
		if (bHasHelper)
		{
			Plan.InvolvedCardIds.Add(HelperId);
		}
		Plan.bAttackerVictoryRequiresP2 = true;
		Plan.bDefenderVictoryEndsAttack = true;
		return Input;
	}

	bool IsDefaultResolverInput(const FFormulaResolverInput& Input)
	{
		return Input.FormulaType == EFormulaType::None
			&& Input.Attacker.BaseValue == 0.0f
			&& Input.Attacker.Modifier == 0.0f
			&& Input.Attacker.ComparePoint == 0
			&& !Input.Attacker.bComparePointWasRolledOnD6
			&& Input.Attacker.ParticipatingStamina.IsEmpty()
			&& Input.Defender.BaseValue == 0.0f
			&& Input.Defender.Modifier == 0.0f
			&& Input.Defender.ComparePoint == 0
			&& !Input.Defender.bComparePointWasRolledOnD6
			&& Input.Defender.ParticipatingStamina.IsEmpty()
			&& !Input.bGoalkeeperParticipated
			&& !Input.LogId.IsValid()
			&& Input.TurnIndex == 0
			&& Input.AttackerPlayerId.IsNone()
			&& Input.DefenderPlayerId.IsNone()
			&& Input.InvolvedCardIds.IsEmpty();
	}

	bool AreResolverInputsEqual(
		const FFormulaResolverInput& Left,
		const FFormulaResolverInput& Right)
	{
		return Left.FormulaType == Right.FormulaType
			&& Left.Attacker.BaseValue == Right.Attacker.BaseValue
			&& Left.Attacker.Modifier == Right.Attacker.Modifier
			&& Left.Attacker.ComparePoint == Right.Attacker.ComparePoint
			&& Left.Attacker.bComparePointWasRolledOnD6
				== Right.Attacker.bComparePointWasRolledOnD6
			&& Left.Attacker.ParticipatingStamina
				== Right.Attacker.ParticipatingStamina
			&& Left.Defender.BaseValue == Right.Defender.BaseValue
			&& Left.Defender.Modifier == Right.Defender.Modifier
			&& Left.Defender.ComparePoint == Right.Defender.ComparePoint
			&& Left.Defender.bComparePointWasRolledOnD6
				== Right.Defender.bComparePointWasRolledOnD6
			&& Left.Defender.ParticipatingStamina
				== Right.Defender.ParticipatingStamina
			&& Left.bGoalkeeperParticipated == Right.bGoalkeeperParticipated
			&& Left.LogId == Right.LogId
			&& Left.TurnIndex == Right.TurnIndex
			&& Left.AttackerPlayerId == Right.AttackerPlayerId
			&& Left.DefenderPlayerId == Right.DefenderPlayerId
			&& Left.InvolvedCardIds == Right.InvolvedCardIds;
	}

	FAssemblyResult EvaluateSuccess(
		FAutomationTestBase& Test,
		const FAssemblyInput& Input)
	{
		const FAssemblyResult Result =
			FThroughBallBehindDefenseP1FormulaResolverInputAssembler::Assemble(Input);
		Test.TestTrue(TEXT("Assembly succeeds"), Result.bSuccess);
		Test.TestEqual(TEXT("No assembly error"), Result.ErrorCode, EAssemblyError::None);
		Test.TestTrue(TEXT("Assembly has Resolver Input"), Result.bHasResolverInput);
		return Result;
	}

	FAssemblyResult EvaluateFailure(
		FAutomationTestBase& Test,
		const FAssemblyInput& Input,
		const EAssemblyError ExpectedError,
		const FName ExpectedField)
	{
		const FAssemblyResult Result =
			FThroughBallBehindDefenseP1FormulaResolverInputAssembler::Assemble(Input);
		Test.TestFalse(TEXT("Assembly fails"), Result.bSuccess);
		Test.TestEqual(TEXT("Expected assembly error"), Result.ErrorCode, ExpectedError);
		Test.TestEqual(TEXT("Expected invalid field"), Result.InvalidField, ExpectedField);
		Test.TestFalse(TEXT("Failure has no Resolver Input"), Result.bHasResolverInput);
		Test.TestTrue(TEXT("Failure Resolver Input is default"),
			IsDefaultResolverInput(Result.ResolverInput));
		return Result;
	}

	bool RunCase(FAutomationTestBase& Test, const int32 Case)
	{
		FAssemblyInput Input = MakeInput();
		FAssemblyResult Result;
		switch (Case)
		{
		case 1:
		{
			const FAssemblyInput DefaultInput;
			Result = FThroughBallBehindDefenseP1FormulaResolverInputAssembler::Assemble(DefaultInput);
			Test.TestFalse(TEXT("Default Input fails"), Result.bSuccess);
			Test.TestEqual(TEXT("Default Input reports upstream failure"),
				Result.ErrorCode, EAssemblyError::PlanQueryFailed);
			Test.TestTrue(TEXT("Default Result Resolver Input is safe"),
				IsDefaultResolverInput(FAssemblyResult().ResolverInput));
			break;
		}
		case 2:
			EvaluateSuccess(Test, Input);
			break;
		case 3:
			EvaluateSuccess(Test, MakeInput(true));
			break;
		case 4:
			Result = EvaluateSuccess(Test, Input);
			Test.TestEqual(TEXT("Attack type"), Result.ResolverInput.FormulaType, EFormulaType::Transition);
			Test.TestEqual(TEXT("Attack Base"), Result.ResolverInput.Attacker.BaseValue, 5.5f);
			Test.TestEqual(TEXT("Attack Modifier"), Result.ResolverInput.Attacker.Modifier, 0.0f);
			Test.TestEqual(TEXT("Attack D6"), Result.ResolverInput.Attacker.ComparePoint, 3);
			Test.TestTrue(TEXT("Attack D6 rolled"), Result.ResolverInput.Attacker.bComparePointWasRolledOnD6);
			Test.TestTrue(TEXT("Attack stamina"), Result.ResolverInput.Attacker.ParticipatingStamina == TArray<int32>{4, 5});
			break;
		case 5:
			Result = EvaluateSuccess(Test, MakeInput(true));
			Test.TestEqual(TEXT("Defense Base"), Result.ResolverInput.Defender.BaseValue, 5.0f);
			Test.TestEqual(TEXT("Defense Modifier"), Result.ResolverInput.Defender.Modifier, 1.0f);
			Test.TestEqual(TEXT("Defense D6"), Result.ResolverInput.Defender.ComparePoint, 1);
			Test.TestTrue(TEXT("Defense D6 rolled"), Result.ResolverInput.Defender.bComparePointWasRolledOnD6);
			Test.TestTrue(TEXT("Defense stamina"), Result.ResolverInput.Defender.ParticipatingStamina == TArray<int32>{6, 7});
			break;
		case 6:
			Result = EvaluateSuccess(Test, Input);
			Test.TestEqual(TEXT("LogId mapped"), Result.ResolverInput.LogId, Input.PlanQueryResult.FormulaPlan.LogId);
			Test.TestEqual(TEXT("Turn mapped"), Result.ResolverInput.TurnIndex, 2);
			Test.TestEqual(TEXT("Attack Owner mapped"), Result.ResolverInput.AttackerPlayerId, AttackingOwnerId);
			Test.TestEqual(TEXT("Defense Owner mapped"), Result.ResolverInput.DefenderPlayerId, DefendingOwnerId);
			Test.TestTrue(TEXT("Cards mapped"), Result.ResolverInput.InvolvedCardIds == Input.PlanQueryResult.FormulaPlan.InvolvedCardIds);
			break;
		case 7:
			Result = EvaluateSuccess(Test, Input);
			Test.TestFalse(TEXT("P1 has no GK participation"), Result.ResolverInput.bGoalkeeperParticipated);
			Test.TestTrue(TEXT("Winner metadata remains in preserved Plan"),
				Result.Input.PlanQueryResult.FormulaPlan.bAttackerVictoryRequiresP2
				&& Result.Input.PlanQueryResult.FormulaPlan.bDefenderVictoryEndsAttack);
			break;
		case 8:
			Input.PlanQueryResult.Input.AttackD6 = 99;
			Result = EvaluateSuccess(Test, Input);
			Test.TestEqual(TEXT("Full upstream Input preserved"), Result.Input.PlanQueryResult.Input.AttackD6, 99);
			Test.TestEqual(TEXT("Plan preserved"), Result.Input.PlanQueryResult.FormulaPlan.RunnerId, RunnerId);
			break;
		case 9:
			Input.PlanQueryResult.bSuccess = false;
			Input.PlanQueryResult.Decision = EThroughBallBehindDefenseP1PlanQueryDecision::OutOfPlay;
			Input.PlanQueryResult.bHasFormulaPlan = false;
			EvaluateFailure(Test, Input, EAssemblyError::PlanQueryFailed, TEXT("PlanQueryResult"));
			break;
		case 10:
			Input.PlanQueryResult.ErrorCode = EThroughBallBehindDefenseP1PlanQueryErrorCode::InvalidDefenseD6;
			EvaluateFailure(Test, Input, EAssemblyError::InvalidPlanQueryResult, TEXT("ErrorCode"));
			break;
		case 11:
			Input.PlanQueryResult.ErrorMessage = TEXT("Unexpected");
			EvaluateFailure(Test, Input, EAssemblyError::InvalidPlanQueryResult, TEXT("ErrorMessage"));
			break;
		case 12:
			Input.PlanQueryResult.InvalidField = TEXT("UnexpectedField");
			EvaluateFailure(Test, Input, EAssemblyError::InvalidPlanQueryResult, TEXT("InvalidField"));
			break;
		case 13:
			Input.PlanQueryResult.Decision = EThroughBallBehindDefenseP1PlanQueryDecision::None;
			EvaluateFailure(Test, Input, EAssemblyError::UnsupportedPlanQueryDecision, TEXT("Decision"));
			break;
		case 14:
			Input.PlanQueryResult.Decision = EThroughBallBehindDefenseP1PlanQueryDecision::OutOfPlay;
			Input.PlanQueryResult.bHasFormulaPlan = false;
			Input.PlanQueryResult.FormulaPlan = FThroughBallBehindDefenseP1FormulaPlan();
			EvaluateFailure(Test, Input, EAssemblyError::UnsupportedPlanQueryDecision, TEXT("Decision"));
			break;
		case 15:
			Input.PlanQueryResult.bHasFormulaPlan = false;
			EvaluateFailure(Test, Input, EAssemblyError::MissingFormulaPlan, TEXT("bHasFormulaPlan"));
			break;
		case 16:
			Input.PlanQueryResult.bAttackEnded = true;
			EvaluateFailure(Test, Input, EAssemblyError::InvalidPlanQueryResult, TEXT("bAttackEnded"));
			break;
		case 17:
			Input.PlanQueryResult.bContinueResolution = true;
			EvaluateFailure(Test, Input, EAssemblyError::InvalidPlanQueryResult, TEXT("bContinueResolution"));
			break;
		case 18:
			Input.PlanQueryResult.bRequiresP2 = true;
			EvaluateFailure(Test, Input, EAssemblyError::InvalidPlanQueryResult, TEXT("bRequiresP2"));
			break;
		case 19:
			Input.PlanQueryResult.FormulaPlan.FormulaType = EFormulaType::Finishing;
			EvaluateFailure(Test, Input, EAssemblyError::UnsupportedFormulaType, TEXT("FormulaType"));
			break;
		case 20:
			Input.PlanQueryResult.FormulaPlan.CarrierId = NAME_None;
			EvaluateFailure(Test, Input, EAssemblyError::InvalidRequiredParticipantIdentity, TEXT("CarrierId"));
			break;
		case 21:
			Input.PlanQueryResult.FormulaPlan.RunnerId = NAME_None;
			EvaluateFailure(Test, Input, EAssemblyError::InvalidRequiredParticipantIdentity, TEXT("RunnerId"));
			break;
		case 22:
			Input.PlanQueryResult.FormulaPlan.MarkerId = NAME_None;
			EvaluateFailure(Test, Input, EAssemblyError::InvalidRequiredParticipantIdentity, TEXT("MarkerId"));
			break;
		case 23:
			Input = MakeInput(true); Input.PlanQueryResult.FormulaPlan.HelperId = NAME_None;
			EvaluateFailure(Test, Input, EAssemblyError::InvalidOptionalParticipantState, TEXT("HelperId"));
			break;
		case 24:
			Input.PlanQueryResult.FormulaPlan.HelperId = HelperId;
			EvaluateFailure(Test, Input, EAssemblyError::InvalidOptionalParticipantState, TEXT("HelperId"));
			break;
		case 25:
			Input.PlanQueryResult.FormulaPlan.HelperSpeed = 1;
			EvaluateFailure(Test, Input, EAssemblyError::InvalidOptionalParticipantState, TEXT("HelperSpeed"));
			break;
		case 26:
			Input.PlanQueryResult.FormulaPlan.HelperStamina = 1;
			EvaluateFailure(Test, Input, EAssemblyError::InvalidOptionalParticipantState, TEXT("HelperStamina"));
			break;
		case 27:
			Input.PlanQueryResult.FormulaPlan.AttackBaseValue =
				std::numeric_limits<float>::quiet_NaN();
			EvaluateFailure(Test, Input, EAssemblyError::InvalidAttackFormulaData, TEXT("AttackBaseValue"));
			break;
		case 28:
			Input.PlanQueryResult.FormulaPlan.AttackBaseValue =
				std::numeric_limits<float>::infinity();
			EvaluateFailure(Test, Input, EAssemblyError::InvalidAttackFormulaData, TEXT("AttackBaseValue"));
			break;
		case 29:
			Input.PlanQueryResult.FormulaPlan.AttackExternalModifier = 0.5f;
			EvaluateFailure(Test, Input, EAssemblyError::InvalidAttackFormulaData, TEXT("AttackExternalModifier"));
			break;
		case 30:
			Input.PlanQueryResult.FormulaPlan.AttackD6 = 2;
			EvaluateFailure(Test, Input, EAssemblyError::InvalidAttackFormulaData, TEXT("AttackD6"));
			break;
		case 31:
			Input.PlanQueryResult.FormulaPlan.AttackD6 = 7;
			EvaluateFailure(Test, Input, EAssemblyError::InvalidAttackFormulaData, TEXT("AttackD6"));
			break;
		case 32:
			Input.PlanQueryResult.FormulaPlan.DefenseBaseValue =
				std::numeric_limits<float>::quiet_NaN();
			EvaluateFailure(Test, Input, EAssemblyError::InvalidDefenseFormulaData, TEXT("DefenseBaseValue"));
			break;
		case 33:
			Input.PlanQueryResult.FormulaPlan.DefenseBaseValue =
				std::numeric_limits<float>::infinity();
			EvaluateFailure(Test, Input, EAssemblyError::InvalidDefenseFormulaData, TEXT("DefenseBaseValue"));
			break;
		case 34:
			Input.PlanQueryResult.FormulaPlan.DefenseExternalModifier = 0.0f;
			EvaluateFailure(Test, Input, EAssemblyError::InvalidDefenseFormulaData, TEXT("DefenseExternalModifier"));
			break;
		case 35:
			Input.PlanQueryResult.FormulaPlan.DefenseD6 = 0;
			EvaluateFailure(Test, Input, EAssemblyError::InvalidDefenseFormulaData, TEXT("DefenseD6"));
			break;
		case 36:
			Input.PlanQueryResult.FormulaPlan.DefenseD6 = 7;
			EvaluateFailure(Test, Input, EAssemblyError::InvalidDefenseFormulaData, TEXT("DefenseD6"));
			break;
		case 37:
			Input.PlanQueryResult.FormulaPlan.AttackParticipatingStamina = {4};
			EvaluateFailure(Test, Input, EAssemblyError::InvalidAttackParticipatingStamina, TEXT("AttackParticipatingStamina"));
			break;
		case 38:
			Input.PlanQueryResult.FormulaPlan.AttackParticipatingStamina = {5, 4};
			EvaluateFailure(Test, Input, EAssemblyError::InvalidAttackParticipatingStamina, TEXT("AttackParticipatingStamina"));
			break;
		case 39:
			Input.PlanQueryResult.FormulaPlan.DefenseParticipatingStamina.Empty();
			EvaluateFailure(Test, Input, EAssemblyError::InvalidDefenseParticipatingStamina, TEXT("DefenseParticipatingStamina"));
			break;
		case 40:
			Input = MakeInput(true); Input.PlanQueryResult.FormulaPlan.DefenseParticipatingStamina = {7, 6};
			EvaluateFailure(Test, Input, EAssemblyError::InvalidDefenseParticipatingStamina, TEXT("DefenseParticipatingStamina"));
			break;
		case 41:
			Input.PlanQueryResult.FormulaPlan.LogId.Invalidate();
			EvaluateFailure(Test, Input, EAssemblyError::InvalidLogContext, TEXT("LogId"));
			break;
		case 42:
			Input.PlanQueryResult.FormulaPlan.TurnIndex = -1;
			EvaluateFailure(Test, Input, EAssemblyError::InvalidLogContext, TEXT("TurnIndex"));
			break;
		case 43:
		{
			FAssemblyInput MissingAttack = MakeInput(); MissingAttack.PlanQueryResult.FormulaPlan.AttackingOwnerId = NAME_None;
			EvaluateFailure(Test, MissingAttack, EAssemblyError::InvalidOwnerIdentity, TEXT("AttackingOwnerId"));
			FAssemblyInput MissingDefense = MakeInput(); MissingDefense.PlanQueryResult.FormulaPlan.DefendingOwnerId = NAME_None;
			EvaluateFailure(Test, MissingDefense, EAssemblyError::InvalidOwnerIdentity, TEXT("DefendingOwnerId"));
			FAssemblyInput SameOwner = MakeInput(); SameOwner.PlanQueryResult.FormulaPlan.DefendingOwnerId = AttackingOwnerId;
			EvaluateFailure(Test, SameOwner, EAssemblyError::InvalidOwnerIdentity, TEXT("DefendingOwnerId"));
			break;
		}
		case 44:
		{
			FAssemblyInput BadCards = MakeInput(); BadCards.PlanQueryResult.FormulaPlan.InvolvedCardIds = {RunnerId, CarrierId, MarkerId};
			EvaluateFailure(Test, BadCards, EAssemblyError::InvalidInvolvedCardIds, TEXT("InvolvedCardIds"));
			FAssemblyInput CrossSideSameId = MakeInput();
			CrossSideSameId.PlanQueryResult.FormulaPlan.MarkerId = CarrierId;
			CrossSideSameId.PlanQueryResult.FormulaPlan.InvolvedCardIds = {CarrierId, RunnerId, CarrierId};
			Result = EvaluateSuccess(Test, CrossSideSameId);
			Test.TestEqual(TEXT("Duplicate CardId preserved"), Result.ResolverInput.InvolvedCardIds.Num(), 3);
			Test.TestEqual(TEXT("Duplicate CardId remains in defense slot"), Result.ResolverInput.InvolvedCardIds[2], CarrierId);
			break;
		}
		case 45:
		{
			FAssemblyInput MissingP2 = MakeInput(); MissingP2.PlanQueryResult.FormulaPlan.bAttackerVictoryRequiresP2 = false;
			EvaluateFailure(Test, MissingP2, EAssemblyError::InvalidWinnerMetadata, TEXT("bAttackerVictoryRequiresP2"));
			FAssemblyInput MissingEnd = MakeInput(); MissingEnd.PlanQueryResult.FormulaPlan.bDefenderVictoryEndsAttack = false;
			EvaluateFailure(Test, MissingEnd, EAssemblyError::InvalidWinnerMetadata, TEXT("bDefenderVictoryEndsAttack"));
			break;
		}
		case 46:
		{
			Input.PlanQueryResult.FormulaPlan.AttackBaseValue = 123.4f;
			Input.PlanQueryResult.FormulaPlan.DefenseBaseValue = -12.5f;
			const FAssemblyInput Before = Input;
			const FAssemblyResult First = EvaluateSuccess(Test, Input);
			const FAssemblyResult Second = EvaluateSuccess(Test, Input);
			Test.TestEqual(TEXT("Finite Attack Base is not recalculated"), First.ResolverInput.Attacker.BaseValue, 123.4f);
			Test.TestEqual(TEXT("Finite Defense Base is not recalculated"), First.ResolverInput.Defender.BaseValue, -12.5f);
			Test.TestEqual(TEXT("Caller Input remains unchanged"), Input.PlanQueryResult.FormulaPlan.AttackBaseValue, Before.PlanQueryResult.FormulaPlan.AttackBaseValue);
			Test.TestTrue(TEXT("Repeated assembly is deterministic"), AreResolverInputsEqual(First.ResolverInput, Second.ResolverInput));
			break;
		}
		default:
			Test.AddError(TEXT("Unknown Behind Defense P1 Resolver Input Assembler test case."));
			break;
		}
		return !Test.HasAnyErrors();
	}
}

#define THROUGH_BALL_BEHIND_DEFENSE_P1_ASSEMBLER_TEST(Number, Name) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
		FThroughBallBehindDefenseP1AssemblerTest##Number, \
		"FMCodex.CoreRules.ThroughBallBehindDefenseP1FormulaResolverInputAssembler." Name, \
		EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter) \
	bool FThroughBallBehindDefenseP1AssemblerTest##Number::RunTest( \
		const FString& Parameters) \
	{ \
		return ThroughBallBehindDefenseP1FormulaResolverInputAssemblerTests \
			::RunCase(*this, Number); \
	}

THROUGH_BALL_BEHIND_DEFENSE_P1_ASSEMBLER_TEST(1, "01DefaultInputAndResultAreFailureSafe")
THROUGH_BALL_BEHIND_DEFENSE_P1_ASSEMBLER_TEST(2, "02SucceedsWithoutHelper")
THROUGH_BALL_BEHIND_DEFENSE_P1_ASSEMBLER_TEST(3, "03SucceedsWithHelper")
THROUGH_BALL_BEHIND_DEFENSE_P1_ASSEMBLER_TEST(4, "04MapsAttackSide")
THROUGH_BALL_BEHIND_DEFENSE_P1_ASSEMBLER_TEST(5, "05MapsDefenseSide")
THROUGH_BALL_BEHIND_DEFENSE_P1_ASSEMBLER_TEST(6, "06MapsContextOwnersAndCards")
THROUGH_BALL_BEHIND_DEFENSE_P1_ASSEMBLER_TEST(7, "07MapsRolledFlagsAndNoGoalkeeper")
THROUGH_BALL_BEHIND_DEFENSE_P1_ASSEMBLER_TEST(8, "08PreservesCompleteInput")
THROUGH_BALL_BEHIND_DEFENSE_P1_ASSEMBLER_TEST(9, "09RejectsPlanQueryFailureFirst")
THROUGH_BALL_BEHIND_DEFENSE_P1_ASSEMBLER_TEST(10, "10RejectsSuccessWithErrorCode")
THROUGH_BALL_BEHIND_DEFENSE_P1_ASSEMBLER_TEST(11, "11RejectsSuccessWithErrorMessage")
THROUGH_BALL_BEHIND_DEFENSE_P1_ASSEMBLER_TEST(12, "12RejectsSuccessWithInvalidField")
THROUGH_BALL_BEHIND_DEFENSE_P1_ASSEMBLER_TEST(13, "13RejectsNoneDecision")
THROUGH_BALL_BEHIND_DEFENSE_P1_ASSEMBLER_TEST(14, "14RejectsOutOfPlayBeforePlanPresence")
THROUGH_BALL_BEHIND_DEFENSE_P1_ASSEMBLER_TEST(15, "15RejectsFormulaDecisionWithoutPlan")
THROUGH_BALL_BEHIND_DEFENSE_P1_ASSEMBLER_TEST(16, "16RejectsPreExecutedAttackEnd")
THROUGH_BALL_BEHIND_DEFENSE_P1_ASSEMBLER_TEST(17, "17RejectsPreExecutedContinuation")
THROUGH_BALL_BEHIND_DEFENSE_P1_ASSEMBLER_TEST(18, "18RejectsPreExecutedP2Requirement")
THROUGH_BALL_BEHIND_DEFENSE_P1_ASSEMBLER_TEST(19, "19RejectsNonTransitionFormula")
THROUGH_BALL_BEHIND_DEFENSE_P1_ASSEMBLER_TEST(20, "20RejectsMissingCarrier")
THROUGH_BALL_BEHIND_DEFENSE_P1_ASSEMBLER_TEST(21, "21RejectsMissingRunner")
THROUGH_BALL_BEHIND_DEFENSE_P1_ASSEMBLER_TEST(22, "22RejectsMissingMarker")
THROUGH_BALL_BEHIND_DEFENSE_P1_ASSEMBLER_TEST(23, "23RejectsPresentHelperWithoutIdentity")
THROUGH_BALL_BEHIND_DEFENSE_P1_ASSEMBLER_TEST(24, "24RejectsAbsentHelperIdentity")
THROUGH_BALL_BEHIND_DEFENSE_P1_ASSEMBLER_TEST(25, "25RejectsAbsentHelperSpeed")
THROUGH_BALL_BEHIND_DEFENSE_P1_ASSEMBLER_TEST(26, "26RejectsAbsentHelperStamina")
THROUGH_BALL_BEHIND_DEFENSE_P1_ASSEMBLER_TEST(27, "27RejectsAttackBaseNaN")
THROUGH_BALL_BEHIND_DEFENSE_P1_ASSEMBLER_TEST(28, "28RejectsAttackBaseInfinity")
THROUGH_BALL_BEHIND_DEFENSE_P1_ASSEMBLER_TEST(29, "29RejectsAttackModifier")
THROUGH_BALL_BEHIND_DEFENSE_P1_ASSEMBLER_TEST(30, "30RejectsAttackD6Two")
THROUGH_BALL_BEHIND_DEFENSE_P1_ASSEMBLER_TEST(31, "31RejectsAttackD6Seven")
THROUGH_BALL_BEHIND_DEFENSE_P1_ASSEMBLER_TEST(32, "32RejectsDefenseBaseNaN")
THROUGH_BALL_BEHIND_DEFENSE_P1_ASSEMBLER_TEST(33, "33RejectsDefenseBaseInfinity")
THROUGH_BALL_BEHIND_DEFENSE_P1_ASSEMBLER_TEST(34, "34RejectsDefenseModifier")
THROUGH_BALL_BEHIND_DEFENSE_P1_ASSEMBLER_TEST(35, "35RejectsDefenseD6Zero")
THROUGH_BALL_BEHIND_DEFENSE_P1_ASSEMBLER_TEST(36, "36RejectsDefenseD6Seven")
THROUGH_BALL_BEHIND_DEFENSE_P1_ASSEMBLER_TEST(37, "37RejectsAttackStaminaCount")
THROUGH_BALL_BEHIND_DEFENSE_P1_ASSEMBLER_TEST(38, "38RejectsAttackStaminaOrderOrValue")
THROUGH_BALL_BEHIND_DEFENSE_P1_ASSEMBLER_TEST(39, "39RejectsDefenseStaminaCount")
THROUGH_BALL_BEHIND_DEFENSE_P1_ASSEMBLER_TEST(40, "40RejectsDefenseStaminaOrderOrValue")
THROUGH_BALL_BEHIND_DEFENSE_P1_ASSEMBLER_TEST(41, "41RejectsInvalidLogId")
THROUGH_BALL_BEHIND_DEFENSE_P1_ASSEMBLER_TEST(42, "42RejectsNegativeTurnIndex")
THROUGH_BALL_BEHIND_DEFENSE_P1_ASSEMBLER_TEST(43, "43RejectsInvalidOwnerStates")
THROUGH_BALL_BEHIND_DEFENSE_P1_ASSEMBLER_TEST(44, "44ValidatesCardsAndPreservesCrossSideDuplicate")
THROUGH_BALL_BEHIND_DEFENSE_P1_ASSEMBLER_TEST(45, "45ValidatesWinnerMetadata")
THROUGH_BALL_BEHIND_DEFENSE_P1_ASSEMBLER_TEST(46, "46DoesNotRecalculateBaseAndIsDeterministic")

#undef THROUGH_BALL_BEHIND_DEFENSE_P1_ASSEMBLER_TEST

#endif
