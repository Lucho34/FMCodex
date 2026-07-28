#include "MatchPlayCurrentAttackRunnerSelectionLegality.h"

namespace MatchPlayCurrentAttackRunnerSelectionLegalityImplementation
{
	void SetError(
		FMatchPlayCurrentAttackRunnerSelectionLegalityResult& Result,
		const EMatchPlayCurrentAttackRunnerSelectionErrorCode ErrorCode,
		const FString& ErrorMessage)
	{
		Result.ErrorCode = ErrorCode;
		Result.ErrorMessage = ErrorMessage;
	}
}

FMatchPlayCurrentAttackRunnerSelectionLegalityResult
FMatchPlayCurrentAttackRunnerSelectionLegalityEvaluator::Evaluate(
	const FMatchPlayState& BeforeState,
	const FMatchPlayCurrentAttackRunnerSelectionRequest& Request)
{
	using namespace
		MatchPlayCurrentAttackRunnerSelectionLegalityImplementation;

	FMatchPlayCurrentAttackRunnerSelectionLegalityResult Result;
	Result.Request = Request;
	Result.GlobalContextResult =
		FMatchPlayCurrentAttackRunnerSelectionGlobalContextQuery::Query(
			BeforeState,
			Request.AttackSequence,
			Request.RequestingSide);
	if (!Result.GlobalContextResult.bSuccess)
	{
		SetError(
			Result,
			Result.GlobalContextResult.ErrorCode,
			Result.GlobalContextResult.ErrorMessage);
		return Result;
	}
	Result.ResolvedActionType =
		Result.GlobalContextResult.FrozenActionType;

	if (Request.RunnerCardId.IsNone())
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackRunnerSelectionErrorCode
				::InvalidRunnerCardId,
			TEXT("RunnerCardId must be non-empty."));
		return Result;
	}

	for (const FMatchPlayDeploymentPlacement& Placement :
		Result.GlobalContextResult.AttackingPlayerPlacements)
	{
		if (Placement.CardId == Request.RunnerCardId)
		{
			++Result.MatchingRunnerPlacementCount;
			Result.RunnerPlacement = Placement;
		}
	}
	if (Result.MatchingRunnerPlacementCount == 0)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackRunnerSelectionErrorCode
				::RunnerNotDeployed,
			TEXT("Runner must be deployed by the current attacker."));
		return Result;
	}
	if (Result.MatchingRunnerPlacementCount > 1)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackRunnerSelectionErrorCode
				::RunnerDeploymentAmbiguous,
			TEXT("Runner has multiple current-attacker deployment placements."));
		return Result;
	}

	Result.RunnerSnapshotQueryResult =
		FMatchPlayCardSnapshotAuthorityQuery::FindByPlayerSideAndCardId(
			BeforeState.CardSnapshotAuthority,
			Result.GlobalContextResult.CurrentAttackingPlayer,
			Request.RunnerCardId);
	if (!Result.RunnerSnapshotQueryResult.bSuccess)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackRunnerSelectionErrorCode
				::RunnerSnapshotQueryFailed,
			Result.RunnerSnapshotQueryResult.ErrorMessage);
		return Result;
	}
	Result.ResolvedRunnerSnapshot =
		Result.RunnerSnapshotQueryResult.Snapshot;

	if (Result.ResolvedRunnerSnapshot.bIsGoalkeeper)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackRunnerSelectionErrorCode
				::RunnerIsGoalkeeper,
			TEXT("A goalkeeper cannot be selected as the attack runner."));
		return Result;
	}

	if (Request.RunnerCardId
		== Result.GlobalContextResult.FrozenCarrierCardId)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackRunnerSelectionErrorCode
				::RunnerMatchesCarrier,
			TEXT("Runner and carrier must be different current-attacker participants."));
		return Result;
	}

	if (Result.ResolvedActionType == ESkillRuleType::PassControl)
	{
		if (!Result.ResolvedRunnerSnapshot.PositionTypes.Contains(
				EPlayerPositionType::Midfield))
		{
			SetError(
				Result,
				EMatchPlayCurrentAttackRunnerSelectionErrorCode
					::RunnerMissingRequiredPositionType,
				TEXT("PassControl requires a runner with Midfield position."));
			return Result;
		}
	}
	else if (Result.ResolvedActionType == ESkillRuleType::Cross)
	{
		if (!Result.ResolvedRunnerSnapshot.PositionTypes.Contains(
				EPlayerPositionType::Attack))
		{
			SetError(
				Result,
				EMatchPlayCurrentAttackRunnerSelectionErrorCode
					::RunnerMissingRequiredPositionType,
				TEXT("Cross requires a runner with Attack position."));
			return Result;
		}
	}
	else if (Result.ResolvedActionType
		== ESkillRuleType::ThroughBall)
	{
		Result.RelativeZoneResolveResult =
			FMatchPlayRelativeDeploymentZoneResolver::Resolve(
				BeforeState.DeploymentSlotCatalog,
				Result.RunnerPlacement.SlotId,
				Result.GlobalContextResult.CurrentAttackingPlayer,
				Result.GlobalContextResult.CurrentAttackingPlayer);
		if (!Result.RelativeZoneResolveResult.bSuccess)
		{
			SetError(
				Result,
				EMatchPlayCurrentAttackRunnerSelectionErrorCode
					::RunnerPhysicalAreaResolutionFailed,
				Result.RelativeZoneResolveResult.ErrorMessage);
			return Result;
		}
		if (Result.RelativeZoneResolveResult.RelativeZone
			!= EMatchPlayRelativeDeploymentZone::Forward)
		{
			SetError(
				Result,
				EMatchPlayCurrentAttackRunnerSelectionErrorCode
					::RunnerNotInAttackingForwardArea,
				TEXT("ThroughBall requires the runner in the attacking Forward area."));
			return Result;
		}
	}
	else
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackRunnerSelectionErrorCode
				::UnsupportedRunnerActionType,
			TEXT("Runner selection does not support the frozen action type."));
		return Result;
	}

	Result.bIsLegal = true;
	Result.ErrorCode =
		EMatchPlayCurrentAttackRunnerSelectionErrorCode::None;
	Result.ErrorMessage.Empty();
	return Result;
}
