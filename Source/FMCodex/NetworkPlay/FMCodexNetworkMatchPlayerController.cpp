#include "FMCodexNetworkMatchPlayerController.h"

#include "FMCodexNetworkMatchGameState.h"
#include "FMCodexNetworkMatchPlayerState.h"

#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "FMCodexNetworkMatchPlayerController"

namespace FMCodexNetworkMatchPlayerController
{
	FString SideLabel(const EInitialTurnOrderPlayer Side)
	{
		switch (Side)
		{
		case EInitialTurnOrderPlayer::PlayerA: return TEXT("A");
		case EInitialTurnOrderPlayer::PlayerB: return TEXT("B");
		default: return TEXT("未分配");
		}
	}

	FString BootstrapLabel(const EFMCodexNetworkBootstrapState State)
	{
		switch (State)
		{
		case EFMCodexNetworkBootstrapState::WaitingForPlayers:
			return TEXT("等待两名玩家连接");
		case EFMCodexNetworkBootstrapState::MatchReady:
			return TEXT("比赛已由服务器初始化");
		case EFMCodexNetworkBootstrapState::ParticipantDisconnected:
			return TEXT("一名玩家已断开；本阶段不会重分配阵营");
		case EFMCodexNetworkBootstrapState::MatchEnded:
			return TEXT("比赛已结束");
		default:
			return TEXT("网络比赛初始化失败");
		}
	}

	FString InteractionLabel(
		const EFMCodexNetworkClientInteractionState State)
	{
		switch (State)
		{
		case EFMCodexNetworkClientInteractionState
			::WaitingForOwnInitialActionPoint:
			return TEXT("等待你掷本回合 Full D12（Stage 7.3 接入）");
		case EFMCodexNetworkClientInteractionState
			::WaitingForOpponentInitialActionPoint:
			return TEXT("等待对手掷本回合 Full D12");
		case EFMCodexNetworkClientInteractionState::WaitingForOwnIntent:
			return TEXT("等待你的玩家操作（Stage 7.3 接入）");
		case EFMCodexNetworkClientInteractionState::WaitingForOpponentIntent:
			return TEXT("等待对手操作");
		case EFMCodexNetworkClientInteractionState::MatchEnded:
			return TEXT("权威比赛已结束");
		default:
			return TEXT("等待网络比赛就绪");
		}
	}
}

AFMCodexNetworkMatchPlayerController::AFMCodexNetworkMatchPlayerController()
{
	bReplicates = true;
}

void AFMCodexNetworkMatchPlayerController::BeginPlay()
{
	Super::BeginPlay();
	if (IsLocalController())
	{
		InitializeDeveloperStatusUI();
		RefreshNetworkBootstrapUI();
	}
}

void AFMCodexNetworkMatchPlayerController::EndPlay(
	const EEndPlayReason::Type EndPlayReason)
{
#if !UE_BUILD_SHIPPING
	if (StatusViewportWidget.IsValid() && GEngine != nullptr
		&& GEngine->GameViewport != nullptr)
	{
		GEngine->GameViewport->RemoveViewportWidgetContent(
			StatusViewportWidget.ToSharedRef());
	}
	StatusText.Reset();
	StatusViewportWidget.Reset();
#endif
	Super::EndPlay(EndPlayReason);
}

void AFMCodexNetworkMatchPlayerController::SetOwnerViewOnServer(
	const FFMCodexNetworkClientViewSnapshot& InOwnerView)
{
	check(HasAuthority());
	OwnerView = InOwnerView;
	ForceNetUpdate();
	if (IsLocalController())
	{
		RefreshNetworkBootstrapUI();
	}
}

const FFMCodexNetworkClientViewSnapshot&
AFMCodexNetworkMatchPlayerController::GetOwnerView() const
{
	return OwnerView;
}

void AFMCodexNetworkMatchPlayerController::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(
		AFMCodexNetworkMatchPlayerController,
		OwnerView,
		COND_OwnerOnly);
}

void AFMCodexNetworkMatchPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	// Identity OnRep may precede this controller's PlayerState association.
	// Re-read current replicated facts regardless of their arrival order.
	RefreshNetworkBootstrapUI();
}

