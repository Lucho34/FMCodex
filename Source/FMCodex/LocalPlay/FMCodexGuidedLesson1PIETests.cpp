#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && !UE_BUILD_SHIPPING
#include "FMCodexGuidedLesson1.h"
#include "FMCodexGuidedLesson1Focus.h"
#include "FMCodexPitchSlotWidget.h"
#include "FMCodexInteractionPanelWidget.h"
#include "FMCodexLocalMatchPlayerController.h"
#include "FMCodexLocalMatchScreenWidget.h"
#include "FMCodexCardRackWidget.h"
#include "FMCodexPlayerCardWidget.h"
#include "Components/Button.h"
#include "Components/Border.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Editor.h"
#include "Engine/World.h"
#include "Engine/GameViewportClient.h"
#include "Framework/Application/SlateApplication.h"
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
			.ClientSize(FVector2D(1600,900)).AutoCenter(EAutoCenter::None).ScreenPosition(FVector2D(0,0))
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
			if(auto* Status=Normal->GetInteractionPanel()->GetWidgetFromName(TEXT("InteractionBoundedFallback")))
				T->TestEqual(TEXT("Normal dock status restored after tutorial polish"),Status->GetRenderOpacity(),1.f);
			const auto& Cards=Normal->GetLocalRackWidget()->GetRenderedCardWidgets();
			for (auto Card:Cards) if (Card->CanExposeFullCardDetail()) { Pointer(Center(Card)); break; }
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
		case Step::InspectGyokeres:
		{
			const auto Targets=FMCodexLesson1Focus::Targets(S,*L);
			if (!T->TestEqual(TEXT("One semantic hover target"),Targets.Num(),1)) return true;
			T->TestFalse(TEXT("Cannot drag before inspection"),S->GetLocalRackWidget()->GetRenderedCardWidgets()[0]->IsDeploymentDragEnabled());
			Pointer(Center(Targets[0]));
			T->TestTrue(TEXT("Real pointer opens Full Card"),S->IsDetailOverlayVisible());
			break;
		}
		case Step::SkillRangeExplanation:
			T->TestTrue(TEXT("Full Card retained beside skill explanation"),S->IsDetailOverlayVisible());
			Capture(TEXT("06_SkillRangeInspection.png"));
			C->GuidedLesson1Primary(); break;
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
		case Step::FinishDeployment: FocusClick(S,*L); break;
		case Step::Carrier: FocusClick(S,*L); break;
		case Step::Skill:
			T->TestTrue(TEXT("Canonical Runner absence"),C->GetInteractionView().SelectedRunnerCardId.IsNone());
			FocusClick(S,*L); break;
		case Step::DirectShot:
			if(auto* Other=Cast<UButton>(S->GetWidgetFromName(TEXT("TheaterNearCombination"))))
				T->TestFalse(TEXT("Other method visibly disabled"),Other->GetIsEnabled());
			if(!L->IsComparison()) Capture(TEXT("08_DirectShotFocus.png")); FocusClick(S,*L); break;
		case Step::AttackRoll: FocusClick(S,*L); break;
		case Step::Rewind:
			Capture(TEXT("02_FirstMissAndRewind.png"));
			CheckTooltip(S,TEXT("TheaterDefenseBaseHover"),TEXT("+3"));
			T->TestEqual(TEXT("First miss 0-0"),C->GetInteractionView().PlayerAScore,0);
			C->GuidedLesson1Primary(); break;
		case Step::Summary:
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
		auto Window=GEditor->PlayWorld->GetGameViewport()->GetWindow();
		TArray<FColor> Pixels; FIntVector Size = FIntVector::ZeroValue;
		if(!Window || !FSlateApplication::Get().TakeScreenshot(Window->GetContent(),Pixels,Size)) { T->AddError(TEXT("PIE capture unavailable")); return; }
		const FString Dir=FPaths::ProjectSavedDir()/TEXT("Stage8_21A/PolishPIE"); IFileManager::Get().MakeDirectory(*Dir,true);
		TArray64<uint8> PNG; FImageUtils::PNGCompressImageArray(Size.X,Size.Y,Pixels,PNG);
		T->TestTrue(TEXT("Actual lesson window captured"),FFileHelper::SaveArrayToFile(PNG,*(Dir/Name)));
	}
	FAutomationTestBase* T;
	double Started=FPlatformTime::Seconds(),Changed=0;
	bool bStarted=false,bCompared=false,bLastComparison=false,bSummaryInspected=false,bExited=false;
	int32 DragPhase=0;
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
