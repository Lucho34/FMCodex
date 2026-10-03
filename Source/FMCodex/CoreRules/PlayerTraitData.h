#pragma once
#include "CoreMinimal.h"
#include "PlayerTraitData.generated.h"

UENUM(BlueprintType)
enum class EPlayerTraitRank : uint8 { None, S, A, B };

USTRUCT(BlueprintType)
struct FMCODEX_API FPlayerRankedTrait
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Rules|Traits")
	FName TraitId = NAME_None;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Rules|Traits")
	EPlayerTraitRank Rank = EPlayerTraitRank::None;
	bool operator==(const FPlayerRankedTrait& Other) const { return TraitId == Other.TraitId && Rank == Other.Rank; }
};

// Fixed storage taxonomy only. Activation and bonus evaluation are deliberately absent.
inline bool IsKnownRankedPlayerTrait(const FName Id)
{
	static const TSet<FName> Ids = {
		TEXT("Trait.LongShotCarrier"),
		TEXT("Trait.CutInsideCarrier"),
		TEXT("Trait.CrossCarrier"),
		TEXT("Trait.CrossHighRunner"),
		TEXT("Trait.CrossLowRunner"),
		TEXT("Trait.ThroughBallCarrier"),
		TEXT("Trait.ThroughBallBehindRunner"),
		TEXT("Trait.ThroughBallFeetRunner"),
		TEXT("Trait.CornerHighThreat"),
		TEXT("Trait.CornerLowThreat"),
		TEXT("Trait.NearFreeKickTaker"),
		TEXT("Trait.LongFreeKickTaker"),
		TEXT("Trait.PenaltyTaker"),
		TEXT("Trait.LongShotBlocker"),
		TEXT("Trait.CutInsideStopper"),
		TEXT("Trait.CrossMarkerBlocker"),
		TEXT("Trait.CrossHighHelperDefense"),
		TEXT("Trait.CrossLowHelperDefense"),
		TEXT("Trait.ThroughBallMarkerDefense"),
		TEXT("Trait.ThroughBallFeetHelperDefense"),
		TEXT("Trait.ThroughBallBehindHelperDefense"),
		TEXT("Trait.CornerHighDefense"),
		TEXT("Trait.CornerLowDefense")
	};
	return Ids.Contains(Id);
}

inline bool IsKnownBinaryPlayerTrait(const FName Id)
{
	return Id == FName(TEXT("Trait.ThroughBallAntiRunner"));
}

inline bool ValidatePassivePlayerTraits(const TArray<FPlayerRankedTrait>& Ranked,
	const TArray<FName>& Binary, const bool bGoalkeeper)
{
	if (Ranked.Num() > 23 || Binary.Num() > 1 || (bGoalkeeper && (!Ranked.IsEmpty() || !Binary.IsEmpty()))) return false;
	TSet<FName> Seen;
	for (const auto& Trait : Ranked)
	{
		if (!IsKnownRankedPlayerTrait(Trait.TraitId) || Seen.Contains(Trait.TraitId)
			|| (Trait.Rank != EPlayerTraitRank::S && Trait.Rank != EPlayerTraitRank::A && Trait.Rank != EPlayerTraitRank::B)) return false;
		Seen.Add(Trait.TraitId);
	}
	return Binary.IsEmpty() || IsKnownBinaryPlayerTrait(Binary[0]);
}
