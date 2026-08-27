#include "FMCodexLocalMatchResolutionFeedback.h"

#include "FMCodexLocalMatchHostGameMode.h"

namespace FMCodexLocalMatchResolutionFeedback
{
	FString PlayerLabel(const EInitialTurnOrderPlayer Player)
	{
		return FFMCodexLocalMatchInteractionViewBuilder::ToString(Player);
	}

	FString WinnerLabel(const EFormulaWinner Winner)
	{
		switch (Winner)
		{
		case EFormulaWinner::Attacker: return TEXT("Attacker");
		case EFormulaWinner::Defender: return TEXT("Defender");
		default: return TEXT("None");
		}
	}

	FString WinReasonLabel(const EFormulaWinReason Reason)
	{
		switch (Reason)
		{
		case EFormulaWinReason::HigherFinalValue:
			return TEXT("Higher final value");
		case EFormulaWinReason::StaminaTieBreaker:
			return TEXT("Stamina tie-breaker");
		case EFormulaWinReason::DefenderWinsEqualStamina:
			return TEXT("Defender wins equal stamina");
		case EFormulaWinReason::DefenderWinsGoalkeeperTie:
			return TEXT("Defender wins goalkeeper tie");
		case EFormulaWinReason::FastSuppression:
			return TEXT("Fast suppression");
		default:
			return TEXT("Canonical Formula result");
		}
	}

	FString ReadableCommand(const FString& CommandName)
	{
		if (CommandName == TEXT("BeginResolutionSession"))
		{
			return TEXT("Resolution Started");
		}
		if (CommandName == TEXT("ResolveIntentDeterminedRoute")
			|| CommandName == TEXT("ResolveInitialRoute"))
		{
			return TEXT("Route Resolved");
		}
		if (CommandName == TEXT("ResolveCrossPostRoutePlan"))
		{
			return TEXT("Cross Formula Plan");
		}
		if (CommandName == TEXT("ResolvePassControlPostRoutePlan"))
		{
			return TEXT("Pass Control Formula Plan");
		}
		if (CommandName == TEXT("ResolveThroughBallFeetPostRoutePlan"))
		{
			return TEXT("Through Ball - To Feet Plan");
		}
		if (CommandName == TEXT("ResolveThroughBallFeetAttackRoll"))
		{
			return TEXT("直塞脚下球 - 进攻方掷点");
		}
		if (CommandName == TEXT("ResolveThroughBallFeetDefenseRoll"))
		{
			return TEXT("直塞脚下球 - 防守方掷点");
		}
		if (CommandName == TEXT("ResolveThroughBallAntiOffsideDecision"))
		{
			return TEXT("Through Ball - Anti-Offside");
		}
		if (CommandName.Contains(TEXT("BehindDefenseP1")))
		{
			return TEXT("Through Ball - Behind Defense P1");
		}
		if (CommandName.Contains(TEXT("BehindDefenseP2")))
		{
			return TEXT("Through Ball - Behind Defense P2");
		}
		if (CommandName.Contains(TEXT("OneOnOneChipShot")))
		{
			return TEXT("One-on-One - Chip Shot");
		}
		if (CommandName.Contains(TEXT("OneOnOneDirectShot")))
		{
			return TEXT("One-on-One - Direct Shot");
		}
		if (CommandName == TEXT("ResolveDeadCornerPostRouteDecision"))
		{
			return TEXT("Dead Corner Decision");
		}
		if (CommandName == TEXT("ResolveDirectShotPostRouteDecisionOrPlan"))
		{
			return TEXT("Direct Shot Decision");
		}
		if (CommandName.StartsWith(TEXT("Apply")))
		{
			return TEXT("Attack Completed");
		}
		if (CommandName == TEXT("SubmitBranchIntent"))
		{
			return TEXT("Shot / Cross Type Selected");
		}
		if (CommandName == TEXT("SubmitThroughBallOneOnOneShotChoice"))
		{
			return TEXT("One-on-One Shot Selected");
		}
		return CommandName.IsEmpty()
			? TEXT("Local Match")
			: CommandName;
	}

	void AddViewEvidence(
		FFMCodexLocalMatchResolutionFeedback& Feedback,
		const FFMCodexLocalMatchInteractionView& BeforeView,
		const FFMCodexLocalMatchInteractionView& AfterView)
	{
		const FFMCodexLocalMatchInteractionView& EvidenceView =
			!AfterView.ActualBranchLabel.IsEmpty()
				? AfterView : BeforeView;
		const FString Action = !EvidenceView.ActionLabel.IsEmpty()
			? EvidenceView.ActionLabel : BeforeView.ActionLabel;
		const FString Branch = !EvidenceView.ActualBranchLabel.IsEmpty()
			? EvidenceView.ActualBranchLabel : BeforeView.ActualBranchLabel;
		if (!Action.IsEmpty() && !Branch.IsEmpty())
		{
			Feedback.RouteSummary = FString::Printf(
				TEXT("%s -> %s"), *Action, *Branch);
		}
		else if (!Action.IsEmpty())
		{
			Feedback.RouteSummary = Action;
		}

		Feedback.DiceEntries = !AfterView.AcceptedRolls.IsEmpty()
			? AfterView.AcceptedRolls : BeforeView.AcceptedRolls;
		Feedback.ResolutionFacts = AfterView.ResolutionFacts.bHasFacts
			? AfterView.ResolutionFacts : BeforeView.ResolutionFacts;
		if (AfterView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::SelectOneOnOneShot)
		{
			Feedback.ContinuationSummary =
				TEXT("Continue: One-on-One shot choice required");
		}
		else if (AfterView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::ContinueResolution)
		{
			Feedback.ContinuationSummary = AfterView.ContinueActionLabel;
		}
	}

