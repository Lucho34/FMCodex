#include "MatchPlayCurrentAttackSkillSelectionAvailability.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "MatchPlayCurrentAttackSkillSelectionTestFixtures.h"
#include "Misc/AutomationTest.h"

#define SKILL_AVAILABILITY_TEST(TestClass, TestName) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
		TestClass, \
		"FMCodex.CoreRules.MatchPlayCurrentAttackSkillSelectionAvailability." TestName, \
		EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

SKILL_AVAILABILITY_TEST(
	FSkillAvailabilityOrderAndCandidateResultsTest,
	"OriginalOrderAndCompleteCandidateResults")

bool FSkillAvailabilityOrderAndCandidateResultsTest::RunTest(
	const FString& Parameters)
{
	using namespace
		FMCodex::Tests::MatchPlayCurrentAttackSkillSelection;
	const FMatchPlayState State =
		MakeState(
			{CrossSkillId, LongShotSkillId, PassControlSkillId});
	const FSkillRuleSnapshotSet Rules = MakeRuleSet();
	const auto Result =
		FMatchPlayCurrentAttackSkillSelectionAvailability::Query(
			State,
			ValidAttackSequence,
			EInitialTurnOrderPlayer::PlayerA,
			Rules);

	TestTrue(TEXT("Query succeeds"), Result.bQuerySucceeded);
	TestTrue(TEXT("Any legal"), Result.bCanSelectAnySkill);
	TestFalse(
		TEXT("No global blocker"),
		Result.bHasGlobalBlockingLegalityResult);
	TestEqual(TEXT("Three candidates"), Result.Candidates.Num(), 3);
	TestEqual(
		TEXT("Original first"),
		Result.Candidates[0].SkillId,
		CrossSkillId);
	TestEqual(
		TEXT("Original second"),
		Result.Candidates[1].SkillId,
		LongShotSkillId);
	TestEqual(
		TEXT("Original third"),
		Result.Candidates[2].SkillId,
		PassControlSkillId);
	for (const auto& Candidate : Result.Candidates)
	{
		TestEqual(
			TEXT("Complete legality request retained"),
			Candidate.LegalityResult.Request.SkillId,
			Candidate.SkillId);
		TestTrue(
			TEXT("Candidate legality retained"),
			Candidate.LegalityResult.bIsLegal);
	}
	return true;
}

SKILL_AVAILABILITY_TEST(
	FSkillAvailabilityZeroLegalTest,
	"EmptyAndActionPointMismatchAreSuccessfulZeroLegal")

bool FSkillAvailabilityZeroLegalTest::RunTest(
	const FString& Parameters)
{
	using namespace
		FMCodex::Tests::MatchPlayCurrentAttackSkillSelection;
	const FSkillRuleSnapshotSet Rules = MakeRuleSet();
	const auto EmptyResult =
		FMatchPlayCurrentAttackSkillSelectionAvailability::Query(
			MakeState({}),
			ValidAttackSequence,
			EInitialTurnOrderPlayer::PlayerA,
			Rules);
	TestTrue(
		TEXT("Empty skills query succeeds"),
		EmptyResult.bQuerySucceeded);
	TestFalse(
		TEXT("Empty skills has no legal"),
		EmptyResult.bCanSelectAnySkill);
	TestEqual(
		TEXT("Empty skills no candidates"),
		EmptyResult.Candidates.Num(),
		0);

	FMatchPlayState RangeState = MakeState({LongShotSkillId});
	FSkillRuleSnapshotSet RangeRules;
	RangeRules.SkillRules = {
		MakeRule(
			LongShotSkillId,
			ESkillRuleType::LongShot,
			6,
			8)
	};
	const auto RangeResult =
		FMatchPlayCurrentAttackSkillSelectionAvailability::Query(
			RangeState,
			ValidAttackSequence,
			EInitialTurnOrderPlayer::PlayerA,
			RangeRules);
	TestTrue(
		TEXT("AP mismatch query succeeds"),
		RangeResult.bQuerySucceeded);
	TestFalse(
		TEXT("AP mismatch has no legal"),
		RangeResult.bCanSelectAnySkill);
	TestEqual(
		TEXT("AP mismatch candidate retained"),
		RangeResult.Candidates.Num(),
		1);
	TestEqual(
		TEXT("AP mismatch exact candidate error"),
		RangeResult.Candidates[0].LegalityResult.ErrorCode,
		EMatchPlayCurrentAttackSkillSelectionErrorCode
			::ActionPointOutsideSkillRange);
	return true;
}

