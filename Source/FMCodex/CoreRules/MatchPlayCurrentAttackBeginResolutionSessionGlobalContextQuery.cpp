#include "MatchPlayCurrentAttackBeginResolutionSessionGlobalContextQuery.h"

#include "MatchPlayBoundActionParticipantNormalizationQuery.h"

class FMatchPlayValidatedResolutionPreparationAccess final
{
public:
	static bool PopulateBinding(
		const FMatchPlayState& State,
		const int64 AttackSequence,
		const FMatchPlayCurrentAttackReadyValidationResult&
			ReadyValidationResult,
		FMatchPlayCurrentAttackResolutionBindingResult& OutResult)
	{
		return FMatchPlayCurrentAttackResolutionBinding
			::PopulateFromSuccessfulReadyValidation(
				State,
				AttackSequence,
				ReadyValidationResult,
				OutResult);
	}

	static FMatchPlayBoundActionParticipantNormalizationResult Normalize(
		const FMatchPlayState& State,
		const FMatchPlayBoundActionParticipantNormalizationRequest&
			Request,
		const FMatchPlayCurrentAttackResolutionBindingResult&
			BindingResult)
	{
		return FMatchPlayBoundActionParticipantNormalizationQuery
			::QueryFromSuccessfulBinding(
				State,
				Request,
				BindingResult);
	}
};

namespace MatchPlayCurrentAttackBeginResolutionSessionGlobalContextImplementation
{
	void SetFailure(
		FMatchPlayCurrentAttackBeginResolutionSessionGlobalContextResult&
			Result,
		const EMatchPlayCurrentAttackBeginResolutionSessionErrorCode
			ErrorCode,
		const FString& ErrorMessage)
	{
		Result.ErrorCode = ErrorCode;
		Result.ErrorMessage = ErrorMessage;
	}

	FMatchPlayCurrentAttackResolutionSessionParticipant CopyParticipant(
		const FMatchPlayBoundActionNormalizedParticipant& Source)
	{
		FMatchPlayCurrentAttackResolutionSessionParticipant Result;
		Result.bIsPresent = Source.bIsPresent;
		Result.Side = Source.Side;
		Result.CardId = Source.CardId;
		Result.Values = Source.Values;
		return Result;
	}

	FMatchPlayCurrentAttackResolutionSession BuildSession(
		const FMatchPlayBoundActionParticipantNormalizationResult&
			NormalizationResult)
	{
		const FMatchPlayBoundActionNormalizedParticipantBundle& Source =
			NormalizationResult.Bundle;
		FMatchPlayCurrentAttackResolutionSession Session;
		Session.AttackSequence = Source.AttackSequence;
		Session.Stage =
			EMatchPlayCurrentAttackResolutionStage::AwaitingRoute;
		Session.Bundle.Binding = Source.Binding;
		Session.Bundle.CurrentAttackingPlayer =
			Source.CurrentAttackingPlayer;
		Session.Bundle.CurrentDefendingPlayer =
			Source.CurrentDefendingPlayer;
		Session.Bundle.Carrier = CopyParticipant(Source.Carrier);
		Session.Bundle.Marker = CopyParticipant(Source.Marker);
		Session.Bundle.bHasRunner = Source.bHasRunner;
		Session.Bundle.Runner = CopyParticipant(Source.Runner);
		Session.Bundle.bHasHelper = Source.bHasHelper;
		Session.Bundle.Helper = CopyParticipant(Source.Helper);
		return Session;
	}
}

