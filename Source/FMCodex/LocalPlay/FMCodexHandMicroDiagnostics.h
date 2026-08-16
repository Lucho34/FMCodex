#pragma once

#include "CoreMinimal.h"

namespace FMCodexHandMicroDiagnostics
{
	constexpr float CardWidth = 220.0f;
	constexpr float CardHeight = 68.0f;
	constexpr float PortraitWidth = 96.0f;
	constexpr float PortraitImageHeight = 64.0f;
	constexpr float IdentityWidth = 120.0f;
	constexpr float RarityWidth = 4.0f;
	constexpr float NamePaddingLeft = 4.0f;
	constexpr float NamePaddingRight = 4.0f;
	constexpr float NameSafeWidth = 112.0f;

	constexpr float RackWidth = 476.0f;
	constexpr float PitchWidth = 968.0f;
	constexpr float GoldenLayoutWidth = 1920.0f;
	constexpr float RackFrameHorizontalCost = 12.0f;
	constexpr float GridCellHorizontalCost = 12.0f;
	constexpr float MinimumRackWidth =
		2.0f * (CardWidth + GridCellHorizontalCost)
		+ RackFrameHorizontalCost;
	constexpr int32 StandardNameFontSize = 16;
	constexpr int32 MinimumNameFontSize = 12;

	FMCODEX_API bool IsProductionReviewEnabled();
	FMCODEX_API int32 GetProductionReviewPage();
}
