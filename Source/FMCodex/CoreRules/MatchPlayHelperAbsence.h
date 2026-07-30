#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCurrentAttackHelperSelectionAvailability.h"
#include "MatchPlayCurrentAttackReadyForResolutionValidator.h"
#include "MatchPlayCurrentAttackSelectionStateValidator.h"

#include "MatchPlayHelperAbsence.generated.h"

UENUM(BlueprintType)
enum class EMatchPlayHelperAbsenceReason : uint8
{
	None UMETA(DisplayName = "None"),
	NoLegalHelper UMETA(DisplayName = "No Legal Helper"),
	HelperDeclined UMETA(DisplayName = "Helper Declined")
};

UENUM(BlueprintType)
enum class EMatchPlayHelperAbsenceSource : uint8
{
	None UMETA(DisplayName = "None"),
	ResolveNoLegalHelper UMETA(DisplayName = "Resolve No Legal Helper"),
	HelperDecline UMETA(DisplayName = "Helper Decline")
};

UENUM(BlueprintType)
enum class EMatchPlayHelperAbsenceErrorCode : uint8
{
	None UMETA(DisplayName = "None"),
	MatchPlayStateNotInitialized
		UMETA(DisplayName = "Match Play State Not Initialized"),
	NoCurrentAttack UMETA(DisplayName = "No Current Attack"),
	InvalidCurrentAttackingPlayer
		UMETA(DisplayName = "Invalid Current Attacking Player"),
	InvalidCurrentDefendingPlayer
		UMETA(DisplayName = "Invalid Current Defending Player"),
	AvailabilityQueryFailed
		UMETA(DisplayName = "Availability Query Failed"),
	LegalHelperExists UMETA(DisplayName = "Legal Helper Exists"),
	NoLegalHelperToDecline
		UMETA(DisplayName = "No Legal Helper To Decline"),
	InvalidCapability UMETA(DisplayName = "Invalid Capability"),
	InvalidCapabilitySourceReason
		UMETA(DisplayName = "Invalid Capability Source Reason"),
	FinalizationFailed UMETA(DisplayName = "Finalization Failed"),
	SelectionStateValidationFailed
		UMETA(DisplayName = "Selection State Validation Failed"),
	ReadyValidationFailed UMETA(DisplayName = "Ready Validation Failed")
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayResolveNoLegalHelperRequest
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Rules|Match Play|Helper Absence")
	int64 AttackSequence = 0;
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayHelperDeclineRequest
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Rules|Match Play|Helper Absence")
	int64 AttackSequence = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Rules|Match Play|Helper Absence")
	EInitialTurnOrderPlayer RequestingSide =
		EInitialTurnOrderPlayer::None;
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayHelperAbsenceFinalizationResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Helper Absence")
	bool bSuccess = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Helper Absence")
	FMatchPlayState AfterState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Helper Absence")
	FMatchPlayCurrentAttackReadyValidationResult ReadyValidationResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Helper Absence")
	FMatchPlayCurrentAttackSelectionStateValidationResult
		SelectionStateValidationResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Helper Absence")
	EMatchPlayHelperAbsenceErrorCode ErrorCode =
		EMatchPlayHelperAbsenceErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Helper Absence")
	FString ErrorMessage;
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayResolveNoLegalHelperResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Helper Absence")
	bool bSuccess = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Helper Absence")
	FMatchPlayResolveNoLegalHelperRequest Request;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Helper Absence")
	FMatchPlayState BeforeState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Helper Absence")
	FMatchPlayState AfterState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Helper Absence")
	FMatchPlayCurrentAttackHelperSelectionAvailabilityResult
		HelperAvailabilityResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Helper Absence")
	EMatchPlayHelperAbsenceReason Reason =
		EMatchPlayHelperAbsenceReason::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Helper Absence")
	EMatchPlayHelperAbsenceSource Source =
		EMatchPlayHelperAbsenceSource::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Helper Absence")
	FMatchPlayHelperAbsenceFinalizationResult FinalizationResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Helper Absence")
	EMatchPlayHelperAbsenceErrorCode ErrorCode =
		EMatchPlayHelperAbsenceErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Helper Absence")
	FString ErrorMessage;
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayHelperDeclineResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Helper Absence")
	bool bSuccess = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Helper Absence")
	FMatchPlayHelperDeclineRequest Request;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Helper Absence")
	FMatchPlayState BeforeState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Helper Absence")
	FMatchPlayState AfterState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Helper Absence")
	FMatchPlayCurrentAttackHelperSelectionAvailabilityResult
		HelperAvailabilityResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Helper Absence")
	EMatchPlayHelperAbsenceReason Reason =
		EMatchPlayHelperAbsenceReason::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Helper Absence")
	EMatchPlayHelperAbsenceSource Source =
		EMatchPlayHelperAbsenceSource::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Helper Absence")
	FMatchPlayHelperAbsenceFinalizationResult FinalizationResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Helper Absence")
	EMatchPlayHelperAbsenceErrorCode ErrorCode =
		EMatchPlayHelperAbsenceErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Helper Absence")
	FString ErrorMessage;
};

class FMCODEX_API FMatchPlayResolveNoLegalHelper final
{
public:
	static FMatchPlayResolveNoLegalHelperResult Resolve(
		const FMatchPlayState& BeforeState,
		const FMatchPlayResolveNoLegalHelperRequest& Request);
};

class FMCODEX_API FMatchPlayHelperDecline final
{
public:
	static FMatchPlayHelperDeclineResult Decline(
		const FMatchPlayState& BeforeState,
		const FMatchPlayHelperDeclineRequest& Request);
};
