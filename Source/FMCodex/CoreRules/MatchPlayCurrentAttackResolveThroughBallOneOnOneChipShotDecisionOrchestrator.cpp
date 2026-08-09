#include "MatchPlayCurrentAttackResolveThroughBallOneOnOneChipShotDecisionOrchestrator.h"

#include "MatchPlayCurrentAttackResolveThroughBallAntiOffsideDecisionOrchestrator.h"
#include "MatchPlayCurrentAttackResolveThroughBallBehindDefenseP2DecisionOrchestrator.h"

namespace MatchPlayCurrentAttackResolveThroughBallOneOnOneChipShotDecision
{
	using FResult =
		FMatchPlayCurrentAttackResolveThroughBallOneOnOneChipShotDecisionResult;
	using EError =
		EMatchPlayCurrentAttackResolveThroughBallOneOnOneChipShotDecisionErrorCode;
	using ESource = EMatchPlayThroughBallOneOnOneSource;
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

	bool BuildSourceProvenanceState(FResult& Result)
	{
		Result.SourceProvenanceState = Result.BeforeState;
		FMatchPlayCurrentAttackResolutionSession& Session =
			Result.SourceProvenanceState.CurrentAttack.ResolutionSession;
		FMatchPlayCurrentAttackPostRouteRollProgress& Progress =
			Session.PostRouteRollProgress;
		if (Result.Source == ESource::AntiOffside)
		{
			if (Progress.Phase == EPhase::OneOnOneChipShot)
			{
				Progress.Phase = EPhase::PrimaryBranch;
				Progress.RollRecords.SetNum(1);
			}
			else if (Progress.Phase != EPhase::PrimaryBranch)
			{
				SetFailure(
					Result,
					Result.BeforeProgressResult.bContractComplete
						? EError::UnsupportedSourcePhase
						: EError::IncompleteSourceProvenance,
					TEXT("AntiOffside OneOnOne requires complete PrimaryBranch provenance."));
				return false;
			}
		}
		else
		{
			if (Progress.Phase == EPhase::OneOnOneChipShot)
			{
				Progress.Phase = EPhase::BehindDefenseP2;
				Progress.RollRecords.SetNum(3);
			}
			else if (Progress.Phase != EPhase::BehindDefenseP2
				&& Progress.Phase != EPhase::PrimaryBranch)
			{
				SetFailure(
					Result,
					Result.BeforeProgressResult.bContractComplete
						? EError::UnsupportedSourcePhase
						: EError::IncompleteSourceProvenance,
					TEXT("BehindDefense OneOnOne requires complete P1 or P2 provenance."));
				return false;
			}
		}

		const auto Validation =
			FMatchPlayCurrentAttackResolutionSessionStateValidator::Validate(
				Result.SourceProvenanceState);
		if (!Validation.bIsCanonical)
		{
			SetFailure(
				Result,
				EError::InvalidSourceProvenanceState,
				Validation.ErrorMessage);
			return false;
		}
		Result.SourceProvenanceProgressResult =
			FMatchPlayCurrentAttackPostRouteRollProgressQuery::Evaluate(Session);
		if (!Result.SourceProvenanceProgressResult.bIsCanonical)
		{
			SetFailure(
				Result,
				EError::InvalidSourceProvenanceState,
				Result.SourceProvenanceProgressResult.ErrorMessage);
			return false;
		}
		if (!Result.SourceProvenanceProgressResult.bContractComplete)
		{
			SetFailure(
				Result,
				EError::IncompleteSourceProvenance,
				TEXT("OneOnOne source roll provenance is incomplete."));
			return false;
		}
		return true;
	}

