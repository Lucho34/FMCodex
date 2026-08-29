#include "MatchPlayCurrentAttackResolveDirectShotPostRouteDecisionOrPlanOrchestrator.h"

#include "MatchPlayCardSnapshotAuthority.h"

namespace MatchPlayCurrentAttackResolveDirectShotPostRouteDecisionOrPlan
{
	using EError =
		EMatchPlayCurrentAttackResolveDirectShotPostRouteDecisionOrPlanErrorCode;
	using EMode =
		FMatchPlayCurrentAttackResolveDirectShotPostRouteDecisionOrPlanRequest
			::EMode;
	using EPurpose = EMatchPlayCurrentAttackPostRouteRollPurpose;
	using EPhase = EMatchPlayCurrentAttackPostRouteRollPhase;
	using FResult =
		FMatchPlayCurrentAttackResolveDirectShotPostRouteDecisionOrPlanResult;

	void SetFailure(
		FResult& Result,
		const EError ErrorCode,
		const FString& ErrorMessage)
	{
		Result.ErrorCode = ErrorCode;
		Result.ErrorMessage = ErrorMessage;
	}

	bool IsDirectShotBranch(
		const FMatchPlayCurrentAttackActualBranch& Branch)
	{
		return (Branch.ActionType == ESkillRuleType::LongShot
				&& Branch.LongShot
					== EMatchPlayLongShotActualBranch::DirectShot)
			|| (Branch.ActionType == ESkillRuleType::CutInsideShot
				&& Branch.CutInsideShot
					== EMatchPlayCutInsideShotActualBranch::DirectShot);
	}

	FName MakePlayerId(const EInitialTurnOrderPlayer Side)
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

