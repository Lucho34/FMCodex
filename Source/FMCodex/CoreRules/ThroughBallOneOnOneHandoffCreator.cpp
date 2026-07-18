#include "ThroughBallOneOnOneHandoffCreator.h"

#include "ThroughBallAntiOffsideOutcomeQuery.h"
#include "ThroughBallBehindDefenseP2OutcomeQuery.h"

namespace ThroughBallOneOnOneHandoffCreator
{
	const FName SourceOutcomeResultField(TEXT("SourceOutcomeResult"));
	const FName ErrorCodeField(TEXT("ErrorCode"));
	const FName ErrorMessageField(TEXT("ErrorMessage"));
	const FName InvalidFieldField(TEXT("InvalidField"));
	const FName DecisionField(TEXT("Decision"));
	const FName AttackEndedField(TEXT("bAttackEnded"));
	const FName ContinueResolutionField(TEXT("bContinueResolution"));
	const FName RequiresOneOnOneField(TEXT("bRequiresOneOnOne"));
	const FName RunnerIdField(TEXT("RunnerId"));
	const FName P1ExecutionResultField(TEXT("P1ExecutionResult"));
	const FName P1ErrorCodeField(TEXT("P1ExecutionResult.ErrorCode"));
	const FName P1ErrorMessageField(TEXT("P1ExecutionResult.ErrorMessage"));
	const FName P1InvalidFieldField(TEXT("P1ExecutionResult.InvalidField"));
	const FName P1DecisionField(TEXT("P1ExecutionResult.Decision"));
	const FName P1AttackEndedField(TEXT("P1ExecutionResult.bAttackEnded"));
	const FName P1ContinueResolutionField(
		TEXT("P1ExecutionResult.bContinueResolution"));
	const FName P1RequiresP2Field(TEXT("P1ExecutionResult.bRequiresP2"));
	const FName PlanQueryResultField(
		TEXT("P1ExecutionResult.PlanQueryResult"));
	const FName PlanErrorCodeField(
		TEXT("P1ExecutionResult.PlanQueryResult.ErrorCode"));
	const FName PlanErrorMessageField(
		TEXT("P1ExecutionResult.PlanQueryResult.ErrorMessage"));
	const FName PlanInvalidFieldField(
		TEXT("P1ExecutionResult.PlanQueryResult.InvalidField"));
	const FName PlanDecisionField(
		TEXT("P1ExecutionResult.PlanQueryResult.Decision"));
	const FName HasFormulaPlanField(
		TEXT("P1ExecutionResult.PlanQueryResult.bHasFormulaPlan"));
	const FName BranchSelectionResultField(TEXT("BranchSelectionResult"));
	const FName BranchErrorCodeField(TEXT("BranchSelectionResult.ErrorCode"));
	const FName BranchErrorMessageField(
		TEXT("BranchSelectionResult.ErrorMessage"));
	const FName BranchInvalidFieldField(
		TEXT("BranchSelectionResult.InvalidField"));
	const FName HasSelectedBranchField(
		TEXT("BranchSelectionResult.bHasSelectedThroughBallBranch"));
	const FName SelectedBranchField(
		TEXT("BranchSelectionResult.SelectedThroughBallBranch"));
	const FName ParticipantEligibilityResultField(
		TEXT("ParticipantEligibilityResult"));
	const FName EligibilityErrorCodeField(
		TEXT("ParticipantEligibilityResult.ErrorCode"));
	const FName EligibilityErrorMessageField(
		TEXT("ParticipantEligibilityResult.ErrorMessage"));
	const FName EligibilityInvalidFieldField(
		TEXT("ParticipantEligibilityResult.InvalidField"));
	const FName AttackingOwnerIdField(TEXT("AttackingOwnerId"));
	const FName DefendingOwnerIdField(TEXT("DefendingOwnerId"));
	const FName ShooterCardIdField(TEXT("ShooterCardId"));

	void SetFailure(
		FThroughBallOneOnOneHandoffCreationResult& Result,
		const EThroughBallOneOnOneHandoffCreationErrorCode ErrorCode,
		const FString& ErrorMessage,
		const FName InvalidField)
	{
		Result.ErrorCode = ErrorCode;
		Result.ErrorMessage = ErrorMessage;
		Result.InvalidField = InvalidField;
	}

	bool RejectInvalidSource(
		FThroughBallOneOnOneHandoffCreationResult& Result,
		const bool bIsValid,
		const FName InvalidField)
	{
		if (bIsValid)
		{
			return false;
		}

		SetFailure(
			Result,
			EThroughBallOneOnOneHandoffCreationErrorCode
				::InvalidSourceOutcomeResult,
			TEXT("Successful source Outcome Result is inconsistent."),
			InvalidField);
		return true;
	}

