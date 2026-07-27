#include "MatchPlayMarkerNoSelectionGoal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "MatchPlayBeginOrdinaryAttack.h"
#include "MatchPlayMarkerNoSelectionGoalTestFixtures.h"
#include "Misc/AutomationTest.h"

namespace CurrentAttackCompletionTests
{
	using namespace FMCodex::Tests::MatchPlayMarkerNoSelectionGoal;

	bool ExpectDeclineCompletionFailure(
		FAutomationTestBase& Test,
		const TCHAR* Context,
		const FMatchPlayState& State,
		const EMatchPlayCurrentAttackCompletionErrorCode ErrorCode)
	{
		const FMatchPlayState Original = State;
		const EInitialTurnOrderPlayer Defender =
			GetDefender(State.RuntimeState.CurrentAttackingPlayer);
		const FMatchPlayMarkerDeclineResult Result =
			FMatchPlayMarkerDecline::Decline(
				State,
				MakeDeclineRequest(Defender));
		Test.TestFalse(
			*FString::Printf(TEXT("%s fails"), Context),
			Result.bSuccess);
		Test.TestEqual(
			*FString::Printf(TEXT("%s maps to CompletionFailed"), Context),
			Result.ErrorCode,
			EMatchPlayMarkerDeclineErrorCode::CompletionFailed);
		Test.TestEqual(
			*FString::Printf(TEXT("%s exact completion error"), Context),
			Result.CompletionResult.ErrorCode,
			ErrorCode);
		Test.TestTrue(
			*FString::Printf(TEXT("%s AfterState atomic"), Context),
			AreStatesEqual(Result.AfterState, State));
		Test.TestTrue(
			*FString::Printf(TEXT("%s completion state atomic"), Context),
			AreStatesEqual(Result.CompletionResult.AfterState, State));
		Test.TestTrue(
			*FString::Printf(TEXT("%s input unchanged"), Context),
			AreStatesEqual(State, Original));
		return true;
	}
}

#define CURRENT_ATTACK_COMPLETION_TEST(TestClass, TestName) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
		TestClass, \
		"FMCodex.CoreRules.MatchPlayMarkerNoSelectionGoal.Completion." TestName, \
		EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

CURRENT_ATTACK_COMPLETION_TEST(
	FCurrentAttackCompletionDiagnosticContractTest,
	"PublicResultIsDiagnosticOnly")

bool FCurrentAttackCompletionDiagnosticContractTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayCurrentAttackCompletionResult Result;
	TestFalse(TEXT("Default result fails"), Result.bSuccess);
	TestEqual(TEXT("Default error is None"), Result.ErrorCode,
		EMatchPlayCurrentAttackCompletionErrorCode::None);
	TestEqual(TEXT("Default reason is None"), Result.Reason,
		EMatchPlayMarkerNoSelectionGoalReason::None);
	TestEqual(TEXT("Default source is None"), Result.Source,
		EMatchPlayMarkerNoSelectionGoalSource::None);
	TestNotNull(TEXT("Completion diagnostic result remains reflected"),
		FMatchPlayCurrentAttackCompletionResult::StaticStruct());
	TestNull(TEXT("Completion result exposes no capability"),
		FMatchPlayCurrentAttackCompletionResult::StaticStruct()
			->FindPropertyByName(TEXT("Capability")));
	TestNull(TEXT("Completion result exposes no projection"),
		FMatchPlayCurrentAttackCompletionResult::StaticStruct()
			->FindPropertyByName(TEXT("Projection")));
	return true;
}

CURRENT_ATTACK_COMPLETION_TEST(
	FCurrentAttackCompletionPlayerBScoreTest,
	"PlayerBScoresExactlyOneThroughPublicEntry")

