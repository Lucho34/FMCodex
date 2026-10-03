#pragma once
#include "CoreMinimal.h"
#include "PlayerCardRuleSnapshot.h"
#include "PlayerTraitFormula.generated.h"

UENUM(BlueprintType)
enum class EMatchPlayResolutionParticipantRole : uint8
{
	None = 0 UMETA(DisplayName = "None"),
	Carrier = 1 UMETA(DisplayName = "Carrier"),
	Runner = 2 UMETA(DisplayName = "Runner"),
	Marker = 3 UMETA(DisplayName = "Marker"),
	Helper = 4 UMETA(DisplayName = "Helper"),
	Goalkeeper = 5 UMETA(DisplayName = "Goalkeeper"),
	Taker = 6 UMETA(DisplayName = "Taker")
};

UENUM(BlueprintType)
enum class EMatchPlayResolutionFormulaAttribute : uint8
{
	None = 0 UMETA(DisplayName = "None"),
	Shooting = 1 UMETA(DisplayName = "Shooting"),
	Control = 2 UMETA(DisplayName = "Control"),
	Passing = 3 UMETA(DisplayName = "Passing"),
	Defense = 6 UMETA(DisplayName = "Defense"),
	Speed = 7 UMETA(DisplayName = "Speed"),
	Strength = 8 UMETA(DisplayName = "Strength"),
	GoalkeeperHandling = 10 UMETA(DisplayName = "Goalkeeper Handling"),
	GoalkeeperPositioning = 11 UMETA(DisplayName = "Goalkeeper Positioning"),
	GoalkeeperReflex = 12 UMETA(DisplayName = "Goalkeeper Reflex"),
	GoalkeeperAerial = 13 UMETA(DisplayName = "Goalkeeper Aerial"),
	GoalkeeperOneOnOne = 14 UMETA(DisplayName = "Goalkeeper One-on-One")
};

// Formula-local fact; never written back to a player snapshot. Context comes from
// the authoritative plan, not from a player intent or presentation catalog.
USTRUCT(BlueprintType)
struct FMCODEX_API FPlayerTraitFormulaOperand
{
 GENERATED_BODY()
 UPROPERTY() bool bValid = false;
 UPROPERTY() FName CardId = NAME_None;
 UPROPERTY() EMatchPlayResolutionParticipantRole Role = EMatchPlayResolutionParticipantRole::None;
 UPROPERTY() EMatchPlayResolutionFormulaAttribute Attribute = EMatchPlayResolutionFormulaAttribute::None;
 UPROPERTY() int32 BaseValue = 0;
 UPROPERTY() FName TraitId = NAME_None;
 UPROPERTY() EPlayerTraitRank Rank = EPlayerTraitRank::None;
 UPROPERTY() int32 Bonus = 0;
 UPROPERTY() int32 EffectiveValue = 0;
};

struct FMCODEX_API FPlayerTraitTakerFormula
{
 bool bValid = false;
 TArray<FPlayerTraitFormulaOperand> Operands;
 int32 SelectedEffectiveValue = 0;
};

class FMCODEX_API FPlayerTraitFormula final
{
public:
 // Value-only role/attribute types; Formula plans do not depend on match state.
 using Role = EMatchPlayResolutionParticipantRole;
 using Attribute = EMatchPlayResolutionFormulaAttribute;
 static int32 RankBonus(EPlayerTraitRank Rank);
 static FPlayerTraitFormulaOperand Resolve(const FPlayerCardRuleSnapshot& Participant,
  FName FormulaContext, EMatchPlayResolutionParticipantRole Role,
  EMatchPlayResolutionFormulaAttribute Attribute);
 // Near/Penalty resolve BOTH candidates before selection; Long resolves Shooting.
 static FPlayerTraitTakerFormula ResolveTaker(const FPlayerCardRuleSnapshot& Taker, FName FormulaContext);
 static FText DisplayName(FName TraitId);
};
