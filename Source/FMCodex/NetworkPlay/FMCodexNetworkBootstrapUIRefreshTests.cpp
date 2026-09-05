#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING

#include "Misc/AutomationTest.h"
#include "FMCodexNetworkRNGTestEntropy.h"
#include "FMCodexNetworkMatchGameMode.h"
#include "FMCodexNetworkMatchGameState.h"
#include "FMCodexNetworkMatchPlayerController.h"
#include "FMCodexNetworkMatchPlayerState.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformProcess.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"

// Test-only access to the actual Slate text and replication callbacks. No
// production cache/formatter or gameplay mutation seam is introduced.
struct FFMCodexNetworkBootstrapUIRefreshTestAccess
{
	static void AttachPanel(AFMCodexNetworkMatchPlayerController& Controller,
		const bool bRefresh = true)
	{
		Controller.StatusViewportWidget = SNew(SBorder)
		[
			SAssignNew(Controller.StatusText, STextBlock)
		];
		if (bRefresh)
		{
			Controller.RefreshNetworkBootstrapUI();
		}
	}

	static FString Text(const AFMCodexNetworkMatchPlayerController& Controller)
	{
		return Controller.StatusText->GetText().ToString();
	}

	static const SWidget* Panel(const AFMCodexNetworkMatchPlayerController& Controller)
	{
		return Controller.StatusViewportWidget.Get();
	}

	static void AssociatePlayerState(AFMCodexNetworkMatchPlayerController& Controller,
		AFMCodexNetworkMatchPlayerState* PlayerState)
	{
		Controller.PlayerState = PlayerState;
		Controller.OnRep_PlayerState();
	}

	static void ReceiveOwnerView(AFMCodexNetworkMatchPlayerController& Controller,
		const FFMCodexNetworkClientViewSnapshot& View)
	{
		Controller.OwnerView = View;
		Controller.OnRep_OwnerView();
	}

	static void RepeatRefresh(AFMCodexNetworkMatchPlayerController& Controller)
	{
		Controller.InitializeDeveloperStatusUI();
		Controller.OnRep_PlayerState();
		Controller.OnRep_OwnerView();
		Controller.RefreshNetworkBootstrapUI();
	}

	static void BeginPlay(AFMCodexNetworkMatchPlayerController& Controller)
	{
		Controller.DispatchBeginPlay();
	}
};

namespace FMCodexNetworkBootstrapUIRefreshTests
{
	using FAccess = FFMCodexNetworkBootstrapUIRefreshTestAccess;

	struct FFixture
	{
		UWorld* World = nullptr;
		AFMCodexNetworkMatchPlayerController* Controller = nullptr;
		AFMCodexNetworkMatchPlayerState* PlayerState = nullptr;
		AFMCodexNetworkMatchGameState* GameState = nullptr;
		const FGuid MatchId = FGuid(0x10203040, 0x50607080, 0x90A0B0C0, 0xD0E0F001);
		FFMCodexNetworkBootstrapConfiguration Config =
			FFMCodexNetworkBootstrapConfigurationFactory::CreatePrototypeMatch();
		FFMCodexNetworkClientViewSnapshot View;

		explicit FFixture(const EInitialTurnOrderPlayer ViewerSide = EInitialTurnOrderPlayer::PlayerB)
		{
			World = UWorld::CreateWorld(EWorldType::Game, false);
			if (World == nullptr || GEngine == nullptr)
			{
				return;
			}
			GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
			Controller = World->SpawnActor<AFMCodexNetworkMatchPlayerController>();
			// This isolated world has not run actor component initialization.
			// Register the controller as the engine does in PostInitializeComponents.
			World->AddController(Controller);
			PlayerState = World->SpawnActor<AFMCodexNetworkMatchPlayerState>();
			FFMCodexNetworkMatchRuntime Runtime(MatchId, MakeUnique<FFMCodexNetworkScriptedEntropy>());
			Runtime.InitializeOnce(Config);
			View = Runtime.BuildClientView(ViewerSide, 3,
				EFMCodexNetworkBootstrapState::MatchReady);
		}

		~FFixture()
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

		void Identity(const EInitialTurnOrderPlayer Side = EInitialTurnOrderPlayer::PlayerB)
		{
			PlayerState->SetNetworkIdentityOnServer(Side,
				Side == EInitialTurnOrderPlayer::PlayerA ? Config.PlayerATeam : Config.PlayerBTeam,
				Side == EInitialTurnOrderPlayer::PlayerA ? TEXT("玩家 A") : TEXT("玩家 B"));
			// The same notification as the real identity RepNotify, including
			// the association guard that originally lost this notification.
			PlayerState->ProcessEvent(PlayerState->FindFunctionChecked(
				TEXT("OnRep_NetworkIdentity")), nullptr);
		}

