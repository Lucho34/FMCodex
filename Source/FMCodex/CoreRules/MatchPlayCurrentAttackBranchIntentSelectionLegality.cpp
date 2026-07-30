#include "MatchPlayCurrentAttackBranchIntentSelectionLegality.h"

#include "MatchPlayElectiveBranchIntentRules.h"

FMatchPlayCurrentAttackBranchIntentSelectionLegalityResult
FMatchPlayCurrentAttackBranchIntentSelectionLegalityEvaluator::Evaluate(
	const FMatchPlayState& BeforeState,
	const FMatchPlayCurrentAttackBranchIntentSelectionRequest& Request)
{
	FMatchPlayCurrentAttackBranchIntentSelectionLegalityResult Result;
	Result.Request = Request;
	Result.GlobalContextResult =
		FMatchPlayCurrentAttackBranchIntentSelectionGlobalContextQuery
			::Query(
				BeforeState,
				Request.AttackSequence,
				Request.RequestingSide);
	if (!Result.GlobalContextResult.bSuccess)
	{
		Result.ErrorCode =
			EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
				::GlobalContextFailed;
		Result.ErrorMessage =
			Result.GlobalContextResult.ErrorMessage;
		return Result;
	}

	Result.ResolvedActionType =
		Result.GlobalContextResult.FrozenActionType;
	Result.ResolvedIntent = Request.Intent;
	if (!MatchPlayElectiveBranchIntentRules::IsKnownIntent(
			Request.Intent)
		|| Request.Intent
			== EMatchPlayElectiveBranchIntent::None)
	{
		Result.ErrorCode =
			EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
				::InvalidIntent;
		Result.ErrorMessage =
			TEXT("Branch Intent must be a known non-None player choice.");
		return Result;
	}
	if (!MatchPlayElectiveBranchIntentRules::IsLegalIntent(
			Result.ResolvedActionType,
			Request.Intent))
	{
		Result.ErrorCode =
			EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
				::IntentActionTypeMismatch;
		Result.ErrorMessage =
			TEXT("Branch Intent is not legal for the frozen action type.");
		return Result;
	}

	Result.bIsLegal = true;
	Result.ErrorCode =
		EMatchPlayCurrentAttackBranchIntentSelectionErrorCode::None;
	Result.ErrorMessage.Empty();
	return Result;
}
