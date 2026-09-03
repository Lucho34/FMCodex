#include "MatchPlayCurrentAttackRouteStateValidator.h"
#include "MatchPlayCornerResolution.h"

#include "MatchPlaySetPieceParticipantEligibility.h"
#include "MatchPlayDefendingGoalkeeperQuery.h"
#include "SetPieceTypeSelectionQuery.h"
#include "SingleCardFormulaResolutionExecutor.h"

namespace MatchPlayCurrentAttackRouteStateValidator
{
	void SetFailure(
		FMatchPlayCurrentAttackRouteStateValidationResult& Result,
		const EMatchPlayCurrentAttackRouteStateValidationErrorCode ErrorCode,
		const FString& ErrorMessage)
	{
		Result.ErrorCode = ErrorCode;
		Result.ErrorMessage = ErrorMessage;
	}

	bool IsDefaultSendingOffState(
		const FMatchPlaySendingOffRouteState& State)
	{
		const FMatchPlaySendingOffRouteState DefaultState;
		return FMatchPlaySendingOffRouteState::StaticStruct()
			->CompareScriptStruct(&State, &DefaultState, 0);
	}

	bool IsDefaultSetPieceState(
		const FMatchPlaySetPieceRouteState& State)
	{
		const FMatchPlaySetPieceRouteState DefaultState;
		return FMatchPlaySetPieceRouteState::StaticStruct()
			->CompareScriptStruct(&State, &DefaultState, 0);
	}

	template <typename TStruct>
	bool IsDefaultStruct(const TStruct& State)
	{
		const TStruct DefaultState;
		return TStruct::StaticStruct()->CompareScriptStruct(
			&State,
			&DefaultState,
			0);
	}

	bool HasDefaultConcreteSetPiecePayloads(
		const FMatchPlaySetPieceRouteState& State)
	{
		return IsDefaultStruct(State.ShortFreeKick)
			&& IsDefaultStruct(State.LongFreeKick)
			&& IsDefaultStruct(State.Penalty)
			&& IsDefaultStruct(State.Corner);
	}

	int32 CountActiveConcreteSetPiecePayloads(
		const FMatchPlaySetPieceRouteState& State)
	{
		int32 Count = 0;
		Count += State.ShortFreeKick.Stage
			!= EMatchPlaySetPieceCarrierRouteStage::None;
		Count += State.LongFreeKick.Stage
			!= EMatchPlaySetPieceCarrierRouteStage::None;
		Count += State.Penalty.Stage
			!= EMatchPlaySetPieceCarrierRouteStage::None;
		Count += State.Corner.Stage
			!= EMatchPlaySetPieceCornerRouteStage::None;
		return Count;
	}

	bool HasMatchingActiveConcretePayload(
		const FMatchPlaySetPieceRouteState& State)
	{
		switch (State.SelectedType)
		{
		case ESetPieceSelectedType::ShortFreeKick:
			return State.ShortFreeKick.Stage
				!= EMatchPlaySetPieceCarrierRouteStage::None;
		case ESetPieceSelectedType::LongFreeKick:
			return State.LongFreeKick.Stage
				!= EMatchPlaySetPieceCarrierRouteStage::None;
		case ESetPieceSelectedType::Penalty:
			return State.Penalty.Stage
				!= EMatchPlaySetPieceCarrierRouteStage::None;
		case ESetPieceSelectedType::Corner:
			return State.Corner.Stage
				!= EMatchPlaySetPieceCornerRouteStage::None;
		default:
			return false;
		}
	}

	bool ValidateBoundCarrier(
		FMatchPlayCurrentAttackRouteStateValidationResult& Result,
		const FMatchPlayState& State,
		const FMatchPlaySetPieceParticipantBinding& Carrier)
	{
		if (!Carrier.bIsBound || Carrier.CardId.IsNone())
		{
			SetFailure(Result,
				EMatchPlayCurrentAttackRouteStateValidationErrorCode
					::AwaitingMethodMissingCarrier,
				TEXT("AwaitingMethod requires a bound Carrier."));
			return false;
		}
		if (Carrier.OwnerSide
			!= State.RuntimeState.CurrentAttackingPlayer)
		{
			SetFailure(Result,
				EMatchPlayCurrentAttackRouteStateValidationErrorCode
					::CarrierOwnerSideMismatch,
				TEXT("Set Piece Carrier must belong to CurrentAttackingPlayer."));
			return false;
		}

		FMatchPlaySetPieceParticipantEligibilityRequest Request;
		Request.ExpectedOwnerSide = Carrier.OwnerSide;
		Request.CardId = Carrier.CardId;
		Request.Role = EMatchPlaySetPieceParticipantRole::Carrier;
		const FMatchPlaySetPieceParticipantEligibilityResult Eligibility =
			FMatchPlaySetPieceParticipantEligibility::Evaluate(State, Request);
		if (!Eligibility.bIsEligible)
		{
			SetFailure(Result,
				EMatchPlayCurrentAttackRouteStateValidationErrorCode
					::CarrierEligibilityFailed,
				Eligibility.ErrorMessage);
			return false;
		}
		if (!FPlayerCardRuleSnapshot::StaticStruct()->CompareScriptStruct(
				&Carrier.Snapshot,
				&Eligibility.Binding.Snapshot,
				0))
		{
			SetFailure(Result,
				EMatchPlayCurrentAttackRouteStateValidationErrorCode
					::CarrierSnapshotBindingMismatch,
				TEXT("Persisted Carrier snapshot must match canonical side-owned snapshot authority."));
			return false;
		}
		return true;
	}

	bool ValidateCarrierPayload(
		FMatchPlayCurrentAttackRouteStateValidationResult& Result,
		const FMatchPlayState& State,
		const EMatchPlaySetPieceCarrierRouteStage Stage,
		const FMatchPlaySetPieceParticipantBinding& Carrier)
	{
		if (Stage == EMatchPlaySetPieceCarrierRouteStage::AwaitingCarrier)
		{
			if (!IsDefaultStruct(Carrier))
			{
				SetFailure(Result,
					EMatchPlayCurrentAttackRouteStateValidationErrorCode
						::AwaitingCarrierHasBoundCarrier,
					TEXT("AwaitingCarrier cannot carry a selected Carrier binding."));
				return false;
			}
			return true;
		}
		if (Stage != EMatchPlaySetPieceCarrierRouteStage::AwaitingMethod)
		{
			SetFailure(Result,
				EMatchPlayCurrentAttackRouteStateValidationErrorCode
					::InvalidConcreteSetPieceStage,
				TEXT("Carrier-based Set Piece requires AwaitingCarrier or AwaitingMethod stage."));
			return false;
		}
		return ValidateBoundCarrier(Result, State, Carrier);
	}

	bool HasDefaultShortResolutionFacts(
		const FMatchPlayShortFreeKickRouteState& Short)
	{
		return Short.Method == EMatchPlayShortFreeKickMethod::None
			&& !Short.bHasAttackD6 && Short.AttackD6 == 0
			&& !Short.bHasDefenseD6 && Short.DefenseD6 == 0
			&& !Short.bHasAngledD6Pair
			&& Short.AngledD6A == 0 && Short.AngledD6B == 0
			&& !Short.bHasFormulaResolution
			&& IsDefaultStruct(Short.FormulaResolution)
			&& Short.GameplayOutcome
				== EMatchPlayShortFreeKickGameplayOutcome::None
			&& !Short.bHasGoalScorer
			&& Short.GoalScorerCardId.IsNone()
			&& !Short.bNoLegalCarrier;
	}

