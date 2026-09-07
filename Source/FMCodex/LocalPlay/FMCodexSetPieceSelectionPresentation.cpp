#include "FMCodexSetPieceSelectionPresentation.h"
#include "FMCodexLocalMatchInteractionView.h"

void FFMCodexSetPieceSelectionPresentation::DisableActions()
{
 bCanRollType = false;
 TakerOptions.Reset(); NearMethods.Reset(); LongMethods.Reset(); PenaltyMethods.Reset();
}
FFMCodexSetPieceSelectionPresentation FFMCodexSetPieceSelectionPresentation::Build(
 const FFMCodexLocalMatchInteractionView& V, EInitialTurnOrderPlayer Viewer)
{
 FFMCodexSetPieceSelectionPresentation P;
 if (V.RouteKind != EMatchPlayCurrentAttackRouteKind::SetPiece) return P;
 using C = EFMCodexLocalMatchInteractionCategory;
 P.bVisible = true;
 P.bNoTakerNoGoal = V.bTerminalPendingAdvance && V.bSetPieceNoLegalCarrier && V.bHasSetPieceOutcome && !V.bSetPieceGoal;
 P.AttackingSide = V.CurrentAttackingPlayer; P.ActingSide = V.ExpectedActingPlayer;
 P.bTypeWait = V.SetPieceStage == EMatchPlaySetPieceRouteStage::AwaitingTypeRoll;
 P.bTakerWait = V.InteractionCategory == C::SelectSetPieceCarrier || V.InteractionCategory == C::ConfirmSetPieceCarrier;
 P.bMethodWait = V.InteractionCategory == C::SelectSetPieceMethod;
 P.Type = V.SetPieceType; P.TypeLabel = FText::FromString(V.ActionLabel);
 P.TypeD6 = V.bHasSetPieceTypeRoll ? V.RawSetPieceTypeD6 : 0;
 P.CarrierStage = V.SetPieceCarrierStage; P.CornerStage = V.CornerStage;
 P.NearMethod = V.SelectedNearMethod; P.LongMethod = V.SelectedLongMethod; P.PenaltyMethod = V.SelectedPenaltyMethod;
 const auto& Roster = V.CurrentAttackingPlayer == EInitialTurnOrderPlayer::PlayerA ? V.PlayerACardRoster : V.PlayerBCardRoster;
 if (V.SetPieceCarrier.bIsBound)
 {
  P.TakerCardId = V.SetPieceCarrier.CardId;
  const auto* Card = Roster.FindByPredicate([&](const auto& C){return C.CardId == P.TakerCardId;});
  P.TakerLabel = FText::FromString(Card && !Card->DisplayLabel.IsEmpty() ? Card->DisplayLabel : TEXT("球员"));
 }
 if (!V.bHumanInteraction || Viewer == EInitialTurnOrderPlayer::None || V.ExpectedActingPlayer != Viewer) return P;
 P.bCanRollType = P.bTypeWait && V.InteractionCategory == C::RollSetPieceType;
 if (P.bTakerWait)
 {
  TSet<FName> Seen;
  if (V.LegalSetPieceCardIds.Num() > MaxTakers) P.bOptionsUnavailable = true;
  for (FName Id : V.LegalSetPieceCardIds)
  {
   const auto* Card = Roster.FindByPredicate([&](const auto& C){return C.CardId == Id;});
   if (Id.IsNone() || Id.ToString().Len() > 128 || Seen.Contains(Id) || !Card || !Card->bAvailable || Card->bGoalkeeper || Card->bUsed || Card->bEjected)
    P.bOptionsUnavailable = true;
   Seen.Add(Id);
  }
  if (!P.bOptionsUnavailable) P.TakerOptions = V.LegalSetPieceCardIds;
 }
 if (P.bMethodWait)
 {
  P.NearMethods = V.LegalNearMethods; P.LongMethods = V.LegalLongMethods; P.PenaltyMethods = V.LegalPenaltyMethods;
 }
 return P;
}
