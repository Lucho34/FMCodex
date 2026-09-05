#include "FMCodexNetworkMatchPlayerController.h"

#include "FMCodexNetworkMatchGameState.h"
#include "FMCodexNetworkMatchGameMode.h"
#include "FMCodexNetworkMatchPlayerState.h"

#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SOverlay.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SButton.h"
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
			return TEXT("等待你掷本回合 Full D12");
		case EFMCodexNetworkClientInteractionState
			::WaitingForOpponentInitialActionPoint:
			return TEXT("等待对手掷本回合 Full D12");
		case EFMCodexNetworkClientInteractionState::WaitingForOwnIntent:
			return TEXT("等待你的操作");
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
#if !UE_BUILD_SHIPPING
		bShowMouseCursor = true;
		SetInputMode(FInputModeGameAndUI());
#endif
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
	IntentClientState.ObserveView(OwnerView);
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
	IntentClientState.ObserveView(OwnerView);
	UE_LOG(LogFMCodexNetworkPlay, Log,
		TEXT("Owner view applied: Revision=%d Pending=%lld DeploymentCount=%d LastSide=%d Card=%s Slot=%s"),
		OwnerView.ViewRevision, IntentClientState.GetPendingRequestId(), OwnerView.DeploymentCount,
		static_cast<int32>(OwnerView.LastDeployment.Side),
		*OwnerView.LastDeployment.Placement.Choice.CardId.ToString(),
		*OwnerView.LastDeployment.Placement.Choice.SlotId.ToString());
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
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						StatusText.ToSharedRef()
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0, 12, 0, 0)
					[
						SNew(SButton)
						.Visibility_Lambda([this]()
						{
							return OwnerView.InteractionState ==
								EFMCodexNetworkClientInteractionState::WaitingForOwnInitialActionPoint
								? EVisibility::Visible : EVisibility::Collapsed;
						})
						.IsEnabled_Lambda([this]() { return CanRequestInitialActionPointRoll(); })
						.Text(LOCTEXT("RollInitialD12", "掷本回合 Full D12"))
						.OnClicked_Lambda([this]()
						{
							DevRequestInitialActionPointRoll();
							return FReply::Handled();
						})
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0, 12, 0, 0)
					[
						SNew(SButton)
						.Visibility_Lambda([this]() { return OwnerView.DeploymentOptions.IsEmpty()
							? EVisibility::Collapsed : EVisibility::Visible; })
						.IsEnabled_Lambda([this]() { return CanDeployOrdinary(); })
						.Text_Lambda([this]()
						{
							if (OwnerView.DeploymentOptions.IsEmpty()) { return LOCTEXT("Deploy", "部署球员"); }
							const auto& Option = OwnerView.DeploymentOptions[0];
							return FText::Format(LOCTEXT("DeployChoice", "部署 {0} → {1}"),
								Option.CardLabel, Option.SlotLabel);
						})
						.OnClicked_Lambda([this]()
						{
							DevDeployOrdinary();
							return FReply::Handled();
						})
					]
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

	FString EntryText = TEXT("尚未公开本回合骰子");
	if (OwnerView.DisclosedInitialD12 != 0)
	{
		const TCHAR* Branch = OwnerView.EntryBranch == EFMCodexNetworkEntryBranch::SendingOff
			? TEXT("罚下事件（AP1）") : OwnerView.EntryBranch == EFMCodexNetworkEntryBranch::Ordinary
			? TEXT("运动战") : OwnerView.EntryBranch == EFMCodexNetworkEntryBranch::SetPiece
			? TEXT("定位球") : TEXT("等待服务器");
		const TCHAR* Wait = OwnerView.EntryWait == EFMCodexNetworkEntryWait::TerminalPendingAdvance
			? TEXT("已结算，等待下一回合") : OwnerView.EntryWait == EFMCodexNetworkEntryWait::Deployment
			? TEXT("等待部署") : OwnerView.EntryWait == EFMCodexNetworkEntryWait::SetPieceTypeRoll
			? TEXT("等待定位球类型掷点") : TEXT("等待服务器");
		EntryText = FString::Printf(TEXT("已公开 Full D12：%d · %s\n%s（完成部署与战术尚未联网）"),
			OwnerView.DisclosedInitialD12, Branch, Wait);
	}
	if (OwnerView.DeploymentCount > 0)
	{
		const auto& Last = OwnerView.LastDeployment;
		EntryText += FString::Printf(TEXT("\n已部署 %d 张 · 最近：玩家 %s · %s → %s"),
			OwnerView.DeploymentCount, *SideLabel(Last.Side),
			*Last.Placement.CardLabel.ToString(), *Last.Placement.SlotLabel.ToString());
	}
	const auto& Ack = IntentClientState.GetLastAck();
	FString AckText = TEXT("暂无请求回执");
	switch (Ack.Code)
	{
	case EFMCodexNetworkIntentAckCode::Accepted: AckText = TEXT("服务器已接受"); break;
	case EFMCodexNetworkIntentAckCode::MatchMismatch: AckText = TEXT("比赛实例不匹配"); break;
	case EFMCodexNetworkIntentAckCode::NotParticipant: AckText = TEXT("连接未加入比赛"); break;
	case EFMCodexNetworkIntentAckCode::WrongSide: AckText = TEXT("当前不属于你的操作"); break;
	case EFMCodexNetworkIntentAckCode::StaleAttackSequence: AckText = TEXT("攻击序列已过期"); break;
	case EFMCodexNetworkIntentAckCode::NotPlayerIntent: AckText = TEXT("不支持此玩家意图"); break;
	case EFMCodexNetworkIntentAckCode::InvalidPayload: AckText = TEXT("请求格式无效"); break;
	case EFMCodexNetworkIntentAckCode::InvalidPhase: AckText = TEXT("当前阶段不可操作"); break;
	case EFMCodexNetworkIntentAckCode::DuplicateOrAlreadyResolved: AckText = TEXT("重复或已处理的请求"); break;
	case EFMCodexNetworkIntentAckCode::AuthorityRejected: AckText = TEXT("权威规则拒绝请求"); break;
	case EFMCodexNetworkIntentAckCode::InternalFailure: AckText = TEXT("服务器内部推进失败"); break;
	default: break;
	}
	const FString IntentStatus = FString::Printf(
		TEXT("待确认请求：%lld · %s\n最近回执：%s · Request %lld · Revision %d\n%s"),
		IntentClientState.GetPendingRequestId(),
		IntentClientState.IsPending() ? TEXT("等待回执及权威视图") : TEXT("无等待请求"),
		*AckText, Ack.RequestId, Ack.ViewRevision, *EntryText);
	return FText::FromString(FString::Printf(
		TEXT("FMCODEX · 网络玩家意图测试\n")
		TEXT("%s  |  玩家：%s  |  队伍：%s\n")
		TEXT("比赛实例：%s\n")
		TEXT("服务器状态：%s\n")
		TEXT("客户端视图：Side %s · Revision %d\n")
		TEXT("比分：%d - %d  |  进攻方：%s  |  Attack #%lld\n")
		TEXT("%s\n%s"),
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
		*InteractionLabel(OwnerView.InteractionState), *IntentStatus));
}

