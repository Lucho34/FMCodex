#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCardSnapshotAuthority.h"
#include "MatchPlayCurrentAttackBranchIntentSelectionTypes.h"
#include "MatchPlayCurrentAttackHelperParticipantAuthority.h"
#include "MatchPlayCurrentAttackSelectionStateValidator.h"

struct FMCODEX_API
	FMatchPlayCurrentAttackBranchIntentSelectionGlobalContextResult
{
	bool bSuccess = false;
	int64 RequestedAttackSequence = 0;
	int64 AuthoritativeAttackSequence = 0;
	EInitialTurnOrderPlayer RequestingSide =
		EInitialTurnOrderPlayer::None;
	EInitialTurnOrderPlayer CurrentAttackingPlayer =
		EInitialTurnOrderPlayer::None;
	EInitialTurnOrderPlayer CurrentDefendingPlayer =
		EInitialTurnOrderPlayer::None;
	FMatchPlayCurrentAttackSelectionStateValidationResult
		SelectionStateValidationResult;
	FMatchPlayCurrentAttackActionPreparationState Preparation;
	ESkillRuleType FrozenActionType = ESkillRuleType::None;
	int32 MatchingCarrierPlacementCount = 0;
	int32 MatchingMarkerPlacementCount = 0;
	int32 MatchingRunnerPlacementCount = 0;
	FMatchPlayCardSnapshotAuthorityQueryResult
		CarrierSnapshotQueryResult;
	FMatchPlayCardSnapshotAuthorityQueryResult
		MarkerSnapshotQueryResult;
	FMatchPlayCardSnapshotAuthorityQueryResult
		RunnerSnapshotQueryResult;
	FMatchPlayCurrentAttackHelperParticipantAuthorityResult
		HelperAuthorityResult;
	EMatchPlayCurrentAttackBranchIntentSelectionErrorCode ErrorCode =
		EMatchPlayCurrentAttackBranchIntentSelectionErrorCode::None;
	FString ErrorMessage;
};

class FMCODEX_API
	FMatchPlayCurrentAttackBranchIntentSelectionGlobalContextQuery final
{
public:
	static
		FMatchPlayCurrentAttackBranchIntentSelectionGlobalContextResult
		Query(
			const FMatchPlayState& State,
			int64 AttackSequence,
			EInitialTurnOrderPlayer RequestingSide);
};
