#include "FMCodexFullCardDiagnostics.h"

#include "HAL/IConsoleManager.h"

namespace
{
	TAutoConsoleVariable<int32> CVarFMCodexFullCardProductionReview(
		TEXT("FMCodex.UI.FullCardReview"),
		0,
		TEXT("Development-only In-Match Full Card production review: "
			"0=hidden, 1=Saliba/Odegaard, 2=Rice/Haaland, "
			"3=Foden/Dias, 4=Saka/Rodri, "
			"5=0/3 Skill capacity stress."),
		ECVF_Cheat);
}

int32 FMCodexFullCardDiagnostics::GetProductionReviewPage()
{
#if UE_BUILD_SHIPPING
	return 0;
#else
	return FMath::Clamp(
		CVarFMCodexFullCardProductionReview.GetValueOnGameThread() - 1, 0, 4);
#endif
}

bool FMCodexFullCardDiagnostics::IsProductionReviewEnabled()
{
#if UE_BUILD_SHIPPING
	return false;
#else
	return CVarFMCodexFullCardProductionReview.GetValueOnGameThread() != 0;
#endif
}
