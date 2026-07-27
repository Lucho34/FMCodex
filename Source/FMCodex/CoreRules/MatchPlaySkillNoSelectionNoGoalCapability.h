#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCurrentAttackSkillSelectionAvailability.h"

#include "MatchPlaySkillNoSelectionNoGoalCapability.generated.h"

UENUM(BlueprintType)
enum class EMatchPlaySkillNoSelectionNoGoalReason : uint8
{
	None UMETA(DisplayName = "None"),
	NoLegalSkill UMETA(DisplayName = "No Legal Skill"),
	SkillDeclined UMETA(DisplayName = "Skill Declined")
};

UENUM(BlueprintType)
enum class EMatchPlaySkillNoSelectionNoGoalSource : uint8
{
	None UMETA(DisplayName = "None"),
	ResolveNoLegalSkill UMETA(DisplayName = "Resolve No Legal Skill"),
	DeclineSkill UMETA(DisplayName = "Decline Skill")
};

class FMatchPlayCurrentAttackCompletion;
class FMatchPlayResolveNoLegalSkill;
class FMatchPlaySkillDecline;

class FMCODEX_API FMatchPlaySkillNoSelectionNoGoalCapability final
{
public:
	FMatchPlaySkillNoSelectionNoGoalCapability() = delete;

	int64 GetAttackSequence() const
	{
		return AttackSequence;
	}

	EMatchPlaySkillNoSelectionNoGoalReason GetReason() const
	{
		return Reason;
	}

	EMatchPlaySkillNoSelectionNoGoalSource GetSource() const
	{
		return Source;
	}

	const FMatchPlayCurrentAttackSkillSelectionAvailabilityResult&
	GetAuthorityResult() const
	{
		return AuthorityResult;
	}

private:
	friend class FMatchPlayCurrentAttackCompletion;
	friend class FMatchPlayResolveNoLegalSkill;
	friend class FMatchPlaySkillDecline;

	class FResolveNoLegalSkillIssuerTag final
	{
	private:
		friend class FMatchPlayResolveNoLegalSkill;
		FResolveNoLegalSkillIssuerTag() = default;
	};

	class FDeclineSkillIssuerTag final
	{
	private:
		friend class FMatchPlaySkillDecline;
		FDeclineSkillIssuerTag() = default;
	};

	FMatchPlaySkillNoSelectionNoGoalCapability(
		FResolveNoLegalSkillIssuerTag,
		const int64 InAttackSequence,
		const FMatchPlayCurrentAttackSkillSelectionAvailabilityResult&
			InAuthorityResult)
		: AttackSequence(InAttackSequence)
		, Reason(
			EMatchPlaySkillNoSelectionNoGoalReason::NoLegalSkill)
		, Source(
			EMatchPlaySkillNoSelectionNoGoalSource
				::ResolveNoLegalSkill)
		, AuthorityResult(InAuthorityResult)
	{
	}

	FMatchPlaySkillNoSelectionNoGoalCapability(
		FDeclineSkillIssuerTag,
		const int64 InAttackSequence,
		const FMatchPlayCurrentAttackSkillSelectionAvailabilityResult&
			InAuthorityResult)
		: AttackSequence(InAttackSequence)
		, Reason(
			EMatchPlaySkillNoSelectionNoGoalReason::SkillDeclined)
		, Source(
			EMatchPlaySkillNoSelectionNoGoalSource::DeclineSkill)
		, AuthorityResult(InAuthorityResult)
	{
	}

	FMatchPlaySkillNoSelectionNoGoalCapability(
		const FMatchPlaySkillNoSelectionNoGoalCapability&) = delete;
	FMatchPlaySkillNoSelectionNoGoalCapability(
		FMatchPlaySkillNoSelectionNoGoalCapability&&) = delete;
	FMatchPlaySkillNoSelectionNoGoalCapability& operator=(
		const FMatchPlaySkillNoSelectionNoGoalCapability&) = delete;
	FMatchPlaySkillNoSelectionNoGoalCapability& operator=(
		FMatchPlaySkillNoSelectionNoGoalCapability&&) = delete;

	int64 AttackSequence = 0;
	EMatchPlaySkillNoSelectionNoGoalReason Reason =
		EMatchPlaySkillNoSelectionNoGoalReason::None;
	EMatchPlaySkillNoSelectionNoGoalSource Source =
		EMatchPlaySkillNoSelectionNoGoalSource::None;
	FMatchPlayCurrentAttackSkillSelectionAvailabilityResult
		AuthorityResult;
};
