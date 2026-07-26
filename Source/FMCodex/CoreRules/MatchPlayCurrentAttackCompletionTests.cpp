#include "MatchPlayCurrentAttackCompletion.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "MatchPlayBeginOrdinaryAttack.h"
#include "MatchPlayMarkerNoSelectionGoalTestFixtures.h"
#include "Misc/AutomationTest.h"

#include <type_traits>

namespace CurrentAttackCompletionTests
{
	using namespace FMCodex::Tests::MatchPlayMarkerNoSelectionGoal;

	bool ExpectFailure(
		FAutomationTestBase& Test,
		const TCHAR* Context,
		const FMatchPlayState& State,
		const FMatchPlayMarkerNoSelectionGoalProjection& Projection,
		const EMatchPlayCurrentAttackCompletionErrorCode ErrorCode)
	{
		const FMatchPlayState Original = State;
		const FMatchPlayCurrentAttackCompletionResult Result =
			FMatchPlayCurrentAttackCompletion::Complete(
				State,
				Projection);
		Test.TestFalse(
			*FString::Printf(TEXT("%s fails"), Context),
			Result.bSuccess);
		Test.TestEqual(
			*FString::Printf(TEXT("%s exact error"), Context),
			Result.ErrorCode,
			ErrorCode);
		Test.TestTrue(
			*FString::Printf(TEXT("%s AfterState atomic"), Context),
			AreStatesEqual(Result.AfterState, State));
		Test.TestTrue(
			*FString::Printf(TEXT("%s input unchanged"), Context),
			AreStatesEqual(State, Original));
		return true;
	}

	FMatchPlayMarkerNoSelectionGoalProjection MakeDeclineProjection(
		const FMatchPlayState& State)
	{
		return MakeProjection(
			State,
			EMatchPlayMarkerNoSelectionGoalSource::DeclineMarker,
			EMatchPlayMarkerNoSelectionGoalReason::MarkerDeclined);
	}

	FMatchPlayMarkerNoSelectionGoalProjection MakeNoLegalProjection(
		const FMatchPlayState& State)
	{
		const EInitialTurnOrderPlayer Defender =
			GetDefender(State.RuntimeState.CurrentAttackingPlayer);
		int32 DefenderPlacementCount = 0;
		for (const FMatchPlayDeploymentPlacement& Placement :
			State.CurrentAttack.DeploymentPlacements)
		{
			if (Placement.PlayerSide == Defender)
			{
				++DefenderPlacementCount;
			}
		}
		return MakeProjection(
			State,
			EMatchPlayMarkerNoSelectionGoalSource
				::ResolveNoLegalMarker,
			DefenderPlacementCount == 0
				? EMatchPlayMarkerNoSelectionGoalReason
					::DefenderHasNoDeployedPlayers
				: EMatchPlayMarkerNoSelectionGoalReason
					::NoLegalMarker);
	}
}

#define CURRENT_ATTACK_COMPLETION_TEST(TestClass, TestName) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
		TestClass, \
		"FMCodex.CoreRules.MatchPlayMarkerNoSelectionGoal.Completion." TestName, \
		EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

#define MARKER_GOAL_PROJECTION_TEST(TestClass, TestName) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
		TestClass, \
		"FMCodex.CoreRules.MatchPlayMarkerNoSelectionGoal.Projection." TestName, \
		EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

CURRENT_ATTACK_COMPLETION_TEST(
	FCurrentAttackCompletionContractTest,
	"SinglePublicSignatureAndDefaults")

bool FCurrentAttackCompletionContractTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayCurrentAttackCompletionResult Result;
	TestFalse(TEXT("Default result fails"), Result.bSuccess);
	TestEqual(TEXT("Default error None"), Result.ErrorCode,
		EMatchPlayCurrentAttackCompletionErrorCode::None);
	TestNotNull(TEXT("Projection reflected"),
		FMatchPlayMarkerNoSelectionGoalProjection::StaticStruct());
	TestNotNull(TEXT("Completion result reflected"),
		FMatchPlayCurrentAttackCompletionResult::StaticStruct());
	using FSignature = FMatchPlayCurrentAttackCompletionResult (*)(
		const FMatchPlayState&,
		const FMatchPlayMarkerNoSelectionGoalProjection&);
	TestTrue(TEXT("Exactly one Complete signature"),
		(std::is_same_v<
			decltype(&FMatchPlayCurrentAttackCompletion::Complete),
			FSignature>));
	return true;
}

