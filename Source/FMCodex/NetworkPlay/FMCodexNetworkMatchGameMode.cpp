#include "FMCodexNetworkMatchGameMode.h"
#include "../Diagnostics/FMCodexHandoffLatencyAudit.h"

#include "FMCodexNetworkMatchGameState.h"
#include "FMCodexNetworkMatchPlayerController.h"
#include "FMCodexNetworkMatchPlayerState.h"

#include "GameFramework/PlayerState.h"
#include "HAL/PlatformProcess.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

namespace FMCodexNetworkMatchGameMode
{
	const TCHAR* SideLogLabel(const EInitialTurnOrderPlayer Side)
	{
		return Side == EInitialTurnOrderPlayer::PlayerA
			? TEXT("A")
			: Side == EInitialTurnOrderPlayer::PlayerB
				? TEXT("B")
				: TEXT("None");
	}
}

AFMCodexNetworkMatchGameMode::AFMCodexNetworkMatchGameMode()
{
	PlayerControllerClass =
		AFMCodexNetworkMatchPlayerController::StaticClass();
	PlayerStateClass = AFMCodexNetworkMatchPlayerState::StaticClass();
	GameStateClass = AFMCodexNetworkMatchGameState::StaticClass();
	DefaultPawnClass = nullptr;
}

AFMCodexNetworkMatchGameMode::~AFMCodexNetworkMatchGameMode() = default;

void AFMCodexNetworkMatchGameMode::BeginPlay()
{
	Super::BeginPlay();
	if (!HasAuthority())
	{
		return;
	}
	EnsureMatchInstanceId();
	EnsureBootstrapConfiguration();
	PublishParticipantState(
		EFMCodexNetworkBootstrapState::WaitingForPlayers);
}

void AFMCodexNetworkMatchGameMode::PreLogin(
	const FString& Options,
	const FString& Address,
	const FUniqueNetIdRepl& UniqueId,
	FString& ErrorMessage)
{
	Super::PreLogin(Options, Address, UniqueId, ErrorMessage);
	if (ErrorMessage.IsEmpty()
		&& ParticipantRegistry.HasReservedBothSides())
	{
		ErrorMessage = TEXT("MatchFull");
		UE_LOG(LogFMCodexNetworkPlay, Warning,
			TEXT("Rejected connection: both gameplay sides are reserved."));
	}
}

void AFMCodexNetworkMatchGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	EnsureMatchInstanceId();
	EnsureBootstrapConfiguration();
	AFMCodexNetworkMatchPlayerController* NetworkController = Cast<
		AFMCodexNetworkMatchPlayerController>(NewPlayer);
	AFMCodexNetworkMatchPlayerState* NetworkPlayerState = NetworkController
		!= nullptr
		? NetworkController->GetPlayerState<
			AFMCodexNetworkMatchPlayerState>()
		: nullptr;
	const FFMCodexNetworkAdmissionResult Admission =
		ParticipantRegistry.Admit(NetworkController, NetworkPlayerState);
	if (!Admission.bAccepted)
	{
		UE_LOG(LogFMCodexNetworkPlay, Error,
			TEXT("PostLogin admission failed closed (%d)."),
			static_cast<int32>(Admission.Error));
		if (NewPlayer != nullptr)
		{
			NewPlayer->ClientReturnToMainMenuWithTextReason(
				FText::FromString(TEXT("网络比赛已满或身份无效。")));
		}
		return;
	}

	const FFMCodexNetworkTeamIdentity& Team =
		Admission.AssignedSide == EInitialTurnOrderPlayer::PlayerA
			? BootstrapConfiguration.PlayerATeam
			: BootstrapConfiguration.PlayerBTeam;
	const FString PlayerDisplayName = SelectPlayerDisplayName(
		NetworkPlayerState->GetPlayerName(),
		Admission.AssignedSide);
	NetworkPlayerState->SetNetworkIdentityOnServer(
		Admission.AssignedSide,
		Team,
		PlayerDisplayName);
	FFMCodexNetworkParticipantPublicIdentity& PublicIdentity =
		Admission.AssignedSide == EInitialTurnOrderPlayer::PlayerA
			? PlayerAIdentity
			: PlayerBIdentity;
	PublicIdentity.bAssigned = true;
	PublicIdentity.bConnected = true;
	PublicIdentity.GameplaySide = Admission.AssignedSide;
	PublicIdentity.PlayerDisplayName = PlayerDisplayName;
	PublicIdentity.Team = Team;
	UE_LOG(LogFMCodexNetworkPlay, Log,
		TEXT("Admitted participant as Side %s (same path for host/remote)."),
		FMCodexNetworkMatchGameMode::SideLogLabel(
			Admission.AssignedSide));

	PublishParticipantState(
		EFMCodexNetworkBootstrapState::WaitingForPlayers);
	PublishOwnerViews(
		EFMCodexNetworkBootstrapState::WaitingForPlayers);
	TryInitializeNetworkMatch();
}

void AFMCodexNetworkMatchGameMode::Logout(AController* Exiting)
{
	const EInitialTurnOrderPlayer Side =
		ParticipantRegistry.ResolveSide(Exiting);
	ParticipantRegistry.MarkDisconnected(Exiting);
	Super::Logout(Exiting);
	if (Side != EInitialTurnOrderPlayer::None)
	{
		FFMCodexNetworkParticipantPublicIdentity& PublicIdentity =
			Side == EInitialTurnOrderPlayer::PlayerA
				? PlayerAIdentity
				: PlayerBIdentity;
		PublicIdentity.bConnected = false;
		UE_LOG(LogFMCodexNetworkPlay, Warning,
			TEXT("Side %s disconnected; its reservation is retained."),
			FMCodexNetworkMatchGameMode::SideLogLabel(Side));
		PublishParticipantState(
			EFMCodexNetworkBootstrapState::ParticipantDisconnected);
		PublishOwnerViews(
			EFMCodexNetworkBootstrapState::ParticipantDisconnected);
	}
}

EInitialTurnOrderPlayer
AFMCodexNetworkMatchGameMode::ResolveSideForController(
	const AController* Controller) const
{
	return ParticipantRegistry.ResolveSide(Controller);
}

const FGuid& AFMCodexNetworkMatchGameMode::GetMatchInstanceId() const
{
	return MatchInstanceId;
}

bool AFMCodexNetworkMatchGameMode::IsNetworkMatchInitialized() const
{
	return MatchRuntime.IsValid() && MatchRuntime->IsInitialized();
}

void AFMCodexNetworkMatchGameMode::EnsureMatchInstanceId()
{
	if (!MatchInstanceId.IsValid())
	{
		MatchInstanceId = FGuid::NewGuid();
		UE_LOG(LogFMCodexNetworkPlay, Log,
			TEXT("Created MatchInstanceId %s."),
			*MatchInstanceId.ToString(EGuidFormats::DigitsWithHyphensLower));
	}
}

void AFMCodexNetworkMatchGameMode::EnsureBootstrapConfiguration()
{
	if (BootstrapConfiguration.PlayerATeam.TeamId.IsNone()
		|| BootstrapConfiguration.PlayerBTeam.TeamId.IsNone())
	{
		BootstrapConfiguration =
			FFMCodexNetworkBootstrapConfigurationFactory::CreatePrototypeMatch();
#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
		// Server command line only. Changes opening fixture, never the entropy source.
		if (HasAuthority() && FParse::Param(FCommandLine::Get(), TEXT("FMCodexNetworkTestBFirst")))
		{
			BootstrapConfiguration = FFMCodexNetworkBootstrapConfigurationFactory::CreateBFirstAutomationMatch(
				FParse::Param(FCommandLine::Get(), TEXT("FMCodexNetworkBranchSlice")));
			UE_LOG(LogFMCodexNetworkPlay, Log, TEXT("Server automation opening fixture: Side B first; production secure RNG unchanged."));
		}
#endif
	}
}

