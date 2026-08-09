#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCurrentAttackCompletion.h"

enum class EMatchPlayCarrierNoSelectionNoGoalErrorCode : uint8
{
	None,
	MatchPlayStateNotInitialized,
	NoCurrentAttack,
	InvalidCurrentAttackingPlayer,
	AvailabilityQueryFailed,
	LegalCarrierExists,
	CompletionFailed
};

struct FMCODEX_API FMatchPlayResolveNoLegalCarrierResult
{
	bool bSuccess = false;
	FMatchPlayState BeforeState;
	FMatchPlayState AfterState;
	FMatchPlayCurrentAttackCarrierSelectionAvailabilityResult
		CarrierAvailabilityResult;
	int32 CompletionExecutionCount = 0;
	FMatchPlayCurrentAttackCompletionResult CompletionResult;
	EMatchPlayCarrierNoSelectionNoGoalErrorCode ErrorCode =
		EMatchPlayCarrierNoSelectionNoGoalErrorCode::None;
	FString ErrorMessage;
};

class FMCODEX_API FMatchPlayResolveNoLegalCarrier final
{
public:
	static FMatchPlayResolveNoLegalCarrierResult Resolve(
		const FMatchPlayState& BeforeState);
};
