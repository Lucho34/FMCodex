#include "FMCodexLocalMatchHostGameMode.h"

namespace FMCodexLocalMatchHost
{
	constexpr const TCHAR* NoActiveMatchMessage =
		TEXT("No local match is active.");

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
}

AFMCodexLocalMatchHostGameMode::FLocalMatchRuntime::FLocalMatchRuntime(
	const int32 Seed)
	: D6Provider(Seed)
	, AuthoritativeSession(
		D6Provider,
		D6Provider,
		FSkillRuleSnapshotSet())
{
}

bool AFMCodexLocalMatchHostGameMode::HasActiveLocalMatch() const
{
	return ActiveMatchRuntime.IsValid();
}

FFMCodexStartNewLocalMatchResult
AFMCodexLocalMatchHostGameMode::StartNewLocalMatch(
	const FMatchPlayOpeningInitializeInput& Input)
{
	using namespace FMCodexLocalMatchHost;

	FFMCodexStartNewLocalMatchResult Result;
	TUniquePtr<FLocalMatchRuntime> CandidateRuntime =
		MakeUnique<FLocalMatchRuntime>(GenerateLocalMatchSeed());
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

	Result.AuthoritativeResult =
		ActiveMatchRuntime->AuthoritativeSession.SubmitSkill(
			SkillRuleSet,
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

	Result.AuthoritativeResult =
		ActiveMatchRuntime->AuthoritativeSession.ResolveNoLegalSkill(
			SkillRuleSet);
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

	Result.AuthoritativeResult =
		ActiveMatchRuntime->AuthoritativeSession.DeclineSkill(
			SkillRuleSet,
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
