#include "MatchPlayOrdinaryDeploymentWriter.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "MatchPlayFinishDeployment.h"
#include "Misc/AutomationTest.h"

#include <type_traits>

namespace FMCodex::Tests::MatchPlayOrdinaryDeploymentWriter
{
	constexpr int64 ValidAttackSequence = 41;
	const FName SharedCardId(TEXT("Shared01"));
	const FName PlayerAOtherCardId(TEXT("PlayerA.Other"));
	const FName PlayerAGoalkeeperCardId(TEXT("PlayerA.GK"));
	const FName PlayerBOtherCardId(TEXT("PlayerB.Other"));
	const FName NearPlayerASlotId(TEXT("Slot.NearPlayerA"));
	const FName OtherNearPlayerASlotId(TEXT("Slot.NearPlayerA.Other"));
	const FName NearPlayerBSlotId(TEXT("Slot.NearPlayerB"));
	const FName OtherNearPlayerBSlotId(TEXT("Slot.NearPlayerB.Other"));

	FPlayerCardRuleSnapshot MakeSnapshot(
		const FName CardId,
		const TArray<EPlayerPositionType>& PositionTypes,
		const bool bIsGoalkeeper = false)
	{
		FPlayerCardRuleSnapshot Snapshot;
		Snapshot.CardId = CardId;
		Snapshot.PositionTypes = PositionTypes;
		Snapshot.bIsGoalkeeper = bIsGoalkeeper;
		Snapshot.bHasGoalkeeperAttributes = bIsGoalkeeper;
		return Snapshot;
	}

	FMatchPlayDeploymentSlotDefinition MakeSlot(
		const FName SlotId,
		const EMatchPlayNeutralSlotSide NeutralSide)
	{
		FMatchPlayDeploymentSlotDefinition Slot;
		Slot.SlotId = SlotId;
		Slot.NeutralSide = NeutralSide;
		return Slot;
	}

	FMatchPlayDeploymentPlacement MakePlacement(
		const EInitialTurnOrderPlayer PlayerSide,
		const FName CardId,
		const FName SlotId)
	{
		FMatchPlayDeploymentPlacement Placement;
		Placement.PlayerSide = PlayerSide;
		Placement.CardId = CardId;
		Placement.SlotId = SlotId;
		return Placement;
	}

