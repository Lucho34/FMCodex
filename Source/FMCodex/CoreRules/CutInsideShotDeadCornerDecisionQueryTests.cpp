#if WITH_DEV_AUTOMATION_TESTS

#include "CutInsideShotDeadCornerDecisionQuery.h"

#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace CutInsideShotDeadCornerDecisionQueryTests
{
	const FName SkillId(TEXT("Skill.CutInsideShot.DeadCorner"));
	const FName AttackerCardId(TEXT("CutInsideDeadCorner.Attacker"));
	const FName AttackerPlayerId(TEXT("Player.CutInsideDeadCorner.Attacker"));
	const FGuid LogId(810, 820, 830, 840);

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
		Rule.SkillType = ESkillRuleType::CutInsideShot;
		Rule.MinTriggerActionPoint = 3;
		Rule.MaxTriggerActionPoint = 6;

		FSkillRuleSnapshotSet SnapshotSet;
		SnapshotSet.SkillRules = { Rule };
		return SnapshotSet;
	}

	FCutInsideShotDeadCornerDecisionQueryInput MakeInput(
		const int32 D6A = 5,
		const int32 D6B = 6)
	{
		FCutInsideShotDeadCornerDecisionQueryInput Input;
		Input.SkillId = SkillId;
		Input.AttackerCardId = AttackerCardId;
		Input.CurrentActionPoint = 4;
		Input.bHasExternalAttackD6A = true;
		Input.ExternalAttackD6A = D6A;
		Input.bHasExternalAttackD6B = true;
		Input.ExternalAttackD6B = D6B;
		Input.LogId = LogId;
		Input.TurnIndex = 17;
		Input.AttackerPlayerId = AttackerPlayerId;
		return Input;
	}

	bool ExpectError(
		FAutomationTestBase& Test,
		const FCutInsideShotDeadCornerDecisionQueryInput& Input,
		const FPlayerCardRuleSnapshotSet& PlayerCardSnapshots,
		const FSkillRuleSnapshotSet& SkillRules,
		const ECutInsideShotDeadCornerDecisionQueryErrorCode ErrorCode,
		const FName InvalidField)
	{
		const FCutInsideShotDeadCornerDecisionQueryResult Result =
			FCutInsideShotDeadCornerDecisionQuery::Evaluate(
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
			ECutInsideShotDeadCornerDecision::None);
		Test.TestFalse(
			TEXT("Failure does not end attack"),
			Result.bAttackEnded);
		Test.TestFalse(
			TEXT("Failure is not a goal"),
			Result.bIsGoal);
		return true;
	}

	bool AreInputsEqual(
		const FCutInsideShotDeadCornerDecisionQueryInput& Left,
		const FCutInsideShotDeadCornerDecisionQueryInput& Right)
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
				TEXT("CutInsideShotDeadCornerDecisionQuery.h")));
		const bool bImplementationLoaded =
			FFileHelper::LoadFileToString(
				Implementation,
				*FPaths::Combine(
					Directory,
					TEXT("CutInsideShotDeadCornerDecisionQuery.cpp")));
		OutSource = Header + Implementation;
		return bHeaderLoaded && bImplementationLoaded;
	}
}

#define CUT_INSIDE_DEAD_CORNER_TEST(ClassName, TestName) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
		ClassName, \
		"FMCodex.CoreRules.CutInsideShotDeadCornerDecisionQuery." TestName, \
		EAutomationTestFlags::EditorContext \
			| EAutomationTestFlags::EngineFilter)

CUT_INSIDE_DEAD_CORNER_TEST(
	FCutInsideDeadCornerRejectsEmptySkillIdTest,
	"RejectsEmptySkillId")

bool FCutInsideDeadCornerRejectsEmptySkillIdTest::RunTest(
	const FString& Parameters)
{
	using namespace CutInsideShotDeadCornerDecisionQueryTests;
	FCutInsideShotDeadCornerDecisionQueryInput Input = MakeInput();
	Input.SkillId = NAME_None;
	return ExpectError(
		*this,
		Input,
		MakePlayerCardSnapshots(),
		MakeSkillRules(),
		ECutInsideShotDeadCornerDecisionQueryErrorCode::MissingSkillId,
		TEXT("SkillId"));
}

CUT_INSIDE_DEAD_CORNER_TEST(
	FCutInsideDeadCornerRejectsMissingAttackerTest,
	"RejectsMissingAttackerSnapshot")

