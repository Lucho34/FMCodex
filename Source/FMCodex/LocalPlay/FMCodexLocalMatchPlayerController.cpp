#include "FMCodexLocalMatchPlayerController.h"

#include "FMCodexLocalMatchDemoConfiguration.h"
#include "FMCodexLocalMatchHostGameMode.h"
#include "FMCodexLocalMatchScreenWidget.h"
#include "FMCodexLocalMatchUMGPresentation.h"
#if !UE_BUILD_SHIPPING
#include "FMCodexLocalDevRollOverrideWidget.h"
#endif

#include "../CoreRules/MatchPlayCurrentAttackPostRouteRollProgressQuery.h"
#include "../CoreRules/MatchPlayCurrentAttackResolveThroughBallBehindDefenseP1FormulaOrchestrator.h"

#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "EngineUtils.h"
#endif
#include "Engine/World.h"
#include "Framework/Application/SlateApplication.h"
#include "GameFramework/GameModeBase.h"
#include "Styling/CoreStyle.h"
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

void AFMCodexLocalMatchPlayerController::RefreshPresentation()
{
	AFMCodexLocalMatchHostGameMode* Host = FindLocalMatchHost();
	if (Host == nullptr || !Host->HasActiveLocalMatch())
	{
		const auto NewView =
			FFMCodexLocalMatchInteractionViewBuilder::BuildNoActiveMatch();
		InteractionView = NewView;
		RebuildControlSurface();
		return;
	}

	const FFMCodexLocalMatchSnapshotResult State = Host->GetMatchSnapshot();
	const FFMCodexLocalMatchSkillRuleSnapshotResult Rules =
		Host->GetSkillRuleSnapshot();
	if (!State.bSuccess || !Rules.bSuccess)
	{
		auto NewView =
			FFMCodexLocalMatchInteractionViewBuilder::BuildNoActiveMatch();
		NewView.bMatchActive = true;
		NewView.InteractionCategory =
			EFMCodexLocalMatchInteractionCategory::None;
		NewView.Diagnostic = !State.bSuccess
			? State.ErrorMessage
			: Rules.ErrorMessage;
		InteractionView = MoveTemp(NewView);
		RebuildControlSurface();
		return;
	}

	auto NewView = FFMCodexLocalMatchInteractionViewBuilder::Build(
		State.Snapshot, Rules.Snapshot);
	InteractionView = MoveTemp(NewView);
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
	if (InteractionView.InteractionCategory
			!= EFMCodexLocalMatchInteractionCategory::SelectHelper
		|| InteractionView.bCanDecline
		|| !InteractionView.bCanResolveNoLegalChoice)
	{
		return;
	}

	AFMCodexLocalMatchHostGameMode* Host = FindLocalMatchHost();
	if (Host == nullptr)
	{
		RecordLocalFailure(
			TEXT("ResolveNoLegalHelper"), TEXT("Host unavailable."));
		return;
	}

	RecordCommandResult(
		TEXT("ResolveNoLegalHelper"), Host->ResolveNoLegalHelper());
}