	FMatchPlayState MakeState(
		const EInitialTurnOrderPlayer CurrentAttacker =
			EInitialTurnOrderPlayer::PlayerA,
		const EInitialTurnOrderPlayer CurrentLegalSide =
			EInitialTurnOrderPlayer::PlayerA,
		const bool bAttackerFinished = false,
		const bool bDefenderFinished = false)
	{
		FMatchPlayState State;
		State.RuntimeState.bIsInitialized = true;
		State.RuntimeState.CurrentAttackingPlayer = CurrentAttacker;
		State.RuntimeState.FirstPlayer = EInitialTurnOrderPlayer::PlayerA;
		State.RuntimeState.SecondPlayer = EInitialTurnOrderPlayer::PlayerB;
		State.RuntimeState.PlayerAState.TotalAttackCount = 4;
		State.RuntimeState.PlayerAState.UsedAttackCount = 1;
		State.RuntimeState.PlayerAState.Score = 2;
		State.RuntimeState.PlayerBState.TotalAttackCount = 5;
		State.RuntimeState.PlayerBState.UsedAttackCount = 2;
		State.RuntimeState.PlayerBState.Score = 1;

		State.CardUsageState.PlayerACardUsageState.AvailableCardIds = {
			SharedCardId,
			PlayerAOtherCardId,
			PlayerAGoalkeeperCardId
		};
		State.CardUsageState.PlayerBCardUsageState.AvailableCardIds = {
			SharedCardId,
			PlayerBOtherCardId
		};

		State.CardSnapshotAuthority.PlayerACardSnapshots.Cards = {
			MakeSnapshot(
				SharedCardId,
				{ EPlayerPositionType::Attack }),
			MakeSnapshot(
				PlayerAOtherCardId,
				{ EPlayerPositionType::Defense }),
			MakeSnapshot(
				PlayerAGoalkeeperCardId,
				{ EPlayerPositionType::Goalkeeper },
				true)
		};
		State.CardSnapshotAuthority.PlayerBCardSnapshots.Cards = {
			MakeSnapshot(
				SharedCardId,
				{ EPlayerPositionType::Defense }),
			MakeSnapshot(
				PlayerBOtherCardId,
				{ EPlayerPositionType::Attack })
		};

		State.DeploymentSlotCatalog.Slots = {
			MakeSlot(
				NearPlayerASlotId,
				EMatchPlayNeutralSlotSide::NearPlayerA),
			MakeSlot(
				OtherNearPlayerASlotId,
				EMatchPlayNeutralSlotSide::NearPlayerA),
			MakeSlot(
				NearPlayerBSlotId,
				EMatchPlayNeutralSlotSide::NearPlayerB),
			MakeSlot(
				OtherNearPlayerBSlotId,
				EMatchPlayNeutralSlotSide::NearPlayerB)
		};

		State.bHasCurrentAttack = true;
		State.CurrentAttack.Phase =
			EMatchPlayCurrentAttackPhase::Deployment;
		State.CurrentAttack.AttackSequence = ValidAttackSequence;
		State.CurrentAttack.ActionPoint = 6;
		State.CurrentAttack.CurrentLegalDeploymentSide = CurrentLegalSide;
		State.CurrentAttack.bAttackerDeploymentFinished =
			bAttackerFinished;
		State.CurrentAttack.bDefenderDeploymentFinished =
			bDefenderFinished;
		State.CurrentAttack.bCurrentDefenseGoalkeeperActivated = true;
		return State;
	}

	FMatchPlayOrdinaryDeploymentRequest MakeRequest(
		const EInitialTurnOrderPlayer RequestingSide =
			EInitialTurnOrderPlayer::PlayerA,
		const FName CardId = SharedCardId,
		const FName SlotId = NearPlayerASlotId,
		const int64 AttackSequence = ValidAttackSequence)
	{
		FMatchPlayOrdinaryDeploymentRequest Request;
		Request.AttackSequence = AttackSequence;
		Request.RequestingSide = RequestingSide;
		Request.CardId = CardId;
		Request.SlotId = SlotId;
		return Request;
	}

	bool AreStatesEqual(
		const FMatchPlayState& Left,
		const FMatchPlayState& Right)
	{
		return FMatchPlayState::StaticStruct()->CompareScriptStruct(
			&Left,
			&Right,
			0);
	}

	bool TestLegalityFailure(
		FAutomationTestBase& Test,
		const TCHAR* Context,
		const FMatchPlayState& State,
		const FMatchPlayOrdinaryDeploymentRequest& Request,
		const EMatchPlayOrdinaryDeploymentErrorCode ExpectedError)
	{
		const FMatchPlayState OriginalInput = State;
		const FMatchPlayOrdinaryDeploymentWriterResult Result =
			FMatchPlayOrdinaryDeploymentWriter::Deploy(State, Request);
		Test.TestFalse(
			*FString::Printf(TEXT("%s fails"), Context),
			Result.bSuccess);
		Test.TestEqual(
			*FString::Printf(TEXT("%s uses writer legality error"), Context),
			Result.ErrorCode,
			EMatchPlayOrdinaryDeploymentWriterErrorCode::LegalityFailed);
		Test.TestEqual(
			*FString::Printf(TEXT("%s preserves exact legality error"), Context),
			Result.LegalityResult.ErrorCode,
			ExpectedError);
		Test.TestEqual(
			*FString::Printf(TEXT("%s has no rotation error"), Context),
			Result.UnderlyingTurnRotationErrorCode,
			EMatchPlayDeploymentTurnRotationErrorCode::None);
		Test.TestTrue(
			*FString::Printf(TEXT("%s preserves result BeforeState"), Context),
			AreStatesEqual(Result.BeforeState, State));
		Test.TestTrue(
			*FString::Printf(TEXT("%s preserves result AfterState"), Context),
			AreStatesEqual(Result.AfterState, State));
		Test.TestTrue(
			*FString::Printf(TEXT("%s does not mutate input"), Context),
			AreStatesEqual(State, OriginalInput));
		return true;
	}
}