bool FCutInsideDeadCornerRejectsMissingAttackerTest::RunTest(
	const FString& Parameters)
{
	using namespace CutInsideShotDeadCornerDecisionQueryTests;
	FCutInsideShotDeadCornerDecisionQueryInput Input = MakeInput();
	Input.AttackerCardId = TEXT("CutInsideDeadCorner.Missing");
	return ExpectError(
		*this,
		Input,
		MakePlayerCardSnapshots(),
		MakeSkillRules(),
		ECutInsideShotDeadCornerDecisionQueryErrorCode
			::AttackerSnapshotQueryFailed,
		TEXT("AttackerCardId"));
}

CUT_INSIDE_DEAD_CORNER_TEST(
	FCutInsideDeadCornerRejectsMissingSkillRuleTest,
	"RejectsSkillRuleNotFound")

bool FCutInsideDeadCornerRejectsMissingSkillRuleTest::RunTest(
	const FString& Parameters)
{
	using namespace CutInsideShotDeadCornerDecisionQueryTests;
	return ExpectError(
		*this,
		MakeInput(),
		MakePlayerCardSnapshots(),
		FSkillRuleSnapshotSet{},
		ECutInsideShotDeadCornerDecisionQueryErrorCode
			::SkillRuleQueryFailed,
		TEXT("SkillId"));
}

CUT_INSIDE_DEAD_CORNER_TEST(
	FCutInsideDeadCornerRejectsUnownedSkillTest,
	"RejectsSkillNotOwnedByAttacker")

bool FCutInsideDeadCornerRejectsUnownedSkillTest::RunTest(
	const FString& Parameters)
{
	using namespace CutInsideShotDeadCornerDecisionQueryTests;
	return ExpectError(
		*this,
		MakeInput(),
		MakePlayerCardSnapshots(false),
		MakeSkillRules(),
		ECutInsideShotDeadCornerDecisionQueryErrorCode
			::SkillNotOwnedByAttacker,
		TEXT("SkillId"));
}

CUT_INSIDE_DEAD_CORNER_TEST(
	FCutInsideDeadCornerRejectsSkillTypeMismatchTest,
	"RejectsSkillTypeMismatch")

bool FCutInsideDeadCornerRejectsSkillTypeMismatchTest::RunTest(
	const FString& Parameters)
{
	using namespace CutInsideShotDeadCornerDecisionQueryTests;
	FSkillRuleSnapshotSet SkillRules = MakeSkillRules();
	SkillRules.SkillRules[0].SkillType = ESkillRuleType::LongShot;
	return ExpectError(
		*this,
		MakeInput(),
		MakePlayerCardSnapshots(),
		SkillRules,
		ECutInsideShotDeadCornerDecisionQueryErrorCode
			::SkillTypeMismatch,
		TEXT("SkillType"));
}

CUT_INSIDE_DEAD_CORNER_TEST(
	FCutInsideDeadCornerRejectsInvalidActionPointTest,
	"RejectsInvalidActionPoint")

bool FCutInsideDeadCornerRejectsInvalidActionPointTest::RunTest(
	const FString& Parameters)
{
	using namespace CutInsideShotDeadCornerDecisionQueryTests;
	FCutInsideShotDeadCornerDecisionQueryInput Input = MakeInput();
	Input.CurrentActionPoint = 1;
	return ExpectError(
		*this,
		Input,
		MakePlayerCardSnapshots(),
		MakeSkillRules(),
		ECutInsideShotDeadCornerDecisionQueryErrorCode
			::InvalidActionPoint,
		TEXT("CurrentActionPoint"));
}

CUT_INSIDE_DEAD_CORNER_TEST(
	FCutInsideDeadCornerRejectsActionPointRangeTest,
	"RejectsActionPointOutOfRange")

bool FCutInsideDeadCornerRejectsActionPointRangeTest::RunTest(
	const FString& Parameters)
{
	using namespace CutInsideShotDeadCornerDecisionQueryTests;
	FCutInsideShotDeadCornerDecisionQueryInput Input = MakeInput();
	Input.CurrentActionPoint = 2;
	return ExpectError(
		*this,
		Input,
		MakePlayerCardSnapshots(),
		MakeSkillRules(),
		ECutInsideShotDeadCornerDecisionQueryErrorCode
			::ActionPointOutOfRange,
		TEXT("CurrentActionPoint"));
}

