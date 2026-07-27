#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCurrentAttackSelectionStateValidator.h"
#include "MatchPlayCurrentAttackSkillSelectionTypes.h"
#include "SkillRuleSnapshotValidator.h"

struct FMCODEX_API
	FMatchPlayCurrentAttackSkillSelectionGlobalContextResult
{
	bool bSuccess = false;
	int64 RequestedAttackSequence = 0;
	EInitialTurnOrderPlayer RequestingSide =
		EInitialTurnOrderPlayer::None;
	EMatchPlayCurrentAttackSkillSelectionErrorCode ErrorCode =
		EMatchPlayCurrentAttackSkillSelectionErrorCode::None;
	int64 AuthoritativeAttackSequence = 0;
	EInitialTurnOrderPlayer CurrentAttackingPlayer =
		EInitialTurnOrderPlayer::None;
	EInitialTurnOrderPlayer CurrentDefendingPlayer =
		EInitialTurnOrderPlayer::None;
	FMatchPlayCurrentAttackSelectionStateValidationResult
		SelectionStateValidationResult;
	FName FrozenCarrierCardId = NAME_None;
	FName FrozenMarkerCardId = NAME_None;
	int32 MatchingFrozenCarrierPlacementCount = 0;
	int32 MatchingFrozenMarkerPlacementCount = 0;
	FMatchPlayDeploymentPlacement FrozenCarrierPlacement;
	FMatchPlayDeploymentPlacement FrozenMarkerPlacement;
	FMatchPlayCardSnapshotAuthorityQueryResult
		CarrierSnapshotQueryResult;
	FPlayerCardRuleSnapshot ResolvedCarrierSnapshot;
	FSkillRuleSnapshotValidationResult
		SkillRuleSetValidationResult;
	int32 ValidatedActionPoint = 0;
	FString ErrorMessage;
};

class FMCODEX_API
	FMatchPlayCurrentAttackSkillSelectionGlobalContextQuery final
{
public:
	static FMatchPlayCurrentAttackSkillSelectionGlobalContextResult Query(
		const FMatchPlayState& State,
		int64 AttackSequence,
		EInitialTurnOrderPlayer RequestingSide,
		const FSkillRuleSnapshotSet& SkillRuleSet);
};
