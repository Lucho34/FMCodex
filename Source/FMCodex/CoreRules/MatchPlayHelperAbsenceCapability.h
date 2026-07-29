#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCurrentAttackHelperSelectionAvailability.h"

enum class EMatchPlayHelperAbsenceReason : uint8;
enum class EMatchPlayHelperAbsenceSource : uint8;

class FMatchPlayHelperAbsenceFinalizer;
class FMatchPlayResolveNoLegalHelper;
class FMatchPlayHelperDecline;

class FMatchPlayHelperAbsenceCapability final
{
public:
	FMatchPlayHelperAbsenceCapability() = delete;
	FMatchPlayHelperAbsenceCapability(
		const FMatchPlayHelperAbsenceCapability&) = delete;
	FMatchPlayHelperAbsenceCapability(
		FMatchPlayHelperAbsenceCapability&&) = delete;
	FMatchPlayHelperAbsenceCapability& operator=(
		const FMatchPlayHelperAbsenceCapability&) = delete;
	FMatchPlayHelperAbsenceCapability& operator=(
		FMatchPlayHelperAbsenceCapability&&) = delete;

	int64 GetAttackSequence() const
	{
		return AttackSequence;
	}

	EMatchPlayHelperAbsenceReason GetReason() const
	{
		return Reason;
	}

	EMatchPlayHelperAbsenceSource GetSource() const
	{
		return Source;
	}

	const FMatchPlayCurrentAttackHelperSelectionAvailabilityResult&
	GetAvailabilityResult() const
	{
		return AvailabilityResult;
	}

private:
	friend class FMatchPlayHelperAbsenceFinalizer;
	friend class FMatchPlayResolveNoLegalHelper;
	friend class FMatchPlayHelperDecline;

	class FResolveNoLegalHelperIssuerTag final
	{
	private:
		friend class FMatchPlayResolveNoLegalHelper;
		FResolveNoLegalHelperIssuerTag() = default;
	};

	class FHelperDeclineIssuerTag final
	{
	private:
		friend class FMatchPlayHelperDecline;
		FHelperDeclineIssuerTag() = default;
	};

	FMatchPlayHelperAbsenceCapability(
		FResolveNoLegalHelperIssuerTag,
		int64 InAttackSequence,
		const FMatchPlayCurrentAttackHelperSelectionAvailabilityResult&
			InAvailabilityResult);

	FMatchPlayHelperAbsenceCapability(
		FHelperDeclineIssuerTag,
		int64 InAttackSequence,
		const FMatchPlayCurrentAttackHelperSelectionAvailabilityResult&
			InAvailabilityResult);

	int64 AttackSequence = 0;
	EMatchPlayHelperAbsenceReason Reason;
	EMatchPlayHelperAbsenceSource Source;
	FMatchPlayCurrentAttackHelperSelectionAvailabilityResult
		AvailabilityResult;
};
