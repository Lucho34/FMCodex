#include "ThroughBallBehindDefenseP1FormulaResolverInputAssembler.h"

namespace ThroughBallBehindDefenseP1FormulaResolverInputAssembler
{
	const FName PlanQueryResultField(TEXT("PlanQueryResult"));
	const FName ErrorCodeField(TEXT("ErrorCode"));
	const FName ErrorMessageField(TEXT("ErrorMessage"));
	const FName InvalidFieldField(TEXT("InvalidField"));
	const FName DecisionField(TEXT("Decision"));
	const FName HasFormulaPlanField(TEXT("bHasFormulaPlan"));
	const FName AttackEndedField(TEXT("bAttackEnded"));
	const FName ContinueResolutionField(TEXT("bContinueResolution"));
	const FName RequiresP2Field(TEXT("bRequiresP2"));
	const FName FormulaTypeField(TEXT("FormulaType"));
	const FName CarrierIdField(TEXT("CarrierId"));
	const FName RunnerIdField(TEXT("RunnerId"));
	const FName MarkerIdField(TEXT("MarkerId"));
	const FName HelperIdField(TEXT("HelperId"));
	const FName HelperSpeedField(TEXT("HelperSpeed"));
	const FName HelperStaminaField(TEXT("HelperStamina"));
	const FName AttackBaseValueField(TEXT("AttackBaseValue"));
	const FName AttackExternalModifierField(TEXT("AttackExternalModifier"));
	const FName AttackD6Field(TEXT("AttackD6"));
	const FName DefenseBaseValueField(TEXT("DefenseBaseValue"));
	const FName DefenseExternalModifierField(TEXT("DefenseExternalModifier"));
	const FName DefenseD6Field(TEXT("DefenseD6"));
	const FName AttackParticipatingStaminaField(TEXT("AttackParticipatingStamina"));
	const FName DefenseParticipatingStaminaField(TEXT("DefenseParticipatingStamina"));
	const FName LogIdField(TEXT("LogId"));
	const FName TurnIndexField(TEXT("TurnIndex"));
	const FName AttackingOwnerIdField(TEXT("AttackingOwnerId"));
	const FName DefendingOwnerIdField(TEXT("DefendingOwnerId"));
	const FName InvolvedCardIdsField(TEXT("InvolvedCardIds"));
	const FName AttackerVictoryRequiresP2Field(TEXT("bAttackerVictoryRequiresP2"));
	const FName DefenderVictoryEndsAttackField(TEXT("bDefenderVictoryEndsAttack"));

	constexpr float AttackModifier = 0.0f;
	constexpr float DefenseModifier = 1.0f;

	void SetFailure(
		FThroughBallBehindDefenseP1FormulaResolverInputAssemblyResult& Result,
		const EThroughBallBehindDefenseP1FormulaResolverInputAssemblyErrorCode ErrorCode,
		const FString& ErrorMessage,
		const FName InvalidField)
	{
		Result.ErrorCode = ErrorCode;
		Result.ErrorMessage = ErrorMessage;
		Result.InvalidField = InvalidField;
	}

	bool IsInRange(const int32 Value, const int32 Minimum, const int32 Maximum)
	{
		return Value >= Minimum && Value <= Maximum;
	}

	TArray<int32> MakeExpectedDefenseStamina(
		const FThroughBallBehindDefenseP1FormulaPlan& Plan)
	{
		TArray<int32> Expected{Plan.MarkerStamina};
		if (Plan.bHasHelper)
		{
			Expected.Add(Plan.HelperStamina);
		}
		return Expected;
	}

	TArray<FName> MakeExpectedInvolvedCardIds(
		const FThroughBallBehindDefenseP1FormulaPlan& Plan)
	{
		TArray<FName> Expected{Plan.CarrierId, Plan.RunnerId, Plan.MarkerId};
		if (Plan.bHasHelper)
		{
			Expected.Add(Plan.HelperId);
		}
		return Expected;
	}

