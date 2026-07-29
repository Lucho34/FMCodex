#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCurrentAttackHelperSelectionLegality.h"

#include "MatchPlayCurrentAttackHelperSelectionAvailability.generated.h"

USTRUCT(BlueprintType)
struct FMCODEX_API
	FMatchPlayCurrentAttackHelperSelectionCandidateAvailability
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Helper Selection Availability")
	FName HelperCardId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Helper Selection Availability")
	FMatchPlayCurrentAttackHelperSelectionLegalityResult LegalityResult;
};

USTRUCT(BlueprintType)
struct FMCODEX_API
	FMatchPlayCurrentAttackHelperSelectionAvailabilityResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Helper Selection Availability")
	bool bQuerySucceeded = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Helper Selection Availability")
	bool bCanSelectAnyHelper = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Helper Selection Availability")
	int64 AttackSequence = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Helper Selection Availability")
	EInitialTurnOrderPlayer RequestingSide =
		EInitialTurnOrderPlayer::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Helper Selection Availability")
	EMatchPlayCurrentAttackHelperSelectionErrorCode ErrorCode =
		EMatchPlayCurrentAttackHelperSelectionErrorCode::None;

	FMatchPlayCurrentAttackHelperSelectionGlobalContextResult
		GlobalContextResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Helper Selection Availability")
	TArray<FMatchPlayCurrentAttackHelperSelectionCandidateAvailability>
		Candidates;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Helper Selection Availability")
	FString ErrorMessage;
};

class FMCODEX_API
	FMatchPlayCurrentAttackHelperSelectionAvailability final
{
public:
	static FMatchPlayCurrentAttackHelperSelectionAvailabilityResult Query(
		const FMatchPlayState& BeforeState,
		int64 AttackSequence,
		EInitialTurnOrderPlayer RequestingSide);
};
