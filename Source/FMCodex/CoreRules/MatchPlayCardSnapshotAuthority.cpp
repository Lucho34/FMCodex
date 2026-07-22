#include "MatchPlayCardSnapshotAuthority.h"

namespace MatchPlayCardSnapshotAuthority
{
	FPlayerCardRuleSnapshot ProjectCard(
		const FPlayerCardData& CardData)
	{
		FPlayerCardRuleSnapshot Snapshot;
		Snapshot.CardId = CardData.CardId;
		Snapshot.PositionTypes = CardData.PositionTypes;
		Snapshot.Attributes = CardData.Attributes;
		Snapshot.bIsGoalkeeper = CardData.bIsGoalkeeper;
		Snapshot.bHasGoalkeeperAttributes = CardData.bIsGoalkeeper;
		Snapshot.GoalkeeperAttributes = CardData.GoalkeeperAttributes;
		Snapshot.Rarity = CardData.Rarity;
		Snapshot.SkillIds = CardData.AttackSkillIds;
		return Snapshot;
	}

	FPlayerCardRuleSnapshotSet ProjectDeck(
		const TArray<FPlayerCardData>& Deck)
	{
		FPlayerCardRuleSnapshotSet SnapshotSet;
		SnapshotSet.Cards.Reserve(Deck.Num());
		for (const FPlayerCardData& CardData : Deck)
		{
			SnapshotSet.Cards.Add(ProjectCard(CardData));
		}
		return SnapshotSet;
	}

	void SetDeckFailure(
		FMatchPlayCardSnapshotAuthorityBuildResult& Result,
		const EInitialTurnOrderPlayer PlayerSide,
		const FDeckValidationResult& DeckValidationResult)
	{
		Result.ErrorCode = EMatchPlayCardSnapshotAuthorityBuildErrorCode
			::DeckValidationFailed;
		Result.FailingPlayerSide = PlayerSide;
		if (!DeckValidationResult.ErrorCodes.IsEmpty())
		{
			Result.UnderlyingDeckValidationErrorCode =
				DeckValidationResult.ErrorCodes[0];
		}
		Result.ErrorMessage = DeckValidationResult.ErrorMessages.IsEmpty()
			? TEXT("Deck validation failed.")
			: FString::Join(
				DeckValidationResult.ErrorMessages,
				TEXT(" | "));
	}

	void SetSnapshotFailure(
		FMatchPlayCardSnapshotAuthorityBuildResult& Result,
		const EInitialTurnOrderPlayer PlayerSide,
		const FPlayerCardRuleSnapshotValidationResult& ValidationResult)
	{
		Result.ErrorCode = EMatchPlayCardSnapshotAuthorityBuildErrorCode
			::SnapshotValidationFailed;
		Result.FailingPlayerSide = PlayerSide;
		Result.UnderlyingPlayerCardRuleSnapshotValidationErrorCode =
			ValidationResult.ErrorCode;
		Result.ErrorMessage = ValidationResult.ErrorMessage;
	}

	bool IsPlayer(const EInitialTurnOrderPlayer PlayerSide)
	{
		return PlayerSide == EInitialTurnOrderPlayer::PlayerA
			|| PlayerSide == EInitialTurnOrderPlayer::PlayerB;
	}
}

