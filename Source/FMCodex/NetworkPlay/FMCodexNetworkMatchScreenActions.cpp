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
	case K::SetPieceType: return View.Presentation.SetPiece.bSelectionSupported && Client.BeginSetPiece(View, N::RequestSetPieceTypeRoll, Out);
	case K::SetPieceTaker: return View.Presentation.SetPiece.bSelectionSupported && Client.BeginSetPiece(View, N::SubmitSetPieceCarrier, Out, Request.OptionId);
	case K::NearMethod: return View.Presentation.SetPiece.bSelectionSupported && Client.BeginSetPiece(View, N::SubmitShortFreeKickMethod, Out, NAME_None, Request.NearMethod);
	case K::LongMethod: return View.Presentation.SetPiece.bSelectionSupported && Client.BeginSetPiece(View, N::SubmitLongFreeKickMethod, Out, NAME_None, EMatchPlayShortFreeKickMethod::None, Request.LongMethod);
	case K::PenaltyMethod: return View.Presentation.SetPiece.bSelectionSupported && Client.BeginSetPiece(View, N::SubmitPenaltyMethod, Out, NAME_None, EMatchPlayShortFreeKickMethod::None, EMatchPlayLongFreeKickMethod::None, Request.PenaltyMethod);
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
			[&](const auto& O) { return O.OptionId == Request.OptionId && O.bEnabled; });
		if (!Choice) return false;
		FFMCodexNetworkSubmitSkillPayload P; P.SkillId = Request.OptionId;
		return Client.BeginSkill(View, P, Out);
	}
	case K::Branch:
	{
		FFMCodexNetworkSubmitBranchIntentPayload P;
		switch (Request.Branch)
		{
		case EFMCodexUMGBranchIntent::DirectShot: P.Intent = EMatchPlayElectiveBranchIntent::DirectShot; break;
		case EFMCodexUMGBranchIntent::DeadCorner: P.Intent = EMatchPlayElectiveBranchIntent::DeadCorner; break;
		case EFMCodexUMGBranchIntent::CrossHigh: P.Intent = EMatchPlayElectiveBranchIntent::CrossHigh; break;
		case EFMCodexUMGBranchIntent::CrossLow: P.Intent = EMatchPlayElectiveBranchIntent::CrossLow; break;
		default: return false;
		}
		return Client.BeginBranch(View, P, Out);
	}
	case K::Decline:
		if (Request.Category != View.Presentation.Interaction.Category || !View.Presentation.Interaction.bCanDecline) return false;
		switch (View.DeclineAction)
		{
		case EFMCodexNetworkDeclineAction::Runner: return Client.BeginDecline(View, N::DeclineRunner, Out);
		case EFMCodexNetworkDeclineAction::Helper: return Client.BeginDecline(View, N::DeclineHelper, Out);
		case EFMCodexNetworkDeclineAction::Skill: return Client.BeginDecline(View, N::DeclineSkill, Out);
		case EFMCodexNetworkDeclineAction::Marker: return Client.BeginDecline(View, N::DeclineMarker, Out);
		default: return false;
		}
	case K::OneOnOne:
		switch (Request.OneOnOne)
		{
		case EFMCodexUMGOneOnOneChoice::DirectShot: return Client.BeginOneOnOne(View, EMatchPlayThroughBallOneOnOneShotChoice::DirectShot, Out);
		case EFMCodexUMGOneOnOneChoice::ChipShot: return Client.BeginOneOnOne(View, EMatchPlayThroughBallOneOnOneShotChoice::ChipShot, Out);
		default: return false;
		}
	case K::Continue:
		if (Request.Category != View.Presentation.Interaction.Category) return false;
		switch (Request.Category)
		{
		case C::RollShortFreeKickDirectAttack: return Client.BeginSetPiece(View, N::ResolveShortFreeKickDirectAttackRoll, Out);
		case C::RollShortFreeKickDirectDefense: return Client.BeginSetPiece(View, N::ResolveShortFreeKickDirectDefenseRoll, Out);
		case C::RollShortFreeKickAngled: return Client.BeginSetPiece(View, N::ResolveShortFreeKickAngledRoll, Out);
		case C::RollLongFreeKickDirectAttack: return Client.BeginSetPiece(View, N::ResolveLongFreeKickDirectAttackRoll, Out);
		case C::RollLongFreeKickDirectDefense: return Client.BeginSetPiece(View, N::ResolveLongFreeKickDirectDefenseRoll, Out);
		case C::RollLongFreeKickPower: return Client.BeginSetPiece(View, N::ResolveLongFreeKickPowerRoll, Out);
		case C::RollLongShotDirectAttack: return Client.BeginOrdinaryContest(View, N::LongShotDirectAttackRoll, Out);
		case C::RollLongShotDirectDefense: return Client.BeginOrdinaryContest(View, N::LongShotDirectDefenseRoll, Out);
		case C::RollLongShotDeadCorner: return Client.BeginOrdinaryContest(View, N::LongShotDeadCornerRoll, Out);
		case C::RollCutInsideShotDirectAttack: return Client.BeginOrdinaryContest(View, N::CutInsideShotDirectAttackRoll, Out);
		case C::RollCutInsideShotDirectDefense: return Client.BeginOrdinaryContest(View, N::CutInsideShotDirectDefenseRoll, Out);
		case C::RollCutInsideShotDeadCorner: return Client.BeginOrdinaryContest(View, N::CutInsideShotDeadCornerRoll, Out);
		case C::RollPassControlRoute: return Client.BeginInitialRoute(View, N::PassControlInitialRouteRoll, Out);
		case C::RollThroughBallInitialRoute: return Client.BeginInitialRoute(View, N::ThroughBallInitialRouteRoll, Out);
		case C::RollCrossRoute: return Client.BeginInitialRoute(View, N::CrossInitialRouteRoll, Out);
		case C::AdvanceAfterTerminal: return Client.BeginAdvance(View, Out);
		case C::RollPassControlAttack: return Client.BeginOrdinaryContest(View, N::PassControlAttackRoll, Out);
		case C::RollPassControlDefense: return Client.BeginOrdinaryContest(View, N::PassControlDefenseRoll, Out);
		case C::RollThroughBallFeetAttack: return Client.BeginOrdinaryContest(View, N::ThroughBallFeetAttackRoll, Out);
		case C::RollThroughBallFeetDefense: return Client.BeginOrdinaryContest(View, N::ThroughBallFeetDefenseRoll, Out);
		case C::RollThroughBallBehindDefenseAttack: return Client.BeginOrdinaryContest(View, N::ThroughBallBehindDefenseP1AttackRoll, Out);
		case C::RollThroughBallBehindDefenseDefense: return Client.BeginOrdinaryContest(View, N::ThroughBallBehindDefenseP1DefenseRoll, Out);
		case C::RollThroughBallAntiOffsideAttack: return Client.BeginOrdinaryContest(View, N::ThroughBallAntiOffsideAttackRoll, Out);
		case C::RollThroughBallOneOnOneDirectShotAttack: return Client.BeginOrdinaryContest(View, N::ThroughBallOneOnOneDirectShotAttackRoll, Out);
		case C::RollThroughBallOneOnOneDirectShotDefense: return Client.BeginOrdinaryContest(View, N::ThroughBallOneOnOneDirectShotDefenseRoll, Out);
		case C::RollThroughBallOneOnOneChipShotAttack: return Client.BeginOrdinaryContest(View, N::ThroughBallOneOnOneChipShotAttackRoll, Out);
		case C::RollCrossAttack:
		case C::RollCrossDefense:
			switch (View.ContestAction)
			{
			case EFMCodexNetworkContestAction::CrossHighAttackRoll: return Client.BeginOrdinaryContest(View, N::CrossHighAttackRoll, Out);
			case EFMCodexNetworkContestAction::CrossHighDefenseRoll: return Client.BeginOrdinaryContest(View, N::CrossHighDefenseRoll, Out);
			case EFMCodexNetworkContestAction::CrossLowAttackRoll: return Client.BeginOrdinaryContest(View, N::CrossLowAttackRoll, Out);
			case EFMCodexNetworkContestAction::CrossLowDefenseRoll: return Client.BeginOrdinaryContest(View, N::CrossLowDefenseRoll, Out);
			default: return false;
			}
		default: return false;
		}
	default: return false; // Local Start, internal no-legal and unnetworked families have no transport capability.
	}
}
