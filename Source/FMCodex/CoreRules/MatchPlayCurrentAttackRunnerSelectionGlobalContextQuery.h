#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCurrentAttackRunnerSelectionTypes.h"
#include "MatchPlayCurrentAttackSelectionStateValidator.h"
#include "MatchPlayDeploymentSlotCatalog.h"
#include "MatchPlaySkillParticipantRequirementQuery.h"
#include "PlayerCardRuleSnapshotValidator.h"

struct FMCODEX_API
	FMatchPlayCurrentAttackRunnerSelectionGlobalContextResult
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
	FMatchPlaySkillParticipantRequirementResult
		ParticipantRequirementResult;
	int32 MatchingFrozenCarrierPlacementCount = 0;
	int32 MatchingFrozenMarkerPlacementCount = 0;
	FMatchPlayDeploymentPlacement FrozenCarrierPlacement;
	FMatchPlayDeploymentPlacement FrozenMarkerPlacement;
	TArray<FMatchPlayDeploymentPlacement> AttackingPlayerPlacements;
	FPlayerCardRuleSnapshotValidationResult
		AttackingSnapshotSetValidationResult;
	FMatchPlayDeploymentSlotCatalogValidationResult
		SlotCatalogValidationResult;
	EMatchPlayCurrentAttackRunnerSelectionErrorCode ErrorCode =
		EMatchPlayCurrentAttackRunnerSelectionErrorCode::None;
	FString ErrorMessage;
};

class FMCODEX_API
	FMatchPlayCurrentAttackRunnerSelectionGlobalContextQuery final
{
public:
	static FMatchPlayCurrentAttackRunnerSelectionGlobalContextResult Query(
		const FMatchPlayState& State,
		int64 AttackSequence,
		EInitialTurnOrderPlayer RequestingSide);
};
