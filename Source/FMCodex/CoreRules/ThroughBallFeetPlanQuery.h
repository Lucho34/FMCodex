#pragma once

#include "CoreMinimal.h"

#include "CoreRuleEnums.h"
#include "PlayerCardRuleSnapshotValidator.h"
#include "ThroughBallParticipantEligibilityQuery.h"

enum class EThroughBallFeetPlanQueryErrorCode : uint8
{
	None,
	ParticipantEligibilityFailed,
	InvalidParticipantEligibilityResult,
	InvalidAttackD6,
	InvalidDefenseD6,
	InvalidLogContext,
	InvalidActiveGoalkeeperIdentity,
	InvalidActiveGoalkeeperSnapshot,
	ActiveGoalkeeperMustBeGoalkeeper,
	DuplicateDefendingGoalkeeperParticipant
};

enum class EThroughBallFeetPlanQueryDecision : uint8
{
	None,
	FormulaResolutionRequired
};

struct FMCODEX_API FThroughBallFeetPlanQueryInput
{
	FThroughBallParticipantEligibilityQueryResult
		ParticipantEligibilityResult;

	int32 AttackD6 = 0;
	int32 DefenseD6 = 0;

	bool bHasActiveGoalkeeper = false;
	FPlayerCardRuleSnapshot ActiveGoalkeeperSnapshot;

	FGuid LogId;
	int32 TurnIndex = 0;
};

struct FMCODEX_API FThroughBallFeetFormulaPlan
{
	EFormulaType FormulaType = EFormulaType::None;

	FName CarrierId = NAME_None;
	int32 CarrierPassing = 0;
	int32 CarrierStamina = 0;

	FName RunnerId = NAME_None;
	int32 RunnerOffBall = 0;
	int32 RunnerStamina = 0;

	int32 AttackD6 = 0;
	float AttackBaseValue = 0.0f;
	float AttackExternalModifier = 0.0f;
	TArray<int32> AttackParticipatingStamina;

	FName MarkerId = NAME_None;
	int32 MarkerTackling = 0;
	int32 MarkerStamina = 0;

	bool bHasHelper = false;
	FName HelperId = NAME_None;
	int32 HelperMarking = 0;
	int32 HelperStamina = 0;

	bool bHasActiveGoalkeeper = false;
	FName ActiveGoalkeeperId = NAME_None;
	int32 GoalkeeperOneOnOne = 0;
	int32 GoalkeeperStamina = 0;

	int32 DefenseD6 = 0;
	float DefenseBaseValue = 0.0f;
	float DefenseExternalModifier = 0.0f;
	TArray<int32> DefenseParticipatingStamina;

	FGuid LogId;
	int32 TurnIndex = 0;

	FName AttackingOwnerId = NAME_None;
	FName DefendingOwnerId = NAME_None;

	TArray<FName> InvolvedCardIds;
	FName GoalScorerCardId = NAME_None;

	bool bAttackVictoryIsGoal = false;
	bool bDefenderVictoryIsMiss = false;
	bool bAttackEndsAfterResolution = false;
	bool bContinueResolution = false;
};

struct FMCODEX_API FThroughBallFeetPlanQueryResult
{
	bool bSuccess = false;

	EThroughBallFeetPlanQueryDecision Decision =
		EThroughBallFeetPlanQueryDecision::None;

	EThroughBallFeetPlanQueryErrorCode ErrorCode =
		EThroughBallFeetPlanQueryErrorCode::None;

	FString ErrorMessage;
	FName InvalidField = NAME_None;

	FThroughBallFeetPlanQueryInput Input;

	FPlayerCardRuleSnapshotValidationResult
		ActiveGoalkeeperValidationResult;

	bool bHasFormulaPlan = false;
	FThroughBallFeetFormulaPlan FormulaPlan;
};

class FMCODEX_API FThroughBallFeetPlanQuery final
{
public:
	static FThroughBallFeetPlanQueryResult Evaluate(
		const FThroughBallFeetPlanQueryInput& Input);
};
