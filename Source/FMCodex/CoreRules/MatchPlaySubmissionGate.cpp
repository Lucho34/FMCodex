#include "MatchPlaySubmissionGate.h"

namespace
{
	void SetFailure(
		FMatchPlaySubmissionGateResult& Result,
		const EMatchPlaySubmissionGateCode Code,
		const FString& ErrorMessage)
	{
		Result.bCanSubmit = false;
		Result.Code = Code;
		Result.Reason = ErrorMessage;
		Result.ErrorMessage = ErrorMessage;
	}

	EMatchPlaySubmissionGateCode MapReadinessCode(
		const EMatchPlayLoopReadinessCode Code)
	{
		switch (Code)
		{
		case EMatchPlayLoopReadinessCode::ReadyForExternalAttackRequest:
			return EMatchPlaySubmissionGateCode::CanSubmit;
		case EMatchPlayLoopReadinessCode::NoRemainingAttackOpportunities:
			return EMatchPlaySubmissionGateCode::MatchAlreadyFinished;
		case EMatchPlayLoopReadinessCode
			::CurrentAttackerHasNoRemainingAttackOpportunity:
			return EMatchPlaySubmissionGateCode
				::NoRemainingAttackOpportunity;
		case EMatchPlayLoopReadinessCode::MatchStateNotInitialized:
		case EMatchPlayLoopReadinessCode::CurrentAttackerHasNoAvailableCards:
		case EMatchPlayLoopReadinessCode::None:
		default:
			return EMatchPlaySubmissionGateCode
				::CannotAcceptAttackRequestNow;
		}
	}

	EMatchPlaySubmissionGateCode MapValidationCode(
		const EMatchPlayRequestValidationCode Code)
	{
		switch (Code)
		{
		case EMatchPlayRequestValidationCode::Valid:
			return EMatchPlaySubmissionGateCode::CanSubmit;
		case EMatchPlayRequestValidationCode::MatchAlreadyFinished:
			return EMatchPlaySubmissionGateCode::MatchAlreadyFinished;
		case EMatchPlayRequestValidationCode
			::CannotAcceptAttackRequestNow:
			return EMatchPlaySubmissionGateCode
				::CannotAcceptAttackRequestNow;
		case EMatchPlayRequestValidationCode::InvalidAttackingPlayer:
			return EMatchPlaySubmissionGateCode::InvalidAttackingPlayer;
		case EMatchPlayRequestValidationCode::NotCurrentAttacker:
			return EMatchPlaySubmissionGateCode::NotCurrentAttacker;
		case EMatchPlayRequestValidationCode::EmptyCardId:
			return EMatchPlaySubmissionGateCode::EmptyCardId;
		case EMatchPlayRequestValidationCode::CardNotAvailable:
			return EMatchPlaySubmissionGateCode::CardNotAvailable;
		case EMatchPlayRequestValidationCode::CardAlreadyUsed:
			return EMatchPlaySubmissionGateCode::CardAlreadyUsed;
		case EMatchPlayRequestValidationCode
			::NoRemainingAttackOpportunity:
			return EMatchPlaySubmissionGateCode
				::NoRemainingAttackOpportunity;
		case EMatchPlayRequestValidationCode::InvalidRequest:
		default:
			return EMatchPlaySubmissionGateCode::InvalidRequest;
		}
	}
}

FMatchPlaySubmissionGateResult FMatchPlaySubmissionGate::CanSubmit(
	const FMatchPlayState& State,
	const FMatchPlayAttackRequest& Request)
{
	FMatchPlaySubmissionGateResult Result;
	Result.LoopReadinessResult = FMatchPlayLoopReadiness::Evaluate(State);

	if (!Result.LoopReadinessResult.bCanAcceptExternalAttackRequest)
	{
		SetFailure(
			Result,
			MapReadinessCode(Result.LoopReadinessResult.Code),
			Result.LoopReadinessResult.ErrorMessage);
		return Result;
	}

	Result.RequestValidationReport =
		FMatchPlayRequestValidationReportQuery::Validate(State, Request);
	if (!Result.RequestValidationReport.bCanSubmitAttackRequest)
	{
		SetFailure(
			Result,
			MapValidationCode(Result.RequestValidationReport.Code),
			Result.RequestValidationReport.ErrorMessage);
		return Result;
	}

	Result.bCanSubmit = true;
	Result.Code = EMatchPlaySubmissionGateCode::CanSubmit;
	Result.Reason =
		TEXT("External attack request can be submitted.");
	Result.ErrorMessage.Reset();
	return Result;
}
