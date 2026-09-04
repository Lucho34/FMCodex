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

namespace FMCodexLocalMatchHostTests
{
	FPlayerCardData MakeDeckCard(
		const FString& CardId,
		const ECardRarity Rarity,
		const bool bIsGoalkeeper)
	{
		FPlayerCardData Card;
		Card.CardId = FName(*CardId);
		Card.Rarity = Rarity;
		Card.bIsGoalkeeper = bIsGoalkeeper;
		Card.PositionTypes = {
			bIsGoalkeeper
				? EPlayerPositionType::Goalkeeper
				: EPlayerPositionType::Attack
		};
		return Card;
	}

	TArray<FPlayerCardData> MakeValidDeck(
		const FString& Prefix,
		const ECardRarity Rarity)
	{
		TArray<FPlayerCardData> Deck;
		Deck.Reserve(20);
		for (int32 Index = 0; Index < 19; ++Index)
		{
			Deck.Add(MakeDeckCard(
				FString::Printf(TEXT("%s_OUT_%02d"), *Prefix, Index),
				Rarity,
				false));
		}
		Deck.Add(MakeDeckCard(
			FString::Printf(TEXT("%s_GK"), *Prefix),
			Rarity,
			true));
		return Deck;
	}

	FMatchPlayOpeningInitializeInput MakeValidInput(
		const FString& Prefix)
	{
		FMatchPlayOpeningInitializeInput Input;
		Input.OpeningInput.PlayerADeck = MakeValidDeck(
			Prefix + TEXT("_A"), ECardRarity::Common);
		Input.OpeningInput.PlayerBDeck = MakeValidDeck(
			Prefix + TEXT("_B"), ECardRarity::Common);
		Input.OpeningInput.PlayerAAttackCountD6Roll = 1;
		Input.OpeningInput.PlayerBAttackCountD6Roll = 1;
		Input.OpeningInput.PlayerATieBreakerRoll = 2;
		Input.OpeningInput.PlayerBTieBreakerRoll = 6;

		FMatchPlayDeploymentSlotDefinition PlayerASlot;
		PlayerASlot.SlotId = FName(*FString::Printf(
			TEXT("%s_SlotA"), *Prefix));
		PlayerASlot.NeutralSide =
			EMatchPlayNeutralSlotSide::NearPlayerA;
		FMatchPlayDeploymentSlotDefinition PlayerBSlot;
		PlayerBSlot.SlotId = FName(*FString::Printf(
			TEXT("%s_SlotB"), *Prefix));
		PlayerBSlot.NeutralSide =
			EMatchPlayNeutralSlotSide::NearPlayerB;
		Input.DeploymentSlotCatalog.Slots = {
			PlayerASlot,
			PlayerBSlot
		};
		return Input;
	}

	FMatchPlayOpeningInitializeInput MakeInteractionInput(
		const FString& Prefix,
		const FName SkillId = NAME_None)
	{
		FMatchPlayOpeningInitializeInput Input = MakeValidInput(Prefix);
		for (TArray<FPlayerCardData>* Deck : {
			&Input.OpeningInput.PlayerADeck,
			&Input.OpeningInput.PlayerBDeck })
		{
			for (FPlayerCardData& Card : *Deck)
			{
				if (!Card.bIsGoalkeeper)
				{
					Card.PositionTypes = {
						EPlayerPositionType::Attack,
						EPlayerPositionType::Midfield,
						EPlayerPositionType::Defense
					};
					if (!SkillId.IsNone())
					{
						Card.AttackSkillIds = { SkillId };
					}
				}
			}
		}

		Input.DeploymentSlotCatalog.Slots.Reset();
		for (int32 Index = 0; Index < 3; ++Index)
		{
			FMatchPlayDeploymentSlotDefinition NearA;
			NearA.SlotId = FName(*FString::Printf(
				TEXT("%s_NearA_%d"), *Prefix, Index));
			NearA.NeutralSide = EMatchPlayNeutralSlotSide::NearPlayerA;
			Input.DeploymentSlotCatalog.Slots.Add(NearA);
			FMatchPlayDeploymentSlotDefinition NearB;
			NearB.SlotId = FName(*FString::Printf(
				TEXT("%s_NearB_%d"), *Prefix, Index));
			NearB.NeutralSide = EMatchPlayNeutralSlotSide::NearPlayerB;
			Input.DeploymentSlotCatalog.Slots.Add(NearB);
		}
		return Input;
	}

	FSkillRuleSnapshotSet MakeSkillRuleSet(
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

	const TArray<FName>& AvailableCards(
		const FMatchPlayState& State,
		const EInitialTurnOrderPlayer Side)
	{
		return Side == EInitialTurnOrderPlayer::PlayerA
			? State.CardUsageState.PlayerACardUsageState.AvailableCardIds
			: State.CardUsageState.PlayerBCardUsageState.AvailableCardIds;
	}

	bool FindLegalDeployment(
		const FMatchPlayState& State,
		const EMatchPlayRelativeDeploymentZone PreferredZone,
		FMatchPlayAuthoritativeDeployOrdinaryRequest& OutRequest)
	{
		if (!State.bHasCurrentAttack)
		{
			return false;
		}
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
					OutRequest.ExpectedAttackSequence =
						State.CurrentAttack.AttackSequence;
					OutRequest.RequestingSide = Side;
					OutRequest.CardId = CardId;
					OutRequest.SlotId = Slot.SlotId;
					return true;
				}
			}
		}
		return false;
	}

	bool FindLegalCarrier(
		const FMatchPlayState& State,
		const EInitialTurnOrderPlayer AttackingSide,
		FName& OutCardId)
	{
		const auto Availability =
			FMatchPlayCurrentAttackCarrierSelectionAvailability::Query(
				State,
				State.CurrentAttack.AttackSequence,
				AttackingSide);
		for (const auto& Candidate : Availability.Candidates)
		{
			if (Candidate.LegalityResult.bIsLegal)
			{
				OutCardId = Candidate.CarrierCardId;
				return true;
			}
		}
		return false;
	}

	bool FindLegalMarker(
		const FMatchPlayState& State,
		const EInitialTurnOrderPlayer DefendingSide,
		FName& OutCardId)
	{
		const auto Availability =
			FMatchPlayCurrentAttackMarkerSelectionAvailability::Query(
				State,
				State.CurrentAttack.AttackSequence,
				DefendingSide);
		for (const auto& Candidate : Availability.Candidates)
		{
			if (Candidate.LegalityResult.bIsLegal)
			{
				OutCardId = Candidate.MarkerCardId;
				return true;
			}
		}
		return false;
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

	bool LoadProductionSource(
		const TCHAR* RelativePath,
		FString& OutSource)
	{
		return FFileHelper::LoadFileToString(
			OutSource,
			*FPaths::ConvertRelativePathToFull(
				FPaths::ProjectDir() / RelativePath));
	}

	int32 CountOccurrences(
		const FString& Text,
		const FString& Needle)
	{
		int32 Count = 0;
		int32 SearchFrom = 0;
		while ((SearchFrom = Text.Find(
			Needle,
			ESearchCase::CaseSensitive,
			ESearchDir::FromStart,
			SearchFrom)) != INDEX_NONE)
		{
			++Count;
			SearchFrom += Needle.Len();
		}
		return Count;
	}

	class FScopedLocalMatchTestWorld final
	{
	public:
		FScopedLocalMatchTestWorld()
		{
			World = UWorld::CreateWorld(EWorldType::Game, false);
			if (World != nullptr && GEngine != nullptr)
			{
				FWorldContext& WorldContext =
					GEngine->CreateNewWorldContext(EWorldType::Game);
				WorldContext.SetCurrentWorld(World);
				Host = World->SpawnActor<
					AFMCodexLocalMatchHostGameMode>();
			}
		}

		~FScopedLocalMatchTestWorld()
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

		AFMCodexLocalMatchHostGameMode* GetHost() const
		{
			return Host;
		}

	private:
		UWorld* World = nullptr;
		AFMCodexLocalMatchHostGameMode* Host = nullptr;
	};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexLocalMatchHostSurfaceAndFailureTest,
	"FMCodex.LocalPlay.LocalMatchHost.01.SurfaceAndPreInitialization",
	EAutomationTestFlags::EditorContext
		| EAutomationTestFlags::EngineFilter)

