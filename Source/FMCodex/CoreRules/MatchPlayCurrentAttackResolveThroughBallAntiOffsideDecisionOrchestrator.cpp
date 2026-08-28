#include "MatchPlayCurrentAttackResolveThroughBallAntiOffsideDecisionOrchestrator.h"

#include "MatchPlayCardSnapshotAuthority.h"

namespace MatchPlayCurrentAttackResolveThroughBallAntiOffsideDecision
{
	using EError =
		EMatchPlayCurrentAttackResolveThroughBallAntiOffsideDecisionErrorCode;
	using EMode =
		FMatchPlayCurrentAttackResolveThroughBallAntiOffsideDecisionRequest
			::EMode;
	using EPurpose = EMatchPlayCurrentAttackPostRouteRollPurpose;
	using EPhase = EMatchPlayCurrentAttackPostRouteRollPhase;
	using FResult =
		FMatchPlayCurrentAttackResolveThroughBallAntiOffsideDecisionResult;

	void SetFailure(FResult& Result, const EError ErrorCode,
		const FString& ErrorMessage)
	{
		Result.ErrorCode = ErrorCode;
		Result.ErrorMessage = ErrorMessage;
	}

	FPlayerAttributes CopyAttributes(
		const FMatchPlayBoundActionNormalizedParticipantValues& Values)
	{
		FPlayerAttributes Result;
		Result.Shooting = Values.Shooting;
		Result.Dribbling = Values.Dribbling;
		Result.Passing = Values.Passing;
		Result.OffBall = Values.OffBall;
		Result.Marking = Values.Marking;
		Result.Tackling = Values.Tackling;
		Result.Speed = Values.Speed;
		Result.Strength = Values.Strength;
		Result.Stamina = Values.Stamina;
		Result.LongShot = Values.LongShot;
		return Result;
	}

	bool QueryBoundSnapshot(
		const FMatchPlayState& State,
		const FMatchPlayCurrentAttackResolutionSessionParticipant& Participant,
		FPlayerCardRuleSnapshot& OutSnapshot,
		FString& OutErrorMessage)
	{
		const FMatchPlayCardSnapshotAuthorityQueryResult QueryResult =
			FMatchPlayCardSnapshotAuthorityQuery::FindByPlayerSideAndCardId(
				State.CardSnapshotAuthority, Participant.Side, Participant.CardId);
		if (!QueryResult.bSuccess)
		{
			OutErrorMessage = QueryResult.ErrorMessage;
			return false;
		}
		OutSnapshot = QueryResult.Snapshot;
		OutSnapshot.Attributes = CopyAttributes(Participant.Values);
		return true;
	}

	FName MakeOwnerId(const EInitialTurnOrderPlayer Side)
	{
		if (Side == EInitialTurnOrderPlayer::PlayerA)
		{
			return TEXT("PlayerA");
		}
		if (Side == EInitialTurnOrderPlayer::PlayerB)
		{
			return TEXT("PlayerB");
		}
		return NAME_None;
	}