CUT_INSIDE_DEAD_CORNER_TEST(
	FCutInsideDeadCornerRejectsActionPointAboveRangeTest,
	"RejectsActionPointAboveRange")

bool FCutInsideDeadCornerRejectsActionPointAboveRangeTest::RunTest(
	const FString& Parameters)
{
	using namespace CutInsideShotDeadCornerDecisionQueryTests;
	FCutInsideShotDeadCornerDecisionQueryInput Input = MakeInput();
	Input.CurrentActionPoint = 7;
	return ExpectError(
		*this,
		Input,
		MakePlayerCardSnapshots(),
		MakeSkillRules(),
		ECutInsideShotDeadCornerDecisionQueryErrorCode
			::ActionPointOutOfRange,
		TEXT("CurrentActionPoint"));
}

CUT_INSIDE_DEAD_CORNER_TEST(
	FCutInsideDeadCornerRejectsGoalkeeperTest,
	"RejectsGoalkeeperAttacker")

bool FCutInsideDeadCornerRejectsGoalkeeperTest::RunTest(
	const FString& Parameters)
{
	using namespace CutInsideShotDeadCornerDecisionQueryTests;
	return ExpectError(
		*this,
		MakeInput(),
		MakePlayerCardSnapshots(true, true),
		MakeSkillRules(),
		ECutInsideShotDeadCornerDecisionQueryErrorCode
			::UnsupportedGoalkeeperParticipant,
		TEXT("AttackerCardId"));
}

CUT_INSIDE_DEAD_CORNER_TEST(
	FCutInsideDeadCornerRejectsMissingD6ATest,
	"RejectsMissingExternalAttackD6A")

bool FCutInsideDeadCornerRejectsMissingD6ATest::RunTest(
	const FString& Parameters)
{
	using namespace CutInsideShotDeadCornerDecisionQueryTests;
	FCutInsideShotDeadCornerDecisionQueryInput Input = MakeInput();
	Input.bHasExternalAttackD6A = false;
	return ExpectError(
		*this,
		Input,
		MakePlayerCardSnapshots(),
		MakeSkillRules(),
		ECutInsideShotDeadCornerDecisionQueryErrorCode
			::MissingExternalAttackD6A,
		TEXT("ExternalAttackD6A"));
}

CUT_INSIDE_DEAD_CORNER_TEST(
	FCutInsideDeadCornerRejectsMissingD6BTest,
	"RejectsMissingExternalAttackD6B")

bool FCutInsideDeadCornerRejectsMissingD6BTest::RunTest(
	const FString& Parameters)
{
	using namespace CutInsideShotDeadCornerDecisionQueryTests;
	FCutInsideShotDeadCornerDecisionQueryInput Input = MakeInput();
	Input.bHasExternalAttackD6B = false;
	return ExpectError(
		*this,
		Input,
		MakePlayerCardSnapshots(),
		MakeSkillRules(),
		ECutInsideShotDeadCornerDecisionQueryErrorCode
			::MissingExternalAttackD6B,
		TEXT("ExternalAttackD6B"));
}

CUT_INSIDE_DEAD_CORNER_TEST(
	FCutInsideDeadCornerRejectsInvalidD6ATest,
	"RejectsInvalidAttackD6A")

bool FCutInsideDeadCornerRejectsInvalidD6ATest::RunTest(
	const FString& Parameters)
{
	using namespace CutInsideShotDeadCornerDecisionQueryTests;
	FCutInsideShotDeadCornerDecisionQueryInput Input = MakeInput();
	Input.ExternalAttackD6A = 0;
	return ExpectError(
		*this,
		Input,
		MakePlayerCardSnapshots(),
		MakeSkillRules(),
		ECutInsideShotDeadCornerDecisionQueryErrorCode
			::InvalidAttackD6A,
		TEXT("ExternalAttackD6A"));
}

CUT_INSIDE_DEAD_CORNER_TEST(
	FCutInsideDeadCornerRejectsInvalidD6BTest,
	"RejectsInvalidAttackD6B")

bool FCutInsideDeadCornerRejectsInvalidD6BTest::RunTest(
	const FString& Parameters)
{
	using namespace CutInsideShotDeadCornerDecisionQueryTests;
	FCutInsideShotDeadCornerDecisionQueryInput Input = MakeInput();
	Input.ExternalAttackD6B = 7;
	return ExpectError(
		*this,
		Input,
		MakePlayerCardSnapshots(),
		MakeSkillRules(),
		ECutInsideShotDeadCornerDecisionQueryErrorCode
			::InvalidAttackD6B,
		TEXT("ExternalAttackD6B"));
}