bool FFMCodexLocalMatchHostSurfaceAndFailureTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexLocalMatchHostTests;
	using FStartMethod = FFMCodexStartNewLocalMatchResult
		(AFMCodexLocalMatchHostGameMode::*)(
			const FMatchPlayOpeningInitializeInput&);
	using FSnapshotMethod = FFMCodexLocalMatchSnapshotResult
		(AFMCodexLocalMatchHostGameMode::*)() const;
	using FBeginMethod = FFMCodexLocalMatchBeginOrdinaryAttackResult
		(AFMCodexLocalMatchHostGameMode::*)(int32);
	using FRollTacticalPointsMethod =
		FFMCodexLocalMatchRollTacticalPointsResult
		(AFMCodexLocalMatchHostGameMode::*)(EInitialTurnOrderPlayer);
	using FOneOnOneChoiceMethod =
		FFMCodexLocalMatchSubmitThroughBallOneOnOneShotChoiceResult
		(AFMCodexLocalMatchHostGameMode::*)(
			const FMatchPlayAuthoritativeSubmitThroughBallOneOnOneShotChoiceRequest&);
	TestTrue(TEXT("StartNewLocalMatch is a typed canonical-input method"),
		(std::is_same_v<
			decltype(static_cast<FStartMethod>(
				&AFMCodexLocalMatchHostGameMode::StartNewLocalMatch)),
			FStartMethod>));
	TestTrue(TEXT("Snapshot is returned through one const by-value method"),
		(std::is_same_v<
			decltype(&AFMCodexLocalMatchHostGameMode::GetMatchSnapshot),
			FSnapshotMethod>));
	TestTrue(TEXT("BeginOrdinaryAttack forwards only ActionPoint"),
		(std::is_same_v<
			decltype(&AFMCodexLocalMatchHostGameMode::BeginOrdinaryAttack),
			FBeginMethod>));
	TestTrue(TEXT("Production Tactical Point command requires requesting side"),
		(std::is_same_v<
			decltype(&AFMCodexLocalMatchHostGameMode::RollTacticalPoints),
			FRollTacticalPointsMethod>));
	TestTrue(TEXT("OneOnOne choice preserves its exact typed request/result"),
		(std::is_same_v<
			decltype(&AFMCodexLocalMatchHostGameMode
				::SubmitThroughBallOneOnOneShotChoice),
			FOneOnOneChoiceMethod>));

	FString Header;
	FString Source;
	TestTrue(TEXT("Local host header loads"), LoadProductionSource(
		TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchHostGameMode.h"),
		Header));
	TestTrue(TEXT("Local host source loads"), LoadProductionSource(
		TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchHostGameMode.cpp"),
		Source));
	TestEqual(TEXT("Host owns exactly one authoritative Session"),
		CountOccurrences(Header,
			TEXT("FMatchPlayAuthoritativeSession AuthoritativeSession;")),
		1);
	TestEqual(TEXT("Host owns exactly one provider composition"),
		CountOccurrences(Header,
			TEXT("FFMCodexLocalMatchD6Provider D6Provider;")),
		1);
	TestFalse(TEXT("No public GetSession escape exists"),
		Header.Contains(TEXT("GetSession")));
	TestFalse(TEXT("No mutable State reference exists"),
		Header.Contains(TEXT("FMatchPlayState&")));
	TestFalse(TEXT("No generic command dispatcher exists"),
		Header.Contains(TEXT("ExecuteCommand"))
			|| Header.Contains(TEXT("ExecuteAction"))
			|| Source.Contains(TEXT("ExecuteCommand"))
			|| Source.Contains(TEXT("ExecuteAction")));
	TestFalse(TEXT("Interaction routing does not consume D6"),
		Source.Contains(TEXT(".RollD6(")));
	for (const TCHAR* CacheName : {
		TEXT("CachedScore"),
		TEXT("CachedCurrentPlayer"),
		TEXT("CachedCurrentAttack"),
		TEXT("CachedSelectionStage") })
	{
		TestFalse(*FString::Printf(TEXT("No duplicate State cache: %s"), CacheName),
			Header.Contains(CacheName) || Source.Contains(CacheName));
	}
	TestEqual(TEXT("InitializeMatch has one host delegation"),
		CountOccurrences(Source,
			TEXT("CandidateRuntime->AuthoritativeSession.InitializeMatch(Input)")),
		1);
	TestEqual(TEXT("Test seam plus production roll have two Begin delegations"),
		CountOccurrences(Source,
			TEXT("ActiveMatchRuntime->AuthoritativeSession.BeginOrdinaryAttack(")),
		2);
	TestEqual(TEXT("Public snapshot, tactical preflight, and DEV route identity have three reads"),
		CountOccurrences(Source,
			TEXT("ActiveMatchRuntime->AuthoritativeSession.GetStateSnapshot()")),
		3);
	for (const TCHAR* Delegation : {
		TEXT(".DeployOrdinary("),
		TEXT(".DeployGoalkeeper("),
		TEXT(".FinishDeployment("),
		TEXT(".SubmitCarrier("),
		TEXT(".ResolveNoLegalCarrier("),
		TEXT(".SubmitMarker("),
		TEXT(".ResolveNoLegalMarker("),
		TEXT(".DeclineMarker("),
		TEXT(".SubmitSkill("),
		TEXT(".ResolveNoLegalSkill("),
		TEXT(".DeclineSkill("),
		TEXT(".SubmitRunner("),
		TEXT(".ResolveNoLegalRunner("),
		TEXT(".DeclineRunner("),
		TEXT(".SubmitHelper("),
		TEXT(".ResolveNoLegalHelper("),
		TEXT(".DeclineHelper("),
		TEXT(".SubmitBranchIntent("),
		TEXT(".SubmitThroughBallOneOnOneShotChoice(") })
	{
		TestEqual(*FString::Printf(
			TEXT("Exactly one typed Session delegation exists: %s"),
			Delegation),
			CountOccurrences(Source, Delegation),
			1);
	}
	TestEqual(TEXT("Provider and Session adopt through one runtime bundle"),
		CountOccurrences(Source,
			TEXT("ActiveMatchRuntime = MoveTemp(CandidateRuntime);")),
		1);
	for (const TCHAR* ForbiddenWrite : {
		TEXT("RuntimeState.Score ="),
		TEXT("CurrentAttackingPlayer ="),
		TEXT("CurrentAttack ="),
		TEXT("bHasCurrentAttack ="),
		TEXT("UsedAttackCount =") })
	{
		TestFalse(*FString::Printf(
			TEXT("Host contains no direct rule write: %s"), ForbiddenWrite),
			Source.Contains(ForbiddenWrite));
	}

	FScopedLocalMatchTestWorld TestWorld;
	AFMCodexLocalMatchHostGameMode* Host = TestWorld.GetHost();
	TestNotNull(TEXT("A map-lifetime GameMode host can be spawned"), Host);
	if (Host == nullptr)
	{
		return false;
	}
	TestFalse(TEXT("Host begins without an active match"),
		Host->HasActiveLocalMatch());
	const FFMCodexLocalMatchSnapshotResult Snapshot =
		Host->GetMatchSnapshot();
	TestFalse(TEXT("Pre-start snapshot fails"), Snapshot.bSuccess);
	TestEqual(TEXT("Pre-start snapshot exact host error"),
		Snapshot.ErrorCode,
		EFMCodexLocalMatchHostErrorCode::NoActiveMatch);
	const FFMCodexLocalMatchBeginOrdinaryAttackResult Begin =
		Host->BeginOrdinaryAttack(6);
	TestFalse(TEXT("Pre-start Begin fails"), Begin.bSuccess);
	TestEqual(TEXT("Pre-start Begin exact host error"),
		Begin.ErrorCode,
		EFMCodexLocalMatchHostErrorCode::NoActiveMatch);
	TestEqual(TEXT("Pre-start Begin does not invent a Session command"),
		Begin.AuthoritativeResult.RuntimeEnvelope.CommandKind,
		EMatchPlayAuthoritativeCommandKind::None);

	const FSkillRuleSnapshotSet EmptyRules;
	auto TestNoActiveCommand = [this](
		const TCHAR* Label,
		const auto& Result)
	{
		TestFalse(*FString::Printf(TEXT("%s fails before start"), Label),
			Result.bSuccess);
		TestEqual(*FString::Printf(TEXT("%s reports NoActiveMatch"), Label),
			Result.ErrorCode,
			EFMCodexLocalMatchHostErrorCode::NoActiveMatch);
		TestEqual(*FString::Printf(TEXT("%s never calls Session"), Label),
			Result.AuthoritativeResult.RuntimeEnvelope.CommandKind,
			EMatchPlayAuthoritativeCommandKind::None);
	};
	TestNoActiveCommand(TEXT("DeployOrdinary"), Host->DeployOrdinary({}));
	TestNoActiveCommand(TEXT("DeployGoalkeeper"), Host->DeployGoalkeeper({}));
	TestNoActiveCommand(TEXT("FinishDeployment"), Host->FinishDeployment(
		0, EInitialTurnOrderPlayer::None));
	TestNoActiveCommand(TEXT("SubmitCarrier"), Host->SubmitCarrier({}));
	TestNoActiveCommand(TEXT("ResolveNoLegalCarrier"),
		Host->ResolveNoLegalCarrier());
	TestNoActiveCommand(TEXT("SubmitMarker"), Host->SubmitMarker({}));
	TestNoActiveCommand(TEXT("ResolveNoLegalMarker"),
		Host->ResolveNoLegalMarker());
	TestNoActiveCommand(TEXT("DeclineMarker"), Host->DeclineMarker({}));
	TestNoActiveCommand(TEXT("SubmitSkill"),
		Host->SubmitSkill(EmptyRules, {}));
	TestNoActiveCommand(TEXT("ResolveNoLegalSkill"),
		Host->ResolveNoLegalSkill(EmptyRules));
	TestNoActiveCommand(TEXT("DeclineSkill"),
		Host->DeclineSkill(EmptyRules, {}));
	TestNoActiveCommand(TEXT("SubmitRunner"), Host->SubmitRunner({}));
	TestNoActiveCommand(TEXT("ResolveNoLegalRunner"),
		Host->ResolveNoLegalRunner());
	TestNoActiveCommand(TEXT("DeclineRunner"), Host->DeclineRunner({}));
	TestNoActiveCommand(TEXT("SubmitHelper"), Host->SubmitHelper({}));
	TestNoActiveCommand(TEXT("ResolveNoLegalHelper"),
		Host->ResolveNoLegalHelper());
	TestNoActiveCommand(TEXT("DeclineHelper"), Host->DeclineHelper({}));
	TestNoActiveCommand(TEXT("SubmitBranchIntent"),
		Host->SubmitBranchIntent({}));
	TestNoActiveCommand(TEXT("SubmitThroughBallOneOnOneShotChoice"),
		Host->SubmitThroughBallOneOnOneShotChoice({}));

	const FFMCodexStartNewLocalMatchResult InvalidStart =
		Host->StartNewLocalMatch({});
	TestFalse(TEXT("Invalid canonical initialization is preserved"),
		InvalidStart.bSuccess);
	TestEqual(TEXT("Invalid start exact host error"),
		InvalidStart.ErrorCode,
		EFMCodexLocalMatchHostErrorCode
			::AuthoritativeInitializationFailed);
	TestTrue(TEXT("Invalid start preserves authoritative diagnostics"),
		InvalidStart.AuthoritativeResult.RuntimeEnvelope.bAccepted
			&& !InvalidStart.AuthoritativeResult.OpeningResult.bSuccess
			&& !InvalidStart.AuthoritativeResult.OpeningResult
				.ErrorMessage.IsEmpty());
	TestFalse(TEXT("Invalid start retains no failed Session"),
		Host->HasActiveLocalMatch());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexLocalMatchHostLifecycleEquivalenceTest,
	"FMCodex.LocalPlay.LocalMatchHost.02.LifecycleEquivalenceAndReset",
	EAutomationTestFlags::EditorContext
		| EAutomationTestFlags::EngineFilter)

