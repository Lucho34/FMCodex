#pragma once

#include "CoreMinimal.h"

#include "ThroughBallBranchSelectionQuery.h"
#include "ThroughBallParticipantEligibilityQuery.h"

enum class EThroughBallAntiOffsideOutcomeDecision : uint8
{
	None,
	Offside,
	OneOnOneRequired
};

enum class EThroughBallAntiOffsideOutcomeQueryErrorCode : uint8
{
	None,
	BranchSelectionFailed,
	InvalidBranchSelectionResult,
	UnsupportedBranch,
	ParticipantEligibilityFailed,
	InvalidParticipantEligibilityResult,
	MissingAntiOffsideAttackD6,
	InvalidAntiOffsideAttackD6
};

struct FMCODEX_API FThroughBallAntiOffsideOutcomeQueryInput
{
	FThroughBallBranchSelectionQueryResult BranchSelectionResult;

	FThroughBallParticipantEligibilityQueryResult
		ParticipantEligibilityResult;

	bool bHasAntiOffsideAttackD6 = false;
	int32 AntiOffsideAttackD6 = 0;
};

struct FMCODEX_API FThroughBallAntiOffsideOutcomeQueryResult
{
	bool bSuccess = false;

	EThroughBallAntiOffsideOutcomeQueryErrorCode ErrorCode =
		EThroughBallAntiOffsideOutcomeQueryErrorCode::None;

	FString ErrorMessage;
	FName InvalidField = NAME_None;

	FThroughBallAntiOffsideOutcomeQueryInput Input;

	EThroughBallAntiOffsideOutcomeDecision Decision =
		EThroughBallAntiOffsideOutcomeDecision::None;

	bool bAttackEnded = false;
	bool bContinueResolution = false;
	bool bRequiresOneOnOne = false;

	FName RunnerId = NAME_None;
};

class FMCODEX_API FThroughBallAntiOffsideOutcomeQuery final
{
public:
	static FThroughBallAntiOffsideOutcomeQueryResult Evaluate(
		const FThroughBallAntiOffsideOutcomeQueryInput& Input);
};
