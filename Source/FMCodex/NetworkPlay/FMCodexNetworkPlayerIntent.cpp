#include "FMCodexNetworkPlayerIntent.h"

EFMCodexNetworkIntentAckCode FFMCodexNetworkPlayerIntentEnvelope::ValidatePayloadShape() const
{
	using Code = EFMCodexNetworkIntentAckCode;
 using K = EFMCodexNetworkPlayerIntentKind;
 const bool LegacyEmpty = Deployment.IsEmpty() && Goalkeeper.IsEmpty() && Carrier.IsEmpty() && Marker.IsEmpty()
  && Runner.IsEmpty() && Helper.IsEmpty() && Skill.IsEmpty() && Branch.IsEmpty()
  && OneOnOneChoice == EMatchPlayThroughBallOneOnOneShotChoice::None;
 const bool CardEmpty = SetPieceCardId.IsNone();
 const bool NearEmpty = NearMethod == EMatchPlayShortFreeKickMethod::None;
 const bool LongEmpty = LongMethod == EMatchPlayLongFreeKickMethod::None;
 const bool PenaltyEmpty = PenaltyMethod == EMatchPlayPenaltyMethod::None;
 switch (IntentKind)
 {
 case K::ResolveShortFreeKickDirectAttackRoll:
 case K::ResolveShortFreeKickDirectDefenseRoll:
 case K::ResolveShortFreeKickAngledRoll:
 case K::ResolveLongFreeKickDirectAttackRoll:
 case K::ResolveLongFreeKickDirectDefenseRoll:
 case K::ResolveLongFreeKickPowerRoll:
 case K::ResolvePenaltyDirectAttackRoll:
 case K::ResolvePenaltyDirectDefenseRoll:
 case K::ResolvePenaltyPanenkaRoll:
 case K::RequestSetPieceTypeRoll: return LegacyEmpty && CardEmpty && NearEmpty && LongEmpty && PenaltyEmpty ? Code::None : Code::InvalidPayload;
 case K::SubmitSetPieceCarrier: return LegacyEmpty && !CardEmpty && SetPieceCardId.ToString().Len() <= 128 && NearEmpty && LongEmpty && PenaltyEmpty ? Code::None : Code::InvalidPayload;
 case K::SubmitShortFreeKickMethod: return LegacyEmpty && CardEmpty && LongEmpty && PenaltyEmpty
  && (NearMethod == EMatchPlayShortFreeKickMethod::Direct || NearMethod == EMatchPlayShortFreeKickMethod::Angled) ? Code::None : Code::InvalidPayload;
 case K::SubmitLongFreeKickMethod: return LegacyEmpty && CardEmpty && NearEmpty && PenaltyEmpty
  && (LongMethod == EMatchPlayLongFreeKickMethod::Direct || LongMethod == EMatchPlayLongFreeKickMethod::Power) ? Code::None : Code::InvalidPayload;
 case K::SubmitPenaltyMethod: return LegacyEmpty && CardEmpty && NearEmpty && LongEmpty
  && (PenaltyMethod == EMatchPlayPenaltyMethod::Direct || PenaltyMethod == EMatchPlayPenaltyMethod::Panenka) ? Code::None : Code::InvalidPayload;
 default: break;
 }
 if (IntentKind != K::None && int32(IntentKind) <= int32(K::DeclineMarker)
  && !(CardEmpty && NearEmpty && LongEmpty && PenaltyEmpty)) return Code::InvalidPayload;

	switch (IntentKind)
	{
	case EFMCodexNetworkPlayerIntentKind::DeclineMarker:
	case EFMCodexNetworkPlayerIntentKind::DeclineRunner:
	case EFMCodexNetworkPlayerIntentKind::DeclineHelper:
	case EFMCodexNetworkPlayerIntentKind::DeclineSkill:
	case EFMCodexNetworkPlayerIntentKind::RequestInitialActionPointRoll:
	case EFMCodexNetworkPlayerIntentKind::FinishDeployment:
	case EFMCodexNetworkPlayerIntentKind::CrossInitialRouteRoll:
	case EFMCodexNetworkPlayerIntentKind::PassControlInitialRouteRoll:
	case EFMCodexNetworkPlayerIntentKind::ThroughBallInitialRouteRoll:
	case EFMCodexNetworkPlayerIntentKind::PassControlAttackRoll:
	case EFMCodexNetworkPlayerIntentKind::PassControlDefenseRoll:
	case EFMCodexNetworkPlayerIntentKind::ThroughBallFeetAttackRoll:
	case EFMCodexNetworkPlayerIntentKind::ThroughBallFeetDefenseRoll:
	case EFMCodexNetworkPlayerIntentKind::CrossHighAttackRoll:
	case EFMCodexNetworkPlayerIntentKind::CrossHighDefenseRoll:
	case EFMCodexNetworkPlayerIntentKind::CrossLowAttackRoll:
	case EFMCodexNetworkPlayerIntentKind::AdvanceAfterTerminal:
	case EFMCodexNetworkPlayerIntentKind::ThroughBallBehindDefenseP1AttackRoll:
	case EFMCodexNetworkPlayerIntentKind::ThroughBallBehindDefenseP1DefenseRoll:
	case EFMCodexNetworkPlayerIntentKind::ThroughBallAntiOffsideAttackRoll:
	case EFMCodexNetworkPlayerIntentKind::ThroughBallOneOnOneDirectShotAttackRoll:
	case EFMCodexNetworkPlayerIntentKind::ThroughBallOneOnOneDirectShotDefenseRoll:
	case EFMCodexNetworkPlayerIntentKind::ThroughBallOneOnOneChipShotAttackRoll:
	case EFMCodexNetworkPlayerIntentKind::LongShotDirectAttackRoll:
	case EFMCodexNetworkPlayerIntentKind::LongShotDirectDefenseRoll:
	case EFMCodexNetworkPlayerIntentKind::LongShotDeadCornerRoll:
	case EFMCodexNetworkPlayerIntentKind::CutInsideShotDirectAttackRoll:
	case EFMCodexNetworkPlayerIntentKind::CutInsideShotDirectDefenseRoll:
	case EFMCodexNetworkPlayerIntentKind::CutInsideShotDeadCornerRoll:
	case EFMCodexNetworkPlayerIntentKind::CrossLowDefenseRoll:
		return Deployment.IsEmpty() && Goalkeeper.IsEmpty() && Carrier.IsEmpty() && Marker.IsEmpty() && Runner.IsEmpty() && Helper.IsEmpty() && Skill.IsEmpty() && Branch.IsEmpty() && OneOnOneChoice == EMatchPlayThroughBallOneOnOneShotChoice::None ? Code::None : Code::InvalidPayload;
	case EFMCodexNetworkPlayerIntentKind::DeployOrdinary:
		return Deployment.IsValidShape() && Goalkeeper.IsEmpty() && Carrier.IsEmpty() && Marker.IsEmpty() && Runner.IsEmpty() && Helper.IsEmpty() && Skill.IsEmpty() && Branch.IsEmpty() && OneOnOneChoice == EMatchPlayThroughBallOneOnOneShotChoice::None ? Code::None : Code::InvalidPayload;
	case EFMCodexNetworkPlayerIntentKind::DeployGoalkeeper:
		return Deployment.IsEmpty() && Goalkeeper.IsValidShape() && Carrier.IsEmpty() && Marker.IsEmpty() && Runner.IsEmpty() && Helper.IsEmpty() && Skill.IsEmpty() && Branch.IsEmpty() && OneOnOneChoice == EMatchPlayThroughBallOneOnOneShotChoice::None ? Code::None : Code::InvalidPayload;
	case EFMCodexNetworkPlayerIntentKind::SubmitCarrier:
		return Deployment.IsEmpty() && Goalkeeper.IsEmpty() && Carrier.IsValidShape() && Marker.IsEmpty() && Runner.IsEmpty() && Helper.IsEmpty() && Skill.IsEmpty() && Branch.IsEmpty() && OneOnOneChoice == EMatchPlayThroughBallOneOnOneShotChoice::None ? Code::None : Code::InvalidPayload;
	case EFMCodexNetworkPlayerIntentKind::SubmitMarker:
		return Deployment.IsEmpty() && Goalkeeper.IsEmpty() && Carrier.IsEmpty() && Marker.IsValidShape() && Runner.IsEmpty() && Helper.IsEmpty() && Skill.IsEmpty() && Branch.IsEmpty() && OneOnOneChoice == EMatchPlayThroughBallOneOnOneShotChoice::None ? Code::None : Code::InvalidPayload;
	case EFMCodexNetworkPlayerIntentKind::SubmitRunner:
		return Deployment.IsEmpty() && Goalkeeper.IsEmpty() && Carrier.IsEmpty() && Marker.IsEmpty() && Runner.IsValidShape() && Helper.IsEmpty() && Skill.IsEmpty() && Branch.IsEmpty() && OneOnOneChoice == EMatchPlayThroughBallOneOnOneShotChoice::None ? Code::None : Code::InvalidPayload;
	case EFMCodexNetworkPlayerIntentKind::SubmitHelper:
		return Deployment.IsEmpty() && Goalkeeper.IsEmpty() && Carrier.IsEmpty() && Marker.IsEmpty() && Runner.IsEmpty() && Helper.IsValidShape() && Skill.IsEmpty() && Branch.IsEmpty() && OneOnOneChoice == EMatchPlayThroughBallOneOnOneShotChoice::None ? Code::None : Code::InvalidPayload;
	case EFMCodexNetworkPlayerIntentKind::SubmitSkill:
		return Deployment.IsEmpty() && Goalkeeper.IsEmpty() && Carrier.IsEmpty() && Marker.IsEmpty() && Runner.IsEmpty() && Helper.IsEmpty() && Skill.IsValidShape() && Branch.IsEmpty() && OneOnOneChoice == EMatchPlayThroughBallOneOnOneShotChoice::None ? Code::None : Code::InvalidPayload;
	case EFMCodexNetworkPlayerIntentKind::SubmitBranchIntent:
		return Deployment.IsEmpty() && Goalkeeper.IsEmpty() && Carrier.IsEmpty() && Marker.IsEmpty() && Runner.IsEmpty() && Helper.IsEmpty() && Skill.IsEmpty() && Branch.IsValidShape() && OneOnOneChoice == EMatchPlayThroughBallOneOnOneShotChoice::None ? Code::None : Code::InvalidPayload;
	case EFMCodexNetworkPlayerIntentKind::SubmitThroughBallOneOnOneShotChoice:
		return Deployment.IsEmpty() && Goalkeeper.IsEmpty() && Carrier.IsEmpty() && Marker.IsEmpty()
			&& Runner.IsEmpty() && Helper.IsEmpty() && Skill.IsEmpty() && Branch.IsEmpty()
			&& (OneOnOneChoice == EMatchPlayThroughBallOneOnOneShotChoice::DirectShot
				|| OneOnOneChoice == EMatchPlayThroughBallOneOnOneShotChoice::ChipShot) ? Code::None : Code::InvalidPayload;
	default:
		return Code::NotPlayerIntent;
	}
}
EFMCodexNetworkIntentAckCode FFMCodexNetworkIntentLedger::Check(
	const FGuid& ServerMatch, const FFMCodexNetworkPlayerIntentEnvelope& Envelope) const
{
	using Code = EFMCodexNetworkIntentAckCode;
	if (!ServerMatch.IsValid() || Envelope.MatchInstanceId != ServerMatch) { return Code::MatchMismatch; }
	if (Envelope.RequestId <= 0) { return Code::InvalidPayload; }
	const int64 HighWater = Match == ServerMatch ? HighestRequestId : 0;
	if (Envelope.RequestId <= HighWater) { return Code::DuplicateOrAlreadyResolved; }
	// Both operands are nonnegative and RequestId > HighWater: subtraction cannot overflow.
	if (Envelope.RequestId - HighWater > MaxForwardDelta) { return Code::InvalidPayload; }
	return Code::None;
}

