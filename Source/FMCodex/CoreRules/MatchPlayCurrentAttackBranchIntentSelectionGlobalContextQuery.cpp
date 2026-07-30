#include "MatchPlayCurrentAttackBranchIntentSelectionGlobalContextQuery.h"

#include "MatchPlayElectiveBranchIntentRules.h"

namespace MatchPlayCurrentAttackBranchIntentSelectionGlobalContextImplementation
{
	void SetError(
		FMatchPlayCurrentAttackBranchIntentSelectionGlobalContextResult&
			Result,
		const EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
			ErrorCode,
		const FString& ErrorMessage)
	{
		Result.ErrorCode = ErrorCode;
		Result.ErrorMessage = ErrorMessage;
	}

	bool IsPlayerSide(const EInitialTurnOrderPlayer Side)
	{
		return Side == EInitialTurnOrderPlayer::PlayerA
			|| Side == EInitialTurnOrderPlayer::PlayerB;
	}

	EInitialTurnOrderPlayer GetDefender(
		const EInitialTurnOrderPlayer Attacker)
	{
		if (Attacker == EInitialTurnOrderPlayer::PlayerA)
		{
			return EInitialTurnOrderPlayer::PlayerB;
		}
		if (Attacker == EInitialTurnOrderPlayer::PlayerB)
		{
			return EInitialTurnOrderPlayer::PlayerA;
		}
		return EInitialTurnOrderPlayer::None;
	}

	FString MakeCardKey(
		const EInitialTurnOrderPlayer Side,
		const FName CardId)
	{
		return FString::Printf(
			TEXT("%d:%s"),
			static_cast<int32>(Side),
			*CardId.ToString());
	}
}

