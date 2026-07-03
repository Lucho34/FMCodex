#pragma once

#include "CoreMinimal.h"

#include "PlayerCardRuleSnapshotValidator.h"

enum class EPlayerCardRuleSnapshotQueryErrorCode : uint8
{
	None,
	InvalidCardId,
	InvalidSnapshotSet,
	CardNotFound
};

struct FMCODEX_API FPlayerCardRuleSnapshotQueryResult
{
	bool bSuccess = false;
	bool bFound = false;
	EPlayerCardRuleSnapshotQueryErrorCode ErrorCode =
		EPlayerCardRuleSnapshotQueryErrorCode::None;
	FString ErrorMessage;
	FName CardId = NAME_None;
	FPlayerCardRuleSnapshot Snapshot;
	FPlayerCardRuleSnapshotValidationResult ValidationResult;
};

class FMCODEX_API FPlayerCardRuleSnapshotQuery final
{
public:
	static FPlayerCardRuleSnapshotQueryResult FindByCardId(
		const FPlayerCardRuleSnapshotSet& SnapshotSet,
		FName CardId);
};