	bool ValidateShortFreeKickPayload(
		FMatchPlayCurrentAttackRouteStateValidationResult& Result,
		const FMatchPlayState& State,
		const FMatchPlayShortFreeKickRouteState& Short)
	{
		const bool bTerminalLifecycle =
			State.CurrentAttack.LifecycleState
				== EMatchPlayCurrentAttackLifecycleState
					::TerminalPendingAdvance;
		if (Short.Stage == EMatchPlaySetPieceCarrierRouteStage::AwaitingCarrier)
		{
			if (bTerminalLifecycle || !IsDefaultStruct(Short.Carrier)
				|| !HasDefaultShortResolutionFacts(Short))
			{
				SetFailure(Result,
					EMatchPlayCurrentAttackRouteStateValidationErrorCode
						::AwaitingCarrierHasBoundCarrier,
					TEXT("Short Free Kick AwaitingCarrier requires no participant, method, roll, outcome, or terminal payload."));
				return false;
			}
			return true;
		}

		if (Short.Stage == EMatchPlaySetPieceCarrierRouteStage::Terminal
			&& Short.bNoLegalCarrier)
		{
			if (!bTerminalLifecycle || !IsDefaultStruct(Short.Carrier)
				|| Short.Method != EMatchPlayShortFreeKickMethod::None
				|| Short.bHasAttackD6 || Short.AttackD6 != 0
				|| Short.bHasDefenseD6 || Short.DefenseD6 != 0
				|| Short.bHasAngledD6Pair || Short.AngledD6A != 0
				|| Short.AngledD6B != 0 || Short.bHasFormulaResolution
				|| !IsDefaultStruct(Short.FormulaResolution)
				|| Short.GameplayOutcome
					!= EMatchPlayShortFreeKickGameplayOutcome::NoGoal
				|| Short.bHasGoalScorer
				|| !Short.GoalScorerCardId.IsNone())
			{
				SetFailure(Result,
					EMatchPlayCurrentAttackRouteStateValidationErrorCode
						::InvalidConcreteSetPieceStage,
					TEXT("No-legal-Carrier Short Free Kick terminal payload is incoherent."));
				return false;
			}
			return true;
		}

		if (!ValidateBoundCarrier(Result, State, Short.Carrier))
		{
			return false;
		}
		if (Short.bNoLegalCarrier)
		{
			SetFailure(Result,
				EMatchPlayCurrentAttackRouteStateValidationErrorCode
					::InvalidConcreteSetPieceStage,
				TEXT("A bound Short Free Kick Carrier cannot be marked absent."));
			return false;
		}

		const bool bNoOutcome = Short.GameplayOutcome
			== EMatchPlayShortFreeKickGameplayOutcome::None
			&& !Short.bHasGoalScorer
			&& Short.GoalScorerCardId.IsNone();
		switch (Short.Stage)
		{
		case EMatchPlaySetPieceCarrierRouteStage::AwaitingMethod:
			if (!bTerminalLifecycle && HasDefaultShortResolutionFacts(Short))
			{
				return true;
			}
			break;
		case EMatchPlaySetPieceCarrierRouteStage::DirectAwaitingAttackRoll:
			if (!bTerminalLifecycle
				&& Short.Method == EMatchPlayShortFreeKickMethod::Direct
				&& !Short.bHasAttackD6 && Short.AttackD6 == 0
				&& !Short.bHasDefenseD6 && Short.DefenseD6 == 0
				&& !Short.bHasAngledD6Pair
				&& Short.AngledD6A == 0 && Short.AngledD6B == 0
				&& !Short.bHasFormulaResolution
				&& IsDefaultStruct(Short.FormulaResolution)
				&& bNoOutcome)
			{
				return true;
			}
			break;
		case EMatchPlaySetPieceCarrierRouteStage::DirectAwaitingDefenseRoll:
			if (!bTerminalLifecycle
				&& Short.Method == EMatchPlayShortFreeKickMethod::Direct
				&& Short.bHasAttackD6 && Short.AttackD6 >= 1
				&& Short.AttackD6 <= 6
				&& !Short.bHasDefenseD6 && Short.DefenseD6 == 0
				&& !Short.bHasAngledD6Pair
				&& Short.AngledD6A == 0 && Short.AngledD6B == 0
				&& !Short.bHasFormulaResolution
				&& IsDefaultStruct(Short.FormulaResolution)
				&& bNoOutcome)
			{
				return true;
			}
			break;
		case EMatchPlaySetPieceCarrierRouteStage::AngledAwaitingRoll:
			if (!bTerminalLifecycle
				&& Short.Method == EMatchPlayShortFreeKickMethod::Angled
				&& Short.Carrier.Snapshot.Attributes.Shooting
					+ Short.Carrier.Snapshot.Attributes.Passing >= 8
				&& !Short.bHasAttackD6 && Short.AttackD6 == 0
				&& !Short.bHasDefenseD6 && Short.DefenseD6 == 0
				&& !Short.bHasAngledD6Pair
				&& Short.AngledD6A == 0 && Short.AngledD6B == 0
				&& !Short.bHasFormulaResolution
				&& IsDefaultStruct(Short.FormulaResolution)
				&& bNoOutcome)
			{
				return true;
			}
			break;
		case EMatchPlaySetPieceCarrierRouteStage::Terminal:
		{
			const bool bGoal = Short.GameplayOutcome
				== EMatchPlayShortFreeKickGameplayOutcome::Goal;
			const bool bScorerCoherent = bGoal
				? Short.bHasGoalScorer
					&& Short.GoalScorerCardId == Short.Carrier.CardId
				: !Short.bHasGoalScorer
					&& Short.GoalScorerCardId.IsNone();
			if (!bTerminalLifecycle
				|| (Short.GameplayOutcome
					!= EMatchPlayShortFreeKickGameplayOutcome::Goal
					&& Short.GameplayOutcome
						!= EMatchPlayShortFreeKickGameplayOutcome::NoGoal)
				|| !bScorerCoherent)
			{
				break;
			}
			if (Short.Method == EMatchPlayShortFreeKickMethod::Direct)
			{
				const bool bFormulaGoal = Short.FormulaResolution.bIsGoal;
				if (Short.bHasAttackD6 && Short.AttackD6 >= 1
					&& Short.AttackD6 <= 6 && Short.bHasDefenseD6
					&& Short.DefenseD6 >= 1 && Short.DefenseD6 <= 6
					&& !Short.bHasAngledD6Pair
					&& Short.AngledD6A == 0 && Short.AngledD6B == 0
					&& Short.bHasFormulaResolution
					&& Short.FormulaResolution.FormulaType
						== EFormulaType::Finishing
					&& Short.FormulaResolution.bAttackEnded
					&& !Short.FormulaResolution.bContinueResolution
					&& bFormulaGoal == bGoal)
				{
					const EInitialTurnOrderPlayer Attacker =
						State.RuntimeState.CurrentAttackingPlayer;
					const EInitialTurnOrderPlayer Defender =
						Attacker == EInitialTurnOrderPlayer::PlayerA
							? EInitialTurnOrderPlayer::PlayerB
							: EInitialTurnOrderPlayer::PlayerA;
					const FMatchPlayDefendingGoalkeeperQueryResult Gk =
						FMatchPlayDefendingGoalkeeperQuery::Query(
							State, Defender);
					if (Gk.bSuccess)
					{
						FFormulaResolverInput Input;
						Input.FormulaType = EFormulaType::Finishing;
						Input.Attacker.BaseValue = FMath::Max(
							Short.Carrier.Snapshot.Attributes.Shooting,
							Short.Carrier.Snapshot.Attributes.Passing);
						Input.Attacker.ComparePoint = Short.AttackD6;
						Input.Attacker.bComparePointWasRolledOnD6 = true;
						Input.Attacker.ParticipatingStamina.Add(
							Short.Carrier.Snapshot.Attributes.Stamina);
						Input.Defender.BaseValue =
							Gk.Snapshot.GoalkeeperAttributes.Handling;
						Input.Defender.Modifier = 1.0f;
						Input.Defender.ComparePoint = Short.DefenseD6;
						Input.Defender.bComparePointWasRolledOnD6 = true;
						Input.Defender.ParticipatingStamina.Add(
							Gk.Snapshot.Attributes.Stamina);
						Input.bGoalkeeperParticipated = true;
						Input.TurnIndex = static_cast<int32>(
							State.CurrentAttack.AttackSequence);
						Input.AttackerPlayerId = Attacker
							== EInitialTurnOrderPlayer::PlayerA
								? FName(TEXT("PlayerA"))
								: FName(TEXT("PlayerB"));
						Input.DefenderPlayerId = Defender
							== EInitialTurnOrderPlayer::PlayerA
								? FName(TEXT("PlayerA"))
								: FName(TEXT("PlayerB"));
						Input.InvolvedCardIds = {
							Short.Carrier.CardId, Gk.CardId };
						const auto Execution =
							FSingleCardFormulaResolutionExecutor::Execute(Input);
						if (Execution.bSuccess
							&& FFormulaResolutionResult::StaticStruct()
								->CompareScriptStruct(
									&Short.FormulaResolution,
									&Execution.FormulaResolutionResult,
									0))
						{
							return true;
						}
					}
				}
			}
			else if (Short.Method
				== EMatchPlayShortFreeKickMethod::Angled)
			{
				const int32 PairSum = Short.AngledD6A + Short.AngledD6B;
				if (!Short.bHasAttackD6 && Short.AttackD6 == 0
					&& !Short.bHasDefenseD6 && Short.DefenseD6 == 0
					&& Short.bHasAngledD6Pair
					&& Short.AngledD6A >= 1 && Short.AngledD6A <= 6
					&& Short.AngledD6B >= 1 && Short.AngledD6B <= 6
					&& !Short.bHasFormulaResolution
					&& IsDefaultStruct(Short.FormulaResolution)
					&& (PairSum >= 9) == bGoal)
				{
					return true;
				}
			}
			break;
		}
		default:
			break;
		}

		SetFailure(Result,
			EMatchPlayCurrentAttackRouteStateValidationErrorCode
				::InvalidConcreteSetPieceStage,
			TEXT("Short Free Kick stage, method, rolls, outcome, scorer, and lifecycle are incoherent."));
		return false;
	}

	bool HasDefaultLongResolutionFacts(
		const FMatchPlayLongFreeKickRouteState& Long)
	{
		return Long.Method == EMatchPlayLongFreeKickMethod::None
			&& !Long.bHasAttackD6 && Long.AttackD6 == 0
			&& !Long.bHasDefenseD6 && Long.DefenseD6 == 0
			&& !Long.bHasPowerD6Pair
			&& Long.PowerD6A == 0 && Long.PowerD6B == 0
			&& !Long.bHasFormulaResolution
			&& IsDefaultStruct(Long.FormulaResolution)
			&& Long.GameplayOutcome
				== EMatchPlayLongFreeKickGameplayOutcome::None
			&& !Long.bHasGoalScorer
			&& Long.GoalScorerCardId.IsNone()
			&& !Long.bNoLegalCarrier;
	}

