#if WITH_DEV_AUTOMATION_TESTS

#include "CutInsideShotDirectShotPlanQuery.h"

#include "HAL/FileManager.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace CutInsideShotDirectShotPlanQueryTests
{
	const FName SkillId(TEXT("Skill.CutInsideShot.Primary"));
	const FName AttackerCardId(TEXT("Card.CutInside.Attacker"));
	const FName DefenderCardId(TEXT("Card.CutInside.Defender"));
	const FName AttackerPlayerId(TEXT("Player.CutInside.Attacker"));
	const FName DefenderPlayerId(TEXT("Player.CutInside.Defender"));

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

	FPlayerCardRuleSnapshotSet MakePlayerCardSnapshots(
		const int32 Shooting = 4,
		const int32 Dribbling = 6,
		const int32 Tackling = 5)
	{
		FPlayerCardRuleSnapshot Attacker = MakeOutfieldCard(
			AttackerCardId,
			EPlayerPositionType::Attack,
			{ SkillId });
		Attacker.Attributes.Shooting = Shooting;
		Attacker.Attributes.Dribbling = Dribbling;

		FPlayerCardRuleSnapshot Defender = MakeOutfieldCard(
			DefenderCardId,
			EPlayerPositionType::Defense);
		Defender.Attributes.Tackling = Tackling;

		FPlayerCardRuleSnapshotSet SnapshotSet;
		SnapshotSet.Cards = { Attacker, Defender };
		return SnapshotSet;
	}

	FSkillRuleSnapshotSet MakeSkillRules(
		const ESkillRuleType SkillType = ESkillRuleType::CutInsideShot)
	{
		FSkillRuleSnapshot Rule;
		Rule.SkillId = SkillId;
		Rule.SkillType = SkillType;
		Rule.MinTriggerActionPoint = 3;
		Rule.MaxTriggerActionPoint = 6;

		FSkillRuleSnapshotSet SnapshotSet;
		SnapshotSet.SkillRules = { Rule };
		return SnapshotSet;
	}

	FCutInsideShotDirectShotPlanQueryInput MakeValidInput()
	{
		FCutInsideShotDirectShotPlanQueryInput Input;
		Input.SkillId = SkillId;
		Input.AttackerCardId = AttackerCardId;
		Input.DefenderCardId = DefenderCardId;
		Input.CurrentActionPoint = 4;
		Input.bHasExternalAttackD6 = true;
		Input.ExternalAttackD6 = 4;
		Input.bHasExternalDefenseD6 = true;
		Input.ExternalDefenseD6 = 3;
		Input.LogId = FGuid(10, 20, 30, 40);
		Input.TurnIndex = 9;
		Input.AttackerPlayerId = AttackerPlayerId;
		Input.DefenderPlayerId = DefenderPlayerId;
		return Input;
	}

	bool AreInputsEqual(
		const FCutInsideShotDirectShotPlanQueryInput& Left,
		const FCutInsideShotDirectShotPlanQueryInput& Right)
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

#define CUT_INSIDE_DIRECT_SHOT_PLAN_TEST(ClassName, TestName) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
		ClassName, \
		"FMCodex.CoreRules.CutInsideShotDirectShotPlanQuery." TestName, \
		EAutomationTestFlags::EditorContext \
			| EAutomationTestFlags::EngineFilter)

CUT_INSIDE_DIRECT_SHOT_PLAN_TEST(
	FCutInsideShotPlanAttackD6OneImmediateMissTest,
	"AttackD6OneReturnsImmediateMiss")

