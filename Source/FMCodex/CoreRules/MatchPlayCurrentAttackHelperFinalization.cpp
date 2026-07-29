#include "MatchPlayCurrentAttackHelperFinalization.h"

void FMatchPlayCurrentAttackHelperFinalization::ApplyFinalSelectedAction(
	FMatchPlayState& WorkingState,
	const FMatchPlayValidatedHelperPresence& Presence)
{
	FMatchPlayCurrentAttackState& Attack = WorkingState.CurrentAttack;
	const FMatchPlayCurrentAttackActionPreparationState Preparation =
		Attack.ActionPreparation;

	Attack.SelectedAction.CarrierCardId = Preparation.CarrierCardId;
	Attack.SelectedAction.MarkerCardId = Preparation.MarkerCardId;
	Attack.SelectedAction.SkillId = Preparation.SkillId;
	Attack.SelectedAction.ActionType = Preparation.ActionType;
	Attack.SelectedAction.RunnerCardId = Preparation.RunnerCardId;
	Attack.SelectedAction.bHasHelper = Presence.HasHelper();
	Attack.SelectedAction.HelperCardId = Presence.GetHelperCardId();
	Attack.ActionPreparation =
		FMatchPlayCurrentAttackActionPreparationState();
	Attack.bHasSelectedAction = true;
	Attack.SelectionStage =
		EMatchPlayCurrentAttackSelectionStage::ReadyForResolution;
}
