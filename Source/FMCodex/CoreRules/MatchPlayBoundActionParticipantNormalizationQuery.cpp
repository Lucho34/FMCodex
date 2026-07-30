#include "MatchPlayBoundActionParticipantNormalizationQuery.h"

namespace MatchPlayBoundActionParticipantNormalizationImplementation
{
	bool IsPlayerSide(const EInitialTurnOrderPlayer Side)
	{
		return Side == EInitialTurnOrderPlayer::PlayerA
			|| Side == EInitialTurnOrderPlayer::PlayerB;
	}

	bool IsSupportedActionType(const ESkillRuleType ActionType)
	{
		return ActionType == ESkillRuleType::LongShot
			|| ActionType == ESkillRuleType::CutInsideShot
			|| ActionType == ESkillRuleType::PassControl
			|| ActionType == ESkillRuleType::Cross
			|| ActionType == ESkillRuleType::ThroughBall;
	}

	bool RequiresRunner(const ESkillRuleType ActionType)
	{
		return ActionType == ESkillRuleType::PassControl
			|| ActionType == ESkillRuleType::Cross
			|| ActionType == ESkillRuleType::ThroughBall;
	}

	EMatchPlayBoundActionParticipantSnapshotQueryErrorCode
	MapSnapshotQueryError(
		const EMatchPlayCardSnapshotAuthorityQueryErrorCode ErrorCode)
	{
		switch (ErrorCode)
		{
			case EMatchPlayCardSnapshotAuthorityQueryErrorCode::None:
				return EMatchPlayBoundActionParticipantSnapshotQueryErrorCode
					::None;
			case EMatchPlayCardSnapshotAuthorityQueryErrorCode
				::InvalidPlayerSide:
				return EMatchPlayBoundActionParticipantSnapshotQueryErrorCode
					::InvalidPlayerSide;
			case EMatchPlayCardSnapshotAuthorityQueryErrorCode::InvalidCardId:
				return EMatchPlayBoundActionParticipantSnapshotQueryErrorCode
					::InvalidCardId;
			case EMatchPlayCardSnapshotAuthorityQueryErrorCode
				::SnapshotValidationFailed:
				return EMatchPlayBoundActionParticipantSnapshotQueryErrorCode
					::SnapshotValidationFailed;
			case EMatchPlayCardSnapshotAuthorityQueryErrorCode
				::SnapshotNotFound:
				return EMatchPlayBoundActionParticipantSnapshotQueryErrorCode
					::SnapshotNotFound;
			default:
				return EMatchPlayBoundActionParticipantSnapshotQueryErrorCode
					::SnapshotValidationFailed;
		}
	}

	FMatchPlayBoundActionNormalizedParticipantValues CopyValues(
		const FPlayerAttributes& Source)
	{
		FMatchPlayBoundActionNormalizedParticipantValues Values;
		Values.Shooting = Source.Shooting;
		Values.Dribbling = Source.Dribbling;
		Values.Passing = Source.Passing;
		Values.OffBall = Source.OffBall;
		Values.Marking = Source.Marking;
		Values.Tackling = Source.Tackling;
		Values.Speed = Source.Speed;
		Values.Strength = Source.Strength;
		Values.Stamina = Source.Stamina;
		Values.LongShot = Source.LongShot;
		return Values;
	}

	FMatchPlayBoundActionNormalizedParticipant MakePresentParticipant(
		const FMatchPlayCardSnapshotAuthorityQueryResult& QueryResult)
	{
		FMatchPlayBoundActionNormalizedParticipant Participant;
		Participant.bIsPresent = QueryResult.bSuccess;
		Participant.Side = QueryResult.PlayerSide;
		Participant.CardId = QueryResult.CardId;
		Participant.bSnapshotQueryAttempted = true;
		Participant.bSnapshotQuerySucceeded = QueryResult.bSuccess;
		Participant.SnapshotQueryErrorCode =
			MapSnapshotQueryError(QueryResult.ErrorCode);
		Participant.SnapshotQueryErrorMessage = QueryResult.ErrorMessage;
		if (QueryResult.bSuccess)
		{
			Participant.Values =
				CopyValues(QueryResult.Snapshot.Attributes);
		}
		return Participant;
	}

