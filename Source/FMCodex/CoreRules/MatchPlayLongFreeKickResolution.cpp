#include "MatchPlayLongFreeKickResolution.h"

#include "GoalResolver.h"

namespace MatchPlayLongFreeKickResolution
{
	using EError = EMatchPlayLongFreeKickResolutionErrorCode;

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
		FMatchPlayLongFreeKickResolutionResult& Result,
		const EError ErrorCode,
		const FString& ErrorMessage)
	{
		Result.ErrorCode = ErrorCode;
		Result.ErrorMessage = ErrorMessage;
		Result.AfterState = Result.BeforeState;
	}

	bool ValidateBase(
		FMatchPlayLongFreeKickResolutionResult& Result,
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
				TEXT("Long Free Kick resolution requires initialized match play state."));
			return false;
		}
		if (!State.bHasCurrentAttack)
		{
			Fail(Result, EError::NoCurrentAttack,
				TEXT("Long Free Kick resolution requires a current attack."));
			return false;
		}
		if (AttackSequence <= 0 || State.CurrentAttack.AttackSequence <= 0)
		{
			Fail(Result, EError::InvalidAttackSequence,
				TEXT("Long Free Kick AttackSequence must be positive."));
			return false;
		}
		if (AttackSequence != State.CurrentAttack.AttackSequence)
		{
			Fail(Result, EError::AttackSequenceMismatch,
				TEXT("Long Free Kick request sequence does not match the current attack."));
			return false;
		}
		if (State.CurrentAttack.RouteKind
			!= EMatchPlayCurrentAttackRouteKind::SetPiece)
		{
			Fail(Result, EError::WrongRoute,
				TEXT("Long Free Kick resolution requires the SetPiece route."));
			return false;
		}
		if (State.CurrentAttack.SetPieceRoute.SelectedType
			!= ESetPieceSelectedType::LongFreeKick)
		{
			Fail(Result, EError::WrongSetPieceType,
				TEXT("Long Free Kick resolution requires LongFreeKick type."));
			return false;
		}
		if (State.CurrentAttack.SetPieceRoute.LongFreeKick.Stage
			!= RequiredStage)
		{
			Fail(Result, EError::WrongStage,
				TEXT("Long Free Kick request is not legal in the current stage."));
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
					? TEXT("Only the current defender may request this Long Free Kick roll.")
					: TEXT("Only the current attacker may submit this Long Free Kick request."));
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
		FMatchPlayLongFreeKickResolutionResult& Result,
		const EMatchPlayCurrentAttackPostRouteRollPurpose Purpose,
		const FMatchPlayPostRouteRollProviderResult& ProviderResult,
		FMatchPlayPostRouteRollProviderResultValidationResult& Validation)
	{
		Validation = FMatchPlayPostRouteRollProviderResultValidator::Validate(
			Purpose, ProviderResult);
		if (Validation.bIsCanonical)
		{
			return true;
		}
		Fail(Result,
			Validation.ErrorCode
				== EMatchPlayPostRouteRollProviderResultValidationErrorCode
					::ProviderFailure
				? EError::ProviderFailure
				: EError::MalformedProviderResult,
			Validation.ErrorMessage);
		return false;
	}

	bool ValidateCandidate(
		FMatchPlayLongFreeKickResolutionResult& Result,
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

FMatchPlayLongFreeKickResolutionResult
FMatchPlayLongFreeKickResolution::SubmitMethod(
	const FMatchPlayState& BeforeState,
	const FMatchPlayLongFreeKickMethodRequest& Request)
{
	using namespace MatchPlayLongFreeKickResolution;
	FMatchPlayLongFreeKickResolutionResult Result;
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
	if (Request.Method != EMatchPlayLongFreeKickMethod::Direct
		&& Request.Method != EMatchPlayLongFreeKickMethod::Power)
	{
		Fail(Result, EError::InvalidMethod,
			TEXT("Long Free Kick method must be Direct or Power."));
		return Result;
	}

	FMatchPlayState Candidate = BeforeState;
	FMatchPlayLongFreeKickRouteState& Long =
		Candidate.CurrentAttack.SetPieceRoute.LongFreeKick;
	Long.Method = Request.Method;
	Long.Stage = Request.Method == EMatchPlayLongFreeKickMethod::Direct
		? EMatchPlaySetPieceCarrierRouteStage::DirectAwaitingAttackRoll
		: EMatchPlaySetPieceCarrierRouteStage::PowerAwaitingRoll;
	ValidateCandidate(Result, MoveTemp(Candidate));
	return Result;
}

FMatchPlayLongFreeKickResolutionResult
FMatchPlayLongFreeKickResolution::ResolveDirectAttackRoll(
	const FMatchPlayState& BeforeState,
	const FMatchPlayLongFreeKickRollRequest& Request,
	IMatchPlayPostRouteRollProvider* RollProvider)
{
	using namespace MatchPlayLongFreeKickResolution;
	FMatchPlayLongFreeKickResolutionResult Result;
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
	if (BeforeState.CurrentAttack.SetPieceRoute.LongFreeKick.Method
		!= EMatchPlayLongFreeKickMethod::Direct)
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
			TEXT("No authoritative Long Free Kick roll provider is configured."));
		return Result;
	}
	const auto Purpose = EMatchPlayCurrentAttackPostRouteRollPurpose
		::LongFreeKickDirectAttack;
	Result.FirstProviderResult = RollProvider->RollD6(Purpose);
	if (!ValidateProvider(Result, Purpose, Result.FirstProviderResult,
		Result.FirstProviderValidation))
	{
		return Result;
	}

	FMatchPlayState Candidate = BeforeState;
	FMatchPlayLongFreeKickRouteState& Long =
		Candidate.CurrentAttack.SetPieceRoute.LongFreeKick;
	Long.bHasAttackD6 = true;
	Long.AttackD6 = Result.FirstProviderResult.RawD6;
	if (Long.AttackD6 <= 2)
	{
		PersistTerminal(Result, BeforeState, MoveTemp(Candidate), Attacker, false);
		return Result;
	}
	Long.Stage = EMatchPlaySetPieceCarrierRouteStage
		::DirectAwaitingDefenseRoll;
	ValidateCandidate(Result, MoveTemp(Candidate));
	return Result;
}

