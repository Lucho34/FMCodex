#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && !UE_BUILD_SHIPPING
#include "FMCodexInlineResolutionFormulaSurfaceWidget.h"
#include "FMCodexMatchHeaderWidget.h"
#include "FMCodexRollReelWidget.h"
#include "FMCodexPitchWidget.h"
#include "FMCodexCardRackWidget.h"
#include "FMCodexPlayerCardWidget.h"
#include "FMCodexPitchSlotWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"
#include "Components/RichTextBlock.h"
#include "FMCodexLocalMatchPlayerController.h"
#include "FMCodexLocalMatchScreenWidget.h"
#include "Editor.h"
#include "Components/Button.h"
#include "Components/Border.h"
#include "Components/SizeBox.h"
#include "Components/Image.h"
#include "Fonts/FontMeasure.h"
#include "Rendering/SlateRenderer.h"
#include "Engine/Texture2D.h"
#include "UObject/UnrealType.h"
#include "HAL/IConsoleManager.h"
#include "Engine/World.h"
#include "Engine/GameViewportClient.h"
#include "Framework/Application/SlateApplication.h"
#include "HAL/FileManager.h"
#include "ImageUtils.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "PlayInEditorDataTypes.h"
#include "Settings/LevelEditorPlaySettings.h"
#include "Tests/AutomationCommon.h"
#include "Tests/AutomationEditorCommon.h"
#include "Widgets/SWindow.h"
namespace
{
 TSharedPtr<SWindow> TheaterPIEWindow;

	bool DeployNextOrdinary(
		AFMCodexLocalMatchPlayerController& Controller,
		const FString& SlotFragment)
	{
		Controller.RefreshPresentation();
		const FFMCodexLocalMatchInteractionView& View =
			Controller.GetInteractionView();
		const bool bDeployingAttacker =
			View.CurrentLegalDeploymentSide == View.CurrentAttackingPlayer;
		FName PreferredCardId = NAME_None;
		if (bDeployingAttacker)
		{
			const FName RequiredCarrierId = View.CurrentAttackingPlayer
				== EInitialTurnOrderPlayer::PlayerA
					? FName(TEXT("Prototype.Arsenal.BukayoSaka"))
					: FName(TEXT("Prototype.ManchesterCity.RayanAitNouri"));
			const FName RequiredRunnerId = View.CurrentAttackingPlayer
				== EInitialTurnOrderPlayer::PlayerA
					? FName(TEXT("Prototype.Arsenal.ViktorGyokeres"))
					: FName(TEXT("Prototype.ManchesterCity.ErlingHaaland"));
			const TArray<FFMCodexLocalMatchCardView>& Roster =
				View.CurrentAttackingPlayer == EInitialTurnOrderPlayer::PlayerA
					? View.PlayerACardRoster : View.PlayerBCardRoster;
			const bool bCarrierDeployed = Roster.ContainsByPredicate(
				[RequiredCarrierId](const FFMCodexLocalMatchCardView& Card)
				{
					return Card.bDeployed && Card.CardId == RequiredCarrierId;
				});
			PreferredCardId = bCarrierDeployed
				? RequiredRunnerId : RequiredCarrierId;
		}

		else
		{
			// Legal, reference-comparable participants; only existing deployment intents.
			const bool bDefenderA = View.CurrentLegalDeploymentSide == EInitialTurnOrderPlayer::PlayerA;
			const FName Marker = bDefenderA ? FName(TEXT("Prototype.Arsenal.WilliamSaliba")) : FName(TEXT("Prototype.ManchesterCity.JohnStones"));
			const FName Helper = bDefenderA ? FName(TEXT("Prototype.Arsenal.BenWhite")) : FName(TEXT("Prototype.ManchesterCity.NathanAke"));
			const auto& Roster = bDefenderA ? View.PlayerACardRoster : View.PlayerBCardRoster;
			PreferredCardId = Roster.ContainsByPredicate([Marker](const FFMCodexLocalMatchCardView& Card)
				{ return Card.bDeployed && Card.CardId==Marker; }) ? Helper : Marker;
		}

		const FFMCodexLocalMatchDeploymentOption* Option =
			View.DeploymentOptions.FindByPredicate(
				[PreferredCardId, &SlotFragment](
					const FFMCodexLocalMatchDeploymentOption& Candidate)
				{
					return !Candidate.bGoalkeeper
						&& Candidate.SlotId.ToString().Contains(SlotFragment)
						&& (PreferredCardId.IsNone()
							|| Candidate.CardId == PreferredCardId);
				});
		if (Option == nullptr && !PreferredCardId.IsNone())
		{
			Option = View.DeploymentOptions.FindByPredicate(
				[&SlotFragment](
					const FFMCodexLocalMatchDeploymentOption& Candidate)
				{
					return !Candidate.bGoalkeeper
						&& Candidate.SlotId.ToString().Contains(SlotFragment);
				});
		}
		if (Option == nullptr)
		{
			return false;
		}
		Controller.DeployOrdinary(Option->CardId, Option->SlotId);
		return Controller.GetLastDiagnostic().bHostSuccess;
	}

