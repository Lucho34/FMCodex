#pragma once

#include "CoreMinimal.h"

#include "../CoreRules/MatchPlayCurrentAttackResolutionFactProjection.h"

/** Presentation-only branch identity used to map canonical resolution facts. */
enum class EFMCodexTacticalNarrativeBranch : uint8
{
	None,
	LongShotDirect,
	LongShotDeadCorner,
	CutInsideDirect,
	CutInsideDeadCorner,
	PassControlPassAdvance,
	PassControlDribbleAdvance,
	PassControlRunAdvance,
	CrossHigh,
	CrossLow,
	ThroughBallFeet,
	ThroughBallBehindDefense,
	ThroughBallAntiOffside,
	ThroughBallOneOnOneDirect,
	ThroughBallOneOnOneChip
};

/** Player-facing result taxonomy. It does not add or replace gameplay outcomes. */
enum class EFMCodexTacticalNarrativeResultCategory : uint8
{
	None,
	Goal,
	ImmediateMiss,
	Miss,
	DefensiveSuccess,
	DefensiveStop,
	OutOfPlay,
	Offside,
	OneOnOneCreated,
	GoalkeeperSave,
	ChipMiss
};

struct FMCODEX_API FFMCodexTacticalNarrativeActor
{
	FName CardId = NAME_None;
	FText DisplayName;

	bool HasPlayerFacingName() const
	{
		return !DisplayName.IsEmpty();
	}
};

struct FMCODEX_API FFMCodexTacticalNarrativePresentationInput
{
	EFMCodexTacticalNarrativeBranch Branch =
		EFMCodexTacticalNarrativeBranch::None;
	EMatchPlayResolutionDecisionOutcome AuthorityOutcome =
		EMatchPlayResolutionDecisionOutcome::None;

	/** Immutable authoritative identity. No random or frame-local value is valid. */
	int64 AttackSequence = 0;
	FName StableEventId = NAME_None;

	FFMCodexTacticalNarrativeActor Carrier;
	FFMCodexTacticalNarrativeActor Runner;
	FFMCodexTacticalNarrativeActor Marker;
	FFMCodexTacticalNarrativeActor Helper;
	FFMCodexTacticalNarrativeActor Goalkeeper;
};

struct FMCODEX_API FFMCodexTacticalNarrativePresentation
{
	bool bSuccess = false;
	bool bNarrativeAvailable = false;
	EFMCodexTacticalNarrativeResultCategory ResultCategory =
		EFMCodexTacticalNarrativeResultCategory::None;
	FText ResultTitle;
	FText NarrativeText;

	/** Presentation dramatization only; never an authoritative causal fact. */
	EMatchPlayResolutionParticipantRole DefensivePerformerRole =
		EMatchPlayResolutionParticipantRole::None;
	FName DefensivePerformerCardId = NAME_None;
};

/**
 * Pure, read-only mapping from authoritative resolution semantics to one-line
 * player-facing Result and Narrative text. It owns no gameplay state or RNG.
 */
class FMCODEX_API FFMCodexTacticalResolutionNarrativePresentationBuilder final
{
public:
	static FFMCodexTacticalNarrativePresentation Build(
		const FFMCodexTacticalNarrativePresentationInput& Input);

	static EMatchPlayResolutionParticipantRole
	ChooseDeterministicDefensivePerformer(
		int64 AttackSequence,
		FName StableEventId,
		const FFMCodexTacticalNarrativeActor& Marker,
		const FFMCodexTacticalNarrativeActor& Helper);
};