bool FCurrentAttackCompletionPlayerBScoreTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayMarkerNoSelectionGoal;
	const FMatchPlayState State =
		MakeState(EInitialTurnOrderPlayer::PlayerB);
	const FMatchPlayMarkerDeclineResult Result =
		FMatchPlayMarkerDecline::Decline(
			State,
			MakeDeclineRequest(EInitialTurnOrderPlayer::PlayerA));
	TestTrue(TEXT("Decline succeeds"), Result.bSuccess);
	TestTrue(TEXT("Completion succeeds"),
		Result.CompletionResult.bSuccess);
	TestEqual(TEXT("PlayerA unchanged"),
		Result.AfterState.RuntimeState.PlayerAState.Score, 2);
	TestEqual(TEXT("PlayerB +1"),
		Result.AfterState.RuntimeState.PlayerBState.Score, 2);
	TestEqual(TEXT("Scoring side derived from runtime"),
		Result.CompletionResult.ScoringSide,
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
	const FMatchPlayMarkerDeclineResult Result =
		FMatchPlayMarkerDecline::Decline(
			State,
			MakeDeclineRequest());
	TestTrue(TEXT("Decline succeeds"), Result.bSuccess);
	const TArray<FPlayCardResolveResult>& UsageResults =
		Result.CompletionResult.OrdinaryCardUsageResults;
	TestEqual(TEXT("Three ordinary cards consumed"),
		UsageResults.Num(), 3);
	if (UsageResults.Num() == 3)
	{
		TestEqual(TEXT("Carrier first"), UsageResults[0].CardId,
			PlayerAOne);
		TestEqual(TEXT("Unselected attacker second"),
			UsageResults[1].CardId, PlayerATwo);
		TestEqual(TEXT("Defender third"), UsageResults[2].CardId,
			PlayerBOne);
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
	"MidLoopCardFailureDoesNotCommit")

bool FCurrentAttackCompletionCardUsageAtomicTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayMarkerNoSelectionGoal;
	FMatchPlayState State = MakeState();
	State.CardUsageState.PlayerACardUsageState.AvailableCardIds
		.Remove(PlayerATwo);
	return CurrentAttackCompletionTests::ExpectDeclineCompletionFailure(
		*this,
		TEXT("Mid-loop CardUsage failure"),
		State,
		EMatchPlayCurrentAttackCompletionErrorCode
			::OrdinaryCardUsageConsumptionFailed);
}

CURRENT_ATTACK_COMPLETION_TEST(
	FCurrentAttackCompletionDuplicateSlotTest,
	"DuplicateSlotRejectedGloballyBeforeMutation")

bool FCurrentAttackCompletionDuplicateSlotTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayMarkerNoSelectionGoal;

	FMatchPlayState State = MakeState();
	State.CurrentAttack.DeploymentPlacements[1].SlotId =
		State.CurrentAttack.DeploymentPlacements[0].SlotId;
	CurrentAttackCompletionTests::ExpectDeclineCompletionFailure(
		*this,
		TEXT("Same-side duplicate SlotId"),
		State,
		EMatchPlayCurrentAttackCompletionErrorCode
			::DuplicateDeploymentSlot);

	State = MakeState();
	State.CurrentAttack.DeploymentPlacements[2].SlotId =
		State.CurrentAttack.DeploymentPlacements[0].SlotId;
	CurrentAttackCompletionTests::ExpectDeclineCompletionFailure(
		*this,
		TEXT("Cross-side duplicate SlotId"),
		State,
		EMatchPlayCurrentAttackCompletionErrorCode
			::DuplicateDeploymentSlot);

	State = MakeState();
	State.CurrentAttack.DeploymentPlacements.Add(
		MakePlacement(
			EInitialTurnOrderPlayer::PlayerB,
			PlayerBGoalkeeper,
			State.CurrentAttack.DeploymentPlacements[1].SlotId));
	State.CurrentAttack.bCurrentDefenseGoalkeeperActivated = true;
	State.GoalkeeperUsageState.bPlayerBGoalkeeperCardUsed = true;
	CurrentAttackCompletionTests::ExpectDeclineCompletionFailure(
		*this,
		TEXT("Ordinary and goalkeeper duplicate SlotId"),
		State,
		EMatchPlayCurrentAttackCompletionErrorCode
			::DuplicateDeploymentSlot);
	return true;
}

CURRENT_ATTACK_COMPLETION_TEST(
	FCurrentAttackCompletionScoreFailuresTest,
	"OverflowAndNegativeScoresRejectAtomically")

bool FCurrentAttackCompletionScoreFailuresTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayMarkerNoSelectionGoal;
	FMatchPlayState State = MakeState();
	State.RuntimeState.PlayerAState.Score = MAX_int32;
	CurrentAttackCompletionTests::ExpectDeclineCompletionFailure(
		*this,
		TEXT("Score overflow"),
		State,
		EMatchPlayCurrentAttackCompletionErrorCode::GoalResolutionFailed);

	State = MakeState();
	State.RuntimeState.PlayerBState.Score = -1;
	CurrentAttackCompletionTests::ExpectDeclineCompletionFailure(
		*this,
		TEXT("Negative score"),
		State,
		EMatchPlayCurrentAttackCompletionErrorCode::InvalidScoreState);
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
	State.CurrentAttack.bCurrentDefenseGoalkeeperActivated = false;
	const FMatchPlayResolveNoLegalMarkerResult Result =
		FMatchPlayResolveNoLegalMarker::Resolve(
			State,
			MakeNoLegalRequest());
	TestFalse(TEXT("Corrupt GK state rejects"), Result.bSuccess);
	TestEqual(TEXT("Mapped to CompletionFailed"), Result.ErrorCode,
		EMatchPlayResolveNoLegalMarkerErrorCode::CompletionFailed);
	TestEqual(TEXT("Exact GK completion error"),
		Result.CompletionResult.ErrorCode,
		EMatchPlayCurrentAttackCompletionErrorCode
			::InvalidGoalkeeperCompletionState);
	TestTrue(TEXT("GK rejection is atomic"),
		AreStatesEqual(Result.AfterState, State));
	return true;
}

