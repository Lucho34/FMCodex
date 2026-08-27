#pragma once

#include "CoreMinimal.h"

#if !UE_BUILD_SHIPPING

#include "Widgets/SCompoundWidget.h"

class AFMCodexLocalMatchPlayerController;

/** Small, collapsed-by-default LocalPlay developer control on the right edge. */
class SFMCodexLocalDevRollOverrideWidget final : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SFMCodexLocalDevRollOverrideWidget) {}
		SLATE_ARGUMENT(TWeakObjectPtr<AFMCodexLocalMatchPlayerController>, Controller)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	FReply SelectPreviousTarget();
	FReply SelectNextTarget();
	FReply SelectPreviousValue();
	FReply SelectNextValue();
	FReply SetSelectedOverride();
	FReply ClearSelectedOverride();
	FReply ClearAllOverrides();

	FText SelectedTargetText() const;
	FText SelectedValueText() const;
	FText PendingOverridesText() const;
	FText LastCommandText() const;
	int32 SelectedMinimum() const;
	int32 SelectedMaximum() const;
	void ClampSelectedValue();

	TWeakObjectPtr<AFMCodexLocalMatchPlayerController> Controller;
	int32 SelectedTargetIndex = 0;
	int32 SelectedValue = 1;
	FString LastCommand = TEXT("Ready");
};

#endif
