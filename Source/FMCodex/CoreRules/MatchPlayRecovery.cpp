#include "MatchPlayRecovery.h"

#include "MatchPlayCardUsageStateValidator.h"

namespace MatchPlayRecovery
{
	constexpr int32 MinimumStamina = 1;
	constexpr int32 MaximumStamina = 6;
	constexpr int32 CanonicalReturnCount = 2;

	const FCardUsageState& UsageForSide(
		const FMatchCardUsageState& State,
		const EInitialTurnOrderPlayer Side)
	{
		return Side == EInitialTurnOrderPlayer::PlayerA
			? State.PlayerACardUsageState
			: State.PlayerBCardUsageState;
	}

	FCardUsageState& UsageForSide(
		FMatchCardUsageState& State,
		const EInitialTurnOrderPlayer Side)
	{
		return Side == EInitialTurnOrderPlayer::PlayerA
			? State.PlayerACardUsageState
			: State.PlayerBCardUsageState;
	}

	const FPlayerCardRuleSnapshotSet& SnapshotsForSide(
		const FMatchPlayPerSideCardSnapshotAuthority& Authority,
		const EInitialTurnOrderPlayer Side)
	{
		return Side == EInitialTurnOrderPlayer::PlayerA
			? Authority.PlayerACardSnapshots
			: Authority.PlayerBCardSnapshots;
	}

	bool IsPlayerSide(const EInitialTurnOrderPlayer Side)
	{
		return Side == EInitialTurnOrderPlayer::PlayerA
			|| Side == EInitialTurnOrderPlayer::PlayerB;
	}

	void ReorderAvailableByRoster(
		FCardUsageState& Usage,
		const FPlayerCardRuleSnapshotSet& Snapshots)
	{
		TSet<FName> AvailableSet;
		AvailableSet.Append(Usage.AvailableCardIds);
		TArray<FName> Ordered;
		Ordered.Reserve(Usage.AvailableCardIds.Num());
		for (const FPlayerCardRuleSnapshot& Snapshot : Snapshots.Cards)
		{
			if (AvailableSet.Contains(Snapshot.CardId))
			{
				Ordered.Add(Snapshot.CardId);
			}
		}
		Usage.AvailableCardIds = MoveTemp(Ordered);
	}

	void SetResolveError(
		FMatchPlayRecoveryResolveResult& Result,
		const EMatchPlayRecoveryResolveErrorCode ErrorCode,
		const FString& ErrorMessage)
	{
		Result.ErrorCode = ErrorCode;
		Result.ErrorMessage = ErrorMessage;
	}
}

FMatchPlayRecoveryCandidateQueryResult
FMatchPlayRecoveryCandidateQuery::Build(
	const FMatchCardUsageState& CardUsageState,
	const FMatchPlayPerSideCardSnapshotAuthority& CardSnapshotAuthority)
{
	using namespace MatchPlayRecovery;

	FMatchPlayRecoveryCandidateQueryResult Result;
	const FMatchPlayCardUsageStateValidationResult StateValidation =
		FMatchPlayCardUsageStateValidator::Validate(
			CardUsageState,
			CardSnapshotAuthority);
	if (!StateValidation.bIsCanonical)
	{
		Result.ErrorCode =
			EMatchPlayRecoveryCandidateQueryErrorCode::InvalidCardUsageState;
		Result.FailingOwnerSide = StateValidation.FailingPlayerSide;
		Result.FailingCardId = StateValidation.InvalidCardId;
		Result.ErrorMessage = StateValidation.ErrorMessage;
		return Result;
	}

	for (const EInitialTurnOrderPlayer Side : {
		EInitialTurnOrderPlayer::PlayerA,
		EInitialTurnOrderPlayer::PlayerB })
	{
		const FCardUsageState& Usage = UsageForSide(CardUsageState, Side);
		for (const FName CardId : Usage.UsedCardIds)
		{
			const FMatchPlayCardSnapshotAuthorityQueryResult Snapshot =
				FMatchPlayCardSnapshotAuthorityQuery::FindByPlayerSideAndCardId(
					CardSnapshotAuthority,
					Side,
					CardId);
			if (!Snapshot.bSuccess)
			{
				Result.ErrorCode = EMatchPlayRecoveryCandidateQueryErrorCode
					::SnapshotQueryFailed;
				Result.FailingOwnerSide = Side;
				Result.FailingCardId = CardId;
				Result.ErrorMessage = Snapshot.ErrorMessage;
				return Result;
			}
			if (Snapshot.Snapshot.bIsGoalkeeper)
			{
				Result.ErrorCode = EMatchPlayRecoveryCandidateQueryErrorCode
					::GoalkeeperInUsedZone;
				Result.FailingOwnerSide = Side;
				Result.FailingCardId = CardId;
				Result.ErrorMessage = FString::Printf(
					TEXT("Goalkeeper CardId '%s' cannot be a Used Recovery candidate."),
					*CardId.ToString());
				return Result;
			}

			const int32 Stamina = Snapshot.Snapshot.Attributes.Stamina;
			if (Stamina < MinimumStamina || Stamina > MaximumStamina)
			{
				Result.ErrorCode = EMatchPlayRecoveryCandidateQueryErrorCode
					::InvalidStaminaWeight;
				Result.FailingOwnerSide = Side;
				Result.FailingCardId = CardId;
				Result.ErrorMessage = FString::Printf(
					TEXT("Recovery Stamina for CardId '%s' must be in range [1, 6]."),
					*CardId.ToString());
				return Result;
			}

			FMatchPlayRecoveryCandidate Candidate;
			Candidate.OwnerSide = Side;
			Candidate.CardId = CardId;
			Candidate.StaminaWeight = Stamina;
			Result.Candidates.Add(Candidate);
		}
	}

	Result.bSuccess = true;
	return Result;
}

