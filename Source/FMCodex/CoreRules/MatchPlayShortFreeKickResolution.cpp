#include "MatchPlayShortFreeKickResolution.h"

#include "GoalResolver.h"

namespace MatchPlayShortFreeKickResolution
{
	using EError = EMatchPlayShortFreeKickResolutionErrorCode;

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
		FMatchPlayShortFreeKickResolutionResult& Result,
		const EError ErrorCode,
		const FString& ErrorMessage)
	{
		Result.ErrorCode = ErrorCode;
		Result.ErrorMessage = ErrorMessage;
		Result.AfterState = Result.BeforeState;
	}

	bool ValidateBase(
		FMatchPlayShortFreeKickResolutionResult& Result,
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
				TEXT("Short Free Kick resolution requires initialized match play state."));
			return false;
		}
		if (!State.bHasCurrentAttack)
		{
			Fail(Result, EError::NoCurrentAttack,
				TEXT("Short Free Kick resolution requires a current attack."));
			return false;
		}
		if (AttackSequence <= 0
			|| State.CurrentAttack.AttackSequence <= 0)
		{
			Fail(Result, EError::InvalidAttackSequence,
				TEXT("Short Free Kick AttackSequence must be positive."));
			return false;
		}
		if (AttackSequence != State.CurrentAttack.AttackSequence)
		{
			Fail(Result, EError::AttackSequenceMismatch,
				TEXT("Short Free Kick request sequence does not match the current attack."));
			return false;
		}
		if (State.CurrentAttack.RouteKind
			!= EMatchPlayCurrentAttackRouteKind::SetPiece)
		{
			Fail(Result, EError::WrongRoute,
				TEXT("Short Free Kick resolution requires the SetPiece route."));
			return false;
		}
		if (State.CurrentAttack.SetPieceRoute.SelectedType
			!= ESetPieceSelectedType::ShortFreeKick)
		{
			Fail(Result, EError::WrongSetPieceType,
				TEXT("Short Free Kick resolution requires ShortFreeKick type."));
			return false;
		}
		if (State.CurrentAttack.SetPieceRoute.ShortFreeKick.Stage
			!= RequiredStage)
		{
			Fail(Result, EError::WrongStage,
				TEXT("Short Free Kick request is not legal in the current stage."));
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
					? TEXT("Only the current defender may request this Short Free Kick roll.")
					: TEXT("Only the current attacker may submit this Short Free Kick request."));
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
		FMatchPlayShortFreeKickResolutionResult& Result,
		const EMatchPlayCurrentAttackPostRouteRollPurpose Purpose,
		const FMatchPlayPostRouteRollProviderResult& ProviderResult,
		FMatchPlayPostRouteRollProviderResultValidationResult& Validation)
	{
		Validation = FMatchPlayPostRouteRollProviderResultValidator::Validate(
			Purpose,
			ProviderResult);
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
		FMatchPlayShortFreeKickResolutionResult& Result,
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

FMatchPlayShortFreeKickResolutionResult
FMatchPlayShortFreeKickResolution::SubmitMethod(
	const FMatchPlayState& BeforeState,
	const FMatchPlayShortFreeKickMethodRequest& Request)
{
	using namespace MatchPlayShortFreeKickResolution;
	FMatchPlayShortFreeKickResolutionResult Result;
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
	if (Request.Method != EMatchPlayShortFreeKickMethod::Direct
		&& Request.Method != EMatchPlayShortFreeKickMethod::Angled)
	{
		Fail(Result, EError::InvalidMethod,
			TEXT("Short Free Kick method must be Direct or Angled."));
		return Result;
	}

	FMatchPlayState Candidate = BeforeState;
	FMatchPlayShortFreeKickRouteState& Short =
		Candidate.CurrentAttack.SetPieceRoute.ShortFreeKick;
	if (Request.Method == EMatchPlayShortFreeKickMethod::Angled
		&& Short.Carrier.Snapshot.Attributes.Shooting
			+ Short.Carrier.Snapshot.Attributes.Passing < 8)
	{
		Fail(Result, EError::AngledMethodNotEligible,
			TEXT("Angled Short Free Kick requires frozen Shooting plus Passing of at least 8."));
		return Result;
	}
	Short.Method = Request.Method;
	Short.Stage = Request.Method == EMatchPlayShortFreeKickMethod::Direct
		? EMatchPlaySetPieceCarrierRouteStage::DirectAwaitingAttackRoll
		: EMatchPlaySetPieceCarrierRouteStage::AngledAwaitingRoll;
	ValidateCandidate(Result, MoveTemp(Candidate));
	return Result;
}

FMatchPlayShortFreeKickResolutionResult
FMatchPlayShortFreeKickResolution::ResolveDirectAttackRoll(
	const FMatchPlayState& BeforeState,
	const FMatchPlayShortFreeKickRollRequest& Request,
	IMatchPlayPostRouteRollProvider* RollProvider)
{
	using namespace MatchPlayShortFreeKickResolution;
	FMatchPlayShortFreeKickResolutionResult Result;
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
	const FMatchPlayShortFreeKickRouteState& BeforeShort =
		BeforeState.CurrentAttack.SetPieceRoute.ShortFreeKick;
	if (BeforeShort.Method != EMatchPlayShortFreeKickMethod::Direct)
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
			TEXT("No authoritative Short Free Kick roll provider is configured."));
		return Result;
	}
	const auto Purpose = EMatchPlayCurrentAttackPostRouteRollPurpose
		::ShortFreeKickDirectAttack;
	Result.FirstProviderResult = RollProvider->RollD6(Purpose);
	if (!ValidateProvider(Result, Purpose, Result.FirstProviderResult,
		Result.FirstProviderValidation))
	{
		return Result;
	}

	FMatchPlayState Candidate = BeforeState;
	FMatchPlayShortFreeKickRouteState& Short =
		Candidate.CurrentAttack.SetPieceRoute.ShortFreeKick;
	Short.bHasAttackD6 = true;
	Short.AttackD6 = Result.FirstProviderResult.RawD6;
	Short.Stage =
		EMatchPlaySetPieceCarrierRouteStage::DirectAwaitingDefenseRoll;
	ValidateCandidate(Result, MoveTemp(Candidate));
	return Result;
}