	bool BuildQueryInput(
		const FMatchPlayState& State,
		const FSkillRuleSnapshotSet& SkillRules,
		FResult& Result)
	{
		const FMatchPlayCurrentAttackState& CurrentAttack = State.CurrentAttack;
		const FMatchPlayCurrentAttackResolutionSession& Session =
			CurrentAttack.ResolutionSession;
		const FMatchPlayCurrentAttackResolutionSessionBundle& Bundle =
			Session.Bundle;
		const TArray<FMatchPlayCurrentAttackPostRouteRollRecord>& Records =
			Session.PostRouteRollProgress.RollRecords;
		if (Records.Num() != 1
			|| Session.PostRouteRollProgress.Phase != EPhase::PrimaryBranch
			|| Session.InitialRouteRollRecords.Num() != 1)
		{
			SetFailure(Result, EError::InputAdaptationFailed,
				TEXT("AntiOffside adaptation requires one initial-route and one primary Attack roll."));
			return false;
		}

		FThroughBallBranchSelectionQueryInput BranchInput;
		BranchInput.bHasExternalSelectionD6 = true;
		BranchInput.ExternalSelectionD6 =
			Session.InitialRouteRollRecords[0].RawD6;
		Result.BranchSelectionResult =
			FThroughBallBranchSelectionQuery::Select(BranchInput);
		if (!Result.BranchSelectionResult.bSuccess
			|| Result.BranchSelectionResult.SelectedThroughBallBranch
				!= EThroughBallSelectedBranch::AntiOffside)
		{
			SetFailure(Result, EError::BranchSelectionFailed,
				Result.BranchSelectionResult.bSuccess
					? TEXT("Canonical Initial Route record does not select AntiOffside.")
					: Result.BranchSelectionResult.ErrorMessage);
			return false;
		}

		FThroughBallParticipantEligibilityQueryInput EligibilityInput;
		EligibilityInput.SelectedSkillId = Bundle.Binding.SkillId;
		EligibilityInput.CurrentActionPoint = CurrentAttack.ActionPoint;
		EligibilityInput.AttackingOwnerId =
			MakeOwnerId(Bundle.CurrentAttackingPlayer);
		EligibilityInput.DefendingOwnerId =
			MakeOwnerId(Bundle.CurrentDefendingPlayer);
		EligibilityInput.bHasHelper = Bundle.bHasHelper;
		// Runner selection was frozen only after canonical forward-area
		// eligibility succeeded; no mutable participant selection is repeated.
		EligibilityInput.bIsRunnerInAttackingForwardArea = true;
		FString SnapshotError;
		if (!QueryBoundSnapshot(State, Bundle.Carrier,
				EligibilityInput.CarrierSnapshot, SnapshotError)
			|| !QueryBoundSnapshot(State, Bundle.Runner,
				EligibilityInput.RunnerSnapshot, SnapshotError)
			|| !QueryBoundSnapshot(State, Bundle.Marker,
				EligibilityInput.MarkerSnapshot, SnapshotError)
			|| (Bundle.bHasHelper && !QueryBoundSnapshot(State, Bundle.Helper,
				EligibilityInput.HelperSnapshot, SnapshotError)))
		{
			SetFailure(Result, EError::ParticipantSnapshotUnavailable,
				FString::Printf(TEXT("AntiOffside participant snapshot adaptation failed: %s"),
					*SnapshotError));
			return false;
		}
		Result.ParticipantEligibilityResult =
			FThroughBallParticipantEligibilityQuery::Evaluate(
				SkillRules, EligibilityInput);
		if (!Result.ParticipantEligibilityResult.bSuccess)
		{
			SetFailure(Result, EError::ParticipantEligibilityFailed,
				Result.ParticipantEligibilityResult.ErrorMessage);
			return false;
		}
		Result.QueryInput.BranchSelectionResult = Result.BranchSelectionResult;
		Result.QueryInput.ParticipantEligibilityResult =
			Result.ParticipantEligibilityResult;
		Result.QueryInput.bHasAntiOffsideAttackD6 = true;
		Result.QueryInput.AntiOffsideAttackD6 = Records[0].RawD6;
		return true;
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
}

FMatchPlayCurrentAttackResolveThroughBallAntiOffsideDecisionResult
FMatchPlayCurrentAttackResolveThroughBallAntiOffsideDecisionOrchestrator::Resolve(
	const FMatchPlayState& BeforeState,
	const FMatchPlayCurrentAttackResolveThroughBallAntiOffsideDecisionRequest& Request,
	const FSkillRuleSnapshotSet* SkillRuleSet,
	IMatchPlayPostRouteRollProvider* RollProvider)
{
	using namespace MatchPlayCurrentAttackResolveThroughBallAntiOffsideDecision;
	FResult Result;
	Result.Request = Request;
	Result.BeforeState = BeforeState;
	Result.AfterState = BeforeState;
	if (!BeforeState.RuntimeState.bIsInitialized)
	{
		SetFailure(Result, EError::MatchPlayStateNotInitialized,
			TEXT("AntiOffside decision requires initialized MatchPlay State."));
		return Result;
	}
	if (!BeforeState.bHasCurrentAttack)
	{
		SetFailure(Result, EError::NoCurrentAttack,
			TEXT("AntiOffside decision requires an active CurrentAttack."));
		return Result;
	}
	if (BeforeState.CurrentAttack.AttackSequence <= 0)
	{
		SetFailure(Result, EError::InvalidCurrentAttackSequence,
			TEXT("CurrentAttack AttackSequence must be positive."));
		return Result;
	}
	if (Request.AttackSequence <= 0)
	{
		SetFailure(Result, EError::InvalidRequestedAttackSequence,
			TEXT("Requested AttackSequence must be positive."));
		return Result;
	}
	if (Request.AttackSequence != BeforeState.CurrentAttack.AttackSequence)
	{
		SetFailure(Result, EError::AttackSequenceMismatch,
			TEXT("Requested AttackSequence does not match CurrentAttack."));
		return Result;
	}
	if (!BeforeState.CurrentAttack.bHasResolutionSession)
	{
		SetFailure(Result, EError::MissingResolutionSession,
			TEXT("AntiOffside decision requires a Resolution Session."));
		return Result;
	}
	Result.SessionStateValidationResult =
		FMatchPlayCurrentAttackResolutionSessionStateValidator::Validate(BeforeState);
	if (!Result.SessionStateValidationResult.bIsCanonical)
	{
		SetFailure(Result, EError::InvalidResolutionSessionState,
			Result.SessionStateValidationResult.ErrorMessage);
		return Result;
	}
	const FMatchPlayCurrentAttackResolutionSession& BeforeSession =
		BeforeState.CurrentAttack.ResolutionSession;
	if (BeforeSession.Stage != EMatchPlayCurrentAttackResolutionStage::RouteResolved)
	{
		SetFailure(Result, EError::RouteNotResolved,
			TEXT("AntiOffside decision requires RouteResolved."));
		return Result;
	}
	if (!BeforeSession.bHasActualBranch
		|| BeforeSession.ActualBranch.ActionType != ESkillRuleType::ThroughBall
		|| BeforeSession.ActualBranch.ThroughBall
			!= EMatchPlayThroughBallActualBranch::AntiOffside)
	{
		SetFailure(Result, EError::NotThroughBallAntiOffsideBranch,
			TEXT("This operation supports only resolved ThroughBall AntiOffside."));
		return Result;
	}

	FMatchPlayState CandidateState = BeforeState;
	FMatchPlayCurrentAttackResolutionSession& CandidateSession =
		CandidateState.CurrentAttack.ResolutionSession;
	if (CandidateSession.PostRouteRollProgress.Phase == EPhase::None)
	{
		CandidateSession.PostRouteRollProgress.Phase = EPhase::PrimaryBranch;
	}
	else if (CandidateSession.PostRouteRollProgress.Phase != EPhase::PrimaryBranch)
	{
		SetFailure(Result, EError::InvalidPostRouteProgress,
			TEXT("AntiOffside decision requires primary-branch roll progress."));
		return Result;
	}
	Result.ProgressResult =
		FMatchPlayCurrentAttackPostRouteRollProgressQuery::Evaluate(CandidateSession);
	if (!Result.ProgressResult.bIsCanonical)
	{
		SetFailure(Result, EError::InvalidPostRouteProgress,
			Result.ProgressResult.ErrorMessage);
		return Result;
	}
	const bool bExplicitAttackRoll = Request.Mode == EMode::ResolveAttackRoll;
	if (Request.Mode == EMode::RegenerateCompletedDecision
		&& !Result.ProgressResult.bContractComplete)
	{
		SetFailure(Result, EError::CompletedDecisionRequired,
			TEXT("AntiOffside regeneration requires an already-complete roll contract."));
		return Result;
	}
	if (bExplicitAttackRoll)
	{
		if (Request.RequestingSide != EInitialTurnOrderPlayer::PlayerA
			&& Request.RequestingSide != EInitialTurnOrderPlayer::PlayerB)
		{
			SetFailure(Result, EError::InvalidRequestingSide,
				TEXT("AntiOffside roll requires PlayerA or PlayerB as RequestingSide."));
			return Result;
		}
		if (Result.ProgressResult.bContractComplete
			|| Result.ProgressResult.NextPurpose != EPurpose::PrimaryAttack)
		{
			SetFailure(Result, EError::WrongAntiOffsideRollStep,
				TEXT("The AntiOffside attack roll is not the current authoritative step."));
			return Result;
		}
		if (Request.RequestingSide
			!= BeforeSession.Bundle.CurrentAttackingPlayer)
		{
			SetFailure(Result, EError::WrongRequestingSide,
				TEXT("Only the current attacking side owns the AntiOffside roll."));
			return Result;
		}
	}
	if (!Result.ProgressResult.bContractComplete && RollProvider == nullptr)
	{
		SetFailure(Result, EError::PostRouteRollProviderUnavailable,
			TEXT("AntiOffside post-route roll provider is unavailable."));
		return Result;
	}
	if (SkillRuleSet == nullptr)
	{
		SetFailure(Result, EError::SkillRuleSetUnavailable,
			TEXT("AntiOffside decision requires authoritative SkillRuleSet."));
		return Result;
	}
	const int32 MaximumRollsThisCommand = bExplicitAttackRoll ? 1 : MAX_int32;
	while (!Result.ProgressResult.bContractComplete
		&& Result.ProviderCallCount < MaximumRollsThisCommand)
	{
		const EPurpose Purpose = Result.ProgressResult.NextPurpose;
		const FMatchPlayPostRouteRollProviderResult ProviderResult =
			RollProvider->RollD6(Purpose);
		++Result.ProviderCallCount;
		Result.ProviderResults.Add(ProviderResult);
		const FMatchPlayPostRouteRollProviderResultValidationResult Validation =
			FMatchPlayPostRouteRollProviderResultValidator::Validate(Purpose, ProviderResult);
		Result.ProviderValidationResults.Add(Validation);
		if (!Validation.bIsCanonical)
		{
			SetFailure(Result, MapProviderValidationError(Validation.ErrorCode),
				Validation.ErrorMessage);
			return Result;
		}
		FMatchPlayCurrentAttackPostRouteRollRecord Record;
		Record.Purpose = Purpose;
		Record.RawD6 = ProviderResult.RawD6;
		CandidateSession.PostRouteRollProgress.RollRecords.Add(Record);
		Result.ProgressResult =
			FMatchPlayCurrentAttackPostRouteRollProgressQuery::Evaluate(
				CandidateSession);
		if (!Result.ProgressResult.bIsCanonical)
		{
			SetFailure(Result, EError::InvalidPostRouteProgress,
				Result.ProgressResult.ErrorMessage);
			return Result;
		}
	}
	Result.SessionStateValidationResult =
		FMatchPlayCurrentAttackResolutionSessionStateValidator::Validate(CandidateState);
	if (!Result.SessionStateValidationResult.bIsCanonical)
	{
		SetFailure(Result, EError::InvalidPostRouteProgress,
			Result.SessionStateValidationResult.ErrorMessage);
		return Result;
	}
	if (!BuildQueryInput(CandidateState, *SkillRuleSet, Result))
	{
		return Result;
	}
	Result.OutcomeResult =
		FThroughBallAntiOffsideOutcomeQuery::Evaluate(Result.QueryInput);
	if (!Result.OutcomeResult.bSuccess)
	{
		SetFailure(Result, EError::AntiOffsideOutcomeQueryFailed,
			Result.OutcomeResult.ErrorMessage);
		return Result;
	}
	Result.AfterState = MoveTemp(CandidateState);
	Result.bResolvedNewRolls = Result.ProviderCallCount > 0;
	Result.bReplayedCompleteRolls = Result.ProviderCallCount == 0;
	Result.bSuccess = true;
	return Result;
}
