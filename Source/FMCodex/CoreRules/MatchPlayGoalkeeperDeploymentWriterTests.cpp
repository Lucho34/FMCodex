#include "MatchPlayGoalkeeperDeploymentWriter.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "MatchPlayBeginOrdinaryAttack.h"
#include "MatchPlayFinishDeployment.h"
#include "MatchPlayOrdinaryDeploymentWriter.h"
#include "Misc/AutomationTest.h"

#include <type_traits>

namespace FMCodex::Tests::MatchPlayGoalkeeperDeploymentWriter
{
	constexpr int64 ValidAttackSequence = 5;
	const FName PlayerAGoalkeeperCardId(TEXT("PlayerA.GK"));
	const FName PlayerAOutfieldCardId(TEXT("PlayerA.Attacker"));
	const FName PlayerBGoalkeeperCardId(TEXT("PlayerB.GK"));
	const FName PlayerBOutfieldCardId(TEXT("PlayerB.Attacker"));
	const FName NearPlayerASlotId(TEXT("Slot.NearPlayerA"));
	const FName NearPlayerBSlotId(TEXT("Slot.NearPlayerB"));
	const FName OtherNearPlayerBSlotId(TEXT("Slot.NearPlayerB.Other"));

	FPlayerCardRuleSnapshot MakeGoalkeeperWriterSnapshot(
		const FName CardId,
		const EPlayerPositionType PositionType,
		const bool bIsGoalkeeper = false)
	{
		FPlayerCardRuleSnapshot Snapshot;
		Snapshot.CardId = CardId;
		Snapshot.PositionTypes = { PositionType };
		Snapshot.bIsGoalkeeper = bIsGoalkeeper;
		Snapshot.bHasGoalkeeperAttributes = bIsGoalkeeper;
		return Snapshot;
	}

	FMatchPlayDeploymentSlotDefinition MakeGoalkeeperWriterSlot(
		const FName SlotId,
		const EMatchPlayNeutralSlotSide NeutralSide)
	{
		FMatchPlayDeploymentSlotDefinition Slot;
		Slot.SlotId = SlotId;
		Slot.NeutralSide = NeutralSide;
		return Slot;
	}

