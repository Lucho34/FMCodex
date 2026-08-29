#include "FMCodexLocalMatchHostGameMode.h"

#include "FMCodexLocalMatchPlayerController.h"

namespace FMCodexLocalMatchHost
{
	constexpr const TCHAR* NoActiveMatchMessage =
		TEXT("No local match is active.");
	constexpr const TCHAR* RuleConfigurationMismatchMessage =
		TEXT("The supplied Skill rule configuration does not match the "
			"active local match configuration.");

	FString SelectAuthoritativeErrorMessage(
		const FMatchPlayAuthoritativeRuntimeEnvelope& RuntimeEnvelope,
		const FString& DomainErrorMessage)
	{
		return !RuntimeEnvelope.ErrorMessage.IsEmpty()
			? RuntimeEnvelope.ErrorMessage
			: DomainErrorMessage;
	}

	int32 GenerateLocalMatchSeed()
	{
		const FGuid MatchGuid = FGuid::NewGuid();
		return static_cast<int32>(
			MatchGuid.A ^ MatchGuid.B ^ MatchGuid.C ^ MatchGuid.D);
	}

	bool AreSkillRuleSetsEqual(
		const FSkillRuleSnapshotSet& Left,
		const FSkillRuleSnapshotSet& Right)
	{
		if (Left.SkillRules.Num() != Right.SkillRules.Num())
		{
			return false;
		}
		for (int32 Index = 0; Index < Left.SkillRules.Num(); ++Index)
		{
			const FSkillRuleSnapshot& LeftRule = Left.SkillRules[Index];
			const FSkillRuleSnapshot& RightRule = Right.SkillRules[Index];
			if (LeftRule.SkillId != RightRule.SkillId
				|| LeftRule.SkillType != RightRule.SkillType
				|| LeftRule.MinTriggerActionPoint
					!= RightRule.MinTriggerActionPoint
				|| LeftRule.MaxTriggerActionPoint
					!= RightRule.MaxTriggerActionPoint)
			{
				return false;
			}
		}
		return true;
	}

#if !UE_BUILD_SHIPPING
	EFMCodexLocalDevRollInvocation InitialRouteInvocation(
		const FMatchPlayState& State)
	{
		if (!State.bHasCurrentAttack
			|| !State.CurrentAttack.bHasSelectedAction)
		{
			return EFMCodexLocalDevRollInvocation::None;
		}
		switch (State.CurrentAttack.SelectedAction.ActionType)
		{
		case ESkillRuleType::ThroughBall:
			return EFMCodexLocalDevRollInvocation::ThroughBallInitialRoute;
		case ESkillRuleType::Cross:
			return EFMCodexLocalDevRollInvocation::CrossInitialRoute;
		default:
			return EFMCodexLocalDevRollInvocation::None;
		}
	}
#endif
}

AFMCodexLocalMatchHostGameMode::AFMCodexLocalMatchHostGameMode()
{
	PlayerControllerClass = AFMCodexLocalMatchPlayerController::StaticClass();
}

AFMCodexLocalMatchHostGameMode::FLocalMatchRuntime::FLocalMatchRuntime(
	const int32 Seed,
	const FSkillRuleSnapshotSet& InSkillRuleSet)
	: D6Provider(Seed)
#if !UE_BUILD_SHIPPING
	, DevRollOverride(D6Provider)
#endif
	, SkillRuleSet(InSkillRuleSet)
	, AuthoritativeSession(
#if !UE_BUILD_SHIPPING
		DevRollOverride,
		DevRollOverride,
#else
		D6Provider,
		D6Provider,
#endif
		SkillRuleSet)
{
}

#if !UE_BUILD_SHIPPING
FFMCodexLocalDevRollOverrideCommandResult
AFMCodexLocalMatchHostGameMode::SetLocalDevRollOverride(
	const FFMCodexLocalDevRollOverrideRequest& Request)
{
	if (!ActiveMatchRuntime.IsValid())
	{
		FFMCodexLocalDevRollOverrideCommandResult Result;
		Result.ErrorMessage = FMCodexLocalMatchHost::NoActiveMatchMessage;
		return Result;
	}
	return ActiveMatchRuntime->DevRollOverride.SetOverride(Request);
}

bool AFMCodexLocalMatchHostGameMode::ClearLocalDevRollOverride(
	const EFMCodexLocalDevRollTarget Target)
{
	return ActiveMatchRuntime.IsValid()
		&& ActiveMatchRuntime->DevRollOverride.ClearOverride(Target);
}

void AFMCodexLocalMatchHostGameMode::ClearAllLocalDevRollOverrides()
{
	if (ActiveMatchRuntime.IsValid())
	{
		ActiveMatchRuntime->DevRollOverride.ClearAllOverrides();
	}
}

TArray<FFMCodexLocalDevPendingRollOverride>
AFMCodexLocalMatchHostGameMode::GetLocalDevPendingRollOverrides() const
{
	return ActiveMatchRuntime.IsValid()
		? ActiveMatchRuntime->DevRollOverride.GetPendingOverrides()
		: TArray<FFMCodexLocalDevPendingRollOverride>();
}
#endif

bool AFMCodexLocalMatchHostGameMode::HasActiveLocalMatch() const
{
	return ActiveMatchRuntime.IsValid();
}

FFMCodexStartNewLocalMatchResult
AFMCodexLocalMatchHostGameMode::StartNewLocalMatch(
	const FMatchPlayOpeningInitializeInput& Input)
{
	return StartNewLocalMatch(Input, FSkillRuleSnapshotSet());
}

FFMCodexStartNewLocalMatchResult
AFMCodexLocalMatchHostGameMode::StartNewLocalMatch(
	const FMatchPlayOpeningInitializeInput& Input,
	const FSkillRuleSnapshotSet& SkillRuleSet)
{
	return StartNewLocalMatchWithSeed(
		Input,
		SkillRuleSet,
		FMCodexLocalMatchHost::GenerateLocalMatchSeed());
}

#if WITH_DEV_AUTOMATION_TESTS
FFMCodexStartNewLocalMatchResult
AFMCodexLocalMatchHostGameMode::StartNewLocalMatch(
	const FMatchPlayOpeningInitializeInput& Input,
	const FSkillRuleSnapshotSet& SkillRuleSet,
	const int32 DeterministicSeedForTesting)
{
	return StartNewLocalMatchWithSeed(
		Input,
		SkillRuleSet,
		DeterministicSeedForTesting);
}
#endif

FFMCodexStartNewLocalMatchResult
AFMCodexLocalMatchHostGameMode::StartNewLocalMatchWithSeed(
	const FMatchPlayOpeningInitializeInput& Input,
	const FSkillRuleSnapshotSet& SkillRuleSet,
	const int32 Seed)
{
	using namespace FMCodexLocalMatchHost;

	FFMCodexStartNewLocalMatchResult Result;
	TUniquePtr<FLocalMatchRuntime> CandidateRuntime =
		MakeUnique<FLocalMatchRuntime>(
			Seed,
			SkillRuleSet);
	Result.AuthoritativeResult =
		CandidateRuntime->AuthoritativeSession.InitializeMatch(Input);
	Result.bSuccess =
		Result.AuthoritativeResult.RuntimeEnvelope.bAccepted
		&& Result.AuthoritativeResult.RuntimeEnvelope.bDomainSuccess
		&& Result.AuthoritativeResult.OpeningResult.bSuccess;
	if (!Result.bSuccess)
	{
		Result.ErrorCode = EFMCodexLocalMatchHostErrorCode
			::AuthoritativeInitializationFailed;
		Result.ErrorMessage = SelectAuthoritativeErrorMessage(
			Result.AuthoritativeResult.RuntimeEnvelope,
			Result.AuthoritativeResult.OpeningResult.ErrorMessage);
		return Result;
	}

	Result.bReplacedExistingMatch = ActiveMatchRuntime.IsValid();
	ActiveMatchRuntime = MoveTemp(CandidateRuntime);
	return Result;
}

FFMCodexLocalMatchSnapshotResult
AFMCodexLocalMatchHostGameMode::GetMatchSnapshot() const
{
	using namespace FMCodexLocalMatchHost;

	FFMCodexLocalMatchSnapshotResult Result;
	if (!ActiveMatchRuntime.IsValid())
	{
		Result.ErrorCode =
			EFMCodexLocalMatchHostErrorCode::NoActiveMatch;
		Result.ErrorMessage = NoActiveMatchMessage;
		return Result;
	}

	Result.Snapshot =
		ActiveMatchRuntime->AuthoritativeSession.GetStateSnapshot();
	Result.bSuccess = true;
	return Result;
}

FFMCodexLocalMatchSkillRuleSnapshotResult
AFMCodexLocalMatchHostGameMode::GetSkillRuleSnapshot() const
{
	using namespace FMCodexLocalMatchHost;

	FFMCodexLocalMatchSkillRuleSnapshotResult Result;
	if (!ActiveMatchRuntime.IsValid())
	{
		Result.ErrorCode = EFMCodexLocalMatchHostErrorCode::NoActiveMatch;
		Result.ErrorMessage = NoActiveMatchMessage;
		return Result;
	}

	Result.Snapshot = ActiveMatchRuntime->SkillRuleSet;
	Result.bSuccess = true;
	return Result;
}

#if WITH_DEV_AUTOMATION_TESTS
FFMCodexLocalMatchBeginOrdinaryAttackResult
AFMCodexLocalMatchHostGameMode::BeginOrdinaryAttack(
	const int32 ActionPoint)
{
	using namespace FMCodexLocalMatchHost;

	FFMCodexLocalMatchBeginOrdinaryAttackResult Result;
	if (!ActiveMatchRuntime.IsValid())
	{
		Result.ErrorCode =
			EFMCodexLocalMatchHostErrorCode::NoActiveMatch;
		Result.ErrorMessage = NoActiveMatchMessage;
		return Result;
	}

	Result.AuthoritativeResult =
		ActiveMatchRuntime->AuthoritativeSession.BeginOrdinaryAttack(
			ActionPoint);
	Result.bSuccess =
		Result.AuthoritativeResult.RuntimeEnvelope.bAccepted
		&& Result.AuthoritativeResult.RuntimeEnvelope.bDomainSuccess
		&& Result.AuthoritativeResult.BeginResult.bSuccess;
	if (!Result.bSuccess)
	{
		Result.ErrorCode =
			EFMCodexLocalMatchHostErrorCode::AuthoritativeCommandFailed;
		Result.ErrorMessage = SelectAuthoritativeErrorMessage(
			Result.AuthoritativeResult.RuntimeEnvelope,
			Result.AuthoritativeResult.BeginResult.ErrorMessage);
	}
	return Result;
}
#endif

FFMCodexLocalMatchRollTacticalPointsResult
AFMCodexLocalMatchHostGameMode::RollTacticalPoints(
	const EInitialTurnOrderPlayer RequestingSide)
{
	using namespace FMCodexLocalMatchHost;

	FFMCodexLocalMatchRollTacticalPointsResult Result;
	if (!ActiveMatchRuntime.IsValid())
	{
		Result.ErrorCode = EFMCodexLocalMatchHostErrorCode::NoActiveMatch;
		Result.ErrorMessage = NoActiveMatchMessage;
		return Result;
	}

	const FMatchPlayState Snapshot =
		ActiveMatchRuntime->AuthoritativeSession.GetStateSnapshot();
	if (RequestingSide != EInitialTurnOrderPlayer::PlayerA
		&& RequestingSide != EInitialTurnOrderPlayer::PlayerB)
	{
		Result.ErrorCode =
			EFMCodexLocalMatchHostErrorCode::InvalidRequestingSide;
		Result.ErrorMessage =
			TEXT("Tactical Point roll requires PlayerA or PlayerB.");
		return Result;
	}
	if (RequestingSide != Snapshot.RuntimeState.CurrentAttackingPlayer)
	{
		Result.ErrorCode = EFMCodexLocalMatchHostErrorCode
			::RequestingSideNotCurrentAttacker;
		Result.ErrorMessage =
			TEXT("Only the current attacking player may roll Tactical Points.");
		return Result;
	}

	const FPlayerRuntimeState& AttackerState =
		RequestingSide == EInitialTurnOrderPlayer::PlayerA
			? Snapshot.RuntimeState.PlayerAState
			: Snapshot.RuntimeState.PlayerBState;
	if (Snapshot.bHasCurrentAttack
		|| AttackerState.UsedAttackCount >= AttackerState.TotalAttackCount)
	{
		Result.ErrorCode =
			EFMCodexLocalMatchHostErrorCode::TacticalPointRollNotReady;
		Result.ErrorMessage =
			TEXT("Tactical Point roll is not ready for the current match state.");
		return Result;
	}

#if !UE_BUILD_SHIPPING
	Result.TacticalPoints =
		ActiveMatchRuntime->DevRollOverride.RollOrdinaryTacticalPoint();
#else
	Result.TacticalPoints =
		ActiveMatchRuntime->D6Provider.RollOrdinaryTacticalPoint();
#endif
	Result.AuthoritativeResult =
		ActiveMatchRuntime->AuthoritativeSession.BeginOrdinaryAttack(
			Result.TacticalPoints);
	Result.bSuccess =
		Result.AuthoritativeResult.RuntimeEnvelope.bAccepted
		&& Result.AuthoritativeResult.RuntimeEnvelope.bDomainSuccess
		&& Result.AuthoritativeResult.BeginResult.bSuccess;
	if (!Result.bSuccess)
	{
		Result.ErrorCode =
			EFMCodexLocalMatchHostErrorCode::AuthoritativeCommandFailed;
		Result.ErrorMessage = SelectAuthoritativeErrorMessage(
			Result.AuthoritativeResult.RuntimeEnvelope,
			Result.AuthoritativeResult.BeginResult.ErrorMessage);
	}
	return Result;
}

