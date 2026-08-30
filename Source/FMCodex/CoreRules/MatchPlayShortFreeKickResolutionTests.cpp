#include "MatchPlayOpeningInitializer.h"
#include "MatchPlaySetPieceCarrierSelection.h"
#include "MatchPlayShortFreeKickResolution.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

namespace MatchPlayShortFreeKickResolutionTests
{
	FPlayerCardData MakeCard(const FString& Id, const bool bGoalkeeper)
	{
		FPlayerCardData Card;
		Card.CardId = FName(*Id);
		Card.Rarity = ECardRarity::Common;
		Card.bIsGoalkeeper = bGoalkeeper;
		Card.PositionTypes = { bGoalkeeper
			? EPlayerPositionType::Goalkeeper
			: EPlayerPositionType::Attack };
		Card.Attributes.Stamina = 5;
		Card.Attributes.Shooting = 5;
		Card.Attributes.Passing = 4;
		Card.GoalkeeperAttributes.Handling = 4;
		return Card;
	}

	TArray<FPlayerCardData> MakeDeck(const FString& Prefix)
	{
		TArray<FPlayerCardData> Deck;
		for (int32 Index = 0; Index < 19; ++Index)
		{
			Deck.Add(MakeCard(
				FString::Printf(TEXT("%s_OUT_%02d"), *Prefix, Index),
				false));
		}
		Deck.Add(MakeCard(Prefix + TEXT("_GK"), true));
		return Deck;
	}

	FMatchPlayState MakeShortAwaitingCarrier(const FString& Prefix)
	{
		FMatchPlayOpeningInitializeInput Input;
		Input.OpeningInput.PlayerADeck = MakeDeck(Prefix + TEXT("_A"));
		Input.OpeningInput.PlayerBDeck = MakeDeck(Prefix + TEXT("_B"));
		Input.OpeningInput.PlayerAAttackCountD6Roll = 1;
		Input.OpeningInput.PlayerBAttackCountD6Roll = 1;
		Input.OpeningInput.PlayerATieBreakerRoll = 6;
		Input.OpeningInput.PlayerBTieBreakerRoll = 2;
		FMatchPlayDeploymentSlotDefinition ASlot;
		ASlot.SlotId = FName(*(Prefix + TEXT("_SlotA")));
		ASlot.NeutralSide = EMatchPlayNeutralSlotSide::NearPlayerA;
		FMatchPlayDeploymentSlotDefinition BSlot;
		BSlot.SlotId = FName(*(Prefix + TEXT("_SlotB")));
		BSlot.NeutralSide = EMatchPlayNeutralSlotSide::NearPlayerB;
		Input.DeploymentSlotCatalog.Slots = { ASlot, BSlot };
		FMatchPlayState State =
			FMatchPlayOpeningInitializer::InitializeMatchPlayOpening(Input)
				.MatchPlayState;
		State.bHasCurrentAttack = true;
		State.CurrentAttack.Phase = EMatchPlayCurrentAttackPhase::RoutePending;
		State.CurrentAttack.AttackSequence = 1;
		State.CurrentAttack.ActionPoint = 9;
		State.CurrentAttack.RawInitialD12 = 9;
		State.CurrentAttack.RouteKind =
			EMatchPlayCurrentAttackRouteKind::SetPiece;
		State.CurrentAttack.SetPieceRoute.Stage =
			EMatchPlaySetPieceRouteStage::TypeResolved;
		State.CurrentAttack.SetPieceRoute.bHasTypeRoll = true;
		State.CurrentAttack.SetPieceRoute.RawTypeD6 = 5;
		State.CurrentAttack.SetPieceRoute.SelectedType =
			ESetPieceSelectedType::ShortFreeKick;
		State.CurrentAttack.SetPieceRoute.ShortFreeKick.Stage =
			EMatchPlaySetPieceCarrierRouteStage::AwaitingCarrier;
		return State;
	}

