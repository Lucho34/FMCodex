#include "MatchPlayRunnerNoSelectionNoGoal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "MatchPlayRunnerNoSelectionNoGoalTestFixtures.h"
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
	"Completion.ScoreCardUsageGoalkeeperClearAndOpportunity")

bool FRunnerNoGoalLifecycleTest::RunTest(
	const FString& Parameters)
{
	using namespace RunnerNoSelectionNoGoalTests;
	FMatchPlayState State = MakeState();
	AddLegalRunner(State);
	const EInitialTurnOrderPlayer Attacker =
		State.RuntimeState.CurrentAttackingPlayer;
	const EInitialTurnOrderPlayer Defender =
		GetDefender(Attacker);
	AddOrdinaryDeployment(
		State,
		Attacker,
		ExtraAttackerCardId,
		{EPlayerPositionType::Defense},
		GetNearSide(Attacker));
	AddOrdinaryDeployment(
		State,
		Defender,
		ExtraDefenderCardId,
		{EPlayerPositionType::Defense},
		GetNearSide(Defender));
	AddActiveGoalkeeper(State);
	const FMatchPlayState Before = State;

	const FMatchPlayRunnerDeclineResult Result =
		FMatchPlayRunnerDecline::Decline(
			State,
			MakeDeclineRequest(Attacker));
	TestTrue(TEXT("Lifecycle completion succeeds"),
		Result.bSuccess);
	TestEqual(TEXT("PlayerA score unchanged"),
		Result.AfterState.RuntimeState.PlayerAState.Score,
		Before.RuntimeState.PlayerAState.Score);
	TestEqual(TEXT("PlayerB score unchanged"),
		Result.AfterState.RuntimeState.PlayerBState.Score,
		Before.RuntimeState.PlayerBState.Score);
	TestFalse(TEXT("Runner NoGoal has no GoalResolver result"),
		Result.CompletionResult.GoalResolveResult.bSuccess);
	TestEqual(TEXT("Five ordinary cards consumed"),
		Result.CompletionResult.OrdinaryCardUsageResults.Num(),
		5);
	const TArray<FName> ExpectedOrder = {
		RunnerFixtures::CarrierId,
		RunnerFixtures::MarkerId,
		LegalRunnerOneId,
		ExtraAttackerCardId,
		ExtraDefenderCardId
	};
	for (int32 Index = 0;
		Index < ExpectedOrder.Num()
			&& Index
				< Result.CompletionResult
					.OrdinaryCardUsageResults.Num();
		++Index)
	{
		TestEqual(TEXT("Ordinary cards preserve placement order"),
			Result.CompletionResult
				.OrdinaryCardUsageResults[Index].CardId,
			ExpectedOrder[Index]);
	}
	for (const TPair<EInitialTurnOrderPlayer, FName>& ExpectedUsed :
		{TPair<EInitialTurnOrderPlayer, FName>(
				Attacker,
				RunnerFixtures::CarrierId),
			TPair<EInitialTurnOrderPlayer, FName>(
				Defender,
				RunnerFixtures::MarkerId),
			TPair<EInitialTurnOrderPlayer, FName>(
				Attacker,
				LegalRunnerOneId),
			TPair<EInitialTurnOrderPlayer, FName>(
				Attacker,
				ExtraAttackerCardId),
			TPair<EInitialTurnOrderPlayer, FName>(
				Defender,
				ExtraDefenderCardId)})
	{
		TestTrue(TEXT("Every ordinary deployment is Used"),
			IsCardUsed(
				Result.AfterState,
				ExpectedUsed.Key,
				ExpectedUsed.Value));
	}
	TestTrue(TEXT("Goalkeeper remains Available"),
		IsCardAvailable(
			Result.AfterState,
			Defender,
			GoalkeeperCardId));
	TestFalse(TEXT("Goalkeeper is not ordinary Used"),
		IsCardUsed(
			Result.AfterState,
			Defender,
			GoalkeeperCardId));
	TestTrue(TEXT("Persistent goalkeeper usage preserved"),
		Defender == EInitialTurnOrderPlayer::PlayerA
			? Result.AfterState.GoalkeeperUsageState
				.bPlayerAGoalkeeperCardUsed
			: Result.AfterState.GoalkeeperUsageState
				.bPlayerBGoalkeeperCardUsed);
	TestFalse(TEXT("Current attack flag cleared"),
		Result.AfterState.bHasCurrentAttack);
	const FMatchPlayCurrentAttackState EmptyAttack;
	TestTrue(TEXT("Current attack payload fully cleared"),
		FMatchPlayCurrentAttackState::StaticStruct()
			->CompareScriptStruct(
				&Result.AfterState.CurrentAttack,
				&EmptyAttack,
				0));
	TestEqual(TEXT("Attacker opportunity consumed once"),
		Result.AfterState.RuntimeState.PlayerAState.UsedAttackCount,
		Before.RuntimeState.PlayerAState.UsedAttackCount + 1);
	TestEqual(TEXT("Defender opportunity unchanged"),
		Result.AfterState.RuntimeState.PlayerBState.UsedAttackCount,
		Before.RuntimeState.PlayerBState.UsedAttackCount);
	TestFalse(TEXT("No automatic Begin"),
		Result.AfterState.bHasCurrentAttack);
	return true;
}