	bool AddBoundSnapshot(
		const FMatchPlayState& State,
		const FMatchPlayCurrentAttackResolutionSessionParticipant& Participant,
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

	template <typename TInput>
	bool BuildInput(
		const FMatchPlayState& State,
		TInput& OutInput,
		FPlayerCardRuleSnapshotSet& OutSnapshots,
		EError& OutErrorCode,
		FString& OutErrorMessage)
	{
		const FMatchPlayCurrentAttackState& CurrentAttack = State.CurrentAttack;
		const FMatchPlayCurrentAttackResolutionSession& Session =
			CurrentAttack.ResolutionSession;
		const FMatchPlayCurrentAttackResolutionSessionBundle& Bundle =
			Session.Bundle;
		const TArray<FMatchPlayCurrentAttackPostRouteRollRecord>& Records =
			Session.PostRouteRollProgress.RollRecords;
		if (Session.PostRouteRollProgress.Phase != EPhase::PrimaryBranch
			|| (Records.Num() != 1 && Records.Num() != 2)
			|| Session.AttackSequence > MAX_int32)
		{
			OutErrorCode = EError::InputAdaptationFailed;
			OutErrorMessage =
				TEXT("DirectShot adaptation requires a completed conditional primary-roll contract and a representable TurnIndex.");
			return false;
		}
		if (!AddBoundSnapshot(
				State, Bundle.Carrier, OutSnapshots, OutErrorMessage)
			|| !AddBoundSnapshot(
				State, Bundle.Marker, OutSnapshots, OutErrorMessage))
		{
			OutErrorCode = EError::ParticipantSnapshotUnavailable;
			return false;
		}

		OutInput.SkillId = Bundle.Binding.SkillId;
		OutInput.AttackerCardId = Bundle.Carrier.CardId;
		OutInput.DefenderCardId = Bundle.Marker.CardId;
		OutInput.CurrentActionPoint = CurrentAttack.ActionPoint;
		OutInput.bHasExternalAttackD6 = true;
		OutInput.ExternalAttackD6 = Records[0].RawD6;
		OutInput.bHasExternalDefenseD6 = Records.Num() == 2;
		if (OutInput.bHasExternalDefenseD6)
		{
			OutInput.ExternalDefenseD6 = Records[1].RawD6;
		}
		const uint64 Sequence = static_cast<uint64>(Session.AttackSequence);
		OutInput.LogId = FGuid(
			0x44534854,
			static_cast<uint32>(Sequence >> 32),
			static_cast<uint32>(Sequence),
			0x504C414E);
		OutInput.TurnIndex = static_cast<int32>(Session.AttackSequence - 1);
		OutInput.AttackerPlayerId = MakePlayerId(Bundle.Carrier.Side);
		OutInput.DefenderPlayerId = MakePlayerId(Bundle.Marker.Side);
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

FMatchPlayCurrentAttackResolveDirectShotPostRouteDecisionOrPlanResult
FMatchPlayCurrentAttackResolveDirectShotPostRouteDecisionOrPlanOrchestrator
	::Resolve(
		const FMatchPlayState& BeforeState,
		const FMatchPlayCurrentAttackResolveDirectShotPostRouteDecisionOrPlanRequest&
			Request,
		const FSkillRuleSnapshotSet* SkillRuleSet,
		IMatchPlayPostRouteRollProvider* RollProvider)
{
	using namespace
		MatchPlayCurrentAttackResolveDirectShotPostRouteDecisionOrPlan;
	FResult Result;
	Result.Request = Request;
	Result.BeforeState = BeforeState;
	Result.AfterState = BeforeState;
	if (!BeforeState.RuntimeState.bIsInitialized)
	{
		SetFailure(Result, EError::MatchPlayStateNotInitialized,
			TEXT("DirectShot resolution requires initialized MatchPlay State."));
		return Result;
	}
	if (!BeforeState.bHasCurrentAttack)
	{
		SetFailure(Result, EError::NoCurrentAttack,
			TEXT("DirectShot resolution requires an active CurrentAttack."));
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
			TEXT("DirectShot resolution requires a Resolution Session."));
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
			TEXT("DirectShot resolution requires RouteResolved."));
		return Result;
	}
	if (!BeforeSession.bHasActualBranch
		|| !IsDirectShotBranch(BeforeSession.ActualBranch))
	{
		SetFailure(Result, EError::NotDirectShotBranch,
			TEXT("This operation supports only LongShot/CutInsideShot DirectShot."));
		return Result;
	}
	Result.ActionType = BeforeSession.ActualBranch.ActionType;

	const bool bExplicitLongShotRoll =
		Request.Mode == EMode::ResolveLongShotAttackRoll
		|| Request.Mode == EMode::ResolveLongShotDefenseRoll;
	const bool bExplicitCutInsideShotRoll =
		Request.Mode == EMode::ResolveCutInsideShotAttackRoll
		|| Request.Mode == EMode::ResolveCutInsideShotDefenseRoll;
	const bool bExplicitPlayerOwnedRoll =
		bExplicitLongShotRoll || bExplicitCutInsideShotRoll;
	const bool bCompletedPlanRegeneration =
		Request.Mode == EMode::RegenerateCompletedPlan;
	if (bExplicitLongShotRoll
		&& (BeforeSession.ActualBranch.ActionType != ESkillRuleType::LongShot
			|| BeforeSession.ActualBranch.LongShot
				!= EMatchPlayLongShotActualBranch::DirectShot))
	{
		SetFailure(Result, EError::NotLongShotDirectShotBranch,
			TEXT("Explicit LongShot Direct rolls require the LongShot DirectShot branch."));
		return Result;
	}
	if (bExplicitCutInsideShotRoll
		&& (BeforeSession.ActualBranch.ActionType
				!= ESkillRuleType::CutInsideShot
			|| BeforeSession.ActualBranch.CutInsideShot
				!= EMatchPlayCutInsideShotActualBranch::DirectShot))
	{
		SetFailure(Result, EError::NotCutInsideShotDirectShotBranch,
			TEXT("Explicit CutInsideShot Direct rolls require the CutInsideShot DirectShot branch."));
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
		SetFailure(Result, EError::InvalidPostRouteProgress,
			TEXT("DirectShot resolution requires primary-branch roll progress."));
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
			TEXT("DirectShot regeneration requires an already-complete roll contract."));
		return Result;
	}
	if (bExplicitPlayerOwnedRoll)
	{
		if (Request.RequestingSide != EInitialTurnOrderPlayer::PlayerA
			&& Request.RequestingSide != EInitialTurnOrderPlayer::PlayerB)
		{
			SetFailure(Result, EError::InvalidRequestingSide,
				TEXT("Explicit DirectShot roll commands require PlayerA or PlayerB as RequestingSide."));
			return Result;
		}
		const EPurpose RequestedPurpose =
			Request.Mode == EMode::ResolveLongShotAttackRoll
				|| Request.Mode == EMode::ResolveCutInsideShotAttackRoll
				? EPurpose::PrimaryAttack
				: EPurpose::PrimaryDefense;
		if (Result.ProgressResult.bContractComplete
			|| Result.ProgressResult.NextPurpose != RequestedPurpose)
		{
			SetFailure(Result,
				bExplicitLongShotRoll
					? EError::WrongLongShotDirectRollStep
					: EError::WrongCutInsideShotDirectRollStep,
				TEXT("The requested explicit DirectShot roll is not the current authoritative step."));
			return Result;
		}
		const EInitialTurnOrderPlayer ExpectedSide =
			RequestedPurpose == EPurpose::PrimaryAttack
				? BeforeSession.Bundle.CurrentAttackingPlayer
				: BeforeSession.Bundle.CurrentDefendingPlayer;
		if (Request.RequestingSide != ExpectedSide)
		{
			SetFailure(Result, EError::WrongRequestingSide,
				TEXT("The requesting side does not own the current explicit DirectShot roll."));
			return Result;
		}
	}
	if (!Result.ProgressResult.bContractComplete && RollProvider == nullptr)
	{
		SetFailure(Result, EError::PostRouteRollProviderUnavailable,
			TEXT("DirectShot post-route roll provider is unavailable."));
		return Result;
	}
	if (SkillRuleSet == nullptr)
	{
		SetFailure(Result, EError::SkillRuleSetUnavailable,
			TEXT("DirectShot resolution requires authoritative SkillRuleSet."));
		return Result;
	}