		void PublicBootstrap()
		{
			if (GameState == nullptr)
			{
				GameState = World->SpawnActor<AFMCodexNetworkMatchGameState>();
				World->SetGameState(GameState);
			}
			GameState->SetBootstrapStateOnServer(MatchId,
				EFMCodexNetworkBootstrapState::MatchReady, {}, {});
			GameState->ProcessEvent(GameState->FindFunctionChecked(
				TEXT("OnRep_PublicBootstrap")), nullptr);
		}
	};
}

IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFMCodexNetworkBootstrapReplicationOrderTest,
	"FMCodex.NetworkPlay.NetworkBootstrap.05.ReplicationOrder",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

void FFMCodexNetworkBootstrapReplicationOrderTest::GetTests(
	TArray<FString>& OutBeautifiedNames, TArray<FString>& OutTestCommands) const
{
	for (const TCHAR* Order : {
		TEXT("A.IdentityAssociationOwnerView"),
		TEXT("B.OwnerViewIdentityAssociation"),
		TEXT("C.GameStateOwnerViewPlayerState"),
		TEXT("D.PlayerStateOwnerViewGameState") })
	{
		OutBeautifiedNames.Add(Order);
		OutTestCommands.Add(Order);
	}
}

bool FFMCodexNetworkBootstrapReplicationOrderTest::RunTest(const FString& Parameters)
{
	using namespace FMCodexNetworkBootstrapUIRefreshTests;
	FFixture Fixture;
	if (!TestNotNull(TEXT("Controller exists"), Fixture.Controller)
		|| !TestNotNull(TEXT("PlayerState exists"), Fixture.PlayerState))
	{
		return false;
	}
	FAccess::AttachPanel(*Fixture.Controller);
	TestTrue(TEXT("Missing identity renders partial status"),
		FAccess::Text(*Fixture.Controller).Contains(TEXT("Side 未分配")));
	const auto Associate = [&]() {
		FAccess::AssociatePlayerState(*Fixture.Controller, Fixture.PlayerState);
	};
	const auto OwnerView = [&]() {
		FAccess::ReceiveOwnerView(*Fixture.Controller, Fixture.View);
	};
	if (Parameters.StartsWith(TEXT("A.")))
	{
		Fixture.Identity();
		Associate();
		TestTrue(TEXT("Association alone recovers early identity"),
			FAccess::Text(*Fixture.Controller).Contains(TEXT("玩家 B")));
		OwnerView();
		Fixture.PublicBootstrap();
	}
	else if (Parameters.StartsWith(TEXT("B.")))
	{
		Fixture.PublicBootstrap();
		OwnerView();
		Fixture.Identity();
		TestTrue(TEXT("Identity callback before association cannot find owner"),
			FAccess::Text(*Fixture.Controller).Contains(TEXT("Side 未分配")));
		TestTrue(TEXT("Owner view is already visible"),
			FAccess::Text(*Fixture.Controller).Contains(TEXT("Revision 3")));
		Associate(); // Last event: regression fails without OnRep_PlayerState repair.
	}
	else if (Parameters.StartsWith(TEXT("C.")))
	{
		Fixture.PublicBootstrap();
		OwnerView();
		Associate();
		Fixture.Identity(); // Identity RepNotify must refresh an associated owner.
	}
	else
	{
		Associate();
		Fixture.Identity();
		OwnerView();
		Fixture.PublicBootstrap(); // Public RepNotify is the last arrival.
	}
	TestTrue(TEXT("Super associates PlayerState owner via ClientInitialize"),
		Fixture.PlayerState->GetOwner() == Fixture.Controller);
	const FString FinalText = FAccess::Text(*Fixture.Controller);
	TestTrue(TEXT("Visible player identity converges"), FinalText.Contains(TEXT("玩家 B")));
	TestTrue(TEXT("Visible team identity converges"), FinalText.Contains(TEXT("曼彻斯特城")));
	TestTrue(TEXT("Visible assigned Side converges"), FinalText.Contains(TEXT("Side B")));
	TestFalse(TEXT("No stale unassigned text remains"), FinalText.Contains(TEXT("待分配")));
	TestTrue(TEXT("Visible bootstrap is ready"), FinalText.Contains(TEXT("比赛已由服务器初始化")));
	TestTrue(TEXT("Visible match id comes from replicated facts"), FinalText.Contains(
		Fixture.MatchId.ToString(EGuidFormats::DigitsWithHyphensLower)));
	TestTrue(TEXT("Visible initial score and sequence are preserved"),
		FinalText.Contains(TEXT("比分：0 - 0")) && FinalText.Contains(TEXT("Attack #1")));
	TestTrue(TEXT("Visible safe waiting status is present"), FinalText.Contains(TEXT("Full D12")));
	TestEqual(TEXT("Association cannot republish or change revision"),
		Fixture.Controller->GetOwnerView().ViewRevision, 3);

	// Compare actual rendered text against a different arrival order.
	FFixture Reference;
	FAccess::AttachPanel(*Reference.Controller);
	FAccess::AssociatePlayerState(*Reference.Controller, Reference.PlayerState);
	Reference.Identity();
	Reference.PublicBootstrap();
	FAccess::ReceiveOwnerView(*Reference.Controller, Reference.View);
	TestEqual(TEXT("All orders produce identical final Slate text"), FinalText,
		FAccess::Text(*Reference.Controller));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexNetworkBootstrapHostRefreshTest,
	"FMCodex.NetworkPlay.NetworkBootstrap.06.HostAndIdempotentRefresh",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexNetworkBootstrapHostRefreshTest::RunTest(const FString& Parameters)
{
	using namespace FMCodexNetworkBootstrapUIRefreshTests;
	FFixture Fixture(EInitialTurnOrderPlayer::PlayerA);
	if (!TestNotNull(TEXT("Controller exists"), Fixture.Controller)
		|| !TestNotNull(TEXT("PlayerState exists"), Fixture.PlayerState))
	{
		return false;
	}
	// Replication may arrive before BeginPlay creates presentation.
	Fixture.Controller->PlayerState = Fixture.PlayerState;
	Fixture.Identity(EInitialTurnOrderPlayer::PlayerA);
	Fixture.PublicBootstrap();
	Fixture.Controller->SetOwnerViewOnServer(Fixture.View);
	FAccess::AttachPanel(*Fixture.Controller, false);
	FAccess::BeginPlay(*Fixture.Controller);
	const FString Before = FAccess::Text(*Fixture.Controller);
	const SWidget* OriginalPanel = FAccess::Panel(*Fixture.Controller);
	TestTrue(TEXT("Host keeps its role and identity"),
		Before.Contains(TEXT("监听主机玩家")) && Before.Contains(TEXT("玩家 A"))
		&& Before.Contains(TEXT("阿森纳")) && Before.Contains(TEXT("Side A")));
	TestTrue(TEXT("Host remains MatchReady"), Before.Contains(TEXT("比赛已由服务器初始化")));
	for (int32 Repeat = 0; Repeat < 5; ++Repeat)
	{
		FAccess::RepeatRefresh(*Fixture.Controller);
		Fixture.Identity(EInitialTurnOrderPlayer::PlayerA);
		Fixture.PublicBootstrap();
		TestTrue(TEXT("Refresh never creates a second panel"),
			FAccess::Panel(*Fixture.Controller) == OriginalPanel);
		TestEqual(TEXT("Repeated callbacks replace text idempotently"),
			FAccess::Text(*Fixture.Controller), Before);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexNetworkPlayerDisplayNameFallbackTest,
	"FMCodex.NetworkPlay.NetworkBootstrap.07.PlayerDisplayNameFallback",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexNetworkPlayerDisplayNameFallbackTest::RunTest(const FString& Parameters)
{
	for (const EInitialTurnOrderPlayer Side :
		{ EInitialTurnOrderPlayer::PlayerA, EInitialTurnOrderPlayer::PlayerB })
	{
		const FString Expected = Side == EInitialTurnOrderPlayer::PlayerA
			? TEXT("玩家 A") : TEXT("玩家 B");
		for (const FString& Candidate : TArray<FString>{ TEXT(""), TEXT("  "),
			TEXT("Player"), TEXT(" player "),
			FString(FPlatformProcess::ComputerName()) + TEXT("-012345") })
		{
			TestEqual(TEXT("Unusable player name preserves side fallback"),
				AFMCodexNetworkMatchGameMode::SelectPlayerDisplayName(Candidate, Side), Expected);
		}
		TestEqual(TEXT("Valid display name is retained and trimmed"),
			AFMCodexNetworkMatchGameMode::SelectPlayerDisplayName(TEXT("  测试玩家  "), Side),
			FString(TEXT("测试玩家")));
	}
	return true;
}

#endif
