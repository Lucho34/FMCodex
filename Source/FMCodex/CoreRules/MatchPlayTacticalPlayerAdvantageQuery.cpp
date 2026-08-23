#include "MatchPlayTacticalPlayerAdvantageQuery.h"

namespace MatchPlayTacticalPlayerAdvantageQuery
{
	void Fail(
		FMatchPlayTacticalPlayerAdvantageResult& Result,
		const EMatchPlayTacticalPlayerAdvantageErrorCode ErrorCode,
		const TCHAR* ErrorMessage)
	{
		Result.ErrorCode = ErrorCode;
		Result.ErrorMessage = ErrorMessage;
	}

	bool IsPlayerSide(const EInitialTurnOrderPlayer Side)
	{
		return Side == EInitialTurnOrderPlayer::PlayerA
			|| Side == EInitialTurnOrderPlayer::PlayerB;
	}

	bool MatchesPosition(
		const FPlayerCardRuleSnapshot& Snapshot,
		const EMatchPlayRelativeDeploymentZone RelativeZone)
	{
		switch (RelativeZone)
		{
		case EMatchPlayRelativeDeploymentZone::Forward:
			return Snapshot.PositionTypes.Contains(EPlayerPositionType::Attack);
		case EMatchPlayRelativeDeploymentZone::Midfield:
			return Snapshot.PositionTypes.Contains(EPlayerPositionType::Midfield);
		case EMatchPlayRelativeDeploymentZone::Backfield:
			return Snapshot.PositionTypes.Contains(EPlayerPositionType::Defense);
		case EMatchPlayRelativeDeploymentZone::None:
		default:
			return false;
		}
	}
}

FMatchPlayTacticalPlayerAdvantageResult
FMatchPlayTacticalPlayerAdvantageQuery::Evaluate(
	const FMatchPlayState& State)
{
	using namespace MatchPlayTacticalPlayerAdvantageQuery;

	FMatchPlayTacticalPlayerAdvantageResult Result;
	if (!State.bHasCurrentAttack)
	{
		Fail(Result, EMatchPlayTacticalPlayerAdvantageErrorCode::NoCurrentAttack,
			TEXT("Tactical-player advantage requires a CurrentAttack."));
		return Result;
	}
	if (!State.CurrentAttack.bHasResolutionSession)
	{
		Fail(Result,
			EMatchPlayTacticalPlayerAdvantageErrorCode::MissingResolutionSession,
			TEXT("Tactical-player advantage requires a Resolution Session."));
		return Result;
	}

	const FMatchPlayCurrentAttackResolutionSessionBundle& Bundle =
		State.CurrentAttack.ResolutionSession.Bundle;
	Result.AttackingPlayer = Bundle.CurrentAttackingPlayer;
	Result.DefendingPlayer = Bundle.CurrentDefendingPlayer;
	if (!IsPlayerSide(Result.AttackingPlayer))
	{
		Fail(Result,
			EMatchPlayTacticalPlayerAdvantageErrorCode::InvalidAttackingPlayer,
			TEXT("Tactical-player advantage requires PlayerA or PlayerB as the attacking player."));
		return Result;
	}
	if (!IsPlayerSide(Result.DefendingPlayer)
		|| Result.DefendingPlayer == Result.AttackingPlayer)
	{
		Fail(Result,
			EMatchPlayTacticalPlayerAdvantageErrorCode::InvalidDefendingPlayer,
			TEXT("Tactical-player advantage requires the opposing defending player."));
		return Result;
	}

	for (const FMatchPlayDeploymentPlacement& Placement
		: State.CurrentAttack.DeploymentPlacements)
	{
		const FMatchPlayRelativeDeploymentZoneResolveResult Zone =
			FMatchPlayRelativeDeploymentZoneResolver::Resolve(
				State.DeploymentSlotCatalog,
				Placement.SlotId,
				Result.AttackingPlayer,
				Placement.PlayerSide);
		if (!Zone.bSuccess)
		{
			continue;
		}
		const FMatchPlayCardSnapshotAuthorityQueryResult Snapshot =
			FMatchPlayCardSnapshotAuthorityQuery::FindByPlayerSideAndCardId(
				State.CardSnapshotAuthority,
				Placement.PlayerSide,
				Placement.CardId);
		if (!Snapshot.bSuccess || Snapshot.Snapshot.bIsGoalkeeper
			|| !MatchesPosition(Snapshot.Snapshot, Zone.RelativeZone))
		{
			continue;
		}

		Result.TacticalPlayers.Add({
			Placement.PlayerSide,
			Placement.CardId,
			Zone.RelativeZone });
		if (Placement.PlayerSide == Result.AttackingPlayer)
		{
			++Result.AttackerTacticalPlayerCount;
		}
		else if (Placement.PlayerSide == Result.DefendingPlayer)
		{
			++Result.DefenderTacticalPlayerCount;
		}
	}

	Result.AttackerFinishingModifier = ModifierForCountAdvantage(
		Result.AttackerTacticalPlayerCount
			- Result.DefenderTacticalPlayerCount);
	Result.DefenderFinishingModifier = ModifierForCountAdvantage(
		Result.DefenderTacticalPlayerCount
			- Result.AttackerTacticalPlayerCount);
	Result.bSuccess = true;
	return Result;
}

float FMatchPlayTacticalPlayerAdvantageQuery::ModifierForCountAdvantage(
	const int32 CountAdvantage)
{
	if (CountAdvantage >= 3)
	{
		return 2.0f;
	}
	if (CountAdvantage >= 2)
	{
		return 1.0f;
	}
	return 0.0f;
}
