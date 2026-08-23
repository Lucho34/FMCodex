#include "MatchPlayCurrentAttackHelperFinalization.h"

void FMatchPlayCurrentAttackHelperFinalization
	::ApplyValidatedHelperCompletion(
	FMatchPlayState& WorkingState,
	const FMatchPlayValidatedHelperPresence& Presence)
{
	FMatchPlayCurrentAttackState& Attack =
		WorkingState.CurrentAttack;
	Attack.ActionPreparation.bHasHelper = Presence.HasHelper();
	Attack.ActionPreparation.HelperCardId =
		Presence.GetHelperCardId();
	if (Attack.ActionPreparation.bSkillSelectionDeferred)
	{
		Attack.SelectionStage =
			EMatchPlayCurrentAttackSelectionStage::AwaitingSkill;
		return;
	}
	if (Attack.ActionPreparation.ActionType == ESkillRuleType::Cross)
	{
		Attack.SelectionStage =
			EMatchPlayCurrentAttackSelectionStage
				::AwaitingBranchIntent;
		return;
	}

	ApplyFinalSelectedAction(
		WorkingState,
		EMatchPlayElectiveBranchIntent::None);
}

void FMatchPlayCurrentAttackHelperFinalization::ApplyFinalSelectedAction(
	FMatchPlayState& WorkingState,
	const EMatchPlayElectiveBranchIntent Intent)
{
	FMatchPlayCurrentAttackState& Attack =
		WorkingState.CurrentAttack;
	const FMatchPlayCurrentAttackActionPreparationState Preparation =
		Attack.ActionPreparation;

	Attack.SelectedAction.CarrierCardId = Preparation.CarrierCardId;
	Attack.SelectedAction.MarkerCardId = Preparation.MarkerCardId;
	Attack.SelectedAction.SkillId = Preparation.SkillId;
	Attack.SelectedAction.ActionType = Preparation.ActionType;
	Attack.SelectedAction.RunnerCardId = Preparation.RunnerCardId;
	Attack.SelectedAction.bHasHelper = Preparation.bHasHelper;
	Attack.SelectedAction.HelperCardId = Preparation.HelperCardId;
	Attack.SelectedAction.ElectiveBranchIntent = Intent;
	Attack.ActionPreparation =
		FMatchPlayCurrentAttackActionPreparationState();
	Attack.bHasSelectedAction = true;
	Attack.SelectionStage =
		EMatchPlayCurrentAttackSelectionStage::ReadyForResolution;
}
