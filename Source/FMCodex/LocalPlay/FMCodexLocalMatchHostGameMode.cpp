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
