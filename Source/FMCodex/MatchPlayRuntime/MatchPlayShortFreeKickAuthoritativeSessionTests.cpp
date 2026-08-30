#include "MatchPlayAuthoritativeSession.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

namespace MatchPlayShortFreeKickAuthoritativeSessionTests
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

	FName FindOutfield(
		const FMatchPlayState& State,
		const EInitialTurnOrderPlayer Side)
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
		Result.ErrorMessage = TEXT("Injected Session Short FK failure.");
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
			const EMatchPlayAttackEntryRollPurpose Purpose) override
		{
			++D12Calls;
			return D12Results.IsValidIndex(NextD12)
				? D12Results[NextD12++] : FMatchPlayAttackEntryRollProviderResult();
		}
		virtual FMatchPlayAttackEntryRollProviderResult RollD6(
			const EMatchPlayAttackEntryRollPurpose Purpose) override
		{
			++EntryD6Calls;
			return EntryD6Results.IsValidIndex(NextEntryD6)
				? EntryD6Results[NextEntryD6++]
				: FMatchPlayAttackEntryRollProviderResult();
		}
		virtual FMatchPlayAttackEntrySelectionProviderResult SelectUniformIndex(
			EMatchPlayAttackEntryRollPurpose Purpose,
			int32 CandidateCount) override
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
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayShortFreeKickAuthoritativeSessionEndToEndTest,
	"FMCodex.MatchPlayRuntime.AuthoritativeSession.ShortFreeKickEndToEnd",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayShortFreeKickAuthoritativeSessionEndToEndTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayShortFreeKickAuthoritativeSessionTests;
	FAllProvider Provider;
	Provider.D12Results = { EntrySuccess(9) };
	Provider.EntryD6Results = { EntrySuccess(5) };
	Provider.PostResults = { PostFailure(), PostSuccess(6), PostSuccess(1) };
	FSkillRuleSnapshotSet Skills;
	FMatchPlayAuthoritativeSession Session(
		Provider, Provider, Provider, Provider, Skills);
	const auto Initialized = Session.InitializeMatch(MakeOpening(TEXT("SFK_Session")));
	TestTrue(TEXT("Session initializes"), Initialized.OpeningResult.bSuccess);
	FMatchPlayState State = Session.GetStateSnapshot();
	const EInitialTurnOrderPlayer Attacker =
		State.RuntimeState.CurrentAttackingPlayer;
	const EInitialTurnOrderPlayer Defender = Other(Attacker);
	FMatchPlayFullD12EntryRequest EntryRequest;
	EntryRequest.RequestingSide = Attacker;
	EntryRequest.ExpectedAttackSequence = 1;
	TestTrue(TEXT("Full D12 enters Set Piece"),
		Session.RequestInitialActionPointRoll(EntryRequest).EntryResult.bSuccess);
	State = Session.GetStateSnapshot();
	FMatchPlaySetPieceTypeRollRequest TypeRequest;
	TypeRequest.RequestingSide = Attacker;
	TypeRequest.AttackSequence = State.CurrentAttack.AttackSequence;
	TestTrue(TEXT("Type D6 selects Short Free Kick"),
		Session.RequestSetPieceTypeRoll(TypeRequest).TypeRollResult.bSuccess);
	State = Session.GetStateSnapshot();
	const FName Carrier = FindOutfield(State, Attacker);
	FMatchPlaySetPieceCarrierSelectionRequest CarrierRequest;
	CarrierRequest.RequestingSide = Attacker;
	CarrierRequest.AttackSequence = State.CurrentAttack.AttackSequence;
	CarrierRequest.CardId = Carrier;
	TestTrue(TEXT("Session Carrier selection succeeds"),
		Session.SubmitSetPieceCarrier(CarrierRequest).CarrierResult.bSuccess);
	State = Session.GetStateSnapshot();

	FMatchPlayShortFreeKickMethodRequest Method;
	Method.RequestingSide = Attacker;
	Method.AttackSequence = State.CurrentAttack.AttackSequence;
	Method.Method = EMatchPlayShortFreeKickMethod::Direct;
	FMatchPlayShortFreeKickMethodRequest WrongMethodSide = Method;
	WrongMethodSide.RequestingSide = Defender;
	const int32 CallsBeforeWrong = Provider.PostPurposes.Num();
	TestFalse(TEXT("Wrong-side method rejected"),
		Session.SubmitShortFreeKickMethod(WrongMethodSide)
			.ResolutionResult.bSuccess);
	TestEqual(TEXT("Wrong-side method consumes zero post-route RNG"),
		Provider.PostPurposes.Num(), CallsBeforeWrong);
	TestTrue(TEXT("Direct method accepted"),
		Session.SubmitShortFreeKickMethod(Method).ResolutionResult.bSuccess);
	State = Session.GetStateSnapshot();

	FMatchPlayShortFreeKickRollRequest AttackRoll;
	AttackRoll.RequestingSide = Attacker;
	AttackRoll.AttackSequence = State.CurrentAttack.AttackSequence;
	FMatchPlayShortFreeKickRollRequest Stale = AttackRoll;
	++Stale.AttackSequence;
	const int32 CallsBeforeStale = Provider.PostPurposes.Num();
	TestFalse(TEXT("Stale attack roll rejected"),
		Session.ResolveShortFreeKickDirectAttackRoll(Stale)
			.ResolutionResult.bSuccess);
	TestEqual(TEXT("Stale request consumes zero provider calls"),
		Provider.PostPurposes.Num(), CallsBeforeStale);
	const FMatchPlayState BeforeFailure = Session.GetStateSnapshot();
	TestFalse(TEXT("Injected attack provider failure is retryable"),
		Session.ResolveShortFreeKickDirectAttackRoll(AttackRoll)
			.ResolutionResult.bSuccess);
	TestTrue(TEXT("Session failure does not adopt mutation"),
		Equal(BeforeFailure, Session.GetStateSnapshot()));
	TestTrue(TEXT("Attack retry succeeds"),
		Session.ResolveShortFreeKickDirectAttackRoll(AttackRoll)
			.ResolutionResult.bSuccess);
	const FMatchPlayState MidDirect = Session.GetStateSnapshot();
	TestEqual(TEXT("Mid-Direct stores only attack D6"),
		MidDirect.CurrentAttack.SetPieceRoute.ShortFreeKick.Stage,
		EMatchPlaySetPieceCarrierRouteStage::DirectAwaitingDefenseRoll);

	FAllProvider RebuildProvider;
	RebuildProvider.PostResults = { PostSuccess(1) };
	FMatchPlayAuthoritativeSession RebuiltMid(
		MidDirect, RebuildProvider, RebuildProvider, RebuildProvider);
	FMatchPlayShortFreeKickRollRequest DefenseRoll;
	DefenseRoll.RequestingSide = Defender;
	DefenseRoll.AttackSequence = MidDirect.CurrentAttack.AttackSequence;
	TestTrue(TEXT("Fresh Session reconstructs and completes mid-Direct"),
		RebuiltMid.ResolveShortFreeKickDirectDefenseRoll(DefenseRoll)
			.ResolutionResult.bSuccess);

	TestTrue(TEXT("Original Session defense resolves terminal Goal"),
		Session.ResolveShortFreeKickDirectDefenseRoll(DefenseRoll)
			.ResolutionResult.bSuccess);
	const FMatchPlayState Terminal = Session.GetStateSnapshot();
	TestTrue(TEXT("Terminal Goal persists scorer and score"),
		Terminal.CurrentAttack.LifecycleState
			== EMatchPlayCurrentAttackLifecycleState::TerminalPendingAdvance
			&& Terminal.CurrentAttack.SetPieceRoute.ShortFreeKick
				.GoalScorerCardId == Carrier
			&& Terminal.RuntimeState.PlayerAState.Score
				+ Terminal.RuntimeState.PlayerBState.Score == 1);
	const int32 CallsBeforeDuplicate = Provider.PostPurposes.Num();
	const auto Duplicate = Session.ResolveShortFreeKickDirectDefenseRoll(DefenseRoll);
	TestEqual(TEXT("Terminal duplicate rejected by Session gate"),
		Duplicate.RuntimeEnvelope.RuntimeFailureCode,
		EMatchPlayAuthoritativeRuntimeFailureCode::TerminalAdvanceRequired);
	TestEqual(TEXT("Terminal duplicate consumes zero RNG"),
		Provider.PostPurposes.Num(), CallsBeforeDuplicate);

	FMatchPlayAuthoritativeAdvanceAfterTerminalRequest Advance;
	Advance.RequestingSide = Attacker;
	Advance.AttackSequence = Terminal.CurrentAttack.AttackSequence;
	const auto Advanced = Session.AdvanceAfterTerminal(Advance);
	TestTrue(TEXT("Shared Session Advance succeeds"),
		Advanced.CompletionResult.bSuccess);
	TestEqual(TEXT("Shared Advance consumes one Set Piece participant"),
		Advanced.CompletionResult.SetPieceCardUsageResults.Num(), 1);
	TestTrue(TEXT("Recovery saw newly Used Carrier"),
		Advanced.CompletionResult.RecoveryResolveResult.CandidateQueryResult
			.Candidates.ContainsByPredicate(
			[Carrier](const FMatchPlayRecoveryCandidate& Candidate)
			{ return Candidate.CardId == Carrier; }));

	FAllProvider TerminalProvider;
	FMatchPlayAuthoritativeSession RebuiltTerminal(
		Terminal, TerminalProvider, TerminalProvider, TerminalProvider);
	TestTrue(TEXT("Fresh Session reconstructs terminal and advances"),
		RebuiltTerminal.AdvanceAfterTerminal(Advance)
			.CompletionResult.bSuccess);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayShortFreeKickAuthoritativeSessionAngledAndNoCarrierTest,
	"FMCodex.MatchPlayRuntime.AuthoritativeSession.ShortFreeKickAngledAndNoCarrier",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayShortFreeKickAuthoritativeSessionAngledAndNoCarrierTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayShortFreeKickAuthoritativeSessionTests;
	FAllProvider Bootstrap;
	Bootstrap.D12Results = { EntrySuccess(9) };
	Bootstrap.EntryD6Results = { EntrySuccess(5) };
	FSkillRuleSnapshotSet Skills;
	FMatchPlayAuthoritativeSession BootstrapSession(
		Bootstrap, Bootstrap, Bootstrap, Bootstrap, Skills);
	BootstrapSession.InitializeMatch(MakeOpening(TEXT("SFK_SessionAlt")));
	FMatchPlayState State = BootstrapSession.GetStateSnapshot();
	const EInitialTurnOrderPlayer Attacker =
		State.RuntimeState.CurrentAttackingPlayer;
	FMatchPlayFullD12EntryRequest Entry;
	Entry.RequestingSide = Attacker;
	Entry.ExpectedAttackSequence = 1;
	BootstrapSession.RequestInitialActionPointRoll(Entry);
	State = BootstrapSession.GetStateSnapshot();
	FMatchPlaySetPieceTypeRollRequest Type;
	Type.RequestingSide = Attacker;
	Type.AttackSequence = 1;
	BootstrapSession.RequestSetPieceTypeRoll(Type);
	const FMatchPlayState AwaitingCarrier = BootstrapSession.GetStateSnapshot();

	FMatchPlaySetPieceCarrierSelectionRequest Carrier;
	Carrier.RequestingSide = Attacker;
	Carrier.AttackSequence = 1;
	Carrier.CardId = FindOutfield(AwaitingCarrier, Attacker);
	BootstrapSession.SubmitSetPieceCarrier(Carrier);
	const FMatchPlayState AwaitingMethod = BootstrapSession.GetStateSnapshot();
	FAllProvider AngledProvider;
	AngledProvider.PostResults = { PostSuccess(4), PostSuccess(5) };
	FMatchPlayAuthoritativeSession AngledSession(
		AwaitingMethod, AngledProvider, AngledProvider, AngledProvider);
	FMatchPlayShortFreeKickMethodRequest Method;
	Method.RequestingSide = Attacker;
	Method.AttackSequence = 1;
	Method.Method = EMatchPlayShortFreeKickMethod::Angled;
	TestTrue(TEXT("Reconstructed Session accepts Angled method"),
		AngledSession.SubmitShortFreeKickMethod(Method)
			.ResolutionResult.bSuccess);
	FMatchPlayShortFreeKickRollRequest Roll;
	Roll.RequestingSide = Attacker;
	Roll.AttackSequence = 1;
	TestTrue(TEXT("Session Angled pair resolves Goal"),
		AngledSession.ResolveShortFreeKickAngledRoll(Roll)
			.ResolutionResult.bSuccess);
	TestEqual(TEXT("Angled Session uses two explicit pair calls"),
		AngledProvider.PostPurposes.Num(), 2);

	FMatchPlayState NoCarrier = AwaitingCarrier;
	FCardUsageState& AttackerUsage = Usage(NoCarrier, Attacker);
	for (const FPlayerCardRuleSnapshot& Snapshot : Snapshots(NoCarrier, Attacker).Cards)
	{
		if (!Snapshot.bIsGoalkeeper)
		{
			AttackerUsage.AvailableCardIds.Remove(Snapshot.CardId);
			AttackerUsage.UsedCardIds.Add(Snapshot.CardId);
		}
	}
	FAllProvider NoCarrierProvider;
	FMatchPlayAuthoritativeSession NoCarrierSession(
		NoCarrier, NoCarrierProvider, NoCarrierProvider, NoCarrierProvider);
	FMatchPlayShortFreeKickNoLegalCarrierRequest NoCarrierRequest;
	NoCarrierRequest.RequestingSide = Attacker;
	NoCarrierRequest.AttackSequence = 1;
	TestTrue(TEXT("Session authority verifies and resolves no legal Carrier"),
		NoCarrierSession.ResolveNoLegalSetPieceCarrier(NoCarrierRequest)
			.ResolutionResult.bSuccess);
	TestEqual(TEXT("No-Carrier command consumes zero post-route RNG"),
		NoCarrierProvider.PostPurposes.Num(), 0);
	return true;
}

#endif
