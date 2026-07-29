#pragma once

#include "CoreMinimal.h"

#include "MatchPlayState.h"

class FMatchPlayCurrentAttackHelperSelectionWriter;
class FMatchPlayHelperAbsenceFinalizer;

class FMatchPlayValidatedHelperPresence final
{
public:
	FMatchPlayValidatedHelperPresence() = delete;
	FMatchPlayValidatedHelperPresence(
		const FMatchPlayValidatedHelperPresence&) = delete;
	FMatchPlayValidatedHelperPresence(
		FMatchPlayValidatedHelperPresence&&) = delete;
	FMatchPlayValidatedHelperPresence& operator=(
		const FMatchPlayValidatedHelperPresence&) = delete;
	FMatchPlayValidatedHelperPresence& operator=(
		FMatchPlayValidatedHelperPresence&&) = delete;

	bool HasHelper() const
	{
		return bHasHelper;
	}

	FName GetHelperCardId() const
	{
		return HelperCardId;
	}

private:
	friend class FMatchPlayCurrentAttackHelperSelectionWriter;
	friend class FMatchPlayHelperAbsenceFinalizer;

	class FSelectedHelperTag final
	{
	private:
		friend class FMatchPlayCurrentAttackHelperSelectionWriter;
		FSelectedHelperTag() = default;
	};

	class FAbsentHelperTag final
	{
	private:
		friend class FMatchPlayHelperAbsenceFinalizer;
		FAbsentHelperTag() = default;
	};

	FMatchPlayValidatedHelperPresence(
		FSelectedHelperTag,
		const FName InHelperCardId)
		: bHasHelper(true)
		, HelperCardId(InHelperCardId)
	{
	}

	explicit FMatchPlayValidatedHelperPresence(FAbsentHelperTag)
		: bHasHelper(false)
		, HelperCardId(NAME_None)
	{
	}

	bool bHasHelper = false;
	FName HelperCardId = NAME_None;
};

class FMatchPlayCurrentAttackHelperFinalization final
{
private:
	friend class FMatchPlayCurrentAttackHelperSelectionWriter;
	friend class FMatchPlayHelperAbsenceFinalizer;

	static void ApplyFinalSelectedAction(
		FMatchPlayState& WorkingState,
		const FMatchPlayValidatedHelperPresence& Presence);
};
