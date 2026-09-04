#include "FMCodexLocalMatchPlayerController.h"

#include "FMCodexLocalMatchDemoConfiguration.h"
#include "FMCodexLocalMatchHostGameMode.h"
#include "FMCodexLocalMatchScreenWidget.h"
#include "FMCodexLocalMatchUMGPresentation.h"
#if !UE_BUILD_SHIPPING
#include "FMCodexLocalDevRollOverrideWidget.h"
#endif

#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "EngineUtils.h"
#endif
#include "Engine/World.h"
#include "Framework/Application/SlateApplication.h"
#include "GameFramework/GameModeBase.h"
#include "Styling/CoreStyle.h"
#include "TimerManager.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SExpandableArea.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Layout/SWrapBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

namespace FMCodexLocalMatchPlayerController
{
	FString IntentLabel(const EMatchPlayElectiveBranchIntent Intent)
	{
		switch (Intent)
		{
		case EMatchPlayElectiveBranchIntent::DirectShot: return TEXT("Direct Shot");
		case EMatchPlayElectiveBranchIntent::DeadCorner: return TEXT("Dead Corner");
		case EMatchPlayElectiveBranchIntent::CrossHigh: return TEXT("Cross High");
		case EMatchPlayElectiveBranchIntent::CrossLow: return TEXT("Cross Low");
		default: return TEXT("Unknown Intent");
		}
	}

	FString ShotChoiceLabel(
		const EMatchPlayThroughBallOneOnOneShotChoice Choice)
	{
		return Choice == EMatchPlayThroughBallOneOnOneShotChoice::ChipShot
			? TEXT("Chip Shot")
			: TEXT("Direct Shot");
	}

	TSharedRef<SWidget> MakeButton(
		const FString& Label,
		TFunction<void()> Action)
	{
		return SNew(SButton)
			.ContentPadding(FMargin(8.0f, 4.0f))
			.OnClicked_Lambda([Action = MoveTemp(Action)]()
			{
				Action();
				return FReply::Handled();
			})
			[
				SNew(STextBlock).Text(FText::FromString(Label))
			];
	}

	TSharedRef<SWidget> MakeCardPanel(
		const FFMCodexLocalMatchCardView& Card,
		const bool bShowLocation = true)
	{
		TSharedRef<SVerticalBox> Body = SNew(SVerticalBox);
		const FLinearColor SideColor =
			Card.Side == EInitialTurnOrderPlayer::PlayerA
				? FLinearColor(0.12f, 0.30f, 0.55f, 0.95f)
				: FLinearColor(0.50f, 0.16f, 0.14f, 0.95f);
		Body->AddSlot().AutoHeight()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(1.0f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(Card.DisplayLabel))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))
				.ColorAndOpacity(FLinearColor(1.0f, 0.90f, 0.52f))
				.AutoWrapText(true)
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(8.0f, 0.0f)
			[
				SNew(SBorder)
				.Padding(FMargin(8.0f, 3.0f))
				.BorderBackgroundColor(FLinearColor(0.04f, 0.07f, 0.10f, 0.72f))
				[
					SNew(STextBlock)
					.Text(FText::FromString(Card.CompactRoleLabel))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
				]
			]
		];
		Body->AddSlot().AutoHeight().Padding(0.0f, 3.0f, 0.0f, 7.0f)
		[
			SNew(STextBlock)
			.Text(FText::FromString(FString::Printf(
				TEXT("%s  |  %s"),
				*FFMCodexLocalMatchInteractionViewBuilder::ToString(Card.Side),
				Card.PositionLabel.IsEmpty()
					? TEXT("Role unavailable") : *Card.PositionLabel)))
			.ColorAndOpacity(FLinearColor(0.78f, 0.84f, 0.90f))
		];
		Body->AddSlot().AutoHeight().Padding(0.0f, 2.0f, 0.0f, 7.0f)
		[
			SNew(SBorder)
			.Padding(7.0f)
			.BorderBackgroundColor(FLinearColor(0.05f, 0.16f, 0.10f, 0.90f))
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("SKILL")))
					.ColorAndOpacity(FLinearColor(0.62f, 0.78f, 0.66f))
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 2.0f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(Card.SkillSummaryLabel))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
					.ColorAndOpacity(FLinearColor(0.82f, 1.0f, 0.82f))
					.AutoWrapText(true)
				]
			]
		];
		Body->AddSlot().AutoHeight().Padding(0.0f, 2.0f)
		[
			SNew(STextBlock)
			.Text(FText::FromString(Card.bGoalkeeper
				? TEXT("GOALKEEPER ATTRIBUTES") : TEXT("OUTFIELD ATTRIBUTES")))
			.ColorAndOpacity(FLinearColor(0.62f, 0.68f, 0.74f))
		];
		Body->AddSlot().AutoHeight().Padding(0.0f, 2.0f, 0.0f, 7.0f)
		[
			SNew(STextBlock)
			.Text(FText::FromString(Card.bGoalkeeper
				&& !Card.GoalkeeperAttributeSummary.IsEmpty()
					? Card.GoalkeeperAttributeSummary
					: Card.AttributeSummary.IsEmpty()
						? TEXT("Attributes unavailable") : Card.AttributeSummary))
			.AutoWrapText(true)
		];
		Body->AddSlot().AutoHeight().Padding(0.0f, 2.0f, 0.0f, 5.0f)
		[
			SNew(SBorder)
			.Padding(FMargin(7.0f, 4.0f))
			.BorderBackgroundColor(FLinearColor(0.05f, 0.08f, 0.11f, 0.82f))
			[
				SNew(STextBlock)
				.Text(FText::FromString(
					TEXT("STATUS  |  ") + Card.StatusSummaryLabel))
				.ColorAndOpacity(Card.bGoalkeeperActivatedThisAttack
					? FLinearColor(1.0f, 0.80f, 0.28f)
					: FLinearColor(0.72f, 0.80f, 0.88f))
				.AutoWrapText(true)
			]
		];
		if (bShowLocation && Card.bDeployed)
		{
			Body->AddSlot().AutoHeight().Padding(0.0f, 3.0f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(FString::Printf(
					TEXT("Location: %s | %s | Slot %s"),
					*FFMCodexLocalMatchInteractionViewBuilder::ToString(
						Card.NeutralSide),
					*FFMCodexLocalMatchInteractionViewBuilder::ToString(
						Card.RelativeZone),
					*Card.SlotId.ToString())))
				.ColorAndOpacity(FLinearColor(0.58f, 0.66f, 0.72f))
				.AutoWrapText(true)
			];
		}
		Body->AddSlot().AutoHeight().Padding(0.0f, 4.0f, 0.0f, 0.0f)
		[
			SNew(STextBlock)
			.Text(FText::FromString(Card.DeveloperReferenceLabel))
			.ColorAndOpacity(FLinearColor(0.44f, 0.50f, 0.56f))
			.AutoWrapText(true)
		];

		return SNew(SBox)
			.MaxDesiredWidth(520.0f)
			[
				SNew(SBorder)
				.Padding(10.0f)
				.BorderBackgroundColor(SideColor)
				[
					Body
				]
			];
	}

	TSharedRef<SWidget> MakeCompactPitchCard(
		const FFMCodexLocalMatchCardView& Card)
	{
		TSharedRef<SVerticalBox> Body = SNew(SVerticalBox);
		Body->AddSlot().AutoHeight()
		[
			SNew(STextBlock)
			.Text(FText::FromString(Card.DisplayLabel))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
			.ColorAndOpacity(FLinearColor(1.0f, 0.90f, 0.52f))
			.AutoWrapText(true)
		];
		Body->AddSlot().AutoHeight().Padding(0.0f, 2.0f, 0.0f, 5.0f)
		[
			SNew(STextBlock)
			.Text(FText::FromString(FString::Printf(
				TEXT("%s  |  %s"),
				*FFMCodexLocalMatchInteractionViewBuilder::ToString(Card.Side),
				*Card.CompactRoleLabel)))
			.ColorAndOpacity(FLinearColor(0.80f, 0.86f, 0.92f))
		];
		Body->AddSlot().AutoHeight().Padding(0.0f, 2.0f, 0.0f, 5.0f)
		[
			SNew(SBorder)
			.Padding(5.0f)
			.BorderBackgroundColor(FLinearColor(0.04f, 0.13f, 0.08f, 0.92f))
			[
				SNew(STextBlock)
				.Text(FText::FromString(Card.SkillSummaryLabel))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
				.ColorAndOpacity(FLinearColor(0.80f, 1.0f, 0.80f))
				.AutoWrapText(true)
			]
		];
		Body->AddSlot().AutoHeight().Padding(0.0f, 1.0f, 0.0f, 5.0f)
		[
			SNew(STextBlock)
			.Text(FText::FromString(Card.CompactAttributeSummary.IsEmpty()
				? TEXT("Attributes unavailable") : Card.CompactAttributeSummary))
			.ColorAndOpacity(FLinearColor(0.76f, 0.82f, 0.88f))
			.AutoWrapText(true)
		];
		Body->AddSlot().AutoHeight()
		[
			SNew(STextBlock)
			.Text(FText::FromString(Card.StatusSummaryLabel))
			.ColorAndOpacity(Card.bGoalkeeperActivatedThisAttack
				? FLinearColor(1.0f, 0.78f, 0.28f)
				: FLinearColor(0.58f, 0.68f, 0.76f))
			.AutoWrapText(true)
		];

		return SNew(SBorder)
			.Padding(6.0f)
			.BorderBackgroundColor(
				Card.Side == EInitialTurnOrderPlayer::PlayerA
					? FLinearColor(0.10f, 0.25f, 0.45f, 1.0f)
					: FLinearColor(0.42f, 0.13f, 0.12f, 1.0f))
			[
				Body
			];
	}

	TSharedRef<SWidget> MakePitchSlot(
		const FFMCodexLocalMatchPitchSlotView& Slot)
	{
		TSharedRef<SVerticalBox> Body = SNew(SVerticalBox);
		Body->AddSlot().AutoHeight()
		[
			SNew(STextBlock)
			.Text(FText::FromString(Slot.Label))
			.ColorAndOpacity(FLinearColor(0.72f, 0.82f, 0.72f))
			.AutoWrapText(true)
		];
		Body->AddSlot().AutoHeight().Padding(0.0f, 2.0f, 0.0f, 5.0f)
		[
			SNew(STextBlock)
			.Text(FText::FromString(FString::Printf(
				TEXT("A %s  |  B %s"),
				*FFMCodexLocalMatchInteractionViewBuilder::ToString(
					Slot.PlayerARelativeZone),
				*FFMCodexLocalMatchInteractionViewBuilder::ToString(
					Slot.PlayerBRelativeZone))))
			.ColorAndOpacity(FLinearColor(0.50f, 0.60f, 0.54f))
		];
		if (Slot.bOccupied)
		{
			Body->AddSlot().AutoHeight()
			[
				MakeCompactPitchCard(Slot.Card)
			];
		}
		else
		{
			Body->AddSlot().AutoHeight().Padding(5.0f, 12.0f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT(
					"EMPTY SLOT\nLegality is shown in player actions")))
				.Justification(ETextJustify::Center)
				.ColorAndOpacity(FLinearColor(0.42f, 0.52f, 0.45f))
				.AutoWrapText(true)
			];
		}

		return SNew(SBox)
			.WidthOverride(218.0f)
			[
				SNew(SBorder)
				.Padding(7.0f)
				.BorderBackgroundColor(FLinearColor(0.035f, 0.13f, 0.06f, 1.0f))
				[
					Body
				]
			];
	}

	TSharedRef<SWidget> MakeFeedbackPanel(
		const FFMCodexLocalMatchResolutionFeedback& Feedback)
	{
		TSharedRef<SVerticalBox> Body = SNew(SVerticalBox);
		auto AddLine = [&Body](
			const FString& Text,
			const FLinearColor Color = FLinearColor::White)
		{
			if (!Text.IsEmpty())
			{
				Body->AddSlot().AutoHeight().Padding(2.0f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(Text))
					.ColorAndOpacity(Color)
				];
			}
		};

		const bool bResolutionOperation =
			Feedback.CommandName.StartsWith(TEXT("Resolve"))
			|| Feedback.CommandName.StartsWith(TEXT("Apply"))
			|| Feedback.CommandName == TEXT("BeginResolutionSession");
		AddLine(
			Feedback.bRejected
				? TEXT("COMMAND REJECTED")
				: bResolutionOperation
					? TEXT("RESOLUTION RESULT") : TEXT("LAST ACTION"),
			Feedback.bRejected
				? FLinearColor(1.0f, 0.38f, 0.30f)
				: FLinearColor(0.42f, 0.82f, 1.0f));
		AddLine(TEXT("Step: ") + Feedback.StepTitle);
		AddLine(Feedback.StepSummary);
		AddLine(Feedback.RouteSummary.IsEmpty()
			? FString() : TEXT("Route: ") + Feedback.RouteSummary);

		if (!Feedback.DiceEntries.IsEmpty())
		{
			AddLine(TEXT("DICE"), FLinearColor(0.62f, 0.78f, 0.92f));
			TSharedRef<SWrapBox> DiceRow = SNew(SWrapBox)
				.UseAllottedSize(true)
				.InnerSlotPadding(FVector2D(4.0f, 4.0f));
			for (const FFMCodexLocalMatchRollView& Die : Feedback.DiceEntries)
			{
				DiceRow->AddSlot()
				[
					SNew(SBorder)
					.Padding(FMargin(10.0f, 6.0f))
					.BorderBackgroundColor(FLinearColor(0.13f, 0.20f, 0.27f, 1.0f))
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight()
						[
							SNew(STextBlock)
							.Text(FText::FromString(Die.Purpose))
							.ColorAndOpacity(FLinearColor(0.70f, 0.78f, 0.86f))
						]
						+ SVerticalBox::Slot().AutoHeight()
						[
							SNew(STextBlock)
							.Text(FText::FromString(FString::Printf(
								TEXT("D6  %d"), Die.RawD6)))
							.ColorAndOpacity(FLinearColor(1.0f, 0.88f, 0.42f))
						]
					]
				];
			}
			Body->AddSlot().AutoHeight().Padding(0.0f, 2.0f, 0.0f, 6.0f)
			[
				DiceRow
			];
		}
		if (!Feedback.ComparisonEntries.IsEmpty())
		{
			AddLine(TEXT("FORMULA COMPARISON"),
				FLinearColor(0.62f, 0.78f, 0.92f));
			TSharedRef<SHorizontalBox> ComparisonRow = SNew(SHorizontalBox);
			for (int32 Index = 0;
				Index < FMath::Min(2, Feedback.ComparisonEntries.Num()); ++Index)
			{
				ComparisonRow->AddSlot().FillWidth(1.0f).Padding(2.0f)
				[
					SNew(SBorder)
					.Padding(7.0f)
					.BorderBackgroundColor(Index == 0
						? FLinearColor(0.08f, 0.24f, 0.40f, 1.0f)
						: FLinearColor(0.34f, 0.11f, 0.10f, 1.0f))
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight()
						[
							SNew(STextBlock)
							.Text(FText::FromString(Index == 0
								? TEXT("ATTACK") : TEXT("DEFENSE")))
						]
						+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 3.0f)
						[
							SNew(STextBlock)
							.Text(FText::FromString(
								Feedback.ComparisonEntries[Index]))
							.AutoWrapText(true)
						]
					]
				];
			}
			Body->AddSlot().AutoHeight()[ComparisonRow];
			for (int32 Index = 2; Index < Feedback.ComparisonEntries.Num(); ++Index)
			{
				AddLine(TEXT("Evidence: ") + Feedback.ComparisonEntries[Index]);
			}
		}
		AddLine(Feedback.DecisionSummary.IsEmpty()
			? FString() : TEXT("Decision: ") + Feedback.DecisionSummary);
		AddLine(Feedback.ContinuationSummary);
		if (!Feedback.TerminalSummary.IsEmpty())
		{
			Body->AddSlot().AutoHeight().Padding(0.0f, 8.0f, 0.0f, 2.0f)
			[
				SNew(SBorder)
				.Padding(10.0f)
				.BorderBackgroundColor(FLinearColor(0.36f, 0.24f, 0.03f, 1.0f))
				[
					SNew(STextBlock)
					.Text(FText::FromString(Feedback.TerminalSummary))
					.Justification(ETextJustify::Center)
					.ColorAndOpacity(FLinearColor(1.0f, 0.88f, 0.42f))
				]
			];
		}
		if (Feedback.bRejected)
		{
			AddLine(TEXT("Command: ") + Feedback.CommandName);
			AddLine(TEXT("Reason: ") + Feedback.StepSummary);
			AddLine(TEXT("Message: ") + Feedback.ErrorMessage);
		}

		return SNew(SBorder)
			.Padding(10.0f)
			.BorderBackgroundColor(Feedback.bRejected
				? FLinearColor(0.30f, 0.06f, 0.05f, 0.96f)
				: FLinearColor(0.05f, 0.14f, 0.22f, 0.96f))
			[
				Body
			];
	}
}

