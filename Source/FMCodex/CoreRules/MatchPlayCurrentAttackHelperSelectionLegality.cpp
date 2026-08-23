#include "MatchPlayCurrentAttackHelperSelectionLegality.h"

FMatchPlayCurrentAttackHelperSelectionLegalityResult
FMatchPlayCurrentAttackHelperSelectionLegalityEvaluator::Evaluate(
	const FMatchPlayState& BeforeState,
	const FMatchPlayCurrentAttackHelperSelectionRequest& Request)
{
	const FMatchPlayCurrentAttackHelperSelectionGlobalContextResult Global =
		FMatchPlayCurrentAttackHelperSelectionGlobalContextQuery::Query(
			BeforeState,
			Request.AttackSequence,
			Request.RequestingSide);
	return EvaluateWithGlobalContext(BeforeState, Request, Global);
}

FMatchPlayCurrentAttackHelperSelectionLegalityResult
FMatchPlayCurrentAttackHelperSelectionLegalityEvaluator
	::EvaluateWithGlobalContext(
		const FMatchPlayState& BeforeState,
		const FMatchPlayCurrentAttackHelperSelectionRequest& Request,
		const FMatchPlayCurrentAttackHelperSelectionGlobalContextResult&
			GlobalContext)
{
	FMatchPlayCurrentAttackHelperSelectionLegalityResult Result;
	Result.Request = Request;
	Result.GlobalContextResult = GlobalContext;
	if (!GlobalContext.bSuccess)
	{
		Result.ErrorCode =
			EMatchPlayCurrentAttackHelperSelectionErrorCode
				::GlobalContextFailed;
		Result.ErrorMessage = GlobalContext.ErrorMessage;
		return Result;
	}
	Result.ResolvedActionType = GlobalContext.FrozenActionType;
	if (Result.ResolvedActionType != ESkillRuleType::None
		&& Result.ResolvedActionType != ESkillRuleType::PassControl
		&& Result.ResolvedActionType != ESkillRuleType::Cross
		&& Result.ResolvedActionType != ESkillRuleType::ThroughBall)
	{
		Result.ErrorCode =
			EMatchPlayCurrentAttackHelperSelectionErrorCode
				::UnsupportedHelperActionType;
		Result.ErrorMessage =
			TEXT("Helper selection does not support the frozen action type.");
		return Result;
	}

	const FMatchPlayCurrentAttackHelperParticipantAuthorityResult
		Authority =
			FMatchPlayCurrentAttackHelperParticipantAuthority::Evaluate(
				BeforeState,
				GlobalContext.CurrentDefendingPlayer,
				GlobalContext.FrozenMarkerCardId,
				Request.HelperCardId);
	Result.MatchingHelperPlacementCount =
		Authority.MatchingPlacementCount;
	Result.HelperPlacement = Authority.Placement;
	Result.HelperSnapshotQueryResult = Authority.SnapshotQueryResult;
	Result.ResolvedHelperSnapshot = Authority.Snapshot;
	Result.ErrorCode = Authority.ErrorCode;
	Result.ErrorMessage = Authority.ErrorMessage;
	if (!Authority.bSuccess)
	{
		return Result;
	}

	Result.PhysicalAreaMatchResult =
		FMatchPlayDeploymentPhysicalAreaMatchQuery::Query(
			BeforeState.DeploymentSlotCatalog,
			GlobalContext.CurrentAttackingPlayer,
			GlobalContext.FrozenRunnerPlacement,
			Result.HelperPlacement);
	if (!Result.PhysicalAreaMatchResult.bSuccess)
	{
		Result.ErrorCode =
			EMatchPlayCurrentAttackHelperSelectionErrorCode
				::PhysicalAreaQueryFailed;
		Result.ErrorMessage = Result.PhysicalAreaMatchResult.ErrorMessage;
		return Result;
	}

	if (!Result.PhysicalAreaMatchResult.bSamePhysicalArea)
	{
		Result.ErrorCode =
			EMatchPlayCurrentAttackHelperSelectionErrorCode
				::HelperNotInRunnerPhysicalArea;
		Result.ErrorMessage =
			TEXT("Helper must occupy the runner's physical area.");
		return Result;
	}

	Result.bSuccess = true;
	Result.ErrorCode =
		EMatchPlayCurrentAttackHelperSelectionErrorCode::None;
	Result.ErrorMessage.Empty();
	return Result;
}
