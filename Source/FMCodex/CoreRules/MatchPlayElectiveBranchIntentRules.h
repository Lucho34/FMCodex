#pragma once

#include "MatchPlayState.h"

namespace MatchPlayElectiveBranchIntentRules
{
	inline bool IsElectiveActionType(const ESkillRuleType ActionType)
	{
		return ActionType == ESkillRuleType::LongShot
			|| ActionType == ESkillRuleType::CutInsideShot
			|| ActionType == ESkillRuleType::Cross;
	}

	inline bool IsKnownIntent(
		const EMatchPlayElectiveBranchIntent Intent)
	{
		return Intent == EMatchPlayElectiveBranchIntent::None
			|| Intent == EMatchPlayElectiveBranchIntent::DirectShot
			|| Intent == EMatchPlayElectiveBranchIntent::DeadCorner
			|| Intent == EMatchPlayElectiveBranchIntent::CrossHigh
			|| Intent == EMatchPlayElectiveBranchIntent::CrossLow;
	}

	inline bool IsLegalIntent(
		const ESkillRuleType ActionType,
		const EMatchPlayElectiveBranchIntent Intent)
	{
		switch (ActionType)
		{
		case ESkillRuleType::LongShot:
		case ESkillRuleType::CutInsideShot:
			return Intent
					== EMatchPlayElectiveBranchIntent::DirectShot
				|| Intent
					== EMatchPlayElectiveBranchIntent::DeadCorner;

		case ESkillRuleType::Cross:
			return Intent == EMatchPlayElectiveBranchIntent::CrossHigh
				|| Intent == EMatchPlayElectiveBranchIntent::CrossLow;

		case ESkillRuleType::PassControl:
		case ESkillRuleType::ThroughBall:
			return Intent == EMatchPlayElectiveBranchIntent::None;

		case ESkillRuleType::None:
		default:
			return false;
		}
	}
}
