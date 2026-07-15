#include "ThroughBallFeetFormulaResolverInputAssembler.h"

namespace ThroughBallFeetFormulaResolverInputAssembler
{
	const FName FormulaTypeField(TEXT("FormulaType"));
	const FName CarrierIdField(TEXT("CarrierId"));
	const FName RunnerIdField(TEXT("RunnerId"));
	const FName MarkerIdField(TEXT("MarkerId"));
	const FName HelperIdField(TEXT("HelperId"));
	const FName HelperMarkingField(TEXT("HelperMarking"));
	const FName HelperStaminaField(TEXT("HelperStamina"));
	const FName ActiveGoalkeeperIdField(TEXT("ActiveGoalkeeperId"));
	const FName GoalkeeperOneOnOneField(TEXT("GoalkeeperOneOnOne"));
	const FName GoalkeeperStaminaField(TEXT("GoalkeeperStamina"));
	const FName AttackBaseValueField(TEXT("AttackBaseValue"));
	const FName AttackExternalModifierField(TEXT("AttackExternalModifier"));
	const FName AttackD6Field(TEXT("AttackD6"));
	const FName DefenseBaseValueField(TEXT("DefenseBaseValue"));
	const FName DefenseExternalModifierField(TEXT("DefenseExternalModifier"));
	const FName DefenseD6Field(TEXT("DefenseD6"));
	const FName AttackParticipatingStaminaField(
		TEXT("AttackParticipatingStamina"));
	const FName DefenseParticipatingStaminaField(
		TEXT("DefenseParticipatingStamina"));
	const FName LogIdField(TEXT("LogId"));
	const FName TurnIndexField(TEXT("TurnIndex"));
	const FName AttackingOwnerIdField(TEXT("AttackingOwnerId"));
	const FName DefendingOwnerIdField(TEXT("DefendingOwnerId"));
	const FName InvolvedCardIdsField(TEXT("InvolvedCardIds"));
	const FName GoalScorerCardIdField(TEXT("GoalScorerCardId"));
	const FName AttackVictoryIsGoalField(TEXT("bAttackVictoryIsGoal"));
	const FName DefenderVictoryIsMissField(TEXT("bDefenderVictoryIsMiss"));
	const FName AttackEndsAfterResolutionField(
		TEXT("bAttackEndsAfterResolution"));
	const FName ContinueResolutionField(TEXT("bContinueResolution"));

	void SetFailure(
		FThroughBallFeetFormulaResolverInputAssemblyResult& Result,
		const EThroughBallFeetFormulaResolverInputAssemblyErrorCode ErrorCode,
		const FString& ErrorMessage,
		const FName InvalidField)
	{
		Result.ErrorCode = ErrorCode;
		Result.ErrorMessage = ErrorMessage;
		Result.InvalidField = InvalidField;
	}

	bool IsD6(const int32 Value)
	{
		return Value >= 1 && Value <= 6;
	}

	TArray<int32> MakeExpectedDefenseStamina(
		const FThroughBallFeetFormulaPlan& Plan)
	{
		TArray<int32> Expected{Plan.MarkerStamina};
		if (Plan.bHasHelper)
		{
			Expected.Add(Plan.HelperStamina);
		}
		if (Plan.bHasActiveGoalkeeper)
		{
			Expected.Add(Plan.GoalkeeperStamina);
		}
		return Expected;
	}

	TArray<FName> MakeExpectedInvolvedCardIds(
		const FThroughBallFeetFormulaPlan& Plan)
	{
		TArray<FName> Expected{Plan.CarrierId, Plan.RunnerId, Plan.MarkerId};
		if (Plan.bHasHelper)
		{
			Expected.Add(Plan.HelperId);
		}
		if (Plan.bHasActiveGoalkeeper)
		{
			Expected.Add(Plan.ActiveGoalkeeperId);
		}
		return Expected;
	}

