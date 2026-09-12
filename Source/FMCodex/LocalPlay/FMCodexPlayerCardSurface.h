#pragma once

#include "CoreMinimal.h"
#include "Components/Border.h"
#include "FMCodexPlayerCardSurface.generated.h"

/** Shared scalable card material. No player-specific material instances or baked text. */
UCLASS()
class FMCODEX_API UFMCodexPlayerCardSurface : public UBorder
{
	GENERATED_BODY()
public:
	bool bPremium = false;
protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
};