CURRENT_ATTACK_COMPLETION_TEST(
	FCurrentAttackCompletionPlayerBScoreTest,
	"PlayerBScoresExactlyOne")

bool FCurrentAttackCompletionPlayerBScoreTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayMarkerNoSelectionGoal;
	const FMatchPlayState State =
		MakeState(EInitialTurnOrderPlayer::PlayerB);
	const FMatchPlayCurrentAttackCompletionResult Result =
		FMatchPlayCurrentAttackCompletion::Complete(
			State,
			CurrentAttackCompletionTests::MakeDeclineProjection(State));
	TestTrue(TEXT("Completion succeeds"), Result.bSuccess);
	TestEqual(TEXT("PlayerA unchanged"),
		Result.AfterState.RuntimeState.PlayerAState.Score, 2);
	TestEqual(TEXT("PlayerB +1"),
		Result.AfterState.RuntimeState.PlayerBState.Score, 2);
	TestEqual(TEXT("Scoring side derived from runtime"),
		Result.ScoringSide,
		EInitialTurnOrderPlayer::PlayerB);
	return true;
}

CURRENT_ATTACK_COMPLETION_TEST(
	FCurrentAttackCompletionDeploymentOrderTest,
	"AllOrdinaryDeploymentsConsumedInOriginalOrder")

bool FCurrentAttackCompletionDeploymentOrderTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayMarkerNoSelectionGoal;
	const FMatchPlayState State = MakeState();
	const FMatchPlayCurrentAttackCompletionResult Result =
		FMatchPlayCurrentAttackCompletion::Complete(
			State,
			CurrentAttackCompletionTests::MakeDeclineProjection(State));
	TestTrue(TEXT("Completion succeeds"), Result.bSuccess);
	TestEqual(TEXT("Three ordinary cards consumed"),
		Result.OrdinaryCardUsageResults.Num(), 3);
	if (Result.OrdinaryCardUsageResults.Num() == 3)
	{
		TestEqual(TEXT("Carrier first"),
			Result.OrdinaryCardUsageResults[0].CardId, PlayerAOne);
		TestEqual(TEXT("Unselected attacker second"),
			Result.OrdinaryCardUsageResults[1].CardId, PlayerATwo);
		TestEqual(TEXT("Defender third"),
			Result.OrdinaryCardUsageResults[2].CardId, PlayerBOne);
	}
	TestTrue(TEXT("PlayerA carrier Used"),
		Result.AfterState.CardUsageState.PlayerACardUsageState
			.UsedCardIds.Contains(PlayerAOne));
	TestTrue(TEXT("PlayerA unselected deployment Used"),
		Result.AfterState.CardUsageState.PlayerACardUsageState
			.UsedCardIds.Contains(PlayerATwo));
	TestTrue(TEXT("PlayerB defender Used"),
		Result.AfterState.CardUsageState.PlayerBCardUsageState
			.UsedCardIds.Contains(PlayerBOne));
	return true;
}

CURRENT_ATTACK_COMPLETION_TEST(
	FCurrentAttackCompletionCardUsageAtomicTest,
	"MidLoopCardFailureDoesNotCommitGoalOrPriorCard")

bool FCurrentAttackCompletionCardUsageAtomicTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayMarkerNoSelectionGoal;
	FMatchPlayState State = MakeState();
	State.CardUsageState.PlayerACardUsageState.AvailableCardIds
		.Remove(PlayerATwo);
	const FMatchPlayMarkerNoSelectionGoalProjection Projection =
		CurrentAttackCompletionTests::MakeDeclineProjection(State);
	const FMatchPlayCurrentAttackCompletionResult Result =
		FMatchPlayCurrentAttackCompletion::Complete(State, Projection);
	TestFalse(TEXT("Completion fails"), Result.bSuccess);
	TestEqual(TEXT("Exact CardUsage error"), Result.ErrorCode,
		EMatchPlayCurrentAttackCompletionErrorCode
			::OrdinaryCardUsageConsumptionFailed);
	TestEqual(TEXT("First working result was produced"),
		Result.OrdinaryCardUsageResults.Num(), 2);
	TestTrue(TEXT("AfterState is original"),
		AreStatesEqual(Result.AfterState, State));
	TestEqual(TEXT("Score not committed"),
		Result.AfterState.RuntimeState.PlayerAState.Score, 2);
	TestFalse(TEXT("First card not committed"),
		Result.AfterState.CardUsageState.PlayerACardUsageState
			.UsedCardIds.Contains(PlayerAOne));
	TestTrue(TEXT("CurrentAttack not cleared"),
		Result.AfterState.bHasCurrentAttack);
	return true;
}

