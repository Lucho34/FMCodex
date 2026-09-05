#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

#include "FMCodexNetworkMatchGameMode.h"
#include "FMCodexNetworkMatchGameState.h"
#include "FMCodexNetworkMatchPlayerController.h"
#include "FMCodexNetworkMatchPlayerState.h"
#include "FMCodexNetworkMatchRuntime.h"
#include "FMCodexNetworkMatchTypes.h"
#include "FMCodexNetworkParticipantRegistry.h"
#include "../LocalPlay/FMCodexLocalMatchInteractionView.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "UObject/UnrealType.h"

namespace FMCodexNetworkBootstrapTests
{
	bool LoadProductionSource(const TCHAR* RelativePath, FString& OutSource)
	{
		return FFileHelper::LoadFileToString(
			OutSource,
			*FPaths::ConvertRelativePathToFull(
				FPaths::ProjectDir() / RelativePath));
	}

	class FScopedNetworkTestWorld final
	{
	public:
		FScopedNetworkTestWorld()
		{
			World = UWorld::CreateWorld(EWorldType::Game, false);
			if (World != nullptr && GEngine != nullptr)
			{
				FWorldContext& Context =
					GEngine->CreateNewWorldContext(EWorldType::Game);
				Context.SetCurrentWorld(World);
			}
		}

		~FScopedNetworkTestWorld()
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

		template <typename TActor>
		TActor* Spawn() const
		{
			return World != nullptr ? World->SpawnActor<TActor>() : nullptr;
		}

	private:
		UWorld* World = nullptr;
	};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexNetworkParticipantAdmissionTest,
	"FMCodex.NetworkPlay.NetworkBootstrap.01.ParticipantAdmission",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexNetworkParticipantAdmissionTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexNetworkBootstrapTests;
	FScopedNetworkTestWorld TestWorld;
	AFMCodexNetworkMatchPlayerController* First =
		TestWorld.Spawn<AFMCodexNetworkMatchPlayerController>();
	AFMCodexNetworkMatchPlayerController* Second =
		TestWorld.Spawn<AFMCodexNetworkMatchPlayerController>();
	AFMCodexNetworkMatchPlayerController* Third =
		TestWorld.Spawn<AFMCodexNetworkMatchPlayerController>();
	AFMCodexNetworkMatchPlayerController* Replacement =
		TestWorld.Spawn<AFMCodexNetworkMatchPlayerController>();
	AFMCodexNetworkMatchPlayerState* FirstState =
		TestWorld.Spawn<AFMCodexNetworkMatchPlayerState>();
	AFMCodexNetworkMatchPlayerState* SecondState =
		TestWorld.Spawn<AFMCodexNetworkMatchPlayerState>();
	AFMCodexNetworkMatchPlayerState* ThirdState =
		TestWorld.Spawn<AFMCodexNetworkMatchPlayerState>();
	AFMCodexNetworkMatchPlayerState* ReplacementState =
		TestWorld.Spawn<AFMCodexNetworkMatchPlayerState>();
	if (!TestNotNull(TEXT("First controller exists"), First)
		|| !TestNotNull(TEXT("Second controller exists"), Second)
		|| !TestNotNull(TEXT("Third controller exists"), Third)
		|| !TestNotNull(TEXT("Replacement controller exists"), Replacement)
		|| !TestNotNull(TEXT("First state exists"), FirstState)
		|| !TestNotNull(TEXT("Second state exists"), SecondState)
		|| !TestNotNull(TEXT("Third state exists"), ThirdState)
		|| !TestNotNull(TEXT("Replacement state exists"), ReplacementState))
	{
		return false;
	}

	FFMCodexNetworkParticipantRegistry Registry;
	const FFMCodexNetworkAdmissionResult FirstAdmission =
		Registry.Admit(First, FirstState);
	const FFMCodexNetworkAdmissionResult DuplicateAdmission =
		Registry.Admit(First, FirstState);
	const FFMCodexNetworkAdmissionResult SecondAdmission =
		Registry.Admit(Second, SecondState);
	TestTrue(TEXT("First participant accepted"), FirstAdmission.bAccepted);
	TestEqual(TEXT("First accepted participant owns Side A"),
		FirstAdmission.AssignedSide, EInitialTurnOrderPlayer::PlayerA);
	TestTrue(TEXT("Duplicate admission is idempotently accepted"),
		DuplicateAdmission.bAccepted && DuplicateAdmission.bAlreadyAdmitted);
	TestEqual(TEXT("Duplicate retains Side A"),
		DuplicateAdmission.AssignedSide, EInitialTurnOrderPlayer::PlayerA);
	TestTrue(TEXT("Second participant accepted"), SecondAdmission.bAccepted);
	TestEqual(TEXT("Second accepted participant owns Side B"),
		SecondAdmission.AssignedSide, EInitialTurnOrderPlayer::PlayerB);
	TestTrue(TEXT("Both participant slots are connected"),
		Registry.HasBothParticipants());
	TestEqual(TEXT("Exactly two gameplay participants connected"),
		Registry.GetConnectedParticipantCount(), 2);

