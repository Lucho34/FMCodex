#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && !UE_BUILD_SHIPPING
#include "FMCodexGuidedLesson1.h"
#include "FMCodexGuidedLesson1Focus.h"
#include "FMCodexPitchSlotWidget.h"
#include "FMCodexInteractionPanelWidget.h"
#include "FMCodexLocalMatchPlayerController.h"
#include "FMCodexLocalMatchScreenWidget.h"
#include "FMCodexCardRackWidget.h"
#include "FMCodexPlayerCardWidget.h"
#include "FMCodexMatchHeaderWidget.h"
#include "Components/Button.h"
#include "Components/Border.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Editor.h"
#include "Engine/World.h"
#include "Engine/GameViewportClient.h"
#include "Framework/Application/SlateApplication.h"
#include "Layout/WidgetPath.h"
#include "ImageUtils.h"
#include "HAL/FileManager.h"
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
TSharedPtr<SWindow> LessonWindow;
class FStartLessonPIE : public IAutomationLatentCommand
{
public:
	bool Update() override
	{
		auto* Settings=DuplicateObject<ULevelEditorPlaySettings>(GetDefault<ULevelEditorPlaySettings>(),GetTransientPackage());
		Settings->NewWindowWidth=1600; Settings->NewWindowHeight=900;
		Settings->SetPlayNetMode(EPlayNetMode::PIE_Standalone); Settings->SetPlayNumberOfClients(1);
		LessonWindow=SNew(SWindow).Title(FText::FromString(TEXT("Guided Match Lesson 1 PIE")))
			.ClientSize(FVector2D(1600,900)).AutoCenter(EAutoCenter::None).ScreenPosition(FVector2D(144,96))
			.SaneWindowPlacement(false).AdjustInitialSizeAndPositionForDPIScale(false);
		FSlateApplication::Get().AddWindow(LessonWindow.ToSharedRef());
		FRequestPlaySessionParams Params; Params.EditorPlaySettings=Settings; Params.CustomPIEWindow=LessonWindow;
		GEditor->RequestPlaySession(Params); return true;
	}
};

