#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && !UE_BUILD_SHIPPING
#include "FMCodexLocalMatchPlayerController.h"
#include "FMCodexLocalMatchScreenWidget.h"
#include "FMCodexLongShotResolutionSurfaceWidget.h"
#include "FMCodexRollReelWidget.h"
#include "FMCodexCardRackWidget.h"
#include "FMCodexPlayerCardWidget.h"
#include "FMCodexInteractionPanelWidget.h"
#include "FMCodexInteractionOptionWidget.h"
#include "FMCodexTacticalDetailPanelWidget.h"
#include "FMCodexResolutionPanelWidget.h"
#include "Components/PanelWidget.h"
#include "FMCodexOutcomePresentation.h"
#include "Components/Button.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "Components/RichTextBlock.h"
#include "Editor.h"
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
TSharedPtr<SWindow> ShotPIEWindow;
class FStartDirectShotPIE final : public IAutomationLatentCommand
{
public:
	bool Update() override
	{
		auto* Settings=DuplicateObject<ULevelEditorPlaySettings>(GetDefault<ULevelEditorPlaySettings>(),GetTransientPackage());
		Settings->NewWindowWidth=1920; Settings->NewWindowHeight=1080;
		Settings->SetPlayNetMode(EPlayNetMode::PIE_Standalone); Settings->SetPlayNumberOfClients(1);
		ShotPIEWindow=SNew(SWindow).Title(FText::FromString(TEXT("Shot Theater PIE")))
			.ClientSize(FVector2D(1920,1080)).AutoCenter(EAutoCenter::None).ScreenPosition(FVector2D(0,0))
			.SaneWindowPlacement(false).AdjustInitialSizeAndPositionForDPIScale(false);
		FSlateApplication::Get().AddWindow(ShotPIEWindow.ToSharedRef());
		FRequestPlaySessionParams Params; Params.EditorPlaySettings=Settings; Params.CustomPIEWindow=ShotPIEWindow;
		GEditor->RequestPlaySession(Params); return true;
	}
};

