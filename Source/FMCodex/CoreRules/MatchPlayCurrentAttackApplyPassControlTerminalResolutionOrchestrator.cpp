#include "MatchPlayCurrentAttackApplyPassControlTerminalResolutionOrchestrator.h"

#include "MatchPlayCurrentAttackResolveSingleCardFinishingFormulaOrchestrator.h"

namespace MatchPlayCurrentAttackApplyPassControlTerminalResolution
{
	// Terminal application replays persisted PassControl rolls through the
	// canonical branch-plan and SingleCard Formula authorities.
	using FResult =
		FMatchPlayCurrentAttackApplyPassControlTerminalResolutionResult;
	using EError =
		EMatchPlayCurrentAttackApplyPassControlTerminalResolutionErrorCode;
	using ESource = EMatchPlayPassControlTerminalSource;
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

	EFamily SelectExpectedFamily(
		const EMatchPlayPassControlActualBranch Branch)
	{
		switch (Branch)
		{
		case EMatchPlayPassControlActualBranch::PassAdvance:
			return EFamily::PassAdvance;
		case EMatchPlayPassControlActualBranch::DribbleAdvance:
			return EFamily::DribbleAdvance;
		case EMatchPlayPassControlActualBranch::RunAdvance:
			return EFamily::RunAdvance;
		default:
			return EFamily::None;
		}
	}

	ESource SelectSource(
		const EMatchPlayPassControlActualBranch Branch,
		const bool bIsGoal)
	{
		switch (Branch)
		{
		case EMatchPlayPassControlActualBranch::PassAdvance:
			return bIsGoal
				? ESource::PassAdvanceFormulaGoal
				: ESource::PassAdvanceFormulaMiss;
		case EMatchPlayPassControlActualBranch::DribbleAdvance:
			return bIsGoal
				? ESource::DribbleAdvanceFormulaGoal
				: ESource::DribbleAdvanceFormulaMiss;
		case EMatchPlayPassControlActualBranch::RunAdvance:
			return bIsGoal
				? ESource::RunAdvanceFormulaGoal
				: ESource::RunAdvanceFormulaMiss;
		default:
			return ESource::None;
		}
	}
}

FMatchPlayCurrentAttackApplyPassControlTerminalResolutionResult
FMatchPlayCurrentAttackApplyPassControlTerminalResolutionOrchestrator::Resolve(
	const FMatchPlayState& BeforeState,
	const FSkillRuleSnapshotSet* SkillRuleSet)
{
	using namespace MatchPlayCurrentAttackApplyPassControlTerminalResolution;

	FResult Result;
	Result.BeforeState = BeforeState;
	Result.AfterState = BeforeState;
	if (!BeforeState.RuntimeState.bIsInitialized)
	{
		SetFailure(
			Result,
			EError::MatchPlayStateNotInitialized,
			TEXT("PassControl terminal application requires initialized State."));
		return Result;
	}
	if (!BeforeState.bHasCurrentAttack)
	{
		SetFailure(
			Result,
			EError::NoCurrentAttack,
			TEXT("PassControl terminal application requires CurrentAttack."));
		return Result;
	}
	if (!BeforeState.CurrentAttack.bHasResolutionSession)
	{
		SetFailure(
			Result,
			EError::MissingResolutionSession,
			TEXT("PassControl terminal application requires a Resolution Session."));
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
			TEXT("PassControl terminal application requires RouteResolved."));
		return Result;
	}
	if (!Session.bHasActualBranch
		|| Session.ActualBranch.ActionType != ESkillRuleType::PassControl)
	{
		SetFailure(
			Result,
			EError::NotPassControlResolution,
			TEXT("This operation supports only PassControl resolution."));
		return Result;
	}
	const EMatchPlayPassControlActualBranch PassControlBranch =
		Session.ActualBranch.PassControl;
	const EFamily ExpectedFamily = SelectExpectedFamily(PassControlBranch);
	if (ExpectedFamily == EFamily::None)
	{
		SetFailure(
			Result,
			EError::InvalidPassControlBranch,
			TEXT("PassControl terminal application requires PassAdvance, DribbleAdvance, or RunAdvance ActualBranch."));
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
			TEXT("Canonical PassControl Formula result is not terminal."),
			TEXT("FormulaResolutionResult"));
		return Result;
	}
	if (Result.RegenerationProviderCallCount != 0)
	{
		SetFailure(
			Result,
			EError::TerminalRegenerationConsumedRng,
			TEXT("PassControl terminal regeneration must consume zero RNG."));
		return Result;
	}

	Result.bIsGoal = FormulaResult.bIsGoal;
	Result.TerminalSource = SelectSource(PassControlBranch, Result.bIsGoal);
	if (Result.TerminalSource == ESource::None)
	{
		SetFailure(
			Result,
			EError::InvalidTerminalFormulaSemantic,
			TEXT("PassControl terminal source could not be selected."));
		return Result;
	}

	FMatchPlayPassControlResolutionTerminalCapability Capability(
		FMatchPlayPassControlResolutionTerminalCapability
			::FAuthoritativeTerminalIssuerTag(),
		BeforeState.CurrentAttack.AttackSequence,
		Result.TerminalSource,
		Result.bIsGoal);
	++Result.CompletionExecutionCount;
	Result.CompletionResult =
		FMatchPlayCurrentAttackCompletion::CompletePassControlResolution(
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
