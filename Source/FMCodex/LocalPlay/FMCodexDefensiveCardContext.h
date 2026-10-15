#pragma once
#include "CoreMinimal.h"

struct FFMCodexLocalMatchInteractionView;
struct FFMCodexUMGMatchScreenViewModel;

/** Read-only potential counters, derived only after the existing viewer-safe projection. */
class FMCODEX_API FFMCodexDefensiveCardContext final
{
public:
    static void Apply(const FFMCodexLocalMatchInteractionView& PublicView, FFMCodexUMGMatchScreenViewModel& Model);
    static FName Attribute(FName Trait);
    static FText CompactLabels(const TArray<FName>& Traits, TFunctionRef<bool(const FText&)> Fits,
        bool bAllowPartialPrefix = true);
};
