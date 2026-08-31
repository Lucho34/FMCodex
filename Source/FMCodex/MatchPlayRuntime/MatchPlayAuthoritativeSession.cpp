#include "MatchPlayAuthoritativeSession.h"

namespace MatchPlayAuthoritativeSession
{
	constexpr const TCHAR* NotInitializedMessage =
		TEXT("Match play session is not initialized.");
	constexpr const TCHAR* AlreadyInitializedMessage =
		TEXT("Match play session is already initialized.");
	constexpr const TCHAR* ReentrantCommandMessage =
		TEXT("Match play session rejected a reentrant command.");
	constexpr const TCHAR* TerminalAdvanceRequiredMessage =
		TEXT("The current attack is terminal; only AdvanceAfterTerminal may progress authoritative state.");

	class FScopedCommandExecution final
	{
	public:
		explicit FScopedCommandExecution(bool& InExecutingCommand)
			: bExecutingCommand(InExecutingCommand)
		{
			bExecutingCommand = true;
		}

		~FScopedCommandExecution()
		{
			bExecutingCommand = false;
		}

	private:
		bool& bExecutingCommand;
	};
}

FMatchPlayAuthoritativeStateAdoptionResult
FMatchPlayAuthoritativeStateAdoptionPolicy::Apply(
	const FMatchPlayState& CurrentAuthoritativeState,
	const FMatchPlayState& CandidateAfterState,
	const EMatchPlayAuthoritativeStateDisposition StateDisposition)
{
	FMatchPlayAuthoritativeStateAdoptionResult Result;
	Result.bStateAdvanced =
		StateDisposition == EMatchPlayAuthoritativeStateDisposition::Adopt;
	Result.AdoptedAfterState = Result.bStateAdvanced
		? CandidateAfterState
		: CurrentAuthoritativeState;
	return Result;
}

FMatchPlayAuthoritativeSession::FMatchPlayAuthoritativeSession() = default;

FMatchPlayAuthoritativeSession::FMatchPlayAuthoritativeSession(
	IMatchPlayInitialRouteRollProvider& InInitialRouteRollProvider)
	: InitialRouteRollProvider(&InInitialRouteRollProvider)
{
}

FMatchPlayAuthoritativeSession::FMatchPlayAuthoritativeSession(
	IMatchPlayAttackEntryRollProvider& InAttackEntryRollProvider)
	: AttackEntryRollProvider(&InAttackEntryRollProvider)
{
}

FMatchPlayAuthoritativeSession::FMatchPlayAuthoritativeSession(
	IMatchPlayInitialRouteRollProvider& InInitialRouteRollProvider,
	IMatchPlayPostRouteRollProvider& InPostRouteRollProvider,
	const FSkillRuleSnapshotSet& InSkillRuleSet)
	: InitialRouteRollProvider(&InInitialRouteRollProvider)
	, PostRouteRollProvider(&InPostRouteRollProvider)
	, AuthoritativeSkillRuleSet(InSkillRuleSet)
	, bHasSkillRuleSet(true)
{
}

FMatchPlayAuthoritativeSession::FMatchPlayAuthoritativeSession(
	IMatchPlayInitialRouteRollProvider& InInitialRouteRollProvider,
	IMatchPlayPostRouteRollProvider& InPostRouteRollProvider,
	IMatchPlayRecoveryProvider& InRecoveryProvider,
	const FSkillRuleSnapshotSet& InSkillRuleSet)
	: InitialRouteRollProvider(&InInitialRouteRollProvider)
	, PostRouteRollProvider(&InPostRouteRollProvider)
	, RecoveryProvider(&InRecoveryProvider)
	, AuthoritativeSkillRuleSet(InSkillRuleSet)
	, bHasSkillRuleSet(true)
{
}

FMatchPlayAuthoritativeSession::FMatchPlayAuthoritativeSession(
	IMatchPlayAttackEntryRollProvider& InAttackEntryRollProvider,
	IMatchPlayInitialRouteRollProvider& InInitialRouteRollProvider,
	IMatchPlayPostRouteRollProvider& InPostRouteRollProvider,
	const FSkillRuleSnapshotSet& InSkillRuleSet)
	: AttackEntryRollProvider(&InAttackEntryRollProvider)
	, InitialRouteRollProvider(&InInitialRouteRollProvider)
	, PostRouteRollProvider(&InPostRouteRollProvider)
	, AuthoritativeSkillRuleSet(InSkillRuleSet)
	, bHasSkillRuleSet(true)
{
}

FMatchPlayAuthoritativeSession::FMatchPlayAuthoritativeSession(
	IMatchPlayAttackEntryRollProvider& InAttackEntryRollProvider,
	IMatchPlayInitialRouteRollProvider& InInitialRouteRollProvider,
	IMatchPlayPostRouteRollProvider& InPostRouteRollProvider,
	IMatchPlayRecoveryProvider& InRecoveryProvider,
	const FSkillRuleSnapshotSet& InSkillRuleSet)
	: AttackEntryRollProvider(&InAttackEntryRollProvider)
	, InitialRouteRollProvider(&InInitialRouteRollProvider)
	, PostRouteRollProvider(&InPostRouteRollProvider)
	, RecoveryProvider(&InRecoveryProvider)
	, AuthoritativeSkillRuleSet(InSkillRuleSet)
	, bHasSkillRuleSet(true)
{
}

FMatchPlayAuthoritativeSession::~FMatchPlayAuthoritativeSession() = default;

#if WITH_DEV_AUTOMATION_TESTS
FMatchPlayAuthoritativeSession::FMatchPlayAuthoritativeSession(
	FMatchPlayState InReconstructedState,
	IMatchPlayAttackEntryRollProvider& InAttackEntryRollProvider)
	: AuthoritativeState(MoveTemp(InReconstructedState))
	, AttackEntryRollProvider(&InAttackEntryRollProvider)
{
}

FMatchPlayAuthoritativeSession::FMatchPlayAuthoritativeSession(
	FMatchPlayState InReconstructedState,
	IMatchPlayAttackEntryRollProvider& InAttackEntryRollProvider,
	IMatchPlayRecoveryProvider& InRecoveryProvider)
	: AuthoritativeState(MoveTemp(InReconstructedState))
	, AttackEntryRollProvider(&InAttackEntryRollProvider)
	, RecoveryProvider(&InRecoveryProvider)
{
}

FMatchPlayAuthoritativeSession::FMatchPlayAuthoritativeSession(
	FMatchPlayState InReconstructedState,
	IMatchPlayAttackEntryRollProvider& InAttackEntryRollProvider,
	IMatchPlayPostRouteRollProvider& InPostRouteRollProvider)
	: AuthoritativeState(MoveTemp(InReconstructedState))
	, AttackEntryRollProvider(&InAttackEntryRollProvider)
	, PostRouteRollProvider(&InPostRouteRollProvider)
{
}

FMatchPlayAuthoritativeSession::FMatchPlayAuthoritativeSession(
	FMatchPlayState InReconstructedState,
	IMatchPlayAttackEntryRollProvider& InAttackEntryRollProvider,
	IMatchPlayPostRouteRollProvider& InPostRouteRollProvider,
	IMatchPlayRecoveryProvider& InRecoveryProvider)
	: AuthoritativeState(MoveTemp(InReconstructedState))
	, AttackEntryRollProvider(&InAttackEntryRollProvider)
	, PostRouteRollProvider(&InPostRouteRollProvider)
	, RecoveryProvider(&InRecoveryProvider)
{
}
#endif

template <typename TTypedResult, typename TExecuteDomain>
TTypedResult FMatchPlayAuthoritativeSession::ExecuteSerialized(
	const EMatchPlayAuthoritativeCommandKind CommandKind,
	const bool bRequiresInitializedState,
	const int64 CommandAttackSequence,
	TExecuteDomain&& ExecuteDomain)
{
	TTypedResult Result;
	FMatchPlayAuthoritativeRuntimeEnvelope& Envelope =
		Result.RuntimeEnvelope;
	const FMatchPlayState BeforeState = AuthoritativeState;
	Envelope.BeforeState = BeforeState;
	Envelope.AfterState = BeforeState;
	Envelope.CommandKind = CommandKind;
	Envelope.AttackSequence = CommandAttackSequence;

	if (bExecutingCommand)
	{
		Envelope.RuntimeFailureCode =
			EMatchPlayAuthoritativeRuntimeFailureCode::ReentrantCommand;
		Envelope.ErrorMessage =
			MatchPlayAuthoritativeSession::ReentrantCommandMessage;
		return Result;
	}

	MatchPlayAuthoritativeSession::FScopedCommandExecution ExecutionGuard(
		bExecutingCommand);

	const bool bIsInitialized =
		BeforeState.RuntimeState.bIsInitialized;
	if (bRequiresInitializedState && !bIsInitialized)
	{
		Envelope.RuntimeFailureCode =
			EMatchPlayAuthoritativeRuntimeFailureCode::NotInitialized;
		Envelope.ErrorMessage =
			MatchPlayAuthoritativeSession::NotInitializedMessage;
		return Result;
	}

	if (!bRequiresInitializedState && bIsInitialized)
	{
		Envelope.RuntimeFailureCode =
			EMatchPlayAuthoritativeRuntimeFailureCode::AlreadyInitialized;
		Envelope.ErrorMessage =
			MatchPlayAuthoritativeSession::AlreadyInitializedMessage;
		return Result;
	}

	if (bRequiresInitializedState
		&& BeforeState.bHasCurrentAttack
		&& BeforeState.CurrentAttack.LifecycleState
			== EMatchPlayCurrentAttackLifecycleState::TerminalPendingAdvance
		&& CommandKind
			!= EMatchPlayAuthoritativeCommandKind::AdvanceAfterTerminal)
	{
		Envelope.RuntimeFailureCode =
			EMatchPlayAuthoritativeRuntimeFailureCode
				::TerminalAdvanceRequired;
		Envelope.ErrorMessage =
			MatchPlayAuthoritativeSession
				::TerminalAdvanceRequiredMessage;
		return Result;
	}

	Envelope.bAccepted = true;
	const FDomainExecution DomainExecution =
		Forward<TExecuteDomain>(ExecuteDomain)(Result, BeforeState);
	const FMatchPlayAuthoritativeStateAdoptionResult Adoption =
		FMatchPlayAuthoritativeStateAdoptionPolicy::Apply(
			BeforeState,
			DomainExecution.CandidateAfterState,
			DomainExecution.StateDisposition);

	AuthoritativeState = Adoption.AdoptedAfterState;
	Envelope.bDomainSuccess = DomainExecution.bSuccess;
	Envelope.bStateAdvanced = Adoption.bStateAdvanced;
	Envelope.StateDisposition = DomainExecution.StateDisposition;
	Envelope.AfterState = Adoption.AdoptedAfterState;
	Envelope.AttackSequence = DomainExecution.AttackSequence;
	return Result;
}

FMatchPlayAuthoritativeInitializeMatchResult
FMatchPlayAuthoritativeSession::InitializeMatch(
	const FMatchPlayOpeningInitializeInput& Input)
{
	return ExecuteSerialized<
		FMatchPlayAuthoritativeInitializeMatchResult>(
		EMatchPlayAuthoritativeCommandKind::InitializeMatch,
		false,
		0,
		[&Input](
			FMatchPlayAuthoritativeInitializeMatchResult& Result,
			const FMatchPlayState& BeforeState)
		{
			Result.OpeningResult =
				FMatchPlayOpeningInitializer::InitializeMatchPlayOpening(
					Input);

			FDomainExecution Execution;
			Execution.bSuccess = Result.OpeningResult.bSuccess;
			Execution.CandidateAfterState =
				Result.OpeningResult.bSuccess
					? Result.OpeningResult.MatchPlayState
					: BeforeState;
			Execution.StateDisposition =
				Result.OpeningResult.bSuccess
					? EMatchPlayAuthoritativeStateDisposition::Adopt
					: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			return Execution;
		});
}

FMatchPlayAuthoritativeBeginOrdinaryAttackResult
FMatchPlayAuthoritativeSession::BeginOrdinaryAttack(
	const int32 ActionPoint)
{
	return ExecuteSerialized<
		FMatchPlayAuthoritativeBeginOrdinaryAttackResult>(
		EMatchPlayAuthoritativeCommandKind::BeginOrdinaryAttack,
		true,
		0,
		[ActionPoint](
			FMatchPlayAuthoritativeBeginOrdinaryAttackResult& Result,
			const FMatchPlayState& BeforeState)
		{
			Result.BeginResult = FMatchPlayBeginOrdinaryAttack::Begin(
				BeforeState,
				ActionPoint);

			FDomainExecution Execution;
			Execution.bSuccess = Result.BeginResult.bSuccess;
			Execution.CandidateAfterState = Result.BeginResult.AfterState;
			Execution.StateDisposition = Result.BeginResult.bSuccess
				? EMatchPlayAuthoritativeStateDisposition::Adopt
				: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence =
				Result.BeginResult.AfterState.bHasCurrentAttack
					? Result.BeginResult.AfterState.CurrentAttack
						.AttackSequence
					: 0;
			return Execution;
		});
}

FMatchPlayAuthoritativeRequestInitialActionPointRollResult
FMatchPlayAuthoritativeSession::RequestInitialActionPointRoll(
	const FMatchPlayFullD12EntryRequest& Request)
{
	return ExecuteSerialized<
		FMatchPlayAuthoritativeRequestInitialActionPointRollResult>(
		EMatchPlayAuthoritativeCommandKind::RequestInitialActionPointRoll,
		true,
		Request.ExpectedAttackSequence,
		[this, &Request](
			FMatchPlayAuthoritativeRequestInitialActionPointRollResult& Result,
			const FMatchPlayState& BeforeState)
		{
			Result.EntryResult = FMatchPlayFullD12Entry::Enter(
				BeforeState,
				Request,
				AttackEntryRollProvider);

			FDomainExecution Execution;
			Execution.bSuccess = Result.EntryResult.bSuccess;
			Execution.CandidateAfterState = Result.EntryResult.AfterState;
			Execution.StateDisposition = Result.EntryResult.bSuccess
				? EMatchPlayAuthoritativeStateDisposition::Adopt
				: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence =
				Result.EntryResult.AuthoritativeAttackSequence;
			return Execution;
		});
}

FMatchPlayAuthoritativeRequestSetPieceTypeRollResult
FMatchPlayAuthoritativeSession::RequestSetPieceTypeRoll(
	const FMatchPlaySetPieceTypeRollRequest& Request)
{
	return ExecuteSerialized<
		FMatchPlayAuthoritativeRequestSetPieceTypeRollResult>(
		EMatchPlayAuthoritativeCommandKind::RequestSetPieceTypeRoll,
		true,
		Request.AttackSequence,
		[this, &Request](
			FMatchPlayAuthoritativeRequestSetPieceTypeRollResult& Result,
			const FMatchPlayState& BeforeState)
		{
			Result.TypeRollResult = FMatchPlaySetPieceTypeRoll::Resolve(
				BeforeState,
				Request,
				AttackEntryRollProvider);

			FDomainExecution Execution;
			Execution.bSuccess = Result.TypeRollResult.bSuccess;
			Execution.CandidateAfterState =
				Result.TypeRollResult.AfterState;
			Execution.StateDisposition = Result.TypeRollResult.bSuccess
				? EMatchPlayAuthoritativeStateDisposition::Adopt
				: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence = Request.AttackSequence;
			return Execution;
		});
}

FMatchPlayAuthoritativeResolveSendingOffResult
FMatchPlayAuthoritativeSession::ResolveSendingOff(
	const FMatchPlaySendingOffResolutionRequest& Request)
{
	return ExecuteSerialized<FMatchPlayAuthoritativeResolveSendingOffResult>(
		EMatchPlayAuthoritativeCommandKind::ResolveSendingOff,
		true,
		Request.AttackSequence,
		[this, &Request](
			FMatchPlayAuthoritativeResolveSendingOffResult& Result,
			const FMatchPlayState& BeforeState)
		{
			Result.ResolutionResult = FMatchPlaySendingOffResolution::Resolve(
				BeforeState,
				Request,
				AttackEntryRollProvider);

			FDomainExecution Execution;
			Execution.bSuccess = Result.ResolutionResult.bSuccess;
			Execution.CandidateAfterState =
				Result.ResolutionResult.AfterState;
			Execution.StateDisposition = Result.ResolutionResult.bSuccess
				? EMatchPlayAuthoritativeStateDisposition::Adopt
				: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence = Request.AttackSequence;
			return Execution;
		});
}

FMatchPlayAuthoritativeSubmitSetPieceCarrierResult
FMatchPlayAuthoritativeSession::SubmitSetPieceCarrier(
	const FMatchPlaySetPieceCarrierSelectionRequest& Request)
{
	return ExecuteSerialized<
		FMatchPlayAuthoritativeSubmitSetPieceCarrierResult>(
		EMatchPlayAuthoritativeCommandKind::SubmitSetPieceCarrier,
		true,
		Request.AttackSequence,
		[&Request](
			FMatchPlayAuthoritativeSubmitSetPieceCarrierResult& Result,
			const FMatchPlayState& BeforeState)
		{
			Result.CarrierResult =
				FMatchPlaySetPieceCarrierSelection::Submit(
					BeforeState,
					Request);

			FDomainExecution Execution;
			Execution.bSuccess = Result.CarrierResult.bSuccess;
			Execution.CandidateAfterState =
				Result.CarrierResult.AfterState;
			Execution.StateDisposition = Result.CarrierResult.bSuccess
				? EMatchPlayAuthoritativeStateDisposition::Adopt
				: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence = Request.AttackSequence;
			return Execution;
		});
}

FMatchPlayAuthoritativeSubmitShortFreeKickMethodResult
FMatchPlayAuthoritativeSession::SubmitShortFreeKickMethod(
	const FMatchPlayShortFreeKickMethodRequest& Request)
{
	return ExecuteSerialized<
		FMatchPlayAuthoritativeSubmitShortFreeKickMethodResult>(
		EMatchPlayAuthoritativeCommandKind::SubmitShortFreeKickMethod,
		true,
		Request.AttackSequence,
		[&Request](
			FMatchPlayAuthoritativeSubmitShortFreeKickMethodResult& Result,
			const FMatchPlayState& BeforeState)
		{
			Result.ResolutionResult =
				FMatchPlayShortFreeKickResolution::SubmitMethod(
					BeforeState,
					Request);
			FDomainExecution Execution;
			Execution.bSuccess = Result.ResolutionResult.bSuccess;
			Execution.CandidateAfterState = Result.ResolutionResult.AfterState;
			Execution.StateDisposition = Result.ResolutionResult.bSuccess
				? EMatchPlayAuthoritativeStateDisposition::Adopt
				: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence = Request.AttackSequence;
			return Execution;
		});
}

FMatchPlayAuthoritativeResolveShortFreeKickDirectAttackRollResult
FMatchPlayAuthoritativeSession::ResolveShortFreeKickDirectAttackRoll(
	const FMatchPlayShortFreeKickRollRequest& Request)
{
	return ExecuteSerialized<
		FMatchPlayAuthoritativeResolveShortFreeKickDirectAttackRollResult>(
		EMatchPlayAuthoritativeCommandKind
			::ResolveShortFreeKickDirectAttackRoll,
		true,
		Request.AttackSequence,
		[this, &Request](auto& Result, const FMatchPlayState& BeforeState)
		{
			Result.ResolutionResult =
				FMatchPlayShortFreeKickResolution::ResolveDirectAttackRoll(
					BeforeState,
					Request,
					PostRouteRollProvider);
			FDomainExecution Execution;
			Execution.bSuccess = Result.ResolutionResult.bSuccess;
			Execution.CandidateAfterState = Result.ResolutionResult.AfterState;
			Execution.StateDisposition = Result.ResolutionResult.bSuccess
				? EMatchPlayAuthoritativeStateDisposition::Adopt
				: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence = Request.AttackSequence;
			return Execution;
		});
}

