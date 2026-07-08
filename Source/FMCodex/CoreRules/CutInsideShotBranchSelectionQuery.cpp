#include "CutInsideShotBranchSelectionQuery.h"

namespace CutInsideShotBranchSelectionQuery
{
	void SetFailure(
		FCutInsideShotBranchSelectionQueryResult& Result,
		const ECutInsideShotBranchSelectionQueryErrorCode ErrorCode,
		const FString& ErrorMessage,
		const FName InvalidField)
	{
		Result.ErrorCode = ErrorCode;
		Result.ErrorMessage = ErrorMessage;
		Result.InvalidField = InvalidField;
	}
}

FCutInsideShotBranchSelectionQueryResult
FCutInsideShotBranchSelectionQuery::Select(
	const FPlayerCardRuleSnapshotSet& PlayerCardSnapshots,
	const FSkillRuleSnapshotSet& SkillRules,
	const FCutInsideShotBranchSelectionQueryInput& Input)
{
	FCutInsideShotBranchSelectionQueryResult Result;
	Result.SelectedBranch = Input.Branch;
	Result.Input = Input;

	switch (Input.Branch)
	{
	case ECutInsideShotBranch::DirectShot:
		Result.DirectShotResult =
			FCutInsideShotDirectShotPlanQuery::BuildPlan(
				PlayerCardSnapshots,
				SkillRules,
				Input.DirectShotInput);
		if (!Result.DirectShotResult.bSuccess)
		{
			CutInsideShotBranchSelectionQuery::SetFailure(
				Result,
				ECutInsideShotBranchSelectionQueryErrorCode
					::DirectShotQueryFailed,
				Result.DirectShotResult.ErrorMessage,
				Result.DirectShotResult.InvalidField);
			return Result;
		}

		if (Result.DirectShotResult.Decision
			== ECutInsideShotDirectShotDecision::ImmediateMiss)
		{
			Result.bSuccess = true;
			Result.Outcome =
				ECutInsideShotBranchSelectionOutcome
					::DirectShotImmediateMiss;
			Result.bAttackEnded =
				Result.DirectShotResult.bAttackEnded;
			Result.bIsGoal = Result.DirectShotResult.bIsGoal;
			return Result;
		}

		if (Result.DirectShotResult.Decision
			== ECutInsideShotDirectShotDecision
				::FormulaResolutionRequired)
		{
			Result.bSuccess = true;
			Result.Outcome =
				ECutInsideShotBranchSelectionOutcome
					::DirectShotFormulaPlanRequired;
			Result.bAttackEnded =
				Result.DirectShotResult.bAttackEnded;
			Result.bIsGoal = Result.DirectShotResult.bIsGoal;
			return Result;
		}

		CutInsideShotBranchSelectionQuery::SetFailure(
			Result,
			ECutInsideShotBranchSelectionQueryErrorCode
				::DirectShotQueryFailed,
			TEXT("The Direct Shot query returned an unsupported decision."),
			TEXT("Decision"));
		return Result;

	case ECutInsideShotBranch::DeadCorner:
		Result.DeadCornerResult =
			FCutInsideShotDeadCornerDecisionQuery::Evaluate(
				PlayerCardSnapshots,
				SkillRules,
				Input.DeadCornerInput);
		if (!Result.DeadCornerResult.bSuccess)
		{
			CutInsideShotBranchSelectionQuery::SetFailure(
				Result,
				ECutInsideShotBranchSelectionQueryErrorCode
					::DeadCornerQueryFailed,
				Result.DeadCornerResult.ErrorMessage,
				Result.DeadCornerResult.InvalidField);
			return Result;
		}

		if (Result.DeadCornerResult.Decision
			== ECutInsideShotDeadCornerDecision::Goal)
		{
			Result.bSuccess = true;
			Result.Outcome =
				ECutInsideShotBranchSelectionOutcome::DeadCornerGoal;
			Result.bAttackEnded =
				Result.DeadCornerResult.bAttackEnded;
			Result.bIsGoal = Result.DeadCornerResult.bIsGoal;
			return Result;
		}

		if (Result.DeadCornerResult.Decision
			== ECutInsideShotDeadCornerDecision::Miss)
		{
			Result.bSuccess = true;
			Result.Outcome =
				ECutInsideShotBranchSelectionOutcome::DeadCornerMiss;
			Result.bAttackEnded =
				Result.DeadCornerResult.bAttackEnded;
			Result.bIsGoal = Result.DeadCornerResult.bIsGoal;
			return Result;
		}

		CutInsideShotBranchSelectionQuery::SetFailure(
			Result,
			ECutInsideShotBranchSelectionQueryErrorCode
				::DeadCornerQueryFailed,
			TEXT("The Dead Corner query returned an unsupported decision."),
			TEXT("Decision"));
		return Result;

	case ECutInsideShotBranch::None:
	default:
		CutInsideShotBranchSelectionQuery::SetFailure(
			Result,
			ECutInsideShotBranchSelectionQueryErrorCode::InvalidBranch,
			TEXT("Cut Inside Shot Branch Selection requires DirectShot or DeadCorner."),
			TEXT("Branch"));
		return Result;
	}
}