CURRENT_ATTACK_COMPLETION_TEST(
	FCurrentAttackCompletionDeploymentValidationTest,
	"InvalidDuplicateAndMissingSnapshotRejectBeforeMutation")

bool FCurrentAttackCompletionDeploymentValidationTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayMarkerNoSelectionGoal;
	FMatchPlayState State = MakeState();
	FMatchPlayMarkerNoSelectionGoalProjection Projection =
		CurrentAttackCompletionTests::MakeDeclineProjection(State);
	State.CurrentAttack.DeploymentPlacements[1].CardId = NAME_None;
	CurrentAttackCompletionTests::ExpectFailure(
		*this,
		TEXT("Empty placement CardId"),
		State,
		Projection,
		EMatchPlayCurrentAttackCompletionErrorCode
			::InvalidDeploymentPlacement);

	State = MakeState();
	Projection =
		CurrentAttackCompletionTests::MakeDeclineProjection(State);
	const FMatchPlayDeploymentPlacement DuplicatePlacement =
		State.CurrentAttack.DeploymentPlacements[0];
	State.CurrentAttack.DeploymentPlacements.Add(DuplicatePlacement);
	CurrentAttackCompletionTests::ExpectFailure(
		*this,
		TEXT("Duplicate card"),
		State,
		Projection,
		EMatchPlayCurrentAttackCompletionErrorCode
			::DuplicateDeploymentCard);

	State = MakeState();
	Projection =
		CurrentAttackCompletionTests::MakeDeclineProjection(State);
	State.CardSnapshotAuthority.PlayerACardSnapshots.Cards.RemoveAll(
		[](const FPlayerCardRuleSnapshot& Snapshot)
		{
			return Snapshot.CardId == PlayerATwo;
		});
	CurrentAttackCompletionTests::ExpectFailure(
		*this,
		TEXT("Missing snapshot"),
		State,
		Projection,
		EMatchPlayCurrentAttackCompletionErrorCode
			::DeploymentSnapshotQueryFailed);
	return true;
}

CURRENT_ATTACK_COMPLETION_TEST(
	FCurrentAttackCompletionGoalFailureTest,
	"OverflowAndNegativeScoresRejectAtomically")

bool FCurrentAttackCompletionGoalFailureTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayMarkerNoSelectionGoal;
	FMatchPlayState State = MakeState();
	State.RuntimeState.PlayerAState.Score = MAX_int32;
	FMatchPlayMarkerNoSelectionGoalProjection Projection =
		CurrentAttackCompletionTests::MakeDeclineProjection(State);
	CurrentAttackCompletionTests::ExpectFailure(
		*this,
		TEXT("Score overflow"),
		State,
		Projection,
		EMatchPlayCurrentAttackCompletionErrorCode
			::GoalResolutionFailed);

	State = MakeState();
	State.RuntimeState.PlayerBState.Score = -1;
	Projection =
		CurrentAttackCompletionTests::MakeDeclineProjection(State);
	CurrentAttackCompletionTests::ExpectFailure(
		*this,
		TEXT("Negative score"),
		State,
		Projection,
		EMatchPlayCurrentAttackCompletionErrorCode
			::InvalidScoreState);
	return true;
}

CURRENT_ATTACK_COMPLETION_TEST(
	FCurrentAttackCompletionGoalkeeperValidationTest,
	"GoalkeeperCorruptionRejectsAtomically")

