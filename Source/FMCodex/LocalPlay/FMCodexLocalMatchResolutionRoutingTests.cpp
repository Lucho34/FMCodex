#include "FMCodexLocalMatchHostGameMode.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "../CoreRules/MatchPlayCurrentAttackCarrierSelectionAvailability.h"
#include "../CoreRules/MatchPlayCurrentAttackHelperSelectionAvailability.h"
#include "../CoreRules/MatchPlayCurrentAttackMarkerSelectionAvailability.h"
#include "../CoreRules/MatchPlayCurrentAttackRunnerSelectionAvailability.h"
#include "../CoreRules/MatchPlayGoalkeeperDeploymentAvailability.h"
#include "../CoreRules/MatchPlayOrdinaryDeploymentAvailability.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

#include <type_traits>

namespace FMCodexLocalMatchResolutionRoutingTests
{
	FPlayerCardData MakeCard(
		const FString& CardId,
		const bool bGoalkeeper,
		const FName SkillId)
	{
		FPlayerCardData Card;
		Card.CardId = FName(*CardId);
		Card.Rarity = ECardRarity::Common;
		Card.bIsGoalkeeper = bGoalkeeper;
		Card.PositionTypes = bGoalkeeper
			? TArray<EPlayerPositionType>{ EPlayerPositionType::Goalkeeper }
			: TArray<EPlayerPositionType>{
				EPlayerPositionType::Attack,
				EPlayerPositionType::Midfield,
				EPlayerPositionType::Defense
			};
		if (!bGoalkeeper && !SkillId.IsNone())
		{
			Card.AttackSkillIds = { SkillId };
		}
		return Card;
	}

	TArray<FPlayerCardData> MakeDeck(
		const FString& Prefix,
		const FName SkillId)
	{
		TArray<FPlayerCardData> Deck;
		for (int32 Index = 0; Index < 19; ++Index)
		{
			Deck.Add(MakeCard(
				FString::Printf(TEXT("%s_OUT_%02d"), *Prefix, Index),
				false,
				SkillId));
		}
		Deck.Add(MakeCard(Prefix + TEXT("_GK"), true, NAME_None));
		return Deck;
	}

	FMatchPlayOpeningInitializeInput MakeInput(
		const FString& Prefix,
		const FName SkillId)
	{
		FMatchPlayOpeningInitializeInput Input;
		Input.OpeningInput.PlayerADeck = MakeDeck(Prefix + TEXT("_A"), SkillId);
		Input.OpeningInput.PlayerBDeck = MakeDeck(Prefix + TEXT("_B"), SkillId);
		Input.OpeningInput.PlayerAAttackCountD6Roll = 1;
		Input.OpeningInput.PlayerBAttackCountD6Roll = 1;
		Input.OpeningInput.PlayerATieBreakerRoll = 2;
		Input.OpeningInput.PlayerBTieBreakerRoll = 6;
		for (int32 Index = 0; Index < 10; ++Index)
		{
			FMatchPlayDeploymentSlotDefinition NearA;
			NearA.SlotId = FName(*FString::Printf(
				TEXT("%s_NearA_%02d"), *Prefix, Index));
			NearA.NeutralSide = EMatchPlayNeutralSlotSide::NearPlayerA;
			Input.DeploymentSlotCatalog.Slots.Add(NearA);
			FMatchPlayDeploymentSlotDefinition NearB;
			NearB.SlotId = FName(*FString::Printf(
				TEXT("%s_NearB_%02d"), *Prefix, Index));
			NearB.NeutralSide = EMatchPlayNeutralSlotSide::NearPlayerB;
			Input.DeploymentSlotCatalog.Slots.Add(NearB);
		}
		return Input;
	}

	FSkillRuleSnapshotSet MakeRules(
		const FName SkillId,
		const ESkillRuleType SkillType)
	{
		FSkillRuleSnapshot Rule;
		Rule.SkillId = SkillId;
		Rule.SkillType = SkillType;
		Rule.MinTriggerActionPoint = 2;
		Rule.MaxTriggerActionPoint = 8;
		FSkillRuleSnapshotSet Rules;
		Rules.SkillRules = { Rule };
		return Rules;
	}

	EInitialTurnOrderPlayer OtherPlayer(
		const EInitialTurnOrderPlayer Player)
	{
		return Player == EInitialTurnOrderPlayer::PlayerA
			? EInitialTurnOrderPlayer::PlayerB
			: EInitialTurnOrderPlayer::PlayerA;
	}

	int32 ScoreFor(
		const FMatchPlayState& State,
		const EInitialTurnOrderPlayer Side)
	{
		return Side == EInitialTurnOrderPlayer::PlayerA
			? State.RuntimeState.PlayerAState.Score
			: State.RuntimeState.PlayerBState.Score;
	}