FMatchPlayCurrentAttackBeginResolutionSessionGlobalContextResult
FMatchPlayCurrentAttackBeginResolutionSessionGlobalContextQuery::Query(
	const FMatchPlayState& State,
	const FMatchPlayCurrentAttackBeginResolutionSessionRequest& Request)
{
	using namespace
		MatchPlayCurrentAttackBeginResolutionSessionGlobalContextImplementation;

	FMatchPlayCurrentAttackBeginResolutionSessionGlobalContextResult
		Result;
	Result.Request = Request;
	if (!State.RuntimeState.bIsInitialized)
	{
		SetFailure(
			Result,
			EMatchPlayCurrentAttackBeginResolutionSessionErrorCode
				::MatchPlayStateNotInitialized,
			TEXT("Match play state must be initialized before beginning a Resolution Session."));
		return Result;
	}
	if (!State.bHasCurrentAttack)
	{
		SetFailure(
			Result,
			EMatchPlayCurrentAttackBeginResolutionSessionErrorCode
				::NoCurrentAttack,
			TEXT("Beginning a Resolution Session requires an active current attack."));
		return Result;
	}

	const FMatchPlayCurrentAttackState& CurrentAttack =
		State.CurrentAttack;
	if (CurrentAttack.AttackSequence <= 0)
	{
		SetFailure(
			Result,
			EMatchPlayCurrentAttackBeginResolutionSessionErrorCode
				::InvalidCurrentAttackSequence,
			TEXT("Current attack sequence must be greater than zero."));
		return Result;
	}
	if (Request.AttackSequence <= 0)
	{
		SetFailure(
			Result,
			EMatchPlayCurrentAttackBeginResolutionSessionErrorCode
				::InvalidRequestedAttackSequence,
			TEXT("Requested attack sequence must be greater than zero."));
		return Result;
	}
	if (Request.AttackSequence != CurrentAttack.AttackSequence)
	{
		SetFailure(
			Result,
			EMatchPlayCurrentAttackBeginResolutionSessionErrorCode
				::AttackSequenceMismatch,
			TEXT("Requested attack sequence does not match CurrentAttack."));
		return Result;
	}
	if (CurrentAttack.Phase != EMatchPlayCurrentAttackPhase::Resolution)
	{
		SetFailure(
			Result,
			EMatchPlayCurrentAttackBeginResolutionSessionErrorCode
				::CurrentAttackNotInResolution,
			TEXT("CurrentAttack must be in Resolution phase."));
		return Result;
	}

	Result.SessionStateValidationResult =
		FMatchPlayCurrentAttackResolutionSessionStateValidator::Validate(
			State);
	if (!Result.SessionStateValidationResult.bIsCanonical)
	{
		SetFailure(
			Result,
			EMatchPlayCurrentAttackBeginResolutionSessionErrorCode
				::InvalidExistingSessionState,
			Result.SessionStateValidationResult.ErrorMessage);
		return Result;
	}
	if (CurrentAttack.bHasResolutionSession)
	{
		Result.bSuccess = true;
		Result.bSessionAlreadyExists = true;
		Result.Session = CurrentAttack.ResolutionSession;
		return Result;
	}

	if (CurrentAttack.SelectionStage
		!= EMatchPlayCurrentAttackSelectionStage::ReadyForResolution)
	{
		SetFailure(
			Result,
			EMatchPlayCurrentAttackBeginResolutionSessionErrorCode
				::WrongSelectionStage,
			TEXT("First Resolution Session Begin requires ReadyForResolution."));
		return Result;
	}

	Result.ReadyValidationResult =
		FMatchPlayCurrentAttackReadyForResolutionValidator::Validate(
			State);
	if (!Result.ReadyValidationResult.bSuccess)
	{
		SetFailure(
			Result,
			EMatchPlayCurrentAttackBeginResolutionSessionErrorCode
				::ReadyValidationFailed,
			Result.ReadyValidationResult.ErrorMessage);
		return Result;
	}

	if (!FMatchPlayValidatedResolutionPreparationAccess::PopulateBinding(
			State,
			Request.AttackSequence,
			Result.ReadyValidationResult,
			Result.BindingResult))
	{
		SetFailure(
			Result,
			EMatchPlayCurrentAttackBeginResolutionSessionErrorCode
				::BindingFailed,
			Result.BindingResult.ErrorMessage);
		return Result;
	}

	FMatchPlayBoundActionParticipantNormalizationRequest
		NormalizationRequest;
	NormalizationRequest.AttackSequence = Request.AttackSequence;
	Result.NormalizationResult =
		FMatchPlayValidatedResolutionPreparationAccess::Normalize(
			State,
			NormalizationRequest,
			Result.BindingResult);
	if (!Result.NormalizationResult.bSuccess)
	{
		SetFailure(
			Result,
			EMatchPlayCurrentAttackBeginResolutionSessionErrorCode
				::NormalizationFailed,
			Result.NormalizationResult.ErrorMessage);
		return Result;
	}

	Result.Session = BuildSession(Result.NormalizationResult);
	Result.SessionStateValidationResult =
		FMatchPlayCurrentAttackResolutionSessionStateValidator::Validate(
			State,
			&Result.Session);
	if (!Result.SessionStateValidationResult.bIsCanonical)
	{
		SetFailure(
			Result,
			EMatchPlayCurrentAttackBeginResolutionSessionErrorCode
				::InvalidCanonicalBundle,
			Result.SessionStateValidationResult.ErrorMessage);
		return Result;
	}

	Result.bSuccess = true;
	return Result;
}
