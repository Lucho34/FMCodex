#pragma once

#include "CoreMinimal.h"

#include "MatchPlayState.h"

#include "MatchPlayDeploymentTurnRotation.generated.h"

UENUM(BlueprintType)
enum class EMatchPlayDeploymentTurnRotationErrorCode : uint8
{
	None UMETA(DisplayName = "None"),
	InvalidCurrentAttackingPlayer
		UMETA(DisplayName = "Invalid Current Attacking Player"),
	InvalidActingSide UMETA(DisplayName = "Invalid Acting Side")
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayDeploymentTurnRotationResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Deployment Turn Rotation")
	bool bSuccess = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Deployment Turn Rotation")
	EMatchPlayDeploymentTurnRotationErrorCode ErrorCode =
		EMatchPlayDeploymentTurnRotationErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Deployment Turn Rotation")
	EMatchPlayCurrentAttackPhase NextPhase =
		EMatchPlayCurrentAttackPhase::Deployment;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Deployment Turn Rotation")
	EInitialTurnOrderPlayer NextLegalDeploymentSide =
		EInitialTurnOrderPlayer::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Deployment Turn Rotation")
	FString ErrorMessage;
};

class FMCODEX_API FMatchPlayDeploymentTurnRotation final
{
public:
	static FMatchPlayDeploymentTurnRotationResult Resolve(
		EInitialTurnOrderPlayer CurrentAttackingPlayer,
		EInitialTurnOrderPlayer ActingSide,
		bool bAttackerDeploymentFinished,
		bool bDefenderDeploymentFinished);
};
