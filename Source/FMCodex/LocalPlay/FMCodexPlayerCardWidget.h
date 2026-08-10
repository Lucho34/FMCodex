#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "FMCodexLocalMatchUMGPresentation.h"

#include "FMCodexPlayerCardWidget.generated.h"

class UTextBlock;
class SWidget;

UCLASS(Blueprintable)
class FMCODEX_API UFMCodexPlayerCardWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Local Match|Presentation")
	void RefreshFromPresentation(
		const FFMCodexUMGCardViewModel& InPresentation);

	const FFMCodexUMGCardViewModel& GetPresentation() const;

protected:
	virtual void NativeOnInitialized() override;
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	void BuildWidgetTree();
	void RefreshVisuals();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Local Match|Presentation",
		meta = (AllowPrivateAccess = "true"))
	FFMCodexUMGCardViewModel Presentation;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> IdentityText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> RoleText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> SkillText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> AttributeText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> StatusText;
};
