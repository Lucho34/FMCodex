#pragma once

#include "CoreMinimal.h"

#include "PlayerCardRuleSnapshotQuery.h"
#include "SingleCardFormulaInputAssemblyQuery.h"
#include "SkillRuleSnapshotQuery.h"

enum class ECrossPlanActualType : uint8
{
	None,
	High,
	Low
};

enum class ECrossPlanQueryDecision : uint8
{
	None,
	FormulaResolutionRequired
};

enum class ECrossPlanQueryErrorCode : uint8
{
	None,
	MissingSkillId,
	InvalidActualCrossType,
	MissingCarrierIdentity,
	MissingRunnerIdentity,
	MissingMarkerIdentity,
	MissingHelperIdentity,
	UnexpectedHelperIdentity,
	MissingGoalkeeperIdentity,
	UnexpectedGoalkeeperIdentity,
	CarrierSnapshotQueryFailed,
	RunnerSnapshotQueryFailed,
	MarkerSnapshotQueryFailed,
	HelperSnapshotQueryFailed,
	GoalkeeperSnapshotQueryFailed,
	SkillRuleQueryFailed,
	SkillRuleTypeMismatch,
	CarrierDoesNotOwnSkill,
	UnsupportedGoalkeeperCarrier,
	UnsupportedGoalkeeperRunner,
	RunnerNotForward,
	CarrierRunnerIdentityConflict,
	UnsupportedGoalkeeperMarker,
	UnsupportedGoalkeeperHelper,
	MarkerHelperIdentityConflict,
	GoalkeeperSnapshotNotGoalkeeper,
	MarkerGoalkeeperIdentityConflict,
	HelperGoalkeeperIdentityConflict,
	InvalidCurrentActionPoint,
	ActionPointOutOfRange,
	InvalidAttackD6,
	InvalidDefenseD6,
	InvalidLogContext
};

struct FMCODEX_API FCrossPlanQueryInput
{
	FName SkillId = NAME_None;
	ECrossPlanActualType ActualCrossType = ECrossPlanActualType::None;
	FName CarrierCardId = NAME_None;
	FName CarrierPlayerId = NAME_None;
	FName RunnerCardId = NAME_None;
	FName RunnerPlayerId = NAME_None;
	FName MarkerCardId = NAME_None;
	FName MarkerPlayerId = NAME_None;
	bool bHasHelper = false;
	FName HelperCardId = NAME_None;
	FName HelperPlayerId = NAME_None;
	bool bUseGoalkeeper = false;
	FName GoalkeeperCardId = NAME_None;
	FName GoalkeeperPlayerId = NAME_None;
	int32 CurrentActionPoint = 0;
	int32 AttackD6 = 0;
	int32 DefenseD6 = 0;
	FGuid LogId;
	int32 TurnIndex = 0;
};

struct FMCODEX_API FCrossFormulaPlan
{
	ECrossPlanActualType ActualCrossType = ECrossPlanActualType::None;
	FSingleCardFormulaInputAssemblyQueryInput AttackerQueryInput;
	FSingleCardFormulaInputAssemblyQueryInput DefenderQueryInput;
	FName CarrierCardId = NAME_None;
	FName CarrierPlayerId = NAME_None;
	FName RunnerCardId = NAME_None;
	FName RunnerPlayerId = NAME_None;
	FName MarkerCardId = NAME_None;
	FName MarkerPlayerId = NAME_None;
	bool bHasHelper = false;
	FName HelperCardId = NAME_None;
	FName HelperPlayerId = NAME_None;
	bool bUseGoalkeeper = false;
	FName GoalkeeperCardId = NAME_None;
	FName GoalkeeperPlayerId = NAME_None;
	FName GoalScorerCardId = NAME_None;
	FName GoalScorerPlayerId = NAME_None;
};

struct FMCODEX_API FCrossPlanQueryResult
{
	bool bSuccess = false;
	ECrossPlanQueryDecision Decision = ECrossPlanQueryDecision::None;
	ECrossPlanQueryErrorCode ErrorCode = ECrossPlanQueryErrorCode::None;
	FString ErrorMessage;
	FName InvalidField = NAME_None;
	FCrossPlanQueryInput Input;
	bool bHasHelper = false;
	bool bUseGoalkeeper = false;
	FPlayerCardRuleSnapshotQueryResult CarrierSnapshotQueryResult;
	FPlayerCardRuleSnapshotQueryResult RunnerSnapshotQueryResult;
	FPlayerCardRuleSnapshotQueryResult MarkerSnapshotQueryResult;
	FPlayerCardRuleSnapshotQueryResult HelperSnapshotQueryResult;
	FPlayerCardRuleSnapshotQueryResult GoalkeeperSnapshotQueryResult;
	FSkillRuleSnapshotQueryResult SkillRuleQueryResult;
	bool bHasFormulaPlan = false;
	FCrossFormulaPlan FormulaPlan;
};

class FMCODEX_API FCrossPlanQuery final
{
public:
	static FCrossPlanQueryResult BuildPlan(
		const FPlayerCardRuleSnapshotSet& PlayerCardSnapshots,
		const FSkillRuleSnapshotSet& SkillRules,
		const FCrossPlanQueryInput& Input);
};