FMatchPlayRecoveryProviderValidationResult
FMatchPlayRecoveryProviderResultValidator::Validate(
	const EMatchPlayRecoveryPurpose Purpose,
	const TArray<FMatchPlayRecoveryCandidate>& OrderedCandidates,
	const int32 ReturnCount,
	const FMatchPlayRecoveryProviderResult& ProviderResult)
{
	using namespace MatchPlayRecovery;

	FMatchPlayRecoveryProviderValidationResult Result;
	if (Purpose != EMatchPlayRecoveryPurpose::ConsumedRecovery
		|| OrderedCandidates.Num() < CanonicalReturnCount
		|| ReturnCount != CanonicalReturnCount)
	{
		Result.ErrorCode =
			EMatchPlayRecoveryProviderValidationErrorCode::InvalidRequest;
		Result.ErrorMessage =
			TEXT("Consumed Recovery requires at least two candidates and ReturnCount 2.");
		return Result;
	}
	for (const FMatchPlayRecoveryCandidate& Candidate : OrderedCandidates)
	{
		if (!IsPlayerSide(Candidate.OwnerSide)
			|| Candidate.CardId.IsNone()
			|| Candidate.StaminaWeight < MinimumStamina
			|| Candidate.StaminaWeight > MaximumStamina)
		{
			Result.ErrorCode =
				EMatchPlayRecoveryProviderValidationErrorCode::InvalidRequest;
			Result.ErrorMessage =
				TEXT("Consumed Recovery provider input contains an invalid candidate.");
			return Result;
		}
	}

	if (ProviderResult.bSuccess)
	{
		if (ProviderResult.ErrorCode != EMatchPlayRecoveryProviderErrorCode::None
			|| !ProviderResult.ErrorMessage.IsEmpty()
			|| ProviderResult.SelectedCandidateIndices.Num() != ReturnCount)
		{
			Result.ErrorCode = EMatchPlayRecoveryProviderValidationErrorCode
				::MalformedProviderResult;
			Result.ErrorMessage =
				TEXT("A successful Recovery provider result must contain exactly two indices and no error payload.");
			return Result;
		}

		TSet<int32> UniqueIndices;
		for (const int32 Index : ProviderResult.SelectedCandidateIndices)
		{
			if (!OrderedCandidates.IsValidIndex(Index)
				|| UniqueIndices.Contains(Index))
			{
				Result.ErrorCode = EMatchPlayRecoveryProviderValidationErrorCode
					::MalformedProviderResult;
				Result.ErrorMessage =
					TEXT("Recovery provider indices must be in range and distinct.");
				return Result;
			}
			UniqueIndices.Add(Index);
		}
		Result.bIsCanonical = true;
		return Result;
	}

	if (ProviderResult.SelectedCandidateIndices.IsEmpty()
		&& ProviderResult.ErrorCode
			== EMatchPlayRecoveryProviderErrorCode::ProviderFailure
		&& !ProviderResult.ErrorMessage.IsEmpty())
	{
		Result.ErrorCode =
			EMatchPlayRecoveryProviderValidationErrorCode::ProviderFailure;
		Result.ErrorMessage = ProviderResult.ErrorMessage;
		return Result;
	}

	Result.ErrorCode = EMatchPlayRecoveryProviderValidationErrorCode
		::MalformedProviderResult;
	Result.ErrorMessage =
		TEXT("A failed Recovery provider result is not canonical.");
	return Result;
}

