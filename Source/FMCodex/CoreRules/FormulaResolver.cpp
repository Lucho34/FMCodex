#include "FormulaResolver.h"

namespace FormulaResolver
{
	const FName FormulaResolvedEventType(TEXT("FormulaResolved"));
	const FName AttackerRole(TEXT("Attacker"));
	const FName DefenderRole(TEXT("Defender"));

	int32 SumStamina(const TArray<int32>& StaminaValues)
	{
		int32 Total = 0;
		for (const int32 Stamina : StaminaValues)
		{
			Total += Stamina;
		}
		return Total;
	}

	bool IsD6Result(const int32 Value)
	{
		return Value >= 1 && Value <= 6;
	}

	bool IsLowSuppressedRoll(const int32 Value)
	{
		return Value == 1 || Value == 2;
	}

	const TCHAR* WinnerToText(const EFormulaWinner Winner)
	{
		switch (Winner)
		{
		case EFormulaWinner::Attacker:
			return TEXT("Attacker");
		case EFormulaWinner::Defender:
			return TEXT("Defender");
		default:
			return TEXT("None");
		}
	}

	const TCHAR* WinReasonToText(const EFormulaWinReason WinReason)
	{
		switch (WinReason)
		{
		case EFormulaWinReason::HigherFinalValue:
			return TEXT("HigherFinalValue");
		case EFormulaWinReason::StaminaTieBreaker:
			return TEXT("StaminaTieBreaker");
		case EFormulaWinReason::DefenderWinsEqualStamina:
			return TEXT("DefenderWinsEqualStamina");
		case EFormulaWinReason::DefenderWinsGoalkeeperTie:
			return TEXT("DefenderWinsGoalkeeperTie");
		case EFormulaWinReason::FastSuppression:
			return TEXT("FastSuppression");
		default:
			return TEXT("None");
		}
	}

	EFormulaWinner ResolveFastSuppression(
		const FFormulaSideInput& Attacker,
		const FFormulaSideInput& Defender)
	{
		if (!Attacker.bComparePointWasRolledOnD6
			|| !Defender.bComparePointWasRolledOnD6
			|| !IsD6Result(Attacker.ComparePoint)
			|| !IsD6Result(Defender.ComparePoint))
		{
			return EFormulaWinner::None;
		}

		if (Attacker.ComparePoint == 6 && IsLowSuppressedRoll(Defender.ComparePoint))
		{
			return EFormulaWinner::Attacker;
		}

		if (Defender.ComparePoint == 6 && IsLowSuppressedRoll(Attacker.ComparePoint))
		{
			return EFormulaWinner::Defender;
		}

		return EFormulaWinner::None;
	}

	void ResolveTie(const FFormulaResolverInput& Input, FFormulaResolutionResult& Result)
	{
		if (Input.bGoalkeeperParticipated)
		{
			Result.Winner = EFormulaWinner::Defender;
			Result.WinReason = EFormulaWinReason::DefenderWinsGoalkeeperTie;
			return;
		}

		const int32 AttackerStamina = SumStamina(Input.Attacker.ParticipatingStamina);
		const int32 DefenderStamina = SumStamina(Input.Defender.ParticipatingStamina);
		if (AttackerStamina > DefenderStamina)
		{
			Result.Winner = EFormulaWinner::Attacker;
			Result.WinReason = EFormulaWinReason::StaminaTieBreaker;
		}
		else if (DefenderStamina > AttackerStamina)
		{
			Result.Winner = EFormulaWinner::Defender;
			Result.WinReason = EFormulaWinReason::StaminaTieBreaker;
		}
		else
		{
			Result.Winner = EFormulaWinner::Defender;
			Result.WinReason = EFormulaWinReason::DefenderWinsEqualStamina;
		}
	}

	void ApplyFormulaOutcome(FFormulaResolutionResult& Result)
	{
		if (Result.FormulaType == EFormulaType::Transition)
		{
			Result.bContinueResolution = Result.Winner == EFormulaWinner::Attacker;
			Result.bAttackEnded = Result.Winner == EFormulaWinner::Defender;
			return;
		}

		if (Result.FormulaType == EFormulaType::Finishing)
		{
			Result.bIsGoal = Result.Winner == EFormulaWinner::Attacker;
			Result.bAttackEnded = Result.Winner != EFormulaWinner::None;
			Result.bContinueResolution = false;
		}
	}

