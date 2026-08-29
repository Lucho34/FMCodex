#include "MatchPlayCurrentAttackResolveSingleCardFinishingFormulaOrchestrator.h"

#include "MatchPlayCardSnapshotAuthority.h"
#include "MatchPlayCurrentAttackResolveCrossPostRoutePlanOrchestrator.h"
#include "MatchPlayCurrentAttackResolveDirectShotPostRouteDecisionOrPlanOrchestrator.h"
#include "MatchPlayCurrentAttackResolvePassControlPostRoutePlanOrchestrator.h"

namespace MatchPlayCurrentAttackResolveSingleCardFinishingFormula
{
	using FResult =
		FMatchPlayCurrentAttackResolveSingleCardFinishingFormulaResult;
	using EError =
		EMatchPlayCurrentAttackResolveSingleCardFinishingFormulaErrorCode;
	using EFamily = EMatchPlaySingleCardFinishingFormulaFamily;

	void SetFailure(
		FResult& Result,
		const EError ErrorCode,
		const FString& ErrorMessage,
		const FName InvalidField = NAME_None)
	{
		Result.ErrorCode = ErrorCode;
		Result.ErrorMessage = ErrorMessage;
		Result.InvalidField = InvalidField;
	}

	bool AreStatesEqual(
		const FMatchPlayState& Left,
		const FMatchPlayState& Right)
	{
		return FMatchPlayState::StaticStruct()->CompareScriptStruct(
			&Left,
			&Right,
			0);
	}

	EFamily SelectFamily(
		const FMatchPlayCurrentAttackActualBranch& Branch)
	{
		switch (Branch.ActionType)
		{
		case ESkillRuleType::Cross:
			return Branch.Cross == EMatchPlayCrossActualBranch::High
				|| Branch.Cross == EMatchPlayCrossActualBranch::Low
				? EFamily::Cross
				: EFamily::None;

		case ESkillRuleType::PassControl:
			switch (Branch.PassControl)
			{
			case EMatchPlayPassControlActualBranch::PassAdvance:
				return EFamily::PassAdvance;
			case EMatchPlayPassControlActualBranch::DribbleAdvance:
				return EFamily::DribbleAdvance;
			case EMatchPlayPassControlActualBranch::RunAdvance:
				return EFamily::RunAdvance;
			default:
				return EFamily::None;
			}

		case ESkillRuleType::LongShot:
			return Branch.LongShot
				== EMatchPlayLongShotActualBranch::DirectShot
				? EFamily::LongShotDirectShot
				: EFamily::None;

		case ESkillRuleType::CutInsideShot:
			return Branch.CutInsideShot
				== EMatchPlayCutInsideShotActualBranch::DirectShot
				? EFamily::CutInsideShotDirectShot
				: EFamily::None;

		default:
			return EFamily::None;
		}
	}

	FName GetGoalkeeperCardId(
		const FMatchPlayState& State,
		const EInitialTurnOrderPlayer DefendingSide)
	{
		if (DefendingSide == EInitialTurnOrderPlayer::PlayerA)
		{
			return FName(*State.RuntimeState.PlayerAState.GoalkeeperCardId);
		}
		if (DefendingSide == EInitialTurnOrderPlayer::PlayerB)
		{
			return FName(*State.RuntimeState.PlayerBState.GoalkeeperCardId);
		}
		return NAME_None;
	}