FMatchPlayAuthoritativeResolveShortFreeKickDirectDefenseRollResult
FMatchPlayAuthoritativeSession::ResolveShortFreeKickDirectDefenseRoll(
	const FMatchPlayShortFreeKickRollRequest& Request)
{
	return ExecuteSerialized<
		FMatchPlayAuthoritativeResolveShortFreeKickDirectDefenseRollResult>(
		EMatchPlayAuthoritativeCommandKind
			::ResolveShortFreeKickDirectDefenseRoll,
		true,
		Request.AttackSequence,
		[this, &Request](auto& Result, const FMatchPlayState& BeforeState)
		{
			Result.ResolutionResult =
				FMatchPlayShortFreeKickResolution::ResolveDirectDefenseRoll(
					BeforeState,
					Request,
					PostRouteRollProvider);
			FDomainExecution Execution;
			Execution.bSuccess = Result.ResolutionResult.bSuccess;
			Execution.CandidateAfterState = Result.ResolutionResult.AfterState;
			Execution.StateDisposition = Result.ResolutionResult.bSuccess
				? EMatchPlayAuthoritativeStateDisposition::Adopt
				: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence = Request.AttackSequence;
			return Execution;
		});
}

FMatchPlayAuthoritativeResolveShortFreeKickAngledRollResult
FMatchPlayAuthoritativeSession::ResolveShortFreeKickAngledRoll(
	const FMatchPlayShortFreeKickRollRequest& Request)
{
	return ExecuteSerialized<
		FMatchPlayAuthoritativeResolveShortFreeKickAngledRollResult>(
		EMatchPlayAuthoritativeCommandKind::ResolveShortFreeKickAngledRoll,
		true,
		Request.AttackSequence,
		[this, &Request](auto& Result, const FMatchPlayState& BeforeState)
		{
			Result.ResolutionResult =
				FMatchPlayShortFreeKickResolution::ResolveAngledRoll(
					BeforeState,
					Request,
					PostRouteRollProvider);
			FDomainExecution Execution;
			Execution.bSuccess = Result.ResolutionResult.bSuccess;
			Execution.CandidateAfterState = Result.ResolutionResult.AfterState;
			Execution.StateDisposition = Result.ResolutionResult.bSuccess
				? EMatchPlayAuthoritativeStateDisposition::Adopt
				: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence = Request.AttackSequence;
			return Execution;
		});
}

FMatchPlayAuthoritativeResolveNoLegalSetPieceCarrierResult
FMatchPlayAuthoritativeSession::ResolveNoLegalSetPieceCarrier(
	const FMatchPlayShortFreeKickNoLegalCarrierRequest& Request)
{
	return ExecuteSerialized<
		FMatchPlayAuthoritativeResolveNoLegalSetPieceCarrierResult>(
		EMatchPlayAuthoritativeCommandKind::ResolveNoLegalSetPieceCarrier,
		true,
		Request.AttackSequence,
		[&Request](auto& Result, const FMatchPlayState& BeforeState)
		{
			Result.ResolutionResult =
				FMatchPlayShortFreeKickResolution::ResolveNoLegalCarrier(
					BeforeState,
					Request);
			FDomainExecution Execution;
			Execution.bSuccess = Result.ResolutionResult.bSuccess;
			Execution.CandidateAfterState = Result.ResolutionResult.AfterState;
			Execution.StateDisposition = Result.ResolutionResult.bSuccess
				? EMatchPlayAuthoritativeStateDisposition::Adopt
				: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence = Request.AttackSequence;
			return Execution;
		});
}

FMatchPlayAuthoritativeSubmitLongFreeKickMethodResult
FMatchPlayAuthoritativeSession::SubmitLongFreeKickMethod(
	const FMatchPlayLongFreeKickMethodRequest& Request)
{
	return ExecuteSerialized<
		FMatchPlayAuthoritativeSubmitLongFreeKickMethodResult>(
		EMatchPlayAuthoritativeCommandKind::SubmitLongFreeKickMethod,
		true,
		Request.AttackSequence,
		[&Request](auto& Result, const FMatchPlayState& BeforeState)
		{
			Result.ResolutionResult =
				FMatchPlayLongFreeKickResolution::SubmitMethod(
					BeforeState, Request);
			FDomainExecution Execution;
			Execution.bSuccess = Result.ResolutionResult.bSuccess;
			Execution.CandidateAfterState = Result.ResolutionResult.AfterState;
			Execution.StateDisposition = Result.ResolutionResult.bSuccess
				? EMatchPlayAuthoritativeStateDisposition::Adopt
				: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence = Request.AttackSequence;
			return Execution;
		});
}

FMatchPlayAuthoritativeResolveLongFreeKickDirectAttackRollResult
FMatchPlayAuthoritativeSession::ResolveLongFreeKickDirectAttackRoll(
	const FMatchPlayLongFreeKickRollRequest& Request)
{
	return ExecuteSerialized<
		FMatchPlayAuthoritativeResolveLongFreeKickDirectAttackRollResult>(
		EMatchPlayAuthoritativeCommandKind
			::ResolveLongFreeKickDirectAttackRoll,
		true,
		Request.AttackSequence,
		[this, &Request](auto& Result, const FMatchPlayState& BeforeState)
		{
			Result.ResolutionResult =
				FMatchPlayLongFreeKickResolution::ResolveDirectAttackRoll(
					BeforeState, Request, PostRouteRollProvider);
			FDomainExecution Execution;
			Execution.bSuccess = Result.ResolutionResult.bSuccess;
			Execution.CandidateAfterState = Result.ResolutionResult.AfterState;
			Execution.StateDisposition = Result.ResolutionResult.bSuccess
				? EMatchPlayAuthoritativeStateDisposition::Adopt
				: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence = Request.AttackSequence;
			return Execution;
		});
}

FMatchPlayAuthoritativeResolveLongFreeKickDirectDefenseRollResult
FMatchPlayAuthoritativeSession::ResolveLongFreeKickDirectDefenseRoll(
	const FMatchPlayLongFreeKickRollRequest& Request)
{
	return ExecuteSerialized<
		FMatchPlayAuthoritativeResolveLongFreeKickDirectDefenseRollResult>(
		EMatchPlayAuthoritativeCommandKind
			::ResolveLongFreeKickDirectDefenseRoll,
		true,
		Request.AttackSequence,
		[this, &Request](auto& Result, const FMatchPlayState& BeforeState)
		{
			Result.ResolutionResult =
				FMatchPlayLongFreeKickResolution::ResolveDirectDefenseRoll(
					BeforeState, Request, PostRouteRollProvider);
			FDomainExecution Execution;
			Execution.bSuccess = Result.ResolutionResult.bSuccess;
			Execution.CandidateAfterState = Result.ResolutionResult.AfterState;
			Execution.StateDisposition = Result.ResolutionResult.bSuccess
				? EMatchPlayAuthoritativeStateDisposition::Adopt
				: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence = Request.AttackSequence;
			return Execution;
		});
}

FMatchPlayAuthoritativeResolveLongFreeKickPowerRollResult
FMatchPlayAuthoritativeSession::ResolveLongFreeKickPowerRoll(
	const FMatchPlayLongFreeKickRollRequest& Request)
{
	return ExecuteSerialized<
		FMatchPlayAuthoritativeResolveLongFreeKickPowerRollResult>(
		EMatchPlayAuthoritativeCommandKind::ResolveLongFreeKickPowerRoll,
		true,
		Request.AttackSequence,
		[this, &Request](auto& Result, const FMatchPlayState& BeforeState)
		{
			Result.ResolutionResult =
				FMatchPlayLongFreeKickResolution::ResolvePowerRoll(
					BeforeState, Request, PostRouteRollProvider);
			FDomainExecution Execution;
			Execution.bSuccess = Result.ResolutionResult.bSuccess;
			Execution.CandidateAfterState = Result.ResolutionResult.AfterState;
			Execution.StateDisposition = Result.ResolutionResult.bSuccess
				? EMatchPlayAuthoritativeStateDisposition::Adopt
				: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence = Request.AttackSequence;
			return Execution;
		});
}

FMatchPlayAuthoritativeResolveNoLegalLongFreeKickCarrierResult
FMatchPlayAuthoritativeSession::ResolveNoLegalSetPieceCarrier(
	const FMatchPlayLongFreeKickNoLegalCarrierRequest& Request)
{
	return ExecuteSerialized<
		FMatchPlayAuthoritativeResolveNoLegalLongFreeKickCarrierResult>(
		EMatchPlayAuthoritativeCommandKind::ResolveNoLegalSetPieceCarrier,
		true,
		Request.AttackSequence,
		[&Request](auto& Result, const FMatchPlayState& BeforeState)
		{
			Result.ResolutionResult =
				FMatchPlayLongFreeKickResolution::ResolveNoLegalCarrier(
					BeforeState, Request);
			FDomainExecution Execution;
			Execution.bSuccess = Result.ResolutionResult.bSuccess;
			Execution.CandidateAfterState = Result.ResolutionResult.AfterState;
			Execution.StateDisposition = Result.ResolutionResult.bSuccess
				? EMatchPlayAuthoritativeStateDisposition::Adopt
				: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence = Request.AttackSequence;
			return Execution;
		});
}

FMatchPlayAuthoritativeSubmitPenaltyMethodResult
FMatchPlayAuthoritativeSession::SubmitPenaltyMethod(
	const FMatchPlayPenaltyMethodRequest& Request)
{
	return ExecuteSerialized<FMatchPlayAuthoritativeSubmitPenaltyMethodResult>(
		EMatchPlayAuthoritativeCommandKind::SubmitPenaltyMethod,
		true,
		Request.AttackSequence,
		[&Request](auto& Result, const FMatchPlayState& BeforeState)
		{
			Result.ResolutionResult = FMatchPlayPenaltyResolution::SubmitMethod(
				BeforeState, Request);
			FDomainExecution Execution;
			Execution.bSuccess = Result.ResolutionResult.bSuccess;
			Execution.CandidateAfterState = Result.ResolutionResult.AfterState;
			Execution.StateDisposition = Result.ResolutionResult.bSuccess
				? EMatchPlayAuthoritativeStateDisposition::Adopt
				: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence = Request.AttackSequence;
			return Execution;
		});
}

FMatchPlayAuthoritativeResolvePenaltyDirectAttackRollResult
FMatchPlayAuthoritativeSession::ResolvePenaltyDirectAttackRoll(
	const FMatchPlayPenaltyRollRequest& Request)
{
	return ExecuteSerialized<
		FMatchPlayAuthoritativeResolvePenaltyDirectAttackRollResult>(
		EMatchPlayAuthoritativeCommandKind::ResolvePenaltyDirectAttackRoll,
		true,
		Request.AttackSequence,
		[this, &Request](auto& Result, const FMatchPlayState& BeforeState)
		{
			Result.ResolutionResult =
				FMatchPlayPenaltyResolution::ResolveDirectAttackRoll(
					BeforeState, Request, PostRouteRollProvider);
			FDomainExecution Execution;
			Execution.bSuccess = Result.ResolutionResult.bSuccess;
			Execution.CandidateAfterState = Result.ResolutionResult.AfterState;
			Execution.StateDisposition = Result.ResolutionResult.bSuccess
				? EMatchPlayAuthoritativeStateDisposition::Adopt
				: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence = Request.AttackSequence;
			return Execution;
		});
}

FMatchPlayAuthoritativeResolvePenaltyDirectDefenseRollResult
FMatchPlayAuthoritativeSession::ResolvePenaltyDirectDefenseRoll(
	const FMatchPlayPenaltyRollRequest& Request)
{
	return ExecuteSerialized<
		FMatchPlayAuthoritativeResolvePenaltyDirectDefenseRollResult>(
		EMatchPlayAuthoritativeCommandKind::ResolvePenaltyDirectDefenseRoll,
		true,
		Request.AttackSequence,
		[this, &Request](auto& Result, const FMatchPlayState& BeforeState)
		{
			Result.ResolutionResult =
				FMatchPlayPenaltyResolution::ResolveDirectDefenseRoll(
					BeforeState, Request, PostRouteRollProvider);
			FDomainExecution Execution;
			Execution.bSuccess = Result.ResolutionResult.bSuccess;
			Execution.CandidateAfterState = Result.ResolutionResult.AfterState;
			Execution.StateDisposition = Result.ResolutionResult.bSuccess
				? EMatchPlayAuthoritativeStateDisposition::Adopt
				: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence = Request.AttackSequence;
			return Execution;
		});
}

FMatchPlayAuthoritativeResolvePenaltyPanenkaRollResult
FMatchPlayAuthoritativeSession::ResolvePenaltyPanenkaRoll(
	const FMatchPlayPenaltyRollRequest& Request)
{
	return ExecuteSerialized<
		FMatchPlayAuthoritativeResolvePenaltyPanenkaRollResult>(
		EMatchPlayAuthoritativeCommandKind::ResolvePenaltyPanenkaRoll,
		true,
		Request.AttackSequence,
		[this, &Request](auto& Result, const FMatchPlayState& BeforeState)
		{
			Result.ResolutionResult =
				FMatchPlayPenaltyResolution::ResolvePanenkaRoll(
					BeforeState, Request, PostRouteRollProvider);
			FDomainExecution Execution;
			Execution.bSuccess = Result.ResolutionResult.bSuccess;
			Execution.CandidateAfterState = Result.ResolutionResult.AfterState;
			Execution.StateDisposition = Result.ResolutionResult.bSuccess
				? EMatchPlayAuthoritativeStateDisposition::Adopt
				: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence = Request.AttackSequence;
			return Execution;
		});
}

FMatchPlayAuthoritativeResolveNoLegalPenaltyCarrierResult
FMatchPlayAuthoritativeSession::ResolveNoLegalSetPieceCarrier(
	const FMatchPlayPenaltyNoLegalCarrierRequest& Request)
{
	return ExecuteSerialized<
		FMatchPlayAuthoritativeResolveNoLegalPenaltyCarrierResult>(
		EMatchPlayAuthoritativeCommandKind::ResolveNoLegalSetPieceCarrier,
		true,
		Request.AttackSequence,
		[&Request](auto& Result, const FMatchPlayState& BeforeState)
		{
			Result.ResolutionResult =
				FMatchPlayPenaltyResolution::ResolveNoLegalCarrier(
					BeforeState, Request);
			FDomainExecution Execution;
			Execution.bSuccess = Result.ResolutionResult.bSuccess;
			Execution.CandidateAfterState = Result.ResolutionResult.AfterState;
			Execution.StateDisposition = Result.ResolutionResult.bSuccess
				? EMatchPlayAuthoritativeStateDisposition::Adopt
				: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence = Request.AttackSequence;
			return Execution;
		});
}

FMatchPlayAuthoritativeSubmitCornerAttackerNominationsResult
FMatchPlayAuthoritativeSession::SubmitCornerAttackerNominations(
	const FMatchPlayCornerNominationRequest& Request)
{
	return ExecuteSerialized<
		FMatchPlayAuthoritativeSubmitCornerAttackerNominationsResult>(
		EMatchPlayAuthoritativeCommandKind
			::SubmitCornerAttackerNominations,
		true,
		Request.AttackSequence,
		[&Request](auto& Result, const FMatchPlayState& BeforeState)
		{
			Result.ResolutionResult =
				FMatchPlayCornerResolution::SubmitAttackerNominations(
					BeforeState, Request);
			FDomainExecution Execution;
			Execution.bSuccess = Result.ResolutionResult.bSuccess;
			Execution.CandidateAfterState = Result.ResolutionResult.AfterState;
			Execution.StateDisposition = Result.ResolutionResult.bSuccess
				? EMatchPlayAuthoritativeStateDisposition::Adopt
				: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence = Request.AttackSequence;
			return Execution;
		});
}

FMatchPlayAuthoritativeSubmitCornerDefenderNominationsResult
FMatchPlayAuthoritativeSession::SubmitCornerDefenderNominations(
	const FMatchPlayCornerNominationRequest& Request)
{
	return ExecuteSerialized<
		FMatchPlayAuthoritativeSubmitCornerDefenderNominationsResult>(
		EMatchPlayAuthoritativeCommandKind
			::SubmitCornerDefenderNominations,
		true,
		Request.AttackSequence,
		[&Request](auto& Result, const FMatchPlayState& BeforeState)
		{
			Result.ResolutionResult =
				FMatchPlayCornerResolution::SubmitDefenderNominations(
					BeforeState, Request);
			FDomainExecution Execution;
			Execution.bSuccess = Result.ResolutionResult.bSuccess;
			Execution.CandidateAfterState = Result.ResolutionResult.AfterState;
			Execution.StateDisposition = Result.ResolutionResult.bSuccess
				? EMatchPlayAuthoritativeStateDisposition::Adopt
				: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence = Request.AttackSequence;
			return Execution;
		});
}

FMatchPlayAuthoritativeRequestCornerParticipantSelectionRollResult
FMatchPlayAuthoritativeSession::RequestCornerParticipantSelectionRoll(
	const FMatchPlayCornerRollRequest& Request)
{
	return ExecuteSerialized<
		FMatchPlayAuthoritativeRequestCornerParticipantSelectionRollResult>(
		EMatchPlayAuthoritativeCommandKind
			::RequestCornerParticipantSelectionRoll,
		true,
		Request.AttackSequence,
		[this, &Request](auto& Result, const FMatchPlayState& BeforeState)
		{
			Result.ResolutionResult =
				FMatchPlayCornerResolution::RequestParticipantSelectionRoll(
					BeforeState, Request, PostRouteRollProvider);
			FDomainExecution Execution;
			Execution.bSuccess = Result.ResolutionResult.bSuccess;
			Execution.CandidateAfterState = Result.ResolutionResult.AfterState;
			Execution.StateDisposition = Result.ResolutionResult.bSuccess
				? EMatchPlayAuthoritativeStateDisposition::Adopt
				: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence = Request.AttackSequence;
			return Execution;
		});
}

FMatchPlayAuthoritativeSubmitCornerIntentResult
FMatchPlayAuthoritativeSession::SubmitCornerIntent(
	const FMatchPlayCornerIntentRequest& Request)
{
	return ExecuteSerialized<
		FMatchPlayAuthoritativeSubmitCornerIntentResult>(
		EMatchPlayAuthoritativeCommandKind::SubmitCornerIntent,
		true,
		Request.AttackSequence,
		[&Request](auto& Result, const FMatchPlayState& BeforeState)
		{
			Result.ResolutionResult = FMatchPlayCornerResolution::SubmitIntent(
				BeforeState, Request);
			FDomainExecution Execution;
			Execution.bSuccess = Result.ResolutionResult.bSuccess;
			Execution.CandidateAfterState = Result.ResolutionResult.AfterState;
			Execution.StateDisposition = Result.ResolutionResult.bSuccess
				? EMatchPlayAuthoritativeStateDisposition::Adopt
				: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence = Request.AttackSequence;
			return Execution;
		});
}