void AFMCodexLocalMatchPlayerController::RecordLocalFailure(
	const FString& CommandName,
	const FString& Message)
{
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

void AFMCodexLocalMatchPlayerController::StartNewDemoMatch()
{
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
	AFMCodexLocalMatchHostGameMode* Host = FindLocalMatchHost();
	if (Host == nullptr)
	{
		RecordLocalFailure(TEXT("RollTacticalPoints"), TEXT("Host unavailable."));
		return;
	}
	RecordCommandResult(
		TEXT("RollTacticalPoints"),
		Host->RollTacticalPoints(InteractionView.ExpectedActingPlayer));
}

void AFMCodexLocalMatchPlayerController::DeployOrdinary(
	const FName CardId,
	const FName SlotId)
{
	AFMCodexLocalMatchHostGameMode* Host = FindLocalMatchHost();
	if (Host == nullptr)
	{
		RecordLocalFailure(TEXT("DeployOrdinary"), TEXT("Host unavailable."));
		return;
	}
	FMatchPlayAuthoritativeDeployOrdinaryRequest Request;
	Request.RequestingSide = InteractionView.ExpectedActingPlayer;
	Request.CardId = CardId;
	Request.SlotId = SlotId;
	RecordCommandResult(TEXT("DeployOrdinary"), Host->DeployOrdinary(Request));
}

void AFMCodexLocalMatchPlayerController::DeployGoalkeeper(const FName SlotId)
{
	AFMCodexLocalMatchHostGameMode* Host = FindLocalMatchHost();
	if (Host == nullptr)
	{
		RecordLocalFailure(TEXT("DeployGoalkeeper"), TEXT("Host unavailable."));
		return;
	}
	FMatchPlayAuthoritativeDeployGoalkeeperRequest Request;
	Request.SlotId = SlotId;
	RecordCommandResult(
		TEXT("DeployGoalkeeper"), Host->DeployGoalkeeper(Request));
}

void AFMCodexLocalMatchPlayerController::FinishDeployment()
{
	AFMCodexLocalMatchHostGameMode* Host = FindLocalMatchHost();
	if (Host == nullptr)
	{
		RecordLocalFailure(TEXT("FinishDeployment"), TEXT("Host unavailable."));
		return;
	}
	RecordCommandResult(
		TEXT("FinishDeployment"),
		Host->FinishDeployment(
			InteractionView.AttackSequence,
			InteractionView.ExpectedActingPlayer));
}

void AFMCodexLocalMatchPlayerController::SubmitCarrier(const FName CardId)
{
	AFMCodexLocalMatchHostGameMode* Host = FindLocalMatchHost();
	if (Host == nullptr)
	{
		RecordLocalFailure(TEXT("SubmitCarrier"), TEXT("Host unavailable."));
		return;
	}
	FMatchPlayAuthoritativeSubmitCarrierRequest Request;
	Request.RequestingSide = InteractionView.ExpectedActingPlayer;
	Request.CarrierCardId = CardId;
	RecordCommandResult(TEXT("SubmitCarrier"), Host->SubmitCarrier(Request));
}

void AFMCodexLocalMatchPlayerController::SubmitMarker(const FName CardId)
{
	AFMCodexLocalMatchHostGameMode* Host = FindLocalMatchHost();
	if (Host == nullptr)
	{
		RecordLocalFailure(TEXT("SubmitMarker"), TEXT("Host unavailable."));
		return;
	}
	FMatchPlayAuthoritativeSubmitMarkerRequest Request;
	Request.RequestingSide = InteractionView.ExpectedActingPlayer;
	Request.MarkerCardId = CardId;
	RecordCommandResult(TEXT("SubmitMarker"), Host->SubmitMarker(Request));
}

void AFMCodexLocalMatchPlayerController::SubmitSkill(const FName SkillId)
{
	AFMCodexLocalMatchHostGameMode* Host = FindLocalMatchHost();
	if (Host == nullptr)
	{
		RecordLocalFailure(TEXT("SubmitSkill"), TEXT("Host unavailable."));
		return;
	}
	const auto Rules = Host->GetSkillRuleSnapshot();
	if (!Rules.bSuccess)
	{
		RecordLocalFailure(TEXT("SubmitSkill"), Rules.ErrorMessage);
		return;
	}
	FMatchPlayAuthoritativeSubmitSkillRequest Request;
	Request.RequestingSide = InteractionView.ExpectedActingPlayer;
	Request.SkillId = SkillId;
	RecordCommandResult(
		TEXT("SubmitSkill"), Host->SubmitSkill(Rules.Snapshot, Request));
}

void AFMCodexLocalMatchPlayerController::SubmitRunner(const FName CardId)
{
	AFMCodexLocalMatchHostGameMode* Host = FindLocalMatchHost();
	if (Host == nullptr)
	{
		RecordLocalFailure(TEXT("SubmitRunner"), TEXT("Host unavailable."));
		return;
	}
	FMatchPlayAuthoritativeSubmitRunnerRequest Request;
	Request.RequestingSide = InteractionView.ExpectedActingPlayer;
	Request.RunnerCardId = CardId;
	RecordCommandResult(TEXT("SubmitRunner"), Host->SubmitRunner(Request));
	if (LastDiagnostic.bHostSuccess)
	{
		ResolveAutomaticNoLegalHelperIfNeeded();
	}
}

void AFMCodexLocalMatchPlayerController::SubmitHelper(const FName CardId)
{
	AFMCodexLocalMatchHostGameMode* Host = FindLocalMatchHost();
	if (Host == nullptr)
	{
		RecordLocalFailure(TEXT("SubmitHelper"), TEXT("Host unavailable."));
		return;
	}
	FMatchPlayAuthoritativeSubmitHelperRequest Request;
	Request.RequestingSide = InteractionView.ExpectedActingPlayer;
	Request.HelperCardId = CardId;
	RecordCommandResult(TEXT("SubmitHelper"), Host->SubmitHelper(Request));
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
	AFMCodexLocalMatchHostGameMode* Host = FindLocalMatchHost();
	if (Host == nullptr)
	{
		RecordLocalFailure(TEXT("DeclineSelection"), TEXT("Host unavailable."));
		return;
	}

	switch (InteractionView.InteractionCategory)
	{
	case EFMCodexLocalMatchInteractionCategory::SelectMarker:
	{
		FMatchPlayAuthoritativeDeclineMarkerRequest Request;
		Request.RequestingSide = InteractionView.ExpectedActingPlayer;
		RecordCommandResult(TEXT("DeclineMarker"), Host->DeclineMarker(Request));
		break;
	}
	case EFMCodexLocalMatchInteractionCategory::SelectSkill:
	{
		const auto Rules = Host->GetSkillRuleSnapshot();
		if (!Rules.bSuccess)
		{
			RecordLocalFailure(TEXT("DeclineSkill"), Rules.ErrorMessage);
			return;
		}
		FMatchPlayAuthoritativeDeclineSkillRequest Request;
		Request.RequestingSide = InteractionView.ExpectedActingPlayer;
		RecordCommandResult(
			TEXT("DeclineSkill"),
			Host->DeclineSkill(Rules.Snapshot, Request));
		break;
	}
	case EFMCodexLocalMatchInteractionCategory::SelectRunner:
	{
		FMatchPlayAuthoritativeDeclineRunnerRequest Request;
		Request.RequestingSide = InteractionView.ExpectedActingPlayer;
		RecordCommandResult(TEXT("DeclineRunner"), Host->DeclineRunner(Request));
		break;
	}
	case EFMCodexLocalMatchInteractionCategory::SelectHelper:
	{
		FMatchPlayAuthoritativeDeclineHelperRequest Request;
		Request.RequestingSide = InteractionView.ExpectedActingPlayer;
		RecordCommandResult(TEXT("DeclineHelper"), Host->DeclineHelper(Request));
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
	AFMCodexLocalMatchHostGameMode* Host = FindLocalMatchHost();
	if (Host == nullptr)
	{
		RecordLocalFailure(TEXT("ResolveNoLegalSelection"), TEXT("Host unavailable."));
		return;
	}

	switch (InteractionView.InteractionCategory)
	{
	case EFMCodexLocalMatchInteractionCategory::SelectCarrier:
		RecordCommandResult(
			TEXT("ResolveNoLegalCarrier"), Host->ResolveNoLegalCarrier());
		break;
	case EFMCodexLocalMatchInteractionCategory::SelectMarker:
		RecordCommandResult(
			TEXT("ResolveNoLegalMarker"), Host->ResolveNoLegalMarker());
		break;
	case EFMCodexLocalMatchInteractionCategory::SelectSkill:
	{
		const auto Rules = Host->GetSkillRuleSnapshot();
		if (!Rules.bSuccess)
		{
			RecordLocalFailure(TEXT("ResolveNoLegalSkill"), Rules.ErrorMessage);
			return;
		}
		RecordCommandResult(
			TEXT("ResolveNoLegalSkill"),
			Host->ResolveNoLegalSkill(Rules.Snapshot));
		break;
	}
	case EFMCodexLocalMatchInteractionCategory::SelectRunner:
		RecordCommandResult(
			TEXT("ResolveNoLegalRunner"), Host->ResolveNoLegalRunner());
		break;
	case EFMCodexLocalMatchInteractionCategory::SelectHelper:
		RecordCommandResult(
			TEXT("ResolveNoLegalHelper"), Host->ResolveNoLegalHelper());
		break;
	default:
		RecordLocalFailure(
			TEXT("ResolveNoLegalSelection"),
			TEXT("No authoritative no-legal operation is available now."));
		break;
	}
}

void AFMCodexLocalMatchPlayerController::SubmitBranchIntent(
	const EMatchPlayElectiveBranchIntent Intent)
{
	AFMCodexLocalMatchHostGameMode* Host = FindLocalMatchHost();
	if (Host == nullptr)
	{
		RecordLocalFailure(TEXT("SubmitBranchIntent"), TEXT("Host unavailable."));
		return;
	}
	FMatchPlayAuthoritativeSubmitBranchIntentRequest Request;
	Request.AttackSequence = InteractionView.AttackSequence;
	Request.RequestingSide = InteractionView.ExpectedActingPlayer;
	Request.Intent = Intent;
	const auto Result = Host->SubmitBranchIntent(Request);
	RecordCommandResult(TEXT("SubmitBranchIntent"), Result);
	if (Result.bSuccess
		&& (InteractionView.PresentedActionType == ESkillRuleType::LongShot
			|| InteractionView.PresentedActionType
				== ESkillRuleType::CutInsideShot))
	{
		// Shot session creation and intent-determined route consume no RNG and
		// have no player choice. Keep both behind the selected branch click.
		if (InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::ContinueResolution)
		{
			ContinueResolution();
		}
		if (LastDiagnostic.bHostSuccess
			&& InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::ContinueResolution)
		{
			ContinueResolution();
		}
	}
}

void AFMCodexLocalMatchPlayerController::SubmitOneOnOneShotChoice(
	const EMatchPlayThroughBallOneOnOneShotChoice Choice)
{
	AFMCodexLocalMatchHostGameMode* Host = FindLocalMatchHost();
	if (Host == nullptr)
	{
		RecordLocalFailure(
			TEXT("SubmitThroughBallOneOnOneShotChoice"),
			TEXT("Host unavailable."));
		return;
	}
	FMatchPlayAuthoritativeSubmitThroughBallOneOnOneShotChoiceRequest Request;
	Request.RequestingSide = InteractionView.ExpectedActingPlayer;
	Request.Choice = Choice;
	RecordCommandResult(
		TEXT("SubmitThroughBallOneOnOneShotChoice"),
		Host->SubmitThroughBallOneOnOneShotChoice(Request));
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
	AFMCodexLocalMatchHostGameMode* Host = FindLocalMatchHost();
	if (Host == nullptr)
	{
		RecordLocalFailure(
			TEXT("ResolveCrossAttackRoll"), TEXT("Host unavailable."));
		return;
	}
	const bool bHigh = InteractionView.ResolutionFacts.bHasActualBranch
		&& InteractionView.ResolutionFacts.ActualBranch.Cross
			== EMatchPlayCrossActualBranch::High;
	bCrossRollCommandInFlight = true;
	if (bHigh)
	{
		FMatchPlayAuthoritativeResolveCrossHighAttackRollRequest Request;
		Request.RequestingSide = InteractionView.ExpectedActingPlayer;
		const auto Result = Host->ResolveCrossHighAttackRoll(Request);
		bCrossRollCommandInFlight = false;
		RecordCommandResult(TEXT("ResolveCrossHighAttackRoll"), Result);
	}
	else
	{
		FMatchPlayAuthoritativeResolveCrossLowAttackRollRequest Request;
		Request.RequestingSide = InteractionView.ExpectedActingPlayer;
		const auto Result = Host->ResolveCrossLowAttackRoll(Request);
		bCrossRollCommandInFlight = false;
		RecordCommandResult(TEXT("ResolveCrossLowAttackRoll"), Result);
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
	AFMCodexLocalMatchHostGameMode* Host = FindLocalMatchHost();
	if (Host == nullptr)
	{
		RecordLocalFailure(
			TEXT("ResolveCrossDefenseRoll"), TEXT("Host unavailable."));
		return;
	}
	const bool bHigh = InteractionView.ResolutionFacts.bHasActualBranch
		&& InteractionView.ResolutionFacts.ActualBranch.Cross
			== EMatchPlayCrossActualBranch::High;
	bCrossRollCommandInFlight = true;
	if (bHigh)
	{
		FMatchPlayAuthoritativeResolveCrossHighDefenseRollRequest Request;
		Request.RequestingSide = InteractionView.ExpectedActingPlayer;
		const auto Result = Host->ResolveCrossHighDefenseRoll(Request);
		bCrossRollCommandInFlight = false;
		RecordCommandResult(TEXT("ResolveCrossHighDefenseRoll"), Result);
		if (Result.bSuccess
			&& InteractionView.InteractionCategory
				== EFMCodexLocalMatchInteractionCategory
					::ApplyCrossTerminalResolution)
		{
			ApplyCrossTerminalResolution();
		}
	}
	else
	{
		FMatchPlayAuthoritativeResolveCrossLowDefenseRollRequest Request;
		Request.RequestingSide = InteractionView.ExpectedActingPlayer;
		const auto Result = Host->ResolveCrossLowDefenseRoll(Request);
		bCrossRollCommandInFlight = false;
		RecordCommandResult(TEXT("ResolveCrossLowDefenseRoll"), Result);
		if (Result.bSuccess
			&& InteractionView.InteractionCategory
				== EFMCodexLocalMatchInteractionCategory
					::ApplyCrossTerminalResolution)
		{
			ApplyCrossTerminalResolution();
		}
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
	AFMCodexLocalMatchHostGameMode* Host = FindLocalMatchHost();
	if (Host == nullptr)
	{
		RecordLocalFailure(
			TEXT("ApplyCrossTerminalResolution"), TEXT("Host unavailable."));
		return;
	}
	RecordCommandResult(
		TEXT("ApplyCrossTerminalResolution"),
		Host->ApplyCrossTerminalResolution());
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
	AFMCodexLocalMatchHostGameMode* Host = FindLocalMatchHost();
	if (Host == nullptr)
	{
		RecordLocalFailure(
			TEXT("ResolveLongShotDirectAttackRoll"), TEXT("Host unavailable."));
		return;
	}
	FMatchPlayAuthoritativeResolveLongShotDirectAttackRollRequest Request;
	Request.AttackSequence = InteractionView.AttackSequence;
	Request.RequestingSide = InteractionView.ExpectedActingPlayer;
	bLongShotRollCommandInFlight = true;
	const auto Result = Host->ResolveLongShotDirectAttackRoll(Request);
	bLongShotRollCommandInFlight = false;
	RecordCommandResult(TEXT("ResolveLongShotDirectAttackRoll"), Result);
	if (Result.bSuccess && InteractionView.InteractionCategory
		== EFMCodexLocalMatchInteractionCategory::ContinueResolution)
	{
		ContinueResolution();
	}
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
	AFMCodexLocalMatchHostGameMode* Host = FindLocalMatchHost();
	if (Host == nullptr)
	{
		RecordLocalFailure(
			TEXT("ResolveLongShotDirectDefenseRoll"), TEXT("Host unavailable."));
		return;
	}
	FMatchPlayAuthoritativeResolveLongShotDirectDefenseRollRequest Request;
	Request.AttackSequence = InteractionView.AttackSequence;
	Request.RequestingSide = InteractionView.ExpectedActingPlayer;
	bLongShotRollCommandInFlight = true;
	const auto Result = Host->ResolveLongShotDirectDefenseRoll(Request);
	bLongShotRollCommandInFlight = false;
	RecordCommandResult(TEXT("ResolveLongShotDirectDefenseRoll"), Result);
	if (Result.bSuccess && InteractionView.InteractionCategory
		== EFMCodexLocalMatchInteractionCategory::ContinueResolution)
	{
		ContinueResolution();
	}
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
	AFMCodexLocalMatchHostGameMode* Host = FindLocalMatchHost();
	if (Host == nullptr)
	{
		RecordLocalFailure(
			TEXT("ResolveLongShotDeadCornerRoll"), TEXT("Host unavailable."));
		return;
	}
	FMatchPlayAuthoritativeResolveLongShotDeadCornerRollRequest Request;
	Request.AttackSequence = InteractionView.AttackSequence;
	Request.RequestingSide = InteractionView.ExpectedActingPlayer;
	bLongShotRollCommandInFlight = true;
	const auto Result = Host->ResolveLongShotDeadCornerRoll(Request);
	bLongShotRollCommandInFlight = false;
	RecordCommandResult(TEXT("ResolveLongShotDeadCornerRoll"), Result);
	if (Result.bSuccess && InteractionView.InteractionCategory
		== EFMCodexLocalMatchInteractionCategory::ContinueResolution)
	{
		ContinueResolution();
	}
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
	AFMCodexLocalMatchHostGameMode* Host = FindLocalMatchHost();
	if (Host == nullptr)
	{
		RecordLocalFailure(
			TEXT("ResolveCutInsideShotDirectAttackRoll"),
			TEXT("Host unavailable."));
		return;
	}
	FMatchPlayAuthoritativeResolveCutInsideShotDirectAttackRollRequest Request;
	Request.AttackSequence = InteractionView.AttackSequence;
	Request.RequestingSide = InteractionView.ExpectedActingPlayer;
	bCutInsideShotRollCommandInFlight = true;
	const auto Result = Host->ResolveCutInsideShotDirectAttackRoll(Request);
	bCutInsideShotRollCommandInFlight = false;
	RecordCommandResult(TEXT("ResolveCutInsideShotDirectAttackRoll"), Result);
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
	AFMCodexLocalMatchHostGameMode* Host = FindLocalMatchHost();
	if (Host == nullptr)
	{
		RecordLocalFailure(
			TEXT("ResolveCutInsideShotDirectDefenseRoll"),
			TEXT("Host unavailable."));
		return;
	}
	FMatchPlayAuthoritativeResolveCutInsideShotDirectDefenseRollRequest Request;
	Request.AttackSequence = InteractionView.AttackSequence;
	Request.RequestingSide = InteractionView.ExpectedActingPlayer;
	bCutInsideShotRollCommandInFlight = true;
	const auto Result = Host->ResolveCutInsideShotDirectDefenseRoll(Request);
	bCutInsideShotRollCommandInFlight = false;
	RecordCommandResult(TEXT("ResolveCutInsideShotDirectDefenseRoll"), Result);
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
	AFMCodexLocalMatchHostGameMode* Host = FindLocalMatchHost();
	if (Host == nullptr)
	{
		RecordLocalFailure(
			TEXT("ResolveCutInsideShotDeadCornerRoll"),
			TEXT("Host unavailable."));
		return;
	}
	FMatchPlayAuthoritativeResolveCutInsideShotDeadCornerRollRequest Request;
	Request.AttackSequence = InteractionView.AttackSequence;
	Request.RequestingSide = InteractionView.ExpectedActingPlayer;
	bCutInsideShotRollCommandInFlight = true;
	const auto Result = Host->ResolveCutInsideShotDeadCornerRoll(Request);
	bCutInsideShotRollCommandInFlight = false;
	RecordCommandResult(TEXT("ResolveCutInsideShotDeadCornerRoll"), Result);
}

void AFMCodexLocalMatchPlayerController::CompleteCrossAndAdvance()
{
	ApplyCrossTerminalResolution();
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
	AFMCodexLocalMatchHostGameMode* Host = FindLocalMatchHost();
	if (Host == nullptr)
	{
		RecordLocalFailure(
			TEXT("ResolveThroughBallInitialRouteRoll"),
			TEXT("Host unavailable."));
		return;
	}
	FMatchPlayAuthoritativeResolveThroughBallInitialRouteRollRequest Request;
	Request.AttackSequence = InteractionView.AttackSequence;
	Request.RequestingSide = InteractionView.ExpectedActingPlayer;
	bThroughBallRouteCommandInFlight = true;
	const auto Result = Host->ResolveThroughBallInitialRouteRoll(Request);
	bThroughBallRouteCommandInFlight = false;
	RecordCommandResult(TEXT("ResolveThroughBallInitialRouteRoll"), Result);
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
	AFMCodexLocalMatchHostGameMode* Host = FindLocalMatchHost();
	if (Host == nullptr)
	{
		RecordLocalFailure(
			TEXT("ResolveThroughBallFeetAttackRoll"),
			TEXT("Host unavailable."));
		return;
	}
	FMatchPlayAuthoritativeResolveThroughBallFeetAttackRollRequest Request;
	Request.AttackSequence = InteractionView.AttackSequence;
	Request.RequestingSide = InteractionView.ExpectedActingPlayer;
	bThroughBallFeetRollCommandInFlight = true;
	const auto Result = Host->ResolveThroughBallFeetAttackRoll(Request);
	bThroughBallFeetRollCommandInFlight = false;
	RecordCommandResult(TEXT("ResolveThroughBallFeetAttackRoll"), Result);
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
	AFMCodexLocalMatchHostGameMode* Host = FindLocalMatchHost();
	if (Host == nullptr)
	{
		RecordLocalFailure(
			TEXT("ResolveThroughBallFeetDefenseRoll"),
			TEXT("Host unavailable."));
		return;
	}
	FMatchPlayAuthoritativeResolveThroughBallFeetDefenseRollRequest Request;
	Request.AttackSequence = InteractionView.AttackSequence;
	Request.RequestingSide = InteractionView.ExpectedActingPlayer;
	bThroughBallFeetRollCommandInFlight = true;
	const auto Result = Host->ResolveThroughBallFeetDefenseRoll(Request);
	bThroughBallFeetRollCommandInFlight = false;
	RecordCommandResult(TEXT("ResolveThroughBallFeetDefenseRoll"), Result);
	if (Result.bSuccess
		&& InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory
				::ApplyThroughBallFeetTerminalResolution)
	{
		ApplyThroughBallFeetTerminalResolution();
	}
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
	AFMCodexLocalMatchHostGameMode* Host = FindLocalMatchHost();
	if (Host == nullptr)
	{
		RecordLocalFailure(
			TEXT("ResolveThroughBallAntiOffsideAttackRoll"),
			TEXT("Host unavailable."));
		return;
	}
	FMatchPlayAuthoritativeResolveThroughBallAntiOffsideAttackRollRequest
		Request;
	Request.AttackSequence = InteractionView.AttackSequence;
	Request.RequestingSide = InteractionView.ExpectedActingPlayer;
	bThroughBallAntiOffsideRollCommandInFlight = true;
	const auto Result = Host->ResolveThroughBallAntiOffsideAttackRoll(Request);
	bThroughBallAntiOffsideRollCommandInFlight = false;
	RecordCommandResult(
		TEXT("ResolveThroughBallAntiOffsideAttackRoll"), Result);
	if (Result.bSuccess
		&& InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::ContinueResolution)
	{
		ContinueResolution();
	}
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
	AFMCodexLocalMatchHostGameMode* Host = FindLocalMatchHost();
	if (Host == nullptr)
	{
		RecordLocalFailure(
			TEXT("ResolveThroughBallOneOnOneChipShotAttackRoll"),
			TEXT("Host unavailable."));
		return;
	}
	FMatchPlayAuthoritativeResolveThroughBallOneOnOneChipShotAttackRollRequest
		Request;
	Request.AttackSequence = InteractionView.AttackSequence;
	Request.RequestingSide = InteractionView.ExpectedActingPlayer;
	bThroughBallOneOnOneRollCommandInFlight = true;
	const auto Result =
		Host->ResolveThroughBallOneOnOneChipShotAttackRoll(Request);
	bThroughBallOneOnOneRollCommandInFlight = false;
	RecordCommandResult(
		TEXT("ResolveThroughBallOneOnOneChipShotAttackRoll"), Result);
	if (Result.bSuccess
		&& InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::ContinueResolution)
	{
		ContinueResolution();
	}
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
	AFMCodexLocalMatchHostGameMode* Host = FindLocalMatchHost();
	if (Host == nullptr)
	{
		RecordLocalFailure(
			TEXT("ResolveThroughBallOneOnOneDirectShotAttackRoll"),
			TEXT("Host unavailable."));
		return;
	}
	FMatchPlayAuthoritativeResolveThroughBallOneOnOneDirectShotAttackRollRequest
		Request;
	Request.AttackSequence = InteractionView.AttackSequence;
	Request.RequestingSide = InteractionView.ExpectedActingPlayer;
	bThroughBallOneOnOneRollCommandInFlight = true;
	const auto Result =
		Host->ResolveThroughBallOneOnOneDirectShotAttackRoll(Request);
	bThroughBallOneOnOneRollCommandInFlight = false;
	RecordCommandResult(
		TEXT("ResolveThroughBallOneOnOneDirectShotAttackRoll"), Result);
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
	AFMCodexLocalMatchHostGameMode* Host = FindLocalMatchHost();
	if (Host == nullptr)
	{
		RecordLocalFailure(
			TEXT("ResolveThroughBallOneOnOneDirectShotDefenseRoll"),
			TEXT("Host unavailable."));
		return;
	}
	FMatchPlayAuthoritativeResolveThroughBallOneOnOneDirectShotDefenseRollRequest
		Request;
	Request.AttackSequence = InteractionView.AttackSequence;
	Request.RequestingSide = InteractionView.ExpectedActingPlayer;
	bThroughBallOneOnOneRollCommandInFlight = true;
	const auto Result =
		Host->ResolveThroughBallOneOnOneDirectShotDefenseRoll(Request);
	bThroughBallOneOnOneRollCommandInFlight = false;
	RecordCommandResult(
		TEXT("ResolveThroughBallOneOnOneDirectShotDefenseRoll"), Result);
	if (Result.bSuccess
		&& InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::ContinueResolution)
	{
		ContinueResolution();
	}
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
	AFMCodexLocalMatchHostGameMode* Host = FindLocalMatchHost();
	if (Host == nullptr)
	{
		RecordLocalFailure(
			TEXT("ResolveThroughBallBehindDefenseP1AttackRoll"),
			TEXT("Host unavailable."));
		return;
	}
	FMatchPlayAuthoritativeResolveThroughBallBehindDefenseP1AttackRollRequest
		Request;
	Request.AttackSequence = InteractionView.AttackSequence;
	Request.RequestingSide = InteractionView.ExpectedActingPlayer;
	bThroughBallBehindDefenseRollCommandInFlight = true;
	const auto Result =
		Host->ResolveThroughBallBehindDefenseP1AttackRoll(Request);
	bThroughBallBehindDefenseRollCommandInFlight = false;
	RecordCommandResult(
		TEXT("ResolveThroughBallBehindDefenseP1AttackRoll"), Result);
	if (Result.bSuccess
		&& InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::ContinueResolution)
	{
		ContinueResolution();
	}
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
	AFMCodexLocalMatchHostGameMode* Host = FindLocalMatchHost();
	if (Host == nullptr)
	{
		RecordLocalFailure(
			TEXT("ResolveThroughBallBehindDefenseP1DefenseRoll"),
			TEXT("Host unavailable."));
		return;
	}
	FMatchPlayAuthoritativeResolveThroughBallBehindDefenseP1DefenseRollRequest
		Request;
	Request.AttackSequence = InteractionView.AttackSequence;
	Request.RequestingSide = InteractionView.ExpectedActingPlayer;
	bThroughBallBehindDefenseRollCommandInFlight = true;
	const auto Result =
		Host->ResolveThroughBallBehindDefenseP1DefenseRoll(Request);
	bThroughBallBehindDefenseRollCommandInFlight = false;
	RecordCommandResult(
		TEXT("ResolveThroughBallBehindDefenseP1DefenseRoll"), Result);
	if (Result.bSuccess
		&& InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::ContinueResolution)
	{
		ContinueResolution();
	}
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
	AFMCodexLocalMatchHostGameMode* Host = FindLocalMatchHost();
	if (Host == nullptr)
	{
		RecordLocalFailure(
			TEXT("ApplyThroughBallTerminalResolution"),
			TEXT("Host unavailable."));
		return;
	}
	RecordCommandResult(
		TEXT("ApplyThroughBallTerminalResolution"),
		Host->ApplyThroughBallTerminalResolution());
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
	AFMCodexLocalMatchHostGameMode* Host = FindLocalMatchHost();
	if (Host == nullptr)
	{
		RecordLocalFailure(
			TEXT("AdvanceAfterTerminal"), TEXT("Host unavailable."));
		return;
	}
	FMatchPlayAuthoritativeAdvanceAfterTerminalRequest Request;
	Request.AttackSequence = InteractionView.AttackSequence;
	Request.RequestingSide = InteractionView.ExpectedActingPlayer;
	RecordCommandResult(
		TEXT("AdvanceAfterTerminal"),
		Host->AdvanceAfterTerminal(Request));
}

void AFMCodexLocalMatchPlayerController::ContinueResolution()
{
	if (InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::RollThroughBallInitialRoute
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::RollLongShotDirectAttack
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::RollLongShotDirectDefense
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::RollLongShotDeadCorner
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory
				::RollCutInsideShotDirectAttack
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory
				::RollCutInsideShotDirectDefense
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory
				::RollCutInsideShotDeadCorner
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::RollCrossAttack
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::RollCrossDefense
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory
				::ApplyCrossTerminalResolution
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::RollThroughBallFeetAttack
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::RollThroughBallFeetDefense
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory
				::RollThroughBallAntiOffsideAttack
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory
				::RollThroughBallOneOnOneChipShotAttack
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory
				::RollThroughBallOneOnOneDirectShotAttack
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory
				::RollThroughBallOneOnOneDirectShotDefense
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory
				::RollThroughBallBehindDefenseAttack
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory
				::RollThroughBallBehindDefenseDefense
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory
				::ApplyThroughBallFeetTerminalResolution
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::AdvanceAfterTerminal)
	{
		RecordLocalFailure(
			TEXT("ContinueResolution"),
			TEXT("Side-owned arithmetic rolls and terminal completion require their explicit commands."));
		return;
	}
	AFMCodexLocalMatchHostGameMode* Host = FindLocalMatchHost();
	if (Host == nullptr)
	{
		RecordLocalFailure(TEXT("ContinueResolution"), TEXT("Host unavailable."));
		return;
	}
	const auto Snapshot = Host->GetMatchSnapshot();
	const auto Rules = Host->GetSkillRuleSnapshot();
	if (!Snapshot.bSuccess || !Rules.bSuccess
		|| !Snapshot.Snapshot.bHasCurrentAttack)
	{
		RecordLocalFailure(
			TEXT("ContinueResolution"),
			!Snapshot.bSuccess ? Snapshot.ErrorMessage
				: !Rules.bSuccess ? Rules.ErrorMessage
					: TEXT("No current attack is available to continue."));
		return;
	}

	const FMatchPlayCurrentAttackState& Attack =
		Snapshot.Snapshot.CurrentAttack;
	if (!Attack.bHasResolutionSession)
	{
		if (Attack.bHasSelectedAction
			&& Attack.SelectedAction.ActionType == ESkillRuleType::Cross)
		{
			if (bCrossRouteCommandInFlight)
			{
				return;
			}
			bCrossRouteCommandInFlight = true;
			const auto BeginResult = Host->BeginResolutionSession();
			if (!BeginResult.bSuccess)
			{
				bCrossRouteCommandInFlight = false;
				RecordCommandResult(
					TEXT("BeginResolutionSession"), BeginResult);
				return;
			}
			const auto RouteResult = Host->ResolveInitialRoute();
			bCrossRouteCommandInFlight = false;
			RecordCommandResult(TEXT("ResolveCrossRoute"), RouteResult);
			return;
		}
		RecordCommandResult(
			TEXT("BeginResolutionSession"), Host->BeginResolutionSession());
		return;
	}

	const FMatchPlayCurrentAttackResolutionSession& Session =
		Attack.ResolutionSession;
	if (Session.Stage == EMatchPlayCurrentAttackResolutionStage::AwaitingRoute)
	{
		if (Session.Bundle.Binding.ActionType == ESkillRuleType::LongShot
			|| Session.Bundle.Binding.ActionType
				== ESkillRuleType::CutInsideShot)
		{
			RecordCommandResult(
				TEXT("ResolveIntentDeterminedRoute"),
				Host->ResolveIntentDeterminedRoute());
		}
		else
		{
			RecordCommandResult(
				TEXT("ResolveInitialRoute"), Host->ResolveInitialRoute());
		}
		return;
	}

	if (Session.Stage != EMatchPlayCurrentAttackResolutionStage::RouteResolved
		|| !Session.bHasActualBranch)
	{
		RecordLocalFailure(
			TEXT("ContinueResolution"),
			TEXT("The snapshot does not identify one canonical resolution continuation."));
		return;
	}

	const auto Progress =
		FMatchPlayCurrentAttackPostRouteRollProgressQuery::Evaluate(Session);
	if (!Progress.bIsCanonical)
	{
		RecordLocalFailure(TEXT("ContinueResolution"), Progress.ErrorMessage);
		return;
	}

	switch (Session.ActualBranch.ActionType)
	{
	case ESkillRuleType::Cross:
		if (!Progress.bContractComplete)
		{
			RecordLocalFailure(
				TEXT("ContinueResolution"),
				TEXT("Cross High and Low require explicit side-owned arithmetic roll commands."));
		}
		else
		{
			RecordCommandResult(
				TEXT("ApplyCrossTerminalResolution"),
				Host->ApplyCrossTerminalResolution());
		}
		return;

	case ESkillRuleType::PassControl:
		if (!Progress.bContractComplete)
		{
			RecordCommandResult(
				TEXT("ResolvePassControlPostRoutePlan"),
				Host->ResolvePassControlPostRoutePlan());
		}
		else
		{
			RecordCommandResult(
				TEXT("ApplyPassControlTerminalResolution"),
				Host->ApplyPassControlTerminalResolution());
		}
		return;

	case ESkillRuleType::CutInsideShot:
		if (!Progress.bContractComplete)
		{
			RecordLocalFailure(
				TEXT("ContinueResolution"),
				TEXT("CutInsideShot player-owned rolls require their explicit typed commands."));
		}
		else
		{
			RecordCommandResult(
				TEXT("ApplyShotTerminalResolution"),
				Host->ApplyShotTerminalResolution());
		}
		return;

	case ESkillRuleType::LongShot:
		if (!Progress.bContractComplete)
		{
			RecordLocalFailure(
				TEXT("ContinueResolution"),
				TEXT("LongShot player-owned rolls require their explicit typed commands."));
		}
		else
		{
			RecordCommandResult(
				TEXT("ApplyShotTerminalResolution"),
				Host->ApplyShotTerminalResolution());
		}
		return;

	case ESkillRuleType::ThroughBall:
		break;

	default:
		RecordLocalFailure(
			TEXT("ContinueResolution"),
			TEXT("The resolved action family has no LocalPlay continuation mapping."));
		return;
	}

	if (Session.ActualBranch.ThroughBall
		== EMatchPlayThroughBallActualBranch::Feet)
	{
		RecordLocalFailure(
			TEXT("ContinueResolution"),
			TEXT("ThroughBall Feet requires explicit attacker roll, defender roll, and terminal commands."));
		return;
	}

	const EMatchPlayThroughBallOneOnOneShotChoice Choice =
		Session.ThroughBallOneOnOneShotChoice;
	if (Choice == EMatchPlayThroughBallOneOnOneShotChoice::ChipShot)
	{
		if (Session.PostRouteRollProgress.Phase
				!= EMatchPlayCurrentAttackPostRouteRollPhase::OneOnOneChipShot
			|| !Progress.bContractComplete)
		{
			RecordLocalFailure(
				TEXT("ContinueResolution"),
				TEXT("OneOnOne ChipShot requires its explicit attacker-owned roll command."));
		}
		else
		{
			RecordCommandResult(
				TEXT("ApplyThroughBallTerminalResolution"),
				Host->ApplyThroughBallTerminalResolution());
		}
		return;
	}
	if (Choice == EMatchPlayThroughBallOneOnOneShotChoice::DirectShot)
	{
		if (Session.PostRouteRollProgress.Phase
				!= EMatchPlayCurrentAttackPostRouteRollPhase::OneOnOneDirectShot
			|| !Progress.bContractComplete)
		{
			RecordLocalFailure(
				TEXT("ContinueResolution"),
				TEXT("OneOnOne DirectShot requires explicit attacker and defender roll commands."));
		}
		else
		{
			RecordCommandResult(
				TEXT("ApplyThroughBallTerminalResolution"),
				Host->ApplyThroughBallTerminalResolution());
		}
		return;
	}

	if (Session.ActualBranch.ThroughBall
		== EMatchPlayThroughBallActualBranch::AntiOffside)
	{
		if (!Progress.bContractComplete)
		{
			RecordLocalFailure(
				TEXT("ContinueResolution"),
				TEXT("AntiOffside requires its explicit attacker-owned roll command."));
		}
		else
		{
			RecordCommandResult(
				TEXT("ApplyThroughBallTerminalResolution"),
				Host->ApplyThroughBallTerminalResolution());
		}
		return;
	}

	if (Session.ActualBranch.ThroughBall
		== EMatchPlayThroughBallActualBranch::BehindDefense)
	{
		if (!Progress.bContractComplete)
		{
			RecordLocalFailure(
				TEXT("ContinueResolution"),
				TEXT("BehindDefense P1 rolls require their explicit side-owned commands."));
			return;
		}

		if (Session.PostRouteRollProgress.Phase
			== EMatchPlayCurrentAttackPostRouteRollPhase::PrimaryBranch)
		{
			const auto Formula =
				FMatchPlayCurrentAttackResolveThroughBallBehindDefenseP1FormulaOrchestrator
					::Resolve(Snapshot.Snapshot, &Rules.Snapshot);
			if (Formula.bSuccess
				&& Formula.FormulaResolutionResult.bContinueResolution)
			{
				RecordCommandResult(
					TEXT("ResolveThroughBallBehindDefenseP1Formula"),
					Host->ResolveThroughBallBehindDefenseP1Formula());
			}
			else
			{
				RecordCommandResult(
					TEXT("ApplyThroughBallTerminalResolution"),
					Host->ApplyThroughBallTerminalResolution());
			}
			return;
		}

		RecordCommandResult(
			TEXT("ApplyThroughBallTerminalResolution"),
			Host->ApplyThroughBallTerminalResolution());
		return;
	}

	RecordLocalFailure(
		TEXT("ContinueResolution"),
		TEXT("The ThroughBall snapshot does not identify a supported branch."));
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
		AddText(TEXT("System-controlled resolution step"));
		AddButton(MakeButton(InteractionView.ContinueActionLabel, [this]()
		{
			ContinueResolution();
		}));
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
