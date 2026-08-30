#include "MatchPlaySendingOffResolution.h"

namespace MatchPlaySendingOffResolution
{
	bool IsPlayer(const EInitialTurnOrderPlayer Side)
	{
		return Side == EInitialTurnOrderPlayer::PlayerA
			|| Side == EInitialTurnOrderPlayer::PlayerB;
	}

	FCardUsageState& GetMutableUsage(
		FMatchCardUsageState& State,
		const EInitialTurnOrderPlayer Side)
	{
		return Side == EInitialTurnOrderPlayer::PlayerA
			? State.PlayerACardUsageState
			: State.PlayerBCardUsageState;
	}

	const FCardUsageState& GetUsage(
		const FMatchCardUsageState& State,
		const EInitialTurnOrderPlayer Side)
	{
		return Side == EInitialTurnOrderPlayer::PlayerA
			? State.PlayerACardUsageState
			: State.PlayerBCardUsageState;
	}

	EInitialTurnOrderPlayer GetDefender(
		const EInitialTurnOrderPlayer Attacker)
	{
		return Attacker == EInitialTurnOrderPlayer::PlayerA
			? EInitialTurnOrderPlayer::PlayerB
			: EInitialTurnOrderPlayer::PlayerA;
	}

	void SetError(
		FMatchPlaySendingOffResolutionResult& Result,
		const EMatchPlaySendingOffResolutionErrorCode ErrorCode,
		const FString& ErrorMessage)
	{
		Result.ErrorCode = ErrorCode;
		Result.ErrorMessage = ErrorMessage;
	}
}

FMatchPlaySendingOffCandidateQueryResult
FMatchPlaySendingOffCandidateQuery::Query(const FMatchPlayState& State)
{
	using namespace MatchPlaySendingOffResolution;

	FMatchPlaySendingOffCandidateQueryResult Result;
	Result.RouteValidationResult =
		FMatchPlayCurrentAttackRouteStateValidator::Validate(State);
	if (!Result.RouteValidationResult.bIsCanonical
		|| !State.bHasCurrentAttack
		|| State.CurrentAttack.RouteKind
			!= EMatchPlayCurrentAttackRouteKind::SendingOff
		|| State.CurrentAttack.SendingOffRoute.Stage
			!= EMatchPlaySendingOffRouteStage::AwaitingResolution)
	{
		Result.ErrorCode =
			EMatchPlaySendingOffResolutionErrorCode::InvalidRouteState;
		Result.ErrorMessage = Result.RouteValidationResult.bIsCanonical
			? TEXT("Sending-Off candidate query requires AwaitingResolution.")
			: Result.RouteValidationResult.ErrorMessage;
		return Result;
	}

	const EInitialTurnOrderPlayer Attacker =
		State.RuntimeState.CurrentAttackingPlayer;
	if (!IsPlayer(Attacker))
	{
		Result.ErrorCode =
			EMatchPlaySendingOffResolutionErrorCode::InvalidCurrentAttacker;
		Result.ErrorMessage =
			TEXT("Sending-Off requires PlayerA or PlayerB as current attacker.");
		return Result;
	}

	Result.CardUsageValidationResult =
		FMatchPlayCardUsageStateValidator::Validate(
			State.CardUsageState,
			State.CardSnapshotAuthority);
	if (!Result.CardUsageValidationResult.bIsCanonical)
	{
		Result.ErrorCode =
			EMatchPlaySendingOffResolutionErrorCode::InvalidCardUsageState;
		Result.ErrorMessage = Result.CardUsageValidationResult.ErrorMessage;
		return Result;
	}

	const FCardUsageState& AttackerUsage =
		GetUsage(State.CardUsageState, Attacker);
	for (const FName CardId : AttackerUsage.AvailableCardIds)
	{
		const FMatchPlayCardSnapshotAuthorityQueryResult SnapshotQuery =
			FMatchPlayCardSnapshotAuthorityQuery::FindByPlayerSideAndCardId(
				State.CardSnapshotAuthority,
				Attacker,
				CardId);
		if (!SnapshotQuery.bSuccess)
		{
			Result.ErrorCode = EMatchPlaySendingOffResolutionErrorCode
				::CandidateSnapshotQueryFailed;
			Result.ErrorMessage = SnapshotQuery.ErrorMessage;
			return Result;
		}
		if (!SnapshotQuery.Snapshot.bIsGoalkeeper)
		{
			Result.CandidateCardIds.Add(CardId);
		}
	}

	Result.bSuccess = true;
	return Result;
}