	void AddFormulaEvidence(
		FFMCodexLocalMatchResolutionFeedback& Feedback,
		const FFormulaResolverInput& Input,
		const FFormulaResolutionResult& Resolution)
	{
		Feedback.ComparisonEntries.Add(FString::Printf(
			TEXT("Attacker: base %.1f | modifier %+.1f | compare %d | final %.1f"),
			Input.Attacker.BaseValue,
			Input.Attacker.Modifier,
			Input.Attacker.ComparePoint,
			Resolution.AttackerFinalValue));
		Feedback.ComparisonEntries.Add(FString::Printf(
			TEXT("Defender: base %.1f | modifier %+.1f | compare %d | final %.1f"),
			Input.Defender.BaseValue,
			Input.Defender.Modifier,
			Input.Defender.ComparePoint,
			Resolution.DefenderFinalValue));
		if (Input.bGoalkeeperParticipated)
		{
			Feedback.ComparisonEntries.Add(
				TEXT("Goalkeeper participated in the authoritative Formula"));
		}
		Feedback.DecisionSummary = FString::Printf(
			TEXT("Winner: %s | Reason: %s"),
			*WinnerLabel(Resolution.Winner),
			*WinReasonLabel(Resolution.WinReason));
		Feedback.StepSummary = TEXT("Formula resolved");
	}

	void AddSingleCardFormulaEvidence(
		FFMCodexLocalMatchResolutionFeedback& Feedback,
		const FMatchPlayCurrentAttackResolveSingleCardFinishingFormulaResult& Formula)
	{
		if (Formula.bHasFormulaResolution
			&& Formula.ResolverInputAssemblyResult.bSuccess)
		{
			AddFormulaEvidence(
				Feedback,
				Formula.ResolverInputAssemblyResult.ResolverInput,
				Formula.FormulaResolutionResult);
		}
	}

	void AddFeetFormulaEvidence(
		FFMCodexLocalMatchResolutionFeedback& Feedback,
		const FMatchPlayCurrentAttackResolveThroughBallFeetFormulaResult& Formula)
	{
		if (Formula.bHasFormulaResolution
			&& Formula.ResolverInputAssemblyResult.bHasResolverInput)
		{
			AddFormulaEvidence(
				Feedback,
				Formula.ResolverInputAssemblyResult.ResolverInput,
				Formula.FormulaResolutionResult);
		}
	}

	void AddBehindDefenseP1FormulaEvidence(
		FFMCodexLocalMatchResolutionFeedback& Feedback,
		const FMatchPlayCurrentAttackResolveThroughBallBehindDefenseP1FormulaResult& Formula)
	{
		if (Formula.bHasFormulaResolution
			&& Formula.ResolverInputAssemblyResult.bHasResolverInput)
		{
			AddFormulaEvidence(
				Feedback,
				Formula.ResolverInputAssemblyResult.ResolverInput,
				Formula.FormulaResolutionResult);
		}
		if (Formula.FormulaExecutionResult.Decision
			== EThroughBallBehindDefenseP1FormulaResolutionExecutionDecision
				::OneOnOneRequired)
		{
			Feedback.DecisionSummary = TEXT("One-on-One choice required");
		}
		else if (Formula.FormulaExecutionResult.Decision
			== EThroughBallBehindDefenseP1FormulaResolutionExecutionDecision
				::DefenderStoppedAttack)
		{
			Feedback.DecisionSummary =
				TEXT("Defender stopped attack");
		}
	}

	void AddOneOnOneDirectShotEvidence(
		FFMCodexLocalMatchResolutionFeedback& Feedback,
		const FThroughBallOneOnOneDirectShotFormulaResult& Formula)
	{
		const FThroughBallOneOnOneDirectShotFormulaPlan& Plan = Formula.Plan;
		if (Formula.bHasResolverInput && Formula.bHasFormulaResolution)
		{
			Feedback.ComparisonEntries.Add(FString::Printf(
				TEXT("Shooter: Shooting %d | modifier %+.1f | D6 %d | final %.1f"),
				Plan.ShooterShooting,
				Formula.ResolverInput.Attacker.Modifier,
				Plan.AttackD6,
				Formula.FormulaResolutionResult.AttackerFinalValue));
			Feedback.ComparisonEntries.Add(FString::Printf(
				TEXT("Goalkeeper: OneOnOne %d | activation %s | authoritative modifier %+.1f | D6 %d | final %.1f"),
				Plan.GoalkeeperOneOnOne,
				Plan.bGoalkeeperActivated ? TEXT("active") : TEXT("inactive"),
				Formula.ResolverInput.Defender.Modifier,
				Plan.DefenseD6,
				Formula.FormulaResolutionResult.DefenderFinalValue));
			Feedback.DecisionSummary = FString::Printf(
				TEXT("Winner: %s | Reason: %s"),
				*WinnerLabel(Formula.FormulaResolutionResult.Winner),
				*WinReasonLabel(Formula.FormulaResolutionResult.WinReason));
		}
		Feedback.StepSummary = TEXT("One-on-One Direct Shot Formula resolved");
	}

