#include "ThroughBallFeetFormulaResolutionExecutor.h"

namespace ThroughBallFeetFormulaResolutionExecutor
{
	const FName AssemblyResultField(TEXT("ResolverInputAssemblyResult"));
	const FName ErrorCodeField(TEXT("ErrorCode"));
	const FName ErrorMessageField(TEXT("ErrorMessage"));
	const FName InvalidFieldField(TEXT("InvalidField"));
	const FName HasResolverInputField(TEXT("bHasResolverInput"));
	const FName PlanFormulaTypeField(TEXT("FormulaPlan.FormulaType"));
	const FName ResolverFormulaTypeField(TEXT("ResolverInput.FormulaType"));
	const FName AttackerBaseValueField(
		TEXT("ResolverInput.Attacker.BaseValue"));
	const FName AttackerModifierField(
		TEXT("ResolverInput.Attacker.Modifier"));
	const FName AttackerComparePointField(
		TEXT("ResolverInput.Attacker.ComparePoint"));
	const FName AttackerD6RolledField(
		TEXT("ResolverInput.Attacker.bComparePointWasRolledOnD6"));
	const FName AttackerStaminaField(
		TEXT("ResolverInput.Attacker.ParticipatingStamina"));
	const FName DefenderBaseValueField(
		TEXT("ResolverInput.Defender.BaseValue"));
	const FName DefenderModifierField(
		TEXT("ResolverInput.Defender.Modifier"));
	const FName DefenderComparePointField(
		TEXT("ResolverInput.Defender.ComparePoint"));
	const FName DefenderD6RolledField(
		TEXT("ResolverInput.Defender.bComparePointWasRolledOnD6"));
	const FName DefenderStaminaField(
		TEXT("ResolverInput.Defender.ParticipatingStamina"));
	const FName GoalkeeperParticipationField(
		TEXT("ResolverInput.bGoalkeeperParticipated"));
	const FName LogIdMappingField(TEXT("ResolverInput.LogId"));
	const FName TurnIndexMappingField(TEXT("ResolverInput.TurnIndex"));
	const FName AttackerOwnerMappingField(
		TEXT("ResolverInput.AttackerPlayerId"));
	const FName DefenderOwnerMappingField(
		TEXT("ResolverInput.DefenderPlayerId"));
	const FName InvolvedCardIdsField(
		TEXT("ResolverInput.InvolvedCardIds"));

	void SetFailure(
		FThroughBallFeetFormulaResolutionExecutionResult& Result,
		const EThroughBallFeetFormulaResolutionExecutionErrorCode ErrorCode,
		const FString& ErrorMessage,
		const FName InvalidField)
	{
		Result.ErrorCode = ErrorCode;
		Result.ErrorMessage = ErrorMessage;
		Result.InvalidField = InvalidField;
	}

	bool RejectMismatch(
		FThroughBallFeetFormulaResolutionExecutionResult& Result,
		const bool bMatches,
		const FName InvalidField)
	{
		if (bMatches)
		{
			return false;
		}

		SetFailure(
			Result,
			EThroughBallFeetFormulaResolutionExecutionErrorCode
				::InvalidResolverInputAssemblyResult,
			TEXT("Resolver Input does not match the assembled Feet Formula Plan."),
			InvalidField);
		return true;
	}
}

