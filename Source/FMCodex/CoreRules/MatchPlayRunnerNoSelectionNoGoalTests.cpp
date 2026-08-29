#include "MatchPlayRunnerNoSelectionNoGoal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "MatchPlayRunnerNoSelectionNoGoalTestFixtures.h"
#include "MatchPlayCurrentAttackSkillSelectionWriter.h"
#include "MatchPlaySkillNoSelectionNoGoalTestFixtures.h"
#include "Misc/AutomationTest.h"

namespace RunnerNoSelectionNoGoalTests
{
	using namespace
		FMCodex::Tests::MatchPlayRunnerNoSelectionNoGoal;
	namespace MarkerFixtures =
		FMCodex::Tests::MatchPlayMarkerNoSelectionGoal;
	namespace SkillNoGoalFixtures =
		FMCodex::Tests::MatchPlaySkillNoSelectionNoGoal;
	namespace SkillFixtures =
		FMCodex::Tests::MatchPlayCurrentAttackSkillSelection;

	void TestStateUnchanged(
		FAutomationTestBase& Test,
		const TCHAR* Context,
		const FMatchPlayState& Before,
		const FMatchPlayState& After)
	{
		Test.TestTrue(
			*FString::Printf(TEXT("%s state unchanged"), Context),
			AreStatesEqual(Before, After));
	}

	bool IsCardUsed(
		const FMatchPlayState& State,
		const EInitialTurnOrderPlayer Side,
		const FName CardId)
	{
		return Side == EInitialTurnOrderPlayer::PlayerA
			? State.CardUsageState.PlayerACardUsageState
				.UsedCardIds.Contains(CardId)
			: State.CardUsageState.PlayerBCardUsageState
				.UsedCardIds.Contains(CardId);
	}

	bool IsCardAvailable(
		const FMatchPlayState& State,
		const EInitialTurnOrderPlayer Side,
		const FName CardId)
	{
		return Side == EInitialTurnOrderPlayer::PlayerA
			? State.CardUsageState.PlayerACardUsageState
				.AvailableCardIds.Contains(CardId)
			: State.CardUsageState.PlayerBCardUsageState
				.AvailableCardIds.Contains(CardId);
	}

	void TestFormalRunnerAbsenceProgression(
		FAutomationTestBase& Test,
		const TCHAR* Context,
		const FMatchPlayState& Before,
		const FMatchPlayCurrentAttackCompletionResult& Completion,
		const FMatchPlayState& After)
	{
		FMatchPlayState Expected = Before;
		FMatchPlayCurrentAttackState& Attack = Expected.CurrentAttack;
		Attack.ActionPreparation.bSkillSelectionDeferred = true;
		Attack.ActionPreparation.SkillId = NAME_None;
		Attack.ActionPreparation.ActionType = ESkillRuleType::None;
		Attack.ActionPreparation.RunnerCardId = NAME_None;
		Attack.ActionPreparation.bHasHelper = false;
		Attack.ActionPreparation.HelperCardId = NAME_None;
		Attack.SelectionStage =
			EMatchPlayCurrentAttackSelectionStage::AwaitingSkill;

		Test.TestTrue(
			*FString::Printf(TEXT("%s exact non-terminal mutation"), Context),
			AreStatesEqual(Expected, After));
		Test.TestTrue(
			*FString::Printf(TEXT("%s attack remains active"), Context),
			After.bHasCurrentAttack
				&& After.CurrentAttack.LifecycleState
					== EMatchPlayCurrentAttackLifecycleState::Active);
		Test.TestEqual(
			*FString::Printf(TEXT("%s attack sequence preserved"), Context),
			After.CurrentAttack.AttackSequence,
			Before.CurrentAttack.AttackSequence);
		Test.TestEqual(
			*FString::Printf(TEXT("%s attacker preserved"), Context),
			After.RuntimeState.CurrentAttackingPlayer,
			Before.RuntimeState.CurrentAttackingPlayer);
		Test.TestFalse(
			*FString::Printf(TEXT("%s no terminal handoff"), Context),
			Completion.bMatchEnded);
		Test.TestEqual(
			*FString::Printf(TEXT("%s no next attacker"), Context),
			Completion.NextAttackingPlayer,
			EInitialTurnOrderPlayer::None);
		Test.TestEqual(
			*FString::Printf(TEXT("%s consumes no ordinary cards"), Context),
			Completion.OrdinaryCardUsageResults.Num(),
			0);
		Test.TestFalse(
			*FString::Printf(TEXT("%s consumes no opportunity"), Context),
			Completion.OpportunityResolveResult.bSuccess);
	}
}

#define RUNNER_NO_SELECTION_NO_GOAL_TEST(TestClass, TestName) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
		TestClass, \
		"FMCodex.CoreRules.MatchPlayRunnerNoSelectionNoGoal." TestName, \
		EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

RUNNER_NO_SELECTION_NO_GOAL_TEST(
	FRunnerNoGoalMutualExclusivityTest,
	"Entries.MutualExclusivityAndBothAttackers")

