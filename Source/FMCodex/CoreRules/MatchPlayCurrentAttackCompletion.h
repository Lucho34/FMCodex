#pragma once

#include "CoreMinimal.h"

#include "AttackOpportunityResolver.h"
#include "GoalResolver.h"
#include "MatchPlayCrossResolutionTerminalCapability.h"
#include "MatchPlayPassControlResolutionTerminalCapability.h"
#include "MatchPlayShotResolutionTerminalCapability.h"
#include "MatchPlayCurrentAttackSelectionStateValidator.h"
#include "MatchPlayThroughBallResolutionTerminalCapability.h"
#include "MatchPlayMarkerNoSelectionGoalCapability.h"
#include "MatchPlayRunnerNoSelectionNoGoalCapability.h"
#include "MatchPlaySkillNoSelectionNoGoalCapability.h"
#include "MatchResultResolver.h"
#include "PlayCardResolver.h"

#include "MatchPlayCurrentAttackCompletion.generated.h"

UENUM(BlueprintType)
enum class EMatchPlayCurrentAttackCompletionErrorCode : uint8
{
	None UMETA(DisplayName = "None"),
	MatchPlayStateNotInitialized
		UMETA(DisplayName = "Match Play State Not Initialized"),
	NoCurrentAttack UMETA(DisplayName = "No Current Attack"),
	InvalidCurrentAttackSequence
		UMETA(DisplayName = "Invalid Current Attack Sequence"),
	UnsupportedCapabilitySource
		UMETA(DisplayName = "Unsupported Capability Source"),
	InvalidCapabilityReason
		UMETA(DisplayName = "Invalid Capability Reason"),
	CapabilitySequenceMismatch
		UMETA(DisplayName = "Capability Sequence Mismatch"),
	CurrentAttackNotInResolution
		UMETA(DisplayName = "Current Attack Not In Resolution"),
	InvalidSelectionState UMETA(DisplayName = "Invalid Selection State"),
	WrongSelectionStage UMETA(DisplayName = "Wrong Selection Stage"),
	InvalidCurrentAttackingPlayer
		UMETA(DisplayName = "Invalid Current Attacking Player"),
	InvalidCurrentDefendingPlayer
		UMETA(DisplayName = "Invalid Current Defending Player"),
	InvalidCapabilityProvenance
		UMETA(DisplayName = "Invalid Capability Provenance"),
	InvalidScoreState UMETA(DisplayName = "Invalid Score State"),
	InvalidOpportunityState
		UMETA(DisplayName = "Invalid Opportunity State"),
	InvalidDeploymentPlacement
		UMETA(DisplayName = "Invalid Deployment Placement"),
	DeploymentSnapshotQueryFailed
		UMETA(DisplayName = "Deployment Snapshot Query Failed"),
	DuplicateDeploymentCard
		UMETA(DisplayName = "Duplicate Deployment Card"),
	DuplicateDeploymentSlot
		UMETA(DisplayName = "Duplicate Deployment Slot"),
	InvalidGoalkeeperCompletionState
		UMETA(DisplayName = "Invalid Goalkeeper Completion State"),
	GoalResolutionFailed UMETA(DisplayName = "Goal Resolution Failed"),
	OrdinaryCardUsageConsumptionFailed
		UMETA(DisplayName = "Ordinary Card Usage Consumption Failed"),
	OpportunityConsumptionFailed
		UMETA(DisplayName = "Opportunity Consumption Failed"),
	NextAttackerResolutionFailed
		UMETA(DisplayName = "Next Attacker Resolution Failed"),
	MatchEndResolutionFailed
		UMETA(DisplayName = "Match End Resolution Failed"),
	MatchResultResolutionFailed
		UMETA(DisplayName = "Match Result Resolution Failed")
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayCurrentAttackCompletionResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Completion")
	bool bSuccess = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Completion")
	FMatchPlayState BeforeState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Completion")
	FMatchPlayState AfterState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Completion")
	EMatchPlayMarkerNoSelectionGoalReason Reason =
		EMatchPlayMarkerNoSelectionGoalReason::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Completion")
	EMatchPlayMarkerNoSelectionGoalSource Source =
		EMatchPlayMarkerNoSelectionGoalSource::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Completion")
	EMatchPlayCurrentAttackCompletionErrorCode ErrorCode =
		EMatchPlayCurrentAttackCompletionErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Completion")
	FMatchPlayCurrentAttackSelectionStateValidationResult
		SelectionStateValidationResult;

