#include "FMCodexFullCardDiagnostics.h"
#include "FMCodexPrototypeTeamContent.h"

#include "HAL/IConsoleManager.h"
#include "FMCodexLocalMatchUMGPresentation.h"

namespace
{
#if !UE_BUILD_SHIPPING
    TAutoConsoleVariable<int32> CVarFullCardSampleNumbers(
        TEXT("FMCodex.UI.FullCardSampleNumbers"), 0,
        TEXT("DEV Full hover only: 1=resolve an empty review field from configured defaults; "
             "0=actual production assignments. Re-enter hover after changing. Never updates Hand/Pitch or roster."),
        ECVF_Cheat);
#endif
	TAutoConsoleVariable<int32> CVarFMCodexFullCardProductionReview(
		TEXT("FMCodex.UI.FullCardReview"),
		0,
		TEXT("Development-only In-Match Full Card production review: "
			"0=hidden, 1=Saliba/Odegaard, 2=Rice/Haaland, "
			"3=Foden/Dias, 4=Saka/Rodri, "
			"5=0/3 Skill capacity stress, 6=Raya/Haaland. Pages 4/6 use DEV shirt numbers."),
		ECVF_Cheat);
}

int32 FMCodexFullCardDiagnostics::GetProductionReviewPage()
{
#if UE_BUILD_SHIPPING
	return 0;
#else
	return FMath::Clamp(
		CVarFMCodexFullCardProductionReview.GetValueOnGameThread() - 1, 0, 5);
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


bool FMCodexFullCardDiagnostics::IsHoverSampleNumbersEnabled()
{
#if UE_BUILD_SHIPPING
    return false;
#else
    return CVarFullCardSampleNumbers.GetValueOnGameThread() != 0;
#endif
}

void FMCodexFullCardDiagnostics::ApplySampleNumber(FFMCodexUMGCardViewModel& ReviewModel)
{
#if !UE_BUILD_SHIPPING
    if (!ReviewModel.AssignedPlayerNumber.IsEmpty()) return;
    ReviewModel.AssignedPlayerNumber = FFMCodexPrototypeTeamContent::ResolvePlayerNumber(
        ReviewModel.CardId, ReviewModel.AssignedPlayerNumber);
#endif
}
