#include "MatchPlayCurrentAttackHelperSelectionGlobalContextQuery.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "MatchPlayCurrentAttackHelperSelectionTestFixtures.h"
#include "Misc/AutomationTest.h"

namespace HelperFixtures =
	FMCodex::Tests::MatchPlayCurrentAttackHelperSelection;

namespace HelperGlobalTests
{
	bool ArePlacementsEqual(
		const TArray<FMatchPlayDeploymentPlacement>& Left,
		const TArray<FMatchPlayDeploymentPlacement>& Right)
	{
		if (Left.Num() != Right.Num())
		{
			return false;
		}
		for (int32 Index = 0; Index < Left.Num(); ++Index)
		{
			if (!FMatchPlayDeploymentPlacement::StaticStruct()
				->CompareScriptStruct(
					&Left[Index],
					&Right[Index],
					0))
			{
				return false;
			}
		}
		return true;
	}

	bool AreResultsEqual(
		const FMatchPlayCurrentAttackHelperSelectionGlobalContextResult&
			Left,
		const FMatchPlayCurrentAttackHelperSelectionGlobalContextResult&
			Right)
	{
		return Left.bSuccess == Right.bSuccess
			&& Left.RequestedAttackSequence
				== Right.RequestedAttackSequence
			&& Left.AuthoritativeAttackSequence
				== Right.AuthoritativeAttackSequence
			&& Left.RequestingSide == Right.RequestingSide
			&& Left.CurrentAttackingPlayer
				== Right.CurrentAttackingPlayer
			&& Left.CurrentDefendingPlayer
				== Right.CurrentDefendingPlayer
			&& FMatchPlayCurrentAttackSelectionStateValidationResult
				::StaticStruct()->CompareScriptStruct(
					&Left.SelectionStateValidationResult,
					&Right.SelectionStateValidationResult,
					0)
			&& Left.FrozenCarrierCardId
				== Right.FrozenCarrierCardId
			&& Left.FrozenMarkerCardId
				== Right.FrozenMarkerCardId
			&& Left.FrozenSkillId == Right.FrozenSkillId
			&& Left.FrozenActionType == Right.FrozenActionType
			&& Left.FrozenRunnerCardId
				== Right.FrozenRunnerCardId
			&& FMatchPlaySkillParticipantRequirementResult
				::StaticStruct()->CompareScriptStruct(
					&Left.ParticipantRequirementResult,
					&Right.ParticipantRequirementResult,
					0)
			&& Left.MatchingFrozenCarrierPlacementCount
				== Right.MatchingFrozenCarrierPlacementCount
			&& Left.MatchingFrozenMarkerPlacementCount
				== Right.MatchingFrozenMarkerPlacementCount
			&& Left.MatchingFrozenRunnerPlacementCount
				== Right.MatchingFrozenRunnerPlacementCount
			&& FMatchPlayDeploymentPlacement::StaticStruct()
				->CompareScriptStruct(
					&Left.FrozenCarrierPlacement,
					&Right.FrozenCarrierPlacement,
					0)
			&& FMatchPlayDeploymentPlacement::StaticStruct()
				->CompareScriptStruct(
					&Left.FrozenMarkerPlacement,
					&Right.FrozenMarkerPlacement,
					0)
			&& FMatchPlayDeploymentPlacement::StaticStruct()
				->CompareScriptStruct(
					&Left.FrozenRunnerPlacement,
					&Right.FrozenRunnerPlacement,
					0)
			&& ArePlacementsEqual(
				Left.DefendingPlayerPlacements,
				Right.DefendingPlayerPlacements)
			&& Left.AttackingSnapshotSetValidationResult.bSuccess
				== Right.AttackingSnapshotSetValidationResult.bSuccess
			&& Left.AttackingSnapshotSetValidationResult.ErrorCode
				== Right.AttackingSnapshotSetValidationResult.ErrorCode
			&& Left.AttackingSnapshotSetValidationResult.ErrorMessage
				== Right.AttackingSnapshotSetValidationResult.ErrorMessage
			&& Left.DefendingSnapshotSetValidationResult.bSuccess
				== Right.DefendingSnapshotSetValidationResult.bSuccess
			&& Left.DefendingSnapshotSetValidationResult.ErrorCode
				== Right.DefendingSnapshotSetValidationResult.ErrorCode
			&& Left.DefendingSnapshotSetValidationResult.ErrorMessage
				== Right.DefendingSnapshotSetValidationResult.ErrorMessage
			&& Left.ErrorCode == Right.ErrorCode
			&& Left.ErrorMessage == Right.ErrorMessage;
	}
}

