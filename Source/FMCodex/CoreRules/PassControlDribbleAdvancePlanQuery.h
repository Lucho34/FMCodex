#pragma once

#include "CoreMinimal.h"

#include "PassControlAdvanceSelectionQuery.h"
#include "PlayerCardRuleSnapshotQuery.h"
#include "SingleCardFormulaInputAssemblyQuery.h"
#include "SkillRuleSnapshotQuery.h"

enum class EPassControlDribbleAdvancePlanDecision : uint8
{
	None,
	FormulaResolutionRequired
};

enum class EPassControlDribbleAdvancePlanQueryErrorCode : uint8
{
	None,
	MissingSkillId,
	InvalidAdvanceType,
	MissingRequiredIdentity,
	CarrierSnapshotQueryFailed,
	RunnerSnapshotQueryFailed,
	MarkerSnapshotQueryFailed,
	MissingHelperIdentity,
	UnexpectedHelperIdentity,
	HelperSnapshotQueryFailed,
	SkillRuleQueryFailed,
	CarrierMissingSkill,
	SkillRuleTypeMismatch,
	InvalidActionPoint,
	ActionPointOutOfRange,
	RunnerNotMidfield,
	MissingExternalAttackD6,
	InvalidAttackD6,
	MissingExternalDefenseD6,
	InvalidDefenseD6,
	InvalidLogContext,
	UnsupportedGoalkeeperParticipant
};

struct FMCODEX_API FPassControlDribbleAdvancePlanQueryInput
{
	FName SkillId = NAME_None;
	EPassControlAdvanceType AdvanceType = EPassControlAdvanceType::None;
	FName CarrierCardId = NAME_None;
	FName RunnerCardId = NAME_None;
	FName MarkerCardId = NAME_None;
	bool bHasHelper = false;
	FName HelperCardId = NAME_None;
	int32 CurrentActionPoint = 0;
	bool bHasExternalAttackD6 = false;
	int32 ExternalAttackD6 = 0;
	bool bHasExternalDefenseD6 = false;
	int32 ExternalDefenseD6 = 0;
	FGuid LogId;
	int32 TurnIndex = 0;
	FName CarrierPlayerId = NAME_None;
	FName RunnerPlayerId = NAME_None;
	FName MarkerPlayerId = NAME_None;
	FName HelperPlayerId = NAME_None;
};

struct FMCODEX_API FPassControlDribbleAdvanceFormulaPlan
{
	FSingleCardFormulaInputAssemblyQueryInput AttackerQueryInput;
	FSingleCardFormulaInputAssemblyQueryInput DefenderQueryInput;
	FName CarrierPlayerId = NAME_None;
	FName RunnerPlayerId = NAME_None;
	FName MarkerPlayerId = NAME_None;
	FName HelperPlayerId = NAME_None;
	FName RunnerCardId = NAME_None;
	FName HelperCardId = NAME_None;
};

struct FMCODEX_API FPassControlDribbleAdvancePlanQueryResult
{
	bool bSuccess = false;
	EPassControlDribbleAdvancePlanDecision Decision =
		EPassControlDribbleAdvancePlanDecision::None;
	EPassControlDribbleAdvancePlanQueryErrorCode ErrorCode =
		EPassControlDribbleAdvancePlanQueryErrorCode::None;
	FString ErrorMessage;
	FName InvalidField = NAME_None;
	FPassControlDribbleAdvancePlanQueryInput Input;
	bool bHasHelper = false;
	FPlayerCardRuleSnapshotQueryResult CarrierSnapshotQueryResult;
	FPlayerCardRuleSnapshotQueryResult RunnerSnapshotQueryResult;
	FPlayerCardRuleSnapshotQueryResult MarkerSnapshotQueryResult;
	FPlayerCardRuleSnapshotQueryResult HelperSnapshotQueryResult;
	FSkillRuleSnapshotQueryResult SkillRuleQueryResult;
	bool bHasFormulaPlan = false;
	FPassControlDribbleAdvanceFormulaPlan FormulaPlan;
};

class FMCODEX_API FPassControlDribbleAdvancePlanQuery final
{
public:
	static FPassControlDribbleAdvancePlanQueryResult BuildPlan(
		const FPlayerCardRuleSnapshotSet& PlayerCardSnapshots,
		const FSkillRuleSnapshotSet& SkillRules,
		const FPassControlDribbleAdvancePlanQueryInput& Input);
};
