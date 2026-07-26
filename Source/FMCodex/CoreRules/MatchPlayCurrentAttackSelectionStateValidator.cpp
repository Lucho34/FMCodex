#include "MatchPlayCurrentAttackSelectionStateValidator.h"

namespace MatchPlayCurrentAttackSelectionStateValidatorImplementation
{
	void SetError(
		FMatchPlayCurrentAttackSelectionStateValidationResult& Result,
		const EMatchPlayCurrentAttackSelectionStateValidationErrorCode
			ErrorCode,
		const TCHAR* ErrorMessage)
	{
		Result.ErrorCode = ErrorCode;
		Result.ErrorMessage = ErrorMessage;
	}

	bool IsSelectedActionPayloadEmpty(
		const FMatchPlayCurrentAttackSelectedAction& SelectedAction)
	{
		return SelectedAction.CarrierCardId.IsNone()
			&& SelectedAction.SkillId.IsNone()
			&& SelectedAction.ActionType == ESkillRuleType::None;
	}
}

FMatchPlayCurrentAttackSelectionStateValidationResult
FMatchPlayCurrentAttackSelectionStateValidator::Validate(
	const FMatchPlayCurrentAttackState& CurrentAttack)
{
	using namespace
		MatchPlayCurrentAttackSelectionStateValidatorImplementation;

	FMatchPlayCurrentAttackSelectionStateValidationResult Result;
	const bool bPreparationHasCarrier =
		!CurrentAttack.ActionPreparation.CarrierCardId.IsNone();
	const bool bSelectedActionHasCarrier =
		!CurrentAttack.SelectedAction.CarrierCardId.IsNone();

	if (bPreparationHasCarrier && bSelectedActionHasCarrier)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackSelectionStateValidationErrorCode
				::DuplicateCarrierAuthority,
			TEXT("Preparation and SelectedAction cannot both hold a carrier."));
		return Result;
	}

	if (CurrentAttack.bHasSelectedAction)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackSelectionStateValidationErrorCode
				::SelectedActionUnexpectedlyPresent,
			TEXT("Carrier selection foundation does not support a final selected action."));
		return Result;
	}

	if (!IsSelectedActionPayloadEmpty(CurrentAttack.SelectedAction))
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackSelectionStateValidationErrorCode
				::SelectedActionPayloadNotEmpty,
			TEXT("SelectedAction must remain empty during carrier selection."));
		return Result;
	}

	switch (CurrentAttack.Phase)
	{
	case EMatchPlayCurrentAttackPhase::Deployment:
		if (CurrentAttack.SelectionStage
			!= EMatchPlayCurrentAttackSelectionStage::None)
		{
			SetError(
				Result,
				EMatchPlayCurrentAttackSelectionStateValidationErrorCode
					::SelectionStageDoesNotMatchPhase,
				TEXT("Deployment requires SelectionStage None."));
			return Result;
		}

		if (bPreparationHasCarrier)
		{
			SetError(
				Result,
				EMatchPlayCurrentAttackSelectionStateValidationErrorCode
					::UnexpectedPreparationCarrier,
				TEXT("Deployment cannot contain a preparation carrier."));
			return Result;
		}
		break;

	case EMatchPlayCurrentAttackPhase::Resolution:
		switch (CurrentAttack.SelectionStage)
		{
		case EMatchPlayCurrentAttackSelectionStage::AwaitingCarrier:
			if (bPreparationHasCarrier)
			{
				SetError(
					Result,
					EMatchPlayCurrentAttackSelectionStateValidationErrorCode
						::UnexpectedPreparationCarrier,
					TEXT("AwaitingCarrier requires an empty preparation carrier."));
				return Result;
			}
			break;

		case EMatchPlayCurrentAttackSelectionStage::AwaitingMarker:
			if (!bPreparationHasCarrier)
			{
				SetError(
					Result,
					EMatchPlayCurrentAttackSelectionStateValidationErrorCode
						::MissingPreparationCarrier,
					TEXT("AwaitingMarker requires a frozen preparation carrier."));
				return Result;
			}
			break;

		case EMatchPlayCurrentAttackSelectionStage::None:
			SetError(
				Result,
				EMatchPlayCurrentAttackSelectionStateValidationErrorCode
					::SelectionStageDoesNotMatchPhase,
				TEXT("Resolution requires an implemented selection stage."));
			return Result;

		default:
			SetError(
				Result,
				EMatchPlayCurrentAttackSelectionStateValidationErrorCode
					::UnsupportedSelectionStage,
				TEXT("SelectionStage contains an unknown or unsupported value."));
			return Result;
		}
		break;

	default:
		SetError(
			Result,
			EMatchPlayCurrentAttackSelectionStateValidationErrorCode
				::UnsupportedCurrentAttackPhase,
			TEXT("Current attack phase contains an unknown value."));
		return Result;
	}

	Result.bIsCanonical = true;
	Result.ErrorCode =
		EMatchPlayCurrentAttackSelectionStateValidationErrorCode::None;
	Result.ErrorMessage.Empty();
	return Result;
}
