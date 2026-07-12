#include "CrossPlanQuery.h"

namespace CrossPlanQuery
{
	constexpr int32 MinD6 = 1;
	constexpr int32 MaxD6 = 6;
	constexpr float DefenseBonus = 2.0f;

	void SetFailure(
		FCrossPlanQueryResult& Result,
		const ECrossPlanQueryErrorCode ErrorCode,
		const FString& ErrorMessage,
		const FName InvalidField)
	{
		Result.ErrorCode = ErrorCode;
		Result.ErrorMessage = ErrorMessage;
		Result.InvalidField = InvalidField;
	}

	bool IsValidActualType(const ECrossPlanActualType ActualType)
	{
		return ActualType == ECrossPlanActualType::High
			|| ActualType == ECrossPlanActualType::Low;
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

	bool HasIdentityConflict(
		const FName LeftCardId,
		const FName LeftPlayerId,
		const FName RightCardId,
		const FName RightPlayerId)
	{
		return LeftCardId == RightCardId || LeftPlayerId == RightPlayerId;
	}

	FSingleCardFormulaInputAssemblyQueryInput MakeFormulaQueryInput(
		const FName CardId,
		const ESingleCardFormulaParticipantRole ParticipantRole,
		const ESingleCardFormulaAttribute Attribute,
		const int32 D6,
		const float Modifier,
		const FCrossPlanQueryInput& Input)
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
		const FCrossPlanQueryInput& Input,
		FName& OutInvalidField,
		ECrossPlanQueryErrorCode& OutErrorCode)
	{
		if (Input.CarrierCardId.IsNone() || Input.CarrierPlayerId.IsNone())
		{
			OutInvalidField = Input.CarrierCardId.IsNone()
				? FName(TEXT("CarrierCardId"))
				: FName(TEXT("CarrierPlayerId"));
			OutErrorCode = ECrossPlanQueryErrorCode::MissingCarrierIdentity;
			return false;
		}
		if (Input.RunnerCardId.IsNone() || Input.RunnerPlayerId.IsNone())
		{
			OutInvalidField = Input.RunnerCardId.IsNone()
				? FName(TEXT("RunnerCardId"))
				: FName(TEXT("RunnerPlayerId"));
			OutErrorCode = ECrossPlanQueryErrorCode::MissingRunnerIdentity;
			return false;
		}
		if (Input.MarkerCardId.IsNone() || Input.MarkerPlayerId.IsNone())
		{
			OutInvalidField = Input.MarkerCardId.IsNone()
				? FName(TEXT("MarkerCardId"))
				: FName(TEXT("MarkerPlayerId"));
			OutErrorCode = ECrossPlanQueryErrorCode::MissingMarkerIdentity;
			return false;
		}
		return true;
	}
}