#define MATCH_PLAY_ORDINARY_DEPLOYMENT_WRITER_TEST(TestClass, TestName) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
		TestClass, \
		"FMCodex.CoreRules.MatchPlayOrdinaryDeployment.Writer." TestName, \
		EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

MATCH_PLAY_ORDINARY_DEPLOYMENT_WRITER_TEST(
	FMatchPlayOrdinaryDeploymentWriterPublicContractTest,
	"PublicContractAndDefaults")

bool FMatchPlayOrdinaryDeploymentWriterPublicContractTest::RunTest(
	const FString& Parameters)
{
	using FExpectedSignature = FMatchPlayOrdinaryDeploymentWriterResult (*)(
		const FMatchPlayState&,
		const FMatchPlayOrdinaryDeploymentRequest&);
	static_assert(std::is_same_v<
		decltype(&FMatchPlayOrdinaryDeploymentWriter::Deploy),
		FExpectedSignature>);

	const FMatchPlayOrdinaryDeploymentWriterResult Result;
	TestFalse(TEXT("Default result fails"), Result.bSuccess);
	TestEqual(TEXT("Default writer error is None"), Result.ErrorCode,
		EMatchPlayOrdinaryDeploymentWriterErrorCode::None);
	TestFalse(TEXT("Default legality is not legal"),
		Result.LegalityResult.bIsLegal);
	TestEqual(TEXT("Default rotation error is None"),
		Result.UnderlyingTurnRotationErrorCode,
		EMatchPlayDeploymentTurnRotationErrorCode::None);
	return true;
}

MATCH_PLAY_ORDINARY_DEPLOYMENT_WRITER_TEST(
	FMatchPlayOrdinaryDeploymentWriterSuccessTest,
	"SuccessAppendsExactPlacementAndRotates")

bool FMatchPlayOrdinaryDeploymentWriterSuccessTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayOrdinaryDeploymentWriter;
	const FMatchPlayState State = MakeState();
	const FMatchPlayOrdinaryDeploymentRequest Request = MakeRequest();
	const FMatchPlayOrdinaryDeploymentWriterResult Result =
		FMatchPlayOrdinaryDeploymentWriter::Deploy(State, Request);
	TestTrue(TEXT("Writer succeeds"), Result.bSuccess);
	TestTrue(TEXT("Underlying legality succeeds"),
		Result.LegalityResult.bIsLegal);
	TestEqual(TEXT("Writer error is clean"), Result.ErrorCode,
		EMatchPlayOrdinaryDeploymentWriterErrorCode::None);
	TestEqual(TEXT("Rotation error is clean"),
		Result.UnderlyingTurnRotationErrorCode,
		EMatchPlayDeploymentTurnRotationErrorCode::None);
	TestEqual(TEXT("One placement is appended"),
		Result.AfterState.CurrentAttack.DeploymentPlacements.Num(), 1);
	const FMatchPlayDeploymentPlacement& Placement =
		Result.AfterState.CurrentAttack.DeploymentPlacements[0];
	TestEqual(TEXT("Placement side is exact"),
		Placement.PlayerSide, Request.RequestingSide);
	TestEqual(TEXT("Placement CardId is exact"),
		Placement.CardId, Request.CardId);
	TestEqual(TEXT("Placement SlotId is exact"),
		Placement.SlotId, Request.SlotId);
	TestEqual(TEXT("Legal side rotates to PlayerB"),
		Result.AfterState.CurrentAttack.CurrentLegalDeploymentSide,
		EInitialTurnOrderPlayer::PlayerB);
	return true;
}

