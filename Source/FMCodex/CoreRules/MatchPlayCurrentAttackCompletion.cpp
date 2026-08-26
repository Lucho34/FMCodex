#include "MatchPlayCurrentAttackCompletion.h"

#include "MatchEndResolver.h"
#include "MatchPlayCurrentAttackResolutionSessionStateValidator.h"

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

	bool ValidateCommonOuter(
		const FMatchPlayState& BeforeState,
		const int64 CapabilityAttackSequence,
		const EMatchPlayCurrentAttackSelectionStage RequiredStage,
		FMatchPlayCurrentAttackCompletionResult& Result,
		EInitialTurnOrderPlayer& OutAttacker,
		EInitialTurnOrderPlayer& OutDefender)
	{
		if (!BeforeState.RuntimeState.bIsInitialized)
		{
			SetError(
				Result,
				EMatchPlayCurrentAttackCompletionErrorCode
					::MatchPlayStateNotInitialized,
				TEXT("Match play state must be initialized before completing an attack."));
			return false;
		}
		if (!BeforeState.bHasCurrentAttack)
		{
			SetError(
				Result,
				EMatchPlayCurrentAttackCompletionErrorCode
					::NoCurrentAttack,
				TEXT("Current attack completion requires an active current attack."));
			return false;
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
			return false;
		}
		if (CapabilityAttackSequence != CurrentAttack.AttackSequence)
		{
			SetError(
				Result,
				EMatchPlayCurrentAttackCompletionErrorCode
					::CapabilitySequenceMismatch,
				TEXT("Capability sequence does not match the current attack."));
			return false;
		}
		if (CurrentAttack.Phase
			!= EMatchPlayCurrentAttackPhase::Resolution)
		{
			SetError(
				Result,
				EMatchPlayCurrentAttackCompletionErrorCode
					::CurrentAttackNotInResolution,
				TEXT("Current attack must be in Resolution phase for completion."));
			return false;
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
			return false;
		}
		if (CurrentAttack.SelectionStage != RequiredStage)
		{
			FString StageErrorMessage;
			switch (RequiredStage)
			{
			case EMatchPlayCurrentAttackSelectionStage::AwaitingCarrier:
				StageErrorMessage =
					TEXT("Carrier no-selection completion requires AwaitingCarrier stage.");
				break;
			case EMatchPlayCurrentAttackSelectionStage::AwaitingMarker:
				StageErrorMessage =
					TEXT("Marker no-selection completion requires AwaitingMarker stage.");
				break;
			case EMatchPlayCurrentAttackSelectionStage::AwaitingSkill:
				StageErrorMessage =
					TEXT("Skill no-selection completion requires AwaitingSkill stage.");
				break;
			case EMatchPlayCurrentAttackSelectionStage::AwaitingRunner:
				StageErrorMessage =
					TEXT("Runner no-selection completion requires AwaitingRunner stage.");
				break;
			default:
				StageErrorMessage =
					TEXT("Current attack is not in the required completion selection stage.");
				break;
			}
			SetError(
				Result,
				EMatchPlayCurrentAttackCompletionErrorCode
					::WrongSelectionStage,
				StageErrorMessage);
			return false;
		}

		OutAttacker =
			BeforeState.RuntimeState.CurrentAttackingPlayer;
		if (!IsCompletionPlayer(OutAttacker))
		{
			SetError(
				Result,
				EMatchPlayCurrentAttackCompletionErrorCode
					::InvalidCurrentAttackingPlayer,
				TEXT("CurrentAttackingPlayer must be PlayerA or PlayerB."));
			return false;
		}
		OutDefender = GetCompletionDefender(OutAttacker);
		if (!IsCompletionPlayer(OutDefender))
		{
			SetError(
				Result,
				EMatchPlayCurrentAttackCompletionErrorCode
					::InvalidCurrentDefendingPlayer,
				TEXT("Current defending player could not be derived."));
			return false;
		}

		const FMatchEndResolveResult PreCompletionMatchEndResult =
			FMatchEndResolver::ResolveMatchEnd(
				BeforeState.RuntimeState);
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
			return false;
		}
		const FPlayerRuntimeState& AttackerRuntimeState =
			OutAttacker == EInitialTurnOrderPlayer::PlayerA
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
			return false;
		}
		return true;
	}

	bool ValidateScoreState(
		const FMatchPlayState& BeforeState,
		FMatchPlayCurrentAttackCompletionResult& Result)
	{
		if (BeforeState.RuntimeState.PlayerAState.Score < 0
			|| BeforeState.RuntimeState.PlayerBState.Score < 0)
		{
			SetError(
				Result,
				EMatchPlayCurrentAttackCompletionErrorCode
					::InvalidScoreState,
				TEXT("Match scores cannot be negative."));
			return false;
		}
		return true;
	}

	bool ValidateCarrierAvailabilityProvenance(
		const FMatchPlayState& BeforeState,
		const FMatchPlayNoLegalCarrierCompletionCapability& Capability,
		const EInitialTurnOrderPlayer Attacker,
		FString& OutErrorMessage)
	{
		const FMatchPlayCurrentAttackCarrierSelectionAvailabilityResult&
			Availability = Capability.GetAuthorityResult();
		if (!Availability.bQuerySucceeded)
		{
			OutErrorMessage =
				TEXT("Carrier capability availability query did not succeed.");
			return false;
		}
		if (Availability.bHasGlobalBlockingLegalityResult)
		{
			OutErrorMessage =
				TEXT("Carrier capability availability contains a global blocker.");
			return false;
		}
		if (Availability.AttackSequence != Capability.GetAttackSequence())
		{
			OutErrorMessage =
				TEXT("Carrier capability and availability sequences do not match.");
			return false;
		}
		if (Availability.RequestingSide != Attacker)
		{
			OutErrorMessage =
				TEXT("Carrier capability availability was not queried for the current attacker.");
			return false;
		}

		TArray<const FMatchPlayDeploymentPlacement*> AttackerPlacements;
		for (const FMatchPlayDeploymentPlacement& Placement :
			BeforeState.CurrentAttack.DeploymentPlacements)
		{
			if (Placement.PlayerSide == Attacker)
			{
				AttackerPlacements.Add(&Placement);
			}
		}
		if (Availability.Candidates.Num() != AttackerPlacements.Num())
		{
			OutErrorMessage =
				TEXT("Carrier capability candidates do not match attacker placements.");
			return false;
		}

		bool bAnyLegalCandidate = false;
		for (int32 Index = 0; Index < AttackerPlacements.Num(); ++Index)
		{
			const FMatchPlayCurrentAttackCarrierSelectionCandidateAvailability&
				Candidate = Availability.Candidates[Index];
			const FMatchPlayDeploymentPlacement& Placement =
				*AttackerPlacements[Index];
			if (Candidate.CarrierCardId != Placement.CardId)
			{
				OutErrorMessage =
					TEXT("Carrier capability candidate order or identity is invalid.");
				return false;
			}
			const FMatchPlayCurrentAttackCarrierSelectionRequest& Request =
				Candidate.LegalityResult.Request;
			if (Request.AttackSequence != Capability.GetAttackSequence()
				|| Request.RequestingSide != Attacker
				|| Request.CarrierCardId != Candidate.CarrierCardId)
			{
				OutErrorMessage =
					TEXT("Carrier capability candidate request provenance is invalid.");
				return false;
			}
			if (Candidate.LegalityResult.bIsLegal
				!= (Candidate.LegalityResult.ErrorCode
					== EMatchPlayCurrentAttackCarrierSelectionErrorCode::None))
			{
				OutErrorMessage =
					TEXT("Carrier capability candidate legality is inconsistent.");
				return false;
			}
			bAnyLegalCandidate =
				bAnyLegalCandidate || Candidate.LegalityResult.bIsLegal;
		}
		if (Availability.bCanSelectAnyCarrier != bAnyLegalCandidate)
		{
			OutErrorMessage =
				TEXT("Carrier capability availability summary is inconsistent.");
			return false;
		}
		if (Availability.bCanSelectAnyCarrier)
		{
			OutErrorMessage =
				TEXT("No-legal Carrier capability contains a legal Carrier.");
			return false;
		}
		return true;
	}

	bool ValidateMarkerAvailabilityProvenance(
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

	bool ValidateSkillAvailabilityProvenance(
		const FMatchPlayState& BeforeState,
		const FMatchPlaySkillNoSelectionNoGoalCapability& Capability,
		const EInitialTurnOrderPlayer Attacker,
		const EInitialTurnOrderPlayer Defender,
		FString& OutErrorMessage)
	{
		const FMatchPlayCurrentAttackSkillSelectionAvailabilityResult&
			Availability = Capability.GetAuthorityResult();
		if (!Availability.bQuerySucceeded
			|| !Availability.GlobalContextResult.bSuccess)
		{
			OutErrorMessage =
				TEXT("Skill capability availability did not formally succeed.");
			return false;
		}
		if (Availability.AttackSequence != Capability.GetAttackSequence()
			|| Availability.GlobalContextResult.RequestedAttackSequence
				!= Capability.GetAttackSequence()
			|| Availability.GlobalContextResult
					.AuthoritativeAttackSequence
				!= Capability.GetAttackSequence())
		{
			OutErrorMessage =
				TEXT("Skill capability sequence provenance is invalid.");
			return false;
		}
		if (Availability.RequestingSide != Attacker
			|| Availability.GlobalContextResult.RequestingSide
				!= Attacker
			|| Availability.GlobalContextResult.CurrentAttackingPlayer
				!= Attacker
			|| Availability.GlobalContextResult.CurrentDefendingPlayer
				!= Defender)
		{
			OutErrorMessage =
				TEXT("Skill capability actor provenance is invalid.");
			return false;
		}
		if (Availability.GlobalContextResult.FrozenCarrierCardId
				!= BeforeState.CurrentAttack.ActionPreparation
					.CarrierCardId
			|| Availability.GlobalContextResult.FrozenMarkerCardId
				!= BeforeState.CurrentAttack.ActionPreparation
					.MarkerCardId)
		{
			OutErrorMessage =
				TEXT("Skill capability frozen participant provenance is invalid.");
			return false;
		}

		switch (Capability.GetSource())
		{
		case EMatchPlaySkillNoSelectionNoGoalSource
			::ResolveNoLegalSkill:
			if (Capability.GetReason()
					!= EMatchPlaySkillNoSelectionNoGoalReason
						::NoLegalSkill
				|| Availability.bCanSelectAnySkill)
			{
				OutErrorMessage =
					TEXT("ResolveNoLegalSkill capability requires NoLegalSkill and zero legal skills.");
				return false;
			}
			return true;

		case EMatchPlaySkillNoSelectionNoGoalSource::DeclineSkill:
			if (Capability.GetReason()
					!= EMatchPlaySkillNoSelectionNoGoalReason
						::SkillDeclined
				|| !Availability.bCanSelectAnySkill)
			{
				OutErrorMessage =
					TEXT("DeclineSkill capability requires SkillDeclined and at least one legal skill.");
				return false;
			}
			return true;

		case EMatchPlaySkillNoSelectionNoGoalSource::None:
		default:
			OutErrorMessage =
				TEXT("Skill capability source is not supported.");
			return false;
		}
	}

	bool ValidateRunnerAvailabilityProvenance(
		const FMatchPlayState& BeforeState,
		const FMatchPlayRunnerNoSelectionNoGoalCapability& Capability,
		const EInitialTurnOrderPlayer Attacker,
		const EInitialTurnOrderPlayer Defender,
		FString& OutErrorMessage)
	{
		const FMatchPlayCurrentAttackRunnerSelectionAvailabilityResult&
			Availability = Capability.GetAuthorityResult();
		if (!Availability.bQuerySucceeded
			|| !Availability.GlobalContextResult.bSuccess)
		{
			OutErrorMessage =
				TEXT("Runner capability availability did not formally succeed.");
			return false;
		}
		if (Availability.AttackSequence != Capability.GetAttackSequence()
			|| Availability.GlobalContextResult.RequestedAttackSequence
				!= Capability.GetAttackSequence()
			|| Availability.GlobalContextResult
					.AuthoritativeAttackSequence
				!= Capability.GetAttackSequence())
		{
			OutErrorMessage =
				TEXT("Runner capability sequence provenance is invalid.");
			return false;
		}
		if (Availability.RequestingSide != Attacker
			|| Availability.GlobalContextResult.RequestingSide
				!= Attacker
			|| Availability.GlobalContextResult.CurrentAttackingPlayer
				!= Attacker
			|| Availability.GlobalContextResult.CurrentDefendingPlayer
				!= Defender)
		{
			OutErrorMessage =
				TEXT("Runner capability actor provenance is invalid.");
			return false;
		}

		const FMatchPlayCurrentAttackActionPreparationState& Preparation =
			BeforeState.CurrentAttack.ActionPreparation;
		if (Availability.GlobalContextResult.FrozenCarrierCardId
				!= Preparation.CarrierCardId
			|| Availability.GlobalContextResult.FrozenMarkerCardId
				!= Preparation.MarkerCardId
			|| Availability.GlobalContextResult.FrozenSkillId
				!= Preparation.SkillId
			|| Availability.GlobalContextResult.FrozenActionType
				!= Preparation.ActionType)
		{
			OutErrorMessage =
				TEXT("Runner capability frozen preparation provenance is invalid.");
			return false;
		}

		switch (Capability.GetSource())
		{
		case EMatchPlayRunnerNoSelectionNoGoalSource
			::ResolveNoLegalRunner:
			if (Capability.GetReason()
					!= EMatchPlayRunnerNoSelectionNoGoalReason
						::NoLegalRunner
				|| Availability.bCanSelectAnyRunner)
			{
				OutErrorMessage =
					TEXT("ResolveNoLegalRunner capability requires NoLegalRunner and zero legal runners.");
				return false;
			}
			return true;

		case EMatchPlayRunnerNoSelectionNoGoalSource::RunnerDecline:
			if (Capability.GetReason()
					!= EMatchPlayRunnerNoSelectionNoGoalReason
						::RunnerDeclined
				|| !Availability.bCanSelectAnyRunner)
			{
				OutErrorMessage =
					TEXT("RunnerDecline capability requires RunnerDeclined and at least one legal runner.");
				return false;
			}
			return true;

		case EMatchPlayRunnerNoSelectionNoGoalSource::None:
		default:
			OutErrorMessage =
				TEXT("Runner capability source is not supported.");
			return false;
		}
	}
}