FFMCodexLocalMatchDeployOrdinaryResult
AFMCodexLocalMatchHostGameMode::DeployOrdinary(
	const FMatchPlayAuthoritativeDeployOrdinaryRequest& Request)
{
	using namespace FMCodexLocalMatchHost;

	FFMCodexLocalMatchDeployOrdinaryResult Result;
	if (!ActiveMatchRuntime.IsValid())
	{
		Result.ErrorCode = EFMCodexLocalMatchHostErrorCode::NoActiveMatch;
		Result.ErrorMessage = NoActiveMatchMessage;
		return Result;
	}

	Result.AuthoritativeResult =
		ActiveMatchRuntime->AuthoritativeSession.DeployOrdinary(Request);
	Result.bSuccess = Result.AuthoritativeResult.RuntimeEnvelope.bAccepted
		&& Result.AuthoritativeResult.RuntimeEnvelope.bDomainSuccess
		&& Result.AuthoritativeResult.DeploymentResult.bSuccess;
	if (!Result.bSuccess)
	{
		Result.ErrorCode =
			EFMCodexLocalMatchHostErrorCode::AuthoritativeCommandFailed;
		Result.ErrorMessage = SelectAuthoritativeErrorMessage(
			Result.AuthoritativeResult.RuntimeEnvelope,
			Result.AuthoritativeResult.DeploymentResult.ErrorMessage);
	}
	return Result;
}

FFMCodexLocalMatchDeployGoalkeeperResult
AFMCodexLocalMatchHostGameMode::DeployGoalkeeper(
	const FMatchPlayAuthoritativeDeployGoalkeeperRequest& Request)
{
	using namespace FMCodexLocalMatchHost;

	FFMCodexLocalMatchDeployGoalkeeperResult Result;
	if (!ActiveMatchRuntime.IsValid())
	{
		Result.ErrorCode = EFMCodexLocalMatchHostErrorCode::NoActiveMatch;
		Result.ErrorMessage = NoActiveMatchMessage;
		return Result;
	}

	Result.AuthoritativeResult =
		ActiveMatchRuntime->AuthoritativeSession.DeployGoalkeeper(Request);
	Result.bSuccess = Result.AuthoritativeResult.RuntimeEnvelope.bAccepted
		&& Result.AuthoritativeResult.RuntimeEnvelope.bDomainSuccess
		&& Result.AuthoritativeResult.DeploymentResult.bSucceeded;
	if (!Result.bSuccess)
	{
		Result.ErrorCode =
			EFMCodexLocalMatchHostErrorCode::AuthoritativeCommandFailed;
		Result.ErrorMessage = SelectAuthoritativeErrorMessage(
			Result.AuthoritativeResult.RuntimeEnvelope,
			Result.AuthoritativeResult.DeploymentResult.ErrorMessage);
	}
	return Result;
}

FFMCodexLocalMatchFinishDeploymentResult
AFMCodexLocalMatchHostGameMode::FinishDeployment(
	const int64 AttackSequence,
	const EInitialTurnOrderPlayer RequestingSide)
{
	using namespace FMCodexLocalMatchHost;

	FFMCodexLocalMatchFinishDeploymentResult Result;
	if (!ActiveMatchRuntime.IsValid())
	{
		Result.ErrorCode = EFMCodexLocalMatchHostErrorCode::NoActiveMatch;
		Result.ErrorMessage = NoActiveMatchMessage;
		return Result;
	}

	Result.AuthoritativeResult =
		ActiveMatchRuntime->AuthoritativeSession.FinishDeployment(
			AttackSequence,
			RequestingSide);
	Result.bSuccess = Result.AuthoritativeResult.RuntimeEnvelope.bAccepted
		&& Result.AuthoritativeResult.RuntimeEnvelope.bDomainSuccess
		&& Result.AuthoritativeResult.FinishResult.bSuccess;
	if (!Result.bSuccess)
	{
		Result.ErrorCode =
			EFMCodexLocalMatchHostErrorCode::AuthoritativeCommandFailed;
		Result.ErrorMessage = SelectAuthoritativeErrorMessage(
			Result.AuthoritativeResult.RuntimeEnvelope,
			Result.AuthoritativeResult.FinishResult.ErrorMessage);
	}
	return Result;
}

FFMCodexLocalMatchSubmitCarrierResult
AFMCodexLocalMatchHostGameMode::SubmitCarrier(
	const FMatchPlayAuthoritativeSubmitCarrierRequest& Request)
{
	using namespace FMCodexLocalMatchHost;

	FFMCodexLocalMatchSubmitCarrierResult Result;
	if (!ActiveMatchRuntime.IsValid())
	{
		Result.ErrorCode = EFMCodexLocalMatchHostErrorCode::NoActiveMatch;
		Result.ErrorMessage = NoActiveMatchMessage;
		return Result;
	}

	Result.AuthoritativeResult =
		ActiveMatchRuntime->AuthoritativeSession.SubmitCarrier(Request);
	Result.bSuccess = Result.AuthoritativeResult.RuntimeEnvelope.bAccepted
		&& Result.AuthoritativeResult.RuntimeEnvelope.bDomainSuccess
		&& Result.AuthoritativeResult.CarrierResult.bSuccess;
	if (!Result.bSuccess)
	{
		Result.ErrorCode =
			EFMCodexLocalMatchHostErrorCode::AuthoritativeCommandFailed;
		Result.ErrorMessage = SelectAuthoritativeErrorMessage(
			Result.AuthoritativeResult.RuntimeEnvelope,
			Result.AuthoritativeResult.CarrierResult.ErrorMessage);
	}
	return Result;
}

FFMCodexLocalMatchResolveNoLegalCarrierResult
AFMCodexLocalMatchHostGameMode::ResolveNoLegalCarrier()
{
	using namespace FMCodexLocalMatchHost;

	FFMCodexLocalMatchResolveNoLegalCarrierResult Result;
	if (!ActiveMatchRuntime.IsValid())
	{
		Result.ErrorCode = EFMCodexLocalMatchHostErrorCode::NoActiveMatch;
		Result.ErrorMessage = NoActiveMatchMessage;
		return Result;
	}

	Result.AuthoritativeResult =
		ActiveMatchRuntime->AuthoritativeSession.ResolveNoLegalCarrier();
	Result.bSuccess = Result.AuthoritativeResult.RuntimeEnvelope.bAccepted
		&& Result.AuthoritativeResult.RuntimeEnvelope.bDomainSuccess
		&& Result.AuthoritativeResult.ResolutionResult.bSuccess;
	if (!Result.bSuccess)
	{
		Result.ErrorCode =
			EFMCodexLocalMatchHostErrorCode::AuthoritativeCommandFailed;
		Result.ErrorMessage = SelectAuthoritativeErrorMessage(
			Result.AuthoritativeResult.RuntimeEnvelope,
			Result.AuthoritativeResult.ResolutionResult.ErrorMessage);
	}
	return Result;
}

FFMCodexLocalMatchSubmitMarkerResult
AFMCodexLocalMatchHostGameMode::SubmitMarker(
	const FMatchPlayAuthoritativeSubmitMarkerRequest& Request)
{
	using namespace FMCodexLocalMatchHost;

	FFMCodexLocalMatchSubmitMarkerResult Result;
	if (!ActiveMatchRuntime.IsValid())
	{
		Result.ErrorCode = EFMCodexLocalMatchHostErrorCode::NoActiveMatch;
		Result.ErrorMessage = NoActiveMatchMessage;
		return Result;
	}

	Result.AuthoritativeResult =
		ActiveMatchRuntime->AuthoritativeSession.SubmitMarker(Request);
	Result.bSuccess = Result.AuthoritativeResult.RuntimeEnvelope.bAccepted
		&& Result.AuthoritativeResult.RuntimeEnvelope.bDomainSuccess
		&& Result.AuthoritativeResult.MarkerResult.bSuccess;
	if (!Result.bSuccess)
	{
		Result.ErrorCode =
			EFMCodexLocalMatchHostErrorCode::AuthoritativeCommandFailed;
		Result.ErrorMessage = SelectAuthoritativeErrorMessage(
			Result.AuthoritativeResult.RuntimeEnvelope,
			Result.AuthoritativeResult.MarkerResult.ErrorMessage);
	}
	return Result;
}

FFMCodexLocalMatchResolveNoLegalMarkerResult
AFMCodexLocalMatchHostGameMode::ResolveNoLegalMarker()
{
	using namespace FMCodexLocalMatchHost;

	FFMCodexLocalMatchResolveNoLegalMarkerResult Result;
	if (!ActiveMatchRuntime.IsValid())
	{
		Result.ErrorCode = EFMCodexLocalMatchHostErrorCode::NoActiveMatch;
		Result.ErrorMessage = NoActiveMatchMessage;
		return Result;
	}

	Result.AuthoritativeResult =
		ActiveMatchRuntime->AuthoritativeSession.ResolveNoLegalMarker();
	Result.bSuccess = Result.AuthoritativeResult.RuntimeEnvelope.bAccepted
		&& Result.AuthoritativeResult.RuntimeEnvelope.bDomainSuccess
		&& Result.AuthoritativeResult.ResolutionResult.bSuccess;
	if (!Result.bSuccess)
	{
		Result.ErrorCode =
			EFMCodexLocalMatchHostErrorCode::AuthoritativeCommandFailed;
		Result.ErrorMessage = SelectAuthoritativeErrorMessage(
			Result.AuthoritativeResult.RuntimeEnvelope,
			Result.AuthoritativeResult.ResolutionResult.ErrorMessage);
	}
	return Result;
}

FFMCodexLocalMatchDeclineMarkerResult
AFMCodexLocalMatchHostGameMode::DeclineMarker(
	const FMatchPlayAuthoritativeDeclineMarkerRequest& Request)
{
	using namespace FMCodexLocalMatchHost;

	FFMCodexLocalMatchDeclineMarkerResult Result;
	if (!ActiveMatchRuntime.IsValid())
	{
		Result.ErrorCode = EFMCodexLocalMatchHostErrorCode::NoActiveMatch;
		Result.ErrorMessage = NoActiveMatchMessage;
		return Result;
	}

	Result.AuthoritativeResult =
		ActiveMatchRuntime->AuthoritativeSession.DeclineMarker(Request);
	Result.bSuccess = Result.AuthoritativeResult.RuntimeEnvelope.bAccepted
		&& Result.AuthoritativeResult.RuntimeEnvelope.bDomainSuccess
		&& Result.AuthoritativeResult.DeclineResult.bSuccess;
	if (!Result.bSuccess)
	{
		Result.ErrorCode =
			EFMCodexLocalMatchHostErrorCode::AuthoritativeCommandFailed;
		Result.ErrorMessage = SelectAuthoritativeErrorMessage(
			Result.AuthoritativeResult.RuntimeEnvelope,
			Result.AuthoritativeResult.DeclineResult.ErrorMessage);
	}
	return Result;
}

FFMCodexLocalMatchSubmitSkillResult
AFMCodexLocalMatchHostGameMode::SubmitSkill(
	const FSkillRuleSnapshotSet& SkillRuleSet,
	const FMatchPlayAuthoritativeSubmitSkillRequest& Request)
{
	using namespace FMCodexLocalMatchHost;

	FFMCodexLocalMatchSubmitSkillResult Result;
	if (!ActiveMatchRuntime.IsValid())
	{
		Result.ErrorCode = EFMCodexLocalMatchHostErrorCode::NoActiveMatch;
		Result.ErrorMessage = NoActiveMatchMessage;
		return Result;
	}
	if (!AreSkillRuleSetsEqual(
		SkillRuleSet,
		ActiveMatchRuntime->SkillRuleSet))
	{
		Result.ErrorCode =
			EFMCodexLocalMatchHostErrorCode::RuleConfigurationMismatch;
		Result.ErrorMessage = RuleConfigurationMismatchMessage;
		return Result;
	}

	Result.AuthoritativeResult =
		ActiveMatchRuntime->AuthoritativeSession.SubmitSkill(
			ActiveMatchRuntime->SkillRuleSet,
			Request);
	Result.bSuccess = Result.AuthoritativeResult.RuntimeEnvelope.bAccepted
		&& Result.AuthoritativeResult.RuntimeEnvelope.bDomainSuccess
		&& Result.AuthoritativeResult.SkillResult.bSuccess;
	if (!Result.bSuccess)
	{
		Result.ErrorCode =
			EFMCodexLocalMatchHostErrorCode::AuthoritativeCommandFailed;
		Result.ErrorMessage = SelectAuthoritativeErrorMessage(
			Result.AuthoritativeResult.RuntimeEnvelope,
			Result.AuthoritativeResult.SkillResult.ErrorMessage);
	}
	return Result;
}

