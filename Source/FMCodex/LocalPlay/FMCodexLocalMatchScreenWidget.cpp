#include "FMCodexLocalMatchScreenWidget.h"

#include "FMCodexInteractionPanelWidget.h"
#include "FMCodexCardRackWidget.h"
#include "FMCodexHandMicroDiagnostics.h"
#include "FMCodexLocalMatchPlayerController.h"
#include "FMCodexMatchHeaderWidget.h"
#include "FMCodexPitchWidget.h"
#include "FMCodexPlayerCardWidget.h"
#include "FMCodexPlayerUIStyle.h"
#include "FMCodexResolutionPanelWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/BorderSlot.h"
#include "Components/Button.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Engine/Texture2D.h"

namespace FMCodexLocalMatchScreenWidget
{
	UTextBlock* MakeText(
		UWidgetTree& Tree,
		const FName Name,
		const FString& Text = FString())
	{
		UTextBlock* Result = Tree.ConstructWidget<UTextBlock>(
			UTextBlock::StaticClass(), Name);
		Result->SetText(FText::FromString(Text));
		Result->SetAutoWrapText(true);
		return Result;
	}

	UBorder* MakeRegion(UWidgetTree& Tree, const FName Name)
	{
		UBorder* Result = Tree.ConstructWidget<UBorder>(
			UBorder::StaticClass(), Name);
		FFMCodexPlayerUIStyle::Get().ApplyBorder(
			*Result, EFMCodexPlayerUIColorRole::PanelBackground,
			FFMCodexPlayerUIStyle::Get().GetOuterPadding());
		return Result;
	}

	UButton* MakeButton(
		UWidgetTree& Tree,
		const FName Name,
		const FString& Label)
	{
		UButton* Result = Tree.ConstructWidget<UButton>(
			UButton::StaticClass(), Name);
		UTextBlock* LabelText = MakeText(
			Tree, FName(*(Name.ToString() + TEXT("Label"))), Label);
		LabelText->SetJustification(ETextJustify::Center);
		FFMCodexPlayerUIStyle::Get().ApplyText(
			*LabelText, EFMCodexPlayerUITextRole::Status);
		Result->AddChild(LabelText);
		FFMCodexPlayerUIStyle::Get().ApplyButton(
			*Result, EFMCodexPlayerUIActionRole::Primary);
		return Result;
	}
}

UFMCodexLocalMatchScreenWidget::UFMCodexLocalMatchScreenWidget(
	const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	MatchHeaderWidgetClass = UFMCodexMatchHeaderWidget::StaticClass();
	PitchWidgetClass = UFMCodexPitchWidget::StaticClass();
	InteractionPanelWidgetClass = UFMCodexInteractionPanelWidget::StaticClass();
	ResolutionPanelWidgetClass = UFMCodexResolutionPanelWidget::StaticClass();
}

void UFMCodexLocalMatchScreenWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	BuildWidgetTree();
	RefreshVisuals();
}

TSharedRef<SWidget> UFMCodexLocalMatchScreenWidget::RebuildWidget()
{
	if (WidgetTree == nullptr)
	{
		WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"));
	}
	BuildWidgetTree();
	RefreshVisuals();
	return Super::RebuildWidget();
}

void UFMCodexLocalMatchScreenWidget::SetMatchController(
	AFMCodexLocalMatchPlayerController* InController)
{
	MatchController = InController;
}

void UFMCodexLocalMatchScreenWidget::ClearMatchController()
{
	MatchController = nullptr;
}

void UFMCodexLocalMatchScreenWidget::RefreshFromPresentation(
	const FFMCodexUMGMatchScreenViewModel& InPresentation)
{
	if (PitchWidget != nullptr)
	{
		PitchWidget->EndDeploymentDrag();
	}
	Presentation = InPresentation;
	RefreshVisuals();
}

const FFMCodexUMGMatchScreenViewModel&
UFMCodexLocalMatchScreenWidget::GetPresentation() const
{
	return Presentation;
}

AFMCodexLocalMatchPlayerController*
UFMCodexLocalMatchScreenWidget::GetMatchController() const
{
	return MatchController;
}

UFMCodexMatchHeaderWidget*
UFMCodexLocalMatchScreenWidget::GetMatchHeader() const
{
	return MatchHeader;
}

UFMCodexPitchWidget* UFMCodexLocalMatchScreenWidget::GetPitchWidget() const
{
	return PitchWidget;
}

UFMCodexInteractionPanelWidget*
UFMCodexLocalMatchScreenWidget::GetInteractionPanel() const
{
	return InteractionPanel;
}

UFMCodexResolutionPanelWidget*
UFMCodexLocalMatchScreenWidget::GetResolutionPanel() const
{
	return ResolutionPanel;
}

UFMCodexCardRackWidget*
UFMCodexLocalMatchScreenWidget::GetLocalRackWidget() const
{
	return LocalRackWidget;
}

UFMCodexCardRackWidget*
UFMCodexLocalMatchScreenWidget::GetOpponentRackWidget() const
{
	return OpponentRackWidget;
}

const TArray<TObjectPtr<UFMCodexPlayerCardWidget>>&
UFMCodexLocalMatchScreenWidget::GetRenderedCandidateCardWidgets() const
{
	static const TArray<TObjectPtr<UFMCodexPlayerCardWidget>> Empty;
	return InteractionPanel != nullptr
		? InteractionPanel->GetRenderedCandidateCardWidgets() : Empty;
}

void UFMCodexLocalMatchScreenWidget::RequestStartNewMatch()
{
	if (MatchController != nullptr)
	{
		MatchController->StartNewDemoMatch();
	}
}

void UFMCodexLocalMatchScreenWidget::RequestBeginOrdinaryAttack()
{
	if (MatchController != nullptr)
	{
		MatchController->BeginDemoOrdinaryAttack();
	}
}

void UFMCodexLocalMatchScreenWidget::RequestDeployOrdinary(
	const FName CardId,
	const FName SlotId)
{
	if (MatchController != nullptr)
	{
		MatchController->DeployOrdinary(CardId, SlotId);
	}
}

void UFMCodexLocalMatchScreenWidget::RequestDeployGoalkeeper(
	const FName SlotId)
{
	if (MatchController != nullptr)
	{
		MatchController->DeployGoalkeeper(SlotId);
	}
}

void UFMCodexLocalMatchScreenWidget::RequestFinishDeployment()
{
	if (MatchController != nullptr)
	{
		MatchController->FinishDeployment();
	}
}

void UFMCodexLocalMatchScreenWidget::RequestSubmitCarrier(const FName CardId)
{
	if (MatchController != nullptr)
	{
		MatchController->SubmitCarrier(CardId);
	}
}

void UFMCodexLocalMatchScreenWidget::RequestSubmitMarker(const FName CardId)
{
	if (MatchController != nullptr)
	{
		MatchController->SubmitMarker(CardId);
	}
}

void UFMCodexLocalMatchScreenWidget::RequestSubmitSkill(const FName SkillId)
{
	if (MatchController != nullptr)
	{
		MatchController->SubmitSkill(SkillId);
	}
}

void UFMCodexLocalMatchScreenWidget::RequestSubmitRunner(const FName CardId)
{
	if (MatchController != nullptr)
	{
		MatchController->SubmitRunner(CardId);
	}
}

void UFMCodexLocalMatchScreenWidget::RequestSubmitHelper(const FName CardId)
{
	if (MatchController != nullptr)
	{
		MatchController->SubmitHelper(CardId);
	}
}

void UFMCodexLocalMatchScreenWidget::RequestDeclineSelection()
{
	if (MatchController != nullptr)
	{
		MatchController->DeclineCurrentSelection();
	}
}

void UFMCodexLocalMatchScreenWidget::RequestResolveNoLegalSelection()
{
	if (MatchController != nullptr)
	{
		MatchController->ResolveNoLegalCurrentSelection();
	}
}

void UFMCodexLocalMatchScreenWidget::RequestSubmitBranchIntent(
	const EFMCodexUMGBranchIntent Intent)
{
	if (MatchController == nullptr)
	{
		return;
	}
	switch (Intent)
	{
	case EFMCodexUMGBranchIntent::DirectShot:
		MatchController->SubmitBranchIntent(
			EMatchPlayElectiveBranchIntent::DirectShot);
		break;
	case EFMCodexUMGBranchIntent::DeadCorner:
		MatchController->SubmitBranchIntent(
			EMatchPlayElectiveBranchIntent::DeadCorner);
		break;
	case EFMCodexUMGBranchIntent::CrossHigh:
		MatchController->SubmitBranchIntent(
			EMatchPlayElectiveBranchIntent::CrossHigh);
		break;
	case EFMCodexUMGBranchIntent::CrossLow:
		MatchController->SubmitBranchIntent(
			EMatchPlayElectiveBranchIntent::CrossLow);
		break;
	default:
		break;
	}
}

void UFMCodexLocalMatchScreenWidget::RequestSubmitOneOnOneChoice(
	const EFMCodexUMGOneOnOneChoice Choice)
{
	if (MatchController == nullptr)
	{
		return;
	}
	if (Choice == EFMCodexUMGOneOnOneChoice::ChipShot)
	{
		MatchController->SubmitOneOnOneShotChoice(
			EMatchPlayThroughBallOneOnOneShotChoice::ChipShot);
	}
	else if (Choice == EFMCodexUMGOneOnOneChoice::DirectShot)
	{
		MatchController->SubmitOneOnOneShotChoice(
			EMatchPlayThroughBallOneOnOneShotChoice::DirectShot);
	}
}

