#pragma once

#include "CoreMinimal.h"
#include "MatchRuntimeStateTypes.h"

#include "MatchEndResolver.generated.h"

UENUM(BlueprintType)
enum class EMatchEndResolveErrorCode : uint8
{
	None UMETA(DisplayName = "None"),
	RuntimeStateNotInitialized UMETA(DisplayName = "Runtime State Not Initialized"),
	InvalidPlayerAAttackCountState UMETA(DisplayName = "Invalid Player A Attack Count State"),
	InvalidPlayerBAttackCountState UMETA(DisplayName = "Invalid Player B Attack Count State")
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchEndResolveResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match End")
	bool bSuccess = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match End")
	bool bIsMatchEnded = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match End")
	EMatchEndResolveErrorCode ErrorCode = EMatchEndResolveErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match End")
	FString ErrorMessage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match End")
	int32 PlayerATotalAttackCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match End")
	int32 PlayerAUsedAttackCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match End")
	int32 PlayerARemainingAttackCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match End")
	int32 PlayerBTotalAttackCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match End")
	int32 PlayerBUsedAttackCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match End")
	int32 PlayerBRemainingAttackCount = 0;
};

class FMCODEX_API FMatchEndResolver final
{
public:
	static FMatchEndResolveResult ResolveMatchEnd(
		const FMatchRuntimeState& RuntimeState);
};
