#include "MatchPlayCurrentAttackRouteStateValidator.h"

#include "SetPieceTypeSelectionQuery.h"

namespace MatchPlayCurrentAttackRouteStateValidator
{
	void SetFailure(
		FMatchPlayCurrentAttackRouteStateValidationResult& Result,
		const EMatchPlayCurrentAttackRouteStateValidationErrorCode ErrorCode,
		const FString& ErrorMessage)
	{
		Result.ErrorCode = ErrorCode;
		Result.ErrorMessage = ErrorMessage;
	}

	bool IsDefaultSendingOffState(
		const FMatchPlaySendingOffRouteState& State)
	{
		const FMatchPlaySendingOffRouteState DefaultState;
		return FMatchPlaySendingOffRouteState::StaticStruct()
			->CompareScriptStruct(&State, &DefaultState, 0);
	}

	bool IsDefaultSetPieceState(
		const FMatchPlaySetPieceRouteState& State)
	{
		const FMatchPlaySetPieceRouteState DefaultState;
		return FMatchPlaySetPieceRouteState::StaticStruct()
			->CompareScriptStruct(&State, &DefaultState, 0);
	}

	template <typename TStruct>
	bool IsDefaultStruct(const TStruct& State)
	{
		const TStruct DefaultState;
		return TStruct::StaticStruct()->CompareScriptStruct(
			&State,
			&DefaultState,
			0);
	}

	bool HasDefaultOrdinaryPayload(
		const FMatchPlayCurrentAttackState& Attack)
	{
		return Attack.SelectionStage
				== EMatchPlayCurrentAttackSelectionStage::None
			&& IsDefaultStruct(Attack.ActionPreparation)
			&& Attack.CurrentLegalDeploymentSide
				== EInitialTurnOrderPlayer::None
			&& !Attack.bAttackerDeploymentFinished
			&& !Attack.bDefenderDeploymentFinished
			&& Attack.DeploymentPlacements.IsEmpty()
			&& !Attack.bCurrentDefenseGoalkeeperActivated
			&& !Attack.bHasSelectedAction
			&& IsDefaultStruct(Attack.SelectedAction)
			&& !Attack.bHasResolutionSession
			&& IsDefaultStruct(Attack.ResolutionSession);
	}

	int64 GetExpectedAttackSequence(const FMatchPlayState& State)
	{
		return static_cast<int64>(
			State.RuntimeState.PlayerAState.UsedAttackCount)
			+ static_cast<int64>(
				State.RuntimeState.PlayerBState.UsedAttackCount)
			+ 1;
	}
}

