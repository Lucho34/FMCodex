#include "MatchPlayCurrentAttackResolveDeadCornerPostRouteDecisionOrchestrator.h"

#include "MatchPlayCardSnapshotAuthority.h"

namespace MatchPlayCurrentAttackResolveDeadCornerPostRouteDecision
{
	using EError =
		EMatchPlayCurrentAttackResolveDeadCornerPostRouteDecisionErrorCode;
	using EPurpose = EMatchPlayCurrentAttackPostRouteRollPurpose;
	using EPhase = EMatchPlayCurrentAttackPostRouteRollPhase;
	using FResult =
		FMatchPlayCurrentAttackResolveDeadCornerPostRouteDecisionResult;

	void SetFailure(
		FResult& Result,
		const EError ErrorCode,
		const FString& ErrorMessage)
	{
		Result.ErrorCode = ErrorCode;
		Result.ErrorMessage = ErrorMessage;
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

	template <typename TInput>
	bool BuildInput(
		const FMatchPlayState& State,
		TInput& OutInput,
		FPlayerCardRuleSnapshotSet& OutSnapshots,
		FString& OutErrorMessage)
	{
		const FMatchPlayCurrentAttackState& CurrentAttack = State.CurrentAttack;
		const FMatchPlayCurrentAttackResolutionSession& Session =
			CurrentAttack.ResolutionSession;
		const FMatchPlayCurrentAttackResolutionSessionBundle& Bundle =
			Session.Bundle;
		const TArray<FMatchPlayCurrentAttackPostRouteRollRecord>& Records =
			Session.PostRouteRollProgress.RollRecords;
		if (Records.Num() != 2 || Session.AttackSequence > MAX_int32)
		{
			OutErrorMessage =
				TEXT("DeadCorner adaptation requires two paired rolls and a representable TurnIndex.");
			return false;
		}

		const FMatchPlayCardSnapshotAuthorityQueryResult SnapshotQuery =
			FMatchPlayCardSnapshotAuthorityQuery::FindByPlayerSideAndCardId(
				State.CardSnapshotAuthority,
				Bundle.Carrier.Side,
				Bundle.Carrier.CardId);
		if (!SnapshotQuery.bSuccess)
		{
			OutErrorMessage = SnapshotQuery.ErrorMessage;
			return false;
		}
		OutSnapshots.Cards.Add(SnapshotQuery.Snapshot);
		OutInput.SkillId = Bundle.Binding.SkillId;
		OutInput.AttackerCardId = Bundle.Carrier.CardId;
		OutInput.CurrentActionPoint = CurrentAttack.ActionPoint;
		OutInput.bHasExternalAttackD6A = true;
		OutInput.ExternalAttackD6A = Records[0].RawD6;
		OutInput.bHasExternalAttackD6B = true;
		OutInput.ExternalAttackD6B = Records[1].RawD6;
		const uint64 Sequence = static_cast<uint64>(Session.AttackSequence);
		OutInput.LogId = FGuid(
			0x44434F52,
			static_cast<uint32>(Sequence >> 32),
			static_cast<uint32>(Sequence),
			0x44454349);
		OutInput.TurnIndex = static_cast<int32>(Session.AttackSequence - 1);
		OutInput.AttackerPlayerId = MakePlayerId(Bundle.Carrier.Side);
		return true;
	}

	bool IsDeadCornerBranch(
		const FMatchPlayCurrentAttackActualBranch& Branch)
	{
		return (Branch.ActionType == ESkillRuleType::LongShot
				&& Branch.LongShot == EMatchPlayLongShotActualBranch::DeadCorner)
			|| (Branch.ActionType == ESkillRuleType::CutInsideShot
				&& Branch.CutInsideShot
					== EMatchPlayCutInsideShotActualBranch::DeadCorner);
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

FMatchPlayCurrentAttackResolveDeadCornerPostRouteDecisionResult
FMatchPlayCurrentAttackResolveDeadCornerPostRouteDecisionOrchestrator::Resolve(
	const FMatchPlayState& BeforeState,
	const FMatchPlayCurrentAttackResolveDeadCornerPostRouteDecisionRequest& Request,
	const FSkillRuleSnapshotSet* SkillRuleSet,
	IMatchPlayPostRouteRollProvider* RollProvider)
{
	using namespace MatchPlayCurrentAttackResolveDeadCornerPostRouteDecision;
	FResult Result;
	Result.Request = Request;
	Result.BeforeState = BeforeState;
	Result.AfterState = BeforeState;
	if (!BeforeState.RuntimeState.bIsInitialized)
	{
		SetFailure(Result, EError::MatchPlayStateNotInitialized,
			TEXT("DeadCorner post-route decision requires initialized MatchPlay State."));
		return Result;
	}
	if (!BeforeState.bHasCurrentAttack)
	{
		SetFailure(Result, EError::NoCurrentAttack,
			TEXT("DeadCorner post-route decision requires an active CurrentAttack."));
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
			TEXT("DeadCorner post-route decision requires a Resolution Session."));
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
			TEXT("DeadCorner post-route decision requires RouteResolved."));
		return Result;
	}
	if (!BeforeSession.bHasActualBranch
		|| !IsDeadCornerBranch(BeforeSession.ActualBranch))
	{
		SetFailure(Result, EError::NotDeadCornerBranch,
			TEXT("This operation supports only LongShot/CutInsideShot DeadCorner."));
		return Result;
	}
	Result.ActionType = BeforeSession.ActualBranch.ActionType;

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
			TEXT("DeadCorner decision requires primary-branch roll progress."));
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
	if (!Result.ProgressResult.bContractComplete && RollProvider == nullptr)
	{
		SetFailure(Result, EError::PostRouteRollProviderUnavailable,
			TEXT("DeadCorner post-route roll provider is unavailable."));
		return Result;
	}
	if (SkillRuleSet == nullptr)
	{
		SetFailure(Result, EError::SkillRuleSetUnavailable,
			TEXT("DeadCorner decision requires authoritative SkillRuleSet."));
		return Result;
	}