	FString ThroughBallTerminalLabel(
		const EMatchPlayThroughBallTerminalSource Source)
	{
		switch (Source)
		{
		case EMatchPlayThroughBallTerminalSource::FeetFormulaGoal:
		case EMatchPlayThroughBallTerminalSource::AntiOffsideOneOnOneGoal:
		case EMatchPlayThroughBallTerminalSource::BehindDefenseOneOnOneGoal:
			return TEXT("GOAL");
		case EMatchPlayThroughBallTerminalSource::AntiOffsideOffside:
		case EMatchPlayThroughBallTerminalSource::BehindDefenseP2Offside:
			return TEXT("OFFSIDE");
		case EMatchPlayThroughBallTerminalSource::BehindDefenseOutOfPlay:
			return TEXT("OUT OF PLAY");
		case EMatchPlayThroughBallTerminalSource::BehindDefenseDefenderStoppedAttack:
			return TEXT("DEFENDER STOPPED ATTACK");
		case EMatchPlayThroughBallTerminalSource::FeetFormulaMiss:
		case EMatchPlayThroughBallTerminalSource::AntiOffsideOneOnOneMiss:
		case EMatchPlayThroughBallTerminalSource::BehindDefenseOneOnOneMiss:
			return TEXT("MISS");
		default:
			return TEXT("NO GOAL");
		}
	}

	FString ShotTerminalLabel(const EMatchPlayShotTerminalSource Source)
	{
		switch (Source)
		{
		case EMatchPlayShotTerminalSource::LongShotDirectShotFormulaGoal:
		case EMatchPlayShotTerminalSource::CutInsideShotDirectShotFormulaGoal:
		case EMatchPlayShotTerminalSource::LongShotDeadCornerGoal:
		case EMatchPlayShotTerminalSource::CutInsideShotDeadCornerGoal:
			return TEXT("GOAL");
		case EMatchPlayShotTerminalSource::LongShotDirectShotImmediateMiss:
		case EMatchPlayShotTerminalSource::CutInsideShotDirectShotImmediateMiss:
			return TEXT("IMMEDIATE MISS");
		case EMatchPlayShotTerminalSource::LongShotDirectShotFormulaMiss:
		case EMatchPlayShotTerminalSource::CutInsideShotDirectShotFormulaMiss:
		case EMatchPlayShotTerminalSource::LongShotDeadCornerMiss:
		case EMatchPlayShotTerminalSource::CutInsideShotDeadCornerMiss:
			return TEXT("MISS");
		default:
			return TEXT("NO GOAL");
		}
	}

	void AddCompletionEvidence(
		FFMCodexLocalMatchResolutionFeedback& Feedback,
		const FMatchPlayCurrentAttackCompletionResult& Completion)
	{
		if (!Completion.bSuccess)
		{
			return;
		}
		const FMatchRuntimeState& Runtime = Completion.AfterState.RuntimeState;
		if (Completion.AfterState.bHasCurrentAttack
			&& Completion.AfterState.CurrentAttack.LifecycleState
				== EMatchPlayCurrentAttackLifecycleState
					::TerminalPendingAdvance)
		{
			Feedback.ContinuationSummary = FString::Printf(
				TEXT("Result persisted | Score: Player A %d - %d Player B | Current attacker unchanged: %s | Opportunity pending explicit 下一回合"),
				Runtime.PlayerAState.Score,
				Runtime.PlayerBState.Score,
				*PlayerLabel(Runtime.CurrentAttackingPlayer));
			return;
		}
		Feedback.ContinuationSummary = FString::Printf(
			TEXT("Attack complete | Score: Player A %d - %d Player B | Next attacker: %s | Opportunity consumed: %s"),
			Runtime.PlayerAState.Score,
			Runtime.PlayerBState.Score,
			*PlayerLabel(Completion.NextAttackingPlayer),
			Completion.OpportunityResolveResult.bSuccess
				? TEXT("yes") : TEXT("no"));
		if (Completion.bMatchEnded)
		{
			Feedback.ContinuationSummary += TEXT(" | Match ended");
		}
	}

