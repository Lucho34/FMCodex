#include "MatchPlayMarkerNoSelectionGoal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "MatchPlayMarkerNoSelectionGoalTestFixtures.h"
#include "Misc/AutomationTest.h"

#include <type_traits>

namespace MarkerNoSelectionGoalTests
{
	using namespace FMCodex::Tests::MatchPlayMarkerNoSelectionGoal;

	bool ExpectNoLegalFailure(
		FAutomationTestBase& Test,
		const TCHAR* Context,
		const FMatchPlayState& State,
		const FMatchPlayResolveNoLegalMarkerRequest& Request,
		const EMatchPlayResolveNoLegalMarkerErrorCode ErrorCode)
	{
		const FMatchPlayState Original = State;
		const FMatchPlayResolveNoLegalMarkerResult Result =
			FMatchPlayResolveNoLegalMarker::Resolve(State, Request);
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

	bool ExpectDeclineFailure(
		FAutomationTestBase& Test,
		const TCHAR* Context,
		const FMatchPlayState& State,
		const FMatchPlayMarkerDeclineRequest& Request,
		const EMatchPlayMarkerDeclineErrorCode ErrorCode)
	{
		const FMatchPlayState Original = State;
		const FMatchPlayMarkerDeclineResult Result =
			FMatchPlayMarkerDecline::Decline(State, Request);
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
}

#define NO_LEGAL_MARKER_GOAL_TEST(TestClass, TestName) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
		TestClass, \
		"FMCodex.CoreRules.MatchPlayMarkerNoSelectionGoal.NoLegalMarker." TestName, \
		EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

#define MARKER_DECLINE_GOAL_TEST(TestClass, TestName) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
		TestClass, \
		"FMCodex.CoreRules.MatchPlayMarkerNoSelectionGoal.Decline." TestName, \
		EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

NO_LEGAL_MARKER_GOAL_TEST(
	FResolveNoLegalMarkerContractTest,
	"RequestResultAndSignature")

bool FResolveNoLegalMarkerContractTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayResolveNoLegalMarkerRequest Request;
	TestEqual(TEXT("Request defaults sequence"), Request.AttackSequence, int64{ 0 });
	TestNotNull(TEXT("Request reflected"),
		FMatchPlayResolveNoLegalMarkerRequest::StaticStruct());
	TestNotNull(TEXT("Result reflected"),
		FMatchPlayResolveNoLegalMarkerResult::StaticStruct());
	using FSignature = FMatchPlayResolveNoLegalMarkerResult (*)(
		const FMatchPlayState&,
		const FMatchPlayResolveNoLegalMarkerRequest&);
	TestTrue(TEXT("Exactly one Resolve signature"),
		(std::is_same_v<
			decltype(&FMatchPlayResolveNoLegalMarker::Resolve),
			FSignature>));
	return true;
}

NO_LEGAL_MARKER_GOAL_TEST(
	FResolveNoLegalMarkerZeroDefenderTest,
	"ZeroDefenderPlacementsScoresAndCompletes")

bool FResolveNoLegalMarkerZeroDefenderTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayMarkerNoSelectionGoal;
	FMatchPlayState State = MakeState();
	RemoveDefenderPlacements(State);
	const FMatchPlayState Original = State;
	const FMatchPlayResolveNoLegalMarkerResult Result =
		FMatchPlayResolveNoLegalMarker::Resolve(
			State,
			MakeNoLegalRequest());