	while (!Result.ProgressResult.bContractComplete)
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
		FMatchPlayCurrentAttackResolutionSessionStateValidator::Validate(
			CandidateState);
	if (!Result.SessionStateValidationResult.bIsCanonical)
	{
		SetFailure(Result, EError::InvalidPostRouteProgress,
			Result.SessionStateValidationResult.ErrorMessage);
		return Result;
	}
	FString InputError;
	if (Result.ActionType == ESkillRuleType::LongShot)
	{
		if (!BuildInput(
				CandidateState,
				Result.LongShotInput,
				Result.PlayerCardSnapshots,
				InputError))
		{
			SetFailure(Result, EError::InputAdaptationFailed, InputError);
			return Result;
		}
		Result.LongShotResult = FLongShotDeadCornerDecisionQuery::Evaluate(
			Result.PlayerCardSnapshots, *SkillRuleSet, Result.LongShotInput);
		if (!Result.LongShotResult.bSuccess)
		{
			SetFailure(Result, EError::LongShotDecisionQueryFailed,
				Result.LongShotResult.ErrorMessage);
			return Result;
		}
	}
	else if (Result.ActionType == ESkillRuleType::CutInsideShot)
	{
		if (!BuildInput(
				CandidateState,
				Result.CutInsideShotInput,
				Result.PlayerCardSnapshots,
				InputError))
		{
			SetFailure(Result, EError::InputAdaptationFailed, InputError);
			return Result;
		}
		Result.CutInsideShotResult =
			FCutInsideShotDeadCornerDecisionQuery::Evaluate(
				Result.PlayerCardSnapshots,
				*SkillRuleSet,
				Result.CutInsideShotInput);
		if (!Result.CutInsideShotResult.bSuccess)
		{
			SetFailure(Result, EError::CutInsideShotDecisionQueryFailed,
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
	Result.bReplayedCompleteRolls = Result.ProviderCallCount == 0;
	Result.bSuccess = true;
	return Result;
}