	EInitialTurnOrderPlayer Other(const EInitialTurnOrderPlayer Side)
	{
		return Side == EInitialTurnOrderPlayer::PlayerA
			? EInitialTurnOrderPlayer::PlayerB
			: EInitialTurnOrderPlayer::PlayerA;
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

	FPlayerCardRuleSnapshot* FindSnapshot(
		FMatchPlayState& State,
		const EInitialTurnOrderPlayer Side,
		const bool bGoalkeeper,
		const int32 MatchIndex = 0)
	{
		int32 Current = 0;
		for (FPlayerCardRuleSnapshot& Snapshot : Snapshots(State, Side).Cards)
		{
			if (Snapshot.bIsGoalkeeper == bGoalkeeper
				&& Current++ == MatchIndex)
			{
				return &Snapshot;
			}
		}
		return nullptr;
	}

	FName FindCard(
		const FMatchPlayState& State,
		const EInitialTurnOrderPlayer Side,
		const bool bGoalkeeper,
		const int32 MatchIndex = 0)
	{
		int32 Current = 0;
		for (const FPlayerCardRuleSnapshot& Snapshot : Snapshots(State, Side).Cards)
		{
			if (Snapshot.bIsGoalkeeper == bGoalkeeper
				&& Current++ == MatchIndex)
			{
				return Snapshot.CardId;
			}
		}
		return NAME_None;
	}

	bool Equal(const FMatchPlayState& Left, const FMatchPlayState& Right)
	{
		return FMatchPlayState::StaticStruct()->CompareScriptStruct(
			&Left, &Right, 0);
	}

	FMatchPlayState BindCarrier(FMatchPlayState State, FName* OutCardId = nullptr)
	{
		const EInitialTurnOrderPlayer Attacker =
			State.RuntimeState.CurrentAttackingPlayer;
		const FName CardId = FindCard(State, Attacker, false);
		FMatchPlaySetPieceCarrierSelectionRequest Request;
		Request.RequestingSide = Attacker;
		Request.AttackSequence = State.CurrentAttack.AttackSequence;
		Request.CardId = CardId;
		const auto Result = FMatchPlaySetPieceCarrierSelection::Submit(
			State, Request);
		if (OutCardId != nullptr)
		{
			*OutCardId = CardId;
		}
		return Result.bSuccess ? Result.AfterState : State;
	}

	FMatchPlayShortFreeKickMethodRequest MethodRequest(
		const FMatchPlayState& State,
		const EMatchPlayShortFreeKickMethod Method)
	{
		FMatchPlayShortFreeKickMethodRequest Request;
		Request.RequestingSide = State.RuntimeState.CurrentAttackingPlayer;
		Request.AttackSequence = State.CurrentAttack.AttackSequence;
		Request.Method = Method;
		return Request;
	}

	FMatchPlayShortFreeKickRollRequest RollRequest(
		const FMatchPlayState& State,
		const bool bDefender)
	{
		FMatchPlayShortFreeKickRollRequest Request;
		Request.RequestingSide = bDefender
			? Other(State.RuntimeState.CurrentAttackingPlayer)
			: State.RuntimeState.CurrentAttackingPlayer;
		Request.AttackSequence = State.CurrentAttack.AttackSequence;
		return Request;
	}

	FMatchPlayPostRouteRollProviderResult RollSuccess(const int32 RawD6)
	{
		FMatchPlayPostRouteRollProviderResult Result;
		Result.bSuccess = true;
		Result.RawD6 = RawD6;
		return Result;
	}

	FMatchPlayPostRouteRollProviderResult RollFailure()
	{
		FMatchPlayPostRouteRollProviderResult Result;
		Result.ErrorCode =
			EMatchPlayPostRouteRollProviderErrorCode::ProviderFailure;
		Result.ErrorMessage = TEXT("Injected Short Free Kick roll failure.");
		return Result;
	}

	class FQueueRollProvider final : public IMatchPlayPostRouteRollProvider
	{
	public:
		virtual FMatchPlayPostRouteRollProviderResult RollD6(
			const EMatchPlayCurrentAttackPostRouteRollPurpose Purpose) override
		{
			Purposes.Add(Purpose);
			return Results.IsValidIndex(Next) ? Results[Next++] : RollFailure();
		}
		TArray<FMatchPlayPostRouteRollProviderResult> Results;
		TArray<EMatchPlayCurrentAttackPostRouteRollPurpose> Purposes;
		int32 Next = 0;
	};

	class FCapturingRecoveryProvider final : public IMatchPlayRecoveryProvider
	{
	public:
		virtual FMatchPlayRecoveryProviderResult DrawWeightedWithoutReplacement(
			const EMatchPlayRecoveryPurpose Purpose,
			const TArray<FMatchPlayRecoveryCandidate>& OrderedCandidates,
			const int32 ReturnCount) override
		{
			++CallCount;
			Candidates = OrderedCandidates;
			FMatchPlayRecoveryProviderResult Result;
			if (bFail)
			{
				Result.ErrorCode =
					EMatchPlayRecoveryProviderErrorCode::ProviderFailure;
				Result.ErrorMessage = TEXT("Injected Recovery failure.");
				return Result;
			}
			Result.bSuccess = true;
			int32 PreferredIndex = INDEX_NONE;
			for (int32 Index = 0; Index < OrderedCandidates.Num(); ++Index)
			{
				if (OrderedCandidates[Index].CardId == PreferredCard)
				{
					PreferredIndex = Index;
					break;
				}
			}
			if (bSelectPreferred && PreferredIndex != INDEX_NONE
				&& Result.SelectedCandidateIndices.Num() < ReturnCount)
			{
				Result.SelectedCandidateIndices.Add(PreferredIndex);
			}
			for (int32 Index = 0;
				Index < OrderedCandidates.Num()
					&& Result.SelectedCandidateIndices.Num() < ReturnCount;
				++Index)
			{
				if (Index != PreferredIndex
					&& !Result.SelectedCandidateIndices.Contains(Index))
				{
					Result.SelectedCandidateIndices.Add(Index);
				}
			}
			if (!bSelectPreferred && PreferredIndex != INDEX_NONE
				&& Result.SelectedCandidateIndices.Num() < ReturnCount)
			{
				Result.SelectedCandidateIndices.Add(PreferredIndex);
			}
			return Result;
		}

		bool bFail = false;
		bool bSelectPreferred = false;
		FName PreferredCard = NAME_None;
		int32 CallCount = 0;
		TArray<FMatchPlayRecoveryCandidate> Candidates;
	};

	FMatchPlayShortFreeKickResolutionResult MakeDirectTerminal(
		FMatchPlayState State,
		const int32 Shooting,
		const int32 Passing,
		const int32 Handling,
		const int32 AttackD6,
		const int32 DefenseD6)
	{
		const EInitialTurnOrderPlayer Attacker =
			State.RuntimeState.CurrentAttackingPlayer;
		FindSnapshot(State, Attacker, false)->Attributes.Shooting = Shooting;
		FindSnapshot(State, Attacker, false)->Attributes.Passing = Passing;
		FindSnapshot(State, Other(Attacker), true)
			->GoalkeeperAttributes.Handling = Handling;
		State = BindCarrier(MoveTemp(State));
		State = FMatchPlayShortFreeKickResolution::SubmitMethod(
			State,
			MethodRequest(State, EMatchPlayShortFreeKickMethod::Direct))
			.AfterState;
		FQueueRollProvider Provider;
		Provider.Results = { RollSuccess(AttackD6), RollSuccess(DefenseD6) };
		State = FMatchPlayShortFreeKickResolution::ResolveDirectAttackRoll(
			State, RollRequest(State, false), &Provider).AfterState;
		return FMatchPlayShortFreeKickResolution::ResolveDirectDefenseRoll(
			State, RollRequest(State, true), &Provider);
	}

	void MoveToUsed(
		FMatchPlayState& State,
		const EInitialTurnOrderPlayer Side,
		const FName CardId)
	{
		FCardUsageState& CardUsage = Usage(State, Side);
		CardUsage.AvailableCardIds.Remove(CardId);
		CardUsage.UsedCardIds.Add(CardId);
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayShortFreeKickMethodAndValidationTest,
	"FMCodex.CoreRules.MatchPlayShortFreeKick.MethodAndStateValidation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayShortFreeKickMethodAndValidationTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayShortFreeKickResolutionTests;
	FMatchPlayState State = BindCarrier(MakeShortAwaitingCarrier(TEXT("SFK_Method")));
	const FMatchPlayState Before = State;
	const auto Direct = FMatchPlayShortFreeKickResolution::SubmitMethod(
		State, MethodRequest(State, EMatchPlayShortFreeKickMethod::Direct));
	TestTrue(TEXT("Direct method succeeds without RNG"), Direct.bSuccess);
	TestEqual(TEXT("Direct stage"),
		Direct.AfterState.CurrentAttack.SetPieceRoute.ShortFreeKick.Stage,
		EMatchPlaySetPieceCarrierRouteStage::DirectAwaitingAttackRoll);
	TestTrue(TEXT("Direct candidate validates"),
		Direct.AfterRouteValidation.bIsCanonical);
	const auto Duplicate = FMatchPlayShortFreeKickResolution::SubmitMethod(
		Direct.AfterState,
		MethodRequest(Direct.AfterState, EMatchPlayShortFreeKickMethod::Direct));
	TestFalse(TEXT("Duplicate method rejected"), Duplicate.bSuccess);

	FMatchPlayShortFreeKickMethodRequest WrongSide = MethodRequest(
		Before, EMatchPlayShortFreeKickMethod::Direct);
	WrongSide.RequestingSide = Other(WrongSide.RequestingSide);
	TestFalse(TEXT("Wrong side rejected"),
		FMatchPlayShortFreeKickResolution::SubmitMethod(Before, WrongSide).bSuccess);
	FMatchPlayShortFreeKickMethodRequest Stale = MethodRequest(
		Before, EMatchPlayShortFreeKickMethod::Direct);
	++Stale.AttackSequence;
	TestFalse(TEXT("Stale method rejected"),
		FMatchPlayShortFreeKickResolution::SubmitMethod(Before, Stale).bSuccess);
	TestTrue(TEXT("Rejected method requests preserve exact state"),
		Equal(Before,
			FMatchPlayShortFreeKickResolution::SubmitMethod(Before, Stale)
				.AfterState));

	FMatchPlayState Ineligible = MakeShortAwaitingCarrier(TEXT("SFK_Ineligible"));
	const EInitialTurnOrderPlayer Attacker =
		Ineligible.RuntimeState.CurrentAttackingPlayer;
	FindSnapshot(Ineligible, Attacker, false)->Attributes.Shooting = 3;
	FindSnapshot(Ineligible, Attacker, false)->Attributes.Passing = 4;
	Ineligible = BindCarrier(MoveTemp(Ineligible));
	const auto AngledRejected =
		FMatchPlayShortFreeKickResolution::SubmitMethod(
			Ineligible,
			MethodRequest(Ineligible, EMatchPlayShortFreeKickMethod::Angled));
	TestEqual(TEXT("Angled 7 rejected"), AngledRejected.ErrorCode,
		EMatchPlayShortFreeKickResolutionErrorCode::AngledMethodNotEligible);
	TestTrue(TEXT("Angled rejection exact state"),
		Equal(Ineligible, AngledRejected.AfterState));
	FMatchPlayState Boundary = MakeShortAwaitingCarrier(TEXT("SFK_Boundary"));
	FindSnapshot(Boundary,
		Boundary.RuntimeState.CurrentAttackingPlayer,
		false)->Attributes.Shooting = 4;
	FindSnapshot(Boundary,
		Boundary.RuntimeState.CurrentAttackingPlayer,
		false)->Attributes.Passing = 4;
	Boundary = BindCarrier(MoveTemp(Boundary));
	const auto Angled = FMatchPlayShortFreeKickResolution::SubmitMethod(
		Boundary,
		MethodRequest(Boundary, EMatchPlayShortFreeKickMethod::Angled));
	TestTrue(TEXT("Angled 8 accepted"), Angled.bSuccess);
	TestEqual(TEXT("Angled stage"),
		Angled.AfterState.CurrentAttack.SetPieceRoute.ShortFreeKick.Stage,
		EMatchPlaySetPieceCarrierRouteStage::AngledAwaitingRoll);

	FMatchPlayState WrongStage = MakeShortAwaitingCarrier(TEXT("SFK_WrongStage"));
	TestFalse(TEXT("Method before Carrier is rejected"),
		FMatchPlayShortFreeKickResolution::SubmitMethod(
			WrongStage,
			MethodRequest(WrongStage, EMatchPlayShortFreeKickMethod::Direct))
			.bSuccess);
	FMatchPlayState MissingCarrier = Before;
	MissingCarrier.CurrentAttack.SetPieceRoute.ShortFreeKick.Carrier =
		FMatchPlaySetPieceParticipantBinding();
	TestFalse(TEXT("Corrupt missing Carrier is rejected"),
		FMatchPlayShortFreeKickResolution::SubmitMethod(
			MissingCarrier,
			MethodRequest(MissingCarrier, EMatchPlayShortFreeKickMethod::Direct))
			.bSuccess);
	FMatchPlayState WrongType = MakeShortAwaitingCarrier(TEXT("SFK_WrongType"));
	WrongType.CurrentAttack.SetPieceRoute.RawTypeD6 = 3;
	WrongType.CurrentAttack.SetPieceRoute.SelectedType =
		ESetPieceSelectedType::LongFreeKick;
	WrongType.CurrentAttack.SetPieceRoute.ShortFreeKick =
		FMatchPlayShortFreeKickRouteState();
	WrongType.CurrentAttack.SetPieceRoute.LongFreeKick.Stage =
		EMatchPlaySetPieceCarrierRouteStage::AwaitingCarrier;
	WrongType = BindCarrier(MoveTemp(WrongType));
	TestEqual(TEXT("Wrong concrete Set Piece type rejected"),
		FMatchPlayShortFreeKickResolution::SubmitMethod(
			WrongType,
			MethodRequest(WrongType, EMatchPlayShortFreeKickMethod::Direct))
			.ErrorCode,
		EMatchPlayShortFreeKickResolutionErrorCode::WrongSetPieceType);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayShortFreeKickDirectFormulaTest,
	"FMCodex.CoreRules.MatchPlayShortFreeKick.DirectFormulaAndRetry",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayShortFreeKickDirectFormulaTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayShortFreeKickResolutionTests;
	const auto Goal = MakeDirectTerminal(
		MakeShortAwaitingCarrier(TEXT("SFK_DirectGoal")), 6, 4, 3, 3, 1);
	TestTrue(TEXT("Direct goal terminal succeeds"), Goal.bSuccess);
	const auto& GoalShort =
		Goal.AfterState.CurrentAttack.SetPieceRoute.ShortFreeKick;
	TestEqual(TEXT("Shooting max is attacker base plus roll"),
		GoalShort.FormulaResolution.AttackerFinalValue, 9.0f);
	TestEqual(TEXT("GK Handling plus one and defense roll exactly once"),
		GoalShort.FormulaResolution.DefenderFinalValue, 5.0f);
	TestTrue(TEXT("Direct clear win is Goal"),
		GoalShort.GameplayOutcome
			== EMatchPlayShortFreeKickGameplayOutcome::Goal
			&& GoalShort.GoalScorerCardId == GoalShort.Carrier.CardId);
	TestEqual(TEXT("Score increments exactly once"),
		Goal.AfterState.RuntimeState.PlayerAState.Score
			+ Goal.AfterState.RuntimeState.PlayerBState.Score, 1);

	const auto Tie = MakeDirectTerminal(
		MakeShortAwaitingCarrier(TEXT("SFK_DirectTie")), 3, 5, 4, 2, 2);
	TestTrue(TEXT("Tie terminal succeeds"), Tie.bSuccess);
	const auto& TieFormula =
		Tie.AfterState.CurrentAttack.SetPieceRoute.ShortFreeKick.FormulaResolution;
	TestEqual(TEXT("Passing is selected when higher"),
		TieFormula.AttackerFinalValue, 7.0f);
	TestEqual(TEXT("Exact total tie"), TieFormula.DefenderFinalValue, 7.0f);
	TestEqual(TEXT("GK participation owns tie"), TieFormula.WinReason,
		EFormulaWinReason::DefenderWinsGoalkeeperTie);
	TestFalse(TEXT("Tie is NoGoal"), TieFormula.bIsGoal);
	const auto EqualAttributes = MakeDirectTerminal(
		MakeShortAwaitingCarrier(TEXT("SFK_DirectEqual")), 5, 5, 2, 1, 1);
	TestEqual(TEXT("Equal Shooting and Passing are deterministic"),
		EqualAttributes.AfterState.CurrentAttack.SetPieceRoute.ShortFreeKick
			.FormulaResolution.AttackerFinalValue,
		6.0f);
	const auto DefenderWin = MakeDirectTerminal(
		MakeShortAwaitingCarrier(TEXT("SFK_DirectDefender")), 2, 3, 6, 1, 6);
	TestTrue(TEXT("Defender clear win is NoGoal with no scorer"),
		DefenderWin.bSuccess
			&& DefenderWin.AfterState.CurrentAttack.SetPieceRoute.ShortFreeKick
				.GameplayOutcome
					== EMatchPlayShortFreeKickGameplayOutcome::NoGoal
			&& !DefenderWin.AfterState.CurrentAttack.SetPieceRoute.ShortFreeKick
				.bHasGoalScorer);

	FMatchPlayState Retry = BindCarrier(
		MakeShortAwaitingCarrier(TEXT("SFK_DirectRetry")));
	Retry = FMatchPlayShortFreeKickResolution::SubmitMethod(
		Retry,
		MethodRequest(Retry, EMatchPlayShortFreeKickMethod::Direct)).AfterState;
	FQueueRollProvider WrongSideProvider;
	WrongSideProvider.Results = { RollSuccess(6) };
	FMatchPlayShortFreeKickRollRequest WrongSideRoll = RollRequest(Retry, false);
	WrongSideRoll.RequestingSide = Other(WrongSideRoll.RequestingSide);
	TestFalse(TEXT("Wrong-side Direct attack request rejected"),
		FMatchPlayShortFreeKickResolution::ResolveDirectAttackRoll(
			Retry, WrongSideRoll, &WrongSideProvider).bSuccess);
	TestEqual(TEXT("Wrong-side Direct request consumes zero RNG"),
		WrongSideProvider.Purposes.Num(), 0);
	FQueueRollProvider AttackFailure;
	AttackFailure.Results = { RollFailure() };
	const FMatchPlayState BeforeAttack = Retry;
	const auto FailedAttack =
		FMatchPlayShortFreeKickResolution::ResolveDirectAttackRoll(
			Retry, RollRequest(Retry, false), &AttackFailure);
	TestFalse(TEXT("Attack provider failure rejected"), FailedAttack.bSuccess);
	TestTrue(TEXT("Attack failure consumes exactly the Attack purpose"),
		AttackFailure.Purposes.Num() == 1
			&& AttackFailure.Purposes[0]
				== EMatchPlayCurrentAttackPostRouteRollPurpose
					::ShortFreeKickDirectAttack);
	TestTrue(TEXT("Attack failure preserves exact prefix"),
		Equal(BeforeAttack, FailedAttack.AfterState));

	FQueueRollProvider AttackSuccess;
	AttackSuccess.Results = { RollSuccess(4) };
	Retry = FMatchPlayShortFreeKickResolution::ResolveDirectAttackRoll(
		Retry, RollRequest(Retry, false), &AttackSuccess).AfterState;
	TestTrue(TEXT("Attack retry consumes exactly one Attack purpose"),
		AttackSuccess.Purposes.Num() == 1
			&& AttackSuccess.Purposes[0]
				== EMatchPlayCurrentAttackPostRouteRollPurpose
					::ShortFreeKickDirectAttack);
	FQueueRollProvider DuplicateAttackProvider;
	DuplicateAttackProvider.Results = { RollSuccess(6) };
	TestFalse(TEXT("Duplicate Direct attack roll rejected"),
		FMatchPlayShortFreeKickResolution::ResolveDirectAttackRoll(
			Retry, RollRequest(Retry, false), &DuplicateAttackProvider).bSuccess);
	TestEqual(TEXT("Duplicate Direct attack consumes zero RNG"),
		DuplicateAttackProvider.Purposes.Num(), 0);
	FQueueRollProvider WrongDefenseSideProvider;
	WrongDefenseSideProvider.Results = { RollSuccess(6) };
	FMatchPlayShortFreeKickRollRequest WrongDefenseSide =
		RollRequest(Retry, true);
	WrongDefenseSide.RequestingSide = Other(WrongDefenseSide.RequestingSide);
	TestFalse(TEXT("Wrong-side Direct defense request rejected"),
		FMatchPlayShortFreeKickResolution::ResolveDirectDefenseRoll(
			Retry, WrongDefenseSide, &WrongDefenseSideProvider).bSuccess);
	TestEqual(TEXT("Wrong-side Direct defense consumes zero RNG"),
		WrongDefenseSideProvider.Purposes.Num(), 0);
	FQueueRollProvider DefenseFailure;
	DefenseFailure.Results = { RollFailure() };
	const FMatchPlayState BeforeDefense = Retry;
	const auto FailedDefense =
		FMatchPlayShortFreeKickResolution::ResolveDirectDefenseRoll(
			Retry, RollRequest(Retry, true), &DefenseFailure);
	TestFalse(TEXT("Defense provider failure rejected"), FailedDefense.bSuccess);
	TestTrue(TEXT("Defense failure consumes exactly the Defense purpose"),
		DefenseFailure.Purposes.Num() == 1
			&& DefenseFailure.Purposes[0]
				== EMatchPlayCurrentAttackPostRouteRollPurpose
					::ShortFreeKickDirectDefense);
	TestTrue(TEXT("Defense failure preserves stored attack roll"),
		Equal(BeforeDefense, FailedDefense.AfterState)
			&& FailedDefense.AfterState.CurrentAttack.SetPieceRoute
				.ShortFreeKick.AttackD6 == 4);
	const int32 AttackCallsBeforeDefenseRetry = AttackSuccess.Purposes.Num();
	FQueueRollProvider DefenseRetry;
	DefenseRetry.Results = { RollSuccess(1) };
	const auto RetriedDefense =
		FMatchPlayShortFreeKickResolution::ResolveDirectDefenseRoll(
			Retry, RollRequest(Retry, true), &DefenseRetry);
	TestTrue(TEXT("Defense-only retry reaches terminal"),
		RetriedDefense.bSuccess
			&& RetriedDefense.AfterState.CurrentAttack.LifecycleState
				== EMatchPlayCurrentAttackLifecycleState
					::TerminalPendingAdvance);
	TestTrue(TEXT("Defense retry consumes exactly one Defense purpose"),
		DefenseRetry.Purposes.Num() == 1
			&& DefenseRetry.Purposes[0]
				== EMatchPlayCurrentAttackPostRouteRollPurpose
					::ShortFreeKickDirectDefense);
	TestEqual(TEXT("Defense retry does not reroll Attack"),
		AttackSuccess.Purposes.Num(), AttackCallsBeforeDefenseRetry);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayShortFreeKickAngledTest,
	"FMCodex.CoreRules.MatchPlayShortFreeKick.AngledAtomicThresholds",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayShortFreeKickAngledTest::RunTest(const FString& Parameters)
{
	using namespace MatchPlayShortFreeKickResolutionTests;
	auto ResolvePair = [this](const FString& Prefix, const int32 A, const int32 B)
	{
		FMatchPlayState State = BindCarrier(MakeShortAwaitingCarrier(Prefix));
		State = FMatchPlayShortFreeKickResolution::SubmitMethod(
			State,
			MethodRequest(State, EMatchPlayShortFreeKickMethod::Angled))
			.AfterState;
		FQueueRollProvider Provider;
		Provider.Results = { RollSuccess(A), RollSuccess(B) };
		const auto Result = FMatchPlayShortFreeKickResolution::ResolveAngledRoll(
			State, RollRequest(State, false), &Provider);
		TestEqual(TEXT("Angled consumes exactly two ordered semantic dice"),
			Provider.Purposes.Num(), 2);
		TestTrue(TEXT("Angled A/B purposes are explicit"),
			Provider.Purposes.Num() == 2
				&& Provider.Purposes[0]
					== EMatchPlayCurrentAttackPostRouteRollPurpose
						::ShortFreeKickAngledA
				&& Provider.Purposes[1]
					== EMatchPlayCurrentAttackPostRouteRollPurpose
						::ShortFreeKickAngledB);
		return Result;
	};
	const auto Sum8 = ResolvePair(TEXT("SFK_Angled8"), 4, 4);
	TestEqual(TEXT("Angled sum 8 NoGoal"),
		Sum8.AfterState.CurrentAttack.SetPieceRoute.ShortFreeKick.GameplayOutcome,
		EMatchPlayShortFreeKickGameplayOutcome::NoGoal);
	const auto Sum9 = ResolvePair(TEXT("SFK_Angled9"), 4, 5);
	TestEqual(TEXT("Angled sum 9 Goal"),
		Sum9.AfterState.CurrentAttack.SetPieceRoute.ShortFreeKick.GameplayOutcome,
		EMatchPlayShortFreeKickGameplayOutcome::Goal);
	const auto Sum12 = ResolvePair(TEXT("SFK_Angled12"), 6, 6);
	TestTrue(TEXT("Angled sum 12 scores Carrier"),
		Sum12.AfterState.CurrentAttack.SetPieceRoute.ShortFreeKick
			.GoalScorerCardId
			== Sum12.AfterState.CurrentAttack.SetPieceRoute.ShortFreeKick
				.Carrier.CardId);

	FMatchPlayState Atomic = BindCarrier(
		MakeShortAwaitingCarrier(TEXT("SFK_AngledFailure")));
	Atomic = FMatchPlayShortFreeKickResolution::SubmitMethod(
		Atomic,
		MethodRequest(Atomic, EMatchPlayShortFreeKickMethod::Angled)).AfterState;
	FQueueRollProvider Failure;
	Failure.Results = { RollSuccess(6), RollFailure() };
	const auto Failed = FMatchPlayShortFreeKickResolution::ResolveAngledRoll(
		Atomic, RollRequest(Atomic, false), &Failure);
	TestFalse(TEXT("Second pair die failure rejects transaction"), Failed.bSuccess);
	TestTrue(TEXT("Pair failure publishes no partial die"),
		Equal(Atomic, Failed.AfterState)
			&& !Failed.AfterState.CurrentAttack.SetPieceRoute.ShortFreeKick
				.bHasAngledD6Pair);
	FQueueRollProvider DuplicateProvider;
	DuplicateProvider.Results = { RollSuccess(1), RollSuccess(1) };
	const auto Duplicate = FMatchPlayShortFreeKickResolution::ResolveAngledRoll(
		Sum9.AfterState, RollRequest(Sum9.AfterState, false), &DuplicateProvider);
	TestFalse(TEXT("Terminal duplicate rejected"), Duplicate.bSuccess);
	TestEqual(TEXT("Terminal duplicate consumes zero RNG"),
		DuplicateProvider.Purposes.Num(), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayShortFreeKickNoLegalCarrierTest,
	"FMCodex.CoreRules.MatchPlayShortFreeKick.NoLegalCarrier",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayShortFreeKickNoLegalCarrierTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayShortFreeKickResolutionTests;
	FMatchPlayState Legal = MakeShortAwaitingCarrier(TEXT("SFK_NoCarrierLegal"));
	const EInitialTurnOrderPlayer LegalAttacker =
		Legal.RuntimeState.CurrentAttackingPlayer;
	const FName SoleLegalCarrier = FindCard(Legal, LegalAttacker, false);
	FCardUsageState& LegalUsage = Usage(Legal, LegalAttacker);
	for (const FName CardId : TArray<FName>(LegalUsage.AvailableCardIds))
	{
		if (CardId != SoleLegalCarrier
			&& CardId != FindCard(Legal, LegalAttacker, true))
		{
			LegalUsage.AvailableCardIds.Remove(CardId);
			LegalUsage.UsedCardIds.Add(CardId);
		}
	}
	FMatchPlayShortFreeKickNoLegalCarrierRequest Request;
	Request.RequestingSide = Legal.RuntimeState.CurrentAttackingPlayer;
	Request.AttackSequence = Legal.CurrentAttack.AttackSequence;
	const auto Rejected =
		FMatchPlayShortFreeKickResolution::ResolveNoLegalCarrier(Legal, Request);
	TestEqual(TEXT("Exactly one legal Carrier blocks shortage claim"), Rejected.ErrorCode,
		EMatchPlayShortFreeKickResolutionErrorCode::LegalCarrierExists);
	TestTrue(TEXT("Rejected shortage claim preserves exact state"),
		Equal(Legal, Rejected.AfterState));

	FMatchPlayState Empty = MakeShortAwaitingCarrier(TEXT("SFK_NoCarrierEmpty"));
	const EInitialTurnOrderPlayer Attacker =
		Empty.RuntimeState.CurrentAttackingPlayer;
	FCardUsageState& AttackerUsage = Usage(Empty, Attacker);
	const FName Goalkeeper = FindCard(Empty, Attacker, true);
	for (const FName CardId : TArray<FName>(AttackerUsage.AvailableCardIds))
	{
		if (CardId != Goalkeeper)
		{
			AttackerUsage.AvailableCardIds.Remove(CardId);
			AttackerUsage.UsedCardIds.Add(CardId);
		}
	}
	Request.RequestingSide = Attacker;
	Request.AttackSequence = Empty.CurrentAttack.AttackSequence;
	const int32 ScoreBefore = Empty.RuntimeState.PlayerAState.Score
		+ Empty.RuntimeState.PlayerBState.Score;
	const auto Terminal =
		FMatchPlayShortFreeKickResolution::ResolveNoLegalCarrier(Empty, Request);
	TestTrue(TEXT("Only GK available proves no legal Carrier"), Terminal.bSuccess);
	const auto& Short =
		Terminal.AfterState.CurrentAttack.SetPieceRoute.ShortFreeKick;
	TestTrue(TEXT("Shortage persists no-Carrier NoGoal terminal"),
		Short.bNoLegalCarrier
			&& Short.GameplayOutcome
				== EMatchPlayShortFreeKickGameplayOutcome::NoGoal
			&& Terminal.AfterState.CurrentAttack.LifecycleState
				== EMatchPlayCurrentAttackLifecycleState::TerminalPendingAdvance);
	TestFalse(TEXT("No Carrier means no scorer"), Short.bHasGoalScorer);
	TestEqual(TEXT("No-Carrier preserves score"),
		Terminal.AfterState.RuntimeState.PlayerAState.Score
			+ Terminal.AfterState.RuntimeState.PlayerBState.Score,
		ScoreBefore);

	FCapturingRecoveryProvider Recovery;
	const auto Advanced = FMatchPlayCurrentAttackCompletion::AdvanceAfterTerminal(
		Terminal.AfterState,
		Terminal.AfterState.CurrentAttack.AttackSequence,
		Attacker,
		&Recovery);
	TestTrue(TEXT("No-Carrier non-final Advance succeeds"), Advanced.bSuccess);
	TestEqual(TEXT("No-Carrier consumes zero Set Piece cards"),
		Advanced.SetPieceCardUsageResults.Num(), 0);
	TestEqual(TEXT("No-Carrier still consumes one opportunity"),
		(Attacker == EInitialTurnOrderPlayer::PlayerA
			? Advanced.AfterState.RuntimeState.PlayerAState.UsedAttackCount
			: Advanced.AfterState.RuntimeState.PlayerBState.UsedAttackCount),
		1);
	TestTrue(TEXT("No-Carrier non-final path still runs shared Recovery"),
		Recovery.CallCount == 1 && !Recovery.Candidates.IsEmpty());

	FCapturingRecoveryProvider FailedRecovery;
	FailedRecovery.bFail = true;
	const auto FailedAdvance =
		FMatchPlayCurrentAttackCompletion::AdvanceAfterTerminal(
			Terminal.AfterState,
			Terminal.AfterState.CurrentAttack.AttackSequence,
			Attacker,
			&FailedRecovery);
	TestFalse(TEXT("No-Carrier Recovery failure rolls back Advance"),
		FailedAdvance.bSuccess);
	TestTrue(TEXT("No-Carrier failed Advance preserves terminal"),
		Equal(Terminal.AfterState, FailedAdvance.AfterState));

	FMatchPlayState Final = Terminal.AfterState;
	FPlayerRuntimeState& FinalAttacker = Attacker
		== EInitialTurnOrderPlayer::PlayerA
			? Final.RuntimeState.PlayerAState
			: Final.RuntimeState.PlayerBState;
	FPlayerRuntimeState& FinalDefender = Attacker
		== EInitialTurnOrderPlayer::PlayerA
			? Final.RuntimeState.PlayerBState
			: Final.RuntimeState.PlayerAState;
	FinalAttacker.TotalAttackCount = 1;
	FinalAttacker.UsedAttackCount = 0;
	FinalDefender.TotalAttackCount = 1;
	FinalDefender.UsedAttackCount = 1;
	Final.CurrentAttack.AttackSequence = 2;
	FCapturingRecoveryProvider FinalProvider;
	const auto FinalAdvance = FMatchPlayCurrentAttackCompletion::AdvanceAfterTerminal(
		Final, 2, Attacker, &FinalProvider);
	TestTrue(TEXT("No-Carrier final Advance resolves match"),
		FinalAdvance.bSuccess && FinalAdvance.bMatchEnded);
	TestEqual(TEXT("No-Carrier final Advance skips Recovery"),
		FinalProvider.CallCount, 0);

	FMatchPlayState Ejected = MakeShortAwaitingCarrier(TEXT("SFK_NoCarrierEjected"));
	const EInitialTurnOrderPlayer EjectedAttacker =
		Ejected.RuntimeState.CurrentAttackingPlayer;
	FCardUsageState& EjectedUsage = Usage(Ejected, EjectedAttacker);
	for (const FPlayerCardRuleSnapshot& Snapshot :
		Snapshots(Ejected, EjectedAttacker).Cards)
	{
		if (!Snapshot.bIsGoalkeeper)
		{
			EjectedUsage.AvailableCardIds.Remove(Snapshot.CardId);
			EjectedUsage.EjectedCardIds.Add(Snapshot.CardId);
		}
	}
	FMatchPlayShortFreeKickNoLegalCarrierRequest EjectedRequest;
	EjectedRequest.RequestingSide = EjectedAttacker;
	EjectedRequest.AttackSequence = 1;
	TestTrue(TEXT("Only Ejected non-GKs also prove zero legal Carrier"),
		FMatchPlayShortFreeKickResolution::ResolveNoLegalCarrier(
			Ejected, EjectedRequest).bSuccess);

	FMatchPlayState TrulyEmpty =
		MakeShortAwaitingCarrier(TEXT("SFK_NoCarrierTrulyEmpty"));
	const EInitialTurnOrderPlayer EmptyAttacker =
		TrulyEmpty.RuntimeState.CurrentAttackingPlayer;
	Usage(TrulyEmpty, EmptyAttacker).AvailableCardIds.Reset();
	FMatchPlayShortFreeKickNoLegalCarrierRequest EmptyRequest;
	EmptyRequest.RequestingSide = EmptyAttacker;
	EmptyRequest.AttackSequence = 1;
	TestTrue(TEXT("Truly empty Available pool proves zero legal Carrier"),
		FMatchPlayShortFreeKickResolution::ResolveNoLegalCarrier(
			TrulyEmpty, EmptyRequest).bSuccess);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayShortFreeKickAdvanceRecoveryTest,
	"FMCodex.CoreRules.MatchPlayShortFreeKick.AdvanceRecoveryAtomicity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayShortFreeKickAdvanceRecoveryTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayShortFreeKickResolutionTests;
	FMatchPlayState Base = MakeShortAwaitingCarrier(TEXT("SFK_Advance"));
	const EInitialTurnOrderPlayer Attacker =
		Base.RuntimeState.CurrentAttackingPlayer;
	FName Carrier = NAME_None;
	Base = BindCarrier(MoveTemp(Base), &Carrier);
	for (int32 Index = 1; Index <= 3; ++Index)
	{
		MoveToUsed(Base, Attacker, FindCard(Base, Attacker, false, Index));
	}
	Base = FMatchPlayShortFreeKickResolution::SubmitMethod(
		Base,
		MethodRequest(Base, EMatchPlayShortFreeKickMethod::Angled)).AfterState;
	FQueueRollProvider Rolls;
	Rolls.Results = { RollSuccess(4), RollSuccess(4) };
	const auto TerminalResult = FMatchPlayShortFreeKickResolution::ResolveAngledRoll(
		Base, RollRequest(Base, false), &Rolls);
	const FMatchPlayState Terminal = TerminalResult.AfterState;
	TestTrue(TEXT("Carrier remains Available before acknowledge"),
		Usage(Terminal, Attacker).AvailableCardIds.Contains(Carrier));

	FCapturingRecoveryProvider NoCarrierRecovery;
	NoCarrierRecovery.PreferredCard = Carrier;
	const auto Advanced = FMatchPlayCurrentAttackCompletion::AdvanceAfterTerminal(
		Terminal, Terminal.CurrentAttack.AttackSequence, Attacker,
		&NoCarrierRecovery);
	TestTrue(TEXT("Non-final shared Advance succeeds"), Advanced.bSuccess);
	TestTrue(TEXT("Recovery query sees newly Used Carrier"),
		NoCarrierRecovery.Candidates.ContainsByPredicate(
			[Carrier](const FMatchPlayRecoveryCandidate& Candidate)
			{ return Candidate.CardId == Carrier; }));
	TestTrue(TEXT("Unselected Carrier remains Used"),
		Usage(Advanced.AfterState, Attacker).UsedCardIds.Contains(Carrier));
	TestFalse(TEXT("Successful Advance clears CurrentAttack"),
		Advanced.AfterState.bHasCurrentAttack);

	FCapturingRecoveryProvider RecoverCarrier;
	RecoverCarrier.PreferredCard = Carrier;
	RecoverCarrier.bSelectPreferred = true;
	const auto Recovered = FMatchPlayCurrentAttackCompletion::AdvanceAfterTerminal(
		Terminal, Terminal.CurrentAttack.AttackSequence, Attacker,
		&RecoverCarrier);
	TestTrue(TEXT("Same transaction may recover Carrier"),
		Recovered.bSuccess
			&& Usage(Recovered.AfterState, Attacker)
				.AvailableCardIds.Contains(Carrier));

	FCapturingRecoveryProvider Failure;
	Failure.bFail = true;
	const auto Failed = FMatchPlayCurrentAttackCompletion::AdvanceAfterTerminal(
		Terminal, Terminal.CurrentAttack.AttackSequence, Attacker, &Failure);
	TestFalse(TEXT("Recovery failure fails whole Advance"), Failed.bSuccess);
	TestTrue(TEXT("Recovery failure preserves terminal snapshot"),
		Equal(Terminal, Failed.AfterState)
			&& Usage(Failed.AfterState, Attacker)
				.AvailableCardIds.Contains(Carrier));

	FMatchPlayState Final = Terminal;
	FPlayerRuntimeState& AttackerRuntime = Attacker == EInitialTurnOrderPlayer::PlayerA
		? Final.RuntimeState.PlayerAState : Final.RuntimeState.PlayerBState;
	FPlayerRuntimeState& DefenderRuntime = Attacker == EInitialTurnOrderPlayer::PlayerA
		? Final.RuntimeState.PlayerBState : Final.RuntimeState.PlayerAState;
	AttackerRuntime.TotalAttackCount = 1;
	AttackerRuntime.UsedAttackCount = 0;
	DefenderRuntime.TotalAttackCount = 1;
	DefenderRuntime.UsedAttackCount = 1;
	Final.CurrentAttack.AttackSequence = 2;
	FCapturingRecoveryProvider FinalRecovery;
	const auto FinalAdvance = FMatchPlayCurrentAttackCompletion::AdvanceAfterTerminal(
		Final, 2, Attacker, &FinalRecovery);
	TestTrue(TEXT("Final Advance succeeds and consumes Carrier"),
		FinalAdvance.bSuccess
			&& Usage(FinalAdvance.AfterState, Attacker).UsedCardIds.Contains(Carrier));
	TestTrue(TEXT("Final result produced"),
		FinalAdvance.bMatchEnded
			&& FinalAdvance.MatchResultResolveResult.bSuccess);
	TestEqual(TEXT("Final Advance skips Recovery provider"),
		FinalRecovery.CallCount, 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayShortFreeKickReconstructionTest,
	"FMCodex.CoreRules.MatchPlayShortFreeKick.ReconstructionStates",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayShortFreeKickReconstructionTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayShortFreeKickResolutionTests;
	FMatchPlayState AwaitingCarrier = MakeShortAwaitingCarrier(TEXT("SFK_Rebuild"));
	TestTrue(TEXT("AwaitingCarrier reconstructs"),
		FMatchPlayCurrentAttackRouteStateValidator::Validate(AwaitingCarrier)
			.bIsCanonical);
	FMatchPlayState AwaitingMethod = BindCarrier(AwaitingCarrier);
	TestTrue(TEXT("AwaitingMethod reconstructs"),
		FMatchPlayCurrentAttackRouteStateValidator::Validate(AwaitingMethod)
			.bIsCanonical);
	FMatchPlayState DirectAttack =
		FMatchPlayShortFreeKickResolution::SubmitMethod(
			AwaitingMethod,
			MethodRequest(AwaitingMethod,
				EMatchPlayShortFreeKickMethod::Direct)).AfterState;
	TestTrue(TEXT("Direct AwaitingAttack reconstructs"),
		FMatchPlayCurrentAttackRouteStateValidator::Validate(DirectAttack)
			.bIsCanonical);
	FQueueRollProvider Provider;
	Provider.Results = { RollSuccess(3) };
	FMatchPlayState DirectDefense =
		FMatchPlayShortFreeKickResolution::ResolveDirectAttackRoll(
			DirectAttack, RollRequest(DirectAttack, false), &Provider).AfterState;
	TestTrue(TEXT("Direct AwaitingDefense with prefix reconstructs"),
		FMatchPlayCurrentAttackRouteStateValidator::Validate(DirectDefense)
			.bIsCanonical);
	FMatchPlayState Angled =
		FMatchPlayShortFreeKickResolution::SubmitMethod(
			AwaitingMethod,
			MethodRequest(AwaitingMethod,
				EMatchPlayShortFreeKickMethod::Angled)).AfterState;
	TestTrue(TEXT("Angled AwaitingRoll reconstructs"),
		FMatchPlayCurrentAttackRouteStateValidator::Validate(Angled)
			.bIsCanonical);
	const auto Goal = MakeDirectTerminal(
		MakeShortAwaitingCarrier(TEXT("SFK_RebuildGoal")), 6, 4, 3, 3, 1);
	const auto NoGoal = MakeDirectTerminal(
		MakeShortAwaitingCarrier(TEXT("SFK_RebuildNoGoal")), 2, 2, 6, 1, 6);
	TestTrue(TEXT("Goal terminal reconstructs"),
		FMatchPlayCurrentAttackRouteStateValidator::Validate(Goal.AfterState)
			.bIsCanonical);
	TestTrue(TEXT("NoGoal terminal reconstructs"),
		FMatchPlayCurrentAttackRouteStateValidator::Validate(NoGoal.AfterState)
			.bIsCanonical);
	FMatchPlayState Corrupt = Goal.AfterState;
	Corrupt.CurrentAttack.SetPieceRoute.ShortFreeKick.GoalScorerCardId = NAME_None;
	TestFalse(TEXT("Corrupt terminal scorer rejected"),
		FMatchPlayCurrentAttackRouteStateValidator::Validate(Corrupt)
			.bIsCanonical);
	FMatchPlayState CorruptFormula = Goal.AfterState;
	CorruptFormula.CurrentAttack.SetPieceRoute.ShortFreeKick
		.FormulaResolution.DefenderFinalValue += 1.0f;
	TestFalse(TEXT("Corrupt reconstructed Formula result rejected"),
		FMatchPlayCurrentAttackRouteStateValidator::Validate(CorruptFormula)
			.bIsCanonical);
	return true;
}

#endif
