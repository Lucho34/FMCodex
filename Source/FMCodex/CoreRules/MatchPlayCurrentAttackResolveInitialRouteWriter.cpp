#include "MatchPlayCurrentAttackResolveInitialRouteWriter.h"

namespace MatchPlayCurrentAttackResolveInitialRouteWriterImplementation
{
	void SetFailure(
		FMatchPlayCurrentAttackResolveInitialRouteWriterResult& Result,
		const EMatchPlayCurrentAttackResolveInitialRouteWriterErrorCode
			ErrorCode,
		const EMatchPlayCurrentAttackResolveInitialRouteFailureDisposition
			FailureDisposition,
		const FString& ErrorMessage)
	{
		Result.ErrorCode = ErrorCode;
		Result.FailureDisposition = FailureDisposition;
		Result.ErrorMessage = ErrorMessage;
	}

	bool IsCanonicalProviderFailure(
		const FMatchPlayInitialRouteRollProviderResult& ProviderResult)
	{
		return !ProviderResult.bSuccess
			&& ProviderResult.RawD6 == 0
			&& ProviderResult.ErrorCode
				== EMatchPlayInitialRouteRollProviderErrorCode
					::ProviderFailure
			&& !ProviderResult.ErrorMessage.IsEmpty();
	}

	bool IsCanonicalProviderSuccessShape(
		const FMatchPlayInitialRouteRollProviderResult& ProviderResult)
	{
		return ProviderResult.bSuccess
			&& ProviderResult.ErrorCode
				== EMatchPlayInitialRouteRollProviderErrorCode::None
			&& ProviderResult.ErrorMessage.IsEmpty();
	}
}

