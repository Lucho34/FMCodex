#pragma once

#include "CoreMinimal.h"

#include "FMCodexLocalMatchInteractionView.h"

struct FMCODEX_API FFMCodexMatchClientViewRequest
{
	EInitialTurnOrderPlayer ViewerSide = EInitialTurnOrderPlayer::None;
	FFMCodexLocalMatchViewerDisclosure Disclosure;
};

struct FMCODEX_API FFMCodexMatchClientViewResult
{
	bool bSuccess = false;
	FFMCodexLocalMatchInteractionView View;
	FString ErrorMessage;
};

/** Read-only client seam. Raw authoritative state never crosses this port. */
class FMCODEX_API IFMCodexMatchClientViewPort
{
public:
	virtual ~IFMCodexMatchClientViewPort() = default;

	virtual FFMCodexMatchClientViewResult GetViewForViewer(
		const FFMCodexMatchClientViewRequest& Request) const = 0;
};
