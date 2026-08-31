#include "MatchPlayCornerResolution.h"

#include "GoalResolver.h"

namespace MatchPlayCornerResolution
{
	using EError = EMatchPlayCornerResolutionErrorCode;

	bool IsPlayer(const EInitialTurnOrderPlayer Side)
	{
		return Side == EInitialTurnOrderPlayer::PlayerA
			|| Side == EInitialTurnOrderPlayer::PlayerB;
	}

	EInitialTurnOrderPlayer GetDefender(
		const EInitialTurnOrderPlayer Attacker)
	{
		return Attacker == EInitialTurnOrderPlayer::PlayerA
			? EInitialTurnOrderPlayer::PlayerB
			: Attacker == EInitialTurnOrderPlayer::PlayerB
				? EInitialTurnOrderPlayer::PlayerA
				: EInitialTurnOrderPlayer::None;
	}

	FName GetSideId(const EInitialTurnOrderPlayer Side)
	{
		return Side == EInitialTurnOrderPlayer::PlayerA
			? FName(TEXT("PlayerA"))
			: Side == EInitialTurnOrderPlayer::PlayerB
				? FName(TEXT("PlayerB"))
				: NAME_None;
	}

	void Fail(
		FMatchPlayCornerResolutionResult& Result,
		const EError ErrorCode,
		const FString& ErrorMessage)
	{
		Result.ErrorCode = ErrorCode;
		Result.ErrorMessage = ErrorMessage;
		Result.AfterState = Result.BeforeState;
	}

	bool ValidateBase(
		FMatchPlayCornerResolutionResult& Result,
		const FMatchPlayState& State,
		const int64 AttackSequence,
		const EInitialTurnOrderPlayer RequestingSide,
		const EMatchPlaySetPieceCornerRouteStage RequiredStage,
		const bool bDefenderOwned,
		EInitialTurnOrderPlayer& OutAttacker,
		EInitialTurnOrderPlayer& OutDefender)
	{
		if (!State.RuntimeState.bIsInitialized)
		{
			Fail(Result, EError::MatchPlayStateNotInitialized,
				TEXT("Corner resolution requires initialized match play state."));
			return false;
		}
		if (!State.bHasCurrentAttack)
		{
			Fail(Result, EError::NoCurrentAttack,
				TEXT("Corner resolution requires a current attack."));
			return false;
		}
		if (AttackSequence <= 0 || State.CurrentAttack.AttackSequence <= 0)
		{
			Fail(Result, EError::InvalidAttackSequence,
				TEXT("Corner AttackSequence must be positive."));
			return false;
		}
		if (AttackSequence != State.CurrentAttack.AttackSequence)
		{
			Fail(Result, EError::AttackSequenceMismatch,
				TEXT("Corner request sequence does not match the current attack."));
			return false;
		}
		if (State.CurrentAttack.RouteKind
			!= EMatchPlayCurrentAttackRouteKind::SetPiece)
		{
			Fail(Result, EError::WrongRoute,
				TEXT("Corner resolution requires the SetPiece route."));
			return false;
		}
		if (State.CurrentAttack.SetPieceRoute.SelectedType
			!= ESetPieceSelectedType::Corner)
		{
			Fail(Result, EError::WrongSetPieceType,
				TEXT("Corner resolution requires Corner type."));
			return false;
		}
		if (State.CurrentAttack.SetPieceRoute.Corner.Stage != RequiredStage)
		{
			Fail(Result, EError::WrongStage,
				TEXT("Corner request is not legal in the current stage."));
			return false;
		}
		if (!IsPlayer(RequestingSide))
		{
			Fail(Result, EError::InvalidRequestingSide,
				TEXT("RequestingSide must be PlayerA or PlayerB."));
			return false;
		}

		OutAttacker = State.RuntimeState.CurrentAttackingPlayer;
		OutDefender = GetDefender(OutAttacker);
		if (!IsPlayer(OutAttacker) || !IsPlayer(OutDefender)
			|| RequestingSide != (bDefenderOwned ? OutDefender : OutAttacker))
		{
			Fail(Result, EError::UnauthorizedRequester,
				bDefenderOwned
					? TEXT("Only the current defender may submit this Corner request.")
					: TEXT("Only the current attacker may submit this Corner request."));
			return false;
		}

		Result.BeforeRouteValidation =
			FMatchPlayCurrentAttackRouteStateValidator::Validate(State);
		if (!Result.BeforeRouteValidation.bIsCanonical)
		{
			Fail(Result, EError::InvalidRouteState,
				Result.BeforeRouteValidation.ErrorMessage);
			return false;
		}
		return true;
	}

