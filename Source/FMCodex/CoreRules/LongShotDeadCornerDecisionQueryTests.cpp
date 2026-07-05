#if WITH_DEV_AUTOMATION_TESTS

#include "LongShotDeadCornerDecisionQuery.h"

#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace LongShotDeadCornerDecisionQueryTests
{
	const FName SkillId(TEXT("Skill.LongShot.DeadCorner"));
	const FName AttackerCardId(TEXT("DeadCorner.Attacker"));
	const FName AttackerPlayerId(TEXT("Player.Attacker"));
	const FGuid LogId(710, 720, 730, 740);

	FPlayerCardRuleSnapshot MakeAttacker(
		const bool bOwnsSkill = true,
		const bool bGoalkeeper = false)
	{
		FPlayerCardRuleSnapshot Card;
		Card.CardId = AttackerCardId;
		Card.SkillIds = bOwnsSkill
			? TArray<FName>{ SkillId }
			: TArray<FName>{};
		if (bGoalkeeper)
		{
			Card.PositionTypes = {
				EPlayerPositionType::Goalkeeper
			};
			Card.bIsGoalkeeper = true;
			Card.bHasGoalkeeperAttributes = true;
		}
		else
		{
			Card.PositionTypes = {
				EPlayerPositionType::Attack
			};
		}
		return Card;
	}

	FPlayerCardRuleSnapshotSet MakePlayerCardSnapshots(
		const bool bOwnsSkill = true,
		const bool bGoalkeeper = false)
	{
		FPlayerCardRuleSnapshotSet SnapshotSet;
		SnapshotSet.Cards = {
			MakeAttacker(bOwnsSkill, bGoalkeeper)
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

	FLongShotDeadCornerDecisionQueryInput MakeInput(
		const int32 D6A = 5,
		const int32 D6B = 6)
	{
		FLongShotDeadCornerDecisionQueryInput Input;
		Input.SkillId = SkillId;
		Input.AttackerCardId = AttackerCardId;
		Input.CurrentActionPoint = 4;
		Input.bHasExternalAttackD6A = true;
		Input.ExternalAttackD6A = D6A;
		Input.bHasExternalAttackD6B = true;
		Input.ExternalAttackD6B = D6B;
		Input.LogId = LogId;
		Input.TurnIndex = 13;
		Input.AttackerPlayerId = AttackerPlayerId;
		return Input;
	}

	bool ExpectError(
		FAutomationTestBase& Test,
		const FLongShotDeadCornerDecisionQueryInput& Input,
		const FPlayerCardRuleSnapshotSet& PlayerCardSnapshots,
		const FSkillRuleSnapshotSet& SkillRules,
		const ELongShotDeadCornerDecisionQueryErrorCode ErrorCode,
		const FName InvalidField)
	{
		const FLongShotDeadCornerDecisionQueryResult Result =
			FLongShotDeadCornerDecisionQuery::Evaluate(
				PlayerCardSnapshots,
				SkillRules,
				Input);
		Test.TestFalse(TEXT("Decision Query fails"), Result.bSuccess);
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
		Test.TestEqual(
			TEXT("Failure produces no decision"),
			Result.Decision,
			ELongShotDeadCornerDecision::None);
		Test.TestFalse(
			TEXT("Failure does not end attack"),
			Result.bAttackEnded);
		Test.TestFalse(
			TEXT("Failure is not a goal"),
			Result.bIsGoal);
		return true;
	}

	bool AreInputsEqual(
		const FLongShotDeadCornerDecisionQueryInput& Left,
		const FLongShotDeadCornerDecisionQueryInput& Right)
	{
		return Left.SkillId == Right.SkillId
			&& Left.AttackerCardId == Right.AttackerCardId
			&& Left.CurrentActionPoint == Right.CurrentActionPoint
			&& Left.bHasExternalAttackD6A
				== Right.bHasExternalAttackD6A
			&& Left.ExternalAttackD6A == Right.ExternalAttackD6A
			&& Left.bHasExternalAttackD6B
				== Right.bHasExternalAttackD6B
			&& Left.ExternalAttackD6B == Right.ExternalAttackD6B
			&& Left.LogId == Right.LogId
			&& Left.TurnIndex == Right.TurnIndex
			&& Left.AttackerPlayerId == Right.AttackerPlayerId;
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
				TEXT("LongShotDeadCornerDecisionQuery.h")));
		const bool bImplementationLoaded =
			FFileHelper::LoadFileToString(
				Implementation,
				*FPaths::Combine(
					Directory,
					TEXT("LongShotDeadCornerDecisionQuery.cpp")));
		OutSource = Header + Implementation;
		return bHeaderLoaded && bImplementationLoaded;
	}
}