	const int32 MaximumRollsThisCommand =
		bExplicitPlayerOwnedRoll ? 1 : MAX_int32;
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

	if (bExplicitPlayerOwnedRoll && !Result.ProgressResult.bContractComplete)
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

	EError InputErrorCode = EError::InputAdaptationFailed;
	FString InputErrorMessage;
	if (Result.ActionType == ESkillRuleType::LongShot)
	{
		if (!BuildInput(CandidateState, Result.LongShotInput,
			Result.PlayerCardSnapshots, InputErrorCode, InputErrorMessage))
		{
			SetFailure(Result, InputErrorCode, InputErrorMessage);
			return Result;
		}
		Result.LongShotResult = FLongShotDirectShotPlanQuery::BuildPlan(
			Result.PlayerCardSnapshots, *SkillRuleSet, Result.LongShotInput);
		if (!Result.LongShotResult.bSuccess)
		{
			SetFailure(Result, EError::LongShotPlanQueryFailed,
				Result.LongShotResult.ErrorMessage);
			return Result;
		}
	}
	else if (Result.ActionType == ESkillRuleType::CutInsideShot)
	{
		if (!BuildInput(CandidateState, Result.CutInsideShotInput,
			Result.PlayerCardSnapshots, InputErrorCode, InputErrorMessage))
		{
			SetFailure(Result, InputErrorCode, InputErrorMessage);
			return Result;
		}
		Result.CutInsideShotResult =
			FCutInsideShotDirectShotPlanQuery::BuildPlan(
				Result.PlayerCardSnapshots,
				*SkillRuleSet,
				Result.CutInsideShotInput);
		if (!Result.CutInsideShotResult.bSuccess)
		{
			SetFailure(Result, EError::CutInsideShotPlanQueryFailed,
				Result.CutInsideShotResult.ErrorMessage);
			return Result;
		}
	}
	else
	{
		checkNoEntry();
		return Result;
	}

	Result.AfterState = MoveTemp(CandidateState);
	Result.bResolvedNewRolls = Result.ProviderCallCount > 0;
	Result.bReplayedCompleteRolls = !bExplicitPlayerOwnedRoll
		&& Result.ProviderCallCount == 0;
	Result.bSuccess = true;
	return Result;
}
