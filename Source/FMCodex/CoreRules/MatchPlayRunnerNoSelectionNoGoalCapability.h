#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCurrentAttackRunnerSelectionAvailability.h"

#include "MatchPlayRunnerNoSelectionNoGoalCapability.generated.h"

UENUM(BlueprintType)
enum class EMatchPlayRunnerNoSelectionNoGoalReason : uint8
{
	None UMETA(DisplayName = "None"),
	NoLegalRunner UMETA(DisplayName = "No Legal Runner"),
	RunnerDeclined UMETA(DisplayName = "Runner Declined")
};

UENUM(BlueprintType)
enum class EMatchPlayRunnerNoSelectionNoGoalSource : uint8
{
	None UMETA(DisplayName = "None"),
	ResolveNoLegalRunner UMETA(DisplayName = "Resolve No Legal Runner"),
	RunnerDecline UMETA(DisplayName = "Runner Decline")
};

class FMatchPlayCurrentAttackCompletion;
class FMatchPlayResolveNoLegalRunner;
class FMatchPlayRunnerDecline;

class FMCODEX_API FMatchPlayRunnerNoSelectionNoGoalCapability final
{
public:
	FMatchPlayRunnerNoSelectionNoGoalCapability() = delete;

	int64 GetAttackSequence() const
	{
		return AttackSequence;
	}

	EMatchPlayRunnerNoSelectionNoGoalReason GetReason() const
	{
		return Reason;
	}

	EMatchPlayRunnerNoSelectionNoGoalSource GetSource() const
	{
		return Source;
	}

	const FMatchPlayCurrentAttackRunnerSelectionAvailabilityResult&
	GetAuthorityResult() const
	{
		return AuthorityResult;
	}

private:
	friend class FMatchPlayCurrentAttackCompletion;
	friend class FMatchPlayResolveNoLegalRunner;
	friend class FMatchPlayRunnerDecline;

	class FResolveNoLegalRunnerIssuerTag final
	{
	private:
		friend class FMatchPlayResolveNoLegalRunner;
		FResolveNoLegalRunnerIssuerTag() = default;
	};

	class FRunnerDeclineIssuerTag final
	{
	private:
		friend class FMatchPlayRunnerDecline;
		FRunnerDeclineIssuerTag() = default;
	};

	FMatchPlayRunnerNoSelectionNoGoalCapability(
		FResolveNoLegalRunnerIssuerTag,
		const int64 InAttackSequence,
		const FMatchPlayCurrentAttackRunnerSelectionAvailabilityResult&
			InAuthorityResult)
		: AttackSequence(InAttackSequence)
		, Reason(
			EMatchPlayRunnerNoSelectionNoGoalReason::NoLegalRunner)
		, Source(
			EMatchPlayRunnerNoSelectionNoGoalSource
				::ResolveNoLegalRunner)
		, AuthorityResult(InAuthorityResult)
	{
	}

	FMatchPlayRunnerNoSelectionNoGoalCapability(
		FRunnerDeclineIssuerTag,
		const int64 InAttackSequence,
		const FMatchPlayCurrentAttackRunnerSelectionAvailabilityResult&
			InAuthorityResult)
		: AttackSequence(InAttackSequence)
		, Reason(
			EMatchPlayRunnerNoSelectionNoGoalReason::RunnerDeclined)
		, Source(
			EMatchPlayRunnerNoSelectionNoGoalSource::RunnerDecline)
		, AuthorityResult(InAuthorityResult)
	{
	}

	FMatchPlayRunnerNoSelectionNoGoalCapability(
		const FMatchPlayRunnerNoSelectionNoGoalCapability&) = delete;
	FMatchPlayRunnerNoSelectionNoGoalCapability(
		FMatchPlayRunnerNoSelectionNoGoalCapability&&) = delete;
	FMatchPlayRunnerNoSelectionNoGoalCapability& operator=(
		const FMatchPlayRunnerNoSelectionNoGoalCapability&) = delete;
	FMatchPlayRunnerNoSelectionNoGoalCapability& operator=(
		FMatchPlayRunnerNoSelectionNoGoalCapability&&) = delete;

	int64 AttackSequence = 0;
	EMatchPlayRunnerNoSelectionNoGoalReason Reason =
		EMatchPlayRunnerNoSelectionNoGoalReason::None;
	EMatchPlayRunnerNoSelectionNoGoalSource Source =
		EMatchPlayRunnerNoSelectionNoGoalSource::None;
	FMatchPlayCurrentAttackRunnerSelectionAvailabilityResult
		AuthorityResult;
};
