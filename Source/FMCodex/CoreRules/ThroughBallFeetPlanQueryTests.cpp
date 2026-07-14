#if WITH_DEV_AUTOMATION_TESTS

#include "ThroughBallFeetPlanQuery.h"

#include "Misc/AutomationTest.h"

namespace ThroughBallFeetPlanQueryTests
{
	const FName CarrierId(TEXT("ThroughBall.Carrier"));
	const FName RunnerId(TEXT("ThroughBall.Runner"));
	const FName MarkerId(TEXT("ThroughBall.Marker"));
	const FName HelperId(TEXT("ThroughBall.Helper"));
	const FName GoalkeeperId(TEXT("ThroughBall.Goalkeeper"));
	const FName AttackingOwnerId(TEXT("Player.Attacking"));
	const FName DefendingOwnerId(TEXT("Player.Defending"));
	const FGuid LogId(171, 172, 173, 174);

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
		Snapshot.PositionTypes = { Position };
		Snapshot.Attributes.Passing = Passing;
		Snapshot.Attributes.OffBall = OffBall;
		Snapshot.Attributes.Marking = Marking;
		Snapshot.Attributes.Tackling = Tackling;
		Snapshot.Attributes.Stamina = Stamina;
		return Snapshot;
	}

	FPlayerCardRuleSnapshot MakeGoalkeeper(
		const FName CardId = GoalkeeperId,
		const int32 OneOnOne = 5,
		const int32 Stamina = 6)
	{
		FPlayerCardRuleSnapshot Snapshot;
		Snapshot.CardId = CardId;
		Snapshot.PositionTypes = { EPlayerPositionType::Goalkeeper };
		Snapshot.Attributes.Stamina = Stamina;
		Snapshot.bIsGoalkeeper = true;
		Snapshot.bHasGoalkeeperAttributes = true;
		Snapshot.GoalkeeperAttributes.OneOnOne = OneOnOne;
		return Snapshot;
	}

	FThroughBallParticipantEligibilityQueryResult MakeEligibility(
		const bool bHasHelper = false)
	{
		FThroughBallParticipantEligibilityQueryResult Result;
		Result.bSuccess = true;
		Result.Input.AttackingOwnerId = AttackingOwnerId;
		Result.Input.DefendingOwnerId = DefendingOwnerId;
		Result.Input.CarrierSnapshot = MakeOutfield(
			CarrierId, EPlayerPositionType::Midfield, 4, 2, 1, 1, 5);
		Result.Input.RunnerSnapshot = MakeOutfield(
			RunnerId, EPlayerPositionType::Attack, 1, 5, 1, 1, 4);
		Result.Input.MarkerSnapshot = MakeOutfield(
			MarkerId, EPlayerPositionType::Defense, 1, 1, 2, 5, 3);
		Result.Input.bHasHelper = bHasHelper;
		if (bHasHelper)
		{
			Result.Input.HelperSnapshot = MakeOutfield(
				HelperId, EPlayerPositionType::Midfield, 1, 1, 4, 1, 2);
		}
		Result.bHasHelper = bHasHelper;
		Result.SkillRuleQueryResult.bSuccess = true;
		Result.CarrierSnapshotValidationResult.bSuccess = true;
		Result.RunnerSnapshotValidationResult.bSuccess = true;
		Result.MarkerSnapshotValidationResult.bSuccess = true;
		Result.HelperSnapshotValidationResult.bSuccess = bHasHelper;
		return Result;
	}

	FThroughBallFeetPlanQueryInput MakeInput(
		const bool bHasHelper = false,
		const bool bHasGoalkeeper = false)
	{
		FThroughBallFeetPlanQueryInput Input;
		Input.ParticipantEligibilityResult = MakeEligibility(bHasHelper);
		Input.AttackD6 = 3;
		Input.DefenseD6 = 4;
		Input.bHasActiveGoalkeeper = bHasGoalkeeper;
		if (bHasGoalkeeper)
		{
			Input.ActiveGoalkeeperSnapshot = MakeGoalkeeper();
		}
		Input.LogId = LogId;
		Input.TurnIndex = 7;
		return Input;
	}

	bool AreSnapshotsEqual(
		const FPlayerCardRuleSnapshot& Left,
		const FPlayerCardRuleSnapshot& Right)
	{
		return Left.CardId == Right.CardId
			&& Left.PositionTypes == Right.PositionTypes
			&& Left.Attributes.Shooting == Right.Attributes.Shooting
			&& Left.Attributes.Dribbling == Right.Attributes.Dribbling
			&& Left.Attributes.Passing == Right.Attributes.Passing
			&& Left.Attributes.OffBall == Right.Attributes.OffBall
			&& Left.Attributes.Marking == Right.Attributes.Marking
			&& Left.Attributes.Tackling == Right.Attributes.Tackling
			&& Left.Attributes.Speed == Right.Attributes.Speed
			&& Left.Attributes.Strength == Right.Attributes.Strength
			&& Left.Attributes.Stamina == Right.Attributes.Stamina
			&& Left.Attributes.LongShot == Right.Attributes.LongShot
			&& Left.bIsGoalkeeper == Right.bIsGoalkeeper
			&& Left.bHasGoalkeeperAttributes == Right.bHasGoalkeeperAttributes
			&& Left.GoalkeeperAttributes.Handling
				== Right.GoalkeeperAttributes.Handling
			&& Left.GoalkeeperAttributes.Positioning
				== Right.GoalkeeperAttributes.Positioning
			&& Left.GoalkeeperAttributes.Reflex
				== Right.GoalkeeperAttributes.Reflex
			&& Left.GoalkeeperAttributes.Aerial
				== Right.GoalkeeperAttributes.Aerial
			&& Left.GoalkeeperAttributes.Anticipation
				== Right.GoalkeeperAttributes.Anticipation
			&& Left.GoalkeeperAttributes.OneOnOne
				== Right.GoalkeeperAttributes.OneOnOne
			&& Left.Rarity == Right.Rarity
			&& Left.SkillIds == Right.SkillIds;
	}

	bool AreEligibilityResultsEqual(
		const FThroughBallParticipantEligibilityQueryResult& Left,
		const FThroughBallParticipantEligibilityQueryResult& Right)
	{
		return Left.bSuccess == Right.bSuccess
			&& Left.ErrorCode == Right.ErrorCode
			&& Left.ErrorMessage == Right.ErrorMessage
			&& Left.InvalidField == Right.InvalidField
			&& Left.bHasHelper == Right.bHasHelper
			&& Left.Input.SelectedSkillId == Right.Input.SelectedSkillId
			&& Left.Input.CurrentActionPoint == Right.Input.CurrentActionPoint
			&& Left.Input.AttackingOwnerId == Right.Input.AttackingOwnerId
			&& Left.Input.DefendingOwnerId == Right.Input.DefendingOwnerId
			&& Left.Input.bHasHelper == Right.Input.bHasHelper
			&& Left.Input.bIsRunnerInAttackingForwardArea
				== Right.Input.bIsRunnerInAttackingForwardArea
			&& AreSnapshotsEqual(
				Left.Input.CarrierSnapshot, Right.Input.CarrierSnapshot)
			&& AreSnapshotsEqual(
				Left.Input.RunnerSnapshot, Right.Input.RunnerSnapshot)
			&& AreSnapshotsEqual(
				Left.Input.MarkerSnapshot, Right.Input.MarkerSnapshot)
			&& AreSnapshotsEqual(
				Left.Input.HelperSnapshot, Right.Input.HelperSnapshot)
			&& Left.SkillRuleQueryResult.bSuccess
				== Right.SkillRuleQueryResult.bSuccess
			&& Left.SkillRuleQueryResult.ErrorCode
				== Right.SkillRuleQueryResult.ErrorCode
			&& Left.CarrierSnapshotValidationResult.bSuccess
				== Right.CarrierSnapshotValidationResult.bSuccess
			&& Left.RunnerSnapshotValidationResult.bSuccess
				== Right.RunnerSnapshotValidationResult.bSuccess
			&& Left.MarkerSnapshotValidationResult.bSuccess
				== Right.MarkerSnapshotValidationResult.bSuccess
			&& Left.HelperSnapshotValidationResult.bSuccess
				== Right.HelperSnapshotValidationResult.bSuccess;
	}

	bool AreInputsEqual(
		const FThroughBallFeetPlanQueryInput& Left,
		const FThroughBallFeetPlanQueryInput& Right)
	{
		return AreEligibilityResultsEqual(
				Left.ParticipantEligibilityResult,
				Right.ParticipantEligibilityResult)
			&& Left.AttackD6 == Right.AttackD6
			&& Left.DefenseD6 == Right.DefenseD6
			&& Left.bHasActiveGoalkeeper == Right.bHasActiveGoalkeeper
			&& AreSnapshotsEqual(
				Left.ActiveGoalkeeperSnapshot,
				Right.ActiveGoalkeeperSnapshot)
			&& Left.LogId == Right.LogId
			&& Left.TurnIndex == Right.TurnIndex;
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

	bool IsDefaultPlan(const FThroughBallFeetFormulaPlan& Plan)
	{
		return ArePlansEqual(Plan, FThroughBallFeetFormulaPlan{});
	}

	FThroughBallFeetPlanQueryResult EvaluateSuccess(
		FAutomationTestBase& Test,
		const FThroughBallFeetPlanQueryInput& Input)
	{
		const FThroughBallFeetPlanQueryResult Result =
			FThroughBallFeetPlanQuery::Evaluate(Input);
		Test.TestTrue(TEXT("Query succeeds"), Result.bSuccess);
		Test.TestTrue(TEXT("Success has Formula Plan"), Result.bHasFormulaPlan);
		Test.TestEqual(
			TEXT("Success requires formula resolution"),
			Result.Decision,
			EThroughBallFeetPlanQueryDecision::FormulaResolutionRequired);
		Test.TestEqual(
			TEXT("Success has no error"),
			Result.ErrorCode,
			EThroughBallFeetPlanQueryErrorCode::None);
		Test.TestTrue(TEXT("Success message is empty"), Result.ErrorMessage.IsEmpty());
		Test.TestTrue(TEXT("Success invalid field is empty"), Result.InvalidField.IsNone());
		return Result;
	}

	FThroughBallFeetPlanQueryResult EvaluateFailure(
		FAutomationTestBase& Test,
		const FThroughBallFeetPlanQueryInput& Input,
		const EThroughBallFeetPlanQueryErrorCode ExpectedError,
		const FName ExpectedField)
	{
		const FThroughBallFeetPlanQueryResult Result =
			FThroughBallFeetPlanQuery::Evaluate(Input);
		Test.TestFalse(TEXT("Query fails"), Result.bSuccess);
		Test.TestFalse(TEXT("Failure has no Formula Plan"), Result.bHasFormulaPlan);
		Test.TestEqual(
			TEXT("Failure has no decision"),
			Result.Decision,
			EThroughBallFeetPlanQueryDecision::None);
		Test.TestEqual(TEXT("Expected error"), Result.ErrorCode, ExpectedError);
		Test.TestEqual(TEXT("Expected invalid field"), Result.InvalidField, ExpectedField);
		Test.TestFalse(TEXT("Failure message is non-empty"), Result.ErrorMessage.IsEmpty());
		Test.TestTrue(TEXT("Failure Plan is default"), IsDefaultPlan(Result.FormulaPlan));
		return Result;
	}

	bool RunCase(FAutomationTestBase& Test, const int32 Case)
	{
		FThroughBallFeetPlanQueryInput Input;
		FThroughBallFeetPlanQueryResult Result;
		switch (Case)
		{
		case 1:
			EvaluateSuccess(Test, MakeInput(false, false));
			break;
		case 2:
			EvaluateSuccess(Test, MakeInput(true, false));
			break;
		case 3:
			EvaluateSuccess(Test, MakeInput(false, true));
			break;
		case 4:
			EvaluateSuccess(Test, MakeInput(true, true));
			break;
		case 5:
			Input = MakeInput(); Input.AttackD6 = 1;
			Test.TestEqual(TEXT("Minimum AttackD6 retained"),
				EvaluateSuccess(Test, Input).FormulaPlan.AttackD6, 1);
			break;
		case 6:
			Input = MakeInput(); Input.AttackD6 = 6;
			Test.TestEqual(TEXT("Maximum AttackD6 retained"),
				EvaluateSuccess(Test, Input).FormulaPlan.AttackD6, 6);
			break;
		case 7:
			Input = MakeInput(); Input.DefenseD6 = 1;
			Test.TestEqual(TEXT("Minimum DefenseD6 retained"),
				EvaluateSuccess(Test, Input).FormulaPlan.DefenseD6, 1);
			break;
		case 8:
			Input = MakeInput(); Input.DefenseD6 = 6;
			Test.TestEqual(TEXT("Maximum DefenseD6 retained"),
				EvaluateSuccess(Test, Input).FormulaPlan.DefenseD6, 6);
			break;
		case 9:
			Input = MakeInput(true);
			Input.ParticipantEligibilityResult.Input.MarkerSnapshot.CardId = CarrierId;
			Input.ParticipantEligibilityResult.Input.HelperSnapshot.CardId = RunnerId;
			EvaluateSuccess(Test, Input);
			break;
		case 10:
			Input = MakeInput(false, true);
			Input.ActiveGoalkeeperSnapshot.CardId = CarrierId;
			EvaluateSuccess(Test, Input);
			break;
		case 11:
			Input = MakeInput(); Input.ParticipantEligibilityResult.bSuccess = false;
			EvaluateFailure(Test, Input,
				EThroughBallFeetPlanQueryErrorCode::ParticipantEligibilityFailed,
				TEXT("ParticipantEligibilityResult"));
			break;
		case 12:
		{
			TArray<FThroughBallFeetPlanQueryInput> InvalidInputs;
			Input = MakeInput();
			Input.ParticipantEligibilityResult.ErrorCode =
				EThroughBallParticipantEligibilityQueryErrorCode::InvalidCarrierIdentity;
			InvalidInputs.Add(Input);
			Input = MakeInput(); Input.ParticipantEligibilityResult.ErrorMessage = TEXT("stale"); InvalidInputs.Add(Input);
			Input = MakeInput(); Input.ParticipantEligibilityResult.InvalidField = TEXT("stale"); InvalidInputs.Add(Input);
			Input = MakeInput(); Input.ParticipantEligibilityResult.SkillRuleQueryResult.bSuccess = false; InvalidInputs.Add(Input);
			Input = MakeInput(); Input.ParticipantEligibilityResult.CarrierSnapshotValidationResult.bSuccess = false; InvalidInputs.Add(Input);
			Input = MakeInput(); Input.ParticipantEligibilityResult.RunnerSnapshotValidationResult.bSuccess = false; InvalidInputs.Add(Input);
			Input = MakeInput(); Input.ParticipantEligibilityResult.MarkerSnapshotValidationResult.bSuccess = false; InvalidInputs.Add(Input);
			Input = MakeInput(true); Input.ParticipantEligibilityResult.HelperSnapshotValidationResult.bSuccess = false; InvalidInputs.Add(Input);
			for (const FThroughBallFeetPlanQueryInput& InvalidInput : InvalidInputs)
			{
				EvaluateFailure(Test, InvalidInput,
					EThroughBallFeetPlanQueryErrorCode::InvalidParticipantEligibilityResult,
					TEXT("ParticipantEligibilityResult"));
			}
			break;
		}
		case 13:
			Input = MakeInput(true); Input.ParticipantEligibilityResult.bHasHelper = false;
			EvaluateFailure(Test, Input,
				EThroughBallFeetPlanQueryErrorCode::InvalidParticipantEligibilityResult,
				TEXT("ParticipantEligibilityResult"));
			break;
		case 14:
			Input = MakeInput(); Input.ParticipantEligibilityResult.bSuccess = false; Input.AttackD6 = 0;
			EvaluateFailure(Test, Input,
				EThroughBallFeetPlanQueryErrorCode::ParticipantEligibilityFailed,
				TEXT("ParticipantEligibilityResult"));
			break;
		case 15:
			Input = MakeInput(); Input.ParticipantEligibilityResult.bSuccess = false; Input.bHasActiveGoalkeeper = true;
			EvaluateFailure(Test, Input,
				EThroughBallFeetPlanQueryErrorCode::ParticipantEligibilityFailed,
				TEXT("ParticipantEligibilityResult"));
			break;
		case 16:
			Input = MakeInput(); Input.AttackD6 = 0;
			EvaluateFailure(Test, Input, EThroughBallFeetPlanQueryErrorCode::InvalidAttackD6, TEXT("AttackD6"));
			break;
		case 17:
			Input = MakeInput(); Input.AttackD6 = 7;
			EvaluateFailure(Test, Input, EThroughBallFeetPlanQueryErrorCode::InvalidAttackD6, TEXT("AttackD6"));
			break;
		case 18:
			Input = MakeInput(); Input.DefenseD6 = 0;
			EvaluateFailure(Test, Input, EThroughBallFeetPlanQueryErrorCode::InvalidDefenseD6, TEXT("DefenseD6"));
			break;
		case 19:
			Input = MakeInput(); Input.DefenseD6 = 7;
			EvaluateFailure(Test, Input, EThroughBallFeetPlanQueryErrorCode::InvalidDefenseD6, TEXT("DefenseD6"));
			break;
		case 20:
			Input = MakeInput(); Input.AttackD6 = 0; Input.DefenseD6 = 0;
			EvaluateFailure(Test, Input, EThroughBallFeetPlanQueryErrorCode::InvalidAttackD6, TEXT("AttackD6"));
			break;
		case 21:
			Input = MakeInput(); Input.AttackD6 = 2; Input.DefenseD6 = 5;
			Result = EvaluateSuccess(Test, Input);
			Test.TestEqual(TEXT("Feet attack roll is explicit"), Result.FormulaPlan.AttackD6, 2);
			Test.TestEqual(TEXT("Feet defense roll is explicit"), Result.FormulaPlan.DefenseD6, 5);
			break;
		case 22:
			Input = MakeInput(); Input.ActiveGoalkeeperSnapshot.CardId = NAME_None;
			EvaluateSuccess(Test, Input);
			break;
		case 23:
			Input = MakeInput(); Input.ActiveGoalkeeperSnapshot = MakeGoalkeeper(); Input.ActiveGoalkeeperSnapshot.Attributes.Stamina = 99;
			EvaluateSuccess(Test, Input);
			break;
		case 24:
			Input = MakeInput(); Input.ActiveGoalkeeperSnapshot = MakeOutfield(TEXT("Ignored"), EPlayerPositionType::Defense, 1, 1, 1, 1, 1);
			EvaluateSuccess(Test, Input);
			break;
		case 25:
			Input = MakeInput(); Input.ActiveGoalkeeperSnapshot = MakeGoalkeeper(MarkerId);
			EvaluateSuccess(Test, Input);
			break;
		case 26:
			Result = EvaluateSuccess(Test, MakeInput());
			Test.TestFalse(TEXT("Absent GK validator is skipped"), Result.ActiveGoalkeeperValidationResult.bSuccess);
			Test.TestEqual(TEXT("Absent GK validator error is default"), Result.ActiveGoalkeeperValidationResult.ErrorCode, EPlayerCardRuleSnapshotValidationErrorCode::None);
			break;
		case 27:
			Result = EvaluateSuccess(Test, MakeInput());
			Test.TestFalse(TEXT("GK absent in Plan"), Result.FormulaPlan.bHasActiveGoalkeeper);
			Test.TestTrue(TEXT("GK identity default"), Result.FormulaPlan.ActiveGoalkeeperId.IsNone());
			Test.TestEqual(TEXT("GK value zero"), Result.FormulaPlan.GoalkeeperOneOnOne, 0);
			Test.TestEqual(TEXT("Defense has Marker only"), Result.FormulaPlan.DefenseParticipatingStamina, TArray<int32>{3});
			break;
		case 28:
			Input = MakeInput(); Input.bHasActiveGoalkeeper = true;
			EvaluateFailure(Test, Input, EThroughBallFeetPlanQueryErrorCode::InvalidActiveGoalkeeperIdentity, TEXT("ActiveGoalkeeperSnapshot"));
			break;
		case 29:
			Input = MakeInput(false, true); Input.ActiveGoalkeeperSnapshot.Attributes.Stamina = 0;
			Result = EvaluateFailure(Test, Input, EThroughBallFeetPlanQueryErrorCode::InvalidActiveGoalkeeperSnapshot, TEXT("ActiveGoalkeeperSnapshot"));
			Test.TestFalse(TEXT("Nested GK validation fails"), Result.ActiveGoalkeeperValidationResult.bSuccess);
			break;
		case 30:
			Input = MakeInput(); Input.bHasActiveGoalkeeper = true; Input.ActiveGoalkeeperSnapshot = MakeOutfield(GoalkeeperId, EPlayerPositionType::Defense, 1, 1, 1, 1, 1);
			EvaluateFailure(Test, Input, EThroughBallFeetPlanQueryErrorCode::ActiveGoalkeeperMustBeGoalkeeper, TEXT("ActiveGoalkeeperSnapshot"));
			break;
		case 31:
			Input = MakeInput(false, true); Input.ActiveGoalkeeperSnapshot.CardId = MarkerId;
			EvaluateFailure(Test, Input, EThroughBallFeetPlanQueryErrorCode::DuplicateDefendingGoalkeeperParticipant, TEXT("ActiveGoalkeeperSnapshot"));
			break;
		case 32:
			Input = MakeInput(true, true); Input.ActiveGoalkeeperSnapshot.CardId = HelperId;
			EvaluateFailure(Test, Input, EThroughBallFeetPlanQueryErrorCode::DuplicateDefendingGoalkeeperParticipant, TEXT("ActiveGoalkeeperSnapshot"));
			break;
		case 33:
			Input = MakeInput(false, true); Input.ActiveGoalkeeperSnapshot.CardId = RunnerId;
			EvaluateSuccess(Test, Input);
			break;
		case 34:
			Input = MakeInput(); Input.bHasActiveGoalkeeper = true; Input.ActiveGoalkeeperSnapshot = MakeOutfield(GoalkeeperId, EPlayerPositionType::Defense, 1, 1, 1, 1, 0);
			EvaluateFailure(Test, Input, EThroughBallFeetPlanQueryErrorCode::InvalidActiveGoalkeeperSnapshot, TEXT("ActiveGoalkeeperSnapshot"));
			break;
		case 35:
			Input = MakeInput(); Input.bHasActiveGoalkeeper = true; Input.ActiveGoalkeeperSnapshot = MakeOutfield(MarkerId, EPlayerPositionType::Defense, 1, 1, 1, 1, 1);
			EvaluateFailure(Test, Input, EThroughBallFeetPlanQueryErrorCode::ActiveGoalkeeperMustBeGoalkeeper, TEXT("ActiveGoalkeeperSnapshot"));
			break;
		case 36:
			Input = MakeInput(false, true); Input.ActiveGoalkeeperSnapshot = MakeGoalkeeper(GoalkeeperId, 5, 6);
			Test.TestEqual(TEXT("OneOnOne half contributes 2.5 plus two"), EvaluateSuccess(Test, Input).FormulaPlan.DefenseExternalModifier, 4.5f);
			break;
		case 37:
			Test.TestEqual(TEXT("Formula is Finishing"), EvaluateSuccess(Test, MakeInput()).FormulaPlan.FormulaType, EFormulaType::Finishing);
			break;
		case 38:
			Result = EvaluateSuccess(Test, MakeInput()); Test.TestEqual(TEXT("Carrier identity"), Result.FormulaPlan.CarrierId, CarrierId); Test.TestEqual(TEXT("Carrier Passing"), Result.FormulaPlan.CarrierPassing, 4);
			break;
		case 39:
			Result = EvaluateSuccess(Test, MakeInput()); Test.TestEqual(TEXT("Runner identity"), Result.FormulaPlan.RunnerId, RunnerId); Test.TestEqual(TEXT("Runner OffBall"), Result.FormulaPlan.RunnerOffBall, 5);
			break;
		case 40:
			Test.TestEqual(TEXT("Attack average preserves half"), EvaluateSuccess(Test, MakeInput()).FormulaPlan.AttackBaseValue, 4.5f);
			break;
		case 41:
			Test.TestEqual(TEXT("Attack modifier zero"), EvaluateSuccess(Test, MakeInput()).FormulaPlan.AttackExternalModifier, 0.0f);
			break;
		case 42:
			Test.TestEqual(TEXT("Attack D6 mapped"), EvaluateSuccess(Test, MakeInput()).FormulaPlan.AttackD6, 3);
			break;
		case 43:
			Test.TestEqual(TEXT("Attack stamina ordered"), EvaluateSuccess(Test, MakeInput()).FormulaPlan.AttackParticipatingStamina, TArray<int32>({5, 4}));
			break;
		case 44:
			Result = EvaluateSuccess(Test, MakeInput()); Test.TestEqual(TEXT("Marker identity"), Result.FormulaPlan.MarkerId, MarkerId); Test.TestEqual(TEXT("Marker Tackling"), Result.FormulaPlan.MarkerTackling, 5);
			break;
		case 45:
			Result = EvaluateSuccess(Test, MakeInput(true)); Test.TestTrue(TEXT("Helper present"), Result.FormulaPlan.bHasHelper); Test.TestEqual(TEXT("Helper identity"), Result.FormulaPlan.HelperId, HelperId); Test.TestEqual(TEXT("Helper Marking"), Result.FormulaPlan.HelperMarking, 4); Test.TestEqual(TEXT("Helper stamina"), Result.FormulaPlan.HelperStamina, 2);
			break;
		case 46:
			Result = EvaluateSuccess(Test, MakeInput()); Test.TestFalse(TEXT("Helper absent"), Result.FormulaPlan.bHasHelper); Test.TestTrue(TEXT("Helper identity default"), Result.FormulaPlan.HelperId.IsNone()); Test.TestEqual(TEXT("Helper Marking zero"), Result.FormulaPlan.HelperMarking, 0); Test.TestEqual(TEXT("Helper stamina zero"), Result.FormulaPlan.HelperStamina, 0);
			break;
		case 47:
			Test.TestEqual(TEXT("Defense average preserves half"), EvaluateSuccess(Test, MakeInput(true)).FormulaPlan.DefenseBaseValue, 4.5f);
			break;
		case 48:
			Result = EvaluateSuccess(Test, MakeInput(false, true)); Test.TestEqual(TEXT("GK identity"), Result.FormulaPlan.ActiveGoalkeeperId, GoalkeeperId); Test.TestEqual(TEXT("GK raw OneOnOne"), Result.FormulaPlan.GoalkeeperOneOnOne, 5); Test.TestEqual(TEXT("GK half included"), Result.FormulaPlan.DefenseExternalModifier, 4.5f);
			break;
		case 49:
			Result = EvaluateSuccess(Test, MakeInput()); Test.TestFalse(TEXT("GK absent"), Result.FormulaPlan.bHasActiveGoalkeeper); Test.TestTrue(TEXT("GK id default"), Result.FormulaPlan.ActiveGoalkeeperId.IsNone()); Test.TestEqual(TEXT("GK raw zero"), Result.FormulaPlan.GoalkeeperOneOnOne, 0);
			break;
		case 50:
			Input = MakeInput(false, true); Input.ActiveGoalkeeperSnapshot = MakeGoalkeeper(GoalkeeperId, 6, 6);
			Test.TestEqual(TEXT("Defense modifier is half plus two"), EvaluateSuccess(Test, Input).FormulaPlan.DefenseExternalModifier, 5.0f);
			break;
		case 51:
			Test.TestEqual(TEXT("Defense D6 mapped"), EvaluateSuccess(Test, MakeInput()).FormulaPlan.DefenseD6, 4);
			break;
		case 52:
			Test.TestEqual(TEXT("All defense stamina ordered"), EvaluateSuccess(Test, MakeInput(true, true)).FormulaPlan.DefenseParticipatingStamina, TArray<int32>({3, 2, 6}));
			Test.TestEqual(TEXT("Absent optional stamina omitted"), EvaluateSuccess(Test, MakeInput()).FormulaPlan.DefenseParticipatingStamina, TArray<int32>({3}));
			break;
		case 53:
			Result = EvaluateSuccess(Test, MakeInput(true, true)); Test.TestEqual(TEXT("LogId"), Result.FormulaPlan.LogId, LogId); Test.TestEqual(TEXT("TurnIndex"), Result.FormulaPlan.TurnIndex, 7); Test.TestEqual(TEXT("Attacking owner"), Result.FormulaPlan.AttackingOwnerId, AttackingOwnerId); Test.TestEqual(TEXT("Defending owner"), Result.FormulaPlan.DefendingOwnerId, DefendingOwnerId); Test.TestEqual(TEXT("Involved cards ordered"), Result.FormulaPlan.InvolvedCardIds, TArray<FName>({CarrierId, RunnerId, MarkerId, HelperId, GoalkeeperId})); Test.TestEqual(TEXT("Runner scores"), Result.FormulaPlan.GoalScorerCardId, RunnerId);
			break;
		case 54:
			Result = EvaluateSuccess(Test, MakeInput()); Test.TestTrue(TEXT("Attack win maps to goal"), Result.FormulaPlan.bAttackVictoryIsGoal); Test.TestTrue(TEXT("Defender win maps to miss"), Result.FormulaPlan.bDefenderVictoryIsMiss); Test.TestTrue(TEXT("Resolution ends attack"), Result.FormulaPlan.bAttackEndsAfterResolution); Test.TestFalse(TEXT("Resolution does not continue"), Result.FormulaPlan.bContinueResolution);
			break;
		case 55:
			Input = MakeInput(); Input.ParticipantEligibilityResult.bSuccess = false; Input.AttackD6 = 0;
			EvaluateFailure(Test, Input, EThroughBallFeetPlanQueryErrorCode::ParticipantEligibilityFailed, TEXT("ParticipantEligibilityResult"));
			break;
		case 56:
			Input = MakeInput(); Input.AttackD6 = 0; Input.DefenseD6 = 0;
			EvaluateFailure(Test, Input, EThroughBallFeetPlanQueryErrorCode::InvalidAttackD6, TEXT("AttackD6"));
			break;
		case 57:
			Input = MakeInput(); Input.AttackD6 = 0; Input.LogId.Invalidate();
			EvaluateFailure(Test, Input, EThroughBallFeetPlanQueryErrorCode::InvalidAttackD6, TEXT("AttackD6"));
			break;
		case 58:
			Input = MakeInput(); Input.LogId.Invalidate(); Input.bHasActiveGoalkeeper = true;
			EvaluateFailure(Test, Input, EThroughBallFeetPlanQueryErrorCode::InvalidLogContext, TEXT("LogId"));
			break;
		case 59:
			Input = MakeInput(); Input.bHasActiveGoalkeeper = true; Input.ActiveGoalkeeperSnapshot.Attributes.Stamina = 0;
			EvaluateFailure(Test, Input, EThroughBallFeetPlanQueryErrorCode::InvalidActiveGoalkeeperIdentity, TEXT("ActiveGoalkeeperSnapshot"));
			break;
		case 60:
			Input = MakeInput(); Input.bHasActiveGoalkeeper = true; Input.ActiveGoalkeeperSnapshot = MakeOutfield(GoalkeeperId, EPlayerPositionType::Defense, 1, 1, 1, 1, 0);
			EvaluateFailure(Test, Input, EThroughBallFeetPlanQueryErrorCode::InvalidActiveGoalkeeperSnapshot, TEXT("ActiveGoalkeeperSnapshot"));
			break;
		case 61:
			Input = MakeInput(); Input.bHasActiveGoalkeeper = true; Input.ActiveGoalkeeperSnapshot = MakeOutfield(MarkerId, EPlayerPositionType::Defense, 1, 1, 1, 1, 1);
			EvaluateFailure(Test, Input, EThroughBallFeetPlanQueryErrorCode::ActiveGoalkeeperMustBeGoalkeeper, TEXT("ActiveGoalkeeperSnapshot"));
			break;
		case 62:
			Input = MakeInput(true, true); { const auto Before = Input; Result = EvaluateSuccess(Test, Input); Test.TestTrue(TEXT("Success caller input unchanged"), AreInputsEqual(Input, Before)); Test.TestTrue(TEXT("Success result copies input"), AreInputsEqual(Result.Input, Before)); }
			break;
		case 63:
			Input = MakeInput(); Input.AttackD6 = 0; { const auto Before = Input; Result = EvaluateFailure(Test, Input, EThroughBallFeetPlanQueryErrorCode::InvalidAttackD6, TEXT("AttackD6")); Test.TestTrue(TEXT("Failure caller input unchanged"), AreInputsEqual(Input, Before)); Test.TestTrue(TEXT("Failure result copies input"), AreInputsEqual(Result.Input, Before)); }
			break;
		case 64:
			Input = MakeInput(true, true); { const auto Before = Input.ParticipantEligibilityResult; EvaluateSuccess(Test, Input); Test.TestTrue(TEXT("Eligibility Result unchanged"), AreEligibilityResultsEqual(Input.ParticipantEligibilityResult, Before)); }
			break;
		case 65:
			Input = MakeInput(true, true); { const auto First = EvaluateSuccess(Test, Input); const auto Second = EvaluateSuccess(Test, Input); Test.TestTrue(TEXT("Same input produces same Plan"), ArePlansEqual(First.FormulaPlan, Second.FormulaPlan)); Test.TestEqual(TEXT("Same success"), First.bSuccess, Second.bSuccess); Test.TestEqual(TEXT("Same decision"), First.Decision, Second.Decision); Test.TestEqual(TEXT("Same error"), First.ErrorCode, Second.ErrorCode); }
			break;
		case 66:
			Input = MakeInput(); Input.TurnIndex = -1; Result = EvaluateFailure(Test, Input, EThroughBallFeetPlanQueryErrorCode::InvalidLogContext, TEXT("TurnIndex")); Test.TestTrue(TEXT("Every Plan field remains default"), IsDefaultPlan(Result.FormulaPlan));
			break;
		default:
			Test.AddError(TEXT("Unknown Through Ball Feet Plan test case."));
			break;
		}
		return !Test.HasAnyErrors();
	}
}