	bool RegenerateSourceDecision(
		FResult& Result,
		const FSkillRuleSnapshotSet* SkillRuleSet)
	{
		++Result.SourceDecisionRegenerationCount;
		if (Result.Source == ESource::AntiOffside)
		{
			FMatchPlayCurrentAttackResolveThroughBallAntiOffsideDecisionRequest
				Request;
			Request.AttackSequence =
				Result.SourceProvenanceState.CurrentAttack.AttackSequence;
			Result.AntiOffsideRegenerationResult =
				FMatchPlayCurrentAttackResolveThroughBallAntiOffsideDecisionOrchestrator
					::Resolve(
						Result.SourceProvenanceState,
						Request,
						SkillRuleSet,
						nullptr);
			if (!Result.AntiOffsideRegenerationResult.bSuccess)
			{
				SetFailure(
					Result,
					EError::SourceDecisionRegenerationFailed,
					Result.AntiOffsideRegenerationResult.ErrorMessage);
				return false;
			}
			if (Result.AntiOffsideRegenerationResult.OutcomeResult.Decision
				!= EThroughBallAntiOffsideOutcomeDecision::OneOnOneRequired)
			{
				SetFailure(
					Result,
					EError::SourceDecisionDoesNotRequireOneOnOne,
					TEXT("Canonical AntiOffside decision does not require OneOnOne."));
				return false;
			}
			++Result.HandoffCreationCount;
			Result.HandoffCreationResult =
				FThroughBallOneOnOneHandoffCreator::CreateFromAntiOffside(
					Result.AntiOffsideRegenerationResult.OutcomeResult);
		}
		else
		{
			Result.BehindDefenseP2RegenerationResult =
				FMatchPlayCurrentAttackResolveThroughBallBehindDefenseP2DecisionOrchestrator
					::Resolve(
						Result.SourceProvenanceState,
						SkillRuleSet,
						nullptr);
			if (!Result.BehindDefenseP2RegenerationResult.bSuccess)
			{
				SetFailure(
					Result,
					EError::SourceDecisionRegenerationFailed,
					Result.BehindDefenseP2RegenerationResult.ErrorMessage,
					Result.BehindDefenseP2RegenerationResult.InvalidField);
				return false;
			}
			if (Result.BehindDefenseP2RegenerationResult.QueryResult.Decision
				!= EThroughBallBehindDefenseP2OutcomeDecision::OneOnOneRequired)
			{
				SetFailure(
					Result,
					EError::SourceDecisionDoesNotRequireOneOnOne,
					TEXT("Canonical BehindDefense P2 decision does not require OneOnOne."));
				return false;
			}
			++Result.HandoffCreationCount;
			Result.HandoffCreationResult =
				FThroughBallOneOnOneHandoffCreator::CreateFromBehindDefenseP2(
					Result.BehindDefenseP2RegenerationResult.QueryResult);
		}

		if (!Result.HandoffCreationResult.bSuccess
			|| !Result.HandoffCreationResult.bHasHandoff)
		{
			SetFailure(
				Result,
				EError::HandoffCreationFailed,
				Result.HandoffCreationResult.ErrorMessage,
				Result.HandoffCreationResult.InvalidField);
			return false;
		}
		return true;
	}
} // namespace MatchPlayCurrentAttackResolveThroughBallOneOnOneChipShotDecision