FMatchPlayCurrentAttackResolveInitialRouteWriterResult
FMatchPlayCurrentAttackResolveInitialRouteWriter::Resolve(
	const FMatchPlayState& BeforeState,
	const FMatchPlayCurrentAttackResolveInitialRouteRequest& Request,
	IMatchPlayInitialRouteRollProvider* RollProvider)
{
	using namespace
		MatchPlayCurrentAttackResolveInitialRouteWriterImplementation;

	FMatchPlayCurrentAttackResolveInitialRouteWriterResult Result;
	Result.Request = Request;
	Result.BeforeState = BeforeState;
	Result.AfterState = BeforeState;
	Result.LegalityResult =
		FMatchPlayCurrentAttackResolveInitialRouteLegalityQuery::Query(
			BeforeState,
			Request);
	Result.GlobalContextResult =
		Result.LegalityResult.GlobalContextResult;
	if (!Result.LegalityResult.bIsLegal)
	{
		const bool bGlobalFailed =
			!Result.GlobalContextResult.bSuccess;
		SetFailure(
			Result,
			bGlobalFailed
				? EMatchPlayCurrentAttackResolveInitialRouteWriterErrorCode
					::GlobalContextFailed
				: EMatchPlayCurrentAttackResolveInitialRouteWriterErrorCode
					::LegalityFailed,
			EMatchPlayCurrentAttackResolveInitialRouteFailureDisposition
				::None,
			Result.LegalityResult.ErrorMessage);
		return Result;
	}

	if (Result.GlobalContextResult.bIsCanonicalDuplicate)
	{
		Result.ActualBranch =
			Result.GlobalContextResult.ExistingActualBranch;
		Result.InitialRouteRollRecords =
			Result.GlobalContextResult.ExistingInitialRouteRollRecords;
		Result.bSuccess = true;
		return Result;
	}

	FMatchPlayCurrentAttackInitialRouteMappingInput MappingInput;
	MappingInput.ActionType = Result.GlobalContextResult.ActionType;
	MappingInput.Intent = Result.GlobalContextResult.Intent;
	if (Result.GlobalContextResult.bRequiresInitialRouteD6)
	{
		if (RollProvider == nullptr)
		{
			SetFailure(
				Result,
				EMatchPlayCurrentAttackResolveInitialRouteWriterErrorCode
					::RngProviderUnavailable,
				EMatchPlayCurrentAttackResolveInitialRouteFailureDisposition
					::RetryableExecutionFailure,
				TEXT("Initial Route D6 action requires a roll provider."));
			return Result;
		}

		Result.bProviderCalled = true;
		Result.ProviderResult = RollProvider->RollD6(
			Result.GlobalContextResult.ExpectedRollPurpose);
		if (!Result.ProviderResult.bSuccess)
		{
			if (IsCanonicalProviderFailure(Result.ProviderResult))
			{
				SetFailure(
					Result,
					EMatchPlayCurrentAttackResolveInitialRouteWriterErrorCode
						::RngProviderFailed,
					EMatchPlayCurrentAttackResolveInitialRouteFailureDisposition
						::RetryableExecutionFailure,
					Result.ProviderResult.ErrorMessage);
				return Result;
			}
			SetFailure(
				Result,
				EMatchPlayCurrentAttackResolveInitialRouteWriterErrorCode
					::InvalidRngResult,
				EMatchPlayCurrentAttackResolveInitialRouteFailureDisposition
					::NonRetryableExecutionFailure,
				TEXT("Initial Route roll provider returned a malformed failure result."));
			return Result;
		}
		if (!IsCanonicalProviderSuccessShape(Result.ProviderResult))
		{
			SetFailure(
				Result,
				EMatchPlayCurrentAttackResolveInitialRouteWriterErrorCode
					::InvalidRngResult,
				EMatchPlayCurrentAttackResolveInitialRouteFailureDisposition
					::NonRetryableExecutionFailure,
				TEXT("Initial Route roll provider returned a malformed success result."));
			return Result;
		}
		if (Result.ProviderResult.RawD6 < 1
			|| Result.ProviderResult.RawD6 > 6)
		{
			SetFailure(
				Result,
				EMatchPlayCurrentAttackResolveInitialRouteWriterErrorCode
					::InvalidRngResult,
				EMatchPlayCurrentAttackResolveInitialRouteFailureDisposition
					::NonRetryableExecutionFailure,
				TEXT("Initial Route roll provider RawD6 must be in range [1, 6]."));
			return Result;
		}

		MappingInput.bHasInitialRouteD6 = true;
		MappingInput.InitialRouteD6 = Result.ProviderResult.RawD6;
	}

	Result.MappingResult =
		FMatchPlayCurrentAttackInitialRouteMappingQuery::Map(MappingInput);
	if (!Result.MappingResult.bSuccess)
	{
		SetFailure(
			Result,
			EMatchPlayCurrentAttackResolveInitialRouteWriterErrorCode
				::InitialRouteMappingInvariantViolation,
			EMatchPlayCurrentAttackResolveInitialRouteFailureDisposition
				::NonRetryableInvariantFailure,
			Result.MappingResult.ErrorMessage);
		return Result;
	}

	FMatchPlayState WorkingState = BeforeState;
	FMatchPlayCurrentAttackResolutionSession& WorkingSession =
		WorkingState.CurrentAttack.ResolutionSession;
	WorkingSession.bHasActualBranch = true;
	WorkingSession.ActualBranch = Result.MappingResult.ActualBranch;
	if (Result.GlobalContextResult.bRequiresInitialRouteD6)
	{
		FMatchPlayCurrentAttackResolutionRollRecord Record;
		Record.Purpose = Result.GlobalContextResult.ExpectedRollPurpose;
		Record.RawD6 = Result.ProviderResult.RawD6;
		WorkingSession.InitialRouteRollRecords.Add(Record);
	}
	WorkingSession.Stage =
		EMatchPlayCurrentAttackResolutionStage::RouteResolved;

	Result.CandidateValidationResult =
		FMatchPlayCurrentAttackResolutionSessionStateValidator::Validate(
			WorkingState);
	if (!Result.CandidateValidationResult.bIsCanonical)
	{
		SetFailure(
			Result,
			EMatchPlayCurrentAttackResolveInitialRouteWriterErrorCode
				::CandidateInvariantViolation,
			EMatchPlayCurrentAttackResolveInitialRouteFailureDisposition
				::NonRetryableInvariantFailure,
			Result.CandidateValidationResult.ErrorMessage);
		return Result;
	}

	Result.ActualBranch = WorkingSession.ActualBranch;
	Result.InitialRouteRollRecords =
		WorkingSession.InitialRouteRollRecords;
	Result.AfterState = MoveTemp(WorkingState);
	Result.bSuccess = true;
	Result.bResolvedNewRoute = true;
	return Result;
}
