#include "FMCodexHandMicroDiagnostics.h"

#include "HAL/IConsoleManager.h"

namespace
{
	TAutoConsoleVariable<int32> CVarFMCodexHandMicroProductionReview(
		TEXT("FMCodex.UI.HandMicroReview"),
		0,
		TEXT("Development-only Hand Micro production review surface: 0=hidden, 1=shown."),
		ECVF_Cheat);

	TAutoConsoleVariable<int32> CVarFMCodexHandMicroProductionReviewPage(
		TEXT("FMCodex.UI.HandMicroReviewPage"),
		0,
		TEXT("Development-only production review page: 0=approved portraits, "
			"1=name stress, 2=layout and ghost edge cases."),
		ECVF_Cheat);
}

bool FMCodexHandMicroDiagnostics::IsProductionReviewEnabled()
{
#if UE_BUILD_SHIPPING
	return false;
#else
	return CVarFMCodexHandMicroProductionReview.GetValueOnGameThread() != 0;
#endif
}

int32 FMCodexHandMicroDiagnostics::GetProductionReviewPage()
{
#if UE_BUILD_SHIPPING
	return 0;
#else
	return FMath::Clamp(
		CVarFMCodexHandMicroProductionReviewPage.GetValueOnGameThread(), 0, 2);
#endif
}
