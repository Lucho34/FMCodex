#include "SkillRuleSnapshotValidator.h"

namespace SkillRuleSnapshotValidator
{
	void SetFailure(
		FSkillRuleSnapshotValidationResult& Result,
		const ESkillRuleSnapshotValidationErrorCode ErrorCode,
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

FSkillRuleSnapshotValidationResult
FSkillRuleSnapshotValidator::Validate(
	const FSkillRuleSnapshotSet& SnapshotSet)
{
	FSkillRuleSnapshotValidationResult Result;
	TSet<FName> SeenSkillIds;

	for (const FSkillRuleSnapshot& SkillRule : SnapshotSet.SkillRules)
	{
		if (SkillRule.SkillId.IsNone())
		{
			SkillRuleSnapshotValidator::SetFailure(
				Result,
				ESkillRuleSnapshotValidationErrorCode::InvalidSkillId,
				TEXT("SkillId must not be None."),
				SkillRule.SkillId,
				TEXT("SkillId"));
			return Result;
		}

		if (SeenSkillIds.Contains(SkillRule.SkillId))
		{
			SkillRuleSnapshotValidator::SetFailure(
				Result,
				ESkillRuleSnapshotValidationErrorCode
					::DuplicateSkillId,
				FString::Printf(
					TEXT("SkillId '%s' must be unique in the skill rule snapshot set."),
					*SkillRule.SkillId.ToString()),
				SkillRule.SkillId,
				TEXT("SkillId"));
			return Result;
		}
		SeenSkillIds.Add(SkillRule.SkillId);

		if (SkillRule.SkillType == ESkillRuleType::None)
		{
			SkillRuleSnapshotValidator::SetFailure(
				Result,
				ESkillRuleSnapshotValidationErrorCode
					::InvalidSkillType,
				FString::Printf(
					TEXT("SkillId '%s' must define a SkillType."),
					*SkillRule.SkillId.ToString()),
				SkillRule.SkillId,
				TEXT("SkillType"));
			return Result;
		}

		if (SkillRule.SkillType != ESkillRuleType::LongShot
			&& SkillRule.SkillType
				!= ESkillRuleType::CutInsideShot)
		{
			SkillRuleSnapshotValidator::SetFailure(
				Result,
				ESkillRuleSnapshotValidationErrorCode
					::UnsupportedSkillType,
				FString::Printf(
					TEXT("SkillId '%s' must use a supported LongShot or CutInsideShot SkillType."),
					*SkillRule.SkillId.ToString()),
				SkillRule.SkillId,
				TEXT("SkillType"));
			return Result;
		}

		if (SkillRule.MinTriggerActionPoint
			< MinTriggerActionPoint)
		{
			SkillRuleSnapshotValidator::SetFailure(
				Result,
				ESkillRuleSnapshotValidationErrorCode
					::TriggerActionPointBelowMinimum,
				FString::Printf(
					TEXT("SkillId '%s' MinTriggerActionPoint must be at least %d."),
					*SkillRule.SkillId.ToString(),
					MinTriggerActionPoint),
				SkillRule.SkillId,
				TEXT("MinTriggerActionPoint"));
			return Result;
		}

		if (SkillRule.MaxTriggerActionPoint
			< MinTriggerActionPoint)
		{
			SkillRuleSnapshotValidator::SetFailure(
				Result,
				ESkillRuleSnapshotValidationErrorCode
					::TriggerActionPointBelowMinimum,
				FString::Printf(
					TEXT("SkillId '%s' MaxTriggerActionPoint must be at least %d."),
					*SkillRule.SkillId.ToString(),
					MinTriggerActionPoint),
				SkillRule.SkillId,
				TEXT("MaxTriggerActionPoint"));
			return Result;
		}

		if (SkillRule.MinTriggerActionPoint
			> MaxTriggerActionPoint)
		{
			SkillRuleSnapshotValidator::SetFailure(
				Result,
				ESkillRuleSnapshotValidationErrorCode
					::TriggerActionPointAboveMaximum,
				FString::Printf(
					TEXT("SkillId '%s' MinTriggerActionPoint must not exceed %d."),
					*SkillRule.SkillId.ToString(),
					MaxTriggerActionPoint),
				SkillRule.SkillId,
				TEXT("MinTriggerActionPoint"));
			return Result;
		}

		if (SkillRule.MaxTriggerActionPoint
			> MaxTriggerActionPoint)
		{
			SkillRuleSnapshotValidator::SetFailure(
				Result,
				ESkillRuleSnapshotValidationErrorCode
					::TriggerActionPointAboveMaximum,
				FString::Printf(
					TEXT("SkillId '%s' MaxTriggerActionPoint must not exceed %d."),
					*SkillRule.SkillId.ToString(),
					MaxTriggerActionPoint),
				SkillRule.SkillId,
				TEXT("MaxTriggerActionPoint"));
			return Result;
		}

		if (SkillRule.MinTriggerActionPoint
			> SkillRule.MaxTriggerActionPoint)
		{
			SkillRuleSnapshotValidator::SetFailure(
				Result,
				ESkillRuleSnapshotValidationErrorCode
					::InvalidTriggerActionPointRange,
				FString::Printf(
					TEXT("SkillId '%s' MinTriggerActionPoint must not exceed MaxTriggerActionPoint."),
					*SkillRule.SkillId.ToString()),
				SkillRule.SkillId,
				TEXT("MinTriggerActionPoint"));
			return Result;
		}
	}

	Result.bSuccess = true;
	Result.bIsValid = true;
	return Result;
}
