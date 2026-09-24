#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "FMCodexLocalMatchUMGPresentation.h"
#include "FMCodexMatchFlowPanel.h"

#include "FMCodexInlineResolutionFormulaSurfaceWidget.generated.h"

class UButton;
class UBorder;
class UTextBlock;
class UVerticalBox;
class UWrapBox;
class UFMCodexRollReelWidget;
class SWidget;

enum class EFMCodexFormulaComponentRole : uint8 { Operand, Modifier, Roll, PendingRoll };

DECLARE_DYNAMIC_MULTICAST_DELEGATE(
	FFMCodexInlineFormulaContinueRequested);

/**
 * Generic renderer for an already-projected inline arithmetic contest.
 * It does not read Match State, choose a route, sum terms, or consume RNG.
 */
UCLASS(Blueprintable)
class FMCODEX_API UFMCodexInlineResolutionFormulaSurfaceWidget final
	: public UUserWidget
{
	GENERATED_BODY()

public:
	UFMCodexInlineResolutionFormulaSurfaceWidget(
		const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Local Match|Inline Formula")
	void RefreshFromPresentation(
		const FFMCodexUMGInlineFormulaSurfaceViewModel& InPresentation);

	const FFMCodexUMGInlineFormulaSurfaceViewModel& GetPresentation() const;
	int32 GetRenderedAttackTermCount() const;
	int32 GetRenderedDefenseTermCount() const;
	int32 GetRenderedPendingTermCount() const;
	UFMCodexRollReelWidget* GetRollReelWidget() const;
	EFMCodexFormulaEmphasis GetRowEmphasis(bool bAttack) const;
	static EFMCodexFormulaComponentRole GetComponentRole(
		const FFMCodexUMGInlineFormulaTermViewModel& Term, EFMCodexFormulaEmphasis Emphasis);
	bool IsBroadcastPrototypeVisible() const;

	// Geometry only: the containing contest widget already paints the outer shell.
	void SetEmbeddedFormulaLayout(bool bEmbedded);

	UFUNCTION(BlueprintCallable, Category = "Local Match|Inline Formula")
	void RequestContinue();

	UPROPERTY(BlueprintAssignable, Category = "Local Match|Inline Formula")
	FFMCodexInlineFormulaContinueRequested OnContinueRequested;

protected:
	virtual void NativeOnInitialized() override;
	virtual TSharedRef<SWidget> RebuildWidget() override;
#if !UE_BUILD_SHIPPING
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
#endif

private:
	void BuildWidgetTree();
	void RefreshVisuals();
	void RefreshRow(
		const FFMCodexUMGInlineFormulaRowViewModel& Row,
		EFMCodexFormulaEmphasis Emphasis,
		const FString& WidgetNamePrefix,
		UTextBlock* SideText,
		UWrapBox* ParticipantBody,
		UTextBlock* KnownSubtotalText,
		UWrapBox* FormulaBody,
		UTextBlock* FinalValueText,
		TArray<TObjectPtr<UWidget>>& ParticipantItems,
		TArray<TObjectPtr<UWidget>>& TermItems);

	UFUNCTION()
	void HandleContinueClicked();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Local Match|Inline Formula",
		meta = (AllowPrivateAccess = "true"))
	FFMCodexUMGInlineFormulaSurfaceViewModel Presentation;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> ContestText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> StatusText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> RollHelperText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> RouteResultText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> TacticalPlayerText;

	UPROPERTY(Transient)
	TObjectPtr<UVerticalBox> TypeInformationBody;

	UPROPERTY(Transient)
	TObjectPtr<UBorder> DiceRevealRegion;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> DiceOwnerText;

	UPROPERTY(Transient)
	TObjectPtr<UFMCodexRollReelWidget> RollReel;

	UPROPERTY(Transient)
	TObjectPtr<UBorder> AttackRegion;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> AttackSideText;

	UPROPERTY(Transient)
	TObjectPtr<UWrapBox> AttackParticipantBody;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> AttackKnownSubtotalText;

	UPROPERTY(Transient)
	TObjectPtr<UWrapBox> AttackFormulaBody;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> AttackFinalValueText;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UWidget>> AttackParticipantItems;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UWidget>> AttackTermItems;

	UPROPERTY(Transient)
	TObjectPtr<UBorder> DefenseRegion;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> DefenseSideText;

	UPROPERTY(Transient)
	TObjectPtr<UWrapBox> DefenseParticipantBody;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> DefenseKnownSubtotalText;

	UPROPERTY(Transient)
	TObjectPtr<UWrapBox> DefenseFormulaBody;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> DefenseFinalValueText;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UWidget>> DefenseParticipantItems;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UWidget>> DefenseTermItems;

	UPROPERTY(Transient)
	TObjectPtr<UButton> ContinueButton;

	int32 RenderedAttackTermCount = 0;
	int32 RenderedDefenseTermCount = 0;
	int32 RenderedPendingTermCount = 0;
	bool bEmbeddedFormulaLayout = false;
};
