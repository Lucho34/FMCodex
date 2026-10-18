#pragma once

#include "CoreMinimal.h"
#include "FMCodexLocalMatchUMGPresentation.h"
#include "FMCodexNextRoundCountdown.h"

/** Player-facing gestures only. No Side, result, roll or authoritative state. */
enum class EFMCodexMatchScreenIntent : uint8
{
	StartMatch, TacticalPoints, DeployOrdinary, DeployGoalkeeper, FinishDeployment,
	Carrier, Marker, Runner, Helper, Skill, Branch, Decline, NoLegal, OneOnOne, Continue, SetPieceType, SetPieceTaker, NearMethod, LongMethod, PenaltyMethod, CornerNominations, CornerIntent
};

struct FMCODEX_API FFMCodexMatchScreenRequest
{
	EFMCodexMatchScreenIntent Kind = EFMCodexMatchScreenIntent::Continue;
	TArray<FName> CornerCandidateIds;
	EMatchPlayCornerRouteIntent CornerIntent = EMatchPlayCornerRouteIntent::None;
	FName OptionId = NAME_None;
	FName SlotId = NAME_None;
	EMatchPlayShortFreeKickMethod NearMethod = EMatchPlayShortFreeKickMethod::None;
	EMatchPlayLongFreeKickMethod LongMethod = EMatchPlayLongFreeKickMethod::None;
	EMatchPlayPenaltyMethod PenaltyMethod = EMatchPlayPenaltyMethod::None;
	EFMCodexUMGBranchIntent Branch = EFMCodexUMGBranchIntent::None;
	EFMCodexUMGOneOnOneChoice OneOnOne = EFMCodexUMGOneOnOneChoice::None;
	EFMCodexUMGInteractionCategory Category = EFMCodexUMGInteractionCategory::None;
};

enum class EFMCodexMatchScreenSubmission : uint8 { Rejected, Completed, Queued };

/** Cross presentation gestures survive screen reconstruction on the same backend.
 * No gameplay state or RNG: the safe view still authorizes every request. */
struct FFMCodexCrossPresentationFlow
{
	int64 AttackSequence = 0;
	EFMCodexUMGBranchIntent Draft = EFMCodexUMGBranchIntent::None;
	bool bBranchSubmitted = false;
	bool bRouteSubmitted = false;
	bool bRouteRejected = false;
	void Observe(int64 Sequence)
	{
		if (AttackSequence != Sequence) { *this = {}; AttackSequence = Sequence; }
	}
};

/** Real Local/Network action seam. Read-side adapters push the same value presentation into the screen. */
class FMCODEX_API IFMCodexMatchScreenBackend
{
public:
	virtual ~IFMCodexMatchScreenBackend() = default;
	virtual EFMCodexMatchScreenSubmission SubmitScreenIntent(const FFMCodexMatchScreenRequest& Request) = 0;
	virtual bool IsScreenIntentPending() const = 0;
	FFMCodexCrossPresentationFlow CrossPresentationFlow;
	FFMCodexNextRoundDispatch NextRoundDispatch;
};