RUNNER_NO_SELECTION_NO_GOAL_TEST(
	FRunnerNoGoalPlacementAtomicityTest,
	"Completion.PlacementCardUsageScoreAndOpportunityFailuresAreAtomic")

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
	AddOrdinaryDeployment(
		State,
		EInitialTurnOrderPlayer::PlayerB,
		ExtraDefenderCardId,
		{EPlayerPositionType::Defense},
		GetNearSide(EInitialTurnOrderPlayer::PlayerB));
	State.CardSnapshotAuthority.PlayerBCardSnapshots.Cards.RemoveAll(
		[](const FPlayerCardRuleSnapshot& Snapshot)
		{
			return Snapshot.CardId == ExtraDefenderCardId;
		});
	Result = FMatchPlayRunnerDecline::Decline(
		State,
		MakeDeclineRequest());
	TestEqual(TEXT("Missing defender Snapshot reaches shared core"),
		Result.CompletionResult.ErrorCode,
		EMatchPlayCurrentAttackCompletionErrorCode
			::DeploymentSnapshotQueryFailed);
	TestStateUnchanged(
		*this,
		TEXT("Missing defender Snapshot"),
		State,
		Result.AfterState);

	State = MakeState();
	AddLegalRunner(State);
	AddOrdinaryDeployment(
		State,
		EInitialTurnOrderPlayer::PlayerB,
		ExtraDefenderCardId,
		{EPlayerPositionType::Defense},
		GetNearSide(EInitialTurnOrderPlayer::PlayerB));
	State.CardUsageState.PlayerBCardUsageState.AvailableCardIds
		.Remove(ExtraDefenderCardId);
	Result = FMatchPlayRunnerDecline::Decline(
		State,
		MakeDeclineRequest());
	TestEqual(TEXT("Mid-loop CardUsage failure exact error"),
		Result.CompletionResult.ErrorCode,
		EMatchPlayCurrentAttackCompletionErrorCode
			::OrdinaryCardUsageConsumptionFailed);
	TestStateUnchanged(
		*this,
		TEXT("Mid-loop CardUsage failure"),
		State,
		Result.AfterState);

	State = MakeState();
	AddLegalRunner(State);
	State.RuntimeState.PlayerAState.UsedAttackCount =
		State.RuntimeState.PlayerAState.TotalAttackCount;
	Result = FMatchPlayRunnerDecline::Decline(
		State,
		MakeDeclineRequest());
	TestEqual(TEXT("Exhausted opportunity exact error"),
		Result.CompletionResult.ErrorCode,
		EMatchPlayCurrentAttackCompletionErrorCode
			::InvalidOpportunityState);
	TestStateUnchanged(
		*this,
		TEXT("Exhausted opportunity"),
		State,
		Result.AfterState);

	State = MakeState();
	AddLegalRunner(State);
	State.RuntimeState.PlayerBState.TotalAttackCount = -1;
	Result = FMatchPlayRunnerDecline::Decline(
		State,
		MakeDeclineRequest());
	TestEqual(TEXT("Corrupt opportunity exact error"),
		Result.CompletionResult.ErrorCode,
		EMatchPlayCurrentAttackCompletionErrorCode
			::InvalidOpportunityState);
	TestStateUnchanged(
		*this,
		TEXT("Corrupt opportunity"),
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
	"Completion.AttackerSideGoalkeeperAtomicFailure")

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
	TestFalse(TEXT("Attacker-side GK never completes"),
		Result.bSuccess);
	TestFalse(TEXT("Attacker-side GK is not a legal Runner"),
		Result.RunnerAvailabilityResult.bCanSelectAnyRunner);
	TestEqual(TEXT("Attacker-side GK exact Completion error"),
		Result.CompletionResult.ErrorCode,
		EMatchPlayCurrentAttackCompletionErrorCode
			::InvalidGoalkeeperCompletionState);
	TestStateUnchanged(
		*this,
		TEXT("Attacker-side GK"),
		State,
		Result.AfterState);
	return true;
}

