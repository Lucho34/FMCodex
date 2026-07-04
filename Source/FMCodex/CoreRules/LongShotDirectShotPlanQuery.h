#pragma once

#include "CoreMinimal.h"

#include "PlayerCardRuleSnapshotQuery.h"
#include "SingleCardFormulaInputAssemblyQuery.h"
#include "SkillRuleSnapshotQuery.h"

enum class ELongShotDirectShotDecision : uint8
{
	None,
	ImmediateMiss,
	FormulaResolutionRequired
};

enum class ELongShotDirectShotPlanQueryErrorCode : uint8
{
	None,
	MissingSkillId,
	AttackerSnapshotQueryFailed,
	DefenderSnapshotQueryFailed,
	SkillRuleQueryFailed,
	SkillNotOwnedByAttacker,
	SkillTypeMismatch,
	InvalidActionPoint,
	ActionPointOutOfRange,
	UnsupportedGoalkeeperParticipant,
	MissingExternalAttackD6,
	InvalidAttackD6,
	MissingExternalDefenseD6,
	InvalidDefenseD6,
	InvalidLogContext
};

struct FMCODEX_API FLongShotDirectShotPlanQueryInput
{
	FName SkillId = NAME_None;
	FName AttackerCardId = NAME_None;
	FName DefenderCardId = NAME_None;
	int32 CurrentActionPoint = 0;
	bool bHasExternalAttackD6 = false;
	int32 ExternalAttackD6 = 0;
	bool bHasExternalDefenseD6 = false;
	int32 ExternalDefenseD6 = 0;
	FGuid LogId;
	int32 TurnIndex = 0;
	FName AttackerPlayerId = NAME_None;
	FName DefenderPlayerId = NAME_None;
};

struct FMCODEX_API FLongShotDirectShotFormulaPlan
{
	FSingleCardFormulaInputAssemblyQueryInput AttackerQueryInput;
	FSingleCardFormulaInputAssemblyQueryInput DefenderQueryInput;
	FName AttackerPlayerId = NAME_None;
	FName DefenderPlayerId = NAME_None;
};

struct FMCODEX_API FLongShotDirectShotPlanQueryResult
{
	bool bSuccess = false;
	ELongShotDirectShotDecision Decision =
		ELongShotDirectShotDecision::None;
	ELongShotDirectShotPlanQueryErrorCode ErrorCode =
		ELongShotDirectShotPlanQueryErrorCode::None;
	FString ErrorMessage;
	FName InvalidField = NAME_None;
	FLongShotDirectShotPlanQueryInput Input;
	FPlayerCardRuleSnapshotQueryResult AttackerSnapshotQueryResult;
	FPlayerCardRuleSnapshotQueryResult DefenderSnapshotQueryResult;
	FSkillRuleSnapshotQueryResult SkillRuleQueryResult;
	bool bAttackEnded = false;
	bool bIsGoal = false;
	bool bHasFormulaPlan = false;
	FLongShotDirectShotFormulaPlan FormulaPlan;
};

class FMCODEX_API FLongShotDirectShotPlanQuery final
{
public:
	static FLongShotDirectShotPlanQueryResult BuildPlan(
		const FPlayerCardRuleSnapshotSet& PlayerCardSnapshots,
		const FSkillRuleSnapshotSet& SkillRules,
		const FLongShotDirectShotPlanQueryInput& Input);
};