bool FRunnerNoGoalMutualExclusivityTest::RunTest(
	const FString& Parameters)
{
	using namespace RunnerNoSelectionNoGoalTests;
	for (const EInitialTurnOrderPlayer Attacker :
		{EInitialTurnOrderPlayer::PlayerA,
			EInitialTurnOrderPlayer::PlayerB})
	{
		const FMatchPlayState NoLegalState = MakeState(Attacker);
		const FMatchPlayResolveNoLegalRunnerResult Resolved =
			FMatchPlayResolveNoLegalRunner::Resolve(
				NoLegalState,
				MakeResolveRequest());
		TestTrue(TEXT("No-legal Runner system path succeeds"),
			Resolved.bSuccess);
		TestTrue(TEXT("Resolve Availability succeeds"),
			Resolved.RunnerAvailabilityResult.bQuerySucceeded);
		TestFalse(TEXT("Resolve has no legal Runner"),
			Resolved.RunnerAvailabilityResult.bCanSelectAnyRunner);
		TestEqual(TEXT("Resolve source"),
			Resolved.Source,
			EMatchPlayRunnerNoSelectionNoGoalSource
				::ResolveNoLegalRunner);
		TestEqual(TEXT("Resolve reason"),
			Resolved.Reason,
			EMatchPlayRunnerNoSelectionNoGoalReason
				::NoLegalRunner);

		const FMatchPlayRunnerDeclineResult EmptyDecline =
			FMatchPlayRunnerDecline::Decline(
				NoLegalState,
				MakeDeclineRequest(Attacker));
		TestFalse(TEXT("Zero-legal Decline rejects"),
			EmptyDecline.bSuccess);
		TestEqual(TEXT("Zero-legal Decline exact error"),
			EmptyDecline.ErrorCode,
			EMatchPlayRunnerNoSelectionNoGoalErrorCode
				::NoLegalRunnerToDecline);
		TestStateUnchanged(
			*this,
			TEXT("Zero-legal Decline"),
			NoLegalState,
			EmptyDecline.AfterState);

		FMatchPlayState LegalState = MakeState(Attacker);
		AddLegalRunner(LegalState);
		const FMatchPlayResolveNoLegalRunnerResult LegalResolve =
			FMatchPlayResolveNoLegalRunner::Resolve(
				LegalState,
				MakeResolveRequest());
		TestFalse(TEXT("Legal Runner system path rejects"),
			LegalResolve.bSuccess);
		TestEqual(TEXT("Legal Runner exact system error"),
			LegalResolve.ErrorCode,
			EMatchPlayRunnerNoSelectionNoGoalErrorCode
				::LegalRunnerExists);
		TestStateUnchanged(
			*this,
			TEXT("Legal Runner system path"),
			LegalState,
			LegalResolve.AfterState);

		const FMatchPlayRunnerDeclineResult Declined =
			FMatchPlayRunnerDecline::Decline(
				LegalState,
				MakeDeclineRequest(Attacker));
		TestTrue(TEXT("Legal Runner Decline succeeds"),
			Declined.bSuccess);
		TestTrue(TEXT("Decline Availability has legal Runner"),
			Declined.RunnerAvailabilityResult.bCanSelectAnyRunner);
		TestEqual(TEXT("Decline source"),
			Declined.Source,
			EMatchPlayRunnerNoSelectionNoGoalSource
				::RunnerDecline);
		TestEqual(TEXT("Decline reason"),
			Declined.Reason,
			EMatchPlayRunnerNoSelectionNoGoalReason
				::RunnerDeclined);

		FMatchPlayState MultipleState = MakeState(Attacker);
		AddLegalRunner(MultipleState, LegalRunnerOneId);
		AddLegalRunner(MultipleState, LegalRunnerTwoId);
		const FMatchPlayResolveNoLegalRunnerResult MultipleResolve =
			FMatchPlayResolveNoLegalRunner::Resolve(
				MultipleState,
				MakeResolveRequest());
		TestEqual(TEXT("Multiple legal Runners reject Resolve"),
			MultipleResolve.ErrorCode,
			EMatchPlayRunnerNoSelectionNoGoalErrorCode
				::LegalRunnerExists);
		const FMatchPlayRunnerDeclineResult MultipleDecline =
			FMatchPlayRunnerDecline::Decline(
				MultipleState,
				MakeDeclineRequest(Attacker));
		TestTrue(TEXT("Multiple legal Runners allow Decline"),
			MultipleDecline.bSuccess);

		FMatchPlayState MixedLegalState = MakeState(Attacker);
		AddLegalRunner(MixedLegalState);
		AddIllegalRunner(MixedLegalState);
		const FMatchPlayRunnerDeclineResult MixedLegalDecline =
			FMatchPlayRunnerDecline::Decline(
				MixedLegalState,
				MakeDeclineRequest(Attacker));
		TestTrue(TEXT("Legal/illegal mix allows Decline"),
			MixedLegalDecline.bSuccess);
	}
	return true;
}

