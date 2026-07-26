#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCurrentAttackMarkerSelectionAvailability.h"

#include "MatchPlayMarkerNoSelectionGoalCapability.generated.h"

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

class FMatchPlayCurrentAttackCompletion;
class FMatchPlayMarkerDecline;
class FMatchPlayResolveNoLegalMarker;

class FMCODEX_API FMatchPlayMarkerNoSelectionGoalCapability final
{
public:
	FMatchPlayMarkerNoSelectionGoalCapability() = delete;

	int64 GetAttackSequence() const
	{
		return AttackSequence;
	}

	EMatchPlayMarkerNoSelectionGoalReason GetReason() const
	{
		return Reason;
	}

	EMatchPlayMarkerNoSelectionGoalSource GetSource() const
	{
		return Source;
	}

	const FMatchPlayCurrentAttackMarkerSelectionAvailabilityResult&
	GetAuthorityResult() const
	{
		return AuthorityResult;
	}

private:
	friend class FMatchPlayCurrentAttackCompletion;
	friend class FMatchPlayMarkerDecline;
	friend class FMatchPlayResolveNoLegalMarker;

	FMatchPlayMarkerNoSelectionGoalCapability(
		const int64 InAttackSequence,
		const EMatchPlayMarkerNoSelectionGoalReason InReason,
		const EMatchPlayMarkerNoSelectionGoalSource InSource,
		const FMatchPlayCurrentAttackMarkerSelectionAvailabilityResult&
			InAuthorityResult)
		: AttackSequence(InAttackSequence)
		, Reason(InReason)
		, Source(InSource)
		, AuthorityResult(InAuthorityResult)
	{
	}

	FMatchPlayMarkerNoSelectionGoalCapability(
		const FMatchPlayMarkerNoSelectionGoalCapability&) = delete;
	FMatchPlayMarkerNoSelectionGoalCapability(
		FMatchPlayMarkerNoSelectionGoalCapability&&) = delete;
	FMatchPlayMarkerNoSelectionGoalCapability& operator=(
		const FMatchPlayMarkerNoSelectionGoalCapability&) = delete;
	FMatchPlayMarkerNoSelectionGoalCapability& operator=(
		FMatchPlayMarkerNoSelectionGoalCapability&&) = delete;

	int64 AttackSequence = 0;
	EMatchPlayMarkerNoSelectionGoalReason Reason =
		EMatchPlayMarkerNoSelectionGoalReason::None;
	EMatchPlayMarkerNoSelectionGoalSource Source =
		EMatchPlayMarkerNoSelectionGoalSource::None;
	FMatchPlayCurrentAttackMarkerSelectionAvailabilityResult AuthorityResult;
};
