#include "MatchPlayCurrentAttackApplyShotTerminalResolutionOrchestrator.h"

#include "MatchPlayCurrentAttackResolveDeadCornerPostRouteDecisionOrchestrator.h"
#include "MatchPlayCurrentAttackResolveDirectShotPostRouteDecisionOrPlanOrchestrator.h"
#include "MatchPlayCurrentAttackResolveSingleCardFinishingFormulaOrchestrator.h"

namespace MatchPlayCurrentAttackApplyShotTerminalResolution
{
	using FResult = FMatchPlayCurrentAttackApplyShotTerminalResolutionResult;
	using EError =
		EMatchPlayCurrentAttackApplyShotTerminalResolutionErrorCode;
	using ESource = EMatchPlayShotTerminalSource;
	using EFamily = EMatchPlaySingleCardFinishingFormulaFamily;

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

	bool AreStatesEqual(
		const FMatchPlayState& Left,
		const FMatchPlayState& Right)
	{
		return FMatchPlayState::StaticStruct()->CompareScriptStruct(
			&Left,
			&Right,
			0);
	}

	bool IsShotAction(const ESkillRuleType ActionType)
	{
		return ActionType == ESkillRuleType::LongShot
			|| ActionType == ESkillRuleType::CutInsideShot;
	}

	bool IsDirectShotBranch(
		const FMatchPlayCurrentAttackActualBranch& Branch)
	{
		return (Branch.ActionType == ESkillRuleType::LongShot
				&& Branch.LongShot
					== EMatchPlayLongShotActualBranch::DirectShot)
			|| (Branch.ActionType == ESkillRuleType::CutInsideShot
				&& Branch.CutInsideShot
					== EMatchPlayCutInsideShotActualBranch::DirectShot);
	}

	bool IsDeadCornerBranch(
		const FMatchPlayCurrentAttackActualBranch& Branch)
	{
		return (Branch.ActionType == ESkillRuleType::LongShot
				&& Branch.LongShot
					== EMatchPlayLongShotActualBranch::DeadCorner)
			|| (Branch.ActionType == ESkillRuleType::CutInsideShot
				&& Branch.CutInsideShot
					== EMatchPlayCutInsideShotActualBranch::DeadCorner);
	}

	ESource SelectDirectShotImmediateMissSource(
		const ESkillRuleType ActionType)
	{
		if (ActionType == ESkillRuleType::LongShot)
		{
			return ESource::LongShotDirectShotImmediateMiss;
		}
		if (ActionType == ESkillRuleType::CutInsideShot)
		{
			return ESource::CutInsideShotDirectShotImmediateMiss;
		}
		return ESource::None;
	}

	ESource SelectDirectShotFormulaSource(
		const ESkillRuleType ActionType,
		const bool bIsGoal)
	{
		if (ActionType == ESkillRuleType::LongShot)
		{
			return bIsGoal
				? ESource::LongShotDirectShotFormulaGoal
				: ESource::LongShotDirectShotFormulaMiss;
		}
		if (ActionType == ESkillRuleType::CutInsideShot)
		{
			return bIsGoal
				? ESource::CutInsideShotDirectShotFormulaGoal
				: ESource::CutInsideShotDirectShotFormulaMiss;
		}
		return ESource::None;
	}

	ESource SelectDeadCornerSource(
		const ESkillRuleType ActionType,
		const bool bIsGoal)
	{
		if (ActionType == ESkillRuleType::LongShot)
		{
			return bIsGoal
				? ESource::LongShotDeadCornerGoal
				: ESource::LongShotDeadCornerMiss;
		}
		if (ActionType == ESkillRuleType::CutInsideShot)
		{
			return bIsGoal
				? ESource::CutInsideShotDeadCornerGoal
				: ESource::CutInsideShotDeadCornerMiss;
		}
		return ESource::None;
	}

