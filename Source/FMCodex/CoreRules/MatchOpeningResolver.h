#pragma once

#include "CoreMinimal.h"

#include "AttackCountResolver.h"
#include "InitialTurnOrderResolver.h"
#include "MatchInitializer.h"

#include "MatchOpeningResolver.generated.h"

UENUM(BlueprintType)
enum class EMatchOpeningResolveErrorCode : uint8
{
	None UMETA(DisplayName = "None"),
	MatchInitializationFailed UMETA(DisplayName = "Match Initialization Failed"),
	AttackCountResolutionFailed UMETA(DisplayName = "Attack Count Resolution Failed"),
	InitialTurnOrderResolutionFailed UMETA(DisplayName = "Initial Turn Order Resolution Failed")
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchOpeningResolveInput
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Rules|Match Opening")
	TArray<FPlayerCardData> PlayerADeck;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Rules|Match Opening")
	TArray<FPlayerCardData> PlayerBDeck;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Rules|Match Opening")
	int32 PlayerAAttackCountD6Roll = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Rules|Match Opening")
	int32 PlayerBAttackCountD6Roll = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Rules|Match Opening")
	int32 PlayerATieBreakerRoll = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Rules|Match Opening")
	int32 PlayerBTieBreakerRoll = 0;
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchOpeningResolveResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Opening")
	bool bSuccess = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Opening")
	FMatchInitializationResult MatchInitializationResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Opening")
	FAttackCountResolveResult AttackCountResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Opening")
	FInitialTurnOrderResult InitialTurnOrderResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Opening")
	TArray<EMatchOpeningResolveErrorCode> ErrorCodes;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Opening")
	TArray<FString> ErrorMessages;
};

class FMCODEX_API FMatchOpeningResolver final
{
public:
	static FMatchOpeningResolveResult ResolveMatchOpening(
		const FMatchOpeningResolveInput& Input);
};
