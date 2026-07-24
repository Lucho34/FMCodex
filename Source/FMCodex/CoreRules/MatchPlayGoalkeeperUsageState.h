#pragma once

#include "CoreMinimal.h"

#include "InitialTurnOrderResolver.h"

#include "MatchPlayGoalkeeperUsageState.generated.h"

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayGoalkeeperUsageState
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Goalkeeper Usage")
	bool bPlayerAGoalkeeperCardUsed = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Goalkeeper Usage")
	bool bPlayerBGoalkeeperCardUsed = false;
};

UENUM(BlueprintType)
enum class EMatchPlayGoalkeeperUsageErrorCode : uint8
{
	None UMETA(DisplayName = "None"),
	InvalidPlayerSide UMETA(DisplayName = "Invalid Player Side"),
	GoalkeeperAlreadyUsed UMETA(DisplayName = "Goalkeeper Already Used")
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayGoalkeeperUsageQueryResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Goalkeeper Usage")
	bool bSucceeded = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Goalkeeper Usage")
	EInitialTurnOrderPlayer PlayerSide = EInitialTurnOrderPlayer::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Goalkeeper Usage")
	bool bGoalkeeperCardUsed = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Goalkeeper Usage")
	EMatchPlayGoalkeeperUsageErrorCode ErrorCode =
		EMatchPlayGoalkeeperUsageErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Goalkeeper Usage")
	FString ErrorMessage;
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayGoalkeeperUsageUpdateResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Goalkeeper Usage")
	bool bSucceeded = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Goalkeeper Usage")
	EInitialTurnOrderPlayer PlayerSide = EInitialTurnOrderPlayer::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Goalkeeper Usage")
	FMatchPlayGoalkeeperUsageState UpdatedUsageState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Goalkeeper Usage")
	EMatchPlayGoalkeeperUsageErrorCode ErrorCode =
		EMatchPlayGoalkeeperUsageErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Goalkeeper Usage")
	FString ErrorMessage;
};

class FMCODEX_API FMatchPlayGoalkeeperUsageStateResolver final
{
public:
	static FMatchPlayGoalkeeperUsageQueryResult Query(
		const FMatchPlayGoalkeeperUsageState& BeforeState,
		EInitialTurnOrderPlayer PlayerSide);

	static FMatchPlayGoalkeeperUsageUpdateResult MarkUsed(
		const FMatchPlayGoalkeeperUsageState& BeforeState,
		EInitialTurnOrderPlayer PlayerSide);
};
