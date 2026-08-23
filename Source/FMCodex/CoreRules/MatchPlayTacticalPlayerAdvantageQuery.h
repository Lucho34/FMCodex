#pragma once

#include "CoreMinimal.h"

#include "MatchPlayState.h"

enum class EMatchPlayTacticalPlayerAdvantageErrorCode : uint8
{
	None,
	NoCurrentAttack,
	MissingResolutionSession,
	InvalidAttackingPlayer,
	InvalidDefendingPlayer
};

struct FMCODEX_API FMatchPlayTacticalPlayerIdentity
{
	EInitialTurnOrderPlayer Side = EInitialTurnOrderPlayer::None;
	FName CardId = NAME_None;
	EMatchPlayRelativeDeploymentZone RelativeZone =
		EMatchPlayRelativeDeploymentZone::None;
};

/**
 * Canonical Rules 4.4 query. It classifies deployed outfield players against
 * the relative zone for the frozen current-attack ownership, then derives the
 * finishing-formula advantage modifier from the side counts.
 */
struct FMCODEX_API FMatchPlayTacticalPlayerAdvantageResult
{
	bool bSuccess = false;
	EMatchPlayTacticalPlayerAdvantageErrorCode ErrorCode =
		EMatchPlayTacticalPlayerAdvantageErrorCode::None;
	FString ErrorMessage;
	EInitialTurnOrderPlayer AttackingPlayer =
		EInitialTurnOrderPlayer::None;
	EInitialTurnOrderPlayer DefendingPlayer =
		EInitialTurnOrderPlayer::None;
	int32 AttackerTacticalPlayerCount = 0;
	int32 DefenderTacticalPlayerCount = 0;
	float AttackerFinishingModifier = 0.0f;
	float DefenderFinishingModifier = 0.0f;
	TArray<FMatchPlayTacticalPlayerIdentity> TacticalPlayers;
};

class FMCODEX_API FMatchPlayTacticalPlayerAdvantageQuery final
{
public:
	static FMatchPlayTacticalPlayerAdvantageResult Evaluate(
		const FMatchPlayState& State);

	/**
	 * Read-only board-status projection. Unlike the finishing-formula query,
	 * this remains valid before a Resolution Session and between attacks. It
	 * uses the authoritative current attacking side to resolve relative zones
	 * and returns zero counts when no CurrentAttack placements exist.
	 */
	static FMatchPlayTacticalPlayerAdvantageResult EvaluateBoardStatus(
		const FMatchPlayState& State);

	static float ModifierForCountAdvantage(int32 CountAdvantage);
};
