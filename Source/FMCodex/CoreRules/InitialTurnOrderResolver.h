#pragma once

#include "CoreMinimal.h"

#include "InitialTurnOrderResolver.generated.h"

UENUM(BlueprintType)
enum class EInitialTurnOrderPlayer : uint8
{
	None UMETA(DisplayName = "None"),
	PlayerA UMETA(DisplayName = "Player A"),
	PlayerB UMETA(DisplayName = "Player B")
};

UENUM(BlueprintType)
enum class EInitialTurnOrderReason : uint8
{
	None UMETA(DisplayName = "None"),
	HigherAttackCount UMETA(DisplayName = "Higher Attack Count"),
	LowerInitialDeckRarityScore UMETA(DisplayName = "Lower Initial Deck Rarity Score"),
	LowerTieBreakerRoll UMETA(DisplayName = "Lower Tie Breaker Roll"),
	TieBreakerStillTied UMETA(DisplayName = "Tie Breaker Still Tied")
};

UENUM(BlueprintType)
enum class EInitialTurnOrderErrorCode : uint8
{
	None UMETA(DisplayName = "None"),
	InvalidPlayerAAttackCount UMETA(DisplayName = "Invalid Player A Attack Count"),
	InvalidPlayerBAttackCount UMETA(DisplayName = "Invalid Player B Attack Count"),
	InvalidPlayerAInitialDeckRarityScore
		UMETA(DisplayName = "Invalid Player A Initial Deck Rarity Score"),
	InvalidPlayerBInitialDeckRarityScore
		UMETA(DisplayName = "Invalid Player B Initial Deck Rarity Score"),
	InvalidPlayerATieBreakerRoll UMETA(DisplayName = "Invalid Player A Tie Breaker Roll"),
	InvalidPlayerBTieBreakerRoll UMETA(DisplayName = "Invalid Player B Tie Breaker Roll"),
	TieBreakerStillTied UMETA(DisplayName = "Tie Breaker Still Tied")
};

USTRUCT(BlueprintType)
struct FMCODEX_API FInitialTurnOrderResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Initial Turn Order")
	bool bSuccess = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Initial Turn Order")
	bool bRequiresTieBreakerReroll = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Initial Turn Order")
	EInitialTurnOrderPlayer FirstPlayer = EInitialTurnOrderPlayer::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Initial Turn Order")
	EInitialTurnOrderReason Reason = EInitialTurnOrderReason::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Initial Turn Order")
	TArray<EInitialTurnOrderErrorCode> ErrorCodes;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Initial Turn Order")
	TArray<FString> ErrorMessages;
};

class FMCODEX_API FInitialTurnOrderResolver final
{
public:
	static FInitialTurnOrderResult ResolveInitialTurnOrder(
		int32 PlayerAAttackCount,
		int32 PlayerBAttackCount,
		int32 PlayerAInitialDeckRarityScore,
		int32 PlayerBInitialDeckRarityScore,
		int32 PlayerATieBreakerRoll,
		int32 PlayerBTieBreakerRoll);
};