CURRENT_ATTACK_COMPLETION_TEST(
	FCurrentAttackCompletionCurrentAttackClearTest,
	"ClearsPayloadAndPreservesPersistentAuthorities")

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
	const FMatchPlayResolveNoLegalMarkerResult Result =
		FMatchPlayResolveNoLegalMarker::Resolve(
			State,
			MakeNoLegalRequest());
	const FMatchPlayCurrentAttackState EmptyAttack;
	TestTrue(TEXT("Resolve succeeds"), Result.bSuccess);
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
	"OpportunityOutcomesWithoutAutoBegin")

bool FCurrentAttackCompletionNextAttackerTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayMarkerNoSelectionGoal;
	FMatchPlayState State = MakeState();
	FMatchPlayMarkerDeclineResult Result =
		FMatchPlayMarkerDecline::Decline(
			State,
			MakeDeclineRequest());
	TestTrue(TEXT("Opponent case succeeds"), Result.bSuccess);
	TestEqual(TEXT("Opponent attacks next"),
		Result.CompletionResult.NextAttackingPlayer,
		EInitialTurnOrderPlayer::PlayerB);
	TestFalse(TEXT("No attack automatically begun"),
		Result.AfterState.bHasCurrentAttack);

	State = MakeState();
	State.RuntimeState.PlayerBState.UsedAttackCount =
		State.RuntimeState.PlayerBState.TotalAttackCount;
	Result = FMatchPlayMarkerDecline::Decline(
		State,
		MakeDeclineRequest());
	TestTrue(TEXT("Same-side case succeeds"), Result.bSuccess);
	TestEqual(TEXT("Current attacker continues"),
		Result.CompletionResult.NextAttackingPlayer,
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
	FMatchPlayMarkerDeclineResult Result =
		FMatchPlayMarkerDecline::Decline(
			State,
			MakeDeclineRequest());
	TestTrue(TEXT("Home-win completion succeeds"), Result.bSuccess);
	TestTrue(TEXT("Match ended"),
		Result.CompletionResult.bMatchEnded);
	TestEqual(TEXT("Home win derived"),
		Result.CompletionResult.MatchResultResolveResult.ResultType,
		EMatchResultType::HomeWin);

	State = MakeState(EInitialTurnOrderPlayer::PlayerB);
	State.RuntimeState.PlayerAState.TotalAttackCount = 1;
	State.RuntimeState.PlayerAState.UsedAttackCount = 1;
	State.RuntimeState.PlayerBState.TotalAttackCount = 2;
	State.RuntimeState.PlayerBState.UsedAttackCount = 1;
	State.RuntimeState.PlayerAState.Score = 1;
	State.RuntimeState.PlayerBState.Score = 1;
	Result = FMatchPlayMarkerDecline::Decline(
		State,
		MakeDeclineRequest(EInitialTurnOrderPlayer::PlayerA));
	TestTrue(TEXT("Away-win completion succeeds"), Result.bSuccess);
	TestEqual(TEXT("Away win derived"),
		Result.CompletionResult.MatchResultResolveResult.ResultType,
		EMatchResultType::AwayWin);

	State = MakeState();
	State.RuntimeState.PlayerAState.TotalAttackCount = 2;
	State.RuntimeState.PlayerAState.UsedAttackCount = 1;
	State.RuntimeState.PlayerBState.TotalAttackCount = 1;
	State.RuntimeState.PlayerBState.UsedAttackCount = 1;
	State.RuntimeState.PlayerAState.Score = 0;
	State.RuntimeState.PlayerBState.Score = 1;
	Result = FMatchPlayMarkerDecline::Decline(
		State,
		MakeDeclineRequest());
	TestTrue(TEXT("Draw completion succeeds"), Result.bSuccess);
	TestEqual(TEXT("Draw derived"),
		Result.CompletionResult.MatchResultResolveResult.ResultType,
		EMatchResultType::Draw);
	return true;
}

CURRENT_ATTACK_COMPLETION_TEST(
	FCurrentAttackCompletionOpportunityFailureTest,
	"ExhaustedAndCorruptOpportunityRejectAtomically")

