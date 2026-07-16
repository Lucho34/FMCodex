#include "ThroughBallBehindDefenseP1FormulaResolutionExecutor.h"

namespace ThroughBallBehindDefenseP1FormulaResolutionExecutor
{
	const FName AssemblyResultField(TEXT("ResolverInputAssemblyResult"));
	const FName ErrorCodeField(TEXT("ErrorCode"));
	const FName ErrorMessageField(TEXT("ErrorMessage"));
	const FName InvalidFieldField(TEXT("InvalidField"));
	const FName HasResolverInputField(TEXT("bHasResolverInput"));
	const FName PlanQuerySuccessField(TEXT("PlanQueryResult.bSuccess"));
	const FName PlanQueryErrorCodeField(TEXT("PlanQueryResult.ErrorCode"));
	const FName PlanQueryErrorMessageField(TEXT("PlanQueryResult.ErrorMessage"));
	const FName PlanQueryInvalidFieldField(TEXT("PlanQueryResult.InvalidField"));
	const FName PlanQueryDecisionField(TEXT("PlanQueryResult.Decision"));
	const FName HasFormulaPlanField(TEXT("PlanQueryResult.bHasFormulaPlan"));
	const FName AttackEndedField(TEXT("PlanQueryResult.bAttackEnded"));
	const FName ContinueResolutionField(TEXT("PlanQueryResult.bContinueResolution"));
	const FName RequiresP2Field(TEXT("PlanQueryResult.bRequiresP2"));
	const FName PlanFormulaTypeField(TEXT("FormulaPlan.FormulaType"));
	const FName RunnerIdField(TEXT("FormulaPlan.RunnerId"));
	const FName AttackerWinnerPolicyField(
		TEXT("FormulaPlan.bAttackerVictoryRequiresP2"));
	const FName DefenderWinnerPolicyField(
		TEXT("FormulaPlan.bDefenderVictoryEndsAttack"));
	const FName ResolverFormulaTypeField(TEXT("ResolverInput.FormulaType"));
	const FName AttackerBaseValueField(TEXT("ResolverInput.Attacker.BaseValue"));
	const FName AttackerModifierField(TEXT("ResolverInput.Attacker.Modifier"));
	const FName AttackerComparePointField(
		TEXT("ResolverInput.Attacker.ComparePoint"));
	const FName AttackerRolledField(
		TEXT("ResolverInput.Attacker.bComparePointWasRolledOnD6"));
	const FName AttackerStaminaField(
		TEXT("ResolverInput.Attacker.ParticipatingStamina"));
	const FName DefenderBaseValueField(TEXT("ResolverInput.Defender.BaseValue"));
	const FName DefenderModifierField(TEXT("ResolverInput.Defender.Modifier"));
	const FName DefenderComparePointField(
		TEXT("ResolverInput.Defender.ComparePoint"));
	const FName DefenderRolledField(
		TEXT("ResolverInput.Defender.bComparePointWasRolledOnD6"));
	const FName DefenderStaminaField(
		TEXT("ResolverInput.Defender.ParticipatingStamina"));
	const FName GoalkeeperParticipationField(
		TEXT("ResolverInput.bGoalkeeperParticipated"));
	const FName LogIdField(TEXT("ResolverInput.LogId"));
	const FName TurnIndexField(TEXT("ResolverInput.TurnIndex"));
	const FName AttackerOwnerField(TEXT("ResolverInput.AttackerPlayerId"));
	const FName DefenderOwnerField(TEXT("ResolverInput.DefenderPlayerId"));
	const FName InvolvedCardIdsField(TEXT("ResolverInput.InvolvedCardIds"));
	const FName FormulaResolutionTypeField(
		TEXT("FormulaResolutionResult.FormulaType"));
	const FName AttackerFinalValueField(
		TEXT("FormulaResolutionResult.AttackerFinalValue"));
	const FName DefenderFinalValueField(
		TEXT("FormulaResolutionResult.DefenderFinalValue"));
	const FName WinnerField(TEXT("FormulaResolutionResult.Winner"));
	const FName WinReasonField(TEXT("FormulaResolutionResult.WinReason"));
	const FName IsGoalField(TEXT("FormulaResolutionResult.bIsGoal"));
	const FName ResolutionAttackEndedField(
		TEXT("FormulaResolutionResult.bAttackEnded"));
	const FName ResolutionContinueField(
		TEXT("FormulaResolutionResult.bContinueResolution"));
	const FName MatchLogIdField(
		TEXT("FormulaResolutionResult.MatchLogEntry.LogId"));
	const FName MatchLogTurnIndexField(
		TEXT("FormulaResolutionResult.MatchLogEntry.TurnIndex"));
	const FName MatchLogFormulaTypeField(
		TEXT("FormulaResolutionResult.MatchLogEntry.FormulaType"));
	const FName MatchLogActingPlayerField(
		TEXT("FormulaResolutionResult.MatchLogEntry.ActingPlayerId"));
	const FName MatchLogCardsField(
		TEXT("FormulaResolutionResult.MatchLogEntry.InvolvedCardIds"));

