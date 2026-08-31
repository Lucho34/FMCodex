#include "MatchPlayAuthoritativeSession.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

namespace MatchPlayLongFreeKickAuthoritativeSessionTests
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

	const FCardUsageState& Usage(
		const FMatchCardUsageState& State,
		const EInitialTurnOrderPlayer Side)
	{
		return Side == EInitialTurnOrderPlayer::PlayerA
			? State.PlayerACardUsageState
			: State.PlayerBCardUsageState;
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
		Result.ErrorMessage = TEXT("Injected Session Long FK failure.");
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
			return D12Results.IsValidIndex(NextD12)
				? D12Results[NextD12++] : FMatchPlayAttackEntryRollProviderResult();
		}
		virtual FMatchPlayAttackEntryRollProviderResult RollD6(
			EMatchPlayAttackEntryRollPurpose Purpose) override
		{
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
		int32 RecoveryCalls = 0;
	};

	bool Equal(const FMatchPlayState& Left, const FMatchPlayState& Right)
	{
		return FMatchPlayState::StaticStruct()->CompareScriptStruct(
			&Left, &Right, 0);
	}

	FMatchPlayState BootstrapLong(
		const FString& Prefix,
		const bool bBindCarrier)
	{
		FAllProvider Provider;
		Provider.D12Results = { EntrySuccess(9) };
		Provider.EntryD6Results = { EntrySuccess(3) };
		FSkillRuleSnapshotSet Skills;
		FMatchPlayAuthoritativeSession Session(
			Provider, Provider, Provider, Provider, Skills);
		Session.InitializeMatch(MakeOpening(Prefix));
		FMatchPlayState State = Session.GetStateSnapshot();
		const EInitialTurnOrderPlayer Attacker =
			State.RuntimeState.CurrentAttackingPlayer;
		FMatchPlayFullD12EntryRequest Entry;
		Entry.RequestingSide = Attacker;
		Entry.ExpectedAttackSequence = 1;
		Session.RequestInitialActionPointRoll(Entry);
		State = Session.GetStateSnapshot();
		FMatchPlaySetPieceTypeRollRequest Type;
		Type.RequestingSide = Attacker;
		Type.AttackSequence = 1;
		Session.RequestSetPieceTypeRoll(Type);
		State = Session.GetStateSnapshot();
		if (bBindCarrier)
		{
			FMatchPlaySetPieceCarrierSelectionRequest Carrier;
			Carrier.RequestingSide = Attacker;
			Carrier.AttackSequence = 1;
			Carrier.CardId = FindOutfield(State, Attacker);
			Session.SubmitSetPieceCarrier(Carrier);
			State = Session.GetStateSnapshot();
		}
		return State;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayLongFreeKickAuthoritativeSessionDirectEndToEndTest,
	"FMCodex.MatchPlayRuntime.AuthoritativeSession.LongFreeKickDirectEndToEnd",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayLongFreeKickAuthoritativeSessionDirectEndToEndTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayLongFreeKickAuthoritativeSessionTests;
	FAllProvider Provider;
	Provider.D12Results = { EntrySuccess(9) };
	Provider.EntryD6Results = { EntrySuccess(3) };
	Provider.PostResults = { PostFailure(), PostSuccess(6), PostSuccess(1) };
	FSkillRuleSnapshotSet Skills;
	FMatchPlayAuthoritativeSession Session(
		Provider, Provider, Provider, Provider, Skills);
	TestTrue(TEXT("Session initializes"),
		Session.InitializeMatch(MakeOpening(TEXT("LFK_Session")))
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
	TestTrue(TEXT("Type D6 selects Long Free Kick"),
		Session.RequestSetPieceTypeRoll(Type).TypeRollResult.bSuccess);
	State = Session.GetStateSnapshot();
	TestEqual(TEXT("Selected type is Long Free Kick"),
		State.CurrentAttack.SetPieceRoute.SelectedType,
		ESetPieceSelectedType::LongFreeKick);
	const FName CarrierCardId = FindOutfield(State, Attacker);
	FMatchPlaySetPieceCarrierSelectionRequest Carrier;
	Carrier.RequestingSide = Attacker;
	Carrier.AttackSequence = 1;
	Carrier.CardId = CarrierCardId;
	TestTrue(TEXT("Carrier selection succeeds"),
		Session.SubmitSetPieceCarrier(Carrier).CarrierResult.bSuccess);

	FMatchPlayLongFreeKickMethodRequest Method;
	Method.RequestingSide = Attacker;
	Method.AttackSequence = 1;
	Method.Method = EMatchPlayLongFreeKickMethod::Direct;
	FMatchPlayLongFreeKickMethodRequest WrongMethodSide = Method;
	WrongMethodSide.RequestingSide = Defender;
	TestFalse(TEXT("Wrong-side method rejected"),
		Session.SubmitLongFreeKickMethod(WrongMethodSide)
			.ResolutionResult.bSuccess);
	TestTrue(TEXT("Direct method accepted"),
		Session.SubmitLongFreeKickMethod(Method).ResolutionResult.bSuccess);

	FMatchPlayLongFreeKickRollRequest AttackRoll;
	AttackRoll.RequestingSide = Attacker;
	AttackRoll.AttackSequence = 1;
	FMatchPlayLongFreeKickRollRequest Stale = AttackRoll;
	++Stale.AttackSequence;
	const int32 CallsBeforeStale = Provider.PostPurposes.Num();
	TestFalse(TEXT("Stale attack roll rejected"),
		Session.ResolveLongFreeKickDirectAttackRoll(Stale)
			.ResolutionResult.bSuccess);
	TestEqual(TEXT("Stale request consumes zero RNG"),
		Provider.PostPurposes.Num(), CallsBeforeStale);
	const FMatchPlayState BeforeFailure = Session.GetStateSnapshot();
	TestFalse(TEXT("Provider failure remains retryable"),
		Session.ResolveLongFreeKickDirectAttackRoll(AttackRoll)
			.ResolutionResult.bSuccess);
	TestTrue(TEXT("Failed command adopts no state"),
		Equal(BeforeFailure, Session.GetStateSnapshot()));
	TestTrue(TEXT("Attack retry succeeds"),
		Session.ResolveLongFreeKickDirectAttackRoll(AttackRoll)
			.ResolutionResult.bSuccess);
	const FMatchPlayState MidDirect = Session.GetStateSnapshot();
	const auto& MidLong = MidDirect.CurrentAttack.SetPieceRoute.LongFreeKick;
	TestTrue(TEXT("Attack 3-6 stores reconstructable prefix only"),
		MidLong.Stage
			== EMatchPlaySetPieceCarrierRouteStage::DirectAwaitingDefenseRoll
		&& MidLong.bHasAttackD6 && MidLong.AttackD6 == 6
		&& !MidLong.bHasDefenseD6 && !MidLong.bHasFormulaResolution);
	TestTrue(TEXT("Carrier remains Available before Advance"),
		Usage(MidDirect.CardUsageState, Attacker)
			.AvailableCardIds.Contains(CarrierCardId));

	FMatchPlayLongFreeKickRollRequest DefenseRoll;
	DefenseRoll.RequestingSide = Defender;
	DefenseRoll.AttackSequence = 1;
	FAllProvider RebuildProvider;
	RebuildProvider.PostResults = { PostSuccess(1) };
	FMatchPlayAuthoritativeSession RebuiltMid(
		MidDirect, RebuildProvider, RebuildProvider, RebuildProvider);
	TestTrue(TEXT("Fresh Session reconstructs mid-Direct"),
		RebuiltMid.ResolveLongFreeKickDirectDefenseRoll(DefenseRoll)
			.ResolutionResult.bSuccess);
	TestTrue(TEXT("Original Session resolves terminal Goal"),
		Session.ResolveLongFreeKickDirectDefenseRoll(DefenseRoll)
			.ResolutionResult.bSuccess);
	const FMatchPlayState Terminal = Session.GetStateSnapshot();
	const auto& TerminalLong = Terminal.CurrentAttack.SetPieceRoute.LongFreeKick;
	TestTrue(TEXT("Terminal persists score/scorer/formula"),
		Terminal.CurrentAttack.LifecycleState
			== EMatchPlayCurrentAttackLifecycleState::TerminalPendingAdvance
		&& TerminalLong.GameplayOutcome
			== EMatchPlayLongFreeKickGameplayOutcome::Goal
		&& TerminalLong.GoalScorerCardId == CarrierCardId
		&& TerminalLong.FormulaResolution.AttackerFinalValue == 12.0f
		&& TerminalLong.FormulaResolution.DefenderFinalValue == 7.0f
		&& Terminal.RuntimeState.PlayerAState.Score
			+ Terminal.RuntimeState.PlayerBState.Score == 1);
	TestTrue(TEXT("Carrier remains Available at terminal"),
		Usage(Terminal.CardUsageState, Attacker)
			.AvailableCardIds.Contains(CarrierCardId));
	const int32 CallsBeforeDuplicate = Provider.PostPurposes.Num();
	const auto Duplicate =
		Session.ResolveLongFreeKickDirectDefenseRoll(DefenseRoll);
	TestEqual(TEXT("Duplicate terminal command rejected by Session"),
		Duplicate.RuntimeEnvelope.RuntimeFailureCode,
		EMatchPlayAuthoritativeRuntimeFailureCode::TerminalAdvanceRequired);
	TestEqual(TEXT("Duplicate consumes zero RNG"),
		Provider.PostPurposes.Num(), CallsBeforeDuplicate);

	FMatchPlayAuthoritativeAdvanceAfterTerminalRequest Advance;
	Advance.RequestingSide = Attacker;
	Advance.AttackSequence = 1;
	const auto Advanced = Session.AdvanceAfterTerminal(Advance);
	TestTrue(TEXT("Shared Advance succeeds"),
		Advanced.CompletionResult.bSuccess);
	TestEqual(TEXT("Exactly one Set Piece participant is consumed"),
		Advanced.CompletionResult.SetPieceCardUsageResults.Num(), 1);
	if (Advanced.CompletionResult.SetPieceCardUsageResults.Num() == 1)
	{
		TestTrue(TEXT("Carrier is Used before Recovery"),
			Usage(Advanced.CompletionResult.SetPieceCardUsageResults[0]
				.UpdatedMatchCardUsageState, Attacker)
				.UsedCardIds.Contains(CarrierCardId));
	}
	TestTrue(TEXT("Recovery immediately sees newly Used Carrier"),
		Advanced.CompletionResult.RecoveryResolveResult.CandidateQueryResult
			.Candidates.ContainsByPredicate(
				[CarrierCardId](const FMatchPlayRecoveryCandidate& Candidate)
				{ return Candidate.CardId == CarrierCardId; }));

	FAllProvider TerminalProvider;
	FMatchPlayAuthoritativeSession RebuiltTerminal(
		Terminal, TerminalProvider, TerminalProvider, TerminalProvider);
	TestTrue(TEXT("Fresh Session reconstructs terminal and advances"),
		RebuiltTerminal.AdvanceAfterTerminal(Advance)
			.CompletionResult.bSuccess);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayLongFreeKickAuthoritativeSessionPathsTest,
	"FMCodex.MatchPlayRuntime.AuthoritativeSession.LongFreeKickConditionalAndPower",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayLongFreeKickAuthoritativeSessionPathsTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayLongFreeKickAuthoritativeSessionTests;
	const FMatchPlayState AwaitingMethod =
		BootstrapLong(TEXT("LFK_Paths"), true);
	const EInitialTurnOrderPlayer Attacker =
		AwaitingMethod.RuntimeState.CurrentAttackingPlayer;

	FAllProvider MissProvider;
	MissProvider.PostResults = { PostSuccess(2) };
	FMatchPlayAuthoritativeSession MissSession(
		AwaitingMethod, MissProvider, MissProvider, MissProvider);
	FMatchPlayLongFreeKickMethodRequest DirectMethod;
	DirectMethod.RequestingSide = Attacker;
	DirectMethod.AttackSequence = 1;
	DirectMethod.Method = EMatchPlayLongFreeKickMethod::Direct;
	TestTrue(TEXT("Direct method accepted"),
		MissSession.SubmitLongFreeKickMethod(DirectMethod)
			.ResolutionResult.bSuccess);
	FMatchPlayLongFreeKickRollRequest AttackRoll;
	AttackRoll.RequestingSide = Attacker;
	AttackRoll.AttackSequence = 1;
	TestTrue(TEXT("Attack D6 1-2 resolves immediate NoGoal"),
		MissSession.ResolveLongFreeKickDirectAttackRoll(AttackRoll)
			.ResolutionResult.bSuccess);
	const auto& Miss = MissSession.GetStateSnapshot()
		.CurrentAttack.SetPieceRoute.LongFreeKick;
	TestTrue(TEXT("Immediate miss has no defender input or Formula"),
		Miss.Stage == EMatchPlaySetPieceCarrierRouteStage::Terminal
		&& Miss.AttackD6 == 2 && !Miss.bHasDefenseD6
		&& !Miss.bHasFormulaResolution
		&& Miss.GameplayOutcome
			== EMatchPlayLongFreeKickGameplayOutcome::NoGoal);
	TestTrue(TEXT("Immediate miss consumes only attack purpose"),
		MissProvider.PostPurposes == TArray<
			EMatchPlayCurrentAttackPostRouteRollPurpose>{
				EMatchPlayCurrentAttackPostRouteRollPurpose
					::LongFreeKickDirectAttack });
	FMatchPlayAuthoritativeAdvanceAfterTerminalRequest MissAdvance;
	MissAdvance.RequestingSide = Attacker;
	MissAdvance.AttackSequence = 1;
	TestTrue(TEXT("Immediate miss follows shared Session Advance"),
		MissSession.AdvanceAfterTerminal(MissAdvance)
			.CompletionResult.bSuccess);

	FAllProvider PowerProvider;
	PowerProvider.PostResults = {
		PostSuccess(6), PostFailure(), PostSuccess(6), PostSuccess(5) };
	FMatchPlayAuthoritativeSession PowerSession(
		AwaitingMethod, PowerProvider, PowerProvider, PowerProvider);
	FMatchPlayLongFreeKickMethodRequest PowerMethod = DirectMethod;
	PowerMethod.Method = EMatchPlayLongFreeKickMethod::Power;
	TestTrue(TEXT("Power method accepted"),
		PowerSession.SubmitLongFreeKickMethod(PowerMethod)
			.ResolutionResult.bSuccess);
	const FMatchPlayState BeforePowerFailure = PowerSession.GetStateSnapshot();
	TestFalse(TEXT("Second Power provider failure rejects atomically"),
		PowerSession.ResolveLongFreeKickPowerRoll(AttackRoll)
			.ResolutionResult.bSuccess);
	TestTrue(TEXT("Failed Power pair adopts no partial roll"),
		Equal(BeforePowerFailure, PowerSession.GetStateSnapshot()));
	TestTrue(TEXT("Power retry re-requests both ordered purposes"),
		PowerSession.ResolveLongFreeKickPowerRoll(AttackRoll)
			.ResolutionResult.bSuccess
		&& PowerProvider.PostPurposes.Num() == 4
		&& PowerProvider.PostPurposes[2]
			== EMatchPlayCurrentAttackPostRouteRollPurpose::LongFreeKickPowerA
		&& PowerProvider.PostPurposes[3]
			== EMatchPlayCurrentAttackPostRouteRollPurpose::LongFreeKickPowerB);
	const auto& Power = PowerSession.GetStateSnapshot()
		.CurrentAttack.SetPieceRoute.LongFreeKick;
	TestTrue(TEXT("Power 11 is Goal without defense or Formula"),
		Power.bHasPowerD6Pair && Power.PowerD6A == 6 && Power.PowerD6B == 5
		&& !Power.bHasDefenseD6 && !Power.bHasFormulaResolution
		&& Power.GameplayOutcome
			== EMatchPlayLongFreeKickGameplayOutcome::Goal);
	FMatchPlayAuthoritativeAdvanceAfterTerminalRequest PowerAdvance;
	PowerAdvance.RequestingSide = Attacker;
	PowerAdvance.AttackSequence = 1;
	TestTrue(TEXT("Power terminal follows shared Session Advance"),
		PowerSession.AdvanceAfterTerminal(PowerAdvance)
			.CompletionResult.bSuccess);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayLongFreeKickAuthoritativeSessionNoCarrierTest,
	"FMCodex.MatchPlayRuntime.AuthoritativeSession.LongFreeKickNoLegalCarrier",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayLongFreeKickAuthoritativeSessionNoCarrierTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayLongFreeKickAuthoritativeSessionTests;
	FMatchPlayState State = BootstrapLong(TEXT("LFK_NoCarrier"), false);
	const EInitialTurnOrderPlayer Attacker =
		State.RuntimeState.CurrentAttackingPlayer;
	FCardUsageState& AttackerUsage = Usage(State, Attacker);
	for (const FPlayerCardRuleSnapshot& Snapshot : Snapshots(State, Attacker).Cards)
	{
		if (!Snapshot.bIsGoalkeeper)
		{
			AttackerUsage.AvailableCardIds.Remove(Snapshot.CardId);
			AttackerUsage.UsedCardIds.Add(Snapshot.CardId);
		}
	}
	FAllProvider Provider;
	FMatchPlayAuthoritativeSession Session(
		State, Provider, Provider, Provider);
	FMatchPlayLongFreeKickNoLegalCarrierRequest Request;
	Request.RequestingSide = Attacker;
	Request.AttackSequence = 1;
	TestTrue(TEXT("Common authority proves no legal Carrier"),
		Session.ResolveNoLegalSetPieceCarrier(Request)
			.ResolutionResult.bSuccess);
	TestEqual(TEXT("No-Carrier resolution consumes zero RNG"),
		Provider.PostPurposes.Num(), 0);
	FMatchPlayAuthoritativeAdvanceAfterTerminalRequest Advance;
	Advance.RequestingSide = Attacker;
	Advance.AttackSequence = 1;
	const auto Advanced = Session.AdvanceAfterTerminal(Advance);
	TestTrue(TEXT("No-Carrier terminal uses shared Advance"),
		Advanced.CompletionResult.bSuccess);
	TestEqual(TEXT("No-Carrier Advance consumes no participant"),
		Advanced.CompletionResult.SetPieceCardUsageResults.Num(), 0);
	return true;
}

#endif