	bool ValidateOwners(
		FThroughBallOneOnOneHandoffCreationResult& Result,
		const FName AttackingOwnerId,
		const FName DefendingOwnerId)
	{
		if (AttackingOwnerId.IsNone())
		{
			SetFailure(
				Result,
				EThroughBallOneOnOneHandoffCreationErrorCode
					::InvalidAttackingOwnerIdentity,
				TEXT("AttackingOwnerId must not be None."),
				AttackingOwnerIdField);
			return false;
		}

		if (DefendingOwnerId.IsNone())
		{
			SetFailure(
				Result,
				EThroughBallOneOnOneHandoffCreationErrorCode
					::InvalidDefendingOwnerIdentity,
				TEXT("DefendingOwnerId must not be None."),
				DefendingOwnerIdField);
			return false;
		}

		if (AttackingOwnerId == DefendingOwnerId)
		{
			SetFailure(
				Result,
				EThroughBallOneOnOneHandoffCreationErrorCode
					::DuplicateOwnerIdentity,
				TEXT("AttackingOwnerId and DefendingOwnerId must differ."),
				DefendingOwnerIdField);
			return false;
		}

		return true;
	}

	FThroughBallOneOnOneHandoffCreationResult CreateSuccess(
		const FName AttackingOwnerId,
		const FName DefendingOwnerId,
		const FName ShooterCardId)
	{
		FThroughBallOneOnOneHandoffCreationResult Result;
		Result.Handoff.AttackingOwnerId = AttackingOwnerId;
		Result.Handoff.DefendingOwnerId = DefendingOwnerId;
		Result.Handoff.ShooterCardId = ShooterCardId;
		Result.bHasHandoff = true;
		Result.bSuccess = true;
		return Result;
	}
}

