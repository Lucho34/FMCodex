#include "MatchPlayFullD12Entry.h"

namespace MatchPlayFullD12Entry
{
	void SetFailure(
		FMatchPlayFullD12EntryResult& Result,
		const EMatchPlayFullD12EntryErrorCode ErrorCode,
		const FString& ErrorMessage)
	{
		Result.ErrorCode = ErrorCode;
		Result.ErrorMessage = ErrorMessage;
	}

	bool IsPlayer(const EInitialTurnOrderPlayer Player)
	{
		return Player == EInitialTurnOrderPlayer::PlayerA
			|| Player == EInitialTurnOrderPlayer::PlayerB;
	}

	bool IsAttackCountStateValid(const FPlayerRuntimeState& PlayerState)
	{
		return PlayerState.TotalAttackCount >= 0
			&& PlayerState.UsedAttackCount >= 0
			&& PlayerState.UsedAttackCount <= PlayerState.TotalAttackCount;
	}

	int32 GetRemainingAttackCount(const FPlayerRuntimeState& PlayerState)
	{
		return PlayerState.TotalAttackCount - PlayerState.UsedAttackCount;
	}

	FMatchPlayCurrentAttackState MakePendingRouteAttack(
		const int64 AttackSequence,
		const int32 RawD12,
		const EMatchPlayCurrentAttackRouteKind RouteKind)
	{
		FMatchPlayCurrentAttackState Attack;
		Attack.Phase = EMatchPlayCurrentAttackPhase::RoutePending;
		Attack.AttackSequence = AttackSequence;
		Attack.ActionPoint = RawD12;
		Attack.RawInitialD12 = RawD12;
		Attack.RouteKind = RouteKind;
		return Attack;
	}
}

