#include "PassControlDribbleAdvancePlanQuery.h"

namespace PassControlDribbleAdvancePlanQuery
{
	constexpr int32 MinD6 = 1;
	constexpr int32 MaxD6 = 6;
	constexpr float DefenseBonus = 2.0f;

	void SetFailure(
		FPassControlDribbleAdvancePlanQueryResult& Result,
		const EPassControlDribbleAdvancePlanQueryErrorCode ErrorCode,
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

	float MakeAverageModifier(
		const int32 PrimaryAttribute,
		const int32 SecondaryAttribute)
	{
		const int32 ModifierTenths =
			(SecondaryAttribute - PrimaryAttribute) * 5;
		return static_cast<float>(ModifierTenths) / 10.0f;
	}

	FSingleCardFormulaInputAssemblyQueryInput MakeFormulaQueryInput(
		const FName CardId,
		const ESingleCardFormulaParticipantRole ParticipantRole,
		const ESingleCardFormulaAttribute Attribute,
		const int32 D6,
		const float Modifier,
		const FPassControlDribbleAdvancePlanQueryInput& Input)
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

	bool HasRequiredIdentity(
		const FPassControlDribbleAdvancePlanQueryInput& Input,
		FName& OutInvalidField)
	{
		if (Input.CarrierCardId.IsNone())
		{
			OutInvalidField = TEXT("CarrierCardId");
			return false;
		}
		if (Input.RunnerCardId.IsNone())
		{
			OutInvalidField = TEXT("RunnerCardId");
			return false;
		}
		if (Input.MarkerCardId.IsNone())
		{
			OutInvalidField = TEXT("MarkerCardId");
			return false;
		}
		if (Input.CarrierPlayerId.IsNone())
		{
			OutInvalidField = TEXT("CarrierPlayerId");
			return false;
		}
		if (Input.RunnerPlayerId.IsNone())
		{
			OutInvalidField = TEXT("RunnerPlayerId");
			return false;
		}
		if (Input.MarkerPlayerId.IsNone())
		{
			OutInvalidField = TEXT("MarkerPlayerId");
			return false;
		}
		return true;
	}
}

FPassControlDribbleAdvancePlanQueryResult
FPassControlDribbleAdvancePlanQuery::BuildPlan(
	const FPlayerCardRuleSnapshotSet& PlayerCardSnapshots,
	const FSkillRuleSnapshotSet& SkillRules,
	const FPassControlDribbleAdvancePlanQueryInput& Input)
{
	FPassControlDribbleAdvancePlanQueryResult Result;
	Result.Input = Input;
	Result.bHasHelper = Input.bHasHelper;

	if (Input.SkillId.IsNone())
	{
		PassControlDribbleAdvancePlanQuery::SetFailure(
			Result,
			EPassControlDribbleAdvancePlanQueryErrorCode::MissingSkillId,
			TEXT("Pass Control DribbleAdvance requires a non-empty SkillId."),
			TEXT("SkillId"));
		return Result;
	}

	if (Input.AdvanceType != EPassControlAdvanceType::DribbleAdvance)
	{
		PassControlDribbleAdvancePlanQuery::SetFailure(
			Result,
			EPassControlDribbleAdvancePlanQueryErrorCode::InvalidAdvanceType,
			TEXT("Pass Control DribbleAdvance requires explicit DribbleAdvance AdvanceType."),
			TEXT("AdvanceType"));
		return Result;
	}

	FName InvalidRequiredIdentity;
	if (!PassControlDribbleAdvancePlanQuery::HasRequiredIdentity(
			Input,
			InvalidRequiredIdentity))
	{
		PassControlDribbleAdvancePlanQuery::SetFailure(
			Result,
			EPassControlDribbleAdvancePlanQueryErrorCode::MissingRequiredIdentity,
			TEXT("Carrier, Runner, and Marker identities are required."),
			InvalidRequiredIdentity);
		return Result;
	}

	if (Input.bHasHelper)
	{
		if (Input.HelperCardId.IsNone() || Input.HelperPlayerId.IsNone())
		{
			PassControlDribbleAdvancePlanQuery::SetFailure(
				Result,
				EPassControlDribbleAdvancePlanQueryErrorCode::MissingHelperIdentity,
				TEXT("A selected Helper requires both HelperCardId and HelperPlayerId."),
				Input.HelperCardId.IsNone()
					? FName(TEXT("HelperCardId"))
					: FName(TEXT("HelperPlayerId")));
			return Result;
		}
	}
	else if (!Input.HelperCardId.IsNone() || !Input.HelperPlayerId.IsNone())
	{
		PassControlDribbleAdvancePlanQuery::SetFailure(
			Result,
			EPassControlDribbleAdvancePlanQueryErrorCode::UnexpectedHelperIdentity,
			TEXT("An unselected Helper must not provide HelperCardId or HelperPlayerId."),
			!Input.HelperCardId.IsNone()
				? FName(TEXT("HelperCardId"))
				: FName(TEXT("HelperPlayerId")));
		return Result;
	}

	Result.CarrierSnapshotQueryResult = FPlayerCardRuleSnapshotQuery::FindByCardId(
		PlayerCardSnapshots, Input.CarrierCardId);
	if (!Result.CarrierSnapshotQueryResult.bSuccess)
	{
		PassControlDribbleAdvancePlanQuery::SetFailure(
			Result,
			EPassControlDribbleAdvancePlanQueryErrorCode::CarrierSnapshotQueryFailed,
			Result.CarrierSnapshotQueryResult.ErrorMessage,
			TEXT("CarrierCardId"));
		return Result;
	}

	Result.RunnerSnapshotQueryResult = FPlayerCardRuleSnapshotQuery::FindByCardId(
		PlayerCardSnapshots, Input.RunnerCardId);
	if (!Result.RunnerSnapshotQueryResult.bSuccess)
	{
		PassControlDribbleAdvancePlanQuery::SetFailure(
			Result,
			EPassControlDribbleAdvancePlanQueryErrorCode::RunnerSnapshotQueryFailed,
			Result.RunnerSnapshotQueryResult.ErrorMessage,
			TEXT("RunnerCardId"));
		return Result;
	}

	Result.MarkerSnapshotQueryResult = FPlayerCardRuleSnapshotQuery::FindByCardId(
		PlayerCardSnapshots, Input.MarkerCardId);
	if (!Result.MarkerSnapshotQueryResult.bSuccess)
	{
		PassControlDribbleAdvancePlanQuery::SetFailure(
			Result,
			EPassControlDribbleAdvancePlanQueryErrorCode::MarkerSnapshotQueryFailed,
			Result.MarkerSnapshotQueryResult.ErrorMessage,
			TEXT("MarkerCardId"));
		return Result;
	}

	if (Input.bHasHelper)
	{
		Result.HelperSnapshotQueryResult = FPlayerCardRuleSnapshotQuery::FindByCardId(
			PlayerCardSnapshots, Input.HelperCardId);
		if (!Result.HelperSnapshotQueryResult.bSuccess)
		{
			PassControlDribbleAdvancePlanQuery::SetFailure(
				Result,
				EPassControlDribbleAdvancePlanQueryErrorCode::HelperSnapshotQueryFailed,
				Result.HelperSnapshotQueryResult.ErrorMessage,
				TEXT("HelperCardId"));
			return Result;
		}
	}

	const FSkillRuleSnapshotQueryInput SkillQueryInput = { Input.SkillId };
	Result.SkillRuleQueryResult = FSkillRuleSnapshotQuery::FindBySkillId(
		SkillRules, SkillQueryInput);
	if (!Result.SkillRuleQueryResult.bSuccess)
	{
		const ESkillRuleSnapshotValidationErrorCode ValidationError =
			Result.SkillRuleQueryResult.ValidationResult.ErrorCode;
		const bool bSkillTypeFailure =
			Result.SkillRuleQueryResult.ValidationResult.InvalidSkillId == Input.SkillId
			&& (ValidationError == ESkillRuleSnapshotValidationErrorCode::InvalidSkillType
				|| ValidationError == ESkillRuleSnapshotValidationErrorCode::UnsupportedSkillType);
		PassControlDribbleAdvancePlanQuery::SetFailure(
			Result,
			bSkillTypeFailure
				? EPassControlDribbleAdvancePlanQueryErrorCode::SkillRuleTypeMismatch
				: EPassControlDribbleAdvancePlanQueryErrorCode::SkillRuleQueryFailed,
			Result.SkillRuleQueryResult.ErrorMessage,
			bSkillTypeFailure ? FName(TEXT("SkillType")) : FName(TEXT("SkillId")));
		return Result;
	}

	const FPlayerCardRuleSnapshot& CarrierSnapshot =
		Result.CarrierSnapshotQueryResult.Snapshot;
	const FPlayerCardRuleSnapshot& RunnerSnapshot =
		Result.RunnerSnapshotQueryResult.Snapshot;
	const FPlayerCardRuleSnapshot& MarkerSnapshot =
		Result.MarkerSnapshotQueryResult.Snapshot;

	if (!CarrierSnapshot.SkillIds.Contains(Input.SkillId))
	{
		PassControlDribbleAdvancePlanQuery::SetFailure(
			Result,
			EPassControlDribbleAdvancePlanQueryErrorCode::CarrierMissingSkill,
			TEXT("The carrier snapshot does not own the requested SkillId."),
			TEXT("SkillId"));
		return Result;
	}

	if (Result.SkillRuleQueryResult.Snapshot.SkillType != ESkillRuleType::PassControl)
	{
		PassControlDribbleAdvancePlanQuery::SetFailure(
			Result,
			EPassControlDribbleAdvancePlanQueryErrorCode::SkillRuleTypeMismatch,
			TEXT("The requested skill rule must use PassControl SkillType."),
			TEXT("SkillType"));
		return Result;
	}

	if (CarrierSnapshot.bIsGoalkeeper)
	{
		PassControlDribbleAdvancePlanQuery::SetFailure(
			Result,
			EPassControlDribbleAdvancePlanQueryErrorCode::UnsupportedGoalkeeperParticipant,
			TEXT("Pass Control DribbleAdvance does not support a goalkeeper carrier."),
			TEXT("CarrierCardId"));
		return Result;
	}

	if (!RunnerSnapshot.PositionTypes.Contains(EPlayerPositionType::Midfield))
	{
		PassControlDribbleAdvancePlanQuery::SetFailure(
			Result,
			EPassControlDribbleAdvancePlanQueryErrorCode::RunnerNotMidfield,
			TEXT("Pass Control DribbleAdvance requires a midfield runner."),
			TEXT("RunnerCardId"));
		return Result;
	}

	if (Input.CurrentActionPoint < FSkillRuleSnapshotValidator::MinTriggerActionPoint
		|| Input.CurrentActionPoint > FSkillRuleSnapshotValidator::MaxTriggerActionPoint)
	{
		PassControlDribbleAdvancePlanQuery::SetFailure(
			Result,
			EPassControlDribbleAdvancePlanQueryErrorCode::InvalidActionPoint,
			TEXT("CurrentActionPoint must be in the supported range [2, 8]."),
			TEXT("CurrentActionPoint"));
		return Result;
	}

	const FSkillRuleSnapshot& SkillRule = Result.SkillRuleQueryResult.Snapshot;
	if (Input.CurrentActionPoint < SkillRule.MinTriggerActionPoint
		|| Input.CurrentActionPoint > SkillRule.MaxTriggerActionPoint)
	{
		PassControlDribbleAdvancePlanQuery::SetFailure(
			Result,
			EPassControlDribbleAdvancePlanQueryErrorCode::ActionPointOutOfRange,
			TEXT("CurrentActionPoint is outside the skill trigger range."),
			TEXT("CurrentActionPoint"));
		return Result;
	}

	if (!Input.bHasExternalAttackD6)
	{
		PassControlDribbleAdvancePlanQuery::SetFailure(
			Result,
			EPassControlDribbleAdvancePlanQueryErrorCode::MissingExternalAttackD6,
			TEXT("An externally supplied attack D6 is required."),
			TEXT("ExternalAttackD6"));
		return Result;
	}
	if (!PassControlDribbleAdvancePlanQuery::IsD6InRange(Input.ExternalAttackD6))
	{
		PassControlDribbleAdvancePlanQuery::SetFailure(
			Result,
			EPassControlDribbleAdvancePlanQueryErrorCode::InvalidAttackD6,
			TEXT("ExternalAttackD6 must be in range [1, 6]."),
			TEXT("ExternalAttackD6"));
		return Result;
	}
	if (!Input.bHasExternalDefenseD6)
	{
		PassControlDribbleAdvancePlanQuery::SetFailure(
			Result,
			EPassControlDribbleAdvancePlanQueryErrorCode::MissingExternalDefenseD6,
			TEXT("An externally supplied defense D6 is required."),
			TEXT("ExternalDefenseD6"));
		return Result;
	}
	if (!PassControlDribbleAdvancePlanQuery::IsD6InRange(Input.ExternalDefenseD6))
	{
		PassControlDribbleAdvancePlanQuery::SetFailure(
			Result,
			EPassControlDribbleAdvancePlanQueryErrorCode::InvalidDefenseD6,
			TEXT("ExternalDefenseD6 must be in range [1, 6]."),
			TEXT("ExternalDefenseD6"));
		return Result;
	}

	if (!Input.LogId.IsValid() || Input.TurnIndex < 0)
	{
		PassControlDribbleAdvancePlanQuery::SetFailure(
			Result,
			EPassControlDribbleAdvancePlanQueryErrorCode::InvalidLogContext,
			TEXT("LogId and TurnIndex must form valid log context."),
			!Input.LogId.IsValid() ? FName(TEXT("LogId")) : FName(TEXT("TurnIndex")));
		return Result;
	}

	const float AttackerModifier = PassControlDribbleAdvancePlanQuery::MakeAverageModifier(
		CarrierSnapshot.Attributes.Dribbling,
		RunnerSnapshot.Attributes.Passing);
	const int32 HelperMarking = Input.bHasHelper
		? Result.HelperSnapshotQueryResult.Snapshot.Attributes.Marking
		: 0;
	const float DefenderModifier = PassControlDribbleAdvancePlanQuery::MakeAverageModifier(
		MarkerSnapshot.Attributes.Tackling, HelperMarking)
		+ PassControlDribbleAdvancePlanQuery::DefenseBonus;

	Result.FormulaPlan.AttackerQueryInput =
		PassControlDribbleAdvancePlanQuery::MakeFormulaQueryInput(
			Input.CarrierCardId,
			ESingleCardFormulaParticipantRole::Attacker,
			ESingleCardFormulaAttribute::Dribbling,
			Input.ExternalAttackD6,
			AttackerModifier,
			Input);
	Result.FormulaPlan.DefenderQueryInput =
		PassControlDribbleAdvancePlanQuery::MakeFormulaQueryInput(
			Input.MarkerCardId,
			ESingleCardFormulaParticipantRole::Defender,
			ESingleCardFormulaAttribute::Tackling,
			Input.ExternalDefenseD6,
			DefenderModifier,
			Input);
	Result.FormulaPlan.CarrierPlayerId = Input.CarrierPlayerId;
	Result.FormulaPlan.RunnerPlayerId = Input.RunnerPlayerId;
	Result.FormulaPlan.MarkerPlayerId = Input.MarkerPlayerId;
	Result.FormulaPlan.HelperPlayerId = Input.HelperPlayerId;
	Result.FormulaPlan.RunnerCardId = Input.RunnerCardId;
	Result.FormulaPlan.HelperCardId = Input.HelperCardId;
	Result.bSuccess = true;
	Result.Decision = EPassControlDribbleAdvancePlanDecision::FormulaResolutionRequired;
	Result.bHasFormulaPlan = true;
	return Result;
}