void AFMCodexNetworkMatchGameMode::TryInitializeNetworkMatch()
{
	if (bBootstrapAttempted || !ParticipantRegistry.HasBothParticipants())
	{
		return;
	}
	bBootstrapAttempted = true;
	EnsureMatchInstanceId();
	EnsureBootstrapConfiguration();
	MatchRuntime = MakeUnique<FFMCodexNetworkMatchRuntime>(
		MatchInstanceId);
	if (FParse::Param(FCommandLine::Get(), TEXT("FMCodexNetworkPlayerFacingUI")))
		MatchRuntime->EnablePlayerFacingPresentation();
#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
	FString TerminalMilestone;
	ESkillRuleType PlayerFacingFamily = ESkillRuleType::Cross;
	bool bPlayerFacingMilestone = HasAuthority() && FParse::Value(FCommandLine::Get(), TEXT("FMCodexNetworkPlayerFacingCrossMilestone="), TerminalMilestone);
	if (HasAuthority() && FParse::Value(FCommandLine::Get(), TEXT("FMCodexNetworkPlayerFacingPassControlMilestone="), TerminalMilestone))
	{ bPlayerFacingMilestone = true; PlayerFacingFamily = ESkillRuleType::PassControl; }
	if (HasAuthority() && FParse::Value(FCommandLine::Get(), TEXT("FMCodexNetworkPlayerFacingThroughBallFeetMilestone="), TerminalMilestone))
	{ bPlayerFacingMilestone = true; PlayerFacingFamily = ESkillRuleType::ThroughBall; }
	FString ConditionalPath, ConditionalActor = TEXT("A");
	const bool bConditional = HasAuthority() && FParse::Value(FCommandLine::Get(), TEXT("FMCodexNetworkPlayerFacingThroughBallMilestone="), ConditionalPath);
	if (bConditional)
	{
		bPlayerFacingMilestone = true; PlayerFacingFamily = ESkillRuleType::ThroughBall;
		FParse::Value(FCommandLine::Get(), TEXT("FMCodexNetworkThroughBallActor="), ConditionalActor);
		TerminalMilestone = FParse::Param(FCommandLine::Get(), TEXT("FMCodexNetworkThroughBallNoGoal")) ? TEXT("NoGoal") : TEXT("Goal");
		if (FParse::Param(FCommandLine::Get(), TEXT("FMCodexNetworkThroughBallFinal"))) TerminalMilestone = TEXT("Final") + TerminalMilestone;
	}
	FString ShotMilestone, ShotActor = TEXT("A");
	bool bShot = HasAuthority() && FParse::Value(FCommandLine::Get(), TEXT("FMCodexNetworkPlayerFacingLongShotMilestone="), ShotMilestone);
	if (bShot) PlayerFacingFamily = ESkillRuleType::LongShot;
	if (HasAuthority() && FParse::Value(FCommandLine::Get(), TEXT("FMCodexNetworkPlayerFacingCutInsideMilestone="), ShotMilestone))
	{ bShot = true; PlayerFacingFamily = ESkillRuleType::CutInsideShot; }
	const bool bValidShot = ShotMilestone == TEXT("DirectGoal") || ShotMilestone == TEXT("DirectMiss")
		|| ShotMilestone == TEXT("ImmediateMiss") || ShotMilestone == TEXT("DeadCornerGoal") || ShotMilestone == TEXT("DeadCornerMiss");
	if (bShot)
	{
		bPlayerFacingMilestone = true;
		FParse::Value(FCommandLine::Get(), TEXT("FMCodexNetworkShotActor="), ShotActor);
		TerminalMilestone = ShotMilestone.EndsWith(TEXT("Goal")) ? TEXT("Goal") : TEXT("NoGoal");
		if (FParse::Param(FCommandLine::Get(), TEXT("FMCodexNetworkShotFinal"))) TerminalMilestone = TEXT("Final") + TerminalMilestone;
	}
	FString DeclineMilestone, DeclineActor = TEXT("A");
	const bool bDecline = HasAuthority() && FParse::Value(FCommandLine::Get(), TEXT("FMCodexNetworkPlayerFacingDeclineMilestone="), DeclineMilestone);
	EFMCodexNetworkDeclineAction DeclineStop = EFMCodexNetworkDeclineAction::None;
	if (bDecline)
	{
		DeclineStop = DeclineMilestone == TEXT("Runner") ? EFMCodexNetworkDeclineAction::Runner
			: DeclineMilestone == TEXT("Helper") ? EFMCodexNetworkDeclineAction::Helper
			: DeclineMilestone == TEXT("Skill") ? EFMCodexNetworkDeclineAction::Skill
			: DeclineMilestone == TEXT("Marker") ? EFMCodexNetworkDeclineAction::Marker : EFMCodexNetworkDeclineAction::None;
		FParse::Value(FCommandLine::Get(), TEXT("FMCodexNetworkDeclineActor="), DeclineActor);
		bPlayerFacingMilestone = true; PlayerFacingFamily = ESkillRuleType::LongShot;
		TerminalMilestone = FParse::Param(FCommandLine::Get(), TEXT("FMCodexNetworkDeclineFinal")) ? TEXT("FinalGoal") : TEXT("Goal");
	}
	const bool bTerminalMilestone = bPlayerFacingMilestone || (HasAuthority() && FParse::Value(FCommandLine::Get(), TEXT("FMCodexNetworkCrossTerminalMilestone="), TerminalMilestone));
	const bool bTerminalFinal = TerminalMilestone.StartsWith(TEXT("Final"));
	const bool bTerminalGoal = TerminalMilestone == TEXT("Goal") || TerminalMilestone == TEXT("FinalGoal");
	const bool bValidTerminalMilestone = bTerminalGoal || TerminalMilestone == TEXT("NoGoal") || TerminalMilestone == TEXT("FinalNoGoal");
	if (bTerminalMilestone)
	{
		if (!bValidTerminalMilestone || (bDecline && (DeclineStop == EFMCodexNetworkDeclineAction::None || (DeclineActor != TEXT("A") && DeclineActor != TEXT("B")))) || (bShot && (!bValidShot || (ShotActor != TEXT("A") && ShotActor != TEXT("B")))) || (bConditional && (ConditionalPath != TEXT("BehindOneOnOne") && ConditionalPath != TEXT("BehindOutOfPlay")
            && ConditionalPath != TEXT("AntiOffside") && ConditionalPath != TEXT("AntiOneOnOne")))
            || (bConditional && ConditionalActor != TEXT("A") && ConditionalActor != TEXT("B")))
		{
			bTransportFault = true;
			PublishParticipantState(EFMCodexNetworkBootstrapState::BootstrapFailed);
			PublishOwnerViews(EFMCodexNetworkBootstrapState::BootstrapFailed);
			return;
		}
		const bool BFirst = (bDecline ? ((DeclineActor == TEXT("B")) != (DeclineStop == EFMCodexNetworkDeclineAction::Helper || DeclineStop == EFMCodexNetworkDeclineAction::Marker)) : bShot ? ShotActor == TEXT("B") : bConditional ? ConditionalActor == TEXT("B") : !bTerminalGoal) != bTerminalFinal;
		BootstrapConfiguration = BFirst ? FFMCodexNetworkBootstrapConfigurationFactory::CreateBFirstAutomationMatch()
			: FFMCodexNetworkBootstrapConfigurationFactory::CreatePrototypeMatch();
		BootstrapConfiguration.MatchConfiguration.OpeningInput.OpeningInput.bUseDevOneAttackPerSide = bTerminalFinal;
		BootstrapConfiguration.AttackOpportunitiesPerSide = bTerminalFinal ? 1 : 3;
		int32 RouteD6 = PlayerFacingFamily == ESkillRuleType::Cross ? 5 : bTerminalGoal ? 1 : 3;
		FParse::Value(FCommandLine::Get(), TEXT("FMCodexNetworkPassControlRouteD6="), RouteD6);
		if (RouteD6 < 1 || RouteD6 > 6) RouteD6 = 1;
		if (bDecline) MatchRuntime->EnableSpecializedShotAutomation(false, false, bTerminalFinal);
		else if (bShot) MatchRuntime->EnableSpecializedShotAutomation(bTerminalGoal, ShotMilestone == TEXT("ImmediateMiss"), bTerminalFinal);
		else if (bConditional) MatchRuntime->EnableThroughBallConditionalAutomation(ConditionalPath, bTerminalGoal, bTerminalFinal);
		else MatchRuntime->EnableOrdinaryTerminalAutomation(bTerminalGoal, bTerminalFinal, PlayerFacingFamily, RouteD6);
	}
	FString Milestone;
	const bool bMilestone = HasAuthority() && FParse::Value(FCommandLine::Get(), TEXT("FMCodexNetworkRouteMilestone="), Milestone);
	const bool bSkillSlice = bMilestone || FParse::Param(FCommandLine::Get(), TEXT("FMCodexNetworkSkillSlice"));
	int32 AutomationAttackD6 = 0, AutomationDefenseD6 = 0;
	if (HasAuthority()
		&& FParse::Value(FCommandLine::Get(), TEXT("FMCodexNetworkContestAttackD6="), AutomationAttackD6)
		&& FParse::Value(FCommandLine::Get(), TEXT("FMCodexNetworkContestDefenseD6="), AutomationDefenseD6)
		&& AutomationAttackD6 >= 1 && AutomationAttackD6 <= 6 && AutomationDefenseD6 >= 1 && AutomationDefenseD6 <= 6)
	{
		MatchRuntime->EnablePostRouteAutomation(AutomationAttackD6, AutomationDefenseD6);
	}
	int32 AutomationRouteD6 = 0;
	if (HasAuthority() && FParse::Value(FCommandLine::Get(), TEXT("FMCodexNetworkInitialRouteD6="), AutomationRouteD6)
		&& AutomationRouteD6 >= 1 && AutomationRouteD6 <= 6)
	{
		MatchRuntime->EnableInitialRouteAutomation(AutomationRouteD6);
	}
	if (HasAuthority() && (bSkillSlice || FParse::Param(FCommandLine::Get(), TEXT("FMCodexNetworkDeploymentSlice"))))
	{
		MatchRuntime->EnableDeploymentAutomationEntry(bSkillSlice ? 6 : 4);
	}
	FString NearMilestone, NearActor = TEXT("A");
	if (HasAuthority() && FParse::Value(FCommandLine::Get(), TEXT("FMCodexNetworkPlayerFacingNearFreeKickMilestone="), NearMilestone))
	{
		FParse::Value(FCommandLine::Get(), TEXT("FMCodexNetworkSetPieceActor="), NearActor);
		if ((NearMilestone != TEXT("DirectGoal") && NearMilestone != TEXT("DirectMiss")
			&& NearMilestone != TEXT("AngledGoal") && NearMilestone != TEXT("AngledMiss"))
			|| (NearActor != TEXT("A") && NearActor != TEXT("B")) || bTerminalMilestone || bMilestone || bSkillSlice)
		{
			bTransportFault = true;
			PublishParticipantState(EFMCodexNetworkBootstrapState::BootstrapFailed);
			PublishOwnerViews(EFMCodexNetworkBootstrapState::BootstrapFailed); return;
		}
		BootstrapConfiguration = NearActor == TEXT("B") ? FFMCodexNetworkBootstrapConfigurationFactory::CreateBFirstAutomationMatch()
			: FFMCodexNetworkBootstrapConfigurationFactory::CreatePrototypeMatch();
		MatchRuntime->EnableNearFreeKickMilestone(NearMilestone.EndsWith(TEXT("Goal")));
	}
	FString LongMilestone, LongActor = TEXT("A");
	if (HasAuthority() && FParse::Value(FCommandLine::Get(), TEXT("FMCodexNetworkPlayerFacingLongFreeKickMilestone="), LongMilestone))
	{
		FParse::Value(FCommandLine::Get(), TEXT("FMCodexNetworkSetPieceActor="), LongActor);
		if ((LongMilestone != TEXT("DirectGoal") && LongMilestone != TEXT("DirectMiss")
			&& LongMilestone != TEXT("DirectEarlyNoGoal") && LongMilestone != TEXT("PowerGoal") && LongMilestone != TEXT("PowerMiss"))
			|| (LongActor != TEXT("A") && LongActor != TEXT("B")) || bTerminalMilestone || bMilestone || bSkillSlice || !NearMilestone.IsEmpty())
		{
			bTransportFault = true;
			PublishParticipantState(EFMCodexNetworkBootstrapState::BootstrapFailed);
			PublishOwnerViews(EFMCodexNetworkBootstrapState::BootstrapFailed); return;
		}
		BootstrapConfiguration = LongActor == TEXT("B") ? FFMCodexNetworkBootstrapConfigurationFactory::CreateBFirstAutomationMatch()
			: FFMCodexNetworkBootstrapConfigurationFactory::CreatePrototypeMatch();
		MatchRuntime->EnableLongFreeKickMilestone(LongMilestone == TEXT("DirectGoal") || LongMilestone == TEXT("PowerGoal"), LongMilestone == TEXT("DirectEarlyNoGoal"));
	}
	FString PenaltyMilestone, PenaltyActor = TEXT("A");
	if (HasAuthority() && FParse::Value(FCommandLine::Get(), TEXT("FMCodexNetworkPlayerFacingPenaltyMilestone="), PenaltyMilestone))
	{
		FParse::Value(FCommandLine::Get(), TEXT("FMCodexNetworkSetPieceActor="), PenaltyActor);
		if ((PenaltyMilestone != TEXT("DirectGoal") && PenaltyMilestone != TEXT("DirectMiss")
			&& PenaltyMilestone != TEXT("PanenkaGoal") && PenaltyMilestone != TEXT("PanenkaMiss"))
			|| (PenaltyActor != TEXT("A") && PenaltyActor != TEXT("B")) || bTerminalMilestone || bMilestone || bSkillSlice || !NearMilestone.IsEmpty() || !LongMilestone.IsEmpty())
		{
			bTransportFault = true;
			PublishParticipantState(EFMCodexNetworkBootstrapState::BootstrapFailed);
			PublishOwnerViews(EFMCodexNetworkBootstrapState::BootstrapFailed); return;
		}
		BootstrapConfiguration = PenaltyActor == TEXT("B") ? FFMCodexNetworkBootstrapConfigurationFactory::CreateBFirstAutomationMatch()
			: FFMCodexNetworkBootstrapConfigurationFactory::CreatePrototypeMatch();
		MatchRuntime->EnablePenaltyMilestone(PenaltyMilestone.EndsWith(TEXT("Goal")));
	}
	FString SetPieceMilestone, SetPieceActor = TEXT("A");
	if (HasAuthority() && FParse::Value(FCommandLine::Get(), TEXT("FMCodexNetworkPlayerFacingSetPieceMilestone="), SetPieceMilestone))
	{
		FParse::Value(FCommandLine::Get(), TEXT("FMCodexNetworkSetPieceActor="), SetPieceActor);
		const int32 TypeD6 = SetPieceMilestone == TEXT("Near") ? 5 : SetPieceMilestone == TEXT("Long") ? 3
			: SetPieceMilestone == TEXT("Penalty") ? 6 : SetPieceMilestone == TEXT("Corner") ? 1 : 0;
		if (!NearMilestone.IsEmpty() || !LongMilestone.IsEmpty() || !PenaltyMilestone.IsEmpty() || TypeD6 == 0 || (SetPieceActor != TEXT("A") && SetPieceActor != TEXT("B")) || bTerminalMilestone || bMilestone || bSkillSlice)
		{
			bTransportFault = true;
			PublishParticipantState(EFMCodexNetworkBootstrapState::BootstrapFailed);
			PublishOwnerViews(EFMCodexNetworkBootstrapState::BootstrapFailed); return;
		}
		BootstrapConfiguration = SetPieceActor == TEXT("B") ? FFMCodexNetworkBootstrapConfigurationFactory::CreateBFirstAutomationMatch()
			: FFMCodexNetworkBootstrapConfigurationFactory::CreatePrototypeMatch();
		MatchRuntime->EnableSetPieceSelectionMilestone(TypeD6);
	}
#endif
	const FFMCodexNetworkRuntimeInitializeResult Result =
		MatchRuntime->InitializeOnce(BootstrapConfiguration);
	if (!Result.bSuccess)
	{
		UE_LOG(LogFMCodexNetworkPlay, Error,
			TEXT("Authoritative network bootstrap failed: %s"),
			*Result.ErrorMessage);
		PublishParticipantState(
			EFMCodexNetworkBootstrapState::BootstrapFailed);
		PublishOwnerViews(
			EFMCodexNetworkBootstrapState::BootstrapFailed);
		return;
	}
#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
	if (bTerminalMilestone && !MatchRuntime->PrepareOrdinaryTerminalMilestone(bTerminalGoal, bTerminalFinal, bPlayerFacingMilestone, PlayerFacingFamily, DeclineStop))
	{
		UE_LOG(LogFMCodexNetworkPlay, Error, TEXT("Terminal milestone setup failed; no partial fixture is playable."));
		bTransportFault = true;
		PublishParticipantState(EFMCodexNetworkBootstrapState::BootstrapFailed);
		PublishOwnerViews(EFMCodexNetworkBootstrapState::BootstrapFailed);
		return;
	}
	if (bMilestone)
	{
		const auto Family = Milestone == TEXT("Cross") ? ESkillRuleType::Cross
			: Milestone == TEXT("PassControl") ? ESkillRuleType::PassControl
			: Milestone == TEXT("ThroughBall") ? ESkillRuleType::ThroughBall : ESkillRuleType::None;
		if (!MatchRuntime->PrepareInitialRouteMilestone(Family))
		{
			UE_LOG(LogFMCodexNetworkPlay, Error, TEXT("InitialRoute milestone setup failed; no partial fixture is playable."));
			bTransportFault = true;
			PublishParticipantState(EFMCodexNetworkBootstrapState::BootstrapFailed);
			PublishOwnerViews(EFMCodexNetworkBootstrapState::BootstrapFailed);
			return;
		}
	}
#endif
	UE_LOG(LogFMCodexNetworkPlay, Log,
		TEXT("Initialized prototype network match exactly once (%d+%d)."),
		BootstrapConfiguration.AttackOpportunitiesPerSide, BootstrapConfiguration.AttackOpportunitiesPerSide);
	PublishParticipantState(EFMCodexNetworkBootstrapState::MatchReady);
	PublishOwnerViews(EFMCodexNetworkBootstrapState::MatchReady);
}

