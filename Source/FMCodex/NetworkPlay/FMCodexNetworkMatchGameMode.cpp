#include "FMCodexNetworkMatchGameMode.h"

#include "FMCodexNetworkMatchGameState.h"
#include "FMCodexNetworkMatchPlayerController.h"
#include "FMCodexNetworkMatchPlayerState.h"

#include "GameFramework/PlayerState.h"
#include "HAL/PlatformProcess.h"

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
		MatchInstanceId,
		GenerateServerSeed(MatchInstanceId));
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
	UE_LOG(LogFMCodexNetworkPlay, Log,
		TEXT("Initialized prototype network match exactly once (3+3)."));
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

int32 AFMCodexNetworkMatchGameMode::GenerateServerSeed(
	const FGuid& MatchId)
{
	return static_cast<int32>(
		MatchId.A ^ MatchId.B ^ MatchId.C ^ MatchId.D);
}

FFMCodexNetworkPlayerIntentAck AFMCodexNetworkMatchGameMode::SubmitConnectionPlayerIntent(
	AFMCodexNetworkMatchPlayerController* Controller,
	const FFMCodexNetworkPlayerIntentEnvelope& Envelope)
{
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
		Ack.ViewRevision = ViewRevision;
		UE_LOG(LogFMCodexNetworkPlay, Log,
			TEXT("Intent server: Match=%s Request=%lld Controller=%s ResolvedSide=%d ExpectedSequence=%lld ACK=%s Revision=%d->%d EntryProviderCalls=%d D12ProviderCalls=%d"),
			*Envelope.MatchInstanceId.ToString(EGuidFormats::DigitsWithHyphensLower),
			Envelope.RequestId, *GetNameSafe(Controller), static_cast<int32>(Side),
			Envelope.ExpectedAttackSequence,
			*StaticEnum<EFMCodexNetworkIntentAckCode>()->GetNameStringByValue(static_cast<int64>(Code)),
			PreviousRevision, ViewRevision,
			MatchRuntime ? MatchRuntime->GetEntryProviderInvocationCount() : 0,
			MatchRuntime ? MatchRuntime->GetD12ProviderInvocationCount() : 0);
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
	// At most two records: nonparticipants never allocate a ledger.
	if (!IntentLedgers.FindOrAdd(Controller).Consume(MatchInstanceId, Envelope))
	{
		return Finish(AckCode::DuplicateOrAlreadyResolved);
	}
	if (Envelope.IntentKind != EFMCodexNetworkPlayerIntentKind::RequestInitialActionPointRoll)
	{
		return Finish(AckCode::NotPlayerIntent);
	}
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
	if (Side != Before.CurrentAttackingSide)
	{
		return Finish(AckCode::WrongSide);
	}
	if (Before.InteractionState != EFMCodexNetworkClientInteractionState::WaitingForOwnInitialActionPoint)
	{
		return Finish(AckCode::InvalidPhase);
	}
	FMatchPlayFullD12EntryRequest Request;
	Request.RequestingSide = Side; // Exclusively the server registry, never a client claim.
	Request.ExpectedAttackSequence = Envelope.ExpectedAttackSequence;
	IMatchPlayPlayerIntentPort& HostPort = *MatchRuntime;
	const auto Result = HostPort.SubmitPlayerIntent(FMatchPlayPlayerIntent::Create(
		EMatchPlayAuthoritativeCommandKind::RequestInitialActionPointRoll, Request));
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