	FMatchPlayDeploymentPlacement MakeGoalkeeperWriterPlacement(
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

	FMatchPlayState MakeGoalkeeperWriterState(
		const EInitialTurnOrderPlayer CurrentAttacker =
			EInitialTurnOrderPlayer::PlayerA)
	{
		FMatchPlayState State;
		State.RuntimeState.bIsInitialized = true;
		State.RuntimeState.FirstPlayer = EInitialTurnOrderPlayer::PlayerA;
		State.RuntimeState.SecondPlayer = EInitialTurnOrderPlayer::PlayerB;
		State.RuntimeState.CurrentAttackingPlayer = CurrentAttacker;
		State.RuntimeState.PlayerAState.TotalAttackCount = 6;
		State.RuntimeState.PlayerAState.UsedAttackCount = 2;
		State.RuntimeState.PlayerAState.Score = 3;
		State.RuntimeState.PlayerBState.TotalAttackCount = 6;
		State.RuntimeState.PlayerBState.UsedAttackCount = 2;
		State.RuntimeState.PlayerBState.Score = 2;

		State.CardUsageState.PlayerACardUsageState.AvailableCardIds = {
			PlayerAGoalkeeperCardId,
			PlayerAOutfieldCardId
		};
		State.CardUsageState.PlayerBCardUsageState.AvailableCardIds = {
			PlayerBGoalkeeperCardId,
			PlayerBOutfieldCardId
		};
		State.CardSnapshotAuthority.PlayerACardSnapshots.Cards = {
			MakeGoalkeeperWriterSnapshot(
				PlayerAGoalkeeperCardId,
				EPlayerPositionType::Goalkeeper,
				true),
			MakeGoalkeeperWriterSnapshot(
				PlayerAOutfieldCardId,
				EPlayerPositionType::Attack)
		};
		State.CardSnapshotAuthority.PlayerBCardSnapshots.Cards = {
			MakeGoalkeeperWriterSnapshot(
				PlayerBGoalkeeperCardId,
				EPlayerPositionType::Goalkeeper,
				true),
			MakeGoalkeeperWriterSnapshot(
				PlayerBOutfieldCardId,
				EPlayerPositionType::Attack)
		};
		State.DeploymentSlotCatalog.Slots = {
			MakeGoalkeeperWriterSlot(
				NearPlayerASlotId,
				EMatchPlayNeutralSlotSide::NearPlayerA),
			MakeGoalkeeperWriterSlot(
				NearPlayerBSlotId,
				EMatchPlayNeutralSlotSide::NearPlayerB),
			MakeGoalkeeperWriterSlot(
				OtherNearPlayerBSlotId,
				EMatchPlayNeutralSlotSide::NearPlayerB)
		};

		State.bHasCurrentAttack = true;
		State.CurrentAttack.Phase =
			EMatchPlayCurrentAttackPhase::Deployment;
		State.CurrentAttack.AttackSequence = ValidAttackSequence;
		State.CurrentAttack.ActionPoint = 7;
		State.CurrentAttack.CurrentLegalDeploymentSide =
			CurrentAttacker == EInitialTurnOrderPlayer::PlayerA
				? EInitialTurnOrderPlayer::PlayerB
				: EInitialTurnOrderPlayer::PlayerA;
		return State;
	}

	FMatchPlayGoalkeeperDeploymentRequest MakeGoalkeeperWriterRequest(
		const EInitialTurnOrderPlayer CurrentAttacker =
			EInitialTurnOrderPlayer::PlayerA)
	{
		FMatchPlayGoalkeeperDeploymentRequest Request;
		Request.AttackSequence = ValidAttackSequence;
		Request.RequestingSide =
			CurrentAttacker == EInitialTurnOrderPlayer::PlayerA
				? EInitialTurnOrderPlayer::PlayerB
				: EInitialTurnOrderPlayer::PlayerA;
		Request.CardId =
			Request.RequestingSide == EInitialTurnOrderPlayer::PlayerA
				? PlayerAGoalkeeperCardId
				: PlayerBGoalkeeperCardId;
		Request.SlotId =
			Request.RequestingSide == EInitialTurnOrderPlayer::PlayerA
				? NearPlayerASlotId
				: NearPlayerBSlotId;
		return Request;
	}

	bool AreGoalkeeperWriterStatesEqual(
		const FMatchPlayState& Left,
		const FMatchPlayState& Right)
	{
		return FMatchPlayState::StaticStruct()->CompareScriptStruct(
			&Left,
			&Right,
			0);
	}

	bool AreGoalkeeperWriterLegalityResultsEqual(
		const FMatchPlayGoalkeeperDeploymentLegalityResult& Left,
		const FMatchPlayGoalkeeperDeploymentLegalityResult& Right)
	{
		return FMatchPlayGoalkeeperDeploymentLegalityResult::StaticStruct()
				->CompareScriptStruct(&Left, &Right, 0)
			&& Left.UnderlyingSnapshotAuthorityQueryErrorCode
				== Right.UnderlyingSnapshotAuthorityQueryErrorCode;
	}

	bool ExpectGoalkeeperWriterLegalityFailure(
		FAutomationTestBase& Test,
		const TCHAR* Context,
		const FMatchPlayState& State,
		const FMatchPlayGoalkeeperDeploymentRequest& Request,
		const EMatchPlayGoalkeeperDeploymentErrorCode ExpectedError)
	{
		const FMatchPlayState OriginalState = State;
		const FMatchPlayGoalkeeperDeploymentWriterResult Result =
			FMatchPlayGoalkeeperDeploymentWriter::Deploy(State, Request);
		Test.TestFalse(
			*FString::Printf(TEXT("%s fails"), Context),
			Result.bSucceeded);
		Test.TestEqual(
			*FString::Printf(TEXT("%s maps writer error"), Context),
			Result.ErrorCode,
			EMatchPlayGoalkeeperDeploymentWriterErrorCode::LegalityFailed);
		Test.TestEqual(
			*FString::Printf(TEXT("%s preserves legality error"), Context),
			Result.LegalityResult.ErrorCode,
			ExpectedError);
		Test.TestEqual(
			*FString::Printf(TEXT("%s has no rotation error"), Context),
			Result.UnderlyingTurnRotationErrorCode,
			EMatchPlayDeploymentTurnRotationErrorCode::None);
		Test.TestEqual(
			*FString::Printf(TEXT("%s has no usage update error"), Context),
			Result.UnderlyingGoalkeeperUsageErrorCode,
			EMatchPlayGoalkeeperUsageErrorCode::None);
		Test.TestTrue(
			*FString::Printf(TEXT("%s preserves BeforeState"), Context),
			AreGoalkeeperWriterStatesEqual(Result.BeforeState, State));
		Test.TestTrue(
			*FString::Printf(TEXT("%s returns BeforeState atomically"), Context),
			AreGoalkeeperWriterStatesEqual(Result.AfterState, State));
		Test.TestTrue(
			*FString::Printf(TEXT("%s does not mutate input"), Context),
			AreGoalkeeperWriterStatesEqual(State, OriginalState));
		return true;
	}

	FMatchPlayGoalkeeperDeploymentWriterResult
		MakeGoalkeeperWriterPrecommitResult(
			const FMatchPlayState& State,
			const FMatchPlayGoalkeeperDeploymentRequest& Request)
	{
		FMatchPlayGoalkeeperDeploymentWriterResult Result;
		Result.Request = Request;
		Result.BeforeState = State;
		Result.AfterState = State;
		Result.LegalityResult =
			FMatchPlayGoalkeeperDeploymentLegalityEvaluator::Evaluate(
				State,
				Request);
		return Result;
	}
}

#define MATCH_PLAY_GOALKEEPER_DEPLOYMENT_WRITER_TEST(TestClass, TestName) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
		TestClass, \
		"FMCodex.CoreRules.MatchPlayGoalkeeperDeployment.Writer." TestName, \
		EAutomationTestFlags::EditorContext \
			| EAutomationTestFlags::EngineFilter)

