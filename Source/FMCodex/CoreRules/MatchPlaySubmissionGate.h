#pragma once

#include "CoreMinimal.h"

#include "MatchPlayLoopReadiness.h"
#include "MatchPlayRequestValidationReport.h"

#include "MatchPlaySubmissionGate.generated.h"

UENUM(BlueprintType)
enum class EMatchPlaySubmissionGateCode : uint8
{
	CanSubmit UMETA(DisplayName = "Can Submit"),
	CannotAcceptAttackRequestNow
		UMETA(DisplayName = "Cannot Accept Attack Request Now"),
	MatchAlreadyFinished UMETA(DisplayName = "Match Already Finished"),
	NoRemainingAttackOpportunity
		UMETA(DisplayName = "No Remaining Attack Opportunity"),
	InvalidRequest UMETA(DisplayName = "Invalid Request"),
	InvalidAttackingPlayer UMETA(DisplayName = "Invalid Attacking Player"),
	NotCurrentAttacker UMETA(DisplayName = "Not Current Attacker"),
	EmptyCardId UMETA(DisplayName = "Empty Card ID"),
	CardNotAvailable UMETA(DisplayName = "Card Not Available"),
	CardAlreadyUsed UMETA(DisplayName = "Card Already Used")
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlaySubmissionGateResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Submission Gate")
	bool bCanSubmit = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Submission Gate")
	EMatchPlaySubmissionGateCode Code =
		EMatchPlaySubmissionGateCode::CannotAcceptAttackRequestNow;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Submission Gate")
	FString Reason;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Submission Gate")
	FString ErrorMessage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Submission Gate")
	FMatchPlayLoopReadinessResult LoopReadinessResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Submission Gate")
	FMatchPlayRequestValidationReport RequestValidationReport;
};

class FMCODEX_API FMatchPlaySubmissionGate final
{
public:
	static FMatchPlaySubmissionGateResult CanSubmit(
		const FMatchPlayState& State,
		const FMatchPlayAttackRequest& Request);
};