	void FinalizeTerminal(
		FFMCodexLocalMatchResolutionFeedback& Feedback,
		const FString& Semantic,
		const FMatchPlayCurrentAttackCompletionResult& Completion)
	{
		Feedback.bTerminal = true;
		Feedback.StepTitle = TEXT("Attack Completed");
		Feedback.TerminalSummary = TEXT("RESULT: ") + Semantic;
		Feedback.DecisionSummary = Feedback.DecisionSummary.IsEmpty()
			? Feedback.TerminalSummary : Feedback.DecisionSummary;
		AddCompletionEvidence(Feedback, Completion);
	}
}

FFMCodexLocalMatchResolutionFeedback
FFMCodexLocalMatchResolutionFeedbackBuilder::BuildGenericAccepted(
	const FString& CommandName,
	const FFMCodexLocalMatchInteractionView& BeforeView,
	const FFMCodexLocalMatchInteractionView& AfterView)
{
	using namespace FMCodexLocalMatchResolutionFeedback;
	FFMCodexLocalMatchResolutionFeedback Feedback;
	Feedback.bVisible =
		BeforeView.MajorPhase == EFMCodexLocalMatchMajorPhase::Resolution
		|| AfterView.MajorPhase == EFMCodexLocalMatchMajorPhase::Resolution;
	Feedback.CommandName = CommandName;
	Feedback.StepTitle = ReadableCommand(CommandName);
	Feedback.StepSummary = TEXT("Accepted by the authoritative local Host");
	AddViewEvidence(Feedback, BeforeView, AfterView);
	return Feedback;
}

FFMCodexLocalMatchResolutionFeedback
FFMCodexLocalMatchResolutionFeedbackBuilder::BuildRejected(
	const FString& CommandName,
	const FString& ErrorMessage,
	const bool bAuthoritativeAccepted)
{
	FFMCodexLocalMatchResolutionFeedback Feedback;
	Feedback.bVisible = true;
	Feedback.bRejected = true;
	Feedback.CommandName = CommandName;
	Feedback.StepTitle = TEXT("Command Rejected");
	Feedback.StepSummary = bAuthoritativeAccepted
		? TEXT("The authoritative domain rejected this command")
		: TEXT("The command was blocked before authoritative mutation");
	Feedback.DecisionSummary = TEXT("No resolution result was accepted");
	Feedback.ErrorMessage = ErrorMessage;
	return Feedback;
}

FFMCodexLocalMatchResolutionFeedback
FFMCodexLocalMatchResolutionFeedbackBuilder::BuildFromTerminalSnapshot(
	const FFMCodexLocalMatchInteractionView& TerminalView)
{
	FFMCodexLocalMatchResolutionFeedback Feedback;
	Feedback.bVisible = true;
	Feedback.bTerminal = true;
	Feedback.CommandName = TEXT("TerminalSnapshot");
	Feedback.StepTitle = TEXT("Attack Completed");
	Feedback.StepSummary = TEXT("Authoritative terminal result restored");
	Feedback.ResolutionFacts = TerminalView.ResolutionFacts;

	FString Semantic = TEXT("NO GOAL");
	for (const FMatchPlayResolutionDecisionFact& Decision
		: TerminalView.ResolutionFacts.Decisions)
	{
		if (!Decision.bResolved)
		{
			continue;
		}
		switch (Decision.Outcome)
		{
		case EMatchPlayResolutionDecisionOutcome::Goal:
			Semantic = TEXT("GOAL");
			break;
		case EMatchPlayResolutionDecisionOutcome::Miss:
			Semantic = TEXT("MISS");
			break;
		case EMatchPlayResolutionDecisionOutcome::ImmediateMiss:
			Semantic = TEXT("IMMEDIATE MISS");
			break;
		case EMatchPlayResolutionDecisionOutcome::Offside:
			Semantic = TEXT("OFFSIDE");
			break;
		case EMatchPlayResolutionDecisionOutcome::OutOfPlay:
			Semantic = TEXT("OUT OF PLAY");
			break;
		case EMatchPlayResolutionDecisionOutcome::DefenderStoppedAttack:
			Semantic = TEXT("DEFENDER STOPPED ATTACK");
			break;
		default:
			continue;
		}
	}
	Feedback.TerminalSummary = TEXT("RESULT: ") + Semantic;
	Feedback.DecisionSummary = Feedback.TerminalSummary;
	Feedback.ContinuationSummary =
		TEXT("Opportunity pending explicit 下一回合");
	for (const FMatchPlayResolutionFormulaContestFact& Contest
		: TerminalView.ResolutionFacts.FormulaContests)
	{
		if (!Contest.bHasResolvedFormula)
		{
			continue;
		}
		Feedback.ComparisonEntries.Add(FString::Printf(
			TEXT("Attack final %.1f | Defense final %.1f"),
			Contest.ResolvedResult.AttackerFinalValue,
			Contest.ResolvedResult.DefenderFinalValue));
	}
	return Feedback;
}