void UFMCodexLocalMatchScreenWidget::RequestContinueResolution()
{
	if (MatchController != nullptr)
	{
		MatchController->ContinueResolution();
	}
}

void UFMCodexLocalMatchScreenWidget::RequestReady()
{
	if (MatchController != nullptr)
	{
		MatchController->AcknowledgeHotSeatHandoff();
	}
}

void UFMCodexLocalMatchScreenWidget::HandleStartNewMatchClicked()
{
	RequestStartNewMatch();
}

void UFMCodexLocalMatchScreenWidget::HandleBeginOrdinaryAttackClicked()
{
	RequestBeginOrdinaryAttack();
}

void UFMCodexLocalMatchScreenWidget::HandleDeployOrdinaryRequested(
	const FName CardId,
	const FName SlotId)
{
	RequestDeployOrdinary(CardId, SlotId);
}

void UFMCodexLocalMatchScreenWidget::HandleDeployGoalkeeperRequested(
	const FName SlotId)
{
	RequestDeployGoalkeeper(SlotId);
}

void UFMCodexLocalMatchScreenWidget::HandleFinishDeploymentClicked()
{
	RequestFinishDeployment();
}

void UFMCodexLocalMatchScreenWidget::HandleCarrierRequested(const FName CardId)
{
	RequestSubmitCarrier(CardId);
}

void UFMCodexLocalMatchScreenWidget::HandleMarkerRequested(const FName CardId)
{
	RequestSubmitMarker(CardId);
}

void UFMCodexLocalMatchScreenWidget::HandleSkillRequested(const FName SkillId)
{
	RequestSubmitSkill(SkillId);
}

void UFMCodexLocalMatchScreenWidget::HandleRunnerRequested(const FName CardId)
{
	RequestSubmitRunner(CardId);
}

void UFMCodexLocalMatchScreenWidget::HandleHelperRequested(const FName CardId)
{
	RequestSubmitHelper(CardId);
}

void UFMCodexLocalMatchScreenWidget::HandleDeclineRequested()
{
	RequestDeclineSelection();
}

void UFMCodexLocalMatchScreenWidget::HandleNoLegalRequested()
{
	RequestResolveNoLegalSelection();
}

void UFMCodexLocalMatchScreenWidget::HandleBranchRequested(
	const EFMCodexUMGBranchIntent Intent)
{
	RequestSubmitBranchIntent(Intent);
}

void UFMCodexLocalMatchScreenWidget::HandleOneOnOneRequested(
	const EFMCodexUMGOneOnOneChoice Choice)
{
	RequestSubmitOneOnOneChoice(Choice);
}

void UFMCodexLocalMatchScreenWidget::HandleContinueRequested()
{
	RequestContinueResolution();
}

void UFMCodexLocalMatchScreenWidget::HandleReadyClicked()
{
	RequestReady();
}

void UFMCodexLocalMatchScreenWidget::HandleDeploymentDragStarted(
	const FName CardId,
	const bool bGoalkeeper)
{
	if (PitchWidget == nullptr || Presentation.Handoff.bVisible
		|| Presentation.Interaction.Category
			!= EFMCodexUMGInteractionCategory::Deploy)
	{
		return;
	}
	const bool bPresentedChoice =
		Presentation.Interaction.DeploymentChoices.ContainsByPredicate(
			[CardId, bGoalkeeper](
				const FFMCodexUMGDeploymentChoiceViewModel& Choice)
			{
				return Choice.CardId == CardId
					&& Choice.bGoalkeeper == bGoalkeeper;
			});
	if (bPresentedChoice)
	{
		PitchWidget->BeginDeploymentDrag(
			CardId, Presentation.Interaction.DeploymentChoices);
	}
}

void UFMCodexLocalMatchScreenWidget::HandleDeploymentDragFinished()
{
	if (PitchWidget != nullptr)
	{
		PitchWidget->EndDeploymentDrag();
	}
}

void UFMCodexLocalMatchScreenWidget::HandlePitchDeploymentDropped(
	const FName CardId,
	const FName SlotId,
	const bool bGoalkeeper)
{
	if (PitchWidget != nullptr)
	{
		PitchWidget->EndDeploymentDrag();
	}
	if (bGoalkeeper)
	{
		RequestDeployGoalkeeper(SlotId);
	}
	else
	{
		RequestDeployOrdinary(CardId, SlotId);
	}
}