bool FCutInsideShotPlanAttackD6OneImmediateMissTest::RunTest(
	const FString& Parameters)
{
	FCutInsideShotDirectShotPlanQueryInput Input =
		CutInsideShotDirectShotPlanQueryTests::MakeValidInput();
	Input.ExternalAttackD6 = 1;
	const FCutInsideShotDirectShotPlanQueryResult Result =
		FCutInsideShotDirectShotPlanQuery::BuildPlan(
			CutInsideShotDirectShotPlanQueryTests
				::MakePlayerCardSnapshots(),
			CutInsideShotDirectShotPlanQueryTests::MakeSkillRules(),
			Input);
	TestTrue(TEXT("D6 1 succeeds"), Result.bSuccess);
	TestEqual(
		TEXT("D6 1 immediate miss"),
		Result.Decision,
		ECutInsideShotDirectShotDecision::ImmediateMiss);
	TestTrue(TEXT("Immediate miss ends attack"), Result.bAttackEnded);
	TestFalse(TEXT("Immediate miss is not goal"), Result.bIsGoal);
	return true;
}

CUT_INSIDE_DIRECT_SHOT_PLAN_TEST(
	FCutInsideShotPlanAttackD6TwoImmediateMissTest,
	"AttackD6TwoReturnsImmediateMiss")

bool FCutInsideShotPlanAttackD6TwoImmediateMissTest::RunTest(
	const FString& Parameters)
{
	FCutInsideShotDirectShotPlanQueryInput Input =
		CutInsideShotDirectShotPlanQueryTests::MakeValidInput();
	Input.ExternalAttackD6 = 2;
	const FCutInsideShotDirectShotPlanQueryResult Result =
		FCutInsideShotDirectShotPlanQuery::BuildPlan(
			CutInsideShotDirectShotPlanQueryTests
				::MakePlayerCardSnapshots(),
			CutInsideShotDirectShotPlanQueryTests::MakeSkillRules(),
			Input);
	TestTrue(TEXT("D6 2 succeeds"), Result.bSuccess);
	TestEqual(
		TEXT("D6 2 immediate miss"),
		Result.Decision,
		ECutInsideShotDirectShotDecision::ImmediateMiss);
	TestFalse(TEXT("Immediate miss has no plan"), Result.bHasFormulaPlan);
	return true;
}

CUT_INSIDE_DIRECT_SHOT_PLAN_TEST(
	FCutInsideShotPlanImmediateMissNoDefenseD6Test,
	"ImmediateMissDoesNotRequireDefenseD6")

bool FCutInsideShotPlanImmediateMissNoDefenseD6Test::RunTest(
	const FString& Parameters)
{
	FCutInsideShotDirectShotPlanQueryInput Input =
		CutInsideShotDirectShotPlanQueryTests::MakeValidInput();
	Input.ExternalAttackD6 = 1;
	Input.bHasExternalDefenseD6 = false;
	Input.ExternalDefenseD6 = 0;
	const FCutInsideShotDirectShotPlanQueryResult Result =
		FCutInsideShotDirectShotPlanQuery::BuildPlan(
			CutInsideShotDirectShotPlanQueryTests
				::MakePlayerCardSnapshots(),
			CutInsideShotDirectShotPlanQueryTests::MakeSkillRules(),
			Input);
	TestTrue(TEXT("Immediate miss needs no defense D6"), Result.bSuccess);
	TestEqual(
		TEXT("Decision remains immediate miss"),
		Result.Decision,
		ECutInsideShotDirectShotDecision::ImmediateMiss);
	return true;
}

CUT_INSIDE_DIRECT_SHOT_PLAN_TEST(
	FCutInsideShotPlanImmediateMissNoFormulaPlanTest,
	"ImmediateMissDoesNotGenerateFormulaPlan")

bool FCutInsideShotPlanImmediateMissNoFormulaPlanTest::RunTest(
	const FString& Parameters)
{
	FCutInsideShotDirectShotPlanQueryInput Input =
		CutInsideShotDirectShotPlanQueryTests::MakeValidInput();
	Input.ExternalAttackD6 = 2;
	const FCutInsideShotDirectShotPlanQueryResult Result =
		FCutInsideShotDirectShotPlanQuery::BuildPlan(
			CutInsideShotDirectShotPlanQueryTests
				::MakePlayerCardSnapshots(),
			CutInsideShotDirectShotPlanQueryTests::MakeSkillRules(),
			Input);
	TestFalse(TEXT("No formula plan flag"), Result.bHasFormulaPlan);
	TestEqual(
		TEXT("Default attacker formula type"),
		Result.FormulaPlan.AttackerQueryInput.FormulaType,
		EFormulaType::None);
	return true;
}

