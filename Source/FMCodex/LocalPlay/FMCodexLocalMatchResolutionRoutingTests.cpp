#include "FMCodexLocalMatchHostGameMode.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "../CoreRules/MatchPlayCurrentAttackCarrierSelectionAvailability.h"
#include "../CoreRules/MatchPlayCurrentAttackHelperSelectionAvailability.h"
#include "../CoreRules/MatchPlayCurrentAttackMarkerSelectionAvailability.h"
#include "../CoreRules/MatchPlayCurrentAttackRunnerSelectionAvailability.h"
#include "../CoreRules/MatchPlayGoalkeeperDeploymentAvailability.h"
#include "../CoreRules/MatchPlayOrdinaryDeploymentAvailability.h"

#include "FMCodexLocalMatchInteractionView.h"
#include "FMCodexLocalMatchPlayerController.h"

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
				Controller =
					World->SpawnActor<AFMCodexLocalMatchPlayerController>();
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
		AFMCodexLocalMatchPlayerController* GetController() const
		{
			return Controller;
		}

	private:
		UWorld* World = nullptr;
		AFMCodexLocalMatchHostGameMode* Host = nullptr;
		AFMCodexLocalMatchPlayerController* Controller = nullptr;
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
		FReadyTrace& OutTrace,
		const EMatchPlayElectiveBranchIntent ShotIntent =
			EMatchPlayElectiveBranchIntent::DirectShot)
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

		if (SkillType == ESkillRuleType::Cross
			|| SkillType == ESkillRuleType::LongShot
			|| SkillType == ESkillRuleType::CutInsideShot)
		{
			FMatchPlayAuthoritativeSubmitBranchIntentRequest IntentRequest;
			IntentRequest.AttackSequence =
				Direct.GetStateSnapshot().CurrentAttack.AttackSequence;
			IntentRequest.RequestingSide = OutTrace.Attacker;
			IntentRequest.Intent = SkillType == ESkillRuleType::Cross
				? EMatchPlayElectiveBranchIntent::CrossHigh
				: ShotIntent;
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
	FString ControllerSource;
	TestTrue(TEXT("Host header loads"), LoadSource(
		TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchHostGameMode.h"),
		Header));
	TestTrue(TEXT("Host source loads"), LoadSource(
		TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchHostGameMode.cpp"),
		Source));
	TestTrue(TEXT("Session command enum loads"), LoadSource(
		TEXT("Source/FMCodex/MatchPlayRuntime/MatchPlayAuthoritativeSessionTypes.h"),
		SessionTypes));
	TestTrue(TEXT("Player controller source loads"), LoadSource(
		TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchPlayerController.cpp"),
		ControllerSource));
	TestFalse(TEXT("Production controller never requests legacy BehindDefense P2"),
		ControllerSource.Contains(
			TEXT("ResolveThroughBallBehindDefenseP2Decision(")));

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
		{ TEXT("ResolveThroughBallInitialRouteRoll"), TEXT("ResolveThroughBallInitialRouteRoll(") },
		{ TEXT("ResolveCrossPostRoutePlan"), TEXT("ResolveCrossPostRoutePlan(") },
		{ TEXT("ResolveThroughBallFeetPostRoutePlan"), TEXT("ResolveThroughBallFeetPostRoutePlan(") },
		{ TEXT("ResolveThroughBallFeetAttackRoll"), TEXT("ResolveThroughBallFeetAttackRoll(") },
		{ TEXT("ResolveThroughBallFeetDefenseRoll"), TEXT("ResolveThroughBallFeetDefenseRoll(") },
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
	TestEqual(TEXT("Session mutation inventory remains 45"),
		static_cast<int32>(UE_ARRAY_COUNT(Commands)), 45);
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
		TEXT(".ResolveThroughBallInitialRouteRoll("),
		TEXT(".ResolveCrossPostRoutePlan("),
		TEXT(".ResolveThroughBallFeetPostRoutePlan("),
		TEXT(".ResolveThroughBallFeetAttackRoll("),
		TEXT(".ResolveThroughBallFeetDefenseRoll("),
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
		TEXT("LastRoll"), TEXT("CurrentDice"), TEXT("CachedRouteRoll"),
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
	FMatchPlayAuthoritativeResolveThroughBallInitialRouteRollRequest
		ThroughBallRouteRequest;
	ThroughBallRouteRequest.AttackSequence = 1;
	ThroughBallRouteRequest.RequestingSide = EInitialTurnOrderPlayer::PlayerA;
	TestNoActive(TEXT("ResolveThroughBallInitialRouteRoll"),
		EmptyHost->ResolveThroughBallInitialRouteRoll(ThroughBallRouteRequest));
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
	FMatchPlayAuthoritativeResolveThroughBallFeetAttackRollRequest
		FeetAttackRequest;
	FeetAttackRequest.RequestingSide = EInitialTurnOrderPlayer::PlayerA;
	TestNoActive(TEXT("ResolveThroughBallFeetAttackRoll"),
		EmptyHost->ResolveThroughBallFeetAttackRoll(FeetAttackRequest));
	FMatchPlayAuthoritativeResolveThroughBallFeetDefenseRollRequest
		FeetDefenseRequest;
	FeetDefenseRequest.RequestingSide = EInitialTurnOrderPlayer::PlayerB;
	TestNoActive(TEXT("ResolveThroughBallFeetDefenseRoll"),
		EmptyHost->ResolveThroughBallFeetDefenseRoll(FeetDefenseRequest));
	TestNoActive(TEXT("ResolvePassControlPostRoutePlan"), EmptyHost->ResolvePassControlPostRoutePlan());
	TestNoActive(TEXT("ResolveDeadCornerPostRouteDecision"), EmptyHost->ResolveDeadCornerPostRouteDecision());
	TestNoActive(TEXT("ResolveThroughBallAntiOffsideDecision"), EmptyHost->ResolveThroughBallAntiOffsideDecision());
	FMatchPlayAuthoritativeResolveThroughBallAntiOffsideAttackRollRequest
		AntiOffsideAttackRequest;
	AntiOffsideAttackRequest.AttackSequence = 1;
	AntiOffsideAttackRequest.RequestingSide = EInitialTurnOrderPlayer::PlayerA;
	TestNoActive(TEXT("ResolveThroughBallAntiOffsideAttackRoll"),
		EmptyHost->ResolveThroughBallAntiOffsideAttackRoll(
			AntiOffsideAttackRequest));
	TestNoActive(TEXT("ResolveDirectShotPostRouteDecisionOrPlan"), EmptyHost->ResolveDirectShotPostRouteDecisionOrPlan());
	TestNoActive(TEXT("ResolveThroughBallBehindDefenseP1DecisionOrPlan"), EmptyHost->ResolveThroughBallBehindDefenseP1DecisionOrPlan());
	TestNoActive(TEXT("ResolveSingleCardFinishingFormula"), EmptyHost->ResolveSingleCardFinishingFormula());
	TestNoActive(TEXT("ResolveThroughBallFeetFormula"), EmptyHost->ResolveThroughBallFeetFormula());
	TestNoActive(TEXT("ResolveThroughBallBehindDefenseP1Formula"), EmptyHost->ResolveThroughBallBehindDefenseP1Formula());
	TestNoActive(TEXT("ResolveThroughBallBehindDefenseP2Decision"), EmptyHost->ResolveThroughBallBehindDefenseP2Decision());
	TestNoActive(TEXT("ResolveThroughBallOneOnOneChipShotDecision"), EmptyHost->ResolveThroughBallOneOnOneChipShotDecision());
	FMatchPlayAuthoritativeResolveThroughBallOneOnOneChipShotAttackRollRequest
		ChipShotAttackRequest;
	ChipShotAttackRequest.AttackSequence = 1;
	ChipShotAttackRequest.RequestingSide = EInitialTurnOrderPlayer::PlayerA;
	TestNoActive(TEXT("ResolveThroughBallOneOnOneChipShotAttackRoll"),
		EmptyHost->ResolveThroughBallOneOnOneChipShotAttackRoll(
			ChipShotAttackRequest));
	TestNoActive(TEXT("ResolveThroughBallOneOnOneDirectShotPostRoutePlan"), EmptyHost->ResolveThroughBallOneOnOneDirectShotPostRoutePlan());
	FMatchPlayAuthoritativeResolveThroughBallOneOnOneDirectShotAttackRollRequest
		DirectShotAttackRequest;
	DirectShotAttackRequest.AttackSequence = 1;
	DirectShotAttackRequest.RequestingSide = EInitialTurnOrderPlayer::PlayerA;
	TestNoActive(TEXT("ResolveThroughBallOneOnOneDirectShotAttackRoll"),
		EmptyHost->ResolveThroughBallOneOnOneDirectShotAttackRoll(
			DirectShotAttackRequest));
	FMatchPlayAuthoritativeResolveThroughBallOneOnOneDirectShotDefenseRollRequest
		DirectShotDefenseRequest;
	DirectShotDefenseRequest.AttackSequence = 1;
	DirectShotDefenseRequest.RequestingSide = EInitialTurnOrderPlayer::PlayerB;
	TestNoActive(TEXT("ResolveThroughBallOneOnOneDirectShotDefenseRoll"),
		EmptyHost->ResolveThroughBallOneOnOneDirectShotDefenseRoll(
			DirectShotDefenseRequest));
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
	const FMatchPlayState Pending = Host->GetMatchSnapshot().Snapshot;
	TestTrue(TEXT("Cross terminal preserves CurrentAttack"),
		Pending.bHasCurrentAttack);
	TestEqual(TEXT("Cross terminal awaits explicit advance"),
		Pending.CurrentAttack.LifecycleState,
		EMatchPlayCurrentAttackLifecycleState::TerminalPendingAdvance);
	TestEqual(TEXT("Cross terminal leaves opportunity pending"),
		UsedAttacksFor(Pending, Trace.Attacker) - UsedBefore,
		0);
	TestEqual(TEXT("Cross score matches authoritative result"),
		ScoreFor(Pending, Trace.Attacker) - ScoreBefore,
		HostTerminal.AuthoritativeResult.OrchestrationResult.bIsGoal ? 1 : 0);
	TestTrue(TEXT("Terminal State equals direct Session"), AreStatesEqual(
		Pending, Direct.GetStateSnapshot()));
	FMatchPlayAuthoritativeAdvanceAfterTerminalRequest AdvanceRequest;
	AdvanceRequest.AttackSequence = Pending.CurrentAttack.AttackSequence;
	AdvanceRequest.RequestingSide = Trace.Attacker;
	const auto DirectAdvance = Direct.AdvanceAfterTerminal(AdvanceRequest);
	const auto HostAdvance = Host->AdvanceAfterTerminal(AdvanceRequest);
	const FMatchPlayState Completed = Host->GetMatchSnapshot().Snapshot;
	TestTrue(TEXT("Cross explicit advance succeeds through Host/direct"),
		DirectAdvance.CompletionResult.bSuccess && HostAdvance.bSuccess);
	TestFalse(TEXT("Cross explicit advance clears CurrentAttack"),
		Completed.bHasCurrentAttack);
	TestEqual(TEXT("Cross explicit advance consumes one opportunity"),
		UsedAttacksFor(Completed, Trace.Attacker) - UsedBefore,
		1);
	TestEqual(TEXT("Cross next attacker is canonical"),
		Completed.RuntimeState.CurrentAttackingPlayer,
		HostAdvance.AuthoritativeResult.CompletionResult.NextAttackingPlayer);
	TestTrue(TEXT("Advanced State equals direct Session"), AreStatesEqual(
		Completed, Direct.GetStateSnapshot()));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexLocalMatchThroughBallFeetManualResolutionTest,
	"FMCodex.LocalPlay.LocalMatchHost.07A.ThroughBallFeetManualResolution",
	EAutomationTestFlags::EditorContext
		| EAutomationTestFlags::EngineFilter)

