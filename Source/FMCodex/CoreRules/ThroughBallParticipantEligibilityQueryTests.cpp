#if WITH_DEV_AUTOMATION_TESTS

#include "ThroughBallParticipantEligibilityQuery.h"

#include "Misc/AutomationTest.h"

namespace ThroughBallParticipantEligibilityQueryTests
{
	const FName SelectedSkillId(TEXT("Skill.ThroughBall.Selected"));
	const FName OtherThroughBallSkillId(TEXT("Skill.ThroughBall.Other"));
	const FName AttackingOwnerId(TEXT("Player.Attacking"));
	const FName DefendingOwnerId(TEXT("Player.Defending"));

	FPlayerCardRuleSnapshot MakeSnapshot(
		const FName CardId,
		const EPlayerPositionType PositionType =
			EPlayerPositionType::Midfield)
	{
		FPlayerCardRuleSnapshot Snapshot;
		Snapshot.CardId = CardId;
		Snapshot.PositionTypes = { PositionType };
		return Snapshot;
	}

	FPlayerCardRuleSnapshot MakeGoalkeeper(const FName CardId)
	{
		FPlayerCardRuleSnapshot Snapshot =
			MakeSnapshot(CardId, EPlayerPositionType::Goalkeeper);
		Snapshot.bIsGoalkeeper = true;
		Snapshot.bHasGoalkeeperAttributes = true;
		return Snapshot;
	}

	FSkillRuleSnapshotSet MakeSkillRules(
		const int32 MinActionPoint = 3,
		const int32 MaxActionPoint = 6)
	{
		FSkillRuleSnapshot Rule;
		Rule.SkillId = SelectedSkillId;
		Rule.SkillType = ESkillRuleType::ThroughBall;
		Rule.MinTriggerActionPoint = MinActionPoint;
		Rule.MaxTriggerActionPoint = MaxActionPoint;

		FSkillRuleSnapshot OtherRule;
		OtherRule.SkillId = OtherThroughBallSkillId;
		OtherRule.SkillType = ESkillRuleType::ThroughBall;
		OtherRule.MinTriggerActionPoint = 2;
		OtherRule.MaxTriggerActionPoint = 8;

		FSkillRuleSnapshotSet SkillRules;
		SkillRules.SkillRules = { Rule, OtherRule };
		return SkillRules;
	}

	FThroughBallParticipantEligibilityQueryInput MakeValidInput(
		const bool bHasHelper = false)
	{
		FThroughBallParticipantEligibilityQueryInput Input;
		Input.SelectedSkillId = SelectedSkillId;
		Input.CurrentActionPoint = 4;
		Input.AttackingOwnerId = AttackingOwnerId;
		Input.DefendingOwnerId = DefendingOwnerId;
		Input.CarrierSnapshot =
			MakeSnapshot(TEXT("Card.Carrier"), EPlayerPositionType::Midfield);
		Input.CarrierSnapshot.SkillIds = { SelectedSkillId };
		Input.RunnerSnapshot =
			MakeSnapshot(TEXT("Card.Runner"), EPlayerPositionType::Defense);
		Input.MarkerSnapshot =
			MakeSnapshot(TEXT("Card.Marker"), EPlayerPositionType::Defense);
		Input.bHasHelper = bHasHelper;
		Input.HelperSnapshot =
			MakeSnapshot(TEXT("Card.Helper"), EPlayerPositionType::Attack);
		Input.bIsRunnerInAttackingForwardArea = true;
		return Input;
	}

	bool AreAttributesEqual(
		const FPlayerAttributes& Left,
		const FPlayerAttributes& Right)
	{
		return Left.Shooting == Right.Shooting
			&& Left.Dribbling == Right.Dribbling
			&& Left.Passing == Right.Passing
			&& Left.OffBall == Right.OffBall
			&& Left.Marking == Right.Marking
			&& Left.Tackling == Right.Tackling
			&& Left.Speed == Right.Speed
			&& Left.Strength == Right.Strength
			&& Left.Stamina == Right.Stamina
			&& Left.LongShot == Right.LongShot;
	}

	bool AreGoalkeeperAttributesEqual(
		const FGoalkeeperAttributes& Left,
		const FGoalkeeperAttributes& Right)
	{
		return Left.Handling == Right.Handling
			&& Left.Positioning == Right.Positioning
			&& Left.Reflex == Right.Reflex
			&& Left.Aerial == Right.Aerial
			&& Left.Anticipation == Right.Anticipation
			&& Left.OneOnOne == Right.OneOnOne;
	}

	bool AreSnapshotsEqual(
		const FPlayerCardRuleSnapshot& Left,
		const FPlayerCardRuleSnapshot& Right)
	{
		return Left.CardId == Right.CardId
			&& Left.PositionTypes == Right.PositionTypes
			&& AreAttributesEqual(Left.Attributes, Right.Attributes)
			&& Left.bIsGoalkeeper == Right.bIsGoalkeeper
			&& Left.bHasGoalkeeperAttributes
				== Right.bHasGoalkeeperAttributes
			&& AreGoalkeeperAttributesEqual(
				Left.GoalkeeperAttributes,
				Right.GoalkeeperAttributes)
			&& Left.Rarity == Right.Rarity
			&& Left.SkillIds == Right.SkillIds;
	}

	bool AreInputsEqual(
		const FThroughBallParticipantEligibilityQueryInput& Left,
		const FThroughBallParticipantEligibilityQueryInput& Right)
	{
		return Left.SelectedSkillId == Right.SelectedSkillId
			&& Left.CurrentActionPoint == Right.CurrentActionPoint
			&& Left.AttackingOwnerId == Right.AttackingOwnerId
			&& Left.DefendingOwnerId == Right.DefendingOwnerId
			&& AreSnapshotsEqual(Left.CarrierSnapshot, Right.CarrierSnapshot)
			&& AreSnapshotsEqual(Left.RunnerSnapshot, Right.RunnerSnapshot)
			&& AreSnapshotsEqual(Left.MarkerSnapshot, Right.MarkerSnapshot)
			&& Left.bHasHelper == Right.bHasHelper
			&& AreSnapshotsEqual(Left.HelperSnapshot, Right.HelperSnapshot)
			&& Left.bIsRunnerInAttackingForwardArea
				== Right.bIsRunnerInAttackingForwardArea;
	}

