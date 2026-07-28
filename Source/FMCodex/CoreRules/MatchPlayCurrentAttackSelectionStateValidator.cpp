#include "MatchPlayCurrentAttackSelectionStateValidator.h"

#include "MatchPlaySkillParticipantRequirementQuery.h"
#include "SkillRuleSnapshotValidator.h"

namespace MatchPlayCurrentAttackSelectionStateValidatorImplementation
{
	void SetError(
		FMatchPlayCurrentAttackSelectionStateValidationResult& Result,
		const EMatchPlayCurrentAttackSelectionStateValidationErrorCode
			ErrorCode,
		const FString& ErrorMessage)
	{
		Result.ErrorCode = ErrorCode;
		Result.ErrorMessage = ErrorMessage;
	}

	bool IsSelectedActionPayloadEmpty(
		const FMatchPlayCurrentAttackSelectedAction& SelectedAction)
	{
		return SelectedAction.CarrierCardId.IsNone()
			&& SelectedAction.MarkerCardId.IsNone()
			&& SelectedAction.SkillId.IsNone()
			&& SelectedAction.ActionType == ESkillRuleType::None;
	}

	bool IsPreparationEmpty(
		const FMatchPlayCurrentAttackActionPreparationState& Preparation)
	{
		return Preparation.CarrierCardId.IsNone()
			&& Preparation.MarkerCardId.IsNone()
			&& Preparation.SkillId.IsNone()
			&& Preparation.ActionType == ESkillRuleType::None
			&& Preparation.RunnerCardId.IsNone();
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
	const bool bPreparationHasMarker =
		!CurrentAttack.ActionPreparation.MarkerCardId.IsNone();
	const bool bPreparationHasSkill =
		!CurrentAttack.ActionPreparation.SkillId.IsNone();
	const bool bPreparationHasActionType =
		CurrentAttack.ActionPreparation.ActionType != ESkillRuleType::None;
	const bool bPreparationHasRunner =
		!CurrentAttack.ActionPreparation.RunnerCardId.IsNone();
	const bool bSelectedActionHasCarrier =
		!CurrentAttack.SelectedAction.CarrierCardId.IsNone();
	const bool bSelectedActionPayloadEmpty =
		IsSelectedActionPayloadEmpty(CurrentAttack.SelectedAction);

	if (bPreparationHasCarrier && bSelectedActionHasCarrier)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackSelectionStateValidationErrorCode
				::DuplicateCarrierAuthority,
			TEXT("Preparation and SelectedAction cannot both hold a carrier."));
		return Result;
	}

	if (CurrentAttack.SelectionStage
			!= EMatchPlayCurrentAttackSelectionStage::ReadyForResolution
		&& CurrentAttack.bHasSelectedAction)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackSelectionStateValidationErrorCode
				::SelectedActionUnexpectedlyPresent,
			TEXT("An incomplete selection stage cannot contain a final selected action."));
		return Result;
	}

	if (CurrentAttack.SelectionStage
			!= EMatchPlayCurrentAttackSelectionStage::ReadyForResolution
		&& !bSelectedActionPayloadEmpty)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackSelectionStateValidationErrorCode
				::SelectedActionPayloadNotEmpty,
			TEXT("SelectedAction must remain empty until selection is ready for resolution."));
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
		if (bPreparationHasMarker)
		{
			SetError(
				Result,
				EMatchPlayCurrentAttackSelectionStateValidationErrorCode
					::UnexpectedPreparationMarker,
				TEXT("Deployment cannot contain a preparation marker."));
			return Result;
		}
		if (bPreparationHasSkill)
		{
			SetError(
				Result,
				EMatchPlayCurrentAttackSelectionStateValidationErrorCode
					::UnexpectedPreparationSkill,
				TEXT("Deployment cannot contain a preparation skill."));
			return Result;
		}
		if (bPreparationHasActionType)
		{
			SetError(
				Result,
				EMatchPlayCurrentAttackSelectionStateValidationErrorCode
					::UnexpectedPreparationActionType,
				TEXT("Deployment cannot contain a preparation action type."));
			return Result;
		}
		if (bPreparationHasRunner)
		{
			SetError(
				Result,
				EMatchPlayCurrentAttackSelectionStateValidationErrorCode
					::UnexpectedPreparationRunner,
				TEXT("Deployment cannot contain a preparation runner."));
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
			if (bPreparationHasMarker)
			{
				SetError(
					Result,
					EMatchPlayCurrentAttackSelectionStateValidationErrorCode
						::UnexpectedPreparationMarker,
					TEXT("AwaitingCarrier requires an empty preparation marker."));
				return Result;
			}
			if (bPreparationHasSkill)
			{
				SetError(
					Result,
					EMatchPlayCurrentAttackSelectionStateValidationErrorCode
						::UnexpectedPreparationSkill,
					TEXT("AwaitingCarrier requires an empty preparation skill."));
				return Result;
			}
			if (bPreparationHasActionType)
			{
				SetError(
					Result,
					EMatchPlayCurrentAttackSelectionStateValidationErrorCode
						::UnexpectedPreparationActionType,
					TEXT("AwaitingCarrier requires an empty preparation action type."));
				return Result;
			}
			if (bPreparationHasRunner)
			{
				SetError(
					Result,
					EMatchPlayCurrentAttackSelectionStateValidationErrorCode
						::UnexpectedPreparationRunner,
					TEXT("AwaitingCarrier requires an empty preparation runner."));
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
			if (bPreparationHasMarker)
			{
				SetError(
					Result,
					EMatchPlayCurrentAttackSelectionStateValidationErrorCode
						::UnexpectedPreparationMarker,
					TEXT("AwaitingMarker requires an empty preparation marker."));
				return Result;
			}
			if (bPreparationHasSkill)
			{
				SetError(
					Result,
					EMatchPlayCurrentAttackSelectionStateValidationErrorCode
						::UnexpectedPreparationSkill,
					TEXT("AwaitingMarker requires an empty preparation skill."));
				return Result;
			}
			if (bPreparationHasActionType)
			{
				SetError(
					Result,
					EMatchPlayCurrentAttackSelectionStateValidationErrorCode
						::UnexpectedPreparationActionType,
					TEXT("AwaitingMarker requires an empty preparation action type."));
				return Result;
			}
			if (bPreparationHasRunner)
			{
				SetError(
					Result,
					EMatchPlayCurrentAttackSelectionStateValidationErrorCode
						::UnexpectedPreparationRunner,
					TEXT("AwaitingMarker requires an empty preparation runner."));
				return Result;
			}
			break;

		case EMatchPlayCurrentAttackSelectionStage::AwaitingSkill:
			if (!bPreparationHasCarrier)
			{
				SetError(
					Result,
					EMatchPlayCurrentAttackSelectionStateValidationErrorCode
						::MissingPreparationCarrier,
					TEXT("AwaitingSkill requires a frozen preparation carrier."));
				return Result;
			}
			if (!bPreparationHasMarker)
			{
				SetError(
					Result,
					EMatchPlayCurrentAttackSelectionStateValidationErrorCode
						::MissingPreparationMarker,
					TEXT("AwaitingSkill requires a frozen preparation marker."));
				return Result;
			}
			if (bPreparationHasSkill)
			{
				SetError(
					Result,
					EMatchPlayCurrentAttackSelectionStateValidationErrorCode
						::UnexpectedPreparationSkill,
					TEXT("AwaitingSkill requires an empty preparation skill."));
				return Result;
			}
			if (bPreparationHasActionType)
			{
				SetError(
					Result,
					EMatchPlayCurrentAttackSelectionStateValidationErrorCode
						::UnexpectedPreparationActionType,
					TEXT("AwaitingSkill requires an empty preparation action type."));
				return Result;
			}
			if (bPreparationHasRunner)
			{
				SetError(
					Result,
					EMatchPlayCurrentAttackSelectionStateValidationErrorCode
						::UnexpectedPreparationRunner,
					TEXT("AwaitingSkill requires an empty preparation runner."));
				return Result;
			}
			break;

		case EMatchPlayCurrentAttackSelectionStage::AwaitingRunner:
			if (!bPreparationHasCarrier)
			{
				SetError(
					Result,
					EMatchPlayCurrentAttackSelectionStateValidationErrorCode
						::MissingPreparationCarrier,
					TEXT("AwaitingRunner requires a frozen preparation carrier."));
				return Result;
			}
			if (!bPreparationHasMarker)
			{
				SetError(
					Result,
					EMatchPlayCurrentAttackSelectionStateValidationErrorCode
						::MissingPreparationMarker,
					TEXT("AwaitingRunner requires a frozen preparation marker."));
				return Result;
			}
			if (!bPreparationHasSkill)
			{
				SetError(
					Result,
					EMatchPlayCurrentAttackSelectionStateValidationErrorCode
						::MissingPreparationSkill,
					TEXT("AwaitingRunner requires a frozen preparation skill."));
				return Result;
			}
			if (!bPreparationHasActionType)
			{
				SetError(
					Result,
					EMatchPlayCurrentAttackSelectionStateValidationErrorCode
						::MissingPreparationActionType,
					TEXT("AwaitingRunner requires a frozen preparation action type."));
				return Result;
			}
			if (bPreparationHasRunner)
			{
				SetError(
					Result,
					EMatchPlayCurrentAttackSelectionStateValidationErrorCode
						::UnexpectedPreparationRunner,
					TEXT("AwaitingRunner requires an empty preparation runner."));
				return Result;
			}
			if (!CurrentAttack.bAttackerDeploymentFinished
				|| !CurrentAttack.bDefenderDeploymentFinished)
			{
				SetError(
					Result,
					EMatchPlayCurrentAttackSelectionStateValidationErrorCode
						::ResolutionDeploymentNotComplete,
					TEXT("AwaitingRunner requires both deployment sides to be finished."));
				return Result;
			}
			if (CurrentAttack.CurrentLegalDeploymentSide
				!= EInitialTurnOrderPlayer::None)
			{
				SetError(
					Result,
					EMatchPlayCurrentAttackSelectionStateValidationErrorCode
						::ResolutionLegalDeploymentSideNotCleared,
					TEXT("AwaitingRunner requires no legal deployment side."));
				return Result;
			}
			if (CurrentAttack.ActionPoint
					< FSkillRuleSnapshotValidator::MinTriggerActionPoint
				|| CurrentAttack.ActionPoint
					> FSkillRuleSnapshotValidator::MaxTriggerActionPoint)
			{
				SetError(
					Result,
					EMatchPlayCurrentAttackSelectionStateValidationErrorCode
						::InvalidResolutionActionPoint,
					TEXT("AwaitingRunner requires ActionPoint within 2 through 8."));
				return Result;
			}
			{
				const FMatchPlaySkillParticipantRequirementResult
					Requirement =
						FMatchPlaySkillParticipantRequirementQuery::Query(
							CurrentAttack.ActionPreparation.ActionType);
				if (!Requirement.bSuccess)
				{
					SetError(
						Result,
						EMatchPlayCurrentAttackSelectionStateValidationErrorCode
							::ParticipantRequirementResolutionFailed,
						Requirement.ErrorMessage);
					return Result;
				}
				if (!Requirement.bRequiresRunner
					|| !Requirement.bRequiresHelperStage
					|| Requirement.bCanBecomeReadyImmediately)
				{
					SetError(
						Result,
						EMatchPlayCurrentAttackSelectionStateValidationErrorCode
							::ActionTypeDoesNotMatchSelectionStage,
						TEXT("AwaitingRunner requires a runner-and-helper skill action type."));
					return Result;
				}
			}
			break;

		case EMatchPlayCurrentAttackSelectionStage::AwaitingHelper:
			if (!bPreparationHasCarrier)
			{
				SetError(
					Result,
					EMatchPlayCurrentAttackSelectionStateValidationErrorCode
						::MissingPreparationCarrier,
					TEXT("AwaitingHelper requires a frozen preparation carrier."));
				return Result;
			}
			if (!bPreparationHasMarker)
			{
				SetError(
					Result,
					EMatchPlayCurrentAttackSelectionStateValidationErrorCode
						::MissingPreparationMarker,
					TEXT("AwaitingHelper requires a frozen preparation marker."));
				return Result;
			}
			if (!bPreparationHasSkill)
			{
				SetError(
					Result,
					EMatchPlayCurrentAttackSelectionStateValidationErrorCode
						::MissingPreparationSkill,
					TEXT("AwaitingHelper requires a frozen preparation skill."));
				return Result;
			}
			if (!bPreparationHasActionType)
			{
				SetError(
					Result,
					EMatchPlayCurrentAttackSelectionStateValidationErrorCode
						::MissingPreparationActionType,
					TEXT("AwaitingHelper requires a frozen preparation action type."));
				return Result;
			}
			if (!bPreparationHasRunner)
			{
				SetError(
					Result,
					EMatchPlayCurrentAttackSelectionStateValidationErrorCode
						::MissingPreparationRunner,
					TEXT("AwaitingHelper requires a frozen preparation runner."));
				return Result;
			}
			if (!CurrentAttack.bAttackerDeploymentFinished
				|| !CurrentAttack.bDefenderDeploymentFinished)
			{
				SetError(
					Result,
					EMatchPlayCurrentAttackSelectionStateValidationErrorCode
						::ResolutionDeploymentNotComplete,
					TEXT("AwaitingHelper requires both deployment sides to be finished."));
				return Result;
			}
			if (CurrentAttack.CurrentLegalDeploymentSide
				!= EInitialTurnOrderPlayer::None)
			{
				SetError(
					Result,
					EMatchPlayCurrentAttackSelectionStateValidationErrorCode
						::ResolutionLegalDeploymentSideNotCleared,
					TEXT("AwaitingHelper requires no legal deployment side."));
				return Result;
			}
			if (CurrentAttack.ActionPoint
					< FSkillRuleSnapshotValidator::MinTriggerActionPoint
				|| CurrentAttack.ActionPoint
					> FSkillRuleSnapshotValidator::MaxTriggerActionPoint)
			{
				SetError(
					Result,
					EMatchPlayCurrentAttackSelectionStateValidationErrorCode
						::InvalidResolutionActionPoint,
					TEXT("AwaitingHelper requires ActionPoint within 2 through 8."));
				return Result;
			}
			{
				const FMatchPlaySkillParticipantRequirementResult
					Requirement =
						FMatchPlaySkillParticipantRequirementQuery::Query(
							CurrentAttack.ActionPreparation.ActionType);
				if (!Requirement.bSuccess)
				{
					SetError(
						Result,
						EMatchPlayCurrentAttackSelectionStateValidationErrorCode
							::ParticipantRequirementResolutionFailed,
						Requirement.ErrorMessage);
					return Result;
				}
				if (!Requirement.bRequiresRunner
					|| !Requirement.bRequiresHelperStage
					|| Requirement.bCanBecomeReadyImmediately)
				{
					SetError(
						Result,
						EMatchPlayCurrentAttackSelectionStateValidationErrorCode
							::ActionTypeDoesNotMatchSelectionStage,
						TEXT("AwaitingHelper requires a runner-and-helper skill action type."));
					return Result;
				}
			}
			break;

		case EMatchPlayCurrentAttackSelectionStage::ReadyForResolution:
			if (!IsPreparationEmpty(CurrentAttack.ActionPreparation))
			{
				SetError(
					Result,
					EMatchPlayCurrentAttackSelectionStateValidationErrorCode
						::PreparationAndSelectedActionCoexist,
					TEXT("ReadyForResolution requires empty preparation authority."));
				return Result;
			}
			if (!CurrentAttack.bHasSelectedAction)
			{
				SetError(
					Result,
					EMatchPlayCurrentAttackSelectionStateValidationErrorCode
						::MissingSelectedAction,
					TEXT("ReadyForResolution requires a final selected action."));
				return Result;
			}
			if (CurrentAttack.SelectedAction.CarrierCardId.IsNone())
			{
				SetError(
					Result,
					EMatchPlayCurrentAttackSelectionStateValidationErrorCode
						::MissingSelectedActionCarrier,
					TEXT("ReadyForResolution requires a selected action carrier."));
				return Result;
			}
			if (CurrentAttack.SelectedAction.MarkerCardId.IsNone())
			{
				SetError(
					Result,
					EMatchPlayCurrentAttackSelectionStateValidationErrorCode
						::MissingSelectedActionMarker,
					TEXT("ReadyForResolution requires a selected action marker."));
				return Result;
			}
			if (CurrentAttack.SelectedAction.SkillId.IsNone())
			{
				SetError(
					Result,
					EMatchPlayCurrentAttackSelectionStateValidationErrorCode
						::MissingSelectedActionSkill,
					TEXT("ReadyForResolution requires a selected action skill."));
				return Result;
			}
			if (CurrentAttack.SelectedAction.ActionType
				== ESkillRuleType::None)
			{
				SetError(
					Result,
					EMatchPlayCurrentAttackSelectionStateValidationErrorCode
						::MissingSelectedActionActionType,
					TEXT("ReadyForResolution requires a selected action type."));
				return Result;
			}
			{
				const FMatchPlaySkillParticipantRequirementResult
					Requirement =
						FMatchPlaySkillParticipantRequirementQuery::Query(
							CurrentAttack.SelectedAction.ActionType);
				if (!Requirement.bSuccess)
				{
					SetError(
						Result,
						EMatchPlayCurrentAttackSelectionStateValidationErrorCode
							::ParticipantRequirementResolutionFailed,
						Requirement.ErrorMessage);
					return Result;
				}
				if (Requirement.bRequiresRunner
					|| Requirement.bRequiresHelperStage
					|| !Requirement.bCanBecomeReadyImmediately)
				{
					SetError(
						Result,
						EMatchPlayCurrentAttackSelectionStateValidationErrorCode
							::ActionTypeDoesNotMatchSelectionStage,
						TEXT("ReadyForResolution currently supports only no-runner skill action types."));
					return Result;
				}
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