MATCH_PLAY_ORDINARY_DEPLOYMENT_WRITER_TEST(
	FMatchPlayOrdinaryDeploymentWriterAlternationTest,
	"TwoSuccessfulActionsAlternateAndShareCardIdentity")

bool FMatchPlayOrdinaryDeploymentWriterAlternationTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayOrdinaryDeploymentWriter;
	const FMatchPlayOrdinaryDeploymentWriterResult First =
		FMatchPlayOrdinaryDeploymentWriter::Deploy(
			MakeState(),
			MakeRequest());
	const FMatchPlayOrdinaryDeploymentWriterResult Second =
		FMatchPlayOrdinaryDeploymentWriter::Deploy(
			First.AfterState,
			MakeRequest(
				EInitialTurnOrderPlayer::PlayerB,
				SharedCardId,
				NearPlayerBSlotId));
	TestTrue(TEXT("Attacker deployment succeeds"), First.bSuccess);
	TestTrue(TEXT("Defender deployment succeeds"), Second.bSuccess);
	TestEqual(TEXT("Both placements are present"),
		Second.AfterState.CurrentAttack.DeploymentPlacements.Num(), 2);
	TestEqual(TEXT("First placement belongs to PlayerA"),
		Second.AfterState.CurrentAttack.DeploymentPlacements[0].PlayerSide,
		EInitialTurnOrderPlayer::PlayerA);
	TestEqual(TEXT("Second placement belongs to PlayerB"),
		Second.AfterState.CurrentAttack.DeploymentPlacements[1].PlayerSide,
		EInitialTurnOrderPlayer::PlayerB);
	TestEqual(TEXT("CardId can be shared across sides"),
		Second.AfterState.CurrentAttack.DeploymentPlacements[1].CardId,
		SharedCardId);
	TestEqual(TEXT("Turn returns to PlayerA"),
		Second.AfterState.CurrentAttack.CurrentLegalDeploymentSide,
		EInitialTurnOrderPlayer::PlayerA);
	return true;
}

MATCH_PLAY_ORDINARY_DEPLOYMENT_WRITER_TEST(
	FMatchPlayOrdinaryDeploymentWriterOpponentFinishedTest,
	"OpponentFinishedKeepsPlayerAActing")

bool FMatchPlayOrdinaryDeploymentWriterOpponentFinishedTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayOrdinaryDeploymentWriter;
	const FMatchPlayOrdinaryDeploymentWriterResult Result =
		FMatchPlayOrdinaryDeploymentWriter::Deploy(
			MakeState(
				EInitialTurnOrderPlayer::PlayerA,
				EInitialTurnOrderPlayer::PlayerA,
				false,
				true),
			MakeRequest());
	TestTrue(TEXT("Deployment succeeds"), Result.bSuccess);
	TestEqual(TEXT("PlayerA keeps legal side"),
		Result.AfterState.CurrentAttack.CurrentLegalDeploymentSide,
		EInitialTurnOrderPlayer::PlayerA);
	TestEqual(TEXT("Phase remains Deployment"),
		Result.AfterState.CurrentAttack.Phase,
		EMatchPlayCurrentAttackPhase::Deployment);
	return true;
}

MATCH_PLAY_ORDINARY_DEPLOYMENT_WRITER_TEST(
	FMatchPlayOrdinaryDeploymentWriterOpponentFinishedMirrorTest,
	"OpponentFinishedKeepsPlayerBAttackerActing")

