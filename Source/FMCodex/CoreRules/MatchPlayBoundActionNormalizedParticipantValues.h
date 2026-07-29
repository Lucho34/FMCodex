#pragma once

#include "CoreMinimal.h"

#include "MatchPlayBoundActionNormalizedParticipantValues.generated.h"

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayBoundActionNormalizedParticipantValues
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Bound Action Participant Normalization")
	int32 Shooting = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Bound Action Participant Normalization")
	int32 Dribbling = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Bound Action Participant Normalization")
	int32 Passing = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Bound Action Participant Normalization")
	int32 OffBall = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Bound Action Participant Normalization")
	int32 Marking = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Bound Action Participant Normalization")
	int32 Tackling = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Bound Action Participant Normalization")
	int32 Speed = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Bound Action Participant Normalization")
	int32 Strength = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Bound Action Participant Normalization")
	int32 Stamina = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Bound Action Participant Normalization")
	int32 LongShot = 0;
};