CUT_INSIDE_DEAD_CORNER_TEST(
	FCutInsideDeadCornerTotalElevenGoalTest,
	"AttackD6TotalElevenReturnsGoal")

bool FCutInsideDeadCornerTotalElevenGoalTest::RunTest(
	const FString& Parameters)
{
	using namespace CutInsideShotDeadCornerDecisionQueryTests;
	const FCutInsideShotDeadCornerDecisionQueryResult Result =
		FCutInsideShotDeadCornerDecisionQuery::Evaluate(
			MakePlayerCardSnapshots(),
			MakeSkillRules(),
			MakeInput(5, 6));
	TestTrue(TEXT("Total eleven succeeds"), Result.bSuccess);
	TestEqual(
		TEXT("Total eleven is Goal"),
		Result.Decision,
		ECutInsideShotDeadCornerDecision::Goal);
	TestTrue(TEXT("Goal flag is true"), Result.bIsGoal);
	TestTrue(TEXT("Goal ends attack"), Result.bAttackEnded);
	return true;
}

CUT_INSIDE_DEAD_CORNER_TEST(
	FCutInsideDeadCornerTotalTwelveGoalTest,
	"AttackD6TotalTwelveReturnsGoal")

bool FCutInsideDeadCornerTotalTwelveGoalTest::RunTest(
	const FString& Parameters)
{
	using namespace CutInsideShotDeadCornerDecisionQueryTests;
	const FCutInsideShotDeadCornerDecisionQueryResult Result =
		FCutInsideShotDeadCornerDecisionQuery::Evaluate(
			MakePlayerCardSnapshots(),
			MakeSkillRules(),
			MakeInput(6, 6));
	TestTrue(TEXT("Total twelve succeeds"), Result.bSuccess);
	TestEqual(
		TEXT("Total twelve is Goal"),
		Result.Decision,
		ECutInsideShotDeadCornerDecision::Goal);
	TestTrue(TEXT("Goal flag is true"), Result.bIsGoal);
	TestTrue(TEXT("Goal ends attack"), Result.bAttackEnded);
	return true;
}

CUT_INSIDE_DEAD_CORNER_TEST(
	FCutInsideDeadCornerTotalBelowElevenMissTest,
	"AttackD6TotalBelowElevenReturnsMiss")

bool FCutInsideDeadCornerTotalBelowElevenMissTest::RunTest(
	const FString& Parameters)
{
	using namespace CutInsideShotDeadCornerDecisionQueryTests;
	const FCutInsideShotDeadCornerDecisionQueryResult Result =
		FCutInsideShotDeadCornerDecisionQuery::Evaluate(
			MakePlayerCardSnapshots(),
			MakeSkillRules(),
			MakeInput(5, 5));
	TestTrue(TEXT("Total ten succeeds"), Result.bSuccess);
	TestEqual(
		TEXT("Total ten is Miss"),
		Result.Decision,
		ECutInsideShotDeadCornerDecision::Miss);
	TestFalse(TEXT("Miss is not a goal"), Result.bIsGoal);
	TestTrue(TEXT("Miss ends attack"), Result.bAttackEnded);
	return true;
}

CUT_INSIDE_DEAD_CORNER_TEST(
	FCutInsideDeadCornerOtherTotalMissTest,
	"AttackD6TotalOtherThanElevenOrTwelveReturnsMiss")

bool FCutInsideDeadCornerOtherTotalMissTest::RunTest(
	const FString& Parameters)
{
	using namespace CutInsideShotDeadCornerDecisionQueryTests;
	const FCutInsideShotDeadCornerDecisionQueryResult Result =
		FCutInsideShotDeadCornerDecisionQuery::Evaluate(
			MakePlayerCardSnapshots(),
			MakeSkillRules(),
			MakeInput(1, 1));
	TestTrue(TEXT("Total two succeeds"), Result.bSuccess);
	TestEqual(
		TEXT("Total two is Miss"),
		Result.Decision,
		ECutInsideShotDeadCornerDecision::Miss);
	TestFalse(TEXT("Miss is not a goal"), Result.bIsGoal);
	TestTrue(TEXT("Miss ends attack"), Result.bAttackEnded);
	return true;
}

