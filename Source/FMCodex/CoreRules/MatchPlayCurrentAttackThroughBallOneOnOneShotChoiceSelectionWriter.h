#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCurrentAttackThroughBallOneOnOneShotChoiceSelectionLegality.h"

#include "MatchPlayCurrentAttackThroughBallOneOnOneShotChoiceSelectionWriter.generated.h"

UENUM(BlueprintType)
enum class
	EMatchPlayCurrentAttackThroughBallOneOnOneShotChoiceSelectionWriterErrorCode
	: uint8
{
	None UMETA(DisplayName = "None"),
	LegalityFailed UMETA(DisplayName = "Legality Failed"),
	InvalidCandidateState UMETA(DisplayName = "Invalid Candidate State")
};

USTRUCT(BlueprintType)
struct FMCODEX_API
	FMatchPlayCurrentAttackThroughBallOneOnOneShotChoiceSelectionWriterResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Through Ball One-on-One Shot Choice")
	bool bSuccess = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Through Ball One-on-One Shot Choice")
	FMatchPlayCurrentAttackThroughBallOneOnOneShotChoiceSelectionRequest Request;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Through Ball One-on-One Shot Choice")
	FMatchPlayState BeforeState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Through Ball One-on-One Shot Choice")
	FMatchPlayState AfterState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Through Ball One-on-One Shot Choice")
	FMatchPlayCurrentAttackThroughBallOneOnOneShotChoiceSelectionLegalityResult
		LegalityResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Through Ball One-on-One Shot Choice")
	FMatchPlayCurrentAttackResolutionSessionStateValidationResult
		CandidateStateValidationResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Through Ball One-on-One Shot Choice")
	EMatchPlayCurrentAttackThroughBallOneOnOneShotChoiceSelectionWriterErrorCode
		ErrorCode =
			EMatchPlayCurrentAttackThroughBallOneOnOneShotChoiceSelectionWriterErrorCode
				::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Through Ball One-on-One Shot Choice")
	FString ErrorMessage;
};

class FMCODEX_API
	FMatchPlayCurrentAttackThroughBallOneOnOneShotChoiceSelectionWriter final
{
public:
	static
		FMatchPlayCurrentAttackThroughBallOneOnOneShotChoiceSelectionWriterResult
	Select(
		const FMatchPlayState& BeforeState,
		const
			FMatchPlayCurrentAttackThroughBallOneOnOneShotChoiceSelectionRequest&
				Request,
		const FSkillRuleSnapshotSet* SkillRuleSet);
};
