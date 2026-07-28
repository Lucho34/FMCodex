#include "MatchPlayCurrentAttackRunnerSelectionGlobalContextQuery.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "MatchPlayCurrentAttackRunnerSelectionTestFixtures.h"
#include "Misc/AutomationTest.h"

namespace RunnerFixtures =
	FMCodex::Tests::MatchPlayCurrentAttackRunnerSelection;

#define RUNNER_GLOBAL_TEST(TestClass, TestName) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
		TestClass, \
		"FMCodex.CoreRules.MatchPlayCurrentAttackRunnerSelection.GlobalContext." TestName, \
		EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

RUNNER_GLOBAL_TEST(FRunnerGlobalValidActionsTest, "ValidActions")

bool FRunnerGlobalValidActionsTest::RunTest(const FString& Parameters)
{
	for (const ESkillRuleType ActionType : {
		ESkillRuleType::PassControl,
		ESkillRuleType::Cross,
		ESkillRuleType::ThroughBall})
	{
		FMatchPlayState State = RunnerFixtures::MakeState(ActionType);
		if (ActionType != ESkillRuleType::ThroughBall)
		{
			State.DeploymentSlotCatalog = {};
		}
		const FMatchPlayState Original = State;
		const auto First =
			FMatchPlayCurrentAttackRunnerSelectionGlobalContextQuery::Query(
				State,
				RunnerFixtures::ValidAttackSequence,
				EInitialTurnOrderPlayer::PlayerA);
		const auto Second =
			FMatchPlayCurrentAttackRunnerSelectionGlobalContextQuery::Query(
				State,
				RunnerFixtures::ValidAttackSequence,
				EInitialTurnOrderPlayer::PlayerA);
		TestTrue(TEXT("Valid action succeeds"), First.bSuccess);
		TestEqual(TEXT("Attacker placements retain count"),
			First.AttackingPlayerPlacements.Num(), 6);
		TestTrue(TEXT("Repeated result deterministic"),
			RunnerFixtures::AreGlobalResultsEqual(First, Second));
		TestTrue(TEXT("Input unchanged"),
			RunnerFixtures::AreStatesEqual(State, Original));
	}
	return true;
}

RUNNER_GLOBAL_TEST(FRunnerGlobalLifecycleFailuresTest, "LifecycleFailures")

bool FRunnerGlobalLifecycleFailuresTest::RunTest(
	const FString& Parameters)
{
	using E = EMatchPlayCurrentAttackRunnerSelectionErrorCode;
	auto Expect = [this](
		const TCHAR* Label,
		FMatchPlayState State,
		const int64 Sequence,
		const EInitialTurnOrderPlayer Side,
		const E Expected)
	{
		const FMatchPlayState Original = State;
		const auto Result =
			FMatchPlayCurrentAttackRunnerSelectionGlobalContextQuery::Query(
				State, Sequence, Side);
		TestFalse(Label, Result.bSuccess);
		TestEqual(Label, Result.ErrorCode, Expected);
		TestTrue(TEXT("Failure is read-only"),
			RunnerFixtures::AreStatesEqual(State, Original));
	};

	FMatchPlayState NoAttack = RunnerFixtures::MakeState();
	NoAttack.bHasCurrentAttack = false;
	Expect(TEXT("No attack"), NoAttack,
		RunnerFixtures::ValidAttackSequence,
		EInitialTurnOrderPlayer::PlayerA, E::NoCurrentAttack);
	Expect(TEXT("Stale sequence"), RunnerFixtures::MakeState(),
		RunnerFixtures::ValidAttackSequence + 1,
		EInitialTurnOrderPlayer::PlayerA, E::AttackSequenceMismatch);

	FMatchPlayState WrongPhase = RunnerFixtures::MakeState();
	WrongPhase.CurrentAttack.Phase =
		EMatchPlayCurrentAttackPhase::Deployment;
	Expect(TEXT("Wrong phase"), WrongPhase,
		RunnerFixtures::ValidAttackSequence,
		EInitialTurnOrderPlayer::PlayerA,
		E::CurrentAttackNotInResolution);

	FMatchPlayState WrongStage = RunnerFixtures::MakeState();
	WrongStage.CurrentAttack.SelectionStage =
		EMatchPlayCurrentAttackSelectionStage::AwaitingHelper;
	WrongStage.CurrentAttack.ActionPreparation.RunnerCardId =
		RunnerFixtures::MidfieldRunnerId;
	Expect(TEXT("Wrong stage"), WrongStage,
		RunnerFixtures::ValidAttackSequence,
		EInitialTurnOrderPlayer::PlayerA, E::WrongSelectionStage);
	Expect(TEXT("Invalid requester"), RunnerFixtures::MakeState(),
		RunnerFixtures::ValidAttackSequence,
		EInitialTurnOrderPlayer::None, E::InvalidRequestingSide);
	Expect(TEXT("Wrong requester"), RunnerFixtures::MakeState(),
		RunnerFixtures::ValidAttackSequence,
		EInitialTurnOrderPlayer::PlayerB,
		E::RequestingSideIsNotCurrentAttacker);
	return true;
}