	int32 UsedAttacksFor(
		const FMatchPlayState& State,
		const EInitialTurnOrderPlayer Side)
	{
		return Side == EInitialTurnOrderPlayer::PlayerA
			? State.RuntimeState.PlayerAState.UsedAttackCount
			: State.RuntimeState.PlayerBState.UsedAttackCount;
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

	bool LoadSource(const TCHAR* RelativePath, FString& OutSource)
	{
		return FFileHelper::LoadFileToString(
			OutSource,
			*FPaths::ConvertRelativePathToFull(
				FPaths::ProjectDir() / RelativePath));
	}

	int32 CountOccurrences(const FString& Source, const FString& Needle)
	{
		int32 Count = 0;
		int32 Offset = 0;
		while ((Offset = Source.Find(
			Needle,
			ESearchCase::CaseSensitive,
			ESearchDir::FromStart,
			Offset)) != INDEX_NONE)
		{
			++Count;
			Offset += Needle.Len();
		}
		return Count;
	}

	class FScopedWorld final
	{
	public:
		FScopedWorld()
		{
			World = UWorld::CreateWorld(EWorldType::Game, false);
			if (World != nullptr && GEngine != nullptr)
			{
				FWorldContext& Context =
					GEngine->CreateNewWorldContext(EWorldType::Game);
				Context.SetCurrentWorld(World);
				Host = World->SpawnActor<AFMCodexLocalMatchHostGameMode>();
			}
		}

		~FScopedWorld()
		{
			if (World != nullptr)
			{
				if (GEngine != nullptr)
				{
					GEngine->DestroyWorldContext(World);
				}
				World->DestroyWorld(false);
			}
		}

		AFMCodexLocalMatchHostGameMode* GetHost() const { return Host; }

	private:
		UWorld* World = nullptr;
		AFMCodexLocalMatchHostGameMode* Host = nullptr;
	};

	const TArray<FName>& AvailableCards(
		const FMatchPlayState& State,
		const EInitialTurnOrderPlayer Side)
	{
		return Side == EInitialTurnOrderPlayer::PlayerA
			? State.CardUsageState.PlayerACardUsageState.AvailableCardIds
			: State.CardUsageState.PlayerBCardUsageState.AvailableCardIds;
	}

	bool FindDeployment(
		const FMatchPlayState& State,
		const EMatchPlayRelativeDeploymentZone PreferredZone,
		FMatchPlayAuthoritativeDeployOrdinaryRequest& OutRequest)
	{
		const EInitialTurnOrderPlayer Side =
			State.CurrentAttack.CurrentLegalDeploymentSide;
		for (const FName CardId : AvailableCards(State, Side))
		{
			const auto Availability =
				FMatchPlayOrdinaryDeploymentAvailability::Query(
					State,
					State.CurrentAttack.AttackSequence,
					Side,
					CardId);
			for (const auto& Slot : Availability.SlotResults)
			{
				if (Slot.LegalityResult.bIsLegal
					&& Slot.LegalityResult.ResolvedRelativeZone
						== PreferredZone)
				{
					OutRequest.RequestingSide = Side;
					OutRequest.CardId = CardId;
					OutRequest.SlotId = Slot.SlotId;
					return true;
				}
			}
		}
		return false;
	}

	struct FReadyTrace
	{
		EInitialTurnOrderPlayer Attacker = EInitialTurnOrderPlayer::None;
		EInitialTurnOrderPlayer Defender = EInitialTurnOrderPlayer::None;
		FName SkillId = NAME_None;
	};

	bool BuildReadyForResolution(
		AFMCodexLocalMatchHostGameMode& Host,
		FMatchPlayAuthoritativeSession& Direct,
		const FMatchPlayOpeningInitializeInput& Input,
		const FSkillRuleSnapshotSet& Rules,
		const int32 Seed,
		const ESkillRuleType SkillType,
		const bool bDeployGoalkeeper,
		FReadyTrace& OutTrace)
	{
		if (!Direct.InitializeMatch(Input).OpeningResult.bSuccess
			|| !Host.StartNewLocalMatch(Input, Rules, Seed).bSuccess
			|| !Direct.BeginOrdinaryAttack(6).BeginResult.bSuccess
			|| !Host.BeginOrdinaryAttack(6).bSuccess)
		{
			return false;
		}
		FMatchPlayState State = Host.GetMatchSnapshot().Snapshot;
		OutTrace.Attacker = State.RuntimeState.CurrentAttackingPlayer;
		OutTrace.Defender = OtherPlayer(OutTrace.Attacker);
		OutTrace.SkillId = Rules.SkillRules[0].SkillId;
		const int32 OrdinaryDeployments = bDeployGoalkeeper ? 5 : 4;
		for (int32 Index = 0; Index < OrdinaryDeployments; ++Index)
		{
			State = Host.GetMatchSnapshot().Snapshot;
			const bool bAttackerDeploying =
				State.CurrentAttack.CurrentLegalDeploymentSide
					== OutTrace.Attacker;
			FMatchPlayAuthoritativeDeployOrdinaryRequest Request;
			if (!FindDeployment(
				State,
				bAttackerDeploying
					? EMatchPlayRelativeDeploymentZone::Forward
					: EMatchPlayRelativeDeploymentZone::Backfield,
				Request)
				|| !Direct.DeployOrdinary(Request).DeploymentResult.bSuccess
				|| !Host.DeployOrdinary(Request).bSuccess)
			{
				return false;
			}
			if (Index == 0 && bDeployGoalkeeper)
			{
				State = Host.GetMatchSnapshot().Snapshot;
				const FName GoalkeeperCardId = FName(*(
					OutTrace.Defender == EInitialTurnOrderPlayer::PlayerA
						? State.RuntimeState.PlayerAState.GoalkeeperCardId
						: State.RuntimeState.PlayerBState.GoalkeeperCardId));
				const auto Availability =
					FMatchPlayGoalkeeperDeploymentAvailability::Query(
						State,
						State.CurrentAttack.AttackSequence,
						OutTrace.Defender,
						GoalkeeperCardId);
				if (Availability.LegalSlotIds.IsEmpty())
				{
					return false;
				}
				FMatchPlayAuthoritativeDeployGoalkeeperRequest GoalkeeperRequest;
				GoalkeeperRequest.SlotId = Availability.LegalSlotIds.Last();
				if (!Direct.DeployGoalkeeper(GoalkeeperRequest)
						.DeploymentResult.bSucceeded
					|| !Host.DeployGoalkeeper(GoalkeeperRequest).bSuccess)
				{
					return false;
				}
			}
		}
		for (int32 Index = 0; Index < 2; ++Index)
		{
			State = Host.GetMatchSnapshot().Snapshot;
			const int64 Sequence = State.CurrentAttack.AttackSequence;
			const EInitialTurnOrderPlayer Side =
				State.CurrentAttack.CurrentLegalDeploymentSide;
			if (!Direct.FinishDeployment(Sequence, Side).FinishResult.bSuccess
				|| !Host.FinishDeployment(Sequence, Side).bSuccess)
			{
				return false;
			}
		}

		State = Host.GetMatchSnapshot().Snapshot;
		const auto CarrierAvailability =
			FMatchPlayCurrentAttackCarrierSelectionAvailability::Query(
				State,
				State.CurrentAttack.AttackSequence,
				OutTrace.Attacker);
		FName CarrierId = NAME_None;
		for (const auto& Candidate : CarrierAvailability.Candidates)
		{
			if (Candidate.LegalityResult.bIsLegal)
			{
				CarrierId = Candidate.CarrierCardId;
				break;
			}
		}
		FMatchPlayAuthoritativeSubmitCarrierRequest CarrierRequest;
		CarrierRequest.RequestingSide = OutTrace.Attacker;
		CarrierRequest.CarrierCardId = CarrierId;
		if (CarrierId.IsNone()
			|| !Direct.SubmitCarrier(CarrierRequest).CarrierResult.bSuccess
			|| !Host.SubmitCarrier(CarrierRequest).bSuccess)
		{
			return false;
		}

		State = Host.GetMatchSnapshot().Snapshot;
		const auto MarkerAvailability =
			FMatchPlayCurrentAttackMarkerSelectionAvailability::Query(
				State,
				State.CurrentAttack.AttackSequence,
				OutTrace.Defender);
		FName MarkerId = NAME_None;
		for (const auto& Candidate : MarkerAvailability.Candidates)
		{
			if (Candidate.LegalityResult.bIsLegal)
			{
				MarkerId = Candidate.MarkerCardId;
				break;
			}
		}
		FMatchPlayAuthoritativeSubmitMarkerRequest MarkerRequest;
		MarkerRequest.RequestingSide = OutTrace.Defender;
		MarkerRequest.MarkerCardId = MarkerId;
		if (MarkerId.IsNone()
			|| !Direct.SubmitMarker(MarkerRequest).MarkerResult.bSuccess
			|| !Host.SubmitMarker(MarkerRequest).bSuccess)
		{
			return false;
		}

		FMatchPlayAuthoritativeSubmitSkillRequest SkillRequest;
		SkillRequest.RequestingSide = OutTrace.Attacker;
		SkillRequest.SkillId = OutTrace.SkillId;
		auto SubmitSkillToBoth = [&]()
		{
			return Direct.SubmitSkill(Rules, SkillRequest)
					.SkillResult.bSuccess
				&& Host.SubmitSkill(Rules, SkillRequest).bSuccess;
		};
		State = Host.GetMatchSnapshot().Snapshot;
		const auto RunnerAvailability =
			FMatchPlayCurrentAttackRunnerSelectionAvailability::Query(
				State,
				State.CurrentAttack.AttackSequence,
				OutTrace.Attacker);
		FName RunnerId = NAME_None;
		for (const auto& Candidate : RunnerAvailability.Candidates)
		{
			if (Candidate.LegalityResult.bIsLegal)
			{
				RunnerId = Candidate.RunnerCardId;
				break;
			}
		}
		FMatchPlayAuthoritativeSubmitRunnerRequest RunnerRequest;
		RunnerRequest.RequestingSide = OutTrace.Attacker;
		RunnerRequest.RunnerCardId = RunnerId;
		if (RunnerId.IsNone()
			|| !Direct.SubmitRunner(RunnerRequest).RunnerResult.bSuccess
			|| !Host.SubmitRunner(RunnerRequest).bSuccess)
		{
			return false;
		}

		if (bDeployGoalkeeper)
		{
			if (!Direct.ResolveNoLegalHelper().ResolutionResult.bSuccess
				|| !Host.ResolveNoLegalHelper().bSuccess)
			{
				return false;
			}
		}
		else
		{
			State = Host.GetMatchSnapshot().Snapshot;
			const auto HelperAvailability =
				FMatchPlayCurrentAttackHelperSelectionAvailability::Query(
					State,
					State.CurrentAttack.AttackSequence,
					OutTrace.Defender);
			FName HelperId = NAME_None;
			for (const auto& Candidate : HelperAvailability.Candidates)
			{
				if (Candidate.LegalityResult.bSuccess)
				{
					HelperId = Candidate.HelperCardId;
					break;
				}
			}
			FMatchPlayAuthoritativeSubmitHelperRequest HelperRequest;
			HelperRequest.RequestingSide = OutTrace.Defender;
			HelperRequest.HelperCardId = HelperId;
			if (HelperId.IsNone()
				|| !Direct.SubmitHelper(HelperRequest).HelperResult.bSuccess
				|| !Host.SubmitHelper(HelperRequest).bSuccess)
			{
				return false;
			}
		}

		if (!SubmitSkillToBoth())
		{
			return false;
		}

		if (SkillType == ESkillRuleType::Cross)
		{
			FMatchPlayAuthoritativeSubmitBranchIntentRequest IntentRequest;
			IntentRequest.RequestingSide = OutTrace.Attacker;
			IntentRequest.Intent = EMatchPlayElectiveBranchIntent::CrossHigh;
			if (!Direct.SubmitBranchIntent(IntentRequest).IntentResult.bSuccess
				|| !Host.SubmitBranchIntent(IntentRequest).bSuccess)
			{
				return false;
			}
		}

		return AreStatesEqual(
			Host.GetMatchSnapshot().Snapshot,
			Direct.GetStateSnapshot())
			&& Host.GetMatchSnapshot().Snapshot.CurrentAttack.SelectionStage
				== EMatchPlayCurrentAttackSelectionStage::ReadyForResolution;
	}

	int32 FindSeedForRolls(const TArray<int32>& Rolls)
	{
		for (int32 Seed = 0; Seed < 4000000; ++Seed)
		{
			FRandomStream Stream(Seed);
			bool bMatches = true;
			for (const int32 Expected : Rolls)
			{
				if (Stream.RandRange(1, 6) != Expected)
				{
					bMatches = false;
					break;
				}
			}
			if (bMatches)
			{
				return Seed;
			}
		}
		return INDEX_NONE;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexLocalMatchResolutionSurfaceTest,
	"FMCodex.LocalPlay.LocalMatchHost.06.ResolutionSurfaceAndConfiguration",
	EAutomationTestFlags::EditorContext
		| EAutomationTestFlags::EngineFilter)

bool FFMCodexLocalMatchResolutionSurfaceTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexLocalMatchResolutionRoutingTests;
	using FRouteMethod = FFMCodexLocalMatchResolveInitialRouteResult
		(AFMCodexLocalMatchHostGameMode::*)();
	using FFormulaMethod =
		FFMCodexLocalMatchResolveSingleCardFinishingFormulaResult
		(AFMCodexLocalMatchHostGameMode::*)();
	using FTerminalMethod =
		FFMCodexLocalMatchApplyThroughBallTerminalResolutionResult
		(AFMCodexLocalMatchHostGameMode::*)();
	static_assert(std::is_same_v<
		decltype(&AFMCodexLocalMatchHostGameMode::ResolveInitialRoute),
		FRouteMethod>);
	static_assert(std::is_same_v<
		decltype(&AFMCodexLocalMatchHostGameMode
			::ResolveSingleCardFinishingFormula),
		FFormulaMethod>);
	static_assert(std::is_same_v<
		decltype(&AFMCodexLocalMatchHostGameMode
			::ApplyThroughBallTerminalResolution),
		FTerminalMethod>);

	FString Header;
	FString Source;
	FString SessionTypes;
	TestTrue(TEXT("Host header loads"), LoadSource(
		TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchHostGameMode.h"),
		Header));
	TestTrue(TEXT("Host source loads"), LoadSource(
		TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchHostGameMode.cpp"),
		Source));
	TestTrue(TEXT("Session command enum loads"), LoadSource(
		TEXT("Source/FMCodex/MatchPlayRuntime/MatchPlayAuthoritativeSessionTypes.h"),
		SessionTypes));

	struct FReachability
	{
		const TCHAR* SessionCommand;
		const TCHAR* HostRoute;
	};
	const FReachability Commands[] = {
		{ TEXT("InitializeMatch"), TEXT("StartNewLocalMatch(") },
		{ TEXT("BeginOrdinaryAttack"), TEXT("BeginOrdinaryAttack(") },
		{ TEXT("FinishDeployment"), TEXT("FinishDeployment(") },
		{ TEXT("DeployOrdinary"), TEXT("DeployOrdinary(") },
		{ TEXT("SubmitCarrier"), TEXT("SubmitCarrier(") },
		{ TEXT("SubmitMarker"), TEXT("SubmitMarker(") },
		{ TEXT("ResolveNoLegalMarker"), TEXT("ResolveNoLegalMarker(") },
		{ TEXT("DeclineMarker"), TEXT("DeclineMarker(") },
		{ TEXT("SubmitSkill"), TEXT("SubmitSkill(") },
		{ TEXT("ResolveNoLegalSkill"), TEXT("ResolveNoLegalSkill(") },
		{ TEXT("DeclineSkill"), TEXT("DeclineSkill(") },
		{ TEXT("SubmitRunner"), TEXT("SubmitRunner(") },
		{ TEXT("ResolveNoLegalRunner"), TEXT("ResolveNoLegalRunner(") },
		{ TEXT("DeclineRunner"), TEXT("DeclineRunner(") },
		{ TEXT("SubmitHelper"), TEXT("SubmitHelper(") },
		{ TEXT("ResolveNoLegalHelper"), TEXT("ResolveNoLegalHelper(") },
		{ TEXT("DeclineHelper"), TEXT("DeclineHelper(") },
		{ TEXT("BeginResolutionSession"), TEXT("BeginResolutionSession(") },
		{ TEXT("SubmitBranchIntent"), TEXT("SubmitBranchIntent(") },
		{ TEXT("ResolveIntentDeterminedRoute"), TEXT("ResolveIntentDeterminedRoute(") },
		{ TEXT("ResolveInitialRoute"), TEXT("ResolveInitialRoute(") },
		{ TEXT("ResolveCrossPostRoutePlan"), TEXT("ResolveCrossPostRoutePlan(") },
		{ TEXT("ResolveThroughBallFeetPostRoutePlan"), TEXT("ResolveThroughBallFeetPostRoutePlan(") },
		{ TEXT("ResolvePassControlPostRoutePlan"), TEXT("ResolvePassControlPostRoutePlan(") },
		{ TEXT("ResolveDeadCornerPostRouteDecision"), TEXT("ResolveDeadCornerPostRouteDecision(") },
		{ TEXT("ResolveThroughBallAntiOffsideDecision"), TEXT("ResolveThroughBallAntiOffsideDecision(") },
		{ TEXT("ResolveDirectShotPostRouteDecisionOrPlan"), TEXT("ResolveDirectShotPostRouteDecisionOrPlan(") },
		{ TEXT("ResolveThroughBallBehindDefenseP1DecisionOrPlan"), TEXT("ResolveThroughBallBehindDefenseP1DecisionOrPlan(") },
		{ TEXT("ResolveSingleCardFinishingFormula"), TEXT("ResolveSingleCardFinishingFormula(") },
		{ TEXT("ResolveThroughBallFeetFormula"), TEXT("ResolveThroughBallFeetFormula(") },
		{ TEXT("ResolveThroughBallBehindDefenseP1Formula"), TEXT("ResolveThroughBallBehindDefenseP1Formula(") },
		{ TEXT("ResolveThroughBallBehindDefenseP2Decision"), TEXT("ResolveThroughBallBehindDefenseP2Decision(") },
		{ TEXT("SubmitThroughBallOneOnOneShotChoice"), TEXT("SubmitThroughBallOneOnOneShotChoice(") },
		{ TEXT("ResolveThroughBallOneOnOneChipShotDecision"), TEXT("ResolveThroughBallOneOnOneChipShotDecision(") },
		{ TEXT("ResolveThroughBallOneOnOneDirectShotPostRoutePlan"), TEXT("ResolveThroughBallOneOnOneDirectShotPostRoutePlan(") },
		{ TEXT("ResolveThroughBallOneOnOneDirectShotFormula"), TEXT("ResolveThroughBallOneOnOneDirectShotFormula(") },
		{ TEXT("ApplyThroughBallTerminalResolution"), TEXT("ApplyThroughBallTerminalResolution(") },
		{ TEXT("ApplyCrossTerminalResolution"), TEXT("ApplyCrossTerminalResolution(") },
		{ TEXT("ApplyPassControlTerminalResolution"), TEXT("ApplyPassControlTerminalResolution(") },
		{ TEXT("ApplyShotTerminalResolution"), TEXT("ApplyShotTerminalResolution(") },
		{ TEXT("DeployGoalkeeper"), TEXT("DeployGoalkeeper(") },
		{ TEXT("ResolveNoLegalCarrier"), TEXT("ResolveNoLegalCarrier(") }
	};
	TestEqual(TEXT("Session mutation inventory remains 42"),
		static_cast<int32>(UE_ARRAY_COUNT(Commands)), 42);
	for (const FReachability& Command : Commands)
	{
		TestTrue(*FString::Printf(
			TEXT("Session enum contains %s"), Command.SessionCommand),
			SessionTypes.Contains(Command.SessionCommand));
		TestTrue(*FString::Printf(
			TEXT("Host typed route exists for %s"), Command.SessionCommand),
			Header.Contains(Command.HostRoute));
	}
	for (const TCHAR* Delegation : {
		TEXT(".BeginResolutionSession("),
		TEXT(".ResolveIntentDeterminedRoute("),
		TEXT(".ResolveInitialRoute("),
		TEXT(".ResolveCrossPostRoutePlan("),
		TEXT(".ResolveThroughBallFeetPostRoutePlan("),
		TEXT(".ResolvePassControlPostRoutePlan("),
		TEXT(".ResolveDeadCornerPostRouteDecision("),
		TEXT(".ResolveThroughBallAntiOffsideDecision("),
		TEXT(".ResolveDirectShotPostRouteDecisionOrPlan("),
		TEXT(".ResolveThroughBallBehindDefenseP1DecisionOrPlan("),
		TEXT(".ResolveSingleCardFinishingFormula("),
		TEXT(".ResolveThroughBallFeetFormula("),
		TEXT(".ResolveThroughBallBehindDefenseP1Formula("),
		TEXT(".ResolveThroughBallBehindDefenseP2Decision("),
		TEXT(".ResolveThroughBallOneOnOneChipShotDecision("),
		TEXT(".ResolveThroughBallOneOnOneDirectShotPostRoutePlan("),
		TEXT(".ResolveThroughBallOneOnOneDirectShotFormula("),
		TEXT(".ApplyThroughBallTerminalResolution("),
		TEXT(".ApplyCrossTerminalResolution("),
		TEXT(".ApplyPassControlTerminalResolution("),
		TEXT(".ApplyShotTerminalResolution(") })
	{
		TestEqual(*FString::Printf(
			TEXT("Exactly one Session delegation exists: %s"), Delegation),
			CountOccurrences(Source, Delegation),
			1);
	}
	for (const TCHAR* Forbidden : {
		TEXT("ExecuteResolution("),
		TEXT("ExecuteCommand("),
		TEXT("Dispatch("),
		TEXT("FName Command"),
		TEXT("ProcessEvent(") })
	{
		TestFalse(*FString::Printf(TEXT("No generic dispatch: %s"), Forbidden),
			Header.Contains(Forbidden) || Source.Contains(Forbidden));
	}
	for (const TCHAR* Cache : {
		TEXT("LastRoll"), TEXT("CurrentDice"), TEXT("RouteRoll"),
		TEXT("AllRolls"), TEXT("CachedResolution"), TEXT("CachedOutcome") })
	{
		TestFalse(*FString::Printf(TEXT("No duplicate cache: %s"), Cache),
			Header.Contains(Cache) || Source.Contains(Cache));
	}

	FScopedWorld EmptyWorld;
	auto* EmptyHost = EmptyWorld.GetHost();
	TestNotNull(TEXT("Empty Host exists"), EmptyHost);
	if (EmptyHost == nullptr)
	{
		return false;
	}
	auto TestNoActive = [this](const TCHAR* Label, const auto& Result)
	{
		TestFalse(*FString::Printf(TEXT("%s fails without match"), Label),
			Result.bSuccess);
		TestEqual(*FString::Printf(TEXT("%s host error"), Label),
			Result.ErrorCode,
			EFMCodexLocalMatchHostErrorCode::NoActiveMatch);
		TestEqual(*FString::Printf(TEXT("%s Session not called"), Label),
			Result.AuthoritativeResult.RuntimeEnvelope.CommandKind,
			EMatchPlayAuthoritativeCommandKind::None);
	};
	TestNoActive(TEXT("BeginResolutionSession"), EmptyHost->BeginResolutionSession());
	TestNoActive(TEXT("ResolveIntentDeterminedRoute"), EmptyHost->ResolveIntentDeterminedRoute());
	TestNoActive(TEXT("ResolveInitialRoute"), EmptyHost->ResolveInitialRoute());
	TestNoActive(TEXT("ResolveCrossPostRoutePlan"), EmptyHost->ResolveCrossPostRoutePlan());
	FMatchPlayAuthoritativeResolveCrossLowAttackRollRequest LowAttackRequest;
	LowAttackRequest.RequestingSide = EInitialTurnOrderPlayer::PlayerA;
	TestNoActive(TEXT("ResolveCrossLowAttackRoll"),
		EmptyHost->ResolveCrossLowAttackRoll(LowAttackRequest));
	FMatchPlayAuthoritativeResolveCrossLowDefenseRollRequest LowDefenseRequest;
	LowDefenseRequest.RequestingSide = EInitialTurnOrderPlayer::PlayerB;
	TestNoActive(TEXT("ResolveCrossLowDefenseRoll"),
		EmptyHost->ResolveCrossLowDefenseRoll(LowDefenseRequest));
	TestNoActive(TEXT("ResolveThroughBallFeetPostRoutePlan"), EmptyHost->ResolveThroughBallFeetPostRoutePlan());
	TestNoActive(TEXT("ResolvePassControlPostRoutePlan"), EmptyHost->ResolvePassControlPostRoutePlan());
	TestNoActive(TEXT("ResolveDeadCornerPostRouteDecision"), EmptyHost->ResolveDeadCornerPostRouteDecision());
	TestNoActive(TEXT("ResolveThroughBallAntiOffsideDecision"), EmptyHost->ResolveThroughBallAntiOffsideDecision());
	TestNoActive(TEXT("ResolveDirectShotPostRouteDecisionOrPlan"), EmptyHost->ResolveDirectShotPostRouteDecisionOrPlan());
	TestNoActive(TEXT("ResolveThroughBallBehindDefenseP1DecisionOrPlan"), EmptyHost->ResolveThroughBallBehindDefenseP1DecisionOrPlan());
	TestNoActive(TEXT("ResolveSingleCardFinishingFormula"), EmptyHost->ResolveSingleCardFinishingFormula());
	TestNoActive(TEXT("ResolveThroughBallFeetFormula"), EmptyHost->ResolveThroughBallFeetFormula());
	TestNoActive(TEXT("ResolveThroughBallBehindDefenseP1Formula"), EmptyHost->ResolveThroughBallBehindDefenseP1Formula());
	TestNoActive(TEXT("ResolveThroughBallBehindDefenseP2Decision"), EmptyHost->ResolveThroughBallBehindDefenseP2Decision());
	TestNoActive(TEXT("ResolveThroughBallOneOnOneChipShotDecision"), EmptyHost->ResolveThroughBallOneOnOneChipShotDecision());
	TestNoActive(TEXT("ResolveThroughBallOneOnOneDirectShotPostRoutePlan"), EmptyHost->ResolveThroughBallOneOnOneDirectShotPostRoutePlan());
	TestNoActive(TEXT("ResolveThroughBallOneOnOneDirectShotFormula"), EmptyHost->ResolveThroughBallOneOnOneDirectShotFormula());
	TestNoActive(TEXT("ApplyThroughBallTerminalResolution"), EmptyHost->ApplyThroughBallTerminalResolution());
	TestNoActive(TEXT("ApplyCrossTerminalResolution"), EmptyHost->ApplyCrossTerminalResolution());
	TestNoActive(TEXT("ApplyPassControlTerminalResolution"), EmptyHost->ApplyPassControlTerminalResolution());
	TestNoActive(TEXT("ApplyShotTerminalResolution"), EmptyHost->ApplyShotTerminalResolution());

	FScopedWorld ConfigWorld;
	auto* ConfigHost = ConfigWorld.GetHost();
	const FName SkillId(TEXT("Skill.Configuration.Cross"));
	const auto Rules = MakeRules(SkillId, ESkillRuleType::Cross);
	TestTrue(TEXT("Configured match starts"), ConfigHost != nullptr
		&& ConfigHost->StartNewLocalMatch(MakeInput(TEXT("Config"), SkillId), Rules, 9)
			.bSuccess);
	if (ConfigHost == nullptr)
	{
		return false;
	}
	const FMatchPlayState ConfigBefore = ConfigHost->GetMatchSnapshot().Snapshot;
	auto MismatchedRules = Rules;
	MismatchedRules.SkillRules[0].MaxTriggerActionPoint = 7;
	FMatchPlayAuthoritativeSubmitSkillRequest SkillRequest;
	SkillRequest.RequestingSide = EInitialTurnOrderPlayer::PlayerA;
	SkillRequest.SkillId = SkillId;
	const auto Mismatch = ConfigHost->SubmitSkill(MismatchedRules, SkillRequest);
	TestEqual(TEXT("Rule mismatch has explicit Host error"),
		Mismatch.ErrorCode,
		EFMCodexLocalMatchHostErrorCode::RuleConfigurationMismatch);
	TestEqual(TEXT("Rule mismatch does not call Session"),
		Mismatch.AuthoritativeResult.RuntimeEnvelope.CommandKind,
		EMatchPlayAuthoritativeCommandKind::None);
	TestTrue(TEXT("Rule mismatch preserves State"), AreStatesEqual(
		ConfigHost->GetMatchSnapshot().Snapshot, ConfigBefore));

	TestTrue(TEXT("Attack begins for wrong-stage probes"),
		ConfigHost->BeginOrdinaryAttack(6).bSuccess);
	const FMatchPlayState WrongStageBefore =
		ConfigHost->GetMatchSnapshot().Snapshot;
	const auto EarlyBegin = ConfigHost->BeginResolutionSession();
	TestFalse(TEXT("BeginResolutionSession too early is preserved"),
		EarlyBegin.bSuccess);
	TestEqual(TEXT("Early Begin command kind"),
		EarlyBegin.AuthoritativeResult.RuntimeEnvelope.CommandKind,
		EMatchPlayAuthoritativeCommandKind::BeginResolutionSession);
	const auto EarlyRoute = ConfigHost->ResolveInitialRoute();
	TestFalse(TEXT("ResolveInitialRoute on wrong state is preserved"),
		EarlyRoute.bSuccess);
	const auto EarlyFormula = ConfigHost->ResolveSingleCardFinishingFormula();
	TestFalse(TEXT("Formula before plan is preserved"), EarlyFormula.bSuccess);
	const auto EarlyTerminal = ConfigHost->ApplyShotTerminalResolution();
	TestFalse(TEXT("Terminal before semantic is preserved"),
		EarlyTerminal.bSuccess);
	TestTrue(TEXT("Wrong-stage probes preserve State and consume no RNG"),
		AreStatesEqual(
			ConfigHost->GetMatchSnapshot().Snapshot,
			WrongStageBefore));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexLocalMatchCrossResolutionTest,
	"FMCodex.LocalPlay.LocalMatchHost.07.CrossFullResolution",
	EAutomationTestFlags::EditorContext
		| EAutomationTestFlags::EngineFilter)

bool FFMCodexLocalMatchCrossResolutionTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexLocalMatchResolutionRoutingTests;
	const int32 Seed = 12054;
	const FName SkillId(TEXT("Skill.Host.Cross"));
	const auto Rules = MakeRules(SkillId, ESkillRuleType::Cross);
	const auto Input = MakeInput(TEXT("HostCross"), SkillId);
	FFMCodexLocalMatchD6Provider DirectProvider(Seed);
	FMatchPlayAuthoritativeSession Direct(DirectProvider, DirectProvider, Rules);
	FScopedWorld World;
	auto* Host = World.GetHost();
	TestNotNull(TEXT("Cross Host exists"), Host);
	if (Host == nullptr)
	{
		return false;
	}
	FReadyTrace Trace;
	TestTrue(TEXT("Cross Host/direct reach ReadyForResolution"),
		BuildReadyForResolution(
			*Host, Direct, Input, Rules, Seed,
			ESkillRuleType::Cross, false, Trace));
	const FMatchPlayState Ready = Host->GetMatchSnapshot().Snapshot;
	TestFalse(TEXT("Ready snapshot has no resolution cache/session"),
		Ready.CurrentAttack.bHasResolutionSession);

