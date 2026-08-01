#pragma once

#include "CoreMinimal.h"

#include "MatchPlayState.h"

enum class EMatchPlayCurrentAttackInitialRouteMappingErrorCode : uint8
{
	None,
	UnsupportedActionType,
	InvalidIntent,
	UnexpectedInitialRouteD6,
	MissingInitialRouteD6,
	InvalidInitialRouteD6,
	ActionSpecificMappingFailed
};

struct FMCODEX_API FMatchPlayCurrentAttackInitialRouteMappingInput
{
	ESkillRuleType ActionType = ESkillRuleType::None;
	EMatchPlayElectiveBranchIntent Intent =
		EMatchPlayElectiveBranchIntent::None;
	bool bHasInitialRouteD6 = false;
	int32 InitialRouteD6 = 0;
};

struct FMCODEX_API FMatchPlayCurrentAttackInitialRouteMappingResult
{
	bool bSuccess = false;
	FMatchPlayCurrentAttackActualBranch ActualBranch;
	EMatchPlayCurrentAttackInitialRouteMappingErrorCode ErrorCode =
		EMatchPlayCurrentAttackInitialRouteMappingErrorCode::None;
	FString ErrorMessage;
	FName InvalidField = NAME_None;
	FMatchPlayCurrentAttackInitialRouteMappingInput Input;
};

class FMCODEX_API FMatchPlayCurrentAttackInitialRouteMappingQuery final
{
public:
	static FMatchPlayCurrentAttackInitialRouteMappingResult Map(
		const FMatchPlayCurrentAttackInitialRouteMappingInput& Input);
};