bool FFMCodexLocalMatchHostLifecycleEquivalenceTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexLocalMatchHostTests;
	FScopedLocalMatchTestWorld TestWorld;
	AFMCodexLocalMatchHostGameMode* Host = TestWorld.GetHost();
	TestNotNull(TEXT("Lifecycle host exists"), Host);
	if (Host == nullptr)
	{
		return false;
	}

	const FMatchPlayOpeningInitializeInput InitialInput =
		MakeValidInput(TEXT("LocalHostLifecycle"));
	FMatchPlayAuthoritativeSession DirectSession;
	const auto DirectInitialize = DirectSession.InitializeMatch(InitialInput);
	const auto HostInitialize = Host->StartNewLocalMatch(InitialInput);
	TestTrue(TEXT("Direct Initialize succeeds"),
		DirectInitialize.OpeningResult.bSuccess);
	TestTrue(TEXT("Host StartNewLocalMatch succeeds"),
		HostInitialize.bSuccess);
	TestFalse(TEXT("First start does not replace a match"),
		HostInitialize.bReplacedExistingMatch);
	TestTrue(TEXT("Host now owns an active match"),
		Host->HasActiveLocalMatch());
	const auto HostInitialSnapshot = Host->GetMatchSnapshot();
	TestTrue(TEXT("Initialized snapshot succeeds"),
		HostInitialSnapshot.bSuccess);
	TestTrue(TEXT("Initialize State equals direct Session"),
		AreStatesEqual(
			HostInitialSnapshot.Snapshot,
			DirectSession.GetStateSnapshot()));
	TestTrue(TEXT("Initialized State is canonical"),
		HostInitialSnapshot.Snapshot.RuntimeState.bIsInitialized
			&& !HostInitialSnapshot.Snapshot.bHasCurrentAttack);
	TestEqual(TEXT("Initial Player A score"),
		HostInitialSnapshot.Snapshot.RuntimeState.PlayerAState.Score, 0);
	TestEqual(TEXT("Initial Player B score"),
		HostInitialSnapshot.Snapshot.RuntimeState.PlayerBState.Score, 0);

	const auto DirectBegin = DirectSession.BeginOrdinaryAttack(6);
	const auto HostBegin = Host->BeginOrdinaryAttack(6);
	TestTrue(TEXT("Direct Begin succeeds"), DirectBegin.BeginResult.bSuccess);
	TestTrue(TEXT("Host Begin succeeds"), HostBegin.bSuccess);
	TestEqual(TEXT("Host preserves canonical command kind"),
		HostBegin.AuthoritativeResult.RuntimeEnvelope.CommandKind,
		EMatchPlayAuthoritativeCommandKind::BeginOrdinaryAttack);
	const auto HostActiveSnapshot = Host->GetMatchSnapshot();
	TestTrue(TEXT("Begin State equals direct Session"),
		AreStatesEqual(
			HostActiveSnapshot.Snapshot,
			DirectSession.GetStateSnapshot()));
	TestTrue(TEXT("Host snapshot contains canonical CurrentAttack"),
		HostActiveSnapshot.Snapshot.bHasCurrentAttack);
	TestEqual(TEXT("Host preserves genuine ActionPoint"),
		HostActiveSnapshot.Snapshot.CurrentAttack.ActionPoint, 6);

	const FMatchPlayState BeforeFailedBegin =
		HostActiveSnapshot.Snapshot;
	const auto FailedBegin = Host->BeginOrdinaryAttack(5);
	TestFalse(TEXT("Authoritative Begin domain failure is not hidden"),
		FailedBegin.bSuccess);
	TestEqual(TEXT("Failed Begin exact host error"),
		FailedBegin.ErrorCode,
		EFMCodexLocalMatchHostErrorCode::AuthoritativeCommandFailed);
	TestEqual(TEXT("Failed Begin preserves canonical domain error"),
		FailedBegin.AuthoritativeResult.BeginResult.ErrorCode,
		EMatchPlayBeginOrdinaryAttackErrorCode::CurrentAttackAlreadyActive);
	TestTrue(TEXT("Failed Begin does not mutate hosted State"),
		AreStatesEqual(
			Host->GetMatchSnapshot().Snapshot,
			BeforeFailedBegin));

	const auto FailedReplacement = Host->StartNewLocalMatch({});
	TestFalse(TEXT("Invalid replacement fails"),
		FailedReplacement.bSuccess);
	TestTrue(TEXT("Invalid replacement preserves active match"),
		Host->HasActiveLocalMatch()
			&& AreStatesEqual(
				Host->GetMatchSnapshot().Snapshot,
				BeforeFailedBegin));

	const FMatchPlayOpeningInitializeInput ResetInput =
		MakeValidInput(TEXT("LocalHostReset"));
	FMatchPlayAuthoritativeSession FreshDirectSession;
	const auto FreshDirectInitialize =
		FreshDirectSession.InitializeMatch(ResetInput);
	const auto Reset = Host->StartNewLocalMatch(ResetInput);
	TestTrue(TEXT("Fresh direct Initialize succeeds"),
		FreshDirectInitialize.OpeningResult.bSuccess);
	TestTrue(TEXT("Starting a new match succeeds"), Reset.bSuccess);
	TestTrue(TEXT("Starting a new match reports replacement"),
		Reset.bReplacedExistingMatch);
	const FMatchPlayState ResetSnapshot =
		Host->GetMatchSnapshot().Snapshot;
	TestTrue(TEXT("Reset State equals a freshly constructed Session"),
		AreStatesEqual(
			ResetSnapshot,
			FreshDirectSession.GetStateSnapshot()));
	TestFalse(TEXT("Reset clears stale CurrentAttack"),
		ResetSnapshot.bHasCurrentAttack);
	TestEqual(TEXT("Reset clears Player A used opportunities"),
		ResetSnapshot.RuntimeState.PlayerAState.UsedAttackCount, 0);
	TestEqual(TEXT("Reset clears Player B used opportunities"),
		ResetSnapshot.RuntimeState.PlayerBState.UsedAttackCount, 0);
	TestEqual(TEXT("Reset restores Player A score"),
		ResetSnapshot.RuntimeState.PlayerAState.Score, 0);
	TestEqual(TEXT("Reset restores Player B score"),
		ResetSnapshot.RuntimeState.PlayerBState.Score, 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexLocalMatchHostIsolationTest,
	"FMCodex.LocalPlay.LocalMatchHost.03.TwoHostIsolation",
	EAutomationTestFlags::EditorContext
		| EAutomationTestFlags::EngineFilter)

bool FFMCodexLocalMatchHostIsolationTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexLocalMatchHostTests;
	FScopedLocalMatchTestWorld WorldA;
	FScopedLocalMatchTestWorld WorldB;
	AFMCodexLocalMatchHostGameMode* HostA = WorldA.GetHost();
	AFMCodexLocalMatchHostGameMode* HostB = WorldB.GetHost();
	TestNotNull(TEXT("Isolation Host A exists"), HostA);
	TestNotNull(TEXT("Isolation Host B exists"), HostB);
	if (HostA == nullptr || HostB == nullptr)
	{
		return false;
	}

	const FMatchPlayOpeningInitializeInput Input =
		MakeValidInput(TEXT("LocalHostIsolation"));
	TestTrue(TEXT("Host A starts independently"),
		HostA->StartNewLocalMatch(Input).bSuccess);
	TestFalse(TEXT("Starting Host A leaves Host B inactive"),
		HostB->HasActiveLocalMatch());
	TestTrue(TEXT("Host B starts independently"),
		HostB->StartNewLocalMatch(Input).bSuccess);
	const FMatchPlayState BBeforeA =
		HostB->GetMatchSnapshot().Snapshot;
	TestTrue(TEXT("Host A advances independently"),
		HostA->BeginOrdinaryAttack(6).bSuccess);
	TestTrue(TEXT("Advancing Host A cannot mutate Host B"),
		AreStatesEqual(
			HostB->GetMatchSnapshot().Snapshot,
			BBeforeA));
	const FMatchPlayState ABeforeB =
		HostA->GetMatchSnapshot().Snapshot;
	TestTrue(TEXT("Host B advances independently"),
		HostB->BeginOrdinaryAttack(6).bSuccess);
	TestTrue(TEXT("Advancing Host B cannot mutate Host A"),
		AreStatesEqual(
			HostA->GetMatchSnapshot().Snapshot,
			ABeforeB));
	TestTrue(TEXT("Identical isolated hosts remain deterministic"),
		AreStatesEqual(
			HostA->GetMatchSnapshot().Snapshot,
			HostB->GetMatchSnapshot().Snapshot));

	const FMatchPlayState BBeforeDeployment =
		HostB->GetMatchSnapshot().Snapshot;
	FMatchPlayAuthoritativeDeployOrdinaryRequest DeploymentRequest;
	TestTrue(TEXT("Host A has a legal deployment"),
		FindLegalDeployment(
			HostA->GetMatchSnapshot().Snapshot,
			EMatchPlayRelativeDeploymentZone::Forward,
			DeploymentRequest));
	TestTrue(TEXT("Host A deployment advances independently"),
		HostA->DeployOrdinary(DeploymentRequest).bSuccess);
	TestTrue(TEXT("Host A deployment cannot mutate Host B"),
		AreStatesEqual(
			HostB->GetMatchSnapshot().Snapshot,
			BBeforeDeployment));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexLocalMatchHostInteractionEquivalenceTest,
	"FMCodex.LocalPlay.LocalMatchHost.04.InteractionEquivalence",
	EAutomationTestFlags::EditorContext
		| EAutomationTestFlags::EngineFilter)