MATCH_PLAY_GOALKEEPER_DEPLOYMENT_WRITER_TEST(
	FMatchPlayGoalkeeperDeploymentWriterPublicContractTest,
	"PublicContractAndDefaults")

bool FMatchPlayGoalkeeperDeploymentWriterPublicContractTest::RunTest(
	const FString& Parameters)
{
	using FExpectedSignature =
		FMatchPlayGoalkeeperDeploymentWriterResult (*)(
			const FMatchPlayState&,
			const FMatchPlayGoalkeeperDeploymentRequest&);
	static_assert(std::is_same_v<
		decltype(&FMatchPlayGoalkeeperDeploymentWriter::Deploy),
		FExpectedSignature>);

	const FMatchPlayGoalkeeperDeploymentWriterResult Result;
	TestFalse(TEXT("Default result fails"), Result.bSucceeded);
	TestEqual(TEXT("Default writer error is None"), Result.ErrorCode,
		EMatchPlayGoalkeeperDeploymentWriterErrorCode::None);
	TestEqual(TEXT("Default rotation error is None"),
		Result.UnderlyingTurnRotationErrorCode,
		EMatchPlayDeploymentTurnRotationErrorCode::None);
	TestEqual(TEXT("Default usage error is None"),
		Result.UnderlyingGoalkeeperUsageErrorCode,
		EMatchPlayGoalkeeperUsageErrorCode::None);
	return true;
}

MATCH_PLAY_GOALKEEPER_DEPLOYMENT_WRITER_TEST(
	FMatchPlayGoalkeeperDeploymentWriterPlayerBSuccessTest,
	"PlayerBDefenderSuccessMutatesOnlyAuthorizedFields")

bool FMatchPlayGoalkeeperDeploymentWriterPlayerBSuccessTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayGoalkeeperDeploymentWriter;
	FMatchPlayState State = MakeGoalkeeperWriterState();
	State.CurrentAttack.DeploymentPlacements.Add(
		MakeGoalkeeperWriterPlacement(
			EInitialTurnOrderPlayer::PlayerA,
			PlayerAOutfieldCardId,
			NearPlayerASlotId));
	const FMatchPlayState OriginalInput = State;
	const FMatchPlayGoalkeeperDeploymentRequest Request =
		MakeGoalkeeperWriterRequest();
	const FMatchPlayGoalkeeperDeploymentWriterResult Result =
		FMatchPlayGoalkeeperDeploymentWriter::Deploy(State, Request);

	FMatchPlayState ExpectedState = State;
	ExpectedState.CurrentAttack.DeploymentPlacements.Add(
		MakeGoalkeeperWriterPlacement(
			Request.RequestingSide,
			Request.CardId,
			Request.SlotId));
	ExpectedState.GoalkeeperUsageState.bPlayerBGoalkeeperCardUsed = true;
	ExpectedState.CurrentAttack.bCurrentDefenseGoalkeeperActivated = true;
	ExpectedState.CurrentAttack.CurrentLegalDeploymentSide =
		EInitialTurnOrderPlayer::PlayerA;

	TestTrue(TEXT("Writer succeeds"), Result.bSucceeded);
	TestTrue(TEXT("Legality succeeds"), Result.LegalityResult.bIsLegal);
	TestEqual(TEXT("Writer error is clean"), Result.ErrorCode,
		EMatchPlayGoalkeeperDeploymentWriterErrorCode::None);
	TestEqual(TEXT("Rotation error is clean"),
		Result.UnderlyingTurnRotationErrorCode,
		EMatchPlayDeploymentTurnRotationErrorCode::None);
	TestEqual(TEXT("Usage error is clean"),
		Result.UnderlyingGoalkeeperUsageErrorCode,
		EMatchPlayGoalkeeperUsageErrorCode::None);
	TestTrue(TEXT("Whole success State matches four authorized mutations"),
		AreGoalkeeperWriterStatesEqual(Result.AfterState, ExpectedState));
	TestTrue(TEXT("Input State remains unchanged"),
		AreGoalkeeperWriterStatesEqual(State, OriginalInput));
	TestEqual(TEXT("Existing placement remains first"),
		Result.AfterState.CurrentAttack.DeploymentPlacements[0].CardId,
		PlayerAOutfieldCardId);
	const FMatchPlayDeploymentPlacement& GoalkeeperPlacement =
		Result.AfterState.CurrentAttack.DeploymentPlacements.Last();
	TestEqual(TEXT("GK placement side is exact"),
		GoalkeeperPlacement.PlayerSide, Request.RequestingSide);
	TestEqual(TEXT("GK placement CardId is exact"),
		GoalkeeperPlacement.CardId, Request.CardId);
	TestEqual(TEXT("GK placement SlotId is exact"),
		GoalkeeperPlacement.SlotId, Request.SlotId);
	TestTrue(TEXT("GK remains CardUsage Available"),
		Result.AfterState.CardUsageState.PlayerBCardUsageState
			.AvailableCardIds.Contains(Request.CardId));
	TestFalse(TEXT("GK does not enter CardUsage Used"),
		Result.AfterState.CardUsageState.PlayerBCardUsageState
			.UsedCardIds.Contains(Request.CardId));
	return true;
}

