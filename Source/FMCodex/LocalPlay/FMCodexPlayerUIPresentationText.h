#pragma once

#include "CoreMinimal.h"

#include "../CoreRules/MatchPlayCurrentAttackResolutionFactProjection.h"

/**
 * Bounded, presentation-only zh-CN terminology for player-card UMG.
 * Canonical gameplay values remain English and never depend on these labels.
 */
class FMCODEX_API FFMCodexPlayerUIPresentationText final
{
public:
	static FText PlayerName(FName CardId, const FString& FallbackLabel);
	static FText InMatchShortPlayerName(
		FName CardId, const FString& FallbackLabel);
	static FText CompactPlayerName(FName CardId, const FString& FallbackLabel);
	static FText HandMicroPlayerName(FName CardId, const FString& FallbackLabel);
	static FText HandMicroFallbackPlayerName(FName CardId);
	static FText HandMicroCompactRole(const FString& CanonicalLabel);
	static FText PitchMiniCompactRole(const FString& CanonicalLabel);
	static FText MatchScreenLabel(const FString& CanonicalLabel);
	static FText SelectedRoleTag(const FString& CanonicalRole);
	static FText SelectionFeedback(const FString& CanonicalReason);
	static FText TeamName(FName CardId);
	static FText Role(const FString& CanonicalLabel);
	static FText CompactRole(const FString& CanonicalLabel);
	static FText InMatchCompactRole(const FString& CanonicalLabel);
	static FText Skill(const FString& CanonicalLabel);
	static FText Attribute(const FString& CanonicalEntry);
	static FText AttributeLabel(const FString& CanonicalToken);
	static FText Status(const FString& CanonicalLabel);
	static FText Rarity(const FString& CanonicalLabel);
	static FText CompactRarity(const FString& CanonicalLabel);
	static FText Owner(const FString& CanonicalLabel);
	static FText RackHeading(const FString& SideLabel, bool bLocalRack);
	static FText TacticalRegion(const FString& CanonicalLabel);
	static FText TacticalLaneHeading(
		const FString& CanonicalLabel, bool bAttacking);
	static FText Turn(int64 AttackSequence);
	static FText TacticalPoints(int32 ActionPoint);
	static FText TacticalPointsHeading();
	static FText AttackTurnHeading();
	static FText CurrentAttackProgress(
		const FString& PlayerLabel, int32 AttackIndex, int32 MaxAttackTurns);
	static FText WaitingForTacticalPointRoll();
	static FText ResolutionContest(FName ContestId);
	static FText ResolutionParticipantRole(
		EMatchPlayResolutionParticipantRole Role);
	static FText TacticalDetailParticipantRole(
		EMatchPlayResolutionParticipantRole Role);
	static FText TacticalOutcome(
		FName BranchId, FName OutcomeId);
	static FText TacticalOutcomeRange(
		int32 Minimum, int32 Maximum, const FText& OutcomeLabel);
	static FText ResolutionAttribute(
		EMatchPlayResolutionFormulaAttribute Attribute);
	static FText ResolutionAttackRow();
	static FText ResolutionDefenseRow();
	static FText ResolutionPending();
	static FText ResolutionResolved();
	static FText ResolutionAttackRoll();
	static FText ResolutionAttackRollSettled();
	static FText ResolutionDefenseRoll();
	static FText ResolutionDefenseRollSettled();
	static FText ContinueResolution();
	static FText ThroughBallTitle();
	static FText ThroughBallInitialRouteStage();
	static FText ThroughBallInitialRouteAction();
	static FText ThroughBallRoute(
		EMatchPlayThroughBallActualBranch Route);
	static FText ThroughBallFeetStage();
	static FText ThroughBallBehindDefenseStage();
	static FText ThroughBallAntiOffsideStage();
	static FText ThroughBallOneOnOneStage();
	static FText ThroughBallOneOnOnePrompt();
	static FText ThroughBallDirectChoiceHint();
	static FText ThroughBallChipChoiceHint();
	static FText ThroughBallRouteResult(
		int32 RawD6, EMatchPlayThroughBallActualBranch Route);
	static FText LongShotTitle();
	static FText LongShotBranchChoiceStage();
	static FText LongShotDirectChoiceHint();
	static FText LongShotDeadCornerChoiceHint();
	static FText LongShotDirectStage();
	static FText LongShotDeadCornerStage();
	static FText LongShotDirectOutcomeHint();
	static FText LongShotDeadCornerOutcomeHint();
	static FText ShotAttackRollAction();
	static FText ShotDefenseRollAction();
	static FText ShotPairedRollAction();
	static FText CutInsideTitle();
	static FText CutInsideBranchChoiceStage();
	static FText CutInsideDirectStage();
	static FText CutInsideDeadCornerStage();
	static FText CutInsideDirectOutcomeHint();
	static FText CrossTitle();
	static FText CrossBranchChoiceStage();
	static FText BroadcastStatus(bool bMatchEnded, bool bAttackActive,
		const FString& MatchResultLabel);

	static FText UnknownCard();
	static FText UnknownRole();
	static FText UnknownRarity();
	static FText NoSkill();
	static FText AttributesUnavailable();
	static FText Unavailable();
	static FText PortraitPlaceholder();
	static FText SkillsHeading();
	static FText AttributesHeading();
	static FText FullCardAttributesHeading();
	static FText OverallHeading();
	static FText PositionHeading();
	static FText FullCardPositionTypeHeading();
	static FText FullCardIdentitySupplement(
		const FString& NationalityLabel, const FString& ClubLabel);
	static FText BirthDateHeading();
	static FText HeightHeading();
	static FText WeightHeading();
	static FText DeploymentHandInstruction();
	static FText EmptyPitchSlot();
	static FText ValidDeploymentTarget();
	static FText InvalidDeploymentTarget();
	static FText OccupiedDeploymentTarget();
	static FText UnavailableDeploymentTarget();
};
