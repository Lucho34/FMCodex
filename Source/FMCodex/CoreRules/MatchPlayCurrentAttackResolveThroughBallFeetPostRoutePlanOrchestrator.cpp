#include "MatchPlayCurrentAttackResolveThroughBallFeetPostRoutePlanOrchestrator.h"

#include "MatchPlayCardSnapshotAuthority.h"

namespace MatchPlayCurrentAttackResolveThroughBallFeetPostRoutePlan
{
	using EError =
		EMatchPlayCurrentAttackResolveThroughBallFeetPostRoutePlanErrorCode;
	using EPurpose = EMatchPlayCurrentAttackPostRouteRollPurpose;
	using EPhase = EMatchPlayCurrentAttackPostRouteRollPhase;
	using FResult =
		FMatchPlayCurrentAttackResolveThroughBallFeetPostRoutePlanResult;

	void SetFailure(
		FResult& Result,
		const EError ErrorCode,
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
		const FMatchPlayCurrentAttackResolutionSessionParticipant&
			Participant,
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

	FName GetGoalkeeperCardId(
		const FMatchPlayState& State,
		const EInitialTurnOrderPlayer DefendingSide)
	{
		if (DefendingSide == EInitialTurnOrderPlayer::PlayerA)
		{
			return FName(*State.RuntimeState.PlayerAState.GoalkeeperCardId);
		}
		if (DefendingSide == EInitialTurnOrderPlayer::PlayerB)
		{
			return FName(*State.RuntimeState.PlayerBState.GoalkeeperCardId);
		}
		return NAME_None;
	}

	bool QueryGoalkeeperSnapshot(
		const FMatchPlayState& State,
		const EInitialTurnOrderPlayer DefendingSide,
		FPlayerCardRuleSnapshot& OutSnapshot,
		FString& OutErrorMessage)
	{
		const FName GoalkeeperCardId =
			GetGoalkeeperCardId(State, DefendingSide);
		const FMatchPlayCardSnapshotAuthorityQueryResult QueryResult =
			FMatchPlayCardSnapshotAuthorityQuery::FindByPlayerSideAndCardId(
				State.CardSnapshotAuthority,
				DefendingSide,
				GoalkeeperCardId);
		if (!QueryResult.bSuccess)
		{
			OutErrorMessage = QueryResult.ErrorMessage;
			return false;
		}
		OutSnapshot = QueryResult.Snapshot;
		return true;
	}

	bool BuildFeetQueryInput(
		const FMatchPlayState& State,
		const FSkillRuleSnapshotSet& SkillRuleSet,
		FResult& Result)
	{
		const FMatchPlayCurrentAttackState& CurrentAttack =
			State.CurrentAttack;
		const FMatchPlayCurrentAttackResolutionSession& Session =
			CurrentAttack.ResolutionSession;
		const FMatchPlayCurrentAttackResolutionSessionBundle& Bundle =
			Session.Bundle;
		const TArray<FMatchPlayCurrentAttackPostRouteRollRecord>& Records =
			Session.PostRouteRollProgress.RollRecords;
		if (Records.Num() != 2
			|| Session.PostRouteRollProgress.Phase != EPhase::PrimaryBranch)
		{
			SetFailure(
				Result,
				EError::FeetInputAdaptationFailed,
				TEXT("ThroughBall Feet adaptation requires exactly two complete primary post-route rolls."));
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
		// Runner selection was already frozen only after canonical forward-area
		// eligibility succeeded. No mutable participant selection is repeated.
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
				FString::Printf(
					TEXT("ThroughBall Feet participant snapshot adaptation failed: %s"),
					*SnapshotError));
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

		FThroughBallFeetPlanQueryInput& Input = Result.QueryInput;
		Input.ParticipantEligibilityResult =
			Result.ParticipantEligibilityResult;
		Input.AttackD6 = Records[0].RawD6;
		Input.DefenseD6 = Records[1].RawD6;
		Input.bHasActiveGoalkeeper =
			CurrentAttack.bCurrentDefenseGoalkeeperActivated;
		if (Input.bHasActiveGoalkeeper
			&& !QueryGoalkeeperSnapshot(
				State,
				Bundle.CurrentDefendingPlayer,
				Input.ActiveGoalkeeperSnapshot,
				SnapshotError))
		{
			SetFailure(
				Result,
				EError::ParticipantSnapshotUnavailable,
				FString::Printf(
					TEXT("ThroughBall Feet goalkeeper snapshot adaptation failed: %s"),
					*SnapshotError));
			return false;
		}

		if (Session.AttackSequence > MAX_int32)
		{
			SetFailure(
				Result,
				EError::FeetInputAdaptationFailed,
				TEXT("ThroughBall Feet TurnIndex cannot represent the authoritative AttackSequence."));
			return false;
		}
		const uint64 Sequence = static_cast<uint64>(Session.AttackSequence);
		Input.LogId = FGuid(
			0x54424654,
			static_cast<uint32>(Sequence >> 32),
			static_cast<uint32>(Sequence),
			0x504C414E);
		Input.TurnIndex = static_cast<int32>(Session.AttackSequence - 1);
		return true;
	}

	EError MapProviderValidationError(
		const EMatchPlayPostRouteRollProviderResultValidationErrorCode
			ErrorCode)
	{
		return ErrorCode
			== EMatchPlayPostRouteRollProviderResultValidationErrorCode
				::ProviderFailure
			? EError::PostRouteRollProviderFailed
			: EError::MalformedPostRouteRollProviderResult;
	}
}

FMatchPlayCurrentAttackResolveThroughBallFeetPostRoutePlanResult
FMatchPlayCurrentAttackResolveThroughBallFeetPostRoutePlanOrchestrator
	::Resolve(
		const FMatchPlayState& BeforeState,
		const
			FMatchPlayCurrentAttackResolveThroughBallFeetPostRoutePlanRequest&
				Request,
		const FSkillRuleSnapshotSet* SkillRuleSet,
		IMatchPlayPostRouteRollProvider* RollProvider)
{
	using namespace
		MatchPlayCurrentAttackResolveThroughBallFeetPostRoutePlan;

	FResult Result;
	Result.Request = Request;
	Result.BeforeState = BeforeState;
	Result.AfterState = BeforeState;
	if (!BeforeState.RuntimeState.bIsInitialized)
	{
		SetFailure(
			Result,
			EError::MatchPlayStateNotInitialized,
			TEXT("ThroughBall Feet post-route plan resolution requires initialized MatchPlay State."));
		return Result;
	}
	if (!BeforeState.bHasCurrentAttack)
	{
		SetFailure(
			Result,
			EError::NoCurrentAttack,
			TEXT("ThroughBall Feet post-route plan resolution requires an active CurrentAttack."));
		return Result;
	}
	if (BeforeState.CurrentAttack.AttackSequence <= 0)
	{
		SetFailure(
			Result,
			EError::InvalidCurrentAttackSequence,
			TEXT("CurrentAttack AttackSequence must be positive."));
		return Result;
	}
	if (Request.AttackSequence <= 0)
	{
		SetFailure(
			Result,
			EError::InvalidRequestedAttackSequence,
			TEXT("Requested AttackSequence must be positive."));
		return Result;
	}
	if (Request.AttackSequence
		!= BeforeState.CurrentAttack.AttackSequence)
	{
		SetFailure(
			Result,
			EError::AttackSequenceMismatch,
			TEXT("Requested AttackSequence does not match CurrentAttack."));
		return Result;
	}
	if (!BeforeState.CurrentAttack.bHasResolutionSession)
	{
		SetFailure(
			Result,
			EError::MissingResolutionSession,
			TEXT("ThroughBall Feet post-route plan resolution requires a Resolution Session."));
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
			TEXT("ThroughBall Feet post-route plan resolution requires RouteResolved."));
		return Result;
	}
	if (!BeforeSession.bHasActualBranch
		|| BeforeSession.ActualBranch.ActionType
			!= ESkillRuleType::ThroughBall
		|| BeforeSession.ActualBranch.ThroughBall
			!= EMatchPlayThroughBallActualBranch::Feet)
	{
		SetFailure(
			Result,
			EError::NotThroughBallFeetBranch,
			TEXT("This operation supports only the resolved ThroughBall Feet branch."));
		return Result;
	}

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
		SetFailure(
			Result,
			EError::InvalidPostRouteProgress,
			TEXT("ThroughBall Feet plan resolution requires primary-branch roll progress."));
		return Result;
	}

	Result.ProgressResult =
		FMatchPlayCurrentAttackPostRouteRollProgressQuery::Evaluate(
			CandidateSession);
	if (!Result.ProgressResult.bIsCanonical)
	{
		SetFailure(
			Result,
			EError::InvalidPostRouteProgress,
			Result.ProgressResult.ErrorMessage);
		return Result;
	}
	if (!Result.ProgressResult.bContractComplete
		&& RollProvider == nullptr)
	{
		SetFailure(
			Result,
			EError::PostRouteRollProviderUnavailable,
			TEXT("ThroughBall Feet post-route roll provider is unavailable."));
		return Result;
	}
	if (SkillRuleSet == nullptr)
	{
		SetFailure(
			Result,
			EError::SkillRuleSetUnavailable,
			TEXT("ThroughBall Feet plan resolution requires an authoritative SkillRuleSet dependency."));
		return Result;
	}

	while (!Result.ProgressResult.bContractComplete)
	{
		const EPurpose Purpose = Result.ProgressResult.NextPurpose;
		const FMatchPlayPostRouteRollProviderResult ProviderResult =
			RollProvider->RollD6(Purpose);
		++Result.ProviderCallCount;
		Result.ProviderResults.Add(ProviderResult);
		const FMatchPlayPostRouteRollProviderResultValidationResult
			Validation =
				FMatchPlayPostRouteRollProviderResultValidator::Validate(
					Purpose,
					ProviderResult);
		Result.ProviderValidationResults.Add(Validation);
		if (!Validation.bIsCanonical)
		{
			SetFailure(
				Result,
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
			SetFailure(
				Result,
				EError::InvalidPostRouteProgress,
				Result.ProgressResult.ErrorMessage);
			return Result;
		}
	}

	Result.SessionStateValidationResult =
		FMatchPlayCurrentAttackResolutionSessionStateValidator::Validate(
			CandidateState);
	if (!Result.SessionStateValidationResult.bIsCanonical)
	{
		SetFailure(
			Result,
			EError::InvalidPostRouteProgress,
			Result.SessionStateValidationResult.ErrorMessage);
		return Result;
	}
	if (!BuildFeetQueryInput(CandidateState, *SkillRuleSet, Result))
	{
		return Result;
	}

	Result.PlanResult =
		FThroughBallFeetPlanQuery::Evaluate(Result.QueryInput);
	if (!Result.PlanResult.bSuccess)
	{
		SetFailure(
			Result,
			EError::FeetPlanQueryFailed,
			Result.PlanResult.ErrorMessage);
		return Result;
	}

	Result.AfterState = MoveTemp(CandidateState);
	Result.bResolvedNewRolls = Result.ProviderCallCount > 0;
	Result.bReplayedCompleteRolls = Result.ProviderCallCount == 0;
	Result.bSuccess = true;
	return Result;
}
