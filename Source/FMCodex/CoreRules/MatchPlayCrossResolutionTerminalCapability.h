#pragma once

#include "CoreMinimal.h"

enum class EMatchPlayCrossTerminalSource : uint8
{
	None,
	HighFormulaGoal,
	HighFormulaMiss,
	LowFormulaGoal,
	LowFormulaMiss
};

class FMatchPlayCurrentAttackApplyCrossTerminalResolutionOrchestrator;
class FMatchPlayCurrentAttackCompletion;

class FMCODEX_API FMatchPlayCrossResolutionTerminalCapability final
{
public:
	FMatchPlayCrossResolutionTerminalCapability() = delete;

	int64 GetAttackSequence() const
	{
		return AttackSequence;
	}

	EMatchPlayCrossTerminalSource GetSource() const
	{
		return Source;
	}

	bool IsGoal() const
	{
		return bIsGoal;
	}

private:
	friend class FMatchPlayCurrentAttackApplyCrossTerminalResolutionOrchestrator;
	friend class FMatchPlayCurrentAttackCompletion;

	class FAuthoritativeTerminalIssuerTag final
	{
	private:
		friend class
			FMatchPlayCurrentAttackApplyCrossTerminalResolutionOrchestrator;
		FAuthoritativeTerminalIssuerTag() = default;
	};

	FMatchPlayCrossResolutionTerminalCapability(
		FAuthoritativeTerminalIssuerTag,
		int64 InAttackSequence,
		EMatchPlayCrossTerminalSource InSource,
		bool bInIsGoal)
		: AttackSequence(InAttackSequence)
		, Source(InSource)
		, bIsGoal(bInIsGoal)
	{
	}

	FMatchPlayCrossResolutionTerminalCapability(
		const FMatchPlayCrossResolutionTerminalCapability&) = delete;
	FMatchPlayCrossResolutionTerminalCapability(
		FMatchPlayCrossResolutionTerminalCapability&&) = delete;
	FMatchPlayCrossResolutionTerminalCapability& operator=(
		const FMatchPlayCrossResolutionTerminalCapability&) = delete;
	FMatchPlayCrossResolutionTerminalCapability& operator=(
		FMatchPlayCrossResolutionTerminalCapability&&) = delete;

	int64 AttackSequence = 0;
	EMatchPlayCrossTerminalSource Source = EMatchPlayCrossTerminalSource::None;
	bool bIsGoal = false;
};
