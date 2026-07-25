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

	bool IsSupportedFrozenActionType(const ESkillRuleType ActionType)
	{
		return ActionType >= ESkillRuleType::LongShot
			&& ActionType <= ESkillRuleType::ThroughBall;
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

	const FMatchPlayCurrentAttackSelectedAction& SelectedAction =
		CurrentAttack.SelectedAction;
	if (!CurrentAttack.bHasSelectedAction)
	{
		const bool bCanonicalEmpty =
			SelectedAction.CarrierCardId.IsNone()
			&& SelectedAction.SkillId.IsNone()
			&& SelectedAction.ActionType == ESkillRuleType::None;
		SetError(
			Result,
			bCanonicalEmpty
				? EMatchPlayCurrentAttackResolutionBindingErrorCode
					::ActionNotSelected
				: EMatchPlayCurrentAttackResolutionBindingErrorCode
					::InvalidSelectedActionState,
			bCanonicalEmpty
				? TEXT("Current attack does not yet have a selected action.")
				: TEXT("Unselected current attack contains a non-empty selected-action payload."));
		return Result;
	}

	if (SelectedAction.CarrierCardId.IsNone()
		|| SelectedAction.SkillId.IsNone())
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackResolutionBindingErrorCode
				::InvalidSelectedActionState,
			TEXT("Selected action must contain non-empty carrier and skill identities."));
		return Result;
	}

	if (!IsSupportedFrozenActionType(SelectedAction.ActionType))
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackResolutionBindingErrorCode
				::UnsupportedActionType,
			TEXT("Selected action contains a None, unknown, or unsupported action type."));
		return Result;
	}

	Result.Binding.AttackSequence = CurrentAttack.AttackSequence;
	Result.Binding.CarrierCardId = SelectedAction.CarrierCardId;
	Result.Binding.SkillId = SelectedAction.SkillId;
	Result.Binding.ActionType = SelectedAction.ActionType;
	Result.bSuccess = true;
	Result.ErrorCode =
		EMatchPlayCurrentAttackResolutionBindingErrorCode::None;
	Result.ErrorMessage.Empty();
	return Result;
}
