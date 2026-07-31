#include "MatchPlayCurrentAttackResolutionBinding.h"

namespace MatchPlayCurrentAttackResolutionBindingImplementation
{
	void SetError(
		FMatchPlayCurrentAttackResolutionBindingResult& Result,
		const EMatchPlayCurrentAttackResolutionBindingErrorCode ErrorCode,
		const FString& ErrorMessage)
	{
		Result.ErrorCode = ErrorCode;
		Result.ErrorMessage = ErrorMessage;
	}

}

FMatchPlayCurrentAttackResolutionBindingResult
FMatchPlayCurrentAttackResolutionBinding::Query(
	const FMatchPlayState& MatchPlayState,
	const int64 AttackSequence)
{
	using namespace
		MatchPlayCurrentAttackResolutionBindingImplementation;

	FMatchPlayCurrentAttackResolutionBindingResult Result;
	Result.RequestedAttackSequence = AttackSequence;

	if (!MatchPlayState.RuntimeState.bIsInitialized)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackResolutionBindingErrorCode
				::MatchPlayStateNotInitialized,
			TEXT("Match play state must be initialized before resolution binding."));
		return Result;
	}

	if (!MatchPlayState.bHasCurrentAttack)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackResolutionBindingErrorCode
				::NoCurrentAttack,
			TEXT("Resolution binding requires an active current attack."));
		return Result;
	}

	const FMatchPlayCurrentAttackState& CurrentAttack =
		MatchPlayState.CurrentAttack;
	if (CurrentAttack.AttackSequence <= 0)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackResolutionBindingErrorCode
				::InvalidCurrentAttackSequence,
			TEXT("Current attack sequence must be greater than zero."));
		return Result;
	}

	if (AttackSequence != CurrentAttack.AttackSequence)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackResolutionBindingErrorCode
				::AttackSequenceMismatch,
			TEXT("Requested attack sequence does not match the current attack."));
		return Result;
	}

	if (CurrentAttack.Phase != EMatchPlayCurrentAttackPhase::Resolution)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackResolutionBindingErrorCode
				::CurrentAttackNotInResolution,
			TEXT("Current attack must be in Resolution phase for resolution binding."));
		return Result;
	}

	Result.SelectionStateValidationResult =
		FMatchPlayCurrentAttackSelectionStateValidator::Validate(
			CurrentAttack);
	if (!Result.SelectionStateValidationResult.bIsCanonical)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackResolutionBindingErrorCode
				::InvalidSelectionState,
			Result.SelectionStateValidationResult.ErrorMessage);
		return Result;
	}

	if (CurrentAttack.SelectionStage
			== EMatchPlayCurrentAttackSelectionStage::AwaitingCarrier
		|| CurrentAttack.SelectionStage
			== EMatchPlayCurrentAttackSelectionStage::AwaitingMarker
		|| CurrentAttack.SelectionStage
			== EMatchPlayCurrentAttackSelectionStage::AwaitingSkill
		|| CurrentAttack.SelectionStage
			== EMatchPlayCurrentAttackSelectionStage::AwaitingRunner
		|| CurrentAttack.SelectionStage
			== EMatchPlayCurrentAttackSelectionStage::AwaitingHelper
		|| CurrentAttack.SelectionStage
			== EMatchPlayCurrentAttackSelectionStage::AwaitingBranchIntent)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackResolutionBindingErrorCode
				::SelectionNotComplete,
			TEXT("Current attack participant and action selection is not complete."));
		return Result;
	}

	if (CurrentAttack.SelectionStage
		== EMatchPlayCurrentAttackSelectionStage::ReadyForResolution)
	{
		Result.ReadyValidationResult =
			FMatchPlayCurrentAttackReadyForResolutionValidator::Validate(
				MatchPlayState);
		if (!Result.ReadyValidationResult.bSuccess)
		{
			SetError(
				Result,
				EMatchPlayCurrentAttackResolutionBindingErrorCode
					::InvalidSelectionState,
				Result.ReadyValidationResult.ErrorMessage);
			return Result;
		}
		PopulateFromSuccessfulReadyValidation(
			MatchPlayState,
			AttackSequence,
			Result.ReadyValidationResult,
			Result);
		return Result;
	}

	SetError(
		Result,
		EMatchPlayCurrentAttackResolutionBindingErrorCode
			::InvalidSelectionState,
		TEXT("Current attack has no implemented complete selection stage."));
	return Result;
}

bool FMatchPlayCurrentAttackResolutionBinding
	::PopulateFromSuccessfulReadyValidation(
	const FMatchPlayState& MatchPlayState,
	const int64 AttackSequence,
	const FMatchPlayCurrentAttackReadyValidationResult&
		ReadyValidationResult,
	FMatchPlayCurrentAttackResolutionBindingResult& OutResult)
{
	using namespace
		MatchPlayCurrentAttackResolutionBindingImplementation;

	OutResult.RequestedAttackSequence = AttackSequence;
	OutResult.ReadyValidationResult = ReadyValidationResult;
	if (!ReadyValidationResult.bSuccess
		|| !MatchPlayState.bHasCurrentAttack
		|| MatchPlayState.CurrentAttack.AttackSequence != AttackSequence)
	{
		SetError(
			OutResult,
			EMatchPlayCurrentAttackResolutionBindingErrorCode
				::InvalidSelectionState,
			TEXT("Resolution Binding requires a successful Ready validation for the current attack."));
		return false;
	}

	const FMatchPlayCurrentAttackState& CurrentAttack =
		MatchPlayState.CurrentAttack;
	OutResult.Binding.AttackSequence = CurrentAttack.AttackSequence;
	OutResult.Binding.CarrierCardId =
		CurrentAttack.SelectedAction.CarrierCardId;
	OutResult.Binding.MarkerCardId =
		CurrentAttack.SelectedAction.MarkerCardId;
	OutResult.Binding.SkillId =
		CurrentAttack.SelectedAction.SkillId;
	OutResult.Binding.ActionType =
		CurrentAttack.SelectedAction.ActionType;
	OutResult.Binding.RunnerCardId =
		CurrentAttack.SelectedAction.RunnerCardId;
	OutResult.Binding.bHasHelper =
		CurrentAttack.SelectedAction.bHasHelper;
	OutResult.Binding.HelperCardId =
		CurrentAttack.SelectedAction.HelperCardId;
	OutResult.Binding.ElectiveBranchIntent =
		CurrentAttack.SelectedAction.ElectiveBranchIntent;
	OutResult.bSuccess = true;
	OutResult.ErrorCode =
		EMatchPlayCurrentAttackResolutionBindingErrorCode::None;
	OutResult.ErrorMessage.Empty();
	return true;
}