bool FFMCodexNetworkIntentLedger::Consume(const FGuid& ServerMatch,
	const FFMCodexNetworkPlayerIntentEnvelope& Envelope)
{
	if (Check(ServerMatch, Envelope) != EFMCodexNetworkIntentAckCode::None)
	{
		return false;
	}
	if (Match != ServerMatch)
	{
		Match = ServerMatch;
		HighestRequestId = 0;
	}
	if (Envelope.RequestId <= HighestRequestId)
	{
		return false;
	}
	HighestRequestId = Envelope.RequestId;
	return true;
}

void FFMCodexNetworkIntentClientState::ObserveView(const FFMCodexNetworkClientViewSnapshot& View)
{
	if (!View.MatchInstanceId.IsValid())
	{
		return;
	}
	if (Match != View.MatchInstanceId)
	{
		Match = View.MatchInstanceId;
		PendingRequestId = 0;
		SeenViewRevision = 0;
		LastAck = {};
		// IDs stay monotonic for this controller's lifetime, including new matches.
	}
	SeenViewRevision = FMath::Max(SeenViewRevision, View.ViewRevision);
	CompleteIfReady();
}

bool FFMCodexNetworkIntentClientState::Begin(const FFMCodexNetworkClientViewSnapshot& View,
	FFMCodexNetworkPlayerIntentEnvelope& OutEnvelope)
{
	return BeginIntent(View, EFMCodexNetworkPlayerIntentKind::RequestInitialActionPointRoll, {}, {}, {}, {}, {}, {}, {}, {}, OutEnvelope);
}

