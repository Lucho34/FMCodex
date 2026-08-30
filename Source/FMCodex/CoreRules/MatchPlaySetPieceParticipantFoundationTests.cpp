#include "MatchPlayDefendingGoalkeeperQuery.h"
#include "MatchPlayOpeningInitializer.h"
#include "MatchPlaySetPieceCarrierAvailability.h"
#include "MatchPlaySetPieceCarrierSelection.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

namespace MatchPlaySetPieceParticipantFoundationTests
{
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

	FMatchPlayState MakeInitializedState(const FString& Prefix)
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
		return FMatchPlayOpeningInitializer::InitializeMatchPlayOpening(Input)
			.MatchPlayState;
	}

	EInitialTurnOrderPlayer OtherSide(const EInitialTurnOrderPlayer Side)
	{
		return Side == EInitialTurnOrderPlayer::PlayerA
			? EInitialTurnOrderPlayer::PlayerB
			: EInitialTurnOrderPlayer::PlayerA;
	}

	FCardUsageState& Usage(
		FMatchPlayState& State,
		const EInitialTurnOrderPlayer Side)
	{
		return Side == EInitialTurnOrderPlayer::PlayerA
			? State.CardUsageState.PlayerACardUsageState
			: State.CardUsageState.PlayerBCardUsageState;
	}

	const FCardUsageState& Usage(
		const FMatchPlayState& State,
		const EInitialTurnOrderPlayer Side)
	{
		return Side == EInitialTurnOrderPlayer::PlayerA
			? State.CardUsageState.PlayerACardUsageState
			: State.CardUsageState.PlayerBCardUsageState;
	}

	FPlayerCardRuleSnapshotSet& Snapshots(
		FMatchPlayState& State,
		const EInitialTurnOrderPlayer Side)
	{
		return Side == EInitialTurnOrderPlayer::PlayerA
			? State.CardSnapshotAuthority.PlayerACardSnapshots
			: State.CardSnapshotAuthority.PlayerBCardSnapshots;
	}

	const FPlayerCardRuleSnapshotSet& Snapshots(
		const FMatchPlayState& State,
		const EInitialTurnOrderPlayer Side)
	{
		return Side == EInitialTurnOrderPlayer::PlayerA
			? State.CardSnapshotAuthority.PlayerACardSnapshots
			: State.CardSnapshotAuthority.PlayerBCardSnapshots;
	}

	FName FindCard(
		const FMatchPlayState& State,
		const EInitialTurnOrderPlayer Side,
		const bool bGoalkeeper,
		const int32 MatchIndex = 0)
	{
		int32 CurrentIndex = 0;
		for (const FPlayerCardRuleSnapshot& Snapshot : Snapshots(State, Side).Cards)
		{
			if (Snapshot.bIsGoalkeeper == bGoalkeeper)
			{
				if (CurrentIndex == MatchIndex)
				{
					return Snapshot.CardId;
				}
				++CurrentIndex;
			}
		}
		return NAME_None;
	}

	int32 TypeRoll(const ESetPieceSelectedType Type)
	{
		switch (Type)
		{
		case ESetPieceSelectedType::Corner:
			return 1;
		case ESetPieceSelectedType::LongFreeKick:
			return 3;
		case ESetPieceSelectedType::ShortFreeKick:
			return 5;
		case ESetPieceSelectedType::Penalty:
			return 6;
		default:
			return 0;
		}
	}

	FMatchPlayState MakeConcreteState(
		const FString& Prefix,
		const ESetPieceSelectedType Type)
	{
		FMatchPlayState State = MakeInitializedState(Prefix);
		State.bHasCurrentAttack = true;
		State.CurrentAttack.Phase = EMatchPlayCurrentAttackPhase::RoutePending;
		State.CurrentAttack.AttackSequence = 1;
		State.CurrentAttack.ActionPoint = 9;
		State.CurrentAttack.RawInitialD12 = 9;
		State.CurrentAttack.RouteKind = EMatchPlayCurrentAttackRouteKind::SetPiece;
		State.CurrentAttack.SetPieceRoute.Stage =
			EMatchPlaySetPieceRouteStage::TypeResolved;
		State.CurrentAttack.SetPieceRoute.bHasTypeRoll = true;
		State.CurrentAttack.SetPieceRoute.RawTypeD6 = TypeRoll(Type);
		State.CurrentAttack.SetPieceRoute.SelectedType = Type;
		switch (Type)
		{
		case ESetPieceSelectedType::ShortFreeKick:
			State.CurrentAttack.SetPieceRoute.ShortFreeKick.Stage =
				EMatchPlaySetPieceCarrierRouteStage::AwaitingCarrier;
			break;
		case ESetPieceSelectedType::LongFreeKick:
			State.CurrentAttack.SetPieceRoute.LongFreeKick.Stage =
				EMatchPlaySetPieceCarrierRouteStage::AwaitingCarrier;
			break;
		case ESetPieceSelectedType::Penalty:
			State.CurrentAttack.SetPieceRoute.Penalty.Stage =
				EMatchPlaySetPieceCarrierRouteStage::AwaitingCarrier;
			break;
		case ESetPieceSelectedType::Corner:
			State.CurrentAttack.SetPieceRoute.Corner.Stage =
				EMatchPlaySetPieceCornerRouteStage
					::AwaitingAttackerNominations;
			break;
		default:
			break;
		}
		return State;
	}

	void MoveCard(
		FMatchPlayState& State,
		const EInitialTurnOrderPlayer Side,
		const FName CardId,
		const bool bEjected)
	{
		FCardUsageState& CardUsage = Usage(State, Side);
		CardUsage.AvailableCardIds.Remove(CardId);
		(bEjected ? CardUsage.EjectedCardIds : CardUsage.UsedCardIds)
			.Add(CardId);
	}

	bool StatesEqual(const FMatchPlayState& Left, const FMatchPlayState& Right)
	{
		return FMatchPlayState::StaticStruct()->CompareScriptStruct(
			&Left,
			&Right,
			0);
	}

	FMatchPlaySetPieceParticipantEligibilityResult Evaluate(
		const FMatchPlayState& State,
		const EInitialTurnOrderPlayer Side,
		const FName CardId,
		const EMatchPlaySetPieceParticipantRole Role)
	{
		FMatchPlaySetPieceParticipantEligibilityRequest Request;
		Request.ExpectedOwnerSide = Side;
		Request.CardId = CardId;
		Request.Role = Role;
		return FMatchPlaySetPieceParticipantEligibility::Evaluate(
			State,
			Request);
	}

	FMatchPlaySetPieceCarrierSelectionRequest SelectionRequest(
		const FMatchPlayState& State,
		const FName CardId)
	{
		FMatchPlaySetPieceCarrierSelectionRequest Request;
		Request.RequestingSide = State.RuntimeState.CurrentAttackingPlayer;
		Request.AttackSequence = State.CurrentAttack.AttackSequence;
		Request.CardId = CardId;
		return Request;
	}

	FMatchPlaySetPieceParticipantBinding& ActiveCarrier(
		FMatchPlayState& State)
	{
		switch (State.CurrentAttack.SetPieceRoute.SelectedType)
		{
		case ESetPieceSelectedType::ShortFreeKick:
			return State.CurrentAttack.SetPieceRoute.ShortFreeKick.Carrier;
		case ESetPieceSelectedType::LongFreeKick:
			return State.CurrentAttack.SetPieceRoute.LongFreeKick.Carrier;
		default:
			return State.CurrentAttack.SetPieceRoute.Penalty.Carrier;
		}
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlaySetPieceParticipantEligibilityMatrixTest,
	"FMCodex.CoreRules.MatchPlaySetPieceFoundation.ParticipantEligibilityAndGoalkeeper",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlaySetPieceParticipantEligibilityMatrixTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlaySetPieceParticipantFoundationTests;
	const FMatchPlayState State = MakeInitializedState(TEXT("SP_Eligibility"));
	const EInitialTurnOrderPlayer Attacker =
		State.RuntimeState.CurrentAttackingPlayer;
	const EInitialTurnOrderPlayer Defender = OtherSide(Attacker);
	const FName AttackerCard = FindCard(State, Attacker, false);
	const FName DefenderCard = FindCard(State, Defender, false);
	const FName AttackerGoalkeeper = FindCard(State, Attacker, true);

	const FMatchPlayState Before = State;
	TestTrue(TEXT("Available attacker is a valid Carrier"),
		Evaluate(State, Attacker, AttackerCard,
			EMatchPlaySetPieceParticipantRole::Carrier).bIsEligible);
	TestTrue(TEXT("Same common contract accepts future Corner Runner"),
		Evaluate(State, Attacker, AttackerCard,
			EMatchPlaySetPieceParticipantRole::CornerRunner).bIsEligible);
	TestTrue(TEXT("Same common contract accepts defending Corner Helper"),
		Evaluate(State, Defender, DefenderCard,
			EMatchPlaySetPieceParticipantRole::CornerHelper).bIsEligible);
	TestFalse(TEXT("Opponent card is not attacker-owned"),
		Evaluate(State, Attacker, DefenderCard,
			EMatchPlaySetPieceParticipantRole::Carrier).bIsEligible);
	const auto Goalkeeper = Evaluate(State, Attacker, AttackerGoalkeeper,
		EMatchPlaySetPieceParticipantRole::Carrier);
	TestEqual(TEXT("GK is rejected by canonical snapshot truth"),
		Goalkeeper.ErrorCode,
		EMatchPlaySetPieceParticipantEligibilityErrorCode
			::GoalkeeperNotEligible);
	TestTrue(TEXT("Pure eligibility leaves state unchanged"),
		StatesEqual(State, Before));

	FMatchPlayState Used = State;
	MoveCard(Used, Attacker, AttackerCard, false);
	TestEqual(TEXT("Used card is rejected"),
		Evaluate(Used, Attacker, AttackerCard,
			EMatchPlaySetPieceParticipantRole::Carrier).ErrorCode,
		EMatchPlaySetPieceParticipantEligibilityErrorCode::CardUsed);
	FMatchPlayState Ejected = State;
	MoveCard(Ejected, Attacker, AttackerCard, true);
	TestEqual(TEXT("Ejected card is rejected"),
		Evaluate(Ejected, Attacker, AttackerCard,
			EMatchPlaySetPieceParticipantRole::Carrier).ErrorCode,
		EMatchPlaySetPieceParticipantEligibilityErrorCode::CardEjected);

	FMatchPlayState MissingSnapshot = State;
	Snapshots(MissingSnapshot, Attacker).Cards.RemoveAll(
		[AttackerCard](const FPlayerCardRuleSnapshot& Snapshot)
		{
			return Snapshot.CardId == AttackerCard;
		});
	TestEqual(TEXT("Missing authoritative Snapshot invalidates eligibility"),
		Evaluate(MissingSnapshot, Attacker, AttackerCard,
			EMatchPlaySetPieceParticipantRole::Carrier).ErrorCode,
		EMatchPlaySetPieceParticipantEligibilityErrorCode
			::InvalidUnderlyingState);

	FMatchPlayState WrongProvenance = State;
	FPlayerCardRuleSnapshot MovedSnapshot;
	for (const FPlayerCardRuleSnapshot& Snapshot : Snapshots(State, Attacker).Cards)
	{
		if (Snapshot.CardId == AttackerCard)
		{
			MovedSnapshot = Snapshot;
			break;
		}
	}
	Snapshots(WrongProvenance, Attacker).Cards.RemoveAll(
		[AttackerCard](const FPlayerCardRuleSnapshot& Snapshot)
		{
			return Snapshot.CardId == AttackerCard;
		});
	Snapshots(WrongProvenance, Defender).Cards.Add(MovedSnapshot);
	TestFalse(TEXT("Wrong-side Snapshot provenance is rejected"),
		Evaluate(WrongProvenance, Attacker, AttackerCard,
			EMatchPlaySetPieceParticipantRole::Carrier).bIsEligible);

	const FMatchPlayState GkBefore = State;
	const auto GkResult = FMatchPlayDefendingGoalkeeperQuery::Query(
		State,
		Defender);
	TestTrue(TEXT("Unique defending GK lookup succeeds"), GkResult.bSuccess);
	TestEqual(TEXT("Defending GK side is explicit"),
		GkResult.DefendingSide, Defender);
	TestTrue(TEXT("Defending GK snapshot is canonical GK"),
		GkResult.Snapshot.bIsGoalkeeper
			&& GkResult.Snapshot.bHasGoalkeeperAttributes);
	TestTrue(TEXT("GK lookup neither activates nor consumes"),
		StatesEqual(State, GkBefore));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlaySetPieceCarrierAvailabilityTest,
	"FMCodex.CoreRules.MatchPlaySetPieceFoundation.CarrierAvailability",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlaySetPieceCarrierAvailabilityTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlaySetPieceParticipantFoundationTests;
	for (const ESetPieceSelectedType Type : {
		ESetPieceSelectedType::ShortFreeKick,
		ESetPieceSelectedType::LongFreeKick,
		ESetPieceSelectedType::Penalty })
	{
		FMatchPlayState State = MakeConcreteState(
			FString::Printf(TEXT("SP_Availability_%d"),
				static_cast<int32>(Type)),
			Type);
		const EInitialTurnOrderPlayer Attacker =
			State.RuntimeState.CurrentAttackingPlayer;
		const FName First = FindCard(State, Attacker, false, 2);
		const FName Second = FindCard(State, Attacker, false, 7);
		const FName Goalkeeper = FindCard(State, Attacker, true);
		FCardUsageState& AttackerUsage = Usage(State, Attacker);
		const TArray<FName> OriginalAvailable = AttackerUsage.AvailableCardIds;
		AttackerUsage.AvailableCardIds = { First, Goalkeeper, Second };
		AttackerUsage.UsedCardIds.Reset();
		for (const FName CardId : OriginalAvailable)
		{
			if (!AttackerUsage.AvailableCardIds.Contains(CardId))
			{
				AttackerUsage.UsedCardIds.Add(CardId);
			}
		}
		FMatchPlaySetPieceCarrierAvailabilityRequest Request;
		Request.AttackSequence = State.CurrentAttack.AttackSequence;
		const FMatchPlayState Before = State;
		const auto Result = FMatchPlaySetPieceCarrierAvailability::Query(
			State,
			Request);
		TestTrue(TEXT("Carrier availability succeeds"), Result.bSuccess);
		TestEqual(TEXT("Only two ordered outfield candidates remain"),
			Result.LegalCarrierCardIds.Num(), 2);
		if (Result.LegalCarrierCardIds.Num() == 2)
		{
			TestEqual(TEXT("Available order is stable at first candidate"),
				Result.LegalCarrierCardIds[0], First);
			TestEqual(TEXT("Available order is stable at second candidate"),
				Result.LegalCarrierCardIds[1], Second);
		}
		TestTrue(TEXT("Availability query is pure"), StatesEqual(State, Before));

		FMatchPlayState Empty = MakeConcreteState(
			FString::Printf(TEXT("SP_AvailabilityEmpty_%d"),
				static_cast<int32>(Type)),
			Type);
		FCardUsageState& EmptyUsage = Usage(
			Empty,
			Empty.RuntimeState.CurrentAttackingPlayer);
		const FName EmptyGk = FindCard(
			Empty,
			Empty.RuntimeState.CurrentAttackingPlayer,
			true);
		const TArray<FName> EmptyOriginal = EmptyUsage.AvailableCardIds;
		EmptyUsage.AvailableCardIds = { EmptyGk };
		EmptyUsage.UsedCardIds.Reset();
		for (const FName CardId : EmptyOriginal)
		{
			if (CardId != EmptyGk)
			{
				EmptyUsage.UsedCardIds.Add(CardId);
			}
		}
		Request.AttackSequence = Empty.CurrentAttack.AttackSequence;
		const auto EmptyResult =
			FMatchPlaySetPieceCarrierAvailability::Query(Empty, Request);
		TestTrue(TEXT("Zero legal Carrier query succeeds"), EmptyResult.bSuccess);
		TestFalse(TEXT("Zero legal Carrier is explicit"),
			EmptyResult.bHasLegalCarrier);
		TestEqual(TEXT("Zero legal Carrier list is empty"),
			EmptyResult.LegalCarrierCardIds.Num(), 0);
	}

	FMatchPlayState Invalid = MakeConcreteState(
		TEXT("SP_AvailabilityInvalid"),
		ESetPieceSelectedType::ShortFreeKick);
	const EInitialTurnOrderPlayer InvalidAttacker =
		Invalid.RuntimeState.CurrentAttackingPlayer;
	const FName InvalidCard = FindCard(Invalid, InvalidAttacker, false);
	Snapshots(Invalid, InvalidAttacker).Cards.RemoveAll(
		[InvalidCard](const FPlayerCardRuleSnapshot& Snapshot)
		{
			return Snapshot.CardId == InvalidCard;
		});
	FMatchPlaySetPieceCarrierAvailabilityRequest InvalidRequest;
	InvalidRequest.AttackSequence = Invalid.CurrentAttack.AttackSequence;
	TestFalse(TEXT("Invalid underlying state fails availability query"),
		FMatchPlaySetPieceCarrierAvailability::Query(
			Invalid,
			InvalidRequest).bSuccess);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlaySetPieceCarrierSelectionAndValidationTest,
	"FMCodex.CoreRules.MatchPlaySetPieceFoundation.CarrierSelectionAndValidation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlaySetPieceCarrierSelectionAndValidationTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlaySetPieceParticipantFoundationTests;
	for (const ESetPieceSelectedType Type : {
		ESetPieceSelectedType::ShortFreeKick,
		ESetPieceSelectedType::LongFreeKick,
		ESetPieceSelectedType::Penalty })
	{
		const FString Prefix = FString::Printf(
			TEXT("SP_Selection_%d"), static_cast<int32>(Type));
		const FMatchPlayState State = MakeConcreteState(Prefix, Type);
		const EInitialTurnOrderPlayer Attacker =
			State.RuntimeState.CurrentAttackingPlayer;
		const EInitialTurnOrderPlayer Defender = OtherSide(Attacker);
		const FName Carrier = FindCard(State, Attacker, false);
		const FName Goalkeeper = FindCard(State, Attacker, true);
		const FName Opponent = FindCard(State, Defender, false);
		const auto Request = SelectionRequest(State, Carrier);
		const auto Success = FMatchPlaySetPieceCarrierSelection::Submit(
			State,
			Request);
		TestTrue(TEXT("Carrier selection succeeds"), Success.bSuccess);
		TestTrue(TEXT("Carrier identity and frozen snapshot are persisted"),
			Success.EligibilityResult.Binding.bIsBound
				&& Success.EligibilityResult.Binding.OwnerSide == Attacker
				&& Success.EligibilityResult.Binding.CardId == Carrier);
		TestTrue(TEXT("Selection leaves CardUsage unchanged"),
			FMatchCardUsageState::StaticStruct()->CompareScriptStruct(
				&State.CardUsageState,
				&Success.AfterState.CardUsageState,
				0));
		TestTrue(TEXT("Selected Carrier remains Available"),
			Usage(Success.AfterState, Attacker)
				.AvailableCardIds.Contains(Carrier));
		TestTrue(TEXT("AwaitingMethod candidate validates"),
			FMatchPlayCurrentAttackRouteStateValidator::Validate(
				Success.AfterState).bIsCanonical);

		const auto Duplicate = FMatchPlaySetPieceCarrierSelection::Submit(
			Success.AfterState,
			Request);
		TestEqual(TEXT("Duplicate follows wrong-stage convention"),
			Duplicate.ErrorCode,
			EMatchPlaySetPieceCarrierSelectionErrorCode::WrongStage);
		TestTrue(TEXT("Duplicate leaves accepted state unchanged"),
			StatesEqual(Duplicate.AfterState, Success.AfterState));

		auto WrongSide = Request;
		WrongSide.RequestingSide = Defender;
		const auto WrongSideResult =
			FMatchPlaySetPieceCarrierSelection::Submit(State, WrongSide);
		TestEqual(TEXT("Wrong side is rejected"),
			WrongSideResult.ErrorCode,
			EMatchPlaySetPieceCarrierSelectionErrorCode
				::RequestingSideNotCurrentAttacker);
		TestTrue(TEXT("Wrong side preserves state"),
			StatesEqual(WrongSideResult.AfterState, State));

		auto Stale = Request;
		++Stale.AttackSequence;
		TestEqual(TEXT("Stale request is rejected"),
			FMatchPlaySetPieceCarrierSelection::Submit(State, Stale).ErrorCode,
			EMatchPlaySetPieceCarrierSelectionErrorCode
				::AttackSequenceMismatch);
		auto GoalkeeperRequest = SelectionRequest(State, Goalkeeper);
		TestFalse(TEXT("GK selection is rejected"),
			FMatchPlaySetPieceCarrierSelection::Submit(
				State,
				GoalkeeperRequest).bSuccess);
		auto OpponentRequest = SelectionRequest(State, Opponent);
		TestFalse(TEXT("Opponent selection is rejected"),
			FMatchPlaySetPieceCarrierSelection::Submit(
				State,
				OpponentRequest).bSuccess);

		FMatchPlayState Used = State;
		MoveCard(Used, Attacker, Carrier, false);
		TestFalse(TEXT("Used selection is rejected"),
			FMatchPlaySetPieceCarrierSelection::Submit(Used, Request).bSuccess);
		FMatchPlayState Ejected = State;
		MoveCard(Ejected, Attacker, Carrier, true);
		TestFalse(TEXT("Ejected selection is rejected"),
			FMatchPlaySetPieceCarrierSelection::Submit(Ejected, Request).bSuccess);
	}

	FMatchPlayState Corner = MakeConcreteState(
		TEXT("SP_CornerReject"),
		ESetPieceSelectedType::Corner);
	const FName CornerCandidate = FindCard(
		Corner,
		Corner.RuntimeState.CurrentAttackingPlayer,
		false);
	TestEqual(TEXT("Corner rejects Carrier command"),
		FMatchPlaySetPieceCarrierSelection::Submit(
			Corner,
			SelectionRequest(Corner, CornerCandidate)).ErrorCode,
		EMatchPlaySetPieceCarrierSelectionErrorCode
			::UnsupportedSetPieceType);

	using EError = EMatchPlayCurrentAttackRouteStateValidationErrorCode;
	auto ExpectError = [this](
		const TCHAR* Label,
		const FMatchPlayState& State,
		const EError Error)
	{
		const auto Validation =
			FMatchPlayCurrentAttackRouteStateValidator::Validate(State);
		TestFalse(Label, Validation.bIsCanonical);
		TestEqual(Label, Validation.ErrorCode, Error);
	};

	FMatchPlayState WrongPayload = MakeConcreteState(
		TEXT("SP_ValidatorWrongPayload"),
		ESetPieceSelectedType::LongFreeKick);
	WrongPayload.CurrentAttack.SetPieceRoute.LongFreeKick =
		FMatchPlayLongFreeKickRouteState();
	WrongPayload.CurrentAttack.SetPieceRoute.ShortFreeKick.Stage =
		EMatchPlaySetPieceCarrierRouteStage::AwaitingCarrier;
	ExpectError(TEXT("Short payload under Long type"), WrongPayload,
		EError::ConcretePayloadTypeMismatch);

	FMatchPlayState TwoPayloads = MakeConcreteState(
		TEXT("SP_ValidatorTwoPayloads"),
		ESetPieceSelectedType::ShortFreeKick);
	TwoPayloads.CurrentAttack.SetPieceRoute.Penalty.Stage =
		EMatchPlaySetPieceCarrierRouteStage::AwaitingCarrier;
	ExpectError(TEXT("Two concrete payloads"), TwoPayloads,
		EError::ConcretePayloadCountMismatch);

	FMatchPlayState PrematureCarrier = MakeConcreteState(
		TEXT("SP_ValidatorPrematureCarrier"),
		ESetPieceSelectedType::ShortFreeKick);
	const EInitialTurnOrderPlayer PrematureAttacker =
		PrematureCarrier.RuntimeState.CurrentAttackingPlayer;
	const FName PrematureCard = FindCard(
		PrematureCarrier, PrematureAttacker, false);
	PrematureCarrier.CurrentAttack.SetPieceRoute.ShortFreeKick.Carrier =
		Evaluate(PrematureCarrier, PrematureAttacker, PrematureCard,
			EMatchPlaySetPieceParticipantRole::Carrier).Binding;
	ExpectError(TEXT("AwaitingCarrier with Carrier"), PrematureCarrier,
		EError::AwaitingCarrierHasBoundCarrier);

	FMatchPlayState MissingCarrier = MakeConcreteState(
		TEXT("SP_ValidatorMissingCarrier"),
		ESetPieceSelectedType::Penalty);
	MissingCarrier.CurrentAttack.SetPieceRoute.Penalty.Stage =
		EMatchPlaySetPieceCarrierRouteStage::AwaitingMethod;
	ExpectError(TEXT("AwaitingMethod missing Carrier"), MissingCarrier,
		EError::AwaitingMethodMissingCarrier);

	FMatchPlayState Bound = MakeConcreteState(
		TEXT("SP_ValidatorBound"),
		ESetPieceSelectedType::ShortFreeKick);
	const EInitialTurnOrderPlayer BoundAttacker =
		Bound.RuntimeState.CurrentAttackingPlayer;
	const FName BoundCard = FindCard(Bound, BoundAttacker, false);
	Bound = FMatchPlaySetPieceCarrierSelection::Submit(
		Bound,
		SelectionRequest(Bound, BoundCard)).AfterState;
	FMatchPlayState WrongOwner = Bound;
	ActiveCarrier(WrongOwner).OwnerSide = OtherSide(BoundAttacker);
	ExpectError(TEXT("Carrier wrong owner"), WrongOwner,
		EError::CarrierOwnerSideMismatch);
	FMatchPlayState BoundGoalkeeper = Bound;
	const FName BoundGk = FindCard(BoundGoalkeeper, BoundAttacker, true);
	const auto GkSnapshot =
		FMatchPlayCardSnapshotAuthorityQuery::FindByPlayerSideAndCardId(
			BoundGoalkeeper.CardSnapshotAuthority,
			BoundAttacker,
			BoundGk);
	ActiveCarrier(BoundGoalkeeper).CardId = BoundGk;
	ActiveCarrier(BoundGoalkeeper).Snapshot = GkSnapshot.Snapshot;
	ExpectError(TEXT("Carrier GK"), BoundGoalkeeper,
		EError::CarrierEligibilityFailed);
	FMatchPlayState BoundUsed = Bound;
	MoveCard(BoundUsed, BoundAttacker, BoundCard, false);
	ExpectError(TEXT("Carrier not Available"), BoundUsed,
		EError::CarrierEligibilityFailed);
	FMatchPlayState SnapshotMismatch = Bound;
	++ActiveCarrier(SnapshotMismatch).Snapshot.Attributes.Shooting;
	ExpectError(TEXT("Carrier frozen Snapshot mismatch"), SnapshotMismatch,
		EError::CarrierSnapshotBindingMismatch);
	FMatchPlayState BadSequence = MakeConcreteState(
		TEXT("SP_ValidatorSequence"),
		ESetPieceSelectedType::Penalty);
	BadSequence.CurrentAttack.AttackSequence = 0;
	ExpectError(TEXT("Invalid AttackSequence remains formal"), BadSequence,
		EError::InvalidAttackSequence);

	FMatchPlayState CornerContamination = MakeConcreteState(
		TEXT("SP_ValidatorCorner"),
		ESetPieceSelectedType::Corner);
	CornerContamination.CurrentAttack.SetPieceRoute.ShortFreeKick.Stage =
		EMatchPlaySetPieceCarrierRouteStage::AwaitingCarrier;
	ExpectError(TEXT("Corner rejects foreign participant payload"),
		CornerContamination,
		EError::ConcretePayloadCountMismatch);
	return true;
}

#endif