FMatchPlayAuthoritativeRequestCornerRouteRollResult
FMatchPlayAuthoritativeSession::RequestCornerRouteRoll(
	const FMatchPlayCornerRollRequest& Request)
{
	return ExecuteSerialized<
		FMatchPlayAuthoritativeRequestCornerRouteRollResult>(
		EMatchPlayAuthoritativeCommandKind::RequestCornerRouteRoll,
		true,
		Request.AttackSequence,
		[this, &Request](auto& Result, const FMatchPlayState& BeforeState)
		{
			Result.ResolutionResult =
				FMatchPlayCornerResolution::RequestRouteRoll(
					BeforeState, Request, PostRouteRollProvider);
			FDomainExecution Execution;
			Execution.bSuccess = Result.ResolutionResult.bSuccess;
			Execution.CandidateAfterState = Result.ResolutionResult.AfterState;
			Execution.StateDisposition = Result.ResolutionResult.bSuccess
				? EMatchPlayAuthoritativeStateDisposition::Adopt
				: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence = Request.AttackSequence;
			return Execution;
		});
}

FMatchPlayAuthoritativeRequestCornerAttackRollResult
FMatchPlayAuthoritativeSession::RequestCornerAttackRoll(
	const FMatchPlayCornerRollRequest& Request)
{
	return ExecuteSerialized<
		FMatchPlayAuthoritativeRequestCornerAttackRollResult>(
		EMatchPlayAuthoritativeCommandKind::RequestCornerAttackRoll,
		true,
		Request.AttackSequence,
		[this, &Request](auto& Result, const FMatchPlayState& BeforeState)
		{
			Result.ResolutionResult =
				FMatchPlayCornerResolution::RequestAttackRoll(
					BeforeState, Request, PostRouteRollProvider);
			FDomainExecution Execution;
			Execution.bSuccess = Result.ResolutionResult.bSuccess;
			Execution.CandidateAfterState = Result.ResolutionResult.AfterState;
			Execution.StateDisposition = Result.ResolutionResult.bSuccess
				? EMatchPlayAuthoritativeStateDisposition::Adopt
				: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence = Request.AttackSequence;
			return Execution;
		});
}

FMatchPlayAuthoritativeRequestCornerDefenseRollResult
FMatchPlayAuthoritativeSession::RequestCornerDefenseRoll(
	const FMatchPlayCornerRollRequest& Request)
{
	return ExecuteSerialized<
		FMatchPlayAuthoritativeRequestCornerDefenseRollResult>(
		EMatchPlayAuthoritativeCommandKind::RequestCornerDefenseRoll,
		true,
		Request.AttackSequence,
		[this, &Request](auto& Result, const FMatchPlayState& BeforeState)
		{
			Result.ResolutionResult =
				FMatchPlayCornerResolution::RequestDefenseRoll(
					BeforeState, Request, PostRouteRollProvider);
			FDomainExecution Execution;
			Execution.bSuccess = Result.ResolutionResult.bSuccess;
			Execution.CandidateAfterState = Result.ResolutionResult.AfterState;
			Execution.StateDisposition = Result.ResolutionResult.bSuccess
				? EMatchPlayAuthoritativeStateDisposition::Adopt
				: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence = Request.AttackSequence;
			return Execution;
		});
}

FMatchPlayAuthoritativeFinishDeploymentResult
FMatchPlayAuthoritativeSession::FinishDeployment(
	const int64 AttackSequence,
	const EInitialTurnOrderPlayer RequestingSide)
{
	return ExecuteSerialized<
		FMatchPlayAuthoritativeFinishDeploymentResult>(
		EMatchPlayAuthoritativeCommandKind::FinishDeployment,
		true,
		AttackSequence,
		[AttackSequence, RequestingSide](
			FMatchPlayAuthoritativeFinishDeploymentResult& Result,
			const FMatchPlayState& BeforeState)
		{
			Result.FinishResult = FMatchPlayFinishDeployment::Finish(
				BeforeState,
				AttackSequence,
				RequestingSide);

			FDomainExecution Execution;
			Execution.bSuccess = Result.FinishResult.bSuccess;
			Execution.CandidateAfterState = Result.FinishResult.AfterState;
			Execution.StateDisposition = Result.FinishResult.bSuccess
				? EMatchPlayAuthoritativeStateDisposition::Adopt
				: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence =
				Result.FinishResult.AttackSequence;
			return Execution;
		});
}

FMatchPlayAuthoritativeDeployOrdinaryResult
FMatchPlayAuthoritativeSession::DeployOrdinary(
	const FMatchPlayAuthoritativeDeployOrdinaryRequest& Request)
{
	const int64 AttackSequence = AuthoritativeState.bHasCurrentAttack
		? AuthoritativeState.CurrentAttack.AttackSequence
		: 0;
	return ExecuteSerialized<
		FMatchPlayAuthoritativeDeployOrdinaryResult>(
		EMatchPlayAuthoritativeCommandKind::DeployOrdinary,
		true,
		AttackSequence,
		[Request, AttackSequence](
			FMatchPlayAuthoritativeDeployOrdinaryResult& Result,
			const FMatchPlayState& BeforeState)
		{
			FMatchPlayOrdinaryDeploymentRequest DomainRequest;
			DomainRequest.AttackSequence = AttackSequence;
			DomainRequest.RequestingSide = Request.RequestingSide;
			DomainRequest.CardId = Request.CardId;
			DomainRequest.SlotId = Request.SlotId;
			Result.DeploymentResult =
				FMatchPlayOrdinaryDeploymentWriter::Deploy(
					BeforeState,
					DomainRequest);

			FDomainExecution Execution;
			Execution.bSuccess = Result.DeploymentResult.bSuccess;
			Execution.CandidateAfterState =
				Result.DeploymentResult.AfterState;
			Execution.StateDisposition =
				Result.DeploymentResult.bSuccess
					? EMatchPlayAuthoritativeStateDisposition::Adopt
					: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence = AttackSequence;
			return Execution;
		});
}

FMatchPlayAuthoritativeDeployGoalkeeperResult
FMatchPlayAuthoritativeSession::DeployGoalkeeper(
	const FMatchPlayAuthoritativeDeployGoalkeeperRequest& Request)
{
	const int64 AttackSequence = AuthoritativeState.bHasCurrentAttack
		? AuthoritativeState.CurrentAttack.AttackSequence
		: 0;
	const EInitialTurnOrderPlayer RequestingSide =
		AuthoritativeState.bHasCurrentAttack
			? AuthoritativeState.CurrentAttack.CurrentLegalDeploymentSide
			: EInitialTurnOrderPlayer::None;
	const FName GoalkeeperCardId = RequestingSide
		== EInitialTurnOrderPlayer::PlayerA
			? FName(*AuthoritativeState.RuntimeState.PlayerAState.GoalkeeperCardId)
			: RequestingSide == EInitialTurnOrderPlayer::PlayerB
				? FName(*AuthoritativeState.RuntimeState.PlayerBState.GoalkeeperCardId)
				: NAME_None;
	return ExecuteSerialized<
		FMatchPlayAuthoritativeDeployGoalkeeperResult>(
		EMatchPlayAuthoritativeCommandKind::DeployGoalkeeper,
		true,
		AttackSequence,
		[Request, AttackSequence, RequestingSide, GoalkeeperCardId](
			FMatchPlayAuthoritativeDeployGoalkeeperResult& Result,
			const FMatchPlayState& BeforeState)
		{
			Result.AvailabilityResult =
				FMatchPlayGoalkeeperDeploymentAvailability::Query(
					BeforeState,
					AttackSequence,
					RequestingSide,
					GoalkeeperCardId);

			FMatchPlayGoalkeeperDeploymentRequest DomainRequest;
			DomainRequest.AttackSequence = AttackSequence;
			DomainRequest.RequestingSide = RequestingSide;
			DomainRequest.CardId = GoalkeeperCardId;
			DomainRequest.SlotId = Request.SlotId;
			Result.DeploymentResult =
				FMatchPlayGoalkeeperDeploymentWriter::Deploy(
					BeforeState,
					DomainRequest);

			FDomainExecution Execution;
			Execution.bSuccess = Result.DeploymentResult.bSucceeded;
			Execution.CandidateAfterState =
				Result.DeploymentResult.AfterState;
			Execution.StateDisposition = Result.DeploymentResult.bSucceeded
				? EMatchPlayAuthoritativeStateDisposition::Adopt
				: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence = AttackSequence;
			return Execution;
		});
}

FMatchPlayAuthoritativeSubmitCarrierResult
FMatchPlayAuthoritativeSession::SubmitCarrier(
	const FMatchPlayAuthoritativeSubmitCarrierRequest& Request)
{
	const int64 AttackSequence = AuthoritativeState.bHasCurrentAttack
		? AuthoritativeState.CurrentAttack.AttackSequence
		: 0;
	return ExecuteSerialized<
		FMatchPlayAuthoritativeSubmitCarrierResult>(
		EMatchPlayAuthoritativeCommandKind::SubmitCarrier,
		true,
		AttackSequence,
		[Request, AttackSequence](
			FMatchPlayAuthoritativeSubmitCarrierResult& Result,
			const FMatchPlayState& BeforeState)
		{
			FMatchPlayCurrentAttackCarrierSelectionRequest DomainRequest;
			DomainRequest.AttackSequence = AttackSequence;
			DomainRequest.RequestingSide = Request.RequestingSide;
			DomainRequest.CarrierCardId = Request.CarrierCardId;
			Result.CarrierResult =
				FMatchPlayCurrentAttackCarrierSelectionWriter::Select(
					BeforeState,
					DomainRequest);

			FDomainExecution Execution;
			Execution.bSuccess = Result.CarrierResult.bSuccess;
			Execution.CandidateAfterState = Result.CarrierResult.AfterState;
			Execution.StateDisposition = Result.CarrierResult.bSuccess
				? EMatchPlayAuthoritativeStateDisposition::Adopt
				: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence = AttackSequence;
			return Execution;
		});
}

FMatchPlayAuthoritativeResolveNoLegalCarrierResult
FMatchPlayAuthoritativeSession::ResolveNoLegalCarrier()
{
	const int64 AttackSequence = AuthoritativeState.bHasCurrentAttack
		? AuthoritativeState.CurrentAttack.AttackSequence
		: 0;
	return ExecuteSerialized<
		FMatchPlayAuthoritativeResolveNoLegalCarrierResult>(
		EMatchPlayAuthoritativeCommandKind::ResolveNoLegalCarrier,
		true,
		AttackSequence,
		[AttackSequence](
			FMatchPlayAuthoritativeResolveNoLegalCarrierResult& Result,
			const FMatchPlayState& BeforeState)
		{
			Result.ResolutionResult =
				FMatchPlayResolveNoLegalCarrier::Resolve(BeforeState);

			FDomainExecution Execution;
			Execution.bSuccess = Result.ResolutionResult.bSuccess;
			Execution.CandidateAfterState =
				Result.ResolutionResult.AfterState;
			Execution.StateDisposition = Result.ResolutionResult.bSuccess
				? EMatchPlayAuthoritativeStateDisposition::Adopt
				: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence = AttackSequence;
			return Execution;
		});
}

FMatchPlayAuthoritativeSubmitMarkerResult
FMatchPlayAuthoritativeSession::SubmitMarker(
	const FMatchPlayAuthoritativeSubmitMarkerRequest& Request)
{
	const int64 AttackSequence = AuthoritativeState.bHasCurrentAttack
		? AuthoritativeState.CurrentAttack.AttackSequence
		: 0;
	return ExecuteSerialized<FMatchPlayAuthoritativeSubmitMarkerResult>(
		EMatchPlayAuthoritativeCommandKind::SubmitMarker,
		true,
		AttackSequence,
		[this, Request, AttackSequence](
			FMatchPlayAuthoritativeSubmitMarkerResult& Result,
			const FMatchPlayState& BeforeState)
		{
			FMatchPlayCurrentAttackMarkerSelectionRequest DomainRequest;
			DomainRequest.AttackSequence = AttackSequence;
			DomainRequest.RequestingSide = Request.RequestingSide;
			DomainRequest.MarkerCardId = Request.MarkerCardId;
			Result.MarkerResult =
				bHasSkillRuleSet
					? FMatchPlayCurrentAttackMarkerSelectionWriter::SelectWithSkillRules(
						BeforeState,
						AuthoritativeSkillRuleSet,
						DomainRequest)
					: FMatchPlayCurrentAttackMarkerSelectionWriter::Select(
						BeforeState,
						DomainRequest);

			FDomainExecution Execution;
			Execution.bSuccess = Result.MarkerResult.bSuccess;
			Execution.CandidateAfterState = Result.MarkerResult.AfterState;
			Execution.StateDisposition = Result.MarkerResult.bSuccess
				? EMatchPlayAuthoritativeStateDisposition::Adopt
				: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence = AttackSequence;
			return Execution;
		});
}

FMatchPlayAuthoritativeResolveNoLegalMarkerResult
FMatchPlayAuthoritativeSession::ResolveNoLegalMarker()
{
	const int64 AttackSequence = AuthoritativeState.bHasCurrentAttack
		? AuthoritativeState.CurrentAttack.AttackSequence
		: 0;
	return ExecuteSerialized<
		FMatchPlayAuthoritativeResolveNoLegalMarkerResult>(
		EMatchPlayAuthoritativeCommandKind::ResolveNoLegalMarker,
		true,
		AttackSequence,
		[AttackSequence](
			FMatchPlayAuthoritativeResolveNoLegalMarkerResult& Result,
			const FMatchPlayState& BeforeState)
		{
			FMatchPlayResolveNoLegalMarkerRequest DomainRequest;
			DomainRequest.AttackSequence = AttackSequence;
			Result.ResolutionResult =
				FMatchPlayResolveNoLegalMarker::Resolve(
					BeforeState,
					DomainRequest);

			FDomainExecution Execution;
			Execution.bSuccess = Result.ResolutionResult.bSuccess;
			Execution.CandidateAfterState =
				Result.ResolutionResult.AfterState;
			Execution.StateDisposition = Result.ResolutionResult.bSuccess
				? EMatchPlayAuthoritativeStateDisposition::Adopt
				: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence = AttackSequence;
			return Execution;
		});
}

FMatchPlayAuthoritativeDeclineMarkerResult
FMatchPlayAuthoritativeSession::DeclineMarker(
	const FMatchPlayAuthoritativeDeclineMarkerRequest& Request)
{
	const int64 AttackSequence = AuthoritativeState.bHasCurrentAttack
		? AuthoritativeState.CurrentAttack.AttackSequence
		: 0;
	return ExecuteSerialized<FMatchPlayAuthoritativeDeclineMarkerResult>(
		EMatchPlayAuthoritativeCommandKind::DeclineMarker,
		true,
		AttackSequence,
		[Request, AttackSequence](
			FMatchPlayAuthoritativeDeclineMarkerResult& Result,
			const FMatchPlayState& BeforeState)
		{
			FMatchPlayMarkerDeclineRequest DomainRequest;
			DomainRequest.AttackSequence = AttackSequence;
			DomainRequest.RequestingSide = Request.RequestingSide;
			Result.DeclineResult = FMatchPlayMarkerDecline::Decline(
				BeforeState,
				DomainRequest);

			FDomainExecution Execution;
			Execution.bSuccess = Result.DeclineResult.bSuccess;
			Execution.CandidateAfterState = Result.DeclineResult.AfterState;
			Execution.StateDisposition = Result.DeclineResult.bSuccess
				? EMatchPlayAuthoritativeStateDisposition::Adopt
				: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence = AttackSequence;
			return Execution;
		});
}

FMatchPlayAuthoritativeSubmitSkillResult
FMatchPlayAuthoritativeSession::SubmitSkill(
	const FSkillRuleSnapshotSet& SkillRuleSet,
	const FMatchPlayAuthoritativeSubmitSkillRequest& Request)
{
	const int64 AttackSequence = AuthoritativeState.bHasCurrentAttack
		? AuthoritativeState.CurrentAttack.AttackSequence
		: 0;
	return ExecuteSerialized<FMatchPlayAuthoritativeSubmitSkillResult>(
		EMatchPlayAuthoritativeCommandKind::SubmitSkill,
		true,
		AttackSequence,
		[&SkillRuleSet, Request, AttackSequence](
			FMatchPlayAuthoritativeSubmitSkillResult& Result,
			const FMatchPlayState& BeforeState)
		{
			FMatchPlayCurrentAttackSkillSelectionRequest DomainRequest;
			DomainRequest.AttackSequence = AttackSequence;
			DomainRequest.RequestingSide = Request.RequestingSide;
			DomainRequest.SkillId = Request.SkillId;
			Result.SkillResult =
				FMatchPlayCurrentAttackSkillSelectionWriter::Select(
					BeforeState,
					SkillRuleSet,
					DomainRequest);

			FDomainExecution Execution;
			Execution.bSuccess = Result.SkillResult.bSuccess;
			Execution.CandidateAfterState = Result.SkillResult.AfterState;
			Execution.StateDisposition = Result.SkillResult.bSuccess
				? EMatchPlayAuthoritativeStateDisposition::Adopt
				: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence = AttackSequence;
			return Execution;
		});
}

FMatchPlayAuthoritativeResolveNoLegalSkillResult
FMatchPlayAuthoritativeSession::ResolveNoLegalSkill(
	const FSkillRuleSnapshotSet& SkillRuleSet)
{
	const int64 AttackSequence = AuthoritativeState.bHasCurrentAttack
		? AuthoritativeState.CurrentAttack.AttackSequence
		: 0;
	return ExecuteSerialized<
		FMatchPlayAuthoritativeResolveNoLegalSkillResult>(
		EMatchPlayAuthoritativeCommandKind::ResolveNoLegalSkill,
		true,
		AttackSequence,
		[&SkillRuleSet, AttackSequence](
			FMatchPlayAuthoritativeResolveNoLegalSkillResult& Result,
			const FMatchPlayState& BeforeState)
		{
			FMatchPlayResolveNoLegalSkillRequest DomainRequest;
			DomainRequest.AttackSequence = AttackSequence;
			Result.ResolutionResult = FMatchPlayResolveNoLegalSkill::Resolve(
				BeforeState,
				SkillRuleSet,
				DomainRequest);

			FDomainExecution Execution;
			Execution.bSuccess = Result.ResolutionResult.bSuccess;
			Execution.CandidateAfterState =
				Result.ResolutionResult.AfterState;
			Execution.StateDisposition = Result.ResolutionResult.bSuccess
				? EMatchPlayAuthoritativeStateDisposition::Adopt
				: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence = AttackSequence;
			return Execution;
		});
}

FMatchPlayAuthoritativeDeclineSkillResult
FMatchPlayAuthoritativeSession::DeclineSkill(
	const FSkillRuleSnapshotSet& SkillRuleSet,
	const FMatchPlayAuthoritativeDeclineSkillRequest& Request)
{
	const int64 AttackSequence = AuthoritativeState.bHasCurrentAttack
		? AuthoritativeState.CurrentAttack.AttackSequence
		: 0;
	return ExecuteSerialized<FMatchPlayAuthoritativeDeclineSkillResult>(
		EMatchPlayAuthoritativeCommandKind::DeclineSkill,
		true,
		AttackSequence,
		[&SkillRuleSet, Request, AttackSequence](
			FMatchPlayAuthoritativeDeclineSkillResult& Result,
			const FMatchPlayState& BeforeState)
		{
			FMatchPlaySkillDeclineRequest DomainRequest;
			DomainRequest.AttackSequence = AttackSequence;
			DomainRequest.RequestingSide = Request.RequestingSide;
			Result.DeclineResult = FMatchPlaySkillDecline::Decline(
				BeforeState,
				SkillRuleSet,
				DomainRequest);

			FDomainExecution Execution;
			Execution.bSuccess = Result.DeclineResult.bSuccess;
			Execution.CandidateAfterState = Result.DeclineResult.AfterState;
			Execution.StateDisposition = Result.DeclineResult.bSuccess
				? EMatchPlayAuthoritativeStateDisposition::Adopt
				: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence = AttackSequence;
			return Execution;
		});
}