#define FMCODEX_ROUTE_BUILD(ResultType) \
	FFMCodexLocalMatchResolutionFeedback \
	FFMCodexLocalMatchResolutionFeedbackBuilder::Build( \
		const FString& CommandName, const ResultType& Result, \
		const FFMCodexLocalMatchInteractionView& BeforeView, \
		const FFMCodexLocalMatchInteractionView& AfterView) \
	{ \
		auto Feedback = BuildGenericAccepted(CommandName, BeforeView, AfterView); \
		Feedback.StepSummary = Feedback.RouteSummary.IsEmpty() \
			? TEXT("Authoritative route resolved") \
			: Feedback.RouteSummary; \
		return Feedback; \
	}

FMCODEX_ROUTE_BUILD(FFMCodexLocalMatchResolveIntentDeterminedRouteResult)
FMCODEX_ROUTE_BUILD(FFMCodexLocalMatchResolveInitialRouteResult)

#undef FMCODEX_ROUTE_BUILD

#define FMCODEX_PLAN_BUILD(ResultType, SummaryText) \
	FFMCodexLocalMatchResolutionFeedback \
	FFMCodexLocalMatchResolutionFeedbackBuilder::Build( \
		const FString& CommandName, const ResultType& Result, \
		const FFMCodexLocalMatchInteractionView& BeforeView, \
		const FFMCodexLocalMatchInteractionView& AfterView) \
	{ \
		auto Feedback = BuildGenericAccepted(CommandName, BeforeView, AfterView); \
		Feedback.StepSummary = SummaryText; \
		Feedback.DecisionSummary = TEXT("Formula resolution required"); \
		return Feedback; \
	}

FMCODEX_PLAN_BUILD(
	FFMCodexLocalMatchResolveCrossPostRoutePlanResult,
	TEXT("Cross post-route plan accepted"))
FMCODEX_PLAN_BUILD(
	FFMCodexLocalMatchResolveThroughBallFeetPostRoutePlanResult,
	TEXT("Through Ball to Feet plan accepted"))
FMCODEX_PLAN_BUILD(
	FFMCodexLocalMatchResolvePassControlPostRoutePlanResult,
	TEXT("Pass Control post-route plan accepted"))

#undef FMCODEX_PLAN_BUILD

FFMCodexLocalMatchResolutionFeedback
FFMCodexLocalMatchResolutionFeedbackBuilder::Build(
	const FString& CommandName,
	const FFMCodexLocalMatchResolveThroughBallFeetAttackRollResult& Result,
	const FFMCodexLocalMatchInteractionView& BeforeView,
	const FFMCodexLocalMatchInteractionView& AfterView)
{
	auto Feedback = BuildGenericAccepted(CommandName, BeforeView, AfterView);
	Feedback.StepSummary = TEXT("进攻方权威点数已记录");
	Feedback.ContinuationSummary = TEXT("等待防守方掷点");
	return Feedback;
}

FFMCodexLocalMatchResolutionFeedback
FFMCodexLocalMatchResolutionFeedbackBuilder::Build(
	const FString& CommandName,
	const FFMCodexLocalMatchResolveThroughBallFeetDefenseRollResult& Result,
	const FFMCodexLocalMatchInteractionView& BeforeView,
	const FFMCodexLocalMatchInteractionView& AfterView)
{
	auto Feedback = BuildGenericAccepted(CommandName, BeforeView, AfterView);
	Feedback.StepSummary = TEXT("防守方权威点数已记录");
	Feedback.DecisionSummary = TEXT("属性对抗已完成");
	Feedback.ContinuationSummary = TEXT("等待权威终结应用");
	return Feedback;
}

FFMCodexLocalMatchResolutionFeedback
FFMCodexLocalMatchResolutionFeedbackBuilder::Build(
	const FString& CommandName,
	const FFMCodexLocalMatchResolveThroughBallBehindDefenseP1AttackRollResult& Result,
	const FFMCodexLocalMatchInteractionView& BeforeView,
	const FFMCodexLocalMatchInteractionView& AfterView)
{
	auto Feedback = BuildGenericAccepted(CommandName, BeforeView, AfterView);
	Feedback.StepSummary = TEXT("进攻方权威点数已记录");
	if (AfterView.InteractionCategory
		== EFMCodexLocalMatchInteractionCategory
			::RollThroughBallBehindDefenseDefense)
	{
		Feedback.ContinuationSummary = TEXT("等待防守方掷点");
	}
	return Feedback;
}

FFMCodexLocalMatchResolutionFeedback
FFMCodexLocalMatchResolutionFeedbackBuilder::Build(
	const FString& CommandName,
	const FFMCodexLocalMatchResolveThroughBallBehindDefenseP1DefenseRollResult& Result,
	const FFMCodexLocalMatchInteractionView& BeforeView,
	const FFMCodexLocalMatchInteractionView& AfterView)
{
	auto Feedback = BuildGenericAccepted(CommandName, BeforeView, AfterView);
	Feedback.StepSummary = TEXT("防守方权威点数已记录");
	Feedback.DecisionSummary = AfterView.InteractionCategory
		== EFMCodexLocalMatchInteractionCategory::SelectOneOnOneShot
			? TEXT("One-on-One choice required")
			: TEXT("Defender stopped attack");
	return Feedback;
}

