#pragma once

#include "CoreMinimal.h"
#include "CoreRuleEnums.h"

#include "MatchStateTypes.generated.h"

USTRUCT(BlueprintType)
struct FMCODEX_API FPlayerMatchState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Rules|Player Match State")
	FName PlayerId = NAME_None;

	// Display-only until the home/away rule is confirmed.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Rules|Player Match State")
	FName TeamSide = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Rules|Player Match State")
	TArray<FName> HandCardIds;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Rules|Player Match State")
	TArray<FName> ConsumedZoneCardIds;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Rules|Player Match State")
	TArray<FName> DiscardPileCardIds;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Rules|Player Match State")
	bool bUsedGoalkeeperActivation = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Rules|Player Match State")
	int32 RemainingAttackCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Rules|Player Match State")
	bool bHasFinishedDeployment = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Rules|Player Match State")
	FName SelectedBallCarrierCardId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Rules|Player Match State")
	FName SelectedMarkerCardId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Rules|Player Match State")
	FName SelectedRunnerCardId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Rules|Player Match State")
	FName SelectedSupportDefenderCardId = NAME_None;
};

USTRUCT(BlueprintType)
struct FMCODEX_API FBoardState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Rules|Board State")
	TArray<FName> SharedSlotIds;

	// Slot ID to logical zone ID, such as Forward, Midfield, or Back.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Rules|Board State")
	TMap<FName, FName> SlotZoneTypes;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Rules|Board State")
	TMap<FName, FName> SlotOccupantCardIds;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Rules|Board State")
	TMap<FName, FName> SlotOwnerPlayerIds;

	// Presentation mapping only. It must not affect rule resolution.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Rules|Board State")
	FName ViewMappingId = NAME_None;
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchLogEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Log")
	FGuid LogId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Log")
	int32 TurnIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Log")
	FName EventType = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Log")
	FName ActingPlayerId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Log")
	TArray<FName> InvolvedCardIds;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Log")
	TArray<int32> DiceResults;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Log")
	TArray<FName> DiceOrder;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Log")
	EFormulaType FormulaType = EFormulaType::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Log")
	FString FormulaInputs;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Log")
	FString FormulaResult;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Log")
	TMap<FName, int32> ScoreAfterEvent;

	// Card ID to zone ID for cards moved by this event.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Log")
	TMap<FName, FName> ZonesAfterEvent;
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Rules|Match State")
	FGuid MatchId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Rules|Match State")
	EMatchPhase CurrentPhase = EMatchPhase::NotStarted;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Rules|Match State")
	int32 CurrentActionPoint = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Rules|Match State")
	FName CurrentAttackingPlayerId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Rules|Match State")
	FName CurrentDefendingPlayerId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Rules|Match State")
	TMap<FName, int32> RemainingAttackCounts;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Rules|Match State")
	TArray<FName> AttackOrderQueue;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Rules|Match State")
	TMap<FName, int32> InitialDeckRarityScores;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Rules|Match State")
	TMap<FName, int32> Score;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Rules|Match State")
	FBoardState BoardState;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Rules|Match State")
	TArray<FPlayerMatchState> PlayerStates;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Rules|Match State")
	int32 RandomSeed = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Rules|Match State")
	TArray<FMatchLogEntry> MatchLog;
};