bool FFMCodexNetworkIntentClientState::BeginDeployment(const FFMCodexNetworkClientViewSnapshot& View,
	const FFMCodexNetworkDeployOrdinaryPayload& Choice, FFMCodexNetworkPlayerIntentEnvelope& OutEnvelope)
{
	return BeginIntent(View, EFMCodexNetworkPlayerIntentKind::DeployOrdinary, Choice, {}, {}, {}, {}, {}, {}, {}, OutEnvelope);
}

bool FFMCodexNetworkIntentClientState::BeginGoalkeeper(const FFMCodexNetworkClientViewSnapshot& View,
	const FFMCodexNetworkDeployGoalkeeperPayload& Choice, FFMCodexNetworkPlayerIntentEnvelope& OutEnvelope)
{
	return BeginIntent(View, EFMCodexNetworkPlayerIntentKind::DeployGoalkeeper, {}, Choice, {}, {}, {}, {}, {}, {}, OutEnvelope);
}
bool FFMCodexNetworkIntentClientState::BeginFinishDeployment(const FFMCodexNetworkClientViewSnapshot& View,
	FFMCodexNetworkPlayerIntentEnvelope& OutEnvelope)
{
	return BeginIntent(View, EFMCodexNetworkPlayerIntentKind::FinishDeployment, {}, {}, {}, {}, {}, {}, {}, {}, OutEnvelope);
}
bool FFMCodexNetworkIntentClientState::BeginCarrier(const FFMCodexNetworkClientViewSnapshot& View,
	const FFMCodexNetworkSubmitCarrierPayload& Choice, FFMCodexNetworkPlayerIntentEnvelope& OutEnvelope)
{
	return BeginIntent(View, EFMCodexNetworkPlayerIntentKind::SubmitCarrier, {}, {}, Choice, {}, {}, {}, {}, {}, OutEnvelope);
}
bool FFMCodexNetworkIntentClientState::BeginMarker(const FFMCodexNetworkClientViewSnapshot& View,
	const FFMCodexNetworkSubmitMarkerPayload& Choice, FFMCodexNetworkPlayerIntentEnvelope& OutEnvelope)
{
	return BeginIntent(View, EFMCodexNetworkPlayerIntentKind::SubmitMarker, {}, {}, {}, Choice, {}, {}, {}, {}, OutEnvelope);
}
bool FFMCodexNetworkIntentClientState::BeginRunner(const FFMCodexNetworkClientViewSnapshot& View,
	const FFMCodexNetworkSubmitRunnerPayload& Choice, FFMCodexNetworkPlayerIntentEnvelope& OutEnvelope)
{
	return BeginIntent(View, EFMCodexNetworkPlayerIntentKind::SubmitRunner, {}, {}, {}, {}, Choice, {}, {}, {}, OutEnvelope);
}
bool FFMCodexNetworkIntentClientState::BeginHelper(const FFMCodexNetworkClientViewSnapshot& View,
	const FFMCodexNetworkSubmitHelperPayload& Choice, FFMCodexNetworkPlayerIntentEnvelope& OutEnvelope)
{
	return BeginIntent(View, EFMCodexNetworkPlayerIntentKind::SubmitHelper, {}, {}, {}, {}, {}, Choice, {}, {}, OutEnvelope);
}
bool FFMCodexNetworkIntentClientState::BeginSkill(const FFMCodexNetworkClientViewSnapshot& View,
	const FFMCodexNetworkSubmitSkillPayload& Choice, FFMCodexNetworkPlayerIntentEnvelope& OutEnvelope)
{
	return BeginIntent(View, EFMCodexNetworkPlayerIntentKind::SubmitSkill, {}, {}, {}, {}, {}, {}, Choice, {}, OutEnvelope);
}
bool FFMCodexNetworkIntentClientState::BeginBranch(const FFMCodexNetworkClientViewSnapshot& View,
	const FFMCodexNetworkSubmitBranchIntentPayload& Choice, FFMCodexNetworkPlayerIntentEnvelope& OutEnvelope)
{
	return BeginIntent(View, EFMCodexNetworkPlayerIntentKind::SubmitBranchIntent, {}, {}, {}, {}, {}, {}, {}, Choice, OutEnvelope);
}
bool FFMCodexNetworkIntentClientState::BeginInitialRoute(const FFMCodexNetworkClientViewSnapshot& View,
	EFMCodexNetworkPlayerIntentKind Kind, FFMCodexNetworkPlayerIntentEnvelope& OutEnvelope)
{
	if (Kind != EFMCodexNetworkPlayerIntentKind::CrossInitialRouteRoll
		&& Kind != EFMCodexNetworkPlayerIntentKind::PassControlInitialRouteRoll
		&& Kind != EFMCodexNetworkPlayerIntentKind::ThroughBallInitialRouteRoll) { return false; }
	return BeginIntent(View, Kind, {}, {}, {}, {}, {}, {}, {}, {}, OutEnvelope);
}
bool FFMCodexNetworkIntentClientState::BeginAdvance(const FFMCodexNetworkClientViewSnapshot& View,
	FFMCodexNetworkPlayerIntentEnvelope& OutEnvelope)
{
	return BeginIntent(View, EFMCodexNetworkPlayerIntentKind::AdvanceAfterTerminal, {}, {}, {}, {}, {}, {}, {}, {}, OutEnvelope);
}
bool FFMCodexNetworkIntentClientState::BeginOrdinaryContest(const FFMCodexNetworkClientViewSnapshot& View,
	EFMCodexNetworkPlayerIntentKind Kind, FFMCodexNetworkPlayerIntentEnvelope& OutEnvelope)
{
	if (Kind != EFMCodexNetworkPlayerIntentKind::CrossHighAttackRoll
		&& Kind != EFMCodexNetworkPlayerIntentKind::CrossHighDefenseRoll
		&& Kind != EFMCodexNetworkPlayerIntentKind::CrossLowAttackRoll
		&& Kind != EFMCodexNetworkPlayerIntentKind::CrossLowDefenseRoll
		&& Kind != EFMCodexNetworkPlayerIntentKind::PassControlAttackRoll
		&& Kind != EFMCodexNetworkPlayerIntentKind::PassControlDefenseRoll
		&& Kind != EFMCodexNetworkPlayerIntentKind::ThroughBallFeetAttackRoll
		&& Kind != EFMCodexNetworkPlayerIntentKind::ThroughBallFeetDefenseRoll
		&& Kind != EFMCodexNetworkPlayerIntentKind::ThroughBallBehindDefenseP1AttackRoll
		&& Kind != EFMCodexNetworkPlayerIntentKind::ThroughBallBehindDefenseP1DefenseRoll
		&& Kind != EFMCodexNetworkPlayerIntentKind::ThroughBallAntiOffsideAttackRoll
		&& Kind != EFMCodexNetworkPlayerIntentKind::ThroughBallOneOnOneDirectShotAttackRoll
		&& Kind != EFMCodexNetworkPlayerIntentKind::ThroughBallOneOnOneDirectShotDefenseRoll
		&& Kind != EFMCodexNetworkPlayerIntentKind::ThroughBallOneOnOneChipShotAttackRoll
		&& Kind != EFMCodexNetworkPlayerIntentKind::LongShotDirectAttackRoll
		&& Kind != EFMCodexNetworkPlayerIntentKind::LongShotDirectDefenseRoll
		&& Kind != EFMCodexNetworkPlayerIntentKind::LongShotDeadCornerRoll
		&& Kind != EFMCodexNetworkPlayerIntentKind::CutInsideShotDirectAttackRoll
		&& Kind != EFMCodexNetworkPlayerIntentKind::CutInsideShotDirectDefenseRoll
		&& Kind != EFMCodexNetworkPlayerIntentKind::CutInsideShotDeadCornerRoll
) { return false; }
	return BeginIntent(View, Kind, {}, {}, {}, {}, {}, {}, {}, {}, OutEnvelope);
}
bool FFMCodexNetworkIntentClientState::BeginDecline(const FFMCodexNetworkClientViewSnapshot& View,
	EFMCodexNetworkPlayerIntentKind Kind, FFMCodexNetworkPlayerIntentEnvelope& OutEnvelope)
{
	if (Kind != EFMCodexNetworkPlayerIntentKind::DeclineRunner
		&& Kind != EFMCodexNetworkPlayerIntentKind::DeclineHelper
		&& Kind != EFMCodexNetworkPlayerIntentKind::DeclineSkill
		&& Kind != EFMCodexNetworkPlayerIntentKind::DeclineMarker) return false;
	return BeginIntent(View, Kind, {}, {}, {}, {}, {}, {}, {}, {}, OutEnvelope);
}
bool FFMCodexNetworkIntentClientState::BeginOneOnOne(const FFMCodexNetworkClientViewSnapshot& View,
	EMatchPlayThroughBallOneOnOneShotChoice Choice, FFMCodexNetworkPlayerIntentEnvelope& OutEnvelope)
{
	if (!View.OneOnOneOptions.Contains(Choice)) return false;
	return BeginIntent(View, EFMCodexNetworkPlayerIntentKind::SubmitThroughBallOneOnOneShotChoice,
		{}, {}, {}, {}, {}, {}, {}, {}, OutEnvelope, Choice);
}
bool FFMCodexNetworkIntentClientState::BeginIntent(const FFMCodexNetworkClientViewSnapshot& View,
	EFMCodexNetworkPlayerIntentKind Kind, const FFMCodexNetworkDeployOrdinaryPayload& Choice,
	const FFMCodexNetworkDeployGoalkeeperPayload& GoalkeeperChoice,
	const FFMCodexNetworkSubmitCarrierPayload& CarrierChoice,
	const FFMCodexNetworkSubmitMarkerPayload& MarkerChoice,
	const FFMCodexNetworkSubmitRunnerPayload& RunnerChoice,
	const FFMCodexNetworkSubmitHelperPayload& HelperChoice,
	const FFMCodexNetworkSubmitSkillPayload& SkillChoice,
	const FFMCodexNetworkSubmitBranchIntentPayload& BranchChoice,
	FFMCodexNetworkPlayerIntentEnvelope& OutEnvelope,
	EMatchPlayThroughBallOneOnOneShotChoice OneOnOneChoice, FName SetPieceCardId,
 EMatchPlayShortFreeKickMethod NearMethod, EMatchPlayLongFreeKickMethod LongMethod, EMatchPlayPenaltyMethod PenaltyMethod)
{
	ObserveView(View);
	bool bActionable = false;
	switch (Kind)
	{
 case EFMCodexNetworkPlayerIntentKind::RequestSetPieceTypeRoll: bActionable = View.SetPiece.bCanRollType; break;
 case EFMCodexNetworkPlayerIntentKind::SubmitSetPieceCarrier: bActionable = !View.SetPiece.bOptionsUnavailable && View.SetPiece.TakerOptions.Contains(SetPieceCardId); break;
 case EFMCodexNetworkPlayerIntentKind::SubmitShortFreeKickMethod: bActionable = View.SetPiece.NearMethods.Contains(NearMethod); break;
 case EFMCodexNetworkPlayerIntentKind::SubmitLongFreeKickMethod: bActionable = View.SetPiece.LongMethods.Contains(LongMethod); break;
 case EFMCodexNetworkPlayerIntentKind::SubmitPenaltyMethod: bActionable = View.SetPiece.PenaltyMethods.Contains(PenaltyMethod); break;
	case EFMCodexNetworkPlayerIntentKind::DeclineRunner:
		bActionable = View.DeclineAction == EFMCodexNetworkDeclineAction::Runner;
		break;
	case EFMCodexNetworkPlayerIntentKind::DeclineHelper:
		bActionable = View.DeclineAction == EFMCodexNetworkDeclineAction::Helper;
		break;
	case EFMCodexNetworkPlayerIntentKind::DeclineSkill:
		bActionable = View.DeclineAction == EFMCodexNetworkDeclineAction::Skill;
		break;
	case EFMCodexNetworkPlayerIntentKind::DeclineMarker:
		bActionable = View.DeclineAction == EFMCodexNetworkDeclineAction::Marker;
		break;
	case EFMCodexNetworkPlayerIntentKind::RequestInitialActionPointRoll:
		bActionable = View.InteractionState == EFMCodexNetworkClientInteractionState::WaitingForOwnInitialActionPoint;
		break;
	case EFMCodexNetworkPlayerIntentKind::DeployOrdinary:
		bActionable = View.EntryBranch == EFMCodexNetworkEntryBranch::Ordinary
			&& View.EntryWait == EFMCodexNetworkEntryWait::Deployment && !View.DeploymentOptions.IsEmpty();
		break;
	case EFMCodexNetworkPlayerIntentKind::DeployGoalkeeper:
		bActionable = View.bCanDeployGoalkeeper;
		break;
	case EFMCodexNetworkPlayerIntentKind::FinishDeployment:
		bActionable = View.bCanFinishDeployment;
		break;
	case EFMCodexNetworkPlayerIntentKind::SubmitCarrier:
		bActionable = View.EntryWait == EFMCodexNetworkEntryWait::CarrierSelection
			&& !View.bCarrierOptionsUnavailable && !View.CarrierOptions.IsEmpty();
		break;
	case EFMCodexNetworkPlayerIntentKind::SubmitMarker:
		bActionable = View.EntryWait == EFMCodexNetworkEntryWait::MarkerSelection
			&& !View.bMarkerOptionsUnavailable && !View.MarkerOptions.IsEmpty();
		break;
	case EFMCodexNetworkPlayerIntentKind::SubmitRunner:
		bActionable = View.EntryWait == EFMCodexNetworkEntryWait::RunnerSelection
			&& !View.bRunnerOptionsUnavailable && !View.RunnerOptions.IsEmpty();
		break;
	case EFMCodexNetworkPlayerIntentKind::SubmitHelper:
		bActionable = View.EntryWait == EFMCodexNetworkEntryWait::HelperSelection
			&& !View.bHelperOptionsUnavailable && !View.HelperOptions.IsEmpty();
		break;
	case EFMCodexNetworkPlayerIntentKind::SubmitSkill:
		bActionable = View.EntryWait == EFMCodexNetworkEntryWait::SkillSelection
			&& !View.bSkillOptionsUnavailable && !View.SkillOptions.IsEmpty();
		break;
	case EFMCodexNetworkPlayerIntentKind::SubmitBranchIntent:
		bActionable = View.EntryWait == EFMCodexNetworkEntryWait::BranchIntentSelection
			&& !View.bBranchOptionsUnavailable && !View.BranchOptions.IsEmpty();
		break;
	case EFMCodexNetworkPlayerIntentKind::CrossInitialRouteRoll:
		bActionable = View.InitialRouteAction == EFMCodexNetworkInitialRouteAction::Cross;
		break;
	case EFMCodexNetworkPlayerIntentKind::PassControlInitialRouteRoll:
		bActionable = View.InitialRouteAction == EFMCodexNetworkInitialRouteAction::PassControl;
		break;
	case EFMCodexNetworkPlayerIntentKind::ThroughBallInitialRouteRoll:
		bActionable = View.InitialRouteAction == EFMCodexNetworkInitialRouteAction::ThroughBall;
		break;
	case EFMCodexNetworkPlayerIntentKind::ResolveShortFreeKickDirectAttackRoll:
		bActionable = View.ContestAction == EFMCodexNetworkContestAction::ResolveShortFreeKickDirectAttackRoll;
		break;
	case EFMCodexNetworkPlayerIntentKind::ResolveShortFreeKickDirectDefenseRoll:
		bActionable = View.ContestAction == EFMCodexNetworkContestAction::ResolveShortFreeKickDirectDefenseRoll;
		break;
	case EFMCodexNetworkPlayerIntentKind::ResolveShortFreeKickAngledRoll:
		bActionable = View.ContestAction == EFMCodexNetworkContestAction::ResolveShortFreeKickAngledRoll;
		break;
	case EFMCodexNetworkPlayerIntentKind::ResolveLongFreeKickDirectAttackRoll:
		bActionable = View.ContestAction == EFMCodexNetworkContestAction::ResolveLongFreeKickDirectAttackRoll;
		break;
	case EFMCodexNetworkPlayerIntentKind::ResolveLongFreeKickDirectDefenseRoll:
		bActionable = View.ContestAction == EFMCodexNetworkContestAction::ResolveLongFreeKickDirectDefenseRoll;
		break;
	case EFMCodexNetworkPlayerIntentKind::ResolveLongFreeKickPowerRoll:
		bActionable = View.ContestAction == EFMCodexNetworkContestAction::ResolveLongFreeKickPowerRoll;
		break;
	case EFMCodexNetworkPlayerIntentKind::ResolvePenaltyDirectAttackRoll:
		bActionable = View.ContestAction == EFMCodexNetworkContestAction::ResolvePenaltyDirectAttackRoll;
		break;
	case EFMCodexNetworkPlayerIntentKind::ResolvePenaltyDirectDefenseRoll:
		bActionable = View.ContestAction == EFMCodexNetworkContestAction::ResolvePenaltyDirectDefenseRoll;
		break;
	case EFMCodexNetworkPlayerIntentKind::ResolvePenaltyPanenkaRoll:
		bActionable = View.ContestAction == EFMCodexNetworkContestAction::ResolvePenaltyPanenkaRoll;
		break;
	case EFMCodexNetworkPlayerIntentKind::CrossHighAttackRoll:
		bActionable = View.ContestAction == EFMCodexNetworkContestAction::CrossHighAttackRoll;
		break;
	case EFMCodexNetworkPlayerIntentKind::CrossHighDefenseRoll:
		bActionable = View.ContestAction == EFMCodexNetworkContestAction::CrossHighDefenseRoll;
		break;
	case EFMCodexNetworkPlayerIntentKind::CrossLowAttackRoll:
		bActionable = View.ContestAction == EFMCodexNetworkContestAction::CrossLowAttackRoll;
		break;
	case EFMCodexNetworkPlayerIntentKind::CrossLowDefenseRoll:
		bActionable = View.ContestAction == EFMCodexNetworkContestAction::CrossLowDefenseRoll;
		break;
	case EFMCodexNetworkPlayerIntentKind::PassControlAttackRoll:
		bActionable = View.ContestAction == EFMCodexNetworkContestAction::PassControlAttackRoll;
		break;
	case EFMCodexNetworkPlayerIntentKind::PassControlDefenseRoll:
		bActionable = View.ContestAction == EFMCodexNetworkContestAction::PassControlDefenseRoll;
		break;
	case EFMCodexNetworkPlayerIntentKind::ThroughBallFeetAttackRoll:
		bActionable = View.ContestAction == EFMCodexNetworkContestAction::ThroughBallFeetAttackRoll;
		break;
	case EFMCodexNetworkPlayerIntentKind::ThroughBallFeetDefenseRoll:
		bActionable = View.ContestAction == EFMCodexNetworkContestAction::ThroughBallFeetDefenseRoll;
		break;
	case EFMCodexNetworkPlayerIntentKind::ThroughBallBehindDefenseP1AttackRoll:
		bActionable = View.ContestAction == EFMCodexNetworkContestAction::ThroughBallBehindDefenseP1AttackRoll;
		break;
	case EFMCodexNetworkPlayerIntentKind::ThroughBallBehindDefenseP1DefenseRoll:
		bActionable = View.ContestAction == EFMCodexNetworkContestAction::ThroughBallBehindDefenseP1DefenseRoll;
		break;
	case EFMCodexNetworkPlayerIntentKind::ThroughBallAntiOffsideAttackRoll:
		bActionable = View.ContestAction == EFMCodexNetworkContestAction::ThroughBallAntiOffsideAttackRoll;
		break;
	case EFMCodexNetworkPlayerIntentKind::ThroughBallOneOnOneDirectShotAttackRoll:
		bActionable = View.ContestAction == EFMCodexNetworkContestAction::ThroughBallOneOnOneDirectShotAttackRoll;
		break;
	case EFMCodexNetworkPlayerIntentKind::ThroughBallOneOnOneDirectShotDefenseRoll:
		bActionable = View.ContestAction == EFMCodexNetworkContestAction::ThroughBallOneOnOneDirectShotDefenseRoll;
		break;
	case EFMCodexNetworkPlayerIntentKind::ThroughBallOneOnOneChipShotAttackRoll:
		bActionable = View.ContestAction == EFMCodexNetworkContestAction::ThroughBallOneOnOneChipShotAttackRoll;
		break;
	case EFMCodexNetworkPlayerIntentKind::LongShotDirectAttackRoll:
		bActionable = View.ContestAction == EFMCodexNetworkContestAction::LongShotDirectAttackRoll;
		break;
	case EFMCodexNetworkPlayerIntentKind::LongShotDirectDefenseRoll:
		bActionable = View.ContestAction == EFMCodexNetworkContestAction::LongShotDirectDefenseRoll;
		break;
	case EFMCodexNetworkPlayerIntentKind::LongShotDeadCornerRoll:
		bActionable = View.ContestAction == EFMCodexNetworkContestAction::LongShotDeadCornerRoll;
		break;
	case EFMCodexNetworkPlayerIntentKind::CutInsideShotDirectAttackRoll:
		bActionable = View.ContestAction == EFMCodexNetworkContestAction::CutInsideShotDirectAttackRoll;
		break;
	case EFMCodexNetworkPlayerIntentKind::CutInsideShotDirectDefenseRoll:
		bActionable = View.ContestAction == EFMCodexNetworkContestAction::CutInsideShotDirectDefenseRoll;
		break;
	case EFMCodexNetworkPlayerIntentKind::CutInsideShotDeadCornerRoll:
		bActionable = View.ContestAction == EFMCodexNetworkContestAction::CutInsideShotDeadCornerRoll;
		break;
	case EFMCodexNetworkPlayerIntentKind::SubmitThroughBallOneOnOneShotChoice:
		bActionable = View.OneOnOneOptions.Contains(OneOnOneChoice);
		break;
	case EFMCodexNetworkPlayerIntentKind::AdvanceAfterTerminal:
		bActionable = View.bCanAdvance;
		break;
	default: break;
	}
	FFMCodexNetworkPlayerIntentEnvelope Candidate;
	Candidate.IntentKind = Kind;
	Candidate.Deployment = Choice;
	Candidate.Goalkeeper = GoalkeeperChoice;
	Candidate.Carrier = CarrierChoice;
	Candidate.Marker = MarkerChoice;
	Candidate.Runner = RunnerChoice;
	Candidate.Helper = HelperChoice;
	Candidate.Skill = SkillChoice;
	Candidate.Branch = BranchChoice;
	Candidate.OneOnOneChoice = OneOnOneChoice;
	Candidate.SetPieceCardId = SetPieceCardId; Candidate.NearMethod = NearMethod;
	Candidate.LongMethod = LongMethod; Candidate.PenaltyMethod = PenaltyMethod;
	if (IsPending() || !Match.IsValid() || NextRequestId == MAX_int64
		|| View.ViewRevision < SeenViewRevision || !View.bMatchInitialized
		|| View.BootstrapState != EFMCodexNetworkBootstrapState::MatchReady
		|| !bActionable || Candidate.ValidatePayloadShape() != EFMCodexNetworkIntentAckCode::None
		|| View.ViewerSide == EInitialTurnOrderPlayer::None
		|| View.ExpectedActingSide != View.ViewerSide || View.AttackSequence <= 0)
	{
		return false;
	}
	OutEnvelope = Candidate;
	OutEnvelope.MatchInstanceId = Match;
	OutEnvelope.RequestId = NextRequestId++;
	OutEnvelope.ExpectedAttackSequence = View.AttackSequence;
	PendingRequestId = OutEnvelope.RequestId;
	return true;
}

