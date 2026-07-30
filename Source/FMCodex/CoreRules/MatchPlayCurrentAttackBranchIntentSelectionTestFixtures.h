#pragma once

#if WITH_DEV_AUTOMATION_TESTS

#include "MatchPlayCurrentAttackBranchIntentSelectionWriter.h"
#include "MatchPlayCurrentAttackHelperSelectionTestFixtures.h"
#include "MatchPlayCurrentAttackSkillSelectionTestFixtures.h"
#include "MatchPlayCurrentAttackSkillSelectionWriter.h"
#include "MatchPlayHelperAbsence.h"

namespace FMCodex::Tests::MatchPlayCurrentAttackBranchIntentSelection
{
	namespace HelperFixtures =
		FMCodex::Tests::MatchPlayCurrentAttackHelperSelection;
	namespace SkillFixtures =
		FMCodex::Tests::MatchPlayCurrentAttackSkillSelection;

	enum class ECrossHelperPath : uint8
	{
		Selected,
		Declined,
		NoLegal
	};

	inline FName GetShotSkillId(const ESkillRuleType ActionType)
	{
		return ActionType == ESkillRuleType::CutInsideShot
			? SkillFixtures::CutInsideSkillId
			: SkillFixtures::LongShotSkillId;
	}

	inline FMatchPlayState MakeAwaitingShotIntent(
		const ESkillRuleType ActionType)
	{
		const FName SkillId = GetShotSkillId(ActionType);
		return FMatchPlayCurrentAttackSkillSelectionWriter::Select(
			SkillFixtures::MakeState({SkillId}),
			SkillFixtures::MakeRuleSet(),
			SkillFixtures::MakeRequest(SkillId))
			.AfterState;
	}

	inline FMatchPlayState MakeAwaitingCrossIntent(
		const ECrossHelperPath Path)
	{
		if (Path == ECrossHelperPath::Selected)
		{
			return
				FMatchPlayCurrentAttackHelperSelectionWriter::Select(
					HelperFixtures::MakeState(
						ESkillRuleType::Cross),
					HelperFixtures::MakeRequest())
					.AfterState;
		}

		if (Path == ECrossHelperPath::Declined)
		{
			FMatchPlayHelperDeclineRequest Request;
			Request.AttackSequence =
				HelperFixtures::ValidAttackSequence;
			Request.RequestingSide =
				EInitialTurnOrderPlayer::PlayerB;
			return FMatchPlayHelperDecline::Decline(
				HelperFixtures::MakeState(
					ESkillRuleType::Cross),
				Request)
				.AfterState;
		}

		FMatchPlayResolveNoLegalHelperRequest Request;
		Request.AttackSequence =
			HelperFixtures::ValidAttackSequence;
		return FMatchPlayResolveNoLegalHelper::Resolve(
			HelperFixtures::MakeZeroLegalStateForType(
				ESkillRuleType::Cross),
			Request)
			.AfterState;
	}

	inline FMatchPlayCurrentAttackBranchIntentSelectionRequest
	MakeRequest(
		const FMatchPlayState& State,
		const EMatchPlayElectiveBranchIntent Intent)
	{
		FMatchPlayCurrentAttackBranchIntentSelectionRequest Request;
		Request.AttackSequence =
			State.CurrentAttack.AttackSequence;
		Request.RequestingSide =
			State.RuntimeState.CurrentAttackingPlayer;
		Request.Intent = Intent;
		return Request;
	}

	inline FMatchPlayState MakeReadyState(
		const ESkillRuleType ActionType,
		const EMatchPlayElectiveBranchIntent Intent,
		const bool bHasHelper = true)
	{
		FMatchPlayState Awaiting;
		if (ActionType == ESkillRuleType::Cross)
		{
			Awaiting = MakeAwaitingCrossIntent(
				bHasHelper
					? ECrossHelperPath::Selected
					: ECrossHelperPath::Declined);
		}
		else
		{
			Awaiting = MakeAwaitingShotIntent(ActionType);
		}
		return
			FMatchPlayCurrentAttackBranchIntentSelectionWriter
				::Select(Awaiting, MakeRequest(Awaiting, Intent))
				.AfterState;
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
}

#endif
