#pragma once

#include "CoreMinimal.h"

enum class EMatchPlayShotTerminalSource : uint8
{
	None,
	LongShotDirectShotImmediateMiss,
	LongShotDirectShotFormulaGoal,
	LongShotDirectShotFormulaMiss,
	CutInsideShotDirectShotImmediateMiss,
	CutInsideShotDirectShotFormulaGoal,
	CutInsideShotDirectShotFormulaMiss,
	LongShotDeadCornerGoal,
	LongShotDeadCornerMiss,
	CutInsideShotDeadCornerGoal,
	CutInsideShotDeadCornerMiss
};

class FMatchPlayCurrentAttackApplyShotTerminalResolutionOrchestrator;
class FMatchPlayCurrentAttackCompletion;

class FMCODEX_API FMatchPlayShotResolutionTerminalCapability final
{
public:
	FMatchPlayShotResolutionTerminalCapability() = delete;

	int64 GetAttackSequence() const
	{
		return AttackSequence;
	}

	EMatchPlayShotTerminalSource GetSource() const
	{
		return Source;
	}

	bool IsGoal() const
	{
		return bIsGoal;
	}

private:
	friend class FMatchPlayCurrentAttackApplyShotTerminalResolutionOrchestrator;
	friend class FMatchPlayCurrentAttackCompletion;

	class FAuthoritativeTerminalIssuerTag final
	{
	private:
		friend class
			FMatchPlayCurrentAttackApplyShotTerminalResolutionOrchestrator;
		FAuthoritativeTerminalIssuerTag() = default;
	};

	FMatchPlayShotResolutionTerminalCapability(
		FAuthoritativeTerminalIssuerTag,
		int64 InAttackSequence,
		EMatchPlayShotTerminalSource InSource,
		bool bInIsGoal)
		: AttackSequence(InAttackSequence)
		, Source(InSource)
		, bIsGoal(bInIsGoal)
	{
	}

	FMatchPlayShotResolutionTerminalCapability(
		const FMatchPlayShotResolutionTerminalCapability&) = delete;
	FMatchPlayShotResolutionTerminalCapability(
		FMatchPlayShotResolutionTerminalCapability&&) = delete;
	FMatchPlayShotResolutionTerminalCapability& operator=(
		const FMatchPlayShotResolutionTerminalCapability&) = delete;
	FMatchPlayShotResolutionTerminalCapability& operator=(
		FMatchPlayShotResolutionTerminalCapability&&) = delete;

	int64 AttackSequence = 0;
	EMatchPlayShotTerminalSource Source = EMatchPlayShotTerminalSource::None;
	bool bIsGoal = false;
};