FMatchPlayShortFreeKickResolutionResult
FMatchPlayShortFreeKickResolution::ResolveDirectDefenseRoll(
	const FMatchPlayState& BeforeState,
	const FMatchPlayShortFreeKickRollRequest& Request,
	IMatchPlayPostRouteRollProvider* RollProvider)
{
	using namespace MatchPlayShortFreeKickResolution;
	FMatchPlayShortFreeKickResolutionResult Result;
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
	const FMatchPlayShortFreeKickRouteState& BeforeShort =
		BeforeState.CurrentAttack.SetPieceRoute.ShortFreeKick;
	if (BeforeShort.Method != EMatchPlayShortFreeKickMethod::Direct
		|| !BeforeShort.bHasAttackD6)
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
			TEXT("No authoritative Short Free Kick roll provider is configured."));
		return Result;
	}
	const auto Purpose = EMatchPlayCurrentAttackPostRouteRollPurpose
		::ShortFreeKickDirectDefense;
	Result.FirstProviderResult = RollProvider->RollD6(Purpose);
	if (!ValidateProvider(Result, Purpose, Result.FirstProviderResult,
		Result.FirstProviderValidation))
	{
		return Result;
	}

	FMatchPlayState Candidate = BeforeState;
	FMatchPlayShortFreeKickRouteState& Short =
		Candidate.CurrentAttack.SetPieceRoute.ShortFreeKick;
	Short.bHasDefenseD6 = true;
	Short.DefenseD6 = Result.FirstProviderResult.RawD6;

	FFormulaResolverInput FormulaInput;
	FormulaInput.FormulaType = EFormulaType::Finishing;
	FormulaInput.Attacker.BaseValue = FMath::Max(
		Short.Carrier.Snapshot.Attributes.Shooting,
		Short.Carrier.Snapshot.Attributes.Passing);
	FormulaInput.Attacker.ComparePoint = Short.AttackD6;
	FormulaInput.Attacker.bComparePointWasRolledOnD6 = true;
	FormulaInput.Attacker.ParticipatingStamina.Add(
		Short.Carrier.Snapshot.Attributes.Stamina);
	FormulaInput.Defender.BaseValue =
		Result.GoalkeeperQueryResult.Snapshot.GoalkeeperAttributes.Handling;
	FormulaInput.Defender.Modifier = 1.0f;
	FormulaInput.Defender.ComparePoint = Short.DefenseD6;
	FormulaInput.Defender.bComparePointWasRolledOnD6 = true;
	FormulaInput.Defender.ParticipatingStamina.Add(
		Result.GoalkeeperQueryResult.Snapshot.Attributes.Stamina);
	FormulaInput.bGoalkeeperParticipated = true;
	FormulaInput.TurnIndex = static_cast<int32>(Request.AttackSequence);
	FormulaInput.AttackerPlayerId = GetSideId(Attacker);
	FormulaInput.DefenderPlayerId = GetSideId(Defender);
	FormulaInput.InvolvedCardIds = {
		Short.Carrier.CardId,
		Result.GoalkeeperQueryResult.CardId };
	Result.FormulaExecutionResult =
		FSingleCardFormulaResolutionExecutor::Execute(FormulaInput);
	if (!Result.FormulaExecutionResult.bSuccess)
	{
		Fail(Result, EError::FormulaResolutionFailed,
			Result.FormulaExecutionResult.ErrorMessage);
		return Result;
	}
	Short.bHasFormulaResolution = true;
	Short.FormulaResolution =
		Result.FormulaExecutionResult.FormulaResolutionResult;
	const bool bGoal = Short.FormulaResolution.bIsGoal;
	PersistTerminal(Result, BeforeState, MoveTemp(Candidate), Attacker, bGoal);
	return Result;
}

