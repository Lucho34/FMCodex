#pragma once

#include "CoreMinimal.h"
#include "PlayerStaminaTier.h"

#include "MatchPlayBoundActionNormalizedParticipantValues.generated.h"

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayBoundActionNormalizedParticipantValues
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Bound Action Participant Normalization")
	int32 Shooting = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Bound Action Participant Normalization")
	int32 Control = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Bound Action Participant Normalization")
	int32 Passing = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Bound Action Participant Normalization")
	int32 Defense = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Bound Action Participant Normalization")
	int32 Speed = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Bound Action Participant Normalization")
	int32 Strength = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Bound Action Participant Normalization")
	EPlayerStaminaTier StaminaTier = EPlayerStaminaTier::None;
};