#define HELPER_GLOBAL_TEST(TestClass, TestName) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
		TestClass, \
		"FMCodex.CoreRules.MatchPlayCurrentAttackHelperSelection.GlobalContext." TestName, \
		EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

HELPER_GLOBAL_TEST(FHelperGlobalValidTest, "ValidActionsAndSides")

bool FHelperGlobalValidTest::RunTest(const FString& Parameters)
{
	for (const ESkillRuleType Type : {
		ESkillRuleType::PassControl,
		ESkillRuleType::Cross,
		ESkillRuleType::ThroughBall})
	{
		for (const EInitialTurnOrderPlayer Attacker : {
			EInitialTurnOrderPlayer::PlayerA,
			EInitialTurnOrderPlayer::PlayerB})
		{
			const FMatchPlayState State =
				HelperFixtures::MakeState(Type, Attacker);
			const FMatchPlayState Original = State;
			const auto Result =
				FMatchPlayCurrentAttackHelperSelectionGlobalContextQuery
					::Query(
						State,
						HelperFixtures::ValidAttackSequence,
						HelperFixtures::GetDefender(Attacker));
			const auto Repeated =
				FMatchPlayCurrentAttackHelperSelectionGlobalContextQuery
					::Query(
						State,
						HelperFixtures::ValidAttackSequence,
						HelperFixtures::GetDefender(Attacker));
			TestTrue(TEXT("Global succeeds"), Result.bSuccess);
			TestEqual(TEXT("Four defender placements"),
				Result.DefendingPlayerPlacements.Num(), 4);
			TestEqual(TEXT("Frozen Runner"),
				Result.FrozenRunnerCardId, HelperFixtures::RunnerId);
			TestTrue(TEXT("Global query read-only"),
				HelperFixtures::AreStatesEqual(State, Original));
			TestTrue(TEXT("Complete Global result deterministic"),
				HelperGlobalTests::AreResultsEqual(
					Result,
					Repeated));
		}
	}
	return true;
}

HELPER_GLOBAL_TEST(FHelperGlobalLifecycleTest, "LifecycleFailures")

bool FHelperGlobalLifecycleTest::RunTest(const FString& Parameters)
{
	using E = EMatchPlayCurrentAttackHelperSelectionErrorCode;
	auto Expect = [this](
		const TCHAR* Label,
		FMatchPlayState State,
		const int64 Sequence,
		const EInitialTurnOrderPlayer Side,
		const E Expected)
	{
		const FMatchPlayState Original = State;
		const auto Result =
			FMatchPlayCurrentAttackHelperSelectionGlobalContextQuery
				::Query(State, Sequence, Side);
		TestFalse(Label, Result.bSuccess);
		TestEqual(Label, Result.ErrorCode, Expected);
		TestTrue(TEXT("Failure read-only"),
			HelperFixtures::AreStatesEqual(State, Original));
	};

	FMatchPlayState NoAttack = HelperFixtures::MakeState();
	NoAttack.bHasCurrentAttack = false;
	Expect(TEXT("No attack"), NoAttack,
		HelperFixtures::ValidAttackSequence,
		EInitialTurnOrderPlayer::PlayerB, E::NoCurrentAttack);
	Expect(TEXT("Stale"), HelperFixtures::MakeState(),
		HelperFixtures::ValidAttackSequence + 1,
		EInitialTurnOrderPlayer::PlayerB, E::AttackSequenceMismatch);

	FMatchPlayState Phase = HelperFixtures::MakeState();
	Phase.CurrentAttack.Phase = EMatchPlayCurrentAttackPhase::Deployment;
	Expect(TEXT("Wrong phase"), Phase,
		HelperFixtures::ValidAttackSequence,
		EInitialTurnOrderPlayer::PlayerB,
		E::CurrentAttackNotInResolution);

	FMatchPlayState Stage = HelperFixtures::MakeState();
	Stage.CurrentAttack.SelectionStage =
		EMatchPlayCurrentAttackSelectionStage::AwaitingRunner;
	Stage.CurrentAttack.ActionPreparation.RunnerCardId = NAME_None;
	Expect(TEXT("Wrong stage"), Stage,
		HelperFixtures::ValidAttackSequence,
		EInitialTurnOrderPlayer::PlayerB, E::WrongSelectionStage);
	Expect(TEXT("Invalid side"), HelperFixtures::MakeState(),
		HelperFixtures::ValidAttackSequence,
		EInitialTurnOrderPlayer::None, E::InvalidRequestingSide);
	Expect(TEXT("Attacker side"), HelperFixtures::MakeState(),
		HelperFixtures::ValidAttackSequence,
		EInitialTurnOrderPlayer::PlayerA,
		E::RequestingSideIsNotCurrentDefender);
	return true;
}

