#pragma once

#include "CoreMinimal.h"

#include "PlayerCardRuleSnapshotQuery.h"
#include "SkillRuleSnapshotQuery.h"

enum class EPassControlAdvanceType : uint8
{
	None,
	PassAdvance,
	DribbleAdvance,
	RunAdvance
};

enum class EPassControlAdvanceSelectionQueryErrorCode : uint8
{
	None,
	MissingSkillId,
	CarrierSnapshotQueryFailed,
	SkillRuleQueryFailed,
	SkillNotOwnedByCarrier,
	SkillTypeMismatch,
	InvalidActionPoint,
	ActionPointOutOfRange,
	UnsupportedGoalkeeperParticipant,
	MissingExternalAdvanceD6,
	InvalidAdvanceD6,
	InvalidLogContext
};

struct FMCODEX_API FPassControlAdvanceSelectionQueryInput
{
	FName SkillId = NAME_None;
	FName CarrierCardId = NAME_None;
	int32 CurrentActionPoint = 0;
	bool bHasExternalAdvanceD6 = false;
	int32 ExternalAdvanceD6 = 0;
	FGuid LogId;
	int32 TurnIndex = 0;
	FName AttackerPlayerId = NAME_None;
};

struct FMCODEX_API FPassControlAdvanceSelectionQueryResult
{
	bool bSuccess = false;
	EPassControlAdvanceType AdvanceType =
		EPassControlAdvanceType::None;
	EPassControlAdvanceSelectionQueryErrorCode ErrorCode =
		EPassControlAdvanceSelectionQueryErrorCode::None;
	FString ErrorMessage;
	FName InvalidField = NAME_None;
	FPassControlAdvanceSelectionQueryInput Input;
	FPlayerCardRuleSnapshotQueryResult CarrierSnapshotQueryResult;
	FSkillRuleSnapshotQueryResult SkillRuleQueryResult;
};

class FMCODEX_API FPassControlAdvanceSelectionQuery final
{
public:
	static FPassControlAdvanceSelectionQueryResult Select(
		const FPlayerCardRuleSnapshotSet& PlayerCardSnapshots,
		const FSkillRuleSnapshotSet& SkillRules,
		const FPassControlAdvanceSelectionQueryInput& Input);
};