AFMCodexLocalMatchPlayerController::AFMCodexLocalMatchPlayerController()
{
	bShowMouseCursor = true;
	PlayerMatchScreenClass = UFMCodexLocalMatchScreenWidget::StaticClass();
}

const FFMCodexLocalMatchInteractionView&
AFMCodexLocalMatchPlayerController::GetInteractionView() const
{
	return InteractionView;
}

const FFMCodexLocalMatchCommandDiagnostic&
AFMCodexLocalMatchPlayerController::GetLastDiagnostic() const
{
	return LastDiagnostic;
}

const FFMCodexLocalMatchResolutionFeedback&
AFMCodexLocalMatchPlayerController::GetResolutionFeedback() const
{
	return ResolutionFeedback;
}

UFMCodexLocalMatchScreenWidget*
AFMCodexLocalMatchPlayerController::GetPlayerMatchScreen() const
{
	return PlayerMatchScreen;
}

void AFMCodexLocalMatchPlayerController::BeginPlay()
{
	Super::BeginPlay();
	RefreshPresentation();
	if (IsLocalController())
	{
		InitializePlayerFacingUI();
		InitializeDeveloperSlateSurface();
#if !UE_BUILD_SHIPPING
		InitializeLocalDevRollOverrideSurface();
#endif
	}
}

void AFMCodexLocalMatchPlayerController::InitializePlayerFacingUI()
{
	if (!bEnablePlayerUMGSurface || PlayerMatchScreen != nullptr)
	{
		return;
	}
	UClass* ScreenClass = PlayerMatchScreenClass != nullptr
		? PlayerMatchScreenClass.Get()
		: UFMCodexLocalMatchScreenWidget::StaticClass();
	PlayerMatchScreen = Player != nullptr
		? CreateWidget<UFMCodexLocalMatchScreenWidget>(this, ScreenClass)
		: CreateWidget<UFMCodexLocalMatchScreenWidget>(GetWorld(), ScreenClass);
	if (PlayerMatchScreen == nullptr)
	{
		return;
	}
	PlayerMatchScreen->SetMatchController(this);
	RefreshPlayerMatchScreen();
	if (IsLocalController() && GEngine != nullptr
		&& GEngine->GameViewport != nullptr)
	{
		PlayerMatchScreen->AddToViewport(50);
		FInputModeGameAndUI InputMode;
		InputMode.SetHideCursorDuringCapture(false);
		InputMode.SetWidgetToFocus(PlayerMatchScreen->TakeWidget());
		SetInputMode(InputMode);
	}
}

void AFMCodexLocalMatchPlayerController::InitializeDeveloperSlateSurface()
{
	if (!bEnableDeveloperSlateSurface || SurfaceContainer.IsValid()
		|| GEngine == nullptr || GEngine->GameViewport == nullptr)
	{
		return;
	}
	SAssignNew(SurfaceContainer, SBox)
	[
		BuildControlSurface()
	];
	ViewportWidget = SNew(SBorder)
		.Padding(12.0f)
		.BorderBackgroundColor(FLinearColor(0.02f, 0.02f, 0.02f, 0.94f))
		[
			SurfaceContainer.ToSharedRef()
		];
	GEngine->GameViewport->AddViewportWidgetContent(
		ViewportWidget.ToSharedRef(), 100);
}

#if !UE_BUILD_SHIPPING
void AFMCodexLocalMatchPlayerController::InitializeLocalDevRollOverrideSurface()
{
	if (DevRollOverrideViewportWidget.IsValid()
		|| GEngine == nullptr || GEngine->GameViewport == nullptr)
	{
		return;
	}
	DevRollOverrideViewportWidget =
		SNew(SOverlay)
		+ SOverlay::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Center)
		.Padding(FMargin(0.0f, 0.0f, 12.0f, 0.0f))
		[
			SNew(SFMCodexLocalDevRollOverrideWidget)
			.Controller(this)
		];
	GEngine->GameViewport->AddViewportWidgetContent(
		DevRollOverrideViewportWidget.ToSharedRef(), 200);
}
#endif

void AFMCodexLocalMatchPlayerController::EndPlay(
	const EEndPlayReason::Type EndPlayReason)
{
	CancelRecoveryNotificationDismiss();
	if (PlayerMatchScreen != nullptr)
	{
		PlayerMatchScreen->ClearMatchController();
		PlayerMatchScreen->RemoveFromParent();
		PlayerMatchScreen = nullptr;
	}
	if (ViewportWidget.IsValid() && GEngine != nullptr
		&& GEngine->GameViewport != nullptr)
	{
		GEngine->GameViewport->RemoveViewportWidgetContent(
			ViewportWidget.ToSharedRef());
	}
#if !UE_BUILD_SHIPPING
	if (DevRollOverrideViewportWidget.IsValid() && GEngine != nullptr
		&& GEngine->GameViewport != nullptr)
	{
		GEngine->GameViewport->RemoveViewportWidgetContent(
			DevRollOverrideViewportWidget.ToSharedRef());
	}
	DevRollOverrideViewportWidget.Reset();
#endif
	SurfaceContainer.Reset();
	ViewportWidget.Reset();
	Super::EndPlay(EndPlayReason);
}

#if !UE_BUILD_SHIPPING
FFMCodexLocalDevRollOverrideCommandResult
AFMCodexLocalMatchPlayerController::SetLocalDevRollOverride(
	const FFMCodexLocalDevRollOverrideRequest& Request)
{
	AFMCodexLocalMatchHostGameMode* Host = FindLocalMatchHost();
	if (Host == nullptr)
	{
		FFMCodexLocalDevRollOverrideCommandResult Result;
		Result.ErrorMessage = TEXT("Local authoritative Host unavailable.");
		return Result;
	}
	return Host->SetLocalDevRollOverride(Request);
}

void AFMCodexLocalMatchPlayerController::ClearLocalDevRollOverride(
	const EFMCodexLocalDevRollTarget Target)
{
	if (AFMCodexLocalMatchHostGameMode* Host = FindLocalMatchHost())
	{
		Host->ClearLocalDevRollOverride(Target);
	}
}

void AFMCodexLocalMatchPlayerController::ClearAllLocalDevRollOverrides()
{
	if (AFMCodexLocalMatchHostGameMode* Host = FindLocalMatchHost())
	{
		Host->ClearAllLocalDevRollOverrides();
	}
}

TArray<FFMCodexLocalDevPendingRollOverride>
AFMCodexLocalMatchPlayerController::GetLocalDevPendingRollOverrides() const
{
	if (const AFMCodexLocalMatchHostGameMode* Host = FindLocalMatchHost())
	{
		return Host->GetLocalDevPendingRollOverrides();
	}
	return {};
}
#endif

AFMCodexLocalMatchHostGameMode*
AFMCodexLocalMatchPlayerController::FindLocalMatchHost() const
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return nullptr;
	}
	if (AFMCodexLocalMatchHostGameMode* Host =
		Cast<AFMCodexLocalMatchHostGameMode>(World->GetAuthGameMode()))
	{
		return Host;
	}
#if WITH_DEV_AUTOMATION_TESTS
	for (TActorIterator<AFMCodexLocalMatchHostGameMode> It(World); It; ++It)
	{
		return *It;
	}
#endif
	return nullptr;
}

IMatchPlayPlayerIntentPort*
AFMCodexLocalMatchPlayerController::FindPlayerIntentPort() const
{
	return FindLocalMatchHost();
}

const IFMCodexMatchClientViewPort*
AFMCodexLocalMatchPlayerController::FindClientViewPort() const
{
	return FindLocalMatchHost();
}

void AFMCodexLocalMatchPlayerController::RefreshPresentation()
{
	const IFMCodexMatchClientViewPort* ViewPort = FindClientViewPort();
	if (ViewPort == nullptr)
	{
		const auto NewView =
			FFMCodexLocalMatchInteractionViewBuilder::BuildNoActiveMatch();
		InteractionView = NewView;
		RebuildControlSurface();
		return;
	}

	// Local hot-seat uses the same viewer-safe DTO boundary as a future remote
	// client. Its existing presentation timers still control when already
	// published facts are animated on screen.
	const FFMCodexLocalMatchViewerDisclosure Disclosure =
		FFMCodexLocalMatchViewerDisclosure::FullyDisclosed();
	FFMCodexMatchClientViewRequest PlayerARequest;
	PlayerARequest.ViewerSide = EInitialTurnOrderPlayer::PlayerA;
	PlayerARequest.Disclosure = Disclosure;
	FFMCodexMatchClientViewRequest PlayerBRequest = PlayerARequest;
	PlayerBRequest.ViewerSide = EInitialTurnOrderPlayer::PlayerB;
	FFMCodexMatchClientViewResult ViewForPlayerAResult =
		ViewPort->GetViewForViewer(PlayerARequest);
	FFMCodexMatchClientViewResult ViewForPlayerBResult =
		ViewPort->GetViewForViewer(PlayerBRequest);
	if (!ViewForPlayerAResult.bSuccess || !ViewForPlayerBResult.bSuccess)
	{
		auto NewView =
			FFMCodexLocalMatchInteractionViewBuilder::BuildNoActiveMatch();
		NewView.Diagnostic = !ViewForPlayerAResult.bSuccess
			? ViewForPlayerAResult.ErrorMessage
			: ViewForPlayerBResult.ErrorMessage;
		InteractionView = MoveTemp(NewView);
		RebuildControlSurface();
		return;
	}
	auto& ViewForPlayerA = ViewForPlayerAResult.View;
	auto& ViewForPlayerB = ViewForPlayerBResult.View;
	const EInitialTurnOrderPlayer LocalViewerSide =
		ViewForPlayerA.ExpectedActingPlayer == EInitialTurnOrderPlayer::PlayerB
			? EInitialTurnOrderPlayer::PlayerB
			: ViewForPlayerA.ExpectedActingPlayer
				== EInitialTurnOrderPlayer::PlayerA
				? EInitialTurnOrderPlayer::PlayerA
				: ViewForPlayerA.CurrentAttackingPlayer;
	auto NewView = LocalViewerSide == EInitialTurnOrderPlayer::PlayerB
		? MoveTemp(ViewForPlayerB)
		: MoveTemp(ViewForPlayerA);
	InteractionView = MoveTemp(NewView);
	ReconcileSetPieceDraft();
	if (InteractionView.bTerminalPendingAdvance
		&& !ResolutionFeedback.bTerminal)
	{
		ResolutionFeedback =
			FFMCodexLocalMatchResolutionFeedbackBuilder
				::BuildFromTerminalSnapshot(InteractionView);
	}
	RebuildControlSurface();
}

void AFMCodexLocalMatchPlayerController::ResolveAutomaticNoLegalHelperIfNeeded()
{
	// Compatibility hook only; no-legal Helper progression is server-owned.
	RefreshPresentation();
}

void AFMCodexLocalMatchPlayerController::RecordLocalFailure(
	const FString& CommandName,
	const FString& Message)
{
	CancelRecoveryNotificationDismiss();
	LastDiagnostic.CommandName = CommandName;
	LastDiagnostic.bHostSuccess = false;
	LastDiagnostic.bAuthoritativeAccepted = false;
	LastDiagnostic.bAuthoritativeSuccess = false;
	LastDiagnostic.Message = Message;
	LastDiagnostic.PresentationSummary = TEXT("Command rejected");
	ResolutionFeedback =
		FFMCodexLocalMatchResolutionFeedbackBuilder::BuildRejected(
			CommandName, Message, false);
	RefreshPresentation();
}

