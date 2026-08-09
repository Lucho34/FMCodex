#include "FMCodexLocalMatchHostGameMode.h"

#if WITH_DEV_AUTOMATION_TESTS

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
	TestTrue(TEXT("StartNewLocalMatch is a typed canonical-input method"),
		(std::is_same_v<
			decltype(&AFMCodexLocalMatchHostGameMode::StartNewLocalMatch),
			FStartMethod>));
	TestTrue(TEXT("Snapshot is returned through one const by-value method"),
		(std::is_same_v<
			decltype(&AFMCodexLocalMatchHostGameMode::GetMatchSnapshot),
			FSnapshotMethod>));
	TestTrue(TEXT("BeginOrdinaryAttack forwards only ActionPoint"),
		(std::is_same_v<
			decltype(&AFMCodexLocalMatchHostGameMode::BeginOrdinaryAttack),
			FBeginMethod>));

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
			TEXT("TUniquePtr<FMatchPlayAuthoritativeSession> AuthoritativeSession;")),
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
		CountOccurrences(Source, TEXT("CandidateSession->InitializeMatch(Input)")),
		1);
	TestEqual(TEXT("BeginOrdinaryAttack has one host delegation"),
		CountOccurrences(Source,
			TEXT("AuthoritativeSession->BeginOrdinaryAttack(ActionPoint)")),
		1);
	TestEqual(TEXT("Snapshot has one host delegation"),
		CountOccurrences(Source,
			TEXT("AuthoritativeSession->GetStateSnapshot()")),
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
	return true;
}

#endif
