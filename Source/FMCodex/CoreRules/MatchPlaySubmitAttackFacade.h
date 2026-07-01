#pragma once

#include "CoreMinimal.h"

#include "MatchPlayAttackStep.h"
#include "MatchPlaySubmissionGate.h"

#include "MatchPlaySubmitAttackFacade.generated.h"

UENUM(BlueprintType)
enum class EMatchPlaySubmitAttackFacadeCode : uint8
{
	Submitted UMETA(DisplayName = "Submitted"),
	SubmissionRejected UMETA(DisplayName = "Submission Rejected"),
	AttackStepFailed UMETA(DisplayName = "Attack Step Failed")
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlaySubmitAttackFacadeResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Submit Attack Facade")
	bool bSuccess = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Submit Attack Facade")
	EMatchPlaySubmitAttackFacadeCode Code =
		EMatchPlaySubmitAttackFacadeCode::SubmissionRejected;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Submit Attack Facade")
	FString Reason;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Submit Attack Facade")
	FString ErrorMessage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Submit Attack Facade")
	FMatchPlaySubmissionGateResult SubmissionGateResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Submit Attack Facade")
	FMatchPlayAttackStepResult AttackStepResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Submit Attack Facade")
	FMatchPlayExecutionSummary ExecutionSummary;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Submit Attack Facade")
	FMatchPlayState BeforeState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Submit Attack Facade")
	FMatchPlayState AfterState;
};

class FMCODEX_API FMatchPlaySubmitAttackFacade final
{
public:
	static FMatchPlaySubmitAttackFacadeResult Submit(
		const FMatchPlayState& BeforeState,
		const FMatchPlayAttackRequest& Request);
};