FMatchPlayRecoveryResolveResult FMatchPlayRecoveryResolver::Resolve(
	const FMatchCardUsageState& CardUsageState,
	const FMatchPlayPerSideCardSnapshotAuthority& CardSnapshotAuthority,
	const int64 SourceAttackSequence,
	IMatchPlayRecoveryProvider* Provider)
{
	using namespace MatchPlayRecovery;

	FMatchPlayRecoveryResolveResult Result;
	Result.UpdatedCardUsageState = CardUsageState;
	Result.CandidateQueryResult = FMatchPlayRecoveryCandidateQuery::Build(
		CardUsageState,
		CardSnapshotAuthority);
	if (!Result.CandidateQueryResult.bSuccess)
	{
		SetResolveError(
			Result,
			EMatchPlayRecoveryResolveErrorCode::CandidateQueryFailed,
			Result.CandidateQueryResult.ErrorMessage);
		return Result;
	}
	if (SourceAttackSequence <= 0)
	{
		SetResolveError(
			Result,
			EMatchPlayRecoveryResolveErrorCode::MutationFailed,
			TEXT("Recovery requires a positive source AttackSequence."));
		return Result;
	}

	TArray<int32> SelectedIndices;
	if (Result.CandidateQueryResult.Candidates.Num() == 1)
	{
		SelectedIndices.Add(0);
	}
	else if (Result.CandidateQueryResult.Candidates.Num() >= CanonicalReturnCount)
	{
		if (Provider == nullptr)
		{
			SetResolveError(
				Result,
				EMatchPlayRecoveryResolveErrorCode::MissingProvider,
				TEXT("Recovery with at least two candidates requires an authoritative provider."));
			return Result;
		}

		Result.ProviderResult = Provider->DrawWeightedWithoutReplacement(
			EMatchPlayRecoveryPurpose::ConsumedRecovery,
			Result.CandidateQueryResult.Candidates,
			CanonicalReturnCount);
		const FMatchPlayRecoveryProviderValidationResult Validation =
			FMatchPlayRecoveryProviderResultValidator::Validate(
				EMatchPlayRecoveryPurpose::ConsumedRecovery,
				Result.CandidateQueryResult.Candidates,
				CanonicalReturnCount,
				Result.ProviderResult);
		if (!Validation.bIsCanonical)
		{
			SetResolveError(
				Result,
				Validation.ErrorCode
					== EMatchPlayRecoveryProviderValidationErrorCode
						::ProviderFailure
						? EMatchPlayRecoveryResolveErrorCode::ProviderFailure
						: EMatchPlayRecoveryResolveErrorCode
							::MalformedProviderResult,
				Validation.ErrorMessage);
			return Result;
		}
		SelectedIndices = Result.ProviderResult.SelectedCandidateIndices;
	}

	FMatchCardUsageState Working = CardUsageState;
	FMatchPlayLastRecoveryFact WorkingRecoveryFact;
	WorkingRecoveryFact.bHasRecoveryFact = true;
	WorkingRecoveryFact.SourceAttackSequence = SourceAttackSequence;
	for (const int32 Index : SelectedIndices)
	{
		if (!Result.CandidateQueryResult.Candidates.IsValidIndex(Index))
		{
			SetResolveError(Result,
				EMatchPlayRecoveryResolveErrorCode::MutationFailed,
				TEXT("Recovery selected an invalid candidate during mutation."));
			return Result;
		}
		const FMatchPlayRecoveryCandidate& Candidate =
			Result.CandidateQueryResult.Candidates[Index];
		FCardUsageState& Usage = UsageForSide(Working, Candidate.OwnerSide);
		const int32 UsedIndex = Usage.UsedCardIds.IndexOfByKey(Candidate.CardId);
		if (UsedIndex == INDEX_NONE
			|| Usage.AvailableCardIds.Contains(Candidate.CardId)
			|| Usage.EjectedCardIds.Contains(Candidate.CardId))
		{
			SetResolveError(Result,
				EMatchPlayRecoveryResolveErrorCode::MutationFailed,
				TEXT("Recovery candidate identity no longer matches authoritative Used state."));
			return Result;
		}
		const FMatchPlayCardSnapshotAuthorityQueryResult Snapshot =
			FMatchPlayCardSnapshotAuthorityQuery::FindByPlayerSideAndCardId(
				CardSnapshotAuthority,
				Candidate.OwnerSide,
				Candidate.CardId);
		if (!Snapshot.bSuccess || Snapshot.Snapshot.bIsGoalkeeper
			|| Snapshot.Snapshot.Attributes.Stamina != Candidate.StaminaWeight)
		{
			SetResolveError(Result,
				EMatchPlayRecoveryResolveErrorCode::MutationFailed,
				TEXT("Recovery candidate snapshot identity or weight changed before mutation."));
			return Result;
		}

		Usage.UsedCardIds.RemoveAt(UsedIndex);
		Usage.AvailableCardIds.Add(Candidate.CardId);
		FMatchPlayRecoveredCardFactEntry Entry;
		Entry.OwnerSide = Candidate.OwnerSide;
		Entry.CardId = Candidate.CardId;
		WorkingRecoveryFact.ReturnedCards.Add(Entry);
	}

	for (const EInitialTurnOrderPlayer Side : {
		EInitialTurnOrderPlayer::PlayerA,
		EInitialTurnOrderPlayer::PlayerB })
	{
		ReorderAvailableByRoster(
			UsageForSide(Working, Side),
			SnapshotsForSide(CardSnapshotAuthority, Side));
	}
	const FMatchPlayCardUsageStateValidationResult FinalValidation =
		FMatchPlayCardUsageStateValidator::Validate(
			Working,
			CardSnapshotAuthority);
	if (!FinalValidation.bIsCanonical)
	{
		SetResolveError(Result,
			EMatchPlayRecoveryResolveErrorCode::MutationFailed,
			FinalValidation.ErrorMessage);
		return Result;
	}

	Result.UpdatedCardUsageState = MoveTemp(Working);
	Result.RecoveryFact = MoveTemp(WorkingRecoveryFact);
	Result.bSuccess = true;
	return Result;
}