void UFMCodexLocalMatchScreenWidget::BuildWidgetTree()
{
	using namespace FMCodexLocalMatchScreenWidget;
	if (WidgetTree == nullptr || WidgetTree->RootWidget != nullptr)
	{
		return;
	}

	UOverlay* Root = WidgetTree->ConstructWidget<UOverlay>(
		UOverlay::StaticClass(), TEXT("MatchScreenRoot"));
	WidgetTree->RootWidget = Root;
	UBorder* ScreenBackground = WidgetTree->ConstructWidget<UBorder>(
		UBorder::StaticClass(), TEXT("MatchScreenStyleBackground"));
	FFMCodexPlayerUIStyle::Get().ApplyBorder(
		*ScreenBackground, EFMCodexPlayerUIColorRole::ScreenBackground,
		FMargin(0.0f));
	if (UOverlaySlot* BackgroundSlot = Root->AddChildToOverlay(ScreenBackground))
	{
		BackgroundSlot->SetHorizontalAlignment(HAlign_Fill);
		BackgroundSlot->SetVerticalAlignment(VAlign_Fill);
	}

	MainScreen = WidgetTree->ConstructWidget<UVerticalBox>(
		UVerticalBox::StaticClass(), TEXT("GoldenLayoutMatchScreen"));
	USizeBox* ResponsiveBounds = WidgetTree->ConstructWidget<USizeBox>(
		USizeBox::StaticClass(), TEXT("ResponsiveGoldenLayoutBounds"));
	ResponsiveBounds->SetWidthOverride(1920.0f);
	ResponsiveBounds->AddChild(MainScreen);
	if (UOverlaySlot* MainSlot = Root->AddChildToOverlay(ResponsiveBounds))
	{
		MainSlot->SetHorizontalAlignment(HAlign_Center);
		MainSlot->SetVerticalAlignment(VAlign_Center);
	}

	USizeBox* HeaderBounds = WidgetTree->ConstructWidget<USizeBox>(
		USizeBox::StaticClass(), TEXT("BroadcastMatchHeaderRegion"));
	HeaderBounds->SetHeightOverride(80.0f);
	UBorder* HeaderRegion = MakeRegion(*WidgetTree, TEXT("MatchHeaderRegion"));
	UClass* ResolvedHeaderClass = MatchHeaderWidgetClass != nullptr
		? MatchHeaderWidgetClass.Get() : UFMCodexMatchHeaderWidget::StaticClass();
	MatchHeader = WidgetTree->ConstructWidget<UFMCodexMatchHeaderWidget>(
		ResolvedHeaderClass, TEXT("DedicatedMatchHeaderWidget"));
	HeaderRegion->AddChild(MatchHeader);
	HeaderBounds->AddChild(HeaderRegion);
	MainScreen->AddChildToVerticalBox(HeaderBounds);

	USizeBox* MainAreaBounds = WidgetTree->ConstructWidget<USizeBox>(
		USizeBox::StaticClass(), TEXT("GoldenLayoutMainMatchArea"));
	MainAreaBounds->SetHeightOverride(880.0f);
	UHorizontalBox* MainArea = WidgetTree->ConstructWidget<UHorizontalBox>(
		UHorizontalBox::StaticClass(), TEXT("RackPitchRackHierarchy"));
	MainAreaBounds->AddChild(MainArea);
	LocalRackBounds = WidgetTree->ConstructWidget<USizeBox>(
		USizeBox::StaticClass(), TEXT("LocalPlayerCardRackRegion"));
	LocalRackBounds->SetWidthOverride(422.0f);
	LocalRackWidget = WidgetTree->ConstructWidget<UFMCodexCardRackWidget>(
		UFMCodexCardRackWidget::StaticClass(), TEXT("PersistentLocalCardRack"));
	LocalRackWidget->OnCardDragStarted.AddUObject(
		this, &UFMCodexLocalMatchScreenWidget::HandleDeploymentDragStarted);
	LocalRackWidget->OnCardDragFinished.AddUObject(
		this, &UFMCodexLocalMatchScreenWidget::HandleDeploymentDragFinished);
	LocalRackBounds->AddChild(LocalRackWidget);
	MainArea->AddChildToHorizontalBox(LocalRackBounds);

	PitchBounds = WidgetTree->ConstructWidget<USizeBox>(
		USizeBox::StaticClass(), TEXT("CentralPitchRegion"));
	PitchBounds->SetWidthOverride(1076.0f);
	UBorder* PitchRegion = MakeRegion(*WidgetTree, TEXT("FootballCardFieldRegion"));
	UClass* ResolvedPitchClass = PitchWidgetClass != nullptr
		? PitchWidgetClass.Get() : UFMCodexPitchWidget::StaticClass();
	PitchWidget = WidgetTree->ConstructWidget<UFMCodexPitchWidget>(
		ResolvedPitchClass, TEXT("DedicatedFootballPitchWidget"));
	PitchWidget->OnDeploymentDropped.AddUObject(
		this,
		&UFMCodexLocalMatchScreenWidget::HandlePitchDeploymentDropped);
	PitchRegion->AddChild(PitchWidget);
	PitchBounds->AddChild(PitchRegion);
	if (UHorizontalBoxSlot* PitchSlot = MainArea->AddChildToHorizontalBox(PitchBounds))
	{
		PitchSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
		PitchSlot->SetHorizontalAlignment(HAlign_Fill);
		PitchSlot->SetVerticalAlignment(VAlign_Fill);
	}
	OpponentRackBounds = WidgetTree->ConstructWidget<USizeBox>(
		USizeBox::StaticClass(), TEXT("OpponentCardRackRegion"));
	OpponentRackBounds->SetWidthOverride(422.0f);
	OpponentRackWidget = WidgetTree->ConstructWidget<UFMCodexCardRackWidget>(
		UFMCodexCardRackWidget::StaticClass(), TEXT("PersistentOpponentCardRack"));
	OpponentRackBounds->AddChild(OpponentRackWidget);
	MainArea->AddChildToHorizontalBox(OpponentRackBounds);
	MainScreen->AddChildToVerticalBox(MainAreaBounds);

	USizeBox* DockBounds = WidgetTree->ConstructWidget<USizeBox>(
		USizeBox::StaticClass(), TEXT("ContextActionDockRegion"));
	DockBounds->SetHeightOverride(120.0f);
	DockBounds->SetClipping(EWidgetClipping::ClipToBounds);
	UBorder* InteractionRegion = MakeRegion(
		*WidgetTree, TEXT("CurrentInteractionRegion"));
	UClass* ResolvedInteractionClass = InteractionPanelWidgetClass != nullptr
		? InteractionPanelWidgetClass.Get()
		: UFMCodexInteractionPanelWidget::StaticClass();
	InteractionPanel =
		WidgetTree->ConstructWidget<UFMCodexInteractionPanelWidget>(
			ResolvedInteractionClass, TEXT("DedicatedInteractionPanelWidget"));
	InteractionPanel->OnStartMatchRequested.AddDynamic(
		this, &UFMCodexLocalMatchScreenWidget::HandleStartNewMatchClicked);
	InteractionPanel->OnBeginAttackRequested.AddDynamic(
		this, &UFMCodexLocalMatchScreenWidget::HandleBeginOrdinaryAttackClicked);
	InteractionPanel->OnDeployOrdinaryRequested.AddDynamic(
		this, &UFMCodexLocalMatchScreenWidget::HandleDeployOrdinaryRequested);
	InteractionPanel->OnDeployGoalkeeperRequested.AddDynamic(
		this, &UFMCodexLocalMatchScreenWidget::HandleDeployGoalkeeperRequested);
	InteractionPanel->OnFinishDeploymentRequested.AddDynamic(
		this, &UFMCodexLocalMatchScreenWidget::HandleFinishDeploymentClicked);
	InteractionPanel->OnCarrierRequested.AddDynamic(
		this, &UFMCodexLocalMatchScreenWidget::HandleCarrierRequested);
	InteractionPanel->OnMarkerRequested.AddDynamic(
		this, &UFMCodexLocalMatchScreenWidget::HandleMarkerRequested);
	InteractionPanel->OnSkillRequested.AddDynamic(
		this, &UFMCodexLocalMatchScreenWidget::HandleSkillRequested);
	InteractionPanel->OnRunnerRequested.AddDynamic(
		this, &UFMCodexLocalMatchScreenWidget::HandleRunnerRequested);
	InteractionPanel->OnHelperRequested.AddDynamic(
		this, &UFMCodexLocalMatchScreenWidget::HandleHelperRequested);
	InteractionPanel->OnDeclineRequested.AddDynamic(
		this, &UFMCodexLocalMatchScreenWidget::HandleDeclineRequested);
	InteractionPanel->OnNoLegalRequested.AddDynamic(
		this, &UFMCodexLocalMatchScreenWidget::HandleNoLegalRequested);
	InteractionPanel->OnBranchRequested.AddDynamic(
		this, &UFMCodexLocalMatchScreenWidget::HandleBranchRequested);
	InteractionPanel->OnOneOnOneRequested.AddDynamic(
		this, &UFMCodexLocalMatchScreenWidget::HandleOneOnOneRequested);
	InteractionPanel->OnContinueRequested.AddDynamic(
		this, &UFMCodexLocalMatchScreenWidget::HandleContinueRequested);
	InteractionPanel->OnDeploymentDragStarted.AddUObject(
		this, &UFMCodexLocalMatchScreenWidget::HandleDeploymentDragStarted);
	InteractionPanel->OnDeploymentDragFinished.AddUObject(
		this, &UFMCodexLocalMatchScreenWidget::HandleDeploymentDragFinished);
	InteractionRegion->AddChild(InteractionPanel);
	DockBounds->AddChild(InteractionRegion);
	MainScreen->AddChildToVerticalBox(DockBounds);

#if !UE_BUILD_SHIPPING
	if (FMCodexHandMicroDiagnostics::IsSharpnessComparisonEnabled())
	{
	HandMicroSharpnessDiagnosticSurface = WidgetTree->ConstructWidget<UBorder>(
		UBorder::StaticClass(), TEXT("HandMicroSharpnessDiagnosticSurface"));
	HandMicroSharpnessDiagnosticSurface->SetBrushColor(
		FLinearColor::FromSRGBColor(FColor(0x08, 0x16, 0x20)));
	HandMicroSharpnessDiagnosticSurface->SetPadding(FMargin(16.0f, 12.0f));
	HandMicroSharpnessDiagnosticSurface->SetClipping(EWidgetClipping::ClipToBounds);
	UOverlay* DiagnosticPageStack = WidgetTree->ConstructWidget<UOverlay>(
		UOverlay::StaticClass(), TEXT("HandMicroSharpnessDiagnosticPageStack"));
	HandMicroSharpnessDiagnosticSurface->AddChild(DiagnosticPageStack);
	UOverlay* DiagnosticCardStore = WidgetTree->ConstructWidget<UOverlay>(
		UOverlay::StaticClass(), TEXT("HandMicroSharpnessDiagnosticCardStore"));
	DiagnosticCardStore->SetVisibility(ESlateVisibility::Collapsed);
	DiagnosticPageStack->AddChildToOverlay(DiagnosticCardStore);
	UVerticalBox* DiagnosticBody = WidgetTree->ConstructWidget<UVerticalBox>(
		UVerticalBox::StaticClass(), TEXT("HandMicroSharpnessDiagnosticBody"));
	HandMicroSharpnessRayaPage = DiagnosticBody;
	UTextBlock* DiagnosticTitle = MakeText(*WidgetTree,
		TEXT("HandMicroSharpnessDiagnosticTitle"),
		TEXT("HAND MICRO PORTRAIT SHARPNESS — RAYA — 96x64"));
	DiagnosticTitle->SetAutoWrapText(false);
	DiagnosticTitle->SetJustification(ETextJustify::Center);
	FFMCodexPlayerUIStyle::Get().ApplyText(
		*DiagnosticTitle, EFMCodexPlayerUITextRole::Status);
	DiagnosticBody->AddChildToVerticalBox(DiagnosticTitle);
	UHorizontalBox* DiagnosticColumns =
		WidgetTree->ConstructWidget<UHorizontalBox>(
			UHorizontalBox::StaticClass(),
			TEXT("HandMicroSharpnessDiagnosticColumns"));
	DiagnosticBody->AddChildToVerticalBox(DiagnosticColumns);
	DiagnosticPageStack->AddChildToOverlay(DiagnosticBody);

	HandMicroSharpnessHighResolutionTexture = LoadObject<UTexture2D>(
		nullptr,
		TEXT("/Game/UI/Portraits/PrototypeTeams/Arsenal/HandMicro/"
			"T_Prototype_Arsenal_DavidRaya_HandMicro_06."
			"T_Prototype_Arsenal_DavidRaya_HandMicro_06"));
	HandMicroSharpnessRuntimeTexture = LoadObject<UTexture2D>(
		nullptr,
		TEXT("/Game/Developers/FMCodex/HandMicroDiagnostics/"
			"T_Prototype_Arsenal_DavidRaya_HandMicro_Runtime192."
			"T_Prototype_Arsenal_DavidRaya_HandMicro_Runtime192"));

	FFMCodexUMGCardViewModel RayaDiagnosticCard;
	RayaDiagnosticCard.CardId = TEXT("Prototype.Arsenal.DavidRaya");
	RayaDiagnosticCard.IdentityLabel = TEXT("Raya");
	RayaDiagnosticCard.OwnerLabel = TEXT("Player A");
	RayaDiagnosticCard.RoleLabel = TEXT("GK");
	RayaDiagnosticCard.RarityLabel = TEXT("Continental");
	RayaDiagnosticCard.bGoalkeeper = true;

	auto AddDiagnosticColumn = [this, DiagnosticCardStore](
		UHorizontalBox* TargetColumns,
		const FFMCodexUMGCardViewModel& CardModel,
		const FName ColumnName,
		const FString& Label,
		const FName ViewName,
		const FName ContentName,
		UTexture2D* Texture,
		const bool bDirectImage,
		UFMCodexPlayerCardWidget*& OutCard)
	{
		UVerticalBox* Column = WidgetTree->ConstructWidget<UVerticalBox>(
			UVerticalBox::StaticClass(), ColumnName);
		UTextBlock* LabelText = MakeText(
			*WidgetTree, FName(*(ColumnName.ToString() + TEXT("Label"))), Label);
		LabelText->SetAutoWrapText(false);
		LabelText->SetJustification(ETextJustify::Center);
		FFMCodexPlayerUIStyle::Get().ApplyText(
			*LabelText, EFMCodexPlayerUITextRole::Secondary);
		Column->AddChildToVerticalBox(LabelText);
		USizeBox* View = WidgetTree->ConstructWidget<USizeBox>(
			USizeBox::StaticClass(), ViewName);
		View->SetWidthOverride(FMCodexHandMicroDiagnostics::PortraitWidth);
		View->SetHeightOverride(FMCodexHandMicroDiagnostics::PortraitImageHeight);
		View->SetClipping(EWidgetClipping::ClipToBounds);
		UBorder* ComparisonBackground = WidgetTree->ConstructWidget<UBorder>(
			UBorder::StaticClass(),
			FName(*(ViewName.ToString() + TEXT("ComparisonBackground"))));
		ComparisonBackground->SetBrushColor(
			FLinearColor::FromSRGBColor(FColor(0x0C, 0x23, 0x30)));
		ComparisonBackground->SetPadding(FMargin(0.0f));
		ComparisonBackground->SetClipping(EWidgetClipping::ClipToBounds);
		if (bDirectImage)
		{
			HandMicroSharpnessDirectImage = WidgetTree->ConstructWidget<UImage>(
				UImage::StaticClass(), ContentName);
			HandMicroSharpnessDirectImage->SetBrushFromTexture(Texture, false);
			FSlateBrush DirectBrush = HandMicroSharpnessDirectImage->GetBrush();
			DirectBrush.DrawAs = ESlateBrushDrawType::Image;
			DirectBrush.SetImageSize(FVector2D(
				FMCodexHandMicroDiagnostics::PortraitWidth,
				FMCodexHandMicroDiagnostics::PortraitImageHeight));
			DirectBrush.SetUVRegion(FBox2f(FVector2f(0.0f, 0.0f),
				FVector2f(1.0f, 1.0f)));
			HandMicroSharpnessDirectImage->SetBrush(DirectBrush);
			ComparisonBackground->AddChild(HandMicroSharpnessDirectImage);
		}
		else
		{
			OutCard = WidgetTree->ConstructWidget<UFMCodexPlayerCardWidget>(
				UFMCodexPlayerCardWidget::StaticClass(), ContentName);
			OutCard->RefreshFromPresentation(CardModel,
				EFMCodexPlayerCardPresentationMode::HandMicro);
			if (Texture != nullptr)
			{
				OutCard->SetDiagnosticHandMicroPortraitOverride(Texture);
			}
			OutCard->TakeWidget();
			USizeBox* ProductionPortraitBounds = Cast<USizeBox>(
				OutCard->GetWidgetFromName(TEXT("HandMicroPortraitBounds")));
			if (ProductionPortraitBounds != nullptr)
			{
				ProductionPortraitBounds->RemoveFromParent();
				ComparisonBackground->AddChild(ProductionPortraitBounds);
			}
			DiagnosticCardStore->AddChildToOverlay(OutCard);
		}
		View->AddChild(ComparisonBackground);
		Column->AddChildToVerticalBox(View);
		if (UHorizontalBoxSlot* ColumnSlot =
			TargetColumns->AddChildToHorizontalBox(Column))
		{
			ColumnSlot->SetPadding(FMargin(10.0f, 6.0f));
		}
	};
	UFMCodexPlayerCardWidget* UnusedDirectCard = nullptr;
	AddDiagnosticColumn(DiagnosticColumns, RayaDiagnosticCard,
		TEXT("SharpnessColumnA"), TEXT("A  DIRECT HIGH-RES"),
		TEXT("HandMicroSharpnessAView"), TEXT("HandMicroSharpnessADirectImage"),
		HandMicroSharpnessHighResolutionTexture, true, UnusedDirectCard);
	UFMCodexPlayerCardWidget* ProductionCard = nullptr;
	AddDiagnosticColumn(DiagnosticColumns, RayaDiagnosticCard,
		TEXT("SharpnessColumnB"), TEXT("B  PRODUCTION HIGH-RES"),
		TEXT("HandMicroSharpnessBView"), TEXT("HandMicroSharpnessBProductionCard"),
		nullptr, false, ProductionCard);
	HandMicroSharpnessProductionCard = ProductionCard;
	UFMCodexPlayerCardWidget* RuntimeCard = nullptr;
	AddDiagnosticColumn(DiagnosticColumns, RayaDiagnosticCard,
		TEXT("SharpnessColumnC"), TEXT("C  PRODUCTION 192x128"),
		TEXT("HandMicroSharpnessCView"), TEXT("HandMicroSharpnessCRuntimeCard"),
		HandMicroSharpnessRuntimeTexture, false, RuntimeCard);
	HandMicroSharpnessRuntimeCard = RuntimeCard;

	UVerticalBox* RepresentativePage = WidgetTree->ConstructWidget<UVerticalBox>(
		UVerticalBox::StaticClass(), TEXT("HandMicroSharpnessRepresentativePage"));
	HandMicroSharpnessRepresentativePage = RepresentativePage;
	UTextBlock* RepresentativeTitle = MakeText(*WidgetTree,
		TEXT("HandMicroSharpnessRepresentativeTitle"),
		TEXT("REPRESENTATIVE PORTRAIT B/C — EACH VIEW 96x64"));
	RepresentativeTitle->SetAutoWrapText(false);
	RepresentativeTitle->SetJustification(ETextJustify::Center);
	FFMCodexPlayerUIStyle::Get().ApplyText(
		*RepresentativeTitle, EFMCodexPlayerUITextRole::Status);
	RepresentativePage->AddChildToVerticalBox(RepresentativeTitle);
	UUniformGridPanel* RepresentativeGrid =
		WidgetTree->ConstructWidget<UUniformGridPanel>(
			UUniformGridPanel::StaticClass(),
			TEXT("HandMicroSharpnessRepresentativeGrid"));
	RepresentativeGrid->SetSlotPadding(FMargin(8.0f, 5.0f));
	RepresentativePage->AddChildToVerticalBox(RepresentativeGrid);
	DiagnosticPageStack->AddChildToOverlay(RepresentativePage);

	struct FRepresentativePortrait
	{
		FName CardId;
		FString Label;
		FString Slug;
		FString RuntimeTexturePath;
		FString ConformedTexturePath;
		FString RebalancedTexturePath;
		FString ReferenceATexturePath;
		bool bGoalkeeper = false;
	};
	const TArray<FRepresentativePortrait> Representatives = {
		{ TEXT("Prototype.Arsenal.DavidRaya"), TEXT("RAYA"), TEXT("Raya"),
			TEXT("/Game/Developers/FMCodex/HandMicroDiagnostics/"
				"T_Prototype_Arsenal_DavidRaya_HandMicro_Runtime192."
				"T_Prototype_Arsenal_DavidRaya_HandMicro_Runtime192"),
			TEXT("/Game/Developers/FMCodex/HandMicroArtConformance/"
				"T_Prototype_Arsenal_DavidRaya_HandMicro_ArtConformedRuntime192."
				"T_Prototype_Arsenal_DavidRaya_HandMicro_ArtConformedRuntime192"),
			TEXT("/Game/Developers/FMCodex/HandMicroPortraitRebalance/"
				"T_Prototype_Arsenal_DavidRaya_HandMicro_RebalancedRuntime192."
				"T_Prototype_Arsenal_DavidRaya_HandMicro_RebalancedRuntime192"),
			TEXT("/Game/Developers/FMCodex/HandMicroReferenceA/"
				"T_Prototype_Arsenal_DavidRaya_HandMicro_ReferenceARuntime192."
				"T_Prototype_Arsenal_DavidRaya_HandMicro_ReferenceARuntime192"), true },
		{ TEXT("Prototype.Arsenal.WilliamSaliba"), TEXT("SALIBA"), TEXT("Saliba"),
			TEXT("/Game/Developers/FMCodex/HandMicroDiagnostics/"
				"T_Prototype_Arsenal_WilliamSaliba_HandMicro_Runtime192."
				"T_Prototype_Arsenal_WilliamSaliba_HandMicro_Runtime192"),
			TEXT("/Game/Developers/FMCodex/HandMicroArtConformance/"
				"T_Prototype_Arsenal_WilliamSaliba_HandMicro_ArtConformedRuntime192."
				"T_Prototype_Arsenal_WilliamSaliba_HandMicro_ArtConformedRuntime192"),
			TEXT("/Game/Developers/FMCodex/HandMicroPortraitRebalance/"
				"T_Prototype_Arsenal_WilliamSaliba_HandMicro_RebalancedRuntime192."
				"T_Prototype_Arsenal_WilliamSaliba_HandMicro_RebalancedRuntime192"),
			TEXT("/Game/Developers/FMCodex/HandMicroReferenceA/"
				"T_Prototype_Arsenal_WilliamSaliba_HandMicro_ReferenceARuntime192."
				"T_Prototype_Arsenal_WilliamSaliba_HandMicro_ReferenceARuntime192") },
		{ TEXT("Prototype.Arsenal.BukayoSaka"), TEXT("SAKA"), TEXT("Saka"),
			TEXT("/Game/Developers/FMCodex/HandMicroDiagnostics/"
				"T_Prototype_Arsenal_BukayoSaka_HandMicro_Runtime192."
				"T_Prototype_Arsenal_BukayoSaka_HandMicro_Runtime192"),
			TEXT("/Game/Developers/FMCodex/HandMicroArtConformance/"
				"T_Prototype_Arsenal_BukayoSaka_HandMicro_ArtConformedRuntime192."
				"T_Prototype_Arsenal_BukayoSaka_HandMicro_ArtConformedRuntime192"),
			TEXT("/Game/Developers/FMCodex/HandMicroPortraitRebalance/"
				"T_Prototype_Arsenal_BukayoSaka_HandMicro_RebalancedRuntime192."
				"T_Prototype_Arsenal_BukayoSaka_HandMicro_RebalancedRuntime192"),
			TEXT("/Game/Developers/FMCodex/HandMicroReferenceA/"
				"T_Prototype_Arsenal_BukayoSaka_HandMicro_ReferenceARuntime192."
				"T_Prototype_Arsenal_BukayoSaka_HandMicro_ReferenceARuntime192") },
		{ TEXT("Prototype.Arsenal.MartinOdegaard"), TEXT("ODEGAARD"),
			TEXT("Odegaard"),
			TEXT("/Game/Developers/FMCodex/HandMicroDiagnostics/"
				"T_Prototype_Arsenal_MartinOdegaard_HandMicro_Runtime192."
				"T_Prototype_Arsenal_MartinOdegaard_HandMicro_Runtime192"),
			TEXT("/Game/Developers/FMCodex/HandMicroArtConformance/"
				"T_Prototype_Arsenal_MartinOdegaard_HandMicro_ArtConformedRuntime192."
				"T_Prototype_Arsenal_MartinOdegaard_HandMicro_ArtConformedRuntime192"),
			TEXT("/Game/Developers/FMCodex/HandMicroPortraitRebalance/"
				"T_Prototype_Arsenal_MartinOdegaard_HandMicro_RebalancedRuntime192."
				"T_Prototype_Arsenal_MartinOdegaard_HandMicro_RebalancedRuntime192"),
			TEXT("/Game/Developers/FMCodex/HandMicroReferenceA/"
				"T_Prototype_Arsenal_MartinOdegaard_HandMicro_ReferenceARuntime192."
				"T_Prototype_Arsenal_MartinOdegaard_HandMicro_ReferenceARuntime192") },
		{ TEXT("Prototype.ManchesterCity.GianluigiDonnarumma"),
			TEXT("DONNARUMMA"), TEXT("Donnarumma"),
			TEXT("/Game/Developers/FMCodex/HandMicroDiagnostics/"
				"T_Prototype_ManchesterCity_GianluigiDonnarumma_HandMicro_Runtime192."
				"T_Prototype_ManchesterCity_GianluigiDonnarumma_HandMicro_Runtime192"),
			TEXT("/Game/Developers/FMCodex/HandMicroArtConformance/"
				"T_Prototype_ManchesterCity_GianluigiDonnarumma_HandMicro_"
				"ArtConformedRuntime192."
				"T_Prototype_ManchesterCity_GianluigiDonnarumma_HandMicro_"
				"ArtConformedRuntime192"),
			TEXT("/Game/Developers/FMCodex/HandMicroPortraitRebalance/"
				"T_Prototype_ManchesterCity_GianluigiDonnarumma_HandMicro_"
				"RebalancedRuntime192."
				"T_Prototype_ManchesterCity_GianluigiDonnarumma_HandMicro_"
				"RebalancedRuntime192"),
			TEXT("/Game/Developers/FMCodex/HandMicroReferenceA/"
				"T_Prototype_ManchesterCity_GianluigiDonnarumma_HandMicro_"
				"ReferenceARuntime192."
				"T_Prototype_ManchesterCity_GianluigiDonnarumma_HandMicro_"
				"ReferenceARuntime192"),
			true },
		{ TEXT("Prototype.ManchesterCity.ErlingHaaland"), TEXT("HAALAND"),
			TEXT("Haaland"),
			TEXT("/Game/Developers/FMCodex/HandMicroDiagnostics/"
				"T_Prototype_ManchesterCity_ErlingHaaland_HandMicro_Runtime192."
				"T_Prototype_ManchesterCity_ErlingHaaland_HandMicro_Runtime192"),
			TEXT("/Game/Developers/FMCodex/HandMicroArtConformance/"
				"T_Prototype_ManchesterCity_ErlingHaaland_HandMicro_"
				"ArtConformedRuntime192."
				"T_Prototype_ManchesterCity_ErlingHaaland_HandMicro_"
				"ArtConformedRuntime192"),
			TEXT("/Game/Developers/FMCodex/HandMicroPortraitRebalance/"
				"T_Prototype_ManchesterCity_ErlingHaaland_HandMicro_"
				"RebalancedRuntime192."
				"T_Prototype_ManchesterCity_ErlingHaaland_HandMicro_"
				"RebalancedRuntime192"),
			TEXT("/Game/Developers/FMCodex/HandMicroReferenceA/"
				"T_Prototype_ManchesterCity_ErlingHaaland_HandMicro_"
				"ReferenceARuntime192."
				"T_Prototype_ManchesterCity_ErlingHaaland_HandMicro_"
				"ReferenceARuntime192") }
	};
	for (int32 Index = 0; Index < Representatives.Num(); ++Index)
	{
		const FRepresentativePortrait& Representative = Representatives[Index];
		UVerticalBox* PlayerPair = WidgetTree->ConstructWidget<UVerticalBox>(
			UVerticalBox::StaticClass(),
			FName(*(TEXT("Representative") + Representative.Slug + TEXT("Pair"))));
		UTextBlock* PlayerLabel = MakeText(*WidgetTree,
			FName(*(TEXT("Representative") + Representative.Slug + TEXT("Label"))),
			Representative.Label);
		PlayerLabel->SetAutoWrapText(false);
		PlayerLabel->SetJustification(ETextJustify::Center);
		FFMCodexPlayerUIStyle::Get().ApplyText(
			*PlayerLabel, EFMCodexPlayerUITextRole::Secondary);
		PlayerPair->AddChildToVerticalBox(PlayerLabel);
		UHorizontalBox* PairColumns = WidgetTree->ConstructWidget<UHorizontalBox>(
			UHorizontalBox::StaticClass(),
			FName(*(TEXT("Representative") + Representative.Slug + TEXT("Columns"))));
		PlayerPair->AddChildToVerticalBox(PairColumns);

		FFMCodexUMGCardViewModel CardModel;
		CardModel.CardId = Representative.CardId;
		CardModel.IdentityLabel = Representative.Label;
		CardModel.OwnerLabel = TEXT("Diagnostic");
		CardModel.RoleLabel = Representative.bGoalkeeper ? TEXT("GK") : TEXT("A");
		CardModel.RarityLabel = TEXT("Continental");
		CardModel.bGoalkeeper = Representative.bGoalkeeper;
		UFMCodexPlayerCardWidget* HighResolutionCard = nullptr;
		AddDiagnosticColumn(PairColumns, CardModel,
			FName(*(TEXT("Representative") + Representative.Slug + TEXT("HighColumn"))),
			TEXT("B HIGH-RES"),
			FName(*(TEXT("Representative") + Representative.Slug + TEXT("HighView"))),
			FName(*(TEXT("Representative") + Representative.Slug + TEXT("HighCard"))),
			nullptr, false, HighResolutionCard);
		HandMicroRepresentativeHighResolutionCards.Add(HighResolutionCard);

		UTexture2D* RuntimeTexture = LoadObject<UTexture2D>(
			nullptr, *Representative.RuntimeTexturePath);
		UFMCodexPlayerCardWidget* RuntimeVariantCard = nullptr;
		AddDiagnosticColumn(PairColumns, CardModel,
			FName(*(TEXT("Representative") + Representative.Slug + TEXT("RuntimeColumn"))),
			TEXT("C RUNTIME192"),
			FName(*(TEXT("Representative") + Representative.Slug + TEXT("RuntimeView"))),
			FName(*(TEXT("Representative") + Representative.Slug + TEXT("RuntimeCard"))),
			RuntimeTexture, false, RuntimeVariantCard);
		HandMicroRepresentativeRuntimeCards.Add(RuntimeVariantCard);
		if (UUniformGridSlot* PairSlot =
			RepresentativeGrid->AddChildToUniformGrid(PlayerPair, Index / 2, Index % 2))
		{
			PairSlot->SetHorizontalAlignment(HAlign_Center);
			PairSlot->SetVerticalAlignment(VAlign_Center);
		}
	}

	auto BuildConformancePage = [this, DiagnosticPageStack,
		&Representatives, &AddDiagnosticColumn](
		const FName PageName,
		const FString& Title,
		const int32 FirstRepresentative,
		const bool bRebalanceComparison) -> UWidget*
	{
		UVerticalBox* Page = WidgetTree->ConstructWidget<UVerticalBox>(
			UVerticalBox::StaticClass(), PageName);
		UTextBlock* PageTitle = MakeText(*WidgetTree,
			FName(*(PageName.ToString() + TEXT("Title"))), Title);
		PageTitle->SetAutoWrapText(false);
		PageTitle->SetJustification(ETextJustify::Center);
		FFMCodexPlayerUIStyle::Get().ApplyText(
			*PageTitle, EFMCodexPlayerUITextRole::Status);
		Page->AddChildToVerticalBox(PageTitle);
		UUniformGridPanel* Grid = WidgetTree->ConstructWidget<UUniformGridPanel>(
			UUniformGridPanel::StaticClass(),
			FName(*(PageName.ToString() + TEXT("Grid"))));
		Grid->SetSlotPadding(FMargin(8.0f, 5.0f));
		Page->AddChildToVerticalBox(Grid);
		DiagnosticPageStack->AddChildToOverlay(Page);

		for (int32 LocalIndex = 0; LocalIndex < 3; ++LocalIndex)
		{
			const FRepresentativePortrait& Representative =
				Representatives[FirstRepresentative + LocalIndex];
			const FString Prefix = PageName.ToString() + Representative.Slug;
			UVerticalBox* PlayerPair = WidgetTree->ConstructWidget<UVerticalBox>(
				UVerticalBox::StaticClass(), FName(*(Prefix + TEXT("Pair"))));
			UTextBlock* PlayerLabel = MakeText(*WidgetTree,
				FName(*(Prefix + TEXT("Label"))), Representative.Label);
			PlayerLabel->SetAutoWrapText(false);
			PlayerLabel->SetJustification(ETextJustify::Center);
			FFMCodexPlayerUIStyle::Get().ApplyText(
				*PlayerLabel, EFMCodexPlayerUITextRole::Secondary);
			PlayerPair->AddChildToVerticalBox(PlayerLabel);
			UHorizontalBox* PairColumns =
				WidgetTree->ConstructWidget<UHorizontalBox>(
					UHorizontalBox::StaticClass(),
					FName(*(Prefix + TEXT("Columns"))));
			PlayerPair->AddChildToVerticalBox(PairColumns);

			FFMCodexUMGCardViewModel CardModel;
			CardModel.CardId = Representative.CardId;
			CardModel.IdentityLabel = Representative.Label;
			CardModel.OwnerLabel = TEXT("Diagnostic");
			CardModel.RoleLabel = Representative.bGoalkeeper ? TEXT("GK") : TEXT("A");
			CardModel.RarityLabel = TEXT("Continental");
			CardModel.bGoalkeeper = Representative.bGoalkeeper;

			const FString& CurrentTexturePath = bRebalanceComparison
				? Representative.ConformedTexturePath
				: Representative.RuntimeTexturePath;
			UTexture2D* CurrentTexture = LoadObject<UTexture2D>(
				nullptr, *CurrentTexturePath);
			UFMCodexPlayerCardWidget* CurrentCard = nullptr;
			AddDiagnosticColumn(PairColumns, CardModel,
				FName(*(Prefix + TEXT("CurrentColumn"))),
				bRebalanceComparison ? TEXT("D1 PREVIOUS") : TEXT("C CURRENT192"),
				FName(*(Prefix + TEXT("CurrentView"))),
				FName(*(Prefix + TEXT("CurrentCard"))),
				CurrentTexture, false, CurrentCard);
			if (bRebalanceComparison)
			{
				HandMicroRebalancePreviousCards.Add(CurrentCard);
			}
			else
			{
				HandMicroConformanceCurrentCards.Add(CurrentCard);
			}

			const FString& CandidateTexturePath = bRebalanceComparison
				? Representative.RebalancedTexturePath
				: Representative.ConformedTexturePath;
			UTexture2D* CandidateTexture = LoadObject<UTexture2D>(
				nullptr, *CandidateTexturePath);
			UFMCodexPlayerCardWidget* CandidateCard = nullptr;
			AddDiagnosticColumn(PairColumns, CardModel,
				FName(*(Prefix + TEXT("CandidateColumn"))),
				bRebalanceComparison ? TEXT("D2 REBALANCED") : TEXT("D CONFORMED192"),
				FName(*(Prefix + TEXT("CandidateView"))),
				FName(*(Prefix + TEXT("CandidateCard"))),
				CandidateTexture, false, CandidateCard);
			if (bRebalanceComparison)
			{
				HandMicroRebalanceCandidateCards.Add(CandidateCard);
			}
			else
			{
				HandMicroConformanceCandidateCards.Add(CandidateCard);
			}

			if (UUniformGridSlot* PairSlot =
				Grid->AddChildToUniformGrid(PlayerPair, LocalIndex, 0))
			{
				PairSlot->SetHorizontalAlignment(HAlign_Center);
				PairSlot->SetVerticalAlignment(VAlign_Center);
			}
		}
		return Page;
	};
	HandMicroArtConformancePage2 = BuildConformancePage(
		TEXT("HandMicroArtConformancePage2"),
		TEXT("ART CONFORMANCE C/D — RAYA / SALIBA / SAKA — 96x64"),
		0, false);
	HandMicroArtConformancePage3 = BuildConformancePage(
		TEXT("HandMicroArtConformancePage3"),
		TEXT("ART CONFORMANCE C/D — ODEGAARD / DONNARUMMA / HAALAND — 96x64"),
		3, false);
	HandMicroPortraitRebalancePage4 = BuildConformancePage(
		TEXT("HandMicroPortraitRebalancePage4"),
		TEXT("PORTRAIT REBALANCE D1/D2 — RAYA / SALIBA / SAKA — 96x64"),
		0, true);
	HandMicroPortraitRebalancePage5 = BuildConformancePage(
		TEXT("HandMicroPortraitRebalancePage5"),
		TEXT("PORTRAIT REBALANCE D1/D2 — ODEGAARD / DONNARUMMA / HAALAND — 96x64"),
		3, true);

	auto BuildReferenceAPage = [this, DiagnosticPageStack,
		&Representatives, &AddDiagnosticColumn](
		const FName PageName,
		const FString& Title,
		const int32 FirstRepresentative) -> UWidget*
	{
		UVerticalBox* Page = WidgetTree->ConstructWidget<UVerticalBox>(
			UVerticalBox::StaticClass(), PageName);
		UTextBlock* PageTitle = MakeText(*WidgetTree,
			FName(*(PageName.ToString() + TEXT("Title"))), Title);
		PageTitle->SetAutoWrapText(false);
		PageTitle->SetJustification(ETextJustify::Center);
		FFMCodexPlayerUIStyle::Get().ApplyText(
			*PageTitle, EFMCodexPlayerUITextRole::Status);
		Page->AddChildToVerticalBox(PageTitle);
		UUniformGridPanel* Grid = WidgetTree->ConstructWidget<UUniformGridPanel>(
			UUniformGridPanel::StaticClass(),
			FName(*(PageName.ToString() + TEXT("Grid"))));
		Grid->SetSlotPadding(FMargin(8.0f, 7.0f));
		Page->AddChildToVerticalBox(Grid);
		DiagnosticPageStack->AddChildToOverlay(Page);

		for (int32 LocalIndex = 0; LocalIndex < 2; ++LocalIndex)
		{
			const FRepresentativePortrait& Representative =
				Representatives[FirstRepresentative + LocalIndex];
			const FString Prefix = PageName.ToString() + Representative.Slug;
			UVerticalBox* PlayerPair = WidgetTree->ConstructWidget<UVerticalBox>(
				UVerticalBox::StaticClass(), FName(*(Prefix + TEXT("Pair"))));
			UTextBlock* PlayerLabel = MakeText(*WidgetTree,
				FName(*(Prefix + TEXT("Label"))), Representative.Label);
			PlayerLabel->SetAutoWrapText(false);
			PlayerLabel->SetJustification(ETextJustify::Center);
			FFMCodexPlayerUIStyle::Get().ApplyText(
				*PlayerLabel, EFMCodexPlayerUITextRole::Secondary);
			PlayerPair->AddChildToVerticalBox(PlayerLabel);
			UHorizontalBox* PairColumns =
				WidgetTree->ConstructWidget<UHorizontalBox>(
					UHorizontalBox::StaticClass(),
					FName(*(Prefix + TEXT("Columns"))));
			PlayerPair->AddChildToVerticalBox(PairColumns);

			FFMCodexUMGCardViewModel CardModel;
			CardModel.CardId = Representative.CardId;
			CardModel.IdentityLabel = Representative.Label;
			CardModel.OwnerLabel = TEXT("Diagnostic");
			CardModel.RoleLabel = Representative.bGoalkeeper ? TEXT("GK") : TEXT("A");
			CardModel.RarityLabel = TEXT("Continental");
			CardModel.bGoalkeeper = Representative.bGoalkeeper;
			UTexture2D* D2Texture = LoadObject<UTexture2D>(
				nullptr, *Representative.RebalancedTexturePath);
			UTexture2D* D3Texture = LoadObject<UTexture2D>(
				nullptr, *Representative.ReferenceATexturePath);
			UFMCodexPlayerCardWidget* D2Card = nullptr;
			AddDiagnosticColumn(PairColumns, CardModel,
				FName(*(Prefix + TEXT("D2Column"))), TEXT("D2 BASELINE"),
				FName(*(Prefix + TEXT("D2View"))),
				FName(*(Prefix + TEXT("D2Card"))), D2Texture, false, D2Card);
			UFMCodexPlayerCardWidget* D3Card = nullptr;
			AddDiagnosticColumn(PairColumns, CardModel,
				FName(*(Prefix + TEXT("D3Column"))), TEXT("D3 REFERENCE-A"),
				FName(*(Prefix + TEXT("D3View"))),
				FName(*(Prefix + TEXT("D3Card"))), D3Texture, false, D3Card);
			if (UUniformGridSlot* PairSlot =
				Grid->AddChildToUniformGrid(PlayerPair, LocalIndex, 0))
			{
				PairSlot->SetHorizontalAlignment(HAlign_Center);
				PairSlot->SetVerticalAlignment(VAlign_Center);
			}
		}
		return Page;
	};
	HandMicroReferenceAPage6 = BuildReferenceAPage(
		TEXT("HandMicroReferenceAPage6"),
		TEXT("REFERENCE-A PORTRAIT D2/D3 — RAYA / SALIBA — 96x64"), 0);
	HandMicroReferenceAPage7 = BuildReferenceAPage(
		TEXT("HandMicroReferenceAPage7"),
		TEXT("REFERENCE-A PORTRAIT D2/D3 — SAKA / ODEGAARD — 96x64"), 2);
	HandMicroReferenceAPage8 = BuildReferenceAPage(
		TEXT("HandMicroReferenceAPage8"),
		TEXT("REFERENCE-A PORTRAIT D2/D3 — DONNARUMMA / HAALAND — 96x64"), 4);

	auto BuildFullCardColumn = [this](
		UHorizontalBox* Columns,
		const FFMCodexUMGCardViewModel& CardModel,
		const FString& Prefix,
		const FString& Label,
		const bool bUnifiedName,
		const bool bHeight68,
		UTexture2D* Portrait)
	{
		UVerticalBox* Column = WidgetTree->ConstructWidget<UVerticalBox>(
			UVerticalBox::StaticClass(), FName(*(Prefix + TEXT("Column"))));
		UTextBlock* ColumnLabel = MakeText(*WidgetTree,
			FName(*(Prefix + TEXT("Label"))), Label);
		ColumnLabel->SetAutoWrapText(false);
		ColumnLabel->SetJustification(ETextJustify::Center);
		FFMCodexPlayerUIStyle::Get().ApplyText(
			*ColumnLabel, EFMCodexPlayerUITextRole::Secondary);
		Column->AddChildToVerticalBox(ColumnLabel);
		UFMCodexPlayerCardWidget* Card =
			WidgetTree->ConstructWidget<UFMCodexPlayerCardWidget>(
				UFMCodexPlayerCardWidget::StaticClass(),
				FName(*(Prefix + TEXT("Card"))));
		Card->RefreshFromPresentation(
			CardModel, EFMCodexPlayerCardPresentationMode::HandMicro);
		Card->SetDiagnosticHandMicroUnifiedNameOverride(bUnifiedName);
		Card->SetDiagnosticHandMicroHeight68Override(bHeight68);
		if (Portrait != nullptr)
		{
			Card->SetDiagnosticHandMicroPortraitOverride(Portrait);
		}
		Column->AddChildToVerticalBox(Card);
		if (UHorizontalBoxSlot* ColumnSlot = Columns->AddChildToHorizontalBox(Column))
		{
			ColumnSlot->SetPadding(FMargin(7.0f, 2.0f));
		}
	};

	UVerticalBox* TypographyPage = WidgetTree->ConstructWidget<UVerticalBox>(
		UVerticalBox::StaticClass(), TEXT("HandMicroTypographyPage9"));
	HandMicroTypographyPage9 = TypographyPage;
	UTextBlock* TypographyTitle = MakeText(*WidgetTree,
		TEXT("HandMicroTypographyPage9Title"),
		TEXT("NAME RHYTHM — CURRENT MAXIMIZE VS CANDIDATE STANDARD 16→12"));
	TypographyTitle->SetAutoWrapText(false);
	TypographyTitle->SetJustification(ETextJustify::Center);
	FFMCodexPlayerUIStyle::Get().ApplyText(
		*TypographyTitle, EFMCodexPlayerUITextRole::Status);
	TypographyPage->AddChildToVerticalBox(TypographyTitle);
	UUniformGridPanel* TypographyGrid = WidgetTree->ConstructWidget<UUniformGridPanel>(
		UUniformGridPanel::StaticClass(), TEXT("HandMicroTypographyPage9Grid"));
	TypographyGrid->SetSlotPadding(FMargin(0.0f, 3.0f));
	TypographyPage->AddChildToVerticalBox(TypographyGrid);
	DiagnosticPageStack->AddChildToOverlay(TypographyPage);
	struct FTypographyCase
	{
		FName CardId;
		FString Name;
		FString Slug;
	};
	const TArray<FTypographyCase> TypographyCases = {
		{ TEXT("Prototype.Arsenal.DavidRaya"), TEXT("拉亚"), TEXT("Raya") },
		{ TEXT("Prototype.Arsenal.WilliamSaliba"), TEXT("萨利巴"), TEXT("Saliba") },
		{ TEXT("Demo.A.Outfield.01"), TEXT("马丁内利"), TEXT("Martinelli") },
		{ TEXT("Demo.A.Outfield.02"), TEXT("加布里埃尔"), TEXT("Gabriel") },
		{ TEXT("Visual.HandMicro.Kvaratskhelia"),
			TEXT("克瓦拉茨赫利亚"), TEXT("Stress") }
	};
	for (int32 Index = 0; Index < TypographyCases.Num(); ++Index)
	{
		const FTypographyCase& TypographyCase = TypographyCases[Index];
		UHorizontalBox* Columns = WidgetTree->ConstructWidget<UHorizontalBox>(
			UHorizontalBox::StaticClass(),
			FName(*(TEXT("Typography") + TypographyCase.Slug + TEXT("Columns"))));
		FFMCodexUMGCardViewModel CardModel;
		CardModel.CardId = TypographyCase.CardId;
		CardModel.IdentityLabel = TypographyCase.Name;
		CardModel.OwnerLabel = TEXT("Diagnostic");
		CardModel.RoleLabel = TEXT("AMD");
		CardModel.RarityLabel = TEXT("Continental");
		BuildFullCardColumn(Columns, CardModel,
			TEXT("Typography") + TypographyCase.Slug + TEXT("Current"),
			TEXT("CURRENT"), false, false, nullptr);
		BuildFullCardColumn(Columns, CardModel,
			TEXT("Typography") + TypographyCase.Slug + TEXT("Candidate"),
			TEXT("CANDIDATE"), true, false, nullptr);
		if (UUniformGridSlot* TypographyGridSlot =
			TypographyGrid->AddChildToUniformGrid(Columns, Index, 0))
		{
			TypographyGridSlot->SetHorizontalAlignment(HAlign_Center);
			TypographyGridSlot->SetVerticalAlignment(VAlign_Center);
		}
	}

	UVerticalBox* HeightPage = WidgetTree->ConstructWidget<UVerticalBox>(
		UVerticalBox::StaticClass(), TEXT("HandMicroHeightPage10"));
	HandMicroHeightPage10 = HeightPage;
	UTextBlock* HeightTitle = MakeText(*WidgetTree,
		TEXT("HandMicroHeightPage10Title"),
		TEXT("HEIGHT A/B — D3 + STANDARD NAME HELD CONSTANT — 220x64 VS 220x68"));
	HeightTitle->SetAutoWrapText(false);
	HeightTitle->SetJustification(ETextJustify::Center);
	FFMCodexPlayerUIStyle::Get().ApplyText(
		*HeightTitle, EFMCodexPlayerUITextRole::Status);
	HeightPage->AddChildToVerticalBox(HeightTitle);
	UHorizontalBox* HeightColumns = WidgetTree->ConstructWidget<UHorizontalBox>(
		UHorizontalBox::StaticClass(), TEXT("HandMicroHeightPage10Columns"));
	HeightPage->AddChildToVerticalBox(HeightColumns);
	FFMCodexUMGCardViewModel HeightCardModel;
	HeightCardModel.CardId = TEXT("Prototype.Arsenal.DavidRaya");
	HeightCardModel.IdentityLabel = TEXT("拉亚");
	HeightCardModel.OwnerLabel = TEXT("Diagnostic");
	HeightCardModel.RoleLabel = TEXT("GK");
	HeightCardModel.RarityLabel = TEXT("Continental");
	HeightCardModel.bGoalkeeper = true;
	UTexture2D* HeightD3Texture = LoadObject<UTexture2D>(
		nullptr, *Representatives[0].ReferenceATexturePath);
	BuildFullCardColumn(HeightColumns, HeightCardModel,
		TEXT("HandMicroHeightPage10Baseline"), TEXT("A  220x64"),
		true, false, HeightD3Texture);
	BuildFullCardColumn(HeightColumns, HeightCardModel,
		TEXT("HandMicroHeightPage10Candidate"), TEXT("B  220x68"),
		true, true, HeightD3Texture);
	DiagnosticPageStack->AddChildToOverlay(HeightPage);

	HandMicroSharpnessDiagnosticBounds = WidgetTree->ConstructWidget<USizeBox>(
		USizeBox::StaticClass(), TEXT("HandMicroSharpnessDiagnosticBounds"));
	HandMicroSharpnessDiagnosticBounds->SetWidthOverride(560.0f);
	HandMicroSharpnessDiagnosticBounds->SetHeightOverride(154.0f);
	HandMicroSharpnessDiagnosticBounds->AddChild(
		HandMicroSharpnessDiagnosticSurface);
	}
#endif

	ResolutionOverlay = MakeRegion(
		*WidgetTree, TEXT("ResolutionPresentationLayer"));
	USizeBox* ResolutionBounds = WidgetTree->ConstructWidget<USizeBox>(
		USizeBox::StaticClass(), TEXT("ResolutionResultRegion"));
	ResolutionBounds->SetWidthOverride(720.0f);
	ResolutionBounds->SetMaxDesiredHeight(640.0f);
	UClass* ResolvedResolutionClass = ResolutionPanelWidgetClass != nullptr
		? ResolutionPanelWidgetClass.Get()
		: UFMCodexResolutionPanelWidget::StaticClass();
	ResolutionPanel =
		WidgetTree->ConstructWidget<UFMCodexResolutionPanelWidget>(
			ResolvedResolutionClass, TEXT("DedicatedResolutionPanelWidget"));
	ResolutionBounds->AddChild(ResolutionPanel);
	if (UBorderSlot* ResolutionSlot = Cast<UBorderSlot>(
		ResolutionOverlay->AddChild(ResolutionBounds)))
	{
		ResolutionSlot->SetHorizontalAlignment(HAlign_Center);
		ResolutionSlot->SetVerticalAlignment(VAlign_Center);
	}
	if (UOverlaySlot* OverlaySlot = Root->AddChildToOverlay(ResolutionOverlay))
	{
		OverlaySlot->SetHorizontalAlignment(HAlign_Fill);
		OverlaySlot->SetVerticalAlignment(VAlign_Fill);
	}

	HandoffOverlay = MakeRegion(*WidgetTree, TEXT("HotSeatHandoffOverlay"));
	const FFMCodexPlayerUIStyle& Style = FFMCodexPlayerUIStyle::Get();
	Style.ApplyBorder(*HandoffOverlay,
		EFMCodexPlayerUIColorRole::ScreenBackground, FMargin(0.0f));
	USizeBox* HandoffBounds = WidgetTree->ConstructWidget<USizeBox>(
		USizeBox::StaticClass(), TEXT("HotSeatHandoffBounds"));
	HandoffBounds->SetWidthOverride(520.0f);
	HandoffBounds->SetMinDesiredHeight(260.0f);
	if (UBorderSlot* ModalSlot = Cast<UBorderSlot>(
		HandoffOverlay->AddChild(HandoffBounds)))
	{
		ModalSlot->SetHorizontalAlignment(HAlign_Center);
		ModalSlot->SetVerticalAlignment(VAlign_Center);
	}
	UBorder* HandoffCard = WidgetTree->ConstructWidget<UBorder>(
		UBorder::StaticClass(), TEXT("HotSeatHandoffCard"));
	Style.ApplyBorder(*HandoffCard,
		EFMCodexPlayerUIColorRole::PanelRaised, Style.GetPanelPadding());
	HandoffBounds->AddChild(HandoffCard);
	UVerticalBox* HandoffBody = WidgetTree->ConstructWidget<UVerticalBox>(
		UVerticalBox::StaticClass(), TEXT("HotSeatHandoffBody"));
	HandoffCard->AddChild(HandoffBody);
	HandoffTitleText = MakeText(*WidgetTree, TEXT("HotSeatHandoffText"));
	HandoffTitleText->SetJustification(ETextJustify::Center);
	Style.ApplyText(
		*HandoffTitleText, EFMCodexPlayerUITextRole::HandoffTitle);
	HandoffPlayerText = MakeText(
		*WidgetTree, TEXT("HotSeatNextPlayerText"));
	HandoffPlayerText->SetJustification(ETextJustify::Center);
	Style.ApplyText(
		*HandoffPlayerText, EFMCodexPlayerUITextRole::HandoffPlayer);
	HandoffReadyText = MakeText(
		*WidgetTree, TEXT("HotSeatReadyInstruction"));
	HandoffReadyText->SetJustification(ETextJustify::Center);
	Style.ApplyText(
		*HandoffReadyText, EFMCodexPlayerUITextRole::Body);
	HandoffBody->AddChildToVerticalBox(HandoffTitleText);
	HandoffBody->AddChildToVerticalBox(HandoffPlayerText);
	HandoffBody->AddChildToVerticalBox(HandoffReadyText);
	UButton* ReadyButton = MakeButton(
		*WidgetTree, TEXT("HotSeatReadyButton"), TEXT("Ready"));
	ReadyButton->OnClicked.AddDynamic(
		this, &UFMCodexLocalMatchScreenWidget::HandleReadyClicked);
	HandoffBody->AddChildToVerticalBox(ReadyButton);
	if (UOverlaySlot* HandoffSlot = Root->AddChildToOverlay(HandoffOverlay))
	{
		HandoffSlot->SetHorizontalAlignment(HAlign_Fill);
		HandoffSlot->SetVerticalAlignment(VAlign_Fill);
	}

#if !UE_BUILD_SHIPPING
	// Keep the development comparison above every full-screen player-facing
	// modal. Its previous insertion point left it underneath Resolution and
	// Hot-Seat Handoff, so a valid enabled CVar could still produce no visible
	// pixels in the real Match Screen.
	if (HandMicroSharpnessDiagnosticBounds != nullptr)
	{
		if (UOverlaySlot* DiagnosticSlot = Root->AddChildToOverlay(
			HandMicroSharpnessDiagnosticBounds))
		{
			DiagnosticSlot->SetHorizontalAlignment(HAlign_Center);
			DiagnosticSlot->SetVerticalAlignment(VAlign_Center);
		}
	}
#endif
}

