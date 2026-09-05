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
#include "Widgets/Layout/SScrollBox.h"
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
	ParticipantChoices.Reset();
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
	UE_LOG(LogFMCodexNetworkPlay, Log,
		TEXT("Stable deployment view: Side=%d Revision=%d GkSide=%d FinishedA=%d FinishedB=%d Complete=%d Wait=%d ExpectedSide=%d"),
		static_cast<int32>(OwnerView.ViewerSide), OwnerView.ViewRevision, static_cast<int32>(OwnerView.GoalkeeperDeployment.Side),
		OwnerView.bPlayerADeploymentFinished, OwnerView.bPlayerBDeploymentFinished, OwnerView.bDeploymentComplete,
		static_cast<int32>(OwnerView.EntryWait), static_cast<int32>(OwnerView.ExpectedActingSide));
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
	SAssignNew(ParticipantChoices, SVerticalBox);
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
					+ SVerticalBox::Slot().AutoHeight().Padding(0, 8, 0, 0)
					[
						SNew(SButton)
						.Visibility_Lambda([this]() { return OwnerView.bCanDeployGoalkeeper ? EVisibility::Visible : EVisibility::Collapsed; })
						.IsEnabled_Lambda([this]() { return CanDeployGoalkeeper(); })
						.Text_Lambda([this]() { return FText::Format(LOCTEXT("DeployGK", "部署门将 {0} → {1}"),
							OwnerView.GoalkeeperOption.CardLabel, OwnerView.GoalkeeperOption.SlotLabel); })
						.OnClicked_Lambda([this]() { DevDeployGoalkeeper(); return FReply::Handled(); })
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0, 8, 0, 0)
					[
						SNew(SButton)
						.Visibility_Lambda([this]() { return OwnerView.bCanFinishDeployment ? EVisibility::Visible : EVisibility::Collapsed; })
						.IsEnabled_Lambda([this]() { return CanFinishDeployment(); })
						.Text(LOCTEXT("FinishDeployment", "完成部署"))
						.OnClicked_Lambda([this]() { DevFinishDeployment(); return FReply::Handled(); })
					]
					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(SBox).MaxDesiredHeight(220.0f)
						[
							SNew(SScrollBox)
							+ SScrollBox::Slot()[ParticipantChoices.ToSharedRef()]
						]
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
	if (ParticipantChoices.IsValid())
	{
		ParticipantChoices->ClearChildren();
		if (OwnerView.InitialRouteAction != EFMCodexNetworkInitialRouteAction::None)
		{
			const FText Label = OwnerView.InitialRouteAction == EFMCodexNetworkInitialRouteAction::Cross
				? LOCTEXT("RollCrossRoute", "掷传中路线骰")
				: OwnerView.InitialRouteAction == EFMCodexNetworkInitialRouteAction::PassControl
					? LOCTEXT("RollPassControlRoute", "掷控球推进路线骰")
					: LOCTEXT("RollThroughBallRoute", "掷直塞路线骰");
			ParticipantChoices->AddSlot().AutoHeight().Padding(0, 8, 0, 0)
			[
				SNew(SButton).Text(Label)
				.IsEnabled_Lambda([this]() { return CanRequestInitialRoute(); })
				.OnClicked_Lambda([this]() { DevRequestInitialRoute(); return FReply::Handled(); })
			];
		}
		for (const auto& Option : OwnerView.CarrierOptions)
		{
			const FName Id = Option.Choice.CarrierCardId;
			ParticipantChoices->AddSlot().AutoHeight().Padding(0, 8, 0, 0)
			[
				SNew(SButton)
				.Text(FText::Format(LOCTEXT("CarrierChoice", "持球：{0}"), Option.CardLabel))
				.IsEnabled_Lambda([this]() { return CanSubmitCarrier(); })
				.OnClicked_Lambda([this, Id]() { DevSubmitCarrier(Id); return FReply::Handled(); })
			];
		}
		for (const auto& Option : OwnerView.MarkerOptions)
		{
			const FName Id = Option.Choice.MarkerCardId;
			ParticipantChoices->AddSlot().AutoHeight().Padding(0, 8, 0, 0)
			[
				SNew(SButton)
				.Text(FText::Format(LOCTEXT("MarkerChoice", "盯人：{0}"), Option.CardLabel))
				.IsEnabled_Lambda([this]() { return CanSubmitMarker(); })
				.OnClicked_Lambda([this, Id]() { DevSubmitMarker(Id); return FReply::Handled(); })
			];
		}
		for (const auto& Option : OwnerView.RunnerOptions)
		{
			const FName Id = Option.Choice.RunnerCardId;
			ParticipantChoices->AddSlot().AutoHeight().Padding(0, 8, 0, 0)
			[
				SNew(SButton)
				.Text(FText::Format(LOCTEXT("RunnerChoice", "跑位：{0}"), Option.CardLabel))
				.IsEnabled_Lambda([this]() { return CanSubmitRunner(); })
				.OnClicked_Lambda([this, Id]() { DevSubmitRunner(Id); return FReply::Handled(); })
			];
		}
		for (const auto& Option : OwnerView.HelperOptions)
		{
			const FName Id = Option.Choice.HelperCardId;
			ParticipantChoices->AddSlot().AutoHeight().Padding(0, 8, 0, 0)
			[
				SNew(SButton)
				.Text(FText::Format(LOCTEXT("HelperChoice", "协防：{0}"), Option.CardLabel))
				.IsEnabled_Lambda([this]() { return CanSubmitHelper(); })
				.OnClicked_Lambda([this, Id]() { DevSubmitHelper(Id); return FReply::Handled(); })
			];
		}
		for (const auto& Option : OwnerView.SkillOptions)
		{
			const FName Id = Option.Choice.SkillId;
			ParticipantChoices->AddSlot().AutoHeight().Padding(0, 8, 0, 0)
			[
				SNew(SButton)
				.Text(FText::Format(LOCTEXT("SkillChoice", "战术：{0}"), Option.SkillLabel))
				.IsEnabled_Lambda([this]() { return CanSubmitSkill(); })
				.OnClicked_Lambda([this, Id]() { DevSubmitSkill(Id); return FReply::Handled(); })
			];
		}
		for (const auto& Option : OwnerView.BranchOptions)
		{
			const auto Choice = Option.Choice;
			ParticipantChoices->AddSlot().AutoHeight().Padding(0, 8, 0, 0)
			[
				SNew(SButton)
				.Text(FText::Format(LOCTEXT("BranchChoice", "分支：{0}"), Option.BranchLabel))
				.IsEnabled_Lambda([this]() { return CanSubmitBranch(); })
				.OnClicked_Lambda([this, Choice]() { SubmitBranchChoice(Choice); return FReply::Handled(); })
			];
		}

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
			? TEXT("等待部署") : OwnerView.EntryWait == EFMCodexNetworkEntryWait::CarrierSelection
			? TEXT("部署已完成，等待选择持球球员") : OwnerView.EntryWait == EFMCodexNetworkEntryWait::MarkerSelection
			? TEXT("等待选择盯人球员") : OwnerView.EntryWait == EFMCodexNetworkEntryWait::RunnerSelection
			? (OwnerView.ExpectedActingSide == OwnerView.ViewerSide
				? TEXT("等待选择跑位球员") : TEXT("等待对手选择跑位球员")) : OwnerView.EntryWait == EFMCodexNetworkEntryWait::HelperSelection
			? (OwnerView.ExpectedActingSide == OwnerView.ViewerSide
				? TEXT("等待选择协防球员") : TEXT("等待对手选择协防球员")) : OwnerView.EntryWait == EFMCodexNetworkEntryWait::SkillSelection
			? (OwnerView.ExpectedActingSide == OwnerView.ViewerSide ? TEXT("等待选择战术") : TEXT("等待对手选择战术")) : OwnerView.EntryWait == EFMCodexNetworkEntryWait::BranchIntentSelection
			? (OwnerView.ExpectedActingSide == OwnerView.ViewerSide ? TEXT("等待选择战术分支") : TEXT("等待对手选择战术分支")) : OwnerView.EntryWait == EFMCodexNetworkEntryWait::PassControlRouteRoll
			? (OwnerView.ExpectedActingSide == OwnerView.ViewerSide ? TEXT("等待掷控球推进路线骰") : TEXT("等待对手掷控球推进路线骰")) : OwnerView.EntryWait == EFMCodexNetworkEntryWait::ThroughBallRouteRoll
			? (OwnerView.ExpectedActingSide == OwnerView.ViewerSide ? TEXT("等待掷直塞路线骰") : TEXT("等待对手掷直塞路线骰")) : OwnerView.EntryWait == EFMCodexNetworkEntryWait::CrossRouteRoll
			? (OwnerView.ExpectedActingSide == OwnerView.ViewerSide ? TEXT("等待掷传中路线骰") : TEXT("等待对手掷传中路线骰")) : OwnerView.EntryWait == EFMCodexNetworkEntryWait::LongShotDirectAttackRoll
			? TEXT("等待进攻方掷远射点数（尚未联网）") : OwnerView.EntryWait == EFMCodexNetworkEntryWait::LongShotDeadCornerRoll
			? TEXT("等待进攻方掷远射双骰（尚未联网）") : OwnerView.EntryWait == EFMCodexNetworkEntryWait::CutInsideDirectAttackRoll
			? TEXT("等待进攻方掷内切射门点数（尚未联网）") : OwnerView.EntryWait == EFMCodexNetworkEntryWait::CutInsideDeadCornerRoll
			? TEXT("等待进攻方掷内切死角双骰（尚未联网）") : OwnerView.EntryWait == EFMCodexNetworkEntryWait::CrossAttackRoll
			? TEXT("等待进攻方掷传中点数（尚未联网）") : OwnerView.EntryWait == EFMCodexNetworkEntryWait::PassControlAttackRoll
			? TEXT("等待进攻方掷控球推进点数（尚未联网）") : OwnerView.EntryWait == EFMCodexNetworkEntryWait::ThroughBallFeetAttackRoll
			? TEXT("等待进攻方掷直塞脚下球点数（尚未联网）") : OwnerView.EntryWait == EFMCodexNetworkEntryWait::ThroughBallBehindDefenseAttackRoll
			? TEXT("等待进攻方掷直塞身后球点数（尚未联网）") : OwnerView.EntryWait == EFMCodexNetworkEntryWait::ThroughBallAntiOffsideAttackRoll
			? TEXT("等待进攻方掷反越位点数（尚未联网）") : OwnerView.EntryWait == EFMCodexNetworkEntryWait::SetPieceTypeRoll
			? TEXT("等待定位球类型掷点") : TEXT("等待服务器");
		EntryText = FString::Printf(TEXT("已公开 Full D12：%d · %s\n%s"),
			OwnerView.DisclosedInitialD12, Branch, Wait);
	}
	if (OwnerView.DeploymentCount > 0)
	{
		const auto& Last = OwnerView.LastDeployment;
		EntryText += FString::Printf(TEXT("\n已部署 %d 张 · 最近：玩家 %s · %s → %s"),
			OwnerView.DeploymentCount, *SideLabel(Last.Side),
			*Last.Placement.CardLabel.ToString(), *Last.Placement.SlotLabel.ToString());
	}
	if (OwnerView.EntryBranch == EFMCodexNetworkEntryBranch::Ordinary)
	{
		EntryText += FString::Printf(TEXT("\n部署状态：A %s · B %s"),
			OwnerView.bPlayerADeploymentFinished ? TEXT("已完成") : TEXT("未完成"),
			OwnerView.bPlayerBDeploymentFinished ? TEXT("已完成") : TEXT("未完成"));
		const auto& GK = OwnerView.GoalkeeperDeployment;
		EntryText += GK.Side == EInitialTurnOrderPlayer::None ? TEXT("\n门将尚未主动部署")
			: FString::Printf(TEXT("\n门将已部署：玩家 %s · %s → %s"),
				*SideLabel(GK.Side), *GK.Placement.CardLabel.ToString(), *GK.Placement.SlotLabel.ToString());
		if (OwnerView.EntryWait == EFMCodexNetworkEntryWait::CarrierSelection
			|| OwnerView.EntryWait == EFMCodexNetworkEntryWait::MarkerSelection
			|| OwnerView.EntryWait == EFMCodexNetworkEntryWait::RunnerSelection
			|| OwnerView.EntryWait == EFMCodexNetworkEntryWait::HelperSelection
			|| OwnerView.EntryWait == EFMCodexNetworkEntryWait::SkillSelection
			|| OwnerView.EntryWait == EFMCodexNetworkEntryWait::BranchIntentSelection
			|| OwnerView.EntryWait == EFMCodexNetworkEntryWait::PassControlRouteRoll
			|| OwnerView.EntryWait == EFMCodexNetworkEntryWait::ThroughBallRouteRoll
			|| OwnerView.EntryWait == EFMCodexNetworkEntryWait::CrossRouteRoll
			|| OwnerView.EntryWait == EFMCodexNetworkEntryWait::LongShotDirectAttackRoll
			|| OwnerView.EntryWait == EFMCodexNetworkEntryWait::LongShotDeadCornerRoll
			|| OwnerView.EntryWait == EFMCodexNetworkEntryWait::CutInsideDirectAttackRoll
			|| OwnerView.EntryWait == EFMCodexNetworkEntryWait::CutInsideDeadCornerRoll
			|| OwnerView.EntryWait == EFMCodexNetworkEntryWait::CrossAttackRoll
			|| OwnerView.EntryWait == EFMCodexNetworkEntryWait::PassControlAttackRoll
			|| OwnerView.EntryWait == EFMCodexNetworkEntryWait::ThroughBallFeetAttackRoll
			|| OwnerView.EntryWait == EFMCodexNetworkEntryWait::ThroughBallBehindDefenseAttackRoll
			|| OwnerView.EntryWait == EFMCodexNetworkEntryWait::ThroughBallAntiOffsideAttackRoll)
		{
			EntryText += FString::Printf(TEXT("\n下一操作方：玩家 %s"), *SideLabel(OwnerView.ExpectedActingSide));
		}
	}
	if (!OwnerView.SelectedCarrier.Choice.IsEmpty())
	{
		EntryText += FText::Format(LOCTEXT("SelectedCarrier", "\n已选持球球员：{0}"), OwnerView.SelectedCarrier.CardLabel).ToString();
	}
	if (OwnerView.bCarrierOptionsUnavailable)
	{
		EntryText += LOCTEXT("CarrierProjectionUnavailable", "\n持球候选视图不可用，请检查服务器配置").ToString();
	}
	if (!OwnerView.SelectedMarker.Choice.IsEmpty())
	{
		EntryText += FText::Format(LOCTEXT("SelectedMarker", "\n已选盯人球员：{0}"), OwnerView.SelectedMarker.CardLabel).ToString();
	}
	if (OwnerView.bMarkerOptionsUnavailable)
	{
		EntryText += LOCTEXT("MarkerProjectionUnavailable", "\n盯人候选视图不可用，请检查服务器配置").ToString();
	}
	if (!OwnerView.SelectedRunner.Choice.IsEmpty())
	{
		EntryText += FText::Format(LOCTEXT("SelectedRunner", "\n已选跑位球员：{0}"), OwnerView.SelectedRunner.CardLabel).ToString();
	}
	if (OwnerView.bRunnerOptionsUnavailable)
	{
		EntryText += LOCTEXT("RunnerProjectionUnavailable", "\n跑位候选视图不可用，请检查服务器配置").ToString();
	}
	if (!OwnerView.SelectedHelper.Choice.IsEmpty())
	{
		EntryText += FText::Format(LOCTEXT("SelectedHelper", "\n已选协防球员：{0}"), OwnerView.SelectedHelper.CardLabel).ToString();
	}
	if (OwnerView.bHelperOptionsUnavailable)
	{
		EntryText += LOCTEXT("HelperProjectionUnavailable", "\n协防候选视图不可用，请检查服务器配置").ToString();
	}
	if (!OwnerView.SelectedSkill.Choice.IsEmpty())
	{
		EntryText += FText::Format(LOCTEXT("SelectedSkill", "\n已选战术：{0}"), OwnerView.SelectedSkill.SkillLabel).ToString();
	}
	if (OwnerView.bSkillOptionsUnavailable)
	{
		EntryText += LOCTEXT("SkillProjectionUnavailable", "\n战术候选视图不可用，请检查服务器配置").ToString();
	}
	if (OwnerView.InitialRoute.D6 != 0)
	{
		EntryText += FText::Format(LOCTEXT("InitialRouteFact", "\n路线骰：{0} · 实际路线：{1}"),
			FText::AsNumber(OwnerView.InitialRoute.D6), OwnerView.InitialRoute.RouteLabel).ToString();
	}
	if (!OwnerView.SelectedBranch.Choice.IsEmpty())
	{
		EntryText += FText::Format(LOCTEXT("SelectedBranch", "\n已选分支：{0}"), OwnerView.SelectedBranch.BranchLabel).ToString();
	}
	if (OwnerView.bBranchOptionsUnavailable)
	{
		EntryText += LOCTEXT("BranchProjectionUnavailable", "\n分支候选视图不可用，请检查服务器配置").ToString();
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
bool AFMCodexNetworkMatchPlayerController::CanSubmitCarrier() const
{
	return IsLocalController() && !IntentClientState.IsPending() && OwnerView.bMatchInitialized
		&& OwnerView.BootstrapState == EFMCodexNetworkBootstrapState::MatchReady
		&& OwnerView.EntryWait == EFMCodexNetworkEntryWait::CarrierSelection
		&& OwnerView.ExpectedActingSide == OwnerView.ViewerSide
		&& !OwnerView.bCarrierOptionsUnavailable && !OwnerView.CarrierOptions.IsEmpty();
}
void AFMCodexNetworkMatchPlayerController::DevSubmitCarrier(FName CarrierCardId)
{
#if !UE_BUILD_SHIPPING
	if (!CanSubmitCarrier()) { return; }
	const auto* Option = OwnerView.CarrierOptions.FindByPredicate(
		[&](const auto& C) { return C.Choice.CarrierCardId == CarrierCardId; });
	if (Option) { SubmitCarrierChoice(Option->Choice); }
#endif
}
void AFMCodexNetworkMatchPlayerController::SubmitCarrierChoice(const FFMCodexNetworkSubmitCarrierPayload& Choice)
{
#if !UE_BUILD_SHIPPING
	if (!IsLocalController()) { return; }
	FFMCodexNetworkPlayerIntentEnvelope Envelope;
	if (!IntentClientState.BeginCarrier(OwnerView, Choice, Envelope)) { return; }
	RefreshNetworkBootstrapUI();
	UE_LOG(LogFMCodexNetworkPlay, Log,
		TEXT("Carrier owner submit: Match=%s Request=%lld ViewerSide=%d ExpectedSequence=%lld Carrier=%s"),
		*Envelope.MatchInstanceId.ToString(EGuidFormats::DigitsWithHyphensLower), Envelope.RequestId,
		static_cast<int32>(OwnerView.ViewerSide), Envelope.ExpectedAttackSequence, *Choice.CarrierCardId.ToString());
	ServerSubmitPlayerIntent(Envelope); // Generated owning RPC for both Host and Remote.
#endif
}
void AFMCodexNetworkMatchPlayerController::DevProbeInvalidCarrier()
{
#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
	if (!CanSubmitCarrier()) { return; }
	FFMCodexNetworkSubmitCarrierPayload Choice;
	Choice.CarrierCardId = TEXT("DEV.NonexistentCarrier");
	SubmitCarrierChoice(Choice);
#endif
}
bool AFMCodexNetworkMatchPlayerController::CanSubmitMarker() const
{
	return IsLocalController() && !IntentClientState.IsPending() && OwnerView.bMatchInitialized
		&& OwnerView.BootstrapState == EFMCodexNetworkBootstrapState::MatchReady
		&& OwnerView.EntryWait == EFMCodexNetworkEntryWait::MarkerSelection
		&& OwnerView.ExpectedActingSide == OwnerView.ViewerSide
		&& !OwnerView.bMarkerOptionsUnavailable && !OwnerView.MarkerOptions.IsEmpty();
}
void AFMCodexNetworkMatchPlayerController::DevSubmitMarker(FName MarkerCardId)
{
#if !UE_BUILD_SHIPPING
	if (!CanSubmitMarker()) { return; }
	const auto* Option = OwnerView.MarkerOptions.FindByPredicate(
		[&](const auto& C) { return C.Choice.MarkerCardId == MarkerCardId; });
	if (Option) { SubmitMarkerChoice(Option->Choice); }
#endif
}
void AFMCodexNetworkMatchPlayerController::SubmitMarkerChoice(const FFMCodexNetworkSubmitMarkerPayload& Choice)
{
#if !UE_BUILD_SHIPPING
	if (!IsLocalController()) { return; }
	FFMCodexNetworkPlayerIntentEnvelope Envelope;
	if (!IntentClientState.BeginMarker(OwnerView, Choice, Envelope)) { return; }
	RefreshNetworkBootstrapUI();
	UE_LOG(LogFMCodexNetworkPlay, Log,
		TEXT("Marker owner submit: Match=%s Request=%lld ViewerSide=%d ExpectedSequence=%lld Marker=%s"),
		*Envelope.MatchInstanceId.ToString(EGuidFormats::DigitsWithHyphensLower), Envelope.RequestId,
		static_cast<int32>(OwnerView.ViewerSide), Envelope.ExpectedAttackSequence, *Choice.MarkerCardId.ToString());
	ServerSubmitPlayerIntent(Envelope); // Generated owning RPC for both Host and Remote.
#endif
}
void AFMCodexNetworkMatchPlayerController::DevProbeInvalidMarker()
{
#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
	if (!CanSubmitMarker()) { return; }
	FFMCodexNetworkSubmitMarkerPayload Choice;
	Choice.MarkerCardId = TEXT("DEV.NonexistentMarker");
	SubmitMarkerChoice(Choice);
#endif
}
bool AFMCodexNetworkMatchPlayerController::CanSubmitRunner() const
{
	return IsLocalController() && !IntentClientState.IsPending() && OwnerView.bMatchInitialized
		&& OwnerView.BootstrapState == EFMCodexNetworkBootstrapState::MatchReady
		&& OwnerView.EntryWait == EFMCodexNetworkEntryWait::RunnerSelection
		&& OwnerView.ExpectedActingSide == OwnerView.ViewerSide
		&& !OwnerView.bRunnerOptionsUnavailable && !OwnerView.RunnerOptions.IsEmpty();
}
void AFMCodexNetworkMatchPlayerController::DevSubmitRunner(FName RunnerCardId)
{
#if !UE_BUILD_SHIPPING
	if (!CanSubmitRunner()) { return; }
	const auto* Option = OwnerView.RunnerOptions.FindByPredicate(
		[&](const auto& C) { return C.Choice.RunnerCardId == RunnerCardId; });
	if (Option) { SubmitRunnerChoice(Option->Choice); }
#endif
}
void AFMCodexNetworkMatchPlayerController::SubmitRunnerChoice(const FFMCodexNetworkSubmitRunnerPayload& Choice)
{
#if !UE_BUILD_SHIPPING
	if (!IsLocalController()) { return; }
	FFMCodexNetworkPlayerIntentEnvelope Envelope;
	if (!IntentClientState.BeginRunner(OwnerView, Choice, Envelope)) { return; }
	RefreshNetworkBootstrapUI();
	UE_LOG(LogFMCodexNetworkPlay, Log,
		TEXT("Runner owner submit: Match=%s Request=%lld ViewerSide=%d ExpectedSequence=%lld Runner=%s"),
		*Envelope.MatchInstanceId.ToString(EGuidFormats::DigitsWithHyphensLower), Envelope.RequestId,
		static_cast<int32>(OwnerView.ViewerSide), Envelope.ExpectedAttackSequence, *Choice.RunnerCardId.ToString());
	ServerSubmitPlayerIntent(Envelope); // Generated owning RPC for both Host and Remote.
#endif
}
void AFMCodexNetworkMatchPlayerController::DevProbeInvalidRunner()
{
#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
	if (!CanSubmitRunner()) { return; }
	FFMCodexNetworkSubmitRunnerPayload Choice;
	Choice.RunnerCardId = TEXT("DEV.NonexistentRunner");
	SubmitRunnerChoice(Choice);
#endif
}
bool AFMCodexNetworkMatchPlayerController::CanSubmitHelper() const
{
	return IsLocalController() && !IntentClientState.IsPending() && OwnerView.bMatchInitialized
		&& OwnerView.BootstrapState == EFMCodexNetworkBootstrapState::MatchReady
		&& OwnerView.EntryWait == EFMCodexNetworkEntryWait::HelperSelection
		&& OwnerView.ExpectedActingSide == OwnerView.ViewerSide
		&& !OwnerView.bHelperOptionsUnavailable && !OwnerView.HelperOptions.IsEmpty();
}
void AFMCodexNetworkMatchPlayerController::DevSubmitHelper(FName HelperCardId)
{
#if !UE_BUILD_SHIPPING
	if (!CanSubmitHelper()) { return; }
	const auto* Option = OwnerView.HelperOptions.FindByPredicate(
		[&](const auto& C) { return C.Choice.HelperCardId == HelperCardId; });
	if (Option) { SubmitHelperChoice(Option->Choice); }
#endif
}
void AFMCodexNetworkMatchPlayerController::SubmitHelperChoice(const FFMCodexNetworkSubmitHelperPayload& Choice)
{
#if !UE_BUILD_SHIPPING
	if (!IsLocalController()) { return; }
	FFMCodexNetworkPlayerIntentEnvelope Envelope;
	if (!IntentClientState.BeginHelper(OwnerView, Choice, Envelope)) { return; }
	RefreshNetworkBootstrapUI();
	UE_LOG(LogFMCodexNetworkPlay, Log,
		TEXT("Helper owner submit: Match=%s Request=%lld ViewerSide=%d ExpectedSequence=%lld Helper=%s"),
		*Envelope.MatchInstanceId.ToString(EGuidFormats::DigitsWithHyphensLower), Envelope.RequestId,
		static_cast<int32>(OwnerView.ViewerSide), Envelope.ExpectedAttackSequence, *Choice.HelperCardId.ToString());
	ServerSubmitPlayerIntent(Envelope); // Generated owning RPC for both Host and Remote.
#endif
}
void AFMCodexNetworkMatchPlayerController::DevProbeInvalidHelper()
{
#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
	if (!CanSubmitHelper()) { return; }
	FFMCodexNetworkSubmitHelperPayload Choice;
	Choice.HelperCardId = TEXT("DEV.NonexistentHelper");
	SubmitHelperChoice(Choice);
#endif
}
bool AFMCodexNetworkMatchPlayerController::CanSubmitSkill() const
{
	return IsLocalController() && !IntentClientState.IsPending() && OwnerView.bMatchInitialized
		&& OwnerView.BootstrapState == EFMCodexNetworkBootstrapState::MatchReady
		&& OwnerView.EntryWait == EFMCodexNetworkEntryWait::SkillSelection
		&& OwnerView.ExpectedActingSide == OwnerView.ViewerSide
		&& !OwnerView.bSkillOptionsUnavailable && !OwnerView.SkillOptions.IsEmpty();
}
void AFMCodexNetworkMatchPlayerController::DevSubmitSkill(FName SkillId)
{
#if !UE_BUILD_SHIPPING
	if (!CanSubmitSkill()) { return; }
	const auto* Option = OwnerView.SkillOptions.FindByPredicate(
		[&](const auto& C) { return C.Choice.SkillId == SkillId; });
	if (Option) { SubmitSkillChoice(Option->Choice); }
#endif
}
void AFMCodexNetworkMatchPlayerController::SubmitSkillChoice(const FFMCodexNetworkSubmitSkillPayload& Choice)
{
#if !UE_BUILD_SHIPPING
	if (!IsLocalController()) { return; }
	FFMCodexNetworkPlayerIntentEnvelope Envelope;
	if (!IntentClientState.BeginSkill(OwnerView, Choice, Envelope)) { return; }
	RefreshNetworkBootstrapUI();
	UE_LOG(LogFMCodexNetworkPlay, Log,
		TEXT("Skill owner submit: Match=%s Request=%lld ViewerSide=%d ExpectedSequence=%lld Skill=%s"),
		*Envelope.MatchInstanceId.ToString(EGuidFormats::DigitsWithHyphensLower), Envelope.RequestId,
		static_cast<int32>(OwnerView.ViewerSide), Envelope.ExpectedAttackSequence, *Choice.SkillId.ToString());
	ServerSubmitPlayerIntent(Envelope); // Generated owning RPC for both Host and Remote.
#endif
}
void AFMCodexNetworkMatchPlayerController::DevProbeInvalidSkill()
{
#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
	if (!CanSubmitSkill()) { return; }
	FFMCodexNetworkSubmitSkillPayload Choice;
	Choice.SkillId = TEXT("DEV.NonexistentSkill");
	SubmitSkillChoice(Choice);
#endif
}
bool AFMCodexNetworkMatchPlayerController::CanSubmitBranch() const
{
	return IsLocalController() && !IntentClientState.IsPending() && OwnerView.bMatchInitialized
		&& OwnerView.BootstrapState == EFMCodexNetworkBootstrapState::MatchReady
		&& OwnerView.EntryWait == EFMCodexNetworkEntryWait::BranchIntentSelection
		&& OwnerView.ExpectedActingSide == OwnerView.ViewerSide
		&& !OwnerView.bBranchOptionsUnavailable && !OwnerView.BranchOptions.IsEmpty();
}
void AFMCodexNetworkMatchPlayerController::DevSubmitBranchIntent(int32 Intent)
{
#if !UE_BUILD_SHIPPING
	if (!CanSubmitBranch()) { return; }
	const auto* Option = OwnerView.BranchOptions.FindByPredicate(
		[&](const auto& O) { return static_cast<int32>(O.Choice.Intent) == Intent; });
	if (Option) { SubmitBranchChoice(Option->Choice); }
#endif
}
void AFMCodexNetworkMatchPlayerController::SubmitBranchChoice(const FFMCodexNetworkSubmitBranchIntentPayload& Choice)
{
#if !UE_BUILD_SHIPPING
	if (!IsLocalController()) { return; }
	FFMCodexNetworkPlayerIntentEnvelope Envelope;
	if (!IntentClientState.BeginBranch(OwnerView, Choice, Envelope)) { return; }
	RefreshNetworkBootstrapUI();
	UE_LOG(LogFMCodexNetworkPlay, Log,
		TEXT("Branch owner submit: Match=%s Request=%lld ViewerSide=%d ExpectedSequence=%lld Branch=%d"),
		*Envelope.MatchInstanceId.ToString(EGuidFormats::DigitsWithHyphensLower), Envelope.RequestId,
		static_cast<int32>(OwnerView.ViewerSide), Envelope.ExpectedAttackSequence, static_cast<int32>(Choice.Intent));
	ServerSubmitPlayerIntent(Envelope); // Same generated owning RPC for Host and Remote.
#endif
}
void AFMCodexNetworkMatchPlayerController::DevProbeInvalidBranch()
{
#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
	if (!CanSubmitBranch()) { return; }
	FFMCodexNetworkSubmitBranchIntentPayload Choice; Choice.Intent = EMatchPlayElectiveBranchIntent::CrossHigh;
	SubmitBranchChoice(Choice);
#endif
}
bool AFMCodexNetworkMatchPlayerController::CanDeployGoalkeeper() const
{
	return IsLocalController() && !IntentClientState.IsPending() && OwnerView.bMatchInitialized
		&& OwnerView.BootstrapState == EFMCodexNetworkBootstrapState::MatchReady && OwnerView.bCanDeployGoalkeeper;
}
bool AFMCodexNetworkMatchPlayerController::CanFinishDeployment() const
{
	return IsLocalController() && !IntentClientState.IsPending() && OwnerView.bMatchInitialized
		&& OwnerView.BootstrapState == EFMCodexNetworkBootstrapState::MatchReady && OwnerView.bCanFinishDeployment;
}
void AFMCodexNetworkMatchPlayerController::DevDeployGoalkeeper()
{
#if !UE_BUILD_SHIPPING
	if (CanDeployGoalkeeper()) { SubmitDeploymentCompletion(EFMCodexNetworkPlayerIntentKind::DeployGoalkeeper); }
#endif
}
void AFMCodexNetworkMatchPlayerController::DevFinishDeployment()
{
#if !UE_BUILD_SHIPPING
	if (CanFinishDeployment()) { SubmitDeploymentCompletion(EFMCodexNetworkPlayerIntentKind::FinishDeployment); }
#endif
}
void AFMCodexNetworkMatchPlayerController::SubmitDeploymentCompletion(EFMCodexNetworkPlayerIntentKind Kind)
{
#if !UE_BUILD_SHIPPING
	if (!IsLocalController()) { return; }
	FFMCodexNetworkPlayerIntentEnvelope Envelope;
	const bool bBegan = Kind == EFMCodexNetworkPlayerIntentKind::DeployGoalkeeper
		? IntentClientState.BeginGoalkeeper(OwnerView, OwnerView.GoalkeeperOption.Choice, Envelope)
		: Kind == EFMCodexNetworkPlayerIntentKind::FinishDeployment
			&& IntentClientState.BeginFinishDeployment(OwnerView, Envelope);
	if (!bBegan) { return; }
	RefreshNetworkBootstrapUI();
	UE_LOG(LogFMCodexNetworkPlay, Log,
		TEXT("Deployment completion owner submit: Match=%s Request=%lld ViewerSide=%d ExpectedSequence=%lld Kind=%d GKSlot=%s"),
		*Envelope.MatchInstanceId.ToString(EGuidFormats::DigitsWithHyphensLower), Envelope.RequestId,
		static_cast<int32>(OwnerView.ViewerSide), Envelope.ExpectedAttackSequence, static_cast<int32>(Kind),
		*Envelope.Goalkeeper.SlotId.ToString());
	// Both new intents use the same generated owning RPC on Remote and Listen Host.
	ServerSubmitPlayerIntent(Envelope);
#endif
}
void AFMCodexNetworkMatchPlayerController::DevProbeInvalidGoalkeeperSlot()
{
#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
	if (!CanDeployGoalkeeper()) { return; }
	auto Choice = OwnerView.GoalkeeperOption.Choice;
	Choice.SlotId = TEXT("DEV.NonexistentGoalkeeperSlot");
	FFMCodexNetworkPlayerIntentEnvelope Envelope;
	if (!IntentClientState.BeginGoalkeeper(OwnerView, Choice, Envelope)) { return; }
	RefreshNetworkBootstrapUI();
	UE_LOG(LogFMCodexNetworkPlay, Log, TEXT("DEV negative goalkeeper slot: Request=%lld GKSlot=%s"),
		Envelope.RequestId, *Envelope.Goalkeeper.SlotId.ToString());
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

bool AFMCodexNetworkMatchPlayerController::CanRequestInitialRoute() const
{
	return IsLocalController() && !IntentClientState.IsPending() && OwnerView.bMatchInitialized
		&& OwnerView.BootstrapState == EFMCodexNetworkBootstrapState::MatchReady
		&& OwnerView.InitialRouteAction != EFMCodexNetworkInitialRouteAction::None
		&& OwnerView.ViewerSide != EInitialTurnOrderPlayer::None
		&& OwnerView.ExpectedActingSide == OwnerView.ViewerSide;
}
void AFMCodexNetworkMatchPlayerController::DevRequestInitialRoute()
{
#if !UE_BUILD_SHIPPING
	if (!CanRequestInitialRoute()) { return; }
	switch (OwnerView.InitialRouteAction)
	{
	case EFMCodexNetworkInitialRouteAction::Cross: SubmitInitialRoute(EFMCodexNetworkPlayerIntentKind::CrossInitialRouteRoll); break;
	case EFMCodexNetworkInitialRouteAction::PassControl: SubmitInitialRoute(EFMCodexNetworkPlayerIntentKind::PassControlInitialRouteRoll); break;
	case EFMCodexNetworkInitialRouteAction::ThroughBall: SubmitInitialRoute(EFMCodexNetworkPlayerIntentKind::ThroughBallInitialRouteRoll); break;
	default: break;
	}
#endif
}
void AFMCodexNetworkMatchPlayerController::SubmitInitialRoute(EFMCodexNetworkPlayerIntentKind Kind)
{
#if !UE_BUILD_SHIPPING
	if (!IsLocalController()) { return; }
	FFMCodexNetworkPlayerIntentEnvelope Envelope;
	if (!IntentClientState.BeginInitialRoute(OwnerView, Kind, Envelope)) { return; }
	RefreshNetworkBootstrapUI();
	UE_LOG(LogFMCodexNetworkPlay, Log, TEXT("InitialRoute owner submit: Match=%s Request=%lld ViewerSide=%d ExpectedSequence=%lld Kind=%d"),
		*Envelope.MatchInstanceId.ToString(EGuidFormats::DigitsWithHyphensLower), Envelope.RequestId,
		static_cast<int32>(OwnerView.ViewerSide), Envelope.ExpectedAttackSequence, static_cast<int32>(Kind));
	ServerSubmitPlayerIntent(Envelope);
#endif
}
void AFMCodexNetworkMatchPlayerController::DevProbeWrongRouteFamily()
{
#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
	if (!CanRequestInitialRoute()) { return; }
	const auto Offered = OwnerView.InitialRouteAction;
	FFMCodexNetworkPlayerIntentEnvelope Envelope;
	const auto Kind = Offered == EFMCodexNetworkInitialRouteAction::Cross
		? EFMCodexNetworkPlayerIntentKind::CrossInitialRouteRoll
		: Offered == EFMCodexNetworkInitialRouteAction::PassControl
			? EFMCodexNetworkPlayerIntentKind::PassControlInitialRouteRoll
			: EFMCodexNetworkPlayerIntentKind::ThroughBallInitialRouteRoll;
	if (!IntentClientState.BeginInitialRoute(OwnerView, Kind, Envelope)) { return; }
	Envelope.IntentKind = Kind == EFMCodexNetworkPlayerIntentKind::CrossInitialRouteRoll
		? EFMCodexNetworkPlayerIntentKind::PassControlInitialRouteRoll : EFMCodexNetworkPlayerIntentKind::CrossInitialRouteRoll;
	RefreshNetworkBootstrapUI();
	ServerSubmitPlayerIntent(Envelope);
#endif
}

#undef LOCTEXT_NAMESPACE