FFMCodexLocalMatchResolveNoLegalSkillResult
AFMCodexLocalMatchHostGameMode::ResolveNoLegalSkill(
	const FSkillRuleSnapshotSet& SkillRuleSet)
{
	using namespace FMCodexLocalMatchHost;

	FFMCodexLocalMatchResolveNoLegalSkillResult Result;
	if (!ActiveMatchRuntime.IsValid())
	{
		Result.ErrorCode = EFMCodexLocalMatchHostErrorCode::NoActiveMatch;
		Result.ErrorMessage = NoActiveMatchMessage;
		return Result;
	}
	if (!AreSkillRuleSetsEqual(
		SkillRuleSet,
		ActiveMatchRuntime->SkillRuleSet))
	{
		Result.ErrorCode =
			EFMCodexLocalMatchHostErrorCode::RuleConfigurationMismatch;
		Result.ErrorMessage = RuleConfigurationMismatchMessage;
		return Result;
	}

	Result.AuthoritativeResult =
		ActiveMatchRuntime->AuthoritativeSession.ResolveNoLegalSkill(
			ActiveMatchRuntime->SkillRuleSet);
	Result.bSuccess = Result.AuthoritativeResult.RuntimeEnvelope.bAccepted
		&& Result.AuthoritativeResult.RuntimeEnvelope.bDomainSuccess
		&& Result.AuthoritativeResult.ResolutionResult.bSuccess;
	if (!Result.bSuccess)
	{
		Result.ErrorCode =
			EFMCodexLocalMatchHostErrorCode::AuthoritativeCommandFailed;
		Result.ErrorMessage = SelectAuthoritativeErrorMessage(
			Result.AuthoritativeResult.RuntimeEnvelope,
			Result.AuthoritativeResult.ResolutionResult.ErrorMessage);
	}
	return Result;
}

FFMCodexLocalMatchDeclineSkillResult
AFMCodexLocalMatchHostGameMode::DeclineSkill(
	const FSkillRuleSnapshotSet& SkillRuleSet,
	const FMatchPlayAuthoritativeDeclineSkillRequest& Request)
{
	using namespace FMCodexLocalMatchHost;

	FFMCodexLocalMatchDeclineSkillResult Result;
	if (!ActiveMatchRuntime.IsValid())
	{
		Result.ErrorCode = EFMCodexLocalMatchHostErrorCode::NoActiveMatch;
		Result.ErrorMessage = NoActiveMatchMessage;
		return Result;
	}
	if (!AreSkillRuleSetsEqual(
		SkillRuleSet,
		ActiveMatchRuntime->SkillRuleSet))
	{
		Result.ErrorCode =
			EFMCodexLocalMatchHostErrorCode::RuleConfigurationMismatch;
		Result.ErrorMessage = RuleConfigurationMismatchMessage;
		return Result;
	}

	Result.AuthoritativeResult =
		ActiveMatchRuntime->AuthoritativeSession.DeclineSkill(
			ActiveMatchRuntime->SkillRuleSet,
			Request);
	Result.bSuccess = Result.AuthoritativeResult.RuntimeEnvelope.bAccepted
		&& Result.AuthoritativeResult.RuntimeEnvelope.bDomainSuccess
		&& Result.AuthoritativeResult.DeclineResult.bSuccess;
	if (!Result.bSuccess)
	{
		Result.ErrorCode =
			EFMCodexLocalMatchHostErrorCode::AuthoritativeCommandFailed;
		Result.ErrorMessage = SelectAuthoritativeErrorMessage(
			Result.AuthoritativeResult.RuntimeEnvelope,
			Result.AuthoritativeResult.DeclineResult.ErrorMessage);
	}
	return Result;
}

FFMCodexLocalMatchSubmitRunnerResult
AFMCodexLocalMatchHostGameMode::SubmitRunner(
	const FMatchPlayAuthoritativeSubmitRunnerRequest& Request)
{
	using namespace FMCodexLocalMatchHost;

	FFMCodexLocalMatchSubmitRunnerResult Result;
	if (!ActiveMatchRuntime.IsValid())
	{
		Result.ErrorCode = EFMCodexLocalMatchHostErrorCode::NoActiveMatch;
		Result.ErrorMessage = NoActiveMatchMessage;
		return Result;
	}

	Result.AuthoritativeResult =
		ActiveMatchRuntime->AuthoritativeSession.SubmitRunner(Request);
	Result.bSuccess = Result.AuthoritativeResult.RuntimeEnvelope.bAccepted
		&& Result.AuthoritativeResult.RuntimeEnvelope.bDomainSuccess
		&& Result.AuthoritativeResult.RunnerResult.bSuccess;
	if (!Result.bSuccess)
	{
		Result.ErrorCode =
			EFMCodexLocalMatchHostErrorCode::AuthoritativeCommandFailed;
		Result.ErrorMessage = SelectAuthoritativeErrorMessage(
			Result.AuthoritativeResult.RuntimeEnvelope,
			Result.AuthoritativeResult.RunnerResult.ErrorMessage);
	}
	return Result;
}

FFMCodexLocalMatchResolveNoLegalRunnerResult
AFMCodexLocalMatchHostGameMode::ResolveNoLegalRunner()
{
	using namespace FMCodexLocalMatchHost;

	FFMCodexLocalMatchResolveNoLegalRunnerResult Result;
	if (!ActiveMatchRuntime.IsValid())
	{
		Result.ErrorCode = EFMCodexLocalMatchHostErrorCode::NoActiveMatch;
		Result.ErrorMessage = NoActiveMatchMessage;
		return Result;
	}

	Result.AuthoritativeResult =
		ActiveMatchRuntime->AuthoritativeSession.ResolveNoLegalRunner();
	Result.bSuccess = Result.AuthoritativeResult.RuntimeEnvelope.bAccepted
		&& Result.AuthoritativeResult.RuntimeEnvelope.bDomainSuccess
		&& Result.AuthoritativeResult.ResolutionResult.bSuccess;
	if (!Result.bSuccess)
	{
		Result.ErrorCode =
			EFMCodexLocalMatchHostErrorCode::AuthoritativeCommandFailed;
		Result.ErrorMessage = SelectAuthoritativeErrorMessage(
			Result.AuthoritativeResult.RuntimeEnvelope,
			Result.AuthoritativeResult.ResolutionResult.ErrorMessage);
	}
	return Result;
}

FFMCodexLocalMatchDeclineRunnerResult
AFMCodexLocalMatchHostGameMode::DeclineRunner(
	const FMatchPlayAuthoritativeDeclineRunnerRequest& Request)
{
	using namespace FMCodexLocalMatchHost;

	FFMCodexLocalMatchDeclineRunnerResult Result;
	if (!ActiveMatchRuntime.IsValid())
	{
		Result.ErrorCode = EFMCodexLocalMatchHostErrorCode::NoActiveMatch;
		Result.ErrorMessage = NoActiveMatchMessage;
		return Result;
	}

	Result.AuthoritativeResult =
		ActiveMatchRuntime->AuthoritativeSession.DeclineRunner(Request);
	Result.bSuccess = Result.AuthoritativeResult.RuntimeEnvelope.bAccepted
		&& Result.AuthoritativeResult.RuntimeEnvelope.bDomainSuccess
		&& Result.AuthoritativeResult.DeclineResult.bSuccess;
	if (!Result.bSuccess)
	{
		Result.ErrorCode =
			EFMCodexLocalMatchHostErrorCode::AuthoritativeCommandFailed;
		Result.ErrorMessage = SelectAuthoritativeErrorMessage(
			Result.AuthoritativeResult.RuntimeEnvelope,
			Result.AuthoritativeResult.DeclineResult.ErrorMessage);
	}
	return Result;
}

FFMCodexLocalMatchSubmitHelperResult
AFMCodexLocalMatchHostGameMode::SubmitHelper(
	const FMatchPlayAuthoritativeSubmitHelperRequest& Request)
{
	using namespace FMCodexLocalMatchHost;

	FFMCodexLocalMatchSubmitHelperResult Result;
	if (!ActiveMatchRuntime.IsValid())
	{
		Result.ErrorCode = EFMCodexLocalMatchHostErrorCode::NoActiveMatch;
		Result.ErrorMessage = NoActiveMatchMessage;
		return Result;
	}

	Result.AuthoritativeResult =
		ActiveMatchRuntime->AuthoritativeSession.SubmitHelper(Request);
	Result.bSuccess = Result.AuthoritativeResult.RuntimeEnvelope.bAccepted
		&& Result.AuthoritativeResult.RuntimeEnvelope.bDomainSuccess
		&& Result.AuthoritativeResult.HelperResult.bSuccess;
	if (!Result.bSuccess)
	{
		Result.ErrorCode =
			EFMCodexLocalMatchHostErrorCode::AuthoritativeCommandFailed;
		Result.ErrorMessage = SelectAuthoritativeErrorMessage(
			Result.AuthoritativeResult.RuntimeEnvelope,
			Result.AuthoritativeResult.HelperResult.ErrorMessage);
	}
	return Result;
}

FFMCodexLocalMatchResolveNoLegalHelperResult
AFMCodexLocalMatchHostGameMode::ResolveNoLegalHelper()
{
	using namespace FMCodexLocalMatchHost;

	FFMCodexLocalMatchResolveNoLegalHelperResult Result;
	if (!ActiveMatchRuntime.IsValid())
	{
		Result.ErrorCode = EFMCodexLocalMatchHostErrorCode::NoActiveMatch;
		Result.ErrorMessage = NoActiveMatchMessage;
		return Result;
	}

	Result.AuthoritativeResult =
		ActiveMatchRuntime->AuthoritativeSession.ResolveNoLegalHelper();
	Result.bSuccess = Result.AuthoritativeResult.RuntimeEnvelope.bAccepted
		&& Result.AuthoritativeResult.RuntimeEnvelope.bDomainSuccess
		&& Result.AuthoritativeResult.ResolutionResult.bSuccess;
	if (!Result.bSuccess)
	{
		Result.ErrorCode =
			EFMCodexLocalMatchHostErrorCode::AuthoritativeCommandFailed;
		Result.ErrorMessage = SelectAuthoritativeErrorMessage(
			Result.AuthoritativeResult.RuntimeEnvelope,
			Result.AuthoritativeResult.ResolutionResult.ErrorMessage);
	}
	return Result;
}

FFMCodexLocalMatchDeclineHelperResult
AFMCodexLocalMatchHostGameMode::DeclineHelper(
	const FMatchPlayAuthoritativeDeclineHelperRequest& Request)
{
	using namespace FMCodexLocalMatchHost;

	FFMCodexLocalMatchDeclineHelperResult Result;
	if (!ActiveMatchRuntime.IsValid())
	{
		Result.ErrorCode = EFMCodexLocalMatchHostErrorCode::NoActiveMatch;
		Result.ErrorMessage = NoActiveMatchMessage;
		return Result;
	}

	Result.AuthoritativeResult =
		ActiveMatchRuntime->AuthoritativeSession.DeclineHelper(Request);
	Result.bSuccess = Result.AuthoritativeResult.RuntimeEnvelope.bAccepted
		&& Result.AuthoritativeResult.RuntimeEnvelope.bDomainSuccess
		&& Result.AuthoritativeResult.DeclineResult.bSuccess;
	if (!Result.bSuccess)
	{
		Result.ErrorCode =
			EFMCodexLocalMatchHostErrorCode::AuthoritativeCommandFailed;
		Result.ErrorMessage = SelectAuthoritativeErrorMessage(
			Result.AuthoritativeResult.RuntimeEnvelope,
			Result.AuthoritativeResult.DeclineResult.ErrorMessage);
	}
	return Result;
}