	bool ValidateCandidate(
		FMatchPlayCornerResolutionResult& Result,
		FMatchPlayState&& Candidate)
	{
		Result.AfterRouteValidation =
			FMatchPlayCurrentAttackRouteStateValidator::Validate(Candidate);
		if (!Result.AfterRouteValidation.bIsCanonical)
		{
			Fail(Result, EError::InvalidCandidateRouteState,
				Result.AfterRouteValidation.ErrorMessage);
			return false;
		}
		Result.AfterState = MoveTemp(Candidate);
		Result.bSuccess = true;
		Result.ErrorCode = EError::None;
		return true;
	}

	bool ValidateNominees(
		FMatchPlayCornerResolutionResult& Result,
		const FMatchPlayState& State,
		const TArray<FName>& OrderedCardIds,
		const EInitialTurnOrderPlayer OwnerSide,
		const EMatchPlaySetPieceParticipantRole Role,
		TArray<FMatchPlaySetPieceParticipantBinding>& OutBindings)
	{
		if (OrderedCardIds.Num() > 3)
		{
			Fail(Result, EError::TooManyNominees,
				TEXT("Corner nomination list may contain at most three cards."));
			return false;
		}

		TSet<FName> SeenCardIds;
		for (const FName CardId : OrderedCardIds)
		{
			if (SeenCardIds.Contains(CardId))
			{
				Fail(Result, EError::DuplicateNominee,
					TEXT("Corner nomination list cannot contain duplicate CardIds."));
				return false;
			}
			SeenCardIds.Add(CardId);

			FMatchPlaySetPieceParticipantEligibilityRequest EligibilityRequest;
			EligibilityRequest.ExpectedOwnerSide = OwnerSide;
			EligibilityRequest.CardId = CardId;
			EligibilityRequest.Role = Role;
			const FMatchPlaySetPieceParticipantEligibilityResult Eligibility =
				FMatchPlaySetPieceParticipantEligibility::Evaluate(
					State, EligibilityRequest);
			Result.NomineeEligibilityResults.Add(Eligibility);
			if (!Eligibility.bIsEligible)
			{
				Fail(Result, EError::NomineeNotEligible,
					Eligibility.ErrorMessage);
				return false;
			}
			OutBindings.Add(Eligibility.Binding);
		}
		return true;
	}

	int32 MapParticipantIndex(const int32 CandidateCount, const int32 RawD6)
	{
		if (CandidateCount == 3)
		{
			return (RawD6 - 1) / 2;
		}
		if (CandidateCount == 2)
		{
			return RawD6 <= 3 ? 0 : 1;
		}
		return CandidateCount == 1 ? 0 : INDEX_NONE;
	}

	void ApplyCandidateBonus(
		FMatchPlayCornerRouteState& Corner,
		const EInitialTurnOrderPlayer Attacker,
		const EInitialTurnOrderPlayer Defender)
	{
		const int32 Difference = FMath::Abs(
			Corner.AttackerNominees.Num() - Corner.DefenderNominees.Num());
		Corner.CandidateBonus = Difference == 1 ? 2 : Difference == 2 ? 3 : 0;
		Corner.CandidateBonusSide = Corner.CandidateBonus == 0
			? EInitialTurnOrderPlayer::None
			: Corner.AttackerNominees.Num() > Corner.DefenderNominees.Num()
				? Attacker
				: Defender;
	}