	TestTrue(TEXT("Direct begins resolution"),
		Direct.BeginResolutionSession().BeginResult.bSuccess);
	TestTrue(TEXT("Host begins resolution"),
		Host->BeginResolutionSession().bSuccess);
	TestTrue(TEXT("Begin snapshot is fresh/equivalent"), AreStatesEqual(
		Host->GetMatchSnapshot().Snapshot, Direct.GetStateSnapshot()));
	TestTrue(TEXT("Direct resolves Cross route"),
		Direct.ResolveInitialRoute().OrchestrationResult.bSuccess);
	const auto HostRoute = Host->ResolveInitialRoute();
	TestTrue(TEXT("Host resolves Cross route"), HostRoute.bSuccess);
	TestTrue(TEXT("Route snapshot equals direct Session"), AreStatesEqual(
		Host->GetMatchSnapshot().Snapshot, Direct.GetStateSnapshot()));

	const FMatchPlayState BeforeWrongFamily = Host->GetMatchSnapshot().Snapshot;
	const auto WrongFamily = Host->ResolvePassControlPostRoutePlan();
	TestFalse(TEXT("PassControl resolver rejects Cross branch"),
		WrongFamily.bSuccess);
	TestEqual(TEXT("Wrong-family typed command kind preserved"),
		WrongFamily.AuthoritativeResult.RuntimeEnvelope.CommandKind,
		EMatchPlayAuthoritativeCommandKind::ResolvePassControlPostRoutePlan);
	TestTrue(TEXT("Wrong-family command preserves State"), AreStatesEqual(
		Host->GetMatchSnapshot().Snapshot, BeforeWrongFamily));

