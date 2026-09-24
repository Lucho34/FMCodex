#pragma once

#include "CoreMinimal.h"

// Stage 8.8A is an opt-in local visual experiment, not a production style token.
#if !UE_BUILD_SHIPPING
class UWidgetTree;
class USizeBox;
class UButton;
struct FFMCodexUMGInlineFormulaSurfaceViewModel;
struct FFMCodexUMGInlineFormulaRowViewModel;
enum class EFMCodexFormulaEmphasis : uint8;

namespace FMCodexFormulaBroadcastPrototype
{
	bool IsEnabledFor(const FFMCodexUMGInlineFormulaSurfaceViewModel& P, bool bEmbedded);
	bool CanOmitContributorNames(const FFMCodexUMGInlineFormulaRowViewModel& Row);
	USizeBox* Build(UWidgetTree& Tree, UButton*& OutContinue);
	void Refresh(UWidgetTree& Tree, const FFMCodexUMGInlineFormulaSurfaceViewModel& P,
		EFMCodexFormulaEmphasis Attack, EFMCodexFormulaEmphasis Defense);
}
#endif