RUNNER_NO_SELECTION_NO_GOAL_TEST(
	FRunnerNoGoalCandidateCasesTest,
	"Entries.CandidateShapesUseAvailabilitySummary")

bool FRunnerNoGoalCandidateCasesTest::RunTest(
	const FString& Parameters)
{
	using namespace RunnerNoSelectionNoGoalTests;

	const FMatchPlayState CarrierOnly = MakeState();
	const FMatchPlayResolveNoLegalRunnerResult CarrierConflict =
		FMatchPlayResolveNoLegalRunner::Resolve(
			CarrierOnly,
			MakeResolveRequest());
	TestTrue(TEXT("Carrier-only candidate resolves NoGoal"),
		CarrierConflict.bSuccess);
	TestEqual(TEXT("Carrier remains diagnostic candidate"),
		CarrierConflict.RunnerAvailabilityResult.Candidates.Num(),
		1);

	FMatchPlayState PassState =
		MakeState(
			EInitialTurnOrderPlayer::PlayerA,
			ESkillRuleType::PassControl);
	AddIllegalRunner(PassState);
	const FMatchPlayResolveNoLegalRunnerResult PassResult =
		FMatchPlayResolveNoLegalRunner::Resolve(
			PassState,
			MakeResolveRequest());
	TestTrue(TEXT("PassControl candidates without Midfield resolve"),
		PassResult.bSuccess);

	FMatchPlayState CrossState =
		MakeState(
			EInitialTurnOrderPlayer::PlayerA,
			ESkillRuleType::Cross);
	AddIllegalRunner(CrossState);
	const FMatchPlayResolveNoLegalRunnerResult CrossResult =
		FMatchPlayResolveNoLegalRunner::Resolve(
			CrossState,
			MakeResolveRequest());
	TestTrue(TEXT("Cross candidates without Attack resolve"),
		CrossResult.bSuccess);

	FMatchPlayState ThroughBallState =
		MakeState(
			EInitialTurnOrderPlayer::PlayerA,
			ESkillRuleType::ThroughBall);
	AddIllegalRunner(ThroughBallState);
	const FMatchPlayResolveNoLegalRunnerResult ThroughBallResult =
		FMatchPlayResolveNoLegalRunner::Resolve(
			ThroughBallState,
			MakeResolveRequest());
	TestTrue(TEXT("ThroughBall non-forward candidates resolve"),
		ThroughBallResult.bSuccess);

	FMatchPlayState MixedState = MakeState();
	AddIllegalRunner(MixedState, IllegalRunnerOneId);
	AddIllegalRunner(MixedState, IllegalRunnerTwoId);
	const FMatchPlayResolveNoLegalRunnerResult MixedResult =
		FMatchPlayResolveNoLegalRunner::Resolve(
			MixedState,
			MakeResolveRequest());
	TestTrue(TEXT("Mixed all-illegal candidates resolve"),
		MixedResult.bSuccess);
	TestFalse(TEXT("Mixed all-illegal summary stays false"),
		MixedResult.RunnerAvailabilityResult.bCanSelectAnyRunner);
	return true;
}

RUNNER_NO_SELECTION_NO_GOAL_TEST(
	FRunnerNoGoalRequestValidationTest,
	"Entries.SequenceStageSideAndGlobalFailures")

