#include "MatchPlayCurrentAttackApplyCrossTerminalResolutionOrchestrator.h"

#include "MatchPlayCurrentAttackResolveSingleCardFinishingFormulaOrchestrator.h"

namespace MatchPlayCurrentAttackApplyCrossTerminalResolution
{
	// Terminal application replays the persisted Cross rolls through Formula authority.
	using FResult =
		FMatchPlayCurrentAttackApplyCrossTerminalResolutionResult;
	using EError =
		EMatchPlayCurrentAttackApplyCrossTerminalResolutionErrorCode;
	using ESource = EMatchPlayCrossTerminalSource;

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

	ESource SelectSource(
		const EMatchPlayCrossActualBranch Branch,
		const bool bIsGoal)
	{
		if (Branch == EMatchPlayCrossActualBranch::High)
		{
			return bIsGoal ? ESource::HighFormulaGoal : ESource::HighFormulaMiss;
		}
		if (Branch == EMatchPlayCrossActualBranch::Low)
		{
			return bIsGoal ? ESource::LowFormulaGoal : ESource::LowFormulaMiss;
		}
		return ESource::None;
	}
}

FMatchPlayCurrentAttackApplyCrossTerminalResolutionResult
FMatchPlayCurrentAttackApplyCrossTerminalResolutionOrchestrator::Resolve(
	const FMatchPlayState& BeforeState,
	const FSkillRuleSnapshotSet* SkillRuleSet)
{
	using namespace MatchPlayCurrentAttackApplyCrossTerminalResolution;

	FResult Result;
	Result.BeforeState = BeforeState;
	Result.AfterState = BeforeState;
	if (!BeforeState.RuntimeState.bIsInitialized)
	{
		SetFailure(
			Result,
			EError::MatchPlayStateNotInitialized,
			TEXT("Cross terminal application requires initialized State."));
		return Result;
	}
	if (!BeforeState.bHasCurrentAttack)
	{
		SetFailure(
			Result,
			EError::NoCurrentAttack,
			TEXT("Cross terminal application requires CurrentAttack."));
		return Result;
	}
	if (!BeforeState.CurrentAttack.bHasResolutionSession)
	{
		SetFailure(
			Result,
			EError::MissingResolutionSession,
			TEXT("Cross terminal application requires a Resolution Session."));
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
			TEXT("Cross terminal application requires RouteResolved."));
		return Result;
	}
	if (!Session.bHasActualBranch
		|| Session.ActualBranch.ActionType != ESkillRuleType::Cross)
	{
		SetFailure(
			Result,
			EError::NotCrossResolution,
			TEXT("This operation supports only Cross resolution."));
		return Result;
	}
	const EMatchPlayCrossActualBranch CrossBranch = Session.ActualBranch.Cross;
	if (CrossBranch != EMatchPlayCrossActualBranch::High
		&& CrossBranch != EMatchPlayCrossActualBranch::Low)
	{
		SetFailure(
			Result,
			EError::InvalidCrossBranch,
			TEXT("Cross terminal application requires High or Low ActualBranch."));
		return Result;
	}

	++Result.FormulaRegenerationCount;
	Result.FormulaRegenerationResult =
		FMatchPlayCurrentAttackResolveSingleCardFinishingFormulaOrchestrator
			::Resolve(BeforeState, SkillRuleSet);
	Result.RegenerationProviderCallCount =
		Result.FormulaRegenerationResult.PlanRegenerationProviderCallCount;
	if (!Result.FormulaRegenerationResult.bSuccess)
	{
		SetFailure(
			Result,
			EError::FormulaRegenerationFailed,
			Result.FormulaRegenerationResult.ErrorMessage,
			Result.FormulaRegenerationResult.InvalidField);
		return Result;
	}
	const FFormulaResolutionResult& FormulaResult =
		Result.FormulaRegenerationResult.FormulaResolutionResult;
	const bool bAttackerWinner =
		FormulaResult.Winner == EFormulaWinner::Attacker;
	const bool bDefenderWinner =
		FormulaResult.Winner == EFormulaWinner::Defender;
	if (Result.FormulaRegenerationResult.Family
			!= EMatchPlaySingleCardFinishingFormulaFamily::Cross
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
			TEXT("Canonical Cross Formula result is not terminal."),
			TEXT("FormulaResolutionResult"));
		return Result;
	}
	if (Result.RegenerationProviderCallCount != 0)
	{
		SetFailure(
			Result,
			EError::TerminalRegenerationConsumedRng,
			TEXT("Cross terminal regeneration must consume zero RNG."));
		return Result;
	}

	Result.bIsGoal = FormulaResult.bIsGoal;
	Result.TerminalSource = SelectSource(CrossBranch, Result.bIsGoal);
	if (Result.TerminalSource == ESource::None)
	{
		SetFailure(
			Result,
			EError::InvalidTerminalFormulaSemantic,
			TEXT("Cross terminal source could not be selected."));
		return Result;
	}

	FMatchPlayCrossResolutionTerminalCapability Capability(
		FMatchPlayCrossResolutionTerminalCapability
			::FAuthoritativeTerminalIssuerTag(),
		BeforeState.CurrentAttack.AttackSequence,
		Result.TerminalSource,
		Result.bIsGoal);
	++Result.CompletionExecutionCount;
	Result.CompletionResult =
		FMatchPlayCurrentAttackCompletion::CompleteCrossResolution(
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