void AFMCodexLocalMatchPlayerController::ScheduleRecoveryNotificationDismiss()
{
	CancelRecoveryNotificationDismiss();
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			RecoveryNotificationDismissTimerHandle,
			this,
			&AFMCodexLocalMatchPlayerController::DismissRecoveryNotification,
			RecoveryNotificationDurationSeconds,
			false);
	}
}

void AFMCodexLocalMatchPlayerController::CancelRecoveryNotificationDismiss()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(
			RecoveryNotificationDismissTimerHandle);
	}
}

void AFMCodexLocalMatchPlayerController::DismissRecoveryNotification()
{
	CancelRecoveryNotificationDismiss();
	if (!ResolutionFeedback.bNonBlockingNotification)
	{
		return;
	}
	ResolutionFeedback = FFMCodexLocalMatchResolutionFeedback();
	RebuildControlSurface();
}

#if WITH_DEV_AUTOMATION_TESTS
bool AFMCodexLocalMatchPlayerController::
IsRecoveryNotificationDismissScheduledForTesting() const
{
	UWorld* World = GetWorld();
	return World != nullptr
		&& World->GetTimerManager().IsTimerActive(
			RecoveryNotificationDismissTimerHandle);
}

void AFMCodexLocalMatchPlayerController::
ExpireRecoveryNotificationForTesting()
{
	DismissRecoveryNotification();
}
#endif

#if !UE_BUILD_SHIPPING
void AFMCodexLocalMatchPlayerController::StartNewDevShortMatch()
{
	AFMCodexLocalMatchHostGameMode* Host = FindLocalMatchHost();
	if (!Host) return;
	ResetSetPieceDraft();
	auto Demo = FFMCodexLocalMatchDemoConfigurationFactory::Create();
	Demo.OpeningInput.OpeningInput.bUseDevOneAttackPerSide = true;
	RecordCommandResult(TEXT("StartNewLocalMatch"),
		Host->StartNewLocalMatch(Demo.OpeningInput, Demo.SkillRuleSet));
}
#endif

void AFMCodexLocalMatchPlayerController::StartNewDemoMatch()
{
	ResetSetPieceDraft();
	AFMCodexLocalMatchHostGameMode* Host = FindLocalMatchHost();
	if (Host == nullptr)
	{
		RecordLocalFailure(TEXT("StartNewLocalMatch"),
			TEXT("The local authoritative GameMode Host is unavailable."));
		return;
	}
	const FFMCodexLocalMatchDemoConfiguration Demo =
		FFMCodexLocalMatchDemoConfigurationFactory::Create();
	#if WITH_DEV_AUTOMATION_TESTS
	if (NextDemoMatchSeedForTesting != INDEX_NONE)
	{
		const int32 Seed = NextDemoMatchSeedForTesting;
		NextDemoMatchSeedForTesting = INDEX_NONE;
		RecordCommandResult(
			TEXT("StartNewLocalMatch"),
			Host->StartNewLocalMatch(
				Demo.OpeningInput, Demo.SkillRuleSet, Seed));
		return;
	}
	#endif
	RecordCommandResult(
		TEXT("StartNewLocalMatch"),
		Host->StartNewLocalMatch(Demo.OpeningInput, Demo.SkillRuleSet));
}

#if WITH_DEV_AUTOMATION_TESTS
void AFMCodexLocalMatchPlayerController::SetNextDemoMatchSeedForTesting(
	const int32 Seed)
{
	NextDemoMatchSeedForTesting = Seed;
}
#endif

void AFMCodexLocalMatchPlayerController::RollDemoTacticalPoints()
{
	FMatchPlayFullD12EntryRequest Request;
	Request.RequestingSide = InteractionView.ExpectedActingPlayer;
	Request.ExpectedAttackSequence = InteractionView.AttackSequence;
	SubmitPlayerIntent(TEXT("RequestInitialActionPointRoll"),
		EMatchPlayAuthoritativeCommandKind::RequestInitialActionPointRoll,
		Request);
}

void AFMCodexLocalMatchPlayerController::ResetSetPieceDraft()
{
	SetPieceDraft = FSetPieceLocalDraft();
}

void AFMCodexLocalMatchPlayerController::ReconcileSetPieceDraft()
{
	const bool bCarrierDraft = InteractionView.InteractionCategory
		== EFMCodexLocalMatchInteractionCategory::SelectSetPieceCarrier
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::ConfirmSetPieceCarrier;
	const bool bCornerDraft = InteractionView.InteractionCategory
		== EFMCodexLocalMatchInteractionCategory::DraftCornerAttacker
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::DraftCornerDefender;
	if (!bCarrierDraft && !bCornerDraft)
	{
		ResetSetPieceDraft();
		return;
	}
	if (SetPieceDraft.AttackSequence != InteractionView.AttackSequence
		|| SetPieceDraft.Side != InteractionView.ExpectedActingPlayer
		|| SetPieceDraft.Type != InteractionView.SetPieceType
		|| (bCornerDraft
			&& SetPieceDraft.CornerStage != InteractionView.CornerStage))
	{
		ResetSetPieceDraft();
		SetPieceDraft.AttackSequence = InteractionView.AttackSequence;
		SetPieceDraft.Side = InteractionView.ExpectedActingPlayer;
		SetPieceDraft.Type = InteractionView.SetPieceType;
		SetPieceDraft.CornerStage = InteractionView.CornerStage;
	}
	if (bCarrierDraft)
	{
		if (!InteractionView.LegalSetPieceCardIds.Contains(
			SetPieceDraft.CarrierCardId))
		{
			SetPieceDraft.CarrierCardId = NAME_None;
		}
		InteractionView.DraftSetPieceCarrierCardId =
			SetPieceDraft.CarrierCardId;
		if (!SetPieceDraft.CarrierCardId.IsNone())
		{
			InteractionView.InteractionCategory =
				EFMCodexLocalMatchInteractionCategory::ConfirmSetPieceCarrier;
		}
		return;
	}
	SetPieceDraft.CornerCardIds.RemoveAll(
		[this](const FName CardId)
		{
			return !InteractionView.LegalSetPieceCardIds.Contains(CardId);
		});
	if (SetPieceDraft.CornerCardIds.Num() > 3)
	{
		SetPieceDraft.CornerCardIds.SetNum(3);
	}
	InteractionView.DraftCornerNomineeCardIds =
		SetPieceDraft.CornerCardIds;
	InteractionView.bCornerLockConfirmationPending = SetPieceDraft.bLockConfirmationPending;
}

void AFMCodexLocalMatchPlayerController::ToggleSetPieceDraftCard(
	const FName CardId)
{
	if (SetPieceDraft.bLockConfirmationPending) return;
	if (!InteractionView.LegalSetPieceCardIds.Contains(CardId))
	{
		RecordLocalFailure(TEXT("SetPieceDraft"),
			TEXT("该球员不在当前权威合法候选中。"));
		return;
	}
	const bool bCarrierDraft = InteractionView.InteractionCategory
		== EFMCodexLocalMatchInteractionCategory::SelectSetPieceCarrier
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::ConfirmSetPieceCarrier;
	if (bCarrierDraft)
	{
		SetPieceDraft.CarrierCardId =
			SetPieceDraft.CarrierCardId == CardId ? NAME_None : CardId;
	}
	else if (InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::DraftCornerAttacker
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::DraftCornerDefender)
	{
		const int32 Existing = SetPieceDraft.CornerCardIds.IndexOfByKey(CardId);
		if (Existing != INDEX_NONE)
		{
			SetPieceDraft.CornerCardIds.RemoveAt(Existing);
		}
		else if (SetPieceDraft.CornerCardIds.Num() < 3)
		{
			SetPieceDraft.CornerCardIds.Add(CardId);
		}
	}
	else
	{
		RecordLocalFailure(TEXT("SetPieceDraft"),
			TEXT("当前阶段不接受手牌定位球选择。"));
		return;
	}
	RefreshPresentation();
}

void AFMCodexLocalMatchPlayerController::ConfirmSetPieceDraft()
{
	const bool bCornerDraft = InteractionView.InteractionCategory
		== EFMCodexLocalMatchInteractionCategory::DraftCornerAttacker
		|| InteractionView.InteractionCategory == EFMCodexLocalMatchInteractionCategory::DraftCornerDefender;
	if (bCornerDraft && SetPieceDraft.CornerCardIds.Num() < 3
		&& !SetPieceDraft.bLockConfirmationPending)
	{
		SetPieceDraft.bLockConfirmationPending = true;
		RefreshPresentation();
		return;
	}
	SetPieceDraft.bLockConfirmationPending = false;
	if (InteractionView.InteractionCategory
		== EFMCodexLocalMatchInteractionCategory::ConfirmSetPieceCarrier)
	{
		FMatchPlaySetPieceCarrierSelectionRequest Request;
		Request.RequestingSide = InteractionView.ExpectedActingPlayer;
		Request.AttackSequence = InteractionView.AttackSequence;
		Request.CardId = SetPieceDraft.CarrierCardId;
		SubmitPlayerIntent(TEXT("SubmitSetPieceCarrier"),
			EMatchPlayAuthoritativeCommandKind::SubmitSetPieceCarrier,
			Request);
		return;
	}
	FMatchPlayCornerNominationRequest Request;
	Request.RequestingSide = InteractionView.ExpectedActingPlayer;
	Request.AttackSequence = InteractionView.AttackSequence;
	Request.OrderedCardIds = SetPieceDraft.CornerCardIds;
	if (InteractionView.InteractionCategory
		== EFMCodexLocalMatchInteractionCategory::DraftCornerAttacker)
	{
		SubmitPlayerIntent(TEXT("SubmitCornerAttackerNominations"),
			EMatchPlayAuthoritativeCommandKind::SubmitCornerAttackerNominations,
			Request);
	}
	else if (InteractionView.InteractionCategory
		== EFMCodexLocalMatchInteractionCategory::DraftCornerDefender)
	{
		SubmitPlayerIntent(TEXT("SubmitCornerDefenderNominations"),
			EMatchPlayAuthoritativeCommandKind::SubmitCornerDefenderNominations,
			Request);
	}
	else
	{
		RecordLocalFailure(TEXT("ConfirmSetPieceDraft"),
			TEXT("当前没有可确认的定位球草稿。"));
	}
}

void AFMCodexLocalMatchPlayerController::CancelCornerLockConfirmation()
{
	SetPieceDraft.bLockConfirmationPending = false;
	RefreshPresentation();
}

void AFMCodexLocalMatchPlayerController::NotifyEntryRevealComplete()
{
	// Authority is already at a stable state. Reveal completion is presentation-only.
	RefreshPresentation();
}

void AFMCodexLocalMatchPlayerController::ResolveAutomaticSetPieceEntryIfNeeded()
{
	// Retained as a presentation compatibility hook. The server coordinator now
	// owns AP1 and no-legal set-piece continuation.
	RefreshPresentation();
}

void AFMCodexLocalMatchPlayerController::SubmitShortFreeKickMethod(
	const EMatchPlayShortFreeKickMethod Method)
{
	FMatchPlayShortFreeKickMethodRequest Request;
	Request.RequestingSide = InteractionView.ExpectedActingPlayer;
	Request.AttackSequence = InteractionView.AttackSequence;
	Request.Method = Method;
	SubmitPlayerIntent(TEXT("SubmitShortFreeKickMethod"),
		EMatchPlayAuthoritativeCommandKind::SubmitShortFreeKickMethod,
		Request);
}

void AFMCodexLocalMatchPlayerController::SubmitLongFreeKickMethod(
	const EMatchPlayLongFreeKickMethod Method)
{
	FMatchPlayLongFreeKickMethodRequest Request;
	Request.RequestingSide = InteractionView.ExpectedActingPlayer;
	Request.AttackSequence = InteractionView.AttackSequence;
	Request.Method = Method;
	SubmitPlayerIntent(TEXT("SubmitLongFreeKickMethod"),
		EMatchPlayAuthoritativeCommandKind::SubmitLongFreeKickMethod,
		Request);
}

void AFMCodexLocalMatchPlayerController::SubmitPenaltyMethod(
	const EMatchPlayPenaltyMethod Method)
{
	FMatchPlayPenaltyMethodRequest Request;
	Request.RequestingSide = InteractionView.ExpectedActingPlayer;
	Request.AttackSequence = InteractionView.AttackSequence;
	Request.Method = Method;
	SubmitPlayerIntent(TEXT("SubmitPenaltyMethod"),
		EMatchPlayAuthoritativeCommandKind::SubmitPenaltyMethod,
		Request);
}

void AFMCodexLocalMatchPlayerController::SubmitCornerIntent(
	const EMatchPlayCornerRouteIntent Intent)
{
	FMatchPlayCornerIntentRequest Request;
	Request.RequestingSide = InteractionView.ExpectedActingPlayer;
	Request.AttackSequence = InteractionView.AttackSequence;
	Request.IntendedRoute = Intent;
	SubmitPlayerIntent(TEXT("SubmitCornerIntent"),
		EMatchPlayAuthoritativeCommandKind::SubmitCornerIntent,
		Request);
}

void AFMCodexLocalMatchPlayerController::DeployOrdinary(
	const FName CardId,
	const FName SlotId)
{
	FMatchPlayAuthoritativeDeployOrdinaryRequest Request;
	Request.ExpectedAttackSequence = InteractionView.AttackSequence;
	Request.RequestingSide = InteractionView.ExpectedActingPlayer;
	Request.CardId = CardId;
	Request.SlotId = SlotId;
	SubmitPlayerIntent(TEXT("DeployOrdinary"),
		EMatchPlayAuthoritativeCommandKind::DeployOrdinary, Request);
}

void AFMCodexLocalMatchPlayerController::DeployGoalkeeper(const FName SlotId)
{
	FMatchPlayAuthoritativeDeployGoalkeeperRequest Request;
	Request.ExpectedAttackSequence = InteractionView.AttackSequence;
	Request.RequestingSide = InteractionView.ExpectedActingPlayer;
	Request.SlotId = SlotId;
	SubmitPlayerIntent(TEXT("DeployGoalkeeper"),
		EMatchPlayAuthoritativeCommandKind::DeployGoalkeeper, Request);
}

