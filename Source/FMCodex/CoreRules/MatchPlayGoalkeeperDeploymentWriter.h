#pragma once

#include "CoreMinimal.h"

#include "MatchPlayDeploymentTurnRotation.h"
#include "MatchPlayGoalkeeperDeploymentLegality.h"

#include "MatchPlayGoalkeeperDeploymentWriter.generated.h"

class FMatchPlayGoalkeeperDeploymentWriterRotationFailureContractTest;
class FMatchPlayGoalkeeperDeploymentWriterUsageFailureContractTest;

UENUM(BlueprintType)
enum class EMatchPlayGoalkeeperDeploymentWriterErrorCode : uint8
{
	None UMETA(DisplayName = "None"),
	LegalityFailed UMETA(DisplayName = "Legality Failed"),
	TurnRotationFailed UMETA(DisplayName = "Turn Rotation Failed"),
	GoalkeeperUsageUpdateFailed
		UMETA(DisplayName = "Goalkeeper Usage Update Failed")
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayGoalkeeperDeploymentWriterResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Goalkeeper Deployment Writer")
	bool bSucceeded = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Goalkeeper Deployment Writer")
	FMatchPlayGoalkeeperDeploymentRequest Request;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Goalkeeper Deployment Writer")
	FMatchPlayState BeforeState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Goalkeeper Deployment Writer")
	FMatchPlayState AfterState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Goalkeeper Deployment Writer")
	EMatchPlayGoalkeeperDeploymentWriterErrorCode ErrorCode =
		EMatchPlayGoalkeeperDeploymentWriterErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Goalkeeper Deployment Writer")
	FMatchPlayGoalkeeperDeploymentLegalityResult LegalityResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Goalkeeper Deployment Writer")
	EMatchPlayDeploymentTurnRotationErrorCode
		UnderlyingTurnRotationErrorCode =
			EMatchPlayDeploymentTurnRotationErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Goalkeeper Deployment Writer")
	EMatchPlayGoalkeeperUsageErrorCode
		UnderlyingGoalkeeperUsageErrorCode =
			EMatchPlayGoalkeeperUsageErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Goalkeeper Deployment Writer")
	FString ErrorMessage;
};

class FMCODEX_API FMatchPlayGoalkeeperDeploymentWriter final
{
public:
	static FMatchPlayGoalkeeperDeploymentWriterResult Deploy(
		const FMatchPlayState& BeforeState,
		const FMatchPlayGoalkeeperDeploymentRequest& Request);

private:
	friend class
		FMatchPlayGoalkeeperDeploymentWriterRotationFailureContractTest;
	friend class
		FMatchPlayGoalkeeperDeploymentWriterUsageFailureContractTest;

	static FMatchPlayGoalkeeperDeploymentWriterResult
		FinalizeAfterRotation(
			const FMatchPlayState& BeforeState,
			const FMatchPlayGoalkeeperDeploymentRequest& Request,
			FMatchPlayGoalkeeperDeploymentWriterResult Result,
			const FMatchPlayDeploymentTurnRotationResult& RotationResult);

	static FMatchPlayGoalkeeperDeploymentWriterResult
		FinalizeAfterUsageUpdate(
			const FMatchPlayState& BeforeState,
			const FMatchPlayGoalkeeperDeploymentRequest& Request,
			FMatchPlayGoalkeeperDeploymentWriterResult Result,
			const FMatchPlayDeploymentTurnRotationResult& RotationResult,
			const FMatchPlayGoalkeeperUsageUpdateResult& UsageUpdateResult);
};
