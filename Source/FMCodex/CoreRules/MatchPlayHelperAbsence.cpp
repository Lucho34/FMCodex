#include "MatchPlayHelperAbsence.h"

#include "MatchPlayCurrentAttackHelperFinalization.h"
#include "MatchPlayHelperAbsenceCapability.h"

namespace MatchPlayHelperAbsenceImplementation
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
}

FMatchPlayHelperAbsenceCapability::FMatchPlayHelperAbsenceCapability(
	FResolveNoLegalHelperIssuerTag,
	const int64 InAttackSequence,
	const FMatchPlayCurrentAttackHelperSelectionAvailabilityResult&
		InAvailabilityResult)
	: AttackSequence(InAttackSequence)
	, Reason(EMatchPlayHelperAbsenceReason::NoLegalHelper)
	, Source(EMatchPlayHelperAbsenceSource::ResolveNoLegalHelper)
	, AvailabilityResult(InAvailabilityResult)
{
}

FMatchPlayHelperAbsenceCapability::FMatchPlayHelperAbsenceCapability(
	FHelperDeclineIssuerTag,
	const int64 InAttackSequence,
	const FMatchPlayCurrentAttackHelperSelectionAvailabilityResult&
		InAvailabilityResult)
	: AttackSequence(InAttackSequence)
	, Reason(EMatchPlayHelperAbsenceReason::HelperDeclined)
	, Source(EMatchPlayHelperAbsenceSource::HelperDecline)
	, AvailabilityResult(InAvailabilityResult)
{
}

class FMatchPlayHelperAbsenceFinalizer final
{
public:
	static FMatchPlayHelperAbsenceFinalizationResult
		FinalizeWithoutHelper(
			const FMatchPlayState& BeforeState,
			const FMatchPlayHelperAbsenceCapability& Capability)
	{
		using namespace MatchPlayHelperAbsenceImplementation;

		FMatchPlayHelperAbsenceFinalizationResult Result;
		Result.AfterState = BeforeState;
		const auto& Availability = Capability.GetAvailabilityResult();
		const auto& Global = Availability.GlobalContextResult;
		const EInitialTurnOrderPlayer Attacker =
			BeforeState.RuntimeState.CurrentAttackingPlayer;
		const EInitialTurnOrderPlayer Defender =
			GetDefender(Attacker);
		const FMatchPlayCurrentAttackActionPreparationState& Preparation =
			BeforeState.CurrentAttack.ActionPreparation;
		if (!BeforeState.bHasCurrentAttack
			|| !IsPlayerSide(Attacker)
			|| !IsPlayerSide(Defender)
			|| Capability.GetAttackSequence()
				!= BeforeState.CurrentAttack.AttackSequence
			|| BeforeState.CurrentAttack.SelectionStage
				!= EMatchPlayCurrentAttackSelectionStage::AwaitingHelper
			|| !Availability.bQuerySucceeded
			|| !Global.bSuccess
			|| Availability.AttackSequence
				!= Capability.GetAttackSequence()
			|| Global.AuthoritativeAttackSequence
				!= Capability.GetAttackSequence()
			|| Global.CurrentAttackingPlayer != Attacker
			|| Global.CurrentDefendingPlayer != Defender
			|| Global.RequestingSide != Defender
			|| Global.FrozenCarrierCardId
				!= Preparation.CarrierCardId
			|| Global.FrozenMarkerCardId
				!= Preparation.MarkerCardId
			|| Global.FrozenSkillId != Preparation.SkillId
			|| Global.FrozenActionType != Preparation.ActionType
			|| Global.FrozenRunnerCardId
				!= Preparation.RunnerCardId)
		{
			Result.ErrorCode =
				EMatchPlayHelperAbsenceErrorCode::InvalidCapability;
			Result.ErrorMessage =
				TEXT("Helper absence capability does not match AwaitingHelper state.");
			return Result;
		}

		const bool bNoLegal =
			Capability.GetSource()
				== EMatchPlayHelperAbsenceSource::ResolveNoLegalHelper
			&& Capability.GetReason()
				== EMatchPlayHelperAbsenceReason::NoLegalHelper
			&& !Availability.bCanSelectAnyHelper;
		const bool bDeclined =
			Capability.GetSource()
				== EMatchPlayHelperAbsenceSource::HelperDecline
			&& Capability.GetReason()
				== EMatchPlayHelperAbsenceReason::HelperDeclined
			&& Availability.bCanSelectAnyHelper;
		if (!bNoLegal && !bDeclined)
		{
			Result.ErrorCode =
				EMatchPlayHelperAbsenceErrorCode
					::InvalidCapabilitySourceReason;
			Result.ErrorMessage =
				TEXT("Helper absence source, reason, and availability are inconsistent.");
			return Result;
		}

		FMatchPlayState WorkingState = BeforeState;
		const FMatchPlayValidatedHelperPresence Presence{
			FMatchPlayValidatedHelperPresence::FAbsentHelperTag()};
		FMatchPlayCurrentAttackHelperFinalization
			::ApplyValidatedHelperCompletion(WorkingState, Presence);
		Result.SelectionStateValidationResult =
			FMatchPlayCurrentAttackSelectionStateValidator::Validate(
				WorkingState.CurrentAttack);
		if (!Result.SelectionStateValidationResult.bIsCanonical)
		{
			Result.ErrorCode =
				EMatchPlayHelperAbsenceErrorCode
					::SelectionStateValidationFailed;
			Result.ErrorMessage =
				Result.SelectionStateValidationResult.ErrorMessage;
			return Result;
		}
		if (WorkingState.CurrentAttack.SelectionStage
			== EMatchPlayCurrentAttackSelectionStage
				::ReadyForResolution)
		{
			Result.ReadyValidationResult =
				FMatchPlayCurrentAttackReadyForResolutionValidator
					::Validate(WorkingState);
			if (!Result.ReadyValidationResult.bSuccess)
			{
				Result.ErrorCode =
					EMatchPlayHelperAbsenceErrorCode
						::ReadyValidationFailed;
				Result.ErrorMessage =
					Result.ReadyValidationResult.ErrorMessage;
				return Result;
			}
		}

		Result.AfterState = MoveTemp(WorkingState);
		Result.bSuccess = true;
		Result.ErrorCode =
			EMatchPlayHelperAbsenceErrorCode::None;
		return Result;
	}
};

