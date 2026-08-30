#include "MatchPlayAuthoritativeSession.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "../CoreRules/MatchPlayCurrentAttackRouteStateValidator.h"

#include "Misc/AutomationTest.h"

namespace MatchPlayFullD12AuthoritativeSessionTests
{
	FMatchPlayAttackEntryRollProviderResult MakeSuccess(const int32 RawRoll)
	{
		FMatchPlayAttackEntryRollProviderResult Result;
		Result.bSuccess = true;
		Result.RawRoll = RawRoll;
		return Result;
	}

	FMatchPlayAttackEntryRollProviderResult MakeFailure()
	{
		FMatchPlayAttackEntryRollProviderResult Result;
		Result.ErrorCode =
			EMatchPlayAttackEntryRollProviderErrorCode::ProviderFailure;
		Result.ErrorMessage = TEXT("Deterministic attack-entry provider failure.");
		return Result;
	}

	FMatchPlayAttackEntrySelectionProviderResult MakeSelectionFailure()
	{
		FMatchPlayAttackEntrySelectionProviderResult Result;
		Result.ErrorCode =
			EMatchPlayAttackEntryRollProviderErrorCode::ProviderFailure;
		Result.ErrorMessage =
			TEXT("Deterministic Sending-Off selection provider failure.");
		return Result;
	}

	FMatchPlayAttackEntrySelectionProviderResult MakeSelectionSuccess(
		const int32 SelectedIndex)
	{
		FMatchPlayAttackEntrySelectionProviderResult Result;
		Result.bSuccess = true;
		Result.SelectedIndex = SelectedIndex;
		return Result;
	}

	class FQueueAttackEntryRollProvider final
		: public IMatchPlayAttackEntryRollProvider
		, public IMatchPlayRecoveryProvider
	{
	public:
		void EnqueueD12(
			const FMatchPlayAttackEntryRollProviderResult& Result)
		{
			D12Results.Add(Result);
		}

		void EnqueueD6(
			const FMatchPlayAttackEntryRollProviderResult& Result)
		{
			D6Results.Add(Result);
		}

		void EnqueueSelection(
			const FMatchPlayAttackEntrySelectionProviderResult& Result)
		{
			SelectionResults.Add(Result);
		}

		virtual FMatchPlayAttackEntryRollProviderResult RollD12(
			const EMatchPlayAttackEntryRollPurpose Purpose) override
		{
			D12Purposes.Add(Purpose);
			if (!D12Results.IsValidIndex(NextD12Index))
			{
				return MakeFailure();
			}
			return D12Results[NextD12Index++];
		}

		virtual FMatchPlayAttackEntryRollProviderResult RollD6(
			const EMatchPlayAttackEntryRollPurpose Purpose) override
		{
			D6Purposes.Add(Purpose);
			if (!D6Results.IsValidIndex(NextD6Index))
			{
				return MakeFailure();
			}
			return D6Results[NextD6Index++];
		}

		virtual FMatchPlayAttackEntrySelectionProviderResult
		SelectUniformIndex(
			const EMatchPlayAttackEntryRollPurpose Purpose,
			const int32 CandidateCount) override
		{
			SelectionPurposes.Add(Purpose);
			SelectionCandidateCounts.Add(CandidateCount);
			if (!SelectionResults.IsValidIndex(NextSelectionIndex))
			{
				return MakeSelectionFailure();
			}
			return SelectionResults[NextSelectionIndex++];
		}

		virtual FMatchPlayRecoveryProviderResult
		DrawWeightedWithoutReplacement(
			const EMatchPlayRecoveryPurpose Purpose,
			const TArray<FMatchPlayRecoveryCandidate>& OrderedCandidates,
			const int32 ReturnCount) override
		{
			++RecoveryCallCount;
			FMatchPlayRecoveryProviderResult Result;
			Result.bSuccess = true;
			Result.SelectedCandidateIndices = { 0, 1 };
			return Result;
		}

		int32 GetD12CallCount() const
		{
			return D12Purposes.Num();
		}

		int32 GetD6CallCount() const
		{
			return D6Purposes.Num();
		}

		int32 GetSelectionCallCount() const
		{
			return SelectionPurposes.Num();
		}

		int32 GetRecoveryCallCount() const
		{
			return RecoveryCallCount;
		}

		TArray<EMatchPlayAttackEntryRollPurpose> D12Purposes;
		TArray<EMatchPlayAttackEntryRollPurpose> D6Purposes;
		TArray<EMatchPlayAttackEntryRollPurpose> SelectionPurposes;
		TArray<int32> SelectionCandidateCounts;

	private:
		TArray<FMatchPlayAttackEntryRollProviderResult> D12Results;
		TArray<FMatchPlayAttackEntryRollProviderResult> D6Results;
		TArray<FMatchPlayAttackEntrySelectionProviderResult> SelectionResults;
		int32 NextD12Index = 0;
		int32 NextD6Index = 0;
		int32 NextSelectionIndex = 0;
		int32 RecoveryCallCount = 0;
	};

	FPlayerCardData MakeDeckCard(
		const FString& CardId,
		const bool bIsGoalkeeper)
	{
		FPlayerCardData Card;
		Card.CardId = FName(*CardId);
		Card.Rarity = ECardRarity::Common;
		Card.bIsGoalkeeper = bIsGoalkeeper;
		Card.PositionTypes = {
			bIsGoalkeeper
				? EPlayerPositionType::Goalkeeper
				: EPlayerPositionType::Attack
		};
		return Card;
	}

	TArray<FPlayerCardData> MakeDeck(const FString& Prefix)
	{
		TArray<FPlayerCardData> Deck;
		for (int32 Index = 0; Index < 19; ++Index)
		{
			Deck.Add(MakeDeckCard(
				FString::Printf(TEXT("%s_OUT_%02d"), *Prefix, Index),
				false));
		}
		Deck.Add(MakeDeckCard(Prefix + TEXT("_GK"), true));
		return Deck;
	}

