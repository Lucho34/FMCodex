#include "FMCodexLocalMatchPlayerController.h"

#include "FMCodexLocalMatchDemoConfiguration.h"
#include "FMCodexLocalMatchHostGameMode.h"
#include "FMCodexLocalMatchScreenWidget.h"
#include "FMCodexLocalMatchUMGPresentation.h"

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

const FFMCodexLocalMatchHotSeatHandoffState&
AFMCodexLocalMatchPlayerController::GetHotSeatHandoffState() const
{
	return HotSeatHandoffState;
}

bool AFMCodexLocalMatchPlayerController::IsAwaitingHotSeatHandoff() const
{
	return HotSeatHandoffState.bAwaitingAcknowledgement;
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
	SurfaceContainer.Reset();
	ViewportWidget.Reset();
	Super::EndPlay(EndPlayReason);
}

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
		UpdateHotSeatHandoff(NewView);
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
		UpdateHotSeatHandoff(NewView);
		InteractionView = MoveTemp(NewView);
		RebuildControlSurface();
		return;
	}

	auto NewView = FFMCodexLocalMatchInteractionViewBuilder::Build(
		State.Snapshot, Rules.Snapshot);
	UpdateHotSeatHandoff(NewView);
	InteractionView = MoveTemp(NewView);
	RebuildControlSurface();
}

void AFMCodexLocalMatchPlayerController::UpdateHotSeatHandoff(
	const FFMCodexLocalMatchInteractionView& NewInteractionView)
{
	FFMCodexLocalMatchHotSeatHandoffPolicy::Reconcile(
		NewInteractionView, HotSeatHandoffState);
}

void FFMCodexLocalMatchHotSeatHandoffPolicy::Reconcile(
	const FFMCodexLocalMatchInteractionView& NewInteractionView,
	FFMCodexLocalMatchHotSeatHandoffState& HandoffState)
{
	if (!NewInteractionView.bMatchActive || NewInteractionView.bMatchEnded)
	{
		HandoffState = {};
		return;
	}

	if (!NewInteractionView.bHumanInteraction
		|| NewInteractionView.ExpectedActingPlayer
			== EInitialTurnOrderPlayer::None)
	{
		HandoffState.bAwaitingAcknowledgement = false;
		HandoffState.PendingPlayer = EInitialTurnOrderPlayer::None;
		HandoffState.PendingInteraction =
			EFMCodexLocalMatchInteractionCategory::None;
		return;
	}

	if (NewInteractionView.ExpectedActingPlayer
		== HandoffState.LastRevealedHumanPlayer)
	{
		HandoffState.bAwaitingAcknowledgement = false;
		HandoffState.PendingPlayer = EInitialTurnOrderPlayer::None;
		HandoffState.PendingInteraction =
			EFMCodexLocalMatchInteractionCategory::None;
		return;
	}

	HandoffState.bAwaitingAcknowledgement = true;
	HandoffState.PendingPlayer =
		NewInteractionView.ExpectedActingPlayer;
	HandoffState.PendingInteraction =
		NewInteractionView.InteractionCategory;
}

bool FFMCodexLocalMatchHotSeatHandoffPolicy::Acknowledge(
	const FFMCodexLocalMatchInteractionView& InteractionView,
	FFMCodexLocalMatchHotSeatHandoffState& HandoffState)
{
	if (!HandoffState.bAwaitingAcknowledgement
		|| !InteractionView.bHumanInteraction
		|| InteractionView.bMatchEnded
		|| HandoffState.PendingPlayer
			!= InteractionView.ExpectedActingPlayer)
	{
		return false;
	}

	HandoffState.LastRevealedHumanPlayer =
		InteractionView.ExpectedActingPlayer;
	HandoffState.bAwaitingAcknowledgement = false;
	HandoffState.PendingPlayer = EInitialTurnOrderPlayer::None;
	HandoffState.PendingInteraction =
		EFMCodexLocalMatchInteractionCategory::None;
	return true;
}

void AFMCodexLocalMatchPlayerController::AcknowledgeHotSeatHandoff()
{
	RefreshPresentation();
	if (!FFMCodexLocalMatchHotSeatHandoffPolicy::Acknowledge(
		InteractionView, HotSeatHandoffState))
	{
		return;
	}
	RebuildControlSurface();
}

bool AFMCodexLocalMatchPlayerController::AllowGameplayCommand(
	const FString& CommandName)
{
	if (!HotSeatHandoffState.bAwaitingAcknowledgement)
	{
		return true;
	}
	RecordLocalFailure(
		CommandName,
		TEXT("Gameplay input is blocked until the pending player confirms Ready."));
	return false;
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
	if (!AllowGameplayCommand(TEXT("StartNewLocalMatch")))
	{
		return;
	}
	AFMCodexLocalMatchHostGameMode* Host = FindLocalMatchHost();
	if (Host == nullptr)
	{
		RecordLocalFailure(TEXT("StartNewLocalMatch"),
			TEXT("The local authoritative GameMode Host is unavailable."));
		return;
	}
	const FFMCodexLocalMatchDemoConfiguration Demo =
		FFMCodexLocalMatchDemoConfigurationFactory::Create();
	RecordCommandResult(
		TEXT("StartNewLocalMatch"),
		Host->StartNewLocalMatch(Demo.OpeningInput, Demo.SkillRuleSet));
}