	FFormulaResolverInput MapResolverInput(
		const FThroughBallBehindDefenseP1FormulaPlan& Plan)
	{
		FFormulaResolverInput ResolverInput;
		ResolverInput.FormulaType = Plan.FormulaType;
		ResolverInput.Attacker.BaseValue = Plan.AttackBaseValue;
		ResolverInput.Attacker.Modifier = Plan.AttackExternalModifier;
		ResolverInput.Attacker.ComparePoint = Plan.AttackD6;
		ResolverInput.Attacker.bComparePointWasRolledOnD6 = true;
		ResolverInput.Attacker.ParticipatingStamina =
			Plan.AttackParticipatingStamina;
		ResolverInput.Defender.BaseValue = Plan.DefenseBaseValue;
		ResolverInput.Defender.Modifier = Plan.DefenseExternalModifier;
		ResolverInput.Defender.ComparePoint = Plan.DefenseD6;
		ResolverInput.Defender.bComparePointWasRolledOnD6 = true;
		ResolverInput.Defender.ParticipatingStamina =
			Plan.DefenseParticipatingStamina;
		ResolverInput.bGoalkeeperParticipated = false;
		ResolverInput.LogId = Plan.LogId;
		ResolverInput.TurnIndex = Plan.TurnIndex;
		ResolverInput.AttackerPlayerId = Plan.AttackingOwnerId;
		ResolverInput.DefenderPlayerId = Plan.DefendingOwnerId;
		ResolverInput.InvolvedCardIds = Plan.InvolvedCardIds;
		return ResolverInput;
	}
}