FFMCodexLocalMatchResolutionFeedback
FFMCodexLocalMatchResolutionFeedbackBuilder::Build(
	const FString& CommandName,
	const FFMCodexLocalMatchResolveDeadCornerPostRouteDecisionResult& Result,
	const FFMCodexLocalMatchInteractionView& BeforeView,
	const FFMCodexLocalMatchInteractionView& AfterView)
{
	using namespace FMCodexLocalMatchResolutionFeedback;
	auto Feedback = BuildGenericAccepted(CommandName, BeforeView, AfterView);
	const auto& Decision =
		Result.AuthoritativeResult.OrchestrationResult;
	const bool bGoal = Decision.ActionType == ESkillRuleType::LongShot
		? Decision.LongShotResult.Decision == ELongShotDeadCornerDecision::Goal
		: Decision.CutInsideShotResult.Decision
			== ECutInsideShotDeadCornerDecision::Goal;
	Feedback.StepSummary = TEXT("Dead Corner decision resolved");
	Feedback.DecisionSummary = bGoal
		? TEXT("Goal") : TEXT("Miss");
	return Feedback;
}

FFMCodexLocalMatchResolutionFeedback
FFMCodexLocalMatchResolutionFeedbackBuilder::Build(
	const FString& CommandName,
	const FFMCodexLocalMatchResolveThroughBallAntiOffsideDecisionResult& Result,
	const FFMCodexLocalMatchInteractionView& BeforeView,
	const FFMCodexLocalMatchInteractionView& AfterView)
{
	auto Feedback = BuildGenericAccepted(CommandName, BeforeView, AfterView);
	const EThroughBallAntiOffsideOutcomeDecision Decision = Result
		.AuthoritativeResult.OrchestrationResult.OutcomeResult.Decision;
	Feedback.StepSummary = TEXT("Anti-Offside decision resolved");
	Feedback.DecisionSummary = Decision
		== EThroughBallAntiOffsideOutcomeDecision::Offside
			? TEXT("Offside")
			: TEXT("One-on-One required");
	return Feedback;
}

FFMCodexLocalMatchResolutionFeedback
FFMCodexLocalMatchResolutionFeedbackBuilder::Build(
	const FString& CommandName,
	const FFMCodexLocalMatchResolveDirectShotPostRouteDecisionOrPlanResult& Result,
	const FFMCodexLocalMatchInteractionView& BeforeView,
	const FFMCodexLocalMatchInteractionView& AfterView)
{
	auto Feedback = BuildGenericAccepted(CommandName, BeforeView, AfterView);
	const auto& Decision = Result.AuthoritativeResult.OrchestrationResult;
	const bool bImmediateMiss = Decision.ActionType == ESkillRuleType::LongShot
		? Decision.LongShotResult.Decision
			== ELongShotDirectShotDecision::ImmediateMiss
		: Decision.CutInsideShotResult.Decision
			== ECutInsideShotDirectShotDecision::ImmediateMiss;
	Feedback.StepSummary = TEXT("Direct Shot decision resolved");
	Feedback.DecisionSummary = bImmediateMiss
		? TEXT("Immediate Miss")
		: TEXT("Formula resolution required");
	return Feedback;
}

FFMCodexLocalMatchResolutionFeedback
FFMCodexLocalMatchResolutionFeedbackBuilder::Build(
	const FString& CommandName,
	const FFMCodexLocalMatchResolveThroughBallBehindDefenseP1DecisionOrPlanResult& Result,
	const FFMCodexLocalMatchInteractionView& BeforeView,
	const FFMCodexLocalMatchInteractionView& AfterView)
{
	auto Feedback = BuildGenericAccepted(CommandName, BeforeView, AfterView);
	const EThroughBallBehindDefenseP1PlanQueryDecision Decision = Result
		.AuthoritativeResult.OrchestrationResult.P1PlanResult.Decision;
	Feedback.StepSummary = TEXT("Behind Defense P1 contest resolved");
	if (Decision == EThroughBallBehindDefenseP1PlanQueryDecision::OutOfPlay)
	{
		Feedback.DecisionSummary = TEXT("Out of Play");
	}
	else if (AfterView.InteractionCategory
		== EFMCodexLocalMatchInteractionCategory::SelectOneOnOneShot)
	{
		// The authoritative interaction projection has already replayed the
		// completed P1 contest from accepted rolls. Present that transition;
		// do not imply that a removed P2 or another Formula command remains.
		Feedback.DecisionSummary = TEXT("One-on-One choice required");
	}
	else
	{
		Feedback.DecisionSummary = TEXT("Defender stopped attack");
	}
	return Feedback;
}