#define DEAD_CORNER_TEST(ClassName, TestName) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
		ClassName, \
		"FMCodex.CoreRules.LongShotDeadCornerDecisionQuery." TestName, \
		EAutomationTestFlags::EditorContext \
			| EAutomationTestFlags::EngineFilter)

DEAD_CORNER_TEST(
	FDeadCornerRejectsEmptySkillIdTest,
	"RejectsEmptySkillId")

bool FDeadCornerRejectsEmptySkillIdTest::RunTest(
	const FString& Parameters)
{
	using namespace LongShotDeadCornerDecisionQueryTests;
	FLongShotDeadCornerDecisionQueryInput Input = MakeInput();
	Input.SkillId = NAME_None;
	return ExpectError(
		*this,
		Input,
		MakePlayerCardSnapshots(),
		MakeSkillRules(),
		ELongShotDeadCornerDecisionQueryErrorCode::MissingSkillId,
		TEXT("SkillId"));
}

DEAD_CORNER_TEST(
	FDeadCornerRejectsMissingAttackerTest,
	"RejectsMissingAttackerSnapshot")

bool FDeadCornerRejectsMissingAttackerTest::RunTest(
	const FString& Parameters)
{
	using namespace LongShotDeadCornerDecisionQueryTests;
	FLongShotDeadCornerDecisionQueryInput Input = MakeInput();
	Input.AttackerCardId = TEXT("DeadCorner.Missing");
	return ExpectError(
		*this,
		Input,
		MakePlayerCardSnapshots(),
		MakeSkillRules(),
		ELongShotDeadCornerDecisionQueryErrorCode
			::AttackerSnapshotQueryFailed,
		TEXT("AttackerCardId"));
}

DEAD_CORNER_TEST(
	FDeadCornerRejectsMissingSkillRuleTest,
	"RejectsSkillRuleNotFound")

bool FDeadCornerRejectsMissingSkillRuleTest::RunTest(
	const FString& Parameters)
{
	using namespace LongShotDeadCornerDecisionQueryTests;
	return ExpectError(
		*this,
		MakeInput(),
		MakePlayerCardSnapshots(),
		FSkillRuleSnapshotSet{},
		ELongShotDeadCornerDecisionQueryErrorCode
			::SkillRuleQueryFailed,
		TEXT("SkillId"));
}

DEAD_CORNER_TEST(
	FDeadCornerRejectsUnownedSkillTest,
	"RejectsSkillNotOwnedByAttacker")

bool FDeadCornerRejectsUnownedSkillTest::RunTest(
	const FString& Parameters)
{
	using namespace LongShotDeadCornerDecisionQueryTests;
	return ExpectError(
		*this,
		MakeInput(),
		MakePlayerCardSnapshots(false),
		MakeSkillRules(),
		ELongShotDeadCornerDecisionQueryErrorCode
			::SkillNotOwnedByAttacker,
		TEXT("SkillId"));
}

DEAD_CORNER_TEST(
	FDeadCornerRejectsSkillTypeMismatchTest,
	"RejectsSkillTypeMismatch")

