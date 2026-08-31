#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCurrentAttackPostRouteRollTypes.generated.h"

UENUM(BlueprintType)
enum class EMatchPlayCurrentAttackPostRouteRollPurpose : uint8
{
	None = 0 UMETA(DisplayName = "None"),
	PrimaryAttack = 1 UMETA(DisplayName = "Primary Attack"),
	PrimaryDefense = 2 UMETA(DisplayName = "Primary Defense"),
	PairedAttackA = 3 UMETA(DisplayName = "Paired Attack A"),
	PairedAttackB = 4 UMETA(DisplayName = "Paired Attack B"),
	BehindDefenseP2Defense = 5
		UMETA(DisplayName = "Behind Defense P2 Defense"),
	OneOnOneChipShotAttack = 6
		UMETA(DisplayName = "One-on-One Chip Shot Attack"),
	OneOnOneDirectShotAttack = 7
		UMETA(DisplayName = "One-on-One Direct Shot Attack"),
	OneOnOneDirectShotDefense = 8
		UMETA(DisplayName = "One-on-One Direct Shot Defense"),
	ShortFreeKickDirectAttack = 9
		UMETA(DisplayName = "Short Free Kick Direct Attack"),
	ShortFreeKickDirectDefense = 10
		UMETA(DisplayName = "Short Free Kick Direct Defense"),
	ShortFreeKickAngledA = 11
		UMETA(DisplayName = "Short Free Kick Angled A"),
	ShortFreeKickAngledB = 12
		UMETA(DisplayName = "Short Free Kick Angled B"),
	LongFreeKickDirectAttack = 13
		UMETA(DisplayName = "Long Free Kick Direct Attack"),
	LongFreeKickDirectDefense = 14
		UMETA(DisplayName = "Long Free Kick Direct Defense"),
	LongFreeKickPowerA = 15
		UMETA(DisplayName = "Long Free Kick Power A"),
	LongFreeKickPowerB = 16
		UMETA(DisplayName = "Long Free Kick Power B"),
	PenaltyDirectAttack = 17
		UMETA(DisplayName = "Penalty Direct Attack"),
	PenaltyDirectDefense = 18
		UMETA(DisplayName = "Penalty Direct Defense"),
	PenaltyPanenka = 19 UMETA(DisplayName = "Penalty Panenka")
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayCurrentAttackPostRouteRollRecord
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Session")
	EMatchPlayCurrentAttackPostRouteRollPurpose Purpose =
		EMatchPlayCurrentAttackPostRouteRollPurpose::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Session")
	int32 RawD6 = 0;
};

UENUM(BlueprintType)
enum class EMatchPlayCurrentAttackPostRouteRollPhase : uint8
{
	None = 0 UMETA(DisplayName = "None"),
	PrimaryBranch = 1 UMETA(DisplayName = "Primary Branch"),
	BehindDefenseP2 = 2 UMETA(DisplayName = "Behind Defense P2"),
	OneOnOneChipShot = 3 UMETA(DisplayName = "One-on-One Chip Shot"),
	OneOnOneDirectShot = 4 UMETA(DisplayName = "One-on-One Direct Shot")
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayCurrentAttackPostRouteRollProgress
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Session")
	EMatchPlayCurrentAttackPostRouteRollPhase Phase =
		EMatchPlayCurrentAttackPostRouteRollPhase::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Session")
	TArray<FMatchPlayCurrentAttackPostRouteRollRecord> RollRecords;
};