	FMatchPlayAuthoritativeResolveCrossHighAttackRollRequest AttackRollRequest;
	AttackRollRequest.RequestingSide = Trace.Attacker;
	TestTrue(TEXT("Direct accepts the explicit Cross High attack roll"),
		Direct.ResolveCrossHighAttackRoll(AttackRollRequest)
			.OrchestrationResult.bSuccess);
	const auto HostAttackRoll = Host->ResolveCrossHighAttackRoll(
		AttackRollRequest);
	TestTrue(TEXT("Host accepts the explicit Cross High attack roll"),
		HostAttackRoll.bSuccess);
	TestTrue(TEXT("Attack-roll snapshot equals direct Session"), AreStatesEqual(
		Host->GetMatchSnapshot().Snapshot, Direct.GetStateSnapshot()));

	FMatchPlayAuthoritativeResolveCrossHighDefenseRollRequest DefenseRollRequest;
	DefenseRollRequest.RequestingSide = Trace.Defender;
	TestTrue(TEXT("Direct accepts the explicit Cross High defense roll"),
		Direct.ResolveCrossHighDefenseRoll(DefenseRollRequest)
			.OrchestrationResult.bSuccess);
	const auto HostPlan = Host->ResolveCrossHighDefenseRoll(
		DefenseRollRequest);
	TestTrue(TEXT("Host accepts the explicit Cross High defense roll"),
		HostPlan.bSuccess);
	const FMatchPlayState Planned = Host->GetMatchSnapshot().Snapshot;
	TestEqual(TEXT("Cross consumes two canonical post-route rolls"),
		Planned.CurrentAttack.ResolutionSession.PostRouteRollProgress
			.RollRecords.Num(),
		2);
	TestTrue(TEXT("Post-route snapshot equals direct Session"), AreStatesEqual(
		Planned, Direct.GetStateSnapshot()));

