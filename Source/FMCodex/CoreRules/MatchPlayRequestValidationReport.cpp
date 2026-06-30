#include "MatchPlayRequestValidationReport.h"

namespace
{
	void SetFailure(
		FMatchPlayRequestValidationReport& Report,
		const EMatchPlayRequestValidationCode Code,
		const FString& ErrorMessage)
	{
		Report.bIsValid = false;
		Report.bCanSubmitAttackRequest = false;
		Report.Code = Code;
		Report.Reason = ErrorMessage;
		Report.ErrorMessage = ErrorMessage;
	}

	EMatchPlayRequestValidationCode MapLoopReadinessCode(
		const EMatchPlayLoopReadinessCode Code)
	{
		switch (Code)
		{
		case EMatchPlayLoopReadinessCode::ReadyForExternalAttackRequest:
			return EMatchPlayRequestValidationCode::Valid;
		case EMatchPlayLoopReadinessCode::NoRemainingAttackOpportunities:
			return EMatchPlayRequestValidationCode::MatchAlreadyFinished;
		case EMatchPlayLoopReadinessCode
			::CurrentAttackerHasNoRemainingAttackOpportunity:
			return EMatchPlayRequestValidationCode
				::NoRemainingAttackOpportunity;
		case EMatchPlayLoopReadinessCode::MatchStateNotInitialized:
		case EMatchPlayLoopReadinessCode::CurrentAttackerHasNoAvailableCards:
		case EMatchPlayLoopReadinessCode::None:
		default:
			return EMatchPlayRequestValidationCode
				::CannotAcceptAttackRequestNow;
		}
	}

	EMatchPlayRequestValidationCode MapAvailabilityCode(
		const EMatchPlayAvailabilityErrorCode Code)
	{
		switch (Code)
		{
		case EMatchPlayAvailabilityErrorCode::InvalidPlayer:
			return EMatchPlayRequestValidationCode::InvalidAttackingPlayer;
		case EMatchPlayAvailabilityErrorCode::NotCurrentAttacker:
			return EMatchPlayRequestValidationCode::NotCurrentAttacker;
		case EMatchPlayAvailabilityErrorCode::NoRemainingAttackOpportunity:
			return EMatchPlayRequestValidationCode
				::NoRemainingAttackOpportunity;
		case EMatchPlayAvailabilityErrorCode::InvalidCardId:
			return EMatchPlayRequestValidationCode::EmptyCardId;
		case EMatchPlayAvailabilityErrorCode::CardNotAvailable:
			return EMatchPlayRequestValidationCode::CardNotAvailable;
		case EMatchPlayAvailabilityErrorCode::CardAlreadyUsed:
			return EMatchPlayRequestValidationCode::CardAlreadyUsed;
		case EMatchPlayAvailabilityErrorCode
			::MatchAlreadyEndedOrNoAttackRemaining:
			return EMatchPlayRequestValidationCode::MatchAlreadyFinished;
		case EMatchPlayAvailabilityErrorCode::InvalidRuntimeState:
		case EMatchPlayAvailabilityErrorCode::InvalidCardUsageState:
		case EMatchPlayAvailabilityErrorCode::CardValidationFailed:
		case EMatchPlayAvailabilityErrorCode::None:
		default:
			return EMatchPlayRequestValidationCode::InvalidRequest;
		}
	}

	EMatchPlayRequestValidationCode MapRequestValidationCode(
		const FMatchPlayAttackRequestValidationResult& Result)
	{
		switch (Result.ErrorCode)
		{
		case EMatchPlayAttackRequestValidationErrorCode::InvalidRequestPlayer:
			return EMatchPlayRequestValidationCode::InvalidAttackingPlayer;
		case EMatchPlayAttackRequestValidationErrorCode::InvalidCardId:
			return EMatchPlayRequestValidationCode::EmptyCardId;
		case EMatchPlayAttackRequestValidationErrorCode::CardNotPlayable:
		case EMatchPlayAttackRequestValidationErrorCode
			::AvailabilityValidationFailed:
			return MapAvailabilityCode(Result.AvailabilityResult.ErrorCode);
		case EMatchPlayAttackRequestValidationErrorCode::MissingFormulaInput:
		case EMatchPlayAttackRequestValidationErrorCode::None:
		default:
			return EMatchPlayRequestValidationCode::InvalidRequest;
		}
	}
}

FMatchPlayRequestValidationReport
FMatchPlayRequestValidationReportQuery::Validate(
	const FMatchPlayState& State,
	const FMatchPlayAttackRequest& Request)
{
	FMatchPlayRequestValidationReport Report;
	Report.LoopReadinessResult = FMatchPlayLoopReadiness::Evaluate(State);
	Report.RequestValidationResult =
		FMatchPlayAttackRequestValidator::ValidateRequest(State, Request);

	const EMatchPlayRequestValidationCode ReadinessCode =
		MapLoopReadinessCode(Report.LoopReadinessResult.Code);
	if (ReadinessCode
			== EMatchPlayRequestValidationCode::MatchAlreadyFinished
		|| ReadinessCode
			== EMatchPlayRequestValidationCode
				::NoRemainingAttackOpportunity
		|| Report.LoopReadinessResult.Code
			== EMatchPlayLoopReadinessCode::MatchStateNotInitialized)
	{
		SetFailure(
			Report,
			ReadinessCode,
			Report.LoopReadinessResult.ErrorMessage);
		return Report;
	}

	if (!Report.RequestValidationResult.bValid)
	{
		SetFailure(
			Report,
			MapRequestValidationCode(Report.RequestValidationResult),
			Report.RequestValidationResult.ErrorMessage);
		return Report;
	}

	if (!Report.LoopReadinessResult.bCanAcceptExternalAttackRequest)
	{
		SetFailure(
			Report,
			ReadinessCode,
			Report.LoopReadinessResult.ErrorMessage);
		return Report;
	}

	Report.bIsValid = true;
	Report.bCanSubmitAttackRequest = true;
	Report.Code = EMatchPlayRequestValidationCode::Valid;
	Report.Reason =
		TEXT("Attack request can be submitted to the external attack flow.");
	Report.ErrorMessage.Reset();
	return Report;
}