FThroughBallFeetFormulaResolutionExecutionResult
FThroughBallFeetFormulaResolutionExecutor::Execute(
	const FThroughBallFeetFormulaResolutionExecutionInput& Input)
{
	using namespace ThroughBallFeetFormulaResolutionExecutor;

	FThroughBallFeetFormulaResolutionExecutionResult Result;
	Result.Input = Input;

	const FThroughBallFeetFormulaResolverInputAssemblyResult& AssemblyResult =
		Input.ResolverInputAssemblyResult;
	if (!AssemblyResult.bSuccess)
	{
		SetFailure(
			Result,
			EThroughBallFeetFormulaResolutionExecutionErrorCode
				::ResolverInputAssemblyFailed,
			TEXT("Through Ball Feet Resolver Input Assembly failed."),
			AssemblyResultField);
		return Result;
	}

	if (AssemblyResult.ErrorCode
		!= EThroughBallFeetFormulaResolverInputAssemblyErrorCode::None)
	{
		SetFailure(
			Result,
			EThroughBallFeetFormulaResolutionExecutionErrorCode
				::InvalidResolverInputAssemblyResult,
			TEXT("Successful Assembly Result must have ErrorCode None."),
			ErrorCodeField);
		return Result;
	}

	if (!AssemblyResult.ErrorMessage.IsEmpty())
	{
		SetFailure(
			Result,
			EThroughBallFeetFormulaResolutionExecutionErrorCode
				::InvalidResolverInputAssemblyResult,
			TEXT("Successful Assembly Result must have an empty ErrorMessage."),
			ErrorMessageField);
		return Result;
	}

	if (!AssemblyResult.InvalidField.IsNone())
	{
		SetFailure(
			Result,
			EThroughBallFeetFormulaResolutionExecutionErrorCode
				::InvalidResolverInputAssemblyResult,
			TEXT("Successful Assembly Result must not identify an invalid field."),
			InvalidFieldField);
		return Result;
	}

	if (!AssemblyResult.bHasResolverInput)
	{
		SetFailure(
			Result,
			EThroughBallFeetFormulaResolutionExecutionErrorCode
				::InvalidResolverInputAssemblyResult,
			TEXT("Successful Assembly Result must contain Resolver Input."),
			HasResolverInputField);
		return Result;
	}

	const FThroughBallFeetFormulaPlan& Plan =
		AssemblyResult.Input.FormulaPlan;
	const FFormulaResolverInput& ResolverInput =
		AssemblyResult.ResolverInput;

	if (Plan.FormulaType != EFormulaType::Finishing)
	{
		SetFailure(
			Result,
			EThroughBallFeetFormulaResolutionExecutionErrorCode
				::InvalidResolverInputAssemblyResult,
			TEXT("Feet Formula Plan must use Finishing."),
			PlanFormulaTypeField);
		return Result;
	}

	if (ResolverInput.FormulaType != EFormulaType::Finishing)
	{
		SetFailure(
			Result,
			EThroughBallFeetFormulaResolutionExecutionErrorCode
				::InvalidResolverInputAssemblyResult,
			TEXT("Feet Resolver Input must use Finishing."),
			ResolverFormulaTypeField);
		return Result;
	}

	if (RejectMismatch(
			Result,
			ResolverInput.FormulaType == Plan.FormulaType,
			ResolverFormulaTypeField)
		|| RejectMismatch(
			Result,
			ResolverInput.Attacker.BaseValue == Plan.AttackBaseValue,
			AttackerBaseValueField)
		|| RejectMismatch(
			Result,
			ResolverInput.Attacker.Modifier == Plan.AttackExternalModifier,
			AttackerModifierField)
		|| RejectMismatch(
			Result,
			ResolverInput.Attacker.ComparePoint == Plan.AttackD6,
			AttackerComparePointField)
		|| RejectMismatch(
			Result,
			ResolverInput.Attacker.bComparePointWasRolledOnD6,
			AttackerD6RolledField)
		|| RejectMismatch(
			Result,
			ResolverInput.Attacker.ParticipatingStamina
				== Plan.AttackParticipatingStamina,
			AttackerStaminaField)
		|| RejectMismatch(
			Result,
			ResolverInput.Defender.BaseValue == Plan.DefenseBaseValue,
			DefenderBaseValueField)
		|| RejectMismatch(
			Result,
			ResolverInput.Defender.Modifier == Plan.DefenseExternalModifier,
			DefenderModifierField)
		|| RejectMismatch(
			Result,
			ResolverInput.Defender.ComparePoint == Plan.DefenseD6,
			DefenderComparePointField)
		|| RejectMismatch(
			Result,
			ResolverInput.Defender.bComparePointWasRolledOnD6,
			DefenderD6RolledField)
		|| RejectMismatch(
			Result,
			ResolverInput.Defender.ParticipatingStamina
				== Plan.DefenseParticipatingStamina,
			DefenderStaminaField)
		|| RejectMismatch(
			Result,
			ResolverInput.bGoalkeeperParticipated
				== Plan.bHasActiveGoalkeeper,
			GoalkeeperParticipationField)
		|| RejectMismatch(
			Result,
			ResolverInput.LogId == Plan.LogId,
			LogIdMappingField)
		|| RejectMismatch(
			Result,
			ResolverInput.TurnIndex == Plan.TurnIndex,
			TurnIndexMappingField)
		|| RejectMismatch(
			Result,
			ResolverInput.AttackerPlayerId == Plan.AttackingOwnerId,
			AttackerOwnerMappingField)
		|| RejectMismatch(
			Result,
			ResolverInput.DefenderPlayerId == Plan.DefendingOwnerId,
			DefenderOwnerMappingField)
		|| RejectMismatch(
			Result,
			ResolverInput.InvolvedCardIds == Plan.InvolvedCardIds,
			InvolvedCardIdsField))
	{
		return Result;
	}

	if (!ResolverInput.LogId.IsValid())
	{
		SetFailure(
			Result,
			EThroughBallFeetFormulaResolutionExecutionErrorCode
				::InvalidResolverInputAssemblyResult,
			TEXT("Resolver Input LogId must be valid."),
			LogIdMappingField);
		return Result;
	}

	if (ResolverInput.TurnIndex < 0)
	{
		SetFailure(
			Result,
			EThroughBallFeetFormulaResolutionExecutionErrorCode
				::InvalidResolverInputAssemblyResult,
			TEXT("Resolver Input TurnIndex must not be negative."),
			TurnIndexMappingField);
		return Result;
	}

	if (ResolverInput.AttackerPlayerId.IsNone())
	{
		SetFailure(
			Result,
			EThroughBallFeetFormulaResolutionExecutionErrorCode
				::InvalidResolverInputAssemblyResult,
			TEXT("Resolver Input AttackerPlayerId must be valid."),
			AttackerOwnerMappingField);
		return Result;
	}

	if (ResolverInput.DefenderPlayerId.IsNone())
	{
		SetFailure(
			Result,
			EThroughBallFeetFormulaResolutionExecutionErrorCode
				::InvalidResolverInputAssemblyResult,
			TEXT("Resolver Input DefenderPlayerId must be valid."),
			DefenderOwnerMappingField);
		return Result;
	}

	if (ResolverInput.AttackerPlayerId == ResolverInput.DefenderPlayerId)
	{
		SetFailure(
			Result,
			EThroughBallFeetFormulaResolutionExecutionErrorCode
				::InvalidResolverInputAssemblyResult,
			TEXT("Resolver Input owner identities must differ."),
			DefenderOwnerMappingField);
		return Result;
	}

	Result.FormulaResolutionResult =
		UFormulaResolver::ResolveFormula(ResolverInput);
	Result.bHasFormulaResolution = true;
	Result.bSuccess = true;
	return Result;
}
