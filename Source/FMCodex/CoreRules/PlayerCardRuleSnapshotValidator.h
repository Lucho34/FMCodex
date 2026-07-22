#pragma once

#include "CoreMinimal.h"

#include "PlayerCardRuleSnapshot.h"

#include "PlayerCardRuleSnapshotValidator.generated.h"

UENUM(BlueprintType)
enum class EPlayerCardRuleSnapshotValidationErrorCode : uint8
{
	None UMETA(DisplayName = "None"),
	InvalidCardId UMETA(DisplayName = "Invalid Card ID"),
	DuplicateCardId UMETA(DisplayName = "Duplicate Card ID"),
	EmptyPositionTypes UMETA(DisplayName = "Empty Position Types"),
	InvalidPositionType UMETA(DisplayName = "Invalid Position Type"),
	DuplicatePositionType UMETA(DisplayName = "Duplicate Position Type"),
	GoalkeeperMustHaveOnlyGoalkeeperPosition UMETA(DisplayName = "Goalkeeper Must Have Only Goalkeeper Position"),
	NonGoalkeeperCannotHaveGoalkeeperPosition UMETA(DisplayName = "Non-Goalkeeper Cannot Have Goalkeeper Position"),
	GoalkeeperAttributesRequired UMETA(DisplayName = "Goalkeeper Attributes Required"),
	NonGoalkeeperHasGoalkeeperAttributes UMETA(DisplayName = "Non-Goalkeeper Has Goalkeeper Attributes"),
	AttributeOutOfRange UMETA(DisplayName = "Attribute Out Of Range"),
	InvalidRarity UMETA(DisplayName = "Invalid Rarity"),
	InvalidSkillId UMETA(DisplayName = "Invalid Skill ID"),
	DuplicateSkillId UMETA(DisplayName = "Duplicate Skill ID"),
	TooManySkillIds UMETA(DisplayName = "Too Many Skill IDs")
};

struct FMCODEX_API FPlayerCardRuleSnapshotValidationResult
{
	bool bSuccess = false;
	bool bIsValid = false;
	EPlayerCardRuleSnapshotValidationErrorCode ErrorCode =
		EPlayerCardRuleSnapshotValidationErrorCode::None;
	FString ErrorMessage;
	FName InvalidCardId = NAME_None;
	TArray<FName> DuplicateCardIds;
};

class FMCODEX_API FPlayerCardRuleSnapshotValidator final
{
public:
	static constexpr int32 MinAttributeValue = 1;
	static constexpr int32 MaxAttributeValue = 6;
	static constexpr int32 MaxSkillIdCount = 3;

	static FPlayerCardRuleSnapshotValidationResult Validate(
		const FPlayerCardRuleSnapshotSet& SnapshotSet);
};