bool FDeadCornerRejectsSkillTypeMismatchTest::RunTest(
	const FString& Parameters)
{
	using namespace LongShotDeadCornerDecisionQueryTests;
	FSkillRuleSnapshotSet SkillRules = MakeSkillRules();
	SkillRules.SkillRules[0].SkillType = ESkillRuleType::None;
	return ExpectError(
		*this,
		MakeInput(),
		MakePlayerCardSnapshots(),
		SkillRules,
		ELongShotDeadCornerDecisionQueryErrorCode
			::SkillTypeMismatch,
		TEXT("SkillType"));
}

DEAD_CORNER_TEST(
	FDeadCornerRejectsInvalidActionPointTest,
	"RejectsInvalidActionPoint")

bool FDeadCornerRejectsInvalidActionPointTest::RunTest(
	const FString& Parameters)
{
	using namespace LongShotDeadCornerDecisionQueryTests;
	FLongShotDeadCornerDecisionQueryInput Input = MakeInput();
	Input.CurrentActionPoint = 1;
	return ExpectError(
		*this,
		Input,
		MakePlayerCardSnapshots(),
		MakeSkillRules(),
		ELongShotDeadCornerDecisionQueryErrorCode
			::InvalidActionPoint,
		TEXT("CurrentActionPoint"));
}

DEAD_CORNER_TEST(
	FDeadCornerRejectsActionPointRangeTest,
	"RejectsActionPointOutOfRange")

bool FDeadCornerRejectsActionPointRangeTest::RunTest(
	const FString& Parameters)
{
	using namespace LongShotDeadCornerDecisionQueryTests;
	FLongShotDeadCornerDecisionQueryInput Input = MakeInput();
	Input.CurrentActionPoint = 2;
	return ExpectError(
		*this,
		Input,
		MakePlayerCardSnapshots(),
		MakeSkillRules(),
		ELongShotDeadCornerDecisionQueryErrorCode
			::ActionPointOutOfRange,
		TEXT("CurrentActionPoint"));
}

DEAD_CORNER_TEST(
	FDeadCornerRejectsGoalkeeperTest,
	"RejectsGoalkeeperAttacker")

bool FDeadCornerRejectsGoalkeeperTest::RunTest(
	const FString& Parameters)
{
	using namespace LongShotDeadCornerDecisionQueryTests;
	return ExpectError(
		*this,
		MakeInput(),
		MakePlayerCardSnapshots(true, true),
		MakeSkillRules(),
		ELongShotDeadCornerDecisionQueryErrorCode
			::UnsupportedGoalkeeperParticipant,
		TEXT("AttackerCardId"));
}

DEAD_CORNER_TEST(
	FDeadCornerRejectsMissingD6ATest,
	"RejectsMissingExternalAttackD6A")

bool FDeadCornerRejectsMissingD6ATest::RunTest(
	const FString& Parameters)
{
	using namespace LongShotDeadCornerDecisionQueryTests;
	FLongShotDeadCornerDecisionQueryInput Input = MakeInput();
	Input.bHasExternalAttackD6A = false;
	return ExpectError(
		*this,
		Input,
		MakePlayerCardSnapshots(),
		MakeSkillRules(),
		ELongShotDeadCornerDecisionQueryErrorCode
			::MissingExternalAttackD6A,
		TEXT("ExternalAttackD6A"));
}

DEAD_CORNER_TEST(
	FDeadCornerRejectsMissingD6BTest,
	"RejectsMissingExternalAttackD6B")

bool FDeadCornerRejectsMissingD6BTest::RunTest(
	const FString& Parameters)
{
	using namespace LongShotDeadCornerDecisionQueryTests;
	FLongShotDeadCornerDecisionQueryInput Input = MakeInput();
	Input.bHasExternalAttackD6B = false;
	return ExpectError(
		*this,
		Input,
		MakePlayerCardSnapshots(),
		MakeSkillRules(),
		ELongShotDeadCornerDecisionQueryErrorCode
			::MissingExternalAttackD6B,
		TEXT("ExternalAttackD6B"));
}