bool FCurrentAttackCompletionGoalkeeperValidationTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayMarkerNoSelectionGoal;
	FMatchPlayState State = MakeState();
	MakeOnlyGoalkeeperDefender(State);
	FMatchPlayMarkerNoSelectionGoalProjection Projection =
		CurrentAttackCompletionTests::MakeNoLegalProjection(State);
	State.CurrentAttack.bCurrentDefenseGoalkeeperActivated = false;
	CurrentAttackCompletionTests::ExpectFailure(
		*this,
		TEXT("Activation mismatch"),
		State,
		Projection,
		EMatchPlayCurrentAttackCompletionErrorCode
			::InvalidGoalkeeperCompletionState);

	State = MakeState();
	MakeOnlyGoalkeeperDefender(State);
	Projection =
		CurrentAttackCompletionTests::MakeNoLegalProjection(State);
	State.GoalkeeperUsageState.bPlayerBGoalkeeperCardUsed = false;
	CurrentAttackCompletionTests::ExpectFailure(
		*this,
		TEXT("Persistent usage mismatch"),
		State,
		Projection,
		EMatchPlayCurrentAttackCompletionErrorCode
			::InvalidGoalkeeperCompletionState);

	State = MakeState();
	MakeOnlyGoalkeeperDefender(State);
	for (FPlayerCardRuleSnapshot& Snapshot :
		State.CardSnapshotAuthority.PlayerBCardSnapshots.Cards)
	{
		if (Snapshot.CardId == PlayerBTwo)
		{
			Snapshot.PositionTypes = {
				EPlayerPositionType::Goalkeeper
			};
			Snapshot.bIsGoalkeeper = true;
			Snapshot.bHasGoalkeeperAttributes = true;
			SetGoalkeeperAttributes(
				Snapshot.GoalkeeperAttributes);
		}
	}
	State.CurrentAttack.DeploymentPlacements.Add(
		MakePlacement(
			EInitialTurnOrderPlayer::PlayerB,
			PlayerBTwo,
			NearPlayerBSlotTwo));
	Projection =
		CurrentAttackCompletionTests::MakeNoLegalProjection(State);
	CurrentAttackCompletionTests::ExpectFailure(
		*this,
		TEXT("Multiple goalkeeper placements"),
		State,
		Projection,
		EMatchPlayCurrentAttackCompletionErrorCode
			::InvalidGoalkeeperCompletionState);
	return true;
}

CURRENT_ATTACK_COMPLETION_TEST(
	FCurrentAttackCompletionGoalkeeperSideTest,
	"AttackerGoalkeeperPlacementRejected")

bool FCurrentAttackCompletionGoalkeeperSideTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayMarkerNoSelectionGoal;
	FMatchPlayState State = MakeState();
	RemoveDefenderPlacements(State);
	State.CurrentAttack.DeploymentPlacements.Add(
		MakePlacement(
			EInitialTurnOrderPlayer::PlayerA,
			PlayerAGoalkeeper,
			NearPlayerBSlotGoalkeeper));
	const FMatchPlayMarkerNoSelectionGoalProjection Projection =
		CurrentAttackCompletionTests::MakeNoLegalProjection(State);
	return CurrentAttackCompletionTests::ExpectFailure(
		*this,
		TEXT("Attacker GK"),
		State,
		Projection,
		EMatchPlayCurrentAttackCompletionErrorCode
			::InvalidGoalkeeperCompletionState);
}

CURRENT_ATTACK_COMPLETION_TEST(
	FCurrentAttackCompletionCurrentAttackClearTest,
	"ClearsWholePayloadAndPreservesPersistentAuthorities")

