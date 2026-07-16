#if WITH_DEV_AUTOMATION_TESTS

#include "ThroughBallBehindDefenseP1PlanQuery.h"

#include "Misc/AutomationTest.h"

namespace ThroughBallBehindDefenseP1PlanQueryTests
{
	const FName SkillId(TEXT("Skill.ThroughBall.BehindDefenseP1"));
	const FName CarrierId(TEXT("ThroughBall.Carrier"));
	const FName RunnerId(TEXT("ThroughBall.Runner"));
	const FName MarkerId(TEXT("ThroughBall.Marker"));
	const FName HelperId(TEXT("ThroughBall.Helper"));
	const FName AttackingOwnerId(TEXT("Player.Attacking"));
	const FName DefendingOwnerId(TEXT("Player.Defending"));
	const FGuid LogId(271, 272, 273, 274);

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
		Snapshot.PositionTypes = { Position };
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
		Rules.SkillRules = { Rule };
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
		Input.CarrierSnapshot.SkillIds = { SkillId };
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

	FThroughBallBehindDefenseP1PlanQueryInput MakeInput(
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
		Input.TurnIndex = 8;
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

	bool IsDefaultPlan(const FThroughBallBehindDefenseP1FormulaPlan& Plan)
	{
		return ArePlansEqual(
			Plan,
			FThroughBallBehindDefenseP1FormulaPlan{});
	}

	bool AreP1InputContractFieldsEqual(
		const FThroughBallBehindDefenseP1PlanQueryInput& Left,
		const FThroughBallBehindDefenseP1PlanQueryInput& Right)
	{
		return Left.ParticipantEligibilityResult.bSuccess
				== Right.ParticipantEligibilityResult.bSuccess
			&& Left.ParticipantEligibilityResult.ErrorCode
				== Right.ParticipantEligibilityResult.ErrorCode
			&& Left.ParticipantEligibilityResult.ErrorMessage
				== Right.ParticipantEligibilityResult.ErrorMessage
			&& Left.ParticipantEligibilityResult.InvalidField
				== Right.ParticipantEligibilityResult.InvalidField
			&& Left.ParticipantEligibilityResult.bHasHelper
				== Right.ParticipantEligibilityResult.bHasHelper
			&& Left.ParticipantEligibilityResult.Input.bHasHelper
				== Right.ParticipantEligibilityResult.Input.bHasHelper
			&& Left.ParticipantEligibilityResult.Input.AttackingOwnerId
				== Right.ParticipantEligibilityResult.Input.AttackingOwnerId
			&& Left.ParticipantEligibilityResult.Input.DefendingOwnerId
				== Right.ParticipantEligibilityResult.Input.DefendingOwnerId
			&& Left.ParticipantEligibilityResult.Input.CarrierSnapshot.CardId
				== Right.ParticipantEligibilityResult.Input.CarrierSnapshot.CardId
			&& Left.ParticipantEligibilityResult.Input.RunnerSnapshot.CardId
				== Right.ParticipantEligibilityResult.Input.RunnerSnapshot.CardId
			&& Left.ParticipantEligibilityResult.Input.MarkerSnapshot.CardId
				== Right.ParticipantEligibilityResult.Input.MarkerSnapshot.CardId
			&& Left.ParticipantEligibilityResult.Input.HelperSnapshot.CardId
				== Right.ParticipantEligibilityResult.Input.HelperSnapshot.CardId
			&& Left.SelectedBranch == Right.SelectedBranch
			&& Left.bHasAttackD6 == Right.bHasAttackD6
			&& Left.AttackD6 == Right.AttackD6
			&& Left.bHasDefenseD6 == Right.bHasDefenseD6
			&& Left.DefenseD6 == Right.DefenseD6
			&& Left.LogId == Right.LogId
			&& Left.TurnIndex == Right.TurnIndex;
	}

	FThroughBallBehindDefenseP1PlanQueryResult EvaluateFailure(
		FAutomationTestBase& Test,
		const FThroughBallBehindDefenseP1PlanQueryInput& Input,
		const EThroughBallBehindDefenseP1PlanQueryErrorCode ExpectedError,
		const FName ExpectedField)
	{
		const FThroughBallBehindDefenseP1PlanQueryResult Result =
			FThroughBallBehindDefenseP1PlanQuery::Evaluate(Input);
		Test.TestFalse(TEXT("Query fails"), Result.bSuccess);
		Test.TestEqual(
			TEXT("Failure has no Decision"),
			Result.Decision,
			EThroughBallBehindDefenseP1PlanQueryDecision::None);
		Test.TestEqual(TEXT("Expected Error"), Result.ErrorCode, ExpectedError);
		Test.TestEqual(TEXT("Expected InvalidField"), Result.InvalidField, ExpectedField);
		Test.TestFalse(TEXT("Failure message is non-empty"), Result.ErrorMessage.IsEmpty());
		Test.TestFalse(TEXT("Failure has no Formula Plan"), Result.bHasFormulaPlan);
		Test.TestTrue(TEXT("Failure Plan is default"), IsDefaultPlan(Result.FormulaPlan));
		Test.TestFalse(TEXT("Failure does not end attack"), Result.bAttackEnded);
		Test.TestFalse(TEXT("Failure does not continue"), Result.bContinueResolution);
		Test.TestFalse(TEXT("Failure does not require P2"), Result.bRequiresP2);
		return Result;
	}

	FThroughBallBehindDefenseP1PlanQueryResult EvaluateOutOfPlay(
		FAutomationTestBase& Test,
		const FThroughBallBehindDefenseP1PlanQueryInput& Input)
	{
		const FThroughBallBehindDefenseP1PlanQueryResult Result =
			FThroughBallBehindDefenseP1PlanQuery::Evaluate(Input);
		Test.TestTrue(TEXT("OutOfPlay succeeds"), Result.bSuccess);
		Test.TestEqual(
			TEXT("OutOfPlay Decision"),
			Result.Decision,
			EThroughBallBehindDefenseP1PlanQueryDecision::OutOfPlay);
		Test.TestEqual(
			TEXT("OutOfPlay has no Error"),
			Result.ErrorCode,
			EThroughBallBehindDefenseP1PlanQueryErrorCode::None);
		Test.TestFalse(TEXT("OutOfPlay has no Formula Plan"), Result.bHasFormulaPlan);
		Test.TestTrue(TEXT("OutOfPlay Plan is default"), IsDefaultPlan(Result.FormulaPlan));
		Test.TestTrue(TEXT("OutOfPlay ends attack"), Result.bAttackEnded);
		Test.TestFalse(TEXT("OutOfPlay does not continue"), Result.bContinueResolution);
		Test.TestFalse(TEXT("OutOfPlay does not require P2"), Result.bRequiresP2);
		return Result;
	}

	FThroughBallBehindDefenseP1PlanQueryResult EvaluateFormulaPlan(
		FAutomationTestBase& Test,
		const FThroughBallBehindDefenseP1PlanQueryInput& Input)
	{
		const FThroughBallBehindDefenseP1PlanQueryResult Result =
			FThroughBallBehindDefenseP1PlanQuery::Evaluate(Input);
		Test.TestTrue(TEXT("Formula Plan Query succeeds"), Result.bSuccess);
		Test.TestEqual(
			TEXT("Formula resolution is required"),
			Result.Decision,
			EThroughBallBehindDefenseP1PlanQueryDecision
				::FormulaResolutionRequired);
		Test.TestEqual(
			TEXT("Formula Plan success has no Error"),
			Result.ErrorCode,
			EThroughBallBehindDefenseP1PlanQueryErrorCode::None);
		Test.TestTrue(TEXT("Formula Plan exists"), Result.bHasFormulaPlan);
		Test.TestFalse(TEXT("Plan Query does not end attack"), Result.bAttackEnded);
		Test.TestFalse(TEXT("Plan Query has no final continuation"), Result.bContinueResolution);
		Test.TestFalse(TEXT("Plan Query has no final P2 requirement"), Result.bRequiresP2);
		return Result;
	}

	bool RunCase(FAutomationTestBase& Test, const int32 Case)
	{
		FThroughBallBehindDefenseP1PlanQueryInput Input;
		FThroughBallBehindDefenseP1PlanQueryResult Result;
		switch (Case)
		{
		case 1:
			Result = FThroughBallBehindDefenseP1PlanQueryResult{};
			Test.TestFalse(TEXT("Default Result fails safely"), Result.bSuccess);
			Test.TestEqual(TEXT("Default Decision"), Result.Decision, EThroughBallBehindDefenseP1PlanQueryDecision::None);
			Test.TestTrue(TEXT("Default Plan"), IsDefaultPlan(Result.FormulaPlan));
			break;
		case 2:
			Input = MakeInput(); Input.ParticipantEligibilityResult.bSuccess = false;
			EvaluateFailure(Test, Input, EThroughBallBehindDefenseP1PlanQueryErrorCode::ParticipantEligibilityFailed, TEXT("ParticipantEligibilityResult"));
			break;
		case 3:
			Input = MakeInput(); Input.ParticipantEligibilityResult.ErrorCode = EThroughBallParticipantEligibilityQueryErrorCode::InvalidCarrierIdentity;
			EvaluateFailure(Test, Input, EThroughBallBehindDefenseP1PlanQueryErrorCode::InvalidParticipantEligibilityResult, TEXT("ParticipantEligibilityResult"));
			break;
		case 4:
			Input = MakeInput(); Input.ParticipantEligibilityResult.ErrorMessage = TEXT("stale");
			EvaluateFailure(Test, Input, EThroughBallBehindDefenseP1PlanQueryErrorCode::InvalidParticipantEligibilityResult, TEXT("ParticipantEligibilityResult"));
			break;
		case 5:
			Input = MakeInput(); Input.ParticipantEligibilityResult.InvalidField = TEXT("stale");
			EvaluateFailure(Test, Input, EThroughBallBehindDefenseP1PlanQueryErrorCode::InvalidParticipantEligibilityResult, TEXT("ParticipantEligibilityResult"));
			break;
		case 6:
			Input = MakeInput(); Input.ParticipantEligibilityResult.Input.CarrierSnapshot.CardId = NAME_None;
			EvaluateFailure(Test, Input, EThroughBallBehindDefenseP1PlanQueryErrorCode::InvalidParticipantEligibilityResult, TEXT("ParticipantEligibilityResult"));
			break;
		case 7:
			Input = MakeInput(true); Input.ParticipantEligibilityResult.bHasHelper = false;
			EvaluateFailure(Test, Input, EThroughBallBehindDefenseP1PlanQueryErrorCode::InvalidParticipantEligibilityResult, TEXT("ParticipantEligibilityResult"));
			break;
		case 8:
			Input = MakeInput(); Input.ParticipantEligibilityResult.SkillRuleQueryResult.bSuccess = false;
			EvaluateFailure(Test, Input, EThroughBallBehindDefenseP1PlanQueryErrorCode::InvalidParticipantEligibilityResult, TEXT("ParticipantEligibilityResult"));
			break;
		case 9:
			Input = MakeInput(); Input.ParticipantEligibilityResult.MarkerSnapshotValidationResult.bSuccess = false;
			EvaluateFailure(Test, Input, EThroughBallBehindDefenseP1PlanQueryErrorCode::InvalidParticipantEligibilityResult, TEXT("ParticipantEligibilityResult"));
			break;
		case 10:
			Test.TestTrue(TEXT("Formal Eligibility succeeds"), MakeInput().ParticipantEligibilityResult.bSuccess);
			EvaluateFormulaPlan(Test, MakeInput());
			break;
		case 11:
			Input = MakeInput(); Input.SelectedBranch = EThroughBallSelectedBranch::None;
			EvaluateFailure(Test, Input, EThroughBallBehindDefenseP1PlanQueryErrorCode::UnsupportedBranch, TEXT("SelectedBranch"));
			break;
		case 12:
			Input = MakeInput(); Input.SelectedBranch = EThroughBallSelectedBranch::Feet;
			EvaluateFailure(Test, Input, EThroughBallBehindDefenseP1PlanQueryErrorCode::UnsupportedBranch, TEXT("SelectedBranch"));
			break;
		case 13:
			Input = MakeInput(); Input.SelectedBranch = EThroughBallSelectedBranch::AntiOffside;
			EvaluateFailure(Test, Input, EThroughBallBehindDefenseP1PlanQueryErrorCode::UnsupportedBranch, TEXT("SelectedBranch"));
			break;
		case 14:
			Input = MakeInput(); Input.bHasAttackD6 = false;
			EvaluateFailure(Test, Input, EThroughBallBehindDefenseP1PlanQueryErrorCode::MissingAttackD6, TEXT("AttackD6"));
			break;
		case 15:
			Input = MakeInput(); Input.AttackD6 = 0;
			EvaluateFailure(Test, Input, EThroughBallBehindDefenseP1PlanQueryErrorCode::InvalidAttackD6, TEXT("AttackD6"));
			break;
		case 16:
			Input = MakeInput(); Input.AttackD6 = 7;
			EvaluateFailure(Test, Input, EThroughBallBehindDefenseP1PlanQueryErrorCode::InvalidAttackD6, TEXT("AttackD6"));
			break;
		case 17:
			Input = MakeInput(); Input.AttackD6 = -1;
			EvaluateFailure(Test, Input, EThroughBallBehindDefenseP1PlanQueryErrorCode::InvalidAttackD6, TEXT("AttackD6"));
			break;
		case 18:
			Input = MakeInput(); Input.AttackD6 = 1; Input.bHasDefenseD6 = false; Input.DefenseD6 = 0;
			EvaluateOutOfPlay(Test, Input);
			break;
		case 19:
			Input = MakeInput(); Input.AttackD6 = 2; Input.bHasDefenseD6 = false; Input.DefenseD6 = 0;
			EvaluateOutOfPlay(Test, Input);
			break;
		case 20:
			Input = MakeInput(); Input.AttackD6 = 1; Input.DefenseD6 = 0;
			EvaluateOutOfPlay(Test, Input);
			break;
		case 21:
			Input = MakeInput(); Input.AttackD6 = 1; Input.DefenseD6 = 7;
			EvaluateOutOfPlay(Test, Input);
			break;
		case 22:
			Input = MakeInput(); Input.AttackD6 = 2; Input.DefenseD6 = -8;
			EvaluateOutOfPlay(Test, Input);
			break;
		case 23:
			Input = MakeInput(true); Input.AttackD6 = 1;
			Result = EvaluateOutOfPlay(Test, Input);
			Test.TestTrue(TEXT("Short circuit has no attack stamina"), Result.FormulaPlan.AttackParticipatingStamina.IsEmpty());
			Test.TestTrue(TEXT("Short circuit has no defense stamina"), Result.FormulaPlan.DefenseParticipatingStamina.IsEmpty());
			Test.TestTrue(TEXT("Short circuit has no involved cards"), Result.FormulaPlan.InvolvedCardIds.IsEmpty());
			break;
		case 24:
			Input = MakeInput(); Input.AttackD6 = 1; Input.DefenseD6 = 99;
			Result = EvaluateOutOfPlay(Test, Input);
			Test.TestTrue(TEXT("Ignored DefenseD6 is preserved"), AreP1InputContractFieldsEqual(Result.Input, Input));
			break;
		case 25:
			Input = MakeInput(); Input.AttackD6 = 1; Input.LogId.Invalidate();
			EvaluateFailure(Test, Input, EThroughBallBehindDefenseP1PlanQueryErrorCode::InvalidLogContext, TEXT("LogId"));
			break;
		case 26:
			Input = MakeInput(); Input.AttackD6 = 2; Input.TurnIndex = -1;
			EvaluateFailure(Test, Input, EThroughBallBehindDefenseP1PlanQueryErrorCode::InvalidLogContext, TEXT("TurnIndex"));
			break;
		case 27:
			Input = MakeInput(); Input.AttackD6 = 1; Input.DefenseD6 = 0; Input.LogId.Invalidate();
			EvaluateFailure(Test, Input, EThroughBallBehindDefenseP1PlanQueryErrorCode::InvalidLogContext, TEXT("LogId"));
			break;
		case 28:
			Input = MakeInput(); Input.AttackD6 = 2; Input.TurnIndex = 0;
			EvaluateOutOfPlay(Test, Input);
			break;
		case 29:
			Input = MakeInput(); Input.bHasDefenseD6 = false; Input.DefenseD6 = 0;
			EvaluateFailure(Test, Input, EThroughBallBehindDefenseP1PlanQueryErrorCode::MissingDefenseD6, TEXT("DefenseD6"));
			break;
		case 30:
			Input = MakeInput(); Input.DefenseD6 = 0;
			EvaluateFailure(Test, Input, EThroughBallBehindDefenseP1PlanQueryErrorCode::InvalidDefenseD6, TEXT("DefenseD6"));
			break;
		case 31:
			Input = MakeInput(); Input.DefenseD6 = 7;
			EvaluateFailure(Test, Input, EThroughBallBehindDefenseP1PlanQueryErrorCode::InvalidDefenseD6, TEXT("DefenseD6"));
			break;
		case 32:
			Input = MakeInput(); Input.DefenseD6 = -1;
			EvaluateFailure(Test, Input, EThroughBallBehindDefenseP1PlanQueryErrorCode::InvalidDefenseD6, TEXT("DefenseD6"));
			break;
		case 33:
			Input = MakeInput(); Input.DefenseD6 = 1;
			Test.TestEqual(TEXT("Minimum DefenseD6 retained"), EvaluateFormulaPlan(Test, Input).FormulaPlan.DefenseD6, 1);
			break;
		case 34:
			Input = MakeInput(); Input.AttackD6 = 6; Input.DefenseD6 = 6;
			Result = EvaluateFormulaPlan(Test, Input);
			Test.TestEqual(TEXT("Maximum AttackD6 retained"), Result.FormulaPlan.AttackD6, 6);
			Test.TestEqual(TEXT("Maximum DefenseD6 retained"), Result.FormulaPlan.DefenseD6, 6);
			break;
		case 35:
			Input = MakeInput(); Input.LogId.Invalidate();
			EvaluateFailure(Test, Input, EThroughBallBehindDefenseP1PlanQueryErrorCode::InvalidLogContext, TEXT("LogId"));
			break;
		case 36:
			Input = MakeInput(); Input.TurnIndex = -1;
			EvaluateFailure(Test, Input, EThroughBallBehindDefenseP1PlanQueryErrorCode::InvalidLogContext, TEXT("TurnIndex"));
			break;
		case 37:
			Result = EvaluateFormulaPlan(Test, MakeInput());
			Test.TestEqual(TEXT("P1 uses Transition"), Result.FormulaPlan.FormulaType, EFormulaType::Transition);
			break;
		case 38:
			Result = EvaluateFormulaPlan(Test, MakeInput());
			Test.TestEqual(TEXT("Carrier Passing"), Result.FormulaPlan.CarrierPassing, 5);
			Test.TestEqual(TEXT("Runner Speed"), Result.FormulaPlan.RunnerSpeed, 4);
			Test.TestEqual(TEXT("Attack average preserves half"), Result.FormulaPlan.AttackBaseValue, 4.5f);
			Test.TestEqual(TEXT("Attack modifier is zero"), Result.FormulaPlan.AttackExternalModifier, 0.0f);
			Test.TestEqual(TEXT("Attack stamina ordered"), Result.FormulaPlan.AttackParticipatingStamina, TArray<int32>({6, 5}));
			break;
		case 39:
			Result = EvaluateFormulaPlan(Test, MakeInput());
			Test.TestEqual(TEXT("Absent Helper contributes zero to average"), Result.FormulaPlan.DefenseBaseValue, 2.5f);
			Test.TestEqual(TEXT("Defense modifier is one"), Result.FormulaPlan.DefenseExternalModifier, 1.0f);
			Test.TestEqual(TEXT("Marker-only stamina"), Result.FormulaPlan.DefenseParticipatingStamina, TArray<int32>({4}));
			break;
		case 40:
			Result = EvaluateFormulaPlan(Test, MakeInput(true));
			Test.TestEqual(TEXT("Helper average preserves half"), Result.FormulaPlan.DefenseBaseValue, 4.5f);
			break;
		case 41:
			Result = EvaluateFormulaPlan(Test, MakeInput(true));
			Test.TestTrue(TEXT("Helper present"), Result.FormulaPlan.bHasHelper);
			Test.TestEqual(TEXT("Helper identity"), Result.FormulaPlan.HelperId, HelperId);
			Test.TestEqual(TEXT("Helper Speed"), Result.FormulaPlan.HelperSpeed, 4);
			Test.TestEqual(TEXT("Helper stamina"), Result.FormulaPlan.HelperStamina, 3);
			Test.TestEqual(TEXT("Defense stamina ordered"), Result.FormulaPlan.DefenseParticipatingStamina, TArray<int32>({4, 3}));
			break;
		case 42:
			Input = MakeInput();
			Input.ParticipantEligibilityResult.Input.HelperSnapshot = MakeOutfield(TEXT("Ignored.GKLike"), EPlayerPositionType::Goalkeeper, 99, 99, 99, 99);
			Result = EvaluateFormulaPlan(Test, Input);
			Test.TestFalse(TEXT("Helper absent"), Result.FormulaPlan.bHasHelper);
			Test.TestTrue(TEXT("Absent Helper identity default"), Result.FormulaPlan.HelperId.IsNone());
			Test.TestEqual(TEXT("Absent Helper Speed zero"), Result.FormulaPlan.HelperSpeed, 0);
			Test.TestEqual(TEXT("Absent Helper stamina zero"), Result.FormulaPlan.HelperStamina, 0);
			break;
		case 43:
			Result = EvaluateFormulaPlan(Test, MakeInput());
			Test.TestEqual(TEXT("LogId mapped"), Result.FormulaPlan.LogId, LogId);
			Test.TestEqual(TEXT("TurnIndex mapped"), Result.FormulaPlan.TurnIndex, 8);
			Test.TestEqual(TEXT("Attacking Owner mapped"), Result.FormulaPlan.AttackingOwnerId, AttackingOwnerId);
			Test.TestEqual(TEXT("Defending Owner mapped"), Result.FormulaPlan.DefendingOwnerId, DefendingOwnerId);
			break;
		case 44:
			Result = EvaluateFormulaPlan(Test, MakeInput(true));
			Test.TestEqual(TEXT("Involved cards preserve role order"), Result.FormulaPlan.InvolvedCardIds, TArray<FName>({CarrierId, RunnerId, MarkerId, HelperId}));
			break;
		case 45:
			Input = MakeInput(false, true);
			Test.TestTrue(TEXT("Cross-side duplicate formal Eligibility succeeds"), Input.ParticipantEligibilityResult.bSuccess);
			Result = EvaluateFormulaPlan(Test, Input);
			Test.TestEqual(TEXT("Cross-side duplicate CardId is retained"), Result.FormulaPlan.InvolvedCardIds, TArray<FName>({CarrierId, RunnerId, CarrierId}));
			break;
		case 46:
			Result = EvaluateFormulaPlan(Test, MakeInput());
			Test.TestTrue(TEXT("Attacker winner requires P2 metadata"), Result.FormulaPlan.bAttackerVictoryRequiresP2);
			Test.TestTrue(TEXT("Defender winner ends attack metadata"), Result.FormulaPlan.bDefenderVictoryEndsAttack);
			break;
		case 47:
			Input = MakeInput(); Input.AttackD6 = 5;
			Test.TestEqual(TEXT("AttackD6 retained"), EvaluateFormulaPlan(Test, Input).FormulaPlan.AttackD6, 5);
			break;
		case 48:
			Input = MakeInput(); Input.DefenseD6 = 2;
			Test.TestEqual(TEXT("DefenseD6 retained"), EvaluateFormulaPlan(Test, Input).FormulaPlan.DefenseD6, 2);
			break;
		case 49:
			Input = MakeInput(); Input.bHasDefenseD6 = false;
			Result = EvaluateFailure(Test, Input, EThroughBallBehindDefenseP1PlanQueryErrorCode::MissingDefenseD6, TEXT("DefenseD6"));
			Test.TestTrue(TEXT("Failure preserves Input"), AreP1InputContractFieldsEqual(Result.Input, Input));
			break;
		case 50:
			Input = MakeInput(); Input.ParticipantEligibilityResult.bSuccess = false; Input.SelectedBranch = EThroughBallSelectedBranch::None;
			EvaluateFailure(Test, Input, EThroughBallBehindDefenseP1PlanQueryErrorCode::ParticipantEligibilityFailed, TEXT("ParticipantEligibilityResult"));
			break;
		case 51:
			Input = MakeInput(); Input.SelectedBranch = EThroughBallSelectedBranch::Feet; Input.bHasAttackD6 = false;
			EvaluateFailure(Test, Input, EThroughBallBehindDefenseP1PlanQueryErrorCode::UnsupportedBranch, TEXT("SelectedBranch"));
			break;
		case 52:
			Input = MakeInput(); Input.bHasAttackD6 = false; Input.LogId.Invalidate();
			EvaluateFailure(Test, Input, EThroughBallBehindDefenseP1PlanQueryErrorCode::MissingAttackD6, TEXT("AttackD6"));
			break;
		case 53:
			Input = MakeInput(); Input.AttackD6 = 0; Input.LogId.Invalidate();
			EvaluateFailure(Test, Input, EThroughBallBehindDefenseP1PlanQueryErrorCode::InvalidAttackD6, TEXT("AttackD6"));
			break;
		case 54:
			Input = MakeInput(); Input.LogId.Invalidate(); Input.bHasDefenseD6 = false;
			EvaluateFailure(Test, Input, EThroughBallBehindDefenseP1PlanQueryErrorCode::InvalidLogContext, TEXT("LogId"));
			break;
		case 55:
		{
			Input = MakeInput(true);
			const FThroughBallBehindDefenseP1PlanQueryInput Before = Input;
			const FThroughBallBehindDefenseP1PlanQueryResult First = EvaluateFormulaPlan(Test, Input);
			const FThroughBallBehindDefenseP1PlanQueryResult Second = EvaluateFormulaPlan(Test, Input);
			Test.TestTrue(TEXT("Caller Input contract fields unchanged"), AreP1InputContractFieldsEqual(Input, Before));
			Test.TestTrue(TEXT("Result copies Input contract fields"), AreP1InputContractFieldsEqual(First.Input, Before));
			Test.TestTrue(TEXT("Same Input produces same Plan"), ArePlansEqual(First.FormulaPlan, Second.FormulaPlan));
			break;
		}
		default:
			Test.AddError(TEXT("Unknown Behind Defense P1 Plan Query test case."));
			break;
		}
		return !Test.HasAnyErrors();
	}
}

#define THROUGH_BALL_BEHIND_DEFENSE_P1_PLAN_TEST(Number, Name) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
		FThroughBallBehindDefenseP1PlanTest##Number, \
		"FMCodex.CoreRules.ThroughBallBehindDefenseP1PlanQuery." Name, \
		EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter) \
	bool FThroughBallBehindDefenseP1PlanTest##Number::RunTest( \
		const FString& Parameters) \
	{ \
		return ThroughBallBehindDefenseP1PlanQueryTests::RunCase(*this, Number); \
	}

