#include "MatchPlayCurrentAttackThroughBallOneOnOneShotChoiceSelectionLegality.h"

#include "MatchPlayCurrentAttackResolveThroughBallAntiOffsideDecisionOrchestrator.h"
#include "MatchPlayCurrentAttackResolveThroughBallBehindDefenseP2DecisionOrchestrator.h"

namespace MatchPlayCurrentAttackThroughBallOneOnOneShotChoiceSelectionLegality
{
	using FResult =
		FMatchPlayCurrentAttackThroughBallOneOnOneShotChoiceSelectionLegalityResult;
	using EError =
		EMatchPlayCurrentAttackThroughBallOneOnOneShotChoiceSelectionErrorCode;
	using EChoice = EMatchPlayThroughBallOneOnOneShotChoice;
	using EPhase = EMatchPlayCurrentAttackPostRouteRollPhase;

	void SetFailure(
		FResult& Result,
		const EError ErrorCode,
		const FString& ErrorMessage)
	{
		Result.ErrorCode = ErrorCode;
		Result.ErrorMessage = ErrorMessage;
	}

	bool IsPlayerSide(const EInitialTurnOrderPlayer Side)
	{
		return Side == EInitialTurnOrderPlayer::PlayerA
			|| Side == EInitialTurnOrderPlayer::PlayerB;
	}

	bool IsKnownChoice(const EChoice Choice)
	{
		return Choice == EChoice::None
			|| Choice == EChoice::ChipShot
			|| Choice == EChoice::DirectShot;
	}
}

