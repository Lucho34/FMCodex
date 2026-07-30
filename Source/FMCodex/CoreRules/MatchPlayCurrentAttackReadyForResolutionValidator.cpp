#include "MatchPlayCurrentAttackReadyForResolutionValidator.h"

#include "MatchPlayElectiveBranchIntentRules.h"

namespace MatchPlayCurrentAttackReadyForResolutionImplementation
{
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

FMatchPlayCurrentAttackReadyValidationResult
FMatchPlayCurrentAttackReadyForResolutionValidator::Validate(
	const FMatchPlayState& State)
{
	using namespace
		MatchPlayCurrentAttackReadyForResolutionImplementation;

	FMatchPlayCurrentAttackReadyValidationResult Result;
	auto Fail = [&Result](
		const EMatchPlayCurrentAttackReadyValidationErrorCode Error,
		const FString& Message)
	{
		Result.ErrorCode = Error;
		Result.ErrorMessage = Message;
	};

	if (!State.RuntimeState.bIsInitialized)
	{
		Fail(
			EMatchPlayCurrentAttackReadyValidationErrorCode
				::MatchPlayStateNotInitialized,
			TEXT("Ready validation requires initialized match play state."));
		return Result;
	}
	if (!State.bHasCurrentAttack)
	{
		Fail(
			EMatchPlayCurrentAttackReadyValidationErrorCode
				::NoCurrentAttack,
			TEXT("Ready validation requires an active current attack."));
		return Result;
	}

	const FMatchPlayCurrentAttackState& Attack = State.CurrentAttack;
	if (Attack.AttackSequence <= 0)
	{
		Fail(
			EMatchPlayCurrentAttackReadyValidationErrorCode
				::InvalidCurrentAttackSequence,
			TEXT("Ready attack sequence must be greater than zero."));
		return Result;
	}
	if (Attack.Phase != EMatchPlayCurrentAttackPhase::Resolution)
	{
		Fail(
			EMatchPlayCurrentAttackReadyValidationErrorCode
				::CurrentAttackNotInResolution,
			TEXT("Ready validation requires Resolution phase."));
		return Result;
	}
	if (Attack.SelectionStage
			== EMatchPlayCurrentAttackSelectionStage::ReadyForResolution
		&& !MatchPlayElectiveBranchIntentRules::IsLegalIntent(
			Attack.SelectedAction.ActionType,
			Attack.SelectedAction.ElectiveBranchIntent))
	{
		Fail(
			EMatchPlayCurrentAttackReadyValidationErrorCode
				::InvalidElectiveBranchIntent,
			TEXT("Ready selected action branch intent does not match its action type."));
		return Result;
	}

	Result.SelectionStateValidationResult =
		FMatchPlayCurrentAttackSelectionStateValidator::Validate(Attack);
	if (!Result.SelectionStateValidationResult.bIsCanonical)
	{
		Fail(
			EMatchPlayCurrentAttackReadyValidationErrorCode
				::InvalidSelectionState,
			Result.SelectionStateValidationResult.ErrorMessage);
		return Result;
	}
	if (Attack.SelectionStage
		!= EMatchPlayCurrentAttackSelectionStage::ReadyForResolution)
	{
		Fail(
			EMatchPlayCurrentAttackReadyValidationErrorCode
				::WrongSelectionStage,
			TEXT("Full-state Ready validation requires ReadyForResolution."));
		return Result;
	}

	Result.CurrentAttackingPlayer =
		State.RuntimeState.CurrentAttackingPlayer;
	if (!IsPlayerSide(Result.CurrentAttackingPlayer))
	{
		Fail(
			EMatchPlayCurrentAttackReadyValidationErrorCode
				::InvalidCurrentAttackingPlayer,
			TEXT("CurrentAttackingPlayer must be PlayerA or PlayerB."));
		return Result;
	}
	Result.CurrentDefendingPlayer =
		GetDefender(Result.CurrentAttackingPlayer);
	if (!IsPlayerSide(Result.CurrentDefendingPlayer))
	{
		Fail(
			EMatchPlayCurrentAttackReadyValidationErrorCode
				::InvalidCurrentDefendingPlayer,
			TEXT("Current defender could not be derived."));
		return Result;
	}

	for (const FMatchPlayDeploymentPlacement& Placement :
		Attack.DeploymentPlacements)
	{
		if (!IsPlayerSide(Placement.PlayerSide)
			|| Placement.CardId.IsNone()
			|| Placement.SlotId.IsNone())
		{
			Fail(
				EMatchPlayCurrentAttackReadyValidationErrorCode
					::InvalidDeploymentPlacement,
				TEXT("Every Ready placement requires side, CardId, and SlotId."));
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
			Fail(
				EMatchPlayCurrentAttackReadyValidationErrorCode
					::DuplicateDeploymentCard,
				TEXT("Ready placements contain a same-side duplicate CardId."));
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
			Fail(
				EMatchPlayCurrentAttackReadyValidationErrorCode
					::DuplicateDeploymentSlot,
				TEXT("Ready placements contain a duplicate SlotId."));
			return Result;
		}
		SeenSlots.Add(Placement.SlotId);
	}

	const FMatchPlayCurrentAttackSelectedAction& Selected =
		Attack.SelectedAction;
	int32 CarrierCount = 0;
	int32 MarkerCount = 0;
	int32 RunnerCount = 0;
	FMatchPlayDeploymentPlacement RunnerPlacement;
	for (const FMatchPlayDeploymentPlacement& Placement :
		Attack.DeploymentPlacements)
	{
		if (Placement.PlayerSide == Result.CurrentAttackingPlayer
			&& Placement.CardId == Selected.CarrierCardId)
		{
			++CarrierCount;
		}
		if (Placement.PlayerSide == Result.CurrentDefendingPlayer
			&& Placement.CardId == Selected.MarkerCardId)
		{
			++MarkerCount;
		}
		if (!Selected.RunnerCardId.IsNone()
			&& Placement.PlayerSide == Result.CurrentAttackingPlayer
			&& Placement.CardId == Selected.RunnerCardId)
		{
			++RunnerCount;
			RunnerPlacement = Placement;
		}
	}
	if (CarrierCount != 1)
	{
		Fail(
			EMatchPlayCurrentAttackReadyValidationErrorCode
				::CarrierDeploymentInvalid,
			TEXT("Ready Carrier must have exactly one attacker placement."));
		return Result;
	}
	if (MarkerCount != 1)
	{
		Fail(
			EMatchPlayCurrentAttackReadyValidationErrorCode
				::MarkerDeploymentInvalid,
			TEXT("Ready Marker must have exactly one defender placement."));
		return Result;
	}

	Result.CarrierSnapshotQueryResult =
		FMatchPlayCardSnapshotAuthorityQuery::FindByPlayerSideAndCardId(
			State.CardSnapshotAuthority,
			Result.CurrentAttackingPlayer,
			Selected.CarrierCardId);
	if (!Result.CarrierSnapshotQueryResult.bSuccess)
	{
		Fail(
			EMatchPlayCurrentAttackReadyValidationErrorCode
				::CarrierSnapshotQueryFailed,
			Result.CarrierSnapshotQueryResult.ErrorMessage);
		return Result;
	}
	Result.MarkerSnapshotQueryResult =
		FMatchPlayCardSnapshotAuthorityQuery::FindByPlayerSideAndCardId(
			State.CardSnapshotAuthority,
			Result.CurrentDefendingPlayer,
			Selected.MarkerCardId);
	if (!Result.MarkerSnapshotQueryResult.bSuccess)
	{
		Fail(
			EMatchPlayCurrentAttackReadyValidationErrorCode
				::MarkerSnapshotQueryFailed,
			Result.MarkerSnapshotQueryResult.ErrorMessage);
		return Result;
	}
	if (Result.CarrierSnapshotQueryResult.Snapshot.bIsGoalkeeper
		|| Result.MarkerSnapshotQueryResult.Snapshot.bIsGoalkeeper)
	{
		Fail(
			EMatchPlayCurrentAttackReadyValidationErrorCode
				::RequiredParticipantIsGoalkeeper,
			TEXT("Ready Carrier and Marker must be ordinary players."));
		return Result;
	}

	Result.ParticipantRequirementResult =
		FMatchPlaySkillParticipantRequirementQuery::Query(
			Selected.ActionType);
	if (!Result.ParticipantRequirementResult.bSuccess)
	{
		Fail(
			EMatchPlayCurrentAttackReadyValidationErrorCode
				::InvalidSelectionState,
			Result.ParticipantRequirementResult.ErrorMessage);
		return Result;
	}
	if (Result.ParticipantRequirementResult.bRequiresRunner)
	{
		if (RunnerCount != 1)
		{
			Fail(
				EMatchPlayCurrentAttackReadyValidationErrorCode
					::RunnerDeploymentInvalid,
				TEXT("Ready Runner must have exactly one attacker placement."));
			return Result;
		}
		Result.RunnerSnapshotQueryResult =
			FMatchPlayCardSnapshotAuthorityQuery
				::FindByPlayerSideAndCardId(
					State.CardSnapshotAuthority,
					Result.CurrentAttackingPlayer,
					Selected.RunnerCardId);
		if (!Result.RunnerSnapshotQueryResult.bSuccess)
		{
			Fail(
				EMatchPlayCurrentAttackReadyValidationErrorCode
					::RunnerSnapshotQueryFailed,
				Result.RunnerSnapshotQueryResult.ErrorMessage);
			return Result;
		}
		const FPlayerCardRuleSnapshot& Runner =
			Result.RunnerSnapshotQueryResult.Snapshot;
		if (Runner.bIsGoalkeeper)
		{
			Fail(
				EMatchPlayCurrentAttackReadyValidationErrorCode
					::RequiredParticipantIsGoalkeeper,
				TEXT("Ready Runner must be an ordinary player."));
			return Result;
		}
		if (Selected.RunnerCardId == Selected.CarrierCardId)
		{
			Fail(
				EMatchPlayCurrentAttackReadyValidationErrorCode
					::RunnerMatchesCarrier,
				TEXT("Ready Runner must differ from Carrier."));
			return Result;
		}
		if (Selected.ActionType == ESkillRuleType::PassControl
			&& !Runner.PositionTypes.Contains(
				EPlayerPositionType::Midfield))
		{
			Fail(
				EMatchPlayCurrentAttackReadyValidationErrorCode
					::RunnerMissingRequiredPositionType,
				TEXT("PassControl Ready Runner must include Midfield."));
			return Result;
		}
		if (Selected.ActionType == ESkillRuleType::Cross
			&& !Runner.PositionTypes.Contains(
				EPlayerPositionType::Attack))
		{
			Fail(
				EMatchPlayCurrentAttackReadyValidationErrorCode
					::RunnerMissingRequiredPositionType,
				TEXT("Cross Ready Runner must include Attack."));
			return Result;
		}
		if (Selected.ActionType == ESkillRuleType::ThroughBall)
		{
			Result.RunnerRelativeZoneResolveResult =
				FMatchPlayRelativeDeploymentZoneResolver::Resolve(
					State.DeploymentSlotCatalog,
					RunnerPlacement.SlotId,
					Result.CurrentAttackingPlayer,
					Result.CurrentAttackingPlayer);
			if (!Result.RunnerRelativeZoneResolveResult.bSuccess)
			{
				Fail(
					EMatchPlayCurrentAttackReadyValidationErrorCode
						::RunnerPhysicalAreaResolutionFailed,
					Result.RunnerRelativeZoneResolveResult.ErrorMessage);
				return Result;
			}
			if (Result.RunnerRelativeZoneResolveResult.RelativeZone
				!= EMatchPlayRelativeDeploymentZone::Forward)
			{
				Fail(
					EMatchPlayCurrentAttackReadyValidationErrorCode
						::RunnerNotInAttackingForwardArea,
					TEXT("ThroughBall Ready Runner must be in Forward area."));
				return Result;
			}
		}

		if (Selected.bHasHelper)
		{
			Result.HelperAuthorityResult =
				FMatchPlayCurrentAttackHelperParticipantAuthority::Evaluate(
					State,
					Result.CurrentDefendingPlayer,
					Selected.MarkerCardId,
					Selected.HelperCardId);
			if (!Result.HelperAuthorityResult.bSuccess)
			{
				Fail(
					EMatchPlayCurrentAttackReadyValidationErrorCode
						::HelperAuthorityFailed,
					Result.HelperAuthorityResult.ErrorMessage);
				return Result;
			}
		}
	}

	Result.bSuccess = true;
	Result.ErrorCode =
		EMatchPlayCurrentAttackReadyValidationErrorCode::None;
	return Result;
}