FMatchPlayCurrentAttackCompletionResult
FMatchPlayCurrentAttackCompletion::CompleteCrossResolution(
	const FMatchPlayState& BeforeState,
	const FMatchPlayCrossResolutionTerminalCapability& Capability)
{
	using namespace MatchPlayCurrentAttackCompletionImplementation;

	FMatchPlayCurrentAttackCompletionResult Result;
	Result.BeforeState = BeforeState;
	Result.AfterState = BeforeState;

	const EMatchPlayCrossTerminalSource Source = Capability.GetSource();
	const bool bGoalSource =
		Source == EMatchPlayCrossTerminalSource::HighFormulaGoal
		|| Source == EMatchPlayCrossTerminalSource::LowFormulaGoal;
	if (Source == EMatchPlayCrossTerminalSource::None)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackCompletionErrorCode
				::UnsupportedCapabilitySource,
			TEXT("Cross completion requires a terminal source."));
		return Result;
	}
	if (Capability.IsGoal() != bGoalSource)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackCompletionErrorCode::InvalidCapabilityReason,
			TEXT("Cross terminal source and Goal effect are inconsistent."));
		return Result;
	}

	EInitialTurnOrderPlayer Attacker = EInitialTurnOrderPlayer::None;
	EInitialTurnOrderPlayer Defender = EInitialTurnOrderPlayer::None;
	if (!ValidateCommonOuter(
		BeforeState,
		Capability.GetAttackSequence(),
		EMatchPlayCurrentAttackSelectionStage::ReadyForResolution,
		Result,
		Attacker,
		Defender))
	{
		return Result;
	}
	if (!BeforeState.CurrentAttack.bHasResolutionSession)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackCompletionErrorCode::InvalidCapabilityProvenance,
			TEXT("Cross completion requires a Resolution Session."));
		return Result;
	}
	const auto SessionValidation =
		FMatchPlayCurrentAttackResolutionSessionStateValidator::Validate(
			BeforeState);
	if (!SessionValidation.bIsCanonical
		|| BeforeState.CurrentAttack.ResolutionSession.Stage
			!= EMatchPlayCurrentAttackResolutionStage::RouteResolved
		|| !BeforeState.CurrentAttack.ResolutionSession.bHasActualBranch
		|| BeforeState.CurrentAttack.ResolutionSession.ActualBranch.ActionType
			!= ESkillRuleType::Cross)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackCompletionErrorCode::InvalidCapabilityProvenance,
			SessionValidation.bIsCanonical
				? TEXT("Cross capability requires a resolved Cross branch.")
				: SessionValidation.ErrorMessage);
		return Result;
	}
	const EMatchPlayCrossActualBranch ActualBranch =
		BeforeState.CurrentAttack.ResolutionSession.ActualBranch.Cross;
	const bool bHighSource =
		Source == EMatchPlayCrossTerminalSource::HighFormulaGoal
		|| Source == EMatchPlayCrossTerminalSource::HighFormulaMiss;
	const bool bLowSource =
		Source == EMatchPlayCrossTerminalSource::LowFormulaGoal
		|| Source == EMatchPlayCrossTerminalSource::LowFormulaMiss;
	if ((!bHighSource && !bLowSource)
		|| (bHighSource && ActualBranch != EMatchPlayCrossActualBranch::High)
		|| (bLowSource && ActualBranch != EMatchPlayCrossActualBranch::Low))
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackCompletionErrorCode::InvalidCapabilityProvenance,
			TEXT("Cross terminal source does not match ActualBranch."));
		return Result;
	}
	if (!ValidateScoreState(BeforeState, Result))
	{
		return Result;
	}

	FMatchPlayState WorkingState = BeforeState;
	if (Capability.IsGoal())
	{
		Result.GoalResolveResult = FGoalResolver::RecordGoal(
			WorkingState.RuntimeState,
			Attacker);
		if (!Result.GoalResolveResult.bSuccess)
		{
			SetError(
				Result,
				EMatchPlayCurrentAttackCompletionErrorCode::GoalResolutionFailed,
				Result.GoalResolveResult.ErrorMessage);
			return Result;
		}
		WorkingState.RuntimeState =
			Result.GoalResolveResult.UpdatedRuntimeState;
		Result.ScoringSide = Attacker;
	}

	return PersistCurrentAttackTerminal(
		BeforeState,
		MoveTemp(WorkingState),
		Attacker,
		Defender,
		MoveTemp(Result));
}