FMatchPlayFullD12EntryResult FMatchPlayFullD12Entry::Enter(
	const FMatchPlayState& BeforeState,
	const FMatchPlayFullD12EntryRequest& Request,
	IMatchPlayAttackEntryRollProvider* RollProvider)
{
	using namespace MatchPlayFullD12Entry;

	FMatchPlayFullD12EntryResult Result;
	Result.BeforeState = BeforeState;
	Result.AfterState = BeforeState;
	Result.Request = Request;

	const FMatchRuntimeState& RuntimeState = BeforeState.RuntimeState;
	if (!RuntimeState.bIsInitialized)
	{
		SetFailure(Result,
			EMatchPlayFullD12EntryErrorCode::MatchPlayStateNotInitialized,
			TEXT("Match play state must be initialized before requesting the initial action-point roll."));
		return Result;
	}
	if (BeforeState.bHasCurrentAttack)
	{
		SetFailure(Result,
			EMatchPlayFullD12EntryErrorCode::CurrentAttackAlreadyActive,
			TEXT("Cannot request an initial action-point roll while a current attack exists."));
		return Result;
	}
	if (!IsAttackCountStateValid(RuntimeState.PlayerAState))
	{
		SetFailure(Result,
			EMatchPlayFullD12EntryErrorCode::InvalidPlayerAAttackCountState,
			TEXT("PlayerA attack counts must satisfy 0 <= used <= total."));
		return Result;
	}
	if (!IsAttackCountStateValid(RuntimeState.PlayerBState))
	{
		SetFailure(Result,
			EMatchPlayFullD12EntryErrorCode::InvalidPlayerBAttackCountState,
			TEXT("PlayerB attack counts must satisfy 0 <= used <= total."));
		return Result;
	}

	const int32 PlayerARemaining =
		GetRemainingAttackCount(RuntimeState.PlayerAState);
	const int32 PlayerBRemaining =
		GetRemainingAttackCount(RuntimeState.PlayerBState);
	if (PlayerARemaining == 0 && PlayerBRemaining == 0)
	{
		SetFailure(Result,
			EMatchPlayFullD12EntryErrorCode::MatchAlreadyEnded,
			TEXT("Cannot request an initial action-point roll after all attack opportunities are exhausted."));
		return Result;
	}
	if (!IsPlayer(RuntimeState.CurrentAttackingPlayer))
	{
		SetFailure(Result,
			EMatchPlayFullD12EntryErrorCode::InvalidCurrentAttackingPlayer,
			TEXT("CurrentAttackingPlayer must be PlayerA or PlayerB."));
		return Result;
	}
	if (!IsPlayer(Request.RequestingSide))
	{
		SetFailure(Result,
			EMatchPlayFullD12EntryErrorCode::InvalidRequestingSide,
			TEXT("RequestingSide must be PlayerA or PlayerB."));
		return Result;
	}
	if (Request.RequestingSide != RuntimeState.CurrentAttackingPlayer)
	{
		SetFailure(Result,
			EMatchPlayFullD12EntryErrorCode::RequestingSideNotCurrentAttacker,
			TEXT("Only the current attacking player may request the initial action-point roll."));
		return Result;
	}

	const int32 CurrentAttackerRemaining =
		RuntimeState.CurrentAttackingPlayer == EInitialTurnOrderPlayer::PlayerA
			? PlayerARemaining
			: PlayerBRemaining;
	if (CurrentAttackerRemaining == 0)
	{
		SetFailure(Result,
			EMatchPlayFullD12EntryErrorCode
				::CurrentAttackerHasNoRemainingAttackOpportunity,
			TEXT("Current attacker has no remaining attack opportunity."));
		return Result;
	}

	Result.AuthoritativeAttackSequence =
		static_cast<int64>(RuntimeState.PlayerAState.UsedAttackCount)
		+ static_cast<int64>(RuntimeState.PlayerBState.UsedAttackCount)
		+ 1;
	if (Result.AuthoritativeAttackSequence <= 0)
	{
		SetFailure(Result,
			EMatchPlayFullD12EntryErrorCode::InvalidDerivedAttackSequence,
			TEXT("Authority could not derive a positive AttackSequence."));
		return Result;
	}
	if (Request.ExpectedAttackSequence <= 0)
	{
		SetFailure(Result,
			EMatchPlayFullD12EntryErrorCode::InvalidExpectedAttackSequence,
			TEXT("ExpectedAttackSequence must be positive."));
		return Result;
	}
	if (Request.ExpectedAttackSequence
		!= Result.AuthoritativeAttackSequence)
	{
		SetFailure(Result,
			EMatchPlayFullD12EntryErrorCode::AttackSequenceMismatch,
			TEXT("ExpectedAttackSequence does not match the authoritative next attack sequence."));
		return Result;
	}
	if (RollProvider == nullptr)
	{
		SetFailure(Result,
			EMatchPlayFullD12EntryErrorCode::ProviderUnavailable,
			TEXT("No authoritative attack-entry roll provider is configured."));
		return Result;
	}

	Result.ProviderResult = RollProvider->RollD12(
		EMatchPlayAttackEntryRollPurpose::InitialActionPoint);
	Result.ProviderValidationResult =
		FMatchPlayAttackEntryRollProviderResultValidator::Validate(
			EMatchPlayAttackEntryRollPurpose::InitialActionPoint,
			Result.ProviderResult);
	if (!Result.ProviderValidationResult.bIsCanonical)
	{
		const bool bProviderFailure =
			Result.ProviderValidationResult.ErrorCode
			== EMatchPlayAttackEntryRollProviderResultValidationErrorCode
				::ProviderFailure;
		SetFailure(Result,
			bProviderFailure
				? EMatchPlayFullD12EntryErrorCode::ProviderFailure
				: EMatchPlayFullD12EntryErrorCode::MalformedProviderResult,
			Result.ProviderValidationResult.ErrorMessage);
		return Result;
	}

	const int32 RawD12 = Result.ProviderResult.RawRoll;
	if (RawD12 >= 2 && RawD12 <= 8)
	{
		Result.OrdinaryBeginResult = FMatchPlayBeginOrdinaryAttack::Begin(
			BeforeState,
			RawD12);
		if (!Result.OrdinaryBeginResult.bSuccess)
		{
			SetFailure(Result,
				EMatchPlayFullD12EntryErrorCode::OrdinaryBeginFailed,
				Result.OrdinaryBeginResult.ErrorMessage);
			return Result;
		}
		Result.AfterState = Result.OrdinaryBeginResult.AfterState;
	}
	else
	{
		FMatchPlayCurrentAttackState Attack = MakePendingRouteAttack(
			Result.AuthoritativeAttackSequence,
			RawD12,
			RawD12 == 1
				? EMatchPlayCurrentAttackRouteKind::SendingOff
				: EMatchPlayCurrentAttackRouteKind::SetPiece);
		if (RawD12 == 1)
		{
			Attack.SendingOffRoute.Stage =
				EMatchPlaySendingOffRouteStage::AwaitingResolution;
		}
		else
		{
			Attack.SetPieceRoute.Stage =
				EMatchPlaySetPieceRouteStage::AwaitingTypeRoll;
		}
		Result.AfterState.CurrentAttack = MoveTemp(Attack);
		Result.AfterState.bHasCurrentAttack = true;
	}

	Result.RouteValidationResult =
		FMatchPlayCurrentAttackRouteStateValidator::Validate(
			Result.AfterState);
	if (!Result.RouteValidationResult.bIsCanonical)
	{
		Result.AfterState = BeforeState;
		SetFailure(Result,
			EMatchPlayFullD12EntryErrorCode::InvalidRouteState,
			Result.RouteValidationResult.ErrorMessage);
		return Result;
	}

	Result.bSuccess = true;
	Result.ErrorCode = EMatchPlayFullD12EntryErrorCode::None;
	return Result;
}
