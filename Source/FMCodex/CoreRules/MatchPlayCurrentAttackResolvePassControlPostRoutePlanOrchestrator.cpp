#include "MatchPlayCurrentAttackResolvePassControlPostRoutePlanOrchestrator.h"

#include "MatchPlayCardSnapshotAuthority.h"

namespace MatchPlayCurrentAttackResolvePassControlPostRoutePlan
{
	using EError =
		EMatchPlayCurrentAttackResolvePassControlPostRoutePlanErrorCode;
	using EPurpose = EMatchPlayCurrentAttackPostRouteRollPurpose;
	using EPhase = EMatchPlayCurrentAttackPostRouteRollPhase;
	using FResult =
		FMatchPlayCurrentAttackResolvePassControlPostRoutePlanResult;

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

	EPassControlAdvanceType MapAdvanceType(
		const EMatchPlayPassControlActualBranch Branch)
	{
		switch (Branch)
		{
		case EMatchPlayPassControlActualBranch::PassAdvance:
			return EPassControlAdvanceType::PassAdvance;
		case EMatchPlayPassControlActualBranch::DribbleAdvance:
			return EPassControlAdvanceType::DribbleAdvance;
		case EMatchPlayPassControlActualBranch::RunAdvance:
			return EPassControlAdvanceType::RunAdvance;
		default:
			return EPassControlAdvanceType::None;
		}
	}

	template <typename TInput>
	void PopulateInput(
		const FMatchPlayCurrentAttackState& CurrentAttack,
		const FMatchPlayCurrentAttackResolutionSession& Session,
		const EPassControlAdvanceType AdvanceType,
		TInput& Input)
	{
		const FMatchPlayCurrentAttackResolutionSessionBundle& Bundle =
			Session.Bundle;
		const TArray<FMatchPlayCurrentAttackPostRouteRollRecord>& Records =
			Session.PostRouteRollProgress.RollRecords;
		Input.SkillId = Bundle.Binding.SkillId;
		Input.AdvanceType = AdvanceType;
		Input.CarrierCardId = Bundle.Carrier.CardId;
		Input.RunnerCardId = Bundle.Runner.CardId;
		Input.MarkerCardId = Bundle.Marker.CardId;
		Input.bHasHelper = Bundle.bHasHelper;
		Input.HelperCardId = Bundle.bHasHelper
			? Bundle.Helper.CardId
			: NAME_None;
		Input.CurrentActionPoint = CurrentAttack.ActionPoint;
		Input.bHasExternalAttackD6 = true;
		Input.ExternalAttackD6 = Records[0].RawD6;
		Input.bHasExternalDefenseD6 = true;
		Input.ExternalDefenseD6 = Records[1].RawD6;
		const uint64 Sequence = static_cast<uint64>(Session.AttackSequence);
		Input.LogId = FGuid(
			0x50434C4E,
			static_cast<uint32>(Sequence >> 32),
			static_cast<uint32>(Sequence),
			0x504C414E);
		Input.TurnIndex = static_cast<int32>(Session.AttackSequence - 1);
		Input.CarrierPlayerId = MakePlayerId(Bundle.Carrier.Side);
		Input.RunnerPlayerId = MakePlayerId(Bundle.Runner.Side);
		Input.MarkerPlayerId = MakePlayerId(Bundle.Marker.Side);
		Input.HelperPlayerId = Bundle.bHasHelper
			? MakePlayerId(Bundle.Helper.Side)
			: NAME_None;
	}