// Real PIE world, production Screen gestures, natural reveal timers, live Full Card
// hover and the registered console entry. No raw state edits or test-clock stepping.
class FPlayLessonPIE : public IAutomationLatentCommand
{
public:
	explicit FPlayLessonPIE(FAutomationTestBase* InTest) : T(InTest) {}
	bool Update() override
	{
		const double Now=FPlatformTime::Seconds();
		if(Now-Started>210) { T->AddError(TEXT("Lesson full PIE timed out")); return true; }
		if(!GEditor->PlayWorld) return false;
		auto* C=Cast<AFMCodexLocalMatchPlayerController>(GEditor->PlayWorld->GetFirstPlayerController());
		if(!C || !C->GetPlayerMatchScreen()) return false;
		if (bExited)
		{
			if (Now-Changed<1.4) return false;
			auto* Normal=C->GetPlayerMatchScreen();
			// The Screen was replaced twice in the preceding update. Observe its new
			// arranged path, then dispatch/verify on separate Slate frames, not stale cache.
			if (!NormalHoverCard.IsValid())
			{
				for (UFMCodexPlayerCardWidget* Card : Normal->GetLocalRackWidget()->GetRenderedCardWidgets())
				{
					FWidgetPath Path; const auto Widget=Card->GetCachedWidget();
					if (Card->CanExposeFullCardDetail() && Widget.IsValid()
						&& FSlateApplication::Get().GeneratePathToWidgetUnchecked(Widget.ToSharedRef(),Path)
						&& Path.IsValid() && Path.Widgets.Last().Geometry.GetLocalSize().X>0)
					{
						NormalHoverCard=Card;
						Pointer(Center(Normal)); // Leave any previous hover before entering the card.
						return false;
					}
				}
				return false; // Existing overall timeout still bounds a missing layout.
			}
			if (NormalHoverSentAt==0)
			{
				FWidgetPath Path;
				if (!FSlateApplication::Get().GeneratePathToWidgetUnchecked(NormalHoverCard->TakeWidget(),Path) || !Path.IsValid()) return false;
				const auto& G=Path.Widgets.Last().Geometry;
				Pointer(G.LocalToAbsolute(G.GetLocalSize()*.5)); NormalHoverSentAt=Now; return false;
			}
			if (!Normal->IsDetailOverlayVisible() && Now-NormalHoverSentAt<1.5) return false;
			if (!Normal->IsDetailOverlayVisible())
			{
				Capture(TEXT("NormalHoverFailure.png"));
				T->AddInfo(FString::Printf(TEXT("NORMAL_HOVER hovered=%d category=%d revealBlocked=%d"),NormalHoverCard->IsHovered(),
					static_cast<int32>(Normal->GetPresentation().Interaction.Category),Normal->IsInlineFormulaRevealInputBlocked()));
			}
			if(auto* Status=Normal->GetInteractionPanel()->GetWidgetFromName(TEXT("InteractionBoundedFallback")))
				T->TestEqual(TEXT("Normal dock status restored after tutorial polish"),Status->GetRenderOpacity(),1.f);
			T->TestTrue(TEXT("Normal real hover restored after exit"),Normal->IsDetailOverlayVisible());
			auto* Roll=Normal->GetInteractionPanel()->GetWidgetFromName(TEXT("InteractionTacticalPointRollButton"));
			if (T->TestNotNull(TEXT("Normal TP control restored"),Roll))
			{ const auto At=Center(Roll); Pointer(At); Pointer(At,1); Pointer(At,2); }
			T->TestTrue(TEXT("Normal real TP input ungated"),C->GetInteractionView().RawInitialD12>0);
			return true;
		}
		if(!bStarted) { C->ConsoleCommand(TEXT("fm.Tutorial.Lesson1")); bStarted=true; Changed=Now; return false; }
		auto* L=C->GetGuidedLesson1();
		auto* S=C->GetPlayerMatchScreen();
		if(!T->TestNotNull(TEXT("Development console entered lesson"),L)) return true;
		if(!bCanonicalChecked)
		{
			T->TestTrue(TEXT("Real PIE consumes validated canonical tutorial content"),L->IsContentReady() && L->ContentSourceHash().Len()==64);
			T->AddInfo(TEXT("GUIDED_CONTENT runtime sha256=")+L->ContentSourceHash());
			bCanonicalChecked=true;
		}
		if (!bExitCancelChecked && L->GetStep()==EFMCodexLesson1Step::Intro && Now-Changed>1.4)
		{
			if (ExitPhase==0)
			{
				Capture(TEXT("01_Intro.png"));
				ExitScreen=S; ExitSequence=C->GetInteractionView().AttackSequence;
				ClickSlate(TEXT("LessonExit"));
				T->TestTrue(TEXT("First exit click opens confirmation, keeps lesson"),C->GetGuidedLesson1()==L && L->IsExitConfirmationOpen());
				ExitPhase=1; Changed=Now; return false;
			}
			Capture(TEXT("01_ExitConfirmation.png")); ClickSlate(TEXT("LessonExitCancel"));
			T->TestFalse(TEXT("Cancel removes dialog"),L->IsExitConfirmationOpen());
			T->TestEqual(TEXT("Cancel retains Intro"),L->GetStep(),EFMCodexLesson1Step::Intro);
			T->TestTrue(TEXT("Cancel did not replace Screen"),ExitScreen.Get()==S);
			T->TestEqual(TEXT("Cancel keeps authoritative sequence"),C->GetInteractionView().AttackSequence,ExitSequence);
			T->TestTrue(TEXT("Cancel keeps empty board"),C->GetInteractionView().DeploymentPlacements.IsEmpty());
			bExitCancelChecked=true; Changed=Now; return false;
		}
		if (L->GetStep()==EFMCodexLesson1Step::OpponentDeploy)
		{
			const int32 Attempt=L->IsComparison()?1:0;
			if (OpponentBegan[Attempt]==0) OpponentBegan[Attempt]=Now;
			if (OpponentObserved[Attempt]>0) OpponentSampleGap[Attempt]=FMath::Max(OpponentSampleGap[Attempt],Now-OpponentObserved[Attempt]);
			OpponentObserved[Attempt]=Now;
			if (L->OpponentTarget()==EFMCodexLesson1OpponentTarget::HandCard)
			{
				bOpponentSource=true;
				if(!bSourceCaptured && Now-OpponentBegan[Attempt]>.15) { Capture(TEXT("09_OpponentSelected.png")); bSourceCaptured=true; }
			}
			if (L->IsOpponentDeploymentMoving())
			{
				if (OpponentMoving[Attempt]==0) OpponentMoving[Attempt]=Now;
				T->TestEqual(TEXT("Proxy motion cannot deploy authoritative Stones early"),C->GetInteractionView().DeploymentPlacements.Num(),1);
				const auto Proxy=FindSlate(LessonWindow->GetContent(),TEXT("LessonOpponentProxy"));
				T->TestTrue(TEXT("Production-style visual proxy is visible in motion"),Proxy.IsValid());
				for(UFMCodexPlayerCardWidget* Card:S->GetOpponentRackWidget()->GetRenderedCardWidgets())
					if(Card->GetPresentation().CardId==L->Stones()) T->TestEqual(TEXT("Source replaced by proxy without duplicate portrait"),Card->GetRenderOpacity(),0.f);
				if(!L->IsComparison() && !bMoveCaptured && Now-OpponentMoving[Attempt]>.3)
				{ Capture(TEXT("09_OpponentMoving.png")); bMoveCaptured=true; }
			}
			if (L->IsOpponentSettling())
			{
				if (OpponentDeployed[Attempt]==0) OpponentDeployed[Attempt]=Now;
				if(Now>OpponentDeployed[Attempt]) // Observe the next arranged Slate frame, not the world-tick transition itself.
					T->TestFalse(TEXT("Accepted board never keeps a duplicate proxy"),FindSlate(LessonWindow->GetContent(),TEXT("LessonOpponentProxy")).IsValid());
				bOpponentPlaced=true;
				T->TestTrue(TEXT("Settled hold displays actual placed card"),C->GetInteractionView().DeploymentPlacements.Num()==2);
				if(!bPlacedCaptured && Now-OpponentDeployed[Attempt]>.15) { Capture(TEXT("10_OpponentPlaced.png")); bPlacedCaptured=true; }
			}
			if (L->IsOpponentFinalHold() && OpponentSettled[Attempt]==0)
			{
				OpponentSettled[Attempt]=Now;
				T->TestTrue(TEXT("Settled phase removes source/destination arrow"),FMCodexLesson1Focus::Targets(S,*L).IsEmpty());
				if (!L->IsComparison()) Capture(TEXT("10_OpponentBoardHold.png"));
			}
		}
		else
		{
			const int32 Attempt=L->IsComparison()?1:0;
			if (OpponentBegan[Attempt]>0 && OpponentFinished[Attempt]==0)
			{
				OpponentFinished[Attempt]=Now;
				T->TestTrue(TEXT("Opponent source visible for deliberate selection"),OpponentDeployed[Attempt]-OpponentBegan[Attempt]>=1.1);
				// Each boundary is observed at the next automation tick; allow the measured
				// sampling gap rather than a threshold tied to a previous pacing constant.
				const double DestinationMinimum=L->Timing(TEXT("OpponentDestinationHold"))*L->OpponentPace()-2.*OpponentSampleGap[Attempt];
				T->TestTrue(TEXT("Opponent destination remains visible"),OpponentSettled[Attempt]-OpponentDeployed[Attempt]>=DestinationMinimum);
				T->TestTrue(TEXT("Full board hold precedes next instruction in both attempts"),Now-OpponentSettled[Attempt]>=L->Timing(TEXT("OpponentDeploySettledHold"))-2.*OpponentSampleGap[Attempt]);
				T->TestTrue(TEXT("Both attempts visibly travel before command acceptance"),OpponentMoving[Attempt]>0 && OpponentDeployed[Attempt]-OpponentMoving[Attempt]>=L->Timing(TEXT("OpponentDeployMove"))*L->OpponentPace()-2.*OpponentSampleGap[Attempt]);
				T->AddInfo(FString::Printf(TEXT("GUIDED_OPPONENT_DEPLOY attempt=%d source=%.2fs destination=%.2fs settle=%.2fs total=%.2fs"),Attempt+1,
					OpponentDeployed[Attempt]-OpponentBegan[Attempt],OpponentSettled[Attempt]-OpponentDeployed[Attempt],Now-OpponentSettled[Attempt],Now-OpponentBegan[Attempt]));
			}
		}
		const auto& Scene=S->GetTacticalScene();
		if(L->GetStep()==EFMCodexLesson1Step::OpponentFinish || L->GetStep()==EFMCodexLesson1Step::OpponentMarker)
		{
			const auto Targets=FMCodexLesson1Focus::Targets(S,*L);
			T->TestEqual(TEXT("Only one opponent operation indicator"),Targets.Num(),1);
			if(Targets.Num()==1)
			{
				const bool Finish=L->GetStep()==EFMCodexLesson1Step::OpponentFinish;
				if(Finish) T->TestTrue(TEXT("End deployment indicates real B status, not a fake button"),Targets[0]==S->GetMatchHeader()->GetWidgetFromName(TEXT("RightPlayerBroadcastRegion")));
				else
				{
					const auto* Card=Cast<UFMCodexPlayerCardWidget>(Targets[0]);
					T->TestTrue(TEXT("Marker indicator points to actual deployed Stones"),Card && Card->GetPresentation().CardId==L->Stones());
				}
				bool& Captured=Finish?bFinishCaptured:bMarkerCaptured;
				if(!Captured && L->IsOpponentSettling()) { Capture(Finish?TEXT("10_OpponentFinish.png"):TEXT("10_OpponentMarker.png")); Captured=true; }
			}
		}
		if(C->GetInteractionView().InteractionCategory==EFMCodexLocalMatchInteractionCategory::RollLongShotDirectDefense)
		{
			T->TestTrue(TEXT("Reel-to-defense transition always yields tutorial"),L->YieldsToProduction(Scene,S->IsInlineFormulaRevealInputBlocked(),C->GetInteractionView().InteractionCategory));
			T->TestFalse(TEXT("No stale compact bar during attack-to-defense handoff"),FindSlate(LessonWindow->GetContent(),TEXT("LessonActionStrip")).IsValid());
			T->TestFalse(TEXT("No intermediate explanation modal during handoff"),FindSlate(LessonWindow->GetContent(),TEXT("LessonModalPanel")).IsValid());
			if(!S->IsInlineFormulaRevealInputBlocked())
			{
				bDefenseHandoff[L->IsComparison()?1:0]=true;
				if(!bHandoffCaptured) { Capture(TEXT("14_PostAttackHandoff.png")); bHandoffCaptured=true; }
			}
		}
		if (L->GetStep()==EFMCodexLesson1Step::FailurePause)
		{
			if (FailureVisibleAt==0) FailureVisibleAt=Now;
			T->TestTrue(TEXT("Failure hold leaves the disclosed production result unobstructed"),L->YieldsToProduction(Scene,false,C->GetInteractionView().InteractionCategory));
			if (!bFailureCaptured && Now-FailureVisibleAt>.25) { Capture(TEXT("11_CleanFailureHold.png")); bFailureCaptured=true; }
		}
		if (L->GetStep()==EFMCodexLesson1Step::FormulaHover && bFormulaHovered)
		{
			FSlateApplication::Get().UpdateToolTip(true); // Normal native tooltip update after real pointer hover.
			// Retain evidence while the transient tooltip is actually visible, before
			// the controller switches to its modal and Slate may close the tooltip.
			bFormulaTooltip |= FormulaTooltipVisible(S);
		}
		if (L->GetStep()==EFMCodexLesson1Step::Rewind && FailurePopupAt==0)
		{
			FailurePopupAt=Now;
			T->TestTrue(TEXT("Popup transition follows extended disclosed failure hold"),FailureVisibleAt>0 && FailurePopupAt-FailureVisibleAt>=L->Timing(TEXT("FailureFollowupDelay"))-.1);
			T->AddInfo(FString::Printf(TEXT("GUIDED_FAILURE_FOLLOWUP_DELAY %.2fs"),FailurePopupAt-FailureVisibleAt));
		}
		if(Scene.IsAnimating() || Scene.Celebration.IsActive())
		{
		 T->TestTrue(TEXT("Teaching yields to production action"),L->YieldsToProduction(Scene,S->IsInlineFormulaRevealInputBlocked(),C->GetInteractionView().InteractionCategory));
		 const auto Before=L->GetStep(); C->GuidedLesson1Primary();
		 T->TestEqual(TEXT("Tutorial primary cannot interrupt production"),L->GetStep(),Before);
		 if(!bSkipChecked && !L->IsComparison() && Scene.Phase==FMCodexTacticalScene::EPhase::Setup)
		 {
		  auto* Field=S->GetWidgetFromName(TEXT("TacticalScene"));
		  if(T->TestNotNull(TEXT("Production field receives skip"),Field))
		  { const auto At=Center(Field); Pointer(At); Pointer(At,1); Pointer(At,2); }
		  T->TestEqual(TEXT("Click advances only production beat"),Scene.Phase,FMCodexTacticalScene::EPhase::Intent);
		  T->TestEqual(TEXT("Click does not advance lesson"),L->GetStep(),Before); bSkipChecked=true;
		 }
		 if(Scene.Phase==FMCodexTacticalScene::EPhase::Outcome) (L->IsComparison()?bGoalOutcome:bFirstOutcome)=true;
		 if(Scene.Celebration.IsActive() && Scene.Celebration.Elapsed>.35f && !bCelebration)
		 { bCelebration=true; Capture(TEXT("05_NaturalGoalCelebration.png")); }
		 return false;
		}
		if(S->IsInlineFormulaRevealInputBlocked() || Now-Changed<1.4) return false;
		using Step=EFMCodexLesson1Step;
		const auto Current=L->GetStep();
		if(Current==LastActedStep && L->IsComparison()==bLastComparison && DragPhase==0) return false;
		LastActedStep=Current; bLastComparison=L->IsComparison(); Changed=Now;
		switch(Current)
		{
		case Step::Intro:
			Capture(TEXT("01_Intro.png"));
			C->GuidedLesson1Primary(); break;
		case Step::TacticPoint:
			if(auto* Status=S->GetInteractionPanel()->GetWidgetFromName(TEXT("InteractionBoundedFallback")))
				T->TestEqual(TEXT("Tutorial focus suppresses unrelated dock status"),Status->GetRenderOpacity(),0.f);
			Capture(TEXT("05_TacticPointFocus.png"));
			T->TestEqual(TEXT("TP semantic focus"),L->FocusTarget(),EFMCodexLesson1Focus::TacticPoint);
			FocusClick(S,*L); break;
		case Step::TacticPointExplanation:
			T->TestTrue(TEXT("TP explanation before deployment"),L->Instruction().ToString().Contains(TEXT("战术点为 3")));
			C->GuidedLesson1Primary(); break;
		case Step::InspectOdegaard: case Step::InspectGyokeres:
		{
			const auto Targets=FMCodexLesson1Focus::Targets(S,*L);
			if (!T->TestEqual(TEXT("One semantic hover target"),Targets.Num(),1)) return true;
			if(L->IsComparison())
			{
			 T->TestTrue(TEXT("Rewind replaced the production Screen"),OldScreen.Get()!=S);
			 if(OldScreen.IsValid()) T->TestEqual(TEXT("Old Scene reset before replacement"),OldScreen->GetTacticalScene().Phase,FMCodexTacticalScene::EPhase::Hidden);
			 T->TestEqual(TEXT("Rewind has no stale scene"),Scene.Phase,FMCodexTacticalScene::EPhase::Hidden);
			 T->TestFalse(TEXT("Rewind has no stale celebration"),Scene.Celebration.bConsumed);
			 T->TestEqual(TEXT("Rewind score restored"),C->GetInteractionView().PlayerAScore,0);
			}
			T->TestFalse(TEXT("Cannot drag before inspection"),S->GetLocalRackWidget()->GetRenderedCardWidgets()[0]->IsDeploymentDragEnabled());
			if (!L->IsComparison()) Capture(TEXT("07_HandFocus.png"));
			Pointer(Center(Targets[0]));
			T->TestTrue(TEXT("Real pointer opens Full Card"),S->IsDetailOverlayVisible());
			break;
		}
		case Step::ShootingExplanation:
			T->TestTrue(TEXT("Standalone Shooting teaching occurs only after rewind and Skill"),L->IsComparison() && bSecondSkill);
			T->TestTrue(TEXT("Real Shooting row retained"),S->IsDetailOverlayVisible() && S->GetDetailOverlayCard()->FindAttributePresentationWidget(TEXT("SHO")));
			Capture(TEXT("06_ShootingInspection.png"));
			C->GuidedLesson1Primary(); break;
		case Step::TraitExplanation:
			T->TestTrue(TEXT("Real Trait A row retained"),S->IsDetailOverlayVisible() && S->GetDetailOverlayCard()->FindTraitPresentationWidget(TEXT("Trait.LongShotCarrier")));
			Capture(TEXT("03_OdegaardTrait.png")); C->GuidedLesson1Primary(); break;
		case Step::SkillRangeExplanation:
			(L->IsComparison()?bSecondSkill:bFirstSkill)=true;
			if (const auto Panel=FindSlate(LessonWindow->GetContent(),TEXT("LessonModalPanel")); Panel.IsValid())
			{
				const auto& P=Panel->GetCachedGeometry(); const auto& W=LessonWindow->GetContent()->GetCachedGeometry();
				T->TestTrue(TEXT("Full Card teaching panel and CTA remain inside the viewport"),P.GetAbsolutePosition().X+P.GetAbsoluteSize().X<=W.GetAbsolutePosition().X+W.GetAbsoluteSize().X);
				const auto& D=S->GetDetailOverlayCard()->GetCachedGeometry();
				const auto Center=P.GetAbsolutePosition()+P.GetAbsoluteSize()*.5f;
				T->TestTrue(TEXT("Skill explanation clears Full Card"),P.GetAbsolutePosition().X>=D.GetAbsolutePosition().X+D.GetAbsoluteSize().X);
				T->TestTrue(TEXT("Skill explanation sits modestly below viewport center"),Center.Y>W.GetAbsolutePosition().Y+W.GetAbsoluteSize().Y*.5f);
				const float OldCenter=(W.GetAbsolutePosition().X+W.GetAbsoluteSize().X+D.GetAbsolutePosition().X+D.GetAbsoluteSize().X)*.5f;
				T->TestTrue(TEXT("Skill explanation moved toward visual center"),Center.X<OldCenter);
			}
			if(auto Exit=FindSlate(LessonWindow->GetContent(),TEXT("LessonExit")))
			{
				const auto& E=Exit->GetCachedGeometry(); const auto& D=S->GetDetailOverlayCard()->GetCachedGeometry();
				T->TestTrue(TEXT("Modal exit stays clear of real Full Card"),E.GetAbsolutePosition().X >= D.GetAbsolutePosition().X+D.GetAbsoluteSize().X);
			}
			T->TestTrue(TEXT("Full Card retained beside skill explanation"),S->IsDetailOverlayVisible() && S->GetDetailOverlayCard()->FindSkillPresentationWidget(L->Skill()));
			Capture(TEXT("06_SkillRangeInspection.png"));
			C->GuidedLesson1Primary();
			T->TestEqual(TEXT("Skill proceeds to first deployment or comparison Shooting"),L->GetStep(),L->IsComparison()?Step::ShootingExplanation:Step::Deploy); break;
		case Step::FinishExplanation: case Step::CarrierExplanation: case Step::SkillExplanation: case Step::DirectExplanation:
			T->TestTrue(TEXT("New concepts use explanation mode"),L->IsExplanationMode());
			if (Current==Step::DirectExplanation) T->TestTrue(TEXT("No secondary method lecture"),L->Explanation().IsEmpty());
			C->GuidedLesson1Primary(); break;
		case Step::Deploy:
			T->TestEqual(TEXT("Live TP 3"),C->GetInteractionView().ActionPoint,3);
			if(L->IsComparison() && !bCompared)
			{
				T->TestTrue(TEXT("Rewind live pitch empty"),C->GetInteractionView().DeploymentPlacements.IsEmpty());
				T->TestTrue(TEXT("Rewind has no stale Formula"),C->GetInteractionView().ResolutionFacts.FormulaContests.IsEmpty());
				T->TestEqual(TEXT("Real hand comparison has two choices"),S->GetPresentation().LocalRack.Cells.Num(),2);
				S->RequestDeployOrdinary(FFMCodexGuidedLesson1::Gyokeres(),FFMCodexGuidedLesson1::AttackerSlot());
				T->TestTrue(TEXT("Wrong comparison retains teaching step"),C->GetInteractionView().DeploymentPlacements.IsEmpty());
				const auto& Cards=S->GetLocalRackWidget()->GetRenderedCardWidgets();
				if(Cards.Num()>0)
				{
					Pointer(Center(Cards[0]));
					T->TestTrue(TEXT("Real Full Card hover opens"),S->IsDetailOverlayVisible());
				}
				bCompared=true; LastActedStep=Step::Intro; return false;
			}
			if(L->IsComparison() && DragPhase==0)
			{
				Capture(TEXT("03_ComparisonFullCard.png"));
				for(auto Card:S->GetLocalRackWidget()->GetRenderedCardWidgets()) Card->TakeWidget()->OnMouseLeave(FPointerEvent());
			}
			{
				const auto Targets=FMCodexLesson1Focus::Targets(S,*L);
				UFMCodexPlayerCardWidget* Source=nullptr; UFMCodexPitchSlotWidget* Target=nullptr;
				for (auto* W : Targets)
				{
					if (auto* Card=Cast<UFMCodexPlayerCardWidget>(W); Card && Card->GetPresentation().CardId==L->Attacker()) Source=Card;
					if (auto* Slot=Cast<UFMCodexPitchSlotWidget>(W)) Target=Slot;
				}
				if (!T->TestNotNull(TEXT("Focused drag source"),Source) || !T->TestNotNull(TEXT("Focused drop slot"),Target)) return true;
				if (DragPhase==0) { if(!L->IsComparison()) Capture(TEXT("07_DeploymentFocus.png")); Pointer(Center(Source)); Pointer(Center(Source),1); DragPhase=1; }
				else if (DragPhase==1) { Pointer(Center(Source)+FVector2D(24,0),0,true); DragPhase=2; }
				else { Pointer(Center(Target),0,true); Pointer(Center(Target),2); DragPhase=0;
					T->TestTrue(TEXT("Real Slate drag/drop deployed intended card"),C->GetInteractionView().DeploymentPlacements.Num()>0); }
				break;
			}
		case Step::FinishDeployment:
			if (!L->IsComparison()) Capture(TEXT("07_EndDeploymentFocus.png"));
			FocusClick(S,*L); break;
		case Step::Carrier:
			T->TestEqual(TEXT("Approved live carrier instruction"),L->Instruction().ToString(),L->IsComparison()
				?FString(TEXT("点击场上的厄德高，将他选中为本进攻回合的持球队员。")):FString(TEXT("点击场上的哲凯赖什，将他选中为本进攻回合的持球队员。")));
			if(!L->IsComparison()) Capture(TEXT("10_CarrierInstruction.png"));
			FocusClick(S,*L); break;
		case Step::Skill:
			T->TestTrue(TEXT("Canonical Runner absence"),C->GetInteractionView().SelectedRunnerCardId.IsNone());
			FocusClick(S,*L); break;
		case Step::DirectShot:
			T->TestTrue(TEXT("Production scene runs in lesson Method Choice"),Scene.Facts.bActive && Scene.Phase==FMCodexTacticalScene::EPhase::Preview);
			if(auto* Other=Cast<UButton>(S->GetWidgetFromName(TEXT("TheaterNearCombination"))))
				T->TestFalse(TEXT("Other method visibly disabled"),Other->GetIsEnabled());
			if(!L->IsComparison()) Capture(TEXT("08_DirectShotFocus.png")); FocusClick(S,*L); break;
		case Step::AttackRoll:
			if (!L->IsComparison()) T->TestTrue(TEXT("Real source tooltip taught before attack click"),bFormulaTooltip);
			if(L->IsComparison()) CheckTooltip(S,TEXT("TheaterAttackBaseHover"),TEXT("远射专家 A"));
			FocusClick(S,*L); break;
		case Step::FormulaHover:
		{
			T->TestEqual(TEXT("Teaching waits for stable production formula"),Scene.Phase,FMCodexTacticalScene::EPhase::FormulaHold);
			auto* Value=S->GetWidgetFromName(TEXT("TheaterAttackBaseHover"));
			auto* Number=Cast<UTextBlock>(S->GetWidgetFromName(TEXT("TheaterAttackNumber")));
			if (!T->TestNotNull(TEXT("Production attribute value"),Value) || !T->TestNotNull(TEXT("Production number"),Number)) return true;
			T->TestEqual(TEXT("First attack base really is 4"),Number->GetText().ToString(),FString(TEXT("4")));
			if (auto* Button=Cast<UButton>(S->GetWidgetFromName(TEXT("TheaterContinue"))))
				T->TestFalse(TEXT("Production CTA gated until inspection"),Button->GetIsEnabled());
			Capture(TEXT("12_FormulaHoverFocus.png"));
			Pointer(Center(Value)); bFormulaHovered=true; break;
		}
		case Step::FormulaExplanation:
		{
			bFormulaTooltip |= FormulaTooltipVisible(S);
			T->TestTrue(TEXT("Real mouse opened native production tooltip"),bFormulaTooltip);
			CheckTooltip(S,TEXT("TheaterAttackBaseHover"),TEXT("射门 4"));
			Capture(TEXT("13_FormulaSourceTeaching.png"));
			const auto Panel=FindSlate(LessonWindow->GetContent(),TEXT("LessonModalPanel"));
			T->TestTrue(TEXT("Formula reuses the same explanation component"),L->IsExplanationMode() && Panel.IsValid());
			if (Panel.IsValid())
			{
				const auto& P=Panel->GetCachedGeometry();
				const auto& F=S->GetWidgetFromName(TEXT("TheaterAttackPanelBounds"))->GetCachedGeometry();
				T->TestTrue(TEXT("Expanded explanation stays inside viewport top"),P.GetAbsolutePosition().Y>=LessonWindow->GetContent()->GetCachedGeometry().GetAbsolutePosition().Y);
				T->TestTrue(TEXT("Explanation leaves the real Formula visible below"),P.GetAbsolutePosition().Y+P.GetAbsoluteSize().Y<=F.GetAbsolutePosition().Y);
			}
			ClickSlate(TEXT("LessonExplanationContinue"));
			T->TestEqual(TEXT("Visible continue permits attack instruction"),L->GetStep(),Step::AttackRoll); break;
		}
		case Step::Rewind:
			T->TestTrue(TEXT("Follow-up waited for extended visible failure hold"),FailureVisibleAt>0 && Now-FailureVisibleAt>=L->Timing(TEXT("FailureFollowupDelay"))-.1);
			T->TestTrue(TEXT("Rewind uses only the concise main explanation"),L->Explanation().IsEmpty());
			T->TestTrue(TEXT("Clean failure moment captured before popup"),bFailureCaptured);
			T->TestTrue(TEXT("First production Outcome was observed"),bFirstOutcome);
			T->TestEqual(TEXT("Explanation waits for production ResultHold"),Scene.Phase,FMCodexTacticalScene::EPhase::ResultHold);
			CheckResult(C,9.f,10.f); OldScreen=S;
			Capture(TEXT("02_FirstMissAndRewind.png"));
			if (auto Panel=FindSlate(LessonWindow->GetContent(),TEXT("LessonModalPanel")))
				T->TestTrue(TEXT("Failure exit belongs to current teaching panel"),FindSlate(Panel.ToSharedRef(),TEXT("LessonExit"))==FindSlate(LessonWindow->GetContent(),TEXT("LessonExit")));
			CheckTooltip(S,TEXT("TheaterDefenseBaseHover"),TEXT("+3"));
			T->TestEqual(TEXT("First miss 0-0"),C->GetInteractionView().PlayerAScore,0);
			C->GuidedLesson1Primary(); break;
		case Step::Summary:
			T->TestTrue(TEXT("Natural Goal and celebration observed before summary"),bGoalOutcome && bCelebration);
			T->TestFalse(TEXT("Summary never covers active celebration"),Scene.Celebration.IsActive());
			CheckResult(C,11.f,10.f);
			if(!bSummaryInspected)
			{
				CheckTooltip(S,TEXT("TheaterAttackBaseHover"),TEXT("射门 4 +2"));
				CheckTooltip(S,TEXT("TheaterAttackBaseHover"),TEXT("远射专家 A"));
				auto* Base=S->GetWidgetFromName(TEXT("TheaterAttackBaseHover"));
				if(Base) FSlateApplication::Get().SetCursorPos(Base->GetCachedGeometry().GetAbsolutePosition()
					+Base->GetCachedGeometry().GetAbsoluteSize()*.5f);
				bSummaryInspected=true; LastActedStep=Step::Intro; return false;
			}
			Capture(TEXT("04_GoalAndSummary.png"));
			T->TestEqual(TEXT("Normal goal 1-0"),C->GetInteractionView().PlayerAScore,1);
			C->GuidedLesson1Primary(); break;
		case Step::Complete:
		{
			T->TestTrue(TEXT("Real click acceleration exercised"),bSkipChecked);
			T->TestTrue(TEXT("Opponent selected and placed states both visible"),bOpponentSource && bOpponentPlaced);
			T->TestTrue(TEXT("Both production attack-to-defense handoffs observed"),bDefenseHandoff[0] && bDefenseHandoff[1]);
			T->TestTrue(TEXT("Movement, finish and marker evidence observed"),bMoveCaptured && bFinishCaptured && bMarkerCaptured);
			T->TestTrue(TEXT("Exit cancel exercised"),bExitCancelChecked);
			T->AddInfo(FString::Printf(TEXT("GUIDED_LESSON1_FULL_PIE elapsed=%.1fs; two production attempts; miss -> checkpoint -> trait goal -> complete"),Now-Started));
			C->ConsoleCommand(TEXT("fm.Tutorial.Lesson1"));
			T->TestEqual(TEXT("Repeat command resets lesson without rebuild"),C->GetGuidedLesson1()->GetStep(),Step::Intro);
			C->GetPlayerMatchScreen()->RequestRollTacticalPoints();
			T->TestFalse(TEXT("Gated gesture cannot strand speculative reveal"),C->GetPlayerMatchScreen()->IsInlineFormulaRevealInputBlocked());
			C->ConsoleCommand(TEXT("fm.Tutorial.Exit"));
			T->TestNull(TEXT("Exit removes lesson"),C->GetGuidedLesson1());
			T->TestFalse(TEXT("Exit removes dim, arrows and hit-test overlay"),C->HasGuidedLesson1Overlay());
			T->TestEqual(TEXT("Normal hand restored"),C->GetPlayerMatchScreen()->GetPresentation().LocalRack.Cells.Num(),20);
			bExited=true; Changed=Now; return false;
		}
		default: break;
		}
		return false;
	}
private:
	bool FormulaTooltipVisible(UFMCodexLocalMatchScreenWidget* S) const
	{
		auto* Value=S->GetWidgetFromName(TEXT("TheaterAttackBaseHover"));
		const auto Tip=Value && Value->GetToolTip()?Value->GetToolTip()->GetCachedWidget():TSharedPtr<SWidget>();
		const auto Window=Tip.IsValid()?FSlateApplication::Get().FindWidgetWindow(Tip.ToSharedRef()):TSharedPtr<SWindow>();
		return Window.IsValid() && Window->IsVisible();
	}
	TSharedPtr<SWidget> FindSlate(const TSharedRef<SWidget>& W, FName Tag)
	{
		if (!W->GetVisibility().IsVisible()) return nullptr;
		if(W->GetTag()==Tag) return W;
		FChildren* Children=W->GetChildren();
		for(int32 I=0;I<Children->Num();++I) if(auto Found=FindSlate(Children->GetChildAt(I),Tag)) return Found;
		return nullptr;
	}
	void ClickSlate(FName Tag)
	{
		auto Widget=FindSlate(LessonWindow->GetContent(),Tag);
		if(!T->TestTrue(TEXT("Tutorial semantic Slate control exists"),Widget.IsValid())) return;
		const auto& G=Widget->GetCachedGeometry(); const auto At=G.LocalToAbsolute(G.GetLocalSize()*.5f);
		Pointer(At); Pointer(At,1); Pointer(At,2);
	}

