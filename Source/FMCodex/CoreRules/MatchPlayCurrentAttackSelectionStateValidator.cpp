#include "MatchPlayCurrentAttackSelectionStateValidator.h"

#include "MatchPlayElectiveBranchIntentRules.h"
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
			&& SelectedAction.ActionType == ESkillRuleType::None
			&& SelectedAction.RunnerCardId.IsNone()
			&& !SelectedAction.bHasHelper
			&& SelectedAction.HelperCardId.IsNone()
			&& SelectedAction.ElectiveBranchIntent
				== EMatchPlayElectiveBranchIntent::None;
	}

	bool IsPreparationEmpty(
		const FMatchPlayCurrentAttackActionPreparationState& Preparation)
	{
		return !Preparation.bSkillSelectionDeferred
			&& Preparation.CarrierCardId.IsNone()
			&& Preparation.MarkerCardId.IsNone()
			&& Preparation.SkillId.IsNone()
			&& Preparation.ActionType == ESkillRuleType::None
			&& Preparation.RunnerCardId.IsNone()
			&& !Preparation.bHasHelper
			&& Preparation.HelperCardId.IsNone();
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
	const bool bSkillSelectionDeferred =
		CurrentAttack.ActionPreparation.bSkillSelectionDeferred;
	const bool bPreparationHasMarker =
		!CurrentAttack.ActionPreparation.MarkerCardId.IsNone();
	const bool bPreparationHasSkill =
		!CurrentAttack.ActionPreparation.SkillId.IsNone();
	const bool bPreparationHasActionType =
		CurrentAttack.ActionPreparation.ActionType != ESkillRuleType::None;
	const bool bPreparationHasRunner =
		!CurrentAttack.ActionPreparation.RunnerCardId.IsNone();
	const bool bPreparationHasHelper =
		CurrentAttack.ActionPreparation.bHasHelper;
	const bool bPreparationHasHelperCardId =
		!CurrentAttack.ActionPreparation.HelperCardId.IsNone();
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

	if (bSkillSelectionDeferred && bPreparationHasActionType
		&& CurrentAttack.ActionPreparation.ActionType != ESkillRuleType::Cross)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackSelectionStateValidationErrorCode
				::ActionTypeDoesNotMatchSelectionStage,
			TEXT("Deferred preparation may be generic or retain the legacy frozen Cross action type only."));
		return Result;
	}

	if (bSkillSelectionDeferred && bPreparationHasSkill)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackSelectionStateValidationErrorCode
				::UnexpectedPreparationSkill,
			TEXT("Deferred Skill selection requires an empty preparation SkillId."));
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

	if (CurrentAttack.SelectionStage
			!= EMatchPlayCurrentAttackSelectionStage::AwaitingSkill
		&& CurrentAttack.SelectionStage
			!= EMatchPlayCurrentAttackSelectionStage::AwaitingBranchIntent
		&& CurrentAttack.SelectionStage
			!= EMatchPlayCurrentAttackSelectionStage::ReadyForResolution
		&& (bPreparationHasHelper || bPreparationHasHelperCardId))
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackSelectionStateValidationErrorCode
				::UnexpectedPreparationHelper,
			TEXT("Preparation Helper authority is only valid while awaiting branch intent."));
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
			if (bSkillSelectionDeferred)
			{
				if (bPreparationHasActionType
					&& CurrentAttack.ActionPreparation.ActionType
						!= ESkillRuleType::Cross)
				{
					SetError(
						Result,
						EMatchPlayCurrentAttackSelectionStateValidationErrorCode
							::UnexpectedPreparationActionType,
						TEXT("Deferred AwaitingSkill supports generic participant-first authority or legacy Cross only."));
					return Result;
				}
				if (!bPreparationHasRunner
					&& (bPreparationHasActionType
						|| bPreparationHasHelper
						|| bPreparationHasHelperCardId))
				{
					SetError(
						Result,
						EMatchPlayCurrentAttackSelectionStateValidationErrorCode
							::MissingPreparationRunner,
						TEXT("Deferred AwaitingSkill without a runner requires generic action authority and no helper."));
					return Result;
				}
				if (bPreparationHasHelper != bPreparationHasHelperCardId)
				{
					SetError(
						Result,
						EMatchPlayCurrentAttackSelectionStateValidationErrorCode
							::InvalidPreparationHelperPresence,
						TEXT("Deferred AwaitingSkill Helper presence is inconsistent."));
					return Result;
				}
			}
			else if (bPreparationHasActionType)
			{
				SetError(
					Result,
					EMatchPlayCurrentAttackSelectionStateValidationErrorCode
						::UnexpectedPreparationActionType,
					TEXT("Ordinary AwaitingSkill requires an empty preparation action type."));
				return Result;
			}
			else if (bPreparationHasRunner
				|| bPreparationHasHelper || bPreparationHasHelperCardId)
			{
				SetError(
					Result,
					EMatchPlayCurrentAttackSelectionStateValidationErrorCode
						::UnexpectedPreparationRunner,
					TEXT("Ordinary AwaitingSkill cannot contain participant selections."));
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
			if (!bPreparationHasSkill && !bSkillSelectionDeferred)
			{
				SetError(
					Result,
					EMatchPlayCurrentAttackSelectionStateValidationErrorCode
						::MissingPreparationSkill,
					TEXT("AwaitingRunner requires a selected Skill or participant-first authority."));
				return Result;
			}
			{
			const bool bGenericParticipantFirstRunner =
				bSkillSelectionDeferred && !bPreparationHasSkill
				&& !bPreparationHasActionType;
			if (!bGenericParticipantFirstRunner && !bPreparationHasActionType)
			{
				SetError(
					Result,
					EMatchPlayCurrentAttackSelectionStateValidationErrorCode
						::MissingPreparationActionType,
					TEXT("Non-generic AwaitingRunner requires a frozen action type."));
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
			if (!bGenericParticipantFirstRunner)
			{
				const FMatchPlaySkillParticipantRequirementResult Requirement =
					FMatchPlaySkillParticipantRequirementQuery::Query(
						CurrentAttack.ActionPreparation.ActionType);
				if (!Requirement.bSuccess || !Requirement.bRequiresRunner
					|| !Requirement.bRequiresHelperStage
					|| Requirement.bCanBecomeReadyImmediately)
				{
					SetError(
						Result,
						EMatchPlayCurrentAttackSelectionStateValidationErrorCode
							::ActionTypeDoesNotMatchSelectionStage,
						TEXT("AwaitingRunner action type must consume Runner and Helper."));
					return Result;
				}
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
			if (!bPreparationHasSkill && !bSkillSelectionDeferred)
			{
				SetError(
					Result,
					EMatchPlayCurrentAttackSelectionStateValidationErrorCode
						::MissingPreparationSkill,
					TEXT("AwaitingHelper requires a selected Skill or participant-first authority."));
				return Result;
			}
			{
			const bool bGenericParticipantFirstHelper =
				bSkillSelectionDeferred && !bPreparationHasSkill
				&& !bPreparationHasActionType;
			if (!bGenericParticipantFirstHelper && !bPreparationHasActionType)
			{
				SetError(
					Result,
					EMatchPlayCurrentAttackSelectionStateValidationErrorCode
						::MissingPreparationActionType,
					TEXT("Non-generic AwaitingHelper requires a frozen action type."));
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
			if (!bGenericParticipantFirstHelper)
			{
				const FMatchPlaySkillParticipantRequirementResult Requirement =
					FMatchPlaySkillParticipantRequirementQuery::Query(
						CurrentAttack.ActionPreparation.ActionType);
				if (!Requirement.bSuccess || !Requirement.bRequiresRunner
					|| !Requirement.bRequiresHelperStage
					|| Requirement.bCanBecomeReadyImmediately)
				{
					SetError(
						Result,
						EMatchPlayCurrentAttackSelectionStateValidationErrorCode
							::ActionTypeDoesNotMatchSelectionStage,
						TEXT("AwaitingHelper action type must consume Runner and Helper."));
					return Result;
				}
			}
			}
			break;

		case EMatchPlayCurrentAttackSelectionStage::AwaitingBranchIntent:
			if (!bPreparationHasCarrier)
			{
				SetError(
					Result,
					EMatchPlayCurrentAttackSelectionStateValidationErrorCode
						::MissingPreparationCarrier,
					TEXT("AwaitingBranchIntent requires a frozen preparation carrier."));
				return Result;
			}
			if (!bPreparationHasMarker)
			{
				SetError(
					Result,
					EMatchPlayCurrentAttackSelectionStateValidationErrorCode
						::MissingPreparationMarker,
					TEXT("AwaitingBranchIntent requires a frozen preparation marker."));
				return Result;
			}
			if (!bPreparationHasSkill)
			{
				SetError(
					Result,
					EMatchPlayCurrentAttackSelectionStateValidationErrorCode
						::MissingPreparationSkill,
					TEXT("AwaitingBranchIntent requires a frozen preparation skill."));
				return Result;
			}
			if (!bPreparationHasActionType)
			{
				SetError(
					Result,
					EMatchPlayCurrentAttackSelectionStateValidationErrorCode
						::MissingPreparationActionType,
					TEXT("AwaitingBranchIntent requires a frozen preparation action type."));
				return Result;
			}
			if (!CurrentAttack.bAttackerDeploymentFinished
				|| !CurrentAttack.bDefenderDeploymentFinished)
			{
				SetError(
					Result,
					EMatchPlayCurrentAttackSelectionStateValidationErrorCode
						::ResolutionDeploymentNotComplete,
					TEXT("AwaitingBranchIntent requires both deployment sides to be finished."));
				return Result;
			}
			if (CurrentAttack.CurrentLegalDeploymentSide
				!= EInitialTurnOrderPlayer::None)
			{
				SetError(
					Result,
					EMatchPlayCurrentAttackSelectionStateValidationErrorCode
						::ResolutionLegalDeploymentSideNotCleared,
					TEXT("AwaitingBranchIntent requires no legal deployment side."));
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
					TEXT("AwaitingBranchIntent requires ActionPoint within 2 through 8."));
				return Result;
			}
			if (CurrentAttack.ActionPreparation.ActionType
					== ESkillRuleType::LongShot
				|| CurrentAttack.ActionPreparation.ActionType
					== ESkillRuleType::CutInsideShot)
			{
				if (bPreparationHasRunner)
				{
					SetError(
						Result,
						EMatchPlayCurrentAttackSelectionStateValidationErrorCode
							::UnexpectedPreparationRunner,
						TEXT("Shot branch intent selection cannot contain a Runner."));
					return Result;
				}
				if (bPreparationHasHelper
					|| bPreparationHasHelperCardId)
				{
					SetError(
						Result,
						EMatchPlayCurrentAttackSelectionStateValidationErrorCode
							::UnexpectedPreparationHelper,
						TEXT("Shot branch intent selection cannot contain a Helper."));
					return Result;
				}
			}
			else if (CurrentAttack.ActionPreparation.ActionType
				== ESkillRuleType::Cross)
			{
				if (!bPreparationHasRunner)
				{
					SetError(
						Result,
						EMatchPlayCurrentAttackSelectionStateValidationErrorCode
							::MissingPreparationRunner,
						TEXT("Cross branch intent selection requires a Runner."));
					return Result;
				}
				if (bPreparationHasHelper
					!= bPreparationHasHelperCardId)
				{
					SetError(
						Result,
						EMatchPlayCurrentAttackSelectionStateValidationErrorCode
							::InvalidPreparationHelperPresence,
						TEXT("Cross branch intent Helper presence is inconsistent."));
					return Result;
				}
			}
			else
			{
				SetError(
					Result,
					EMatchPlayCurrentAttackSelectionStateValidationErrorCode
						::ActionTypeDoesNotMatchSelectionStage,
					TEXT("AwaitingBranchIntent requires LongShot, CutInsideShot, or Cross."));
				return Result;
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
				if (Requirement.bRequiresRunner)
				{
					if (!Requirement.bRequiresHelperStage
						|| Requirement.bCanBecomeReadyImmediately)
					{
						SetError(
							Result,
							EMatchPlayCurrentAttackSelectionStateValidationErrorCode
								::ActionTypeDoesNotMatchSelectionStage,
							TEXT("ReadyForResolution runner skills require the helper selection stage."));
						return Result;
					}
					if (CurrentAttack.SelectedAction.RunnerCardId.IsNone())
					{
						SetError(
							Result,
							EMatchPlayCurrentAttackSelectionStateValidationErrorCode
								::MissingSelectedActionRunner,
							TEXT("ReadyForResolution runner skills require a selected runner."));
						return Result;
					}
					if (CurrentAttack.SelectedAction.bHasHelper
						&& CurrentAttack.SelectedAction.HelperCardId.IsNone())
					{
						SetError(
							Result,
							EMatchPlayCurrentAttackSelectionStateValidationErrorCode
								::MissingSelectedActionHelper,
							TEXT("A present Helper requires a non-empty HelperCardId."));
						return Result;
					}
					if (!CurrentAttack.SelectedAction.bHasHelper
						&& !CurrentAttack.SelectedAction.HelperCardId.IsNone())
					{
						SetError(
							Result,
							EMatchPlayCurrentAttackSelectionStateValidationErrorCode
								::UnexpectedSelectedActionHelper,
							TEXT("An absent Helper requires an empty HelperCardId."));
						return Result;
					}
				}
				else
				{
					if (Requirement.bRequiresHelperStage
						|| !Requirement.bCanBecomeReadyImmediately)
					{
						SetError(
							Result,
							EMatchPlayCurrentAttackSelectionStateValidationErrorCode
								::ActionTypeDoesNotMatchSelectionStage,
							TEXT("ReadyForResolution participant requirements are inconsistent."));
						return Result;
					}
					if (!CurrentAttack.SelectedAction.RunnerCardId.IsNone())
					{
						SetError(
							Result,
							EMatchPlayCurrentAttackSelectionStateValidationErrorCode
								::UnexpectedSelectedActionRunner,
							TEXT("No-runner skills require an empty RunnerCardId."));
						return Result;
					}
					if (CurrentAttack.SelectedAction.bHasHelper
						|| !CurrentAttack.SelectedAction.HelperCardId.IsNone())
					{
						SetError(
							Result,
							EMatchPlayCurrentAttackSelectionStateValidationErrorCode
								::UnexpectedSelectedActionHelper,
							TEXT("No-runner skills cannot contain a Helper."));
						return Result;
					}
				}
			}
			if (!MatchPlayElectiveBranchIntentRules::IsLegalIntent(
				CurrentAttack.SelectedAction.ActionType,
				CurrentAttack.SelectedAction.ElectiveBranchIntent))
			{
				SetError(
					Result,
					EMatchPlayCurrentAttackSelectionStateValidationErrorCode
						::InvalidSelectedActionBranchIntent,
					TEXT("Selected action branch intent does not match its action type."));
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
