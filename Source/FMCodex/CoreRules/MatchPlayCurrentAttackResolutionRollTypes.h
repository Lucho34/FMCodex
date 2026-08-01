#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCurrentAttackResolutionRollTypes.generated.h"

UENUM(BlueprintType)
enum class EMatchPlayCurrentAttackResolutionRollPurpose : uint8
{
	None = 0 UMETA(DisplayName = "None"),
	InitialRoute = 1 UMETA(DisplayName = "Initial Route")
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayCurrentAttackResolutionRollRecord
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Session")
	EMatchPlayCurrentAttackResolutionRollPurpose Purpose =
		EMatchPlayCurrentAttackResolutionRollPurpose::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Session")
	int32 RawD6 = 0;
};
