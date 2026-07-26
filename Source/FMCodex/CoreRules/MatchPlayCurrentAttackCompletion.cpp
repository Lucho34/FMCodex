#include "MatchPlayCurrentAttackCompletion.h"

#include "MatchEndResolver.h"

namespace MatchPlayCurrentAttackCompletionImplementation
{
	void SetError(
		FMatchPlayCurrentAttackCompletionResult& Result,
		const EMatchPlayCurrentAttackCompletionErrorCode ErrorCode,
		const FString& ErrorMessage)
	{
		Result.ErrorCode = ErrorCode;
		Result.ErrorMessage = ErrorMessage;
	}

	bool IsCompletionPlayer(const EInitialTurnOrderPlayer Player)
	{
		return Player == EInitialTurnOrderPlayer::PlayerA
			|| Player == EInitialTurnOrderPlayer::PlayerB;
	}

	EInitialTurnOrderPlayer GetCompletionDefender(
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

	bool IsPersistentGoalkeeperUsed(
		const FMatchPlayGoalkeeperUsageState& UsageState,
		const EInitialTurnOrderPlayer Defender)
	{
		return Defender == EInitialTurnOrderPlayer::PlayerA
			? UsageState.bPlayerAGoalkeeperCardUsed
			: UsageState.bPlayerBGoalkeeperCardUsed;
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

	bool ValidateAvailabilityProvenance(
		const FMatchPlayState& BeforeState,
		const FMatchPlayMarkerNoSelectionGoalCapability& Capability,
		const EInitialTurnOrderPlayer Defender,
		FString& OutErrorMessage)
	{
		const FMatchPlayCurrentAttackMarkerSelectionAvailabilityResult&
			Availability = Capability.GetAuthorityResult();
		if (!Availability.bQuerySucceeded)
		{
			OutErrorMessage =
				TEXT("Capability availability query did not succeed.");
			return false;
		}
		if (Availability.bHasGlobalBlockingLegalityResult)
		{
			OutErrorMessage =
				TEXT("Capability availability contains a global blocker.");
			return false;
		}
		if (Availability.AttackSequence != Capability.GetAttackSequence())
		{
			OutErrorMessage =
				TEXT("Capability and availability sequences do not match.");
			return false;
		}
		if (Availability.RequestingSide != Defender)
		{
			OutErrorMessage =
				TEXT("Capability availability was not queried for the current defender.");
			return false;
		}

		TArray<const FMatchPlayDeploymentPlacement*> DefenderPlacements;
		for (const FMatchPlayDeploymentPlacement& Placement :
			BeforeState.CurrentAttack.DeploymentPlacements)
		{
			if (Placement.PlayerSide == Defender)
			{
				DefenderPlacements.Add(&Placement);
			}
		}
		if (Availability.Candidates.Num() != DefenderPlacements.Num())
		{
			OutErrorMessage =
				TEXT("Capability availability candidates do not match defender placements.");
			return false;
		}

		bool bAnyLegalCandidate = false;
		for (int32 Index = 0; Index < DefenderPlacements.Num(); ++Index)
		{
			const FMatchPlayCurrentAttackMarkerSelectionCandidateAvailability&
				Candidate = Availability.Candidates[Index];
			const FMatchPlayDeploymentPlacement& Placement =
				*DefenderPlacements[Index];
			if (Candidate.MarkerCardId != Placement.CardId)
			{
				OutErrorMessage =
					TEXT("Capability availability candidate order or identity is invalid.");
				return false;
			}
			const FMatchPlayCurrentAttackMarkerSelectionRequest&
				CandidateRequest = Candidate.LegalityResult.Request;
			if (CandidateRequest.AttackSequence
					!= Capability.GetAttackSequence()
				|| CandidateRequest.RequestingSide != Defender
				|| CandidateRequest.MarkerCardId
					!= Candidate.MarkerCardId)
			{
				OutErrorMessage =
					TEXT("Capability candidate legality request provenance is invalid.");
				return false;
			}
			if (Candidate.LegalityResult.bIsLegal
				!= (Candidate.LegalityResult.ErrorCode
					== EMatchPlayCurrentAttackMarkerSelectionErrorCode
						::None))
			{
				OutErrorMessage =
					TEXT("Capability candidate legality result is internally inconsistent.");
				return false;
			}
			bAnyLegalCandidate =
				bAnyLegalCandidate || Candidate.LegalityResult.bIsLegal;
		}
		if (Availability.bCanSelectAnyMarker != bAnyLegalCandidate)
		{
			OutErrorMessage =
				TEXT("Capability availability summary is internally inconsistent.");
			return false;
		}

		switch (Capability.GetSource())
		{
		case EMatchPlayMarkerNoSelectionGoalSource::ResolveNoLegalMarker:
			if (Availability.bCanSelectAnyMarker)
			{
				OutErrorMessage =
					TEXT("No-legal-marker projection contains a legal marker.");
				return false;
			}
			if (DefenderPlacements.IsEmpty())
			{
				if (Capability.GetReason()
					!= EMatchPlayMarkerNoSelectionGoalReason
						::DefenderHasNoDeployedPlayers)
				{
					OutErrorMessage =
						TEXT("Zero defender placements require DefenderHasNoDeployedPlayers.");
					return false;
				}
			}
			else if (Capability.GetReason()
				!= EMatchPlayMarkerNoSelectionGoalReason
					::NoLegalMarker)
			{
				OutErrorMessage =
					TEXT("Defender placements without a legal marker require NoLegalMarker.");
				return false;
			}
			return true;

		case EMatchPlayMarkerNoSelectionGoalSource::DeclineMarker:
			if (Capability.GetReason()
					!= EMatchPlayMarkerNoSelectionGoalReason
						::MarkerDeclined
				|| !Availability.bCanSelectAnyMarker)
			{
				OutErrorMessage =
					TEXT("Decline projection requires MarkerDeclined and a legal marker.");
				return false;
			}
			return true;

		case EMatchPlayMarkerNoSelectionGoalSource::None:
		default:
			OutErrorMessage =
				TEXT("Capability source is not supported.");
			return false;
		}
	}
}

FMatchPlayCurrentAttackCompletionResult
FMatchPlayCurrentAttackCompletion::Complete(
	const FMatchPlayState& BeforeState,
	const FMatchPlayMarkerNoSelectionGoalCapability& Capability)
{
	using namespace MatchPlayCurrentAttackCompletionImplementation;

	FMatchPlayCurrentAttackCompletionResult Result;
	Result.BeforeState = BeforeState;
	Result.AfterState = BeforeState;
	Result.Reason = Capability.GetReason();
	Result.Source = Capability.GetSource();

	if (!BeforeState.RuntimeState.bIsInitialized)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackCompletionErrorCode
				::MatchPlayStateNotInitialized,
			TEXT("Match play state must be initialized before completing an attack."));
		return Result;
	}
	if (!BeforeState.bHasCurrentAttack)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackCompletionErrorCode::NoCurrentAttack,
			TEXT("Current attack completion requires an active current attack."));
		return Result;
	}

	const FMatchPlayCurrentAttackState& CurrentAttack =
		BeforeState.CurrentAttack;
	if (CurrentAttack.AttackSequence <= 0)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackCompletionErrorCode
				::InvalidCurrentAttackSequence,
			TEXT("Current attack sequence must be greater than zero."));
		return Result;
	}
	if (Capability.GetSource()
			!= EMatchPlayMarkerNoSelectionGoalSource
				::ResolveNoLegalMarker
		&& Capability.GetSource()
			!= EMatchPlayMarkerNoSelectionGoalSource::DeclineMarker)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackCompletionErrorCode
				::UnsupportedCapabilitySource,
			TEXT("Capability source is not supported by current attack completion."));
		return Result;
	}
	if (Capability.GetReason()
			!= EMatchPlayMarkerNoSelectionGoalReason
				::DefenderHasNoDeployedPlayers
		&& Capability.GetReason()
			!= EMatchPlayMarkerNoSelectionGoalReason::NoLegalMarker
		&& Capability.GetReason()
			!= EMatchPlayMarkerNoSelectionGoalReason::MarkerDeclined)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackCompletionErrorCode
				::InvalidCapabilityReason,
			TEXT("Capability reason is not supported by current attack completion."));
		return Result;
	}
	if (Capability.GetAttackSequence() != CurrentAttack.AttackSequence)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackCompletionErrorCode
				::CapabilitySequenceMismatch,
			TEXT("Capability sequence does not match the current attack."));
		return Result;
	}
	if (CurrentAttack.Phase
		!= EMatchPlayCurrentAttackPhase::Resolution)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackCompletionErrorCode
				::CurrentAttackNotInResolution,
			TEXT("Current attack must be in Resolution phase for completion."));
		return Result;
	}

	Result.SelectionStateValidationResult =
		FMatchPlayCurrentAttackSelectionStateValidator::Validate(
			CurrentAttack);
	if (!Result.SelectionStateValidationResult.bIsCanonical
		|| !CurrentAttack.bAttackerDeploymentFinished
		|| !CurrentAttack.bDefenderDeploymentFinished
		|| CurrentAttack.CurrentLegalDeploymentSide
			!= EInitialTurnOrderPlayer::None)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackCompletionErrorCode
				::InvalidSelectionState,
			Result.SelectionStateValidationResult.bIsCanonical
				? TEXT("Resolution requires both deployment-finished flags and no legal deployment side.")
				: Result.SelectionStateValidationResult.ErrorMessage);
		return Result;
	}
	if (CurrentAttack.SelectionStage
		!= EMatchPlayCurrentAttackSelectionStage::AwaitingMarker)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackCompletionErrorCode
				::WrongSelectionStage,
			TEXT("Marker no-selection completion requires AwaitingMarker stage."));
		return Result;
	}

	const EInitialTurnOrderPlayer Attacker =
		BeforeState.RuntimeState.CurrentAttackingPlayer;
	if (!IsCompletionPlayer(Attacker))
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackCompletionErrorCode
				::InvalidCurrentAttackingPlayer,
			TEXT("CurrentAttackingPlayer must be PlayerA or PlayerB."));
		return Result;
	}
	const EInitialTurnOrderPlayer Defender =
		GetCompletionDefender(Attacker);
	if (!IsCompletionPlayer(Defender))
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackCompletionErrorCode
				::InvalidCurrentDefendingPlayer,
			TEXT("Current defending player could not be derived."));
		return Result;
	}

	FString CapabilityProvenanceError;
	if (!ValidateAvailabilityProvenance(
		BeforeState,
		Capability,
		Defender,
		CapabilityProvenanceError))
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackCompletionErrorCode
				::InvalidCapabilityProvenance,
			CapabilityProvenanceError);
		return Result;
	}

	if (BeforeState.RuntimeState.PlayerAState.Score < 0
		|| BeforeState.RuntimeState.PlayerBState.Score < 0)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackCompletionErrorCode
				::InvalidScoreState,
			TEXT("Match scores cannot be negative."));
		return Result;
	}

	const FMatchEndResolveResult PreCompletionMatchEndResult =
		FMatchEndResolver::ResolveMatchEnd(BeforeState.RuntimeState);
	if (!PreCompletionMatchEndResult.bSuccess
		|| PreCompletionMatchEndResult.bIsMatchEnded)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackCompletionErrorCode
				::InvalidOpportunityState,
			PreCompletionMatchEndResult.bSuccess
				? TEXT("An active current attack cannot be completed after the match has ended.")
				: PreCompletionMatchEndResult.ErrorMessage);
		return Result;
	}
	const FPlayerRuntimeState& AttackerRuntimeState =
		Attacker == EInitialTurnOrderPlayer::PlayerA
			? BeforeState.RuntimeState.PlayerAState
			: BeforeState.RuntimeState.PlayerBState;
	if (AttackerRuntimeState.UsedAttackCount
		>= AttackerRuntimeState.TotalAttackCount)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackCompletionErrorCode
				::InvalidOpportunityState,
			TEXT("Current attacker must have a remaining attack opportunity."));
		return Result;
	}

	TSet<FString> SeenDeploymentCards;
	int32 GoalkeeperPlacementCount = 0;
	FName GoalkeeperCardId = NAME_None;
	for (const FMatchPlayDeploymentPlacement& Placement :
		CurrentAttack.DeploymentPlacements)
	{
		if (!IsCompletionPlayer(Placement.PlayerSide)
			|| Placement.CardId.IsNone()
			|| Placement.SlotId.IsNone())
		{
			SetError(
				Result,
				EMatchPlayCurrentAttackCompletionErrorCode
					::InvalidDeploymentPlacement,
				TEXT("Every deployment placement requires a valid side, CardId, and SlotId."));
			return Result;
		}

		const FString CardKey =
			MakeDeploymentCardKey(Placement.PlayerSide, Placement.CardId);
		if (SeenDeploymentCards.Contains(CardKey))
		{
			SetError(
				Result,
				EMatchPlayCurrentAttackCompletionErrorCode
					::DuplicateDeploymentCard,
				FString::Printf(
					TEXT("Deployment card '%s' is placed more than once for one side."),
					*Placement.CardId.ToString()));
			return Result;
		}
		SeenDeploymentCards.Add(CardKey);

		const FMatchPlayCardSnapshotAuthorityQueryResult SnapshotResult =
			FMatchPlayCardSnapshotAuthorityQuery
				::FindByPlayerSideAndCardId(
					BeforeState.CardSnapshotAuthority,
					Placement.PlayerSide,
					Placement.CardId);
		Result.DeploymentSnapshotQueryResults.Add(SnapshotResult);
		if (!SnapshotResult.bSuccess)
		{
			SetError(
				Result,
				EMatchPlayCurrentAttackCompletionErrorCode
					::DeploymentSnapshotQueryFailed,
				SnapshotResult.ErrorMessage);
			return Result;
		}

		if (SnapshotResult.Snapshot.bIsGoalkeeper)
		{
			if (Placement.PlayerSide != Defender)
			{
				SetError(
					Result,
					EMatchPlayCurrentAttackCompletionErrorCode
						::InvalidGoalkeeperCompletionState,
					TEXT("A current-attack goalkeeper placement must belong to the defender."));
				return Result;
			}
			++GoalkeeperPlacementCount;
			GoalkeeperCardId = Placement.CardId;
			if (GoalkeeperPlacementCount > 1)
			{
				SetError(
					Result,
					EMatchPlayCurrentAttackCompletionErrorCode
						::InvalidGoalkeeperCompletionState,
					TEXT("A current defense cannot contain multiple goalkeeper placements."));
				return Result;
			}
		}
	}

	const bool bHasGoalkeeperPlacement =
		GoalkeeperPlacementCount == 1;
	if (CurrentAttack.bCurrentDefenseGoalkeeperActivated
		!= bHasGoalkeeperPlacement)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackCompletionErrorCode
				::InvalidGoalkeeperCompletionState,
			TEXT("Current goalkeeper activation and deployment placement are inconsistent."));
		return Result;
	}
	if (bHasGoalkeeperPlacement)
	{
		if (!IsPersistentGoalkeeperUsed(
			BeforeState.GoalkeeperUsageState,
			Defender))
		{
			SetError(
				Result,
				EMatchPlayCurrentAttackCompletionErrorCode
					::InvalidGoalkeeperCompletionState,
				TEXT("An active current goalkeeper requires persistent used-this-match state."));
			return Result;
		}
		const FPlayCardValidationResult GoalkeeperCardUsageValidation =
			FPlayCardResolver::ValidateCanPlayCard(
				BeforeState.CardUsageState,
				Defender,
				GoalkeeperCardId);
		if (!GoalkeeperCardUsageValidation.bSuccess)
		{
			SetError(
				Result,
				EMatchPlayCurrentAttackCompletionErrorCode
					::InvalidGoalkeeperCompletionState,
				TEXT("Current goalkeeper card must remain available in ordinary CardUsage."));
			return Result;
		}
	}

	FMatchPlayState WorkingState = BeforeState;
	Result.GoalResolveResult = FGoalResolver::RecordGoal(
		WorkingState.RuntimeState,
		Attacker);
	if (!Result.GoalResolveResult.bSuccess)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackCompletionErrorCode
				::GoalResolutionFailed,
			Result.GoalResolveResult.ErrorMessage);
		return Result;
	}
	WorkingState.RuntimeState =
		Result.GoalResolveResult.UpdatedRuntimeState;

	for (int32 Index = 0;
		Index < CurrentAttack.DeploymentPlacements.Num();
		++Index)
	{
		const FMatchPlayDeploymentPlacement& Placement =
			CurrentAttack.DeploymentPlacements[Index];
		if (Result.DeploymentSnapshotQueryResults[Index]
			.Snapshot.bIsGoalkeeper)
		{
			continue;
		}

		const FPlayCardResolveResult CardUsageResult =
			FPlayCardResolver::PlayCard(
				WorkingState.CardUsageState,
				Placement.PlayerSide,
				Placement.CardId);
		Result.OrdinaryCardUsageResults.Add(CardUsageResult);
		if (!CardUsageResult.bSuccess)
		{
			SetError(
				Result,
				EMatchPlayCurrentAttackCompletionErrorCode
					::OrdinaryCardUsageConsumptionFailed,
				CardUsageResult.ErrorMessage);
			return Result;
		}
		WorkingState.CardUsageState =
			CardUsageResult.UpdatedMatchCardUsageState;
	}

	WorkingState.bHasCurrentAttack = false;
	WorkingState.CurrentAttack = FMatchPlayCurrentAttackState();

	Result.OpportunityResolveResult =
		FAttackOpportunityResolver::ConsumeCurrentAttackOpportunity(
			WorkingState.RuntimeState);
	if (!Result.OpportunityResolveResult.bSuccess)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackCompletionErrorCode
				::OpportunityConsumptionFailed,
			Result.OpportunityResolveResult.ErrorMessages.IsEmpty()
				? TEXT("Attack opportunity consumption failed.")
				: FString::Join(
					Result.OpportunityResolveResult.ErrorMessages,
					TEXT(" | ")));
		return Result;
	}
	WorkingState.RuntimeState =
		Result.OpportunityResolveResult.UpdatedRuntimeState;
	Result.NextAttackingPlayer =
		Result.OpportunityResolveResult.NextAttackingPlayer;

	const bool bHasRemainingAttack =
		Result.OpportunityResolveResult.bMatchHasRemainingAttack;
	if ((bHasRemainingAttack
			&& !IsCompletionPlayer(Result.NextAttackingPlayer))
		|| (!bHasRemainingAttack
			&& Result.NextAttackingPlayer
				!= EInitialTurnOrderPlayer::None))
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackCompletionErrorCode
				::NextAttackerResolutionFailed,
			TEXT("Opportunity result contains an invalid next attacking player."));
		return Result;
	}

	Result.MatchEndResolveResult =
		FMatchEndResolver::ResolveMatchEnd(
			WorkingState.RuntimeState);
	if (!Result.MatchEndResolveResult.bSuccess)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackCompletionErrorCode
				::MatchEndResolutionFailed,
			Result.MatchEndResolveResult.ErrorMessage);
		return Result;
	}
	Result.bMatchEnded =
		Result.MatchEndResolveResult.bIsMatchEnded;
	if (Result.bMatchEnded)
	{
		Result.MatchResultResolveResult =
			FMatchResultResolver::ResolveMatchResult(
				WorkingState.RuntimeState);
		if (!Result.MatchResultResolveResult.bSuccess)
		{
			SetError(
				Result,
				EMatchPlayCurrentAttackCompletionErrorCode
					::MatchResultResolutionFailed,
				Result.MatchResultResolveResult.ErrorMessage);
			return Result;
		}
	}

	Result.ScoringSide = Attacker;
	Result.AfterState = MoveTemp(WorkingState);
	Result.bSuccess = true;
	Result.ErrorCode =
		EMatchPlayCurrentAttackCompletionErrorCode::None;
	Result.ErrorMessage.Empty();
	return Result;
}