RUNNER_NO_SELECTION_NO_GOAL_TEST(
	FRunnerNoGoalPersistentGoalkeeperAtomicFailureTest,
	"Completion.ActiveGoalkeeperRequiresPersistentUsage")

bool FRunnerNoGoalPersistentGoalkeeperAtomicFailureTest::RunTest(
	const FString& Parameters)
{
	using namespace RunnerNoSelectionNoGoalTests;
	FMatchPlayState State = MakeState();
	AddLegalRunner(State);
	AddActiveGoalkeeper(State);
	State.GoalkeeperUsageState.bPlayerBGoalkeeperCardUsed = false;
	const FMatchPlayRunnerDeclineResult Result =
		FMatchPlayRunnerDecline::Decline(
			State,
			MakeDeclineRequest());
	TestFalse(TEXT("Active GK without persistent usage rejects"),
		Result.bSuccess);
	TestEqual(TEXT("Persistent usage exact Completion error"),
		Result.CompletionResult.ErrorCode,
		EMatchPlayCurrentAttackCompletionErrorCode
			::InvalidGoalkeeperCompletionState);
	TestStateUnchanged(
		*this,
		TEXT("Active GK without persistent usage"),
		State,
		Result.AfterState);

	State = MakeState();
	AddLegalRunner(State);
	AddActiveGoalkeeper(State);
	State.CurrentAttack.bCurrentDefenseGoalkeeperActivated = false;
	const FMatchPlayRunnerDeclineResult Mismatch =
		FMatchPlayRunnerDecline::Decline(
			State,
			MakeDeclineRequest());
	TestFalse(TEXT("GK placement/activation mismatch rejects"),
		Mismatch.bSuccess);
	TestStateUnchanged(
		*this,
		TEXT("GK activation mismatch"),
		State,
		Mismatch.AfterState);

	State = MakeState();
	State.GoalkeeperUsageState.bPlayerBGoalkeeperCardUsed = true;
	const FMatchPlayResolveNoLegalRunnerResult Inactive =
		FMatchPlayResolveNoLegalRunner::Resolve(
			State,
			MakeResolveRequest());
	TestTrue(TEXT("Persistent but inactive GK state is legal"),
		Inactive.bSuccess);
	TestTrue(TEXT("Inactive persistent GK usage is preserved"),
		Inactive.AfterState.GoalkeeperUsageState
			.bPlayerBGoalkeeperCardUsed);
	return true;
}

RUNNER_NO_SELECTION_NO_GOAL_TEST(
	FRunnerNoGoalSameCardAcrossSidesTest,
	"Completion.SameCardIdAcrossSidesAllowed")

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
	TestTrue(TEXT("PlayerA shared card Used"),
		IsCardUsed(
			Result.AfterState,
			EInitialTurnOrderPlayer::PlayerA,
			SharedCardId));
	TestTrue(TEXT("PlayerB shared card Used"),
		IsCardUsed(
			Result.AfterState,
			EInitialTurnOrderPlayer::PlayerB,
			SharedCardId));
	return true;
}