void AFMCodexNetworkMatchGameMode::PublishParticipantState(
	const EFMCodexNetworkBootstrapState NewState)
{
	AFMCodexNetworkMatchGameState* NetworkGameState =
		GetGameState<AFMCodexNetworkMatchGameState>();
	if (NetworkGameState == nullptr)
	{
		return;
	}
	NetworkGameState->SetBootstrapStateOnServer(
		MatchInstanceId,
		NewState,
		BuildPublicIdentity(EInitialTurnOrderPlayer::PlayerA),
		BuildPublicIdentity(EInitialTurnOrderPlayer::PlayerB));
}

void AFMCodexNetworkMatchGameMode::PublishOwnerViews(
	const EFMCodexNetworkBootstrapState State)
{
	++ViewRevision;
	for (const EInitialTurnOrderPlayer Side : {
		EInitialTurnOrderPlayer::PlayerA,
		EInitialTurnOrderPlayer::PlayerB })
	{
		AFMCodexNetworkMatchPlayerController* Controller =
			ParticipantRegistry.FindController(Side);
		if (Controller == nullptr)
		{
			continue;
		}
		const FFMCodexNetworkClientViewSnapshot View = MatchRuntime.IsValid()
			? MatchRuntime->BuildClientView(Side, ViewRevision, State)
			: FFMCodexNetworkClientViewSnapshotFactory::BuildWaiting(
				MatchInstanceId, ViewRevision, Side, State);
		Controller->SetOwnerViewOnServer(View);
		UE_LOG(LogFMCodexNetworkPlay, Log,
			TEXT("Published owner-safe View revision %d to Side %s."),
			ViewRevision,
			FMCodexNetworkMatchGameMode::SideLogLabel(Side));
	}
}

