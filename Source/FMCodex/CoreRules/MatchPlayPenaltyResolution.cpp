#include "MatchPlayPenaltyResolution.h"

#include "GoalResolver.h"

namespace MatchPlayPenaltyResolution
{
	using EError = EMatchPlayPenaltyResolutionErrorCode;

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
		FMatchPlayPenaltyResolutionResult& Result,
		const EError ErrorCode,
		const FString& ErrorMessage)
	{
		Result.ErrorCode = ErrorCode;
		Result.ErrorMessage = ErrorMessage;
		Result.AfterState = Result.BeforeState;
	}

	bool ValidateBase(
		FMatchPlayPenaltyResolutionResult& Result,
		const FMatchPlayState& State,
		const int64 AttackSequence,
		const EInitialTurnOrderPlayer RequestingSide,
		const EMatchPlaySetPieceCarrierRouteStage RequiredStage,
		const bool bDefenderOwned,
		EInitialTurnOrderPlayer& OutAttacker,
		EInitialTurnOrderPlayer& OutDefender)
	{
		if (!State.RuntimeState.bIsInitialized)
		{
			Fail(Result, EError::MatchPlayStateNotInitialized,
				TEXT("Penalty resolution requires initialized match play state."));
			return false;
		}
		if (!State.bHasCurrentAttack)
		{
			Fail(Result, EError::NoCurrentAttack,
				TEXT("Penalty resolution requires a current attack."));
			return false;
		}
		if (AttackSequence <= 0 || State.CurrentAttack.AttackSequence <= 0)
		{
			Fail(Result, EError::InvalidAttackSequence,
				TEXT("Penalty AttackSequence must be positive."));
			return false;
		}
		if (AttackSequence != State.CurrentAttack.AttackSequence)
		{
			Fail(Result, EError::AttackSequenceMismatch,
				TEXT("Penalty request sequence does not match the current attack."));
			return false;
		}
		if (State.CurrentAttack.RouteKind
			!= EMatchPlayCurrentAttackRouteKind::SetPiece)
		{
			Fail(Result, EError::WrongRoute,
				TEXT("Penalty resolution requires the SetPiece route."));
			return false;
		}
		if (State.CurrentAttack.SetPieceRoute.SelectedType
			!= ESetPieceSelectedType::Penalty)
		{
			Fail(Result, EError::WrongSetPieceType,
				TEXT("Penalty resolution requires Penalty type."));
			return false;
		}
		if (State.CurrentAttack.SetPieceRoute.Penalty.Stage != RequiredStage)
		{
			Fail(Result, EError::WrongStage,
				TEXT("Penalty request is not legal in the current stage."));
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
					? TEXT("Only the current defender may request this Penalty roll.")
					: TEXT("Only the current attacker may submit this Penalty request."));
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

	bool ValidateProvider(
		FMatchPlayPenaltyResolutionResult& Result,
		const EMatchPlayCurrentAttackPostRouteRollPurpose Purpose)
	{
		Result.ProviderValidation =
			FMatchPlayPostRouteRollProviderResultValidator::Validate(
				Purpose, Result.ProviderResult);
		if (Result.ProviderValidation.bIsCanonical)
		{
			return true;
		}
		Fail(Result,
			Result.ProviderValidation.ErrorCode
				== EMatchPlayPostRouteRollProviderResultValidationErrorCode
					::ProviderFailure
				? EError::ProviderFailure
				: EError::MalformedProviderResult,
			Result.ProviderValidation.ErrorMessage);
		return false;
	}

	bool ValidateCandidate(
		FMatchPlayPenaltyResolutionResult& Result,
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
		return true;
	}
}

