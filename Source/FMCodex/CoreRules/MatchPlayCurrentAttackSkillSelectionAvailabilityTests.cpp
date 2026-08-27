#include "MatchPlayCurrentAttackSkillSelectionAvailability.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "MatchPlayCurrentAttackSkillSelectionTestFixtures.h"
#include "Misc/AutomationTest.h"

namespace SkillSelectionAvailabilityTests
{
	bool ArePlacementsEqual(
		const FMatchPlayDeploymentPlacement& Left,
		const FMatchPlayDeploymentPlacement& Right)
	{
		return FMatchPlayDeploymentPlacement::StaticStruct()
			->CompareScriptStruct(&Left, &Right, 0);
	}

	bool AreSnapshotsEqual(
		const FPlayerCardRuleSnapshot& Left,
		const FPlayerCardRuleSnapshot& Right)
	{
		return FPlayerCardRuleSnapshot::StaticStruct()
			->CompareScriptStruct(&Left, &Right, 0);
	}

	bool AreSelectionStateValidationResultsEqual(
		const FMatchPlayCurrentAttackSelectionStateValidationResult& Left,
		const FMatchPlayCurrentAttackSelectionStateValidationResult& Right)
	{
		return Left.bIsCanonical == Right.bIsCanonical
			&& Left.ErrorCode == Right.ErrorCode
			&& Left.ErrorMessage == Right.ErrorMessage;
	}