CUT_INSIDE_DIRECT_SHOT_PLAN_TEST(
	FCutInsideShotPlanAttackD6ThreeFormulaPlanTest,
	"AttackD6ThreeGeneratesFinishingFormulaPlan")

bool FCutInsideShotPlanAttackD6ThreeFormulaPlanTest::RunTest(
	const FString& Parameters)
{
	FCutInsideShotDirectShotPlanQueryInput Input =
		CutInsideShotDirectShotPlanQueryTests::MakeValidInput();
	Input.ExternalAttackD6 = 3;
	const FCutInsideShotDirectShotPlanQueryResult Result =
		FCutInsideShotDirectShotPlanQuery::BuildPlan(
			CutInsideShotDirectShotPlanQueryTests
				::MakePlayerCardSnapshots(),
			CutInsideShotDirectShotPlanQueryTests::MakeSkillRules(),
			Input);
	TestTrue(TEXT("D6 3 succeeds"), Result.bSuccess);
	TestEqual(
		TEXT("D6 3 requires formula"),
		Result.Decision,
		ECutInsideShotDirectShotDecision
			::FormulaResolutionRequired);
	TestTrue(TEXT("Formula plan exists"), Result.bHasFormulaPlan);
	TestEqual(
		TEXT("Attacker finishing"),
		Result.FormulaPlan.AttackerQueryInput.FormulaType,
		EFormulaType::Finishing);
	return true;
}

CUT_INSIDE_DIRECT_SHOT_PLAN_TEST(
	FCutInsideShotPlanAttackD6SixFormulaPlanTest,
	"AttackD6SixGeneratesFinishingFormulaPlan")

bool FCutInsideShotPlanAttackD6SixFormulaPlanTest::RunTest(
	const FString& Parameters)
{
	FCutInsideShotDirectShotPlanQueryInput Input =
		CutInsideShotDirectShotPlanQueryTests::MakeValidInput();
	Input.ExternalAttackD6 = 6;
	const FCutInsideShotDirectShotPlanQueryResult Result =
		FCutInsideShotDirectShotPlanQuery::BuildPlan(
			CutInsideShotDirectShotPlanQueryTests
				::MakePlayerCardSnapshots(),
			CutInsideShotDirectShotPlanQueryTests::MakeSkillRules(),
			Input);
	TestTrue(TEXT("D6 6 succeeds"), Result.bSuccess);
	TestEqual(
		TEXT("D6 6 requires formula"),
		Result.Decision,
		ECutInsideShotDirectShotDecision
			::FormulaResolutionRequired);
	TestTrue(TEXT("Formula plan exists"), Result.bHasFormulaPlan);
	TestFalse(TEXT("Formula plan has not ended attack"), Result.bAttackEnded);
	return true;
}

CUT_INSIDE_DIRECT_SHOT_PLAN_TEST(
	FCutInsideShotPlanEqualAttributesModifierTest,
	"EqualShootingAndDribblingProducesZeroModifier")

bool FCutInsideShotPlanEqualAttributesModifierTest::RunTest(
	const FString& Parameters)
{
	const FCutInsideShotDirectShotPlanQueryResult Result =
		FCutInsideShotDirectShotPlanQuery::BuildPlan(
			CutInsideShotDirectShotPlanQueryTests
				::MakePlayerCardSnapshots(5, 5),
			CutInsideShotDirectShotPlanQueryTests::MakeSkillRules(),
			CutInsideShotDirectShotPlanQueryTests::MakeValidInput());
	TestEqual(
		TEXT("Derived modifier"),
		Result.FormulaPlan.AttackerQueryInput.ExternalModifier,
		0.0f);
	return true;
}

CUT_INSIDE_DIRECT_SHOT_PLAN_TEST(
	FCutInsideShotPlanPositiveDerivedModifierTest,
	"ShootingBelowDribblingProducesPositiveModifier")