	FMatchPlayBoundActionNormalizedParticipant MakeAbsentParticipant(
		const EInitialTurnOrderPlayer Side)
	{
		FMatchPlayBoundActionNormalizedParticipant Participant;
		Participant.Side = Side;
		return Participant;
	}

	void SetFailure(
		FMatchPlayBoundActionParticipantNormalizationResult& Result,
		const EMatchPlayBoundActionParticipantNormalizationErrorCode
			ErrorCode,
		const FString& ErrorMessage)
	{
		Result.ErrorCode = ErrorCode;
		Result.ErrorMessage = ErrorMessage;
	}
}

FMatchPlayBoundActionParticipantNormalizationResult
FMatchPlayBoundActionParticipantNormalizationQuery::Query(
	const FMatchPlayState& State,
	const FMatchPlayBoundActionParticipantNormalizationRequest& Request)
{
	using namespace
		MatchPlayBoundActionParticipantNormalizationImplementation;

	FMatchPlayBoundActionParticipantNormalizationResult Result;
	Result.Request = Request;
	Result.BindingResult =
		FMatchPlayCurrentAttackResolutionBinding::Query(
			State,
			Request.AttackSequence);
	if (!Result.BindingResult.bSuccess)
	{
		SetFailure(
			Result,
			EMatchPlayBoundActionParticipantNormalizationErrorCode
				::BindingFailed,
			Result.BindingResult.ErrorMessage);
		return Result;
	}

	const FMatchPlayCurrentAttackResolutionBindingValue& Binding =
		Result.BindingResult.Binding;
	Result.ActionType = Binding.ActionType;
	Result.ElectiveBranchIntent = Binding.ElectiveBranchIntent;
	if (!IsSupportedActionType(Result.ActionType))
	{
		SetFailure(
			Result,
			EMatchPlayBoundActionParticipantNormalizationErrorCode
				::UnsupportedActionType,
			TEXT("Bound action type is not supported by participant normalization."));
		return Result;
	}

	Result.CurrentAttackingPlayer =
		Result.BindingResult.ReadyValidationResult.CurrentAttackingPlayer;
	if (!IsPlayerSide(Result.CurrentAttackingPlayer))
	{
		SetFailure(
			Result,
			EMatchPlayBoundActionParticipantNormalizationErrorCode
				::InvalidAttackingPlayer,
			TEXT("Binding did not provide a valid current attacking player."));
		return Result;
	}

	Result.CurrentDefendingPlayer =
		Result.BindingResult.ReadyValidationResult.CurrentDefendingPlayer;
	if (!IsPlayerSide(Result.CurrentDefendingPlayer))
	{
		SetFailure(
			Result,
			EMatchPlayBoundActionParticipantNormalizationErrorCode
				::InvalidDefendingPlayer,
			TEXT("Binding did not provide a valid current defending player."));
		return Result;
	}

	Result.Bundle.AttackSequence = Binding.AttackSequence;
	Result.Bundle.ActionType = Binding.ActionType;
	Result.Bundle.CurrentAttackingPlayer =
		Result.CurrentAttackingPlayer;
	Result.Bundle.CurrentDefendingPlayer =
		Result.CurrentDefendingPlayer;
	Result.Bundle.Binding = Binding;
	Result.Bundle.ElectiveBranchIntent =
		Binding.ElectiveBranchIntent;

	const FMatchPlayCardSnapshotAuthorityQueryResult& CarrierQueryResult =
		Result.BindingResult.ReadyValidationResult
			.CarrierSnapshotQueryResult;
	Result.Bundle.Carrier = MakePresentParticipant(CarrierQueryResult);
	if (!CarrierQueryResult.bSuccess)
	{
		SetFailure(
			Result,
			EMatchPlayBoundActionParticipantNormalizationErrorCode
				::CarrierSnapshotQueryFailed,
			CarrierQueryResult.ErrorMessage);
		return Result;
	}

	const FMatchPlayCardSnapshotAuthorityQueryResult& MarkerQueryResult =
		Result.BindingResult.ReadyValidationResult
			.MarkerSnapshotQueryResult;
	Result.Bundle.Marker = MakePresentParticipant(MarkerQueryResult);
	if (!MarkerQueryResult.bSuccess)
	{
		SetFailure(
			Result,
			EMatchPlayBoundActionParticipantNormalizationErrorCode
				::MarkerSnapshotQueryFailed,
			MarkerQueryResult.ErrorMessage);
		return Result;
	}

	const bool bRequiresRunner = RequiresRunner(Binding.ActionType);
	if (bRequiresRunner && Binding.RunnerCardId.IsNone())
	{
		SetFailure(
			Result,
			EMatchPlayBoundActionParticipantNormalizationErrorCode
				::MissingRequiredRunner,
			TEXT("Bound action requires a Runner."));
		return Result;
	}
	if (!bRequiresRunner && !Binding.RunnerCardId.IsNone())
	{
		SetFailure(
			Result,
			EMatchPlayBoundActionParticipantNormalizationErrorCode
				::UnexpectedRunner,
			TEXT("Bound action does not permit a Runner."));
		return Result;
	}

	Result.Bundle.bHasRunner = bRequiresRunner;
	if (bRequiresRunner)
	{
		const FMatchPlayCardSnapshotAuthorityQueryResult& RunnerQueryResult =
			Result.BindingResult.ReadyValidationResult
				.RunnerSnapshotQueryResult;
		Result.Bundle.Runner = MakePresentParticipant(RunnerQueryResult);
		if (!RunnerQueryResult.bSuccess)
		{
			SetFailure(
				Result,
				EMatchPlayBoundActionParticipantNormalizationErrorCode
					::RunnerSnapshotQueryFailed,
				RunnerQueryResult.ErrorMessage);
			return Result;
		}
	}
	else
	{
		Result.Bundle.Runner =
			MakeAbsentParticipant(Result.CurrentAttackingPlayer);
	}

	const bool bHelperPresenceIsConsistent =
		Binding.bHasHelper == !Binding.HelperCardId.IsNone();
	if (!bHelperPresenceIsConsistent
		|| (!bRequiresRunner
			&& (Binding.bHasHelper || !Binding.HelperCardId.IsNone())))
	{
		SetFailure(
			Result,
			EMatchPlayBoundActionParticipantNormalizationErrorCode
				::InvalidHelperPresence,
			TEXT("Bound Helper presence is inconsistent with the action contract."));
		return Result;
	}

	Result.Bundle.bHasHelper = Binding.bHasHelper;
	if (Binding.bHasHelper)
	{
		const FMatchPlayCardSnapshotAuthorityQueryResult& HelperQueryResult =
			Result.BindingResult.ReadyValidationResult
				.HelperAuthorityResult.SnapshotQueryResult;
		Result.Bundle.Helper = MakePresentParticipant(HelperQueryResult);
		if (!HelperQueryResult.bSuccess)
		{
			SetFailure(
				Result,
				EMatchPlayBoundActionParticipantNormalizationErrorCode
					::HelperSnapshotQueryFailed,
				HelperQueryResult.ErrorMessage);
			return Result;
		}
	}
	else
	{
		Result.Bundle.Helper =
			MakeAbsentParticipant(Result.CurrentDefendingPlayer);
	}

	Result.bSuccess = true;
	Result.ErrorCode =
		EMatchPlayBoundActionParticipantNormalizationErrorCode::None;
	return Result;
}