	bool ArePlayerSnapshotValidationResultsEqual(
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

	bool ArePlayerSnapshotQueryResultsEqual(
		const FPlayerCardRuleSnapshotQueryResult& Left,
		const FPlayerCardRuleSnapshotQueryResult& Right)
	{
		return Left.bSuccess == Right.bSuccess
			&& Left.bFound == Right.bFound
			&& Left.ErrorCode == Right.ErrorCode
			&& Left.ErrorMessage == Right.ErrorMessage
			&& Left.CardId == Right.CardId
			&& AreSnapshotsEqual(Left.Snapshot, Right.Snapshot)
			&& ArePlayerSnapshotValidationResultsEqual(
				Left.ValidationResult,
				Right.ValidationResult);
	}

	bool AreCarrierSnapshotQueryResultsEqual(
		const FMatchPlayCardSnapshotAuthorityQueryResult& Left,
		const FMatchPlayCardSnapshotAuthorityQueryResult& Right)
	{
		return Left.bSuccess == Right.bSuccess
			&& Left.ErrorCode == Right.ErrorCode
			&& Left.PlayerSide == Right.PlayerSide
			&& Left.CardId == Right.CardId
			&& AreSnapshotsEqual(Left.Snapshot, Right.Snapshot)
			&& ArePlayerSnapshotQueryResultsEqual(
				Left.UnderlyingQueryResult,
				Right.UnderlyingQueryResult)
			&& Left.ErrorMessage == Right.ErrorMessage;
	}

	bool AreSkillValidationResultsEqual(
		const FSkillRuleSnapshotValidationResult& Left,
		const FSkillRuleSnapshotValidationResult& Right)
	{
		return Left.bSuccess == Right.bSuccess
			&& Left.bIsValid == Right.bIsValid
			&& Left.ErrorCode == Right.ErrorCode
			&& Left.ErrorMessage == Right.ErrorMessage
			&& Left.InvalidSkillId == Right.InvalidSkillId
			&& Left.InvalidField == Right.InvalidField;
	}

	bool AreGlobalContextsEqual(
		const FMatchPlayCurrentAttackSkillSelectionGlobalContextResult&
			Left,
		const FMatchPlayCurrentAttackSkillSelectionGlobalContextResult&
			Right)
	{
		return Left.bSuccess == Right.bSuccess
			&& Left.RequestedAttackSequence
				== Right.RequestedAttackSequence
			&& Left.RequestingSide == Right.RequestingSide
			&& Left.ErrorCode == Right.ErrorCode
			&& Left.AuthoritativeAttackSequence
				== Right.AuthoritativeAttackSequence
			&& Left.CurrentAttackingPlayer
				== Right.CurrentAttackingPlayer
			&& Left.CurrentDefendingPlayer
				== Right.CurrentDefendingPlayer
			&& AreSelectionStateValidationResultsEqual(
				Left.SelectionStateValidationResult,
				Right.SelectionStateValidationResult)
			&& Left.FrozenCarrierCardId
				== Right.FrozenCarrierCardId
			&& Left.FrozenMarkerCardId
				== Right.FrozenMarkerCardId
			&& Left.MatchingFrozenCarrierPlacementCount
				== Right.MatchingFrozenCarrierPlacementCount
			&& Left.MatchingFrozenMarkerPlacementCount
				== Right.MatchingFrozenMarkerPlacementCount
			&& ArePlacementsEqual(
				Left.FrozenCarrierPlacement,
				Right.FrozenCarrierPlacement)
			&& ArePlacementsEqual(
				Left.FrozenMarkerPlacement,
				Right.FrozenMarkerPlacement)
			&& AreCarrierSnapshotQueryResultsEqual(
				Left.CarrierSnapshotQueryResult,
				Right.CarrierSnapshotQueryResult)
			&& AreSnapshotsEqual(
				Left.ResolvedCarrierSnapshot,
				Right.ResolvedCarrierSnapshot)
			&& AreSkillValidationResultsEqual(
				Left.SkillRuleSetValidationResult,
				Right.SkillRuleSetValidationResult)
			&& Left.ValidatedActionPoint
				== Right.ValidatedActionPoint
			&& Left.ErrorMessage == Right.ErrorMessage;
	}

	bool AreRulesEqual(
		const FSkillRuleSnapshot& Left,
		const FSkillRuleSnapshot& Right)
	{
		return Left.SkillId == Right.SkillId
			&& Left.SkillType == Right.SkillType
			&& Left.MinTriggerActionPoint
				== Right.MinTriggerActionPoint
			&& Left.MaxTriggerActionPoint
				== Right.MaxTriggerActionPoint;
	}

	bool AreSkillQueryResultsEqual(
		const FSkillRuleSnapshotQueryResult& Left,
		const FSkillRuleSnapshotQueryResult& Right)
	{
		return Left.bSuccess == Right.bSuccess
			&& Left.bFound == Right.bFound
			&& Left.ErrorCode == Right.ErrorCode
			&& Left.ErrorMessage == Right.ErrorMessage
			&& Left.InvalidSkillId == Right.InvalidSkillId
			&& Left.InvalidField == Right.InvalidField
			&& AreRulesEqual(Left.Snapshot, Right.Snapshot)
			&& AreSkillValidationResultsEqual(
				Left.ValidationResult,
				Right.ValidationResult);
	}

	bool AreLegalityResultsEqual(
		const FMatchPlayCurrentAttackSkillSelectionLegalityResult& Left,
		const FMatchPlayCurrentAttackSkillSelectionLegalityResult& Right)
	{
		return Left.bIsLegal == Right.bIsLegal
			&& Left.Request.AttackSequence
				== Right.Request.AttackSequence
			&& Left.Request.RequestingSide
				== Right.Request.RequestingSide
			&& Left.Request.SkillId == Right.Request.SkillId
			&& Left.ErrorCode == Right.ErrorCode
			&& AreSelectionStateValidationResultsEqual(
				Left.SelectionStateValidationResult,
				Right.SelectionStateValidationResult)
			&& AreGlobalContextsEqual(
				Left.GlobalContextResult,
				Right.GlobalContextResult)
			&& Left.FrozenCarrierCardId
				== Right.FrozenCarrierCardId
			&& Left.FrozenMarkerCardId
				== Right.FrozenMarkerCardId
			&& Left.MatchingFrozenCarrierPlacementCount
				== Right.MatchingFrozenCarrierPlacementCount
			&& Left.MatchingFrozenMarkerPlacementCount
				== Right.MatchingFrozenMarkerPlacementCount
			&& ArePlacementsEqual(
				Left.FrozenCarrierPlacement,
				Right.FrozenCarrierPlacement)
			&& ArePlacementsEqual(
				Left.FrozenMarkerPlacement,
				Right.FrozenMarkerPlacement)
			&& AreCarrierSnapshotQueryResultsEqual(
				Left.CarrierSnapshotQueryResult,
				Right.CarrierSnapshotQueryResult)
			&& AreSkillQueryResultsEqual(
				Left.SkillRuleQueryResult,
				Right.SkillRuleQueryResult)
			&& AreRulesEqual(
				Left.ResolvedSkillRule,
				Right.ResolvedSkillRule)
			&& Left.ResolvedActionType
				== Right.ResolvedActionType
			&& Left.ParticipantRequirementResult.bSuccess
				== Right.ParticipantRequirementResult.bSuccess
			&& Left.ParticipantRequirementResult.bRequiresRunner
				== Right.ParticipantRequirementResult.bRequiresRunner
			&& Left.ParticipantRequirementResult.bRequiresHelperStage
				== Right.ParticipantRequirementResult
					.bRequiresHelperStage
			&& Left.ParticipantRequirementResult
					.bCanBecomeReadyImmediately
				== Right.ParticipantRequirementResult
					.bCanBecomeReadyImmediately
			&& Left.ParticipantRequirementResult.ErrorCode
				== Right.ParticipantRequirementResult.ErrorCode
			&& Left.ParticipantRequirementResult.ErrorMessage
				== Right.ParticipantRequirementResult.ErrorMessage
			&& Left.ErrorMessage == Right.ErrorMessage;
	}

	bool AreAvailabilityResultsEqual(
		const FMatchPlayCurrentAttackSkillSelectionAvailabilityResult&
			Left,
		const FMatchPlayCurrentAttackSkillSelectionAvailabilityResult&
			Right)
	{
		if (Left.bQuerySucceeded != Right.bQuerySucceeded
			|| Left.bCanSelectAnySkill != Right.bCanSelectAnySkill
			|| Left.AttackSequence != Right.AttackSequence
			|| Left.RequestingSide != Right.RequestingSide
			|| !AreGlobalContextsEqual(
				Left.GlobalContextResult,
				Right.GlobalContextResult)
			|| !AreCarrierSnapshotQueryResultsEqual(
				Left.CarrierSnapshotQueryResult,
				Right.CarrierSnapshotQueryResult)
			|| !AreSkillValidationResultsEqual(
				Left.SkillRuleSetValidationResult,
				Right.SkillRuleSetValidationResult)
			|| Left.Candidates.Num() != Right.Candidates.Num())
		{
			return false;
		}
		for (int32 Index = 0; Index < Left.Candidates.Num(); ++Index)
		{
			if (Left.Candidates[Index].SkillId
					!= Right.Candidates[Index].SkillId
				|| !AreLegalityResultsEqual(
					Left.Candidates[Index].LegalityResult,
					Right.Candidates[Index].LegalityResult))
			{
				return false;
			}
		}
		return true;
	}

	bool AreRuleSetsEqual(
		const FSkillRuleSnapshotSet& Left,
		const FSkillRuleSnapshotSet& Right)
	{
		if (Left.SkillRules.Num() != Right.SkillRules.Num())
		{
			return false;
		}
		for (int32 Index = 0; Index < Left.SkillRules.Num(); ++Index)
		{
			if (!AreRulesEqual(
				Left.SkillRules[Index],
				Right.SkillRules[Index]))
			{
				return false;
			}
		}
		return true;
	}
}

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
	TestTrue(
		TEXT("Global context succeeded"),
		Result.GlobalContextResult.bSuccess);
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
	FSkillAvailabilityGlobalActionPointTest,
	"GlobalActionPointPrecedesEmptyAndEarlyCandidateFailures")