	bool BuildInputs(const FMatchPlayState& State, FResult& Result)
	{
		const FMatchPlayCurrentAttackState& CurrentAttack = State.CurrentAttack;
		const FMatchPlayCurrentAttackResolutionSession& Session =
			CurrentAttack.ResolutionSession;
		const FMatchPlayCurrentAttackResolutionSessionBundle& Bundle =
			Session.Bundle;
		if (Session.PostRouteRollProgress.Phase != EPhase::PrimaryBranch
			|| Session.PostRouteRollProgress.RollRecords.Num() != 2
			|| Session.AttackSequence > MAX_int32)
		{
			SetFailure(
				Result,
				EError::InputAdaptationFailed,
				TEXT("PassControl adaptation requires two primary rolls and a representable TurnIndex."));
			return false;
		}

		FString SnapshotError;
		if (!AddBoundSnapshot(
				State, Bundle.Carrier, Result.PlayerCardSnapshots, SnapshotError)
			|| !AddBoundSnapshot(
					State, Bundle.Runner, Result.PlayerCardSnapshots, SnapshotError)
			|| !AddBoundSnapshot(
					State, Bundle.Marker, Result.PlayerCardSnapshots, SnapshotError)
			|| (Bundle.bHasHelper
				&& !AddBoundSnapshot(
					State, Bundle.Helper, Result.PlayerCardSnapshots, SnapshotError)))
		{
			SetFailure(
				Result,
				EError::ParticipantSnapshotUnavailable,
				FString::Printf(
					TEXT("PassControl participant snapshot adaptation failed: %s"),
					*SnapshotError));
			return false;
		}

		const EPassControlAdvanceType AdvanceType =
			MapAdvanceType(Result.ActualBranch);
		if (AdvanceType == EPassControlAdvanceType::None)
		{
			SetFailure(
				Result,
				EError::InputAdaptationFailed,
				TEXT("Resolved PassControl branch cannot be adapted."));
			return false;
		}

		switch (Result.ActualBranch)
		{
		case EMatchPlayPassControlActualBranch::PassAdvance:
			PopulateInput(CurrentAttack, Session, AdvanceType,
				Result.PassAdvanceInput);
			break;
		case EMatchPlayPassControlActualBranch::DribbleAdvance:
			PopulateInput(CurrentAttack, Session, AdvanceType,
				Result.DribbleAdvanceInput);
			break;
		case EMatchPlayPassControlActualBranch::RunAdvance:
			PopulateInput(CurrentAttack, Session, AdvanceType,
				Result.RunAdvanceInput);
			break;
		default:
			checkNoEntry();
			return false;
		}
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

FMatchPlayCurrentAttackResolvePassControlPostRoutePlanResult
FMatchPlayCurrentAttackResolvePassControlPostRoutePlanOrchestrator::Resolve(
	const FMatchPlayState& BeforeState,
	const FMatchPlayCurrentAttackResolvePassControlPostRoutePlanRequest& Request,
	const FSkillRuleSnapshotSet* SkillRuleSet,
	IMatchPlayPostRouteRollProvider* RollProvider)
{
	using namespace MatchPlayCurrentAttackResolvePassControlPostRoutePlan;
	FResult Result;
	Result.Request = Request;
	Result.BeforeState = BeforeState;
	Result.AfterState = BeforeState;
	if (!BeforeState.RuntimeState.bIsInitialized)
	{
		SetFailure(Result, EError::MatchPlayStateNotInitialized,
			TEXT("PassControl post-route plan resolution requires initialized MatchPlay State."));
		return Result;
	}
	if (!BeforeState.bHasCurrentAttack)
	{
		SetFailure(Result, EError::NoCurrentAttack,
			TEXT("PassControl post-route plan resolution requires an active CurrentAttack."));
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
			TEXT("PassControl post-route plan resolution requires a Resolution Session."));
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
			TEXT("PassControl post-route plan resolution requires RouteResolved."));
		return Result;
	}
	if (!BeforeSession.bHasActualBranch
		|| BeforeSession.ActualBranch.ActionType != ESkillRuleType::PassControl
		|| BeforeSession.ActualBranch.PassControl
			== EMatchPlayPassControlActualBranch::None)
	{
		SetFailure(Result, EError::NotPassControlBranch,
			TEXT("This operation supports only resolved PassControl branches."));
		return Result;
	}
	Result.ActualBranch = BeforeSession.ActualBranch.PassControl;

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
			TEXT("PassControl plan resolution requires primary-branch roll progress."));
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
			TEXT("PassControl post-route roll provider is unavailable."));
		return Result;
	}
	if (SkillRuleSet == nullptr)
	{
		SetFailure(Result, EError::SkillRuleSetUnavailable,
			TEXT("PassControl plan resolution requires an authoritative SkillRuleSet dependency."));
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

	Result.SessionStateValidationResult =
		FMatchPlayCurrentAttackResolutionSessionStateValidator::Validate(
			CandidateState);
	if (!Result.SessionStateValidationResult.bIsCanonical)
	{
		SetFailure(Result, EError::InvalidPostRouteProgress,
			Result.SessionStateValidationResult.ErrorMessage);
		return Result;
	}
	if (!BuildInputs(CandidateState, Result))
	{
		return Result;
	}

	switch (Result.ActualBranch)
	{
	case EMatchPlayPassControlActualBranch::PassAdvance:
		Result.PassAdvanceResult =
			FPassControlPassAdvancePlanQuery::BuildPlan(
				Result.PlayerCardSnapshots,
				*SkillRuleSet,
				Result.PassAdvanceInput);
		if (!Result.PassAdvanceResult.bSuccess)
		{
			SetFailure(Result, EError::PassAdvancePlanQueryFailed,
				Result.PassAdvanceResult.ErrorMessage);
			return Result;
		}
		break;
	case EMatchPlayPassControlActualBranch::DribbleAdvance:
		Result.DribbleAdvanceResult =
			FPassControlDribbleAdvancePlanQuery::BuildPlan(
				Result.PlayerCardSnapshots,
				*SkillRuleSet,
				Result.DribbleAdvanceInput);
		if (!Result.DribbleAdvanceResult.bSuccess)
		{
			SetFailure(Result, EError::DribbleAdvancePlanQueryFailed,
				Result.DribbleAdvanceResult.ErrorMessage);
			return Result;
		}
		break;
	case EMatchPlayPassControlActualBranch::RunAdvance:
		Result.RunAdvanceResult =
			FPassControlRunAdvancePlanQuery::BuildPlan(
				Result.PlayerCardSnapshots,
				*SkillRuleSet,
				Result.RunAdvanceInput);
		if (!Result.RunAdvanceResult.bSuccess)
		{
			SetFailure(Result, EError::RunAdvancePlanQueryFailed,
				Result.RunAdvanceResult.ErrorMessage);
			return Result;
		}
		break;
	default:
		checkNoEntry();
		return Result;
	}

	Result.AfterState = MoveTemp(CandidateState);
	Result.bResolvedNewRolls = Result.ProviderCallCount > 0;
	Result.bReplayedCompleteRolls = Result.ProviderCallCount == 0;
	Result.bSuccess = true;
	return Result;
}
