#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && !UE_BUILD_SHIPPING
#include "FMCodexLocalMatchPlayerController.h"
#include "FMCodexLocalMatchScreenWidget.h"
#include "FMCodexThroughBallResolutionSurfaceWidget.h"
#include "FMCodexInlineResolutionFormulaSurfaceWidget.h"
#include "FMCodexRollReelWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
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
class FStartThroughBallCompactPIE final : public IAutomationLatentCommand
{
public:
	bool Update() override
	{
		auto* Settings = DuplicateObject<ULevelEditorPlaySettings>(GetDefault<ULevelEditorPlaySettings>(), GetTransientPackage());
		Settings->NewWindowWidth = 1600; Settings->NewWindowHeight = 900;
		Settings->SetPlayNetMode(EPlayNetMode::PIE_Standalone); Settings->SetPlayNumberOfClients(1);
		FRequestPlaySessionParams Params; Params.EditorPlaySettings = Settings;
		GEditor->RequestPlaySession(Params); return true;
	}
};

// Two real Local PIE paths through typed Screen intents and the host-owned DEV
// RNG seam. No raw state writes, forced outcomes or presentation-clock stepping.
class FThroughBallFullPIE final : public IAutomationLatentCommand
{
public:
	FThroughBallFullPIE(FAutomationTestBase* InTest,bool InBehind,bool InPolish=false,bool InBinary=false,bool InBinaryFailure=false) : Test(InTest),bBehind(InBehind),bPolish(InPolish),bBinary(InBinary),bBinaryFailure(InBinaryFailure) {}
	bool Update() override
	{
		if (FPlatformTime::Seconds()-Started>150.)
		{ Test->AddError(FString::Printf(TEXT("ThroughBall PIE timed out at %d"),Step)); return true; }
		if (!GEditor || !GEditor->PlayWorld) return false;
		auto* C=Cast<AFMCodexLocalMatchPlayerController>(GEditor->PlayWorld->GetFirstPlayerController());
		auto* S=C?C->GetPlayerMatchScreen():nullptr;
		if (!S) return false;
		const float Now=GEditor->PlayWorld->GetTimeSeconds();
		if (S->IsInlineFormulaRevealInputBlocked())
		{
			if (Step<4) return false;
			CheckOuter(*S);
			const auto& P=S->GetThroughBallResolutionSurface()->GetPresentation();
			const bool bRoute=Event==0;
			const bool bIndependent=bRoute || bBinary || (bPolish && ((!bBehind && Event==1) || (bBehind && Event==3)));
			const bool bAttack=Event==1 || Event==3;
			auto* Reel=CastChecked<UFMCodexRollReelWidget>(S->GetWidgetFromName(bIndependent?TEXT("TheaterEventReel"):bAttack?TEXT("TheaterAttackReel"):TEXT("TheaterDefenseReel")));
			Test->TestEqual(TEXT("Live event uses mapped production Roll family"),Reel->GetVisualVariant(),bIndependent?EFMCodexRollVisualVariant::CompactBox:EFMCodexRollVisualVariant::TheaterInline);
			Test->TestTrue(TEXT("No future CTA or Full Card during live reveal"),S->GetWidgetFromName(TEXT("TheaterPrimaryBounds"))->GetVisibility()==ESlateVisibility::Collapsed && !S->IsDetailOverlayVisible());
			if (S->GetInlineFormulaRevealPhase()==EFMCodexUMGInlineFormulaRevealPhase::Settling && Now-EventStart<1.30f) Modern[Event]=true;
			if (bPolish && bIndependent && !bBinary)
			{
				Test->TestTrue(TEXT("Route Anti Chip die stays in its lane through motion and hold"),S->GetWidgetFromName(TEXT("TheaterEventCell"))->GetCachedGeometry().GetAbsolutePosition().Equals(IndependentAnchor,1.f));
				const auto& Card=S->GetWidgetFromName(TEXT("TheaterAttackPanelBounds"))->GetCachedGeometry();
				Test->TestTrue(TEXT("Context card stays compact and stationary across roll phases"),Card.GetAbsolutePosition().Equals(ContextAnchor,1.f) && Card.GetLocalSize().Equals(ContextSize,1.f));
				if (!bBehind && bRoute && !MotionCaptured && Now-EventStart>.55f)
				{ Capture(TEXT("A_RouteRolling.png")); MotionCaptured=true; }
			}
			if (Reel->GetPresentation().bAuthoritativeValue && !Landed[Event])
			{
				Landed[Event]=true; HoldStart[Event]=Now;
				const int32 Expected=bRoute?(bBehind?4:bPolish?5:2):bBinary?(Event==1?(bBinaryFailure?4:3):6):bPolish?(!bBehind?2:Event==2?1:6):Event==1?6:Event==4?6:1;
				Test->TestEqual(TEXT("Live provider operand lands naturally"),Reel->GetPresentation().CenterValue,Expected);
				Test->TestEqual(TEXT("Authoritative glyph uses EED7A6"),Reel->GetCenterDigitWidget()->GetColorAndOpacity().GetSpecifiedColor(),FLinearColor::FromSRGBColor(FColor(238,215,166)));
				const auto Phase=S->GetInlineFormulaRevealPhase(); C->RefreshPresentation();
				Test->TestEqual(TEXT("Repeated live View preserves current hold"),S->GetInlineFormulaRevealPhase(),Phase);
			}
			if (bBinary && Event==1 && Landed[Event])
			{
				auto* Second=CastChecked<UFMCodexRollReelWidget>(S->GetWidgetFromName(TEXT("TheaterEventSecondReel")));
				Test->TestEqual(TEXT("Second authoritative six shares the event hold"),Second->GetPresentation().CenterValue,bBinaryFailure?3:6);
				Test->TestTrue(TEXT("Pair uses shared CompactBox and simultaneous hold"),Second->GetVisualVariant()==EFMCodexRollVisualVariant::CompactBox && Second->GetPresentation().bAuthoritativeValue);
				Test->TestTrue(TEXT("Trait attribution and any-six rule visible"),Text(*S,TEXT("TheaterDetail")).Contains(TEXT("反越位专家")));
				Test->TestFalse(TEXT("Pair has no fake Formula"),P.Formula.bVisible);
			}
			if (bPolish && !bBehind && Event==1 && Landed[Event] && !AntiLandedCaptured && Now-HoldStart[Event]>.1f)
			{ Capture(TEXT("B_AntiLanded.png")); AntiLandedCaptured=true; }
			if (bPolish && bBehind && Event==2 && P.Formula.bNarrativeAvailable && NarrativeSeen<0)
			{ NarrativeSeen=Now; Capture(TEXT("C_BehindIntermediate.png")); }
			if (Event==2 && !bBehind && !bBinary)
				Test->TestFalse(TEXT("Feet Goal score remains gated until outcome disclosure"),Text(*S,TEXT("TheaterContext")).Contains(TEXT("1")));
			return false;
		}
		using Category=EFMCodexLocalMatchInteractionCategory;
		if (Step==0)
		{
			// All earlier random events are targeted overrides. Behind defense is
			// the first unconsumed value of the existing deterministic test provider.
			int32 Seed=0; for (;Seed<10000;++Seed) { FRandomStream Probe(Seed); if (Probe.RandRange(1,6)==(bBinaryFailure?3:bBinary?6:1)) break; }
			C->SetNextDemoMatchSeedForTesting(Seed); S->RequestStartNewMatch();
			Test->AddInfo(FString::Printf(TEXT("THROUGHBALL_FULL_PIE path=%s seed=%d"),bBinary?TEXT("BinaryAntiOffside"):bPolish?(bBehind?TEXT("PolishBehindChip"):TEXT("PolishAntiOffside")):(bBehind?TEXT("BehindDirect"):TEXT("Feet")),Seed));
			++Step; return false;
		}
		if (Step==1)
		{
			if (!Override(*C,EFMCodexLocalDevRollTarget::FullD12,5)) return true;
			S->RequestRollTacticalPoints(); ++Step; return false;
		}
		if (Step == 2)
		{
			if (!Test->TestEqual(TEXT("Natural TP5 reveal reaches deployment"), C->GetInteractionView().InteractionCategory, Category::Deploy)) return true;
			const bool IsA = C->GetInteractionView().CurrentAttackingPlayer == EInitialTurnOrderPlayer::PlayerA;
			Carrier = IsA ? TEXT("Prototype.Arsenal.MartinOdegaard") : TEXT("Prototype.ManchesterCity.Rodri");
			Runner = bBinary ? TEXT("Prototype.Arsenal.MikelMerino") : IsA ? TEXT("Prototype.Arsenal.BukayoSaka") : TEXT("Prototype.ManchesterCity.ErlingHaaland");
			const FString Own = IsA ? TEXT("NearA") : TEXT("NearB"), Forward = IsA ? TEXT("NearB") : TEXT("NearA");
			// Deployment alternates after every card. One participant per physical
			// area on each side is sufficient for the ordinary role-selection path.
			for (int32 I = 0; I < 2; ++I)
			{
				for (int32 Side = 0; Side < 2; ++Side)
				{
					const FName Required = Side == 0 ? (I == 0 ? Carrier : Runner) : NAME_None;
					const FString Half = I == 0 ? Own : Forward;
					const auto* O = C->GetInteractionView().DeploymentOptions.FindByPredicate([&](const auto& Candidate)
					{ return !Candidate.bGoalkeeper && Candidate.SlotId.ToString().Contains(Half)
						&& (Required.IsNone() ? Candidate.CardId != Carrier && Candidate.CardId != Runner : Candidate.CardId == Required); });
					if (!Test->TestNotNull(*FString::Printf(TEXT("Canonical deployment candidate: side=%d area=%s card=%s"), Side, *Half, *Required.ToString()), O)) return true;
					const FName Card = O->CardId, Slot = O->SlotId;
					S->RequestDeployOrdinary(Card, Slot); if (!Accepted(*C)) return true;
				}
			}
			S->RequestFinishDeployment(); if (!Accepted(*C)) return true;
			S->RequestFinishDeployment(); if (!Accepted(*C)) return true;
			++Step; return false;
		}
		if (Step == 3)
		{
			const auto& V = C->GetInteractionView();
			switch (V.InteractionCategory)
			{
			case Category::SelectCarrier: S->RequestSubmitCarrier(Carrier); break;
			case Category::SelectMarker:
				if (!Test->TestFalse(TEXT("Canonical Marker exists"), V.SelectionOptions.IsEmpty())) return true;
				S->RequestSubmitMarker(V.SelectionOptions[0].Id); break;
			case Category::SelectRunner: S->RequestSubmitRunner(Runner); break;
			case Category::SelectHelper:
				if (V.bCanResolveNoLegalChoice) S->RequestResolveNoLegalSelection(); else S->RequestDeclineSelection(); break;
			case Category::SelectSkill:
			{
				const auto* Choice = V.SelectionOptions.FindByPredicate([](const auto& O) { return O.SkillType == ESkillRuleType::ThroughBall; });
				if (!Test->TestNotNull(TEXT("Canonical ThroughBall choice at TP5"), Choice)) return true;
				S->RequestSubmitSkill(Choice->Id); break;
			}
			case Category::RollThroughBallInitialRoute:
				if (!Ready(Now,1.1f)) return false;
				Test->TestEqual(TEXT("Route safely shows Carrier and Runner"),VisiblePeople(*S),2);
				Test->TestEqual(TEXT("Route owns concise reference"),Text(*S,TEXT("TheaterDetail")),FString(TEXT("1–2：脚下球　｜　3–4：身后球　｜　5–6：反越位")));
				if (!bBehind) Capture(TEXT("A_RouteContext.png"));
				if (bPolish)
				{
					Test->TestNull(TEXT("No large independent rule chamber"),S->GetWidgetFromName(TEXT("TheaterEventRule")));
					RememberIndependentLayout(*S);
				}
				if (!Override(*C, EFMCodexLocalDevRollTarget::ThroughBallRoute, bBehind?4:bPolish?5:2)) return true;
				Click(*S); EventStart=Now; Event=0; ++Step; PreSeen=-1; break;
			default: Test->AddError(TEXT("Unexpected ThroughBall setup state")); return true;
			}
			if (!Accepted(*C)) return true;
			return false;
		}

		CheckOuter(*S);
		if (bPolish && Step==4 && !bBehind)
		{
			if (!Ready(Now,.3f)) return false;
			if (!Test->TestEqual(TEXT("Route naturally reaches Anti"),C->GetInteractionView().InteractionCategory,Category::RollThroughBallAntiOffsideAttack)) return true;
			Test->TestEqual(TEXT("Anti keeps passer first"),Text(*S,TEXT("TheaterAttackRole0")),FString(TEXT("传球")));
			Test->TestEqual(TEXT("Anti keeps Runner second"),Text(*S,TEXT("TheaterAttackRole1")),FString(TEXT("跑位")));
			Test->TestTrue(TEXT("Anti lower bar owns the rule without route history"),Text(*S,TEXT("TheaterDetail")).Contains(TEXT("6：形成单刀")) && !Text(*S,TEXT("TheaterDetail")).Contains(TEXT("路线掷点")));
			Capture(TEXT("B_Anti.png"));
			RememberIndependentLayout(*S);
			if (!Override(*C,EFMCodexLocalDevRollTarget::ThroughBallAntiOffside,bBinaryFailure?4:bBinary?3:2)) return true;
			Click(*S); Event=1; EventStart=Now; Step=9; PreSeen=-1; return !Accepted(*C);
		}
		if (Step==4)
		{
			if (!Ready(Now,.3f)) return false;
			if (!Test->TestEqual(TEXT("Route reaches correct first Formula attack"),C->GetInteractionView().InteractionCategory,
				bBehind?Category::RollThroughBallBehindDefenseAttack:Category::RollThroughBallFeetAttack)) return true;
			Test->TestTrue(TEXT("Route completed modern motion and readable hold"),Modern[0] && Landed[0] && Now-HoldStart[0]>=1.35f);
			CheckFormula(*S);
			if (!bBehind) Capture(TEXT("B_FeetFormula.png"));
			if (!Override(*C,bBehind?EFMCodexLocalDevRollTarget::ThroughBallBehindDefenseP1:EFMCodexLocalDevRollTarget::ThroughBallFeetAttack,6)) return true;
			Click(*S); Event=1; EventStart=Now; ++Step; PreSeen=-1; return !Accepted(*C);
		}
		if (Step==5)
		{
			if (!Ready(Now,.3f)) return false;
			if (!Test->TestEqual(TEXT("Real defense follows first Formula attack"),C->GetInteractionView().InteractionCategory,
				bBehind?Category::RollThroughBallBehindDefenseDefense:Category::RollThroughBallFeetDefense)) return true;
			Test->TestEqual(TEXT("Attack six remains painted while defense is pending"),Text(*S,TEXT("TheaterAttackRollValue")),FString(TEXT("6")));
			if (!bBehind && !Override(*C,EFMCodexLocalDevRollTarget::ThroughBallFeetDefense,1)) return true;
			if (bBehind && !bPolish) Capture(TEXT("C_BehindDefense.png"));
			Click(*S); Event=2; EventStart=Now; ++Step; PreSeen=-1; return !Accepted(*C);
		}
		if (Step==6 && bBehind)
		{
			if (bPolish && ChoiceSeen<0) ChoiceSeen=Now;
			if (!Ready(Now,.3f)) return false;
			if (!Test->TestEqual(TEXT("Behind attack win naturally creates OneOnOne"),C->GetInteractionView().InteractionCategory,Category::SelectOneOnOneShot)) return true;
			Test->TestTrue(TEXT("Modern choice has Runner, two methods and no next-turn CTA"),
				!Text(*S,TEXT("TheaterAttackName0")).IsEmpty()
				&& CastChecked<UButton>(S->GetWidgetFromName(TEXT("TheaterNearDirect")))->GetIsEnabled()
				&& CastChecked<UButton>(S->GetWidgetFromName(TEXT("TheaterNearCombination")))->GetIsEnabled()
				&& S->GetWidgetFromName(TEXT("TheaterPrimaryBounds"))->GetVisibility()==ESlateVisibility::Collapsed);
			if (bPolish)
			{
				Test->TestTrue(TEXT("Behind narrative remains readable for about 3.1 seconds"),NarrativeSeen>=0 && ChoiceSeen-NarrativeSeen>=3.05f && ChoiceSeen-NarrativeSeen<=3.25f);
				Test->AddInfo(FString::Printf(TEXT("POLISH_INTERMEDIATE readableSeconds=%.3f"),ChoiceSeen-NarrativeSeen));
			}
			if (!bPolish) Capture(TEXT("D_OneOnOneChoice.png"));
			CastChecked<UButton>(S->GetWidgetFromName(bPolish?TEXT("TheaterNearCombination"):TEXT("TheaterNearDirect")))->OnClicked.Broadcast();
			++Step; PreSeen=-1; return !Accepted(*C);
		}
		if (Step==6) Step=9;
		if (bPolish && Step==7)
		{
			if (!Ready(Now,.3f)) return false;
			if (!Test->TestEqual(TEXT("Method click submits actual Chip intent"),C->GetInteractionView().InteractionCategory,Category::RollThroughBallOneOnOneChipShotAttack)) return true;
			Test->TestEqual(TEXT("Chip has one relevant Runner"),VisiblePeople(*S),1);
			Test->TestTrue(TEXT("Chip rule is in the lower bar"),Text(*S,TEXT("TheaterDetail")).Contains(TEXT("4–6：进球")));
			Capture(TEXT("E_Chip.png"));
			RememberIndependentLayout(*S);
			if (!Override(*C,EFMCodexLocalDevRollTarget::OneOnOneChipShotAttack,6)) return true;
			Click(*S); Event=3; EventStart=Now; Step=9; PreSeen=-1; return !Accepted(*C);
		}
		if (Step==7)
		{
			if (!Ready(Now,.3f)) return false;
			if (!Test->TestEqual(TEXT("Method click submits actual DirectShot intent"),C->GetInteractionView().InteractionCategory,Category::RollThroughBallOneOnOneDirectShotAttack)) return true;
			CheckFormula(*S);
			Test->TestEqual(TEXT("Direct has one canonical GK opponent"),S->GetThroughBallResolutionSurface()->GetPresentation().Formula.DefenseRow.Participants.Num(),1);
			if (!Override(*C,EFMCodexLocalDevRollTarget::OneOnOneDirectShotAttack,1)) return true;
			Click(*S); Event=3; EventStart=Now; ++Step; PreSeen=-1; return !Accepted(*C);
		}
		if (Step==8)
		{
			if (!Ready(Now,.3f)) return false;
			if (!Test->TestEqual(TEXT("Direct attack one still requires a new defense roll"),C->GetInteractionView().InteractionCategory,Category::RollThroughBallOneOnOneDirectShotDefense)) return true;
			Test->TestEqual(TEXT("New Direct attack replaces the prior Behind six"),Text(*S,TEXT("TheaterAttackRollValue")),FString(TEXT("1")));
			CheckFormula(*S); Capture(TEXT("E_DirectLowAttack.png"));
			if (!Override(*C,EFMCodexLocalDevRollTarget::OneOnOneDirectShotDefense,6)) return true;
			Click(*S); Event=4; EventStart=Now; ++Step; PreSeen=-1; return !Accepted(*C);
		}
		if (bBinary && !bBinaryFailure && Step==9 && !bBinaryShotSubmitted)
		{
			if (!Ready(Now,.3f)) return false;
			if (!Test->TestEqual(TEXT("Second die six naturally enters the existing OneOnOne choice"),C->GetInteractionView().InteractionCategory,Category::SelectOneOnOneShot)) return true;
			Test->TestTrue(TEXT("Both authoritative dice persisted in one event"),C->GetInteractionView().ResolutionFacts.Rolls.ContainsByPredicate([](const auto& R) { return R.RawD6==3 && R.AntiOffsideSecondD6==6; }));
			Capture(TEXT("C_BinaryOneOnOneChoice.png"));
			CastChecked<UButton>(S->GetWidgetFromName(TEXT("TheaterNearCombination")))->OnClicked.Broadcast();
			Step=11; PreSeen=-1; bBinaryShotSubmitted=true; return !Accepted(*C);
		}
		if (bBinary && Step==11)
		{
			if (!Ready(Now,.3f)) return false;
			if (!Test->TestEqual(TEXT("Normal Chip CTA follows Binary success"),C->GetInteractionView().InteractionCategory,Category::RollThroughBallOneOnOneChipShotAttack)) return true;
			if (!Override(*C,EFMCodexLocalDevRollTarget::OneOnOneChipShotAttack,6)) return true;
			Click(*S); Event=2; EventStart=Now; Step=9; PreSeen=-1; return !Accepted(*C);
		}
		if (Step==9)
		{
			if (!Ready(Now,.3f)) return false;
			if (!Test->TestEqual(TEXT("Natural terminal exposes typed advance"),C->GetInteractionView().InteractionCategory,Category::AdvanceAfterTerminal)) return true;
			Test->TestTrue(TEXT("Shared Narrative and reason stay inside Theater"),
				!CastChecked<URichTextBlock>(S->GetWidgetFromName(TEXT("TheaterOutcome")))->GetText().IsEmpty()
				&& (bPolish || !CastChecked<URichTextBlock>(S->GetWidgetFromName(TEXT("TheaterReasonPrimary")))->GetText().IsEmpty()));
			for (int32 I=0;I<(bBinary?(bBinaryFailure?2:3):bPolish?(bBehind?4:2):(bBehind?5:3));++I) Test->TestTrue(TEXT("Each event had its own visible motion and landing"),Modern[I] && Landed[I]);
			C->RefreshPresentation(); Test->TestFalse(TEXT("Completed live terminal cannot replay"),S->IsInlineFormulaRevealInputBlocked());
			if (bPolish)
			{
				Test->TestEqual(TEXT("Sparse result retains relevant context only"),VisiblePeople(*S),(bBehind || (bBinary && !bBinaryFailure))?1:2);
				Test->TestTrue(TEXT("Sparse result has no unrelated defense"),S->GetWidgetFromName(TEXT("TheaterDefensePanelBounds"))->GetVisibility()==ESlateVisibility::Collapsed);
			}
			if (bPolish && !bBehind && (!bBinary || bBinaryFailure))
			{
				const FString Markup = CastChecked<URichTextBlock>(S->GetWidgetFromName(TEXT("TheaterReasonPrimary")))->GetText().ToString();
				Test->TestEqual(TEXT("Live D6 label remains plain and operands retain Value style"), Markup,
					FString(bBinary ? TEXT("反越位专家 · D6：<Value>4</>、<Value>3</>") : TEXT("D6：<Value>2</>")));
				if (!bBinary) Capture(TEXT("D_NormalD6Label.png"));
			}
			if (bBinaryFailure)
			{
				Test->TestEqual(TEXT("Real pair failure displays exact approved reason"),Text(*S,TEXT("TheaterReasonSecondary")),FString(TEXT("两次判定均未掷出 6，因此越位。")));
				const auto Disclosure=CastChecked<URichTextBlock>(S->GetWidgetFromName(TEXT("TheaterReasonPrimary")))->GetText().ToString();
				Test->TestTrue(TEXT("Both dice and Trait attribution remain disclosed"),Disclosure.Contains(TEXT("反越位专家")) && Disclosure.Contains(TEXT("4")) && Disclosure.Contains(TEXT("3")));
				Capture(TEXT("D_BinaryFailureReason.png"));
			}
			if (!bPolish) Capture(bBehind?TEXT("F_DirectTerminal.png"):TEXT("G_FeetTerminal.png"));
			Test->AddInfo(FString::Printf(TEXT("THROUGHBALL_FULL_PIE path=%s events=%d terminal=%s score=%s"),bBinary?TEXT("BinaryAntiOffside"):bPolish?(bBehind?TEXT("PolishBehindChip"):TEXT("PolishAntiOffside")):(bBehind?TEXT("BehindDirect"):TEXT("Feet")),bBinary?(bBinaryFailure?2:3):bPolish?(bBehind?4:2):(bBehind?5:3),
				*(bPolish?S->GetThroughBallResolutionSurface()->GetPresentation().ResultTitle:S->GetThroughBallResolutionSurface()->GetPresentation().Formula.ResultTitle),*Text(*S,TEXT("TheaterContext"))));
			Click(*S); ++Step; PreSeen=-1; return !Accepted(*C);
		}
		if (!Ready(Now,.6f)) return false;
		Test->TestFalse(TEXT("Advance naturally leaves ThroughBall"),S->GetThroughBallResolutionSurface()->GetPresentation().bVisible);
		Test->AddInfo(FString::Printf(TEXT("THROUGHBALL_FULL_PIE nextAttack=%lld"),C->GetInteractionView().AttackSequence));
		return true;
	}
private:
	void RememberIndependentLayout(UFMCodexLocalMatchScreenWidget& S)
	{
		const auto& Die=S.GetWidgetFromName(TEXT("TheaterEventCell"))->GetCachedGeometry();
		const auto& Card=S.GetWidgetFromName(TEXT("TheaterAttackPanelBounds"))->GetCachedGeometry();
		const auto& Bar=S.GetWidgetFromName(TEXT("TheaterInfoBar"))->GetCachedGeometry();
		const auto& Lane=S.GetWidgetFromName(TEXT("TheaterActionBounds"))->GetCachedGeometry();
		IndependentAnchor=Die.GetAbsolutePosition(); ContextAnchor=Card.GetAbsolutePosition(); ContextSize=Card.GetLocalSize();
		Test->TestNull(TEXT("Context has no stray dice icon"),S.GetWidgetFromName(TEXT("TheaterEventPending")));
		Test->TestTrue(TEXT("Independent die is below the info bar in the established roll lane"),
			IndependentAnchor.Y>=Bar.LocalToAbsolute(Bar.GetLocalSize()).Y
			&& Die.LocalToAbsolute(Die.GetLocalSize()*.5f).Equals(Lane.LocalToAbsolute(Lane.GetLocalSize()*.5f),1.f));
		Test->TestTrue(TEXT("Identity-only card reuses the tight Theater context height"),ContextSize.Y<=180.f);
	}
	bool Ready(float Now,float Delay) { if (PreSeen<0) PreSeen=Now; return Now-PreSeen>=Delay; }
	bool Accepted(AFMCodexLocalMatchPlayerController& C)
	{ return Test->TestTrue(*C.GetLastDiagnostic().Message,C.GetLastDiagnostic().bHostSuccess); }
	bool Override(AFMCodexLocalMatchPlayerController& C,EFMCodexLocalDevRollTarget Target,int32 Value)
	{
		FFMCodexLocalDevRollOverrideRequest R; R.Target=Target; R.Value=Value;
		return Test->TestTrue(TEXT("Host DEV provider accepts override"),C.SetLocalDevRollOverride(R).bSuccess);
	}
	FString Text(UFMCodexLocalMatchScreenWidget& S,const TCHAR* Name)
	{ return CastChecked<UTextBlock>(S.GetWidgetFromName(Name))->GetText().ToString(); }
	int32 VisiblePeople(UFMCodexLocalMatchScreenWidget& S)
	{
		int32 Count=0;
		for (auto* Person:CastChecked<UHorizontalBox>(S.GetWidgetFromName(TEXT("TheaterAttackPeople")))->GetAllChildren())
			if (Person->GetVisibility()!=ESlateVisibility::Collapsed) ++Count;
		return Count;
	}
	void Click(UFMCodexLocalMatchScreenWidget& S)
	{
		auto* Button=CastChecked<UButton>(S.GetWidgetFromName(TEXT("TheaterContinue")));
		Test->TestTrue(TEXT("Theater primary is the enabled typed action"),Button->GetIsEnabled()); Button->OnClicked.Broadcast();
	}
	void CheckOuter(UFMCodexLocalMatchScreenWidget& S)
	{
		if (Step>=10) return;
		Test->TestEqual(TEXT("Live resolution never returns to old ThroughBall outer modal"),S.GetThroughBallResolutionSurface()->GetVisibility(),ESlateVisibility::Collapsed);
		Test->TestTrue(TEXT("Live Theater retains title and denies Full Card"),Text(S,TEXT("TheaterTitle"))==TEXT("直塞") && !S.IsDetailOverlayVisible());
	}
	void CheckFormula(UFMCodexLocalMatchScreenWidget& S)
	{
		Test->TestTrue(TEXT("Real opposed Formula keeps both projected rows"),S.GetThroughBallResolutionSurface()->GetPresentation().Formula.bShowFormulaRows
			&& S.GetWidgetFromName(TEXT("TheaterDefensePanelBounds"))->GetVisibility()!=ESlateVisibility::Collapsed);
		Test->TestEqual(TEXT("Actual Formula attack reel is TheaterInline"),CastChecked<UFMCodexRollReelWidget>(S.GetWidgetFromName(TEXT("TheaterAttackReel")))->GetVisualVariant(),EFMCodexRollVisualVariant::TheaterInline);
	}
	void Capture(const TCHAR* Name)
	{
		auto Window=GEditor->PlayWorld->GetGameViewport()->GetWindow();
		if (!Window.IsValid()) { Test->AddError(TEXT("No real PIE window")); return; }
		TArray<FColor> Pixels; FIntVector Size=FIntVector::ZeroValue;
		if (!FSlateApplication::Get().TakeScreenshot(Window.ToSharedRef(),Pixels,Size)) { Test->AddError(TEXT("PIE capture failed")); return; }
		const FString Dir=FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()/(bBinaryFailure?TEXT("Stage8_20B3/MicrocopyPIE"):bBinary?TEXT("Stage8_20B3/PIE"):bPolish?TEXT("Stage8_19C/AnchorPolish/PIE"):TEXT("Stage8_19C/PIE")));
		IFileManager::Get().MakeDirectory(*Dir,true);
		TArray64<uint8> PNG; FImageUtils::PNGCompressImageArray(Size.X,Size.Y,Pixels,PNG);
		Test->TestTrue(TEXT("Live PIE evidence saved"),FFileHelper::SaveArrayToFile(PNG,*(Dir/Name)));
	}
	FAutomationTestBase* Test;
	bool bBehind, bPolish, bBinary, bBinaryFailure;
	bool bBinaryShotSubmitted=false;
	float NarrativeSeen=-1, ChoiceSeen=-1;
	FVector2D IndependentAnchor=FVector2D::ZeroVector, ContextAnchor=FVector2D::ZeroVector, ContextSize=FVector2D::ZeroVector;
	bool MotionCaptured=false, AntiLandedCaptured=false;
	double Started=FPlatformTime::Seconds();
	int32 Step=0, Event=0;
	float EventStart=0, PreSeen=-1, HoldStart[5]={};
	bool Modern[5]={}, Landed[5]={};
	FName Carrier,Runner;
};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexThroughBallFeetFullPIETest,
	"FMCodex.PIE.ThroughBall.FullFlow.Feet",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexThroughBallFeetFullPIETest::RunTest(const FString&)
{
	FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShareable(new FStartThroughBallCompactPIE()));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(2.f));
	FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShareable(new FThroughBallFullPIE(this,false)));
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand()); return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexThroughBallBehindDirectFullPIETest,
	"FMCodex.PIE.ThroughBall.FullFlow.BehindDirect",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexThroughBallBehindDirectFullPIETest::RunTest(const FString&)
{
	FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShareable(new FStartThroughBallCompactPIE()));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(2.f));
	FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShareable(new FThroughBallFullPIE(this,true)));
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand()); return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexThroughBallPolishAntiOffsidePIETest,
    "FMCodex.PIE.ThroughBall.Polish.AntiOffside",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexThroughBallPolishAntiOffsidePIETest::RunTest(const FString&)
{
    FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShareable(new FStartThroughBallCompactPIE()));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(2.f));
    FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShareable(new FThroughBallFullPIE(this,false,true)));
    ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand()); return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexThroughBallPolishBehindChipPIETest,
    "FMCodex.PIE.ThroughBall.Polish.BehindChip",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexThroughBallPolishBehindChipPIETest::RunTest(const FString&)
{
    FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShareable(new FStartThroughBallCompactPIE()));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(2.f));
    FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShareable(new FThroughBallFullPIE(this,true,true)));
    ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand()); return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAntiOffsideBinaryPIE,
    "FMCodex.PIE.ThroughBall.BinaryTrait",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FAntiOffsideBinaryPIE::RunTest(const FString&)
{
    FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShareable(new FStartThroughBallCompactPIE()));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(2.f));
    FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShareable(new FThroughBallFullPIE(this,false,true,true)));
    ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand()); return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAntiOffsideBinaryFailureReasonPIE,
    "FMCodex.PIE.ThroughBall.BinaryFailureReason",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FAntiOffsideBinaryFailureReasonPIE::RunTest(const FString&)
{
    FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShareable(new FStartThroughBallCompactPIE()));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(2.f));
    FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShareable(new FThroughBallFullPIE(this,false,true,true,true)));
    ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand()); return true;
}

#endif