FMatchPlayShortFreeKickResolutionResult
FMatchPlayShortFreeKickResolution::ResolveAngledRoll(
	const FMatchPlayState& BeforeState,
	const FMatchPlayShortFreeKickRollRequest& Request,
	IMatchPlayPostRouteRollProvider* RollProvider)
{
	using namespace MatchPlayShortFreeKickResolution;
	FMatchPlayShortFreeKickResolutionResult Result;
	Result.BeforeState = BeforeState;
	Result.AfterState = BeforeState;
	EInitialTurnOrderPlayer Attacker = EInitialTurnOrderPlayer::None;
	EInitialTurnOrderPlayer Defender = EInitialTurnOrderPlayer::None;
	if (!ValidateBase(Result, BeforeState, Request.AttackSequence,
		Request.RequestingSide,
		EMatchPlaySetPieceCarrierRouteStage::AngledAwaitingRoll,
		false, Attacker, Defender))
	{
		return Result;
	}
	const FMatchPlayShortFreeKickRouteState& BeforeShort =
		BeforeState.CurrentAttack.SetPieceRoute.ShortFreeKick;
	if (BeforeShort.Method != EMatchPlayShortFreeKickMethod::Angled
		|| BeforeShort.Carrier.Snapshot.Attributes.Shooting
			+ BeforeShort.Carrier.Snapshot.Attributes.Passing < 8)
	{
		Fail(Result, EError::AngledMethodNotEligible,
			TEXT("Angled roll requires an eligible Angled Short Free Kick."));
		return Result;
	}
	if (RollProvider == nullptr)
	{
		Fail(Result, EError::ProviderUnavailable,
			TEXT("No authoritative Short Free Kick roll provider is configured."));
		return Result;
	}
	const auto PurposeA = EMatchPlayCurrentAttackPostRouteRollPurpose
		::ShortFreeKickAngledA;
	const auto PurposeB = EMatchPlayCurrentAttackPostRouteRollPurpose
		::ShortFreeKickAngledB;
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
	FMatchPlayShortFreeKickRouteState& Short =
		Candidate.CurrentAttack.SetPieceRoute.ShortFreeKick;
	Short.bHasAngledD6Pair = true;
	Short.AngledD6A = Result.FirstProviderResult.RawD6;
	Short.AngledD6B = Result.SecondProviderResult.RawD6;
	const bool bGoal = Short.AngledD6A + Short.AngledD6B >= 9;
	PersistTerminal(Result, BeforeState, MoveTemp(Candidate), Attacker, bGoal);
	return Result;
}

FMatchPlayShortFreeKickResolutionResult
FMatchPlayShortFreeKickResolution::ResolveNoLegalCarrier(
	const FMatchPlayState& BeforeState,
	const FMatchPlayShortFreeKickNoLegalCarrierRequest& Request)
{
	using namespace MatchPlayShortFreeKickResolution;
	FMatchPlayShortFreeKickResolutionResult Result;
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
			BeforeState,
			AvailabilityRequest);
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
	Candidate.CurrentAttack.SetPieceRoute.ShortFreeKick.bNoLegalCarrier = true;
	PersistTerminal(Result, BeforeState, MoveTemp(Candidate), Attacker, false);
	return Result;
}

bool FMatchPlayShortFreeKickResolution::PersistTerminal(
	FMatchPlayShortFreeKickResolutionResult& Result,
	const FMatchPlayState& BeforeState,
	FMatchPlayState WorkingState,
	const EInitialTurnOrderPlayer Attacker,
	const bool bGoal)
{
	using namespace MatchPlayShortFreeKickResolution;
	FMatchPlayShortFreeKickRouteState& Short =
		WorkingState.CurrentAttack.SetPieceRoute.ShortFreeKick;
	Short.Stage = EMatchPlaySetPieceCarrierRouteStage::Terminal;
	Short.GameplayOutcome = bGoal
		? EMatchPlayShortFreeKickGameplayOutcome::Goal
		: EMatchPlayShortFreeKickGameplayOutcome::NoGoal;
	Short.bHasGoalScorer = bGoal;
	Short.GoalScorerCardId = bGoal ? Short.Carrier.CardId : NAME_None;

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
			BeforeState,
			MoveTemp(WorkingState),
			Attacker,
			GetDefender(Attacker),
			MoveTemp(Completion));
	if (!Result.CompletionResult.bSuccess)
	{
		Fail(Result, EError::TerminalPersistenceFailed,
			Result.CompletionResult.ErrorMessage);
		return false;
	}
	return ValidateCandidate(
		Result,
		MoveTemp(Result.CompletionResult.AfterState));
}
