#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCurrentAttackRunnerSelectionLegality.h"

#include "MatchPlayCurrentAttackRunnerSelectionAvailability.generated.h"

USTRUCT(BlueprintType)
struct FMCODEX_API
	FMatchPlayCurrentAttackRunnerSelectionCandidateAvailability
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Runner Selection Availability")
	FName RunnerCardId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Runner Selection Availability")
	FMatchPlayCurrentAttackRunnerSelectionLegalityResult
		LegalityResult;
};

USTRUCT(BlueprintType)
struct FMCODEX_API
	FMatchPlayCurrentAttackRunnerSelectionAvailabilityResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Runner Selection Availability")
	bool bQuerySucceeded = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Runner Selection Availability")
	bool bCanSelectAnyRunner = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Runner Selection Availability")
	int64 AttackSequence = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Runner Selection Availability")
	EInitialTurnOrderPlayer RequestingSide =
		EInitialTurnOrderPlayer::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Runner Selection Availability")
	EMatchPlayCurrentAttackRunnerSelectionErrorCode ErrorCode =
		EMatchPlayCurrentAttackRunnerSelectionErrorCode::None;

	FMatchPlayCurrentAttackRunnerSelectionGlobalContextResult
		GlobalContextResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Runner Selection Availability")
	TArray<
		FMatchPlayCurrentAttackRunnerSelectionCandidateAvailability>
		Candidates;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Runner Selection Availability")
	FString ErrorMessage;
};

class FMCODEX_API
	FMatchPlayCurrentAttackRunnerSelectionAvailability final
{
public:
	static FMatchPlayCurrentAttackRunnerSelectionAvailabilityResult Query(
		const FMatchPlayState& BeforeState,
		int64 AttackSequence,
		EInitialTurnOrderPlayer RequestingSide);
};