void AFMCodexLocalMatchPlayerController::FinishDeployment()
{
	FMatchPlayFinishDeploymentIntent Request;
	Request.AttackSequence = InteractionView.AttackSequence;
	Request.RequestingSide = InteractionView.ExpectedActingPlayer;
	SubmitPlayerIntent(TEXT("FinishDeployment"),
		EMatchPlayAuthoritativeCommandKind::FinishDeployment, Request);
}

void AFMCodexLocalMatchPlayerController::SubmitCarrier(const FName CardId)
{
	FMatchPlayAuthoritativeSubmitCarrierRequest Request;
	Request.ExpectedAttackSequence = InteractionView.AttackSequence;
	Request.RequestingSide = InteractionView.ExpectedActingPlayer;
	Request.CarrierCardId = CardId;
	SubmitPlayerIntent(TEXT("SubmitCarrier"),
		EMatchPlayAuthoritativeCommandKind::SubmitCarrier, Request);
}

void AFMCodexLocalMatchPlayerController::SubmitMarker(const FName CardId)
{
	FMatchPlayAuthoritativeSubmitMarkerRequest Request;
	Request.ExpectedAttackSequence = InteractionView.AttackSequence;
	Request.RequestingSide = InteractionView.ExpectedActingPlayer;
	Request.MarkerCardId = CardId;
	SubmitPlayerIntent(TEXT("SubmitMarker"),
		EMatchPlayAuthoritativeCommandKind::SubmitMarker, Request);
}

void AFMCodexLocalMatchPlayerController::SubmitSkill(const FName SkillId)
{
	FMatchPlayAuthoritativeSubmitSkillRequest Request;
	Request.ExpectedAttackSequence = InteractionView.AttackSequence;
	Request.RequestingSide = InteractionView.ExpectedActingPlayer;
	Request.SkillId = SkillId;
	SubmitPlayerIntent(TEXT("SubmitSkill"),
		EMatchPlayAuthoritativeCommandKind::SubmitSkill, Request);
}

void AFMCodexLocalMatchPlayerController::SubmitRunner(const FName CardId)
{
	FMatchPlayAuthoritativeSubmitRunnerRequest Request;
	Request.ExpectedAttackSequence = InteractionView.AttackSequence;
	Request.RequestingSide = InteractionView.ExpectedActingPlayer;
	Request.RunnerCardId = CardId;
	SubmitPlayerIntent(TEXT("SubmitRunner"),
		EMatchPlayAuthoritativeCommandKind::SubmitRunner, Request);
}

void AFMCodexLocalMatchPlayerController::SubmitHelper(const FName CardId)
{
	FMatchPlayAuthoritativeSubmitHelperRequest Request;
	Request.ExpectedAttackSequence = InteractionView.AttackSequence;
	Request.RequestingSide = InteractionView.ExpectedActingPlayer;
	Request.HelperCardId = CardId;
	SubmitPlayerIntent(TEXT("SubmitHelper"),
		EMatchPlayAuthoritativeCommandKind::SubmitHelper, Request);
}

void AFMCodexLocalMatchPlayerController::AbandonCurrentTacticalSelection()
{
	if (InteractionView.InteractionCategory
		!= EFMCodexLocalMatchInteractionCategory::SelectSkill)
	{
		RecordLocalFailure(
			TEXT("AbandonTacticalSelection"),
			TEXT("The current authoritative stage is not tactical selection."));
		return;
	}

	if (InteractionView.bCanDecline
		&& !InteractionView.bCanResolveNoLegalChoice)
	{
		DeclineCurrentSelection();
		return;
	}

	if (!InteractionView.bCanDecline
		&& InteractionView.bCanResolveNoLegalChoice)
	{
		ResolveNoLegalCurrentSelection();
		return;
	}

	RecordLocalFailure(
		TEXT("AbandonTacticalSelection"),
		TEXT("Tactical abandon requires exactly one projected authoritative path."));
}

void AFMCodexLocalMatchPlayerController::DeclineCurrentSelection()
{
	switch (InteractionView.InteractionCategory)
	{
	case EFMCodexLocalMatchInteractionCategory::SelectMarker:
	{
		FMatchPlayAuthoritativeDeclineMarkerRequest Request;
		Request.ExpectedAttackSequence = InteractionView.AttackSequence;
		Request.RequestingSide = InteractionView.ExpectedActingPlayer;
		SubmitPlayerIntent(TEXT("DeclineMarker"),
			EMatchPlayAuthoritativeCommandKind::DeclineMarker, Request);
		break;
	}
	case EFMCodexLocalMatchInteractionCategory::SelectSkill:
	{
		FMatchPlayAuthoritativeDeclineSkillRequest Request;
		Request.ExpectedAttackSequence = InteractionView.AttackSequence;
		Request.RequestingSide = InteractionView.ExpectedActingPlayer;
		SubmitPlayerIntent(TEXT("DeclineSkill"),
			EMatchPlayAuthoritativeCommandKind::DeclineSkill, Request);
		break;
	}
	case EFMCodexLocalMatchInteractionCategory::SelectRunner:
	{
		FMatchPlayAuthoritativeDeclineRunnerRequest Request;
		Request.ExpectedAttackSequence = InteractionView.AttackSequence;
		Request.RequestingSide = InteractionView.ExpectedActingPlayer;
		SubmitPlayerIntent(TEXT("DeclineRunner"),
			EMatchPlayAuthoritativeCommandKind::DeclineRunner, Request);
		break;
	}
	case EFMCodexLocalMatchInteractionCategory::SelectHelper:
	{
		FMatchPlayAuthoritativeDeclineHelperRequest Request;
		Request.ExpectedAttackSequence = InteractionView.AttackSequence;
		Request.RequestingSide = InteractionView.ExpectedActingPlayer;
		SubmitPlayerIntent(TEXT("DeclineHelper"),
			EMatchPlayAuthoritativeCommandKind::DeclineHelper, Request);
		break;
	}
	default:
		RecordLocalFailure(
			TEXT("DeclineSelection"),
			TEXT("The current authoritative stage has no decline contract."));
		break;
	}
}

void AFMCodexLocalMatchPlayerController::ResolveNoLegalCurrentSelection()
{
	// Compatibility/recovery only. Normal player-intent submission already runs
	// the authoritative coordinator past every no-legal participant state.
	ContinueResolution();
}

void AFMCodexLocalMatchPlayerController::SubmitBranchIntent(
	const EMatchPlayElectiveBranchIntent Intent)
{
	FMatchPlayAuthoritativeSubmitBranchIntentRequest Request;
	Request.AttackSequence = InteractionView.AttackSequence;
	Request.RequestingSide = InteractionView.ExpectedActingPlayer;
	Request.Intent = Intent;
	SubmitPlayerIntent(TEXT("SubmitBranchIntent"),
		EMatchPlayAuthoritativeCommandKind::SubmitBranchIntent, Request);
}

void AFMCodexLocalMatchPlayerController::SubmitOneOnOneShotChoice(
	const EMatchPlayThroughBallOneOnOneShotChoice Choice)
{
	FMatchPlayAuthoritativeSubmitThroughBallOneOnOneShotChoiceRequest Request;
	Request.ExpectedAttackSequence = InteractionView.AttackSequence;
	Request.RequestingSide = InteractionView.ExpectedActingPlayer;
	Request.Choice = Choice;
	SubmitPlayerIntent(TEXT("SubmitThroughBallOneOnOneShotChoice"),
		EMatchPlayAuthoritativeCommandKind
			::SubmitThroughBallOneOnOneShotChoice,
		Request);
}

void AFMCodexLocalMatchPlayerController::RollCrossAttack()
{
	if (bCrossRollCommandInFlight)
	{
		return;
	}
	if (InteractionView.InteractionCategory
		!= EFMCodexLocalMatchInteractionCategory::RollCrossAttack)
	{
		RecordLocalFailure(
			TEXT("ResolveCrossAttackRoll"),
			TEXT("Cross attack roll is not the current interaction."));
		return;
	}
	const bool bHigh = InteractionView.ResolutionFacts.bHasActualBranch
		&& InteractionView.ResolutionFacts.ActualBranch.Cross
			== EMatchPlayCrossActualBranch::High;
	bCrossRollCommandInFlight = true;
	if (bHigh)
	{
		FMatchPlayAuthoritativeResolveCrossHighAttackRollRequest Request;
		Request.AttackSequence = InteractionView.AttackSequence;
		Request.RequestingSide = InteractionView.ExpectedActingPlayer;
		SubmitPlayerIntent(
			TEXT("ResolveCrossHighAttackRoll"),
			EMatchPlayAuthoritativeCommandKind::ResolveCrossHighAttackRoll,
			Request);
		bCrossRollCommandInFlight = false;
		RebuildControlSurface();
	}
	else
	{
		FMatchPlayAuthoritativeResolveCrossLowAttackRollRequest Request;
		Request.AttackSequence = InteractionView.AttackSequence;
		Request.RequestingSide = InteractionView.ExpectedActingPlayer;
		SubmitPlayerIntent(
			TEXT("ResolveCrossLowAttackRoll"),
			EMatchPlayAuthoritativeCommandKind::ResolveCrossLowAttackRoll,
			Request);
		bCrossRollCommandInFlight = false;
		RebuildControlSurface();
	}
}

void AFMCodexLocalMatchPlayerController::RollCrossDefense()
{
	if (bCrossRollCommandInFlight)
	{
		return;
	}
	if (InteractionView.InteractionCategory
		!= EFMCodexLocalMatchInteractionCategory::RollCrossDefense)
	{
		RecordLocalFailure(
			TEXT("ResolveCrossDefenseRoll"),
			TEXT("Cross defense roll is not the current interaction."));
		return;
	}
	const bool bHigh = InteractionView.ResolutionFacts.bHasActualBranch
		&& InteractionView.ResolutionFacts.ActualBranch.Cross
			== EMatchPlayCrossActualBranch::High;
	bCrossRollCommandInFlight = true;
	if (bHigh)
	{
		FMatchPlayAuthoritativeResolveCrossHighDefenseRollRequest Request;
		Request.AttackSequence = InteractionView.AttackSequence;
		Request.RequestingSide = InteractionView.ExpectedActingPlayer;
		SubmitPlayerIntent(
			TEXT("ResolveCrossHighDefenseRoll"),
			EMatchPlayAuthoritativeCommandKind::ResolveCrossHighDefenseRoll,
			Request);
		bCrossRollCommandInFlight = false;
		RebuildControlSurface();
	}
	else
	{
		FMatchPlayAuthoritativeResolveCrossLowDefenseRollRequest Request;
		Request.AttackSequence = InteractionView.AttackSequence;
		Request.RequestingSide = InteractionView.ExpectedActingPlayer;
		SubmitPlayerIntent(
			TEXT("ResolveCrossLowDefenseRoll"),
			EMatchPlayAuthoritativeCommandKind::ResolveCrossLowDefenseRoll,
			Request);
		bCrossRollCommandInFlight = false;
		RebuildControlSurface();
	}
}

void AFMCodexLocalMatchPlayerController::ApplyCrossTerminalResolution()
{
	if (InteractionView.InteractionCategory
			!= EFMCodexLocalMatchInteractionCategory
				::ApplyCrossTerminalResolution
		|| !InteractionView.bCrossFormulaComplete
		|| !InteractionView.bCrossTerminalActionAvailable)
	{
		RecordLocalFailure(
			TEXT("ApplyCrossTerminalResolution"),
			TEXT("The completed Cross terminal action is not available."));
		return;
	}
	ContinueResolution();
}

void AFMCodexLocalMatchPlayerController::RollLongShotDirectAttack()
{
	if (bLongShotRollCommandInFlight)
	{
		return;
	}
	if (InteractionView.InteractionCategory
		!= EFMCodexLocalMatchInteractionCategory::RollLongShotDirectAttack)
	{
		RecordLocalFailure(
			TEXT("ResolveLongShotDirectAttackRoll"),
			TEXT("LongShot Direct attack roll is not the current interaction."));
		return;
	}
	FMatchPlayAuthoritativeResolveLongShotDirectAttackRollRequest Request;
	Request.AttackSequence = InteractionView.AttackSequence;
	Request.RequestingSide = InteractionView.ExpectedActingPlayer;
	bLongShotRollCommandInFlight = true;
	SubmitPlayerIntent(
		TEXT("ResolveLongShotDirectAttackRoll"),
		EMatchPlayAuthoritativeCommandKind::ResolveLongShotDirectAttackRoll,
		Request);
	bLongShotRollCommandInFlight = false;
	RebuildControlSurface();
}

void AFMCodexLocalMatchPlayerController::RollLongShotDirectDefense()
{
	if (bLongShotRollCommandInFlight)
	{
		return;
	}
	if (InteractionView.InteractionCategory
		!= EFMCodexLocalMatchInteractionCategory::RollLongShotDirectDefense)
	{
		RecordLocalFailure(
			TEXT("ResolveLongShotDirectDefenseRoll"),
			TEXT("LongShot Direct defense roll is not the current interaction."));
		return;
	}
	FMatchPlayAuthoritativeResolveLongShotDirectDefenseRollRequest Request;
	Request.AttackSequence = InteractionView.AttackSequence;
	Request.RequestingSide = InteractionView.ExpectedActingPlayer;
	bLongShotRollCommandInFlight = true;
	SubmitPlayerIntent(
		TEXT("ResolveLongShotDirectDefenseRoll"),
		EMatchPlayAuthoritativeCommandKind::ResolveLongShotDirectDefenseRoll,
		Request);
	bLongShotRollCommandInFlight = false;
	RebuildControlSurface();
}

void AFMCodexLocalMatchPlayerController::RollLongShotDeadCorner()
{
	if (bLongShotRollCommandInFlight)
	{
		return;
	}
	if (InteractionView.InteractionCategory
		!= EFMCodexLocalMatchInteractionCategory::RollLongShotDeadCorner)
	{
		RecordLocalFailure(
			TEXT("ResolveLongShotDeadCornerRoll"),
			TEXT("LongShot DeadCorner roll is not the current interaction."));
		return;
	}
	FMatchPlayAuthoritativeResolveLongShotDeadCornerRollRequest Request;
	Request.AttackSequence = InteractionView.AttackSequence;
	Request.RequestingSide = InteractionView.ExpectedActingPlayer;
	bLongShotRollCommandInFlight = true;
	SubmitPlayerIntent(
		TEXT("ResolveLongShotDeadCornerRoll"),
		EMatchPlayAuthoritativeCommandKind::ResolveLongShotDeadCornerRoll,
		Request);
	bLongShotRollCommandInFlight = false;
	RebuildControlSurface();
}