	bool ValidateLongFreeKickPayload(
		FMatchPlayCurrentAttackRouteStateValidationResult& Result,
		const FMatchPlayState& State,
		const FMatchPlayLongFreeKickRouteState& Long)
	{
		const bool bTerminalLifecycle =
			State.CurrentAttack.LifecycleState
				== EMatchPlayCurrentAttackLifecycleState
					::TerminalPendingAdvance;
		if (Long.Stage == EMatchPlaySetPieceCarrierRouteStage::AwaitingCarrier)
		{
			if (bTerminalLifecycle || !IsDefaultStruct(Long.Carrier)
				|| !HasDefaultLongResolutionFacts(Long))
			{
				SetFailure(Result,
					EMatchPlayCurrentAttackRouteStateValidationErrorCode
						::AwaitingCarrierHasBoundCarrier,
					TEXT("Long Free Kick AwaitingCarrier requires no participant, method, roll, outcome, or terminal payload."));
				return false;
			}
			return true;
		}

		if (Long.Stage == EMatchPlaySetPieceCarrierRouteStage::Terminal
			&& Long.bNoLegalCarrier)
		{
			if (!bTerminalLifecycle || !IsDefaultStruct(Long.Carrier)
				|| Long.Method != EMatchPlayLongFreeKickMethod::None
				|| Long.bHasAttackD6 || Long.AttackD6 != 0
				|| Long.bHasDefenseD6 || Long.DefenseD6 != 0
				|| Long.bHasPowerD6Pair || Long.PowerD6A != 0
				|| Long.PowerD6B != 0 || Long.bHasFormulaResolution
				|| !IsDefaultStruct(Long.FormulaResolution)
				|| Long.GameplayOutcome
					!= EMatchPlayLongFreeKickGameplayOutcome::NoGoal
				|| Long.bHasGoalScorer
				|| !Long.GoalScorerCardId.IsNone())
			{
				SetFailure(Result,
					EMatchPlayCurrentAttackRouteStateValidationErrorCode
						::InvalidConcreteSetPieceStage,
					TEXT("No-legal-Carrier Long Free Kick terminal payload is incoherent."));
				return false;
			}
			return true;
		}

		if (!ValidateBoundCarrier(Result, State, Long.Carrier))
		{
			return false;
		}
		if (Long.bNoLegalCarrier)
		{
			SetFailure(Result,
				EMatchPlayCurrentAttackRouteStateValidationErrorCode
					::InvalidConcreteSetPieceStage,
				TEXT("A bound Long Free Kick Carrier cannot be marked absent."));
			return false;
		}

		const bool bNoOutcome = Long.GameplayOutcome
			== EMatchPlayLongFreeKickGameplayOutcome::None
			&& !Long.bHasGoalScorer
			&& Long.GoalScorerCardId.IsNone();
		switch (Long.Stage)
		{
		case EMatchPlaySetPieceCarrierRouteStage::AwaitingMethod:
			if (!bTerminalLifecycle && HasDefaultLongResolutionFacts(Long))
			{
				return true;
			}
			break;
		case EMatchPlaySetPieceCarrierRouteStage::DirectAwaitingAttackRoll:
			if (!bTerminalLifecycle
				&& Long.Method == EMatchPlayLongFreeKickMethod::Direct
				&& !Long.bHasAttackD6 && Long.AttackD6 == 0
				&& !Long.bHasDefenseD6 && Long.DefenseD6 == 0
				&& !Long.bHasPowerD6Pair
				&& Long.PowerD6A == 0 && Long.PowerD6B == 0
				&& !Long.bHasFormulaResolution
				&& IsDefaultStruct(Long.FormulaResolution)
				&& bNoOutcome)
			{
				return true;
			}
			break;
		case EMatchPlaySetPieceCarrierRouteStage::DirectAwaitingDefenseRoll:
			if (!bTerminalLifecycle
				&& Long.Method == EMatchPlayLongFreeKickMethod::Direct
				&& Long.bHasAttackD6 && Long.AttackD6 >= 3
				&& Long.AttackD6 <= 6
				&& !Long.bHasDefenseD6 && Long.DefenseD6 == 0
				&& !Long.bHasPowerD6Pair
				&& Long.PowerD6A == 0 && Long.PowerD6B == 0
				&& !Long.bHasFormulaResolution
				&& IsDefaultStruct(Long.FormulaResolution)
				&& bNoOutcome)
			{
				return true;
			}
			break;
		case EMatchPlaySetPieceCarrierRouteStage::PowerAwaitingRoll:
			if (!bTerminalLifecycle
				&& Long.Method == EMatchPlayLongFreeKickMethod::Power
				&& !Long.bHasAttackD6 && Long.AttackD6 == 0
				&& !Long.bHasDefenseD6 && Long.DefenseD6 == 0
				&& !Long.bHasPowerD6Pair
				&& Long.PowerD6A == 0 && Long.PowerD6B == 0
				&& !Long.bHasFormulaResolution
				&& IsDefaultStruct(Long.FormulaResolution)
				&& bNoOutcome)
			{
				return true;
			}
			break;
		case EMatchPlaySetPieceCarrierRouteStage::Terminal:
		{
			const bool bGoal = Long.GameplayOutcome
				== EMatchPlayLongFreeKickGameplayOutcome::Goal;
			const bool bScorerCoherent = bGoal
				? Long.bHasGoalScorer
					&& Long.GoalScorerCardId == Long.Carrier.CardId
				: !Long.bHasGoalScorer
					&& Long.GoalScorerCardId.IsNone();
			if (!bTerminalLifecycle
				|| (Long.GameplayOutcome
					!= EMatchPlayLongFreeKickGameplayOutcome::Goal
					&& Long.GameplayOutcome
						!= EMatchPlayLongFreeKickGameplayOutcome::NoGoal)
				|| !bScorerCoherent)
			{
				break;
			}

			if (Long.Method == EMatchPlayLongFreeKickMethod::Direct)
			{
				const bool bImmediateMiss = Long.bHasAttackD6
					&& Long.AttackD6 >= 1 && Long.AttackD6 <= 2
					&& !Long.bHasDefenseD6 && Long.DefenseD6 == 0
					&& !Long.bHasPowerD6Pair
					&& Long.PowerD6A == 0 && Long.PowerD6B == 0
					&& !Long.bHasFormulaResolution
					&& IsDefaultStruct(Long.FormulaResolution)
					&& !bGoal;
				if (bImmediateMiss)
				{
					return true;
				}

				if (Long.bHasAttackD6 && Long.AttackD6 >= 3
					&& Long.AttackD6 <= 6 && Long.bHasDefenseD6
					&& Long.DefenseD6 >= 1 && Long.DefenseD6 <= 6
					&& !Long.bHasPowerD6Pair
					&& Long.PowerD6A == 0 && Long.PowerD6B == 0
					&& Long.bHasFormulaResolution
					&& Long.FormulaResolution.FormulaType
						== EFormulaType::Finishing
					&& Long.FormulaResolution.bAttackEnded
					&& !Long.FormulaResolution.bContinueResolution
					&& Long.FormulaResolution.bIsGoal == bGoal)
				{
					const EInitialTurnOrderPlayer Attacker =
						State.RuntimeState.CurrentAttackingPlayer;
					const EInitialTurnOrderPlayer Defender =
						Attacker == EInitialTurnOrderPlayer::PlayerA
							? EInitialTurnOrderPlayer::PlayerB
							: EInitialTurnOrderPlayer::PlayerA;
					const FMatchPlayDefendingGoalkeeperQueryResult Gk =
						FMatchPlayDefendingGoalkeeperQuery::Query(
							State, Defender);
					if (Gk.bSuccess)
					{
						FFormulaResolverInput Input;
						Input.FormulaType = EFormulaType::Finishing;
						Input.Attacker.BaseValue =
							Long.Carrier.Snapshot.Attributes.LongShot;
						Input.Attacker.ComparePoint = Long.AttackD6;
						Input.Attacker.bComparePointWasRolledOnD6 = true;
						Input.Attacker.ParticipatingStamina.Add(
							Long.Carrier.Snapshot.Attributes.Stamina);
						Input.Defender.BaseValue =
							Gk.Snapshot.GoalkeeperAttributes.Positioning;
						Input.Defender.Modifier = 2.0f;
						Input.Defender.ComparePoint = Long.DefenseD6;
						Input.Defender.bComparePointWasRolledOnD6 = true;
						Input.Defender.ParticipatingStamina.Add(
							Gk.Snapshot.Attributes.Stamina);
						Input.bGoalkeeperParticipated = true;
						Input.TurnIndex = static_cast<int32>(
							State.CurrentAttack.AttackSequence);
						Input.AttackerPlayerId = Attacker
							== EInitialTurnOrderPlayer::PlayerA
								? FName(TEXT("PlayerA"))
								: FName(TEXT("PlayerB"));
						Input.DefenderPlayerId = Defender
							== EInitialTurnOrderPlayer::PlayerA
								? FName(TEXT("PlayerA"))
								: FName(TEXT("PlayerB"));
						Input.InvolvedCardIds = {
							Long.Carrier.CardId, Gk.CardId };
						const auto Execution =
							FSingleCardFormulaResolutionExecutor::Execute(Input);
						if (Execution.bSuccess
							&& FFormulaResolutionResult::StaticStruct()
								->CompareScriptStruct(
									&Long.FormulaResolution,
									&Execution.FormulaResolutionResult,
									0))
						{
							return true;
						}
					}
				}
			}
			else if (Long.Method == EMatchPlayLongFreeKickMethod::Power)
			{
				const int32 PairSum = Long.PowerD6A + Long.PowerD6B;
				if (!Long.bHasAttackD6 && Long.AttackD6 == 0
					&& !Long.bHasDefenseD6 && Long.DefenseD6 == 0
					&& Long.bHasPowerD6Pair
					&& Long.PowerD6A >= 1 && Long.PowerD6A <= 6
					&& Long.PowerD6B >= 1 && Long.PowerD6B <= 6
					&& !Long.bHasFormulaResolution
					&& IsDefaultStruct(Long.FormulaResolution)
					&& (PairSum >= 11) == bGoal)
				{
					return true;
				}
			}
			break;
		}
		default:
			break;
		}

		SetFailure(Result,
			EMatchPlayCurrentAttackRouteStateValidationErrorCode
				::InvalidConcreteSetPieceStage,
			TEXT("Long Free Kick stage, method, rolls, outcome, scorer, and lifecycle are incoherent."));
		return false;
	}

