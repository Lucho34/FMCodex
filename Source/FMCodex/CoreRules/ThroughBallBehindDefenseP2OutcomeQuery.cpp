#include "ThroughBallBehindDefenseP2OutcomeQuery.h"

namespace ThroughBallBehindDefenseP2OutcomeQuery
{
	const FName P1ExecutionResultField(TEXT("P1ExecutionResult"));
	const FName P1ErrorCodeField(TEXT("P1ExecutionResult.ErrorCode"));
	const FName P1ErrorMessageField(TEXT("P1ExecutionResult.ErrorMessage"));
	const FName P1InvalidFieldField(TEXT("P1ExecutionResult.InvalidField"));
	const FName P1DecisionField(TEXT("P1ExecutionResult.Decision"));
	const FName P1AttackEndedField(TEXT("P1ExecutionResult.bAttackEnded"));
	const FName P1ContinueField(TEXT("P1ExecutionResult.bContinueResolution"));
	const FName P1RequiresP2Field(TEXT("P1ExecutionResult.bRequiresP2"));
	const FName HasFormulaResolutionField(
		TEXT("P1ExecutionResult.bHasFormulaResolution"));
	const FName FormulaTypeField(
		TEXT("P1ExecutionResult.FormulaResolutionResult.FormulaType"));
	const FName FormulaWinnerField(
		TEXT("P1ExecutionResult.FormulaResolutionResult.Winner"));
	const FName FormulaWinReasonField(
		TEXT("P1ExecutionResult.FormulaResolutionResult.WinReason"));
	const FName FormulaIsGoalField(
		TEXT("P1ExecutionResult.FormulaResolutionResult.bIsGoal"));
	const FName FormulaAttackEndedField(
		TEXT("P1ExecutionResult.FormulaResolutionResult.bAttackEnded"));
	const FName FormulaContinueField(
		TEXT("P1ExecutionResult.FormulaResolutionResult.bContinueResolution"));
	const FName AssemblySuccessField(
		TEXT("P1ExecutionResult.Input.ResolverInputAssemblyResult.bSuccess"));
	const FName AssemblyErrorCodeField(
		TEXT("P1ExecutionResult.Input.ResolverInputAssemblyResult.ErrorCode"));
	const FName AssemblyErrorMessageField(
		TEXT("P1ExecutionResult.Input.ResolverInputAssemblyResult.ErrorMessage"));
	const FName AssemblyInvalidFieldField(
		TEXT("P1ExecutionResult.Input.ResolverInputAssemblyResult.InvalidField"));
	const FName HasResolverInputField(
		TEXT("P1ExecutionResult.Input.ResolverInputAssemblyResult.bHasResolverInput"));
	const FName PlanSuccessField(
		TEXT("P1ExecutionResult.Input.ResolverInputAssemblyResult.Input.PlanQueryResult.bSuccess"));
	const FName PlanErrorCodeField(
		TEXT("P1ExecutionResult.Input.ResolverInputAssemblyResult.Input.PlanQueryResult.ErrorCode"));
	const FName PlanErrorMessageField(
		TEXT("P1ExecutionResult.Input.ResolverInputAssemblyResult.Input.PlanQueryResult.ErrorMessage"));
	const FName PlanInvalidFieldField(
		TEXT("P1ExecutionResult.Input.ResolverInputAssemblyResult.Input.PlanQueryResult.InvalidField"));
	const FName PlanDecisionField(
		TEXT("P1ExecutionResult.Input.ResolverInputAssemblyResult.Input.PlanQueryResult.Decision"));
	const FName HasFormulaPlanField(
		TEXT("P1ExecutionResult.Input.ResolverInputAssemblyResult.Input.PlanQueryResult.bHasFormulaPlan"));
	const FName RunnerIdField(TEXT("P1ExecutionResult.RunnerId"));
	const FName PlanRunnerIdField(
		TEXT("P1ExecutionResult.Input.ResolverInputAssemblyResult.Input.PlanQueryResult.FormulaPlan.RunnerId"));
	const FName P2DefenseD6Field(TEXT("P2DefenseD6"));