	const FFMCodexNetworkAdmissionResult ThirdAdmission =
		Registry.Admit(Third, ThirdState);
	TestFalse(TEXT("Third connection is rejected"),
		ThirdAdmission.bAccepted);
	TestEqual(TEXT("Third connection reports MatchFull"),
		ThirdAdmission.Error, EFMCodexNetworkAdmissionError::MatchFull);

	Registry.MarkDisconnected(First);
	TestFalse(TEXT("Disconnect removes active controller mapping"),
		Registry.ResolveSide(First) != EInitialTurnOrderPlayer::None);
	TestEqual(TEXT("Only one participant remains connected"),
		Registry.GetConnectedParticipantCount(), 1);
	TestTrue(TEXT("Both gameplay sides remain permanently reserved"),
		Registry.HasReservedBothSides());
	const FFMCodexNetworkAdmissionResult ReplacementAdmission =
		Registry.Admit(Replacement, ReplacementState);
	TestFalse(TEXT("Disconnected side is not reassigned in Stage 7.2"),
		ReplacementAdmission.bAccepted);
	TestEqual(TEXT("Replacement is rejected as MatchFull"),
		ReplacementAdmission.Error,
		EFMCodexNetworkAdmissionError::MatchFull);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexNetworkPrototypeBootstrapTest,
	"FMCodex.NetworkPlay.NetworkBootstrap.02.PrototypeMatchAndIdentity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexNetworkPrototypeBootstrapTest::RunTest(
	const FString& Parameters)
{
	const FFMCodexNetworkBootstrapConfiguration Configuration =
		FFMCodexNetworkBootstrapConfigurationFactory::CreatePrototypeMatch();
	TestEqual(TEXT("Prototype contract declares three attacks per side"),
		Configuration.AttackOpportunitiesPerSide, 3);
	TestTrue(TEXT("Side A team identity is data driven"),
		!Configuration.PlayerATeam.TeamId.IsNone()
			&& !Configuration.PlayerATeam.TeamDisplayName.IsEmpty());
	TestTrue(TEXT("Side B team identity is data driven"),
		!Configuration.PlayerBTeam.TeamId.IsNone()
			&& !Configuration.PlayerBTeam.TeamDisplayName.IsEmpty());
	TestNotEqual(TEXT("Team identity is not player identity"),
		Configuration.PlayerATeam.TeamId,
		Configuration.PlayerBTeam.TeamId);

	const FGuid MatchId(0x10203040, 0x50607080, 0x90A0B0C0, 0xD0E0F001);
	FFMCodexNetworkMatchRuntime Runtime(MatchId, 7161202);
	const FFMCodexNetworkRuntimeInitializeResult First =
		Runtime.InitializeOnce(Configuration);
	const FFMCodexNetworkRuntimeInitializeResult Duplicate =
		Runtime.InitializeOnce(Configuration);
	TestTrue(TEXT("Server prototype match initializes"), First.bSuccess);
	TestTrue(TEXT("Duplicate bootstrap is an idempotent no-op"),
		Duplicate.bSuccess && Duplicate.bAlreadyInitialized);
	TestEqual(TEXT("Authoritative match initialization executes once"),
		Runtime.GetInitializationCount(), 1);
	TestEqual(TEXT("Two initialization requests are observable"),
		Runtime.GetInitializationAttemptCount(), 2);
	TestEqual(TEXT("Runtime retains the immutable match identity"),
		Runtime.GetMatchInstanceId(), MatchId);

	const FFMCodexNetworkClientViewSnapshot PlayerA =
		Runtime.BuildClientView(
			EInitialTurnOrderPlayer::PlayerA,
			7,
			EFMCodexNetworkBootstrapState::MatchReady);
	const FFMCodexNetworkClientViewSnapshot PlayerB =
		Runtime.BuildClientView(
			EInitialTurnOrderPlayer::PlayerB,
			7,
			EFMCodexNetworkBootstrapState::MatchReady);
	TestTrue(TEXT("Side A receives an initialized owner view"),
		PlayerA.bMatchInitialized);
	TestTrue(TEXT("Side B receives an initialized owner view"),
		PlayerB.bMatchInitialized);
	TestEqual(TEXT("Side A view carries only A viewer identity"),
		PlayerA.ViewerSide, EInitialTurnOrderPlayer::PlayerA);
	TestEqual(TEXT("Side B view carries only B viewer identity"),
		PlayerB.ViewerSide, EInitialTurnOrderPlayer::PlayerB);
	TestEqual(TEXT("Both views share one immutable match id"),
		PlayerA.MatchInstanceId, PlayerB.MatchInstanceId);
	TestEqual(TEXT("Side A sees the 3+3 canonical contract"),
		PlayerA.PlayerAMaxAttackOpportunities, 3);
	TestEqual(TEXT("Side B sees the 3+3 canonical contract"),
		PlayerB.PlayerBMaxAttackOpportunities, 3);
	TestEqual(TEXT("Both safe projections agree on expected actor"),
		PlayerA.ExpectedActingSide, PlayerB.ExpectedActingSide);
	TestTrue(TEXT("Expected actor gets own Full D12 status"),
		(PlayerA.ExpectedActingSide == EInitialTurnOrderPlayer::PlayerA
			&& PlayerA.InteractionState ==
				EFMCodexNetworkClientInteractionState
					::WaitingForOwnInitialActionPoint
			&& PlayerB.InteractionState ==
				EFMCodexNetworkClientInteractionState
					::WaitingForOpponentInitialActionPoint)
		|| (PlayerA.ExpectedActingSide == EInitialTurnOrderPlayer::PlayerB
			&& PlayerB.InteractionState ==
				EFMCodexNetworkClientInteractionState
					::WaitingForOwnInitialActionPoint
			&& PlayerA.InteractionState ==
				EFMCodexNetworkClientInteractionState
					::WaitingForOpponentInitialActionPoint));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexNetworkOwnerViewSecrecyTest,
	"FMCodex.NetworkPlay.NetworkBootstrap.03.OwnerViewSecrecy",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexNetworkOwnerViewSecrecyTest::RunTest(
	const FString& Parameters)
{
	FFMCodexLocalMatchInteractionView SecretBearingCornerView;
	SecretBearingCornerView.bMatchActive = true;
	SecretBearingCornerView.CornerStage =
		EMatchPlaySetPieceCornerRouteStage::AwaitingDefenderNominations;
	SecretBearingCornerView.DraftCornerNomineeCardIds = {
		FName(TEXT("SECRET.Corner.Nominee.01")),
		FName(TEXT("SECRET.Corner.Nominee.02")) };
	SecretBearingCornerView.ExpectedActingPlayer =
		EInitialTurnOrderPlayer::PlayerB;
	const FFMCodexNetworkClientViewSnapshot Snapshot =
		FFMCodexNetworkClientViewSnapshotFactory::Build(
			SecretBearingCornerView,
			FGuid(1, 2, 3, 4),
			11,
			EInitialTurnOrderPlayer::PlayerB,
			EFMCodexNetworkBootstrapState::MatchReady);
	TestEqual(TEXT("Safe DTO retains only viewer identity"),
		Snapshot.ViewerSide, EInitialTurnOrderPlayer::PlayerB);
	const UScriptStruct* SnapshotStruct =
		FFMCodexNetworkClientViewSnapshot::StaticStruct();
	TestNull(TEXT("Owner DTO has no draft Corner card ids"),
		SnapshotStruct->FindPropertyByName(
			TEXT("DraftCornerNomineeCardIds")));
	TestNull(TEXT("Owner DTO has no legal card ids"),
		SnapshotStruct->FindPropertyByName(TEXT("LegalSetPieceCardIds")));
	TestNull(TEXT("Owner DTO has no attacker nominee binding"),
		SnapshotStruct->FindPropertyByName(TEXT("CornerAttackerNominees")));
	TestNull(TEXT("Owner DTO has no defender nominee binding"),
		SnapshotStruct->FindPropertyByName(TEXT("CornerDefenderNominees")));
	TestNull(TEXT("Owner DTO has no automatic scorer RNG fact"),
		SnapshotStruct->FindPropertyByName(TEXT("AutomaticScorerD6")));
	TestNull(TEXT("Owner DTO has no raw authoritative state"),
		SnapshotStruct->FindPropertyByName(TEXT("MatchPlayState")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexNetworkBootstrapArchitectureTest,
	"FMCodex.NetworkPlay.NetworkBootstrap.04.ArchitectureAndOptIn",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexNetworkBootstrapArchitectureTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexNetworkBootstrapTests;
	const AFMCodexNetworkMatchGameMode* Defaults =
		GetDefault<AFMCodexNetworkMatchGameMode>();
	TestTrue(TEXT("Network GameMode spawns Network PlayerController"),
		Defaults->PlayerControllerClass == AFMCodexNetworkMatchPlayerController::StaticClass());
	TestTrue(TEXT("Network GameMode spawns Network GameState"),
		Defaults->GameStateClass == AFMCodexNetworkMatchGameState::StaticClass());
	TestTrue(TEXT("Network GameMode spawns Network PlayerState"),
		Defaults->PlayerStateClass == AFMCodexNetworkMatchPlayerState::StaticClass());

	FString RuntimeSource;
	FString RegistrySource;
	FString ControllerHeader;
	FString GameModeSource;
	FString GameStateHeader;
	FString PlayerStateHeader;
	FString DefaultEngine;
	TestTrue(TEXT("Network runtime source loads"), LoadProductionSource(
		TEXT("Source/FMCodex/NetworkPlay/FMCodexNetworkMatchRuntime.cpp"),
		RuntimeSource));
	TestTrue(TEXT("Participant registry source loads"), LoadProductionSource(
		TEXT("Source/FMCodex/NetworkPlay/FMCodexNetworkParticipantRegistry.cpp"),
		RegistrySource));
	TestTrue(TEXT("Network controller header loads"), LoadProductionSource(
		TEXT("Source/FMCodex/NetworkPlay/FMCodexNetworkMatchPlayerController.h"),
		ControllerHeader));
	TestTrue(TEXT("Network game mode source loads"), LoadProductionSource(
		TEXT("Source/FMCodex/NetworkPlay/FMCodexNetworkMatchGameMode.cpp"),
		GameModeSource));
	TestTrue(TEXT("Network game state header loads"), LoadProductionSource(
		TEXT("Source/FMCodex/NetworkPlay/FMCodexNetworkMatchGameState.h"),
		GameStateHeader));
	TestTrue(TEXT("Network player state header loads"), LoadProductionSource(
		TEXT("Source/FMCodex/NetworkPlay/FMCodexNetworkMatchPlayerState.h"),
		PlayerStateHeader));
	TestTrue(TEXT("Default engine configuration loads"), LoadProductionSource(
		TEXT("Config/DefaultEngine.ini"), DefaultEngine));

	TestTrue(TEXT("Runtime derives every client snapshot through BuildForViewer"),
		RuntimeSource.Contains(TEXT("BuildForViewer(")));
	TestFalse(TEXT("Runtime never requests FullyDisclosed client truth"),
		RuntimeSource.Contains(TEXT("FullyDisclosed")));
	TestFalse(TEXT("Connection admission never distinguishes local host"),
		RegistrySource.Contains(TEXT("IsLocalController"))
			|| RegistrySource.Contains(TEXT("HasAuthority")));
	TestTrue(TEXT("Client view is owner-only replicated"),
		ControllerHeader.Contains(TEXT("ReplicatedUsing = OnRep_OwnerView")));
	TestTrue(TEXT("Stage 7.3 has a typed reliable owner intent RPC"),
		ControllerHeader.Contains(TEXT("UFUNCTION(Server, Reliable)"))
			&& ControllerHeader.Contains(TEXT("FFMCodexNetworkPlayerIntentEnvelope")));
	TestFalse(TEXT("GameState carries no raw authoritative State"),
		GameStateHeader.Contains(TEXT("FMatchPlayState")));
	TestFalse(TEXT("PlayerState carries no raw authoritative State"),
		PlayerStateHeader.Contains(TEXT("FMatchPlayState")));
	TestFalse(TEXT("Network mode does not depend on Local Host GameMode"),
		GameModeSource.Contains(TEXT("LocalMatchHostGameMode")));
	TestTrue(TEXT("Third connection has explicit fail-closed reason"),
		GameModeSource.Contains(TEXT("MatchFull")));
	TestTrue(TEXT("LocalPlay remains the project default GameMode"),
		DefaultEngine.Contains(TEXT(
			"GlobalDefaultGameMode=/Script/FMCodex.FMCodexLocalMatchHostGameMode")));
	return true;
}

#endif
