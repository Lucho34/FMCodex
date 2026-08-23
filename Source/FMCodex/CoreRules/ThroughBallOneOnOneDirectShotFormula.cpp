#include "ThroughBallOneOnOneDirectShotFormula.h"

namespace ThroughBallOneOnOneDirectShotFormula
{
	using FResult = FThroughBallOneOnOneDirectShotFormulaResult;
	using EError = EThroughBallOneOnOneDirectShotFormulaErrorCode;

	void Fail(FResult& Result, EError Error, const TCHAR* Message, const TCHAR* Field)
	{
		Result.ErrorCode = Error;
		Result.ErrorMessage = Message;
		Result.InvalidField = FName(Field);
	}

	bool IsD6(const int32 Value)
	{
		return Value >= 1 && Value <= 6;
	}
}

FThroughBallOneOnOneDirectShotFormulaResult
FThroughBallOneOnOneDirectShotFormula::Resolve(
	const FThroughBallOneOnOneDirectShotFormulaPlan& Plan,
	const float AttackerTacticalPlayerModifier,
	const float DefenderTacticalPlayerModifier)
{
	using namespace ThroughBallOneOnOneDirectShotFormula;
	FResult Result;
	Result.Plan = Plan;
	if (Plan.Handoff.AttackingOwnerId.IsNone()
		|| Plan.Handoff.DefendingOwnerId.IsNone()
		|| Plan.Handoff.AttackingOwnerId == Plan.Handoff.DefendingOwnerId
		|| Plan.Handoff.ShooterCardId.IsNone())
	{
		Fail(Result, EError::InvalidHandoff, TEXT("DirectShot requires a valid OneOnOne handoff."), TEXT("Handoff"));
		return Result;
	}
	if (Plan.GoalkeeperCardId.IsNone()
		|| Plan.GoalkeeperCardId == Plan.Handoff.ShooterCardId)
	{
		Fail(Result, EError::InvalidGoalkeeperIdentity, TEXT("DirectShot requires a distinct canonical goalkeeper."), TEXT("GoalkeeperCardId"));
		return Result;
	}
	if (Plan.ShooterShooting < 0 || Plan.GoalkeeperOneOnOne < 0)
	{
		Fail(Result, EError::InvalidAttribute, TEXT("DirectShot attributes must not be negative."), TEXT("Attributes"));
		return Result;
	}
	if (!IsD6(Plan.AttackD6))
	{
		Fail(Result, EError::InvalidAttackD6, TEXT("DirectShot AttackD6 must be in [1, 6]."), TEXT("AttackD6"));
		return Result;
	}
	if (!IsD6(Plan.DefenseD6))
	{
		Fail(Result, EError::InvalidDefenseD6, TEXT("DirectShot DefenseD6 must be in [1, 6]."), TEXT("DefenseD6"));
		return Result;
	}
	if (!Plan.LogId.IsValid() || Plan.TurnIndex < 0)
	{
		Fail(Result, EError::InvalidLogContext, TEXT("DirectShot requires valid log context."), TEXT("LogId"));
		return Result;
	}
	if (Plan.InvolvedCardIds != TArray<FName>{Plan.Handoff.ShooterCardId, Plan.GoalkeeperCardId})
	{
		Fail(Result, EError::InvalidInvolvedCardIds, TEXT("DirectShot involved cards must be Shooter then Goalkeeper."), TEXT("InvolvedCardIds"));
		return Result;
	}

	FFormulaResolverInput& Input = Result.ResolverInput;
	Input.FormulaType = EFormulaType::Finishing;
	Input.Attacker.BaseValue = static_cast<float>(Plan.ShooterShooting);
	Input.Attacker.Modifier = 1.0f;
	Input.Attacker.TacticalPlayerModifier = AttackerTacticalPlayerModifier;
	Input.Attacker.ComparePoint = Plan.AttackD6;
	Input.Attacker.bComparePointWasRolledOnD6 = true;
	Input.Defender.BaseValue = static_cast<float>(Plan.GoalkeeperOneOnOne);
	Input.Defender.Modifier = Plan.bGoalkeeperActivated
		? UFormulaResolver::CalculateGoalkeeperHalf(Plan.GoalkeeperOneOnOne)
		: 0.0f;
	Input.Defender.TacticalPlayerModifier = DefenderTacticalPlayerModifier;
	Input.Defender.ComparePoint = Plan.DefenseD6;
	Input.Defender.bComparePointWasRolledOnD6 = true;
	Input.bGoalkeeperParticipated = true;
	Input.LogId = Plan.LogId;
	Input.TurnIndex = Plan.TurnIndex;
	Input.AttackerPlayerId = Plan.Handoff.AttackingOwnerId;
	Input.DefenderPlayerId = Plan.Handoff.DefendingOwnerId;
	Input.InvolvedCardIds = Plan.InvolvedCardIds;
	Result.bHasResolverInput = true;

	Result.FormulaResolutionResult = UFormulaResolver::ResolveFormula(Input);
	Result.bHasFormulaResolution = true;
	const FFormulaResolutionResult& Formula = Result.FormulaResolutionResult;
	if (Formula.FormulaType != EFormulaType::Finishing
		|| Formula.Winner == EFormulaWinner::None
		|| !Formula.bAttackEnded
		|| Formula.bContinueResolution)
	{
		Fail(Result, EError::InvalidFormulaResult, TEXT("DirectShot Formula result must be terminal."), TEXT("FormulaResolutionResult"));
		return Result;
	}
	Result.Decision = Formula.Winner == EFormulaWinner::Attacker
		? EThroughBallOneOnOneDirectShotDecision::Goal
		: EThroughBallOneOnOneDirectShotDecision::Miss;
	if (Formula.bIsGoal != (Result.Decision == EThroughBallOneOnOneDirectShotDecision::Goal))
	{
		Fail(Result, EError::InvalidFormulaResult, TEXT("DirectShot Formula goal semantic is inconsistent."), TEXT("bIsGoal"));
		return Result;
	}
	Result.bSuccess = true;
	return Result;
}
