#include "FMCodexNetworkMatchTypes.h"

#include "../LocalPlay/FMCodexLocalMatchInteractionView.h"

DEFINE_LOG_CATEGORY(LogFMCodexNetworkPlay);

namespace FMCodexNetworkMatchTypes
{
	EFMCodexNetworkClientInteractionState SelectInteractionState(
		const FFMCodexLocalMatchInteractionView& View,
		const EInitialTurnOrderPlayer ViewerSide)
	{
		if (View.bMatchEnded)
		{
			return EFMCodexNetworkClientInteractionState::MatchEnded;
		}
		if (View.ExpectedActingPlayer == EInitialTurnOrderPlayer::None)
		{
			return EFMCodexNetworkClientInteractionState
				::WaitingForOpponentIntent;
		}
		const bool bOwnAction = View.ExpectedActingPlayer == ViewerSide;
		if (View.bTacticalPointRollReady)
		{
			return bOwnAction
				? EFMCodexNetworkClientInteractionState
					::WaitingForOwnInitialActionPoint
				: EFMCodexNetworkClientInteractionState
					::WaitingForOpponentInitialActionPoint;
		}
		return bOwnAction
			? EFMCodexNetworkClientInteractionState::WaitingForOwnIntent
			: EFMCodexNetworkClientInteractionState::WaitingForOpponentIntent;
	}
}

FFMCodexNetworkClientViewSnapshot
FFMCodexNetworkClientViewSnapshotFactory::BuildWaiting(
	const FGuid& MatchInstanceId,
	const int32 ViewRevision,
	const EInitialTurnOrderPlayer ViewerSide,
	const EFMCodexNetworkBootstrapState BootstrapState)
{
	FFMCodexNetworkClientViewSnapshot Result;
	Result.MatchInstanceId = MatchInstanceId;
	Result.ViewRevision = ViewRevision;
	Result.ViewerSide = ViewerSide;
	Result.BootstrapState = BootstrapState;
	Result.InteractionState =
		EFMCodexNetworkClientInteractionState::WaitingForPlayers;
	return Result;
}

FFMCodexNetworkClientViewSnapshot
FFMCodexNetworkClientViewSnapshotFactory::Build(
	const FFMCodexLocalMatchInteractionView& SafeViewerView,
	const FGuid& MatchInstanceId,
	const int32 ViewRevision,
	const EInitialTurnOrderPlayer ViewerSide,
	const EFMCodexNetworkBootstrapState BootstrapState)
{
	FFMCodexNetworkClientViewSnapshot Result;
	Result.MatchInstanceId = MatchInstanceId;
	Result.ViewRevision = ViewRevision;
	Result.ViewerSide = ViewerSide;
	Result.BootstrapState = BootstrapState;
	Result.bMatchInitialized = SafeViewerView.bMatchActive
		|| SafeViewerView.bMatchEnded;
	Result.bMatchEnded = SafeViewerView.bMatchEnded;
	Result.AttackSequence = SafeViewerView.AttackSequence;
	Result.CurrentAttackingSide =
		SafeViewerView.CurrentAttackingPlayer;
	Result.ExpectedActingSide = SafeViewerView.ExpectedActingPlayer;
	Result.PlayerAScore = SafeViewerView.PlayerAScore;
	Result.PlayerBScore = SafeViewerView.PlayerBScore;
	Result.PlayerAMaxAttackOpportunities =
		SafeViewerView.PlayerAMaxAttackTurns;
	Result.PlayerBMaxAttackOpportunities =
		SafeViewerView.PlayerBMaxAttackTurns;
	Result.DisclosedInitialD12 = SafeViewerView.RawInitialD12;
	switch (SafeViewerView.RouteKind)
	{
	case EMatchPlayCurrentAttackRouteKind::SendingOff:
		Result.EntryBranch = EFMCodexNetworkEntryBranch::SendingOff;
		break;
	case EMatchPlayCurrentAttackRouteKind::Ordinary:
		Result.EntryBranch = EFMCodexNetworkEntryBranch::Ordinary;
		break;
	case EMatchPlayCurrentAttackRouteKind::SetPiece:
		Result.EntryBranch = EFMCodexNetworkEntryBranch::SetPiece;
		break;
	default: break;
	}
	if (SafeViewerView.bTacticalPointRollReady)
	{
		Result.EntryWait = EFMCodexNetworkEntryWait::InitialD12;
	}
	else if (SafeViewerView.bTerminalPendingAdvance)
	{
		Result.EntryWait = EFMCodexNetworkEntryWait::TerminalPendingAdvance;
	}
	else if (SafeViewerView.RouteKind == EMatchPlayCurrentAttackRouteKind::SetPiece
		&& SafeViewerView.SetPieceStage == EMatchPlaySetPieceRouteStage::AwaitingTypeRoll)
	{
		Result.EntryWait = EFMCodexNetworkEntryWait::SetPieceTypeRoll;
	}
	else if (SafeViewerView.MajorPhase == EFMCodexLocalMatchMajorPhase::Deployment)
	{
		Result.EntryWait = EFMCodexNetworkEntryWait::Deployment;
	}
	Result.InteractionState =
		FMCodexNetworkMatchTypes::SelectInteractionState(
			SafeViewerView,
			ViewerSide);
	return Result;
}