bool FCurrentAttackCompletionOpportunityFailureTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayMarkerNoSelectionGoal;
	FMatchPlayState State = MakeState();
	State.RuntimeState.PlayerAState.UsedAttackCount =
		State.RuntimeState.PlayerAState.TotalAttackCount;
	CurrentAttackCompletionTests::ExpectDeclineCompletionFailure(
		*this,
		TEXT("Exhausted attacker opportunity"),
		State,
		EMatchPlayCurrentAttackCompletionErrorCode
			::InvalidOpportunityState);

	State = MakeState();
	State.RuntimeState.PlayerBState.TotalAttackCount = -1;
	CurrentAttackCompletionTests::ExpectDeclineCompletionFailure(
		*this,
		TEXT("Corrupt opponent opportunity"),
		State,
		EMatchPlayCurrentAttackCompletionErrorCode
			::InvalidOpportunityState);
	return true;
}

CURRENT_ATTACK_COMPLETION_TEST(
	FCurrentAttackCompletionOrdinaryPlusGoalkeeperTest,
	"OrdinaryDeploymentsConsumedAndGoalkeeperPreserved")

bool FCurrentAttackCompletionOrdinaryPlusGoalkeeperTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayMarkerNoSelectionGoal;
	FMatchPlayState State = MakeState();
	State.CurrentAttack.DeploymentPlacements.Add(
		MakePlacement(
			EInitialTurnOrderPlayer::PlayerB,
			PlayerBGoalkeeper,
			GetGoalkeeperSlot(EInitialTurnOrderPlayer::PlayerA)));
	State.CurrentAttack.bCurrentDefenseGoalkeeperActivated = true;
	State.GoalkeeperUsageState.bPlayerBGoalkeeperCardUsed = true;

	const FMatchPlayMarkerDeclineResult Result =
		FMatchPlayMarkerDecline::Decline(
			State,
			MakeDeclineRequest());
	TestTrue(TEXT("Ordinary plus GK completion succeeds"),
		Result.bSuccess);
	TestEqual(TEXT("Only three ordinary cards consumed"),
		Result.CompletionResult.OrdinaryCardUsageResults.Num(), 3);
	TestTrue(TEXT("All attacker ordinary cards Used"),
		Result.AfterState.CardUsageState.PlayerACardUsageState
			.UsedCardIds.Contains(PlayerAOne)
		&& Result.AfterState.CardUsageState.PlayerACardUsageState
			.UsedCardIds.Contains(PlayerATwo));
	TestTrue(TEXT("Defender ordinary card Used"),
		Result.AfterState.CardUsageState.PlayerBCardUsageState
			.UsedCardIds.Contains(PlayerBOne));
	TestTrue(TEXT("GK remains Available"),
		Result.AfterState.CardUsageState.PlayerBCardUsageState
			.AvailableCardIds.Contains(PlayerBGoalkeeper));
	TestFalse(TEXT("GK is not Used"),
		Result.AfterState.CardUsageState.PlayerBCardUsageState
			.UsedCardIds.Contains(PlayerBGoalkeeper));
	TestTrue(TEXT("Persistent GK usage remains"),
		Result.AfterState.GoalkeeperUsageState
			.bPlayerBGoalkeeperCardUsed);
	TestFalse(TEXT("CurrentAttack and activation are cleared"),
		Result.AfterState.bHasCurrentAttack);
	TestEqual(TEXT("Score increments once"),
		Result.AfterState.RuntimeState.PlayerAState.Score, 3);
	TestEqual(TEXT("Opportunity consumed once"),
		Result.AfterState.RuntimeState.PlayerAState.UsedAttackCount,
		State.RuntimeState.PlayerAState.UsedAttackCount + 1);
	TestEqual(TEXT("Next attacker is opponent"),
		Result.CompletionResult.NextAttackingPlayer,
		EInitialTurnOrderPlayer::PlayerB);
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
	const FMatchPlayMarkerDeclineResult Completed =
		FMatchPlayMarkerDecline::Decline(
			State,
			MakeDeclineRequest());
	TestTrue(TEXT("Completion succeeds"), Completed.bSuccess);
	const FMatchPlayMarkerDeclineResult Repeated =
		FMatchPlayMarkerDecline::Decline(
			Completed.AfterState,
			MakeDeclineRequest());
	TestFalse(TEXT("Repeated entry fails"), Repeated.bSuccess);
	TestEqual(TEXT("NoCurrentAttack is repeat guard"),
		Repeated.ErrorCode,
		EMatchPlayMarkerDeclineErrorCode::NoCurrentAttack);
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

#endif
