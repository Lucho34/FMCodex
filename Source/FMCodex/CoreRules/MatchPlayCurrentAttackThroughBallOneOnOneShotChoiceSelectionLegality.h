#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCurrentAttackResolveThroughBallAntiOffsideDecisionTypes.h"
#include "MatchPlayCurrentAttackResolveThroughBallBehindDefenseP1FormulaTypes.h"
#include "MatchPlayCurrentAttackThroughBallOneOnOneShotChoiceSelectionTypes.h"

#include "MatchPlayCurrentAttackThroughBallOneOnOneShotChoiceSelectionLegality.generated.h"

USTRUCT(BlueprintType)
struct FMCODEX_API
	FMatchPlayCurrentAttackThroughBallOneOnOneShotChoiceSelectionLegalityResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Through Ball One-on-One Shot Choice")
	bool bIsLegal = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Through Ball One-on-One Shot Choice")
	FMatchPlayCurrentAttackThroughBallOneOnOneShotChoiceSelectionRequest Request;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Through Ball One-on-One Shot Choice")
	EMatchPlayCurrentAttackThroughBallOneOnOneShotChoiceSelectionErrorCode
		ErrorCode =
			EMatchPlayCurrentAttackThroughBallOneOnOneShotChoiceSelectionErrorCode
				::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Through Ball One-on-One Shot Choice")
	FMatchPlayCurrentAttackResolutionSessionStateValidationResult
		SessionStateValidationResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Through Ball One-on-One Shot Choice")
	FString ErrorMessage;

	int32 SourceRegenerationProviderCallCount = 0;
	FMatchPlayCurrentAttackResolveThroughBallAntiOffsideDecisionResult
		AntiOffsideRegenerationResult;
	FMatchPlayCurrentAttackResolveThroughBallBehindDefenseP1FormulaResult
		BehindDefenseP1RegenerationResult;
};

class FMCODEX_API
	FMatchPlayCurrentAttackThroughBallOneOnOneShotChoiceSelectionLegalityEvaluator
	final
{
public:
	static
		FMatchPlayCurrentAttackThroughBallOneOnOneShotChoiceSelectionLegalityResult
	Evaluate(
		const FMatchPlayState& BeforeState,
		const
			FMatchPlayCurrentAttackThroughBallOneOnOneShotChoiceSelectionRequest&
				Request,
		const FSkillRuleSnapshotSet* SkillRuleSet);
};