DEAD_CORNER_TEST(
	FDeadCornerRejectsInvalidD6ATest,
	"RejectsInvalidAttackD6A")

bool FDeadCornerRejectsInvalidD6ATest::RunTest(
	const FString& Parameters)
{
	using namespace LongShotDeadCornerDecisionQueryTests;
	FLongShotDeadCornerDecisionQueryInput Input = MakeInput();
	Input.ExternalAttackD6A = 0;
	return ExpectError(
		*this,
		Input,
		MakePlayerCardSnapshots(),
		MakeSkillRules(),
		ELongShotDeadCornerDecisionQueryErrorCode::InvalidAttackD6A,
		TEXT("ExternalAttackD6A"));
}

DEAD_CORNER_TEST(
	FDeadCornerRejectsInvalidD6BTest,
	"RejectsInvalidAttackD6B")

bool FDeadCornerRejectsInvalidD6BTest::RunTest(
	const FString& Parameters)
{
	using namespace LongShotDeadCornerDecisionQueryTests;
	FLongShotDeadCornerDecisionQueryInput Input = MakeInput();
	Input.ExternalAttackD6B = 7;
	return ExpectError(
		*this,
		Input,
		MakePlayerCardSnapshots(),
		MakeSkillRules(),
		ELongShotDeadCornerDecisionQueryErrorCode::InvalidAttackD6B,
		TEXT("ExternalAttackD6B"));
}

DEAD_CORNER_TEST(
	FDeadCornerTotalElevenGoalTest,
	"AttackD6TotalElevenReturnsGoal")

bool FDeadCornerTotalElevenGoalTest::RunTest(
	const FString& Parameters)
{
	using namespace LongShotDeadCornerDecisionQueryTests;
	const FLongShotDeadCornerDecisionQueryResult Result =
		FLongShotDeadCornerDecisionQuery::Evaluate(
			MakePlayerCardSnapshots(),
			MakeSkillRules(),
			MakeInput(5, 6));
	TestTrue(TEXT("Total eleven succeeds"), Result.bSuccess);
	TestEqual(
		TEXT("Total eleven is Goal"),
		Result.Decision,
		ELongShotDeadCornerDecision::Goal);
	TestTrue(TEXT("Goal flag is true"), Result.bIsGoal);
	TestTrue(TEXT("Goal ends attack"), Result.bAttackEnded);
	return true;
}

DEAD_CORNER_TEST(
	FDeadCornerTotalTwelveGoalTest,
	"AttackD6TotalTwelveReturnsGoal")

bool FDeadCornerTotalTwelveGoalTest::RunTest(
	const FString& Parameters)
{
	using namespace LongShotDeadCornerDecisionQueryTests;
	const FLongShotDeadCornerDecisionQueryResult Result =
		FLongShotDeadCornerDecisionQuery::Evaluate(
			MakePlayerCardSnapshots(),
			MakeSkillRules(),
			MakeInput(6, 6));
	TestTrue(TEXT("Total twelve succeeds"), Result.bSuccess);
	TestEqual(
		TEXT("Total twelve is Goal"),
		Result.Decision,
		ELongShotDeadCornerDecision::Goal);
	TestTrue(TEXT("Goal flag is true"), Result.bIsGoal);
	TestTrue(TEXT("Goal ends attack"), Result.bAttackEnded);
	return true;
}

DEAD_CORNER_TEST(
	FDeadCornerTotalBelowElevenMissTest,
	"AttackD6TotalBelowElevenReturnsMiss")

bool FDeadCornerTotalBelowElevenMissTest::RunTest(
	const FString& Parameters)
{
	using namespace LongShotDeadCornerDecisionQueryTests;
	const FLongShotDeadCornerDecisionQueryResult Result =
		FLongShotDeadCornerDecisionQuery::Evaluate(
			MakePlayerCardSnapshots(),
			MakeSkillRules(),
			MakeInput(5, 5));
	TestTrue(TEXT("Total ten succeeds"), Result.bSuccess);
	TestEqual(
		TEXT("Total ten is Miss"),
		Result.Decision,
		ELongShotDeadCornerDecision::Miss);
	TestFalse(TEXT("Miss is not a goal"), Result.bIsGoal);
	TestTrue(TEXT("Miss ends attack"), Result.bAttackEnded);
	return true;
}

