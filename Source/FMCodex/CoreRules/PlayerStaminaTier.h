#pragma once

#include "CoreMinimal.h"
#include "PlayerStaminaTier.generated.h"

UENUM(BlueprintType)
enum class EPlayerStaminaTier : uint8
{
	None,
	S,
	A,
	B
};

// Shared by Recovery weights and actual-participant Formula tie inputs.
inline int32 PlayerStaminaGameplayValue(const EPlayerStaminaTier Tier)
{
	switch (Tier)
	{
	case EPlayerStaminaTier::S: return 5;
	case EPlayerStaminaTier::A: return 3;
	case EPlayerStaminaTier::B: return 1;
	default: return 0;
	}
}

inline const TCHAR* PlayerStaminaTierLabel(const EPlayerStaminaTier Tier)
{
	switch (Tier)
	{
	case EPlayerStaminaTier::S: return TEXT("S");
	case EPlayerStaminaTier::A: return TEXT("A");
	case EPlayerStaminaTier::B: return TEXT("B");
	default: return TEXT("");
	}
}