	TArray<FMatchPlayCardSnapshotAuthorityQueryResult>
		DeploymentSnapshotQueryResults;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Completion")
	TArray<FPlayCardResolveResult> OrdinaryCardUsageResults;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Completion")
	FGoalResolveResult GoalResolveResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Completion")
	FAttackOpportunityResolveResult OpportunityResolveResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Completion")
	FMatchEndResolveResult MatchEndResolveResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Completion")
	FMatchResultResolveResult MatchResultResolveResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Completion")
	EInitialTurnOrderPlayer ScoringSide =
		EInitialTurnOrderPlayer::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Completion")
	EInitialTurnOrderPlayer NextAttackingPlayer =
		EInitialTurnOrderPlayer::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Completion")
	bool bMatchEnded = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Completion")
	FString ErrorMessage;
};

class FMCODEX_API FMatchPlayCurrentAttackCompletion final
{
private:
	friend class FMatchPlayMarkerDecline;
	friend class FMatchPlayResolveNoLegalMarker;
	friend class FMatchPlayResolveNoLegalSkill;
	friend class FMatchPlayResolveNoLegalRunner;
	friend class FMatchPlayRunnerDecline;
	friend class FMatchPlaySkillDecline;
	friend class
		FMatchPlayCurrentAttackApplyCrossTerminalResolutionOrchestrator;
	friend class
		FMatchPlayCurrentAttackApplyPassControlTerminalResolutionOrchestrator;
	friend class
		FMatchPlayCurrentAttackApplyShotTerminalResolutionOrchestrator;
	friend class
		FMatchPlayCurrentAttackApplyThroughBallTerminalResolutionOrchestrator;

	FMatchPlayCurrentAttackCompletion() = delete;

	static FMatchPlayCurrentAttackCompletionResult CompleteMarkerGoal(
		const FMatchPlayState& BeforeState,
		const FMatchPlayMarkerNoSelectionGoalCapability& Capability);

	static FMatchPlayCurrentAttackCompletionResult CompleteSkillNoGoal(
		const FMatchPlayState& BeforeState,
		const FMatchPlaySkillNoSelectionNoGoalCapability& Capability);

	static FMatchPlayCurrentAttackCompletionResult CompleteRunnerNoGoal(
		const FMatchPlayState& BeforeState,
		const FMatchPlayRunnerNoSelectionNoGoalCapability& Capability);

	static FMatchPlayCurrentAttackCompletionResult
	ApplyCurrentAttackTerminalMutation(
		const FMatchPlayState& BeforeState,
		FMatchPlayState WorkingState,
		EInitialTurnOrderPlayer Attacker,
		EInitialTurnOrderPlayer Defender,
		FMatchPlayCurrentAttackCompletionResult Result);

	static FMatchPlayCurrentAttackCompletionResult
	CompleteCrossResolution(
		const FMatchPlayState& BeforeState,
		const FMatchPlayCrossResolutionTerminalCapability& Capability);

	static FMatchPlayCurrentAttackCompletionResult
	CompletePassControlResolution(
		const FMatchPlayState& BeforeState,
		const FMatchPlayPassControlResolutionTerminalCapability& Capability);

	static FMatchPlayCurrentAttackCompletionResult CompleteShotResolution(
		const FMatchPlayState& BeforeState,
		const FMatchPlayShotResolutionTerminalCapability& Capability);

	static FMatchPlayCurrentAttackCompletionResult
	CompleteThroughBallResolution(
		const FMatchPlayState& BeforeState,
		const FMatchPlayThroughBallResolutionTerminalCapability& Capability);
};
