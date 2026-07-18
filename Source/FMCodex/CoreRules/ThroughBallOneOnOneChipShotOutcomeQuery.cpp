#include "ThroughBallOneOnOneChipShotOutcomeQuery.h"

namespace ThroughBallOneOnOneChipShotOutcomeQuery
{
	const FName HandoffCreationResultField(TEXT("HandoffCreationResult"));
	const FName HandoffErrorCodeField(
		TEXT("HandoffCreationResult.ErrorCode"));
	const FName HandoffErrorMessageField(
		TEXT("HandoffCreationResult.ErrorMessage"));
	const FName HandoffInvalidFieldField(
		TEXT("HandoffCreationResult.InvalidField"));
	const FName HasHandoffField(
		TEXT("HandoffCreationResult.bHasHandoff"));
	const FName AttackingOwnerIdField(TEXT("Handoff.AttackingOwnerId"));
	const FName DefendingOwnerIdField(TEXT("Handoff.DefendingOwnerId"));
	const FName ShooterCardIdField(TEXT("Handoff.ShooterCardId"));
	const FName ChipShotAttackD6Field(TEXT("ChipShotAttackD6"));
	const FName LogIdField(TEXT("LogId"));
	const FName TurnIndexField(TEXT("TurnIndex"));

	void SetFailure(
		FThroughBallOneOnOneChipShotOutcomeQueryResult& Result,
		const EThroughBallOneOnOneChipShotOutcomeQueryErrorCode ErrorCode,
		const FString& ErrorMessage,
		const FName InvalidField)
	{
		Result.ErrorCode = ErrorCode;
		Result.ErrorMessage = ErrorMessage;
		Result.InvalidField = InvalidField;
	}

	bool RejectInvalidHandoffResult(
		FThroughBallOneOnOneChipShotOutcomeQueryResult& Result,
		const bool bIsValid,
		const FName InvalidField)
	{
		if (bIsValid)
		{
			return false;
		}

		SetFailure(
			Result,
			EThroughBallOneOnOneChipShotOutcomeQueryErrorCode
				::InvalidHandoffCreationResult,
			TEXT("Successful Handoff Creation Result is inconsistent."),
			InvalidField);
		return true;
	}
}

FThroughBallOneOnOneChipShotOutcomeQueryResult
FThroughBallOneOnOneChipShotOutcomeQuery::Evaluate(
	const FThroughBallOneOnOneChipShotOutcomeQueryInput& Input)
{
	using namespace ThroughBallOneOnOneChipShotOutcomeQuery;

	FThroughBallOneOnOneChipShotOutcomeQueryResult Result;
	Result.Input = Input;

	const FThroughBallOneOnOneHandoffCreationResult& HandoffResult =
		Input.HandoffCreationResult;
	if (!HandoffResult.bSuccess)
	{
		SetFailure(
			Result,
			EThroughBallOneOnOneChipShotOutcomeQueryErrorCode
				::HandoffCreationFailed,
			TEXT("One-on-One Handoff Creation must succeed before Chip Shot evaluation."),
			HandoffCreationResultField);
		return Result;
	}

	if (RejectInvalidHandoffResult(
			Result,
			HandoffResult.ErrorCode
				== EThroughBallOneOnOneHandoffCreationErrorCode::None,
			HandoffErrorCodeField)
		|| RejectInvalidHandoffResult(
			Result,
			HandoffResult.ErrorMessage.IsEmpty(),
			HandoffErrorMessageField)
		|| RejectInvalidHandoffResult(
			Result,
			HandoffResult.InvalidField.IsNone(),
			HandoffInvalidFieldField)
		|| RejectInvalidHandoffResult(
			Result,
			HandoffResult.bHasHandoff,
			HasHandoffField)
		|| RejectInvalidHandoffResult(
			Result,
			!HandoffResult.Handoff.AttackingOwnerId.IsNone(),
			AttackingOwnerIdField)
		|| RejectInvalidHandoffResult(
			Result,
			!HandoffResult.Handoff.DefendingOwnerId.IsNone(),
			DefendingOwnerIdField)
		|| RejectInvalidHandoffResult(
			Result,
			HandoffResult.Handoff.AttackingOwnerId
				!= HandoffResult.Handoff.DefendingOwnerId,
			DefendingOwnerIdField)
		|| RejectInvalidHandoffResult(
			Result,
			!HandoffResult.Handoff.ShooterCardId.IsNone(),
			ShooterCardIdField))
	{
		return Result;
	}

	if (!Input.bHasChipShotAttackD6)
	{
		SetFailure(
			Result,
			EThroughBallOneOnOneChipShotOutcomeQueryErrorCode
				::MissingChipShotAttackD6,
			TEXT("An externally supplied Chip Shot attack D6 is required."),
			ChipShotAttackD6Field);
		return Result;
	}

	if (Input.ChipShotAttackD6 < 1 || Input.ChipShotAttackD6 > 6)
	{
		SetFailure(
			Result,
			EThroughBallOneOnOneChipShotOutcomeQueryErrorCode
				::InvalidChipShotAttackD6,
			TEXT("ChipShotAttackD6 must be in range [1, 6]."),
			ChipShotAttackD6Field);
		return Result;
	}

	if (!Input.LogId.IsValid())
	{
		SetFailure(
			Result,
			EThroughBallOneOnOneChipShotOutcomeQueryErrorCode
				::InvalidLogContext,
			TEXT("LogId must be valid."),
			LogIdField);
		return Result;
	}

	if (Input.TurnIndex < 0)
	{
		SetFailure(
			Result,
			EThroughBallOneOnOneChipShotOutcomeQueryErrorCode
				::InvalidLogContext,
			TEXT("TurnIndex must not be negative."),
			TurnIndexField);
		return Result;
	}

	Result.Decision = Input.ChipShotAttackD6 <= 3
		? EThroughBallOneOnOneChipShotOutcomeDecision::Miss
		: EThroughBallOneOnOneChipShotOutcomeDecision::Goal;
	Result.bAttackEnded = true;
	Result.bContinueResolution = false;
	Result.bIsGoal = Result.Decision
		== EThroughBallOneOnOneChipShotOutcomeDecision::Goal;
	Result.bSuccess = true;
	return Result;
}
