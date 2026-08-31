#include "MatchPlayOpeningInitializer.h"
#include "MatchPlaySetPieceCarrierSelection.h"
#include "MatchPlayLongFreeKickResolution.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

namespace MatchPlayLongFreeKickResolutionTests
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
		Card.Attributes.LongShot = 6;
		Card.Attributes.Stamina = 5;
		Card.GoalkeeperAttributes.Positioning = 4;
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

	FMatchPlayState MakeLongAwaitingCarrier(const FString& Prefix)
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
		State.CurrentAttack.SetPieceRoute.RawTypeD6 = 3;
		State.CurrentAttack.SetPieceRoute.SelectedType =
			ESetPieceSelectedType::LongFreeKick;
		State.CurrentAttack.SetPieceRoute.LongFreeKick.Stage =
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
		const bool bGoalkeeper)
	{
		for (FPlayerCardRuleSnapshot& Snapshot : Snapshots(State, Side).Cards)
		{
			if (Snapshot.bIsGoalkeeper == bGoalkeeper)
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

	FMatchPlayState BindCarrier(
		FMatchPlayState State,
		FName* OutCardId = nullptr)
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

	FMatchPlayLongFreeKickMethodRequest MethodRequest(
		const FMatchPlayState& State,
		const EMatchPlayLongFreeKickMethod Method)
	{
		FMatchPlayLongFreeKickMethodRequest Request;
		Request.RequestingSide = State.RuntimeState.CurrentAttackingPlayer;
		Request.AttackSequence = State.CurrentAttack.AttackSequence;
		Request.Method = Method;
		return Request;
	}

	FMatchPlayLongFreeKickRollRequest RollRequest(
		const FMatchPlayState& State,
		const bool bDefender)
	{
		FMatchPlayLongFreeKickRollRequest Request;
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
		Result.ErrorMessage = TEXT("Injected Long Free Kick roll failure.");
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

	class FRecoveryProvider final : public IMatchPlayRecoveryProvider
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
				Result.ErrorMessage = TEXT("Injected Long FK Recovery failure.");
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
			if (bSelectPreferred && PreferredIndex != INDEX_NONE)
			{
				Result.SelectedCandidateIndices.Add(PreferredIndex);
			}
			for (int32 Index = 0;
				Index < OrderedCandidates.Num()
					&& Result.SelectedCandidateIndices.Num() < ReturnCount;
				++Index)
			{
				if (Index != PreferredIndex)
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
	FMatchPlayLongFreeKickDirectConditionalTest,
	"FMCodex.CoreRules.MatchPlayLongFreeKick.DirectConditionalAndFormula",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayLongFreeKickDirectConditionalTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayLongFreeKickResolutionTests;
	FName MissCarrier;
	FMatchPlayState Miss = BindCarrier(
		MakeLongAwaitingCarrier(TEXT("LFK_Immediate")), &MissCarrier);
	const auto Method = FMatchPlayLongFreeKickResolution::SubmitMethod(
		Miss, MethodRequest(Miss, EMatchPlayLongFreeKickMethod::Direct));
	TestTrue(TEXT("Direct method succeeds without RNG"), Method.bSuccess);
	TestFalse(TEXT("Duplicate method is rejected without mutation"),
		FMatchPlayLongFreeKickResolution::SubmitMethod(
			Method.AfterState,
			MethodRequest(Method.AfterState,
				EMatchPlayLongFreeKickMethod::Direct)).bSuccess);
	Miss = Method.AfterState;
	FQueueRollProvider MissProvider;
	MissProvider.Results = { RollSuccess(2) };
	const auto Immediate =
		FMatchPlayLongFreeKickResolution::ResolveDirectAttackRoll(
			Miss, RollRequest(Miss, false), &MissProvider);
	TestTrue(TEXT("Attack D6 1-2 resolves terminal"), Immediate.bSuccess);
	const auto& ImmediateLong =
		Immediate.AfterState.CurrentAttack.SetPieceRoute.LongFreeKick;
	TestTrue(TEXT("Immediate miss stores no defense or Formula"),
		ImmediateLong.Stage == EMatchPlaySetPieceCarrierRouteStage::Terminal
		&& ImmediateLong.AttackD6 == 2
		&& !ImmediateLong.bHasDefenseD6
		&& !ImmediateLong.bHasFormulaResolution
		&& ImmediateLong.GameplayOutcome
			== EMatchPlayLongFreeKickGameplayOutcome::NoGoal);
	TestTrue(TEXT("Immediate terminal is formally canonical"),
		Immediate.AfterRouteValidation.bIsCanonical);
	TestTrue(TEXT("Carrier is still Available at terminal"),
		Usage(Immediate.AfterState,
			Immediate.AfterState.RuntimeState.CurrentAttackingPlayer)
			.AvailableCardIds.Contains(MissCarrier));

	for (const int32 BoundaryRoll : { 1, 3 })
	{
		FMatchPlayState Boundary = BindCarrier(MakeLongAwaitingCarrier(
			FString::Printf(TEXT("LFK_AttackBoundary_%d"), BoundaryRoll)));
		Boundary = FMatchPlayLongFreeKickResolution::SubmitMethod(
			Boundary,
			MethodRequest(Boundary, EMatchPlayLongFreeKickMethod::Direct))
			.AfterState;
		FQueueRollProvider BoundaryProvider;
		BoundaryProvider.Results = { RollSuccess(BoundaryRoll) };
		const auto BoundaryResult =
			FMatchPlayLongFreeKickResolution::ResolveDirectAttackRoll(
				Boundary, RollRequest(Boundary, false), &BoundaryProvider);
		TestTrue(TEXT("Direct Attack boundary resolves"),
			BoundaryResult.bSuccess);
		TestEqual(TEXT("Direct Attack boundary consumes exactly one purpose"),
			BoundaryProvider.Purposes.Num(), 1);
		TestEqual(TEXT("Direct Attack boundary selects exact next stage"),
			BoundaryResult.AfterState.CurrentAttack.SetPieceRoute.LongFreeKick
				.Stage,
			BoundaryRoll <= 2
				? EMatchPlaySetPieceCarrierRouteStage::Terminal
				: EMatchPlaySetPieceCarrierRouteStage
					::DirectAwaitingDefenseRoll);
	}

	FName GoalCarrier;
	FMatchPlayState GoalState = BindCarrier(
		MakeLongAwaitingCarrier(TEXT("LFK_Formula")), &GoalCarrier);
	GoalState = FMatchPlayLongFreeKickResolution::SubmitMethod(
		GoalState,
		MethodRequest(GoalState, EMatchPlayLongFreeKickMethod::Direct))
		.AfterState;
	FQueueRollProvider GoalProvider;
	GoalProvider.Results = { RollSuccess(6), RollSuccess(1) };
	const auto Prefix =
		FMatchPlayLongFreeKickResolution::ResolveDirectAttackRoll(
			GoalState, RollRequest(GoalState, false), &GoalProvider);
	TestTrue(TEXT("Attack D6 3-6 stores prefix"), Prefix.bSuccess);
	TestTrue(TEXT("Prefix is reconstructable and incomplete"),
		Prefix.AfterState.CurrentAttack.SetPieceRoute.LongFreeKick.Stage
			== EMatchPlaySetPieceCarrierRouteStage::DirectAwaitingDefenseRoll
		&& Prefix.AfterRouteValidation.bIsCanonical);
	FMatchPlayState CorruptPrefix = Prefix.AfterState;
	CorruptPrefix.CurrentAttack.SetPieceRoute.LongFreeKick.AttackD6 = 2;
	TestFalse(TEXT("Reconstruction rejects impossible 1-2 defense prefix"),
		FMatchPlayCurrentAttackRouteStateValidator::Validate(CorruptPrefix)
			.bIsCanonical);
	FQueueRollProvider DefenseFailure;
	DefenseFailure.Results = { RollFailure() };
	const auto FailedDefense =
		FMatchPlayLongFreeKickResolution::ResolveDirectDefenseRoll(
			Prefix.AfterState, RollRequest(Prefix.AfterState, true),
			&DefenseFailure);
	TestFalse(TEXT("Defense provider failure remains retryable"),
		FailedDefense.bSuccess);
	TestTrue(TEXT("Defense failure preserves stored AttackD6 exactly"),
		Equal(Prefix.AfterState, FailedDefense.AfterState)
		&& FailedDefense.AfterState.CurrentAttack.SetPieceRoute.LongFreeKick
			.AttackD6 == 6);
	TestTrue(TEXT("Defense failure consumes only Defense purpose"),
		DefenseFailure.Purposes.Num() == 1
		&& DefenseFailure.Purposes[0]
			== EMatchPlayCurrentAttackPostRouteRollPurpose
				::LongFreeKickDirectDefense);
	const auto Goal =
		FMatchPlayLongFreeKickResolution::ResolveDirectDefenseRoll(
			Prefix.AfterState, RollRequest(Prefix.AfterState, true),
			&GoalProvider);
	TestTrue(TEXT("Defense roll resolves terminal Formula"), Goal.bSuccess);
	const auto& GoalLong = Goal.AfterState.CurrentAttack.SetPieceRoute.LongFreeKick;
	TestTrue(TEXT("Formula uses LongShot and GK Positioning plus two"),
		GoalLong.FormulaResolution.AttackerFinalValue == 12.0f
		&& GoalLong.FormulaResolution.DefenderFinalValue == 7.0f);
	TestTrue(TEXT("Goal increments score and binds scorer"),
		GoalLong.GameplayOutcome == EMatchPlayLongFreeKickGameplayOutcome::Goal
		&& GoalLong.GoalScorerCardId == GoalCarrier
		&& Goal.AfterState.RuntimeState.PlayerAState.Score
			+ Goal.AfterState.RuntimeState.PlayerBState.Score == 1);

	FMatchPlayState TieState = MakeLongAwaitingCarrier(TEXT("LFK_Tie"));
	const EInitialTurnOrderPlayer TieAttacker =
		TieState.RuntimeState.CurrentAttackingPlayer;
	FindSnapshot(TieState, TieAttacker, false)->Attributes.LongShot = 3;
	FindSnapshot(TieState, Other(TieAttacker), true)
		->GoalkeeperAttributes.Positioning = 3;
	TieState = BindCarrier(MoveTemp(TieState));
	TieState = FMatchPlayLongFreeKickResolution::SubmitMethod(
		TieState,
		MethodRequest(TieState, EMatchPlayLongFreeKickMethod::Direct))
		.AfterState;
	FQueueRollProvider TieProvider;
	TieProvider.Results = { RollSuccess(3), RollSuccess(1) };
	TieState = FMatchPlayLongFreeKickResolution::ResolveDirectAttackRoll(
		TieState, RollRequest(TieState, false), &TieProvider).AfterState;
	const auto Tie = FMatchPlayLongFreeKickResolution::ResolveDirectDefenseRoll(
		TieState, RollRequest(TieState, true), &TieProvider);
	TestTrue(TEXT("GK owns exact Formula tie"),
		Tie.bSuccess
		&& Tie.AfterState.CurrentAttack.SetPieceRoute.LongFreeKick
			.FormulaResolution.WinReason
				== EFormulaWinReason::DefenderWinsGoalkeeperTie
		&& Tie.AfterState.CurrentAttack.SetPieceRoute.LongFreeKick
			.GameplayOutcome
				== EMatchPlayLongFreeKickGameplayOutcome::NoGoal);
	TestFalse(TEXT("Formula NoGoal has no scorer"),
		Tie.AfterState.CurrentAttack.SetPieceRoute.LongFreeKick
			.bHasGoalScorer);

	FMatchPlayState DefenderWinState =
		MakeLongAwaitingCarrier(TEXT("LFK_DefenderWin"));
	const EInitialTurnOrderPlayer DefenderWinAttacker =
		DefenderWinState.RuntimeState.CurrentAttackingPlayer;
	FindSnapshot(DefenderWinState, DefenderWinAttacker, false)
		->Attributes.LongShot = 1;
	FindSnapshot(DefenderWinState, Other(DefenderWinAttacker), true)
		->GoalkeeperAttributes.Positioning = 6;
	DefenderWinState = BindCarrier(MoveTemp(DefenderWinState));
	DefenderWinState = FMatchPlayLongFreeKickResolution::SubmitMethod(
		DefenderWinState,
		MethodRequest(DefenderWinState,
			EMatchPlayLongFreeKickMethod::Direct)).AfterState;
	FQueueRollProvider DefenderWinProvider;
	DefenderWinProvider.Results = { RollSuccess(3), RollSuccess(6) };
	DefenderWinState =
		FMatchPlayLongFreeKickResolution::ResolveDirectAttackRoll(
			DefenderWinState, RollRequest(DefenderWinState, false),
			&DefenderWinProvider).AfterState;
	const auto DefenderWin =
		FMatchPlayLongFreeKickResolution::ResolveDirectDefenseRoll(
			DefenderWinState, RollRequest(DefenderWinState, true),
			&DefenderWinProvider);
	TestTrue(TEXT("Defender clear win is NoGoal"),
		DefenderWin.bSuccess
		&& DefenderWin.AfterState.CurrentAttack.SetPieceRoute.LongFreeKick
			.GameplayOutcome
				== EMatchPlayLongFreeKickGameplayOutcome::NoGoal);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayLongFreeKickProviderAtomicityTest,
	"FMCodex.CoreRules.MatchPlayLongFreeKick.ProviderRetryAndPowerAtomicity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayLongFreeKickProviderAtomicityTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayLongFreeKickResolutionTests;
	FMatchPlayState AwaitingMethod = BindCarrier(
		MakeLongAwaitingCarrier(TEXT("LFK_Retry")));
	FMatchPlayLongFreeKickMethodRequest StaleMethod = MethodRequest(
		AwaitingMethod, EMatchPlayLongFreeKickMethod::Direct);
	++StaleMethod.AttackSequence;
	const auto StaleMethodResult =
		FMatchPlayLongFreeKickResolution::SubmitMethod(
			AwaitingMethod, StaleMethod);
	TestFalse(TEXT("Stale method rejected"), StaleMethodResult.bSuccess);
	TestTrue(TEXT("Stale method preserves state"),
		Equal(AwaitingMethod, StaleMethodResult.AfterState));
	FMatchPlayState CorruptCarrier = AwaitingMethod;
	CorruptCarrier.CurrentAttack.SetPieceRoute.LongFreeKick.Carrier =
		FMatchPlaySetPieceParticipantBinding();
	TestFalse(TEXT("Corrupt Carrier rejected"),
		FMatchPlayLongFreeKickResolution::SubmitMethod(
			CorruptCarrier,
			MethodRequest(CorruptCarrier,
				EMatchPlayLongFreeKickMethod::Direct)).bSuccess);
	FMatchPlayState WrongStage =
		MakeLongAwaitingCarrier(TEXT("LFK_WrongStage"));
	TestFalse(TEXT("Method before Carrier rejected"),
		FMatchPlayLongFreeKickResolution::SubmitMethod(
			WrongStage,
			MethodRequest(WrongStage,
				EMatchPlayLongFreeKickMethod::Direct)).bSuccess);

	FMatchPlayState WrongType =
		MakeLongAwaitingCarrier(TEXT("LFK_WrongType"));
	WrongType.CurrentAttack.SetPieceRoute.RawTypeD6 = 5;
	WrongType.CurrentAttack.SetPieceRoute.SelectedType =
		ESetPieceSelectedType::ShortFreeKick;
	WrongType.CurrentAttack.SetPieceRoute.LongFreeKick =
		FMatchPlayLongFreeKickRouteState();
	WrongType.CurrentAttack.SetPieceRoute.ShortFreeKick.Stage =
		EMatchPlaySetPieceCarrierRouteStage::AwaitingCarrier;
	WrongType = BindCarrier(MoveTemp(WrongType));
	TestEqual(TEXT("Wrong concrete type rejected"),
		FMatchPlayLongFreeKickResolution::SubmitMethod(
			WrongType,
			MethodRequest(WrongType,
				EMatchPlayLongFreeKickMethod::Direct)).ErrorCode,
		EMatchPlayLongFreeKickResolutionErrorCode::WrongSetPieceType);

	FMatchPlayState Direct = AwaitingMethod;
	Direct = FMatchPlayLongFreeKickResolution::SubmitMethod(
		Direct, MethodRequest(Direct, EMatchPlayLongFreeKickMethod::Direct))
		.AfterState;
	FQueueRollProvider WrongSideProvider;
	WrongSideProvider.Results = { RollSuccess(6) };
	FMatchPlayLongFreeKickRollRequest WrongSide = RollRequest(Direct, false);
	WrongSide.RequestingSide = Other(WrongSide.RequestingSide);
	TestFalse(TEXT("Wrong-side attack request rejected"),
		FMatchPlayLongFreeKickResolution::ResolveDirectAttackRoll(
			Direct, WrongSide, &WrongSideProvider).bSuccess);
	TestEqual(TEXT("Rejected request consumes zero RNG"),
		WrongSideProvider.Purposes.Num(), 0);
	FMatchPlayState InvalidGoalkeeper = Direct;
	const EInitialTurnOrderPlayer InvalidGkDefender =
		Other(InvalidGoalkeeper.RuntimeState.CurrentAttackingPlayer);
	FPlayerRuntimeState& InvalidGkRuntime = InvalidGkDefender
		== EInitialTurnOrderPlayer::PlayerA
			? InvalidGoalkeeper.RuntimeState.PlayerAState
			: InvalidGoalkeeper.RuntimeState.PlayerBState;
	InvalidGkRuntime.GoalkeeperCardId = FindCard(
		InvalidGoalkeeper, InvalidGkDefender, false).ToString();
	FQueueRollProvider InvalidGkProvider;
	InvalidGkProvider.Results = { RollSuccess(2) };
	TestFalse(TEXT("Invalid defending GK rejects before Attack RNG"),
		FMatchPlayLongFreeKickResolution::ResolveDirectAttackRoll(
			InvalidGoalkeeper, RollRequest(InvalidGoalkeeper, false),
			&InvalidGkProvider).bSuccess);
	TestEqual(TEXT("Invalid GK consumes zero provider calls"),
		InvalidGkProvider.Purposes.Num(), 0);
	FQueueRollProvider Failure;
	Failure.Results = { RollFailure() };
	const auto Failed =
		FMatchPlayLongFreeKickResolution::ResolveDirectAttackRoll(
			Direct, RollRequest(Direct, false), &Failure);
	TestFalse(TEXT("Provider failure rejected"), Failed.bSuccess);
	TestTrue(TEXT("Failure preserves exact state"),
		Equal(Direct, Failed.AfterState));

	FMatchPlayState Power = BindCarrier(
		MakeLongAwaitingCarrier(TEXT("LFK_Power")));
	Power = FMatchPlayLongFreeKickResolution::SubmitMethod(
		Power, MethodRequest(Power, EMatchPlayLongFreeKickMethod::Power))
		.AfterState;
	TestEqual(TEXT("Power method selects Power stage"),
		Power.CurrentAttack.SetPieceRoute.LongFreeKick.Stage,
		EMatchPlaySetPieceCarrierRouteStage::PowerAwaitingRoll);
	FQueueRollProvider PairFailure;
	PairFailure.Results = { RollSuccess(6), RollFailure() };
	const auto FailedPair = FMatchPlayLongFreeKickResolution::ResolvePowerRoll(
		Power, RollRequest(Power, false), &PairFailure);
	TestFalse(TEXT("Second provider failure rejects pair"),
		FailedPair.bSuccess);
	TestTrue(TEXT("Power pair failure adopts no partial roll"),
		Equal(Power, FailedPair.AfterState));
	TestTrue(TEXT("Power requests ordered A then B"),
		PairFailure.Purposes.Num() == 2
		&& PairFailure.Purposes[0]
			== EMatchPlayCurrentAttackPostRouteRollPurpose::LongFreeKickPowerA
		&& PairFailure.Purposes[1]
			== EMatchPlayCurrentAttackPostRouteRollPurpose::LongFreeKickPowerB);
	FQueueRollProvider PairSuccess;
	PairSuccess.Results = { RollSuccess(6), RollSuccess(5) };
	const auto Goal = FMatchPlayLongFreeKickResolution::ResolvePowerRoll(
		Power, RollRequest(Power, false), &PairSuccess);
	TestTrue(TEXT("Power 11 resolves Goal"), Goal.bSuccess);
	const auto& GoalLong = Goal.AfterState.CurrentAttack.SetPieceRoute.LongFreeKick;
	TestTrue(TEXT("Power persists pair without defense or Formula"),
		GoalLong.bHasPowerD6Pair && GoalLong.PowerD6A == 6
		&& GoalLong.PowerD6B == 5 && !GoalLong.bHasDefenseD6
		&& !GoalLong.bHasFormulaResolution
		&& GoalLong.GameplayOutcome
			== EMatchPlayLongFreeKickGameplayOutcome::Goal);
	FMatchPlayState CorruptPower = Goal.AfterState;
	auto& CorruptLong =
		CorruptPower.CurrentAttack.SetPieceRoute.LongFreeKick;
	CorruptLong.GameplayOutcome = EMatchPlayLongFreeKickGameplayOutcome::NoGoal;
	CorruptLong.bHasGoalScorer = false;
	CorruptLong.GoalScorerCardId = NAME_None;
	TestFalse(TEXT("Reconstruction rejects Power threshold/outcome mismatch"),
		FMatchPlayCurrentAttackRouteStateValidator::Validate(CorruptPower)
			.bIsCanonical);
	FQueueRollProvider DuplicateProvider;
	DuplicateProvider.Results = { RollSuccess(6), RollSuccess(6) };
	const auto Duplicate = FMatchPlayLongFreeKickResolution::ResolvePowerRoll(
		Goal.AfterState, RollRequest(Goal.AfterState, false), &DuplicateProvider);
	TestFalse(TEXT("Power terminal duplicate rejected"), Duplicate.bSuccess);
	TestEqual(TEXT("Power terminal duplicate consumes zero RNG"),
		DuplicateProvider.Purposes.Num(), 0);

	FMatchPlayState Sum12 = BindCarrier(
		MakeLongAwaitingCarrier(TEXT("LFK_Power12")));
	Sum12 = FMatchPlayLongFreeKickResolution::SubmitMethod(
		Sum12, MethodRequest(Sum12, EMatchPlayLongFreeKickMethod::Power))
		.AfterState;
	FQueueRollProvider Sum12Provider;
	Sum12Provider.Results = { RollSuccess(6), RollSuccess(6) };
	const auto Sum12Result = FMatchPlayLongFreeKickResolution::ResolvePowerRoll(
		Sum12, RollRequest(Sum12, false), &Sum12Provider);
	TestTrue(TEXT("Power 12 resolves Goal"),
		Sum12Result.bSuccess
		&& Sum12Result.AfterState.CurrentAttack.SetPieceRoute.LongFreeKick
			.GameplayOutcome
				== EMatchPlayLongFreeKickGameplayOutcome::Goal);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayLongFreeKickAdvanceAndNoCarrierTest,
	"FMCodex.CoreRules.MatchPlayLongFreeKick.AdvanceRecoveryAndNoCarrier",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayLongFreeKickAdvanceAndNoCarrierTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayLongFreeKickResolutionTests;
	FName Carrier;
	FMatchPlayState State = MakeLongAwaitingCarrier(TEXT("LFK_Advance"));
	const EInitialTurnOrderPlayer Attacker =
		State.RuntimeState.CurrentAttackingPlayer;
	for (int32 Index = 1; Index <= 3; ++Index)
	{
		MoveToUsed(State, Attacker, FindCard(State, Attacker, false, Index));
	}
	State = BindCarrier(MoveTemp(State), &Carrier);
	State = FMatchPlayLongFreeKickResolution::SubmitMethod(
		State, MethodRequest(State, EMatchPlayLongFreeKickMethod::Power))
		.AfterState;
	FQueueRollProvider Rolls;
	Rolls.Results = { RollSuccess(5), RollSuccess(5) };
	const auto Terminal = FMatchPlayLongFreeKickResolution::ResolvePowerRoll(
		State, RollRequest(State, false), &Rolls);
	TestTrue(TEXT("Power NoGoal terminal succeeds"), Terminal.bSuccess);
	TestTrue(TEXT("Carrier remains Available before Advance"),
		Usage(Terminal.AfterState, Attacker).AvailableCardIds.Contains(Carrier));
	FRecoveryProvider Recovery;
	Recovery.PreferredCard = Carrier;
	const auto Advanced = FMatchPlayCurrentAttackCompletion::AdvanceAfterTerminal(
		Terminal.AfterState, 1, Attacker, &Recovery);
	TestTrue(TEXT("Shared Advance succeeds"), Advanced.bSuccess);
	TestEqual(TEXT("Shared consumer extracts exactly Carrier"),
		Advanced.SetPieceCardUsageResults.Num(), 1);
	TestTrue(TEXT("Recovery candidate set includes newly Used Carrier"),
		Advanced.RecoveryResolveResult.CandidateQueryResult.Candidates
			.ContainsByPredicate(
			[Carrier](const FMatchPlayRecoveryCandidate& Candidate)
			{ return Candidate.CardId == Carrier; }));
	TestTrue(TEXT("Unselected Carrier remains Used"),
		Usage(Advanced.AfterState, Attacker).UsedCardIds.Contains(Carrier));
	TestEqual(TEXT("Exactly one opportunity consumed"),
		Attacker == EInitialTurnOrderPlayer::PlayerA
			? Advanced.AfterState.RuntimeState.PlayerAState.UsedAttackCount
			: Advanced.AfterState.RuntimeState.PlayerBState.UsedAttackCount,
		1);
	TestEqual(TEXT("Next attacker follows shared authority"),
		Advanced.NextAttackingPlayer, Other(Attacker));

	FRecoveryProvider RecoverCarrier;
	RecoverCarrier.PreferredCard = Carrier;
	RecoverCarrier.bSelectPreferred = true;
	const auto Recovered = FMatchPlayCurrentAttackCompletion::AdvanceAfterTerminal(
		Terminal.AfterState, 1, Attacker, &RecoverCarrier);
	TestTrue(TEXT("Same transaction may recover Carrier"),
		Recovered.bSuccess
		&& Usage(Recovered.AfterState, Attacker)
			.AvailableCardIds.Contains(Carrier));

	FRecoveryProvider FailedRecovery;
	FailedRecovery.bFail = true;
	const auto FailedAdvance =
		FMatchPlayCurrentAttackCompletion::AdvanceAfterTerminal(
			Terminal.AfterState, 1, Attacker, &FailedRecovery);
	TestFalse(TEXT("Recovery failure rolls back whole Advance"),
		FailedAdvance.bSuccess);
	TestTrue(TEXT("Recovery failure preserves terminal and Available Carrier"),
		Equal(Terminal.AfterState, FailedAdvance.AfterState)
		&& Usage(FailedAdvance.AfterState, Attacker)
			.AvailableCardIds.Contains(Carrier));

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
	FRecoveryProvider FinalProvider;
	const auto FinalAdvance =
		FMatchPlayCurrentAttackCompletion::AdvanceAfterTerminal(
			Final, 2, Attacker, &FinalProvider);
	TestTrue(TEXT("Final Long FK Advance resolves MatchResult"),
		FinalAdvance.bSuccess && FinalAdvance.bMatchEnded
		&& FinalAdvance.MatchResultResolveResult.bSuccess);
	TestTrue(TEXT("Final Advance consumes Carrier"),
		Usage(FinalAdvance.AfterState, Attacker)
			.UsedCardIds.Contains(Carrier));
	TestEqual(TEXT("Final Advance skips Recovery"),
		FinalProvider.CallCount, 0);

	FMatchPlayState OneLegal =
		MakeLongAwaitingCarrier(TEXT("LFK_OneLegal"));
	const EInitialTurnOrderPlayer OneLegalAttacker =
		OneLegal.RuntimeState.CurrentAttackingPlayer;
	const FName SoleLegal = FindCard(OneLegal, OneLegalAttacker, false);
	FCardUsageState& OneLegalUsage = Usage(OneLegal, OneLegalAttacker);
	for (const FName CardId : TArray<FName>(OneLegalUsage.AvailableCardIds))
	{
		if (CardId != SoleLegal
			&& CardId != FindCard(OneLegal, OneLegalAttacker, true))
		{
			OneLegalUsage.AvailableCardIds.Remove(CardId);
			OneLegalUsage.UsedCardIds.Add(CardId);
		}
	}
	FMatchPlayLongFreeKickNoLegalCarrierRequest OneLegalRequest;
	OneLegalRequest.RequestingSide = OneLegalAttacker;
	OneLegalRequest.AttackSequence = 1;
	const auto OneLegalRejected =
		FMatchPlayLongFreeKickResolution::ResolveNoLegalCarrier(
			OneLegal, OneLegalRequest);
	TestEqual(TEXT("One legal Carrier rejects shortage claim"),
		OneLegalRejected.ErrorCode,
		EMatchPlayLongFreeKickResolutionErrorCode::LegalCarrierExists);
	TestTrue(TEXT("Rejected shortage claim preserves state"),
		Equal(OneLegal, OneLegalRejected.AfterState));

	FMatchPlayState NoCarrier =
		MakeLongAwaitingCarrier(TEXT("LFK_NoCarrierCore"));
	const EInitialTurnOrderPlayer NoCarrierAttacker =
		NoCarrier.RuntimeState.CurrentAttackingPlayer;
	FCardUsageState& AttackerUsage = Usage(NoCarrier, NoCarrierAttacker);
	for (const FPlayerCardRuleSnapshot& Snapshot :
		Snapshots(NoCarrier, NoCarrierAttacker).Cards)
	{
		if (!Snapshot.bIsGoalkeeper)
		{
			AttackerUsage.AvailableCardIds.Remove(Snapshot.CardId);
			AttackerUsage.UsedCardIds.Add(Snapshot.CardId);
		}
	}
	FMatchPlayLongFreeKickNoLegalCarrierRequest Request;
	Request.RequestingSide = NoCarrierAttacker;
	Request.AttackSequence = 1;
	const auto NoLegal = FMatchPlayLongFreeKickResolution::ResolveNoLegalCarrier(
		NoCarrier, Request);
	TestTrue(TEXT("Common availability proves no legal Carrier"),
		NoLegal.bSuccess);
	TestTrue(TEXT("No-Carrier terminal is canonical and explicit"),
		NoLegal.AfterRouteValidation.bIsCanonical
		&& NoLegal.AfterState.CurrentAttack.SetPieceRoute.LongFreeKick
			.bNoLegalCarrier);
	FRecoveryProvider NoCarrierRecovery;
	const auto NoCarrierAdvanced =
		FMatchPlayCurrentAttackCompletion::AdvanceAfterTerminal(
			NoLegal.AfterState, 1, NoCarrierAttacker, &NoCarrierRecovery);
	TestTrue(TEXT("No-Carrier uses shared Advance"),
		NoCarrierAdvanced.bSuccess);
	TestEqual(TEXT("No-Carrier consumes zero participant cards"),
		NoCarrierAdvanced.SetPieceCardUsageResults.Num(), 0);

	FMatchPlayState Ejected =
		MakeLongAwaitingCarrier(TEXT("LFK_NoCarrierEjected"));
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
	FMatchPlayLongFreeKickNoLegalCarrierRequest EjectedRequest;
	EjectedRequest.RequestingSide = EjectedAttacker;
	EjectedRequest.AttackSequence = 1;
	TestTrue(TEXT("Only Ejected non-GKs prove no legal Carrier"),
		FMatchPlayLongFreeKickResolution::ResolveNoLegalCarrier(
			Ejected, EjectedRequest).bSuccess);

	FMatchPlayState EmptyAvailable =
		MakeLongAwaitingCarrier(TEXT("LFK_NoCarrierEmptyAvailable"));
	const EInitialTurnOrderPlayer EmptyAttacker =
		EmptyAvailable.RuntimeState.CurrentAttackingPlayer;
	Usage(EmptyAvailable, EmptyAttacker).AvailableCardIds.Reset();
	FMatchPlayLongFreeKickNoLegalCarrierRequest EmptyRequest;
	EmptyRequest.RequestingSide = EmptyAttacker;
	EmptyRequest.AttackSequence = 1;
	TestTrue(TEXT("Empty Available pool proves no legal Carrier"),
		FMatchPlayLongFreeKickResolution::ResolveNoLegalCarrier(
			EmptyAvailable, EmptyRequest).bSuccess);
	return true;
}

#endif