FMatchPlayCurrentAttackCompletionResult
FMatchPlayCurrentAttackCompletion::CompletePassControlResolution(
	const FMatchPlayState& BeforeState,
	const FMatchPlayPassControlResolutionTerminalCapability& Capability)
{
	// The Goal effect and the common terminal mutation remain on one local
	// working State so a later Completion failure cannot expose a partial Goal.
	using namespace MatchPlayCurrentAttackCompletionImplementation;

	FMatchPlayCurrentAttackCompletionResult Result;
	Result.BeforeState = BeforeState;
	Result.AfterState = BeforeState;

	using ESource = EMatchPlayPassControlTerminalSource;
	const ESource Source = Capability.GetSource();
	const bool bGoalSource =
		Source == ESource::PassAdvanceFormulaGoal
		|| Source == ESource::DribbleAdvanceFormulaGoal
		|| Source == ESource::RunAdvanceFormulaGoal;
	if (Source == ESource::None)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackCompletionErrorCode
				::UnsupportedCapabilitySource,
			TEXT("PassControl completion requires a terminal source."));
		return Result;
	}
	if (Capability.IsGoal() != bGoalSource)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackCompletionErrorCode::InvalidCapabilityReason,
			TEXT("PassControl terminal source and Goal effect are inconsistent."));
		return Result;
	}

	EInitialTurnOrderPlayer Attacker = EInitialTurnOrderPlayer::None;
	EInitialTurnOrderPlayer Defender = EInitialTurnOrderPlayer::None;
	if (!ValidateCommonOuter(
		BeforeState,
		Capability.GetAttackSequence(),
		EMatchPlayCurrentAttackSelectionStage::ReadyForResolution,
		Result,
		Attacker,
		Defender))
	{
		return Result;
	}
	if (!BeforeState.CurrentAttack.bHasResolutionSession)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackCompletionErrorCode::InvalidCapabilityProvenance,
			TEXT("PassControl completion requires a Resolution Session."));
		return Result;
	}
	const auto SessionValidation =
		FMatchPlayCurrentAttackResolutionSessionStateValidator::Validate(
			BeforeState);
	if (!SessionValidation.bIsCanonical
		|| BeforeState.CurrentAttack.ResolutionSession.Stage
			!= EMatchPlayCurrentAttackResolutionStage::RouteResolved
		|| !BeforeState.CurrentAttack.ResolutionSession.bHasActualBranch
		|| BeforeState.CurrentAttack.ResolutionSession.ActualBranch.ActionType
			!= ESkillRuleType::PassControl)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackCompletionErrorCode::InvalidCapabilityProvenance,
			SessionValidation.bIsCanonical
				? TEXT("PassControl capability requires a resolved PassControl branch.")
				: SessionValidation.ErrorMessage);
		return Result;
	}
	const EMatchPlayPassControlActualBranch ActualBranch =
		BeforeState.CurrentAttack.ResolutionSession.ActualBranch.PassControl;
	const bool bPassAdvanceSource =
		Source == ESource::PassAdvanceFormulaGoal
		|| Source == ESource::PassAdvanceFormulaMiss;
	const bool bDribbleAdvanceSource =
		Source == ESource::DribbleAdvanceFormulaGoal
		|| Source == ESource::DribbleAdvanceFormulaMiss;
	const bool bRunAdvanceSource =
		Source == ESource::RunAdvanceFormulaGoal
		|| Source == ESource::RunAdvanceFormulaMiss;
	const bool bSourceMatchesBranch =
		(bPassAdvanceSource
			&& ActualBranch == EMatchPlayPassControlActualBranch::PassAdvance)
		|| (bDribbleAdvanceSource
			&& ActualBranch == EMatchPlayPassControlActualBranch::DribbleAdvance)
		|| (bRunAdvanceSource
			&& ActualBranch == EMatchPlayPassControlActualBranch::RunAdvance);
	if (!bSourceMatchesBranch)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackCompletionErrorCode::InvalidCapabilityProvenance,
			TEXT("PassControl terminal source does not match ActualBranch."));
		return Result;
	}
	if (!ValidateScoreState(BeforeState, Result))
	{
		return Result;
	}

	FMatchPlayState WorkingState = BeforeState;
	if (Capability.IsGoal())
	{
		Result.GoalResolveResult = FGoalResolver::RecordGoal(
			WorkingState.RuntimeState,
			Attacker);
		if (!Result.GoalResolveResult.bSuccess)
		{
			SetError(
				Result,
				EMatchPlayCurrentAttackCompletionErrorCode::GoalResolutionFailed,
				Result.GoalResolveResult.ErrorMessage);
			return Result;
		}
		WorkingState.RuntimeState =
			Result.GoalResolveResult.UpdatedRuntimeState;
		Result.ScoringSide = Attacker;
	}

	return PersistCurrentAttackTerminal(
		BeforeState,
		MoveTemp(WorkingState),
		Attacker,
		Defender,
		MoveTemp(Result));
}