FFMCodexLocalMatchSubmitBranchIntentResult
AFMCodexLocalMatchHostGameMode::SubmitBranchIntent(
	const FMatchPlayAuthoritativeSubmitBranchIntentRequest& Request)
{
	using namespace FMCodexLocalMatchHost;

	FFMCodexLocalMatchSubmitBranchIntentResult Result;
	if (!ActiveMatchRuntime.IsValid())
	{
		Result.ErrorCode = EFMCodexLocalMatchHostErrorCode::NoActiveMatch;
		Result.ErrorMessage = NoActiveMatchMessage;
		return Result;
	}

	Result.AuthoritativeResult =
		ActiveMatchRuntime->AuthoritativeSession.SubmitBranchIntent(Request);
	Result.bSuccess = Result.AuthoritativeResult.RuntimeEnvelope.bAccepted
		&& Result.AuthoritativeResult.RuntimeEnvelope.bDomainSuccess
		&& Result.AuthoritativeResult.IntentResult.bSuccess;
	if (!Result.bSuccess)
	{
		Result.ErrorCode =
			EFMCodexLocalMatchHostErrorCode::AuthoritativeCommandFailed;
		Result.ErrorMessage = SelectAuthoritativeErrorMessage(
			Result.AuthoritativeResult.RuntimeEnvelope,
			Result.AuthoritativeResult.IntentResult.ErrorMessage);
	}
	return Result;
}

FFMCodexLocalMatchSubmitThroughBallOneOnOneShotChoiceResult
AFMCodexLocalMatchHostGameMode::SubmitThroughBallOneOnOneShotChoice(
	const FMatchPlayAuthoritativeSubmitThroughBallOneOnOneShotChoiceRequest&
		Request)
{
	using namespace FMCodexLocalMatchHost;

	FFMCodexLocalMatchSubmitThroughBallOneOnOneShotChoiceResult Result;
	if (!ActiveMatchRuntime.IsValid())
	{
		Result.ErrorCode = EFMCodexLocalMatchHostErrorCode::NoActiveMatch;
		Result.ErrorMessage = NoActiveMatchMessage;
		return Result;
	}

	Result.AuthoritativeResult = ActiveMatchRuntime->AuthoritativeSession
		.SubmitThroughBallOneOnOneShotChoice(Request);
	Result.bSuccess = Result.AuthoritativeResult.RuntimeEnvelope.bAccepted
		&& Result.AuthoritativeResult.RuntimeEnvelope.bDomainSuccess
		&& Result.AuthoritativeResult.ChoiceResult.bSuccess;
	if (!Result.bSuccess)
	{
		Result.ErrorCode =
			EFMCodexLocalMatchHostErrorCode::AuthoritativeCommandFailed;
		Result.ErrorMessage = SelectAuthoritativeErrorMessage(
			Result.AuthoritativeResult.RuntimeEnvelope,
			Result.AuthoritativeResult.ChoiceResult.ErrorMessage);
	}
	return Result;
}

FFMCodexLocalMatchBeginResolutionSessionResult
AFMCodexLocalMatchHostGameMode::BeginResolutionSession()
{
	using namespace FMCodexLocalMatchHost;
	FFMCodexLocalMatchBeginResolutionSessionResult Result;
	if (!ActiveMatchRuntime.IsValid())
	{
		Result.ErrorCode = EFMCodexLocalMatchHostErrorCode::NoActiveMatch;
		Result.ErrorMessage = NoActiveMatchMessage;
		return Result;
	}
	Result.AuthoritativeResult = ActiveMatchRuntime->AuthoritativeSession
		.BeginResolutionSession();
	Result.bSuccess = Result.AuthoritativeResult.RuntimeEnvelope.bAccepted
		&& Result.AuthoritativeResult.RuntimeEnvelope.bDomainSuccess
		&& Result.AuthoritativeResult.BeginResult.bSuccess;
	if (!Result.bSuccess)
	{
		Result.ErrorCode =
			EFMCodexLocalMatchHostErrorCode::AuthoritativeCommandFailed;
		Result.ErrorMessage = SelectAuthoritativeErrorMessage(
			Result.AuthoritativeResult.RuntimeEnvelope,
			Result.AuthoritativeResult.BeginResult.ErrorMessage);
	}
	return Result;
}

FFMCodexLocalMatchResolveIntentDeterminedRouteResult
AFMCodexLocalMatchHostGameMode::ResolveIntentDeterminedRoute()
{
	using namespace FMCodexLocalMatchHost;
	FFMCodexLocalMatchResolveIntentDeterminedRouteResult Result;
	if (!ActiveMatchRuntime.IsValid())
	{
		Result.ErrorCode = EFMCodexLocalMatchHostErrorCode::NoActiveMatch;
		Result.ErrorMessage = NoActiveMatchMessage;
		return Result;
	}
	Result.AuthoritativeResult = ActiveMatchRuntime->AuthoritativeSession
		.ResolveIntentDeterminedRoute();
	Result.bSuccess = Result.AuthoritativeResult.RuntimeEnvelope.bAccepted
		&& Result.AuthoritativeResult.RuntimeEnvelope.bDomainSuccess
		&& Result.AuthoritativeResult.RouteResult.bSuccess;
	if (!Result.bSuccess)
	{
		Result.ErrorCode =
			EFMCodexLocalMatchHostErrorCode::AuthoritativeCommandFailed;
		Result.ErrorMessage = SelectAuthoritativeErrorMessage(
			Result.AuthoritativeResult.RuntimeEnvelope,
			Result.AuthoritativeResult.RouteResult.ErrorMessage);
	}
	return Result;
}

FFMCodexLocalMatchResolveInitialRouteResult
AFMCodexLocalMatchHostGameMode::ResolveInitialRoute()
{
	using namespace FMCodexLocalMatchHost;
	FFMCodexLocalMatchResolveInitialRouteResult Result;
	if (!ActiveMatchRuntime.IsValid())
	{
		Result.ErrorCode = EFMCodexLocalMatchHostErrorCode::NoActiveMatch;
		Result.ErrorMessage = NoActiveMatchMessage;
		return Result;
	}
#if !UE_BUILD_SHIPPING
	const EFMCodexLocalDevRollInvocation DevInvocation =
		InitialRouteInvocation(
			ActiveMatchRuntime->AuthoritativeSession.GetStateSnapshot());
#endif
	Result.AuthoritativeResult = ActiveMatchRuntime->ExecuteProviderCall(
#if !UE_BUILD_SHIPPING
		DevInvocation,
#endif
		[this]()
		{
			return ActiveMatchRuntime->AuthoritativeSession.ResolveInitialRoute();
		});
	Result.bSuccess = Result.AuthoritativeResult.RuntimeEnvelope.bAccepted
		&& Result.AuthoritativeResult.RuntimeEnvelope.bDomainSuccess
		&& Result.AuthoritativeResult.OrchestrationResult.bSuccess;
	if (!Result.bSuccess)
	{
		Result.ErrorCode =
			EFMCodexLocalMatchHostErrorCode::AuthoritativeCommandFailed;
		Result.ErrorMessage = SelectAuthoritativeErrorMessage(
			Result.AuthoritativeResult.RuntimeEnvelope,
			Result.AuthoritativeResult.OrchestrationResult.ErrorMessage);
	}
	return Result;
}

FFMCodexLocalMatchResolveThroughBallInitialRouteRollResult
AFMCodexLocalMatchHostGameMode::ResolveThroughBallInitialRouteRoll(
	const FMatchPlayAuthoritativeResolveThroughBallInitialRouteRollRequest&
		Request)
{
	using namespace FMCodexLocalMatchHost;
	FFMCodexLocalMatchResolveThroughBallInitialRouteRollResult Result;
	if (!ActiveMatchRuntime.IsValid())
	{
		Result.ErrorCode = EFMCodexLocalMatchHostErrorCode::NoActiveMatch;
		Result.ErrorMessage = NoActiveMatchMessage;
		return Result;
	}
	Result.AuthoritativeResult = ActiveMatchRuntime->ExecuteProviderCall(
#if !UE_BUILD_SHIPPING
		EFMCodexLocalDevRollInvocation::ThroughBallInitialRoute,
#endif
		[this, &Request]()
		{
			return ActiveMatchRuntime->AuthoritativeSession
				.ResolveThroughBallInitialRouteRoll(Request);
		});
	Result.bSuccess = Result.AuthoritativeResult.RuntimeEnvelope.bAccepted
		&& Result.AuthoritativeResult.RuntimeEnvelope.bDomainSuccess
		&& Result.AuthoritativeResult.OrchestrationResult.bSuccess;
	if (!Result.bSuccess)
	{
		Result.ErrorCode =
			EFMCodexLocalMatchHostErrorCode::AuthoritativeCommandFailed;
		const FString DomainMessage =
			!Result.AuthoritativeResult.ErrorMessage.IsEmpty()
				? Result.AuthoritativeResult.ErrorMessage
				: Result.AuthoritativeResult.OrchestrationResult.ErrorMessage;
		Result.ErrorMessage = SelectAuthoritativeErrorMessage(
			Result.AuthoritativeResult.RuntimeEnvelope,
			DomainMessage);
	}
	return Result;
}

FFMCodexLocalMatchResolveCrossPostRoutePlanResult
AFMCodexLocalMatchHostGameMode::ResolveCrossPostRoutePlan()
{
	using namespace FMCodexLocalMatchHost;
	FFMCodexLocalMatchResolveCrossPostRoutePlanResult Result;
	if (!ActiveMatchRuntime.IsValid())
	{
		Result.ErrorCode = EFMCodexLocalMatchHostErrorCode::NoActiveMatch;
		Result.ErrorMessage = NoActiveMatchMessage;
		return Result;
	}
	Result.AuthoritativeResult = ActiveMatchRuntime->AuthoritativeSession
		.ResolveCrossPostRoutePlan();
	Result.bSuccess = Result.AuthoritativeResult.RuntimeEnvelope.bAccepted
		&& Result.AuthoritativeResult.RuntimeEnvelope.bDomainSuccess
		&& Result.AuthoritativeResult.OrchestrationResult.bSuccess;
	if (!Result.bSuccess)
	{
		Result.ErrorCode =
			EFMCodexLocalMatchHostErrorCode::AuthoritativeCommandFailed;
		Result.ErrorMessage = SelectAuthoritativeErrorMessage(
			Result.AuthoritativeResult.RuntimeEnvelope,
			Result.AuthoritativeResult.OrchestrationResult.ErrorMessage);
	}
	return Result;
}

FFMCodexLocalMatchResolveCrossHighAttackRollResult
AFMCodexLocalMatchHostGameMode::ResolveCrossHighAttackRoll(
	const FMatchPlayAuthoritativeResolveCrossHighAttackRollRequest& Request)
{
	using namespace FMCodexLocalMatchHost;
	FFMCodexLocalMatchResolveCrossHighAttackRollResult Result;
	if (!ActiveMatchRuntime.IsValid())
	{
		Result.ErrorCode = EFMCodexLocalMatchHostErrorCode::NoActiveMatch;
		Result.ErrorMessage = NoActiveMatchMessage;
		return Result;
	}
#if !UE_BUILD_SHIPPING
	const auto DevInvocation = EFMCodexLocalDevRollInvocation::CrossHighAttack;
#endif
	Result.AuthoritativeResult = ActiveMatchRuntime->ExecuteProviderCall(
#if !UE_BUILD_SHIPPING
		DevInvocation,
#endif
		[this, &Request]()
		{
			return ActiveMatchRuntime->AuthoritativeSession
				.ResolveCrossHighAttackRoll(Request);
		});
	Result.bSuccess = Result.AuthoritativeResult.RuntimeEnvelope.bAccepted
		&& Result.AuthoritativeResult.RuntimeEnvelope.bDomainSuccess
		&& Result.AuthoritativeResult.OrchestrationResult.bSuccess;
	if (!Result.bSuccess)
	{
		Result.ErrorCode =
			EFMCodexLocalMatchHostErrorCode::AuthoritativeCommandFailed;
		Result.ErrorMessage = SelectAuthoritativeErrorMessage(
			Result.AuthoritativeResult.RuntimeEnvelope,
			Result.AuthoritativeResult.OrchestrationResult.ErrorMessage);
	}
	return Result;
}

FFMCodexLocalMatchResolveCrossHighDefenseRollResult
AFMCodexLocalMatchHostGameMode::ResolveCrossHighDefenseRoll(
	const FMatchPlayAuthoritativeResolveCrossHighDefenseRollRequest& Request)
{
	using namespace FMCodexLocalMatchHost;
	FFMCodexLocalMatchResolveCrossHighDefenseRollResult Result;
	if (!ActiveMatchRuntime.IsValid())
	{
		Result.ErrorCode = EFMCodexLocalMatchHostErrorCode::NoActiveMatch;
		Result.ErrorMessage = NoActiveMatchMessage;
		return Result;
	}
#if !UE_BUILD_SHIPPING
	const auto DevInvocation = EFMCodexLocalDevRollInvocation::CrossHighDefense;
#endif
	Result.AuthoritativeResult = ActiveMatchRuntime->ExecuteProviderCall(
#if !UE_BUILD_SHIPPING
		DevInvocation,
#endif
		[this, &Request]()
		{
			return ActiveMatchRuntime->AuthoritativeSession
				.ResolveCrossHighDefenseRoll(Request);
		});
	Result.bSuccess = Result.AuthoritativeResult.RuntimeEnvelope.bAccepted
		&& Result.AuthoritativeResult.RuntimeEnvelope.bDomainSuccess
		&& Result.AuthoritativeResult.OrchestrationResult.bSuccess;
	if (!Result.bSuccess)
	{
		Result.ErrorCode =
			EFMCodexLocalMatchHostErrorCode::AuthoritativeCommandFailed;
		Result.ErrorMessage = SelectAuthoritativeErrorMessage(
			Result.AuthoritativeResult.RuntimeEnvelope,
			Result.AuthoritativeResult.OrchestrationResult.ErrorMessage);
	}
	return Result;
}

