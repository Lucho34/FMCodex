#pragma once

#include "CoreMinimal.h"

#include "PlayerCardRuleSnapshotQuery.h"
#include "SkillRuleSnapshotQuery.h"

enum class ECutInsideShotDeadCornerDecision : uint8
{
	None,
	Goal,
	Miss
};

enum class ECutInsideShotDeadCornerDecisionQueryErrorCode : uint8
{
	None,
	MissingSkillId,
	AttackerSnapshotQueryFailed,
	SkillRuleQueryFailed,
	SkillNotOwnedByAttacker,
	SkillTypeMismatch,
	InvalidActionPoint,
	ActionPointOutOfRange,
	UnsupportedGoalkeeperParticipant,
	MissingExternalAttackD6A,
	MissingExternalAttackD6B,
	InvalidAttackD6A,
	InvalidAttackD6B,
	InvalidLogContext
};

struct FMCODEX_API FCutInsideShotDeadCornerDecisionQueryInput
{
	FName SkillId = NAME_None;
	FName AttackerCardId = NAME_None;
	int32 CurrentActionPoint = 0;
	bool bHasExternalAttackD6A = false;
	int32 ExternalAttackD6A = 0;
	bool bHasExternalAttackD6B = false;
	int32 ExternalAttackD6B = 0;
	FGuid LogId;
	int32 TurnIndex = 0;
	FName AttackerPlayerId = NAME_None;
};

struct FMCODEX_API FCutInsideShotDeadCornerDecisionQueryResult
{
	bool bSuccess = false;
	ECutInsideShotDeadCornerDecision Decision =
		ECutInsideShotDeadCornerDecision::None;
	ECutInsideShotDeadCornerDecisionQueryErrorCode ErrorCode =
		ECutInsideShotDeadCornerDecisionQueryErrorCode::None;
	FString ErrorMessage;
	FName InvalidField = NAME_None;
	FCutInsideShotDeadCornerDecisionQueryInput Input;
	FPlayerCardRuleSnapshotQueryResult AttackerSnapshotQueryResult;
	FSkillRuleSnapshotQueryResult SkillRuleQueryResult;
	bool bAttackEnded = false;
	bool bIsGoal = false;
};

class FMCODEX_API FCutInsideShotDeadCornerDecisionQuery final
{
public:
	static FCutInsideShotDeadCornerDecisionQueryResult Evaluate(
		const FPlayerCardRuleSnapshotSet& PlayerCardSnapshots,
		const FSkillRuleSnapshotSet& SkillRules,
		const FCutInsideShotDeadCornerDecisionQueryInput& Input);
};
