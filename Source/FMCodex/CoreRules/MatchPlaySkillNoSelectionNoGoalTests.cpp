#include "MatchPlaySkillNoSelectionNoGoal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "MatchPlaySkillNoSelectionNoGoalTestFixtures.h"
#include "Misc/AutomationTest.h"

namespace SkillNoSelectionNoGoalTests
{
	using namespace
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
}

#define SKILL_NO_SELECTION_NO_GOAL_TEST(TestClass, TestName) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
		TestClass, \
		"FMCodex.CoreRules.MatchPlaySkillNoSelectionNoGoal." TestName, \
		EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

SKILL_NO_SELECTION_NO_GOAL_TEST(
	FSkillNoGoalMutualExclusivityTest,
	"Entries.MutualExclusivityAndBothAttackers")

bool FSkillNoGoalMutualExclusivityTest::RunTest(
	const FString& Parameters)
{
	using namespace SkillNoSelectionNoGoalTests;
	const FSkillRuleSnapshotSet Rules =
		SkillFixtures::MakeRuleSet();

	for (const EInitialTurnOrderPlayer Attacker :
		{EInitialTurnOrderPlayer::PlayerA,
			EInitialTurnOrderPlayer::PlayerB})
	{
		const FMatchPlayState EmptyState =
			MakeState(Attacker, {});
		const FMatchPlayResolveNoLegalSkillResult ResolveResult =
			FMatchPlayResolveNoLegalSkill::Resolve(
				EmptyState,
				Rules,
				MakeResolveRequest());
		TestTrue(TEXT("Zero legal system path succeeds"),
			ResolveResult.bSuccess);
		TestFalse(TEXT("Zero legal has no selectable skill"),
			ResolveResult.SkillAvailabilityResult
				.bCanSelectAnySkill);
		TestEqual(TEXT("System source"),
			ResolveResult.Source,
			EMatchPlaySkillNoSelectionNoGoalSource
				::ResolveNoLegalSkill);
		TestEqual(TEXT("System reason"),
			ResolveResult.Reason,
			EMatchPlaySkillNoSelectionNoGoalReason
				::NoLegalSkill);

		const FMatchPlaySkillDeclineResult EmptyDecline =
			FMatchPlaySkillDecline::Decline(
				EmptyState,
				Rules,
				MakeDeclineRequest(Attacker));
		TestFalse(TEXT("Zero legal decline rejects"),
			EmptyDecline.bSuccess);
		TestEqual(TEXT("Zero legal decline exact error"),
			EmptyDecline.ErrorCode,
			EMatchPlaySkillNoSelectionNoGoalErrorCode
				::NoLegalSkillToDecline);
		TestStateUnchanged(
			*this,
			TEXT("Zero legal decline"),
			EmptyState,
			EmptyDecline.AfterState);

		const FMatchPlayState LegalState =
			MakeState(
				Attacker,
				{SkillFixtures::LongShotSkillId,
					SkillFixtures::CrossSkillId});
		const FMatchPlayResolveNoLegalSkillResult LegalResolve =
			FMatchPlayResolveNoLegalSkill::Resolve(
				LegalState,
				Rules,
				MakeResolveRequest());
		TestFalse(TEXT("Legal skill system path rejects"),
			LegalResolve.bSuccess);
		TestEqual(TEXT("Legal skill exact system error"),
			LegalResolve.ErrorCode,
			EMatchPlaySkillNoSelectionNoGoalErrorCode
				::LegalSkillExists);
		TestStateUnchanged(
			*this,
			TEXT("Legal system path"),
			LegalState,
			LegalResolve.AfterState);

		const FMatchPlaySkillDeclineResult DeclineResult =
			FMatchPlaySkillDecline::Decline(
				LegalState,
				Rules,
				MakeDeclineRequest(Attacker));
		TestTrue(TEXT("Legal skill decline succeeds"),
			DeclineResult.bSuccess);
		TestTrue(TEXT("Decline has selectable skill"),
			DeclineResult.SkillAvailabilityResult
				.bCanSelectAnySkill);
		TestEqual(TEXT("Decline source"),
			DeclineResult.Source,
			EMatchPlaySkillNoSelectionNoGoalSource
				::DeclineSkill);
		TestEqual(TEXT("Decline reason"),
			DeclineResult.Reason,
			EMatchPlaySkillNoSelectionNoGoalReason
				::SkillDeclined);
	}
	return true;
}