FFMCodexLocalMatchResolveCrossLowAttackRollResult
AFMCodexLocalMatchHostGameMode::ResolveCrossLowAttackRoll(
	const FMatchPlayAuthoritativeResolveCrossLowAttackRollRequest& Request)
{
	using namespace FMCodexLocalMatchHost;
	FFMCodexLocalMatchResolveCrossLowAttackRollResult Result;
	if (!ActiveMatchRuntime.IsValid())
	{
		Result.ErrorCode = EFMCodexLocalMatchHostErrorCode::NoActiveMatch;
		Result.ErrorMessage = NoActiveMatchMessage;
		return Result;
	}
#if !UE_BUILD_SHIPPING
	const auto DevInvocation = EFMCodexLocalDevRollInvocation::CrossLowAttack;
#endif
	Result.AuthoritativeResult = ActiveMatchRuntime->ExecuteProviderCall(
#if !UE_BUILD_SHIPPING
		DevInvocation,
#endif
		[this, &Request]()
		{
			return ActiveMatchRuntime->AuthoritativeSession
				.ResolveCrossLowAttackRoll(Request);
		});
	Result.bSuccess = Result.AuthoritativeResult.RuntimeEnvelope.bAccepted
		&& Result.AuthoritativeResult.RuntimeEnvelope.bDomainSuccess
		&& Result.AuthoritativeResult.OrchestrationResult.bSuccess;
	if (!Result.bSuccess)
	{
		Result.ErrorCode =
			EFMCodexLocalMatchHostErrorCode::AuthoritativeCommandFailed;
		Result.ErrorMessage = SelectAuthoritativeErrorMessage(
			Result.AuthoritativeResult.RuntimeEnvelope,
			Result.AuthoritativeResult.OrchestrationResult.ErrorMessage);
	}
	return Result;
}

FFMCodexLocalMatchResolveCrossLowDefenseRollResult
AFMCodexLocalMatchHostGameMode::ResolveCrossLowDefenseRoll(
	const FMatchPlayAuthoritativeResolveCrossLowDefenseRollRequest& Request)
{
	using namespace FMCodexLocalMatchHost;
	FFMCodexLocalMatchResolveCrossLowDefenseRollResult Result;
	if (!ActiveMatchRuntime.IsValid())
	{
		Result.ErrorCode = EFMCodexLocalMatchHostErrorCode::NoActiveMatch;
		Result.ErrorMessage = NoActiveMatchMessage;
		return Result;
	}
#if !UE_BUILD_SHIPPING
	const auto DevInvocation = EFMCodexLocalDevRollInvocation::CrossLowDefense;
#endif
	Result.AuthoritativeResult = ActiveMatchRuntime->ExecuteProviderCall(
#if !UE_BUILD_SHIPPING
		DevInvocation,
#endif
		[this, &Request]()
		{
			return ActiveMatchRuntime->AuthoritativeSession
				.ResolveCrossLowDefenseRoll(Request);
		});
	Result.bSuccess = Result.AuthoritativeResult.RuntimeEnvelope.bAccepted
		&& Result.AuthoritativeResult.RuntimeEnvelope.bDomainSuccess
		&& Result.AuthoritativeResult.OrchestrationResult.bSuccess;
	if (!Result.bSuccess)
	{
		Result.ErrorCode =
			EFMCodexLocalMatchHostErrorCode::AuthoritativeCommandFailed;
		Result.ErrorMessage = SelectAuthoritativeErrorMessage(
			Result.AuthoritativeResult.RuntimeEnvelope,
			Result.AuthoritativeResult.OrchestrationResult.ErrorMessage);
	}
	return Result;
}

FFMCodexLocalMatchResolveThroughBallFeetPostRoutePlanResult
AFMCodexLocalMatchHostGameMode::ResolveThroughBallFeetPostRoutePlan()
{
	using namespace FMCodexLocalMatchHost;
	FFMCodexLocalMatchResolveThroughBallFeetPostRoutePlanResult Result;
	if (!ActiveMatchRuntime.IsValid())
	{
		Result.ErrorCode = EFMCodexLocalMatchHostErrorCode::NoActiveMatch;
		Result.ErrorMessage = NoActiveMatchMessage;
		return Result;
	}
	Result.AuthoritativeResult = ActiveMatchRuntime->AuthoritativeSession
		.ResolveThroughBallFeetPostRoutePlan();
	Result.bSuccess = Result.AuthoritativeResult.RuntimeEnvelope.bAccepted
		&& Result.AuthoritativeResult.RuntimeEnvelope.bDomainSuccess
		&& Result.AuthoritativeResult.OrchestrationResult.bSuccess;
	if (!Result.bSuccess)
	{
		Result.ErrorCode =
			EFMCodexLocalMatchHostErrorCode::AuthoritativeCommandFailed;
		Result.ErrorMessage = SelectAuthoritativeErrorMessage(
			Result.AuthoritativeResult.RuntimeEnvelope,
			Result.AuthoritativeResult.OrchestrationResult.ErrorMessage);
	}
	return Result;
}

FFMCodexLocalMatchResolveThroughBallFeetAttackRollResult
AFMCodexLocalMatchHostGameMode::ResolveThroughBallFeetAttackRoll(
	const FMatchPlayAuthoritativeResolveThroughBallFeetAttackRollRequest& Request)
{
	using namespace FMCodexLocalMatchHost;
	FFMCodexLocalMatchResolveThroughBallFeetAttackRollResult Result;
	if (!ActiveMatchRuntime.IsValid())
	{
		Result.ErrorCode = EFMCodexLocalMatchHostErrorCode::NoActiveMatch;
		Result.ErrorMessage = NoActiveMatchMessage;
		return Result;
	}
#if !UE_BUILD_SHIPPING
	const auto DevInvocation =
		EFMCodexLocalDevRollInvocation::ThroughBallFeetAttack;
#endif
	Result.AuthoritativeResult = ActiveMatchRuntime->ExecuteProviderCall(
#if !UE_BUILD_SHIPPING
		DevInvocation,
#endif
		[this, &Request]()
		{
			return ActiveMatchRuntime->AuthoritativeSession
				.ResolveThroughBallFeetAttackRoll(Request);
		});
	Result.bSuccess = Result.AuthoritativeResult.RuntimeEnvelope.bAccepted
		&& Result.AuthoritativeResult.RuntimeEnvelope.bDomainSuccess
		&& Result.AuthoritativeResult.OrchestrationResult.bSuccess;
	if (!Result.bSuccess)
	{
		Result.ErrorCode =
			EFMCodexLocalMatchHostErrorCode::AuthoritativeCommandFailed;
		Result.ErrorMessage = SelectAuthoritativeErrorMessage(
			Result.AuthoritativeResult.RuntimeEnvelope,
			Result.AuthoritativeResult.OrchestrationResult.ErrorMessage);
	}
	return Result;
}

FFMCodexLocalMatchResolveThroughBallFeetDefenseRollResult
AFMCodexLocalMatchHostGameMode::ResolveThroughBallFeetDefenseRoll(
	const FMatchPlayAuthoritativeResolveThroughBallFeetDefenseRollRequest& Request)
{
	using namespace FMCodexLocalMatchHost;
	FFMCodexLocalMatchResolveThroughBallFeetDefenseRollResult Result;
	if (!ActiveMatchRuntime.IsValid())
	{
		Result.ErrorCode = EFMCodexLocalMatchHostErrorCode::NoActiveMatch;
		Result.ErrorMessage = NoActiveMatchMessage;
		return Result;
	}
#if !UE_BUILD_SHIPPING
	const auto DevInvocation =
		EFMCodexLocalDevRollInvocation::ThroughBallFeetDefense;
#endif
	Result.AuthoritativeResult = ActiveMatchRuntime->ExecuteProviderCall(
#if !UE_BUILD_SHIPPING
		DevInvocation,
#endif
		[this, &Request]()
		{
			return ActiveMatchRuntime->AuthoritativeSession
				.ResolveThroughBallFeetDefenseRoll(Request);
		});
	Result.bSuccess = Result.AuthoritativeResult.RuntimeEnvelope.bAccepted
		&& Result.AuthoritativeResult.RuntimeEnvelope.bDomainSuccess
		&& Result.AuthoritativeResult.OrchestrationResult.bSuccess;
	if (!Result.bSuccess)
	{
		Result.ErrorCode =
			EFMCodexLocalMatchHostErrorCode::AuthoritativeCommandFailed;
		Result.ErrorMessage = SelectAuthoritativeErrorMessage(
			Result.AuthoritativeResult.RuntimeEnvelope,
			Result.AuthoritativeResult.OrchestrationResult.ErrorMessage);
	}
	return Result;
}

FFMCodexLocalMatchResolvePassControlPostRoutePlanResult
AFMCodexLocalMatchHostGameMode::ResolvePassControlPostRoutePlan()
{
	using namespace FMCodexLocalMatchHost;
	FFMCodexLocalMatchResolvePassControlPostRoutePlanResult Result;
	if (!ActiveMatchRuntime.IsValid())
	{
		Result.ErrorCode = EFMCodexLocalMatchHostErrorCode::NoActiveMatch;
		Result.ErrorMessage = NoActiveMatchMessage;
		return Result;
	}
	Result.AuthoritativeResult = ActiveMatchRuntime->AuthoritativeSession
		.ResolvePassControlPostRoutePlan();
	Result.bSuccess = Result.AuthoritativeResult.RuntimeEnvelope.bAccepted
		&& Result.AuthoritativeResult.RuntimeEnvelope.bDomainSuccess
		&& Result.AuthoritativeResult.OrchestrationResult.bSuccess;
	if (!Result.bSuccess)
	{
		Result.ErrorCode =
			EFMCodexLocalMatchHostErrorCode::AuthoritativeCommandFailed;
		Result.ErrorMessage = SelectAuthoritativeErrorMessage(
			Result.AuthoritativeResult.RuntimeEnvelope,
			Result.AuthoritativeResult.OrchestrationResult.ErrorMessage);
	}
	return Result;
}

FFMCodexLocalMatchResolveDeadCornerPostRouteDecisionResult
AFMCodexLocalMatchHostGameMode::ResolveDeadCornerPostRouteDecision()
{
	using namespace FMCodexLocalMatchHost;
	FFMCodexLocalMatchResolveDeadCornerPostRouteDecisionResult Result;
	if (!ActiveMatchRuntime.IsValid())
	{
		Result.ErrorCode = EFMCodexLocalMatchHostErrorCode::NoActiveMatch;
		Result.ErrorMessage = NoActiveMatchMessage;
		return Result;
	}
	Result.AuthoritativeResult = ActiveMatchRuntime->AuthoritativeSession
		.ResolveDeadCornerPostRouteDecision();
	Result.bSuccess = Result.AuthoritativeResult.RuntimeEnvelope.bAccepted
		&& Result.AuthoritativeResult.RuntimeEnvelope.bDomainSuccess
		&& Result.AuthoritativeResult.OrchestrationResult.bSuccess;
	if (!Result.bSuccess)
	{
		Result.ErrorCode =
			EFMCodexLocalMatchHostErrorCode::AuthoritativeCommandFailed;
		Result.ErrorMessage = SelectAuthoritativeErrorMessage(
			Result.AuthoritativeResult.RuntimeEnvelope,
			Result.AuthoritativeResult.OrchestrationResult.ErrorMessage);
	}
	return Result;
}