FCrossPlanQueryResult FCrossPlanQuery::BuildPlan(
	const FPlayerCardRuleSnapshotSet& PlayerCardSnapshots,
	const FSkillRuleSnapshotSet& SkillRules,
	const FCrossPlanQueryInput& Input)
{
	FCrossPlanQueryResult Result;
	Result.Input = Input;
	Result.bHasHelper = Input.bHasHelper;
	Result.bUseGoalkeeper = Input.bUseGoalkeeper;

	if (Input.SkillId.IsNone())
	{
		CrossPlanQuery::SetFailure(
			Result,
			ECrossPlanQueryErrorCode::MissingSkillId,
			TEXT("Cross requires a non-empty SkillId."),
			TEXT("SkillId"));
		return Result;
	}

	if (!CrossPlanQuery::IsValidActualType(Input.ActualCrossType))
	{
		CrossPlanQuery::SetFailure(
			Result,
			ECrossPlanQueryErrorCode::InvalidActualCrossType,
			TEXT("ActualCrossType must be High or Low."),
			TEXT("ActualCrossType"));
		return Result;
	}

	FName InvalidRequiredIdentity;
	ECrossPlanQueryErrorCode MissingIdentityError =
		ECrossPlanQueryErrorCode::None;
	if (!CrossPlanQuery::HasRequiredIdentity(
			Input,
			InvalidRequiredIdentity,
			MissingIdentityError))
	{
		CrossPlanQuery::SetFailure(
			Result,
			MissingIdentityError,
			TEXT("Carrier, Runner, and Marker identities are required."),
			InvalidRequiredIdentity);
		return Result;
	}

	if (Input.bHasHelper)
	{
		if (Input.HelperCardId.IsNone() || Input.HelperPlayerId.IsNone())
		{
			CrossPlanQuery::SetFailure(
				Result,
				ECrossPlanQueryErrorCode::MissingHelperIdentity,
				TEXT("A selected Helper requires both HelperCardId and HelperPlayerId."),
				Input.HelperCardId.IsNone()
					? FName(TEXT("HelperCardId"))
					: FName(TEXT("HelperPlayerId")));
			return Result;
		}
	}
	else if (!Input.HelperCardId.IsNone() || !Input.HelperPlayerId.IsNone())
	{
		CrossPlanQuery::SetFailure(
			Result,
			ECrossPlanQueryErrorCode::UnexpectedHelperIdentity,
			TEXT("An unselected Helper must not provide HelperCardId or HelperPlayerId."),
			!Input.HelperCardId.IsNone()
				? FName(TEXT("HelperCardId"))
				: FName(TEXT("HelperPlayerId")));
		return Result;
	}

	if (Input.bUseGoalkeeper)
	{
		if (Input.GoalkeeperCardId.IsNone() || Input.GoalkeeperPlayerId.IsNone())
		{
			CrossPlanQuery::SetFailure(
				Result,
				ECrossPlanQueryErrorCode::MissingGoalkeeperIdentity,
				TEXT("A selected Goalkeeper requires both GoalkeeperCardId and GoalkeeperPlayerId."),
				Input.GoalkeeperCardId.IsNone()
					? FName(TEXT("GoalkeeperCardId"))
					: FName(TEXT("GoalkeeperPlayerId")));
			return Result;
		}
	}
	else if (!Input.GoalkeeperCardId.IsNone()
		|| !Input.GoalkeeperPlayerId.IsNone())
	{
		CrossPlanQuery::SetFailure(
			Result,
			ECrossPlanQueryErrorCode::UnexpectedGoalkeeperIdentity,
			TEXT("An unselected Goalkeeper must not provide GoalkeeperCardId or GoalkeeperPlayerId."),
			!Input.GoalkeeperCardId.IsNone()
				? FName(TEXT("GoalkeeperCardId"))
				: FName(TEXT("GoalkeeperPlayerId")));
		return Result;
	}

	Result.CarrierSnapshotQueryResult =
		FPlayerCardRuleSnapshotQuery::FindByCardId(
			PlayerCardSnapshots,
			Input.CarrierCardId);
	if (!Result.CarrierSnapshotQueryResult.bSuccess)
	{
		CrossPlanQuery::SetFailure(
			Result,
			ECrossPlanQueryErrorCode::CarrierSnapshotQueryFailed,
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
		CrossPlanQuery::SetFailure(
			Result,
			ECrossPlanQueryErrorCode::RunnerSnapshotQueryFailed,
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
		CrossPlanQuery::SetFailure(
			Result,
			ECrossPlanQueryErrorCode::MarkerSnapshotQueryFailed,
			Result.MarkerSnapshotQueryResult.ErrorMessage,
			TEXT("MarkerCardId"));
		return Result;
	}

	if (Input.bHasHelper)
	{
		Result.HelperSnapshotQueryResult =
			FPlayerCardRuleSnapshotQuery::FindByCardId(
				PlayerCardSnapshots,
				Input.HelperCardId);
		if (!Result.HelperSnapshotQueryResult.bSuccess)
		{
			CrossPlanQuery::SetFailure(
				Result,
				ECrossPlanQueryErrorCode::HelperSnapshotQueryFailed,
				Result.HelperSnapshotQueryResult.ErrorMessage,
				TEXT("HelperCardId"));
			return Result;
		}
	}

	if (Input.bUseGoalkeeper)
	{
		Result.GoalkeeperSnapshotQueryResult =
			FPlayerCardRuleSnapshotQuery::FindByCardId(
				PlayerCardSnapshots,
				Input.GoalkeeperCardId);
		if (!Result.GoalkeeperSnapshotQueryResult.bSuccess)
		{
			CrossPlanQuery::SetFailure(
				Result,
				ECrossPlanQueryErrorCode::GoalkeeperSnapshotQueryFailed,
				Result.GoalkeeperSnapshotQueryResult.ErrorMessage,
				TEXT("GoalkeeperCardId"));
			return Result;
		}
	}

	const FSkillRuleSnapshotQueryInput SkillQueryInput = { Input.SkillId };
	Result.SkillRuleQueryResult = FSkillRuleSnapshotQuery::FindBySkillId(
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
					== ESkillRuleSnapshotValidationErrorCode::InvalidSkillType
				|| ValidationError
					== ESkillRuleSnapshotValidationErrorCode
						::UnsupportedSkillType);
		CrossPlanQuery::SetFailure(
			Result,
			bSkillTypeFailure
				? ECrossPlanQueryErrorCode::SkillRuleTypeMismatch
				: ECrossPlanQueryErrorCode::SkillRuleQueryFailed,
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

	if (Result.SkillRuleQueryResult.Snapshot.SkillType != ESkillRuleType::Cross)
	{
		CrossPlanQuery::SetFailure(
			Result,
			ECrossPlanQueryErrorCode::SkillRuleTypeMismatch,
			TEXT("The requested skill rule must use Cross SkillType."),
			TEXT("SkillType"));
		return Result;
	}

	if (!CarrierSnapshot.SkillIds.Contains(Input.SkillId))
	{
		CrossPlanQuery::SetFailure(
			Result,
			ECrossPlanQueryErrorCode::CarrierDoesNotOwnSkill,
			TEXT("The carrier snapshot does not own the requested SkillId."),
			TEXT("SkillId"));
		return Result;
	}

	if (CarrierSnapshot.bIsGoalkeeper)
	{
		CrossPlanQuery::SetFailure(
			Result,
			ECrossPlanQueryErrorCode::UnsupportedGoalkeeperCarrier,
			TEXT("Cross does not support a goalkeeper carrier."),
			TEXT("CarrierCardId"));
		return Result;
	}

	if (RunnerSnapshot.bIsGoalkeeper)
	{
		CrossPlanQuery::SetFailure(
			Result,
			ECrossPlanQueryErrorCode::UnsupportedGoalkeeperRunner,
			TEXT("Cross does not support a goalkeeper runner."),
			TEXT("RunnerCardId"));
		return Result;
	}

	if (!RunnerSnapshot.PositionTypes.Contains(EPlayerPositionType::Attack))
	{
		CrossPlanQuery::SetFailure(
			Result,
			ECrossPlanQueryErrorCode::RunnerNotForward,
			TEXT("Cross requires a runner with Attack position."),
			TEXT("RunnerCardId"));
		return Result;
	}

	if (CrossPlanQuery::HasIdentityConflict(
			Input.CarrierCardId,
			Input.CarrierPlayerId,
			Input.RunnerCardId,
			Input.RunnerPlayerId))
	{
		CrossPlanQuery::SetFailure(
			Result,
			ECrossPlanQueryErrorCode::CarrierRunnerIdentityConflict,
			TEXT("Carrier and Runner must be different identities."),
			TEXT("RunnerCardId"));
		return Result;
	}

	if (MarkerSnapshot.bIsGoalkeeper)
	{
		CrossPlanQuery::SetFailure(
			Result,
			ECrossPlanQueryErrorCode::UnsupportedGoalkeeperMarker,
			TEXT("Cross does not support a goalkeeper marker."),
			TEXT("MarkerCardId"));
		return Result;
	}

	if (Input.bHasHelper)
	{
		const FPlayerCardRuleSnapshot& HelperSnapshot =
			Result.HelperSnapshotQueryResult.Snapshot;
		if (HelperSnapshot.bIsGoalkeeper)
		{
			CrossPlanQuery::SetFailure(
				Result,
				ECrossPlanQueryErrorCode::UnsupportedGoalkeeperHelper,
				TEXT("Cross does not support a goalkeeper helper."),
				TEXT("HelperCardId"));
			return Result;
		}

		if (CrossPlanQuery::HasIdentityConflict(
				Input.MarkerCardId,
				Input.MarkerPlayerId,
				Input.HelperCardId,
				Input.HelperPlayerId))
		{
			CrossPlanQuery::SetFailure(
				Result,
				ECrossPlanQueryErrorCode::MarkerHelperIdentityConflict,
				TEXT("Marker and Helper must be different identities."),
				TEXT("HelperCardId"));
			return Result;
		}
	}

	if (Input.bUseGoalkeeper)
	{
		const FPlayerCardRuleSnapshot& GoalkeeperSnapshot =
			Result.GoalkeeperSnapshotQueryResult.Snapshot;
		if (!GoalkeeperSnapshot.bIsGoalkeeper)
		{
			CrossPlanQuery::SetFailure(
				Result,
				ECrossPlanQueryErrorCode::GoalkeeperSnapshotNotGoalkeeper,
				TEXT("The selected Goalkeeper snapshot must be a goalkeeper."),
				TEXT("GoalkeeperCardId"));
			return Result;
		}

		if (CrossPlanQuery::HasIdentityConflict(
				Input.MarkerCardId,
				Input.MarkerPlayerId,
				Input.GoalkeeperCardId,
				Input.GoalkeeperPlayerId))
		{
			CrossPlanQuery::SetFailure(
				Result,
				ECrossPlanQueryErrorCode::MarkerGoalkeeperIdentityConflict,
				TEXT("Marker and Goalkeeper must be different identities."),
				TEXT("GoalkeeperCardId"));
			return Result;
		}

		if (Input.bHasHelper && CrossPlanQuery::HasIdentityConflict(
			Input.HelperCardId,
			Input.HelperPlayerId,
			Input.GoalkeeperCardId,
			Input.GoalkeeperPlayerId))
		{
			CrossPlanQuery::SetFailure(
				Result,
				ECrossPlanQueryErrorCode::HelperGoalkeeperIdentityConflict,
				TEXT("Helper and Goalkeeper must be different identities."),
				TEXT("GoalkeeperCardId"));
			return Result;
		}
	}

	if (Input.CurrentActionPoint
		< FSkillRuleSnapshotValidator::MinTriggerActionPoint
		|| Input.CurrentActionPoint
		> FSkillRuleSnapshotValidator::MaxTriggerActionPoint)
	{
		CrossPlanQuery::SetFailure(
			Result,
			ECrossPlanQueryErrorCode::InvalidCurrentActionPoint,
			TEXT("CurrentActionPoint must be in the supported range [2, 8]."),
			TEXT("CurrentActionPoint"));
		return Result;
	}

	const FSkillRuleSnapshot& SkillRule = Result.SkillRuleQueryResult.Snapshot;
	if (Input.CurrentActionPoint < SkillRule.MinTriggerActionPoint
		|| Input.CurrentActionPoint > SkillRule.MaxTriggerActionPoint)
	{
		CrossPlanQuery::SetFailure(
			Result,
			ECrossPlanQueryErrorCode::ActionPointOutOfRange,
			TEXT("CurrentActionPoint is outside the skill trigger range."),
			TEXT("CurrentActionPoint"));
		return Result;
	}

	if (!CrossPlanQuery::IsD6InRange(Input.AttackD6))
	{
		CrossPlanQuery::SetFailure(
			Result,
			ECrossPlanQueryErrorCode::InvalidAttackD6,
			TEXT("AttackD6 must be in range [1, 6]."),
			TEXT("AttackD6"));
		return Result;
	}

	if (!CrossPlanQuery::IsD6InRange(Input.DefenseD6))
	{
		CrossPlanQuery::SetFailure(
			Result,
			ECrossPlanQueryErrorCode::InvalidDefenseD6,
			TEXT("DefenseD6 must be in range [1, 6]."),
			TEXT("DefenseD6"));
		return Result;
	}

	if (!Input.LogId.IsValid() || Input.TurnIndex < 0)
	{
		CrossPlanQuery::SetFailure(
			Result,
			ECrossPlanQueryErrorCode::InvalidLogContext,
			TEXT("LogId and TurnIndex must form valid log context."),
			!Input.LogId.IsValid()
				? FName(TEXT("LogId"))
				: FName(TEXT("TurnIndex")));
		return Result;
	}

	const int32 RunnerAttackAttribute = Input.ActualCrossType
		== ECrossPlanActualType::High
		? RunnerSnapshot.Attributes.Strength
		: RunnerSnapshot.Attributes.Shooting;
	const int32 HelperDefenseAttribute = Input.bHasHelper
		? (Input.ActualCrossType == ECrossPlanActualType::High
			? Result.HelperSnapshotQueryResult.Snapshot.Attributes.Strength
			: Result.HelperSnapshotQueryResult.Snapshot.Attributes.Marking)
		: 0;
	const float GoalkeeperModifier = Input.bUseGoalkeeper
		? static_cast<float>(Input.ActualCrossType == ECrossPlanActualType::High
			? Result.GoalkeeperSnapshotQueryResult.Snapshot.GoalkeeperAttributes.Aerial
			: Result.GoalkeeperSnapshotQueryResult.Snapshot.GoalkeeperAttributes.Reflex)
			* 0.5f
		: 0.0f;
	const float AttackerModifier = CrossPlanQuery::MakeAverageModifier(
		CarrierSnapshot.Attributes.Passing,
		RunnerAttackAttribute);
	const float DefenderModifier = CrossPlanQuery::MakeAverageModifier(
		MarkerSnapshot.Attributes.Tackling,
		HelperDefenseAttribute)
		+ GoalkeeperModifier + CrossPlanQuery::DefenseBonus;

	Result.FormulaPlan.ActualCrossType = Input.ActualCrossType;
	Result.FormulaPlan.AttackerQueryInput =
		CrossPlanQuery::MakeFormulaQueryInput(
			Input.CarrierCardId,
			ESingleCardFormulaParticipantRole::Attacker,
			ESingleCardFormulaAttribute::Passing,
			Input.AttackD6,
			AttackerModifier,
			Input);
	Result.FormulaPlan.DefenderQueryInput =
		CrossPlanQuery::MakeFormulaQueryInput(
			Input.MarkerCardId,
			ESingleCardFormulaParticipantRole::Defender,
			ESingleCardFormulaAttribute::Tackling,
			Input.DefenseD6,
			DefenderModifier,
			Input);
	Result.FormulaPlan.CarrierCardId = Input.CarrierCardId;
	Result.FormulaPlan.CarrierPlayerId = Input.CarrierPlayerId;
	Result.FormulaPlan.RunnerCardId = Input.RunnerCardId;
	Result.FormulaPlan.RunnerPlayerId = Input.RunnerPlayerId;
	Result.FormulaPlan.MarkerCardId = Input.MarkerCardId;
	Result.FormulaPlan.MarkerPlayerId = Input.MarkerPlayerId;
	Result.FormulaPlan.bHasHelper = Input.bHasHelper;
	Result.FormulaPlan.HelperCardId = Input.HelperCardId;
	Result.FormulaPlan.HelperPlayerId = Input.HelperPlayerId;
	Result.FormulaPlan.bUseGoalkeeper = Input.bUseGoalkeeper;
	Result.FormulaPlan.GoalkeeperCardId = Input.GoalkeeperCardId;
	Result.FormulaPlan.GoalkeeperPlayerId = Input.GoalkeeperPlayerId;
	Result.FormulaPlan.GoalScorerCardId = Input.RunnerCardId;
	Result.FormulaPlan.GoalScorerPlayerId = Input.RunnerPlayerId;
	Result.bSuccess = true;
	Result.Decision = ECrossPlanQueryDecision::FormulaResolutionRequired;
	Result.bHasFormulaPlan = true;
	return Result;
}
