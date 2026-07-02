#pragma once

#include "CoreMinimal.h"

#include "MatchPlayExternalStateView.h"
#include "MatchPlaySubmissionGate.h"

#include "MatchPlayExternalAttackRequestPreflight.generated.h"

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayExternalAttackRequestPreflightView
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play External Attack Request Preflight")
	bool bCanSubmit = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play External Attack Request Preflight")
	bool bStateReadyForAttackRequest = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play External Attack Request Preflight")
	bool bRequestAcceptedByGate = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play External Attack Request Preflight")
	EInitialTurnOrderPlayer CurrentAttackingPlayer =
		EInitialTurnOrderPlayer::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play External Attack Request Preflight")
	EInitialTurnOrderPlayer RequestPlayer =
		EInitialTurnOrderPlayer::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play External Attack Request Preflight")
	FName CardId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play External Attack Request Preflight")
	EMatchPlaySubmissionGateCode GateCode =
		EMatchPlaySubmissionGateCode::CannotAcceptAttackRequestNow;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play External Attack Request Preflight")
	FString Reason;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play External Attack Request Preflight")
	FString CannotSubmitReason;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play External Attack Request Preflight")
	FString StateSummary;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play External Attack Request Preflight")
	FMatchPlayExternalStateView StateView;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play External Attack Request Preflight")
	FMatchPlaySubmissionGateResult SubmissionGateResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play External Attack Request Preflight")
	FMatchPlayRequestValidationReport ValidationReport;
};

class FMCODEX_API FMatchPlayExternalAttackRequestPreflight final
{
public:
	static FMatchPlayExternalAttackRequestPreflightView Evaluate(
		const FMatchPlayState& State,
		const FMatchPlayAttackRequest& Request);
};
