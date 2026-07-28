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
			== EMatchPlayCurrentAttackSelectionStage::AwaitingHelper)
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
		Result.Binding.AttackSequence = CurrentAttack.AttackSequence;
		Result.Binding.CarrierCardId =
			CurrentAttack.SelectedAction.CarrierCardId;
		Result.Binding.MarkerCardId =
			CurrentAttack.SelectedAction.MarkerCardId;
		Result.Binding.SkillId =
			CurrentAttack.SelectedAction.SkillId;
		Result.Binding.ActionType =
			CurrentAttack.SelectedAction.ActionType;
		Result.bSuccess = true;
		Result.ErrorCode =
			EMatchPlayCurrentAttackResolutionBindingErrorCode::None;
		Result.ErrorMessage.Empty();
		return Result;
	}

	SetError(
		Result,
		EMatchPlayCurrentAttackResolutionBindingErrorCode
			::InvalidSelectionState,
		TEXT("Current attack has no implemented complete selection stage."));
	return Result;
}