	bool AreSkillRulesEqual(
		const FSkillRuleSnapshotSet& Left,
		const FSkillRuleSnapshotSet& Right)
	{
		if (Left.SkillRules.Num() != Right.SkillRules.Num())
		{
			return false;
		}
		for (int32 Index = 0; Index < Left.SkillRules.Num(); ++Index)
		{
			const FSkillRuleSnapshot& LeftRule = Left.SkillRules[Index];
			const FSkillRuleSnapshot& RightRule = Right.SkillRules[Index];
			if (LeftRule.SkillId != RightRule.SkillId
				|| LeftRule.SkillType != RightRule.SkillType
				|| LeftRule.MinTriggerActionPoint
					!= RightRule.MinTriggerActionPoint
				|| LeftRule.MaxTriggerActionPoint
					!= RightRule.MaxTriggerActionPoint)
			{
				return false;
			}
		}
		return true;
	}

	bool AreValidationResultsEqual(
		const FPlayerCardRuleSnapshotValidationResult& Left,
		const FPlayerCardRuleSnapshotValidationResult& Right)
	{
		return Left.bSuccess == Right.bSuccess
			&& Left.bIsValid == Right.bIsValid
			&& Left.ErrorCode == Right.ErrorCode
			&& Left.ErrorMessage == Right.ErrorMessage
			&& Left.InvalidCardId == Right.InvalidCardId
			&& Left.DuplicateCardIds == Right.DuplicateCardIds;
	}

	bool AreResultsEqual(
		const FThroughBallParticipantEligibilityQueryResult& Left,
		const FThroughBallParticipantEligibilityQueryResult& Right)
	{
		return Left.bSuccess == Right.bSuccess
			&& Left.ErrorCode == Right.ErrorCode
			&& Left.ErrorMessage == Right.ErrorMessage
			&& Left.InvalidField == Right.InvalidField
			&& AreInputsEqual(Left.Input, Right.Input)
			&& Left.bHasHelper == Right.bHasHelper
			&& Left.SkillRuleQueryResult.bSuccess
				== Right.SkillRuleQueryResult.bSuccess
			&& Left.SkillRuleQueryResult.ErrorCode
				== Right.SkillRuleQueryResult.ErrorCode
			&& Left.SkillRuleQueryResult.Snapshot.SkillId
				== Right.SkillRuleQueryResult.Snapshot.SkillId
			&& AreValidationResultsEqual(
				Left.CarrierSnapshotValidationResult,
				Right.CarrierSnapshotValidationResult)
			&& AreValidationResultsEqual(
				Left.RunnerSnapshotValidationResult,
				Right.RunnerSnapshotValidationResult)
			&& AreValidationResultsEqual(
				Left.MarkerSnapshotValidationResult,
				Right.MarkerSnapshotValidationResult)
			&& AreValidationResultsEqual(
				Left.HelperSnapshotValidationResult,
				Right.HelperSnapshotValidationResult);
	}

	bool ExpectSuccess(
		FAutomationTestBase& Test,
		const FSkillRuleSnapshotSet& SkillRules,
		const FThroughBallParticipantEligibilityQueryInput& Input)
	{
		const FThroughBallParticipantEligibilityQueryResult Result =
			FThroughBallParticipantEligibilityQuery::Evaluate(
				SkillRules,
				Input);
		Test.TestTrue(TEXT("Eligibility succeeds"), Result.bSuccess);
		Test.TestEqual(
			TEXT("Success has no error"),
			Result.ErrorCode,
			EThroughBallParticipantEligibilityQueryErrorCode::None);
		Test.TestTrue(
			TEXT("Success has no invalid field"),
			Result.InvalidField.IsNone());
		Test.TestTrue(
			TEXT("Success preserves input"),
			AreInputsEqual(Result.Input, Input));
		Test.TestEqual(
			TEXT("Success preserves Helper presence"),
			Result.bHasHelper,
			Input.bHasHelper);
		return true;
	}

	bool ExpectFailure(
		FAutomationTestBase& Test,
		const FSkillRuleSnapshotSet& SkillRules,
		const FThroughBallParticipantEligibilityQueryInput& Input,
		const EThroughBallParticipantEligibilityQueryErrorCode ExpectedError,
		const FName ExpectedInvalidField)
	{
		const FThroughBallParticipantEligibilityQueryResult Result =
			FThroughBallParticipantEligibilityQuery::Evaluate(
				SkillRules,
				Input);
		Test.TestFalse(TEXT("Eligibility fails"), Result.bSuccess);
		Test.TestEqual(
			TEXT("Expected error is returned"),
			Result.ErrorCode,
			ExpectedError);
		Test.TestEqual(
			TEXT("Expected invalid field is returned"),
			Result.InvalidField,
			ExpectedInvalidField);
		Test.TestFalse(
			TEXT("Failure has an error message"),
			Result.ErrorMessage.IsEmpty());
		Test.TestTrue(
			TEXT("Failure preserves input"),
			AreInputsEqual(Result.Input, Input));
		Test.TestEqual(
			TEXT("Failure preserves Helper presence"),
			Result.bHasHelper,
			Input.bHasHelper);
		return true;
	}
}

#define THROUGH_BALL_ELIGIBILITY_TEST(ClassName, TestName) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
		ClassName, \
		"FMCodex.CoreRules.ThroughBallParticipantEligibilityQuery." TestName, \
		EAutomationTestFlags::EditorContext \
			| EAutomationTestFlags::EngineFilter)

THROUGH_BALL_ELIGIBILITY_TEST(FThroughBallEligibility01, "01SucceedsWithoutHelper")
bool FThroughBallEligibility01::RunTest(const FString& Parameters)
{
	return ThroughBallParticipantEligibilityQueryTests::ExpectSuccess(
		*this,
		ThroughBallParticipantEligibilityQueryTests::MakeSkillRules(),
		ThroughBallParticipantEligibilityQueryTests::MakeValidInput());
}

THROUGH_BALL_ELIGIBILITY_TEST(FThroughBallEligibility02, "02SucceedsWithHelper")
bool FThroughBallEligibility02::RunTest(const FString& Parameters)
{
	return ThroughBallParticipantEligibilityQueryTests::ExpectSuccess(
		*this,
		ThroughBallParticipantEligibilityQueryTests::MakeSkillRules(),
		ThroughBallParticipantEligibilityQueryTests::MakeValidInput(true));
}

