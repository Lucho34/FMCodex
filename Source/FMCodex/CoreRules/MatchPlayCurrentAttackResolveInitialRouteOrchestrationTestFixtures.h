#pragma once

#if WITH_DEV_AUTOMATION_TESTS

#include "MatchPlayCurrentAttackResolveInitialRouteOrchestrator.h"
#include "MatchPlayCurrentAttackResolveInitialRouteTestFixtures.h"

namespace FMCodex::Tests::MatchPlayCurrentAttackResolveInitialRouteOrchestration
{
	namespace RouteFixtures =
		FMCodex::Tests::MatchPlayCurrentAttackResolveInitialRoute;
	namespace BranchIntentFixtures =
		FMCodex::Tests::MatchPlayCurrentAttackBranchIntentSelection;
	namespace SessionFixtures =
		FMCodex::Tests::MatchPlayCurrentAttackResolutionSession;

	inline bool IsD6Action(const ESkillRuleType ActionType)
	{
		return ActionType == ESkillRuleType::Cross
			|| ActionType == ESkillRuleType::PassControl
			|| ActionType == ESkillRuleType::ThroughBall;
	}

	inline FMatchPlayState MakeReadyState(
		const ESkillRuleType ActionType,
		const EMatchPlayElectiveBranchIntent Intent =
			EMatchPlayElectiveBranchIntent::None)
	{
		if (ActionType == ESkillRuleType::LongShot
			|| ActionType == ESkillRuleType::CutInsideShot
			|| ActionType == ESkillRuleType::Cross)
		{
			return BranchIntentFixtures::MakeReadyState(
				ActionType,
				Intent);
		}
		return SessionFixtures::MakeReadyState(ActionType);
	}

	inline FMatchPlayState MakeAwaitingState(
		const ESkillRuleType ActionType,
		const EMatchPlayElectiveBranchIntent Intent =
			EMatchPlayElectiveBranchIntent::None)
	{
		return RouteFixtures::MakeAwaitingState(ActionType, Intent);
	}

	inline FMatchPlayCurrentAttackResolveInitialRouteOrchestrationRequest
	MakeRequest(const FMatchPlayState& State)
	{
		FMatchPlayCurrentAttackResolveInitialRouteOrchestrationRequest
			Request;
		Request.AttackSequence = State.CurrentAttack.AttackSequence;
		return Request;
	}

	inline FMatchPlayCurrentAttackActualBranch MakeExpectedBranch(
		const ESkillRuleType ActionType,
		const EMatchPlayElectiveBranchIntent Intent,
		const int32 RawD6)
	{
		FMatchPlayCurrentAttackActualBranch Branch;
		Branch.ActionType = ActionType;
		switch (ActionType)
		{
		case ESkillRuleType::LongShot:
			Branch.LongShot = Intent
				== EMatchPlayElectiveBranchIntent::DirectShot
					? EMatchPlayLongShotActualBranch::DirectShot
					: EMatchPlayLongShotActualBranch::DeadCorner;
			break;
		case ESkillRuleType::CutInsideShot:
			Branch.CutInsideShot = Intent
				== EMatchPlayElectiveBranchIntent::DirectShot
					? EMatchPlayCutInsideShotActualBranch::DirectShot
					: EMatchPlayCutInsideShotActualBranch::DeadCorner;
			break;
		case ESkillRuleType::Cross:
		{
			const bool bSameIntent = RawD6 <= 4;
			const bool bHigh = Intent
				== EMatchPlayElectiveBranchIntent::CrossHigh
					? bSameIntent
					: !bSameIntent;
			Branch.Cross = bHigh
				? EMatchPlayCrossActualBranch::High
				: EMatchPlayCrossActualBranch::Low;
			break;
		}
		case ESkillRuleType::PassControl:
			Branch.PassControl = RawD6 <= 2
				? EMatchPlayPassControlActualBranch::PassAdvance
				: RawD6 <= 4
					? EMatchPlayPassControlActualBranch::DribbleAdvance
					: EMatchPlayPassControlActualBranch::RunAdvance;
			break;
		case ESkillRuleType::ThroughBall:
			Branch.ThroughBall = RawD6 <= 2
				? EMatchPlayThroughBallActualBranch::Feet
				: RawD6 <= 4
					? EMatchPlayThroughBallActualBranch::BehindDefense
					: EMatchPlayThroughBallActualBranch::AntiOffside;
			break;
		case ESkillRuleType::None:
		default:
			break;
		}
		return Branch;
	}

	inline bool AreBeginResultsEqual(
		const FMatchPlayCurrentAttackBeginResolutionSessionWriterResult& Left,
		const FMatchPlayCurrentAttackBeginResolutionSessionWriterResult& Right)
	{
		return Left.bSuccess == Right.bSuccess
			&& Left.bCreatedNewSession == Right.bCreatedNewSession
			&& Left.Request.AttackSequence == Right.Request.AttackSequence
			&& SessionFixtures::AreStatesEqual(
				Left.BeforeState,
				Right.BeforeState)
			&& SessionFixtures::AreStatesEqual(
				Left.AfterState,
				Right.AfterState)
			&& Left.ErrorCode == Right.ErrorCode
			&& SessionFixtures::AreLegalityResultsEqual(
				Left.LegalityResult,
				Right.LegalityResult)
			&& SessionFixtures::AreSessionsEqual(
				Left.Session,
				Right.Session)
			&& Left.ErrorMessage == Right.ErrorMessage;
	}

	inline bool AreResultsEqual(
		const FMatchPlayCurrentAttackResolveInitialRouteOrchestrationResult&
			Left,
		const FMatchPlayCurrentAttackResolveInitialRouteOrchestrationResult&
			Right)
	{
		return Left.bSuccess == Right.bSuccess
			&& Left.bBeganNewSession == Right.bBeganNewSession
			&& Left.bResolvedNewRoute == Right.bResolvedNewRoute
			&& Left.Request.AttackSequence == Right.Request.AttackSequence
			&& RouteFixtures::AreStatesEqual(
				Left.BeforeState,
				Right.BeforeState)
			&& RouteFixtures::AreStatesEqual(
				Left.AfterState,
				Right.AfterState)
			&& AreBeginResultsEqual(
				Left.BeginResult,
				Right.BeginResult)
			&& RouteFixtures::AreWriterResultsEqual(
				Left.RouteResult,
				Right.RouteResult)
			&& Left.FailureStage == Right.FailureStage
			&& Left.FailureDisposition == Right.FailureDisposition
			&& Left.bProviderCalled == Right.bProviderCalled
			&& Left.ErrorCode == Right.ErrorCode
			&& Left.ErrorMessage == Right.ErrorMessage;
	}
}

#endif