MATCH_PLAY_GOALKEEPER_DEPLOYMENT_WRITER_TEST(
	FMatchPlayGoalkeeperDeploymentWriterPlayerASuccessTest,
	"PlayerADefenderMirrorUpdatesOnlyPlayerAUsage")

bool FMatchPlayGoalkeeperDeploymentWriterPlayerASuccessTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayGoalkeeperDeploymentWriter;
	const FMatchPlayGoalkeeperDeploymentWriterResult Result =
		FMatchPlayGoalkeeperDeploymentWriter::Deploy(
			MakeGoalkeeperWriterState(EInitialTurnOrderPlayer::PlayerB),
			MakeGoalkeeperWriterRequest(EInitialTurnOrderPlayer::PlayerB));
	TestTrue(TEXT("Mirrored Writer succeeds"), Result.bSucceeded);
	TestTrue(TEXT("PlayerA persistent usage is set"),
		Result.AfterState.GoalkeeperUsageState
			.bPlayerAGoalkeeperCardUsed);
	TestFalse(TEXT("PlayerB persistent usage remains false"),
		Result.AfterState.GoalkeeperUsageState
			.bPlayerBGoalkeeperCardUsed);
	TestTrue(TEXT("Current defense activation is set"),
		Result.AfterState.CurrentAttack
			.bCurrentDefenseGoalkeeperActivated);
	TestEqual(TEXT("Turn rotates to PlayerB attacker"),
		Result.AfterState.CurrentAttack.CurrentLegalDeploymentSide,
		EInitialTurnOrderPlayer::PlayerB);
	TestEqual(TEXT("Phase remains Deployment"),
		Result.AfterState.CurrentAttack.Phase,
		EMatchPlayCurrentAttackPhase::Deployment);
	return true;
}

MATCH_PLAY_GOALKEEPER_DEPLOYMENT_WRITER_TEST(
	FMatchPlayGoalkeeperDeploymentWriterOpponentFinishedTest,
	"FinishedAttackerKeepsDefenderLegalAfterSuccess")

bool FMatchPlayGoalkeeperDeploymentWriterOpponentFinishedTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayGoalkeeperDeploymentWriter;
	FMatchPlayState State = MakeGoalkeeperWriterState();
	State.CurrentAttack.bAttackerDeploymentFinished = true;
	const FMatchPlayGoalkeeperDeploymentWriterResult Result =
		FMatchPlayGoalkeeperDeploymentWriter::Deploy(
			State,
			MakeGoalkeeperWriterRequest());
	TestTrue(TEXT("GK deployment succeeds"), Result.bSucceeded);
	TestEqual(TEXT("Defender retains legal side"),
		Result.AfterState.CurrentAttack.CurrentLegalDeploymentSide,
		EInitialTurnOrderPlayer::PlayerB);
	TestEqual(TEXT("Phase remains Deployment"),
		Result.AfterState.CurrentAttack.Phase,
		EMatchPlayCurrentAttackPhase::Deployment);
	TestTrue(TEXT("Attacker finished flag is preserved"),
		Result.AfterState.CurrentAttack.bAttackerDeploymentFinished);
	TestFalse(TEXT("Defender remains unfinished"),
		Result.AfterState.CurrentAttack.bDefenderDeploymentFinished);
	return true;
}

MATCH_PLAY_GOALKEEPER_DEPLOYMENT_WRITER_TEST(
	FMatchPlayGoalkeeperDeploymentWriterAttackerRequestTest,
	"AttackerRequestIsAtomic")

bool FMatchPlayGoalkeeperDeploymentWriterAttackerRequestTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayGoalkeeperDeploymentWriter;
	FMatchPlayState State = MakeGoalkeeperWriterState();
	State.CurrentAttack.CurrentLegalDeploymentSide =
		EInitialTurnOrderPlayer::PlayerA;
	FMatchPlayGoalkeeperDeploymentRequest Request =
		MakeGoalkeeperWriterRequest();
	Request.RequestingSide = EInitialTurnOrderPlayer::PlayerA;
	Request.CardId = PlayerAGoalkeeperCardId;
	Request.SlotId = NearPlayerASlotId;
	return ExpectGoalkeeperWriterLegalityFailure(
		*this,
		TEXT("Attacker request"),
		State,
		Request,
		EMatchPlayGoalkeeperDeploymentErrorCode
			::RequestingSideIsNotDefender);
}

