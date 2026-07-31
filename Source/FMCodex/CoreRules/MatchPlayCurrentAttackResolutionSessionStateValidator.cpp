#include "MatchPlayCurrentAttackResolutionSessionStateValidator.h"

#include "MatchPlayElectiveBranchIntentRules.h"
#include "PlayerCardRuleSnapshotValidator.h"

namespace MatchPlayCurrentAttackResolutionSessionStateValidatorImplementation
{
	bool IsPlayerSide(const EInitialTurnOrderPlayer Side)
	{
		return Side == EInitialTurnOrderPlayer::PlayerA
			|| Side == EInitialTurnOrderPlayer::PlayerB;
	}

	bool AreOpposingSides(
		const EInitialTurnOrderPlayer Attacker,
		const EInitialTurnOrderPlayer Defender)
	{
		return IsPlayerSide(Attacker)
			&& IsPlayerSide(Defender)
			&& Attacker != Defender;
	}

	bool IsSupportedActionType(const ESkillRuleType ActionType)
	{
		return ActionType == ESkillRuleType::LongShot
			|| ActionType == ESkillRuleType::CutInsideShot
			|| ActionType == ESkillRuleType::PassControl
			|| ActionType == ESkillRuleType::Cross
			|| ActionType == ESkillRuleType::ThroughBall;
	}

	bool RequiresRunner(const ESkillRuleType ActionType)
	{
		return ActionType == ESkillRuleType::PassControl
			|| ActionType == ESkillRuleType::Cross
			|| ActionType == ESkillRuleType::ThroughBall;
	}

	bool AreValuesDefault(
		const FMatchPlayBoundActionNormalizedParticipantValues& Values)
	{
		const FMatchPlayBoundActionNormalizedParticipantValues DefaultValues;
		return FMatchPlayBoundActionNormalizedParticipantValues
			::StaticStruct()->CompareScriptStruct(
				&Values,
				&DefaultValues,
				0);
	}

	bool AreValuesInRange(
		const FMatchPlayBoundActionNormalizedParticipantValues& Values)
	{
		const int32 Minimum =
			FPlayerCardRuleSnapshotValidator::MinAttributeValue;
		const int32 Maximum =
			FPlayerCardRuleSnapshotValidator::MaxAttributeValue;
		const int32 AllValues[] = {
			Values.Shooting,
			Values.Dribbling,
			Values.Passing,
			Values.OffBall,
			Values.Marking,
			Values.Tackling,
			Values.Speed,
			Values.Strength,
			Values.Stamina,
			Values.LongShot
		};
		for (const int32 Value : AllValues)
		{
			if (Value < Minimum || Value > Maximum)
			{
				return false;
			}
		}
		return true;
	}

	bool IsParticipantCanonical(
		const FMatchPlayCurrentAttackResolutionSessionParticipant&
			Participant,
		const bool bExpectedPresent,
		const EInitialTurnOrderPlayer ExpectedSide,
		const FName ExpectedCardId)
	{
		if (Participant.bIsPresent != bExpectedPresent
			|| Participant.Side != ExpectedSide)
		{
			return false;
		}

		if (!bExpectedPresent)
		{
			return Participant.CardId.IsNone()
				&& ExpectedCardId.IsNone()
				&& AreValuesDefault(Participant.Values);
		}

		return !ExpectedCardId.IsNone()
			&& Participant.CardId == ExpectedCardId
			&& AreValuesInRange(Participant.Values);
	}

	bool DoesBindingMatchSelectedAction(
		const FMatchPlayCurrentAttackResolutionBindingValue& Binding,
		const FMatchPlayCurrentAttackSelectedAction& SelectedAction,
		const int64 AttackSequence)
	{
		return Binding.AttackSequence == AttackSequence
			&& Binding.CarrierCardId == SelectedAction.CarrierCardId
			&& Binding.MarkerCardId == SelectedAction.MarkerCardId
			&& Binding.SkillId == SelectedAction.SkillId
			&& Binding.ActionType == SelectedAction.ActionType
			&& Binding.RunnerCardId == SelectedAction.RunnerCardId
			&& Binding.bHasHelper == SelectedAction.bHasHelper
			&& Binding.HelperCardId == SelectedAction.HelperCardId
			&& Binding.ElectiveBranchIntent
				== SelectedAction.ElectiveBranchIntent;
	}

	bool IsBundleCanonical(
		const FMatchPlayCurrentAttackResolutionSessionBundle& Bundle,
		const FMatchPlayCurrentAttackState& CurrentAttack)
	{
		const FMatchPlayCurrentAttackResolutionBindingValue& Binding =
			Bundle.Binding;
		if (!IsSupportedActionType(Binding.ActionType)
			|| !DoesBindingMatchSelectedAction(
				Binding,
				CurrentAttack.SelectedAction,
				CurrentAttack.AttackSequence)
			|| !AreOpposingSides(
				Bundle.CurrentAttackingPlayer,
				Bundle.CurrentDefendingPlayer)
			|| !MatchPlayElectiveBranchIntentRules::IsLegalIntent(
				Binding.ActionType,
				Binding.ElectiveBranchIntent))
		{
			return false;
		}

		if (!IsParticipantCanonical(
				Bundle.Carrier,
				true,
				Bundle.CurrentAttackingPlayer,
				Binding.CarrierCardId)
			|| !IsParticipantCanonical(
				Bundle.Marker,
				true,
				Bundle.CurrentDefendingPlayer,
				Binding.MarkerCardId))
		{
			return false;
		}

		const bool bRequiresRunner = RequiresRunner(Binding.ActionType);
		if (Bundle.bHasRunner != bRequiresRunner
			|| !IsParticipantCanonical(
				Bundle.Runner,
				bRequiresRunner,
				Bundle.CurrentAttackingPlayer,
				bRequiresRunner
					? Binding.RunnerCardId
					: NAME_None))
		{
			return false;
		}

		if (Bundle.bHasHelper != Binding.bHasHelper
			|| !IsParticipantCanonical(
				Bundle.Helper,
				Binding.bHasHelper,
				Bundle.CurrentDefendingPlayer,
				Binding.bHasHelper
					? Binding.HelperCardId
					: NAME_None))
		{
			return false;
		}

		return true;
	}

