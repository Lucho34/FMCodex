#include "LongShotDirectShotPlanQuery.h"

namespace LongShotDirectShotPlanQuery
{
	constexpr int32 MinD6 = 1;
	constexpr int32 MaxD6 = 6;
	constexpr int32 ImmediateMissMaxD6 = 2;

	void SetFailure(
		FLongShotDirectShotPlanQueryResult& Result,
		const ELongShotDirectShotPlanQueryErrorCode ErrorCode,
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

	FSingleCardFormulaInputAssemblyQueryInput MakeFormulaQueryInput(
		const FName CardId,
		const ESingleCardFormulaParticipantRole ParticipantRole,
		const ESingleCardFormulaAttribute Attribute,
		const int32 D6,
		const float Modifier,
		const FLongShotDirectShotPlanQueryInput& Input)
	{
		FSingleCardFormulaInputAssemblyQueryInput QueryInput;
		QueryInput.CardId = CardId;
		QueryInput.FormulaType = EFormulaType::Finishing;
		QueryInput.ParticipantRole = ParticipantRole;
		QueryInput.Attribute = Attribute;
		QueryInput.bHasExternalD6ComparePoint = true;
		QueryInput.ExternalD6ComparePoint = D6;
		QueryInput.bHasExternalModifier = true;
		QueryInput.ExternalModifier = Modifier;
		QueryInput.LogId = Input.LogId;
		QueryInput.TurnIndex = Input.TurnIndex;
		return QueryInput;
	}
}

FLongShotDirectShotPlanQueryResult
FLongShotDirectShotPlanQuery::BuildPlan(
	const FPlayerCardRuleSnapshotSet& PlayerCardSnapshots,
	const FSkillRuleSnapshotSet& SkillRules,
	const FLongShotDirectShotPlanQueryInput& Input)
{
	FLongShotDirectShotPlanQueryResult Result;
	Result.Input = Input;

	if (Input.SkillId.IsNone())
	{
		LongShotDirectShotPlanQuery::SetFailure(
			Result,
			ELongShotDirectShotPlanQueryErrorCode::MissingSkillId,
			TEXT("Long Shot direct shot requires a non-empty SkillId."),
			TEXT("SkillId"));
		return Result;
	}

	Result.AttackerSnapshotQueryResult =
		FPlayerCardRuleSnapshotQuery::FindByCardId(
			PlayerCardSnapshots,
			Input.AttackerCardId);
	if (!Result.AttackerSnapshotQueryResult.bSuccess)
	{
		LongShotDirectShotPlanQuery::SetFailure(
			Result,
			ELongShotDirectShotPlanQueryErrorCode
				::AttackerSnapshotQueryFailed,
			Result.AttackerSnapshotQueryResult.ErrorMessage,
			TEXT("AttackerCardId"));
		return Result;
	}

	Result.DefenderSnapshotQueryResult =
		FPlayerCardRuleSnapshotQuery::FindByCardId(
			PlayerCardSnapshots,
			Input.DefenderCardId);
	if (!Result.DefenderSnapshotQueryResult.bSuccess)
	{
		LongShotDirectShotPlanQuery::SetFailure(
			Result,
			ELongShotDirectShotPlanQueryErrorCode
				::DefenderSnapshotQueryFailed,
			Result.DefenderSnapshotQueryResult.ErrorMessage,
			TEXT("DefenderCardId"));
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
		const bool bSkillTypeFailure =
			Result.SkillRuleQueryResult.ValidationResult.InvalidSkillId
				== Input.SkillId
			&& (ValidationError
					== ESkillRuleSnapshotValidationErrorCode
						::InvalidSkillType
				|| ValidationError
					== ESkillRuleSnapshotValidationErrorCode
						::UnsupportedSkillType);
		LongShotDirectShotPlanQuery::SetFailure(
			Result,
			bSkillTypeFailure
				? ELongShotDirectShotPlanQueryErrorCode
					::SkillTypeMismatch
				: ELongShotDirectShotPlanQueryErrorCode
					::SkillRuleQueryFailed,
			Result.SkillRuleQueryResult.ErrorMessage,
			bSkillTypeFailure
				? FName(TEXT("SkillType"))
				: FName(TEXT("SkillId")));
		return Result;
	}

	const FPlayerCardRuleSnapshot& AttackerSnapshot =
		Result.AttackerSnapshotQueryResult.Snapshot;
	const FPlayerCardRuleSnapshot& DefenderSnapshot =
		Result.DefenderSnapshotQueryResult.Snapshot;
	if (!AttackerSnapshot.SkillIds.Contains(Input.SkillId))
	{
		LongShotDirectShotPlanQuery::SetFailure(
			Result,
			ELongShotDirectShotPlanQueryErrorCode
				::SkillNotOwnedByAttacker,
			TEXT("The attacker snapshot does not own the requested SkillId."),
			TEXT("SkillId"));
		return Result;
	}

	if (Result.SkillRuleQueryResult.Snapshot.SkillType
		!= ESkillRuleType::LongShot)
	{
		LongShotDirectShotPlanQuery::SetFailure(
			Result,
			ELongShotDirectShotPlanQueryErrorCode::SkillTypeMismatch,
			TEXT("The requested skill rule must use LongShot SkillType."),
			TEXT("SkillType"));
		return Result;
	}

	if (AttackerSnapshot.bIsGoalkeeper
		|| DefenderSnapshot.bIsGoalkeeper)
	{
		LongShotDirectShotPlanQuery::SetFailure(
			Result,
			ELongShotDirectShotPlanQueryErrorCode
				::UnsupportedGoalkeeperParticipant,
			TEXT("Long Shot direct shot does not support goalkeeper participants."),
			AttackerSnapshot.bIsGoalkeeper
				? FName(TEXT("AttackerCardId"))
				: FName(TEXT("DefenderCardId")));
		return Result;
	}

	if (Input.CurrentActionPoint
			< FSkillRuleSnapshotValidator::MinTriggerActionPoint
		|| Input.CurrentActionPoint
			> FSkillRuleSnapshotValidator::MaxTriggerActionPoint)
	{
		LongShotDirectShotPlanQuery::SetFailure(
			Result,
			ELongShotDirectShotPlanQueryErrorCode::InvalidActionPoint,
			TEXT("CurrentActionPoint must be in the supported range [2, 8]."),
			TEXT("CurrentActionPoint"));
		return Result;
	}

	const FSkillRuleSnapshot& SkillRule =
		Result.SkillRuleQueryResult.Snapshot;
	if (Input.CurrentActionPoint < SkillRule.MinTriggerActionPoint
		|| Input.CurrentActionPoint > SkillRule.MaxTriggerActionPoint)
	{
		LongShotDirectShotPlanQuery::SetFailure(
			Result,
			ELongShotDirectShotPlanQueryErrorCode
				::ActionPointOutOfRange,
			TEXT("CurrentActionPoint is outside the skill trigger range."),
			TEXT("CurrentActionPoint"));
		return Result;
	}

	if (!Input.bHasExternalAttackD6)
	{
		LongShotDirectShotPlanQuery::SetFailure(
			Result,
			ELongShotDirectShotPlanQueryErrorCode
				::MissingExternalAttackD6,
			TEXT("An externally supplied attack D6 is required."),
			TEXT("ExternalAttackD6"));
		return Result;
	}

	if (!LongShotDirectShotPlanQuery::IsD6InRange(
			Input.ExternalAttackD6))
	{
		LongShotDirectShotPlanQuery::SetFailure(
			Result,
			ELongShotDirectShotPlanQueryErrorCode::InvalidAttackD6,
			TEXT("ExternalAttackD6 must be in range [1, 6]."),
			TEXT("ExternalAttackD6"));
		return Result;
	}

	if (!Input.LogId.IsValid()
		|| Input.TurnIndex < 0
		|| Input.AttackerPlayerId.IsNone()
		|| Input.DefenderPlayerId.IsNone())
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
		else if (Input.LogId.IsValid()
			&& Input.TurnIndex >= 0
			&& !Input.AttackerPlayerId.IsNone()
			&& Input.DefenderPlayerId.IsNone())
		{
			InvalidField = TEXT("DefenderPlayerId");
		}

		LongShotDirectShotPlanQuery::SetFailure(
			Result,
			ELongShotDirectShotPlanQueryErrorCode::InvalidLogContext,
			TEXT("LogId, TurnIndex, and both PlayerIds must form valid log context."),
			InvalidField);
		return Result;
	}

