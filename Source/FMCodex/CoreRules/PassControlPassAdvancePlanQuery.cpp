#include "PassControlPassAdvancePlanQuery.h"

namespace PassControlPassAdvancePlanQuery
{
	constexpr int32 MinD6 = 1;
	constexpr int32 MaxD6 = 6;
	constexpr float DefenseBonus = 2.0f;

	void SetFailure(
		FPassControlPassAdvancePlanQueryResult& Result,
		const EPassControlPassAdvancePlanQueryErrorCode ErrorCode,
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

	float MakeDefenderModifier(
		const int32 MarkerTackling,
		const int32 HelperMarking)
	{
		return MakeAverageModifier(MarkerTackling, HelperMarking)
			+ DefenseBonus;
	}

	FSingleCardFormulaInputAssemblyQueryInput MakeFormulaQueryInput(
		const FName CardId,
		const ESingleCardFormulaParticipantRole ParticipantRole,
		const ESingleCardFormulaAttribute Attribute,
		const int32 D6,
		const float Modifier,
		const FPassControlPassAdvancePlanQueryInput& Input)
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

FPassControlPassAdvancePlanQueryResult
FPassControlPassAdvancePlanQuery::BuildPlan(
	const FPlayerCardRuleSnapshotSet& PlayerCardSnapshots,
	const FSkillRuleSnapshotSet& SkillRules,
	const FPassControlPassAdvancePlanQueryInput& Input)
{
	FPassControlPassAdvancePlanQueryResult Result;
	Result.Input = Input;

	if (Input.SkillId.IsNone())
	{
		PassControlPassAdvancePlanQuery::SetFailure(
			Result,
			EPassControlPassAdvancePlanQueryErrorCode::MissingSkillId,
			TEXT("Pass Control PassAdvance requires a non-empty SkillId."),
			TEXT("SkillId"));
		return Result;
	}

	if (Input.AdvanceType != EPassControlAdvanceType::PassAdvance)
	{
		PassControlPassAdvancePlanQuery::SetFailure(
			Result,
			EPassControlPassAdvancePlanQueryErrorCode
				::InvalidAdvanceType,
			TEXT("Pass Control PassAdvance requires explicit PassAdvance AdvanceType."),
			TEXT("AdvanceType"));
		return Result;
	}

	Result.CarrierSnapshotQueryResult =
		FPlayerCardRuleSnapshotQuery::FindByCardId(
			PlayerCardSnapshots,
			Input.CarrierCardId);
	if (!Result.CarrierSnapshotQueryResult.bSuccess)
	{
		PassControlPassAdvancePlanQuery::SetFailure(
			Result,
			EPassControlPassAdvancePlanQueryErrorCode
				::CarrierSnapshotQueryFailed,
			Result.CarrierSnapshotQueryResult.ErrorMessage,
			TEXT("CarrierCardId"));
		return Result;
	}

	Result.RunnerSnapshotQueryResult =
		FPlayerCardRuleSnapshotQuery::FindByCardId(
			PlayerCardSnapshots,
			Input.RunnerCardId);
	if (!Result.RunnerSnapshotQueryResult.bSuccess)
	{
		PassControlPassAdvancePlanQuery::SetFailure(
			Result,
			EPassControlPassAdvancePlanQueryErrorCode
				::RunnerSnapshotQueryFailed,
			Result.RunnerSnapshotQueryResult.ErrorMessage,
			TEXT("RunnerCardId"));
		return Result;
	}

	Result.MarkerSnapshotQueryResult =
		FPlayerCardRuleSnapshotQuery::FindByCardId(
			PlayerCardSnapshots,
			Input.MarkerCardId);
	if (!Result.MarkerSnapshotQueryResult.bSuccess)
	{
		PassControlPassAdvancePlanQuery::SetFailure(
			Result,
			EPassControlPassAdvancePlanQueryErrorCode
				::MarkerSnapshotQueryFailed,
			Result.MarkerSnapshotQueryResult.ErrorMessage,
			TEXT("MarkerCardId"));
		return Result;
	}

	Result.HelperSnapshotQueryResult =
		FPlayerCardRuleSnapshotQuery::FindByCardId(
			PlayerCardSnapshots,
			Input.HelperCardId);
	if (!Result.HelperSnapshotQueryResult.bSuccess)
	{
		PassControlPassAdvancePlanQuery::SetFailure(
			Result,
			EPassControlPassAdvancePlanQueryErrorCode
				::HelperSnapshotQueryFailed,
			Result.HelperSnapshotQueryResult.ErrorMessage,
			TEXT("HelperCardId"));
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
		PassControlPassAdvancePlanQuery::SetFailure(
			Result,
			bSkillTypeFailure
				? EPassControlPassAdvancePlanQueryErrorCode
					::SkillTypeMismatch
				: EPassControlPassAdvancePlanQueryErrorCode
					::SkillRuleQueryFailed,
			Result.SkillRuleQueryResult.ErrorMessage,
			bSkillTypeFailure
				? FName(TEXT("SkillType"))
				: FName(TEXT("SkillId")));
		return Result;
	}

	const FPlayerCardRuleSnapshot& CarrierSnapshot =
		Result.CarrierSnapshotQueryResult.Snapshot;
	const FPlayerCardRuleSnapshot& RunnerSnapshot =
		Result.RunnerSnapshotQueryResult.Snapshot;
	const FPlayerCardRuleSnapshot& MarkerSnapshot =
		Result.MarkerSnapshotQueryResult.Snapshot;
	const FPlayerCardRuleSnapshot& HelperSnapshot =
		Result.HelperSnapshotQueryResult.Snapshot;

	if (!CarrierSnapshot.SkillIds.Contains(Input.SkillId))
	{
		PassControlPassAdvancePlanQuery::SetFailure(
			Result,
			EPassControlPassAdvancePlanQueryErrorCode
				::SkillNotOwnedByCarrier,
			TEXT("The carrier snapshot does not own the requested SkillId."),
			TEXT("SkillId"));
		return Result;
	}

	if (Result.SkillRuleQueryResult.Snapshot.SkillType
		!= ESkillRuleType::PassControl)
	{
		PassControlPassAdvancePlanQuery::SetFailure(
			Result,
			EPassControlPassAdvancePlanQueryErrorCode::SkillTypeMismatch,
			TEXT("The requested skill rule must use PassControl SkillType."),
			TEXT("SkillType"));
		return Result;
	}

	if (CarrierSnapshot.bIsGoalkeeper)
	{
		PassControlPassAdvancePlanQuery::SetFailure(
			Result,
			EPassControlPassAdvancePlanQueryErrorCode
				::UnsupportedGoalkeeperParticipant,
			TEXT("Pass Control PassAdvance does not support a goalkeeper carrier."),
			TEXT("CarrierCardId"));
		return Result;
	}

	if (!RunnerSnapshot.PositionTypes.Contains(
			EPlayerPositionType::Midfield))
	{
		PassControlPassAdvancePlanQuery::SetFailure(
			Result,
			EPassControlPassAdvancePlanQueryErrorCode
				::RunnerPositionMismatch,
			TEXT("Pass Control PassAdvance requires a midfield runner."),
			TEXT("RunnerCardId"));
		return Result;
	}

	if (Input.CurrentActionPoint
			< FSkillRuleSnapshotValidator::MinTriggerActionPoint
		|| Input.CurrentActionPoint
			> FSkillRuleSnapshotValidator::MaxTriggerActionPoint)
	{
		PassControlPassAdvancePlanQuery::SetFailure(
			Result,
			EPassControlPassAdvancePlanQueryErrorCode
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
		PassControlPassAdvancePlanQuery::SetFailure(
			Result,
			EPassControlPassAdvancePlanQueryErrorCode
				::ActionPointOutOfRange,
			TEXT("CurrentActionPoint is outside the skill trigger range."),
			TEXT("CurrentActionPoint"));
		return Result;
	}

	if (!Input.bHasExternalAttackD6)
	{
		PassControlPassAdvancePlanQuery::SetFailure(
			Result,
			EPassControlPassAdvancePlanQueryErrorCode
				::MissingExternalAttackD6,
			TEXT("An externally supplied attack D6 is required."),
			TEXT("ExternalAttackD6"));
		return Result;
	}

	if (!PassControlPassAdvancePlanQuery::IsD6InRange(
			Input.ExternalAttackD6))
	{
		PassControlPassAdvancePlanQuery::SetFailure(
			Result,
			EPassControlPassAdvancePlanQueryErrorCode::InvalidAttackD6,
			TEXT("ExternalAttackD6 must be in range [1, 6]."),
			TEXT("ExternalAttackD6"));
		return Result;
	}

	if (!Input.bHasExternalDefenseD6)
	{
		PassControlPassAdvancePlanQuery::SetFailure(
			Result,
			EPassControlPassAdvancePlanQueryErrorCode
				::MissingExternalDefenseD6,
			TEXT("An externally supplied defense D6 is required."),
			TEXT("ExternalDefenseD6"));
		return Result;
	}

	if (!PassControlPassAdvancePlanQuery::IsD6InRange(
			Input.ExternalDefenseD6))
	{
		PassControlPassAdvancePlanQuery::SetFailure(
			Result,
			EPassControlPassAdvancePlanQueryErrorCode
				::InvalidDefenseD6,
			TEXT("ExternalDefenseD6 must be in range [1, 6]."),
			TEXT("ExternalDefenseD6"));
		return Result;
	}

	if (!Input.LogId.IsValid()
		|| Input.TurnIndex < 0
		|| Input.CarrierPlayerId.IsNone()
		|| Input.RunnerPlayerId.IsNone()
		|| Input.MarkerPlayerId.IsNone()
		|| Input.HelperPlayerId.IsNone())
	{
		FName InvalidField(TEXT("LogId"));
		if (Input.LogId.IsValid() && Input.TurnIndex < 0)
		{
			InvalidField = TEXT("TurnIndex");
		}
		else if (Input.LogId.IsValid()
			&& Input.TurnIndex >= 0
			&& Input.CarrierPlayerId.IsNone())
		{
			InvalidField = TEXT("CarrierPlayerId");
		}
		else if (Input.LogId.IsValid()
			&& Input.TurnIndex >= 0
			&& !Input.CarrierPlayerId.IsNone()
			&& Input.RunnerPlayerId.IsNone())
		{
			InvalidField = TEXT("RunnerPlayerId");
		}
		else if (Input.LogId.IsValid()
			&& Input.TurnIndex >= 0
			&& !Input.CarrierPlayerId.IsNone()
			&& !Input.RunnerPlayerId.IsNone()
			&& Input.MarkerPlayerId.IsNone())
		{
			InvalidField = TEXT("MarkerPlayerId");
		}
		else if (Input.LogId.IsValid()
			&& Input.TurnIndex >= 0
			&& !Input.CarrierPlayerId.IsNone()
			&& !Input.RunnerPlayerId.IsNone()
			&& !Input.MarkerPlayerId.IsNone()
			&& Input.HelperPlayerId.IsNone())
		{
			InvalidField = TEXT("HelperPlayerId");
		}

		PassControlPassAdvancePlanQuery::SetFailure(
			Result,
			EPassControlPassAdvancePlanQueryErrorCode
				::InvalidLogContext,
			TEXT("LogId, TurnIndex, and all participant PlayerIds must form valid log context."),
			InvalidField);
		return Result;
	}

	const float AttackerModifier =
		PassControlPassAdvancePlanQuery::MakeAverageModifier(
			CarrierSnapshot.Attributes.Passing,
			RunnerSnapshot.Attributes.Passing);
	const float DefenderModifier =
		PassControlPassAdvancePlanQuery::MakeDefenderModifier(
			MarkerSnapshot.Attributes.Tackling,
			HelperSnapshot.Attributes.Marking);

	Result.FormulaPlan.AttackerQueryInput =
		PassControlPassAdvancePlanQuery::MakeFormulaQueryInput(
			Input.CarrierCardId,
			ESingleCardFormulaParticipantRole::Attacker,
			ESingleCardFormulaAttribute::Passing,
			Input.ExternalAttackD6,
			AttackerModifier,
			Input);
	Result.FormulaPlan.DefenderQueryInput =
		PassControlPassAdvancePlanQuery::MakeFormulaQueryInput(
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
	Result.Decision =
		EPassControlPassAdvancePlanDecision
			::FormulaResolutionRequired;
	Result.bHasFormulaPlan = true;
	return Result;
}