FMatchPlayLongFreeKickResolutionResult
FMatchPlayLongFreeKickResolution::ResolveDirectDefenseRoll(
	const FMatchPlayState& BeforeState,
	const FMatchPlayLongFreeKickRollRequest& Request,
	IMatchPlayPostRouteRollProvider* RollProvider)
{
	using namespace MatchPlayLongFreeKickResolution;
	FMatchPlayLongFreeKickResolutionResult Result;
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
	const FMatchPlayLongFreeKickRouteState& BeforeLong =
		BeforeState.CurrentAttack.SetPieceRoute.LongFreeKick;
	if (BeforeLong.Method != EMatchPlayLongFreeKickMethod::Direct
		|| !BeforeLong.bHasAttackD6 || BeforeLong.AttackD6 < 3)
	{
		Fail(Result, EError::InvalidMethod,
			TEXT("Direct defense roll requires Direct method and a stored attack D6 in [3, 6]."));
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
			TEXT("No authoritative Long Free Kick roll provider is configured."));
		return Result;
	}
	const auto Purpose = EMatchPlayCurrentAttackPostRouteRollPurpose
		::LongFreeKickDirectDefense;
	Result.FirstProviderResult = RollProvider->RollD6(Purpose);
	if (!ValidateProvider(Result, Purpose, Result.FirstProviderResult,
		Result.FirstProviderValidation))
	{
		return Result;
	}

	FMatchPlayState Candidate = BeforeState;
	FMatchPlayLongFreeKickRouteState& Long =
		Candidate.CurrentAttack.SetPieceRoute.LongFreeKick;
	Long.bHasDefenseD6 = true;
	Long.DefenseD6 = Result.FirstProviderResult.RawD6;

	FFormulaResolverInput FormulaInput;
	FormulaInput.FormulaType = EFormulaType::Finishing;
	FormulaInput.Attacker.BaseValue = Long.Carrier.Snapshot.Attributes.LongShot;
	FormulaInput.Attacker.ComparePoint = Long.AttackD6;
	FormulaInput.Attacker.bComparePointWasRolledOnD6 = true;
	FormulaInput.Attacker.ParticipatingStamina.Add(
		Long.Carrier.Snapshot.Attributes.Stamina);
	FormulaInput.Defender.BaseValue = Result.GoalkeeperQueryResult
		.Snapshot.GoalkeeperAttributes.Positioning;
	FormulaInput.Defender.Modifier = 2.0f;
	FormulaInput.Defender.ComparePoint = Long.DefenseD6;
	FormulaInput.Defender.bComparePointWasRolledOnD6 = true;
	FormulaInput.Defender.ParticipatingStamina.Add(
		Result.GoalkeeperQueryResult.Snapshot.Attributes.Stamina);
	FormulaInput.bGoalkeeperParticipated = true;
	FormulaInput.TurnIndex = static_cast<int32>(Request.AttackSequence);
	FormulaInput.AttackerPlayerId = GetSideId(Attacker);
	FormulaInput.DefenderPlayerId = GetSideId(Defender);
	FormulaInput.InvolvedCardIds = {
		Long.Carrier.CardId, Result.GoalkeeperQueryResult.CardId };
	Result.FormulaExecutionResult =
		FSingleCardFormulaResolutionExecutor::Execute(FormulaInput);
	if (!Result.FormulaExecutionResult.bSuccess)
	{
		Fail(Result, EError::FormulaResolutionFailed,
			Result.FormulaExecutionResult.ErrorMessage);
		return Result;
	}
	Long.bHasFormulaResolution = true;
	Long.FormulaResolution =
		Result.FormulaExecutionResult.FormulaResolutionResult;
	const bool bGoal = Long.FormulaResolution.bIsGoal;
	PersistTerminal(Result, BeforeState, MoveTemp(Candidate), Attacker,
		bGoal);
	return Result;
}

