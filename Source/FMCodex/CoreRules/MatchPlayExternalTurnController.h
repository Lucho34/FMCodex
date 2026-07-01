#pragma once

#include "CoreMinimal.h"

#include "MatchPlaySubmitAttackResultQuery.h"

#include "MatchPlayExternalTurnController.generated.h"

UENUM(BlueprintType)
enum class EMatchPlayExternalTurnControllerCode : uint8
{
	NotHandled UMETA(DisplayName = "Not Handled"),
	SubmittedSuccessfully UMETA(DisplayName = "Submitted Successfully"),
	SubmissionRejected UMETA(DisplayName = "Submission Rejected"),
	AttackStepFailed UMETA(DisplayName = "Attack Step Failed")
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayExternalTurnControllerResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play External Turn Controller")
	bool bHandled = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play External Turn Controller")
	bool bSubmittedSuccessfully = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play External Turn Controller")
	EMatchPlayExternalTurnControllerCode Code =
		EMatchPlayExternalTurnControllerCode::NotHandled;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play External Turn Controller")
	FString Reason;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play External Turn Controller")
	FString ErrorMessage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play External Turn Controller")
	FMatchPlaySubmitAttackFacadeResult SubmitResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play External Turn Controller")
	FMatchPlaySubmitAttackResultView ResultView;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play External Turn Controller")
	FMatchPlayState BeforeState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play External Turn Controller")
	FMatchPlayState AfterState;
};

class FMCODEX_API FMatchPlayExternalTurnController final
{
public:
	static FMatchPlayExternalTurnControllerResult HandleAttackRequest(
		const FMatchPlayState& BeforeState,
		const FMatchPlayAttackRequest& Request);
};