bool FSkillAvailabilityGlobalActionPointTest::RunTest(
	const FString& Parameters)
{
	using namespace
		FMCodex::Tests::MatchPlayCurrentAttackSkillSelection;
	const FSkillRuleSnapshotSet Rules = MakeRuleSet();

	for (const int32 ActionPoint : {1, 9})
	{
		FMatchPlayState EmptyState = MakeState({});
		EmptyState.CurrentAttack.ActionPoint = ActionPoint;
		const auto EmptyResult =
			FMatchPlayCurrentAttackSkillSelectionAvailability::Query(
				EmptyState,
				ValidAttackSequence,
				EInitialTurnOrderPlayer::PlayerA,
				Rules);
		TestFalse(
			TEXT("Empty invalid AP fails globally"),
			EmptyResult.bQuerySucceeded);
		TestFalse(
			TEXT("Empty invalid AP global context fails"),
			EmptyResult.GlobalContextResult.bSuccess);
		TestEqual(
			TEXT("Empty invalid AP exact error"),
			EmptyResult.GlobalContextResult.ErrorCode,
			EMatchPlayCurrentAttackSkillSelectionErrorCode
				::InvalidCurrentAttackActionPoint);
		TestEqual(
			TEXT("Empty invalid AP has no candidates"),
			EmptyResult.Candidates.Num(),
			0);
		TestFalse(
			TEXT("Empty invalid AP has no legal candidate"),
			EmptyResult.bCanSelectAnySkill);

		FMatchPlayState MissingRuleState =
			MakeState({MissingSkillId});
		MissingRuleState.CurrentAttack.ActionPoint = ActionPoint;
		const auto MissingRuleResult =
			FMatchPlayCurrentAttackSkillSelectionAvailability::Query(
				MissingRuleState,
				ValidAttackSequence,
				EInitialTurnOrderPlayer::PlayerA,
				Rules);
		TestFalse(
			TEXT("Missing rule cannot hide invalid AP"),
			MissingRuleResult.bQuerySucceeded);
		TestEqual(
			TEXT("Global AP precedes missing candidate rule"),
			MissingRuleResult.GlobalContextResult.ErrorCode,
			EMatchPlayCurrentAttackSkillSelectionErrorCode
				::InvalidCurrentAttackActionPoint);
		TestEqual(
			TEXT("Missing rule invalid AP enumerates nothing"),
			MissingRuleResult.Candidates.Num(),
			0);

		FMatchPlayState AllEarlyFailuresState =
			MakeState({MissingSkillId, FName(TEXT("Skill.MissingTwo"))});
		AllEarlyFailuresState.CurrentAttack.ActionPoint =
			ActionPoint;
		const auto AllEarlyFailuresResult =
			FMatchPlayCurrentAttackSkillSelectionAvailability::Query(
				AllEarlyFailuresState,
				ValidAttackSequence,
				EInitialTurnOrderPlayer::PlayerA,
				Rules);
		TestFalse(
			TEXT("All early failures cannot hide invalid AP"),
			AllEarlyFailuresResult.bQuerySucceeded);
		TestEqual(
			TEXT("All early failures exact global AP error"),
			AllEarlyFailuresResult.GlobalContextResult.ErrorCode,
			EMatchPlayCurrentAttackSkillSelectionErrorCode
				::InvalidCurrentAttackActionPoint);
		TestEqual(
			TEXT("All early failures enumerate nothing"),
			AllEarlyFailuresResult.Candidates.Num(),
			0);
	}
	return true;
}