FMatchPlayCardSnapshotAuthorityBuildResult
FMatchPlayCardSnapshotAuthorityBuilder::Build(
	const TArray<FPlayerCardData>& PlayerADeck,
	const TArray<FPlayerCardData>& PlayerBDeck)
{
	FMatchPlayCardSnapshotAuthorityBuildResult Result;

	const FDeckValidationResult PlayerADeckValidation =
		FDeckValidator::ValidateDeck(PlayerADeck);
	if (!PlayerADeckValidation.bIsValid)
	{
		MatchPlayCardSnapshotAuthority::SetDeckFailure(
			Result,
			EInitialTurnOrderPlayer::PlayerA,
			PlayerADeckValidation);
		return Result;
	}

	const FPlayerCardRuleSnapshotSet PlayerASnapshots =
		MatchPlayCardSnapshotAuthority::ProjectDeck(PlayerADeck);
	const FPlayerCardRuleSnapshotValidationResult PlayerAValidation =
		FPlayerCardRuleSnapshotValidator::Validate(PlayerASnapshots);
	if (!PlayerAValidation.bSuccess)
	{
		MatchPlayCardSnapshotAuthority::SetSnapshotFailure(
			Result,
			EInitialTurnOrderPlayer::PlayerA,
			PlayerAValidation);
		return Result;
	}

	const FDeckValidationResult PlayerBDeckValidation =
		FDeckValidator::ValidateDeck(PlayerBDeck);
	if (!PlayerBDeckValidation.bIsValid)
	{
		MatchPlayCardSnapshotAuthority::SetDeckFailure(
			Result,
			EInitialTurnOrderPlayer::PlayerB,
			PlayerBDeckValidation);
		return Result;
	}

	const FPlayerCardRuleSnapshotSet PlayerBSnapshots =
		MatchPlayCardSnapshotAuthority::ProjectDeck(PlayerBDeck);
	const FPlayerCardRuleSnapshotValidationResult PlayerBValidation =
		FPlayerCardRuleSnapshotValidator::Validate(PlayerBSnapshots);
	if (!PlayerBValidation.bSuccess)
	{
		MatchPlayCardSnapshotAuthority::SetSnapshotFailure(
			Result,
			EInitialTurnOrderPlayer::PlayerB,
			PlayerBValidation);
		return Result;
	}

	Result.CardSnapshotAuthority.PlayerACardSnapshots =
		PlayerASnapshots;
	Result.CardSnapshotAuthority.PlayerBCardSnapshots =
		PlayerBSnapshots;
	Result.bSuccess = true;
	return Result;
}

FMatchPlayCardSnapshotAuthorityQueryResult
FMatchPlayCardSnapshotAuthorityQuery::FindByPlayerSideAndCardId(
	const FMatchPlayPerSideCardSnapshotAuthority& Authority,
	const EInitialTurnOrderPlayer PlayerSide,
	const FName CardId)
{
	FMatchPlayCardSnapshotAuthorityQueryResult Result;
	Result.PlayerSide = PlayerSide;
	Result.CardId = CardId;

	if (!MatchPlayCardSnapshotAuthority::IsPlayer(PlayerSide))
	{
		Result.ErrorCode = EMatchPlayCardSnapshotAuthorityQueryErrorCode
			::InvalidPlayerSide;
		Result.ErrorMessage =
			TEXT("PlayerSide must be PlayerA or PlayerB.");
		return Result;
	}

	if (CardId.IsNone())
	{
		Result.ErrorCode = EMatchPlayCardSnapshotAuthorityQueryErrorCode
			::InvalidCardId;
		Result.ErrorMessage = TEXT("CardId must not be None.");
		return Result;
	}

	const FPlayerCardRuleSnapshotSet& SelectedSnapshots =
		PlayerSide == EInitialTurnOrderPlayer::PlayerA
			? Authority.PlayerACardSnapshots
			: Authority.PlayerBCardSnapshots;
	Result.UnderlyingQueryResult =
		FPlayerCardRuleSnapshotQuery::FindByCardId(
			SelectedSnapshots,
			CardId);
	if (!Result.UnderlyingQueryResult.bSuccess)
	{
		Result.ErrorCode = Result.UnderlyingQueryResult.ErrorCode
			== EPlayerCardRuleSnapshotQueryErrorCode::InvalidSnapshotSet
				? EMatchPlayCardSnapshotAuthorityQueryErrorCode
					::SnapshotValidationFailed
				: EMatchPlayCardSnapshotAuthorityQueryErrorCode
					::SnapshotNotFound;
		Result.ErrorMessage = Result.UnderlyingQueryResult.ErrorMessage;
		return Result;
	}

	Result.Snapshot = Result.UnderlyingQueryResult.Snapshot;
	Result.bSuccess = true;
	return Result;
}
