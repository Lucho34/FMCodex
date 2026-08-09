#pragma once

#include "CoreMinimal.h"

#include "../CoreRules/MatchPlayState.h"
#include "../CoreRules/MatchResultResolver.h"

enum class EFMCodexLocalMatchMajorPhase : uint8
{
	NoActiveMatch,
	BetweenAttacks,
	Deployment,
	Selection,
	Resolution,
	Complete
};

enum class EFMCodexLocalMatchInteractionCategory : uint8
{
	None,
	StartMatch,
	BeginAttack,
	Deploy,
	SelectCarrier,
	SelectMarker,
	SelectSkill,
	SelectRunner,
	SelectHelper,
	SelectBranchIntent,
	SelectOneOnOneShot,
	ContinueResolution,
	AttackComplete,
	MatchEnded
};

struct FMCODEX_API FFMCodexLocalMatchDeploymentOption
{
	EInitialTurnOrderPlayer Side = EInitialTurnOrderPlayer::None;
	FName CardId = NAME_None;
	FName SlotId = NAME_None;
	bool bGoalkeeper = false;
};

struct FMCODEX_API FFMCodexLocalMatchDeploymentGroup
{
	EInitialTurnOrderPlayer Side = EInitialTurnOrderPlayer::None;
	FName CardId = NAME_None;
	bool bGoalkeeper = false;
	TArray<FName> LegalSlotIds;
};

struct FMCODEX_API FFMCodexLocalMatchSelectionOption
{
	EInitialTurnOrderPlayer Side = EInitialTurnOrderPlayer::None;
	FName Id = NAME_None;
	FString Label;
};

enum class EFMCodexLocalMatchRollGroup : uint8
{
	InitialRoute,
	PostRoute
};

struct FMCODEX_API FFMCodexLocalMatchRollView
{
	EFMCodexLocalMatchRollGroup Group =
		EFMCodexLocalMatchRollGroup::InitialRoute;
	FString Purpose;
	int32 RawD6 = 0;
};

struct FMCODEX_API FFMCodexLocalMatchInteractionView
{
	bool bMatchActive = false;
	bool bMatchEnded = false;
	EMatchResultType MatchResult = EMatchResultType::NotEnded;
	int32 PlayerAScore = 0;
	int32 PlayerBScore = 0;
	EInitialTurnOrderPlayer CurrentAttackingPlayer =
		EInitialTurnOrderPlayer::None;
	EInitialTurnOrderPlayer ExpectedActingPlayer =
		EInitialTurnOrderPlayer::None;
	bool bCurrentAttackActive = false;
	int64 AttackSequence = 0;
	int32 ActionPoint = 0;
	EFMCodexLocalMatchMajorPhase MajorPhase =
		EFMCodexLocalMatchMajorPhase::NoActiveMatch;
	EMatchPlayCurrentAttackSelectionStage SelectionStage =
		EMatchPlayCurrentAttackSelectionStage::None;
	EInitialTurnOrderPlayer CurrentLegalDeploymentSide =
		EInitialTurnOrderPlayer::None;
	EFMCodexLocalMatchInteractionCategory InteractionCategory =
		EFMCodexLocalMatchInteractionCategory::StartMatch;
	bool bHumanInteraction = false;
	bool bCanDecline = false;
	bool bCanResolveNoLegalChoice = false;
	FString Diagnostic;
	TArray<FMatchPlayDeploymentPlacement> DeploymentPlacements;
	TArray<FFMCodexLocalMatchDeploymentOption> DeploymentOptions;
	TArray<FFMCodexLocalMatchDeploymentGroup> DeploymentGroups;
	TArray<FFMCodexLocalMatchSelectionOption> SelectionOptions;
	TArray<EMatchPlayElectiveBranchIntent> BranchIntentOptions;
	TArray<EMatchPlayThroughBallOneOnOneShotChoice> OneOnOneOptions;
	TArray<FFMCodexLocalMatchRollView> AcceptedRolls;
	FString ContinueActionLabel = TEXT("Continue Resolution");
};

class FMCODEX_API FFMCodexLocalMatchInteractionViewBuilder final
{
public:
	static FFMCodexLocalMatchInteractionView BuildNoActiveMatch();

	static FFMCodexLocalMatchInteractionView Build(
		const FMatchPlayState& Snapshot,
		const FSkillRuleSnapshotSet& SkillRuleSet);

	static FString ToString(EFMCodexLocalMatchMajorPhase Phase);
	static FString ToString(EFMCodexLocalMatchInteractionCategory Category);
	static FString ToString(EInitialTurnOrderPlayer Player);
	static FString ToString(EMatchResultType Result);
};
