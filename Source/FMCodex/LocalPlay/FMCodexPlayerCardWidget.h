#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "FMCodexLocalMatchUMGPresentation.h"

#include "FMCodexPlayerCardWidget.generated.h"

class UBorder;
class UHorizontalBox;
class UImage;
class USizeBox;
class UTextBlock;
class UTexture2D;
class UUniformGridPanel;
class UVerticalBox;
class UWrapBox;
class SWidget;

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

protected:
	virtual void NativeOnInitialized() override;
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	void BuildWidgetTree();
	void RefreshVisuals();
	void RefreshPresentationArt();
	void RefreshSkills();
	void RefreshAttributes();
	void RefreshStatusBadges();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Local Match|Presentation",
		meta = (AllowPrivateAccess = "true"))
	FFMCodexUMGCardViewModel Presentation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Local Match|Presentation",
		meta = (AllowPrivateAccess = "true"))
	EFMCodexPlayerCardPresentationMode PresentationMode =
		EFMCodexPlayerCardPresentationMode::InteractionChoice;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,
		Category = "Local Match|Visual Hooks",
		meta = (AllowPrivateAccess = "true"))
	FText PortraitPlaceholderLabel =
		FText::FromString(TEXT("PORTRAIT\nASSET READY"));

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
};