bool FRunnerNoGoalRequestValidationTest::RunTest(
	const FString& Parameters)
{
	using namespace RunnerNoSelectionNoGoalTests;
	const FMatchPlayState State = MakeState();

	FMatchPlayResolveNoLegalRunnerRequest StaleRequest =
		MakeResolveRequest();
	++StaleRequest.AttackSequence;
	const FMatchPlayResolveNoLegalRunnerResult Stale =
		FMatchPlayResolveNoLegalRunner::Resolve(
			State,
			StaleRequest);
	TestEqual(TEXT("Stale sequence maps exactly"),
		Stale.ErrorCode,
		EMatchPlayRunnerNoSelectionNoGoalErrorCode
			::AttackSequenceMismatch);
	TestStateUnchanged(
		*this,
		TEXT("Stale sequence"),
		State,
		Stale.AfterState);

	const FMatchPlayRunnerDeclineResult WrongSide =
		FMatchPlayRunnerDecline::Decline(
			State,
			MakeDeclineRequest(
				EInitialTurnOrderPlayer::PlayerB));
	TestEqual(TEXT("Wrong side maps exactly"),
		WrongSide.ErrorCode,
		EMatchPlayRunnerNoSelectionNoGoalErrorCode
			::RequestingSideIsNotCurrentAttacker);
	TestStateUnchanged(
		*this,
		TEXT("Wrong side"),
		State,
		WrongSide.AfterState);

	const FMatchPlayRunnerDeclineResult InvalidSide =
		FMatchPlayRunnerDecline::Decline(
			State,
			MakeDeclineRequest(
				EInitialTurnOrderPlayer::None));
	TestEqual(TEXT("Invalid side maps exactly"),
		InvalidSide.ErrorCode,
		EMatchPlayRunnerNoSelectionNoGoalErrorCode
			::InvalidRequestingSide);

	FMatchPlayState NoAttack = State;
	NoAttack.bHasCurrentAttack = false;
	NoAttack.CurrentAttack = FMatchPlayCurrentAttackState();
	const FMatchPlayResolveNoLegalRunnerResult NoAttackResolve =
		FMatchPlayResolveNoLegalRunner::Resolve(
			NoAttack,
			MakeResolveRequest());
	TestEqual(TEXT("Resolve no CurrentAttack maps exactly"),
		NoAttackResolve.ErrorCode,
		EMatchPlayRunnerNoSelectionNoGoalErrorCode
			::NoCurrentAttack);
	const FMatchPlayRunnerDeclineResult NoAttackDecline =
		FMatchPlayRunnerDecline::Decline(
			NoAttack,
			MakeDeclineRequest());
	TestEqual(TEXT("Decline no CurrentAttack maps exactly"),
		NoAttackDecline.ErrorCode,
		EMatchPlayRunnerNoSelectionNoGoalErrorCode
			::NoCurrentAttack);

	for (const EMatchPlayCurrentAttackSelectionStage WrongStage :
		{EMatchPlayCurrentAttackSelectionStage::AwaitingCarrier,
			EMatchPlayCurrentAttackSelectionStage::AwaitingMarker,
			EMatchPlayCurrentAttackSelectionStage::AwaitingSkill,
			EMatchPlayCurrentAttackSelectionStage::AwaitingHelper,
			EMatchPlayCurrentAttackSelectionStage::ReadyForResolution})
	{
		FMatchPlayState WrongStageState = State;
		WrongStageState.CurrentAttack.SelectionStage = WrongStage;
		const FMatchPlayResolveNoLegalRunnerResult Result =
			FMatchPlayResolveNoLegalRunner::Resolve(
				WrongStageState,
				MakeResolveRequest());
		TestFalse(TEXT("Every non-Runner stage rejects"),
			Result.bSuccess);
		TestStateUnchanged(
			*this,
			TEXT("Wrong stage"),
			WrongStageState,
			Result.AfterState);
	}

	FMatchPlayState CorruptPreparation = State;
	CorruptPreparation.CurrentAttack.ActionPreparation.SkillId =
		NAME_None;
	const FMatchPlayResolveNoLegalRunnerResult CorruptResult =
		FMatchPlayResolveNoLegalRunner::Resolve(
			CorruptPreparation,
			MakeResolveRequest());
	TestFalse(TEXT("Corrupt preparation rejects"),
		CorruptResult.bSuccess);
	TestStateUnchanged(
		*this,
		TEXT("Corrupt preparation"),
		CorruptPreparation,
		CorruptResult.AfterState);

	FMatchPlayState InvalidSnapshots = State;
	const FPlayerCardRuleSnapshot DuplicateSnapshot =
		InvalidSnapshots.CardSnapshotAuthority
			.PlayerACardSnapshots.Cards[0];
	InvalidSnapshots.CardSnapshotAuthority.PlayerACardSnapshots.Cards.Add(
		DuplicateSnapshot);
	const FMatchPlayResolveNoLegalRunnerResult SnapshotResult =
		FMatchPlayResolveNoLegalRunner::Resolve(
			InvalidSnapshots,
			MakeResolveRequest());
	TestEqual(TEXT("Invalid attacking Snapshot set maps exactly"),
		SnapshotResult.ErrorCode,
		EMatchPlayRunnerNoSelectionNoGoalErrorCode
			::InvalidAttackingSnapshotSet);
	TestStateUnchanged(
		*this,
		TEXT("Invalid Snapshot set"),
		InvalidSnapshots,
		SnapshotResult.AfterState);

	FMatchPlayState InvalidCatalog =
		MakeState(
			EInitialTurnOrderPlayer::PlayerA,
			ESkillRuleType::ThroughBall);
	InvalidCatalog.DeploymentSlotCatalog.Slots.Reset();
	const FMatchPlayResolveNoLegalRunnerResult CatalogResult =
		FMatchPlayResolveNoLegalRunner::Resolve(
			InvalidCatalog,
			MakeResolveRequest());
	TestEqual(TEXT("Invalid ThroughBall catalog maps exactly"),
		CatalogResult.ErrorCode,
		EMatchPlayRunnerNoSelectionNoGoalErrorCode
			::InvalidSlotCatalog);
	TestStateUnchanged(
		*this,
		TEXT("Invalid Slot catalog"),
		InvalidCatalog,
		CatalogResult.AfterState);
	return true;
}

RUNNER_NO_SELECTION_NO_GOAL_TEST(
	FRunnerNoGoalLifecycleTest,
	"Progression.FormalAbsencePreservesAttackAndResources")