bool FFMCodexLocalMatchThroughBallFeetManualResolutionTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexLocalMatchResolutionRoutingTests;
	(void)Parameters;
	const int32 Seed = FindSeedForRolls({ 2, 4, 3 });
	TestTrue(TEXT("A deterministic Feet manual seed exists"),
		Seed != INDEX_NONE);
	if (Seed == INDEX_NONE)
	{
		return false;
	}
	const FName SkillId(TEXT("Skill.Host.ThroughBall.FeetManual"));
	const auto Rules = MakeRules(SkillId, ESkillRuleType::ThroughBall);
	const auto Input = MakeInput(TEXT("HostThroughBallFeetManual"), SkillId);
	FFMCodexLocalMatchD6Provider DirectProvider(Seed);
	FMatchPlayAuthoritativeSession Direct(
		DirectProvider, DirectProvider, Rules);
	FScopedWorld World;
	auto* Host = World.GetHost();
	auto* Controller = World.GetController();
	TestNotNull(TEXT("Feet manual Host exists"), Host);
	TestNotNull(TEXT("Feet manual Controller exists"), Controller);
	if (Host == nullptr || Controller == nullptr)
	{
		return false;
	}
	FReadyTrace Trace;
	TestTrue(TEXT("Feet manual Host/direct reach ReadyForResolution"),
		BuildReadyForResolution(
			*Host, Direct, Input, Rules, Seed,
			ESkillRuleType::ThroughBall, false, Trace));
	const FMatchPlayState ReadyState = Host->GetMatchSnapshot().Snapshot;
	const FFMCodexLocalMatchInteractionView RouteView =
		FFMCodexLocalMatchInteractionViewBuilder::Build(ReadyState, Rules);
	TestTrue(TEXT("ThroughBall ready state projects attacker-owned typed route"),
		RouteView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory
				::RollThroughBallInitialRoute
			&& RouteView.ExpectedActingPlayer == Trace.Attacker
			&& RouteView.AttackSequence
				== ReadyState.CurrentAttack.AttackSequence);
	for (int32 RefreshIndex = 0; RefreshIndex < 4; ++RefreshIndex)
	{
		Controller->RefreshPresentation();
		const FMatchPlayState RefreshedState = Host->GetMatchSnapshot().Snapshot;
		const FFMCodexLocalMatchInteractionView RebuiltView =
			FFMCodexLocalMatchInteractionViewBuilder::Build(
				RefreshedState, Rules);
		TestTrue(*FString::Printf(
			TEXT("Refresh/reconstruction %d remains manual Route Pending"),
			RefreshIndex + 1),
			AreStatesEqual(ReadyState, RefreshedState)
				&& !RefreshedState.CurrentAttack.bHasResolutionSession
				&& RebuiltView.InteractionCategory
					== EFMCodexLocalMatchInteractionCategory
						::RollThroughBallInitialRoute
				&& RebuiltView.AttackSequence
					== ReadyState.CurrentAttack.AttackSequence);
	}
	FMatchPlayAuthoritativeResolveThroughBallInitialRouteRollRequest
		DirectRouteRequest;
	DirectRouteRequest.AttackSequence = ReadyState.CurrentAttack.AttackSequence;
	DirectRouteRequest.RequestingSide = Trace.Attacker;
	TestTrue(TEXT("Feet manual Direct accepts typed route"),
		Direct.ResolveThroughBallInitialRouteRoll(DirectRouteRequest)
			.OrchestrationResult.bSuccess);
	Controller->RefreshPresentation();
	Controller->ContinueResolution();
	TestTrue(TEXT("Generic Continue cannot own the ThroughBall route roll"),
		!Controller->GetLastDiagnostic().bHostSuccess
			&& Controller->GetLastDiagnostic().CommandName
				== TEXT("ContinueResolution")
			&& AreStatesEqual(
				ReadyState,
				Host->GetMatchSnapshot().Snapshot));
	Controller->RollThroughBallInitialRoute();
	TestTrue(TEXT("Controller routes typed ThroughBall route through Host"),
		Controller->GetLastDiagnostic().bHostSuccess
			&& Controller->GetLastDiagnostic().CommandName
				== TEXT("ResolveThroughBallInitialRouteRoll"));
	const FMatchPlayState RouteState = Host->GetMatchSnapshot().Snapshot;
	TestEqual(TEXT("One accepted click persists exactly one Route D6"),
		RouteState.CurrentAttack.ResolutionSession.InitialRouteRollRecords.Num(),
		1);
	for (int32 RefreshIndex = 0; RefreshIndex < 3; ++RefreshIndex)
	{
		Controller->RefreshPresentation();
	}
	TestEqual(TEXT("Post-click refresh/reveal setup cannot double-dispatch Route"),
		Host->GetMatchSnapshot().Snapshot.CurrentAttack.ResolutionSession
			.InitialRouteRollRecords.Num(),
		1);
	TestTrue(TEXT("Feet route snapshot equals direct Session"),
		AreStatesEqual(RouteState, Direct.GetStateSnapshot()));
	TestEqual(TEXT("Deterministic route is Feet"),
		RouteState.CurrentAttack.ResolutionSession.ActualBranch.ThroughBall,
		EMatchPlayThroughBallActualBranch::Feet);

	const FFMCodexLocalMatchInteractionView AttackView =
		FFMCodexLocalMatchInteractionViewBuilder::Build(RouteState, Rules);
	Controller->RefreshPresentation();
	TestTrue(TEXT("Feet route projects only the attacker-owned typed roll"),
		AttackView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory
				::RollThroughBallFeetAttack
			&& AttackView.ExpectedActingPlayer == Trace.Attacker
			&& AttackView.bThroughBallFeetAttackRollPending
			&& !AttackView.bThroughBallFeetDefenseRollPending
			&& AttackView.ContinueActionLabel == TEXT("掷进攻方点数"));
	const auto* PreviewContest =
		AttackView.ResolutionFacts.FormulaContests.FindByPredicate(
			[](const FMatchPlayResolutionFormulaContestFact& Contest)
			{
				return Contest.ContestId == TEXT("ThroughBall.Feet");
			});
	TestTrue(TEXT("InteractionView carries Feet formula preview facts"),
		PreviewContest != nullptr
			&& PreviewContest->AttackRow.bKnownNonRollSubtotalResolved
			&& PreviewContest->DefenseRow.bKnownNonRollSubtotalResolved
			&& !PreviewContest->AttackRow.bFinalValueResolved
			&& !PreviewContest->DefenseRow.bFinalValueResolved);
	Controller->ContinueResolution();
	TestTrue(TEXT("Generic Continue is retired from the Feet production path"),
		!Controller->GetLastDiagnostic().bHostSuccess
			&& Controller->GetLastDiagnostic().CommandName
				== TEXT("ContinueResolution")
			&& AreStatesEqual(
				RouteState,
				Host->GetMatchSnapshot().Snapshot));

	FMatchPlayAuthoritativeResolveThroughBallFeetAttackRollRequest AttackRequest;
	AttackRequest.AttackSequence = RouteState.CurrentAttack.AttackSequence;
	AttackRequest.RequestingSide = Trace.Attacker;
	TestTrue(TEXT("Direct accepts typed Feet attack roll"),
		Direct.ResolveThroughBallFeetAttackRoll(AttackRequest)
			.OrchestrationResult.bSuccess);
	Controller->RollThroughBallFeetAttack();
	TestTrue(TEXT("Controller routes typed Feet attack intent through Host"),
		Controller->GetLastDiagnostic().bHostSuccess
			&& Controller->GetLastDiagnostic().CommandName
				== TEXT("ResolveThroughBallFeetAttackRoll"));
	const FMatchPlayState AttackState = Host->GetMatchSnapshot().Snapshot;
	TestTrue(TEXT("Feet attack Host State equals direct Session"),
		AreStatesEqual(AttackState, Direct.GetStateSnapshot()));
	const FFMCodexLocalMatchInteractionView DefenseView =
		FFMCodexLocalMatchInteractionViewBuilder::Build(AttackState, Rules);
	TestTrue(TEXT("Refresh retains attack fact and projects defender-owned roll"),
		DefenseView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory
				::RollThroughBallFeetDefense
			&& DefenseView.ExpectedActingPlayer == Trace.Defender
			&& !DefenseView.bThroughBallFeetAttackRollPending
			&& DefenseView.bThroughBallFeetDefenseRollPending
			&& DefenseView.AcceptedRolls.Num() == 2
			&& DefenseView.AcceptedRolls.Last().RawD6 == 4
			&& DefenseView.ContinueActionLabel == TEXT("掷防守方点数"));

	FMatchPlayAuthoritativeResolveThroughBallFeetDefenseRollRequest
		DefenseRequest;
	DefenseRequest.AttackSequence = AttackState.CurrentAttack.AttackSequence;
	DefenseRequest.RequestingSide = Trace.Defender;
	TestTrue(TEXT("Direct accepts typed Feet defense roll"),
		Direct.ResolveThroughBallFeetDefenseRoll(DefenseRequest)
			.OrchestrationResult.bSuccess);
	const auto DirectTerminal = Direct.ApplyThroughBallTerminalResolution();
	TestTrue(TEXT("Direct persists Feet terminal after final defense roll"),
		DirectTerminal.OrchestrationResult.bSuccess);
	Controller->RollThroughBallFeetDefense();
	TestTrue(TEXT("Controller routes defense and zero-RNG terminal transitions"),
		Controller->GetLastDiagnostic().bHostSuccess
			&& Controller->GetLastDiagnostic().CommandName
				== TEXT("ApplyThroughBallTerminalResolution"));
	const FMatchPlayState CompleteState = Host->GetMatchSnapshot().Snapshot;
	TestTrue(TEXT("Feet terminal Host State equals direct Session"),
		AreStatesEqual(CompleteState, Direct.GetStateSnapshot()));
	const FFMCodexLocalMatchInteractionView TerminalView =
		FFMCodexLocalMatchInteractionViewBuilder::Build(CompleteState, Rules);
	TestTrue(TEXT("Persisted Feet terminal projects only explicit next round"),
		TerminalView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory
				::AdvanceAfterTerminal
			&& TerminalView.bTerminalPendingAdvance
			&& TerminalView.bThroughBallFeetFormulaComplete
			&& !TerminalView.bThroughBallFeetTerminalActionAvailable
			&& !TerminalView.bThroughBallFeetAttackRollPending
			&& !TerminalView.bThroughBallFeetDefenseRollPending
			&& !TerminalView.SelectedCarrierCardId.IsNone()
			&& !TerminalView.SelectedRunnerCardId.IsNone()
			&& !TerminalView.SelectedMarkerCardId.IsNone()
			&& !TerminalView.DeploymentPlacements.IsEmpty()
			&& TerminalView.bHasTacticalPlayerCounts
			&& TerminalView.PlayerATacticalPlayerCount
				+ TerminalView.PlayerBTacticalPlayerCount > 0
			&& TerminalView.ContinueActionLabel == TEXT("下一回合"));
	const auto RefreshedTerminalView =
		FFMCodexLocalMatchInteractionViewBuilder::Build(
			Host->GetMatchSnapshot().Snapshot, Rules);
	TestTrue(TEXT("Feet terminal refresh preserves facts and next-round action"),
		RefreshedTerminalView.bTerminalPendingAdvance
			&& RefreshedTerminalView.ResolutionFacts.bHasFacts
			&& FMatchPlayCurrentAttackResolutionFactProjection::StaticStruct()
				->CompareScriptStruct(
					&TerminalView.ResolutionFacts,
					&RefreshedTerminalView.ResolutionFacts,
					0));
	const auto RestoredFeedback =
		FFMCodexLocalMatchResolutionFeedbackBuilder::BuildFromTerminalSnapshot(
			RefreshedTerminalView);
	TestTrue(TEXT("Feet terminal refresh reconstructs result feedback"),
		RestoredFeedback.bTerminal
			&& RestoredFeedback.ResolutionFacts.bHasFacts
			&& RestoredFeedback.TerminalSummary.StartsWith(TEXT("RESULT: "))
			&& RestoredFeedback.ContinuationSummary.Contains(TEXT("下一回合")));
	FMatchPlayAuthoritativeAdvanceAfterTerminalRequest DirectAdvanceRequest;
	DirectAdvanceRequest.AttackSequence = TerminalView.AttackSequence;
	DirectAdvanceRequest.RequestingSide = TerminalView.ExpectedActingPlayer;
	TestTrue(TEXT("Direct explicit Feet advance succeeds"),
		Direct.AdvanceAfterTerminal(DirectAdvanceRequest)
			.CompletionResult.bSuccess);
	Controller->AdvanceAfterTerminal();
	TestTrue(TEXT("Controller routes explicit next-round intent through Host"),
		Controller->GetLastDiagnostic().bHostSuccess
			&& Controller->GetLastDiagnostic().CommandName
				== TEXT("AdvanceAfterTerminal"));
	TestTrue(TEXT("Feet manual advanced State equals direct Session"),
		AreStatesEqual(
			Host->GetMatchSnapshot().Snapshot,
			Direct.GetStateSnapshot()));
	const auto& AdvancedView = Controller->GetInteractionView();
	TestTrue(TEXT("Feet advance clears pitch, roles, and Tactical Player counts"),
		!AdvancedView.bCurrentAttackActive
			&& AdvancedView.DeploymentPlacements.IsEmpty()
			&& AdvancedView.SelectedCarrierCardId.IsNone()
			&& AdvancedView.SelectedRunnerCardId.IsNone()
			&& AdvancedView.SelectedMarkerCardId.IsNone()
			&& AdvancedView.SelectedHelperCardId.IsNone()
			&& AdvancedView.bHasTacticalPlayerCounts
			&& AdvancedView.PlayerATacticalPlayerCount == 0
			&& AdvancedView.PlayerBTacticalPlayerCount == 0
			&& AdvancedView.bTacticalPointRollReady);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexLocalMatchThroughBallBehindDefenseManualResolutionTest,
	"FMCodex.LocalPlay.LocalMatchHost.07B.ThroughBallBehindDefenseManualResolution",
	EAutomationTestFlags::EditorContext
		| EAutomationTestFlags::EngineFilter)

