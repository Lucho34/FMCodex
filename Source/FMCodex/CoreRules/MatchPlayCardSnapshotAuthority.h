#pragma once

#include "CoreMinimal.h"

#include "DeckValidator.h"
#include "InitialTurnOrderResolver.h"
#include "PlayerCardRuleSnapshotQuery.h"

#include "MatchPlayCardSnapshotAuthority.generated.h"

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayPerSideCardSnapshotAuthority
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Card Snapshot Authority")
	FPlayerCardRuleSnapshotSet PlayerACardSnapshots;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Card Snapshot Authority")
	FPlayerCardRuleSnapshotSet PlayerBCardSnapshots;
};

UENUM(BlueprintType)
enum class EMatchPlayCardSnapshotAuthorityBuildErrorCode : uint8
{
	None UMETA(DisplayName = "None"),
	DeckValidationFailed UMETA(DisplayName = "Deck Validation Failed"),
	SnapshotValidationFailed UMETA(DisplayName = "Snapshot Validation Failed")
};

struct FMCODEX_API FMatchPlayCardSnapshotAuthorityBuildResult
{
	bool bSuccess = false;
	FMatchPlayPerSideCardSnapshotAuthority CardSnapshotAuthority;
	EMatchPlayCardSnapshotAuthorityBuildErrorCode ErrorCode =
		EMatchPlayCardSnapshotAuthorityBuildErrorCode::None;
	EInitialTurnOrderPlayer FailingPlayerSide =
		EInitialTurnOrderPlayer::None;
	EDeckValidationErrorCode UnderlyingDeckValidationErrorCode =
		EDeckValidationErrorCode::None;
	EPlayerCardRuleSnapshotValidationErrorCode
		UnderlyingPlayerCardRuleSnapshotValidationErrorCode =
			EPlayerCardRuleSnapshotValidationErrorCode::None;
	FString ErrorMessage;
};

class FMCODEX_API FMatchPlayCardSnapshotAuthorityBuilder final
{
public:
	static FMatchPlayCardSnapshotAuthorityBuildResult Build(
		const TArray<FPlayerCardData>& PlayerADeck,
		const TArray<FPlayerCardData>& PlayerBDeck);
};

enum class EMatchPlayCardSnapshotAuthorityQueryErrorCode : uint8
{
	None,
	InvalidPlayerSide,
	InvalidCardId,
	SnapshotValidationFailed,
	SnapshotNotFound
};

struct FMCODEX_API FMatchPlayCardSnapshotAuthorityQueryResult
{
	bool bSuccess = false;
	EMatchPlayCardSnapshotAuthorityQueryErrorCode ErrorCode =
		EMatchPlayCardSnapshotAuthorityQueryErrorCode::None;
	EInitialTurnOrderPlayer PlayerSide = EInitialTurnOrderPlayer::None;
	FName CardId = NAME_None;
	FPlayerCardRuleSnapshot Snapshot;
	FPlayerCardRuleSnapshotQueryResult UnderlyingQueryResult;
	FString ErrorMessage;
};

class FMCODEX_API FMatchPlayCardSnapshotAuthorityQuery final
{
public:
	static FMatchPlayCardSnapshotAuthorityQueryResult
		FindByPlayerSideAndCardId(
			const FMatchPlayPerSideCardSnapshotAuthority& Authority,
			EInitialTurnOrderPlayer PlayerSide,
			FName CardId);
};