FMatchPlayPenaltyResolutionResult FMatchPlayPenaltyResolution::SubmitMethod(
	const FMatchPlayState& BeforeState,
	const FMatchPlayPenaltyMethodRequest& Request)
{
	using namespace MatchPlayPenaltyResolution;
	FMatchPlayPenaltyResolutionResult Result;
	Result.BeforeState = BeforeState;
	Result.AfterState = BeforeState;
	EInitialTurnOrderPlayer Attacker = EInitialTurnOrderPlayer::None;
	EInitialTurnOrderPlayer Defender = EInitialTurnOrderPlayer::None;
	if (!ValidateBase(Result, BeforeState, Request.AttackSequence,
		Request.RequestingSide,
		EMatchPlaySetPieceCarrierRouteStage::AwaitingMethod,
		false, Attacker, Defender))
	{
		return Result;
	}
	if (Request.Method != EMatchPlayPenaltyMethod::Direct
		&& Request.Method != EMatchPlayPenaltyMethod::Panenka)
	{
		Fail(Result, EError::InvalidMethod,
			TEXT("Penalty method must be Direct or Panenka."));
		return Result;
	}

	FMatchPlayState Candidate = BeforeState;
	FMatchPlayPenaltyRouteState& Penalty =
		Candidate.CurrentAttack.SetPieceRoute.Penalty;
	Penalty.Method = Request.Method;
	Penalty.Stage = Request.Method == EMatchPlayPenaltyMethod::Direct
		? EMatchPlaySetPieceCarrierRouteStage::DirectAwaitingAttackRoll
		: EMatchPlaySetPieceCarrierRouteStage::PanenkaAwaitingRoll;
	ValidateCandidate(Result, MoveTemp(Candidate));
	return Result;
}

