#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCardSnapshotAuthority.h"
#include "MatchPlayDeploymentSlotCatalog.h"
#include "MatchPlayGoalkeeperUsageState.h"
#include "MatchRuntimeStateTypes.h"
#include "PlayCardResolver.h"
#include "SkillRuleSnapshot.h"

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
struct FMCODEX_API FMatchPlayCurrentAttackSelectedAction
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack")
	FName CarrierCardId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack")
	FName SkillId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack")
	ESkillRuleType ActionType = ESkillRuleType::None;
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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack")
	bool bHasSelectedAction = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack")
	FMatchPlayCurrentAttackSelectedAction SelectedAction;
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
	FMatchPlayPerSideCardSnapshotAuthority CardSnapshotAuthority;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play")
	FMatchPlayGoalkeeperUsageState GoalkeeperUsageState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play")
	bool bHasCurrentAttack = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play")
	FMatchPlayCurrentAttackState CurrentAttack;

private:
	friend class FMatchPlayStateInitializer;

	static FMatchPlayState Create(
		const FMatchRuntimeState& InRuntimeState,
		const FMatchCardUsageState& InCardUsageState,
		const FMatchPlayDeploymentSlotCatalog& InDeploymentSlotCatalog,
		const FMatchPlayPerSideCardSnapshotAuthority&
			InCardSnapshotAuthority,
		const FMatchPlayGoalkeeperUsageState& InGoalkeeperUsageState)
	{
		FMatchPlayState Result;
		Result.RuntimeState = InRuntimeState;
		Result.CardUsageState = InCardUsageState;
		Result.DeploymentSlotCatalog = InDeploymentSlotCatalog;
		Result.CardSnapshotAuthority = InCardSnapshotAuthority;
		Result.GoalkeeperUsageState = InGoalkeeperUsageState;
		return Result;
	}
};
