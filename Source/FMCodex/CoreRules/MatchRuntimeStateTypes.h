#pragma once

#include "CoreMinimal.h"

#include "MatchOpeningResolver.h"

#include "MatchRuntimeStateTypes.generated.h"

USTRUCT(BlueprintType)
struct FMCODEX_API FPlayerRuntimeState
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Runtime")
	int32 TotalAttackCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Runtime")
	int32 UsedAttackCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Runtime")
	int32 Score = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Runtime")
	FString GoalkeeperCardId;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Runtime")
	int32 InitialDeckRarityScore = 0;
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchRuntimeState
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Runtime")
	bool bIsInitialized = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Runtime")
	FPlayerRuntimeState PlayerAState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Runtime")
	FPlayerRuntimeState PlayerBState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Runtime")
	EInitialTurnOrderPlayer FirstPlayer = EInitialTurnOrderPlayer::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Runtime")
	EInitialTurnOrderPlayer SecondPlayer = EInitialTurnOrderPlayer::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Runtime")
	EInitialTurnOrderPlayer CurrentAttackingPlayer = EInitialTurnOrderPlayer::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Runtime")
	FMatchOpeningResolveResult OpeningResultSnapshot;
};
