#if WITH_DEV_AUTOMATION_TESTS
#include "MatchPlayCurrentAttackHelperSelectionTestFixtures.h"
#include "MatchPlayCurrentAttackRunnerSelectionAvailability.h"
#include "MatchPlayCurrentAttackSkillSelectionAvailability.h"
#include "MatchPlayCurrentAttackSkillSelectionWriter.h"
#include "Misc/AutomationTest.h"

namespace CrossRunnerZoneTests
{
namespace Fixtures = FMCodex::Tests::MatchPlayCurrentAttackHelperSelection;

FMatchPlayState MakePrepared(const EInitialTurnOrderPlayer Side, const bool bAuthoredAttack)
{
	auto State = Fixtures::MakeState(ESkillRuleType::Cross, Side);
	auto& Cards = Side == EInitialTurnOrderPlayer::PlayerA
		? State.CardSnapshotAuthority.PlayerACardSnapshots.Cards
		: State.CardSnapshotAuthority.PlayerBCardSnapshots.Cards;
	for (auto& Card : Cards)
	{
		if (Card.CardId == Fixtures::CarrierId) Card.SkillIds = {Fixtures::CrossSkillId};
		if (Card.CardId == Fixtures::RunnerId)
			Card.PositionTypes = bAuthoredAttack ? TArray<EPlayerPositionType>{EPlayerPositionType::Attack}
				: TArray<EPlayerPositionType>{EPlayerPositionType::Midfield, EPlayerPositionType::Defense};
	}
	State.CurrentAttack.ActionPoint = 4;
	State.CurrentAttack.SelectionStage = EMatchPlayCurrentAttackSelectionStage::AwaitingSkill;
	auto& P = State.CurrentAttack.ActionPreparation;
	P.SkillId = NAME_None; P.ActionType = ESkillRuleType::None; P.bSkillSelectionDeferred = true;
	return State;
}

FSkillRuleSnapshotSet Rules()
{
	FSkillRuleSnapshot Rule; Rule.SkillId = Fixtures::CrossSkillId; Rule.SkillType = ESkillRuleType::Cross;
	Rule.MinTriggerActionPoint = 4; Rule.MaxTriggerActionPoint = 4;
	FSkillRuleSnapshotSet Result; Result.SkillRules.Add(Rule); return Result;
}

auto RunnerCandidates(const FMatchPlayState& Prepared)
{
	auto State = Prepared;
	State.CurrentAttack.SelectionStage = EMatchPlayCurrentAttackSelectionStage::AwaitingRunner;
	auto& P = State.CurrentAttack.ActionPreparation;
	P.RunnerCardId = NAME_None; P.SkillId = Fixtures::CrossSkillId;
	P.ActionType = ESkillRuleType::Cross; P.bSkillSelectionDeferred = false;
	return FMatchPlayCurrentAttackRunnerSelectionAvailability::Query(
		State, Fixtures::ValidAttackSequence, State.RuntimeState.CurrentAttackingPlayer);
}

auto ChooseCross(const FMatchPlayState& State)
{
	FMatchPlayCurrentAttackSkillSelectionRequest R;
	R.AttackSequence = Fixtures::ValidAttackSequence; R.RequestingSide = State.RuntimeState.CurrentAttackingPlayer;
	R.SkillId = Fixtures::CrossSkillId;
	return FMatchPlayCurrentAttackSkillSelectionWriter::Select(State, Rules(), R);
}

auto ChooseHigh(const FMatchPlayState& State)
{
	FMatchPlayCurrentAttackBranchIntentSelectionRequest R;
	R.AttackSequence = Fixtures::ValidAttackSequence; R.RequestingSide = State.RuntimeState.CurrentAttackingPlayer;
	R.Intent = EMatchPlayElectiveBranchIntent::CrossHigh;
	return FMatchPlayCurrentAttackBranchIntentSelectionWriter::Select(State, R);
}

void MoveOutsideForward(FMatchPlayState& State)
{
	for (auto& Slot : State.DeploymentSlotCatalog.Slots)
		if (Slot.SlotId == TEXT("Slot.HelperRunner"))
			Slot.NeutralSide = State.RuntimeState.CurrentAttackingPlayer == EInitialTurnOrderPlayer::PlayerA
				? EMatchPlayNeutralSlotSide::NearPlayerA : EMatchPlayNeutralSlotSide::NearPlayerB;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCrossRunnerCurrentZonePositive,
	"FMCodex.CoreRules.CrossRunnerZone.NonAForwardAvailabilityAndContinuation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FCrossRunnerCurrentZonePositive::RunTest(const FString&)
{
	using namespace CrossRunnerZoneTests;
	for (const auto Side : {EInitialTurnOrderPlayer::PlayerA, EInitialTurnOrderPlayer::PlayerB})
	{
		const auto State = MakePrepared(Side, false);
		const auto Candidates = RunnerCandidates(State);
		TestTrue(TEXT("Canonical runner query succeeds"), Candidates.bQuerySucceeded);
		const auto* Runner = Candidates.Candidates.FindByPredicate([](const auto& C) { return C.RunnerCardId == Fixtures::RunnerId; });
		if (!TestNotNull(TEXT("Non-A attacking runner is projected in canonical candidates"), Runner)) return false;
		TestTrue(TEXT("M/D in current Forward qualifies"), Runner->LegalityResult.bIsLegal);
		TestEqual(TEXT("Candidate order retained; Carrier first"), Candidates.Candidates[0].RunnerCardId, Fixtures::CarrierId);
		TestEqual(TEXT("Carrier exclusion retained"), Candidates.Candidates[0].LegalityResult.ErrorCode,
			EMatchPlayCurrentAttackRunnerSelectionErrorCode::RunnerMatchesCarrier);
		const auto Available = FMatchPlayCurrentAttackSkillSelectionAvailability::Query(State, Fixtures::ValidAttackSequence, Side, Rules());
		TestTrue(TEXT("TP4 Cross available with prepared M/D Forward runner"), Available.bQuerySucceeded && Available.bCanSelectAnySkill);
		const auto Selected = ChooseCross(State);
		if (!TestTrue(TEXT("Canonical Cross selection succeeds"), Selected.bSuccess)) return false;
		const auto Ready = ChooseHigh(Selected.AfterState);
		TestTrue(TEXT("Branch and Ready validators accept the same M/D runner"), Ready.bSuccess && Ready.ReadyValidationResult.bSuccess);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCrossRunnerCurrentZoneNegative,
	"FMCodex.CoreRules.CrossRunnerZone.AuthoredAOutsideForwardRejected",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FCrossRunnerCurrentZoneNegative::RunTest(const FString&)
{
	using namespace CrossRunnerZoneTests;
	const auto Forward = MakePrepared(EInitialTurnOrderPlayer::PlayerA, true);
	auto Outside = Forward; MoveOutsideForward(Outside);
	const auto Candidates = RunnerCandidates(Outside);
	TestTrue(TEXT("Negative query remains valid"), Candidates.bQuerySucceeded);
	const auto* Runner = Candidates.Candidates.FindByPredicate([](const auto& C) { return C.RunnerCardId == Fixtures::RunnerId; });
	if (!TestNotNull(TEXT("Authored-A retained as rejected candidate"), Runner)) return false;
	TestEqual(TEXT("Authored A cannot override current zone"), Runner->LegalityResult.ErrorCode,
		EMatchPlayCurrentAttackRunnerSelectionErrorCode::RunnerNotInAttackingForwardArea);
	TestFalse(TEXT("No legal Cross runner remains"), Candidates.bCanSelectAnyRunner);
	const auto Available = FMatchPlayCurrentAttackSkillSelectionAvailability::Query(
		Outside, Fixtures::ValidAttackSequence, EInitialTurnOrderPlayer::PlayerA, Rules());
	TestTrue(TEXT("Availability query succeeds but Cross is unavailable"), Available.bQuerySucceeded && !Available.bCanSelectAnySkill);
	TestFalse(TEXT("Cross submission is rejected as well"), ChooseCross(Outside).bSuccess);
	const auto Selected = ChooseCross(Forward);
	if (!TestTrue(TEXT("Normal authored-A Forward fixture still valid"), Selected.bSuccess)) return false;
	auto InvalidBranch = Selected.AfterState; MoveOutsideForward(InvalidBranch);
	TestFalse(TEXT("Branch gate rechecks current placement"), ChooseHigh(InvalidBranch).bSuccess);
	const auto Ready = ChooseHigh(Selected.AfterState);
	if (!TestTrue(TEXT("Normal Cross reaches Ready"), Ready.bSuccess)) return false;
	auto InvalidReady = Ready.AfterState; MoveOutsideForward(InvalidReady);
	TestEqual(TEXT("Ready gate also rejects authored A outside Forward"),
		FMatchPlayCurrentAttackReadyForResolutionValidator::Validate(InvalidReady).ErrorCode,
		EMatchPlayCurrentAttackReadyValidationErrorCode::RunnerNotInAttackingForwardArea);
	return true;
}
#endif