SKILL_NO_SELECTION_NO_GOAL_TEST(
	FSkillNoGoalCandidateAndGlobalFailureTest,
	"Entries.CandidateCasesAndGlobalFailures")

bool FSkillNoGoalCandidateAndGlobalFailureTest::RunTest(
	const FString& Parameters)
{
	using namespace SkillNoSelectionNoGoalTests;
	const FSkillRuleSnapshotSet Rules =
		SkillFixtures::MakeRuleSet();

	FMatchPlayState State =
		MakeState(
			EInitialTurnOrderPlayer::PlayerA,
			{SkillFixtures::LongShotSkillId});
	FSkillRuleSnapshotSet RangeRules;
	RangeRules.SkillRules = {
		SkillFixtures::MakeRule(
			SkillFixtures::LongShotSkillId,
			ESkillRuleType::LongShot,
			6,
			8)
	};
	const FMatchPlayResolveNoLegalSkillResult RangeResult =
		FMatchPlayResolveNoLegalSkill::Resolve(
			State,
			RangeRules,
			MakeResolveRequest());
	TestTrue(TEXT("All AP-mismatch candidates resolve"),
		RangeResult.bSuccess);

	State = MakeState(
		EInitialTurnOrderPlayer::PlayerA,
		{SkillFixtures::MissingSkillId});
	const FMatchPlayResolveNoLegalSkillResult AllIllegal =
		FMatchPlayResolveNoLegalSkill::Resolve(
			State,
			Rules,
			MakeResolveRequest());
	TestTrue(TEXT("All illegal candidates resolve"),
		AllIllegal.bSuccess);

	State = MakeState(
		EInitialTurnOrderPlayer::PlayerA,
		{SkillFixtures::MissingSkillId,
			SkillFixtures::CrossSkillId});
	FSkillRuleSnapshotSet MixedIllegalRules;
	MixedIllegalRules.SkillRules = {
		SkillFixtures::MakeRule(
			SkillFixtures::CrossSkillId,
			ESkillRuleType::Cross,
			6,
			8)
	};
	const FMatchPlayResolveNoLegalSkillResult MixedIllegal =
		FMatchPlayResolveNoLegalSkill::Resolve(
			State,
			MixedIllegalRules,
			MakeResolveRequest());
	TestTrue(TEXT("Mixed all-illegal candidates resolve"),
		MixedIllegal.bSuccess);

	for (const int32 InvalidActionPoint : {1, 9})
	{
		State = MakeState(
			EInitialTurnOrderPlayer::PlayerA,
			{});
		State.CurrentAttack.ActionPoint = InvalidActionPoint;
		const FMatchPlayResolveNoLegalSkillResult Result =
			FMatchPlayResolveNoLegalSkill::Resolve(
				State,
				Rules,
				MakeResolveRequest());
		TestFalse(TEXT("Invalid AP never resolves no legal"),
			Result.bSuccess);
		TestEqual(TEXT("Invalid AP remains global failure"),
			Result.ErrorCode,
			EMatchPlaySkillNoSelectionNoGoalErrorCode
				::AvailabilityQueryFailed);
		TestStateUnchanged(
			*this,
			TEXT("Invalid AP"),
			State,
			Result.AfterState);
	}

	FSkillRuleSnapshotSet CorruptRules = Rules;
	CorruptRules.SkillRules[0].SkillId = NAME_None;
	State = MakeState(EInitialTurnOrderPlayer::PlayerA, {});
	const FMatchPlayResolveNoLegalSkillResult CorruptRulesResult =
		FMatchPlayResolveNoLegalSkill::Resolve(
			State,
			CorruptRules,
			MakeResolveRequest());
	TestFalse(TEXT("Corrupt RuleSet never resolves"),
		CorruptRulesResult.bSuccess);
	TestStateUnchanged(
		*this,
		TEXT("Corrupt RuleSet"),
		State,
		CorruptRulesResult.AfterState);
	return true;
}