FFMCodexLocalMatchResolveLongShotDeadCornerRollResult
AFMCodexLocalMatchHostGameMode::ResolveLongShotDeadCornerRoll(
	const FMatchPlayAuthoritativeResolveLongShotDeadCornerRollRequest& Request)
{
	using namespace FMCodexLocalMatchHost;
	FFMCodexLocalMatchResolveLongShotDeadCornerRollResult Result;
	if (!ActiveMatchRuntime.IsValid())
	{
		Result.ErrorCode = EFMCodexLocalMatchHostErrorCode::NoActiveMatch;
		Result.ErrorMessage = NoActiveMatchMessage;
		return Result;
	}
#if !UE_BUILD_SHIPPING
	const auto DevInvocation = EFMCodexLocalDevRollInvocation::LongShotDeadCorner;
#endif
	Result.AuthoritativeResult = ActiveMatchRuntime->ExecuteProviderCall(
#if !UE_BUILD_SHIPPING
		DevInvocation,
#endif
		[this, &Request]()
		{
			return ActiveMatchRuntime->AuthoritativeSession
				.ResolveLongShotDeadCornerRoll(Request);
		});
	Result.bSuccess = Result.AuthoritativeResult.RuntimeEnvelope.bAccepted
		&& Result.AuthoritativeResult.RuntimeEnvelope.bDomainSuccess
		&& Result.AuthoritativeResult.OrchestrationResult.bSuccess;
	if (!Result.bSuccess)
	{
		Result.ErrorCode =
			EFMCodexLocalMatchHostErrorCode::AuthoritativeCommandFailed;
		Result.ErrorMessage = SelectAuthoritativeErrorMessage(
			Result.AuthoritativeResult.RuntimeEnvelope,
			Result.AuthoritativeResult.OrchestrationResult.ErrorMessage);
	}
	return Result;
}

FFMCodexLocalMatchResolveThroughBallAntiOffsideDecisionResult
AFMCodexLocalMatchHostGameMode::ResolveThroughBallAntiOffsideDecision()
{
	using namespace FMCodexLocalMatchHost;
	FFMCodexLocalMatchResolveThroughBallAntiOffsideDecisionResult Result;
	if (!ActiveMatchRuntime.IsValid())
	{
		Result.ErrorCode = EFMCodexLocalMatchHostErrorCode::NoActiveMatch;
		Result.ErrorMessage = NoActiveMatchMessage;
		return Result;
	}
#if !UE_BUILD_SHIPPING
	const auto DevInvocation =
		EFMCodexLocalDevRollInvocation::ThroughBallAntiOffside;
#endif
	Result.AuthoritativeResult = ActiveMatchRuntime->ExecuteProviderCall(
#if !UE_BUILD_SHIPPING
		DevInvocation,
#endif
		[this]()
		{
			return ActiveMatchRuntime->AuthoritativeSession
				.ResolveThroughBallAntiOffsideDecision();
		});
	Result.bSuccess = Result.AuthoritativeResult.RuntimeEnvelope.bAccepted
		&& Result.AuthoritativeResult.RuntimeEnvelope.bDomainSuccess
		&& Result.AuthoritativeResult.OrchestrationResult.bSuccess;
	if (!Result.bSuccess)
	{
		Result.ErrorCode =
			EFMCodexLocalMatchHostErrorCode::AuthoritativeCommandFailed;
		Result.ErrorMessage = SelectAuthoritativeErrorMessage(
			Result.AuthoritativeResult.RuntimeEnvelope,
			Result.AuthoritativeResult.OrchestrationResult.ErrorMessage);
	}
	return Result;
}

FFMCodexLocalMatchResolveThroughBallAntiOffsideAttackRollResult
AFMCodexLocalMatchHostGameMode::ResolveThroughBallAntiOffsideAttackRoll(
	const FMatchPlayAuthoritativeResolveThroughBallAntiOffsideAttackRollRequest&
		Request)
{
	using namespace FMCodexLocalMatchHost;
	FFMCodexLocalMatchResolveThroughBallAntiOffsideAttackRollResult Result;
	if (!ActiveMatchRuntime.IsValid())
	{
		Result.ErrorCode = EFMCodexLocalMatchHostErrorCode::NoActiveMatch;
		Result.ErrorMessage = NoActiveMatchMessage;
		return Result;
	}
#if !UE_BUILD_SHIPPING
	const auto DevInvocation =
		EFMCodexLocalDevRollInvocation::ThroughBallAntiOffside;
#endif
	Result.AuthoritativeResult = ActiveMatchRuntime->ExecuteProviderCall(
#if !UE_BUILD_SHIPPING
		DevInvocation,
#endif
		[this, &Request]()
		{
			return ActiveMatchRuntime->AuthoritativeSession
				.ResolveThroughBallAntiOffsideAttackRoll(Request);
		});
	Result.bSuccess = Result.AuthoritativeResult.RuntimeEnvelope.bAccepted
		&& Result.AuthoritativeResult.RuntimeEnvelope.bDomainSuccess
		&& Result.AuthoritativeResult.OrchestrationResult.bSuccess;
	if (!Result.bSuccess)
	{
		Result.ErrorCode =
			EFMCodexLocalMatchHostErrorCode::AuthoritativeCommandFailed;
		Result.ErrorMessage = SelectAuthoritativeErrorMessage(
			Result.AuthoritativeResult.RuntimeEnvelope,
			Result.AuthoritativeResult.OrchestrationResult.ErrorMessage);
	}
	return Result;
}

FFMCodexLocalMatchResolveDirectShotPostRouteDecisionOrPlanResult
AFMCodexLocalMatchHostGameMode::ResolveDirectShotPostRouteDecisionOrPlan()
{
	using namespace FMCodexLocalMatchHost;
	FFMCodexLocalMatchResolveDirectShotPostRouteDecisionOrPlanResult Result;
	if (!ActiveMatchRuntime.IsValid())
	{
		Result.ErrorCode = EFMCodexLocalMatchHostErrorCode::NoActiveMatch;
		Result.ErrorMessage = NoActiveMatchMessage;
		return Result;
	}
	Result.AuthoritativeResult = ActiveMatchRuntime->AuthoritativeSession
		.ResolveDirectShotPostRouteDecisionOrPlan();
	Result.bSuccess = Result.AuthoritativeResult.RuntimeEnvelope.bAccepted
		&& Result.AuthoritativeResult.RuntimeEnvelope.bDomainSuccess
		&& Result.AuthoritativeResult.OrchestrationResult.bSuccess;
	if (!Result.bSuccess)
	{
		Result.ErrorCode =
			EFMCodexLocalMatchHostErrorCode::AuthoritativeCommandFailed;
		Result.ErrorMessage = SelectAuthoritativeErrorMessage(
			Result.AuthoritativeResult.RuntimeEnvelope,
			Result.AuthoritativeResult.OrchestrationResult.ErrorMessage);
	}
	return Result;
}

FFMCodexLocalMatchResolveLongShotDirectAttackRollResult
AFMCodexLocalMatchHostGameMode::ResolveLongShotDirectAttackRoll(
	const FMatchPlayAuthoritativeResolveLongShotDirectAttackRollRequest& Request)
{
	using namespace FMCodexLocalMatchHost;
	FFMCodexLocalMatchResolveLongShotDirectAttackRollResult Result;
	if (!ActiveMatchRuntime.IsValid())
	{
		Result.ErrorCode = EFMCodexLocalMatchHostErrorCode::NoActiveMatch;
		Result.ErrorMessage = NoActiveMatchMessage;
		return Result;
	}
#if !UE_BUILD_SHIPPING
	const auto DevInvocation = EFMCodexLocalDevRollInvocation::LongShotDirectShot;
#endif
	Result.AuthoritativeResult = ActiveMatchRuntime->ExecuteProviderCall(
#if !UE_BUILD_SHIPPING
		DevInvocation,
#endif
		[this, &Request]()
		{
			return ActiveMatchRuntime->AuthoritativeSession
				.ResolveLongShotDirectAttackRoll(Request);
		});
	Result.bSuccess = Result.AuthoritativeResult.RuntimeEnvelope.bAccepted
		&& Result.AuthoritativeResult.RuntimeEnvelope.bDomainSuccess
		&& Result.AuthoritativeResult.OrchestrationResult.bSuccess;
	if (!Result.bSuccess)
	{
		Result.ErrorCode =
			EFMCodexLocalMatchHostErrorCode::AuthoritativeCommandFailed;
		Result.ErrorMessage = SelectAuthoritativeErrorMessage(
			Result.AuthoritativeResult.RuntimeEnvelope,
			Result.AuthoritativeResult.OrchestrationResult.ErrorMessage);
	}
	return Result;
}

FFMCodexLocalMatchResolveLongShotDirectDefenseRollResult
AFMCodexLocalMatchHostGameMode::ResolveLongShotDirectDefenseRoll(
	const FMatchPlayAuthoritativeResolveLongShotDirectDefenseRollRequest& Request)
{
	using namespace FMCodexLocalMatchHost;
	FFMCodexLocalMatchResolveLongShotDirectDefenseRollResult Result;
	if (!ActiveMatchRuntime.IsValid())
	{
		Result.ErrorCode = EFMCodexLocalMatchHostErrorCode::NoActiveMatch;
		Result.ErrorMessage = NoActiveMatchMessage;
		return Result;
	}
#if !UE_BUILD_SHIPPING
	const auto DevInvocation = EFMCodexLocalDevRollInvocation::LongShotDirectShot;
#endif
	Result.AuthoritativeResult = ActiveMatchRuntime->ExecuteProviderCall(
#if !UE_BUILD_SHIPPING
		DevInvocation,
#endif
		[this, &Request]()
		{
			return ActiveMatchRuntime->AuthoritativeSession
				.ResolveLongShotDirectDefenseRoll(Request);
		});
	Result.bSuccess = Result.AuthoritativeResult.RuntimeEnvelope.bAccepted
		&& Result.AuthoritativeResult.RuntimeEnvelope.bDomainSuccess
		&& Result.AuthoritativeResult.OrchestrationResult.bSuccess;
	if (!Result.bSuccess)
	{
		Result.ErrorCode =
			EFMCodexLocalMatchHostErrorCode::AuthoritativeCommandFailed;
		Result.ErrorMessage = SelectAuthoritativeErrorMessage(
			Result.AuthoritativeResult.RuntimeEnvelope,
			Result.AuthoritativeResult.OrchestrationResult.ErrorMessage);
	}
	return Result;
}

FFMCodexLocalMatchResolveThroughBallBehindDefenseP1DecisionOrPlanResult
AFMCodexLocalMatchHostGameMode
	::ResolveThroughBallBehindDefenseP1DecisionOrPlan()
{
	using namespace FMCodexLocalMatchHost;
	FFMCodexLocalMatchResolveThroughBallBehindDefenseP1DecisionOrPlanResult
		Result;
	if (!ActiveMatchRuntime.IsValid())
	{
		Result.ErrorCode = EFMCodexLocalMatchHostErrorCode::NoActiveMatch;
		Result.ErrorMessage = NoActiveMatchMessage;
		return Result;
	}
#if !UE_BUILD_SHIPPING
	const auto DevInvocation =
		EFMCodexLocalDevRollInvocation::ThroughBallBehindDefenseP1;
#endif
	Result.AuthoritativeResult = ActiveMatchRuntime->ExecuteProviderCall(
#if !UE_BUILD_SHIPPING
		DevInvocation,
#endif
		[this]()
		{
			return ActiveMatchRuntime->AuthoritativeSession
				.ResolveThroughBallBehindDefenseP1DecisionOrPlan();
		});
	Result.bSuccess = Result.AuthoritativeResult.RuntimeEnvelope.bAccepted
		&& Result.AuthoritativeResult.RuntimeEnvelope.bDomainSuccess
		&& Result.AuthoritativeResult.OrchestrationResult.bSuccess;
	if (!Result.bSuccess)
	{
		Result.ErrorCode =
			EFMCodexLocalMatchHostErrorCode::AuthoritativeCommandFailed;
		Result.ErrorMessage = SelectAuthoritativeErrorMessage(
			Result.AuthoritativeResult.RuntimeEnvelope,
			Result.AuthoritativeResult.OrchestrationResult.ErrorMessage);
	}
	return Result;
}

FFMCodexLocalMatchResolveThroughBallBehindDefenseP1AttackRollResult
AFMCodexLocalMatchHostGameMode
	::ResolveThroughBallBehindDefenseP1AttackRoll(
		const
			FMatchPlayAuthoritativeResolveThroughBallBehindDefenseP1AttackRollRequest&
				Request)
{
	using namespace FMCodexLocalMatchHost;
	FFMCodexLocalMatchResolveThroughBallBehindDefenseP1AttackRollResult Result;
	if (!ActiveMatchRuntime.IsValid())
	{
		Result.ErrorCode = EFMCodexLocalMatchHostErrorCode::NoActiveMatch;
		Result.ErrorMessage = NoActiveMatchMessage;
		return Result;
	}
#if !UE_BUILD_SHIPPING
	const auto DevInvocation =
		EFMCodexLocalDevRollInvocation::ThroughBallBehindDefenseP1;
#endif
	Result.AuthoritativeResult = ActiveMatchRuntime->ExecuteProviderCall(
#if !UE_BUILD_SHIPPING
		DevInvocation,
#endif
		[this, &Request]()
		{
			return ActiveMatchRuntime->AuthoritativeSession
				.ResolveThroughBallBehindDefenseP1AttackRoll(Request);
		});
	Result.bSuccess = Result.AuthoritativeResult.RuntimeEnvelope.bAccepted
		&& Result.AuthoritativeResult.RuntimeEnvelope.bDomainSuccess
		&& Result.AuthoritativeResult.OrchestrationResult.bSuccess;
	if (!Result.bSuccess)
	{
		Result.ErrorCode =
			EFMCodexLocalMatchHostErrorCode::AuthoritativeCommandFailed;
		Result.ErrorMessage = SelectAuthoritativeErrorMessage(
			Result.AuthoritativeResult.RuntimeEnvelope,
			Result.AuthoritativeResult.OrchestrationResult.ErrorMessage);
	}
	return Result;
}

