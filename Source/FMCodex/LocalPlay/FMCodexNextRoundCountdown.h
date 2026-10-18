#pragma once

#include "CoreMinimal.h"

/** Cosmetic clock only. Eligibility comes from the existing displayed action. */
struct FFMCodexNextRoundCountdown
{
	int64 Sequence = 0;
	FString Actor;
	FString Phase;
	float ReadingElapsed = 0.f;
	float CountdownElapsed = 0.f;
	bool bCounting = false;

	void Cancel() { *this = {}; }
	bool Tick(bool bReady, int64 InSequence, const FString& InActor, const FString& InPhase, float Delta)
	{
		if (!bReady) { Cancel(); return false; }
		if (Sequence != InSequence || Actor != InActor || Phase != InPhase)
		{
			Cancel(); Sequence = InSequence; Actor = InActor; Phase = InPhase;
			return false; // Never charge the frame that first made the result readable.
		}
		Delta = FMath::Max(0.f, Delta);
		if (!bCounting)
		{
			ReadingElapsed += Delta;
			if (ReadingElapsed >= 2.5f) bCounting = true;
			return false; // Render (5) before consuming any countdown time.
		}
		CountdownElapsed += Delta;
		return CountdownElapsed >= 5.f;
	}
	int32 Remaining() const { return bCounting ? FMath::Clamp(FMath::CeilToInt(5.f - CountdownElapsed), 1, 5) : 0; }
};

/** Lives with the action adapter, so replacing the widget cannot repeat a request.
 * A rejected automatic attempt requires an explicit manual retry. */
struct FFMCodexNextRoundDispatch
{
	int64 Sequence = 0;
	bool bSubmitted = false;
	bool bAutoAttempted = false;
	void Observe(int64 InSequence)
	{
		if (InSequence > Sequence) { *this = {}; Sequence = InSequence; }
	}
};