	TestTrue(TEXT("Resolve succeeds"), Result.bSuccess);
	TestEqual(TEXT("Reason distinguishes zero placements"),
		Result.GoalProjection.Reason,
		EMatchPlayMarkerNoSelectionGoalReason
			::DefenderHasNoDeployedPlayers);
	TestEqual(TEXT("Source is system authority"),
		Result.GoalProjection.Source,
		EMatchPlayMarkerNoSelectionGoalSource
			::ResolveNoLegalMarker);
	TestEqual(TEXT("Attacker score increments exactly once"),
		Result.AfterState.RuntimeState.PlayerAState.Score, 3);
	TestEqual(TEXT("Defender score unchanged"),
		Result.AfterState.RuntimeState.PlayerBState.Score, 1);
	TestTrue(TEXT("Carrier consumed"),
		Result.AfterState.CardUsageState.PlayerACardUsageState
			.UsedCardIds.Contains(PlayerAOne));
	TestTrue(TEXT("Unselected attacker deployment consumed"),
		Result.AfterState.CardUsageState.PlayerACardUsageState
			.UsedCardIds.Contains(PlayerATwo));
	TestFalse(TEXT("Current attack cleared"),
		Result.AfterState.bHasCurrentAttack);
	TestEqual(TEXT("Opportunity consumed once"),
		Result.AfterState.RuntimeState.PlayerAState.UsedAttackCount, 2);
	TestEqual(TEXT("Defender count unchanged"),
		Result.AfterState.RuntimeState.PlayerBState.UsedAttackCount, 1);
	TestEqual(TEXT("Defender attacks next"),
		Result.AfterState.RuntimeState.CurrentAttackingPlayer,
		EInitialTurnOrderPlayer::PlayerB);
	TestTrue(TEXT("Input remains unchanged"),
		AreStatesEqual(State, Original));
	return true;
}

NO_LEGAL_MARKER_GOAL_TEST(
	FResolveNoLegalMarkerOnlyGoalkeeperTest,
	"OnlyGoalkeeperIsNoLegalMarkerAndStaysAvailable")

bool FResolveNoLegalMarkerOnlyGoalkeeperTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayMarkerNoSelectionGoal;
	FMatchPlayState State = MakeState();
	MakeOnlyGoalkeeperDefender(State);
	const FMatchPlayResolveNoLegalMarkerResult Result =
		FMatchPlayResolveNoLegalMarker::Resolve(
			State,
			MakeNoLegalRequest());

	TestTrue(TEXT("Resolve succeeds"), Result.bSuccess);
	TestEqual(TEXT("GK-only reason is NoLegalMarker"),
		Result.GoalProjection.Reason,
		EMatchPlayMarkerNoSelectionGoalReason::NoLegalMarker);
	TestTrue(TEXT("GK remains available"),
		Result.AfterState.CardUsageState.PlayerBCardUsageState
			.AvailableCardIds.Contains(PlayerBGoalkeeper));
	TestFalse(TEXT("GK is not marked Used"),
		Result.AfterState.CardUsageState.PlayerBCardUsageState
			.UsedCardIds.Contains(PlayerBGoalkeeper));
	TestTrue(TEXT("Persistent GK usage remains"),
		Result.AfterState.GoalkeeperUsageState
			.bPlayerBGoalkeeperCardUsed);
	TestFalse(TEXT("Current attack is cleared"),
		Result.AfterState.bHasCurrentAttack);
	TestFalse(TEXT("Cleared payload has no GK activation"),
		Result.AfterState.CurrentAttack
			.bCurrentDefenseGoalkeeperActivated);
	return true;
}

NO_LEGAL_MARKER_GOAL_TEST(
	FResolveNoLegalMarkerOtherAreaTest,
	"OrdinaryDefenderInOtherAreaScoresAndIsConsumed")

bool FResolveNoLegalMarkerOtherAreaTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayMarkerNoSelectionGoal;
	FMatchPlayState State = MakeState();
	MakeOnlyOtherAreaDefender(State);
	const FMatchPlayResolveNoLegalMarkerResult Result =
		FMatchPlayResolveNoLegalMarker::Resolve(
			State,
			MakeNoLegalRequest());

	TestTrue(TEXT("Resolve succeeds"), Result.bSuccess);
	TestEqual(TEXT("Reason is NoLegalMarker"),
		Result.GoalProjection.Reason,
		EMatchPlayMarkerNoSelectionGoalReason::NoLegalMarker);
	TestTrue(TEXT("Illegal defender deployment still consumed"),
		Result.AfterState.CardUsageState.PlayerBCardUsageState
			.UsedCardIds.Contains(PlayerBOne));
	TestEqual(TEXT("All three ordinary placements consumed"),
		Result.CompletionResult.OrdinaryCardUsageResults.Num(), 3);
	return true;
}

NO_LEGAL_MARKER_GOAL_TEST(
	FResolveNoLegalMarkerLegalCandidateTest,
	"LegalCandidateRejectsAtomically")

