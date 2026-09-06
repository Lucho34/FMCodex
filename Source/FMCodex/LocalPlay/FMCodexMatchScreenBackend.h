#pragma once

#include "CoreMinimal.h"
#include "FMCodexLocalMatchUMGPresentation.h"

/** Player-facing gestures only. No Side, result, roll or authoritative state. */
enum class EFMCodexMatchScreenIntent : uint8
{
	StartMatch, TacticalPoints, DeployOrdinary, DeployGoalkeeper, FinishDeployment,
	Carrier, Marker, Runner, Helper, Skill, Branch, Decline, NoLegal, OneOnOne, Continue
};

struct FMCODEX_API FFMCodexMatchScreenRequest
{
	EFMCodexMatchScreenIntent Kind = EFMCodexMatchScreenIntent::Continue;
	FName OptionId = NAME_None;
	FName SlotId = NAME_None;
	EFMCodexUMGBranchIntent Branch = EFMCodexUMGBranchIntent::None;
	EFMCodexUMGOneOnOneChoice OneOnOne = EFMCodexUMGOneOnOneChoice::None;
	EFMCodexUMGInteractionCategory Category = EFMCodexUMGInteractionCategory::None;
};

enum class EFMCodexMatchScreenSubmission : uint8 { Rejected, Completed, Queued };

/** Real Local/Network action seam. Read-side adapters push the same value presentation into the screen. */
class FMCODEX_API IFMCodexMatchScreenBackend
{
public:
	virtual ~IFMCodexMatchScreenBackend() = default;
	virtual EFMCodexMatchScreenSubmission SubmitScreenIntent(const FFMCodexMatchScreenRequest& Request) = 0;
	virtual bool IsScreenIntentPending() const = 0;
};
