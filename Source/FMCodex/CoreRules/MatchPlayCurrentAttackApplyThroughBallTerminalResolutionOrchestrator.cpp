#include "MatchPlayCurrentAttackApplyThroughBallTerminalResolutionOrchestrator.h"

#include "MatchPlayCurrentAttackResolveThroughBallAntiOffsideDecisionOrchestrator.h"
#include "MatchPlayCurrentAttackResolveThroughBallBehindDefenseP1DecisionOrPlanOrchestrator.h"
#include "MatchPlayCurrentAttackResolveThroughBallBehindDefenseP1FormulaOrchestrator.h"
#include "MatchPlayCurrentAttackResolveThroughBallFeetFormulaOrchestrator.h"
#include "MatchPlayCurrentAttackResolveThroughBallOneOnOneChipShotDecisionOrchestrator.h"
#include "MatchPlayCurrentAttackResolveThroughBallOneOnOneDirectShotFormulaOrchestrator.h"

namespace MatchPlayCurrentAttackApplyThroughBallTerminalResolution
{
	// Terminal application replays persisted provenance only; providers stay null.
	using FResult =
		FMatchPlayCurrentAttackApplyThroughBallTerminalResolutionResult;
	using EError =
		EMatchPlayCurrentAttackApplyThroughBallTerminalResolutionErrorCode;
	using ESource = EMatchPlayThroughBallTerminalSource;
	using EPhase = EMatchPlayCurrentAttackPostRouteRollPhase;

	void SetFailure(
		FResult& Result,
		const EError ErrorCode,
		const FString& ErrorMessage,
		const FName InvalidField = NAME_None)
	{
		Result.ErrorCode = ErrorCode;
		Result.ErrorMessage = ErrorMessage;
		Result.InvalidField = InvalidField;
	}

	bool SetTerminal(
		FResult& Result,
		const ESource Source,
		const bool bIsGoal)
	{
		if (Source == ESource::None)
		{
			SetFailure(
				Result,
				EError::InvalidTerminalSemantic,
				TEXT("ThroughBall terminal source must not be None."));
			return false;
		}
		Result.TerminalSource = Source;
		Result.bIsGoal = bIsGoal;
		return true;
	}

	bool RegenerateFeet(
		FResult& Result,
		const FSkillRuleSnapshotSet* SkillRuleSet)
	{
		++Result.FeetFormulaRegenerationCount;
		Result.FeetFormulaRegenerationResult =
			FMatchPlayCurrentAttackResolveThroughBallFeetFormulaOrchestrator
				::Resolve(Result.BeforeState, SkillRuleSet);
		Result.RegenerationProviderCallCount +=
			Result.FeetFormulaRegenerationResult
				.PlanRegenerationProviderCallCount;
		if (!Result.FeetFormulaRegenerationResult.bSuccess)
		{
			SetFailure(
				Result,
				EError::SourceRegenerationFailed,
				Result.FeetFormulaRegenerationResult.ErrorMessage,
				Result.FeetFormulaRegenerationResult.InvalidField);
			return false;
		}
		return SetTerminal(
			Result,
			Result.FeetFormulaRegenerationResult.FormulaResolutionResult.bIsGoal
				? ESource::FeetFormulaGoal
				: ESource::FeetFormulaMiss,
			Result.FeetFormulaRegenerationResult.FormulaResolutionResult.bIsGoal);
	}

