#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCurrentAttackCompletion.h"
#include "SkillRuleSnapshot.h"

#include "MatchPlaySkillNoSelectionNoGoal.generated.h"

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayResolveNoLegalSkillRequest
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Rules|Match Play|Skill No-selection NoGoal")
	int64 AttackSequence = 0;
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlaySkillDeclineRequest
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Rules|Match Play|Skill No-selection NoGoal")
	int64 AttackSequence = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Rules|Match Play|Skill No-selection NoGoal")
	EInitialTurnOrderPlayer RequestingSide =
		EInitialTurnOrderPlayer::None;
};

UENUM(BlueprintType)
enum class EMatchPlaySkillNoSelectionNoGoalErrorCode : uint8
{
	None UMETA(DisplayName = "None"),
	MatchPlayStateNotInitialized
		UMETA(DisplayName = "Match Play State Not Initialized"),
	NoCurrentAttack UMETA(DisplayName = "No Current Attack"),
	InvalidCurrentAttackSequence
		UMETA(DisplayName = "Invalid Current Attack Sequence"),
	AttackSequenceMismatch UMETA(DisplayName = "Attack Sequence Mismatch"),
	CurrentAttackNotInResolution
		UMETA(DisplayName = "Current Attack Not In Resolution"),
	InvalidCurrentAttackingPlayer
		UMETA(DisplayName = "Invalid Current Attacking Player"),
	InvalidCurrentDefendingPlayer
		UMETA(DisplayName = "Invalid Current Defending Player"),
	InvalidSelectionState UMETA(DisplayName = "Invalid Selection State"),
	WrongSelectionStage UMETA(DisplayName = "Wrong Selection Stage"),
	InvalidRequestingSide UMETA(DisplayName = "Invalid Requesting Side"),
	RequestingSideIsNotCurrentAttacker
		UMETA(DisplayName = "Requesting Side Is Not Current Attacker"),
	AvailabilityQueryFailed
		UMETA(DisplayName = "Availability Query Failed"),
	LegalSkillExists UMETA(DisplayName = "Legal Skill Exists"),
	NoLegalSkillToDecline
		UMETA(DisplayName = "No Legal Skill To Decline"),
	InvalidCapability UMETA(DisplayName = "Invalid Capability"),
	InvalidCapabilitySourceReason
		UMETA(DisplayName = "Invalid Capability Source Reason"),
	CompletionFailed UMETA(DisplayName = "Completion Failed")
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayResolveNoLegalSkillResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Skill No-selection NoGoal")
	bool bSuccess = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Skill No-selection NoGoal")
	FMatchPlayResolveNoLegalSkillRequest Request;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Skill No-selection NoGoal")
	FMatchPlayState BeforeState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Skill No-selection NoGoal")
	FMatchPlayState AfterState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Skill No-selection NoGoal")
	FMatchPlayCurrentAttackSkillSelectionAvailabilityResult
		SkillAvailabilityResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Skill No-selection NoGoal")
	EMatchPlaySkillNoSelectionNoGoalReason Reason =
		EMatchPlaySkillNoSelectionNoGoalReason::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Skill No-selection NoGoal")
	EMatchPlaySkillNoSelectionNoGoalSource Source =
		EMatchPlaySkillNoSelectionNoGoalSource::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Skill No-selection NoGoal")
	FMatchPlayCurrentAttackCompletionResult CompletionResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Skill No-selection NoGoal")
	EMatchPlaySkillNoSelectionNoGoalErrorCode ErrorCode =
		EMatchPlaySkillNoSelectionNoGoalErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Skill No-selection NoGoal")
	FString ErrorMessage;
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlaySkillDeclineResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Skill No-selection NoGoal")
	bool bSuccess = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Skill No-selection NoGoal")
	FMatchPlaySkillDeclineRequest Request;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Skill No-selection NoGoal")
	FMatchPlayState BeforeState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Skill No-selection NoGoal")
	FMatchPlayState AfterState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Skill No-selection NoGoal")
	FMatchPlayCurrentAttackSkillSelectionAvailabilityResult
		SkillAvailabilityResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Skill No-selection NoGoal")
	EMatchPlaySkillNoSelectionNoGoalReason Reason =
		EMatchPlaySkillNoSelectionNoGoalReason::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Skill No-selection NoGoal")
	EMatchPlaySkillNoSelectionNoGoalSource Source =
		EMatchPlaySkillNoSelectionNoGoalSource::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Skill No-selection NoGoal")
	FMatchPlayCurrentAttackCompletionResult CompletionResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Skill No-selection NoGoal")
	EMatchPlaySkillNoSelectionNoGoalErrorCode ErrorCode =
		EMatchPlaySkillNoSelectionNoGoalErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Skill No-selection NoGoal")
	FString ErrorMessage;
};

class FMCODEX_API FMatchPlayResolveNoLegalSkill final
{
public:
	static FMatchPlayResolveNoLegalSkillResult Resolve(
		const FMatchPlayState& BeforeState,
		const FSkillRuleSnapshotSet& SkillRuleSet,
		const FMatchPlayResolveNoLegalSkillRequest& Request);
};

class FMCODEX_API FMatchPlaySkillDecline final
{
public:
	static FMatchPlaySkillDeclineResult Decline(
		const FMatchPlayState& BeforeState,
		const FSkillRuleSnapshotSet& SkillRuleSet,
		const FMatchPlaySkillDeclineRequest& Request);
};
