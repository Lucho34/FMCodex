#pragma once

#include "CoreMinimal.h"
#include "FMCodexFullTimePresentation.generated.h"

/** Display-ready match summary; no minute/clock is invented by presentation. */
USTRUCT()
struct FMCODEX_API FFMCodexFullTimeTeamPresentation
{
	GENERATED_BODY()
	// Participant identity is separate from the roster-backed secondary team name.
	// Supplied by the same presentation identity mapping as the match header.
	UPROPERTY() FText PlayerDisplayName;
	UPROPERTY() FText Name;
	UPROPERTY() FText Score;
	UPROPERTY() FText BadgeMark;
	UPROPERTY() FLinearColor Color = FLinearColor(0.24f, 0.38f, 0.50f);
	UPROPERTY() TArray<FText> Goals;
};

USTRUCT()
struct FMCODEX_API FFMCodexFullTimePresentation
{
	GENERATED_BODY()
	UPROPERTY() bool bVisible = false;
	UPROPERTY() FFMCodexFullTimeTeamPresentation PlayerA;
	UPROPERTY() FFMCodexFullTimeTeamPresentation PlayerB;
};