FMatchPlayCurrentAttackCompletionResult
FMatchPlayCurrentAttackCompletion::CompleteShotResolution(
	const FMatchPlayState& BeforeState,
	const FMatchPlayShotResolutionTerminalCapability& Capability)
{
	using namespace MatchPlayCurrentAttackCompletionImplementation;
	using ESource = EMatchPlayShotTerminalSource;

	FMatchPlayCurrentAttackCompletionResult Result;
	Result.BeforeState = BeforeState;
	Result.AfterState = BeforeState;

	const ESource Source = Capability.GetSource();
	const bool bGoalSource =
		Source == ESource::LongShotDirectShotFormulaGoal
		|| Source == ESource::CutInsideShotDirectShotFormulaGoal
		|| Source == ESource::LongShotDeadCornerGoal
		|| Source == ESource::CutInsideShotDeadCornerGoal;
	if (Source == ESource::None)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackCompletionErrorCode
				::UnsupportedCapabilitySource,
			TEXT("Shot completion requires a terminal source."));
		return Result;
	}
	if (Capability.IsGoal() != bGoalSource)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackCompletionErrorCode::InvalidCapabilityReason,
			TEXT("Shot terminal source and Goal effect are inconsistent."));
		return Result;
	}

	EInitialTurnOrderPlayer Attacker = EInitialTurnOrderPlayer::None;
	EInitialTurnOrderPlayer Defender = EInitialTurnOrderPlayer::None;
	if (!ValidateCommonOuter(
		BeforeState,
		Capability.GetAttackSequence(),
		EMatchPlayCurrentAttackSelectionStage::ReadyForResolution,
		Result,
		Attacker,
		Defender))
	{
		return Result;
	}
	if (!BeforeState.CurrentAttack.bHasResolutionSession)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackCompletionErrorCode::InvalidCapabilityProvenance,
			TEXT("Shot completion requires a Resolution Session."));
		return Result;
	}
	const auto SessionValidation =
		FMatchPlayCurrentAttackResolutionSessionStateValidator::Validate(
			BeforeState);
	const FMatchPlayCurrentAttackResolutionSession& Session =
		BeforeState.CurrentAttack.ResolutionSession;
	if (!SessionValidation.bIsCanonical
		|| Session.Stage
			!= EMatchPlayCurrentAttackResolutionStage::RouteResolved
		|| !Session.bHasActualBranch
		|| (Session.ActualBranch.ActionType != ESkillRuleType::LongShot
			&& Session.ActualBranch.ActionType
				!= ESkillRuleType::CutInsideShot))
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackCompletionErrorCode::InvalidCapabilityProvenance,
			SessionValidation.bIsCanonical
				? TEXT("Shot capability requires a resolved LongShot/CutInsideShot branch.")
				: SessionValidation.ErrorMessage);
		return Result;
	}

	const bool bLongShotDirectSource =
		Source == ESource::LongShotDirectShotImmediateMiss
		|| Source == ESource::LongShotDirectShotFormulaGoal
		|| Source == ESource::LongShotDirectShotFormulaMiss;
	const bool bCutInsideDirectSource =
		Source == ESource::CutInsideShotDirectShotImmediateMiss
		|| Source == ESource::CutInsideShotDirectShotFormulaGoal
		|| Source == ESource::CutInsideShotDirectShotFormulaMiss;
	const bool bLongShotDeadCornerSource =
		Source == ESource::LongShotDeadCornerGoal
		|| Source == ESource::LongShotDeadCornerMiss;
	const bool bCutInsideDeadCornerSource =
		Source == ESource::CutInsideShotDeadCornerGoal
		|| Source == ESource::CutInsideShotDeadCornerMiss;
	const bool bSourceMatchesBranch =
		(bLongShotDirectSource
			&& Session.ActualBranch.ActionType == ESkillRuleType::LongShot
			&& Session.ActualBranch.LongShot
				== EMatchPlayLongShotActualBranch::DirectShot)
		|| (bCutInsideDirectSource
			&& Session.ActualBranch.ActionType
				== ESkillRuleType::CutInsideShot
			&& Session.ActualBranch.CutInsideShot
				== EMatchPlayCutInsideShotActualBranch::DirectShot)
		|| (bLongShotDeadCornerSource
			&& Session.ActualBranch.ActionType == ESkillRuleType::LongShot
			&& Session.ActualBranch.LongShot
				== EMatchPlayLongShotActualBranch::DeadCorner)
		|| (bCutInsideDeadCornerSource
			&& Session.ActualBranch.ActionType
				== ESkillRuleType::CutInsideShot
			&& Session.ActualBranch.CutInsideShot
				== EMatchPlayCutInsideShotActualBranch::DeadCorner);
	if (!bSourceMatchesBranch)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackCompletionErrorCode::InvalidCapabilityProvenance,
			TEXT("Shot terminal source does not match ActionType/ActualBranch."));
		return Result;
	}
	if (!ValidateScoreState(BeforeState, Result))
	{
		return Result;
	}

	FMatchPlayState WorkingState = BeforeState;
	if (Capability.IsGoal())
	{
		Result.GoalResolveResult = FGoalResolver::RecordGoal(
			WorkingState.RuntimeState,
			Attacker);
		if (!Result.GoalResolveResult.bSuccess)
		{
			SetError(
				Result,
				EMatchPlayCurrentAttackCompletionErrorCode::GoalResolutionFailed,
				Result.GoalResolveResult.ErrorMessage);
			return Result;
		}
		WorkingState.RuntimeState =
			Result.GoalResolveResult.UpdatedRuntimeState;
		Result.ScoringSide = Attacker;
	}

	return PersistCurrentAttackTerminal(
		BeforeState,
		MoveTemp(WorkingState),
		Attacker,
		Defender,
		MoveTemp(Result));
}

