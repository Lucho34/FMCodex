#include "PlayerCardRuleSnapshotValidator.h"

namespace PlayerCardRuleSnapshotValidator
{
	void SetFailure(
		FPlayerCardRuleSnapshotValidationResult& Result,
		const EPlayerCardRuleSnapshotValidationErrorCode ErrorCode,
		const FString& ErrorMessage,
		const FName InvalidCardId)
	{
		Result.ErrorCode = ErrorCode;
		Result.ErrorMessage = ErrorMessage;
		Result.InvalidCardId = InvalidCardId;
	}

	bool IsValidPositionType(const EPlayerPositionType PositionType)
	{
		switch (PositionType)
		{
		case EPlayerPositionType::Attack:
		case EPlayerPositionType::Midfield:
		case EPlayerPositionType::Defense:
		case EPlayerPositionType::Goalkeeper:
			return true;
		default:
			return false;
		}
	}

	bool IsValidRarity(const ECardRarity Rarity)
	{
		switch (Rarity)
		{
		case ECardRarity::Common:
		case ECardRarity::Regional:
		case ECardRarity::National:
		case ECardRarity::Continental:
		case ECardRarity::WorldClass:
			return true;
		default:
			return false;
		}
	}

	bool ValidateAttribute(
		FPlayerCardRuleSnapshotValidationResult& Result,
		const FPlayerCardRuleSnapshot& Card,
		const TCHAR* AttributeName,
		const int32 AttributeValue)
	{
		if (AttributeValue
				>= FPlayerCardRuleSnapshotValidator::MinAttributeValue
			&& AttributeValue
				<= FPlayerCardRuleSnapshotValidator::MaxAttributeValue)
		{
			return true;
		}

		SetFailure(
			Result,
			EPlayerCardRuleSnapshotValidationErrorCode
				::AttributeOutOfRange,
			FString::Printf(
				TEXT("CardId '%s' attribute '%s' must be in range [%d, %d]; received %d."),
				*Card.CardId.ToString(),
				AttributeName,
				FPlayerCardRuleSnapshotValidator::MinAttributeValue,
				FPlayerCardRuleSnapshotValidator::MaxAttributeValue,
				AttributeValue),
			Card.CardId);
		return false;
	}

	bool ValidateBaseAttributes(
		FPlayerCardRuleSnapshotValidationResult& Result,
		const FPlayerCardRuleSnapshot& Card)
	{
		const FPlayerAttributes& Attributes = Card.Attributes;
		return ValidateAttribute(
				Result,
				Card,
				TEXT("Shooting"),
				Attributes.Shooting)
			&& ValidateAttribute(
				Result,
				Card,
				TEXT("Dribbling"),
				Attributes.Dribbling)
			&& ValidateAttribute(
				Result,
				Card,
				TEXT("Passing"),
				Attributes.Passing)
			&& ValidateAttribute(
				Result,
				Card,
				TEXT("OffBall"),
				Attributes.OffBall)
			&& ValidateAttribute(
				Result,
				Card,
				TEXT("Marking"),
				Attributes.Marking)
			&& ValidateAttribute(
				Result,
				Card,
				TEXT("Tackling"),
				Attributes.Tackling)
			&& ValidateAttribute(
				Result,
				Card,
				TEXT("Speed"),
				Attributes.Speed)
			&& ValidateAttribute(
				Result,
				Card,
				TEXT("Strength"),
				Attributes.Strength)
			&& ValidateAttribute(
				Result,
				Card,
				TEXT("Stamina"),
				Attributes.Stamina)
			&& ValidateAttribute(
				Result,
				Card,
				TEXT("LongShot"),
				Attributes.LongShot);
	}