bool FFMCodexNetworkIntentClientState::ObserveAck(const FFMCodexNetworkPlayerIntentAck& Ack)
{
	if (!IsPending() || Ack.MatchInstanceId != Match || Ack.RequestId != PendingRequestId
		|| Ack.Code == EFMCodexNetworkIntentAckCode::None || Ack.ViewRevision < 0
		|| (LastAck.MatchInstanceId == Match && LastAck.RequestId == PendingRequestId))
	{
		return false;
	}
	LastAck = Ack;
	CompleteIfReady();
	return true;
}

void FFMCodexNetworkIntentClientState::CompleteIfReady()
{
	if (IsPending() && LastAck.MatchInstanceId == Match && LastAck.RequestId == PendingRequestId
		&& (LastAck.Code != EFMCodexNetworkIntentAckCode::Accepted
			|| SeenViewRevision >= LastAck.ViewRevision))
	{
		PendingRequestId = 0;
	}
}

bool FFMCodexNetworkIntentClientState::BeginSetPiece(const FFMCodexNetworkClientViewSnapshot& View,
 EFMCodexNetworkPlayerIntentKind Kind, FFMCodexNetworkPlayerIntentEnvelope& Out, FName Card,
 EMatchPlayShortFreeKickMethod Near, EMatchPlayLongFreeKickMethod Long, EMatchPlayPenaltyMethod Penalty)
{
 if (Kind < EFMCodexNetworkPlayerIntentKind::RequestSetPieceTypeRoll || Kind > EFMCodexNetworkPlayerIntentKind::ResolvePenaltyPanenkaRoll) return false;
 return BeginIntent(View, Kind, {}, {}, {}, {}, {}, {}, {}, {}, Out,
  EMatchPlayThroughBallOneOnOneShotChoice::None, Card, Near, Long, Penalty);
}
