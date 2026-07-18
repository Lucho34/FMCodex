#pragma once

#include "CoreMinimal.h"

struct FThroughBallBehindDefenseP2OutcomeQueryResult;
struct FThroughBallAntiOffsideOutcomeQueryResult;

enum class EThroughBallOneOnOneHandoffCreationErrorCode : uint8
{
	None,
	SourceOutcomeFailed,
	InvalidSourceOutcomeResult,
	UnsupportedSourceOutcomeDecision,
	InvalidAttackingOwnerIdentity,
	InvalidDefendingOwnerIdentity,
	DuplicateOwnerIdentity,
	InvalidShooterIdentity,
	InconsistentShooterIdentity
};

struct FMCODEX_API FThroughBallOneOnOneHandoff
{
	FName AttackingOwnerId = NAME_None;
	FName DefendingOwnerId = NAME_None;
	FName ShooterCardId = NAME_None;
};

struct FMCODEX_API FThroughBallOneOnOneHandoffCreationResult
{
	bool bSuccess = false;

	EThroughBallOneOnOneHandoffCreationErrorCode ErrorCode =
		EThroughBallOneOnOneHandoffCreationErrorCode::None;

	FString ErrorMessage;
	FName InvalidField = NAME_None;

	bool bHasHandoff = false;
	FThroughBallOneOnOneHandoff Handoff;
};

class FMCODEX_API FThroughBallOneOnOneHandoffCreator final
{
public:
	static FThroughBallOneOnOneHandoffCreationResult
	CreateFromBehindDefenseP2(
		const FThroughBallBehindDefenseP2OutcomeQueryResult& P2OutcomeResult);

	static FThroughBallOneOnOneHandoffCreationResult
	CreateFromAntiOffside(
		const FThroughBallAntiOffsideOutcomeQueryResult& AntiOffsideOutcomeResult);
};