	FVector2D Center(UWidget* W) { const auto& G=W->GetCachedGeometry(); return G.LocalToAbsolute(G.GetLocalSize()*.5); }
	void Pointer(FVector2D At, int32 Button=0, bool Held=false)
	{
		auto& App=FSlateApplication::Get(); const auto Before=App.GetCursorPos(); App.SetCursorPos(At);
		TSet<FKey> Buttons; if (Held || Button==1) Buttons.Add(EKeys::LeftMouseButton);
		FPointerEvent E(0,At,Before,Buttons,Button?EKeys::LeftMouseButton:EKeys::Invalid,0,FModifierKeysState());
		if (Button==1) App.ProcessMouseButtonDownEvent(nullptr,E);
		else if (Button==2) App.ProcessMouseButtonUpEvent(E);
		else App.ProcessMouseMoveEvent(E,false);
	}
	void FocusClick(UFMCodexLocalMatchScreenWidget* S, const FFMCodexGuidedLesson1& L)
	{
		const auto Targets=FMCodexLesson1Focus::Targets(S,L);
		if (!T->TestEqual(TEXT("Real semantic button target"),Targets.Num(),1)) return;
		const auto At=Center(Targets[0]); Pointer(At); Pointer(At,1); Pointer(At,2);
	}
	void CheckResult(AFMCodexLocalMatchPlayerController* C,float Attack,float Defense)
	{
	 const auto& Facts=C->GetInteractionView().ResolutionFacts.FormulaContests;
	 if(!T->TestEqual(TEXT("Production contest exists"),Facts.Num(),1))return;
	 T->TestEqual(TEXT("Production attack die"),Facts[0].ResolvedInput.Attacker.ComparePoint,5);
	 T->TestEqual(TEXT("Production defense die"),Facts[0].ResolvedInput.Defender.ComparePoint,3);
	 T->TestEqual(TEXT("Production attack total"),Facts[0].AttackRow.FinalValue,Attack);
	 T->TestEqual(TEXT("Production defense total"),Facts[0].DefenseRow.FinalValue,Defense);
	}
	void CheckTooltip(UFMCodexLocalMatchScreenWidget* S,const TCHAR* Name,const TCHAR* Expected)
	{
		auto* Base=Cast<UBorder>(S->GetWidgetFromName(Name));
		auto* Tip=Base?Cast<UBorder>(Base->GetToolTip()):nullptr;
		auto* Bounds=Tip?Cast<USizeBox>(Tip->GetContent()):nullptr;
		auto* Text=Bounds?Cast<UTextBlock>(Bounds->GetContent()):nullptr;
		T->TestTrue(FString(TEXT("Production Formula source: "))+Expected,Text && Text->GetText().ToString().Contains(Expected));
	}
	void Click(UFMCodexLocalMatchScreenWidget* S,const TCHAR* Name)
	{
		auto* B=Cast<UButton>(S->GetWidgetFromName(Name));
		if(T->TestNotNull(TEXT("Production button exists"),B) && T->TestTrue(TEXT("Production button enabled"),B->GetIsEnabled())) B->OnClicked.Broadcast();
	}
	void Capture(const TCHAR* Name)
	{
		// Two representative migration views; behavioral assertions cover the rest.
		const FString N(Name);
		if(N!=TEXT("01_Intro.png") && N!=TEXT("13_FormulaSourceTeaching.png") && N!=TEXT("NormalHoverFailure.png")) return;
		auto Window=GEditor->PlayWorld->GetGameViewport()->GetWindow();
		TArray<FColor> Pixels; FIntVector Size = FIntVector::ZeroValue;
		if(!Window || !FSlateApplication::Get().TakeScreenshot(Window->GetContent(),Pixels,Size)) { T->AddError(TEXT("PIE capture unavailable")); return; }
		const FString Dir=FPaths::ProjectSavedDir()/TEXT("Stage8_25A/PIE"); IFileManager::Get().MakeDirectory(*Dir,true);
		TArray64<uint8> PNG; FImageUtils::PNGCompressImageArray(Size.X,Size.Y,Pixels,PNG);
		T->TestTrue(TEXT("Actual lesson window captured"),FFileHelper::SaveArrayToFile(PNG,*(Dir/Name)));
	}
	FAutomationTestBase* T;
	bool bCanonicalChecked=false;
	double Started=FPlatformTime::Seconds(),Changed=0;
	double NormalHoverSentAt=0;
	TWeakObjectPtr<UFMCodexPlayerCardWidget> NormalHoverCard;
	double FailureVisibleAt=0,FailurePopupAt=0;
	double OpponentBegan[2]={},OpponentDeployed[2]={},OpponentSettled[2]={},OpponentFinished[2]={};
	double OpponentObserved[2]={},OpponentSampleGap[2]={};
	double OpponentMoving[2]={};
	bool bMoveCaptured=false,bFinishCaptured=false,bMarkerCaptured=false,bHandoffCaptured=false,bDefenseHandoff[2]={};
	bool bFormulaHovered=false,bFormulaTooltip=false,bFailureCaptured=false;
	bool bStarted=false,bCompared=false,bLastComparison=false,bSummaryInspected=false,bExited=false;
	bool bSkipChecked=false,bFirstOutcome=false,bGoalOutcome=false,bCelebration=false;
	TWeakObjectPtr<UFMCodexLocalMatchScreenWidget> OldScreen;
	int32 DragPhase=0,ExitPhase=0;
	int64 ExitSequence=0;
	TWeakObjectPtr<UFMCodexLocalMatchScreenWidget> ExitScreen;
	bool bExitCancelChecked=false,bFirstSkill=false,bSecondSkill=false,bOpponentSource=false,bOpponentPlaced=false,bSourceCaptured=false,bPlacedCaptured=false;
	EFMCodexLesson1Step LastActedStep=EFMCodexLesson1Step::Complete;
};
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLessonFullPIETest,"FMCodex.PIE.GuidedLesson1.FullFlow",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FLessonFullPIETest::RunTest(const FString&)
{
	FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShareable(new FStartLessonPIE()));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(2.f));
	FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShareable(new FPlayLessonPIE(this)));
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	return true;
}
#endif
