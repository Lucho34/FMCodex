#include "MatchPlayCurrentAttackHelperSelectionGlobalContextQuery.h"

namespace MatchPlayCurrentAttackHelperSelectionGlobalContextImplementation
{
	void SetError(
		FMatchPlayCurrentAttackHelperSelectionGlobalContextResult& Result,
		const EMatchPlayCurrentAttackHelperSelectionErrorCode ErrorCode,
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

	FString MakePlacementKey(
		const FMatchPlayDeploymentPlacement& Placement)
	{
		return FString::Printf(
			TEXT("%d:%s:%s"),
			static_cast<int32>(Placement.PlayerSide),
			*Placement.CardId.ToString(),
			*Placement.SlotId.ToString());
	}
}

FMatchPlayCurrentAttackHelperSelectionGlobalContextResult
FMatchPlayCurrentAttackHelperSelectionGlobalContextQuery::Query(
	const FMatchPlayState& State,
	const int64 AttackSequence,
	const EInitialTurnOrderPlayer RequestingSide)
{
	using namespace
		MatchPlayCurrentAttackHelperSelectionGlobalContextImplementation;

	FMatchPlayCurrentAttackHelperSelectionGlobalContextResult Result;
	Result.RequestedAttackSequence = AttackSequence;
	Result.RequestingSide = RequestingSide;

	if (!State.RuntimeState.bIsInitialized)
	{
		SetError(Result,
			EMatchPlayCurrentAttackHelperSelectionErrorCode
				::MatchPlayStateNotInitialized,
			TEXT("Match play state must be initialized before Helper selection."));
		return Result;
	}
	if (!State.bHasCurrentAttack)
	{
		SetError(Result,
			EMatchPlayCurrentAttackHelperSelectionErrorCode
				::NoCurrentAttack,
			TEXT("Helper selection requires an active current attack."));
		return Result;
	}

	const FMatchPlayCurrentAttackState& Attack = State.CurrentAttack;
	Result.AuthoritativeAttackSequence = Attack.AttackSequence;
	if (Attack.AttackSequence <= 0)
	{
		SetError(Result,
			EMatchPlayCurrentAttackHelperSelectionErrorCode
				::InvalidCurrentAttackSequence,
			TEXT("Current attack sequence must be greater than zero."));
		return Result;
	}
	if (AttackSequence != Attack.AttackSequence)
	{
		SetError(Result,
			EMatchPlayCurrentAttackHelperSelectionErrorCode
				::AttackSequenceMismatch,
			TEXT("Helper selection sequence does not match the current attack."));
		return Result;
	}
	if (Attack.Phase != EMatchPlayCurrentAttackPhase::Resolution)
	{
		SetError(Result,
			EMatchPlayCurrentAttackHelperSelectionErrorCode
				::CurrentAttackNotInResolution,
			TEXT("Helper selection requires Resolution phase."));
		return Result;
	}

	Result.CurrentAttackingPlayer =
		State.RuntimeState.CurrentAttackingPlayer;
	if (!IsPlayerSide(Result.CurrentAttackingPlayer))
	{
		SetError(Result,
			EMatchPlayCurrentAttackHelperSelectionErrorCode
				::InvalidCurrentAttackingPlayer,
			TEXT("CurrentAttackingPlayer must be PlayerA or PlayerB."));
		return Result;
	}
	Result.CurrentDefendingPlayer =
		GetDefender(Result.CurrentAttackingPlayer);
	if (!IsPlayerSide(Result.CurrentDefendingPlayer))
	{
		SetError(Result,
			EMatchPlayCurrentAttackHelperSelectionErrorCode
				::InvalidCurrentDefendingPlayer,
			TEXT("Current defending player could not be derived."));
		return Result;
	}

	Result.SelectionStateValidationResult =
		FMatchPlayCurrentAttackSelectionStateValidator::Validate(Attack);
	if (!Result.SelectionStateValidationResult.bIsCanonical)
	{
		SetError(Result,
			EMatchPlayCurrentAttackHelperSelectionErrorCode
				::InvalidSelectionState,
			Result.SelectionStateValidationResult.ErrorMessage);
		return Result;
	}
	if (Attack.SelectionStage
		!= EMatchPlayCurrentAttackSelectionStage::AwaitingHelper)
	{
		SetError(Result,
			EMatchPlayCurrentAttackHelperSelectionErrorCode
				::WrongSelectionStage,
			TEXT("Helper selection requires AwaitingHelper stage."));
		return Result;
	}
	if (!IsPlayerSide(RequestingSide))
	{
		SetError(Result,
			EMatchPlayCurrentAttackHelperSelectionErrorCode
				::InvalidRequestingSide,
			TEXT("RequestingSide must be PlayerA or PlayerB."));
		return Result;
	}
	if (RequestingSide != Result.CurrentDefendingPlayer)
	{
		SetError(Result,
			EMatchPlayCurrentAttackHelperSelectionErrorCode
				::RequestingSideIsNotCurrentDefender,
			TEXT("Only the current defender may select the Helper."));
		return Result;
	}

	Result.FrozenCarrierCardId =
		Attack.ActionPreparation.CarrierCardId;
	Result.FrozenMarkerCardId =
		Attack.ActionPreparation.MarkerCardId;
	Result.FrozenSkillId = Attack.ActionPreparation.SkillId;
	Result.FrozenActionType = Attack.ActionPreparation.ActionType;
	Result.FrozenRunnerCardId =
		Attack.ActionPreparation.RunnerCardId;
	if (Result.FrozenCarrierCardId.IsNone())
	{
		SetError(Result,
			EMatchPlayCurrentAttackHelperSelectionErrorCode
				::InvalidFrozenCarrierCardId,
			TEXT("Frozen CarrierCardId must be non-empty."));
		return Result;
	}
	if (Result.FrozenMarkerCardId.IsNone())
	{
		SetError(Result,
			EMatchPlayCurrentAttackHelperSelectionErrorCode
				::InvalidFrozenMarkerCardId,
			TEXT("Frozen MarkerCardId must be non-empty."));
		return Result;
	}
	const bool bParticipantFirstPreparation =
		Attack.ActionPreparation.bSkillSelectionDeferred
		&& Result.FrozenSkillId.IsNone()
		&& Result.FrozenActionType == ESkillRuleType::None;
	if (Result.FrozenSkillId.IsNone() && !bParticipantFirstPreparation)
	{
		SetError(Result,
			EMatchPlayCurrentAttackHelperSelectionErrorCode
				::InvalidFrozenSkillId,
			TEXT("Frozen SkillId may be empty only for explicit deferred-Skill authority."));
		return Result;
	}
	if (Result.FrozenActionType == ESkillRuleType::None
		&& !bParticipantFirstPreparation)
	{
		SetError(Result,
			EMatchPlayCurrentAttackHelperSelectionErrorCode
				::InvalidFrozenActionType,
			TEXT("Frozen ActionType must be non-empty."));
		return Result;
	}
	if (Result.FrozenRunnerCardId.IsNone())
	{
		SetError(Result,
			EMatchPlayCurrentAttackHelperSelectionErrorCode
				::InvalidFrozenRunnerCardId,
			TEXT("Frozen RunnerCardId must be non-empty."));
		return Result;
	}

	if (!bParticipantFirstPreparation)
	{
		Result.ParticipantRequirementResult =
			FMatchPlaySkillParticipantRequirementQuery::Query(
				Result.FrozenActionType);
		if (!Result.ParticipantRequirementResult.bSuccess)
		{
			SetError(Result,
				EMatchPlayCurrentAttackHelperSelectionErrorCode
					::ParticipantRequirementResolutionFailed,
				Result.ParticipantRequirementResult.ErrorMessage);
			return Result;
		}
		if (!Result.ParticipantRequirementResult.bRequiresRunner
			|| !Result.ParticipantRequirementResult.bRequiresHelperStage
			|| Result.ParticipantRequirementResult.bCanBecomeReadyImmediately)
		{
			SetError(Result,
				EMatchPlayCurrentAttackHelperSelectionErrorCode
					::ParticipantRequirementMismatch,
				TEXT("AwaitingHelper requires participant-first preparation or a runner-and-helper action type."));
			return Result;
		}
	}

	for (const FMatchPlayDeploymentPlacement& Placement :
		Attack.DeploymentPlacements)
	{
		if (!IsPlayerSide(Placement.PlayerSide)
			|| Placement.CardId.IsNone()
			|| Placement.SlotId.IsNone())
		{
			SetError(Result,
				EMatchPlayCurrentAttackHelperSelectionErrorCode
					::InvalidDeploymentPlacement,
				TEXT("Every placement requires a valid side, CardId, and SlotId."));
			return Result;
		}
	}

	TSet<FString> SeenCards;
	for (const FMatchPlayDeploymentPlacement& Placement :
		Attack.DeploymentPlacements)
	{
		const FString Key =
			MakeCardKey(Placement.PlayerSide, Placement.CardId);
		if (SeenCards.Contains(Key))
		{
			SetError(Result,
				EMatchPlayCurrentAttackHelperSelectionErrorCode
					::DuplicateDeploymentCard,
				TEXT("Deployment CardIds must be unique within each side."));
			return Result;
		}
		SeenCards.Add(Key);
	}

	TSet<FName> SeenSlots;
	for (const FMatchPlayDeploymentPlacement& Placement :
		Attack.DeploymentPlacements)
	{
		if (SeenSlots.Contains(Placement.SlotId))
		{
			SetError(Result,
				EMatchPlayCurrentAttackHelperSelectionErrorCode
					::DuplicateDeploymentSlot,
				TEXT("Deployment SlotIds must be globally unique."));
			return Result;
		}
		SeenSlots.Add(Placement.SlotId);
	}

	TSet<FString> SeenPlacements;
	for (const FMatchPlayDeploymentPlacement& Placement :
		Attack.DeploymentPlacements)
	{
		const FString Key = MakePlacementKey(Placement);
		if (SeenPlacements.Contains(Key))
		{
			SetError(Result,
				EMatchPlayCurrentAttackHelperSelectionErrorCode
					::DuplicateDeploymentPlacement,
				TEXT("Deployment placements must be unique."));
			return Result;
		}
		SeenPlacements.Add(Key);

		if (Placement.PlayerSide == Result.CurrentDefendingPlayer)
		{
			Result.DefendingPlayerPlacements.Add(Placement);
			if (Placement.CardId == Result.FrozenMarkerCardId)
			{
				++Result.MatchingFrozenMarkerPlacementCount;
				Result.FrozenMarkerPlacement = Placement;
			}
		}
		if (Placement.PlayerSide == Result.CurrentAttackingPlayer)
		{
			if (Placement.CardId == Result.FrozenCarrierCardId)
			{
				++Result.MatchingFrozenCarrierPlacementCount;
				Result.FrozenCarrierPlacement = Placement;
			}
			if (Placement.CardId == Result.FrozenRunnerCardId)
			{
				++Result.MatchingFrozenRunnerPlacementCount;
				Result.FrozenRunnerPlacement = Placement;
			}
		}
	}

	auto RequirePlacement = [&Result](
		const int32 Count,
		const EMatchPlayCurrentAttackHelperSelectionErrorCode Missing,
		const EMatchPlayCurrentAttackHelperSelectionErrorCode Ambiguous,
		const TCHAR* Label) -> bool
	{
		if (Count == 0)
		{
			SetError(Result, Missing,
				FString::Printf(TEXT("Frozen %s is not deployed."), Label));
			return false;
		}
		if (Count > 1)
		{
			SetError(Result, Ambiguous,
				FString::Printf(TEXT("Frozen %s deployment is ambiguous."), Label));
			return false;
		}
		return true;
	};
	if (!RequirePlacement(
			Result.MatchingFrozenCarrierPlacementCount,
			EMatchPlayCurrentAttackHelperSelectionErrorCode
				::FrozenCarrierNotDeployed,
			EMatchPlayCurrentAttackHelperSelectionErrorCode
				::FrozenCarrierDeploymentAmbiguous,
			TEXT("Carrier")))
	{
		return Result;
	}
	if (!RequirePlacement(
			Result.MatchingFrozenMarkerPlacementCount,
			EMatchPlayCurrentAttackHelperSelectionErrorCode
				::FrozenMarkerNotDeployed,
			EMatchPlayCurrentAttackHelperSelectionErrorCode
				::FrozenMarkerDeploymentAmbiguous,
			TEXT("Marker")))
	{
		return Result;
	}
	if (!RequirePlacement(
			Result.MatchingFrozenRunnerPlacementCount,
			EMatchPlayCurrentAttackHelperSelectionErrorCode
				::FrozenRunnerNotDeployed,
			EMatchPlayCurrentAttackHelperSelectionErrorCode
				::FrozenRunnerDeploymentAmbiguous,
			TEXT("Runner")))
	{
		return Result;
	}

	const FPlayerCardRuleSnapshotSet& AttackingSnapshots =
		Result.CurrentAttackingPlayer == EInitialTurnOrderPlayer::PlayerA
			? State.CardSnapshotAuthority.PlayerACardSnapshots
			: State.CardSnapshotAuthority.PlayerBCardSnapshots;
	Result.AttackingSnapshotSetValidationResult =
		FPlayerCardRuleSnapshotValidator::Validate(AttackingSnapshots);
	if (!Result.AttackingSnapshotSetValidationResult.bSuccess)
	{
		SetError(Result,
			EMatchPlayCurrentAttackHelperSelectionErrorCode
				::InvalidAttackingSnapshotSet,
			Result.AttackingSnapshotSetValidationResult.ErrorMessage);
		return Result;
	}

	const FPlayerCardRuleSnapshotSet& DefendingSnapshots =
		Result.CurrentDefendingPlayer == EInitialTurnOrderPlayer::PlayerA
			? State.CardSnapshotAuthority.PlayerACardSnapshots
			: State.CardSnapshotAuthority.PlayerBCardSnapshots;
	Result.DefendingSnapshotSetValidationResult =
		FPlayerCardRuleSnapshotValidator::Validate(DefendingSnapshots);
	if (!Result.DefendingSnapshotSetValidationResult.bSuccess)
	{
		SetError(Result,
			EMatchPlayCurrentAttackHelperSelectionErrorCode
				::InvalidDefendingSnapshotSet,
			Result.DefendingSnapshotSetValidationResult.ErrorMessage);
		return Result;
	}

	Result.bSuccess = true;
	Result.ErrorCode =
		EMatchPlayCurrentAttackHelperSelectionErrorCode::None;
	return Result;
}
