#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && !UE_BUILD_SHIPPING
#include "FMCodexLocalMatchPlayerController.h"
#include "FMCodexLocalMatchScreenWidget.h"
#include "FMCodexLongShotResolutionSurfaceWidget.h"
#include "FMCodexRollReelWidget.h"
#include "FMCodexCardRackWidget.h"
#include "FMCodexPlayerCardWidget.h"
#include "FMCodexInteractionPanelWidget.h"
#include "FMCodexOutcomePresentation.h"
#include "Components/Button.h"
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
	explicit FDirectShotPIE(FAutomationTestBase* InTest, bool bInDead=false, bool bInMiss=false)
		: Test(InTest), bDead(bInDead), bMiss(bInMiss) {}
	bool Update() override
	{
		if (FPlatformTime::Seconds()-Started>120.) {Test->AddError(FString::Printf(TEXT("Shot PIE timed out at step %d"),Step));return true;}
		if (!GEditor || !GEditor->PlayWorld) return false;
		auto* C=Cast<AFMCodexLocalMatchPlayerController>(GEditor->PlayWorld->GetFirstPlayerController());
		auto* S=C?C->GetPlayerMatchScreen():nullptr;
		if (!S || FPlatformTime::Seconds()-Changed<.25) return false;
		auto Next=[&](){++Step;Changed=FPlatformTime::Seconds();};
		auto Shown=[S](const TCHAR* Name){auto* W=S->GetWidgetFromName(Name);return W && W->GetVisibility()!=ESlateVisibility::Collapsed && W->GetVisibility()!=ESlateVisibility::Hidden;};
		auto Text=[S](const TCHAR* Name){return CastChecked<UTextBlock>(S->GetWidgetFromName(Name))->GetText().ToString();};
		if (bMiss && Step==6)
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
							if (!bAttackMotion) Capture(TEXT("ImmediateMiss_AttackRolling.png"));
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
					if (bMiss) Test->TestEqual(TEXT("Every naturally landed frame has current status"),Text(TEXT("TheaterDetail")),FString(TEXT("进攻方掷点已落定")));
					if (Step==6) bAttackLanded=true; else bDefenseLanded=true;
				}
			}
			return false;
		}
		if (Step==0) {S->RequestStartNewMatch();Next();return false;}
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
			Carrier=IsA?TEXT("Prototype.Arsenal.MartinOdegaard"):TEXT("Prototype.ManchesterCity.PhilFoden");
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
				const auto* Choice=S->GetPresentation().Interaction.SelectionChoices.FindByPredicate([](const auto& O){return O.SkillType==ESkillRuleType::LongShot && O.bEnabled;});
				if (!Test->TestNotNull(TEXT("Reached genuine ordinary LongShot Tactical Choice"),Choice)) return true;
				S->RequestSubmitSkill(Choice->OptionId);if (!Accepted(*C)) return true;Next();return false;
			}
			default:Test->AddError(TEXT("Unexpected ordinary selection state"));return true;
			}
			if (!Accepted(*C)) return true;Changed=FPlatformTime::Seconds();return false;
		}
		if (Step==4)
		{
			if (FPlatformTime::Seconds()-Changed<1.) return false;
			Test->TestTrue(TEXT("Actual shot choice now stays in Theater"),Shown(TEXT("ResolutionTheater")) && Shown(TEXT("TheaterNearMethods")));
			Test->TestTrue(TEXT("Actual branch has a named Carrier context"),Shown(TEXT("TheaterDuel")) && !Text(TEXT("TheaterAttackName0")).IsEmpty() && Text(TEXT("TheaterAttackRole0"))==TEXT("持球"));
			Test->TestEqual(TEXT("Actual branch uses the concise direct summary"),Text(TEXT("TheaterNearDirectHint")),FString(TEXT("持球：远射\n对抗盯人：抢断\n进攻掷点 1–2：射门偏出")));
			Test->TestEqual(TEXT("Actual pair choice keeps only the success condition"),Text(TEXT("TheaterNearCombinationHint")),FString(TEXT("进攻方依次掷两枚骰子\n总和 11–12：进球")));
			if (bDead) Capture(TEXT("Shot_BranchChoice.png"));
			CastChecked<UButton>(S->GetWidgetFromName(bDead?TEXT("TheaterNearCombination"):TEXT("TheaterNearDirect")))->OnClicked.Broadcast();
			if (!Accepted(*C)) return true;Next();return false;
		}
		if ((bDead || bMiss) && Step==6)
		{
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
			Test->AddInfo(bDead?TEXT("SHOT_COMPLETION_PIE DeadCorner pair=5,5 miss_reason=1 rolling_hint=1 natural_hold=1 terminal=1"):TEXT("SHOT_COMPLETION_PIE ImmediateMiss attack=2 stale_status=0 defense_rolls=0 natural_hold=1 terminal=1"));
			CastChecked<UButton>(S->GetWidgetFromName(TEXT("TheaterContinue")))->OnClicked.Broadcast();
			if (!Accepted(*C)) return true;Step=8;Changed=FPlatformTime::Seconds();return false;
		}
		if (Step==5 || Step==6)
		{
			if (FPlatformTime::Seconds()-Changed<1.) return false;
			Test->TestTrue(TEXT("Actual DirectShot consumer owns Theater"),Shown(TEXT("ResolutionTheater")));
			if (bDead)
			{
				Capture(TEXT("DeadCorner_Entry.png"));
				if (!Override(*C,EFMCodexLocalDevRollTarget::LongShotDeadCornerA,5) || !Override(*C,EFMCodexLocalDevRollTarget::LongShotDeadCornerB,5)) return true;
			}
			else
			{
			if (Step==6) Test->TestTrue(TEXT("Attack completed natural motion and hold before defense"),bAttackMotion && bAttackLanded);
			if (!Override(*C,Step==5?EFMCodexLocalDevRollTarget::LongShotDirectAttack:EFMCodexLocalDevRollTarget::LongShotDirectDefense,Step==5?(bMiss?2:4):3)) return true;
			if (bMiss)
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
			Test->TestTrue(TEXT("Both real motion paths and holds observed"),bAttackMotion && bDefenseMotion && bAttackLanded && bDefenseLanded);
			Test->TestTrue(TEXT("Canonical terminal owns existing Outcome and CTA"),C->GetInteractionView().bTerminalPendingAdvance && Shown(TEXT("TheaterOutcome")) && Shown(TEXT("TheaterPrimaryBounds")));
			Test->TestFalse(TEXT("Full Card remains denied during resolution"),S->IsDetailOverlayVisible());
			Capture(TEXT("LongShot_Resolved.png"));
			Test->AddInfo(FString::Printf(TEXT("DIRECT_SHOT_PIE carrier=%s attack=4 defense=3 motion=v2 natural_hold=1 terminal=1"),*Carrier.ToString()));
			CastChecked<UButton>(S->GetWidgetFromName(TEXT("TheaterContinue")))->OnClicked.Broadcast();
			if (!Accepted(*C)) return true;Next();return false;
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
		auto Window=GEditor->PlayWorld->GetGameViewport()->GetWindow();
		if (!Test->TestTrue(TEXT("Real PIE viewport exists"),Window.IsValid())) return;
		TArray<FColor> Pixels; FIntVector Size;
		if (!Test->TestTrue(TEXT("Real PIE frame captured"),FSlateApplication::Get().TakeScreenshot(Window->GetContent(),Pixels,Size))) return;
		const FString Dir=FPaths::ProjectSavedDir()/(bDead || bMiss?TEXT("Stage8_18E/PIE"):TEXT("Stage8_18B/PIE"));IFileManager::Get().MakeDirectory(*Dir,true);
		TArray64<uint8> PNG;FImageUtils::PNGCompressImageArray(Size.X,Size.Y,Pixels,PNG);
		Test->TestTrue(TEXT("PIE evidence saved"),FFileHelper::SaveArrayToFile(PNG,*(Dir/File)));
	}
	FAutomationTestBase* Test;double Started=FPlatformTime::Seconds(),Changed=0;int32 Step=0;FName Carrier;
	bool bDead=false,bMiss=false;
	bool bAttackMotion=false,bDefenseMotion=false,bAttackLanded=false,bDefenseLanded=false;
	bool bMissTransitionCaptured=false;
	FVector2D AttackCardSize,AttackFigureSize;
};
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