	FFormulaResolverInput MapResolverInput(
		const FThroughBallFeetFormulaPlan& Plan)
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
		ResolverInput.bGoalkeeperParticipated = Plan.bHasActiveGoalkeeper;
		ResolverInput.LogId = Plan.LogId;
		ResolverInput.TurnIndex = Plan.TurnIndex;
		ResolverInput.AttackerPlayerId = Plan.AttackingOwnerId;
		ResolverInput.DefenderPlayerId = Plan.DefendingOwnerId;
		ResolverInput.InvolvedCardIds = Plan.InvolvedCardIds;
		return ResolverInput;
	}
}

FThroughBallFeetFormulaResolverInputAssemblyResult
FThroughBallFeetFormulaResolverInputAssembler::Assemble(
	const FThroughBallFeetFormulaResolverInputAssemblyInput& Input)
{
	using namespace ThroughBallFeetFormulaResolverInputAssembler;

	FThroughBallFeetFormulaResolverInputAssemblyResult Result;
	Result.Input = Input;

	const FThroughBallFeetFormulaPlan& Plan = Input.FormulaPlan;
	if (Plan.FormulaType != EFormulaType::Finishing)
	{
		SetFailure(
			Result,
			EThroughBallFeetFormulaResolverInputAssemblyErrorCode
				::UnsupportedFormulaType,
			TEXT("Through Ball Feet Formula Plan must use Finishing."),
			FormulaTypeField);
		return Result;
	}

	if (Plan.CarrierId.IsNone())
	{
		SetFailure(
			Result,
			EThroughBallFeetFormulaResolverInputAssemblyErrorCode
				::InvalidRequiredParticipantIdentity,
			TEXT("CarrierId must not be None."),
			CarrierIdField);
		return Result;
	}

	if (Plan.RunnerId.IsNone())
	{
		SetFailure(
			Result,
			EThroughBallFeetFormulaResolverInputAssemblyErrorCode
				::InvalidRequiredParticipantIdentity,
			TEXT("RunnerId must not be None."),
			RunnerIdField);
		return Result;
	}

	if (Plan.MarkerId.IsNone())
	{
		SetFailure(
			Result,
			EThroughBallFeetFormulaResolverInputAssemblyErrorCode
				::InvalidRequiredParticipantIdentity,
			TEXT("MarkerId must not be None."),
			MarkerIdField);
		return Result;
	}

	if (!Plan.bHasHelper)
	{
		if (!Plan.HelperId.IsNone())
		{
			SetFailure(
				Result,
				EThroughBallFeetFormulaResolverInputAssemblyErrorCode
					::InvalidOptionalParticipantState,
				TEXT("Absent Helper must keep HelperId at None."),
				HelperIdField);
			return Result;
		}
		if (Plan.HelperMarking != 0)
		{
			SetFailure(
				Result,
				EThroughBallFeetFormulaResolverInputAssemblyErrorCode
					::InvalidOptionalParticipantState,
				TEXT("Absent Helper must keep HelperMarking at zero."),
				HelperMarkingField);
			return Result;
		}
		if (Plan.HelperStamina != 0)
		{
			SetFailure(
				Result,
				EThroughBallFeetFormulaResolverInputAssemblyErrorCode
					::InvalidOptionalParticipantState,
				TEXT("Absent Helper must keep HelperStamina at zero."),
				HelperStaminaField);
			return Result;
		}
	}
	else if (Plan.HelperId.IsNone())
	{
		SetFailure(
			Result,
			EThroughBallFeetFormulaResolverInputAssemblyErrorCode
				::InvalidOptionalParticipantState,
			TEXT("Present Helper must have a valid HelperId."),
			HelperIdField);
		return Result;
	}

	if (!Plan.bHasActiveGoalkeeper)
	{
		if (!Plan.ActiveGoalkeeperId.IsNone())
		{
			SetFailure(
				Result,
				EThroughBallFeetFormulaResolverInputAssemblyErrorCode
					::InvalidOptionalParticipantState,
				TEXT("Absent Active Goalkeeper must keep its identity at None."),
				ActiveGoalkeeperIdField);
			return Result;
		}
		if (Plan.GoalkeeperOneOnOne != 0)
		{
			SetFailure(
				Result,
				EThroughBallFeetFormulaResolverInputAssemblyErrorCode
					::InvalidOptionalParticipantState,
				TEXT("Absent Active Goalkeeper must keep OneOnOne at zero."),
				GoalkeeperOneOnOneField);
			return Result;
		}
		if (Plan.GoalkeeperStamina != 0)
		{
			SetFailure(
				Result,
				EThroughBallFeetFormulaResolverInputAssemblyErrorCode
					::InvalidOptionalParticipantState,
				TEXT("Absent Active Goalkeeper must keep stamina at zero."),
				GoalkeeperStaminaField);
			return Result;
		}
	}
	else if (Plan.ActiveGoalkeeperId.IsNone())
	{
		SetFailure(
			Result,
			EThroughBallFeetFormulaResolverInputAssemblyErrorCode
				::InvalidOptionalParticipantState,
			TEXT("Present Active Goalkeeper must have a valid identity."),
			ActiveGoalkeeperIdField);
		return Result;
	}

	if (!FMath::IsFinite(Plan.AttackBaseValue))
	{
		SetFailure(
			Result,
			EThroughBallFeetFormulaResolverInputAssemblyErrorCode
				::InvalidAttackFormulaData,
			TEXT("AttackBaseValue must be finite."),
			AttackBaseValueField);
		return Result;
	}

	if (!FMath::IsFinite(Plan.AttackExternalModifier))
	{
		SetFailure(
			Result,
			EThroughBallFeetFormulaResolverInputAssemblyErrorCode
				::InvalidAttackFormulaData,
			TEXT("AttackExternalModifier must be finite."),
			AttackExternalModifierField);
		return Result;
	}

	if (!IsD6(Plan.AttackD6))
	{
		SetFailure(
			Result,
			EThroughBallFeetFormulaResolverInputAssemblyErrorCode
				::InvalidAttackFormulaData,
			TEXT("AttackD6 must be in range [1, 6]."),
			AttackD6Field);
		return Result;
	}

	if (!FMath::IsFinite(Plan.DefenseBaseValue))
	{
		SetFailure(
			Result,
			EThroughBallFeetFormulaResolverInputAssemblyErrorCode
				::InvalidDefenseFormulaData,
			TEXT("DefenseBaseValue must be finite."),
			DefenseBaseValueField);
		return Result;
	}

	if (!FMath::IsFinite(Plan.DefenseExternalModifier))
	{
		SetFailure(
			Result,
			EThroughBallFeetFormulaResolverInputAssemblyErrorCode
				::InvalidDefenseFormulaData,
			TEXT("DefenseExternalModifier must be finite."),
			DefenseExternalModifierField);
		return Result;
	}

	if (!IsD6(Plan.DefenseD6))
	{
		SetFailure(
			Result,
			EThroughBallFeetFormulaResolverInputAssemblyErrorCode
				::InvalidDefenseFormulaData,
			TEXT("DefenseD6 must be in range [1, 6]."),
			DefenseD6Field);
		return Result;
	}

	if (Plan.AttackParticipatingStamina
		!= TArray<int32>{Plan.CarrierStamina, Plan.RunnerStamina})
	{
		SetFailure(
			Result,
			EThroughBallFeetFormulaResolverInputAssemblyErrorCode
				::InvalidAttackParticipatingStamina,
			TEXT("Attack stamina must contain Carrier then Runner stamina."),
			AttackParticipatingStaminaField);
		return Result;
	}

	if (Plan.DefenseParticipatingStamina
		!= MakeExpectedDefenseStamina(Plan))
	{
		SetFailure(
			Result,
			EThroughBallFeetFormulaResolverInputAssemblyErrorCode
				::InvalidDefenseParticipatingStamina,
			TEXT("Defense stamina must contain Marker, optional Helper, then optional Active Goalkeeper stamina."),
			DefenseParticipatingStaminaField);
		return Result;
	}

	if (!Plan.LogId.IsValid())
	{
		SetFailure(
			Result,
			EThroughBallFeetFormulaResolverInputAssemblyErrorCode
				::InvalidLogContext,
			TEXT("LogId must be valid."),
			LogIdField);
		return Result;
	}

	if (Plan.TurnIndex < 0)
	{
		SetFailure(
			Result,
			EThroughBallFeetFormulaResolverInputAssemblyErrorCode
				::InvalidLogContext,
			TEXT("TurnIndex must not be negative."),
			TurnIndexField);
		return Result;
	}

	if (Plan.AttackingOwnerId.IsNone())
	{
		SetFailure(
			Result,
			EThroughBallFeetFormulaResolverInputAssemblyErrorCode
				::InvalidOwnerIdentity,
			TEXT("AttackingOwnerId must not be None."),
			AttackingOwnerIdField);
		return Result;
	}

	if (Plan.DefendingOwnerId.IsNone())
	{
		SetFailure(
			Result,
			EThroughBallFeetFormulaResolverInputAssemblyErrorCode
				::InvalidOwnerIdentity,
			TEXT("DefendingOwnerId must not be None."),
			DefendingOwnerIdField);
		return Result;
	}

	if (Plan.AttackingOwnerId == Plan.DefendingOwnerId)
	{
		SetFailure(
			Result,
			EThroughBallFeetFormulaResolverInputAssemblyErrorCode
				::InvalidOwnerIdentity,
			TEXT("Attacking and Defending Owner identities must differ."),
			DefendingOwnerIdField);
		return Result;
	}

	if (Plan.InvolvedCardIds != MakeExpectedInvolvedCardIds(Plan))
	{
		SetFailure(
			Result,
			EThroughBallFeetFormulaResolverInputAssemblyErrorCode
				::InvalidInvolvedCardIds,
			TEXT("InvolvedCardIds must preserve the Feet participant order."),
			InvolvedCardIdsField);
		return Result;
	}

	if (Plan.GoalScorerCardId.IsNone()
		|| Plan.GoalScorerCardId != Plan.RunnerId)
	{
		SetFailure(
			Result,
			EThroughBallFeetFormulaResolverInputAssemblyErrorCode
				::InvalidGoalScorerIdentity,
			TEXT("GoalScorerCardId must identify the Runner."),
			GoalScorerCardIdField);
		return Result;
	}

	if (!Plan.bAttackVictoryIsGoal)
	{
		SetFailure(
			Result,
			EThroughBallFeetFormulaResolverInputAssemblyErrorCode
				::InvalidTerminalMetadata,
			TEXT("Attack victory must retain Goal metadata."),
			AttackVictoryIsGoalField);
		return Result;
	}

	if (!Plan.bDefenderVictoryIsMiss)
	{
		SetFailure(
			Result,
			EThroughBallFeetFormulaResolverInputAssemblyErrorCode
				::InvalidTerminalMetadata,
			TEXT("Defender victory must retain Miss metadata."),
			DefenderVictoryIsMissField);
		return Result;
	}

	if (!Plan.bAttackEndsAfterResolution)
	{
		SetFailure(
			Result,
			EThroughBallFeetFormulaResolverInputAssemblyErrorCode
				::InvalidTerminalMetadata,
			TEXT("Feet resolution must retain attack-end metadata."),
			AttackEndsAfterResolutionField);
		return Result;
	}

	if (Plan.bContinueResolution)
	{
		SetFailure(
			Result,
			EThroughBallFeetFormulaResolverInputAssemblyErrorCode
				::InvalidTerminalMetadata,
			TEXT("Feet resolution must not retain continuation metadata."),
			ContinueResolutionField);
		return Result;
	}

	Result.ResolverInput = MapResolverInput(Plan);
	Result.bSuccess = true;
	Result.bHasResolverInput = true;
	return Result;
}