	if (Input.ExternalAttackD6
		<= LongShotDirectShotPlanQuery::ImmediateMissMaxD6)
	{
		Result.bSuccess = true;
		Result.Decision =
			ELongShotDirectShotDecision::ImmediateMiss;
		Result.bAttackEnded = true;
		return Result;
	}

	if (!Input.bHasExternalDefenseD6)
	{
		LongShotDirectShotPlanQuery::SetFailure(
			Result,
			ELongShotDirectShotPlanQueryErrorCode
				::MissingExternalDefenseD6,
			TEXT("An externally supplied defense D6 is required for formula resolution."),
			TEXT("ExternalDefenseD6"));
		return Result;
	}

	if (!LongShotDirectShotPlanQuery::IsD6InRange(
			Input.ExternalDefenseD6))
	{
		LongShotDirectShotPlanQuery::SetFailure(
			Result,
			ELongShotDirectShotPlanQueryErrorCode
				::InvalidDefenseD6,
			TEXT("ExternalDefenseD6 must be in range [1, 6]."),
			TEXT("ExternalDefenseD6"));
		return Result;
	}

	Result.FormulaPlan.AttackerQueryInput =
		LongShotDirectShotPlanQuery::MakeFormulaQueryInput(
			Input.AttackerCardId,
			ESingleCardFormulaParticipantRole::Attacker,
			ESingleCardFormulaAttribute::LongShot,
			Input.ExternalAttackD6,
			0.0f,
			Input);
	Result.FormulaPlan.DefenderQueryInput =
		LongShotDirectShotPlanQuery::MakeFormulaQueryInput(
			Input.DefenderCardId,
			ESingleCardFormulaParticipantRole::Defender,
			ESingleCardFormulaAttribute::Tackling,
			Input.ExternalDefenseD6,
			2.0f,
			Input);
	Result.FormulaPlan.AttackerPlayerId =
		Input.AttackerPlayerId;
	Result.FormulaPlan.DefenderPlayerId =
		Input.DefenderPlayerId;
	Result.bSuccess = true;
	Result.Decision =
		ELongShotDirectShotDecision::FormulaResolutionRequired;
	Result.bHasFormulaPlan = true;
	return Result;
}