	FMatchPlayOpeningInitializeInput MakeValidInput(
		const FString& Prefix)
	{
		FMatchPlayOpeningInitializeInput Input;
		Input.OpeningInput.PlayerADeck = MakeDeck(Prefix + TEXT("_A"));
		Input.OpeningInput.PlayerBDeck = MakeDeck(Prefix + TEXT("_B"));
		Input.OpeningInput.PlayerAAttackCountD6Roll = 1;
		Input.OpeningInput.PlayerBAttackCountD6Roll = 1;
		Input.OpeningInput.PlayerATieBreakerRoll = 6;
		Input.OpeningInput.PlayerBTieBreakerRoll = 2;

		FMatchPlayDeploymentSlotDefinition PlayerASlot;
		PlayerASlot.SlotId = FName(*(Prefix + TEXT("_SlotA")));
		PlayerASlot.NeutralSide = EMatchPlayNeutralSlotSide::NearPlayerA;
		FMatchPlayDeploymentSlotDefinition PlayerBSlot;
		PlayerBSlot.SlotId = FName(*(Prefix + TEXT("_SlotB")));
		PlayerBSlot.NeutralSide = EMatchPlayNeutralSlotSide::NearPlayerB;
		Input.DeploymentSlotCatalog.Slots = { PlayerASlot, PlayerBSlot };
		return Input;
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

	template <typename TStruct>
	bool AreStructsEqual(const TStruct& Left, const TStruct& Right)
	{
		return TStruct::StaticStruct()->CompareScriptStruct(
			&Left,
			&Right,
			0);
	}

	bool AreNonCurrentAttackFactsEqual(
		const FMatchPlayState& Left,
		const FMatchPlayState& Right)
	{
		return AreStructsEqual(Left.RuntimeState, Right.RuntimeState)
			&& AreStructsEqual(Left.CardUsageState, Right.CardUsageState)
			&& AreStructsEqual(
				Left.DeploymentSlotCatalog,
				Right.DeploymentSlotCatalog)
			&& AreStructsEqual(
				Left.CardSnapshotAuthority,
				Right.CardSnapshotAuthority)
			&& AreStructsEqual(
				Left.GoalkeeperUsageState,
				Right.GoalkeeperUsageState)
			&& AreStructsEqual(
				Left.LastRecoveryFact,
				Right.LastRecoveryFact);
	}

	int64 GetExpectedSequence(const FMatchPlayState& State)
	{
		return static_cast<int64>(
			State.RuntimeState.PlayerAState.UsedAttackCount)
			+ static_cast<int64>(
				State.RuntimeState.PlayerBState.UsedAttackCount)
			+ 1;
	}

	EInitialTurnOrderPlayer GetOtherSide(
		const EInitialTurnOrderPlayer Side)
	{
		return Side == EInitialTurnOrderPlayer::PlayerA
			? EInitialTurnOrderPlayer::PlayerB
			: EInitialTurnOrderPlayer::PlayerA;
	}

	FMatchPlayFullD12EntryRequest MakeInitialRequest(
		const FMatchPlayState& State)
	{
		FMatchPlayFullD12EntryRequest Request;
		Request.RequestingSide = State.RuntimeState.CurrentAttackingPlayer;
		Request.ExpectedAttackSequence = GetExpectedSequence(State);
		return Request;
	}

	FMatchPlaySetPieceTypeRollRequest MakeTypeRequest(
		const FMatchPlayState& State)
	{
		FMatchPlaySetPieceTypeRollRequest Request;
		Request.RequestingSide = State.RuntimeState.CurrentAttackingPlayer;
		Request.AttackSequence = State.CurrentAttack.AttackSequence;
		return Request;
	}

	bool Initialize(
		FAutomationTestBase& Test,
		FMatchPlayAuthoritativeSession& Session,
		const TCHAR* Prefix)
	{
		const FMatchPlayAuthoritativeInitializeMatchResult Result =
			Session.InitializeMatch(MakeValidInput(Prefix));
		Test.TestTrue(TEXT("Session initialization succeeds"),
			Result.OpeningResult.bSuccess);
		return Result.OpeningResult.bSuccess;
	}

	FMatchPlayState MakeReconstructedPoolState(
		FAutomationTestBase& Test,
		FQueueAttackEntryRollProvider& Provider,
		const TCHAR* Prefix,
		const int32 AvailableOutfieldCount)
	{
		FMatchPlayAuthoritativeSession SeedSession(Provider);
		if (!Initialize(Test, SeedSession, Prefix))
		{
			return FMatchPlayState();
		}
		FMatchPlayState State = SeedSession.GetStateSnapshot();
		FCardUsageState& AttackerUsage =
			State.RuntimeState.CurrentAttackingPlayer
				== EInitialTurnOrderPlayer::PlayerA
				? State.CardUsageState.PlayerACardUsageState
				: State.CardUsageState.PlayerBCardUsageState;
		TArray<FName> Outfield;
		FName Goalkeeper = NAME_None;
		for (const FPlayerCardRuleSnapshot& Snapshot :
			(State.RuntimeState.CurrentAttackingPlayer
				== EInitialTurnOrderPlayer::PlayerA
					? State.CardSnapshotAuthority.PlayerACardSnapshots.Cards
					: State.CardSnapshotAuthority.PlayerBCardSnapshots.Cards))
		{
			if (Snapshot.bIsGoalkeeper)
			{
				Goalkeeper = Snapshot.CardId;
			}
			else
			{
				Outfield.Add(Snapshot.CardId);
			}
		}
		AttackerUsage.AvailableCardIds.Reset();
		AttackerUsage.UsedCardIds.Reset();
		AttackerUsage.EjectedCardIds.Reset();
		for (int32 Index = 0; Index < Outfield.Num(); ++Index)
		{
			(Index < AvailableOutfieldCount
				? AttackerUsage.AvailableCardIds
				: AttackerUsage.UsedCardIds).Add(Outfield[Index]);
		}
		AttackerUsage.AvailableCardIds.Add(Goalkeeper);
		return State;
	}

	FMatchPlayState MakeMinimalState(
		const int32 PlayerATotal = 4,
		const int32 PlayerAUsed = 0,
		const int32 PlayerBTotal = 4,
		const int32 PlayerBUsed = 0)
	{
		FMatchPlayState State;
		State.RuntimeState.bIsInitialized = true;
		State.RuntimeState.PlayerAState.TotalAttackCount = PlayerATotal;
		State.RuntimeState.PlayerAState.UsedAttackCount = PlayerAUsed;
		State.RuntimeState.PlayerBState.TotalAttackCount = PlayerBTotal;
		State.RuntimeState.PlayerBState.UsedAttackCount = PlayerBUsed;
		State.RuntimeState.CurrentAttackingPlayer =
			EInitialTurnOrderPlayer::PlayerA;
		return State;
	}

	FMatchPlayState MakePendingRouteState(
		const EMatchPlayCurrentAttackRouteKind Route,
		const int32 RawD12)
	{
		FMatchPlayState State = MakeMinimalState();
		State.bHasCurrentAttack = true;
		State.CurrentAttack.Phase = EMatchPlayCurrentAttackPhase::RoutePending;
		State.CurrentAttack.AttackSequence = 1;
		State.CurrentAttack.ActionPoint = RawD12;
		State.CurrentAttack.RawInitialD12 = RawD12;
		State.CurrentAttack.RouteKind = Route;
		if (Route == EMatchPlayCurrentAttackRouteKind::SendingOff)
		{
			State.CurrentAttack.SendingOffRoute.Stage =
				EMatchPlaySendingOffRouteStage::AwaitingResolution;
		}
		else if (Route == EMatchPlayCurrentAttackRouteKind::SetPiece)
		{
			State.CurrentAttack.SetPieceRoute.Stage =
				EMatchPlaySetPieceRouteStage::AwaitingTypeRoll;
		}
		return State;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayFullD12CorePreconditionTest,
	"FMCodex.CoreRules.MatchPlayFullD12Entry.PreconditionsBeforeProvider",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayFullD12CorePreconditionTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayFullD12AuthoritativeSessionTests;

	FQueueAttackEntryRollProvider Provider;
	Provider.EnqueueD12(MakeSuccess(2));

	FMatchPlayState Ended = MakeMinimalState(0, 0, 0, 0);
	FMatchPlayFullD12EntryRequest EndedRequest = MakeInitialRequest(Ended);
	const FMatchPlayFullD12EntryResult EndedResult =
		FMatchPlayFullD12Entry::Enter(Ended, EndedRequest, &Provider);
	TestFalse(TEXT("Ended match is rejected"), EndedResult.bSuccess);
	TestEqual(TEXT("Ended match error"), EndedResult.ErrorCode,
		EMatchPlayFullD12EntryErrorCode::MatchAlreadyEnded);
	TestEqual(TEXT("Ended match consumes no D12"),
		Provider.GetD12CallCount(), 0);
	TestTrue(TEXT("Ended match preserves state"),
		AreStatesEqual(EndedResult.AfterState, Ended));

	FMatchPlayState Exhausted = MakeMinimalState(1, 1, 2, 0);
	FMatchPlayFullD12EntryRequest ExhaustedRequest =
		MakeInitialRequest(Exhausted);
	const FMatchPlayFullD12EntryResult ExhaustedResult =
		FMatchPlayFullD12Entry::Enter(
			Exhausted,
			ExhaustedRequest,
			&Provider);
	TestFalse(TEXT("Exhausted current attacker is rejected"),
		ExhaustedResult.bSuccess);
	TestEqual(TEXT("Exhausted attacker error"), ExhaustedResult.ErrorCode,
		EMatchPlayFullD12EntryErrorCode
			::CurrentAttackerHasNoRemainingAttackOpportunity);
	TestEqual(TEXT("Exhausted attacker consumes no D12"),
		Provider.GetD12CallCount(), 0);
	TestTrue(TEXT("Exhausted attacker preserves state"),
		AreStatesEqual(ExhaustedResult.AfterState, Exhausted));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayCurrentAttackRouteStateValidatorTest,
	"FMCodex.CoreRules.MatchPlayCurrentAttackRouteStateValidator.CanonicalAndInvalidCombinations",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayCurrentAttackRouteStateValidatorTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayFullD12AuthoritativeSessionTests;
	using EError = EMatchPlayCurrentAttackRouteStateValidationErrorCode;

	auto ExpectError = [this](
		const TCHAR* Label,
		const FMatchPlayState& State,
		const EError Error)
	{
		const FMatchPlayCurrentAttackRouteStateValidationResult Result =
			FMatchPlayCurrentAttackRouteStateValidator::Validate(State);
		TestFalse(*FString::Printf(TEXT("%s is invalid"), Label),
			Result.bIsCanonical);
		TestEqual(*FString::Printf(TEXT("%s error"), Label),
			Result.ErrorCode, Error);
	};

	const FMatchPlayState Sending = MakePendingRouteState(
		EMatchPlayCurrentAttackRouteKind::SendingOff,
		1);
	const FMatchPlayState Awaiting = MakePendingRouteState(
		EMatchPlayCurrentAttackRouteKind::SetPiece,
		9);
	TestTrue(TEXT("Canonical Sending-Off state validates"),
		FMatchPlayCurrentAttackRouteStateValidator::Validate(Sending)
			.bIsCanonical);
	TestTrue(TEXT("Canonical Set Piece awaiting state validates"),
		FMatchPlayCurrentAttackRouteStateValidator::Validate(Awaiting)
			.bIsCanonical);

	FMatchPlayState Invalid = MakePendingRouteState(
		EMatchPlayCurrentAttackRouteKind::Ordinary,
		1);
	ExpectError(TEXT("Ordinary with D12 1"), Invalid,
		EError::OrdinaryD12Mismatch);
	Invalid = Sending;
	Invalid.CurrentAttack.RawInitialD12 = 2;
	Invalid.CurrentAttack.ActionPoint = 2;
	ExpectError(TEXT("Sending-Off with D12 2"), Invalid,
		EError::SendingOffD12Mismatch);
	Invalid = Sending;
	Invalid.CurrentAttack.LifecycleState =
		EMatchPlayCurrentAttackLifecycleState::TerminalPendingAdvance;
	ExpectError(TEXT("Sending-Off cannot fabricate terminal state"), Invalid,
		EError::WrongRouteLifecycle);
	Invalid = Awaiting;
	Invalid.CurrentAttack.RawInitialD12 = 8;
	Invalid.CurrentAttack.ActionPoint = 8;
	ExpectError(TEXT("Set Piece with D12 8"), Invalid,
		EError::SetPieceD12Mismatch);
	Invalid = Awaiting;
	Invalid.CurrentAttack.SetPieceRoute.bHasTypeRoll = true;
	Invalid.CurrentAttack.SetPieceRoute.RawTypeD6 = 1;
	Invalid.CurrentAttack.SetPieceRoute.SelectedType =
		ESetPieceSelectedType::Corner;
	ExpectError(TEXT("Awaiting type with selected type"), Invalid,
		EError::AwaitingTypeHasResolvedPayload);
	Invalid = Awaiting;
	Invalid.CurrentAttack.SetPieceRoute.Stage =
		EMatchPlaySetPieceRouteStage::TypeResolved;
	ExpectError(TEXT("Resolved type without roll"), Invalid,
		EError::TypeResolvedMissingRoll);
	Invalid.CurrentAttack.SetPieceRoute.bHasTypeRoll = true;
	Invalid.CurrentAttack.SetPieceRoute.RawTypeD6 = 7;
	Invalid.CurrentAttack.SetPieceRoute.SelectedType =
		ESetPieceSelectedType::Penalty;
	ExpectError(TEXT("Resolved type with D6 7"), Invalid,
		EError::InvalidSetPieceTypeRoll);
	Invalid = Awaiting;
	Invalid.CurrentAttack.AttackSequence = 0;
	ExpectError(TEXT("Nonpositive sequence"), Invalid,
		EError::InvalidAttackSequence);
	Invalid = Awaiting;
	Invalid.CurrentAttack.SetPieceRoute.Stage =
		EMatchPlaySetPieceRouteStage::TypeResolved;
	Invalid.CurrentAttack.SetPieceRoute.bHasTypeRoll = true;
	Invalid.CurrentAttack.SetPieceRoute.RawTypeD6 = 6;
	Invalid.CurrentAttack.SetPieceRoute.SelectedType =
		ESetPieceSelectedType::Corner;
	ExpectError(TEXT("Mismatched canonical type mapping"), Invalid,
		EError::SetPieceTypeMappingMismatch);
	Invalid = FMatchPlayBeginOrdinaryAttack::Begin(
		MakeMinimalState(), 2).AfterState;
	Invalid.CurrentAttack.SendingOffRoute.Stage =
		EMatchPlaySendingOffRouteStage::AwaitingResolution;
	ExpectError(TEXT("Ordinary with Sending-Off payload"), Invalid,
		EError::UnexpectedSendingOffPayload);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAuthoritativeFullD12RouteTest,
	"FMCodex.MatchPlayRuntime.AuthoritativeSession.FullD12RoutesAndOrdinaryReuse",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAuthoritativeFullD12RouteTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayFullD12AuthoritativeSessionTests;

	for (const int32 RawD12 : { 1, 2, 8, 9, 12 })
	{
		FQueueAttackEntryRollProvider Provider;
		Provider.EnqueueD12(MakeSuccess(RawD12));
		FMatchPlayAuthoritativeSession Session(Provider);
		if (!Initialize(*this, Session,
			*FString::Printf(TEXT("FullD12_%d"), RawD12)))
		{
			continue;
		}

		const FMatchPlayState Before = Session.GetStateSnapshot();
		const FMatchPlayFullD12EntryRequest Request =
			MakeInitialRequest(Before);
		const FMatchPlayAuthoritativeRequestInitialActionPointRollResult
			Result = Session.RequestInitialActionPointRoll(Request);
		const FMatchPlayState After = Session.GetStateSnapshot();
		TestTrue(*FString::Printf(TEXT("D12 %d accepted"), RawD12),
			Result.RuntimeEnvelope.bAccepted);
		TestTrue(*FString::Printf(TEXT("D12 %d succeeds"), RawD12),
			Result.EntryResult.bSuccess);
		TestTrue(*FString::Printf(TEXT("D12 %d adopts state"), RawD12),
			Result.RuntimeEnvelope.bStateAdvanced);
		TestEqual(*FString::Printf(TEXT("D12 %d consumed once"), RawD12),
			Provider.GetD12CallCount(), 1);
		TestEqual(*FString::Printf(TEXT("D12 %d consumes no type D6"), RawD12),
			Provider.GetD6CallCount(), 0);
		TestEqual(TEXT("D12 semantic purpose is InitialActionPoint"),
			Provider.D12Purposes[0],
			EMatchPlayAttackEntryRollPurpose::InitialActionPoint);
		TestTrue(TEXT("Non-current-attack authority is preserved"),
			AreNonCurrentAttackFactsEqual(Before, After));
		TestTrue(TEXT("Stored route state validates"),
			FMatchPlayCurrentAttackRouteStateValidator::Validate(After)
				.bIsCanonical);
		TestEqual(TEXT("AttackSequence is preserved"),
			After.CurrentAttack.AttackSequence,
			Request.ExpectedAttackSequence);
		TestEqual(TEXT("Raw D12 is stored"),
			After.CurrentAttack.RawInitialD12, RawD12);

		if (RawD12 >= 2 && RawD12 <= 8)
		{
			const FMatchPlayBeginOrdinaryAttackResult Expected =
				FMatchPlayBeginOrdinaryAttack::Begin(Before, RawD12);
			TestEqual(TEXT("Ordinary route is explicit"),
				After.CurrentAttack.RouteKind,
				EMatchPlayCurrentAttackRouteKind::Ordinary);
			TestTrue(TEXT("Full D12 ordinary state equals canonical Begin"),
				AreStatesEqual(After, Expected.AfterState));
			TestEqual(TEXT("Ordinary legal deployment side is preserved"),
				After.CurrentAttack.CurrentLegalDeploymentSide,
				Before.RuntimeState.CurrentAttackingPlayer);
			TestTrue(TEXT("Ordinary placements begin empty"),
				After.CurrentAttack.DeploymentPlacements.IsEmpty());
		}
		else
		{
			TestEqual(TEXT("Pending route does not begin deployment"),
				After.CurrentAttack.Phase,
				EMatchPlayCurrentAttackPhase::RoutePending);
			TestEqual(TEXT("Pending route has no deployment owner"),
				After.CurrentAttack.CurrentLegalDeploymentSide,
				EInitialTurnOrderPlayer::None);
			TestTrue(TEXT("Pending route has no placements"),
				After.CurrentAttack.DeploymentPlacements.IsEmpty());
			if (RawD12 == 1)
			{
				TestEqual(TEXT("D12 1 routes to Sending-Off"),
					After.CurrentAttack.RouteKind,
					EMatchPlayCurrentAttackRouteKind::SendingOff);
				TestEqual(TEXT("Sending-Off awaits future resolution"),
					After.CurrentAttack.SendingOffRoute.Stage,
					EMatchPlaySendingOffRouteStage::AwaitingResolution);
			}
			else
			{
				TestEqual(TEXT("D12 9-12 routes to Set Piece"),
					After.CurrentAttack.RouteKind,
					EMatchPlayCurrentAttackRouteKind::SetPiece);
				TestEqual(TEXT("Set Piece awaits type roll"),
					After.CurrentAttack.SetPieceRoute.Stage,
					EMatchPlaySetPieceRouteStage::AwaitingTypeRoll);
			}
		}
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAuthoritativeFullD12RejectionTest,
	"FMCodex.MatchPlayRuntime.AuthoritativeSession.FullD12AtomicRejectionsAndRetry",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAuthoritativeFullD12RejectionTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayFullD12AuthoritativeSessionTests;

	FQueueAttackEntryRollProvider Provider;
	Provider.EnqueueD12(MakeFailure());
	Provider.EnqueueD12(MakeSuccess(9));
	FMatchPlayAuthoritativeSession Session(Provider);
	if (!Initialize(*this, Session, TEXT("FullD12Rejection")))
	{
		return false;
	}
	const FMatchPlayState Before = Session.GetStateSnapshot();
	const FMatchPlayFullD12EntryRequest Valid = MakeInitialRequest(Before);

	FMatchPlayFullD12EntryRequest WrongSide = Valid;
	WrongSide.RequestingSide = GetOtherSide(Valid.RequestingSide);
	const auto WrongSideResult =
		Session.RequestInitialActionPointRoll(WrongSide);
	TestEqual(TEXT("Wrong side domain error"),
		WrongSideResult.EntryResult.ErrorCode,
		EMatchPlayFullD12EntryErrorCode::RequestingSideNotCurrentAttacker);
	TestEqual(TEXT("Wrong side consumes zero D12"),
		Provider.GetD12CallCount(), 0);
	TestTrue(TEXT("Wrong side preserves snapshot"),
		AreStatesEqual(Session.GetStateSnapshot(), Before));

	FMatchPlayFullD12EntryRequest Stale = Valid;
	++Stale.ExpectedAttackSequence;
	const auto StaleResult = Session.RequestInitialActionPointRoll(Stale);
	TestEqual(TEXT("Stale request domain error"),
		StaleResult.EntryResult.ErrorCode,
		EMatchPlayFullD12EntryErrorCode::AttackSequenceMismatch);
	TestEqual(TEXT("Stale request consumes zero D12"),
		Provider.GetD12CallCount(), 0);
	TestTrue(TEXT("Stale request preserves snapshot"),
		AreStatesEqual(Session.GetStateSnapshot(), Before));

	const auto Failed = Session.RequestInitialActionPointRoll(Valid);
	TestFalse(TEXT("Provider failure fails domain"),
		Failed.EntryResult.bSuccess);
	TestEqual(TEXT("Provider failure consumes one D12"),
		Provider.GetD12CallCount(), 1);
	TestTrue(TEXT("Provider failure preserves whole state"),
		AreStatesEqual(Session.GetStateSnapshot(), Before));

	const auto Retry = Session.RequestInitialActionPointRoll(Valid);
	TestTrue(TEXT("Retry succeeds"), Retry.EntryResult.bSuccess);
	TestEqual(TEXT("Retry consumes second D12"),
		Provider.GetD12CallCount(), 2);
	const FMatchPlayState AfterSuccess = Session.GetStateSnapshot();
	const auto Duplicate = Session.RequestInitialActionPointRoll(Valid);
	TestEqual(TEXT("Duplicate is blocked by current attack"),
		Duplicate.EntryResult.ErrorCode,
		EMatchPlayFullD12EntryErrorCode::CurrentAttackAlreadyActive);
	TestEqual(TEXT("Duplicate consumes no additional D12"),
		Provider.GetD12CallCount(), 2);
	TestTrue(TEXT("Duplicate preserves successful route"),
		AreStatesEqual(Session.GetStateSnapshot(), AfterSuccess));

	FQueueAttackEntryRollProvider MalformedProvider;
	MalformedProvider.EnqueueD12(MakeSuccess(13));
	FMatchPlayAuthoritativeSession MalformedSession(MalformedProvider);
	Initialize(*this, MalformedSession, TEXT("FullD12Malformed"));
	const FMatchPlayState MalformedBefore =
		MalformedSession.GetStateSnapshot();
	const auto Malformed = MalformedSession.RequestInitialActionPointRoll(
		MakeInitialRequest(MalformedBefore));
	TestEqual(TEXT("D12 13 is malformed"),
		Malformed.EntryResult.ErrorCode,
		EMatchPlayFullD12EntryErrorCode::MalformedProviderResult);
	TestTrue(TEXT("Malformed D12 preserves whole state"),
		AreStatesEqual(MalformedSession.GetStateSnapshot(), MalformedBefore));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAuthoritativeSetPieceTypeMappingTest,
	"FMCodex.MatchPlayRuntime.AuthoritativeSession.SetPieceTypeAllD6Mappings",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAuthoritativeSetPieceTypeMappingTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayFullD12AuthoritativeSessionTests;
	const ESetPieceSelectedType ExpectedTypes[] = {
		ESetPieceSelectedType::Corner,
		ESetPieceSelectedType::Corner,
		ESetPieceSelectedType::LongFreeKick,
		ESetPieceSelectedType::LongFreeKick,
		ESetPieceSelectedType::ShortFreeKick,
		ESetPieceSelectedType::Penalty
	};

	for (int32 RawD6 = 1; RawD6 <= 6; ++RawD6)
	{
		FQueueAttackEntryRollProvider Provider;
		Provider.EnqueueD12(MakeSuccess(9));
		Provider.EnqueueD6(MakeSuccess(RawD6));
		FMatchPlayAuthoritativeSession Session(Provider);
		if (!Initialize(*this, Session,
			*FString::Printf(TEXT("SetPieceType_%d"), RawD6)))
		{
			continue;
		}
		const auto Entry = Session.RequestInitialActionPointRoll(
			MakeInitialRequest(Session.GetStateSnapshot()));
		TestTrue(TEXT("Set Piece entry succeeds"), Entry.EntryResult.bSuccess);
		const FMatchPlayState BeforeType = Session.GetStateSnapshot();
		const auto Type = Session.RequestSetPieceTypeRoll(
			MakeTypeRequest(BeforeType));
		const FMatchPlayState AfterType = Session.GetStateSnapshot();
		TestTrue(*FString::Printf(TEXT("D6 %d type succeeds"), RawD6),
			Type.TypeRollResult.bSuccess);
		TestEqual(TEXT("Initial D12 consumed once"),
			Provider.GetD12CallCount(), 1);
		TestEqual(TEXT("Type D6 consumed once"),
			Provider.GetD6CallCount(), 1);
		TestEqual(TEXT("Type semantic purpose is SetPieceType"),
			Provider.D6Purposes[0],
			EMatchPlayAttackEntryRollPurpose::SetPieceType);
		TestEqual(TEXT("Raw type D6 is stored"),
			AfterType.CurrentAttack.SetPieceRoute.RawTypeD6, RawD6);
		TestEqual(TEXT("Canonical type mapping is stored"),
			AfterType.CurrentAttack.SetPieceRoute.SelectedType,
			ExpectedTypes[RawD6 - 1]);
		TestEqual(TEXT("Type stage advances"),
			AfterType.CurrentAttack.SetPieceRoute.Stage,
			EMatchPlaySetPieceRouteStage::TypeResolved);
		TestEqual(TEXT("AttackSequence does not change"),
			AfterType.CurrentAttack.AttackSequence,
			BeforeType.CurrentAttack.AttackSequence);
		TestEqual(TEXT("Raw initial D12 is preserved"),
			AfterType.CurrentAttack.RawInitialD12,
			BeforeType.CurrentAttack.RawInitialD12);
		const int32 ActiveConcretePayloadCount =
			(AfterType.CurrentAttack.SetPieceRoute.ShortFreeKick.Stage
				!= EMatchPlaySetPieceCarrierRouteStage::None)
			+ (AfterType.CurrentAttack.SetPieceRoute.LongFreeKick.Stage
				!= EMatchPlaySetPieceCarrierRouteStage::None)
			+ (AfterType.CurrentAttack.SetPieceRoute.Penalty.Stage
				!= EMatchPlaySetPieceCarrierRouteStage::None)
			+ (AfterType.CurrentAttack.SetPieceRoute.Corner.Stage
				!= EMatchPlaySetPieceCornerRouteStage::None);
		TestEqual(TEXT("Exactly one concrete payload is initialized atomically"),
			ActiveConcretePayloadCount, 1);
		switch (ExpectedTypes[RawD6 - 1])
		{
		case ESetPieceSelectedType::ShortFreeKick:
			TestEqual(TEXT("Short FK awaits Carrier"),
				AfterType.CurrentAttack.SetPieceRoute.ShortFreeKick.Stage,
				EMatchPlaySetPieceCarrierRouteStage::AwaitingCarrier);
			break;
		case ESetPieceSelectedType::LongFreeKick:
			TestEqual(TEXT("Long FK awaits Carrier"),
				AfterType.CurrentAttack.SetPieceRoute.LongFreeKick.Stage,
				EMatchPlaySetPieceCarrierRouteStage::AwaitingCarrier);
			break;
		case ESetPieceSelectedType::Penalty:
			TestEqual(TEXT("Penalty awaits Carrier"),
				AfterType.CurrentAttack.SetPieceRoute.Penalty.Stage,
				EMatchPlaySetPieceCarrierRouteStage::AwaitingCarrier);
			break;
		case ESetPieceSelectedType::Corner:
			TestEqual(TEXT("Corner awaits attacker nominations"),
				AfterType.CurrentAttack.SetPieceRoute.Corner.Stage,
				EMatchPlaySetPieceCornerRouteStage
					::AwaitingAttackerNominations);
			break;
		default:
			AddError(TEXT("Unexpected Set Piece type in concrete-entry test."));
			break;
		}
		TestEqual(TEXT("Concrete entry has no ordinary deployment"),
			AfterType.CurrentAttack.DeploymentPlacements.Num(), 0);
		TestEqual(TEXT("Concrete entry remains non-terminal"),
			AfterType.CurrentAttack.LifecycleState,
			EMatchPlayCurrentAttackLifecycleState::Active);
		TestTrue(TEXT("Type roll preserves non-current authority"),
			AreNonCurrentAttackFactsEqual(BeforeType, AfterType));
		TestTrue(TEXT("Resolved type state validates"),
			FMatchPlayCurrentAttackRouteStateValidator::Validate(AfterType)
				.bIsCanonical);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAuthoritativeSetPieceTypeRejectionTest,
	"FMCodex.MatchPlayRuntime.AuthoritativeSession.SetPieceTypeAtomicRejectionsAndRetry",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAuthoritativeSetPieceTypeRejectionTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayFullD12AuthoritativeSessionTests;

	FQueueAttackEntryRollProvider Provider;
	Provider.EnqueueD12(MakeSuccess(12));
	Provider.EnqueueD6(MakeFailure());
	Provider.EnqueueD6(MakeSuccess(6));
	FMatchPlayAuthoritativeSession Session(Provider);
	if (!Initialize(*this, Session, TEXT("SetPieceTypeRejection")))
	{
		return false;
	}
	Session.RequestInitialActionPointRoll(
		MakeInitialRequest(Session.GetStateSnapshot()));
	const FMatchPlayState Awaiting = Session.GetStateSnapshot();
	const FMatchPlaySetPieceTypeRollRequest Valid = MakeTypeRequest(Awaiting);

	FMatchPlaySetPieceTypeRollRequest WrongSide = Valid;
	WrongSide.RequestingSide = GetOtherSide(Valid.RequestingSide);
	const auto WrongSideResult = Session.RequestSetPieceTypeRoll(WrongSide);
	TestEqual(TEXT("Wrong-side type error"),
		WrongSideResult.TypeRollResult.ErrorCode,
		EMatchPlaySetPieceTypeRollErrorCode::RequestingSideNotCurrentAttacker);
	TestEqual(TEXT("Wrong-side type consumes zero D6"),
		Provider.GetD6CallCount(), 0);

	FMatchPlaySetPieceTypeRollRequest Stale = Valid;
	++Stale.AttackSequence;
	const auto StaleResult = Session.RequestSetPieceTypeRoll(Stale);
	TestEqual(TEXT("Stale type error"),
		StaleResult.TypeRollResult.ErrorCode,
		EMatchPlaySetPieceTypeRollErrorCode::AttackSequenceMismatch);
	TestEqual(TEXT("Stale type consumes zero D6"),
		Provider.GetD6CallCount(), 0);
	TestTrue(TEXT("Pre-provider rejections preserve awaiting state"),
		AreStatesEqual(Session.GetStateSnapshot(), Awaiting));

	const auto Failed = Session.RequestSetPieceTypeRoll(Valid);
	TestEqual(TEXT("Type provider failure error"),
		Failed.TypeRollResult.ErrorCode,
		EMatchPlaySetPieceTypeRollErrorCode::ProviderFailure);
	TestEqual(TEXT("Failed provider consumes one D6"),
		Provider.GetD6CallCount(), 1);
	TestEqual(TEXT("Failed type does not reroll original D12"),
		Provider.GetD12CallCount(), 1);
	TestTrue(TEXT("Failed type preserves awaiting state"),
		AreStatesEqual(Session.GetStateSnapshot(), Awaiting));

	const auto Retry = Session.RequestSetPieceTypeRoll(Valid);
	TestTrue(TEXT("Type retry succeeds"), Retry.TypeRollResult.bSuccess);
	TestEqual(TEXT("Type retry consumes second D6"),
		Provider.GetD6CallCount(), 2);
	TestEqual(TEXT("Type retry does not reroll D12"),
		Provider.GetD12CallCount(), 1);
	const FMatchPlayState Resolved = Session.GetStateSnapshot();
	const auto Duplicate = Session.RequestSetPieceTypeRoll(Valid);
	TestEqual(TEXT("Duplicate type has wrong-stage error"),
		Duplicate.TypeRollResult.ErrorCode,
		EMatchPlaySetPieceTypeRollErrorCode::WrongStage);
	TestEqual(TEXT("Duplicate type consumes no additional D6"),
		Provider.GetD6CallCount(), 2);
	TestTrue(TEXT("Duplicate type preserves resolved state"),
		AreStatesEqual(Session.GetStateSnapshot(), Resolved));

	for (const int32 RawD12 : { 1, 2 })
	{
		FQueueAttackEntryRollProvider WrongRouteProvider;
		WrongRouteProvider.EnqueueD12(MakeSuccess(RawD12));
		FMatchPlayAuthoritativeSession WrongRouteSession(
			WrongRouteProvider);
		Initialize(*this, WrongRouteSession,
			*FString::Printf(TEXT("WrongRoute_%d"), RawD12));
		WrongRouteSession.RequestInitialActionPointRoll(
			MakeInitialRequest(WrongRouteSession.GetStateSnapshot()));
		const FMatchPlayState WrongRouteState =
			WrongRouteSession.GetStateSnapshot();
		const auto WrongRoute = WrongRouteSession.RequestSetPieceTypeRoll(
			MakeTypeRequest(WrongRouteState));
		TestEqual(TEXT("Non-Set-Piece route is rejected"),
			WrongRoute.TypeRollResult.ErrorCode,
			EMatchPlaySetPieceTypeRollErrorCode::WrongRoute);
		TestEqual(TEXT("Wrong route consumes zero type D6"),
			WrongRouteProvider.GetD6CallCount(), 0);
	}

	FQueueAttackEntryRollProvider MalformedProvider;
	MalformedProvider.EnqueueD12(MakeSuccess(9));
	MalformedProvider.EnqueueD6(MakeSuccess(7));
	FMatchPlayAuthoritativeSession MalformedSession(MalformedProvider);
	Initialize(*this, MalformedSession, TEXT("SetPieceTypeMalformed"));
	MalformedSession.RequestInitialActionPointRoll(
		MakeInitialRequest(MalformedSession.GetStateSnapshot()));
	const FMatchPlayState MalformedBefore =
		MalformedSession.GetStateSnapshot();
	const auto Malformed = MalformedSession.RequestSetPieceTypeRoll(
		MakeTypeRequest(MalformedBefore));
	TestEqual(TEXT("D6 7 is malformed"),
		Malformed.TypeRollResult.ErrorCode,
		EMatchPlaySetPieceTypeRollErrorCode::MalformedProviderResult);
	TestTrue(TEXT("Malformed D6 preserves AwaitingTypeRoll"),
		AreStatesEqual(MalformedSession.GetStateSnapshot(), MalformedBefore));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAuthoritativeSendingOffLifecycleTest,
	"FMCodex.MatchPlayRuntime.AuthoritativeSession.SendingOffLifecycleAndRetry",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAuthoritativeSendingOffLifecycleTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayFullD12AuthoritativeSessionTests;

	for (const int32 PoolSize : { 0, 1 })
	{
		FQueueAttackEntryRollProvider Provider;
		Provider.EnqueueD12(MakeSuccess(1));
		FMatchPlayState Reconstructed = MakeReconstructedPoolState(
			*this,
			Provider,
			*FString::Printf(TEXT("SendingOffPool%d"), PoolSize),
			PoolSize);
		FMatchPlayAuthoritativeSession Session(
			Reconstructed, Provider, Provider);
		const EInitialTurnOrderPlayer Attacker =
			Reconstructed.RuntimeState.CurrentAttackingPlayer;
		const FMatchPlayFullD12EntryRequest EntryRequest =
			MakeInitialRequest(Session.GetStateSnapshot());
		const auto Entry = Session.RequestInitialActionPointRoll(EntryRequest);
		TestTrue(TEXT("Pool0/1 D12=1 entry succeeds"),
			Entry.EntryResult.bSuccess);
		const int64 Sequence =
			Session.GetStateSnapshot().CurrentAttack.AttackSequence;
		FMatchPlaySendingOffResolutionRequest ResolveRequest;
		ResolveRequest.AttackSequence = Sequence;
		const auto Resolved = Session.ResolveSendingOff(ResolveRequest);
		const FMatchPlayState Terminal = Session.GetStateSnapshot();
		TestTrue(TEXT("Pool0/1 AP1 resolves through real Session"),
			Resolved.ResolutionResult.bSuccess);
		TestEqual(TEXT("Pool0/1 D12 is consumed exactly once"),
			Provider.GetD12CallCount(), 1);
		TestEqual(TEXT("Pool0/1 selection RNG is not called"),
			Provider.GetSelectionCallCount(), 0);
		TestEqual(TEXT("Pool0/1 terminal lifecycle is persisted"),
			Terminal.CurrentAttack.LifecycleState,
			EMatchPlayCurrentAttackLifecycleState::TerminalPendingAdvance);
		TestEqual(TEXT("Pool0/1 raw D12 reconstructs"),
			Terminal.CurrentAttack.RawInitialD12, 1);
		TestEqual(TEXT("Pool0/1 same sequence reconstructs"),
			Terminal.CurrentAttack.AttackSequence, Sequence);
		TestEqual(TEXT("Pool0/1 explicit selection fact"),
			Terminal.CurrentAttack.SendingOffRoute.SelectionOutcome,
			PoolSize == 0
				? EMatchPlaySendingOffSelectionOutcome::NoEligibleCandidate
				: EMatchPlaySendingOffSelectionOutcome::CardEjected);
		if (PoolSize == 1)
		{
			const FName Ejected =
				Terminal.CurrentAttack.SendingOffRoute.EjectedCardId;
			const FCardUsageState& TerminalAttackerUsage =
				Attacker == EInitialTurnOrderPlayer::PlayerA
					? Terminal.CardUsageState.PlayerACardUsageState
					: Terminal.CardUsageState.PlayerBCardUsageState;
			TestTrue(TEXT("Pool1 terminal snapshot contains Ejected identity"),
				TerminalAttackerUsage
					.EjectedCardIds.Contains(Ejected));
			TestFalse(TEXT("Pool1 Ejected identity is not Used"),
				TerminalAttackerUsage
					.UsedCardIds.Contains(Ejected));
		}

		FMatchPlayAuthoritativeAdvanceAfterTerminalRequest AdvanceRequest;
		AdvanceRequest.AttackSequence = Sequence;
		AdvanceRequest.RequestingSide =
			Terminal.RuntimeState.CurrentAttackingPlayer;
		const auto Advanced = Session.AdvanceAfterTerminal(AdvanceRequest);
		TestTrue(TEXT("Pool0/1 shared advance succeeds"),
			Advanced.CompletionResult.bSuccess);
		const FMatchPlayState AdvancedState = Session.GetStateSnapshot();
		TestEqual(TEXT("Pool0/1 shared advance consumes one opportunity"),
			Attacker == EInitialTurnOrderPlayer::PlayerA
				? AdvancedState.RuntimeState.PlayerAState.UsedAttackCount
				: AdvancedState.RuntimeState.PlayerBState.UsedAttackCount,
			1);
	}

	FQueueAttackEntryRollProvider RetryProvider;
	RetryProvider.EnqueueD12(MakeSuccess(1));
	RetryProvider.EnqueueSelection(MakeSelectionFailure());
	RetryProvider.EnqueueSelection(MakeSelectionSuccess(1));
	FMatchPlayState RetryState = MakeReconstructedPoolState(
		*this,
		RetryProvider,
		TEXT("SendingOffRetry"),
		3);
	FMatchPlayAuthoritativeSession RetrySession(
		RetryState, RetryProvider, RetryProvider);
	const EInitialTurnOrderPlayer RetryAttacker =
		RetryState.RuntimeState.CurrentAttackingPlayer;
	const auto Entry = RetrySession.RequestInitialActionPointRoll(
		MakeInitialRequest(RetrySession.GetStateSnapshot()));
	TestTrue(TEXT("Retry fixture accepts D12=1"), Entry.EntryResult.bSuccess);
	const FMatchPlayState Awaiting = RetrySession.GetStateSnapshot();
	FMatchPlaySendingOffResolutionRequest ResolveRequest;
	ResolveRequest.AttackSequence = Awaiting.CurrentAttack.AttackSequence;
	const auto Failed = RetrySession.ResolveSendingOff(ResolveRequest);
	TestEqual(TEXT("AP1 provider failure is retryable domain failure"),
		Failed.ResolutionResult.ErrorCode,
		EMatchPlaySendingOffResolutionErrorCode::SelectionProviderFailure);
	TestEqual(TEXT("Failure consumed initial D12 once"),
		RetryProvider.GetD12CallCount(), 1);
	TestEqual(TEXT("Failure consumed AP1 selection once"),
		RetryProvider.GetSelectionCallCount(), 1);
	TestTrue(TEXT("Failure preserves accepted D12 pending state"),
		AreStatesEqual(RetrySession.GetStateSnapshot(), Awaiting));

	const auto Retry = RetrySession.ResolveSendingOff(ResolveRequest);
	const FMatchPlayState Terminal = RetrySession.GetStateSnapshot();
	TestTrue(TEXT("AP1-only retry succeeds"),
		Retry.ResolutionResult.bSuccess);
	TestEqual(TEXT("Retry D12 provider delta is zero"),
		RetryProvider.GetD12CallCount(), 1);
	TestEqual(TEXT("Retry calls AP1 selection exactly once more"),
		RetryProvider.GetSelectionCallCount(), 2);
	TestEqual(TEXT("Retry keeps accepted RawInitialD12"),
		Terminal.CurrentAttack.RawInitialD12, 1);
	TestEqual(TEXT("Retry keeps accepted AttackSequence"),
		Terminal.CurrentAttack.AttackSequence,
		ResolveRequest.AttackSequence);
	TestEqual(TEXT("Retry result is NoGoal"),
		Terminal.CurrentAttack.SendingOffRoute.GameplayOutcome,
		EMatchPlaySendingOffGameplayOutcome::NoGoal);
	TestEqual(TEXT("Retry does not change score"),
		Terminal.RuntimeState.PlayerAState.Score,
		Awaiting.RuntimeState.PlayerAState.Score);
	TestEqual(TEXT("Retry does not consume opportunity before advance"),
		RetryAttacker == EInitialTurnOrderPlayer::PlayerA
			? Terminal.RuntimeState.PlayerAState.UsedAttackCount
			: Terminal.RuntimeState.PlayerBState.UsedAttackCount,
		RetryAttacker == EInitialTurnOrderPlayer::PlayerA
			? Awaiting.RuntimeState.PlayerAState.UsedAttackCount
			: Awaiting.RuntimeState.PlayerBState.UsedAttackCount);

	const auto Duplicate = RetrySession.ResolveSendingOff(ResolveRequest);
	TestEqual(TEXT("Duplicate AP1 is terminal-gated"),
		Duplicate.RuntimeEnvelope.RuntimeFailureCode,
		EMatchPlayAuthoritativeRuntimeFailureCode::TerminalAdvanceRequired);
	TestEqual(TEXT("Duplicate AP1 consumes no further selection RNG"),
		RetryProvider.GetSelectionCallCount(), 2);
	TestTrue(TEXT("Duplicate AP1 preserves terminal snapshot"),
		AreStatesEqual(RetrySession.GetStateSnapshot(), Terminal));

	FMatchPlayAuthoritativeAdvanceAfterTerminalRequest AdvanceRequest;
	AdvanceRequest.AttackSequence = ResolveRequest.AttackSequence;
	AdvanceRequest.RequestingSide = RetryAttacker;
	const auto Advanced = RetrySession.AdvanceAfterTerminal(AdvanceRequest);
	const FMatchPlayState PostAdvance = RetrySession.GetStateSnapshot();
	TestTrue(TEXT("Session AP1 advance succeeds"),
		Advanced.CompletionResult.bSuccess);
	TestFalse(TEXT("Session AP1 advance clears CurrentAttack"),
		PostAdvance.bHasCurrentAttack);
	TestEqual(TEXT("Session AP1 advance consumes one opportunity"),
		RetryAttacker == EInitialTurnOrderPlayer::PlayerA
			? PostAdvance.RuntimeState.PlayerAState.UsedAttackCount
			: PostAdvance.RuntimeState.PlayerBState.UsedAttackCount,
		1);
	const FCardUsageState& PostAdvanceAttackerUsage =
		RetryAttacker == EInitialTurnOrderPlayer::PlayerA
			? PostAdvance.CardUsageState.PlayerACardUsageState
			: PostAdvance.CardUsageState.PlayerBCardUsageState;
	TestTrue(TEXT("Post-advance reconstruction preserves Ejected identity"),
		PostAdvanceAttackerUsage.EjectedCardIds.Contains(
			Terminal.CurrentAttack.SendingOffRoute.EjectedCardId));

	const auto DuplicateAdvance = RetrySession.AdvanceAfterTerminal(
		AdvanceRequest);
	TestFalse(TEXT("Duplicate Session advance is rejected"),
		DuplicateAdvance.CompletionResult.bSuccess);
	const FMatchPlayState AfterDuplicateAdvance =
		RetrySession.GetStateSnapshot();
	TestEqual(TEXT("Duplicate Session advance does not consume twice"),
		RetryAttacker == EInitialTurnOrderPlayer::PlayerA
			? AfterDuplicateAdvance.RuntimeState.PlayerAState.UsedAttackCount
			: AfterDuplicateAdvance.RuntimeState.PlayerBState.UsedAttackCount,
		1);

	FQueueAttackEntryRollProvider FinalProvider;
	FinalProvider.EnqueueD12(MakeSuccess(1));
	FinalProvider.EnqueueSelection(MakeSelectionSuccess(0));
	FMatchPlayState FinalState = MakeReconstructedPoolState(
		*this,
		FinalProvider,
		TEXT("SendingOffFinal"),
		2);
	const EInitialTurnOrderPlayer FinalAttacker =
		FinalState.RuntimeState.CurrentAttackingPlayer;
	FinalState.RuntimeState.PlayerAState.TotalAttackCount =
		FinalAttacker == EInitialTurnOrderPlayer::PlayerA ? 1 : 0;
	FinalState.RuntimeState.PlayerBState.TotalAttackCount =
		FinalAttacker == EInitialTurnOrderPlayer::PlayerB ? 1 : 0;
	FinalState.RuntimeState.PlayerAState.Score = 2;
	FinalState.RuntimeState.PlayerBState.Score = 1;
	FMatchPlayAuthoritativeSession FinalSession(
		FinalState, FinalProvider, FinalProvider);
	FinalSession.RequestInitialActionPointRoll(
		MakeInitialRequest(FinalSession.GetStateSnapshot()));
	FMatchPlaySendingOffResolutionRequest FinalResolve;
	FinalResolve.AttackSequence =
		FinalSession.GetStateSnapshot().CurrentAttack.AttackSequence;
	FinalSession.ResolveSendingOff(FinalResolve);
	FMatchPlayAuthoritativeAdvanceAfterTerminalRequest FinalAdvanceRequest;
	FinalAdvanceRequest.AttackSequence = FinalResolve.AttackSequence;
	FinalAdvanceRequest.RequestingSide = FinalAttacker;
	const auto FinalAdvance =
		FinalSession.AdvanceAfterTerminal(FinalAdvanceRequest);
	TestTrue(TEXT("Final Session AP1 advance succeeds"),
		FinalAdvance.CompletionResult.bSuccess);
	TestTrue(TEXT("Final Session AP1 ends match"),
		FinalAdvance.CompletionResult.bMatchEnded);
	TestTrue(TEXT("Final Session MatchResult succeeds"),
		FinalAdvance.CompletionResult.MatchResultResolveResult.bSuccess);
	TestEqual(TEXT("Final AP1 score remains authoritative"),
		FinalSession.GetStateSnapshot().RuntimeState.PlayerAState.Score, 2);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAuthoritativeSetPieceCarrierFoundationTest,
	"FMCodex.MatchPlayRuntime.AuthoritativeSession.SetPieceCarrierFoundation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAuthoritativeSetPieceCarrierFoundationTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayFullD12AuthoritativeSessionTests;
	TestEqual(TEXT("Set Piece Carrier command is append-only"),
		static_cast<uint8>(
			EMatchPlayAuthoritativeCommandKind::SubmitSetPieceCarrier),
		static_cast<uint8>(
			EMatchPlayAuthoritativeCommandKind::ResolveSendingOff) + 1);

	struct FCarrierCase
	{
		int32 RawTypeD6 = 0;
		ESetPieceSelectedType Type = ESetPieceSelectedType::None;
		const TCHAR* Label = TEXT("");
	};
	const FCarrierCase Cases[] = {
		{ 5, ESetPieceSelectedType::ShortFreeKick, TEXT("Short") },
		{ 3, ESetPieceSelectedType::LongFreeKick, TEXT("Long") },
		{ 6, ESetPieceSelectedType::Penalty, TEXT("Penalty") }
	};

	for (const FCarrierCase& Case : Cases)
	{
		FQueueAttackEntryRollProvider Provider;
		Provider.EnqueueD12(MakeSuccess(9));
		Provider.EnqueueD6(MakeSuccess(Case.RawTypeD6));
		FMatchPlayAuthoritativeSession Session(Provider);
		if (!Initialize(*this, Session,
			*FString::Printf(TEXT("SetPieceCarrier_%s"), Case.Label)))
		{
			continue;
		}
		Session.RequestInitialActionPointRoll(
			MakeInitialRequest(Session.GetStateSnapshot()));
		Session.RequestSetPieceTypeRoll(
			MakeTypeRequest(Session.GetStateSnapshot()));
		const FMatchPlayState Awaiting = Session.GetStateSnapshot();
		TestEqual(TEXT("Session derives expected concrete type"),
			Awaiting.CurrentAttack.SetPieceRoute.SelectedType,
			Case.Type);
		const EInitialTurnOrderPlayer Attacker =
			Awaiting.RuntimeState.CurrentAttackingPlayer;
		const EInitialTurnOrderPlayer Defender = GetOtherSide(Attacker);
		const FPlayerCardRuleSnapshotSet& AttackerSnapshots =
			Attacker == EInitialTurnOrderPlayer::PlayerA
				? Awaiting.CardSnapshotAuthority.PlayerACardSnapshots
				: Awaiting.CardSnapshotAuthority.PlayerBCardSnapshots;
		FName CarrierCardId = NAME_None;
		for (const FPlayerCardRuleSnapshot& Snapshot : AttackerSnapshots.Cards)
		{
			if (!Snapshot.bIsGoalkeeper)
			{
				CarrierCardId = Snapshot.CardId;
				break;
			}
		}

		FMatchPlaySetPieceCarrierSelectionRequest Request;
		Request.RequestingSide = Attacker;
		Request.AttackSequence = Awaiting.CurrentAttack.AttackSequence;
		Request.CardId = CarrierCardId;
		FMatchPlaySetPieceCarrierSelectionRequest WrongSide = Request;
		WrongSide.RequestingSide = Defender;
		const auto WrongSideResult = Session.SubmitSetPieceCarrier(WrongSide);
		TestEqual(TEXT("Session wrong-side Carrier fails at domain boundary"),
			WrongSideResult.CarrierResult.ErrorCode,
			EMatchPlaySetPieceCarrierSelectionErrorCode
				::RequestingSideNotCurrentAttacker);
		TestFalse(TEXT("Wrong-side Carrier is not adopted"),
			WrongSideResult.RuntimeEnvelope.bStateAdvanced);
		TestTrue(TEXT("Wrong-side Carrier preserves snapshot"),
			AreStatesEqual(Session.GetStateSnapshot(), Awaiting));

		FMatchPlaySetPieceCarrierSelectionRequest Stale = Request;
		++Stale.AttackSequence;
		const auto StaleResult = Session.SubmitSetPieceCarrier(Stale);
		TestEqual(TEXT("Session stale Carrier is rejected"),
			StaleResult.CarrierResult.ErrorCode,
			EMatchPlaySetPieceCarrierSelectionErrorCode
				::AttackSequenceMismatch);
		TestTrue(TEXT("Stale Carrier preserves snapshot"),
			AreStatesEqual(Session.GetStateSnapshot(), Awaiting));

		const int32 D12BeforeCarrier = Provider.GetD12CallCount();
		const int32 D6BeforeCarrier = Provider.GetD6CallCount();
		const int32 SelectionBeforeCarrier = Provider.GetSelectionCallCount();
		const auto Success = Session.SubmitSetPieceCarrier(Request);
		const FMatchPlayState Selected = Session.GetStateSnapshot();
		TestTrue(TEXT("Session Carrier succeeds"),
			Success.RuntimeEnvelope.bAccepted
				&& Success.RuntimeEnvelope.bDomainSuccess
				&& Success.RuntimeEnvelope.bStateAdvanced
				&& Success.CarrierResult.bSuccess);
		TestEqual(TEXT("Session Carrier command kind is typed"),
			Success.RuntimeEnvelope.CommandKind,
			EMatchPlayAuthoritativeCommandKind::SubmitSetPieceCarrier);
		TestEqual(TEXT("Carrier command consumes no D12"),
			Provider.GetD12CallCount(), D12BeforeCarrier);
		TestEqual(TEXT("Carrier command consumes no D6"),
			Provider.GetD6CallCount(), D6BeforeCarrier);
		TestEqual(TEXT("Carrier command consumes no selection RNG"),
			Provider.GetSelectionCallCount(), SelectionBeforeCarrier);
		TestTrue(TEXT("Carrier selection preserves score, opportunity, usage, and authority"),
			AreNonCurrentAttackFactsEqual(Awaiting, Selected));
		const FMatchPlaySetPieceParticipantBinding* Binding = nullptr;
		switch (Case.Type)
		{
		case ESetPieceSelectedType::ShortFreeKick:
			TestEqual(TEXT("Short Session advances to AwaitingMethod"),
				Selected.CurrentAttack.SetPieceRoute.ShortFreeKick.Stage,
				EMatchPlaySetPieceCarrierRouteStage::AwaitingMethod);
			Binding = &Selected.CurrentAttack.SetPieceRoute.ShortFreeKick.Carrier;
			break;
		case ESetPieceSelectedType::LongFreeKick:
			TestEqual(TEXT("Long Session advances to AwaitingMethod"),
				Selected.CurrentAttack.SetPieceRoute.LongFreeKick.Stage,
				EMatchPlaySetPieceCarrierRouteStage::AwaitingMethod);
			Binding = &Selected.CurrentAttack.SetPieceRoute.LongFreeKick.Carrier;
			break;
		case ESetPieceSelectedType::Penalty:
			TestEqual(TEXT("Penalty Session advances to AwaitingMethod"),
				Selected.CurrentAttack.SetPieceRoute.Penalty.Stage,
				EMatchPlaySetPieceCarrierRouteStage::AwaitingMethod);
			Binding = &Selected.CurrentAttack.SetPieceRoute.Penalty.Carrier;
			break;
		default:
			break;
		}
		TestTrue(TEXT("Reconstructable Carrier binding is complete"),
			Binding != nullptr
				&& Binding->bIsBound
				&& Binding->OwnerSide == Attacker
				&& Binding->CardId == CarrierCardId
				&& Binding->Snapshot.CardId == CarrierCardId);

		const auto Duplicate = Session.SubmitSetPieceCarrier(Request);
		TestEqual(TEXT("Duplicate Session Carrier is wrong-stage rejected"),
			Duplicate.CarrierResult.ErrorCode,
			EMatchPlaySetPieceCarrierSelectionErrorCode::WrongStage);
		TestFalse(TEXT("Duplicate Session Carrier is not adopted"),
			Duplicate.RuntimeEnvelope.bStateAdvanced);
		TestTrue(TEXT("Duplicate Session Carrier preserves accepted snapshot"),
			AreStatesEqual(Session.GetStateSnapshot(), Selected));

		FMatchPlayAuthoritativeSession Reconstructed(Selected, Provider);
		TestTrue(TEXT("Fresh Session reconstructs exact AwaitingMethod snapshot"),
			AreStatesEqual(Reconstructed.GetStateSnapshot(), Selected));
		const auto ReconstructedDuplicate =
			Reconstructed.SubmitSetPieceCarrier(Request);
		TestEqual(TEXT("Reconstructed Session keeps Carrier gate closed"),
			ReconstructedDuplicate.CarrierResult.ErrorCode,
			EMatchPlaySetPieceCarrierSelectionErrorCode::WrongStage);
	}

	FQueueAttackEntryRollProvider CornerProvider;
	CornerProvider.EnqueueD12(MakeSuccess(12));
	CornerProvider.EnqueueD6(MakeSuccess(1));
	FMatchPlayAuthoritativeSession CornerSession(CornerProvider);
	if (!Initialize(*this, CornerSession, TEXT("SetPieceCarrier_Corner")))
	{
		return false;
	}
	CornerSession.RequestInitialActionPointRoll(
		MakeInitialRequest(CornerSession.GetStateSnapshot()));
	CornerSession.RequestSetPieceTypeRoll(
		MakeTypeRequest(CornerSession.GetStateSnapshot()));
	const FMatchPlayState Corner = CornerSession.GetStateSnapshot();
	TestEqual(TEXT("Corner reconstructs AwaitingAttackerNominations"),
		Corner.CurrentAttack.SetPieceRoute.Corner.Stage,
		EMatchPlaySetPieceCornerRouteStage::AwaitingAttackerNominations);
	const EInitialTurnOrderPlayer CornerAttacker =
		Corner.RuntimeState.CurrentAttackingPlayer;
	const FPlayerCardRuleSnapshotSet& CornerSnapshots =
		CornerAttacker == EInitialTurnOrderPlayer::PlayerA
			? Corner.CardSnapshotAuthority.PlayerACardSnapshots
			: Corner.CardSnapshotAuthority.PlayerBCardSnapshots;
	FName CornerCard = NAME_None;
	for (const FPlayerCardRuleSnapshot& Snapshot : CornerSnapshots.Cards)
	{
		if (!Snapshot.bIsGoalkeeper)
		{
			CornerCard = Snapshot.CardId;
			break;
		}
	}
	FMatchPlaySetPieceCarrierSelectionRequest CornerRequest;
	CornerRequest.RequestingSide = CornerAttacker;
	CornerRequest.AttackSequence = Corner.CurrentAttack.AttackSequence;
	CornerRequest.CardId = CornerCard;
	const int32 CornerD6Before = CornerProvider.GetD6CallCount();
	const auto CornerReject =
		CornerSession.SubmitSetPieceCarrier(CornerRequest);
	TestEqual(TEXT("Corner rejects Carrier at Session boundary"),
		CornerReject.CarrierResult.ErrorCode,
		EMatchPlaySetPieceCarrierSelectionErrorCode
			::UnsupportedSetPieceType);
	TestEqual(TEXT("Corner rejection consumes zero RNG"),
		CornerProvider.GetD6CallCount(), CornerD6Before);
	TestTrue(TEXT("Corner rejection preserves exact pending snapshot"),
		AreStatesEqual(CornerSession.GetStateSnapshot(), Corner));
	return true;
}

#endif