THROUGH_BALL_ELIGIBILITY_TEST(FThroughBallEligibility03, "03AllowsSameCardIdsAcrossOwners")
bool FThroughBallEligibility03::RunTest(const FString& Parameters)
{
	auto Input = ThroughBallParticipantEligibilityQueryTests::MakeValidInput(true);
	Input.MarkerSnapshot.CardId = Input.CarrierSnapshot.CardId;
	Input.HelperSnapshot.CardId = Input.RunnerSnapshot.CardId;
	return ThroughBallParticipantEligibilityQueryTests::ExpectSuccess(
		*this,
		ThroughBallParticipantEligibilityQueryTests::MakeSkillRules(),
		Input);
}

THROUGH_BALL_ELIGIBILITY_TEST(FThroughBallEligibility04, "04UsesExplicitRunnerForwardProof")
bool FThroughBallEligibility04::RunTest(const FString& Parameters)
{
	auto Input = ThroughBallParticipantEligibilityQueryTests::MakeValidInput();
	Input.RunnerSnapshot.PositionTypes = { EPlayerPositionType::Defense };
	return ThroughBallParticipantEligibilityQueryTests::ExpectSuccess(
		*this,
		ThroughBallParticipantEligibilityQueryTests::MakeSkillRules(),
		Input);
}

THROUGH_BALL_ELIGIBILITY_TEST(FThroughBallEligibility05, "05RunnerNeedsNoThroughBallSkill")
bool FThroughBallEligibility05::RunTest(const FString& Parameters)
{
	auto Input = ThroughBallParticipantEligibilityQueryTests::MakeValidInput();
	Input.RunnerSnapshot.SkillIds.Reset();
	return ThroughBallParticipantEligibilityQueryTests::ExpectSuccess(
		*this,
		ThroughBallParticipantEligibilityQueryTests::MakeSkillRules(),
		Input);
}

THROUGH_BALL_ELIGIBILITY_TEST(FThroughBallEligibility06, "06AcceptsMinimumActionPoint")
bool FThroughBallEligibility06::RunTest(const FString& Parameters)
{
	auto Input = ThroughBallParticipantEligibilityQueryTests::MakeValidInput();
	Input.CurrentActionPoint = 3;
	return ThroughBallParticipantEligibilityQueryTests::ExpectSuccess(
		*this,
		ThroughBallParticipantEligibilityQueryTests::MakeSkillRules(),
		Input);
}

THROUGH_BALL_ELIGIBILITY_TEST(FThroughBallEligibility07, "07AcceptsMaximumActionPoint")
bool FThroughBallEligibility07::RunTest(const FString& Parameters)
{
	auto Input = ThroughBallParticipantEligibilityQueryTests::MakeValidInput();
	Input.CurrentActionPoint = 6;
	return ThroughBallParticipantEligibilityQueryTests::ExpectSuccess(
		*this,
		ThroughBallParticipantEligibilityQueryTests::MakeSkillRules(),
		Input);
}

THROUGH_BALL_ELIGIBILITY_TEST(FThroughBallEligibility08, "08RejectsEmptySelectedSkillId")
bool FThroughBallEligibility08::RunTest(const FString& Parameters)
{
	auto Input = ThroughBallParticipantEligibilityQueryTests::MakeValidInput();
	Input.SelectedSkillId = NAME_None;
	return ThroughBallParticipantEligibilityQueryTests::ExpectFailure(
		*this,
		ThroughBallParticipantEligibilityQueryTests::MakeSkillRules(),
		Input,
		EThroughBallParticipantEligibilityQueryErrorCode::InvalidSelectedSkillId,
		TEXT("SelectedSkillId"));
}

THROUGH_BALL_ELIGIBILITY_TEST(FThroughBallEligibility09, "09RejectsMissingSkillRule")
bool FThroughBallEligibility09::RunTest(const FString& Parameters)
{
	auto Input = ThroughBallParticipantEligibilityQueryTests::MakeValidInput();
	Input.SelectedSkillId = TEXT("Skill.ThroughBall.Missing");
	return ThroughBallParticipantEligibilityQueryTests::ExpectFailure(
		*this,
		ThroughBallParticipantEligibilityQueryTests::MakeSkillRules(),
		Input,
		EThroughBallParticipantEligibilityQueryErrorCode::SkillRuleLookupFailed,
		TEXT("SelectedSkillId"));
}

THROUGH_BALL_ELIGIBILITY_TEST(FThroughBallEligibility10, "10RejectsInvalidSkillRuleSet")
bool FThroughBallEligibility10::RunTest(const FString& Parameters)
{
	auto Rules = ThroughBallParticipantEligibilityQueryTests::MakeSkillRules();
	Rules.SkillRules[1].SkillId = Rules.SkillRules[0].SkillId;
	const auto Input = ThroughBallParticipantEligibilityQueryTests::MakeValidInput();
	const auto Result = FThroughBallParticipantEligibilityQuery::Evaluate(Rules, Input);
	TestFalse(TEXT("Eligibility fails"), Result.bSuccess);
	TestEqual(TEXT("Lookup failure is returned"), Result.ErrorCode,
		EThroughBallParticipantEligibilityQueryErrorCode::SkillRuleLookupFailed);
	TestEqual(TEXT("Nested validator error is retained"),
		Result.SkillRuleQueryResult.ValidationResult.ErrorCode,
		ESkillRuleSnapshotValidationErrorCode::DuplicateSkillId);
	return true;
}

THROUGH_BALL_ELIGIBILITY_TEST(FThroughBallEligibility11, "11RejectsNonThroughBallSkillRule")
bool FThroughBallEligibility11::RunTest(const FString& Parameters)
{
	auto Rules = ThroughBallParticipantEligibilityQueryTests::MakeSkillRules();
	Rules.SkillRules[0].SkillType = ESkillRuleType::Cross;
	return ThroughBallParticipantEligibilityQueryTests::ExpectFailure(
		*this, Rules,
		ThroughBallParticipantEligibilityQueryTests::MakeValidInput(),
		EThroughBallParticipantEligibilityQueryErrorCode::UnsupportedSkillRuleType,
		TEXT("SelectedSkillId"));
}

THROUGH_BALL_ELIGIBILITY_TEST(FThroughBallEligibility12, "12RejectsActionPointBelowMinimum")
bool FThroughBallEligibility12::RunTest(const FString& Parameters)
{
	auto Input = ThroughBallParticipantEligibilityQueryTests::MakeValidInput();
	Input.CurrentActionPoint = 2;
	return ThroughBallParticipantEligibilityQueryTests::ExpectFailure(
		*this, ThroughBallParticipantEligibilityQueryTests::MakeSkillRules(), Input,
		EThroughBallParticipantEligibilityQueryErrorCode::ActionPointBelowMinimum,
		TEXT("CurrentActionPoint"));
}