FMatchPlayCurrentAttackCompletionResult
FMatchPlayCurrentAttackCompletion::CompleteThroughBallResolution(
	const FMatchPlayState& BeforeState,
	const FMatchPlayThroughBallResolutionTerminalCapability& Capability)
{
	using namespace MatchPlayCurrentAttackCompletionImplementation;

	FMatchPlayCurrentAttackCompletionResult Result;
	Result.BeforeState = BeforeState;
	Result.AfterState = BeforeState;

	const EMatchPlayThroughBallTerminalSource Source = Capability.GetSource();
	const bool bGoalSource =
		Source == EMatchPlayThroughBallTerminalSource::FeetFormulaGoal
		|| Source
			== EMatchPlayThroughBallTerminalSource::AntiOffsideOneOnOneGoal
		|| Source
			== EMatchPlayThroughBallTerminalSource::BehindDefenseOneOnOneGoal;
	if (Source == EMatchPlayThroughBallTerminalSource::None)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackCompletionErrorCode
				::UnsupportedCapabilitySource,
			TEXT("ThroughBall completion requires a terminal source."));
		return Result;
	}
	if (Capability.IsGoal() != bGoalSource)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackCompletionErrorCode::InvalidCapabilityReason,
			TEXT("ThroughBall terminal source and Goal effect are inconsistent."));
		return Result;
	}

	EInitialTurnOrderPlayer Attacker = EInitialTurnOrderPlayer::None;
	EInitialTurnOrderPlayer Defender = EInitialTurnOrderPlayer::None;
	if (!ValidateCommonOuter(
		BeforeState,
		Capability.GetAttackSequence(),
		EMatchPlayCurrentAttackSelectionStage::ReadyForResolution,
		Result,
		Attacker,
		Defender))
	{
		return Result;
	}
	if (!BeforeState.CurrentAttack.bHasResolutionSession)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackCompletionErrorCode::InvalidCapabilityProvenance,
			TEXT("ThroughBall completion requires a Resolution Session."));
		return Result;
	}
	const auto SessionValidation =
		FMatchPlayCurrentAttackResolutionSessionStateValidator::Validate(
			BeforeState);
	if (!SessionValidation.bIsCanonical
		|| BeforeState.CurrentAttack.ResolutionSession.Stage
			!= EMatchPlayCurrentAttackResolutionStage::RouteResolved
		|| !BeforeState.CurrentAttack.ResolutionSession.bHasActualBranch
		|| BeforeState.CurrentAttack.ResolutionSession.ActualBranch.ActionType
			!= ESkillRuleType::ThroughBall)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackCompletionErrorCode::InvalidCapabilityProvenance,
			SessionValidation.bIsCanonical
				? TEXT("ThroughBall capability requires a resolved ThroughBall branch.")
				: SessionValidation.ErrorMessage);
		return Result;
	}
	const EMatchPlayThroughBallActualBranch ActualBranch =
		BeforeState.CurrentAttack.ResolutionSession.ActualBranch.ThroughBall;
	const bool bFeetSource =
		Source == EMatchPlayThroughBallTerminalSource::FeetFormulaGoal
		|| Source == EMatchPlayThroughBallTerminalSource::FeetFormulaMiss;
	const bool bAntiOffsideSource =
		Source == EMatchPlayThroughBallTerminalSource::AntiOffsideOffside
		|| Source
			== EMatchPlayThroughBallTerminalSource::AntiOffsideOneOnOneGoal
		|| Source
			== EMatchPlayThroughBallTerminalSource::AntiOffsideOneOnOneMiss;
	const bool bSourceMatchesBranch =
		(bFeetSource
			&& ActualBranch == EMatchPlayThroughBallActualBranch::Feet)
		|| (bAntiOffsideSource
			&& ActualBranch == EMatchPlayThroughBallActualBranch::AntiOffside)
		|| (!bFeetSource && !bAntiOffsideSource
			&& ActualBranch
				== EMatchPlayThroughBallActualBranch::BehindDefense);
	if (!bSourceMatchesBranch)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackCompletionErrorCode::InvalidCapabilityProvenance,
			TEXT("ThroughBall terminal source does not match ActualBranch."));
		return Result;
	}
	if (!ValidateScoreState(BeforeState, Result))
	{
		return Result;
	}

	FMatchPlayState WorkingState = BeforeState;
	if (Capability.IsGoal())
	{
		Result.GoalResolveResult = FGoalResolver::RecordGoal(
			WorkingState.RuntimeState,
			Attacker);
		if (!Result.GoalResolveResult.bSuccess)
		{
			SetError(
				Result,
				EMatchPlayCurrentAttackCompletionErrorCode::GoalResolutionFailed,
				Result.GoalResolveResult.ErrorMessage);
			return Result;
		}
		WorkingState.RuntimeState =
			Result.GoalResolveResult.UpdatedRuntimeState;
		Result.ScoringSide = Attacker;
	}

	return PersistCurrentAttackTerminal(
		BeforeState,
		MoveTemp(WorkingState),
		Attacker,
		Defender,
		MoveTemp(Result));
}

