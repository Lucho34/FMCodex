#pragma once

#include "CoreMinimal.h"

/** Development-only switch for the bounded In-Match Full Card review page. */
namespace FMCodexFullCardDiagnostics
{
	FMCODEX_API bool IsProductionReviewEnabled();
	FMCODEX_API int32 GetProductionReviewPage();
}