bool FFMCodexLocalMatchThroughBallBehindDefenseManualResolutionTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexLocalMatchResolutionRoutingTests;
	(void)Parameters;
	const int32 Seed = FindSeedForRolls({ 4, 3, 6 });
	TestTrue(TEXT("A deterministic Behind manual seed exists"),
		Seed != INDEX_NONE);
	if (Seed == INDEX_NONE)
	{
		return false;
	}

	const FName SkillId(TEXT("Skill.Host.ThroughBall.BehindManual"));
	const auto Rules = MakeRules(SkillId, ESkillRuleType::ThroughBall);
	const auto Input = MakeInput(TEXT("HostThroughBallBehindManual"), SkillId);
	FFMCodexLocalMatchD6Provider DirectProvider(Seed);
	FMatchPlayAuthoritativeSession Direct(
		DirectProvider, DirectProvider, Rules);
	FScopedWorld World;
	auto* Host = World.GetHost();
	auto* Controller = World.GetController();
	TestNotNull(TEXT("Behind manual Host exists"), Host);
	TestNotNull(TEXT("Behind manual Controller exists"), Controller);
	if (Host == nullptr || Controller == nullptr)
	{
		return false;
	}

	FReadyTrace Trace;
	TestTrue(TEXT("Behind manual Host/direct reach ReadyForResolution"),
		BuildReadyForResolution(
			*Host, Direct, Input, Rules, Seed,
			ESkillRuleType::ThroughBall, false, Trace));
	TestTrue(TEXT("Behind manual Direct begins resolution"),
		Direct.BeginResolutionSession().BeginResult.bSuccess);
	TestTrue(TEXT("Behind manual Host begins resolution"),
		Host->BeginResolutionSession().bSuccess);
	TestTrue(TEXT("Behind manual Direct resolves route"),
		Direct.ResolveInitialRoute().OrchestrationResult.bSuccess);
	TestTrue(TEXT("Behind manual Host resolves route"),
		Host->ResolveInitialRoute().bSuccess);
	const FMatchPlayState RouteState = Host->GetMatchSnapshot().Snapshot;
	TestEqual(TEXT("Deterministic route is BehindDefense"),
		RouteState.CurrentAttack.ResolutionSession.ActualBranch.ThroughBall,
		EMatchPlayThroughBallActualBranch::BehindDefense);
	TestTrue(TEXT("Behind route snapshot equals direct Session"),
		AreStatesEqual(RouteState, Direct.GetStateSnapshot()));

	Controller->RefreshPresentation();
	const auto AttackView = Controller->GetInteractionView();
	TestTrue(TEXT("Behind route projects attacker-owned typed roll"),
		AttackView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory
				::RollThroughBallBehindDefenseAttack
			&& AttackView.ExpectedActingPlayer == Trace.Attacker
			&& AttackView.bThroughBallBehindDefenseAttackRollPending
			&& !AttackView.bThroughBallBehindDefenseDefenseRollPending);
	Controller->ContinueResolution();
	TestTrue(TEXT("Generic Continue is retired from Behind P1 rolls"),
		!Controller->GetLastDiagnostic().bHostSuccess
			&& Controller->GetLastDiagnostic().CommandName
				== TEXT("ContinueResolution")
			&& AreStatesEqual(RouteState, Host->GetMatchSnapshot().Snapshot));

	FMatchPlayAuthoritativeResolveThroughBallBehindDefenseP1AttackRollRequest
		AttackRequest;
	AttackRequest.AttackSequence = RouteState.CurrentAttack.AttackSequence;
	AttackRequest.RequestingSide = Trace.Attacker;
	TestTrue(TEXT("Direct accepts typed Behind Attack"),
		Direct.ResolveThroughBallBehindDefenseP1AttackRoll(AttackRequest)
			.OrchestrationResult.bSuccess);
	Controller->RollThroughBallBehindDefenseAttack();
	TestTrue(TEXT("Controller routes typed Behind Attack through Host"),
		Controller->GetLastDiagnostic().bHostSuccess
			&& Controller->GetLastDiagnostic().CommandName
				== TEXT("ResolveThroughBallBehindDefenseP1AttackRoll"));
	const FMatchPlayState AttackState = Host->GetMatchSnapshot().Snapshot;
	TestTrue(TEXT("Behind Attack Host State equals direct Session"),
		AreStatesEqual(AttackState, Direct.GetStateSnapshot()));
	const auto DefenseView = Controller->GetInteractionView();
	TestTrue(TEXT("Behind Attack refresh projects defender-owned typed roll"),
		DefenseView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory
				::RollThroughBallBehindDefenseDefense
			&& DefenseView.ExpectedActingPlayer == Trace.Defender
			&& !DefenseView.bThroughBallBehindDefenseAttackRollPending
			&& DefenseView.bThroughBallBehindDefenseDefenseRollPending
			&& DefenseView.AcceptedRolls.Last().RawD6 == 3);

	FMatchPlayAuthoritativeResolveThroughBallBehindDefenseP1DefenseRollRequest
		DefenseRequest;
	DefenseRequest.AttackSequence = AttackState.CurrentAttack.AttackSequence;
	DefenseRequest.RequestingSide = Trace.Defender;
	const auto DirectDefense =
		Direct.ResolveThroughBallBehindDefenseP1DefenseRoll(DefenseRequest);
	TestTrue(TEXT("Direct accepts typed Behind Defense"),
		DirectDefense.OrchestrationResult.bSuccess);
	const auto DirectFormula = Direct.ResolveThroughBallBehindDefenseP1Formula();
	TestTrue(TEXT("Direct resolves Behind Formula with zero new roll"),
		DirectFormula.OrchestrationResult.bSuccess
			&& DirectFormula.OrchestrationResult
				.PlanRegenerationProviderCallCount == 0);
	const bool bAttackerWin = DirectFormula.OrchestrationResult
		.FormulaResolutionResult.bContinueResolution;
	if (!bAttackerWin)
	{
		TestTrue(TEXT("Direct applies defender-win terminal"),
			Direct.ApplyThroughBallTerminalResolution()
				.OrchestrationResult.bSuccess);
	}

	Controller->RollThroughBallBehindDefenseDefense();
	const FMatchPlayState CompleteState = Host->GetMatchSnapshot().Snapshot;
	TestTrue(TEXT("Controller typed Defense reaches same canonical State"),
		AreStatesEqual(CompleteState, Direct.GetStateSnapshot()));
	if (bAttackerWin)
	{
		const auto ChoiceView = Controller->GetInteractionView();
		TestTrue(TEXT("Attacker win rebuilds existing OneOnOne typed choices"),
			ChoiceView.InteractionCategory
				== EFMCodexLocalMatchInteractionCategory::SelectOneOnOneShot
				&& ChoiceView.ExpectedActingPlayer == Trace.Attacker
				&& ChoiceView.OneOnOneOptions.Num() == 2);
	}
	else
	{
		const auto TerminalView = Controller->GetInteractionView();
		TestTrue(TEXT("Defender win auto-applies zero-RNG terminal"),
			TerminalView.InteractionCategory
				== EFMCodexLocalMatchInteractionCategory::AdvanceAfterTerminal
				&& TerminalView.bTerminalPendingAdvance);
	}
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
	TestEqual(TEXT("P1 directly requires OneOnOne"),
		P1Formula.AuthoritativeResult.OrchestrationResult
			.FormulaExecutionResult.Decision,
		EThroughBallBehindDefenseP1FormulaResolutionExecutionDecision
			::OneOnOneRequired);
	const FMatchPlayState BeforeLegacyP2 = Direct.GetStateSnapshot();
	TestFalse(TEXT("Direct rejects legacy P2"),
		Direct.ResolveThroughBallBehindDefenseP2Decision()
			.OrchestrationResult.bSuccess);
	TestFalse(TEXT("Host rejects legacy P2"),
		HostA->ResolveThroughBallBehindDefenseP2Decision().bSuccess);
	TestTrue(TEXT("Rejected legacy P2 leaves direct State stable"),
		AreStatesEqual(BeforeLegacyP2, Direct.GetStateSnapshot()));
	TestTrue(TEXT("Rejected legacy P2 leaves Host State stable"),
		AreStatesEqual(BeforeLegacyP2,
			HostA->GetMatchSnapshot().Snapshot));

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

	FMatchPlayAuthoritativeResolveThroughBallOneOnOneDirectShotAttackRollRequest
		DirectAttackRequest;
	DirectAttackRequest.AttackSequence = HostA->GetMatchSnapshot().Snapshot
		.CurrentAttack.AttackSequence;
	DirectAttackRequest.RequestingSide = TraceA.Attacker;
	TestTrue(TEXT("Direct resolves owned DirectShot Attack"),
		Direct.ResolveThroughBallOneOnOneDirectShotAttackRoll(
			DirectAttackRequest).OrchestrationResult.bSuccess);
	const auto HostDirectAttack =
		HostA->ResolveThroughBallOneOnOneDirectShotAttackRoll(
			DirectAttackRequest);
	TestTrue(TEXT("Host routes owned DirectShot Attack"),
		HostDirectAttack.bSuccess);
	const FMatchPlayState AfterDirectAttack =
		HostA->GetMatchSnapshot().Snapshot;
	TestEqual(TEXT("Deep path persists attack-only third record"),
		AfterDirectAttack.CurrentAttack.ResolutionSession
			.PostRouteRollProgress.RollRecords.Num(),
		3);
	TestTrue(TEXT("DirectShot attack-only State equals direct Session"),
		AreStatesEqual(AfterDirectAttack, Direct.GetStateSnapshot()));

	FMatchPlayAuthoritativeResolveThroughBallOneOnOneDirectShotDefenseRollRequest
		DirectDefenseRequest;
	DirectDefenseRequest.AttackSequence =
		DirectAttackRequest.AttackSequence;
	DirectDefenseRequest.RequestingSide = OtherPlayer(TraceA.Attacker);
	TestTrue(TEXT("Direct resolves owned DirectShot Defense"),
		Direct.ResolveThroughBallOneOnOneDirectShotDefenseRoll(
			DirectDefenseRequest).OrchestrationResult.bSuccess);
	const auto DirectShotPlan =
		HostA->ResolveThroughBallOneOnOneDirectShotDefenseRoll(
			DirectDefenseRequest);
	TestTrue(TEXT("Host routes owned DirectShot Defense"),
		DirectShotPlan.bSuccess
			&& DirectShotPlan.AuthoritativeResult.OrchestrationResult
				.bHasFormulaPlan);
	const FMatchPlayState AfterDirectPlan =
		HostA->GetMatchSnapshot().Snapshot;
	TestEqual(TEXT("Deep path records four post-route rolls"),
		AfterDirectPlan.CurrentAttack.ResolutionSession
			.PostRouteRollProgress.RollRecords.Num(),
		4);
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
	const FMatchPlayState Pending = HostA->GetMatchSnapshot().Snapshot;
	TestTrue(TEXT("ThroughBall terminal preserves CurrentAttack"),
		Pending.bHasCurrentAttack);
	TestEqual(TEXT("ThroughBall terminal awaits explicit advance"),
		Pending.CurrentAttack.LifecycleState,
		EMatchPlayCurrentAttackLifecycleState::TerminalPendingAdvance);
	TestEqual(TEXT("ThroughBall terminal leaves opportunity pending"),
		UsedAttacksFor(Pending, TraceA.Attacker) - UsedBefore,
		0);
	TestEqual(TEXT("ThroughBall score matches authoritative semantic"),
		ScoreFor(Pending, TraceA.Attacker) - ScoreBefore,
		HostTerminal.AuthoritativeResult.OrchestrationResult.bIsGoal ? 1 : 0);
	TestTrue(TEXT("ThroughBall terminal State equals direct Session"),
		AreStatesEqual(Pending, Direct.GetStateSnapshot()));

	const int32 ReplayScore = ScoreFor(Pending, TraceA.Attacker);
	const int32 ReplayUsed = UsedAttacksFor(Pending, TraceA.Attacker);
	const auto Replay = HostA->ApplyThroughBallTerminalResolution();
	TestFalse(TEXT("Terminal replay rejects"), Replay.bSuccess);
	TestEqual(TEXT("Terminal replay requires explicit advance"),
		Replay.AuthoritativeResult.RuntimeEnvelope.RuntimeFailureCode,
		EMatchPlayAuthoritativeRuntimeFailureCode::TerminalAdvanceRequired);
	TestTrue(TEXT("Terminal replay leaves State unchanged"), AreStatesEqual(
		HostA->GetMatchSnapshot().Snapshot, Pending));
	TestEqual(TEXT("Terminal replay score delta is zero"),
		ScoreFor(HostA->GetMatchSnapshot().Snapshot, TraceA.Attacker)
			- ReplayScore,
		0);
	TestEqual(TEXT("Terminal replay opportunity delta is zero"),
		UsedAttacksFor(HostA->GetMatchSnapshot().Snapshot, TraceA.Attacker)
			- ReplayUsed,
		0);

	FMatchPlayAuthoritativeAdvanceAfterTerminalRequest AdvanceRequest;
	AdvanceRequest.AttackSequence = Pending.CurrentAttack.AttackSequence;
	AdvanceRequest.RequestingSide = TraceA.Attacker;
	const auto DirectAdvance = Direct.AdvanceAfterTerminal(AdvanceRequest);
	const auto HostAdvance = HostA->AdvanceAfterTerminal(AdvanceRequest);
	const FMatchPlayState Completed = HostA->GetMatchSnapshot().Snapshot;
	TestTrue(TEXT("ThroughBall explicit advance succeeds through Host/direct"),
		DirectAdvance.CompletionResult.bSuccess && HostAdvance.bSuccess);
	TestFalse(TEXT("ThroughBall explicit advance clears CurrentAttack"),
		Completed.bHasCurrentAttack);
	TestEqual(TEXT("ThroughBall explicit advance consumes one opportunity"),
		UsedAttacksFor(Completed, TraceA.Attacker) - UsedBefore,
		1);
	TestTrue(TEXT("ThroughBall advanced State equals direct Session"),
		AreStatesEqual(Completed, Direct.GetStateSnapshot()));
	const auto RepeatedAdvance = HostA->AdvanceAfterTerminal(AdvanceRequest);
	TestFalse(TEXT("Repeated ThroughBall advance rejects"),
		RepeatedAdvance.bSuccess);
	TestTrue(TEXT("Repeated ThroughBall advance leaves State unchanged"),
		AreStatesEqual(HostA->GetMatchSnapshot().Snapshot, Completed));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexLocalMatchThroughBallDecisiveRollAutoProgressionTest,
	"FMCodex.LocalPlay.LocalMatchHost.08B.ThroughBallDecisiveRollAutoProgression",
	EAutomationTestFlags::EditorContext
		| EAutomationTestFlags::EngineFilter)

