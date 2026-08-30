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

	class FQueueAttackEntryRollProvider final
		: public IMatchPlayAttackEntryRollProvider
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

		int32 GetD12CallCount() const
		{
			return D12Purposes.Num();
		}

		int32 GetD6CallCount() const
		{
			return D6Purposes.Num();
		}

		TArray<EMatchPlayAttackEntryRollPurpose> D12Purposes;
		TArray<EMatchPlayAttackEntryRollPurpose> D6Purposes;

	private:
		TArray<FMatchPlayAttackEntryRollProviderResult> D12Results;
		TArray<FMatchPlayAttackEntryRollProviderResult> D6Results;
		int32 NextD12Index = 0;
		int32 NextD6Index = 0;
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
				Right.GoalkeeperUsageState);
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

#endif
