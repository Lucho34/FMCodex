#include "ThroughBallFeetPlanQuery.h"
#include "PlayerTraitFormula.h"

namespace ThroughBallFeetPlanQuery
{
	const FName ParticipantEligibilityResultField(
		TEXT("ParticipantEligibilityResult"));
	const FName AttackD6Field(TEXT("AttackD6"));
	const FName DefenseD6Field(TEXT("DefenseD6"));
	const FName LogIdField(TEXT("LogId"));
	const FName TurnIndexField(TEXT("TurnIndex"));
	const FName ActiveGoalkeeperSnapshotField(
		TEXT("ActiveGoalkeeperSnapshot"));

	void SetFailure(
		FThroughBallFeetPlanQueryResult& Result,
		const EThroughBallFeetPlanQueryErrorCode ErrorCode,
		const FString& ErrorMessage,
		const FName InvalidField)
	{
		Result.ErrorCode = ErrorCode;
		Result.ErrorMessage = ErrorMessage;
		Result.InvalidField = InvalidField;
	}

	bool IsEligibilitySuccessStateConsistent(
		const FThroughBallParticipantEligibilityQueryResult& Result)
	{
		return Result.bSuccess
			&& Result.ErrorCode
				== EThroughBallParticipantEligibilityQueryErrorCode::None
			&& Result.ErrorMessage.IsEmpty()
			&& Result.InvalidField.IsNone()
			&& Result.bHasHelper == Result.Input.bHasHelper
			&& Result.SkillRuleQueryResult.bSuccess
			&& Result.CarrierSnapshotValidationResult.bSuccess
			&& Result.RunnerSnapshotValidationResult.bSuccess
			&& Result.MarkerSnapshotValidationResult.bSuccess
			&& (!Result.bHasHelper
				|| Result.HelperSnapshotValidationResult.bSuccess);
	}