THROUGH_BALL_ELIGIBILITY_TEST(FThroughBallEligibility13, "13RejectsActionPointAboveMaximum")
bool FThroughBallEligibility13::RunTest(const FString& Parameters)
{
	auto Input = ThroughBallParticipantEligibilityQueryTests::MakeValidInput();
	Input.CurrentActionPoint = 7;
	return ThroughBallParticipantEligibilityQueryTests::ExpectFailure(
		*this, ThroughBallParticipantEligibilityQueryTests::MakeSkillRules(), Input,
		EThroughBallParticipantEligibilityQueryErrorCode::ActionPointAboveMaximum,
		TEXT("CurrentActionPoint"));
}

THROUGH_BALL_ELIGIBILITY_TEST(FThroughBallEligibility14, "14RejectsCarrierWithoutSelectedSkill")
bool FThroughBallEligibility14::RunTest(const FString& Parameters)
{
	auto Input = ThroughBallParticipantEligibilityQueryTests::MakeValidInput();
	Input.CarrierSnapshot.SkillIds.Reset();
	return ThroughBallParticipantEligibilityQueryTests::ExpectFailure(
		*this, ThroughBallParticipantEligibilityQueryTests::MakeSkillRules(), Input,
		EThroughBallParticipantEligibilityQueryErrorCode::CarrierDoesNotOwnSelectedSkill,
		TEXT("CarrierSnapshot"));
}

THROUGH_BALL_ELIGIBILITY_TEST(FThroughBallEligibility15, "15RejectsCarrierWithOnlyOtherThroughBallSkill")
bool FThroughBallEligibility15::RunTest(const FString& Parameters)
{
	auto Input = ThroughBallParticipantEligibilityQueryTests::MakeValidInput();
	Input.CarrierSnapshot.SkillIds =
		{ ThroughBallParticipantEligibilityQueryTests::OtherThroughBallSkillId };
	return ThroughBallParticipantEligibilityQueryTests::ExpectFailure(
		*this, ThroughBallParticipantEligibilityQueryTests::MakeSkillRules(), Input,
		EThroughBallParticipantEligibilityQueryErrorCode::CarrierDoesNotOwnSelectedSkill,
		TEXT("CarrierSnapshot"));
}

THROUGH_BALL_ELIGIBILITY_TEST(FThroughBallEligibility16, "16RejectsEmptyAttackingOwnerId")
bool FThroughBallEligibility16::RunTest(const FString& Parameters)
{
	auto Input = ThroughBallParticipantEligibilityQueryTests::MakeValidInput();
	Input.AttackingOwnerId = NAME_None;
	return ThroughBallParticipantEligibilityQueryTests::ExpectFailure(
		*this, ThroughBallParticipantEligibilityQueryTests::MakeSkillRules(), Input,
		EThroughBallParticipantEligibilityQueryErrorCode::InvalidAttackingOwnerIdentity,
		TEXT("AttackingOwnerId"));
}

THROUGH_BALL_ELIGIBILITY_TEST(FThroughBallEligibility17, "17RejectsEmptyDefendingOwnerId")
bool FThroughBallEligibility17::RunTest(const FString& Parameters)
{
	auto Input = ThroughBallParticipantEligibilityQueryTests::MakeValidInput();
	Input.DefendingOwnerId = NAME_None;
	return ThroughBallParticipantEligibilityQueryTests::ExpectFailure(
		*this, ThroughBallParticipantEligibilityQueryTests::MakeSkillRules(), Input,
		EThroughBallParticipantEligibilityQueryErrorCode::InvalidDefendingOwnerIdentity,
		TEXT("DefendingOwnerId"));
}

THROUGH_BALL_ELIGIBILITY_TEST(FThroughBallEligibility18, "18RejectsDuplicateOwnerIdentity")
bool FThroughBallEligibility18::RunTest(const FString& Parameters)
{
	auto Input = ThroughBallParticipantEligibilityQueryTests::MakeValidInput();
	Input.DefendingOwnerId = Input.AttackingOwnerId;
	return ThroughBallParticipantEligibilityQueryTests::ExpectFailure(
		*this, ThroughBallParticipantEligibilityQueryTests::MakeSkillRules(), Input,
		EThroughBallParticipantEligibilityQueryErrorCode::DuplicateOwnerIdentity,
		TEXT("DefendingOwnerId"));
}

THROUGH_BALL_ELIGIBILITY_TEST(FThroughBallEligibility19, "19RejectsEmptyCarrierCardId")
bool FThroughBallEligibility19::RunTest(const FString& Parameters)
{
	auto Input = ThroughBallParticipantEligibilityQueryTests::MakeValidInput();
	Input.CarrierSnapshot.CardId = NAME_None;
	return ThroughBallParticipantEligibilityQueryTests::ExpectFailure(
		*this, ThroughBallParticipantEligibilityQueryTests::MakeSkillRules(), Input,
		EThroughBallParticipantEligibilityQueryErrorCode::InvalidCarrierIdentity,
		TEXT("CarrierSnapshot"));
}

THROUGH_BALL_ELIGIBILITY_TEST(FThroughBallEligibility20, "20RejectsInvalidCarrierSnapshot")
bool FThroughBallEligibility20::RunTest(const FString& Parameters)
{
	auto Input = ThroughBallParticipantEligibilityQueryTests::MakeValidInput();
	Input.CarrierSnapshot.Attributes.Shooting = 0;
	const auto Result = FThroughBallParticipantEligibilityQuery::Evaluate(
		ThroughBallParticipantEligibilityQueryTests::MakeSkillRules(), Input);
	TestEqual(TEXT("Carrier Snapshot error is returned"), Result.ErrorCode,
		EThroughBallParticipantEligibilityQueryErrorCode::InvalidCarrierSnapshot);
	TestEqual(TEXT("Carrier validator detail is retained"),
		Result.CarrierSnapshotValidationResult.ErrorCode,
		EPlayerCardRuleSnapshotValidationErrorCode::AttributeOutOfRange);
	return true;
}

