#if WITH_DEV_AUTOMATION_TESTS

#include "PassControlPassAdvancePlanQuery.h"

#include "HAL/FileManager.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace PassControlPassAdvancePlanQueryTests
{
	const FName SkillId(TEXT("Skill.PassControl.PassAdvance"));
	const FName CarrierCardId(TEXT("PassAdvance.Carrier"));
	const FName RunnerCardId(TEXT("PassAdvance.Runner"));
	const FName MarkerCardId(TEXT("PassAdvance.Marker"));
	const FName HelperCardId(TEXT("PassAdvance.Helper"));
	const FName CarrierPlayerId(TEXT("Player.PassAdvance.Carrier"));
	const FName RunnerPlayerId(TEXT("Player.PassAdvance.Runner"));
	const FName MarkerPlayerId(TEXT("Player.PassAdvance.Marker"));
	const FName HelperPlayerId(TEXT("Player.PassAdvance.Helper"));
	const FGuid LogId(1001, 1002, 1003, 1004);

	FPlayerCardRuleSnapshot MakeCard(
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
		const int32 CarrierPassing = 4,
		const int32 RunnerPassing = 6,
		const int32 MarkerTackling = 5,
		const int32 HelperMarking = 3,
		const bool bCarrierOwnsSkill = true,
		const bool bCarrierGoalkeeper = false,
		const bool bRunnerMidfield = true)
	{
		FPlayerCardRuleSnapshot Carrier = MakeCard(
			CarrierCardId,
			bCarrierGoalkeeper
				? EPlayerPositionType::Goalkeeper
				: EPlayerPositionType::Midfield,
			bCarrierOwnsSkill
				? TArray<FName>{ SkillId }
				: TArray<FName>{});
		Carrier.Attributes.Passing = CarrierPassing;
		Carrier.bIsGoalkeeper = bCarrierGoalkeeper;
		Carrier.bHasGoalkeeperAttributes = bCarrierGoalkeeper;

		FPlayerCardRuleSnapshot Runner = MakeCard(
			RunnerCardId,
			bRunnerMidfield
				? EPlayerPositionType::Midfield
				: EPlayerPositionType::Attack);
		Runner.Attributes.Passing = RunnerPassing;

		FPlayerCardRuleSnapshot Marker = MakeCard(
			MarkerCardId,
			EPlayerPositionType::Defense);
		Marker.Attributes.Tackling = MarkerTackling;

		FPlayerCardRuleSnapshot Helper = MakeCard(
			HelperCardId,
			EPlayerPositionType::Defense);
		Helper.Attributes.Marking = HelperMarking;

		FPlayerCardRuleSnapshotSet SnapshotSet;
		SnapshotSet.Cards = { Carrier, Runner, Marker, Helper };
		return SnapshotSet;
	}

	FSkillRuleSnapshotSet MakeSkillRules(
		const ESkillRuleType SkillType = ESkillRuleType::PassControl)
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

	FPassControlPassAdvancePlanQueryInput MakeValidInput()
	{
		FPassControlPassAdvancePlanQueryInput Input;
		Input.SkillId = SkillId;
		Input.AdvanceType = EPassControlAdvanceType::PassAdvance;
		Input.CarrierCardId = CarrierCardId;
		Input.RunnerCardId = RunnerCardId;
		Input.MarkerCardId = MarkerCardId;
		Input.HelperCardId = HelperCardId;
		Input.CurrentActionPoint = 4;
		Input.bHasExternalAttackD6 = true;
		Input.ExternalAttackD6 = 4;
		Input.bHasExternalDefenseD6 = true;
		Input.ExternalDefenseD6 = 3;
		Input.LogId = LogId;
		Input.TurnIndex = 21;
		Input.CarrierPlayerId = CarrierPlayerId;
		Input.RunnerPlayerId = RunnerPlayerId;
		Input.MarkerPlayerId = MarkerPlayerId;
		Input.HelperPlayerId = HelperPlayerId;
		return Input;
	}

	bool ExpectError(
		FAutomationTestBase& Test,
		const FPassControlPassAdvancePlanQueryInput& Input,
		const FPlayerCardRuleSnapshotSet& PlayerCardSnapshots,
		const FSkillRuleSnapshotSet& SkillRules,
		const EPassControlPassAdvancePlanQueryErrorCode ErrorCode,
		const FName InvalidField)
	{
		const FPassControlPassAdvancePlanQueryResult Result =
			FPassControlPassAdvancePlanQuery::BuildPlan(
				PlayerCardSnapshots,
				SkillRules,
				Input);
		Test.TestFalse(TEXT("PassAdvance Plan Query fails"), Result.bSuccess);
		Test.TestEqual(
			TEXT("Expected error code is returned"),
			Result.ErrorCode,
			ErrorCode);
		Test.TestEqual(
			TEXT("Expected invalid field is returned"),
			Result.InvalidField,
			InvalidField);
		Test.TestFalse(
			TEXT("Failure message is not empty"),
			Result.ErrorMessage.IsEmpty());
		Test.TestFalse(TEXT("Failure has no formula plan"), Result.bHasFormulaPlan);
		return true;
	}

	bool AreInputsEqual(
		const FPassControlPassAdvancePlanQueryInput& Left,
		const FPassControlPassAdvancePlanQueryInput& Right)
	{
		return Left.SkillId == Right.SkillId
			&& Left.AdvanceType == Right.AdvanceType
			&& Left.CarrierCardId == Right.CarrierCardId
			&& Left.RunnerCardId == Right.RunnerCardId
			&& Left.MarkerCardId == Right.MarkerCardId
			&& Left.HelperCardId == Right.HelperCardId
			&& Left.CurrentActionPoint == Right.CurrentActionPoint
			&& Left.bHasExternalAttackD6 == Right.bHasExternalAttackD6
			&& Left.ExternalAttackD6 == Right.ExternalAttackD6
			&& Left.bHasExternalDefenseD6 == Right.bHasExternalDefenseD6
			&& Left.ExternalDefenseD6 == Right.ExternalDefenseD6
			&& Left.LogId == Right.LogId
			&& Left.TurnIndex == Right.TurnIndex
			&& Left.CarrierPlayerId == Right.CarrierPlayerId
			&& Left.RunnerPlayerId == Right.RunnerPlayerId
			&& Left.MarkerPlayerId == Right.MarkerPlayerId
			&& Left.HelperPlayerId == Right.HelperPlayerId;
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

	bool ArePlayerSnapshotSetsEqual(
		const FPlayerCardRuleSnapshotSet& Left,
		const FPlayerCardRuleSnapshotSet& Right)
	{
		if (Left.Cards.Num() != Right.Cards.Num())
		{
			return false;
		}
		for (int32 Index = 0; Index < Left.Cards.Num(); ++Index)
		{
			const FPlayerCardRuleSnapshot& LeftCard = Left.Cards[Index];
			const FPlayerCardRuleSnapshot& RightCard = Right.Cards[Index];
			if (LeftCard.CardId != RightCard.CardId
				|| LeftCard.PositionTypes != RightCard.PositionTypes
				|| !ArePlayerAttributesEqual(
					LeftCard.Attributes,
					RightCard.Attributes)
				|| LeftCard.bIsGoalkeeper != RightCard.bIsGoalkeeper
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

	bool AreSkillRuleSnapshotSetsEqual(
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

	bool LoadQuerySource(FString& OutSource)
	{
		const FString Directory = FPaths::Combine(
			FPaths::ProjectDir(),
			TEXT("Source/FMCodex/CoreRules"));
		FString Header;
		FString Implementation;
		const bool bHeaderLoaded = FFileHelper::LoadFileToString(
			Header,
			*FPaths::Combine(
				Directory,
				TEXT("PassControlPassAdvancePlanQuery.h")));
		const bool bImplementationLoaded =
			FFileHelper::LoadFileToString(
				Implementation,
				*FPaths::Combine(
					Directory,
					TEXT("PassControlPassAdvancePlanQuery.cpp")));
		OutSource = Header + Implementation;
		return bHeaderLoaded && bImplementationLoaded;
	}
}

#define PASS_CONTROL_PASS_ADVANCE_PLAN_TEST(ClassName, TestName) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
		ClassName, \
		"FMCodex.CoreRules.PassControlPassAdvancePlanQuery." TestName, \
		EAutomationTestFlags::EditorContext \
			| EAutomationTestFlags::EngineFilter)

PASS_CONTROL_PASS_ADVANCE_PLAN_TEST(
	FPassControlPassAdvancePlanGeneratesFinishingTest,
	"GeneratesFinishingFormulaPlan")

bool FPassControlPassAdvancePlanGeneratesFinishingTest::RunTest(
	const FString& Parameters)
{
	using namespace PassControlPassAdvancePlanQueryTests;
	const FPassControlPassAdvancePlanQueryResult Result =
		FPassControlPassAdvancePlanQuery::BuildPlan(
			MakePlayerCardSnapshots(),
			MakeSkillRules(),
			MakeValidInput());
	TestTrue(TEXT("Query succeeds"), Result.bSuccess);
	TestEqual(
		TEXT("Formula resolution is required"),
		Result.Decision,
		EPassControlPassAdvancePlanDecision::FormulaResolutionRequired);
	TestTrue(TEXT("Formula plan exists"), Result.bHasFormulaPlan);
	TestEqual(
		TEXT("Attacker FormulaType is Finishing"),
		Result.FormulaPlan.AttackerQueryInput.FormulaType,
		EFormulaType::Finishing);
	TestEqual(
		TEXT("Defender FormulaType is Finishing"),
		Result.FormulaPlan.DefenderQueryInput.FormulaType,
		EFormulaType::Finishing);
	return true;
}

PASS_CONTROL_PASS_ADVANCE_PLAN_TEST(
	FPassControlPassAdvanceRejectsNoneAdvanceTypeTest,
	"RejectsNoneAdvanceType")

bool FPassControlPassAdvanceRejectsNoneAdvanceTypeTest::RunTest(
	const FString& Parameters)
{
	using namespace PassControlPassAdvancePlanQueryTests;
	FPassControlPassAdvancePlanQueryInput Input = MakeValidInput();
	Input.AdvanceType = EPassControlAdvanceType::None;
	return ExpectError(
		*this,
		Input,
		MakePlayerCardSnapshots(),
		MakeSkillRules(),
		EPassControlPassAdvancePlanQueryErrorCode::InvalidAdvanceType,
		TEXT("AdvanceType"));
}

PASS_CONTROL_PASS_ADVANCE_PLAN_TEST(
	FPassControlPassAdvanceRejectsDribbleAdvanceTypeTest,
	"RejectsDribbleAdvanceType")

bool FPassControlPassAdvanceRejectsDribbleAdvanceTypeTest::RunTest(
	const FString& Parameters)
{
	using namespace PassControlPassAdvancePlanQueryTests;
	FPassControlPassAdvancePlanQueryInput Input = MakeValidInput();
	Input.AdvanceType = EPassControlAdvanceType::DribbleAdvance;
	return ExpectError(
		*this,
		Input,
		MakePlayerCardSnapshots(),
		MakeSkillRules(),
		EPassControlPassAdvancePlanQueryErrorCode::InvalidAdvanceType,
		TEXT("AdvanceType"));
}

PASS_CONTROL_PASS_ADVANCE_PLAN_TEST(
	FPassControlPassAdvanceRejectsRunAdvanceTypeTest,
	"RejectsRunAdvanceType")

bool FPassControlPassAdvanceRejectsRunAdvanceTypeTest::RunTest(
	const FString& Parameters)
{
	using namespace PassControlPassAdvancePlanQueryTests;
	FPassControlPassAdvancePlanQueryInput Input = MakeValidInput();
	Input.AdvanceType = EPassControlAdvanceType::RunAdvance;
	return ExpectError(
		*this,
		Input,
		MakePlayerCardSnapshots(),
		MakeSkillRules(),
		EPassControlPassAdvancePlanQueryErrorCode::InvalidAdvanceType,
		TEXT("AdvanceType"));
}

PASS_CONTROL_PASS_ADVANCE_PLAN_TEST(
	FPassControlPassAdvanceRejectsNonPassControlSkillTypeTest,
	"RejectsNonPassControlSkillType")

bool FPassControlPassAdvanceRejectsNonPassControlSkillTypeTest::RunTest(
	const FString& Parameters)
{
	using namespace PassControlPassAdvancePlanQueryTests;
	return ExpectError(
		*this,
		MakeValidInput(),
		MakePlayerCardSnapshots(),
		MakeSkillRules(ESkillRuleType::LongShot),
		EPassControlPassAdvancePlanQueryErrorCode::SkillTypeMismatch,
		TEXT("SkillType"));
}

PASS_CONTROL_PASS_ADVANCE_PLAN_TEST(
	FPassControlPassAdvanceRejectsMissingSkillRuleTest,
	"RejectsSkillRuleQueryFailure")

bool FPassControlPassAdvanceRejectsMissingSkillRuleTest::RunTest(
	const FString& Parameters)
{
	using namespace PassControlPassAdvancePlanQueryTests;
	return ExpectError(
		*this,
		MakeValidInput(),
		MakePlayerCardSnapshots(),
		FSkillRuleSnapshotSet{},
		EPassControlPassAdvancePlanQueryErrorCode::SkillRuleQueryFailed,
		TEXT("SkillId"));
}

PASS_CONTROL_PASS_ADVANCE_PLAN_TEST(
	FPassControlPassAdvanceRejectsMissingCarrierTest,
	"RejectsCarrierSnapshotQueryFailure")

bool FPassControlPassAdvanceRejectsMissingCarrierTest::RunTest(
	const FString& Parameters)
{
	using namespace PassControlPassAdvancePlanQueryTests;
	FPassControlPassAdvancePlanQueryInput Input = MakeValidInput();
	Input.CarrierCardId = TEXT("PassAdvance.MissingCarrier");
	return ExpectError(
		*this,
		Input,
		MakePlayerCardSnapshots(),
		MakeSkillRules(),
		EPassControlPassAdvancePlanQueryErrorCode
			::CarrierSnapshotQueryFailed,
		TEXT("CarrierCardId"));
}

PASS_CONTROL_PASS_ADVANCE_PLAN_TEST(
	FPassControlPassAdvanceRejectsMissingRunnerTest,
	"RejectsRunnerSnapshotQueryFailure")

bool FPassControlPassAdvanceRejectsMissingRunnerTest::RunTest(
	const FString& Parameters)
{
	using namespace PassControlPassAdvancePlanQueryTests;
	FPassControlPassAdvancePlanQueryInput Input = MakeValidInput();
	Input.RunnerCardId = TEXT("PassAdvance.MissingRunner");
	return ExpectError(
		*this,
		Input,
		MakePlayerCardSnapshots(),
		MakeSkillRules(),
		EPassControlPassAdvancePlanQueryErrorCode
			::RunnerSnapshotQueryFailed,
		TEXT("RunnerCardId"));
}

PASS_CONTROL_PASS_ADVANCE_PLAN_TEST(
	FPassControlPassAdvanceRejectsMissingMarkerTest,
	"RejectsMarkerSnapshotQueryFailure")

bool FPassControlPassAdvanceRejectsMissingMarkerTest::RunTest(
	const FString& Parameters)
{
	using namespace PassControlPassAdvancePlanQueryTests;
	FPassControlPassAdvancePlanQueryInput Input = MakeValidInput();
	Input.MarkerCardId = TEXT("PassAdvance.MissingMarker");
	return ExpectError(
		*this,
		Input,
		MakePlayerCardSnapshots(),
		MakeSkillRules(),
		EPassControlPassAdvancePlanQueryErrorCode
			::MarkerSnapshotQueryFailed,
		TEXT("MarkerCardId"));
}

PASS_CONTROL_PASS_ADVANCE_PLAN_TEST(
	FPassControlPassAdvanceRejectsMissingHelperTest,
	"RejectsHelperSnapshotQueryFailure")

bool FPassControlPassAdvanceRejectsMissingHelperTest::RunTest(
	const FString& Parameters)
{
	using namespace PassControlPassAdvancePlanQueryTests;
	FPassControlPassAdvancePlanQueryInput Input = MakeValidInput();
	Input.HelperCardId = TEXT("PassAdvance.MissingHelper");
	return ExpectError(
		*this,
		Input,
		MakePlayerCardSnapshots(),
		MakeSkillRules(),
		EPassControlPassAdvancePlanQueryErrorCode
			::HelperSnapshotQueryFailed,
		TEXT("HelperCardId"));
}

PASS_CONTROL_PASS_ADVANCE_PLAN_TEST(
	FPassControlPassAdvanceRejectsUnownedSkillTest,
	"RejectsSkillNotOwnedByCarrier")

bool FPassControlPassAdvanceRejectsUnownedSkillTest::RunTest(
	const FString& Parameters)
{
	using namespace PassControlPassAdvancePlanQueryTests;
	return ExpectError(
		*this,
		MakeValidInput(),
		MakePlayerCardSnapshots(4, 6, 5, 3, false),
		MakeSkillRules(),
		EPassControlPassAdvancePlanQueryErrorCode
			::SkillNotOwnedByCarrier,
		TEXT("SkillId"));
}

PASS_CONTROL_PASS_ADVANCE_PLAN_TEST(
	FPassControlPassAdvanceRejectsGoalkeeperCarrierTest,
	"RejectsGoalkeeperCarrier")

bool FPassControlPassAdvanceRejectsGoalkeeperCarrierTest::RunTest(
	const FString& Parameters)
{
	using namespace PassControlPassAdvancePlanQueryTests;
	return ExpectError(
		*this,
		MakeValidInput(),
		MakePlayerCardSnapshots(4, 6, 5, 3, true, true),
		MakeSkillRules(),
		EPassControlPassAdvancePlanQueryErrorCode
			::UnsupportedGoalkeeperParticipant,
		TEXT("CarrierCardId"));
}

PASS_CONTROL_PASS_ADVANCE_PLAN_TEST(
	FPassControlPassAdvanceRejectsNonMidfieldRunnerTest,
	"RejectsNonMidfieldRunner")

bool FPassControlPassAdvanceRejectsNonMidfieldRunnerTest::RunTest(
	const FString& Parameters)
{
	using namespace PassControlPassAdvancePlanQueryTests;
	return ExpectError(
		*this,
		MakeValidInput(),
		MakePlayerCardSnapshots(4, 6, 5, 3, true, false, false),
		MakeSkillRules(),
		EPassControlPassAdvancePlanQueryErrorCode
			::RunnerPositionMismatch,
		TEXT("RunnerCardId"));
}

PASS_CONTROL_PASS_ADVANCE_PLAN_TEST(
	FPassControlPassAdvanceRejectsActionPointBelowSkillRangeTest,
	"RejectsActionPointBelowSkillRange")

bool FPassControlPassAdvanceRejectsActionPointBelowSkillRangeTest::RunTest(
	const FString& Parameters)
{
	using namespace PassControlPassAdvancePlanQueryTests;
	FPassControlPassAdvancePlanQueryInput Input = MakeValidInput();
	Input.CurrentActionPoint = 2;
	return ExpectError(
		*this,
		Input,
		MakePlayerCardSnapshots(),
		MakeSkillRules(),
		EPassControlPassAdvancePlanQueryErrorCode::ActionPointOutOfRange,
		TEXT("CurrentActionPoint"));
}

PASS_CONTROL_PASS_ADVANCE_PLAN_TEST(
	FPassControlPassAdvanceRejectsActionPointAboveSkillRangeTest,
	"RejectsActionPointAboveSkillRange")

bool FPassControlPassAdvanceRejectsActionPointAboveSkillRangeTest::RunTest(
	const FString& Parameters)
{
	using namespace PassControlPassAdvancePlanQueryTests;
	FPassControlPassAdvancePlanQueryInput Input = MakeValidInput();
	Input.CurrentActionPoint = 7;
	return ExpectError(
		*this,
		Input,
		MakePlayerCardSnapshots(),
		MakeSkillRules(),
		EPassControlPassAdvancePlanQueryErrorCode::ActionPointOutOfRange,
		TEXT("CurrentActionPoint"));
}

PASS_CONTROL_PASS_ADVANCE_PLAN_TEST(
	FPassControlPassAdvanceRejectsMissingAttackD6Test,
	"RejectsMissingExternalAttackD6")

bool FPassControlPassAdvanceRejectsMissingAttackD6Test::RunTest(
	const FString& Parameters)
{
	using namespace PassControlPassAdvancePlanQueryTests;
	FPassControlPassAdvancePlanQueryInput Input = MakeValidInput();
	Input.bHasExternalAttackD6 = false;
	return ExpectError(
		*this,
		Input,
		MakePlayerCardSnapshots(),
		MakeSkillRules(),
		EPassControlPassAdvancePlanQueryErrorCode
			::MissingExternalAttackD6,
		TEXT("ExternalAttackD6"));
}

PASS_CONTROL_PASS_ADVANCE_PLAN_TEST(
	FPassControlPassAdvanceRejectsMissingDefenseD6Test,
	"RejectsMissingExternalDefenseD6")

bool FPassControlPassAdvanceRejectsMissingDefenseD6Test::RunTest(
	const FString& Parameters)
{
	using namespace PassControlPassAdvancePlanQueryTests;
	FPassControlPassAdvancePlanQueryInput Input = MakeValidInput();
	Input.bHasExternalDefenseD6 = false;
	return ExpectError(
		*this,
		Input,
		MakePlayerCardSnapshots(),
		MakeSkillRules(),
		EPassControlPassAdvancePlanQueryErrorCode
			::MissingExternalDefenseD6,
		TEXT("ExternalDefenseD6"));
}

PASS_CONTROL_PASS_ADVANCE_PLAN_TEST(
	FPassControlPassAdvanceRejectsInvalidAttackD6Test,
	"RejectsInvalidAttackD6")

bool FPassControlPassAdvanceRejectsInvalidAttackD6Test::RunTest(
	const FString& Parameters)
{
	using namespace PassControlPassAdvancePlanQueryTests;
	FPassControlPassAdvancePlanQueryInput Input = MakeValidInput();
	Input.ExternalAttackD6 = 7;
	return ExpectError(
		*this,
		Input,
		MakePlayerCardSnapshots(),
		MakeSkillRules(),
		EPassControlPassAdvancePlanQueryErrorCode::InvalidAttackD6,
		TEXT("ExternalAttackD6"));
}

PASS_CONTROL_PASS_ADVANCE_PLAN_TEST(
	FPassControlPassAdvanceRejectsInvalidDefenseD6Test,
	"RejectsInvalidDefenseD6")

bool FPassControlPassAdvanceRejectsInvalidDefenseD6Test::RunTest(
	const FString& Parameters)
{
	using namespace PassControlPassAdvancePlanQueryTests;
	FPassControlPassAdvancePlanQueryInput Input = MakeValidInput();
	Input.ExternalDefenseD6 = 0;
	return ExpectError(
		*this,
		Input,
		MakePlayerCardSnapshots(),
		MakeSkillRules(),
		EPassControlPassAdvancePlanQueryErrorCode::InvalidDefenseD6,
		TEXT("ExternalDefenseD6"));
}

PASS_CONTROL_PASS_ADVANCE_PLAN_TEST(
	FPassControlPassAdvanceEqualPassingModifierTest,
	"EqualCarrierAndRunnerPassingProducesZeroModifier")

bool FPassControlPassAdvanceEqualPassingModifierTest::RunTest(
	const FString& Parameters)
{
	using namespace PassControlPassAdvancePlanQueryTests;
	const FPassControlPassAdvancePlanQueryResult Result =
		FPassControlPassAdvancePlanQuery::BuildPlan(
			MakePlayerCardSnapshots(5, 5),
			MakeSkillRules(),
			MakeValidInput());
	TestEqual(
		TEXT("Attacker modifier is zero"),
		Result.FormulaPlan.AttackerQueryInput.ExternalModifier,
		0.0f);
	return true;
}

PASS_CONTROL_PASS_ADVANCE_PLAN_TEST(
	FPassControlPassAdvancePositivePassingModifierTest,
	"RunnerPassingAboveCarrierPassingProducesPositiveModifier")

bool FPassControlPassAdvancePositivePassingModifierTest::RunTest(
	const FString& Parameters)
{
	using namespace PassControlPassAdvancePlanQueryTests;
	const FPassControlPassAdvancePlanQueryResult Result =
		FPassControlPassAdvancePlanQuery::BuildPlan(
			MakePlayerCardSnapshots(4, 6),
			MakeSkillRules(),
			MakeValidInput());
	TestEqual(
		TEXT("Attacker modifier is positive"),
		Result.FormulaPlan.AttackerQueryInput.ExternalModifier,
		1.0f);
	return true;
}

PASS_CONTROL_PASS_ADVANCE_PLAN_TEST(
	FPassControlPassAdvanceNegativePassingModifierTest,
	"RunnerPassingBelowCarrierPassingProducesNegativeModifier")

bool FPassControlPassAdvanceNegativePassingModifierTest::RunTest(
	const FString& Parameters)
{
	using namespace PassControlPassAdvancePlanQueryTests;
	const FPassControlPassAdvancePlanQueryResult Result =
		FPassControlPassAdvancePlanQuery::BuildPlan(
			MakePlayerCardSnapshots(6, 3),
			MakeSkillRules(),
			MakeValidInput());
	TestEqual(
		TEXT("Attacker modifier is negative"),
		Result.FormulaPlan.AttackerQueryInput.ExternalModifier,
		-1.5f);
	return true;
}

PASS_CONTROL_PASS_ADVANCE_PLAN_TEST(
	FPassControlPassAdvanceOddPassingAverageTest,
	"OddCarrierAndRunnerPassingSumPreservesHalfAverage")

bool FPassControlPassAdvanceOddPassingAverageTest::RunTest(
	const FString& Parameters)
{
	using namespace PassControlPassAdvancePlanQueryTests;
	const int32 CarrierPassing = 3;
	const int32 RunnerPassing = 6;
	const FPassControlPassAdvancePlanQueryResult Result =
		FPassControlPassAdvancePlanQuery::BuildPlan(
			MakePlayerCardSnapshots(CarrierPassing, RunnerPassing),
			MakeSkillRules(),
			MakeValidInput());
	const int32 ExpectedAverageTenths =
		(CarrierPassing + RunnerPassing) * 5;
	const int32 ActualTenths =
		CarrierPassing * 10
		+ FMath::RoundToInt(
			Result.FormulaPlan.AttackerQueryInput.ExternalModifier
			* 10.0f);
	TestEqual(TEXT("Attacker average tenths"), ActualTenths, ExpectedAverageTenths);
	TestEqual(
		TEXT("Attacker half modifier"),
		Result.FormulaPlan.AttackerQueryInput.ExternalModifier,
		1.5f);
	return true;
}

PASS_CONTROL_PASS_ADVANCE_PLAN_TEST(
	FPassControlPassAdvanceEqualDefenseModifierTest,
	"EqualMarkerTacklingAndHelperMarkingProducesPlusTwoModifier")

bool FPassControlPassAdvanceEqualDefenseModifierTest::RunTest(
	const FString& Parameters)
{
	using namespace PassControlPassAdvancePlanQueryTests;
	const FPassControlPassAdvancePlanQueryResult Result =
		FPassControlPassAdvancePlanQuery::BuildPlan(
			MakePlayerCardSnapshots(4, 6, 5, 5),
			MakeSkillRules(),
			MakeValidInput());
	TestEqual(
		TEXT("Defender modifier is plus two"),
		Result.FormulaPlan.DefenderQueryInput.ExternalModifier,
		2.0f);
	return true;
}

PASS_CONTROL_PASS_ADVANCE_PLAN_TEST(
	FPassControlPassAdvancePositiveDefenseModifierTest,
	"HelperMarkingAboveMarkerTacklingProducesModifierAbovePlusTwo")

bool FPassControlPassAdvancePositiveDefenseModifierTest::RunTest(
	const FString& Parameters)
{
	using namespace PassControlPassAdvancePlanQueryTests;
	const FPassControlPassAdvancePlanQueryResult Result =
		FPassControlPassAdvancePlanQuery::BuildPlan(
			MakePlayerCardSnapshots(4, 6, 4, 6),
			MakeSkillRules(),
			MakeValidInput());
	TestEqual(
		TEXT("Defender modifier is above plus two"),
		Result.FormulaPlan.DefenderQueryInput.ExternalModifier,
		3.0f);
	return true;
}

PASS_CONTROL_PASS_ADVANCE_PLAN_TEST(
	FPassControlPassAdvanceNegativeDefenseModifierTest,
	"HelperMarkingBelowMarkerTacklingProducesModifierBelowPlusTwo")

bool FPassControlPassAdvanceNegativeDefenseModifierTest::RunTest(
	const FString& Parameters)
{
	using namespace PassControlPassAdvancePlanQueryTests;
	const FPassControlPassAdvancePlanQueryResult Result =
		FPassControlPassAdvancePlanQuery::BuildPlan(
			MakePlayerCardSnapshots(4, 6, 6, 3),
			MakeSkillRules(),
			MakeValidInput());
	TestEqual(
		TEXT("Defender modifier is below plus two"),
		Result.FormulaPlan.DefenderQueryInput.ExternalModifier,
		0.5f);
	return true;
}

PASS_CONTROL_PASS_ADVANCE_PLAN_TEST(
	FPassControlPassAdvanceOddDefenseAverageTest,
	"OddMarkerAndHelperSumPreservesHalfAverageThenPlusTwo")

bool FPassControlPassAdvanceOddDefenseAverageTest::RunTest(
	const FString& Parameters)
{
	using namespace PassControlPassAdvancePlanQueryTests;
	const int32 MarkerTackling = 4;
	const int32 HelperMarking = 5;
	const FPassControlPassAdvancePlanQueryResult Result =
		FPassControlPassAdvancePlanQuery::BuildPlan(
			MakePlayerCardSnapshots(4, 6, MarkerTackling, HelperMarking),
			MakeSkillRules(),
			MakeValidInput());
	const int32 ExpectedAverageTenths =
		(MarkerTackling + HelperMarking) * 5 + 20;
	const int32 ActualTenths =
		MarkerTackling * 10
		+ FMath::RoundToInt(
			Result.FormulaPlan.DefenderQueryInput.ExternalModifier
			* 10.0f);
	TestEqual(TEXT("Defender average tenths"), ActualTenths, ExpectedAverageTenths);
	TestEqual(
		TEXT("Defender half plus two modifier"),
		Result.FormulaPlan.DefenderQueryInput.ExternalModifier,
		2.5f);
	return true;
}

PASS_CONTROL_PASS_ADVANCE_PLAN_TEST(
	FPassControlPassAdvancePreservesD6Test,
	"FormulaPlanPreservesAttackAndDefenseD6")

bool FPassControlPassAdvancePreservesD6Test::RunTest(
	const FString& Parameters)
{
	using namespace PassControlPassAdvancePlanQueryTests;
	FPassControlPassAdvancePlanQueryInput Input = MakeValidInput();
	Input.ExternalAttackD6 = 6;
	Input.ExternalDefenseD6 = 1;
	const FPassControlPassAdvancePlanQueryResult Result =
		FPassControlPassAdvancePlanQuery::BuildPlan(
			MakePlayerCardSnapshots(),
			MakeSkillRules(),
			Input);
	TestTrue(TEXT("Attacker D6 flag"), Result.FormulaPlan.AttackerQueryInput.bHasExternalD6ComparePoint);
	TestEqual(TEXT("Attacker D6"), Result.FormulaPlan.AttackerQueryInput.ExternalD6ComparePoint, 6);
	TestTrue(TEXT("Defender D6 flag"), Result.FormulaPlan.DefenderQueryInput.bHasExternalD6ComparePoint);
	TestEqual(TEXT("Defender D6"), Result.FormulaPlan.DefenderQueryInput.ExternalD6ComparePoint, 1);
	return true;
}

PASS_CONTROL_PASS_ADVANCE_PLAN_TEST(
	FPassControlPassAdvancePreservesContextTest,
	"FormulaPlanPreservesPlayerCardLogAndTurnContext")

bool FPassControlPassAdvancePreservesContextTest::RunTest(
	const FString& Parameters)
{
	using namespace PassControlPassAdvancePlanQueryTests;
	const FPassControlPassAdvancePlanQueryInput Input = MakeValidInput();
	const FPassControlPassAdvancePlanQueryResult Result =
		FPassControlPassAdvancePlanQuery::BuildPlan(
			MakePlayerCardSnapshots(),
			MakeSkillRules(),
			Input);
	TestEqual(TEXT("Carrier card"), Result.FormulaPlan.AttackerQueryInput.CardId, CarrierCardId);
	TestEqual(TEXT("Marker card"), Result.FormulaPlan.DefenderQueryInput.CardId, MarkerCardId);
	TestEqual(TEXT("Runner card"), Result.FormulaPlan.RunnerCardId, RunnerCardId);
	TestEqual(TEXT("Helper card"), Result.FormulaPlan.HelperCardId, HelperCardId);
	TestEqual(TEXT("Attacker log"), Result.FormulaPlan.AttackerQueryInput.LogId, LogId);
	TestEqual(TEXT("Defender log"), Result.FormulaPlan.DefenderQueryInput.LogId, LogId);
	TestEqual(TEXT("Attacker turn"), Result.FormulaPlan.AttackerQueryInput.TurnIndex, Input.TurnIndex);
	TestEqual(TEXT("Carrier player"), Result.FormulaPlan.CarrierPlayerId, CarrierPlayerId);
	TestEqual(TEXT("Runner player"), Result.FormulaPlan.RunnerPlayerId, RunnerPlayerId);
	TestEqual(TEXT("Marker player"), Result.FormulaPlan.MarkerPlayerId, MarkerPlayerId);
	TestEqual(TEXT("Helper player"), Result.FormulaPlan.HelperPlayerId, HelperPlayerId);
	return true;
}

PASS_CONTROL_PASS_ADVANCE_PLAN_TEST(
	FPassControlPassAdvancePreservesSnapshotDiagnosticsTest,
	"ResultPreservesAllSnapshotQueryResults")

bool FPassControlPassAdvancePreservesSnapshotDiagnosticsTest::RunTest(
	const FString& Parameters)
{
	using namespace PassControlPassAdvancePlanQueryTests;
	const FPassControlPassAdvancePlanQueryResult Result =
		FPassControlPassAdvancePlanQuery::BuildPlan(
			MakePlayerCardSnapshots(),
			MakeSkillRules(),
			MakeValidInput());
	TestTrue(TEXT("Carrier query succeeds"), Result.CarrierSnapshotQueryResult.bSuccess);
	TestTrue(TEXT("Runner query succeeds"), Result.RunnerSnapshotQueryResult.bSuccess);
	TestTrue(TEXT("Marker query succeeds"), Result.MarkerSnapshotQueryResult.bSuccess);
	TestTrue(TEXT("Helper query succeeds"), Result.HelperSnapshotQueryResult.bSuccess);
	TestEqual(TEXT("Carrier query CardId"), Result.CarrierSnapshotQueryResult.CardId, CarrierCardId);
	TestEqual(TEXT("Runner query CardId"), Result.RunnerSnapshotQueryResult.CardId, RunnerCardId);
	TestEqual(TEXT("Marker query CardId"), Result.MarkerSnapshotQueryResult.CardId, MarkerCardId);
	TestEqual(TEXT("Helper query CardId"), Result.HelperSnapshotQueryResult.CardId, HelperCardId);
	return true;
}

PASS_CONTROL_PASS_ADVANCE_PLAN_TEST(
	FPassControlPassAdvanceDoesNotMutateInputTest,
	"DoesNotMutateInput")

bool FPassControlPassAdvanceDoesNotMutateInputTest::RunTest(
	const FString& Parameters)
{
	using namespace PassControlPassAdvancePlanQueryTests;
	const FPassControlPassAdvancePlanQueryInput Input = MakeValidInput();
	const FPassControlPassAdvancePlanQueryInput Before = Input;
	const FPassControlPassAdvancePlanQueryResult Result =
		FPassControlPassAdvancePlanQuery::BuildPlan(
			MakePlayerCardSnapshots(),
			MakeSkillRules(),
			Input);
	TestTrue(TEXT("Input remains unchanged"), AreInputsEqual(Input, Before));
	TestTrue(TEXT("Result preserves input copy"), AreInputsEqual(Result.Input, Before));
	return true;
}

PASS_CONTROL_PASS_ADVANCE_PLAN_TEST(
	FPassControlPassAdvanceDoesNotMutatePlayerSnapshotsTest,
	"DoesNotMutatePlayerCardSnapshotSet")

bool FPassControlPassAdvanceDoesNotMutatePlayerSnapshotsTest::RunTest(
	const FString& Parameters)
{
	using namespace PassControlPassAdvancePlanQueryTests;
	const FPlayerCardRuleSnapshotSet SnapshotSet =
		MakePlayerCardSnapshots();
	const FPlayerCardRuleSnapshotSet Before = SnapshotSet;
	FPassControlPassAdvancePlanQuery::BuildPlan(
		SnapshotSet,
		MakeSkillRules(),
		MakeValidInput());
	TestTrue(
		TEXT("Player snapshots remain unchanged"),
		ArePlayerSnapshotSetsEqual(SnapshotSet, Before));
	return true;
}

PASS_CONTROL_PASS_ADVANCE_PLAN_TEST(
	FPassControlPassAdvanceDoesNotMutateSkillRulesTest,
	"DoesNotMutateSkillRuleSnapshotSet")

bool FPassControlPassAdvanceDoesNotMutateSkillRulesTest::RunTest(
	const FString& Parameters)
{
	using namespace PassControlPassAdvancePlanQueryTests;
	const FSkillRuleSnapshotSet SnapshotSet = MakeSkillRules();
	const FSkillRuleSnapshotSet Before = SnapshotSet;
	FPassControlPassAdvancePlanQuery::BuildPlan(
		MakePlayerCardSnapshots(),
		SnapshotSet,
		MakeValidInput());
	TestTrue(
		TEXT("Skill rules remain unchanged"),
		AreSkillRuleSnapshotSetsEqual(SnapshotSet, Before));
	return true;
}

PASS_CONTROL_PASS_ADVANCE_PLAN_TEST(
	FPassControlPassAdvanceDoesNotExecuteFormulaChainTest,
	"DoesNotExecuteFormulaChain")

bool FPassControlPassAdvanceDoesNotExecuteFormulaChainTest::RunTest(
	const FString& Parameters)
{
	using namespace PassControlPassAdvancePlanQueryTests;
	FString Source;
	TestTrue(TEXT("Query source loads"), LoadQuerySource(Source));
	TestFalse(
		TEXT("Does not execute Input Assembly Query"),
		Source.Contains(
			TEXT("FSingleCardFormulaInputAssemblyQuery::Assemble")));
	TestFalse(
		TEXT("Does not reference Resolver Input Assembler"),
		Source.Contains(TEXT("SingleCardFormulaResolverInputAssembler")));
	TestFalse(
		TEXT("Does not reference Executor"),
		Source.Contains(TEXT("SingleCardFormulaResolutionExecutor")));
	TestFalse(
		TEXT("Does not reference FormulaResolver"),
		Source.Contains(TEXT("FormulaResolver")));
	return true;
}

PASS_CONTROL_PASS_ADVANCE_PLAN_TEST(
	FPassControlPassAdvanceNoFormulaExecutionDependenciesTest,
	"DoesNotCallInputAssemblyAssemblerExecutorOrFormulaResolver")

bool FPassControlPassAdvanceNoFormulaExecutionDependenciesTest::RunTest(
	const FString& Parameters)
{
	using namespace PassControlPassAdvancePlanQueryTests;
	FString Source;
	TestTrue(TEXT("Query source loads"), LoadQuerySource(Source));
	TestFalse(TEXT("No InputAssemblyQuery call"), Source.Contains(TEXT("::Assemble(")));
	TestFalse(TEXT("No FormulaResolver"), Source.Contains(TEXT("FormulaResolver")));
	TestFalse(TEXT("No FormulaAttackFlow"), Source.Contains(TEXT("FormulaAttackFlow")));
	return true;
}

PASS_CONTROL_PASS_ADVANCE_PLAN_TEST(
	FPassControlPassAdvanceNoExternalFlowDependenciesTest,
	"DoesNotDependOnMatchPlayExternalApiOrFormulaAttackFlow")

bool FPassControlPassAdvanceNoExternalFlowDependenciesTest::RunTest(
	const FString& Parameters)
{
	using namespace PassControlPassAdvancePlanQueryTests;
	FString Source;
	TestTrue(TEXT("Query source loads"), LoadQuerySource(Source));
	TestFalse(TEXT("No MatchPlay"), Source.Contains(TEXT("MatchPlay")));
	TestFalse(TEXT("No ExternalApi"), Source.Contains(TEXT("ExternalApi")));
	TestFalse(TEXT("No FormulaAttackFlow"), Source.Contains(TEXT("FormulaAttackFlow")));
	return true;
}

PASS_CONTROL_PASS_ADVANCE_PLAN_TEST(
	FPassControlPassAdvanceNoSkillFrameworkDependenciesTest,
	"DoesNotIntroduceSkillPipelineSkillEffectOrGenericFramework")

bool FPassControlPassAdvanceNoSkillFrameworkDependenciesTest::RunTest(
	const FString& Parameters)
{
	using namespace PassControlPassAdvancePlanQueryTests;
	FString Source;
	TestTrue(TEXT("Query source loads"), LoadQuerySource(Source));
	TestFalse(TEXT("No SkillPipeline"), Source.Contains(TEXT("SkillPipeline")));
	TestFalse(TEXT("No SkillEffect"), Source.Contains(TEXT("SkillEffect")));
	TestFalse(TEXT("No SkillFramework"), Source.Contains(TEXT("SkillFramework")));
	return true;
}

PASS_CONTROL_PASS_ADVANCE_PLAN_TEST(
	FPassControlPassAdvanceNoRandomGenerationTest,
	"DoesNotGenerateRandomNumbers")

bool FPassControlPassAdvanceNoRandomGenerationTest::RunTest(
	const FString& Parameters)
{
	using namespace PassControlPassAdvancePlanQueryTests;
	FString Source;
	TestTrue(TEXT("Query source loads"), LoadQuerySource(Source));
	TestFalse(TEXT("No FMath::Rand"), Source.Contains(TEXT("FMath::Rand")));
	TestFalse(TEXT("No FRandomStream"), Source.Contains(TEXT("FRandomStream")));
	return true;
}

PASS_CONTROL_PASS_ADVANCE_PLAN_TEST(
	FPassControlPassAdvanceDoesNotImplementOtherBranchesTest,
	"DoesNotImplementDribbleAdvanceOrRunAdvance")

bool FPassControlPassAdvanceDoesNotImplementOtherBranchesTest::RunTest(
	const FString& Parameters)
{
	using namespace PassControlPassAdvancePlanQueryTests;
	FString Source;
	TestTrue(TEXT("Query source loads"), LoadQuerySource(Source));
	TestFalse(TEXT("No DribbleAdvance implementation"), Source.Contains(TEXT("DribbleAdvance")));
	TestFalse(TEXT("No RunAdvance implementation"), Source.Contains(TEXT("RunAdvance")));
	TestFalse(TEXT("No full PassControlPlanQuery"), Source.Contains(TEXT("PassControlPlanQuery")));
	return true;
}

PASS_CONTROL_PASS_ADVANCE_PLAN_TEST(
	FPassControlPassAdvanceDoesNotModifyAdvanceSelectionTest,
	"DoesNotModifyPassControlAdvanceSelectionQuery")

bool FPassControlPassAdvanceDoesNotModifyAdvanceSelectionTest::RunTest(
	const FString& Parameters)
{
	const FString SourceDirectory = FPaths::Combine(
		FPaths::ProjectDir(),
		TEXT("Source/FMCodex/CoreRules"));
	TestTrue(
		TEXT("Advance Selection header still exists"),
		IFileManager::Get().FileExists(
			*FPaths::Combine(
				SourceDirectory,
				TEXT("PassControlAdvanceSelectionQuery.h"))));
	TestTrue(
		TEXT("Advance Selection implementation still exists"),
		IFileManager::Get().FileExists(
			*FPaths::Combine(
				SourceDirectory,
				TEXT("PassControlAdvanceSelectionQuery.cpp"))));
	return true;
}

PASS_CONTROL_PASS_ADVANCE_PLAN_TEST(
	FPassControlPassAdvanceFormulaMappingTest,
	"FormulaPlanUsesCarrierPassingMarkerTacklingAndDerivedModifiers")

bool FPassControlPassAdvanceFormulaMappingTest::RunTest(
	const FString& Parameters)
{
	using namespace PassControlPassAdvancePlanQueryTests;
	const FPassControlPassAdvancePlanQueryResult Result =
		FPassControlPassAdvancePlanQuery::BuildPlan(
			MakePlayerCardSnapshots(),
			MakeSkillRules(),
			MakeValidInput());
	TestEqual(
		TEXT("Attacker role"),
		Result.FormulaPlan.AttackerQueryInput.ParticipantRole,
		ESingleCardFormulaParticipantRole::Attacker);
	TestEqual(
		TEXT("Defender role"),
		Result.FormulaPlan.DefenderQueryInput.ParticipantRole,
		ESingleCardFormulaParticipantRole::Defender);
	TestEqual(
		TEXT("Attacker Passing"),
		Result.FormulaPlan.AttackerQueryInput.Attribute,
		ESingleCardFormulaAttribute::Passing);
	TestEqual(
		TEXT("Defender Tackling"),
		Result.FormulaPlan.DefenderQueryInput.Attribute,
		ESingleCardFormulaAttribute::Tackling);
	TestTrue(
		TEXT("Attacker modifier flag"),
		Result.FormulaPlan.AttackerQueryInput.bHasExternalModifier);
	TestTrue(
		TEXT("Defender modifier flag"),
		Result.FormulaPlan.DefenderQueryInput.bHasExternalModifier);
	return true;
}

PASS_CONTROL_PASS_ADVANCE_PLAN_TEST(
	FPassControlPassAdvanceRejectsInvalidLogIdTest,
	"RejectsInvalidLogId")

bool FPassControlPassAdvanceRejectsInvalidLogIdTest::RunTest(
	const FString& Parameters)
{
	using namespace PassControlPassAdvancePlanQueryTests;
	FPassControlPassAdvancePlanQueryInput Input = MakeValidInput();
	Input.LogId.Invalidate();
	return ExpectError(
		*this,
		Input,
		MakePlayerCardSnapshots(),
		MakeSkillRules(),
		EPassControlPassAdvancePlanQueryErrorCode::InvalidLogContext,
		TEXT("LogId"));
}

PASS_CONTROL_PASS_ADVANCE_PLAN_TEST(
	FPassControlPassAdvanceRejectsInvalidTurnIndexTest,
	"RejectsInvalidTurnIndex")

bool FPassControlPassAdvanceRejectsInvalidTurnIndexTest::RunTest(
	const FString& Parameters)
{
	using namespace PassControlPassAdvancePlanQueryTests;
	FPassControlPassAdvancePlanQueryInput Input = MakeValidInput();
	Input.TurnIndex = -1;
	return ExpectError(
		*this,
		Input,
		MakePlayerCardSnapshots(),
		MakeSkillRules(),
		EPassControlPassAdvancePlanQueryErrorCode::InvalidLogContext,
		TEXT("TurnIndex"));
}

PASS_CONTROL_PASS_ADVANCE_PLAN_TEST(
	FPassControlPassAdvanceRejectsMissingCarrierPlayerIdTest,
	"RejectsMissingCarrierPlayerId")

bool FPassControlPassAdvanceRejectsMissingCarrierPlayerIdTest::RunTest(
	const FString& Parameters)
{
	using namespace PassControlPassAdvancePlanQueryTests;
	FPassControlPassAdvancePlanQueryInput Input = MakeValidInput();
	Input.CarrierPlayerId = NAME_None;
	return ExpectError(
		*this,
		Input,
		MakePlayerCardSnapshots(),
		MakeSkillRules(),
		EPassControlPassAdvancePlanQueryErrorCode::InvalidLogContext,
		TEXT("CarrierPlayerId"));
}

PASS_CONTROL_PASS_ADVANCE_PLAN_TEST(
	FPassControlPassAdvanceRejectsInvalidGlobalActionPointTest,
	"RejectsInvalidGlobalActionPoint")

bool FPassControlPassAdvanceRejectsInvalidGlobalActionPointTest::RunTest(
	const FString& Parameters)
{
	using namespace PassControlPassAdvancePlanQueryTests;
	FPassControlPassAdvancePlanQueryInput Input = MakeValidInput();
	Input.CurrentActionPoint = 1;
	return ExpectError(
		*this,
		Input,
		MakePlayerCardSnapshots(),
		MakeSkillRules(),
		EPassControlPassAdvancePlanQueryErrorCode::InvalidActionPoint,
		TEXT("CurrentActionPoint"));
}

PASS_CONTROL_PASS_ADVANCE_PLAN_TEST(
	FPassControlPassAdvanceRejectsUnknownAdvanceTypeTest,
	"RejectsUnknownAdvanceType")

bool FPassControlPassAdvanceRejectsUnknownAdvanceTypeTest::RunTest(
	const FString& Parameters)
{
	using namespace PassControlPassAdvancePlanQueryTests;
	FPassControlPassAdvancePlanQueryInput Input = MakeValidInput();
	Input.AdvanceType = static_cast<EPassControlAdvanceType>(255);
	return ExpectError(
		*this,
		Input,
		MakePlayerCardSnapshots(),
		MakeSkillRules(),
		EPassControlPassAdvancePlanQueryErrorCode::InvalidAdvanceType,
		TEXT("AdvanceType"));
}

PASS_CONTROL_PASS_ADVANCE_PLAN_TEST(
	FPassControlPassAdvancePlanDoesNotUseAdvanceD6Test,
	"DoesNotReprocessAdvanceSelectionD6")

bool FPassControlPassAdvancePlanDoesNotUseAdvanceD6Test::RunTest(
	const FString& Parameters)
{
	using namespace PassControlPassAdvancePlanQueryTests;
	FString Source;
	TestTrue(TEXT("Query source loads"), LoadQuerySource(Source));
	TestFalse(TEXT("No ExternalAdvanceD6"), Source.Contains(TEXT("ExternalAdvanceD6")));
	TestFalse(TEXT("No bHasExternalAdvanceD6"), Source.Contains(TEXT("bHasExternalAdvanceD6")));
	return true;
}

PASS_CONTROL_PASS_ADVANCE_PLAN_TEST(
	FPassControlPassAdvanceNoDataSourceOrDeckLogicTest,
	"DoesNotIntroduceDataSourceOrDeckLogic")

bool FPassControlPassAdvanceNoDataSourceOrDeckLogicTest::RunTest(
	const FString& Parameters)
{
	using namespace PassControlPassAdvancePlanQueryTests;
	FString Source;
	TestTrue(TEXT("Query source loads"), LoadQuerySource(Source));
	const TArray<FString> ForbiddenTerms = {
		TEXT("DataTable"),
		TEXT("Provider"),
		TEXT("CardDatabase"),
		TEXT("Deck"),
		TEXT("Hand"),
		TEXT("Shuffle"),
		TEXT("Draw")
	};
	for (const FString& Term : ForbiddenTerms)
	{
		TestFalse(TEXT("Forbidden data/deck term is absent"), Source.Contains(Term));
	}
	return true;
}

#undef PASS_CONTROL_PASS_ADVANCE_PLAN_TEST

#endif
