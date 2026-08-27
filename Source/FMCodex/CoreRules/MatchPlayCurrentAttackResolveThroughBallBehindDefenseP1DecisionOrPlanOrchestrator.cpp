#include "MatchPlayCurrentAttackResolveThroughBallBehindDefenseP1DecisionOrPlanOrchestrator.h"

#include "MatchPlayCardSnapshotAuthority.h"

namespace MatchPlayCurrentAttackResolveThroughBallBehindDefenseP1DecisionOrPlan
{
	using EError =
		EMatchPlayCurrentAttackResolveThroughBallBehindDefenseP1DecisionOrPlanErrorCode;
	using EMode =
		FMatchPlayCurrentAttackResolveThroughBallBehindDefenseP1DecisionOrPlanRequest
			::EMode;
	using EPurpose = EMatchPlayCurrentAttackPostRouteRollPurpose;
	using EPhase = EMatchPlayCurrentAttackPostRouteRollPhase;
	using FResult =
		FMatchPlayCurrentAttackResolveThroughBallBehindDefenseP1DecisionOrPlanResult;

	void SetFailure(
		FResult& Result,
		const EError ErrorCode,
		const FString& ErrorMessage)
	{
		Result.ErrorCode = ErrorCode;
		Result.ErrorMessage = ErrorMessage;
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
				State.CardSnapshotAuthority,
				Participant.Side,
				Participant.CardId);
		if (!QueryResult.bSuccess)
		{
			OutErrorMessage = QueryResult.ErrorMessage;
			return false;
		}
		OutSnapshot = QueryResult.Snapshot;
		OutSnapshot.Attributes = CopyAttributes(Participant.Values);
		return true;
	}

	bool BuildQueryInput(
		const FMatchPlayState& State,
		const FSkillRuleSnapshotSet& SkillRuleSet,
		FResult& Result)
	{
		const FMatchPlayCurrentAttackState& CurrentAttack = State.CurrentAttack;
		const FMatchPlayCurrentAttackResolutionSession& Session =
			CurrentAttack.ResolutionSession;
		const FMatchPlayCurrentAttackResolutionSessionBundle& Bundle =
			Session.Bundle;
		const TArray<FMatchPlayCurrentAttackPostRouteRollRecord>& Rolls =
			Session.PostRouteRollProgress.RollRecords;
		if (Session.PostRouteRollProgress.Phase != EPhase::PrimaryBranch
			|| (Rolls.Num() != 1 && Rolls.Num() != 2))
		{
			SetFailure(
				Result,
				EError::InputAdaptationFailed,
				TEXT("BehindDefense P1 requires a completed conditional primary-roll contract."));
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
		EligibilityInput.bIsRunnerInAttackingForwardArea = true;

		FString SnapshotError;
		if (!QueryBoundSnapshot(
				State,
				Bundle.Carrier,
				EligibilityInput.CarrierSnapshot,
				SnapshotError)
			|| !QueryBoundSnapshot(
				State,
				Bundle.Runner,
				EligibilityInput.RunnerSnapshot,
				SnapshotError)
			|| !QueryBoundSnapshot(
				State,
				Bundle.Marker,
				EligibilityInput.MarkerSnapshot,
				SnapshotError)
			|| (Bundle.bHasHelper
				&& !QueryBoundSnapshot(
					State,
					Bundle.Helper,
					EligibilityInput.HelperSnapshot,
					SnapshotError)))
		{
			SetFailure(
				Result,
				EError::ParticipantSnapshotUnavailable,
				SnapshotError);
			return false;
		}

		Result.ParticipantEligibilityResult =
			FThroughBallParticipantEligibilityQuery::Evaluate(
				SkillRuleSet,
				EligibilityInput);
		if (!Result.ParticipantEligibilityResult.bSuccess)
		{
			SetFailure(
				Result,
				EError::ParticipantEligibilityFailed,
				Result.ParticipantEligibilityResult.ErrorMessage);
			return false;
		}

		FThroughBallBehindDefenseP1PlanQueryInput& Input = Result.QueryInput;
		Input.ParticipantEligibilityResult = Result.ParticipantEligibilityResult;
		Input.SelectedBranch = EThroughBallSelectedBranch::BehindDefense;
		Input.bHasAttackD6 = true;
		Input.AttackD6 = Rolls[0].RawD6;
		Input.bHasDefenseD6 = Rolls.Num() == 2;
		if (Input.bHasDefenseD6)
		{
			Input.DefenseD6 = Rolls[1].RawD6;
		}

		const uint64 Sequence = static_cast<uint64>(Session.AttackSequence);
		Input.LogId = FGuid(
			0x42445031,
			static_cast<uint32>(Sequence >> 32),
			static_cast<uint32>(Sequence),
			0x504C414E);
		Input.TurnIndex = static_cast<int32>(Session.AttackSequence - 1);
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

FMatchPlayCurrentAttackResolveThroughBallBehindDefenseP1DecisionOrPlanResult
FMatchPlayCurrentAttackResolveThroughBallBehindDefenseP1DecisionOrPlanOrchestrator
	::Resolve(
		const FMatchPlayState& BeforeState,
		const
			FMatchPlayCurrentAttackResolveThroughBallBehindDefenseP1DecisionOrPlanRequest&
				Request,
		const FSkillRuleSnapshotSet* SkillRuleSet,
		IMatchPlayPostRouteRollProvider* RollProvider)
{
	using namespace
		MatchPlayCurrentAttackResolveThroughBallBehindDefenseP1DecisionOrPlan;

	FResult Result;
	Result.Request = Request;
	Result.BeforeState = BeforeState;
	Result.AfterState = BeforeState;
	if (!BeforeState.RuntimeState.bIsInitialized)
	{
		SetFailure(Result, EError::MatchPlayStateNotInitialized,
			TEXT("BehindDefense P1 requires initialized MatchPlay State."));
		return Result;
	}
	if (!BeforeState.bHasCurrentAttack)
	{
		SetFailure(Result, EError::NoCurrentAttack,
			TEXT("BehindDefense P1 requires an active CurrentAttack."));
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
			TEXT("BehindDefense P1 requires a Resolution Session."));
		return Result;
	}

	Result.SessionStateValidationResult =
		FMatchPlayCurrentAttackResolutionSessionStateValidator::Validate(
			BeforeState);
	if (!Result.SessionStateValidationResult.bIsCanonical)
	{
		SetFailure(Result, EError::InvalidResolutionSessionState,
			Result.SessionStateValidationResult.ErrorMessage);
		return Result;
	}

	const FMatchPlayCurrentAttackResolutionSession& BeforeSession =
		BeforeState.CurrentAttack.ResolutionSession;
	if (BeforeSession.Stage
		!= EMatchPlayCurrentAttackResolutionStage::RouteResolved)
	{
		SetFailure(Result, EError::RouteNotResolved,
			TEXT("BehindDefense P1 requires RouteResolved."));
		return Result;
	}
	if (!BeforeSession.bHasActualBranch
		|| BeforeSession.ActualBranch.ActionType != ESkillRuleType::ThroughBall
		|| BeforeSession.ActualBranch.ThroughBall
			!= EMatchPlayThroughBallActualBranch::BehindDefense)
	{
		SetFailure(Result, EError::NotThroughBallBehindDefenseBranch,
			TEXT("This operation supports only ThroughBall BehindDefense."));
		return Result;
	}

	const bool bExplicitRollStep = Request.Mode == EMode::ResolveAttackRoll
		|| Request.Mode == EMode::ResolveDefenseRoll;
	const bool bCompletedPlanRegeneration =
		Request.Mode == EMode::RegenerateCompletedPlan;

	FMatchPlayState CandidateState = BeforeState;
	FMatchPlayCurrentAttackResolutionSession& CandidateSession =
		CandidateState.CurrentAttack.ResolutionSession;
	if (CandidateSession.PostRouteRollProgress.Phase == EPhase::None)
	{
		CandidateSession.PostRouteRollProgress.Phase = EPhase::PrimaryBranch;
	}
	else if (CandidateSession.PostRouteRollProgress.Phase
		!= EPhase::PrimaryBranch)
	{
		SetFailure(Result, EError::InvalidPostRouteProgress,
			TEXT("BehindDefense P1 requires primary-branch roll progress."));
		return Result;
	}

	Result.ProgressResult =
		FMatchPlayCurrentAttackPostRouteRollProgressQuery::Evaluate(
			CandidateSession);
	if (!Result.ProgressResult.bIsCanonical)
	{
		SetFailure(Result, EError::InvalidPostRouteProgress,
			Result.ProgressResult.ErrorMessage);
		return Result;
	}
	if (bCompletedPlanRegeneration && !Result.ProgressResult.bContractComplete)
	{
		SetFailure(Result, EError::CompletedPlanRequired,
			TEXT("BehindDefense P1 regeneration requires an already-complete roll contract."));
		return Result;
	}
	if (bExplicitRollStep)
	{
		if (Request.RequestingSide != EInitialTurnOrderPlayer::PlayerA
			&& Request.RequestingSide != EInitialTurnOrderPlayer::PlayerB)
		{
			SetFailure(Result, EError::InvalidRequestingSide,
				TEXT("BehindDefense P1 roll commands require PlayerA or PlayerB as RequestingSide."));
			return Result;
		}
		const EPurpose RequestedPurpose = Request.Mode == EMode::ResolveAttackRoll
			? EPurpose::PrimaryAttack
			: EPurpose::PrimaryDefense;
		if (Result.ProgressResult.bContractComplete
			|| Result.ProgressResult.NextPurpose != RequestedPurpose)
		{
			SetFailure(Result, EError::WrongBehindDefenseRollStep,
				TEXT("The requested BehindDefense P1 roll is not the current authoritative step."));
			return Result;
		}
		const EInitialTurnOrderPlayer ExpectedSide = RequestedPurpose
			== EPurpose::PrimaryAttack
				? BeforeSession.Bundle.CurrentAttackingPlayer
				: BeforeSession.Bundle.CurrentDefendingPlayer;
		if (Request.RequestingSide != ExpectedSide)
		{
			SetFailure(Result, EError::WrongRequestingSide,
				TEXT("The requesting side does not own the current BehindDefense P1 roll."));
			return Result;
		}
	}
	if (!Result.ProgressResult.bContractComplete && RollProvider == nullptr)
	{
		SetFailure(Result, EError::PostRouteRollProviderUnavailable,
			TEXT("BehindDefense P1 post-route roll provider is unavailable."));
		return Result;
	}
	if (SkillRuleSet == nullptr)
	{
		SetFailure(Result, EError::SkillRuleSetUnavailable,
			TEXT("BehindDefense P1 requires authoritative SkillRuleSet."));
		return Result;
	}

	const int32 MaximumRollsThisCommand = bExplicitRollStep ? 1 : MAX_int32;
	while (!Result.ProgressResult.bContractComplete
		&& Result.ProviderCallCount < MaximumRollsThisCommand)
	{
		const EPurpose Purpose = Result.ProgressResult.NextPurpose;
		const FMatchPlayPostRouteRollProviderResult ProviderResult =
			RollProvider->RollD6(Purpose);
		++Result.ProviderCallCount;
		Result.ProviderResults.Add(ProviderResult);
		const FMatchPlayPostRouteRollProviderResultValidationResult Validation =
			FMatchPlayPostRouteRollProviderResultValidator::Validate(
				Purpose, ProviderResult);
		Result.ProviderValidationResults.Add(Validation);
		if (!Validation.bIsCanonical)
		{
			SetFailure(Result,
				MapProviderValidationError(Validation.ErrorCode),
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

	if (bExplicitRollStep && !Result.ProgressResult.bContractComplete)
	{
		Result.SessionStateValidationResult =
			FMatchPlayCurrentAttackResolutionSessionStateValidator::Validate(
				CandidateState);
		if (!Result.SessionStateValidationResult.bIsCanonical)
		{
			SetFailure(Result, EError::InvalidPostRouteProgress,
				Result.SessionStateValidationResult.ErrorMessage);
			return Result;
		}
		Result.AfterState = MoveTemp(CandidateState);
		Result.bResolvedNewRolls = Result.ProviderCallCount > 0;
		Result.bSuccess = true;
		return Result;
	}

	Result.SessionStateValidationResult =
		FMatchPlayCurrentAttackResolutionSessionStateValidator::Validate(
			CandidateState);
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

	Result.P1PlanResult =
		FThroughBallBehindDefenseP1PlanQuery::Evaluate(Result.QueryInput);
	if (!Result.P1PlanResult.bSuccess)
	{
		SetFailure(Result, EError::P1PlanQueryFailed,
			Result.P1PlanResult.ErrorMessage);
		return Result;
	}

	Result.AfterState = MoveTemp(CandidateState);
	Result.bResolvedNewRolls = Result.ProviderCallCount > 0;
	Result.bReplayedCompleteRolls = !bExplicitRollStep
		&& Result.ProviderCallCount == 0;
	Result.bSuccess = true;
	return Result;
}
