#include "MatchPlaySkillParticipantRequirementQuery.h"

FMatchPlaySkillParticipantRequirementResult
FMatchPlaySkillParticipantRequirementQuery::Query(
	const ESkillRuleType SkillRuleType)
{
	FMatchPlaySkillParticipantRequirementResult Result;

	switch (SkillRuleType)
	{
	case ESkillRuleType::LongShot:
	case ESkillRuleType::CutInsideShot:
		Result.bRequiresRunner = false;
		Result.bRequiresHelperStage = false;
		Result.bCanBecomeReadyImmediately = true;
		break;

	case ESkillRuleType::PassControl:
	case ESkillRuleType::Cross:
	case ESkillRuleType::ThroughBall:
		Result.bRequiresRunner = true;
		Result.bRequiresHelperStage = true;
		Result.bCanBecomeReadyImmediately = false;
		break;

	case ESkillRuleType::None:
	default:
		Result.ErrorCode =
			EMatchPlaySkillParticipantRequirementErrorCode
				::UnsupportedSkillRuleType;
		Result.ErrorMessage =
			TEXT("SkillRuleType is not supported by match play participant selection.");
		return Result;
	}

	Result.bSuccess = true;
	Result.ErrorCode =
		EMatchPlaySkillParticipantRequirementErrorCode::None;
	return Result;
}