	bool RegenerateDirectShotFormula(
		FResult& Result,
		const FMatchPlayState& BeforeState,
		const FSkillRuleSnapshotSet* SkillRuleSet)
	{
		++Result.FormulaRegenerationCount;
		Result.FormulaRegenerationResult =
			FMatchPlayCurrentAttackResolveSingleCardFinishingFormulaOrchestrator
				::Resolve(BeforeState, SkillRuleSet);
		Result.RegenerationProviderCallCount +=
			Result.FormulaRegenerationResult
				.PlanRegenerationProviderCallCount;
		if (!Result.FormulaRegenerationResult.bSuccess)
		{
			SetFailure(
				Result,
				EError::FormulaRegenerationFailed,
				Result.FormulaRegenerationResult.ErrorMessage,
				Result.FormulaRegenerationResult.InvalidField);
			return false;
		}

		const EFamily ExpectedFamily =
			Result.ActionType == ESkillRuleType::LongShot
				? EFamily::LongShotDirectShot
				: EFamily::CutInsideShotDirectShot;
		const FFormulaResolutionResult& FormulaResult =
			Result.FormulaRegenerationResult.FormulaResolutionResult;
		const bool bAttackerWinner =
			FormulaResult.Winner == EFormulaWinner::Attacker;
		const bool bDefenderWinner =
			FormulaResult.Winner == EFormulaWinner::Defender;
		if (Result.FormulaRegenerationResult.Family != ExpectedFamily
			|| !Result.FormulaRegenerationResult.bHasFormulaResolution
			|| (!bAttackerWinner && !bDefenderWinner)
			|| FormulaResult.FormulaType != EFormulaType::Finishing
			|| !FormulaResult.bAttackEnded
			|| FormulaResult.bContinueResolution
			|| FormulaResult.bIsGoal != bAttackerWinner)
		{
			SetFailure(
				Result,
				EError::InvalidTerminalFormulaSemantic,
				TEXT("Canonical DirectShot Formula result is not terminal."),
				TEXT("FormulaResolutionResult"));
			return false;
		}

		Result.bUsedFormula = true;
		Result.bIsGoal = FormulaResult.bIsGoal;
		Result.TerminalSource = SelectDirectShotFormulaSource(
			Result.ActionType,
			Result.bIsGoal);
		return Result.TerminalSource != ESource::None;
	}
}

