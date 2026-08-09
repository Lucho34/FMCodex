#include "MatchPlayCurrentAttackResolveThroughBallBehindDefenseP2DecisionOrchestrator.h"

#include "MatchPlayCurrentAttackResolveThroughBallBehindDefenseP1FormulaOrchestrator.h"

namespace MatchPlayCurrentAttackResolveThroughBallBehindDefenseP2Decision
{
	using FResult =
		FMatchPlayCurrentAttackResolveThroughBallBehindDefenseP2DecisionResult;
	using EError =
		EMatchPlayCurrentAttackResolveThroughBallBehindDefenseP2DecisionErrorCode;
	using EPhase = EMatchPlayCurrentAttackPostRouteRollPhase;
	using EPurpose = EMatchPlayCurrentAttackPostRouteRollPurpose;

	void SetFailure(
		FResult& Result,
		const EError ErrorCode,
		const FString& ErrorMessage,
		const FName InvalidField = NAME_None)
	{
		Result.ErrorCode = ErrorCode;
		Result.ErrorMessage = ErrorMessage;
		Result.InvalidField = InvalidField;
	}

	EError MapProviderValidationError(
		const EMatchPlayPostRouteRollProviderResultValidationErrorCode ErrorCode)
	{
		return ErrorCode
			== EMatchPlayPostRouteRollProviderResultValidationErrorCode
				::ProviderFailure
			? EError::PostRouteRollProviderFailed
			: EError::MalformedPostRouteRollProviderResult;
	}

	bool BuildP1ProvenanceState(FResult& Result)
	{
		Result.P1ProvenanceState = Result.BeforeState;
		FMatchPlayCurrentAttackResolutionSession& Session =
			Result.P1ProvenanceState.CurrentAttack.ResolutionSession;
		FMatchPlayCurrentAttackPostRouteRollProgress& Progress =
			Session.PostRouteRollProgress;
		if (Progress.Phase == EPhase::BehindDefenseP2)
		{
			Progress.Phase = EPhase::PrimaryBranch;
			Progress.RollRecords.SetNum(2);
			// This is a historical P1 provenance projection. The late
			// OneOnOne choice did not exist at that point in the flow.
			Session.ThroughBallOneOnOneShotChoice =
				EMatchPlayThroughBallOneOnOneShotChoice::None;
		}
		else if (Progress.Phase != EPhase::PrimaryBranch)
		{
			SetFailure(
				Result,
				EError::UnsupportedPostRoutePhase,
				TEXT("BehindDefense P2 requires PrimaryBranch or BehindDefenseP2 progress."));
			return false;
		}

		const auto Validation =
			FMatchPlayCurrentAttackResolutionSessionStateValidator::Validate(
				Result.P1ProvenanceState);
		if (!Validation.bIsCanonical)
		{
			SetFailure(
				Result,
				EError::InvalidP1ProvenanceState,
				Validation.ErrorMessage);
			return false;
		}
		Result.P1ProvenanceProgressResult =
			FMatchPlayCurrentAttackPostRouteRollProgressQuery::Evaluate(Session);
		if (!Result.P1ProvenanceProgressResult.bIsCanonical)
		{
			SetFailure(
				Result,
				EError::InvalidP1ProvenanceState,
				Result.P1ProvenanceProgressResult.ErrorMessage);
			return false;
		}
		if (!Result.P1ProvenanceProgressResult.bContractComplete)
		{
			SetFailure(
				Result,
				EError::IncompleteP1Progress,
				TEXT("BehindDefense P2 requires complete P1 Attack and Defense records."));
			return false;
		}
		return true;
	}
} // namespace MatchPlayCurrentAttackResolveThroughBallBehindDefenseP2Decision