void AFMCodexLocalMatchPlayerController::RollCutInsideShotDirectAttack()
{
	if (bCutInsideShotRollCommandInFlight)
	{
		return;
	}
	if (InteractionView.InteractionCategory
		!= EFMCodexLocalMatchInteractionCategory
			::RollCutInsideShotDirectAttack)
	{
		RecordLocalFailure(
			TEXT("ResolveCutInsideShotDirectAttackRoll"),
			TEXT("CutInsideShot Direct attack roll is not the current interaction."));
		return;
	}
	FMatchPlayAuthoritativeResolveCutInsideShotDirectAttackRollRequest Request;
	Request.AttackSequence = InteractionView.AttackSequence;
	Request.RequestingSide = InteractionView.ExpectedActingPlayer;
	bCutInsideShotRollCommandInFlight = true;
	SubmitPlayerIntent(TEXT("ResolveCutInsideShotDirectAttackRoll"),
		EMatchPlayAuthoritativeCommandKind
			::ResolveCutInsideShotDirectAttackRoll,
		Request);
	bCutInsideShotRollCommandInFlight = false;
	RebuildControlSurface();
}

void AFMCodexLocalMatchPlayerController::RollCutInsideShotDirectDefense()
{
	if (bCutInsideShotRollCommandInFlight)
	{
		return;
	}
	if (InteractionView.InteractionCategory
		!= EFMCodexLocalMatchInteractionCategory
			::RollCutInsideShotDirectDefense)
	{
		RecordLocalFailure(
			TEXT("ResolveCutInsideShotDirectDefenseRoll"),
			TEXT("CutInsideShot Direct defense roll is not the current interaction."));
		return;
	}
	FMatchPlayAuthoritativeResolveCutInsideShotDirectDefenseRollRequest Request;
	Request.AttackSequence = InteractionView.AttackSequence;
	Request.RequestingSide = InteractionView.ExpectedActingPlayer;
	bCutInsideShotRollCommandInFlight = true;
	SubmitPlayerIntent(TEXT("ResolveCutInsideShotDirectDefenseRoll"),
		EMatchPlayAuthoritativeCommandKind
			::ResolveCutInsideShotDirectDefenseRoll,
		Request);
	bCutInsideShotRollCommandInFlight = false;
	RebuildControlSurface();
}

void AFMCodexLocalMatchPlayerController::RollCutInsideShotDeadCorner()
{
	if (bCutInsideShotRollCommandInFlight)
	{
		return;
	}
	if (InteractionView.InteractionCategory
		!= EFMCodexLocalMatchInteractionCategory
			::RollCutInsideShotDeadCorner)
	{
		RecordLocalFailure(
			TEXT("ResolveCutInsideShotDeadCornerRoll"),
			TEXT("CutInsideShot DeadCorner roll is not the current interaction."));
		return;
	}
	FMatchPlayAuthoritativeResolveCutInsideShotDeadCornerRollRequest Request;
	Request.AttackSequence = InteractionView.AttackSequence;
	Request.RequestingSide = InteractionView.ExpectedActingPlayer;
	bCutInsideShotRollCommandInFlight = true;
	SubmitPlayerIntent(TEXT("ResolveCutInsideShotDeadCornerRoll"),
		EMatchPlayAuthoritativeCommandKind::ResolveCutInsideShotDeadCornerRoll,
		Request);
	bCutInsideShotRollCommandInFlight = false;
	RebuildControlSurface();
}

void AFMCodexLocalMatchPlayerController::CompleteCrossAndAdvance()
{
	ApplyCrossTerminalResolution();
}

void AFMCodexLocalMatchPlayerController::RollCrossRoute()
{
	if (bCrossRouteCommandInFlight)
	{
		return;
	}
	if (InteractionView.InteractionCategory
		!= EFMCodexLocalMatchInteractionCategory::RollCrossRoute)
	{
		RecordLocalFailure(
			TEXT("ResolveCrossInitialRouteRoll"),
			TEXT("Cross Initial Route roll is not the current interaction."));
		return;
	}
	FMatchPlayAuthoritativeResolveCrossInitialRouteRollRequest Request;
	Request.AttackSequence = InteractionView.AttackSequence;
	Request.RequestingSide = InteractionView.ExpectedActingPlayer;
	bCrossRouteCommandInFlight = true;
	SubmitPlayerIntent(TEXT("ResolveCrossInitialRouteRoll"),
		EMatchPlayAuthoritativeCommandKind::ResolveCrossInitialRouteRoll,
		Request);
	bCrossRouteCommandInFlight = false;
	RebuildControlSurface();
}

void AFMCodexLocalMatchPlayerController::RollPassControlRoute()
{
	if (bPassControlRollCommandInFlight)
	{
		return;
	}
	if (InteractionView.InteractionCategory
		!= EFMCodexLocalMatchInteractionCategory::RollPassControlRoute)
	{
		RecordLocalFailure(
			TEXT("ResolvePassControlInitialRouteRoll"),
			TEXT("PassControl Initial Route roll is not the current interaction."));
		return;
	}
	FMatchPlayAuthoritativeResolvePassControlInitialRouteRollRequest Request;
	Request.AttackSequence = InteractionView.AttackSequence;
	Request.RequestingSide = InteractionView.ExpectedActingPlayer;
	bPassControlRollCommandInFlight = true;
	SubmitPlayerIntent(TEXT("ResolvePassControlInitialRouteRoll"),
		EMatchPlayAuthoritativeCommandKind::ResolvePassControlInitialRouteRoll,
		Request);
	bPassControlRollCommandInFlight = false;
	RebuildControlSurface();
}

void AFMCodexLocalMatchPlayerController::RollPassControlAttack()
{
	if (bPassControlRollCommandInFlight)
	{
		return;
	}
	if (InteractionView.InteractionCategory
		!= EFMCodexLocalMatchInteractionCategory::RollPassControlAttack)
	{
		RecordLocalFailure(
			TEXT("ResolvePassControlAttackRoll"),
			TEXT("PassControl attack roll is not the current interaction."));
		return;
	}
	FMatchPlayAuthoritativeResolvePassControlAttackRollRequest Request;
	Request.AttackSequence = InteractionView.AttackSequence;
	Request.RequestingSide = InteractionView.ExpectedActingPlayer;
	bPassControlRollCommandInFlight = true;
	SubmitPlayerIntent(TEXT("ResolvePassControlAttackRoll"),
		EMatchPlayAuthoritativeCommandKind::ResolvePassControlAttackRoll,
		Request);
	bPassControlRollCommandInFlight = false;
	RebuildControlSurface();
}

void AFMCodexLocalMatchPlayerController::RollPassControlDefense()
{
	if (bPassControlRollCommandInFlight)
	{
		return;
	}
	if (InteractionView.InteractionCategory
		!= EFMCodexLocalMatchInteractionCategory::RollPassControlDefense)
	{
		RecordLocalFailure(
			TEXT("ResolvePassControlDefenseRoll"),
			TEXT("PassControl defense roll is not the current interaction."));
		return;
	}
	FMatchPlayAuthoritativeResolvePassControlDefenseRollRequest Request;
	Request.AttackSequence = InteractionView.AttackSequence;
	Request.RequestingSide = InteractionView.ExpectedActingPlayer;
	bPassControlRollCommandInFlight = true;
	SubmitPlayerIntent(TEXT("ResolvePassControlDefenseRoll"),
		EMatchPlayAuthoritativeCommandKind::ResolvePassControlDefenseRoll,
		Request);
	bPassControlRollCommandInFlight = false;
	RebuildControlSurface();
}

void AFMCodexLocalMatchPlayerController::RollThroughBallInitialRoute()
{
	if (bThroughBallRouteCommandInFlight)
	{
		return;
	}
	if (InteractionView.InteractionCategory
		!= EFMCodexLocalMatchInteractionCategory::RollThroughBallInitialRoute)
	{
		RecordLocalFailure(
			TEXT("ResolveThroughBallInitialRouteRoll"),
			TEXT("ThroughBall Initial Route roll is not the current interaction."));
		return;
	}
	FMatchPlayAuthoritativeResolveThroughBallInitialRouteRollRequest Request;
	Request.AttackSequence = InteractionView.AttackSequence;
	Request.RequestingSide = InteractionView.ExpectedActingPlayer;
	bThroughBallRouteCommandInFlight = true;
	SubmitPlayerIntent(TEXT("ResolveThroughBallInitialRouteRoll"),
		EMatchPlayAuthoritativeCommandKind::ResolveThroughBallInitialRouteRoll,
		Request);
	bThroughBallRouteCommandInFlight = false;
	RebuildControlSurface();
}

void AFMCodexLocalMatchPlayerController::RollThroughBallFeetAttack()
{
	if (bThroughBallFeetRollCommandInFlight)
	{
		return;
	}
	if (InteractionView.InteractionCategory
		!= EFMCodexLocalMatchInteractionCategory::RollThroughBallFeetAttack)
	{
		RecordLocalFailure(
			TEXT("ResolveThroughBallFeetAttackRoll"),
			TEXT("ThroughBall Feet attack roll is not the current interaction."));
		return;
	}
	FMatchPlayAuthoritativeResolveThroughBallFeetAttackRollRequest Request;
	Request.AttackSequence = InteractionView.AttackSequence;
	Request.RequestingSide = InteractionView.ExpectedActingPlayer;
	bThroughBallFeetRollCommandInFlight = true;
	SubmitPlayerIntent(TEXT("ResolveThroughBallFeetAttackRoll"),
		EMatchPlayAuthoritativeCommandKind::ResolveThroughBallFeetAttackRoll,
		Request);
	bThroughBallFeetRollCommandInFlight = false;
	RebuildControlSurface();
}

void AFMCodexLocalMatchPlayerController::RollThroughBallFeetDefense()
{
	if (bThroughBallFeetRollCommandInFlight)
	{
		return;
	}
	if (InteractionView.InteractionCategory
		!= EFMCodexLocalMatchInteractionCategory::RollThroughBallFeetDefense)
	{
		RecordLocalFailure(
			TEXT("ResolveThroughBallFeetDefenseRoll"),
			TEXT("ThroughBall Feet defense roll is not the current interaction."));
		return;
	}
	FMatchPlayAuthoritativeResolveThroughBallFeetDefenseRollRequest Request;
	Request.AttackSequence = InteractionView.AttackSequence;
	Request.RequestingSide = InteractionView.ExpectedActingPlayer;
	bThroughBallFeetRollCommandInFlight = true;
	SubmitPlayerIntent(TEXT("ResolveThroughBallFeetDefenseRoll"),
		EMatchPlayAuthoritativeCommandKind::ResolveThroughBallFeetDefenseRoll,
		Request);
	bThroughBallFeetRollCommandInFlight = false;
	RebuildControlSurface();
}

void AFMCodexLocalMatchPlayerController::RollThroughBallAntiOffsideAttack()
{
	if (bThroughBallAntiOffsideRollCommandInFlight)
	{
		return;
	}
	if (InteractionView.InteractionCategory
		!= EFMCodexLocalMatchInteractionCategory
			::RollThroughBallAntiOffsideAttack)
	{
		RecordLocalFailure(
			TEXT("ResolveThroughBallAntiOffsideAttackRoll"),
			TEXT("AntiOffside attack roll is not the current interaction."));
		return;
	}
	FMatchPlayAuthoritativeResolveThroughBallAntiOffsideAttackRollRequest
		Request;
	Request.AttackSequence = InteractionView.AttackSequence;
	Request.RequestingSide = InteractionView.ExpectedActingPlayer;
	bThroughBallAntiOffsideRollCommandInFlight = true;
	SubmitPlayerIntent(TEXT("ResolveThroughBallAntiOffsideAttackRoll"),
		EMatchPlayAuthoritativeCommandKind
			::ResolveThroughBallAntiOffsideAttackRoll,
		Request);
	bThroughBallAntiOffsideRollCommandInFlight = false;
	RebuildControlSurface();
}

void AFMCodexLocalMatchPlayerController
	::RollThroughBallOneOnOneChipShotAttack()
{
	if (bThroughBallOneOnOneRollCommandInFlight)
	{
		return;
	}
	if (InteractionView.InteractionCategory
		!= EFMCodexLocalMatchInteractionCategory
			::RollThroughBallOneOnOneChipShotAttack)
	{
		RecordLocalFailure(
			TEXT("ResolveThroughBallOneOnOneChipShotAttackRoll"),
			TEXT("OneOnOne ChipShot attack roll is not the current interaction."));
		return;
	}
	FMatchPlayAuthoritativeResolveThroughBallOneOnOneChipShotAttackRollRequest
		Request;
	Request.AttackSequence = InteractionView.AttackSequence;
	Request.RequestingSide = InteractionView.ExpectedActingPlayer;
	bThroughBallOneOnOneRollCommandInFlight = true;
	SubmitPlayerIntent(TEXT("ResolveThroughBallOneOnOneChipShotAttackRoll"),
		EMatchPlayAuthoritativeCommandKind
			::ResolveThroughBallOneOnOneChipShotAttackRoll,
		Request);
	bThroughBallOneOnOneRollCommandInFlight = false;
	RebuildControlSurface();
}

void AFMCodexLocalMatchPlayerController
	::RollThroughBallOneOnOneDirectShotAttack()
{
	if (bThroughBallOneOnOneRollCommandInFlight)
	{
		return;
	}
	if (InteractionView.InteractionCategory
		!= EFMCodexLocalMatchInteractionCategory
			::RollThroughBallOneOnOneDirectShotAttack)
	{
		RecordLocalFailure(
			TEXT("ResolveThroughBallOneOnOneDirectShotAttackRoll"),
			TEXT("OneOnOne DirectShot attack roll is not the current interaction."));
		return;
	}
	FMatchPlayAuthoritativeResolveThroughBallOneOnOneDirectShotAttackRollRequest
		Request;
	Request.AttackSequence = InteractionView.AttackSequence;
	Request.RequestingSide = InteractionView.ExpectedActingPlayer;
	bThroughBallOneOnOneRollCommandInFlight = true;
	SubmitPlayerIntent(TEXT("ResolveThroughBallOneOnOneDirectShotAttackRoll"),
		EMatchPlayAuthoritativeCommandKind
			::ResolveThroughBallOneOnOneDirectShotAttackRoll,
		Request);
	bThroughBallOneOnOneRollCommandInFlight = false;
	RebuildControlSurface();
}