FMatchPlayCurrentAttackCompletionResult
FMatchPlayCurrentAttackCompletion::CompleteCarrierNoGoal(
	const FMatchPlayState& BeforeState,
	const FMatchPlayNoLegalCarrierCompletionCapability& Capability)
{
	using namespace MatchPlayCurrentAttackCompletionImplementation;

	FMatchPlayCurrentAttackCompletionResult Result;
	Result.BeforeState = BeforeState;
	Result.AfterState = BeforeState;

	EInitialTurnOrderPlayer Attacker = EInitialTurnOrderPlayer::None;
	EInitialTurnOrderPlayer Defender = EInitialTurnOrderPlayer::None;
	if (!ValidateCommonOuter(
		BeforeState,
		Capability.GetAttackSequence(),
		EMatchPlayCurrentAttackSelectionStage::AwaitingCarrier,
		Result,
		Attacker,
		Defender))
	{
		return Result;
	}

	FString CapabilityProvenanceError;
	if (!ValidateCarrierAvailabilityProvenance(
		BeforeState,
		Capability,
		Attacker,
		CapabilityProvenanceError))
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackCompletionErrorCode
				::InvalidCapabilityProvenance,
			CapabilityProvenanceError);
		return Result;
	}
	if (!ValidateScoreState(BeforeState, Result))
	{
		return Result;
	}

	return ApplyCurrentAttackAdvanceMutation(
		BeforeState,
		BeforeState,
		Attacker,
		Defender,
		MoveTemp(Result));
}