RUNNER_NO_SELECTION_NO_GOAL_TEST(
	FRunnerNoGoalNextAttackerAndMatchResultTest,
	"Completion.NextAttackerAndTerminalResults")

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
	TestEqual(TEXT("Opponent attacks next"),
		Result.CompletionResult.NextAttackingPlayer,
		EInitialTurnOrderPlayer::PlayerB);

	State = MakeState();
	AddLegalRunner(State);
	State.RuntimeState.PlayerBState.UsedAttackCount =
		State.RuntimeState.PlayerBState.TotalAttackCount;
	Result = FMatchPlayRunnerDecline::Decline(
		State,
		MakeDeclineRequest());
	TestEqual(TEXT("Current attacker continues"),
		Result.CompletionResult.NextAttackingPlayer,
		EInitialTurnOrderPlayer::PlayerA);

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
		TestTrue(TEXT("Terminal Runner NoGoal succeeds"),
			Result.bSuccess);
		TestTrue(TEXT("Match ended"),
			Result.CompletionResult.bMatchEnded);
		TestEqual(TEXT("Terminal result uses unchanged Score"),
			Result.CompletionResult.MatchResultResolveResult
				.ResultType,
			Case.Get<2>());
		TestEqual(TEXT("No next attacker at Match End"),
			Result.CompletionResult.NextAttackingPlayer,
			EInitialTurnOrderPlayer::None);
	}
	return true;
}

RUNNER_NO_SELECTION_NO_GOAL_TEST(
	FRunnerNoGoalRepeatedCallTest,
	"Completion.SameAndCrossEntryRepeatsAreSafe")

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
	TestEqual(TEXT("Resolve then Resolve has no CurrentAttack"),
		RepeatedResolve.ErrorCode,
		EMatchPlayRunnerNoSelectionNoGoalErrorCode
			::NoCurrentAttack);
	TestStateUnchanged(
		*this,
		TEXT("Resolve then Resolve"),
		Resolved.AfterState,
		RepeatedResolve.AfterState);
	const FMatchPlayRunnerDeclineResult ResolveThenDecline =
		FMatchPlayRunnerDecline::Decline(
			Resolved.AfterState,
			MakeDeclineRequest());
	TestEqual(TEXT("Resolve then Decline has no CurrentAttack"),
		ResolveThenDecline.ErrorCode,
		EMatchPlayRunnerNoSelectionNoGoalErrorCode
			::NoCurrentAttack);
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
	TestEqual(TEXT("Decline then Decline has no CurrentAttack"),
		RepeatedDecline.ErrorCode,
		EMatchPlayRunnerNoSelectionNoGoalErrorCode
			::NoCurrentAttack);
	TestStateUnchanged(
		*this,
		TEXT("Decline then Decline"),
		Declined.AfterState,
		RepeatedDecline.AfterState);
	const FMatchPlayResolveNoLegalRunnerResult DeclineThenResolve =
		FMatchPlayResolveNoLegalRunner::Resolve(
			Declined.AfterState,
			MakeResolveRequest());
	TestEqual(TEXT("Decline then Resolve has no CurrentAttack"),
		DeclineThenResolve.ErrorCode,
		EMatchPlayRunnerNoSelectionNoGoalErrorCode
			::NoCurrentAttack);
	TestStateUnchanged(
		*this,
		TEXT("Decline then Resolve"),
		Declined.AfterState,
		DeclineThenResolve.AfterState);
	return true;
}