SKILL_NO_SELECTION_NO_GOAL_TEST(
	FSkillNoGoalRequestValidationTest,
	"Entries.SequenceStageSideAndNoCurrentAttack")

bool FSkillNoGoalRequestValidationTest::RunTest(
	const FString& Parameters)
{
	using namespace SkillNoSelectionNoGoalTests;
	const FSkillRuleSnapshotSet Rules =
		SkillFixtures::MakeRuleSet();
	const FMatchPlayState State = MakeState();

	FMatchPlayResolveNoLegalSkillRequest ResolveRequest =
		MakeResolveRequest();
	++ResolveRequest.AttackSequence;
	const FMatchPlayResolveNoLegalSkillResult StaleResolve =
		FMatchPlayResolveNoLegalSkill::Resolve(
			State,
			Rules,
			ResolveRequest);
	TestFalse(TEXT("Stale system sequence rejects"),
		StaleResolve.bSuccess);
	TestEqual(TEXT("Stale sequence maps exactly"),
		StaleResolve.ErrorCode,
		EMatchPlaySkillNoSelectionNoGoalErrorCode
			::AttackSequenceMismatch);
	TestStateUnchanged(
		*this,
		TEXT("Stale system sequence"),
		State,
		StaleResolve.AfterState);

	FMatchPlaySkillDeclineRequest DeclineRequest =
		MakeDeclineRequest(EInitialTurnOrderPlayer::PlayerB);
	const FMatchPlaySkillDeclineResult WrongSide =
		FMatchPlaySkillDecline::Decline(
			State,
			Rules,
			DeclineRequest);
	TestFalse(TEXT("Wrong side rejects"), WrongSide.bSuccess);
	TestEqual(TEXT("Wrong side maps exactly"),
		WrongSide.ErrorCode,
		EMatchPlaySkillNoSelectionNoGoalErrorCode
			::RequestingSideIsNotCurrentAttacker);
	TestStateUnchanged(
		*this,
		TEXT("Wrong side"),
		State,
		WrongSide.AfterState);

	DeclineRequest.RequestingSide =
		EInitialTurnOrderPlayer::None;
	const FMatchPlaySkillDeclineResult InvalidSide =
		FMatchPlaySkillDecline::Decline(
			State,
			Rules,
			DeclineRequest);
	TestEqual(TEXT("Invalid side maps exactly"),
		InvalidSide.ErrorCode,
		EMatchPlaySkillNoSelectionNoGoalErrorCode
			::InvalidRequestingSide);

	FMatchPlayState WrongStage = State;
	WrongStage.CurrentAttack.SelectionStage =
		EMatchPlayCurrentAttackSelectionStage::AwaitingRunner;
	WrongStage.CurrentAttack.ActionPreparation.SkillId =
		SkillFixtures::CrossSkillId;
	WrongStage.CurrentAttack.ActionPreparation.ActionType =
		ESkillRuleType::Cross;
	const FMatchPlaySkillDeclineResult WrongStageResult =
		FMatchPlaySkillDecline::Decline(
			WrongStage,
			Rules,
			MakeDeclineRequest());
	TestFalse(TEXT("Wrong stage rejects"),
		WrongStageResult.bSuccess);
	TestEqual(TEXT("Wrong stage maps exactly"),
		WrongStageResult.ErrorCode,
		EMatchPlaySkillNoSelectionNoGoalErrorCode
			::WrongSelectionStage);

	FMatchPlayState NoAttack = State;
	NoAttack.bHasCurrentAttack = false;
	NoAttack.CurrentAttack = FMatchPlayCurrentAttackState();
	const FMatchPlayResolveNoLegalSkillResult NoAttackResult =
		FMatchPlayResolveNoLegalSkill::Resolve(
			NoAttack,
			Rules,
			MakeResolveRequest());
	TestEqual(TEXT("No current attack maps exactly"),
		NoAttackResult.ErrorCode,
		EMatchPlaySkillNoSelectionNoGoalErrorCode
			::NoCurrentAttack);
	return true;
}

