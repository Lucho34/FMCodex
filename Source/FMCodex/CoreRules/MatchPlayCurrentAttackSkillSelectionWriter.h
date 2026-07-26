#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCurrentAttackSkillSelectionLegality.h"

#include "MatchPlayCurrentAttackSkillSelectionWriter.generated.h"

UENUM(BlueprintType)
enum class EMatchPlayCurrentAttackSkillSelectionWriterErrorCode : uint8
{
	None UMETA(DisplayName = "None"),
	LegalityFailed UMETA(DisplayName = "Legality Failed")
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayCurrentAttackSkillSelectionWriterResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Skill Selection Writer")
	bool bSuccess = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Skill Selection Writer")
	FMatchPlayCurrentAttackSkillSelectionRequest Request;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Skill Selection Writer")
	FMatchPlayState BeforeState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Skill Selection Writer")
	FMatchPlayState AfterState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Skill Selection Writer")
	EMatchPlayCurrentAttackSkillSelectionWriterErrorCode ErrorCode =
		EMatchPlayCurrentAttackSkillSelectionWriterErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Skill Selection Writer")
	FMatchPlayCurrentAttackSkillSelectionLegalityResult LegalityResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Skill Selection Writer")
	FName SelectedSkillId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Skill Selection Writer")
	ESkillRuleType SelectedActionType = ESkillRuleType::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Skill Selection Writer")
	FString ErrorMessage;
};

class FMCODEX_API FMatchPlayCurrentAttackSkillSelectionWriter final
{
public:
	static FMatchPlayCurrentAttackSkillSelectionWriterResult Select(
		const FMatchPlayState& BeforeState,
		const FSkillRuleSnapshotSet& SkillRuleSet,
		const FMatchPlayCurrentAttackSkillSelectionRequest& Request);
};
