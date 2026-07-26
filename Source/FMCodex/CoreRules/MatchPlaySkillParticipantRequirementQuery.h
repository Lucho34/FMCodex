#pragma once

#include "CoreMinimal.h"

#include "SkillRuleSnapshot.h"

#include "MatchPlaySkillParticipantRequirementQuery.generated.h"

UENUM(BlueprintType)
enum class EMatchPlaySkillParticipantRequirementErrorCode : uint8
{
	None UMETA(DisplayName = "None"),
	UnsupportedSkillRuleType
		UMETA(DisplayName = "Unsupported Skill Rule Type")
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlaySkillParticipantRequirementResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Skill Participant Requirement")
	bool bSuccess = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Skill Participant Requirement")
	bool bRequiresRunner = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Skill Participant Requirement")
	bool bRequiresHelperStage = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Skill Participant Requirement")
	bool bCanBecomeReadyImmediately = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Skill Participant Requirement")
	EMatchPlaySkillParticipantRequirementErrorCode ErrorCode =
		EMatchPlaySkillParticipantRequirementErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Skill Participant Requirement")
	FString ErrorMessage;
};

class FMCODEX_API FMatchPlaySkillParticipantRequirementQuery final
{
public:
	static FMatchPlaySkillParticipantRequirementResult Query(
		ESkillRuleType SkillRuleType);
};