bool AFMCodexNetworkMatchPlayerController::CanRequestInitialActionPointRoll() const
{
	return IsLocalController() && !IntentClientState.IsPending()
		&& OwnerView.bMatchInitialized
		&& OwnerView.BootstrapState == EFMCodexNetworkBootstrapState::MatchReady
		&& OwnerView.InteractionState ==
			EFMCodexNetworkClientInteractionState::WaitingForOwnInitialActionPoint;
}

void AFMCodexNetworkMatchPlayerController::DevRequestInitialActionPointRoll()
{
#if !UE_BUILD_SHIPPING
	if (!IsLocalController())
	{
		return;
	}
	FFMCodexNetworkPlayerIntentEnvelope Envelope;
	if (!IntentClientState.Begin(OwnerView, Envelope))
	{
		return;
	}
	RefreshNetworkBootstrapUI();
	UE_LOG(LogFMCodexNetworkPlay, Log,
		TEXT("Intent owner submit: Match=%s Request=%lld ViewerSide=%d ExpectedSequence=%lld"),
		*Envelope.MatchInstanceId.ToString(EGuidFormats::DigitsWithHyphensLower),
		Envelope.RequestId, static_cast<int32>(OwnerView.ViewerSide), Envelope.ExpectedAttackSequence);
	// Always invoke the generated RPC wrapper, including the listen host.
	ServerSubmitPlayerIntent(Envelope);
#endif
}

