#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCurrentAttackHelperParticipantAuthority.h"
#include "MatchPlayCurrentAttackHelperSelectionGlobalContextQuery.h"

#include "MatchPlayCurrentAttackHelperSelectionLegality.generated.h"

USTRUCT(BlueprintType)
struct FMCODEX_API
	FMatchPlayCurrentAttackHelperSelectionLegalityResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Helper Selection")
	bool bSuccess = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Helper Selection")
	FMatchPlayCurrentAttackHelperSelectionRequest Request;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Helper Selection")
	EMatchPlayCurrentAttackHelperSelectionErrorCode ErrorCode =
		EMatchPlayCurrentAttackHelperSelectionErrorCode::None;

	FMatchPlayCurrentAttackHelperSelectionGlobalContextResult
		GlobalContextResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Helper Selection")
	int32 MatchingHelperPlacementCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Helper Selection")
	FMatchPlayDeploymentPlacement HelperPlacement;

	FMatchPlayCardSnapshotAuthorityQueryResult
		HelperSnapshotQueryResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Helper Selection")
	FPlayerCardRuleSnapshot ResolvedHelperSnapshot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Helper Selection")
	ESkillRuleType ResolvedActionType = ESkillRuleType::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Helper Selection")
	FString ErrorMessage;
};

class FMatchPlayCurrentAttackHelperSelectionAvailability;

class FMCODEX_API
	FMatchPlayCurrentAttackHelperSelectionLegalityEvaluator final
{
public:
	static FMatchPlayCurrentAttackHelperSelectionLegalityResult Evaluate(
		const FMatchPlayState& BeforeState,
		const FMatchPlayCurrentAttackHelperSelectionRequest& Request);

private:
	friend class FMatchPlayCurrentAttackHelperSelectionAvailability;

	static FMatchPlayCurrentAttackHelperSelectionLegalityResult
		EvaluateWithGlobalContext(
			const FMatchPlayState& BeforeState,
			const FMatchPlayCurrentAttackHelperSelectionRequest& Request,
			const FMatchPlayCurrentAttackHelperSelectionGlobalContextResult&
				GlobalContext);
};