bool FCutInsideShotPlanPositiveDerivedModifierTest::RunTest(
	const FString& Parameters)
{
	const FCutInsideShotDirectShotPlanQueryResult Result =
		FCutInsideShotDirectShotPlanQuery::BuildPlan(
			CutInsideShotDirectShotPlanQueryTests
				::MakePlayerCardSnapshots(4, 6),
			CutInsideShotDirectShotPlanQueryTests::MakeSkillRules(),
			CutInsideShotDirectShotPlanQueryTests::MakeValidInput());
	TestEqual(
		TEXT("Positive modifier"),
		Result.FormulaPlan.AttackerQueryInput.ExternalModifier,
		1.0f);
	return true;
}

CUT_INSIDE_DIRECT_SHOT_PLAN_TEST(
	FCutInsideShotPlanNegativeDerivedModifierTest,
	"ShootingAboveDribblingProducesNegativeModifier")

bool FCutInsideShotPlanNegativeDerivedModifierTest::RunTest(
	const FString& Parameters)
{
	const FCutInsideShotDirectShotPlanQueryResult Result =
		FCutInsideShotDirectShotPlanQuery::BuildPlan(
			CutInsideShotDirectShotPlanQueryTests
				::MakePlayerCardSnapshots(6, 3),
			CutInsideShotDirectShotPlanQueryTests::MakeSkillRules(),
			CutInsideShotDirectShotPlanQueryTests::MakeValidInput());
	TestEqual(
		TEXT("Negative modifier"),
		Result.FormulaPlan.AttackerQueryInput.ExternalModifier,
		-1.5f);
	return true;
}

CUT_INSIDE_DIRECT_SHOT_PLAN_TEST(
	FCutInsideShotPlanOddSumHalfAverageTest,
	"OddShootingAndDribblingSumProducesHalfAverage")

bool FCutInsideShotPlanOddSumHalfAverageTest::RunTest(
	const FString& Parameters)
{
	const int32 Shooting = 3;
	const int32 Dribbling = 6;
	const FCutInsideShotDirectShotPlanQueryResult Result =
		FCutInsideShotDirectShotPlanQuery::BuildPlan(
			CutInsideShotDirectShotPlanQueryTests
				::MakePlayerCardSnapshots(Shooting, Dribbling),
			CutInsideShotDirectShotPlanQueryTests::MakeSkillRules(),
			CutInsideShotDirectShotPlanQueryTests::MakeValidInput());
	const int32 AverageTenths = (Shooting + Dribbling) * 5;
	const int32 ActualTenths =
		Shooting * 10
		+ FMath::RoundToInt(
			Result.FormulaPlan.AttackerQueryInput.ExternalModifier
			* 10.0f);
	TestEqual(TEXT("Average tenths"), ActualTenths, AverageTenths);
	TestEqual(
		TEXT("Half-point modifier"),
		Result.FormulaPlan.AttackerQueryInput.ExternalModifier,
		1.5f);
	return true;
}

CUT_INSIDE_DIRECT_SHOT_PLAN_TEST(
	FCutInsideShotPlanFormulaMappingTest,
	"FormulaPlanUsesShootingTacklingAndPlusTwo")

bool FCutInsideShotPlanFormulaMappingTest::RunTest(
	const FString& Parameters)
{
	const FCutInsideShotDirectShotPlanQueryResult Result =
		FCutInsideShotDirectShotPlanQuery::BuildPlan(
			CutInsideShotDirectShotPlanQueryTests
				::MakePlayerCardSnapshots(),
			CutInsideShotDirectShotPlanQueryTests::MakeSkillRules(),
			CutInsideShotDirectShotPlanQueryTests::MakeValidInput());
	const FSingleCardFormulaInputAssemblyQueryInput& Attacker =
		Result.FormulaPlan.AttackerQueryInput;
	const FSingleCardFormulaInputAssemblyQueryInput& Defender =
		Result.FormulaPlan.DefenderQueryInput;
	TestEqual(TEXT("Attacker role"), Attacker.ParticipantRole, ESingleCardFormulaParticipantRole::Attacker);
	TestEqual(TEXT("Defender role"), Defender.ParticipantRole, ESingleCardFormulaParticipantRole::Defender);
	TestEqual(TEXT("Attacker Shooting"), Attacker.Attribute, ESingleCardFormulaAttribute::Shooting);
	TestEqual(TEXT("Defender Tackling"), Defender.Attribute, ESingleCardFormulaAttribute::Tackling);
	TestEqual(TEXT("Defender modifier"), Defender.ExternalModifier, 2.0f);
	return true;
}

