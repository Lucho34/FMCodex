#include "MatchPlayCurrentAttackRunnerSelectionGlobalContextQuery.h"

namespace MatchPlayCurrentAttackRunnerSelectionGlobalContextImplementation
{
	void SetError(
		FMatchPlayCurrentAttackRunnerSelectionGlobalContextResult& Result,
		const EMatchPlayCurrentAttackRunnerSelectionErrorCode ErrorCode,
		const FString& ErrorMessage)
	{
		Result.ErrorCode = ErrorCode;
		Result.ErrorMessage = ErrorMessage;
	}

	bool IsPlayerSide(const EInitialTurnOrderPlayer PlayerSide)
	{
		return PlayerSide == EInitialTurnOrderPlayer::PlayerA
			|| PlayerSide == EInitialTurnOrderPlayer::PlayerB;
	}

	EInitialTurnOrderPlayer GetDefendingPlayer(
		const EInitialTurnOrderPlayer AttackingPlayer)
	{
		if (AttackingPlayer == EInitialTurnOrderPlayer::PlayerA)
		{
			return EInitialTurnOrderPlayer::PlayerB;
		}
		if (AttackingPlayer == EInitialTurnOrderPlayer::PlayerB)
		{
			return EInitialTurnOrderPlayer::PlayerA;
		}
		return EInitialTurnOrderPlayer::None;
	}

	FString MakeDeploymentCardKey(
		const EInitialTurnOrderPlayer PlayerSide,
		const FName CardId)
	{
		return FString::Printf(
			TEXT("%d:%s"),
			static_cast<int32>(PlayerSide),
			*CardId.ToString());
	}
}

