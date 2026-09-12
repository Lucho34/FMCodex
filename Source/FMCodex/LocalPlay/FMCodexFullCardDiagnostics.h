#pragma once

#include "CoreMinimal.h"

struct FFMCodexUMGCardViewModel;

/** Development-only switch for the bounded In-Match Full Card review page. */
namespace FMCodexFullCardDiagnostics
{
	FMCODEX_API bool IsProductionReviewEnabled();
	FMCODEX_API int32 GetProductionReviewPage();
    FMCODEX_API bool IsHoverSampleNumbersEnabled();
    // Applies only to an empty copied DEV presentation; never to roster or authoritative state.
    FMCODEX_API void ApplySampleNumber(FFMCodexUMGCardViewModel& ReviewModel);
}