FThroughBallOneOnOneHandoffCreationResult
FThroughBallOneOnOneHandoffCreator::CreateFromBehindDefenseP2(
	const FThroughBallBehindDefenseP2OutcomeQueryResult& P2OutcomeResult)
{
	using namespace ThroughBallOneOnOneHandoffCreator;

	FThroughBallOneOnOneHandoffCreationResult Result;
	if (!P2OutcomeResult.bSuccess)
	{
		SetFailure(
			Result,
			EThroughBallOneOnOneHandoffCreationErrorCode::SourceOutcomeFailed,
			TEXT("Behind Defense P2 Outcome must succeed before Handoff creation."),
			SourceOutcomeResultField);
		return Result;
	}

	if (RejectInvalidSource(
			Result,
			P2OutcomeResult.ErrorCode
				== EThroughBallBehindDefenseP2OutcomeQueryErrorCode::None,
			ErrorCodeField)
		|| RejectInvalidSource(
			Result,
			P2OutcomeResult.ErrorMessage.IsEmpty(),
			ErrorMessageField)
		|| RejectInvalidSource(
			Result,
			P2OutcomeResult.InvalidField.IsNone(),
			InvalidFieldField))
	{
		return Result;
	}

	if (P2OutcomeResult.Decision
		!= EThroughBallBehindDefenseP2OutcomeDecision::Offside
		&& P2OutcomeResult.Decision
			!= EThroughBallBehindDefenseP2OutcomeDecision::OneOnOneRequired)
	{
		RejectInvalidSource(Result, false, DecisionField);
		return Result;
	}

	const bool bIsOffside = P2OutcomeResult.Decision
		== EThroughBallBehindDefenseP2OutcomeDecision::Offside;
	if (RejectInvalidSource(
			Result,
			P2OutcomeResult.bAttackEnded == bIsOffside,
			AttackEndedField)
		|| RejectInvalidSource(
			Result,
			P2OutcomeResult.bContinueResolution == !bIsOffside,
			ContinueResolutionField)
		|| RejectInvalidSource(
			Result,
			P2OutcomeResult.bRequiresOneOnOne == !bIsOffside,
			RequiresOneOnOneField)
		|| RejectInvalidSource(
			Result,
			!bIsOffside || P2OutcomeResult.RunnerId.IsNone(),
			RunnerIdField))
	{
		return Result;
	}

	if (bIsOffside)
	{
		SetFailure(
			Result,
			EThroughBallOneOnOneHandoffCreationErrorCode
				::UnsupportedSourceOutcomeDecision,
			TEXT("Offside does not create a One-on-One Handoff."),
			DecisionField);
		return Result;
	}

	const FThroughBallBehindDefenseP1FormulaResolutionExecutionResult& P1Result =
		P2OutcomeResult.Input.P1ExecutionResult;
	if (RejectInvalidSource(Result, P1Result.bSuccess, P1ExecutionResultField)
		|| RejectInvalidSource(
			Result,
			P1Result.ErrorCode
				== EThroughBallBehindDefenseP1FormulaResolutionExecutionErrorCode::None,
			P1ErrorCodeField)
		|| RejectInvalidSource(
			Result,
			P1Result.ErrorMessage.IsEmpty(),
			P1ErrorMessageField)
		|| RejectInvalidSource(
			Result,
			P1Result.InvalidField.IsNone(),
			P1InvalidFieldField)
		|| RejectInvalidSource(
			Result,
			P1Result.Decision
				== EThroughBallBehindDefenseP1FormulaResolutionExecutionDecision
					::P2Required,
			P1DecisionField)
		|| RejectInvalidSource(
			Result,
			!P1Result.bAttackEnded,
			P1AttackEndedField)
		|| RejectInvalidSource(
			Result,
			P1Result.bContinueResolution,
			P1ContinueResolutionField)
		|| RejectInvalidSource(
			Result,
			P1Result.bRequiresP2,
			P1RequiresP2Field))
	{
		return Result;
	}

	const FThroughBallBehindDefenseP1PlanQueryResult& PlanResult =
		P1Result.Input.ResolverInputAssemblyResult.Input.PlanQueryResult;
	if (RejectInvalidSource(Result, PlanResult.bSuccess, PlanQueryResultField)
		|| RejectInvalidSource(
			Result,
			PlanResult.ErrorCode
				== EThroughBallBehindDefenseP1PlanQueryErrorCode::None,
			PlanErrorCodeField)
		|| RejectInvalidSource(
			Result,
			PlanResult.ErrorMessage.IsEmpty(),
			PlanErrorMessageField)
		|| RejectInvalidSource(
			Result,
			PlanResult.InvalidField.IsNone(),
			PlanInvalidFieldField)
		|| RejectInvalidSource(
			Result,
			PlanResult.Decision
				== EThroughBallBehindDefenseP1PlanQueryDecision
					::FormulaResolutionRequired,
			PlanDecisionField)
		|| RejectInvalidSource(
			Result,
			PlanResult.bHasFormulaPlan,
			HasFormulaPlanField))
	{
		return Result;
	}

	const FName AttackingOwnerId = PlanResult.FormulaPlan.AttackingOwnerId;
	const FName DefendingOwnerId = PlanResult.FormulaPlan.DefendingOwnerId;
	if (!ValidateOwners(Result, AttackingOwnerId, DefendingOwnerId))
	{
		return Result;
	}

	const FName P2RunnerId = P2OutcomeResult.RunnerId;
	const FName P1RunnerId = P1Result.RunnerId;
	const FName PlanRunnerId = PlanResult.FormulaPlan.RunnerId;
	if (P2RunnerId.IsNone() || P1RunnerId.IsNone() || PlanRunnerId.IsNone())
	{
		SetFailure(
			Result,
			EThroughBallOneOnOneHandoffCreationErrorCode
				::InvalidShooterIdentity,
			TEXT("Shooter identity provenance must not be None."),
			ShooterCardIdField);
		return Result;
	}

	if (P2RunnerId != P1RunnerId || P2RunnerId != PlanRunnerId)
	{
		SetFailure(
			Result,
			EThroughBallOneOnOneHandoffCreationErrorCode
				::InconsistentShooterIdentity,
			TEXT("Shooter identity provenance must agree."),
			ShooterCardIdField);
		return Result;
	}

	return CreateSuccess(AttackingOwnerId, DefendingOwnerId, P2RunnerId);
}