	bool SubmitFirst(
		AFMCodexLocalMatchPlayerController& Controller,
		const EFMCodexLocalMatchInteractionCategory Expected, FName PreferredId = NAME_None)
	{
		Controller.RefreshPresentation();
		const FFMCodexLocalMatchInteractionView& View =
			Controller.GetInteractionView();
		if (View.InteractionCategory != Expected
			|| View.SelectionOptions.IsEmpty())
		{
			return false;
		}
		const auto* Preferred = View.SelectionOptions.FindByPredicate([PreferredId](const auto& Option) { return Option.Id == PreferredId; });
		const FName Id = Preferred ? Preferred->Id : View.SelectionOptions[0].Id;
		switch (Expected)
		{
		case EFMCodexLocalMatchInteractionCategory::SelectMarker:
			Controller.SubmitMarker(Id);
			break;
		case EFMCodexLocalMatchInteractionCategory::SelectRunner:
			Controller.SubmitRunner(Id);
			break;
		case EFMCodexLocalMatchInteractionCategory::SelectHelper:
			Controller.SubmitHelper(Id);
			break;
		default:
			return false;
		}
		return Controller.GetLastDiagnostic().bHostSuccess;
	}



class FStartTheaterPIE final : public IAutomationLatentCommand
{
public:
 bool Update() override
 {
  auto* Settings=DuplicateObject<ULevelEditorPlaySettings>(GetDefault<ULevelEditorPlaySettings>(),GetTransientPackage());
  Settings->NewWindowWidth=1920; Settings->NewWindowHeight=1080;
  Settings->SetPlayNetMode(EPlayNetMode::PIE_Standalone); Settings->SetPlayNumberOfClients(1);
  TheaterPIEWindow=SNew(SWindow).Title(FText::FromString(TEXT("Resolution Theater PIE")))
   .ClientSize(FVector2D(1920,1080)).ScreenPosition(FVector2D(0,0)).AutoCenter(EAutoCenter::None)
   .SaneWindowPlacement(false).AdjustInitialSizeAndPositionForDPIScale(false).SizingRule(ESizingRule::UserSized);
  FSlateApplication::Get().AddWindow(TheaterPIEWindow.ToSharedRef());
  FRequestPlaySessionParams Params; Params.EditorPlaySettings=Settings; Params.CustomPIEWindow=TheaterPIEWindow;
  GEditor->RequestPlaySession(Params); return true;
 }
};
class FTheaterPIE final : public IAutomationLatentCommand
{
public:
 explicit FTheaterPIE(FAutomationTestBase* InTest, bool InLow=false, bool InRouteOnly=false, bool InCompact=false):Test(InTest),bLow(InLow),bRouteOnly(InRouteOnly),bCompact(InCompact)
 {
  Mode=IConsoleManager::Get().FindConsoleVariable(TEXT("fm.UI.ResolutionStageV2")); Previous=Mode->GetInt(); Test->TestEqual(TEXT("Fresh PIE requires no theater enable command"),Previous,1);
 }
 ~FTheaterPIE() { Mode->Set(Previous,ECVF_SetByCode); }
 bool Update() override
 {
  if (FPlatformTime::Seconds()-Start>150) { Test->AddError(TEXT("Theater PIE timed out")); return true; }
  if (!GEditor || !GEditor->PlayWorld) return false;
  auto* C=Cast<AFMCodexLocalMatchPlayerController>(GEditor->PlayWorld->GetFirstPlayerController());
  auto* S=C ? C->GetPlayerMatchScreen() : nullptr;
  if (!S) return false;
  auto Visible=[&](const TCHAR* Name)
  {
   auto* W=S->GetWidgetFromName(Name);
   return W && W->GetVisibility()!=ESlateVisibility::Collapsed && W->GetVisibility()!=ESlateVisibility::Hidden;
  };
  auto Text=[&](const TCHAR* Name) { return CastChecked<UTextBlock>(S->GetWidgetFromName(Name))->GetText().ToString(); };
  if (bCompact && Step==5)
  {
   auto* R=CastChecked<UFMCodexRollReelWidget>(S->GetWidgetFromName(TEXT("TheaterReel")));
   const auto& P=S->GetInlineFormulaSurface()->GetPresentation();
   const auto& Reel=R->GetPresentation();
   const int32 Phase=int32(S->GetInlineFormulaRevealPhase());
   Test->TestEqual(TEXT("Route keeps the same public goalkeeper"),Text(TEXT("TheaterDefenseName2")),KeeperName);
   if (GEditor->PlayWorld->GetTimeSeconds()-LastMovieTime>=.035f && MovieFrames.Num()<100) CaptureMovieFrame(S);
   if (Phase!=LastRoutePhase)
   {
    LastRoutePhase=Phase;
    Test->AddInfo(FString::Printf(TEXT("COMPACT_ROUTE game=%.3f phase=%d digit=%d static=%d title=%s route=%s"),
     GEditor->PlayWorld->GetTimeSeconds(),Phase,Reel.CenterValue,Reel.bStaticResult,*Text(TEXT("TheaterTitle")),*P.RouteResultLabel));
   }
   if (Reel.bMoving)
   {
    Test->TestEqual(TEXT("Route uses CompactBox in real PIE"),R->GetVisualVariant(),EFMCodexRollVisualVariant::CompactBox);
    Test->TestTrue(TEXT("Natural route remains private until landing"),P.RouteResultLabel.IsEmpty() && Text(TEXT("TheaterTitle"))==TEXT("传中"));
    Test->TestFalse(TEXT("Rolling route has no actionable CTA"),Visible(TEXT("TheaterPrimaryBounds")));
    Test->TestEqual(TEXT("Real rolling message"),Text(TEXT("TheaterDetail")),FString(TEXT("正在判定传中路线")));
    if (!bRouteMoving && Reel.ContinuousPositionCells>2.f)
    { bRouteMoving=true; CaptureRollFrame(S,TEXT("03_Route_CompactBox_Rolling_Message.png")); }
   }
   if (Reel.bStaticResult && !bRouteLanded)
   {
    bRouteLanded=true;
    Test->TestEqual(TEXT("Real landed copy is gated authoritative result"),Text(TEXT("TheaterDetail")),FString(TEXT("掷点结果为 5，判定为低球传中")));
    Test->TestEqual(TEXT("Route lands on provider result"),Reel.CenterValue,bLow?5:2);
    Test->TestTrue(TEXT("Landed route retains one crisp digit"),R->IsStaticResultTileVisible());
    CaptureRollFrame(S,TEXT("04_Route_Landed_ResultMessage.png"));
   }
  }
  if (bLow && Step==5)
  {
   Test->TestTrue(TEXT("No board bounce throughout real Low route hold"),Visible(TEXT("TheaterContent")));
   Test->TestEqual(TEXT("Board input stays suppressed across Low disclosure"),S->GetWidgetFromName(TEXT("MatchShellViewportFit"))->GetVisibility(),ESlateVisibility::HitTestInvisible);
   const auto& Display=S->GetInlineFormulaSurface()->GetPresentation();
   if (Display.ContestId==TEXT("Cross.Route") && Display.RouteResultLabel.IsEmpty())
    Test->TestEqual(TEXT("Hidden actual Low cannot leak into title"),Text(TEXT("TheaterTitle")),FString(TEXT("传中")));
  }
  if (Step==8 || Step==9)
  {
   CheckStableEquation(S,false);
   if (!bLow && Step==8 && GEditor->PlayWorld->GetTimeSeconds()-LastMovieTime>=.025f && MovieFrames.Num()<100) CaptureMovieFrame(S);
   const int32 Phase=int32(S->GetInlineFormulaRevealPhase());
   const auto& P=S->GetInlineFormulaSurface()->GetPresentation();
   const auto& Reel=CastChecked<UFMCodexRollReelWidget>(S->GetWidgetFromName(Step==8?TEXT("TheaterAttackReel"):TEXT("TheaterDefenseReel")))->GetPresentation();
   const int32 Disclosed=P.AttackRow.bDisplayedResultIsFinalValue + 2*P.DefenseRow.bDisplayedResultIsFinalValue;
   if (Phase!=LastRollPhase || Step!=LastRollStep || Disclosed!=LastDisclosed ||
    (Reel.bMoving && FPlatformTime::Seconds()-LastRollLog>.12))
   {
    LastRollPhase=Phase; LastRollStep=Step; LastDisclosed=Disclosed; LastRollLog=FPlatformTime::Seconds();
    Test->AddInfo(FString::Printf(TEXT("ROLL_V2_FRAME game=%.3f side=%s phase=%d position=%.3f digit=%d static=%d rhsFade=%.3f attack=%s defense=%s finalFlags=%d CTA=%d outcome=%d"),
     GEditor->PlayWorld->GetTimeSeconds(),Step==8?TEXT("Attack"):TEXT("Defense"),Phase,Reel.ContinuousPositionCells,Reel.CenterValue,Reel.bStaticResult,Reel.FormulaFinalRevealProgress,
     *Text(TEXT("TheaterAttackFinalNumber")),*Text(TEXT("TheaterDefenseFinalNumber")),Disclosed,Visible(TEXT("TheaterPrimaryBounds")),Visible(TEXT("TheaterOutcome"))));
   }
   if (Step==8 && Reel.bStaticResult && Reel.FormulaFinalRevealProgress<0.f && !(RollFrames&8))
   {
    RollFrames|=8;
   }
   if (Step==8 && Reel.FormulaFinalRevealProgress>=1.f && !(RollFrames&16))
   {
    RollFrames|=16;
   }
   if (Reel.bMoving && Reel.ContinuousPositionCells>1.f && !(RollFrames & (Step==8?1:2)))
   {
    RollFrames |= Step==8?1:2;
    if (Step==9) CaptureRollFrame(S,TEXT("04_DefenseRolling.png"));
   }
   if (Step==9 && P.DefenseRow.bDisplayedResultIsFinalValue && !Visible(TEXT("TheaterOutcome")) && !(RollFrames&4))
   {
    RollFrames|=4; CaptureRollFrame(S,TEXT("05_BothResolved.png"));
   }
  }
  if (Step==3)
  {
   const double EntryTime=FPlatformTime::Seconds()-Changed;
   int32 MotionIndex=0;
   for (const auto Name:{TEXT("TheaterAttackPanel"),TEXT("TheaterDefensePanel"),TEXT("TheaterVS"),TEXT("TheaterBottom")})
   {
    const uint8 Bit=1<<MotionIndex++;
    if (!(MotionSeen&Bit) && S->GetWidgetFromName(Name)->GetRenderOpacity()>0.f)
    {
     MotionSeen|=Bit;
     Test->AddInfo(FString::Printf(TEXT("THEATER_ARRIVAL %s game=%.3f"),Name,GEditor->PlayWorld->GetTimeSeconds()));
    }
   }
   if (EntryTime>=NextMotionSample && EntryTime<.95)
   {
    Test->AddInfo(FString::Printf(TEXT("THEATER_MOTION t=%.3f attack=%.3f defense=%.3f vs=%.3f action=%.3f fieldScale=%.3f"),
     EntryTime,S->GetWidgetFromName(TEXT("TheaterAttackPanel"))->GetRenderOpacity(),S->GetWidgetFromName(TEXT("TheaterDefensePanel"))->GetRenderOpacity(),
     S->GetWidgetFromName(TEXT("TheaterVS"))->GetRenderOpacity(),S->GetWidgetFromName(TEXT("TheaterBottom"))->GetRenderOpacity(),S->GetPitchWidget()->GetRenderTransform().Scale.X));
    NextMotionSample=EntryTime+.12;
   }

  }
  if (Step==8 && S->GetInlineFormulaRevealPhase()==EFMCodexUMGInlineFormulaRevealPhase::Cycling && !bRollCaptured)
  {
   Test->TestFalse(TEXT("Natural rolling keeps outcome hidden"),Visible(TEXT("TheaterOutcome")));
   Test->TestTrue(TEXT("Natural rolling retains formula operands"),Visible(TEXT("TheaterAttackValue")));
   Test->TestTrue(TEXT("Reel replaces only the Attack question mark"),Visible(TEXT("TheaterAttackReelHost")) && !Visible(TEXT("TheaterAttackPending")) && Visible(TEXT("TheaterDefensePending")));
   Test->TestFalse(TEXT("No disconnected Formula roll in action lane"),Visible(TEXT("TheaterRoll")));
   Test->TestTrue(TEXT("Compact roll info bar remains visible without enclosing CTA"),Visible(TEXT("TheaterLaneGlass")));
   QuestionPosition=S->GetWidgetFromName(TEXT("TheaterAttackUnknownSlot"))->GetCachedGeometry().GetAbsolutePosition();
   bRollCaptured=true;
   if (bCompact) CaptureRollFrame(S,TEXT("06_TheaterInline_Regression.png"));
  }
  if (Step==8 && S->GetInlineFormulaRevealPhase()==EFMCodexUMGInlineFormulaRevealPhase::ResultHold && !bInlineHoldChecked)
  {
   auto* Reel=CastChecked<UFMCodexRollReelWidget>(S->GetWidgetFromName(TEXT("TheaterAttackReel")));
   Test->TestTrue(TEXT("Same inline chamber holds one authoritative result"),Reel->IsStaticResultTileVisible());
   Test->TestEqual(TEXT("Inline reel lands on DEV provider result, not cosmetic number"),Reel->GetPresentation().CenterValue,bLow?6:4);
   Test->TestTrue(TEXT("Question slot stays in place through landing"),QuestionPosition.Equals(S->GetWidgetFromName(TEXT("TheaterAttackUnknownSlot"))->GetCachedGeometry().GetAbsolutePosition(),1.f));
   const auto& Display=S->GetInlineFormulaSurface()->GetPresentation();
   // Screenshot readback may skip a short display window. Exact .18s gating
   // is verified by UnifiedCoveredRolls; this path logs natural elapsed frames.
   Test->AddInfo(FString::Printf(TEXT("ROLL_V2_FIRST_STATIC finalAllowed=%d"),Display.AttackRow.bDisplayedResultIsFinalValue));
   Test->TestTrue(TEXT("Real High formula uses v2 skin"),Reel->UsesTheaterInlineSkin());
   bInlineHoldChecked=true;
  }
  if (Step==9 && S->GetInlineFormulaRevealPhase()==EFMCodexUMGInlineFormulaRevealPhase::Cycling)
  {
   if (!bScoreChecked)
   {
    Test->TestEqual(TEXT("Real defense roll holds prior visible score"),S->GetMatchHeader()->GetDisplayedScoreLabel(),BeforeScore);
    Test->TestFalse(TEXT("No premature outcome headline"),Visible(TEXT("TheaterOutcome")));
    Test->TestFalse(TEXT("No premature continue action"),Visible(TEXT("TheaterPrimaryBounds")));
    bScoreChecked=true;
   }
  }
  if (Step==9 && S->GetInlineFormulaRevealPhase()==EFMCodexUMGInlineFormulaRevealPhase::ResultHold
   && S->GetInlineFormulaSurface()->GetPresentation().bNarrativeAvailable && !bHoldChecked)
  {
   Test->TestFalse(TEXT("Outcome Option A holds headline until reel exits"),Visible(TEXT("TheaterOutcome")));
   Test->TestEqual(TEXT("Hold status retains operation, never terminal alias"),Text(TEXT("TheaterStatus")),S->GetInlineFormulaSurface()->GetPresentation().DiceOwnerLabel);
   bHoldChecked=true;
  }
  if (FPlatformTime::Seconds()-Changed<.95) return false;
  if (Step==0) { S->RequestStartNewMatch(); Next(); return false; }
  if (Step==1) { if (!Override(*C,EFMCodexLocalDevRollTarget::FullD12,6)) return true; S->RequestRollTacticalPoints(); Next(); return false; }
  if (S->IsInlineFormulaRevealInputBlocked()) return false; // Natural elapsed game time only.
  if (Step==2)
  {
   const auto Attacker=C->GetInteractionView().CurrentAttackingPlayer;
   const FString Forward=Attacker==EInitialTurnOrderPlayer::PlayerA ? TEXT("NearB") : TEXT("NearA");
   for (int32 I=0;I<4;++I) if (!DeployNextOrdinary(*C,Forward)) { Test->AddError(TEXT("Legal deployment failed")); return true; }
   C->FinishDeployment();
   if (bCompact)
   {
    const auto* Keeper=C->GetInteractionView().DeploymentOptions.FindByPredicate([](const auto& O){ return O.bGoalkeeper; });
    if (!Test->TestNotNull(TEXT("Canonical defender offers goalkeeper deployment"),Keeper)) return true;
    const FName KeeperSlot=Keeper->SlotId; C->DeployGoalkeeper(KeeperSlot);
    if (!Test->TestTrue(TEXT("Real goalkeeper deployment accepted"),C->GetLastDiagnostic().bHostSuccess)) return true;
   }
   C->FinishDeployment();
   S->RequestSubmitCarrier(Attacker==EInitialTurnOrderPlayer::PlayerA ? FName(TEXT("Prototype.Arsenal.BukayoSaka")) : FName(TEXT("Prototype.ManchesterCity.RayanAitNouri")));
   if (!SubmitFirst(*C,EFMCodexLocalMatchInteractionCategory::SelectMarker,Attacker==EInitialTurnOrderPlayer::PlayerA ? FName(TEXT("Prototype.ManchesterCity.JohnStones")) : FName(TEXT("Prototype.Arsenal.WilliamSaliba")))
    || !SubmitFirst(*C,EFMCodexLocalMatchInteractionCategory::SelectRunner)) { Test->AddError(TEXT("Legal role selection failed")); return true; }
   if (!C->GetInteractionView().SelectionOptions.IsEmpty())
   {
    if (!SubmitFirst(*C,EFMCodexLocalMatchInteractionCategory::SelectHelper)) { Test->AddError(TEXT("Legal helper selection failed")); return true; }
   }
   else if (C->GetInteractionView().bCanResolveNoLegalChoice) C->ResolveNoLegalCurrentSelection();
   else C->DeclineCurrentSelection();
   Test->TestFalse(TEXT("Theater does not precede tactical choice"),Visible(TEXT("ResolutionTheater")));
   S->RequestSubmitSkill(TEXT("Canonical.Skill.Cross.4.6")); Next(); return false;
  }
  if (Step==3)
  {
   if (S->GetWidgetFromName(TEXT("TheaterBottom"))->GetRenderOpacity()<.99f) return false;
   if (!Test->TestTrue(TEXT("Immediate post-tactical entry"),Visible(TEXT("ResolutionTheater")) && Visible(TEXT("MatchShellViewportFit")))) return true;
   Test->TestEqual(TEXT("Racks receded while real pitch remains"),S->GetWidgetFromName(TEXT("LocalPlayerCardRackRegion"))->GetRenderOpacity(),0.f);
   Test->TestTrue(TEXT("Existing pitch transforms into theater field"),S->GetPitchWidget()->GetRenderTransform().Scale.X>1.f);
   for (const auto& Slot:S->GetPitchWidget()->GetRenderedSlotWidgets()) Test->TestEqual(TEXT("No pitch cards or empty slots remain visible"),Slot->GetVisibility(),ESlateVisibility::Hidden);
   Test->TestEqual(TEXT("Top-down pitch geometry is fully suppressed in theater"),S->GetPitchWidget()->WidgetTree->FindWidget(TEXT("TwoLanePitchCanvas"))->GetRenderOpacity(),0.f);
   Test->TestEqual(TEXT("Old overlays cannot paint through glass"),S->GetWidgetFromName(TEXT("BoardResolutionOverlays"))->GetVisibility(),ESlateVisibility::Hidden);
   Test->TestEqual(TEXT("Entry is honest about unknown route"),Text(TEXT("TheaterTitle")),FString(TEXT("传中")));
   Test->TestEqual(TEXT("Carrier role readable"),Text(TEXT("TheaterAttackRole0")),FString(TEXT("持球")));
   Test->TestEqual(TEXT("Runner role readable"),Text(TEXT("TheaterAttackRole1")),FString(TEXT("跑位")));
   Test->TestEqual(TEXT("Marker role readable"),Text(TEXT("TheaterDefenseRole0")),FString(TEXT("盯人")));
   Test->TestEqual(TEXT("Helper role readable"),Text(TEXT("TheaterDefenseRole1")),FString(TEXT("协防")));
   ParticipantHeight=S->GetWidgetFromName(TEXT("TheaterAttackPanel"))->GetCachedGeometry().GetAbsoluteSize().Y;
   Test->TestTrue(TEXT("Participants use compact natural content height"),ParticipantHeight<260.f);
   if (bCompact)
   {
    Test->TestEqual(TEXT("Known activated keeper is present before branch choice"),Text(TEXT("TheaterDefenseRole2")),FString(TEXT("门将")));
    KeeperName=Text(TEXT("TheaterDefenseName2"));
    Test->TestFalse(TEXT("Actual keeper has player-facing name"),KeeperName.IsEmpty());
    CaptureRollFrame(S,TEXT("01_RouteParticipants_WithGoalkeeper.png"));
   }
   else if (bLow) CaptureRollFrame(S,TEXT("00_CrossNeutral.png"));
   CastChecked<UButton>(S->GetWidgetFromName(TEXT("TheaterHigh")))->OnClicked.Broadcast();
   if (!Override(*C,EFMCodexLocalDevRollTarget::CrossRoute,bLow?5:2)
    || !Override(*C,bLow?EFMCodexLocalDevRollTarget::CrossLowAttack:EFMCodexLocalDevRollTarget::CrossHighAttack,bLow?6:4)
    || !Override(*C,bLow?EFMCodexLocalDevRollTarget::CrossLowDefense:EFMCodexLocalDevRollTarget::CrossHighDefense,bLow?2:3)) return true;
   Next(); return false;
  }
  if (Step==4)
  {
   if (!Test->TestTrue(TEXT("Natural route action available"),C->GetInteractionView().InteractionCategory==EFMCodexLocalMatchInteractionCategory::RollCrossRoute)) return true;
   if (bCompact)
   {
    Test->TestFalse(TEXT("Pre-action route cell stays hidden, no fake result"),Visible(TEXT("TheaterRoll")));
    Test->TestEqual(TEXT("Pre-roll message retains canonical hint"),Text(TEXT("TheaterDetail")),S->GetInlineFormulaSurface()->GetPresentation().RollHelperLabel);
    CaptureRollFrame(S,TEXT("02_Route_PreRoll_Message.png")); CaptureMovieFrame(S);
   }
   Click(S); Next(); return false;
  }
  if (Step==5)
  {
   Test->TestTrue(TEXT("Cross formula retains same theater"),Visible(TEXT("ResolutionTheater")) && Visible(TEXT("TheaterAttackPending")));
   Test->TestEqual(TEXT("Real actual Cross formula"),S->GetInlineFormulaSurface()->GetPresentation().ContestId,FName(bLow?TEXT("Cross.Low"):TEXT("Cross.High")));
   Test->TestEqual(TEXT("Disclosed tactical title"),Text(TEXT("TheaterTitle")),FString(bLow?TEXT("低球传中"):TEXT("高球传中")));
   if (bCompact)
   {
    Test->TestTrue(TEXT("Real route visited rolling and landing"),bRouteMoving && bRouteLanded);
    Test->TestEqual(TEXT("Formula keeps the same goalkeeper"),Text(TEXT("TheaterDefenseName2")),KeeperName);
    CaptureRollFrame(S,TEXT("05_Route_ToFormula_ParticipantContinuity.png")); SaveMovieFrames();
   }
   if (bRouteOnly) return true;
   Test->TestEqual(TEXT("Current subtotal is authority projection"),Text(TEXT("TheaterAttackNumber")),S->GetInlineFormulaSurface()->GetPresentation().AttackRow.DisplayedResultLabel);
   const float FormulaHeight=S->GetWidgetFromName(TEXT("TheaterAttackPanel"))->GetCachedGeometry().GetAbsoluteSize().Y;
   Test->TestTrue(TEXT("Formula expands with content without the old giant slab"),FormulaHeight>ParticipantHeight && FormulaHeight<460.f);
   Test->TestFalse(TEXT("No final divider during unresolved formula"),Visible(TEXT("TheaterOutcomeDivider")));
   CaptureRollFrame(S,TEXT("01_PreRoll.png"));
   CheckEquation(S);
   CheckNumericWidths(S);
   for (const auto Prefix:{TEXT("TheaterAttack"),TEXT("TheaterDefense")})
   {
    const auto Base=S->GetWidgetFromName(FName(*(FString(Prefix)+TEXT("BaseHover"))))->GetCachedGeometry();
    auto* Line=CastChecked<UBorder>(S->GetWidgetFromName(FName(*(FString(Prefix)+TEXT("BaseUnderline")))));
    const auto LineGeometry=Line->GetCachedGeometry();
    const auto Number=S->GetWidgetFromName(FName(*(FString(Prefix)+TEXT("Number"))))->GetCachedGeometry();
    Test->TestTrue(TEXT("Solid inspect cue is at least two rendered pixels"),LineGeometry.GetAbsoluteSize().Y>=2.f);
    Test->TestTrue(TEXT("Inspect cue is close to numeric layout instead of old 76px cell"),
     LineGeometry.GetAbsolutePosition().Y-(Number.GetAbsolutePosition().Y+Number.GetAbsoluteSize().Y)<6.f);
    Test->TestTrue(TEXT("Single solid inspect cue is narrower than base and centered"),Line->GetChildrenCount()==0
     && LineGeometry.GetAbsoluteSize().X<Base.GetAbsoluteSize().X
     && FMath::Abs(LineGeometry.GetAbsolutePosition().X+LineGeometry.GetAbsoluteSize().X*.5-Base.GetAbsolutePosition().X-Base.GetAbsoluteSize().X*.5)<1.f);
   }
   if (bCompact) { Step=7; Changed=FPlatformTime::Seconds(); return false; }
   MoveTo(S,TEXT("TheaterDefenseBaseHover")); Next(); return false;
  }
  if (Step==6)
  {
   // Actual native hover dispatch, then allow Slate's normal tooltip delay.
   auto* Hover=CastChecked<UBorder>(S->GetWidgetFromName(TEXT("TheaterDefenseBaseHover")));
   const auto Tip=Hover->GetToolTip()->GetCachedWidget();
   const auto TipWindow=Tip.IsValid() ? FSlateApplication::Get().FindWidgetWindow(Tip.ToSharedRef()) : TSharedPtr<SWindow>();
   if (!TipWindow.IsValid() || !TipWindow->IsVisible())
   {
    FSlateApplication::Get().UpdateToolTip(true);
    if (FPlatformTime::Seconds()-Changed<8) return false;
    Test->AddError(TEXT("Native base tooltip did not open from real hover")); return true;
   }
   const auto Explanation=CastChecked<UTextBlock>(CastChecked<USizeBox>(CastChecked<UBorder>(Hover->GetToolTip())->GetContent())->GetContent())->GetText().ToString();
   Test->TestTrue(TEXT("Real hover shows safe defense modifier"),Explanation.Contains(TEXT("防守加成 +2")));
   Test->TestTrue(TEXT("Actual native hover brightens the inspect cue"),
    CastChecked<UBorder>(S->GetWidgetFromName(TEXT("TheaterDefenseBaseUnderline")))->GetBrushColor().A>.9f);
   MoveTo(S,TEXT("TheaterContinue")); Next(); return false;
  }
  if (Step==7)
  {
   CheckStableEquation(S,true);
   Test->TestTrue(TEXT("Inspect cue returns to quiet after pointer leaves"),
    CastChecked<UBorder>(S->GetWidgetFromName(TEXT("TheaterDefenseBaseUnderline")))->GetBrushColor().A<.8f);
   Test->TestTrue(TEXT("Native tooltip leaves underlying action enabled"),CastChecked<UButton>(S->GetWidgetFromName(TEXT("TheaterContinue")))->GetIsEnabled());
   if (!bCompact) CaptureMovieFrame(S); Click(S); Next(); return false;
  }
  if (Step==8)
  {
   Test->TestTrue(TEXT("Roll rendered along natural game clock"),bRollCaptured);
   Test->TestTrue(TEXT("Inline authoritative hold observed"),bInlineHoldChecked);
   Test->TestTrue(TEXT("Attack settled before defense CTA"),Visible(TEXT("TheaterPrimaryBounds")));
   Test->TestEqual(TEXT("Partial reveal has final attack label"),Text(TEXT("TheaterAttackValueLabel")),FString(TEXT("最终值")));
   Test->TestEqual(TEXT("Partial reveal retains current defense label"),Text(TEXT("TheaterDefenseValueLabel")),FString(TEXT("当前值")));
   Paint(S); CheckEquation(S);
   CaptureRollFrame(S,TEXT("05_MixedState.png"));
   if (!bLow) SaveMovieFrames();
   if (bCompact) return true;
   BeforeScore=S->GetMatchHeader()->GetDisplayedScoreLabel(); Click(S); Next(); return false;
  }
  if (Step==9)
  {
   Test->TestTrue(TEXT("Score gate observed in real cycling"),bScoreChecked);
   Test->TestTrue(TEXT("ResultHold alias gate observed on natural clock"),bHoldChecked);
   Test->TestTrue(TEXT("Outcome stays in coherent theater"),Visible(TEXT("TheaterOutcome")));
   Test->TestFalse(TEXT("Defense final replaces pending marker"),Visible(TEXT("TheaterDefensePending")));
   Test->TestEqual(TEXT("Defense value is projected final"),Text(TEXT("TheaterDefenseFinalNumber")),S->GetInlineFormulaSurface()->GetPresentation().DefenseRow.DisplayedResultLabel);
   Test->TestTrue(TEXT("Headline separator appears only after final reveal"),Visible(TEXT("TheaterOutcomeDivider")));
   const auto& Facts=C->GetInteractionView().ResolutionFacts;
   if (!Test->TestEqual(TEXT("One real High Cross contest"),Facts.FormulaContests.Num(),1)) return true;
   const auto& Contest=Facts.FormulaContests[0];
   const auto& Result=Contest.ResolvedResult;
   if (!bLow)
   {
   Test->TestTrue(TEXT("Legitimate DEV dice produce an authority-owned stamina tie"),
    Result.WinReason==EFormulaWinReason::StaminaTieBreaker || Result.WinReason==EFormulaWinReason::DefenderWinsEqualStamina);
   Test->TestEqual(TEXT("Real tie totals match"),Result.AttackerFinalValue,Result.DefenderFinalValue);
   }
   else Test->TestEqual(TEXT("Low preserves authoritative special-rule reason"),Result.WinReason,EFormulaWinReason::FastSuppression);
   Test->TestTrue(TEXT("Safe row stamina matches the existing authoritative Formula operands"),
    Contest.AttackRow.ParticipatingStamina.Num()==2 && Contest.DefenseRow.ParticipatingStamina.Num()==2
    && Contest.AttackRow.ParticipatingStamina==Contest.ResolvedInput.Attacker.ParticipatingStamina
    && Contest.DefenseRow.ParticipatingStamina==Contest.ResolvedInput.Defender.ParticipatingStamina);
   Test->AddInfo(FString::Printf(TEXT("THEATER_AUTHORITY_TIE winner=%d reason=%d stamina=%d/%d total=%.1f/%.1f"),
    int32(Result.Winner),int32(Result.WinReason),Result.AttackerParticipatingStaminaTotal,Result.DefenderParticipatingStaminaTotal,
    Result.AttackerFinalValue,Result.DefenderFinalValue));
   const auto LabelGeometry=S->GetWidgetFromName(TEXT("TheaterContinueLabel"))->GetCachedGeometry();
   const auto ChevronGeometry=S->GetWidgetFromName(TEXT("TheaterNextIcon"))->GetCachedGeometry();
   Test->TestTrue(TEXT("Continue chevron lies after text with a clean gap"),
    ChevronGeometry.GetAbsolutePosition().X>LabelGeometry.GetAbsolutePosition().X+LabelGeometry.GetAbsoluteSize().X+8);
   CaptureRollFrame(S,TEXT("06_Result.png"));
   CheckEquation(S);
   const auto Main=S->GetWidgetFromName(TEXT("TheaterReasonPrimary"))->GetCachedGeometry();
   const auto Secondary=S->GetWidgetFromName(TEXT("TheaterReasonSecondary"))->GetCachedGeometry();
   const auto Separator=S->GetWidgetFromName(TEXT("TheaterReasonSeparator"))->GetCachedGeometry();
   Test->TestTrue(TEXT("Reason has icon separator then left-aligned two-line copy"),
    Separator.GetAbsolutePosition().X<Main.GetAbsolutePosition().X
    && FMath::Abs(Main.GetAbsolutePosition().X-Secondary.GetAbsolutePosition().X)<1.f
    && Main.GetAbsolutePosition().Y+Main.GetAbsoluteSize().Y<=Secondary.GetAbsolutePosition().Y);
   const FString SecondaryCopy=Text(TEXT("TheaterReasonSecondary"));
   if (!bLow) Test->TestTrue(TEXT("Real tie explanation contains the safe stamina value"),SecondaryCopy.Contains(TEXT("体力"))
    && SecondaryCopy.Contains(FString::FromInt(Result.DefenderParticipatingStaminaTotal)));
   else Test->TestTrue(TEXT("Low reason explains special-rule priority"),SecondaryCopy.Contains(TEXT("不比较最终总值")));
   auto* Primary=CastChecked<UButton>(S->GetWidgetFromName(TEXT("TheaterContinue")));
   Primary->SetKeyboardFocus(); Test->TestTrue(TEXT("Continue remains keyboard focusable"),Primary->HasKeyboardFocus());
   // The closeout reason-bar capture is not repeated by this roll pass.
   if (!bLow) CheckLongNames(S);
   Click(S); Next(); return false;
  }
  Test->TestFalse(TEXT("Natural continue removes theater"),Visible(TEXT("ResolutionTheater")));
  Test->TestTrue(TEXT("Racks pitch and HUD restored"),Visible(TEXT("MatchShellViewportFit")));
  Test->TestEqual(TEXT("Original pitch geometry restores with board"),S->GetPitchWidget()->WidgetTree->FindWidget(TEXT("TwoLanePitchCanvas"))->GetRenderOpacity(),1.f);
  Test->TestEqual(TEXT("Pitch transform fully restored"),S->GetPitchWidget()->GetRenderTransform().Scale,FVector2D(1.f));
  Test->TestEqual(TEXT("Racks fully restored"),S->GetWidgetFromName(TEXT("LocalPlayerCardRackRegion"))->GetRenderOpacity(),1.f);
  Test->AddInfo(FString::Printf(TEXT("ROLL_V2_GEOMETRY maximum horizontal delta=%.4f Slate units"),MaximumEquationDelta));
  Test->TestTrue(TEXT("Terminal advanced once"),C->GetLastDiagnostic().bHostSuccess && !C->GetInteractionView().bTerminalPendingAdvance);
  if (bLow) CaptureRollFrame(S,TEXT("07_Return.png"));
  return true;
 }
private:
 struct FMovieFrame { TArray<FColor> Pixels; FIntVector Size; float Time=0.f; };
 TArray<FMovieFrame> MovieFrames;
 float LastMovieTime=-1.f;
 void CaptureMovieFrame(UFMCodexLocalMatchScreenWidget* S)
 {
  const auto Window=FSlateApplication::Get().FindWidgetWindow(S->TakeWidget());
  if (!Window.IsValid()) return;
  const auto WG=Window->GetContent()->GetTickSpaceGeometry();
  const auto G=S->GetWidgetFromName(bCompact?TEXT("TheaterBottom"):TEXT("TheaterAttackPanel"))->GetCachedGeometry();
  const auto Origin=G.GetAbsolutePosition()-WG.GetAbsolutePosition();
  const auto End=Origin+G.GetAbsoluteSize();
  const FIntRect Area(FMath::FloorToInt(Origin.X),FMath::FloorToInt(Origin.Y),FMath::CeilToInt(End.X),FMath::CeilToInt(End.Y));
  FMovieFrame Frame; Frame.Time=GEditor->PlayWorld->GetTimeSeconds();
  if (FSlateApplication::Get().TakeScreenshot(Window->GetContent(),Area,Frame.Pixels,Frame.Size))
  { LastMovieTime=Frame.Time; MovieFrames.Add(MoveTemp(Frame)); }
 }
 void SaveMovieFrames()
 {
  const FString Dir=FPaths::ProjectSavedDir()/(bCompact?TEXT("Stage8_10A_1/Motion"):TEXT("Stage8_9A_2/Motion")); IFileManager::Get().MakeDirectory(*Dir,true);
  FString Times=TEXT("frame,game_seconds\n");
  for (int32 I=0;I<MovieFrames.Num();++I)
  {
   const auto& F=MovieFrames[I]; TArray64<uint8> PNG; FImageUtils::PNGCompressImageArray(F.Size.X,F.Size.Y,F.Pixels,PNG);
   Test->TestTrue(TEXT("Natural roll motion frame saved"),FFileHelper::SaveArrayToFile(PNG,*(Dir/FString::Printf(TEXT("%03d.png"),I))));
   Times+=FString::Printf(TEXT("%03d,%.6f\n"),I,F.Time);
  }
  FFileHelper::SaveStringToFile(Times,*(Dir/TEXT("times.csv")));
  Test->AddInfo(FString::Printf(TEXT("ROLL_V2_MOVIE natural-clock cropped frames=%d"),MovieFrames.Num()));
 }
 void CheckStableEquation(UFMCodexLocalMatchScreenWidget* S,bool bRecord)
 {
  for (const auto Prefix:{TEXT("TheaterAttack"),TEXT("TheaterDefense")})
  {
   const auto Panel=S->GetWidgetFromName(FName(*(FString(Prefix)+TEXT("Panel"))))->GetCachedGeometry();
   for (const auto Suffix:{TEXT("BaseHover"),TEXT("Plus"),TEXT("UnknownSlot"),TEXT("Equal"),TEXT("ResultColumn")})
   {
    const FString Name=FString(Prefix)+Suffix;
    const auto G=S->GetWidgetFromName(FName(*Name))->GetCachedGeometry();
    const FVector2D Metric(Panel.AbsoluteToLocal(G.GetAbsolutePosition()).X, G.GetAbsoluteSize().X/Panel.Scale);
    if (bRecord) EquationMetrics.Add(Name,Metric);
    else if (const auto* Before=EquationMetrics.Find(Name))
    {
     const float Delta=FMath::Max(FMath::Abs(Metric.X-Before->X),FMath::Abs(Metric.Y-Before->Y));
     MaximumEquationDelta=FMath::Max(MaximumEquationDelta,Delta);
     Test->TestTrue(FString::Printf(TEXT("%s fixed x/width through natural roll phases"),*Name),Delta<.5f);
    }
   }
  }
 }
 void CaptureRollFrame(UFMCodexLocalMatchScreenWidget* S,const TCHAR* File)
 {
  const auto Window=FSlateApplication::Get().FindWidgetWindow(S->TakeWidget());
  if (!Window.IsValid()) { Test->AddError(TEXT("No PIE window for Roll v2 evidence")); return; }
  TArray<FColor> Pixels; FIntVector Size;
  if (!Test->TestTrue(TEXT("Real Roll v2 PIE frame captured"),FSlateApplication::Get().TakeScreenshot(Window->GetContent(),Pixels,Size))) return;
  const FString Dir=FPaths::ProjectSavedDir()/(bCompact?TEXT("Stage8_10A_1/PIE"):bLow?TEXT("Stage8_9B/PIE"):TEXT("Stage8_9A_2")); IFileManager::Get().MakeDirectory(*Dir,true);
  TArray64<uint8> PNG; FImageUtils::PNGCompressImageArray(Size.X,Size.Y,Pixels,PNG);
  Test->TestTrue(TEXT("Roll v2 frame saved"),FFileHelper::SaveArrayToFile(PNG,*(Dir/File)));
  Test->AddInfo(FString::Printf(TEXT("ROLL_V2_CAPTURE %s game=%.3f"),File,GEditor->PlayWorld->GetTimeSeconds()));
 }
 void Next() { ++Step; Changed=FPlatformTime::Seconds(); Test->AddInfo(FString::Printf(TEXT("THEATER_PIE step=%d game=%.3f"),Step,GEditor->PlayWorld->GetTimeSeconds())); }
 bool Override(AFMCodexLocalMatchPlayerController& C,EFMCodexLocalDevRollTarget Target,int32 Value)
 {
  FFMCodexLocalDevRollOverrideRequest R; R.Target=Target; R.Value=Value;
  return Test->TestTrue(TEXT("Existing DEV provider accepts deterministic roll"),C.SetLocalDevRollOverride(R).bSuccess);
 }
 void Click(UFMCodexLocalMatchScreenWidget* S)
 {
  auto* B=CastChecked<UButton>(S->GetWidgetFromName(TEXT("TheaterContinue")));
  Test->TestTrue(TEXT("Theater CTA uses existing enabled action"),B->GetIsEnabled()); B->OnClicked.Broadcast();
 }
 void MoveTo(UFMCodexLocalMatchScreenWidget* S,const TCHAR* Name)
 {
  auto& App=FSlateApplication::Get(); const auto& G=S->GetWidgetFromName(Name)->GetCachedGeometry();
  const FVector2D Pos=G.LocalToAbsolute(G.GetLocalSize()*.5f),Old=App.GetCursorPos();
  App.SetCursorPos(Pos);
  const TSet<FKey> Buttons;
  App.ProcessMouseMoveEvent(FPointerEvent(0,Pos,Old,Buttons,EKeys::Invalid,0,FModifierKeysState()),false);
  App.UpdateToolTip(true);
 }
 // Paint first: checks inspect the actual arranged tree, including nested rows
 // and the independently changing current/final font size.
 void Paint(UFMCodexLocalMatchScreenWidget* S)
 {
  TArray<FColor> Pixels; FIntVector Size;
  Test->TestTrue(TEXT("Numeric layout frame painted"),FSlateApplication::Get().TakeScreenshot(
   GEditor->PlayWorld->GetGameViewport()->GetWindow()->GetContent(),Pixels,Size));
 }
 void CheckEquation(UFMCodexLocalMatchScreenWidget* S)
 {
  auto Measure=FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
  for (const auto Prefix:{TEXT("TheaterAttack"),TEXT("TheaterDefense")})
  {
   float ReferenceBaseline=0.f,PreviousRight=0.f;
   for (const auto Suffix:{TEXT("Number"),TEXT("Plus"),TEXT("Pending"),TEXT("RollValue"),TEXT("Equal"),TEXT("FinalNumber")})
   {
    auto* T=CastChecked<UTextBlock>(S->GetWidgetFromName(FName(*(FString(Prefix)+Suffix))));
    if (T->GetVisibility()==ESlateVisibility::Collapsed) continue;
    const auto G=T->GetCachedGeometry();
    const float Baseline=G.LocalToAbsolute(FVector2D(0,G.GetLocalSize().Y+Measure->GetBaseline(T->GetFont()))).Y;
    if (ReferenceBaseline==0.f) ReferenceBaseline=Baseline;
    Test->AddInfo(FString::Printf(TEXT("EQUATION_METRIC %s %s baseline=%.3f ref=%.3f pos=%s size=%s maxH=%d descent=%d"),Prefix,Suffix,Baseline,ReferenceBaseline,*G.GetAbsolutePosition().ToString(),*G.GetLocalSize().ToString(),Measure->GetMaxCharacterHeight(T->GetFont()),Measure->GetBaseline(T->GetFont())));
    Test->TestTrue(FString::Printf(TEXT("%s %s shares the painted equation baseline"),Prefix,Suffix),FMath::Abs(Baseline-ReferenceBaseline)<1.6f);
    Test->TestTrue(TEXT("Adjacent equation tokens do not overlap"),G.GetAbsolutePosition().X>=PreviousRight);
    PreviousRight=G.GetAbsolutePosition().X+G.GetAbsoluteSize().X;
   }
   const auto Final=S->GetWidgetFromName(FName(*(FString(Prefix)+TEXT("FinalNumber"))))->GetCachedGeometry();
   const auto Caption=S->GetWidgetFromName(FName(*(FString(Prefix)+TEXT("ValueLabel"))))->GetCachedGeometry();
   const auto Safe=S->GetWidgetFromName(FName(*(FString(Prefix)+TEXT("SafeContent"))))->GetCachedGeometry();
   Test->AddInfo(FString::Printf(TEXT("EQUATION_CAPTION %s captionPos=%s captionSize=%s finalPos=%s finalSize=%s"),Prefix,*Caption.GetAbsolutePosition().ToString(),*Caption.GetAbsoluteSize().ToString(),*Final.GetAbsolutePosition().ToString(),*Final.GetAbsoluteSize().ToString()));
   Test->TestTrue(TEXT("Caption is centered directly above RHS total"),
    FMath::Abs(Caption.GetAbsolutePosition().X+Caption.GetAbsoluteSize().X*.5-Final.GetAbsolutePosition().X-Final.GetAbsoluteSize().X*.5)<1.f
    && Caption.GetAbsolutePosition().Y+Caption.GetAbsoluteSize().Y<=Final.GetAbsolutePosition().Y+1.f);
   Test->TestTrue(TEXT("RHS numeric column remains inside safe panel content"),
    Final.GetAbsolutePosition().X+Final.GetAbsoluteSize().X<=Safe.GetAbsolutePosition().X+Safe.GetAbsoluteSize().X+1.f);
  }
 }
 void CheckNumericWidths(UFMCodexLocalMatchScreenWidget* S)
 {
  // Cosmetic width fixtures only; no authority, dice or displayed score changes.
  auto* Base=CastChecked<UTextBlock>(S->GetWidgetFromName(TEXT("TheaterDefenseNumber")));
  auto* Final=CastChecked<UTextBlock>(S->GetWidgetFromName(TEXT("TheaterDefenseFinalNumber")));
  const FText OldBase=Base->GetText(),OldFinal=Final->GetText();
  for (const auto Value:{TEXT("7"),TEXT("12.5")})
  {
   Base->SetText(FText::FromString(Value)); Final->SetText(FText::FromString(Value));
   Paint(S); CheckEquation(S);
  }
  Base->SetText(OldBase); Final->SetText(OldFinal); Paint(S);
 }
 void CheckLongNames(UFMCodexLocalMatchScreenWidget* S)
 {
  auto* Name=CastChecked<UTextBlock>(S->GetWidgetFromName(TEXT("TheaterDefenseName0")));
  const FText Original=Name->GetText();
  Name->SetText(FText::FromString(TEXT("刘易斯-斯凯利")));
  const auto Window=GEditor->PlayWorld->GetGameViewport()->GetWindow();
  TArray<FColor> Pixels; FIntVector Size;
  Test->TestTrue(TEXT("Long-name cosmetic fixture renders without changing match facts"),FSlateApplication::Get().TakeScreenshot(Window->GetContent(),Pixels,Size));
  const auto N=Name->GetCachedGeometry();
  const auto Other=S->GetWidgetFromName(TEXT("TheaterDefenseName1"))->GetCachedGeometry();
  const auto Safe=S->GetWidgetFromName(TEXT("TheaterDefenseSafeContent"))->GetCachedGeometry();
  Test->TestTrue(TEXT("Long Chinese name remains in safe zone and clears adjacent participant"),
   N.GetAbsolutePosition().X>=Safe.GetAbsolutePosition().X-1
   && N.GetAbsolutePosition().X+N.GetAbsoluteSize().X<Other.GetAbsolutePosition().X
   && N.GetAbsoluteSize().Y>=23.f);
  Test->AddInfo(FString::Printf(TEXT("THEATER_LONG_NAME rendered=%.1fx%.1f font=%s outline=%d"),
   N.GetAbsoluteSize().X,N.GetAbsoluteSize().Y,*Name->GetFont().TypefaceFontName.ToString(),Name->GetFont().OutlineSettings.OutlineSize));
  Name->SetText(Original);
 }
 void CaptureDetail(UFMCodexLocalMatchScreenWidget* S,const TCHAR* Name,const TCHAR* File)
 {
  const auto Window=GEditor->PlayWorld->GetGameViewport()->GetWindow();
  const auto& WindowGeometry=Window->GetContent()->GetTickSpaceGeometry();
  const auto& G=S->GetWidgetFromName(Name)->GetCachedGeometry();
  const FVector2D Origin=G.GetAbsolutePosition()-WindowGeometry.GetAbsolutePosition(), Size=G.GetAbsoluteSize();
  // UE 5.3's screenshot clamp does not repair a negative padded minimum;
  // bound both corners before sending a detail rectangle to the RHI.
  const auto Extent=WindowGeometry.GetAbsoluteSize();
  const FIntRect Area(FMath::Max(0,FMath::FloorToInt(Origin.X)-12),FMath::Max(0,FMath::FloorToInt(Origin.Y)-12),
   FMath::Min(FMath::FloorToInt(Extent.X),FMath::CeilToInt(Origin.X+Size.X)+12),
   FMath::Min(FMath::FloorToInt(Extent.Y),FMath::CeilToInt(Origin.Y+Size.Y)+12));
  TArray<FColor> Pixels; FIntVector OutputSize;
  if (!Test->TestTrue(TEXT("Actual theater detail captured"),FSlateApplication::Get().TakeScreenshot(Window->GetContent(),Area,Pixels,OutputSize))) return;
  TArray64<uint8> PNG; FImageUtils::PNGCompressImageArray(OutputSize.X,OutputSize.Y,Pixels,PNG);
  Test->TestTrue(TEXT("Theater detail evidence saved"),FFileHelper::SaveArrayToFile(PNG,*(FPaths::ProjectSavedDir()/TEXT("Stage8_8F_Closeout")/File)));
 }
 void Capture(UFMCodexLocalMatchScreenWidget* S,const TCHAR* File,TSharedPtr<SWindow> TooltipWindow=nullptr)
 {
  const auto Window=GEditor->PlayWorld->GetGameViewport()->GetWindow();
  if (!Window.IsValid()) { Test->AddError(TEXT("PIE window missing")); return; }
  // TakeScreenshot performs Slate PrivateDrawWindows. Validate geometry only
  // after that paint, so bounds and pixels describe the same captured frame.
  TArray<FColor> Pixels; FIntVector Size;
  if (!Test->TestTrue(TEXT("Real PIE viewport captured"),FSlateApplication::Get().TakeScreenshot(Window->GetContent(),Pixels,Size))) return;
  auto* Root=S->GetWidgetFromName(TEXT("ResolutionTheater"));
  if (Root && Root->GetVisibility()!=ESlateVisibility::Collapsed)
  {
   const auto& G=Root->GetCachedGeometry(); const FVector2D StartPos=G.GetAbsolutePosition(),End=StartPos+G.GetAbsoluteSize();
   for (const auto Name:{TEXT("TheaterAttackPanel"),TEXT("TheaterDefensePanel"),TEXT("TheaterBottom")})
   {
    // Hidden entry beats do not yet have painted/cached geometry. Validate
    // them in the subsequent fully assembled capture, not as zero-size boxes.
    auto* W=S->GetWidgetFromName(Name);
    if (W->GetVisibility()==ESlateVisibility::Hidden || W->GetVisibility()==ESlateVisibility::Collapsed) continue;
    const auto& Child=S->GetWidgetFromName(Name)->GetCachedGeometry(); const FVector2D Pos=Child.GetAbsolutePosition(),ChildEnd=Pos+Child.GetAbsoluteSize();
    Test->TestTrue(FString::Printf(TEXT("%s fits actual full viewport"),Name),Pos.X>=StartPos.X-1 && Pos.Y>=StartPos.Y-1 && ChildEnd.X<=End.X+1 && ChildEnd.Y<=End.Y+1);
   }
   const auto Bar=S->GetWidgetFromName(TEXT("TheaterInfoBar"))->GetCachedGeometry();
   for (const auto Name:{TEXT("TheaterDetail"),TEXT("TheaterReasonPrimary"),TEXT("TheaterReasonSecondary"),TEXT("TheaterReasonMark"),TEXT("TheaterReasonSeparator")})
   {
    auto* W=S->GetWidgetFromName(Name);
    if (W->GetVisibility()==ESlateVisibility::Collapsed || W->GetParent()->GetVisibility()==ESlateVisibility::Collapsed) continue;
    const auto TextGeometry=W->GetCachedGeometry();
    const auto TextStart=TextGeometry.GetAbsolutePosition(),TextEnd=TextStart+TextGeometry.GetAbsoluteSize();
    Test->TestTrue(FString::Printf(TEXT("%s: %s fits the painted reason bar"),File,Name),
     TextStart.X>=Bar.GetAbsolutePosition().X && TextStart.Y>=Bar.GetAbsolutePosition().Y
     && TextEnd.X<=Bar.GetAbsolutePosition().X+Bar.GetAbsoluteSize().X+1
     && TextEnd.Y<=Bar.GetAbsolutePosition().Y+Bar.GetAbsoluteSize().Y+1);
   }
   for (const auto Prefix:{TEXT("TheaterAttack"),TEXT("TheaterDefense")})
   {
    const auto Player=S->GetWidgetFromName(FName(*(FString(Prefix)+TEXT("SilhouetteBounds"))))->GetCachedGeometry();
    const auto Content=S->GetWidgetFromName(FName(*(FString(Prefix)+TEXT("SafeContent"))))->GetCachedGeometry();
    const auto Panel=S->GetWidgetFromName(FName(*(FString(Prefix)+TEXT("Panel"))))->GetCachedGeometry();
    const auto Accent=S->GetWidgetFromName(FName(*(FString(Prefix)+TEXT("Accent"))))->GetCachedGeometry();
    Test->TestTrue(TEXT("Whole text column is separate from athlete decoration"),
     Player.GetAbsolutePosition().X+Player.GetAbsoluteSize().X+4<Content.GetAbsolutePosition().X
     || Content.GetAbsolutePosition().X+Content.GetAbsoluteSize().X+4<Player.GetAbsolutePosition().X);
    Test->TestTrue(TEXT("Accent fills panel height with aligned clean terminations"),
     FMath::Abs(Accent.GetAbsolutePosition().Y-Panel.GetAbsolutePosition().Y)<1.f
     && FMath::Abs(Accent.GetAbsoluteSize().Y-Panel.GetAbsoluteSize().Y)<1.f);
   }
   if (S->GetWidgetFromName(TEXT("TheaterDefensePanel"))->GetVisibility()!=ESlateVisibility::Hidden)
   {
    const auto A=S->GetWidgetFromName(TEXT("TheaterAttackPanel"))->GetCachedGeometry();
    const auto D=S->GetWidgetFromName(TEXT("TheaterDefensePanel"))->GetCachedGeometry();
    Test->TestTrue(TEXT("Attack and defense use screen width with clear separation"),A.GetAbsolutePosition().X+A.GetAbsoluteSize().X<D.GetAbsolutePosition().X);
   }
  }
  if (Root && Root->GetVisibility()!=ESlateVisibility::Collapsed)
  {
   // Probe actual atlas torso pixels, not merely a valid decoration rectangle.
   const auto& RootGeometry=Root->GetCachedGeometry();
   for (const auto Prefix:{TEXT("TheaterAttack"),TEXT("TheaterDefense")})
   {
    const bool Attack=FString(Prefix)==TEXT("TheaterAttack");
    auto* Image=CastChecked<UImage>(S->GetWidgetFromName(FName(*(FString(Prefix)+TEXT("Silhouette")))));
    const auto* Atlas=Cast<UTexture2D>(Image->GetBrush().GetResourceObject());
    Test->TestTrue(TEXT("Fresh PIE loads the imported RGBA athlete atlas"),Atlas && Atlas->GetSizeX()==1448 && Atlas->GetSizeY()==1086);
    const auto Player=Image->GetCachedGeometry();
    auto Sample=[&](FVector2D UV)
    {
     const FVector2D Abs=Player.LocalToAbsolute(UV*Player.GetLocalSize());
     const FVector2D RootUV=(Abs-RootGeometry.GetAbsolutePosition())/RootGeometry.GetAbsoluteSize();
     const int32 X=FMath::Clamp(FMath::RoundToInt(RootUV.X*Size.X),0,Size.X-1),Y=FMath::Clamp(FMath::RoundToInt(RootUV.Y*Size.Y),0,Size.Y-1);
     return Pixels[Y*Size.X+X];
    };
    const auto Torso=Sample(Attack?FVector2D(.54,.36):FVector2D(.63,.36));
    const auto Empty=Sample(FVector2D(.04,.36));
    Test->TestTrue(TEXT("Original athlete torso is actually painted over glass"),Torso.B>Empty.B+4);
   }
   // Empty left inset of the painted glass, outside all text/accents. Catch
   // missing brush tint in custom Slate paint, which geometry tests cannot see.
   const auto& RG=Root->GetCachedGeometry();
   for (const auto Name:{TEXT("TheaterAttackPanel"),TEXT("TheaterDefensePanel")})
   {
    const auto& PG=S->GetWidgetFromName(Name)->GetCachedGeometry();
    const FVector2D Pos=(PG.GetAbsolutePosition()+FVector2D(20,PG.GetAbsoluteSize().Y*.5)-RG.GetAbsolutePosition())/RG.GetAbsoluteSize();
    const int32 X=FMath::Clamp(FMath::RoundToInt(Pos.X*Size.X),0,Size.X-1);
    const int32 Y=FMath::Clamp(FMath::RoundToInt(Pos.Y*Size.Y),0,Size.Y-1);
    const FColor Pixel=Pixels[Y*Size.X+X];
    Test->TestTrue(TEXT("Glass retains dark reading protection in actual render"),Pixel.R<110 && Pixel.G<130 && Pixel.B<150);
   }
  }
  if (TooltipWindow.IsValid() && TooltipWindow!=Window)
  {
   // Separate native popup: composite actual pixels at the actual screen position.
   // The game-layer tooltip case is already part of the viewport screenshot.
   TArray<FColor> TipPixels; FIntVector TipSize;
   if (!Test->TestTrue(TEXT("Real native tooltip window captured"),FSlateApplication::Get().TakeScreenshot(TooltipWindow->GetContent(),TipPixels,TipSize))) return;
   const FVector2D Offset=TooltipWindow->GetContent()->GetTickSpaceGeometry().GetAbsolutePosition()-Window->GetContent()->GetTickSpaceGeometry().GetAbsolutePosition();
   const int32 OX=FMath::RoundToInt(Offset.X),OY=FMath::RoundToInt(Offset.Y);
   Test->TestTrue(TEXT("Tooltip fits viewport beside actual hovered base"),OX>=0 && OY>=0 && OX+TipSize.X<=Size.X && OY+TipSize.Y<=Size.Y);
   for (int32 Y=0;Y<TipSize.Y;++Y) for (int32 X=0;X<TipSize.X;++X)
    if (X+OX>=0 && X+OX<Size.X && Y+OY>=0 && Y+OY<Size.Y) Pixels[(Y+OY)*Size.X+X+OX]=TipPixels[Y*TipSize.X+X];
   Test->AddInfo(FString::Printf(TEXT("THEATER_NATIVE_TOOLTIP origin=%d,%d size=%dx%d"),OX,OY,TipSize.X,TipSize.Y));
  }
  if (TooltipWindow.IsValid() && TooltipWindow==Window)
   Test->AddInfo(TEXT("THEATER_NATIVE_TOOLTIP rendered by the native game-layer tooltip host inside the captured viewport"));
  const FString Dir=FPaths::ProjectSavedDir()/TEXT("Stage8_8F_Closeout"); IFileManager::Get().MakeDirectory(*Dir,true);
  TArray64<uint8> PNG; FImageUtils::PNGCompressImageArray(Size.X,Size.Y,Pixels,PNG);
  Test->TestTrue(TEXT("PIE evidence saved"),FFileHelper::SaveArrayToFile(PNG,*(Dir/File)));
  Test->AddInfo(FString::Printf(TEXT("THEATER_VIEWPORT %s %dx%d"),File,Size.X,Size.Y));
 }
 FAutomationTestBase* Test;
 IConsoleVariable* Mode=nullptr;
 int32 Previous=0,Step=0;
 int32 LastRollPhase=-1,LastRollStep=-1,LastDisclosed=-1;
 TMap<FString,FVector2D> EquationMetrics;
 float MaximumEquationDelta=0.f;
 uint8 RollFrames=0;
 double LastRollLog=0;
 double Start=FPlatformTime::Seconds(),Changed=0;
 bool bRollCaptured=false,bScoreChecked=false,bHoldChecked=false,bInlineHoldChecked=false;
 double NextMotionSample=0;
 uint8 MotionSeen=0;
 float ParticipantHeight=0;
 FVector2D QuestionPosition;
 FString BeforeScore;
 FString KeeperName;
 bool bLow=false,bRouteOnly=false,bCompact=false;
 bool bRouteMoving=false,bRouteLanded=false;
 int32 LastRoutePhase=-1;
};
class FNearTheaterPIE final : public IAutomationLatentCommand
{
public:
 FNearTheaterPIE(FAutomationTestBase* InTest,bool InPair,bool InPolish=false,bool InInspection=false,bool InLong=false,bool InEarly=false):Test(InTest),bPair(InPair),bPolish(InPolish || InInspection || InLong),bInspection(InInspection),bLong(InLong),bEarly(InEarly){}
 bool Update() override
 {
  if (FPlatformTime::Seconds()-Start>120) { Test->AddError(TEXT("Near Theater PIE timed out")); return true; }
  if (!GEditor || !GEditor->PlayWorld) return false;
  auto* C=Cast<AFMCodexLocalMatchPlayerController>(GEditor->PlayWorld->GetFirstPlayerController());
  auto* S=C?C->GetPlayerMatchScreen():nullptr; if (!S) return false;
  auto Visible=[&](const TCHAR* Name){auto* W=S->GetWidgetFromName(Name);return W && W->GetVisibility()!=ESlateVisibility::Collapsed;};
  auto Click=[&](const TCHAR* Name){auto* B=Cast<UButton>(S->GetWidgetFromName(Name));if (!B || !B->GetIsEnabled()) { Test->AddError(FString(TEXT("Missing legal PIE CTA: "))+Name);return false;} B->OnClicked.Broadcast();return true;};
  auto CheckSelectionCopy=[&](const FString& SelectedName,const FString& SubjectName,bool bEligible)
  {
   auto Text=[&](const TCHAR* Name){return CastChecked<UTextBlock>(S->GetWidgetFromName(Name))->GetText().ToString();};
   const FString Direct=TEXT("直接射门：取射门 / 传球较高值，与对方门将手控球进行判定");
   FString Combination=TEXT("战术配合：需射门 + 传球 ≥ 8；两枚骰子总和 ≥ 9 进球");
   if (!SubjectName.IsEmpty()) Combination+=TEXT("，")+SubjectName+(bEligible?TEXT("可用"):TEXT("不可用"));
   Test->TestEqual(TEXT("PIE subtitle follows selection only"),Text(TEXT("TheaterSubtitle")),SelectedName.IsEmpty()?FString(TEXT("选择主罚球员")):FString(TEXT("已选主罚球员："))+SelectedName);
   Test->TestEqual(TEXT("PIE info line one remains exact direct rule"),Text(TEXT("TheaterDetail")),Direct);
   Test->TestEqual(TEXT("PIE info line two appends candidate eligibility"),Text(TEXT("TheaterReasonSecondary")),Combination);
   Test->AddInfo(FString::Printf(TEXT("NEAR_SELECTION_COPY selected=%s subject=%s eligible=%d"),*SelectedName,*SubjectName,bEligible));
  };
  const float Game=GEditor->PlayWorld->GetTimeSeconds();
  if (S->IsInlineFormulaRevealInputBlocked())
  {
   if (Step==3) Test->TestFalse(TEXT("Natural Type reveal retains legacy board"),Visible(TEXT("ResolutionTheater")));
   if ((Step==7 || Step==8) && !bRollSeen && S->GetInlineFormulaRevealPhase()==EFMCodexUMGInlineFormulaRevealPhase::Cycling)
   {
    auto* Reel=CastChecked<UFMCodexRollReelWidget>(S->GetWidgetFromName(bPair?TEXT("TheaterPairAReel"):TEXT("TheaterAttackReel")));
    Test->TestEqual(TEXT("Real PIE uses production TheaterInline"),Reel->GetVisualVariant(),EFMCodexRollVisualVariant::TheaterInline);
    if (!bPair)
    {
     Test->TestEqual(TEXT("Attack role helper is hidden"),S->GetWidgetFromName(TEXT("TheaterStatus"))->GetVisibility(),ESlateVisibility::Hidden);
     Test->TestEqual(TEXT("Primary attack roll status stays visible"),CastChecked<UTextBlock>(S->GetWidgetFromName(TEXT("TheaterDetail")))->GetText().ToString(),FString(TEXT("进攻方掷点中")));
     if (bLong) Test->TestEqual(TEXT("Early miss explanation remains during attack roll"),CastChecked<UTextBlock>(S->GetWidgetFromName(TEXT("TheaterReasonSecondary")))->GetText().ToString(),FString(TEXT("进攻掷点 1–2：直接射偏")));
     CheckStableGeometry(S);
    }
    Capture(bPair?TEXT("CombinationRolling"):TEXT("DirectRolling")); bRollSeen=true;
   }
   if (bPolish && !bPair && Step==8)
   {
    Test->TestFalse(TEXT("Real defense reveal never replays attack reel"),Visible(TEXT("TheaterAttackReelHost")));
    Test->TestTrue(TEXT("Real defense reveal preserves settled attack operand"),Visible(TEXT("TheaterAttackRollValue")));
    Test->TestEqual(TEXT("Real attack die remains six"),CastChecked<UTextBlock>(S->GetWidgetFromName(TEXT("TheaterAttackRollValue")))->GetText().ToString(),FString(TEXT("6")));
    Test->TestTrue(TEXT("Real defense remains current reveal owner"),Visible(TEXT("TheaterDefenseReelHost")));
    Test->TestEqual(TEXT("Defense helper is hidden without collapsing its allocation"),S->GetWidgetFromName(TEXT("TheaterStatus"))->GetVisibility(),ESlateVisibility::Hidden);
    Test->TestEqual(TEXT("Defense primary bar retains the sole roll status"),CastChecked<UTextBlock>(S->GetWidgetFromName(TEXT("TheaterDetail")))->GetText().ToString(),FString(TEXT("防守方掷点中")));
    if (bLong) Test->TestTrue(TEXT("Defense reveal does not repeat the attack-only miss hint"),CastChecked<UTextBlock>(S->GetWidgetFromName(TEXT("TheaterReasonSecondary")))->GetText().IsEmpty());
    if (!bDefenseSeen && S->GetInlineFormulaRevealPhase()==EFMCodexUMGInlineFormulaRevealPhase::Cycling)
    {CheckStableGeometry(S);Capture(TEXT("DefenseRolling"));bDefenseSeen=true;}
   }
   return false;
  }
  if (Game-Changed<.55f) return false;
  auto Next=[&](){++Step;Changed=Game;Test->AddInfo(FString::Printf(TEXT("%s_PIE method=%s step=%d game=%.3f"),bLong?TEXT("LONG"):TEXT("NEAR"),bPair?(bLong?TEXT("Power"):TEXT("Combination")):bEarly?TEXT("Early"):TEXT("Direct"),Step,Game));};
  if (Step==0) {S->RequestStartNewMatch();Next();return false;}
  if (Step==1) {if (!Override(*C,EFMCodexLocalDevRollTarget::FullD12,9)) return true;S->RequestRollTacticalPoints();Next();return false;}
  if (Step==2) {if (!Override(*C,EFMCodexLocalDevRollTarget::SetPieceType,bLong?3:5)) return true;S->DevSetPieceAction(TEXT("SetPieceType"),NAME_None);Next();return false;}
  if (Step>=3 && Step<=8)
  {
   auto* Bottom=S->GetWidgetFromName(TEXT("TheaterBottom"));
   if (!Bottom) {Test->AddError(FString::Printf(TEXT("Free kick Theater absent: step=%d type=%d visible=%d category=%d"),Step,int32(S->GetPresentation().SetPiece.Type),S->GetPresentation().SetPiece.bVisible,int32(S->GetPresentation().Interaction.Category)));return true;}
   if (Bottom->GetRenderOpacity()<.99f) return false;
   const auto Bar=S->GetWidgetFromName(TEXT("TheaterInfoBar"))->GetCachedGeometry();
   const auto Helper=S->GetWidgetFromName(TEXT("TheaterStatus"))->GetCachedGeometry();
   const float BarCenter=Bar.GetAbsolutePosition().X+Bar.GetAbsoluteSize().X*.5f;
   Test->TestTrue(TEXT("Free kick helper is centered beneath information bar"),FMath::IsNearlyEqual(BarCenter,Helper.GetAbsolutePosition().X+Helper.GetAbsoluteSize().X*.5f,1.f));
   if (Step<=4 || Step==5 || (Step==6 && !bPair))
   {
    const auto Left=S->GetWidgetFromName(Step<=4?TEXT("TheaterTakerBounds"):Step==5?TEXT("TheaterNearDirect"):TEXT("TheaterDuel"))->GetCachedGeometry();
    const auto Right=Step==5?S->GetWidgetFromName(TEXT("TheaterNearCombination"))->GetCachedGeometry():Left;
    Test->TestTrue(TEXT("Info bar left edge aligns with visible content"),FMath::IsNearlyEqual(Bar.GetAbsolutePosition().X,Left.GetAbsolutePosition().X,1.f));
    Test->TestTrue(TEXT("Info bar right edge aligns with visible content"),FMath::IsNearlyEqual(Bar.GetAbsolutePosition().X+Bar.GetAbsoluteSize().X,Right.GetAbsolutePosition().X+Right.GetAbsoluteSize().X,1.f));
   }
   Test->TestEqual(TEXT("Non-rolling stages retain useful operator helper"),S->GetWidgetFromName(TEXT("TheaterStatus"))->GetVisibility(),ESlateVisibility::SelfHitTestInvisible);
   Test->TestFalse(TEXT("Operator helper has content"),CastChecked<UTextBlock>(S->GetWidgetFromName(TEXT("TheaterStatus")))->GetText().IsEmpty());
   if (Step<=4)
   {
    const auto* A=CastChecked<UTextBlock>(S->GetWidgetFromName(TEXT("TheaterDetail")));
    const auto* B=CastChecked<UTextBlock>(S->GetWidgetFromName(TEXT("TheaterReasonSecondary")));
    Test->TestTrue(TEXT("Real taker rule fonts match"),A->GetFont()==B->GetFont());
    Test->TestTrue(TEXT("Real taker rule brightness matches"),A->GetColorAndOpacity()==B->GetColorAndOpacity());
    Test->TestTrue(TEXT("Real taker rule line heights match"),FMath::IsNearlyEqual(A->GetCachedGeometry().GetAbsoluteSize().Y,B->GetCachedGeometry().GetAbsoluteSize().Y,1.f));
    Test->TestTrue(TEXT("Real taker rules share left indentation"),FMath::IsNearlyEqual(A->GetCachedGeometry().GetAbsolutePosition().X,B->GetCachedGeometry().GetAbsolutePosition().X,1.f));
   }
   if (Step==5)
   {
    const auto* Direct=CastChecked<UTextBlock>(S->GetWidgetFromName(TEXT("TheaterNearDirectLabel")));
    const auto* Alternative=CastChecked<UTextBlock>(S->GetWidgetFromName(TEXT("TheaterNearCombinationLabel")));
    Test->TestTrue(TEXT("Peer methods have identical typography"),Direct->GetFont()==Alternative->GetFont());
    Test->TestTrue(TEXT("Both legal methods have identical emphasis"),Direct->GetColorAndOpacity()==Alternative->GetColorAndOpacity());
    Test->TestTrue(TEXT("Peer title rendered heights match"),FMath::IsNearlyEqual(Direct->GetCachedGeometry().GetAbsoluteSize().Y,Alternative->GetCachedGeometry().GetAbsoluteSize().Y,1.f));
   }
  }
  if (Step==3)
  {
   if (!Test->TestTrue(TEXT("Near naturally enters Theater after Type"),Visible(TEXT("ResolutionTheater")))) return true;
   auto* Rack=CastChecked<UFMCodexCardRackWidget>(S->GetWidgetFromName(TEXT("TheaterTakers")));
   const auto& Cards=Rack->GetRenderedCardWidgets();
   // Stable public catalog identity only chooses a legal candidate; no state edit or forced route.
   const bool A=C->GetInteractionView().CurrentAttackingPlayer==EInitialTurnOrderPlayer::PlayerA;
   const FName Preferred=A?FName(TEXT("Prototype.Arsenal.EberechiEze")):FName(TEXT("Prototype.ManchesterCity.PhilFoden"));
   const auto* Card=Cards.FindByPredicate([Preferred](const auto& V){return V && V->GetPresentation().CardId==Preferred;});
   if (!Card) {Test->AddError(TEXT("Expected eligible Near candidate absent"));return true;}
   if (bInspection)
   {
    auto* Full=CastChecked<UFMCodexPlayerCardWidget>(S->GetWidgetFromName(TEXT("TheaterTakerFullCard")));
    const auto* WeakFact=S->GetPresentation().SetPiece.NearTakerEligibility.FindByPredicate([](const auto& V){return !V.bCanUseTacticalCombination;});
    if (!Test->TestNotNull(TEXT("Real legal pool includes ineligible combination taker"),WeakFact)) return true;
    const auto* Weak=Cards.FindByPredicate([&](const auto& V){return V->GetPresentation().CardId==WeakFact->CardId;});
    if (InspectStep==0)
    {CheckSelectionCopy(FString(),FString(),false);Capture(TEXT("Empty"));Test->TestTrue(TEXT("Initial inspector is empty"),Full->GetPresentation().CardId.IsNone());(*Card)->RequestFullCardDetailHover();++InspectStep;Changed=Game;return false;}
    if (InspectStep==1)
    {
     CheckSelectionCopy(FString(),(*Card)->GetPresentation().IdentityLabel,true);Capture(TEXT("Hover"));Test->TestEqual(TEXT("PIE hover shows chosen candidate"),Full->GetPresentation().CardId,Preferred);
     const auto RackGeometry=Rack->GetCachedGeometry(), FullGeometry=Full->GetCachedGeometry();
     const auto RackEnd=RackGeometry.LocalToAbsolute(RackGeometry.GetLocalSize());
     Test->TestTrue(TEXT("Inspector reserves space beside candidate hit targets"),FullGeometry.GetAbsolutePosition().X>RackEnd.X);
     Test->TestTrue(TEXT("Full Card remains readable at 1080p"),FullGeometry.GetAbsoluteSize().Y>=500.f);
     (*Weak)->RequestFullCardDetailHover();++InspectStep;Changed=Game;return false;
    }
    if (InspectStep==2)
    {CheckSelectionCopy(FString(),(*Weak)->GetPresentation().IdentityLabel,false);Capture(TEXT("Ineligible"));Test->TestEqual(TEXT("PIE can inspect second candidate"),Full->GetPresentation().CardId,WeakFact->CardId);(*Weak)->OnDetailHoverDismissed.Broadcast(Weak->Get());++InspectStep;Changed=Game;return false;}
    CheckSelectionCopy(FString(),FString(),false);
    Test->TestTrue(TEXT("Leaving hover without a selection empties inspector"),Full->GetPresentation().CardId.IsNone());
    (*Card)->RequestOnPitchSelection();Next();return false;
   }
   if (bPolish && !bHoverSeen)
   {
    // Exercise the same card enter callback as the pointer; allow a real Slate frame before capture.
    Capture(TEXT("Empty")); (*Card)->RequestFullCardDetailHover(); bHoverSeen=true; Changed=Game; return false;
   }
   if (bPolish) Capture(TEXT("Hover"));
   else if (!bPair) Capture(TEXT("Candidates"));
   if (!Test->TestTrue(TEXT("Real candidate widget submits selection"),(*Card)->RequestOnPitchSelection())) return true;
   Next();return false;
  }
  if (Step==4)
  {
   if (bPolish)
   {
    auto* Rack=CastChecked<UFMCodexCardRackWidget>(S->GetWidgetFromName(TEXT("TheaterTakers")));
    const auto* Cell=Rack->GetPresentation().Cells.FindByPredicate([](const auto& V){return V.bSetPieceSelected;});
    if (!Test->TestNotNull(TEXT("Draft remains selected in real PIE"),Cell)) return true;
    auto* Outline=Rack->GetWidgetFromName(FName(*FString::Printf(TEXT("CardDraftOutline%d"),Cell->StableIndex)));
    if (!Test->TestNotNull(TEXT("Draft outline exists"),Outline)) return true;
    const auto* Card=Rack->GetRenderedCardWidgets().FindByPredicate([Cell](const auto& V){return V->GetPresentation().CardId==Cell->Card.CardId;});
    const FVector2D CardSize=(*Card)->GetCachedGeometry().GetLocalSize(), OutlineSize=Outline->GetCachedGeometry().GetLocalSize();
    Test->TestTrue(TEXT("Selected outline spans the full inset card, not its desired-size corner"),OutlineSize.X>CardSize.X*.9f && OutlineSize.Y>CardSize.Y*.9f);
    Test->TestTrue(TEXT("Selected outline stays inside rarity border"),OutlineSize.X<CardSize.X && OutlineSize.Y<CardSize.Y);
   }
   if (bInspection)
   {
    auto* Full=CastChecked<UFMCodexPlayerCardWidget>(S->GetWidgetFromName(TEXT("TheaterTakerFullCard")));
    auto* Rack=CastChecked<UFMCodexCardRackWidget>(S->GetWidgetFromName(TEXT("TheaterTakers")));
    const auto* Selected=Rack->GetPresentation().Cells.FindByPredicate([](const auto& V){return V.bSetPieceSelected;});
    const auto* Other=Rack->GetRenderedCardWidgets().FindByPredicate([&](const auto& V){return V->GetPresentation().CardId!=Selected->Card.CardId;});
    const auto* SubjectFact=S->GetPresentation().SetPiece.NearTakerEligibility.FindByPredicate([&](const auto& V){return V.CardId==Full->GetPresentation().CardId;});
    if (!Test->TestNotNull(TEXT("Inspected candidate has safe eligibility fact"),SubjectFact)) return true;
    CheckSelectionCopy(Selected->Card.IdentityLabel,Full->GetPresentation().IdentityLabel,SubjectFact->bCanUseTacticalCombination);
    if (InspectStep==3) {Test->TestEqual(TEXT("PIE selection persists without hover"),Full->GetPresentation().CardId,Selected->Card.CardId);(*Other)->RequestFullCardDetailHover();++InspectStep;Changed=Game;return false;}
    if (InspectStep==4) {Test->TestEqual(TEXT("PIE comparison temporarily overrides selection"),Full->GetPresentation().CardId,(*Other)->GetPresentation().CardId);(*Other)->OnDetailHoverDismissed.Broadcast(Other->Get());++InspectStep;Changed=Game;return false;}
    Test->TestEqual(TEXT("PIE leaving comparison restores selection"),Full->GetPresentation().CardId,Selected->Card.CardId);
   }
   if (!bPair) Capture(TEXT("Selected"));if (!Click(TEXT("TheaterContinue"))) return true;Next();return false;
  }
  if (Step==5) {if (bInspection) {Test->TestEqual(TEXT("PIE confirm restores method subtitle"),CastChecked<UTextBlock>(S->GetWidgetFromName(TEXT("TheaterSubtitle")))->GetText().ToString(),FString(TEXT("选择结算方式")));Test->TestFalse(TEXT("PIE methods hide inspector"),Visible(TEXT("TheaterTakerInspector")));Test->TestTrue(TEXT("PIE methods clear Full Card identity"),CastChecked<UFMCodexPlayerCardWidget>(S->GetWidgetFromName(TEXT("TheaterTakerFullCard")))->GetPresentation().CardId.IsNone());} if (!bPair) Capture(TEXT("Methods"));if (!Click(bPair?TEXT("TheaterNearCombination"):TEXT("TheaterNearDirect"))) return true;Next();return false;}
  if (Step==6)
  {
   const auto ButtonGeometry=S->GetWidgetFromName(TEXT("TheaterContinue"))->GetCachedGeometry();
   const auto LabelGeometry=S->GetWidgetFromName(TEXT("TheaterContinueLabel"))->GetCachedGeometry();
   const FVector2D LabelOrigin=ButtonGeometry.AbsoluteToLocal(LabelGeometry.GetAbsolutePosition());
   const FVector2D LabelEnd=ButtonGeometry.AbsoluteToLocal(LabelGeometry.LocalToAbsolute(LabelGeometry.GetLocalSize()));
   Test->TestTrue(TEXT("Roll CTA label remains inside its button"),LabelOrigin.X>=0 && LabelEnd.X<=ButtonGeometry.GetLocalSize().X);
   if (bInspection) Test->TestFalse(TEXT("PIE Formula excludes full card inspection"),Visible(TEXT("TheaterTakerInspector")));
   for (const auto Name:{TEXT("TheaterAttackPanelBounds"),TEXT("TheaterDefensePanelBounds"),TEXT("TheaterBottom"),TEXT("TheaterStatus")})
   {
    const auto G=S->GetWidgetFromName(Name)->GetCachedGeometry();
    StablePositions.Add(Name,G.GetAbsolutePosition()); StableSizes.Add(Name,G.GetAbsoluteSize());
   }
   Capture(bPair?TEXT("CombinationPending"):TEXT("DirectPending"));
   if (!Override(*C,bLong?(bPair?EFMCodexLocalDevRollTarget::LongFreeKickPowerA:EFMCodexLocalDevRollTarget::LongFreeKickDirectAttack):(bPair?EFMCodexLocalDevRollTarget::ShortFreeKickAngledA:EFMCodexLocalDevRollTarget::ShortFreeKickDirectAttack),bEarly?1:6)
    || !Override(*C,bLong?(bPair?EFMCodexLocalDevRollTarget::LongFreeKickPowerB:EFMCodexLocalDevRollTarget::LongFreeKickDirectDefense):(bPair?EFMCodexLocalDevRollTarget::ShortFreeKickAngledB:EFMCodexLocalDevRollTarget::ShortFreeKickDirectDefense),bPair?(bLong?5:3):1)) return true;
   if (!Click(TEXT("TheaterContinue"))) return true;Next();return false;
  }
  if (Step==7 && !bPair && !bEarly) {if (!Click(TEXT("TheaterContinue"))) return true;Next();return false;}
  if (Step==7 || Step==8)
  {
   Test->TestTrue(TEXT("Outcome remains in Theater until Next"),Visible(TEXT("TheaterOutcome")));
   Test->TestEqual(TEXT("Authority supplied expected outcome"),C->GetInteractionView().bSetPieceGoal,!bEarly);
   if (bEarly) Test->TestFalse(TEXT("Early miss result has no invented defense"),Visible(TEXT("TheaterDefensePanelBounds")));
   if (bEarly) Test->TestEqual(TEXT("Real early miss uses attack-defense comparison wording"),CastChecked<UTextBlock>(S->GetWidgetFromName(TEXT("TheaterReasonSecondary")))->GetText().ToString(),FString(TEXT("进攻掷点 1–2 时结束，不进行攻防比较")));
   Test->TestTrue(TEXT("Natural roll phase was seen"),bRollSeen);
   if (bPolish && !bPair && !bEarly) Test->TestTrue(TEXT("Natural defense cycle was observed"),bDefenseSeen);
   Capture(bPair?TEXT("CombinationResult"):TEXT("DirectResult"));
   if (!Click(TEXT("TheaterContinue"))) return true;
   Step=9;Changed=Game;return false;
  }
  // Allow the real Slate exit transition to finish; latent commands run before widget tick.
  if (Visible(TEXT("ResolutionTheater")) && Game-Changed<3.f) return false;
  Test->TestFalse(TEXT("Next returns to board"),Visible(TEXT("ResolutionTheater")));
  Test->TestTrue(TEXT("Next actor can roll D12"),S->GetPresentation().Interaction.bCanRollTacticalPoints);
  if (!bPair) Capture(TEXT("ReturnedBoard"));
  return true;
 }
private:
 bool Override(AFMCodexLocalMatchPlayerController& C,EFMCodexLocalDevRollTarget Target,int32 Value)
 {
  FFMCodexLocalDevRollOverrideRequest R;R.Target=Target;R.Value=Value;
  return Test->TestTrue(TEXT("Natural command uses existing DEV provider seam"),C.SetLocalDevRollOverride(R).bSuccess);
 }
 void CheckStableGeometry(UFMCodexLocalMatchScreenWidget* S)
 {
  for (const auto& Pair:StablePositions)
  {
   const auto G=S->GetWidgetFromName(Pair.Key)->GetCachedGeometry();
   Test->TestTrue(TEXT("Hidden helper preserves panel/footer positions"),G.GetAbsolutePosition().Equals(Pair.Value,1.f));
   Test->TestTrue(TEXT("Hidden helper preserves panel/footer allocations"),G.GetAbsoluteSize().Equals(StableSizes[Pair.Key],1.f));
  }
 }
 TMap<FName,FVector2D> StablePositions,StableSizes;
 void Capture(const TCHAR* Name)
 {
  if (bPolish && !bInspection && !bLong && FString(Name)!=TEXT("Hover") && FString(Name)!=TEXT("Selected")
   && FString(Name)!=TEXT("Methods") && FString(Name)!=TEXT("DefenseRolling") && FString(Name)!=TEXT("DirectRolling") && FString(Name)!=TEXT("Empty")) return;
  if (bInspection && (FString(Name)==TEXT("DirectRolling") || FString(Name)==TEXT("DirectResult") || FString(Name)==TEXT("ReturnedBoard"))) return;
  if (bLong && (bPair || bEarly) && (FString(Name)==TEXT("Hover") || FString(Name)==TEXT("Selected") || FString(Name)==TEXT("Methods"))) return;
  if (bLong && FString(Name)==TEXT("ReturnedBoard")) return;
  TArray<FColor> Pixels;FIntVector Size=FIntVector::ZeroValue;
  if (!Test->TestTrue(TEXT("Real PIE frame captured"),TheaterPIEWindow.IsValid() && FSlateApplication::Get().TakeScreenshot(TheaterPIEWindow->GetContent(),Pixels,Size))) return;
  const FString Dir=FPaths::ProjectSavedDir()/(bLong?(bPair?TEXT("Stage8_11B_2/PIE/Power"):bEarly?TEXT("Stage8_11B_2/PIE/Early"):TEXT("Stage8_11B_2/PIE/Direct")):bInspection?TEXT("Stage8_11A_3/PIE"):bPolish?TEXT("Stage8_11B_2/PIE/Near"):TEXT("Stage8_11A/PIE"));IFileManager::Get().MakeDirectory(*Dir,true);
  TArray64<uint8> PNG;FImageUtils::PNGCompressImageArray(Size.X,Size.Y,Pixels,PNG);
  Test->TestTrue(TEXT("PIE frame saved"),FFileHelper::SaveArrayToFile(PNG,*(Dir/((bLong?FString(Name).Replace(TEXT("Combination"),TEXT("Power")):FString(Name))+TEXT(".png")))));
 }
 FAutomationTestBase* Test;bool bPair=false,bRollSeen=false,bPolish=false,bHoverSeen=false,bDefenseSeen=false,bInspection=false,bLong=false,bEarly=false;int32 Step=0,InspectStep=0;
 double Start=FPlatformTime::Seconds();float Changed=0;
};

}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FResolutionTheaterPIETest,"FMCodex.PIE.ResolutionTheater.HighCross",
 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FResolutionTheaterPIETest::RunTest(const FString&)
{
 FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShareable(new FStartTheaterPIE()));
 ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(2.f));
 FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShareable(new FTheaterPIE(this)));
 ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
 return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FResolutionTheaterLowPIETest,"FMCodex.PIE.ResolutionTheater.LowCross",
 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FResolutionTheaterLowPIETest::RunTest(const FString&)
{
 FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShareable(new FStartTheaterPIE()));
 ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(2.f));
 FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShareable(new FTheaterPIE(this,true)));
 ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
 return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FResolutionTheaterHighRoutePIETest,"FMCodex.PIE.ResolutionTheater.HighRouteContinuity",
 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FResolutionTheaterHighRoutePIETest::RunTest(const FString&)
{
 FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShareable(new FStartTheaterPIE()));
 ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(2.f));
 FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShareable(new FTheaterPIE(this,false,true)));
 ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
 return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FResolutionTheaterCompactRoutePIETest,"FMCodex.PIE.ResolutionTheater.CompactBoxRoute",
 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FResolutionTheaterCompactRoutePIETest::RunTest(const FString&)
{
 FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShareable(new FStartTheaterPIE()));
 ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(2.f));
 FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShareable(new FTheaterPIE(this,true,false,true)));
 ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
 return true;
}
IMPLEMENT_COMPLEX_AUTOMATION_TEST(FNearTheaterPIETest,"FMCodex.PIE.ResolutionTheater.NearFreeKick",
 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
void FNearTheaterPIETest::GetTests(TArray<FString>& N,TArray<FString>& C) const
{ for (const TCHAR* M:{TEXT("Direct"),TEXT("Combination"),TEXT("DirectPolish"),TEXT("Inspection")}) {N.Add(M);C.Add(M);} }
bool FNearTheaterPIETest::RunTest(const FString& P)
{
 FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShareable(new FStartTheaterPIE()));
 ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(2.f));
 FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShareable(new FNearTheaterPIE(this,P==TEXT("Combination"),P==TEXT("DirectPolish"),P==TEXT("Inspection"))));
 ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
 return true;
}
IMPLEMENT_COMPLEX_AUTOMATION_TEST(FLongTheaterPIETest,"FMCodex.PIE.ResolutionTheater.LongFreeKick",
 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
void FLongTheaterPIETest::GetTests(TArray<FString>& N,TArray<FString>& C) const
{for(const TCHAR* M:{TEXT("Direct"),TEXT("Early"),TEXT("Power")}) {N.Add(M);C.Add(M);}}
bool FLongTheaterPIETest::RunTest(const FString& P)
{
 FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShareable(new FStartTheaterPIE()));
 ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(2.f));
 FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShareable(new FNearTheaterPIE(this,P==TEXT("Power"),false,false,true,P==TEXT("Early"))));
 ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
 return true;
}
#endif