THROUGH_BALL_ELIGIBILITY_TEST(FThroughBallEligibility21, "21RejectsGoalkeeperCarrier")
bool FThroughBallEligibility21::RunTest(const FString& Parameters)
{
	auto Input = ThroughBallParticipantEligibilityQueryTests::MakeValidInput();
	Input.CarrierSnapshot = ThroughBallParticipantEligibilityQueryTests::MakeGoalkeeper(
		TEXT("Card.Carrier.GK"));
	Input.CarrierSnapshot.SkillIds =
		{ ThroughBallParticipantEligibilityQueryTests::SelectedSkillId };
	return ThroughBallParticipantEligibilityQueryTests::ExpectFailure(
		*this, ThroughBallParticipantEligibilityQueryTests::MakeSkillRules(), Input,
		EThroughBallParticipantEligibilityQueryErrorCode::UnsupportedCarrierGoalkeeper,
		TEXT("CarrierSnapshot"));
}

THROUGH_BALL_ELIGIBILITY_TEST(FThroughBallEligibility22, "22AddsNoCarrierPositionRestriction")
bool FThroughBallEligibility22::RunTest(const FString& Parameters)
{
	auto Input = ThroughBallParticipantEligibilityQueryTests::MakeValidInput();
	Input.CarrierSnapshot.PositionTypes = { EPlayerPositionType::Defense };
	return ThroughBallParticipantEligibilityQueryTests::ExpectSuccess(
		*this, ThroughBallParticipantEligibilityQueryTests::MakeSkillRules(), Input);
}

THROUGH_BALL_ELIGIBILITY_TEST(FThroughBallEligibility23, "23RejectsEmptyRunnerCardId")
bool FThroughBallEligibility23::RunTest(const FString& Parameters)
{
	auto Input = ThroughBallParticipantEligibilityQueryTests::MakeValidInput();
	Input.RunnerSnapshot.CardId = NAME_None;
	return ThroughBallParticipantEligibilityQueryTests::ExpectFailure(
		*this, ThroughBallParticipantEligibilityQueryTests::MakeSkillRules(), Input,
		EThroughBallParticipantEligibilityQueryErrorCode::InvalidRunnerIdentity,
		TEXT("RunnerSnapshot"));
}

THROUGH_BALL_ELIGIBILITY_TEST(FThroughBallEligibility24, "24RejectsInvalidRunnerSnapshot")
bool FThroughBallEligibility24::RunTest(const FString& Parameters)
{
	auto Input = ThroughBallParticipantEligibilityQueryTests::MakeValidInput();
	Input.RunnerSnapshot.PositionTypes.Reset();
	const auto Result = FThroughBallParticipantEligibilityQuery::Evaluate(
		ThroughBallParticipantEligibilityQueryTests::MakeSkillRules(), Input);
	TestEqual(TEXT("Runner Snapshot error is returned"), Result.ErrorCode,
		EThroughBallParticipantEligibilityQueryErrorCode::InvalidRunnerSnapshot);
	TestEqual(TEXT("Runner validator detail is retained"),
		Result.RunnerSnapshotValidationResult.ErrorCode,
		EPlayerCardRuleSnapshotValidationErrorCode::EmptyPositionTypes);
	return true;
}

THROUGH_BALL_ELIGIBILITY_TEST(FThroughBallEligibility25, "25RejectsGoalkeeperRunner")
bool FThroughBallEligibility25::RunTest(const FString& Parameters)
{
	auto Input = ThroughBallParticipantEligibilityQueryTests::MakeValidInput();
	Input.RunnerSnapshot = ThroughBallParticipantEligibilityQueryTests::MakeGoalkeeper(
		TEXT("Card.Runner.GK"));
	return ThroughBallParticipantEligibilityQueryTests::ExpectFailure(
		*this, ThroughBallParticipantEligibilityQueryTests::MakeSkillRules(), Input,
		EThroughBallParticipantEligibilityQueryErrorCode::UnsupportedRunnerGoalkeeper,
		TEXT("RunnerSnapshot"));
}

THROUGH_BALL_ELIGIBILITY_TEST(FThroughBallEligibility26, "26RejectsMissingRunnerForwardProof")
bool FThroughBallEligibility26::RunTest(const FString& Parameters)
{
	auto Input = ThroughBallParticipantEligibilityQueryTests::MakeValidInput();
	Input.bIsRunnerInAttackingForwardArea = false;
	return ThroughBallParticipantEligibilityQueryTests::ExpectFailure(
		*this, ThroughBallParticipantEligibilityQueryTests::MakeSkillRules(), Input,
		EThroughBallParticipantEligibilityQueryErrorCode::RunnerNotInAttackingForwardArea,
		TEXT("bIsRunnerInAttackingForwardArea"));
}

THROUGH_BALL_ELIGIBILITY_TEST(FThroughBallEligibility27, "27RejectsDuplicateAttackingParticipant")
bool FThroughBallEligibility27::RunTest(const FString& Parameters)
{
	auto Input = ThroughBallParticipantEligibilityQueryTests::MakeValidInput();
	Input.RunnerSnapshot.CardId = Input.CarrierSnapshot.CardId;
	return ThroughBallParticipantEligibilityQueryTests::ExpectFailure(
		*this, ThroughBallParticipantEligibilityQueryTests::MakeSkillRules(), Input,
		EThroughBallParticipantEligibilityQueryErrorCode::DuplicateAttackingParticipant,
		TEXT("RunnerSnapshot"));
}

THROUGH_BALL_ELIGIBILITY_TEST(FThroughBallEligibility28, "28AttackPositionDoesNotOverrideMissingForwardProof")
bool FThroughBallEligibility28::RunTest(const FString& Parameters)
{
	auto Input = ThroughBallParticipantEligibilityQueryTests::MakeValidInput();
	Input.RunnerSnapshot.PositionTypes = { EPlayerPositionType::Attack };
	Input.bIsRunnerInAttackingForwardArea = false;
	return ThroughBallParticipantEligibilityQueryTests::ExpectFailure(
		*this, ThroughBallParticipantEligibilityQueryTests::MakeSkillRules(), Input,
		EThroughBallParticipantEligibilityQueryErrorCode::RunnerNotInAttackingForwardArea,
		TEXT("bIsRunnerInAttackingForwardArea"));
}

THROUGH_BALL_ELIGIBILITY_TEST(FThroughBallEligibility29, "29RejectsEmptyMarkerCardId")
bool FThroughBallEligibility29::RunTest(const FString& Parameters)
{
	auto Input = ThroughBallParticipantEligibilityQueryTests::MakeValidInput();
	Input.MarkerSnapshot.CardId = NAME_None;
	return ThroughBallParticipantEligibilityQueryTests::ExpectFailure(
		*this, ThroughBallParticipantEligibilityQueryTests::MakeSkillRules(), Input,
		EThroughBallParticipantEligibilityQueryErrorCode::InvalidMarkerIdentity,
		TEXT("MarkerSnapshot"));
}