	void SetFailure(
		FThroughBallBehindDefenseP1FormulaResolutionExecutionResult& Result,
		const EThroughBallBehindDefenseP1FormulaResolutionExecutionErrorCode ErrorCode,
		const FString& ErrorMessage,
		const FName InvalidField)
	{
		Result.ErrorCode = ErrorCode;
		Result.ErrorMessage = ErrorMessage;
		Result.InvalidField = InvalidField;
	}

	bool RejectAssemblyMismatch(
		FThroughBallBehindDefenseP1FormulaResolutionExecutionResult& Result,
		const bool bMatches,
		const FName InvalidField)
	{
		if (bMatches)
		{
			return false;
		}

		SetFailure(
			Result,
			EThroughBallBehindDefenseP1FormulaResolutionExecutionErrorCode
				::InvalidResolverInputAssemblyResult,
			TEXT("Resolver Input Assembly Result is internally inconsistent."),
			InvalidField);
		return true;
	}

	bool RejectResolutionMismatch(
		FThroughBallBehindDefenseP1FormulaResolutionExecutionResult& Result,
		const bool bMatches,
		const FName InvalidField)
	{
		if (bMatches)
		{
			return false;
		}

		SetFailure(
			Result,
			EThroughBallBehindDefenseP1FormulaResolutionExecutionErrorCode
				::InvalidFormulaResolutionResult,
			TEXT("FormulaResolver returned an invalid Transition result."),
			InvalidField);
		return true;
	}
}