bool FMatchPlayOrdinaryDeploymentWriterOpponentFinishedMirrorTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayOrdinaryDeploymentWriter;
	const FMatchPlayOrdinaryDeploymentWriterResult Result =
		FMatchPlayOrdinaryDeploymentWriter::Deploy(
			MakeState(
				EInitialTurnOrderPlayer::PlayerB,
				EInitialTurnOrderPlayer::PlayerB,
				false,
				true),
			MakeRequest(
				EInitialTurnOrderPlayer::PlayerB,
				SharedCardId,
				NearPlayerBSlotId));
	TestTrue(TEXT("Mirrored deployment succeeds"), Result.bSuccess);
	TestEqual(TEXT("PlayerB keeps legal side"),
		Result.AfterState.CurrentAttack.CurrentLegalDeploymentSide,
		EInitialTurnOrderPlayer::PlayerB);
	return true;
}

MATCH_PLAY_ORDINARY_DEPLOYMENT_WRITER_TEST(
	FMatchPlayOrdinaryDeploymentWriterGlobalOccupancyTest,
	"GlobalSlotOccupancyIsAtomic")

bool FMatchPlayOrdinaryDeploymentWriterGlobalOccupancyTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayOrdinaryDeploymentWriter;
	FMatchPlayState State = MakeState(
		EInitialTurnOrderPlayer::PlayerA,
		EInitialTurnOrderPlayer::PlayerB);
	State.CurrentAttack.DeploymentPlacements.Add(MakePlacement(
		EInitialTurnOrderPlayer::PlayerA,
		PlayerAOtherCardId,
		NearPlayerBSlotId));
	return TestLegalityFailure(
		*this,
		TEXT("Cross-side occupied Slot"),
		State,
		MakeRequest(
			EInitialTurnOrderPlayer::PlayerB,
			SharedCardId,
			NearPlayerBSlotId),
		EMatchPlayOrdinaryDeploymentErrorCode::SlotAlreadyOccupied);
}

MATCH_PLAY_ORDINARY_DEPLOYMENT_WRITER_TEST(
	FMatchPlayOrdinaryDeploymentWriterSameSideDuplicateTest,
	"SameSideDuplicateIsAtomic")

bool FMatchPlayOrdinaryDeploymentWriterSameSideDuplicateTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayOrdinaryDeploymentWriter;
	FMatchPlayState State = MakeState();
	State.CurrentAttack.DeploymentPlacements.Add(MakePlacement(
		EInitialTurnOrderPlayer::PlayerA,
		SharedCardId,
		OtherNearPlayerASlotId));
	return TestLegalityFailure(
		*this,
		TEXT("Same-side duplicate CardId"),
		State,
		MakeRequest(),
		EMatchPlayOrdinaryDeploymentErrorCode::CardAlreadyPlacedBySide);
}

MATCH_PLAY_ORDINARY_DEPLOYMENT_WRITER_TEST(
	FMatchPlayOrdinaryDeploymentWriterRelativeZoneTest,
	"PlayerBAttackerUsesRelativeZoneAuthority")

bool FMatchPlayOrdinaryDeploymentWriterRelativeZoneTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayOrdinaryDeploymentWriter;
	const FMatchPlayOrdinaryDeploymentWriterResult Result =
		FMatchPlayOrdinaryDeploymentWriter::Deploy(
			MakeState(
				EInitialTurnOrderPlayer::PlayerB,
				EInitialTurnOrderPlayer::PlayerB),
			MakeRequest(
				EInitialTurnOrderPlayer::PlayerB,
				PlayerBOtherCardId,
				NearPlayerASlotId));
	TestTrue(TEXT("PlayerB attack card deploys to its Forward zone"),
		Result.bSuccess);
	TestEqual(TEXT("Resolved zone is Forward"),
		Result.LegalityResult.ResolvedRelativeZone,
		EMatchPlayRelativeDeploymentZone::Forward);
	return true;
}

MATCH_PLAY_ORDINARY_DEPLOYMENT_WRITER_TEST(
	FMatchPlayOrdinaryDeploymentWriterGoalkeeperTest,
	"GoalkeeperOrdinaryRequestIsAtomic")

