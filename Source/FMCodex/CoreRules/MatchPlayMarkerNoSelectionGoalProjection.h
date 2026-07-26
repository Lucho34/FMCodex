#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCurrentAttackMarkerSelectionAvailability.h"

#include "MatchPlayMarkerNoSelectionGoalProjection.generated.h"

UENUM(BlueprintType)
enum class EMatchPlayMarkerNoSelectionGoalReason : uint8
{
	None UMETA(DisplayName = "None"),
	DefenderHasNoDeployedPlayers
		UMETA(DisplayName = "Defender Has No Deployed Players"),
	NoLegalMarker UMETA(DisplayName = "No Legal Marker"),
	MarkerDeclined UMETA(DisplayName = "Marker Declined")
};

UENUM(BlueprintType)
enum class EMatchPlayMarkerNoSelectionGoalSource : uint8
{
	None UMETA(DisplayName = "None"),
	ResolveNoLegalMarker UMETA(DisplayName = "Resolve No Legal Marker"),
	DeclineMarker UMETA(DisplayName = "Decline Marker")
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayMarkerNoSelectionGoalProjection
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Marker No-selection Goal")
	int64 AttackSequence = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Marker No-selection Goal")
	bool bFormalSuccess = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Marker No-selection Goal")
	bool bIsGoal = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Marker No-selection Goal")
	EMatchPlayMarkerNoSelectionGoalReason Reason =
		EMatchPlayMarkerNoSelectionGoalReason::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Marker No-selection Goal")
	EMatchPlayMarkerNoSelectionGoalSource Source =
		EMatchPlayMarkerNoSelectionGoalSource::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Marker No-selection Goal")
	FMatchPlayCurrentAttackMarkerSelectionAvailabilityResult
		MarkerAvailabilityResult;
};