void AFMCodexLocalMatchPlayerController::BeginDemoOrdinaryAttack()
{
	if (!AllowGameplayCommand(TEXT("BeginOrdinaryAttack")))
	{
		return;
	}
	AFMCodexLocalMatchHostGameMode* Host = FindLocalMatchHost();
	if (Host == nullptr)
	{
		RecordLocalFailure(TEXT("BeginOrdinaryAttack"), TEXT("Host unavailable."));
		return;
	}
	const int32 ActionPoint =
		FFMCodexLocalMatchDemoConfigurationFactory::Create()
			.OrdinaryAttackActionPoint;
	RecordCommandResult(
		TEXT("BeginOrdinaryAttack"), Host->BeginOrdinaryAttack(ActionPoint));
}

void AFMCodexLocalMatchPlayerController::DeployOrdinary(
	const FName CardId,
	const FName SlotId)
{
	if (!AllowGameplayCommand(TEXT("DeployOrdinary")))
	{
		return;
	}
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
	if (!AllowGameplayCommand(TEXT("DeployGoalkeeper")))
	{
		return;
	}
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
	if (!AllowGameplayCommand(TEXT("FinishDeployment")))
	{
		return;
	}
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
	if (!AllowGameplayCommand(TEXT("SubmitCarrier")))
	{
		return;
	}
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
	if (!AllowGameplayCommand(TEXT("SubmitMarker")))
	{
		return;
	}
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
	if (!AllowGameplayCommand(TEXT("SubmitSkill")))
	{
		return;
	}
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
	if (!AllowGameplayCommand(TEXT("SubmitRunner")))
	{
		return;
	}
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
}