bool FRunnerNoGoalLifecycleTest::RunTest(
	const FString& Parameters)
{
	using namespace RunnerNoSelectionNoGoalTests;
	for (const EInitialTurnOrderPlayer Attacker :
		{EInitialTurnOrderPlayer::PlayerA,
			EInitialTurnOrderPlayer::PlayerB})
	{
		FMatchPlayState DeclineState = MakeState(Attacker);
		AddLegalRunner(DeclineState);
		AddActiveGoalkeeper(DeclineState);
		const FMatchPlayRunnerDeclineResult Declined =
			FMatchPlayRunnerDecline::Decline(
				DeclineState,
				MakeDeclineRequest(Attacker));
		TestTrue(TEXT("Runner decline succeeds"), Declined.bSuccess);
		TestFormalRunnerAbsenceProgression(
			*this,
			TEXT("Runner decline"),
			DeclineState,
			Declined.CompletionResult,
			Declined.AfterState);

		const FMatchPlayState NoLegalState = MakeState(Attacker);
		const FMatchPlayResolveNoLegalRunnerResult Resolved =
			FMatchPlayResolveNoLegalRunner::Resolve(
				NoLegalState,
				MakeResolveRequest());
		TestTrue(TEXT("No-legal Runner resolution succeeds"),
			Resolved.bSuccess);
		TestFormalRunnerAbsenceProgression(
			*this,
			TEXT("No-legal Runner"),
			NoLegalState,
			Resolved.CompletionResult,
			Resolved.AfterState);
	}
	return true;
}

RUNNER_NO_SELECTION_NO_GOAL_TEST(
	FRunnerNoGoalPlacementAtomicityTest,
	"Progression.InputValidationAndScoreFailureAreAtomic")

bool FRunnerNoGoalPlacementAtomicityTest::RunTest(
	const FString& Parameters)
{
	using namespace RunnerNoSelectionNoGoalTests;

	FMatchPlayState State = MakeState();
	AddLegalRunner(State);
	const FMatchPlayDeploymentPlacement DuplicateRunnerPlacement =
		State.CurrentAttack.DeploymentPlacements.Last();
	State.CurrentAttack.DeploymentPlacements.Add(
		DuplicateRunnerPlacement);
	FMatchPlayRunnerDeclineResult Result =
		FMatchPlayRunnerDecline::Decline(
			State,
			MakeDeclineRequest());
	TestEqual(TEXT("Same-side duplicate CardId rejects"),
		Result.ErrorCode,
		EMatchPlayRunnerNoSelectionNoGoalErrorCode
			::DuplicateDeploymentCard);
	TestStateUnchanged(
		*this,
		TEXT("Same-side duplicate CardId"),
		State,
		Result.AfterState);

	State = MakeState();
	AddLegalRunner(State);
	State.CurrentAttack.DeploymentPlacements.Last().SlotId =
		State.CurrentAttack.DeploymentPlacements[0].SlotId;
	Result = FMatchPlayRunnerDecline::Decline(
		State,
		MakeDeclineRequest());
	TestEqual(TEXT("Duplicate SlotId rejects"),
		Result.ErrorCode,
		EMatchPlayRunnerNoSelectionNoGoalErrorCode
			::DuplicateDeploymentSlot);
	TestStateUnchanged(
		*this,
		TEXT("Duplicate SlotId"),
		State,
		Result.AfterState);

	State = MakeState();
	AddLegalRunner(State);
	State.RuntimeState.PlayerBState.Score = -1;
	Result = FMatchPlayRunnerDecline::Decline(
		State,
		MakeDeclineRequest());
	TestEqual(TEXT("Negative Score exact error"),
		Result.CompletionResult.ErrorCode,
		EMatchPlayCurrentAttackCompletionErrorCode
			::InvalidScoreState);
	TestStateUnchanged(
		*this,
		TEXT("Negative Score"),
		State,
		Result.AfterState);
	return true;
}

RUNNER_NO_SELECTION_NO_GOAL_TEST(
	FRunnerNoGoalAttackerGoalkeeperAtomicFailureTest,
	"Progression.AttackerGoalkeeperIsPreserved")

bool FRunnerNoGoalAttackerGoalkeeperAtomicFailureTest::RunTest(
	const FString& Parameters)
{
	using namespace RunnerNoSelectionNoGoalTests;
	FMatchPlayState State = MakeState();
	AddAttackerGoalkeeper(State);
	const FMatchPlayResolveNoLegalRunnerResult Result =
		FMatchPlayResolveNoLegalRunner::Resolve(
			State,
			MakeResolveRequest());
	TestTrue(TEXT("Attacker-side GK does not block progression"),
		Result.bSuccess);
	TestFalse(TEXT("Attacker-side GK is not a legal Runner"),
		Result.RunnerAvailabilityResult.bCanSelectAnyRunner);
	TestFormalRunnerAbsenceProgression(
		*this,
		TEXT("Attacker-side GK"),
		State,
		Result.CompletionResult,
		Result.AfterState);
	return true;
}

RUNNER_NO_SELECTION_NO_GOAL_TEST(
	FRunnerNoGoalPersistentGoalkeeperAtomicFailureTest,
	"Progression.ActiveGoalkeeperStateIsPreserved")

