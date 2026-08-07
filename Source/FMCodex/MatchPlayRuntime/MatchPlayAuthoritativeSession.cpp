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

FMatchPlayState FMatchPlayAuthoritativeSession::GetStateSnapshot() const
{
	return AuthoritativeState;
}