HELPER_GLOBAL_TEST(FHelperGlobalAuthorityTest, "AuthorityAndFirstError")

bool FHelperGlobalAuthorityTest::RunTest(const FString& Parameters)
{
	using E = EMatchPlayCurrentAttackHelperSelectionErrorCode;
	auto Expect = [this](const TCHAR* Label, FMatchPlayState State, E Expected)
	{
		const auto Result =
			FMatchPlayCurrentAttackHelperSelectionGlobalContextQuery
				::Query(
					State,
					HelperFixtures::ValidAttackSequence,
					EInitialTurnOrderPlayer::PlayerB);
		TestFalse(Label, Result.bSuccess);
		TestEqual(Label, Result.ErrorCode, Expected);
	};

	FMatchPlayState InvalidPlacement = HelperFixtures::MakeState();
	InvalidPlacement.CurrentAttack.DeploymentPlacements[0].SlotId =
		NAME_None;
	Expect(TEXT("Invalid placement"), InvalidPlacement,
		E::InvalidDeploymentPlacement);

	FMatchPlayState DuplicateCard = HelperFixtures::MakeState();
	DuplicateCard.CurrentAttack.DeploymentPlacements.Add(
		HelperFixtures::MakePlacement(
			EInitialTurnOrderPlayer::PlayerB,
			HelperFixtures::HelperId,
			TEXT("Slot.HelperDuplicate")));
	Expect(TEXT("Duplicate card"), DuplicateCard,
		E::DuplicateDeploymentCard);

	FMatchPlayState DuplicateSlot = HelperFixtures::MakeState();
	DuplicateSlot.CurrentAttack.DeploymentPlacements[3].SlotId =
		DuplicateSlot.CurrentAttack.DeploymentPlacements[2].SlotId;
	Expect(TEXT("Duplicate slot"), DuplicateSlot,
		E::DuplicateDeploymentSlot);

	FMatchPlayState Compound = HelperFixtures::MakeState();
	const FMatchPlayDeploymentPlacement DuplicatePlacement =
		Compound.CurrentAttack.DeploymentPlacements[3];
	Compound.CurrentAttack.DeploymentPlacements.Add(
		DuplicatePlacement);
	Expect(TEXT("Card precedes slot and placement"), Compound,
		E::DuplicateDeploymentCard);

	FMatchPlayState MissingRunner = HelperFixtures::MakeState();
	MissingRunner.CurrentAttack.DeploymentPlacements.RemoveAll(
		[](const FMatchPlayDeploymentPlacement& Placement)
		{
			return Placement.CardId == HelperFixtures::RunnerId;
		});
	Expect(TEXT("Missing frozen Runner"), MissingRunner,
		E::FrozenRunnerNotDeployed);

	FMatchPlayState InvalidSnapshots = HelperFixtures::MakeState();
	const FPlayerCardRuleSnapshot DuplicateSnapshot =
		InvalidSnapshots.CardSnapshotAuthority
			.PlayerBCardSnapshots.Cards[0];
	InvalidSnapshots.CardSnapshotAuthority
		.PlayerBCardSnapshots.Cards.Add(
			DuplicateSnapshot);
	Expect(TEXT("Invalid defender snapshots"), InvalidSnapshots,
		E::InvalidDefendingSnapshotSet);
	return true;
}

#undef HELPER_GLOBAL_TEST

#endif
