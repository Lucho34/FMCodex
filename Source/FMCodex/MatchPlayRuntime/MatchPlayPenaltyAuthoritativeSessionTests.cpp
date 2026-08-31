#include "MatchPlayAuthoritativeSession.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

namespace MatchPlayPenaltyAuthoritativeSessionTests
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
		Card.Attributes.Shooting = 5;
		Card.Attributes.Passing = 4;
		Card.Attributes.Stamina = 5;
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

	FMatchPlayOpeningInitializeInput MakeOpening(const FString& Prefix)
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
		return Input;
	}

	EInitialTurnOrderPlayer Other(const EInitialTurnOrderPlayer Side)
	{
		return Side == EInitialTurnOrderPlayer::PlayerA
			? EInitialTurnOrderPlayer::PlayerB
			: EInitialTurnOrderPlayer::PlayerA;
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

	FName FindOutfield(
		const FMatchPlayState& State, const EInitialTurnOrderPlayer Side)
	{
		for (const FPlayerCardRuleSnapshot& Snapshot : Snapshots(State, Side).Cards)
		{
			if (!Snapshot.bIsGoalkeeper)
			{
				return Snapshot.CardId;
			}
		}
		return NAME_None;
	}

	FMatchPlayAttackEntryRollProviderResult EntrySuccess(const int32 Raw)
	{
		FMatchPlayAttackEntryRollProviderResult Result;
		Result.bSuccess = true;
		Result.RawRoll = Raw;
		return Result;
	}

	FMatchPlayPostRouteRollProviderResult PostSuccess(const int32 Raw)
	{
		FMatchPlayPostRouteRollProviderResult Result;
		Result.bSuccess = true;
		Result.RawD6 = Raw;
		return Result;
	}

	FMatchPlayPostRouteRollProviderResult PostFailure()
	{
		FMatchPlayPostRouteRollProviderResult Result;
		Result.ErrorCode =
			EMatchPlayPostRouteRollProviderErrorCode::ProviderFailure;
		Result.ErrorMessage = TEXT("Injected Session Penalty failure.");
		return Result;
	}

	class FAllProvider final
		: public IMatchPlayAttackEntryRollProvider
		, public IMatchPlayInitialRouteRollProvider
		, public IMatchPlayPostRouteRollProvider
		, public IMatchPlayRecoveryProvider
	{
	public:
		virtual FMatchPlayAttackEntryRollProviderResult RollD12(
			EMatchPlayAttackEntryRollPurpose Purpose) override
		{
			++D12Calls;
			return D12Results.IsValidIndex(NextD12)
				? D12Results[NextD12++] : FMatchPlayAttackEntryRollProviderResult();
		}
		virtual FMatchPlayAttackEntryRollProviderResult RollD6(
			EMatchPlayAttackEntryRollPurpose Purpose) override
		{
			++EntryD6Calls;
			return EntryD6Results.IsValidIndex(NextEntryD6)
				? EntryD6Results[NextEntryD6++]
				: FMatchPlayAttackEntryRollProviderResult();
		}
		virtual FMatchPlayAttackEntrySelectionProviderResult SelectUniformIndex(
			EMatchPlayAttackEntryRollPurpose Purpose, int32 CandidateCount) override
		{
			return FMatchPlayAttackEntrySelectionProviderResult();
		}
		virtual FMatchPlayInitialRouteRollProviderResult RollD6(
			EMatchPlayCurrentAttackResolutionRollPurpose Purpose) override
		{
			return FMatchPlayInitialRouteRollProviderResult();
		}
		virtual FMatchPlayPostRouteRollProviderResult RollD6(
			const EMatchPlayCurrentAttackPostRouteRollPurpose Purpose) override
		{
			PostPurposes.Add(Purpose);
			return PostResults.IsValidIndex(NextPost)
				? PostResults[NextPost++] : PostFailure();
		}
		virtual FMatchPlayRecoveryProviderResult DrawWeightedWithoutReplacement(
			EMatchPlayRecoveryPurpose Purpose,
			const TArray<FMatchPlayRecoveryCandidate>& OrderedCandidates,
			const int32 ReturnCount) override
		{
			++RecoveryCalls;
			RecoveryCandidates = OrderedCandidates;
			FMatchPlayRecoveryProviderResult Result;
			Result.bSuccess = true;
			for (int32 Index = 0; Index < ReturnCount; ++Index)
			{
				Result.SelectedCandidateIndices.Add(Index);
			}
			return Result;
		}

		TArray<FMatchPlayAttackEntryRollProviderResult> D12Results;
		TArray<FMatchPlayAttackEntryRollProviderResult> EntryD6Results;
		TArray<FMatchPlayPostRouteRollProviderResult> PostResults;
		TArray<EMatchPlayCurrentAttackPostRouteRollPurpose> PostPurposes;
		TArray<FMatchPlayRecoveryCandidate> RecoveryCandidates;
		int32 NextD12 = 0;
		int32 NextEntryD6 = 0;
		int32 NextPost = 0;
		int32 D12Calls = 0;
		int32 EntryD6Calls = 0;
		int32 RecoveryCalls = 0;
	};

	bool Equal(const FMatchPlayState& Left, const FMatchPlayState& Right)
	{
		return FMatchPlayState::StaticStruct()->CompareScriptStruct(
			&Left, &Right, 0);
	}

	FMatchPlayState BootstrapAwaitingCarrier(
		FAllProvider& Provider, const FString& Prefix)
	{
		Provider.D12Results = { EntrySuccess(9) };
		Provider.EntryD6Results = { EntrySuccess(6) };
		FSkillRuleSnapshotSet Skills;
		FMatchPlayAuthoritativeSession Session(
			Provider, Provider, Provider, Provider, Skills);
		Session.InitializeMatch(MakeOpening(Prefix));
		FMatchPlayState State = Session.GetStateSnapshot();
		FMatchPlayFullD12EntryRequest Entry;
		Entry.RequestingSide = State.RuntimeState.CurrentAttackingPlayer;
		Entry.ExpectedAttackSequence = 1;
		Session.RequestInitialActionPointRoll(Entry);
		FMatchPlaySetPieceTypeRollRequest Type;
		Type.RequestingSide = Entry.RequestingSide;
		Type.AttackSequence = 1;
		Session.RequestSetPieceTypeRoll(Type);
		return Session.GetStateSnapshot();
	}

	FMatchPlayState SelectCarrier(
		FMatchPlayState State, FAllProvider& Provider, FName* OutCarrier = nullptr)
	{
		const EInitialTurnOrderPlayer Attacker =
			State.RuntimeState.CurrentAttackingPlayer;
		const FName Carrier = FindOutfield(State, Attacker);
		FMatchPlayAuthoritativeSession Session(
			State, Provider, Provider, Provider);
		FMatchPlaySetPieceCarrierSelectionRequest Request;
		Request.RequestingSide = Attacker;
		Request.AttackSequence = State.CurrentAttack.AttackSequence;
		Request.CardId = Carrier;
		Session.SubmitSetPieceCarrier(Request);
		if (OutCarrier != nullptr)
		{
			*OutCarrier = Carrier;
		}
		return Session.GetStateSnapshot();
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayPenaltyAuthoritativeSessionDirectEndToEndTest,
	"FMCodex.MatchPlayRuntime.AuthoritativeSession.PenaltyDirectEndToEnd",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayPenaltyAuthoritativeSessionDirectEndToEndTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayPenaltyAuthoritativeSessionTests;
	FAllProvider Provider;
	Provider.D12Results = { EntrySuccess(12) };
	Provider.EntryD6Results = { EntrySuccess(6) };
	Provider.PostResults = { PostFailure(), PostSuccess(3), PostSuccess(1) };
	FSkillRuleSnapshotSet Skills;
	FMatchPlayAuthoritativeSession Session(
		Provider, Provider, Provider, Provider, Skills);
	TestTrue(TEXT("Session initializes"),
		Session.InitializeMatch(MakeOpening(TEXT("PenaltySessionDirect")))
			.OpeningResult.bSuccess);
	FMatchPlayState State = Session.GetStateSnapshot();
	const EInitialTurnOrderPlayer Attacker =
		State.RuntimeState.CurrentAttackingPlayer;
	const EInitialTurnOrderPlayer Defender = Other(Attacker);
	FMatchPlayFullD12EntryRequest Entry;
	Entry.RequestingSide = Attacker;
	Entry.ExpectedAttackSequence = 1;
	TestTrue(TEXT("Full D12 enters Set Piece"),
		Session.RequestInitialActionPointRoll(Entry).EntryResult.bSuccess);
	FMatchPlaySetPieceTypeRollRequest Type;
	Type.RequestingSide = Attacker;
	Type.AttackSequence = 1;
	TestTrue(TEXT("Type D6 6 selects Penalty"),
		Session.RequestSetPieceTypeRoll(Type).TypeRollResult.bSuccess);
	State = Session.GetStateSnapshot();
	const FName Carrier = FindOutfield(State, Attacker);
	FMatchPlaySetPieceCarrierSelectionRequest CarrierRequest;
	CarrierRequest.RequestingSide = Attacker;
	CarrierRequest.AttackSequence = 1;
	CarrierRequest.CardId = Carrier;
	TestTrue(TEXT("Session selects Carrier"),
		Session.SubmitSetPieceCarrier(CarrierRequest).CarrierResult.bSuccess);

	FMatchPlayPenaltyMethodRequest Method;
	Method.RequestingSide = Attacker;
	Method.AttackSequence = 1;
	Method.Method = EMatchPlayPenaltyMethod::Direct;
	FMatchPlayPenaltyMethodRequest WrongMethod = Method;
	WrongMethod.RequestingSide = Defender;
	const int32 BeforeWrongMethod = Provider.PostPurposes.Num();
	TestFalse(TEXT("Wrong-side method rejected"),
		Session.SubmitPenaltyMethod(WrongMethod).ResolutionResult.bSuccess);
	TestEqual(TEXT("Wrong-side method consumes zero RNG"),
		Provider.PostPurposes.Num(), BeforeWrongMethod);
	TestTrue(TEXT("Direct method accepted"),
		Session.SubmitPenaltyMethod(Method).ResolutionResult.bSuccess);

	FMatchPlayPenaltyRollRequest AttackRoll;
	AttackRoll.RequestingSide = Attacker;
	AttackRoll.AttackSequence = 1;
	FMatchPlayPenaltyRollRequest Stale = AttackRoll;
	++Stale.AttackSequence;
	const int32 BeforeStale = Provider.PostPurposes.Num();
	TestFalse(TEXT("Stale attack request rejected"),
		Session.ResolvePenaltyDirectAttackRoll(Stale)
			.ResolutionResult.bSuccess);
	TestEqual(TEXT("Stale request consumes zero RNG"),
		Provider.PostPurposes.Num(), BeforeStale);
	const FMatchPlayState BeforeFailure = Session.GetStateSnapshot();
	TestFalse(TEXT("Provider failure remains retryable"),
		Session.ResolvePenaltyDirectAttackRoll(AttackRoll)
			.ResolutionResult.bSuccess);
	TestTrue(TEXT("Provider failure does not adopt state"),
		Equal(BeforeFailure, Session.GetStateSnapshot()));
	TestTrue(TEXT("Fresh attack retry succeeds"),
		Session.ResolvePenaltyDirectAttackRoll(AttackRoll)
			.ResolutionResult.bSuccess);
	const FMatchPlayState MidDirect = Session.GetStateSnapshot();
	TestTrue(TEXT("Mid-Direct stores attack prefix only"),
		MidDirect.CurrentAttack.SetPieceRoute.Penalty.bHasAttackD6
			&& !MidDirect.CurrentAttack.SetPieceRoute.Penalty.bHasDefenseD6);

	FMatchPlayPenaltyRollRequest DefenseRoll;
	DefenseRoll.RequestingSide = Defender;
	DefenseRoll.AttackSequence = 1;
	FAllProvider RebuildProvider;
	RebuildProvider.PostResults = { PostSuccess(1) };
	FMatchPlayAuthoritativeSession RebuiltMid(
		MidDirect, RebuildProvider, RebuildProvider, RebuildProvider);
	TestTrue(TEXT("Reconstructed attack prefix completes"),
		RebuiltMid.ResolvePenaltyDirectDefenseRoll(DefenseRoll)
			.ResolutionResult.bSuccess);
	TestTrue(TEXT("Original Session resolves Direct Goal"),
		Session.ResolvePenaltyDirectDefenseRoll(DefenseRoll)
			.ResolutionResult.bSuccess);
	const FMatchPlayState Terminal = Session.GetStateSnapshot();
	TestTrue(TEXT("Direct Goal persists score/scorer/terminal"),
		Terminal.CurrentAttack.LifecycleState
			== EMatchPlayCurrentAttackLifecycleState::TerminalPendingAdvance
			&& Terminal.CurrentAttack.SetPieceRoute.Penalty.GoalScorerCardId
				== Carrier
			&& Terminal.RuntimeState.PlayerAState.Score
				+ Terminal.RuntimeState.PlayerBState.Score == 1);
	const int32 BeforeDuplicate = Provider.PostPurposes.Num();
	const auto Duplicate = Session.ResolvePenaltyDirectDefenseRoll(DefenseRoll);
	TestEqual(TEXT("Terminal duplicate rejected by Session gate"),
		Duplicate.RuntimeEnvelope.RuntimeFailureCode,
		EMatchPlayAuthoritativeRuntimeFailureCode::TerminalAdvanceRequired);
	TestEqual(TEXT("Terminal duplicate consumes zero RNG"),
		Provider.PostPurposes.Num(), BeforeDuplicate);

	FMatchPlayAuthoritativeAdvanceAfterTerminalRequest Advance;
	Advance.RequestingSide = Attacker;
	Advance.AttackSequence = 1;
	TestTrue(TEXT("Shared Session Advance consumes Penalty participant"),
		Session.AdvanceAfterTerminal(Advance)
			.CompletionResult.SetPieceCardUsageResults.Num() == 1);
	FAllProvider TerminalProvider;
	FMatchPlayAuthoritativeSession RebuiltTerminal(
		Terminal, TerminalProvider, TerminalProvider, TerminalProvider);
	TestTrue(TEXT("Reconstructed terminal advances"),
		RebuiltTerminal.AdvanceAfterTerminal(Advance)
			.CompletionResult.bSuccess);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayPenaltyAuthoritativeSessionAlternatePathsTest,
	"FMCodex.MatchPlayRuntime.AuthoritativeSession.PenaltyAlternatePaths",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayPenaltyAuthoritativeSessionAlternatePathsTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayPenaltyAuthoritativeSessionTests;
	FAllProvider Bootstrap;
	const FMatchPlayState AwaitingCarrier =
		BootstrapAwaitingCarrier(Bootstrap, TEXT("PenaltySessionAlternates"));
	const EInitialTurnOrderPlayer Attacker =
		AwaitingCarrier.RuntimeState.CurrentAttackingPlayer;
	const EInitialTurnOrderPlayer Defender = Other(Attacker);
	FAllProvider SelectionProvider;
	FName Carrier = NAME_None;
	const FMatchPlayState AwaitingMethod = SelectCarrier(
		AwaitingCarrier, SelectionProvider, &Carrier);

	auto ResolvePanenka = [&](const int32 D6)
	{
		FAllProvider Provider;
		Provider.PostResults = { PostSuccess(D6) };
		FMatchPlayAuthoritativeSession Session(
			AwaitingMethod, Provider, Provider, Provider);
		FMatchPlayPenaltyMethodRequest Method;
		Method.RequestingSide = Attacker;
		Method.AttackSequence = 1;
		Method.Method = EMatchPlayPenaltyMethod::Panenka;
		Session.SubmitPenaltyMethod(Method);
		FMatchPlayPenaltyRollRequest Roll;
		Roll.RequestingSide = Attacker;
		Roll.AttackSequence = 1;
		const auto Result = Session.ResolvePenaltyPanenkaRoll(Roll);
		return Result.ResolutionResult.AfterState;
	};
	const FMatchPlayState PanenkaMiss = ResolvePanenka(1);
	const FMatchPlayState PanenkaGoal = ResolvePanenka(6);
	TestTrue(TEXT("Real Session Panenka Miss and Goal persist"),
		PanenkaMiss.CurrentAttack.SetPieceRoute.Penalty.GameplayOutcome
			== EMatchPlayPenaltyGameplayOutcome::NoGoal
			&& PanenkaGoal.CurrentAttack.SetPieceRoute.Penalty.GameplayOutcome
				== EMatchPlayPenaltyGameplayOutcome::Goal
			&& PanenkaGoal.CurrentAttack.SetPieceRoute.Penalty.GoalScorerCardId
				== Carrier);

	FAllProvider DirectMissProvider;
	DirectMissProvider.PostResults = { PostSuccess(1), PostSuccess(6) };
	FMatchPlayAuthoritativeSession DirectMissSession(
		AwaitingMethod, DirectMissProvider, DirectMissProvider,
		DirectMissProvider);
	FMatchPlayPenaltyMethodRequest Direct;
	Direct.RequestingSide = Attacker;
	Direct.AttackSequence = 1;
	Direct.Method = EMatchPlayPenaltyMethod::Direct;
	DirectMissSession.SubmitPenaltyMethod(Direct);
	FMatchPlayPenaltyRollRequest AttackRoll;
	AttackRoll.RequestingSide = Attacker;
	AttackRoll.AttackSequence = 1;
	DirectMissSession.ResolvePenaltyDirectAttackRoll(AttackRoll);
	FMatchPlayPenaltyRollRequest DefenseRoll;
	DefenseRoll.RequestingSide = Defender;
	DefenseRoll.AttackSequence = 1;
	TestTrue(TEXT("Real Session Direct NoGoal persists"),
		DirectMissSession.ResolvePenaltyDirectDefenseRoll(DefenseRoll)
			.ResolutionResult.AfterState.CurrentAttack.SetPieceRoute.Penalty
			.GameplayOutcome == EMatchPlayPenaltyGameplayOutcome::NoGoal);

	FMatchPlayState NoCarrier = AwaitingCarrier;
	FCardUsageState& CardUsage = Usage(NoCarrier, Attacker);
	for (const FPlayerCardRuleSnapshot& Snapshot : Snapshots(NoCarrier, Attacker).Cards)
	{
		if (!Snapshot.bIsGoalkeeper)
		{
			CardUsage.AvailableCardIds.Remove(Snapshot.CardId);
			CardUsage.UsedCardIds.Add(Snapshot.CardId);
		}
	}
	FAllProvider NoCarrierProvider;
	FMatchPlayAuthoritativeSession NoCarrierSession(
		NoCarrier, NoCarrierProvider, NoCarrierProvider, NoCarrierProvider);
	FMatchPlayPenaltyNoLegalCarrierRequest NoCarrierRequest;
	NoCarrierRequest.RequestingSide = Attacker;
	NoCarrierRequest.AttackSequence = 1;
	TestTrue(TEXT("Real Session resolves authoritative no-legal Carrier"),
		NoCarrierSession.ResolveNoLegalSetPieceCarrier(NoCarrierRequest)
			.ResolutionResult.bSuccess);
	TestEqual(TEXT("No-legal Carrier consumes zero post-route RNG"),
		NoCarrierProvider.PostPurposes.Num(), 0);

	FMatchPlayState Final = PanenkaMiss;
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
	FAllProvider FinalProvider;
	FMatchPlayAuthoritativeSession FinalSession(
		Final, FinalProvider, FinalProvider, FinalProvider);
	FMatchPlayAuthoritativeAdvanceAfterTerminalRequest FinalAdvance;
	FinalAdvance.RequestingSide = Attacker;
	FinalAdvance.AttackSequence = 2;
	const auto FinalResult = FinalSession.AdvanceAfterTerminal(FinalAdvance);
	TestTrue(TEXT("Real Session final Advance ends match and consumes Carrier"),
		FinalResult.CompletionResult.bSuccess
			&& FinalResult.CompletionResult.bMatchEnded
			&& FinalProvider.RecoveryCalls == 0);
	return true;
}

#endif