FMatchPlayCurrentAttackCompletionResult
FMatchPlayCurrentAttackCompletion::CompleteMarkerGoal(
	const FMatchPlayState& BeforeState,
	const FMatchPlayMarkerNoSelectionGoalCapability& Capability)
{
	using namespace MatchPlayCurrentAttackCompletionImplementation;

	FMatchPlayCurrentAttackCompletionResult Result;
	Result.BeforeState = BeforeState;
	Result.AfterState = BeforeState;
	Result.Reason = Capability.GetReason();
	Result.Source = Capability.GetSource();

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
			TEXT("Capability source is not supported by marker completion."));
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
			TEXT("Capability reason is not supported by marker completion."));
		return Result;
	}

	EInitialTurnOrderPlayer Attacker =
		EInitialTurnOrderPlayer::None;
	EInitialTurnOrderPlayer Defender =
		EInitialTurnOrderPlayer::None;
	if (!ValidateCommonOuter(
		BeforeState,
		Capability.GetAttackSequence(),
		EMatchPlayCurrentAttackSelectionStage::AwaitingMarker,
		Result,
		Attacker,
		Defender))
	{
		return Result;
	}

	FString CapabilityProvenanceError;
	if (!ValidateMarkerAvailabilityProvenance(
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
	if (!ValidateScoreState(BeforeState, Result))
	{
		return Result;
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
	Result.ScoringSide = Attacker;

	return ApplyCurrentAttackAdvanceMutation(
		BeforeState,
		MoveTemp(WorkingState),
		Attacker,
		Defender,
		MoveTemp(Result));
}

FMatchPlayCurrentAttackCompletionResult
FMatchPlayCurrentAttackCompletion::CompleteSkillNoGoal(
	const FMatchPlayState& BeforeState,
	const FMatchPlaySkillNoSelectionNoGoalCapability& Capability)
{
	using namespace MatchPlayCurrentAttackCompletionImplementation;

	FMatchPlayCurrentAttackCompletionResult Result;
	Result.BeforeState = BeforeState;
	Result.AfterState = BeforeState;

	if (Capability.GetSource()
			!= EMatchPlaySkillNoSelectionNoGoalSource
				::ResolveNoLegalSkill
		&& Capability.GetSource()
			!= EMatchPlaySkillNoSelectionNoGoalSource::DeclineSkill)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackCompletionErrorCode
				::UnsupportedCapabilitySource,
			TEXT("Capability source is not supported by skill no-goal completion."));
		return Result;
	}
	if ((Capability.GetSource()
				== EMatchPlaySkillNoSelectionNoGoalSource
					::ResolveNoLegalSkill
			&& Capability.GetReason()
				!= EMatchPlaySkillNoSelectionNoGoalReason
					::NoLegalSkill)
		|| (Capability.GetSource()
				== EMatchPlaySkillNoSelectionNoGoalSource
					::DeclineSkill
			&& Capability.GetReason()
				!= EMatchPlaySkillNoSelectionNoGoalReason
					::SkillDeclined))
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackCompletionErrorCode
				::InvalidCapabilityReason,
			TEXT("Skill capability Source and Reason are not a supported pair."));
		return Result;
	}

	EInitialTurnOrderPlayer Attacker =
		EInitialTurnOrderPlayer::None;
	EInitialTurnOrderPlayer Defender =
		EInitialTurnOrderPlayer::None;
	if (!ValidateCommonOuter(
		BeforeState,
		Capability.GetAttackSequence(),
		EMatchPlayCurrentAttackSelectionStage::AwaitingSkill,
		Result,
		Attacker,
		Defender))
	{
		return Result;
	}

	FString CapabilityProvenanceError;
	if (!ValidateSkillAvailabilityProvenance(
		BeforeState,
		Capability,
		Attacker,
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
	if (!ValidateScoreState(BeforeState, Result))
	{
		return Result;
	}

	return ApplyCurrentAttackAdvanceMutation(
		BeforeState,
		BeforeState,
		Attacker,
		Defender,
		MoveTemp(Result));
}

FMatchPlayCurrentAttackCompletionResult
FMatchPlayCurrentAttackCompletion::CompleteRunnerNoGoal(
	const FMatchPlayState& BeforeState,
	const FMatchPlayRunnerNoSelectionNoGoalCapability& Capability)
{
	using namespace MatchPlayCurrentAttackCompletionImplementation;

	FMatchPlayCurrentAttackCompletionResult Result;
	Result.BeforeState = BeforeState;
	Result.AfterState = BeforeState;

	if (Capability.GetSource()
			!= EMatchPlayRunnerNoSelectionNoGoalSource
				::ResolveNoLegalRunner
		&& Capability.GetSource()
			!= EMatchPlayRunnerNoSelectionNoGoalSource::RunnerDecline)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackCompletionErrorCode
				::UnsupportedCapabilitySource,
			TEXT("Capability source is not supported by runner no-goal completion."));
		return Result;
	}
	if ((Capability.GetSource()
				== EMatchPlayRunnerNoSelectionNoGoalSource
					::ResolveNoLegalRunner
			&& Capability.GetReason()
				!= EMatchPlayRunnerNoSelectionNoGoalReason
					::NoLegalRunner)
		|| (Capability.GetSource()
				== EMatchPlayRunnerNoSelectionNoGoalSource
					::RunnerDecline
			&& Capability.GetReason()
				!= EMatchPlayRunnerNoSelectionNoGoalReason
					::RunnerDeclined))
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackCompletionErrorCode
				::InvalidCapabilityReason,
			TEXT("Runner capability Source and Reason are not a supported pair."));
		return Result;
	}

	EInitialTurnOrderPlayer Attacker =
		EInitialTurnOrderPlayer::None;
	EInitialTurnOrderPlayer Defender =
		EInitialTurnOrderPlayer::None;
	if (!ValidateCommonOuter(
		BeforeState,
		Capability.GetAttackSequence(),
		EMatchPlayCurrentAttackSelectionStage::AwaitingRunner,
		Result,
		Attacker,
		Defender))
	{
		return Result;
	}

	FString CapabilityProvenanceError;
	if (!ValidateRunnerAvailabilityProvenance(
		BeforeState,
		Capability,
		Attacker,
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
	if (!ValidateScoreState(BeforeState, Result))
	{
		return Result;
	}

	return ApplyCurrentAttackAdvanceMutation(
		BeforeState,
		BeforeState,
		Attacker,
		Defender,
		MoveTemp(Result));
}

FMatchPlayCurrentAttackCompletionResult
FMatchPlayCurrentAttackCompletion
	::PersistCurrentAttackTerminal(
		const FMatchPlayState& BeforeState,
		FMatchPlayState WorkingState,
		const EInitialTurnOrderPlayer Attacker,
		const EInitialTurnOrderPlayer Defender,
		FMatchPlayCurrentAttackCompletionResult Result)
{
	using namespace MatchPlayCurrentAttackCompletionImplementation;

	if (BeforeState.CurrentAttack.LifecycleState
		!= EMatchPlayCurrentAttackLifecycleState::Active)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackCompletionErrorCode
				::CurrentAttackAlreadyTerminal,
			TEXT("Current attack terminal outcome is already persisted and awaiting advance."));
		return Result;
	}

	// Validate the exact future clear/consume/handoff mutation now, but do not
	// adopt any of it. Terminal application is an authoritative persistence
	// transition: score/result effects are final, while pitch, roles, card
	// usage, opportunity count, and attacker ownership remain unchanged.
	FMatchPlayState TerminalState = WorkingState;
	FMatchPlayCurrentAttackCompletionResult ValidatedAdvance =
		ApplyCurrentAttackAdvanceMutation(
			BeforeState,
			MoveTemp(WorkingState),
			Attacker,
			Defender,
			MoveTemp(Result));
	if (!ValidatedAdvance.bSuccess)
	{
		return ValidatedAdvance;
	}

	TerminalState.CurrentAttack.LifecycleState =
		EMatchPlayCurrentAttackLifecycleState::TerminalPendingAdvance;
	ValidatedAdvance.AfterState = MoveTemp(TerminalState);
	ValidatedAdvance.OrdinaryCardUsageResults.Reset();
	ValidatedAdvance.OpportunityResolveResult =
		FAttackOpportunityResolveResult();
	ValidatedAdvance.MatchEndResolveResult = FMatchEndResolveResult();
	ValidatedAdvance.MatchResultResolveResult = FMatchResultResolveResult();
	ValidatedAdvance.NextAttackingPlayer = EInitialTurnOrderPlayer::None;
	ValidatedAdvance.bMatchEnded = false;
	return ValidatedAdvance;
}

