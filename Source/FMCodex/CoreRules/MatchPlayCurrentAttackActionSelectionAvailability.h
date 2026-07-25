#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCurrentAttackActionSelectionLegality.h"

#include "MatchPlayCurrentAttackActionSelectionAvailability.generated.h"

UENUM(BlueprintType)
enum class EMatchPlayCurrentAttackActionSelectionAvailabilityErrorCode : uint8
{
	None UMETA(DisplayName = "None"),
	SkillRuleSetValidationFailed
		UMETA(DisplayName = "Skill Rule Set Validation Failed"),
	CarrierSnapshotEnumerationFailed
		UMETA(DisplayName = "Carrier Snapshot Enumeration Failed")
};

USTRUCT(BlueprintType)
struct FMCODEX_API
	FMatchPlayCurrentAttackActionSelectionCandidateAvailability
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Action Selection Availability")
	FName CarrierCardId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Action Selection Availability")
	FName SkillId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Action Selection Availability")
	FMatchPlayCurrentAttackActionSelectionLegalityResult LegalityResult;
};

USTRUCT(BlueprintType)
struct FMCODEX_API
	FMatchPlayCurrentAttackActionSelectionAvailabilityResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Action Selection Availability")
	bool bQuerySucceeded = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Action Selection Availability")
	bool bCanSelectAnyAction = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Action Selection Availability")
	int64 AttackSequence = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Action Selection Availability")
	EInitialTurnOrderPlayer RequestingSide =
		EInitialTurnOrderPlayer::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Action Selection Availability")
	TArray<FMatchPlayCurrentAttackActionSelectionCandidateAvailability>
		Candidates;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Action Selection Availability")
	bool bHasFirstBlockingLegalityResult = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Action Selection Availability")
	FMatchPlayCurrentAttackActionSelectionLegalityResult
		FirstBlockingLegalityResult;

	ESkillRuleSnapshotValidationErrorCode
		UnderlyingSkillRuleSetValidationErrorCode =
			ESkillRuleSnapshotValidationErrorCode::None;

	EMatchPlayCardSnapshotAuthorityQueryErrorCode
		UnderlyingSnapshotAuthorityQueryErrorCode =
			EMatchPlayCardSnapshotAuthorityQueryErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Action Selection Availability")
	EMatchPlayCurrentAttackActionSelectionAvailabilityErrorCode ErrorCode =
		EMatchPlayCurrentAttackActionSelectionAvailabilityErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Action Selection Availability")
	FString ErrorMessage;
};

class FMCODEX_API FMatchPlayCurrentAttackActionSelectionAvailability final
{
public:
	static FMatchPlayCurrentAttackActionSelectionAvailabilityResult Query(
		const FMatchPlayState& BeforeState,
		int64 AttackSequence,
		EInitialTurnOrderPlayer RequestingSide,
		const FSkillRuleSnapshotSet& SkillRules);
};
