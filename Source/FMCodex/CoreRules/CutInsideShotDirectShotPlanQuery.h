#pragma once

#include "CoreMinimal.h"

#include "PlayerCardRuleSnapshotQuery.h"
#include "SingleCardFormulaInputAssemblyQuery.h"
#include "SkillRuleSnapshotQuery.h"

enum class ECutInsideShotDirectShotDecision : uint8
{
	None,
	ImmediateMiss,
	FormulaResolutionRequired
};

enum class ECutInsideShotDirectShotPlanQueryErrorCode : uint8
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

struct FMCODEX_API FCutInsideShotDirectShotPlanQueryInput
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

struct FMCODEX_API FCutInsideShotDirectShotFormulaPlan
{
	FSingleCardFormulaInputAssemblyQueryInput AttackerQueryInput;
	FSingleCardFormulaInputAssemblyQueryInput DefenderQueryInput;
	FName AttackerPlayerId = NAME_None;
	FName DefenderPlayerId = NAME_None;
};

struct FMCODEX_API FCutInsideShotDirectShotPlanQueryResult
{
	bool bSuccess = false;
	ECutInsideShotDirectShotDecision Decision =
		ECutInsideShotDirectShotDecision::None;
	ECutInsideShotDirectShotPlanQueryErrorCode ErrorCode =
		ECutInsideShotDirectShotPlanQueryErrorCode::None;
	FString ErrorMessage;
	FName InvalidField = NAME_None;
	FCutInsideShotDirectShotPlanQueryInput Input;
	FPlayerCardRuleSnapshotQueryResult AttackerSnapshotQueryResult;
	FPlayerCardRuleSnapshotQueryResult DefenderSnapshotQueryResult;
	FSkillRuleSnapshotQueryResult SkillRuleQueryResult;
	bool bAttackEnded = false;
	bool bIsGoal = false;
	bool bHasFormulaPlan = false;
	FCutInsideShotDirectShotFormulaPlan FormulaPlan;
};

class FMCODEX_API FCutInsideShotDirectShotPlanQuery final
{
public:
	static FCutInsideShotDirectShotPlanQueryResult BuildPlan(
		const FPlayerCardRuleSnapshotSet& PlayerCardSnapshots,
		const FSkillRuleSnapshotSet& SkillRules,
		const FCutInsideShotDirectShotPlanQueryInput& Input);
};
