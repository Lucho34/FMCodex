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
}

bool AFMCodexLocalMatchHostGameMode::HasActiveLocalMatch() const
{
	return AuthoritativeSession.IsValid();
}

FFMCodexStartNewLocalMatchResult
AFMCodexLocalMatchHostGameMode::StartNewLocalMatch(
	const FMatchPlayOpeningInitializeInput& Input)
{
	using namespace FMCodexLocalMatchHost;

	FFMCodexStartNewLocalMatchResult Result;
	TUniquePtr<FMatchPlayAuthoritativeSession> CandidateSession =
		MakeUnique<FMatchPlayAuthoritativeSession>();
	Result.AuthoritativeResult = CandidateSession->InitializeMatch(Input);
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

	Result.bReplacedExistingMatch = AuthoritativeSession.IsValid();
	AuthoritativeSession = MoveTemp(CandidateSession);
	return Result;
}

FFMCodexLocalMatchSnapshotResult
AFMCodexLocalMatchHostGameMode::GetMatchSnapshot() const
{
	using namespace FMCodexLocalMatchHost;

	FFMCodexLocalMatchSnapshotResult Result;
	if (!AuthoritativeSession.IsValid())
	{
		Result.ErrorCode =
			EFMCodexLocalMatchHostErrorCode::NoActiveMatch;
		Result.ErrorMessage = NoActiveMatchMessage;
		return Result;
	}

	Result.Snapshot = AuthoritativeSession->GetStateSnapshot();
	Result.bSuccess = true;
	return Result;
}

FFMCodexLocalMatchBeginOrdinaryAttackResult
AFMCodexLocalMatchHostGameMode::BeginOrdinaryAttack(
	const int32 ActionPoint)
{
	using namespace FMCodexLocalMatchHost;

	FFMCodexLocalMatchBeginOrdinaryAttackResult Result;
	if (!AuthoritativeSession.IsValid())
	{
		Result.ErrorCode =
			EFMCodexLocalMatchHostErrorCode::NoActiveMatch;
		Result.ErrorMessage = NoActiveMatchMessage;
		return Result;
	}

	Result.AuthoritativeResult =
		AuthoritativeSession->BeginOrdinaryAttack(ActionPoint);
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