FMatchPlayCurrentAttackResolveThroughBallOneOnOneChipShotDecisionResult
FMatchPlayCurrentAttackResolveThroughBallOneOnOneChipShotDecisionOrchestrator
	::Resolve(
		const FMatchPlayState& BeforeState,
		const FSkillRuleSnapshotSet* SkillRuleSet,
		IMatchPlayPostRouteRollProvider* RollProvider)
{
	using namespace
		MatchPlayCurrentAttackResolveThroughBallOneOnOneChipShotDecision;

	FResult Result;
	Result.BeforeState = BeforeState;
	Result.AfterState = BeforeState;
	if (!BeforeState.RuntimeState.bIsInitialized)
	{
		SetFailure(
			Result,
			EError::MatchPlayStateNotInitialized,
			TEXT("OneOnOne ChipShot requires initialized MatchPlay State."));
		return Result;
	}
	if (!BeforeState.bHasCurrentAttack)
	{
		SetFailure(
			Result,
			EError::NoCurrentAttack,
			TEXT("OneOnOne ChipShot requires an active CurrentAttack."));
		return Result;
	}
	if (!BeforeState.CurrentAttack.bHasResolutionSession)
	{
		SetFailure(
			Result,
			EError::MissingResolutionSession,
			TEXT("OneOnOne ChipShot requires a Resolution Session."));
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
			TEXT("OneOnOne ChipShot requires RouteResolved."));
		return Result;
	}
	if (!BeforeSession.bHasActualBranch
		|| BeforeSession.ActualBranch.ActionType != ESkillRuleType::ThroughBall)
	{
		SetFailure(
			Result,
			EError::UnsupportedThroughBallBranch,
			TEXT("OneOnOne ChipShot supports only ThroughBall continuations."));
		return Result;
	}
	if (BeforeSession.ActualBranch.ThroughBall
		== EMatchPlayThroughBallActualBranch::AntiOffside)
	{
		Result.Source = ESource::AntiOffside;
	}
	else if (BeforeSession.ActualBranch.ThroughBall
		== EMatchPlayThroughBallActualBranch::BehindDefense)
	{
		Result.Source = ESource::BehindDefenseP2;
	}
	else
	{
		SetFailure(
			Result,
			EError::UnsupportedThroughBallBranch,
			TEXT("This ThroughBall branch has no supported OneOnOne provenance."));
		return Result;
	}
	if (BeforeSession.ThroughBallOneOnOneShotChoice
		== EMatchPlayThroughBallOneOnOneShotChoice::None)
	{
		SetFailure(
			Result,
			EError::OneOnOneShotChoiceNotSelected,
			TEXT("OneOnOne ChipShot requires an accepted ChipShot choice."));
		return Result;
	}
	if (BeforeSession.ThroughBallOneOnOneShotChoice
		!= EMatchPlayThroughBallOneOnOneShotChoice::ChipShot)
	{
		SetFailure(
			Result,
			EError::OneOnOneShotChoiceDoesNotPermitChipShot,
			TEXT("The accepted OneOnOne Shot Choice does not permit ChipShot resolution."));
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
	if (!BuildSourceProvenanceState(Result)
		|| !RegenerateSourceDecision(Result, SkillRuleSet))
	{
		return Result;
	}

	FMatchPlayState CandidateState = BeforeState;
	FMatchPlayCurrentAttackResolutionSession& CandidateSession =
		CandidateState.CurrentAttack.ResolutionSession;
	FMatchPlayCurrentAttackPostRouteRollProgress& CandidateProgress =
		CandidateSession.PostRouteRollProgress;
	const EPhase ExpectedSourcePhase = Result.Source == ESource::AntiOffside
		? EPhase::PrimaryBranch
		: EPhase::BehindDefenseP2;
	if (CandidateProgress.Phase == ExpectedSourcePhase)
	{
		CandidateProgress.Phase = EPhase::OneOnOneChipShot;
	}
	else if (CandidateProgress.Phase != EPhase::OneOnOneChipShot)
	{
		SetFailure(
			Result,
			EError::UnsupportedSourcePhase,
			TEXT("OneOnOne ChipShot source phase is unsupported."));
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
			!= EPurpose::OneOnOneChipShotAttack)
		{
			SetFailure(
				Result,
				EError::UnexpectedChipShotRollPurpose,
				TEXT("OneOnOne progress requested an unexpected roll purpose."));
			return Result;
		}
		if (RollProvider == nullptr)
		{
			SetFailure(
				Result,
				EError::PostRouteRollProviderUnavailable,
				TEXT("OneOnOne ChipShot roll provider is unavailable."));
			return Result;
		}

		Result.ProviderResult =
			RollProvider->RollD6(EPurpose::OneOnOneChipShotAttack);
		++Result.ProviderCallCount;
		Result.ProviderValidationResult =
			FMatchPlayPostRouteRollProviderResultValidator::Validate(
				EPurpose::OneOnOneChipShotAttack,
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
		Record.Purpose = EPurpose::OneOnOneChipShotAttack;
		Record.RawD6 = Result.ProviderResult.RawD6;
		CandidateProgress.RollRecords.Add(Record);
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
	if (CandidateSession.AttackSequence > MAX_int32)
	{
		SetFailure(
			Result,
			EError::UnrepresentableTurnIndex,
			TEXT("OneOnOne ChipShot TurnIndex cannot represent AttackSequence."));
		return Result;
	}

	const uint64 Sequence =
		static_cast<uint64>(CandidateSession.AttackSequence);
	Result.QueryInput.HandoffCreationResult = Result.HandoffCreationResult;
	Result.QueryInput.bHasChipShotAttackD6 = true;
	Result.QueryInput.ChipShotAttackD6 =
		CandidateProgress.RollRecords.Last().RawD6;
	Result.QueryInput.LogId = FGuid(
		0x54423131,
		static_cast<uint32>(Sequence >> 32),
		static_cast<uint32>(Sequence),
		0x43484950);
	Result.QueryInput.TurnIndex =
		static_cast<int32>(CandidateSession.AttackSequence - 1);
	++Result.ChipShotQueryExecutionCount;
	Result.QueryResult =
		FThroughBallOneOnOneChipShotOutcomeQuery::Evaluate(Result.QueryInput);
	if (!Result.QueryResult.bSuccess)
	{
		SetFailure(
			Result,
			EError::ChipShotOutcomeQueryFailed,
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
