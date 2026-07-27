#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCurrentAttackSkillSelectionGlobalContextQuery.h"
#include "MatchPlaySkillParticipantRequirementQuery.h"
#include "SkillRuleSnapshotQuery.h"

#include "MatchPlayCurrentAttackSkillSelectionLegality.generated.h"

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayCurrentAttackSkillSelectionLegalityResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Skill Selection")
	bool bIsLegal = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Skill Selection")
	FMatchPlayCurrentAttackSkillSelectionRequest Request;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Skill Selection")
	EMatchPlayCurrentAttackSkillSelectionErrorCode ErrorCode =
		EMatchPlayCurrentAttackSkillSelectionErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Skill Selection")
	FMatchPlayCurrentAttackSelectionStateValidationResult
		SelectionStateValidationResult;

	FMatchPlayCurrentAttackSkillSelectionGlobalContextResult
		GlobalContextResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Skill Selection")
	FName FrozenCarrierCardId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Skill Selection")
	FName FrozenMarkerCardId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Skill Selection")
	int32 MatchingFrozenCarrierPlacementCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Skill Selection")
	int32 MatchingFrozenMarkerPlacementCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Skill Selection")
	FMatchPlayDeploymentPlacement FrozenCarrierPlacement;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Skill Selection")
	FMatchPlayDeploymentPlacement FrozenMarkerPlacement;

	FMatchPlayCardSnapshotAuthorityQueryResult CarrierSnapshotQueryResult;
	FSkillRuleSnapshotQueryResult SkillRuleQueryResult;
	FSkillRuleSnapshot ResolvedSkillRule;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Skill Selection")
	ESkillRuleType ResolvedActionType = ESkillRuleType::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Skill Selection")
	FMatchPlaySkillParticipantRequirementResult
		ParticipantRequirementResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Skill Selection")
	FString ErrorMessage;
};

class FMCODEX_API
	FMatchPlayCurrentAttackSkillSelectionLegalityEvaluator final
{
public:
	static FMatchPlayCurrentAttackSkillSelectionLegalityResult Evaluate(
		const FMatchPlayState& BeforeState,
		const FSkillRuleSnapshotSet& SkillRuleSet,
		const FMatchPlayCurrentAttackSkillSelectionRequest& Request);
};
