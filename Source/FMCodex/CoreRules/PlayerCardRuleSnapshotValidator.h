#pragma once

#include "CoreMinimal.h"

#include "PlayerCardRuleSnapshot.h"

enum class EPlayerCardRuleSnapshotValidationErrorCode : uint8
{
	None,
	InvalidCardId,
	DuplicateCardId,
	EmptyPositionTypes,
	InvalidPositionType,
	DuplicatePositionType,
	GoalkeeperMustHaveOnlyGoalkeeperPosition,
	NonGoalkeeperCannotHaveGoalkeeperPosition,
	GoalkeeperAttributesRequired,
	NonGoalkeeperHasGoalkeeperAttributes,
	AttributeOutOfRange,
	InvalidRarity,
	InvalidSkillId,
	DuplicateSkillId,
	TooManySkillIds
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