#define THROUGH_BALL_FEET_PLAN_TEST(Number, Name) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
		FThroughBallFeetPlanTest##Number, \
		"FMCodex.CoreRules.ThroughBallFeetPlanQuery." Name, \
		EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter) \
	bool FThroughBallFeetPlanTest##Number::RunTest(const FString& Parameters) \
	{ \
		return ThroughBallFeetPlanQueryTests::RunCase(*this, Number); \
	}

THROUGH_BALL_FEET_PLAN_TEST(1, "01SuccessWithoutHelperOrGoalkeeper")
THROUGH_BALL_FEET_PLAN_TEST(2, "02SuccessWithHelperWithoutGoalkeeper")
THROUGH_BALL_FEET_PLAN_TEST(3, "03SuccessWithoutHelperWithGoalkeeper")
THROUGH_BALL_FEET_PLAN_TEST(4, "04SuccessWithHelperAndGoalkeeper")
THROUGH_BALL_FEET_PLAN_TEST(5, "05AcceptsMinimumAttackD6")
THROUGH_BALL_FEET_PLAN_TEST(6, "06AcceptsMaximumAttackD6")
THROUGH_BALL_FEET_PLAN_TEST(7, "07AcceptsMinimumDefenseD6")
THROUGH_BALL_FEET_PLAN_TEST(8, "08AcceptsMaximumDefenseD6")
THROUGH_BALL_FEET_PLAN_TEST(9, "09AllowsSameCardIdsAcrossOwners")
THROUGH_BALL_FEET_PLAN_TEST(10, "10AllowsGoalkeeperCardIdInAttackingIdentitySpace")
THROUGH_BALL_FEET_PLAN_TEST(11, "11RejectsFailedEligibility")
THROUGH_BALL_FEET_PLAN_TEST(12, "12RejectsInconsistentEligibilitySuccessState")
THROUGH_BALL_FEET_PLAN_TEST(13, "13RejectsEligibilityHelperMirrorMismatch")
THROUGH_BALL_FEET_PLAN_TEST(14, "14EligibilityFailurePrecedesD6")
THROUGH_BALL_FEET_PLAN_TEST(15, "15EligibilityFailurePrecedesGoalkeeper")
THROUGH_BALL_FEET_PLAN_TEST(16, "16RejectsAttackD6BelowMinimum")
THROUGH_BALL_FEET_PLAN_TEST(17, "17RejectsAttackD6AboveMaximum")
THROUGH_BALL_FEET_PLAN_TEST(18, "18RejectsDefenseD6BelowMinimum")
THROUGH_BALL_FEET_PLAN_TEST(19, "19RejectsDefenseD6AboveMaximum")
THROUGH_BALL_FEET_PLAN_TEST(20, "20AttackD6PrecedesDefenseD6")
THROUGH_BALL_FEET_PLAN_TEST(21, "21ConsumesOnlyFeetComparisonD6Values")
THROUGH_BALL_FEET_PLAN_TEST(22, "22AbsentGoalkeeperIgnoresEmptyIdentity")
THROUGH_BALL_FEET_PLAN_TEST(23, "23AbsentGoalkeeperIgnoresInvalidSnapshot")
THROUGH_BALL_FEET_PLAN_TEST(24, "24AbsentGoalkeeperIgnoresOutfieldType")
THROUGH_BALL_FEET_PLAN_TEST(25, "25AbsentGoalkeeperIgnoresMarkerDuplicate")
THROUGH_BALL_FEET_PLAN_TEST(26, "26AbsentGoalkeeperLeavesValidationDefault")
THROUGH_BALL_FEET_PLAN_TEST(27, "27AbsentGoalkeeperLeavesPlanContributionEmpty")
THROUGH_BALL_FEET_PLAN_TEST(28, "28RejectsEmptyActiveGoalkeeperIdentity")
THROUGH_BALL_FEET_PLAN_TEST(29, "29RejectsInvalidActiveGoalkeeperSnapshot")
THROUGH_BALL_FEET_PLAN_TEST(30, "30RejectsNonGoalkeeperActiveGoalkeeper")
THROUGH_BALL_FEET_PLAN_TEST(31, "31RejectsGoalkeeperMarkerDuplicate")
THROUGH_BALL_FEET_PLAN_TEST(32, "32RejectsGoalkeeperHelperDuplicate")
THROUGH_BALL_FEET_PLAN_TEST(33, "33AllowsGoalkeeperAttackerCardId")
THROUGH_BALL_FEET_PLAN_TEST(34, "34GoalkeeperSnapshotErrorPrecedesType")
THROUGH_BALL_FEET_PLAN_TEST(35, "35GoalkeeperTypeErrorPrecedesDuplicate")
THROUGH_BALL_FEET_PLAN_TEST(36, "36MapsOneOnOneHalfContribution")
THROUGH_BALL_FEET_PLAN_TEST(37, "37UsesFinishingFormula")
THROUGH_BALL_FEET_PLAN_TEST(38, "38MapsCarrierIdentityAndPassing")
THROUGH_BALL_FEET_PLAN_TEST(39, "39MapsRunnerIdentityAndOffBall")
THROUGH_BALL_FEET_PLAN_TEST(40, "40CalculatesAttackAverage")
THROUGH_BALL_FEET_PLAN_TEST(41, "41UsesZeroAttackModifier")
THROUGH_BALL_FEET_PLAN_TEST(42, "42MapsAttackD6")
THROUGH_BALL_FEET_PLAN_TEST(43, "43OrdersAttackStamina")
THROUGH_BALL_FEET_PLAN_TEST(44, "44MapsMarkerIdentityAndTackling")
THROUGH_BALL_FEET_PLAN_TEST(45, "45MapsPresentHelper")
THROUGH_BALL_FEET_PLAN_TEST(46, "46DefaultsAbsentHelper")
THROUGH_BALL_FEET_PLAN_TEST(47, "47CalculatesDefenseAverage")
THROUGH_BALL_FEET_PLAN_TEST(48, "48MapsPresentGoalkeeper")
THROUGH_BALL_FEET_PLAN_TEST(49, "49DefaultsAbsentGoalkeeper")
THROUGH_BALL_FEET_PLAN_TEST(50, "50CalculatesDefenseModifier")
THROUGH_BALL_FEET_PLAN_TEST(51, "51MapsDefenseD6")
THROUGH_BALL_FEET_PLAN_TEST(52, "52OrdersAndOmitsDefenseStamina")
THROUGH_BALL_FEET_PLAN_TEST(53, "53MapsIdentityAndLogMetadata")
THROUGH_BALL_FEET_PLAN_TEST(54, "54MapsOutcomeAndTerminalMetadata")
THROUGH_BALL_FEET_PLAN_TEST(55, "55EligibilityPrecedesD6")
THROUGH_BALL_FEET_PLAN_TEST(56, "56AttackD6PrecedesDefenseD6Again")
THROUGH_BALL_FEET_PLAN_TEST(57, "57D6PrecedesLogContext")
THROUGH_BALL_FEET_PLAN_TEST(58, "58LogContextPrecedesGoalkeeper")
THROUGH_BALL_FEET_PLAN_TEST(59, "59GoalkeeperIdentityPrecedesSnapshot")
THROUGH_BALL_FEET_PLAN_TEST(60, "60GoalkeeperSnapshotPrecedesType")
THROUGH_BALL_FEET_PLAN_TEST(61, "61GoalkeeperTypePrecedesDuplicate")
THROUGH_BALL_FEET_PLAN_TEST(62, "62SuccessPreservesInput")
THROUGH_BALL_FEET_PLAN_TEST(63, "63FailurePreservesInput")
THROUGH_BALL_FEET_PLAN_TEST(64, "64PreservesEligibilityResult")
THROUGH_BALL_FEET_PLAN_TEST(65, "65IsDeterministic")
THROUGH_BALL_FEET_PLAN_TEST(66, "66FailureKeepsDefaultFormulaPlan")

#undef THROUGH_BALL_FEET_PLAN_TEST

#endif
