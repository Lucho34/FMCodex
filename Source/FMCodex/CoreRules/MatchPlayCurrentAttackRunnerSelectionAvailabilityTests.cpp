#include "MatchPlayCurrentAttackRunnerSelectionAvailability.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "MatchPlayCurrentAttackRunnerSelectionTestFixtures.h"
#include "Misc/AutomationTest.h"

namespace RunnerAvailabilityFixtures =
	FMCodex::Tests::MatchPlayCurrentAttackRunnerSelection;

#define RUNNER_AVAILABILITY_TEST(TestClass, TestName) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
		TestClass, \
		"FMCodex.CoreRules.MatchPlayCurrentAttackRunnerSelection.Availability." TestName, \
		EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

RUNNER_AVAILABILITY_TEST(
	FRunnerAvailabilityOrderAndRetentionTest,
	"OrderAndRetention")

bool FRunnerAvailabilityOrderAndRetentionTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayState State = RunnerAvailabilityFixtures::MakeState();
	const FMatchPlayState Original = State;
	const auto First =
		FMatchPlayCurrentAttackRunnerSelectionAvailability::Query(
			State,
			RunnerAvailabilityFixtures::ValidAttackSequence,
			EInitialTurnOrderPlayer::PlayerA);
	const auto Second =
		FMatchPlayCurrentAttackRunnerSelectionAvailability::Query(
			State,
			RunnerAvailabilityFixtures::ValidAttackSequence,
			EInitialTurnOrderPlayer::PlayerA);
	TestTrue(TEXT("Query succeeds"), First.bQuerySucceeded);
	TestTrue(TEXT("Mixed candidates aggregate legal"), First.bCanSelectAnyRunner);
	TestEqual(TEXT("Only attacker placements enumerated"),
		First.Candidates.Num(), 6);
	const TArray<FName> ExpectedOrder = {
		RunnerAvailabilityFixtures::CarrierId,
		RunnerAvailabilityFixtures::MidfieldRunnerId,
		RunnerAvailabilityFixtures::AttackRunnerId,
		RunnerAvailabilityFixtures::ForwardRunnerId,
		RunnerAvailabilityFixtures::DefenseRunnerId,
		RunnerAvailabilityFixtures::GoalkeeperId
	};
	for (int32 Index = 0; Index < ExpectedOrder.Num(); ++Index)
	{
		TestEqual(TEXT("Original order retained"),
			First.Candidates[Index].RunnerCardId, ExpectedOrder[Index]);
		TestEqual(TEXT("Repeated order retained"),
			Second.Candidates[Index].RunnerCardId, ExpectedOrder[Index]);
		TestEqual(TEXT("Repeated legality retained"),
			First.Candidates[Index].LegalityResult.ErrorCode,
			Second.Candidates[Index].LegalityResult.ErrorCode);
	}
	TestEqual(TEXT("Carrier retained as illegal"),
		First.Candidates[0].LegalityResult.ErrorCode,
		EMatchPlayCurrentAttackRunnerSelectionErrorCode
			::RunnerMatchesCarrier);
	TestEqual(TEXT("GK retained as illegal"),
		First.Candidates.Last().LegalityResult.ErrorCode,
		EMatchPlayCurrentAttackRunnerSelectionErrorCode
			::RunnerIsGoalkeeper);
	TestTrue(TEXT("Input unchanged"),
		RunnerAvailabilityFixtures::AreStatesEqual(State, Original));
	return true;
}

RUNNER_AVAILABILITY_TEST(
	FRunnerAvailabilityMissingSnapshotAndZeroLegalTest,
	"MissingSnapshotAndZeroLegal")

