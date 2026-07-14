#include "ThroughBallParticipantEligibilityQuery.h"

namespace ThroughBallParticipantEligibilityQuery
{
	const FName SelectedSkillIdField(TEXT("SelectedSkillId"));
	const FName CurrentActionPointField(TEXT("CurrentActionPoint"));
	const FName AttackingOwnerIdField(TEXT("AttackingOwnerId"));
	const FName DefendingOwnerIdField(TEXT("DefendingOwnerId"));
	const FName CarrierSnapshotField(TEXT("CarrierSnapshot"));
	const FName RunnerSnapshotField(TEXT("RunnerSnapshot"));
	const FName MarkerSnapshotField(TEXT("MarkerSnapshot"));
	const FName HelperSnapshotField(TEXT("HelperSnapshot"));
	const FName RunnerForwardAreaField(
		TEXT("bIsRunnerInAttackingForwardArea"));

	void SetFailure(
		FThroughBallParticipantEligibilityQueryResult& Result,
		const EThroughBallParticipantEligibilityQueryErrorCode ErrorCode,
		const FString& ErrorMessage,
		const FName InvalidField)
	{
		Result.ErrorCode = ErrorCode;
		Result.ErrorMessage = ErrorMessage;
		Result.InvalidField = InvalidField;
	}

	FPlayerCardRuleSnapshotValidationResult ValidateSnapshot(
		const FPlayerCardRuleSnapshot& Snapshot)
	{
		FPlayerCardRuleSnapshotSet SnapshotSet;
		SnapshotSet.Cards.Add(Snapshot);
		return FPlayerCardRuleSnapshotValidator::Validate(SnapshotSet);
	}
}

