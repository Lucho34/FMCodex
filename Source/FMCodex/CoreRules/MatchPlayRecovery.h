#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCardSnapshotAuthority.h"
#include "PlayCardResolver.h"

#include "MatchPlayRecovery.generated.h"

UENUM(BlueprintType)
enum class EMatchPlayRecoveryPurpose : uint8
{
	None = 0 UMETA(DisplayName = "None"),
	ConsumedRecovery = 1 UMETA(DisplayName = "Consumed Recovery")
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayRecoveredCardFactEntry
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Core Rules|Match Play|Recovery")
	EInitialTurnOrderPlayer OwnerSide = EInitialTurnOrderPlayer::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Core Rules|Match Play|Recovery")
	FName CardId = NAME_None;
};

/** Bounded latest-event fact. CardUsageState remains the legality truth. */
USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayLastRecoveryFact
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Core Rules|Match Play|Recovery")
	bool bHasRecoveryFact = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Core Rules|Match Play|Recovery")
	int64 SourceAttackSequence = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Core Rules|Match Play|Recovery")
	TArray<FMatchPlayRecoveredCardFactEntry> ReturnedCards;
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayRecoveryCandidate
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Core Rules|Match Play|Recovery")
	EInitialTurnOrderPlayer OwnerSide = EInitialTurnOrderPlayer::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Core Rules|Match Play|Recovery")
	FName CardId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Core Rules|Match Play|Recovery")
	int32 StaminaWeight = 0;
};

enum class EMatchPlayRecoveryCandidateQueryErrorCode : uint8
{
	None,
	InvalidCardUsageState,
	SnapshotQueryFailed,
	GoalkeeperInUsedZone,
	InvalidStaminaWeight
};

struct FMCODEX_API FMatchPlayRecoveryCandidateQueryResult
{
	bool bSuccess = false;
	TArray<FMatchPlayRecoveryCandidate> Candidates;
	EMatchPlayRecoveryCandidateQueryErrorCode ErrorCode =
		EMatchPlayRecoveryCandidateQueryErrorCode::None;
	EInitialTurnOrderPlayer FailingOwnerSide = EInitialTurnOrderPlayer::None;
	FName FailingCardId = NAME_None;
	FString ErrorMessage;
};

class FMCODEX_API FMatchPlayRecoveryCandidateQuery final
{
public:
	static FMatchPlayRecoveryCandidateQueryResult Build(
		const FMatchCardUsageState& CardUsageState,
		const FMatchPlayPerSideCardSnapshotAuthority& CardSnapshotAuthority);
};

enum class EMatchPlayRecoveryProviderErrorCode : uint8
{
	None,
	InvalidPurpose,
	ProviderFailure
};

struct FMCODEX_API FMatchPlayRecoveryProviderResult
{
	bool bSuccess = false;
	TArray<int32> SelectedCandidateIndices;
	EMatchPlayRecoveryProviderErrorCode ErrorCode =
		EMatchPlayRecoveryProviderErrorCode::None;
	FString ErrorMessage;
};

class FMCODEX_API IMatchPlayRecoveryProvider
{
public:
	virtual ~IMatchPlayRecoveryProvider() = default;

	/** One semantic operation: the complete ordered pair succeeds or fails. */
	virtual FMatchPlayRecoveryProviderResult DrawWeightedWithoutReplacement(
		EMatchPlayRecoveryPurpose Purpose,
		const TArray<FMatchPlayRecoveryCandidate>& OrderedCandidates,
		int32 ReturnCount) = 0;
};

enum class EMatchPlayRecoveryProviderValidationErrorCode : uint8
{
	None,
	InvalidRequest,
	ProviderFailure,
	MalformedProviderResult
};

struct FMCODEX_API FMatchPlayRecoveryProviderValidationResult
{
	bool bIsCanonical = false;
	EMatchPlayRecoveryProviderValidationErrorCode ErrorCode =
		EMatchPlayRecoveryProviderValidationErrorCode::None;
	FString ErrorMessage;
};

class FMCODEX_API FMatchPlayRecoveryProviderResultValidator final
{
public:
	static FMatchPlayRecoveryProviderValidationResult Validate(
		EMatchPlayRecoveryPurpose Purpose,
		const TArray<FMatchPlayRecoveryCandidate>& OrderedCandidates,
		int32 ReturnCount,
		const FMatchPlayRecoveryProviderResult& ProviderResult);
};

enum class EMatchPlayRecoveryResolveErrorCode : uint8
{
	None,
	CandidateQueryFailed,
	MissingProvider,
	ProviderFailure,
	MalformedProviderResult,
	MutationFailed
};

struct FMCODEX_API FMatchPlayRecoveryResolveResult
{
	bool bSuccess = false;
	FMatchCardUsageState UpdatedCardUsageState;
	FMatchPlayLastRecoveryFact RecoveryFact;
	FMatchPlayRecoveryCandidateQueryResult CandidateQueryResult;
	FMatchPlayRecoveryProviderResult ProviderResult;
	EMatchPlayRecoveryResolveErrorCode ErrorCode =
		EMatchPlayRecoveryResolveErrorCode::None;
	FString ErrorMessage;
};

class FMCODEX_API FMatchPlayRecoveryResolver final
{
public:
	static FMatchPlayRecoveryResolveResult Resolve(
		const FMatchCardUsageState& CardUsageState,
		const FMatchPlayPerSideCardSnapshotAuthority& CardSnapshotAuthority,
		int64 SourceAttackSequence,
		IMatchPlayRecoveryProvider* Provider);
};
