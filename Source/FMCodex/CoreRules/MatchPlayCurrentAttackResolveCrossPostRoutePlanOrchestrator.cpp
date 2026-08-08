#include "MatchPlayCurrentAttackResolveCrossPostRoutePlanOrchestrator.h"

#include "MatchPlayCardSnapshotAuthority.h"

namespace MatchPlayCurrentAttackResolveCrossPostRoutePlan
{
	using EError =
		EMatchPlayCurrentAttackResolveCrossPostRoutePlanErrorCode;
	using EPurpose = EMatchPlayCurrentAttackPostRouteRollPurpose;
	using EPhase = EMatchPlayCurrentAttackPostRouteRollPhase;
	using FResult =
		FMatchPlayCurrentAttackResolveCrossPostRoutePlanResult;

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

	bool AddBoundParticipantSnapshot(
		const FMatchPlayState& State,
		const FMatchPlayCurrentAttackResolutionSessionParticipant&
			Participant,
		FPlayerCardRuleSnapshotSet& OutSnapshots,
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

		FPlayerCardRuleSnapshot Snapshot = QueryResult.Snapshot;
		Snapshot.Attributes = CopyAttributes(Participant.Values);
		OutSnapshots.Cards.Add(MoveTemp(Snapshot));
		return true;
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

	FName MakeCanonicalPlayerId(const FName CardId)
	{
		// MatchPlay currently binds participant identity by CardId. Project the
		// same immutable identity into the standalone query's PlayerId slot.
		return CardId;
	}

	bool AddGoalkeeperSnapshot(
		const FMatchPlayState& State,
		const EInitialTurnOrderPlayer DefendingSide,
		const FName GoalkeeperCardId,
		FPlayerCardRuleSnapshotSet& OutSnapshots,
		FString& OutErrorMessage)
	{
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
		OutSnapshots.Cards.Add(QueryResult.Snapshot);
		return true;
	}

	bool BuildCrossQueryInput(
		const FMatchPlayState& State,
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
				EError::CrossInputAdaptationFailed,
				TEXT("Cross query adaptation requires exactly two complete primary post-route rolls."));
			return false;
		}

		FCrossPlanQueryInput& Input = Result.QueryInput;
		Input.SkillId = Bundle.Binding.SkillId;
		if (Session.ActualBranch.Cross == EMatchPlayCrossActualBranch::High)
		{
			Input.ActualCrossType = ECrossPlanActualType::High;
		}
		else if (Session.ActualBranch.Cross
			== EMatchPlayCrossActualBranch::Low)
		{
			Input.ActualCrossType = ECrossPlanActualType::Low;
		}
		else
		{
			SetFailure(
				Result,
				EError::CrossInputAdaptationFailed,
				TEXT("Cross query adaptation requires Cross High or Low."));
			return false;
		}

		Input.CarrierCardId = Bundle.Carrier.CardId;
		Input.CarrierPlayerId = MakeCanonicalPlayerId(Bundle.Carrier.CardId);
		Input.RunnerCardId = Bundle.Runner.CardId;
		Input.RunnerPlayerId = MakeCanonicalPlayerId(Bundle.Runner.CardId);
		Input.MarkerCardId = Bundle.Marker.CardId;
		Input.MarkerPlayerId = MakeCanonicalPlayerId(Bundle.Marker.CardId);
		Input.bHasHelper = Bundle.bHasHelper;
		Input.HelperCardId = Bundle.bHasHelper
			? Bundle.Helper.CardId
			: NAME_None;
		Input.HelperPlayerId = MakeCanonicalPlayerId(Input.HelperCardId);

		Input.bUseGoalkeeper =
			CurrentAttack.bCurrentDefenseGoalkeeperActivated;
		if (Input.bUseGoalkeeper)
		{
			Input.GoalkeeperCardId = GetGoalkeeperCardId(
				State,
				Bundle.CurrentDefendingPlayer);
			Input.GoalkeeperPlayerId =
				MakeCanonicalPlayerId(Input.GoalkeeperCardId);
			if (Input.GoalkeeperCardId.IsNone())
			{
				SetFailure(
					Result,
					EError::CrossInputAdaptationFailed,
					TEXT("An activated defending goalkeeper requires an authoritative goalkeeper CardId."));
				return false;
			}
		}