SKILL_AVAILABILITY_TEST(
	FSkillAvailabilityMixedCandidateTest,
	"PartialLegalMissingRuleAndNoSilentFiltering")

bool FSkillAvailabilityMixedCandidateTest::RunTest(
	const FString& Parameters)
{
	using namespace
		FMCodex::Tests::MatchPlayCurrentAttackSkillSelection;
	FSkillRuleSnapshotSet Rules;
	Rules.SkillRules = {
		MakeRule(
			LongShotSkillId,
			ESkillRuleType::LongShot,
			2,
			8),
		MakeRule(CrossSkillId, ESkillRuleType::Cross, 6, 8)
	};
	const FMatchPlayState State =
		MakeState(
			{CrossSkillId, MissingSkillId, LongShotSkillId});
	const auto Result =
		FMatchPlayCurrentAttackSkillSelectionAvailability::Query(
			State,
			ValidAttackSequence,
			EInitialTurnOrderPlayer::PlayerA,
			Rules);
	TestTrue(TEXT("Mixed query succeeds"), Result.bQuerySucceeded);
	TestTrue(TEXT("Mixed contains legal"), Result.bCanSelectAnySkill);
	TestEqual(
		TEXT("All candidates retained"),
		Result.Candidates.Num(),
		3);
	TestEqual(
		TEXT("First AP error"),
		Result.Candidates[0].LegalityResult.ErrorCode,
		EMatchPlayCurrentAttackSkillSelectionErrorCode
			::ActionPointOutsideSkillRange);
	TestEqual(
		TEXT("Missing rule candidate error"),
		Result.Candidates[1].LegalityResult.ErrorCode,
		EMatchPlayCurrentAttackSkillSelectionErrorCode
			::SkillRuleNotFound);
	TestTrue(
		TEXT("Final legal"),
		Result.Candidates[2].LegalityResult.bIsLegal);
	return true;
}

SKILL_AVAILABILITY_TEST(
	FSkillAvailabilityGlobalBlockersTest,
	"DuplicateDataWrongStageAndStaleSequenceBlockGlobally")