FMatchPlayPenaltyResolutionResult
FMatchPlayPenaltyResolution::ResolveDirectAttackRoll(
	const FMatchPlayState& BeforeState,
	const FMatchPlayPenaltyRollRequest& Request,
	IMatchPlayPostRouteRollProvider* RollProvider)
{
	using namespace MatchPlayPenaltyResolution;
	FMatchPlayPenaltyResolutionResult Result;
	Result.BeforeState = BeforeState;
	Result.AfterState = BeforeState;
	EInitialTurnOrderPlayer Attacker = EInitialTurnOrderPlayer::None;
	EInitialTurnOrderPlayer Defender = EInitialTurnOrderPlayer::None;
	if (!ValidateBase(Result, BeforeState, Request.AttackSequence,
		Request.RequestingSide,
		EMatchPlaySetPieceCarrierRouteStage::DirectAwaitingAttackRoll,
		false, Attacker, Defender))
	{
		return Result;
	}
	if (BeforeState.CurrentAttack.SetPieceRoute.Penalty.Method
		!= EMatchPlayPenaltyMethod::Direct)
	{
		Fail(Result, EError::InvalidMethod,
			TEXT("Direct attack roll requires the Direct method."));
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
	if (RollProvider == nullptr)
	{
		Fail(Result, EError::ProviderUnavailable,
			TEXT("No authoritative Penalty roll provider is configured."));
		return Result;
	}
	const auto Purpose = EMatchPlayCurrentAttackPostRouteRollPurpose
		::PenaltyDirectAttack;
	Result.ProviderResult = RollProvider->RollD6(Purpose);
	if (!ValidateProvider(Result, Purpose))
	{
		return Result;
	}

	FMatchPlayState Candidate = BeforeState;
	FMatchPlayPenaltyRouteState& Penalty =
		Candidate.CurrentAttack.SetPieceRoute.Penalty;
	Penalty.bHasAttackD6 = true;
	Penalty.AttackD6 = Result.ProviderResult.RawD6;
	Penalty.Stage =
		EMatchPlaySetPieceCarrierRouteStage::DirectAwaitingDefenseRoll;
	ValidateCandidate(Result, MoveTemp(Candidate));
	return Result;
}

FMatchPlayPenaltyResolutionResult
FMatchPlayPenaltyResolution::ResolveDirectDefenseRoll(
	const FMatchPlayState& BeforeState,
	const FMatchPlayPenaltyRollRequest& Request,
	IMatchPlayPostRouteRollProvider* RollProvider)
{
	using namespace MatchPlayPenaltyResolution;
	FMatchPlayPenaltyResolutionResult Result;
	Result.BeforeState = BeforeState;
	Result.AfterState = BeforeState;
	EInitialTurnOrderPlayer Attacker = EInitialTurnOrderPlayer::None;
	EInitialTurnOrderPlayer Defender = EInitialTurnOrderPlayer::None;
	if (!ValidateBase(Result, BeforeState, Request.AttackSequence,
		Request.RequestingSide,
		EMatchPlaySetPieceCarrierRouteStage::DirectAwaitingDefenseRoll,
		true, Attacker, Defender))
	{
		return Result;
	}
	const FMatchPlayPenaltyRouteState& BeforePenalty =
		BeforeState.CurrentAttack.SetPieceRoute.Penalty;
	if (BeforePenalty.Method != EMatchPlayPenaltyMethod::Direct
		|| !BeforePenalty.bHasAttackD6)
	{
		Fail(Result, EError::InvalidMethod,
			TEXT("Direct defense roll requires Direct method and a stored attack D6."));
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
	if (RollProvider == nullptr)
	{
		Fail(Result, EError::ProviderUnavailable,
			TEXT("No authoritative Penalty roll provider is configured."));
		return Result;
	}
	const auto Purpose = EMatchPlayCurrentAttackPostRouteRollPurpose
		::PenaltyDirectDefense;
	Result.ProviderResult = RollProvider->RollD6(Purpose);
	if (!ValidateProvider(Result, Purpose))
	{
		return Result;
	}

	FMatchPlayState Candidate = BeforeState;
	FMatchPlayPenaltyRouteState& Penalty =
		Candidate.CurrentAttack.SetPieceRoute.Penalty;
	Penalty.bHasDefenseD6 = true;
	Penalty.DefenseD6 = Result.ProviderResult.RawD6;

	FFormulaResolverInput FormulaInput;
	FormulaInput.FormulaType = EFormulaType::Finishing;
	FormulaInput.Attacker.BaseValue = FMath::Max(
		Penalty.Carrier.Snapshot.Attributes.Shooting,
		Penalty.Carrier.Snapshot.Attributes.Passing);
	FormulaInput.Attacker.ComparePoint = Penalty.AttackD6;
	FormulaInput.Attacker.bComparePointWasRolledOnD6 = true;
	FormulaInput.Attacker.ParticipatingStamina.Add(
		Penalty.Carrier.Snapshot.Attributes.Stamina);
	FormulaInput.Defender.BaseValue = Result.GoalkeeperQueryResult.Snapshot
		.GoalkeeperAttributes.Anticipation;
	FormulaInput.Defender.Modifier = -3.0f;
	FormulaInput.Defender.ComparePoint = Penalty.DefenseD6;
	FormulaInput.Defender.bComparePointWasRolledOnD6 = true;
	FormulaInput.Defender.ParticipatingStamina.Add(
		Result.GoalkeeperQueryResult.Snapshot.Attributes.Stamina);
	FormulaInput.bGoalkeeperParticipated = true;
	FormulaInput.TurnIndex = static_cast<int32>(Request.AttackSequence);
	FormulaInput.AttackerPlayerId = GetSideId(Attacker);
	FormulaInput.DefenderPlayerId = GetSideId(Defender);
	FormulaInput.InvolvedCardIds = {
		Penalty.Carrier.CardId, Result.GoalkeeperQueryResult.CardId };
	Result.FormulaExecutionResult =
		FSingleCardFormulaResolutionExecutor::Execute(FormulaInput);
	if (!Result.FormulaExecutionResult.bSuccess)
	{
		Fail(Result, EError::FormulaResolutionFailed,
			Result.FormulaExecutionResult.ErrorMessage);
		return Result;
	}
	Penalty.bHasFormulaResolution = true;
	Penalty.FormulaResolution =
		Result.FormulaExecutionResult.FormulaResolutionResult;
	const bool bGoal = Penalty.FormulaResolution.bIsGoal;
	PersistTerminal(Result, BeforeState, MoveTemp(Candidate), Attacker, bGoal);
	return Result;
}

FMatchPlayPenaltyResolutionResult FMatchPlayPenaltyResolution::ResolvePanenkaRoll(
	const FMatchPlayState& BeforeState,
	const FMatchPlayPenaltyRollRequest& Request,
	IMatchPlayPostRouteRollProvider* RollProvider)
{
	using namespace MatchPlayPenaltyResolution;
	FMatchPlayPenaltyResolutionResult Result;
	Result.BeforeState = BeforeState;
	Result.AfterState = BeforeState;
	EInitialTurnOrderPlayer Attacker = EInitialTurnOrderPlayer::None;
	EInitialTurnOrderPlayer Defender = EInitialTurnOrderPlayer::None;
	if (!ValidateBase(Result, BeforeState, Request.AttackSequence,
		Request.RequestingSide,
		EMatchPlaySetPieceCarrierRouteStage::PanenkaAwaitingRoll,
		false, Attacker, Defender))
	{
		return Result;
	}
	if (BeforeState.CurrentAttack.SetPieceRoute.Penalty.Method
		!= EMatchPlayPenaltyMethod::Panenka)
	{
		Fail(Result, EError::InvalidMethod,
			TEXT("Panenka roll requires the Panenka method."));
		return Result;
	}
	if (RollProvider == nullptr)
	{
		Fail(Result, EError::ProviderUnavailable,
			TEXT("No authoritative Penalty roll provider is configured."));
		return Result;
	}
	const auto Purpose = EMatchPlayCurrentAttackPostRouteRollPurpose
		::PenaltyPanenka;
	Result.ProviderResult = RollProvider->RollD6(Purpose);
	if (!ValidateProvider(Result, Purpose))
	{
		return Result;
	}

	FMatchPlayState Candidate = BeforeState;
	FMatchPlayPenaltyRouteState& Penalty =
		Candidate.CurrentAttack.SetPieceRoute.Penalty;
	Penalty.bHasPanenkaD6 = true;
	Penalty.PanenkaD6 = Result.ProviderResult.RawD6;
	const bool bGoal = Penalty.PanenkaD6 >= 2;
	PersistTerminal(Result, BeforeState, MoveTemp(Candidate), Attacker, bGoal);
	return Result;
}

FMatchPlayPenaltyResolutionResult
FMatchPlayPenaltyResolution::ResolveNoLegalCarrier(
	const FMatchPlayState& BeforeState,
	const FMatchPlayPenaltyNoLegalCarrierRequest& Request)
{
	using namespace MatchPlayPenaltyResolution;
	FMatchPlayPenaltyResolutionResult Result;
	Result.BeforeState = BeforeState;
	Result.AfterState = BeforeState;
	EInitialTurnOrderPlayer Attacker = EInitialTurnOrderPlayer::None;
	EInitialTurnOrderPlayer Defender = EInitialTurnOrderPlayer::None;
	if (!ValidateBase(Result, BeforeState, Request.AttackSequence,
		Request.RequestingSide,
		EMatchPlaySetPieceCarrierRouteStage::AwaitingCarrier,
		false, Attacker, Defender))
	{
		return Result;
	}
	FMatchPlaySetPieceCarrierAvailabilityRequest AvailabilityRequest;
	AvailabilityRequest.AttackSequence = Request.AttackSequence;
	Result.CarrierAvailabilityResult =
		FMatchPlaySetPieceCarrierAvailability::Query(
			BeforeState, AvailabilityRequest);
	if (!Result.CarrierAvailabilityResult.bSuccess)
	{
		Fail(Result, EError::CarrierAvailabilityFailed,
			Result.CarrierAvailabilityResult.ErrorMessage);
		return Result;
	}
	if (Result.CarrierAvailabilityResult.bHasLegalCarrier)
	{
		Fail(Result, EError::LegalCarrierExists,
			TEXT("No-legal-Carrier resolution is invalid while an authoritative legal Carrier exists."));
		return Result;
	}
	FMatchPlayState Candidate = BeforeState;
	Candidate.CurrentAttack.SetPieceRoute.Penalty.bNoLegalCarrier = true;
	PersistTerminal(Result, BeforeState, MoveTemp(Candidate), Attacker, false);
	return Result;
}

bool FMatchPlayPenaltyResolution::PersistTerminal(
	FMatchPlayPenaltyResolutionResult& Result,
	const FMatchPlayState& BeforeState,
	FMatchPlayState WorkingState,
	const EInitialTurnOrderPlayer Attacker,
	const bool bGoal)
{
	using namespace MatchPlayPenaltyResolution;
	FMatchPlayPenaltyRouteState& Penalty =
		WorkingState.CurrentAttack.SetPieceRoute.Penalty;
	Penalty.Stage = EMatchPlaySetPieceCarrierRouteStage::Terminal;
	Penalty.GameplayOutcome = bGoal
		? EMatchPlayPenaltyGameplayOutcome::Goal
		: EMatchPlayPenaltyGameplayOutcome::NoGoal;
	Penalty.bHasGoalScorer = bGoal;
	Penalty.GoalScorerCardId = bGoal ? Penalty.Carrier.CardId : NAME_None;

	FMatchPlayCurrentAttackCompletionResult Completion;
	Completion.BeforeState = BeforeState;
	Completion.AfterState = BeforeState;
	if (bGoal)
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
	return ValidateCandidate(Result, MoveTemp(Result.CompletionResult.AfterState));
}