bool FRunnerNoGoalPersistentGoalkeeperAtomicFailureTest::RunTest(
	const FString& Parameters)
{
	using namespace RunnerNoSelectionNoGoalTests;
	FMatchPlayState State = MakeState();
	AddLegalRunner(State);
	AddActiveGoalkeeper(State);
	const FMatchPlayRunnerDeclineResult Result =
		FMatchPlayRunnerDecline::Decline(
			State,
			MakeDeclineRequest());
	TestTrue(TEXT("Active goalkeeper progression succeeds"),
		Result.bSuccess);
	TestFormalRunnerAbsenceProgression(
		*this,
		TEXT("Active goalkeeper"),
		State,
		Result.CompletionResult,
		Result.AfterState);
	return true;
}

RUNNER_NO_SELECTION_NO_GOAL_TEST(
	FRunnerNoGoalSameCardAcrossSidesTest,
	"Progression.SameCardIdAcrossSidesRemainsAvailable")

bool FRunnerNoGoalSameCardAcrossSidesTest::RunTest(
	const FString& Parameters)
{
	using namespace RunnerNoSelectionNoGoalTests;
	FMatchPlayState State = MakeState();
	AddLegalRunner(State);
	AddOrdinaryDeployment(
		State,
		EInitialTurnOrderPlayer::PlayerA,
		SharedCardId,
		{EPlayerPositionType::Defense},
		GetNearSide(EInitialTurnOrderPlayer::PlayerA));
	State.CardSnapshotAuthority.PlayerBCardSnapshots.Cards.Add(
		RunnerFixtures::MakeCard(
			SharedCardId,
			{EPlayerPositionType::Defense}));
	State.CardUsageState.PlayerBCardUsageState.AvailableCardIds.Add(
		SharedCardId);
	State.CurrentAttack.DeploymentPlacements.Add(
		RunnerFixtures::MakePlacement(
			EInitialTurnOrderPlayer::PlayerB,
			SharedCardId,
			TEXT("Slot.RunnerNoGoal.Shared.PlayerB")));
	State.DeploymentSlotCatalog.Slots.Add(
		RunnerFixtures::MakeSlot(
			TEXT("Slot.RunnerNoGoal.Shared.PlayerB"),
			GetNearSide(EInitialTurnOrderPlayer::PlayerB)));
	const FMatchPlayRunnerDeclineResult Result =
		FMatchPlayRunnerDecline::Decline(
			State,
			MakeDeclineRequest());
	TestTrue(TEXT("Same CardId across sides succeeds"),
		Result.bSuccess);
	TestTrue(TEXT("PlayerA shared card remains Available"),
		IsCardAvailable(
			Result.AfterState,
			EInitialTurnOrderPlayer::PlayerA,
			SharedCardId));
	TestTrue(TEXT("PlayerB shared card remains Available"),
		IsCardAvailable(
			Result.AfterState,
			EInitialTurnOrderPlayer::PlayerB,
			SharedCardId));
	TestFalse(TEXT("PlayerA shared card is not consumed"),
		IsCardUsed(Result.AfterState,
			EInitialTurnOrderPlayer::PlayerA, SharedCardId));
	TestFalse(TEXT("PlayerB shared card is not consumed"),
		IsCardUsed(Result.AfterState,
			EInitialTurnOrderPlayer::PlayerB, SharedCardId));
	return true;
}

RUNNER_NO_SELECTION_NO_GOAL_TEST(
	FRunnerNoGoalNextAttackerAndMatchResultTest,
	"Progression.NeverHandsOffOrCreatesTerminalResult")

bool FRunnerNoGoalNextAttackerAndMatchResultTest::RunTest(
	const FString& Parameters)
{
	using namespace RunnerNoSelectionNoGoalTests;
	FMatchPlayState State = MakeState();
	AddLegalRunner(State);
	FMatchPlayRunnerDeclineResult Result =
		FMatchPlayRunnerDecline::Decline(
			State,
			MakeDeclineRequest());
	TestEqual(TEXT("No opponent handoff"),
		Result.CompletionResult.NextAttackingPlayer,
		EInitialTurnOrderPlayer::None);
	TestEqual(TEXT("Current attacker remains authoritative"),
		Result.AfterState.RuntimeState.CurrentAttackingPlayer,
		EInitialTurnOrderPlayer::PlayerA);

	State = MakeState();
	AddLegalRunner(State);
	State.RuntimeState.PlayerBState.UsedAttackCount =
		State.RuntimeState.PlayerBState.TotalAttackCount;
	Result = FMatchPlayRunnerDecline::Decline(
		State,
		MakeDeclineRequest());
	TestEqual(TEXT("No handoff when defender has no opportunities"),
		Result.CompletionResult.NextAttackingPlayer,
		EInitialTurnOrderPlayer::None);

	for (const TTuple<int32, int32, EMatchResultType>& Case :
		{MakeTuple(2, 1, EMatchResultType::HomeWin),
			MakeTuple(1, 2, EMatchResultType::AwayWin),
			MakeTuple(1, 1, EMatchResultType::Draw)})
	{
		State = MakeState();
		AddLegalRunner(State);
		State.RuntimeState.PlayerAState.TotalAttackCount = 2;
		State.RuntimeState.PlayerAState.UsedAttackCount = 1;
		State.RuntimeState.PlayerBState.TotalAttackCount = 1;
		State.RuntimeState.PlayerBState.UsedAttackCount = 1;
		State.RuntimeState.PlayerAState.Score = Case.Get<0>();
		State.RuntimeState.PlayerBState.Score = Case.Get<1>();
		Result = FMatchPlayRunnerDecline::Decline(
			State,
			MakeDeclineRequest());
		TestTrue(TEXT("Runner absence still succeeds at last opportunity"),
			Result.bSuccess);
		TestFalse(TEXT("Runner absence does not end match"),
			Result.CompletionResult.bMatchEnded);
		TestFalse(TEXT("No match result is resolved"),
			Result.CompletionResult.MatchResultResolveResult.bSuccess);
		TestEqual(TEXT("No next attacker before shot resolves"),
			Result.CompletionResult.NextAttackingPlayer,
			EInitialTurnOrderPlayer::None);
	}
	return true;
}