	bool HasDefaultPenaltyResolutionFacts(
		const FMatchPlayPenaltyRouteState& Penalty)
	{
		return Penalty.Method == EMatchPlayPenaltyMethod::None
			&& !Penalty.bHasAttackD6 && Penalty.AttackD6 == 0
			&& !Penalty.bHasDefenseD6 && Penalty.DefenseD6 == 0
			&& !Penalty.bHasPanenkaD6 && Penalty.PanenkaD6 == 0
			&& !Penalty.bHasFormulaResolution
			&& IsDefaultStruct(Penalty.FormulaResolution)
			&& Penalty.GameplayOutcome
				== EMatchPlayPenaltyGameplayOutcome::None
			&& !Penalty.bHasGoalScorer
			&& Penalty.GoalScorerCardId.IsNone()
			&& !Penalty.bNoLegalCarrier;
	}

	bool ValidatePenaltyPayload(
		FMatchPlayCurrentAttackRouteStateValidationResult& Result,
		const FMatchPlayState& State,
		const FMatchPlayPenaltyRouteState& Penalty)
	{
		const bool bTerminalLifecycle =
			State.CurrentAttack.LifecycleState
				== EMatchPlayCurrentAttackLifecycleState
					::TerminalPendingAdvance;
		if (Penalty.Stage
			== EMatchPlaySetPieceCarrierRouteStage::AwaitingCarrier)
		{
			if (bTerminalLifecycle || !IsDefaultStruct(Penalty.Carrier)
				|| !HasDefaultPenaltyResolutionFacts(Penalty))
			{
				SetFailure(Result,
					EMatchPlayCurrentAttackRouteStateValidationErrorCode
						::AwaitingCarrierHasBoundCarrier,
					TEXT("Penalty AwaitingCarrier requires no participant, method, roll, outcome, or terminal payload."));
				return false;
			}
			return true;
		}

		if (Penalty.Stage == EMatchPlaySetPieceCarrierRouteStage::Terminal
			&& Penalty.bNoLegalCarrier)
		{
			if (!bTerminalLifecycle || !IsDefaultStruct(Penalty.Carrier)
				|| Penalty.Method != EMatchPlayPenaltyMethod::None
				|| Penalty.bHasAttackD6 || Penalty.AttackD6 != 0
				|| Penalty.bHasDefenseD6 || Penalty.DefenseD6 != 0
				|| Penalty.bHasPanenkaD6 || Penalty.PanenkaD6 != 0
				|| Penalty.bHasFormulaResolution
				|| !IsDefaultStruct(Penalty.FormulaResolution)
				|| Penalty.GameplayOutcome
					!= EMatchPlayPenaltyGameplayOutcome::NoGoal
				|| Penalty.bHasGoalScorer
				|| !Penalty.GoalScorerCardId.IsNone())
			{
				SetFailure(Result,
					EMatchPlayCurrentAttackRouteStateValidationErrorCode
						::InvalidConcreteSetPieceStage,
					TEXT("No-legal-Carrier Penalty terminal payload is incoherent."));
				return false;
			}
			return true;
		}

		if (!ValidateBoundCarrier(Result, State, Penalty.Carrier))
		{
			return false;
		}
		if (Penalty.bNoLegalCarrier)
		{
			SetFailure(Result,
				EMatchPlayCurrentAttackRouteStateValidationErrorCode
					::InvalidConcreteSetPieceStage,
				TEXT("A bound Penalty Carrier cannot be marked absent."));
			return false;
		}

		const bool bNoOutcome = Penalty.GameplayOutcome
			== EMatchPlayPenaltyGameplayOutcome::None
			&& !Penalty.bHasGoalScorer
			&& Penalty.GoalScorerCardId.IsNone();
		switch (Penalty.Stage)
		{
		case EMatchPlaySetPieceCarrierRouteStage::AwaitingMethod:
			if (!bTerminalLifecycle
				&& HasDefaultPenaltyResolutionFacts(Penalty))
			{
				return true;
			}
			break;
		case EMatchPlaySetPieceCarrierRouteStage::DirectAwaitingAttackRoll:
			if (!bTerminalLifecycle
				&& Penalty.Method == EMatchPlayPenaltyMethod::Direct
				&& !Penalty.bHasAttackD6 && Penalty.AttackD6 == 0
				&& !Penalty.bHasDefenseD6 && Penalty.DefenseD6 == 0
				&& !Penalty.bHasPanenkaD6 && Penalty.PanenkaD6 == 0
				&& !Penalty.bHasFormulaResolution
				&& IsDefaultStruct(Penalty.FormulaResolution)
				&& bNoOutcome)
			{
				return true;
			}
			break;
		case EMatchPlaySetPieceCarrierRouteStage::DirectAwaitingDefenseRoll:
			if (!bTerminalLifecycle
				&& Penalty.Method == EMatchPlayPenaltyMethod::Direct
				&& Penalty.bHasAttackD6 && Penalty.AttackD6 >= 1
				&& Penalty.AttackD6 <= 6
				&& !Penalty.bHasDefenseD6 && Penalty.DefenseD6 == 0
				&& !Penalty.bHasPanenkaD6 && Penalty.PanenkaD6 == 0
				&& !Penalty.bHasFormulaResolution
				&& IsDefaultStruct(Penalty.FormulaResolution)
				&& bNoOutcome)
			{
				return true;
			}
			break;
		case EMatchPlaySetPieceCarrierRouteStage::PanenkaAwaitingRoll:
			if (!bTerminalLifecycle
				&& Penalty.Method == EMatchPlayPenaltyMethod::Panenka
				&& !Penalty.bHasAttackD6 && Penalty.AttackD6 == 0
				&& !Penalty.bHasDefenseD6 && Penalty.DefenseD6 == 0
				&& !Penalty.bHasPanenkaD6 && Penalty.PanenkaD6 == 0
				&& !Penalty.bHasFormulaResolution
				&& IsDefaultStruct(Penalty.FormulaResolution)
				&& bNoOutcome)
			{
				return true;
			}
			break;
		case EMatchPlaySetPieceCarrierRouteStage::Terminal:
		{
			const bool bGoal = Penalty.GameplayOutcome
				== EMatchPlayPenaltyGameplayOutcome::Goal;
			const bool bScorerCoherent = bGoal
				? Penalty.bHasGoalScorer
					&& Penalty.GoalScorerCardId == Penalty.Carrier.CardId
				: !Penalty.bHasGoalScorer
					&& Penalty.GoalScorerCardId.IsNone();
			if (!bTerminalLifecycle
				|| (Penalty.GameplayOutcome
						!= EMatchPlayPenaltyGameplayOutcome::Goal
					&& Penalty.GameplayOutcome
						!= EMatchPlayPenaltyGameplayOutcome::NoGoal)
				|| !bScorerCoherent)
			{
				break;
			}

			if (Penalty.Method == EMatchPlayPenaltyMethod::Direct
				&& Penalty.bHasAttackD6 && Penalty.AttackD6 >= 1
				&& Penalty.AttackD6 <= 6 && Penalty.bHasDefenseD6
				&& Penalty.DefenseD6 >= 1 && Penalty.DefenseD6 <= 6
				&& !Penalty.bHasPanenkaD6 && Penalty.PanenkaD6 == 0
				&& Penalty.bHasFormulaResolution
				&& Penalty.FormulaResolution.FormulaType
					== EFormulaType::Finishing
				&& Penalty.FormulaResolution.bAttackEnded
				&& !Penalty.FormulaResolution.bContinueResolution
				&& Penalty.FormulaResolution.bIsGoal == bGoal)
			{
				const EInitialTurnOrderPlayer Attacker =
					State.RuntimeState.CurrentAttackingPlayer;
				const EInitialTurnOrderPlayer Defender =
					Attacker == EInitialTurnOrderPlayer::PlayerA
						? EInitialTurnOrderPlayer::PlayerB
						: EInitialTurnOrderPlayer::PlayerA;
				const FMatchPlayDefendingGoalkeeperQueryResult Gk =
					FMatchPlayDefendingGoalkeeperQuery::Query(
						State, Defender);
				if (Gk.bSuccess)
				{
					FFormulaResolverInput Input;
					Input.FormulaType = EFormulaType::Finishing;
					Input.Attacker.BaseValue = FMath::Max(
						Penalty.Carrier.Snapshot.Attributes.Shooting,
						Penalty.Carrier.Snapshot.Attributes.Passing);
					Input.Attacker.ComparePoint = Penalty.AttackD6;
					Input.Attacker.bComparePointWasRolledOnD6 = true;
					Input.Attacker.ParticipatingStamina.Add(
						Penalty.Carrier.Snapshot.Attributes.Stamina);
					Input.Defender.BaseValue =
						Gk.Snapshot.GoalkeeperAttributes.Anticipation;
					Input.Defender.Modifier = -3.0f;
					Input.Defender.ComparePoint = Penalty.DefenseD6;
					Input.Defender.bComparePointWasRolledOnD6 = true;
					Input.Defender.ParticipatingStamina.Add(
						Gk.Snapshot.Attributes.Stamina);
					Input.bGoalkeeperParticipated = true;
					Input.TurnIndex = static_cast<int32>(
						State.CurrentAttack.AttackSequence);
					Input.AttackerPlayerId = Attacker
						== EInitialTurnOrderPlayer::PlayerA
							? FName(TEXT("PlayerA"))
							: FName(TEXT("PlayerB"));
					Input.DefenderPlayerId = Defender
						== EInitialTurnOrderPlayer::PlayerA
							? FName(TEXT("PlayerA"))
							: FName(TEXT("PlayerB"));
					Input.InvolvedCardIds = {
						Penalty.Carrier.CardId, Gk.CardId };
					const auto Execution =
						FSingleCardFormulaResolutionExecutor::Execute(Input);
					if (Execution.bSuccess
						&& FFormulaResolutionResult::StaticStruct()
							->CompareScriptStruct(
								&Penalty.FormulaResolution,
								&Execution.FormulaResolutionResult, 0))
					{
						return true;
					}
				}
			}
			else if (Penalty.Method == EMatchPlayPenaltyMethod::Panenka
				&& !Penalty.bHasAttackD6 && Penalty.AttackD6 == 0
				&& !Penalty.bHasDefenseD6 && Penalty.DefenseD6 == 0
				&& Penalty.bHasPanenkaD6 && Penalty.PanenkaD6 >= 1
				&& Penalty.PanenkaD6 <= 6
				&& !Penalty.bHasFormulaResolution
				&& IsDefaultStruct(Penalty.FormulaResolution)
				&& (Penalty.PanenkaD6 >= 2) == bGoal)
			{
				return true;
			}
			break;
		}
		default:
			break;
		}

		SetFailure(Result,
			EMatchPlayCurrentAttackRouteStateValidationErrorCode
				::InvalidConcreteSetPieceStage,
			TEXT("Penalty stage, method, rolls, outcome, scorer, and lifecycle are incoherent."));
		return false;
	}