	void SetFailure(
		FMatchPlayCurrentAttackResolutionSessionStateValidationResult&
			Result,
		const
			EMatchPlayCurrentAttackResolutionSessionStateValidationErrorCode
				ErrorCode,
		const FString& ErrorMessage)
	{
		Result.ErrorCode = ErrorCode;
		Result.ErrorMessage = ErrorMessage;
	}
}

FMatchPlayCurrentAttackResolutionSessionStateValidationResult
FMatchPlayCurrentAttackResolutionSessionStateValidator::Validate(
	const FMatchPlayCurrentAttackState& CurrentAttack,
	const FMatchPlayCurrentAttackResolutionSession* ProposedSession)
{
	using namespace
		MatchPlayCurrentAttackResolutionSessionStateValidatorImplementation;

	FMatchPlayCurrentAttackResolutionSessionStateValidationResult Result;
	const bool bValidatingProposedSession = ProposedSession != nullptr;
	if (!CurrentAttack.bHasResolutionSession
		&& !bValidatingProposedSession)
	{
		const FMatchPlayCurrentAttackResolutionSession DefaultSession;
		if (!FMatchPlayCurrentAttackResolutionSession::StaticStruct()
			->CompareScriptStruct(
				&CurrentAttack.ResolutionSession,
				&DefaultSession,
				0))
		{
			SetFailure(
				Result,
				EMatchPlayCurrentAttackResolutionSessionStateValidationErrorCode
					::AbsentSessionHasPayload,
				TEXT("An absent Resolution Session must have a default payload."));
			return Result;
		}

		Result.bIsCanonical = true;
		return Result;
	}

	if (bValidatingProposedSession)
	{
		const FMatchPlayCurrentAttackResolutionSession DefaultSession;
		if (CurrentAttack.bHasResolutionSession
			|| !FMatchPlayCurrentAttackResolutionSession::StaticStruct()
				->CompareScriptStruct(
					&CurrentAttack.ResolutionSession,
					&DefaultSession,
					0))
		{
			SetFailure(
				Result,
				EMatchPlayCurrentAttackResolutionSessionStateValidationErrorCode
					::AbsentSessionHasPayload,
				TEXT("A proposed Session requires canonical absent Session state."));
			return Result;
		}
	}

	const FMatchPlayCurrentAttackResolutionSession& Session =
		bValidatingProposedSession
			? *ProposedSession
			: CurrentAttack.ResolutionSession;
	if (Session.AttackSequence <= 0)
	{
		SetFailure(
			Result,
			EMatchPlayCurrentAttackResolutionSessionStateValidationErrorCode
				::InvalidSessionAttackSequence,
			TEXT("A present Resolution Session requires a positive AttackSequence."));
		return Result;
	}
	if (Session.AttackSequence != CurrentAttack.AttackSequence)
	{
		SetFailure(
			Result,
			EMatchPlayCurrentAttackResolutionSessionStateValidationErrorCode
				::SessionAttackSequenceMismatch,
			TEXT("Resolution Session AttackSequence must match CurrentAttack."));
		return Result;
	}
	if (Session.Stage
		!= EMatchPlayCurrentAttackResolutionStage::AwaitingRoute)
	{
		SetFailure(
			Result,
			EMatchPlayCurrentAttackResolutionSessionStateValidationErrorCode
				::InvalidResolutionStage,
			TEXT("A present foundation Session must be AwaitingRoute."));
		return Result;
	}
	if (CurrentAttack.Phase != EMatchPlayCurrentAttackPhase::Resolution)
	{
		SetFailure(
			Result,
			EMatchPlayCurrentAttackResolutionSessionStateValidationErrorCode
				::CurrentAttackNotInResolution,
			TEXT("A present Resolution Session requires Resolution phase."));
		return Result;
	}
	if (CurrentAttack.SelectionStage
		!= EMatchPlayCurrentAttackSelectionStage::ReadyForResolution)
	{
		SetFailure(
			Result,
			EMatchPlayCurrentAttackResolutionSessionStateValidationErrorCode
				::WrongSelectionStage,
			TEXT("A present Resolution Session requires ReadyForResolution."));
		return Result;
	}
	if (!IsBundleCanonical(Session.Bundle, CurrentAttack))
	{
		SetFailure(
			Result,
			EMatchPlayCurrentAttackResolutionSessionStateValidationErrorCode
				::InvalidSessionBundle,
			TEXT("Resolution Session bundle is not canonical."));
		return Result;
	}

	Result.bIsCanonical = true;
	return Result;
}