void AFMCodexNetworkMatchPlayerController::OnRep_OwnerView()
{
	UE_LOG(LogFMCodexNetworkPlay, Log,
		TEXT("Client owner-safe View: MatchInstanceId=%s Side=%d Revision=%d Ready=%d."),
		*OwnerView.MatchInstanceId.ToString(
			EGuidFormats::DigitsWithHyphensLower),
		static_cast<int32>(OwnerView.ViewerSide),
		OwnerView.ViewRevision,
		OwnerView.bMatchInitialized ? 1 : 0);
	RefreshNetworkBootstrapUI();
}

void AFMCodexNetworkMatchPlayerController::InitializeDeveloperStatusUI()
{
#if !UE_BUILD_SHIPPING
	if (StatusViewportWidget.IsValid() || GEngine == nullptr
		|| GEngine->GameViewport == nullptr)
	{
		return;
	}
	SAssignNew(StatusText, STextBlock)
		.Font(FCoreStyle::GetDefaultFontStyle("Regular", 15))
		.ColorAndOpacity(FLinearColor(0.91f, 0.95f, 1.0f))
		.AutoWrapText(true);
	StatusViewportWidget =
		SNew(SOverlay)
		+ SOverlay::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Top)
		.Padding(FMargin(24.0f, 24.0f, 0.0f, 0.0f))
		[
			SNew(SBox)
			.WidthOverride(570.0f)
			[
				SNew(SBorder)
				.Padding(FMargin(18.0f, 14.0f))
				.BorderBackgroundColor(
					FLinearColor(0.025f, 0.06f, 0.10f, 0.94f))
				[
					StatusText.ToSharedRef()
				]
			]
		];
	GEngine->GameViewport->AddViewportWidgetContent(
		StatusViewportWidget.ToSharedRef(), 150);
#endif
}

void AFMCodexNetworkMatchPlayerController::RefreshNetworkBootstrapUI()
{
#if !UE_BUILD_SHIPPING
	if (StatusText.IsValid())
	{
		StatusText->SetText(BuildStatusText());
	}
#endif
}

FText AFMCodexNetworkMatchPlayerController::BuildStatusText() const
{
	using namespace FMCodexNetworkMatchPlayerController;
	const AFMCodexNetworkMatchPlayerState* NetworkPlayerState =
		GetPlayerState<AFMCodexNetworkMatchPlayerState>();
	const AFMCodexNetworkMatchGameState* NetworkGameState =
		GetWorld() != nullptr
			? GetWorld()->GetGameState<AFMCodexNetworkMatchGameState>()
			: nullptr;
	const FString PlayerName = NetworkPlayerState != nullptr
		? NetworkPlayerState->GetNetworkPlayerDisplayName()
		: TEXT("连接中");
	const FString TeamName = NetworkPlayerState != nullptr
		? NetworkPlayerState->GetTeamIdentity().TeamDisplayName
		: TEXT("待分配");
	const EInitialTurnOrderPlayer AssignedSide = NetworkPlayerState != nullptr
		? NetworkPlayerState->GetGameplaySide()
		: EInitialTurnOrderPlayer::None;
	const EFMCodexNetworkBootstrapState State = NetworkGameState != nullptr
		? NetworkGameState->GetBootstrapState()
		: OwnerView.BootstrapState;
	const FString MatchId = NetworkGameState != nullptr
		&& NetworkGameState->GetMatchInstanceId().IsValid()
			? NetworkGameState->GetMatchInstanceId().ToString(
				EGuidFormats::DigitsWithHyphensLower)
			: TEXT("同步中");

	return FText::FromString(FString::Printf(
		TEXT("FMCODEX · LISTEN SERVER BOOTSTRAP\n")
		TEXT("%s  |  玩家：%s  |  队伍：%s\n")
		TEXT("比赛实例：%s\n")
		TEXT("服务器状态：%s\n")
		TEXT("客户端视图：Side %s · Revision %d\n")
		TEXT("比分：%d - %d  |  进攻方：%s  |  Attack #%lld\n")
		TEXT("%s"),
		HasAuthority() ? TEXT("监听主机玩家") : TEXT("远端客户端玩家"),
		*PlayerName,
		*TeamName,
		*MatchId,
		*BootstrapLabel(State),
		*SideLabel(AssignedSide),
		OwnerView.ViewRevision,
		OwnerView.PlayerAScore,
		OwnerView.PlayerBScore,
		*SideLabel(OwnerView.CurrentAttackingSide),
		OwnerView.AttackSequence,
		*InteractionLabel(OwnerView.InteractionState)));
}

#undef LOCTEXT_NAMESPACE
