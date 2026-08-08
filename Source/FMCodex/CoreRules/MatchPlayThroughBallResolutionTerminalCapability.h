#pragma once

#include "CoreMinimal.h"

enum class EMatchPlayThroughBallTerminalSource : uint8
{
	None,
	FeetFormulaGoal,
	FeetFormulaMiss,
	AntiOffsideOffside,
	AntiOffsideOneOnOneGoal,
	AntiOffsideOneOnOneMiss,
	BehindDefenseOutOfPlay,
	BehindDefenseDefenderStoppedAttack,
	BehindDefenseP2Offside,
	BehindDefenseOneOnOneGoal,
	BehindDefenseOneOnOneMiss
};

class FMatchPlayCurrentAttackApplyThroughBallTerminalResolutionOrchestrator;
class FMatchPlayCurrentAttackCompletion;

class FMCODEX_API FMatchPlayThroughBallResolutionTerminalCapability final
{
public:
	FMatchPlayThroughBallResolutionTerminalCapability() = delete;

	int64 GetAttackSequence() const
	{
		return AttackSequence;
	}

	EMatchPlayThroughBallTerminalSource GetSource() const
	{
		return Source;
	}

	bool IsGoal() const
	{
		return bIsGoal;
	}

private:
	friend class
		FMatchPlayCurrentAttackApplyThroughBallTerminalResolutionOrchestrator;
	friend class FMatchPlayCurrentAttackCompletion;

	class FAuthoritativeTerminalIssuerTag final
	{
	private:
		friend class
			FMatchPlayCurrentAttackApplyThroughBallTerminalResolutionOrchestrator;
		FAuthoritativeTerminalIssuerTag() = default;
	};

	FMatchPlayThroughBallResolutionTerminalCapability(
		FAuthoritativeTerminalIssuerTag,
		const int64 InAttackSequence,
		const EMatchPlayThroughBallTerminalSource InSource,
		const bool bInIsGoal)
		: AttackSequence(InAttackSequence)
		, Source(InSource)
		, bIsGoal(bInIsGoal)
	{
	}

	FMatchPlayThroughBallResolutionTerminalCapability(
		const FMatchPlayThroughBallResolutionTerminalCapability&) = delete;
	FMatchPlayThroughBallResolutionTerminalCapability(
		FMatchPlayThroughBallResolutionTerminalCapability&&) = delete;
	FMatchPlayThroughBallResolutionTerminalCapability& operator=(
		const FMatchPlayThroughBallResolutionTerminalCapability&) = delete;
	FMatchPlayThroughBallResolutionTerminalCapability& operator=(
		FMatchPlayThroughBallResolutionTerminalCapability&&) = delete;

	int64 AttackSequence = 0;
	EMatchPlayThroughBallTerminalSource Source =
		EMatchPlayThroughBallTerminalSource::None;
	bool bIsGoal = false;
};