void AFMCodexLocalMatchPlayerController
	::RollThroughBallOneOnOneDirectShotDefense()
{
	if (bThroughBallOneOnOneRollCommandInFlight)
	{
		return;
	}
	if (InteractionView.InteractionCategory
		!= EFMCodexLocalMatchInteractionCategory
			::RollThroughBallOneOnOneDirectShotDefense)
	{
		RecordLocalFailure(
			TEXT("ResolveThroughBallOneOnOneDirectShotDefenseRoll"),
			TEXT("OneOnOne DirectShot defense roll is not the current interaction."));
		return;
	}
	FMatchPlayAuthoritativeResolveThroughBallOneOnOneDirectShotDefenseRollRequest
		Request;
	Request.AttackSequence = InteractionView.AttackSequence;
	Request.RequestingSide = InteractionView.ExpectedActingPlayer;
	bThroughBallOneOnOneRollCommandInFlight = true;
	SubmitPlayerIntent(TEXT("ResolveThroughBallOneOnOneDirectShotDefenseRoll"),
		EMatchPlayAuthoritativeCommandKind
			::ResolveThroughBallOneOnOneDirectShotDefenseRoll,
		Request);
	bThroughBallOneOnOneRollCommandInFlight = false;
	RebuildControlSurface();
}

void AFMCodexLocalMatchPlayerController
	::RollThroughBallBehindDefenseAttack()
{
	if (bThroughBallBehindDefenseRollCommandInFlight)
	{
		return;
	}
	if (InteractionView.InteractionCategory
		!= EFMCodexLocalMatchInteractionCategory
			::RollThroughBallBehindDefenseAttack)
	{
		RecordLocalFailure(
			TEXT("ResolveThroughBallBehindDefenseP1AttackRoll"),
			TEXT("BehindDefense attack roll is not the current interaction."));
		return;
	}
	FMatchPlayAuthoritativeResolveThroughBallBehindDefenseP1AttackRollRequest
		Request;
	Request.AttackSequence = InteractionView.AttackSequence;
	Request.RequestingSide = InteractionView.ExpectedActingPlayer;
	bThroughBallBehindDefenseRollCommandInFlight = true;
	SubmitPlayerIntent(TEXT("ResolveThroughBallBehindDefenseP1AttackRoll"),
		EMatchPlayAuthoritativeCommandKind
			::ResolveThroughBallBehindDefenseP1AttackRoll,
		Request);
	bThroughBallBehindDefenseRollCommandInFlight = false;
	RebuildControlSurface();
}

void AFMCodexLocalMatchPlayerController
	::RollThroughBallBehindDefenseDefense()
{
	if (bThroughBallBehindDefenseRollCommandInFlight)
	{
		return;
	}
	if (InteractionView.InteractionCategory
		!= EFMCodexLocalMatchInteractionCategory
			::RollThroughBallBehindDefenseDefense)
	{
		RecordLocalFailure(
			TEXT("ResolveThroughBallBehindDefenseP1DefenseRoll"),
			TEXT("BehindDefense defense roll is not the current interaction."));
		return;
	}
	FMatchPlayAuthoritativeResolveThroughBallBehindDefenseP1DefenseRollRequest
		Request;
	Request.AttackSequence = InteractionView.AttackSequence;
	Request.RequestingSide = InteractionView.ExpectedActingPlayer;
	bThroughBallBehindDefenseRollCommandInFlight = true;
	SubmitPlayerIntent(TEXT("ResolveThroughBallBehindDefenseP1DefenseRoll"),
		EMatchPlayAuthoritativeCommandKind
			::ResolveThroughBallBehindDefenseP1DefenseRoll,
		Request);
	bThroughBallBehindDefenseRollCommandInFlight = false;
	RebuildControlSurface();
}

void AFMCodexLocalMatchPlayerController
	::ApplyThroughBallFeetTerminalResolution()
{
	if (InteractionView.InteractionCategory
			!= EFMCodexLocalMatchInteractionCategory
				::ApplyThroughBallFeetTerminalResolution
		|| !InteractionView.bThroughBallFeetFormulaComplete
		|| !InteractionView.bThroughBallFeetTerminalActionAvailable)
	{
		RecordLocalFailure(
			TEXT("ApplyThroughBallTerminalResolution"),
			TEXT("The completed ThroughBall Feet terminal action is not available."));
		return;
	}
	ContinueResolution();
}

void AFMCodexLocalMatchPlayerController::CompleteThroughBallFeetAndAdvance()
{
	ApplyThroughBallFeetTerminalResolution();
}

void AFMCodexLocalMatchPlayerController::AdvanceAfterTerminal()
{
	if (InteractionView.InteractionCategory
			!= EFMCodexLocalMatchInteractionCategory::AdvanceAfterTerminal
		|| !InteractionView.bTerminalPendingAdvance)
	{
		RecordLocalFailure(
			TEXT("AdvanceAfterTerminal"),
			TEXT("The authoritative terminal next-round action is not available."));
		return;
	}
	FMatchPlayAuthoritativeAdvanceAfterTerminalRequest Request;
	Request.AttackSequence = InteractionView.AttackSequence;
	Request.RequestingSide = InteractionView.ExpectedActingPlayer;
	SubmitPlayerIntent(
		TEXT("AdvanceAfterTerminal"),
		EMatchPlayAuthoritativeCommandKind::AdvanceAfterTerminal,
		Request);
}

void AFMCodexLocalMatchPlayerController::SubmitProjectedPrimaryPlayerIntent()
{
	auto SubmitSetPieceRoll = [this](
		const EMatchPlayAuthoritativeCommandKind CommandKind,
		const FString& CommandName)
	{
		FMatchPlaySetPieceTypeRollRequest Request;
		Request.RequestingSide = InteractionView.ExpectedActingPlayer;
		Request.AttackSequence = InteractionView.AttackSequence;
		SubmitPlayerIntent(CommandName, CommandKind, Request);
	};
	auto SubmitCornerRoll = [this](
		const EMatchPlayAuthoritativeCommandKind CommandKind,
		const FString& CommandName)
	{
		FMatchPlayCornerRollRequest Request;
		Request.RequestingSide = InteractionView.ExpectedActingPlayer;
		Request.AttackSequence = InteractionView.AttackSequence;
		SubmitPlayerIntent(CommandName, CommandKind, Request);
	};

	switch (InteractionView.InteractionCategory)
	{
	case EFMCodexLocalMatchInteractionCategory::RollSetPieceType:
		SubmitSetPieceRoll(
			EMatchPlayAuthoritativeCommandKind::RequestSetPieceTypeRoll,
			TEXT("RequestSetPieceTypeRoll"));
		return;
	case EFMCodexLocalMatchInteractionCategory::ConfirmSetPieceCarrier:
	case EFMCodexLocalMatchInteractionCategory::DraftCornerAttacker:
	case EFMCodexLocalMatchInteractionCategory::DraftCornerDefender:
		ConfirmSetPieceDraft();
		return;
	case EFMCodexLocalMatchInteractionCategory::RollShortFreeKickDirectAttack:
	case EFMCodexLocalMatchInteractionCategory::RollShortFreeKickDirectDefense:
	case EFMCodexLocalMatchInteractionCategory::RollShortFreeKickAngled:
	{
		FMatchPlayShortFreeKickRollRequest Request;
		Request.RequestingSide = InteractionView.ExpectedActingPlayer;
		Request.AttackSequence = InteractionView.AttackSequence;
		const auto Category = InteractionView.InteractionCategory;
		SubmitPlayerIntent(
			Category == EFMCodexLocalMatchInteractionCategory
				::RollShortFreeKickDirectAttack
				? TEXT("ResolveShortFreeKickDirectAttackRoll")
				: Category == EFMCodexLocalMatchInteractionCategory
					::RollShortFreeKickDirectDefense
					? TEXT("ResolveShortFreeKickDirectDefenseRoll")
					: TEXT("ResolveShortFreeKickAngledRoll"),
			Category == EFMCodexLocalMatchInteractionCategory
				::RollShortFreeKickDirectAttack
				? EMatchPlayAuthoritativeCommandKind
					::ResolveShortFreeKickDirectAttackRoll
				: Category == EFMCodexLocalMatchInteractionCategory
					::RollShortFreeKickDirectDefense
					? EMatchPlayAuthoritativeCommandKind
						::ResolveShortFreeKickDirectDefenseRoll
					: EMatchPlayAuthoritativeCommandKind
						::ResolveShortFreeKickAngledRoll,
			Request);
		return;
	}
	case EFMCodexLocalMatchInteractionCategory::RollLongFreeKickDirectAttack:
	case EFMCodexLocalMatchInteractionCategory::RollLongFreeKickDirectDefense:
	case EFMCodexLocalMatchInteractionCategory::RollLongFreeKickPower:
	{
		FMatchPlayLongFreeKickRollRequest Request;
		Request.RequestingSide = InteractionView.ExpectedActingPlayer;
		Request.AttackSequence = InteractionView.AttackSequence;
		const auto Category = InteractionView.InteractionCategory;
		SubmitPlayerIntent(
			Category == EFMCodexLocalMatchInteractionCategory
				::RollLongFreeKickDirectAttack
				? TEXT("ResolveLongFreeKickDirectAttackRoll")
				: Category == EFMCodexLocalMatchInteractionCategory
					::RollLongFreeKickDirectDefense
					? TEXT("ResolveLongFreeKickDirectDefenseRoll")
					: TEXT("ResolveLongFreeKickPowerRoll"),
			Category == EFMCodexLocalMatchInteractionCategory
				::RollLongFreeKickDirectAttack
				? EMatchPlayAuthoritativeCommandKind
					::ResolveLongFreeKickDirectAttackRoll
				: Category == EFMCodexLocalMatchInteractionCategory
					::RollLongFreeKickDirectDefense
					? EMatchPlayAuthoritativeCommandKind
						::ResolveLongFreeKickDirectDefenseRoll
					: EMatchPlayAuthoritativeCommandKind
						::ResolveLongFreeKickPowerRoll,
			Request);
		return;
	}
	case EFMCodexLocalMatchInteractionCategory::RollPenaltyDirectAttack:
	case EFMCodexLocalMatchInteractionCategory::RollPenaltyDirectDefense:
	case EFMCodexLocalMatchInteractionCategory::RollPenaltyPanenka:
	{
		FMatchPlayPenaltyRollRequest Request;
		Request.RequestingSide = InteractionView.ExpectedActingPlayer;
		Request.AttackSequence = InteractionView.AttackSequence;
		const auto Category = InteractionView.InteractionCategory;
		SubmitPlayerIntent(
			Category == EFMCodexLocalMatchInteractionCategory
				::RollPenaltyDirectAttack
				? TEXT("ResolvePenaltyDirectAttackRoll")
				: Category == EFMCodexLocalMatchInteractionCategory
					::RollPenaltyDirectDefense
					? TEXT("ResolvePenaltyDirectDefenseRoll")
					: TEXT("ResolvePenaltyPanenkaRoll"),
			Category == EFMCodexLocalMatchInteractionCategory
				::RollPenaltyDirectAttack
				? EMatchPlayAuthoritativeCommandKind
					::ResolvePenaltyDirectAttackRoll
				: Category == EFMCodexLocalMatchInteractionCategory
					::RollPenaltyDirectDefense
					? EMatchPlayAuthoritativeCommandKind
						::ResolvePenaltyDirectDefenseRoll
					: EMatchPlayAuthoritativeCommandKind
						::ResolvePenaltyPanenkaRoll,
			Request);
		return;
	}
	case EFMCodexLocalMatchInteractionCategory::RollCornerParticipantSelection:
		SubmitCornerRoll(
			EMatchPlayAuthoritativeCommandKind
				::RequestCornerParticipantSelectionRoll,
			TEXT("RequestCornerParticipantSelectionRoll"));
		return;
	case EFMCodexLocalMatchInteractionCategory::RollCornerRoute:
		SubmitCornerRoll(
			EMatchPlayAuthoritativeCommandKind::RequestCornerRouteRoll,
			TEXT("RequestCornerRouteRoll"));
		return;
	case EFMCodexLocalMatchInteractionCategory::RollCornerAttack:
		SubmitCornerRoll(
			EMatchPlayAuthoritativeCommandKind::RequestCornerAttackRoll,
			TEXT("RequestCornerAttackRoll"));
		return;
	case EFMCodexLocalMatchInteractionCategory::RollCornerDefense:
		SubmitCornerRoll(
			EMatchPlayAuthoritativeCommandKind::RequestCornerDefenseRoll,
			TEXT("RequestCornerDefenseRoll"));
		return;
	case EFMCodexLocalMatchInteractionCategory::RollCrossRoute:
		RollCrossRoute();
		return;
	case EFMCodexLocalMatchInteractionCategory::RollCrossAttack:
		RollCrossAttack();
		return;
	case EFMCodexLocalMatchInteractionCategory::RollCrossDefense:
		RollCrossDefense();
		return;
	case EFMCodexLocalMatchInteractionCategory::RollLongShotDirectAttack:
		RollLongShotDirectAttack();
		return;
	case EFMCodexLocalMatchInteractionCategory::RollLongShotDirectDefense:
		RollLongShotDirectDefense();
		return;
	case EFMCodexLocalMatchInteractionCategory::RollLongShotDeadCorner:
		RollLongShotDeadCorner();
		return;
	case EFMCodexLocalMatchInteractionCategory::RollCutInsideShotDirectAttack:
		RollCutInsideShotDirectAttack();
		return;
	case EFMCodexLocalMatchInteractionCategory::RollCutInsideShotDirectDefense:
		RollCutInsideShotDirectDefense();
		return;
	case EFMCodexLocalMatchInteractionCategory::RollCutInsideShotDeadCorner:
		RollCutInsideShotDeadCorner();
		return;
	case EFMCodexLocalMatchInteractionCategory::RollPassControlRoute:
		RollPassControlRoute();
		return;
	case EFMCodexLocalMatchInteractionCategory::RollPassControlAttack:
		RollPassControlAttack();
		return;
	case EFMCodexLocalMatchInteractionCategory::RollPassControlDefense:
		RollPassControlDefense();
		return;
	case EFMCodexLocalMatchInteractionCategory::RollThroughBallInitialRoute:
		RollThroughBallInitialRoute();
		return;
	case EFMCodexLocalMatchInteractionCategory::RollThroughBallFeetAttack:
		RollThroughBallFeetAttack();
		return;
	case EFMCodexLocalMatchInteractionCategory::RollThroughBallFeetDefense:
		RollThroughBallFeetDefense();
		return;
	case EFMCodexLocalMatchInteractionCategory
		::RollThroughBallAntiOffsideAttack:
		RollThroughBallAntiOffsideAttack();
		return;
	case EFMCodexLocalMatchInteractionCategory
		::RollThroughBallOneOnOneChipShotAttack:
		RollThroughBallOneOnOneChipShotAttack();
		return;
	case EFMCodexLocalMatchInteractionCategory
		::RollThroughBallOneOnOneDirectShotAttack:
		RollThroughBallOneOnOneDirectShotAttack();
		return;
	case EFMCodexLocalMatchInteractionCategory
		::RollThroughBallOneOnOneDirectShotDefense:
		RollThroughBallOneOnOneDirectShotDefense();
		return;
	case EFMCodexLocalMatchInteractionCategory
		::RollThroughBallBehindDefenseAttack:
		RollThroughBallBehindDefenseAttack();
		return;
	case EFMCodexLocalMatchInteractionCategory
		::RollThroughBallBehindDefenseDefense:
		RollThroughBallBehindDefenseDefense();
		return;
	case EFMCodexLocalMatchInteractionCategory::AdvanceAfterTerminal:
		AdvanceAfterTerminal();
		return;
	default:
		break;
	}

	RecordLocalFailure(
		TEXT("SubmitProjectedPrimaryPlayerIntent"),
		TEXT("No player intent is available for the projected interaction."));
}

