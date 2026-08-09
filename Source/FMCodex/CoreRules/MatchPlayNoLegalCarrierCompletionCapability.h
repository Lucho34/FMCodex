#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCurrentAttackCarrierSelectionAvailability.h"

class FMatchPlayCurrentAttackCompletion;
class FMatchPlayResolveNoLegalCarrier;

class FMCODEX_API FMatchPlayNoLegalCarrierCompletionCapability final
{
public:
	FMatchPlayNoLegalCarrierCompletionCapability() = delete;

	int64 GetAttackSequence() const
	{
		return AttackSequence;
	}

	const FMatchPlayCurrentAttackCarrierSelectionAvailabilityResult&
	GetAuthorityResult() const
	{
		return AuthorityResult;
	}

private:
	friend class FMatchPlayCurrentAttackCompletion;
	friend class FMatchPlayResolveNoLegalCarrier;

	class FResolveNoLegalCarrierIssuerTag final
	{
	private:
		friend class FMatchPlayResolveNoLegalCarrier;
		FResolveNoLegalCarrierIssuerTag() = default;
	};

	FMatchPlayNoLegalCarrierCompletionCapability(
		FResolveNoLegalCarrierIssuerTag,
		const int64 InAttackSequence,
		const FMatchPlayCurrentAttackCarrierSelectionAvailabilityResult&
			InAuthorityResult)
		: AttackSequence(InAttackSequence)
		, AuthorityResult(InAuthorityResult)
	{
	}

	FMatchPlayNoLegalCarrierCompletionCapability(
		const FMatchPlayNoLegalCarrierCompletionCapability&) = delete;
	FMatchPlayNoLegalCarrierCompletionCapability(
		FMatchPlayNoLegalCarrierCompletionCapability&&) = delete;
	FMatchPlayNoLegalCarrierCompletionCapability& operator=(
		const FMatchPlayNoLegalCarrierCompletionCapability&) = delete;
	FMatchPlayNoLegalCarrierCompletionCapability& operator=(
		FMatchPlayNoLegalCarrierCompletionCapability&&) = delete;

	int64 AttackSequence = 0;
	FMatchPlayCurrentAttackCarrierSelectionAvailabilityResult AuthorityResult;
};
