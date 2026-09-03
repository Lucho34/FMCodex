#include "MatchPlayCornerResolution.h"
#include "MatchPlayOpeningInitializer.h"
#include "MatchPlaySetPieceParticipantConsumption.h"
#include "TacticalRuleDescription.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

namespace MatchPlayCornerResolutionTests
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

	FMatchPlayState MakeAwaitingAttacker(const FString& Prefix)
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
		State.CurrentAttack.SetPieceRoute.RawTypeD6 = 1;
		State.CurrentAttack.SetPieceRoute.SelectedType =
			ESetPieceSelectedType::Corner;
		State.CurrentAttack.SetPieceRoute.Corner.Stage =
			EMatchPlaySetPieceCornerRouteStage::AwaitingAttackerNominations;
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
		Result.ErrorMessage = TEXT("Injected Corner roll failure.");
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
				Result.ErrorMessage = TEXT("Injected Corner Recovery failure.");
				return Result;
			}
			Result.bSuccess = true;
			for (int32 Index = 0; Index < ReturnCount; ++Index)
			{
				Result.SelectedCandidateIndices.Add(Index);
			}
			return Result;
		}

		bool bFail = false;
		int32 CallCount = 0;
		TArray<FMatchPlayRecoveryCandidate> Candidates;
	};

	FMatchPlayState LockBoth(
		FMatchPlayState State, const int32 AttackerCount, const int32 DefenderCount)
	{
		const EInitialTurnOrderPlayer Attacker =
			State.RuntimeState.CurrentAttackingPlayer;
		State = FMatchPlayCornerResolution::SubmitAttackerNominations(
			State, Nomination(State, Attacker, AttackerCount)).AfterState;
		FQueueRollProvider Provider;
		Provider.Results = { RollSuccess(1) };
		return FMatchPlayCornerResolution::SubmitDefenderNominations(
			State, Nomination(State, Other(Attacker), DefenderCount), &Provider).AfterState;
	}

	FMatchPlayState SelectParticipants(
		FMatchPlayState State,
		const int32 AttackerCount,
		const int32 DefenderCount,
		const int32 SharedD6)
	{
		State = LockBoth(MoveTemp(State), AttackerCount, DefenderCount);
		FQueueRollProvider Provider;
		Provider.Results = { RollSuccess(SharedD6) };
		return FMatchPlayCornerResolution::RequestParticipantSelectionRoll(
			State, RollRequest(State, false), &Provider).AfterState;
	}

	FMatchPlayCornerResolutionResult MakeTerminal(
		FMatchPlayState State,
		const int32 AttackerCount,
		const int32 DefenderCount,
		const int32 SharedD6,
		const EMatchPlayCornerRouteIntent Intent,
		const int32 RouteD6,
		const int32 AttackD6,
		const int32 DefenseD6)
	{
		State = SelectParticipants(MoveTemp(State), AttackerCount,
			DefenderCount, SharedD6);
		FMatchPlayCornerIntentRequest IntentRequest;
		IntentRequest.RequestingSide =
			State.RuntimeState.CurrentAttackingPlayer;
		IntentRequest.AttackSequence = State.CurrentAttack.AttackSequence;
		IntentRequest.IntendedRoute = Intent;
		State = FMatchPlayCornerResolution::SubmitIntent(
			State, IntentRequest).AfterState;
		FQueueRollProvider Provider;
		Provider.Results = {
			RollSuccess(RouteD6), RollSuccess(AttackD6), RollSuccess(DefenseD6) };
		State = FMatchPlayCornerResolution::RequestRouteRoll(
			State, RollRequest(State, false), &Provider).AfterState;
		State = FMatchPlayCornerResolution::RequestAttackRoll(
			State, RollRequest(State, false), &Provider).AfterState;
		return FMatchPlayCornerResolution::RequestDefenseRoll(
			State, RollRequest(State, true), &Provider);
	}

	int32 ExpectedIndex(const int32 Count, const int32 RawD6)
	{
		return Count == 3 ? (RawD6 - 1) / 2
			: Count == 2 ? (RawD6 <= 3 ? 0 : 1) : 0;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayCornerNominationsTest,
	"FMCodex.CoreRules.MatchPlayCorner.NominationsSealedOrderAndFailures",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayCornerNominationsTest::RunTest(const FString& Parameters)
{
	using namespace MatchPlayCornerResolutionTests;
	for (int32 Count = 0; Count <= 3; ++Count)
	{
		FMatchPlayState State = MakeAwaitingAttacker(
			FString::Printf(TEXT("CornerNom%d"), Count));
		const EInitialTurnOrderPlayer Attacker =
			State.RuntimeState.CurrentAttackingPlayer;
		const TArray<FName> Expected = FirstOutfield(State, Attacker, Count);
		const auto Result =
			FMatchPlayCornerResolution::SubmitAttackerNominations(
				State, Nomination(State, Attacker, Count));
		const FMatchPlayCornerRouteState& Corner =
			Result.AfterState.CurrentAttack.SetPieceRoute.Corner;
		TestTrue(TEXT("Attacker ordered 0-3 nominations accepted and locked"),
			Result.bSuccess && Corner.bAttackerNominationsLocked
				&& !Corner.bDefenderNominationsLocked
				&& Corner.Stage == EMatchPlaySetPieceCornerRouteStage
					::AwaitingDefenderNominations
				&& Corner.AttackerNominees.Num() == Count);
		for (int32 Index = 0; Index < Expected.Num(); ++Index)
		{
			TestEqual(TEXT("Attacker nomination order preserved"),
				Corner.AttackerNominees[Index].CardId, Expected[Index]);
		}
		FQueueRollProvider PrematureProvider;
		PrematureProvider.Results = { RollSuccess(1) };
		TestTrue(TEXT("Stage prevents participant D6 before defender lock"),
			!FMatchPlayCornerResolution::RequestParticipantSelectionRoll(
				Result.AfterState, RollRequest(Result.AfterState, false),
				&PrematureProvider).bSuccess
				&& PrematureProvider.Purposes.IsEmpty());
	}

	FMatchPlayState State = MakeAwaitingAttacker(TEXT("CornerNomFailures"));
	const EInitialTurnOrderPlayer Attacker =
		State.RuntimeState.CurrentAttackingPlayer;
	const EInitialTurnOrderPlayer Defender = Other(Attacker);
	auto AssertAtomicFailure = [this, &State](
		const TCHAR* Label, const FMatchPlayCornerNominationRequest& Request)
	{
		const auto Result =
			FMatchPlayCornerResolution::SubmitAttackerNominations(State, Request);
		TestTrue(Label, !Result.bSuccess && Equal(State, Result.AfterState));
	};
	FMatchPlayCornerNominationRequest TooMany = Nomination(State, Attacker, 3);
	TooMany.OrderedCardIds.Add(FindCard(State, Attacker, false, 3));
	AssertAtomicFailure(TEXT(">3 attacker nominees rejected atomically"), TooMany);
	FMatchPlayCornerNominationRequest Duplicate = Nomination(State, Attacker, 2);
	Duplicate.OrderedCardIds[1] = Duplicate.OrderedCardIds[0];
	AssertAtomicFailure(TEXT("Duplicate attacker nominee rejected atomically"), Duplicate);
	FMatchPlayCornerNominationRequest Goalkeeper = Nomination(State, Attacker, 0);
	Goalkeeper.OrderedCardIds = { FindCard(State, Attacker, true) };
	AssertAtomicFailure(TEXT("Goalkeeper nominee rejected atomically"), Goalkeeper);
	FMatchPlayCornerNominationRequest Opponent = Nomination(State, Attacker, 0);
	Opponent.OrderedCardIds = { FindCard(State, Defender, false) };
	AssertAtomicFailure(TEXT("Opponent nominee rejected atomically"), Opponent);
	FMatchPlayCornerNominationRequest Stale = Nomination(State, Attacker, 1);
	++Stale.AttackSequence;
	AssertAtomicFailure(TEXT("Stale attacker lock rejected atomically"), Stale);
	FMatchPlayCornerNominationRequest WrongSide = Nomination(State, Attacker, 1);
	WrongSide.RequestingSide = Defender;
	AssertAtomicFailure(TEXT("Wrong-side attacker lock rejected atomically"), WrongSide);

	for (const bool bEjected : { false, true })
	{
		FMatchPlayState Unavailable = MakeAwaitingAttacker(
			bEjected ? TEXT("CornerEjected") : TEXT("CornerUsed"));
		const EInitialTurnOrderPlayer Side =
			Unavailable.RuntimeState.CurrentAttackingPlayer;
		const FName CardId = FindCard(Unavailable, Side, false);
		Usage(Unavailable, Side).AvailableCardIds.Remove(CardId);
		(bEjected ? Usage(Unavailable, Side).EjectedCardIds
			: Usage(Unavailable, Side).UsedCardIds).Add(CardId);
		const auto Failed = FMatchPlayCornerResolution::SubmitAttackerNominations(
			Unavailable, Nomination(Unavailable, Side, 1));
		TestTrue(TEXT("Used/Ejected attacker nominee rejected atomically"),
			!Failed.bSuccess && Equal(Unavailable, Failed.AfterState));
	}

	const auto AttackerLocked =
		FMatchPlayCornerResolution::SubmitAttackerNominations(
			State, Nomination(State, Attacker, 3));
	const TArray<FMatchPlaySetPieceParticipantBinding> LockedAttackerList =
		AttackerLocked.AfterState.CurrentAttack.SetPieceRoute.Corner
			.AttackerNominees;
	TestFalse(TEXT("Duplicate attacker lock rejected"),
		FMatchPlayCornerResolution::SubmitAttackerNominations(
			AttackerLocked.AfterState,
			Nomination(AttackerLocked.AfterState, Attacker, 1)).bSuccess);
	auto AssertDefenderAtomicFailure = [this, &AttackerLocked](
		const TCHAR* Label, const FMatchPlayCornerNominationRequest& Request)
	{
		const auto Result =
			FMatchPlayCornerResolution::SubmitDefenderNominations(
				AttackerLocked.AfterState, Request);
		TestTrue(Label, !Result.bSuccess
			&& Equal(AttackerLocked.AfterState, Result.AfterState));
	};
	FMatchPlayCornerNominationRequest DefenderTooMany =
		Nomination(AttackerLocked.AfterState, Defender, 3);
	DefenderTooMany.OrderedCardIds.Add(
		FindCard(AttackerLocked.AfterState, Defender, false, 3));
	AssertDefenderAtomicFailure(TEXT(">3 defender nominees rejected atomically"),
		DefenderTooMany);
	FMatchPlayCornerNominationRequest DefenderDuplicate =
		Nomination(AttackerLocked.AfterState, Defender, 2);
	DefenderDuplicate.OrderedCardIds[1] =
		DefenderDuplicate.OrderedCardIds[0];
	AssertDefenderAtomicFailure(TEXT("Duplicate defender nominee rejected atomically"),
		DefenderDuplicate);
	FMatchPlayCornerNominationRequest DefenderGk =
		Nomination(AttackerLocked.AfterState, Defender, 0);
	DefenderGk.OrderedCardIds = {
		FindCard(AttackerLocked.AfterState, Defender, true) };
	AssertDefenderAtomicFailure(TEXT("Defender goalkeeper rejected atomically"),
		DefenderGk);
	FMatchPlayCornerNominationRequest DefenderOpponent =
		Nomination(AttackerLocked.AfterState, Defender, 0);
	DefenderOpponent.OrderedCardIds = {
		FindCard(AttackerLocked.AfterState, Attacker, false) };
	AssertDefenderAtomicFailure(TEXT("Attacker card rejected from defender list"),
		DefenderOpponent);
	FMatchPlayCornerNominationRequest DefenderStale =
		Nomination(AttackerLocked.AfterState, Defender, 1);
	++DefenderStale.AttackSequence;
	AssertDefenderAtomicFailure(TEXT("Stale defender lock rejected atomically"),
		DefenderStale);
	for (const bool bEjected : { false, true })
	{
		FMatchPlayState UnavailableDefender = AttackerLocked.AfterState;
		const FName CardId = FindCard(
			UnavailableDefender, Defender, false);
		Usage(UnavailableDefender, Defender).AvailableCardIds.Remove(CardId);
		(bEjected ? Usage(UnavailableDefender, Defender).EjectedCardIds
			: Usage(UnavailableDefender, Defender).UsedCardIds).Add(CardId);
		const auto Failed =
			FMatchPlayCornerResolution::SubmitDefenderNominations(
				UnavailableDefender,
				Nomination(UnavailableDefender, Defender, 1));
		TestTrue(TEXT("Used/Ejected defender nominee rejected atomically"),
			!Failed.bSuccess && Equal(UnavailableDefender, Failed.AfterState));
	}
	const auto DefenderLocked =
		FMatchPlayCornerResolution::SubmitDefenderNominations(
			AttackerLocked.AfterState,
			Nomination(AttackerLocked.AfterState, Defender, 3));
	TestTrue(TEXT("Defender equivalent accepts ordered three and preserves attacker seal"),
		DefenderLocked.bSuccess
			&& DefenderLocked.AfterState.CurrentAttack.SetPieceRoute.Corner
				.bDefenderNominationsLocked
			&& FMatchPlaySetPieceParticipantBinding::StaticStruct()
				->CompareScriptStruct(
					&DefenderLocked.AfterState.CurrentAttack.SetPieceRoute.Corner
						.AttackerNominees[1], &LockedAttackerList[1], 0));
	FMatchPlayCornerNominationRequest DefenderWrong =
		Nomination(AttackerLocked.AfterState, Defender, 1);
	DefenderWrong.RequestingSide = Attacker;
	TestTrue(TEXT("Wrong-side defender lock rejected atomically"),
		!FMatchPlayCornerResolution::SubmitDefenderNominations(
			AttackerLocked.AfterState, DefenderWrong).bSuccess);
	TestFalse(TEXT("Duplicate defender lock rejected"),
		FMatchPlayCornerResolution::SubmitDefenderNominations(
			DefenderLocked.AfterState,
			Nomination(DefenderLocked.AfterState, Defender, 1)).bSuccess);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayCornerZeroPrecedenceTest,
	"FMCodex.CoreRules.MatchPlayCorner.ZeroPrecedenceAutomaticGoalAndConsumption",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayCornerZeroPrecedenceTest::RunTest(const FString& Parameters)
{
	using namespace MatchPlayCornerResolutionTests;
	for (int32 DefenderCount = 0; DefenderCount <= 3; ++DefenderCount)
	{
		FMatchPlayState State = MakeAwaitingAttacker(
			FString::Printf(TEXT("CornerA0D%d"), DefenderCount));
		const int32 ScoreBefore = State.RuntimeState.PlayerAState.Score
			+ State.RuntimeState.PlayerBState.Score;
		const auto Attacker = State.RuntimeState.CurrentAttackingPlayer;
		State = FMatchPlayCornerResolution::SubmitAttackerNominations(
			State, Nomination(State, Attacker, 0)).AfterState;
		FQueueRollProvider ZeroProvider;
		const FMatchPlayState Terminal = FMatchPlayCornerResolution::SubmitDefenderNominations(
			State, Nomination(State, Other(Attacker), DefenderCount), &ZeroProvider).AfterState;
		TestTrue(TEXT("Attacker-zero branch never requests backend RNG"), ZeroProvider.Purposes.IsEmpty());
		const FMatchPlayCornerRouteState& Corner =
			Terminal.CurrentAttack.SetPieceRoute.Corner;
		const auto Consumption =
			FMatchPlaySetPieceParticipantConsumption::Extract(Terminal);
		TestTrue(TEXT("A=0 takes NoGoal precedence for D=0-3 with zero RNG/participants"),
			Corner.Stage == EMatchPlaySetPieceCornerRouteStage::Terminal
				&& Corner.GameplayOutcome
					== EMatchPlayCornerGameplayOutcome::NoGoal
				&& !Corner.bHasSharedParticipantD6
				&& !Corner.bHasRouteD6 && !Corner.bHasAttackD6
				&& !Corner.bHasDefenseD6 && !Corner.bHasFormulaResolution
				&& !Corner.bHasGoalScorer
				&& Consumption.bSuccess && Consumption.Participants.IsEmpty()
				&& Terminal.RuntimeState.PlayerAState.Score
					+ Terminal.RuntimeState.PlayerBState.Score == ScoreBefore);
	}

	for (int32 AttackerCount = 1; AttackerCount <= 3; ++AttackerCount)
	{
		FMatchPlayState State = MakeAwaitingAttacker(
			FString::Printf(TEXT("CornerA%dD0"), AttackerCount));
		const EInitialTurnOrderPlayer Attacker =
			State.RuntimeState.CurrentAttackingPlayer;
		const int32 ScoreBefore = Attacker == EInitialTurnOrderPlayer::PlayerA
			? State.RuntimeState.PlayerAState.Score
			: State.RuntimeState.PlayerBState.Score;
		const FMatchPlayState Terminal = LockBoth(State, AttackerCount, 0);
		const FMatchPlayCornerRouteState& Corner =
			Terminal.CurrentAttack.SetPieceRoute.Corner;
		const int32 ScoreAfter = Attacker == EInitialTurnOrderPlayer::PlayerA
			? Terminal.RuntimeState.PlayerAState.Score
			: Terminal.RuntimeState.PlayerBState.Score;
		const auto Consumption =
			FMatchPlaySetPieceParticipantConsumption::Extract(Terminal);
		TestTrue(TEXT("D=0 creates one Goal with an actual scorer and no player rolls"),
			Corner.GameplayOutcome
				== EMatchPlayCornerGameplayOutcome::Goal
				&& ScoreAfter == ScoreBefore + 1
				&& Corner.bHasGoalScorer && Corner.GoalScorerCardId == Corner.Runner.CardId
				&& !Corner.bHasSharedParticipantD6
				&& !Corner.bHasRouteD6 && !Corner.bHasAttackD6
				&& !Corner.bHasDefenseD6 && !Corner.bHasFormulaResolution
				&& Consumption.bSuccess && Consumption.Participants.Num() == 1);
		const auto Duplicate =
			FMatchPlayCornerResolution::SubmitDefenderNominations(
				Terminal, Nomination(Terminal, Other(Attacker), 0));
		TestTrue(TEXT("Duplicate shortage command cannot score twice"),
			!Duplicate.bSuccess && Equal(Terminal, Duplicate.AfterState));
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayCornerSharedD6MappingTest,
	"FMCodex.CoreRules.MatchPlayCorner.SharedD6MappingAndCandidateModifiers",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayCornerSharedD6MappingTest::RunTest(const FString& Parameters)
{
	using namespace MatchPlayCornerResolutionTests;
	for (const auto Owner : { EInitialTurnOrderPlayer::PlayerA, EInitialTurnOrderPlayer::PlayerB })
	for (int32 AttackerCount = 1; AttackerCount <= 3; ++AttackerCount)
	{
		for (int32 DefenderCount = 1; DefenderCount <= 3; ++DefenderCount)
		{
			for (int32 RawD6 = 1; RawD6 <= 6; ++RawD6)
			{
				FMatchPlayState Seed = MakeAwaitingAttacker(FString::Printf(
						TEXT("CornerMap%d%d%d"), AttackerCount,
						DefenderCount, RawD6));
				Seed.RuntimeState.CurrentAttackingPlayer = Owner;
				FMatchPlayState State = LockBoth(Seed, AttackerCount, DefenderCount);
				FQueueRollProvider Provider;
				Provider.Results = { RollSuccess(RawD6) };
				const auto Result =
					FMatchPlayCornerResolution::RequestParticipantSelectionRoll(
						State, RollRequest(State, false), &Provider);
				const FMatchPlayCornerRouteState& Corner =
					Result.AfterState.CurrentAttack.SetPieceRoute.Corner;
				const int32 Difference = FMath::Abs(
					AttackerCount - DefenderCount);
				const int32 ExpectedBonus = Difference == 1 ? 2
					: Difference == 2 ? 3 : 0;
				const EInitialTurnOrderPlayer Attacker =
					State.RuntimeState.CurrentAttackingPlayer;
				const EInitialTurnOrderPlayer ExpectedSide = ExpectedBonus == 0
					? EInitialTurnOrderPlayer::None
					: AttackerCount > DefenderCount
						? Attacker : Other(Attacker);
				TestTrue(TEXT("One shared D6 maps each ordered list independently"),
					Result.bSuccess && Provider.Purposes.Num() == 1
						&& Provider.Purposes[0]
							== EMatchPlayCurrentAttackPostRouteRollPurpose
								::CornerParticipantSelection
						&& Corner.Runner.CardId == Corner.AttackerNominees[
							ExpectedIndex(AttackerCount, RawD6)].CardId
						&& Corner.Helper.CardId == Corner.DefenderNominees[
							ExpectedIndex(DefenderCount, RawD6)].CardId
						&& Corner.Runner.OwnerSide == Owner && Corner.Helper.OwnerSide == Other(Owner));
				TestTrue(TEXT("Canonical +2/+3 bonus belongs only to larger list"),
					Corner.CandidateBonus == ExpectedBonus
						&& Corner.CandidateBonusSide == ExpectedSide
						&& Corner.CandidateBonus >= 0);
			}
		}
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayCornerIntentFormulaTest,
	"FMCodex.CoreRules.MatchPlayCorner.IntentRouteFormulaPrecisionAndRetry",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayCornerIntentFormulaTest::RunTest(const FString& Parameters)
{
	using namespace MatchPlayCornerResolutionTests;
	for (const EMatchPlayCornerRouteIntent Intent : {
		EMatchPlayCornerRouteIntent::High, EMatchPlayCornerRouteIntent::Low })
	{
		for (const int32 RawD6 : { 1, 2, 3, 4, 5, 6 })
		{
			FMatchPlayState State = SelectParticipants(
				MakeAwaitingAttacker(FString::Printf(TEXT("CornerRoute%d%d"),
					static_cast<int32>(Intent), RawD6)), 1, 1, 1);
			FMatchPlayCornerIntentRequest IntentRequest;
			IntentRequest.RequestingSide =
				State.RuntimeState.CurrentAttackingPlayer;
			IntentRequest.AttackSequence = 1;
			IntentRequest.IntendedRoute = Intent;
			const auto IntentResult =
				FMatchPlayCornerResolution::SubmitIntent(State, IntentRequest);
			TestTrue(TEXT("Intent submission is zero-RNG typed state"),
				IntentResult.bSuccess
					&& IntentResult.AfterState.CurrentAttack.SetPieceRoute.Corner
						.IntendedRoute == Intent);
			FQueueRollProvider Provider;
			Provider.Results = { RollSuccess(RawD6) };
			const auto Route = FMatchPlayCornerResolution::RequestRouteRoll(
				IntentResult.AfterState,
				RollRequest(IntentResult.AfterState, false), &Provider);
			const EMatchPlayCornerRouteIntent Expected = RawD6 <= 4 ? Intent
				: Intent == EMatchPlayCornerRouteIntent::High
					? EMatchPlayCornerRouteIntent::Low
					: EMatchPlayCornerRouteIntent::High;
			TestTrue(TEXT("Route D6 1-4 keeps and 5-6 switches intent"),
				Route.bSuccess && Provider.Purposes.Num() == 1
					&& Route.AfterState.CurrentAttack.SetPieceRoute.Corner
						.ActualRoute == Expected);
			const auto* Description = FTacticalRuleDescriptionCatalog::GetCornerInitialRouteOutcomes()
				.FindByPredicate([RawD6](const FTacticalRuleDescriptionOutcome& Outcome)
					{ return RawD6 >= Outcome.Minimum && RawD6 <= Outcome.Maximum; });
			TestTrue(TEXT("Every educational Corner route range agrees with the authoritative resolver"),
				Description && (Description->OutcomeId == TEXT("Corner.PreferredRoute"))
					== (Route.AfterState.CurrentAttack.SetPieceRoute.Corner.ActualRoute == Intent));
		}
	}

	FMatchPlayState RetryState = SelectParticipants(
		MakeAwaitingAttacker(TEXT("CornerRetry")), 1, 1, 1);
	FMatchPlayCornerIntentRequest IntentRequest;
	IntentRequest.RequestingSide =
		RetryState.RuntimeState.CurrentAttackingPlayer;
	IntentRequest.AttackSequence = 1;
	IntentRequest.IntendedRoute = EMatchPlayCornerRouteIntent::High;
	RetryState = FMatchPlayCornerResolution::SubmitIntent(
		RetryState, IntentRequest).AfterState;
	FQueueRollProvider Failure;
	Failure.Results = { RollFailure() };
	const auto Failed = FMatchPlayCornerResolution::RequestRouteRoll(
		RetryState, RollRequest(RetryState, false), &Failure);
	TestTrue(TEXT("Provider failure is atomic and retryable"),
		!Failed.bSuccess && Equal(RetryState, Failed.AfterState));
	FMatchPlayCornerRollRequest Wrong = RollRequest(RetryState, false);
	Wrong.RequestingSide = Other(Wrong.RequestingSide);
	FQueueRollProvider NoCall;
	NoCall.Results = { RollSuccess(1) };
	TestTrue(TEXT("Wrong-side route request consumes zero RNG"),
		!FMatchPlayCornerResolution::RequestRouteRoll(
			RetryState, Wrong, &NoCall).bSuccess && NoCall.Purposes.IsEmpty());
	FMatchPlayCornerRollRequest Stale = RollRequest(RetryState, false);
	++Stale.AttackSequence;
	TestTrue(TEXT("Stale route request consumes zero RNG"),
		!FMatchPlayCornerResolution::RequestRouteRoll(
			RetryState, Stale, &NoCall).bSuccess && NoCall.Purposes.IsEmpty());

	FMatchPlayState HighState = MakeAwaitingAttacker(TEXT("CornerHighHalf"));
	const EInitialTurnOrderPlayer HighAttacker =
		HighState.RuntimeState.CurrentAttackingPlayer;
	const EInitialTurnOrderPlayer HighDefender = Other(HighAttacker);
	FindSnapshot(HighState, HighAttacker, false)->Attributes.Strength = 6;
	FindSnapshot(HighState, HighAttacker, false)->Attributes.Shooting = 1;
	FindSnapshot(HighState, HighDefender, false)->Attributes.Strength = 3;
	FindSnapshot(HighState, HighDefender, false)->Attributes.Marking = 6;
	FindSnapshot(HighState, HighDefender, true)->GoalkeeperAttributes.Aerial = 4;
	FindSnapshot(HighState, HighDefender, true)->GoalkeeperAttributes.Reflex = 6;
	const auto High = MakeTerminal(HighState, 1, 1, 1,
		EMatchPlayCornerRouteIntent::High, 1, 1, 1);
	const FMatchPlayCornerRouteState& HighCorner =
		High.AfterState.CurrentAttack.SetPieceRoute.Corner;
	TestTrue(TEXT("High uses Strength/Aerial 3.5 average plus fixed +2 without Low leakage"),
		High.bSuccess
			&& FMath::IsNearlyEqual(High.FormulaInput.Defender.BaseValue, 3.5f)
			&& FMath::IsNearlyEqual(High.FormulaInput.Defender.Modifier, 2.0f)
			&& FMath::IsNearlyEqual(
				HighCorner.FormulaResolution.AttackerFinalValue, 7.0f)
			&& FMath::IsNearlyEqual(
				HighCorner.FormulaResolution.DefenderFinalValue, 6.5f)
			&& HighCorner.GameplayOutcome
				== EMatchPlayCornerGameplayOutcome::Goal
			&& HighCorner.GoalScorerCardId == HighCorner.Runner.CardId);

	FMatchPlayState TieState = MakeAwaitingAttacker(TEXT("CornerHighTie"));
	const EInitialTurnOrderPlayer TieAttacker =
		TieState.RuntimeState.CurrentAttackingPlayer;
	const EInitialTurnOrderPlayer TieDefender = Other(TieAttacker);
	FindSnapshot(TieState, TieAttacker, false)->Attributes.Strength = 4;
	FindSnapshot(TieState, TieDefender, false)->Attributes.Strength = 3;
	FindSnapshot(TieState, TieDefender, true)->GoalkeeperAttributes.Aerial = 3;
	const auto Tie = MakeTerminal(TieState, 1, 1, 1,
		EMatchPlayCornerRouteIntent::High, 1, 2, 1);
	TestTrue(TEXT("Exact Corner Formula tie belongs to goalkeeper defender"),
		Tie.bSuccess && Tie.FormulaResolution.Winner == EFormulaWinner::Defender
			&& Tie.FormulaResolution.WinReason
				== EFormulaWinReason::DefenderWinsGoalkeeperTie
			&& Tie.AfterState.CurrentAttack.SetPieceRoute.Corner.GameplayOutcome
				== EMatchPlayCornerGameplayOutcome::NoGoal);

	FMatchPlayState LowState = MakeAwaitingAttacker(TEXT("CornerLowHalf"));
	const EInitialTurnOrderPlayer LowAttacker =
		LowState.RuntimeState.CurrentAttackingPlayer;
	const EInitialTurnOrderPlayer LowDefender = Other(LowAttacker);
	FindSnapshot(LowState, LowAttacker, false)->Attributes.Shooting = 5;
	FindSnapshot(LowState, LowAttacker, false)->Attributes.Strength = 1;
	FindSnapshot(LowState, LowDefender, false)->Attributes.Marking = 3;
	FindSnapshot(LowState, LowDefender, false)->Attributes.Strength = 6;
	FindSnapshot(LowState, LowDefender, true)->GoalkeeperAttributes.Reflex = 4;
	FindSnapshot(LowState, LowDefender, true)->GoalkeeperAttributes.Aerial = 6;
	const auto Low = MakeTerminal(LowState, 1, 1, 1,
		EMatchPlayCornerRouteIntent::Low, 1, 1, 1);
	TestTrue(TEXT("Low uses Shooting/Marking/Reflex with half precision and no High leakage"),
		Low.bSuccess
			&& FMath::IsNearlyEqual(Low.FormulaInput.Defender.BaseValue, 3.5f)
			&& FMath::IsNearlyEqual(Low.FormulaResolution.AttackerFinalValue, 6.0f)
			&& FMath::IsNearlyEqual(Low.FormulaResolution.DefenderFinalValue, 6.5f)
			&& Low.FormulaResolution.Winner == EFormulaWinner::Defender);

	for (const TPair<int32, int32> Counts : {
		TPair<int32, int32>(2, 1), TPair<int32, int32>(3, 1),
		TPair<int32, int32>(1, 2), TPair<int32, int32>(1, 3) })
	{
		const auto BonusResult = MakeTerminal(
			MakeAwaitingAttacker(FString::Printf(TEXT("CornerBonus%d%d"),
				Counts.Key, Counts.Value)), Counts.Key, Counts.Value, 1,
			EMatchPlayCornerRouteIntent::High, 1, 3, 3);
		const int32 Bonus = FMath::Abs(Counts.Key - Counts.Value) == 1 ? 2 : 3;
		const bool bAttackerLarger = Counts.Key > Counts.Value;
		TestTrue(TEXT("Candidate +2/+3 is applied once to only the larger Formula side"),
			BonusResult.bSuccess
				&& FMath::IsNearlyEqual(BonusResult.FormulaInput.Attacker.Modifier,
					bAttackerLarger ? static_cast<float>(Bonus) : 0.0f)
				&& FMath::IsNearlyEqual(BonusResult.FormulaInput.Defender.Modifier,
					2.0f + (bAttackerLarger ? 0.0f
						: static_cast<float>(Bonus))));
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayCornerAdvanceRecoveryReconstructionTest,
	"FMCodex.CoreRules.MatchPlayCorner.AdvanceRecoveryConsumptionAndReconstruction",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayCornerAdvanceRecoveryReconstructionTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayCornerResolutionTests;
	FMatchPlayState State = MakeAwaitingAttacker(TEXT("CornerAdvance"));
	const EInitialTurnOrderPlayer Attacker =
		State.RuntimeState.CurrentAttackingPlayer;
	const EInitialTurnOrderPlayer Defender = Other(Attacker);
	State = FMatchPlayCornerResolution::SubmitAttackerNominations(
		State, Nomination(State, Attacker, 2)).AfterState;
	TestTrue(TEXT("AwaitingDefenderNominations reconstructs"),
		FMatchPlayCurrentAttackRouteStateValidator::Validate(State).bIsCanonical);
	State = FMatchPlayCornerResolution::SubmitDefenderNominations(
		State, Nomination(State, Defender, 2)).AfterState;
	TestTrue(TEXT("AwaitingParticipantSelectionRoll reconstructs"),
		FMatchPlayCurrentAttackRouteStateValidator::Validate(State).bIsCanonical);
	FQueueRollProvider Provider;
	Provider.Results = { RollSuccess(1), RollSuccess(1),
		RollSuccess(5), RollSuccess(1) };
	State = FMatchPlayCornerResolution::RequestParticipantSelectionRoll(
		State, RollRequest(State, false), &Provider).AfterState;
	TestTrue(TEXT("AwaitingIntent reconstructs"),
		FMatchPlayCurrentAttackRouteStateValidator::Validate(State).bIsCanonical);
	const FName Runner = State.CurrentAttack.SetPieceRoute.Corner.Runner.CardId;
	const FName Helper = State.CurrentAttack.SetPieceRoute.Corner.Helper.CardId;
	const FName UnselectedAttacker =
		State.CurrentAttack.SetPieceRoute.Corner.AttackerNominees[1].CardId;
	const FName UnselectedDefender =
		State.CurrentAttack.SetPieceRoute.Corner.DefenderNominees[1].CardId;
	const FName Goalkeeper = FindCard(State, Defender, true);
	FMatchPlayCornerIntentRequest Intent;
	Intent.RequestingSide = Attacker;
	Intent.AttackSequence = 1;
	Intent.IntendedRoute = EMatchPlayCornerRouteIntent::High;
	State = FMatchPlayCornerResolution::SubmitIntent(State, Intent).AfterState;
	TestTrue(TEXT("AwaitingRouteRoll reconstructs"),
		FMatchPlayCurrentAttackRouteStateValidator::Validate(State).bIsCanonical);
	State = FMatchPlayCornerResolution::RequestRouteRoll(
		State, RollRequest(State, false), &Provider).AfterState;
	State = FMatchPlayCornerResolution::RequestAttackRoll(
		State, RollRequest(State, false), &Provider).AfterState;
	TestTrue(TEXT("AwaitingDefenseRoll with attack prefix reconstructs"),
		FMatchPlayCurrentAttackRouteStateValidator::Validate(State).bIsCanonical);
	const auto TerminalResult = FMatchPlayCornerResolution::RequestDefenseRoll(
		State, RollRequest(State, true), &Provider);
	const FMatchPlayState Terminal = TerminalResult.AfterState;
	TestTrue(TEXT("Normal terminal reconstructs and selected cards remain Available"),
		TerminalResult.bSuccess
			&& FMatchPlayCurrentAttackRouteStateValidator::Validate(Terminal)
				.bIsCanonical
			&& Usage(Terminal, Attacker).AvailableCardIds.Contains(Runner)
			&& Usage(Terminal, Defender).AvailableCardIds.Contains(Helper));
	const auto Extracted =
		FMatchPlaySetPieceParticipantConsumption::Extract(Terminal);
	TestTrue(TEXT("Only Runner and Helper are extracted for consumption"),
		Extracted.bSuccess && Extracted.Participants.Num() == 2
			&& Extracted.Participants.ContainsByPredicate(
				[Runner](const auto& P) { return P.CardId == Runner; })
			&& Extracted.Participants.ContainsByPredicate(
				[Helper](const auto& P) { return P.CardId == Helper; }));

	FCapturingRecoveryProvider Failure;
	Failure.bFail = true;
	const auto Failed = FMatchPlayCurrentAttackCompletion::AdvanceAfterTerminal(
		Terminal, 1, Attacker, &Failure);
	TestTrue(TEXT("Recovery failure rolls back consumption and opportunity"),
		!Failed.bSuccess && Equal(Terminal, Failed.AfterState)
			&& Usage(Failed.AfterState, Attacker)
				.AvailableCardIds.Contains(Runner)
			&& Usage(Failed.AfterState, Defender)
				.AvailableCardIds.Contains(Helper));

	FCapturingRecoveryProvider Recovery;
	const auto Advanced = FMatchPlayCurrentAttackCompletion::AdvanceAfterTerminal(
		Terminal, 1, Attacker, &Recovery);
	TestTrue(TEXT("Advance consumes exactly two and Recovery sees both immediately"),
		Advanced.bSuccess && Advanced.SetPieceCardUsageResults.Num() == 2
			&& Recovery.Candidates.ContainsByPredicate(
				[Runner](const auto& C) { return C.CardId == Runner; })
			&& Recovery.Candidates.ContainsByPredicate(
				[Helper](const auto& C) { return C.CardId == Helper; })
			&& !Advanced.AfterState.bHasCurrentAttack
			&& Advanced.NextAttackingPlayer == Defender);
	TestTrue(TEXT("Non-selected nominees and GK remain Available"),
		Usage(Advanced.AfterState, Attacker)
			.AvailableCardIds.Contains(UnselectedAttacker)
			&& Usage(Advanced.AfterState, Defender)
				.AvailableCardIds.Contains(UnselectedDefender)
			&& Usage(Advanced.AfterState, Defender)
				.AvailableCardIds.Contains(Goalkeeper));

	FMatchPlayState FinalSeed = MakeAwaitingAttacker(TEXT("CornerFinal"));
	const EInitialTurnOrderPlayer FinalAttacker =
		FinalSeed.RuntimeState.CurrentAttackingPlayer;
	const EInitialTurnOrderPlayer FinalDefender = Other(FinalAttacker);
	FPlayerRuntimeState& FinalSeedAttackerRuntime =
		FinalAttacker == EInitialTurnOrderPlayer::PlayerA
			? FinalSeed.RuntimeState.PlayerAState
			: FinalSeed.RuntimeState.PlayerBState;
	FPlayerRuntimeState& FinalSeedDefenderRuntime =
		FinalAttacker == EInitialTurnOrderPlayer::PlayerA
			? FinalSeed.RuntimeState.PlayerBState
			: FinalSeed.RuntimeState.PlayerAState;
	FinalSeedAttackerRuntime.TotalAttackCount = 1;
	FinalSeedAttackerRuntime.UsedAttackCount = 0;
	FinalSeedDefenderRuntime.TotalAttackCount = 1;
	FinalSeedDefenderRuntime.UsedAttackCount = 1;
	FinalSeed.CurrentAttack.AttackSequence = 2;
	const auto FinalTerminalResult = MakeTerminal(
		FinalSeed, 1, 1, 1, EMatchPlayCornerRouteIntent::High, 1, 5, 1);
	const FMatchPlayState Final = FinalTerminalResult.AfterState;
	FCapturingRecoveryProvider FinalRecovery;
	const auto FinalAdvance =
		FMatchPlayCurrentAttackCompletion::AdvanceAfterTerminal(
			Final, 2, FinalAttacker, &FinalRecovery);
	TestTrue(TEXT("Final consumes Runner/Helper, resolves MatchResult, skips Recovery"),
		FinalAdvance.bSuccess && FinalAdvance.bMatchEnded
			&& FinalAdvance.SetPieceCardUsageResults.Num() == 2
			&& FinalAdvance.MatchResultResolveResult.bSuccess
			&& FinalRecovery.CallCount == 0);

	const FMatchPlayState AZero = LockBoth(
		MakeAwaitingAttacker(TEXT("CornerRebuildA0")), 0, 2);
	const FMatchPlayState DZero = LockBoth(
		MakeAwaitingAttacker(TEXT("CornerRebuildD0")), 2, 0);
	TestTrue(TEXT("NoGoal and automatic scorer Goal terminals reconstruct"),
		FMatchPlayCurrentAttackRouteStateValidator::Validate(AZero).bIsCanonical
			&& FMatchPlayCurrentAttackRouteStateValidator::Validate(DZero)
				.bIsCanonical);
	FCapturingRecoveryProvider ShortageRecovery;
	const auto ShortageAdvance =
		FMatchPlayCurrentAttackCompletion::AdvanceAfterTerminal(
			AZero, 1, AZero.RuntimeState.CurrentAttackingPlayer,
			&ShortageRecovery);
	TestTrue(TEXT("Shortage Advance consumes zero Corner participant cards"),
		ShortageAdvance.bSuccess
			&& ShortageAdvance.SetPieceCardUsageResults.IsEmpty());

	FMatchPlayState Corrupt = Terminal;
	Corrupt.CurrentAttack.SetPieceRoute.Corner.FormulaResolution
		.DefenderFinalValue += 1.0f;
	TestFalse(TEXT("Reconstruction rejects corrupt Corner Formula"),
		FMatchPlayCurrentAttackRouteStateValidator::Validate(Corrupt)
			.bIsCanonical);
	Corrupt = Terminal;
	Corrupt.CurrentAttack.SetPieceRoute.Corner.Runner.CardId =
		FindCard(Corrupt, Attacker, false, 5);
	TestFalse(TEXT("Reconstruction rejects corrupt shared-D6 mapping"),
		FMatchPlayCurrentAttackRouteStateValidator::Validate(Corrupt)
			.bIsCanonical);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayCornerAutomaticScorerMatrixTest,
	"FMCodex.CoreRules.MatchPlayCorner.AutomaticScorerMappingRetryAndLifecycle",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayCornerAutomaticScorerMatrixTest::RunTest(const FString& Parameters)
{
	using namespace MatchPlayCornerResolutionTests;
	const int32 Expected[3][6] = { { 0,0,0,0,0,0 }, { 0,0,0,1,1,1 }, { 0,0,1,1,2,2 } };
	TestEqual(TEXT("Empty nominations have no mapping"), FMatchPlayCornerResolution::MapParticipantIndex(0, 1), INDEX_NONE);
	TestEqual(TEXT("Invalid D6 has no mapping"), FMatchPlayCornerResolution::MapParticipantIndex(3, 7), INDEX_NONE);
	for (const auto Attacker : { EInitialTurnOrderPlayer::PlayerA, EInitialTurnOrderPlayer::PlayerB })
	for (int32 Count = 1; Count <= 3; ++Count)
	for (int32 D6 = 1; D6 <= 6; ++D6)
	{
		TestEqual(TEXT("Uniform canonical mapping table"),
			FMatchPlayCornerResolution::MapParticipantIndex(Count, D6), Expected[Count - 1][D6 - 1]);
		FMatchPlayState State = MakeAwaitingAttacker(TEXT("AutoScorer"));
		State.RuntimeState.CurrentAttackingPlayer = Attacker;
		State = FMatchPlayCornerResolution::SubmitAttackerNominations(
			State, Nomination(State, Attacker, Count)).AfterState;
		const auto Request = Nomination(State, Other(Attacker), 0);
		FQueueRollProvider Provider;
		Provider.Results = { RollSuccess(D6) };
		auto Stale = Request;
		Stale.AttackSequence += 1;
		const auto Rejected = FMatchPlayCornerResolution::SubmitDefenderNominations(State, Stale, &Provider);
		TestTrue(TEXT("Stale lock neither adopts nor consumes scorer RNG"),
			!Rejected.bSuccess && Equal(State, Rejected.AfterState) && Provider.Purposes.IsEmpty());
		auto WrongSide = Request;
		WrongSide.RequestingSide = Attacker;
		TestFalse(TEXT("Wrong owner cannot trigger scorer selection"),
			FMatchPlayCornerResolution::SubmitDefenderNominations(State, WrongSide, &Provider).bSuccess);
		TestTrue(TEXT("Wrong owner consumes zero RNG"), Provider.Purposes.IsEmpty());
		if (Count > 1)
		{
			FQueueRollProvider Failure;
			Failure.Results = { RollSuccess(7) };
			const auto Failed = FMatchPlayCornerResolution::SubmitDefenderNominations(State, Request, &Failure);
			TestTrue(TEXT("Malformed backend D6 rolls back lock, scorer and score"),
				!Failed.bSuccess && Equal(State, Failed.AfterState));
		}
		const auto Resolved = FMatchPlayCornerResolution::SubmitDefenderNominations(State, Request, &Provider);
		if (!TestTrue(TEXT("Automatic scorer resolves atomically"), Resolved.bSuccess)) return false;
		const auto& Terminal = Resolved.AfterState;
		const auto& Corner = Terminal.CurrentAttack.SetPieceRoute.Corner;
		const FName Scorer = Corner.AttackerNominees[Expected[Count - 1][D6 - 1]].CardId;
		TestTrue(TEXT("Actual scorer is the mapped ordered nominee; backend draw is typed"),
			Corner.GameplayOutcome == EMatchPlayCornerGameplayOutcome::Goal
				&& Corner.bHasGoalScorer && Corner.GoalScorerCardId == Scorer
				&& Corner.Runner.OwnerSide == Attacker && !Corner.Helper.bIsBound
				&& Corner.AutomaticScorerD6 == (Count == 1 ? 0 : D6)
				&& Provider.Purposes.Num() == (Count == 1 ? 0 : 1)
				&& (Count == 1 || Provider.Purposes[0] == EMatchPlayCurrentAttackPostRouteRollPurpose::CornerAutomaticScorer));
		TestTrue(TEXT("Automatic goal has no player roll/route/formula and awaits Advance"),
			!Corner.bHasSharedParticipantD6 && !Corner.bHasRouteD6
				&& !Corner.bHasAttackD6 && !Corner.bHasDefenseD6 && !Corner.bHasFormulaResolution
				&& Corner.IntendedRoute == EMatchPlayCornerRouteIntent::None
				&& Terminal.CurrentAttack.LifecycleState == EMatchPlayCurrentAttackLifecycleState::TerminalPendingAdvance
				&& Usage(Terminal, Attacker).AvailableCardIds.Contains(Scorer));
		const int32 DrawCount = Provider.Purposes.Num();
		TestFalse(TEXT("Duplicate lock cannot score or draw again"),
			FMatchPlayCornerResolution::SubmitDefenderNominations(Terminal, Request, &Provider).bSuccess);
		TestEqual(TEXT("Duplicate has zero extra draws"), Provider.Purposes.Num(), DrawCount);
		FCapturingRecoveryProvider Recovery;
		const auto Advanced = FMatchPlayCurrentAttackCompletion::AdvanceAfterTerminal(Terminal, 1, Attacker, &Recovery);
		TestTrue(TEXT("Advance consumes exactly the actual scorer and completes shared Recovery"),
			Advanced.bSuccess && Advanced.SetPieceCardUsageResults.Num() == 1);
		for (const auto& Nominee : Corner.AttackerNominees)
			if (Nominee.CardId != Scorer)
				TestTrue(TEXT("Unselected nominee remains Available after Advance"),
					Usage(Advanced.AfterState, Attacker).AvailableCardIds.Contains(Nominee.CardId));
		FMatchPlayState Corrupt = Terminal;
		Corrupt.CurrentAttack.SetPieceRoute.Corner.AutomaticScorerD6 = Count == 1 ? 1 : 7;
		TestFalse(TEXT("Reconstruction rejects corrupt hidden draw"),
			FMatchPlayCurrentAttackRouteStateValidator::Validate(Corrupt).bIsCanonical);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayCornerPreviewTest,
	"FMCodex.CoreRules.MatchPlayCorner.ProgressiveFormulaReadOnly",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayCornerPreviewTest::RunTest(const FString& Parameters)
{
	using namespace MatchPlayCornerResolutionTests;
	for (const auto Intent : { EMatchPlayCornerRouteIntent::High, EMatchPlayCornerRouteIntent::Low })
	for (const auto Attacker : { EInitialTurnOrderPlayer::PlayerA, EInitialTurnOrderPlayer::PlayerB })
	{
		FMatchPlayState Seed = MakeAwaitingAttacker(TEXT("CornerPreview"));
		Seed.RuntimeState.CurrentAttackingPlayer = Attacker;
		FMatchPlayState State = SelectParticipants(Seed, 3, 2, 5);
		TestTrue(TEXT("Unequal side-owned selection uses its own count"),
			State.CurrentAttack.SetPieceRoute.Corner.Runner.CardId == FirstOutfield(Seed, Attacker, 3)[2]
				&& State.CurrentAttack.SetPieceRoute.Corner.Helper.CardId == FirstOutfield(Seed, Other(Attacker), 2)[1]);
		FMatchPlayCornerIntentRequest IntentRequest;
		IntentRequest.AttackSequence = 1;
		IntentRequest.RequestingSide = Attacker;
		IntentRequest.IntendedRoute = Intent;
		State = FMatchPlayCornerResolution::SubmitIntent(State, IntentRequest).AfterState;
		FQueueRollProvider Provider;
		Provider.Results = { RollSuccess(1), RollSuccess(3), RollSuccess(4) };
		State = FMatchPlayCornerResolution::RequestRouteRoll(State, RollRequest(State, false), &Provider).AfterState;
		const auto Before = State;
		const auto Base = FMatchPlayCornerResolution::QueryFormulaPreview(State);
		TestTrue(TEXT("Preview publishes known numeric totals without mutation or RNG"),
			Base.bAvailable && Base.AttackKnownSubtotal == Base.AttackCurrentTotal
				&& Base.DefenseKnownSubtotal == Base.DefenseCurrentTotal
				&& Equal(Before, State) && Provider.Purposes.Num() == 1);
		State = FMatchPlayCornerResolution::RequestAttackRoll(State, RollRequest(State, false), &Provider).AfterState;
		const auto AttackOnly = FMatchPlayCornerResolution::QueryFormulaPreview(State);
		TestTrue(TEXT("Attack prefix updates without resolving defense"),
			AttackOnly.bAvailable && AttackOnly.AttackCurrentTotal == Base.AttackKnownSubtotal + 3
				&& AttackOnly.DefenseCurrentTotal == Base.DefenseKnownSubtotal);
		State = FMatchPlayCornerResolution::RequestDefenseRoll(State, RollRequest(State, true), &Provider).AfterState;
		const auto Final = FMatchPlayCornerResolution::QueryFormulaPreview(State);
		TestTrue(TEXT("Read-only projection equals canonical final Formula"),
			Final.bAvailable && Final.AttackCurrentTotal == State.CurrentAttack.SetPieceRoute.Corner.FormulaResolution.AttackerFinalValue
				&& Final.DefenseCurrentTotal == State.CurrentAttack.SetPieceRoute.Corner.FormulaResolution.DefenderFinalValue);
	}
	return true;
}

#endif