void AFMCodexLocalMatchPlayerController::ContinueResolution()
{
	AFMCodexLocalMatchHostGameMode* LocalHost = FindLocalMatchHost();
	if (LocalHost == nullptr)
	{
		RecordLocalFailure(TEXT("ContinueResolution"), TEXT("Host unavailable."));
		return;
	}
	const FMatchPlayServerCoordinatorResult Result =
		LocalHost->AdvanceServerCoordinator();
	if (!Result.bSuccess)
	{
		RecordLocalFailure(TEXT("ContinueResolution"), Result.ErrorMessage);
		return;
	}
	LastDiagnostic.CommandName = TEXT("ContinueResolution");
	LastDiagnostic.bHostSuccess = true;
	LastDiagnostic.bAuthoritativeAccepted = false;
	LastDiagnostic.bAuthoritativeSuccess = true;
	LastDiagnostic.Message = Result.bStateAdvanced
		? TEXT("Server coordinator reached a stable authoritative state.")
		: TEXT("Server coordinator was already at a stable state.");
	RefreshPresentation();
	RebuildControlSurface();
}
void AFMCodexLocalMatchPlayerController::RebuildControlSurface()
{
	RefreshPlayerMatchScreen();
	if (SurfaceContainer.IsValid())
	{
		SurfaceContainer->SetContent(BuildControlSurface());
	}
}

void AFMCodexLocalMatchPlayerController::RefreshPlayerMatchScreen()
{
	if (PlayerMatchScreen == nullptr)
	{
		return;
	}
	const EInitialTurnOrderPlayer LocalViewerSide =
		InteractionView.ExpectedActingPlayer == EInitialTurnOrderPlayer::PlayerA
			|| InteractionView.ExpectedActingPlayer
				== EInitialTurnOrderPlayer::PlayerB
				? InteractionView.ExpectedActingPlayer
				: InteractionView.CurrentAttackingPlayer
					== EInitialTurnOrderPlayer::PlayerA
					|| InteractionView.CurrentAttackingPlayer
						== EInitialTurnOrderPlayer::PlayerB
						? InteractionView.CurrentAttackingPlayer
						: EInitialTurnOrderPlayer::PlayerA;
	PlayerMatchScreen->RefreshFromPresentation(
		FFMCodexLocalMatchUMGPresentationBuilder::Build(
			InteractionView,
			ResolutionFeedback,
			LastDiagnostic.Message,
			LocalViewerSide));
}