THROUGH_BALL_BEHIND_DEFENSE_P1_PLAN_TEST(1, "01DefaultResultIsFailureSafe")
THROUGH_BALL_BEHIND_DEFENSE_P1_PLAN_TEST(2, "02RejectsFailedEligibility")
THROUGH_BALL_BEHIND_DEFENSE_P1_PLAN_TEST(3, "03RejectsEligibilityErrorCodeInconsistency")
THROUGH_BALL_BEHIND_DEFENSE_P1_PLAN_TEST(4, "04RejectsEligibilityMessageInconsistency")
THROUGH_BALL_BEHIND_DEFENSE_P1_PLAN_TEST(5, "05RejectsEligibilityInvalidFieldInconsistency")
THROUGH_BALL_BEHIND_DEFENSE_P1_PLAN_TEST(6, "06RejectsMissingRequiredParticipantInSuccess")
THROUGH_BALL_BEHIND_DEFENSE_P1_PLAN_TEST(7, "07RejectsHelperMirrorMismatch")
THROUGH_BALL_BEHIND_DEFENSE_P1_PLAN_TEST(8, "08RejectsNestedSkillQueryFailure")
THROUGH_BALL_BEHIND_DEFENSE_P1_PLAN_TEST(9, "09RejectsNestedParticipantValidationFailure")
THROUGH_BALL_BEHIND_DEFENSE_P1_PLAN_TEST(10, "10ConsumesFormalSuccessfulEligibility")
THROUGH_BALL_BEHIND_DEFENSE_P1_PLAN_TEST(11, "11RejectsNoneBranch")
THROUGH_BALL_BEHIND_DEFENSE_P1_PLAN_TEST(12, "12RejectsFeetBranch")
THROUGH_BALL_BEHIND_DEFENSE_P1_PLAN_TEST(13, "13RejectsAntiOffsideBranch")
THROUGH_BALL_BEHIND_DEFENSE_P1_PLAN_TEST(14, "14RejectsMissingAttackD6")
THROUGH_BALL_BEHIND_DEFENSE_P1_PLAN_TEST(15, "15RejectsAttackD6Zero")
THROUGH_BALL_BEHIND_DEFENSE_P1_PLAN_TEST(16, "16RejectsAttackD6Seven")
THROUGH_BALL_BEHIND_DEFENSE_P1_PLAN_TEST(17, "17RejectsNegativeAttackD6")
THROUGH_BALL_BEHIND_DEFENSE_P1_PLAN_TEST(18, "18AttackD6OneNeedsNoDefenseD6")
THROUGH_BALL_BEHIND_DEFENSE_P1_PLAN_TEST(19, "19AttackD6TwoNeedsNoDefenseD6")
THROUGH_BALL_BEHIND_DEFENSE_P1_PLAN_TEST(20, "20ShortCircuitIgnoresDefenseD6Zero")
THROUGH_BALL_BEHIND_DEFENSE_P1_PLAN_TEST(21, "21ShortCircuitIgnoresDefenseD6Seven")
THROUGH_BALL_BEHIND_DEFENSE_P1_PLAN_TEST(22, "22ShortCircuitIgnoresNegativeDefenseD6")
THROUGH_BALL_BEHIND_DEFENSE_P1_PLAN_TEST(23, "23OutOfPlayHasNoPlanCollections")
THROUGH_BALL_BEHIND_DEFENSE_P1_PLAN_TEST(24, "24OutOfPlayPreservesIgnoredDefenseD6")
THROUGH_BALL_BEHIND_DEFENSE_P1_PLAN_TEST(25, "25ShortCircuitStillRequiresLogId")
THROUGH_BALL_BEHIND_DEFENSE_P1_PLAN_TEST(26, "26ShortCircuitStillRequiresTurnIndex")
THROUGH_BALL_BEHIND_DEFENSE_P1_PLAN_TEST(27, "27LogErrorPrecedesShortCircuit")
THROUGH_BALL_BEHIND_DEFENSE_P1_PLAN_TEST(28, "28TurnIndexZeroIsValid")
THROUGH_BALL_BEHIND_DEFENSE_P1_PLAN_TEST(29, "29FormulaPathRequiresDefenseD6")
THROUGH_BALL_BEHIND_DEFENSE_P1_PLAN_TEST(30, "30RejectsDefenseD6Zero")
THROUGH_BALL_BEHIND_DEFENSE_P1_PLAN_TEST(31, "31RejectsDefenseD6Seven")
THROUGH_BALL_BEHIND_DEFENSE_P1_PLAN_TEST(32, "32RejectsNegativeDefenseD6")
THROUGH_BALL_BEHIND_DEFENSE_P1_PLAN_TEST(33, "33AcceptsMinimumDefenseD6")
THROUGH_BALL_BEHIND_DEFENSE_P1_PLAN_TEST(34, "34AcceptsMaximumFormulaD6Values")
THROUGH_BALL_BEHIND_DEFENSE_P1_PLAN_TEST(35, "35FormulaPathRequiresLogId")
THROUGH_BALL_BEHIND_DEFENSE_P1_PLAN_TEST(36, "36FormulaPathRequiresTurnIndex")
THROUGH_BALL_BEHIND_DEFENSE_P1_PLAN_TEST(37, "37UsesTransitionFormula")
THROUGH_BALL_BEHIND_DEFENSE_P1_PLAN_TEST(38, "38BuildsAttackFormulaPlan")
THROUGH_BALL_BEHIND_DEFENSE_P1_PLAN_TEST(39, "39BuildsDefensePlanWithoutHelper")
THROUGH_BALL_BEHIND_DEFENSE_P1_PLAN_TEST(40, "40CalculatesHelperDefenseAverage")
THROUGH_BALL_BEHIND_DEFENSE_P1_PLAN_TEST(41, "41MapsPresentHelper")
THROUGH_BALL_BEHIND_DEFENSE_P1_PLAN_TEST(42, "42AbsentHelperIgnoresGarbageSnapshot")
THROUGH_BALL_BEHIND_DEFENSE_P1_PLAN_TEST(43, "43MapsLogAndOwners")
THROUGH_BALL_BEHIND_DEFENSE_P1_PLAN_TEST(44, "44OrdersInvolvedCardIds")
THROUGH_BALL_BEHIND_DEFENSE_P1_PLAN_TEST(45, "45RetainsCrossSideDuplicateCardId")
THROUGH_BALL_BEHIND_DEFENSE_P1_PLAN_TEST(46, "46MapsFutureWinnerSemantics")
THROUGH_BALL_BEHIND_DEFENSE_P1_PLAN_TEST(47, "47RetainsAttackD6")
THROUGH_BALL_BEHIND_DEFENSE_P1_PLAN_TEST(48, "48RetainsDefenseD6")
THROUGH_BALL_BEHIND_DEFENSE_P1_PLAN_TEST(49, "49FailurePreservesInputAndDefaults")
THROUGH_BALL_BEHIND_DEFENSE_P1_PLAN_TEST(50, "50EligibilityFailurePrecedesBranch")
THROUGH_BALL_BEHIND_DEFENSE_P1_PLAN_TEST(51, "51BranchPrecedesAttackPresence")
THROUGH_BALL_BEHIND_DEFENSE_P1_PLAN_TEST(52, "52AttackPresencePrecedesLog")
THROUGH_BALL_BEHIND_DEFENSE_P1_PLAN_TEST(53, "53AttackRangePrecedesLog")
THROUGH_BALL_BEHIND_DEFENSE_P1_PLAN_TEST(54, "54LogPrecedesDefensePresence")
THROUGH_BALL_BEHIND_DEFENSE_P1_PLAN_TEST(55, "55InputIsImmutableAndResultDeterministic")

#undef THROUGH_BALL_BEHIND_DEFENSE_P1_PLAN_TEST

#endif
