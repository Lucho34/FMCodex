#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"

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

private:
	TUniquePtr<FMatchPlayAuthoritativeSession> AuthoritativeSession;
};