FThroughBallBehindDefenseP1FormulaResolverInputAssemblyResult
FThroughBallBehindDefenseP1FormulaResolverInputAssembler::Assemble(
	const FThroughBallBehindDefenseP1FormulaResolverInputAssemblyInput& Input)
{
	using namespace ThroughBallBehindDefenseP1FormulaResolverInputAssembler;

	FThroughBallBehindDefenseP1FormulaResolverInputAssemblyResult Result;
	Result.Input = Input;

	const FThroughBallBehindDefenseP1PlanQueryResult& PlanResult =
		Input.PlanQueryResult;
	if (!PlanResult.bSuccess)
	{
		SetFailure(
			Result,
			EThroughBallBehindDefenseP1FormulaResolverInputAssemblyErrorCode
				::PlanQueryFailed,
			TEXT("Behind Defense P1 Plan Query must succeed before assembly."),
			PlanQueryResultField);
		return Result;
	}

	if (PlanResult.ErrorCode
		!= EThroughBallBehindDefenseP1PlanQueryErrorCode::None)
	{
		SetFailure(Result,
			EThroughBallBehindDefenseP1FormulaResolverInputAssemblyErrorCode
				::InvalidPlanQueryResult,
			TEXT("Successful Plan Query Result must have ErrorCode None."),
			ErrorCodeField);
		return Result;
	}

	if (!PlanResult.ErrorMessage.IsEmpty())
	{
		SetFailure(Result,
			EThroughBallBehindDefenseP1FormulaResolverInputAssemblyErrorCode
				::InvalidPlanQueryResult,
			TEXT("Successful Plan Query Result must have an empty ErrorMessage."),
			ErrorMessageField);
		return Result;
	}

	if (!PlanResult.InvalidField.IsNone())
	{
		SetFailure(Result,
			EThroughBallBehindDefenseP1FormulaResolverInputAssemblyErrorCode
				::InvalidPlanQueryResult,
			TEXT("Successful Plan Query Result must not identify an invalid field."),
			InvalidFieldField);
		return Result;
	}

	if (PlanResult.Decision
		!= EThroughBallBehindDefenseP1PlanQueryDecision
			::FormulaResolutionRequired)
	{
		SetFailure(Result,
			EThroughBallBehindDefenseP1FormulaResolverInputAssemblyErrorCode
				::UnsupportedPlanQueryDecision,
			TEXT("Assembly requires the FormulaResolutionRequired decision."),
			DecisionField);
		return Result;
	}

	if (!PlanResult.bHasFormulaPlan)
	{
		SetFailure(Result,
			EThroughBallBehindDefenseP1FormulaResolverInputAssemblyErrorCode
				::MissingFormulaPlan,
			TEXT("FormulaResolutionRequired must contain a Formula Plan."),
			HasFormulaPlanField);
		return Result;
	}

	if (PlanResult.bAttackEnded)
	{
		SetFailure(Result,
			EThroughBallBehindDefenseP1FormulaResolverInputAssemblyErrorCode
				::InvalidPlanQueryResult,
			TEXT("Formula path must not have ended the attack before execution."),
			AttackEndedField);
		return Result;
	}

	if (PlanResult.bContinueResolution)
	{
		SetFailure(Result,
			EThroughBallBehindDefenseP1FormulaResolverInputAssemblyErrorCode
				::InvalidPlanQueryResult,
			TEXT("Formula path must not continue before execution."),
			ContinueResolutionField);
		return Result;
	}

	if (PlanResult.bRequiresP2)
	{
		SetFailure(Result,
			EThroughBallBehindDefenseP1FormulaResolverInputAssemblyErrorCode
				::InvalidPlanQueryResult,
			TEXT("Formula path must not require P2 before execution."),
			RequiresP2Field);
		return Result;
	}

	const FThroughBallBehindDefenseP1FormulaPlan& Plan = PlanResult.FormulaPlan;
	if (Plan.FormulaType != EFormulaType::Transition)
	{
		SetFailure(Result,
			EThroughBallBehindDefenseP1FormulaResolverInputAssemblyErrorCode
				::UnsupportedFormulaType,
			TEXT("Behind Defense P1 Formula Plan must use Transition."),
			FormulaTypeField);
		return Result;
	}

	if (Plan.CarrierId.IsNone())
	{
		SetFailure(Result,
			EThroughBallBehindDefenseP1FormulaResolverInputAssemblyErrorCode
				::InvalidRequiredParticipantIdentity,
			TEXT("CarrierId must not be None."), CarrierIdField);
		return Result;
	}

	if (Plan.RunnerId.IsNone())
	{
		SetFailure(Result,
			EThroughBallBehindDefenseP1FormulaResolverInputAssemblyErrorCode
				::InvalidRequiredParticipantIdentity,
			TEXT("RunnerId must not be None."), RunnerIdField);
		return Result;
	}

	if (Plan.MarkerId.IsNone())
	{
		SetFailure(Result,
			EThroughBallBehindDefenseP1FormulaResolverInputAssemblyErrorCode
				::InvalidRequiredParticipantIdentity,
			TEXT("MarkerId must not be None."), MarkerIdField);
		return Result;
	}

	if (!Plan.bHasHelper)
	{
		if (!Plan.HelperId.IsNone())
		{
			SetFailure(Result,
				EThroughBallBehindDefenseP1FormulaResolverInputAssemblyErrorCode
					::InvalidOptionalParticipantState,
				TEXT("Absent Helper must keep HelperId at None."), HelperIdField);
			return Result;
		}
		if (Plan.HelperSpeed != 0)
		{
			SetFailure(Result,
				EThroughBallBehindDefenseP1FormulaResolverInputAssemblyErrorCode
					::InvalidOptionalParticipantState,
				TEXT("Absent Helper must keep HelperSpeed at zero."), HelperSpeedField);
			return Result;
		}
		if (Plan.HelperStamina != 0)
		{
			SetFailure(Result,
				EThroughBallBehindDefenseP1FormulaResolverInputAssemblyErrorCode
					::InvalidOptionalParticipantState,
				TEXT("Absent Helper must keep HelperStamina at zero."), HelperStaminaField);
			return Result;
		}
	}
	else if (Plan.HelperId.IsNone())
	{
		SetFailure(Result,
			EThroughBallBehindDefenseP1FormulaResolverInputAssemblyErrorCode
				::InvalidOptionalParticipantState,
			TEXT("Present Helper must have a valid HelperId."), HelperIdField);
		return Result;
	}

	if (!FMath::IsFinite(Plan.AttackBaseValue))
	{
		SetFailure(Result,
			EThroughBallBehindDefenseP1FormulaResolverInputAssemblyErrorCode
				::InvalidAttackFormulaData,
			TEXT("AttackBaseValue must be finite."), AttackBaseValueField);
		return Result;
	}

	if (!FMath::IsFinite(Plan.AttackExternalModifier)
		|| Plan.AttackExternalModifier != AttackModifier)
	{
		SetFailure(Result,
			EThroughBallBehindDefenseP1FormulaResolverInputAssemblyErrorCode
				::InvalidAttackFormulaData,
			TEXT("AttackExternalModifier must be finite and equal zero."),
			AttackExternalModifierField);
		return Result;
	}

	if (!IsInRange(Plan.AttackD6, 3, 6))
	{
		SetFailure(Result,
			EThroughBallBehindDefenseP1FormulaResolverInputAssemblyErrorCode
				::InvalidAttackFormulaData,
			TEXT("Formula-path AttackD6 must be in range [3, 6]."), AttackD6Field);
		return Result;
	}

	if (!FMath::IsFinite(Plan.DefenseBaseValue))
	{
		SetFailure(Result,
			EThroughBallBehindDefenseP1FormulaResolverInputAssemblyErrorCode
				::InvalidDefenseFormulaData,
			TEXT("DefenseBaseValue must be finite."), DefenseBaseValueField);
		return Result;
	}

	if (!FMath::IsFinite(Plan.DefenseExternalModifier)
		|| Plan.DefenseExternalModifier != DefenseModifier)
	{
		SetFailure(Result,
			EThroughBallBehindDefenseP1FormulaResolverInputAssemblyErrorCode
				::InvalidDefenseFormulaData,
			TEXT("DefenseExternalModifier must be finite and equal one."),
			DefenseExternalModifierField);
		return Result;
	}

	if (!IsInRange(Plan.DefenseD6, 1, 6))
	{
		SetFailure(Result,
			EThroughBallBehindDefenseP1FormulaResolverInputAssemblyErrorCode
				::InvalidDefenseFormulaData,
			TEXT("DefenseD6 must be in range [1, 6]."), DefenseD6Field);
		return Result;
	}

	if (Plan.AttackParticipatingStamina
		!= TArray<int32>{Plan.CarrierStamina, Plan.RunnerStamina})
	{
		SetFailure(Result,
			EThroughBallBehindDefenseP1FormulaResolverInputAssemblyErrorCode
				::InvalidAttackParticipatingStamina,
			TEXT("Attack stamina must contain Carrier then Runner stamina."),
			AttackParticipatingStaminaField);
		return Result;
	}

	if (Plan.DefenseParticipatingStamina != MakeExpectedDefenseStamina(Plan))
	{
		SetFailure(Result,
			EThroughBallBehindDefenseP1FormulaResolverInputAssemblyErrorCode
				::InvalidDefenseParticipatingStamina,
			TEXT("Defense stamina must contain Marker then optional Helper stamina."),
			DefenseParticipatingStaminaField);
		return Result;
	}

	if (!Plan.LogId.IsValid())
	{
		SetFailure(Result,
			EThroughBallBehindDefenseP1FormulaResolverInputAssemblyErrorCode
				::InvalidLogContext,
			TEXT("LogId must be valid."), LogIdField);
		return Result;
	}

	if (Plan.TurnIndex < 0)
	{
		SetFailure(Result,
			EThroughBallBehindDefenseP1FormulaResolverInputAssemblyErrorCode
				::InvalidLogContext,
			TEXT("TurnIndex must not be negative."), TurnIndexField);
		return Result;
	}

	if (Plan.AttackingOwnerId.IsNone())
	{
		SetFailure(Result,
			EThroughBallBehindDefenseP1FormulaResolverInputAssemblyErrorCode
				::InvalidOwnerIdentity,
			TEXT("AttackingOwnerId must not be None."), AttackingOwnerIdField);
		return Result;
	}

	if (Plan.DefendingOwnerId.IsNone())
	{
		SetFailure(Result,
			EThroughBallBehindDefenseP1FormulaResolverInputAssemblyErrorCode
				::InvalidOwnerIdentity,
			TEXT("DefendingOwnerId must not be None."), DefendingOwnerIdField);
		return Result;
	}

	if (Plan.AttackingOwnerId == Plan.DefendingOwnerId)
	{
		SetFailure(Result,
			EThroughBallBehindDefenseP1FormulaResolverInputAssemblyErrorCode
				::InvalidOwnerIdentity,
			TEXT("Attacking and Defending Owner identities must differ."),
			DefendingOwnerIdField);
		return Result;
	}

	if (Plan.InvolvedCardIds != MakeExpectedInvolvedCardIds(Plan))
	{
		SetFailure(Result,
			EThroughBallBehindDefenseP1FormulaResolverInputAssemblyErrorCode
				::InvalidInvolvedCardIds,
			TEXT("InvolvedCardIds must preserve the P1 participant order."),
			InvolvedCardIdsField);
		return Result;
	}

	if (!Plan.bAttackerVictoryRequiresP2)
	{
		SetFailure(Result,
			EThroughBallBehindDefenseP1FormulaResolverInputAssemblyErrorCode
				::InvalidWinnerMetadata,
			TEXT("Attacker victory must retain P2-required metadata."),
			AttackerVictoryRequiresP2Field);
		return Result;
	}

	if (!Plan.bDefenderVictoryEndsAttack)
	{
		SetFailure(Result,
			EThroughBallBehindDefenseP1FormulaResolverInputAssemblyErrorCode
				::InvalidWinnerMetadata,
			TEXT("Defender victory must retain attack-end metadata."),
			DefenderVictoryEndsAttackField);
		return Result;
	}

	Result.ResolverInput = MapResolverInput(Plan);
	Result.bHasResolverInput = true;
	Result.bSuccess = true;
	return Result;
}