FMatchPlayResolveNoLegalHelperResult
FMatchPlayResolveNoLegalHelper::Resolve(
	const FMatchPlayState& BeforeState,
	const FMatchPlayResolveNoLegalHelperRequest& Request)
{
	using namespace MatchPlayHelperAbsenceImplementation;

	FMatchPlayResolveNoLegalHelperResult Result;
	Result.Request = Request;
	Result.BeforeState = BeforeState;
	Result.AfterState = BeforeState;
	if (!BeforeState.RuntimeState.bIsInitialized)
	{
		Result.ErrorCode =
			EMatchPlayHelperAbsenceErrorCode
				::MatchPlayStateNotInitialized;
		Result.ErrorMessage =
			TEXT("Match play state must be initialized before resolving Helper absence.");
		return Result;
	}
	if (!BeforeState.bHasCurrentAttack)
	{
		Result.ErrorCode =
			EMatchPlayHelperAbsenceErrorCode::NoCurrentAttack;
		Result.ErrorMessage =
			TEXT("Resolving Helper absence requires an active current attack.");
		return Result;
	}
	const EInitialTurnOrderPlayer Attacker =
		BeforeState.RuntimeState.CurrentAttackingPlayer;
	if (!IsPlayerSide(Attacker))
	{
		Result.ErrorCode =
			EMatchPlayHelperAbsenceErrorCode
				::InvalidCurrentAttackingPlayer;
		Result.ErrorMessage =
			TEXT("CurrentAttackingPlayer must be PlayerA or PlayerB.");
		return Result;
	}
	const EInitialTurnOrderPlayer Defender = GetDefender(Attacker);
	if (!IsPlayerSide(Defender))
	{
		Result.ErrorCode =
			EMatchPlayHelperAbsenceErrorCode
				::InvalidCurrentDefendingPlayer;
		Result.ErrorMessage =
			TEXT("Current defender could not be derived.");
		return Result;
	}

	Result.HelperAvailabilityResult =
		FMatchPlayCurrentAttackHelperSelectionAvailability::Query(
			BeforeState,
			Request.AttackSequence,
			Defender);
	if (!Result.HelperAvailabilityResult.bQuerySucceeded)
	{
		Result.ErrorCode =
			EMatchPlayHelperAbsenceErrorCode
				::AvailabilityQueryFailed;
		Result.ErrorMessage =
			Result.HelperAvailabilityResult.ErrorMessage;
		return Result;
	}
	if (Result.HelperAvailabilityResult.bCanSelectAnyHelper)
	{
		Result.ErrorCode =
			EMatchPlayHelperAbsenceErrorCode::LegalHelperExists;
		Result.ErrorMessage =
			TEXT("No-legal-Helper resolution requires zero legal Helpers.");
		return Result;
	}

	Result.Reason = EMatchPlayHelperAbsenceReason::NoLegalHelper;
	Result.Source =
		EMatchPlayHelperAbsenceSource::ResolveNoLegalHelper;
	const FMatchPlayHelperAbsenceCapability Capability(
		FMatchPlayHelperAbsenceCapability
			::FResolveNoLegalHelperIssuerTag(),
		Request.AttackSequence,
		Result.HelperAvailabilityResult);
	Result.FinalizationResult =
		FMatchPlayHelperAbsenceFinalizer::FinalizeWithoutHelper(
			BeforeState,
			Capability);
	if (!Result.FinalizationResult.bSuccess)
	{
		Result.ErrorCode =
			EMatchPlayHelperAbsenceErrorCode::FinalizationFailed;
		Result.ErrorMessage =
			Result.FinalizationResult.ErrorMessage;
		return Result;
	}

	Result.AfterState = Result.FinalizationResult.AfterState;
	Result.bSuccess = true;
	Result.ErrorCode =
		EMatchPlayHelperAbsenceErrorCode::None;
	return Result;
}

