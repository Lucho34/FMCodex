#pragma once

#include "CoreMinimal.h"

#include "MatchPlayDeploymentSlotCatalog.h"
#include "MatchRuntimeStateTypes.h"
#include "PlayCardResolver.h"

#include "MatchPlayState.generated.h"

UENUM(BlueprintType)
enum class EMatchPlayCurrentAttackPhase : uint8
{
	Deployment UMETA(DisplayName = "Deployment"),
	Resolution UMETA(DisplayName = "Resolution")
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayDeploymentPlacement
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack")
	EInitialTurnOrderPlayer PlayerSide = EInitialTurnOrderPlayer::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack")
	FName CardId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack")
	FName SlotId = NAME_None;
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayCurrentAttackState
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack")
	EMatchPlayCurrentAttackPhase Phase =
		EMatchPlayCurrentAttackPhase::Deployment;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack")
	int64 AttackSequence = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack")
	int32 ActionPoint = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack")
	EInitialTurnOrderPlayer CurrentLegalDeploymentSide =
		EInitialTurnOrderPlayer::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack")
	bool bAttackerDeploymentFinished = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack")
	bool bDefenderDeploymentFinished = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack")
	TArray<FMatchPlayDeploymentPlacement> DeploymentPlacements;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack")
	bool bCurrentDefenseGoalkeeperActivated = false;
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayState
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play")
	FMatchRuntimeState RuntimeState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play")
	FMatchCardUsageState CardUsageState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play")
	FMatchPlayDeploymentSlotCatalog DeploymentSlotCatalog;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play")
	bool bHasCurrentAttack = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play")
	FMatchPlayCurrentAttackState CurrentAttack;

private:
	friend class FMatchPlayStateInitializer;

	static FMatchPlayState Create(
		const FMatchRuntimeState& InRuntimeState,
		const FMatchCardUsageState& InCardUsageState,
		const FMatchPlayDeploymentSlotCatalog& InDeploymentSlotCatalog)
	{
		FMatchPlayState Result;
		Result.RuntimeState = InRuntimeState;
		Result.CardUsageState = InCardUsageState;
		Result.DeploymentSlotCatalog = InDeploymentSlotCatalog;
		return Result;
	}
};