FMatchPlayCurrentAttackBranchIntentSelectionGlobalContextResult
FMatchPlayCurrentAttackBranchIntentSelectionGlobalContextQuery::Query(
	const FMatchPlayState& State,
	const int64 AttackSequence,
	const EInitialTurnOrderPlayer RequestingSide)
{
	using namespace
		MatchPlayCurrentAttackBranchIntentSelectionGlobalContextImplementation;

	FMatchPlayCurrentAttackBranchIntentSelectionGlobalContextResult Result;
	Result.RequestedAttackSequence = AttackSequence;
	Result.RequestingSide = RequestingSide;

	if (!State.RuntimeState.bIsInitialized)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
				::MatchPlayStateNotInitialized,
			TEXT("Match play state must be initialized before Branch Intent selection."));
		return Result;
	}
	if (!State.bHasCurrentAttack)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
				::NoCurrentAttack,
			TEXT("Branch Intent selection requires an active current attack."));
		return Result;
	}

	const FMatchPlayCurrentAttackState& Attack = State.CurrentAttack;
	Result.AuthoritativeAttackSequence = Attack.AttackSequence;
	if (Attack.AttackSequence <= 0)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
				::InvalidCurrentAttackSequence,
			TEXT("Current attack sequence must be greater than zero."));
		return Result;
	}
	if (AttackSequence != Attack.AttackSequence)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
				::AttackSequenceMismatch,
			TEXT("Branch Intent sequence does not match the current attack."));
		return Result;
	}
	Result.CurrentAttackingPlayer =
		State.RuntimeState.CurrentAttackingPlayer;
	if (!IsPlayerSide(Result.CurrentAttackingPlayer))
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
				::InvalidCurrentAttackingPlayer,
			TEXT("CurrentAttackingPlayer must be PlayerA or PlayerB."));
		return Result;
	}
	Result.CurrentDefendingPlayer =
		GetDefender(Result.CurrentAttackingPlayer);
	if (!IsPlayerSide(Result.CurrentDefendingPlayer))
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
				::InvalidCurrentDefendingPlayer,
			TEXT("Current defending player could not be derived."));
		return Result;
	}

	if (!IsPlayerSide(RequestingSide))
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
				::InvalidRequestingSide,
			TEXT("RequestingSide must be PlayerA or PlayerB."));
		return Result;
	}
	if (RequestingSide != Result.CurrentAttackingPlayer)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
				::RequestingSideIsNotCurrentAttacker,
			TEXT("Only the current attacker may select Branch Intent."));
		return Result;
	}

	if (Attack.Phase != EMatchPlayCurrentAttackPhase::Resolution)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
				::CurrentAttackNotInResolution,
			TEXT("Branch Intent selection requires Resolution phase."));
		return Result;
	}

	if (Attack.SelectionStage
		!= EMatchPlayCurrentAttackSelectionStage::AwaitingBranchIntent)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
				::WrongSelectionStage,
			TEXT("Branch Intent selection requires AwaitingBranchIntent stage."));
		return Result;
	}

	Result.SelectionStateValidationResult =
		FMatchPlayCurrentAttackSelectionStateValidator::Validate(Attack);
	if (!Result.SelectionStateValidationResult.bIsCanonical)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
				::InvalidSelectionState,
			Result.SelectionStateValidationResult.ErrorMessage);
		return Result;
	}

	Result.Preparation = Attack.ActionPreparation;
	Result.FrozenActionType = Result.Preparation.ActionType;
	if (!MatchPlayElectiveBranchIntentRules::IsElectiveActionType(
			Result.FrozenActionType))
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
				::UnsupportedElectiveIntentActionType,
			TEXT("The frozen action type does not support elective Branch Intent."));
		return Result;
	}

	for (const FMatchPlayDeploymentPlacement& Placement :
		Attack.DeploymentPlacements)
	{
		if (!IsPlayerSide(Placement.PlayerSide)
			|| Placement.CardId.IsNone()
			|| Placement.SlotId.IsNone())
		{
			SetError(
				Result,
				EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
					::InvalidDeploymentPlacement,
				TEXT("Every placement requires side, CardId, and SlotId."));
			return Result;
		}
	}

	TSet<FString> SeenCards;
	TSet<FName> SeenSlots;
	for (const FMatchPlayDeploymentPlacement& Placement :
		Attack.DeploymentPlacements)
	{
		const FString CardKey =
			MakeCardKey(Placement.PlayerSide, Placement.CardId);
		if (SeenCards.Contains(CardKey))
		{
			SetError(
				Result,
				EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
					::DuplicateDeploymentCard,
				TEXT("Deployment CardIds must be unique within each side."));
			return Result;
		}
		SeenCards.Add(CardKey);
		if (SeenSlots.Contains(Placement.SlotId))
		{
			SetError(
				Result,
				EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
					::DuplicateDeploymentSlot,
				TEXT("Deployment SlotIds must be globally unique."));
			return Result;
		}
		SeenSlots.Add(Placement.SlotId);

		if (Placement.PlayerSide == Result.CurrentAttackingPlayer
			&& Placement.CardId
				== Result.Preparation.CarrierCardId)
		{
			++Result.MatchingCarrierPlacementCount;
		}
		if (Placement.PlayerSide == Result.CurrentDefendingPlayer
			&& Placement.CardId
				== Result.Preparation.MarkerCardId)
		{
			++Result.MatchingMarkerPlacementCount;
		}
		if (!Result.Preparation.RunnerCardId.IsNone()
			&& Placement.PlayerSide
				== Result.CurrentAttackingPlayer
			&& Placement.CardId
				== Result.Preparation.RunnerCardId)
		{
			++Result.MatchingRunnerPlacementCount;
		}
	}

	if (Result.MatchingCarrierPlacementCount != 1)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
				::CarrierDeploymentInvalid,
			TEXT("Frozen Carrier must have exactly one attacker placement."));
		return Result;
	}
	if (Result.MatchingMarkerPlacementCount != 1)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
				::MarkerDeploymentInvalid,
			TEXT("Frozen Marker must have exactly one defender placement."));
		return Result;
	}
	if (Result.FrozenActionType == ESkillRuleType::Cross
		&& Result.MatchingRunnerPlacementCount != 1)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
				::RunnerDeploymentInvalid,
			TEXT("Cross requires exactly one frozen attacker Runner placement."));
		return Result;
	}

	Result.CarrierSnapshotQueryResult =
		FMatchPlayCardSnapshotAuthorityQuery::FindByPlayerSideAndCardId(
			State.CardSnapshotAuthority,
			Result.CurrentAttackingPlayer,
			Result.Preparation.CarrierCardId);
	if (!Result.CarrierSnapshotQueryResult.bSuccess)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
				::CarrierSnapshotQueryFailed,
			Result.CarrierSnapshotQueryResult.ErrorMessage);
		return Result;
	}
	Result.MarkerSnapshotQueryResult =
		FMatchPlayCardSnapshotAuthorityQuery::FindByPlayerSideAndCardId(
			State.CardSnapshotAuthority,
			Result.CurrentDefendingPlayer,
			Result.Preparation.MarkerCardId);
	if (!Result.MarkerSnapshotQueryResult.bSuccess)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
				::MarkerSnapshotQueryFailed,
			Result.MarkerSnapshotQueryResult.ErrorMessage);
		return Result;
	}
	if (Result.CarrierSnapshotQueryResult.Snapshot.bIsGoalkeeper
		|| Result.MarkerSnapshotQueryResult.Snapshot.bIsGoalkeeper)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
				::RequiredParticipantIsGoalkeeper,
			TEXT("Frozen Carrier and Marker must be ordinary players."));
		return Result;
	}

	if (Result.FrozenActionType == ESkillRuleType::Cross)
	{
		Result.RunnerSnapshotQueryResult =
			FMatchPlayCardSnapshotAuthorityQuery
				::FindByPlayerSideAndCardId(
					State.CardSnapshotAuthority,
					Result.CurrentAttackingPlayer,
					Result.Preparation.RunnerCardId);
		if (!Result.RunnerSnapshotQueryResult.bSuccess)
		{
			SetError(
				Result,
				EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
					::RunnerSnapshotQueryFailed,
				Result.RunnerSnapshotQueryResult.ErrorMessage);
			return Result;
		}
		if (Result.RunnerSnapshotQueryResult.Snapshot.bIsGoalkeeper)
		{
			SetError(
				Result,
				EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
					::RequiredParticipantIsGoalkeeper,
				TEXT("Frozen Runner must be an ordinary player."));
			return Result;
		}
		if (Result.Preparation.RunnerCardId
			== Result.Preparation.CarrierCardId)
		{
			SetError(
				Result,
				EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
					::RunnerMatchesCarrier,
				TEXT("Frozen Runner must differ from Carrier."));
			return Result;
		}
		if (!Result.RunnerSnapshotQueryResult.Snapshot.PositionTypes
				.Contains(EPlayerPositionType::Attack))
		{
			SetError(
				Result,
				EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
					::RunnerMissingRequiredPositionType,
				TEXT("Cross Runner must include Attack position."));
			return Result;
		}
		if (Result.Preparation.bHasHelper)
		{
			Result.HelperAuthorityResult =
				FMatchPlayCurrentAttackHelperParticipantAuthority::Evaluate(
					State,
					Result.CurrentDefendingPlayer,
					Result.Preparation.MarkerCardId,
					Result.Preparation.HelperCardId);
			if (!Result.HelperAuthorityResult.bSuccess)
			{
				SetError(
					Result,
					EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
						::HelperAuthorityFailed,
					Result.HelperAuthorityResult.ErrorMessage);
				return Result;
			}
		}
	}

	Result.bSuccess = true;
	Result.ErrorCode =
		EMatchPlayCurrentAttackBranchIntentSelectionErrorCode::None;
	Result.ErrorMessage.Empty();
	return Result;
}