MATCH_PLAY_GOALKEEPER_DEPLOYMENT_WRITER_TEST(
	FMatchPlayGoalkeeperDeploymentWriterWrongSideTest,
	"WrongLegalSideIsAtomic")

bool FMatchPlayGoalkeeperDeploymentWriterWrongSideTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayGoalkeeperDeploymentWriter;
	FMatchPlayState State = MakeGoalkeeperWriterState();
	State.CurrentAttack.CurrentLegalDeploymentSide =
		EInitialTurnOrderPlayer::PlayerA;
	return ExpectGoalkeeperWriterLegalityFailure(
		*this,
		TEXT("Wrong legal side"),
		State,
		MakeGoalkeeperWriterRequest(),
		EMatchPlayGoalkeeperDeploymentErrorCode
			::RequestingSideNotCurrentLegalDeploymentSide);
}

MATCH_PLAY_GOALKEEPER_DEPLOYMENT_WRITER_TEST(
	FMatchPlayGoalkeeperDeploymentWriterFinishedTest,
	"FinishedDefenderIsAtomic")

bool FMatchPlayGoalkeeperDeploymentWriterFinishedTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayGoalkeeperDeploymentWriter;
	FMatchPlayState State = MakeGoalkeeperWriterState();
	State.CurrentAttack.bDefenderDeploymentFinished = true;
	return ExpectGoalkeeperWriterLegalityFailure(
		*this,
		TEXT("Finished defender"),
		State,
		MakeGoalkeeperWriterRequest(),
		EMatchPlayGoalkeeperDeploymentErrorCode
			::RequestingSideAlreadyFinished);
}

MATCH_PLAY_GOALKEEPER_DEPLOYMENT_WRITER_TEST(
	FMatchPlayGoalkeeperDeploymentWriterBothFinishedTest,
	"BothFinishedCorruptionIsAtomic")

bool FMatchPlayGoalkeeperDeploymentWriterBothFinishedTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayGoalkeeperDeploymentWriter;
	FMatchPlayState State = MakeGoalkeeperWriterState();
	State.CurrentAttack.bAttackerDeploymentFinished = true;
	State.CurrentAttack.bDefenderDeploymentFinished = true;
	return ExpectGoalkeeperWriterLegalityFailure(
		*this,
		TEXT("Both-finished corruption"),
		State,
		MakeGoalkeeperWriterRequest(),
		EMatchPlayGoalkeeperDeploymentErrorCode
			::InvalidDeploymentFinishedState);
}

MATCH_PLAY_GOALKEEPER_DEPLOYMENT_WRITER_TEST(
	FMatchPlayGoalkeeperDeploymentWriterAlreadyUsedTest,
	"AlreadyUsedIsAtomic")

bool FMatchPlayGoalkeeperDeploymentWriterAlreadyUsedTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayGoalkeeperDeploymentWriter;
	FMatchPlayState State = MakeGoalkeeperWriterState();
	State.GoalkeeperUsageState.bPlayerBGoalkeeperCardUsed = true;
	return ExpectGoalkeeperWriterLegalityFailure(
		*this,
		TEXT("Already-used goalkeeper"),
		State,
		MakeGoalkeeperWriterRequest(),
		EMatchPlayGoalkeeperDeploymentErrorCode
			::GoalkeeperAlreadyUsedThisMatch);
}

MATCH_PLAY_GOALKEEPER_DEPLOYMENT_WRITER_TEST(
	FMatchPlayGoalkeeperDeploymentWriterAlreadyActivatedTest,
	"AlreadyActivatedPrecedesUsedAndSlot")

bool FMatchPlayGoalkeeperDeploymentWriterAlreadyActivatedTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayGoalkeeperDeploymentWriter;
	FMatchPlayState State = MakeGoalkeeperWriterState();
	State.GoalkeeperUsageState.bPlayerBGoalkeeperCardUsed = true;
	State.CurrentAttack.bCurrentDefenseGoalkeeperActivated = true;
	State.CurrentAttack.DeploymentPlacements.Add(
		MakeGoalkeeperWriterPlacement(
			EInitialTurnOrderPlayer::PlayerB,
			PlayerBGoalkeeperCardId,
			NearPlayerBSlotId));
	FMatchPlayGoalkeeperDeploymentRequest Request =
		MakeGoalkeeperWriterRequest();
	Request.SlotId = OtherNearPlayerBSlotId;
	return ExpectGoalkeeperWriterLegalityFailure(
		*this,
		TEXT("Repeated current-attack request"),
		State,
		Request,
		EMatchPlayGoalkeeperDeploymentErrorCode
			::GoalkeeperAlreadyActivatedThisAttack);
}

MATCH_PLAY_GOALKEEPER_DEPLOYMENT_WRITER_TEST(
	FMatchPlayGoalkeeperDeploymentWriterOrdinaryOccupiedTest,
	"OrdinaryPlacementBlocksGoalkeeperAtomically")

