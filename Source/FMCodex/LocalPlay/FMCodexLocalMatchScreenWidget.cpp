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
	if (FMCodexHandMicroDiagnostics::IsProductionReviewEnabled())
	{
		HandMicroProductionReviewSurface = WidgetTree->ConstructWidget<UBorder>(
			UBorder::StaticClass(), TEXT("HandMicroProductionReviewSurface"));
		HandMicroProductionReviewSurface->SetBrushColor(
			FLinearColor::FromSRGBColor(FColor(0x08, 0x16, 0x20)));
		HandMicroProductionReviewSurface->SetPadding(FMargin(16.0f, 12.0f));
		HandMicroProductionReviewSurface->SetClipping(
			EWidgetClipping::ClipToBounds);
		UOverlay* ReviewPageStack = WidgetTree->ConstructWidget<UOverlay>(
			UOverlay::StaticClass(), TEXT("HandMicroProductionReviewPageStack"));
		HandMicroProductionReviewSurface->AddChild(ReviewPageStack);

		struct FProductionReviewCase
		{
			FName CardId;
			FString Name;
			FString Position;
			FString Slug;
			FString Rarity;
			bool bGoalkeeper = false;
		};
		const TArray<FProductionReviewCase> ProductionCases = {
			{ TEXT("Prototype.Arsenal.DavidRaya"), TEXT("拉亚"), TEXT("GK"),
				TEXT("Raya"), TEXT("Legendary"), true },
			{ TEXT("Prototype.Arsenal.WilliamSaliba"), TEXT("萨利巴"), TEXT("D"),
				TEXT("Saliba"), TEXT("Continental") },
			{ TEXT("Prototype.Arsenal.DeclanRice"), TEXT("赖斯"), TEXT("M/D"),
				TEXT("Rice"), TEXT("Continental") },
			{ TEXT("Demo.A.Outfield.01"), TEXT("马丁内利"), TEXT("A/M/D"),
				TEXT("Martinelli"), TEXT("Common") },
			{ TEXT("Demo.A.Outfield.02"), TEXT("加布里埃尔"), TEXT("A/M/D"),
				TEXT("Gabriel"), TEXT("Common") },
			{ TEXT("Demo.A.Outfield.03"), TEXT("梅里诺"), TEXT("A/M/D"),
				TEXT("Merino"), TEXT("Common") },
			{ TEXT("Prototype.Arsenal.BukayoSaka"), TEXT("萨卡"), TEXT("A/M"),
				TEXT("Saka"), TEXT("Legendary") },
			{ TEXT("Prototype.Arsenal.MartinOdegaard"), TEXT("厄德高"), TEXT("M/A"),
				TEXT("Odegaard"), TEXT("Continental") },
			{ TEXT("Prototype.ManchesterCity.GianluigiDonnarumma"),
				TEXT("多纳鲁马"), TEXT("GK"), TEXT("Donnarumma"),
				TEXT("Continental"), true },
			{ TEXT("Prototype.ManchesterCity.RubenDias"), TEXT("迪亚斯"), TEXT("D"),
				TEXT("Dias"), TEXT("Continental") },
			{ TEXT("Prototype.ManchesterCity.Rodri"), TEXT("罗德里"), TEXT("M/D"),
				TEXT("Rodri"), TEXT("Continental") },
			{ TEXT("Demo.B.Outfield.01"), TEXT("格瓦迪奥尔"), TEXT("A/M/D"),
				TEXT("Gvardiol"), TEXT("Common") },
			{ TEXT("Demo.B.Outfield.02"), TEXT("贝尔纳多"), TEXT("A/M/D"),
				TEXT("Bernardo"), TEXT("Common") },
			{ TEXT("Demo.B.Outfield.03"), TEXT("多库"), TEXT("A/M/D"),
				TEXT("Doku"), TEXT("Common") },
			{ TEXT("Prototype.ManchesterCity.PhilFoden"), TEXT("福登"), TEXT("A/M"),
				TEXT("Foden"), TEXT("Continental") },
			{ TEXT("Prototype.ManchesterCity.ErlingHaaland"), TEXT("哈兰德"), TEXT("A"),
				TEXT("Haaland"), TEXT("Legendary") }
		};
		auto MakeReviewCard = [this](const FProductionReviewCase& ReviewCase,
			const FString& Prefix) -> UFMCodexPlayerCardWidget*
		{
			FFMCodexUMGCardViewModel CardModel;
			CardModel.CardId = ReviewCase.CardId;
			CardModel.IdentityLabel = ReviewCase.Name;
			CardModel.OwnerLabel = TEXT("Production Review");
			CardModel.RoleLabel = ReviewCase.Position;
			CardModel.RarityLabel = ReviewCase.Rarity;
			CardModel.bGoalkeeper = ReviewCase.bGoalkeeper;
			UFMCodexPlayerCardWidget* Card =
				WidgetTree->ConstructWidget<UFMCodexPlayerCardWidget>(
					UFMCodexPlayerCardWidget::StaticClass(),
					FName(*(Prefix + ReviewCase.Slug + TEXT("Card"))));
			Card->RefreshFromPresentation(
				CardModel, EFMCodexPlayerCardPresentationMode::HandMicro);
			return Card;
		};

		UVerticalBox* PortraitsPage = WidgetTree->ConstructWidget<UVerticalBox>(
			UVerticalBox::StaticClass(),
			TEXT("HandMicroProductionReviewPortraitsPage"));
		HandMicroProductionReviewPortraitsPage = PortraitsPage;
		UTextBlock* PortraitsTitle = MakeText(*WidgetTree,
			TEXT("HandMicroProductionReviewPortraitsTitle"),
			TEXT("HAND MICRO PRODUCTION PORTRAITS — APPROVED 16"));
		PortraitsTitle->SetAutoWrapText(false);
		PortraitsTitle->SetJustification(ETextJustify::Center);
		FFMCodexPlayerUIStyle::Get().ApplyText(
			*PortraitsTitle, EFMCodexPlayerUITextRole::Status);
		PortraitsPage->AddChildToVerticalBox(PortraitsTitle);
		UUniformGridPanel* PortraitsGrid =
			WidgetTree->ConstructWidget<UUniformGridPanel>(
				UUniformGridPanel::StaticClass(),
				TEXT("HandMicroProductionReviewPortraitsGrid"));
		PortraitsGrid->SetSlotPadding(FMargin(5.0f, 3.0f));
		PortraitsPage->AddChildToVerticalBox(PortraitsGrid);
		ReviewPageStack->AddChildToOverlay(PortraitsPage);
		for (int32 Index = 0; Index < ProductionCases.Num(); ++Index)
		{
			UFMCodexPlayerCardWidget* Card = MakeReviewCard(
				ProductionCases[Index], TEXT("ProductionPortrait"));
			if (UUniformGridSlot* ReviewGridSlot =
				PortraitsGrid->AddChildToUniformGrid(Card, Index / 2, Index % 2))
			{
				ReviewGridSlot->SetHorizontalAlignment(HAlign_Center);
				ReviewGridSlot->SetVerticalAlignment(VAlign_Center);
			}
		}

		UVerticalBox* TypographyPage = WidgetTree->ConstructWidget<UVerticalBox>(
			UVerticalBox::StaticClass(),
			TEXT("HandMicroProductionReviewTypographyPage"));
		HandMicroProductionReviewTypographyPage = TypographyPage;
		UTextBlock* TypographyTitle = MakeText(*WidgetTree,
			TEXT("HandMicroProductionReviewTypographyTitle"),
			TEXT("HAND MICRO PRODUCTION NAME STRESS — 16 SHRINK-ONLY TO 12"));
		TypographyTitle->SetAutoWrapText(false);
		TypographyTitle->SetJustification(ETextJustify::Center);
		FFMCodexPlayerUIStyle::Get().ApplyText(
			*TypographyTitle, EFMCodexPlayerUITextRole::Status);
		TypographyPage->AddChildToVerticalBox(TypographyTitle);
		UUniformGridPanel* TypographyGrid =
			WidgetTree->ConstructWidget<UUniformGridPanel>(
				UUniformGridPanel::StaticClass(),
				TEXT("HandMicroProductionReviewTypographyGrid"));
		TypographyGrid->SetSlotPadding(FMargin(5.0f, 4.0f));
		TypographyPage->AddChildToVerticalBox(TypographyGrid);
		ReviewPageStack->AddChildToOverlay(TypographyPage);
		const TArray<FProductionReviewCase> TypographyCases = {
			ProductionCases[0], ProductionCases[1], ProductionCases[3],
			ProductionCases[4],
			{ TEXT("Visual.HandMicro.Kvaratskhelia"), TEXT("克瓦拉茨赫利亚"),
				TEXT("A/M/D"), TEXT("Stress"), TEXT("Continental") }
		};
		for (int32 Index = 0; Index < TypographyCases.Num(); ++Index)
		{
			UFMCodexPlayerCardWidget* Card = MakeReviewCard(
				TypographyCases[Index], TEXT("ProductionTypography"));
			if (UUniformGridSlot* ReviewGridSlot =
				TypographyGrid->AddChildToUniformGrid(Card, Index, 0))
			{
				ReviewGridSlot->SetHorizontalAlignment(HAlign_Center);
				ReviewGridSlot->SetVerticalAlignment(VAlign_Center);
			}
		}

		UVerticalBox* LayoutPage = WidgetTree->ConstructWidget<UVerticalBox>(
			UVerticalBox::StaticClass(),
			TEXT("HandMicroProductionReviewLayoutPage"));
		HandMicroProductionReviewLayoutPage = LayoutPage;
		UTextBlock* LayoutTitle = MakeText(*WidgetTree,
			TEXT("HandMicroProductionReviewLayoutTitle"),
			TEXT("HAND MICRO PRODUCTION LAYOUT — 2x10 / GHOST / NO PAGING"));
		LayoutTitle->SetAutoWrapText(false);
		LayoutTitle->SetJustification(ETextJustify::Center);
		FFMCodexPlayerUIStyle::Get().ApplyText(
			*LayoutTitle, EFMCodexPlayerUITextRole::Status);
		LayoutPage->AddChildToVerticalBox(LayoutTitle);
		FFMCodexUMGCardRackViewModel ReviewRackModel;
		ReviewRackModel.SideLabel = TEXT("Production Review");
		ReviewRackModel.bLocalRack = true;
		ReviewRackModel.ColumnCount = 2;
		ReviewRackModel.RowCount = 10;
		for (int32 Index = 0; Index < 20; ++Index)
		{
			FFMCodexUMGCardRackCellViewModel& Cell =
				ReviewRackModel.Cells.AddDefaulted_GetRef();
			Cell.StableIndex = Index;
			Cell.bPlayed = Index >= 4;
			const FProductionReviewCase& ReviewCase =
				ProductionCases[Index % ProductionCases.Num()];
			Cell.Card.CardId = ReviewCase.CardId;
			Cell.Card.IdentityLabel = ReviewCase.Name;
			Cell.Card.OwnerLabel = TEXT("Production Review");
			Cell.Card.RoleLabel = ReviewCase.Position;
			Cell.Card.RarityLabel = ReviewCase.Rarity;
			Cell.Card.bGoalkeeper = ReviewCase.bGoalkeeper;
		}
		UFMCodexCardRackWidget* ReviewRack =
			WidgetTree->ConstructWidget<UFMCodexCardRackWidget>(
				UFMCodexCardRackWidget::StaticClass(),
				TEXT("HandMicroProductionReviewRack"));
		ReviewRack->RefreshFromPresentation(ReviewRackModel);
		LayoutPage->AddChildToVerticalBox(ReviewRack);
		ReviewPageStack->AddChildToOverlay(LayoutPage);

		HandMicroProductionReviewBounds = WidgetTree->ConstructWidget<USizeBox>(
			USizeBox::StaticClass(), TEXT("HandMicroProductionReviewBounds"));
		HandMicroProductionReviewBounds->SetWidthOverride(560.0f);
		HandMicroProductionReviewBounds->SetHeightOverride(660.0f);
		HandMicroProductionReviewBounds->AddChild(
			HandMicroProductionReviewSurface);
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
	// Keep the opt-in development review above full-screen presentation layers.
	if (HandMicroProductionReviewBounds != nullptr)
	{
		if (UOverlaySlot* ReviewSlot = Root->AddChildToOverlay(
			HandMicroProductionReviewBounds))
		{
			ReviewSlot->SetHorizontalAlignment(HAlign_Center);
			ReviewSlot->SetVerticalAlignment(VAlign_Center);
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
	LocalRackBounds->SetWidthOverride(FMCodexHandMicroDiagnostics::RackWidth);
	PitchBounds->SetWidthOverride(FMCodexHandMicroDiagnostics::PitchWidth);
	OpponentRackBounds->SetWidthOverride(FMCodexHandMicroDiagnostics::RackWidth);
#if !UE_BUILD_SHIPPING
	if (HandMicroProductionReviewBounds != nullptr)
	{
		const int32 ReviewPage =
			FMCodexHandMicroDiagnostics::GetProductionReviewPage();
		if (HandMicroProductionReviewPortraitsPage != nullptr)
		{
			HandMicroProductionReviewPortraitsPage->SetVisibility(ReviewPage == 0
				? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
		}
		if (HandMicroProductionReviewTypographyPage != nullptr)
		{
			HandMicroProductionReviewTypographyPage->SetVisibility(ReviewPage == 1
				? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
		}
		if (HandMicroProductionReviewLayoutPage != nullptr)
		{
			HandMicroProductionReviewLayoutPage->SetVisibility(ReviewPage == 2
				? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
		}
		HandMicroProductionReviewBounds->SetHeightOverride(
			ReviewPage == 0 ? 660.0f : ReviewPage == 1 ? 430.0f : 830.0f);
		HandMicroProductionReviewBounds->SetVisibility(
			FMCodexHandMicroDiagnostics::IsProductionReviewEnabled()
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