FMatchPlayHelperDeclineResult FMatchPlayHelperDecline::Decline(
	const FMatchPlayState& BeforeState,
	const FMatchPlayHelperDeclineRequest& Request)
{
	FMatchPlayHelperDeclineResult Result;
	Result.Request = Request;
	Result.BeforeState = BeforeState;
	Result.AfterState = BeforeState;
	Result.HelperAvailabilityResult =
		FMatchPlayCurrentAttackHelperSelectionAvailability::Query(
			BeforeState,
			Request.AttackSequence,
			Request.RequestingSide);
	if (!Result.HelperAvailabilityResult.bQuerySucceeded)
	{
		Result.ErrorCode =
			EMatchPlayHelperAbsenceErrorCode
				::AvailabilityQueryFailed;
		Result.ErrorMessage =
			Result.HelperAvailabilityResult.ErrorMessage;
		return Result;
	}
	if (!Result.HelperAvailabilityResult.bCanSelectAnyHelper)
	{
		Result.ErrorCode =
			EMatchPlayHelperAbsenceErrorCode
				::NoLegalHelperToDecline;
		Result.ErrorMessage =
			TEXT("Helper decline requires at least one legal Helper.");
		return Result;
	}

	Result.Reason = EMatchPlayHelperAbsenceReason::HelperDeclined;
	Result.Source = EMatchPlayHelperAbsenceSource::HelperDecline;
	const FMatchPlayHelperAbsenceCapability Capability(
		FMatchPlayHelperAbsenceCapability::FHelperDeclineIssuerTag(),
		Request.AttackSequence,
		Result.HelperAvailabilityResult);
	Result.FinalizationResult =
		FMatchPlayHelperAbsenceFinalizer::FinalizeWithoutHelper(
			BeforeState,
			Capability);
	if (!Result.FinalizationResult.bSuccess)
	{
		Result.ErrorCode =
			EMatchPlayHelperAbsenceErrorCode::FinalizationFailed;
		Result.ErrorMessage =
			Result.FinalizationResult.ErrorMessage;
		return Result;
	}

	Result.AfterState = Result.FinalizationResult.AfterState;
	Result.bSuccess = true;
	Result.ErrorCode =
		EMatchPlayHelperAbsenceErrorCode::None;
	return Result;
}
