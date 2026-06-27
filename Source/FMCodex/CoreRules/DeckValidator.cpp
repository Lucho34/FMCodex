#include "DeckValidator.h"

namespace DeckValidator
{
	constexpr int32 RequiredCardCount = 20;

	void AddError(
		FDeckValidationResult& Result,
		const EDeckValidationErrorCode ErrorCode,
		const FString& ErrorMessage)
	{
		Result.ErrorCodes.Add(ErrorCode);
		Result.ErrorMessages.Add(ErrorMessage);
	}

	bool HasOnlyGoalkeeperPosition(const FPlayerCardData& Card)
	{
		return Card.PositionTypes.Num() == 1
			&& Card.PositionTypes[0] == EPlayerPositionType::Goalkeeper;
	}
}

FDeckValidationResult FDeckValidator::ValidateDeck(const TArray<FPlayerCardData>& Deck)
{
	FDeckValidationResult Result;

	if (Deck.Num() != DeckValidator::RequiredCardCount)
	{
		DeckValidator::AddError(
			Result,
			EDeckValidationErrorCode::InvalidCardCount,
			FString::Printf(
				TEXT("Deck must contain exactly %d player cards; received %d."),
				DeckValidator::RequiredCardCount,
				Deck.Num()));
	}

	int32 GoalkeeperCount = 0;
	FString SingleGoalkeeperCardId;

	for (const FPlayerCardData& Card : Deck)
	{
		Result.InitialDeckRarityScore += GetRarityScore(Card.Rarity);

		if (Card.bIsGoalkeeper)
		{
			++GoalkeeperCount;
			SingleGoalkeeperCardId = Card.CardId.ToString();

			if (!DeckValidator::HasOnlyGoalkeeperPosition(Card))
			{
				DeckValidator::AddError(
					Result,
					EDeckValidationErrorCode::GoalkeeperHasMixedPositionTypes,
					FString::Printf(
						TEXT("Goalkeeper card '%s' must have GK as its only position type."),
						*Card.CardId.ToString()));
			}
		}
		else if (Card.PositionTypes.Contains(EPlayerPositionType::Goalkeeper))
		{
			DeckValidator::AddError(
				Result,
				EDeckValidationErrorCode::NonGoalkeeperContainsGoalkeeperPosition,
				FString::Printf(
					TEXT("Non-goalkeeper card '%s' cannot contain the GK position type."),
					*Card.CardId.ToString()));
		}
	}

	if (GoalkeeperCount == 0)
	{
		DeckValidator::AddError(
			Result,
			EDeckValidationErrorCode::NoGoalkeeper,
			TEXT("Deck must contain exactly one goalkeeper; none was found."));
	}
	else if (GoalkeeperCount > 1)
	{
		DeckValidator::AddError(
			Result,
			EDeckValidationErrorCode::MultipleGoalkeepers,
			FString::Printf(
				TEXT("Deck must contain exactly one goalkeeper; received %d."),
				GoalkeeperCount));
	}
	else
	{
		Result.GoalkeeperCardId = SingleGoalkeeperCardId;
	}

	Result.bIsValid = Result.ErrorCodes.IsEmpty();
	return Result;
}

int32 FDeckValidator::GetRarityScore(const ECardRarity Rarity)
{
	switch (Rarity)
	{
	case ECardRarity::WorldClass:
		return 7;
	case ECardRarity::Continental:
		return 5;
	case ECardRarity::National:
		return 3;
	case ECardRarity::Regional:
		return 2;
	case ECardRarity::Common:
		return 1;
	default:
		return 0;
	}
}