#include "MatchPlayAuthoritativeSession.h"

namespace MatchPlayAuthoritativeSession
{
	constexpr const TCHAR* NotInitializedMessage =
		TEXT("Match play session is not initialized.");
	constexpr const TCHAR* AlreadyInitializedMessage =
		TEXT("Match play session is already initialized.");
	constexpr const TCHAR* ReentrantCommandMessage =
		TEXT("Match play session rejected a reentrant command.");

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
	IMatchPlayInitialRouteRollProvider& InInitialRouteRollProvider,
	IMatchPlayPostRouteRollProvider& InPostRouteRollProvider,
	const FSkillRuleSnapshotSet& InSkillRuleSet)
	: InitialRouteRollProvider(&InInitialRouteRollProvider)
	, PostRouteRollProvider(&InPostRouteRollProvider)
	, AuthoritativeSkillRuleSet(InSkillRuleSet)
	, bHasSkillRuleSet(true)
{
}

FMatchPlayAuthoritativeSession::~FMatchPlayAuthoritativeSession() = default;

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
		[Request, AttackSequence](
			FMatchPlayAuthoritativeSubmitMarkerResult& Result,
			const FMatchPlayState& BeforeState)
		{
			FMatchPlayCurrentAttackMarkerSelectionRequest DomainRequest;
			DomainRequest.AttackSequence = AttackSequence;
			DomainRequest.RequestingSide = Request.RequestingSide;
			DomainRequest.MarkerCardId = Request.MarkerCardId;
			Result.MarkerResult =
				FMatchPlayCurrentAttackMarkerSelectionWriter::Select(
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
	const int64 AttackSequence = AuthoritativeState.bHasCurrentAttack
		? AuthoritativeState.CurrentAttack.AttackSequence
		: 0;
	return ExecuteSerialized<
		FMatchPlayAuthoritativeSubmitBranchIntentResult>(
		EMatchPlayAuthoritativeCommandKind::SubmitBranchIntent,
		true,
		AttackSequence,
		[Request, AttackSequence](
			FMatchPlayAuthoritativeSubmitBranchIntentResult& Result,
			const FMatchPlayState& BeforeState)
		{
			FMatchPlayCurrentAttackBranchIntentSelectionRequest
				DomainRequest;
			DomainRequest.AttackSequence = AttackSequence;
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
			Execution.AttackSequence = AttackSequence;
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

FMatchPlayState FMatchPlayAuthoritativeSession::GetStateSnapshot() const
{
	return AuthoritativeState;
}