DEAD_CORNER_TEST(
	FDeadCornerOtherTotalMissTest,
	"AttackD6TotalOtherThanElevenOrTwelveReturnsMiss")

bool FDeadCornerOtherTotalMissTest::RunTest(
	const FString& Parameters)
{
	using namespace LongShotDeadCornerDecisionQueryTests;
	const FLongShotDeadCornerDecisionQueryResult Result =
		FLongShotDeadCornerDecisionQuery::Evaluate(
			MakePlayerCardSnapshots(),
			MakeSkillRules(),
			MakeInput(1, 1));
	TestTrue(TEXT("Total two succeeds"), Result.bSuccess);
	TestEqual(
		TEXT("Total two is Miss"),
		Result.Decision,
		ELongShotDeadCornerDecision::Miss);
	TestFalse(TEXT("Miss is not a goal"), Result.bIsGoal);
	TestTrue(TEXT("Miss ends attack"), Result.bAttackEnded);
	return true;
}

DEAD_CORNER_TEST(
	FDeadCornerGoalAndMissEndTest,
	"GoalAndMissBothEndAttack")

bool FDeadCornerGoalAndMissEndTest::RunTest(
	const FString& Parameters)
{
	using namespace LongShotDeadCornerDecisionQueryTests;
	const FLongShotDeadCornerDecisionQueryResult Goal =
		FLongShotDeadCornerDecisionQuery::Evaluate(
			MakePlayerCardSnapshots(),
			MakeSkillRules(),
			MakeInput(6, 5));
	const FLongShotDeadCornerDecisionQueryResult Miss =
		FLongShotDeadCornerDecisionQuery::Evaluate(
			MakePlayerCardSnapshots(),
			MakeSkillRules(),
			MakeInput(4, 4));
	TestTrue(TEXT("Goal ends attack"), Goal.bAttackEnded);
	TestTrue(TEXT("Miss ends attack"), Miss.bAttackEnded);
	TestTrue(TEXT("Goal is goal"), Goal.bIsGoal);
	TestFalse(TEXT("Miss is not goal"), Miss.bIsGoal);
	return true;
}

DEAD_CORNER_TEST(
	FDeadCornerNoDefenderTest,
	"DoesNotRequireDefenderOrDefenseD6")

bool FDeadCornerNoDefenderTest::RunTest(
	const FString& Parameters)
{
	using namespace LongShotDeadCornerDecisionQueryTests;
	const FLongShotDeadCornerDecisionQueryResult Result =
		FLongShotDeadCornerDecisionQuery::Evaluate(
			MakePlayerCardSnapshots(),
			MakeSkillRules(),
			MakeInput());
	TestTrue(
		TEXT("Attacker-only decision succeeds"),
		Result.bSuccess);
	FString Source;
	TestTrue(TEXT("Query source loads"), LoadQuerySource(Source));
	TestFalse(
		TEXT("Query contains no defender contract"),
		Source.Contains(TEXT("Defender")));
	TestFalse(
		TEXT("Query contains no defense D6 contract"),
		Source.Contains(TEXT("DefenseD6")));
	return true;
}

DEAD_CORNER_TEST(
	FDeadCornerNoFormulaPlanTest,
	"DoesNotGenerateFormulaPlan")

bool FDeadCornerNoFormulaPlanTest::RunTest(
	const FString& Parameters)
{
	using namespace LongShotDeadCornerDecisionQueryTests;
	FString Source;
	TestTrue(TEXT("Query source loads"), LoadQuerySource(Source));
	TestFalse(
		TEXT("Query has no Formula Plan"),
		Source.Contains(TEXT("FormulaPlan"))
			|| Source.Contains(TEXT("Formula Plan")));
	return true;
}