FThroughBallBehindDefenseP1FormulaResolutionExecutionResult
FThroughBallBehindDefenseP1FormulaResolutionExecutor::Execute(
	const FThroughBallBehindDefenseP1FormulaResolutionExecutionInput& Input)
{
	using namespace ThroughBallBehindDefenseP1FormulaResolutionExecutor;

	FThroughBallBehindDefenseP1FormulaResolutionExecutionResult Result;
	Result.Input = Input;

	const FThroughBallBehindDefenseP1FormulaResolverInputAssemblyResult&
		AssemblyResult = Input.ResolverInputAssemblyResult;
	if (!AssemblyResult.bSuccess)
	{
		SetFailure(
			Result,
			EThroughBallBehindDefenseP1FormulaResolutionExecutionErrorCode
				::ResolverInputAssemblyFailed,
			TEXT("Through Ball Behind Defense P1 Resolver Input Assembly failed."),
			AssemblyResultField);
		return Result;
	}

	if (AssemblyResult.ErrorCode
		!= EThroughBallBehindDefenseP1FormulaResolverInputAssemblyErrorCode::None)
	{
		SetFailure(
			Result,
			EThroughBallBehindDefenseP1FormulaResolutionExecutionErrorCode
				::InvalidResolverInputAssemblyResult,
			TEXT("Successful Assembly Result must have ErrorCode None."),
			ErrorCodeField);
		return Result;
	}

	if (!AssemblyResult.ErrorMessage.IsEmpty())
	{
		SetFailure(
			Result,
			EThroughBallBehindDefenseP1FormulaResolutionExecutionErrorCode
				::InvalidResolverInputAssemblyResult,
			TEXT("Successful Assembly Result must have an empty ErrorMessage."),
			ErrorMessageField);
		return Result;
	}

	if (!AssemblyResult.InvalidField.IsNone())
	{
		SetFailure(
			Result,
			EThroughBallBehindDefenseP1FormulaResolutionExecutionErrorCode
				::InvalidResolverInputAssemblyResult,
			TEXT("Successful Assembly Result must not identify an invalid field."),
			InvalidFieldField);
		return Result;
	}

	if (!AssemblyResult.bHasResolverInput)
	{
		SetFailure(
			Result,
			EThroughBallBehindDefenseP1FormulaResolutionExecutionErrorCode
				::InvalidResolverInputAssemblyResult,
			TEXT("Successful Assembly Result must contain Resolver Input."),
			HasResolverInputField);
		return Result;
	}

	const FThroughBallBehindDefenseP1PlanQueryResult& PlanQueryResult =
		AssemblyResult.Input.PlanQueryResult;
	if (!PlanQueryResult.bSuccess)
	{
		SetFailure(
			Result,
			EThroughBallBehindDefenseP1FormulaResolutionExecutionErrorCode
				::InvalidResolverInputAssemblyResult,
			TEXT("Successful Assembly must preserve a successful Plan Query Result."),
			PlanQuerySuccessField);
		return Result;
	}

	if (PlanQueryResult.ErrorCode
		!= EThroughBallBehindDefenseP1PlanQueryErrorCode::None)
	{
		SetFailure(
			Result,
			EThroughBallBehindDefenseP1FormulaResolutionExecutionErrorCode
				::InvalidResolverInputAssemblyResult,
			TEXT("Successful Plan Query Result must have ErrorCode None."),
			PlanQueryErrorCodeField);
		return Result;
	}

	if (!PlanQueryResult.ErrorMessage.IsEmpty())
	{
		SetFailure(
			Result,
			EThroughBallBehindDefenseP1FormulaResolutionExecutionErrorCode
				::InvalidResolverInputAssemblyResult,
			TEXT("Successful Plan Query Result must have an empty ErrorMessage."),
			PlanQueryErrorMessageField);
		return Result;
	}

	if (!PlanQueryResult.InvalidField.IsNone())
	{
		SetFailure(
			Result,
			EThroughBallBehindDefenseP1FormulaResolutionExecutionErrorCode
				::InvalidResolverInputAssemblyResult,
			TEXT("Successful Plan Query Result must not identify an invalid field."),
			PlanQueryInvalidFieldField);
		return Result;
	}

	if (PlanQueryResult.Decision
		!= EThroughBallBehindDefenseP1PlanQueryDecision::FormulaResolutionRequired)
	{
		SetFailure(
			Result,
			EThroughBallBehindDefenseP1FormulaResolutionExecutionErrorCode
				::InvalidResolverInputAssemblyResult,
			TEXT("P1 execution requires FormulaResolutionRequired."),
			PlanQueryDecisionField);
		return Result;
	}

	if (!PlanQueryResult.bHasFormulaPlan)
	{
		SetFailure(
			Result,
			EThroughBallBehindDefenseP1FormulaResolutionExecutionErrorCode
				::InvalidResolverInputAssemblyResult,
			TEXT("P1 execution requires a Formula Plan."),
			HasFormulaPlanField);
		return Result;
	}

	if (PlanQueryResult.bAttackEnded
		|| PlanQueryResult.bContinueResolution
		|| PlanQueryResult.bRequiresP2)
	{
		const FName InvalidField = PlanQueryResult.bAttackEnded
			? AttackEndedField
			: PlanQueryResult.bContinueResolution
				? ContinueResolutionField
				: RequiresP2Field;
		SetFailure(
			Result,
			EThroughBallBehindDefenseP1FormulaResolutionExecutionErrorCode
				::InvalidResolverInputAssemblyResult,
			TEXT("Plan Query Result must remain in its pre-execution state."),
			InvalidField);
		return Result;
	}

	const FThroughBallBehindDefenseP1FormulaPlan& Plan =
		PlanQueryResult.FormulaPlan;
	if (Plan.FormulaType != EFormulaType::Transition)
	{
		SetFailure(
			Result,
			EThroughBallBehindDefenseP1FormulaResolutionExecutionErrorCode
				::InvalidResolverInputAssemblyResult,
			TEXT("P1 Formula Plan must use Transition."),
			PlanFormulaTypeField);
		return Result;
	}

	if (Plan.RunnerId.IsNone())
	{
		SetFailure(
			Result,
			EThroughBallBehindDefenseP1FormulaResolutionExecutionErrorCode
				::InvalidResolverInputAssemblyResult,
			TEXT("P1 Formula Plan must preserve RunnerId."),
			RunnerIdField);
		return Result;
	}

	if (!Plan.bAttackerVictoryRequiresP2)
	{
		SetFailure(
			Result,
			EThroughBallBehindDefenseP1FormulaResolutionExecutionErrorCode
				::InvalidResolverInputAssemblyResult,
			TEXT("Attacker victory must require P2."),
			AttackerWinnerPolicyField);
		return Result;
	}

	if (!Plan.bDefenderVictoryEndsAttack)
	{
		SetFailure(
			Result,
			EThroughBallBehindDefenseP1FormulaResolutionExecutionErrorCode
				::InvalidResolverInputAssemblyResult,
			TEXT("Defender victory must end the attack."),
			DefenderWinnerPolicyField);
		return Result;
	}

	const FFormulaResolverInput& ResolverInput = AssemblyResult.ResolverInput;
	if (RejectAssemblyMismatch(Result, ResolverInput.FormulaType == Plan.FormulaType, ResolverFormulaTypeField)
		|| RejectAssemblyMismatch(Result, ResolverInput.Attacker.BaseValue == Plan.AttackBaseValue, AttackerBaseValueField)
		|| RejectAssemblyMismatch(Result, ResolverInput.Attacker.Modifier == Plan.AttackExternalModifier, AttackerModifierField)
		|| RejectAssemblyMismatch(Result, ResolverInput.Attacker.ComparePoint == Plan.AttackD6, AttackerComparePointField)
		|| RejectAssemblyMismatch(Result, ResolverInput.Attacker.bComparePointWasRolledOnD6, AttackerRolledField)
		|| RejectAssemblyMismatch(Result, ResolverInput.Attacker.ParticipatingStamina == Plan.AttackParticipatingStamina, AttackerStaminaField)
		|| RejectAssemblyMismatch(Result, ResolverInput.Defender.BaseValue == Plan.DefenseBaseValue, DefenderBaseValueField)
		|| RejectAssemblyMismatch(Result, ResolverInput.Defender.Modifier == Plan.DefenseExternalModifier, DefenderModifierField)
		|| RejectAssemblyMismatch(Result, ResolverInput.Defender.ComparePoint == Plan.DefenseD6, DefenderComparePointField)
		|| RejectAssemblyMismatch(Result, ResolverInput.Defender.bComparePointWasRolledOnD6, DefenderRolledField)
		|| RejectAssemblyMismatch(Result, ResolverInput.Defender.ParticipatingStamina == Plan.DefenseParticipatingStamina, DefenderStaminaField)
		|| RejectAssemblyMismatch(Result, !ResolverInput.bGoalkeeperParticipated, GoalkeeperParticipationField)
		|| RejectAssemblyMismatch(Result, ResolverInput.LogId == Plan.LogId, LogIdField)
		|| RejectAssemblyMismatch(Result, ResolverInput.TurnIndex == Plan.TurnIndex, TurnIndexField)
		|| RejectAssemblyMismatch(Result, ResolverInput.AttackerPlayerId == Plan.AttackingOwnerId, AttackerOwnerField)
		|| RejectAssemblyMismatch(Result, ResolverInput.DefenderPlayerId == Plan.DefendingOwnerId, DefenderOwnerField)
		|| RejectAssemblyMismatch(Result, ResolverInput.InvolvedCardIds == Plan.InvolvedCardIds, InvolvedCardIdsField))
	{
		return Result;
	}

	Result.FormulaResolutionResult =
		UFormulaResolver::ResolveFormula(ResolverInput);
	Result.bHasFormulaResolution = true;

	const FFormulaResolutionResult& Resolution = Result.FormulaResolutionResult;
	const bool bAttackerWinner = Resolution.Winner == EFormulaWinner::Attacker;
	const bool bDefenderWinner = Resolution.Winner == EFormulaWinner::Defender;
	if (RejectResolutionMismatch(Result, Resolution.FormulaType == EFormulaType::Transition, FormulaResolutionTypeField)
		|| RejectResolutionMismatch(Result, FMath::IsFinite(Resolution.AttackerFinalValue), AttackerFinalValueField)
		|| RejectResolutionMismatch(Result, FMath::IsFinite(Resolution.DefenderFinalValue), DefenderFinalValueField)
		|| RejectResolutionMismatch(Result, bAttackerWinner || bDefenderWinner, WinnerField)
		|| RejectResolutionMismatch(Result, Resolution.WinReason != EFormulaWinReason::None, WinReasonField)
		|| RejectResolutionMismatch(Result, !Resolution.bIsGoal, IsGoalField)
		|| RejectResolutionMismatch(Result, Resolution.bAttackEnded == bDefenderWinner, ResolutionAttackEndedField)
		|| RejectResolutionMismatch(Result, Resolution.bContinueResolution == bAttackerWinner, ResolutionContinueField)
		|| RejectResolutionMismatch(Result, Resolution.MatchLogEntry.LogId == ResolverInput.LogId, MatchLogIdField)
		|| RejectResolutionMismatch(Result, Resolution.MatchLogEntry.TurnIndex == ResolverInput.TurnIndex, MatchLogTurnIndexField)
		|| RejectResolutionMismatch(Result, Resolution.MatchLogEntry.FormulaType == ResolverInput.FormulaType, MatchLogFormulaTypeField)
		|| RejectResolutionMismatch(Result, Resolution.MatchLogEntry.ActingPlayerId == ResolverInput.AttackerPlayerId, MatchLogActingPlayerField)
		|| RejectResolutionMismatch(Result, Resolution.MatchLogEntry.InvolvedCardIds == ResolverInput.InvolvedCardIds, MatchLogCardsField))
	{
		return Result;
	}

	if (bDefenderWinner)
	{
		Result.Decision =
			EThroughBallBehindDefenseP1FormulaResolutionExecutionDecision
				::DefenderStoppedAttack;
		Result.bAttackEnded = true;
	}
	else if (bAttackerWinner)
	{
		Result.Decision =
			EThroughBallBehindDefenseP1FormulaResolutionExecutionDecision
				::P2Required;
		Result.bContinueResolution = true;
		Result.bRequiresP2 = true;
		Result.RunnerId = Plan.RunnerId;
	}
	else
	{
		SetFailure(
			Result,
			EThroughBallBehindDefenseP1FormulaResolutionExecutionErrorCode
				::InvalidFormulaResolutionResult,
			TEXT("Transition formula must produce Attacker or Defender winner."),
			WinnerField);
		return Result;
	}

	Result.bSuccess = true;
	return Result;
}
