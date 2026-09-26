// Explicit opt-in DEV UI driver for the Stage 8.11A.2 two-process evidence run.
// No authority access, injected View/ACK, transport shortcut, or production dependency.
#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "FMCodexNetworkMatchPlayerController.h"
#include "../LocalPlay/FMCodexLocalMatchScreenWidget.h"
#include "../LocalPlay/FMCodexCardRackWidget.h"
#include "../LocalPlay/FMCodexPlayerCardWidget.h"
#include "Components/Button.h"
#include "Containers/Ticker.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "HAL/PlatformMisc.h"

namespace
{
FAutoConsoleCommand NearInspectionEvidence(TEXT("fm.Dev.NearInspectionEvidence"),
 TEXT("Opt-in real-process Near taker inspection UI evidence; use the server-only Near RNG fixture."),
 FConsoleCommandDelegate::CreateLambda([]
 {
  FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda(
   [Step=0,Started=FPlatformTime::Seconds(),Last=0.,Done=0.,Chosen=FName(),bWaitingSeen=false](float) mutable
   {
    const double Now=FPlatformTime::Seconds();
    if (Done>0.) {if (Now-Done>8.) {FPlatformMisc::RequestExit(false);return false;}return true;}
    auto Fail=[&](const TCHAR* Reason){UE_LOG(LogTemp,Error,TEXT("NEAR_INSPECTION FAIL %s"),Reason);Done=Now;return true;};
    if (Now-Started>110.) return Fail(TEXT("timeout"));
    UWorld* World=nullptr;
    for (const auto& Context:GEngine->GetWorldContexts()) if (Context.WorldType==EWorldType::Game) {World=Context.World();break;}
    auto* PC=World?Cast<AFMCodexNetworkMatchPlayerController>(World->GetFirstPlayerController()):nullptr;
    auto* Screen=PC?PC->GetPlayerMatchScreen():nullptr;
    if (!Screen || PC->GetOwnerView().BootstrapState!=EFMCodexNetworkBootstrapState::MatchReady
     || PC->IsScreenIntentPending() || Screen->IsInlineFormulaRevealInputBlocked() || Now-Last<1.) return true;
    const auto& View=PC->GetOwnerView();const auto& P=Screen->GetPresentation().SetPiece;
    auto Shown=[&](const TCHAR* Name){const auto* W=Screen->GetWidgetFromName(Name);return W && W->GetVisibility()!=ESlateVisibility::Collapsed && W->GetVisibility()!=ESlateVisibility::Hidden;};
    auto Click=[&](const TCHAR* Name){auto* B=Cast<UButton>(Screen->GetWidgetFromName(Name));if (!B || !B->GetIsEnabled()) return false;B->OnClicked.Broadcast();return true;};
    const bool Actor=View.ViewerSide==View.CurrentAttackingSide;
    if (!Actor)
    {
     if (!P.NearTakerEligibility.IsEmpty() || !View.SetPiece.NearTakerEligibility.IsEmpty() || Shown(TEXT("TheaterTakerInspector"))) return Fail(TEXT("waiting viewer leaked candidates"));
     if (P.bTakerWait && !bWaitingSeen) {bWaitingSeen=true;UE_LOG(LogTemp,Display,TEXT("NEAR_INSPECTION waiting Side=%d Revision=%d Candidates=0 Eligibility=0 Inspector=0"),int32(View.ViewerSide),View.ViewRevision);}
    }
    if (P.NearMethod==EMatchPlayShortFreeKickMethod::Direct)
    {
     if (Shown(TEXT("TheaterTakerInspector")) || !P.NearTakerEligibility.IsEmpty()) return Fail(TEXT("inspector/facts survived method"));
     if (!Actor && !bWaitingSeen) return Fail(TEXT("waiting selection not observed"));
     UE_LOG(LogTemp,Display,TEXT("NEAR_INSPECTION PASS Side=%d Revision=%d Sequence=%lld Actor=%d Pending=0 Method=Direct Inspector=0"),int32(View.ViewerSide),View.ViewRevision,View.AttackSequence,Actor);
     PC->DevPlayerFacingEvidence();Done=Now;return true;
    }
    if (!Actor) return true;
    if (Step==0 && Screen->GetPresentation().Interaction.bCanRollTacticalPoints) {PC->DevPlayerFacingAction(TEXT("D12"),NAME_None);++Step;Last=Now;return true;}
    if (Step==1 && P.bCanRollType) {PC->DevPlayerFacingAction(TEXT("SetPieceType"),NAME_None);++Step;Last=Now;return true;}
    if (Step>=2 && Step<=4 && P.bTakerWait && Shown(TEXT("TheaterTakerBounds")))
    {
     auto* Rack=CastChecked<UFMCodexCardRackWidget>(Screen->GetWidgetFromName(TEXT("TheaterTakers")));
     auto* Full=CastChecked<UFMCodexPlayerCardWidget>(Screen->GetWidgetFromName(TEXT("TheaterTakerFullCard")));
     const auto* Eligible=P.NearTakerEligibility.FindByPredicate([](const auto& F){return F.bCanUseTacticalCombination;});
     const auto* Ineligible=P.NearTakerEligibility.FindByPredicate([](const auto& F){return !F.bCanUseTacticalCombination;});
     if (!Eligible || !Ineligible || P.NearTakerEligibility.Num()!=P.TakerOptions.Num()) return Fail(TEXT("incomplete replicated candidate facts"));
     const FName Id=Step==2?Ineligible->CardId:Eligible->CardId;
     const auto* Card=Rack->GetRenderedCardWidgets().FindByPredicate([Id](const auto& C){return C->GetPresentation().CardId==Id;});
     if (!Card) return Fail(TEXT("missing safe candidate"));
     if (Step==2)
     {
      UE_LOG(LogTemp,Display,TEXT("NEAR_INSPECTION actor Side=%d Revision=%d Candidates=%d Eligibility=%d Eligible=%s Ineligible=%s"),int32(View.ViewerSide),View.ViewRevision,P.TakerOptions.Num(),P.NearTakerEligibility.Num(),*Eligible->CardId.ToString(),*Ineligible->CardId.ToString());
      (*Card)->RequestFullCardDetailHover();
      if (Full->GetPresentation().CardId!=Id) return Fail(TEXT("remote hover did not show canonical full card"));
      (*Card)->OnDetailHoverDismissed.Broadcast(Card->Get());
     }
     else if (Step==3) {Chosen=Id;(*Card)->RequestOnPitchSelection();}
     else
     {
      if (Full->GetPresentation().CardId!=Chosen || !View.SetPiece.TakerCardId.IsNone()) return Fail(TEXT("draft inspection persistence/authority"));
      if (!Click(TEXT("TheaterContinue"))) return Fail(TEXT("confirm unavailable"));
     }
     ++Step;Last=Now;return true;
    }
    if (Step==5 && P.bMethodWait)
    {
     if (P.TakerCardId!=Chosen || !P.NearMethods.Contains(EMatchPlayShortFreeKickMethod::Angled) || Shown(TEXT("TheaterTakerInspector"))) return Fail(TEXT("preview/method mismatch"));
     UE_LOG(LogTemp,Display,TEXT("NEAR_INSPECTION confirmed Side=%d Revision=%d Taker=%s PreviewEligible=1 MethodAngled=1 Inspector=0"),int32(View.ViewerSide),View.ViewRevision,*Chosen.ToString());
     if (!Click(TEXT("TheaterNearDirect"))) return Fail(TEXT("direct unavailable"));
     ++Step;Last=Now;
    }
    return true;
   }),.1f);
 }));
}
#endif
