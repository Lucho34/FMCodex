#pragma once

#if WITH_DEV_AUTOMATION_TESTS

#include "MatchPlayBoundActionParticipantNormalizationTestFixtures.h"
#include "MatchPlayCurrentAttackBeginResolutionSessionWriter.h"

namespace FMCodex::Tests::MatchPlayCurrentAttackResolutionSession
{
	namespace NormalizationFixtures =
		FMCodex::Tests::MatchPlayBoundActionParticipantNormalization;

	inline FMatchPlayState MakeReadyState(
		const ESkillRuleType ActionType,
		const bool bHasHelper = false)
	{
		return NormalizationFixtures::MakeReadyState(
			ActionType,
			bHasHelper);
	}

	inline FMatchPlayCurrentAttackBeginResolutionSessionRequest
	MakeRequest(const FMatchPlayState& State)
	{
		FMatchPlayCurrentAttackBeginResolutionSessionRequest Request;
		Request.AttackSequence =
			State.CurrentAttack.AttackSequence;
		return Request;
	}

	inline bool AreStatesEqual(
		const FMatchPlayState& Left,
		const FMatchPlayState& Right)
	{
		return FMatchPlayState::StaticStruct()->CompareScriptStruct(
			&Left,
			&Right,
			0);
	}

	inline bool AreSessionsEqual(
		const FMatchPlayCurrentAttackResolutionSession& Left,
		const FMatchPlayCurrentAttackResolutionSession& Right)
	{
		return FMatchPlayCurrentAttackResolutionSession::StaticStruct()
			->CompareScriptStruct(&Left, &Right, 0);
	}

	inline bool ParticipantMatches(
		const FMatchPlayCurrentAttackResolutionSessionParticipant&
			SessionParticipant,
		const FMatchPlayBoundActionNormalizedParticipant&
			NormalizedParticipant)
	{
		return SessionParticipant.bIsPresent
				== NormalizedParticipant.bIsPresent
			&& SessionParticipant.Side == NormalizedParticipant.Side
			&& SessionParticipant.CardId
				== NormalizedParticipant.CardId
			&& FMatchPlayBoundActionNormalizedParticipantValues
				::StaticStruct()->CompareScriptStruct(
					&SessionParticipant.Values,
					&NormalizedParticipant.Values,
					0);
	}

	inline bool SessionMatchesNormalization(
		const FMatchPlayCurrentAttackResolutionSession& Session,
		const FMatchPlayBoundActionParticipantNormalizationResult&
			Normalization)
	{
		const FMatchPlayBoundActionNormalizedParticipantBundle& Source =
			Normalization.Bundle;
		const FMatchPlayCurrentAttackActualBranch DefaultActualBranch;
		return Session.AttackSequence == Source.AttackSequence
			&& Session.Stage
				== EMatchPlayCurrentAttackResolutionStage
					::AwaitingRoute
			&& !Session.bHasActualBranch
			&& FMatchPlayCurrentAttackActualBranch::StaticStruct()
				->CompareScriptStruct(
					&Session.ActualBranch,
					&DefaultActualBranch,
					0)
			&& Session.InitialRouteRollRecords.IsEmpty()
			&& FMatchPlayCurrentAttackResolutionBindingValue
				::StaticStruct()->CompareScriptStruct(
					&Session.Bundle.Binding,
					&Source.Binding,
					0)
			&& Session.Bundle.CurrentAttackingPlayer
				== Source.CurrentAttackingPlayer
			&& Session.Bundle.CurrentDefendingPlayer
				== Source.CurrentDefendingPlayer
			&& ParticipantMatches(
				Session.Bundle.Carrier,
				Source.Carrier)
			&& ParticipantMatches(
				Session.Bundle.Marker,
				Source.Marker)
			&& Session.Bundle.bHasRunner == Source.bHasRunner
			&& ParticipantMatches(
				Session.Bundle.Runner,
				Source.Runner)
			&& Session.Bundle.bHasHelper == Source.bHasHelper
			&& ParticipantMatches(
				Session.Bundle.Helper,
				Source.Helper);
	}

	inline bool OnlySessionChanged(
		const FMatchPlayState& Before,
		const FMatchPlayState& After)
	{
		FMatchPlayState WithoutSession = After;
		WithoutSession.CurrentAttack.bHasResolutionSession = false;
		WithoutSession.CurrentAttack.ResolutionSession =
			FMatchPlayCurrentAttackResolutionSession();
		return AreStatesEqual(Before, WithoutSession);
	}

	inline bool AreGlobalContextResultsEqual(
		const
			FMatchPlayCurrentAttackBeginResolutionSessionGlobalContextResult&
				Left,
		const
			FMatchPlayCurrentAttackBeginResolutionSessionGlobalContextResult&
				Right)
	{
		return
			FMatchPlayCurrentAttackBeginResolutionSessionGlobalContextResult
				::StaticStruct()->CompareScriptStruct(
					&Left,
					&Right,
					0)
			&& NormalizationFixtures::AreReadyResultsEqual(
				Left.ReadyValidationResult,
				Right.ReadyValidationResult)
			&& NormalizationFixtures::AreReadyResultsEqual(
				Left.BindingResult.ReadyValidationResult,
				Right.BindingResult.ReadyValidationResult)
			&& NormalizationFixtures::AreResultsEqual(
				Left.NormalizationResult,
				Right.NormalizationResult);
	}

	inline bool AreLegalityResultsEqual(
		const FMatchPlayCurrentAttackBeginResolutionSessionLegalityResult&
			Left,
		const FMatchPlayCurrentAttackBeginResolutionSessionLegalityResult&
			Right)
	{
		return
			FMatchPlayCurrentAttackBeginResolutionSessionLegalityResult
				::StaticStruct()->CompareScriptStruct(
					&Left,
					&Right,
					0)
			&& AreGlobalContextResultsEqual(
				Left.GlobalContextResult,
				Right.GlobalContextResult);
	}
}

#endif