CUT_INSIDE_DEAD_CORNER_TEST(
	FCutInsideDeadCornerGoalAndMissEndTest,
	"GoalAndMissBothEndAttack")

bool FCutInsideDeadCornerGoalAndMissEndTest::RunTest(
	const FString& Parameters)
{
	using namespace CutInsideShotDeadCornerDecisionQueryTests;
	const FCutInsideShotDeadCornerDecisionQueryResult Goal =
		FCutInsideShotDeadCornerDecisionQuery::Evaluate(
			MakePlayerCardSnapshots(),
			MakeSkillRules(),
			MakeInput(6, 5));
	const FCutInsideShotDeadCornerDecisionQueryResult Miss =
		FCutInsideShotDeadCornerDecisionQuery::Evaluate(
			MakePlayerCardSnapshots(),
			MakeSkillRules(),
			MakeInput(4, 4));
	TestTrue(TEXT("Goal ends attack"), Goal.bAttackEnded);
	TestTrue(TEXT("Miss ends attack"), Miss.bAttackEnded);
	TestTrue(TEXT("Goal is goal"), Goal.bIsGoal);
	TestFalse(TEXT("Miss is not goal"), Miss.bIsGoal);
	return true;
}

CUT_INSIDE_DEAD_CORNER_TEST(
	FCutInsideDeadCornerNoFormulaPlanTest,
	"DoesNotGenerateFormulaPlan")

bool FCutInsideDeadCornerNoFormulaPlanTest::RunTest(
	const FString& Parameters)
{
	using namespace CutInsideShotDeadCornerDecisionQueryTests;
	FString Source;
	TestTrue(TEXT("Query source loads"), LoadQuerySource(Source));
	TestFalse(
		TEXT("Query has no Formula Plan"),
		Source.Contains(TEXT("FormulaPlan"))
			|| Source.Contains(TEXT("Formula Plan")));
	return true;
}

CUT_INSIDE_DEAD_CORNER_TEST(
	FCutInsideDeadCornerNoFormulaChainTest,
	"DoesNotCallFormulaChain")