	void BuildMatchLog(
		const FFormulaResolverInput& Input,
		const int32 AttackerStamina,
		const int32 DefenderStamina,
		FFormulaResolutionResult& Result)
	{
		FMatchLogEntry& Log = Result.MatchLogEntry;
		Log.LogId = Input.LogId;
		Log.TurnIndex = Input.TurnIndex;
		Log.EventType = FormulaResolvedEventType;
		Log.ActingPlayerId = Input.AttackerPlayerId;
		Log.InvolvedCardIds = Input.InvolvedCardIds;
		Log.FormulaType = Input.FormulaType;

		if (Input.Attacker.bComparePointWasRolledOnD6)
		{
			Log.DiceResults.Add(Input.Attacker.ComparePoint);
			Log.DiceOrder.Add(Input.AttackerPlayerId.IsNone() ? AttackerRole : Input.AttackerPlayerId);
		}
		if (Input.Defender.bComparePointWasRolledOnD6)
		{
			Log.DiceResults.Add(Input.Defender.ComparePoint);
			Log.DiceOrder.Add(Input.DefenderPlayerId.IsNone() ? DefenderRole : Input.DefenderPlayerId);
		}

		Log.FormulaInputs = FString::Printf(
			TEXT("AttackerBase=%.1f; AttackerModifier=%.1f; AttackerComparePoint=%d; ")
			TEXT("DefenderBase=%.1f; DefenderModifier=%.1f; DefenderComparePoint=%d; ")
			TEXT("AttackerStamina=%d; DefenderStamina=%d; GoalkeeperParticipated=%s"),
			Input.Attacker.BaseValue,
			Input.Attacker.Modifier,
			Input.Attacker.ComparePoint,
			Input.Defender.BaseValue,
			Input.Defender.Modifier,
			Input.Defender.ComparePoint,
			AttackerStamina,
			DefenderStamina,
			Input.bGoalkeeperParticipated ? TEXT("true") : TEXT("false"));

		Log.FormulaResult = FString::Printf(
			TEXT("AttackerFinal=%.1f; DefenderFinal=%.1f; Winner=%s; WinReason=%s; Goal=%s; ")
			TEXT("AttackEnded=%s; ContinueResolution=%s; FastSuppression=%s"),
			Result.AttackerFinalValue,
			Result.DefenderFinalValue,
			WinnerToText(Result.Winner),
			WinReasonToText(Result.WinReason),
			Result.bIsGoal ? TEXT("true") : TEXT("false"),
			Result.bAttackEnded ? TEXT("true") : TEXT("false"),
			Result.bContinueResolution ? TEXT("true") : TEXT("false"),
			Result.bFastSuppressionTriggered ? TEXT("true") : TEXT("false"));
	}
}

FFormulaResolutionResult UFormulaResolver::ResolveFormula(const FFormulaResolverInput& Input)
{
	FFormulaResolutionResult Result;
	Result.FormulaType = Input.FormulaType;
	Result.AttackerFinalValue = RoundToOneDecimal(
		Input.Attacker.BaseValue + Input.Attacker.Modifier + Input.Attacker.ComparePoint);
	Result.DefenderFinalValue = RoundToOneDecimal(
		Input.Defender.BaseValue + Input.Defender.Modifier + Input.Defender.ComparePoint);

	const bool bIsSupportedFormula = Input.FormulaType == EFormulaType::Transition
		|| Input.FormulaType == EFormulaType::Finishing;

	if (bIsSupportedFormula)
	{
		Result.Winner = FormulaResolver::ResolveFastSuppression(Input.Attacker, Input.Defender);
		Result.bFastSuppressionTriggered = Result.Winner != EFormulaWinner::None;
		if (Result.bFastSuppressionTriggered)
		{
			Result.WinReason = EFormulaWinReason::FastSuppression;
		}

		if (!Result.bFastSuppressionTriggered)
		{
			if (Result.AttackerFinalValue > Result.DefenderFinalValue)
			{
				Result.Winner = EFormulaWinner::Attacker;
				Result.WinReason = EFormulaWinReason::HigherFinalValue;
			}
			else if (Result.DefenderFinalValue > Result.AttackerFinalValue)
			{
				Result.Winner = EFormulaWinner::Defender;
				Result.WinReason = EFormulaWinReason::HigherFinalValue;
			}
			else
			{
				FormulaResolver::ResolveTie(Input, Result);
			}
		}

		FormulaResolver::ApplyFormulaOutcome(Result);
	}

	const int32 AttackerStamina = FormulaResolver::SumStamina(Input.Attacker.ParticipatingStamina);
	const int32 DefenderStamina = FormulaResolver::SumStamina(Input.Defender.ParticipatingStamina);
	FormulaResolver::BuildMatchLog(Input, AttackerStamina, DefenderStamina, Result);
	return Result;
}

float UFormulaResolver::RoundToOneDecimal(const float Value)
{
	return FMath::RoundToFloat(Value * 10.0f) / 10.0f;
}

float UFormulaResolver::DivideByTwo(const float Value)
{
	return RoundToOneDecimal(Value / 2.0f);
}

float UFormulaResolver::Average(const float FirstValue, const float SecondValue)
{
	return DivideByTwo(FirstValue + SecondValue);
}

float UFormulaResolver::CalculateGoalkeeperHalf(const float GoalkeeperAttribute)
{
	return DivideByTwo(GoalkeeperAttribute);
}