FMatchPlayAuthoritativeSubmitRunnerResult
FMatchPlayAuthoritativeSession::SubmitRunner(
	const FMatchPlayAuthoritativeSubmitRunnerRequest& Request)
{
	const int64 AttackSequence = AuthoritativeState.bHasCurrentAttack
		? AuthoritativeState.CurrentAttack.AttackSequence
		: 0;
	return ExecuteSerialized<FMatchPlayAuthoritativeSubmitRunnerResult>(
		EMatchPlayAuthoritativeCommandKind::SubmitRunner,
		true,
		AttackSequence,
		[Request, AttackSequence](
			FMatchPlayAuthoritativeSubmitRunnerResult& Result,
			const FMatchPlayState& BeforeState)
		{
			FMatchPlayCurrentAttackRunnerSelectionRequest DomainRequest;
			DomainRequest.AttackSequence = AttackSequence;
			DomainRequest.RequestingSide = Request.RequestingSide;
			DomainRequest.RunnerCardId = Request.RunnerCardId;
			Result.RunnerResult =
				FMatchPlayCurrentAttackRunnerSelectionWriter::Select(
					BeforeState,
					DomainRequest);

			FDomainExecution Execution;
			Execution.bSuccess = Result.RunnerResult.bSuccess;
			Execution.CandidateAfterState = Result.RunnerResult.AfterState;
			Execution.StateDisposition = Result.RunnerResult.bSuccess
				? EMatchPlayAuthoritativeStateDisposition::Adopt
				: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence = AttackSequence;
			return Execution;
		});
}

FMatchPlayAuthoritativeResolveNoLegalRunnerResult
FMatchPlayAuthoritativeSession::ResolveNoLegalRunner()
{
	const int64 AttackSequence = AuthoritativeState.bHasCurrentAttack
		? AuthoritativeState.CurrentAttack.AttackSequence
		: 0;
	return ExecuteSerialized<
		FMatchPlayAuthoritativeResolveNoLegalRunnerResult>(
		EMatchPlayAuthoritativeCommandKind::ResolveNoLegalRunner,
		true,
		AttackSequence,
		[AttackSequence](
			FMatchPlayAuthoritativeResolveNoLegalRunnerResult& Result,
			const FMatchPlayState& BeforeState)
		{
			FMatchPlayResolveNoLegalRunnerRequest DomainRequest;
			DomainRequest.AttackSequence = AttackSequence;
			Result.ResolutionResult =
				FMatchPlayResolveNoLegalRunner::Resolve(
					BeforeState,
					DomainRequest);

			FDomainExecution Execution;
			Execution.bSuccess = Result.ResolutionResult.bSuccess;
			Execution.CandidateAfterState =
				Result.ResolutionResult.AfterState;
			Execution.StateDisposition = Result.ResolutionResult.bSuccess
				? EMatchPlayAuthoritativeStateDisposition::Adopt
				: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence = AttackSequence;
			return Execution;
		});
}

FMatchPlayAuthoritativeDeclineRunnerResult
FMatchPlayAuthoritativeSession::DeclineRunner(
	const FMatchPlayAuthoritativeDeclineRunnerRequest& Request)
{
	const int64 AttackSequence = AuthoritativeState.bHasCurrentAttack
		? AuthoritativeState.CurrentAttack.AttackSequence
		: 0;
	return ExecuteSerialized<FMatchPlayAuthoritativeDeclineRunnerResult>(
		EMatchPlayAuthoritativeCommandKind::DeclineRunner,
		true,
		AttackSequence,
		[Request, AttackSequence](
			FMatchPlayAuthoritativeDeclineRunnerResult& Result,
			const FMatchPlayState& BeforeState)
		{
			FMatchPlayRunnerDeclineRequest DomainRequest;
			DomainRequest.AttackSequence = AttackSequence;
			DomainRequest.RequestingSide = Request.RequestingSide;
			Result.DeclineResult = FMatchPlayRunnerDecline::Decline(
				BeforeState,
				DomainRequest);

			FDomainExecution Execution;
			Execution.bSuccess = Result.DeclineResult.bSuccess;
			Execution.CandidateAfterState = Result.DeclineResult.AfterState;
			Execution.StateDisposition = Result.DeclineResult.bSuccess
				? EMatchPlayAuthoritativeStateDisposition::Adopt
				: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence = AttackSequence;
			return Execution;
		});
}

FMatchPlayAuthoritativeSubmitHelperResult
FMatchPlayAuthoritativeSession::SubmitHelper(
	const FMatchPlayAuthoritativeSubmitHelperRequest& Request)
{
	const int64 AttackSequence = AuthoritativeState.bHasCurrentAttack
		? AuthoritativeState.CurrentAttack.AttackSequence
		: 0;
	return ExecuteSerialized<FMatchPlayAuthoritativeSubmitHelperResult>(
		EMatchPlayAuthoritativeCommandKind::SubmitHelper,
		true,
		AttackSequence,
		[Request, AttackSequence](
			FMatchPlayAuthoritativeSubmitHelperResult& Result,
			const FMatchPlayState& BeforeState)
		{
			FMatchPlayCurrentAttackHelperSelectionRequest DomainRequest;
			DomainRequest.AttackSequence = AttackSequence;
			DomainRequest.RequestingSide = Request.RequestingSide;
			DomainRequest.HelperCardId = Request.HelperCardId;
			Result.HelperResult =
				FMatchPlayCurrentAttackHelperSelectionWriter::Select(
					BeforeState,
					DomainRequest);

			FDomainExecution Execution;
			Execution.bSuccess = Result.HelperResult.bSuccess;
			Execution.CandidateAfterState = Result.HelperResult.AfterState;
			Execution.StateDisposition = Result.HelperResult.bSuccess
				? EMatchPlayAuthoritativeStateDisposition::Adopt
				: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence = AttackSequence;
			return Execution;
		});
}

FMatchPlayAuthoritativeResolveNoLegalHelperResult
FMatchPlayAuthoritativeSession::ResolveNoLegalHelper()
{
	const int64 AttackSequence = AuthoritativeState.bHasCurrentAttack
		? AuthoritativeState.CurrentAttack.AttackSequence
		: 0;
	return ExecuteSerialized<
		FMatchPlayAuthoritativeResolveNoLegalHelperResult>(
		EMatchPlayAuthoritativeCommandKind::ResolveNoLegalHelper,
		true,
		AttackSequence,
		[AttackSequence](
			FMatchPlayAuthoritativeResolveNoLegalHelperResult& Result,
			const FMatchPlayState& BeforeState)
		{
			FMatchPlayResolveNoLegalHelperRequest DomainRequest;
			DomainRequest.AttackSequence = AttackSequence;
			Result.ResolutionResult =
				FMatchPlayResolveNoLegalHelper::Resolve(
					BeforeState,
					DomainRequest);

			FDomainExecution Execution;
			Execution.bSuccess = Result.ResolutionResult.bSuccess;
			Execution.CandidateAfterState =
				Result.ResolutionResult.AfterState;
			Execution.StateDisposition = Result.ResolutionResult.bSuccess
				? EMatchPlayAuthoritativeStateDisposition::Adopt
				: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence = AttackSequence;
			return Execution;
		});
}

FMatchPlayAuthoritativeDeclineHelperResult
FMatchPlayAuthoritativeSession::DeclineHelper(
	const FMatchPlayAuthoritativeDeclineHelperRequest& Request)
{
	const int64 AttackSequence = AuthoritativeState.bHasCurrentAttack
		? AuthoritativeState.CurrentAttack.AttackSequence
		: 0;
	return ExecuteSerialized<FMatchPlayAuthoritativeDeclineHelperResult>(
		EMatchPlayAuthoritativeCommandKind::DeclineHelper,
		true,
		AttackSequence,
		[Request, AttackSequence](
			FMatchPlayAuthoritativeDeclineHelperResult& Result,
			const FMatchPlayState& BeforeState)
		{
			FMatchPlayHelperDeclineRequest DomainRequest;
			DomainRequest.AttackSequence = AttackSequence;
			DomainRequest.RequestingSide = Request.RequestingSide;
			Result.DeclineResult = FMatchPlayHelperDecline::Decline(
				BeforeState,
				DomainRequest);

			FDomainExecution Execution;
			Execution.bSuccess = Result.DeclineResult.bSuccess;
			Execution.CandidateAfterState = Result.DeclineResult.AfterState;
			Execution.StateDisposition = Result.DeclineResult.bSuccess
				? EMatchPlayAuthoritativeStateDisposition::Adopt
				: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence = AttackSequence;
			return Execution;
		});
}

FMatchPlayAuthoritativeBeginResolutionSessionResult
FMatchPlayAuthoritativeSession::BeginResolutionSession()
{
	const int64 AttackSequence = AuthoritativeState.bHasCurrentAttack
		? AuthoritativeState.CurrentAttack.AttackSequence
		: 0;
	return ExecuteSerialized<
		FMatchPlayAuthoritativeBeginResolutionSessionResult>(
		EMatchPlayAuthoritativeCommandKind::BeginResolutionSession,
		true,
		AttackSequence,
		[AttackSequence](
			FMatchPlayAuthoritativeBeginResolutionSessionResult& Result,
			const FMatchPlayState& BeforeState)
		{
			FMatchPlayCurrentAttackBeginResolutionSessionRequest
				DomainRequest;
			DomainRequest.AttackSequence = AttackSequence;
			Result.BeginResult =
				FMatchPlayCurrentAttackBeginResolutionSessionWriter::Begin(
					BeforeState,
					DomainRequest);

			FDomainExecution Execution;
			Execution.bSuccess = Result.BeginResult.bSuccess;
			Execution.CandidateAfterState = Result.BeginResult.AfterState;
			Execution.StateDisposition = Result.BeginResult.bSuccess
				? EMatchPlayAuthoritativeStateDisposition::Adopt
				: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence = AttackSequence;
			return Execution;
		});
}

FMatchPlayAuthoritativeSubmitBranchIntentResult
FMatchPlayAuthoritativeSession::SubmitBranchIntent(
	const FMatchPlayAuthoritativeSubmitBranchIntentRequest& Request)
{
	return ExecuteSerialized<
		FMatchPlayAuthoritativeSubmitBranchIntentResult>(
		EMatchPlayAuthoritativeCommandKind::SubmitBranchIntent,
		true,
		Request.AttackSequence,
		[Request](
			FMatchPlayAuthoritativeSubmitBranchIntentResult& Result,
			const FMatchPlayState& BeforeState)
		{
			FMatchPlayCurrentAttackBranchIntentSelectionRequest
				DomainRequest;
			DomainRequest.AttackSequence = Request.AttackSequence;
			DomainRequest.RequestingSide = Request.RequestingSide;
			DomainRequest.Intent = Request.Intent;
			Result.IntentResult =
				FMatchPlayCurrentAttackBranchIntentSelectionWriter::Select(
					BeforeState,
					DomainRequest);

			FDomainExecution Execution;
			Execution.bSuccess = Result.IntentResult.bSuccess;
			Execution.CandidateAfterState = Result.IntentResult.AfterState;
			Execution.StateDisposition = Result.IntentResult.bSuccess
				? EMatchPlayAuthoritativeStateDisposition::Adopt
				: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence = Request.AttackSequence;
			return Execution;
		});
}

FMatchPlayAuthoritativeResolveIntentDeterminedRouteResult
FMatchPlayAuthoritativeSession::ResolveIntentDeterminedRoute()
{
	const int64 AttackSequence = AuthoritativeState.bHasCurrentAttack
		? AuthoritativeState.CurrentAttack.AttackSequence
		: 0;
	return ExecuteSerialized<
		FMatchPlayAuthoritativeResolveIntentDeterminedRouteResult>(
		EMatchPlayAuthoritativeCommandKind::ResolveIntentDeterminedRoute,
		true,
		AttackSequence,
		[AttackSequence](
			FMatchPlayAuthoritativeResolveIntentDeterminedRouteResult& Result,
			const FMatchPlayState& BeforeState)
		{
			FMatchPlayCurrentAttackResolveInitialRouteRequest DomainRequest;
			DomainRequest.AttackSequence = AttackSequence;
			Result.RouteResult =
				FMatchPlayCurrentAttackResolveInitialRouteWriter::Resolve(
					BeforeState,
					DomainRequest,
					nullptr);

			FDomainExecution Execution;
			Execution.bSuccess = Result.RouteResult.bSuccess;
			Execution.CandidateAfterState = Result.RouteResult.AfterState;
			Execution.StateDisposition = Result.RouteResult.bSuccess
				? EMatchPlayAuthoritativeStateDisposition::Adopt
				: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence = AttackSequence;
			return Execution;
		});
}

FMatchPlayAuthoritativeResolveInitialRouteResult
FMatchPlayAuthoritativeSession::ResolveInitialRoute()
{
	const int64 AttackSequence = AuthoritativeState.bHasCurrentAttack
		? AuthoritativeState.CurrentAttack.AttackSequence
		: 0;
	return ExecuteSerialized<FMatchPlayAuthoritativeResolveInitialRouteResult>(
		EMatchPlayAuthoritativeCommandKind::ResolveInitialRoute,
		true,
		AttackSequence,
		[this, AttackSequence](
			FMatchPlayAuthoritativeResolveInitialRouteResult& Result,
			const FMatchPlayState& BeforeState)
		{
			FMatchPlayCurrentAttackResolveInitialRouteOrchestrationRequest
				DomainRequest;
			DomainRequest.AttackSequence = AttackSequence;
			Result.OrchestrationResult =
				FMatchPlayCurrentAttackResolveInitialRouteOrchestrator::Resolve(
					BeforeState,
					DomainRequest,
					InitialRouteRollProvider);

			FDomainExecution Execution;
			Execution.bSuccess = Result.OrchestrationResult.bSuccess;
			Execution.CandidateAfterState =
				Result.OrchestrationResult.AfterState;
			Execution.StateDisposition = Result.OrchestrationResult.bSuccess
				? EMatchPlayAuthoritativeStateDisposition::Adopt
				: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence = AttackSequence;
			return Execution;
		});
}

FMatchPlayAuthoritativeResolveThroughBallInitialRouteRollResult
FMatchPlayAuthoritativeSession::ResolveThroughBallInitialRouteRoll(
	const FMatchPlayAuthoritativeResolveThroughBallInitialRouteRollRequest&
		Request)
{
	return ExecuteSerialized<
		FMatchPlayAuthoritativeResolveThroughBallInitialRouteRollResult>(
		EMatchPlayAuthoritativeCommandKind::ResolveThroughBallInitialRouteRoll,
		true,
		Request.AttackSequence,
		[this, Request](
			FMatchPlayAuthoritativeResolveThroughBallInitialRouteRollResult& Result,
			const FMatchPlayState& BeforeState)
		{
			FDomainExecution Execution;
			Execution.CandidateAfterState = BeforeState;
			Execution.StateDisposition =
				EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence = Request.AttackSequence;

			using ERouteError =
				EMatchPlayAuthoritativeThroughBallInitialRouteRollErrorCode;
			auto Reject = [&Result, &Execution](
				const ERouteError ErrorCode,
				const TCHAR* ErrorMessage)
			{
				Result.ErrorCode = ErrorCode;
				Result.ErrorMessage = ErrorMessage;
				Result.OrchestrationResult.ErrorMessage = ErrorMessage;
				return Execution;
			};

			if (!BeforeState.bHasCurrentAttack)
			{
				return Reject(
					ERouteError::NoCurrentAttack,
					TEXT("ThroughBall Initial Route requires a current attack."));
			}
			if (Request.AttackSequence <= 0)
			{
				return Reject(
					ERouteError::InvalidAttackSequence,
					TEXT("ThroughBall Initial Route requires a positive AttackSequence."));
			}
			if (Request.AttackSequence
				!= BeforeState.CurrentAttack.AttackSequence)
			{
				return Reject(
					ERouteError::AttackSequenceMismatch,
					TEXT("ThroughBall Initial Route request is stale."));
			}
			if (Request.RequestingSide == EInitialTurnOrderPlayer::None)
			{
				return Reject(
					ERouteError::InvalidRequestingSide,
					TEXT("ThroughBall Initial Route requires a requesting side."));
			}
			if (Request.RequestingSide
				!= BeforeState.RuntimeState.CurrentAttackingPlayer)
			{
				return Reject(
					ERouteError::RequestingSideNotCurrentAttacker,
					TEXT("Only the current attacker may roll the ThroughBall Initial Route."));
			}

			const FMatchPlayCurrentAttackState& Attack =
				BeforeState.CurrentAttack;
			const ESkillRuleType ActionType = Attack.bHasResolutionSession
				? Attack.ResolutionSession.Bundle.Binding.ActionType
				: Attack.bHasSelectedAction
					? Attack.SelectedAction.ActionType
					: Attack.ActionPreparation.ActionType;
			if (ActionType != ESkillRuleType::ThroughBall)
			{
				return Reject(
					ERouteError::WrongResolutionFamily,
					TEXT("Typed ThroughBall Initial Route requires a ThroughBall attack."));
			}
			const bool bReadyWithoutSession = !Attack.bHasResolutionSession
				&& Attack.SelectionStage
					== EMatchPlayCurrentAttackSelectionStage::ReadyForResolution;
			const bool bAwaitingUnresolvedRoute = Attack.bHasResolutionSession
				&& Attack.ResolutionSession.Stage
					== EMatchPlayCurrentAttackResolutionStage::AwaitingRoute
				&& !Attack.ResolutionSession.bHasActualBranch
				&& Attack.ResolutionSession.InitialRouteRollRecords.IsEmpty();
			if (!bReadyWithoutSession && !bAwaitingUnresolvedRoute)
			{
				return Reject(
					ERouteError::RouteRollNotPending,
					TEXT("ThroughBall Initial Route roll is not pending."));
			}

			FMatchPlayCurrentAttackResolveInitialRouteOrchestrationRequest
				DomainRequest;
			DomainRequest.AttackSequence = Request.AttackSequence;
			Result.OrchestrationResult =
				FMatchPlayCurrentAttackResolveInitialRouteOrchestrator::Resolve(
					BeforeState,
					DomainRequest,
					InitialRouteRollProvider);

			Execution.bSuccess = Result.OrchestrationResult.bSuccess;
			Execution.CandidateAfterState =
				Result.OrchestrationResult.AfterState;
			Execution.StateDisposition = Result.OrchestrationResult.bSuccess
				? EMatchPlayAuthoritativeStateDisposition::Adopt
				: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			return Execution;
		});
}