RUNNER_NO_SELECTION_NO_GOAL_TEST(
	FRunnerNoGoalCrossFamilyRepeatedCallTest,
	"Completion.MarkerSkillRunnerCrossFamilyRepeatsAreSafe")

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
		const FMatchPlayResolveNoLegalSkillResult SkillResolve =
			FMatchPlayResolveNoLegalSkill::Resolve(
				*RunnerCompleted,
				SkillRules,
				SkillNoGoalFixtures::MakeResolveRequest());
		const FMatchPlaySkillDeclineResult SkillDecline =
			FMatchPlaySkillDecline::Decline(
				*RunnerCompleted,
				SkillRules,
				SkillNoGoalFixtures::MakeDeclineRequest());
		TestEqual(TEXT("Runner then Skill Resolve has no attack"),
			SkillResolve.ErrorCode,
			EMatchPlaySkillNoSelectionNoGoalErrorCode
				::NoCurrentAttack);
		TestEqual(TEXT("Runner then Skill Decline has no attack"),
			SkillDecline.ErrorCode,
			EMatchPlaySkillNoSelectionNoGoalErrorCode
				::NoCurrentAttack);
		TestStateUnchanged(
			*this,
			TEXT("Runner then Skill Resolve"),
			*RunnerCompleted,
			SkillResolve.AfterState);
		TestStateUnchanged(
			*this,
			TEXT("Runner then Skill Decline"),
			*RunnerCompleted,
			SkillDecline.AfterState);

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

	FMatchPlayState SkillState =
		SkillNoGoalFixtures::MakeState(
			EInitialTurnOrderPlayer::PlayerA,
			{});
	const FMatchPlayResolveNoLegalSkillResult SkillCompleted =
		FMatchPlayResolveNoLegalSkill::Resolve(
			SkillState,
			SkillRules,
			SkillNoGoalFixtures::MakeResolveRequest());
	TestTrue(TEXT("Skill NoGoal seed succeeds"),
		SkillCompleted.bSuccess);
	const FMatchPlayResolveNoLegalRunnerResult SkillThenRunnerResolve =
		FMatchPlayResolveNoLegalRunner::Resolve(
			SkillCompleted.AfterState,
			MakeResolveRequest());
	const FMatchPlayRunnerDeclineResult SkillThenRunnerDecline =
		FMatchPlayRunnerDecline::Decline(
			SkillCompleted.AfterState,
			MakeDeclineRequest());
	TestEqual(TEXT("Skill then Runner Resolve has no attack"),
		SkillThenRunnerResolve.ErrorCode,
		EMatchPlayRunnerNoSelectionNoGoalErrorCode
			::NoCurrentAttack);
	TestEqual(TEXT("Skill then Runner Decline has no attack"),
		SkillThenRunnerDecline.ErrorCode,
		EMatchPlayRunnerNoSelectionNoGoalErrorCode
			::NoCurrentAttack);

	FMatchPlayState MarkerState = MarkerFixtures::MakeState();
	MarkerFixtures::RemoveDefenderPlacements(MarkerState);
	const FMatchPlayResolveNoLegalMarkerResult MarkerCompleted =
		FMatchPlayResolveNoLegalMarker::Resolve(
			MarkerState,
			MarkerFixtures::MakeNoLegalRequest());
	TestTrue(TEXT("Marker Goal seed succeeds"),
		MarkerCompleted.bSuccess);
	const FMatchPlayResolveNoLegalRunnerResult MarkerThenRunnerResolve =
		FMatchPlayResolveNoLegalRunner::Resolve(
			MarkerCompleted.AfterState,
			MakeResolveRequest());
	const FMatchPlayRunnerDeclineResult MarkerThenRunnerDecline =
		FMatchPlayRunnerDecline::Decline(
			MarkerCompleted.AfterState,
			MakeDeclineRequest());
	TestEqual(TEXT("Marker then Runner Resolve has no attack"),
		MarkerThenRunnerResolve.ErrorCode,
		EMatchPlayRunnerNoSelectionNoGoalErrorCode
			::NoCurrentAttack);
	TestEqual(TEXT("Marker then Runner Decline has no attack"),
		MarkerThenRunnerDecline.ErrorCode,
		EMatchPlayRunnerNoSelectionNoGoalErrorCode
			::NoCurrentAttack);

	const FMatchPlayResolveNoLegalMarkerResult SkillThenMarker =
		FMatchPlayResolveNoLegalMarker::Resolve(
			SkillCompleted.AfterState,
			MarkerFixtures::MakeNoLegalRequest());
	TestFalse(TEXT("Skill then Marker rejects"),
		SkillThenMarker.bSuccess);
	TestStateUnchanged(
		*this,
		TEXT("Skill then Marker"),
		SkillCompleted.AfterState,
		SkillThenMarker.AfterState);

	const FMatchPlayResolveNoLegalSkillResult MarkerThenSkill =
		FMatchPlayResolveNoLegalSkill::Resolve(
			MarkerCompleted.AfterState,
			SkillRules,
			SkillNoGoalFixtures::MakeResolveRequest());
	TestEqual(TEXT("Marker then Skill has no attack"),
		MarkerThenSkill.ErrorCode,
		EMatchPlaySkillNoSelectionNoGoalErrorCode
			::NoCurrentAttack);
	TestStateUnchanged(
		*this,
		TEXT("Marker then Skill"),
		MarkerCompleted.AfterState,
		MarkerThenSkill.AfterState);
	return true;
}

#undef RUNNER_NO_SELECTION_NO_GOAL_TEST

#endif
