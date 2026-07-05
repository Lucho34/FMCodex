#include "LongShotBranchSelectionQuery.h"

namespace LongShotBranchSelectionQuery
{
	void SetFailure(
		FLongShotBranchSelectionQueryResult& Result,
		const ELongShotBranchSelectionQueryErrorCode ErrorCode,
		const FString& ErrorMessage,
		const FName InvalidField)
	{
		Result.ErrorCode = ErrorCode;
		Result.ErrorMessage = ErrorMessage;
		Result.InvalidField = InvalidField;
	}
}

FLongShotBranchSelectionQueryResult
FLongShotBranchSelectionQuery::Select(
	const FPlayerCardRuleSnapshotSet& PlayerCardSnapshots,
	const FSkillRuleSnapshotSet& SkillRules,
	const FLongShotBranchSelectionQueryInput& Input)
{
	FLongShotBranchSelectionQueryResult Result;
	Result.SelectedBranch = Input.Branch;
	Result.Input = Input;

	switch (Input.Branch)
	{
	case ELongShotShotBranch::DirectShot:
		Result.DirectShotResult =
			FLongShotDirectShotPlanQuery::BuildPlan(
				PlayerCardSnapshots,
				SkillRules,
				Input.DirectShotInput);
		if (!Result.DirectShotResult.bSuccess)
		{
			LongShotBranchSelectionQuery::SetFailure(
				Result,
				ELongShotBranchSelectionQueryErrorCode
					::DirectShotQueryFailed,
				Result.DirectShotResult.ErrorMessage,
				Result.DirectShotResult.InvalidField);
			return Result;
		}

		if (Result.DirectShotResult.Decision
			== ELongShotDirectShotDecision::ImmediateMiss)
		{
			Result.bSuccess = true;
			Result.Outcome =
				ELongShotBranchSelectionOutcome
					::DirectShotImmediateMiss;
			Result.bAttackEnded =
				Result.DirectShotResult.bAttackEnded;
			Result.bIsGoal = Result.DirectShotResult.bIsGoal;
			return Result;
		}

		if (Result.DirectShotResult.Decision
			== ELongShotDirectShotDecision
				::FormulaResolutionRequired)
		{
			Result.bSuccess = true;
			Result.Outcome =
				ELongShotBranchSelectionOutcome
					::DirectShotFormulaPlanRequired;
			Result.bAttackEnded =
				Result.DirectShotResult.bAttackEnded;
			Result.bIsGoal = Result.DirectShotResult.bIsGoal;
			return Result;
		}

		LongShotBranchSelectionQuery::SetFailure(
			Result,
			ELongShotBranchSelectionQueryErrorCode
				::DirectShotQueryFailed,
			TEXT("The Direct Shot query returned an unsupported decision."),
			TEXT("Decision"));
		return Result;

	case ELongShotShotBranch::DeadCorner:
		Result.DeadCornerResult =
			FLongShotDeadCornerDecisionQuery::Evaluate(
				PlayerCardSnapshots,
				SkillRules,
				Input.DeadCornerInput);
		if (!Result.DeadCornerResult.bSuccess)
		{
			LongShotBranchSelectionQuery::SetFailure(
				Result,
				ELongShotBranchSelectionQueryErrorCode
					::DeadCornerQueryFailed,
				Result.DeadCornerResult.ErrorMessage,
				Result.DeadCornerResult.InvalidField);
			return Result;
		}

		if (Result.DeadCornerResult.Decision
			== ELongShotDeadCornerDecision::Goal)
		{
			Result.bSuccess = true;
			Result.Outcome =
				ELongShotBranchSelectionOutcome::DeadCornerGoal;
			Result.bAttackEnded =
				Result.DeadCornerResult.bAttackEnded;
			Result.bIsGoal = Result.DeadCornerResult.bIsGoal;
			return Result;
		}

		if (Result.DeadCornerResult.Decision
			== ELongShotDeadCornerDecision::Miss)
		{
			Result.bSuccess = true;
			Result.Outcome =
				ELongShotBranchSelectionOutcome::DeadCornerMiss;
			Result.bAttackEnded =
				Result.DeadCornerResult.bAttackEnded;
			Result.bIsGoal = Result.DeadCornerResult.bIsGoal;
			return Result;
		}

		LongShotBranchSelectionQuery::SetFailure(
			Result,
			ELongShotBranchSelectionQueryErrorCode
				::DeadCornerQueryFailed,
			TEXT("The Dead Corner query returned an unsupported decision."),
			TEXT("Decision"));
		return Result;

	case ELongShotShotBranch::None:
	default:
		LongShotBranchSelectionQuery::SetFailure(
			Result,
			ELongShotBranchSelectionQueryErrorCode::InvalidBranch,
			TEXT("Long Shot Branch Selection requires DirectShot or DeadCorner."),
			TEXT("Branch"));
		return Result;
	}
}
