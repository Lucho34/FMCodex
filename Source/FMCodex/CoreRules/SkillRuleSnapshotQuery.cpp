#include "SkillRuleSnapshotQuery.h"

namespace SkillRuleSnapshotQuery
{
	void SetFailure(
		FSkillRuleSnapshotQueryResult& Result,
		const ESkillRuleSnapshotQueryErrorCode ErrorCode,
		const FString& ErrorMessage,
		const FName InvalidSkillId,
		const FName InvalidField)
	{
		Result.ErrorCode = ErrorCode;
		Result.ErrorMessage = ErrorMessage;
		Result.InvalidSkillId = InvalidSkillId;
		Result.InvalidField = InvalidField;
	}
}

FSkillRuleSnapshotQueryResult
FSkillRuleSnapshotQuery::FindBySkillId(
	const FSkillRuleSnapshotSet& SnapshotSet,
	const FSkillRuleSnapshotQueryInput& Input)
{
	FSkillRuleSnapshotQueryResult Result;

	if (Input.SkillId.IsNone())
	{
		SkillRuleSnapshotQuery::SetFailure(
			Result,
			ESkillRuleSnapshotQueryErrorCode::InvalidSkillId,
			TEXT("SkillId must not be None."),
			Input.SkillId,
			TEXT("SkillId"));
		return Result;
	}

	Result.ValidationResult =
		FSkillRuleSnapshotValidator::Validate(SnapshotSet);
	if (!Result.ValidationResult.bSuccess)
	{
		SkillRuleSnapshotQuery::SetFailure(
			Result,
			ESkillRuleSnapshotQueryErrorCode
				::SnapshotSetValidationFailed,
			Result.ValidationResult.ErrorMessage,
			Result.ValidationResult.InvalidSkillId,
			Result.ValidationResult.InvalidField);
		return Result;
	}

	for (const FSkillRuleSnapshot& Snapshot
		: SnapshotSet.SkillRules)
	{
		if (Snapshot.SkillId == Input.SkillId)
		{
			Result.bSuccess = true;
			Result.bFound = true;
			Result.Snapshot = Snapshot;
			return Result;
		}
	}

	SkillRuleSnapshotQuery::SetFailure(
		Result,
		ESkillRuleSnapshotQueryErrorCode::SkillRuleNotFound,
		FString::Printf(
			TEXT("SkillId '%s' was not found in the skill rule snapshot set."),
			*Input.SkillId.ToString()),
		Input.SkillId,
		TEXT("SkillId"));
	return Result;
}
