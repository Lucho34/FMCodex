#include "MatchPlaySetPieceTypeRoll.h"

namespace MatchPlaySetPieceTypeRoll
{
	void SetFailure(
		FMatchPlaySetPieceTypeRollResult& Result,
		const EMatchPlaySetPieceTypeRollErrorCode ErrorCode,
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
}

FMatchPlaySetPieceTypeRollResult FMatchPlaySetPieceTypeRoll::Resolve(
	const FMatchPlayState& BeforeState,
	const FMatchPlaySetPieceTypeRollRequest& Request,
	IMatchPlayAttackEntryRollProvider* RollProvider)
{
	using namespace MatchPlaySetPieceTypeRoll;

	FMatchPlaySetPieceTypeRollResult Result;
	Result.BeforeState = BeforeState;
	Result.AfterState = BeforeState;
	Result.Request = Request;

	if (!BeforeState.RuntimeState.bIsInitialized)
	{
		SetFailure(Result,
			EMatchPlaySetPieceTypeRollErrorCode::MatchPlayStateNotInitialized,
			TEXT("Match play state must be initialized before requesting a Set Piece type roll."));
		return Result;
	}
	if (!BeforeState.bHasCurrentAttack)
	{
		SetFailure(Result,
			EMatchPlaySetPieceTypeRollErrorCode::NoCurrentAttack,
			TEXT("A current Set Piece attack is required."));
		return Result;
	}
	if (Request.AttackSequence <= 0)
	{
		SetFailure(Result,
			EMatchPlaySetPieceTypeRollErrorCode::InvalidAttackSequence,
			TEXT("AttackSequence must be positive."));
		return Result;
	}
	if (Request.AttackSequence
		!= BeforeState.CurrentAttack.AttackSequence)
	{
		SetFailure(Result,
			EMatchPlaySetPieceTypeRollErrorCode::AttackSequenceMismatch,
			TEXT("AttackSequence does not match the current attack."));
		return Result;
	}
	if (BeforeState.CurrentAttack.RouteKind
		!= EMatchPlayCurrentAttackRouteKind::SetPiece)
	{
		SetFailure(Result,
			EMatchPlaySetPieceTypeRollErrorCode::WrongRoute,
			TEXT("Set Piece type roll requires the SetPiece route."));
		return Result;
	}
	if (BeforeState.CurrentAttack.SetPieceRoute.Stage
		!= EMatchPlaySetPieceRouteStage::AwaitingTypeRoll)
	{
		SetFailure(Result,
			EMatchPlaySetPieceTypeRollErrorCode::WrongStage,
			TEXT("Set Piece type roll is only valid while AwaitingTypeRoll."));
		return Result;
	}
	if (!IsPlayer(BeforeState.RuntimeState.CurrentAttackingPlayer))
	{
		SetFailure(Result,
			EMatchPlaySetPieceTypeRollErrorCode::InvalidCurrentAttackingPlayer,
			TEXT("CurrentAttackingPlayer must be PlayerA or PlayerB."));
		return Result;
	}
	if (!IsPlayer(Request.RequestingSide))
	{
		SetFailure(Result,
			EMatchPlaySetPieceTypeRollErrorCode::InvalidRequestingSide,
			TEXT("RequestingSide must be PlayerA or PlayerB."));
		return Result;
	}
	if (Request.RequestingSide
		!= BeforeState.RuntimeState.CurrentAttackingPlayer)
	{
		SetFailure(Result,
			EMatchPlaySetPieceTypeRollErrorCode
				::RequestingSideNotCurrentAttacker,
			TEXT("Only the current attacking player may request the Set Piece type roll."));
		return Result;
	}

	Result.RouteValidationResult =
		FMatchPlayCurrentAttackRouteStateValidator::Validate(BeforeState);
	if (!Result.RouteValidationResult.bIsCanonical)
	{
		SetFailure(Result,
			EMatchPlaySetPieceTypeRollErrorCode::InvalidRouteState,
			Result.RouteValidationResult.ErrorMessage);
		return Result;
	}
	if (RollProvider == nullptr)
	{
		SetFailure(Result,
			EMatchPlaySetPieceTypeRollErrorCode::ProviderUnavailable,
			TEXT("No authoritative attack-entry roll provider is configured."));
		return Result;
	}

	Result.ProviderResult = RollProvider->RollD6(
		EMatchPlayAttackEntryRollPurpose::SetPieceType);
	Result.ProviderValidationResult =
		FMatchPlayAttackEntryRollProviderResultValidator::Validate(
			EMatchPlayAttackEntryRollPurpose::SetPieceType,
			Result.ProviderResult);
	if (!Result.ProviderValidationResult.bIsCanonical)
	{
		const bool bProviderFailure =
			Result.ProviderValidationResult.ErrorCode
			== EMatchPlayAttackEntryRollProviderResultValidationErrorCode
				::ProviderFailure;
		SetFailure(Result,
			bProviderFailure
				? EMatchPlaySetPieceTypeRollErrorCode::ProviderFailure
				: EMatchPlaySetPieceTypeRollErrorCode::MalformedProviderResult,
			Result.ProviderValidationResult.ErrorMessage);
		return Result;
	}

	FSetPieceTypeSelectionQueryInput QueryInput;
	QueryInput.CurrentActionPoint =
		BeforeState.CurrentAttack.RawInitialD12;
	QueryInput.bHasExternalSelectionD6 = true;
	QueryInput.ExternalSelectionD6 = Result.ProviderResult.RawRoll;
	Result.SelectionResult = FSetPieceTypeSelectionQuery::Select(QueryInput);
	if (!Result.SelectionResult.bSuccess)
	{
		SetFailure(Result,
			EMatchPlaySetPieceTypeRollErrorCode::TypeSelectionFailed,
			Result.SelectionResult.ErrorMessage);
		return Result;
	}

	Result.AfterState.CurrentAttack.SetPieceRoute.Stage =
		EMatchPlaySetPieceRouteStage::TypeResolved;
	Result.AfterState.CurrentAttack.SetPieceRoute.bHasTypeRoll = true;
	Result.AfterState.CurrentAttack.SetPieceRoute.RawTypeD6 =
		Result.ProviderResult.RawRoll;
	Result.AfterState.CurrentAttack.SetPieceRoute.SelectedType =
		Result.SelectionResult.SelectedSetPieceType;
	switch (Result.SelectionResult.SelectedSetPieceType)
	{
	case ESetPieceSelectedType::ShortFreeKick:
		Result.AfterState.CurrentAttack.SetPieceRoute.ShortFreeKick.Stage =
			EMatchPlaySetPieceCarrierRouteStage::AwaitingCarrier;
		break;
	case ESetPieceSelectedType::LongFreeKick:
		Result.AfterState.CurrentAttack.SetPieceRoute.LongFreeKick.Stage =
			EMatchPlaySetPieceCarrierRouteStage::AwaitingCarrier;
		break;
	case ESetPieceSelectedType::Penalty:
		Result.AfterState.CurrentAttack.SetPieceRoute.Penalty.Stage =
			EMatchPlaySetPieceCarrierRouteStage::AwaitingCarrier;
		break;
	case ESetPieceSelectedType::Corner:
		Result.AfterState.CurrentAttack.SetPieceRoute.Corner.Stage =
			EMatchPlaySetPieceCornerRouteStage
				::AwaitingAttackerNominations;
		break;
	case ESetPieceSelectedType::None:
	default:
		Result.AfterState = BeforeState;
		SetFailure(Result,
			EMatchPlaySetPieceTypeRollErrorCode::TypeSelectionFailed,
			TEXT("Canonical Set Piece type selection did not produce a concrete type."));
		return Result;
	}
	Result.RouteValidationResult =
		FMatchPlayCurrentAttackRouteStateValidator::Validate(
			Result.AfterState);
	if (!Result.RouteValidationResult.bIsCanonical)
	{
		Result.AfterState = BeforeState;
		SetFailure(Result,
			EMatchPlaySetPieceTypeRollErrorCode::InvalidRouteState,
			Result.RouteValidationResult.ErrorMessage);
		return Result;
	}

	Result.bSuccess = true;
	Result.ErrorCode = EMatchPlaySetPieceTypeRollErrorCode::None;
	return Result;
}
