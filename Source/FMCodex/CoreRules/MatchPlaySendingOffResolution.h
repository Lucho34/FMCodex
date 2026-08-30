#pragma once

#include "CoreMinimal.h"

#include "MatchPlayAttackEntryRollProvider.h"
#include "MatchPlayCardUsageStateValidator.h"
#include "MatchPlayCurrentAttackCompletion.h"
#include "MatchPlayCurrentAttackRouteStateValidator.h"

#include "MatchPlaySendingOffResolution.generated.h"

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlaySendingOffResolutionRequest
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Sending Off")
	int64 AttackSequence = 0;
};

UENUM(BlueprintType)
enum class EMatchPlaySendingOffResolutionErrorCode : uint8
{
	None UMETA(DisplayName = "None"),
	InvalidRouteState UMETA(DisplayName = "Invalid Route State"),
	InvalidAttackSequence UMETA(DisplayName = "Invalid Attack Sequence"),
	AttackSequenceMismatch UMETA(DisplayName = "Attack Sequence Mismatch"),
	InvalidCurrentAttacker UMETA(DisplayName = "Invalid Current Attacker"),
	InvalidCardUsageState UMETA(DisplayName = "Invalid Card Usage State"),
	CandidateSnapshotQueryFailed UMETA(DisplayName = "Candidate Snapshot Query Failed"),
	MissingSelectionProvider UMETA(DisplayName = "Missing Selection Provider"),
	SelectionProviderFailure UMETA(DisplayName = "Selection Provider Failure"),
	MalformedSelectionProviderResult UMETA(DisplayName = "Malformed Selection Provider Result"),
	EjectionMutationFailed UMETA(DisplayName = "Ejection Mutation Failed"),
	TerminalPersistenceFailed UMETA(DisplayName = "Terminal Persistence Failed")
};

struct FMCODEX_API FMatchPlaySendingOffCandidateQueryResult
{
	bool bSuccess = false;
	TArray<FName> CandidateCardIds;
	EMatchPlaySendingOffResolutionErrorCode ErrorCode =
		EMatchPlaySendingOffResolutionErrorCode::None;
	FMatchPlayCurrentAttackRouteStateValidationResult RouteValidationResult;
	FMatchPlayCardUsageStateValidationResult CardUsageValidationResult;
	FString ErrorMessage;
};

class FMCODEX_API FMatchPlaySendingOffCandidateQuery final
{
public:
	static FMatchPlaySendingOffCandidateQueryResult Query(
		const FMatchPlayState& State);
};

struct FMCODEX_API FMatchPlaySendingOffResolutionResult
{
	bool bSuccess = false;
	FMatchPlayState BeforeState;
	FMatchPlayState AfterState;
	TArray<FName> CandidateCardIds;
	FName EjectedCardId = NAME_None;
	bool bSelectionProviderCalled = false;
	FMatchPlayAttackEntrySelectionProviderResult ProviderResult;
	FMatchPlayAttackEntrySelectionProviderResultValidationResult
		ProviderValidationResult;
	FMatchPlayCurrentAttackCompletionResult CompletionResult;
	EMatchPlaySendingOffResolutionErrorCode ErrorCode =
		EMatchPlaySendingOffResolutionErrorCode::None;
	FString ErrorMessage;
};

class FMCODEX_API FMatchPlaySendingOffResolution final
{
public:
	static FMatchPlaySendingOffResolutionResult Resolve(
		const FMatchPlayState& BeforeState,
		const FMatchPlaySendingOffResolutionRequest& Request,
		IMatchPlayAttackEntryRollProvider* SelectionProvider);
};