	bool ResolveAdditionalActiveGoalkeeperContribution(
		FResult& Result,
		const EFamily Family,
		float& OutContribution,
		bool& bOutGoalkeeperParticipated)
	{
		OutContribution = 0.0f;
		bOutGoalkeeperParticipated = false;
		if (!Result.BeforeState.CurrentAttack
				.bCurrentDefenseGoalkeeperActivated)
		{
			return true;
		}

		const bool bUsesPositioning =
			Family == EFamily::LongShotDirectShot;
		const bool bUsesHandling =
			Family == EFamily::CutInsideShotDirectShot
			|| Family == EFamily::PassAdvance
			|| Family == EFamily::DribbleAdvance
			|| Family == EFamily::RunAdvance;
		if (!bUsesPositioning && !bUsesHandling)
		{
			return true;
		}

		const EInitialTurnOrderPlayer DefendingSide = Result.BeforeState
			.CurrentAttack.ResolutionSession.Bundle.CurrentDefendingPlayer;
		const FName GoalkeeperCardId =
			GetGoalkeeperCardId(Result.BeforeState, DefendingSide);
		if (GoalkeeperCardId.IsNone())
		{
			SetFailure(
				Result,
				EError::ActiveGoalkeeperSnapshotUnavailable,
				TEXT("An active goalkeeper contribution requires an authoritative goalkeeper CardId."),
				TEXT("GoalkeeperCardId"));
			return false;
		}

		const FMatchPlayCardSnapshotAuthorityQueryResult QueryResult =
			FMatchPlayCardSnapshotAuthorityQuery::FindByPlayerSideAndCardId(
				Result.BeforeState.CardSnapshotAuthority,
				DefendingSide,
				GoalkeeperCardId);
		if (!QueryResult.bSuccess)
		{
			SetFailure(
				Result,
				EError::ActiveGoalkeeperSnapshotUnavailable,
				QueryResult.ErrorMessage,
				TEXT("GoalkeeperSnapshot"));
			return false;
		}
		if (!QueryResult.Snapshot.bIsGoalkeeper
			|| !QueryResult.Snapshot.bHasGoalkeeperAttributes)
		{
			SetFailure(
				Result,
				EError::InvalidActiveGoalkeeperSnapshot,
				TEXT("The active goalkeeper CardId must resolve to a canonical goalkeeper snapshot."),
				TEXT("GoalkeeperSnapshot"));
			return false;
		}

		const int32 CanonicalAttribute = bUsesPositioning
			? QueryResult.Snapshot.GoalkeeperAttributes.Positioning
			: QueryResult.Snapshot.GoalkeeperAttributes.Handling;
		OutContribution = static_cast<float>(CanonicalAttribute) * 0.5f;
		bOutGoalkeeperParticipated = true;
		return true;
	}

	bool ValidateRegeneration(
		FResult& Result,
		const bool bSucceeded,
		const int32 ProviderCallCount,
		const FMatchPlayState& RegeneratedAfterState,
		const FString& FailureMessage)
	{
		Result.PlanRegenerationProviderCallCount = ProviderCallCount;
		if (!bSucceeded)
		{
			SetFailure(Result, EError::PlanRegenerationFailed, FailureMessage);
			return false;
		}
		if (ProviderCallCount != 0)
		{
			SetFailure(
				Result,
				EError::PlanRegenerationConsumedRng,
				TEXT("Formula plan regeneration must consume zero post-route RNG."));
			return false;
		}
		if (!AreStatesEqual(Result.BeforeState, RegeneratedAfterState))
		{
			SetFailure(
				Result,
				EError::PlanRegenerationMutatedState,
				TEXT("Formula plan regeneration must not mutate authoritative State."));
			return false;
		}
		return true;
	}

