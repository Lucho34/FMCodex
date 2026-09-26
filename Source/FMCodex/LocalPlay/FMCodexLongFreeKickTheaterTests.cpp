#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && !UE_BUILD_SHIPPING
#include "../NetworkPlay/FMCodexNetworkLongFreeKickTestFixture.h"
#include "FMCodexResolutionTheaterPrototype.h"
#include "FMCodexCardRackWidget.h"
#include "FMCodexPlayerCardWidget.h"
#include "FMCodexMatchFlowPanel.h"
#include "Components/Button.h"
#include "Components/Border.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/RichTextBlock.h"
#include "HAL/IConsoleManager.h"

namespace
{
using namespace FMCodexLongFreeKickTests;
using namespace FMCodexPlayerFacingOrdinaryUITests;
bool Shown(UFMCodexLocalMatchScreenWidget* S,const TCHAR* Name)
{
	auto* W=S->GetWidgetFromName(Name);
	return W && W->GetVisibility()!=ESlateVisibility::Collapsed && W->GetVisibility()!=ESlateVisibility::Hidden;
}
FString Text(UFMCodexLocalMatchScreenWidget* S,const TCHAR* Name)
{
	return CastChecked<UTextBlock>(S->GetWidgetFromName(Name))->GetText().ToString();
}
FString Tip(UFMCodexLocalMatchScreenWidget* S,const TCHAR* Name)
{
	auto* Hover=CastChecked<UBorder>(S->GetWidgetFromName(Name));
	return CastChecked<UTextBlock>(CastChecked<USizeBox>(CastChecked<UBorder>(Hover->GetToolTip())->GetContent())->GetContent())->GetText().ToString();
}
void Click(UFMCodexLocalMatchScreenWidget* S,const TCHAR* Name)
{
	CastChecked<UButton>(S->GetWidgetFromName(Name))->OnClicked.Broadcast();
}
}