	FPlayerCardRuleSnapshotValidationResult ValidateSnapshot(
		const FPlayerCardRuleSnapshot& Snapshot)
	{
		FPlayerCardRuleSnapshotSet SnapshotSet;
		SnapshotSet.Cards.Add(Snapshot);
		return FPlayerCardRuleSnapshotValidator::Validate(SnapshotSet);
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

	float HalfOneDecimal(const int32 Value)
	{
		return RoundOneDecimal(static_cast<float>(Value) / 2.0f);
	}

	bool BuildFormulaPlan(
		const FThroughBallFeetPlanQueryInput& Input, FThroughBallFeetFormulaPlan& Plan)
	{
		const FThroughBallParticipantEligibilityQueryResult& Eligibility =
			Input.ParticipantEligibilityResult;
		const FThroughBallParticipantEligibilityQueryInput& Participants =
			Eligibility.Input;

		const FPlayerCardRuleSnapshot& Carrier = Participants.CarrierSnapshot;
		const FPlayerCardRuleSnapshot& Runner = Participants.RunnerSnapshot;
		const FPlayerCardRuleSnapshot& Marker = Participants.MarkerSnapshot;

		using R = FPlayerTraitFormula::Role;
		using A = FPlayerTraitFormula::Attribute;
		const FName Context = TEXT("ThroughBall.Feet");
		const auto C = FPlayerTraitFormula::Resolve(Carrier, Context, R::Carrier, A::Passing);
		const auto Rn = FPlayerTraitFormula::Resolve(Runner, Context, R::Runner, A::Control);
		const auto M = FPlayerTraitFormula::Resolve(Marker, Context, R::Marker, A::Defense);
		const auto H = Eligibility.bHasHelper ? FPlayerTraitFormula::Resolve(Participants.HelperSnapshot, Context, R::Helper, A::Defense) : FPlayerTraitFormulaOperand();
		if (!C.bValid || !Rn.bValid || !M.bValid || (Eligibility.bHasHelper && !H.bValid)) return false;
		Plan.FormulaType = EFormulaType::Finishing;

		Plan.CarrierId = Carrier.CardId;
		Plan.CarrierPassing = C.EffectiveValue;
		Plan.CarrierStamina = PlayerStaminaGameplayValue(Carrier.Attributes.StaminaTier);
		Plan.RunnerId = Runner.CardId;
		Plan.RunnerControl = Rn.EffectiveValue;
		Plan.RunnerStamina = PlayerStaminaGameplayValue(Runner.Attributes.StaminaTier);
		Plan.AttackD6 = Input.AttackD6;
		Plan.AttackBaseValue = AverageOneDecimal(
			Plan.CarrierPassing,
			Plan.RunnerControl);
		Plan.AttackExternalModifier = 0.0f;
		Plan.AttackParticipatingStamina = {
			Plan.CarrierStamina,
			Plan.RunnerStamina
		};

		Plan.MarkerId = Marker.CardId;
		Plan.MarkerDefense = M.EffectiveValue;
		Plan.MarkerStamina = PlayerStaminaGameplayValue(Marker.Attributes.StaminaTier);
		Plan.bHasHelper = Eligibility.bHasHelper;
		if (Plan.bHasHelper)
		{
			const FPlayerCardRuleSnapshot& Helper =
				Participants.HelperSnapshot;
			Plan.HelperId = Helper.CardId;
			Plan.HelperDefense = H.EffectiveValue;
			Plan.HelperStamina = PlayerStaminaGameplayValue(Helper.Attributes.StaminaTier);
		}

		Plan.bHasActiveGoalkeeper = Input.bHasActiveGoalkeeper;
		float GoalkeeperContribution = 0.0f;
		if (Plan.bHasActiveGoalkeeper)
		{
			const FPlayerCardRuleSnapshot& Goalkeeper =
				Input.ActiveGoalkeeperSnapshot;
			Plan.ActiveGoalkeeperId = Goalkeeper.CardId;
			Plan.GoalkeeperOneOnOne =
				Goalkeeper.GoalkeeperAttributes.OneOnOne;
			GoalkeeperContribution =
				HalfOneDecimal(Plan.GoalkeeperOneOnOne);
		}

		Plan.DefenseD6 = Input.DefenseD6;
		Plan.DefenseBaseValue = AverageOneDecimal(
			Plan.MarkerDefense,
			Plan.bHasHelper ? Plan.HelperDefense : 0);
		Plan.DefenseExternalModifier = GoalkeeperContribution + 2.0f;
		Plan.DefenseParticipatingStamina.Add(Plan.MarkerStamina);
		if (Plan.bHasHelper)
		{
			Plan.DefenseParticipatingStamina.Add(Plan.HelperStamina);
		}
		// GK identity/OneOnOne participates, but goalkeepers have no stamina.

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
		if (Plan.bHasActiveGoalkeeper)
		{
			Plan.InvolvedCardIds.Add(Plan.ActiveGoalkeeperId);
		}

		Plan.GoalScorerCardId = Plan.RunnerId;
		Plan.bAttackVictoryIsGoal = true;
		Plan.bDefenderVictoryIsMiss = true;
		Plan.bAttackEndsAfterResolution = true;
		Plan.bContinueResolution = false;
		return true;
	}
}

FThroughBallFeetPlanQueryResult FThroughBallFeetPlanQuery::Evaluate(
	const FThroughBallFeetPlanQueryInput& Input)
{
	using namespace ThroughBallFeetPlanQuery;

	FThroughBallFeetPlanQueryResult Result;
	Result.Input = Input;

	const FThroughBallParticipantEligibilityQueryResult& Eligibility =
		Input.ParticipantEligibilityResult;
	if (!Eligibility.bSuccess)
	{
		SetFailure(
			Result,
			EThroughBallFeetPlanQueryErrorCode::ParticipantEligibilityFailed,
			TEXT("Participant Eligibility must succeed before building the Feet Plan."),
			ParticipantEligibilityResultField);
		return Result;
	}

	if (!IsEligibilitySuccessStateConsistent(Eligibility))
	{
		SetFailure(
			Result,
			EThroughBallFeetPlanQueryErrorCode
				::InvalidParticipantEligibilityResult,
			TEXT("Participant Eligibility success state is inconsistent."),
			ParticipantEligibilityResultField);
		return Result;
	}

	if (Input.AttackD6 < 1 || Input.AttackD6 > 6)
	{
		SetFailure(
			Result,
			EThroughBallFeetPlanQueryErrorCode::InvalidAttackD6,
			TEXT("AttackD6 must be in range [1, 6]."),
			AttackD6Field);
		return Result;
	}

	if (Input.DefenseD6 < 1 || Input.DefenseD6 > 6)
	{
		SetFailure(
			Result,
			EThroughBallFeetPlanQueryErrorCode::InvalidDefenseD6,
			TEXT("DefenseD6 must be in range [1, 6]."),
			DefenseD6Field);
		return Result;
	}

	if (!Input.LogId.IsValid())
	{
		SetFailure(
			Result,
			EThroughBallFeetPlanQueryErrorCode::InvalidLogContext,
			TEXT("LogId must be valid."),
			LogIdField);
		return Result;
	}

	if (Input.TurnIndex < 0)
	{
		SetFailure(
			Result,
			EThroughBallFeetPlanQueryErrorCode::InvalidLogContext,
			TEXT("TurnIndex must not be negative."),
			TurnIndexField);
		return Result;
	}

	if (Input.bHasActiveGoalkeeper)
	{
		const FPlayerCardRuleSnapshot& Goalkeeper =
			Input.ActiveGoalkeeperSnapshot;
		if (Goalkeeper.CardId.IsNone())
		{
			SetFailure(
				Result,
				EThroughBallFeetPlanQueryErrorCode
					::InvalidActiveGoalkeeperIdentity,
				TEXT("Active Goalkeeper CardId must not be None."),
				ActiveGoalkeeperSnapshotField);
			return Result;
		}

		Result.ActiveGoalkeeperValidationResult =
			ValidateSnapshot(Goalkeeper);
		if (!Result.ActiveGoalkeeperValidationResult.bSuccess)
		{
			SetFailure(
				Result,
				EThroughBallFeetPlanQueryErrorCode
					::InvalidActiveGoalkeeperSnapshot,
				Result.ActiveGoalkeeperValidationResult.ErrorMessage,
				ActiveGoalkeeperSnapshotField);
			return Result;
		}

		if (!Goalkeeper.bIsGoalkeeper)
		{
			SetFailure(
				Result,
				EThroughBallFeetPlanQueryErrorCode
					::ActiveGoalkeeperMustBeGoalkeeper,
				TEXT("Active Goalkeeper Snapshot must be a goalkeeper."),
				ActiveGoalkeeperSnapshotField);
			return Result;
		}

		const FThroughBallParticipantEligibilityQueryResult& EligibilityResult =
			Input.ParticipantEligibilityResult;
		if (Goalkeeper.CardId
			== EligibilityResult.Input.MarkerSnapshot.CardId)
		{
			SetFailure(
				Result,
				EThroughBallFeetPlanQueryErrorCode
					::DuplicateDefendingGoalkeeperParticipant,
				TEXT("Active Goalkeeper must differ from Marker."),
				ActiveGoalkeeperSnapshotField);
			return Result;
		}

		if (EligibilityResult.bHasHelper
			&& Goalkeeper.CardId
				== EligibilityResult.Input.HelperSnapshot.CardId)
		{
			SetFailure(
				Result,
				EThroughBallFeetPlanQueryErrorCode
					::DuplicateDefendingGoalkeeperParticipant,
				TEXT("Active Goalkeeper must differ from Helper."),
				ActiveGoalkeeperSnapshotField);
			return Result;
		}
	}

	if (!BuildFormulaPlan(Input, Result.FormulaPlan))
	{
		SetFailure(Result, EThroughBallFeetPlanQueryErrorCode::InvalidTraitConfiguration,
			TEXT("Invalid ranked Trait configuration."), TEXT("RankedTraits"));
		return Result;
	}
	Result.bSuccess = true;
	Result.Decision =
		EThroughBallFeetPlanQueryDecision::FormulaResolutionRequired;
	Result.bHasFormulaPlan = true;
	return Result;
}