bool FResolveNoLegalMarkerLegalCandidateTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayMarkerNoSelectionGoal;
	return MarkerNoSelectionGoalTests::ExpectNoLegalFailure(
		*this,
		TEXT("Legal candidate"),
		MakeState(),
		MakeNoLegalRequest(),
		EMatchPlayResolveNoLegalMarkerErrorCode::LegalMarkerExists);
}

NO_LEGAL_MARKER_GOAL_TEST(
	FResolveNoLegalMarkerGlobalBlockerTest,
	"AvailabilityGlobalBlockerRejectsAtomically")

bool FResolveNoLegalMarkerGlobalBlockerTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayMarkerNoSelectionGoal;
	FMatchPlayState State = MakeState();
	State.DeploymentSlotCatalog.Slots.RemoveAll(
		[](const FMatchPlayDeploymentSlotDefinition& Slot)
		{
			return Slot.SlotId == NearPlayerBSlotOne;
		});
	const FMatchPlayResolveNoLegalMarkerResult Result =
		FMatchPlayResolveNoLegalMarker::Resolve(
			State,
			MakeNoLegalRequest());
	TestFalse(TEXT("Global blocker rejects"), Result.bSuccess);
	TestEqual(TEXT("Mapped to availability failure"),
		Result.ErrorCode,
		EMatchPlayResolveNoLegalMarkerErrorCode
			::MarkerAvailabilityFailed);
	TestTrue(TEXT("Underlying blocker retained"),
		Result.MarkerAvailabilityResult
			.bHasGlobalBlockingLegalityResult);
	TestTrue(TEXT("Atomic failure"),
		AreStatesEqual(Result.AfterState, State));
	return true;
}

NO_LEGAL_MARKER_GOAL_TEST(
	FResolveNoLegalMarkerFirstErrorsTest,
	"StableLifecycleFirstErrors")

bool FResolveNoLegalMarkerFirstErrorsTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayMarkerNoSelectionGoal;
	FMatchPlayState State = MakeState();
	FMatchPlayResolveNoLegalMarkerRequest Request =
		MakeNoLegalRequest();
	State.RuntimeState.bIsInitialized = false;
	MarkerNoSelectionGoalTests::ExpectNoLegalFailure(
		*this,
		TEXT("Uninitialized"),
		State,
		Request,
		EMatchPlayResolveNoLegalMarkerErrorCode
			::MatchPlayStateNotInitialized);

	State = MakeState();
	State.bHasCurrentAttack = false;
	MarkerNoSelectionGoalTests::ExpectNoLegalFailure(
		*this,
		TEXT("No current attack"),
		State,
		Request,
		EMatchPlayResolveNoLegalMarkerErrorCode::NoCurrentAttack);

	State = MakeState();
	Request.AttackSequence = ValidAttackSequence + 1;
	MarkerNoSelectionGoalTests::ExpectNoLegalFailure(
		*this,
		TEXT("Stale sequence"),
		State,
		Request,
		EMatchPlayResolveNoLegalMarkerErrorCode
			::AttackSequenceMismatch);

	State = MakeState();
	Request = MakeNoLegalRequest();
	State.CurrentAttack.Phase =
		EMatchPlayCurrentAttackPhase::Deployment;
	MarkerNoSelectionGoalTests::ExpectNoLegalFailure(
		*this,
		TEXT("Wrong phase"),
		State,
		Request,
		EMatchPlayResolveNoLegalMarkerErrorCode
			::CurrentAttackNotInResolution);

	State = MakeState();
	State.CurrentAttack.SelectionStage =
		EMatchPlayCurrentAttackSelectionStage::AwaitingSkill;
	State.CurrentAttack.ActionPreparation.MarkerCardId = PlayerBOne;
	MarkerNoSelectionGoalTests::ExpectNoLegalFailure(
		*this,
		TEXT("Wrong stage"),
		State,
		Request,
		EMatchPlayResolveNoLegalMarkerErrorCode::WrongSelectionStage);

	State = MakeState();
	State.CurrentAttack.ActionPreparation.CarrierCardId = NAME_None;
	MarkerNoSelectionGoalTests::ExpectNoLegalFailure(
		*this,
		TEXT("Corrupt canonical state"),
		State,
		Request,
		EMatchPlayResolveNoLegalMarkerErrorCode
			::InvalidSelectionState);
	return true;
}