		Input.CurrentActionPoint = CurrentAttack.ActionPoint;
		Input.AttackD6 = Records[0].RawD6;
		Input.DefenseD6 = Records[1].RawD6;
		if (Session.AttackSequence > MAX_int32)
		{
			SetFailure(
				Result,
				EError::CrossInputAdaptationFailed,
				TEXT("Cross query TurnIndex cannot represent the authoritative AttackSequence."));
			return false;
		}
		const uint64 Sequence = static_cast<uint64>(Session.AttackSequence);
		Input.LogId = FGuid(
			0x43524F53,
			static_cast<uint32>(Sequence >> 32),
			static_cast<uint32>(Sequence),
			0x504C414E);
		Input.TurnIndex = static_cast<int32>(Session.AttackSequence - 1);

		FString SnapshotError;
		if (!AddBoundParticipantSnapshot(
				State,
				Bundle.Carrier,
				Result.ScopedPlayerCardSnapshots,
				SnapshotError)
			|| !AddBoundParticipantSnapshot(
				State,
				Bundle.Runner,
				Result.ScopedPlayerCardSnapshots,
				SnapshotError)
			|| !AddBoundParticipantSnapshot(
				State,
				Bundle.Marker,
				Result.ScopedPlayerCardSnapshots,
				SnapshotError)
			|| (Bundle.bHasHelper
				&& !AddBoundParticipantSnapshot(
					State,
					Bundle.Helper,
					Result.ScopedPlayerCardSnapshots,
					SnapshotError))
			|| (Input.bUseGoalkeeper
				&& !AddGoalkeeperSnapshot(
					State,
					Bundle.CurrentDefendingPlayer,
					Input.GoalkeeperCardId,
					Result.ScopedPlayerCardSnapshots,
					SnapshotError)))
		{
			SetFailure(
				Result,
				EError::ParticipantSnapshotUnavailable,
				FString::Printf(
					TEXT("Cross participant snapshot adaptation failed: %s"),
					*SnapshotError));
			return false;
		}

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

FMatchPlayCurrentAttackResolveCrossPostRoutePlanResult
FMatchPlayCurrentAttackResolveCrossPostRoutePlanOrchestrator::Resolve(
	const FMatchPlayState& BeforeState,
	const FMatchPlayCurrentAttackResolveCrossPostRoutePlanRequest& Request,
	const FSkillRuleSnapshotSet* SkillRuleSet,
	IMatchPlayPostRouteRollProvider* RollProvider)
{
	using namespace MatchPlayCurrentAttackResolveCrossPostRoutePlan;

	FResult Result;
	Result.Request = Request;
	Result.BeforeState = BeforeState;
	Result.AfterState = BeforeState;
	if (!BeforeState.RuntimeState.bIsInitialized)
	{
		SetFailure(
			Result,
			EError::MatchPlayStateNotInitialized,
			TEXT("Cross post-route plan resolution requires initialized MatchPlay State."));
		return Result;
	}
	if (!BeforeState.bHasCurrentAttack)
	{
		SetFailure(
			Result,
			EError::NoCurrentAttack,
			TEXT("Cross post-route plan resolution requires an active CurrentAttack."));
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
			TEXT("Cross post-route plan resolution requires a Resolution Session."));
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
			TEXT("Cross post-route plan resolution requires RouteResolved."));
		return Result;
	}
	if (!BeforeSession.bHasActualBranch
		|| BeforeSession.ActualBranch.ActionType != ESkillRuleType::Cross
		|| (BeforeSession.ActualBranch.Cross
				!= EMatchPlayCrossActualBranch::High
			&& BeforeSession.ActualBranch.Cross
				!= EMatchPlayCrossActualBranch::Low))
	{
		SetFailure(
			Result,
			EError::NotCrossBranch,
			TEXT("This operation supports only resolved Cross High or Low branches."));
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
			TEXT("Cross post-route plan resolution requires primary-branch roll progress."));
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
			TEXT("Cross post-route roll provider is unavailable."));
		return Result;
	}
	if (SkillRuleSet == nullptr)
	{
		SetFailure(
			Result,
			EError::SkillRuleSetUnavailable,
			TEXT("Cross plan resolution requires an authoritative SkillRuleSet dependency."));
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
	if (!BuildCrossQueryInput(CandidateState, Result))
	{
		return Result;
	}

	Result.PlanResult = FCrossPlanQuery::BuildPlan(
		Result.ScopedPlayerCardSnapshots,
		*SkillRuleSet,
		Result.QueryInput);
	if (!Result.PlanResult.bSuccess)
	{
		SetFailure(
			Result,
			EError::CrossPlanQueryFailed,
			Result.PlanResult.ErrorMessage);
		return Result;
	}

	Result.AfterState = MoveTemp(CandidateState);
	Result.bResolvedNewRolls = Result.ProviderCallCount > 0;
	Result.bReplayedCompleteRolls = Result.ProviderCallCount == 0;
	Result.bSuccess = true;
	return Result;
}
