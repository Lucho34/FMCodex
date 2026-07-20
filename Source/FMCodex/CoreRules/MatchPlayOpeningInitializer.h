#pragma once

#include "CoreMinimal.h"

#include "MatchOpeningResolver.h"
#include "MatchPlayStateInitializer.h"
#include "MatchRuntimeInitializer.h"

#include "MatchPlayOpeningInitializer.generated.h"

UENUM(BlueprintType)
enum class EMatchPlayOpeningInitializeErrorCode : uint8
{
	None UMETA(DisplayName = "None"),
	OpeningResolveFailed UMETA(DisplayName = "Opening Resolve Failed"),
	RuntimeInitializationFailed
		UMETA(DisplayName = "Runtime Initialization Failed"),
	PlayStateInitializationFailed
		UMETA(DisplayName = "Play State Initialization Failed")
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayOpeningInitializeInput
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Rules|Match Play Opening")
	FMatchOpeningResolveInput OpeningInput;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Rules|Match Play Opening")
	TArray<FName> PlayerACardIds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Rules|Match Play Opening")
	TArray<FName> PlayerBCardIds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Rules|Match Play Opening")
	FMatchPlayDeploymentSlotCatalog DeploymentSlotCatalog;
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayOpeningInitializeResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Opening")
	bool bSuccess = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Opening")
	FMatchPlayState MatchPlayState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Opening")
	EMatchPlayOpeningInitializeErrorCode ErrorCode =
		EMatchPlayOpeningInitializeErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Opening")
	TArray<EMatchOpeningResolveErrorCode> UnderlyingOpeningErrorCodes;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Opening")
	TArray<EMatchRuntimeInitializeErrorCode>
		UnderlyingRuntimeInitializeErrorCodes;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Opening")
	EMatchPlayStateInitializeErrorCode UnderlyingPlayStateInitializeErrorCode =
		EMatchPlayStateInitializeErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Opening")
	EMatchCardUsageInitializeErrorCode UnderlyingCardUsageErrorCode =
		EMatchCardUsageInitializeErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Opening")
	EMatchPlayDeploymentSlotCatalogValidationErrorCode
		UnderlyingDeploymentSlotCatalogValidationErrorCode =
			EMatchPlayDeploymentSlotCatalogValidationErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Opening")
	bool bRequiresTieBreakerReroll = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Opening")
	FString ErrorMessage;
};

class FMCODEX_API FMatchPlayOpeningInitializer final
{
public:
	static FMatchPlayOpeningInitializeResult InitializeMatchPlayOpening(
		const FMatchPlayOpeningInitializeInput& Input);
};