RUNNER_GLOBAL_TEST(FRunnerGlobalAuthorityFailuresTest, "AuthorityFailures")

bool FRunnerGlobalAuthorityFailuresTest::RunTest(
	const FString& Parameters)
{
	using E = EMatchPlayCurrentAttackRunnerSelectionErrorCode;
	auto Expect = [this](const TCHAR* Label, FMatchPlayState State, E Expected)
	{
		const auto Result =
			FMatchPlayCurrentAttackRunnerSelectionGlobalContextQuery::Query(
				State, RunnerFixtures::ValidAttackSequence,
				EInitialTurnOrderPlayer::PlayerA);
		TestFalse(Label, Result.bSuccess);
		TestEqual(Label, Result.ErrorCode, Expected);
	};

	FMatchPlayState MissingCarrier = RunnerFixtures::MakeState();
	MissingCarrier.CurrentAttack.ActionPreparation.CarrierCardId = NAME_None;
	Expect(TEXT("Missing carrier canonical"), MissingCarrier,
		E::InvalidSelectionState);
	FMatchPlayState MissingMarker = RunnerFixtures::MakeState();
	MissingMarker.CurrentAttack.ActionPreparation.MarkerCardId = NAME_None;
	Expect(TEXT("Missing marker canonical"), MissingMarker,
		E::InvalidSelectionState);
	FMatchPlayState MissingSkill = RunnerFixtures::MakeState();
	MissingSkill.CurrentAttack.ActionPreparation.SkillId = NAME_None;
	Expect(TEXT("Missing skill canonical"), MissingSkill,
		E::InvalidSelectionState);
	FMatchPlayState InvalidType = RunnerFixtures::MakeState();
	InvalidType.CurrentAttack.ActionPreparation.ActionType =
		ESkillRuleType::LongShot;
	Expect(TEXT("LongShot mismatch"), InvalidType, E::InvalidSelectionState);
	InvalidType.CurrentAttack.ActionPreparation.ActionType =
		ESkillRuleType::CutInsideShot;
	Expect(TEXT("CutInside mismatch"), InvalidType, E::InvalidSelectionState);
	FMatchPlayState Prefilled = RunnerFixtures::MakeState();
	Prefilled.CurrentAttack.ActionPreparation.RunnerCardId =
		RunnerFixtures::MidfieldRunnerId;
	Expect(TEXT("Prefilled runner"), Prefilled, E::InvalidSelectionState);

	FMatchPlayState DuplicateCard = RunnerFixtures::MakeState();
	DuplicateCard.CurrentAttack.DeploymentPlacements.Add(
		RunnerFixtures::MakePlacement(
			EInitialTurnOrderPlayer::PlayerA,
			RunnerFixtures::MidfieldRunnerId,
			TEXT("Slot.DuplicateCard")));
	Expect(TEXT("Duplicate card"), DuplicateCard,
		E::DuplicateDeploymentCard);

	FMatchPlayState DuplicateSlot = RunnerFixtures::MakeState();
	DuplicateSlot.CurrentAttack.DeploymentPlacements[2].SlotId =
		DuplicateSlot.CurrentAttack.DeploymentPlacements[0].SlotId;
	Expect(TEXT("Duplicate slot"), DuplicateSlot,
		E::DuplicateDeploymentSlot);

	FMatchPlayState InvalidPlacement = RunnerFixtures::MakeState();
	InvalidPlacement.CurrentAttack.DeploymentPlacements[2].SlotId =
		NAME_None;
	Expect(TEXT("Invalid placement"), InvalidPlacement,
		E::InvalidDeploymentPlacement);

	FMatchPlayState InvalidSnapshots = RunnerFixtures::MakeState();
	const FPlayerCardRuleSnapshot DuplicateSnapshot =
		InvalidSnapshots.CardSnapshotAuthority
			.PlayerACardSnapshots.Cards[0];
	InvalidSnapshots.CardSnapshotAuthority.PlayerACardSnapshots.Cards.Add(
		DuplicateSnapshot);
	Expect(TEXT("Invalid snapshot set"), InvalidSnapshots,
		E::InvalidAttackingSnapshotSet);

	FMatchPlayState InvalidCatalog =
		RunnerFixtures::MakeState(ESkillRuleType::ThroughBall);
	InvalidCatalog.DeploymentSlotCatalog.Slots[0].SlotId = NAME_None;
	Expect(TEXT("ThroughBall invalid catalog"), InvalidCatalog,
		E::InvalidSlotCatalog);
	return true;
}

#undef RUNNER_GLOBAL_TEST

#endif
