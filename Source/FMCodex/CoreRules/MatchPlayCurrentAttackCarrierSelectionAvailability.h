#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCurrentAttackCarrierSelectionLegality.h"

#include "MatchPlayCurrentAttackCarrierSelectionAvailability.generated.h"

USTRUCT(BlueprintType)
struct FMCODEX_API
	FMatchPlayCurrentAttackCarrierSelectionCandidateAvailability
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Carrier Selection Availability")
	FName CarrierCardId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Carrier Selection Availability")
	FMatchPlayCurrentAttackCarrierSelectionLegalityResult LegalityResult;
};

USTRUCT(BlueprintType)
struct FMCODEX_API
	FMatchPlayCurrentAttackCarrierSelectionAvailabilityResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Carrier Selection Availability")
	bool bQuerySucceeded = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Carrier Selection Availability")
	bool bCanSelectAnyCarrier = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Carrier Selection Availability")
	int64 AttackSequence = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Carrier Selection Availability")
	EInitialTurnOrderPlayer RequestingSide =
		EInitialTurnOrderPlayer::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Carrier Selection Availability")
	TArray<FMatchPlayCurrentAttackCarrierSelectionCandidateAvailability>
		Candidates;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Carrier Selection Availability")
	bool bHasGlobalBlockingLegalityResult = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Carrier Selection Availability")
	FMatchPlayCurrentAttackCarrierSelectionLegalityResult
		GlobalBlockingLegalityResult;
};

class FMCODEX_API
	FMatchPlayCurrentAttackCarrierSelectionAvailability final
{
public:
	static FMatchPlayCurrentAttackCarrierSelectionAvailabilityResult Query(
		const FMatchPlayState& BeforeState,
		int64 AttackSequence,
		EInitialTurnOrderPlayer RequestingSide);
};