FMatchPlayCurrentAttackCompletionResult
FMatchPlayCurrentAttackCompletion::AdvanceAfterTerminal(
	const FMatchPlayState& BeforeState,
	const int64 AttackSequence,
	const EInitialTurnOrderPlayer RequestingSide)
{
	using namespace MatchPlayCurrentAttackCompletionImplementation;

	FMatchPlayCurrentAttackCompletionResult Result;
	Result.BeforeState = BeforeState;
	Result.AfterState = BeforeState;

	EInitialTurnOrderPlayer Attacker = EInitialTurnOrderPlayer::None;
	EInitialTurnOrderPlayer Defender = EInitialTurnOrderPlayer::None;
	const EMatchPlayCurrentAttackSelectionStage CurrentStage =
		BeforeState.bHasCurrentAttack
			? BeforeState.CurrentAttack.SelectionStage
			: EMatchPlayCurrentAttackSelectionStage::None;
	if (!ValidateCommonOuter(
		BeforeState,
		AttackSequence,
		CurrentStage,
		Result,
		Attacker,
		Defender))
	{
		return Result;
	}
	if (BeforeState.CurrentAttack.LifecycleState
		!= EMatchPlayCurrentAttackLifecycleState::TerminalPendingAdvance)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackCompletionErrorCode
				::CurrentAttackNotTerminalPendingAdvance,
			TEXT("Advance requires a terminal current attack awaiting explicit next-round progression."));
		return Result;
	}
	if (RequestingSide != Attacker)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackCompletionErrorCode
				::UnauthorizedAdvanceRequester,
			TEXT("Only the current attacking player may advance after terminal."));
		return Result;
	}
	if (!ValidateScoreState(BeforeState, Result))
	{
		return Result;
	}

	return ApplyCurrentAttackAdvanceMutation(
		BeforeState,
		BeforeState,
		Attacker,
		Defender,
		MoveTemp(Result));
}

FMatchPlayCurrentAttackCompletionResult
FMatchPlayCurrentAttackCompletion
	::ApplyCurrentAttackAdvanceMutation(
		const FMatchPlayState& BeforeState,
		FMatchPlayState WorkingState,
		const EInitialTurnOrderPlayer Attacker,
		const EInitialTurnOrderPlayer Defender,
		FMatchPlayCurrentAttackCompletionResult Result)
{
	using namespace MatchPlayCurrentAttackCompletionImplementation;

	const FMatchPlayCurrentAttackState& CurrentAttack =
		BeforeState.CurrentAttack;
	TSet<FString> SeenDeploymentCards;
	TSet<FName> SeenDeploymentSlots;
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

		if (SeenDeploymentSlots.Contains(Placement.SlotId))
		{
			SetError(
				Result,
				EMatchPlayCurrentAttackCompletionErrorCode
					::DuplicateDeploymentSlot,
				FString::Printf(
					TEXT("Deployment slot '%s' is occupied more than once."),
					*Placement.SlotId.ToString()));
			return Result;
		}
		SeenDeploymentSlots.Add(Placement.SlotId);

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

	Result.AfterState = MoveTemp(WorkingState);
	Result.bSuccess = true;
	Result.ErrorCode =
		EMatchPlayCurrentAttackCompletionErrorCode::None;
	Result.ErrorMessage.Empty();
	return Result;
}