bool FCurrentAttackCompletionCurrentAttackClearTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayMarkerNoSelectionGoal;
	FMatchPlayState State = MakeState();
	MakeOnlyGoalkeeperDefender(State);
	const FMatchPlayDeploymentSlotCatalog CatalogBefore =
		State.DeploymentSlotCatalog;
	const FMatchPlayPerSideCardSnapshotAuthority SnapshotsBefore =
		State.CardSnapshotAuthority;
	const FMatchPlayGoalkeeperUsageState GoalkeeperUsageBefore =
		State.GoalkeeperUsageState;
	const FMatchPlayCurrentAttackCompletionResult Result =
		FMatchPlayCurrentAttackCompletion::Complete(
			State,
			CurrentAttackCompletionTests::MakeNoLegalProjection(State));
	const FMatchPlayCurrentAttackState EmptyAttack;
	TestTrue(TEXT("Completion succeeds"), Result.bSuccess);
	TestFalse(TEXT("CurrentAttack flag cleared"),
		Result.AfterState.bHasCurrentAttack);
	TestTrue(TEXT("Whole payload is canonical default"),
		FMatchPlayCurrentAttackState::StaticStruct()
			->CompareScriptStruct(
				&Result.AfterState.CurrentAttack,
				&EmptyAttack,
				0));
	TestTrue(TEXT("Slot catalog preserved"),
		FMatchPlayDeploymentSlotCatalog::StaticStruct()
			->CompareScriptStruct(
				&Result.AfterState.DeploymentSlotCatalog,
				&CatalogBefore,
				0));
	TestTrue(TEXT("Snapshot authority preserved"),
		FMatchPlayPerSideCardSnapshotAuthority::StaticStruct()
			->CompareScriptStruct(
				&Result.AfterState.CardSnapshotAuthority,
				&SnapshotsBefore,
				0));
	TestTrue(TEXT("Persistent GK usage preserved"),
		FMatchPlayGoalkeeperUsageState::StaticStruct()
			->CompareScriptStruct(
				&Result.AfterState.GoalkeeperUsageState,
				&GoalkeeperUsageBefore,
				0));
	return true;
}

CURRENT_ATTACK_COMPLETION_TEST(
	FCurrentAttackCompletionNextAttackerTest,
	"OpponentOrSameAttackerDerivedWithoutAutoBegin")

bool FCurrentAttackCompletionNextAttackerTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayMarkerNoSelectionGoal;
	FMatchPlayState State = MakeState();
	FMatchPlayCurrentAttackCompletionResult Result =
		FMatchPlayCurrentAttackCompletion::Complete(
			State,
			CurrentAttackCompletionTests::MakeDeclineProjection(State));
	TestTrue(TEXT("Opponent case succeeds"), Result.bSuccess);
	TestEqual(TEXT("Opponent attacks next"),
		Result.NextAttackingPlayer,
		EInitialTurnOrderPlayer::PlayerB);
	TestFalse(TEXT("No attack automatically begun"),
		Result.AfterState.bHasCurrentAttack);

	State = MakeState();
	State.RuntimeState.PlayerBState.UsedAttackCount =
		State.RuntimeState.PlayerBState.TotalAttackCount;
	Result = FMatchPlayCurrentAttackCompletion::Complete(
		State,
		CurrentAttackCompletionTests::MakeDeclineProjection(State));
	TestTrue(TEXT("Same-side case succeeds"), Result.bSuccess);
	TestEqual(TEXT("Current attacker continues"),
		Result.NextAttackingPlayer,
		EInitialTurnOrderPlayer::PlayerA);
	TestFalse(TEXT("Still no auto Begin"),
		Result.AfterState.bHasCurrentAttack);
	return true;
}

CURRENT_ATTACK_COMPLETION_TEST(
	FCurrentAttackCompletionMatchResultsTest,
	"FinalOpportunityDerivesHomeAwayAndDraw")

bool FCurrentAttackCompletionMatchResultsTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayMarkerNoSelectionGoal;
	FMatchPlayState State = MakeState();
	State.RuntimeState.PlayerAState.TotalAttackCount = 2;
	State.RuntimeState.PlayerAState.UsedAttackCount = 1;
	State.RuntimeState.PlayerBState.TotalAttackCount = 1;
	State.RuntimeState.PlayerBState.UsedAttackCount = 1;
	State.RuntimeState.PlayerAState.Score = 1;
	State.RuntimeState.PlayerBState.Score = 1;
	FMatchPlayCurrentAttackCompletionResult Result =
		FMatchPlayCurrentAttackCompletion::Complete(
			State,
			CurrentAttackCompletionTests::MakeDeclineProjection(State));
	TestTrue(TEXT("Home-win completion succeeds"), Result.bSuccess);
	TestTrue(TEXT("Match ended"), Result.bMatchEnded);
	TestEqual(TEXT("Home win derived"),
		Result.MatchResultResolveResult.ResultType,
		EMatchResultType::HomeWin);
	TestEqual(TEXT("Final attacker None"),
		Result.AfterState.RuntimeState.CurrentAttackingPlayer,
		EInitialTurnOrderPlayer::None);

	State = MakeState(EInitialTurnOrderPlayer::PlayerB);
	State.RuntimeState.PlayerAState.TotalAttackCount = 1;
	State.RuntimeState.PlayerAState.UsedAttackCount = 1;
	State.RuntimeState.PlayerBState.TotalAttackCount = 2;
	State.RuntimeState.PlayerBState.UsedAttackCount = 1;
	State.RuntimeState.PlayerAState.Score = 1;
	State.RuntimeState.PlayerBState.Score = 1;
	Result = FMatchPlayCurrentAttackCompletion::Complete(
		State,
		CurrentAttackCompletionTests::MakeDeclineProjection(State));
	TestTrue(TEXT("Away-win completion succeeds"), Result.bSuccess);
	TestEqual(TEXT("Away win derived"),
		Result.MatchResultResolveResult.ResultType,
		EMatchResultType::AwayWin);

	State = MakeState();
	State.RuntimeState.PlayerAState.TotalAttackCount = 2;
	State.RuntimeState.PlayerAState.UsedAttackCount = 1;
	State.RuntimeState.PlayerBState.TotalAttackCount = 1;
	State.RuntimeState.PlayerBState.UsedAttackCount = 1;
	State.RuntimeState.PlayerAState.Score = 0;
	State.RuntimeState.PlayerBState.Score = 1;
	Result = FMatchPlayCurrentAttackCompletion::Complete(
		State,
		CurrentAttackCompletionTests::MakeDeclineProjection(State));
	TestTrue(TEXT("Draw completion succeeds"), Result.bSuccess);
	TestEqual(TEXT("Draw derived"),
		Result.MatchResultResolveResult.ResultType,
		EMatchResultType::Draw);
	return true;
}

CURRENT_ATTACK_COMPLETION_TEST(
	FCurrentAttackCompletionSequenceAndRepeatTest,
	"ExternalBeginAdvancesSequenceAndRepeatIsSafe")

bool FCurrentAttackCompletionSequenceAndRepeatTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayMarkerNoSelectionGoal;
	const FMatchPlayState State = MakeState();
	const FMatchPlayMarkerNoSelectionGoalProjection Projection =
		CurrentAttackCompletionTests::MakeDeclineProjection(State);
	const FMatchPlayCurrentAttackCompletionResult Completed =
		FMatchPlayCurrentAttackCompletion::Complete(State, Projection);
	TestTrue(TEXT("Completion succeeds"), Completed.bSuccess);
	const FMatchPlayCurrentAttackCompletionResult Repeated =
		FMatchPlayCurrentAttackCompletion::Complete(
			Completed.AfterState,
			Projection);
	TestFalse(TEXT("Repeated completion fails"), Repeated.bSuccess);
	TestEqual(TEXT("NoCurrentAttack is repeat guard"),
		Repeated.ErrorCode,
		EMatchPlayCurrentAttackCompletionErrorCode::NoCurrentAttack);
	TestTrue(TEXT("Repeated state unchanged"),
		AreStatesEqual(Repeated.AfterState, Completed.AfterState));

	const FMatchPlayBeginOrdinaryAttackResult NextBegin =
		FMatchPlayBeginOrdinaryAttack::Begin(
			Completed.AfterState,
			6);
	TestTrue(TEXT("External Begin succeeds"), NextBegin.bSuccess);
	TestEqual(TEXT("Sequence increments"),
		NextBegin.AfterState.CurrentAttack.AttackSequence,
		ValidAttackSequence + 1);
	TestEqual(TEXT("External ActionPoint retained"),
		NextBegin.AfterState.CurrentAttack.ActionPoint, 6);
	return true;
}

MARKER_GOAL_PROJECTION_TEST(
	FMarkerGoalProjectionBasicForgeryTest,
	"FormalGoalSourceReasonAndSequenceValidated")