TSharedRef<SWidget> AFMCodexLocalMatchPlayerController::BuildControlSurface()
{
	using namespace FMCodexLocalMatchPlayerController;
	const FFMCodexLocalMatchScreenPresentation Screen =
		FFMCodexLocalMatchInteractionViewBuilder::BuildScreenPresentation(
			InteractionView);
	TSharedRef<SVerticalBox> InteractionBody = SNew(SVerticalBox);
	auto AddText = [&InteractionBody](const FString& Text)
	{
		InteractionBody->AddSlot().AutoHeight().Padding(2.0f)
		[
			SNew(STextBlock)
			.Text(FText::FromString(Text))
			.ColorAndOpacity(FLinearColor::White)
			.AutoWrapText(true)
		];
	};
	auto AddButton = [&InteractionBody](const TSharedRef<SWidget>& Button)
	{
		InteractionBody->AddSlot().AutoHeight().Padding(2.0f)[Button];
	};

	const TSharedRef<SWidget> Header =
		SNew(SBorder)
		.Padding(12.0f)
		.BorderBackgroundColor(FLinearColor(0.07f, 0.11f, 0.15f, 1.0f))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1.0f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("MATCH HEADER  |  FMCODEX LOCAL MATCH")))
					.ColorAndOpacity(FLinearColor(1.0f, 0.88f, 0.42f))
				]
				+ SHorizontalBox::Slot().AutoWidth()
				[
					SNew(STextBlock)
					.Text(FText::FromString(Screen.MatchStatusLabel))
					.ColorAndOpacity(FLinearColor(0.58f, 0.86f, 1.0f))
				]
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 10.0f, 0.0f, 5.0f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1.0f).HAlign(HAlign_Right)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("PLAYER A")))
					.ColorAndOpacity(FLinearColor(0.45f, 0.72f, 1.0f))
				]
				+ SHorizontalBox::Slot().AutoWidth().Padding(20.0f, 0.0f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(FString::Printf(
						TEXT("%d   -   %d"),
						InteractionView.PlayerAScore,
						InteractionView.PlayerBScore)))
					.ColorAndOpacity(FLinearColor(1.0f, 0.88f, 0.42f))
				]
				+ SHorizontalBox::Slot().FillWidth(1.0f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("PLAYER B")))
					.ColorAndOpacity(FLinearColor(1.0f, 0.52f, 0.46f))
				]
			]
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(STextBlock)
				.Text(FText::FromString(FString::Printf(
					TEXT("ATTACKER: %s   |   %s"),
					*FFMCodexLocalMatchInteractionViewBuilder::ToString(
						InteractionView.CurrentAttackingPlayer),
					*Screen.ActingStatusLabel)))
				.Justification(ETextJustify::Center)
			]
		];

	TSharedRef<SVerticalBox> FieldBody = SNew(SVerticalBox);
	FieldBody->AddSlot().AutoHeight().Padding(2.0f, 2.0f, 2.0f, 6.0f)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().FillWidth(1.0f)
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("FOOTBALL FIELD")))
			.ColorAndOpacity(FLinearColor(0.72f, 0.94f, 0.74f))
		]
		+ SHorizontalBox::Slot().AutoWidth()
		[
			SNew(STextBlock)
			.Text(FText::FromString(InteractionView.AttackDirectionLabel))
			.ColorAndOpacity(FLinearColor(1.0f, 0.82f, 0.35f))
		]
	];
	TSharedRef<SVerticalBox> Pitch = SNew(SVerticalBox);
	for (int32 RegionIndex = 0;
		RegionIndex < InteractionView.PitchRegions.Num(); ++RegionIndex)
	{
		const FFMCodexLocalMatchPitchRegionView& Region =
			InteractionView.PitchRegions[RegionIndex];
		if (RegionIndex > 0)
		{
			Pitch->AddSlot().AutoHeight().Padding(4.0f, 7.0f)
			[
				SNew(SBorder)
				.Padding(5.0f)
				.BorderBackgroundColor(FLinearColor(0.68f, 0.68f, 0.62f, 0.85f))
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT(
						"CENTER / PHYSICAL HALF BOUNDARY")))
					.Justification(ETextJustify::Center)
					.ColorAndOpacity(FLinearColor(0.08f, 0.10f, 0.09f))
				]
			];
		}

		TSharedRef<SWrapBox> SlotGrid = SNew(SWrapBox)
			.UseAllottedSize(true)
			.InnerSlotPadding(FVector2D(5.0f, 5.0f));
		for (const FFMCodexLocalMatchPitchSlotView& Slot : Region.Slots)
		{
			SlotGrid->AddSlot()
			[
				MakePitchSlot(Slot)
			];
		}

		Pitch->AddSlot().AutoHeight().Padding(2.0f)
		[
			SNew(SBorder)
			.Padding(9.0f)
			.BorderBackgroundColor(
				Region.NeutralSide == EMatchPlayNeutralSlotSide::NearPlayerB
					? FLinearColor(0.11f, 0.20f, 0.12f, 1.0f)
					: FLinearColor(0.045f, 0.17f, 0.13f, 1.0f))
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().FillWidth(1.0f)
					[
						SNew(STextBlock)
						.Text(FText::FromString(Region.Label.ToUpper()))
						.ColorAndOpacity(FLinearColor(0.84f, 0.96f, 0.84f))
					]
					+ SHorizontalBox::Slot().AutoWidth()
					[
						SNew(STextBlock)
						.Text(FText::FromString(Region.bCurrentAttackingSide
							? TEXT("CURRENT ATTACKING SIDE") : TEXT("DEFENDING SIDE")))
						.ColorAndOpacity(Region.bCurrentAttackingSide
							? FLinearColor(1.0f, 0.82f, 0.35f)
							: FLinearColor(0.62f, 0.70f, 0.66f))
					]
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 5.0f, 0.0f, 7.0f)
				[
					SNew(SBorder)
					.Padding(5.0f)
					.BorderBackgroundColor(FLinearColor(0.035f, 0.11f, 0.055f, 1.0f))
					[
						SNew(STextBlock)
						.Text(FText::FromString(
							TEXT("RELATIVE ZONES  |  ") + Region.ZoneContextLabel))
						.Justification(ETextJustify::Center)
					]
				]
				+ SVerticalBox::Slot().AutoHeight()
				[
					SlotGrid
				]
			]
		];
	}
	FieldBody->AddSlot().AutoHeight().Padding(2.0f, 2.0f, 2.0f, 10.0f)
	[
		Pitch
	];

	InteractionBody->AddSlot().AutoHeight().Padding(2.0f, 2.0f, 2.0f, 8.0f)
	[
		SNew(SBorder)
		.Padding(9.0f)
		.BorderBackgroundColor(Screen.bSystemResolution
			? FLinearColor(0.12f, 0.19f, 0.28f, 1.0f)
			: FLinearColor(0.12f, 0.24f, 0.19f, 1.0f))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("CURRENT INTERACTION")))
				.ColorAndOpacity(FLinearColor(0.62f, 0.78f, 0.92f))
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 5.0f, 0.0f, 2.0f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(Screen.InteractionKicker))
				.ColorAndOpacity(FLinearColor(1.0f, 0.88f, 0.42f))
			]
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(STextBlock)
				.Text(FText::FromString(Screen.InteractionTitle))
				.AutoWrapText(true)
			]
		]
	];
	AddText(Screen.bSystemResolution
		? TEXT("The match system has the next step.")
		: Screen.ActingStatusLabel);
	if (InteractionView.bMatchEnded)
	{
		AddText(FString(TEXT("FINAL RESULT: "))
			+ FFMCodexLocalMatchInteractionViewBuilder::ToString(
				InteractionView.MatchResult));
	}

	TSharedRef<SVerticalBox> DiagnosticsBody = SNew(SVerticalBox);
	auto AddDiagnostic = [&DiagnosticsBody](const FString& Text)
	{
		DiagnosticsBody->AddSlot().AutoHeight().Padding(2.0f)
		[
			SNew(STextBlock)
			.Text(FText::FromString(Text))
			.AutoWrapText(true)
			.ColorAndOpacity(FLinearColor(0.64f, 0.69f, 0.74f))
		];
	};
	AddDiagnostic(TEXT("Presentation: ") + LastDiagnostic.PresentationSummary);
	AddDiagnostic(FString::Printf(
		TEXT("Last: %s | Host=%s | Accepted=%s | Domain=%s | %s"),
		*LastDiagnostic.CommandName,
		LastDiagnostic.bHostSuccess ? TEXT("OK") : TEXT("FAIL"),
		LastDiagnostic.bAuthoritativeAccepted ? TEXT("YES") : TEXT("NO"),
		LastDiagnostic.bAuthoritativeSuccess ? TEXT("OK") : TEXT("FAIL"),
		*LastDiagnostic.Message));
	if (!InteractionView.Diagnostic.IsEmpty())
	{
		AddDiagnostic(TEXT("View diagnostic: ") + InteractionView.Diagnostic);
	}
	AddDiagnostic(FString::Printf(
		TEXT("Phase: %s | Interaction: %s | Attack #%lld | AP %d | CurrentAttack: %s"),
		*FFMCodexLocalMatchInteractionViewBuilder::ToString(
			InteractionView.MajorPhase),
		*FFMCodexLocalMatchInteractionViewBuilder::ToString(
			InteractionView.InteractionCategory),
		InteractionView.AttackSequence,
		InteractionView.ActionPoint,
		InteractionView.bCurrentAttackActive ? TEXT("Active") : TEXT("None")));

	for (const FMatchPlayDeploymentPlacement& Placement
		: InteractionView.DeploymentPlacements)
	{
		AddDiagnostic(FString::Printf(
			TEXT("Placed: %s / %s -> %s"),
			*FFMCodexLocalMatchInteractionViewBuilder::ToString(
				Placement.PlayerSide),
			*Placement.CardId.ToString(),
			*Placement.SlotId.ToString()));
	}
	EFMCodexLocalMatchRollGroup LastRollGroup =
		EFMCodexLocalMatchRollGroup::PostRoute;
	bool bHasRollGroup = false;
	for (const FFMCodexLocalMatchRollView& Roll : InteractionView.AcceptedRolls)
	{
		if (!bHasRollGroup || Roll.Group != LastRollGroup)
		{
			FString RollHeading;
			switch (Roll.Group)
			{
			case EFMCodexLocalMatchRollGroup::InitialRoute:
				RollHeading = TEXT("Accepted Dice - Initial Route");
				break;
			case EFMCodexLocalMatchRollGroup::OneOnOne:
				RollHeading = TEXT("Accepted Dice - One-on-One");
				break;
			default:
				RollHeading = TEXT("Accepted Dice - Post-route");
				break;
			}
			AddDiagnostic(RollHeading);
			LastRollGroup = Roll.Group;
			bHasRollGroup = true;
		}
		AddDiagnostic(FString::Printf(
			TEXT("  %s: D6 = %d"), *Roll.Purpose, Roll.RawD6));
	}

	switch (InteractionView.InteractionCategory)
	{
	case EFMCodexLocalMatchInteractionCategory::StartMatch:
	case EFMCodexLocalMatchInteractionCategory::MatchEnded:
		AddButton(MakeButton(
			InteractionView.bMatchEnded
				? TEXT("Start New Local Match")
				: TEXT("Start Local Match"),
			[this]()
		{
			StartNewDemoMatch();
		}));
		break;
	case EFMCodexLocalMatchInteractionCategory::TacticalPointRoll:
		AddButton(MakeButton(TEXT("Roll Tactical Points"), [this]()
		{
			RollDemoTacticalPoints();
		}));
		break;
	case EFMCodexLocalMatchInteractionCategory::Deploy:
		AddText(TEXT("Current Deployment Side: ")
			+ FFMCodexLocalMatchInteractionViewBuilder::ToString(
				InteractionView.CurrentLegalDeploymentSide));
		AddText(TEXT("Ordinary Cards"));
		for (const FFMCodexLocalMatchDeploymentGroup& Group
			: InteractionView.DeploymentGroups)
		{
			if (Group.bGoalkeeper)
			{
				continue;
			}
			InteractionBody->AddSlot().AutoHeight().Padding(3.0f)
			[
				MakeCardPanel(Group.Card, false)
			];
			AddText(TEXT("Legal deployment locations:"));
			TSharedRef<SWrapBox> Slots = SNew(SWrapBox)
				.UseAllottedSize(true)
				.InnerSlotPadding(FVector2D(3.0f, 3.0f));
			for (const FFMCodexLocalMatchSlotView& Slot : Group.LegalSlots)
			{
				const FName SlotId = Slot.SlotId;
				Slots->AddSlot()
				[
					MakeButton(Slot.Label, [this, Group, SlotId]()
					{
						DeployOrdinary(Group.CardId, SlotId);
					})
				];
			}
			AddButton(Slots);
		}
		AddText(TEXT("Goalkeeper"));
		for (const FFMCodexLocalMatchDeploymentGroup& Group
			: InteractionView.DeploymentGroups)
		{
			if (!Group.bGoalkeeper)
			{
				continue;
			}
			InteractionBody->AddSlot().AutoHeight().Padding(3.0f)
			[
				MakeCardPanel(Group.Card, false)
			];
			AddText(TEXT("Legal goalkeeper locations:"));
			TSharedRef<SWrapBox> Slots = SNew(SWrapBox)
				.UseAllottedSize(true)
				.InnerSlotPadding(FVector2D(3.0f, 3.0f));
			for (const FFMCodexLocalMatchSlotView& Slot : Group.LegalSlots)
			{
				const FName SlotId = Slot.SlotId;
				Slots->AddSlot()
				[
					MakeButton(Slot.Label, [this, SlotId]()
					{
						DeployGoalkeeper(SlotId);
					})
				];
			}
			AddButton(Slots);
		}
		AddButton(MakeButton(TEXT("Finish Deployment"), [this]()
		{
			FinishDeployment();
		}));
		break;
	case EFMCodexLocalMatchInteractionCategory::SelectCarrier:
	case EFMCodexLocalMatchInteractionCategory::SelectMarker:
	case EFMCodexLocalMatchInteractionCategory::SelectSkill:
	case EFMCodexLocalMatchInteractionCategory::SelectRunner:
	case EFMCodexLocalMatchInteractionCategory::SelectHelper:
		AddText(FFMCodexLocalMatchInteractionViewBuilder::ToString(
			InteractionView.InteractionCategory)
			+ TEXT(" | Acting Side: ")
			+ FFMCodexLocalMatchInteractionViewBuilder::ToString(
				InteractionView.ExpectedActingPlayer));
		for (const FFMCodexLocalMatchSelectionOption& Option
			: InteractionView.SelectionOptions)
		{
			if (Option.bHasCard)
			{
				InteractionBody->AddSlot().AutoHeight().Padding(3.0f)
				[
					MakeCardPanel(Option.Card)
				];
			}
			const FString Label = FString::Printf(
				TEXT("Choose %s | Side: %s"),
				*Option.Label,
				*FFMCodexLocalMatchInteractionViewBuilder::ToString(Option.Side));
			AddButton(MakeButton(Label, [this, Option]()
			{
				switch (InteractionView.InteractionCategory)
				{
				case EFMCodexLocalMatchInteractionCategory::SelectCarrier:
					SubmitCarrier(Option.Id); break;
				case EFMCodexLocalMatchInteractionCategory::SelectMarker:
					SubmitMarker(Option.Id); break;
				case EFMCodexLocalMatchInteractionCategory::SelectSkill:
					SubmitSkill(Option.Id); break;
				case EFMCodexLocalMatchInteractionCategory::SelectRunner:
					SubmitRunner(Option.Id); break;
				case EFMCodexLocalMatchInteractionCategory::SelectHelper:
					SubmitHelper(Option.Id); break;
				default: break;
				}
			}));
		}
		if (InteractionView.bCanDecline)
		{
			AddButton(MakeButton(TEXT("Decline"), [this]()
			{
				DeclineCurrentSelection();
			}));
		}
		if (InteractionView.bCanResolveNoLegalChoice)
		{
			AddButton(MakeButton(TEXT("Resolve No Legal Choice"), [this]()
			{
				ResolveNoLegalCurrentSelection();
			}));
		}
		break;
	case EFMCodexLocalMatchInteractionCategory::SelectBranchIntent:
	case EFMCodexLocalMatchInteractionCategory::SelectLongShotBranch:
		AddText(TEXT("Branch / Shot Type"));
		for (const EMatchPlayElectiveBranchIntent Intent
			: InteractionView.BranchIntentOptions)
		{
			AddButton(MakeButton(IntentLabel(Intent), [this, Intent]()
			{
				SubmitBranchIntent(Intent);
			}));
		}
		break;
	case EFMCodexLocalMatchInteractionCategory::SelectOneOnOneShot:
		AddText(TEXT("One-on-One Shot Type"));
		for (const EMatchPlayThroughBallOneOnOneShotChoice Choice
			: InteractionView.OneOnOneOptions)
		{
			AddButton(MakeButton(ShotChoiceLabel(Choice), [this, Choice]()
			{
				SubmitOneOnOneShotChoice(Choice);
			}));
		}
		break;
	case EFMCodexLocalMatchInteractionCategory::RollCrossRoute:
		AddButton(MakeButton(InteractionView.ContinueActionLabel, [this]()
		{
			RollCrossRoute();
		}));
		break;
	case EFMCodexLocalMatchInteractionCategory::RollCrossAttack:
		AddButton(MakeButton(InteractionView.ContinueActionLabel, [this]()
		{
			RollCrossAttack();
		}));
		break;
	case EFMCodexLocalMatchInteractionCategory::RollCrossDefense:
		AddButton(MakeButton(InteractionView.ContinueActionLabel, [this]()
		{
			RollCrossDefense();
		}));
		break;
	case EFMCodexLocalMatchInteractionCategory::RollLongShotDirectAttack:
		AddButton(MakeButton(InteractionView.ContinueActionLabel, [this]()
		{
			RollLongShotDirectAttack();
		}));
		break;
	case EFMCodexLocalMatchInteractionCategory::RollLongShotDirectDefense:
		AddButton(MakeButton(InteractionView.ContinueActionLabel, [this]()
		{
			RollLongShotDirectDefense();
		}));
		break;
	case EFMCodexLocalMatchInteractionCategory::RollLongShotDeadCorner:
		AddButton(MakeButton(InteractionView.ContinueActionLabel, [this]()
		{
			RollLongShotDeadCorner();
		}));
		break;
	case EFMCodexLocalMatchInteractionCategory::RollCutInsideShotDirectAttack:
		AddButton(MakeButton(InteractionView.ContinueActionLabel, [this]()
		{
			RollCutInsideShotDirectAttack();
		}));
		break;
	case EFMCodexLocalMatchInteractionCategory::RollCutInsideShotDirectDefense:
		AddButton(MakeButton(InteractionView.ContinueActionLabel, [this]()
		{
			RollCutInsideShotDirectDefense();
		}));
		break;
	case EFMCodexLocalMatchInteractionCategory::RollCutInsideShotDeadCorner:
		AddButton(MakeButton(InteractionView.ContinueActionLabel, [this]()
		{
			RollCutInsideShotDeadCorner();
		}));
		break;
	case EFMCodexLocalMatchInteractionCategory::ApplyCrossTerminalResolution:
		AddButton(MakeButton(InteractionView.ContinueActionLabel, [this]()
		{
			ApplyCrossTerminalResolution();
		}));
		break;
	case EFMCodexLocalMatchInteractionCategory::RollThroughBallFeetAttack:
		AddButton(MakeButton(InteractionView.ContinueActionLabel, [this]()
		{
			RollThroughBallFeetAttack();
		}));
		break;
	case EFMCodexLocalMatchInteractionCategory::RollThroughBallFeetDefense:
		AddButton(MakeButton(InteractionView.ContinueActionLabel, [this]()
		{
			RollThroughBallFeetDefense();
		}));
		break;
	case EFMCodexLocalMatchInteractionCategory
		::RollThroughBallAntiOffsideAttack:
		AddButton(MakeButton(InteractionView.ContinueActionLabel, [this]()
		{
			RollThroughBallAntiOffsideAttack();
		}));
		break;
	case EFMCodexLocalMatchInteractionCategory
		::RollThroughBallOneOnOneChipShotAttack:
		AddButton(MakeButton(InteractionView.ContinueActionLabel, [this]()
		{
			RollThroughBallOneOnOneChipShotAttack();
		}));
		break;
	case EFMCodexLocalMatchInteractionCategory
		::RollThroughBallOneOnOneDirectShotAttack:
		AddButton(MakeButton(InteractionView.ContinueActionLabel, [this]()
		{
			RollThroughBallOneOnOneDirectShotAttack();
		}));
		break;
	case EFMCodexLocalMatchInteractionCategory
		::RollThroughBallOneOnOneDirectShotDefense:
		AddButton(MakeButton(InteractionView.ContinueActionLabel, [this]()
		{
			RollThroughBallOneOnOneDirectShotDefense();
		}));
		break;
	case EFMCodexLocalMatchInteractionCategory
		::RollThroughBallBehindDefenseAttack:
		AddButton(MakeButton(InteractionView.ContinueActionLabel, [this]()
		{
			RollThroughBallBehindDefenseAttack();
		}));
		break;
	case EFMCodexLocalMatchInteractionCategory
		::RollThroughBallBehindDefenseDefense:
		AddButton(MakeButton(InteractionView.ContinueActionLabel, [this]()
		{
			RollThroughBallBehindDefenseDefense();
		}));
		break;
	case EFMCodexLocalMatchInteractionCategory
		::ApplyThroughBallFeetTerminalResolution:
		AddButton(MakeButton(InteractionView.ContinueActionLabel, [this]()
		{
			ApplyThroughBallFeetTerminalResolution();
		}));
		break;
	case EFMCodexLocalMatchInteractionCategory::AdvanceAfterTerminal:
		AddButton(MakeButton(InteractionView.ContinueActionLabel, [this]()
		{
			AdvanceAfterTerminal();
		}));
		break;
	case EFMCodexLocalMatchInteractionCategory::ContinueResolution:
		AddText(TEXT("正在同步比赛进程…"));
		break;
	default:
		break;
	}

	TSharedRef<SWidget> ResultPanel =
		SNew(SBorder)
			.Padding(10.0f)
			.BorderBackgroundColor(FLinearColor(0.05f, 0.09f, 0.12f, 1.0f))
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT(
					"RESOLUTION RESULT  |  No resolution result yet")))
				.ColorAndOpacity(FLinearColor(0.54f, 0.62f, 0.70f))
			];
	if (ResolutionFeedback.bVisible)
	{
		ResultPanel = MakeFeedbackPanel(ResolutionFeedback);
	}

	return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(2.0f, 2.0f, 2.0f, 8.0f)
		[
			Header
		]
		+ SVerticalBox::Slot().FillHeight(1.0f).Padding(2.0f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(1.45f).Padding(0.0f, 0.0f, 4.0f, 0.0f)
			[
				SNew(SBorder)
				.Padding(8.0f)
				.BorderBackgroundColor(FLinearColor(0.025f, 0.10f, 0.045f, 1.0f))
				[
					SNew(SScrollBox)
					+ SScrollBox::Slot()
					[
						FieldBody
					]
				]
			]
			+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(4.0f, 0.0f, 0.0f, 0.0f)
			[
				SNew(SBorder)
				.Padding(8.0f)
				.BorderBackgroundColor(FLinearColor(0.05f, 0.07f, 0.09f, 1.0f))
				[
					SNew(SScrollBox)
					.ScrollBarAlwaysVisible(true)
					+ SScrollBox::Slot()
					[
						InteractionBody
					]
				]
			]
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(2.0f, 8.0f, 2.0f, 2.0f)
		[
			ResultPanel
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(2.0f)
		[
			SNew(SExpandableArea)
			.InitiallyCollapsed(true)
			.HeaderContent()
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("Developer Details")))
				.ColorAndOpacity(FLinearColor(0.54f, 0.60f, 0.66f))
			]
			.BodyContent()
			[
				DiagnosticsBody
			]
		];
}