CUT_INSIDE_DIRECT_SHOT_PLAN_TEST(
	FCutInsideShotPlanExternalD6MappingTest,
	"FormulaPlanPreservesExternalD6AndSourceFlags")

bool FCutInsideShotPlanExternalD6MappingTest::RunTest(
	const FString& Parameters)
{
	FCutInsideShotDirectShotPlanQueryInput Input =
		CutInsideShotDirectShotPlanQueryTests::MakeValidInput();
	Input.ExternalAttackD6 = 5;
	Input.ExternalDefenseD6 = 2;
	const FCutInsideShotDirectShotPlanQueryResult Result =
		FCutInsideShotDirectShotPlanQuery::BuildPlan(
			CutInsideShotDirectShotPlanQueryTests
				::MakePlayerCardSnapshots(),
			CutInsideShotDirectShotPlanQueryTests::MakeSkillRules(),
			Input);
	TestTrue(TEXT("Attacker D6 source flag"), Result.FormulaPlan.AttackerQueryInput.bHasExternalD6ComparePoint);
	TestEqual(TEXT("Attacker D6"), Result.FormulaPlan.AttackerQueryInput.ExternalD6ComparePoint, 5);
	TestTrue(TEXT("Defender D6 source flag"), Result.FormulaPlan.DefenderQueryInput.bHasExternalD6ComparePoint);
	TestEqual(TEXT("Defender D6"), Result.FormulaPlan.DefenderQueryInput.ExternalD6ComparePoint, 2);
	TestTrue(TEXT("Attacker modifier source flag"), Result.FormulaPlan.AttackerQueryInput.bHasExternalModifier);
	TestTrue(TEXT("Defender modifier source flag"), Result.FormulaPlan.DefenderQueryInput.bHasExternalModifier);
	return true;
}

CUT_INSIDE_DIRECT_SHOT_PLAN_TEST(
	FCutInsideShotPlanRejectsSkillTypeMismatchTest,
	"RejectsNonCutInsideShotSkillType")

bool FCutInsideShotPlanRejectsSkillTypeMismatchTest::RunTest(
	const FString& Parameters)
{
	const FCutInsideShotDirectShotPlanQueryResult Result =
		FCutInsideShotDirectShotPlanQuery::BuildPlan(
			CutInsideShotDirectShotPlanQueryTests
				::MakePlayerCardSnapshots(),
			CutInsideShotDirectShotPlanQueryTests::MakeSkillRules(
				ESkillRuleType::LongShot),
			CutInsideShotDirectShotPlanQueryTests::MakeValidInput());
	TestFalse(TEXT("Wrong skill type fails"), Result.bSuccess);
	TestEqual(
		TEXT("Type mismatch error"),
		Result.ErrorCode,
		ECutInsideShotDirectShotPlanQueryErrorCode::SkillTypeMismatch);
	return true;
}

CUT_INSIDE_DIRECT_SHOT_PLAN_TEST(
	FCutInsideShotPlanRejectsInvalidAttackD6Test,
	"RejectsInvalidAttackD6")