bool FFMCodexLocalMatchThroughBallDecisiveRollAutoProgressionTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexLocalMatchResolutionRoutingTests;
	using EChoice = EMatchPlayThroughBallOneOnOneShotChoice;
	using ETarget = EFMCodexLocalDevRollTarget;
	(void)Parameters;

	auto RunScenario = [this](
		const FString& Suffix,
		const int32 AntiRoll,
		const EChoice Choice)
	{
		const int32 Seed = 61420 + Suffix.Len();
		const FName SkillId(*FString::Printf(
			TEXT("Skill.AutoProgress.%s"), *Suffix));
		const auto Rules = MakeRules(SkillId, ESkillRuleType::ThroughBall);
		const auto Input = MakeInput(
			FString::Printf(TEXT("AutoProgress%s"), *Suffix), SkillId);
		FFMCodexLocalMatchD6Provider DirectProvider(Seed);
		FMatchPlayAuthoritativeSession Direct(
			DirectProvider, DirectProvider, Rules);
		FScopedWorld World;
		auto* Host = World.GetHost();
		auto* Controller = World.GetController();
		if (Host == nullptr || Controller == nullptr)
		{
			AddError(FString::Printf(
				TEXT("%s Host/Controller allocation failed"), *Suffix));
			return false;
		}
		FReadyTrace Trace;
		if (!BuildReadyForResolution(
			*Host, Direct, Input, Rules, Seed,
			ESkillRuleType::ThroughBall, false, Trace))
		{
			AddError(FString::Printf(
				TEXT("%s did not reach ReadyForResolution"), *Suffix));
			return false;
		}
		auto SetOverride = [this, Host, &Suffix](
			const ETarget Target, const int32 Value)
		{
			FFMCodexLocalDevRollOverrideRequest Request;
			Request.Target = Target;
			Request.Value = Value;
			const bool bSuccess =
				Host->SetLocalDevRollOverride(Request).bSuccess;
			TestTrue(FString::Printf(
				TEXT("%s accepts deterministic roll override"), *Suffix),
				bSuccess);
			return bSuccess;
		};
		if (!SetOverride(ETarget::ThroughBallRoute, 6)
			|| !SetOverride(ETarget::ThroughBallAntiOffside, AntiRoll)
			|| (Choice == EChoice::ChipShot
				&& !SetOverride(ETarget::OneOnOneChipShotAttack, 6))
			|| (Choice == EChoice::DirectShot
				&& (!SetOverride(ETarget::OneOnOneDirectShotAttack, 6)
					|| !SetOverride(
						ETarget::OneOnOneDirectShotDefense, 1))))
		{
			return false;
		}

		const int32 UsedBefore = UsedAttacksFor(
			Host->GetMatchSnapshot().Snapshot, Trace.Attacker);
		for (int32 RefreshIndex = 0; RefreshIndex < 4; ++RefreshIndex)
		{
			Controller->RefreshPresentation();
			const FMatchPlayState Pending = Host->GetMatchSnapshot().Snapshot;
			TestTrue(FString::Printf(
				TEXT("%s refresh %d preserves manual route and DEV one-shot"),
				*Suffix, RefreshIndex + 1),
				!Pending.CurrentAttack.bHasResolutionSession
					&& Controller->GetInteractionView().InteractionCategory
						== EFMCodexLocalMatchInteractionCategory
							::RollThroughBallInitialRoute
					&& Host->GetLocalDevPendingRollOverrides()
						.ContainsByPredicate(
							[](const FFMCodexLocalDevPendingRollOverride& Item)
							{
								return Item.Target
									== ETarget::ThroughBallRoute;
							}));
		}
		Controller->RollThroughBallInitialRoute();
		TestEqual(FString::Printf(
			TEXT("%s route reaches typed Anti roll"), *Suffix),
			Controller->GetInteractionView().InteractionCategory,
			EFMCodexLocalMatchInteractionCategory
				::RollThroughBallAntiOffsideAttack);
		Controller->RollThroughBallAntiOffsideAttack();

		if (AntiRoll != 6)
		{
			const FMatchPlayState State = Host->GetMatchSnapshot().Snapshot;
			const auto& View = Controller->GetInteractionView();
			TestTrue(FString::Printf(
				TEXT("%s decisive Anti roll auto-applies terminal"), *Suffix),
				State.CurrentAttack.LifecycleState
					== EMatchPlayCurrentAttackLifecycleState
						::TerminalPendingAdvance
					&& View.InteractionCategory
						== EFMCodexLocalMatchInteractionCategory
							::AdvanceAfterTerminal
					&& View.ContinueActionLabel == TEXT("下一回合")
					&& !View.ContinueActionLabel.Contains(
						TEXT("继续直塞结算"))
					&& State.CurrentAttack.ResolutionSession
						.PostRouteRollProgress.RollRecords.Num() == 1
					&& UsedAttacksFor(State, Trace.Attacker) == UsedBefore);
			return true;
		}

		TestTrue(FString::Printf(
			TEXT("%s Anti success stops directly at shot choice"), *Suffix),
			Controller->GetInteractionView().InteractionCategory
				== EFMCodexLocalMatchInteractionCategory::SelectOneOnOneShot
				&& Host->GetMatchSnapshot().Snapshot.CurrentAttack.LifecycleState
					== EMatchPlayCurrentAttackLifecycleState::Active
				&& Host->GetMatchSnapshot().Snapshot.CurrentAttack
					.ResolutionSession.PostRouteRollProgress.RollRecords.Num()
						== 1);
		if (Choice == EChoice::None)
		{
			return true;
		}

		Controller->SubmitOneOnOneShotChoice(Choice);
		if (Choice == EChoice::ChipShot)
		{
			TestEqual(FString::Printf(
				TEXT("%s choice reaches typed Chip roll"), *Suffix),
				Controller->GetInteractionView().InteractionCategory,
				EFMCodexLocalMatchInteractionCategory
					::RollThroughBallOneOnOneChipShotAttack);
			Controller->RollThroughBallOneOnOneChipShotAttack();
		}
		else
		{
			TestEqual(FString::Printf(
				TEXT("%s choice reaches typed Direct attack roll"), *Suffix),
				Controller->GetInteractionView().InteractionCategory,
				EFMCodexLocalMatchInteractionCategory
					::RollThroughBallOneOnOneDirectShotAttack);
			Controller->RollThroughBallOneOnOneDirectShotAttack();
			TestEqual(FString::Printf(
				TEXT("%s attack roll stops at typed Direct defense"), *Suffix),
				Controller->GetInteractionView().InteractionCategory,
				EFMCodexLocalMatchInteractionCategory
					::RollThroughBallOneOnOneDirectShotDefense);
			Controller->RollThroughBallOneOnOneDirectShotDefense();
		}

		const FMatchPlayState State = Host->GetMatchSnapshot().Snapshot;
		const auto& View = Controller->GetInteractionView();
		const int32 ExpectedPostRouteRolls =
			Choice == EChoice::ChipShot ? 2 : 3;
		TestTrue(FString::Printf(
			TEXT("%s final roll auto-applies zero-RNG terminal"), *Suffix),
			State.CurrentAttack.LifecycleState
				== EMatchPlayCurrentAttackLifecycleState::TerminalPendingAdvance
				&& View.InteractionCategory
					== EFMCodexLocalMatchInteractionCategory::AdvanceAfterTerminal
				&& View.ContinueActionLabel == TEXT("下一回合")
				&& !View.ContinueActionLabel.Contains(TEXT("继续直塞结算"))
				&& State.CurrentAttack.ResolutionSession.PostRouteRollProgress
					.RollRecords.Num() == ExpectedPostRouteRolls
				&& UsedAttacksFor(State, Trace.Attacker) == UsedBefore
				&& Host->GetLocalDevPendingRollOverrides().IsEmpty());
		return true;
	};

	const bool bOffside = RunScenario(TEXT("Offside"), 1, EChoice::None);
	const bool bChoice = RunScenario(TEXT("Choice"), 6, EChoice::None);
	const bool bChip = RunScenario(TEXT("Chip"), 6, EChoice::ChipShot);
	const bool bDirect = RunScenario(TEXT("Direct"), 6, EChoice::DirectShot);
	return bOffside && bChoice && bChip && bDirect;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexLocalDevRollOverrideAuthorityFlowTest,
	"FMCodex.LocalPlay.DevRollOverride.05.RealAuthorityFlows",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexLocalDevRollOverrideAuthorityFlowTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexLocalMatchResolutionRoutingTests;
	using ETarget = EFMCodexLocalDevRollTarget;
	(void)Parameters;
	auto Set = [this](
		AFMCodexLocalMatchHostGameMode& Host,
		const ETarget Target,
		const int32 Value)
	{
		FFMCodexLocalDevRollOverrideRequest Request;
		Request.Target = Target;
		Request.Value = Value;
		const auto Result = Host.SetLocalDevRollOverride(Request);
		TestTrue(TEXT("Authority accepts typed DEV override"), Result.bSuccess);
		return Result.bSuccess;
	};

	{
		const int32 Seed = 10613;
		const FName SkillId(TEXT("Skill.DevOverride.ThroughBall"));
		const auto Rules = MakeRules(SkillId, ESkillRuleType::ThroughBall);
		const auto Input = MakeInput(TEXT("DevOverrideTB"), SkillId);
		FFMCodexLocalMatchD6Provider DirectProvider(Seed);
		FMatchPlayAuthoritativeSession Direct(
			DirectProvider, DirectProvider, Rules);
		FScopedWorld World;
		auto* Host = World.GetHost();
		TestNotNull(TEXT("ThroughBall override Host exists"), Host);
		if (Host == nullptr)
		{
			return false;
		}
		FReadyTrace Trace;
		TestTrue(TEXT("ThroughBall override reaches ready state"),
			BuildReadyForResolution(
				*Host, Direct, Input, Rules, Seed,
				ESkillRuleType::ThroughBall, false, Trace));
		TestTrue(TEXT("Route and P1 overrides set"),
			Set(*Host, ETarget::ThroughBallRoute, 4)
				&& Set(*Host, ETarget::ThroughBallBehindDefenseP1, 3));
		TestEqual(TEXT("Two semantic entries are pending"),
			Host->GetLocalDevPendingRollOverrides().Num(), 2);
		const FMatchPlayState ReadyState = Host->GetMatchSnapshot().Snapshot;
		FMatchPlayAuthoritativeResolveThroughBallInitialRouteRollRequest
			RouteRequest;
		RouteRequest.AttackSequence =
			ReadyState.CurrentAttack.AttackSequence + 1;
		RouteRequest.RequestingSide = Trace.Attacker;
		TestFalse(TEXT("Stale typed route request rejects"),
			Host->ResolveThroughBallInitialRouteRoll(RouteRequest).bSuccess);
		TestTrue(TEXT("Stale typed route retains pending override"),
			Host->GetLocalDevPendingRollOverrides().ContainsByPredicate(
				[](const FFMCodexLocalDevPendingRollOverride& Item)
				{
					return Item.Target == ETarget::ThroughBallRoute;
				}));
		RouteRequest.AttackSequence = ReadyState.CurrentAttack.AttackSequence;
		const auto Route =
			Host->ResolveThroughBallInitialRouteRoll(RouteRequest);
		const FMatchPlayState RouteState = Host->GetMatchSnapshot().Snapshot;
		TestTrue(TEXT("Real Host route command succeeds"), Route.bSuccess);
		TestEqual(TEXT("Authority stored overridden route RawD6"),
			RouteState.CurrentAttack.ResolutionSession
				.InitialRouteRollRecords[0].RawD6, 4);
		TestEqual(TEXT("Canonical mapping turns 4 into BehindDefense"),
			RouteState.CurrentAttack.ResolutionSession.ActualBranch.ThroughBall,
			EMatchPlayThroughBallActualBranch::BehindDefense);
		TestFalse(TEXT("Route override auto-consumed"),
			Host->GetLocalDevPendingRollOverrides().ContainsByPredicate(
				[](const FFMCodexLocalDevPendingRollOverride& Item)
				{
					return Item.Target == ETarget::ThroughBallRoute;
				}));
		const auto P1 =
			Host->ResolveThroughBallBehindDefenseP1DecisionOrPlan();
		const FMatchPlayState P1State = Host->GetMatchSnapshot().Snapshot;
		TestTrue(TEXT("Real Host P1 command succeeds"), P1.bSuccess);
		TestEqual(TEXT("P1 primary authority record uses override"),
			P1State.CurrentAttack.ResolutionSession.PostRouteRollProgress
				.RollRecords[0].RawD6, 3);
		TestTrue(TEXT("Route/P1 overrides independently auto-clear"),
			Host->GetLocalDevPendingRollOverrides().IsEmpty());
	}

	{
		const int32 Seed = 20613;
		const FName SkillId(TEXT("Skill.DevOverride.OneOnOneOwned"));
		const auto Rules = MakeRules(SkillId, ESkillRuleType::ThroughBall);
		const auto Input = MakeInput(TEXT("DevOverrideOwnedOneOnOne"), SkillId);
		FFMCodexLocalMatchD6Provider DirectProvider(Seed);
		FMatchPlayAuthoritativeSession Direct(
			DirectProvider, DirectProvider, Rules);
		FScopedWorld World;
		auto* Host = World.GetHost();
		TestNotNull(TEXT("Owned OneOnOne override Host exists"), Host);
		if (Host == nullptr)
		{
			return false;
		}
		FReadyTrace Trace;
		TestTrue(TEXT("Owned OneOnOne override reaches ready state"),
			BuildReadyForResolution(
				*Host, Direct, Input, Rules, Seed,
				ESkillRuleType::ThroughBall, false, Trace));
		TestTrue(TEXT("Owned OneOnOne semantic overrides set"),
			Set(*Host, ETarget::ThroughBallRoute, 5)
				&& Set(*Host, ETarget::ThroughBallAntiOffside, 6)
				&& Set(*Host, ETarget::OneOnOneDirectShotAttack, 5)
				&& Set(*Host, ETarget::OneOnOneDirectShotDefense, 1));
		const FMatchPlayState ReadyState = Host->GetMatchSnapshot().Snapshot;
		FMatchPlayAuthoritativeResolveThroughBallInitialRouteRollRequest
			RouteRequest;
		RouteRequest.AttackSequence = ReadyState.CurrentAttack.AttackSequence;
		RouteRequest.RequestingSide = Trace.Attacker;
		TestTrue(TEXT("Owned OneOnOne authority reaches AntiOffside"),
			Host->ResolveThroughBallInitialRouteRoll(RouteRequest).bSuccess);
		const FMatchPlayState RouteState = Host->GetMatchSnapshot().Snapshot;
		TestEqual(TEXT("Override route selects AntiOffside"),
			RouteState.CurrentAttack.ResolutionSession.ActualBranch.ThroughBall,
			EMatchPlayThroughBallActualBranch::AntiOffside);

		FMatchPlayAuthoritativeResolveThroughBallAntiOffsideAttackRollRequest
			AntiRequest;
		AntiRequest.AttackSequence = RouteState.CurrentAttack.AttackSequence;
		AntiRequest.RequestingSide = Trace.Defender;
		const auto WrongAnti =
			Host->ResolveThroughBallAntiOffsideAttackRoll(AntiRequest);
		TestFalse(TEXT("Wrong-side Anti override command rejects"),
			WrongAnti.bSuccess);
		TestTrue(TEXT("Rejected Anti request retains pending override"),
			Host->GetLocalDevPendingRollOverrides().ContainsByPredicate(
				[](const FFMCodexLocalDevPendingRollOverride& Item)
				{
					return Item.Target
						== ETarget::ThroughBallAntiOffside;
				}));
		AntiRequest.RequestingSide = Trace.Attacker;
		TestTrue(TEXT("Accepted Anti command consumes override"),
			Host->ResolveThroughBallAntiOffsideAttackRoll(AntiRequest).bSuccess);
		TestEqual(TEXT("Accepted Anti stores overridden 6"),
			Host->GetMatchSnapshot().Snapshot.CurrentAttack.ResolutionSession
				.PostRouteRollProgress.RollRecords.Last().RawD6,
			6);

		FMatchPlayAuthoritativeSubmitThroughBallOneOnOneShotChoiceRequest Choice;
		Choice.RequestingSide = Trace.Attacker;
		Choice.Choice =
			EMatchPlayThroughBallOneOnOneShotChoice::DirectShot;
		TestTrue(TEXT("Owned override flow selects DirectShot"),
			Host->SubmitThroughBallOneOnOneShotChoice(Choice).bSuccess);

		FMatchPlayAuthoritativeResolveThroughBallOneOnOneDirectShotDefenseRollRequest
			Defense;
		Defense.AttackSequence = AntiRequest.AttackSequence;
		Defense.RequestingSide = Trace.Defender;
		const auto EarlyDefense = Host
			->ResolveThroughBallOneOnOneDirectShotDefenseRoll(Defense);
		TestFalse(TEXT("Defense-before-Attack override command rejects"),
			EarlyDefense.bSuccess);
		TestTrue(TEXT("Rejected early Defense retains pending override"),
			Host->GetLocalDevPendingRollOverrides().ContainsByPredicate(
				[](const FFMCodexLocalDevPendingRollOverride& Item)
				{
					return Item.Target
						== ETarget::OneOnOneDirectShotDefense;
				}));

		FMatchPlayAuthoritativeResolveThroughBallOneOnOneDirectShotAttackRollRequest
			Attack;
		Attack.AttackSequence = AntiRequest.AttackSequence;
		Attack.RequestingSide = Trace.Defender;
		const auto WrongAttack = Host
			->ResolveThroughBallOneOnOneDirectShotAttackRoll(Attack);
		TestFalse(TEXT("Wrong-side Direct Attack override command rejects"),
			WrongAttack.bSuccess);
		TestTrue(TEXT("Rejected Direct Attack retains pending override"),
			Host->GetLocalDevPendingRollOverrides().ContainsByPredicate(
				[](const FFMCodexLocalDevPendingRollOverride& Item)
				{
					return Item.Target
						== ETarget::OneOnOneDirectShotAttack;
				}));
		Attack.RequestingSide = Trace.Attacker;
		TestTrue(TEXT("Accepted Direct Attack consumes only Attack override"),
			Host->ResolveThroughBallOneOnOneDirectShotAttackRoll(Attack).bSuccess);
		TestFalse(TEXT("Direct Attack override auto-clears"),
			Host->GetLocalDevPendingRollOverrides().ContainsByPredicate(
				[](const FFMCodexLocalDevPendingRollOverride& Item)
				{
					return Item.Target
						== ETarget::OneOnOneDirectShotAttack;
				}));
		TestTrue(TEXT("Direct Defense override remains after Attack"),
			Host->GetLocalDevPendingRollOverrides().ContainsByPredicate(
				[](const FFMCodexLocalDevPendingRollOverride& Item)
				{
					return Item.Target
						== ETarget::OneOnOneDirectShotDefense;
				}));
		TestTrue(TEXT("Accepted Direct Defense consumes Defense override"),
			Host->ResolveThroughBallOneOnOneDirectShotDefenseRoll(Defense).bSuccess);
		const auto& Records = Host->GetMatchSnapshot().Snapshot.CurrentAttack
			.ResolutionSession.PostRouteRollProgress.RollRecords;
		TestTrue(TEXT("Owned Direct overrides persist exact ordered values"),
			Records.Num() == 3
				&& Records[1].Purpose
					== EMatchPlayCurrentAttackPostRouteRollPurpose
						::OneOnOneDirectShotAttack
				&& Records[1].RawD6 == 5
				&& Records[2].Purpose
					== EMatchPlayCurrentAttackPostRouteRollPurpose
						::OneOnOneDirectShotDefense
				&& Records[2].RawD6 == 1);
		TestTrue(TEXT("Owned OneOnOne pending overrides fully consumed"),
			Host->GetLocalDevPendingRollOverrides().IsEmpty());
	}

	{
		const int32 Seed = 20613;
		const FName SkillId(TEXT("Skill.DevOverride.Feet"));
		const auto Rules = MakeRules(SkillId, ESkillRuleType::ThroughBall);
		const auto Input = MakeInput(TEXT("DevOverrideFeet"), SkillId);
		FFMCodexLocalMatchD6Provider DirectProvider(Seed);
		FMatchPlayAuthoritativeSession Direct(
			DirectProvider, DirectProvider, Rules);
		FScopedWorld World;
		auto* Host = World.GetHost();
		TestNotNull(TEXT("Feet override Host exists"), Host);
		if (Host == nullptr)
		{
			return false;
		}
		FReadyTrace Trace;
		TestTrue(TEXT("Feet override reaches ready state"),
			BuildReadyForResolution(
				*Host, Direct, Input, Rules, Seed,
				ESkillRuleType::ThroughBall, false, Trace));
		TestTrue(TEXT("Feet route/attack/defense overrides set"),
			Set(*Host, ETarget::ThroughBallRoute, 1)
				&& Set(*Host, ETarget::ThroughBallFeetAttack, 6)
				&& Set(*Host, ETarget::ThroughBallFeetDefense, 1));
		const FMatchPlayState ReadyState = Host->GetMatchSnapshot().Snapshot;
		FMatchPlayAuthoritativeResolveThroughBallInitialRouteRollRequest
			RouteRequest;
		RouteRequest.AttackSequence = ReadyState.CurrentAttack.AttackSequence;
		RouteRequest.RequestingSide = Trace.Attacker;
		TestTrue(TEXT("Feet authority resolves overridden typed route"),
			Host->ResolveThroughBallInitialRouteRoll(RouteRequest).bSuccess);
		const FMatchPlayState RouteState = Host->GetMatchSnapshot().Snapshot;
		TestEqual(TEXT("Canonical route 1 is Feet"),
			RouteState.CurrentAttack.ResolutionSession.ActualBranch.ThroughBall,
			EMatchPlayThroughBallActualBranch::Feet);
		FMatchPlayAuthoritativeResolveThroughBallFeetAttackRollRequest Attack;
		Attack.AttackSequence = RouteState.CurrentAttack.AttackSequence;
		Attack.RequestingSide = Trace.Attacker;
		FMatchPlayAuthoritativeResolveThroughBallFeetDefenseRollRequest Defense;
		Defense.AttackSequence = RouteState.CurrentAttack.AttackSequence;
		Defense.RequestingSide = Trace.Defender;
		FMatchPlayAuthoritativeResolveThroughBallFeetAttackRollRequest StaleAttack =
			Attack;
		++StaleAttack.AttackSequence;
		TestFalse(TEXT("Stale Feet Attack override command rejects"),
			Host->ResolveThroughBallFeetAttackRoll(StaleAttack).bSuccess);
		TestTrue(TEXT("Stale Feet Attack retains pending override"),
			Host->GetLocalDevPendingRollOverrides().ContainsByPredicate(
				[](const FFMCodexLocalDevPendingRollOverride& Item)
				{
					return Item.Target == ETarget::ThroughBallFeetAttack;
				}));
		TestTrue(TEXT("Feet typed Attack authority roll succeeds"),
			Host->ResolveThroughBallFeetAttackRoll(Attack).bSuccess);
		FMatchPlayAuthoritativeResolveThroughBallFeetDefenseRollRequest
			StaleDefense = Defense;
		++StaleDefense.AttackSequence;
		TestFalse(TEXT("Stale Feet Defense override command rejects"),
			Host->ResolveThroughBallFeetDefenseRoll(StaleDefense).bSuccess);
		TestTrue(TEXT("Stale Feet Defense retains pending override"),
			Host->GetLocalDevPendingRollOverrides().ContainsByPredicate(
				[](const FFMCodexLocalDevPendingRollOverride& Item)
				{
					return Item.Target == ETarget::ThroughBallFeetDefense;
				}));
		TestTrue(TEXT("Feet typed Defense authority roll succeeds"),
			Host->ResolveThroughBallFeetDefenseRoll(Defense).bSuccess);
		const FMatchPlayState RollState = Host->GetMatchSnapshot().Snapshot;
		const auto& Records = RollState.CurrentAttack.ResolutionSession
			.PostRouteRollProgress.RollRecords;
		TestEqual(TEXT("Feet attack authority RawD6 is 6"),
			Records[0].RawD6, 6);
		TestEqual(TEXT("Feet defense authority RawD6 is 1"),
			Records[1].RawD6, 1);
		const auto Formula = Host->ResolveThroughBallFeetFormula();
		TestTrue(TEXT("Normal Feet Formula executes"), Formula.bSuccess);
		TestEqual(TEXT("Formula plan consumes authoritative attack 6"),
			Formula.AuthoritativeResult.OrchestrationResult
				.PlanRegenerationResult.PlanResult.FormulaPlan.AttackD6, 6);
		TestEqual(TEXT("Formula plan consumes authoritative defense 1"),
			Formula.AuthoritativeResult.OrchestrationResult
				.PlanRegenerationResult.PlanResult.FormulaPlan.DefenseD6, 1);
		const auto View = FFMCodexLocalMatchInteractionViewBuilder::Build(
			Host->GetMatchSnapshot().Snapshot, Rules);
		TestTrue(TEXT("Presentation facts retain authoritative Feet rolls"),
			View.AcceptedRolls.ContainsByPredicate([](const auto& Roll)
			{
				return Roll.Group == EFMCodexLocalMatchRollGroup::PostRoute
					&& Roll.Purpose == TEXT("Primary Attack")
					&& Roll.RawD6 == 6;
			}));
		TestTrue(TEXT("Feet pending map fully consumed"),
			Host->GetLocalDevPendingRollOverrides().IsEmpty());
	}

	{
		const int32 Seed = 30613;
		const FName SkillId(TEXT("Skill.DevOverride.Cross"));
		const auto Rules = MakeRules(SkillId, ESkillRuleType::Cross);
		const auto Input = MakeInput(TEXT("DevOverrideCross"), SkillId);
		FFMCodexLocalMatchD6Provider DirectProvider(Seed);
		FMatchPlayAuthoritativeSession Direct(
			DirectProvider, DirectProvider, Rules);
		FScopedWorld World;
		auto* Host = World.GetHost();
		TestNotNull(TEXT("Cross override Host exists"), Host);
		if (Host == nullptr)
		{
			return false;
		}
		FReadyTrace Trace;
		TestTrue(TEXT("Cross override reaches ready state"),
			BuildReadyForResolution(
				*Host, Direct, Input, Rules, Seed,
				ESkillRuleType::Cross, false, Trace));
		TestTrue(TEXT("Cross semantic overrides set"),
			Set(*Host, ETarget::CrossRoute, 1)
				&& Set(*Host, ETarget::CrossHighAttack, 6)
				&& Set(*Host, ETarget::CrossHighDefense, 1));
		TestTrue(TEXT("Cross authority route succeeds"),
			Host->BeginResolutionSession().bSuccess
				&& Host->ResolveInitialRoute().bSuccess);
		const FMatchPlayState RouteState = Host->GetMatchSnapshot().Snapshot;
		TestEqual(TEXT("Cross authority route RawD6 is 1"),
			RouteState.CurrentAttack.ResolutionSession
				.InitialRouteRollRecords[0].RawD6, 1);
		TestEqual(TEXT("Canonical Cross mapping preserves intended High"),
			RouteState.CurrentAttack.ResolutionSession.ActualBranch.Cross,
			EMatchPlayCrossActualBranch::High);
		FMatchPlayAuthoritativeResolveCrossHighAttackRollRequest Attack;
		Attack.RequestingSide = Trace.Attacker;
		FMatchPlayAuthoritativeResolveCrossHighDefenseRollRequest Defense;
		Defense.RequestingSide = Trace.Defender;
		TestTrue(TEXT("Cross typed authority rolls succeed"),
			Host->ResolveCrossHighAttackRoll(Attack).bSuccess
				&& Host->ResolveCrossHighDefenseRoll(Defense).bSuccess);
		const auto& Records = Host->GetMatchSnapshot().Snapshot.CurrentAttack
			.ResolutionSession.PostRouteRollProgress.RollRecords;
		TestEqual(TEXT("Cross attack authority RawD6 is 6"),
			Records[0].RawD6, 6);
		TestEqual(TEXT("Cross defense authority RawD6 is 1"),
			Records[1].RawD6, 1);
		const auto View = FFMCodexLocalMatchInteractionViewBuilder::Build(
			Host->GetMatchSnapshot().Snapshot, Rules);
		TestTrue(TEXT("Cross presentation receives route reel target 1"),
			View.AcceptedRolls.ContainsByPredicate([](const auto& Roll)
			{
				return Roll.Group == EFMCodexLocalMatchRollGroup::InitialRoute
					&& Roll.RawD6 == 1;
			}));
		TestTrue(TEXT("Cross presentation receives attack reel target 6"),
			View.AcceptedRolls.ContainsByPredicate([](const auto& Roll)
			{
				return Roll.Group == EFMCodexLocalMatchRollGroup::PostRoute
					&& Roll.Purpose == TEXT("Primary Attack")
					&& Roll.RawD6 == 6;
			}));
		TestTrue(TEXT("Cross pending map fully consumed"),
			Host->GetLocalDevPendingRollOverrides().IsEmpty());
	}

	{
		const int32 Seed = 35613;
		const FName SkillId(TEXT("Skill.DevOverride.CutInsideDirect"));
		const auto Rules = MakeRules(SkillId, ESkillRuleType::CutInsideShot);
		const auto Input = MakeInput(TEXT("DevOverrideCutInsideDirect"), SkillId);
		FFMCodexLocalMatchD6Provider DirectProvider(Seed);
		FMatchPlayAuthoritativeSession Direct(
			DirectProvider, DirectProvider, Rules);
		FScopedWorld World;
		auto* Host = World.GetHost();
		TestNotNull(TEXT("CutInside Direct override Host exists"), Host);
		if (Host == nullptr)
		{
			return false;
		}
		FReadyTrace Trace;
		TestTrue(TEXT("CutInside Direct override reaches ready state"),
			BuildReadyForResolution(
				*Host, Direct, Input, Rules, Seed,
				ESkillRuleType::CutInsideShot, false, Trace));
		TestTrue(TEXT("CutInside Direct route resolves without gameplay RNG"),
			Host->BeginResolutionSession().bSuccess
				&& Host->ResolveIntentDeterminedRoute().bSuccess);
		TestTrue(TEXT("CutInside Direct semantic overrides set"),
			Set(*Host, ETarget::CutInsideShotDirectAttack, 3)
				&& Set(*Host, ETarget::CutInsideShotDirectDefense, 4));

		const FMatchPlayState ReadyState = Host->GetMatchSnapshot().Snapshot;
		FMatchPlayAuthoritativeResolveCutInsideShotDirectAttackRollRequest Attack;
		Attack.AttackSequence = ReadyState.CurrentAttack.AttackSequence;
		Attack.RequestingSide = Trace.Defender;
		TestFalse(TEXT("Wrong-side CutInside Attack override command rejects"),
			Host->ResolveCutInsideShotDirectAttackRoll(Attack).bSuccess);
		TestTrue(TEXT("Wrong-side CutInside Attack retains override"),
			Host->GetLocalDevPendingRollOverrides().ContainsByPredicate(
				[](const FFMCodexLocalDevPendingRollOverride& Item)
				{
					return Item.Target
						== ETarget::CutInsideShotDirectAttack;
				}));
		Attack.RequestingSide = Trace.Attacker;
		++Attack.AttackSequence;
		TestFalse(TEXT("Stale CutInside Attack override command rejects"),
			Host->ResolveCutInsideShotDirectAttackRoll(Attack).bSuccess);
		TestTrue(TEXT("Stale CutInside Attack retains override"),
			Host->GetLocalDevPendingRollOverrides().ContainsByPredicate(
				[](const FFMCodexLocalDevPendingRollOverride& Item)
				{
					return Item.Target
						== ETarget::CutInsideShotDirectAttack;
				}));

		FMatchPlayAuthoritativeResolveCutInsideShotDirectDefenseRollRequest Defense;
		Defense.AttackSequence = ReadyState.CurrentAttack.AttackSequence;
		Defense.RequestingSide = Trace.Defender;
		TestFalse(TEXT("Premature CutInside Defense override command rejects"),
			Host->ResolveCutInsideShotDirectDefenseRoll(Defense).bSuccess);
		TestTrue(TEXT("Premature CutInside Defense retains override"),
			Host->GetLocalDevPendingRollOverrides().ContainsByPredicate(
				[](const FFMCodexLocalDevPendingRollOverride& Item)
				{
					return Item.Target
						== ETarget::CutInsideShotDirectDefense;
				}));

		Attack.AttackSequence = ReadyState.CurrentAttack.AttackSequence;
		TestTrue(TEXT("Accepted CutInside Attack consumes Attack override"),
			Host->ResolveCutInsideShotDirectAttackRoll(Attack).bSuccess);
		const FMatchPlayState AttackState = Host->GetMatchSnapshot().Snapshot;
		const auto& AttackRecords = AttackState.CurrentAttack.ResolutionSession
			.PostRouteRollProgress.RollRecords;
		TestTrue(TEXT("CutInside Attack override commits only raw 3"),
			AttackRecords.Num() == 1 && AttackRecords[0].RawD6 == 3);
		TestEqual(TEXT("CutInside Attack override remains non-terminal"),
			AttackState.CurrentAttack.LifecycleState,
			EMatchPlayCurrentAttackLifecycleState::Active);
		TestFalse(TEXT("Accepted CutInside Attack clears Attack override"),
			Host->GetLocalDevPendingRollOverrides().ContainsByPredicate(
				[](const FFMCodexLocalDevPendingRollOverride& Item)
				{
					return Item.Target
						== ETarget::CutInsideShotDirectAttack;
				}));
		TestTrue(TEXT("CutInside Defense override remains pending"),
			Host->GetLocalDevPendingRollOverrides().ContainsByPredicate(
				[](const FFMCodexLocalDevPendingRollOverride& Item)
				{
					return Item.Target
						== ETarget::CutInsideShotDirectDefense;
				}));

		Defense.RequestingSide = Trace.Attacker;
		TestFalse(TEXT("Wrong-side CutInside Defense override command rejects"),
			Host->ResolveCutInsideShotDirectDefenseRoll(Defense).bSuccess);
		Defense.RequestingSide = Trace.Defender;
		++Defense.AttackSequence;
		TestFalse(TEXT("Stale CutInside Defense override command rejects"),
			Host->ResolveCutInsideShotDirectDefenseRoll(Defense).bSuccess);
		TestTrue(TEXT("Rejected CutInside Defense commands retain override"),
			Host->GetLocalDevPendingRollOverrides().ContainsByPredicate(
				[](const FFMCodexLocalDevPendingRollOverride& Item)
				{
					return Item.Target
						== ETarget::CutInsideShotDirectDefense;
				}));
		Defense.AttackSequence = ReadyState.CurrentAttack.AttackSequence;
		const auto DefenseResult =
			Host->ResolveCutInsideShotDirectDefenseRoll(Defense);
		const FMatchPlayState TerminalState = Host->GetMatchSnapshot().Snapshot;
		const auto& TerminalRecords = TerminalState.CurrentAttack.ResolutionSession
			.PostRouteRollProgress.RollRecords;
		TestTrue(TEXT("Accepted CutInside Defense consumes Defense override"),
			DefenseResult.bSuccess);
		TestTrue(TEXT("CutInside Direct override values persist in order"),
			TerminalRecords.Num() == 2
				&& TerminalRecords[0].RawD6 == 3
				&& TerminalRecords[1].RawD6 == 4);
		TestEqual(TEXT("CutInside Direct override stops at explicit NextRound"),
			TerminalState.CurrentAttack.LifecycleState,
			EMatchPlayCurrentAttackLifecycleState::TerminalPendingAdvance);
		TestTrue(TEXT("CutInside Direct pending overrides fully consumed"),
			Host->GetLocalDevPendingRollOverrides().IsEmpty());
	}

	{
		const int32 Seed = 36613;
		const FName SkillId(TEXT("Skill.DevOverride.CutInsideDeadCorner"));
		const auto Rules = MakeRules(SkillId, ESkillRuleType::CutInsideShot);
		const auto Input = MakeInput(
			TEXT("DevOverrideCutInsideDeadCorner"), SkillId);
		FFMCodexLocalMatchD6Provider DirectProvider(Seed);
		FMatchPlayAuthoritativeSession Direct(
			DirectProvider, DirectProvider, Rules);
		FScopedWorld World;
		auto* Host = World.GetHost();
		TestNotNull(TEXT("CutInside DeadCorner override Host exists"), Host);
		if (Host == nullptr)
		{
			return false;
		}
		FReadyTrace Trace;
		TestTrue(TEXT("CutInside DeadCorner override reaches ready state"),
			BuildReadyForResolution(
				*Host, Direct, Input, Rules, Seed,
				ESkillRuleType::CutInsideShot, false, Trace,
				EMatchPlayElectiveBranchIntent::DeadCorner));
		TestTrue(TEXT("CutInside DeadCorner route resolves without gameplay RNG"),
			Host->BeginResolutionSession().bSuccess
				&& Host->ResolveIntentDeterminedRoute().bSuccess);
		TestTrue(TEXT("CutInside DeadCorner paired overrides set"),
			Set(*Host, ETarget::CutInsideShotDeadCornerA, 6)
				&& Set(*Host, ETarget::CutInsideShotDeadCornerB, 5));

		const FMatchPlayState ReadyState = Host->GetMatchSnapshot().Snapshot;
		FMatchPlayAuthoritativeResolveCutInsideShotDeadCornerRollRequest Request;
		Request.AttackSequence = ReadyState.CurrentAttack.AttackSequence;
		Request.RequestingSide = Trace.Defender;
		TestFalse(TEXT("Wrong-side CutInside DeadCorner override rejects"),
			Host->ResolveCutInsideShotDeadCornerRoll(Request).bSuccess);
		Request.RequestingSide = Trace.Attacker;
		++Request.AttackSequence;
		TestFalse(TEXT("Stale CutInside DeadCorner override rejects"),
			Host->ResolveCutInsideShotDeadCornerRoll(Request).bSuccess);
		TestEqual(TEXT("Rejected DeadCorner requests retain both overrides"),
			Host->GetLocalDevPendingRollOverrides().Num(), 2);

		Request.AttackSequence = ReadyState.CurrentAttack.AttackSequence;
		const auto Outcome = Host->ResolveCutInsideShotDeadCornerRoll(Request);
		const FMatchPlayState TerminalState = Host->GetMatchSnapshot().Snapshot;
		const auto& Records = TerminalState.CurrentAttack.ResolutionSession
			.PostRouteRollProgress.RollRecords;
		TestTrue(TEXT("Accepted CutInside DeadCorner consumes paired overrides"),
			Outcome.bSuccess
				&& Outcome.AuthoritativeResult.TerminalResult.bIsGoal);
		TestTrue(TEXT("CutInside DeadCorner override pair persists atomically"),
			Records.Num() == 2
				&& Records[0].RawD6 == 6
				&& Records[1].RawD6 == 5);
		TestEqual(TEXT("CutInside DeadCorner override stops at NextRound"),
			TerminalState.CurrentAttack.LifecycleState,
			EMatchPlayCurrentAttackLifecycleState::TerminalPendingAdvance);
		TestTrue(TEXT("CutInside DeadCorner overrides fully consumed"),
			Host->GetLocalDevPendingRollOverrides().IsEmpty());
	}

	{
		const int32 Seed = 40613;
		const auto Input = MakeInput(TEXT("DevOverrideTactical"), NAME_None);
		FScopedWorld World;
		auto* Host = World.GetHost();
		TestNotNull(TEXT("Tactical override Host exists"), Host);
		if (Host == nullptr)
		{
			return false;
		}
		const FSkillRuleSnapshotSet Rules;
		TestTrue(TEXT("Tactical match starts"),
			Host->StartNewLocalMatch(Input, Rules, Seed).bSuccess);
		TestTrue(TEXT("Tactical 8 override set"),
			Set(*Host, ETarget::TacticalPoint, 8));
		const EInitialTurnOrderPlayer Attacker = Host->GetMatchSnapshot().Snapshot
			.RuntimeState.CurrentAttackingPlayer;
		const auto Roll = Host->RollTacticalPoints(Attacker);
		TestTrue(TEXT("Real tactical authority command succeeds"), Roll.bSuccess);
		TestEqual(TEXT("Authority tactical output is overridden 8"),
			Roll.TacticalPoints, 8);
		TestEqual(TEXT("CurrentAttack stores the same tactical point"),
			Host->GetMatchSnapshot().Snapshot.CurrentAttack.ActionPoint, 8);
		TestTrue(TEXT("Tactical override auto-clears"),
			Host->GetLocalDevPendingRollOverrides().IsEmpty());
	}
	return true;
}

#endif