	TestTrue(TEXT("Direct resolves Cross Formula"),
		Direct.ResolveSingleCardFinishingFormula()
			.OrchestrationResult.bSuccess);
	const auto HostFormula = Host->ResolveSingleCardFinishingFormula();
	TestTrue(TEXT("Host resolves Cross Formula"), HostFormula.bSuccess);
	TestTrue(TEXT("Formula snapshot equals direct Session"), AreStatesEqual(
		Host->GetMatchSnapshot().Snapshot, Direct.GetStateSnapshot()));

	const FMatchPlayState BeforeTerminal = Host->GetMatchSnapshot().Snapshot;
	const int32 ScoreBefore = ScoreFor(BeforeTerminal, Trace.Attacker);
	const int32 UsedBefore = UsedAttacksFor(BeforeTerminal, Trace.Attacker);
	const auto DirectTerminal = Direct.ApplyCrossTerminalResolution();
	const auto HostTerminal = Host->ApplyCrossTerminalResolution();
	TestTrue(TEXT("Cross terminal succeeds through Host/direct"),
		DirectTerminal.OrchestrationResult.bSuccess
			&& HostTerminal.bSuccess);
	const FMatchPlayState Completed = Host->GetMatchSnapshot().Snapshot;
	TestFalse(TEXT("Cross terminal clears CurrentAttack"),
		Completed.bHasCurrentAttack);
	TestEqual(TEXT("Cross terminal consumes one opportunity"),
		UsedAttacksFor(Completed, Trace.Attacker) - UsedBefore,
		1);
	TestEqual(TEXT("Cross score matches authoritative result"),
		ScoreFor(Completed, Trace.Attacker) - ScoreBefore,
		HostTerminal.AuthoritativeResult.OrchestrationResult.bIsGoal ? 1 : 0);
	TestEqual(TEXT("Cross next attacker is canonical"),
		Completed.RuntimeState.CurrentAttackingPlayer,
		HostTerminal.AuthoritativeResult.OrchestrationResult
			.CompletionResult.NextAttackingPlayer);
	TestTrue(TEXT("Terminal State equals direct Session"), AreStatesEqual(
		Completed, Direct.GetStateSnapshot()));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexLocalMatchThroughBallResolutionTest,
	"FMCodex.LocalPlay.LocalMatchHost.08.ThroughBallDeepResolutionAndIsolation",
	EAutomationTestFlags::EditorContext
		| EAutomationTestFlags::EngineFilter)

