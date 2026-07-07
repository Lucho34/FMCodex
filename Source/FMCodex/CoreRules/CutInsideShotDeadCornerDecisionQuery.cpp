#include "CutInsideShotDeadCornerDecisionQuery.h"

namespace CutInsideShotDeadCornerDecisionQuery
{
	constexpr int32 MinD6 = 1;
	constexpr int32 MaxD6 = 6;
	constexpr int32 MinGoalTotal = 11;

	void SetFailure(
		FCutInsideShotDeadCornerDecisionQueryResult& Result,
		const ECutInsideShotDeadCornerDecisionQueryErrorCode ErrorCode,
		const FString& ErrorMessage,
		const FName InvalidField)
	{
		Result.ErrorCode = ErrorCode;
		Result.ErrorMessage = ErrorMessage;
		Result.InvalidField = InvalidField;
	}

	bool IsD6InRange(const int32 D6)
	{
		return D6 >= MinD6 && D6 <= MaxD6;
	}
}

FCutInsideShotDeadCornerDecisionQueryResult
FCutInsideShotDeadCornerDecisionQuery::Evaluate(
	const FPlayerCardRuleSnapshotSet& PlayerCardSnapshots,
	const FSkillRuleSnapshotSet& SkillRules,
	const FCutInsideShotDeadCornerDecisionQueryInput& Input)
{
	FCutInsideShotDeadCornerDecisionQueryResult Result;
	Result.Input = Input;

	if (Input.SkillId.IsNone())
	{
		CutInsideShotDeadCornerDecisionQuery::SetFailure(
			Result,
			ECutInsideShotDeadCornerDecisionQueryErrorCode
				::MissingSkillId,
			TEXT("Cut Inside Shot Dead Corner requires a non-empty SkillId."),
			TEXT("SkillId"));
		return Result;
	}

	Result.AttackerSnapshotQueryResult =
		FPlayerCardRuleSnapshotQuery::FindByCardId(
			PlayerCardSnapshots,
			Input.AttackerCardId);
	if (!Result.AttackerSnapshotQueryResult.bSuccess)
	{
		CutInsideShotDeadCornerDecisionQuery::SetFailure(
			Result,
			ECutInsideShotDeadCornerDecisionQueryErrorCode
				::AttackerSnapshotQueryFailed,
			Result.AttackerSnapshotQueryResult.ErrorMessage,
			TEXT("AttackerCardId"));
		return Result;
	}

	const FSkillRuleSnapshotQueryInput SkillQueryInput = {
		Input.SkillId
	};
	Result.SkillRuleQueryResult =
		FSkillRuleSnapshotQuery::FindBySkillId(
			SkillRules,
			SkillQueryInput);
	if (!Result.SkillRuleQueryResult.bSuccess)
	{
		const ESkillRuleSnapshotValidationErrorCode ValidationError =
			Result.SkillRuleQueryResult.ValidationResult.ErrorCode;
		const bool bRequestedSkillTypeFailure =
			Result.SkillRuleQueryResult.ValidationResult.InvalidSkillId
				== Input.SkillId
			&& (ValidationError
					== ESkillRuleSnapshotValidationErrorCode
						::InvalidSkillType
				|| ValidationError
					== ESkillRuleSnapshotValidationErrorCode
						::UnsupportedSkillType);
		CutInsideShotDeadCornerDecisionQuery::SetFailure(
			Result,
			bRequestedSkillTypeFailure
				? ECutInsideShotDeadCornerDecisionQueryErrorCode
					::SkillTypeMismatch
				: ECutInsideShotDeadCornerDecisionQueryErrorCode
					::SkillRuleQueryFailed,
			Result.SkillRuleQueryResult.ErrorMessage,
			bRequestedSkillTypeFailure
				? FName(TEXT("SkillType"))
				: FName(TEXT("SkillId")));
		return Result;
	}

	const FPlayerCardRuleSnapshot& AttackerSnapshot =
		Result.AttackerSnapshotQueryResult.Snapshot;
	if (!AttackerSnapshot.SkillIds.Contains(Input.SkillId))
	{
		CutInsideShotDeadCornerDecisionQuery::SetFailure(
			Result,
			ECutInsideShotDeadCornerDecisionQueryErrorCode
				::SkillNotOwnedByAttacker,
			TEXT("The attacker snapshot does not own the requested SkillId."),
			TEXT("SkillId"));
		return Result;
	}

	if (Result.SkillRuleQueryResult.Snapshot.SkillType
		!= ESkillRuleType::CutInsideShot)
	{
		CutInsideShotDeadCornerDecisionQuery::SetFailure(
			Result,
			ECutInsideShotDeadCornerDecisionQueryErrorCode
				::SkillTypeMismatch,
			TEXT("The requested skill rule must use CutInsideShot SkillType."),
			TEXT("SkillType"));
		return Result;
	}

	if (AttackerSnapshot.bIsGoalkeeper)
	{
		CutInsideShotDeadCornerDecisionQuery::SetFailure(
			Result,
			ECutInsideShotDeadCornerDecisionQueryErrorCode
				::UnsupportedGoalkeeperParticipant,
			TEXT("Cut Inside Shot Dead Corner does not support a goalkeeper attacker."),
			TEXT("AttackerCardId"));
		return Result;
	}

	if (Input.CurrentActionPoint
			< FSkillRuleSnapshotValidator::MinTriggerActionPoint
		|| Input.CurrentActionPoint
			> FSkillRuleSnapshotValidator::MaxTriggerActionPoint)
	{
		CutInsideShotDeadCornerDecisionQuery::SetFailure(
			Result,
			ECutInsideShotDeadCornerDecisionQueryErrorCode
				::InvalidActionPoint,
			TEXT("CurrentActionPoint must be in the supported range [2, 8]."),
			TEXT("CurrentActionPoint"));
		return Result;
	}

	const FSkillRuleSnapshot& SkillRule =
		Result.SkillRuleQueryResult.Snapshot;
	if (Input.CurrentActionPoint < SkillRule.MinTriggerActionPoint
		|| Input.CurrentActionPoint > SkillRule.MaxTriggerActionPoint)
	{
		CutInsideShotDeadCornerDecisionQuery::SetFailure(
			Result,
			ECutInsideShotDeadCornerDecisionQueryErrorCode
				::ActionPointOutOfRange,
			TEXT("CurrentActionPoint is outside the skill trigger range."),
			TEXT("CurrentActionPoint"));
		return Result;
	}

	if (!Input.bHasExternalAttackD6A)
	{
		CutInsideShotDeadCornerDecisionQuery::SetFailure(
			Result,
			ECutInsideShotDeadCornerDecisionQueryErrorCode
				::MissingExternalAttackD6A,
			TEXT("An externally supplied attack D6A is required."),
			TEXT("ExternalAttackD6A"));
		return Result;
	}

	if (!Input.bHasExternalAttackD6B)
	{
		CutInsideShotDeadCornerDecisionQuery::SetFailure(
			Result,
			ECutInsideShotDeadCornerDecisionQueryErrorCode
				::MissingExternalAttackD6B,
			TEXT("An externally supplied attack D6B is required."),
			TEXT("ExternalAttackD6B"));
		return Result;
	}

	if (!CutInsideShotDeadCornerDecisionQuery::IsD6InRange(
			Input.ExternalAttackD6A))
	{
		CutInsideShotDeadCornerDecisionQuery::SetFailure(
			Result,
			ECutInsideShotDeadCornerDecisionQueryErrorCode
				::InvalidAttackD6A,
			TEXT("ExternalAttackD6A must be in range [1, 6]."),
			TEXT("ExternalAttackD6A"));
		return Result;
	}

	if (!CutInsideShotDeadCornerDecisionQuery::IsD6InRange(
			Input.ExternalAttackD6B))
	{
		CutInsideShotDeadCornerDecisionQuery::SetFailure(
			Result,
			ECutInsideShotDeadCornerDecisionQueryErrorCode
				::InvalidAttackD6B,
			TEXT("ExternalAttackD6B must be in range [1, 6]."),
			TEXT("ExternalAttackD6B"));
		return Result;
	}

	if (!Input.LogId.IsValid()
		|| Input.TurnIndex < 0
		|| Input.AttackerPlayerId.IsNone())
	{
		FName InvalidField(TEXT("LogId"));
		if (Input.LogId.IsValid() && Input.TurnIndex < 0)
		{
			InvalidField = TEXT("TurnIndex");
		}
		else if (Input.LogId.IsValid()
			&& Input.TurnIndex >= 0
			&& Input.AttackerPlayerId.IsNone())
		{
			InvalidField = TEXT("AttackerPlayerId");
		}

		CutInsideShotDeadCornerDecisionQuery::SetFailure(
			Result,
			ECutInsideShotDeadCornerDecisionQueryErrorCode
				::InvalidLogContext,
			TEXT("LogId, TurnIndex, and AttackerPlayerId must form valid log context."),
			InvalidField);
		return Result;
	}

	const int32 D6Total =
		Input.ExternalAttackD6A + Input.ExternalAttackD6B;
	Result.bSuccess = true;
	Result.bAttackEnded = true;
	Result.bIsGoal =
		D6Total >= CutInsideShotDeadCornerDecisionQuery::MinGoalTotal;
	Result.Decision = Result.bIsGoal
		? ECutInsideShotDeadCornerDecision::Goal
		: ECutInsideShotDeadCornerDecision::Miss;
	return Result;
}