DEAD_CORNER_TEST(
	FDeadCornerNoFormulaChainTest,
	"DoesNotCallFormulaChain")

bool FDeadCornerNoFormulaChainTest::RunTest(
	const FString& Parameters)
{
	using namespace LongShotDeadCornerDecisionQueryTests;
	FString Source;
	TestTrue(TEXT("Query source loads"), LoadQuerySource(Source));
	TestFalse(
		TEXT("Query does not call Input Assembly"),
		Source.Contains(TEXT("SingleCardFormulaInputAssembly")));
	TestFalse(
		TEXT("Query does not call Resolver Input Assembler"),
		Source.Contains(TEXT("SingleCardFormulaResolverInputAssembler")));
	TestFalse(
		TEXT("Query does not call Executor"),
		Source.Contains(TEXT("SingleCardFormulaResolutionExecutor")));
	TestFalse(
		TEXT("Query does not call FormulaResolver"),
		Source.Contains(TEXT("FormulaResolver")));
	return true;
}

DEAD_CORNER_TEST(
	FDeadCornerSnapshotDiagnosticsTest,
	"PreservesSnapshotQueryDiagnostics")

bool FDeadCornerSnapshotDiagnosticsTest::RunTest(
	const FString& Parameters)
{
	using namespace LongShotDeadCornerDecisionQueryTests;
	FLongShotDeadCornerDecisionQueryInput Input = MakeInput();
	Input.AttackerCardId = TEXT("DeadCorner.Missing");
	const FLongShotDeadCornerDecisionQueryResult Result =
		FLongShotDeadCornerDecisionQuery::Evaluate(
			MakePlayerCardSnapshots(),
			MakeSkillRules(),
			Input);
	TestEqual(
		TEXT("Top-level attacker query error"),
		Result.ErrorCode,
		ELongShotDeadCornerDecisionQueryErrorCode
			::AttackerSnapshotQueryFailed);
	TestEqual(
		TEXT("Nested CardNotFound is preserved"),
		Result.AttackerSnapshotQueryResult.ErrorCode,
		EPlayerCardRuleSnapshotQueryErrorCode::CardNotFound);
	TestEqual(
		TEXT("Requested missing CardId is preserved"),
		Result.AttackerSnapshotQueryResult.CardId,
		Input.AttackerCardId);
	return true;
}

DEAD_CORNER_TEST(
	FDeadCornerSkillDiagnosticsTest,
	"PreservesSkillRuleQueryDiagnostics")

bool FDeadCornerSkillDiagnosticsTest::RunTest(
	const FString& Parameters)
{
	using namespace LongShotDeadCornerDecisionQueryTests;
	const FLongShotDeadCornerDecisionQueryResult Result =
		FLongShotDeadCornerDecisionQuery::Evaluate(
			MakePlayerCardSnapshots(),
			FSkillRuleSnapshotSet{},
			MakeInput());
	TestEqual(
		TEXT("Top-level skill query error"),
		Result.ErrorCode,
		ELongShotDeadCornerDecisionQueryErrorCode
			::SkillRuleQueryFailed);
	TestEqual(
		TEXT("Nested SkillRuleNotFound is preserved"),
		Result.SkillRuleQueryResult.ErrorCode,
		ESkillRuleSnapshotQueryErrorCode::SkillRuleNotFound);
	TestEqual(
		TEXT("Missing SkillId is preserved"),
		Result.SkillRuleQueryResult.InvalidSkillId,
		SkillId);
	return true;
}

DEAD_CORNER_TEST(
	FDeadCornerInputImmutabilityTest,
	"DoesNotMutateInput")

