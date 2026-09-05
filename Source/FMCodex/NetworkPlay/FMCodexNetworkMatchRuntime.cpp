#include "FMCodexNetworkMatchRuntime.h"

#include "../LocalPlay/FMCodexLocalMatchD6Provider.h"
#include "../LocalPlay/FMCodexLocalMatchInteractionView.h"
#include "../LocalPlay/FMCodexPrototypeTeamContent.h"
#include "../MatchPlayRuntime/MatchPlayAuthoritativeSession.h"
#include "../MatchPlayRuntime/MatchPlayServerCoordinator.h"

namespace FMCodexNetworkMatchRuntime
{
	FFMCodexNetworkTeamIdentity MakeTeamIdentity(const FName TeamId)
	{
		FFMCodexNetworkTeamIdentity Result;
		Result.TeamId = TeamId;
		for (const FFMCodexPrototypePlayerDefinition& Definition
			: FFMCodexPrototypeTeamContent::GetDefinitions())
		{
			if (Definition.TeamId == TeamId)
			{
				Result.TeamDisplayName =
					Definition.TeamDisplayName.ToString();
				break;
			}
		}
		return Result;
	}
}

FFMCodexNetworkBootstrapConfiguration
FFMCodexNetworkBootstrapConfigurationFactory::CreatePrototypeMatch()
{
	FFMCodexNetworkBootstrapConfiguration Result;
	Result.MatchConfiguration =
		FFMCodexLocalMatchDemoConfigurationFactory::Create();
	Result.PlayerATeam = FMCodexNetworkMatchRuntime::MakeTeamIdentity(
		FFMCodexPrototypeTeamContent::ArsenalTeamId());
	Result.PlayerBTeam = FMCodexNetworkMatchRuntime::MakeTeamIdentity(
		FFMCodexPrototypeTeamContent::ManchesterCityTeamId());
	Result.AttackOpportunitiesPerSide = 3;
	return Result;
}

FFMCodexNetworkMatchRuntime::FFMCodexNetworkMatchRuntime(
	const FGuid& InMatchInstanceId,
	const int32 Seed)
	: MatchInstanceId(InMatchInstanceId)
	, RollProvider(MakeUnique<FFMCodexLocalMatchD6Provider>(Seed))
{
}

FFMCodexNetworkMatchRuntime::~FFMCodexNetworkMatchRuntime() = default;

FFMCodexNetworkRuntimeInitializeResult
FFMCodexNetworkMatchRuntime::InitializeOnce(
	const FFMCodexNetworkBootstrapConfiguration& Configuration)
{
	FFMCodexNetworkRuntimeInitializeResult Result;
	++InitializationAttemptCount;
	if (bInitialized)
	{
		Result.bSuccess = true;
		Result.bAlreadyInitialized = true;
		return Result;
	}
	if (!MatchInstanceId.IsValid())
	{
		Result.ErrorMessage = TEXT("MatchInstanceId is invalid.");
		return Result;
	}

	SkillRuleSet = Configuration.MatchConfiguration.SkillRuleSet;
	AuthoritativeSession = MakeUnique<FMatchPlayAuthoritativeSession>(
		*RollProvider,
		*RollProvider,
		*RollProvider,
		*RollProvider,
		SkillRuleSet);
	ServerCoordinator = MakeUnique<FMatchPlayServerCoordinator>(
		*AuthoritativeSession,
		SkillRuleSet);
	const FMatchPlayAuthoritativeInitializeMatchResult InitializeResult =
		AuthoritativeSession->InitializeMatch(
			Configuration.MatchConfiguration.OpeningInput);
	if (!InitializeResult.RuntimeEnvelope.bAccepted
		|| !InitializeResult.RuntimeEnvelope.bDomainSuccess
		|| !InitializeResult.OpeningResult.bSuccess)
	{
		Result.ErrorMessage = !InitializeResult.RuntimeEnvelope.ErrorMessage.IsEmpty()
			? InitializeResult.RuntimeEnvelope.ErrorMessage
			: InitializeResult.OpeningResult.ErrorMessage;
		AuthoritativeSession.Reset();
		ServerCoordinator.Reset();
		return Result;
	}
	const FMatchPlayServerCoordinatorResult CoordinatorResult =
		ServerCoordinator->AdvanceToStableState();
	if (!CoordinatorResult.bSuccess)
	{
		Result.ErrorMessage = CoordinatorResult.ErrorMessage;
		AuthoritativeSession.Reset();
		ServerCoordinator.Reset();
		return Result;
	}

	bInitialized = true;
	++InitializationCount;
	Result.bSuccess = true;
	return Result;
}

FFMCodexNetworkClientViewSnapshot
FFMCodexNetworkMatchRuntime::BuildClientView(
	const EInitialTurnOrderPlayer ViewerSide,
	const int32 ViewRevision,
	const EFMCodexNetworkBootstrapState BootstrapState) const
{
	if (!bInitialized || !AuthoritativeSession.IsValid())
	{
		return FFMCodexNetworkClientViewSnapshotFactory::BuildWaiting(
			MatchInstanceId,
			ViewRevision,
			ViewerSide,
			BootstrapState);
	}
	const FFMCodexLocalMatchInteractionView SafeViewerView =
		FFMCodexLocalMatchInteractionViewBuilder::BuildForViewer(
			AuthoritativeSession->GetStateSnapshot(),
			SkillRuleSet,
			ViewerSide);
	return FFMCodexNetworkClientViewSnapshotFactory::Build(
		SafeViewerView,
		MatchInstanceId,
		ViewRevision,
		ViewerSide,
		BootstrapState);
}

bool FFMCodexNetworkMatchRuntime::IsInitialized() const
{
	return bInitialized;
}

int32 FFMCodexNetworkMatchRuntime::GetInitializationAttemptCount() const
{
	return InitializationAttemptCount;
}

int32 FFMCodexNetworkMatchRuntime::GetInitializationCount() const
{
	return InitializationCount;
}

const FGuid& FFMCodexNetworkMatchRuntime::GetMatchInstanceId() const
{
	return MatchInstanceId;
}
