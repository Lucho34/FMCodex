#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "FMCodexLocalMatchUMGPresentation.h"

#include "FMCodexPlayerCardWidget.generated.h"

class UBorder;
class UDragDropOperation;
class UHorizontalBox;
class UImage;
class USizeBox;
class UTextBlock;
class UTexture2D;
class UUniformGridPanel;
class UVerticalBox;
class UWrapBox;
class SWidget;
class UFMCodexDeploymentDragDropOperation;

DECLARE_MULTICAST_DELEGATE_TwoParams(
	FFMCodexDeploymentCardDragStarted, FName, bool);
DECLARE_MULTICAST_DELEGATE(FFMCodexDeploymentCardDragFinished);

UENUM(BlueprintType)
enum class EFMCodexPlayerCardPresentationMode : uint8
{
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
	bool IsGoalkeeperVisualVariant() const;
	FName GetResolvedArtIdentity() const;
	UTexture2D* GetResolvedCardFrameTexture() const;
	UTexture2D* GetResolvedPortraitTexture() const;
	UTexture2D* GetResolvedRoleIconTexture() const;
	UTexture2D* GetResolvedLongShotSkillIconTexture() const;

	void ConfigureDeploymentDrag(FName CardId, bool bGoalkeeper);
	void ClearDeploymentDrag();
	bool IsDeploymentDragEnabled() const;
	FName GetDeploymentDragCardId() const;
	bool IsDeploymentDragGoalkeeper() const;
	UFMCodexDeploymentDragDropOperation* BeginDeploymentDrag();

	FFMCodexDeploymentCardDragStarted OnDeploymentDragStarted;
	FFMCodexDeploymentCardDragFinished OnDeploymentDragFinished;

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

private:
	void BuildWidgetTree();
	void RefreshVisuals();
	void RefreshPresentationArt();
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
	TObjectPtr<UTextBlock> IdentityText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> RoleText;

	UPROPERTY(Transient)
	TObjectPtr<UImage> RoleIconImage;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> RarityText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> PortraitPlaceholderText;

	UPROPERTY(Transient)
	TObjectPtr<UImage> PortraitImage;

	UPROPERTY(Transient)
	TObjectPtr<UTexture2D> ResolvedCardFrameTexture;

	UPROPERTY(Transient)
	TObjectPtr<UTexture2D> ResolvedPortraitTexture;

	UPROPERTY(Transient)
	TObjectPtr<UTexture2D> ResolvedRoleIconTexture;

	UPROPERTY(Transient)
	TObjectPtr<UTexture2D> ResolvedLongShotSkillIconTexture;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> OwnerText;

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
	TArray<TObjectPtr<UTextBlock>> RenderedStatusTexts;

	FString RenderedAttributeSummary;
	FName ResolvedArtIdentity = NAME_None;
	FName DeploymentDragCardId = NAME_None;
	bool bDeploymentDragEnabled = false;
	bool bDeploymentDragGoalkeeper = false;
};