	bool HasDefaultCornerSelectionFacts(
		const FMatchPlayCornerRouteState& Corner)
	{
		return !Corner.bHasSharedParticipantD6
			&& Corner.SharedParticipantD6 == 0
			&& Corner.AutomaticScorerD6 == 0
			&& IsDefaultStruct(Corner.Runner)
			&& IsDefaultStruct(Corner.Helper)
			&& Corner.CandidateBonusSide == EInitialTurnOrderPlayer::None
			&& Corner.CandidateBonus == 0;
	}

	bool HasDefaultCornerRouteFacts(
		const FMatchPlayCornerRouteState& Corner)
	{
		return Corner.IntendedRoute == EMatchPlayCornerRouteIntent::None
			&& !Corner.bHasRouteD6 && Corner.RawRouteD6 == 0
			&& Corner.ActualRoute == EMatchPlayCornerRouteIntent::None
			&& !Corner.bHasAttackD6 && Corner.AttackD6 == 0
			&& !Corner.bHasDefenseD6 && Corner.DefenseD6 == 0
			&& !Corner.bHasFormulaResolution
			&& IsDefaultStruct(Corner.FormulaResolution)
			&& Corner.GameplayOutcome
				== EMatchPlayCornerGameplayOutcome::None
			&& !Corner.bHasGoalScorer
			&& Corner.GoalScorerCardId.IsNone();
	}

	bool ValidateCornerNomineeList(
		FMatchPlayCurrentAttackRouteStateValidationResult& Result,
		const FMatchPlayState& State,
		const TArray<FMatchPlaySetPieceParticipantBinding>& Nominees,
		const EInitialTurnOrderPlayer OwnerSide,
		const EMatchPlaySetPieceParticipantRole Role)
	{
		if (Nominees.Num() > 3)
		{
			SetFailure(Result,
				EMatchPlayCurrentAttackRouteStateValidationErrorCode
					::InvalidCornerNominationState,
				TEXT("Corner nomination lists may contain at most three cards."));
			return false;
		}

		TSet<FName> SeenCardIds;
		for (const FMatchPlaySetPieceParticipantBinding& Nominee : Nominees)
		{
			if (!Nominee.bIsBound || Nominee.CardId.IsNone()
				|| Nominee.OwnerSide != OwnerSide
				|| SeenCardIds.Contains(Nominee.CardId))
			{
				SetFailure(Result,
					EMatchPlayCurrentAttackRouteStateValidationErrorCode
						::InvalidCornerNominationState,
					TEXT("Corner nominees must be unique bound cards owned by the expected side."));
				return false;
			}
			SeenCardIds.Add(Nominee.CardId);

			FMatchPlaySetPieceParticipantEligibilityRequest Request;
			Request.ExpectedOwnerSide = OwnerSide;
			Request.CardId = Nominee.CardId;
			Request.Role = Role;
			const FMatchPlaySetPieceParticipantEligibilityResult Eligibility =
				FMatchPlaySetPieceParticipantEligibility::Evaluate(State, Request);
			if (!Eligibility.bIsEligible
				|| !FMatchPlaySetPieceParticipantBinding::StaticStruct()
					->CompareScriptStruct(
						&Nominee, &Eligibility.Binding, 0))
			{
				SetFailure(Result,
					EMatchPlayCurrentAttackRouteStateValidationErrorCode
						::CornerNomineeEligibilityFailed,
					Eligibility.bIsEligible
						? TEXT("Corner nominee binding must match canonical side-owned snapshot authority.")
						: Eligibility.ErrorMessage);
				return false;
			}
		}
		return true;
	}

	bool ValidateCornerSelection(
		FMatchPlayCurrentAttackRouteStateValidationResult& Result,
		const FMatchPlayCornerRouteState& Corner,
		const EInitialTurnOrderPlayer Attacker,
		const EInitialTurnOrderPlayer Defender)
	{
		const int32 RunnerIndex = FMatchPlayCornerResolution::MapParticipantIndex(
			Corner.AttackerNominees.Num(), Corner.SharedParticipantD6);
		const int32 HelperIndex = FMatchPlayCornerResolution::MapParticipantIndex(
			Corner.DefenderNominees.Num(), Corner.SharedParticipantD6);
		const int32 Difference = FMath::Abs(
			Corner.AttackerNominees.Num() - Corner.DefenderNominees.Num());
		const int32 ExpectedBonus = Difference == 1 ? 2
			: Difference == 2 ? 3 : 0;
		const EInitialTurnOrderPlayer ExpectedBonusSide = ExpectedBonus == 0
			? EInitialTurnOrderPlayer::None
			: Corner.AttackerNominees.Num() > Corner.DefenderNominees.Num()
				? Attacker : Defender;

		if (!Corner.bHasSharedParticipantD6
			|| Corner.AutomaticScorerD6 != 0
			|| Corner.SharedParticipantD6 < 1
			|| Corner.SharedParticipantD6 > 6
			|| !Corner.AttackerNominees.IsValidIndex(RunnerIndex)
			|| !Corner.DefenderNominees.IsValidIndex(HelperIndex)
			|| !FMatchPlaySetPieceParticipantBinding::StaticStruct()
				->CompareScriptStruct(&Corner.Runner,
					&Corner.AttackerNominees[RunnerIndex], 0)
			|| !FMatchPlaySetPieceParticipantBinding::StaticStruct()
				->CompareScriptStruct(&Corner.Helper,
					&Corner.DefenderNominees[HelperIndex], 0)
			|| Corner.CandidateBonus != ExpectedBonus
			|| Corner.CandidateBonusSide != ExpectedBonusSide)
		{
			SetFailure(Result,
				EMatchPlayCurrentAttackRouteStateValidationErrorCode
					::CornerParticipantMappingMismatch,
				TEXT("Corner shared D6 participants and candidate-count bonus must match canonical mapping."));
			return false;
		}
		return true;
	}

	EMatchPlayCornerRouteIntent GetActualCornerRoute(
		const EMatchPlayCornerRouteIntent IntendedRoute,
		const int32 RawD6)
	{
		if (RawD6 <= 4)
		{
			return IntendedRoute;
		}
		return IntendedRoute == EMatchPlayCornerRouteIntent::High
			? EMatchPlayCornerRouteIntent::Low
			: EMatchPlayCornerRouteIntent::High;
	}

	FFormulaResolverInput BuildCornerFormulaInput(
		const FMatchPlayState& State,
		const FMatchPlayCornerRouteState& Corner,
		const FMatchPlayDefendingGoalkeeperQueryResult& Goalkeeper,
		const EInitialTurnOrderPlayer Attacker,
		const EInitialTurnOrderPlayer Defender)
	{
		return FMatchPlayCornerResolution::BuildFormulaInput(Corner, Goalkeeper,
			Attacker, Defender, State.CurrentAttack.AttackSequence);
	}