THROUGH_BALL_ELIGIBILITY_TEST(FThroughBallEligibility30, "30RejectsInvalidMarkerSnapshot")
bool FThroughBallEligibility30::RunTest(const FString& Parameters)
{
	auto Input = ThroughBallParticipantEligibilityQueryTests::MakeValidInput();
	Input.MarkerSnapshot.Attributes.Marking = 7;
	const auto Result = FThroughBallParticipantEligibilityQuery::Evaluate(
		ThroughBallParticipantEligibilityQueryTests::MakeSkillRules(), Input);
	TestEqual(TEXT("Marker Snapshot error is returned"), Result.ErrorCode,
		EThroughBallParticipantEligibilityQueryErrorCode::InvalidMarkerSnapshot);
	TestEqual(TEXT("Marker validator detail is retained"),
		Result.MarkerSnapshotValidationResult.ErrorCode,
		EPlayerCardRuleSnapshotValidationErrorCode::AttributeOutOfRange);
	return true;
}

THROUGH_BALL_ELIGIBILITY_TEST(FThroughBallEligibility31, "31RejectsGoalkeeperMarker")
bool FThroughBallEligibility31::RunTest(const FString& Parameters)
{
	auto Input = ThroughBallParticipantEligibilityQueryTests::MakeValidInput();
	Input.MarkerSnapshot = ThroughBallParticipantEligibilityQueryTests::MakeGoalkeeper(
		TEXT("Card.Marker.GK"));
	return ThroughBallParticipantEligibilityQueryTests::ExpectFailure(
		*this, ThroughBallParticipantEligibilityQueryTests::MakeSkillRules(), Input,
		EThroughBallParticipantEligibilityQueryErrorCode::UnsupportedMarkerGoalkeeper,
		TEXT("MarkerSnapshot"));
}

THROUGH_BALL_ELIGIBILITY_TEST(FThroughBallEligibility32, "32AddsNoMarkerPositionRestriction")
bool FThroughBallEligibility32::RunTest(const FString& Parameters)
{
	auto Input = ThroughBallParticipantEligibilityQueryTests::MakeValidInput();
	Input.MarkerSnapshot.PositionTypes = { EPlayerPositionType::Attack };
	return ThroughBallParticipantEligibilityQueryTests::ExpectSuccess(
		*this, ThroughBallParticipantEligibilityQueryTests::MakeSkillRules(), Input);
}

THROUGH_BALL_ELIGIBILITY_TEST(FThroughBallEligibility33, "33RejectsPresentHelperWithEmptyCardId")
bool FThroughBallEligibility33::RunTest(const FString& Parameters)
{
	auto Input = ThroughBallParticipantEligibilityQueryTests::MakeValidInput(true);
	Input.HelperSnapshot.CardId = NAME_None;
	return ThroughBallParticipantEligibilityQueryTests::ExpectFailure(
		*this, ThroughBallParticipantEligibilityQueryTests::MakeSkillRules(), Input,
		EThroughBallParticipantEligibilityQueryErrorCode::InvalidHelperIdentity,
		TEXT("HelperSnapshot"));
}

THROUGH_BALL_ELIGIBILITY_TEST(FThroughBallEligibility34, "34RejectsInvalidPresentHelperSnapshot")
bool FThroughBallEligibility34::RunTest(const FString& Parameters)
{
	auto Input = ThroughBallParticipantEligibilityQueryTests::MakeValidInput(true);
	Input.HelperSnapshot.SkillIds = { NAME_None };
	const auto Result = FThroughBallParticipantEligibilityQuery::Evaluate(
		ThroughBallParticipantEligibilityQueryTests::MakeSkillRules(), Input);
	TestEqual(TEXT("Helper Snapshot error is returned"), Result.ErrorCode,
		EThroughBallParticipantEligibilityQueryErrorCode::InvalidHelperSnapshot);
	TestEqual(TEXT("Helper validator detail is retained"),
		Result.HelperSnapshotValidationResult.ErrorCode,
		EPlayerCardRuleSnapshotValidationErrorCode::InvalidSkillId);
	return true;
}

THROUGH_BALL_ELIGIBILITY_TEST(FThroughBallEligibility35, "35RejectsGoalkeeperHelper")
bool FThroughBallEligibility35::RunTest(const FString& Parameters)
{
	auto Input = ThroughBallParticipantEligibilityQueryTests::MakeValidInput(true);
	Input.HelperSnapshot = ThroughBallParticipantEligibilityQueryTests::MakeGoalkeeper(
		TEXT("Card.Helper.GK"));
	return ThroughBallParticipantEligibilityQueryTests::ExpectFailure(
		*this, ThroughBallParticipantEligibilityQueryTests::MakeSkillRules(), Input,
		EThroughBallParticipantEligibilityQueryErrorCode::UnsupportedHelperGoalkeeper,
		TEXT("HelperSnapshot"));
}

THROUGH_BALL_ELIGIBILITY_TEST(FThroughBallEligibility36, "36RejectsDuplicateDefendingParticipant")
bool FThroughBallEligibility36::RunTest(const FString& Parameters)
{
	auto Input = ThroughBallParticipantEligibilityQueryTests::MakeValidInput(true);
	Input.HelperSnapshot.CardId = Input.MarkerSnapshot.CardId;
	return ThroughBallParticipantEligibilityQueryTests::ExpectFailure(
		*this, ThroughBallParticipantEligibilityQueryTests::MakeSkillRules(), Input,
		EThroughBallParticipantEligibilityQueryErrorCode::DuplicateDefendingParticipant,
		TEXT("HelperSnapshot"));
}

THROUGH_BALL_ELIGIBILITY_TEST(FThroughBallEligibility37, "37AbsentHelperIgnoresEmptyCardId")
bool FThroughBallEligibility37::RunTest(const FString& Parameters)
{
	auto Input = ThroughBallParticipantEligibilityQueryTests::MakeValidInput();
	Input.HelperSnapshot.CardId = NAME_None;
	return ThroughBallParticipantEligibilityQueryTests::ExpectSuccess(
		*this, ThroughBallParticipantEligibilityQueryTests::MakeSkillRules(), Input);
}