	bool RegenerateAntiOffside(
		FResult& Result,
		const FSkillRuleSnapshotSet* SkillRuleSet,
		const EPhase Phase)
	{
		if (Phase == EPhase::OneOnOneDirectShot)
		{
			++Result.DirectShotRegenerationCount;
			Result.DirectShotRegenerationResult =
				FMatchPlayCurrentAttackResolveThroughBallOneOnOneDirectShotFormulaOrchestrator::Resolve(
					Result.BeforeState, SkillRuleSet);
			Result.RegenerationProviderCallCount +=
				Result.DirectShotRegenerationResult.PlanRegenerationProviderCallCount;
			if (!Result.DirectShotRegenerationResult.bSuccess)
			{
				SetFailure(Result, EError::SourceRegenerationFailed,
					Result.DirectShotRegenerationResult.ErrorMessage,
					Result.DirectShotRegenerationResult.InvalidField);
				return false;
			}
			const bool bGoal = Result.DirectShotRegenerationResult.Decision
				== EThroughBallOneOnOneDirectShotDecision::Goal;
			return SetTerminal(Result,
				bGoal ? ESource::AntiOffsideOneOnOneGoal : ESource::AntiOffsideOneOnOneMiss,
				bGoal);
		}
		if (Phase == EPhase::OneOnOneChipShot)
		{
			++Result.OneOnOneRegenerationCount;
			Result.OneOnOneRegenerationResult =
				FMatchPlayCurrentAttackResolveThroughBallOneOnOneChipShotDecisionOrchestrator
					::Resolve(Result.BeforeState, SkillRuleSet, nullptr);
			Result.RegenerationProviderCallCount +=
				Result.OneOnOneRegenerationResult.ProviderCallCount;
			if (!Result.OneOnOneRegenerationResult.bSuccess)
			{
				SetFailure(
					Result,
					EError::SourceRegenerationFailed,
					Result.OneOnOneRegenerationResult.ErrorMessage,
					Result.OneOnOneRegenerationResult.InvalidField);
				return false;
			}
			const bool bGoal =
				Result.OneOnOneRegenerationResult.QueryResult.bIsGoal;
			return SetTerminal(
				Result,
				bGoal
					? ESource::AntiOffsideOneOnOneGoal
					: ESource::AntiOffsideOneOnOneMiss,
				bGoal);
		}
		if (Phase != EPhase::PrimaryBranch)
		{
			SetFailure(
				Result,
				EError::IncompleteTerminalProvenance,
				TEXT("AntiOffside terminal provenance is incomplete."));
			return false;
		}

		FMatchPlayCurrentAttackResolveThroughBallAntiOffsideDecisionRequest
			Request;
		Request.AttackSequence = Result.BeforeState.CurrentAttack.AttackSequence;
		++Result.AntiOffsideRegenerationCount;
		Result.AntiOffsideRegenerationResult =
			FMatchPlayCurrentAttackResolveThroughBallAntiOffsideDecisionOrchestrator
				::Resolve(
					Result.BeforeState,
					Request,
					SkillRuleSet,
					nullptr);
		Result.RegenerationProviderCallCount +=
			Result.AntiOffsideRegenerationResult.ProviderCallCount;
		if (!Result.AntiOffsideRegenerationResult.bSuccess)
		{
			SetFailure(
				Result,
				EError::SourceRegenerationFailed,
				Result.AntiOffsideRegenerationResult.ErrorMessage);
			return false;
		}
		if (Result.AntiOffsideRegenerationResult.OutcomeResult.Decision
			== EThroughBallAntiOffsideOutcomeDecision::OneOnOneRequired)
		{
			SetFailure(
				Result,
				EError::SourceSemanticIsNonTerminal,
				TEXT("AntiOffside OneOnOne requires a completed ChipShot."));
			return false;
		}
		if (Result.AntiOffsideRegenerationResult.OutcomeResult.Decision
			!= EThroughBallAntiOffsideOutcomeDecision::Offside)
		{
			SetFailure(
				Result,
				EError::InvalidTerminalSemantic,
				TEXT("AntiOffside regeneration returned an unsupported semantic."));
			return false;
		}
		return SetTerminal(Result, ESource::AntiOffsideOffside, false);
	}

