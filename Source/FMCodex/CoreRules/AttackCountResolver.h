#pragma once

#include "CoreMinimal.h"

#include "AttackCountResolver.generated.h"

UENUM(BlueprintType)
enum class EAttackCountResolveErrorCode : uint8
{
	None UMETA(DisplayName = "None"),
	PlayerAD6RollOutOfRange UMETA(DisplayName = "Player A D6 Roll Out Of Range"),
	PlayerBD6RollOutOfRange UMETA(DisplayName = "Player B D6 Roll Out Of Range")
};

USTRUCT(BlueprintType)
struct FMCODEX_API FAttackCountResolveResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Attack Count")
	bool bSuccess = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Attack Count")
	int32 PlayerAAttackCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Attack Count")
	int32 PlayerBAttackCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Attack Count")
	int32 PlayerABaseAttackCount = 3;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Attack Count")
	int32 PlayerBBaseAttackCount = 3;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Attack Count")
	int32 PlayerARarityBonusAttackCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Attack Count")
	int32 PlayerBRarityBonusAttackCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Attack Count")
	int32 PlayerARandomBonusAttackCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Attack Count")
	int32 PlayerBRandomBonusAttackCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Attack Count")
	TArray<EAttackCountResolveErrorCode> ErrorCodes;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Attack Count")
	TArray<FString> ErrorMessages;
};

class FMCODEX_API FAttackCountResolver final
{
public:
	static FAttackCountResolveResult ResolveAttackCounts(
		int32 PlayerAInitialDeckRarityScore,
		int32 PlayerBInitialDeckRarityScore,
		int32 PlayerAD6Roll,
		int32 PlayerBD6Roll);
};