bool FFMCodexLocalMatchHostInteractionEquivalenceTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexLocalMatchHostTests;
	FScopedLocalMatchTestWorld TestWorld;
	AFMCodexLocalMatchHostGameMode* Host = TestWorld.GetHost();
	TestNotNull(TEXT("Interaction host exists"), Host);
	if (Host == nullptr)
	{
		return false;
	}
	{
		const FMatchPlayOpeningInitializeInput GoalkeeperInput =
			MakeInteractionInput(TEXT("LocalHostGoalkeeper"));
		FMatchPlayAuthoritativeSession GoalkeeperDirect;
		TestTrue(TEXT("Goalkeeper direct initialize succeeds"),
			GoalkeeperDirect.InitializeMatch(GoalkeeperInput)
				.OpeningResult.bSuccess);
		TestTrue(TEXT("Goalkeeper Host initialize succeeds"),
			Host->StartNewLocalMatch(GoalkeeperInput).bSuccess);
		TestTrue(TEXT("Goalkeeper direct attack begins"),
			GoalkeeperDirect.BeginOrdinaryAttack(6).BeginResult.bSuccess);
		TestTrue(TEXT("Goalkeeper Host attack begins"),
			Host->BeginOrdinaryAttack(6).bSuccess);
		FMatchPlayAuthoritativeDeployOrdinaryRequest AttackDeploy;
		TestTrue(TEXT("Goalkeeper fixture attacker deployment exists"),
			FindLegalDeployment(
				Host->GetMatchSnapshot().Snapshot,
				EMatchPlayRelativeDeploymentZone::Forward,
				AttackDeploy));
		TestTrue(TEXT("Goalkeeper fixture direct attacker deploys"),
			GoalkeeperDirect.DeployOrdinary(AttackDeploy)
				.DeploymentResult.bSuccess);
		TestTrue(TEXT("Goalkeeper fixture Host attacker deploys"),
			Host->DeployOrdinary(AttackDeploy).bSuccess);
		const FMatchPlayState GoalkeeperState =
			Host->GetMatchSnapshot().Snapshot;
		const EInitialTurnOrderPlayer Defender = OtherPlayer(
			GoalkeeperState.RuntimeState.CurrentAttackingPlayer);
		const FName GoalkeeperCardId = FName(*(
			Defender == EInitialTurnOrderPlayer::PlayerA
				? GoalkeeperState.RuntimeState.PlayerAState.GoalkeeperCardId
				: GoalkeeperState.RuntimeState.PlayerBState.GoalkeeperCardId));
		const auto Availability =
			FMatchPlayGoalkeeperDeploymentAvailability::Query(
				GoalkeeperState,
				GoalkeeperState.CurrentAttack.AttackSequence,
				Defender,
				GoalkeeperCardId);
		TestTrue(TEXT("A legal derived goalkeeper slot exists"),
			Availability.bCanDeployToAnySlot
				&& !Availability.LegalSlotIds.IsEmpty());
		if (Availability.LegalSlotIds.IsEmpty())
		{
			return false;
		}
		FMatchPlayAuthoritativeDeployGoalkeeperRequest Request;
		Request.ExpectedAttackSequence =
			GoalkeeperState.CurrentAttack.AttackSequence;
		Request.RequestingSide = Defender;
		Request.SlotId = Availability.LegalSlotIds[0];
		TestTrue(TEXT("Direct goalkeeper deployment succeeds"),
			GoalkeeperDirect.DeployGoalkeeper(Request)
				.DeploymentResult.bSucceeded);
		const auto HostGoalkeeper = Host->DeployGoalkeeper(Request);
		TestTrue(TEXT("Host goalkeeper deployment succeeds"),
			HostGoalkeeper.bSuccess);
		TestEqual(TEXT("Goalkeeper request preserves genuine SlotId"),
			HostGoalkeeper.AuthoritativeResult.DeploymentResult.Request.SlotId,
			Request.SlotId);
		TestTrue(TEXT("Goalkeeper Host State equals direct Session"),
			AreStatesEqual(
				Host->GetMatchSnapshot().Snapshot,
				GoalkeeperDirect.GetStateSnapshot()));
	}

	const FName SkillId(TEXT("Skill.LocalHost.LongShot"));
	const FSkillRuleSnapshotSet Rules =
		MakeSkillRuleSet(SkillId, ESkillRuleType::LongShot);
	const FMatchPlayOpeningInitializeInput Input =
		MakeInteractionInput(TEXT("LocalHostInteraction"), SkillId);
	FMatchPlayAuthoritativeSession DirectSession;
	TestTrue(TEXT("Direct initialize succeeds"),
		DirectSession.InitializeMatch(Input).OpeningResult.bSuccess);
	TestTrue(TEXT("Host initialize succeeds"),
		Host->StartNewLocalMatch(Input, Rules).bSuccess);
	TestTrue(TEXT("Direct begin succeeds"),
		DirectSession.BeginOrdinaryAttack(6).BeginResult.bSuccess);
	TestTrue(TEXT("Host begin succeeds"),
		Host->BeginOrdinaryAttack(6).bSuccess);

	auto TestEquivalent = [this, Host, &DirectSession](const TCHAR* Label)
	{
		const auto Snapshot = Host->GetMatchSnapshot();
		TestTrue(*FString::Printf(TEXT("%s snapshot succeeds"), Label),
			Snapshot.bSuccess);
		TestTrue(*FString::Printf(TEXT("%s State equals direct Session"), Label),
			AreStatesEqual(Snapshot.Snapshot, DirectSession.GetStateSnapshot()));
	};
	TestEquivalent(TEXT("Begin"));

	FMatchPlayAuthoritativeDeployOrdinaryRequest AttackerDeploy;
	TestTrue(TEXT("A legal attacker Forward deployment exists"),
		FindLegalDeployment(
			Host->GetMatchSnapshot().Snapshot,
			EMatchPlayRelativeDeploymentZone::Forward,
			AttackerDeploy));
	const FMatchPlayState BeforeWrongTurn = Host->GetMatchSnapshot().Snapshot;
	auto WrongTurn = AttackerDeploy;
	WrongTurn.RequestingSide = OtherPlayer(WrongTurn.RequestingSide);
	const auto WrongTurnResult = Host->DeployOrdinary(WrongTurn);
	TestFalse(TEXT("Wrong deployment side remains an authoritative failure"),
		WrongTurnResult.bSuccess);
	TestEqual(TEXT("Wrong deployment side maps to Host command failure"),
		WrongTurnResult.ErrorCode,
		EFMCodexLocalMatchHostErrorCode::AuthoritativeCommandFailed);
	TestTrue(TEXT("Wrong deployment side preserves typed diagnostics"),
		WrongTurnResult.AuthoritativeResult.RuntimeEnvelope.bAccepted
			&& !WrongTurnResult.AuthoritativeResult.DeploymentResult.bSuccess
			&& !WrongTurnResult.AuthoritativeResult.DeploymentResult
				.ErrorMessage.IsEmpty());
	TestTrue(TEXT("Wrong deployment side preserves hosted State"),
		AreStatesEqual(
			Host->GetMatchSnapshot().Snapshot,
			BeforeWrongTurn));

	TestTrue(TEXT("Direct attacker deployment succeeds"),
		DirectSession.DeployOrdinary(AttackerDeploy)
			.DeploymentResult.bSuccess);
	const auto HostAttackerDeploy = Host->DeployOrdinary(AttackerDeploy);
	TestTrue(TEXT("Host attacker deployment succeeds"),
		HostAttackerDeploy.bSuccess);
	TestEqual(TEXT("DeployOrdinary preserves command kind"),
		HostAttackerDeploy.AuthoritativeResult.RuntimeEnvelope.CommandKind,
		EMatchPlayAuthoritativeCommandKind::DeployOrdinary);
	TestEquivalent(TEXT("Attacker deployment"));

	FMatchPlayState State = Host->GetMatchSnapshot().Snapshot;
	const EInitialTurnOrderPlayer AttackingSide =
		State.RuntimeState.CurrentAttackingPlayer;
	const EInitialTurnOrderPlayer DefendingSide = OtherPlayer(AttackingSide);

	FMatchPlayAuthoritativeDeployOrdinaryRequest DefenderDeploy;
	TestTrue(TEXT("A legal defender Backfield deployment exists"),
		FindLegalDeployment(
			Host->GetMatchSnapshot().Snapshot,
			EMatchPlayRelativeDeploymentZone::Backfield,
			DefenderDeploy));
	TestTrue(TEXT("Direct defender deployment succeeds"),
		DirectSession.DeployOrdinary(DefenderDeploy)
			.DeploymentResult.bSuccess);
	TestTrue(TEXT("Host defender deployment succeeds"),
		Host->DeployOrdinary(DefenderDeploy).bSuccess);
	TestEquivalent(TEXT("Defender deployment"));
	for (int32 Index = 0; Index < 1; ++Index)
	{
		State = Host->GetMatchSnapshot().Snapshot;
		const bool bAttackerDeploying =
			State.CurrentAttack.CurrentLegalDeploymentSide == AttackingSide;
		FMatchPlayAuthoritativeDeployOrdinaryRequest AdditionalDeploy;
		TestTrue(TEXT("An additional participant deployment exists"),
			FindLegalDeployment(
				State,
				bAttackerDeploying
					? EMatchPlayRelativeDeploymentZone::Forward
					: EMatchPlayRelativeDeploymentZone::Backfield,
				AdditionalDeploy));
		TestTrue(TEXT("Direct additional participant deploys"),
			DirectSession.DeployOrdinary(AdditionalDeploy)
				.DeploymentResult.bSuccess);
		TestTrue(TEXT("Host additional participant deploys"),
			Host->DeployOrdinary(AdditionalDeploy).bSuccess);
		TestEquivalent(TEXT("Additional participant deployment"));
	}

	for (int32 FinishIndex = 0; FinishIndex < 2; ++FinishIndex)
	{
		State = Host->GetMatchSnapshot().Snapshot;
		const int64 AttackSequence = State.CurrentAttack.AttackSequence;
		const EInitialTurnOrderPlayer Side =
			State.CurrentAttack.CurrentLegalDeploymentSide;
		TestTrue(TEXT("Direct FinishDeployment succeeds"),
			DirectSession.FinishDeployment(AttackSequence, Side)
				.FinishResult.bSuccess);
		TestTrue(TEXT("Host FinishDeployment succeeds"),
			Host->FinishDeployment(AttackSequence, Side).bSuccess);
		TestEquivalent(TEXT("FinishDeployment"));
	}
	State = Host->GetMatchSnapshot().Snapshot;
	TestEqual(TEXT("Both finishes canonically reach AwaitingCarrier"),
		State.CurrentAttack.SelectionStage,
		EMatchPlayCurrentAttackSelectionStage::AwaitingCarrier);

	FName CarrierCardId;
	TestTrue(TEXT("A legal Carrier exists"),
		FindLegalCarrier(State, AttackingSide, CarrierCardId));
	FMatchPlayAuthoritativeSubmitCarrierRequest CarrierRequest;
	CarrierRequest.ExpectedAttackSequence = State.CurrentAttack.AttackSequence;
	CarrierRequest.RequestingSide = AttackingSide;
	CarrierRequest.CarrierCardId = CarrierCardId;
	TestTrue(TEXT("Direct Carrier succeeds"),
		DirectSession.SubmitCarrier(CarrierRequest).CarrierResult.bSuccess);
	TestTrue(TEXT("Host Carrier succeeds"),
		Host->SubmitCarrier(CarrierRequest).bSuccess);
	TestEquivalent(TEXT("Carrier"));

	State = Host->GetMatchSnapshot().Snapshot;
	FName MarkerCardId;
	TestTrue(TEXT("A legal Marker exists"),
		FindLegalMarker(State, DefendingSide, MarkerCardId));
	FMatchPlayAuthoritativeSubmitMarkerRequest MarkerRequest;
	MarkerRequest.ExpectedAttackSequence = State.CurrentAttack.AttackSequence;
	MarkerRequest.RequestingSide = DefendingSide;
	MarkerRequest.MarkerCardId = MarkerCardId;
	TestTrue(TEXT("Direct Marker succeeds"),
		DirectSession.SubmitMarker(MarkerRequest).MarkerResult.bSuccess);
	TestTrue(TEXT("Host Marker succeeds"),
		Host->SubmitMarker(MarkerRequest).bSuccess);
	TestEquivalent(TEXT("Marker"));

	State = Host->GetMatchSnapshot().Snapshot;
	const auto RunnerAvailability =
		FMatchPlayCurrentAttackRunnerSelectionAvailability::Query(
			State, State.CurrentAttack.AttackSequence, AttackingSide);
	const auto* Runner = RunnerAvailability.Candidates.FindByPredicate(
		[](const FMatchPlayCurrentAttackRunnerSelectionCandidateAvailability& Candidate)
		{
			return Candidate.LegalityResult.bIsLegal;
		});
	TestNotNull(TEXT("A legal participant-first Runner exists"), Runner);
	if (Runner == nullptr)
	{
		return false;
	}
	FMatchPlayAuthoritativeSubmitRunnerRequest RunnerRequest;
	RunnerRequest.ExpectedAttackSequence = State.CurrentAttack.AttackSequence;
	RunnerRequest.RequestingSide = AttackingSide;
	RunnerRequest.RunnerCardId = Runner->RunnerCardId;
	TestTrue(TEXT("Direct Runner succeeds"),
		DirectSession.SubmitRunner(RunnerRequest).RunnerResult.bSuccess);
	TestTrue(TEXT("Host Runner succeeds"),
		Host->SubmitRunner(RunnerRequest).bSuccess);
	TestEquivalent(TEXT("Runner"));

	State = Host->GetMatchSnapshot().Snapshot;
	const auto HelperAvailability =
		FMatchPlayCurrentAttackHelperSelectionAvailability::Query(
			State, State.CurrentAttack.AttackSequence, DefendingSide);
	const auto* Helper = HelperAvailability.Candidates.FindByPredicate(
		[](const FMatchPlayCurrentAttackHelperSelectionCandidateAvailability& Candidate)
		{
			return Candidate.LegalityResult.bSuccess;
		});
	if (Helper != nullptr)
	{
		FMatchPlayAuthoritativeSubmitHelperRequest HelperRequest;
		HelperRequest.ExpectedAttackSequence =
			State.CurrentAttack.AttackSequence;
		HelperRequest.RequestingSide = DefendingSide;
		HelperRequest.HelperCardId = Helper->HelperCardId;
		TestTrue(TEXT("Direct Helper succeeds"),
			DirectSession.SubmitHelper(HelperRequest).HelperResult.bSuccess);
		TestTrue(TEXT("Host Helper succeeds"),
			Host->SubmitHelper(HelperRequest).bSuccess);
	}
	else
	{
		TestTrue(TEXT("Direct explicit no-legal Helper succeeds"),
			DirectSession.ResolveNoLegalHelper().ResolutionResult.bSuccess);
		TestTrue(TEXT("Host explicit no-legal Helper succeeds"),
			Host->ResolveNoLegalHelper().bSuccess);
	}
	TestEquivalent(TEXT("Helper"));

	FMatchPlayAuthoritativeSubmitSkillRequest SkillRequest;
	SkillRequest.ExpectedAttackSequence =
		Host->GetMatchSnapshot().Snapshot.CurrentAttack.AttackSequence;
	SkillRequest.RequestingSide = AttackingSide;
	SkillRequest.SkillId = SkillId;
	TestTrue(TEXT("Direct Skill succeeds"),
		DirectSession.SubmitSkill(Rules, SkillRequest).SkillResult.bSuccess);
	TestTrue(TEXT("Host Skill succeeds with current external rule set"),
		Host->SubmitSkill(Rules, SkillRequest).bSuccess);
	TestEquivalent(TEXT("Skill"));
	State = Host->GetMatchSnapshot().Snapshot;
	TestEqual(TEXT("LongShot reaches AwaitingBranchIntent"),
		State.CurrentAttack.SelectionStage,
		EMatchPlayCurrentAttackSelectionStage::AwaitingBranchIntent);

	FMatchPlayAuthoritativeSubmitBranchIntentRequest IntentRequest;
	IntentRequest.AttackSequence = State.CurrentAttack.AttackSequence;
	IntentRequest.RequestingSide = AttackingSide;
	IntentRequest.Intent = EMatchPlayElectiveBranchIntent::DirectShot;
	TestTrue(TEXT("Direct BranchIntent succeeds"),
		DirectSession.SubmitBranchIntent(IntentRequest)
			.IntentResult.bSuccess);
	const auto HostIntent = Host->SubmitBranchIntent(IntentRequest);
	TestTrue(TEXT("Host BranchIntent succeeds"), HostIntent.bSuccess);
	TestEquivalent(TEXT("BranchIntent"));
	State = Host->GetMatchSnapshot().Snapshot;
	TestTrue(TEXT("Branch choice reaches canonical State"),
		State.CurrentAttack.bHasSelectedAction
			&& State.CurrentAttack.SelectionStage
				== EMatchPlayCurrentAttackSelectionStage::ReadyForResolution
			&& !State.CurrentAttack.bHasResolutionSession);

	const FMatchPlayState BeforeRepeatedIntent = State;
	const auto RepeatedIntent = Host->SubmitBranchIntent(IntentRequest);
	TestFalse(TEXT("Repeated BranchIntent is authoritatively rejected"),
		RepeatedIntent.bSuccess);
	TestTrue(TEXT("Repeated BranchIntent preserves exact State"),
		AreStatesEqual(
			Host->GetMatchSnapshot().Snapshot,
			BeforeRepeatedIntent));
	TestFalse(TEXT("Focused flow never starts resolution or consumes route RNG"),
		State.CurrentAttack.bHasResolutionSession);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexLocalMatchHostExceptionalRoutingTest,
	"FMCodex.LocalPlay.LocalMatchHost.05.ExceptionalRouting",
	EAutomationTestFlags::EditorContext
		| EAutomationTestFlags::EngineFilter)

