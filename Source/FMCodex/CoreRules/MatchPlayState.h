#pragma once

#include "CoreMinimal.h"

#include "MatchRuntimeStateTypes.h"
#include "PlayCardResolver.h"

#include "MatchPlayState.generated.h"

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayState
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play")
	FMatchRuntimeState RuntimeState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play")
	FMatchCardUsageState CardUsageState;

	static FMatchPlayState Create(
		const FMatchRuntimeState& InRuntimeState,
		const FMatchCardUsageState& InCardUsageState)
	{
		FMatchPlayState Result;
		Result.RuntimeState = InRuntimeState;
		Result.CardUsageState = InCardUsageState;
		return Result;
	}
};