FFMCodexNetworkParticipantPublicIdentity
AFMCodexNetworkMatchGameMode::BuildPublicIdentity(
	const EInitialTurnOrderPlayer Side) const
{
	return Side == EInitialTurnOrderPlayer::PlayerA
		? PlayerAIdentity
		: PlayerBIdentity;
}

FString AFMCodexNetworkMatchGameMode::SelectPlayerDisplayName(
	const FString& Candidate,
	const EInitialTurnOrderPlayer Side)
{
	const FString Trimmed = Candidate.TrimStartAndEnd();
	const FString ComputerName = FPlatformProcess::ComputerName();
	const bool bGeneratedPieName = !ComputerName.IsEmpty()
		&& Trimmed.StartsWith(ComputerName + TEXT("-"),
			ESearchCase::IgnoreCase);
	if (!Trimmed.IsEmpty()
		&& !Trimmed.Equals(TEXT("Player"), ESearchCase::IgnoreCase)
		&& !bGeneratedPieName)
	{
		return Trimmed;
	}
	return Side == EInitialTurnOrderPlayer::PlayerA
		? TEXT("玩家 A")
		: TEXT("玩家 B");
}

FFMCodexNetworkPlayerIntentAck AFMCodexNetworkMatchGameMode::SubmitConnectionPlayerIntent(
	AFMCodexNetworkMatchPlayerController* Controller,
	const FFMCodexNetworkPlayerIntentEnvelope& Envelope)
{
#if !UE_BUILD_SHIPPING
 FMCodexHandoffAudit::FContext TraceContext;
 if(FMCodexHandoffAudit::Enabled())
 {
  TraceContext.Match=Envelope.MatchInstanceId; TraceContext.Sequence=Envelope.ExpectedAttackSequence;
  TraceContext.Request=Envelope.RequestId; TraceContext.Revision=ViewRevision;
  TraceContext.Source=static_cast<int32>(ResolveSideForController(Controller)); TraceContext.Viewer=TraceContext.Source;
  TraceContext.Role=TEXT("Authority");
  TraceContext.Intent=StaticEnum<EFMCodexNetworkPlayerIntentKind>()->GetNameStringByValue(static_cast<int64>(Envelope.IntentKind));
  FMCodexHandoffAudit::Emit(TEXT("T1.ServerReceive"),TraceContext);
 }
 FMCodexHandoffAudit::FServerScope TraceScope(TraceContext);
#endif
	using AckCode = EFMCodexNetworkIntentAckCode;
	FFMCodexNetworkPlayerIntentAck Ack;
	// Echo the submitted correlation even for MatchMismatch so that owner can clear pending.
	Ack.MatchInstanceId = Envelope.MatchInstanceId;
	Ack.RequestId = Envelope.RequestId;
	const int32 PreviousRevision = ViewRevision;
	const EInitialTurnOrderPlayer Side = ResolveSideForController(Controller);
	auto Finish = [&](AckCode Code)
	{
		Ack.Code = Code;
#if !UE_BUILD_SHIPPING
  if(Code!=AckCode::Accepted) FMCodexHandoffAudit::Emit(TEXT("ServerRejected"),TraceContext,FString::Printf(TEXT(",\"AckCode\":%d"),static_cast<int32>(Code)));
#endif
		Ack.ViewRevision = ViewRevision;
		UE_LOG(LogFMCodexNetworkPlay, Log,
			TEXT("Intent server: Match=%s Request=%lld Controller=%s ResolvedSide=%d ExpectedSequence=%lld Kind=%d Card=%s Slot=%s GKSlot=%s Carrier=%s Marker=%s Runner=%s Helper=%s Skill=%s Branch=%d OneOnOne=%d ACK=%s Revision=%d->%d EntryProviderCalls=%d D12ProviderCalls=%d"),
			*Envelope.MatchInstanceId.ToString(EGuidFormats::DigitsWithHyphensLower),
			Envelope.RequestId, *GetNameSafe(Controller), static_cast<int32>(Side),
			Envelope.ExpectedAttackSequence, static_cast<int32>(Envelope.IntentKind),
			*Envelope.Deployment.CardId.ToString(), *Envelope.Deployment.SlotId.ToString(),
			*Envelope.Goalkeeper.SlotId.ToString(), *Envelope.Carrier.CarrierCardId.ToString(),
			*Envelope.Marker.MarkerCardId.ToString(),
			*Envelope.Runner.RunnerCardId.ToString(),
			*Envelope.Helper.HelperCardId.ToString(),
			*Envelope.Skill.SkillId.ToString(), static_cast<int32>(Envelope.Branch.Intent), static_cast<int32>(Envelope.OneOnOneChoice),
			*StaticEnum<EFMCodexNetworkIntentAckCode>()->GetNameStringByValue(static_cast<int64>(Code)),
			PreviousRevision, ViewRevision,
			MatchRuntime ? MatchRuntime->GetEntryProviderInvocationCount() : 0,
			MatchRuntime ? MatchRuntime->GetD12ProviderInvocationCount() : 0);
#if WITH_DEV_AUTOMATION_TESTS
		UE_LOG(LogFMCodexNetworkPlay, Log, TEXT("DEV coordinator accounting: Request=%lld Calls=%d"),
			Envelope.RequestId, MatchRuntime ? MatchRuntime->GetCoordinatorInvocationCountForTests() : 0);
#endif
		return Ack;
	};
	if (!HasAuthority() || Side == EInitialTurnOrderPlayer::None || Controller == nullptr)
	{
		return Finish(AckCode::NotParticipant);
	}
	if (!MatchInstanceId.IsValid() || Envelope.MatchInstanceId != MatchInstanceId)
	{
		return Finish(AckCode::MatchMismatch);
	}
	if (Envelope.RequestId <= 0 || Envelope.ExpectedAttackSequence <= 0)
	{
		return Finish(AckCode::InvalidPayload);
	}
	// Common transport validation has no attacker or deployment-side assumption.
	const AckCode Shape = Envelope.ValidatePayloadShape();
	if (Shape != AckCode::None) { return Finish(Shape); }
	// At most two records; malformed/wrong-match/nonparticipant requests never allocate one.
	auto& Ledger = IntentLedgers.FindOrAdd(Controller);
	const AckCode Admission = Ledger.Check(MatchInstanceId, Envelope);
	if (Admission != AckCode::None) { return Finish(Admission); }
	Ledger.Consume(MatchInstanceId, Envelope);
	if (bTransportFault || !IsNetworkMatchInitialized() || !ParticipantRegistry.HasBothParticipants())
	{
		return Finish(AckCode::InvalidPhase);
	}
	const auto Before = MatchRuntime->BuildClientView(
		Side, ViewRevision, EFMCodexNetworkBootstrapState::MatchReady);
	if (Envelope.ExpectedAttackSequence != Before.AttackSequence)
	{
		return Finish(AckCode::StaleAttackSequence);
	}
	FMatchPlayPlayerIntent Intent;
	switch (Envelope.IntentKind)
	{
	case EFMCodexNetworkPlayerIntentKind::RequestInitialActionPointRoll:
	{
		if (Side != Before.CurrentAttackingSide) { return Finish(AckCode::WrongSide); }
		if (Before.InteractionState != EFMCodexNetworkClientInteractionState::WaitingForOwnInitialActionPoint)
		{
			return Finish(AckCode::InvalidPhase);
		}
		FMatchPlayFullD12EntryRequest Request;
		Request.RequestingSide = Side;
		Request.ExpectedAttackSequence = Envelope.ExpectedAttackSequence;
		Intent = FMatchPlayPlayerIntent::Create(EMatchPlayAuthoritativeCommandKind::RequestInitialActionPointRoll, Request);
		break;
	}
	case EFMCodexNetworkPlayerIntentKind::RequestSetPieceTypeRoll:
	{
		if (Before.EntryWait != EFMCodexNetworkEntryWait::SetPieceTypeRoll) return Finish(AckCode::InvalidPhase);
		if (Side != Before.ExpectedActingSide) return Finish(AckCode::WrongSide);
		FMatchPlaySetPieceTypeRollRequest Request;
		Request.RequestingSide = Side;
		Request.AttackSequence = Envelope.ExpectedAttackSequence;

		Intent = FMatchPlayPlayerIntent::Create(EMatchPlayAuthoritativeCommandKind::RequestSetPieceTypeRoll, Request);
		break;
	}
	case EFMCodexNetworkPlayerIntentKind::SubmitSetPieceCarrier:
	{
		if (Before.EntryWait != EFMCodexNetworkEntryWait::SetPieceTakerSelection) return Finish(AckCode::InvalidPhase);
		if (Side != Before.ExpectedActingSide) return Finish(AckCode::WrongSide);
		FMatchPlaySetPieceCarrierSelectionRequest Request;
		Request.RequestingSide = Side;
		Request.AttackSequence = Envelope.ExpectedAttackSequence;
		Request.CardId = Envelope.SetPieceCardId;
		Intent = FMatchPlayPlayerIntent::Create(EMatchPlayAuthoritativeCommandKind::SubmitSetPieceCarrier, Request);
		break;
	}
	case EFMCodexNetworkPlayerIntentKind::SubmitShortFreeKickMethod:
	{
		if (Before.EntryWait != EFMCodexNetworkEntryWait::SetPieceMethodSelection) return Finish(AckCode::InvalidPhase);
		if (Before.SetPiece.Type != ESetPieceSelectedType::ShortFreeKick) return Finish(AckCode::InvalidPhase);
		if (Side != Before.ExpectedActingSide) return Finish(AckCode::WrongSide);
		FMatchPlayShortFreeKickMethodRequest Request;
		Request.RequestingSide = Side;
		Request.AttackSequence = Envelope.ExpectedAttackSequence;
		Request.Method = Envelope.NearMethod;
		Intent = FMatchPlayPlayerIntent::Create(EMatchPlayAuthoritativeCommandKind::SubmitShortFreeKickMethod, Request);
		break;
	}
	case EFMCodexNetworkPlayerIntentKind::SubmitLongFreeKickMethod:
	{
		if (Before.EntryWait != EFMCodexNetworkEntryWait::SetPieceMethodSelection) return Finish(AckCode::InvalidPhase);
		if (Before.SetPiece.Type != ESetPieceSelectedType::LongFreeKick) return Finish(AckCode::InvalidPhase);
		if (Side != Before.ExpectedActingSide) return Finish(AckCode::WrongSide);
		FMatchPlayLongFreeKickMethodRequest Request;
		Request.RequestingSide = Side;
		Request.AttackSequence = Envelope.ExpectedAttackSequence;
		Request.Method = Envelope.LongMethod;
		Intent = FMatchPlayPlayerIntent::Create(EMatchPlayAuthoritativeCommandKind::SubmitLongFreeKickMethod, Request);
		break;
	}
	case EFMCodexNetworkPlayerIntentKind::SubmitPenaltyMethod:
	{
		if (Before.EntryWait != EFMCodexNetworkEntryWait::SetPieceMethodSelection) return Finish(AckCode::InvalidPhase);
		if (Before.SetPiece.Type != ESetPieceSelectedType::Penalty) return Finish(AckCode::InvalidPhase);
		if (Side != Before.ExpectedActingSide) return Finish(AckCode::WrongSide);
		FMatchPlayPenaltyMethodRequest Request;
		Request.RequestingSide = Side;
		Request.AttackSequence = Envelope.ExpectedAttackSequence;
		Request.Method = Envelope.PenaltyMethod;
		Intent = FMatchPlayPlayerIntent::Create(EMatchPlayAuthoritativeCommandKind::SubmitPenaltyMethod, Request);
		break;
	}
	case EFMCodexNetworkPlayerIntentKind::ResolveShortFreeKickDirectAttackRoll:
	{
		if (Before.EntryBranch != EFMCodexNetworkEntryBranch::SetPiece
			|| Before.SetPiece.Type != ESetPieceSelectedType::ShortFreeKick
			|| Before.EntryWait != EFMCodexNetworkEntryWait::NearDirectAttackRoll) return Finish(AckCode::InvalidPhase);
		if (Side != Before.ExpectedActingSide) return Finish(AckCode::WrongSide);
		FMatchPlayShortFreeKickRollRequest Request;
		Request.RequestingSide = Side;
		Request.AttackSequence = Envelope.ExpectedAttackSequence;
		Intent = FMatchPlayPlayerIntent::Create(EMatchPlayAuthoritativeCommandKind::ResolveShortFreeKickDirectAttackRoll, Request);
		break;
	}
	case EFMCodexNetworkPlayerIntentKind::ResolveShortFreeKickDirectDefenseRoll:
	{
		if (Before.EntryBranch != EFMCodexNetworkEntryBranch::SetPiece
			|| Before.SetPiece.Type != ESetPieceSelectedType::ShortFreeKick
			|| Before.EntryWait != EFMCodexNetworkEntryWait::NearDirectDefenseRoll) return Finish(AckCode::InvalidPhase);
		if (Side != Before.ExpectedActingSide) return Finish(AckCode::WrongSide);
		FMatchPlayShortFreeKickRollRequest Request;
		Request.RequestingSide = Side;
		Request.AttackSequence = Envelope.ExpectedAttackSequence;
		Intent = FMatchPlayPlayerIntent::Create(EMatchPlayAuthoritativeCommandKind::ResolveShortFreeKickDirectDefenseRoll, Request);
		break;
	}
	case EFMCodexNetworkPlayerIntentKind::ResolveShortFreeKickAngledRoll:
	{
		if (Before.EntryBranch != EFMCodexNetworkEntryBranch::SetPiece
			|| Before.SetPiece.Type != ESetPieceSelectedType::ShortFreeKick
			|| Before.EntryWait != EFMCodexNetworkEntryWait::NearAngledRoll) return Finish(AckCode::InvalidPhase);
		if (Side != Before.ExpectedActingSide) return Finish(AckCode::WrongSide);
		FMatchPlayShortFreeKickRollRequest Request;
		Request.RequestingSide = Side;
		Request.AttackSequence = Envelope.ExpectedAttackSequence;
		Intent = FMatchPlayPlayerIntent::Create(EMatchPlayAuthoritativeCommandKind::ResolveShortFreeKickAngledRoll, Request);
		break;
	}
	case EFMCodexNetworkPlayerIntentKind::ResolveLongFreeKickDirectAttackRoll:
	{
		if (Before.EntryBranch != EFMCodexNetworkEntryBranch::SetPiece
			|| Before.SetPiece.Type != ESetPieceSelectedType::LongFreeKick
			|| Before.EntryWait != EFMCodexNetworkEntryWait::LongFreeKickDirectAttackRoll) return Finish(AckCode::InvalidPhase);
		if (Side != Before.ExpectedActingSide) return Finish(AckCode::WrongSide);
		FMatchPlayLongFreeKickRollRequest Request;
		Request.RequestingSide = Side;
		Request.AttackSequence = Envelope.ExpectedAttackSequence;
		Intent = FMatchPlayPlayerIntent::Create(EMatchPlayAuthoritativeCommandKind::ResolveLongFreeKickDirectAttackRoll, Request);
		break;
	}
	case EFMCodexNetworkPlayerIntentKind::ResolveLongFreeKickDirectDefenseRoll:
	{
		if (Before.EntryBranch != EFMCodexNetworkEntryBranch::SetPiece
			|| Before.SetPiece.Type != ESetPieceSelectedType::LongFreeKick
			|| Before.EntryWait != EFMCodexNetworkEntryWait::LongFreeKickDirectDefenseRoll) return Finish(AckCode::InvalidPhase);
		if (Side != Before.ExpectedActingSide) return Finish(AckCode::WrongSide);
		FMatchPlayLongFreeKickRollRequest Request;
		Request.RequestingSide = Side;
		Request.AttackSequence = Envelope.ExpectedAttackSequence;
		Intent = FMatchPlayPlayerIntent::Create(EMatchPlayAuthoritativeCommandKind::ResolveLongFreeKickDirectDefenseRoll, Request);
		break;
	}
	case EFMCodexNetworkPlayerIntentKind::ResolveLongFreeKickPowerRoll:
	{
		if (Before.EntryBranch != EFMCodexNetworkEntryBranch::SetPiece
			|| Before.SetPiece.Type != ESetPieceSelectedType::LongFreeKick
			|| Before.EntryWait != EFMCodexNetworkEntryWait::LongFreeKickPowerRoll) return Finish(AckCode::InvalidPhase);
		if (Side != Before.ExpectedActingSide) return Finish(AckCode::WrongSide);
		FMatchPlayLongFreeKickRollRequest Request;
		Request.RequestingSide = Side;
		Request.AttackSequence = Envelope.ExpectedAttackSequence;
		Intent = FMatchPlayPlayerIntent::Create(EMatchPlayAuthoritativeCommandKind::ResolveLongFreeKickPowerRoll, Request);
		break;
	}
	case EFMCodexNetworkPlayerIntentKind::ResolvePenaltyDirectAttackRoll:
	{
		if (Before.EntryBranch != EFMCodexNetworkEntryBranch::SetPiece
			|| Before.SetPiece.Type != ESetPieceSelectedType::Penalty
			|| Before.EntryWait != EFMCodexNetworkEntryWait::PenaltyDirectAttackRoll) return Finish(AckCode::InvalidPhase);
		if (Side != Before.ExpectedActingSide) return Finish(AckCode::WrongSide);
		FMatchPlayPenaltyRollRequest Request;
		Request.RequestingSide = Side;
		Request.AttackSequence = Envelope.ExpectedAttackSequence;
		Intent = FMatchPlayPlayerIntent::Create(EMatchPlayAuthoritativeCommandKind::ResolvePenaltyDirectAttackRoll, Request);
		break;
	}
	case EFMCodexNetworkPlayerIntentKind::ResolvePenaltyDirectDefenseRoll:
	{
		if (Before.EntryBranch != EFMCodexNetworkEntryBranch::SetPiece
			|| Before.SetPiece.Type != ESetPieceSelectedType::Penalty
			|| Before.EntryWait != EFMCodexNetworkEntryWait::PenaltyDirectDefenseRoll) return Finish(AckCode::InvalidPhase);
		if (Side != Before.ExpectedActingSide) return Finish(AckCode::WrongSide);
		FMatchPlayPenaltyRollRequest Request;
		Request.RequestingSide = Side;
		Request.AttackSequence = Envelope.ExpectedAttackSequence;
		Intent = FMatchPlayPlayerIntent::Create(EMatchPlayAuthoritativeCommandKind::ResolvePenaltyDirectDefenseRoll, Request);
		break;
	}
	case EFMCodexNetworkPlayerIntentKind::ResolvePenaltyPanenkaRoll:
	{
		if (Before.EntryBranch != EFMCodexNetworkEntryBranch::SetPiece
			|| Before.SetPiece.Type != ESetPieceSelectedType::Penalty
			|| Before.EntryWait != EFMCodexNetworkEntryWait::PenaltyPanenkaRoll) return Finish(AckCode::InvalidPhase);
		if (Side != Before.ExpectedActingSide) return Finish(AckCode::WrongSide);
		FMatchPlayPenaltyRollRequest Request;
		Request.RequestingSide = Side;
		Request.AttackSequence = Envelope.ExpectedAttackSequence;
		Intent = FMatchPlayPlayerIntent::Create(EMatchPlayAuthoritativeCommandKind::ResolvePenaltyPanenkaRoll, Request);
		break;
	}
	case EFMCodexNetworkPlayerIntentKind::DeployOrdinary:
	{
		if (Before.EntryBranch != EFMCodexNetworkEntryBranch::Ordinary
			|| Before.EntryWait != EFMCodexNetworkEntryWait::Deployment) { return Finish(AckCode::InvalidPhase); }
		// ExpectedActingSide is projected from canonical CurrentLegalDeploymentSide.
		// Session still owns card/slot/phase/sequence/ownership legality.
		if (Side != Before.ExpectedActingSide) { return Finish(AckCode::WrongSide); }
		FMatchPlayAuthoritativeDeployOrdinaryRequest Request;
		Request.RequestingSide = Side; // Exclusively server registry; never a claimed payload field.
		Request.ExpectedAttackSequence = Envelope.ExpectedAttackSequence;
		Request.CardId = Envelope.Deployment.CardId;
		Request.SlotId = Envelope.Deployment.SlotId;
		Intent = FMatchPlayPlayerIntent::Create(EMatchPlayAuthoritativeCommandKind::DeployOrdinary, Request);
		break;
	}
	case EFMCodexNetworkPlayerIntentKind::DeployGoalkeeper:
	{
		// Session owns defender/turn/phase/usage/slot legality and derives the unique GK card.
		FMatchPlayAuthoritativeDeployGoalkeeperRequest Request;
		Request.RequestingSide = Side;
		Request.ExpectedAttackSequence = Envelope.ExpectedAttackSequence;
		Request.SlotId = Envelope.Goalkeeper.SlotId;
		Intent = FMatchPlayPlayerIntent::Create(EMatchPlayAuthoritativeCommandKind::DeployGoalkeeper, Request);
		break;
	}
	case EFMCodexNetworkPlayerIntentKind::FinishDeployment:
	{
		// No guessed count or GK prerequisite: canonical Finish performs every gameplay check.
		FMatchPlayFinishDeploymentIntent Request;
		Request.RequestingSide = Side;
		Request.AttackSequence = Envelope.ExpectedAttackSequence;
		Intent = FMatchPlayPlayerIntent::Create(EMatchPlayAuthoritativeCommandKind::FinishDeployment, Request);
		break;
	}
	case EFMCodexNetworkPlayerIntentKind::SubmitCarrier:
	{
		// Side comes only from the connection. Session owns attacker/stage/candidate legality.
		FMatchPlayAuthoritativeSubmitCarrierRequest Request;
		Request.RequestingSide = Side;
		Request.ExpectedAttackSequence = Envelope.ExpectedAttackSequence;
		Request.CarrierCardId = Envelope.Carrier.CarrierCardId;
		Intent = FMatchPlayPlayerIntent::Create(EMatchPlayAuthoritativeCommandKind::SubmitCarrier, Request);
		break;
	}
	case EFMCodexNetworkPlayerIntentKind::SubmitMarker:
	{
		// Marker ownership, physical area and phase are canonical Session checks.
		FMatchPlayAuthoritativeSubmitMarkerRequest Request;
		Request.RequestingSide = Side;
		Request.ExpectedAttackSequence = Envelope.ExpectedAttackSequence;
		Request.MarkerCardId = Envelope.Marker.MarkerCardId;
		Intent = FMatchPlayPlayerIntent::Create(EMatchPlayAuthoritativeCommandKind::SubmitMarker, Request);
		break;
	}
	case EFMCodexNetworkPlayerIntentKind::DeclineMarker:
	{
		if (Before.EntryWait != EFMCodexNetworkEntryWait::MarkerSelection) { return Finish(AckCode::InvalidPhase); }
		if (Side != Before.ExpectedActingSide) { return Finish(AckCode::WrongSide); }
		FMatchPlayAuthoritativeDeclineMarkerRequest Request;
		Request.RequestingSide = Side; // Registry identity only; Session validates optionality.
		Request.ExpectedAttackSequence = Envelope.ExpectedAttackSequence;
		Intent = FMatchPlayPlayerIntent::Create(EMatchPlayAuthoritativeCommandKind::DeclineMarker, Request);
		break;
	}
	case EFMCodexNetworkPlayerIntentKind::DeclineRunner:
	{
		if (Before.EntryWait != EFMCodexNetworkEntryWait::RunnerSelection) { return Finish(AckCode::InvalidPhase); }
		if (Side != Before.ExpectedActingSide) { return Finish(AckCode::WrongSide); }
		FMatchPlayAuthoritativeDeclineRunnerRequest Request;
		Request.RequestingSide = Side; // Registry identity only; Session validates optionality.
		Request.ExpectedAttackSequence = Envelope.ExpectedAttackSequence;
		Intent = FMatchPlayPlayerIntent::Create(EMatchPlayAuthoritativeCommandKind::DeclineRunner, Request);
		break;
	}
	case EFMCodexNetworkPlayerIntentKind::DeclineHelper:
	{
		if (Before.EntryWait != EFMCodexNetworkEntryWait::HelperSelection) { return Finish(AckCode::InvalidPhase); }
		if (Side != Before.ExpectedActingSide) { return Finish(AckCode::WrongSide); }
		FMatchPlayAuthoritativeDeclineHelperRequest Request;
		Request.RequestingSide = Side; // Registry identity only; Session validates optionality.
		Request.ExpectedAttackSequence = Envelope.ExpectedAttackSequence;
		Intent = FMatchPlayPlayerIntent::Create(EMatchPlayAuthoritativeCommandKind::DeclineHelper, Request);
		break;
	}
	case EFMCodexNetworkPlayerIntentKind::DeclineSkill:
	{
		if (Before.EntryWait != EFMCodexNetworkEntryWait::SkillSelection) { return Finish(AckCode::InvalidPhase); }
		if (Side != Before.ExpectedActingSide) { return Finish(AckCode::WrongSide); }
		FMatchPlayAuthoritativeDeclineSkillRequest Request;
		Request.RequestingSide = Side; // Registry identity only; Session validates optionality.
		Request.ExpectedAttackSequence = Envelope.ExpectedAttackSequence;
		Intent = FMatchPlayPlayerIntent::Create(EMatchPlayAuthoritativeCommandKind::DeclineSkill, Request);
		break;
	}
	case EFMCodexNetworkPlayerIntentKind::SubmitRunner:
	{
		// Runner ownership, deployment and phase are canonical Session checks.
		FMatchPlayAuthoritativeSubmitRunnerRequest Request;
		Request.RequestingSide = Side;
		Request.ExpectedAttackSequence = Envelope.ExpectedAttackSequence;
		Request.RunnerCardId = Envelope.Runner.RunnerCardId;
		Intent = FMatchPlayPlayerIntent::Create(EMatchPlayAuthoritativeCommandKind::SubmitRunner, Request);
		break;
	}
	case EFMCodexNetworkPlayerIntentKind::SubmitHelper:
	{
		// Helper ownership, deployment and phase are canonical Session checks.
		FMatchPlayAuthoritativeSubmitHelperRequest Request;
		Request.RequestingSide = Side;
		Request.ExpectedAttackSequence = Envelope.ExpectedAttackSequence;
		Request.HelperCardId = Envelope.Helper.HelperCardId;
		Intent = FMatchPlayPlayerIntent::Create(EMatchPlayAuthoritativeCommandKind::SubmitHelper, Request);
		break;
	}
	case EFMCodexNetworkPlayerIntentKind::SubmitSkill:
	{
		// Session resolves this SkillId using its private pinned rule set. No rule data enters the adapter.
		FMatchPlayAuthoritativeSubmitSkillRequest Request;
		Request.RequestingSide = Side;
		Request.ExpectedAttackSequence = Envelope.ExpectedAttackSequence;
		Request.SkillId = Envelope.Skill.SkillId;
		Intent = FMatchPlayPlayerIntent::Create(EMatchPlayAuthoritativeCommandKind::SubmitSkill, Request);
		break;
	}
	case EFMCodexNetworkPlayerIntentKind::SubmitBranchIntent:
	{
		FMatchPlayAuthoritativeSubmitBranchIntentRequest Request;
		Request.RequestingSide = Side;
		Request.AttackSequence = Envelope.ExpectedAttackSequence;
		Request.Intent = Envelope.Branch.Intent;
		Intent = FMatchPlayPlayerIntent::Create(EMatchPlayAuthoritativeCommandKind::SubmitBranchIntent, Request);
		break;
	}
	case EFMCodexNetworkPlayerIntentKind::CrossInitialRouteRoll:
	{
		FMatchPlayAuthoritativeResolveCrossInitialRouteRollRequest Request;
		Request.RequestingSide = Side;
		Request.AttackSequence = Envelope.ExpectedAttackSequence;
		Intent = FMatchPlayPlayerIntent::Create(EMatchPlayAuthoritativeCommandKind::ResolveCrossInitialRouteRoll, Request);
		break;
	}
	case EFMCodexNetworkPlayerIntentKind::PassControlInitialRouteRoll:
	{
		FMatchPlayAuthoritativeResolvePassControlInitialRouteRollRequest Request;
		Request.RequestingSide = Side;
		Request.AttackSequence = Envelope.ExpectedAttackSequence;
		Intent = FMatchPlayPlayerIntent::Create(EMatchPlayAuthoritativeCommandKind::ResolvePassControlInitialRouteRoll, Request);
		break;
	}
	case EFMCodexNetworkPlayerIntentKind::ThroughBallInitialRouteRoll:
	{
		FMatchPlayAuthoritativeResolveThroughBallInitialRouteRollRequest Request;
		Request.RequestingSide = Side;
		Request.AttackSequence = Envelope.ExpectedAttackSequence;
		Intent = FMatchPlayPlayerIntent::Create(EMatchPlayAuthoritativeCommandKind::ResolveThroughBallInitialRouteRoll, Request);
		break;
	}
	case EFMCodexNetworkPlayerIntentKind::CrossHighAttackRoll:
	{
		FMatchPlayAuthoritativeResolveCrossHighAttackRollRequest Request;
		Request.RequestingSide = Side;
		Request.AttackSequence = Envelope.ExpectedAttackSequence;
		Intent = FMatchPlayPlayerIntent::Create(EMatchPlayAuthoritativeCommandKind::ResolveCrossHighAttackRoll, Request);
		break;
	}
	case EFMCodexNetworkPlayerIntentKind::CrossHighDefenseRoll:
	{
		FMatchPlayAuthoritativeResolveCrossHighDefenseRollRequest Request;
		Request.RequestingSide = Side;
		Request.AttackSequence = Envelope.ExpectedAttackSequence;
		Intent = FMatchPlayPlayerIntent::Create(EMatchPlayAuthoritativeCommandKind::ResolveCrossHighDefenseRoll, Request);
		break;
	}
	case EFMCodexNetworkPlayerIntentKind::CrossLowAttackRoll:
	{
		FMatchPlayAuthoritativeResolveCrossLowAttackRollRequest Request;
		Request.RequestingSide = Side;
		Request.AttackSequence = Envelope.ExpectedAttackSequence;
		Intent = FMatchPlayPlayerIntent::Create(EMatchPlayAuthoritativeCommandKind::ResolveCrossLowAttackRoll, Request);
		break;
	}
	case EFMCodexNetworkPlayerIntentKind::CrossLowDefenseRoll:
	{
		FMatchPlayAuthoritativeResolveCrossLowDefenseRollRequest Request;
		Request.RequestingSide = Side;
		Request.AttackSequence = Envelope.ExpectedAttackSequence;
		Intent = FMatchPlayPlayerIntent::Create(EMatchPlayAuthoritativeCommandKind::ResolveCrossLowDefenseRoll, Request);
		break;
	}
	case EFMCodexNetworkPlayerIntentKind::PassControlAttackRoll:
	{
		FMatchPlayAuthoritativeResolvePassControlAttackRollRequest Request;
		Request.RequestingSide = Side;
		Request.AttackSequence = Envelope.ExpectedAttackSequence;
		Intent = FMatchPlayPlayerIntent::Create(EMatchPlayAuthoritativeCommandKind::ResolvePassControlAttackRoll, Request);
		break;
	}
	case EFMCodexNetworkPlayerIntentKind::PassControlDefenseRoll:
	{
		FMatchPlayAuthoritativeResolvePassControlDefenseRollRequest Request;
		Request.RequestingSide = Side;
		Request.AttackSequence = Envelope.ExpectedAttackSequence;
		Intent = FMatchPlayPlayerIntent::Create(EMatchPlayAuthoritativeCommandKind::ResolvePassControlDefenseRoll, Request);
		break;
	}
	case EFMCodexNetworkPlayerIntentKind::ThroughBallFeetAttackRoll:
	{
		FMatchPlayAuthoritativeResolveThroughBallFeetAttackRollRequest Request;
		Request.RequestingSide = Side;
		Request.AttackSequence = Envelope.ExpectedAttackSequence;
		Intent = FMatchPlayPlayerIntent::Create(EMatchPlayAuthoritativeCommandKind::ResolveThroughBallFeetAttackRoll, Request);
		break;
	}
	case EFMCodexNetworkPlayerIntentKind::ThroughBallFeetDefenseRoll:
	{
		FMatchPlayAuthoritativeResolveThroughBallFeetDefenseRollRequest Request;
		Request.RequestingSide = Side;
		Request.AttackSequence = Envelope.ExpectedAttackSequence;
		Intent = FMatchPlayPlayerIntent::Create(EMatchPlayAuthoritativeCommandKind::ResolveThroughBallFeetDefenseRoll, Request);
		break;
	}
	case EFMCodexNetworkPlayerIntentKind::ThroughBallBehindDefenseP1AttackRoll:
	{
		FMatchPlayAuthoritativeResolveThroughBallBehindDefenseP1AttackRollRequest Request;
		Request.RequestingSide = Side;
		Request.AttackSequence = Envelope.ExpectedAttackSequence;
		Intent = FMatchPlayPlayerIntent::Create(EMatchPlayAuthoritativeCommandKind::ResolveThroughBallBehindDefenseP1AttackRoll, Request);
		break;
	}
	case EFMCodexNetworkPlayerIntentKind::ThroughBallBehindDefenseP1DefenseRoll:
	{
		FMatchPlayAuthoritativeResolveThroughBallBehindDefenseP1DefenseRollRequest Request;
		Request.RequestingSide = Side;
		Request.AttackSequence = Envelope.ExpectedAttackSequence;
		Intent = FMatchPlayPlayerIntent::Create(EMatchPlayAuthoritativeCommandKind::ResolveThroughBallBehindDefenseP1DefenseRoll, Request);
		break;
	}
	case EFMCodexNetworkPlayerIntentKind::ThroughBallAntiOffsideAttackRoll:
	{
		FMatchPlayAuthoritativeResolveThroughBallAntiOffsideAttackRollRequest Request;
		Request.RequestingSide = Side;
		Request.AttackSequence = Envelope.ExpectedAttackSequence;
		Intent = FMatchPlayPlayerIntent::Create(EMatchPlayAuthoritativeCommandKind::ResolveThroughBallAntiOffsideAttackRoll, Request);
		break;
	}
	case EFMCodexNetworkPlayerIntentKind::ThroughBallOneOnOneDirectShotAttackRoll:
	{
		FMatchPlayAuthoritativeResolveThroughBallOneOnOneDirectShotAttackRollRequest Request;
		Request.RequestingSide = Side;
		Request.AttackSequence = Envelope.ExpectedAttackSequence;
		Intent = FMatchPlayPlayerIntent::Create(EMatchPlayAuthoritativeCommandKind::ResolveThroughBallOneOnOneDirectShotAttackRoll, Request);
		break;
	}
	case EFMCodexNetworkPlayerIntentKind::ThroughBallOneOnOneDirectShotDefenseRoll:
	{
		FMatchPlayAuthoritativeResolveThroughBallOneOnOneDirectShotDefenseRollRequest Request;
		Request.RequestingSide = Side;
		Request.AttackSequence = Envelope.ExpectedAttackSequence;
		Intent = FMatchPlayPlayerIntent::Create(EMatchPlayAuthoritativeCommandKind::ResolveThroughBallOneOnOneDirectShotDefenseRoll, Request);
		break;
	}
	case EFMCodexNetworkPlayerIntentKind::ThroughBallOneOnOneChipShotAttackRoll:
	{
		FMatchPlayAuthoritativeResolveThroughBallOneOnOneChipShotAttackRollRequest Request;
		Request.RequestingSide = Side;
		Request.AttackSequence = Envelope.ExpectedAttackSequence;
		Intent = FMatchPlayPlayerIntent::Create(EMatchPlayAuthoritativeCommandKind::ResolveThroughBallOneOnOneChipShotAttackRoll, Request);
		break;
	}
	case EFMCodexNetworkPlayerIntentKind::SubmitThroughBallOneOnOneShotChoice:
	{
		FMatchPlayAuthoritativeSubmitThroughBallOneOnOneShotChoiceRequest Request;
		Request.RequestingSide = Side;
		Request.ExpectedAttackSequence = Envelope.ExpectedAttackSequence;
		Request.Choice = Envelope.OneOnOneChoice;
		Intent = FMatchPlayPlayerIntent::Create(EMatchPlayAuthoritativeCommandKind::SubmitThroughBallOneOnOneShotChoice, Request);
		break;
	}
	case EFMCodexNetworkPlayerIntentKind::LongShotDirectAttackRoll:
	{
		FMatchPlayAuthoritativeResolveLongShotDirectAttackRollRequest Request;
		Request.AttackSequence = Envelope.ExpectedAttackSequence;
		Request.RequestingSide = Side;
		Intent = FMatchPlayPlayerIntent::Create(EMatchPlayAuthoritativeCommandKind::ResolveLongShotDirectAttackRoll, Request);
		break;
	}
	case EFMCodexNetworkPlayerIntentKind::LongShotDirectDefenseRoll:
	{
		FMatchPlayAuthoritativeResolveLongShotDirectDefenseRollRequest Request;
		Request.AttackSequence = Envelope.ExpectedAttackSequence;
		Request.RequestingSide = Side;
		Intent = FMatchPlayPlayerIntent::Create(EMatchPlayAuthoritativeCommandKind::ResolveLongShotDirectDefenseRoll, Request);
		break;
	}
	case EFMCodexNetworkPlayerIntentKind::LongShotDeadCornerRoll:
	{
		FMatchPlayAuthoritativeResolveLongShotDeadCornerRollRequest Request;
		Request.AttackSequence = Envelope.ExpectedAttackSequence;
		Request.RequestingSide = Side;
		Intent = FMatchPlayPlayerIntent::Create(EMatchPlayAuthoritativeCommandKind::ResolveLongShotDeadCornerRoll, Request);
		break;
	}
	case EFMCodexNetworkPlayerIntentKind::CutInsideShotDirectAttackRoll:
	{
		FMatchPlayAuthoritativeResolveCutInsideShotDirectAttackRollRequest Request;
		Request.AttackSequence = Envelope.ExpectedAttackSequence;
		Request.RequestingSide = Side;
		Intent = FMatchPlayPlayerIntent::Create(EMatchPlayAuthoritativeCommandKind::ResolveCutInsideShotDirectAttackRoll, Request);
		break;
	}
	case EFMCodexNetworkPlayerIntentKind::CutInsideShotDirectDefenseRoll:
	{
		FMatchPlayAuthoritativeResolveCutInsideShotDirectDefenseRollRequest Request;
		Request.AttackSequence = Envelope.ExpectedAttackSequence;
		Request.RequestingSide = Side;
		Intent = FMatchPlayPlayerIntent::Create(EMatchPlayAuthoritativeCommandKind::ResolveCutInsideShotDirectDefenseRoll, Request);
		break;
	}
	case EFMCodexNetworkPlayerIntentKind::CutInsideShotDeadCornerRoll:
	{
		FMatchPlayAuthoritativeResolveCutInsideShotDeadCornerRollRequest Request;
		Request.AttackSequence = Envelope.ExpectedAttackSequence;
		Request.RequestingSide = Side;
		Intent = FMatchPlayPlayerIntent::Create(EMatchPlayAuthoritativeCommandKind::ResolveCutInsideShotDeadCornerRoll, Request);
		break;
	}
	case EFMCodexNetworkPlayerIntentKind::AdvanceAfterTerminal:
	{
		FMatchPlayAuthoritativeAdvanceAfterTerminalRequest Request;
		Request.AttackSequence = Envelope.ExpectedAttackSequence;
		Request.RequestingSide = Side;
		Intent = FMatchPlayPlayerIntent::Create(EMatchPlayAuthoritativeCommandKind::AdvanceAfterTerminal, Request);
		break;
	}
	default: return Finish(AckCode::NotPlayerIntent);
	}
	IMatchPlayPlayerIntentPort& HostPort = *MatchRuntime;
	const auto Result = HostPort.SubmitPlayerIntent(Intent);
	if (!Result.bSuccess)
	{
		if (Result.ErrorCode == EMatchPlayPlayerIntentPortErrorCode::ServerCoordinatorFailed)
		{
			// Entry may already be committed. Do not leave clients on a stale actionable view.
			bTransportFault = true;
			PublishParticipantState(EFMCodexNetworkBootstrapState::BootstrapFailed);
			PublishOwnerViews(EFMCodexNetworkBootstrapState::BootstrapFailed);
			return Finish(AckCode::InternalFailure);
		}
		return Finish(AckCode::AuthorityRejected);
	}
	PublishOwnerViews(EFMCodexNetworkBootstrapState::MatchReady);
	const auto After = MatchRuntime->BuildClientView(
		Side, ViewRevision, EFMCodexNetworkBootstrapState::MatchReady);
	UE_LOG(LogFMCodexNetworkPlay, Log,
		TEXT("Intent disclosed: Match=%s Request=%lld D12=%d Branch=%d Wait=%d Revision=%d"),
		*MatchInstanceId.ToString(EGuidFormats::DigitsWithHyphensLower), Envelope.RequestId,
		After.DisclosedInitialD12, static_cast<int32>(After.EntryBranch),
		static_cast<int32>(After.EntryWait), ViewRevision);
	return Finish(AckCode::Accepted);
}
