#pragma once

#include "CoreMinimal.h"

#include "MatchPlayAttackRequest.h"
#include "MatchPlayLoopReadiness.h"

#include "MatchPlayRequestValidationReport.generated.h"

UENUM(BlueprintType)
enum class EMatchPlayRequestValidationCode : uint8
{
	Valid UMETA(DisplayName = "Valid"),
	MatchAlreadyFinished UMETA(DisplayName = "Match Already Finished"),
	CannotAcceptAttackRequestNow
		UMETA(DisplayName = "Cannot Accept Attack Request Now"),
	InvalidAttackingPlayer UMETA(DisplayName = "Invalid Attacking Player"),
	NotCurrentAttacker UMETA(DisplayName = "Not Current Attacker"),
	EmptyCardId UMETA(DisplayName = "Empty Card ID"),
	CardNotAvailable UMETA(DisplayName = "Card Not Available"),
	CardAlreadyUsed UMETA(DisplayName = "Card Already Used"),
	NoRemainingAttackOpportunity
		UMETA(DisplayName = "No Remaining Attack Opportunity"),
	InvalidRequest UMETA(DisplayName = "Invalid Request")
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayRequestValidationReport
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Request Validation Report")
	bool bIsValid = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Request Validation Report")
	bool bCanSubmitAttackRequest = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Request Validation Report")
	EMatchPlayRequestValidationCode Code =
		EMatchPlayRequestValidationCode::InvalidRequest;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Request Validation Report")
	FString Reason;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Request Validation Report")
	FString ErrorMessage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Request Validation Report")
	FMatchPlayLoopReadinessResult LoopReadinessResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Request Validation Report")
	FMatchPlayAttackRequestValidationResult RequestValidationResult;
};

class FMCODEX_API FMatchPlayRequestValidationReportQuery final
{
public:
	static FMatchPlayRequestValidationReport Validate(
		const FMatchPlayState& State,
		const FMatchPlayAttackRequest& Request);
};