FMatchPlayCurrentAttackResolveThroughBallBehindDefenseP2DecisionResult
FMatchPlayCurrentAttackResolveThroughBallBehindDefenseP2DecisionOrchestrator
	::Resolve(
		const FMatchPlayState& BeforeState,
		const FSkillRuleSnapshotSet* SkillRuleSet,
		IMatchPlayPostRouteRollProvider* RollProvider)
{
	using namespace
		MatchPlayCurrentAttackResolveThroughBallBehindDefenseP2Decision;

	FResult Result;
	Result.BeforeState = BeforeState;
	Result.AfterState = BeforeState;
	if (!BeforeState.RuntimeState.bIsInitialized)
	{
		SetFailure(
			Result,
			EError::MatchPlayStateNotInitialized,
			TEXT("BehindDefense P2 resolution requires initialized MatchPlay State."));
		return Result;
	}
	if (!BeforeState.bHasCurrentAttack)
	{
		SetFailure(
			Result,
			EError::NoCurrentAttack,
			TEXT("BehindDefense P2 resolution requires an active CurrentAttack."));
		return Result;
	}
	if (!BeforeState.CurrentAttack.bHasResolutionSession)
	{
		SetFailure(
			Result,
			EError::MissingResolutionSession,
			TEXT("BehindDefense P2 resolution requires a canonical Resolution Session."));
		return Result;
	}

	Result.SessionStateValidationResult =
		FMatchPlayCurrentAttackResolutionSessionStateValidator::Validate(
			BeforeState);
	if (!Result.SessionStateValidationResult.bIsCanonical)
	{
		SetFailure(
			Result,
			EError::InvalidResolutionSessionState,
			Result.SessionStateValidationResult.ErrorMessage);
		return Result;
	}

	const FMatchPlayCurrentAttackResolutionSession& BeforeSession =
		BeforeState.CurrentAttack.ResolutionSession;
	if (BeforeSession.Stage
		!= EMatchPlayCurrentAttackResolutionStage::RouteResolved)
	{
		SetFailure(
			Result,
			EError::RouteNotResolved,
			TEXT("BehindDefense P2 resolution requires RouteResolved."));
		return Result;
	}
	if (!BeforeSession.bHasActualBranch
		|| BeforeSession.ActualBranch.ActionType != ESkillRuleType::ThroughBall
		|| BeforeSession.ActualBranch.ThroughBall
			!= EMatchPlayThroughBallActualBranch::BehindDefense)
	{
		SetFailure(
			Result,
			EError::NotThroughBallBehindDefenseBranch,
			TEXT("This operation supports only the resolved ThroughBall BehindDefense branch."));
		return Result;
	}

	Result.BeforeProgressResult =
		FMatchPlayCurrentAttackPostRouteRollProgressQuery::Evaluate(
			BeforeSession);
	if (!Result.BeforeProgressResult.bIsCanonical)
	{
		SetFailure(
			Result,
			EError::InvalidPostRouteProgress,
			Result.BeforeProgressResult.ErrorMessage);
		return Result;
	}
	if (BeforeSession.PostRouteRollProgress.Phase != EPhase::BehindDefenseP2
		&& !Result.BeforeProgressResult.bContractComplete)
	{
		SetFailure(
			Result,
			EError::IncompleteP1Progress,
			TEXT("BehindDefense P2 requires complete P1 Attack and Defense records."));
		return Result;
	}
	if (!BuildP1ProvenanceState(Result))
	{
		return Result;
	}

	Result.P1FormulaRegenerationResult =
		FMatchPlayCurrentAttackResolveThroughBallBehindDefenseP1FormulaOrchestrator
			::Resolve(Result.P1ProvenanceState, SkillRuleSet);
	if (!Result.P1FormulaRegenerationResult.bSuccess)
	{
		SetFailure(
			Result,
			Result.P1FormulaRegenerationResult.ErrorCode
				== EMatchPlayCurrentAttackResolveThroughBallBehindDefenseP1FormulaErrorCode
					::FormulaPlanUnavailable
				? EError::P1FormulaUnavailable
				: EError::P1FormulaRegenerationFailed,
			Result.P1FormulaRegenerationResult.ErrorMessage,
			Result.P1FormulaRegenerationResult.InvalidField);
		return Result;
	}
	if (Result.P1FormulaRegenerationResult.FormulaExecutionResult.Decision
		!= EThroughBallBehindDefenseP1FormulaResolutionExecutionDecision
			::P2Required)
	{
		SetFailure(
			Result,
			EError::P1ResultDoesNotRequireP2,
			TEXT("Canonical BehindDefense P1 execution does not require P2."));
		return Result;
	}

	FMatchPlayState CandidateState = BeforeState;
	FMatchPlayCurrentAttackResolutionSession& CandidateSession =
		CandidateState.CurrentAttack.ResolutionSession;
	if (CandidateSession.PostRouteRollProgress.Phase
		== EPhase::PrimaryBranch)
	{
		CandidateSession.PostRouteRollProgress.Phase =
			EPhase::BehindDefenseP2;
	}
	else if (CandidateSession.PostRouteRollProgress.Phase
		!= EPhase::BehindDefenseP2)
	{
		SetFailure(
			Result,
			EError::UnsupportedPostRoutePhase,
			TEXT("BehindDefense P2 requires PrimaryBranch or BehindDefenseP2 progress."));
		return Result;
	}

	Result.AfterProgressResult =
		FMatchPlayCurrentAttackPostRouteRollProgressQuery::Evaluate(
			CandidateSession);
	if (!Result.AfterProgressResult.bIsCanonical)
	{
		SetFailure(
			Result,
			EError::InvalidPostRouteProgress,
			Result.AfterProgressResult.ErrorMessage);
		return Result;
	}
	if (!Result.AfterProgressResult.bContractComplete)
	{
		if (Result.AfterProgressResult.NextPurpose
			!= EPurpose::BehindDefenseP2Defense)
		{
			SetFailure(
				Result,
				EError::UnexpectedP2RollPurpose,
				TEXT("BehindDefense P2 progress requested an unexpected roll purpose."));
			return Result;
		}
		if (RollProvider == nullptr)
		{
			SetFailure(
				Result,
				EError::PostRouteRollProviderUnavailable,
				TEXT("BehindDefense P2 post-route roll provider is unavailable."));
			return Result;
		}

		Result.ProviderResult =
			RollProvider->RollD6(EPurpose::BehindDefenseP2Defense);
		++Result.ProviderCallCount;
		Result.ProviderValidationResult =
			FMatchPlayPostRouteRollProviderResultValidator::Validate(
				EPurpose::BehindDefenseP2Defense,
				Result.ProviderResult);
		if (!Result.ProviderValidationResult.bIsCanonical)
		{
			SetFailure(
				Result,
				MapProviderValidationError(
					Result.ProviderValidationResult.ErrorCode),
				Result.ProviderValidationResult.ErrorMessage);
			return Result;
		}

		FMatchPlayCurrentAttackPostRouteRollRecord Record;
		Record.Purpose = EPurpose::BehindDefenseP2Defense;
		Record.RawD6 = Result.ProviderResult.RawD6;
		CandidateSession.PostRouteRollProgress.RollRecords.Add(Record);
		Result.AfterProgressResult =
			FMatchPlayCurrentAttackPostRouteRollProgressQuery::Evaluate(
				CandidateSession);
		if (!Result.AfterProgressResult.bIsCanonical
			|| !Result.AfterProgressResult.bContractComplete)
		{
			SetFailure(
				Result,
				EError::InvalidPostRouteProgress,
				Result.AfterProgressResult.ErrorMessage);
			return Result;
		}
		Result.bResolvedNewRoll = true;
	}
	else
	{
		Result.bReplayedAcceptedRoll = true;
	}

	Result.SessionStateValidationResult =
		FMatchPlayCurrentAttackResolutionSessionStateValidator::Validate(
			CandidateState);
	if (!Result.SessionStateValidationResult.bIsCanonical)
	{
		SetFailure(
			Result,
			EError::InvalidCandidateState,
			Result.SessionStateValidationResult.ErrorMessage);
		return Result;
	}

	const TArray<FMatchPlayCurrentAttackPostRouteRollRecord>& Records =
		CandidateSession.PostRouteRollProgress.RollRecords;
	Result.QueryInput.P1ExecutionResult =
		Result.P1FormulaRegenerationResult.FormulaExecutionResult;
	Result.QueryInput.bHasP2DefenseD6 = true;
	Result.QueryInput.P2DefenseD6 = Records[2].RawD6;
	++Result.P2QueryExecutionCount;
	Result.QueryResult =
		FThroughBallBehindDefenseP2OutcomeQuery::Evaluate(Result.QueryInput);
	if (!Result.QueryResult.bSuccess)
	{
		SetFailure(
			Result,
			EError::P2OutcomeQueryFailed,
			Result.QueryResult.ErrorMessage,
			Result.QueryResult.InvalidField);
		return Result;
	}

	Result.AfterState = MoveTemp(CandidateState);
	Result.bSuccess = true;
	Result.ErrorCode = EError::None;
	Result.ErrorMessage.Empty();
	Result.InvalidField = NAME_None;
	return Result;
}
