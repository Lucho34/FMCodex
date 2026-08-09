#include "MatchPlayCurrentAttackResolutionSessionStateValidator.h"

#include "MatchPlayCurrentAttackInitialRouteMappingQuery.h"
#include "MatchPlayCurrentAttackPostRouteRollProgressQuery.h"
#include "MatchPlayCurrentAttackSelectionStateValidator.h"
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

	EInitialTurnOrderPlayer GetDefendingPlayer(
		const EInitialTurnOrderPlayer Attacker)
	{
		if (Attacker == EInitialTurnOrderPlayer::PlayerA)
		{
			return EInitialTurnOrderPlayer::PlayerB;
		}
		if (Attacker == EInitialTurnOrderPlayer::PlayerB)
		{
			return EInitialTurnOrderPlayer::PlayerA;
		}
		return EInitialTurnOrderPlayer::None;
	}

	bool IsActionPreparationDefault(
		const FMatchPlayCurrentAttackActionPreparationState& Preparation)
	{
		const FMatchPlayCurrentAttackActionPreparationState DefaultPreparation;
		return FMatchPlayCurrentAttackActionPreparationState::StaticStruct()
			->CompareScriptStruct(
				&Preparation,
				&DefaultPreparation,
				0);
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

	bool IsActualBranchDefault(
		const FMatchPlayCurrentAttackActualBranch& ActualBranch)
	{
		const FMatchPlayCurrentAttackActualBranch DefaultActualBranch;
		return FMatchPlayCurrentAttackActualBranch::StaticStruct()
			->CompareScriptStruct(
				&ActualBranch,
				&DefaultActualBranch,
				0);
	}

	bool IsPostRouteRollProgressDefault(
		const FMatchPlayCurrentAttackPostRouteRollProgress& Progress)
	{
		const FMatchPlayCurrentAttackPostRouteRollProgress DefaultProgress;
		return FMatchPlayCurrentAttackPostRouteRollProgress::StaticStruct()
			->CompareScriptStruct(
				&Progress,
				&DefaultProgress,
				0);
	}

	bool IsKnownOneOnOneShotChoice(
		const EMatchPlayThroughBallOneOnOneShotChoice Choice)
	{
		return Choice == EMatchPlayThroughBallOneOnOneShotChoice::None
			|| Choice == EMatchPlayThroughBallOneOnOneShotChoice::ChipShot
			|| Choice == EMatchPlayThroughBallOneOnOneShotChoice::DirectShot;
	}

	bool ValidateOneOnOneShotChoice(
		const FMatchPlayCurrentAttackResolutionSession& Session,
		const FMatchPlayCurrentAttackPostRouteRollProgressResult&
			ProgressResult,
		FMatchPlayCurrentAttackResolutionSessionStateValidationResult& Result)
	{
		const EMatchPlayThroughBallOneOnOneShotChoice Choice =
			Session.ThroughBallOneOnOneShotChoice;
		if (!IsKnownOneOnOneShotChoice(Choice))
		{
			SetFailure(
				Result,
				EMatchPlayCurrentAttackResolutionSessionStateValidationErrorCode
					::InvalidOneOnOneShotChoice,
				TEXT("ThroughBall OneOnOne Shot Choice must be a known enum value."));
			return false;
		}

		const EMatchPlayCurrentAttackPostRouteRollPhase Phase =
			Session.PostRouteRollProgress.Phase;
		if (Phase == EMatchPlayCurrentAttackPostRouteRollPhase::OneOnOneChipShot)
		{
			if (Choice
				!= EMatchPlayThroughBallOneOnOneShotChoice::ChipShot)
			{
				SetFailure(
					Result,
					EMatchPlayCurrentAttackResolutionSessionStateValidationErrorCode
						::OneOnOneChipShotPhaseChoiceMismatch,
					TEXT("OneOnOneChipShot phase requires the accepted ChipShot choice."));
				return false;
			}
			return true;
		}

		if (Choice == EMatchPlayThroughBallOneOnOneShotChoice::None)
		{
			return true;
		}

		const bool bIsThroughBall =
			Session.Bundle.Binding.ActionType == ESkillRuleType::ThroughBall
			&& Session.ActualBranch.ActionType == ESkillRuleType::ThroughBall;
		const bool bIsAntiOffsideSource = bIsThroughBall
			&& Session.ActualBranch.ThroughBall
				== EMatchPlayThroughBallActualBranch::AntiOffside
			&& Phase == EMatchPlayCurrentAttackPostRouteRollPhase::PrimaryBranch;
		const bool bIsBehindDefenseP2Source = bIsThroughBall
			&& Session.ActualBranch.ThroughBall
				== EMatchPlayThroughBallActualBranch::BehindDefense
			&& Phase == EMatchPlayCurrentAttackPostRouteRollPhase::BehindDefenseP2;
		if ((!bIsAntiOffsideSource && !bIsBehindDefenseP2Source)
			|| !ProgressResult.bContractComplete)
		{
			SetFailure(
				Result,
				EMatchPlayCurrentAttackResolutionSessionStateValidationErrorCode
					::OneOnOneShotChoiceRequiresCanonicalSource,
				TEXT("An accepted OneOnOne Shot Choice requires complete AntiOffside or BehindDefense P2 source progress."));
			return false;
		}

		return true;
	}

	bool ValidateActualBranchPayload(
		const FMatchPlayCurrentAttackResolutionSession& Session,
		FMatchPlayCurrentAttackResolutionSessionStateValidationResult&
			Result)
	{
		const FMatchPlayCurrentAttackActualBranch& ActualBranch =
			Session.ActualBranch;
		const ESkillRuleType ActionType =
			Session.Bundle.Binding.ActionType;
		if (ActualBranch.ActionType != ActionType)
		{
			SetFailure(
				Result,
				EMatchPlayCurrentAttackResolutionSessionStateValidationErrorCode
					::ActualBranchActionMismatch,
				TEXT("Actual Branch ActionType must match the Session Bundle."));
			return false;
		}

		bool bActivePayloadValid = false;
		bool bInactivePayloadsDefault = false;
		switch (ActionType)
		{
		case ESkillRuleType::LongShot:
			bActivePayloadValid = ActualBranch.LongShot
				== EMatchPlayLongShotActualBranch::DirectShot
				|| ActualBranch.LongShot
					== EMatchPlayLongShotActualBranch::DeadCorner;
			bInactivePayloadsDefault = ActualBranch.CutInsideShot
					== EMatchPlayCutInsideShotActualBranch::None
				&& ActualBranch.Cross
					== EMatchPlayCrossActualBranch::None
				&& ActualBranch.PassControl
					== EMatchPlayPassControlActualBranch::None
				&& ActualBranch.ThroughBall
					== EMatchPlayThroughBallActualBranch::None;
			break;

		case ESkillRuleType::CutInsideShot:
			bActivePayloadValid = ActualBranch.CutInsideShot
				== EMatchPlayCutInsideShotActualBranch::DirectShot
				|| ActualBranch.CutInsideShot
					== EMatchPlayCutInsideShotActualBranch::DeadCorner;
			bInactivePayloadsDefault = ActualBranch.LongShot
					== EMatchPlayLongShotActualBranch::None
				&& ActualBranch.Cross
					== EMatchPlayCrossActualBranch::None
				&& ActualBranch.PassControl
					== EMatchPlayPassControlActualBranch::None
				&& ActualBranch.ThroughBall
					== EMatchPlayThroughBallActualBranch::None;
			break;

		case ESkillRuleType::Cross:
			bActivePayloadValid = ActualBranch.Cross
				== EMatchPlayCrossActualBranch::High
				|| ActualBranch.Cross
					== EMatchPlayCrossActualBranch::Low;
			bInactivePayloadsDefault = ActualBranch.LongShot
					== EMatchPlayLongShotActualBranch::None
				&& ActualBranch.CutInsideShot
					== EMatchPlayCutInsideShotActualBranch::None
				&& ActualBranch.PassControl
					== EMatchPlayPassControlActualBranch::None
				&& ActualBranch.ThroughBall
					== EMatchPlayThroughBallActualBranch::None;
			break;

		case ESkillRuleType::PassControl:
			bActivePayloadValid = ActualBranch.PassControl
				== EMatchPlayPassControlActualBranch::PassAdvance
				|| ActualBranch.PassControl
					== EMatchPlayPassControlActualBranch::DribbleAdvance
				|| ActualBranch.PassControl
					== EMatchPlayPassControlActualBranch::RunAdvance;
			bInactivePayloadsDefault = ActualBranch.LongShot
					== EMatchPlayLongShotActualBranch::None
				&& ActualBranch.CutInsideShot
					== EMatchPlayCutInsideShotActualBranch::None
				&& ActualBranch.Cross
					== EMatchPlayCrossActualBranch::None
				&& ActualBranch.ThroughBall
					== EMatchPlayThroughBallActualBranch::None;
			break;

		case ESkillRuleType::ThroughBall:
			bActivePayloadValid = ActualBranch.ThroughBall
				== EMatchPlayThroughBallActualBranch::Feet
				|| ActualBranch.ThroughBall
					== EMatchPlayThroughBallActualBranch::BehindDefense
				|| ActualBranch.ThroughBall
					== EMatchPlayThroughBallActualBranch::AntiOffside;
			bInactivePayloadsDefault = ActualBranch.LongShot
					== EMatchPlayLongShotActualBranch::None
				&& ActualBranch.CutInsideShot
					== EMatchPlayCutInsideShotActualBranch::None
				&& ActualBranch.Cross
					== EMatchPlayCrossActualBranch::None
				&& ActualBranch.PassControl
					== EMatchPlayPassControlActualBranch::None;
			break;

		case ESkillRuleType::None:
		default:
			break;
		}

		if (!bActivePayloadValid)
		{
			SetFailure(
				Result,
				EMatchPlayCurrentAttackResolutionSessionStateValidationErrorCode
					::InvalidActiveBranch,
				TEXT("The active Actual Branch payload is invalid or None."));
			return false;
		}
		if (!bInactivePayloadsDefault)
		{
			SetFailure(
				Result,
				EMatchPlayCurrentAttackResolutionSessionStateValidationErrorCode
					::NonDefaultInactiveBranchPayload,
				TEXT("All inactive Actual Branch payloads must be default."));
			return false;
		}

		return true;
	}

	bool ValidateRouteState(
		const FMatchPlayCurrentAttackResolutionSession& Session,
		FMatchPlayCurrentAttackResolutionSessionStateValidationResult&
			Result)
	{
		if (Session.Stage
			== EMatchPlayCurrentAttackResolutionStage::AwaitingRoute)
		{
			if (Session.ThroughBallOneOnOneShotChoice
				!= EMatchPlayThroughBallOneOnOneShotChoice::None)
			{
				SetFailure(
					Result,
					EMatchPlayCurrentAttackResolutionSessionStateValidationErrorCode
						::UnexpectedOneOnOneShotChoiceWhileAwaitingRoute,
					TEXT("AwaitingRoute must not contain a OneOnOne Shot Choice."));
				return false;
			}
			if (Session.bHasActualBranch
				|| !IsActualBranchDefault(Session.ActualBranch))
			{
				SetFailure(
					Result,
					EMatchPlayCurrentAttackResolutionSessionStateValidationErrorCode
						::UnexpectedActualBranchWhileAwaitingRoute,
					TEXT("AwaitingRoute must not contain an Actual Branch."));
				return false;
			}
			if (!Session.InitialRouteRollRecords.IsEmpty())
			{
				SetFailure(
					Result,
					EMatchPlayCurrentAttackResolutionSessionStateValidationErrorCode
						::UnexpectedInitialRouteRollWhileAwaitingRoute,
					TEXT("AwaitingRoute must not contain Initial Route rolls."));
				return false;
			}
			if (!IsPostRouteRollProgressDefault(
					Session.PostRouteRollProgress))
			{
				SetFailure(
					Result,
					EMatchPlayCurrentAttackResolutionSessionStateValidationErrorCode
						::UnexpectedPostRouteRollProgressWhileAwaitingRoute,
					TEXT("AwaitingRoute must not contain post-route roll progress."));
				return false;
			}
			return true;
		}

		if (!Session.bHasActualBranch)
		{
			SetFailure(
				Result,
				EMatchPlayCurrentAttackResolutionSessionStateValidationErrorCode
					::MissingActualBranchForRouteResolved,
				TEXT("RouteResolved requires explicit Actual Branch presence."));
			return false;
		}
		if (IsActualBranchDefault(Session.ActualBranch))
		{
			SetFailure(
				Result,
				EMatchPlayCurrentAttackResolutionSessionStateValidationErrorCode
					::DefaultActualBranchForRouteResolved,
				TEXT("RouteResolved requires a non-default Actual Branch."));
			return false;
		}
		if (!ValidateActualBranchPayload(Session, Result))
		{
			return false;
		}

		const ESkillRuleType ActionType =
			Session.Bundle.Binding.ActionType;
		const bool bRequiresInitialRouteRoll =
			ActionType == ESkillRuleType::Cross
			|| ActionType == ESkillRuleType::PassControl
			|| ActionType == ESkillRuleType::ThroughBall;
		const int32 ExpectedRollCount =
			bRequiresInitialRouteRoll ? 1 : 0;
		if (Session.InitialRouteRollRecords.Num() != ExpectedRollCount)
		{
			SetFailure(
				Result,
				EMatchPlayCurrentAttackResolutionSessionStateValidationErrorCode
					::UnexpectedInitialRouteRollCount,
				TEXT("Initial Route roll count does not match the ActionType."));
			return false;
		}

		FMatchPlayCurrentAttackInitialRouteMappingInput MappingInput;
		MappingInput.ActionType = ActionType;
		MappingInput.Intent =
			Session.Bundle.Binding.ElectiveBranchIntent;
		if (bRequiresInitialRouteRoll)
		{
			const FMatchPlayCurrentAttackResolutionRollRecord& Record =
				Session.InitialRouteRollRecords[0];
			if (Record.Purpose
				!= EMatchPlayCurrentAttackResolutionRollPurpose::InitialRoute)
			{
				SetFailure(
					Result,
					EMatchPlayCurrentAttackResolutionSessionStateValidationErrorCode
						::InvalidRollPurpose,
					TEXT("Initial Route roll must use InitialRoute purpose."));
				return false;
			}
			if (Record.RawD6 < 1 || Record.RawD6 > 6)
			{
				SetFailure(
					Result,
					EMatchPlayCurrentAttackResolutionSessionStateValidationErrorCode
						::InvalidD6,
					TEXT("Initial Route RawD6 must be in range [1, 6]."));
				return false;
			}
			MappingInput.bHasInitialRouteD6 = true;
			MappingInput.InitialRouteD6 = Record.RawD6;
		}

		const FMatchPlayCurrentAttackInitialRouteMappingResult MappingResult =
			FMatchPlayCurrentAttackInitialRouteMappingQuery::Map(
				MappingInput);
		if (!MappingResult.bSuccess
			|| !FMatchPlayCurrentAttackActualBranch::StaticStruct()
				->CompareScriptStruct(
					&Session.ActualBranch,
					&MappingResult.ActualBranch,
					0))
		{
			SetFailure(
				Result,
				EMatchPlayCurrentAttackResolutionSessionStateValidationErrorCode
					::InitialRouteMappingMismatch,
				TEXT("Stored Intent, D6, and Actual Branch do not match."));
			return false;
		}

		const FMatchPlayCurrentAttackPostRouteRollProgressResult
			PostRouteProgressResult =
				FMatchPlayCurrentAttackPostRouteRollProgressQuery::Evaluate(
					Session);
		if (!PostRouteProgressResult.bIsCanonical)
		{
			SetFailure(
				Result,
				EMatchPlayCurrentAttackResolutionSessionStateValidationErrorCode
					::InvalidPostRouteRollProgress,
				FString::Printf(
					TEXT("Post-route roll progress is not canonical: %s"),
					*PostRouteProgressResult.ErrorMessage));
			return false;
		}
		if (!ValidateOneOnOneShotChoice(
				Session,
				PostRouteProgressResult,
				Result))
		{
			return false;
		}

		return true;
	}
}