FMatchPlayAuthoritativeResolvePassControlInitialRouteRollResult
FMatchPlayAuthoritativeSession::ResolvePassControlInitialRouteRoll(
	const FMatchPlayAuthoritativeResolvePassControlInitialRouteRollRequest&
		Request)
{
	return ExecuteSerialized<
		FMatchPlayAuthoritativeResolvePassControlInitialRouteRollResult>(
		EMatchPlayAuthoritativeCommandKind::ResolvePassControlInitialRouteRoll,
		true,
		Request.AttackSequence,
		[this, Request](
			FMatchPlayAuthoritativeResolvePassControlInitialRouteRollResult& Result,
			const FMatchPlayState& BeforeState)
		{
			FDomainExecution Execution;
			Execution.CandidateAfterState = BeforeState;
			Execution.StateDisposition =
				EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence = Request.AttackSequence;

			using ERouteError =
				EMatchPlayAuthoritativePassControlInitialRouteRollErrorCode;
			auto Reject = [&Result, &Execution](
				const ERouteError ErrorCode,
				const TCHAR* ErrorMessage)
			{
				Result.ErrorCode = ErrorCode;
				Result.ErrorMessage = ErrorMessage;
				Result.OrchestrationResult.ErrorMessage = ErrorMessage;
				return Execution;
			};

			if (!BeforeState.bHasCurrentAttack)
			{
				return Reject(ERouteError::NoCurrentAttack,
					TEXT("PassControl Initial Route requires a current attack."));
			}
			if (Request.AttackSequence <= 0)
			{
				return Reject(ERouteError::InvalidAttackSequence,
					TEXT("PassControl Initial Route requires a positive AttackSequence."));
			}
			if (Request.AttackSequence
				!= BeforeState.CurrentAttack.AttackSequence)
			{
				return Reject(ERouteError::AttackSequenceMismatch,
					TEXT("PassControl Initial Route request is stale."));
			}
			if (Request.RequestingSide != EInitialTurnOrderPlayer::PlayerA
				&& Request.RequestingSide != EInitialTurnOrderPlayer::PlayerB)
			{
				return Reject(ERouteError::InvalidRequestingSide,
					TEXT("PassControl Initial Route requires PlayerA or PlayerB as RequestingSide."));
			}
			if (Request.RequestingSide
				!= BeforeState.RuntimeState.CurrentAttackingPlayer)
			{
				return Reject(ERouteError::RequestingSideNotCurrentAttacker,
					TEXT("Only the current attacker may roll the PassControl Initial Route."));
			}

			const FMatchPlayCurrentAttackState& Attack =
				BeforeState.CurrentAttack;
			const ESkillRuleType ActionType = Attack.bHasResolutionSession
				? Attack.ResolutionSession.Bundle.Binding.ActionType
				: Attack.bHasSelectedAction
					? Attack.SelectedAction.ActionType
					: Attack.ActionPreparation.ActionType;
			if (ActionType != ESkillRuleType::PassControl)
			{
				return Reject(ERouteError::WrongResolutionFamily,
					TEXT("Typed PassControl Initial Route requires a PassControl attack."));
			}
			const bool bReadyWithoutSession = !Attack.bHasResolutionSession
				&& Attack.SelectionStage
					== EMatchPlayCurrentAttackSelectionStage::ReadyForResolution;
			const bool bAwaitingUnresolvedRoute = Attack.bHasResolutionSession
				&& Attack.ResolutionSession.Stage
					== EMatchPlayCurrentAttackResolutionStage::AwaitingRoute
				&& !Attack.ResolutionSession.bHasActualBranch
				&& Attack.ResolutionSession.InitialRouteRollRecords.IsEmpty();
			if (!bReadyWithoutSession && !bAwaitingUnresolvedRoute)
			{
				return Reject(ERouteError::RouteRollNotPending,
					TEXT("PassControl Initial Route roll is not pending."));
			}

			FMatchPlayCurrentAttackResolveInitialRouteOrchestrationRequest
				DomainRequest;
			DomainRequest.AttackSequence = Request.AttackSequence;
			Result.OrchestrationResult =
				FMatchPlayCurrentAttackResolveInitialRouteOrchestrator::Resolve(
					BeforeState,
					DomainRequest,
					InitialRouteRollProvider);

			Execution.bSuccess = Result.OrchestrationResult.bSuccess;
			Execution.CandidateAfterState =
				Result.OrchestrationResult.AfterState;
			Execution.StateDisposition = Result.OrchestrationResult.bSuccess
				? EMatchPlayAuthoritativeStateDisposition::Adopt
				: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			return Execution;
		});
}

FMatchPlayAuthoritativeResolveCrossInitialRouteRollResult
FMatchPlayAuthoritativeSession::ResolveCrossInitialRouteRoll(
	const FMatchPlayAuthoritativeResolveCrossInitialRouteRollRequest& Request)
{
	return ExecuteSerialized<
		FMatchPlayAuthoritativeResolveCrossInitialRouteRollResult>(
		EMatchPlayAuthoritativeCommandKind::ResolveCrossInitialRouteRoll,
		true,
		Request.AttackSequence,
		[this, Request](
			FMatchPlayAuthoritativeResolveCrossInitialRouteRollResult& Result,
			const FMatchPlayState& BeforeState)
		{
			FDomainExecution Execution;
			Execution.CandidateAfterState = BeforeState;
			Execution.StateDisposition =
				EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence = Request.AttackSequence;

			using ERouteError =
				EMatchPlayAuthoritativeCrossInitialRouteRollErrorCode;
			auto Reject = [&Result, &Execution](
				const ERouteError ErrorCode,
				const TCHAR* ErrorMessage)
			{
				Result.ErrorCode = ErrorCode;
				Result.ErrorMessage = ErrorMessage;
				Result.OrchestrationResult.ErrorMessage = ErrorMessage;
				return Execution;
			};

			if (!BeforeState.bHasCurrentAttack)
			{
				return Reject(ERouteError::NoCurrentAttack,
					TEXT("Cross Initial Route requires a current attack."));
			}
			if (Request.AttackSequence <= 0)
			{
				return Reject(ERouteError::InvalidAttackSequence,
					TEXT("Cross Initial Route requires a positive AttackSequence."));
			}
			if (Request.AttackSequence
				!= BeforeState.CurrentAttack.AttackSequence)
			{
				return Reject(ERouteError::AttackSequenceMismatch,
					TEXT("Cross Initial Route request is stale."));
			}
			if (Request.RequestingSide != EInitialTurnOrderPlayer::PlayerA
				&& Request.RequestingSide != EInitialTurnOrderPlayer::PlayerB)
			{
				return Reject(ERouteError::InvalidRequestingSide,
					TEXT("Cross Initial Route requires PlayerA or PlayerB as RequestingSide."));
			}
			if (Request.RequestingSide
				!= BeforeState.RuntimeState.CurrentAttackingPlayer)
			{
				return Reject(ERouteError::RequestingSideNotCurrentAttacker,
					TEXT("Only the current attacker may roll the Cross Initial Route."));
			}

			const FMatchPlayCurrentAttackState& Attack =
				BeforeState.CurrentAttack;
			const ESkillRuleType ActionType = Attack.bHasResolutionSession
				? Attack.ResolutionSession.Bundle.Binding.ActionType
				: Attack.bHasSelectedAction
					? Attack.SelectedAction.ActionType
					: Attack.ActionPreparation.ActionType;
			if (ActionType != ESkillRuleType::Cross)
			{
				return Reject(ERouteError::WrongResolutionFamily,
					TEXT("Typed Cross Initial Route requires a Cross attack."));
			}
			const bool bReadyWithoutSession = !Attack.bHasResolutionSession
				&& Attack.SelectionStage
					== EMatchPlayCurrentAttackSelectionStage::ReadyForResolution;
			const bool bAwaitingUnresolvedRoute = Attack.bHasResolutionSession
				&& Attack.ResolutionSession.Stage
					== EMatchPlayCurrentAttackResolutionStage::AwaitingRoute
				&& !Attack.ResolutionSession.bHasActualBranch
				&& Attack.ResolutionSession.InitialRouteRollRecords.IsEmpty();
			if (!bReadyWithoutSession && !bAwaitingUnresolvedRoute)
			{
				return Reject(ERouteError::RouteRollNotPending,
					TEXT("Cross Initial Route roll is not pending."));
			}

			FMatchPlayCurrentAttackResolveInitialRouteOrchestrationRequest
				DomainRequest;
			DomainRequest.AttackSequence = Request.AttackSequence;
			Result.OrchestrationResult =
				FMatchPlayCurrentAttackResolveInitialRouteOrchestrator::Resolve(
					BeforeState,
					DomainRequest,
					InitialRouteRollProvider);

			Execution.bSuccess = Result.OrchestrationResult.bSuccess;
			Execution.CandidateAfterState =
				Result.OrchestrationResult.AfterState;
			Execution.StateDisposition = Result.OrchestrationResult.bSuccess
				? EMatchPlayAuthoritativeStateDisposition::Adopt
				: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			return Execution;
		});
}

FMatchPlayAuthoritativeResolveCrossPostRoutePlanResult
FMatchPlayAuthoritativeSession::ResolveCrossPostRoutePlan()
{
	const int64 AttackSequence = AuthoritativeState.bHasCurrentAttack
		? AuthoritativeState.CurrentAttack.AttackSequence
		: 0;
	return ExecuteSerialized<
		FMatchPlayAuthoritativeResolveCrossPostRoutePlanResult>(
		EMatchPlayAuthoritativeCommandKind::ResolveCrossPostRoutePlan,
		true,
		AttackSequence,
		[this, AttackSequence](
			FMatchPlayAuthoritativeResolveCrossPostRoutePlanResult& Result,
			const FMatchPlayState& BeforeState)
		{
			FMatchPlayCurrentAttackResolveCrossPostRoutePlanRequest
				DomainRequest;
			DomainRequest.AttackSequence = AttackSequence;
			Result.OrchestrationResult =
				FMatchPlayCurrentAttackResolveCrossPostRoutePlanOrchestrator
					::Resolve(
						BeforeState,
						DomainRequest,
						bHasSkillRuleSet
							? &AuthoritativeSkillRuleSet
							: nullptr,
						PostRouteRollProvider);

			FDomainExecution Execution;
			Execution.bSuccess = Result.OrchestrationResult.bSuccess;
			Execution.CandidateAfterState =
				Result.OrchestrationResult.AfterState;
			Execution.StateDisposition =
				Result.OrchestrationResult.bSuccess
					? EMatchPlayAuthoritativeStateDisposition::Adopt
					: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence = AttackSequence;
			return Execution;
		});
}

FMatchPlayAuthoritativeResolveCrossHighAttackRollResult
FMatchPlayAuthoritativeSession::ResolveCrossHighAttackRoll(
	const FMatchPlayAuthoritativeResolveCrossHighAttackRollRequest& Request)
{
	return ExecuteSerialized<
		FMatchPlayAuthoritativeResolveCrossHighAttackRollResult>(
		EMatchPlayAuthoritativeCommandKind::ResolveCrossHighAttackRoll,
		true,
		Request.AttackSequence,
		[this, Request](
			FMatchPlayAuthoritativeResolveCrossHighAttackRollResult& Result,
			const FMatchPlayState& BeforeState)
		{
			FMatchPlayCurrentAttackResolveCrossPostRoutePlanRequest
				DomainRequest;
			DomainRequest.AttackSequence = Request.AttackSequence;
			DomainRequest.Mode = FMatchPlayCurrentAttackResolveCrossPostRoutePlanRequest
				::EMode::ResolveCrossHighAttackRoll;
			DomainRequest.RequestingSide = Request.RequestingSide;
			Result.OrchestrationResult =
				FMatchPlayCurrentAttackResolveCrossPostRoutePlanOrchestrator
					::Resolve(
						BeforeState,
						DomainRequest,
						bHasSkillRuleSet
							? &AuthoritativeSkillRuleSet
							: nullptr,
						PostRouteRollProvider);

			FDomainExecution Execution;
			Execution.bSuccess = Result.OrchestrationResult.bSuccess;
			Execution.CandidateAfterState =
				Result.OrchestrationResult.AfterState;
			Execution.StateDisposition = Result.OrchestrationResult.bSuccess
				? EMatchPlayAuthoritativeStateDisposition::Adopt
				: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence = Request.AttackSequence;
			return Execution;
		});
}

FMatchPlayAuthoritativeResolveCrossHighDefenseRollResult
FMatchPlayAuthoritativeSession::ResolveCrossHighDefenseRoll(
	const FMatchPlayAuthoritativeResolveCrossHighDefenseRollRequest& Request)
{
	return ExecuteSerialized<
		FMatchPlayAuthoritativeResolveCrossHighDefenseRollResult>(
		EMatchPlayAuthoritativeCommandKind::ResolveCrossHighDefenseRoll,
		true,
		Request.AttackSequence,
		[this, Request](
			FMatchPlayAuthoritativeResolveCrossHighDefenseRollResult& Result,
			const FMatchPlayState& BeforeState)
		{
			FMatchPlayCurrentAttackResolveCrossPostRoutePlanRequest
				DomainRequest;
			DomainRequest.AttackSequence = Request.AttackSequence;
			DomainRequest.Mode = FMatchPlayCurrentAttackResolveCrossPostRoutePlanRequest
				::EMode::ResolveCrossHighDefenseRoll;
			DomainRequest.RequestingSide = Request.RequestingSide;
			Result.OrchestrationResult =
				FMatchPlayCurrentAttackResolveCrossPostRoutePlanOrchestrator
					::Resolve(
						BeforeState,
						DomainRequest,
						bHasSkillRuleSet
							? &AuthoritativeSkillRuleSet
							: nullptr,
						PostRouteRollProvider);

			FDomainExecution Execution;
			Execution.bSuccess = Result.OrchestrationResult.bSuccess;
			Execution.CandidateAfterState =
				Result.OrchestrationResult.AfterState;
			Execution.StateDisposition = Result.OrchestrationResult.bSuccess
				? EMatchPlayAuthoritativeStateDisposition::Adopt
				: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence = Request.AttackSequence;
			return Execution;
		});
}

FMatchPlayAuthoritativeResolveCrossLowAttackRollResult
FMatchPlayAuthoritativeSession::ResolveCrossLowAttackRoll(
	const FMatchPlayAuthoritativeResolveCrossLowAttackRollRequest& Request)
{
	return ExecuteSerialized<
		FMatchPlayAuthoritativeResolveCrossLowAttackRollResult>(
		EMatchPlayAuthoritativeCommandKind::ResolveCrossLowAttackRoll,
		true,
		Request.AttackSequence,
		[this, Request](
			FMatchPlayAuthoritativeResolveCrossLowAttackRollResult& Result,
			const FMatchPlayState& BeforeState)
		{
			FMatchPlayCurrentAttackResolveCrossPostRoutePlanRequest
				DomainRequest;
			DomainRequest.AttackSequence = Request.AttackSequence;
			DomainRequest.Mode = FMatchPlayCurrentAttackResolveCrossPostRoutePlanRequest
				::EMode::ResolveCrossLowAttackRoll;
			DomainRequest.RequestingSide = Request.RequestingSide;
			Result.OrchestrationResult =
				FMatchPlayCurrentAttackResolveCrossPostRoutePlanOrchestrator
					::Resolve(
						BeforeState,
						DomainRequest,
						bHasSkillRuleSet
							? &AuthoritativeSkillRuleSet
							: nullptr,
						PostRouteRollProvider);
			FDomainExecution Execution;
			Execution.bSuccess = Result.OrchestrationResult.bSuccess;
			Execution.CandidateAfterState =
				Result.OrchestrationResult.AfterState;
			Execution.StateDisposition = Result.OrchestrationResult.bSuccess
				? EMatchPlayAuthoritativeStateDisposition::Adopt
				: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence = Request.AttackSequence;
			return Execution;
		});
}

FMatchPlayAuthoritativeResolveCrossLowDefenseRollResult
FMatchPlayAuthoritativeSession::ResolveCrossLowDefenseRoll(
	const FMatchPlayAuthoritativeResolveCrossLowDefenseRollRequest& Request)
{
	return ExecuteSerialized<
		FMatchPlayAuthoritativeResolveCrossLowDefenseRollResult>(
		EMatchPlayAuthoritativeCommandKind::ResolveCrossLowDefenseRoll,
		true,
		Request.AttackSequence,
		[this, Request](
			FMatchPlayAuthoritativeResolveCrossLowDefenseRollResult& Result,
			const FMatchPlayState& BeforeState)
		{
			FMatchPlayCurrentAttackResolveCrossPostRoutePlanRequest
				DomainRequest;
			DomainRequest.AttackSequence = Request.AttackSequence;
			DomainRequest.Mode = FMatchPlayCurrentAttackResolveCrossPostRoutePlanRequest
				::EMode::ResolveCrossLowDefenseRoll;
			DomainRequest.RequestingSide = Request.RequestingSide;
			Result.OrchestrationResult =
				FMatchPlayCurrentAttackResolveCrossPostRoutePlanOrchestrator
					::Resolve(
						BeforeState,
						DomainRequest,
						bHasSkillRuleSet
							? &AuthoritativeSkillRuleSet
							: nullptr,
						PostRouteRollProvider);
			FDomainExecution Execution;
			Execution.bSuccess = Result.OrchestrationResult.bSuccess;
			Execution.CandidateAfterState =
				Result.OrchestrationResult.AfterState;
			Execution.StateDisposition = Result.OrchestrationResult.bSuccess
				? EMatchPlayAuthoritativeStateDisposition::Adopt
				: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence = Request.AttackSequence;
			return Execution;
		});
}

FMatchPlayAuthoritativeResolveThroughBallFeetPostRoutePlanResult
FMatchPlayAuthoritativeSession::ResolveThroughBallFeetPostRoutePlan()
{
	const int64 AttackSequence = AuthoritativeState.bHasCurrentAttack
		? AuthoritativeState.CurrentAttack.AttackSequence
		: 0;
	return ExecuteSerialized<
		FMatchPlayAuthoritativeResolveThroughBallFeetPostRoutePlanResult>(
		EMatchPlayAuthoritativeCommandKind
			::ResolveThroughBallFeetPostRoutePlan,
		true,
		AttackSequence,
		[this, AttackSequence](
			FMatchPlayAuthoritativeResolveThroughBallFeetPostRoutePlanResult&
				Result,
			const FMatchPlayState& BeforeState)
		{
			FMatchPlayCurrentAttackResolveThroughBallFeetPostRoutePlanRequest
				DomainRequest;
			DomainRequest.AttackSequence = AttackSequence;
			Result.OrchestrationResult =
				FMatchPlayCurrentAttackResolveThroughBallFeetPostRoutePlanOrchestrator
					::Resolve(
						BeforeState,
						DomainRequest,
						bHasSkillRuleSet
							? &AuthoritativeSkillRuleSet
							: nullptr,
						PostRouteRollProvider);

			FDomainExecution Execution;
			Execution.bSuccess = Result.OrchestrationResult.bSuccess;
			Execution.CandidateAfterState =
				Result.OrchestrationResult.AfterState;
			Execution.StateDisposition =
				Result.OrchestrationResult.bSuccess
					? EMatchPlayAuthoritativeStateDisposition::Adopt
					: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence = AttackSequence;
			return Execution;
		});
}

FMatchPlayAuthoritativeResolveThroughBallFeetAttackRollResult
FMatchPlayAuthoritativeSession::ResolveThroughBallFeetAttackRoll(
	const FMatchPlayAuthoritativeResolveThroughBallFeetAttackRollRequest& Request)
{
	return ExecuteSerialized<
		FMatchPlayAuthoritativeResolveThroughBallFeetAttackRollResult>(
		EMatchPlayAuthoritativeCommandKind::ResolveThroughBallFeetAttackRoll,
		true,
		Request.AttackSequence,
		[this, Request](
			FMatchPlayAuthoritativeResolveThroughBallFeetAttackRollResult& Result,
			const FMatchPlayState& BeforeState)
		{
			FMatchPlayCurrentAttackResolveThroughBallFeetPostRoutePlanRequest
				DomainRequest;
			DomainRequest.AttackSequence = Request.AttackSequence;
			DomainRequest.Mode =
				FMatchPlayCurrentAttackResolveThroughBallFeetPostRoutePlanRequest
					::EMode::ResolveAttackRoll;
			DomainRequest.RequestingSide = Request.RequestingSide;
			Result.OrchestrationResult =
				FMatchPlayCurrentAttackResolveThroughBallFeetPostRoutePlanOrchestrator
					::Resolve(
						BeforeState,
						DomainRequest,
						bHasSkillRuleSet
							? &AuthoritativeSkillRuleSet
							: nullptr,
						PostRouteRollProvider);

			FDomainExecution Execution;
			Execution.bSuccess = Result.OrchestrationResult.bSuccess;
			Execution.CandidateAfterState =
				Result.OrchestrationResult.AfterState;
			Execution.StateDisposition = Result.OrchestrationResult.bSuccess
				? EMatchPlayAuthoritativeStateDisposition::Adopt
				: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence = Request.AttackSequence;
			return Execution;
		});
}

