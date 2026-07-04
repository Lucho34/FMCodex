#include "SingleCardFormulaResolutionExecutor.h"

namespace SingleCardFormulaResolutionExecutor
{
	const FName AttackerSide(TEXT("Attacker"));
	const FName DefenderSide(TEXT("Defender"));

	void SetFailure(
		FSingleCardFormulaResolutionExecutionResult& Result,
		const ESingleCardFormulaResolutionExecutionErrorCode ErrorCode,
		const FString& ErrorMessage,
		const FName InvalidSide,
		const FName InvalidField)
	{
		Result.ErrorCode = ErrorCode;
		Result.ErrorMessage = ErrorMessage;
		Result.InvalidSide = InvalidSide;
		Result.InvalidField = InvalidField;
	}

	bool IsSupportedFormulaType(const EFormulaType FormulaType)
	{
		return FormulaType == EFormulaType::Transition
			|| FormulaType == EFormulaType::Finishing;
	}

	bool ValidateSideInput(
		const FFormulaSideInput& SideInput,
		FName& OutInvalidField,
		FString& OutErrorMessage)
	{
		if (!FMath::IsFinite(SideInput.BaseValue))
		{
			OutInvalidField = TEXT("BaseValue");
			OutErrorMessage = TEXT("BaseValue must be finite.");
			return false;
		}

		if (!FMath::IsFinite(SideInput.Modifier))
		{
			OutInvalidField = TEXT("Modifier");
			OutErrorMessage = TEXT("Modifier must be finite.");
			return false;
		}

		if (!SideInput.bComparePointWasRolledOnD6)
		{
			OutInvalidField = TEXT("bComparePointWasRolledOnD6");
			OutErrorMessage =
				TEXT("ComparePoint must be identified as an external D6 input.");
			return false;
		}

		if (SideInput.ComparePoint < 1 || SideInput.ComparePoint > 6)
		{
			OutInvalidField = TEXT("ComparePoint");
			OutErrorMessage = TEXT("External D6 ComparePoint must be between 1 and 6.");
			return false;
		}

		if (SideInput.ParticipatingStamina.Num() != 1)
		{
			OutInvalidField = TEXT("ParticipatingStamina");
			OutErrorMessage =
				TEXT("Single-card execution requires exactly one participating Stamina value per side.");
			return false;
		}

		const float UnroundedFinalValue =
			SideInput.BaseValue
			+ SideInput.Modifier
			+ static_cast<float>(SideInput.ComparePoint);
		if (!FMath::IsFinite(UnroundedFinalValue))
		{
			OutInvalidField = TEXT("BaseValue");
			OutErrorMessage =
				TEXT("BaseValue, Modifier, and ComparePoint must produce a finite final value.");
			return false;
		}

		return true;
	}
}

FSingleCardFormulaResolutionExecutionResult
FSingleCardFormulaResolutionExecutor::Execute(
	const FFormulaResolverInput& Input)
{
	FSingleCardFormulaResolutionExecutionResult Result;
	Result.ResolverInput = Input;

	if (!SingleCardFormulaResolutionExecutor::IsSupportedFormulaType(
			Input.FormulaType))
	{
		SingleCardFormulaResolutionExecutor::SetFailure(
			Result,
			ESingleCardFormulaResolutionExecutionErrorCode
				::UnsupportedFormulaType,
			TEXT("FormulaType must be Transition or Finishing."),
			NAME_None,
			TEXT("FormulaType"));
		return Result;
	}

	FName InvalidField = NAME_None;
	FString ErrorMessage;
	if (!SingleCardFormulaResolutionExecutor::ValidateSideInput(
			Input.Attacker,
			InvalidField,
			ErrorMessage))
	{
		SingleCardFormulaResolutionExecutor::SetFailure(
			Result,
			ESingleCardFormulaResolutionExecutionErrorCode::InvalidInput,
			ErrorMessage,
			SingleCardFormulaResolutionExecutor::AttackerSide,
			InvalidField);
		return Result;
	}

	if (!SingleCardFormulaResolutionExecutor::ValidateSideInput(
			Input.Defender,
			InvalidField,
			ErrorMessage))
	{
		SingleCardFormulaResolutionExecutor::SetFailure(
			Result,
			ESingleCardFormulaResolutionExecutionErrorCode::InvalidInput,
			ErrorMessage,
			SingleCardFormulaResolutionExecutor::DefenderSide,
			InvalidField);
		return Result;
	}

	if (Input.TurnIndex < 0)
	{
		SingleCardFormulaResolutionExecutor::SetFailure(
			Result,
			ESingleCardFormulaResolutionExecutionErrorCode::InvalidInput,
			TEXT("TurnIndex must not be negative."),
			NAME_None,
			TEXT("TurnIndex"));
		return Result;
	}

	if (Input.InvolvedCardIds.Num() != 2
		|| Input.InvolvedCardIds[0].IsNone()
		|| Input.InvolvedCardIds[1].IsNone())
	{
		SingleCardFormulaResolutionExecutor::SetFailure(
			Result,
			ESingleCardFormulaResolutionExecutionErrorCode::InvalidInput,
			TEXT("InvolvedCardIds must contain valid attacker and defender CardIds in that order."),
			NAME_None,
			TEXT("InvolvedCardIds"));
		return Result;
	}

	Result.bExecuted = true;
	Result.FormulaResolutionResult =
		UFormulaResolver::ResolveFormula(Input);

	if (Result.FormulaResolutionResult.FormulaType != Input.FormulaType
		|| !FMath::IsFinite(
			Result.FormulaResolutionResult.AttackerFinalValue)
		|| !FMath::IsFinite(
			Result.FormulaResolutionResult.DefenderFinalValue))
	{
		SingleCardFormulaResolutionExecutor::SetFailure(
			Result,
			ESingleCardFormulaResolutionExecutionErrorCode
				::FormulaResolverFailed,
			TEXT("FormulaResolver returned an unusable result."),
			NAME_None,
			TEXT("FormulaResolutionResult"));
		return Result;
	}

	Result.bSuccess = true;
	return Result;
}