FMatchPlayCurrentAttackResolutionSessionStateValidationResult
FMatchPlayCurrentAttackResolutionSessionStateValidator::Validate(
	const FMatchPlayState& State,
	const FMatchPlayCurrentAttackResolutionSession* ProposedSession)
{
	using namespace
		MatchPlayCurrentAttackResolutionSessionStateValidatorImplementation;

	FMatchPlayCurrentAttackResolutionSessionStateValidationResult Result;
	const FMatchPlayCurrentAttackState& CurrentAttack =
		State.CurrentAttack;
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

	if (!State.RuntimeState.bIsInitialized)
	{
		SetFailure(
			Result,
			EMatchPlayCurrentAttackResolutionSessionStateValidationErrorCode
				::MatchPlayStateNotInitialized,
			TEXT("A present or proposed Resolution Session requires initialized match play state."));
		return Result;
	}
	if (!State.bHasCurrentAttack)
	{
		SetFailure(
			Result,
			EMatchPlayCurrentAttackResolutionSessionStateValidationErrorCode
				::NoCurrentAttack,
			TEXT("A present or proposed Resolution Session requires CurrentAttack authority."));
		return Result;
	}
	if (CurrentAttack.AttackSequence <= 0)
	{
		SetFailure(
			Result,
			EMatchPlayCurrentAttackResolutionSessionStateValidationErrorCode
				::InvalidCurrentAttackSequence,
			TEXT("A present or proposed Resolution Session requires a positive CurrentAttack sequence."));
		return Result;
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
			!= EMatchPlayCurrentAttackResolutionStage::AwaitingRoute
		&& Session.Stage
			!= EMatchPlayCurrentAttackResolutionStage::RouteResolved)
	{
		SetFailure(
			Result,
			EMatchPlayCurrentAttackResolutionSessionStateValidationErrorCode
				::InvalidResolutionStage,
			TEXT("A present Session must be AwaitingRoute or RouteResolved."));
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

	const FMatchPlayCurrentAttackSelectionStateValidationResult
		SelectionValidation =
			FMatchPlayCurrentAttackSelectionStateValidator::Validate(
				CurrentAttack);
	if (!SelectionValidation.bIsCanonical)
	{
		EMatchPlayCurrentAttackResolutionSessionStateValidationErrorCode
			ErrorCode =
				EMatchPlayCurrentAttackResolutionSessionStateValidationErrorCode
					::InvalidSelectionState;
		if (!CurrentAttack.bHasSelectedAction)
		{
			ErrorCode =
				EMatchPlayCurrentAttackResolutionSessionStateValidationErrorCode
					::MissingSelectedAction;
		}
		else if (!IsActionPreparationDefault(
			CurrentAttack.ActionPreparation))
		{
			ErrorCode =
				EMatchPlayCurrentAttackResolutionSessionStateValidationErrorCode
					::NonDefaultActionPreparation;
		}
		SetFailure(
			Result,
			ErrorCode,
			FString::Printf(
				TEXT("Resolution Session selection authority is not canonical: %s"),
				*SelectionValidation.ErrorMessage));
		return Result;
	}

	const EInitialTurnOrderPlayer RuntimeAttacker =
		State.RuntimeState.CurrentAttackingPlayer;
	if (!IsPlayerSide(RuntimeAttacker))
	{
		SetFailure(
			Result,
			EMatchPlayCurrentAttackResolutionSessionStateValidationErrorCode
				::InvalidRuntimeAttackingPlayer,
			TEXT("Resolution Session requires a valid runtime CurrentAttackingPlayer."));
		return Result;
	}
	if (Session.Bundle.CurrentAttackingPlayer != RuntimeAttacker)
	{
		SetFailure(
			Result,
			EMatchPlayCurrentAttackResolutionSessionStateValidationErrorCode
				::RuntimeAttackerMismatch,
			TEXT("Session Bundle CurrentAttackingPlayer does not match runtime authority."));
		return Result;
	}
	const EInitialTurnOrderPlayer RuntimeDefender =
		GetDefendingPlayer(RuntimeAttacker);
	if (Session.Bundle.CurrentDefendingPlayer != RuntimeDefender)
	{
		SetFailure(
			Result,
			EMatchPlayCurrentAttackResolutionSessionStateValidationErrorCode
				::RuntimeDefenderMismatch,
			TEXT("Session Bundle CurrentDefendingPlayer is not the authoritative opposing side."));
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
	if (!ValidateRouteState(Session, Result))
	{
		return Result;
	}

	Result.bIsCanonical = true;
	return Result;
}