NO_LEGAL_MARKER_GOAL_TEST(
	FResolveNoLegalMarkerRepeatedTest,
	"RepeatedSuccessRequestHasNoSideEffects")

bool FResolveNoLegalMarkerRepeatedTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayMarkerNoSelectionGoal;
	FMatchPlayState State = MakeState();
	RemoveDefenderPlacements(State);
	const FMatchPlayResolveNoLegalMarkerResult First =
		FMatchPlayResolveNoLegalMarker::Resolve(
			State,
			MakeNoLegalRequest());
	TestTrue(TEXT("First succeeds"), First.bSuccess);
	const FMatchPlayResolveNoLegalMarkerResult Repeated =
		FMatchPlayResolveNoLegalMarker::Resolve(
			First.AfterState,
			MakeNoLegalRequest());
	TestFalse(TEXT("Repeated fails"), Repeated.bSuccess);
	TestEqual(TEXT("Repeated returns NoCurrentAttack"),
		Repeated.ErrorCode,
		EMatchPlayResolveNoLegalMarkerErrorCode::NoCurrentAttack);
	TestTrue(TEXT("Repeated leaves completed state unchanged"),
		AreStatesEqual(Repeated.AfterState, First.AfterState));
	return true;
}

MARKER_DECLINE_GOAL_TEST(
	FMarkerDeclineContractTest,
	"RequestResultAndSignature")

bool FMarkerDeclineContractTest::RunTest(const FString& Parameters)
{
	const FMatchPlayMarkerDeclineRequest Request;
	TestEqual(TEXT("Sequence defaults zero"),
		Request.AttackSequence, int64{ 0 });
	TestEqual(TEXT("Side defaults None"),
		Request.RequestingSide,
		EInitialTurnOrderPlayer::None);
	TestNotNull(TEXT("Request reflected"),
		FMatchPlayMarkerDeclineRequest::StaticStruct());
	TestNotNull(TEXT("Result reflected"),
		FMatchPlayMarkerDeclineResult::StaticStruct());
	using FSignature = FMatchPlayMarkerDeclineResult (*)(
		const FMatchPlayState&,
		const FMatchPlayMarkerDeclineRequest&);
	TestTrue(TEXT("Exactly one Decline signature"),
		(std::is_same_v<
			decltype(&FMatchPlayMarkerDecline::Decline),
			FSignature>));
	return true;
}

MARKER_DECLINE_GOAL_TEST(
	FMarkerDeclineSuccessTest,
	"CurrentDefenderWithLegalMarkerScoresAndCompletes")

bool FMarkerDeclineSuccessTest::RunTest(const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayMarkerNoSelectionGoal;
	const FMatchPlayState State = MakeState();
	const FMatchPlayMarkerDeclineResult Result =
		FMatchPlayMarkerDecline::Decline(
			State,
			MakeDeclineRequest());
	TestTrue(TEXT("Decline succeeds"), Result.bSuccess);
	TestEqual(TEXT("Reason is MarkerDeclined"),
		Result.GoalProjection.Reason,
		EMatchPlayMarkerNoSelectionGoalReason::MarkerDeclined);
	TestEqual(TEXT("Source is DeclineMarker"),
		Result.GoalProjection.Source,
		EMatchPlayMarkerNoSelectionGoalSource::DeclineMarker);
	TestTrue(TEXT("Availability proves a legal marker"),
		Result.MarkerAvailabilityResult.bCanSelectAnyMarker);
	TestEqual(TEXT("Attacker scores once"),
		Result.AfterState.RuntimeState.PlayerAState.Score, 3);
	TestTrue(TEXT("Defender ordinary placement consumed"),
		Result.AfterState.CardUsageState.PlayerBCardUsageState
			.UsedCardIds.Contains(PlayerBOne));
	TestFalse(TEXT("CurrentAttack cleared"),
		Result.AfterState.bHasCurrentAttack);
	return true;
}

MARKER_DECLINE_GOAL_TEST(
	FMarkerDeclineWrongSideTests,
	"InvalidAndAttackingSidesRejectAtomically")