	bool ValidateCornerPayload(
		FMatchPlayCurrentAttackRouteStateValidationResult& Result,
		const FMatchPlayState& State,
		const FMatchPlayCornerRouteState& Corner)
	{
		const EInitialTurnOrderPlayer Attacker =
			State.RuntimeState.CurrentAttackingPlayer;
		const EInitialTurnOrderPlayer Defender = Attacker
			== EInitialTurnOrderPlayer::PlayerA
				? EInitialTurnOrderPlayer::PlayerB
				: EInitialTurnOrderPlayer::PlayerA;
		const bool bTerminalLifecycle = State.CurrentAttack.LifecycleState
			== EMatchPlayCurrentAttackLifecycleState::TerminalPendingAdvance;
		const bool bActiveLifecycle = State.CurrentAttack.LifecycleState
			== EMatchPlayCurrentAttackLifecycleState::Active;

		if (Corner.Stage
			== EMatchPlaySetPieceCornerRouteStage::AwaitingAttackerNominations)
		{
			if (bActiveLifecycle && !Corner.bAttackerNominationsLocked
				&& Corner.AttackerNominees.IsEmpty()
				&& !Corner.bDefenderNominationsLocked
				&& Corner.DefenderNominees.IsEmpty()
				&& HasDefaultCornerSelectionFacts(Corner)
				&& HasDefaultCornerRouteFacts(Corner))
			{
				return true;
			}
		}
		else if (Corner.Stage
			== EMatchPlaySetPieceCornerRouteStage::AwaitingDefenderNominations)
		{
			if (bActiveLifecycle && Corner.bAttackerNominationsLocked
				&& !Corner.bDefenderNominationsLocked
				&& Corner.DefenderNominees.IsEmpty()
				&& ValidateCornerNomineeList(Result, State,
					Corner.AttackerNominees, Attacker,
					EMatchPlaySetPieceParticipantRole::CornerRunner)
				&& HasDefaultCornerSelectionFacts(Corner)
				&& HasDefaultCornerRouteFacts(Corner))
			{
				return true;
			}
		}
		else
		{
			if (!Corner.bAttackerNominationsLocked
				|| !Corner.bDefenderNominationsLocked
				|| !ValidateCornerNomineeList(Result, State,
					Corner.AttackerNominees, Attacker,
					EMatchPlaySetPieceParticipantRole::CornerRunner)
				|| !ValidateCornerNomineeList(Result, State,
					Corner.DefenderNominees, Defender,
					EMatchPlaySetPieceParticipantRole::CornerHelper))
			{
				if (Result.ErrorCode
					== EMatchPlayCurrentAttackRouteStateValidationErrorCode::None)
				{
					SetFailure(Result,
						EMatchPlayCurrentAttackRouteStateValidationErrorCode
							::InvalidCornerNominationState,
						TEXT("Corner stages after nomination require both ordered lists to be locked and canonical."));
				}
				return false;
			}

			const bool bBothHaveCandidates = !Corner.AttackerNominees.IsEmpty()
				&& !Corner.DefenderNominees.IsEmpty();
			if (Corner.Stage
				== EMatchPlaySetPieceCornerRouteStage
					::AwaitingParticipantSelectionRoll)
			{
				if (bActiveLifecycle && bBothHaveCandidates
					&& HasDefaultCornerSelectionFacts(Corner)
					&& HasDefaultCornerRouteFacts(Corner))
				{
					return true;
				}
			}
			else if (Corner.Stage
				== EMatchPlaySetPieceCornerRouteStage::Terminal
				&& !bBothHaveCandidates)
			{
				const bool bAttackerShortage =
					Corner.AttackerNominees.IsEmpty();
				const EMatchPlayCornerGameplayOutcome ExpectedOutcome =
					bAttackerShortage
						? EMatchPlayCornerGameplayOutcome::NoGoal
						: EMatchPlayCornerGameplayOutcome::Goal;
				const int32 ScorerIndex = Corner.AttackerNominees.Num() == 1
					? (Corner.AutomaticScorerD6 == 0 ? 0 : INDEX_NONE)
					: FMatchPlayCornerResolution::MapParticipantIndex(
						Corner.AttackerNominees.Num(), Corner.AutomaticScorerD6);
				const bool bAutomaticScorerValid = !bAttackerShortage
					&& Corner.AttackerNominees.IsValidIndex(ScorerIndex)
					&& !Corner.bHasSharedParticipantD6 && Corner.SharedParticipantD6 == 0
					&& IsDefaultStruct(Corner.Helper)
					&& Corner.CandidateBonus == 0
					&& Corner.CandidateBonusSide == EInitialTurnOrderPlayer::None
					&& FMatchPlaySetPieceParticipantBinding::StaticStruct()->CompareScriptStruct(
						&Corner.Runner, &Corner.AttackerNominees[ScorerIndex], 0)
					&& Corner.bHasGoalScorer && Corner.GoalScorerCardId == Corner.Runner.CardId;
				if (bTerminalLifecycle
					&& (bAttackerShortage
						? HasDefaultCornerSelectionFacts(Corner)
							&& !Corner.bHasGoalScorer && Corner.GoalScorerCardId.IsNone()
						: bAutomaticScorerValid)
					&& Corner.IntendedRoute
						== EMatchPlayCornerRouteIntent::None
					&& !Corner.bHasRouteD6 && Corner.RawRouteD6 == 0
					&& Corner.ActualRoute
						== EMatchPlayCornerRouteIntent::None
					&& !Corner.bHasAttackD6 && Corner.AttackD6 == 0
					&& !Corner.bHasDefenseD6 && Corner.DefenseD6 == 0
					&& !Corner.bHasFormulaResolution
					&& IsDefaultStruct(Corner.FormulaResolution)
					&& Corner.GameplayOutcome == ExpectedOutcome)
				{
					return true;
				}
			}
			else if (bBothHaveCandidates
				&& ValidateCornerSelection(Result, Corner,
					Attacker, Defender))
			{
				const bool bNoIntent = Corner.IntendedRoute
					== EMatchPlayCornerRouteIntent::None;
				const bool bNoRoute = !Corner.bHasRouteD6
					&& Corner.RawRouteD6 == 0
					&& Corner.ActualRoute
						== EMatchPlayCornerRouteIntent::None;
				const bool bNoAttack = !Corner.bHasAttackD6
					&& Corner.AttackD6 == 0;
				const bool bNoDefense = !Corner.bHasDefenseD6
					&& Corner.DefenseD6 == 0;
				const bool bNoFormulaOrOutcome =
					!Corner.bHasFormulaResolution
					&& IsDefaultStruct(Corner.FormulaResolution)
					&& Corner.GameplayOutcome
						== EMatchPlayCornerGameplayOutcome::None
					&& !Corner.bHasGoalScorer
					&& Corner.GoalScorerCardId.IsNone();

				if (Corner.Stage
					== EMatchPlaySetPieceCornerRouteStage::AwaitingIntent
					&& bActiveLifecycle && bNoIntent && bNoRoute
					&& bNoAttack && bNoDefense && bNoFormulaOrOutcome)
				{
					return true;
				}

				const bool bValidIntent = Corner.IntendedRoute
					== EMatchPlayCornerRouteIntent::High
					|| Corner.IntendedRoute
						== EMatchPlayCornerRouteIntent::Low;
				if (Corner.Stage
					== EMatchPlaySetPieceCornerRouteStage::AwaitingRouteRoll
					&& bActiveLifecycle && bValidIntent && bNoRoute
					&& bNoAttack && bNoDefense && bNoFormulaOrOutcome)
				{
					return true;
				}

				const bool bValidRoute = bValidIntent && Corner.bHasRouteD6
					&& Corner.RawRouteD6 >= 1 && Corner.RawRouteD6 <= 6
					&& Corner.ActualRoute == GetActualCornerRoute(
						Corner.IntendedRoute, Corner.RawRouteD6);
				if (Corner.Stage
					== EMatchPlaySetPieceCornerRouteStage::AwaitingAttackRoll
					&& bActiveLifecycle && bValidRoute && bNoAttack
					&& bNoDefense && bNoFormulaOrOutcome)
				{
					return true;
				}

				const bool bValidAttack = Corner.bHasAttackD6
					&& Corner.AttackD6 >= 1 && Corner.AttackD6 <= 6;
				if (Corner.Stage
					== EMatchPlaySetPieceCornerRouteStage::AwaitingDefenseRoll
					&& bActiveLifecycle && bValidRoute && bValidAttack
					&& bNoDefense && bNoFormulaOrOutcome)
				{
					return true;
				}

				const bool bValidDefense = Corner.bHasDefenseD6
					&& Corner.DefenseD6 >= 1 && Corner.DefenseD6 <= 6;
				if (Corner.Stage
					== EMatchPlaySetPieceCornerRouteStage::Terminal
					&& bTerminalLifecycle && bValidRoute && bValidAttack
					&& bValidDefense && Corner.bHasFormulaResolution)
				{
					const FMatchPlayDefendingGoalkeeperQueryResult Goalkeeper =
						FMatchPlayDefendingGoalkeeperQuery::Query(State, Defender);
					if (Goalkeeper.bSuccess)
					{
						const FFormulaResolverInput FormulaInput =
							BuildCornerFormulaInput(State, Corner,
								Goalkeeper, Attacker, Defender);
						const FFormulaResolutionResult ExpectedFormula =
							UFormulaResolver::ResolveFormula(FormulaInput);
						const bool bGoal = ExpectedFormula.bIsGoal;
						const bool bOutcomeCoherent = Corner.GameplayOutcome
							== (bGoal
								? EMatchPlayCornerGameplayOutcome::Goal
								: EMatchPlayCornerGameplayOutcome::NoGoal);
						const bool bScorerCoherent = bGoal
							? Corner.bHasGoalScorer
								&& Corner.GoalScorerCardId == Corner.Runner.CardId
							: !Corner.bHasGoalScorer
								&& Corner.GoalScorerCardId.IsNone();
						if (bOutcomeCoherent && bScorerCoherent
							&& FFormulaResolutionResult::StaticStruct()
								->CompareScriptStruct(&Corner.FormulaResolution,
									&ExpectedFormula, 0))
						{
							return true;
						}
					}
					SetFailure(Result,
						EMatchPlayCurrentAttackRouteStateValidationErrorCode
							::CornerFormulaMismatch,
						TEXT("Corner terminal Formula, outcome, scorer, and goalkeeper facts must match canonical authority."));
					return false;
				}
			}
		}

		if (Result.ErrorCode
			== EMatchPlayCurrentAttackRouteStateValidationErrorCode::None)
		{
			SetFailure(Result,
				EMatchPlayCurrentAttackRouteStateValidationErrorCode
					::InvalidConcreteSetPieceStage,
				TEXT("Corner stage, locks, nominations, rolls, Formula, outcome, scorer, and lifecycle are incoherent."));
		}
		return false;
	}

