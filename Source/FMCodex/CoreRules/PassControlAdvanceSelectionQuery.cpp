#include "PassControlAdvanceSelectionQuery.h"

namespace PassControlAdvanceSelectionQuery
{
	constexpr int32 MinD6 = 1;
	constexpr int32 MaxD6 = 6;

	void SetFailure(
		FPassControlAdvanceSelectionQueryResult& Result,
		const EPassControlAdvanceSelectionQueryErrorCode ErrorCode,
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

	EPassControlAdvanceType MapD6ToAdvanceType(const int32 D6)
	{
		if (D6 <= 2)
		{
			return EPassControlAdvanceType::PassAdvance;
		}
		if (D6 <= 4)
		{
			return EPassControlAdvanceType::DribbleAdvance;
		}
		return EPassControlAdvanceType::RunAdvance;
	}
}

FPassControlAdvanceSelectionQueryResult
FPassControlAdvanceSelectionQuery::Select(
	const FPlayerCardRuleSnapshotSet& PlayerCardSnapshots,
	const FSkillRuleSnapshotSet& SkillRules,
	const FPassControlAdvanceSelectionQueryInput& Input)
{
	FPassControlAdvanceSelectionQueryResult Result;
	Result.Input = Input;

	if (Input.SkillId.IsNone())
	{
		PassControlAdvanceSelectionQuery::SetFailure(
			Result,
			EPassControlAdvanceSelectionQueryErrorCode::MissingSkillId,
			TEXT("Pass Control Advance Selection requires a non-empty SkillId."),
			TEXT("SkillId"));
		return Result;
	}

	Result.CarrierSnapshotQueryResult =
		FPlayerCardRuleSnapshotQuery::FindByCardId(
			PlayerCardSnapshots,
			Input.CarrierCardId);
	if (!Result.CarrierSnapshotQueryResult.bSuccess)
	{
		PassControlAdvanceSelectionQuery::SetFailure(
			Result,
			EPassControlAdvanceSelectionQueryErrorCode
				::CarrierSnapshotQueryFailed,
			Result.CarrierSnapshotQueryResult.ErrorMessage,
			TEXT("CarrierCardId"));
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
		PassControlAdvanceSelectionQuery::SetFailure(
			Result,
			bRequestedSkillTypeFailure
				? EPassControlAdvanceSelectionQueryErrorCode
					::SkillTypeMismatch
				: EPassControlAdvanceSelectionQueryErrorCode
					::SkillRuleQueryFailed,
			Result.SkillRuleQueryResult.ErrorMessage,
			bRequestedSkillTypeFailure
				? FName(TEXT("SkillType"))
				: FName(TEXT("SkillId")));
		return Result;
	}

	const FPlayerCardRuleSnapshot& CarrierSnapshot =
		Result.CarrierSnapshotQueryResult.Snapshot;
	if (!CarrierSnapshot.SkillIds.Contains(Input.SkillId))
	{
		PassControlAdvanceSelectionQuery::SetFailure(
			Result,
			EPassControlAdvanceSelectionQueryErrorCode
				::SkillNotOwnedByCarrier,
			TEXT("The carrier snapshot does not own the requested SkillId."),
			TEXT("SkillId"));
		return Result;
	}

	if (Result.SkillRuleQueryResult.Snapshot.SkillType
		!= ESkillRuleType::PassControl)
	{
		PassControlAdvanceSelectionQuery::SetFailure(
			Result,
			EPassControlAdvanceSelectionQueryErrorCode::SkillTypeMismatch,
			TEXT("The requested skill rule must use PassControl SkillType."),
			TEXT("SkillType"));
		return Result;
	}

	if (CarrierSnapshot.bIsGoalkeeper)
	{
		PassControlAdvanceSelectionQuery::SetFailure(
			Result,
			EPassControlAdvanceSelectionQueryErrorCode
				::UnsupportedGoalkeeperParticipant,
			TEXT("Pass Control Advance Selection does not support a goalkeeper carrier."),
			TEXT("CarrierCardId"));
		return Result;
	}

	if (Input.CurrentActionPoint
			< FSkillRuleSnapshotValidator::MinTriggerActionPoint
		|| Input.CurrentActionPoint
			> FSkillRuleSnapshotValidator::MaxTriggerActionPoint)
	{
		PassControlAdvanceSelectionQuery::SetFailure(
			Result,
			EPassControlAdvanceSelectionQueryErrorCode
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
		PassControlAdvanceSelectionQuery::SetFailure(
			Result,
			EPassControlAdvanceSelectionQueryErrorCode
				::ActionPointOutOfRange,
			TEXT("CurrentActionPoint is outside the skill trigger range."),
			TEXT("CurrentActionPoint"));
		return Result;
	}

	if (!Input.bHasExternalAdvanceD6)
	{
		PassControlAdvanceSelectionQuery::SetFailure(
			Result,
			EPassControlAdvanceSelectionQueryErrorCode
				::MissingExternalAdvanceD6,
			TEXT("An externally supplied advance D6 is required."),
			TEXT("ExternalAdvanceD6"));
		return Result;
	}

	if (!PassControlAdvanceSelectionQuery::IsD6InRange(
			Input.ExternalAdvanceD6))
	{
		PassControlAdvanceSelectionQuery::SetFailure(
			Result,
			EPassControlAdvanceSelectionQueryErrorCode
				::InvalidAdvanceD6,
			TEXT("ExternalAdvanceD6 must be in range [1, 6]."),
			TEXT("ExternalAdvanceD6"));
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

		PassControlAdvanceSelectionQuery::SetFailure(
			Result,
			EPassControlAdvanceSelectionQueryErrorCode
				::InvalidLogContext,
			TEXT("LogId, TurnIndex, and AttackerPlayerId must form valid log context."),
			InvalidField);
		return Result;
	}

	Result.bSuccess = true;
	Result.AdvanceType =
		PassControlAdvanceSelectionQuery::MapD6ToAdvanceType(
			Input.ExternalAdvanceD6);
	return Result;
}