bool FMarkerDeclineWrongSideTests::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayMarkerNoSelectionGoal;
	MarkerNoSelectionGoalTests::ExpectDeclineFailure(
		*this,
		TEXT("Attacker"),
		MakeState(),
		MakeDeclineRequest(EInitialTurnOrderPlayer::PlayerA),
		EMatchPlayMarkerDeclineErrorCode
			::RequestingSideIsNotCurrentDefender);
	MarkerNoSelectionGoalTests::ExpectDeclineFailure(
		*this,
		TEXT("Invalid side"),
		MakeState(),
		MakeDeclineRequest(EInitialTurnOrderPlayer::None),
		EMatchPlayMarkerDeclineErrorCode::InvalidRequestingSide);
	return true;
}

MARKER_DECLINE_GOAL_TEST(
	FMarkerDeclineNoLegalMarkerTest,
	"NoLegalMarkerRequiresSystemEntry")

bool FMarkerDeclineNoLegalMarkerTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayMarkerNoSelectionGoal;
	FMatchPlayState State = MakeState();
	MakeOnlyOtherAreaDefender(State);
	return MarkerNoSelectionGoalTests::ExpectDeclineFailure(
		*this,
		TEXT("No legal marker"),
		State,
		MakeDeclineRequest(),
		EMatchPlayMarkerDeclineErrorCode
			::NoLegalMarkerToDecline);
}

MARKER_DECLINE_GOAL_TEST(
	FMarkerDeclineLifecycleFailuresTest,
	"StalePhaseStageAndCanonicalFailures")

bool FMarkerDeclineLifecycleFailuresTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayMarkerNoSelectionGoal;
	FMatchPlayState State = MakeState();
	FMatchPlayMarkerDeclineRequest Request =
		MakeDeclineRequest();
	Request.AttackSequence++;
	MarkerNoSelectionGoalTests::ExpectDeclineFailure(
		*this,
		TEXT("Stale"),
		State,
		Request,
		EMatchPlayMarkerDeclineErrorCode::AttackSequenceMismatch);

	State.CurrentAttack.Phase =
		EMatchPlayCurrentAttackPhase::Deployment;
	Request = MakeDeclineRequest();
	MarkerNoSelectionGoalTests::ExpectDeclineFailure(
		*this,
		TEXT("Wrong phase"),
		State,
		Request,
		EMatchPlayMarkerDeclineErrorCode
			::CurrentAttackNotInResolution);

	State = MakeState();
	State.CurrentAttack.SelectionStage =
		EMatchPlayCurrentAttackSelectionStage::AwaitingCarrier;
	State.CurrentAttack.ActionPreparation.CarrierCardId = NAME_None;
	MarkerNoSelectionGoalTests::ExpectDeclineFailure(
		*this,
		TEXT("Wrong stage"),
		State,
		Request,
		EMatchPlayMarkerDeclineErrorCode::WrongSelectionStage);

	State = MakeState();
	State.CurrentAttack.SelectedAction.SkillId = TEXT("Corrupt");
	MarkerNoSelectionGoalTests::ExpectDeclineFailure(
		*this,
		TEXT("Corrupt"),
		State,
		Request,
		EMatchPlayMarkerDeclineErrorCode::InvalidSelectionState);
	return true;
}

MARKER_DECLINE_GOAL_TEST(
	FMarkerDeclineRepeatedTest,
	"RepeatedSuccessRequestHasNoSideEffects")

bool FMarkerDeclineRepeatedTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayMarkerNoSelectionGoal;
	const FMatchPlayMarkerDeclineResult First =
		FMatchPlayMarkerDecline::Decline(
			MakeState(),
			MakeDeclineRequest());
	TestTrue(TEXT("First succeeds"), First.bSuccess);
	const FMatchPlayMarkerDeclineResult Repeated =
		FMatchPlayMarkerDecline::Decline(
			First.AfterState,
			MakeDeclineRequest());
	TestFalse(TEXT("Repeated fails"), Repeated.bSuccess);
	TestEqual(TEXT("Repeated returns NoCurrentAttack"),
		Repeated.ErrorCode,
		EMatchPlayMarkerDeclineErrorCode::NoCurrentAttack);
	TestTrue(TEXT("Repeated atomic"),
		AreStatesEqual(Repeated.AfterState, First.AfterState));
	return true;
}

#endif