bool FMarkerGoalProjectionBasicForgeryTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayMarkerNoSelectionGoal;
	const FMatchPlayState State = MakeState();
	FMatchPlayMarkerNoSelectionGoalProjection Projection;
	CurrentAttackCompletionTests::ExpectFailure(
		*this,
		TEXT("Default projection"),
		State,
		Projection,
		EMatchPlayCurrentAttackCompletionErrorCode
			::InvalidProjection);

	Projection =
		CurrentAttackCompletionTests::MakeDeclineProjection(State);
	Projection.bFormalSuccess = false;
	CurrentAttackCompletionTests::ExpectFailure(
		*this,
		TEXT("Formal failure"),
		State,
		Projection,
		EMatchPlayCurrentAttackCompletionErrorCode
			::InvalidProjection);

	Projection =
		CurrentAttackCompletionTests::MakeDeclineProjection(State);
	Projection.bIsGoal = false;
	CurrentAttackCompletionTests::ExpectFailure(
		*this,
		TEXT("Not a goal"),
		State,
		Projection,
		EMatchPlayCurrentAttackCompletionErrorCode
			::InvalidProjection);

	Projection =
		CurrentAttackCompletionTests::MakeDeclineProjection(State);
	Projection.Source =
		EMatchPlayMarkerNoSelectionGoalSource::None;
	CurrentAttackCompletionTests::ExpectFailure(
		*this,
		TEXT("Unknown source"),
		State,
		Projection,
		EMatchPlayCurrentAttackCompletionErrorCode
			::UnsupportedProjectionSource);

	Projection =
		CurrentAttackCompletionTests::MakeDeclineProjection(State);
	Projection.Reason =
		EMatchPlayMarkerNoSelectionGoalReason::None;
	CurrentAttackCompletionTests::ExpectFailure(
		*this,
		TEXT("Unknown reason"),
		State,
		Projection,
		EMatchPlayCurrentAttackCompletionErrorCode
			::InvalidProjectionReason);

	Projection =
		CurrentAttackCompletionTests::MakeDeclineProjection(State);
	Projection.AttackSequence++;
	CurrentAttackCompletionTests::ExpectFailure(
		*this,
		TEXT("Sequence mismatch"),
		State,
		Projection,
		EMatchPlayCurrentAttackCompletionErrorCode
			::ProjectionSequenceMismatch);
	return true;
}

MARKER_GOAL_PROJECTION_TEST(
	FMarkerGoalProjectionProvenanceForgeryTest,
	"AvailabilityAndReasonSourceConsistencyValidated")

bool FMarkerGoalProjectionProvenanceForgeryTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayMarkerNoSelectionGoal;
	const FMatchPlayState State = MakeState();
	FMatchPlayMarkerNoSelectionGoalProjection Projection =
		CurrentAttackCompletionTests::MakeDeclineProjection(State);
	Projection.Reason =
		EMatchPlayMarkerNoSelectionGoalReason::NoLegalMarker;
	CurrentAttackCompletionTests::ExpectFailure(
		*this,
		TEXT("Reason source mismatch"),
		State,
		Projection,
		EMatchPlayCurrentAttackCompletionErrorCode
			::InvalidProjectionProvenance);

	Projection =
		CurrentAttackCompletionTests::MakeDeclineProjection(State);
	Projection.MarkerAvailabilityResult.bQuerySucceeded = false;
	CurrentAttackCompletionTests::ExpectFailure(
		*this,
		TEXT("Failed underlying authority"),
		State,
		Projection,
		EMatchPlayCurrentAttackCompletionErrorCode
			::InvalidProjectionProvenance);

	Projection =
		CurrentAttackCompletionTests::MakeDeclineProjection(State);
	Projection.MarkerAvailabilityResult.Candidates.Reset();
	CurrentAttackCompletionTests::ExpectFailure(
		*this,
		TEXT("Forged candidate evidence"),
		State,
		Projection,
		EMatchPlayCurrentAttackCompletionErrorCode
			::InvalidProjectionProvenance);

	Projection =
		CurrentAttackCompletionTests::MakeDeclineProjection(State);
	Projection.MarkerAvailabilityResult.bCanSelectAnyMarker = false;
	CurrentAttackCompletionTests::ExpectFailure(
		*this,
		TEXT("Forged availability summary"),
		State,
		Projection,
		EMatchPlayCurrentAttackCompletionErrorCode
			::InvalidProjectionProvenance);
	return true;
}

#endif