THROUGH_BALL_ELIGIBILITY_TEST(FThroughBallEligibility38, "38AbsentHelperIgnoresInvalidSnapshot")
bool FThroughBallEligibility38::RunTest(const FString& Parameters)
{
	auto Input = ThroughBallParticipantEligibilityQueryTests::MakeValidInput();
	Input.HelperSnapshot.Attributes.Passing = 99;
	Input.HelperSnapshot.PositionTypes.Reset();
	return ThroughBallParticipantEligibilityQueryTests::ExpectSuccess(
		*this, ThroughBallParticipantEligibilityQueryTests::MakeSkillRules(), Input);
}

THROUGH_BALL_ELIGIBILITY_TEST(FThroughBallEligibility39, "39AbsentHelperIgnoresGoalkeeperFlag")
bool FThroughBallEligibility39::RunTest(const FString& Parameters)
{
	auto Input = ThroughBallParticipantEligibilityQueryTests::MakeValidInput();
	Input.HelperSnapshot = ThroughBallParticipantEligibilityQueryTests::MakeGoalkeeper(
		TEXT("Card.Helper.GK"));
	return ThroughBallParticipantEligibilityQueryTests::ExpectSuccess(
		*this, ThroughBallParticipantEligibilityQueryTests::MakeSkillRules(), Input);
}

THROUGH_BALL_ELIGIBILITY_TEST(FThroughBallEligibility40, "40AbsentHelperIgnoresDuplicateCardId")
bool FThroughBallEligibility40::RunTest(const FString& Parameters)
{
	auto Input = ThroughBallParticipantEligibilityQueryTests::MakeValidInput();
	Input.HelperSnapshot.CardId = Input.MarkerSnapshot.CardId;
	return ThroughBallParticipantEligibilityQueryTests::ExpectSuccess(
		*this, ThroughBallParticipantEligibilityQueryTests::MakeSkillRules(), Input);
}

THROUGH_BALL_ELIGIBILITY_TEST(FThroughBallEligibility41, "41SelectedSkillErrorPrecedesParticipantErrors")
bool FThroughBallEligibility41::RunTest(const FString& Parameters)
{
	auto Input = ThroughBallParticipantEligibilityQueryTests::MakeValidInput();
	Input.SelectedSkillId = NAME_None;
	Input.CarrierSnapshot.CardId = NAME_None;
	Input.RunnerSnapshot.CardId = NAME_None;
	Input.MarkerSnapshot.CardId = NAME_None;
	return ThroughBallParticipantEligibilityQueryTests::ExpectFailure(
		*this, ThroughBallParticipantEligibilityQueryTests::MakeSkillRules(), Input,
		EThroughBallParticipantEligibilityQueryErrorCode::InvalidSelectedSkillId,
		TEXT("SelectedSkillId"));
}

THROUGH_BALL_ELIGIBILITY_TEST(FThroughBallEligibility42, "42SkillLookupErrorPrecedesParticipantErrors")
bool FThroughBallEligibility42::RunTest(const FString& Parameters)
{
	auto Input = ThroughBallParticipantEligibilityQueryTests::MakeValidInput();
	Input.SelectedSkillId = TEXT("Skill.ThroughBall.Missing");
	Input.CarrierSnapshot.CardId = NAME_None;
	return ThroughBallParticipantEligibilityQueryTests::ExpectFailure(
		*this, ThroughBallParticipantEligibilityQueryTests::MakeSkillRules(), Input,
		EThroughBallParticipantEligibilityQueryErrorCode::SkillRuleLookupFailed,
		TEXT("SelectedSkillId"));
}

THROUGH_BALL_ELIGIBILITY_TEST(FThroughBallEligibility43, "43CarrierErrorPrecedesRunnerError")
bool FThroughBallEligibility43::RunTest(const FString& Parameters)
{
	auto Input = ThroughBallParticipantEligibilityQueryTests::MakeValidInput();
	Input.CarrierSnapshot.CardId = NAME_None;
	Input.RunnerSnapshot.CardId = NAME_None;
	return ThroughBallParticipantEligibilityQueryTests::ExpectFailure(
		*this, ThroughBallParticipantEligibilityQueryTests::MakeSkillRules(), Input,
		EThroughBallParticipantEligibilityQueryErrorCode::InvalidCarrierIdentity,
		TEXT("CarrierSnapshot"));
}

THROUGH_BALL_ELIGIBILITY_TEST(FThroughBallEligibility44, "44RunnerErrorPrecedesDuplicateAttacker")
bool FThroughBallEligibility44::RunTest(const FString& Parameters)
{
	auto Input = ThroughBallParticipantEligibilityQueryTests::MakeValidInput();
	Input.RunnerSnapshot.CardId = Input.CarrierSnapshot.CardId;
	Input.bIsRunnerInAttackingForwardArea = false;
	return ThroughBallParticipantEligibilityQueryTests::ExpectFailure(
		*this, ThroughBallParticipantEligibilityQueryTests::MakeSkillRules(), Input,
		EThroughBallParticipantEligibilityQueryErrorCode::RunnerNotInAttackingForwardArea,
		TEXT("bIsRunnerInAttackingForwardArea"));
}

THROUGH_BALL_ELIGIBILITY_TEST(FThroughBallEligibility45, "45MarkerErrorPrecedesHelperError")
bool FThroughBallEligibility45::RunTest(const FString& Parameters)
{
	auto Input = ThroughBallParticipantEligibilityQueryTests::MakeValidInput(true);
	Input.MarkerSnapshot.CardId = NAME_None;
	Input.HelperSnapshot.CardId = NAME_None;
	return ThroughBallParticipantEligibilityQueryTests::ExpectFailure(
		*this, ThroughBallParticipantEligibilityQueryTests::MakeSkillRules(), Input,
		EThroughBallParticipantEligibilityQueryErrorCode::InvalidMarkerIdentity,
		TEXT("MarkerSnapshot"));
}

THROUGH_BALL_ELIGIBILITY_TEST(FThroughBallEligibility46, "46HelperErrorPrecedesDuplicateDefender")
bool FThroughBallEligibility46::RunTest(const FString& Parameters)
{
	auto Input = ThroughBallParticipantEligibilityQueryTests::MakeValidInput(true);
	Input.HelperSnapshot.CardId = Input.MarkerSnapshot.CardId;
	Input.HelperSnapshot.Attributes.Strength = 0;
	return ThroughBallParticipantEligibilityQueryTests::ExpectFailure(
		*this, ThroughBallParticipantEligibilityQueryTests::MakeSkillRules(), Input,
		EThroughBallParticipantEligibilityQueryErrorCode::InvalidHelperSnapshot,
		TEXT("HelperSnapshot"));
}

