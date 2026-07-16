#pragma once

#include "CoreMinimal.h"

#include "CoreRuleEnums.h"
#include "ThroughBallBranchSelectionQuery.h"
#include "ThroughBallParticipantEligibilityQuery.h"

enum class EThroughBallBehindDefenseP1PlanQueryDecision : uint8
{
	None,
	OutOfPlay,
	FormulaResolutionRequired
};

enum class EThroughBallBehindDefenseP1PlanQueryErrorCode : uint8
{
	None,
	ParticipantEligibilityFailed,
	InvalidParticipantEligibilityResult,
	UnsupportedBranch,
	MissingAttackD6,
	InvalidAttackD6,
	InvalidLogContext,
	MissingDefenseD6,
	InvalidDefenseD6
};

struct FMCODEX_API FThroughBallBehindDefenseP1PlanQueryInput
{
	FThroughBallParticipantEligibilityQueryResult
		ParticipantEligibilityResult;

	EThroughBallSelectedBranch SelectedBranch =
		EThroughBallSelectedBranch::None;

	bool bHasAttackD6 = false;
	int32 AttackD6 = 0;

	bool bHasDefenseD6 = false;
	int32 DefenseD6 = 0;

	FGuid LogId;
	int32 TurnIndex = 0;
};

struct FMCODEX_API FThroughBallBehindDefenseP1FormulaPlan
{
	EFormulaType FormulaType = EFormulaType::None;

	FName CarrierId = NAME_None;
	int32 CarrierPassing = 0;
	int32 CarrierStamina = 0;

	FName RunnerId = NAME_None;
	int32 RunnerSpeed = 0;
	int32 RunnerStamina = 0;

	int32 AttackD6 = 0;
	float AttackBaseValue = 0.0f;
	float AttackExternalModifier = 0.0f;
	TArray<int32> AttackParticipatingStamina;

	FName MarkerId = NAME_None;
	int32 MarkerMarking = 0;
	int32 MarkerStamina = 0;

	bool bHasHelper = false;
	FName HelperId = NAME_None;
	int32 HelperSpeed = 0;
	int32 HelperStamina = 0;

	int32 DefenseD6 = 0;
	float DefenseBaseValue = 0.0f;
	float DefenseExternalModifier = 0.0f;
	TArray<int32> DefenseParticipatingStamina;

	FGuid LogId;
	int32 TurnIndex = 0;

	FName AttackingOwnerId = NAME_None;
	FName DefendingOwnerId = NAME_None;
	TArray<FName> InvolvedCardIds;

	bool bAttackerVictoryRequiresP2 = false;
	bool bDefenderVictoryEndsAttack = false;
};

struct FMCODEX_API FThroughBallBehindDefenseP1PlanQueryResult
{
	bool bSuccess = false;

	EThroughBallBehindDefenseP1PlanQueryDecision Decision =
		EThroughBallBehindDefenseP1PlanQueryDecision::None;

	EThroughBallBehindDefenseP1PlanQueryErrorCode ErrorCode =
		EThroughBallBehindDefenseP1PlanQueryErrorCode::None;

	FString ErrorMessage;
	FName InvalidField = NAME_None;

	FThroughBallBehindDefenseP1PlanQueryInput Input;

	bool bHasFormulaPlan = false;
	FThroughBallBehindDefenseP1FormulaPlan FormulaPlan;

	bool bAttackEnded = false;
	bool bContinueResolution = false;
	bool bRequiresP2 = false;
};

class FMCODEX_API FThroughBallBehindDefenseP1PlanQuery final
{
public:
	static FThroughBallBehindDefenseP1PlanQueryResult Evaluate(
		const FThroughBallBehindDefenseP1PlanQueryInput& Input);
};
