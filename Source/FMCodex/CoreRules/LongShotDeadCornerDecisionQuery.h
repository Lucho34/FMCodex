#pragma once

#include "CoreMinimal.h"

#include "PlayerCardRuleSnapshotQuery.h"
#include "SkillRuleSnapshotQuery.h"

enum class ELongShotDeadCornerDecision : uint8
{
	None,
	Goal,
	Miss
};

enum class ELongShotDeadCornerDecisionQueryErrorCode : uint8
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

struct FMCODEX_API FLongShotDeadCornerDecisionQueryInput
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

struct FMCODEX_API FLongShotDeadCornerDecisionQueryResult
{
	bool bSuccess = false;
	ELongShotDeadCornerDecision Decision =
		ELongShotDeadCornerDecision::None;
	ELongShotDeadCornerDecisionQueryErrorCode ErrorCode =
		ELongShotDeadCornerDecisionQueryErrorCode::None;
	FString ErrorMessage;
	FName InvalidField = NAME_None;
	FLongShotDeadCornerDecisionQueryInput Input;
	FPlayerCardRuleSnapshotQueryResult AttackerSnapshotQueryResult;
	FSkillRuleSnapshotQueryResult SkillRuleQueryResult;
	bool bAttackEnded = false;
	bool bIsGoal = false;
};

class FMCODEX_API FLongShotDeadCornerDecisionQuery final
{
public:
	static FLongShotDeadCornerDecisionQueryResult Evaluate(
		const FPlayerCardRuleSnapshotSet& PlayerCardSnapshots,
		const FSkillRuleSnapshotSet& SkillRules,
		const FLongShotDeadCornerDecisionQueryInput& Input);
};