	bool ValidateGoalkeeperAttributes(
		FPlayerCardRuleSnapshotValidationResult& Result,
		const FPlayerCardRuleSnapshot& Card)
	{
		const FGoalkeeperAttributes& Attributes =
			Card.GoalkeeperAttributes;
		return ValidateAttribute(
				Result,
				Card,
				TEXT("Handling"),
				Attributes.Handling)
			&& ValidateAttribute(
				Result,
				Card,
				TEXT("Positioning"),
				Attributes.Positioning)
			&& ValidateAttribute(
				Result,
				Card,
				TEXT("Reflex"),
				Attributes.Reflex)
			&& ValidateAttribute(
				Result,
				Card,
				TEXT("Aerial"),
				Attributes.Aerial)
			&& ValidateAttribute(
				Result,
				Card,
				TEXT("Anticipation"),
				Attributes.Anticipation)
			&& ValidateAttribute(
				Result,
				Card,
				TEXT("OneOnOne"),
				Attributes.OneOnOne);
	}

	bool ValidatePositions(
		FPlayerCardRuleSnapshotValidationResult& Result,
		const FPlayerCardRuleSnapshot& Card)
	{
		if (Card.PositionTypes.IsEmpty())
		{
			SetFailure(
				Result,
				EPlayerCardRuleSnapshotValidationErrorCode
					::EmptyPositionTypes,
				FString::Printf(
					TEXT("CardId '%s' must have at least one position type."),
					*Card.CardId.ToString()),
				Card.CardId);
			return false;
		}

		TSet<EPlayerPositionType> SeenPositionTypes;
		for (const EPlayerPositionType PositionType : Card.PositionTypes)
		{
			if (!IsValidPositionType(PositionType))
			{
				SetFailure(
					Result,
					EPlayerCardRuleSnapshotValidationErrorCode
						::InvalidPositionType,
					FString::Printf(
						TEXT("CardId '%s' contains an invalid position type."),
						*Card.CardId.ToString()),
					Card.CardId);
				return false;
			}

			if (SeenPositionTypes.Contains(PositionType))
			{
				SetFailure(
					Result,
					EPlayerCardRuleSnapshotValidationErrorCode
						::DuplicatePositionType,
					FString::Printf(
						TEXT("CardId '%s' contains a duplicate position type."),
						*Card.CardId.ToString()),
					Card.CardId);
				return false;
			}
			SeenPositionTypes.Add(PositionType);
		}

		const bool bHasGoalkeeperPosition =
			SeenPositionTypes.Contains(
				EPlayerPositionType::Goalkeeper);
		if (Card.bIsGoalkeeper)
		{
			if (Card.PositionTypes.Num() != 1
				|| !bHasGoalkeeperPosition)
			{
				SetFailure(
					Result,
					EPlayerCardRuleSnapshotValidationErrorCode
						::GoalkeeperMustHaveOnlyGoalkeeperPosition,
					FString::Printf(
						TEXT("Goalkeeper CardId '%s' must have GK as its only position type."),
						*Card.CardId.ToString()),
					Card.CardId);
				return false;
			}
		}
		else if (bHasGoalkeeperPosition)
		{
			SetFailure(
				Result,
				EPlayerCardRuleSnapshotValidationErrorCode
					::NonGoalkeeperCannotHaveGoalkeeperPosition,
				FString::Printf(
					TEXT("Non-goalkeeper CardId '%s' cannot have the GK position type."),
					*Card.CardId.ToString()),
				Card.CardId);
			return false;
		}

		return true;
	}

	bool ValidateSkills(
		FPlayerCardRuleSnapshotValidationResult& Result,
		const FPlayerCardRuleSnapshot& Card)
	{
		if (Card.SkillIds.Num()
			> FPlayerCardRuleSnapshotValidator::MaxSkillIdCount)
		{
			SetFailure(
				Result,
				EPlayerCardRuleSnapshotValidationErrorCode::TooManySkillIds,
				FString::Printf(
					TEXT("CardId '%s' cannot contain more than %d SkillIds."),
					*Card.CardId.ToString(),
					FPlayerCardRuleSnapshotValidator::MaxSkillIdCount),
				Card.CardId);
			return false;
		}

		TSet<FName> SeenSkillIds;
		for (const FName SkillId : Card.SkillIds)
		{
			if (SkillId.IsNone())
			{
				SetFailure(
					Result,
					EPlayerCardRuleSnapshotValidationErrorCode
						::InvalidSkillId,
					FString::Printf(
						TEXT("CardId '%s' contains an empty SkillId."),
						*Card.CardId.ToString()),
					Card.CardId);
				return false;
			}

			if (SeenSkillIds.Contains(SkillId))
			{
				SetFailure(
					Result,
					EPlayerCardRuleSnapshotValidationErrorCode
						::DuplicateSkillId,
					FString::Printf(
						TEXT("CardId '%s' contains duplicate SkillId '%s'."),
						*Card.CardId.ToString(),
						*SkillId.ToString()),
					Card.CardId);
				return false;
			}
			SeenSkillIds.Add(SkillId);
		}

		return true;
	}
}

