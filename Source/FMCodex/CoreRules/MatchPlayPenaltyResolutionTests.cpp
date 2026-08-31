#include "MatchPlayOpeningInitializer.h"
#include "MatchPlayPenaltyResolution.h"
#include "MatchPlaySetPieceCarrierSelection.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

namespace MatchPlayPenaltyResolutionTests
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
		Card.GoalkeeperAttributes.Anticipation = 4;
		return Card;
	}

	TArray<FPlayerCardData> MakeDeck(const FString& Prefix)
	{
		TArray<FPlayerCardData> Deck;
		for (int32 Index = 0; Index < 19; ++Index)
		{
			Deck.Add(MakeCard(
				FString::Printf(TEXT("%s_OUT_%02d"), *Prefix, Index), false));
		}
		Deck.Add(MakeCard(Prefix + TEXT("_GK"), true));
		return Deck;
	}

	FMatchPlayState MakeAwaitingCarrier(const FString& Prefix)
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
		State.CurrentAttack.ActionPoint = 12;
		State.CurrentAttack.RawInitialD12 = 12;
		State.CurrentAttack.RouteKind =
			EMatchPlayCurrentAttackRouteKind::SetPiece;
		State.CurrentAttack.SetPieceRoute.Stage =
			EMatchPlaySetPieceRouteStage::TypeResolved;
		State.CurrentAttack.SetPieceRoute.bHasTypeRoll = true;
		State.CurrentAttack.SetPieceRoute.RawTypeD6 = 6;
		State.CurrentAttack.SetPieceRoute.SelectedType =
			ESetPieceSelectedType::Penalty;
		State.CurrentAttack.SetPieceRoute.Penalty.Stage =
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
		FMatchPlayState& State, const EInitialTurnOrderPlayer Side)
	{
		return Side == EInitialTurnOrderPlayer::PlayerA
			? State.CardSnapshotAuthority.PlayerACardSnapshots
			: State.CardSnapshotAuthority.PlayerBCardSnapshots;
	}

	const FPlayerCardRuleSnapshotSet& Snapshots(
		const FMatchPlayState& State, const EInitialTurnOrderPlayer Side)
	{
		return Side == EInitialTurnOrderPlayer::PlayerA
			? State.CardSnapshotAuthority.PlayerACardSnapshots
			: State.CardSnapshotAuthority.PlayerBCardSnapshots;
	}

	FCardUsageState& Usage(
		FMatchPlayState& State, const EInitialTurnOrderPlayer Side)
	{
		return Side == EInitialTurnOrderPlayer::PlayerA
			? State.CardUsageState.PlayerACardUsageState
			: State.CardUsageState.PlayerBCardUsageState;
	}

	const FCardUsageState& Usage(
		const FMatchPlayState& State, const EInitialTurnOrderPlayer Side)
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
		const auto Result =
			FMatchPlaySetPieceCarrierSelection::Submit(State, Request);
		if (OutCardId != nullptr)
		{
			*OutCardId = CardId;
		}
		return Result.bSuccess ? Result.AfterState : State;
	}

	FMatchPlayPenaltyMethodRequest MethodRequest(
		const FMatchPlayState& State, const EMatchPlayPenaltyMethod Method)
	{
		FMatchPlayPenaltyMethodRequest Request;
		Request.RequestingSide = State.RuntimeState.CurrentAttackingPlayer;
		Request.AttackSequence = State.CurrentAttack.AttackSequence;
		Request.Method = Method;
		return Request;
	}

	FMatchPlayPenaltyRollRequest RollRequest(
		const FMatchPlayState& State, const bool bDefender)
	{
		FMatchPlayPenaltyRollRequest Request;
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
		Result.ErrorMessage = TEXT("Injected Penalty roll failure.");
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
			EMatchPlayRecoveryPurpose Purpose,
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
				Result.ErrorMessage = TEXT("Injected Penalty Recovery failure.");
				return Result;
			}
			Result.bSuccess = true;
			if (bSelectPreferred)
			{
				for (int32 Index = 0; Index < OrderedCandidates.Num(); ++Index)
				{
					if (OrderedCandidates[Index].CardId == PreferredCard)
					{
						Result.SelectedCandidateIndices.Add(Index);
						break;
					}
				}
			}
			for (int32 Index = 0;
				Index < OrderedCandidates.Num()
					&& Result.SelectedCandidateIndices.Num() < ReturnCount;
				++Index)
			{
				if (!Result.SelectedCandidateIndices.Contains(Index)
					&& (bSelectPreferred
						|| OrderedCandidates[Index].CardId != PreferredCard))
				{
					Result.SelectedCandidateIndices.Add(Index);
				}
			}
			return Result;
		}

		bool bFail = false;
		bool bSelectPreferred = false;
		FName PreferredCard = NAME_None;
		int32 CallCount = 0;
		TArray<FMatchPlayRecoveryCandidate> Candidates;
	};

	FMatchPlayPenaltyResolutionResult MakeDirectTerminal(
		FMatchPlayState State,
		const int32 Shooting,
		const int32 Passing,
		const int32 Anticipation,
		const int32 AttackD6,
		const int32 DefenseD6)
	{
		const EInitialTurnOrderPlayer Attacker =
			State.RuntimeState.CurrentAttackingPlayer;
		FindSnapshot(State, Attacker, false)->Attributes.Shooting = Shooting;
		FindSnapshot(State, Attacker, false)->Attributes.Passing = Passing;
		FindSnapshot(State, Other(Attacker), true)
			->GoalkeeperAttributes.Anticipation = Anticipation;
		State = BindCarrier(MoveTemp(State));
		State = FMatchPlayPenaltyResolution::SubmitMethod(
			State, MethodRequest(State, EMatchPlayPenaltyMethod::Direct))
			.AfterState;
		FQueueRollProvider Provider;
		Provider.Results = { RollSuccess(AttackD6), RollSuccess(DefenseD6) };
		State = FMatchPlayPenaltyResolution::ResolveDirectAttackRoll(
			State, RollRequest(State, false), &Provider).AfterState;
		return FMatchPlayPenaltyResolution::ResolveDirectDefenseRoll(
			State, RollRequest(State, true), &Provider);
	}

	FMatchPlayPenaltyResolutionResult MakePanenkaTerminal(
		FMatchPlayState State, const int32 RawD6, FName* OutCarrier = nullptr)
	{
		State = BindCarrier(MoveTemp(State), OutCarrier);
		State = FMatchPlayPenaltyResolution::SubmitMethod(
			State, MethodRequest(State, EMatchPlayPenaltyMethod::Panenka))
			.AfterState;
		FQueueRollProvider Provider;
		Provider.Results = { RollSuccess(RawD6) };
		return FMatchPlayPenaltyResolution::ResolvePanenkaRoll(
			State, RollRequest(State, false), &Provider);
	}

	void MoveToUsed(FMatchPlayState& State,
		const EInitialTurnOrderPlayer Side, const FName CardId)
	{
		FCardUsageState& CardUsage = Usage(State, Side);
		CardUsage.AvailableCardIds.Remove(CardId);
		CardUsage.UsedCardIds.Add(CardId);
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayPenaltyMethodSequentialRetryTest,
	"FMCodex.CoreRules.MatchPlayPenalty.MethodSequentialRetryAndIdempotency",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayPenaltyMethodSequentialRetryTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayPenaltyResolutionTests;
	FMatchPlayState State = BindCarrier(MakeAwaitingCarrier(TEXT("PenaltyFlow")));
	const EInitialTurnOrderPlayer Attacker =
		State.RuntimeState.CurrentAttackingPlayer;
	const EInitialTurnOrderPlayer Defender = Other(Attacker);
	const FMatchPlayState BeforeMethod = State;
	FMatchPlayPenaltyMethodRequest WrongSide =
		MethodRequest(State, EMatchPlayPenaltyMethod::Direct);
	WrongSide.RequestingSide = Defender;
	TestFalse(TEXT("Wrong-side method rejected"),
		FMatchPlayPenaltyResolution::SubmitMethod(State, WrongSide).bSuccess);
	FMatchPlayPenaltyMethodRequest Stale =
		MethodRequest(State, EMatchPlayPenaltyMethod::Direct);
	++Stale.AttackSequence;
	const auto StaleResult =
		FMatchPlayPenaltyResolution::SubmitMethod(State, Stale);
	TestTrue(TEXT("Stale method rejected without mutation"),
		!StaleResult.bSuccess && Equal(BeforeMethod, StaleResult.AfterState));
	const auto Method = FMatchPlayPenaltyResolution::SubmitMethod(
		State, MethodRequest(State, EMatchPlayPenaltyMethod::Direct));
	TestTrue(TEXT("Direct method enters attack-owned roll stage"),
		Method.bSuccess
			&& Method.AfterState.CurrentAttack.SetPieceRoute.Penalty.Stage
				== EMatchPlaySetPieceCarrierRouteStage
					::DirectAwaitingAttackRoll);
	const auto DuplicateMethod = FMatchPlayPenaltyResolution::SubmitMethod(
		Method.AfterState,
		MethodRequest(Method.AfterState, EMatchPlayPenaltyMethod::Direct));
	TestTrue(TEXT("Duplicate method is rejected without mutation"),
		!DuplicateMethod.bSuccess
			&& Equal(Method.AfterState, DuplicateMethod.AfterState));
	FMatchPlayState PanenkaMethodState = BindCarrier(
		MakeAwaitingCarrier(TEXT("PenaltyPanenkaMethod")));
	const auto PanenkaMethod = FMatchPlayPenaltyResolution::SubmitMethod(
		PanenkaMethodState,
		MethodRequest(PanenkaMethodState, EMatchPlayPenaltyMethod::Panenka));
	TestTrue(TEXT("Panenka method enters its single-roll stage with zero RNG"),
		PanenkaMethod.bSuccess
			&& PanenkaMethod.AfterState.CurrentAttack.SetPieceRoute.Penalty.Stage
				== EMatchPlaySetPieceCarrierRouteStage::PanenkaAwaitingRoll);
	FMatchPlayState WrongStage = MakeAwaitingCarrier(TEXT("PenaltyWrongStage"));
	TestFalse(TEXT("Method before Carrier is rejected"),
		FMatchPlayPenaltyResolution::SubmitMethod(
			WrongStage,
			MethodRequest(WrongStage, EMatchPlayPenaltyMethod::Direct)).bSuccess);
	FMatchPlayState CorruptCarrier = BeforeMethod;
	CorruptCarrier.CurrentAttack.SetPieceRoute.Penalty.Carrier.Snapshot
		.Attributes.Shooting = 1;
	TestFalse(TEXT("Corrupt frozen Carrier binding is rejected"),
		FMatchPlayPenaltyResolution::SubmitMethod(
			CorruptCarrier,
			MethodRequest(CorruptCarrier, EMatchPlayPenaltyMethod::Direct))
			.bSuccess);
	FMatchPlayState WrongType = MakeAwaitingCarrier(TEXT("PenaltyWrongType"));
	WrongType.CurrentAttack.SetPieceRoute.RawTypeD6 = 5;
	WrongType.CurrentAttack.SetPieceRoute.SelectedType =
		ESetPieceSelectedType::ShortFreeKick;
	WrongType.CurrentAttack.SetPieceRoute.Penalty = FMatchPlayPenaltyRouteState();
	WrongType.CurrentAttack.SetPieceRoute.ShortFreeKick.Stage =
		EMatchPlaySetPieceCarrierRouteStage::AwaitingCarrier;
	WrongType = BindCarrier(MoveTemp(WrongType));
	TestEqual(TEXT("Wrong concrete Set Piece type is rejected"),
		FMatchPlayPenaltyResolution::SubmitMethod(
			WrongType,
			MethodRequest(WrongType, EMatchPlayPenaltyMethod::Direct)).ErrorCode,
		EMatchPlayPenaltyResolutionErrorCode::WrongSetPieceType);
	State = Method.AfterState;

	FQueueRollProvider PrematureProvider;
	PrematureProvider.Results = { RollSuccess(6) };
	TestFalse(TEXT("Defense before attack is rejected"),
		FMatchPlayPenaltyResolution::ResolveDirectDefenseRoll(
			State, RollRequest(State, true), &PrematureProvider).bSuccess);
	TestEqual(TEXT("Defense-before-attack consumes zero RNG"),
		PrematureProvider.Purposes.Num(), 0);
	FMatchPlayPenaltyRollRequest WrongAttackSide = RollRequest(State, false);
	WrongAttackSide.RequestingSide = Defender;
	TestFalse(TEXT("Wrong-side attack roll rejected"),
		FMatchPlayPenaltyResolution::ResolveDirectAttackRoll(
			State, WrongAttackSide, &PrematureProvider).bSuccess);
	TestEqual(TEXT("Wrong-side attack consumes zero RNG"),
		PrematureProvider.Purposes.Num(), 0);

	FQueueRollProvider RetryProvider;
	RetryProvider.Results = { RollFailure(), RollSuccess(4) };
	const FMatchPlayState BeforeFailure = State;
	const auto Failed = FMatchPlayPenaltyResolution::ResolveDirectAttackRoll(
		State, RollRequest(State, false), &RetryProvider);
	TestTrue(TEXT("Provider failure is retry-safe"),
		!Failed.bSuccess && Equal(BeforeFailure, Failed.AfterState));
	const auto Attack = FMatchPlayPenaltyResolution::ResolveDirectAttackRoll(
		State, RollRequest(State, false), &RetryProvider);
	TestTrue(TEXT("Attack retry persists only attack D6"),
		Attack.bSuccess
			&& Attack.AfterState.CurrentAttack.SetPieceRoute.Penalty.AttackD6 == 4
			&& !Attack.AfterState.CurrentAttack.SetPieceRoute.Penalty.bHasDefenseD6);
	State = Attack.AfterState;
	const int32 CallsBeforeDuplicate = RetryProvider.Purposes.Num();
	TestFalse(TEXT("Duplicate attack roll rejected"),
		FMatchPlayPenaltyResolution::ResolveDirectAttackRoll(
			State, RollRequest(State, false), &RetryProvider).bSuccess);
	TestEqual(TEXT("Duplicate attack consumes zero RNG"),
		RetryProvider.Purposes.Num(), CallsBeforeDuplicate);
	TestTrue(TEXT("Semantic attack purpose recorded"),
		RetryProvider.Purposes.Last()
			== EMatchPlayCurrentAttackPostRouteRollPurpose::PenaltyDirectAttack);

	FQueueRollProvider DefenseProvider;
	DefenseProvider.Results = { RollFailure(), RollSuccess(1) };
	FMatchPlayPenaltyRollRequest WrongDefenseSide = RollRequest(State, true);
	WrongDefenseSide.RequestingSide = Attacker;
	TestFalse(TEXT("Wrong-side defense roll rejected"),
		FMatchPlayPenaltyResolution::ResolveDirectDefenseRoll(
			State, WrongDefenseSide, &DefenseProvider).bSuccess);
	FMatchPlayPenaltyRollRequest StaleDefense = RollRequest(State, true);
	++StaleDefense.AttackSequence;
	TestFalse(TEXT("Stale defense roll rejected"),
		FMatchPlayPenaltyResolution::ResolveDirectDefenseRoll(
			State, StaleDefense, &DefenseProvider).bSuccess);
	TestEqual(TEXT("Rejected defense requests consume zero RNG"),
		DefenseProvider.Purposes.Num(), 0);
	const FMatchPlayState BeforeDefenseFailure = State;
	const auto DefenseFailure =
		FMatchPlayPenaltyResolution::ResolveDirectDefenseRoll(
			State, RollRequest(State, true), &DefenseProvider);
	TestTrue(TEXT("Defense provider failure preserves stored AttackD6"),
		!DefenseFailure.bSuccess
			&& Equal(BeforeDefenseFailure, DefenseFailure.AfterState)
			&& DefenseFailure.AfterState.CurrentAttack.SetPieceRoute.Penalty
				.AttackD6 == 4);
	const int32 AttackCallsBeforeDefenseRetry =
		RetryProvider.Purposes.Num();
	const auto Defense = FMatchPlayPenaltyResolution::ResolveDirectDefenseRoll(
		State, RollRequest(State, true), &DefenseProvider);
	TestTrue(TEXT("Defense-only retry resolves Formula and terminal"),
		Defense.bSuccess
			&& Defense.AfterState.CurrentAttack.SetPieceRoute.Penalty.Stage
				== EMatchPlaySetPieceCarrierRouteStage::Terminal
			&& Defense.AfterState.CurrentAttack.SetPieceRoute.Penalty
				.bHasFormulaResolution);
	TestTrue(TEXT("Defense retry only calls the Defense semantic provider"),
		DefenseProvider.Purposes.Num() == 2
			&& DefenseProvider.Purposes[0]
				== EMatchPlayCurrentAttackPostRouteRollPurpose
					::PenaltyDirectDefense
			&& DefenseProvider.Purposes[1]
				== EMatchPlayCurrentAttackPostRouteRollPurpose
					::PenaltyDirectDefense
			&& RetryProvider.Purposes.Num() == AttackCallsBeforeDefenseRetry);
	const int32 BeforeTerminalDuplicate = DefenseProvider.Purposes.Num();
	TestFalse(TEXT("Terminal defense duplicate rejected"),
		FMatchPlayPenaltyResolution::ResolveDirectDefenseRoll(
			Defense.AfterState, RollRequest(Defense.AfterState, true),
			&DefenseProvider).bSuccess);
	TestEqual(TEXT("Terminal duplicate consumes zero RNG"),
		DefenseProvider.Purposes.Num(), BeforeTerminalDuplicate);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayPenaltyDirectFormulaTest,
	"FMCodex.CoreRules.MatchPlayPenalty.DirectFormulaModifierScoreAndScorer",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayPenaltyDirectFormulaTest::RunTest(const FString& Parameters)
{
	using namespace MatchPlayPenaltyResolutionTests;
	const auto ModifierChangesWinner = MakeDirectTerminal(
		MakeAwaitingCarrier(TEXT("PenaltyMinusThree")), 2, 1, 6, 3, 1);
	const FMatchPlayPenaltyRouteState& Goal =
		ModifierChangesWinner.AfterState.CurrentAttack.SetPieceRoute.Penalty;
	TestTrue(TEXT("Exact -3 modifier materially permits attacker win"),
		ModifierChangesWinner.bSuccess
			&& Goal.GameplayOutcome == EMatchPlayPenaltyGameplayOutcome::Goal
			&& Goal.FormulaResolution.AttackerFinalValue == 5.0f
			&& Goal.FormulaResolution.DefenderFinalValue == 4.0f
			&& Goal.FormulaResolution.MatchLogEntry.FormulaInputs.Contains(
				TEXT("DefenderModifier=-3.0")));
	TestTrue(TEXT("Direct Goal stores scorer and increments score once"),
		Goal.bHasGoalScorer
			&& Goal.GoalScorerCardId == Goal.Carrier.CardId
			&& ModifierChangesWinner.AfterState.RuntimeState.PlayerAState.Score
				+ ModifierChangesWinner.AfterState.RuntimeState.PlayerBState.Score == 1);

	const auto PassingMaximum = MakeDirectTerminal(
		MakeAwaitingCarrier(TEXT("PenaltyPassingMax")), 1, 5, 4, 2, 4);
	TestEqual(TEXT("Direct uses max Shooting/Passing"),
		PassingMaximum.AfterState.CurrentAttack.SetPieceRoute.Penalty
			.FormulaResolution.AttackerFinalValue,
		7.0f);
	const auto EqualAttributes = MakeDirectTerminal(
		MakeAwaitingCarrier(TEXT("PenaltyEqualAttributes")), 4, 4, 3, 3, 3);
	TestEqual(TEXT("Equal Shooting/Passing is deterministic"),
		EqualAttributes.AfterState.CurrentAttack.SetPieceRoute.Penalty
			.FormulaResolution.AttackerFinalValue,
		7.0f);
	const auto GkTie = MakeDirectTerminal(
		MakeAwaitingCarrier(TEXT("PenaltyGkTie")), 3, 2, 6, 2, 2);
	const FMatchPlayPenaltyRouteState& NoGoal =
		GkTie.AfterState.CurrentAttack.SetPieceRoute.Penalty;
	TestTrue(TEXT("GK-involved exact tie belongs to defender"),
		GkTie.bSuccess
			&& NoGoal.FormulaResolution.AttackerFinalValue == 5.0f
			&& NoGoal.FormulaResolution.DefenderFinalValue == 5.0f
			&& NoGoal.FormulaResolution.Winner == EFormulaWinner::Defender
			&& NoGoal.FormulaResolution.WinReason
				== EFormulaWinReason::DefenderWinsGoalkeeperTie
			&& NoGoal.GameplayOutcome
				== EMatchPlayPenaltyGameplayOutcome::NoGoal
			&& !NoGoal.bHasGoalScorer
			&& NoGoal.GoalScorerCardId.IsNone());
	TestEqual(TEXT("NoGoal leaves aggregate score unchanged"),
		GkTie.AfterState.RuntimeState.PlayerAState.Score
			+ GkTie.AfterState.RuntimeState.PlayerBState.Score,
		0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayPenaltyPanenkaTest,
	"FMCodex.CoreRules.MatchPlayPenalty.PanenkaOneTwoSixAndNoFormula",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayPenaltyPanenkaTest::RunTest(const FString& Parameters)
{
	using namespace MatchPlayPenaltyResolutionTests;
	FMatchPlayState RetryState = BindCarrier(
		MakeAwaitingCarrier(TEXT("PanenkaRetry")));
	RetryState = FMatchPlayPenaltyResolution::SubmitMethod(
		RetryState,
		MethodRequest(RetryState, EMatchPlayPenaltyMethod::Panenka)).AfterState;
	FQueueRollProvider RetryProvider;
	RetryProvider.Results = { RollFailure(), RollSuccess(2) };
	FMatchPlayPenaltyRollRequest WrongSide = RollRequest(RetryState, false);
	WrongSide.RequestingSide = Other(WrongSide.RequestingSide);
	TestFalse(TEXT("Wrong-side Panenka rejected"),
		FMatchPlayPenaltyResolution::ResolvePanenkaRoll(
			RetryState, WrongSide, &RetryProvider).bSuccess);
	FMatchPlayPenaltyRollRequest Stale = RollRequest(RetryState, false);
	++Stale.AttackSequence;
	TestFalse(TEXT("Stale Panenka rejected"),
		FMatchPlayPenaltyResolution::ResolvePanenkaRoll(
			RetryState, Stale, &RetryProvider).bSuccess);
	TestEqual(TEXT("Rejected Panenka requests consume zero RNG"),
		RetryProvider.Purposes.Num(), 0);
	const FMatchPlayState BeforeFailure = RetryState;
	const auto Failure = FMatchPlayPenaltyResolution::ResolvePanenkaRoll(
		RetryState, RollRequest(RetryState, false), &RetryProvider);
	TestTrue(TEXT("Panenka provider failure preserves exact pending state"),
		!Failure.bSuccess && Equal(BeforeFailure, Failure.AfterState));
	const auto Retry = FMatchPlayPenaltyResolution::ResolvePanenkaRoll(
		RetryState, RollRequest(RetryState, false), &RetryProvider);
	TestTrue(TEXT("Panenka retry alone resolves boundary Goal"),
		Retry.bSuccess
			&& Retry.AfterState.CurrentAttack.SetPieceRoute.Penalty.PanenkaD6 == 2
			&& RetryProvider.Purposes.Num() == 2
			&& RetryProvider.Purposes[0]
				== EMatchPlayCurrentAttackPostRouteRollPurpose::PenaltyPanenka
			&& RetryProvider.Purposes[1]
				== EMatchPlayCurrentAttackPostRouteRollPurpose::PenaltyPanenka);
	const int32 ScoreAfterRetry =
		Retry.AfterState.RuntimeState.PlayerAState.Score
			+ Retry.AfterState.RuntimeState.PlayerBState.Score;
	const int32 BeforeDuplicate = RetryProvider.Purposes.Num();
	TestFalse(TEXT("Terminal Panenka duplicate rejected"),
		FMatchPlayPenaltyResolution::ResolvePanenkaRoll(
			Retry.AfterState, RollRequest(Retry.AfterState, false),
			&RetryProvider).bSuccess);
	TestTrue(TEXT("Panenka duplicate consumes no RNG and cannot rescore"),
		RetryProvider.Purposes.Num() == BeforeDuplicate
			&& Retry.AfterState.RuntimeState.PlayerAState.Score
				+ Retry.AfterState.RuntimeState.PlayerBState.Score
				== ScoreAfterRetry);
	const auto Miss = MakePanenkaTerminal(
		MakeAwaitingCarrier(TEXT("PanenkaOne")), 1);
	const auto Two = MakePanenkaTerminal(
		MakeAwaitingCarrier(TEXT("PanenkaTwo")), 2);
	const auto Six = MakePanenkaTerminal(
		MakeAwaitingCarrier(TEXT("PanenkaSix")), 6);
	TestTrue(TEXT("Panenka 1 is NoGoal"),
		Miss.bSuccess
			&& Miss.AfterState.CurrentAttack.SetPieceRoute.Penalty.GameplayOutcome
				== EMatchPlayPenaltyGameplayOutcome::NoGoal);
	TestTrue(TEXT("Panenka 2 and 6 are Goal"),
		Two.bSuccess && Six.bSuccess
			&& Two.AfterState.CurrentAttack.SetPieceRoute.Penalty.GameplayOutcome
				== EMatchPlayPenaltyGameplayOutcome::Goal
			&& Six.AfterState.CurrentAttack.SetPieceRoute.Penalty.GameplayOutcome
				== EMatchPlayPenaltyGameplayOutcome::Goal);
	for (const FMatchPlayPenaltyResolutionResult* Result : { &Miss, &Two, &Six })
	{
		const FMatchPlayPenaltyRouteState& Penalty =
			Result->AfterState.CurrentAttack.SetPieceRoute.Penalty;
		TestTrue(TEXT("Panenka stores one raw roll and no comparison facts"),
			Penalty.bHasPanenkaD6 && Penalty.PanenkaD6 >= 1
				&& Penalty.PanenkaD6 <= 6
				&& !Penalty.bHasAttackD6 && !Penalty.bHasDefenseD6
				&& !Penalty.bHasFormulaResolution
				&& !Result->GoalkeeperQueryResult.bSuccess);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayPenaltyNoLegalCarrierTest,
	"FMCodex.CoreRules.MatchPlayPenalty.NoLegalCarrierAvailabilityVariants",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayPenaltyNoLegalCarrierTest::RunTest(const FString& Parameters)
{
	using namespace MatchPlayPenaltyResolutionTests;
	auto RequestFor = [](const FMatchPlayState& State)
	{
		FMatchPlayPenaltyNoLegalCarrierRequest Request;
		Request.RequestingSide = State.RuntimeState.CurrentAttackingPlayer;
		Request.AttackSequence = State.CurrentAttack.AttackSequence;
		return Request;
	};
	FMatchPlayState Available = MakeAwaitingCarrier(TEXT("PenaltyHasCarrier"));
	const auto Rejected = FMatchPlayPenaltyResolution::ResolveNoLegalCarrier(
		Available, RequestFor(Available));
	TestTrue(TEXT("Available non-GK prevents no-legal resolution"),
		!Rejected.bSuccess
			&& Rejected.ErrorCode
				== EMatchPlayPenaltyResolutionErrorCode::LegalCarrierExists
			&& Equal(Available, Rejected.AfterState));

	for (const bool bEjected : { false, true })
	{
		FMatchPlayState State = MakeAwaitingCarrier(
			bEjected ? TEXT("PenaltyOnlyEjected") : TEXT("PenaltyOnlyUsed"));
		const EInitialTurnOrderPlayer Attacker =
			State.RuntimeState.CurrentAttackingPlayer;
		FCardUsageState& CardUsage = Usage(State, Attacker);
		for (const FPlayerCardRuleSnapshot& Snapshot : Snapshots(State, Attacker).Cards)
		{
			if (!Snapshot.bIsGoalkeeper)
			{
				CardUsage.AvailableCardIds.Remove(Snapshot.CardId);
				(bEjected ? CardUsage.EjectedCardIds : CardUsage.UsedCardIds)
					.Add(Snapshot.CardId);
			}
		}
		const auto Resolved = FMatchPlayPenaltyResolution::ResolveNoLegalCarrier(
			State, RequestFor(State));
		TestTrue(TEXT("Used/Ejected-only pool resolves zero-RNG NoGoal"),
			Resolved.bSuccess
				&& Resolved.AfterState.CurrentAttack.SetPieceRoute.Penalty
					.bNoLegalCarrier
				&& Resolved.AfterState.CurrentAttack.SetPieceRoute.Penalty
					.GameplayOutcome == EMatchPlayPenaltyGameplayOutcome::NoGoal
				&& !Resolved.AfterState.CurrentAttack.SetPieceRoute.Penalty
					.bHasGoalScorer);
	}

	FMatchPlayState Empty = MakeAwaitingCarrier(TEXT("PenaltyEmpty"));
	Usage(Empty, Empty.RuntimeState.CurrentAttackingPlayer)
		.AvailableCardIds.Reset();
	TestTrue(TEXT("Truly empty Available pool resolves no legal Carrier"),
		FMatchPlayPenaltyResolution::ResolveNoLegalCarrier(
			Empty, RequestFor(Empty)).bSuccess);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayPenaltyAdvanceRecoveryReconstructionTest,
	"FMCodex.CoreRules.MatchPlayPenalty.AdvanceRecoveryAndReconstruction",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayPenaltyAdvanceRecoveryReconstructionTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayPenaltyResolutionTests;
	FMatchPlayState Base = MakeAwaitingCarrier(TEXT("PenaltyAdvance"));
	const EInitialTurnOrderPlayer Attacker =
		Base.RuntimeState.CurrentAttackingPlayer;
	FName Carrier = NAME_None;
	TestTrue(TEXT("AwaitingCarrier reconstructs"),
		FMatchPlayCurrentAttackRouteStateValidator::Validate(Base).bIsCanonical);
	Base = BindCarrier(MoveTemp(Base), &Carrier);
	for (int32 Index = 1; Index <= 3; ++Index)
	{
		MoveToUsed(Base, Attacker, FindCard(Base, Attacker, false, Index));
	}
	TestTrue(TEXT("AwaitingMethod reconstructs"),
		FMatchPlayCurrentAttackRouteStateValidator::Validate(Base).bIsCanonical);
	Base = FMatchPlayPenaltyResolution::SubmitMethod(
		Base, MethodRequest(Base, EMatchPlayPenaltyMethod::Panenka)).AfterState;
	TestTrue(TEXT("Panenka pending reconstructs"),
		FMatchPlayCurrentAttackRouteStateValidator::Validate(Base).bIsCanonical);
	FQueueRollProvider Rolls;
	Rolls.Results = { RollSuccess(6) };
	const auto TerminalResult = FMatchPlayPenaltyResolution::ResolvePanenkaRoll(
		Base, RollRequest(Base, false), &Rolls);
	const FMatchPlayState Terminal = TerminalResult.AfterState;
	TestTrue(TEXT("Terminal reconstructs and Carrier is still Available"),
		FMatchPlayCurrentAttackRouteStateValidator::Validate(Terminal).bIsCanonical
			&& Usage(Terminal, Attacker).AvailableCardIds.Contains(Carrier));

	FCapturingRecoveryProvider Failure;
	Failure.bFail = true;
	const auto Failed = FMatchPlayCurrentAttackCompletion::AdvanceAfterTerminal(
		Terminal, Terminal.CurrentAttack.AttackSequence, Attacker, &Failure);
	TestTrue(TEXT("Recovery failure rolls back participant consumption"),
		!Failed.bSuccess && Equal(Terminal, Failed.AfterState)
			&& Usage(Failed.AfterState, Attacker)
				.AvailableCardIds.Contains(Carrier));
	FCapturingRecoveryProvider KeepUsed;
	KeepUsed.PreferredCard = Carrier;
	const auto Advanced = FMatchPlayCurrentAttackCompletion::AdvanceAfterTerminal(
		Terminal, Terminal.CurrentAttack.AttackSequence, Attacker, &KeepUsed);
	TestTrue(TEXT("Advance exposes newly Used Carrier to shared Recovery"),
		Advanced.bSuccess
			&& KeepUsed.Candidates.ContainsByPredicate(
				[Carrier](const FMatchPlayRecoveryCandidate& Candidate)
				{ return Candidate.CardId == Carrier; })
			&& Usage(Advanced.AfterState, Attacker).UsedCardIds.Contains(Carrier)
			&& (Attacker == EInitialTurnOrderPlayer::PlayerA
				? Advanced.AfterState.RuntimeState.PlayerAState.UsedAttackCount
				: Advanced.AfterState.RuntimeState.PlayerBState.UsedAttackCount) == 1);
	FCapturingRecoveryProvider ReturnCarrier;
	ReturnCarrier.PreferredCard = Carrier;
	ReturnCarrier.bSelectPreferred = true;
	const auto Recovered = FMatchPlayCurrentAttackCompletion::AdvanceAfterTerminal(
		Terminal, Terminal.CurrentAttack.AttackSequence, Attacker, &ReturnCarrier);
	TestTrue(TEXT("Same atomic Advance may recover the Penalty Carrier"),
		Recovered.bSuccess
			&& Usage(Recovered.AfterState, Attacker)
				.AvailableCardIds.Contains(Carrier));

	FMatchPlayState Final = Terminal;
	FPlayerRuntimeState& AttackerRuntime =
		Attacker == EInitialTurnOrderPlayer::PlayerA
			? Final.RuntimeState.PlayerAState : Final.RuntimeState.PlayerBState;
	FPlayerRuntimeState& DefenderRuntime =
		Attacker == EInitialTurnOrderPlayer::PlayerA
			? Final.RuntimeState.PlayerBState : Final.RuntimeState.PlayerAState;
	AttackerRuntime.TotalAttackCount = 1;
	AttackerRuntime.UsedAttackCount = 0;
	DefenderRuntime.TotalAttackCount = 1;
	DefenderRuntime.UsedAttackCount = 1;
	Final.CurrentAttack.AttackSequence = 2;
	FCapturingRecoveryProvider FinalRecovery;
	const auto FinalAdvance = FMatchPlayCurrentAttackCompletion::AdvanceAfterTerminal(
		Final, 2, Attacker, &FinalRecovery);
	TestTrue(TEXT("Final Advance consumes Carrier and skips Recovery"),
		FinalAdvance.bSuccess && FinalAdvance.bMatchEnded
			&& Usage(FinalAdvance.AfterState, Attacker).UsedCardIds.Contains(Carrier)
			&& FinalRecovery.CallCount == 0);

	const auto Direct = MakeDirectTerminal(
		MakeAwaitingCarrier(TEXT("PenaltyRebuildDirect")), 5, 4, 4, 3, 3);
	FMatchPlayState DirectAttack = BindCarrier(
		MakeAwaitingCarrier(TEXT("PenaltyRebuildPrefix")));
	DirectAttack = FMatchPlayPenaltyResolution::SubmitMethod(
		DirectAttack,
		MethodRequest(DirectAttack, EMatchPlayPenaltyMethod::Direct)).AfterState;
	TestTrue(TEXT("Direct AwaitingAttack reconstructs"),
		FMatchPlayCurrentAttackRouteStateValidator::Validate(DirectAttack)
			.bIsCanonical);
	FQueueRollProvider PrefixRoll;
	PrefixRoll.Results = { RollSuccess(4) };
	const FMatchPlayState DirectDefense =
		FMatchPlayPenaltyResolution::ResolveDirectAttackRoll(
			DirectAttack, RollRequest(DirectAttack, false), &PrefixRoll).AfterState;
	TestTrue(TEXT("Direct AwaitingDefense prefix reconstructs"),
		FMatchPlayCurrentAttackRouteStateValidator::Validate(DirectDefense)
			.bIsCanonical);
	const auto DirectNoGoal = MakeDirectTerminal(
		MakeAwaitingCarrier(TEXT("PenaltyRebuildNoGoal")), 3, 2, 6, 2, 2);
	TestTrue(TEXT("Direct Goal and NoGoal terminals reconstruct"),
		FMatchPlayCurrentAttackRouteStateValidator::Validate(Direct.AfterState)
			.bIsCanonical
			&& FMatchPlayCurrentAttackRouteStateValidator::Validate(
				DirectNoGoal.AfterState).bIsCanonical);
	const auto PanenkaNoGoal = MakePanenkaTerminal(
		MakeAwaitingCarrier(TEXT("PenaltyRebuildPanenkaMiss")), 1);
	TestTrue(TEXT("Panenka Goal and NoGoal terminals reconstruct"),
		FMatchPlayCurrentAttackRouteStateValidator::Validate(Terminal)
			.bIsCanonical
			&& FMatchPlayCurrentAttackRouteStateValidator::Validate(
				PanenkaNoGoal.AfterState).bIsCanonical);
	FMatchPlayState NoCarrier = MakeAwaitingCarrier(
		TEXT("PenaltyRebuildNoCarrier"));
	const EInitialTurnOrderPlayer NoCarrierAttacker =
		NoCarrier.RuntimeState.CurrentAttackingPlayer;
	for (const FPlayerCardRuleSnapshot& Snapshot :
		Snapshots(NoCarrier, NoCarrierAttacker).Cards)
	{
		if (!Snapshot.bIsGoalkeeper)
		{
			MoveToUsed(NoCarrier, NoCarrierAttacker, Snapshot.CardId);
		}
	}
	FMatchPlayPenaltyNoLegalCarrierRequest NoCarrierRequest;
	NoCarrierRequest.RequestingSide = NoCarrierAttacker;
	NoCarrierRequest.AttackSequence = 1;
	const FMatchPlayState NoCarrierTerminal =
		FMatchPlayPenaltyResolution::ResolveNoLegalCarrier(
			NoCarrier, NoCarrierRequest).AfterState;
	TestTrue(TEXT("No-Carrier terminal reconstructs"),
		FMatchPlayCurrentAttackRouteStateValidator::Validate(NoCarrierTerminal)
			.bIsCanonical);
	FCapturingRecoveryProvider NoCarrierRecovery;
	const auto NoCarrierAdvance =
		FMatchPlayCurrentAttackCompletion::AdvanceAfterTerminal(
			NoCarrierTerminal, 1, NoCarrierAttacker, &NoCarrierRecovery);
	TestTrue(TEXT("No-Carrier Advance consumes zero Set Piece participants"),
		NoCarrierAdvance.bSuccess
			&& NoCarrierAdvance.SetPieceCardUsageResults.IsEmpty());
	FMatchPlayState Corrupt = Direct.AfterState;
	Corrupt.CurrentAttack.SetPieceRoute.Penalty
		.FormulaResolution.DefenderFinalValue += 1.0f;
	TestFalse(TEXT("Corrupt reconstructed Direct Formula rejected"),
		FMatchPlayCurrentAttackRouteStateValidator::Validate(Corrupt).bIsCanonical);
	Corrupt = Terminal;
	Corrupt.CurrentAttack.SetPieceRoute.Penalty.GoalScorerCardId = NAME_None;
	TestFalse(TEXT("Corrupt reconstructed scorer rejected"),
		FMatchPlayCurrentAttackRouteStateValidator::Validate(Corrupt).bIsCanonical);
	return true;
}

#endif
