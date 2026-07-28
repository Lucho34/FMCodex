#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCardSnapshotAuthority.h"
#include "MatchPlayCurrentAttackRunnerSelectionGlobalContextQuery.h"

#include "MatchPlayCurrentAttackRunnerSelectionLegality.generated.h"

USTRUCT(BlueprintType)
struct FMCODEX_API
	FMatchPlayCurrentAttackRunnerSelectionLegalityResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Runner Selection")
	bool bIsLegal = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Runner Selection")
	FMatchPlayCurrentAttackRunnerSelectionRequest Request;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Runner Selection")
	EMatchPlayCurrentAttackRunnerSelectionErrorCode ErrorCode =
		EMatchPlayCurrentAttackRunnerSelectionErrorCode::None;

	FMatchPlayCurrentAttackRunnerSelectionGlobalContextResult
		GlobalContextResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Runner Selection")
	int32 MatchingRunnerPlacementCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Runner Selection")
	FMatchPlayDeploymentPlacement RunnerPlacement;

	FMatchPlayCardSnapshotAuthorityQueryResult
		RunnerSnapshotQueryResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Runner Selection")
	FPlayerCardRuleSnapshot ResolvedRunnerSnapshot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Runner Selection")
	ESkillRuleType ResolvedActionType = ESkillRuleType::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Runner Selection")
	FMatchPlayRelativeDeploymentZoneResolveResult
		RelativeZoneResolveResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Runner Selection")
	FString ErrorMessage;
};

class FMCODEX_API
	FMatchPlayCurrentAttackRunnerSelectionLegalityEvaluator final
{
public:
	static FMatchPlayCurrentAttackRunnerSelectionLegalityResult Evaluate(
		const FMatchPlayState& BeforeState,
		const FMatchPlayCurrentAttackRunnerSelectionRequest& Request);
};
