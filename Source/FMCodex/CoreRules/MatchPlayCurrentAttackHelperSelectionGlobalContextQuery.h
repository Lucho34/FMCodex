#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCurrentAttackHelperSelectionTypes.h"
#include "MatchPlayCurrentAttackSelectionStateValidator.h"
#include "MatchPlaySkillParticipantRequirementQuery.h"
#include "PlayerCardRuleSnapshotValidator.h"

struct FMCODEX_API
	FMatchPlayCurrentAttackHelperSelectionGlobalContextResult
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
	FName FrozenCarrierCardId = NAME_None;
	FName FrozenMarkerCardId = NAME_None;
	FName FrozenSkillId = NAME_None;
	ESkillRuleType FrozenActionType = ESkillRuleType::None;
	FName FrozenRunnerCardId = NAME_None;
	FMatchPlaySkillParticipantRequirementResult
		ParticipantRequirementResult;
	int32 MatchingFrozenCarrierPlacementCount = 0;
	int32 MatchingFrozenMarkerPlacementCount = 0;
	int32 MatchingFrozenRunnerPlacementCount = 0;
	FMatchPlayDeploymentPlacement FrozenCarrierPlacement;
	FMatchPlayDeploymentPlacement FrozenMarkerPlacement;
	FMatchPlayDeploymentPlacement FrozenRunnerPlacement;
	TArray<FMatchPlayDeploymentPlacement> DefendingPlayerPlacements;
	FPlayerCardRuleSnapshotValidationResult
		AttackingSnapshotSetValidationResult;
	FPlayerCardRuleSnapshotValidationResult
		DefendingSnapshotSetValidationResult;
	EMatchPlayCurrentAttackHelperSelectionErrorCode ErrorCode =
		EMatchPlayCurrentAttackHelperSelectionErrorCode::None;
	FString ErrorMessage;
};

class FMCODEX_API
	FMatchPlayCurrentAttackHelperSelectionGlobalContextQuery final
{
public:
	static FMatchPlayCurrentAttackHelperSelectionGlobalContextResult
		Query(
			const FMatchPlayState& State,
			int64 AttackSequence,
			EInitialTurnOrderPlayer RequestingSide);
};