IMPLEMENT_COMPLEX_AUTOMATION_TEST(FLongTheaterLifecycle,"FMCodex.LocalPlay.ResolutionTheater.LongFreeKick.Lifecycle",
	EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
void FLongTheaterLifecycle::GetTests(TArray<FString>& N,TArray<FString>& C) const
{
 for (const TCHAR* Side:{TEXT("A"),TEXT("B")}) for (const TCHAR* Method:{TEXT("Direct"),TEXT("Early"),TEXT("Power"),TEXT("PowerMiss")})
 {const FString P=FString(Side)+TEXT(".")+Method; N.Add(P); C.Add(P);}
}
bool FLongTheaterLifecycle::RunTest(const FString& P)
{
	const bool Early=P.Contains(TEXT("Early")), Miss=P.Contains(TEXT("Miss"));
	const bool PairMethod=P.Contains(TEXT("Power"));
	FUIFixture F(P.StartsWith(TEXT("B"))); Access::SetPiecePresentation(*F.Mode,false);
	auto* Actor=F.Attacker(); auto* Other=F.Defender();
	auto* S=Actor->GetPlayerMatchScreen(); auto* W=Other->GetPlayerMatchScreen();
	F.Entropy->Word=8; S->RequestRollTacticalPoints(); F.Settle();
	TestFalse(TEXT("Unchanged SetPiece Type has no Theater"),Shown(S,TEXT("ResolutionTheater")));
	F.Entropy->Word=2; S->DevSetPieceAction(TEXT("SetPieceType"),NAME_None);
	for (auto* Screen:{S,W})
	{
		Screen->PauseInlineFormulaRevealTimerForTesting();
		TestFalse(TEXT("Future Long does not bypass Type reveal"),Shown(Screen,TEXT("ResolutionTheater")));
		Screen->AdvanceInlineFormulaRevealForTesting(.3f);
		TestFalse(TEXT("Type cycling still owns board"),Shown(Screen,TEXT("ResolutionTheater")));
	}
	F.Settle();
	for (auto* Screen:{S,W}) TestTrue(TEXT("Both viewers enter after Type hold"),Shown(Screen,TEXT("ResolutionTheater")));
	TestEqual(TEXT("Long header"),Text(S,TEXT("TheaterTitle")),FString(TEXT("远距离任意球")));
	const auto* Rack=CastChecked<UFMCodexCardRackWidget>(S->GetWidgetFromName(TEXT("TheaterTakers")));
	TestEqual(TEXT("Every authority-eligible card is inside Theater"),Rack->GetPresentation().Cells.Num(),S->GetPresentation().SetPiece.TakerOptions.Num());
	TestFalse(TEXT("Waiting viewer has no candidate CTA"),Shown(W,TEXT("TheaterTakerBounds")));
	TestFalse(TEXT("No confirm before draft"),Shown(S,TEXT("TheaterPrimaryBounds")));
	const FName Taker=P.StartsWith(TEXT("B"))?FName(TEXT("Prototype.ManchesterCity.PhilFoden")):FName(TEXT("Prototype.Arsenal.EberechiEze"));
	CastChecked<UFMCodexCardRackWidget>(S->GetWidgetFromName(TEXT("TheaterTakers")))->OnCardSelectionRequested.Broadcast(Taker);
	const int32 Sends=F.Backend(Actor).Sends;
	TestTrue(TEXT("Selection is a local draft"),Actor->GetOwnerView().SetPiece.TakerCardId.IsNone());
	TestEqual(TEXT("Same selection state owns confirm"),Text(S,TEXT("TheaterContinueLabel")),FString(TEXT("确认主罚球员")));
	TestTrue(TEXT("Existing Card Selection family marks draft"),CastChecked<UFMCodexCardRackWidget>(S->GetWidgetFromName(TEXT("TheaterTakers")))->GetPresentation().Cells.ContainsByPredicate(
		[Taker](const auto& Cell){return Cell.Card.CardId==Taker && Cell.bSetPieceSelected;}));
 auto* Draft=Rack->GetRenderedCardWidgets().FindByPredicate([Taker](const auto& V){return V->GetPresentation().CardId==Taker;})->Get();
 TestTrue(TEXT("Long selection uses the Full Card hover callback"),Draft->RequestFullCardDetailHover());
 TestTrue(TEXT("Full Card is limited to selection"),Shown(S,TEXT("TheaterTakerFullCard")));
 TestTrue(TEXT("Selected subtitle owns player identity"),Text(S,TEXT("TheaterSubtitle")).StartsWith(TEXT("已选主罚球员：")));
 TestTrue(TEXT("Selection explains Power threshold without fabricated eligibility"),Text(S,TEXT("TheaterReasonSecondary")).Contains(TEXT("≥ 11")));
	Click(S,TEXT("TheaterContinue")); TestEqual(TEXT("Exactly one confirmation intent"),F.Backend(Actor).Sends,Sends+1);
	TestTrue(TEXT("Method choice stays in Theater"),Shown(S,TEXT("TheaterNearMethods")));
 TestFalse(TEXT("Method choice clears Full Card inspection"),Shown(S,TEXT("TheaterTakerInspector")));
	const FString TakerName=Actor->GetOwnerView().SetPiece.TakerLabel.ToString();
	for (auto* Screen:{S,W}) TestEqual(TEXT("Known taker continuous at method"),Text(Screen,TEXT("TheaterAttackName0")),TakerName);
	Click(S,PairMethod?TEXT("TheaterNearCombination"):TEXT("TheaterNearDirect"));
	if (!TestEqual(TEXT("Method uses the existing typed adapter"),F.Backend(Actor).LastCode,Code::Accepted)) return false;
	for (auto* Screen:{S,W})
	{
		TestEqual(TEXT("Only Direct shows defender"),Shown(Screen,TEXT("TheaterDefensePanelBounds")),!PairMethod);
		TestEqual(TEXT("Pair has no invented opponent"),Screen->GetPresentation().InlineFormula.DefenseRow.Participants.Num(),PairMethod?0:1);
		TestEqual(TEXT("Taker continuous into resolution"),Text(Screen,TEXT("TheaterAttackName0")),TakerName);
	}
 // The existing inspector and copy hierarchy are shared, with no invented Power prerequisite.
 TestFalse(TEXT("Full Card clears at method execution"),Shown(S,TEXT("TheaterTakerInspector")));
 TestEqual(TEXT("Canonical alternative name"),Text(S,TEXT("TheaterNearCombinationLabel")),FString(TEXT("重炮轰门")));
 if (!PairMethod)
 {
  TestTrue(TEXT("Direct tooltip explains LongShot"),Tip(S,TEXT("TheaterAttackBaseHover")).Contains(TEXT("远射")));
  TestTrue(TEXT("Direct tooltip preserves fixed plus two"),Tip(S,TEXT("TheaterDefenseBaseHover")).Contains(TEXT("防守加成 2")));
 }
	const auto Before=S->GetPresentation().Header;
	auto RollAndObserve=[&](AFMCodexNetworkMatchPlayerController* PC,Kind K,int32 A,int32 B)
	{
		const FString SettledAttackTotal=Text(S,TEXT("TheaterAttackFinalNumber"));
		F.Entropy->PendingWords={uint32(A-1)}; if (K==Pair) F.Entropy->PendingWords.Add(uint32(B-1));
		Click(PC->GetPlayerMatchScreen(),TEXT("TheaterContinue"));
		if (!TestEqual(TEXT("Theater roll submits existing intent"),F.Backend(PC).LastCode,Code::Accepted)) return false;
		for (auto* Screen:{S,W})
		{
			Screen->PauseInlineFormulaRevealTimerForTesting();
			bool First=false,Second=false;
			for (int32 I=0;I<100 && Screen->IsInlineFormulaRevealInputBlocked();++I)
			{
				const auto& D=Screen->GetInlineFormulaSurface()->GetPresentation();
				TestFalse(TEXT("No CTA while a die is revealing"),Shown(Screen,TEXT("TheaterPrimaryBounds")));
				if (!Shown(Screen,TEXT("TheaterOutcome")))
				{
					TestEqual(TEXT("No early score A"),Screen->GetMatchHeader()->GetPresentation().PlayerAScoreLabel,Before.PlayerAScoreLabel);
					TestEqual(TEXT("No early score B"),Screen->GetMatchHeader()->GetPresentation().PlayerBScoreLabel,Before.PlayerBScoreLabel);
				}
				if (K==Attack || K==Defense)
				{
					const bool AttackRolling=K==Attack;
					TestEqual(TEXT("Only reveal owner has a visible reel"),Shown(Screen,TEXT("TheaterAttackReelHost")),AttackRolling);
					TestEqual(TEXT("Defense reel follows owner, not shared sequence zero"),Shown(Screen,TEXT("TheaterDefenseReelHost")),!AttackRolling);
					TestEqual(TEXT("Current attack emphasis matches reveal owner"),Shown(Screen,TEXT("TheaterAttackBadge")),AttackRolling);
					TestEqual(TEXT("Current defense emphasis matches reveal owner"),Shown(Screen,TEXT("TheaterDefenseBadge")),!AttackRolling);
					if (!AttackRolling)
					{
						TestTrue(TEXT("Landed attack operand stays visible throughout defense"),Shown(Screen,TEXT("TheaterAttackRollValue")));
						TestEqual(TEXT("Accepted attack die never cycles again"),Text(Screen,TEXT("TheaterAttackRollValue")),FString(TEXT("6")));
						TestEqual(TEXT("Attack value remains final during defense"),Text(Screen,TEXT("TheaterAttackValueLabel")),FString(TEXT("最终值")));
						TestEqual(TEXT("Attack total remains the disclosed projection"),Text(Screen,TEXT("TheaterAttackFinalNumber")),SettledAttackTotal);
					}
				}
				if (K==Pair)
				{
					First|=D.ActiveRollSequenceIndex==0; Second|=D.ActiveRollSequenceIndex==1;
					if (D.ActiveRollSequenceIndex==0)
					{
						TestFalse(TEXT("First die cannot disclose second operand"),D.AttackRow.Terms[1].bResolved);
						TestEqual(TEXT("First die cannot disclose sum"),Text(Screen,TEXT("TheaterPairTotal")),FString(TEXT("?")));
					}
					if (D.ActiveRollSequenceIndex==1) TestTrue(TEXT("First operand retained while B rolls"),D.AttackRow.Terms[0].bResolved);
				}
				Screen->AdvanceInlineFormulaRevealForTesting(.25f);
			}
			if (K==Pair) TestTrue(TEXT("One command preserves sequential A/B reveal"),First && Second);
			TestFalse(TEXT("Reveal terminates"),Screen->IsInlineFormulaRevealInputBlocked());
		}
		return true;
	};
	if (!RollAndObserve(Actor,PairMethod?Pair:Attack,Early?1:6,Miss?1:5)) return false;
	if (!PairMethod && !Early && !RollAndObserve(Other,Defense,1,1)) return false;
	for (auto* Screen:{S,W})
	{
		TestTrue(TEXT("Final remains Theater"),Shown(Screen,TEXT("ResolutionTheater")));
		TestTrue(TEXT("Outcome family is visible"),Shown(Screen,TEXT("TheaterOutcome")));
		TestTrue(TEXT("Canonical reason is present"),!Screen->GetPresentation().InlineFormula.ResolutionReasonLabel.IsEmpty());
		TestEqual(TEXT("Known taker continuous through outcome"),Text(Screen,TEXT("TheaterAttackName0")),TakerName);
	}
	if (PairMethod) TestEqual(TEXT("Pair sum comes from authority"),Text(S,TEXT("TheaterPairTotal")),FString(Miss?TEXT("7"):TEXT("11")));
	else if (!Early) TestTrue(TEXT("Direct uses canonical special-rule reason"),S->GetPresentation().InlineFormula.ResolutionReasonLabel.Contains(TEXT("快速压制")));
 if (Early)
 {
  TestFalse(TEXT("Early miss has no fake defending contest"),Shown(S,TEXT("TheaterDefensePanelBounds")));
  TestTrue(TEXT("Early miss explains the special rule"),S->GetPresentation().InlineFormula.ResolutionReasonLabel.Contains(TEXT("不进行攻防比较")));
  TestEqual(TEXT("Early terminal consumes type and attack only"),Actor->GetOwnerView().Presentation.ResolvedRolls.Num(),2);
 }
 TestEqual(TEXT("Canonical outcome, never inferred by Theater"),Actor->GetOwnerView().Terminal.Outcome,Early || Miss?Outcome::NoGoal:Outcome::Goal);
	Click(S,TEXT("TheaterContinue"));
	for (auto* Screen:{S,W}) TestFalse(TEXT("Next clears Theater and returns board"),Shown(Screen,TEXT("ResolutionTheater")));
	return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLongTheaterScope,"FMCodex.LocalPlay.ResolutionTheater.LongFreeKick.Scope",
 EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FLongTheaterScope::RunTest(const FString&)
{
 using namespace FMCodexResolutionTheaterPrototype;
 auto* Mode=IConsoleManager::Get().FindConsoleVariable(TEXT("fm.UI.ResolutionStageV2.LongFreeKick"));
 TestEqual(TEXT("Long Development defaults ON"),Mode->GetInt(),1);
 FFMCodexUMGMatchScreenViewModel P; P.SetPiece.bVisible=true; P.SetPiece.Type=Type::LongFreeKick;
 TestTrue(TEXT("Long selection enters"),WantsTheater(P,{}));
 Mode->Set(0,ECVF_SetByCode); TestFalse(TEXT("Long fallback retains old path"),WantsTheater(P,{})); Mode->Set(1,ECVF_SetByCode);
 for (auto T:{Type::Penalty,Type::Corner}) {P.SetPiece.Type=T;TestFalse(TEXT("Unmigrated routes remain excluded"),WantsTheater(P,{}));}
 return true;
}
#endif