FThroughBallParticipantEligibilityQueryResult
FThroughBallParticipantEligibilityQuery::Evaluate(
	const FSkillRuleSnapshotSet& SkillRules,
	const FThroughBallParticipantEligibilityQueryInput& Input)
{
	using namespace ThroughBallParticipantEligibilityQuery;

	FThroughBallParticipantEligibilityQueryResult Result;
	Result.Input = Input;
	Result.bHasHelper = Input.bHasHelper;

	if (Input.SelectedSkillId.IsNone())
	{
		SetFailure(
			Result,
			EThroughBallParticipantEligibilityQueryErrorCode
				::InvalidSelectedSkillId,
			TEXT("SelectedSkillId must not be None."),
			SelectedSkillIdField);
		return Result;
	}

	FSkillRuleSnapshotQueryInput SkillRuleQueryInput;
	SkillRuleQueryInput.SkillId = Input.SelectedSkillId;
	Result.SkillRuleQueryResult =
		FSkillRuleSnapshotQuery::FindBySkillId(
			SkillRules,
			SkillRuleQueryInput);
	if (!Result.SkillRuleQueryResult.bSuccess)
	{
		SetFailure(
			Result,
			EThroughBallParticipantEligibilityQueryErrorCode
				::SkillRuleLookupFailed,
			Result.SkillRuleQueryResult.ErrorMessage,
			SelectedSkillIdField);
		return Result;
	}

	const FSkillRuleSnapshot& SkillRule =
		Result.SkillRuleQueryResult.Snapshot;
	if (SkillRule.SkillType != ESkillRuleType::ThroughBall)
	{
		SetFailure(
			Result,
			EThroughBallParticipantEligibilityQueryErrorCode
				::UnsupportedSkillRuleType,
			TEXT("SelectedSkillId must resolve to a ThroughBall SkillRule."),
			SelectedSkillIdField);
		return Result;
	}

	if (Input.CurrentActionPoint < SkillRule.MinTriggerActionPoint)
	{
		SetFailure(
			Result,
			EThroughBallParticipantEligibilityQueryErrorCode
				::ActionPointBelowMinimum,
			TEXT("CurrentActionPoint is below the selected SkillRule minimum."),
			CurrentActionPointField);
		return Result;
	}

	if (Input.CurrentActionPoint > SkillRule.MaxTriggerActionPoint)
	{
		SetFailure(
			Result,
			EThroughBallParticipantEligibilityQueryErrorCode
				::ActionPointAboveMaximum,
			TEXT("CurrentActionPoint is above the selected SkillRule maximum."),
			CurrentActionPointField);
		return Result;
	}

	if (Input.AttackingOwnerId.IsNone())
	{
		SetFailure(
			Result,
			EThroughBallParticipantEligibilityQueryErrorCode
				::InvalidAttackingOwnerIdentity,
			TEXT("AttackingOwnerId must not be None."),
			AttackingOwnerIdField);
		return Result;
	}

	if (Input.DefendingOwnerId.IsNone())
	{
		SetFailure(
			Result,
			EThroughBallParticipantEligibilityQueryErrorCode
				::InvalidDefendingOwnerIdentity,
			TEXT("DefendingOwnerId must not be None."),
			DefendingOwnerIdField);
		return Result;
	}

	if (Input.AttackingOwnerId == Input.DefendingOwnerId)
	{
		SetFailure(
			Result,
			EThroughBallParticipantEligibilityQueryErrorCode
				::DuplicateOwnerIdentity,
			TEXT("AttackingOwnerId and DefendingOwnerId must differ."),
			DefendingOwnerIdField);
		return Result;
	}

	if (Input.CarrierSnapshot.CardId.IsNone())
	{
		SetFailure(
			Result,
			EThroughBallParticipantEligibilityQueryErrorCode
				::InvalidCarrierIdentity,
			TEXT("CarrierSnapshot CardId must not be None."),
			CarrierSnapshotField);
		return Result;
	}

	Result.CarrierSnapshotValidationResult =
		ValidateSnapshot(Input.CarrierSnapshot);
	if (!Result.CarrierSnapshotValidationResult.bSuccess)
	{
		SetFailure(
			Result,
			EThroughBallParticipantEligibilityQueryErrorCode
				::InvalidCarrierSnapshot,
			Result.CarrierSnapshotValidationResult.ErrorMessage,
			CarrierSnapshotField);
		return Result;
	}

	if (Input.CarrierSnapshot.bIsGoalkeeper)
	{
		SetFailure(
			Result,
			EThroughBallParticipantEligibilityQueryErrorCode
				::UnsupportedCarrierGoalkeeper,
			TEXT("CarrierSnapshot must not be a goalkeeper."),
			CarrierSnapshotField);
		return Result;
	}

	if (!Input.CarrierSnapshot.SkillIds.Contains(Input.SelectedSkillId))
	{
		SetFailure(
			Result,
			EThroughBallParticipantEligibilityQueryErrorCode
				::CarrierDoesNotOwnSelectedSkill,
			TEXT("CarrierSnapshot must own SelectedSkillId."),
			CarrierSnapshotField);
		return Result;
	}

	if (Input.RunnerSnapshot.CardId.IsNone())
	{
		SetFailure(
			Result,
			EThroughBallParticipantEligibilityQueryErrorCode
				::InvalidRunnerIdentity,
			TEXT("RunnerSnapshot CardId must not be None."),
			RunnerSnapshotField);
		return Result;
	}

	Result.RunnerSnapshotValidationResult =
		ValidateSnapshot(Input.RunnerSnapshot);
	if (!Result.RunnerSnapshotValidationResult.bSuccess)
	{
		SetFailure(
			Result,
			EThroughBallParticipantEligibilityQueryErrorCode
				::InvalidRunnerSnapshot,
			Result.RunnerSnapshotValidationResult.ErrorMessage,
			RunnerSnapshotField);
		return Result;
	}

	if (Input.RunnerSnapshot.bIsGoalkeeper)
	{
		SetFailure(
			Result,
			EThroughBallParticipantEligibilityQueryErrorCode
				::UnsupportedRunnerGoalkeeper,
			TEXT("RunnerSnapshot must not be a goalkeeper."),
			RunnerSnapshotField);
		return Result;
	}

	if (!Input.bIsRunnerInAttackingForwardArea)
	{
		SetFailure(
			Result,
			EThroughBallParticipantEligibilityQueryErrorCode
				::RunnerNotInAttackingForwardArea,
			TEXT("Runner must be in the attacking forward area."),
			RunnerForwardAreaField);
		return Result;
	}

	if (Input.CarrierSnapshot.CardId == Input.RunnerSnapshot.CardId)
	{
		SetFailure(
			Result,
			EThroughBallParticipantEligibilityQueryErrorCode
				::DuplicateAttackingParticipant,
			TEXT("Carrier and Runner must be different attacking participants."),
			RunnerSnapshotField);
		return Result;
	}

	if (Input.MarkerSnapshot.CardId.IsNone())
	{
		SetFailure(
			Result,
			EThroughBallParticipantEligibilityQueryErrorCode
				::InvalidMarkerIdentity,
			TEXT("MarkerSnapshot CardId must not be None."),
			MarkerSnapshotField);
		return Result;
	}

	Result.MarkerSnapshotValidationResult =
		ValidateSnapshot(Input.MarkerSnapshot);
	if (!Result.MarkerSnapshotValidationResult.bSuccess)
	{
		SetFailure(
			Result,
			EThroughBallParticipantEligibilityQueryErrorCode
				::InvalidMarkerSnapshot,
			Result.MarkerSnapshotValidationResult.ErrorMessage,
			MarkerSnapshotField);
		return Result;
	}

	if (Input.MarkerSnapshot.bIsGoalkeeper)
	{
		SetFailure(
			Result,
			EThroughBallParticipantEligibilityQueryErrorCode
				::UnsupportedMarkerGoalkeeper,
			TEXT("MarkerSnapshot must not be a goalkeeper."),
			MarkerSnapshotField);
		return Result;
	}

	if (Input.bHasHelper)
	{
		if (Input.HelperSnapshot.CardId.IsNone())
		{
			SetFailure(
				Result,
				EThroughBallParticipantEligibilityQueryErrorCode
					::InvalidHelperIdentity,
				TEXT("HelperSnapshot CardId must not be None."),
				HelperSnapshotField);
			return Result;
		}

		Result.HelperSnapshotValidationResult =
			ValidateSnapshot(Input.HelperSnapshot);
		if (!Result.HelperSnapshotValidationResult.bSuccess)
		{
			SetFailure(
				Result,
				EThroughBallParticipantEligibilityQueryErrorCode
					::InvalidHelperSnapshot,
				Result.HelperSnapshotValidationResult.ErrorMessage,
				HelperSnapshotField);
			return Result;
		}

		if (Input.HelperSnapshot.bIsGoalkeeper)
		{
			SetFailure(
				Result,
				EThroughBallParticipantEligibilityQueryErrorCode
					::UnsupportedHelperGoalkeeper,
				TEXT("HelperSnapshot must not be a goalkeeper."),
				HelperSnapshotField);
			return Result;
		}

		if (Input.MarkerSnapshot.CardId == Input.HelperSnapshot.CardId)
		{
			SetFailure(
				Result,
				EThroughBallParticipantEligibilityQueryErrorCode
					::DuplicateDefendingParticipant,
				TEXT("Marker and Helper must be different defending participants."),
				HelperSnapshotField);
			return Result;
		}
	}

	Result.bSuccess = true;
	return Result;
}