FFMCodexLocalMatchResolveThroughBallBehindDefenseP1DefenseRollResult
AFMCodexLocalMatchHostGameMode
	::ResolveThroughBallBehindDefenseP1DefenseRoll(
		const
			FMatchPlayAuthoritativeResolveThroughBallBehindDefenseP1DefenseRollRequest&
				Request)
{
	using namespace FMCodexLocalMatchHost;
	FFMCodexLocalMatchResolveThroughBallBehindDefenseP1DefenseRollResult Result;
	if (!ActiveMatchRuntime.IsValid())
	{
		Result.ErrorCode = EFMCodexLocalMatchHostErrorCode::NoActiveMatch;
		Result.ErrorMessage = NoActiveMatchMessage;
		return Result;
	}
#if !UE_BUILD_SHIPPING
	const auto DevInvocation =
		EFMCodexLocalDevRollInvocation::ThroughBallBehindDefenseP1;
#endif
	Result.AuthoritativeResult = ActiveMatchRuntime->ExecuteProviderCall(
#if !UE_BUILD_SHIPPING
		DevInvocation,
#endif
		[this, &Request]()
		{
			return ActiveMatchRuntime->AuthoritativeSession
				.ResolveThroughBallBehindDefenseP1DefenseRoll(Request);
		});
	Result.bSuccess = Result.AuthoritativeResult.RuntimeEnvelope.bAccepted
		&& Result.AuthoritativeResult.RuntimeEnvelope.bDomainSuccess
		&& Result.AuthoritativeResult.OrchestrationResult.bSuccess;
	if (!Result.bSuccess)
	{
		Result.ErrorCode =
			EFMCodexLocalMatchHostErrorCode::AuthoritativeCommandFailed;
		Result.ErrorMessage = SelectAuthoritativeErrorMessage(
			Result.AuthoritativeResult.RuntimeEnvelope,
			Result.AuthoritativeResult.OrchestrationResult.ErrorMessage);
	}
	return Result;
}

FFMCodexLocalMatchResolveSingleCardFinishingFormulaResult
AFMCodexLocalMatchHostGameMode::ResolveSingleCardFinishingFormula()
{
	using namespace FMCodexLocalMatchHost;
	FFMCodexLocalMatchResolveSingleCardFinishingFormulaResult Result;
	if (!ActiveMatchRuntime.IsValid())
	{
		Result.ErrorCode = EFMCodexLocalMatchHostErrorCode::NoActiveMatch;
		Result.ErrorMessage = NoActiveMatchMessage;
		return Result;
	}
	Result.AuthoritativeResult = ActiveMatchRuntime->AuthoritativeSession
		.ResolveSingleCardFinishingFormula();
	Result.bSuccess = Result.AuthoritativeResult.RuntimeEnvelope.bAccepted
		&& Result.AuthoritativeResult.RuntimeEnvelope.bDomainSuccess
		&& Result.AuthoritativeResult.OrchestrationResult.bSuccess;
	if (!Result.bSuccess)
	{
		Result.ErrorCode =
			EFMCodexLocalMatchHostErrorCode::AuthoritativeCommandFailed;
		Result.ErrorMessage = SelectAuthoritativeErrorMessage(
			Result.AuthoritativeResult.RuntimeEnvelope,
			Result.AuthoritativeResult.OrchestrationResult.ErrorMessage);
	}
	return Result;
}

FFMCodexLocalMatchResolveThroughBallFeetFormulaResult
AFMCodexLocalMatchHostGameMode::ResolveThroughBallFeetFormula()
{
	using namespace FMCodexLocalMatchHost;
	FFMCodexLocalMatchResolveThroughBallFeetFormulaResult Result;
	if (!ActiveMatchRuntime.IsValid())
	{
		Result.ErrorCode = EFMCodexLocalMatchHostErrorCode::NoActiveMatch;
		Result.ErrorMessage = NoActiveMatchMessage;
		return Result;
	}
	Result.AuthoritativeResult = ActiveMatchRuntime->AuthoritativeSession
		.ResolveThroughBallFeetFormula();
	Result.bSuccess = Result.AuthoritativeResult.RuntimeEnvelope.bAccepted
		&& Result.AuthoritativeResult.RuntimeEnvelope.bDomainSuccess
		&& Result.AuthoritativeResult.OrchestrationResult.bSuccess;
	if (!Result.bSuccess)
	{
		Result.ErrorCode =
			EFMCodexLocalMatchHostErrorCode::AuthoritativeCommandFailed;
		Result.ErrorMessage = SelectAuthoritativeErrorMessage(
			Result.AuthoritativeResult.RuntimeEnvelope,
			Result.AuthoritativeResult.OrchestrationResult.ErrorMessage);
	}
	return Result;
}

FFMCodexLocalMatchResolveThroughBallBehindDefenseP1FormulaResult
AFMCodexLocalMatchHostGameMode::ResolveThroughBallBehindDefenseP1Formula()
{
	using namespace FMCodexLocalMatchHost;
	FFMCodexLocalMatchResolveThroughBallBehindDefenseP1FormulaResult Result;
	if (!ActiveMatchRuntime.IsValid())
	{
		Result.ErrorCode = EFMCodexLocalMatchHostErrorCode::NoActiveMatch;
		Result.ErrorMessage = NoActiveMatchMessage;
		return Result;
	}
	Result.AuthoritativeResult = ActiveMatchRuntime->AuthoritativeSession
		.ResolveThroughBallBehindDefenseP1Formula();
	Result.bSuccess = Result.AuthoritativeResult.RuntimeEnvelope.bAccepted
		&& Result.AuthoritativeResult.RuntimeEnvelope.bDomainSuccess
		&& Result.AuthoritativeResult.OrchestrationResult.bSuccess;
	if (!Result.bSuccess)
	{
		Result.ErrorCode =
			EFMCodexLocalMatchHostErrorCode::AuthoritativeCommandFailed;
		Result.ErrorMessage = SelectAuthoritativeErrorMessage(
			Result.AuthoritativeResult.RuntimeEnvelope,
			Result.AuthoritativeResult.OrchestrationResult.ErrorMessage);
	}
	return Result;
}

FFMCodexLocalMatchResolveThroughBallBehindDefenseP2DecisionResult
AFMCodexLocalMatchHostGameMode::ResolveThroughBallBehindDefenseP2Decision()
{
	using namespace FMCodexLocalMatchHost;
	FFMCodexLocalMatchResolveThroughBallBehindDefenseP2DecisionResult Result;
	if (!ActiveMatchRuntime.IsValid())
	{
		Result.ErrorCode = EFMCodexLocalMatchHostErrorCode::NoActiveMatch;
		Result.ErrorMessage = NoActiveMatchMessage;
		return Result;
	}
	Result.AuthoritativeResult = ActiveMatchRuntime->AuthoritativeSession
		.ResolveThroughBallBehindDefenseP2Decision();
	Result.bSuccess = Result.AuthoritativeResult.RuntimeEnvelope.bAccepted
		&& Result.AuthoritativeResult.RuntimeEnvelope.bDomainSuccess
		&& Result.AuthoritativeResult.OrchestrationResult.bSuccess;
	if (!Result.bSuccess)
	{
		Result.ErrorCode =
			EFMCodexLocalMatchHostErrorCode::AuthoritativeCommandFailed;
		Result.ErrorMessage = SelectAuthoritativeErrorMessage(
			Result.AuthoritativeResult.RuntimeEnvelope,
			Result.AuthoritativeResult.OrchestrationResult.ErrorMessage);
	}
	return Result;
}

FFMCodexLocalMatchResolveThroughBallOneOnOneChipShotDecisionResult
AFMCodexLocalMatchHostGameMode::ResolveThroughBallOneOnOneChipShotDecision()
{
	using namespace FMCodexLocalMatchHost;
	FFMCodexLocalMatchResolveThroughBallOneOnOneChipShotDecisionResult Result;
	if (!ActiveMatchRuntime.IsValid())
	{
		Result.ErrorCode = EFMCodexLocalMatchHostErrorCode::NoActiveMatch;
		Result.ErrorMessage = NoActiveMatchMessage;
		return Result;
	}
#if !UE_BUILD_SHIPPING
	const auto DevInvocation =
		EFMCodexLocalDevRollInvocation::OneOnOneChipShot;
#endif
	Result.AuthoritativeResult = ActiveMatchRuntime->ExecuteProviderCall(
#if !UE_BUILD_SHIPPING
		DevInvocation,
#endif
		[this]()
		{
			return ActiveMatchRuntime->AuthoritativeSession
				.ResolveThroughBallOneOnOneChipShotDecision();
		});
	Result.bSuccess = Result.AuthoritativeResult.RuntimeEnvelope.bAccepted
		&& Result.AuthoritativeResult.RuntimeEnvelope.bDomainSuccess
		&& Result.AuthoritativeResult.OrchestrationResult.bSuccess;
	if (!Result.bSuccess)
	{
		Result.ErrorCode =
			EFMCodexLocalMatchHostErrorCode::AuthoritativeCommandFailed;
		Result.ErrorMessage = SelectAuthoritativeErrorMessage(
			Result.AuthoritativeResult.RuntimeEnvelope,
			Result.AuthoritativeResult.OrchestrationResult.ErrorMessage);
	}
	return Result;
}

FFMCodexLocalMatchResolveThroughBallOneOnOneChipShotAttackRollResult
AFMCodexLocalMatchHostGameMode::ResolveThroughBallOneOnOneChipShotAttackRoll(
	const
		FMatchPlayAuthoritativeResolveThroughBallOneOnOneChipShotAttackRollRequest&
			Request)
{
	using namespace FMCodexLocalMatchHost;
	FFMCodexLocalMatchResolveThroughBallOneOnOneChipShotAttackRollResult Result;
	if (!ActiveMatchRuntime.IsValid())
	{
		Result.ErrorCode = EFMCodexLocalMatchHostErrorCode::NoActiveMatch;
		Result.ErrorMessage = NoActiveMatchMessage;
		return Result;
	}
#if !UE_BUILD_SHIPPING
	const auto DevInvocation = EFMCodexLocalDevRollInvocation::OneOnOneChipShot;
#endif
	Result.AuthoritativeResult = ActiveMatchRuntime->ExecuteProviderCall(
#if !UE_BUILD_SHIPPING
		DevInvocation,
#endif
		[this, &Request]()
		{
			return ActiveMatchRuntime->AuthoritativeSession
				.ResolveThroughBallOneOnOneChipShotAttackRoll(Request);
		});
	Result.bSuccess = Result.AuthoritativeResult.RuntimeEnvelope.bAccepted
		&& Result.AuthoritativeResult.RuntimeEnvelope.bDomainSuccess
		&& Result.AuthoritativeResult.OrchestrationResult.bSuccess;
	if (!Result.bSuccess)
	{
		Result.ErrorCode =
			EFMCodexLocalMatchHostErrorCode::AuthoritativeCommandFailed;
		Result.ErrorMessage = SelectAuthoritativeErrorMessage(
			Result.AuthoritativeResult.RuntimeEnvelope,
			Result.AuthoritativeResult.OrchestrationResult.ErrorMessage);
	}
	return Result;
}

FFMCodexLocalMatchResolveThroughBallOneOnOneDirectShotPostRoutePlanResult
AFMCodexLocalMatchHostGameMode
	::ResolveThroughBallOneOnOneDirectShotPostRoutePlan()
{
	using namespace FMCodexLocalMatchHost;
	FFMCodexLocalMatchResolveThroughBallOneOnOneDirectShotPostRoutePlanResult
		Result;
	if (!ActiveMatchRuntime.IsValid())
	{
		Result.ErrorCode = EFMCodexLocalMatchHostErrorCode::NoActiveMatch;
		Result.ErrorMessage = NoActiveMatchMessage;
		return Result;
	}
#if !UE_BUILD_SHIPPING
	const auto DevInvocation =
		EFMCodexLocalDevRollInvocation::OneOnOneDirectShot;
#endif
	Result.AuthoritativeResult = ActiveMatchRuntime->ExecuteProviderCall(
#if !UE_BUILD_SHIPPING
		DevInvocation,
#endif
		[this]()
		{
			return ActiveMatchRuntime->AuthoritativeSession
				.ResolveThroughBallOneOnOneDirectShotPostRoutePlan();
		});
	Result.bSuccess = Result.AuthoritativeResult.RuntimeEnvelope.bAccepted
		&& Result.AuthoritativeResult.RuntimeEnvelope.bDomainSuccess
		&& Result.AuthoritativeResult.OrchestrationResult.bSuccess;
	if (!Result.bSuccess)
	{
		Result.ErrorCode =
			EFMCodexLocalMatchHostErrorCode::AuthoritativeCommandFailed;
		Result.ErrorMessage = SelectAuthoritativeErrorMessage(
			Result.AuthoritativeResult.RuntimeEnvelope,
			Result.AuthoritativeResult.OrchestrationResult.ErrorMessage);
	}
	return Result;
}

