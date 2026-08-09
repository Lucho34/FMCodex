#pragma once

#include "CoreMinimal.h"

#include "FormulaResolver.h"
#include "ThroughBallOneOnOneHandoffCreator.h"

enum class EThroughBallOneOnOneDirectShotDecision : uint8
{
	None,
	Goal,
	Miss
};

struct FMCODEX_API FThroughBallOneOnOneDirectShotFormulaPlan
{
	FThroughBallOneOnOneHandoff Handoff;
	FName GoalkeeperCardId = NAME_None;
	int32 ShooterShooting = 0;
	int32 GoalkeeperOneOnOne = 0;
	bool bGoalkeeperActivated = false;
	int32 AttackD6 = 0;
	int32 DefenseD6 = 0;
	FGuid LogId;
	int32 TurnIndex = 0;
	TArray<FName> InvolvedCardIds;
};

enum class EThroughBallOneOnOneDirectShotFormulaErrorCode : uint8
{
	None,
	InvalidHandoff,
	InvalidGoalkeeperIdentity,
	InvalidAttribute,
	InvalidAttackD6,
	InvalidDefenseD6,
	InvalidLogContext,
	InvalidInvolvedCardIds,
	InvalidPlanMapping,
	InvalidFormulaResult
};

struct FMCODEX_API FThroughBallOneOnOneDirectShotFormulaResult
{
	bool bSuccess = false;
	EThroughBallOneOnOneDirectShotFormulaErrorCode ErrorCode =
		EThroughBallOneOnOneDirectShotFormulaErrorCode::None;
	FString ErrorMessage;
	FName InvalidField = NAME_None;
	FThroughBallOneOnOneDirectShotFormulaPlan Plan;
	bool bHasResolverInput = false;
	FFormulaResolverInput ResolverInput;
	bool bHasFormulaResolution = false;
	FFormulaResolutionResult FormulaResolutionResult;
	EThroughBallOneOnOneDirectShotDecision Decision =
		EThroughBallOneOnOneDirectShotDecision::None;
};

class FMCODEX_API FThroughBallOneOnOneDirectShotFormula final
{
public:
	static FThroughBallOneOnOneDirectShotFormulaResult Resolve(
		const FThroughBallOneOnOneDirectShotFormulaPlan& Plan);
};