	EMatchPlayCornerRouteIntent SwitchRoute(
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

	FFormulaResolverInput BuildFormulaInput(
		const FMatchPlayCornerRouteState& Corner,
		const FMatchPlayDefendingGoalkeeperQueryResult& Goalkeeper,
		const EInitialTurnOrderPlayer Attacker,
		const EInitialTurnOrderPlayer Defender,
		const int64 AttackSequence)
	{
		FFormulaResolverInput Input;
		Input.FormulaType = EFormulaType::Finishing;
		Input.Attacker.BaseValue = Corner.ActualRoute
			== EMatchPlayCornerRouteIntent::High
				? Corner.Runner.Snapshot.Attributes.Strength
				: Corner.Runner.Snapshot.Attributes.Shooting;
		Input.Attacker.Modifier = Corner.CandidateBonusSide == Attacker
			? static_cast<float>(Corner.CandidateBonus) : 0.0f;
		Input.Attacker.ComparePoint = Corner.AttackD6;
		Input.Attacker.bComparePointWasRolledOnD6 = true;
		Input.Attacker.ParticipatingStamina.Add(
			Corner.Runner.Snapshot.Attributes.Stamina);

		const float HelperAttribute = Corner.ActualRoute
			== EMatchPlayCornerRouteIntent::High
				? Corner.Helper.Snapshot.Attributes.Strength
				: Corner.Helper.Snapshot.Attributes.Marking;
		const float GoalkeeperAttribute = Corner.ActualRoute
			== EMatchPlayCornerRouteIntent::High
				? Goalkeeper.Snapshot.GoalkeeperAttributes.Aerial
				: Goalkeeper.Snapshot.GoalkeeperAttributes.Reflex;
		Input.Defender.BaseValue = UFormulaResolver::Average(
			HelperAttribute, GoalkeeperAttribute);
		Input.Defender.Modifier = 2.0f
			+ (Corner.CandidateBonusSide == Defender
				? static_cast<float>(Corner.CandidateBonus) : 0.0f);
		Input.Defender.ComparePoint = Corner.DefenseD6;
		Input.Defender.bComparePointWasRolledOnD6 = true;
		Input.Defender.ParticipatingStamina = {
			Corner.Helper.Snapshot.Attributes.Stamina,
			Goalkeeper.Snapshot.Attributes.Stamina };
		Input.bGoalkeeperParticipated = true;
		Input.TurnIndex = static_cast<int32>(AttackSequence);
		Input.AttackerPlayerId = GetSideId(Attacker);
		Input.DefenderPlayerId = GetSideId(Defender);
		Input.InvolvedCardIds = {
			Corner.Runner.CardId, Corner.Helper.CardId, Goalkeeper.CardId };
		return Input;
	}

	bool ValidateFormulaResult(const FFormulaResolutionResult& Formula)
	{
		return Formula.FormulaType == EFormulaType::Finishing
			&& FMath::IsFinite(Formula.AttackerFinalValue)
			&& FMath::IsFinite(Formula.DefenderFinalValue)
			&& Formula.Winner != EFormulaWinner::None
			&& Formula.bAttackEnded
			&& !Formula.bContinueResolution;
	}

	bool ObtainRoll(
		FMatchPlayCornerResolutionResult& Result,
		IMatchPlayPostRouteRollProvider* RollProvider,
		const EMatchPlayCurrentAttackPostRouteRollPurpose Purpose)
	{
		if (RollProvider == nullptr)
		{
			Fail(Result, EError::ProviderUnavailable,
				TEXT("No authoritative Corner roll provider is configured."));
			return false;
		}
		Result.ProviderResult = RollProvider->RollD6(Purpose);
		Result.ProviderValidation =
			FMatchPlayPostRouteRollProviderResultValidator::Validate(
				Purpose, Result.ProviderResult);
		if (!Result.ProviderValidation.bIsCanonical)
		{
			Fail(Result,
				Result.ProviderValidation.ErrorCode
					== EMatchPlayPostRouteRollProviderResultValidationErrorCode
						::ProviderFailure
						? EError::ProviderFailure
						: EError::MalformedProviderResult,
				Result.ProviderValidation.ErrorMessage);
			return false;
		}
		return true;
	}
}

FMatchPlayCornerResolutionResult
FMatchPlayCornerResolution::SubmitAttackerNominations(
	const FMatchPlayState& BeforeState,
	const FMatchPlayCornerNominationRequest& Request)
{
	using namespace MatchPlayCornerResolution;
	FMatchPlayCornerResolutionResult Result;
	Result.BeforeState = BeforeState;
	Result.AfterState = BeforeState;
	EInitialTurnOrderPlayer Attacker = EInitialTurnOrderPlayer::None;
	EInitialTurnOrderPlayer Defender = EInitialTurnOrderPlayer::None;
	if (!ValidateBase(Result, BeforeState, Request.AttackSequence,
		Request.RequestingSide,
		EMatchPlaySetPieceCornerRouteStage::AwaitingAttackerNominations,
		false, Attacker, Defender))
	{
		return Result;
	}

	TArray<FMatchPlaySetPieceParticipantBinding> Bindings;
	if (!ValidateNominees(Result, BeforeState, Request.OrderedCardIds,
		Attacker, EMatchPlaySetPieceParticipantRole::CornerRunner, Bindings))
	{
		return Result;
	}

	FMatchPlayState Candidate = BeforeState;
	FMatchPlayCornerRouteState& Corner =
		Candidate.CurrentAttack.SetPieceRoute.Corner;
	Corner.AttackerNominees = MoveTemp(Bindings);
	Corner.bAttackerNominationsLocked = true;
	Corner.Stage =
		EMatchPlaySetPieceCornerRouteStage::AwaitingDefenderNominations;
	ValidateCandidate(Result, MoveTemp(Candidate));
	return Result;
}

FMatchPlayCornerResolutionResult
FMatchPlayCornerResolution::SubmitDefenderNominations(
	const FMatchPlayState& BeforeState,
	const FMatchPlayCornerNominationRequest& Request)
{
	using namespace MatchPlayCornerResolution;
	FMatchPlayCornerResolutionResult Result;
	Result.BeforeState = BeforeState;
	Result.AfterState = BeforeState;
	EInitialTurnOrderPlayer Attacker = EInitialTurnOrderPlayer::None;
	EInitialTurnOrderPlayer Defender = EInitialTurnOrderPlayer::None;
	if (!ValidateBase(Result, BeforeState, Request.AttackSequence,
		Request.RequestingSide,
		EMatchPlaySetPieceCornerRouteStage::AwaitingDefenderNominations,
		true, Attacker, Defender))
	{
		return Result;
	}

	TArray<FMatchPlaySetPieceParticipantBinding> Bindings;
	if (!ValidateNominees(Result, BeforeState, Request.OrderedCardIds,
		Defender, EMatchPlaySetPieceParticipantRole::CornerHelper, Bindings))
	{
		return Result;
	}

	FMatchPlayState Candidate = BeforeState;
	FMatchPlayCornerRouteState& Corner =
		Candidate.CurrentAttack.SetPieceRoute.Corner;
	Corner.DefenderNominees = MoveTemp(Bindings);
	Corner.bDefenderNominationsLocked = true;
	if (Corner.AttackerNominees.IsEmpty())
	{
		PersistTerminal(Result, BeforeState, MoveTemp(Candidate), Attacker,
			EMatchPlayCornerGameplayOutcome::NoGoal);
		return Result;
	}
	if (Corner.DefenderNominees.IsEmpty())
	{
		PersistTerminal(Result, BeforeState, MoveTemp(Candidate), Attacker,
			EMatchPlayCornerGameplayOutcome::SystemGoal);
		return Result;
	}

	Corner.Stage = EMatchPlaySetPieceCornerRouteStage
		::AwaitingParticipantSelectionRoll;
	ValidateCandidate(Result, MoveTemp(Candidate));
	return Result;
}

FMatchPlayCornerResolutionResult
FMatchPlayCornerResolution::RequestParticipantSelectionRoll(
	const FMatchPlayState& BeforeState,
	const FMatchPlayCornerRollRequest& Request,
	IMatchPlayPostRouteRollProvider* RollProvider)
{
	using namespace MatchPlayCornerResolution;
	FMatchPlayCornerResolutionResult Result;
	Result.BeforeState = BeforeState;
	Result.AfterState = BeforeState;
	EInitialTurnOrderPlayer Attacker = EInitialTurnOrderPlayer::None;
	EInitialTurnOrderPlayer Defender = EInitialTurnOrderPlayer::None;
	if (!ValidateBase(Result, BeforeState, Request.AttackSequence,
		Request.RequestingSide,
		EMatchPlaySetPieceCornerRouteStage::AwaitingParticipantSelectionRoll,
		false, Attacker, Defender))
	{
		return Result;
	}
	if (!ObtainRoll(Result, RollProvider,
		EMatchPlayCurrentAttackPostRouteRollPurpose
			::CornerParticipantSelection))
	{
		return Result;
	}

	FMatchPlayState Candidate = BeforeState;
	FMatchPlayCornerRouteState& Corner =
		Candidate.CurrentAttack.SetPieceRoute.Corner;
	Corner.bHasSharedParticipantD6 = true;
	Corner.SharedParticipantD6 = Result.ProviderResult.RawD6;
	const int32 RunnerIndex = MapParticipantIndex(
		Corner.AttackerNominees.Num(), Corner.SharedParticipantD6);
	const int32 HelperIndex = MapParticipantIndex(
		Corner.DefenderNominees.Num(), Corner.SharedParticipantD6);
	if (!Corner.AttackerNominees.IsValidIndex(RunnerIndex)
		|| !Corner.DefenderNominees.IsValidIndex(HelperIndex))
	{
		Fail(Result, EError::InvalidRouteState,
			TEXT("Corner shared D6 could not map canonical participant lists."));
		return Result;
	}
	Corner.Runner = Corner.AttackerNominees[RunnerIndex];
	Corner.Helper = Corner.DefenderNominees[HelperIndex];
	ApplyCandidateBonus(Corner, Attacker, Defender);
	Corner.Stage = EMatchPlaySetPieceCornerRouteStage::AwaitingIntent;
	ValidateCandidate(Result, MoveTemp(Candidate));
	return Result;
}

FMatchPlayCornerResolutionResult FMatchPlayCornerResolution::SubmitIntent(
	const FMatchPlayState& BeforeState,
	const FMatchPlayCornerIntentRequest& Request)
{
	using namespace MatchPlayCornerResolution;
	FMatchPlayCornerResolutionResult Result;
	Result.BeforeState = BeforeState;
	Result.AfterState = BeforeState;
	EInitialTurnOrderPlayer Attacker = EInitialTurnOrderPlayer::None;
	EInitialTurnOrderPlayer Defender = EInitialTurnOrderPlayer::None;
	if (!ValidateBase(Result, BeforeState, Request.AttackSequence,
		Request.RequestingSide,
		EMatchPlaySetPieceCornerRouteStage::AwaitingIntent,
		false, Attacker, Defender))
	{
		return Result;
	}
	if (Request.IntendedRoute != EMatchPlayCornerRouteIntent::High
		&& Request.IntendedRoute != EMatchPlayCornerRouteIntent::Low)
	{
		Fail(Result, EError::InvalidIntent,
			TEXT("Corner intended route must be High or Low."));
		return Result;
	}

	FMatchPlayState Candidate = BeforeState;
	FMatchPlayCornerRouteState& Corner =
		Candidate.CurrentAttack.SetPieceRoute.Corner;
	Corner.IntendedRoute = Request.IntendedRoute;
	Corner.Stage = EMatchPlaySetPieceCornerRouteStage::AwaitingRouteRoll;
	ValidateCandidate(Result, MoveTemp(Candidate));
	return Result;
}

FMatchPlayCornerResolutionResult FMatchPlayCornerResolution::RequestRouteRoll(
	const FMatchPlayState& BeforeState,
	const FMatchPlayCornerRollRequest& Request,
	IMatchPlayPostRouteRollProvider* RollProvider)
{
	using namespace MatchPlayCornerResolution;
	FMatchPlayCornerResolutionResult Result;
	Result.BeforeState = BeforeState;
	Result.AfterState = BeforeState;
	EInitialTurnOrderPlayer Attacker = EInitialTurnOrderPlayer::None;
	EInitialTurnOrderPlayer Defender = EInitialTurnOrderPlayer::None;
	if (!ValidateBase(Result, BeforeState, Request.AttackSequence,
		Request.RequestingSide,
		EMatchPlaySetPieceCornerRouteStage::AwaitingRouteRoll,
		false, Attacker, Defender))
	{
		return Result;
	}
	if (!ObtainRoll(Result, RollProvider,
		EMatchPlayCurrentAttackPostRouteRollPurpose::CornerRoute))
	{
		return Result;
	}

	FMatchPlayState Candidate = BeforeState;
	FMatchPlayCornerRouteState& Corner =
		Candidate.CurrentAttack.SetPieceRoute.Corner;
	Corner.bHasRouteD6 = true;
	Corner.RawRouteD6 = Result.ProviderResult.RawD6;
	Corner.ActualRoute = SwitchRoute(Corner.IntendedRoute, Corner.RawRouteD6);
	Corner.Stage = EMatchPlaySetPieceCornerRouteStage::AwaitingAttackRoll;
	ValidateCandidate(Result, MoveTemp(Candidate));
	return Result;
}

FMatchPlayCornerResolutionResult FMatchPlayCornerResolution::RequestAttackRoll(
	const FMatchPlayState& BeforeState,
	const FMatchPlayCornerRollRequest& Request,
	IMatchPlayPostRouteRollProvider* RollProvider)
{
	using namespace MatchPlayCornerResolution;
	FMatchPlayCornerResolutionResult Result;
	Result.BeforeState = BeforeState;
	Result.AfterState = BeforeState;
	EInitialTurnOrderPlayer Attacker = EInitialTurnOrderPlayer::None;
	EInitialTurnOrderPlayer Defender = EInitialTurnOrderPlayer::None;
	if (!ValidateBase(Result, BeforeState, Request.AttackSequence,
		Request.RequestingSide,
		EMatchPlaySetPieceCornerRouteStage::AwaitingAttackRoll,
		false, Attacker, Defender))
	{
		return Result;
	}
	Result.GoalkeeperQueryResult =
		FMatchPlayDefendingGoalkeeperQuery::Query(BeforeState, Defender);
	if (!Result.GoalkeeperQueryResult.bSuccess)
	{
		Fail(Result, EError::DefendingGoalkeeperUnavailable,
			Result.GoalkeeperQueryResult.ErrorMessage);
		return Result;
	}
	if (!ObtainRoll(Result, RollProvider,
		EMatchPlayCurrentAttackPostRouteRollPurpose::CornerAttack))
	{
		return Result;
	}

	FMatchPlayState Candidate = BeforeState;
	FMatchPlayCornerRouteState& Corner =
		Candidate.CurrentAttack.SetPieceRoute.Corner;
	Corner.bHasAttackD6 = true;
	Corner.AttackD6 = Result.ProviderResult.RawD6;
	Corner.Stage = EMatchPlaySetPieceCornerRouteStage::AwaitingDefenseRoll;
	ValidateCandidate(Result, MoveTemp(Candidate));
	return Result;
}

FMatchPlayCornerResolutionResult FMatchPlayCornerResolution::RequestDefenseRoll(
	const FMatchPlayState& BeforeState,
	const FMatchPlayCornerRollRequest& Request,
	IMatchPlayPostRouteRollProvider* RollProvider)
{
	using namespace MatchPlayCornerResolution;
	FMatchPlayCornerResolutionResult Result;
	Result.BeforeState = BeforeState;
	Result.AfterState = BeforeState;
	EInitialTurnOrderPlayer Attacker = EInitialTurnOrderPlayer::None;
	EInitialTurnOrderPlayer Defender = EInitialTurnOrderPlayer::None;
	if (!ValidateBase(Result, BeforeState, Request.AttackSequence,
		Request.RequestingSide,
		EMatchPlaySetPieceCornerRouteStage::AwaitingDefenseRoll,
		true, Attacker, Defender))
	{
		return Result;
	}
	Result.GoalkeeperQueryResult =
		FMatchPlayDefendingGoalkeeperQuery::Query(BeforeState, Defender);
	if (!Result.GoalkeeperQueryResult.bSuccess)
	{
		Fail(Result, EError::DefendingGoalkeeperUnavailable,
			Result.GoalkeeperQueryResult.ErrorMessage);
		return Result;
	}
	if (!ObtainRoll(Result, RollProvider,
		EMatchPlayCurrentAttackPostRouteRollPurpose::CornerDefense))
	{
		return Result;
	}

	FMatchPlayState Candidate = BeforeState;
	FMatchPlayCornerRouteState& Corner =
		Candidate.CurrentAttack.SetPieceRoute.Corner;
	Corner.bHasDefenseD6 = true;
	Corner.DefenseD6 = Result.ProviderResult.RawD6;
	Result.FormulaInput = BuildFormulaInput(Corner,
		Result.GoalkeeperQueryResult, Attacker, Defender,
		Request.AttackSequence);
	Result.FormulaResolution =
		UFormulaResolver::ResolveFormula(Result.FormulaInput);
	if (!ValidateFormulaResult(Result.FormulaResolution))
	{
		Fail(Result, EError::FormulaResolutionFailed,
			TEXT("Corner FormulaResolver returned an unusable result."));
		return Result;
	}
	Corner.bHasFormulaResolution = true;
	Corner.FormulaResolution = Result.FormulaResolution;
	const bool bGoal = Corner.FormulaResolution.bIsGoal;
	PersistTerminal(Result, BeforeState, MoveTemp(Candidate), Attacker,
		bGoal
			? EMatchPlayCornerGameplayOutcome::Goal
			: EMatchPlayCornerGameplayOutcome::NoGoal);
	return Result;
}

bool FMatchPlayCornerResolution::PersistTerminal(
	FMatchPlayCornerResolutionResult& Result,
	const FMatchPlayState& BeforeState,
	FMatchPlayState WorkingState,
	const EInitialTurnOrderPlayer Attacker,
	const EMatchPlayCornerGameplayOutcome Outcome)
{
	using namespace MatchPlayCornerResolution;
	FMatchPlayCornerRouteState& Corner =
		WorkingState.CurrentAttack.SetPieceRoute.Corner;
	Corner.Stage = EMatchPlaySetPieceCornerRouteStage::Terminal;
	Corner.GameplayOutcome = Outcome;
	const bool bFormulaGoal = Outcome == EMatchPlayCornerGameplayOutcome::Goal;
	const bool bSystemGoal =
		Outcome == EMatchPlayCornerGameplayOutcome::SystemGoal;
	Corner.bHasGoalScorer = bFormulaGoal;
	Corner.GoalScorerCardId = bFormulaGoal ? Corner.Runner.CardId : NAME_None;

	FMatchPlayCurrentAttackCompletionResult Completion;
	Completion.BeforeState = BeforeState;
	Completion.AfterState = BeforeState;
	if (bFormulaGoal || bSystemGoal)
	{
		Completion.ScoringSide = Attacker;
		Completion.GoalResolveResult =
			FGoalResolver::RecordGoal(WorkingState.RuntimeState, Attacker);
		if (!Completion.GoalResolveResult.bSuccess)
		{
			Fail(Result, EError::GoalResolutionFailed,
				Completion.GoalResolveResult.ErrorMessage);
			return false;
		}
		WorkingState.RuntimeState =
			Completion.GoalResolveResult.UpdatedRuntimeState;
	}

	Result.CompletionResult =
		FMatchPlayCurrentAttackCompletion::PersistCurrentAttackTerminal(
			BeforeState, MoveTemp(WorkingState), Attacker,
			GetDefender(Attacker), MoveTemp(Completion));
	if (!Result.CompletionResult.bSuccess)
	{
		Fail(Result, EError::TerminalPersistenceFailed,
			Result.CompletionResult.ErrorMessage);
		return false;
	}
	return ValidateCandidate(Result,
		MoveTemp(Result.CompletionResult.AfterState));
}
