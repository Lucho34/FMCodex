#include "MatchPlayAttackRequest.h"

namespace MatchPlayAttackRequest
{
	bool IsPlayer(const EInitialTurnOrderPlayer PlayerSide)
	{
		return PlayerSide == EInitialTurnOrderPlayer::PlayerA
			|| PlayerSide == EInitialTurnOrderPlayer::PlayerB;
	}

	bool IsAvailabilityValidationFailure(
		const EMatchPlayAvailabilityErrorCode ErrorCode)
	{
		return ErrorCode
				== EMatchPlayAvailabilityErrorCode::InvalidRuntimeState
			|| ErrorCode
				== EMatchPlayAvailabilityErrorCode::InvalidCardUsageState
			|| ErrorCode
				== EMatchPlayAvailabilityErrorCode::CardValidationFailed;
	}

	void SetError(
		FMatchPlayAttackRequestValidationResult& Result,
		const EMatchPlayAttackRequestValidationErrorCode ErrorCode,
		const FString& ErrorMessage)
	{
		Result.ErrorCode = ErrorCode;
		Result.ErrorMessage = ErrorMessage;
	}
}

FMatchPlayAttackRequestValidationResult
FMatchPlayAttackRequestValidator::ValidateRequest(
	const FMatchPlayState& MatchPlayState,
	const FMatchPlayAttackRequest& AttackRequest)
{
	FMatchPlayAttackRequestValidationResult Result;
	Result.AttackRequest = AttackRequest;
	Result.bHasExternalFormulaInput =
		AttackRequest.bHasExternalFormulaInput;

	if (!MatchPlayAttackRequest::IsPlayer(AttackRequest.PlayerSide))
	{
		MatchPlayAttackRequest::SetError(
			Result,
			EMatchPlayAttackRequestValidationErrorCode
				::InvalidRequestPlayer,
			TEXT("Attack request player must be PlayerA or PlayerB."));
		return Result;
	}

	if (AttackRequest.CardId.IsNone())
	{
		MatchPlayAttackRequest::SetError(
			Result,
			EMatchPlayAttackRequestValidationErrorCode::InvalidCardId,
			TEXT("Attack request CardId cannot be None."));
		return Result;
	}

	if (!AttackRequest.bHasExternalFormulaInput)
	{
		MatchPlayAttackRequest::SetError(
			Result,
			EMatchPlayAttackRequestValidationErrorCode
				::MissingFormulaInput,
			TEXT("Attack request must include external formula input."));
		return Result;
	}

	Result.ActionPreviewResult =
		FMatchPlayActionPreview::PreviewAction(
			MatchPlayState,
			AttackRequest.PlayerSide,
			AttackRequest.CardId);
	Result.AvailabilityResult =
		Result.ActionPreviewResult.AvailabilityResult;
	Result.bCanPlay = Result.AvailabilityResult.bCanPlay;

	if (!Result.bCanPlay)
	{
		const EMatchPlayAttackRequestValidationErrorCode ErrorCode =
			MatchPlayAttackRequest::IsAvailabilityValidationFailure(
				Result.AvailabilityResult.ErrorCode)
				? EMatchPlayAttackRequestValidationErrorCode
					::AvailabilityValidationFailed
				: EMatchPlayAttackRequestValidationErrorCode
					::CardNotPlayable;
		MatchPlayAttackRequest::SetError(
			Result,
			ErrorCode,
			Result.AvailabilityResult.ErrorMessage);
		return Result;
	}

	Result.bValid = true;
	return Result;
}
