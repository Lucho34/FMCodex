#pragma once

#include "CoreMinimal.h"

#include "MatchPlayState.h"

#include "MatchPlayCurrentAttackThroughBallOneOnOneShotChoiceSelectionTypes.generated.h"

USTRUCT(BlueprintType)
struct FMCODEX_API
	FMatchPlayCurrentAttackThroughBallOneOnOneShotChoiceSelectionRequest
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Rules|Match Play|Through Ball One-on-One Shot Choice")
	int64 AttackSequence = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Rules|Match Play|Through Ball One-on-One Shot Choice")
	EInitialTurnOrderPlayer RequestingSide = EInitialTurnOrderPlayer::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Rules|Match Play|Through Ball One-on-One Shot Choice")
	EMatchPlayThroughBallOneOnOneShotChoice Choice =
		EMatchPlayThroughBallOneOnOneShotChoice::None;
};

UENUM(BlueprintType)
enum class
	EMatchPlayCurrentAttackThroughBallOneOnOneShotChoiceSelectionErrorCode
	: uint8
{
	None UMETA(DisplayName = "None"),
	MatchPlayStateNotInitialized
		UMETA(DisplayName = "Match Play State Not Initialized"),
	NoCurrentAttack UMETA(DisplayName = "No Current Attack"),
	InvalidCurrentAttackSequence
		UMETA(DisplayName = "Invalid Current Attack Sequence"),
	InvalidRequestedAttackSequence
		UMETA(DisplayName = "Invalid Requested Attack Sequence"),
	AttackSequenceMismatch UMETA(DisplayName = "Attack Sequence Mismatch"),
	MissingResolutionSession UMETA(DisplayName = "Missing Resolution Session"),
	InvalidResolutionSessionState
		UMETA(DisplayName = "Invalid Resolution Session State"),
	RouteNotResolved UMETA(DisplayName = "Route Not Resolved"),
	NotThroughBallResolution UMETA(DisplayName = "Not Through Ball Resolution"),
	UnsupportedOneOnOneSource
		UMETA(DisplayName = "Unsupported One-on-One Source"),
	InvalidRequestingSide UMETA(DisplayName = "Invalid Requesting Side"),
	RequestingSideIsNotCurrentAttacker
		UMETA(DisplayName = "Requesting Side Is Not Current Attacker"),
	InvalidChoice UMETA(DisplayName = "Invalid Choice"),
	ChoiceAlreadySelected UMETA(DisplayName = "Choice Already Selected"),
	OneOnOneShotResolutionAlreadyStarted
		UMETA(DisplayName = "One-on-One Shot Resolution Already Started"),
	IncompleteSourceProvenance
		UMETA(DisplayName = "Incomplete Source Provenance"),
	SkillRuleSetUnavailable UMETA(DisplayName = "Skill Rule Set Unavailable"),
	SourceRegenerationFailed UMETA(DisplayName = "Source Regeneration Failed"),
	SourceDoesNotRequireOneOnOne
		UMETA(DisplayName = "Source Does Not Require One-on-One"),
	SourceRegenerationConsumedRng
		UMETA(DisplayName = "Source Regeneration Consumed RNG")
};
