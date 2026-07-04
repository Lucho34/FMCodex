#if WITH_DEV_AUTOMATION_TESTS

#include "LongShotDirectShotPlanQuery.h"

#include "HAL/FileManager.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace LongShotDirectShotPlanQueryTests
{
	const FName SkillId(TEXT("Skill.LongShot.Primary"));
	const FName AttackerCardId(TEXT("Card.Attacker"));
	const FName DefenderCardId(TEXT("Card.Defender"));
	const FName AttackerPlayerId(TEXT("Player.Attacker"));
	const FName DefenderPlayerId(TEXT("Player.Defender"));

	FPlayerCardRuleSnapshot MakeOutfieldCard(
		const FName CardId,
		const EPlayerPositionType Position,
		const TArray<FName>& SkillIds = {})
	{
		FPlayerCardRuleSnapshot Card;
		Card.CardId = CardId;
		Card.PositionTypes = { Position };
		Card.SkillIds = SkillIds;
		return Card;
	}

	FPlayerCardRuleSnapshot MakeGoalkeeperCard(
		const FName CardId,
		const TArray<FName>& SkillIds = {})
	{
		FPlayerCardRuleSnapshot Card;
		Card.CardId = CardId;
		Card.PositionTypes = {
			EPlayerPositionType::Goalkeeper
		};
		Card.bIsGoalkeeper = true;
		Card.bHasGoalkeeperAttributes = true;
		Card.SkillIds = SkillIds;
		return Card;
	}

	FPlayerCardRuleSnapshotSet MakePlayerCardSnapshots()
	{
		FPlayerCardRuleSnapshotSet SnapshotSet;
		SnapshotSet.Cards = {
			MakeOutfieldCard(
				AttackerCardId,
				EPlayerPositionType::Attack,
				{ SkillId }),
			MakeOutfieldCard(
				DefenderCardId,
				EPlayerPositionType::Defense)
		};
		return SnapshotSet;
	}

	FSkillRuleSnapshotSet MakeSkillRules()
	{
		FSkillRuleSnapshot Rule;
		Rule.SkillId = SkillId;
		Rule.SkillType = ESkillRuleType::LongShot;
		Rule.MinTriggerActionPoint = 3;
		Rule.MaxTriggerActionPoint = 6;

		FSkillRuleSnapshotSet SnapshotSet;
		SnapshotSet.SkillRules = { Rule };
		return SnapshotSet;
	}

	FLongShotDirectShotPlanQueryInput MakeValidInput()
	{
		FLongShotDirectShotPlanQueryInput Input;
		Input.SkillId = SkillId;
		Input.AttackerCardId = AttackerCardId;
		Input.DefenderCardId = DefenderCardId;
		Input.CurrentActionPoint = 4;
		Input.bHasExternalAttackD6 = true;
		Input.ExternalAttackD6 = 4;
		Input.bHasExternalDefenseD6 = true;
		Input.ExternalDefenseD6 = 3;
		Input.LogId = FGuid(1, 2, 3, 4);
		Input.TurnIndex = 7;
		Input.AttackerPlayerId = AttackerPlayerId;
		Input.DefenderPlayerId = DefenderPlayerId;
		return Input;
	}

	bool AreInputsEqual(
		const FLongShotDirectShotPlanQueryInput& Left,
		const FLongShotDirectShotPlanQueryInput& Right)
	{
		return Left.SkillId == Right.SkillId
			&& Left.AttackerCardId == Right.AttackerCardId
			&& Left.DefenderCardId == Right.DefenderCardId
			&& Left.CurrentActionPoint == Right.CurrentActionPoint
			&& Left.bHasExternalAttackD6
				== Right.bHasExternalAttackD6
			&& Left.ExternalAttackD6 == Right.ExternalAttackD6
			&& Left.bHasExternalDefenseD6
				== Right.bHasExternalDefenseD6
			&& Left.ExternalDefenseD6 == Right.ExternalDefenseD6
			&& Left.LogId == Right.LogId
			&& Left.TurnIndex == Right.TurnIndex
			&& Left.AttackerPlayerId == Right.AttackerPlayerId
			&& Left.DefenderPlayerId == Right.DefenderPlayerId;
	}

	bool ArePlayerAttributesEqual(
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

	bool ArePlayerCardSnapshotsEqual(
		const FPlayerCardRuleSnapshotSet& Left,
		const FPlayerCardRuleSnapshotSet& Right)
	{
		if (Left.Cards.Num() != Right.Cards.Num())
		{
			return false;
		}

		for (int32 Index = 0; Index < Left.Cards.Num(); ++Index)
		{
			const FPlayerCardRuleSnapshot& LeftCard =
				Left.Cards[Index];
			const FPlayerCardRuleSnapshot& RightCard =
				Right.Cards[Index];
			if (LeftCard.CardId != RightCard.CardId
				|| LeftCard.PositionTypes != RightCard.PositionTypes
				|| !ArePlayerAttributesEqual(
					LeftCard.Attributes,
					RightCard.Attributes)
				|| LeftCard.bIsGoalkeeper
					!= RightCard.bIsGoalkeeper
				|| LeftCard.bHasGoalkeeperAttributes
					!= RightCard.bHasGoalkeeperAttributes
				|| !AreGoalkeeperAttributesEqual(
					LeftCard.GoalkeeperAttributes,
					RightCard.GoalkeeperAttributes)
				|| LeftCard.Rarity != RightCard.Rarity
				|| LeftCard.SkillIds != RightCard.SkillIds)
			{
				return false;
			}
		}
		return true;
	}

	bool AreSkillRuleSnapshotsEqual(
		const FSkillRuleSnapshotSet& Left,
		const FSkillRuleSnapshotSet& Right)
	{
		if (Left.SkillRules.Num() != Right.SkillRules.Num())
		{
			return false;
		}

		for (int32 Index = 0; Index < Left.SkillRules.Num(); ++Index)
		{
			const FSkillRuleSnapshot& LeftRule =
				Left.SkillRules[Index];
			const FSkillRuleSnapshot& RightRule =
				Right.SkillRules[Index];
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
}

#define LONG_SHOT_PLAN_TEST(ClassName, TestName) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
		ClassName, \
		"FMCodex.CoreRules.LongShotDirectShotPlanQuery." TestName, \
		EAutomationTestFlags::EditorContext \
			| EAutomationTestFlags::EngineFilter)

LONG_SHOT_PLAN_TEST(
	FLongShotPlanRejectsEmptySkillIdTest,
	"RejectsEmptySkillId")

bool FLongShotPlanRejectsEmptySkillIdTest::RunTest(
	const FString& Parameters)
{
	FLongShotDirectShotPlanQueryInput Input =
		LongShotDirectShotPlanQueryTests::MakeValidInput();
	Input.SkillId = NAME_None;
	const FLongShotDirectShotPlanQueryResult Result =
		FLongShotDirectShotPlanQuery::BuildPlan(
			LongShotDirectShotPlanQueryTests::MakePlayerCardSnapshots(),
			LongShotDirectShotPlanQueryTests::MakeSkillRules(),
			Input);
	TestFalse(TEXT("Empty SkillId fails"), Result.bSuccess);
	TestEqual(
		TEXT("Empty SkillId error"),
		Result.ErrorCode,
		ELongShotDirectShotPlanQueryErrorCode::MissingSkillId);
	return true;
}

LONG_SHOT_PLAN_TEST(
	FLongShotPlanRejectsMissingAttackerSnapshotTest,
	"RejectsMissingAttackerSnapshot")

bool FLongShotPlanRejectsMissingAttackerSnapshotTest::RunTest(
	const FString& Parameters)
{
	FLongShotDirectShotPlanQueryInput Input =
		LongShotDirectShotPlanQueryTests::MakeValidInput();
	Input.AttackerCardId = TEXT("Card.Missing");
	const FLongShotDirectShotPlanQueryResult Result =
		FLongShotDirectShotPlanQuery::BuildPlan(
			LongShotDirectShotPlanQueryTests::MakePlayerCardSnapshots(),
			LongShotDirectShotPlanQueryTests::MakeSkillRules(),
			Input);
	TestEqual(
		TEXT("Missing attacker error"),
		Result.ErrorCode,
		ELongShotDirectShotPlanQueryErrorCode
			::AttackerSnapshotQueryFailed);
	TestFalse(
		TEXT("Attacker nested query failed"),
		Result.AttackerSnapshotQueryResult.bSuccess);
	return true;
}

LONG_SHOT_PLAN_TEST(
	FLongShotPlanRejectsMissingDefenderSnapshotTest,
	"RejectsMissingDefenderSnapshot")

bool FLongShotPlanRejectsMissingDefenderSnapshotTest::RunTest(
	const FString& Parameters)
{
	FLongShotDirectShotPlanQueryInput Input =
		LongShotDirectShotPlanQueryTests::MakeValidInput();
	Input.DefenderCardId = TEXT("Card.Missing");
	const FLongShotDirectShotPlanQueryResult Result =
		FLongShotDirectShotPlanQuery::BuildPlan(
			LongShotDirectShotPlanQueryTests::MakePlayerCardSnapshots(),
			LongShotDirectShotPlanQueryTests::MakeSkillRules(),
			Input);
	TestTrue(
		TEXT("Attacker nested query succeeded"),
		Result.AttackerSnapshotQueryResult.bSuccess);
	TestEqual(
		TEXT("Missing defender error"),
		Result.ErrorCode,
		ELongShotDirectShotPlanQueryErrorCode
			::DefenderSnapshotQueryFailed);
	return true;
}

LONG_SHOT_PLAN_TEST(
	FLongShotPlanRejectsSkillRuleNotFoundTest,
	"RejectsSkillRuleNotFound")

bool FLongShotPlanRejectsSkillRuleNotFoundTest::RunTest(
	const FString& Parameters)
{
	FPlayerCardRuleSnapshotSet PlayerCards =
		LongShotDirectShotPlanQueryTests::MakePlayerCardSnapshots();
	const FName MissingSkillId(TEXT("Skill.LongShot.Missing"));
	PlayerCards.Cards[0].SkillIds = { MissingSkillId };
	FLongShotDirectShotPlanQueryInput Input =
		LongShotDirectShotPlanQueryTests::MakeValidInput();
	Input.SkillId = MissingSkillId;
	const FLongShotDirectShotPlanQueryResult Result =
		FLongShotDirectShotPlanQuery::BuildPlan(
			PlayerCards,
			LongShotDirectShotPlanQueryTests::MakeSkillRules(),
			Input);
	TestEqual(
		TEXT("Missing skill rule error"),
		Result.ErrorCode,
		ELongShotDirectShotPlanQueryErrorCode::SkillRuleQueryFailed);
	TestEqual(
		TEXT("Nested query reports not found"),
		Result.SkillRuleQueryResult.ErrorCode,
		ESkillRuleSnapshotQueryErrorCode::SkillRuleNotFound);
	return true;
}

LONG_SHOT_PLAN_TEST(
	FLongShotPlanRejectsSkillNotOwnedTest,
	"RejectsSkillNotOwnedByAttacker")

bool FLongShotPlanRejectsSkillNotOwnedTest::RunTest(
	const FString& Parameters)
{
	FPlayerCardRuleSnapshotSet PlayerCards =
		LongShotDirectShotPlanQueryTests::MakePlayerCardSnapshots();
	PlayerCards.Cards[0].SkillIds.Reset();
	const FLongShotDirectShotPlanQueryResult Result =
		FLongShotDirectShotPlanQuery::BuildPlan(
			PlayerCards,
			LongShotDirectShotPlanQueryTests::MakeSkillRules(),
			LongShotDirectShotPlanQueryTests::MakeValidInput());
	TestEqual(
		TEXT("Unowned skill error"),
		Result.ErrorCode,
		ELongShotDirectShotPlanQueryErrorCode
			::SkillNotOwnedByAttacker);
	return true;
}

LONG_SHOT_PLAN_TEST(
	FLongShotPlanRejectsSkillTypeMismatchTest,
	"RejectsSkillTypeMismatch")

bool FLongShotPlanRejectsSkillTypeMismatchTest::RunTest(
	const FString& Parameters)
{
	FSkillRuleSnapshotSet SkillRules =
		LongShotDirectShotPlanQueryTests::MakeSkillRules();
	SkillRules.SkillRules[0].SkillType = ESkillRuleType::None;
	const FLongShotDirectShotPlanQueryResult Result =
		FLongShotDirectShotPlanQuery::BuildPlan(
			LongShotDirectShotPlanQueryTests::MakePlayerCardSnapshots(),
			SkillRules,
			LongShotDirectShotPlanQueryTests::MakeValidInput());
	TestEqual(
		TEXT("Type mismatch error"),
		Result.ErrorCode,
		ELongShotDirectShotPlanQueryErrorCode::SkillTypeMismatch);
	TestEqual(
		TEXT("Nested validator diagnosis retained"),
		Result.SkillRuleQueryResult.ValidationResult.ErrorCode,
		ESkillRuleSnapshotValidationErrorCode::InvalidSkillType);
	return true;
}

LONG_SHOT_PLAN_TEST(
	FLongShotPlanRejectsInvalidActionPointTest,
	"RejectsInvalidActionPoint")

bool FLongShotPlanRejectsInvalidActionPointTest::RunTest(
	const FString& Parameters)
{
	FLongShotDirectShotPlanQueryInput Input =
		LongShotDirectShotPlanQueryTests::MakeValidInput();
	Input.CurrentActionPoint = 1;
	const FLongShotDirectShotPlanQueryResult Result =
		FLongShotDirectShotPlanQuery::BuildPlan(
			LongShotDirectShotPlanQueryTests::MakePlayerCardSnapshots(),
			LongShotDirectShotPlanQueryTests::MakeSkillRules(),
			Input);
	TestEqual(
		TEXT("Invalid action point error"),
		Result.ErrorCode,
		ELongShotDirectShotPlanQueryErrorCode::InvalidActionPoint);
	return true;
}

LONG_SHOT_PLAN_TEST(
	FLongShotPlanRejectsActionPointOutOfRangeTest,
	"RejectsActionPointOutOfRange")

bool FLongShotPlanRejectsActionPointOutOfRangeTest::RunTest(
	const FString& Parameters)
{
	FLongShotDirectShotPlanQueryInput Input =
		LongShotDirectShotPlanQueryTests::MakeValidInput();
	Input.CurrentActionPoint = 2;
	const FLongShotDirectShotPlanQueryResult Result =
		FLongShotDirectShotPlanQuery::BuildPlan(
			LongShotDirectShotPlanQueryTests::MakePlayerCardSnapshots(),
			LongShotDirectShotPlanQueryTests::MakeSkillRules(),
			Input);
	TestEqual(
		TEXT("Skill trigger range error"),
		Result.ErrorCode,
		ELongShotDirectShotPlanQueryErrorCode::ActionPointOutOfRange);
	return true;
}

LONG_SHOT_PLAN_TEST(
	FLongShotPlanRejectsGoalkeeperAttackerTest,
	"RejectsGoalkeeperAttacker")

bool FLongShotPlanRejectsGoalkeeperAttackerTest::RunTest(
	const FString& Parameters)
{
	FPlayerCardRuleSnapshotSet PlayerCards =
		LongShotDirectShotPlanQueryTests::MakePlayerCardSnapshots();
	PlayerCards.Cards[0] =
		LongShotDirectShotPlanQueryTests::MakeGoalkeeperCard(
			LongShotDirectShotPlanQueryTests::AttackerCardId,
			{ LongShotDirectShotPlanQueryTests::SkillId });
	const FLongShotDirectShotPlanQueryResult Result =
		FLongShotDirectShotPlanQuery::BuildPlan(
			PlayerCards,
			LongShotDirectShotPlanQueryTests::MakeSkillRules(),
			LongShotDirectShotPlanQueryTests::MakeValidInput());
	TestEqual(
		TEXT("Goalkeeper attacker rejected"),
		Result.ErrorCode,
		ELongShotDirectShotPlanQueryErrorCode
			::UnsupportedGoalkeeperParticipant);
	TestEqual(
		TEXT("Attacker field reported"),
		Result.InvalidField,
		FName(TEXT("AttackerCardId")));
	return true;
}

LONG_SHOT_PLAN_TEST(
	FLongShotPlanRejectsGoalkeeperDefenderTest,
	"RejectsGoalkeeperDefender")

bool FLongShotPlanRejectsGoalkeeperDefenderTest::RunTest(
	const FString& Parameters)
{
	FPlayerCardRuleSnapshotSet PlayerCards =
		LongShotDirectShotPlanQueryTests::MakePlayerCardSnapshots();
	PlayerCards.Cards[1] =
		LongShotDirectShotPlanQueryTests::MakeGoalkeeperCard(
			LongShotDirectShotPlanQueryTests::DefenderCardId);
	const FLongShotDirectShotPlanQueryResult Result =
		FLongShotDirectShotPlanQuery::BuildPlan(
			PlayerCards,
			LongShotDirectShotPlanQueryTests::MakeSkillRules(),
			LongShotDirectShotPlanQueryTests::MakeValidInput());
	TestEqual(
		TEXT("Goalkeeper defender rejected"),
		Result.ErrorCode,
		ELongShotDirectShotPlanQueryErrorCode
			::UnsupportedGoalkeeperParticipant);
	TestEqual(
		TEXT("Defender field reported"),
		Result.InvalidField,
		FName(TEXT("DefenderCardId")));
	return true;
}

LONG_SHOT_PLAN_TEST(
	FLongShotPlanRejectsMissingAttackD6Test,
	"RejectsMissingExternalAttackD6")

bool FLongShotPlanRejectsMissingAttackD6Test::RunTest(
	const FString& Parameters)
{
	FLongShotDirectShotPlanQueryInput Input =
		LongShotDirectShotPlanQueryTests::MakeValidInput();
	Input.bHasExternalAttackD6 = false;
	const FLongShotDirectShotPlanQueryResult Result =
		FLongShotDirectShotPlanQuery::BuildPlan(
			LongShotDirectShotPlanQueryTests::MakePlayerCardSnapshots(),
			LongShotDirectShotPlanQueryTests::MakeSkillRules(),
			Input);
	TestEqual(
		TEXT("Missing attack D6 error"),
		Result.ErrorCode,
		ELongShotDirectShotPlanQueryErrorCode
			::MissingExternalAttackD6);
	return true;
}

LONG_SHOT_PLAN_TEST(
	FLongShotPlanRejectsInvalidAttackD6Test,
	"RejectsInvalidAttackD6")

bool FLongShotPlanRejectsInvalidAttackD6Test::RunTest(
	const FString& Parameters)
{
	FLongShotDirectShotPlanQueryInput Input =
		LongShotDirectShotPlanQueryTests::MakeValidInput();
	Input.ExternalAttackD6 = 7;
	const FLongShotDirectShotPlanQueryResult Result =
		FLongShotDirectShotPlanQuery::BuildPlan(
			LongShotDirectShotPlanQueryTests::MakePlayerCardSnapshots(),
			LongShotDirectShotPlanQueryTests::MakeSkillRules(),
			Input);
	TestEqual(
		TEXT("Invalid attack D6 error"),
		Result.ErrorCode,
		ELongShotDirectShotPlanQueryErrorCode::InvalidAttackD6);
	return true;
}

LONG_SHOT_PLAN_TEST(
	FLongShotPlanAttackD6OneImmediateMissTest,
	"AttackD6OneReturnsImmediateMiss")

bool FLongShotPlanAttackD6OneImmediateMissTest::RunTest(
	const FString& Parameters)
{
	FLongShotDirectShotPlanQueryInput Input =
		LongShotDirectShotPlanQueryTests::MakeValidInput();
	Input.ExternalAttackD6 = 1;
	const FLongShotDirectShotPlanQueryResult Result =
		FLongShotDirectShotPlanQuery::BuildPlan(
			LongShotDirectShotPlanQueryTests::MakePlayerCardSnapshots(),
			LongShotDirectShotPlanQueryTests::MakeSkillRules(),
			Input);
	TestTrue(TEXT("D6 1 succeeds"), Result.bSuccess);
	TestEqual(
		TEXT("D6 1 immediate miss"),
		Result.Decision,
		ELongShotDirectShotDecision::ImmediateMiss);
	TestTrue(TEXT("Immediate miss ends attack"), Result.bAttackEnded);
	TestFalse(TEXT("Immediate miss is not goal"), Result.bIsGoal);
	return true;
}

LONG_SHOT_PLAN_TEST(
	FLongShotPlanAttackD6TwoImmediateMissTest,
	"AttackD6TwoReturnsImmediateMiss")

bool FLongShotPlanAttackD6TwoImmediateMissTest::RunTest(
	const FString& Parameters)
{
	FLongShotDirectShotPlanQueryInput Input =
		LongShotDirectShotPlanQueryTests::MakeValidInput();
	Input.ExternalAttackD6 = 2;
	const FLongShotDirectShotPlanQueryResult Result =
		FLongShotDirectShotPlanQuery::BuildPlan(
			LongShotDirectShotPlanQueryTests::MakePlayerCardSnapshots(),
			LongShotDirectShotPlanQueryTests::MakeSkillRules(),
			Input);
	TestTrue(TEXT("D6 2 succeeds"), Result.bSuccess);
	TestEqual(
		TEXT("D6 2 immediate miss"),
		Result.Decision,
		ELongShotDirectShotDecision::ImmediateMiss);
	return true;
}

LONG_SHOT_PLAN_TEST(
	FLongShotPlanImmediateMissNoDefenseD6Test,
	"ImmediateMissDoesNotRequireDefenseD6")

bool FLongShotPlanImmediateMissNoDefenseD6Test::RunTest(
	const FString& Parameters)
{
	FLongShotDirectShotPlanQueryInput Input =
		LongShotDirectShotPlanQueryTests::MakeValidInput();
	Input.ExternalAttackD6 = 1;
	Input.bHasExternalDefenseD6 = false;
	Input.ExternalDefenseD6 = 0;
	const FLongShotDirectShotPlanQueryResult Result =
		FLongShotDirectShotPlanQuery::BuildPlan(
			LongShotDirectShotPlanQueryTests::MakePlayerCardSnapshots(),
			LongShotDirectShotPlanQueryTests::MakeSkillRules(),
			Input);
	TestTrue(TEXT("Immediate miss needs no defense D6"), Result.bSuccess);
	TestEqual(
		TEXT("Decision remains immediate miss"),
		Result.Decision,
		ELongShotDirectShotDecision::ImmediateMiss);
	return true;
}

LONG_SHOT_PLAN_TEST(
	FLongShotPlanImmediateMissNoFormulaPlanTest,
	"ImmediateMissDoesNotGenerateFormulaPlan")

bool FLongShotPlanImmediateMissNoFormulaPlanTest::RunTest(
	const FString& Parameters)
{
	FLongShotDirectShotPlanQueryInput Input =
		LongShotDirectShotPlanQueryTests::MakeValidInput();
	Input.ExternalAttackD6 = 2;
	const FLongShotDirectShotPlanQueryResult Result =
		FLongShotDirectShotPlanQuery::BuildPlan(
			LongShotDirectShotPlanQueryTests::MakePlayerCardSnapshots(),
			LongShotDirectShotPlanQueryTests::MakeSkillRules(),
			Input);
	TestFalse(TEXT("No formula plan flag"), Result.bHasFormulaPlan);
	TestEqual(
		TEXT("Default attacker formula type"),
		Result.FormulaPlan.AttackerQueryInput.FormulaType,
		EFormulaType::None);
	return true;
}

LONG_SHOT_PLAN_TEST(
	FLongShotPlanRejectsMissingDefenseD6Test,
	"RejectsMissingDefenseD6WhenFormulaRequired")

bool FLongShotPlanRejectsMissingDefenseD6Test::RunTest(
	const FString& Parameters)
{
	FLongShotDirectShotPlanQueryInput Input =
		LongShotDirectShotPlanQueryTests::MakeValidInput();
	Input.bHasExternalDefenseD6 = false;
	const FLongShotDirectShotPlanQueryResult Result =
		FLongShotDirectShotPlanQuery::BuildPlan(
			LongShotDirectShotPlanQueryTests::MakePlayerCardSnapshots(),
			LongShotDirectShotPlanQueryTests::MakeSkillRules(),
			Input);
	TestEqual(
		TEXT("Missing defense D6 error"),
		Result.ErrorCode,
		ELongShotDirectShotPlanQueryErrorCode
			::MissingExternalDefenseD6);
	TestFalse(TEXT("No plan on failure"), Result.bHasFormulaPlan);
	return true;
}

LONG_SHOT_PLAN_TEST(
	FLongShotPlanRejectsInvalidDefenseD6Test,
	"RejectsInvalidDefenseD6WhenFormulaRequired")

bool FLongShotPlanRejectsInvalidDefenseD6Test::RunTest(
	const FString& Parameters)
{
	FLongShotDirectShotPlanQueryInput Input =
		LongShotDirectShotPlanQueryTests::MakeValidInput();
	Input.ExternalDefenseD6 = 0;
	const FLongShotDirectShotPlanQueryResult Result =
		FLongShotDirectShotPlanQuery::BuildPlan(
			LongShotDirectShotPlanQueryTests::MakePlayerCardSnapshots(),
			LongShotDirectShotPlanQueryTests::MakeSkillRules(),
			Input);
	TestEqual(
		TEXT("Invalid defense D6 error"),
		Result.ErrorCode,
		ELongShotDirectShotPlanQueryErrorCode
			::InvalidDefenseD6);
	return true;
}

LONG_SHOT_PLAN_TEST(
	FLongShotPlanAttackD6ThreeToSixTest,
	"AttackD6ThreeToSixGeneratesFormulaPlan")

bool FLongShotPlanAttackD6ThreeToSixTest::RunTest(
	const FString& Parameters)
{
	for (int32 AttackD6 = 3; AttackD6 <= 6; ++AttackD6)
	{
		FLongShotDirectShotPlanQueryInput Input =
			LongShotDirectShotPlanQueryTests::MakeValidInput();
		Input.ExternalAttackD6 = AttackD6;
		const FLongShotDirectShotPlanQueryResult Result =
			FLongShotDirectShotPlanQuery::BuildPlan(
				LongShotDirectShotPlanQueryTests
					::MakePlayerCardSnapshots(),
				LongShotDirectShotPlanQueryTests::MakeSkillRules(),
				Input);
		TestTrue(
			*FString::Printf(TEXT("D6 %d succeeds"), AttackD6),
			Result.bSuccess);
		TestEqual(
			*FString::Printf(
				TEXT("D6 %d needs formula"),
				AttackD6),
			Result.Decision,
			ELongShotDirectShotDecision
				::FormulaResolutionRequired);
		TestTrue(
			*FString::Printf(TEXT("D6 %d has plan"), AttackD6),
			Result.bHasFormulaPlan);
		TestFalse(
			TEXT("Formula plan has not ended attack"),
			Result.bAttackEnded);
		TestFalse(TEXT("Formula plan is not yet a goal"), Result.bIsGoal);
	}
	return true;
}

LONG_SHOT_PLAN_TEST(
	FLongShotPlanFormulaMappingTest,
	"FormulaPlanUsesFinishingLongShotTacklingZeroAndPlusTwo")

bool FLongShotPlanFormulaMappingTest::RunTest(
	const FString& Parameters)
{
	const FLongShotDirectShotPlanQueryResult Result =
		FLongShotDirectShotPlanQuery::BuildPlan(
			LongShotDirectShotPlanQueryTests::MakePlayerCardSnapshots(),
			LongShotDirectShotPlanQueryTests::MakeSkillRules(),
			LongShotDirectShotPlanQueryTests::MakeValidInput());
	const FSingleCardFormulaInputAssemblyQueryInput& Attacker =
		Result.FormulaPlan.AttackerQueryInput;
	const FSingleCardFormulaInputAssemblyQueryInput& Defender =
		Result.FormulaPlan.DefenderQueryInput;
	TestEqual(TEXT("Attacker finishing"), Attacker.FormulaType, EFormulaType::Finishing);
	TestEqual(TEXT("Defender finishing"), Defender.FormulaType, EFormulaType::Finishing);
	TestEqual(TEXT("Attacker role"), Attacker.ParticipantRole, ESingleCardFormulaParticipantRole::Attacker);
	TestEqual(TEXT("Defender role"), Defender.ParticipantRole, ESingleCardFormulaParticipantRole::Defender);
	TestEqual(TEXT("Attacker LongShot"), Attacker.Attribute, ESingleCardFormulaAttribute::LongShot);
	TestEqual(TEXT("Defender Tackling"), Defender.Attribute, ESingleCardFormulaAttribute::Tackling);
	TestEqual(TEXT("Attacker modifier"), Attacker.ExternalModifier, 0.0f);
	TestEqual(TEXT("Defender modifier"), Defender.ExternalModifier, 2.0f);
	return true;
}

LONG_SHOT_PLAN_TEST(
	FLongShotPlanExternalD6MappingTest,
	"FormulaPlanPreservesExternalD6AndSourceFlags")

bool FLongShotPlanExternalD6MappingTest::RunTest(
	const FString& Parameters)
{
	FLongShotDirectShotPlanQueryInput Input =
		LongShotDirectShotPlanQueryTests::MakeValidInput();
	Input.ExternalAttackD6 = 5;
	Input.ExternalDefenseD6 = 2;
	const FLongShotDirectShotPlanQueryResult Result =
		FLongShotDirectShotPlanQuery::BuildPlan(
			LongShotDirectShotPlanQueryTests::MakePlayerCardSnapshots(),
			LongShotDirectShotPlanQueryTests::MakeSkillRules(),
			Input);
	TestTrue(TEXT("Attacker D6 source flag"), Result.FormulaPlan.AttackerQueryInput.bHasExternalD6ComparePoint);
	TestEqual(TEXT("Attacker D6"), Result.FormulaPlan.AttackerQueryInput.ExternalD6ComparePoint, 5);
	TestTrue(TEXT("Defender D6 source flag"), Result.FormulaPlan.DefenderQueryInput.bHasExternalD6ComparePoint);
	TestEqual(TEXT("Defender D6"), Result.FormulaPlan.DefenderQueryInput.ExternalD6ComparePoint, 2);
	TestTrue(TEXT("Attacker modifier source flag"), Result.FormulaPlan.AttackerQueryInput.bHasExternalModifier);
	TestTrue(TEXT("Defender modifier source flag"), Result.FormulaPlan.DefenderQueryInput.bHasExternalModifier);
	return true;
}

LONG_SHOT_PLAN_TEST(
	FLongShotPlanContextMappingTest,
	"FormulaPlanPreservesLogTurnPlayerAndCardIds")

bool FLongShotPlanContextMappingTest::RunTest(
	const FString& Parameters)
{
	const FLongShotDirectShotPlanQueryInput Input =
		LongShotDirectShotPlanQueryTests::MakeValidInput();
	const FLongShotDirectShotPlanQueryResult Result =
		FLongShotDirectShotPlanQuery::BuildPlan(
			LongShotDirectShotPlanQueryTests::MakePlayerCardSnapshots(),
			LongShotDirectShotPlanQueryTests::MakeSkillRules(),
			Input);
	TestEqual(TEXT("Attacker CardId"), Result.FormulaPlan.AttackerQueryInput.CardId, Input.AttackerCardId);
	TestEqual(TEXT("Defender CardId"), Result.FormulaPlan.DefenderQueryInput.CardId, Input.DefenderCardId);
	TestEqual(TEXT("Attacker LogId"), Result.FormulaPlan.AttackerQueryInput.LogId, Input.LogId);
	TestEqual(TEXT("Defender LogId"), Result.FormulaPlan.DefenderQueryInput.LogId, Input.LogId);
	TestEqual(TEXT("Attacker turn"), Result.FormulaPlan.AttackerQueryInput.TurnIndex, Input.TurnIndex);
	TestEqual(TEXT("Defender turn"), Result.FormulaPlan.DefenderQueryInput.TurnIndex, Input.TurnIndex);
	TestEqual(TEXT("Attacker PlayerId"), Result.FormulaPlan.AttackerPlayerId, Input.AttackerPlayerId);
	TestEqual(TEXT("Defender PlayerId"), Result.FormulaPlan.DefenderPlayerId, Input.DefenderPlayerId);
	return true;
}

LONG_SHOT_PLAN_TEST(
	FLongShotPlanDoesNotMutateInputTest,
	"DoesNotMutateInput")

bool FLongShotPlanDoesNotMutateInputTest::RunTest(
	const FString& Parameters)
{
	FLongShotDirectShotPlanQueryInput Input =
		LongShotDirectShotPlanQueryTests::MakeValidInput();
	const FLongShotDirectShotPlanQueryInput Before = Input;
	const FLongShotDirectShotPlanQueryResult Result =
		FLongShotDirectShotPlanQuery::BuildPlan(
			LongShotDirectShotPlanQueryTests::MakePlayerCardSnapshots(),
			LongShotDirectShotPlanQueryTests::MakeSkillRules(),
			Input);
	TestTrue(
		TEXT("Input remains unchanged"),
		LongShotDirectShotPlanQueryTests::AreInputsEqual(Input, Before));
	TestTrue(
		TEXT("Result preserves input copy"),
		LongShotDirectShotPlanQueryTests::AreInputsEqual(
			Result.Input,
			Before));
	return true;
}

LONG_SHOT_PLAN_TEST(
	FLongShotPlanDoesNotMutatePlayerSnapshotsTest,
	"DoesNotMutatePlayerCardSnapshotSet")

bool FLongShotPlanDoesNotMutatePlayerSnapshotsTest::RunTest(
	const FString& Parameters)
{
	FPlayerCardRuleSnapshotSet SnapshotSet =
		LongShotDirectShotPlanQueryTests::MakePlayerCardSnapshots();
	const FPlayerCardRuleSnapshotSet Before = SnapshotSet;
	FLongShotDirectShotPlanQuery::BuildPlan(
		SnapshotSet,
		LongShotDirectShotPlanQueryTests::MakeSkillRules(),
		LongShotDirectShotPlanQueryTests::MakeValidInput());
	TestTrue(
		TEXT("Player snapshots remain unchanged"),
		LongShotDirectShotPlanQueryTests::ArePlayerCardSnapshotsEqual(
			SnapshotSet,
			Before));
	return true;
}

LONG_SHOT_PLAN_TEST(
	FLongShotPlanDoesNotMutateSkillRulesTest,
	"DoesNotMutateSkillRuleSnapshotSet")

bool FLongShotPlanDoesNotMutateSkillRulesTest::RunTest(
	const FString& Parameters)
{
	FSkillRuleSnapshotSet SnapshotSet =
		LongShotDirectShotPlanQueryTests::MakeSkillRules();
	const FSkillRuleSnapshotSet Before = SnapshotSet;
	FLongShotDirectShotPlanQuery::BuildPlan(
		LongShotDirectShotPlanQueryTests::MakePlayerCardSnapshots(),
		SnapshotSet,
		LongShotDirectShotPlanQueryTests::MakeValidInput());
	TestTrue(
		TEXT("Skill rules remain unchanged"),
		LongShotDirectShotPlanQueryTests::AreSkillRuleSnapshotsEqual(
			SnapshotSet,
			Before));
	return true;
}

LONG_SHOT_PLAN_TEST(
	FLongShotPlanRejectsInvalidLogContextTest,
	"RejectsInvalidLogContext")

bool FLongShotPlanRejectsInvalidLogContextTest::RunTest(
	const FString& Parameters)
{
	FLongShotDirectShotPlanQueryInput Input =
		LongShotDirectShotPlanQueryTests::MakeValidInput();
	Input.LogId.Invalidate();
	const FLongShotDirectShotPlanQueryResult Result =
		FLongShotDirectShotPlanQuery::BuildPlan(
			LongShotDirectShotPlanQueryTests::MakePlayerCardSnapshots(),
			LongShotDirectShotPlanQueryTests::MakeSkillRules(),
			Input);
	TestEqual(
		TEXT("Invalid log context error"),
		Result.ErrorCode,
		ELongShotDirectShotPlanQueryErrorCode::InvalidLogContext);
	TestEqual(
		TEXT("Invalid LogId field"),
		Result.InvalidField,
		FName(TEXT("LogId")));
	return true;
}

LONG_SHOT_PLAN_TEST(
	FLongShotPlanHasNoForbiddenDependenciesTest,
	"HasNoForbiddenDependencies")

bool FLongShotPlanHasNoForbiddenDependenciesTest::RunTest(
	const FString& Parameters)
{
	const FString SourceDirectory = FPaths::Combine(
		FPaths::ProjectDir(),
		TEXT("Source/FMCodex/CoreRules"));
	FString HeaderSource;
	FString ImplementationSource;
	const bool bHeaderLoaded = FFileHelper::LoadFileToString(
		HeaderSource,
		*FPaths::Combine(
			SourceDirectory,
			TEXT("LongShotDirectShotPlanQuery.h")));
	const bool bImplementationLoaded = FFileHelper::LoadFileToString(
		ImplementationSource,
		*FPaths::Combine(
			SourceDirectory,
			TEXT("LongShotDirectShotPlanQuery.cpp")));
	TestTrue(TEXT("Plan Query header loads"), bHeaderLoaded);
	TestTrue(TEXT("Plan Query implementation loads"), bImplementationLoaded);
	if (!bHeaderLoaded || !bImplementationLoaded)
	{
		return false;
	}

	const FString CombinedSource = HeaderSource + ImplementationSource;
	TestFalse(
		TEXT("Does not execute Input Assembly Query"),
		CombinedSource.Contains(
			TEXT("FSingleCardFormulaInputAssemblyQuery::Assemble")));
	TestFalse(
		TEXT("Does not reference Resolver Input Assembler"),
		CombinedSource.Contains(
			TEXT("SingleCardFormulaResolverInputAssembler")));
	TestFalse(
		TEXT("Does not reference Executor"),
		CombinedSource.Contains(
			TEXT("SingleCardFormulaResolutionExecutor")));
	TestFalse(
		TEXT("Does not reference FormulaResolver"),
		CombinedSource.Contains(TEXT("FormulaResolver")));
	TestFalse(
		TEXT("Does not reference FormulaAttackFlow"),
		CombinedSource.Contains(TEXT("FormulaAttackFlow")));
	TestFalse(
		TEXT("Does not reference MatchPlay"),
		CombinedSource.Contains(TEXT("MatchPlay")));
	TestFalse(
		TEXT("Does not reference external data sources"),
		CombinedSource.Contains(TEXT("DataTable"))
			|| CombinedSource.Contains(TEXT("Provider"))
			|| CombinedSource.Contains(TEXT("CardDatabase")));
	TestFalse(
		TEXT("Does not generate random values"),
		CombinedSource.Contains(TEXT("FMath::Rand"))
			|| CombinedSource.Contains(TEXT("FRandomStream")));
	TestFalse(
		TEXT("Does not introduce a skill framework"),
		CombinedSource.Contains(TEXT("SkillEffect"))
			|| CombinedSource.Contains(TEXT("SkillPipeline")));
	return true;
}

#undef LONG_SHOT_PLAN_TEST

#endif
