#pragma once

#include "CoreMinimal.h"

#include "MatchPlayState.h"
#include "SkillRuleSnapshotQuery.h"

#include "MatchPlayCurrentAttackActionSelectionLegality.generated.h"

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayCurrentAttackActionSelectionRequest
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Rules|Match Play|Current Attack Action Selection")
	int64 AttackSequence = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Rules|Match Play|Current Attack Action Selection")
	EInitialTurnOrderPlayer RequestingSide =
		EInitialTurnOrderPlayer::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Rules|Match Play|Current Attack Action Selection")
	FName CarrierCardId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Rules|Match Play|Current Attack Action Selection")
	FName SkillId = NAME_None;
};

UENUM(BlueprintType)
enum class EMatchPlayCurrentAttackActionSelectionErrorCode : uint8
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
	InvalidRequestingSide UMETA(DisplayName = "Invalid Requesting Side"),
	RequestingSideIsNotCurrentAttacker
		UMETA(DisplayName = "Requesting Side Is Not Current Attacker"),
	DeploymentNotFullyFinished
		UMETA(DisplayName = "Deployment Not Fully Finished"),
	InvalidCurrentLegalDeploymentSide
		UMETA(DisplayName = "Invalid Current Legal Deployment Side"),
	InvalidSelectedActionState
		UMETA(DisplayName = "Invalid Selected Action State"),
	ActionAlreadySelected UMETA(DisplayName = "Action Already Selected"),
	InvalidCarrierCardId UMETA(DisplayName = "Invalid Carrier Card Id"),
	InvalidSkillId UMETA(DisplayName = "Invalid Skill Id"),
	CarrierNotDeployed UMETA(DisplayName = "Carrier Not Deployed"),
	CarrierDeploymentAmbiguous
		UMETA(DisplayName = "Carrier Deployment Ambiguous"),
	CarrierSnapshotLookupFailed
		UMETA(DisplayName = "Carrier Snapshot Lookup Failed"),
	CarrierIsGoalkeeper UMETA(DisplayName = "Carrier Is Goalkeeper"),
	SkillNotOwnedByCarrier
		UMETA(DisplayName = "Skill Not Owned By Carrier"),
	SkillRuleSetValidationFailed
		UMETA(DisplayName = "Skill Rule Set Validation Failed"),
	SkillRuleLookupFailed UMETA(DisplayName = "Skill Rule Lookup Failed"),
	UnsupportedActionType UMETA(DisplayName = "Unsupported Action Type"),
	InvalidCurrentActionPoint
		UMETA(DisplayName = "Invalid Current Action Point"),
	ActionPointOutsideSkillRange
		UMETA(DisplayName = "Action Point Outside Skill Range")
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayCurrentAttackActionSelectionLegalityResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Action Selection")
	bool bIsLegal = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Action Selection")
	FMatchPlayCurrentAttackActionSelectionRequest Request;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Action Selection")
	EMatchPlayCurrentAttackActionSelectionErrorCode ErrorCode =
		EMatchPlayCurrentAttackActionSelectionErrorCode::None;

	EMatchPlayCardSnapshotAuthorityQueryErrorCode
		UnderlyingSnapshotAuthorityQueryErrorCode =
			EMatchPlayCardSnapshotAuthorityQueryErrorCode::None;

	ESkillRuleSnapshotValidationErrorCode
		UnderlyingSkillRuleSetValidationErrorCode =
			ESkillRuleSnapshotValidationErrorCode::None;

	ESkillRuleSnapshotQueryErrorCode UnderlyingSkillRuleQueryErrorCode =
		ESkillRuleSnapshotQueryErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Action Selection")
	ESkillRuleType ResolvedActionType = ESkillRuleType::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Action Selection")
	int32 ResolvedMinTriggerActionPoint = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Action Selection")
	int32 ResolvedMaxTriggerActionPoint = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Action Selection")
	int32 MatchingCarrierPlacementCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Action Selection")
	FString ErrorMessage;
};

class FMCODEX_API
	FMatchPlayCurrentAttackActionSelectionLegalityEvaluator final
{
public:
	static FMatchPlayCurrentAttackActionSelectionLegalityResult Evaluate(
		const FMatchPlayState& BeforeState,
		const FMatchPlayCurrentAttackActionSelectionRequest& Request,
		const FSkillRuleSnapshotSet& SkillRules);
};
