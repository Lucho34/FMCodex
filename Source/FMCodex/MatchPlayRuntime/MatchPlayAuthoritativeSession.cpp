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

FMatchPlayState FMatchPlayAuthoritativeSession::GetStateSnapshot() const
{
	return AuthoritativeState;
}