RUNNER_NO_SELECTION_NO_GOAL_TEST(
	FRunnerNoGoalRepeatedCallTest,
	"Progression.RepeatedRunnerCommandsAreStaleSafe")

bool FRunnerNoGoalRepeatedCallTest::RunTest(
	const FString& Parameters)
{
	using namespace RunnerNoSelectionNoGoalTests;
	const FMatchPlayResolveNoLegalRunnerResult Resolved =
		FMatchPlayResolveNoLegalRunner::Resolve(
			MakeState(),
			MakeResolveRequest());
	TestTrue(TEXT("Initial Resolve succeeds"),
		Resolved.bSuccess);
	const FMatchPlayResolveNoLegalRunnerResult RepeatedResolve =
		FMatchPlayResolveNoLegalRunner::Resolve(
			Resolved.AfterState,
			MakeResolveRequest());
	TestEqual(TEXT("Resolve then Resolve has wrong stage"),
		RepeatedResolve.ErrorCode,
		EMatchPlayRunnerNoSelectionNoGoalErrorCode
			::WrongSelectionStage);
	TestStateUnchanged(
		*this,
		TEXT("Resolve then Resolve"),
		Resolved.AfterState,
		RepeatedResolve.AfterState);
	const FMatchPlayRunnerDeclineResult ResolveThenDecline =
		FMatchPlayRunnerDecline::Decline(
			Resolved.AfterState,
			MakeDeclineRequest());
	TestEqual(TEXT("Resolve then Decline has wrong stage"),
		ResolveThenDecline.ErrorCode,
		EMatchPlayRunnerNoSelectionNoGoalErrorCode
			::WrongSelectionStage);
	TestStateUnchanged(
		*this,
		TEXT("Resolve then Decline"),
		Resolved.AfterState,
		ResolveThenDecline.AfterState);

	FMatchPlayState LegalState = MakeState();
	AddLegalRunner(LegalState);
	const FMatchPlayRunnerDeclineResult Declined =
		FMatchPlayRunnerDecline::Decline(
			LegalState,
			MakeDeclineRequest());
	TestTrue(TEXT("Initial Decline succeeds"),
		Declined.bSuccess);
	const FMatchPlayRunnerDeclineResult RepeatedDecline =
		FMatchPlayRunnerDecline::Decline(
			Declined.AfterState,
			MakeDeclineRequest());
	TestEqual(TEXT("Decline then Decline has wrong stage"),
		RepeatedDecline.ErrorCode,
		EMatchPlayRunnerNoSelectionNoGoalErrorCode
			::WrongSelectionStage);
	TestStateUnchanged(
		*this,
		TEXT("Decline then Decline"),
		Declined.AfterState,
		RepeatedDecline.AfterState);
	const FMatchPlayResolveNoLegalRunnerResult DeclineThenResolve =
		FMatchPlayResolveNoLegalRunner::Resolve(
			Declined.AfterState,
			MakeResolveRequest());
	TestEqual(TEXT("Decline then Resolve has wrong stage"),
		DeclineThenResolve.ErrorCode,
		EMatchPlayRunnerNoSelectionNoGoalErrorCode
			::WrongSelectionStage);
	TestStateUnchanged(
		*this,
		TEXT("Decline then Resolve"),
		Declined.AfterState,
		DeclineThenResolve.AfterState);
	return true;
}

RUNNER_NO_SELECTION_NO_GOAL_TEST(
	FRunnerNoGoalCrossFamilyRepeatedCallTest,
	"Progression.LongShotAllowedAndRunnerTacticsRejected")