bool FMatchPlayGoalkeeperDeploymentWriterOrdinaryOccupiedTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayGoalkeeperDeploymentWriter;
	FMatchPlayState State = MakeGoalkeeperWriterState();
	State.CurrentAttack.DeploymentPlacements.Add(
		MakeGoalkeeperWriterPlacement(
			EInitialTurnOrderPlayer::PlayerA,
			PlayerAOutfieldCardId,
			NearPlayerBSlotId));
	return ExpectGoalkeeperWriterLegalityFailure(
		*this,
		TEXT("Ordinary-occupied Slot"),
		State,
		MakeGoalkeeperWriterRequest(),
		EMatchPlayGoalkeeperDeploymentErrorCode::SlotAlreadyOccupied);
}

MATCH_PLAY_GOALKEEPER_DEPLOYMENT_WRITER_TEST(
	FMatchPlayGoalkeeperDeploymentWriterNonBackfieldTest,
	"NonBackfieldSlotIsAtomic")

bool FMatchPlayGoalkeeperDeploymentWriterNonBackfieldTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayGoalkeeperDeploymentWriter;
	FMatchPlayGoalkeeperDeploymentRequest Request =
		MakeGoalkeeperWriterRequest();
	Request.SlotId = NearPlayerASlotId;
	return ExpectGoalkeeperWriterLegalityFailure(
		*this,
		TEXT("Defender Midfield Slot"),
		MakeGoalkeeperWriterState(),
		Request,
		EMatchPlayGoalkeeperDeploymentErrorCode
			::GoalkeeperSlotNotInDefenderBackfield);
}

MATCH_PLAY_GOALKEEPER_DEPLOYMENT_WRITER_TEST(
	FMatchPlayGoalkeeperDeploymentWriterUnderlyingLegalityTest,
	"LegalityFailurePreservesCompleteUnderlyingResult")

bool FMatchPlayGoalkeeperDeploymentWriterUnderlyingLegalityTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayGoalkeeperDeploymentWriter;
	const FMatchPlayState State = MakeGoalkeeperWriterState();
	FMatchPlayGoalkeeperDeploymentRequest Request =
		MakeGoalkeeperWriterRequest();
	Request.CardId = TEXT("Missing.Goalkeeper");
	const FMatchPlayGoalkeeperDeploymentLegalityResult Direct =
		FMatchPlayGoalkeeperDeploymentLegalityEvaluator::Evaluate(
			State,
			Request);
	const FMatchPlayGoalkeeperDeploymentWriterResult WriterResult =
		FMatchPlayGoalkeeperDeploymentWriter::Deploy(State, Request);
	TestFalse(TEXT("Writer rejects missing Snapshot"),
		WriterResult.bSucceeded);
	TestEqual(TEXT("Writer maps legality failure"),
		WriterResult.ErrorCode,
		EMatchPlayGoalkeeperDeploymentWriterErrorCode::LegalityFailed);
	TestTrue(TEXT("Complete legality result is preserved"),
		AreGoalkeeperWriterLegalityResultsEqual(
			WriterResult.LegalityResult,
			Direct));
	TestEqual(TEXT("Snapshot underlying error is exact"),
		WriterResult.LegalityResult
			.UnderlyingSnapshotAuthorityQueryErrorCode,
		EMatchPlayCardSnapshotAuthorityQueryErrorCode::SnapshotNotFound);
	TestTrue(TEXT("Underlying legality failure remains atomic"),
		AreGoalkeeperWriterStatesEqual(WriterResult.AfterState, State));
	return true;
}

MATCH_PLAY_GOALKEEPER_DEPLOYMENT_WRITER_TEST(
	FMatchPlayGoalkeeperDeploymentWriterRotationFailureContractTest,
	"InjectedRotationFailureIsAtomic")

bool FMatchPlayGoalkeeperDeploymentWriterRotationFailureContractTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayGoalkeeperDeploymentWriter;
	const FMatchPlayState State = MakeGoalkeeperWriterState();
	const FMatchPlayGoalkeeperDeploymentRequest Request =
		MakeGoalkeeperWriterRequest();
	FMatchPlayDeploymentTurnRotationResult RotationFailure;
	RotationFailure.ErrorCode =
		EMatchPlayDeploymentTurnRotationErrorCode::InvalidActingSide;
	RotationFailure.ErrorMessage = TEXT("Injected rotation failure.");

	const FMatchPlayGoalkeeperDeploymentWriterResult Result =
		FMatchPlayGoalkeeperDeploymentWriter::FinalizeAfterRotation(
			State,
			Request,
			MakeGoalkeeperWriterPrecommitResult(State, Request),
			RotationFailure);
	TestFalse(TEXT("Rotation failure does not succeed"),
		Result.bSucceeded);
	TestEqual(TEXT("Writer maps rotation failure"), Result.ErrorCode,
		EMatchPlayGoalkeeperDeploymentWriterErrorCode
			::TurnRotationFailed);
	TestEqual(TEXT("Underlying rotation error is exact"),
		Result.UnderlyingTurnRotationErrorCode,
		EMatchPlayDeploymentTurnRotationErrorCode::InvalidActingSide);
	TestEqual(TEXT("Usage error remains None"),
		Result.UnderlyingGoalkeeperUsageErrorCode,
		EMatchPlayGoalkeeperUsageErrorCode::None);
	TestTrue(TEXT("Rotation failure is atomic"),
		AreGoalkeeperWriterStatesEqual(Result.AfterState, State));
	return true;
}