	bool RegenerateBehindDefense(
		FResult& Result,
		const FSkillRuleSnapshotSet* SkillRuleSet,
		const EPhase Phase)
	{
		if (Phase == EPhase::OneOnOneDirectShot)
		{
			++Result.DirectShotRegenerationCount;
			Result.DirectShotRegenerationResult =
				FMatchPlayCurrentAttackResolveThroughBallOneOnOneDirectShotFormulaOrchestrator::Resolve(
					Result.BeforeState, SkillRuleSet);
			Result.RegenerationProviderCallCount +=
				Result.DirectShotRegenerationResult.PlanRegenerationProviderCallCount;
			if (!Result.DirectShotRegenerationResult.bSuccess)
			{
				SetFailure(Result, EError::SourceRegenerationFailed,
					Result.DirectShotRegenerationResult.ErrorMessage,
					Result.DirectShotRegenerationResult.InvalidField);
				return false;
			}
			const bool bGoal = Result.DirectShotRegenerationResult.Decision
				== EThroughBallOneOnOneDirectShotDecision::Goal;
			return SetTerminal(Result,
				bGoal ? ESource::BehindDefenseOneOnOneGoal : ESource::BehindDefenseOneOnOneMiss,
				bGoal);
		}
		if (Phase == EPhase::OneOnOneChipShot)
		{
			++Result.OneOnOneRegenerationCount;
			Result.OneOnOneRegenerationResult =
				FMatchPlayCurrentAttackResolveThroughBallOneOnOneChipShotDecisionOrchestrator
					::Resolve(Result.BeforeState, SkillRuleSet, nullptr);
			Result.RegenerationProviderCallCount +=
				Result.OneOnOneRegenerationResult.ProviderCallCount;
			if (!Result.OneOnOneRegenerationResult.bSuccess)
			{
				SetFailure(
					Result,
					EError::SourceRegenerationFailed,
					Result.OneOnOneRegenerationResult.ErrorMessage,
					Result.OneOnOneRegenerationResult.InvalidField);
				return false;
			}
			const bool bGoal =
				Result.OneOnOneRegenerationResult.QueryResult.bIsGoal;
			return SetTerminal(
				Result,
				bGoal
					? ESource::BehindDefenseOneOnOneGoal
					: ESource::BehindDefenseOneOnOneMiss,
				bGoal);
		}
		if (Phase != EPhase::PrimaryBranch)
		{
			SetFailure(
				Result,
				EError::IncompleteTerminalProvenance,
				TEXT("BehindDefense terminal provenance is incomplete."));
			return false;
		}

		FMatchPlayCurrentAttackResolveThroughBallBehindDefenseP1DecisionOrPlanRequest
			Request;
		Request.AttackSequence = Result.BeforeState.CurrentAttack.AttackSequence;
		++Result.BehindDefenseP1PlanRegenerationCount;
		Result.BehindDefenseP1PlanRegenerationResult =
			FMatchPlayCurrentAttackResolveThroughBallBehindDefenseP1DecisionOrPlanOrchestrator
				::Resolve(
					Result.BeforeState,
					Request,
					SkillRuleSet,
					nullptr);
		Result.RegenerationProviderCallCount +=
			Result.BehindDefenseP1PlanRegenerationResult.ProviderCallCount;
		if (!Result.BehindDefenseP1PlanRegenerationResult.bSuccess)
		{
			SetFailure(
				Result,
				EError::SourceRegenerationFailed,
				Result.BehindDefenseP1PlanRegenerationResult.ErrorMessage);
			return false;
		}
		if (Result.BehindDefenseP1PlanRegenerationResult.P1PlanResult.Decision
			== EThroughBallBehindDefenseP1PlanQueryDecision::OutOfPlay)
		{
			return SetTerminal(
				Result,
				ESource::BehindDefenseOutOfPlay,
				false);
		}
		if (Result.BehindDefenseP1PlanRegenerationResult.P1PlanResult.Decision
			!= EThroughBallBehindDefenseP1PlanQueryDecision
				::FormulaResolutionRequired)
		{
			SetFailure(
				Result,
				EError::InvalidTerminalSemantic,
				TEXT("BehindDefense P1 returned an unsupported semantic."));
			return false;
		}

		++Result.BehindDefenseP1FormulaRegenerationCount;
		Result.BehindDefenseP1FormulaRegenerationResult =
			FMatchPlayCurrentAttackResolveThroughBallBehindDefenseP1FormulaOrchestrator
				::Resolve(Result.BeforeState, SkillRuleSet);
		Result.RegenerationProviderCallCount +=
			Result.BehindDefenseP1FormulaRegenerationResult
				.PlanRegenerationProviderCallCount;
		if (!Result.BehindDefenseP1FormulaRegenerationResult.bSuccess)
		{
			SetFailure(
				Result,
				EError::SourceRegenerationFailed,
				Result.BehindDefenseP1FormulaRegenerationResult.ErrorMessage,
				Result.BehindDefenseP1FormulaRegenerationResult.InvalidField);
			return false;
		}
		const auto P1Decision = Result.BehindDefenseP1FormulaRegenerationResult
			.FormulaExecutionResult.Decision;
		if (P1Decision
			== EThroughBallBehindDefenseP1FormulaResolutionExecutionDecision
				::OneOnOneRequired)
		{
			SetFailure(
				Result,
				EError::SourceSemanticIsNonTerminal,
				TEXT("BehindDefense P1 attacker win requires a completed OneOnOne choice."));
			return false;
		}
		if (P1Decision
			!= EThroughBallBehindDefenseP1FormulaResolutionExecutionDecision
				::DefenderStoppedAttack)
		{
			SetFailure(
				Result,
				EError::InvalidTerminalSemantic,
				TEXT("BehindDefense P1 Formula returned an unsupported semantic."));
			return false;
		}
		return SetTerminal(
			Result,
			ESource::BehindDefenseDefenderStoppedAttack,
			false);
	}
} // namespace MatchPlayCurrentAttackApplyThroughBallTerminalResolution

