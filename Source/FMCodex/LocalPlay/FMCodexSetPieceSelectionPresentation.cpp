#include "FMCodexSetPieceSelectionPresentation.h"
#include "FMCodexLocalMatchInteractionView.h"

void FFMCodexSetPieceSelectionPresentation::DisableActions()
{
 bCanRollType = false;
 TakerOptions.Reset(); NearMethods.Reset(); LongMethods.Reset(); PenaltyMethods.Reset(); CornerOptions.Reset();
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
 P.bCornerDraft = V.InteractionCategory == C::DraftCornerAttacker || V.InteractionCategory == C::DraftCornerDefender;
 P.bCornerIntentWait = V.InteractionCategory == C::SelectCornerIntent;
 P.bCornerAttackerLocked = V.bCornerAttackerNominationsLocked;
 P.bCornerDefenderLocked = V.bCornerDefenderNominationsLocked;
 P.bHideCornerAttackerDetails = V.bHideCornerAttackerNomineeDetails;
 for (const auto& B : V.CornerAttackerNominees) P.CornerAttackers.Add(B.CardId);
 for (const auto& B : V.CornerDefenderNominees) P.CornerDefenders.Add(B.CardId);
 P.CornerAttackerRollLabels = V.CornerAttackerNomineeRollLabels;
 P.CornerDefenderRollLabels = V.CornerDefenderNomineeRollLabels;
 P.CornerRunner = V.CornerRunner.CardId; P.CornerHelper = V.CornerHelper.CardId;
 P.CornerSharedD6 = V.bHasCornerSharedParticipantD6 ? V.CornerSharedParticipantD6 : 0;
 P.CornerRouteD6 = V.bHasCornerRouteD6 ? V.CornerRouteD6 : 0;
 P.CornerIntendedRoute = V.CornerIntendedRoute; P.CornerActualRoute = V.CornerActualRoute;
 P.CornerBonusSide = V.CornerCandidateBonusSide; P.CornerBonus = V.CornerCandidateBonus;
 if (P.CornerAttackers.Num() > 3 || P.CornerDefenders.Num() > 3
  || P.CornerAttackerRollLabels.Num() > 3 || P.CornerDefenderRollLabels.Num() > 3) { P.bOptionsUnavailable = true; P.DisableActions(); return P; }
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
 if (P.bTakerWait || P.bCornerDraft)
 {
  const auto& ChoiceRoster = V.ExpectedActingPlayer == EInitialTurnOrderPlayer::PlayerA ? V.PlayerACardRoster : V.PlayerBCardRoster;
  TSet<FName> Seen;
  if (V.LegalSetPieceCardIds.Num() > MaxTakers) P.bOptionsUnavailable = true;
  for (FName Id : V.LegalSetPieceCardIds)
  {
   const auto* Card = ChoiceRoster.FindByPredicate([&](const auto& C){return C.CardId == Id;});
   if (Id.IsNone() || Id.ToString().Len() > 128 || Seen.Contains(Id) || !Card || !Card->bAvailable || Card->bGoalkeeper || Card->bUsed || Card->bEjected)
    P.bOptionsUnavailable = true;
   Seen.Add(Id);
  }
  if (!P.bOptionsUnavailable)
  {
   if (P.bCornerDraft) P.CornerOptions = V.LegalSetPieceCardIds;
   else P.TakerOptions = V.LegalSetPieceCardIds;
  }
 }
 if (P.bMethodWait)
 {
  P.NearMethods = V.LegalNearMethods; P.LongMethods = V.LegalLongMethods; P.PenaltyMethods = V.LegalPenaltyMethods;
 }
 return P;
}
