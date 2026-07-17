#include "ThroughBallAntiOffsideOutcomeQuery.h"

namespace ThroughBallAntiOffsideOutcomeQuery
{
	const FName BranchSelectionResultField(TEXT("BranchSelectionResult"));
	const FName BranchErrorCodeField(TEXT("BranchSelectionResult.ErrorCode"));
	const FName BranchErrorMessageField(TEXT("BranchSelectionResult.ErrorMessage"));
	const FName BranchInvalidFieldField(TEXT("BranchSelectionResult.InvalidField"));
	const FName HasSelectedBranchField(
		TEXT("BranchSelectionResult.bHasSelectedThroughBallBranch"));
	const FName SelectedBranchField(
		TEXT("BranchSelectionResult.SelectedThroughBallBranch"));
	const FName HasBranchSelectionD6Field(
		TEXT("BranchSelectionResult.Input.bHasExternalSelectionD6"));
	const FName BranchSelectionD6Field(
		TEXT("BranchSelectionResult.Input.ExternalSelectionD6"));

	const FName ParticipantEligibilityResultField(
		TEXT("ParticipantEligibilityResult"));
	const FName EligibilityErrorCodeField(
		TEXT("ParticipantEligibilityResult.ErrorCode"));
	const FName EligibilityErrorMessageField(
		TEXT("ParticipantEligibilityResult.ErrorMessage"));
	const FName EligibilityInvalidFieldField(
		TEXT("ParticipantEligibilityResult.InvalidField"));
	const FName EligibilityHasHelperField(
		TEXT("ParticipantEligibilityResult.bHasHelper"));
	const FName SkillRuleQueryResultField(
		TEXT("ParticipantEligibilityResult.SkillRuleQueryResult"));
	const FName CarrierValidationResultField(
		TEXT("ParticipantEligibilityResult.CarrierSnapshotValidationResult"));
	const FName RunnerValidationResultField(
		TEXT("ParticipantEligibilityResult.RunnerSnapshotValidationResult"));
	const FName MarkerValidationResultField(
		TEXT("ParticipantEligibilityResult.MarkerSnapshotValidationResult"));
	const FName HelperValidationResultField(
		TEXT("ParticipantEligibilityResult.HelperSnapshotValidationResult"));
	const FName AttackingOwnerIdField(
		TEXT("ParticipantEligibilityResult.Input.AttackingOwnerId"));
	const FName DefendingOwnerIdField(
		TEXT("ParticipantEligibilityResult.Input.DefendingOwnerId"));
	const FName RunnerSnapshotField(
		TEXT("ParticipantEligibilityResult.Input.RunnerSnapshot"));
	const FName RunnerCardIdField(
		TEXT("ParticipantEligibilityResult.Input.RunnerSnapshot.CardId"));
	const FName RunnerForwardAreaField(
		TEXT("ParticipantEligibilityResult.Input.bIsRunnerInAttackingForwardArea"));
	const FName CarrierCardIdField(
		TEXT("ParticipantEligibilityResult.Input.CarrierSnapshot.CardId"));

	const FName AntiOffsideAttackD6Field(TEXT("AntiOffsideAttackD6"));

	constexpr int32 MinD6 = 1;
	constexpr int32 MaxD6 = 6;

	void SetFailure(
		FThroughBallAntiOffsideOutcomeQueryResult& Result,
		const EThroughBallAntiOffsideOutcomeQueryErrorCode ErrorCode,
		const FString& ErrorMessage,
		const FName InvalidField)
	{
		Result.ErrorCode = ErrorCode;
		Result.ErrorMessage = ErrorMessage;
		Result.InvalidField = InvalidField;
	}

	bool RejectInvalidBranchResult(
		FThroughBallAntiOffsideOutcomeQueryResult& Result,
		const bool bIsValid,
		const FName InvalidField)
	{
		if (bIsValid)
		{
			return false;
		}

		SetFailure(
			Result,
			EThroughBallAntiOffsideOutcomeQueryErrorCode
				::InvalidBranchSelectionResult,
			TEXT("Successful Branch Selection Result is inconsistent."),
			InvalidField);
		return true;
	}

	bool RejectInvalidEligibilityResult(
		FThroughBallAntiOffsideOutcomeQueryResult& Result,
		const bool bIsValid,
		const FName InvalidField)
	{
		if (bIsValid)
		{
			return false;
		}

		SetFailure(
			Result,
			EThroughBallAntiOffsideOutcomeQueryErrorCode
				::InvalidParticipantEligibilityResult,
			TEXT("Successful Participant Eligibility Result is inconsistent."),
			InvalidField);
		return true;
	}