	bool ExecutePlan(
		FResult& Result,
		const FSingleCardFormulaInputAssemblyQueryInput& AttackerInput,
		const FSingleCardFormulaInputAssemblyQueryInput& DefenderInput,
		const FName AttackerPlayerId,
		const FName DefenderPlayerId,
		const float AdditionalGoalkeeperContribution,
		const bool bGoalkeeperParticipated,
		const TArray<int32>* AttackerParticipatingStaminaOverride = nullptr,
		const TArray<int32>* DefenderParticipatingStaminaOverride = nullptr)
	{
		Result.AttackerQueryResult =
			FSingleCardFormulaInputAssemblyQuery::Assemble(
				Result.PlayerCardSnapshots,
				AttackerInput);
		if (!Result.AttackerQueryResult.bSuccess)
		{
			SetFailure(
				Result,
				EError::AttackerInputAssemblyFailed,
				Result.AttackerQueryResult.ErrorMessage,
				Result.AttackerQueryResult.InvalidField);
			return false;
		}

		Result.DefenderQueryResult =
			FSingleCardFormulaInputAssemblyQuery::Assemble(
				Result.PlayerCardSnapshots,
				DefenderInput);
		if (!Result.DefenderQueryResult.bSuccess)
		{
			SetFailure(
				Result,
				EError::DefenderInputAssemblyFailed,
				Result.DefenderQueryResult.ErrorMessage,
				Result.DefenderQueryResult.InvalidField);
			return false;
		}

		FSingleCardFormulaResolverInputAssemblyInput AssemblyInput;
		AssemblyInput.AttackerQueryResult = Result.AttackerQueryResult;
		AssemblyInput.DefenderQueryResult = Result.DefenderQueryResult;
		AssemblyInput.AttackerPlayerId = AttackerPlayerId;
		AssemblyInput.DefenderPlayerId = DefenderPlayerId;
		Result.ResolverInputAssemblyResult =
			FSingleCardFormulaResolverInputAssembler::Assemble(AssemblyInput);
		if (!Result.ResolverInputAssemblyResult.bSuccess)
		{
			SetFailure(
				Result,
				EError::ResolverInputAssemblyFailed,
				Result.ResolverInputAssemblyResult.ErrorMessage,
				Result.ResolverInputAssemblyResult.InvalidField);
			return false;
		}

		// Active-GK contributions remain independent defensive additions.
		// Cross already carries its path-specific contribution in the plan;
		// the repaired families add their frozen 50% contribution here.
		Result.ResolverInputAssemblyResult.ResolverInput.Defender.Modifier +=
			AdditionalGoalkeeperContribution;
		Result.ResolverInputAssemblyResult.ResolverInput.Attacker
			.TacticalPlayerModifier = Result.TacticalPlayerAdvantageResult
			.AttackerFinishingModifier;
		Result.ResolverInputAssemblyResult.ResolverInput.Defender
			.TacticalPlayerModifier = Result.TacticalPlayerAdvantageResult
			.DefenderFinishingModifier;
		Result.ResolverInputAssemblyResult.ResolverInput
			.bGoalkeeperParticipated = bGoalkeeperParticipated;
		if (AttackerParticipatingStaminaOverride != nullptr)
		{
			Result.ResolverInputAssemblyResult.ResolverInput.Attacker
				.ParticipatingStamina = *AttackerParticipatingStaminaOverride;
		}
		if (DefenderParticipatingStaminaOverride != nullptr)
		{
			Result.ResolverInputAssemblyResult.ResolverInput.Defender
				.ParticipatingStamina = *DefenderParticipatingStaminaOverride;
		}

		++Result.FormulaExecutionCount;
		Result.FormulaExecutionResult =
			FSingleCardFormulaResolutionExecutor::Execute(
				Result.ResolverInputAssemblyResult.ResolverInput);
		if (!Result.FormulaExecutionResult.bSuccess
			|| !Result.FormulaExecutionResult.bExecuted)
		{
			SetFailure(
				Result,
				EError::FormulaExecutionFailed,
				Result.FormulaExecutionResult.ErrorMessage,
				Result.FormulaExecutionResult.InvalidField);
			return false;
		}

		const FFormulaResolutionResult& FormulaResult =
			Result.FormulaExecutionResult.FormulaResolutionResult;
		if (FormulaResult.FormulaType != EFormulaType::Finishing
			|| FormulaResult.Winner == EFormulaWinner::None
			|| !FormulaResult.bAttackEnded
			|| FormulaResult.bContinueResolution)
		{
			SetFailure(
				Result,
				EError::InvalidFormulaResolutionResult,
				TEXT("SingleCard finishing execution returned a non-terminal Formula result."),
				TEXT("FormulaResolutionResult"));
			return false;
		}

		Result.FormulaResolutionResult = FormulaResult;
		Result.bHasFormulaResolution = true;
		return true;
	}
}

