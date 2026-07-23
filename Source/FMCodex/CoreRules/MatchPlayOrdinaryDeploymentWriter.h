#pragma once

#include "CoreMinimal.h"

#include "MatchPlayDeploymentTurnRotation.h"
#include "MatchPlayOrdinaryDeploymentLegality.h"

#include "MatchPlayOrdinaryDeploymentWriter.generated.h"

class FMatchPlayOrdinaryDeploymentWriterRotationFailureContractTest;

UENUM(BlueprintType)
enum class EMatchPlayOrdinaryDeploymentWriterErrorCode : uint8
{
	None UMETA(DisplayName = "None"),
	LegalityFailed UMETA(DisplayName = "Legality Failed"),
	TurnRotationFailed UMETA(DisplayName = "Turn Rotation Failed")
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayOrdinaryDeploymentWriterResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Ordinary Deployment Writer")
	bool bSuccess = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Ordinary Deployment Writer")
	FMatchPlayOrdinaryDeploymentRequest Request;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Ordinary Deployment Writer")
	FMatchPlayState BeforeState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Ordinary Deployment Writer")
	FMatchPlayState AfterState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Ordinary Deployment Writer")
	EMatchPlayOrdinaryDeploymentWriterErrorCode ErrorCode =
		EMatchPlayOrdinaryDeploymentWriterErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Ordinary Deployment Writer")
	FMatchPlayOrdinaryDeploymentLegalityResult LegalityResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Ordinary Deployment Writer")
	EMatchPlayDeploymentTurnRotationErrorCode
		UnderlyingTurnRotationErrorCode =
			EMatchPlayDeploymentTurnRotationErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Ordinary Deployment Writer")
	FString ErrorMessage;
};

class FMCODEX_API FMatchPlayOrdinaryDeploymentWriter final
{
public:
	static FMatchPlayOrdinaryDeploymentWriterResult Deploy(
		const FMatchPlayState& BeforeState,
		const FMatchPlayOrdinaryDeploymentRequest& Request);

private:
	friend class
		FMatchPlayOrdinaryDeploymentWriterRotationFailureContractTest;

	static FMatchPlayOrdinaryDeploymentWriterResult
		FinalizeAfterRotation(
			const FMatchPlayState& BeforeState,
			const FMatchPlayOrdinaryDeploymentRequest& Request,
			FMatchPlayOrdinaryDeploymentWriterResult Result,
			const FMatchPlayDeploymentTurnRotationResult& RotationResult);
};