bool AFMCodexNetworkMatchPlayerController::CanDeployOrdinary() const
{
	return IsLocalController() && !IntentClientState.IsPending()
		&& OwnerView.bMatchInitialized && OwnerView.BootstrapState == EFMCodexNetworkBootstrapState::MatchReady
		&& OwnerView.ExpectedActingSide == OwnerView.ViewerSide
		&& OwnerView.EntryBranch == EFMCodexNetworkEntryBranch::Ordinary
		&& OwnerView.EntryWait == EFMCodexNetworkEntryWait::Deployment
		&& !OwnerView.DeploymentOptions.IsEmpty();
}
void AFMCodexNetworkMatchPlayerController::DevDeployOrdinary()
{
#if !UE_BUILD_SHIPPING
	if (CanDeployOrdinary()) { SubmitDeploymentChoice(OwnerView.DeploymentOptions[0].Choice); }
#endif
}
void AFMCodexNetworkMatchPlayerController::SubmitDeploymentChoice(const FFMCodexNetworkDeployOrdinaryPayload& Choice)
{
#if !UE_BUILD_SHIPPING
	FFMCodexNetworkPlayerIntentEnvelope Envelope;
	if (!IsLocalController() || !IntentClientState.BeginDeployment(OwnerView, Choice, Envelope)) { return; }
	RefreshNetworkBootstrapUI();
	UE_LOG(LogFMCodexNetworkPlay, Log,
		TEXT("Deployment owner submit: Match=%s Request=%lld ViewerSide=%d ExpectedSequence=%lld Card=%s Slot=%s"),
		*Envelope.MatchInstanceId.ToString(EGuidFormats::DigitsWithHyphensLower),
		Envelope.RequestId, static_cast<int32>(OwnerView.ViewerSide), Envelope.ExpectedAttackSequence,
		*Envelope.Deployment.CardId.ToString(), *Envelope.Deployment.SlotId.ToString());
	// Same generated reliable owning RPC on host and remote; no direct implementation call.
	ServerSubmitPlayerIntent(Envelope);
#endif
}
void AFMCodexNetworkMatchPlayerController::DevProbeInvalidDeploymentCard()
{
#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
	if (!CanDeployOrdinary()) { return; }
	auto Choice = OwnerView.DeploymentOptions[0].Choice;
	Choice.CardId = TEXT("DEV.NonexistentCard");
	SubmitDeploymentChoice(Choice);
#endif
}
void AFMCodexNetworkMatchPlayerController::DevProbeWrongSideInitialD12()
{
#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
	// Deliberately bypass client eligibility for a fresh nonacting connection.
	// This is a negative protocol probe, not a normal pending/UI request.
	if (!IsLocalController() || !OwnerView.bMatchInitialized
		|| OwnerView.InteractionState !=
			EFMCodexNetworkClientInteractionState::WaitingForOpponentInitialActionPoint)
	{
		return;
	}
	FFMCodexNetworkPlayerIntentEnvelope Envelope;
	Envelope.MatchInstanceId = OwnerView.MatchInstanceId;
	Envelope.RequestId = 1;
	Envelope.ExpectedAttackSequence = OwnerView.AttackSequence;
	Envelope.IntentKind = EFMCodexNetworkPlayerIntentKind::RequestInitialActionPointRoll;
	UE_LOG(LogFMCodexNetworkPlay, Log,
		TEXT("DEV negative owner RPC: Match=%s Request=1 ViewerSide=%d ExpectedSequence=%lld"),
		*Envelope.MatchInstanceId.ToString(EGuidFormats::DigitsWithHyphensLower),
		static_cast<int32>(OwnerView.ViewerSide), Envelope.ExpectedAttackSequence);
	ServerSubmitPlayerIntent(Envelope);
#endif
}
void AFMCodexNetworkMatchPlayerController::ServerSubmitPlayerIntent_Implementation(
	const FFMCodexNetworkPlayerIntentEnvelope& Envelope)
{
	// A direct developer reflection call must not masquerade as a server receipt.
	if (!HasAuthority())
	{
		return;
	}
	AFMCodexNetworkMatchGameMode* Mode = GetWorld()
		? GetWorld()->GetAuthGameMode<AFMCodexNetworkMatchGameMode>() : nullptr;
	FFMCodexNetworkPlayerIntentAck Ack;
	if (Mode)
	{
		Ack = Mode->SubmitConnectionPlayerIntent(this, Envelope);
	}
	else
	{
		Ack.MatchInstanceId = Envelope.MatchInstanceId;
		Ack.RequestId = Envelope.RequestId;
		Ack.Code = EFMCodexNetworkIntentAckCode::InternalFailure;
	}
	ClientReceivePlayerIntentAck(Ack);
}

void AFMCodexNetworkMatchPlayerController::ClientReceivePlayerIntentAck_Implementation(
	const FFMCodexNetworkPlayerIntentAck& Ack)
{
	const bool bCorrelated = IntentClientState.ObserveAck(Ack);
	UE_LOG(LogFMCodexNetworkPlay, Log,
		TEXT("Intent ACK: Match=%s Request=%lld ACK=%s Revision=%d Correlated=%d Pending=%lld"),
		*Ack.MatchInstanceId.ToString(EGuidFormats::DigitsWithHyphensLower), Ack.RequestId,
		*StaticEnum<EFMCodexNetworkIntentAckCode>()->GetNameStringByValue(static_cast<int64>(Ack.Code)),
		Ack.ViewRevision, bCorrelated ? 1 : 0, IntentClientState.GetPendingRequestId());
	RefreshNetworkBootstrapUI();
}
#undef LOCTEXT_NAMESPACE