MATCH_PLAY_GOALKEEPER_DEPLOYMENT_WRITER_TEST(
	FMatchPlayGoalkeeperDeploymentWriterUsageFailureContractTest,
	"InjectedUsageUpdateFailureIsAtomic")

bool FMatchPlayGoalkeeperDeploymentWriterUsageFailureContractTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayGoalkeeperDeploymentWriter;
	const FMatchPlayState State = MakeGoalkeeperWriterState();
	const FMatchPlayGoalkeeperDeploymentRequest Request =
		MakeGoalkeeperWriterRequest();
	const FMatchPlayDeploymentTurnRotationResult RotationResult =
		FMatchPlayDeploymentTurnRotation::Resolve(
			State.RuntimeState.CurrentAttackingPlayer,
			Request.RequestingSide,
			State.CurrentAttack.bAttackerDeploymentFinished,
			State.CurrentAttack.bDefenderDeploymentFinished);
	FMatchPlayGoalkeeperUsageUpdateResult UsageFailure;
	UsageFailure.PlayerSide = Request.RequestingSide;
	UsageFailure.UpdatedUsageState = State.GoalkeeperUsageState;
	UsageFailure.ErrorCode =
		EMatchPlayGoalkeeperUsageErrorCode::GoalkeeperAlreadyUsed;
	UsageFailure.ErrorMessage = TEXT("Injected usage update failure.");

	const FMatchPlayGoalkeeperDeploymentWriterResult Result =
		FMatchPlayGoalkeeperDeploymentWriter::FinalizeAfterUsageUpdate(
			State,
			Request,
			MakeGoalkeeperWriterPrecommitResult(State, Request),
			RotationResult,
			UsageFailure);
	TestFalse(TEXT("Usage failure does not succeed"), Result.bSucceeded);
	TestEqual(TEXT("Writer maps usage failure"), Result.ErrorCode,
		EMatchPlayGoalkeeperDeploymentWriterErrorCode
			::GoalkeeperUsageUpdateFailed);
	TestEqual(TEXT("Rotation error remains None"),
		Result.UnderlyingTurnRotationErrorCode,
		EMatchPlayDeploymentTurnRotationErrorCode::None);
	TestEqual(TEXT("Underlying usage error is exact"),
		Result.UnderlyingGoalkeeperUsageErrorCode,
		EMatchPlayGoalkeeperUsageErrorCode::GoalkeeperAlreadyUsed);
	TestTrue(TEXT("Usage failure is atomic"),
		AreGoalkeeperWriterStatesEqual(Result.AfterState, State));
	return true;
}

MATCH_PLAY_GOALKEEPER_DEPLOYMENT_WRITER_TEST(
	FMatchPlayGoalkeeperDeploymentWriterRepeatedRequestTest,
	"SuccessfulWriteThenRepeatReturnsAlreadyActivated")

bool FMatchPlayGoalkeeperDeploymentWriterRepeatedRequestTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayGoalkeeperDeploymentWriter;
	FMatchPlayState State = MakeGoalkeeperWriterState();
	State.CurrentAttack.bAttackerDeploymentFinished = true;
	const FMatchPlayGoalkeeperDeploymentWriterResult First =
		FMatchPlayGoalkeeperDeploymentWriter::Deploy(
			State,
			MakeGoalkeeperWriterRequest());
	const FMatchPlayState StateAfterFirst = First.AfterState;
	const FMatchPlayGoalkeeperDeploymentWriterResult Second =
		FMatchPlayGoalkeeperDeploymentWriter::Deploy(
			StateAfterFirst,
			MakeGoalkeeperWriterRequest());
	TestTrue(TEXT("First request succeeds"), First.bSucceeded);
	TestFalse(TEXT("Second request fails"), Second.bSucceeded);
	TestEqual(TEXT("Repeat preserves AlreadyActivated priority"),
		Second.LegalityResult.ErrorCode,
		EMatchPlayGoalkeeperDeploymentErrorCode
			::GoalkeeperAlreadyActivatedThisAttack);
	TestTrue(TEXT("Repeat is atomic"),
		AreGoalkeeperWriterStatesEqual(
			Second.AfterState,
			StateAfterFirst));
	return true;
}

MATCH_PLAY_GOALKEEPER_DEPLOYMENT_WRITER_TEST(
	FMatchPlayGoalkeeperDeploymentWriterBlocksOrdinaryTest,
	"GoalkeeperPlacementBlocksOrdinaryWriter")