void UFMCodexLocalMatchScreenWidget::RefreshVisuals()
{
	if (MatchHeader == nullptr)
	{
		return;
	}
	const bool bFullNameCandidate =
		FMCodexHandMicroDiagnostics::IsFullNameCandidateEnabled();
	LocalRackBounds->SetWidthOverride(bFullNameCandidate
		? FMCodexHandMicroDiagnostics::CandidateRackWidth
		: FMCodexHandMicroDiagnostics::ProductionRackWidth);
	PitchBounds->SetWidthOverride(bFullNameCandidate
		? FMCodexHandMicroDiagnostics::CandidatePitchWidth
		: FMCodexHandMicroDiagnostics::ProductionPitchWidth);
	OpponentRackBounds->SetWidthOverride(bFullNameCandidate
		? FMCodexHandMicroDiagnostics::CandidateRackWidth
		: FMCodexHandMicroDiagnostics::ProductionRackWidth);
#if !UE_BUILD_SHIPPING
	if (HandMicroSharpnessDiagnosticBounds != nullptr)
	{
		const int32 DiagnosticPage =
			FMCodexHandMicroDiagnostics::GetSharpnessComparisonPage();
		if (HandMicroSharpnessRayaPage != nullptr)
		{
			HandMicroSharpnessRayaPage->SetVisibility(DiagnosticPage == 0
				? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
		}
		if (HandMicroSharpnessRepresentativePage != nullptr)
		{
			HandMicroSharpnessRepresentativePage->SetVisibility(DiagnosticPage == 1
				? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
		}
		if (HandMicroArtConformancePage2 != nullptr)
		{
			HandMicroArtConformancePage2->SetVisibility(DiagnosticPage == 2
				? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
		}
		if (HandMicroArtConformancePage3 != nullptr)
		{
			HandMicroArtConformancePage3->SetVisibility(DiagnosticPage == 3
				? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
		}
		if (HandMicroPortraitRebalancePage4 != nullptr)
		{
			HandMicroPortraitRebalancePage4->SetVisibility(DiagnosticPage == 4
				? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
		}
		if (HandMicroPortraitRebalancePage5 != nullptr)
		{
			HandMicroPortraitRebalancePage5->SetVisibility(DiagnosticPage == 5
				? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
		}
		if (HandMicroReferenceAPage6 != nullptr)
		{
			HandMicroReferenceAPage6->SetVisibility(DiagnosticPage == 6
				? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
		}
		if (HandMicroReferenceAPage7 != nullptr)
		{
			HandMicroReferenceAPage7->SetVisibility(DiagnosticPage == 7
				? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
		}
		if (HandMicroReferenceAPage8 != nullptr)
		{
			HandMicroReferenceAPage8->SetVisibility(DiagnosticPage == 8
				? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
		}
		if (HandMicroTypographyPage9 != nullptr)
		{
			HandMicroTypographyPage9->SetVisibility(DiagnosticPage == 9
				? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
		}
		if (HandMicroHeightPage10 != nullptr)
		{
			HandMicroHeightPage10->SetVisibility(DiagnosticPage == 10
				? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
		}
		HandMicroSharpnessDiagnosticBounds->SetHeightOverride(
			DiagnosticPage == 0 ? 154.0f
				: DiagnosticPage >= 6 && DiagnosticPage <= 8 ? 258.0f
					: DiagnosticPage == 9 ? 520.0f
						: DiagnosticPage == 10 ? 190.0f : 352.0f);
		HandMicroSharpnessDiagnosticBounds->SetVisibility(
			FMCodexHandMicroDiagnostics::IsSharpnessComparisonEnabled()
				? ESlateVisibility::HitTestInvisible
				: ESlateVisibility::Collapsed);
	}
#endif
	MatchHeader->RefreshFromPresentation(Presentation.Header);
	LocalRackWidget->RefreshFromPresentation(Presentation.LocalRack);
	OpponentRackWidget->RefreshFromPresentation(Presentation.OpponentRack);
	PitchWidget->RefreshFromPitchPresentation(Presentation.PitchRegions);
	InteractionPanel->RefreshFromPresentation(Presentation.Interaction);
	ResolutionPanel->RefreshFromPresentation(Presentation.Resolution);
	ResolutionOverlay->SetVisibility(Presentation.Resolution.bVisible
		? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);

	HandoffOverlay->SetVisibility(Presentation.Handoff.bVisible
		? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	MainScreen->SetIsEnabled(!Presentation.Handoff.bVisible);
	InteractionPanel->SetInteractionBlocked(Presentation.Handoff.bVisible);
	HandoffTitleText->SetText(FText::FromString(
		Presentation.Handoff.TitleLabel));
	HandoffPlayerText->SetText(FText::FromString(
		Presentation.Handoff.NextPlayerLabel));
	HandoffPlayerText->SetColorAndOpacity(FSlateColor(
		FFMCodexPlayerUIStyle::Get().GetPlayerAccentColor(
			Presentation.Handoff.NextPlayerLabel)));
	HandoffReadyText->SetText(FText::FromString(
		Presentation.Handoff.ReadyLabel));
}