FMatchPlayCurrentAttackResolveSingleCardFinishingFormulaResult
FMatchPlayCurrentAttackResolveSingleCardFinishingFormulaOrchestrator::Resolve(
	const FMatchPlayState& BeforeState,
	const FSkillRuleSnapshotSet* SkillRuleSet)
{
	using namespace MatchPlayCurrentAttackResolveSingleCardFinishingFormula;

	FResult Result;
	Result.BeforeState = BeforeState;
	Result.AfterState = BeforeState;
	if (!BeforeState.RuntimeState.bIsInitialized)
	{
		SetFailure(
			Result,
			EError::MatchPlayStateNotInitialized,
			TEXT("SingleCard finishing Formula resolution requires initialized MatchPlay State."));
		return Result;
	}
	if (!BeforeState.bHasCurrentAttack)
	{
		SetFailure(
			Result,
			EError::NoCurrentAttack,
			TEXT("SingleCard finishing Formula resolution requires an active CurrentAttack."));
		return Result;
	}
	if (!BeforeState.CurrentAttack.bHasResolutionSession)
	{
		SetFailure(
			Result,
			EError::MissingResolutionSession,
			TEXT("SingleCard finishing Formula resolution requires a Resolution Session."));
		return Result;
	}

	Result.SessionStateValidationResult =
		FMatchPlayCurrentAttackResolutionSessionStateValidator::Validate(
			BeforeState);
	if (!Result.SessionStateValidationResult.bIsCanonical)
	{
		SetFailure(
			Result,
			EError::InvalidResolutionSessionState,
			Result.SessionStateValidationResult.ErrorMessage);
		return Result;
	}

	const FMatchPlayCurrentAttackResolutionSession& Session =
		BeforeState.CurrentAttack.ResolutionSession;
	if (Session.Stage != EMatchPlayCurrentAttackResolutionStage::RouteResolved)
	{
		SetFailure(
			Result,
			EError::RouteNotResolved,
			TEXT("SingleCard finishing Formula resolution requires RouteResolved."));
		return Result;
	}
	if (!Session.bHasActualBranch)
	{
		SetFailure(
			Result,
			EError::MissingActualBranch,
			TEXT("SingleCard finishing Formula resolution requires an ActualBranch."));
		return Result;
	}

	Result.Family = SelectFamily(Session.ActualBranch);
	if (Result.Family == EFamily::None)
	{
		SetFailure(
			Result,
			EError::UnsupportedFormulaFamily,
			TEXT("Current ActualBranch is not a supported SingleCard finishing Formula family."));
		return Result;
	}

	Result.ProgressResult =
		FMatchPlayCurrentAttackPostRouteRollProgressQuery::Evaluate(Session);
	if (!Result.ProgressResult.bIsCanonical)
	{
		SetFailure(
			Result,
			EError::InvalidPostRouteProgress,
			Result.ProgressResult.ErrorMessage);
		return Result;
	}
	if (!Result.ProgressResult.bContractComplete)
	{
		SetFailure(
			Result,
			EError::IncompletePostRouteRollProgress,
			TEXT("All canonical post-route rolls must exist before Formula execution."));
		return Result;
	}
	if (SkillRuleSet == nullptr)
	{
		SetFailure(
			Result,
			EError::SkillRuleSetUnavailable,
			TEXT("SingleCard finishing Formula resolution requires the authoritative SkillRuleSet."));
		return Result;
	}
	Result.TacticalPlayerAdvantageResult =
		FMatchPlayTacticalPlayerAdvantageQuery::Evaluate(BeforeState);
	if (!Result.TacticalPlayerAdvantageResult.bSuccess)
	{
		SetFailure(
			Result,
			EError::TacticalPlayerAdvantageQueryFailed,
			Result.TacticalPlayerAdvantageResult.ErrorMessage,
			TEXT("TacticalPlayerAdvantage"));
		return Result;
	}

	const int64 AttackSequence = BeforeState.CurrentAttack.AttackSequence;
	float AdditionalGoalkeeperContribution = 0.0f;
	bool bAdditionalGoalkeeperParticipated = false;
	if (!ResolveAdditionalActiveGoalkeeperContribution(
			Result,
			Result.Family,
			AdditionalGoalkeeperContribution,
			bAdditionalGoalkeeperParticipated))
	{
		return Result;
	}
	switch (Result.Family)
	{
	case EFamily::Cross:
	{
		FMatchPlayCurrentAttackResolveCrossPostRoutePlanRequest Request;
		Request.AttackSequence = AttackSequence;
		Request.Mode = FMatchPlayCurrentAttackResolveCrossPostRoutePlanRequest
			::EMode::RegenerateCompletedPlan;
		Result.CrossRegenerationResult =
			FMatchPlayCurrentAttackResolveCrossPostRoutePlanOrchestrator::Resolve(
				BeforeState,
				Request,
				SkillRuleSet,
				nullptr);
		if (!ValidateRegeneration(
				Result,
				Result.CrossRegenerationResult.bSuccess,
				Result.CrossRegenerationResult.ProviderCallCount,
				Result.CrossRegenerationResult.AfterState,
				Result.CrossRegenerationResult.ErrorMessage))
		{
			return Result;
		}
		const FCrossPlanQueryResult& PlanResult =
			Result.CrossRegenerationResult.PlanResult;
		if (!PlanResult.bSuccess
			|| PlanResult.Decision
				!= ECrossPlanQueryDecision::FormulaResolutionRequired
			|| !PlanResult.bHasFormulaPlan)
		{
			SetFailure(
				Result,
				EError::FormulaPlanUnavailable,
				TEXT("Canonical Cross regeneration did not produce a Formula plan."));
			return Result;
		}
		const FCrossFormulaPlan& Plan = PlanResult.FormulaPlan;
		Result.PlayerCardSnapshots =
			Result.CrossRegenerationResult.ScopedPlayerCardSnapshots;
		if (!ExecutePlan(
				Result,
				Plan.AttackerQueryInput,
				Plan.DefenderQueryInput,
				Plan.CarrierPlayerId,
				Plan.MarkerPlayerId,
				0.0f,
				Plan.bUseGoalkeeper))
		{
			return Result;
		}
		break;
	}

	case EFamily::PassAdvance:
	case EFamily::DribbleAdvance:
	case EFamily::RunAdvance:
	{
		FMatchPlayCurrentAttackResolvePassControlPostRoutePlanRequest Request;
		Request.AttackSequence = AttackSequence;
		Request.Mode =
			FMatchPlayCurrentAttackResolvePassControlPostRoutePlanRequest::EMode
				::RegenerateCompletedPlan;
		Result.PassControlRegenerationResult =
			FMatchPlayCurrentAttackResolvePassControlPostRoutePlanOrchestrator
				::Resolve(BeforeState, Request, SkillRuleSet, nullptr);
		if (!ValidateRegeneration(
				Result,
				Result.PassControlRegenerationResult.bSuccess,
				Result.PassControlRegenerationResult.ProviderCallCount,
				Result.PassControlRegenerationResult.AfterState,
				Result.PassControlRegenerationResult.ErrorMessage))
		{
			return Result;
		}
		Result.PlayerCardSnapshots =
			Result.PassControlRegenerationResult.PlayerCardSnapshots;
		const FMatchPlayCurrentAttackResolutionSessionBundle& Bundle =
			BeforeState.CurrentAttack.ResolutionSession.Bundle;
		const TArray<int32> AttackerParticipatingStamina = {
			Bundle.Carrier.Values.Stamina,
			Bundle.Runner.Values.Stamina };
		TArray<int32> DefenderParticipatingStamina = {
			Bundle.Marker.Values.Stamina };
		if (Bundle.bHasHelper)
		{
			DefenderParticipatingStamina.Add(Bundle.Helper.Values.Stamina);
		}
		if (Result.Family == EFamily::PassAdvance)
		{
			const FPassControlPassAdvancePlanQueryResult& PlanResult =
				Result.PassControlRegenerationResult.PassAdvanceResult;
			if (!PlanResult.bSuccess || !PlanResult.bHasFormulaPlan)
			{
				SetFailure(Result, EError::FormulaPlanUnavailable,
					TEXT("Canonical PassAdvance regeneration did not produce a Formula plan."));
				return Result;
			}
			const FPassControlPassAdvanceFormulaPlan& Plan =
				PlanResult.FormulaPlan;
			if (!ExecutePlan(Result, Plan.AttackerQueryInput,
				Plan.DefenderQueryInput, Plan.CarrierPlayerId,
				Plan.MarkerPlayerId, AdditionalGoalkeeperContribution,
				bAdditionalGoalkeeperParticipated,
				&AttackerParticipatingStamina,
				&DefenderParticipatingStamina))
			{
				return Result;
			}
		}
		else if (Result.Family == EFamily::DribbleAdvance)
		{
			const FPassControlDribbleAdvancePlanQueryResult& PlanResult =
				Result.PassControlRegenerationResult.DribbleAdvanceResult;
			if (!PlanResult.bSuccess || !PlanResult.bHasFormulaPlan)
			{
				SetFailure(Result, EError::FormulaPlanUnavailable,
					TEXT("Canonical DribbleAdvance regeneration did not produce a Formula plan."));
				return Result;
			}
			const FPassControlDribbleAdvanceFormulaPlan& Plan =
				PlanResult.FormulaPlan;
			if (!ExecutePlan(Result, Plan.AttackerQueryInput,
				Plan.DefenderQueryInput, Plan.CarrierPlayerId,
				Plan.MarkerPlayerId, AdditionalGoalkeeperContribution,
				bAdditionalGoalkeeperParticipated,
				&AttackerParticipatingStamina,
				&DefenderParticipatingStamina))
			{
				return Result;
			}
		}
		else
		{
			const FPassControlRunAdvancePlanQueryResult& PlanResult =
				Result.PassControlRegenerationResult.RunAdvanceResult;
			if (!PlanResult.bSuccess || !PlanResult.bHasFormulaPlan)
			{
				SetFailure(Result, EError::FormulaPlanUnavailable,
					TEXT("Canonical RunAdvance regeneration did not produce a Formula plan."));
				return Result;
			}
			const FPassControlRunAdvanceFormulaPlan& Plan =
				PlanResult.FormulaPlan;
			if (!ExecutePlan(Result, Plan.AttackerQueryInput,
				Plan.DefenderQueryInput, Plan.CarrierPlayerId,
				Plan.MarkerPlayerId, AdditionalGoalkeeperContribution,
				bAdditionalGoalkeeperParticipated,
				&AttackerParticipatingStamina,
				&DefenderParticipatingStamina))
			{
				return Result;
			}
		}
		break;
	}

	case EFamily::LongShotDirectShot:
	case EFamily::CutInsideShotDirectShot:
	{
		FMatchPlayCurrentAttackResolveDirectShotPostRouteDecisionOrPlanRequest
			Request;
		Request.AttackSequence = AttackSequence;
		Request.Mode =
			FMatchPlayCurrentAttackResolveDirectShotPostRouteDecisionOrPlanRequest
				::EMode::RegenerateCompletedPlan;
		Result.DirectShotRegenerationResult =
			FMatchPlayCurrentAttackResolveDirectShotPostRouteDecisionOrPlanOrchestrator
				::Resolve(BeforeState, Request, SkillRuleSet, nullptr);
		if (!ValidateRegeneration(
				Result,
				Result.DirectShotRegenerationResult.bSuccess,
				Result.DirectShotRegenerationResult.ProviderCallCount,
				Result.DirectShotRegenerationResult.AfterState,
				Result.DirectShotRegenerationResult.ErrorMessage))
		{
			return Result;
		}
		Result.PlayerCardSnapshots =
			Result.DirectShotRegenerationResult.PlayerCardSnapshots;
		if (Result.Family == EFamily::LongShotDirectShot)
		{
			const FLongShotDirectShotPlanQueryResult& PlanResult =
				Result.DirectShotRegenerationResult.LongShotResult;
			if (!PlanResult.bSuccess
				|| PlanResult.Decision
					!= ELongShotDirectShotDecision::FormulaResolutionRequired
				|| !PlanResult.bHasFormulaPlan)
			{
				SetFailure(Result, EError::FormulaPlanUnavailable,
					TEXT("LongShot DirectShot is not on its Formula path."));
				return Result;
			}
			const FLongShotDirectShotFormulaPlan& Plan =
				PlanResult.FormulaPlan;
			if (!ExecutePlan(Result, Plan.AttackerQueryInput,
				Plan.DefenderQueryInput, Plan.AttackerPlayerId,
				Plan.DefenderPlayerId, AdditionalGoalkeeperContribution,
				bAdditionalGoalkeeperParticipated))
			{
				return Result;
			}
		}
		else
		{
			const FCutInsideShotDirectShotPlanQueryResult& PlanResult =
				Result.DirectShotRegenerationResult.CutInsideShotResult;
			if (!PlanResult.bSuccess
				|| PlanResult.Decision
					!= ECutInsideShotDirectShotDecision::FormulaResolutionRequired
				|| !PlanResult.bHasFormulaPlan)
			{
				SetFailure(Result, EError::FormulaPlanUnavailable,
					TEXT("CutInsideShot DirectShot is not on its Formula path."));
				return Result;
			}
			const FCutInsideShotDirectShotFormulaPlan& Plan =
				PlanResult.FormulaPlan;
			if (!ExecutePlan(Result, Plan.AttackerQueryInput,
				Plan.DefenderQueryInput, Plan.AttackerPlayerId,
				Plan.DefenderPlayerId, AdditionalGoalkeeperContribution,
				bAdditionalGoalkeeperParticipated))
			{
				return Result;
			}
		}
		break;
	}

	default:
		checkNoEntry();
		return Result;
	}

	Result.bSuccess = true;
	Result.ErrorCode = EError::None;
	Result.ErrorMessage.Empty();
	Result.InvalidField = NAME_None;
	return Result;
}