bool FDeadCornerInputImmutabilityTest::RunTest(
	const FString& Parameters)
{
	using namespace LongShotDeadCornerDecisionQueryTests;
	const FLongShotDeadCornerDecisionQueryInput Input = MakeInput();
	const FLongShotDeadCornerDecisionQueryInput Before = Input;
	const FLongShotDeadCornerDecisionQueryResult Result =
		FLongShotDeadCornerDecisionQuery::Evaluate(
			MakePlayerCardSnapshots(),
			MakeSkillRules(),
			Input);
	TestTrue(
		TEXT("Input remains unchanged"),
		AreInputsEqual(Input, Before));
	TestTrue(
		TEXT("Result preserves input copy"),
		AreInputsEqual(Result.Input, Before));
	return true;
}

DEAD_CORNER_TEST(
	FDeadCornerPlayerSnapshotsImmutabilityTest,
	"DoesNotMutatePlayerCardSnapshotSet")

bool FDeadCornerPlayerSnapshotsImmutabilityTest::RunTest(
	const FString& Parameters)
{
	using namespace LongShotDeadCornerDecisionQueryTests;
	const FPlayerCardRuleSnapshotSet SnapshotSet =
		MakePlayerCardSnapshots();
	const FPlayerCardRuleSnapshotSet Before = SnapshotSet;
	FLongShotDeadCornerDecisionQuery::Evaluate(
		SnapshotSet,
		MakeSkillRules(),
		MakeInput());
	TestTrue(
		TEXT("Player Snapshot Set remains unchanged"),
		ArePlayerSnapshotSetsEqual(SnapshotSet, Before));
	return true;
}

DEAD_CORNER_TEST(
	FDeadCornerSkillRulesImmutabilityTest,
	"DoesNotMutateSkillRuleSnapshotSet")

bool FDeadCornerSkillRulesImmutabilityTest::RunTest(
	const FString& Parameters)
{
	using namespace LongShotDeadCornerDecisionQueryTests;
	const FSkillRuleSnapshotSet SnapshotSet = MakeSkillRules();
	const FSkillRuleSnapshotSet Before = SnapshotSet;
	FLongShotDeadCornerDecisionQuery::Evaluate(
		MakePlayerCardSnapshots(),
		SnapshotSet,
		MakeInput());
	TestTrue(
		TEXT("Skill Rule Snapshot Set remains unchanged"),
		AreSkillRuleSnapshotSetsEqual(SnapshotSet, Before));
	return true;
}

DEAD_CORNER_TEST(
	FDeadCornerInvalidLogContextTest,
	"RejectsInvalidLogContext")

bool FDeadCornerInvalidLogContextTest::RunTest(
	const FString& Parameters)
{
	using namespace LongShotDeadCornerDecisionQueryTests;
	FLongShotDeadCornerDecisionQueryInput Input = MakeInput();
	Input.LogId.Invalidate();
	return ExpectError(
		*this,
		Input,
		MakePlayerCardSnapshots(),
		MakeSkillRules(),
		ELongShotDeadCornerDecisionQueryErrorCode::InvalidLogContext,
		TEXT("LogId"));
}

DEAD_CORNER_TEST(
	FDeadCornerForbiddenDependenciesTest,
	"HasNoForbiddenDependencies")

bool FDeadCornerForbiddenDependenciesTest::RunTest(
	const FString& Parameters)
{
	using namespace LongShotDeadCornerDecisionQueryTests;
	FString Source;
	TestTrue(TEXT("Query source loads"), LoadQuerySource(Source));
	const TArray<FString> ForbiddenTerms = {
		TEXT("MatchPlay"),
		TEXT("ExternalApi"),
		TEXT("FormulaAttackFlow"),
		TEXT("DataTable"),
		TEXT("Provider"),
		TEXT("CardDatabase"),
		TEXT("SkillEffect"),
		TEXT("SkillPipeline"),
		TEXT("FMath::Rand"),
		TEXT("FRandomStream"),
		TEXT("EFormulaType::Determination"),
		TEXT("LongShotBranchSelection")
	};
	for (const FString& Term : ForbiddenTerms)
	{
		TestFalse(
			TEXT("Forbidden dependency is absent"),
			Source.Contains(Term));
	}
	return true;
}

#undef DEAD_CORNER_TEST

#endif