bool FCutInsideShotPlanRejectsInvalidAttackD6Test::RunTest(
	const FString& Parameters)
{
	FCutInsideShotDirectShotPlanQueryInput Input =
		CutInsideShotDirectShotPlanQueryTests::MakeValidInput();
	Input.ExternalAttackD6 = 7;
	const FCutInsideShotDirectShotPlanQueryResult Result =
		FCutInsideShotDirectShotPlanQuery::BuildPlan(
			CutInsideShotDirectShotPlanQueryTests
				::MakePlayerCardSnapshots(),
			CutInsideShotDirectShotPlanQueryTests::MakeSkillRules(),
			Input);
	TestFalse(TEXT("Invalid attack D6 fails"), Result.bSuccess);
	TestEqual(
		TEXT("Invalid attack D6 error"),
		Result.ErrorCode,
		ECutInsideShotDirectShotPlanQueryErrorCode::InvalidAttackD6);
	return true;
}

CUT_INSIDE_DIRECT_SHOT_PLAN_TEST(
	FCutInsideShotPlanRejectsMissingDefenseD6Test,
	"RejectsMissingDefenseD6WhenFormulaRequired")

bool FCutInsideShotPlanRejectsMissingDefenseD6Test::RunTest(
	const FString& Parameters)
{
	FCutInsideShotDirectShotPlanQueryInput Input =
		CutInsideShotDirectShotPlanQueryTests::MakeValidInput();
	Input.ExternalAttackD6 = 3;
	Input.bHasExternalDefenseD6 = false;
	const FCutInsideShotDirectShotPlanQueryResult Result =
		FCutInsideShotDirectShotPlanQuery::BuildPlan(
			CutInsideShotDirectShotPlanQueryTests
				::MakePlayerCardSnapshots(),
			CutInsideShotDirectShotPlanQueryTests::MakeSkillRules(),
			Input);
	TestFalse(TEXT("Missing defense D6 fails"), Result.bSuccess);
	TestEqual(
		TEXT("Missing defense D6 error"),
		Result.ErrorCode,
		ECutInsideShotDirectShotPlanQueryErrorCode
			::MissingExternalDefenseD6);
	TestFalse(TEXT("No plan on failure"), Result.bHasFormulaPlan);
	return true;
}

CUT_INSIDE_DIRECT_SHOT_PLAN_TEST(
	FCutInsideShotPlanRejectsInvalidDefenseD6Test,
	"RejectsInvalidDefenseD6WhenFormulaRequired")

bool FCutInsideShotPlanRejectsInvalidDefenseD6Test::RunTest(
	const FString& Parameters)
{
	FCutInsideShotDirectShotPlanQueryInput Input =
		CutInsideShotDirectShotPlanQueryTests::MakeValidInput();
	Input.ExternalDefenseD6 = 0;
	const FCutInsideShotDirectShotPlanQueryResult Result =
		FCutInsideShotDirectShotPlanQuery::BuildPlan(
			CutInsideShotDirectShotPlanQueryTests
				::MakePlayerCardSnapshots(),
			CutInsideShotDirectShotPlanQueryTests::MakeSkillRules(),
			Input);
	TestFalse(TEXT("Invalid defense D6 fails"), Result.bSuccess);
	TestEqual(
		TEXT("Invalid defense D6 error"),
		Result.ErrorCode,
		ECutInsideShotDirectShotPlanQueryErrorCode::InvalidDefenseD6);
	return true;
}

CUT_INSIDE_DIRECT_SHOT_PLAN_TEST(
	FCutInsideShotPlanContextMappingTest,
	"FormulaPlanPreservesLogTurnPlayerAndCardIds")

