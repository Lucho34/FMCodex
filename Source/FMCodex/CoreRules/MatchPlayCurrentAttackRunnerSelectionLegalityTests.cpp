#include "MatchPlayCurrentAttackRunnerSelectionLegality.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "MatchPlayCurrentAttackRunnerSelectionTestFixtures.h"
#include "Misc/AutomationTest.h"

namespace RunnerLegalityFixtures =
	FMCodex::Tests::MatchPlayCurrentAttackRunnerSelection;

#define RUNNER_LEGALITY_TEST(TestClass, TestName) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
		TestClass, \
		"FMCodex.CoreRules.MatchPlayCurrentAttackRunnerSelection.Legality." TestName, \
		EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

RUNNER_LEGALITY_TEST(FRunnerLegalitySkillRulesTest, "SkillRules")

bool FRunnerLegalitySkillRulesTest::RunTest(const FString& Parameters)
{
	using E = EMatchPlayCurrentAttackRunnerSelectionErrorCode;
	auto Evaluate = [](const ESkillRuleType Type, const FName Id)
	{
		return FMatchPlayCurrentAttackRunnerSelectionLegalityEvaluator::Evaluate(
			RunnerLegalityFixtures::MakeState(Type),
			RunnerLegalityFixtures::MakeRequest(Id));
	};

	TestTrue(TEXT("PassControl multi-position Midfield legal"),
		Evaluate(ESkillRuleType::PassControl,
			RunnerLegalityFixtures::MidfieldRunnerId).bIsLegal);
	TestEqual(TEXT("PassControl missing Midfield"),
		Evaluate(ESkillRuleType::PassControl,
			RunnerLegalityFixtures::DefenseRunnerId).ErrorCode,
		E::RunnerMissingRequiredPositionType);
	TestTrue(TEXT("PassControl has no forward requirement"),
		Evaluate(ESkillRuleType::PassControl,
			RunnerLegalityFixtures::AttackRunnerId).bIsLegal);

	TestTrue(TEXT("Cross multi-position Attack legal"),
		Evaluate(ESkillRuleType::Cross,
			RunnerLegalityFixtures::AttackRunnerId).bIsLegal);
	TestEqual(TEXT("Cross missing Attack"),
		Evaluate(ESkillRuleType::Cross,
			RunnerLegalityFixtures::MidfieldRunnerId).ErrorCode,
		E::RunnerMissingRequiredPositionType);

	const auto ThroughForward =
		Evaluate(ESkillRuleType::ThroughBall,
			RunnerLegalityFixtures::ForwardRunnerId);
	TestTrue(TEXT("ThroughBall attacker-forward legal"),
		ThroughForward.bIsLegal);
	TestEqual(TEXT("ThroughBall resolved Forward"),
		ThroughForward.RelativeZoneResolveResult.RelativeZone,
		EMatchPlayRelativeDeploymentZone::Forward);
	TestEqual(TEXT("ThroughBall midfield rejected"),
		Evaluate(ESkillRuleType::ThroughBall,
			RunnerLegalityFixtures::MidfieldRunnerId).ErrorCode,
		E::RunnerNotInAttackingForwardArea);
	TestTrue(TEXT("ThroughBall needs no Attack PositionType"),
		ThroughForward.ResolvedRunnerSnapshot.PositionTypes.Contains(
			EPlayerPositionType::Midfield));

	FMatchPlayState PlayerBState =
		RunnerLegalityFixtures::MakeState(
			ESkillRuleType::ThroughBall);
	PlayerBState.RuntimeState.CurrentAttackingPlayer =
		EInitialTurnOrderPlayer::PlayerB;
	for (FMatchPlayDeploymentPlacement& Placement :
		PlayerBState.CurrentAttack.DeploymentPlacements)
	{
		Placement.PlayerSide =
			Placement.PlayerSide == EInitialTurnOrderPlayer::PlayerA
				? EInitialTurnOrderPlayer::PlayerB
				: EInitialTurnOrderPlayer::PlayerA;
	}
	Swap(
		PlayerBState.CardSnapshotAuthority.PlayerACardSnapshots,
		PlayerBState.CardSnapshotAuthority.PlayerBCardSnapshots);
	PlayerBState.CurrentAttack.ActionPreparation.CarrierCardId =
		RunnerLegalityFixtures::CarrierId;
	PlayerBState.CurrentAttack.ActionPreparation.MarkerCardId =
		RunnerLegalityFixtures::MarkerId;
	auto PlayerBRequest =
		RunnerLegalityFixtures::MakeRequest(
			RunnerLegalityFixtures::MidfieldRunnerId);
	PlayerBRequest.RequestingSide =
		EInitialTurnOrderPlayer::PlayerB;
	const auto PlayerBForward =
		FMatchPlayCurrentAttackRunnerSelectionLegalityEvaluator::Evaluate(
			PlayerBState, PlayerBRequest);
	TestTrue(TEXT("PlayerB attacker perspective resolves own Forward"),
		PlayerBForward.bIsLegal);
	TestEqual(TEXT("PlayerB relative zone is Forward"),
		PlayerBForward.RelativeZoneResolveResult.RelativeZone,
		EMatchPlayRelativeDeploymentZone::Forward);
	return true;
}