	constexpr int32 MinD6 = 1;
	constexpr int32 MaxD6 = 6;
	constexpr int32 OneOnOneMaxD6 = 3;

	void SetFailure(
		FThroughBallBehindDefenseP2OutcomeQueryResult& Result,
		const EThroughBallBehindDefenseP2OutcomeQueryErrorCode ErrorCode,
		const FString& ErrorMessage,
		const FName InvalidField)
	{
		Result.ErrorCode = ErrorCode;
		Result.ErrorMessage = ErrorMessage;
		Result.InvalidField = InvalidField;
	}

	bool RejectInvalidP1Result(
		FThroughBallBehindDefenseP2OutcomeQueryResult& Result,
		const bool bIsValid,
		const FName InvalidField)
	{
		if (bIsValid)
		{
			return false;
		}

		SetFailure(
			Result,
			EThroughBallBehindDefenseP2OutcomeQueryErrorCode
				::InvalidP1ExecutionResult,
			TEXT("Successful P1 Execution Result is inconsistent with P2 continuation."),
			InvalidField);
		return true;
	}

	bool IsD6InRange(const int32 D6)
	{
		return D6 >= MinD6 && D6 <= MaxD6;
	}
}

FThroughBallBehindDefenseP2OutcomeQueryResult
FThroughBallBehindDefenseP2OutcomeQuery::Evaluate(
	const FThroughBallBehindDefenseP2OutcomeQueryInput& Input)
{
	using namespace ThroughBallBehindDefenseP2OutcomeQuery;

	FThroughBallBehindDefenseP2OutcomeQueryResult Result;
	Result.Input = Input;

	const FThroughBallBehindDefenseP1FormulaResolutionExecutionResult& P1Result =
		Input.P1ExecutionResult;
	if (!P1Result.bSuccess)
	{
		SetFailure(
			Result,
			EThroughBallBehindDefenseP2OutcomeQueryErrorCode::P1ExecutionFailed,
			TEXT("Behind Defense P1 Execution must succeed before P2."),
			P1ExecutionResultField);
		return Result;
	}

	if (RejectInvalidP1Result(
			Result,
			P1Result.ErrorCode
				== EThroughBallBehindDefenseP1FormulaResolutionExecutionErrorCode::None,
			P1ErrorCodeField)
		|| RejectInvalidP1Result(
			Result,
			P1Result.ErrorMessage.IsEmpty(),
			P1ErrorMessageField)
		|| RejectInvalidP1Result(
			Result,
			P1Result.InvalidField.IsNone(),
			P1InvalidFieldField)
		|| RejectInvalidP1Result(
			Result,
			P1Result.Decision
				== EThroughBallBehindDefenseP1FormulaResolutionExecutionDecision
					::P2Required,
			P1DecisionField)
		|| RejectInvalidP1Result(
			Result,
			!P1Result.bAttackEnded,
			P1AttackEndedField)
		|| RejectInvalidP1Result(
			Result,
			P1Result.bContinueResolution,
			P1ContinueField)
		|| RejectInvalidP1Result(
			Result,
			P1Result.bRequiresP2,
			P1RequiresP2Field)
		|| RejectInvalidP1Result(
			Result,
			P1Result.bHasFormulaResolution,
			HasFormulaResolutionField))
	{
		return Result;
	}

	const FFormulaResolutionResult& FormulaResult =
		P1Result.FormulaResolutionResult;
	if (RejectInvalidP1Result(
			Result,
			FormulaResult.FormulaType == EFormulaType::Transition,
			FormulaTypeField)
		|| RejectInvalidP1Result(
			Result,
			FormulaResult.Winner == EFormulaWinner::Attacker,
			FormulaWinnerField)
		|| RejectInvalidP1Result(
			Result,
			FormulaResult.WinReason != EFormulaWinReason::None,
			FormulaWinReasonField)
		|| RejectInvalidP1Result(
			Result,
			!FormulaResult.bIsGoal,
			FormulaIsGoalField)
		|| RejectInvalidP1Result(
			Result,
			!FormulaResult.bAttackEnded,
			FormulaAttackEndedField)
		|| RejectInvalidP1Result(
			Result,
			FormulaResult.bContinueResolution,
			FormulaContinueField))
	{
		return Result;
	}

	const FThroughBallBehindDefenseP1FormulaResolverInputAssemblyResult&
		AssemblyResult = P1Result.Input.ResolverInputAssemblyResult;
	if (RejectInvalidP1Result(Result, AssemblyResult.bSuccess, AssemblySuccessField)
		|| RejectInvalidP1Result(
			Result,
			AssemblyResult.ErrorCode
				== EThroughBallBehindDefenseP1FormulaResolverInputAssemblyErrorCode::None,
			AssemblyErrorCodeField)
		|| RejectInvalidP1Result(
			Result,
			AssemblyResult.ErrorMessage.IsEmpty(),
			AssemblyErrorMessageField)
		|| RejectInvalidP1Result(
			Result,
			AssemblyResult.InvalidField.IsNone(),
			AssemblyInvalidFieldField)
		|| RejectInvalidP1Result(
			Result,
			AssemblyResult.bHasResolverInput,
			HasResolverInputField))
	{
		return Result;
	}

	const FThroughBallBehindDefenseP1PlanQueryResult& PlanResult =
		AssemblyResult.Input.PlanQueryResult;
	if (RejectInvalidP1Result(Result, PlanResult.bSuccess, PlanSuccessField)
		|| RejectInvalidP1Result(
			Result,
			PlanResult.ErrorCode
				== EThroughBallBehindDefenseP1PlanQueryErrorCode::None,
			PlanErrorCodeField)
		|| RejectInvalidP1Result(
			Result,
			PlanResult.ErrorMessage.IsEmpty(),
			PlanErrorMessageField)
		|| RejectInvalidP1Result(
			Result,
			PlanResult.InvalidField.IsNone(),
			PlanInvalidFieldField)
		|| RejectInvalidP1Result(
			Result,
			PlanResult.Decision
				== EThroughBallBehindDefenseP1PlanQueryDecision
					::FormulaResolutionRequired,
			PlanDecisionField)
		|| RejectInvalidP1Result(
			Result,
			PlanResult.bHasFormulaPlan,
			HasFormulaPlanField))
	{
		return Result;
	}

	if (RejectInvalidP1Result(Result, !P1Result.RunnerId.IsNone(), RunnerIdField)
		|| RejectInvalidP1Result(
			Result,
			!PlanResult.FormulaPlan.RunnerId.IsNone(),
			PlanRunnerIdField)
		|| RejectInvalidP1Result(
			Result,
			P1Result.RunnerId == PlanResult.FormulaPlan.RunnerId,
			RunnerIdField))
	{
		return Result;
	}

	if (!Input.bHasP2DefenseD6)
	{
		SetFailure(
			Result,
			EThroughBallBehindDefenseP2OutcomeQueryErrorCode::MissingP2DefenseD6,
			TEXT("An externally supplied P2DefenseD6 is required."),
			P2DefenseD6Field);
		return Result;
	}

	if (!IsD6InRange(Input.P2DefenseD6))
	{
		SetFailure(
			Result,
			EThroughBallBehindDefenseP2OutcomeQueryErrorCode::InvalidP2DefenseD6,
			TEXT("P2DefenseD6 must be in range [1, 6]."),
			P2DefenseD6Field);
		return Result;
	}

	if (Input.P2DefenseD6 <= OneOnOneMaxD6)
	{
		Result.Decision =
			EThroughBallBehindDefenseP2OutcomeDecision::OneOnOneRequired;
		Result.bContinueResolution = true;
		Result.bRequiresOneOnOne = true;
		Result.RunnerId = P1Result.RunnerId;
	}
	else
	{
		Result.Decision = EThroughBallBehindDefenseP2OutcomeDecision::Offside;
		Result.bAttackEnded = true;
	}

	Result.bSuccess = true;
	return Result;
}