FMatchPlayLongFreeKickResolutionResult
FMatchPlayLongFreeKickResolution::ResolvePowerRoll(
	const FMatchPlayState& BeforeState,
	const FMatchPlayLongFreeKickRollRequest& Request,
	IMatchPlayPostRouteRollProvider* RollProvider)
{
	using namespace MatchPlayLongFreeKickResolution;
	FMatchPlayLongFreeKickResolutionResult Result;
	Result.BeforeState = BeforeState;
	Result.AfterState = BeforeState;
	EInitialTurnOrderPlayer Attacker = EInitialTurnOrderPlayer::None;
	EInitialTurnOrderPlayer Defender = EInitialTurnOrderPlayer::None;
	if (!ValidateBase(Result, BeforeState, Request.AttackSequence,
		Request.RequestingSide,
		EMatchPlaySetPieceCarrierRouteStage::PowerAwaitingRoll,
		false, Attacker, Defender))
	{
		return Result;
	}
	if (BeforeState.CurrentAttack.SetPieceRoute.LongFreeKick.Method
		!= EMatchPlayLongFreeKickMethod::Power)
	{
		Fail(Result, EError::InvalidMethod,
			TEXT("Power roll requires the Power method."));
		return Result;
	}
	if (RollProvider == nullptr)
	{
		Fail(Result, EError::ProviderUnavailable,
			TEXT("No authoritative Long Free Kick roll provider is configured."));
		return Result;
	}

	const auto PurposeA = EMatchPlayCurrentAttackPostRouteRollPurpose
		::LongFreeKickPowerA;
	const auto PurposeB = EMatchPlayCurrentAttackPostRouteRollPurpose
		::LongFreeKickPowerB;
	Result.FirstProviderResult = RollProvider->RollD6(PurposeA);
	if (!ValidateProvider(Result, PurposeA, Result.FirstProviderResult,
		Result.FirstProviderValidation))
	{
		return Result;
	}
	Result.SecondProviderResult = RollProvider->RollD6(PurposeB);
	if (!ValidateProvider(Result, PurposeB, Result.SecondProviderResult,
		Result.SecondProviderValidation))
	{
		return Result;
	}

	FMatchPlayState Candidate = BeforeState;
	FMatchPlayLongFreeKickRouteState& Long =
		Candidate.CurrentAttack.SetPieceRoute.LongFreeKick;
	Long.bHasPowerD6Pair = true;
	Long.PowerD6A = Result.FirstProviderResult.RawD6;
	Long.PowerD6B = Result.SecondProviderResult.RawD6;
	const bool bGoal = Long.PowerD6A + Long.PowerD6B >= 11;
	PersistTerminal(Result, BeforeState, MoveTemp(Candidate), Attacker,
		bGoal);
	return Result;
}

FMatchPlayLongFreeKickResolutionResult
FMatchPlayLongFreeKickResolution::ResolveNoLegalCarrier(
	const FMatchPlayState& BeforeState,
	const FMatchPlayLongFreeKickNoLegalCarrierRequest& Request)
{
	using namespace MatchPlayLongFreeKickResolution;
	FMatchPlayLongFreeKickResolutionResult Result;
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
	Candidate.CurrentAttack.SetPieceRoute.LongFreeKick.bNoLegalCarrier = true;
	PersistTerminal(Result, BeforeState, MoveTemp(Candidate), Attacker, false);
	return Result;
}

bool FMatchPlayLongFreeKickResolution::PersistTerminal(
	FMatchPlayLongFreeKickResolutionResult& Result,
	const FMatchPlayState& BeforeState,
	FMatchPlayState WorkingState,
	const EInitialTurnOrderPlayer Attacker,
	const bool bGoal)
{
	using namespace MatchPlayLongFreeKickResolution;
	FMatchPlayLongFreeKickRouteState& Long =
		WorkingState.CurrentAttack.SetPieceRoute.LongFreeKick;
	Long.Stage = EMatchPlaySetPieceCarrierRouteStage::Terminal;
	Long.GameplayOutcome = bGoal
		? EMatchPlayLongFreeKickGameplayOutcome::Goal
		: EMatchPlayLongFreeKickGameplayOutcome::NoGoal;
	Long.bHasGoalScorer = bGoal;
	Long.GoalScorerCardId = bGoal ? Long.Carrier.CardId : NAME_None;

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
	return ValidateCandidate(Result,
		MoveTemp(Result.CompletionResult.AfterState));
}