bool FSkillAvailabilityGlobalBlockersTest::RunTest(
	const FString& Parameters)
{
	using namespace
		FMCodex::Tests::MatchPlayCurrentAttackSkillSelection;
	{
		const auto Result =
			FMatchPlayCurrentAttackSkillSelectionAvailability
				::Query(
					MakeState(
						{LongShotSkillId, LongShotSkillId}),
					ValidAttackSequence,
					EInitialTurnOrderPlayer::PlayerA,
					MakeRuleSet());
		TestFalse(TEXT("Duplicate query fails"), Result.bQuerySucceeded);
		TestTrue(
			TEXT("Duplicate global blocker"),
			Result.bHasGlobalBlockingLegalityResult);
		TestEqual(
			TEXT("Duplicate exact error"),
			Result.GlobalBlockingLegalityResult.ErrorCode,
			EMatchPlayCurrentAttackSkillSelectionErrorCode
				::DuplicateCarrierSkillId);
	}
	{
		FSkillRuleSnapshotSet Rules = MakeRuleSet();
		const FSkillRuleSnapshot DuplicateRule =
			Rules.SkillRules[0];
		Rules.SkillRules.Add(DuplicateRule);
		const auto Result =
			FMatchPlayCurrentAttackSkillSelectionAvailability
				::Query(
					MakeState({LongShotSkillId}),
					ValidAttackSequence,
					EInitialTurnOrderPlayer::PlayerA,
					Rules);
		TestFalse(
			TEXT("Duplicate rule query fails"),
			Result.bQuerySucceeded);
		TestEqual(
			TEXT("Duplicate rule global error"),
			Result.GlobalBlockingLegalityResult.ErrorCode,
			EMatchPlayCurrentAttackSkillSelectionErrorCode
				::SkillRuleAmbiguous);
	}
	{
		FSkillRuleSnapshotSet Rules = MakeRuleSet();
		Rules.SkillRules[0].SkillType =
			static_cast<ESkillRuleType>(255);
		const auto Result =
			FMatchPlayCurrentAttackSkillSelectionAvailability
				::Query(
					MakeState({LongShotSkillId}),
					ValidAttackSequence,
					EInitialTurnOrderPlayer::PlayerA,
					Rules);
		TestFalse(
			TEXT("Unsupported rule set query fails"),
			Result.bQuerySucceeded);
		TestTrue(
			TEXT("Unsupported rule is a global rule-set blocker"),
			Result.bHasGlobalBlockingLegalityResult);
		TestEqual(
			TEXT("Unsupported rule exact global error"),
			Result.GlobalBlockingLegalityResult.ErrorCode,
			EMatchPlayCurrentAttackSkillSelectionErrorCode
				::InvalidSkillRuleSet);
	}
	{
		FMatchPlayState State = MakeState();
		State.CurrentAttack.SelectionStage =
			EMatchPlayCurrentAttackSelectionStage::AwaitingRunner;
		State.CurrentAttack.ActionPreparation.SkillId = CrossSkillId;
		State.CurrentAttack.ActionPreparation.ActionType =
			ESkillRuleType::Cross;
		const auto Result =
			FMatchPlayCurrentAttackSkillSelectionAvailability
				::Query(
					State,
					ValidAttackSequence,
					EInitialTurnOrderPlayer::PlayerA,
					MakeRuleSet());
		TestFalse(TEXT("Wrong stage fails"), Result.bQuerySucceeded);
		TestEqual(
			TEXT("Wrong stage global"),
			Result.GlobalBlockingLegalityResult.ErrorCode,
			EMatchPlayCurrentAttackSkillSelectionErrorCode
				::WrongSelectionStage);
	}
	{
		const auto Result =
			FMatchPlayCurrentAttackSkillSelectionAvailability
				::Query(
					MakeState(),
					ValidAttackSequence + 1,
					EInitialTurnOrderPlayer::PlayerA,
					MakeRuleSet());
		TestFalse(TEXT("Stale sequence fails"), Result.bQuerySucceeded);
		TestEqual(
			TEXT("Stale sequence global"),
			Result.GlobalBlockingLegalityResult.ErrorCode,
			EMatchPlayCurrentAttackSkillSelectionErrorCode
				::AttackSequenceMismatch);
	}
	return true;
}

SKILL_AVAILABILITY_TEST(
	FSkillAvailabilityReadOnlyDeterminismTest,
	"RepeatedDeterministicAndInputsUnchanged")

bool FSkillAvailabilityReadOnlyDeterminismTest::RunTest(
	const FString& Parameters)
{
	using namespace
		FMCodex::Tests::MatchPlayCurrentAttackSkillSelection;
	const FMatchPlayState State = MakeState();
	const FMatchPlayState OriginalState = State;
	const FSkillRuleSnapshotSet Rules = MakeRuleSet();
	const int32 OriginalRuleCount = Rules.SkillRules.Num();
	const auto First =
		FMatchPlayCurrentAttackSkillSelectionAvailability::Query(
			State,
			ValidAttackSequence,
			EInitialTurnOrderPlayer::PlayerA,
			Rules);
	const auto Second =
		FMatchPlayCurrentAttackSkillSelectionAvailability::Query(
			State,
			ValidAttackSequence,
			EInitialTurnOrderPlayer::PlayerA,
			Rules);
	TestEqual(
		TEXT("Repeated candidate count"),
		First.Candidates.Num(),
		Second.Candidates.Num());
	TestEqual(
		TEXT("Repeated any legal"),
		First.bCanSelectAnySkill,
		Second.bCanSelectAnySkill);
	TestTrue(
		TEXT("State unchanged"),
		AreStatesEqual(State, OriginalState));
	TestEqual(
		TEXT("Rule set unchanged"),
		Rules.SkillRules.Num(),
		OriginalRuleCount);
	return true;
}

#undef SKILL_AVAILABILITY_TEST

#endif
