#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCurrentAttackSkillSelectionLegality.h"

#include "MatchPlayCurrentAttackSkillSelectionAvailability.generated.h"

USTRUCT(BlueprintType)
struct FMCODEX_API
	FMatchPlayCurrentAttackSkillSelectionCandidateAvailability
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Skill Selection Availability")
	FName SkillId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Skill Selection Availability")
	FMatchPlayCurrentAttackSkillSelectionLegalityResult LegalityResult;
};

USTRUCT(BlueprintType)
struct FMCODEX_API
	FMatchPlayCurrentAttackSkillSelectionAvailabilityResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Skill Selection Availability")
	bool bQuerySucceeded = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Skill Selection Availability")
	bool bCanSelectAnySkill = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Skill Selection Availability")
	int64 AttackSequence = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Skill Selection Availability")
	EInitialTurnOrderPlayer RequestingSide =
		EInitialTurnOrderPlayer::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Skill Selection Availability")
	TArray<FMatchPlayCurrentAttackSkillSelectionCandidateAvailability>
		Candidates;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Skill Selection Availability")
	bool bHasGlobalBlockingLegalityResult = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Skill Selection Availability")
	FMatchPlayCurrentAttackSkillSelectionLegalityResult
		GlobalBlockingLegalityResult;

	FMatchPlayCurrentAttackSkillSelectionGlobalContextResult
		GlobalContextResult;
	FMatchPlayCardSnapshotAuthorityQueryResult
		CarrierSnapshotQueryResult;
	FSkillRuleSnapshotValidationResult
		SkillRuleSetValidationResult;
};

class FMCODEX_API
	FMatchPlayCurrentAttackSkillSelectionAvailability final
{
public:
	static FMatchPlayCurrentAttackSkillSelectionAvailabilityResult Query(
		const FMatchPlayState& BeforeState,
		int64 AttackSequence,
		EInitialTurnOrderPlayer RequestingSide,
		const FSkillRuleSnapshotSet& SkillRuleSet);
};
