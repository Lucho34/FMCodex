#include "CutInsideShotDirectShotPlanQuery.h"

namespace CutInsideShotDirectShotPlanQuery
{
	constexpr int32 MinD6 = 1;
	constexpr int32 MaxD6 = 6;
	constexpr int32 ImmediateMissMaxD6 = 2;

	void SetFailure(
		FCutInsideShotDirectShotPlanQueryResult& Result,
		const ECutInsideShotDirectShotPlanQueryErrorCode ErrorCode,
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

	float MakeDerivedAverageModifier(
		const int32 Shooting,
		const int32 Dribbling)
	{
		const int32 DerivedModifierTenths =
			(Dribbling - Shooting) * 5;
		return static_cast<float>(DerivedModifierTenths) / 10.0f;
	}

	FSingleCardFormulaInputAssemblyQueryInput MakeFormulaQueryInput(
		const FName CardId,
		const ESingleCardFormulaParticipantRole ParticipantRole,
		const ESingleCardFormulaAttribute Attribute,
		const int32 D6,
		const float Modifier,
		const FCutInsideShotDirectShotPlanQueryInput& Input)
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

FCutInsideShotDirectShotPlanQueryResult
FCutInsideShotDirectShotPlanQuery::BuildPlan(
	const FPlayerCardRuleSnapshotSet& PlayerCardSnapshots,
	const FSkillRuleSnapshotSet& SkillRules,
	const FCutInsideShotDirectShotPlanQueryInput& Input)
{
	FCutInsideShotDirectShotPlanQueryResult Result;
	Result.Input = Input;

	if (Input.SkillId.IsNone())
	{
		CutInsideShotDirectShotPlanQuery::SetFailure(
			Result,
			ECutInsideShotDirectShotPlanQueryErrorCode::MissingSkillId,
			TEXT("Cut Inside Shot direct shot requires a non-empty SkillId."),
			TEXT("SkillId"));
		return Result;
	}

	Result.AttackerSnapshotQueryResult =
		FPlayerCardRuleSnapshotQuery::FindByCardId(
			PlayerCardSnapshots,
			Input.AttackerCardId);
	if (!Result.AttackerSnapshotQueryResult.bSuccess)
	{
		CutInsideShotDirectShotPlanQuery::SetFailure(
			Result,
			ECutInsideShotDirectShotPlanQueryErrorCode
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
		CutInsideShotDirectShotPlanQuery::SetFailure(
			Result,
			ECutInsideShotDirectShotPlanQueryErrorCode
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
		CutInsideShotDirectShotPlanQuery::SetFailure(
			Result,
			bSkillTypeFailure
				? ECutInsideShotDirectShotPlanQueryErrorCode
					::SkillTypeMismatch
				: ECutInsideShotDirectShotPlanQueryErrorCode
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
		CutInsideShotDirectShotPlanQuery::SetFailure(
			Result,
			ECutInsideShotDirectShotPlanQueryErrorCode
				::SkillNotOwnedByAttacker,
			TEXT("The attacker snapshot does not own the requested SkillId."),
			TEXT("SkillId"));
		return Result;
	}

	if (Result.SkillRuleQueryResult.Snapshot.SkillType
		!= ESkillRuleType::CutInsideShot)
	{
		CutInsideShotDirectShotPlanQuery::SetFailure(
			Result,
			ECutInsideShotDirectShotPlanQueryErrorCode::SkillTypeMismatch,
			TEXT("The requested skill rule must use CutInsideShot SkillType."),
			TEXT("SkillType"));
		return Result;
	}

	if (AttackerSnapshot.bIsGoalkeeper
		|| DefenderSnapshot.bIsGoalkeeper)
	{
		CutInsideShotDirectShotPlanQuery::SetFailure(
			Result,
			ECutInsideShotDirectShotPlanQueryErrorCode
				::UnsupportedGoalkeeperParticipant,
			TEXT("Cut Inside Shot direct shot does not support goalkeeper participants."),
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
		CutInsideShotDirectShotPlanQuery::SetFailure(
			Result,
			ECutInsideShotDirectShotPlanQueryErrorCode::InvalidActionPoint,
			TEXT("CurrentActionPoint must be in the supported range [2, 8]."),
			TEXT("CurrentActionPoint"));
		return Result;
	}

	const FSkillRuleSnapshot& SkillRule =
		Result.SkillRuleQueryResult.Snapshot;
	if (Input.CurrentActionPoint < SkillRule.MinTriggerActionPoint
		|| Input.CurrentActionPoint > SkillRule.MaxTriggerActionPoint)
	{
		CutInsideShotDirectShotPlanQuery::SetFailure(
			Result,
			ECutInsideShotDirectShotPlanQueryErrorCode
				::ActionPointOutOfRange,
			TEXT("CurrentActionPoint is outside the skill trigger range."),
			TEXT("CurrentActionPoint"));
		return Result;
	}

	if (!Input.bHasExternalAttackD6)
	{
		CutInsideShotDirectShotPlanQuery::SetFailure(
			Result,
			ECutInsideShotDirectShotPlanQueryErrorCode
				::MissingExternalAttackD6,
			TEXT("An externally supplied attack D6 is required."),
			TEXT("ExternalAttackD6"));
		return Result;
	}

	if (!CutInsideShotDirectShotPlanQuery::IsD6InRange(
			Input.ExternalAttackD6))
	{
		CutInsideShotDirectShotPlanQuery::SetFailure(
			Result,
			ECutInsideShotDirectShotPlanQueryErrorCode::InvalidAttackD6,
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

		CutInsideShotDirectShotPlanQuery::SetFailure(
			Result,
			ECutInsideShotDirectShotPlanQueryErrorCode::InvalidLogContext,
			TEXT("LogId, TurnIndex, and both PlayerIds must form valid log context."),
			InvalidField);
		return Result;
	}

	if (Input.ExternalAttackD6
		<= CutInsideShotDirectShotPlanQuery::ImmediateMissMaxD6)
	{
		Result.bSuccess = true;
		Result.Decision =
			ECutInsideShotDirectShotDecision::ImmediateMiss;
		Result.bAttackEnded = true;
		return Result;
	}

	if (!Input.bHasExternalDefenseD6)
	{
		CutInsideShotDirectShotPlanQuery::SetFailure(
			Result,
			ECutInsideShotDirectShotPlanQueryErrorCode
				::MissingExternalDefenseD6,
			TEXT("An externally supplied defense D6 is required for formula resolution."),
			TEXT("ExternalDefenseD6"));
		return Result;
	}

	if (!CutInsideShotDirectShotPlanQuery::IsD6InRange(
			Input.ExternalDefenseD6))
	{
		CutInsideShotDirectShotPlanQuery::SetFailure(
			Result,
			ECutInsideShotDirectShotPlanQueryErrorCode
				::InvalidDefenseD6,
			TEXT("ExternalDefenseD6 must be in range [1, 6]."),
			TEXT("ExternalDefenseD6"));
		return Result;
	}

	const float AttackerModifier =
		CutInsideShotDirectShotPlanQuery::MakeDerivedAverageModifier(
			AttackerSnapshot.Attributes.Shooting,
			AttackerSnapshot.Attributes.Dribbling);

	Result.FormulaPlan.AttackerQueryInput =
		CutInsideShotDirectShotPlanQuery::MakeFormulaQueryInput(
			Input.AttackerCardId,
			ESingleCardFormulaParticipantRole::Attacker,
			ESingleCardFormulaAttribute::Shooting,
			Input.ExternalAttackD6,
			AttackerModifier,
			Input);
	Result.FormulaPlan.DefenderQueryInput =
		CutInsideShotDirectShotPlanQuery::MakeFormulaQueryInput(
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
		ECutInsideShotDirectShotDecision::FormulaResolutionRequired;
	Result.bHasFormulaPlan = true;
	return Result;
}
