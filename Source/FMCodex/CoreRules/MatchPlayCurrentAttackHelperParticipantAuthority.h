#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCardSnapshotAuthority.h"
#include "MatchPlayCurrentAttackHelperSelectionTypes.h"
#include "MatchPlayState.h"

struct FMatchPlayCurrentAttackHelperParticipantAuthorityResult
{
	bool bSuccess = false;
	int32 MatchingPlacementCount = 0;
	FMatchPlayDeploymentPlacement Placement;
	FMatchPlayCardSnapshotAuthorityQueryResult SnapshotQueryResult;
	FPlayerCardRuleSnapshot Snapshot;
	EMatchPlayCurrentAttackHelperSelectionErrorCode ErrorCode =
		EMatchPlayCurrentAttackHelperSelectionErrorCode::None;
	FString ErrorMessage;
};

class FMatchPlayCurrentAttackHelperParticipantAuthority final
{
public:
	static FMatchPlayCurrentAttackHelperParticipantAuthorityResult
		Evaluate(
			const FMatchPlayState& State,
			EInitialTurnOrderPlayer DefendingSide,
			FName MarkerCardId,
			FName HelperCardId);
};