FMatchPlayCurrentAttackApplyShotTerminalResolutionResult
FMatchPlayCurrentAttackApplyShotTerminalResolutionOrchestrator::Resolve(
	const FMatchPlayState& BeforeState,
	const FSkillRuleSnapshotSet* SkillRuleSet)
{
	using namespace MatchPlayCurrentAttackApplyShotTerminalResolution;

	FResult Result;
	Result.BeforeState = BeforeState;
	Result.AfterState = BeforeState;
	if (!BeforeState.RuntimeState.bIsInitialized)
	{
		SetFailure(Result, EError::MatchPlayStateNotInitialized,
			TEXT("Shot terminal application requires initialized State."));
		return Result;
	}
	if (!BeforeState.bHasCurrentAttack)
	{
		SetFailure(Result, EError::NoCurrentAttack,
			TEXT("Shot terminal application requires CurrentAttack."));
		return Result;
	}
	if (!BeforeState.CurrentAttack.bHasResolutionSession)
	{
		SetFailure(Result, EError::MissingResolutionSession,
			TEXT("Shot terminal application requires a Resolution Session."));
		return Result;
	}

	Result.SessionStateValidationResult =
		FMatchPlayCurrentAttackResolutionSessionStateValidator::Validate(
			BeforeState);
	if (!Result.SessionStateValidationResult.bIsCanonical)
	{
		SetFailure(Result, EError::InvalidResolutionSessionState,
			Result.SessionStateValidationResult.ErrorMessage);
		return Result;
	}
	const FMatchPlayCurrentAttackResolutionSession& Session =
		BeforeState.CurrentAttack.ResolutionSession;
	if (Session.Stage != EMatchPlayCurrentAttackResolutionStage::RouteResolved)
	{
		SetFailure(Result, EError::RouteNotResolved,
			TEXT("Shot terminal application requires RouteResolved."));
		return Result;
	}
	if (!Session.bHasActualBranch
		|| !IsShotAction(Session.ActualBranch.ActionType))
	{
		SetFailure(Result, EError::NotShotResolution,
			TEXT("This operation supports only LongShot/CutInsideShot resolution."));
		return Result;
	}

	Result.ActionType = Session.ActualBranch.ActionType;
	const bool bDirectShot = IsDirectShotBranch(Session.ActualBranch);
	const bool bDeadCorner = IsDeadCornerBranch(Session.ActualBranch);
	if (bDirectShot == bDeadCorner)
	{
		SetFailure(Result, EError::InvalidShotBranch,
			TEXT("Shot terminal application requires DirectShot or DeadCorner ActualBranch."));
		return Result;
	}

	if (bDirectShot)
	{
		FMatchPlayCurrentAttackResolveDirectShotPostRouteDecisionOrPlanRequest
			Request;
		Request.AttackSequence = BeforeState.CurrentAttack.AttackSequence;
		Request.Mode =
			FMatchPlayCurrentAttackResolveDirectShotPostRouteDecisionOrPlanRequest
				::EMode::RegenerateCompletedPlan;
		++Result.SourceRegenerationCount;
		Result.DirectShotRegenerationResult =
			FMatchPlayCurrentAttackResolveDirectShotPostRouteDecisionOrPlanOrchestrator
				::Resolve(BeforeState, Request, SkillRuleSet, nullptr);
		Result.RegenerationProviderCallCount +=
			Result.DirectShotRegenerationResult.ProviderCallCount;
		if (!Result.DirectShotRegenerationResult.bSuccess)
		{
			SetFailure(Result, EError::SourceRegenerationFailed,
				Result.DirectShotRegenerationResult.ErrorMessage);
			return Result;
		}
		if (!AreStatesEqual(
				BeforeState,
				Result.DirectShotRegenerationResult.AfterState))
		{
			SetFailure(Result, EError::SourceRegenerationMutatedState,
				TEXT("DirectShot terminal regeneration must be State-pure."));
			return Result;
		}
		if (Result.DirectShotRegenerationResult.ActionType != Result.ActionType)
		{
			SetFailure(Result, EError::InvalidDirectShotSemantic,
				TEXT("DirectShot regeneration changed the authoritative ActionType."));
			return Result;
		}

		bool bImmediateMiss = false;
		bool bFormulaRequired = false;
		if (Result.ActionType == ESkillRuleType::LongShot)
		{
			const FLongShotDirectShotPlanQueryResult& SourceResult =
				Result.DirectShotRegenerationResult.LongShotResult;
			bImmediateMiss = SourceResult.bSuccess
				&& SourceResult.Decision
					== ELongShotDirectShotDecision::ImmediateMiss
				&& SourceResult.bAttackEnded
				&& !SourceResult.bIsGoal
				&& !SourceResult.bHasFormulaPlan;
			bFormulaRequired = SourceResult.bSuccess
				&& SourceResult.Decision
					== ELongShotDirectShotDecision::FormulaResolutionRequired
				&& !SourceResult.bAttackEnded
				&& !SourceResult.bIsGoal
				&& SourceResult.bHasFormulaPlan;
		}
		else
		{
			const FCutInsideShotDirectShotPlanQueryResult& SourceResult =
				Result.DirectShotRegenerationResult.CutInsideShotResult;
			bImmediateMiss = SourceResult.bSuccess
				&& SourceResult.Decision
					== ECutInsideShotDirectShotDecision::ImmediateMiss
				&& SourceResult.bAttackEnded
				&& !SourceResult.bIsGoal
				&& !SourceResult.bHasFormulaPlan;
			bFormulaRequired = SourceResult.bSuccess
				&& SourceResult.Decision
					== ECutInsideShotDirectShotDecision::FormulaResolutionRequired
				&& !SourceResult.bAttackEnded
				&& !SourceResult.bIsGoal
				&& SourceResult.bHasFormulaPlan;
		}

		if (bImmediateMiss == bFormulaRequired)
		{
			SetFailure(Result, EError::InvalidDirectShotSemantic,
				TEXT("Canonical DirectShot result is not a supported terminal source."));
			return Result;
		}
		if (bImmediateMiss)
		{
			Result.bIsGoal = false;
			Result.TerminalSource =
				SelectDirectShotImmediateMissSource(Result.ActionType);
		}
		else if (!RegenerateDirectShotFormula(
			Result, BeforeState, SkillRuleSet))
		{
			return Result;
		}
	}
	else
	{
		FMatchPlayCurrentAttackResolveDeadCornerPostRouteDecisionRequest Request;
		Request.AttackSequence = BeforeState.CurrentAttack.AttackSequence;
		Request.Mode =
			FMatchPlayCurrentAttackResolveDeadCornerPostRouteDecisionRequest::EMode
				::RegenerateCompletedDecision;
		++Result.SourceRegenerationCount;
		Result.DeadCornerRegenerationResult =
			FMatchPlayCurrentAttackResolveDeadCornerPostRouteDecisionOrchestrator
				::Resolve(BeforeState, Request, SkillRuleSet, nullptr);
		Result.RegenerationProviderCallCount +=
			Result.DeadCornerRegenerationResult.ProviderCallCount;
		if (!Result.DeadCornerRegenerationResult.bSuccess)
		{
			SetFailure(Result, EError::SourceRegenerationFailed,
				Result.DeadCornerRegenerationResult.ErrorMessage);
			return Result;
		}
		if (!AreStatesEqual(
				BeforeState,
				Result.DeadCornerRegenerationResult.AfterState))
		{
			SetFailure(Result, EError::SourceRegenerationMutatedState,
				TEXT("DeadCorner terminal regeneration must be State-pure."));
			return Result;
		}
		if (Result.DeadCornerRegenerationResult.ActionType != Result.ActionType)
		{
			SetFailure(Result, EError::InvalidDeadCornerSemantic,
				TEXT("DeadCorner regeneration changed the authoritative ActionType."));
			return Result;
		}

		bool bGoal = false;
		bool bMiss = false;
		if (Result.ActionType == ESkillRuleType::LongShot)
		{
			const FLongShotDeadCornerDecisionQueryResult& SourceResult =
				Result.DeadCornerRegenerationResult.LongShotResult;
			bGoal = SourceResult.bSuccess
				&& SourceResult.Decision == ELongShotDeadCornerDecision::Goal
				&& SourceResult.bAttackEnded && SourceResult.bIsGoal;
			bMiss = SourceResult.bSuccess
				&& SourceResult.Decision == ELongShotDeadCornerDecision::Miss
				&& SourceResult.bAttackEnded && !SourceResult.bIsGoal;
		}
		else
		{
			const FCutInsideShotDeadCornerDecisionQueryResult& SourceResult =
				Result.DeadCornerRegenerationResult.CutInsideShotResult;
			bGoal = SourceResult.bSuccess
				&& SourceResult.Decision
					== ECutInsideShotDeadCornerDecision::Goal
				&& SourceResult.bAttackEnded && SourceResult.bIsGoal;
			bMiss = SourceResult.bSuccess
				&& SourceResult.Decision
					== ECutInsideShotDeadCornerDecision::Miss
				&& SourceResult.bAttackEnded && !SourceResult.bIsGoal;
		}
		if (bGoal == bMiss)
		{
			SetFailure(Result, EError::InvalidDeadCornerSemantic,
				TEXT("Canonical DeadCorner result is not Goal or Miss."));
			return Result;
		}
		Result.bIsGoal = bGoal;
		Result.TerminalSource =
			SelectDeadCornerSource(Result.ActionType, Result.bIsGoal);
	}

	if (Result.TerminalSource == ESource::None)
	{
		SetFailure(Result, EError::InvalidShotBranch,
			TEXT("Shot terminal source could not be selected."));
		return Result;
	}
	if (Result.RegenerationProviderCallCount != 0)
	{
		SetFailure(Result, EError::TerminalRegenerationConsumedRng,
			TEXT("Shot terminal regeneration must consume zero RNG."));
		return Result;
	}

	FMatchPlayShotResolutionTerminalCapability Capability(
		FMatchPlayShotResolutionTerminalCapability
			::FAuthoritativeTerminalIssuerTag(),
		BeforeState.CurrentAttack.AttackSequence,
		Result.TerminalSource,
		Result.bIsGoal);
	++Result.CompletionExecutionCount;
	Result.CompletionResult =
		FMatchPlayCurrentAttackCompletion::CompleteShotResolution(
			BeforeState,
			Capability);
	if (!Result.CompletionResult.bSuccess)
	{
		SetFailure(Result, EError::CompletionFailed,
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