bool FMatchPlayOrdinaryDeploymentWriterGoalkeeperTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayOrdinaryDeploymentWriter;
	return TestLegalityFailure(
		*this,
		TEXT("GK ordinary request"),
		MakeState(),
		MakeRequest(
			EInitialTurnOrderPlayer::PlayerA,
			PlayerAGoalkeeperCardId),
		EMatchPlayOrdinaryDeploymentErrorCode::GoalkeeperNotAllowed);
}

MATCH_PLAY_ORDINARY_DEPLOYMENT_WRITER_TEST(
	FMatchPlayOrdinaryDeploymentWriterStaleSequenceTest,
	"StaleSequenceIsAtomic")

bool FMatchPlayOrdinaryDeploymentWriterStaleSequenceTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayOrdinaryDeploymentWriter;
	return TestLegalityFailure(
		*this,
		TEXT("Stale sequence"),
		MakeState(),
		MakeRequest(
			EInitialTurnOrderPlayer::PlayerA,
			SharedCardId,
			NearPlayerASlotId,
			ValidAttackSequence + 1),
		EMatchPlayOrdinaryDeploymentErrorCode::AttackSequenceMismatch);
}

MATCH_PLAY_ORDINARY_DEPLOYMENT_WRITER_TEST(
	FMatchPlayOrdinaryDeploymentWriterInvalidAttackerTest,
	"LegalityFailurePreventsRotation")

bool FMatchPlayOrdinaryDeploymentWriterInvalidAttackerTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayOrdinaryDeploymentWriter;
	FMatchPlayState State = MakeState();
	State.RuntimeState.CurrentAttackingPlayer =
		EInitialTurnOrderPlayer::None;
	return TestLegalityFailure(
		*this,
		TEXT("Invalid attacker"),
		State,
		MakeRequest(),
		EMatchPlayOrdinaryDeploymentErrorCode
			::InvalidCurrentAttackingPlayer);
}

MATCH_PLAY_ORDINARY_DEPLOYMENT_WRITER_TEST(
	FMatchPlayOrdinaryDeploymentWriterPositionFailureTest,
	"PositionFailureIsAtomic")

bool FMatchPlayOrdinaryDeploymentWriterPositionFailureTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayOrdinaryDeploymentWriter;
	return TestLegalityFailure(
		*this,
		TEXT("Defense card in attacker Forward"),
		MakeState(),
		MakeRequest(
			EInitialTurnOrderPlayer::PlayerA,
			PlayerAOtherCardId,
			NearPlayerBSlotId),
		EMatchPlayOrdinaryDeploymentErrorCode::PositionNotEligibleForZone);
}

MATCH_PLAY_ORDINARY_DEPLOYMENT_WRITER_TEST(
	FMatchPlayOrdinaryDeploymentWriterStatePreservationTest,
	"SuccessChangesOnlyPlacementAndLegalSide")

bool FMatchPlayOrdinaryDeploymentWriterStatePreservationTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayOrdinaryDeploymentWriter;
	const FMatchPlayState BeforeState = MakeState();
	const FMatchPlayOrdinaryDeploymentRequest Request = MakeRequest();
	const FMatchPlayOrdinaryDeploymentWriterResult Result =
		FMatchPlayOrdinaryDeploymentWriter::Deploy(BeforeState, Request);

	FMatchPlayState ExpectedState = BeforeState;
	ExpectedState.CurrentAttack.DeploymentPlacements.Add(MakePlacement(
		Request.RequestingSide,
		Request.CardId,
		Request.SlotId));
	ExpectedState.CurrentAttack.CurrentLegalDeploymentSide =
		EInitialTurnOrderPlayer::PlayerB;
	TestTrue(TEXT("Whole result state has only authorized changes"),
		AreStatesEqual(Result.AfterState, ExpectedState));
	TestTrue(TEXT("Input state remains unchanged"),
		AreStatesEqual(BeforeState, Result.BeforeState));
	TestTrue(TEXT("CardUsage remains field-for-field unchanged"),
		FMatchCardUsageState::StaticStruct()->CompareScriptStruct(
			&Result.AfterState.CardUsageState,
			&BeforeState.CardUsageState,
			0));
	TestTrue(TEXT("Card remains Available"),
		Result.AfterState.CardUsageState.PlayerACardUsageState
			.AvailableCardIds.Contains(SharedCardId));
	TestFalse(TEXT("Card does not enter Used"),
		Result.AfterState.CardUsageState.PlayerACardUsageState
			.UsedCardIds.Contains(SharedCardId));
	return true;
}