FFMCodexLocalMatchResolutionFeedback
FFMCodexLocalMatchResolutionFeedbackBuilder::Build(
	const FString& CommandName,
	const FFMCodexLocalMatchResolveThroughBallBehindDefenseP1FormulaResult& Result,
	const FFMCodexLocalMatchInteractionView& BeforeView,
	const FFMCodexLocalMatchInteractionView& AfterView)
{
	using namespace FMCodexLocalMatchResolutionFeedback;
	auto Feedback = BuildGenericAccepted(CommandName, BeforeView, AfterView);
	const auto& Formula = Result.AuthoritativeResult.OrchestrationResult;
	Feedback.StepTitle = TEXT("Through Ball - Behind Defense P1");
	Feedback.StepSummary = TEXT("Behind Defense P1 contest resolved");
	AddBehindDefenseP1FormulaEvidence(Feedback, Formula);
	if (AfterView.InteractionCategory
		== EFMCodexLocalMatchInteractionCategory::SelectOneOnOneShot)
	{
		Feedback.DecisionSummary = TEXT("One-on-One choice required");
	}
	return Feedback;
}

FFMCodexLocalMatchResolutionFeedback
FFMCodexLocalMatchResolutionFeedbackBuilder::Build(
	const FString& CommandName,
	const FFMCodexLocalMatchResolveThroughBallBehindDefenseP2DecisionResult& Result,
	const FFMCodexLocalMatchInteractionView& BeforeView,
	const FFMCodexLocalMatchInteractionView& AfterView)
{
	using namespace FMCodexLocalMatchResolutionFeedback;
	auto Feedback = BuildGenericAccepted(CommandName, BeforeView, AfterView);
	const auto& Decision = Result.AuthoritativeResult.OrchestrationResult;
	AddBehindDefenseP1FormulaEvidence(
		Feedback, Decision.P1FormulaRegenerationResult);
	Feedback.StepTitle = TEXT("Through Ball - Behind Defense P2");
	Feedback.StepSummary = TEXT("Behind Defense P2 decision resolved");
	Feedback.DecisionSummary = Decision.QueryResult.Decision
		== EThroughBallBehindDefenseP2OutcomeDecision::Offside
			? TEXT("Offside")
			: TEXT("One-on-One required");
	return Feedback;
}

FFMCodexLocalMatchResolutionFeedback
FFMCodexLocalMatchResolutionFeedbackBuilder::Build(
	const FString& CommandName,
	const FFMCodexLocalMatchResolveThroughBallOneOnOneChipShotDecisionResult& Result,
	const FFMCodexLocalMatchInteractionView& BeforeView,
	const FFMCodexLocalMatchInteractionView& AfterView)
{
	auto Feedback = BuildGenericAccepted(CommandName, BeforeView, AfterView);
	const auto Decision = Result.AuthoritativeResult.OrchestrationResult
		.QueryResult.Decision;
	Feedback.StepTitle = TEXT("One-on-One - Chip Shot");
	Feedback.StepSummary = TEXT("Chip Shot decision resolved without Formula");
	Feedback.DecisionSummary = Decision
		== EThroughBallOneOnOneChipShotOutcomeDecision::Goal
			? TEXT("Goal") : TEXT("Miss");
	return Feedback;
}

FFMCodexLocalMatchResolutionFeedback
FFMCodexLocalMatchResolutionFeedbackBuilder::Build(
	const FString& CommandName,
	const FFMCodexLocalMatchResolveThroughBallOneOnOneDirectShotPostRoutePlanResult& Result,
	const FFMCodexLocalMatchInteractionView& BeforeView,
	const FFMCodexLocalMatchInteractionView& AfterView)
{
	auto Feedback = BuildGenericAccepted(CommandName, BeforeView, AfterView);
	const FThroughBallOneOnOneDirectShotFormulaPlan& Plan = Result
		.AuthoritativeResult.OrchestrationResult.FormulaPlan;
	Feedback.StepTitle = TEXT("One-on-One - Direct Shot");
	Feedback.StepSummary = TEXT("Direct Shot Formula plan accepted");
	Feedback.ComparisonEntries.Add(FString::Printf(
		TEXT("Shooter: Shooting %d | D6 %d"),
		Plan.ShooterShooting, Plan.AttackD6));
	Feedback.ComparisonEntries.Add(FString::Printf(
		TEXT("Goalkeeper: OneOnOne %d | activation %s | D6 %d"),
		Plan.GoalkeeperOneOnOne,
		Plan.bGoalkeeperActivated ? TEXT("active") : TEXT("inactive"),
		Plan.DefenseD6));
	Feedback.DecisionSummary = TEXT("Formula resolution required");
	return Feedback;
}

FFMCodexLocalMatchResolutionFeedback
FFMCodexLocalMatchResolutionFeedbackBuilder::Build(
	const FString& CommandName,
	const FFMCodexLocalMatchResolveThroughBallOneOnOneDirectShotFormulaResult& Result,
	const FFMCodexLocalMatchInteractionView& BeforeView,
	const FFMCodexLocalMatchInteractionView& AfterView)
{
	using namespace FMCodexLocalMatchResolutionFeedback;
	auto Feedback = BuildGenericAccepted(CommandName, BeforeView, AfterView);
	Feedback.StepTitle = TEXT("One-on-One - Direct Shot");
	AddOneOnOneDirectShotEvidence(
		Feedback,
		Result.AuthoritativeResult.OrchestrationResult.DirectShotFormulaResult);
	return Feedback;
}

