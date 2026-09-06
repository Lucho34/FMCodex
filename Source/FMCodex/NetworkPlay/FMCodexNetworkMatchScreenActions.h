#pragma once
#include "FMCodexNetworkPlayerIntent.h"
#include "../LocalPlay/FMCodexMatchScreenBackend.h"

/** Bounded mapping of shared UI gestures to the existing asynchronous intent client. */
class FMCODEX_API FFMCodexNetworkMatchScreenActions final
{
public:
	static bool Begin(const FFMCodexMatchScreenRequest& Request,
		const FFMCodexNetworkClientViewSnapshot& View,
		FFMCodexNetworkIntentClientState& Client,
		FFMCodexNetworkPlayerIntentEnvelope& Out);
};