bool FCutInsideDeadCornerNoFormulaChainTest::RunTest(
	const FString& Parameters)
{
	using namespace CutInsideShotDeadCornerDecisionQueryTests;
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

CUT_INSIDE_DEAD_CORNER_TEST(
	FCutInsideDeadCornerNoPlayerAttributeReadsTest,
	"DoesNotReadPlayerFormulaAttributes")

bool FCutInsideDeadCornerNoPlayerAttributeReadsTest::RunTest(
	const FString& Parameters)
{
	using namespace CutInsideShotDeadCornerDecisionQueryTests;
	FString Source;
	TestTrue(TEXT("Query source loads"), LoadQuerySource(Source));
	TestFalse(
		TEXT("Query does not read Shooting"),
		Source.Contains(TEXT("Shooting")));
	TestFalse(
		TEXT("Query does not read Dribbling"),
		Source.Contains(TEXT("Dribbling")));
	TestFalse(
		TEXT("Query does not read Tackling"),
		Source.Contains(TEXT("Tackling")));
	return true;
}

CUT_INSIDE_DEAD_CORNER_TEST(
	FCutInsideDeadCornerSnapshotDiagnosticsTest,
	"PreservesSnapshotQueryDiagnostics")

bool FCutInsideDeadCornerSnapshotDiagnosticsTest::RunTest(
	const FString& Parameters)
{
	using namespace CutInsideShotDeadCornerDecisionQueryTests;
	FCutInsideShotDeadCornerDecisionQueryInput Input = MakeInput();
	Input.AttackerCardId = TEXT("CutInsideDeadCorner.Missing");
	const FCutInsideShotDeadCornerDecisionQueryResult Result =
		FCutInsideShotDeadCornerDecisionQuery::Evaluate(
			MakePlayerCardSnapshots(),
			MakeSkillRules(),
			Input);
	TestEqual(
		TEXT("Top-level attacker query error"),
		Result.ErrorCode,
		ECutInsideShotDeadCornerDecisionQueryErrorCode
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

CUT_INSIDE_DEAD_CORNER_TEST(
	FCutInsideDeadCornerSkillDiagnosticsTest,
	"PreservesSkillRuleQueryDiagnostics")

bool FCutInsideDeadCornerSkillDiagnosticsTest::RunTest(
	const FString& Parameters)
{
	using namespace CutInsideShotDeadCornerDecisionQueryTests;
	const FCutInsideShotDeadCornerDecisionQueryResult Result =
		FCutInsideShotDeadCornerDecisionQuery::Evaluate(
			MakePlayerCardSnapshots(),
			FSkillRuleSnapshotSet{},
			MakeInput());
	TestEqual(
		TEXT("Top-level skill query error"),
		Result.ErrorCode,
		ECutInsideShotDeadCornerDecisionQueryErrorCode
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

CUT_INSIDE_DEAD_CORNER_TEST(
	FCutInsideDeadCornerInputImmutabilityTest,
	"DoesNotMutateInput")

bool FCutInsideDeadCornerInputImmutabilityTest::RunTest(
	const FString& Parameters)
{
	using namespace CutInsideShotDeadCornerDecisionQueryTests;
	const FCutInsideShotDeadCornerDecisionQueryInput Input =
		MakeInput();
	const FCutInsideShotDeadCornerDecisionQueryInput Before = Input;
	const FCutInsideShotDeadCornerDecisionQueryResult Result =
		FCutInsideShotDeadCornerDecisionQuery::Evaluate(
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

CUT_INSIDE_DEAD_CORNER_TEST(
	FCutInsideDeadCornerPlayerSnapshotsImmutabilityTest,
	"DoesNotMutatePlayerCardSnapshotSet")

bool FCutInsideDeadCornerPlayerSnapshotsImmutabilityTest::RunTest(
	const FString& Parameters)
{
	using namespace CutInsideShotDeadCornerDecisionQueryTests;
	const FPlayerCardRuleSnapshotSet SnapshotSet =
		MakePlayerCardSnapshots();
	const FPlayerCardRuleSnapshotSet Before = SnapshotSet;
	FCutInsideShotDeadCornerDecisionQuery::Evaluate(
		SnapshotSet,
		MakeSkillRules(),
		MakeInput());
	TestTrue(
		TEXT("Player Snapshot Set remains unchanged"),
		ArePlayerSnapshotSetsEqual(SnapshotSet, Before));
	return true;
}

CUT_INSIDE_DEAD_CORNER_TEST(
	FCutInsideDeadCornerSkillRulesImmutabilityTest,
	"DoesNotMutateSkillRuleSnapshotSet")

bool FCutInsideDeadCornerSkillRulesImmutabilityTest::RunTest(
	const FString& Parameters)
{
	using namespace CutInsideShotDeadCornerDecisionQueryTests;
	const FSkillRuleSnapshotSet SnapshotSet = MakeSkillRules();
	const FSkillRuleSnapshotSet Before = SnapshotSet;
	FCutInsideShotDeadCornerDecisionQuery::Evaluate(
		MakePlayerCardSnapshots(),
		SnapshotSet,
		MakeInput());
	TestTrue(
		TEXT("Skill Rule Snapshot Set remains unchanged"),
		AreSkillRuleSnapshotSetsEqual(SnapshotSet, Before));
	return true;
}

CUT_INSIDE_DEAD_CORNER_TEST(
	FCutInsideDeadCornerInvalidLogContextTest,
	"RejectsInvalidLogContext")

bool FCutInsideDeadCornerInvalidLogContextTest::RunTest(
	const FString& Parameters)
{
	using namespace CutInsideShotDeadCornerDecisionQueryTests;
	FCutInsideShotDeadCornerDecisionQueryInput Input = MakeInput();
	Input.LogId.Invalidate();
	return ExpectError(
		*this,
		Input,
		MakePlayerCardSnapshots(),
		MakeSkillRules(),
		ECutInsideShotDeadCornerDecisionQueryErrorCode::InvalidLogContext,
		TEXT("LogId"));
}

CUT_INSIDE_DEAD_CORNER_TEST(
	FCutInsideDeadCornerForbiddenDependenciesTest,
	"HasNoForbiddenDependencies")

bool FCutInsideDeadCornerForbiddenDependenciesTest::RunTest(
	const FString& Parameters)
{
	using namespace CutInsideShotDeadCornerDecisionQueryTests;
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
		TEXT("DeadCornerDecisionQueryBase"),
		TEXT("FMath::Rand"),
		TEXT("FRandomStream"),
		TEXT("EFormulaType::Determination"),
		TEXT("LongShotDeadCorner"),
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

#undef CUT_INSIDE_DEAD_CORNER_TEST

#endif