	bool IsD6InRange(const int32 D6)
	{
		return D6 >= MinD6 && D6 <= MaxD6;
	}

	EThroughBallSelectedBranch MapSelectionD6ToBranch(const int32 D6)
	{
		if (D6 <= 2)
		{
			return EThroughBallSelectedBranch::Feet;
		}

		if (D6 <= 4)
		{
			return EThroughBallSelectedBranch::BehindDefense;
		}

		return EThroughBallSelectedBranch::AntiOffside;
	}

	bool IsSkillRuleQuerySuccessEnvelopeClean(
		const FSkillRuleSnapshotQueryResult& Result)
	{
		const FSkillRuleSnapshotValidationResult& Validation =
			Result.ValidationResult;
		return Result.bSuccess
			&& Result.bFound
			&& Result.ErrorCode == ESkillRuleSnapshotQueryErrorCode::None
			&& Result.ErrorMessage.IsEmpty()
			&& Result.InvalidSkillId.IsNone()
			&& Result.InvalidField.IsNone()
			&& Validation.bSuccess
			&& Validation.bIsValid
			&& Validation.ErrorCode
				== ESkillRuleSnapshotValidationErrorCode::None
			&& Validation.ErrorMessage.IsEmpty()
			&& Validation.InvalidSkillId.IsNone()
			&& Validation.InvalidField.IsNone();
	}

	bool IsSnapshotValidationSuccessEnvelopeClean(
		const FPlayerCardRuleSnapshotValidationResult& Result)
	{
		return Result.bSuccess
			&& Result.bIsValid
			&& Result.ErrorCode
				== EPlayerCardRuleSnapshotValidationErrorCode::None
			&& Result.ErrorMessage.IsEmpty()
			&& Result.InvalidCardId.IsNone()
			&& Result.DuplicateCardIds.IsEmpty();
	}
}