RUNNER_LEGALITY_TEST(FRunnerLegalityCommonFailuresTest, "CommonFailures")

bool FRunnerLegalityCommonFailuresTest::RunTest(
	const FString& Parameters)
{
	using E = EMatchPlayCurrentAttackRunnerSelectionErrorCode;
	auto Expect = [this](
		const TCHAR* Label,
		FMatchPlayState State,
		FMatchPlayCurrentAttackRunnerSelectionRequest Request,
		const E Expected)
	{
		const FMatchPlayState Original = State;
		const auto First =
			FMatchPlayCurrentAttackRunnerSelectionLegalityEvaluator::Evaluate(
				State, Request);
		const auto Second =
			FMatchPlayCurrentAttackRunnerSelectionLegalityEvaluator::Evaluate(
				State, Request);
		TestFalse(Label, First.bIsLegal);
		TestEqual(Label, First.ErrorCode, Expected);
		TestEqual(TEXT("Repeated error deterministic"),
			First.ErrorCode, Second.ErrorCode);
		TestTrue(TEXT("Input unchanged"),
			RunnerLegalityFixtures::AreStatesEqual(State, Original));
	};

	Expect(TEXT("Empty id"), RunnerLegalityFixtures::MakeState(),
		RunnerLegalityFixtures::MakeRequest(NAME_None),
		E::InvalidRunnerCardId);
	Expect(TEXT("Not deployed"), RunnerLegalityFixtures::MakeState(),
		RunnerLegalityFixtures::MakeRequest(
			RunnerLegalityFixtures::MissingRunnerId),
		E::RunnerNotDeployed);
	Expect(TEXT("Carrier conflict Pass"), RunnerLegalityFixtures::MakeState(),
		RunnerLegalityFixtures::MakeRequest(
			RunnerLegalityFixtures::CarrierId),
		E::RunnerMatchesCarrier);
	Expect(TEXT("Carrier conflict Cross"),
		RunnerLegalityFixtures::MakeState(ESkillRuleType::Cross),
		RunnerLegalityFixtures::MakeRequest(
			RunnerLegalityFixtures::CarrierId),
		E::RunnerMatchesCarrier);
	Expect(TEXT("Carrier conflict ThroughBall"),
		RunnerLegalityFixtures::MakeState(ESkillRuleType::ThroughBall),
		RunnerLegalityFixtures::MakeRequest(
			RunnerLegalityFixtures::CarrierId),
		E::RunnerMatchesCarrier);
	Expect(TEXT("Goalkeeper"), RunnerLegalityFixtures::MakeState(),
		RunnerLegalityFixtures::MakeRequest(
			RunnerLegalityFixtures::GoalkeeperId),
		E::RunnerIsGoalkeeper);

	FMatchPlayState MissingSnapshot = RunnerLegalityFixtures::MakeState();
	MissingSnapshot.CardSnapshotAuthority.PlayerACardSnapshots.Cards
		.RemoveAt(1);
	Expect(TEXT("Missing snapshot"), MissingSnapshot,
		RunnerLegalityFixtures::MakeRequest(
			RunnerLegalityFixtures::MidfieldRunnerId),
		E::RunnerSnapshotQueryFailed);

	FMatchPlayState WrongSide = RunnerLegalityFixtures::MakeState();
	WrongSide.CurrentAttack.DeploymentPlacements[2].CardId =
		TEXT("PlayerB.Only");
	WrongSide.CardSnapshotAuthority.PlayerACardSnapshots.Cards[1].CardId =
		TEXT("PlayerB.Only");
	WrongSide.CurrentAttack.DeploymentPlacements.Add(
		RunnerLegalityFixtures::MakePlacement(
			EInitialTurnOrderPlayer::PlayerB,
			RunnerLegalityFixtures::MissingRunnerId,
			TEXT("Slot.WrongSide")));
	Expect(TEXT("Wrong-side same id cannot fallback"), WrongSide,
		RunnerLegalityFixtures::MakeRequest(
			RunnerLegalityFixtures::MissingRunnerId),
		E::RunnerNotDeployed);

	FMatchPlayState Ambiguous = RunnerLegalityFixtures::MakeState();
	Ambiguous.CurrentAttack.DeploymentPlacements.Add(
		RunnerLegalityFixtures::MakePlacement(
			EInitialTurnOrderPlayer::PlayerA,
			RunnerLegalityFixtures::MidfieldRunnerId,
			TEXT("Slot.Ambiguous")));
	Expect(TEXT("Ambiguous candidate is blocked by global duplicate authority"),
		Ambiguous,
		RunnerLegalityFixtures::MakeRequest(
			RunnerLegalityFixtures::MidfieldRunnerId),
		E::DuplicateDeploymentCard);
	return true;
}

#undef RUNNER_LEGALITY_TEST

#endif