bool FRunnerAvailabilityMissingSnapshotAndZeroLegalTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayState Missing =
		RunnerAvailabilityFixtures::MakeState();
	Missing.CurrentAttack.DeploymentPlacements.Add(
		RunnerAvailabilityFixtures::MakePlacement(
			EInitialTurnOrderPlayer::PlayerA,
			RunnerAvailabilityFixtures::MissingRunnerId,
			TEXT("Slot.MissingSnapshot")));
	const auto MissingResult =
		FMatchPlayCurrentAttackRunnerSelectionAvailability::Query(
			Missing,
			RunnerAvailabilityFixtures::ValidAttackSequence,
			EInitialTurnOrderPlayer::PlayerA);
	TestTrue(TEXT("Missing snapshot is candidate-local"),
		MissingResult.bQuerySucceeded);
	TestEqual(TEXT("Missing snapshot retained last"),
		MissingResult.Candidates.Last().RunnerCardId,
		RunnerAvailabilityFixtures::MissingRunnerId);
	TestEqual(TEXT("Missing snapshot exact error"),
		MissingResult.Candidates.Last().LegalityResult.ErrorCode,
		EMatchPlayCurrentAttackRunnerSelectionErrorCode
			::RunnerSnapshotQueryFailed);

	FMatchPlayState Zero = RunnerAvailabilityFixtures::MakeState();
	for (FPlayerCardRuleSnapshot& Snapshot :
		Zero.CardSnapshotAuthority.PlayerACardSnapshots.Cards)
	{
		if (!Snapshot.bIsGoalkeeper)
		{
			Snapshot.PositionTypes = {EPlayerPositionType::Defense};
		}
	}
	const auto ZeroResult =
		FMatchPlayCurrentAttackRunnerSelectionAvailability::Query(
			Zero,
			RunnerAvailabilityFixtures::ValidAttackSequence,
			EInitialTurnOrderPlayer::PlayerA);
	TestTrue(TEXT("Zero legal remains successful query"),
		ZeroResult.bQuerySucceeded);
	TestFalse(TEXT("Zero legal aggregate false"),
		ZeroResult.bCanSelectAnyRunner);
	TestEqual(TEXT("Zero legal still enumerates all"),
		ZeroResult.Candidates.Num(), 6);

	FMatchPlayState GlobalFailure =
		RunnerAvailabilityFixtures::MakeState();
	GlobalFailure.CurrentAttack.AttackSequence = 0;
	const auto Failure =
		FMatchPlayCurrentAttackRunnerSelectionAvailability::Query(
			GlobalFailure,
			RunnerAvailabilityFixtures::ValidAttackSequence,
			EInitialTurnOrderPlayer::PlayerA);
	TestFalse(TEXT("Global failure fails query"),
		Failure.bQuerySucceeded);
	TestEqual(TEXT("Global failure has no candidates"),
		Failure.Candidates.Num(), 0);
	return true;
}

RUNNER_AVAILABILITY_TEST(
	FRunnerAvailabilityThroughBallZonesTest,
	"ThroughBallZones")

bool FRunnerAvailabilityThroughBallZonesTest::RunTest(
	const FString& Parameters)
{
	const auto Result =
		FMatchPlayCurrentAttackRunnerSelectionAvailability::Query(
			RunnerAvailabilityFixtures::MakeState(
				ESkillRuleType::ThroughBall),
			RunnerAvailabilityFixtures::ValidAttackSequence,
			EInitialTurnOrderPlayer::PlayerA);
	TestTrue(TEXT("Query succeeds"), Result.bQuerySucceeded);
	TestTrue(TEXT("Forward candidate yields aggregate true"),
		Result.bCanSelectAnyRunner);
	TestEqual(TEXT("Forward placement remains fourth attacker candidate"),
		Result.Candidates[3].RunnerCardId,
		RunnerAvailabilityFixtures::ForwardRunnerId);
	TestTrue(TEXT("Forward candidate legal"),
		Result.Candidates[3].LegalityResult.bIsLegal);
	TestEqual(TEXT("Near-attacker candidate is not forward"),
		Result.Candidates[1].LegalityResult.ErrorCode,
		EMatchPlayCurrentAttackRunnerSelectionErrorCode
			::RunnerNotInAttackingForwardArea);
	return true;
}

#undef RUNNER_AVAILABILITY_TEST

#endif