bool FFMCodexLocalMatchHostExceptionalRoutingTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexLocalMatchHostTests;

	FScopedLocalMatchTestWorld NoCarrierWorld;
	auto* NoCarrierHost = NoCarrierWorld.GetHost();
	TestNotNull(TEXT("No-Carrier host exists"), NoCarrierHost);
	if (NoCarrierHost == nullptr)
	{
		return false;
	}
	TestTrue(TEXT("No-Carrier match starts"), NoCarrierHost
		->StartNewLocalMatch(MakeInteractionInput(TEXT("NoCarrier")))
		.bSuccess);
	TestTrue(TEXT("No-Carrier attack begins"),
		NoCarrierHost->BeginOrdinaryAttack(6).bSuccess);
	for (int32 Index = 0; Index < 2; ++Index)
	{
		const FMatchPlayState State =
			NoCarrierHost->GetMatchSnapshot().Snapshot;
		TestTrue(TEXT("Empty deployment side can finish"),
			NoCarrierHost->FinishDeployment(
				State.CurrentAttack.AttackSequence,
				State.CurrentAttack.CurrentLegalDeploymentSide).bSuccess);
	}
	TestEqual(TEXT("Empty deployment reaches AwaitingCarrier"),
		NoCarrierHost->GetMatchSnapshot().Snapshot
			.CurrentAttack.SelectionStage,
		EMatchPlayCurrentAttackSelectionStage::AwaitingCarrier);
	const auto NoCarrier = NoCarrierHost->ResolveNoLegalCarrier();
	TestTrue(TEXT("ResolveNoLegalCarrier is Host-reachable"),
		NoCarrier.bSuccess);
	TestEqual(TEXT("No-Carrier preserves authoritative command kind"),
		NoCarrier.AuthoritativeResult.RuntimeEnvelope.CommandKind,
		EMatchPlayAuthoritativeCommandKind::ResolveNoLegalCarrier);
	TestFalse(TEXT("No legal Carrier canonically completes attack"),
		NoCarrierHost->GetMatchSnapshot().Snapshot.bHasCurrentAttack);

	auto ReachAwaitingMarker = [this](
		AFMCodexLocalMatchHostGameMode& Host,
		const FString& Prefix,
		const EMatchPlayRelativeDeploymentZone DefenderZone,
		EInitialTurnOrderPlayer& OutDefendingSide)
	{
		if (!Host.StartNewLocalMatch(MakeInteractionInput(Prefix)).bSuccess
			|| !Host.BeginOrdinaryAttack(6).bSuccess)
		{
			return false;
		}
		FMatchPlayAuthoritativeDeployOrdinaryRequest Request;
		if (!FindLegalDeployment(
			Host.GetMatchSnapshot().Snapshot,
			EMatchPlayRelativeDeploymentZone::Forward,
			Request)
			|| !Host.DeployOrdinary(Request).bSuccess)
		{
			return false;
		}
		if (!FindLegalDeployment(
			Host.GetMatchSnapshot().Snapshot,
			DefenderZone,
			Request)
			|| !Host.DeployOrdinary(Request).bSuccess)
		{
			return false;
		}
		for (int32 Index = 0; Index < 2; ++Index)
		{
			const FMatchPlayState State = Host.GetMatchSnapshot().Snapshot;
			if (!Host.FinishDeployment(
				State.CurrentAttack.AttackSequence,
				State.CurrentAttack.CurrentLegalDeploymentSide).bSuccess)
			{
				return false;
			}
		}
		const FMatchPlayState State = Host.GetMatchSnapshot().Snapshot;
		const EInitialTurnOrderPlayer AttackingSide =
			State.RuntimeState.CurrentAttackingPlayer;
		OutDefendingSide = OtherPlayer(AttackingSide);
		FName CarrierCardId;
		if (!FindLegalCarrier(State, AttackingSide, CarrierCardId))
		{
			return false;
		}
		FMatchPlayAuthoritativeSubmitCarrierRequest CarrierRequest;
		CarrierRequest.ExpectedAttackSequence =
			State.CurrentAttack.AttackSequence;
		CarrierRequest.RequestingSide = AttackingSide;
		CarrierRequest.CarrierCardId = CarrierCardId;
		return Host.SubmitCarrier(CarrierRequest).bSuccess
			&& Host.GetMatchSnapshot().Snapshot.CurrentAttack.SelectionStage
				== EMatchPlayCurrentAttackSelectionStage::AwaitingMarker;
	};

	FScopedLocalMatchTestWorld NoMarkerWorld;
	auto* NoMarkerHost = NoMarkerWorld.GetHost();
	EInitialTurnOrderPlayer NoMarkerDefender =
		EInitialTurnOrderPlayer::None;
	TestNotNull(TEXT("No-Marker host exists"), NoMarkerHost);
	if (NoMarkerHost == nullptr)
	{
		return false;
	}
	TestTrue(TEXT("Midfield defender flow reaches AwaitingMarker"),
		ReachAwaitingMarker(
			*NoMarkerHost,
			TEXT("NoMarker"),
			EMatchPlayRelativeDeploymentZone::Midfield,
			NoMarkerDefender));
	const auto NoMarker = NoMarkerHost->ResolveNoLegalMarker();
	TestTrue(TEXT("ResolveNoLegalMarker is Host-reachable"),
		NoMarker.bSuccess);
	TestFalse(TEXT("No legal Marker canonically completes attack"),
		NoMarkerHost->GetMatchSnapshot().Snapshot.bHasCurrentAttack);

	FScopedLocalMatchTestWorld DeclineWorld;
	auto* DeclineHost = DeclineWorld.GetHost();
	EInitialTurnOrderPlayer DeclineDefender =
		EInitialTurnOrderPlayer::None;
	TestNotNull(TEXT("Decline host exists"), DeclineHost);
	if (DeclineHost == nullptr)
	{
		return false;
	}
	TestTrue(TEXT("Backfield defender flow reaches AwaitingMarker"),
		ReachAwaitingMarker(
			*DeclineHost,
			TEXT("DeclineMarker"),
			EMatchPlayRelativeDeploymentZone::Backfield,
			DeclineDefender));
	FMatchPlayAuthoritativeDeclineMarkerRequest DeclineRequest;
	DeclineRequest.ExpectedAttackSequence = DeclineHost->GetMatchSnapshot()
		.Snapshot.CurrentAttack.AttackSequence;
	DeclineRequest.RequestingSide = DeclineDefender;
	const auto Decline = DeclineHost->DeclineMarker(DeclineRequest);
	TestTrue(TEXT("Player DeclineMarker is Host-reachable"),
		Decline.bSuccess);
	TestFalse(TEXT("Marker decline canonically completes attack"),
		DeclineHost->GetMatchSnapshot().Snapshot.bHasCurrentAttack);
	return true;
}

#endif
