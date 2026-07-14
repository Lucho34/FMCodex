#pragma once

#include "CoreMinimal.h"

#include "PlayerCardRuleSnapshotValidator.h"
#include "SkillRuleSnapshotQuery.h"

enum class EThroughBallParticipantEligibilityQueryErrorCode : uint8
{
	None,
	InvalidSelectedSkillId,
	SkillRuleLookupFailed,
	UnsupportedSkillRuleType,
	ActionPointBelowMinimum,
	ActionPointAboveMaximum,
	InvalidAttackingOwnerIdentity,
	InvalidDefendingOwnerIdentity,
	DuplicateOwnerIdentity,
	InvalidCarrierIdentity,
	InvalidCarrierSnapshot,
	UnsupportedCarrierGoalkeeper,
	CarrierDoesNotOwnSelectedSkill,
	InvalidRunnerIdentity,
	InvalidRunnerSnapshot,
	UnsupportedRunnerGoalkeeper,
	RunnerNotInAttackingForwardArea,
	DuplicateAttackingParticipant,
	InvalidMarkerIdentity,
	InvalidMarkerSnapshot,
	UnsupportedMarkerGoalkeeper,
	InvalidHelperIdentity,
	InvalidHelperSnapshot,
	UnsupportedHelperGoalkeeper,
	DuplicateDefendingParticipant
};

struct FMCODEX_API FThroughBallParticipantEligibilityQueryInput
{
	FName SelectedSkillId = NAME_None;
	int32 CurrentActionPoint = 0;

	FName AttackingOwnerId = NAME_None;
	FName DefendingOwnerId = NAME_None;

	FPlayerCardRuleSnapshot CarrierSnapshot;
	FPlayerCardRuleSnapshot RunnerSnapshot;
	FPlayerCardRuleSnapshot MarkerSnapshot;

	bool bHasHelper = false;
	FPlayerCardRuleSnapshot HelperSnapshot;

	bool bIsRunnerInAttackingForwardArea = false;
};

struct FMCODEX_API FThroughBallParticipantEligibilityQueryResult
{
	bool bSuccess = false;

	EThroughBallParticipantEligibilityQueryErrorCode ErrorCode =
		EThroughBallParticipantEligibilityQueryErrorCode::None;

	FString ErrorMessage;
	FName InvalidField = NAME_None;

	FThroughBallParticipantEligibilityQueryInput Input;
	bool bHasHelper = false;

	FSkillRuleSnapshotQueryResult SkillRuleQueryResult;

	FPlayerCardRuleSnapshotValidationResult
		CarrierSnapshotValidationResult;
	FPlayerCardRuleSnapshotValidationResult
		RunnerSnapshotValidationResult;
	FPlayerCardRuleSnapshotValidationResult
		MarkerSnapshotValidationResult;
	FPlayerCardRuleSnapshotValidationResult
		HelperSnapshotValidationResult;
};

class FMCODEX_API FThroughBallParticipantEligibilityQuery final
{
public:
	static FThroughBallParticipantEligibilityQueryResult Evaluate(
		const FSkillRuleSnapshotSet& SkillRules,
		const FThroughBallParticipantEligibilityQueryInput& Input);
};