FThroughBallAntiOffsideOutcomeQueryResult
FThroughBallAntiOffsideOutcomeQuery::Evaluate(
	const FThroughBallAntiOffsideOutcomeQueryInput& Input)
{
	using namespace ThroughBallAntiOffsideOutcomeQuery;

	FThroughBallAntiOffsideOutcomeQueryResult Result;
	Result.Input = Input;

	const FThroughBallBranchSelectionQueryResult& BranchResult =
		Input.BranchSelectionResult;
	if (!BranchResult.bSuccess)
	{
		SetFailure(
			Result,
			EThroughBallAntiOffsideOutcomeQueryErrorCode
				::BranchSelectionFailed,
			TEXT("Branch Selection must succeed before Anti-Offside Outcome evaluation."),
			BranchSelectionResultField);
		return Result;
	}

	if (RejectInvalidBranchResult(
			Result,
			BranchResult.ErrorCode
				== EThroughBallBranchSelectionQueryErrorCode::None,
			BranchErrorCodeField)
		|| RejectInvalidBranchResult(
			Result,
			BranchResult.ErrorMessage.IsEmpty(),
			BranchErrorMessageField)
		|| RejectInvalidBranchResult(
			Result,
			BranchResult.InvalidField.IsNone(),
			BranchInvalidFieldField)
		|| RejectInvalidBranchResult(
			Result,
			BranchResult.bHasSelectedThroughBallBranch,
			HasSelectedBranchField)
		|| RejectInvalidBranchResult(
			Result,
			BranchResult.Input.bHasExternalSelectionD6,
			HasBranchSelectionD6Field)
		|| RejectInvalidBranchResult(
			Result,
			IsD6InRange(BranchResult.Input.ExternalSelectionD6),
			BranchSelectionD6Field)
		|| RejectInvalidBranchResult(
			Result,
			BranchResult.SelectedThroughBallBranch
				== MapSelectionD6ToBranch(
					BranchResult.Input.ExternalSelectionD6),
			SelectedBranchField))
	{
		return Result;
	}

	if (BranchResult.SelectedThroughBallBranch
		!= EThroughBallSelectedBranch::AntiOffside)
	{
		SetFailure(
			Result,
			EThroughBallAntiOffsideOutcomeQueryErrorCode::UnsupportedBranch,
			TEXT("Anti-Offside Outcome requires the AntiOffside branch."),
			SelectedBranchField);
		return Result;
	}

	const FThroughBallParticipantEligibilityQueryResult& EligibilityResult =
		Input.ParticipantEligibilityResult;
	if (!EligibilityResult.bSuccess)
	{
		SetFailure(
			Result,
			EThroughBallAntiOffsideOutcomeQueryErrorCode
				::ParticipantEligibilityFailed,
			TEXT("Participant Eligibility must succeed before Anti-Offside Outcome evaluation."),
			ParticipantEligibilityResultField);
		return Result;
	}

	if (RejectInvalidEligibilityResult(
			Result,
			EligibilityResult.ErrorCode
				== EThroughBallParticipantEligibilityQueryErrorCode::None,
			EligibilityErrorCodeField)
		|| RejectInvalidEligibilityResult(
			Result,
			EligibilityResult.ErrorMessage.IsEmpty(),
			EligibilityErrorMessageField)
		|| RejectInvalidEligibilityResult(
			Result,
			EligibilityResult.InvalidField.IsNone(),
			EligibilityInvalidFieldField))
	{
		return Result;
	}

	const FThroughBallParticipantEligibilityQueryInput& Participants =
		EligibilityResult.Input;
	if (RejectInvalidEligibilityResult(
			Result,
			EligibilityResult.bHasHelper == Participants.bHasHelper,
			EligibilityHasHelperField)
		|| RejectInvalidEligibilityResult(
			Result,
			IsSkillRuleQuerySuccessEnvelopeClean(
				EligibilityResult.SkillRuleQueryResult),
			SkillRuleQueryResultField)
		|| RejectInvalidEligibilityResult(
			Result,
			IsSnapshotValidationSuccessEnvelopeClean(
				EligibilityResult.CarrierSnapshotValidationResult),
			CarrierValidationResultField)
		|| RejectInvalidEligibilityResult(
			Result,
			IsSnapshotValidationSuccessEnvelopeClean(
				EligibilityResult.RunnerSnapshotValidationResult),
			RunnerValidationResultField)
		|| RejectInvalidEligibilityResult(
			Result,
			IsSnapshotValidationSuccessEnvelopeClean(
				EligibilityResult.MarkerSnapshotValidationResult),
			MarkerValidationResultField)
		|| RejectInvalidEligibilityResult(
			Result,
			!EligibilityResult.bHasHelper
				|| IsSnapshotValidationSuccessEnvelopeClean(
					EligibilityResult.HelperSnapshotValidationResult),
			HelperValidationResultField))
	{
		return Result;
	}

	if (RejectInvalidEligibilityResult(
			Result,
			!Participants.AttackingOwnerId.IsNone(),
			AttackingOwnerIdField)
		|| RejectInvalidEligibilityResult(
			Result,
			!Participants.DefendingOwnerId.IsNone(),
			DefendingOwnerIdField)
		|| RejectInvalidEligibilityResult(
			Result,
			Participants.AttackingOwnerId
				!= Participants.DefendingOwnerId,
			DefendingOwnerIdField))
	{
		return Result;
	}

	const FPlayerCardRuleSnapshot& Runner = Participants.RunnerSnapshot;
	if (RejectInvalidEligibilityResult(
			Result,
			!Runner.CardId.IsNone(),
			RunnerCardIdField)
		|| RejectInvalidEligibilityResult(
			Result,
			!Runner.bIsGoalkeeper,
			RunnerSnapshotField)
		|| RejectInvalidEligibilityResult(
			Result,
			Participants.bIsRunnerInAttackingForwardArea,
			RunnerForwardAreaField))
	{
		return Result;
	}

	const FName CarrierId = Participants.CarrierSnapshot.CardId;
	if (RejectInvalidEligibilityResult(
			Result,
			!CarrierId.IsNone(),
			CarrierCardIdField)
		|| RejectInvalidEligibilityResult(
			Result,
			CarrierId != Runner.CardId,
			RunnerCardIdField))
	{
		return Result;
	}

	if (!Input.bHasAntiOffsideAttackD6)
	{
		SetFailure(
			Result,
			EThroughBallAntiOffsideOutcomeQueryErrorCode
				::MissingAntiOffsideAttackD6,
			TEXT("An externally supplied AntiOffsideAttackD6 is required."),
			AntiOffsideAttackD6Field);
		return Result;
	}

	if (!IsD6InRange(Input.AntiOffsideAttackD6))
	{
		SetFailure(
			Result,
			EThroughBallAntiOffsideOutcomeQueryErrorCode
				::InvalidAntiOffsideAttackD6,
			TEXT("AntiOffsideAttackD6 must be in range [1, 6]."),
			AntiOffsideAttackD6Field);
		return Result;
	}

	if (Input.AntiOffsideAttackD6 == MaxD6)
	{
		Result.Decision =
			EThroughBallAntiOffsideOutcomeDecision::OneOnOneRequired;
		Result.bContinueResolution = true;
		Result.bRequiresOneOnOne = true;
		Result.RunnerId = Runner.CardId;
	}
	else
	{
		Result.Decision = EThroughBallAntiOffsideOutcomeDecision::Offside;
		Result.bAttackEnded = true;
	}

	Result.bSuccess = true;
	return Result;
}