FFMCodexLocalMatchResolutionFeedback
FFMCodexLocalMatchResolutionFeedbackBuilder::Build(
	const FString& CommandName,
	const FFMCodexLocalMatchApplyThroughBallTerminalResolutionResult& Result,
	const FFMCodexLocalMatchInteractionView& BeforeView,
	const FFMCodexLocalMatchInteractionView& AfterView)
{
	using namespace FMCodexLocalMatchResolutionFeedback;
	auto Feedback = BuildGenericAccepted(CommandName, BeforeView, AfterView);
	const auto& Terminal = Result.AuthoritativeResult.OrchestrationResult;
	switch (Terminal.TerminalSource)
	{
	case EMatchPlayThroughBallTerminalSource::FeetFormulaGoal:
	case EMatchPlayThroughBallTerminalSource::FeetFormulaMiss:
		AddFeetFormulaEvidence(Feedback, Terminal.FeetFormulaRegenerationResult);
		break;
	case EMatchPlayThroughBallTerminalSource::BehindDefenseDefenderStoppedAttack:
		AddBehindDefenseP1FormulaEvidence(
			Feedback, Terminal.BehindDefenseP1FormulaRegenerationResult);
		break;
	case EMatchPlayThroughBallTerminalSource::AntiOffsideOneOnOneGoal:
	case EMatchPlayThroughBallTerminalSource::AntiOffsideOneOnOneMiss:
	case EMatchPlayThroughBallTerminalSource::BehindDefenseOneOnOneGoal:
	case EMatchPlayThroughBallTerminalSource::BehindDefenseOneOnOneMiss:
		if (Terminal.DirectShotRegenerationCount > 0)
		{
			AddOneOnOneDirectShotEvidence(
				Feedback,
				Terminal.DirectShotRegenerationResult.DirectShotFormulaResult);
		}
		else
		{
			Feedback.StepSummary =
				TEXT("One-on-One Chip Shot decision applied without Formula");
		}
		break;
	default:
		break;
	}
	FinalizeTerminal(
		Feedback,
		ThroughBallTerminalLabel(Terminal.TerminalSource),
		Terminal.CompletionResult);
	return Feedback;
}

FFMCodexLocalMatchResolutionFeedback
FFMCodexLocalMatchResolutionFeedbackBuilder::Build(
	const FString& CommandName,
	const FFMCodexLocalMatchApplyCrossTerminalResolutionResult& Result,
	const FFMCodexLocalMatchInteractionView& BeforeView,
	const FFMCodexLocalMatchInteractionView& AfterView)
{
	using namespace FMCodexLocalMatchResolutionFeedback;
	auto Feedback = BuildGenericAccepted(CommandName, BeforeView, AfterView);
	const auto& Terminal = Result.AuthoritativeResult.OrchestrationResult;
	AddSingleCardFormulaEvidence(Feedback, Terminal.FormulaRegenerationResult);
	FinalizeTerminal(
		Feedback,
		Terminal.bIsGoal ? TEXT("GOAL") : TEXT("MISS"),
		Terminal.CompletionResult);
	return Feedback;
}

FFMCodexLocalMatchResolutionFeedback
FFMCodexLocalMatchResolutionFeedbackBuilder::Build(
	const FString& CommandName,
	const FFMCodexLocalMatchApplyPassControlTerminalResolutionResult& Result,
	const FFMCodexLocalMatchInteractionView& BeforeView,
	const FFMCodexLocalMatchInteractionView& AfterView)
{
	using namespace FMCodexLocalMatchResolutionFeedback;
	auto Feedback = BuildGenericAccepted(CommandName, BeforeView, AfterView);
	const auto& Terminal = Result.AuthoritativeResult.OrchestrationResult;
	AddSingleCardFormulaEvidence(Feedback, Terminal.FormulaRegenerationResult);
	FinalizeTerminal(
		Feedback,
		Terminal.bIsGoal ? TEXT("GOAL") : TEXT("MISS"),
		Terminal.CompletionResult);
	return Feedback;
}

FFMCodexLocalMatchResolutionFeedback
FFMCodexLocalMatchResolutionFeedbackBuilder::Build(
	const FString& CommandName,
	const FFMCodexLocalMatchApplyShotTerminalResolutionResult& Result,
	const FFMCodexLocalMatchInteractionView& BeforeView,
	const FFMCodexLocalMatchInteractionView& AfterView)
{
	using namespace FMCodexLocalMatchResolutionFeedback;
	auto Feedback = BuildGenericAccepted(CommandName, BeforeView, AfterView);
	const auto& Terminal = Result.AuthoritativeResult.OrchestrationResult;
	if (Terminal.bUsedFormula)
	{
		AddSingleCardFormulaEvidence(Feedback, Terminal.FormulaRegenerationResult);
	}
	FinalizeTerminal(
		Feedback,
		ShotTerminalLabel(Terminal.TerminalSource),
		Terminal.CompletionResult);
	return Feedback;
}