FMatchPlayCurrentAttackRouteStateValidationResult
FMatchPlayCurrentAttackRouteStateValidator::Validate(
	const FMatchPlayState& State)
{
	using namespace MatchPlayCurrentAttackRouteStateValidator;

	FMatchPlayCurrentAttackRouteStateValidationResult Result;
	if (!State.bHasCurrentAttack)
	{
		const FMatchPlayCurrentAttackState DefaultAttack;
		if (!FMatchPlayCurrentAttackState::StaticStruct()
				->CompareScriptStruct(
					&State.CurrentAttack,
					&DefaultAttack,
					0))
		{
			SetFailure(
				Result,
				EMatchPlayCurrentAttackRouteStateValidationErrorCode
					::InactiveStateHasPayload,
				TEXT("A state without a current attack must retain the default current-attack payload."));
			return Result;
		}

		Result.bIsCanonical = true;
		return Result;
	}

	const FMatchPlayCurrentAttackState& Attack = State.CurrentAttack;
	if (Attack.AttackSequence <= 0)
	{
		SetFailure(
			Result,
			EMatchPlayCurrentAttackRouteStateValidationErrorCode
				::InvalidAttackSequence,
			TEXT("A current attack requires a positive AttackSequence."));
		return Result;
	}

	if (Attack.AttackSequence != GetExpectedAttackSequence(State))
	{
		SetFailure(
			Result,
			EMatchPlayCurrentAttackRouteStateValidationErrorCode
				::AttackSequenceMismatch,
			TEXT("CurrentAttack.AttackSequence must match the authoritative used-attack counts."));
		return Result;
	}

	if (Attack.RawInitialD12 < 1 || Attack.RawInitialD12 > 12)
	{
		SetFailure(
			Result,
			EMatchPlayCurrentAttackRouteStateValidationErrorCode
				::InvalidRawInitialD12,
			TEXT("RawInitialD12 must be in range [1, 12]."));
		return Result;
	}

	if (Attack.ActionPoint != Attack.RawInitialD12)
	{
		SetFailure(
			Result,
			EMatchPlayCurrentAttackRouteStateValidationErrorCode
				::ActionPointMismatch,
			TEXT("ActionPoint must preserve the raw initial D12 value."));
		return Result;
	}

	switch (Attack.RouteKind)
	{
	case EMatchPlayCurrentAttackRouteKind::Ordinary:
		if (Attack.RawInitialD12 < 2 || Attack.RawInitialD12 > 8)
		{
			SetFailure(
				Result,
				EMatchPlayCurrentAttackRouteStateValidationErrorCode
					::OrdinaryD12Mismatch,
				TEXT("Ordinary route requires an initial D12 in range [2, 8]."));
			return Result;
		}
		if (Attack.Phase == EMatchPlayCurrentAttackPhase::RoutePending)
		{
			SetFailure(Result,
				EMatchPlayCurrentAttackRouteStateValidationErrorCode
					::WrongRoutePhase,
				TEXT("Ordinary route must use its production Deployment or Resolution phase."));
			return Result;
		}
		if (!IsDefaultSendingOffState(Attack.SendingOffRoute))
		{
			SetFailure(
				Result,
				EMatchPlayCurrentAttackRouteStateValidationErrorCode
					::UnexpectedSendingOffPayload,
				TEXT("Ordinary route cannot carry Sending-Off state."));
			return Result;
		}
		if (!IsDefaultSetPieceState(Attack.SetPieceRoute))
		{
			SetFailure(
				Result,
				EMatchPlayCurrentAttackRouteStateValidationErrorCode
					::UnexpectedSetPiecePayload,
				TEXT("Ordinary route cannot carry Set Piece state."));
			return Result;
		}
		break;

	case EMatchPlayCurrentAttackRouteKind::SendingOff:
		if (Attack.RawInitialD12 != 1)
		{
			SetFailure(
				Result,
				EMatchPlayCurrentAttackRouteStateValidationErrorCode
					::SendingOffD12Mismatch,
				TEXT("Sending-Off route requires an initial D12 of 1."));
			return Result;
		}
		if (Attack.Phase != EMatchPlayCurrentAttackPhase::RoutePending)
		{
			SetFailure(Result,
				EMatchPlayCurrentAttackRouteStateValidationErrorCode
					::WrongRoutePhase,
				TEXT("Sending-Off route must remain in RoutePending phase."));
			return Result;
		}
		if (Attack.SendingOffRoute.Stage
			== EMatchPlaySendingOffRouteStage::AwaitingResolution)
		{
			if (Attack.LifecycleState
				!= EMatchPlayCurrentAttackLifecycleState::Active)
			{
				SetFailure(Result,
					EMatchPlayCurrentAttackRouteStateValidationErrorCode
						::WrongRouteLifecycle,
					TEXT("Awaiting Sending-Off resolution must remain active."));
				return Result;
			}
			if (Attack.SendingOffRoute.SelectionOutcome
					!= EMatchPlaySendingOffSelectionOutcome::None
				|| !Attack.SendingOffRoute.EjectedCardId.IsNone()
				|| Attack.SendingOffRoute.GameplayOutcome
					!= EMatchPlaySendingOffGameplayOutcome::None)
			{
				SetFailure(Result,
					EMatchPlayCurrentAttackRouteStateValidationErrorCode
						::InvalidSendingOffPayload,
					TEXT("Awaiting Sending-Off resolution cannot carry terminal result payload."));
				return Result;
			}
		}
		else if (Attack.SendingOffRoute.Stage
			== EMatchPlaySendingOffRouteStage::Resolved)
		{
			if (State.RuntimeState.CurrentAttackingPlayer
					!= EInitialTurnOrderPlayer::PlayerA
				&& State.RuntimeState.CurrentAttackingPlayer
					!= EInitialTurnOrderPlayer::PlayerB)
			{
				SetFailure(Result,
					EMatchPlayCurrentAttackRouteStateValidationErrorCode
						::InvalidSendingOffPayload,
					TEXT("Resolved Sending-Off requires a valid current attacking side."));
				return Result;
			}
			if (Attack.LifecycleState
					!= EMatchPlayCurrentAttackLifecycleState
						::TerminalPendingAdvance
				|| Attack.SendingOffRoute.GameplayOutcome
					!= EMatchPlaySendingOffGameplayOutcome::NoGoal)
			{
				SetFailure(Result,
					EMatchPlayCurrentAttackRouteStateValidationErrorCode
						::InvalidSendingOffPayload,
					TEXT("Resolved Sending-Off must persist NoGoal and await explicit advance."));
				return Result;
			}

			const EMatchPlaySendingOffSelectionOutcome SelectionOutcome =
				Attack.SendingOffRoute.SelectionOutcome;
			if (SelectionOutcome
					== EMatchPlaySendingOffSelectionOutcome
						::NoEligibleCandidate)
			{
				if (!Attack.SendingOffRoute.EjectedCardId.IsNone())
				{
					SetFailure(Result,
						EMatchPlayCurrentAttackRouteStateValidationErrorCode
							::InvalidSendingOffPayload,
						TEXT("NoEligibleCandidate cannot carry an ejected CardId."));
					return Result;
				}
			}
			else if (SelectionOutcome
				== EMatchPlaySendingOffSelectionOutcome::CardEjected)
			{
				const FName EjectedCardId =
					Attack.SendingOffRoute.EjectedCardId;
				const FCardUsageState& AttackerUsage =
					State.RuntimeState.CurrentAttackingPlayer
						== EInitialTurnOrderPlayer::PlayerA
						? State.CardUsageState.PlayerACardUsageState
						: State.CardUsageState.PlayerBCardUsageState;
				if (EjectedCardId.IsNone()
					|| !AttackerUsage.EjectedCardIds.Contains(EjectedCardId)
					|| AttackerUsage.AvailableCardIds.Contains(EjectedCardId)
					|| AttackerUsage.UsedCardIds.Contains(EjectedCardId))
				{
					SetFailure(Result,
						EMatchPlayCurrentAttackRouteStateValidationErrorCode
							::SendingOffEjectionMismatch,
						TEXT("Resolved Sending-Off card must exist only in the attacker's Ejected zone."));
					return Result;
				}
			}
			else
			{
				SetFailure(Result,
					EMatchPlayCurrentAttackRouteStateValidationErrorCode
						::InvalidSendingOffPayload,
					TEXT("Resolved Sending-Off requires an explicit selection outcome."));
				return Result;
			}
		}
		else
		{
			SetFailure(Result,
				EMatchPlayCurrentAttackRouteStateValidationErrorCode
					::InvalidSendingOffStage,
				TEXT("Sending-Off route stage is invalid."));
			return Result;
		}
		if (!IsDefaultSetPieceState(Attack.SetPieceRoute))
		{
			SetFailure(Result,
				EMatchPlayCurrentAttackRouteStateValidationErrorCode
					::UnexpectedSetPiecePayload,
				TEXT("Sending-Off route cannot carry Set Piece state."));
			return Result;
		}
		if (!HasDefaultOrdinaryPayload(Attack))
		{
			SetFailure(Result,
				EMatchPlayCurrentAttackRouteStateValidationErrorCode
					::UnexpectedOrdinaryPayload,
				TEXT("Sending-Off route cannot start ordinary attack state."));
			return Result;
		}
		break;

	case EMatchPlayCurrentAttackRouteKind::SetPiece:
		if (Attack.RawInitialD12 < 9 || Attack.RawInitialD12 > 12)
		{
			SetFailure(Result,
				EMatchPlayCurrentAttackRouteStateValidationErrorCode
					::SetPieceD12Mismatch,
				TEXT("Set Piece route requires an initial D12 in range [9, 12]."));
			return Result;
		}
		if (Attack.LifecycleState
			!= EMatchPlayCurrentAttackLifecycleState::Active)
		{
			SetFailure(Result,
				EMatchPlayCurrentAttackRouteStateValidationErrorCode
					::WrongRouteLifecycle,
				TEXT("Set Piece route foundation must remain active while concrete resolution is pending."));
			return Result;
		}
		if (Attack.Phase != EMatchPlayCurrentAttackPhase::RoutePending)
		{
			SetFailure(Result,
				EMatchPlayCurrentAttackRouteStateValidationErrorCode
					::WrongRoutePhase,
				TEXT("Set Piece route must remain in RoutePending phase."));
			return Result;
		}
		if (!IsDefaultSendingOffState(Attack.SendingOffRoute))
		{
			SetFailure(Result,
				EMatchPlayCurrentAttackRouteStateValidationErrorCode
					::UnexpectedSendingOffPayload,
				TEXT("Set Piece route cannot carry Sending-Off state."));
			return Result;
		}
		if (!HasDefaultOrdinaryPayload(Attack))
		{
			SetFailure(Result,
				EMatchPlayCurrentAttackRouteStateValidationErrorCode
					::UnexpectedOrdinaryPayload,
				TEXT("Set Piece route cannot start ordinary attack state."));
			return Result;
		}
		if (Attack.SetPieceRoute.Stage
			== EMatchPlaySetPieceRouteStage::AwaitingTypeRoll)
		{
			if (Attack.SetPieceRoute.bHasTypeRoll
				|| Attack.SetPieceRoute.RawTypeD6 != 0
				|| Attack.SetPieceRoute.SelectedType
					!= ESetPieceSelectedType::None)
			{
				SetFailure(Result,
					EMatchPlayCurrentAttackRouteStateValidationErrorCode
						::AwaitingTypeHasResolvedPayload,
					TEXT("AwaitingTypeRoll cannot carry a resolved Set Piece type."));
				return Result;
			}
		}
		else if (Attack.SetPieceRoute.Stage
			== EMatchPlaySetPieceRouteStage::TypeResolved)
		{
			if (!Attack.SetPieceRoute.bHasTypeRoll)
			{
				SetFailure(Result,
					EMatchPlayCurrentAttackRouteStateValidationErrorCode
						::TypeResolvedMissingRoll,
					TEXT("TypeResolved requires a stored Set Piece type roll."));
				return Result;
			}
			if (Attack.SetPieceRoute.RawTypeD6 < 1
				|| Attack.SetPieceRoute.RawTypeD6 > 6)
			{
				SetFailure(Result,
					EMatchPlayCurrentAttackRouteStateValidationErrorCode
						::InvalidSetPieceTypeRoll,
					TEXT("Resolved Set Piece type D6 must be in range [1, 6]."));
				return Result;
			}
			if (Attack.SetPieceRoute.SelectedType
				== ESetPieceSelectedType::None)
			{
				SetFailure(Result,
					EMatchPlayCurrentAttackRouteStateValidationErrorCode
						::InvalidSetPieceType,
					TEXT("TypeResolved requires a concrete Set Piece type."));
				return Result;
			}

			FSetPieceTypeSelectionQueryInput QueryInput;
			QueryInput.CurrentActionPoint = Attack.RawInitialD12;
			QueryInput.bHasExternalSelectionD6 = true;
			QueryInput.ExternalSelectionD6 = Attack.SetPieceRoute.RawTypeD6;
			const FSetPieceTypeSelectionQueryResult QueryResult =
				FSetPieceTypeSelectionQuery::Select(QueryInput);
			if (!QueryResult.bSuccess
				|| QueryResult.SelectedSetPieceType
					!= Attack.SetPieceRoute.SelectedType)
			{
				SetFailure(Result,
					EMatchPlayCurrentAttackRouteStateValidationErrorCode
						::SetPieceTypeMappingMismatch,
					TEXT("Stored Set Piece type must match the canonical type-roll mapping."));
				return Result;
			}
		}
		else
		{
			SetFailure(Result,
				EMatchPlayCurrentAttackRouteStateValidationErrorCode
					::InvalidSetPieceStage,
				TEXT("Set Piece route requires AwaitingTypeRoll or TypeResolved stage."));
			return Result;
		}
		break;

	case EMatchPlayCurrentAttackRouteKind::None:
	default:
		SetFailure(Result,
			EMatchPlayCurrentAttackRouteStateValidationErrorCode
				::InvalidRouteKind,
			TEXT("An active current attack requires an explicit route kind."));
		return Result;
	}

	Result.bIsCanonical = true;
	return Result;
}
