#include "ThroughBallBehindDefenseP1PlanQuery.h"

namespace ThroughBallBehindDefenseP1PlanQuery
{
	const FName ParticipantEligibilityResultField(
		TEXT("ParticipantEligibilityResult"));
	const FName SelectedBranchField(TEXT("SelectedBranch"));
	const FName AttackD6Field(TEXT("AttackD6"));
	const FName DefenseD6Field(TEXT("DefenseD6"));
	const FName LogIdField(TEXT("LogId"));
	const FName TurnIndexField(TEXT("TurnIndex"));

	constexpr int32 MinD6 = 1;
	constexpr int32 MaxD6 = 6;
	constexpr int32 OutOfPlayMaxAttackD6 = 2;
	constexpr float DefenseModifier = 1.0f;

	void SetFailure(
		FThroughBallBehindDefenseP1PlanQueryResult& Result,
		const EThroughBallBehindDefenseP1PlanQueryErrorCode ErrorCode,
		const FString& ErrorMessage,
		const FName InvalidField)
	{
		Result.ErrorCode = ErrorCode;
		Result.ErrorMessage = ErrorMessage;
		Result.InvalidField = InvalidField;
	}

	bool IsD6InRange(const int32 D6)
	{
		return D6 >= MinD6 && D6 <= MaxD6;
	}

	bool IsEligibilitySuccessStateConsistent(
		const FThroughBallParticipantEligibilityQueryResult& Result)
	{
		const FThroughBallParticipantEligibilityQueryInput& Participants =
			Result.Input;
		return Result.bSuccess
			&& Result.ErrorCode
				== EThroughBallParticipantEligibilityQueryErrorCode::None
			&& Result.ErrorMessage.IsEmpty()
			&& Result.InvalidField.IsNone()
			&& Result.bHasHelper == Participants.bHasHelper
			&& !Participants.CarrierSnapshot.CardId.IsNone()
			&& !Participants.RunnerSnapshot.CardId.IsNone()
			&& !Participants.MarkerSnapshot.CardId.IsNone()
			&& (!Result.bHasHelper
				|| !Participants.HelperSnapshot.CardId.IsNone())
			&& Result.SkillRuleQueryResult.bSuccess
			&& Result.CarrierSnapshotValidationResult.bSuccess
			&& Result.RunnerSnapshotValidationResult.bSuccess
			&& Result.MarkerSnapshotValidationResult.bSuccess
			&& (!Result.bHasHelper
				|| Result.HelperSnapshotValidationResult.bSuccess);
	}

	float RoundOneDecimal(const float Value)
	{
		return FMath::RoundToFloat(Value * 10.0f) / 10.0f;
	}

	float AverageOneDecimal(const int32 Left, const int32 Right)
	{
		return RoundOneDecimal(
			(static_cast<float>(Left) + static_cast<float>(Right))
			/ 2.0f);
	}

	FThroughBallBehindDefenseP1FormulaPlan BuildFormulaPlan(
		const FThroughBallBehindDefenseP1PlanQueryInput& Input)
	{
		const FThroughBallParticipantEligibilityQueryResult& Eligibility =
			Input.ParticipantEligibilityResult;
		const FThroughBallParticipantEligibilityQueryInput& Participants =
			Eligibility.Input;
		const FPlayerCardRuleSnapshot& Carrier = Participants.CarrierSnapshot;
		const FPlayerCardRuleSnapshot& Runner = Participants.RunnerSnapshot;
		const FPlayerCardRuleSnapshot& Marker = Participants.MarkerSnapshot;

		FThroughBallBehindDefenseP1FormulaPlan Plan;
		Plan.FormulaType = EFormulaType::Transition;

		Plan.CarrierId = Carrier.CardId;
		Plan.CarrierPassing = Carrier.Attributes.Passing;
		Plan.CarrierStamina = Carrier.Attributes.Stamina;
		Plan.RunnerId = Runner.CardId;
		Plan.RunnerSpeed = Runner.Attributes.Speed;
		Plan.RunnerStamina = Runner.Attributes.Stamina;
		Plan.AttackD6 = Input.AttackD6;
		Plan.AttackBaseValue = AverageOneDecimal(
			Plan.CarrierPassing,
			Plan.RunnerSpeed);
		Plan.AttackExternalModifier = 0.0f;
		Plan.AttackParticipatingStamina = {
			Plan.CarrierStamina,
			Plan.RunnerStamina
		};

		Plan.MarkerId = Marker.CardId;
		Plan.MarkerMarking = Marker.Attributes.Marking;
		Plan.MarkerStamina = Marker.Attributes.Stamina;
		Plan.bHasHelper = Eligibility.bHasHelper;
		if (Plan.bHasHelper)
		{
			const FPlayerCardRuleSnapshot& Helper = Participants.HelperSnapshot;
			Plan.HelperId = Helper.CardId;
			Plan.HelperSpeed = Helper.Attributes.Speed;
			Plan.HelperStamina = Helper.Attributes.Stamina;
		}

		Plan.DefenseD6 = Input.DefenseD6;
		Plan.DefenseBaseValue = AverageOneDecimal(
			Plan.MarkerMarking,
			Plan.bHasHelper ? Plan.HelperSpeed : 0);
		Plan.DefenseExternalModifier = DefenseModifier;
		Plan.DefenseParticipatingStamina.Add(Plan.MarkerStamina);
		if (Plan.bHasHelper)
		{
			Plan.DefenseParticipatingStamina.Add(Plan.HelperStamina);
		}

		Plan.LogId = Input.LogId;
		Plan.TurnIndex = Input.TurnIndex;
		Plan.AttackingOwnerId = Participants.AttackingOwnerId;
		Plan.DefendingOwnerId = Participants.DefendingOwnerId;
		Plan.InvolvedCardIds = {
			Plan.CarrierId,
			Plan.RunnerId,
			Plan.MarkerId
		};
		if (Plan.bHasHelper)
		{
			Plan.InvolvedCardIds.Add(Plan.HelperId);
		}

		Plan.bAttackerVictoryRequiresP2 = true;
		Plan.bDefenderVictoryEndsAttack = true;
		return Plan;
	}
}

