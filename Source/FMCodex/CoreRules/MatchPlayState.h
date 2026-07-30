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

UENUM(BlueprintType)
enum class EMatchPlayCurrentAttackSelectionStage : uint8
{
	None = 0 UMETA(DisplayName = "None"),
	AwaitingCarrier = 1 UMETA(DisplayName = "Awaiting Carrier"),
	AwaitingMarker = 2 UMETA(DisplayName = "Awaiting Marker"),
	AwaitingSkill = 3 UMETA(DisplayName = "Awaiting Skill"),
	AwaitingRunner = 4 UMETA(DisplayName = "Awaiting Runner"),
	ReadyForResolution = 5 UMETA(DisplayName = "Ready For Resolution"),
	AwaitingHelper = 6 UMETA(DisplayName = "Awaiting Helper"),
	AwaitingBranchIntent = 7
		UMETA(DisplayName = "Awaiting Branch Intent")
};

UENUM(BlueprintType)
enum class EMatchPlayElectiveBranchIntent : uint8
{
	None = 0 UMETA(DisplayName = "None"),
	DirectShot = 1 UMETA(DisplayName = "Direct Shot"),
	DeadCorner = 2 UMETA(DisplayName = "Dead Corner"),
	CrossHigh = 3 UMETA(DisplayName = "Cross High"),
	CrossLow = 4 UMETA(DisplayName = "Cross Low")
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
struct FMCODEX_API FMatchPlayCurrentAttackActionPreparationState
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack")
	FName CarrierCardId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack")
	FName MarkerCardId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack")
	FName SkillId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack")
	ESkillRuleType ActionType = ESkillRuleType::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack")
	FName RunnerCardId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack")
	bool bHasHelper = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack")
	FName HelperCardId = NAME_None;
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayCurrentAttackSelectedAction
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack")
	FName CarrierCardId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack")
	FName MarkerCardId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack")
	FName SkillId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack")
	ESkillRuleType ActionType = ESkillRuleType::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack")
	FName RunnerCardId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack")
	bool bHasHelper = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack")
	FName HelperCardId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack")
	EMatchPlayElectiveBranchIntent ElectiveBranchIntent =
		EMatchPlayElectiveBranchIntent::None;
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
	EMatchPlayCurrentAttackSelectionStage SelectionStage =
		EMatchPlayCurrentAttackSelectionStage::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack")
	FMatchPlayCurrentAttackActionPreparationState ActionPreparation;

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