bool FCutInsideShotPlanContextMappingTest::RunTest(
	const FString& Parameters)
{
	const FCutInsideShotDirectShotPlanQueryInput Input =
		CutInsideShotDirectShotPlanQueryTests::MakeValidInput();
	const FCutInsideShotDirectShotPlanQueryResult Result =
		FCutInsideShotDirectShotPlanQuery::BuildPlan(
			CutInsideShotDirectShotPlanQueryTests
				::MakePlayerCardSnapshots(),
			CutInsideShotDirectShotPlanQueryTests::MakeSkillRules(),
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

CUT_INSIDE_DIRECT_SHOT_PLAN_TEST(
	FCutInsideShotPlanDoesNotMutateInputTest,
	"DoesNotMutateInput")

bool FCutInsideShotPlanDoesNotMutateInputTest::RunTest(
	const FString& Parameters)
{
	FCutInsideShotDirectShotPlanQueryInput Input =
		CutInsideShotDirectShotPlanQueryTests::MakeValidInput();
	const FCutInsideShotDirectShotPlanQueryInput Before = Input;
	const FCutInsideShotDirectShotPlanQueryResult Result =
		FCutInsideShotDirectShotPlanQuery::BuildPlan(
			CutInsideShotDirectShotPlanQueryTests
				::MakePlayerCardSnapshots(),
			CutInsideShotDirectShotPlanQueryTests::MakeSkillRules(),
			Input);
	TestTrue(
		TEXT("Input remains unchanged"),
		CutInsideShotDirectShotPlanQueryTests::AreInputsEqual(
			Input,
			Before));
	TestTrue(
		TEXT("Result preserves input copy"),
		CutInsideShotDirectShotPlanQueryTests::AreInputsEqual(
			Result.Input,
			Before));
	return true;
}

CUT_INSIDE_DIRECT_SHOT_PLAN_TEST(
	FCutInsideShotPlanDoesNotMutatePlayerSnapshotsTest,
	"DoesNotMutatePlayerCardSnapshotSet")

bool FCutInsideShotPlanDoesNotMutatePlayerSnapshotsTest::RunTest(
	const FString& Parameters)
{
	FPlayerCardRuleSnapshotSet SnapshotSet =
		CutInsideShotDirectShotPlanQueryTests
			::MakePlayerCardSnapshots();
	const FPlayerCardRuleSnapshotSet Before = SnapshotSet;
	FCutInsideShotDirectShotPlanQuery::BuildPlan(
		SnapshotSet,
		CutInsideShotDirectShotPlanQueryTests::MakeSkillRules(),
		CutInsideShotDirectShotPlanQueryTests::MakeValidInput());
	TestTrue(
		TEXT("Player snapshots remain unchanged"),
		CutInsideShotDirectShotPlanQueryTests
			::ArePlayerCardSnapshotsEqual(
				SnapshotSet,
				Before));
	return true;
}

CUT_INSIDE_DIRECT_SHOT_PLAN_TEST(
	FCutInsideShotPlanDoesNotMutateSkillRulesTest,
	"DoesNotMutateSkillRuleSnapshotSet")

bool FCutInsideShotPlanDoesNotMutateSkillRulesTest::RunTest(
	const FString& Parameters)
{
	FSkillRuleSnapshotSet SnapshotSet =
		CutInsideShotDirectShotPlanQueryTests::MakeSkillRules();
	const FSkillRuleSnapshotSet Before = SnapshotSet;
	FCutInsideShotDirectShotPlanQuery::BuildPlan(
		CutInsideShotDirectShotPlanQueryTests
			::MakePlayerCardSnapshots(),
		SnapshotSet,
		CutInsideShotDirectShotPlanQueryTests::MakeValidInput());
	TestTrue(
		TEXT("Skill rules remain unchanged"),
		CutInsideShotDirectShotPlanQueryTests
			::AreSkillRuleSnapshotsEqual(SnapshotSet, Before));
	return true;
}

CUT_INSIDE_DIRECT_SHOT_PLAN_TEST(
	FCutInsideShotPlanHasNoForbiddenDependenciesTest,
	"HasNoForbiddenDependencies")

bool FCutInsideShotPlanHasNoForbiddenDependenciesTest::RunTest(
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
			TEXT("CutInsideShotDirectShotPlanQuery.h")));
	const bool bImplementationLoaded = FFileHelper::LoadFileToString(
		ImplementationSource,
		*FPaths::Combine(
			SourceDirectory,
			TEXT("CutInsideShotDirectShotPlanQuery.cpp")));
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

#undef CUT_INSIDE_DIRECT_SHOT_PLAN_TEST

#endif