THROUGH_BALL_ELIGIBILITY_TEST(FThroughBallEligibility47, "47SuccessDoesNotMutateInput")
bool FThroughBallEligibility47::RunTest(const FString& Parameters)
{
	const auto Input = ThroughBallParticipantEligibilityQueryTests::MakeValidInput(true);
	const auto InputBefore = Input;
	const auto Result = FThroughBallParticipantEligibilityQuery::Evaluate(
		ThroughBallParticipantEligibilityQueryTests::MakeSkillRules(), Input);
	TestTrue(TEXT("Eligibility succeeds"), Result.bSuccess);
	TestTrue(TEXT("Call input remains unchanged"),
		ThroughBallParticipantEligibilityQueryTests::AreInputsEqual(Input, InputBefore));
	TestTrue(TEXT("Result preserves the input value copy"),
		ThroughBallParticipantEligibilityQueryTests::AreInputsEqual(
			Result.Input, InputBefore));
	return true;
}

THROUGH_BALL_ELIGIBILITY_TEST(FThroughBallEligibility48, "48FailureDoesNotMutateInput")
bool FThroughBallEligibility48::RunTest(const FString& Parameters)
{
	auto Input = ThroughBallParticipantEligibilityQueryTests::MakeValidInput();
	Input.RunnerSnapshot.CardId = NAME_None;
	const auto InputBefore = Input;
	const auto Result = FThroughBallParticipantEligibilityQuery::Evaluate(
		ThroughBallParticipantEligibilityQueryTests::MakeSkillRules(), Input);
	TestFalse(TEXT("Eligibility fails"), Result.bSuccess);
	TestTrue(TEXT("Call input remains unchanged"),
		ThroughBallParticipantEligibilityQueryTests::AreInputsEqual(Input, InputBefore));
	TestTrue(TEXT("Failure preserves the input value copy"),
		ThroughBallParticipantEligibilityQueryTests::AreInputsEqual(
			Result.Input, InputBefore));
	return true;
}

THROUGH_BALL_ELIGIBILITY_TEST(FThroughBallEligibility49, "49DoesNotMutateSkillRuleSet")
bool FThroughBallEligibility49::RunTest(const FString& Parameters)
{
	const auto Rules = ThroughBallParticipantEligibilityQueryTests::MakeSkillRules();
	const auto RulesBefore = Rules;
	const auto Result = FThroughBallParticipantEligibilityQuery::Evaluate(
		Rules, ThroughBallParticipantEligibilityQueryTests::MakeValidInput());
	TestTrue(TEXT("Eligibility succeeds"), Result.bSuccess);
	TestTrue(TEXT("SkillRuleSet remains unchanged"),
		ThroughBallParticipantEligibilityQueryTests::AreSkillRulesEqual(
			Rules, RulesBefore));
	return true;
}

THROUGH_BALL_ELIGIBILITY_TEST(FThroughBallEligibility50, "50ProducesDeterministicResult")
bool FThroughBallEligibility50::RunTest(const FString& Parameters)
{
	const auto Rules = ThroughBallParticipantEligibilityQueryTests::MakeSkillRules();
	const auto Input = ThroughBallParticipantEligibilityQueryTests::MakeValidInput(true);
	const auto First = FThroughBallParticipantEligibilityQuery::Evaluate(Rules, Input);
	const auto Second = FThroughBallParticipantEligibilityQuery::Evaluate(Rules, Input);
	TestTrue(TEXT("Repeated evaluation returns an identical Result"),
		ThroughBallParticipantEligibilityQueryTests::AreResultsEqual(First, Second));
	return true;
}

THROUGH_BALL_ELIGIBILITY_TEST(FThroughBallEligibility51, "51SuccessRetainsNestedValidationResults")
bool FThroughBallEligibility51::RunTest(const FString& Parameters)
{
	const auto Result = FThroughBallParticipantEligibilityQuery::Evaluate(
		ThroughBallParticipantEligibilityQueryTests::MakeSkillRules(),
		ThroughBallParticipantEligibilityQueryTests::MakeValidInput(true));
	TestTrue(TEXT("Eligibility succeeds"), Result.bSuccess);
	TestTrue(TEXT("SkillRule query succeeds"), Result.SkillRuleQueryResult.bSuccess);
	TestTrue(TEXT("Carrier validation succeeds"),
		Result.CarrierSnapshotValidationResult.bSuccess);
	TestTrue(TEXT("Runner validation succeeds"),
		Result.RunnerSnapshotValidationResult.bSuccess);
	TestTrue(TEXT("Marker validation succeeds"),
		Result.MarkerSnapshotValidationResult.bSuccess);
	TestTrue(TEXT("Helper validation succeeds"),
		Result.HelperSnapshotValidationResult.bSuccess);
	return true;
}

THROUGH_BALL_ELIGIBILITY_TEST(FThroughBallEligibility52, "52AbsentHelperRetainsDefaultValidationResult")
bool FThroughBallEligibility52::RunTest(const FString& Parameters)
{
	auto Input = ThroughBallParticipantEligibilityQueryTests::MakeValidInput();
	Input.HelperSnapshot.Attributes.Shooting = 99;
	const auto Result = FThroughBallParticipantEligibilityQuery::Evaluate(
		ThroughBallParticipantEligibilityQueryTests::MakeSkillRules(), Input);
	TestTrue(TEXT("Eligibility succeeds"), Result.bSuccess);
	TestFalse(TEXT("Helper validation did not run"),
		Result.HelperSnapshotValidationResult.bSuccess);
	TestFalse(TEXT("Default Helper validation is not valid"),
		Result.HelperSnapshotValidationResult.bIsValid);
	TestEqual(TEXT("Default Helper validation has no error"),
		Result.HelperSnapshotValidationResult.ErrorCode,
		EPlayerCardRuleSnapshotValidationErrorCode::None);
	TestTrue(TEXT("Default Helper validation message is empty"),
		Result.HelperSnapshotValidationResult.ErrorMessage.IsEmpty());
	TestTrue(TEXT("Default Helper invalid CardId is empty"),
		Result.HelperSnapshotValidationResult.InvalidCardId.IsNone());
	TestTrue(TEXT("Default Helper duplicate CardIds are empty"),
		Result.HelperSnapshotValidationResult.DuplicateCardIds.IsEmpty());
	return true;
}

#undef THROUGH_BALL_ELIGIBILITY_TEST

#endif
