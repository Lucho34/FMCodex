#pragma once

#include "CoreMinimal.h"

enum class EMatchPlayPassControlTerminalSource : uint8
{
	None,
	PassAdvanceFormulaGoal,
	PassAdvanceFormulaMiss,
	DribbleAdvanceFormulaGoal,
	DribbleAdvanceFormulaMiss,
	RunAdvanceFormulaGoal,
	RunAdvanceFormulaMiss
};

class FMatchPlayCurrentAttackApplyPassControlTerminalResolutionOrchestrator;
class FMatchPlayCurrentAttackCompletion;

class FMCODEX_API FMatchPlayPassControlResolutionTerminalCapability final
{
public:
	FMatchPlayPassControlResolutionTerminalCapability() = delete;

	int64 GetAttackSequence() const
	{
		return AttackSequence;
	}

	EMatchPlayPassControlTerminalSource GetSource() const
	{
		return Source;
	}

	bool IsGoal() const
	{
		return bIsGoal;
	}

private:
	friend class
		FMatchPlayCurrentAttackApplyPassControlTerminalResolutionOrchestrator;
	friend class FMatchPlayCurrentAttackCompletion;

	class FAuthoritativeTerminalIssuerTag final
	{
	private:
		friend class
			FMatchPlayCurrentAttackApplyPassControlTerminalResolutionOrchestrator;
		FAuthoritativeTerminalIssuerTag() = default;
	};

	FMatchPlayPassControlResolutionTerminalCapability(
		FAuthoritativeTerminalIssuerTag,
		int64 InAttackSequence,
		EMatchPlayPassControlTerminalSource InSource,
		bool bInIsGoal)
		: AttackSequence(InAttackSequence)
		, Source(InSource)
		, bIsGoal(bInIsGoal)
	{
	}

	FMatchPlayPassControlResolutionTerminalCapability(
		const FMatchPlayPassControlResolutionTerminalCapability&) = delete;
	FMatchPlayPassControlResolutionTerminalCapability(
		FMatchPlayPassControlResolutionTerminalCapability&&) = delete;
	FMatchPlayPassControlResolutionTerminalCapability& operator=(
		const FMatchPlayPassControlResolutionTerminalCapability&) = delete;
	FMatchPlayPassControlResolutionTerminalCapability& operator=(
		FMatchPlayPassControlResolutionTerminalCapability&&) = delete;

	int64 AttackSequence = 0;
	EMatchPlayPassControlTerminalSource Source =
		EMatchPlayPassControlTerminalSource::None;
	bool bIsGoal = false;
};