FThroughBallOneOnOneHandoffCreationResult
FThroughBallOneOnOneHandoffCreator::CreateFromAntiOffside(
	const FThroughBallAntiOffsideOutcomeQueryResult& AntiOffsideOutcomeResult)
{
	using namespace ThroughBallOneOnOneHandoffCreator;

	FThroughBallOneOnOneHandoffCreationResult Result;
	if (!AntiOffsideOutcomeResult.bSuccess)
	{
		SetFailure(
			Result,
			EThroughBallOneOnOneHandoffCreationErrorCode::SourceOutcomeFailed,
			TEXT("Anti-Offside Outcome must succeed before Handoff creation."),
			SourceOutcomeResultField);
		return Result;
	}

	if (RejectInvalidSource(
			Result,
			AntiOffsideOutcomeResult.ErrorCode
				== EThroughBallAntiOffsideOutcomeQueryErrorCode::None,
			ErrorCodeField)
		|| RejectInvalidSource(
			Result,
			AntiOffsideOutcomeResult.ErrorMessage.IsEmpty(),
			ErrorMessageField)
		|| RejectInvalidSource(
			Result,
			AntiOffsideOutcomeResult.InvalidField.IsNone(),
			InvalidFieldField))
	{
		return Result;
	}

	if (AntiOffsideOutcomeResult.Decision
		!= EThroughBallAntiOffsideOutcomeDecision::Offside
		&& AntiOffsideOutcomeResult.Decision
			!= EThroughBallAntiOffsideOutcomeDecision::OneOnOneRequired)
	{
		RejectInvalidSource(Result, false, DecisionField);
		return Result;
	}

	const bool bIsOffside = AntiOffsideOutcomeResult.Decision
		== EThroughBallAntiOffsideOutcomeDecision::Offside;
	if (RejectInvalidSource(
			Result,
			AntiOffsideOutcomeResult.bAttackEnded == bIsOffside,
			AttackEndedField)
		|| RejectInvalidSource(
			Result,
			AntiOffsideOutcomeResult.bContinueResolution == !bIsOffside,
			ContinueResolutionField)
		|| RejectInvalidSource(
			Result,
			AntiOffsideOutcomeResult.bRequiresOneOnOne == !bIsOffside,
			RequiresOneOnOneField)
		|| RejectInvalidSource(
			Result,
			!bIsOffside || AntiOffsideOutcomeResult.RunnerId.IsNone(),
			RunnerIdField))
	{
		return Result;
	}

	if (bIsOffside)
	{
		SetFailure(
			Result,
			EThroughBallOneOnOneHandoffCreationErrorCode
				::UnsupportedSourceOutcomeDecision,
			TEXT("Offside does not create a One-on-One Handoff."),
			DecisionField);
		return Result;
	}

	const FThroughBallBranchSelectionQueryResult& BranchResult =
		AntiOffsideOutcomeResult.Input.BranchSelectionResult;
	if (RejectInvalidSource(
			Result,
			BranchResult.bSuccess,
			BranchSelectionResultField)
		|| RejectInvalidSource(
			Result,
			BranchResult.ErrorCode
				== EThroughBallBranchSelectionQueryErrorCode::None,
			BranchErrorCodeField)
		|| RejectInvalidSource(
			Result,
			BranchResult.ErrorMessage.IsEmpty(),
			BranchErrorMessageField)
		|| RejectInvalidSource(
			Result,
			BranchResult.InvalidField.IsNone(),
			BranchInvalidFieldField)
		|| RejectInvalidSource(
			Result,
			BranchResult.bHasSelectedThroughBallBranch,
			HasSelectedBranchField)
		|| RejectInvalidSource(
			Result,
			BranchResult.SelectedThroughBallBranch
				== EThroughBallSelectedBranch::AntiOffside,
			SelectedBranchField))
	{
		return Result;
	}

	const FThroughBallParticipantEligibilityQueryResult& EligibilityResult =
		AntiOffsideOutcomeResult.Input.ParticipantEligibilityResult;
	if (RejectInvalidSource(
			Result,
			EligibilityResult.bSuccess,
			ParticipantEligibilityResultField)
		|| RejectInvalidSource(
			Result,
			EligibilityResult.ErrorCode
				== EThroughBallParticipantEligibilityQueryErrorCode::None,
			EligibilityErrorCodeField)
		|| RejectInvalidSource(
			Result,
			EligibilityResult.ErrorMessage.IsEmpty(),
			EligibilityErrorMessageField)
		|| RejectInvalidSource(
			Result,
			EligibilityResult.InvalidField.IsNone(),
			EligibilityInvalidFieldField))
	{
		return Result;
	}

	const FName AttackingOwnerId = EligibilityResult.Input.AttackingOwnerId;
	const FName DefendingOwnerId = EligibilityResult.Input.DefendingOwnerId;
	if (!ValidateOwners(Result, AttackingOwnerId, DefendingOwnerId))
	{
		return Result;
	}

	const FName OutcomeRunnerId = AntiOffsideOutcomeResult.RunnerId;
	const FName EligibilityRunnerId =
		EligibilityResult.Input.RunnerSnapshot.CardId;
	if (OutcomeRunnerId.IsNone() || EligibilityRunnerId.IsNone())
	{
		SetFailure(
			Result,
			EThroughBallOneOnOneHandoffCreationErrorCode
				::InvalidShooterIdentity,
			TEXT("Shooter identity provenance must not be None."),
			ShooterCardIdField);
		return Result;
	}

	if (OutcomeRunnerId != EligibilityRunnerId)
	{
		SetFailure(
			Result,
			EThroughBallOneOnOneHandoffCreationErrorCode
				::InconsistentShooterIdentity,
			TEXT("Shooter identity provenance must agree."),
			ShooterCardIdField);
		return Result;
	}

	return CreateSuccess(
		AttackingOwnerId,
		DefendingOwnerId,
		OutcomeRunnerId);
}
