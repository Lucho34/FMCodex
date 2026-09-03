#include "MatchPlayAuthoritativeSession.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

namespace MatchPlayCornerAuthoritativeSessionTests
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
		Card.Attributes.Strength = 4;
		Card.Attributes.Shooting = 4;
		Card.Attributes.Marking = 4;
		Card.Attributes.Stamina = 5;
		Card.GoalkeeperAttributes.Aerial = 4;
		Card.GoalkeeperAttributes.Reflex = 4;
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

	const FCardUsageState& Usage(
		const FMatchPlayState& State, const EInitialTurnOrderPlayer Side)
	{
		return Side == EInitialTurnOrderPlayer::PlayerA
			? State.CardUsageState.PlayerACardUsageState
			: State.CardUsageState.PlayerBCardUsageState;
	}

	TArray<FName> FirstOutfield(
		const FMatchPlayState& State,
		const EInitialTurnOrderPlayer Side,
		const int32 Count)
	{
		TArray<FName> Result;
		for (const FPlayerCardRuleSnapshot& Snapshot : Snapshots(State, Side).Cards)
		{
			if (!Snapshot.bIsGoalkeeper && Result.Num() < Count)
			{
				Result.Add(Snapshot.CardId);
			}
		}
		return Result;
	}

	bool Equal(const FMatchPlayState& Left, const FMatchPlayState& Right)
	{
		return FMatchPlayState::StaticStruct()->CompareScriptStruct(
			&Left, &Right, 0);
	}

	FMatchPlayAttackEntryRollProviderResult EntrySuccess(const int32 Raw)
	{
		FMatchPlayAttackEntryRollProviderResult Result;
		Result.bSuccess = true;
		Result.RawRoll = Raw;
		return Result;
	}

	FMatchPlayPostRouteRollProviderResult PostSuccess(const int32 RawD6)
	{
		FMatchPlayPostRouteRollProviderResult Result;
		Result.bSuccess = true;
		Result.RawD6 = RawD6;
		return Result;
	}

	FMatchPlayPostRouteRollProviderResult PostFailure()
	{
		FMatchPlayPostRouteRollProviderResult Result;
		Result.ErrorCode =
			EMatchPlayPostRouteRollProviderErrorCode::ProviderFailure;
		Result.ErrorMessage = TEXT("Injected Session Corner failure.");
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
			if (bFailRecovery)
			{
				Result.ErrorCode =
					EMatchPlayRecoveryProviderErrorCode::ProviderFailure;
				Result.ErrorMessage = TEXT("Injected Session Corner Recovery failure.");
				return Result;
			}
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
		bool bFailRecovery = false;
	};

	FMatchPlayCornerNominationRequest Nomination(
		const FMatchPlayState& State,
		const EInitialTurnOrderPlayer Side,
		const int32 Count)
	{
		FMatchPlayCornerNominationRequest Request;
		Request.RequestingSide = Side;
		Request.AttackSequence = State.CurrentAttack.AttackSequence;
		Request.OrderedCardIds = FirstOutfield(State, Side, Count);
		return Request;
	}

	FMatchPlayCornerRollRequest RollRequest(
		const FMatchPlayState& State, const bool bDefender)
	{
		FMatchPlayCornerRollRequest Request;
		const EInitialTurnOrderPlayer Attacker =
			State.RuntimeState.CurrentAttackingPlayer;
		Request.RequestingSide = bDefender ? Other(Attacker) : Attacker;
		Request.AttackSequence = State.CurrentAttack.AttackSequence;
		return Request;
	}

	FMatchPlayState BootstrapAwaitingAttacker(
		FAllProvider& Provider, const FString& Prefix)
	{
		Provider.D12Results = { EntrySuccess(9) };
		Provider.EntryD6Results = { EntrySuccess(1) };
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

	FMatchPlayState CompleteCorner(
		FMatchPlayState State,
		FAllProvider& Provider,
		const int32 AttackerCount,
		const int32 DefenderCount,
		const EMatchPlayCornerRouteIntent Intent)
	{
		FMatchPlayAuthoritativeSession Session(
			State, Provider, Provider, Provider);
		const EInitialTurnOrderPlayer Attacker =
			State.RuntimeState.CurrentAttackingPlayer;
		const EInitialTurnOrderPlayer Defender = Other(Attacker);
		Session.SubmitCornerAttackerNominations(
			Nomination(State, Attacker, AttackerCount));
		State = Session.GetStateSnapshot();
		Session.SubmitCornerDefenderNominations(
			Nomination(State, Defender, DefenderCount));
		State = Session.GetStateSnapshot();
		if (AttackerCount == 0 || DefenderCount == 0)
		{
			return State;
		}
		Session.RequestCornerParticipantSelectionRoll(
			RollRequest(State, false));
		State = Session.GetStateSnapshot();
		FMatchPlayCornerIntentRequest IntentRequest;
		IntentRequest.RequestingSide = Attacker;
		IntentRequest.AttackSequence = State.CurrentAttack.AttackSequence;
		IntentRequest.IntendedRoute = Intent;
		Session.SubmitCornerIntent(IntentRequest);
		State = Session.GetStateSnapshot();
		Session.RequestCornerRouteRoll(RollRequest(State, false));
		State = Session.GetStateSnapshot();
		Session.RequestCornerAttackRoll(RollRequest(State, false));
		State = Session.GetStateSnapshot();
		Session.RequestCornerDefenseRoll(RollRequest(State, true));
		return Session.GetStateSnapshot();
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayCornerAuthoritativeSessionEndToEndTest,
	"FMCodex.MatchPlayRuntime.AuthoritativeSession.CornerHighGoalEndToEnd",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayCornerAuthoritativeSessionEndToEndTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayCornerAuthoritativeSessionTests;
	FAllProvider Provider;
	Provider.D12Results = { EntrySuccess(9) };
	Provider.EntryD6Results = { EntrySuccess(1) };
	Provider.PostResults = {
		PostFailure(), PostSuccess(1),
		PostFailure(), PostSuccess(1),
		PostFailure(), PostSuccess(5),
		PostFailure(), PostSuccess(1) };
	FSkillRuleSnapshotSet Skills;
	FMatchPlayAuthoritativeSession Session(
		Provider, Provider, Provider, Provider, Skills);
	TestTrue(TEXT("Session initializes"),
		Session.InitializeMatch(MakeOpening(TEXT("CornerSessionE2E")))
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
	TestTrue(TEXT("Type D6 1 selects Corner"),
		Session.RequestSetPieceTypeRoll(Type).TypeRollResult.bSuccess);
	State = Session.GetStateSnapshot();
	TestTrue(TEXT("Corner production entry is AwaitingAttackerNominations"),
		State.CurrentAttack.SetPieceRoute.Corner.Stage
			== EMatchPlaySetPieceCornerRouteStage::AwaitingAttackerNominations);

	const auto AttackerLock = Session.SubmitCornerAttackerNominations(
		Nomination(State, Attacker, 2));
	TestTrue(TEXT("Typed attacker lock uses serialized Session"),
		AttackerLock.RuntimeEnvelope.bAccepted
			&& AttackerLock.ResolutionResult.bSuccess);
	State = Session.GetStateSnapshot();
	FAllProvider RebuildDefenderProvider;
	FMatchPlayAuthoritativeSession RebuiltDefender(
		State, RebuildDefenderProvider, RebuildDefenderProvider,
		RebuildDefenderProvider);
	TestTrue(TEXT("Fresh Session resumes AwaitingDefenderNominations"),
		RebuiltDefender.SubmitCornerDefenderNominations(
			Nomination(State, Defender, 2)).ResolutionResult.bSuccess);
	TestTrue(TEXT("Original Session accepts defender sealed list"),
		Session.SubmitCornerDefenderNominations(
			Nomination(State, Defender, 2)).ResolutionResult.bSuccess);
	State = Session.GetStateSnapshot();
	FAllProvider RebuildSelectionProvider;
	RebuildSelectionProvider.PostResults = { PostSuccess(1) };
	FMatchPlayAuthoritativeSession RebuiltSelection(
		State, RebuildSelectionProvider, RebuildSelectionProvider,
		RebuildSelectionProvider);
	TestTrue(TEXT("Fresh Session resumes AwaitingParticipantSelectionRoll"),
		RebuiltSelection.RequestCornerParticipantSelectionRoll(
			RollRequest(State, false)).ResolutionResult.bSuccess);

	FMatchPlayCornerRollRequest Selection = RollRequest(State, false);
	FMatchPlayCornerRollRequest WrongSelection = Selection;
	WrongSelection.RequestingSide = Defender;
	const int32 BeforeWrong = Provider.PostPurposes.Num();
	TestFalse(TEXT("Wrong-side shared D6 rejected"),
		Session.RequestCornerParticipantSelectionRoll(WrongSelection)
			.ResolutionResult.bSuccess);
	TestEqual(TEXT("Wrong-side selection consumes zero RNG"),
		Provider.PostPurposes.Num(), BeforeWrong);
	FMatchPlayCornerRollRequest StaleSelection = Selection;
	++StaleSelection.AttackSequence;
	TestFalse(TEXT("Stale shared D6 rejected"),
		Session.RequestCornerParticipantSelectionRoll(StaleSelection)
			.ResolutionResult.bSuccess);
	const FMatchPlayState BeforeSelectionFailure = Session.GetStateSnapshot();
	TestFalse(TEXT("Participant provider failure is retryable"),
		Session.RequestCornerParticipantSelectionRoll(Selection)
			.ResolutionResult.bSuccess);
	TestTrue(TEXT("Participant provider failure adopts no state"),
		Equal(BeforeSelectionFailure, Session.GetStateSnapshot()));
	TestTrue(TEXT("Participant retry succeeds"),
		Session.RequestCornerParticipantSelectionRoll(Selection)
			.ResolutionResult.bSuccess);
	State = Session.GetStateSnapshot();
	const FName Runner = State.CurrentAttack.SetPieceRoute.Corner.Runner.CardId;
	const FName Helper = State.CurrentAttack.SetPieceRoute.Corner.Helper.CardId;
	FAllProvider RebuildIntentProvider;
	FMatchPlayAuthoritativeSession RebuiltIntent(
		State, RebuildIntentProvider, RebuildIntentProvider,
		RebuildIntentProvider);
	FMatchPlayCornerIntentRequest Intent;
	Intent.RequestingSide = Attacker;
	Intent.AttackSequence = 1;
	Intent.IntendedRoute = EMatchPlayCornerRouteIntent::High;
	TestTrue(TEXT("Fresh Session resumes AwaitingIntent"),
		RebuiltIntent.SubmitCornerIntent(Intent).ResolutionResult.bSuccess);
	TestTrue(TEXT("Original Session accepts zero-RNG High intent"),
		Session.SubmitCornerIntent(Intent).ResolutionResult.bSuccess);
	State = Session.GetStateSnapshot();
	FAllProvider RebuildRouteProvider;
	RebuildRouteProvider.PostResults = { PostSuccess(1) };
	FMatchPlayAuthoritativeSession RebuiltRoute(
		State, RebuildRouteProvider, RebuildRouteProvider,
		RebuildRouteProvider);
	TestTrue(TEXT("Fresh Session resumes AwaitingRouteRoll"),
		RebuiltRoute.RequestCornerRouteRoll(RollRequest(State, false))
			.ResolutionResult.bSuccess);

	auto RetryRoll = [this, &Session, &Provider](
		const TCHAR* FailureLabel,
		const TCHAR* RetryLabel,
		auto Command)
	{
		const FMatchPlayState Before = Session.GetStateSnapshot();
		TestFalse(FailureLabel, Command().ResolutionResult.bSuccess);
		TestTrue(TEXT("Provider failure preserves Session state"),
			Equal(Before, Session.GetStateSnapshot()));
		TestTrue(RetryLabel, Command().ResolutionResult.bSuccess);
	};
	RetryRoll(TEXT("Route provider failure retryable"),
		TEXT("Route retry succeeds"),
		[&]() { return Session.RequestCornerRouteRoll(
			RollRequest(Session.GetStateSnapshot(), false)); });
	RetryRoll(TEXT("Attack provider failure retryable"),
		TEXT("Attack retry succeeds"),
		[&]() { return Session.RequestCornerAttackRoll(
			RollRequest(Session.GetStateSnapshot(), false)); });
	State = Session.GetStateSnapshot();
	FAllProvider RebuildDefenseProvider;
	RebuildDefenseProvider.PostResults = { PostSuccess(1) };
	FMatchPlayAuthoritativeSession RebuiltDefense(
		State, RebuildDefenseProvider, RebuildDefenseProvider,
		RebuildDefenseProvider);
	TestTrue(TEXT("Fresh Session resumes AwaitingDefenseRoll with AttackD6"),
		RebuiltDefense.RequestCornerDefenseRoll(RollRequest(State, true))
			.ResolutionResult.bSuccess);
	RetryRoll(TEXT("Defense provider failure retryable"),
		TEXT("Defense retry resolves Formula"),
		[&]() { return Session.RequestCornerDefenseRoll(
			RollRequest(Session.GetStateSnapshot(), true)); });
	const FMatchPlayState Terminal = Session.GetStateSnapshot();
	TestTrue(TEXT("Real Session persists High Goal, score, Runner scorer, terminal"),
		Terminal.CurrentAttack.LifecycleState
			== EMatchPlayCurrentAttackLifecycleState::TerminalPendingAdvance
			&& Terminal.CurrentAttack.SetPieceRoute.Corner.GameplayOutcome
				== EMatchPlayCornerGameplayOutcome::Goal
			&& Terminal.CurrentAttack.SetPieceRoute.Corner.GoalScorerCardId
				== Runner
			&& Terminal.RuntimeState.PlayerAState.Score
				+ Terminal.RuntimeState.PlayerBState.Score == 1);
	const int32 BeforeDuplicate = Provider.PostPurposes.Num();
	const auto Duplicate = Session.RequestCornerDefenseRoll(
		RollRequest(Terminal, true));
	TestTrue(TEXT("Terminal duplicate is rejected before RNG/rescore"),
		Duplicate.RuntimeEnvelope.RuntimeFailureCode
			== EMatchPlayAuthoritativeRuntimeFailureCode::TerminalAdvanceRequired
			&& Provider.PostPurposes.Num() == BeforeDuplicate);

	FAllProvider FailedAdvanceProvider;
	FailedAdvanceProvider.bFailRecovery = true;
	FMatchPlayAuthoritativeSession FailedAdvance(
		Terminal, FailedAdvanceProvider, FailedAdvanceProvider,
		FailedAdvanceProvider);
	FMatchPlayAuthoritativeAdvanceAfterTerminalRequest Advance;
	Advance.RequestingSide = Attacker;
	Advance.AttackSequence = 1;
	TestTrue(TEXT("Session Recovery failure rolls back entire Advance"),
		!FailedAdvance.AdvanceAfterTerminal(Advance).CompletionResult.bSuccess
			&& Equal(Terminal, FailedAdvance.GetStateSnapshot()));
	const auto Advanced = Session.AdvanceAfterTerminal(Advance);
	TestTrue(TEXT("Session Advance consumes Runner/Helper and invokes Recovery"),
		Advanced.CompletionResult.bSuccess
			&& Advanced.CompletionResult.SetPieceCardUsageResults.Num() == 2
			&& Provider.RecoveryCandidates.ContainsByPredicate(
				[Runner](const auto& C) { return C.CardId == Runner; })
			&& Provider.RecoveryCandidates.ContainsByPredicate(
				[Helper](const auto& C) { return C.CardId == Helper; })
			&& !Session.GetStateSnapshot().bHasCurrentAttack);
	FAllProvider TerminalProvider;
	FMatchPlayAuthoritativeSession RebuiltTerminal(
		Terminal, TerminalProvider, TerminalProvider, TerminalProvider);
	TestTrue(TEXT("Fresh Session resumes terminal without replaying Corner RNG"),
		RebuiltTerminal.AdvanceAfterTerminal(Advance).CompletionResult.bSuccess
			&& TerminalProvider.PostPurposes.IsEmpty());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayCornerAuthoritativeSessionAlternatePathsTest,
	"FMCodex.MatchPlayRuntime.AuthoritativeSession.CornerAlternatePathsAndFinal",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayCornerAuthoritativeSessionAlternatePathsTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayCornerAuthoritativeSessionTests;
	auto Bootstrap = [](const FString& Prefix)
	{
		FAllProvider Provider;
		return BootstrapAwaitingAttacker(Provider, Prefix);
	};

	FMatchPlayState RouteSwitchBase = Bootstrap(TEXT("CornerRouteSwitch"));
	FAllProvider RouteSwitchProvider;
	RouteSwitchProvider.PostResults = {
		PostSuccess(1), PostSuccess(6), PostSuccess(1), PostSuccess(6) };
	const FMatchPlayState RouteSwitch = CompleteCorner(
		RouteSwitchBase, RouteSwitchProvider, 1, 1,
		EMatchPlayCornerRouteIntent::High);
	TestTrue(TEXT("High intent route D6 6 switches to Low Formula"),
		RouteSwitch.CurrentAttack.SetPieceRoute.Corner.ActualRoute
			== EMatchPlayCornerRouteIntent::Low
			&& RouteSwitch.CurrentAttack.SetPieceRoute.Corner.GameplayOutcome
				== EMatchPlayCornerGameplayOutcome::NoGoal);

	FMatchPlayState LowBase = Bootstrap(TEXT("CornerLowMiss"));
	FAllProvider LowProvider;
	LowProvider.PostResults = {
		PostSuccess(1), PostSuccess(1), PostSuccess(1), PostSuccess(6) };
	const FMatchPlayState LowMiss = CompleteCorner(
		LowBase, LowProvider, 1, 1, EMatchPlayCornerRouteIntent::Low);
	TestTrue(TEXT("Real complete Low path persists NoGoal"),
		LowMiss.CurrentAttack.SetPieceRoute.Corner.ActualRoute
			== EMatchPlayCornerRouteIntent::Low
			&& LowMiss.CurrentAttack.SetPieceRoute.Corner.GameplayOutcome
				== EMatchPlayCornerGameplayOutcome::NoGoal);
	FAllProvider RebuildLowProvider;
	FMatchPlayAuthoritativeSession RebuiltLow(
		LowMiss, RebuildLowProvider, RebuildLowProvider, RebuildLowProvider);
	FMatchPlayAuthoritativeAdvanceAfterTerminalRequest RebuildLowAdvance;
	RebuildLowAdvance.RequestingSide =
		LowMiss.RuntimeState.CurrentAttackingPlayer;
	RebuildLowAdvance.AttackSequence = 1;
	TestTrue(TEXT("Fresh Session resumes Low NoGoal terminal"),
		RebuiltLow.AdvanceAfterTerminal(RebuildLowAdvance)
			.CompletionResult.bSuccess);

	for (const bool bAttackerZero : { true, false })
	{
		FMatchPlayState ShortageBase = Bootstrap(
			bAttackerZero ? TEXT("CornerSessionA0") : TEXT("CornerSessionD0"));
		const EInitialTurnOrderPlayer Attacker =
			ShortageBase.RuntimeState.CurrentAttackingPlayer;
		const EInitialTurnOrderPlayer Defender = Other(Attacker);
		FAllProvider ShortageProvider;
		ShortageProvider.PostResults = { PostSuccess(4) };
		FMatchPlayAuthoritativeSession Session(
			ShortageBase, ShortageProvider, ShortageProvider,
			ShortageProvider);
		Session.SubmitCornerAttackerNominations(Nomination(
			ShortageBase, Attacker, bAttackerZero ? 0 : 2));
		FMatchPlayState State = Session.GetStateSnapshot();
		Session.SubmitCornerDefenderNominations(Nomination(
			State, Defender, bAttackerZero ? 2 : 0));
		State = Session.GetStateSnapshot();
		TestTrue(TEXT("Real Session applies zero-attacker precedence or automatic scorer Goal"),
			ShortageProvider.PostPurposes.Num() == (bAttackerZero ? 0 : 1)
				&& State.CurrentAttack.SetPieceRoute.Corner.GameplayOutcome
					== (bAttackerZero
						? EMatchPlayCornerGameplayOutcome::NoGoal
						: EMatchPlayCornerGameplayOutcome::Goal)
				&& State.CurrentAttack.SetPieceRoute.Corner.bHasGoalScorer == !bAttackerZero);
		FAllProvider RebuildShortageProvider;
		FMatchPlayAuthoritativeSession RebuiltShortage(
			State, RebuildShortageProvider, RebuildShortageProvider,
			RebuildShortageProvider);
		FMatchPlayAuthoritativeAdvanceAfterTerminalRequest RebuildAdvance;
		RebuildAdvance.RequestingSide = Attacker;
		RebuildAdvance.AttackSequence = 1;
		TestTrue(TEXT("Fresh Session resumes zero-attacker/automatic Goal terminal"),
			RebuiltShortage.AdvanceAfterTerminal(RebuildAdvance)
				.CompletionResult.bSuccess);
		FMatchPlayAuthoritativeAdvanceAfterTerminalRequest Advance;
		Advance.RequestingSide = Attacker;
		Advance.AttackSequence = 1;
		const auto Advanced = Session.AdvanceAfterTerminal(Advance);
		TestTrue(TEXT("Zero-attacker consumes none; automatic Goal consumes its scorer"),
			Advanced.CompletionResult.bSuccess
				&& Advanced.CompletionResult.SetPieceCardUsageResults.Num() == (bAttackerZero ? 0 : 1));
	}

	FMatchPlayState FinalBase = Bootstrap(TEXT("CornerSessionFinal"));
	const EInitialTurnOrderPlayer FinalAttacker =
		FinalBase.RuntimeState.CurrentAttackingPlayer;
	FPlayerRuntimeState& ARuntime =
		FinalAttacker == EInitialTurnOrderPlayer::PlayerA
			? FinalBase.RuntimeState.PlayerAState
			: FinalBase.RuntimeState.PlayerBState;
	FPlayerRuntimeState& DRuntime =
		FinalAttacker == EInitialTurnOrderPlayer::PlayerA
			? FinalBase.RuntimeState.PlayerBState
			: FinalBase.RuntimeState.PlayerAState;
	ARuntime.TotalAttackCount = 1;
	ARuntime.UsedAttackCount = 0;
	DRuntime.TotalAttackCount = 1;
	DRuntime.UsedAttackCount = 1;
	FinalBase.CurrentAttack.AttackSequence = 2;
	FAllProvider FinalProvider;
	FinalProvider.PostResults = {
		PostSuccess(1), PostSuccess(1), PostSuccess(5), PostSuccess(1) };
	const FMatchPlayState FinalTerminal = CompleteCorner(
		FinalBase, FinalProvider, 1, 1, EMatchPlayCornerRouteIntent::High);
	FMatchPlayAuthoritativeSession FinalSession(
		FinalTerminal, FinalProvider, FinalProvider, FinalProvider);
	FMatchPlayAuthoritativeAdvanceAfterTerminalRequest FinalAdvance;
	FinalAdvance.RequestingSide = FinalAttacker;
	FinalAdvance.AttackSequence = 2;
	const auto Final = FinalSession.AdvanceAfterTerminal(FinalAdvance);
	TestTrue(TEXT("Real Session final Corner consumes two, resolves match, skips Recovery"),
		Final.CompletionResult.bSuccess && Final.CompletionResult.bMatchEnded
			&& Final.CompletionResult.SetPieceCardUsageResults.Num() == 2
			&& Final.CompletionResult.MatchResultResolveResult.bSuccess
			&& FinalProvider.RecoveryCalls == 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayCornerAutomaticScorerSessionTest,
	"FMCodex.MatchPlayRuntime.AuthoritativeSession.CornerAutomaticScorerCorrelationAndReplay",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayCornerAutomaticScorerSessionTest::RunTest(const FString& Parameters)
{
	using namespace MatchPlayCornerAuthoritativeSessionTests;
	for (const bool bFinal : { false, true })
	{
		FAllProvider Bootstrap;
		FMatchPlayState Seed = BootstrapAwaitingAttacker(Bootstrap, TEXT("AutomaticSession"));
		const auto Attacker = Seed.RuntimeState.CurrentAttackingPlayer;
		// Include one old Used card so the non-final two-card Recovery really calls its provider.
		const auto Defender = Other(Attacker);
		const FName OldUsed = FirstOutfield(Seed, Defender, 1)[0];
		auto& DefenderUsage = Defender == EInitialTurnOrderPlayer::PlayerA
			? Seed.CardUsageState.PlayerACardUsageState : Seed.CardUsageState.PlayerBCardUsageState;
		DefenderUsage.AvailableCardIds.Remove(OldUsed);
		DefenderUsage.UsedCardIds.Add(OldUsed);
		if (bFinal)
		{
			auto& A = Attacker == EInitialTurnOrderPlayer::PlayerA ? Seed.RuntimeState.PlayerAState : Seed.RuntimeState.PlayerBState;
			auto& D = Attacker == EInitialTurnOrderPlayer::PlayerA ? Seed.RuntimeState.PlayerBState : Seed.RuntimeState.PlayerAState;
			A.TotalAttackCount = 1; A.UsedAttackCount = 0;
			D.TotalAttackCount = 1; D.UsedAttackCount = 1;
			Seed.CurrentAttack.AttackSequence = 2;
		}
		FAllProvider Provider;
		Provider.PostResults = { PostFailure(), PostSuccess(7), PostSuccess(5) };
		FMatchPlayAuthoritativeSession Session(Seed, Provider, Provider, Provider);
		TestTrue(TEXT("Attacker nominates three"), Session.SubmitCornerAttackerNominations(
			Nomination(Seed, Attacker, 3)).ResolutionResult.bSuccess);
		const auto Locked = Session.GetStateSnapshot();
		const auto Request = Nomination(Locked, Other(Attacker), 0);
		auto Wrong = Request;
		Wrong.RequestingSide = Attacker;
		TestFalse(TEXT("Wrong-side lock rejected before hidden RNG"), Session.SubmitCornerDefenderNominations(Wrong).ResolutionResult.bSuccess);
		Wrong = Request;
		++Wrong.AttackSequence;
		TestFalse(TEXT("Stale lock rejected before hidden RNG"), Session.SubmitCornerDefenderNominations(Wrong).ResolutionResult.bSuccess);
		TestTrue(TEXT("Rejected correlation consumes zero RNG and preserves state"), Provider.PostPurposes.IsEmpty() && Equal(Locked, Session.GetStateSnapshot()));
		for (int32 Failure = 0; Failure < 2; ++Failure)
			TestTrue(TEXT("Backend failure/malformed D6 adopts no lock, scorer, score or terminal"),
				!Session.SubmitCornerDefenderNominations(Request).ResolutionResult.bSuccess && Equal(Locked, Session.GetStateSnapshot()));
		TestTrue(TEXT("Retry accepts one atomic automatic Goal"), Session.SubmitCornerDefenderNominations(Request).ResolutionResult.bSuccess);
		const auto Terminal = Session.GetStateSnapshot();
		const auto& Corner = Terminal.CurrentAttack.SetPieceRoute.Corner;
		TestTrue(TEXT("Snapshot persists typed hidden draw and third actual scorer"),
			Corner.AutomaticScorerD6 == 5 && Corner.GoalScorerCardId == Corner.AttackerNominees[2].CardId
				&& Provider.PostPurposes.Num() == 3 && Provider.PostPurposes.Last() == EMatchPlayCurrentAttackPostRouteRollPurpose::CornerAutomaticScorer);
		TestTrue(TEXT("Duplicate terminal command never redraws or rescores"),
			!Session.SubmitCornerDefenderNominations(Request).ResolutionResult.bSuccess
				&& Provider.PostPurposes.Num() == 3 && Equal(Terminal, Session.GetStateSnapshot()));
		FAllProvider RebuiltProvider;
		FMatchPlayAuthoritativeSession Rebuilt(Terminal, RebuiltProvider, RebuiltProvider, RebuiltProvider);
		FMatchPlayAuthoritativeAdvanceAfterTerminalRequest Advance;
		Advance.RequestingSide = Attacker;
		Advance.AttackSequence = Terminal.CurrentAttack.AttackSequence;
		if (!bFinal)
		{
			RebuiltProvider.bFailRecovery = true;
			TestTrue(TEXT("Automatic scorer Recovery failure rolls back Advance"),
				!Rebuilt.AdvanceAfterTerminal(Advance).CompletionResult.bSuccess && Equal(Terminal, Rebuilt.GetStateSnapshot()));
			RebuiltProvider.bFailRecovery = false;
		}
		const auto Completed = Rebuilt.AdvanceAfterTerminal(Advance).CompletionResult;
		TestTrue(TEXT("Reconstructed terminal consumes exactly scorer without replaying selection"),
			Completed.bSuccess && Completed.SetPieceCardUsageResults.Num() == 1
				&& Completed.bMatchEnded == bFinal && RebuiltProvider.PostPurposes.IsEmpty());
		TestTrue(TEXT("Final skips Recovery; normal Advance offers scorer to shared Recovery"), bFinal
			? RebuiltProvider.RecoveryCalls == 0
			: RebuiltProvider.RecoveryCandidates.ContainsByPredicate([&](const auto& C) { return C.CardId == Corner.GoalScorerCardId; }));
		if (bFinal) TestTrue(TEXT("Final Advance leaves actual scorer Used, other nominees Available"),
			Usage(Rebuilt.GetStateSnapshot(), Attacker).UsedCardIds.Contains(Corner.GoalScorerCardId)
				&& Usage(Rebuilt.GetStateSnapshot(), Attacker).AvailableCardIds.Contains(Corner.AttackerNominees[0].CardId));
	}
	return true;
}

#endif