MATCH_PLAY_ORDINARY_DEPLOYMENT_WRITER_TEST(
	FMatchPlayOrdinaryDeploymentWriterAppendOrderTest,
	"PlacementAppendPreservesActionOrder")

bool FMatchPlayOrdinaryDeploymentWriterAppendOrderTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayOrdinaryDeploymentWriter;
	FMatchPlayState State = MakeState();
	const FMatchPlayDeploymentPlacement Existing = MakePlacement(
		EInitialTurnOrderPlayer::PlayerB,
		PlayerBOtherCardId,
		NearPlayerBSlotId);
	State.CurrentAttack.DeploymentPlacements.Add(Existing);
	const FMatchPlayOrdinaryDeploymentWriterResult Result =
		FMatchPlayOrdinaryDeploymentWriter::Deploy(
			State,
			MakeRequest());
	TestTrue(TEXT("Deployment succeeds"), Result.bSuccess);
	TestTrue(TEXT("Existing placement stays first"),
		FMatchPlayDeploymentPlacement::StaticStruct()
			->CompareScriptStruct(
				&Result.AfterState.CurrentAttack.DeploymentPlacements[0],
				&Existing,
				0));
	TestEqual(TEXT("New placement is appended second"),
		Result.AfterState.CurrentAttack.DeploymentPlacements[1].CardId,
		SharedCardId);
	return true;
}

MATCH_PLAY_ORDINARY_DEPLOYMENT_WRITER_TEST(
	FMatchPlayOrdinaryDeploymentWriterFinishInteractionTest,
	"FinishLeavesUnfinishedSideAbleToDeploy")

bool FMatchPlayOrdinaryDeploymentWriterFinishInteractionTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayOrdinaryDeploymentWriter;
	const FMatchPlayFinishDeploymentResult FinishResult =
		FMatchPlayFinishDeployment::Finish(
			MakeState(),
			ValidAttackSequence,
			EInitialTurnOrderPlayer::PlayerA);
	TestTrue(TEXT("Attacker Finish succeeds"), FinishResult.bSuccess);

	TestLegalityFailure(
		*this,
		TEXT("Finished side cannot act out of turn"),
		FinishResult.AfterState,
		MakeRequest(),
		EMatchPlayOrdinaryDeploymentErrorCode
			::RequestingSideNotCurrentLegalDeploymentSide);

	const FMatchPlayOrdinaryDeploymentWriterResult DefenderResult =
		FMatchPlayOrdinaryDeploymentWriter::Deploy(
			FinishResult.AfterState,
			MakeRequest(
				EInitialTurnOrderPlayer::PlayerB,
				SharedCardId,
				NearPlayerBSlotId));
	TestTrue(TEXT("Unfinished defender can deploy"),
		DefenderResult.bSuccess);
	TestEqual(TEXT("Defender keeps legal side after attacker finished"),
		DefenderResult.AfterState.CurrentAttack.CurrentLegalDeploymentSide,
		EInitialTurnOrderPlayer::PlayerB);
	TestEqual(TEXT("Phase remains Deployment"),
		DefenderResult.AfterState.CurrentAttack.Phase,
		EMatchPlayCurrentAttackPhase::Deployment);
	return true;
}

MATCH_PLAY_ORDINARY_DEPLOYMENT_WRITER_TEST(
	FMatchPlayOrdinaryDeploymentWriterUnavailableTest,
	"UnavailableCardFailureIsAtomic")

