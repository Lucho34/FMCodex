#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"

#include "FMCodexLocalMatchD6Provider.h"
#include "../MatchPlayRuntime/MatchPlayAuthoritativeSession.h"

#include "FMCodexLocalMatchHostGameMode.generated.h"

enum class EFMCodexLocalMatchHostErrorCode : uint8
{
	None,
	NoActiveMatch,
	AuthoritativeInitializationFailed,
	AuthoritativeCommandFailed
};

struct FMCODEX_API FFMCodexStartNewLocalMatchResult
{
	bool bSuccess = false;
	bool bReplacedExistingMatch = false;
	FMatchPlayAuthoritativeInitializeMatchResult AuthoritativeResult;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API FFMCodexLocalMatchSnapshotResult
{
	bool bSuccess = false;
	FMatchPlayState Snapshot;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API FFMCodexLocalMatchBeginOrdinaryAttackResult
{
	bool bSuccess = false;
	FMatchPlayAuthoritativeBeginOrdinaryAttackResult AuthoritativeResult;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API FFMCodexLocalMatchDeployOrdinaryResult
{
	bool bSuccess = false;
	FMatchPlayAuthoritativeDeployOrdinaryResult AuthoritativeResult;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API FFMCodexLocalMatchDeployGoalkeeperResult
{
	bool bSuccess = false;
	FMatchPlayAuthoritativeDeployGoalkeeperResult AuthoritativeResult;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API FFMCodexLocalMatchFinishDeploymentResult
{
	bool bSuccess = false;
	FMatchPlayAuthoritativeFinishDeploymentResult AuthoritativeResult;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API FFMCodexLocalMatchSubmitCarrierResult
{
	bool bSuccess = false;
	FMatchPlayAuthoritativeSubmitCarrierResult AuthoritativeResult;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API FFMCodexLocalMatchResolveNoLegalCarrierResult
{
	bool bSuccess = false;
	FMatchPlayAuthoritativeResolveNoLegalCarrierResult AuthoritativeResult;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API FFMCodexLocalMatchSubmitMarkerResult
{
	bool bSuccess = false;
	FMatchPlayAuthoritativeSubmitMarkerResult AuthoritativeResult;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API FFMCodexLocalMatchResolveNoLegalMarkerResult
{
	bool bSuccess = false;
	FMatchPlayAuthoritativeResolveNoLegalMarkerResult AuthoritativeResult;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API FFMCodexLocalMatchDeclineMarkerResult
{
	bool bSuccess = false;
	FMatchPlayAuthoritativeDeclineMarkerResult AuthoritativeResult;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API FFMCodexLocalMatchSubmitSkillResult
{
	bool bSuccess = false;
	FMatchPlayAuthoritativeSubmitSkillResult AuthoritativeResult;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API FFMCodexLocalMatchResolveNoLegalSkillResult
{
	bool bSuccess = false;
	FMatchPlayAuthoritativeResolveNoLegalSkillResult AuthoritativeResult;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API FFMCodexLocalMatchDeclineSkillResult
{
	bool bSuccess = false;
	FMatchPlayAuthoritativeDeclineSkillResult AuthoritativeResult;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API FFMCodexLocalMatchSubmitRunnerResult
{
	bool bSuccess = false;
	FMatchPlayAuthoritativeSubmitRunnerResult AuthoritativeResult;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API FFMCodexLocalMatchResolveNoLegalRunnerResult
{
	bool bSuccess = false;
	FMatchPlayAuthoritativeResolveNoLegalRunnerResult AuthoritativeResult;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API FFMCodexLocalMatchDeclineRunnerResult
{
	bool bSuccess = false;
	FMatchPlayAuthoritativeDeclineRunnerResult AuthoritativeResult;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API FFMCodexLocalMatchSubmitHelperResult
{
	bool bSuccess = false;
	FMatchPlayAuthoritativeSubmitHelperResult AuthoritativeResult;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API FFMCodexLocalMatchResolveNoLegalHelperResult
{
	bool bSuccess = false;
	FMatchPlayAuthoritativeResolveNoLegalHelperResult AuthoritativeResult;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API FFMCodexLocalMatchDeclineHelperResult
{
	bool bSuccess = false;
	FMatchPlayAuthoritativeDeclineHelperResult AuthoritativeResult;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API FFMCodexLocalMatchSubmitBranchIntentResult
{
	bool bSuccess = false;
	FMatchPlayAuthoritativeSubmitBranchIntentResult AuthoritativeResult;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API FFMCodexLocalMatchSubmitThroughBallOneOnOneShotChoiceResult
{
	bool bSuccess = false;
	FMatchPlayAuthoritativeSubmitThroughBallOneOnOneShotChoiceResult
		AuthoritativeResult;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

UCLASS()
class FMCODEX_API AFMCodexLocalMatchHostGameMode final
	: public AGameModeBase
{
	GENERATED_BODY()

public:
	bool HasActiveLocalMatch() const;

	FFMCodexStartNewLocalMatchResult StartNewLocalMatch(
		const FMatchPlayOpeningInitializeInput& Input);

	FFMCodexLocalMatchSnapshotResult GetMatchSnapshot() const;

	FFMCodexLocalMatchBeginOrdinaryAttackResult BeginOrdinaryAttack(
		int32 ActionPoint);

	FFMCodexLocalMatchDeployOrdinaryResult DeployOrdinary(
		const FMatchPlayAuthoritativeDeployOrdinaryRequest& Request);

	FFMCodexLocalMatchDeployGoalkeeperResult DeployGoalkeeper(
		const FMatchPlayAuthoritativeDeployGoalkeeperRequest& Request);

	FFMCodexLocalMatchFinishDeploymentResult FinishDeployment(
		int64 AttackSequence,
		EInitialTurnOrderPlayer RequestingSide);

	FFMCodexLocalMatchSubmitCarrierResult SubmitCarrier(
		const FMatchPlayAuthoritativeSubmitCarrierRequest& Request);

	FFMCodexLocalMatchResolveNoLegalCarrierResult ResolveNoLegalCarrier();

	FFMCodexLocalMatchSubmitMarkerResult SubmitMarker(
		const FMatchPlayAuthoritativeSubmitMarkerRequest& Request);

	FFMCodexLocalMatchResolveNoLegalMarkerResult ResolveNoLegalMarker();

	FFMCodexLocalMatchDeclineMarkerResult DeclineMarker(
		const FMatchPlayAuthoritativeDeclineMarkerRequest& Request);

	FFMCodexLocalMatchSubmitSkillResult SubmitSkill(
		const FSkillRuleSnapshotSet& SkillRuleSet,
		const FMatchPlayAuthoritativeSubmitSkillRequest& Request);

	FFMCodexLocalMatchResolveNoLegalSkillResult ResolveNoLegalSkill(
		const FSkillRuleSnapshotSet& SkillRuleSet);

	FFMCodexLocalMatchDeclineSkillResult DeclineSkill(
		const FSkillRuleSnapshotSet& SkillRuleSet,
		const FMatchPlayAuthoritativeDeclineSkillRequest& Request);

	FFMCodexLocalMatchSubmitRunnerResult SubmitRunner(
		const FMatchPlayAuthoritativeSubmitRunnerRequest& Request);

	FFMCodexLocalMatchResolveNoLegalRunnerResult ResolveNoLegalRunner();

	FFMCodexLocalMatchDeclineRunnerResult DeclineRunner(
		const FMatchPlayAuthoritativeDeclineRunnerRequest& Request);

	FFMCodexLocalMatchSubmitHelperResult SubmitHelper(
		const FMatchPlayAuthoritativeSubmitHelperRequest& Request);

	FFMCodexLocalMatchResolveNoLegalHelperResult ResolveNoLegalHelper();

	FFMCodexLocalMatchDeclineHelperResult DeclineHelper(
		const FMatchPlayAuthoritativeDeclineHelperRequest& Request);

	FFMCodexLocalMatchSubmitBranchIntentResult SubmitBranchIntent(
		const FMatchPlayAuthoritativeSubmitBranchIntentRequest& Request);

	FFMCodexLocalMatchSubmitThroughBallOneOnOneShotChoiceResult
	SubmitThroughBallOneOnOneShotChoice(
		const FMatchPlayAuthoritativeSubmitThroughBallOneOnOneShotChoiceRequest&
			Request);

private:
	struct FLocalMatchRuntime final
	{
		explicit FLocalMatchRuntime(int32 Seed);

		FFMCodexLocalMatchD6Provider D6Provider;
		FMatchPlayAuthoritativeSession AuthoritativeSession;
	};

	TUniquePtr<FLocalMatchRuntime> ActiveMatchRuntime;
};
