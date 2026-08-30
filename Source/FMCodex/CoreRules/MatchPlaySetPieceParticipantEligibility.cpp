#include "MatchPlaySetPieceParticipantEligibility.h"

namespace MatchPlaySetPieceParticipantEligibility
{
	bool IsPlayer(const EInitialTurnOrderPlayer Side)
	{
		return Side == EInitialTurnOrderPlayer::PlayerA
			|| Side == EInitialTurnOrderPlayer::PlayerB;
	}

	bool IsSupportedRole(const EMatchPlaySetPieceParticipantRole Role)
	{
		return Role == EMatchPlaySetPieceParticipantRole::Carrier
			|| Role == EMatchPlaySetPieceParticipantRole::CornerRunner
			|| Role == EMatchPlaySetPieceParticipantRole::CornerHelper;
	}

	const FCardUsageState& GetUsage(
		const FMatchCardUsageState& State,
		const EInitialTurnOrderPlayer Side)
	{
		return Side == EInitialTurnOrderPlayer::PlayerA
			? State.PlayerACardUsageState
			: State.PlayerBCardUsageState;
	}

	void Fail(
		FMatchPlaySetPieceParticipantEligibilityResult& Result,
		const EMatchPlaySetPieceParticipantEligibilityErrorCode ErrorCode,
		const FString& ErrorMessage)
	{
		Result.ErrorCode = ErrorCode;
		Result.ErrorMessage = ErrorMessage;
	}
}

FMatchPlaySetPieceParticipantEligibilityResult
FMatchPlaySetPieceParticipantEligibility::Evaluate(
	const FMatchPlayState& State,
	const FMatchPlaySetPieceParticipantEligibilityRequest& Request)
{
	using namespace MatchPlaySetPieceParticipantEligibility;

	FMatchPlaySetPieceParticipantEligibilityResult Result;
	Result.Request = Request;
	if (!State.RuntimeState.bIsInitialized)
	{
		Fail(Result,
			EMatchPlaySetPieceParticipantEligibilityErrorCode
				::MatchPlayStateNotInitialized,
			TEXT("Set Piece participant eligibility requires initialized match play state."));
		return Result;
	}
	if (!IsPlayer(Request.ExpectedOwnerSide))
	{
		Fail(Result,
			EMatchPlaySetPieceParticipantEligibilityErrorCode
				::InvalidExpectedOwnerSide,
			TEXT("ExpectedOwnerSide must be PlayerA or PlayerB."));
		return Result;
	}
	if (!IsSupportedRole(Request.Role))
	{
		Fail(Result,
			EMatchPlaySetPieceParticipantEligibilityErrorCode::InvalidRole,
			TEXT("Set Piece participant role is invalid."));
		return Result;
	}
	if (Request.CardId.IsNone())
	{
		Fail(Result,
			EMatchPlaySetPieceParticipantEligibilityErrorCode::InvalidCardId,
			TEXT("Set Piece participant CardId must not be None."));
		return Result;
	}

	Result.CardUsageValidationResult =
		FMatchPlayCardUsageStateValidator::Validate(
			State.CardUsageState,
			State.CardSnapshotAuthority);
	if (!Result.CardUsageValidationResult.bIsCanonical)
	{
		Fail(Result,
			EMatchPlaySetPieceParticipantEligibilityErrorCode
				::InvalidUnderlyingState,
			Result.CardUsageValidationResult.ErrorMessage);
		return Result;
	}

	const FCardUsageState& Usage = GetUsage(
		State.CardUsageState,
		Request.ExpectedOwnerSide);
	if (Usage.UsedCardIds.Contains(Request.CardId))
	{
		Fail(Result,
			EMatchPlaySetPieceParticipantEligibilityErrorCode::CardUsed,
			TEXT("A Used card cannot be a Set Piece participant."));
		return Result;
	}
	if (Usage.EjectedCardIds.Contains(Request.CardId))
	{
		Fail(Result,
			EMatchPlaySetPieceParticipantEligibilityErrorCode::CardEjected,
			TEXT("An Ejected card cannot be a Set Piece participant."));
		return Result;
	}
	if (!Usage.AvailableCardIds.Contains(Request.CardId))
	{
		Fail(Result,
			EMatchPlaySetPieceParticipantEligibilityErrorCode::CardNotAvailable,
			TEXT("Set Piece participant must belong to the expected side's Available cards."));
		return Result;
	}

	Result.SnapshotQueryResult =
		FMatchPlayCardSnapshotAuthorityQuery::FindByPlayerSideAndCardId(
			State.CardSnapshotAuthority,
			Request.ExpectedOwnerSide,
			Request.CardId);
	if (!Result.SnapshotQueryResult.bSuccess)
	{
		Fail(Result,
			EMatchPlaySetPieceParticipantEligibilityErrorCode
				::SnapshotQueryFailed,
			Result.SnapshotQueryResult.ErrorMessage);
		return Result;
	}
	if (Result.SnapshotQueryResult.Snapshot.bIsGoalkeeper)
	{
		Fail(Result,
			EMatchPlaySetPieceParticipantEligibilityErrorCode
				::GoalkeeperNotEligible,
			TEXT("Goalkeepers cannot be Carrier, Corner Runner, or Corner Helper."));
		return Result;
	}

	Result.Binding.bIsBound = true;
	Result.Binding.OwnerSide = Request.ExpectedOwnerSide;
	Result.Binding.CardId = Request.CardId;
	Result.Binding.Snapshot = Result.SnapshotQueryResult.Snapshot;
	Result.bIsEligible = true;
	return Result;
}