	bool HasDefaultOrdinaryPayload(
		const FMatchPlayCurrentAttackState& Attack)
	{
		return Attack.SelectionStage
				== EMatchPlayCurrentAttackSelectionStage::None
			&& IsDefaultStruct(Attack.ActionPreparation)
			&& Attack.CurrentLegalDeploymentSide
				== EInitialTurnOrderPlayer::None
			&& !Attack.bAttackerDeploymentFinished
			&& !Attack.bDefenderDeploymentFinished
			&& Attack.DeploymentPlacements.IsEmpty()
			&& !Attack.bCurrentDefenseGoalkeeperActivated
			&& !Attack.bHasSelectedAction
			&& IsDefaultStruct(Attack.SelectedAction)
			&& !Attack.bHasResolutionSession
			&& IsDefaultStruct(Attack.ResolutionSession);
	}

	int64 GetExpectedAttackSequence(const FMatchPlayState& State)
	{
		return static_cast<int64>(
			State.RuntimeState.PlayerAState.UsedAttackCount)
			+ static_cast<int64>(
				State.RuntimeState.PlayerBState.UsedAttackCount)
			+ 1;
	}
}

FMatchPlayCurrentAttackRouteStateValidationResult
FMatchPlayCurrentAttackRouteStateValidator::Validate(
	const FMatchPlayState& State)
{
	using namespace MatchPlayCurrentAttackRouteStateValidator;

	FMatchPlayCurrentAttackRouteStateValidationResult Result;
	if (!State.bHasCurrentAttack)
	{
		const FMatchPlayCurrentAttackState DefaultAttack;
		if (!FMatchPlayCurrentAttackState::StaticStruct()
				->CompareScriptStruct(
					&State.CurrentAttack,
					&DefaultAttack,
					0))
		{
			SetFailure(
				Result,
				EMatchPlayCurrentAttackRouteStateValidationErrorCode
					::InactiveStateHasPayload,
				TEXT("A state without a current attack must retain the default current-attack payload."));
			return Result;
		}

		Result.bIsCanonical = true;
		return Result;
	}

	const FMatchPlayCurrentAttackState& Attack = State.CurrentAttack;
	if (Attack.AttackSequence <= 0)
	{
		SetFailure(
			Result,
			EMatchPlayCurrentAttackRouteStateValidationErrorCode
				::InvalidAttackSequence,
			TEXT("A current attack requires a positive AttackSequence."));
		return Result;
	}

	if (Attack.AttackSequence != GetExpectedAttackSequence(State))
	{
		SetFailure(
			Result,
			EMatchPlayCurrentAttackRouteStateValidationErrorCode
				::AttackSequenceMismatch,
			TEXT("CurrentAttack.AttackSequence must match the authoritative used-attack counts."));
		return Result;
	}

	if (Attack.RawInitialD12 < 1 || Attack.RawInitialD12 > 12)
	{
		SetFailure(
			Result,
			EMatchPlayCurrentAttackRouteStateValidationErrorCode
				::InvalidRawInitialD12,
			TEXT("RawInitialD12 must be in range [1, 12]."));
		return Result;
	}

	if (Attack.ActionPoint != Attack.RawInitialD12)
	{
		SetFailure(
			Result,
			EMatchPlayCurrentAttackRouteStateValidationErrorCode
				::ActionPointMismatch,
			TEXT("ActionPoint must preserve the raw initial D12 value."));
		return Result;
	}

	switch (Attack.RouteKind)
	{
	case EMatchPlayCurrentAttackRouteKind::Ordinary:
		if (Attack.RawInitialD12 < 2 || Attack.RawInitialD12 > 8)
		{
			SetFailure(
				Result,
				EMatchPlayCurrentAttackRouteStateValidationErrorCode
					::OrdinaryD12Mismatch,
				TEXT("Ordinary route requires an initial D12 in range [2, 8]."));
			return Result;
		}
		if (Attack.Phase == EMatchPlayCurrentAttackPhase::RoutePending)
		{
			SetFailure(Result,
				EMatchPlayCurrentAttackRouteStateValidationErrorCode
					::WrongRoutePhase,
				TEXT("Ordinary route must use its production Deployment or Resolution phase."));
			return Result;
		}
		if (!IsDefaultSendingOffState(Attack.SendingOffRoute))
		{
			SetFailure(
				Result,
				EMatchPlayCurrentAttackRouteStateValidationErrorCode
					::UnexpectedSendingOffPayload,
				TEXT("Ordinary route cannot carry Sending-Off state."));
			return Result;
		}
		if (!IsDefaultSetPieceState(Attack.SetPieceRoute))
		{
			SetFailure(
				Result,
				EMatchPlayCurrentAttackRouteStateValidationErrorCode
					::UnexpectedSetPiecePayload,
				TEXT("Ordinary route cannot carry Set Piece state."));
			return Result;
		}
		break;

	case EMatchPlayCurrentAttackRouteKind::SendingOff:
		if (Attack.RawInitialD12 != 1)
		{
			SetFailure(
				Result,
				EMatchPlayCurrentAttackRouteStateValidationErrorCode
					::SendingOffD12Mismatch,
				TEXT("Sending-Off route requires an initial D12 of 1."));
			return Result;
		}
		if (Attack.Phase != EMatchPlayCurrentAttackPhase::RoutePending)
		{
			SetFailure(Result,
				EMatchPlayCurrentAttackRouteStateValidationErrorCode
					::WrongRoutePhase,
				TEXT("Sending-Off route must remain in RoutePending phase."));
			return Result;
		}
		if (Attack.SendingOffRoute.Stage
			== EMatchPlaySendingOffRouteStage::AwaitingResolution)
		{
			if (Attack.LifecycleState
				!= EMatchPlayCurrentAttackLifecycleState::Active)
			{
				SetFailure(Result,
					EMatchPlayCurrentAttackRouteStateValidationErrorCode
						::WrongRouteLifecycle,
					TEXT("Awaiting Sending-Off resolution must remain active."));
				return Result;
			}
			if (Attack.SendingOffRoute.SelectionOutcome
					!= EMatchPlaySendingOffSelectionOutcome::None
				|| !Attack.SendingOffRoute.EjectedCardId.IsNone()
				|| Attack.SendingOffRoute.GameplayOutcome
					!= EMatchPlaySendingOffGameplayOutcome::None)
			{
				SetFailure(Result,
					EMatchPlayCurrentAttackRouteStateValidationErrorCode
						::InvalidSendingOffPayload,
					TEXT("Awaiting Sending-Off resolution cannot carry terminal result payload."));
				return Result;
			}
		}
		else if (Attack.SendingOffRoute.Stage
			== EMatchPlaySendingOffRouteStage::Resolved)
		{
			if (State.RuntimeState.CurrentAttackingPlayer
					!= EInitialTurnOrderPlayer::PlayerA
				&& State.RuntimeState.CurrentAttackingPlayer
					!= EInitialTurnOrderPlayer::PlayerB)
			{
				SetFailure(Result,
					EMatchPlayCurrentAttackRouteStateValidationErrorCode
						::InvalidSendingOffPayload,
					TEXT("Resolved Sending-Off requires a valid current attacking side."));
				return Result;
			}
			if (Attack.LifecycleState
					!= EMatchPlayCurrentAttackLifecycleState
						::TerminalPendingAdvance
				|| Attack.SendingOffRoute.GameplayOutcome
					!= EMatchPlaySendingOffGameplayOutcome::NoGoal)
			{
				SetFailure(Result,
					EMatchPlayCurrentAttackRouteStateValidationErrorCode
						::InvalidSendingOffPayload,
					TEXT("Resolved Sending-Off must persist NoGoal and await explicit advance."));
				return Result;
			}

			const EMatchPlaySendingOffSelectionOutcome SelectionOutcome =
				Attack.SendingOffRoute.SelectionOutcome;
			if (SelectionOutcome
					== EMatchPlaySendingOffSelectionOutcome
						::NoEligibleCandidate)
			{
				if (!Attack.SendingOffRoute.EjectedCardId.IsNone())
				{
					SetFailure(Result,
						EMatchPlayCurrentAttackRouteStateValidationErrorCode
							::InvalidSendingOffPayload,
						TEXT("NoEligibleCandidate cannot carry an ejected CardId."));
					return Result;
				}
			}
			else if (SelectionOutcome
				== EMatchPlaySendingOffSelectionOutcome::CardEjected)
			{
				const FName EjectedCardId =
					Attack.SendingOffRoute.EjectedCardId;
				const FCardUsageState& AttackerUsage =
					State.RuntimeState.CurrentAttackingPlayer
						== EInitialTurnOrderPlayer::PlayerA
						? State.CardUsageState.PlayerACardUsageState
						: State.CardUsageState.PlayerBCardUsageState;
				if (EjectedCardId.IsNone()
					|| !AttackerUsage.EjectedCardIds.Contains(EjectedCardId)
					|| AttackerUsage.AvailableCardIds.Contains(EjectedCardId)
					|| AttackerUsage.UsedCardIds.Contains(EjectedCardId))
				{
					SetFailure(Result,
						EMatchPlayCurrentAttackRouteStateValidationErrorCode
							::SendingOffEjectionMismatch,
						TEXT("Resolved Sending-Off card must exist only in the attacker's Ejected zone."));
					return Result;
				}
			}
			else
			{
				SetFailure(Result,
					EMatchPlayCurrentAttackRouteStateValidationErrorCode
						::InvalidSendingOffPayload,
					TEXT("Resolved Sending-Off requires an explicit selection outcome."));
				return Result;
			}
		}
		else
		{
			SetFailure(Result,
				EMatchPlayCurrentAttackRouteStateValidationErrorCode
					::InvalidSendingOffStage,
				TEXT("Sending-Off route stage is invalid."));
			return Result;
		}
		if (!IsDefaultSetPieceState(Attack.SetPieceRoute))
		{
			SetFailure(Result,
				EMatchPlayCurrentAttackRouteStateValidationErrorCode
					::UnexpectedSetPiecePayload,
				TEXT("Sending-Off route cannot carry Set Piece state."));
			return Result;
		}
		if (!HasDefaultOrdinaryPayload(Attack))
		{
			SetFailure(Result,
				EMatchPlayCurrentAttackRouteStateValidationErrorCode
					::UnexpectedOrdinaryPayload,
				TEXT("Sending-Off route cannot start ordinary attack state."));
			return Result;
		}
		break;

	case EMatchPlayCurrentAttackRouteKind::SetPiece:
		if (Attack.RawInitialD12 < 9 || Attack.RawInitialD12 > 12)
		{
			SetFailure(Result,
				EMatchPlayCurrentAttackRouteStateValidationErrorCode
					::SetPieceD12Mismatch,
				TEXT("Set Piece route requires an initial D12 in range [9, 12]."));
			return Result;
		}
		if (Attack.LifecycleState
				!= EMatchPlayCurrentAttackLifecycleState::Active
			&& Attack.LifecycleState
				!= EMatchPlayCurrentAttackLifecycleState
					::TerminalPendingAdvance)
		{
			SetFailure(Result,
				EMatchPlayCurrentAttackRouteStateValidationErrorCode
					::WrongRouteLifecycle,
				TEXT("Set Piece route foundation must remain active while concrete resolution is pending."));
			return Result;
		}
		if (Attack.Phase != EMatchPlayCurrentAttackPhase::RoutePending)
		{
			SetFailure(Result,
				EMatchPlayCurrentAttackRouteStateValidationErrorCode
					::WrongRoutePhase,
				TEXT("Set Piece route must remain in RoutePending phase."));
			return Result;
		}
		if (!IsDefaultSendingOffState(Attack.SendingOffRoute))
		{
			SetFailure(Result,
				EMatchPlayCurrentAttackRouteStateValidationErrorCode
					::UnexpectedSendingOffPayload,
				TEXT("Set Piece route cannot carry Sending-Off state."));
			return Result;
		}
		if (!HasDefaultOrdinaryPayload(Attack))
		{
			SetFailure(Result,
				EMatchPlayCurrentAttackRouteStateValidationErrorCode
					::UnexpectedOrdinaryPayload,
				TEXT("Set Piece route cannot start ordinary attack state."));
			return Result;
		}
		if (Attack.SetPieceRoute.Stage
			== EMatchPlaySetPieceRouteStage::AwaitingTypeRoll)
		{
			if (Attack.LifecycleState
				!= EMatchPlayCurrentAttackLifecycleState::Active)
			{
				SetFailure(Result,
					EMatchPlayCurrentAttackRouteStateValidationErrorCode
						::WrongRouteLifecycle,
					TEXT("Awaiting Set Piece type roll must remain active."));
				return Result;
			}
			if (Attack.SetPieceRoute.bHasTypeRoll
				|| Attack.SetPieceRoute.RawTypeD6 != 0
				|| Attack.SetPieceRoute.SelectedType
					!= ESetPieceSelectedType::None)
			{
				SetFailure(Result,
					EMatchPlayCurrentAttackRouteStateValidationErrorCode
						::AwaitingTypeHasResolvedPayload,
					TEXT("AwaitingTypeRoll cannot carry a resolved Set Piece type."));
				return Result;
			}
			if (!HasDefaultConcreteSetPiecePayloads(Attack.SetPieceRoute))
			{
				SetFailure(Result,
					EMatchPlayCurrentAttackRouteStateValidationErrorCode
						::AwaitingTypeHasConcretePayload,
					TEXT("AwaitingTypeRoll cannot carry concrete Set Piece payload."));
				return Result;
			}
		}
		else if (Attack.SetPieceRoute.Stage
			== EMatchPlaySetPieceRouteStage::TypeResolved)
		{
			if (!Attack.SetPieceRoute.bHasTypeRoll)
			{
				SetFailure(Result,
					EMatchPlayCurrentAttackRouteStateValidationErrorCode
						::TypeResolvedMissingRoll,
					TEXT("TypeResolved requires a stored Set Piece type roll."));
				return Result;
			}
			if (Attack.SetPieceRoute.RawTypeD6 < 1
				|| Attack.SetPieceRoute.RawTypeD6 > 6)
			{
				SetFailure(Result,
					EMatchPlayCurrentAttackRouteStateValidationErrorCode
						::InvalidSetPieceTypeRoll,
					TEXT("Resolved Set Piece type D6 must be in range [1, 6]."));
				return Result;
			}
			if (Attack.SetPieceRoute.SelectedType
				== ESetPieceSelectedType::None)
			{
				SetFailure(Result,
					EMatchPlayCurrentAttackRouteStateValidationErrorCode
						::InvalidSetPieceType,
					TEXT("TypeResolved requires a concrete Set Piece type."));
				return Result;
			}

			FSetPieceTypeSelectionQueryInput QueryInput;
			QueryInput.CurrentActionPoint = Attack.RawInitialD12;
			QueryInput.bHasExternalSelectionD6 = true;
			QueryInput.ExternalSelectionD6 = Attack.SetPieceRoute.RawTypeD6;
			const FSetPieceTypeSelectionQueryResult QueryResult =
				FSetPieceTypeSelectionQuery::Select(QueryInput);
			if (!QueryResult.bSuccess
				|| QueryResult.SelectedSetPieceType
					!= Attack.SetPieceRoute.SelectedType)
			{
				SetFailure(Result,
					EMatchPlayCurrentAttackRouteStateValidationErrorCode
						::SetPieceTypeMappingMismatch,
					TEXT("Stored Set Piece type must match the canonical type-roll mapping."));
				return Result;
			}

			if (CountActiveConcreteSetPiecePayloads(
					Attack.SetPieceRoute) != 1)
			{
				SetFailure(Result,
					EMatchPlayCurrentAttackRouteStateValidationErrorCode
						::ConcretePayloadCountMismatch,
					TEXT("TypeResolved Set Piece requires exactly one active concrete payload."));
				return Result;
			}
			if (!HasMatchingActiveConcretePayload(Attack.SetPieceRoute))
			{
				SetFailure(Result,
					EMatchPlayCurrentAttackRouteStateValidationErrorCode
						::ConcretePayloadTypeMismatch,
					TEXT("Active concrete Set Piece payload must match SelectedType."));
				return Result;
			}

			switch (Attack.SetPieceRoute.SelectedType)
			{
			case ESetPieceSelectedType::ShortFreeKick:
				if (!ValidateShortFreeKickPayload(Result, State,
					Attack.SetPieceRoute.ShortFreeKick))
				{
					return Result;
				}
				break;
			case ESetPieceSelectedType::LongFreeKick:
				if (!ValidateLongFreeKickPayload(Result, State,
					Attack.SetPieceRoute.LongFreeKick))
				{
					return Result;
				}
				break;
			case ESetPieceSelectedType::Penalty:
				if (!ValidatePenaltyPayload(Result, State,
					Attack.SetPieceRoute.Penalty))
				{
					return Result;
				}
				break;
			case ESetPieceSelectedType::Corner:
				if (!ValidateCornerPayload(Result, State,
					Attack.SetPieceRoute.Corner))
				{
					return Result;
				}
				break;
			default:
				break;
			}
		}
		else
		{
			SetFailure(Result,
				EMatchPlayCurrentAttackRouteStateValidationErrorCode
					::InvalidSetPieceStage,
				TEXT("Set Piece route requires AwaitingTypeRoll or TypeResolved stage."));
			return Result;
		}
		break;

	case EMatchPlayCurrentAttackRouteKind::None:
	default:
		SetFailure(Result,
			EMatchPlayCurrentAttackRouteStateValidationErrorCode
				::InvalidRouteKind,
			TEXT("An active current attack requires an explicit route kind."));
		return Result;
	}

	Result.bIsCanonical = true;
	return Result;
}