FMatchPlayCurrentAttackThroughBallOneOnOneShotChoiceSelectionLegalityResult
FMatchPlayCurrentAttackThroughBallOneOnOneShotChoiceSelectionLegalityEvaluator
	::Evaluate(
		const FMatchPlayState& BeforeState,
		const
			FMatchPlayCurrentAttackThroughBallOneOnOneShotChoiceSelectionRequest&
				Request,
		const FSkillRuleSnapshotSet* SkillRuleSet)
{
	using namespace
		MatchPlayCurrentAttackThroughBallOneOnOneShotChoiceSelectionLegality;

	FResult Result;
	Result.Request = Request;
	if (!BeforeState.RuntimeState.bIsInitialized)
	{
		SetFailure(
			Result,
			EError::MatchPlayStateNotInitialized,
			TEXT("OneOnOne Shot Choice requires initialized MatchPlay State."));
		return Result;
	}
	if (!BeforeState.bHasCurrentAttack)
	{
		SetFailure(
			Result,
			EError::NoCurrentAttack,
			TEXT("OneOnOne Shot Choice requires an active CurrentAttack."));
		return Result;
	}
	if (BeforeState.CurrentAttack.AttackSequence <= 0)
	{
		SetFailure(
			Result,
			EError::InvalidCurrentAttackSequence,
			TEXT("CurrentAttack AttackSequence must be positive."));
		return Result;
	}
	if (Request.AttackSequence <= 0)
	{
		SetFailure(
			Result,
			EError::InvalidRequestedAttackSequence,
			TEXT("Requested AttackSequence must be positive."));
		return Result;
	}
	if (Request.AttackSequence != BeforeState.CurrentAttack.AttackSequence)
	{
		SetFailure(
			Result,
			EError::AttackSequenceMismatch,
			TEXT("Requested AttackSequence does not match CurrentAttack."));
		return Result;
	}
	if (!BeforeState.CurrentAttack.bHasResolutionSession)
	{
		SetFailure(
			Result,
			EError::MissingResolutionSession,
			TEXT("OneOnOne Shot Choice requires a Resolution Session."));
		return Result;
	}

	Result.SessionStateValidationResult =
		FMatchPlayCurrentAttackResolutionSessionStateValidator::Validate(
			BeforeState);
	if (!Result.SessionStateValidationResult.bIsCanonical)
	{
		SetFailure(
			Result,
			EError::InvalidResolutionSessionState,
			Result.SessionStateValidationResult.ErrorMessage);
		return Result;
	}

	const FMatchPlayCurrentAttackResolutionSession& Session =
		BeforeState.CurrentAttack.ResolutionSession;
	if (Session.Stage != EMatchPlayCurrentAttackResolutionStage::RouteResolved)
	{
		SetFailure(
			Result,
			EError::RouteNotResolved,
			TEXT("OneOnOne Shot Choice requires RouteResolved."));
		return Result;
	}
	if (!Session.bHasActualBranch
		|| Session.ActualBranch.ActionType != ESkillRuleType::ThroughBall)
	{
		SetFailure(
			Result,
			EError::NotThroughBallResolution,
			TEXT("OneOnOne Shot Choice supports only ThroughBall resolution."));
		return Result;
	}
	if (!IsPlayerSide(Request.RequestingSide))
	{
		SetFailure(
			Result,
			EError::InvalidRequestingSide,
			TEXT("RequestingSide must be PlayerA or PlayerB."));
		return Result;
	}
	if (Request.RequestingSide != Session.Bundle.CurrentAttackingPlayer)
	{
		SetFailure(
			Result,
			EError::RequestingSideIsNotCurrentAttacker,
			TEXT("Only the current attacker may select the OneOnOne Shot Choice."));
		return Result;
	}
	if (!IsKnownChoice(Request.Choice) || Request.Choice == EChoice::None)
	{
		SetFailure(
			Result,
			EError::InvalidChoice,
			TEXT("OneOnOne Shot Choice must be ChipShot or DirectShot."));
		return Result;
	}
	if (Session.ThroughBallOneOnOneShotChoice != EChoice::None)
	{
		SetFailure(
			Result,
			EError::ChoiceAlreadySelected,
			TEXT("The OneOnOne Shot Choice has already been selected."));
		return Result;
	}
	if (Session.PostRouteRollProgress.Phase == EPhase::OneOnOneChipShot)
	{
		SetFailure(
			Result,
			EError::OneOnOneShotResolutionAlreadyStarted,
			TEXT("OneOnOne Shot resolution has already started."));
		return Result;
	}
	if (SkillRuleSet == nullptr)
	{
		SetFailure(
			Result,
			EError::SkillRuleSetUnavailable,
			TEXT("OneOnOne Shot Choice requires an authoritative SkillRuleSet."));
		return Result;
	}

	if (Session.ActualBranch.ThroughBall
		== EMatchPlayThroughBallActualBranch::AntiOffside)
	{
		if (Session.PostRouteRollProgress.Phase != EPhase::PrimaryBranch)
		{
			SetFailure(
				Result,
				EError::IncompleteSourceProvenance,
				TEXT("AntiOffside OneOnOne source provenance is incomplete."));
			return Result;
		}
		FMatchPlayCurrentAttackResolveThroughBallAntiOffsideDecisionRequest
			SourceRequest;
		SourceRequest.AttackSequence = Request.AttackSequence;
		Result.AntiOffsideRegenerationResult =
			FMatchPlayCurrentAttackResolveThroughBallAntiOffsideDecisionOrchestrator
				::Resolve(BeforeState, SourceRequest, SkillRuleSet, nullptr);
		Result.SourceRegenerationProviderCallCount =
			Result.AntiOffsideRegenerationResult.ProviderCallCount;
		if (!Result.AntiOffsideRegenerationResult.bSuccess)
		{
			SetFailure(
				Result,
				EError::SourceRegenerationFailed,
				Result.AntiOffsideRegenerationResult.ErrorMessage);
			return Result;
		}
		if (Result.AntiOffsideRegenerationResult.OutcomeResult.Decision
			!= EThroughBallAntiOffsideOutcomeDecision::OneOnOneRequired)
		{
			SetFailure(
				Result,
				EError::SourceDoesNotRequireOneOnOne,
				TEXT("Canonical AntiOffside outcome does not require OneOnOne."));
			return Result;
		}
	}
	else if (Session.ActualBranch.ThroughBall
		== EMatchPlayThroughBallActualBranch::BehindDefense)
	{
		if (Session.PostRouteRollProgress.Phase != EPhase::BehindDefenseP2)
		{
			SetFailure(
				Result,
				EError::IncompleteSourceProvenance,
				TEXT("BehindDefense P2 OneOnOne source provenance is incomplete."));
			return Result;
		}
		Result.BehindDefenseP2RegenerationResult =
			FMatchPlayCurrentAttackResolveThroughBallBehindDefenseP2DecisionOrchestrator
				::Resolve(BeforeState, SkillRuleSet, nullptr);
		Result.SourceRegenerationProviderCallCount =
			Result.BehindDefenseP2RegenerationResult.ProviderCallCount;
		if (!Result.BehindDefenseP2RegenerationResult.bSuccess)
		{
			SetFailure(
				Result,
				EError::SourceRegenerationFailed,
				Result.BehindDefenseP2RegenerationResult.ErrorMessage);
			return Result;
		}
		if (Result.BehindDefenseP2RegenerationResult.QueryResult.Decision
			!= EThroughBallBehindDefenseP2OutcomeDecision::OneOnOneRequired)
		{
			SetFailure(
				Result,
				EError::SourceDoesNotRequireOneOnOne,
				TEXT("Canonical BehindDefense P2 outcome does not require OneOnOne."));
			return Result;
		}
	}
	else
	{
		SetFailure(
			Result,
			EError::UnsupportedOneOnOneSource,
			TEXT("This ThroughBall branch cannot produce a supported OneOnOne source."));
		return Result;
	}

	if (Result.SourceRegenerationProviderCallCount != 0)
	{
		SetFailure(
			Result,
			EError::SourceRegenerationConsumedRng,
			TEXT("OneOnOne source regeneration must consume zero RNG."));
		return Result;
	}

	Result.bIsLegal = true;
	Result.ErrorCode = EError::None;
	Result.ErrorMessage.Empty();
	return Result;
}
