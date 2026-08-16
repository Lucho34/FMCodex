#pragma once

#include "CoreMinimal.h"

namespace FMCodexHandMicroDiagnostics
{
	constexpr float ProductionCardWidth = 188.0f;
	constexpr float CandidateCardWidth = 220.0f;
	constexpr float BaselineCardHeight = 64.0f;
	constexpr float CandidateCardHeight = 68.0f;
	constexpr float PortraitWidth = 96.0f;
	constexpr float PortraitImageHeight = 64.0f;
	constexpr float ProductionIdentityWidth = 88.0f;
	constexpr float CandidateIdentityWidth = 120.0f;
	constexpr float RarityWidth = 4.0f;
	constexpr float ProductionNamePaddingLeft = 10.0f;
	constexpr float ProductionNamePaddingRight = 6.0f;
	constexpr float CandidateNamePaddingLeft = 4.0f;
	constexpr float CandidateNamePaddingRight = 4.0f;
	constexpr float ProductionNameSafeWidth = 72.0f;
	constexpr float CandidateNameSafeWidth = 112.0f;

	constexpr float ProductionRackWidth = 422.0f;
	constexpr float CandidateRackWidth = 476.0f;
	constexpr float ProductionPitchWidth = 1076.0f;
	constexpr float CandidatePitchWidth = 968.0f;
	constexpr float GoldenLayoutWidth = 1920.0f;
	constexpr float RackFrameHorizontalCost = 12.0f;
	constexpr float GridCellHorizontalCost = 12.0f;
	constexpr float CandidateMinimumRackWidth =
		2.0f * (CandidateCardWidth + GridCellHorizontalCost)
		+ RackFrameHorizontalCost;
	constexpr int32 ExistingMaximumNameFontSize = 22;
	constexpr int32 StandardNameSizeCandidate = 16;
	constexpr int32 MinimumNameFontSize = 12;

	FMCODEX_API bool IsFullNameCandidateEnabled();
	FMCODEX_API bool IsUnifiedNameSizeCandidateEnabled();
	FMCODEX_API bool IsHeight68CandidateEnabled();
	FMCODEX_API float GetCardHeight();
	FMCODEX_API bool IsSharpnessComparisonEnabled();
	FMCODEX_API int32 GetSharpnessComparisonPage();
	FMCODEX_API bool IsArtConformanceOverrideEnabled();
	FMCODEX_API int32 GetArtConformanceOverrideMode();
	FMCODEX_API FString GetArtConformanceCandidateTexturePath(FName CardId);
}