// Real Local PIE: normal Screen intents, host-owned DEV provider, natural game
// time and rendering. No state mutation, forced outcomes or test-clock stepping.
class FDirectShotPIE final : public IAutomationLatentCommand
{
public:
	explicit FDirectShotPIE(FAutomationTestBase* InTest, bool bInDead=false, bool bInMiss=false, int32 InCountdownMode=0, bool bInCut=false, bool bInCornerLayout=false)
		: Test(InTest), bDead(bInDead), bMiss(bInMiss), bCut(bInCut), bCornerLayout(bInCornerLayout), CountdownMode(InCountdownMode) {}
	bool Update() override
	{
		if (Started == 0.) Started = FPlatformTime::Seconds();
		if (FPlatformTime::Seconds()-Started>120.) {Test->AddError(FString::Printf(TEXT("Shot PIE timed out at step %d"),Step));return true;}
		if (!GEditor || !GEditor->PlayWorld) return false;
		auto* C=Cast<AFMCodexLocalMatchPlayerController>(GEditor->PlayWorld->GetFirstPlayerController());
		auto* S=C?C->GetPlayerMatchScreen():nullptr;
		if (!S || FPlatformTime::Seconds()-Changed<.25) return false;
		auto Next=[&](){++Step;Changed=FPlatformTime::Seconds();};
		auto Shown=[S](const TCHAR* Name){auto* W=S->GetWidgetFromName(Name);return W && W->GetVisibility()!=ESlateVisibility::Collapsed && W->GetVisibility()!=ESlateVisibility::Hidden;};
		auto Text=[S](const TCHAR* Name){return CastChecked<UTextBlock>(S->GetWidgetFromName(Name))->GetText().ToString();};
		if (bMiss && !bCut && Step==6)
		{
			const auto Card=S->GetWidgetFromName(TEXT("TheaterAttackPanelBounds"))->GetCachedGeometry().GetLocalSize();
			const auto Figure=S->GetWidgetFromName(TEXT("TheaterAttackSilhouetteBounds"))->GetCachedGeometry().GetLocalSize();
			Test->TestTrue(TEXT("Natural ImmediateMiss keeps attack card dimensions"),Card.Equals(AttackCardSize,.1f));
			Test->TestTrue(TEXT("Natural ImmediateMiss keeps silhouette dimensions"),Figure.Equals(AttackFigureSize,.1f));
			if (!Shown(TEXT("TheaterAttackBaseHover")) && !bMissTransitionCaptured)
			{
				Capture(TEXT("ImmediateMiss_Landed.png")); bMissTransitionCaptured=true;
				Test->AddInfo(FString::Printf(TEXT("SHOT_GEOMETRY card=%s silhouette=%s unchanged=1"),*Card.ToString(),*Figure.ToString()));
			}
		}
		if (S->IsInlineFormulaRevealInputBlocked())
		{
			if(bCut && (Step==6 || Step==7))
			{
				Test->TestTrue(TEXT("CutInside waiting loop continues through Roll v2"),S->GetTacticalScene().IsCutInsideIdle());
				Test->TestEqual(TEXT("No future spatial outcome during dice reveal"),S->GetTacticalScene().Facts.Outcome,FMCodexTacticalScene::EOutcome::None);
			}
			if (bDead && Step==6)
			{
				auto* A=CastChecked<UFMCodexRollReelWidget>(S->GetWidgetFromName(TEXT("TheaterPairAReel")));
				auto* B=CastChecked<UFMCodexRollReelWidget>(S->GetWidgetFromName(TEXT("TheaterPairBReel")));
				Test->TestTrue(TEXT("Real pair reuses FK TheaterInline slots"),A->UsesTheaterInlineSkin() && B->UsesTheaterInlineSkin());
				Test->TestFalse(TEXT("Pair never paints a Formula or final CTA early"),Shown(TEXT("TheaterAttackValue")) || Shown(TEXT("TheaterDefensePanelBounds")) || Shown(TEXT("TheaterPrimaryBounds")) || Shown(TEXT("TheaterOutcome")));
				Test->TestTrue(TEXT("Natural paired reveal keeps the goal condition visible"),Shown(TEXT("TheaterReasonSecondary"))
					&& Text(TEXT("TheaterReasonSecondary"))==TEXT("两枚掷点总和达到 11–12：进球"));
				if (A->GetPresentation().bMoving) bAttackMotion=true;
				if (B->GetPresentation().bMoving)
				{
					Test->TestEqual(TEXT("First real die remains landed during second motion"),Text(TEXT("TheaterPairARollValue")),FString(TEXT("5")));
					Test->TestEqual(TEXT("Total is not revealed during second motion"),Text(TEXT("TheaterPairTotal")),FString(TEXT("?")));
					if (!bDefenseMotion) Capture(TEXT("DeadCorner_SecondRolling.png"));
					bDefenseMotion=true;
				}
				if (B->GetPresentation().bAuthoritativeValue) bDefenseLanded=B->GetPresentation().CenterValue==5;
				return false;
			}
			if (Step==6 || Step==7)
			{
				auto* Reel=CastChecked<UFMCodexRollReelWidget>(S->GetWidgetFromName(Step==6?TEXT("TheaterAttackReel"):TEXT("TheaterDefenseReel")));
				Test->TestTrue(TEXT("Real DirectShot uses existing TheaterInline"),Reel->UsesTheaterInlineSkin());
				Test->TestFalse(TEXT("Natural reveal blocks final action"),Shown(TEXT("TheaterPrimaryBounds")));
				if (Reel->GetPresentation().bMoving)
				{
					if (Step==6)
					{
						if (bMiss)
						{
							Test->TestTrue(TEXT("Undisclosed immediate miss preserves normal attack composition"),Shown(TEXT("TheaterDefensePanelBounds")) && Shown(TEXT("TheaterAttackBaseHover")));
							Test->TestFalse(TEXT("There is no duplicate standalone roll"),Shown(TEXT("TheaterRoll")));
							Test->TestFalse(TEXT("Natural motion cannot pre-leak landed status"),Text(TEXT("TheaterDetail")).Contains(TEXT("已落定")));
							if (!bAttackMotion && !bCut) Capture(TEXT("ImmediateMiss_AttackRolling.png"));
						}
						bAttackMotion=true;
					}
					else
					{
						Test->TestEqual(TEXT("Real defense motion retains attack D6"),Text(TEXT("TheaterAttackRollValue")),FString(TEXT("4")));
						if (!bDefenseMotion) Capture(TEXT("LongShot_DefenseRolling.png"));
						bDefenseMotion=true;
					}
				}
				if (Reel->GetPresentation().bAuthoritativeValue)
				{
					Test->TestEqual(TEXT("Real provider result lands naturally"),Reel->GetPresentation().CenterValue,Step==6?(bMiss?2:4):3);
					if (bMiss) Test->TestEqual(TEXT("Every naturally landed frame has current status"),Text(bCut?TEXT("TheaterStatus"):TEXT("TheaterDetail")),FString(bCut?TEXT("掷点已落定"):TEXT("进攻方掷点已落定")));
					if (Step==6) bAttackLanded=true; else bDefenseLanded=true;
				}
			}
			return false;
		}
		if (Step==0) {if (CountdownMode != 2) S->RequestStartNewMatch();Next();return false;}
		if (Step==1)
		{
			CheckTacticalButton(*S);
			if (bDead)
			{
				const auto& Cards=S->GetLocalRackWidget()->GetRenderedCardWidgets();
				if (!Test->TestFalse(TEXT("Pre-TP hand exists"),Cards.IsEmpty())) return true;
				auto* Card=Cards[0].Get();
				Card->TakeWidget()->OnMouseEnter(Card->GetCachedGeometry(),FPointerEvent());
				Test->TestTrue(TEXT("Real pre-TP Hand hover opens read-only inspector"),S->IsDetailOverlayVisible());
				Card->TakeWidget()->OnMouseLeave(FPointerEvent());
			}
			if (!Override(*C,EFMCodexLocalDevRollTarget::FullD12,4)) return true;
			S->RequestRollTacticalPoints();Next();return false;
		}
		if (Step==2)
		{
			const bool IsA=C->GetInteractionView().CurrentAttackingPlayer==EInitialTurnOrderPlayer::PlayerA;
			Carrier=bCut?(IsA?TEXT("Prototype.Arsenal.BukayoSaka"):TEXT("Prototype.ManchesterCity.JeremyDoku"))
				:(IsA?TEXT("Prototype.Arsenal.MartinOdegaard"):TEXT("Prototype.ManchesterCity.PhilFoden"));
			const FString Own=IsA?TEXT("NearA"):TEXT("NearB"),Forward=IsA?TEXT("NearB"):TEXT("NearA");
			for (int32 Side=0;Side<2;++Side)
			{
				for (int32 I=0;I<4;++I)
				{
					const FName Required=Side==0 && I==0?Carrier:NAME_None;
					const FString Half=I<2?Own:Forward;
					const auto* O=C->GetInteractionView().DeploymentOptions.FindByPredicate([&](const auto& Candidate)
					{return !Candidate.bGoalkeeper && Candidate.SlotId.ToString().Contains(Half)
						&& (Required.IsNone()?Candidate.CardId!=Carrier:Candidate.CardId==Required);});
					if (!Test->TestNotNull(TEXT("Canonical legal deployment option"),O)) return true;
					const FName Card=O->CardId,Slot=O->SlotId; S->RequestDeployOrdinary(Card,Slot);
					if (!Accepted(*C)) return true;
				}
				S->RequestFinishDeployment();if (!Accepted(*C)) return true;
			}
			Next();return false;
		}
		if (Step==3)
		{
			using DirectPIECategory=EFMCodexLocalMatchInteractionCategory;
			const auto& V=C->GetInteractionView();
			switch (V.InteractionCategory)
			{
			case DirectPIECategory::SelectCarrier:S->RequestSubmitCarrier(Carrier);break;
			case DirectPIECategory::SelectMarker:
				if (!Test->TestFalse(TEXT("Legal Marker exists"),V.SelectionOptions.IsEmpty())) return true;
				S->RequestSubmitMarker(V.SelectionOptions[0].Id);break;
			case DirectPIECategory::SelectRunner:
			case DirectPIECategory::SelectHelper:
				if (V.bCanResolveNoLegalChoice) S->RequestResolveNoLegalSelection(); else S->RequestDeclineSelection();break;
			case DirectPIECategory::SelectSkill:
			{
				const auto* Choice=S->GetPresentation().Interaction.SelectionChoices.FindByPredicate([&](const auto& O){return O.SkillType==(bCut?ESkillRuleType::CutInsideShot:ESkillRuleType::LongShot) && O.bEnabled;});
				if (!Test->TestNotNull(TEXT("Reached genuine ordinary shot Tactical Choice"),Choice)) return true;
				if (bCut) {Step=31;Changed=FPlatformTime::Seconds();return false;}
				S->RequestSubmitSkill(Choice->OptionId);if (!Accepted(*C)) return true;Next();return false;
			}
			default:Test->AddError(TEXT("Unexpected ordinary selection state"));return true;
			}
			if (!Accepted(*C)) return true;Changed=FPlatformTime::Seconds();return false;
		}
		if (bCut && Step>=31 && Step<=33)
		{
			auto FindOption=[S](bool Direct)->UFMCodexInteractionOptionWidget*
			{
				const auto& Options=S->GetInteractionPanel()->GetRenderedOptionWidgets();
				const auto* Found=Options.FindByPredicate([Direct](const auto& O){return O->GetLabel()==(Direct?TEXT("内切（直接射门）"):TEXT("内切（射向死角）"));});
				return Found?*Found:nullptr;
			};
			auto* Direct=FindOption(true);auto* Corner=FindOption(false);
			if (!Test->TestNotNull(TEXT("Real direct tactical option"),Direct) || !Test->TestNotNull(TEXT("Real corner tactical option"),Corner)) return true;
			auto* DirectButton=CastChecked<UButton>(Direct->GetWidgetFromName(TEXT("InteractionOptionButton")));
			auto* CornerButton=CastChecked<UButton>(Corner->GetWidgetFromName(TEXT("InteractionOptionButton")));
			if (Step==31)
			{
				Test->TestFalse(TEXT("Method decision precedes theater"),Shown(TEXT("ResolutionTheater")));
				for (auto* Option:{Direct,Corner})
				{
					auto* Label=Option->GetWidgetFromName(TEXT("InteractionOptionLabel"));
					const auto& BG=Option->GetWidgetFromName(TEXT("InteractionOptionButton"))->GetCachedGeometry();
					const auto& LG=Label->GetCachedGeometry();const auto& SG=S->GetCachedGeometry();
					const auto Min=BG.AbsoluteToLocal(LG.LocalToAbsolute(FVector2D::ZeroVector));
					const auto Max=BG.AbsoluteToLocal(LG.LocalToAbsolute(LG.GetLocalSize()));
					Test->TestTrue(TEXT("Full long label fits without clipping"),LG.GetLocalSize().X+1>=Label->GetDesiredSize().X
						&& Min.X>=0 && Min.Y>=0 && Max.X<=BG.GetLocalSize().X+1 && Max.Y<=BG.GetLocalSize().Y+1);
					Test->TestTrue(TEXT("Both tactical buttons fit current viewport"),SG.AbsoluteToLocal(BG.LocalToAbsolute(FVector2D::ZeroVector)).X>=0
						&& SG.AbsoluteToLocal(BG.LocalToAbsolute(BG.GetLocalSize())).X<=SG.GetLocalSize().X+1);
				}
				DirectButton->OnHovered.Broadcast();Next();return false;
			}
			if (Step==32)
			{
				Test->TestEqual(TEXT("Real Direct hover is branch-specific"),S->GetTacticalDetailPanel()->GetPresentation().DisplayName,FString(TEXT("内切（直接射门）")));
				Capture(TEXT("CutInside_DirectHover.png"));
				DirectButton->OnUnhovered.Broadcast();CornerButton->OnHovered.Broadcast();Next();return false;
			}
			Test->TestEqual(TEXT("Real Corner hover is branch-specific"),S->GetTacticalDetailPanel()->GetPresentation().DisplayName,FString(TEXT("内切（射向死角）")));
			Capture(TEXT("CutInside_CornerHover.png"));
			(bCornerLayout?CornerButton:DirectButton)->OnClicked.Broadcast();if (!Accepted(*C)) return true;
			Step=5;Changed=FPlatformTime::Seconds();return false;
		}
		if (Step==4)
		{
			if (FPlatformTime::Seconds()-Changed<1.) return false;
			Test->TestTrue(TEXT("Actual shot choice now stays in Theater"),Shown(TEXT("ResolutionTheater")) && Shown(TEXT("TheaterNearMethods")));
			if (CountdownMode == 0)
			{
			Test->TestTrue(TEXT("Actual branch has a named Carrier context"),Shown(TEXT("TheaterDuel")) && !Text(TEXT("TheaterAttackName0")).IsEmpty() && Text(TEXT("TheaterAttackRole0"))==TEXT("持球"));
			Test->TestEqual(TEXT("Actual branch uses the concise direct summary"),Text(TEXT("TheaterNearDirectHint")),FString(TEXT("持球：远射\n对抗盯人：抢断\n进攻掷点 1–2：射门偏出")));
			Test->TestEqual(TEXT("Actual pair choice keeps only the success condition"),Text(TEXT("TheaterNearCombinationHint")),FString(TEXT("进攻方依次掷两枚骰子\n总和 11–12：进球")));
			}
			if (bDead) Capture(TEXT("Shot_BranchChoice.png"));
			CastChecked<UButton>(S->GetWidgetFromName(bDead?TEXT("TheaterNearCombination"):TEXT("TheaterNearDirect")))->OnClicked.Broadcast();
			if (!Accepted(*C)) return true;Next();return false;
		}
		if ((bDead || bMiss) && Step==6)
		{
			if(bCut && !Shown(TEXT("CutInsideMissBounds")))return false; // Let the existing spatial beat finish naturally.
			Test->TestTrue(TEXT("Real natural motion reached its terminal"),bAttackMotion && (bDead?bDefenseMotion && bDefenseLanded:bAttackLanded));
			Test->TestTrue(TEXT("Existing terminal owns Outcome and CTA"),C->GetInteractionView().bTerminalPendingAdvance && Shown(TEXT("TheaterOutcome")) && Shown(TEXT("TheaterPrimaryBounds")));
			const auto& Shot=S->GetPresentation().LongShotResolution;
			const auto& Narrative=bDead?Shot.NarrativeHeadline:Shot.Formula.NarrativeHeadline;
			Test->TestFalse(TEXT("Canonical terminal narrative is populated"),Narrative.IsEmpty());
			Test->TestEqual(TEXT("Theater renders the complete canonical narrative"),
				CastChecked<URichTextBlock>(S->GetWidgetFromName(TEXT("TheaterOutcome")))->GetText().ToString(),
				FMCodexOutcomePresentation::PrimaryMarkup(Narrative,bDead?Shot.OutcomeText:Shot.Formula.OutcomeText));
			if (bMiss) Test->TestFalse(TEXT("Revealed miss has no defense or standalone duplicate"),Shown(TEXT("TheaterDefensePanelBounds")) || Shown(TEXT("TheaterRoll")));
			if (bDead)
			{
				Test->TestEqual(TEXT("Natural miss keeps its arithmetic total"),Text(TEXT("TheaterPairTotal")),FString(TEXT("10")));
				Test->TestTrue(TEXT("Natural miss explains the unmet goal condition"),Shown(TEXT("TheaterReasonSecondary"))
					&& Text(TEXT("TheaterReasonSecondary"))==TEXT("总和未达到 11–12，未进球"));
			}
			Capture(bDead?TEXT("DeadCorner_Resolved.png"):TEXT("ImmediateMiss_Resolved.png"));
			if(bCut)
			{
				Test->TestFalse(TEXT("Compact miss replaces the empty duel"),Shown(TEXT("TheaterDuel")));
				Test->TestEqual(TEXT("Compact miss retains the actual die"),Text(TEXT("CutInsideMissDie")),FString(TEXT("2")));
				Test->TestFalse(TEXT("Stable result has no reveal-in-progress copy"),Text(TEXT("TheaterStatus")).Contains(TEXT("正在揭示")));
				Test->TestTrue(TEXT("Miss banner remains visible"),Shown(TEXT("CrossResultPlate")));
				return true;
			}
			Test->AddInfo(bDead?TEXT("SHOT_COMPLETION_PIE DeadCorner pair=5,5 miss_reason=1 rolling_hint=1 natural_hold=1 terminal=1"):TEXT("SHOT_COMPLETION_PIE ImmediateMiss attack=2 stale_status=0 defense_rolls=0 natural_hold=1 terminal=1"));
			CastChecked<UButton>(S->GetWidgetFromName(TEXT("TheaterContinue")))->OnClicked.Broadcast();
			if (!Accepted(*C)) return true;Step=8;Changed=FPlatformTime::Seconds();return false;
		}
		if (Step==5 || Step==6)
		{
			if (FPlatformTime::Seconds()-Changed<1.) return false;
			if(bCut && Step==5)
			{
				Test->TestFalse(TEXT("Early selection removes duplicate theater chooser"),Shown(TEXT("TheaterNearMethods")));
				Test->TestEqual(TEXT("Selected method awaits its manual roll"),S->GetPresentation().Interaction.Category,bCornerLayout?EFMCodexUMGInteractionCategory::RollCutInsideShotDeadCorner:EFMCodexUMGInteractionCategory::RollCutInsideShotDirectAttack);
				const auto& Scene=S->GetTacticalScene();
				if(!Scene.IsCutInsideIdle())return false;
				const auto Position=FMCodexTacticalScene::CutInsideParticipantAnchor(Scene,EMatchPlayResolutionParticipantRole::Carrier);
				if(IdleSeen<0.){IdleSeen=GEditor->PlayWorld->GetTimeSeconds();IdleCarrier=Position;return false;}
				if(GEditor->PlayWorld->GetTimeSeconds()-IdleSeen<1.2)return false;
				Test->TestTrue(TEXT("Natural painted readiness starts CutInside motion"),Scene.CrossEntrance.Progress()>=1.f);
				Test->TestTrue(TEXT("Actual waiting frame visibly moves Carrier"),(Position-IdleCarrier).Size()>5.f);
				Test->TestTrue(TEXT("Ball stays beside Carrier through natural wait"),(FMCodexTacticalScene::CutInsideBall(Scene)-Position).Equals(FVector2D(48,24),.01f));
				Test->TestTrue(TEXT("Both methods wait near the upper sideline"),Position.Y<240.f && Position.X>700.f);
				if(bCornerLayout)
				{
					const auto& Card=S->GetWidgetFromName(TEXT("TheaterAttackPanel"))->GetCachedGeometry();
					const auto& Row=S->GetWidgetFromName(TEXT("CrossBroadcastRow"))->GetCachedGeometry();
					for(const auto Name:{TEXT("TheaterAttackSide"),TEXT("CutInsidePairRule"),TEXT("TheaterPair"),TEXT("CutInsidePairGoalRule")})
					{
						const auto& Child=S->GetWidgetFromName(Name)->GetCachedGeometry();
						const auto Min=Card.AbsoluteToLocal(Child.LocalToAbsolute(FVector2D::ZeroVector));
						const auto Max=Card.AbsoluteToLocal(Child.LocalToAbsolute(Child.GetLocalSize()));
						Test->TestTrue(FString::Printf(TEXT("%s stays inside the challenge frame"),Name),Min.X>=0 && Min.Y>=0 && Max.X<=Card.GetLocalSize().X+1 && Max.Y<=Card.GetLocalSize().Y+1);
					}
					const auto& Title=S->GetWidgetFromName(TEXT("TheaterAttackSide"))->GetCachedGeometry();
					const auto& Rule=S->GetWidgetFromName(TEXT("CutInsidePairRule"))->GetCachedGeometry();
					Test->TestTrue(TEXT("Dice explanation is beside the title"),Rule.GetAbsolutePosition().X>=Title.LocalToAbsolute(Title.GetLocalSize()).X && FMath::Abs(Rule.GetAbsolutePosition().Y-Title.GetAbsolutePosition().Y)<20);
					const auto ButtonLeft=Card.AbsoluteToLocal(S->GetWidgetFromName(TEXT("TheaterPrimaryBounds"))->GetCachedGeometry().GetAbsolutePosition()).X;
					const auto& Band=S->GetWidgetFromName(TEXT("CrossBroadcastBand"))->GetCachedGeometry();
					const auto GroupCenter=Band.AbsoluteToLocal(Row.LocalToAbsolute(Row.GetLocalSize()*.5f));
					Test->TestTrue(TEXT("Complete three-module group is horizontally centered"),FMath::Abs(GroupCenter.X-Band.GetLocalSize().X*.5f)<1.f);
					Test->TestTrue(TEXT("Button keeps breathing room beside the card"),ButtonLeft-Card.GetLocalSize().X>=20 && ButtonLeft-Card.GetLocalSize().X<40);
					Capture(TEXT("CutInside_CornerLayout.png"));
					return true;
				}
				Test->TestEqual(TEXT("Broadcast replaces the old tall panel"),S->GetWidgetFromName(TEXT("CrossBroadcastBounds"))->GetVisibility(),ESlateVisibility::SelfHitTestInvisible);
				Capture(TEXT("CutInside_Wait.png"));
				if(!bCutTipChecked && !bMiss)
				{
					auto* Hover=CastChecked<UBorder>(S->GetWidgetFromName(TEXT("TheaterDefenseBaseHover")));
					auto& App=FSlateApplication::Get();
					if(CutTipStarted==0)
					{
						const FVector2D Before=App.GetCursorPos();const auto Pos=Hover->GetCachedGeometry().GetAbsolutePosition()+Hover->GetCachedGeometry().GetAbsoluteSize()*.5;
						App.SetCursorPos(Pos);App.ProcessMouseMoveEvent(FPointerEvent(0,Pos,Before,TSet<FKey>(),EKeys::Invalid,0,FModifierKeysState()),false);
						CutTipStarted=FPlatformTime::Seconds();return false;
					}
					const auto Tip=Hover->GetToolTip()->GetCachedWidget();const auto Window=Tip.IsValid()?App.FindWidgetWindow(Tip.ToSharedRef()):TSharedPtr<SWindow>();
					if(!Window.IsValid() || !Window->IsVisible())
					{
						App.UpdateToolTip(true);if(FPlatformTime::Seconds()-CutTipStarted<5.)return false;
						Test->AddError(TEXT("CutInside native calculation tooltip did not open"));return true;
					}
					// Slate may host the tooltip inside the game window. Measure its
					// widget geometry, not the containing 1920x1080 viewport.
					const auto Size=Hover->GetToolTip()->GetCachedGeometry().GetLocalSize();
					Test->TestTrue(TEXT("Calculation tooltip stays compact"),Size.X>0 && Size.Y>0 && Size.X<600 && Size.Y<600);
					TArray<FColor> Pixels;FIntVector ImageSize;
					if(App.TakeScreenshot(Window->GetContent(),Pixels,ImageSize))
					{
						TArray64<uint8> PNG;FImageUtils::PNGCompressImageArray(ImageSize.X,ImageSize.Y,Pixels,PNG);
						FFileHelper::SaveArrayToFile(PNG,*(FPaths::ProjectSavedDir()/TEXT("Stage8_30H/PIE/CutInside_FormulaTooltip.png")));
					}
					App.SetCursorPos(FVector2D(10,10));App.UpdateToolTip(true);bCutTipChecked=true;
				}
			}
			Test->TestTrue(TEXT("Actual DirectShot consumer owns Theater"),Shown(TEXT("ResolutionTheater")));
			if (bDead)
			{
				Capture(TEXT("DeadCorner_Entry.png"));
				if (!Override(*C,EFMCodexLocalDevRollTarget::LongShotDeadCornerA,5) || !Override(*C,EFMCodexLocalDevRollTarget::LongShotDeadCornerB,5)) return true;
			}
			else
			{
			if (Step==6) Test->TestTrue(TEXT("Attack completed natural motion and hold before defense"),bAttackMotion && bAttackLanded);
			const auto Target=bCut?(Step==5?EFMCodexLocalDevRollTarget::CutInsideShotDirectAttack:EFMCodexLocalDevRollTarget::CutInsideShotDirectDefense)
				:(Step==5?EFMCodexLocalDevRollTarget::LongShotDirectAttack:EFMCodexLocalDevRollTarget::LongShotDirectDefense);
			if (!Override(*C,Target,Step==5?(bMiss?2:4):3)) return true;
			if (bMiss && !bCut)
			{
				AttackCardSize=S->GetWidgetFromName(TEXT("TheaterAttackPanelBounds"))->GetCachedGeometry().GetLocalSize();
				AttackFigureSize=S->GetWidgetFromName(TEXT("TheaterAttackSilhouetteBounds"))->GetCachedGeometry().GetLocalSize();
				Test->TestTrue(TEXT("Real entry geometry is valid"),AttackCardSize.X>0 && AttackFigureSize.Y>0);
				Capture(TEXT("DirectShot_Entry.png"));
			}
			}
			CastChecked<UButton>(S->GetWidgetFromName(TEXT("TheaterContinue")))->OnClicked.Broadcast();
			if (!Accepted(*C)) return true;Next();return false;
		}
		if (Step==7)
		{
			if (CountdownMode != 0)
			{
				if(bCut && ResultSeen < 0. && S->GetTacticalScene().Phase!=FMCodexTacticalScene::EPhase::ResultHold)
				{
					if(!bLoggedResultBoundary)
					{
						bLoggedResultBoundary=true;
						Test->AddInfo(FString::Printf(TEXT("CUT_RESULT_BOUNDARY phase=%d active=%d cut=%d contest=%s outcome=%d"),
							int32(S->GetTacticalScene().Phase),S->GetTacticalScene().Facts.bActive,S->GetTacticalScene().Facts.bCutInside,
							*S->GetPresentation().LongShotResolution.Formula.ContestId.ToString(),int32(S->GetTacticalScene().Facts.Outcome)));
					}
					return false;
				}
				if (S->GetTacticalScene().IsAnimating() || S->GetTacticalScene().Celebration.IsActive())
				{
					Test->TestFalse(TEXT("Required scene/celebration never consumes countdown"),Text(TEXT("TheaterContinueLabel")).Contains(TEXT("（")));
					return false;
				}
				const double Now = GEditor->PlayWorld->GetTimeSeconds();
				if (ResultSeen < 0.)
				{
					ResultSeen = Now; TerminalSequence = S->GetPresentation().Header.AttackSequence;
					if(bCut)
					{
						Test->TestTrue(TEXT("Both existing manual dice revealed naturally"),bAttackMotion && bDefenseMotion && bAttackLanded && bDefenseLanded);
						Test->TestTrue(TEXT("Completed CutInside shows accepted broadcast result plate"),Shown(TEXT("CrossResultPlate")));
						Capture(TEXT("CutInside_Result.png"));
						Test->AddInfo(TEXT("CUT_INSIDE_PIE natural_entry=1 idle_motion=1 manual_attack=4 manual_defense=3 result_handoff=1"));
					}
				}
				const FString Label = Text(TEXT("TheaterContinueLabel"));
				if (Label == TEXT("下一回合（5）") && !bSawFive)
				{
					bSawFive = true; CountdownSeen = Now;
					Test->TestTrue(TEXT("Natural result has 2.5 seconds reading time"),Now-ResultSeen >= 2.4);
					Test->TestFalse(TEXT("Narrative closes before the visible countdown"),Shown(TEXT("TheaterOutcome")));
				}
				if (Label == TEXT("下一回合（4）")) bSawFour = true;
				if (Label == TEXT("下一回合（3）")) bSawThree = true;
				if (Label == TEXT("下一回合（2）"))
				{
					bSawTwo = true;
					if (CountdownMode == 2)
					{
						CastChecked<UButton>(S->GetWidgetFromName(TEXT("TheaterContinue")))->OnClicked.Broadcast();
						if (!Accepted(*C)) return true;
						Test->TestTrue(TEXT("Manual click advances immediately"),S->GetPresentation().Header.AttackSequence == TerminalSequence+1);
						Next(); return false;
					}
				}
				if (Label == TEXT("下一回合（1）")) bSawOne = true;
				if (S->GetPresentation().Header.AttackSequence == TerminalSequence) return false;
				Test->TestTrue(TEXT("Auto continuation followed visible 5 / 4 / 3 / 2 / 1"),bSawFive && bSawFour && bSawThree && bSawTwo && bSawOne);
				Test->TestTrue(TEXT("Natural countdown lasts five active seconds"),Now-CountdownSeen >= 4.95);
				Next(); return false;
			}
			Test->TestTrue(TEXT("Both real motion paths and holds observed"),bAttackMotion && bDefenseMotion && bAttackLanded && bDefenseLanded);
			Test->TestTrue(TEXT("Canonical terminal owns existing Outcome and CTA"),C->GetInteractionView().bTerminalPendingAdvance && Shown(TEXT("TheaterOutcome")) && Shown(TEXT("TheaterPrimaryBounds")));
			Test->TestFalse(TEXT("Full Card remains denied during resolution"),S->IsDetailOverlayVisible());
			Capture(TEXT("LongShot_Resolved.png"));
			Test->AddInfo(FString::Printf(TEXT("DIRECT_SHOT_PIE carrier=%s attack=4 defense=3 motion=v2 natural_hold=1 terminal=1"),*Carrier.ToString()));
			CastChecked<UButton>(S->GetWidgetFromName(TEXT("TheaterContinue")))->OnClicked.Broadcast();
			if (!Accepted(*C)) return true;Next();return false;
		}
		if (CountdownMode != 0)
		{
			if (!bRecoveryChecked)
			{
				bRecoveryChecked = true;
				Test->TestTrue(TEXT("Natural continuation exposes its Recovery notification"),
					S->GetPresentation().Resolution.bNonBlockingNotification && S->IsLegacyResolutionOverlayVisible());
				auto* Rows = CastChecked<UPanelWidget>(S->GetResolutionPanel()->GetWidgetFromName(TEXT("RecoverySummaryRows")));
				Test->TestTrue(TEXT("Natural Recovery lists the selected players"), Rows->GetChildrenCount() > 0);
				for (const UWidget* W = Rows; W; W = W->GetParent())
					Test->TestTrue(TEXT("Actual Recovery player rows have no hidden parent"), W->IsVisible());
			}
			if (FPlatformTime::Seconds()-Changed < 3.5) return false;
			Test->TestEqual(TEXT("One continuation only after former timer expires"),S->GetPresentation().Header.AttackSequence,TerminalSequence+1);
			Test->AddInfo(FString::Printf(TEXT("NEXT_ROUND_PIE mode=%s sequence=%lld -> %lld natural_time=1 duplicate=0"),
				CountdownMode==1?TEXT("auto"):TEXT("manual"),TerminalSequence,S->GetPresentation().Header.AttackSequence));
		}
		Test->TestFalse(TEXT("Existing Advance clears prior shot"),S->GetPresentation().LongShotResolution.bVisible);
		CheckTacticalButton(*S);
		if (bDead) Capture(TEXT("PreTP_NextPlayer.png"));
		return true;
	}
private:
	void CheckTacticalButton(UFMCodexLocalMatchScreenWidget& S)
	{
		if (!S.GetPresentation().Interaction.bCanRollTacticalPoints) return;
		auto* Panel=S.GetInteractionPanel();
		auto* Button=Panel->GetWidgetFromName(TEXT("InteractionTacticalPointRollButton"));
		auto* Label=Panel->GetWidgetFromName(TEXT("InteractionTacticalPointRollButtonLabel"));
		const auto& BG=Button->GetCachedGeometry(); const auto& LG=Label->GetCachedGeometry();
		const auto Min=BG.AbsoluteToLocal(LG.LocalToAbsolute(FVector2D::ZeroVector));
		const auto Max=BG.AbsoluteToLocal(LG.LocalToAbsolute(LG.GetLocalSize()));
		Test->TestTrue(TEXT("Actual acting-side TP label fits its button"),Min.X>=0 && Min.Y>=0 && Max.X<=BG.GetLocalSize().X+1 && Max.Y<=BG.GetLocalSize().Y+1);
	}
	bool Accepted(AFMCodexLocalMatchPlayerController& C) {return Test->TestTrue(TEXT("Existing Screen intent accepted"),C.GetLastDiagnostic().bHostSuccess);}
	bool Override(AFMCodexLocalMatchPlayerController& C,EFMCodexLocalDevRollTarget Target,int32 Value)
	{FFMCodexLocalDevRollOverrideRequest R;R.Target=Target;R.Value=Value;return Test->TestTrue(TEXT("Host-owned DEV provider override"),C.SetLocalDevRollOverride(R).bSuccess);}
	void Capture(const TCHAR* File)
	{
		if (CountdownMode != 0 && !bCut) return;
		if (bCut && !FString(File).StartsWith(TEXT("CutInside_"))) return;
		auto Window=GEditor->PlayWorld->GetGameViewport()->GetWindow();
		if (!Test->TestTrue(TEXT("Real PIE viewport exists"),Window.IsValid())) return;
		TArray<FColor> Pixels; FIntVector Size;
		if (!Test->TestTrue(TEXT("Real PIE frame captured"),FSlateApplication::Get().TakeScreenshot(Window->GetContent(),Pixels,Size))) return;
		const FString Dir=FPaths::ProjectSavedDir()/(bCut?TEXT("Stage8_30H/PIE"):bDead || bMiss?TEXT("Stage8_18E/PIE"):TEXT("Stage8_18B/PIE"));IFileManager::Get().MakeDirectory(*Dir,true);
		TArray64<uint8> PNG;FImageUtils::PNGCompressImageArray(Size.X,Size.Y,Pixels,PNG);
		Test->TestTrue(TEXT("PIE evidence saved"),FFileHelper::SaveArrayToFile(PNG,*(Dir/File)));
	}
	bool bCutTipChecked=false;double CutTipStarted=0;
	FAutomationTestBase* Test;double Started=0,Changed=0;int32 Step=0;FName Carrier;
	bool bDead=false,bMiss=false,bCut=false,bCornerLayout=false;
	double IdleSeen=-1.;FVector2D IdleCarrier=FVector2D::ZeroVector;
	int32 CountdownMode=0; int64 TerminalSequence=0;
	double ResultSeen=-1.,CountdownSeen=0.; bool bSawFive=false,bSawFour=false,bSawThree=false,bSawTwo=false,bSawOne=false;
	bool bRecoveryChecked=false,bLoggedResultBoundary=false;
	bool bAttackMotion=false,bDefenseMotion=false,bAttackLanded=false,bDefenseLanded=false;
	bool bMissTransitionCaptured=false;
	FVector2D AttackCardSize,AttackFigureSize;
};
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexNextRoundPIETest,"FMCodex.PIE.NextRoundCountdown",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexNextRoundPIETest::RunTest(const FString&)
{
	FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShareable(new FStartDirectShotPIE()));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(2.f));
	FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShareable(new FDirectShotPIE(this,false,false,1)));
	FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShareable(new FDirectShotPIE(this,false,false,2)));
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand()); return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexCutInsideBroadcastPIETest,"FMCodex.PIE.CutInsideBroadcast",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexCutInsideBroadcastPIETest::RunTest(const FString&)
{
	FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShareable(new FStartDirectShotPIE()));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(2.f));
	FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShareable(new FDirectShotPIE(this,false,false,1,true)));
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexCutInsideCornerLayoutPIETest,"FMCodex.PIE.CutInsideCornerLayout",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexCutInsideCornerLayoutPIETest::RunTest(const FString&)
{
	FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShareable(new FStartDirectShotPIE()));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(2.f));
	FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShareable(new FDirectShotPIE(this,false,false,0,true,true)));
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexCutInsideVisualRefinementPIETest,"FMCodex.PIE.CutInsideVisualRefinement",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexCutInsideVisualRefinementPIETest::RunTest(const FString&)
{
	// One Local PIE session: inspect the paired layout, then play a direct miss.
	FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShareable(new FStartDirectShotPIE()));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(2.f));
	FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShareable(new FDirectShotPIE(this,false,false,0,true,true)));
	FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShareable(new FDirectShotPIE(this,false,true,0,true)));
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexDirectShotPIETest,"FMCodex.PIE.DirectShotTheater.LongShot",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexDirectShotPIETest::RunTest(const FString&)
{
	FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShareable(new FStartDirectShotPIE()));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(2.f));
	FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShareable(new FDirectShotPIE(this)));
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());return true;
}
IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFMCodexShotCompletionPIETest,"FMCodex.PIE.ShotTheater.Completion",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
void FFMCodexShotCompletionPIETest::GetTests(TArray<FString>& Names,TArray<FString>& Commands) const
{ for (const TCHAR* Mode:{TEXT("DeadCorner"),TEXT("ImmediateMiss")}) {Names.Add(Mode);Commands.Add(Mode);} }
bool FFMCodexShotCompletionPIETest::RunTest(const FString& Mode)
{
	FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShareable(new FStartDirectShotPIE()));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(2.f));
	FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShareable(new FDirectShotPIE(this,Mode==TEXT("DeadCorner"),Mode==TEXT("ImmediateMiss"))));
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());return true;
}
#endif