bool FMatchPlayOrdinaryDeploymentWriterUnavailableTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayOrdinaryDeploymentWriter;
	FMatchPlayState State = MakeState();
	State.CardUsageState.PlayerACardUsageState.AvailableCardIds.Remove(
		SharedCardId);
	return TestLegalityFailure(
		*this,
		TEXT("Unavailable card"),
		State,
		MakeRequest(),
		EMatchPlayOrdinaryDeploymentErrorCode::CardNotAvailable);
}

MATCH_PLAY_ORDINARY_DEPLOYMENT_WRITER_TEST(
	FMatchPlayOrdinaryDeploymentWriterDeterministicTest,
	"RepeatedDeploymentIsDeterministic")

bool FMatchPlayOrdinaryDeploymentWriterDeterministicTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayOrdinaryDeploymentWriter;
	const FMatchPlayState State = MakeState();
	const FMatchPlayOrdinaryDeploymentRequest Request = MakeRequest();
	const FMatchPlayOrdinaryDeploymentWriterResult First =
		FMatchPlayOrdinaryDeploymentWriter::Deploy(State, Request);
	const FMatchPlayOrdinaryDeploymentWriterResult Second =
		FMatchPlayOrdinaryDeploymentWriter::Deploy(State, Request);
	TestTrue(TEXT("Repeated results match field-for-field"),
		FMatchPlayOrdinaryDeploymentWriterResult::StaticStruct()
			->CompareScriptStruct(&First, &Second, 0));
	return true;
}

MATCH_PLAY_ORDINARY_DEPLOYMENT_WRITER_TEST(
	FMatchPlayOrdinaryDeploymentWriterRotationFailureContractTest,
	"RotationFailureResultDefaultsRemainAtomic")

bool FMatchPlayOrdinaryDeploymentWriterRotationFailureContractTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayOrdinaryDeploymentWriter;
	const FMatchPlayState State = MakeState();
	const FMatchPlayState OriginalInput = State;
	const FMatchPlayOrdinaryDeploymentRequest Request = MakeRequest();
	FMatchPlayOrdinaryDeploymentWriterResult PendingResult;
	PendingResult.Request = Request;
	PendingResult.BeforeState = State;
	PendingResult.AfterState = State;
	PendingResult.LegalityResult =
		FMatchPlayOrdinaryDeploymentLegalityEvaluator::Evaluate(
			State,
			Request);
	TestTrue(TEXT("Test seam starts after successful legality"),
		PendingResult.LegalityResult.bIsLegal);

	FMatchPlayDeploymentTurnRotationResult RotationFailure;
	RotationFailure.ErrorCode =
		EMatchPlayDeploymentTurnRotationErrorCode::InvalidActingSide;
	RotationFailure.ErrorMessage = TEXT("Forced structural rotation failure.");
	const FMatchPlayOrdinaryDeploymentWriterResult Result =
		FMatchPlayOrdinaryDeploymentWriter::FinalizeAfterRotation(
			State,
			Request,
			MoveTemp(PendingResult),
			RotationFailure);
	TestFalse(TEXT("Rotation failure rejects commit"), Result.bSuccess);
	TestEqual(TEXT("Top-level result is TurnRotationFailed"),
		Result.ErrorCode,
		EMatchPlayOrdinaryDeploymentWriterErrorCode::TurnRotationFailed);
	TestEqual(TEXT("Exact rotation error is preserved"),
		Result.UnderlyingTurnRotationErrorCode,
		EMatchPlayDeploymentTurnRotationErrorCode::InvalidActingSide);
	TestTrue(TEXT("AfterState remains atomic"),
		AreStatesEqual(Result.AfterState, State));
	TestTrue(TEXT("BeforeState remains exact"),
		AreStatesEqual(Result.BeforeState, State));
	TestTrue(TEXT("Input remains unchanged"),
		AreStatesEqual(State, OriginalInput));
	return true;
}

#undef MATCH_PLAY_ORDINARY_DEPLOYMENT_WRITER_TEST

#endif