FPlayerCardRuleSnapshotValidationResult
FPlayerCardRuleSnapshotValidator::Validate(
	const FPlayerCardRuleSnapshotSet& SnapshotSet)
{
	FPlayerCardRuleSnapshotValidationResult Result;
	TSet<FName> SeenCardIds;

	for (const FPlayerCardRuleSnapshot& Card : SnapshotSet.Cards)
	{
		if (Card.CardId.IsNone())
		{
			PlayerCardRuleSnapshotValidator::SetFailure(
				Result,
				EPlayerCardRuleSnapshotValidationErrorCode::InvalidCardId,
				TEXT("Player card rule snapshot contains an empty CardId."),
				Card.CardId);
			return Result;
		}

		if (SeenCardIds.Contains(Card.CardId))
		{
			if (!Result.DuplicateCardIds.Contains(Card.CardId))
			{
				Result.DuplicateCardIds.Add(Card.CardId);
			}
		}
		else
		{
			SeenCardIds.Add(Card.CardId);
		}
	}

	if (!Result.DuplicateCardIds.IsEmpty())
	{
		Result.ErrorCode =
			EPlayerCardRuleSnapshotValidationErrorCode::DuplicateCardId;
		Result.InvalidCardId = Result.DuplicateCardIds[0];
		Result.ErrorMessage = FString::Printf(
			TEXT("Player card rule snapshot contains duplicate CardId '%s'."),
			*Result.InvalidCardId.ToString());
		return Result;
	}

	for (const FPlayerCardRuleSnapshot& Card : SnapshotSet.Cards)
	{
		if (!PlayerCardRuleSnapshotValidator::ValidatePositions(
				Result,
				Card))
		{
			return Result;
		}

		if (Card.bIsGoalkeeper
			&& !Card.bHasGoalkeeperAttributes)
		{
			PlayerCardRuleSnapshotValidator::SetFailure(
				Result,
				EPlayerCardRuleSnapshotValidationErrorCode
					::GoalkeeperAttributesRequired,
				FString::Printf(
					TEXT("Goalkeeper CardId '%s' must provide goalkeeper attributes."),
					*Card.CardId.ToString()),
				Card.CardId);
			return Result;
		}

		if (!Card.bIsGoalkeeper
			&& Card.bHasGoalkeeperAttributes)
		{
			PlayerCardRuleSnapshotValidator::SetFailure(
				Result,
				EPlayerCardRuleSnapshotValidationErrorCode
					::NonGoalkeeperHasGoalkeeperAttributes,
				FString::Printf(
					TEXT("Non-goalkeeper CardId '%s' cannot provide goalkeeper attributes."),
					*Card.CardId.ToString()),
				Card.CardId);
			return Result;
		}

		if (!PlayerCardRuleSnapshotValidator::IsValidRarity(Card.Rarity))
		{
			PlayerCardRuleSnapshotValidator::SetFailure(
				Result,
				EPlayerCardRuleSnapshotValidationErrorCode::InvalidRarity,
				FString::Printf(
					TEXT("CardId '%s' contains an invalid rarity."),
					*Card.CardId.ToString()),
				Card.CardId);
			return Result;
		}

		if (!PlayerCardRuleSnapshotValidator::ValidateBaseAttributes(
				Result,
				Card))
		{
			return Result;
		}

		if (Card.bIsGoalkeeper
			&& !PlayerCardRuleSnapshotValidator
				::ValidateGoalkeeperAttributes(Result, Card))
		{
			return Result;
		}

		if (!PlayerCardRuleSnapshotValidator::ValidateSkills(
				Result,
				Card))
		{
			return Result;
		}
	}

	Result.bSuccess = true;
	Result.bIsValid = true;
	return Result;
}