FMatchPlayAuthoritativeResolveThroughBallFeetDefenseRollResult
FMatchPlayAuthoritativeSession::ResolveThroughBallFeetDefenseRoll(
	const FMatchPlayAuthoritativeResolveThroughBallFeetDefenseRollRequest& Request)
{
	return ExecuteSerialized<
		FMatchPlayAuthoritativeResolveThroughBallFeetDefenseRollResult>(
		EMatchPlayAuthoritativeCommandKind::ResolveThroughBallFeetDefenseRoll,
		true,
		Request.AttackSequence,
		[this, Request](
			FMatchPlayAuthoritativeResolveThroughBallFeetDefenseRollResult& Result,
			const FMatchPlayState& BeforeState)
		{
			FMatchPlayCurrentAttackResolveThroughBallFeetPostRoutePlanRequest
				DomainRequest;
			DomainRequest.AttackSequence = Request.AttackSequence;
			DomainRequest.Mode =
				FMatchPlayCurrentAttackResolveThroughBallFeetPostRoutePlanRequest
					::EMode::ResolveDefenseRoll;
			DomainRequest.RequestingSide = Request.RequestingSide;
			Result.OrchestrationResult =
				FMatchPlayCurrentAttackResolveThroughBallFeetPostRoutePlanOrchestrator
					::Resolve(
						BeforeState,
						DomainRequest,
						bHasSkillRuleSet
							? &AuthoritativeSkillRuleSet
							: nullptr,
						PostRouteRollProvider);

			FDomainExecution Execution;
			Execution.bSuccess = Result.OrchestrationResult.bSuccess;
			Execution.CandidateAfterState =
				Result.OrchestrationResult.AfterState;
			Execution.StateDisposition = Result.OrchestrationResult.bSuccess
				? EMatchPlayAuthoritativeStateDisposition::Adopt
				: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence = Request.AttackSequence;
			return Execution;
		});
}

FMatchPlayAuthoritativeResolvePassControlPostRoutePlanResult
FMatchPlayAuthoritativeSession::ResolvePassControlPostRoutePlan()
{
	const int64 AttackSequence = AuthoritativeState.bHasCurrentAttack
		? AuthoritativeState.CurrentAttack.AttackSequence
		: 0;
	return ExecuteSerialized<
		FMatchPlayAuthoritativeResolvePassControlPostRoutePlanResult>(
		EMatchPlayAuthoritativeCommandKind::ResolvePassControlPostRoutePlan,
		true,
		AttackSequence,
		[this, AttackSequence](
			FMatchPlayAuthoritativeResolvePassControlPostRoutePlanResult& Result,
			const FMatchPlayState& BeforeState)
		{
			FMatchPlayCurrentAttackResolvePassControlPostRoutePlanRequest
				DomainRequest;
			DomainRequest.AttackSequence = AttackSequence;
			Result.OrchestrationResult =
				FMatchPlayCurrentAttackResolvePassControlPostRoutePlanOrchestrator
					::Resolve(
						BeforeState,
						DomainRequest,
						bHasSkillRuleSet
							? &AuthoritativeSkillRuleSet
							: nullptr,
						PostRouteRollProvider);

			FDomainExecution Execution;
			Execution.bSuccess = Result.OrchestrationResult.bSuccess;
			Execution.CandidateAfterState =
				Result.OrchestrationResult.AfterState;
			Execution.StateDisposition =
				Result.OrchestrationResult.bSuccess
					? EMatchPlayAuthoritativeStateDisposition::Adopt
					: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence = AttackSequence;
			return Execution;
		});
}

FMatchPlayAuthoritativeResolvePassControlAttackRollResult
FMatchPlayAuthoritativeSession::ResolvePassControlAttackRoll(
	const FMatchPlayAuthoritativeResolvePassControlAttackRollRequest& Request)
{
	return ExecuteSerialized<
		FMatchPlayAuthoritativeResolvePassControlAttackRollResult>(
		EMatchPlayAuthoritativeCommandKind::ResolvePassControlAttackRoll,
		true,
		Request.AttackSequence,
		[this, Request](
			FMatchPlayAuthoritativeResolvePassControlAttackRollResult& Result,
			const FMatchPlayState& BeforeState)
		{
			FMatchPlayCurrentAttackResolvePassControlPostRoutePlanRequest
				DomainRequest;
			DomainRequest.AttackSequence = Request.AttackSequence;
			DomainRequest.Mode =
				FMatchPlayCurrentAttackResolvePassControlPostRoutePlanRequest::EMode
					::ResolveAttackRoll;
			DomainRequest.RequestingSide = Request.RequestingSide;
			Result.OrchestrationResult =
				FMatchPlayCurrentAttackResolvePassControlPostRoutePlanOrchestrator
					::Resolve(
						BeforeState,
						DomainRequest,
						bHasSkillRuleSet ? &AuthoritativeSkillRuleSet : nullptr,
						PostRouteRollProvider);

			FDomainExecution Execution;
			Execution.bSuccess = Result.OrchestrationResult.bSuccess;
			Execution.CandidateAfterState =
				Result.OrchestrationResult.AfterState;
			Execution.StateDisposition = Result.OrchestrationResult.bSuccess
				? EMatchPlayAuthoritativeStateDisposition::Adopt
				: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence = Request.AttackSequence;
			return Execution;
		});
}

FMatchPlayAuthoritativeResolvePassControlDefenseRollResult
FMatchPlayAuthoritativeSession::ResolvePassControlDefenseRoll(
	const FMatchPlayAuthoritativeResolvePassControlDefenseRollRequest& Request)
{
	return ExecuteSerialized<
		FMatchPlayAuthoritativeResolvePassControlDefenseRollResult>(
		EMatchPlayAuthoritativeCommandKind::ResolvePassControlDefenseRoll,
		true,
		Request.AttackSequence,
		[this, Request](
			FMatchPlayAuthoritativeResolvePassControlDefenseRollResult& Result,
			const FMatchPlayState& BeforeState)
		{
			FMatchPlayCurrentAttackResolvePassControlPostRoutePlanRequest
				DomainRequest;
			DomainRequest.AttackSequence = Request.AttackSequence;
			DomainRequest.Mode =
				FMatchPlayCurrentAttackResolvePassControlPostRoutePlanRequest::EMode
					::ResolveDefenseRoll;
			DomainRequest.RequestingSide = Request.RequestingSide;
			Result.OrchestrationResult =
				FMatchPlayCurrentAttackResolvePassControlPostRoutePlanOrchestrator
					::Resolve(
						BeforeState,
						DomainRequest,
						bHasSkillRuleSet ? &AuthoritativeSkillRuleSet : nullptr,
						PostRouteRollProvider);

			bool bSuccess = Result.OrchestrationResult.bSuccess;
			FMatchPlayState CandidateAfterState =
				Result.OrchestrationResult.AfterState;
			if (bSuccess)
			{
				Result.bTerminalCompletionAttempted = true;
				Result.TerminalResult =
					FMatchPlayCurrentAttackApplyPassControlTerminalResolutionOrchestrator
						::Resolve(
							CandidateAfterState,
							bHasSkillRuleSet
								? &AuthoritativeSkillRuleSet
								: nullptr);
				bSuccess = Result.TerminalResult.bSuccess;
				if (bSuccess)
				{
					CandidateAfterState = Result.TerminalResult.AfterState;
				}
			}

			FDomainExecution Execution;
			Execution.bSuccess = bSuccess;
			Execution.CandidateAfterState = CandidateAfterState;
			Execution.StateDisposition = bSuccess
				? EMatchPlayAuthoritativeStateDisposition::Adopt
				: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence = Request.AttackSequence;
			return Execution;
		});
}

FMatchPlayAuthoritativeResolveDeadCornerPostRouteDecisionResult
FMatchPlayAuthoritativeSession::ResolveDeadCornerPostRouteDecision()
{
	const int64 AttackSequence = AuthoritativeState.bHasCurrentAttack
		? AuthoritativeState.CurrentAttack.AttackSequence
		: 0;
	return ExecuteSerialized<
		FMatchPlayAuthoritativeResolveDeadCornerPostRouteDecisionResult>(
		EMatchPlayAuthoritativeCommandKind
			::ResolveDeadCornerPostRouteDecision,
		true,
		AttackSequence,
		[this, AttackSequence](
			FMatchPlayAuthoritativeResolveDeadCornerPostRouteDecisionResult&
				Result,
			const FMatchPlayState& BeforeState)
		{
			FMatchPlayCurrentAttackResolveDeadCornerPostRouteDecisionRequest
				DomainRequest;
			DomainRequest.AttackSequence = AttackSequence;
			Result.OrchestrationResult =
				FMatchPlayCurrentAttackResolveDeadCornerPostRouteDecisionOrchestrator
					::Resolve(
						BeforeState,
						DomainRequest,
						bHasSkillRuleSet
							? &AuthoritativeSkillRuleSet
							: nullptr,
						PostRouteRollProvider);

			FDomainExecution Execution;
			Execution.bSuccess = Result.OrchestrationResult.bSuccess;
			Execution.CandidateAfterState =
				Result.OrchestrationResult.AfterState;
			Execution.StateDisposition =
				Result.OrchestrationResult.bSuccess
					? EMatchPlayAuthoritativeStateDisposition::Adopt
					: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence = AttackSequence;
			return Execution;
		});
}

FMatchPlayAuthoritativeResolveLongShotDeadCornerRollResult
FMatchPlayAuthoritativeSession::ResolveLongShotDeadCornerRoll(
	const FMatchPlayAuthoritativeResolveLongShotDeadCornerRollRequest& Request)
{
	return ExecuteSerialized<
		FMatchPlayAuthoritativeResolveLongShotDeadCornerRollResult>(
		EMatchPlayAuthoritativeCommandKind::ResolveLongShotDeadCornerRoll,
		true,
		Request.AttackSequence,
		[this, Request](
			FMatchPlayAuthoritativeResolveLongShotDeadCornerRollResult& Result,
			const FMatchPlayState& BeforeState)
		{
			FMatchPlayCurrentAttackResolveDeadCornerPostRouteDecisionRequest
				DomainRequest;
			DomainRequest.AttackSequence = Request.AttackSequence;
			DomainRequest.Mode =
				FMatchPlayCurrentAttackResolveDeadCornerPostRouteDecisionRequest
					::EMode::ResolveLongShotPairedRolls;
			DomainRequest.RequestingSide = Request.RequestingSide;
			Result.OrchestrationResult =
				FMatchPlayCurrentAttackResolveDeadCornerPostRouteDecisionOrchestrator
					::Resolve(
						BeforeState,
						DomainRequest,
						bHasSkillRuleSet ? &AuthoritativeSkillRuleSet : nullptr,
						PostRouteRollProvider);

			FDomainExecution Execution;
			Execution.bSuccess = Result.OrchestrationResult.bSuccess;
			Execution.CandidateAfterState = Result.OrchestrationResult.AfterState;
			Execution.StateDisposition = Result.OrchestrationResult.bSuccess
				? EMatchPlayAuthoritativeStateDisposition::Adopt
				: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence = Request.AttackSequence;
			return Execution;
		});
}

FMatchPlayAuthoritativeResolveThroughBallAntiOffsideDecisionResult
FMatchPlayAuthoritativeSession::ResolveThroughBallAntiOffsideDecision()
{
	const int64 AttackSequence = AuthoritativeState.bHasCurrentAttack
		? AuthoritativeState.CurrentAttack.AttackSequence
		: 0;
	return ExecuteSerialized<
		FMatchPlayAuthoritativeResolveThroughBallAntiOffsideDecisionResult>(
		EMatchPlayAuthoritativeCommandKind
			::ResolveThroughBallAntiOffsideDecision,
		true,
		AttackSequence,
		[this, AttackSequence](
			FMatchPlayAuthoritativeResolveThroughBallAntiOffsideDecisionResult&
				Result,
			const FMatchPlayState& BeforeState)
		{
			FMatchPlayCurrentAttackResolveThroughBallAntiOffsideDecisionRequest
				DomainRequest;
			DomainRequest.AttackSequence = AttackSequence;
			Result.OrchestrationResult =
				FMatchPlayCurrentAttackResolveThroughBallAntiOffsideDecisionOrchestrator
					::Resolve(
						BeforeState,
						DomainRequest,
						bHasSkillRuleSet
							? &AuthoritativeSkillRuleSet
							: nullptr,
						PostRouteRollProvider);

			FDomainExecution Execution;
			Execution.bSuccess = Result.OrchestrationResult.bSuccess;
			Execution.CandidateAfterState =
				Result.OrchestrationResult.AfterState;
			Execution.StateDisposition =
				Result.OrchestrationResult.bSuccess
					? EMatchPlayAuthoritativeStateDisposition::Adopt
					: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence = AttackSequence;
			return Execution;
		});
}

FMatchPlayAuthoritativeResolveThroughBallAntiOffsideAttackRollResult
FMatchPlayAuthoritativeSession::ResolveThroughBallAntiOffsideAttackRoll(
	const FMatchPlayAuthoritativeResolveThroughBallAntiOffsideAttackRollRequest&
		Request)
{
	return ExecuteSerialized<
		FMatchPlayAuthoritativeResolveThroughBallAntiOffsideAttackRollResult>(
		EMatchPlayAuthoritativeCommandKind
			::ResolveThroughBallAntiOffsideAttackRoll,
		true,
		Request.AttackSequence,
		[this, Request](
			FMatchPlayAuthoritativeResolveThroughBallAntiOffsideAttackRollResult&
				Result,
			const FMatchPlayState& BeforeState)
		{
			FMatchPlayCurrentAttackResolveThroughBallAntiOffsideDecisionRequest
				DomainRequest;
			DomainRequest.AttackSequence = Request.AttackSequence;
			DomainRequest.Mode =
				FMatchPlayCurrentAttackResolveThroughBallAntiOffsideDecisionRequest
					::EMode::ResolveAttackRoll;
			DomainRequest.RequestingSide = Request.RequestingSide;
			Result.OrchestrationResult =
				FMatchPlayCurrentAttackResolveThroughBallAntiOffsideDecisionOrchestrator
					::Resolve(
						BeforeState,
						DomainRequest,
						bHasSkillRuleSet
							? &AuthoritativeSkillRuleSet
							: nullptr,
						PostRouteRollProvider);

			FDomainExecution Execution;
			Execution.bSuccess = Result.OrchestrationResult.bSuccess;
			Execution.CandidateAfterState =
				Result.OrchestrationResult.AfterState;
			Execution.StateDisposition = Result.OrchestrationResult.bSuccess
				? EMatchPlayAuthoritativeStateDisposition::Adopt
				: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence = Request.AttackSequence;
			return Execution;
		});
}

FMatchPlayAuthoritativeResolveDirectShotPostRouteDecisionOrPlanResult
FMatchPlayAuthoritativeSession::ResolveDirectShotPostRouteDecisionOrPlan()
{
	const int64 AttackSequence = AuthoritativeState.bHasCurrentAttack
		? AuthoritativeState.CurrentAttack.AttackSequence
		: 0;
	return ExecuteSerialized<
		FMatchPlayAuthoritativeResolveDirectShotPostRouteDecisionOrPlanResult>(
		EMatchPlayAuthoritativeCommandKind
			::ResolveDirectShotPostRouteDecisionOrPlan,
		true,
		AttackSequence,
		[this, AttackSequence](
			FMatchPlayAuthoritativeResolveDirectShotPostRouteDecisionOrPlanResult&
				Result,
			const FMatchPlayState& BeforeState)
		{
			FMatchPlayCurrentAttackResolveDirectShotPostRouteDecisionOrPlanRequest
				DomainRequest;
			DomainRequest.AttackSequence = AttackSequence;
			Result.OrchestrationResult =
				FMatchPlayCurrentAttackResolveDirectShotPostRouteDecisionOrPlanOrchestrator
					::Resolve(
						BeforeState,
						DomainRequest,
						bHasSkillRuleSet
							? &AuthoritativeSkillRuleSet
							: nullptr,
						PostRouteRollProvider);

			FDomainExecution Execution;
			Execution.bSuccess = Result.OrchestrationResult.bSuccess;
			Execution.CandidateAfterState =
				Result.OrchestrationResult.AfterState;
			Execution.StateDisposition =
				Result.OrchestrationResult.bSuccess
					? EMatchPlayAuthoritativeStateDisposition::Adopt
					: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence = AttackSequence;
			return Execution;
		});
}

FMatchPlayAuthoritativeResolveLongShotDirectAttackRollResult
FMatchPlayAuthoritativeSession::ResolveLongShotDirectAttackRoll(
	const FMatchPlayAuthoritativeResolveLongShotDirectAttackRollRequest& Request)
{
	return ExecuteSerialized<
		FMatchPlayAuthoritativeResolveLongShotDirectAttackRollResult>(
		EMatchPlayAuthoritativeCommandKind::ResolveLongShotDirectAttackRoll,
		true,
		Request.AttackSequence,
		[this, Request](
			FMatchPlayAuthoritativeResolveLongShotDirectAttackRollResult& Result,
			const FMatchPlayState& BeforeState)
		{
			FMatchPlayCurrentAttackResolveDirectShotPostRouteDecisionOrPlanRequest
				DomainRequest;
			DomainRequest.AttackSequence = Request.AttackSequence;
			DomainRequest.Mode =
				FMatchPlayCurrentAttackResolveDirectShotPostRouteDecisionOrPlanRequest
					::EMode::ResolveLongShotAttackRoll;
			DomainRequest.RequestingSide = Request.RequestingSide;
			Result.OrchestrationResult =
				FMatchPlayCurrentAttackResolveDirectShotPostRouteDecisionOrPlanOrchestrator
					::Resolve(
						BeforeState,
						DomainRequest,
						bHasSkillRuleSet ? &AuthoritativeSkillRuleSet : nullptr,
						PostRouteRollProvider);

			FDomainExecution Execution;
			Execution.bSuccess = Result.OrchestrationResult.bSuccess;
			Execution.CandidateAfterState = Result.OrchestrationResult.AfterState;
			Execution.StateDisposition = Result.OrchestrationResult.bSuccess
				? EMatchPlayAuthoritativeStateDisposition::Adopt
				: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence = Request.AttackSequence;
			return Execution;
		});
}

FMatchPlayAuthoritativeResolveLongShotDirectDefenseRollResult
FMatchPlayAuthoritativeSession::ResolveLongShotDirectDefenseRoll(
	const FMatchPlayAuthoritativeResolveLongShotDirectDefenseRollRequest& Request)
{
	return ExecuteSerialized<
		FMatchPlayAuthoritativeResolveLongShotDirectDefenseRollResult>(
		EMatchPlayAuthoritativeCommandKind::ResolveLongShotDirectDefenseRoll,
		true,
		Request.AttackSequence,
		[this, Request](
			FMatchPlayAuthoritativeResolveLongShotDirectDefenseRollResult& Result,
			const FMatchPlayState& BeforeState)
		{
			FMatchPlayCurrentAttackResolveDirectShotPostRouteDecisionOrPlanRequest
				DomainRequest;
			DomainRequest.AttackSequence = Request.AttackSequence;
			DomainRequest.Mode =
				FMatchPlayCurrentAttackResolveDirectShotPostRouteDecisionOrPlanRequest
					::EMode::ResolveLongShotDefenseRoll;
			DomainRequest.RequestingSide = Request.RequestingSide;
			Result.OrchestrationResult =
				FMatchPlayCurrentAttackResolveDirectShotPostRouteDecisionOrPlanOrchestrator
					::Resolve(
						BeforeState,
						DomainRequest,
						bHasSkillRuleSet ? &AuthoritativeSkillRuleSet : nullptr,
						PostRouteRollProvider);

			FDomainExecution Execution;
			Execution.bSuccess = Result.OrchestrationResult.bSuccess;
			Execution.CandidateAfterState = Result.OrchestrationResult.AfterState;
			Execution.StateDisposition = Result.OrchestrationResult.bSuccess
				? EMatchPlayAuthoritativeStateDisposition::Adopt
				: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence = Request.AttackSequence;
			return Execution;
		});
}