FMatchPlayCurrentAttackApplyThroughBallTerminalResolutionResult
FMatchPlayCurrentAttackApplyThroughBallTerminalResolutionOrchestrator::Resolve(
	const FMatchPlayState& BeforeState,
	const FSkillRuleSnapshotSet* SkillRuleSet)
{
	using namespace MatchPlayCurrentAttackApplyThroughBallTerminalResolution;

	FResult Result;
	Result.BeforeState = BeforeState;
	Result.AfterState = BeforeState;
	if (!BeforeState.RuntimeState.bIsInitialized)
	{
		SetFailure(
			Result,
			EError::MatchPlayStateNotInitialized,
			TEXT("ThroughBall terminal application requires initialized State."));
		return Result;
	}
	if (!BeforeState.bHasCurrentAttack)
	{
		SetFailure(
			Result,
			EError::NoCurrentAttack,
			TEXT("ThroughBall terminal application requires CurrentAttack."));
		return Result;
	}
	if (!BeforeState.CurrentAttack.bHasResolutionSession)
	{
		SetFailure(
			Result,
			EError::MissingResolutionSession,
			TEXT("ThroughBall terminal application requires a Resolution Session."));
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
			TEXT("ThroughBall terminal application requires RouteResolved."));
		return Result;
	}
	if (!Session.bHasActualBranch
		|| Session.ActualBranch.ActionType != ESkillRuleType::ThroughBall)
	{
		SetFailure(
			Result,
			EError::NotThroughBallResolution,
			TEXT("This operation supports only ThroughBall resolution."));
		return Result;
	}

	const EPhase Phase = Session.PostRouteRollProgress.Phase;
	bool bRegenerated = false;
	switch (Session.ActualBranch.ThroughBall)
	{
	case EMatchPlayThroughBallActualBranch::Feet:
		bRegenerated = RegenerateFeet(Result, SkillRuleSet);
		break;
	case EMatchPlayThroughBallActualBranch::AntiOffside:
		bRegenerated = RegenerateAntiOffside(Result, SkillRuleSet, Phase);
		break;
	case EMatchPlayThroughBallActualBranch::BehindDefense:
		bRegenerated = RegenerateBehindDefense(Result, SkillRuleSet, Phase);
		break;
	default:
		SetFailure(
			Result,
			EError::NotThroughBallResolution,
			TEXT("ThroughBall branch is unsupported for terminal application."));
		return Result;
	}
	if (!bRegenerated)
	{
		return Result;
	}
	if (Result.RegenerationProviderCallCount != 0)
	{
		SetFailure(
			Result,
			EError::TerminalRegenerationConsumedRng,
			TEXT("Terminal semantic regeneration must consume zero RNG."));
		return Result;
	}

	FMatchPlayThroughBallResolutionTerminalCapability Capability(
		FMatchPlayThroughBallResolutionTerminalCapability
			::FAuthoritativeTerminalIssuerTag(),
		BeforeState.CurrentAttack.AttackSequence,
		Result.TerminalSource,
		Result.bIsGoal);
	++Result.CompletionExecutionCount;
	Result.CompletionResult =
		FMatchPlayCurrentAttackCompletion::CompleteThroughBallResolution(
			BeforeState,
			Capability);
	if (!Result.CompletionResult.bSuccess)
	{
		SetFailure(
			Result,
			EError::CompletionFailed,
			Result.CompletionResult.ErrorMessage);
		return Result;
	}

	Result.AfterState = Result.CompletionResult.AfterState;
	Result.bSuccess = true;
	Result.ErrorCode = EError::None;
	Result.ErrorMessage.Empty();
	Result.InvalidField = NAME_None;
	return Result;
}