SKILL_NO_SELECTION_NO_GOAL_TEST(
	FSkillNoGoalLifecycleTest,
	"Completion.ScoreCardUsageGoalkeeperAndClear")

bool FSkillNoGoalLifecycleTest::RunTest(
	const FString& Parameters)
{
	using namespace SkillNoSelectionNoGoalTests;
	FMatchPlayState State = MakeState();
	AddOrdinaryDeployments(State);
	AddActiveGoalkeeper(State);
	const FMatchPlayState Before = State;
	const FMatchPlaySkillDeclineResult Result =
		FMatchPlaySkillDecline::Decline(
			State,
			SkillFixtures::MakeRuleSet(),
			MakeDeclineRequest());

	TestTrue(TEXT("Lifecycle completion succeeds"),
		Result.bSuccess);
	TestEqual(TEXT("PlayerA score unchanged"),
		Result.AfterState.RuntimeState.PlayerAState.Score,
		Before.RuntimeState.PlayerAState.Score);
	TestEqual(TEXT("PlayerB score unchanged"),
		Result.AfterState.RuntimeState.PlayerBState.Score,
		Before.RuntimeState.PlayerBState.Score);
	TestFalse(TEXT("No GoalResolver result"),
		Result.CompletionResult.GoalResolveResult.bSuccess);
	TestEqual(TEXT("Four ordinary cards consumed"),
		Result.CompletionResult.OrdinaryCardUsageResults.Num(),
		4);
	if (Result.CompletionResult.OrdinaryCardUsageResults.Num() == 4)
	{
		TestEqual(TEXT("Carrier first"),
			Result.CompletionResult
				.OrdinaryCardUsageResults[0].CardId,
			SkillFixtures::CarrierId);
		TestEqual(TEXT("Marker second"),
			Result.CompletionResult
				.OrdinaryCardUsageResults[1].CardId,
			SkillFixtures::MarkerId);
		TestEqual(TEXT("Unselected attacker third"),
			Result.CompletionResult
				.OrdinaryCardUsageResults[2].CardId,
			ExtraAttackerCardId);
		TestEqual(TEXT("Unselected defender fourth"),
			Result.CompletionResult
				.OrdinaryCardUsageResults[3].CardId,
			ExtraDefenderCardId);
	}
	TestTrue(TEXT("Carrier Used"),
		IsCardUsed(
			Result.AfterState,
			EInitialTurnOrderPlayer::PlayerA,
			SkillFixtures::CarrierId));
	TestTrue(TEXT("Marker Used"),
		IsCardUsed(
			Result.AfterState,
			EInitialTurnOrderPlayer::PlayerB,
			SkillFixtures::MarkerId));
	TestTrue(TEXT("Unselected attacker Used"),
		IsCardUsed(
			Result.AfterState,
			EInitialTurnOrderPlayer::PlayerA,
			ExtraAttackerCardId));
	TestTrue(TEXT("Unselected defender Used"),
		IsCardUsed(
			Result.AfterState,
			EInitialTurnOrderPlayer::PlayerB,
			ExtraDefenderCardId));
	TestTrue(TEXT("Goalkeeper remains Available"),
		Result.AfterState.CardUsageState.PlayerBCardUsageState
			.AvailableCardIds.Contains(GoalkeeperCardId));
	TestFalse(TEXT("Goalkeeper is not Used"),
		IsCardUsed(
			Result.AfterState,
			EInitialTurnOrderPlayer::PlayerB,
			GoalkeeperCardId));
	TestTrue(TEXT("Persistent goalkeeper usage preserved"),
		Result.AfterState.GoalkeeperUsageState
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
	TestEqual(TEXT("Opportunity consumed once"),
		Result.AfterState.RuntimeState.PlayerAState.UsedAttackCount,
		Before.RuntimeState.PlayerAState.UsedAttackCount + 1);
	TestFalse(TEXT("No automatic Begin"),
		Result.AfterState.bHasCurrentAttack);
	return true;
}

SKILL_NO_SELECTION_NO_GOAL_TEST(
	FSkillNoGoalPlacementAndAtomicityTest,
	"Completion.PlacementDefensesAndAtomicFailures")

bool FSkillNoGoalPlacementAndAtomicityTest::RunTest(
	const FString& Parameters)
{
	using namespace SkillNoSelectionNoGoalTests;
	const FSkillRuleSnapshotSet Rules =
		SkillFixtures::MakeRuleSet();

	FMatchPlayState State = MakeState();
	State.CurrentAttack.DeploymentPlacements[1].SlotId =
		State.CurrentAttack.DeploymentPlacements[0].SlotId;
	FMatchPlaySkillDeclineResult Result =
		FMatchPlaySkillDecline::Decline(
			State,
			Rules,
			MakeDeclineRequest());
	TestFalse(TEXT("Cross-side duplicate slot rejects"),
		Result.bSuccess);
	TestEqual(TEXT("Duplicate slot exact completion error"),
		Result.CompletionResult.ErrorCode,
		EMatchPlayCurrentAttackCompletionErrorCode
			::DuplicateDeploymentSlot);
	TestStateUnchanged(
		*this,
		TEXT("Cross-side duplicate slot"),
		State,
		Result.AfterState);

	State = MakeState();
	AddOrdinaryDeployments(State);
	State.CurrentAttack.DeploymentPlacements[2].SlotId =
		State.CurrentAttack.DeploymentPlacements[0].SlotId;
	Result = FMatchPlaySkillDecline::Decline(
		State,
		Rules,
		MakeDeclineRequest());
	TestFalse(TEXT("Same-side duplicate slot rejects"),
		Result.bSuccess);
	TestStateUnchanged(
		*this,
		TEXT("Same-side duplicate slot"),
		State,
		Result.AfterState);

	State = MakeState();
	AddOrdinaryDeployments(State);
	State.CurrentAttack.DeploymentPlacements.Add(
		SkillFixtures::MakePlacement(
			EInitialTurnOrderPlayer::PlayerA,
			ExtraAttackerCardId,
			TEXT("Slot.DuplicateCard")));
	Result = FMatchPlaySkillDecline::Decline(
		State,
		Rules,
		MakeDeclineRequest());
	TestEqual(TEXT("Same-side duplicate card exact error"),
		Result.CompletionResult.ErrorCode,
		EMatchPlayCurrentAttackCompletionErrorCode
			::DuplicateDeploymentCard);
	TestStateUnchanged(
		*this,
		TEXT("Same-side duplicate card"),
		State,
		Result.AfterState);

	State = MakeState();
	AddOrdinaryDeployments(State);
	State.CardSnapshotAuthority.PlayerBCardSnapshots.Cards
		.RemoveAll(
			[](const FPlayerCardRuleSnapshot& Snapshot)
			{
				return Snapshot.CardId == ExtraDefenderCardId;
			});
	Result = FMatchPlaySkillDecline::Decline(
		State,
		Rules,
		MakeDeclineRequest());
	TestEqual(TEXT("Missing snapshot exact completion error"),
		Result.CompletionResult.ErrorCode,
		EMatchPlayCurrentAttackCompletionErrorCode
			::DeploymentSnapshotQueryFailed);
	TestStateUnchanged(
		*this,
		TEXT("Missing snapshot"),
		State,
		Result.AfterState);

	State = MakeState();
	AddOrdinaryDeployments(State);
	State.CardUsageState.PlayerACardUsageState.AvailableCardIds
		.Remove(ExtraAttackerCardId);
	Result = FMatchPlaySkillDecline::Decline(
		State,
		Rules,
		MakeDeclineRequest());
	TestEqual(TEXT("Mid-loop failure exact completion error"),
		Result.CompletionResult.ErrorCode,
		EMatchPlayCurrentAttackCompletionErrorCode
			::OrdinaryCardUsageConsumptionFailed);
	TestStateUnchanged(
		*this,
		TEXT("Mid-loop failure"),
		State,
		Result.AfterState);

	State = MakeState();
	State.RuntimeState.PlayerAState.UsedAttackCount =
		State.RuntimeState.PlayerAState.TotalAttackCount;
	Result = FMatchPlaySkillDecline::Decline(
		State,
		Rules,
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
	State.RuntimeState.PlayerBState.TotalAttackCount = -1;
	Result = FMatchPlaySkillDecline::Decline(
		State,
		Rules,
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
	State.RuntimeState.PlayerBState.Score = -1;
	Result = FMatchPlaySkillDecline::Decline(
		State,
		Rules,
		MakeDeclineRequest());
	TestEqual(TEXT("Negative score exact error"),
		Result.CompletionResult.ErrorCode,
		EMatchPlayCurrentAttackCompletionErrorCode
			::InvalidScoreState);
	TestStateUnchanged(
		*this,
		TEXT("Negative score"),
		State,
		Result.AfterState);
	return true;
}

SKILL_NO_SELECTION_NO_GOAL_TEST(
	FSkillNoGoalSameCardAcrossSidesTest,
	"Completion.SameCardIdAcrossSidesAllowed")

bool FSkillNoGoalSameCardAcrossSidesTest::RunTest(
	const FString& Parameters)
{
	using namespace SkillNoSelectionNoGoalTests;
	FMatchPlayState State = MakeState();
	const FPlayerCardRuleSnapshot SharedCard =
		SkillFixtures::MakeCard(SkillFixtures::SharedCardId);
	State.CardSnapshotAuthority.PlayerACardSnapshots.Cards.Add(
		SharedCard);
	State.CardSnapshotAuthority.PlayerBCardSnapshots.Cards.Add(
		SharedCard);
	State.CardUsageState.PlayerACardUsageState.AvailableCardIds.Add(
		SkillFixtures::SharedCardId);
	State.CardUsageState.PlayerBCardUsageState.AvailableCardIds.Add(
		SkillFixtures::SharedCardId);
	State.CurrentAttack.DeploymentPlacements.Add(
		SkillFixtures::MakePlacement(
			EInitialTurnOrderPlayer::PlayerA,
			SkillFixtures::SharedCardId,
			TEXT("Slot.SharedA")));
	State.CurrentAttack.DeploymentPlacements.Add(
		SkillFixtures::MakePlacement(
			EInitialTurnOrderPlayer::PlayerB,
			SkillFixtures::SharedCardId,
			TEXT("Slot.SharedB")));
	const FMatchPlaySkillDeclineResult Result =
		FMatchPlaySkillDecline::Decline(
			State,
			SkillFixtures::MakeRuleSet(),
			MakeDeclineRequest());
	TestTrue(TEXT("Same CardId across sides succeeds"),
		Result.bSuccess);
	TestTrue(TEXT("PlayerA shared card Used"),
		IsCardUsed(
			Result.AfterState,
			EInitialTurnOrderPlayer::PlayerA,
			SkillFixtures::SharedCardId));
	TestTrue(TEXT("PlayerB shared card Used"),
		IsCardUsed(
			Result.AfterState,
			EInitialTurnOrderPlayer::PlayerB,
			SkillFixtures::SharedCardId));
	return true;
}

SKILL_NO_SELECTION_NO_GOAL_TEST(
	FSkillNoGoalNextAttackerAndMatchResultTest,
	"Completion.NextAttackerAndTerminalResults")

bool FSkillNoGoalNextAttackerAndMatchResultTest::RunTest(
	const FString& Parameters)
{
	using namespace SkillNoSelectionNoGoalTests;
	const FSkillRuleSnapshotSet Rules =
		SkillFixtures::MakeRuleSet();

	FMatchPlayState State = MakeState();
	FMatchPlaySkillDeclineResult Result =
		FMatchPlaySkillDecline::Decline(
			State,
			Rules,
			MakeDeclineRequest());
	TestEqual(TEXT("Opponent attacks next"),
		Result.CompletionResult.NextAttackingPlayer,
		EInitialTurnOrderPlayer::PlayerB);

	State = MakeState();
	State.RuntimeState.PlayerBState.UsedAttackCount =
		State.RuntimeState.PlayerBState.TotalAttackCount;
	Result = FMatchPlaySkillDecline::Decline(
		State,
		Rules,
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
		State.RuntimeState.PlayerAState.TotalAttackCount = 2;
		State.RuntimeState.PlayerAState.UsedAttackCount = 1;
		State.RuntimeState.PlayerBState.TotalAttackCount = 1;
		State.RuntimeState.PlayerBState.UsedAttackCount = 1;
		State.RuntimeState.PlayerAState.Score = Case.Get<0>();
		State.RuntimeState.PlayerBState.Score = Case.Get<1>();
		Result = FMatchPlaySkillDecline::Decline(
			State,
			Rules,
			MakeDeclineRequest());
		TestTrue(TEXT("Terminal completion succeeds"),
			Result.bSuccess);
		TestTrue(TEXT("Match ended"),
			Result.CompletionResult.bMatchEnded);
		TestEqual(TEXT("Terminal result derived from old score"),
			Result.CompletionResult.MatchResultResolveResult
				.ResultType,
			Case.Get<2>());
		TestEqual(TEXT("No next attacker at match end"),
			Result.CompletionResult.NextAttackingPlayer,
			EInitialTurnOrderPlayer::None);
	}
	return true;
}

SKILL_NO_SELECTION_NO_GOAL_TEST(
	FSkillNoGoalRepeatedCallTest,
	"Completion.SameAndCrossEntryRepeatsAreSafe")

bool FSkillNoGoalRepeatedCallTest::RunTest(
	const FString& Parameters)
{
	using namespace SkillNoSelectionNoGoalTests;
	const FSkillRuleSnapshotSet Rules =
		SkillFixtures::MakeRuleSet();

	const FMatchPlayState EmptyState =
		MakeState(EInitialTurnOrderPlayer::PlayerA, {});
	const FMatchPlayResolveNoLegalSkillResult Resolved =
		FMatchPlayResolveNoLegalSkill::Resolve(
			EmptyState,
			Rules,
			MakeResolveRequest());
	TestTrue(TEXT("Initial Resolve succeeds"), Resolved.bSuccess);
	const FMatchPlayResolveNoLegalSkillResult RepeatedResolve =
		FMatchPlayResolveNoLegalSkill::Resolve(
			Resolved.AfterState,
			Rules,
			MakeResolveRequest());
	TestEqual(TEXT("Repeated Resolve has no current attack"),
		RepeatedResolve.ErrorCode,
		EMatchPlaySkillNoSelectionNoGoalErrorCode
			::NoCurrentAttack);
	TestStateUnchanged(
		*this,
		TEXT("Repeated Resolve"),
		Resolved.AfterState,
		RepeatedResolve.AfterState);
	const FMatchPlaySkillDeclineResult CrossDecline =
		FMatchPlaySkillDecline::Decline(
			Resolved.AfterState,
			Rules,
			MakeDeclineRequest());
	TestEqual(TEXT("Resolve then Decline has no current attack"),
		CrossDecline.ErrorCode,
		EMatchPlaySkillNoSelectionNoGoalErrorCode
			::NoCurrentAttack);
	TestStateUnchanged(
		*this,
		TEXT("Resolve then Decline"),
		Resolved.AfterState,
		CrossDecline.AfterState);

	const FMatchPlayState LegalState = MakeState();
	const FMatchPlaySkillDeclineResult Declined =
		FMatchPlaySkillDecline::Decline(
			LegalState,
			Rules,
			MakeDeclineRequest());
	TestTrue(TEXT("Initial Decline succeeds"), Declined.bSuccess);
	const FMatchPlaySkillDeclineResult RepeatedDecline =
		FMatchPlaySkillDecline::Decline(
			Declined.AfterState,
			Rules,
			MakeDeclineRequest());
	TestEqual(TEXT("Repeated Decline has no current attack"),
		RepeatedDecline.ErrorCode,
		EMatchPlaySkillNoSelectionNoGoalErrorCode
			::NoCurrentAttack);
	const FMatchPlayResolveNoLegalSkillResult CrossResolve =
		FMatchPlayResolveNoLegalSkill::Resolve(
			Declined.AfterState,
			Rules,
			MakeResolveRequest());
	TestEqual(TEXT("Decline then Resolve has no current attack"),
		CrossResolve.ErrorCode,
		EMatchPlaySkillNoSelectionNoGoalErrorCode
			::NoCurrentAttack);
	return true;
}

#undef SKILL_NO_SELECTION_NO_GOAL_TEST

#endif