FMatchPlayAuthoritativeResolveCutInsideShotDirectAttackRollResult
FMatchPlayAuthoritativeSession::ResolveCutInsideShotDirectAttackRoll(
	const FMatchPlayAuthoritativeResolveCutInsideShotDirectAttackRollRequest&
		Request)
{
	return ExecuteSerialized<
		FMatchPlayAuthoritativeResolveCutInsideShotDirectAttackRollResult>(
		EMatchPlayAuthoritativeCommandKind
			::ResolveCutInsideShotDirectAttackRoll,
		true,
		Request.AttackSequence,
		[this, Request](
			FMatchPlayAuthoritativeResolveCutInsideShotDirectAttackRollResult&
				Result,
			const FMatchPlayState& BeforeState)
		{
			FMatchPlayCurrentAttackResolveDirectShotPostRouteDecisionOrPlanRequest
				DomainRequest;
			DomainRequest.AttackSequence = Request.AttackSequence;
			DomainRequest.Mode =
				FMatchPlayCurrentAttackResolveDirectShotPostRouteDecisionOrPlanRequest
					::EMode::ResolveCutInsideShotAttackRoll;
			DomainRequest.RequestingSide = Request.RequestingSide;
			Result.OrchestrationResult =
				FMatchPlayCurrentAttackResolveDirectShotPostRouteDecisionOrPlanOrchestrator
					::Resolve(
						BeforeState,
						DomainRequest,
						bHasSkillRuleSet ? &AuthoritativeSkillRuleSet : nullptr,
						PostRouteRollProvider);

			bool bSuccess = Result.OrchestrationResult.bSuccess;
			FMatchPlayState CandidateAfterState =
				Result.OrchestrationResult.AfterState;
			if (bSuccess
				&& Result.OrchestrationResult.ProgressResult.bContractComplete)
			{
				Result.bTerminalCompletionAttempted = true;
				Result.TerminalResult =
					FMatchPlayCurrentAttackApplyShotTerminalResolutionOrchestrator
						::Resolve(
							CandidateAfterState,
							bHasSkillRuleSet
								? &AuthoritativeSkillRuleSet
								: nullptr);
				bSuccess = Result.TerminalResult.bSuccess;
				if (bSuccess)
				{
					CandidateAfterState = Result.TerminalResult.AfterState;
				}
			}

			FDomainExecution Execution;
			Execution.bSuccess = bSuccess;
			Execution.CandidateAfterState = CandidateAfterState;
			Execution.StateDisposition = bSuccess
				? EMatchPlayAuthoritativeStateDisposition::Adopt
				: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence = Request.AttackSequence;
			return Execution;
		});
}

FMatchPlayAuthoritativeResolveCutInsideShotDirectDefenseRollResult
FMatchPlayAuthoritativeSession::ResolveCutInsideShotDirectDefenseRoll(
	const FMatchPlayAuthoritativeResolveCutInsideShotDirectDefenseRollRequest&
		Request)
{
	return ExecuteSerialized<
		FMatchPlayAuthoritativeResolveCutInsideShotDirectDefenseRollResult>(
		EMatchPlayAuthoritativeCommandKind
			::ResolveCutInsideShotDirectDefenseRoll,
		true,
		Request.AttackSequence,
		[this, Request](
			FMatchPlayAuthoritativeResolveCutInsideShotDirectDefenseRollResult&
				Result,
			const FMatchPlayState& BeforeState)
		{
			FMatchPlayCurrentAttackResolveDirectShotPostRouteDecisionOrPlanRequest
				DomainRequest;
			DomainRequest.AttackSequence = Request.AttackSequence;
			DomainRequest.Mode =
				FMatchPlayCurrentAttackResolveDirectShotPostRouteDecisionOrPlanRequest
					::EMode::ResolveCutInsideShotDefenseRoll;
			DomainRequest.RequestingSide = Request.RequestingSide;
			Result.OrchestrationResult =
				FMatchPlayCurrentAttackResolveDirectShotPostRouteDecisionOrPlanOrchestrator
					::Resolve(
						BeforeState,
						DomainRequest,
						bHasSkillRuleSet ? &AuthoritativeSkillRuleSet : nullptr,
						PostRouteRollProvider);

			bool bSuccess = Result.OrchestrationResult.bSuccess;
			FMatchPlayState CandidateAfterState =
				Result.OrchestrationResult.AfterState;
			if (bSuccess)
			{
				Result.bTerminalCompletionAttempted = true;
				Result.TerminalResult =
					FMatchPlayCurrentAttackApplyShotTerminalResolutionOrchestrator
						::Resolve(
							CandidateAfterState,
							bHasSkillRuleSet
								? &AuthoritativeSkillRuleSet
								: nullptr);
				bSuccess = Result.TerminalResult.bSuccess;
				if (bSuccess)
				{
					CandidateAfterState = Result.TerminalResult.AfterState;
				}
			}

			FDomainExecution Execution;
			Execution.bSuccess = bSuccess;
			Execution.CandidateAfterState = CandidateAfterState;
			Execution.StateDisposition = bSuccess
				? EMatchPlayAuthoritativeStateDisposition::Adopt
				: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence = Request.AttackSequence;
			return Execution;
		});
}

FMatchPlayAuthoritativeResolveCutInsideShotDeadCornerRollResult
FMatchPlayAuthoritativeSession::ResolveCutInsideShotDeadCornerRoll(
	const FMatchPlayAuthoritativeResolveCutInsideShotDeadCornerRollRequest&
		Request)
{
	return ExecuteSerialized<
		FMatchPlayAuthoritativeResolveCutInsideShotDeadCornerRollResult>(
		EMatchPlayAuthoritativeCommandKind::ResolveCutInsideShotDeadCornerRoll,
		true,
		Request.AttackSequence,
		[this, Request](
			FMatchPlayAuthoritativeResolveCutInsideShotDeadCornerRollResult& Result,
			const FMatchPlayState& BeforeState)
		{
			FMatchPlayCurrentAttackResolveDeadCornerPostRouteDecisionRequest
				DomainRequest;
			DomainRequest.AttackSequence = Request.AttackSequence;
			DomainRequest.Mode =
				FMatchPlayCurrentAttackResolveDeadCornerPostRouteDecisionRequest
					::EMode::ResolveCutInsideShotPairedRolls;
			DomainRequest.RequestingSide = Request.RequestingSide;
			Result.OrchestrationResult =
				FMatchPlayCurrentAttackResolveDeadCornerPostRouteDecisionOrchestrator
					::Resolve(
						BeforeState,
						DomainRequest,
						bHasSkillRuleSet ? &AuthoritativeSkillRuleSet : nullptr,
						PostRouteRollProvider);

			bool bSuccess = Result.OrchestrationResult.bSuccess;
			FMatchPlayState CandidateAfterState =
				Result.OrchestrationResult.AfterState;
			if (bSuccess)
			{
				Result.bTerminalCompletionAttempted = true;
				Result.TerminalResult =
					FMatchPlayCurrentAttackApplyShotTerminalResolutionOrchestrator
						::Resolve(
							CandidateAfterState,
							bHasSkillRuleSet
								? &AuthoritativeSkillRuleSet
								: nullptr);
				bSuccess = Result.TerminalResult.bSuccess;
				if (bSuccess)
				{
					CandidateAfterState = Result.TerminalResult.AfterState;
				}
			}

			FDomainExecution Execution;
			Execution.bSuccess = bSuccess;
			Execution.CandidateAfterState = CandidateAfterState;
			Execution.StateDisposition = bSuccess
				? EMatchPlayAuthoritativeStateDisposition::Adopt
				: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence = Request.AttackSequence;
			return Execution;
		});
}

FMatchPlayAuthoritativeResolveThroughBallBehindDefenseP1DecisionOrPlanResult
FMatchPlayAuthoritativeSession::ResolveThroughBallBehindDefenseP1DecisionOrPlan()
{
	const int64 AttackSequence = AuthoritativeState.bHasCurrentAttack
		? AuthoritativeState.CurrentAttack.AttackSequence : 0;
	return ExecuteSerialized<
		FMatchPlayAuthoritativeResolveThroughBallBehindDefenseP1DecisionOrPlanResult>(
		EMatchPlayAuthoritativeCommandKind
			::ResolveThroughBallBehindDefenseP1DecisionOrPlan,
		true, AttackSequence,
		[this, AttackSequence](
			FMatchPlayAuthoritativeResolveThroughBallBehindDefenseP1DecisionOrPlanResult& Result,
			const FMatchPlayState& BeforeState)
		{
			FMatchPlayCurrentAttackResolveThroughBallBehindDefenseP1DecisionOrPlanRequest Request;
			Request.AttackSequence = AttackSequence;
			Result.OrchestrationResult =
				FMatchPlayCurrentAttackResolveThroughBallBehindDefenseP1DecisionOrPlanOrchestrator::Resolve(
					BeforeState, Request,
					bHasSkillRuleSet ? &AuthoritativeSkillRuleSet : nullptr,
					PostRouteRollProvider);
			FDomainExecution Execution;
			Execution.bSuccess = Result.OrchestrationResult.bSuccess;
			Execution.CandidateAfterState = Result.OrchestrationResult.AfterState;
			Execution.StateDisposition = Result.OrchestrationResult.bSuccess
				? EMatchPlayAuthoritativeStateDisposition::Adopt
				: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence = AttackSequence;
			return Execution;
		});
}

FMatchPlayAuthoritativeResolveThroughBallBehindDefenseP1AttackRollResult
FMatchPlayAuthoritativeSession::ResolveThroughBallBehindDefenseP1AttackRoll(
	const
		FMatchPlayAuthoritativeResolveThroughBallBehindDefenseP1AttackRollRequest&
			Request)
{
	return ExecuteSerialized<
		FMatchPlayAuthoritativeResolveThroughBallBehindDefenseP1AttackRollResult>(
		EMatchPlayAuthoritativeCommandKind
			::ResolveThroughBallBehindDefenseP1AttackRoll,
		true,
		Request.AttackSequence,
		[this, Request](
			FMatchPlayAuthoritativeResolveThroughBallBehindDefenseP1AttackRollResult&
				Result,
			const FMatchPlayState& BeforeState)
		{
			FMatchPlayCurrentAttackResolveThroughBallBehindDefenseP1DecisionOrPlanRequest
				DomainRequest;
			DomainRequest.AttackSequence = Request.AttackSequence;
			DomainRequest.Mode =
				FMatchPlayCurrentAttackResolveThroughBallBehindDefenseP1DecisionOrPlanRequest
					::EMode::ResolveAttackRoll;
			DomainRequest.RequestingSide = Request.RequestingSide;
			Result.OrchestrationResult =
				FMatchPlayCurrentAttackResolveThroughBallBehindDefenseP1DecisionOrPlanOrchestrator
					::Resolve(
						BeforeState,
						DomainRequest,
						bHasSkillRuleSet
							? &AuthoritativeSkillRuleSet
							: nullptr,
						PostRouteRollProvider);

			FDomainExecution Execution;
			Execution.bSuccess = Result.OrchestrationResult.bSuccess;
			Execution.CandidateAfterState =
				Result.OrchestrationResult.AfterState;
			Execution.StateDisposition = Result.OrchestrationResult.bSuccess
				? EMatchPlayAuthoritativeStateDisposition::Adopt
				: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence = Request.AttackSequence;
			return Execution;
		});
}

FMatchPlayAuthoritativeResolveThroughBallBehindDefenseP1DefenseRollResult
FMatchPlayAuthoritativeSession::ResolveThroughBallBehindDefenseP1DefenseRoll(
	const
		FMatchPlayAuthoritativeResolveThroughBallBehindDefenseP1DefenseRollRequest&
			Request)
{
	return ExecuteSerialized<
		FMatchPlayAuthoritativeResolveThroughBallBehindDefenseP1DefenseRollResult>(
		EMatchPlayAuthoritativeCommandKind
			::ResolveThroughBallBehindDefenseP1DefenseRoll,
		true,
		Request.AttackSequence,
		[this, Request](
			FMatchPlayAuthoritativeResolveThroughBallBehindDefenseP1DefenseRollResult&
				Result,
			const FMatchPlayState& BeforeState)
		{
			FMatchPlayCurrentAttackResolveThroughBallBehindDefenseP1DecisionOrPlanRequest
				DomainRequest;
			DomainRequest.AttackSequence = Request.AttackSequence;
			DomainRequest.Mode =
				FMatchPlayCurrentAttackResolveThroughBallBehindDefenseP1DecisionOrPlanRequest
					::EMode::ResolveDefenseRoll;
			DomainRequest.RequestingSide = Request.RequestingSide;
			Result.OrchestrationResult =
				FMatchPlayCurrentAttackResolveThroughBallBehindDefenseP1DecisionOrPlanOrchestrator
					::Resolve(
						BeforeState,
						DomainRequest,
						bHasSkillRuleSet
							? &AuthoritativeSkillRuleSet
							: nullptr,
						PostRouteRollProvider);

			FDomainExecution Execution;
			Execution.bSuccess = Result.OrchestrationResult.bSuccess;
			Execution.CandidateAfterState =
				Result.OrchestrationResult.AfterState;
			Execution.StateDisposition = Result.OrchestrationResult.bSuccess
				? EMatchPlayAuthoritativeStateDisposition::Adopt
				: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence = Request.AttackSequence;
			return Execution;
		});
}

FMatchPlayAuthoritativeResolveSingleCardFinishingFormulaResult
FMatchPlayAuthoritativeSession::ResolveSingleCardFinishingFormula()
{
	const int64 AttackSequence = AuthoritativeState.bHasCurrentAttack
		? AuthoritativeState.CurrentAttack.AttackSequence
		: 0;
	return ExecuteSerialized<
		FMatchPlayAuthoritativeResolveSingleCardFinishingFormulaResult>(
		EMatchPlayAuthoritativeCommandKind::ResolveSingleCardFinishingFormula,
		true,
		AttackSequence,
		[this, AttackSequence](
			FMatchPlayAuthoritativeResolveSingleCardFinishingFormulaResult& Result,
			const FMatchPlayState& BeforeState)
		{
			Result.OrchestrationResult =
				FMatchPlayCurrentAttackResolveSingleCardFinishingFormulaOrchestrator
					::Resolve(
						BeforeState,
						bHasSkillRuleSet
							? &AuthoritativeSkillRuleSet
							: nullptr);

			FDomainExecution Execution;
			Execution.bSuccess = Result.OrchestrationResult.bSuccess;
			Execution.CandidateAfterState =
				Result.OrchestrationResult.AfterState;
			Execution.StateDisposition = Result.OrchestrationResult.bSuccess
				? EMatchPlayAuthoritativeStateDisposition::Adopt
				: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence = AttackSequence;
			return Execution;
		});
}

FMatchPlayAuthoritativeResolveThroughBallFeetFormulaResult
FMatchPlayAuthoritativeSession::ResolveThroughBallFeetFormula()
{
	const int64 AttackSequence = AuthoritativeState.bHasCurrentAttack
		? AuthoritativeState.CurrentAttack.AttackSequence
		: 0;
	return ExecuteSerialized<
		FMatchPlayAuthoritativeResolveThroughBallFeetFormulaResult>(
		EMatchPlayAuthoritativeCommandKind::ResolveThroughBallFeetFormula,
		true,
		AttackSequence,
		[this, AttackSequence](
			FMatchPlayAuthoritativeResolveThroughBallFeetFormulaResult& Result,
			const FMatchPlayState& BeforeState)
		{
			Result.OrchestrationResult =
				FMatchPlayCurrentAttackResolveThroughBallFeetFormulaOrchestrator
					::Resolve(
						BeforeState,
						bHasSkillRuleSet
							? &AuthoritativeSkillRuleSet
							: nullptr);

			FDomainExecution Execution;
			Execution.bSuccess = Result.OrchestrationResult.bSuccess;
			Execution.CandidateAfterState =
				Result.OrchestrationResult.AfterState;
			Execution.StateDisposition = Result.OrchestrationResult.bSuccess
				? EMatchPlayAuthoritativeStateDisposition::Adopt
				: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence = AttackSequence;
			return Execution;
		});
}

FMatchPlayAuthoritativeResolveThroughBallBehindDefenseP1FormulaResult
FMatchPlayAuthoritativeSession::ResolveThroughBallBehindDefenseP1Formula()
{
	const int64 AttackSequence = AuthoritativeState.bHasCurrentAttack
		? AuthoritativeState.CurrentAttack.AttackSequence
		: 0;
	return ExecuteSerialized<
		FMatchPlayAuthoritativeResolveThroughBallBehindDefenseP1FormulaResult>(
		EMatchPlayAuthoritativeCommandKind
			::ResolveThroughBallBehindDefenseP1Formula,
		true,
		AttackSequence,
		[this, AttackSequence](
			FMatchPlayAuthoritativeResolveThroughBallBehindDefenseP1FormulaResult&
				Result,
			const FMatchPlayState& BeforeState)
		{
			Result.OrchestrationResult =
				FMatchPlayCurrentAttackResolveThroughBallBehindDefenseP1FormulaOrchestrator
					::Resolve(
						BeforeState,
						bHasSkillRuleSet
							? &AuthoritativeSkillRuleSet
							: nullptr);

			FDomainExecution Execution;
			Execution.bSuccess = Result.OrchestrationResult.bSuccess;
			Execution.CandidateAfterState =
				Result.OrchestrationResult.AfterState;
			Execution.StateDisposition = Result.OrchestrationResult.bSuccess
				? EMatchPlayAuthoritativeStateDisposition::Adopt
				: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence = AttackSequence;
			return Execution;
		});
}

FMatchPlayAuthoritativeResolveThroughBallBehindDefenseP2DecisionResult
FMatchPlayAuthoritativeSession::ResolveThroughBallBehindDefenseP2Decision()
{
	const int64 AttackSequence = AuthoritativeState.bHasCurrentAttack
		? AuthoritativeState.CurrentAttack.AttackSequence
		: 0;
	return ExecuteSerialized<
		FMatchPlayAuthoritativeResolveThroughBallBehindDefenseP2DecisionResult>(
		EMatchPlayAuthoritativeCommandKind
			::ResolveThroughBallBehindDefenseP2Decision,
		true,
		AttackSequence,
		[this, AttackSequence](
			FMatchPlayAuthoritativeResolveThroughBallBehindDefenseP2DecisionResult&
				Result,
			const FMatchPlayState& BeforeState)
		{
			Result.OrchestrationResult =
				FMatchPlayCurrentAttackResolveThroughBallBehindDefenseP2DecisionOrchestrator
					::Resolve(
						BeforeState,
						bHasSkillRuleSet
							? &AuthoritativeSkillRuleSet
							: nullptr,
						PostRouteRollProvider);

			FDomainExecution Execution;
			Execution.bSuccess = Result.OrchestrationResult.bSuccess;
			Execution.CandidateAfterState =
				Result.OrchestrationResult.AfterState;
			Execution.StateDisposition = Result.OrchestrationResult.bSuccess
				? EMatchPlayAuthoritativeStateDisposition::Adopt
				: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence = AttackSequence;
			return Execution;
		});
}