FFMCodexLocalMatchResolveThroughBallOneOnOneDirectShotAttackRollResult
AFMCodexLocalMatchHostGameMode::ResolveThroughBallOneOnOneDirectShotAttackRoll(
	const
		FMatchPlayAuthoritativeResolveThroughBallOneOnOneDirectShotAttackRollRequest&
			Request)
{
	using namespace FMCodexLocalMatchHost;
	FFMCodexLocalMatchResolveThroughBallOneOnOneDirectShotAttackRollResult Result;
	if (!ActiveMatchRuntime.IsValid())
	{
		Result.ErrorCode = EFMCodexLocalMatchHostErrorCode::NoActiveMatch;
		Result.ErrorMessage = NoActiveMatchMessage;
		return Result;
	}
#if !UE_BUILD_SHIPPING
	const auto DevInvocation = EFMCodexLocalDevRollInvocation::OneOnOneDirectShot;
#endif
	Result.AuthoritativeResult = ActiveMatchRuntime->ExecuteProviderCall(
#if !UE_BUILD_SHIPPING
		DevInvocation,
#endif
		[this, &Request]()
		{
			return ActiveMatchRuntime->AuthoritativeSession
				.ResolveThroughBallOneOnOneDirectShotAttackRoll(Request);
		});
	Result.bSuccess = Result.AuthoritativeResult.RuntimeEnvelope.bAccepted
		&& Result.AuthoritativeResult.RuntimeEnvelope.bDomainSuccess
		&& Result.AuthoritativeResult.OrchestrationResult.bSuccess;
	if (!Result.bSuccess)
	{
		Result.ErrorCode =
			EFMCodexLocalMatchHostErrorCode::AuthoritativeCommandFailed;
		Result.ErrorMessage = SelectAuthoritativeErrorMessage(
			Result.AuthoritativeResult.RuntimeEnvelope,
			Result.AuthoritativeResult.OrchestrationResult.ErrorMessage);
	}
	return Result;
}

FFMCodexLocalMatchResolveThroughBallOneOnOneDirectShotDefenseRollResult
AFMCodexLocalMatchHostGameMode::ResolveThroughBallOneOnOneDirectShotDefenseRoll(
	const
		FMatchPlayAuthoritativeResolveThroughBallOneOnOneDirectShotDefenseRollRequest&
			Request)
{
	using namespace FMCodexLocalMatchHost;
	FFMCodexLocalMatchResolveThroughBallOneOnOneDirectShotDefenseRollResult Result;
	if (!ActiveMatchRuntime.IsValid())
	{
		Result.ErrorCode = EFMCodexLocalMatchHostErrorCode::NoActiveMatch;
		Result.ErrorMessage = NoActiveMatchMessage;
		return Result;
	}
#if !UE_BUILD_SHIPPING
	const auto DevInvocation = EFMCodexLocalDevRollInvocation::OneOnOneDirectShot;
#endif
	Result.AuthoritativeResult = ActiveMatchRuntime->ExecuteProviderCall(
#if !UE_BUILD_SHIPPING
		DevInvocation,
#endif
		[this, &Request]()
		{
			return ActiveMatchRuntime->AuthoritativeSession
				.ResolveThroughBallOneOnOneDirectShotDefenseRoll(Request);
		});
	Result.bSuccess = Result.AuthoritativeResult.RuntimeEnvelope.bAccepted
		&& Result.AuthoritativeResult.RuntimeEnvelope.bDomainSuccess
		&& Result.AuthoritativeResult.OrchestrationResult.bSuccess;
	if (!Result.bSuccess)
	{
		Result.ErrorCode =
			EFMCodexLocalMatchHostErrorCode::AuthoritativeCommandFailed;
		Result.ErrorMessage = SelectAuthoritativeErrorMessage(
			Result.AuthoritativeResult.RuntimeEnvelope,
			Result.AuthoritativeResult.OrchestrationResult.ErrorMessage);
	}
	return Result;
}

FFMCodexLocalMatchResolveThroughBallOneOnOneDirectShotFormulaResult
AFMCodexLocalMatchHostGameMode
	::ResolveThroughBallOneOnOneDirectShotFormula()
{
	using namespace FMCodexLocalMatchHost;
	FFMCodexLocalMatchResolveThroughBallOneOnOneDirectShotFormulaResult Result;
	if (!ActiveMatchRuntime.IsValid())
	{
		Result.ErrorCode = EFMCodexLocalMatchHostErrorCode::NoActiveMatch;
		Result.ErrorMessage = NoActiveMatchMessage;
		return Result;
	}
	Result.AuthoritativeResult = ActiveMatchRuntime->AuthoritativeSession
		.ResolveThroughBallOneOnOneDirectShotFormula();
	Result.bSuccess = Result.AuthoritativeResult.RuntimeEnvelope.bAccepted
		&& Result.AuthoritativeResult.RuntimeEnvelope.bDomainSuccess
		&& Result.AuthoritativeResult.OrchestrationResult.bSuccess;
	if (!Result.bSuccess)
	{
		Result.ErrorCode =
			EFMCodexLocalMatchHostErrorCode::AuthoritativeCommandFailed;
		Result.ErrorMessage = SelectAuthoritativeErrorMessage(
			Result.AuthoritativeResult.RuntimeEnvelope,
			Result.AuthoritativeResult.OrchestrationResult.ErrorMessage);
	}
	return Result;
}

FFMCodexLocalMatchApplyThroughBallTerminalResolutionResult
AFMCodexLocalMatchHostGameMode::ApplyThroughBallTerminalResolution()
{
	using namespace FMCodexLocalMatchHost;
	FFMCodexLocalMatchApplyThroughBallTerminalResolutionResult Result;
	if (!ActiveMatchRuntime.IsValid())
	{
		Result.ErrorCode = EFMCodexLocalMatchHostErrorCode::NoActiveMatch;
		Result.ErrorMessage = NoActiveMatchMessage;
		return Result;
	}
	Result.AuthoritativeResult = ActiveMatchRuntime->AuthoritativeSession
		.ApplyThroughBallTerminalResolution();
	Result.bSuccess = Result.AuthoritativeResult.RuntimeEnvelope.bAccepted
		&& Result.AuthoritativeResult.RuntimeEnvelope.bDomainSuccess
		&& Result.AuthoritativeResult.OrchestrationResult.bSuccess;
	if (!Result.bSuccess)
	{
		Result.ErrorCode =
			EFMCodexLocalMatchHostErrorCode::AuthoritativeCommandFailed;
		Result.ErrorMessage = SelectAuthoritativeErrorMessage(
			Result.AuthoritativeResult.RuntimeEnvelope,
			Result.AuthoritativeResult.OrchestrationResult.ErrorMessage);
	}
	return Result;
}

FFMCodexLocalMatchApplyCrossTerminalResolutionResult
AFMCodexLocalMatchHostGameMode::ApplyCrossTerminalResolution()
{
	using namespace FMCodexLocalMatchHost;
	FFMCodexLocalMatchApplyCrossTerminalResolutionResult Result;
	if (!ActiveMatchRuntime.IsValid())
	{
		Result.ErrorCode = EFMCodexLocalMatchHostErrorCode::NoActiveMatch;
		Result.ErrorMessage = NoActiveMatchMessage;
		return Result;
	}
	Result.AuthoritativeResult = ActiveMatchRuntime->AuthoritativeSession
		.ApplyCrossTerminalResolution();
	Result.bSuccess = Result.AuthoritativeResult.RuntimeEnvelope.bAccepted
		&& Result.AuthoritativeResult.RuntimeEnvelope.bDomainSuccess
		&& Result.AuthoritativeResult.OrchestrationResult.bSuccess;
	if (!Result.bSuccess)
	{
		Result.ErrorCode =
			EFMCodexLocalMatchHostErrorCode::AuthoritativeCommandFailed;
		Result.ErrorMessage = SelectAuthoritativeErrorMessage(
			Result.AuthoritativeResult.RuntimeEnvelope,
			Result.AuthoritativeResult.OrchestrationResult.ErrorMessage);
	}
	return Result;
}

FFMCodexLocalMatchApplyPassControlTerminalResolutionResult
AFMCodexLocalMatchHostGameMode::ApplyPassControlTerminalResolution()
{
	using namespace FMCodexLocalMatchHost;
	FFMCodexLocalMatchApplyPassControlTerminalResolutionResult Result;
	if (!ActiveMatchRuntime.IsValid())
	{
		Result.ErrorCode = EFMCodexLocalMatchHostErrorCode::NoActiveMatch;
		Result.ErrorMessage = NoActiveMatchMessage;
		return Result;
	}
	Result.AuthoritativeResult = ActiveMatchRuntime->AuthoritativeSession
		.ApplyPassControlTerminalResolution();
	Result.bSuccess = Result.AuthoritativeResult.RuntimeEnvelope.bAccepted
		&& Result.AuthoritativeResult.RuntimeEnvelope.bDomainSuccess
		&& Result.AuthoritativeResult.OrchestrationResult.bSuccess;
	if (!Result.bSuccess)
	{
		Result.ErrorCode =
			EFMCodexLocalMatchHostErrorCode::AuthoritativeCommandFailed;
		Result.ErrorMessage = SelectAuthoritativeErrorMessage(
			Result.AuthoritativeResult.RuntimeEnvelope,
			Result.AuthoritativeResult.OrchestrationResult.ErrorMessage);
	}
	return Result;
}

FFMCodexLocalMatchApplyShotTerminalResolutionResult
AFMCodexLocalMatchHostGameMode::ApplyShotTerminalResolution()
{
	using namespace FMCodexLocalMatchHost;
	FFMCodexLocalMatchApplyShotTerminalResolutionResult Result;
	if (!ActiveMatchRuntime.IsValid())
	{
		Result.ErrorCode = EFMCodexLocalMatchHostErrorCode::NoActiveMatch;
		Result.ErrorMessage = NoActiveMatchMessage;
		return Result;
	}
	Result.AuthoritativeResult = ActiveMatchRuntime->AuthoritativeSession
		.ApplyShotTerminalResolution();
	Result.bSuccess = Result.AuthoritativeResult.RuntimeEnvelope.bAccepted
		&& Result.AuthoritativeResult.RuntimeEnvelope.bDomainSuccess
		&& Result.AuthoritativeResult.OrchestrationResult.bSuccess;
	if (!Result.bSuccess)
	{
		Result.ErrorCode =
			EFMCodexLocalMatchHostErrorCode::AuthoritativeCommandFailed;
		Result.ErrorMessage = SelectAuthoritativeErrorMessage(
			Result.AuthoritativeResult.RuntimeEnvelope,
			Result.AuthoritativeResult.OrchestrationResult.ErrorMessage);
	}
	return Result;
}

FFMCodexLocalMatchAdvanceAfterTerminalResult
AFMCodexLocalMatchHostGameMode::AdvanceAfterTerminal(
	const FMatchPlayAuthoritativeAdvanceAfterTerminalRequest& Request)
{
	using namespace FMCodexLocalMatchHost;
	FFMCodexLocalMatchAdvanceAfterTerminalResult Result;
	if (!ActiveMatchRuntime.IsValid())
	{
		Result.ErrorCode = EFMCodexLocalMatchHostErrorCode::NoActiveMatch;
		Result.ErrorMessage = NoActiveMatchMessage;
		return Result;
	}
	Result.AuthoritativeResult = ActiveMatchRuntime->AuthoritativeSession
		.AdvanceAfterTerminal(Request);
	Result.bSuccess = Result.AuthoritativeResult.RuntimeEnvelope.bAccepted
		&& Result.AuthoritativeResult.RuntimeEnvelope.bDomainSuccess
		&& Result.AuthoritativeResult.CompletionResult.bSuccess;
	if (!Result.bSuccess)
	{
		Result.ErrorCode =
			EFMCodexLocalMatchHostErrorCode::AuthoritativeCommandFailed;
		Result.ErrorMessage = SelectAuthoritativeErrorMessage(
			Result.AuthoritativeResult.RuntimeEnvelope,
			Result.AuthoritativeResult.CompletionResult.ErrorMessage);
	}
	return Result;
}