FThroughBallBehindDefenseP1PlanQueryResult
FThroughBallBehindDefenseP1PlanQuery::Evaluate(
	const FThroughBallBehindDefenseP1PlanQueryInput& Input)
{
	using namespace ThroughBallBehindDefenseP1PlanQuery;

	FThroughBallBehindDefenseP1PlanQueryResult Result;
	Result.Input = Input;

	const FThroughBallParticipantEligibilityQueryResult& Eligibility =
		Input.ParticipantEligibilityResult;
	if (!Eligibility.bSuccess)
	{
		SetFailure(
			Result,
			EThroughBallBehindDefenseP1PlanQueryErrorCode
				::ParticipantEligibilityFailed,
			TEXT("Participant Eligibility must succeed before building the Behind Defense P1 Plan."),
			ParticipantEligibilityResultField);
		return Result;
	}

	if (!IsEligibilitySuccessStateConsistent(Eligibility))
	{
		SetFailure(
			Result,
			EThroughBallBehindDefenseP1PlanQueryErrorCode
				::InvalidParticipantEligibilityResult,
			TEXT("Participant Eligibility success state is inconsistent."),
			ParticipantEligibilityResultField);
		return Result;
	}

	if (Input.SelectedBranch != EThroughBallSelectedBranch::BehindDefense)
	{
		SetFailure(
			Result,
			EThroughBallBehindDefenseP1PlanQueryErrorCode::UnsupportedBranch,
			TEXT("Behind Defense P1 requires the BehindDefense branch."),
			SelectedBranchField);
		return Result;
	}

	if (!Input.bHasAttackD6)
	{
		SetFailure(
			Result,
			EThroughBallBehindDefenseP1PlanQueryErrorCode::MissingAttackD6,
			TEXT("Behind Defense P1 requires an explicitly supplied AttackD6."),
			AttackD6Field);
		return Result;
	}

	if (!IsD6InRange(Input.AttackD6))
	{
		SetFailure(
			Result,
			EThroughBallBehindDefenseP1PlanQueryErrorCode::InvalidAttackD6,
			TEXT("AttackD6 must be in range [1, 6]."),
			AttackD6Field);
		return Result;
	}

	if (!Input.LogId.IsValid())
	{
		SetFailure(
			Result,
			EThroughBallBehindDefenseP1PlanQueryErrorCode::InvalidLogContext,
			TEXT("LogId must be valid."),
			LogIdField);
		return Result;
	}

	if (Input.TurnIndex < 0)
	{
		SetFailure(
			Result,
			EThroughBallBehindDefenseP1PlanQueryErrorCode::InvalidLogContext,
			TEXT("TurnIndex must not be negative."),
			TurnIndexField);
		return Result;
	}

	if (Input.AttackD6 <= OutOfPlayMaxAttackD6)
	{
		Result.bSuccess = true;
		Result.Decision =
			EThroughBallBehindDefenseP1PlanQueryDecision::OutOfPlay;
		Result.bAttackEnded = true;
		return Result;
	}

	if (!Input.bHasDefenseD6)
	{
		SetFailure(
			Result,
			EThroughBallBehindDefenseP1PlanQueryErrorCode::MissingDefenseD6,
			TEXT("Formula resolution requires an explicitly supplied DefenseD6."),
			DefenseD6Field);
		return Result;
	}

	if (!IsD6InRange(Input.DefenseD6))
	{
		SetFailure(
			Result,
			EThroughBallBehindDefenseP1PlanQueryErrorCode::InvalidDefenseD6,
			TEXT("DefenseD6 must be in range [1, 6]."),
			DefenseD6Field);
		return Result;
	}

	Result.FormulaPlan = BuildFormulaPlan(Input);
	Result.bSuccess = true;
	Result.Decision =
		EThroughBallBehindDefenseP1PlanQueryDecision
			::FormulaResolutionRequired;
	Result.bHasFormulaPlan = true;
	return Result;
}