FMatchPlayAuthoritativeSubmitThroughBallOneOnOneShotChoiceResult
FMatchPlayAuthoritativeSession::SubmitThroughBallOneOnOneShotChoice(
	const FMatchPlayAuthoritativeSubmitThroughBallOneOnOneShotChoiceRequest&
		Request)
{
	const int64 AttackSequence = AuthoritativeState.bHasCurrentAttack
		? AuthoritativeState.CurrentAttack.AttackSequence
		: 0;
	return ExecuteSerialized<
		FMatchPlayAuthoritativeSubmitThroughBallOneOnOneShotChoiceResult>(
		EMatchPlayAuthoritativeCommandKind
			::SubmitThroughBallOneOnOneShotChoice,
		true,
		AttackSequence,
		[this, Request, AttackSequence](
			FMatchPlayAuthoritativeSubmitThroughBallOneOnOneShotChoiceResult&
				Result,
			const FMatchPlayState& BeforeState)
		{
			FMatchPlayCurrentAttackThroughBallOneOnOneShotChoiceSelectionRequest
				DomainRequest;
			DomainRequest.AttackSequence = AttackSequence;
			DomainRequest.RequestingSide = Request.RequestingSide;
			DomainRequest.Choice = Request.Choice;
			Result.ChoiceResult =
				FMatchPlayCurrentAttackThroughBallOneOnOneShotChoiceSelectionWriter
					::Select(
						BeforeState,
						DomainRequest,
						bHasSkillRuleSet
							? &AuthoritativeSkillRuleSet
							: nullptr);

			FDomainExecution Execution;
			Execution.bSuccess = Result.ChoiceResult.bSuccess;
			Execution.CandidateAfterState = Result.ChoiceResult.AfterState;
			Execution.StateDisposition = Result.ChoiceResult.bSuccess
				? EMatchPlayAuthoritativeStateDisposition::Adopt
				: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence = AttackSequence;
			return Execution;
		});
}

FMatchPlayAuthoritativeResolveThroughBallOneOnOneChipShotDecisionResult
FMatchPlayAuthoritativeSession::ResolveThroughBallOneOnOneChipShotDecision()
{
	const int64 AttackSequence = AuthoritativeState.bHasCurrentAttack
		? AuthoritativeState.CurrentAttack.AttackSequence
		: 0;
	return ExecuteSerialized<
		FMatchPlayAuthoritativeResolveThroughBallOneOnOneChipShotDecisionResult>(
		EMatchPlayAuthoritativeCommandKind
			::ResolveThroughBallOneOnOneChipShotDecision,
		true,
		AttackSequence,
		[this, AttackSequence](
			FMatchPlayAuthoritativeResolveThroughBallOneOnOneChipShotDecisionResult&
				Result,
			const FMatchPlayState& BeforeState)
		{
			Result.OrchestrationResult =
				FMatchPlayCurrentAttackResolveThroughBallOneOnOneChipShotDecisionOrchestrator
					::Resolve(
						BeforeState,
						bHasSkillRuleSet
							? &AuthoritativeSkillRuleSet
							: nullptr,
						PostRouteRollProvider);

			FDomainExecution Execution;
			Execution.bSuccess = Result.OrchestrationResult.bSuccess;
			Execution.CandidateAfterState =
				Result.OrchestrationResult.AfterState;
			Execution.StateDisposition = Result.OrchestrationResult.bSuccess
				? EMatchPlayAuthoritativeStateDisposition::Adopt
				: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence = AttackSequence;
			return Execution;
		});
}

FMatchPlayAuthoritativeResolveThroughBallOneOnOneChipShotAttackRollResult
FMatchPlayAuthoritativeSession::ResolveThroughBallOneOnOneChipShotAttackRoll(
	const FMatchPlayAuthoritativeResolveThroughBallOneOnOneChipShotAttackRollRequest&
		Request)
{
	return ExecuteSerialized<
		FMatchPlayAuthoritativeResolveThroughBallOneOnOneChipShotAttackRollResult>(
		EMatchPlayAuthoritativeCommandKind
			::ResolveThroughBallOneOnOneChipShotAttackRoll,
		true,
		Request.AttackSequence,
		[this, Request](
			FMatchPlayAuthoritativeResolveThroughBallOneOnOneChipShotAttackRollResult&
				Result,
			const FMatchPlayState& BeforeState)
		{
			FMatchPlayCurrentAttackResolveThroughBallOneOnOneChipShotDecisionRequest
				DomainRequest;
			DomainRequest.AttackSequence = Request.AttackSequence;
			DomainRequest.Mode =
				FMatchPlayCurrentAttackResolveThroughBallOneOnOneChipShotDecisionRequest
					::EMode::ResolveAttackRoll;
			DomainRequest.RequestingSide = Request.RequestingSide;
			Result.OrchestrationResult =
				FMatchPlayCurrentAttackResolveThroughBallOneOnOneChipShotDecisionOrchestrator
					::Resolve(
						BeforeState,
						DomainRequest,
						bHasSkillRuleSet
							? &AuthoritativeSkillRuleSet
							: nullptr,
						PostRouteRollProvider);
			FDomainExecution Execution;
			Execution.bSuccess = Result.OrchestrationResult.bSuccess;
			Execution.CandidateAfterState =
				Result.OrchestrationResult.AfterState;
			Execution.StateDisposition = Result.OrchestrationResult.bSuccess
				? EMatchPlayAuthoritativeStateDisposition::Adopt
				: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence = Request.AttackSequence;
			return Execution;
		});
}

FMatchPlayAuthoritativeResolveThroughBallOneOnOneDirectShotPostRoutePlanResult
FMatchPlayAuthoritativeSession::ResolveThroughBallOneOnOneDirectShotPostRoutePlan()
{
	const int64 AttackSequence = AuthoritativeState.bHasCurrentAttack
		? AuthoritativeState.CurrentAttack.AttackSequence : 0;
	return ExecuteSerialized<
		FMatchPlayAuthoritativeResolveThroughBallOneOnOneDirectShotPostRoutePlanResult>(
		EMatchPlayAuthoritativeCommandKind::ResolveThroughBallOneOnOneDirectShotPostRoutePlan,
		true,
		AttackSequence,
		[this, AttackSequence](
			FMatchPlayAuthoritativeResolveThroughBallOneOnOneDirectShotPostRoutePlanResult& Result,
			const FMatchPlayState& BeforeState)
		{
			Result.OrchestrationResult =
				FMatchPlayCurrentAttackResolveThroughBallOneOnOneDirectShotPostRoutePlanOrchestrator::Resolve(
					BeforeState,
					bHasSkillRuleSet ? &AuthoritativeSkillRuleSet : nullptr,
					PostRouteRollProvider);
			FDomainExecution Execution;
			Execution.bSuccess = Result.OrchestrationResult.bSuccess;
			Execution.CandidateAfterState = Result.OrchestrationResult.AfterState;
			Execution.StateDisposition = Result.OrchestrationResult.bSuccess
				? EMatchPlayAuthoritativeStateDisposition::Adopt
				: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence = AttackSequence;
			return Execution;
		});
}

FMatchPlayAuthoritativeResolveThroughBallOneOnOneDirectShotAttackRollResult
FMatchPlayAuthoritativeSession::ResolveThroughBallOneOnOneDirectShotAttackRoll(
	const FMatchPlayAuthoritativeResolveThroughBallOneOnOneDirectShotAttackRollRequest&
		Request)
{
	return ExecuteSerialized<
		FMatchPlayAuthoritativeResolveThroughBallOneOnOneDirectShotAttackRollResult>(
		EMatchPlayAuthoritativeCommandKind
			::ResolveThroughBallOneOnOneDirectShotAttackRoll,
		true,
		Request.AttackSequence,
		[this, Request](
			FMatchPlayAuthoritativeResolveThroughBallOneOnOneDirectShotAttackRollResult&
				Result,
			const FMatchPlayState& BeforeState)
		{
			FMatchPlayCurrentAttackResolveThroughBallOneOnOneDirectShotPostRoutePlanRequest
				DomainRequest;
			DomainRequest.AttackSequence = Request.AttackSequence;
			DomainRequest.Mode =
				FMatchPlayCurrentAttackResolveThroughBallOneOnOneDirectShotPostRoutePlanRequest
					::EMode::ResolveAttackRoll;
			DomainRequest.RequestingSide = Request.RequestingSide;
			Result.OrchestrationResult =
				FMatchPlayCurrentAttackResolveThroughBallOneOnOneDirectShotPostRoutePlanOrchestrator
					::Resolve(
						BeforeState,
						DomainRequest,
						bHasSkillRuleSet
							? &AuthoritativeSkillRuleSet
							: nullptr,
						PostRouteRollProvider);
			FDomainExecution Execution;
			Execution.bSuccess = Result.OrchestrationResult.bSuccess;
			Execution.CandidateAfterState =
				Result.OrchestrationResult.AfterState;
			Execution.StateDisposition = Result.OrchestrationResult.bSuccess
				? EMatchPlayAuthoritativeStateDisposition::Adopt
				: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence = Request.AttackSequence;
			return Execution;
		});
}

FMatchPlayAuthoritativeResolveThroughBallOneOnOneDirectShotDefenseRollResult
FMatchPlayAuthoritativeSession::ResolveThroughBallOneOnOneDirectShotDefenseRoll(
	const FMatchPlayAuthoritativeResolveThroughBallOneOnOneDirectShotDefenseRollRequest&
		Request)
{
	return ExecuteSerialized<
		FMatchPlayAuthoritativeResolveThroughBallOneOnOneDirectShotDefenseRollResult>(
		EMatchPlayAuthoritativeCommandKind
			::ResolveThroughBallOneOnOneDirectShotDefenseRoll,
		true,
		Request.AttackSequence,
		[this, Request](
			FMatchPlayAuthoritativeResolveThroughBallOneOnOneDirectShotDefenseRollResult&
				Result,
			const FMatchPlayState& BeforeState)
		{
			FMatchPlayCurrentAttackResolveThroughBallOneOnOneDirectShotPostRoutePlanRequest
				DomainRequest;
			DomainRequest.AttackSequence = Request.AttackSequence;
			DomainRequest.Mode =
				FMatchPlayCurrentAttackResolveThroughBallOneOnOneDirectShotPostRoutePlanRequest
					::EMode::ResolveDefenseRoll;
			DomainRequest.RequestingSide = Request.RequestingSide;
			Result.OrchestrationResult =
				FMatchPlayCurrentAttackResolveThroughBallOneOnOneDirectShotPostRoutePlanOrchestrator
					::Resolve(
						BeforeState,
						DomainRequest,
						bHasSkillRuleSet
							? &AuthoritativeSkillRuleSet
							: nullptr,
						PostRouteRollProvider);
			FDomainExecution Execution;
			Execution.bSuccess = Result.OrchestrationResult.bSuccess;
			Execution.CandidateAfterState =
				Result.OrchestrationResult.AfterState;
			Execution.StateDisposition = Result.OrchestrationResult.bSuccess
				? EMatchPlayAuthoritativeStateDisposition::Adopt
				: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence = Request.AttackSequence;
			return Execution;
		});
}

FMatchPlayAuthoritativeResolveThroughBallOneOnOneDirectShotFormulaResult
FMatchPlayAuthoritativeSession::ResolveThroughBallOneOnOneDirectShotFormula()
{
	const int64 AttackSequence = AuthoritativeState.bHasCurrentAttack
		? AuthoritativeState.CurrentAttack.AttackSequence : 0;
	return ExecuteSerialized<
		FMatchPlayAuthoritativeResolveThroughBallOneOnOneDirectShotFormulaResult>(
		EMatchPlayAuthoritativeCommandKind::ResolveThroughBallOneOnOneDirectShotFormula,
		true,
		AttackSequence,
		[this, AttackSequence](
			FMatchPlayAuthoritativeResolveThroughBallOneOnOneDirectShotFormulaResult& Result,
			const FMatchPlayState& BeforeState)
		{
			Result.OrchestrationResult =
				FMatchPlayCurrentAttackResolveThroughBallOneOnOneDirectShotFormulaOrchestrator::Resolve(
					BeforeState,
					bHasSkillRuleSet ? &AuthoritativeSkillRuleSet : nullptr);
			FDomainExecution Execution;
			Execution.bSuccess = Result.OrchestrationResult.bSuccess;
			Execution.CandidateAfterState = Result.OrchestrationResult.AfterState;
			Execution.StateDisposition = Result.OrchestrationResult.bSuccess
				? EMatchPlayAuthoritativeStateDisposition::Adopt
				: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence = AttackSequence;
			return Execution;
		});
}

FMatchPlayAuthoritativeApplyThroughBallTerminalResolutionResult
FMatchPlayAuthoritativeSession::ApplyThroughBallTerminalResolution()
{
	const int64 AttackSequence = AuthoritativeState.bHasCurrentAttack
		? AuthoritativeState.CurrentAttack.AttackSequence
		: 0;
	return ExecuteSerialized<
		FMatchPlayAuthoritativeApplyThroughBallTerminalResolutionResult>(
		EMatchPlayAuthoritativeCommandKind
			::ApplyThroughBallTerminalResolution,
		true,
		AttackSequence,
		[this, AttackSequence](
			FMatchPlayAuthoritativeApplyThroughBallTerminalResolutionResult&
				Result,
			const FMatchPlayState& BeforeState)
		{
			Result.OrchestrationResult =
				FMatchPlayCurrentAttackApplyThroughBallTerminalResolutionOrchestrator
					::Resolve(
						BeforeState,
						bHasSkillRuleSet
							? &AuthoritativeSkillRuleSet
							: nullptr);

			FDomainExecution Execution;
			Execution.bSuccess = Result.OrchestrationResult.bSuccess;
			Execution.CandidateAfterState =
				Result.OrchestrationResult.AfterState;
			Execution.StateDisposition = Result.OrchestrationResult.bSuccess
				? EMatchPlayAuthoritativeStateDisposition::Adopt
				: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence = AttackSequence;
			return Execution;
		});
}

FMatchPlayAuthoritativeApplyCrossTerminalResolutionResult
FMatchPlayAuthoritativeSession::ApplyCrossTerminalResolution()
{
	const int64 AttackSequence = AuthoritativeState.bHasCurrentAttack
		? AuthoritativeState.CurrentAttack.AttackSequence
		: 0;
	return ExecuteSerialized<
		FMatchPlayAuthoritativeApplyCrossTerminalResolutionResult>(
		EMatchPlayAuthoritativeCommandKind::ApplyCrossTerminalResolution,
		true,
		AttackSequence,
		[this, AttackSequence](
			FMatchPlayAuthoritativeApplyCrossTerminalResolutionResult& Result,
			const FMatchPlayState& BeforeState)
		{
			Result.OrchestrationResult =
				FMatchPlayCurrentAttackApplyCrossTerminalResolutionOrchestrator
					::Resolve(
						BeforeState,
						bHasSkillRuleSet
							? &AuthoritativeSkillRuleSet
							: nullptr);

			FDomainExecution Execution;
			Execution.bSuccess = Result.OrchestrationResult.bSuccess;
			Execution.CandidateAfterState =
				Result.OrchestrationResult.AfterState;
			Execution.StateDisposition = Result.OrchestrationResult.bSuccess
				? EMatchPlayAuthoritativeStateDisposition::Adopt
				: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence = AttackSequence;
			return Execution;
		});
}

FMatchPlayAuthoritativeApplyPassControlTerminalResolutionResult
FMatchPlayAuthoritativeSession::ApplyPassControlTerminalResolution()
{
	// The Session contributes only serialization and State adoption; branch,
	// Formula, and terminal effect are regenerated by the bounded authority.
	const int64 AttackSequence = AuthoritativeState.bHasCurrentAttack
		? AuthoritativeState.CurrentAttack.AttackSequence
		: 0;
	return ExecuteSerialized<
		FMatchPlayAuthoritativeApplyPassControlTerminalResolutionResult>(
		EMatchPlayAuthoritativeCommandKind
			::ApplyPassControlTerminalResolution,
		true,
		AttackSequence,
		[this, AttackSequence](
			FMatchPlayAuthoritativeApplyPassControlTerminalResolutionResult&
				Result,
			const FMatchPlayState& BeforeState)
		{
			Result.OrchestrationResult =
				FMatchPlayCurrentAttackApplyPassControlTerminalResolutionOrchestrator
					::Resolve(
						BeforeState,
						bHasSkillRuleSet
							? &AuthoritativeSkillRuleSet
							: nullptr);

			FDomainExecution Execution;
			Execution.bSuccess = Result.OrchestrationResult.bSuccess;
			Execution.CandidateAfterState =
				Result.OrchestrationResult.AfterState;
			Execution.StateDisposition = Result.OrchestrationResult.bSuccess
				? EMatchPlayAuthoritativeStateDisposition::Adopt
				: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence = AttackSequence;
			return Execution;
		});
}

FMatchPlayAuthoritativeApplyShotTerminalResolutionResult
FMatchPlayAuthoritativeSession::ApplyShotTerminalResolution()
{
	// Session owns only serialization and adoption; all Shot provenance and
	// terminal semantics are regenerated by the bounded domain authority.
	const int64 AttackSequence = AuthoritativeState.bHasCurrentAttack
		? AuthoritativeState.CurrentAttack.AttackSequence
		: 0;
	return ExecuteSerialized<
		FMatchPlayAuthoritativeApplyShotTerminalResolutionResult>(
		EMatchPlayAuthoritativeCommandKind::ApplyShotTerminalResolution,
		true,
		AttackSequence,
		[this, AttackSequence](
			FMatchPlayAuthoritativeApplyShotTerminalResolutionResult& Result,
			const FMatchPlayState& BeforeState)
		{
			Result.OrchestrationResult =
				FMatchPlayCurrentAttackApplyShotTerminalResolutionOrchestrator
					::Resolve(
						BeforeState,
						bHasSkillRuleSet
							? &AuthoritativeSkillRuleSet
							: nullptr);

			FDomainExecution Execution;
			Execution.bSuccess = Result.OrchestrationResult.bSuccess;
			Execution.CandidateAfterState =
				Result.OrchestrationResult.AfterState;
			Execution.StateDisposition = Result.OrchestrationResult.bSuccess
				? EMatchPlayAuthoritativeStateDisposition::Adopt
				: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence = AttackSequence;
			return Execution;
		});
}

FMatchPlayAuthoritativeAdvanceAfterTerminalResult
FMatchPlayAuthoritativeSession::AdvanceAfterTerminal(
	const FMatchPlayAuthoritativeAdvanceAfterTerminalRequest& Request)
{
	return ExecuteSerialized<
		FMatchPlayAuthoritativeAdvanceAfterTerminalResult>(
		EMatchPlayAuthoritativeCommandKind::AdvanceAfterTerminal,
		true,
		Request.AttackSequence,
		[this, Request](
			FMatchPlayAuthoritativeAdvanceAfterTerminalResult& Result,
			const FMatchPlayState& BeforeState)
		{
			Result.CompletionResult =
				FMatchPlayCurrentAttackCompletion::AdvanceAfterTerminal(
					BeforeState,
					Request.AttackSequence,
					Request.RequestingSide,
					RecoveryProvider);

			FDomainExecution Execution;
			Execution.bSuccess = Result.CompletionResult.bSuccess;
			Execution.CandidateAfterState =
				Result.CompletionResult.AfterState;
			Execution.StateDisposition = Result.CompletionResult.bSuccess
				? EMatchPlayAuthoritativeStateDisposition::Adopt
				: EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
			Execution.AttackSequence = Request.AttackSequence;
			return Execution;
		});
}

FMatchPlayState FMatchPlayAuthoritativeSession::GetStateSnapshot() const
{
	return AuthoritativeState;
}