SKILL_AVAILABILITY_TEST(
	FSkillAvailabilityBoundaryAndDeterminismTest,
	"EmptyBoundariesAndFullResultDeterminism")

bool FSkillAvailabilityBoundaryAndDeterminismTest::RunTest(
	const FString& Parameters)
{
	using namespace
		FMCodex::Tests::MatchPlayCurrentAttackSkillSelection;
	using namespace SkillSelectionAvailabilityTests;
	const FSkillRuleSnapshotSet Rules = MakeRuleSet();

	for (const int32 ActionPoint : {2, 8})
	{
		FMatchPlayState State = MakeState({});
		State.CurrentAttack.ActionPoint = ActionPoint;
		const auto Result =
			FMatchPlayCurrentAttackSkillSelectionAvailability::Query(
				State,
				ValidAttackSequence,
				EInitialTurnOrderPlayer::PlayerA,
				Rules);
		TestTrue(
			TEXT("Empty boundary query succeeds"),
			Result.bQuerySucceeded);
		TestFalse(
			TEXT("Empty boundary has no legal candidate"),
			Result.bCanSelectAnySkill);
		TestEqual(
			TEXT("Empty boundary candidates empty"),
			Result.Candidates.Num(),
			0);
	}

	FSkillRuleSnapshotSet CorruptRules = MakeRuleSet();
	CorruptRules.SkillRules[0].SkillId = NAME_None;
	const auto EmptyCorruptRules =
		FMatchPlayCurrentAttackSkillSelectionAvailability::Query(
			MakeState({}),
			ValidAttackSequence,
			EInitialTurnOrderPlayer::PlayerA,
			CorruptRules);
	TestFalse(
		TEXT("Empty skills do not hide corrupt RuleSet"),
		EmptyCorruptRules.bQuerySucceeded);
	TestEqual(
		TEXT("Empty corrupt RuleSet exact global error"),
		EmptyCorruptRules.GlobalContextResult.ErrorCode,
		EMatchPlayCurrentAttackSkillSelectionErrorCode
			::InvalidSkillRuleSet);

	const FMatchPlayState EmptyState = MakeState({});
	const auto EmptyFirst =
		FMatchPlayCurrentAttackSkillSelectionAvailability::Query(
			EmptyState,
			ValidAttackSequence,
			EInitialTurnOrderPlayer::PlayerA,
			Rules);
	const auto EmptySecond =
		FMatchPlayCurrentAttackSkillSelectionAvailability::Query(
			EmptyState,
			ValidAttackSequence,
			EInitialTurnOrderPlayer::PlayerA,
			Rules);
	TestTrue(
		TEXT("Empty result fully deterministic"),
		AreAvailabilityResultsEqual(EmptyFirst, EmptySecond));

	const FMatchPlayState AllInvalidState =
		MakeState({MissingSkillId, FName(TEXT("Skill.MissingTwo"))});
	const auto InvalidFirst =
		FMatchPlayCurrentAttackSkillSelectionAvailability::Query(
			AllInvalidState,
			ValidAttackSequence,
			EInitialTurnOrderPlayer::PlayerA,
			Rules);
	const auto InvalidSecond =
		FMatchPlayCurrentAttackSkillSelectionAvailability::Query(
			AllInvalidState,
			ValidAttackSequence,
			EInitialTurnOrderPlayer::PlayerA,
			Rules);
	TestTrue(
		TEXT("All-invalid result fully deterministic"),
		AreAvailabilityResultsEqual(InvalidFirst, InvalidSecond));
	TestTrue(
		TEXT("All-invalid query itself succeeds"),
		InvalidFirst.bQuerySucceeded);
	TestEqual(
		TEXT("All-invalid candidates retained"),
		InvalidFirst.Candidates.Num(),
		2);

	const FMatchPlayState MixedState =
		MakeState({CrossSkillId, MissingSkillId, LongShotSkillId});
	const auto MixedFirst =
		FMatchPlayCurrentAttackSkillSelectionAvailability::Query(
			MixedState,
			ValidAttackSequence,
			EInitialTurnOrderPlayer::PlayerA,
			Rules);
	const auto MixedSecond =
		FMatchPlayCurrentAttackSkillSelectionAvailability::Query(
			MixedState,
			ValidAttackSequence,
			EInitialTurnOrderPlayer::PlayerA,
			Rules);
	TestTrue(
		TEXT("Mixed result fully deterministic"),
		AreAvailabilityResultsEqual(MixedFirst, MixedSecond));

	FMatchPlayState GlobalFailureState = MakeState();
	GlobalFailureState.CurrentAttack.ActionPoint = 1;
	const auto GlobalFailureFirst =
		FMatchPlayCurrentAttackSkillSelectionAvailability::Query(
			GlobalFailureState,
			ValidAttackSequence,
			EInitialTurnOrderPlayer::PlayerA,
			Rules);
	const auto GlobalFailureSecond =
		FMatchPlayCurrentAttackSkillSelectionAvailability::Query(
			GlobalFailureState,
			ValidAttackSequence,
			EInitialTurnOrderPlayer::PlayerA,
			Rules);
	TestTrue(
		TEXT("Global-failure result fully deterministic"),
		AreAvailabilityResultsEqual(
			GlobalFailureFirst,
			GlobalFailureSecond));
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
		TestFalse(
			TEXT("Duplicate global context fails"),
			Result.GlobalContextResult.bSuccess);
		TestEqual(
			TEXT("Duplicate exact error"),
			Result.GlobalContextResult.ErrorCode,
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
			Result.GlobalContextResult.ErrorCode,
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
		TestFalse(
			TEXT("Unsupported rule fails global context"),
			Result.GlobalContextResult.bSuccess);
		TestEqual(
			TEXT("Unsupported rule exact global error"),
			Result.GlobalContextResult.ErrorCode,
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
			Result.GlobalContextResult.ErrorCode,
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
			Result.GlobalContextResult.ErrorCode,
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
	using namespace SkillSelectionAvailabilityTests;
	const FMatchPlayState State = MakeState();
	const FMatchPlayState OriginalState = State;
	const FSkillRuleSnapshotSet Rules = MakeRuleSet();
	const FSkillRuleSnapshotSet OriginalRules = Rules;
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
		TEXT("Repeated full result"),
		AreAvailabilityResultsEqual(First, Second));
	TestTrue(
		TEXT("State unchanged"),
		AreStatesEqual(State, OriginalState));
	TestTrue(
		TEXT("Rule set unchanged"),
		AreRuleSetsEqual(Rules, OriginalRules));
	return true;
}

SKILL_AVAILABILITY_TEST(
	FSkillAvailabilityParticipantFirstThroughBallProjectionTest,
	"ParticipantFirstRunnerFiltersOnlyIncompatibleTactic")

bool FSkillAvailabilityParticipantFirstThroughBallProjectionTest::RunTest(
	const FString& Parameters)
{
	using namespace
		FMCodex::Tests::MatchPlayCurrentAttackSkillSelection;
	const FSkillRuleSnapshotSet Rules = MakeRuleSet();

	const auto Midfield =
		FMatchPlayCurrentAttackSkillSelectionAvailability::Query(
			MakeParticipantFirstRunnerState(
				EMatchPlayNeutralSlotSide::NearPlayerA),
			ValidAttackSequence,
			EInitialTurnOrderPlayer::PlayerA,
			Rules);
	const auto* MidfieldThroughBall = Midfield.Candidates.FindByPredicate(
		[](const FMatchPlayCurrentAttackSkillSelectionCandidateAvailability& Candidate)
		{
			return Candidate.SkillId == ThroughBallSkillId;
		});
	const auto* MidfieldPassControl = Midfield.Candidates.FindByPredicate(
		[](const FMatchPlayCurrentAttackSkillSelectionCandidateAvailability& Candidate)
		{
			return Candidate.SkillId == PassControlSkillId;
		});
	TestTrue(TEXT("Midfield candidate query remains valid"),
		Midfield.bQuerySucceeded && Midfield.bCanSelectAnySkill);
	TestTrue(TEXT("ThroughBall remains represented with exact incompatibility"),
		MidfieldThroughBall != nullptr
			&& !MidfieldThroughBall->LegalityResult.bIsLegal
			&& MidfieldThroughBall->LegalityResult.ErrorCode
				== EMatchPlayCurrentAttackSkillSelectionErrorCode::
					PreparedRunnerIncompatibleWithSkill);
	TestTrue(TEXT("Other legal tactic remains selectable for midfield Runner"),
		MidfieldPassControl != nullptr
			&& MidfieldPassControl->LegalityResult.bIsLegal);

	const auto Forward =
		FMatchPlayCurrentAttackSkillSelectionAvailability::Query(
			MakeParticipantFirstRunnerState(
				EMatchPlayNeutralSlotSide::NearPlayerB),
			ValidAttackSequence,
			EInitialTurnOrderPlayer::PlayerA,
			Rules);
	const auto* ForwardThroughBall = Forward.Candidates.FindByPredicate(
		[](const FMatchPlayCurrentAttackSkillSelectionCandidateAvailability& Candidate)
		{
			return Candidate.SkillId == ThroughBallSkillId;
		});
	TestTrue(TEXT("Relative-forward Runner exposes legal ThroughBall"),
		Forward.bQuerySucceeded && ForwardThroughBall != nullptr
			&& ForwardThroughBall->LegalityResult.bIsLegal);
	return true;
}

#undef SKILL_AVAILABILITY_TEST

#endif