FMatchPlaySendingOffResolutionResult
FMatchPlaySendingOffResolution::Resolve(
	const FMatchPlayState& BeforeState,
	const FMatchPlaySendingOffResolutionRequest& Request,
	IMatchPlayAttackEntryRollProvider* SelectionProvider)
{
	using namespace MatchPlaySendingOffResolution;

	FMatchPlaySendingOffResolutionResult Result;
	Result.BeforeState = BeforeState;
	Result.AfterState = BeforeState;
	if (Request.AttackSequence <= 0)
	{
		SetError(Result,
			EMatchPlaySendingOffResolutionErrorCode::InvalidAttackSequence,
			TEXT("Sending-Off request requires a positive AttackSequence."));
		return Result;
	}
	if (!BeforeState.bHasCurrentAttack
		|| Request.AttackSequence
			!= BeforeState.CurrentAttack.AttackSequence)
	{
		SetError(Result,
			EMatchPlaySendingOffResolutionErrorCode::AttackSequenceMismatch,
			TEXT("Sending-Off request does not match the current attack sequence."));
		return Result;
	}

	const FMatchPlaySendingOffCandidateQueryResult CandidateQuery =
		FMatchPlaySendingOffCandidateQuery::Query(BeforeState);
	Result.CandidateCardIds = CandidateQuery.CandidateCardIds;
	if (!CandidateQuery.bSuccess)
	{
		SetError(Result, CandidateQuery.ErrorCode, CandidateQuery.ErrorMessage);
		return Result;
	}

	int32 SelectedIndex = INDEX_NONE;
	if (Result.CandidateCardIds.Num() == 1)
	{
		SelectedIndex = 0;
	}
	else if (Result.CandidateCardIds.Num() >= 2)
	{
		if (SelectionProvider == nullptr)
		{
			SetError(Result,
				EMatchPlaySendingOffResolutionErrorCode::MissingSelectionProvider,
				TEXT("Sending-Off selection provider is required for multiple candidates."));
			return Result;
		}
		Result.bSelectionProviderCalled = true;
		Result.ProviderResult = SelectionProvider->SelectUniformIndex(
			EMatchPlayAttackEntryRollPurpose::SendingOffSelection,
			Result.CandidateCardIds.Num());
		Result.ProviderValidationResult =
			FMatchPlayAttackEntrySelectionProviderResultValidator::Validate(
				EMatchPlayAttackEntryRollPurpose::SendingOffSelection,
				Result.CandidateCardIds.Num(),
				Result.ProviderResult);
		if (!Result.ProviderValidationResult.bIsCanonical)
		{
			SetError(Result,
				Result.ProviderValidationResult.ErrorCode
					== EMatchPlayAttackEntryRollProviderResultValidationErrorCode
						::ProviderFailure
					? EMatchPlaySendingOffResolutionErrorCode
						::SelectionProviderFailure
					: EMatchPlaySendingOffResolutionErrorCode
						::MalformedSelectionProviderResult,
				Result.ProviderValidationResult.ErrorMessage);
			return Result;
		}
		SelectedIndex = Result.ProviderResult.SelectedIndex;
	}

	FMatchPlayState WorkingState = BeforeState;
	FMatchPlaySendingOffRouteState& Route =
		WorkingState.CurrentAttack.SendingOffRoute;
	Route.Stage = EMatchPlaySendingOffRouteStage::Resolved;
	Route.GameplayOutcome = EMatchPlaySendingOffGameplayOutcome::NoGoal;
	const EInitialTurnOrderPlayer Attacker =
		BeforeState.RuntimeState.CurrentAttackingPlayer;
	if (SelectedIndex == INDEX_NONE)
	{
		Route.SelectionOutcome =
			EMatchPlaySendingOffSelectionOutcome::NoEligibleCandidate;
	}
	else
	{
		Result.EjectedCardId = Result.CandidateCardIds[SelectedIndex];
		FCardUsageState& Usage =
			GetMutableUsage(WorkingState.CardUsageState, Attacker);
		if (Usage.AvailableCardIds.RemoveSingle(Result.EjectedCardId) != 1)
		{
			SetError(Result,
				EMatchPlaySendingOffResolutionErrorCode::EjectionMutationFailed,
				TEXT("Selected Sending-Off card was not uniquely available."));
			return Result;
		}
		Usage.EjectedCardIds.Add(Result.EjectedCardId);
		Route.SelectionOutcome =
			EMatchPlaySendingOffSelectionOutcome::CardEjected;
		Route.EjectedCardId = Result.EjectedCardId;
	}

	const FMatchPlayCardUsageStateValidationResult UpdatedUsageValidation =
		FMatchPlayCardUsageStateValidator::Validate(
			WorkingState.CardUsageState,
			WorkingState.CardSnapshotAuthority);
	if (!UpdatedUsageValidation.bIsCanonical)
	{
		SetError(Result,
			EMatchPlaySendingOffResolutionErrorCode::EjectionMutationFailed,
			UpdatedUsageValidation.ErrorMessage);
		return Result;
	}

	FMatchPlayCurrentAttackCompletionResult CompletionSeed;
	CompletionSeed.BeforeState = BeforeState;
	CompletionSeed.AfterState = BeforeState;
	Result.CompletionResult =
		FMatchPlayCurrentAttackCompletion::PersistCurrentAttackTerminal(
			BeforeState,
			MoveTemp(WorkingState),
			Attacker,
			GetDefender(Attacker),
			MoveTemp(CompletionSeed));
	if (!Result.CompletionResult.bSuccess)
	{
		SetError(Result,
			EMatchPlaySendingOffResolutionErrorCode::TerminalPersistenceFailed,
			Result.CompletionResult.ErrorMessage);
		return Result;
	}

	Result.AfterState = Result.CompletionResult.AfterState;
	Result.bSuccess = true;
	return Result;
}
