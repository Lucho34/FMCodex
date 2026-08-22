#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "FMCodexLocalMatchUMGPresentation.h"

#include "FMCodexPlayerCardWidget.generated.h"

class UBorder;
class UDragDropOperation;
class UHorizontalBox;
class UImage;
class UOverlay;
class USizeBox;
class UTextBlock;
class UTexture2D;
class UUniformGridPanel;
class UVerticalBox;
class UWrapBox;
class SWidget;
class UFMCodexDeploymentDragDropOperation;
class UFMCodexPlayerCardWidget;

DECLARE_MULTICAST_DELEGATE_TwoParams(
	FFMCodexDeploymentCardDragStarted, FName, bool);
DECLARE_MULTICAST_DELEGATE(FFMCodexDeploymentCardDragFinished);
DECLARE_MULTICAST_DELEGATE_OneParam(
	FFMCodexPlayerCardDetailHover, UFMCodexPlayerCardWidget*);
DECLARE_MULTICAST_DELEGATE_OneParam(
	FFMCodexPlayerCardOnPitchSelectionRequested, FName);

UENUM(BlueprintType)
enum class EFMCodexPlayerCardPresentationMode : uint8
{
	HandMicro,
	PitchMini,
	PitchCompact,
	InteractionChoice
};

UCLASS(Blueprintable)
class FMCODEX_API UFMCodexPlayerCardWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Local Match|Presentation")
	void RefreshFromPresentation(
		const FFMCodexUMGCardViewModel& InPresentation,
		EFMCodexPlayerCardPresentationMode InMode =
			EFMCodexPlayerCardPresentationMode::InteractionChoice);

	UFUNCTION(BlueprintCallable, Category = "Local Match|Presentation")
	void SetPresentationMode(EFMCodexPlayerCardPresentationMode InMode);

	const FFMCodexUMGCardViewModel& GetPresentation() const;
	EFMCodexPlayerCardPresentationMode GetPresentationMode() const;
	const FString& GetRenderedAttributeSummary() const;
	int32 GetRenderedSkillCount() const;
	int32 GetRenderedAttributeCount() const;
	int32 GetRenderedStatusBadgeCount() const;
	FText GetRenderedIdentityText() const;
	FText GetRenderedPositionText() const;
	FText GetRenderedRarityText() const;
	FText GetRenderedTeamText() const;
	int32 GetRenderedBiographyRowCount() const;
	int32 GetFullCardNameFontSize() const;
	bool IsOverallVisible() const;
	bool IsPlayerFacingSerialVisible() const;
	bool IsDeveloperReferenceVisible() const;
	bool IsOwnerVisible() const;
	bool IsTeamVisible() const;
	bool IsRoleIconVisible() const;
	FLinearColor GetFullCardBaseSurfaceColor() const;
	const TArray<FLinearColor>& GetRenderedAttributeTierColors() const;
	static FLinearColor GetAttributeTierColor(int32 Value);
	bool IsGoalkeeperVisualVariant() const;
	FName GetResolvedArtIdentity() const;
	UTexture2D* GetResolvedCardFrameTexture() const;
	UTexture2D* GetResolvedPortraitTexture() const;
	UTexture2D* GetResolvedHandMicroPortraitTexture() const;
	UTexture2D* GetResolvedRoleIconTexture() const;
	UTexture2D* GetResolvedLongShotSkillIconTexture() const;
	FVector2D GetConfiguredDimensions() const;
	static FBox2f CalculatePitchMiniHeroCrop(FIntPoint SourceSize);
	bool CanExposeFullCardDetail() const;
	bool IsDragSourcePresentationActive() const;
	EFMCodexUMGCardInteractionState GetInteractionState() const;

	void ConfigureDeploymentDrag(FName CardId, bool bGoalkeeper);
	void ClearDeploymentDrag();
	bool IsDeploymentDragEnabled() const;
	FName GetDeploymentDragCardId() const;
	bool IsDeploymentDragGoalkeeper() const;
	UFMCodexDeploymentDragDropOperation* BeginDeploymentDrag();
	void ConfigureOnPitchSelection(FName OptionId, bool bSelectable);
	void ClearOnPitchSelection();
	bool IsSelectableForCurrentPrompt() const;
	FName GetOnPitchSelectionOptionId() const;
	bool RequestOnPitchSelection();
	bool RequestFullCardDetailHover();

	FFMCodexDeploymentCardDragStarted OnDeploymentDragStarted;
	FFMCodexDeploymentCardDragFinished OnDeploymentDragFinished;
	FFMCodexPlayerCardDetailHover OnDetailHoverRequested;
	FFMCodexPlayerCardDetailHover OnDetailHoverDismissed;
	FFMCodexPlayerCardOnPitchSelectionRequested OnOnPitchSelectionRequested;

