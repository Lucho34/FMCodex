#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCurrentAttackMarkerSelectionLegality.h"

#include "MatchPlayCurrentAttackMarkerSelectionAvailability.generated.h"

USTRUCT(BlueprintType)
struct FMCODEX_API
	FMatchPlayCurrentAttackMarkerSelectionCandidateAvailability
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Marker Selection Availability")
	FName MarkerCardId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Marker Selection Availability")
	FMatchPlayCurrentAttackMarkerSelectionLegalityResult LegalityResult;
};

USTRUCT(BlueprintType)
struct FMCODEX_API
	FMatchPlayCurrentAttackMarkerSelectionAvailabilityResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Marker Selection Availability")
	bool bQuerySucceeded = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Marker Selection Availability")
	bool bCanSelectAnyMarker = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Marker Selection Availability")
	int64 AttackSequence = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Marker Selection Availability")
	EInitialTurnOrderPlayer RequestingSide =
		EInitialTurnOrderPlayer::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Marker Selection Availability")
	TArray<FMatchPlayCurrentAttackMarkerSelectionCandidateAvailability>
		Candidates;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Marker Selection Availability")
	bool bHasGlobalBlockingLegalityResult = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Marker Selection Availability")
	FMatchPlayCurrentAttackMarkerSelectionLegalityResult
		GlobalBlockingLegalityResult;
};

class FMCODEX_API
	FMatchPlayCurrentAttackMarkerSelectionAvailability final
{
public:
	static FMatchPlayCurrentAttackMarkerSelectionAvailabilityResult Query(
		const FMatchPlayState& BeforeState,
		int64 AttackSequence,
		EInitialTurnOrderPlayer RequestingSide);
};