bool FRunnerNoGoalCrossFamilyRepeatedCallTest::RunTest(
	const FString& Parameters)
{
	using namespace RunnerNoSelectionNoGoalTests;
	const FSkillRuleSnapshotSet SkillRules =
		SkillFixtures::MakeRuleSet();

	FMatchPlayState RunnerLegal = MakeState();
	AddLegalRunner(RunnerLegal);
	const FMatchPlayResolveNoLegalRunnerResult RunnerResolved =
		FMatchPlayResolveNoLegalRunner::Resolve(
			MakeState(),
			MakeResolveRequest());
	const FMatchPlayRunnerDeclineResult RunnerDeclined =
		FMatchPlayRunnerDecline::Decline(
			RunnerLegal,
			MakeDeclineRequest());
	TestTrue(TEXT("Runner Resolve seed succeeds"),
		RunnerResolved.bSuccess);
	TestTrue(TEXT("Runner Decline seed succeeds"),
		RunnerDeclined.bSuccess);

	for (const FMatchPlayState* RunnerCompleted :
		{&RunnerResolved.AfterState,
			&RunnerDeclined.AfterState})
	{
		FMatchPlayState NoRunnerState = *RunnerCompleted;
		FPlayerCardRuleSnapshot* Carrier = GetSnapshots(
			NoRunnerState,
			EInitialTurnOrderPlayer::PlayerA).Cards.FindByPredicate(
			[](const FPlayerCardRuleSnapshot& Snapshot)
			{
				return Snapshot.CardId == RunnerFixtures::CarrierId;
			});
		if (Carrier == nullptr)
		{
			AddError(TEXT("No-runner state lost the carrier snapshot."));
			continue;
		}
		Carrier->SkillIds.AddUnique(SkillFixtures::LongShotSkillId);
		Carrier->SkillIds.AddUnique(SkillFixtures::CrossSkillId);
		FMatchPlayCurrentAttackSkillSelectionRequest LongShotRequest =
			SkillFixtures::MakeRequest(SkillFixtures::LongShotSkillId);
		LongShotRequest.AttackSequence = RunnerFixtures::ValidAttackSequence;

		const FMatchPlayCurrentAttackSkillSelectionWriterResult LongShot =
			FMatchPlayCurrentAttackSkillSelectionWriter::Select(
				NoRunnerState,
				SkillRules,
				LongShotRequest);
		TestTrue(TEXT("LongShot is legal after formal Runner absence"),
			LongShot.bSuccess);
		TestEqual(TEXT("LongShot reaches branch choice"),
			LongShot.AfterState.CurrentAttack.SelectionStage,
			EMatchPlayCurrentAttackSelectionStage::AwaitingBranchIntent);
		TestTrue(TEXT("LongShot keeps the current attack active"),
			LongShot.AfterState.bHasCurrentAttack);
		TestTrue(TEXT("LongShot keeps Runner absent"),
			LongShot.AfterState.CurrentAttack.ActionPreparation
				.RunnerCardId.IsNone());
		TestFalse(TEXT("LongShot keeps Helper not applicable"),
			LongShot.AfterState.CurrentAttack.ActionPreparation.bHasHelper);

		FMatchPlayCurrentAttackSkillSelectionRequest CrossRequest =
			SkillFixtures::MakeRequest(SkillFixtures::CrossSkillId);
		CrossRequest.AttackSequence = RunnerFixtures::ValidAttackSequence;
		const FMatchPlayCurrentAttackSkillSelectionWriterResult Cross =
			FMatchPlayCurrentAttackSkillSelectionWriter::Select(
				NoRunnerState,
				SkillRules,
				CrossRequest);
		TestFalse(TEXT("Runner-required Cross remains illegal"),
			Cross.bSuccess);
		TestEqual(TEXT("Cross reports missing prepared Runner"),
			Cross.LegalityResult.ErrorCode,
			EMatchPlayCurrentAttackSkillSelectionErrorCode
				::PreparedRunnerIncompatibleWithSkill);
		TestStateUnchanged(
			*this,
			TEXT("Rejected Cross"),
			NoRunnerState,
			Cross.AfterState);

		const FMatchPlayResolveNoLegalMarkerResult MarkerResolve =
			FMatchPlayResolveNoLegalMarker::Resolve(
				*RunnerCompleted,
				MarkerFixtures::MakeNoLegalRequest());
		const FMatchPlayMarkerDeclineResult MarkerDecline =
			FMatchPlayMarkerDecline::Decline(
				*RunnerCompleted,
				MarkerFixtures::MakeDeclineRequest());
		TestFalse(TEXT("Runner then Marker Resolve rejects"),
			MarkerResolve.bSuccess);
		TestFalse(TEXT("Runner then Marker Decline rejects"),
			MarkerDecline.bSuccess);
		TestStateUnchanged(
			*this,
			TEXT("Runner then Marker Resolve"),
			*RunnerCompleted,
			MarkerResolve.AfterState);
		TestStateUnchanged(
			*this,
			TEXT("Runner then Marker Decline"),
			*RunnerCompleted,
			MarkerDecline.AfterState);
	}
	return true;
}

#undef RUNNER_NO_SELECTION_NO_GOAL_TEST

#endif