void AFMCodexLocalMatchPlayerController::SubmitHelper(const FName CardId)
{
	if (!AllowGameplayCommand(TEXT("SubmitHelper")))
	{
		return;
	}
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

void AFMCodexLocalMatchPlayerController::DeclineCurrentSelection()
{
	if (!AllowGameplayCommand(TEXT("DeclineSelection")))
	{
		return;
	}
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
	if (!AllowGameplayCommand(TEXT("ResolveNoLegalChoice")))
	{
		return;
	}
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
	if (!AllowGameplayCommand(TEXT("SubmitBranchIntent")))
	{
		return;
	}
	AFMCodexLocalMatchHostGameMode* Host = FindLocalMatchHost();
	if (Host == nullptr)
	{
		RecordLocalFailure(TEXT("SubmitBranchIntent"), TEXT("Host unavailable."));
		return;
	}
	FMatchPlayAuthoritativeSubmitBranchIntentRequest Request;
	Request.RequestingSide = InteractionView.ExpectedActingPlayer;
	Request.Intent = Intent;
	RecordCommandResult(
		TEXT("SubmitBranchIntent"), Host->SubmitBranchIntent(Request));
}

void AFMCodexLocalMatchPlayerController::SubmitOneOnOneShotChoice(
	const EMatchPlayThroughBallOneOnOneShotChoice Choice)
{
	if (!AllowGameplayCommand(TEXT("SubmitThroughBallOneOnOneShotChoice")))
	{
		return;
	}
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

void AFMCodexLocalMatchPlayerController::ContinueResolution()
{
	if (!AllowGameplayCommand(TEXT("ContinueResolution")))
	{
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
			RecordCommandResult(
				TEXT("ResolveCrossPostRoutePlan"),
				Host->ResolveCrossPostRoutePlan());
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

	case ESkillRuleType::LongShot:
	case ESkillRuleType::CutInsideShot:
		if (!Progress.bContractComplete)
		{
			const bool bDeadCorner =
				(Session.ActualBranch.ActionType == ESkillRuleType::LongShot
					&& Session.ActualBranch.LongShot
						== EMatchPlayLongShotActualBranch::DeadCorner)
				|| (Session.ActualBranch.ActionType
						== ESkillRuleType::CutInsideShot
					&& Session.ActualBranch.CutInsideShot
						== EMatchPlayCutInsideShotActualBranch::DeadCorner);
			if (bDeadCorner)
			{
				RecordCommandResult(
					TEXT("ResolveDeadCornerPostRouteDecision"),
					Host->ResolveDeadCornerPostRouteDecision());
			}
			else
			{
				RecordCommandResult(
					TEXT("ResolveDirectShotPostRouteDecisionOrPlan"),
					Host->ResolveDirectShotPostRouteDecisionOrPlan());
			}
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
		if (!Progress.bContractComplete)
		{
			RecordCommandResult(
				TEXT("ResolveThroughBallFeetPostRoutePlan"),
				Host->ResolveThroughBallFeetPostRoutePlan());
		}
		else
		{
			RecordCommandResult(
				TEXT("ApplyThroughBallTerminalResolution"),
				Host->ApplyThroughBallTerminalResolution());
		}
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
			RecordCommandResult(
				TEXT("ResolveThroughBallOneOnOneChipShotDecision"),
				Host->ResolveThroughBallOneOnOneChipShotDecision());
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
			RecordCommandResult(
				TEXT("ResolveThroughBallOneOnOneDirectShotPostRoutePlan"),
				Host->ResolveThroughBallOneOnOneDirectShotPostRoutePlan());
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
			RecordCommandResult(
				TEXT("ResolveThroughBallAntiOffsideDecision"),
				Host->ResolveThroughBallAntiOffsideDecision());
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
			if (Session.PostRouteRollProgress.Phase
				== EMatchPlayCurrentAttackPostRouteRollPhase::BehindDefenseP2)
			{
				RecordCommandResult(
					TEXT("ResolveThroughBallBehindDefenseP2Decision"),
					Host->ResolveThroughBallBehindDefenseP2Decision());
			}
			else
			{
				RecordCommandResult(
					TEXT("ResolveThroughBallBehindDefenseP1DecisionOrPlan"),
					Host->ResolveThroughBallBehindDefenseP1DecisionOrPlan());
			}
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
					TEXT("ResolveThroughBallBehindDefenseP2Decision"),
					Host->ResolveThroughBallBehindDefenseP2Decision());
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
	PlayerMatchScreen->RefreshFromPresentation(
		FFMCodexLocalMatchUMGPresentationBuilder::Build(
			InteractionView,
			ResolutionFeedback,
			LastDiagnostic.Message,
			HotSeatHandoffState.bAwaitingAcknowledgement,
			FFMCodexLocalMatchInteractionViewBuilder::ToString(
				HotSeatHandoffState.PendingPlayer)));
}

TSharedRef<SWidget> AFMCodexLocalMatchPlayerController::BuildControlSurface()
{
	using namespace FMCodexLocalMatchPlayerController;
	if (HotSeatHandoffState.bAwaitingAcknowledgement)
	{
		const FString NextPlayer =
			FFMCodexLocalMatchInteractionViewBuilder::ToString(
				HotSeatHandoffState.PendingPlayer);
		const FString NextInteraction =
			FFMCodexLocalMatchInteractionViewBuilder::ToString(
				HotSeatHandoffState.PendingInteraction);
		const FString Diagnostic = FString::Printf(
			TEXT("Last: %s | Host=%s | Accepted=%s | Domain=%s | %s"),
			*LastDiagnostic.CommandName,
			LastDiagnostic.bHostSuccess ? TEXT("OK") : TEXT("FAIL"),
			LastDiagnostic.bAuthoritativeAccepted ? TEXT("YES") : TEXT("NO"),
			LastDiagnostic.bAuthoritativeSuccess ? TEXT("OK") : TEXT("FAIL"),
			*LastDiagnostic.Message);
		return SNew(SBorder)
			.Padding(24.0f)
			.BorderBackgroundColor(FLinearColor(0.01f, 0.01f, 0.01f, 1.0f))
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SNew(SBox)
				.MinDesiredWidth(640.0f)
				.MinDesiredHeight(420.0f)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight().Padding(8.0f)
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("PASS CONTROL — HANDOFF")))
						.Justification(ETextJustify::Center)
						.ColorAndOpacity(FLinearColor(1.0f, 0.88f, 0.42f))
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(8.0f)
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("Next Player: ") + NextPlayer))
						.Justification(ETextJustify::Center)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(4.0f)
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("Next Interaction: ") + NextInteraction))
						.Justification(ETextJustify::Center)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(4.0f)
					[
						SNew(STextBlock)
						.Text(FText::FromString(FString::Printf(
							TEXT("Score: Player A %d - %d Player B"),
							InteractionView.PlayerAScore,
							InteractionView.PlayerBScore)))
						.Justification(ETextJustify::Center)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(4.0f)
					[
						MakeFeedbackPanel(ResolutionFeedback)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(4.0f)
					[
						SNew(SExpandableArea)
						.InitiallyCollapsed(true)
						.HeaderContent()
						[
							SNew(STextBlock)
							.Text(FText::FromString(TEXT("Developer Details")))
						]
						.BodyContent()
						[
							SNew(STextBlock)
							.Text(FText::FromString(Diagnostic))
							.AutoWrapText(true)
						]
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(12.0f)
					[
						MakeButton(NextPlayer + TEXT(" Ready"), [this]()
						{
							AcknowledgeHotSeatHandoff();
						})
					]
				]
			];
	}

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
	case EFMCodexLocalMatchInteractionCategory::BeginAttack:
		AddButton(MakeButton(TEXT("Begin Ordinary Attack (AP 6)"), [this]()
		{
			BeginDemoOrdinaryAttack();
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
