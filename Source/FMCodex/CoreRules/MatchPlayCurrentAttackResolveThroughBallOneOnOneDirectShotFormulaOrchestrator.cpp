#include "MatchPlayCurrentAttackResolveThroughBallOneOnOneDirectShotFormulaOrchestrator.h"

#include "MatchPlayCurrentAttackResolveThroughBallOneOnOneDirectShotPostRoutePlanOrchestrator.h"

namespace MatchPlayCurrentAttackResolveThroughBallOneOnOneDirectShotFormula
{
	using FResult = FMatchPlayCurrentAttackResolveThroughBallOneOnOneDirectShotFormulaResult;
	using EError = EMatchPlayCurrentAttackResolveThroughBallOneOnOneDirectShotFormulaErrorCode;
	void Fail(FResult& Result, EError Error, const FString& Message, FName Field = NAME_None)
	{
		Result.ErrorCode = Error;
		Result.ErrorMessage = Message;
		Result.InvalidField = Field;
	}
	bool StatesEqual(const FMatchPlayState& Left, const FMatchPlayState& Right)
	{
		return FMatchPlayState::StaticStruct()->CompareScriptStruct(&Left, &Right, 0);
	}
}

FMatchPlayCurrentAttackResolveThroughBallOneOnOneDirectShotFormulaResult
FMatchPlayCurrentAttackResolveThroughBallOneOnOneDirectShotFormulaOrchestrator::Resolve(
	const FMatchPlayState& BeforeState,
	const FSkillRuleSnapshotSet* SkillRuleSet)
{
	using namespace MatchPlayCurrentAttackResolveThroughBallOneOnOneDirectShotFormula;
	FResult Result;
	Result.BeforeState = BeforeState;
	Result.AfterState = BeforeState;
	if (!BeforeState.RuntimeState.bIsInitialized) { Fail(Result, EError::MatchPlayStateNotInitialized, TEXT("DirectShot Formula requires initialized MatchPlay State.")); return Result; }
	if (!BeforeState.bHasCurrentAttack) { Fail(Result, EError::NoCurrentAttack, TEXT("DirectShot Formula requires a CurrentAttack.")); return Result; }
	if (!BeforeState.CurrentAttack.bHasResolutionSession) { Fail(Result, EError::MissingResolutionSession, TEXT("DirectShot Formula requires a Resolution Session.")); return Result; }
	Result.SessionStateValidationResult = FMatchPlayCurrentAttackResolutionSessionStateValidator::Validate(BeforeState);
	if (!Result.SessionStateValidationResult.bIsCanonical) { Fail(Result, EError::InvalidResolutionSessionState, Result.SessionStateValidationResult.ErrorMessage); return Result; }
	const auto& Session = BeforeState.CurrentAttack.ResolutionSession;
	if (Session.Stage != EMatchPlayCurrentAttackResolutionStage::RouteResolved) { Fail(Result, EError::RouteNotResolved, TEXT("DirectShot Formula requires RouteResolved.")); return Result; }
	if (!Session.bHasActualBranch || Session.ActualBranch.ActionType != ESkillRuleType::ThroughBall
		|| (Session.ActualBranch.ThroughBall != EMatchPlayThroughBallActualBranch::AntiOffside
			&& Session.ActualBranch.ThroughBall != EMatchPlayThroughBallActualBranch::BehindDefense))
	{
		Fail(Result, EError::UnsupportedThroughBallBranch, TEXT("DirectShot Formula supports OneOnOne ThroughBall branches only."));
		return Result;
	}
	if (Session.ThroughBallOneOnOneShotChoice != EMatchPlayThroughBallOneOnOneShotChoice::DirectShot)
	{
		Fail(Result, EError::ChoiceDoesNotPermitDirectShot, TEXT("Accepted choice does not permit DirectShot Formula."));
		return Result;
	}
	Result.ProgressResult = FMatchPlayCurrentAttackPostRouteRollProgressQuery::Evaluate(Session);
	if (!Result.ProgressResult.bIsCanonical) { Fail(Result, EError::InvalidPostRouteProgress, Result.ProgressResult.ErrorMessage); return Result; }
	if (Session.PostRouteRollProgress.Phase != EMatchPlayCurrentAttackPostRouteRollPhase::OneOnOneDirectShot
		|| !Result.ProgressResult.bContractComplete)
	{
		Fail(Result, EError::IncompletePostRouteRollProgress, TEXT("DirectShot Formula requires both accepted DirectShot rolls."));
		return Result;
	}
	Result.PlanRegenerationResult =
		FMatchPlayCurrentAttackResolveThroughBallOneOnOneDirectShotPostRoutePlanOrchestrator::Resolve(
			BeforeState, SkillRuleSet, nullptr);
	Result.PlanRegenerationProviderCallCount = Result.PlanRegenerationResult.ProviderCallCount;
	if (!Result.PlanRegenerationResult.bSuccess) { Fail(Result, EError::PlanRegenerationFailed, Result.PlanRegenerationResult.ErrorMessage, Result.PlanRegenerationResult.InvalidField); return Result; }
	if (Result.PlanRegenerationProviderCallCount != 0) { Fail(Result, EError::PlanRegenerationConsumedRng, TEXT("DirectShot Formula plan regeneration must consume zero RNG.")); return Result; }
	if (!StatesEqual(BeforeState, Result.PlanRegenerationResult.AfterState)) { Fail(Result, EError::PlanRegenerationMutatedState, TEXT("DirectShot Formula plan regeneration must not mutate State.")); return Result; }
	if (!Result.PlanRegenerationResult.bHasFormulaPlan) { Fail(Result, EError::FormulaPlanUnavailable, TEXT("DirectShot Formula plan is unavailable.")); return Result; }
	++Result.FormulaExecutionCount;
	Result.DirectShotFormulaResult = FThroughBallOneOnOneDirectShotFormula::Resolve(Result.PlanRegenerationResult.FormulaPlan);
	if (!Result.DirectShotFormulaResult.bSuccess) { Fail(Result, EError::FormulaExecutionFailed, Result.DirectShotFormulaResult.ErrorMessage, Result.DirectShotFormulaResult.InvalidField); return Result; }
	Result.Decision = Result.DirectShotFormulaResult.Decision;
	Result.bSuccess = true;
	return Result;
}
