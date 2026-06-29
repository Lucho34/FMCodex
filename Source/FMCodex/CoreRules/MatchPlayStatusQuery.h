#pragma once

#include "CoreMinimal.h"

#include "MatchPlayState.h"

#include "MatchPlayStatusQuery.generated.h"

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayStatus
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Status")
	bool bIsRuntimeInitialized = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Status")
	EInitialTurnOrderPlayer CurrentAttackingPlayer =
		EInitialTurnOrderPlayer::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Status")
	int32 PlayerAScore = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Status")
	int32 PlayerBScore = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Status")
	int32 PlayerARemainingAttackCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Status")
	int32 PlayerBRemainingAttackCount = 0;

	// FMatchPlayState does not currently persist these two outcome values.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Status")
	bool bHasStoredMatchEndState = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Status")
	bool bHasStoredMatchResultType = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Status")
	int32 PlayerAAvailableCardCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Status")
	int32 PlayerAUsedCardCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Status")
	int32 PlayerBAvailableCardCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Status")
	int32 PlayerBUsedCardCount = 0;
};

class FMCODEX_API FMatchPlayStatusQuery final
{
public:
	static FMatchPlayStatus QueryStatus(
		const FMatchPlayState& MatchPlayState);
};