bool FMatchPlayGoalkeeperDeploymentWriterBlocksOrdinaryTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayGoalkeeperDeploymentWriter;
	const FMatchPlayGoalkeeperDeploymentWriterResult GoalkeeperResult =
		FMatchPlayGoalkeeperDeploymentWriter::Deploy(
			MakeGoalkeeperWriterState(),
			MakeGoalkeeperWriterRequest());
	FMatchPlayOrdinaryDeploymentRequest OrdinaryRequest;
	OrdinaryRequest.AttackSequence = ValidAttackSequence;
	OrdinaryRequest.RequestingSide = EInitialTurnOrderPlayer::PlayerA;
	OrdinaryRequest.CardId = PlayerAOutfieldCardId;
	OrdinaryRequest.SlotId = NearPlayerBSlotId;
	const FMatchPlayOrdinaryDeploymentWriterResult OrdinaryResult =
		FMatchPlayOrdinaryDeploymentWriter::Deploy(
			GoalkeeperResult.AfterState,
			OrdinaryRequest);
	TestTrue(TEXT("GK Writer succeeds"), GoalkeeperResult.bSucceeded);
	TestFalse(TEXT("Ordinary Writer cannot share GK Slot"),
		OrdinaryResult.bSuccess);
	TestEqual(TEXT("Ordinary legality reports global occupancy"),
		OrdinaryResult.LegalityResult.ErrorCode,
		EMatchPlayOrdinaryDeploymentErrorCode::SlotAlreadyOccupied);
	TestTrue(TEXT("Failed ordinary request is atomic"),
		AreGoalkeeperWriterStatesEqual(
			OrdinaryResult.AfterState,
			GoalkeeperResult.AfterState));
	return true;
}

MATCH_PLAY_GOALKEEPER_DEPLOYMENT_WRITER_TEST(
	FMatchPlayGoalkeeperDeploymentWriterLaterAttackTest,
	"LaterAttackPreservesPersistentUseAndReturnsAlreadyUsed")

bool FMatchPlayGoalkeeperDeploymentWriterLaterAttackTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayGoalkeeperDeploymentWriter;
	const FMatchPlayGoalkeeperDeploymentWriterResult First =
		FMatchPlayGoalkeeperDeploymentWriter::Deploy(
			MakeGoalkeeperWriterState(),
			MakeGoalkeeperWriterRequest());
	FMatchPlayState BetweenAttacks = First.AfterState;
	BetweenAttacks.bHasCurrentAttack = false;
	BetweenAttacks.CurrentAttack = FMatchPlayCurrentAttackState();
	++BetweenAttacks.RuntimeState.PlayerAState.UsedAttackCount;

	const FMatchPlayBeginOrdinaryAttackResult BeginResult =
		FMatchPlayBeginOrdinaryAttack::Begin(BetweenAttacks, 6);
	TestTrue(TEXT("Later attack begins"), BeginResult.bSuccess);
	const FMatchPlayFinishDeploymentResult FinishAttackerResult =
		FMatchPlayFinishDeployment::Finish(
			BeginResult.AfterState,
			BeginResult.AfterState.CurrentAttack.AttackSequence,
			EInitialTurnOrderPlayer::PlayerA);
	TestTrue(TEXT("Later attack rotates to defender"),
		FinishAttackerResult.bSuccess);

	FMatchPlayGoalkeeperDeploymentRequest LaterRequest =
		MakeGoalkeeperWriterRequest();
	LaterRequest.AttackSequence =
		FinishAttackerResult.AfterState.CurrentAttack.AttackSequence;
	const FMatchPlayGoalkeeperDeploymentWriterResult LaterResult =
		FMatchPlayGoalkeeperDeploymentWriter::Deploy(
			FinishAttackerResult.AfterState,
			LaterRequest);
	TestFalse(TEXT("Later attack cannot reuse GK"),
		LaterResult.bSucceeded);
	TestEqual(TEXT("Later attack reports match-long use"),
		LaterResult.LegalityResult.ErrorCode,
		EMatchPlayGoalkeeperDeploymentErrorCode
			::GoalkeeperAlreadyUsedThisMatch);
	TestFalse(TEXT("Later attack activation was reset"),
		FinishAttackerResult.AfterState.CurrentAttack
			.bCurrentDefenseGoalkeeperActivated);
	TestTrue(TEXT("Persistent PlayerB usage survived Begin"),
		FinishAttackerResult.AfterState.GoalkeeperUsageState
			.bPlayerBGoalkeeperCardUsed);
	TestEqual(TEXT("Later attack has no matching GK placement"),
		LaterResult.LegalityResult
			.MatchingCurrentGoalkeeperPlacementCount,
		0);
	return true;
}

#undef MATCH_PLAY_GOALKEEPER_DEPLOYMENT_WRITER_TEST

#endif