bool FFMCodexLocalMatchThroughBallResolutionTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexLocalMatchResolutionRoutingTests;
	const int32 Seed = FindSeedForRolls({ 3, 6, 1, 2, 1, 6 });
	TestTrue(TEXT("A deterministic local-provider seed exists"),
		Seed != INDEX_NONE);
	if (Seed == INDEX_NONE)
	{
		return false;
	}
	const FName SkillId(TEXT("Skill.Host.ThroughBall"));
	const auto Rules = MakeRules(SkillId, ESkillRuleType::ThroughBall);
	const auto Input = MakeInput(TEXT("HostThroughBall"), SkillId);
	FFMCodexLocalMatchD6Provider DirectProvider(Seed);
	FMatchPlayAuthoritativeSession Direct(DirectProvider, DirectProvider, Rules);
	FScopedWorld WorldA;
	FScopedWorld WorldB;
	auto* HostA = WorldA.GetHost();
	auto* HostB = WorldB.GetHost();
	TestNotNull(TEXT("ThroughBall Host A exists"), HostA);
	TestNotNull(TEXT("ThroughBall Host B exists"), HostB);
	if (HostA == nullptr || HostB == nullptr)
	{
		return false;
	}
	FReadyTrace TraceA;
	FReadyTrace TraceB;
	TestTrue(TEXT("Host A/direct reach deep-ready state"),
		BuildReadyForResolution(
			*HostA, Direct, Input, Rules, Seed,
			ESkillRuleType::ThroughBall, false, TraceA));
	FFMCodexLocalMatchD6Provider OracleBProvider(Seed);
	FMatchPlayAuthoritativeSession OracleB(
		OracleBProvider, OracleBProvider, Rules);
	TestTrue(TEXT("Host B independently reaches same ready state"),
		BuildReadyForResolution(
			*HostB, OracleB, Input, Rules, Seed,
			ESkillRuleType::ThroughBall, false, TraceB));

	TestTrue(TEXT("Direct begins ThroughBall resolution"),
		Direct.BeginResolutionSession().BeginResult.bSuccess);
	TestTrue(TEXT("Host A begins ThroughBall resolution"),
		HostA->BeginResolutionSession().bSuccess);
	TestTrue(TEXT("Host B begins ThroughBall resolution"),
		HostB->BeginResolutionSession().bSuccess);
	const FMatchPlayState HostBBeforeRoute =
		HostB->GetMatchSnapshot().Snapshot;
	TestTrue(TEXT("Direct resolves initial route"),
		Direct.ResolveInitialRoute().OrchestrationResult.bSuccess);
	const auto RouteA = HostA->ResolveInitialRoute();
	TestTrue(TEXT("Host A consumes its InitialRoute provider"),
		RouteA.bSuccess
			&& RouteA.AuthoritativeResult.OrchestrationResult.bProviderCalled);
	const FMatchPlayState AfterRouteA = HostA->GetMatchSnapshot().Snapshot;
	TestEqual(TEXT("Accepted initial roll is State-owned"),
		AfterRouteA.CurrentAttack.ResolutionSession
			.InitialRouteRollRecords.Num(),
		1);
	TestEqual(TEXT("Deterministic route is BehindDefense"),
		AfterRouteA.CurrentAttack.ResolutionSession.ActualBranch.ThroughBall,
		EMatchPlayThroughBallActualBranch::BehindDefense);
	TestTrue(TEXT("Route State equals direct Session"), AreStatesEqual(
		AfterRouteA, Direct.GetStateSnapshot()));
	TestTrue(TEXT("Host A RNG does not mutate Host B"), AreStatesEqual(
		HostB->GetMatchSnapshot().Snapshot, HostBBeforeRoute));
	TestTrue(TEXT("Host B later consumes its unaffected provider stream"),
		HostB->ResolveInitialRoute().bSuccess);
	TestTrue(TEXT("Equal isolated seeds produce equal route State"),
		AreStatesEqual(
			HostB->GetMatchSnapshot().Snapshot,
			AfterRouteA));

	TestTrue(TEXT("Direct resolves BehindDefense P1 plan"),
		Direct.ResolveThroughBallBehindDefenseP1DecisionOrPlan()
			.OrchestrationResult.bSuccess);
	const auto P1 = HostA->ResolveThroughBallBehindDefenseP1DecisionOrPlan();
	TestTrue(TEXT("Host resolves BehindDefense P1 plan"), P1.bSuccess);
	TestEqual(TEXT("P1 consumes two canonical post-route rolls"),
		HostA->GetMatchSnapshot().Snapshot.CurrentAttack.ResolutionSession
			.PostRouteRollProgress.RollRecords.Num(),
		2);
	TestTrue(TEXT("P1 State equals direct Session"), AreStatesEqual(
		HostA->GetMatchSnapshot().Snapshot, Direct.GetStateSnapshot()));

	TestTrue(TEXT("Direct resolves P1 Formula"),
		Direct.ResolveThroughBallBehindDefenseP1Formula()
			.OrchestrationResult.bSuccess);
	const auto P1Formula =
		HostA->ResolveThroughBallBehindDefenseP1Formula();
	TestTrue(TEXT("Host resolves P1 Formula"), P1Formula.bSuccess);
	TestTrue(TEXT("P1 Formula State equals direct Session"), AreStatesEqual(
		HostA->GetMatchSnapshot().Snapshot, Direct.GetStateSnapshot()));

	TestTrue(TEXT("Direct resolves P2"),
		Direct.ResolveThroughBallBehindDefenseP2Decision()
			.OrchestrationResult.bSuccess);
	const auto P2 = HostA->ResolveThroughBallBehindDefenseP2Decision();
	TestTrue(TEXT("Host resolves P2"), P2.bSuccess);
	TestEqual(TEXT("P2 requires OneOnOne"),
		P2.AuthoritativeResult.OrchestrationResult.QueryResult.Decision,
		EThroughBallBehindDefenseP2OutcomeDecision::OneOnOneRequired);

	FMatchPlayAuthoritativeSubmitThroughBallOneOnOneShotChoiceRequest Choice;
	Choice.RequestingSide = TraceA.Attacker;
	Choice.Choice = EMatchPlayThroughBallOneOnOneShotChoice::DirectShot;
	TestTrue(TEXT("Direct accepts DirectShot choice"),
		Direct.SubmitThroughBallOneOnOneShotChoice(Choice)
			.ChoiceResult.bSuccess);
	TestTrue(TEXT("Host accepts DirectShot choice"),
		HostA->SubmitThroughBallOneOnOneShotChoice(Choice).bSuccess);
	TestEqual(TEXT("Choice is immediately visible in snapshot"),
		HostA->GetMatchSnapshot().Snapshot.CurrentAttack.ResolutionSession
			.ThroughBallOneOnOneShotChoice,
		EMatchPlayThroughBallOneOnOneShotChoice::DirectShot);

	TestTrue(TEXT("Direct resolves DirectShot plan"),
		Direct.ResolveThroughBallOneOnOneDirectShotPostRoutePlan()
			.OrchestrationResult.bSuccess);
	const auto DirectShotPlan =
		HostA->ResolveThroughBallOneOnOneDirectShotPostRoutePlan();
	TestTrue(TEXT("Host resolves DirectShot plan"), DirectShotPlan.bSuccess);
	const FMatchPlayState AfterDirectPlan =
		HostA->GetMatchSnapshot().Snapshot;
	TestEqual(TEXT("Deep path records five post-route rolls"),
		AfterDirectPlan.CurrentAttack.ResolutionSession
			.PostRouteRollProgress.RollRecords.Num(),
		5);
	TestTrue(TEXT("DirectShot plan State equals direct Session"),
		AreStatesEqual(AfterDirectPlan, Direct.GetStateSnapshot()));

	TestTrue(TEXT("Direct resolves DirectShot Formula"),
		Direct.ResolveThroughBallOneOnOneDirectShotFormula()
			.OrchestrationResult.bSuccess);
	const auto DirectShotFormula =
		HostA->ResolveThroughBallOneOnOneDirectShotFormula();
	TestTrue(TEXT("Host resolves DirectShot Formula"),
		DirectShotFormula.bSuccess);
	TestTrue(TEXT("DirectShot Formula State equals direct Session"),
		AreStatesEqual(
			HostA->GetMatchSnapshot().Snapshot,
			Direct.GetStateSnapshot()));

	const FMatchPlayState BeforeTerminal =
		HostA->GetMatchSnapshot().Snapshot;
	const int32 ScoreBefore = ScoreFor(BeforeTerminal, TraceA.Attacker);
	const int32 UsedBefore = UsedAttacksFor(BeforeTerminal, TraceA.Attacker);
	const auto DirectTerminal = Direct.ApplyThroughBallTerminalResolution();
	const auto HostTerminal = HostA->ApplyThroughBallTerminalResolution();
	TestTrue(TEXT("ThroughBall terminal succeeds"),
		DirectTerminal.OrchestrationResult.bSuccess
			&& HostTerminal.bSuccess);
	const FMatchPlayState Completed = HostA->GetMatchSnapshot().Snapshot;
	TestFalse(TEXT("ThroughBall terminal clears CurrentAttack"),
		Completed.bHasCurrentAttack);
	TestEqual(TEXT("ThroughBall terminal consumes one opportunity"),
		UsedAttacksFor(Completed, TraceA.Attacker) - UsedBefore,
		1);
	TestEqual(TEXT("ThroughBall score matches authoritative semantic"),
		ScoreFor(Completed, TraceA.Attacker) - ScoreBefore,
		HostTerminal.AuthoritativeResult.OrchestrationResult.bIsGoal ? 1 : 0);
	TestTrue(TEXT("ThroughBall terminal State equals direct Session"),
		AreStatesEqual(Completed, Direct.GetStateSnapshot()));

	const int32 ReplayScore = ScoreFor(Completed, TraceA.Attacker);
	const int32 ReplayUsed = UsedAttacksFor(Completed, TraceA.Attacker);
	const auto Replay = HostA->ApplyThroughBallTerminalResolution();
	TestFalse(TEXT("Terminal replay rejects"), Replay.bSuccess);
	TestEqual(TEXT("Terminal replay preserves NoCurrentAttack"),
		Replay.AuthoritativeResult.OrchestrationResult.ErrorCode,
		EMatchPlayCurrentAttackApplyThroughBallTerminalResolutionErrorCode
			::NoCurrentAttack);
	TestTrue(TEXT("Terminal replay leaves State unchanged"), AreStatesEqual(
		HostA->GetMatchSnapshot().Snapshot, Completed));
	TestEqual(TEXT("Terminal replay score delta is zero"),
		ScoreFor(HostA->GetMatchSnapshot().Snapshot, TraceA.Attacker)
			- ReplayScore,
		0);
	TestEqual(TEXT("Terminal replay opportunity delta is zero"),
		UsedAttacksFor(HostA->GetMatchSnapshot().Snapshot, TraceA.Attacker)
			- ReplayUsed,
		0);
	return true;
}

#endif
