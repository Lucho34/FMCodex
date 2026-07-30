#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCurrentAttackBranchIntentSelectionGlobalContextQuery.h"

#include "MatchPlayCurrentAttackBranchIntentSelectionLegality.generated.h"

USTRUCT(BlueprintType)
struct FMCODEX_API
	FMatchPlayCurrentAttackBranchIntentSelectionLegalityResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Branch Intent Selection")
	bool bIsLegal = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Branch Intent Selection")
	FMatchPlayCurrentAttackBranchIntentSelectionRequest Request;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Branch Intent Selection")
	EMatchPlayCurrentAttackBranchIntentSelectionErrorCode ErrorCode =
		EMatchPlayCurrentAttackBranchIntentSelectionErrorCode::None;

	FMatchPlayCurrentAttackBranchIntentSelectionGlobalContextResult
		GlobalContextResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Branch Intent Selection")
	ESkillRuleType ResolvedActionType = ESkillRuleType::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Branch Intent Selection")
	EMatchPlayElectiveBranchIntent ResolvedIntent =
		EMatchPlayElectiveBranchIntent::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Branch Intent Selection")
	FString ErrorMessage;
};

class FMCODEX_API
	FMatchPlayCurrentAttackBranchIntentSelectionLegalityEvaluator final
{
public:
	static
		FMatchPlayCurrentAttackBranchIntentSelectionLegalityResult
		Evaluate(
			const FMatchPlayState& BeforeState,
			const FMatchPlayCurrentAttackBranchIntentSelectionRequest&
				Request);
};
