#include "FMCodexNetworkMatchScreenActions.h"

bool FFMCodexNetworkMatchScreenActions::Begin(const FFMCodexMatchScreenRequest& Request,
	const FFMCodexNetworkClientViewSnapshot& View, FFMCodexNetworkIntentClientState& Client,
	FFMCodexNetworkPlayerIntentEnvelope& Out)
{
	using K = EFMCodexMatchScreenIntent;
	using C = EFMCodexUMGInteractionCategory;
	using N = EFMCodexNetworkPlayerIntentKind;
	if (!View.Presentation.bAvailable || Client.IsPending()) return false;
	switch (Request.Kind)
	{
	case K::TacticalPoints: return Client.Begin(View, Out);
	case K::DeployOrdinary:
	{
		FFMCodexNetworkDeployOrdinaryPayload P; P.CardId = Request.OptionId; P.SlotId = Request.SlotId;
		return Client.BeginDeployment(View, P, Out);
	}
	case K::DeployGoalkeeper:
	{
		FFMCodexNetworkDeployGoalkeeperPayload P; P.SlotId = Request.SlotId;
		return Client.BeginGoalkeeper(View, P, Out);
	}
	case K::FinishDeployment: return Client.BeginFinishDeployment(View, Out);
	case K::Carrier:
	{
		FFMCodexNetworkSubmitCarrierPayload P; P.CarrierCardId = Request.OptionId;
		return Client.BeginCarrier(View, P, Out);
	}
	case K::Marker:
	{
		FFMCodexNetworkSubmitMarkerPayload P; P.MarkerCardId = Request.OptionId;
		return Client.BeginMarker(View, P, Out);
	}
	case K::Runner:
	{
		FFMCodexNetworkSubmitRunnerPayload P; P.RunnerCardId = Request.OptionId;
		return Client.BeginRunner(View, P, Out);
	}
	case K::Helper:
	{
		FFMCodexNetworkSubmitHelperPayload P; P.HelperCardId = Request.OptionId;
		return Client.BeginHelper(View, P, Out);
	}
	case K::Skill:
	{
		const auto* Choice = View.Presentation.Interaction.SelectionChoices.FindByPredicate(
			[&](const auto& O) { return O.OptionId == Request.OptionId && O.bEnabled && O.SkillType == ESkillRuleType::Cross; });
		if (!Choice) return false;
		FFMCodexNetworkSubmitSkillPayload P; P.SkillId = Request.OptionId;
		return Client.BeginSkill(View, P, Out);
	}
	case K::Branch:
	{
		FFMCodexNetworkSubmitBranchIntentPayload P;
		switch (Request.Branch)
		{
		case EFMCodexUMGBranchIntent::CrossHigh: P.Intent = EMatchPlayElectiveBranchIntent::CrossHigh; break;
		case EFMCodexUMGBranchIntent::CrossLow: P.Intent = EMatchPlayElectiveBranchIntent::CrossLow; break;
		default: return false;
		}
		return Client.BeginBranch(View, P, Out);
	}
	case K::Continue:
		if (Request.Category != View.Presentation.Interaction.Category) return false;
		switch (Request.Category)
		{
		case C::RollCrossRoute: return Client.BeginInitialRoute(View, N::CrossInitialRouteRoll, Out);
		case C::AdvanceAfterTerminal: return Client.BeginAdvance(View, Out);
		case C::RollCrossAttack:
		case C::RollCrossDefense:
			switch (View.CrossContestAction)
			{
			case EFMCodexNetworkCrossContestAction::CrossHighAttackRoll: return Client.BeginCrossContest(View, N::CrossHighAttackRoll, Out);
			case EFMCodexNetworkCrossContestAction::CrossHighDefenseRoll: return Client.BeginCrossContest(View, N::CrossHighDefenseRoll, Out);
			case EFMCodexNetworkCrossContestAction::CrossLowAttackRoll: return Client.BeginCrossContest(View, N::CrossLowAttackRoll, Out);
			case EFMCodexNetworkCrossContestAction::CrossLowDefenseRoll: return Client.BeginCrossContest(View, N::CrossLowDefenseRoll, Out);
			default: return false;
			}
		default: return false;
		}
	default: return false; // Local Start, declines and unnetworked families have no transport capability.
	}
}