protected:
	virtual void NativeOnInitialized() override;
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual FReply NativeOnMouseButtonDown(
		const FGeometry& InGeometry,
		const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(
		const FGeometry& InGeometry,
		const FPointerEvent& InMouseEvent,
		UDragDropOperation*& OutOperation) override;
	virtual void NativeOnMouseEnter(
		const FGeometry& InGeometry,
		const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(
		const FPointerEvent& InMouseEvent) override;

private:
	void BuildWidgetTree();
	void RefreshVisuals();
	void RefreshPresentationArt();
	void RefreshBiography();
	void RefreshSkills();
	void RefreshAttributes();
	void RefreshStatusBadges();

	UFUNCTION()
	void HandleDeploymentDragFinished(UDragDropOperation* Operation);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Local Match|Presentation",
		meta = (AllowPrivateAccess = "true"))
	FFMCodexUMGCardViewModel Presentation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Local Match|Presentation",
		meta = (AllowPrivateAccess = "true"))
	EFMCodexPlayerCardPresentationMode PresentationMode =
		EFMCodexPlayerCardPresentationMode::InteractionChoice;

	UPROPERTY(Transient)
	TObjectPtr<USizeBox> CardBounds;

	UPROPERTY(Transient)
	TObjectPtr<UBorder> DetailedContentLayer;

	UPROPERTY(Transient)
	TObjectPtr<UBorder> FullCardBaseSurface;

	UPROPERTY(Transient)
	TObjectPtr<UBorder> FullCardInnerFrame;

	UPROPERTY(Transient)
	TObjectPtr<UBorder> FullCardRarityRail;

	UPROPERTY(Transient)
	TObjectPtr<UBorder> FullCardIdentityAccent;

	UPROPERTY(Transient)
	TObjectPtr<UOverlay> HandMicroVisualSystem;

	UPROPERTY(Transient)
	TObjectPtr<UHorizontalBox> HandMicroContent;

	UPROPERTY(Transient)
	TObjectPtr<USizeBox> HandMicroIdentityBounds;

	UPROPERTY(Transient)
	TObjectPtr<UVerticalBox> HandMicroTextHierarchy;

	UPROPERTY(Transient)
	TObjectPtr<UImage> HandMicroPortraitImage;

	UPROPERTY(Transient)
	TObjectPtr<UBorder> HandMicroPortraitFallback;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> HandMicroIdentityText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> HandMicroRoleText;

	UPROPERTY(Transient)
	TObjectPtr<UBorder> HandMicroRarityAccent;

	UPROPERTY(Transient)
	TObjectPtr<UVerticalBox> PitchMiniContent;

	UPROPERTY(Transient)
	TObjectPtr<UImage> PitchMiniPortraitImage;

	UPROPERTY(Transient)
	TObjectPtr<UBorder> PitchMiniPortraitFallback;

	UPROPERTY(Transient)
	TObjectPtr<UBorder> PitchMiniPortraitFallbackAtmosphere;

	UPROPERTY(Transient)
	TObjectPtr<UBorder> PitchMiniPortraitTonalWash;

	UPROPERTY(Transient)
	TObjectPtr<UBorder> PitchMiniOwnershipRailLeft;

	UPROPERTY(Transient)
	TObjectPtr<UBorder> PitchMiniOwnershipRailRight;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UBorder>> PitchMiniTacticalMatchGlowSegments;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UBorder>> PitchMiniTacticalMatchStrokeSegments;

	UPROPERTY(Transient)
	TObjectPtr<UBorder> PitchMiniTacticalMatchPipTop;

	UPROPERTY(Transient)
	TObjectPtr<UBorder> PitchMiniTacticalMatchPipBottom;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> PitchMiniIdentityText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> PitchMiniRoleText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> PitchMiniIdentitySeparatorText;

	FName OnPitchSelectionOptionId = NAME_None;
	bool bSelectableForCurrentPrompt = false;

	UPROPERTY(Transient, BlueprintReadOnly,
		Category = "Local Match|Visual Hooks",
		meta = (AllowPrivateAccess = "true", BindWidgetOptional))
	TObjectPtr<UBorder> CardFrame;

	UPROPERTY(Transient)
	TObjectPtr<UImage> CardFrameImage;

	UPROPERTY(Transient)
	TObjectPtr<UBorder> CardFrameFallbackSurface;

	UPROPERTY(Transient, BlueprintReadOnly,
		Category = "Local Match|Visual Hooks",
		meta = (AllowPrivateAccess = "true", BindWidgetOptional))
	TObjectPtr<UBorder> PortraitPresentationRegion;

	UPROPERTY(Transient, BlueprintReadOnly,
		Category = "Local Match|Visual Hooks",
		meta = (AllowPrivateAccess = "true", BindWidgetOptional))
	TObjectPtr<UBorder> RoleIconHook;

	UPROPERTY(Transient)
	TObjectPtr<USizeBox> PortraitBounds;

	UPROPERTY(Transient)
	TObjectPtr<UBorder> HeaderRegion;

	UPROPERTY(Transient)
	TObjectPtr<UBorder> IdentityRegion;

	UPROPERTY(Transient)
	TObjectPtr<UBorder> SkillRegion;

	UPROPERTY(Transient)
	TObjectPtr<UBorder> AttributeRegion;

	UPROPERTY(Transient)
	TObjectPtr<UBorder> StatusRegion;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> IdentityText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> EnglishIdentityText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> FullCardIdentitySupplementText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> RoleText;

	UPROPERTY(Transient)
	TObjectPtr<UImage> RoleIconImage;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> RarityText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> OverallNumberText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> OverallLabelText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> PlayerFacingSerialText;

	UPROPERTY(Transient)
	TObjectPtr<UBorder> BiographyRegion;

	UPROPERTY(Transient)
	TObjectPtr<UVerticalBox> BiographyList;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> PortraitPlaceholderText;

	UPROPERTY(Transient)
	TObjectPtr<UImage> PortraitImage;

	UPROPERTY(Transient)
	TObjectPtr<UTexture2D> ResolvedCardFrameTexture;

	UPROPERTY(Transient)
	TObjectPtr<UTexture2D> ResolvedPortraitTexture;

	UPROPERTY(Transient)
	TObjectPtr<UTexture2D> ResolvedHandMicroPortraitTexture;

	UPROPERTY(Transient)
	TObjectPtr<UTexture2D> ResolvedRoleIconTexture;

	UPROPERTY(Transient)
	TObjectPtr<UTexture2D> ResolvedLongShotSkillIconTexture;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> OwnerText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> TeamText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> DeveloperReferenceText;

	UPROPERTY(Transient)
	TObjectPtr<UVerticalBox> SkillList;

	UPROPERTY(Transient)
	TObjectPtr<UUniformGridPanel> AttributeGrid;

	UPROPERTY(Transient)
	TObjectPtr<UWrapBox> StatusBadgeBox;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UTextBlock>> RenderedSkillTexts;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UTextBlock>> RenderedAttributeTexts;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UTextBlock>> RenderedAttributeValueTexts;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UTextBlock>> RenderedStatusTexts;

	FString RenderedAttributeSummary;
	TArray<FLinearColor> RenderedAttributeTierColors;
	int32 RenderedBiographyRowCount = 0;
	FName ResolvedArtIdentity = NAME_None;
	FName DeploymentDragCardId = NAME_None;
	bool bDeploymentDragEnabled = false;
	bool bDeploymentDragGoalkeeper = false;
	bool bDragSourcePresentationActive = false;
};