FMatchPlayCurrentAttackRunnerSelectionGlobalContextResult
FMatchPlayCurrentAttackRunnerSelectionGlobalContextQuery::Query(
	const FMatchPlayState& State,
	const int64 AttackSequence,
	const EInitialTurnOrderPlayer RequestingSide)
{
	using namespace
		MatchPlayCurrentAttackRunnerSelectionGlobalContextImplementation;

	FMatchPlayCurrentAttackRunnerSelectionGlobalContextResult Result;
	Result.RequestedAttackSequence = AttackSequence;
	Result.RequestingSide = RequestingSide;

	if (!State.RuntimeState.bIsInitialized)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackRunnerSelectionErrorCode
				::MatchPlayStateNotInitialized,
			TEXT("Match play state must be initialized before runner selection."));
		return Result;
	}
	if (!State.bHasCurrentAttack)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackRunnerSelectionErrorCode
				::NoCurrentAttack,
			TEXT("Runner selection requires an active current attack."));
		return Result;
	}

	const FMatchPlayCurrentAttackState& CurrentAttack =
		State.CurrentAttack;
	Result.AuthoritativeAttackSequence =
		CurrentAttack.AttackSequence;
	if (CurrentAttack.AttackSequence <= 0)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackRunnerSelectionErrorCode
				::InvalidCurrentAttackSequence,
			TEXT("Current attack sequence must be greater than zero."));
		return Result;
	}
	if (AttackSequence != CurrentAttack.AttackSequence)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackRunnerSelectionErrorCode
				::AttackSequenceMismatch,
			TEXT("Runner selection attack sequence does not match the current attack."));
		return Result;
	}
	if (CurrentAttack.Phase != EMatchPlayCurrentAttackPhase::Resolution)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackRunnerSelectionErrorCode
				::CurrentAttackNotInResolution,
			TEXT("Current attack must be in Resolution phase for runner selection."));
		return Result;
	}

	Result.CurrentAttackingPlayer =
		State.RuntimeState.CurrentAttackingPlayer;
	if (!IsPlayerSide(Result.CurrentAttackingPlayer))
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackRunnerSelectionErrorCode
				::InvalidCurrentAttackingPlayer,
			TEXT("CurrentAttackingPlayer must be PlayerA or PlayerB."));
		return Result;
	}
	Result.CurrentDefendingPlayer =
		GetDefendingPlayer(Result.CurrentAttackingPlayer);
	if (!IsPlayerSide(Result.CurrentDefendingPlayer))
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackRunnerSelectionErrorCode
				::InvalidCurrentDefendingPlayer,
			TEXT("Current defending player could not be derived."));
		return Result;
	}

	Result.SelectionStateValidationResult =
		FMatchPlayCurrentAttackSelectionStateValidator::Validate(
			CurrentAttack);
	if (!Result.SelectionStateValidationResult.bIsCanonical)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackRunnerSelectionErrorCode
				::InvalidSelectionState,
			Result.SelectionStateValidationResult.ErrorMessage);
		return Result;
	}
	if (CurrentAttack.SelectionStage
		!= EMatchPlayCurrentAttackSelectionStage::AwaitingRunner)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackRunnerSelectionErrorCode
				::WrongSelectionStage,
			TEXT("Runner selection requires AwaitingRunner stage."));
		return Result;
	}
	if (!IsPlayerSide(RequestingSide))
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackRunnerSelectionErrorCode
				::InvalidRequestingSide,
			TEXT("RequestingSide must be PlayerA or PlayerB."));
		return Result;
	}
	if (RequestingSide != Result.CurrentAttackingPlayer)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackRunnerSelectionErrorCode
				::RequestingSideIsNotCurrentAttacker,
			TEXT("Only the current attacker may select the runner."));
		return Result;
	}

	Result.FrozenCarrierCardId =
		CurrentAttack.ActionPreparation.CarrierCardId;
	Result.FrozenMarkerCardId =
		CurrentAttack.ActionPreparation.MarkerCardId;
	Result.FrozenSkillId =
		CurrentAttack.ActionPreparation.SkillId;
	Result.FrozenActionType =
		CurrentAttack.ActionPreparation.ActionType;
	if (Result.FrozenCarrierCardId.IsNone())
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackRunnerSelectionErrorCode
				::InvalidFrozenCarrierCardId,
			TEXT("The frozen preparation carrier must be non-empty."));
		return Result;
	}
	if (Result.FrozenMarkerCardId.IsNone())
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackRunnerSelectionErrorCode
				::InvalidFrozenMarkerCardId,
			TEXT("The frozen preparation marker must be non-empty."));
		return Result;
	}
	if (Result.FrozenSkillId.IsNone())
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackRunnerSelectionErrorCode
				::InvalidFrozenSkillId,
			TEXT("The frozen preparation skill must be non-empty."));
		return Result;
	}

	Result.ParticipantRequirementResult =
		FMatchPlaySkillParticipantRequirementQuery::Query(
			Result.FrozenActionType);
	if (!Result.ParticipantRequirementResult.bSuccess)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackRunnerSelectionErrorCode
				::ParticipantRequirementResolutionFailed,
			Result.ParticipantRequirementResult.ErrorMessage);
		return Result;
	}
	if (!Result.ParticipantRequirementResult.bRequiresRunner
		|| !Result.ParticipantRequirementResult.bRequiresHelperStage
		|| Result.ParticipantRequirementResult
			.bCanBecomeReadyImmediately)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackRunnerSelectionErrorCode
				::ParticipantRequirementMismatch,
			TEXT("AwaitingRunner requires a runner-and-helper action type."));
		return Result;
	}

	TSet<FString> SeenDeploymentCards;
	TSet<FName> SeenDeploymentSlots;
	for (const FMatchPlayDeploymentPlacement& Placement :
		CurrentAttack.DeploymentPlacements)
	{
		if (!IsPlayerSide(Placement.PlayerSide)
			|| Placement.CardId.IsNone()
			|| Placement.SlotId.IsNone())
		{
			SetError(
				Result,
				EMatchPlayCurrentAttackRunnerSelectionErrorCode
					::InvalidDeploymentPlacement,
				TEXT("Every deployment placement requires a valid side, CardId, and SlotId."));
			return Result;
		}

		const FString CardKey =
			MakeDeploymentCardKey(
				Placement.PlayerSide,
				Placement.CardId);
		if (SeenDeploymentCards.Contains(CardKey))
		{
			SetError(
				Result,
				EMatchPlayCurrentAttackRunnerSelectionErrorCode
					::DuplicateDeploymentCard,
				TEXT("Deployment CardIds must be unique within each player side."));
			return Result;
		}
		SeenDeploymentCards.Add(CardKey);

		if (SeenDeploymentSlots.Contains(Placement.SlotId))
		{
			SetError(
				Result,
				EMatchPlayCurrentAttackRunnerSelectionErrorCode
					::DuplicateDeploymentSlot,
				TEXT("Deployment SlotIds must be globally unique."));
			return Result;
		}
		SeenDeploymentSlots.Add(Placement.SlotId);

		if (Placement.PlayerSide
			== Result.CurrentAttackingPlayer)
		{
			Result.AttackingPlayerPlacements.Add(Placement);
			if (Placement.CardId
				== Result.FrozenCarrierCardId)
			{
				++Result.MatchingFrozenCarrierPlacementCount;
				Result.FrozenCarrierPlacement = Placement;
			}
		}
		else if (
			Placement.PlayerSide
				== Result.CurrentDefendingPlayer
			&& Placement.CardId
				== Result.FrozenMarkerCardId)
		{
			++Result.MatchingFrozenMarkerPlacementCount;
			Result.FrozenMarkerPlacement = Placement;
		}
	}

	if (Result.MatchingFrozenCarrierPlacementCount == 0)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackRunnerSelectionErrorCode
				::FrozenCarrierNotDeployed,
			TEXT("The frozen carrier must be deployed by the current attacker."));
		return Result;
	}
	if (Result.MatchingFrozenCarrierPlacementCount > 1)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackRunnerSelectionErrorCode
				::FrozenCarrierDeploymentAmbiguous,
			TEXT("The frozen carrier has multiple current-attacker placements."));
		return Result;
	}
	if (Result.MatchingFrozenMarkerPlacementCount == 0)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackRunnerSelectionErrorCode
				::FrozenMarkerNotDeployed,
			TEXT("The frozen marker must be deployed by the current defender."));
		return Result;
	}
	if (Result.MatchingFrozenMarkerPlacementCount > 1)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackRunnerSelectionErrorCode
				::FrozenMarkerDeploymentAmbiguous,
			TEXT("The frozen marker has multiple current-defender placements."));
		return Result;
	}

	const FPlayerCardRuleSnapshotSet& AttackingSnapshots =
		Result.CurrentAttackingPlayer
				== EInitialTurnOrderPlayer::PlayerA
			? State.CardSnapshotAuthority.PlayerACardSnapshots
			: State.CardSnapshotAuthority.PlayerBCardSnapshots;
	Result.AttackingSnapshotSetValidationResult =
		FPlayerCardRuleSnapshotValidator::Validate(
			AttackingSnapshots);
	if (!Result.AttackingSnapshotSetValidationResult.bSuccess)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackRunnerSelectionErrorCode
				::InvalidAttackingSnapshotSet,
			Result.AttackingSnapshotSetValidationResult.ErrorMessage);
		return Result;
	}

	if (Result.FrozenActionType == ESkillRuleType::ThroughBall)
	{
		Result.SlotCatalogValidationResult =
			FMatchPlayDeploymentSlotCatalogValidator::Validate(
				State.DeploymentSlotCatalog);
		if (!Result.SlotCatalogValidationResult.bSuccess)
		{
			SetError(
				Result,
				EMatchPlayCurrentAttackRunnerSelectionErrorCode
					::InvalidSlotCatalog,
				Result.SlotCatalogValidationResult.ErrorMessage);
			return Result;
		}
	}

	Result.bSuccess = true;
	Result.ErrorCode =
		EMatchPlayCurrentAttackRunnerSelectionErrorCode::None;
	Result.ErrorMessage.Empty();
	return Result;
}
