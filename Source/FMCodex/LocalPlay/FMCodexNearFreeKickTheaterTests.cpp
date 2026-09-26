#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && !UE_BUILD_SHIPPING
#include "../NetworkPlay/FMCodexNetworkNearFreeKickTestFixture.h"
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
#include "../CoreRules/MatchPlayShortFreeKickResolution.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/MemoryWriter.h"

namespace
{
using namespace FMCodexNearFreeKickTests;
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

IMPLEMENT_COMPLEX_AUTOMATION_TEST(FNearTheaterLifecycle,"FMCodex.LocalPlay.ResolutionTheater.NearFreeKick.Lifecycle",
	EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
void FNearTheaterLifecycle::GetTests(TArray<FString>& N,TArray<FString>& C) const
{
	for (const TCHAR* Side:{TEXT("A"),TEXT("B")}) for (const TCHAR* Method:{TEXT("Direct"),TEXT("Combination")})
	{ const FString P=FString(Side)+TEXT(".")+Method; N.Add(P); C.Add(P); }
}
bool FNearTheaterLifecycle::RunTest(const FString& P)
{
	const bool PairMethod=P.Contains(TEXT("Combination"));
	FUIFixture F(P.StartsWith(TEXT("B"))); Access::SetPiecePresentation(*F.Mode,false);
	auto* Actor=F.Attacker(); auto* Other=F.Defender();
	auto* S=Actor->GetPlayerMatchScreen(); auto* W=Other->GetPlayerMatchScreen();
	F.Entropy->Word=8; S->RequestRollTacticalPoints(); F.Settle();
	TestFalse(TEXT("Unchanged SetPiece Type has no Theater"),Shown(S,TEXT("ResolutionTheater")));
	F.Entropy->Word=4; S->DevSetPieceAction(TEXT("SetPieceType"),NAME_None);
	for (auto* Screen:{S,W})
	{
		Screen->PauseInlineFormulaRevealTimerForTesting();
		TestFalse(TEXT("Future Near does not bypass Type reveal"),Shown(Screen,TEXT("ResolutionTheater")));
		Screen->AdvanceInlineFormulaRevealForTesting(.3f);
		TestFalse(TEXT("Type cycling still owns board"),Shown(Screen,TEXT("ResolutionTheater")));
	}
	F.Settle();
	for (auto* Screen:{S,W}) TestTrue(TEXT("Both viewers enter after Type hold"),Shown(Screen,TEXT("ResolutionTheater")));
	TestEqual(TEXT("Near header"),Text(S,TEXT("TheaterTitle")),FString(TEXT("近距离任意球")));
	const auto* Rack=CastChecked<UFMCodexCardRackWidget>(S->GetWidgetFromName(TEXT("TheaterTakers")));
	TestEqual(TEXT("Every authority-eligible card is inside Theater"),Rack->GetPresentation().Cells.Num(),S->GetPresentation().SetPiece.TakerOptions.Num());
	TestFalse(TEXT("Waiting viewer has no candidate CTA"),Shown(W,TEXT("TheaterTakerBounds")));
	TestFalse(TEXT("No confirm before draft"),Shown(S,TEXT("TheaterPrimaryBounds")));
	const FName Taker=Eligible(F);
	CastChecked<UFMCodexCardRackWidget>(S->GetWidgetFromName(TEXT("TheaterTakers")))->OnCardSelectionRequested.Broadcast(Taker);
	const int32 Sends=F.Backend(Actor).Sends;
	TestTrue(TEXT("Selection is a local draft"),Actor->GetOwnerView().SetPiece.TakerCardId.IsNone());
	TestEqual(TEXT("Same selection state owns confirm"),Text(S,TEXT("TheaterContinueLabel")),FString(TEXT("确认主罚球员")));
	TestTrue(TEXT("Existing Card Selection family marks draft"),CastChecked<UFMCodexCardRackWidget>(S->GetWidgetFromName(TEXT("TheaterTakers")))->GetPresentation().Cells.ContainsByPredicate(
		[Taker](const auto& Cell){return Cell.Card.CardId==Taker && Cell.bSetPieceSelected;}));
	Click(S,TEXT("TheaterContinue")); TestEqual(TEXT("Exactly one confirmation intent"),F.Backend(Actor).Sends,Sends+1);
	TestTrue(TEXT("Method choice stays in Theater"),Shown(S,TEXT("TheaterNearMethods")));
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
	if (!PairMethod)
	{
		FFMCodexLocalMatchViewerDisclosure Disclosure;
		Disclosure.bRevealInitialActionPointRoll=true; Disclosure.bRevealSetPieceTypeRoll=true;
		const auto Safe=FFMCodexLocalMatchInteractionViewBuilder::BuildForViewer(Access::Session(*F.Mode).GetStateSnapshot(),
			Access::CallerRules(*F.Mode),Actor->GetOwnerView().ViewerSide,Disclosure);
		TestTrue(TEXT("Type-hidden projection conceals goalkeeper identity"),Access::Safe(*F.Mode,Actor->GetOwnerView().ViewerSide,true).NearFormulaGoalkeeperCardId.IsNone());
		TestFalse(TEXT("Direct has authority-projected goalkeeper ID"),Safe.NearFormulaGoalkeeperCardId.IsNone());
		TestTrue(TEXT("Base hover preserves max operand explanation"),Tip(S,TEXT("TheaterAttackBaseHover")).Contains(TEXT("取较高")));
		TestTrue(TEXT("Base hover identifies defense modifier"),Tip(S,TEXT("TheaterDefenseBaseHover")).Contains(TEXT("防守加成 1")));
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
	if (!RollAndObserve(Actor,PairMethod?Pair:Attack,6,3)) return false;
	if (!PairMethod && !RollAndObserve(Other,Defense,1,1)) return false;
	for (auto* Screen:{S,W})
	{
		TestTrue(TEXT("Final remains Theater"),Shown(Screen,TEXT("ResolutionTheater")));
		TestTrue(TEXT("Outcome family is visible"),Shown(Screen,TEXT("TheaterOutcome")));
		TestTrue(TEXT("Canonical reason is present"),!Screen->GetPresentation().InlineFormula.ResolutionReasonLabel.IsEmpty());
		TestEqual(TEXT("Known taker continuous through outcome"),Text(Screen,TEXT("TheaterAttackName0")),TakerName);
	}
	if (PairMethod) TestEqual(TEXT("Pair sum comes from authority"),Text(S,TEXT("TheaterPairTotal")),FString(TEXT("9")));
	else TestTrue(TEXT("Direct uses canonical special-rule reason"),S->GetPresentation().InlineFormula.ResolutionReasonLabel.Contains(TEXT("快速压制")));
	Click(S,TEXT("TheaterContinue"));
	for (auto* Screen:{S,W}) TestFalse(TEXT("Next clears Theater and returns board"),Shown(Screen,TEXT("ResolutionTheater")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNearTheaterScope,"FMCodex.LocalPlay.ResolutionTheater.NearFreeKick.ScopeAndAuthority",
	EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FNearTheaterScope::RunTest(const FString&)
{
	using namespace FMCodexResolutionTheaterPrototype;
	auto* Mode=IConsoleManager::Get().FindConsoleVariable(TEXT("fm.UI.ResolutionStageV2.NearFreeKick"));
	TestEqual(TEXT("Development defaults on"),Mode->GetInt(),1);
	FFMCodexUMGMatchScreenViewModel P; P.SetPiece.bVisible=true; P.SetPiece.Type=Type::ShortFreeKick;
	TestTrue(TEXT("Disclosed selection enters"),WantsTheater(P,{}));
	Mode->Set(0,ECVF_SetByCode); TestFalse(TEXT("Practical same-state fallback"),WantsTheater(P,{})); Mode->Set(1,ECVF_SetByCode);
	for (auto T:{Type::Penalty,Type::Corner})
	{ P.SetPiece.Type=T; P.InlineFormula.bVisible=true; P.InlineFormula.ContestId=TEXT("SetPiece.Opposed"); TestFalse(TEXT("Other set pieces retain scope"),WantsTheater(P,P.InlineFormula)); }
	FUIFixture F; F.Entropy->Word=8; F.Attacker()->GetPlayerMatchScreen()->RequestRollTacticalPoints(); F.Settle();
	F.Entropy->Word=4; F.Attacker()->GetPlayerMatchScreen()->DevSetPieceAction(TEXT("SetPieceType"),NAME_None); F.Settle();
	auto* S=F.Attacker()->GetPlayerMatchScreen();
	const FName Weak=Eligible(F,false);
	const int32 Sends=F.Backend(F.Attacker()).Sends, Draws=F.Entropy->Calls;
	Mode->Set(0,ECVF_SetByCode); S->RefreshFromPresentation(S->GetPresentation());
	TestTrue(TEXT("Fallback restores actual legacy selection owner"),Shown(S,TEXT("SetPieceProductionResolutionSurface")));
	Mode->Set(1,ECVF_SetByCode); S->RefreshFromPresentation(S->GetPresentation());
	TestTrue(TEXT("Same-state reentry restores Theater candidates"),Shown(S,TEXT("TheaterTakerBounds")));
	TestEqual(TEXT("Fallback sends no command"),F.Backend(F.Attacker()).Sends,Sends);
	TestEqual(TEXT("Fallback draws no entropy"),F.Entropy->Calls,Draws);
	if (!TestFalse(TEXT("Fixture has a genuinely ineligible combination taker"),Weak.IsNone())) return false;
	S->DevSetPieceAction(TEXT("SetPieceTaker"),Weak); Click(S,TEXT("TheaterContinue"));
	TestFalse(TEXT("Authority excludes Combination"),S->GetPresentation().SetPiece.NearMethods.Contains(EMatchPlayShortFreeKickMethod::Angled));
	TestFalse(TEXT("Choice disabled from projected eligibility"),CastChecked<UButton>(S->GetWidgetFromName(TEXT("TheaterNearCombination")))->GetIsEnabled());
	return true;
}
IMPLEMENT_COMPLEX_AUTOMATION_TEST(FNearTheaterSelectionPolish,"FMCodex.LocalPlay.ResolutionTheater.NearFreeKick.SelectionPolish",
	EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
void FNearTheaterSelectionPolish::GetTests(TArray<FString>& N,TArray<FString>& C) const
{
	for (const TCHAR* Case:{TEXT("Eligible"),TEXT("Ineligible")}) {N.Add(Case);C.Add(Case);}
}
bool FNearTheaterSelectionPolish::RunTest(const FString& Parameters)
{
	const bool Combo=Parameters==TEXT("Eligible");
	FUIFixture F; auto* S=F.Attacker()->GetPlayerMatchScreen();
	F.Entropy->Word=8; S->RequestRollTacticalPoints(); F.Settle();
	F.Entropy->Word=4; S->DevSetPieceAction(TEXT("SetPieceType"),NAME_None); F.Settle();
	auto* Rack=CastChecked<UFMCodexCardRackWidget>(S->GetWidgetFromName(TEXT("TheaterTakers")));
	const FString Empty=Text(S,TEXT("TheaterDetail")), Rule=Text(S,TEXT("TheaterReasonSecondary"));
	TestEqual(TEXT("Exact direct rule without defense modifier wording"),Empty,FString(TEXT("直接射门：取射门 / 传球较高值，与对方门将手控球进行判定")));
	TestEqual(TEXT("Exact base combination rule"),Rule,FString(TEXT("战术配合：需射门 + 传球 ≥ 8；两枚骰子总和 ≥ 9 进球")));
 TestEqual(TEXT("Empty selection subtitle"),Text(S,TEXT("TheaterSubtitle")),FString(TEXT("选择主罚球员")));
	TestNotEqual(TEXT("Helper complements substage title"),Empty,Text(S,TEXT("TheaterSubtitle")));
	const FName Id=Eligible(F,Combo);
	auto FindCard=[&](){return Rack->GetRenderedCardWidgets().FindByPredicate([Id](const auto& C){return C && C->GetPresentation().CardId==Id;});};
	const auto* CardPtr=FindCard(); if (!TestNotNull(TEXT("Canonical eligible taker is rendered"),CardPtr)) return false;
	auto* Card=CardPtr->Get(); const FString Name=Card->GetPresentation().IdentityLabel;
	const auto& Facts=S->GetPresentation().SetPiece.NearTakerEligibility;
 TestEqual(TEXT("Complete per-candidate semantic facts"),Facts.Num(),Rack->GetPresentation().Cells.Num());
 const auto* Fact=Facts.FindByPredicate([Id](const auto& V){return V.CardId==Id;});
 if (!TestNotNull(TEXT("Candidate eligibility is projected"),Fact)) return false;
 TestEqual(TEXT("Candidate semantic eligibility"),Fact->bCanUseTacticalCombination,Combo);
 const auto* Waiting=F.Defender()->GetPlayerMatchScreen();
 TestTrue(TEXT("Waiting viewer has no candidate eligibility"),Waiting->GetPresentation().SetPiece.NearTakerEligibility.IsEmpty());
 auto* Full=CastChecked<UFMCodexPlayerCardWidget>(S->GetWidgetFromName(TEXT("TheaterTakerFullCard")));
 TestFalse(TEXT("Empty selection has no full card"),Shown(S,TEXT("TheaterTakerFullCard")));
 TestTrue(TEXT("Empty selection has quiet placeholder"),Shown(S,TEXT("TheaterTakerPlaceholder")));
	const int32 Sends=F.Backend(F.Attacker()).Sends, Draws=F.Entropy->Calls;
	TestTrue(TEXT("Actual hand hover callback is available"),Card->RequestFullCardDetailHover());
	const FString Hover=Text(S,TEXT("TheaterReasonSecondary"));
	TestTrue(TEXT("Hover names the candidate"),Hover.Contains(Name));
 TestEqual(TEXT("Hover uses canonical Full Card identity"),Full->GetPresentation().CardId,Id);
 TestEqual(TEXT("Existing production Full Card mode"),Full->GetPresentationMode(),EFMCodexPlayerCardPresentationMode::InteractionChoice);
 TestEqual(TEXT("Full Card preserves ten attributes"),Full->GetRenderedAttributeCount(),10);
 TestFalse(TEXT("Full Card never owns selection"),Full->IsSelectableForCurrentPrompt());
 TestTrue(TEXT("Full Card is read-only including its children"),Full->GetVisibility()==ESlateVisibility::HitTestInvisible);
 TestEqual(TEXT("Hover appends projected eligibility to rule two"),Hover,Rule+TEXT("，")+Name+(Combo?TEXT("可用"):TEXT("不可用")));
 TestEqual(TEXT("Hover leaves direct rule unchanged"),Text(S,TEXT("TheaterDetail")),Empty);
 TestEqual(TEXT("Hover alone never selects subtitle"),Text(S,TEXT("TheaterSubtitle")),FString(TEXT("选择主罚球员")));
	TestFalse(TEXT("Candidate hover never opens full-card popup"),Shown(S,TEXT("TransientFullCardDetail")));
	Card->OnDetailHoverDismissed.Broadcast(Card);
	TestEqual(TEXT("Leaving hover restores empty decision support"),Text(S,TEXT("TheaterDetail")),Empty);
 TestEqual(TEXT("Leaving hover removes candidate suffix"),Text(S,TEXT("TheaterReasonSecondary")),Rule);
	TestEqual(TEXT("Hover does not submit intent"),F.Backend(F.Attacker()).Sends,Sends);
	TestEqual(TEXT("Hover does not consume entropy"),F.Entropy->Calls,Draws);
	TestTrue(TEXT("Candidate can be drafted"),Card->RequestOnPitchSelection());
	TestEqual(TEXT("Draft still sends no intent"),F.Backend(F.Attacker()).Sends,Sends);
	const FString Selected=Text(S,TEXT("TheaterSubtitle"));
 TestEqual(TEXT("Draft selection moves to top subtitle"),Selected,FString(TEXT("已选主罚球员："))+Name);
 TestEqual(TEXT("Selected info bar keeps direct rule only"),Text(S,TEXT("TheaterDetail")),Empty);
 TestEqual(TEXT("Selected info bar keeps combination rule with candidate suffix"),Text(S,TEXT("TheaterReasonSecondary")),Hover);
	const auto* Cell=Rack->GetPresentation().Cells.FindByPredicate([Id](const auto& V){return V.Card.CardId==Id;});
	if (TestNotNull(TEXT("Draft still belongs to candidate rack"),Cell))
		TestNotNull(TEXT("Theater draft has prominent inset outline"),Rack->GetWidgetFromName(FName(*FString::Printf(TEXT("CardDraftOutline%d"),Cell->StableIndex))));
	Card=FindCard()->Get(); Card->RequestFullCardDetailHover();
	TestEqual(TEXT("Hover preserves selected-player confirmation context"),Text(S,TEXT("TheaterSubtitle")),Selected);
	Card->OnDetailHoverDismissed.Broadcast(Card);
	TestEqual(TEXT("Leaving hover restores selected context"),Text(S,TEXT("TheaterSubtitle")),Selected);
 TestEqual(TEXT("Selection persists in inspector without hover"),Full->GetPresentation().CardId,Id);
 const FName OtherId=Eligible(F,!Combo);
 auto* OtherCard=Rack->GetRenderedCardWidgets().FindByPredicate([OtherId](const auto& V){return V->GetPresentation().CardId==OtherId;})->Get();
 OtherCard->RequestFullCardDetailHover();
 TestEqual(TEXT("Hover temporarily compares a different candidate"),Full->GetPresentation().CardId,OtherId);
 TestEqual(TEXT("Comparison uses other candidate eligibility"),Text(S,TEXT("TheaterReasonSecondary")),Rule+TEXT("，")+OtherCard->GetPresentation().IdentityLabel+(!Combo?TEXT("可用"):TEXT("不可用")));
 TestEqual(TEXT("Comparison cannot replace selected subtitle"),Text(S,TEXT("TheaterSubtitle")),Selected);
 TestEqual(TEXT("Comparison cannot replace direct explanation"),Text(S,TEXT("TheaterDetail")),Empty);
 Card->OnDetailHoverDismissed.Broadcast(Card);
 TestEqual(TEXT("Out-of-order leave cannot clear new hover"),Full->GetPresentation().CardId,OtherId);
 OtherCard->OnDetailHoverDismissed.Broadcast(OtherCard);
 TestEqual(TEXT("Leave restores selected full card"),Full->GetPresentation().CardId,Id);
 TestEqual(TEXT("Leave restores selected rule suffix"),Text(S,TEXT("TheaterReasonSecondary")),Hover);
 TestEqual(TEXT("Inspection does not change authoritative taker"),F.Backend(F.Attacker()).Sends,Sends);
 TestEqual(TEXT("Inspection draws no RNG"),F.Entropy->Calls,Draws);
 Click(S,TEXT("TheaterContinue"));
 TestFalse(TEXT("Confirm clears inspector before methods"),Shown(S,TEXT("TheaterTakerInspector")));
 TestTrue(TEXT("No retained card after leaving selection"),Full->GetPresentation().CardId.IsNone());
 OtherCard->RequestFullCardDetailHover();
 TestTrue(TEXT("Stale candidate callback cannot restore full card"),Full->GetPresentation().CardId.IsNone());
 TestTrue(TEXT("Methods no longer carry candidate facts"),S->GetPresentation().SetPiece.NearTakerEligibility.IsEmpty());
 TestEqual(TEXT("One explicit confirm sends one intent"),F.Backend(F.Attacker()).Sends,Sends+1);
	TestEqual(TEXT("Method substage remains once"),Text(S,TEXT("TheaterSubtitle")),FString(TEXT("选择结算方式")));
	TestNotEqual(TEXT("Method helper does not repeat the substage"),Text(S,TEXT("TheaterDetail")),Text(S,TEXT("TheaterSubtitle")));
	for (bool Direct:{true,false})
	{
		auto* Diagram=Cast<UFMCodexMatchFlowDiagram>(S->GetWidgetFromName(Direct?TEXT("TheaterNearDirectDiagram"):TEXT("TheaterNearCombinationDiagram")));
		if (TestNotNull(TEXT("Existing tactical diagram is reused"),Diagram))
		{
			TestTrue(TEXT("Supporting diagram is visible"),Diagram->GetVisibility()==ESlateVisibility::HitTestInvisible);
			TestEqual(TEXT("Correct method pictogram"),Diagram->GetDiagram(),Direct?EFMCodexFlowDiagram::Direct:EFMCodexFlowDiagram::Combination);
		}
	}
	TestTrue(TEXT("Direct hint explains opponent"),Text(S,TEXT("TheaterNearDirectHint")).Contains(TEXT("门将手控球 + 防守加成")));
	TestTrue(TEXT("Combination hint states threshold"),Text(S,TEXT("TheaterNearCombinationHint")).Contains(TEXT("≥ 9")));
	TestEqual(TEXT("Combination availability remains authority-projected"),CastChecked<UButton>(S->GetWidgetFromName(TEXT("TheaterNearCombination")))->GetIsEnabled(),Combo);
	if (!Combo) TestTrue(TEXT("Unavailable method has visible reason without a click"),Text(S,TEXT("TheaterDetail")).Contains(TEXT("不可用：需射门 + 传球 ≥ 8")));
	return true;
}

IMPLEMENT_COMPLEX_AUTOMATION_TEST(FNearTakerProjection,"FMCodex.NetworkPlay.SetPieceSelection.NearTakerProjection",
 EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
void FNearTakerProjection::GetTests(TArray<FString>& N,TArray<FString>& C) const
{ for (const TCHAR* Side:{TEXT("A"),TEXT("B")}) {N.Add(Side);C.Add(Side);} }
bool FNearTakerProjection::RunTest(const FString& Side)
{
 FUIFixture F(Side==TEXT("B"));auto* S=F.Attacker()->GetPlayerMatchScreen();
 F.Entropy->Word=8;S->RequestRollTacticalPoints();F.Settle();
 F.Entropy->Word=4;S->DevSetPieceAction(TEXT("SetPieceType"),NAME_None);F.Settle();
 const auto State=Access::Session(*F.Mode).GetStateSnapshot();
 const auto Actor=F.Attacker()->GetOwnerView().ViewerSide;
 const auto Safe=FFMCodexLocalMatchInteractionViewBuilder::BuildForViewer(State,Access::CallerRules(*F.Mode),Actor,FFMCodexLocalMatchViewerDisclosure::FullyDisclosed());
 const auto Projection=FFMCodexSetPieceSelectionPresentation::Build(Safe,Actor);
 TestEqual(TEXT("Complete bounded pool"),Projection.NearTakerEligibility.Num(),Projection.TakerOptions.Num());
 for (const auto& Fact:Projection.NearTakerEligibility)
 {
  FMatchPlaySetPieceParticipantEligibilityRequest R;R.ExpectedOwnerSide=Actor;R.CardId=Fact.CardId;R.Role=EMatchPlaySetPieceParticipantRole::Carrier;
  const auto Q=FMatchPlaySetPieceParticipantEligibility::Evaluate(State,R);
  TestTrue(TEXT("Only authoritative legal pool"),Q.bIsEligible);
  TestEqual(TEXT("Same canonical method query supplies each fact"),Fact.bCanUseTacticalCombination,FMatchPlayShortFreeKickResolution::IsAngledMethodEligible(Q.Binding.Snapshot));
 }
 TestTrue(TEXT("Hidden Type has no candidate facts"),Access::Safe(*F.Mode,Actor,true).NearTakerEligibility.IsEmpty());
 auto Waiting=FFMCodexLocalMatchInteractionViewBuilder::BuildForViewer(State,Access::CallerRules(*F.Mode),F.Defender()->GetOwnerView().ViewerSide,FFMCodexLocalMatchViewerDisclosure::FullyDisclosed());
 TestTrue(TEXT("Waiting view redacts before serialization"),Waiting.NearTakerEligibility.IsEmpty());
 for (int32 Mutation=0;Mutation<4;++Mutation)
 {
  auto Bad=Safe;
  if (Mutation==0) Bad.NearTakerEligibility.Pop();
  if (Mutation==1) {const auto Duplicate=Bad.NearTakerEligibility[0];Bad.NearTakerEligibility.Add(Duplicate);}
  if (Mutation==2) Bad.NearTakerEligibility[1]=Bad.NearTakerEligibility[0];
  if (Mutation==3) Bad.NearTakerEligibility[0].CardId=TEXT("NotInLegalPool");
  const auto Rejected=FFMCodexSetPieceSelectionPresentation::Build(Bad,Actor);
  TestTrue(TEXT("Incomplete/overflow/duplicate/foreign fact maps fail closed"),Rejected.bOptionsUnavailable && Rejected.TakerOptions.IsEmpty() && Rejected.NearTakerEligibility.IsEmpty());
 }
 auto Original=Projection;
 TArray<uint8> Bytes;FMemoryWriter Writer(Bytes);FObjectAndNameAsStringProxyArchive Out(Writer,false);
 FFMCodexSetPieceSelectionPresentation::StaticStruct()->SerializeItem(Out,&Original,nullptr);
 FFMCodexSetPieceSelectionPresentation Copy;FMemoryReader Reader(Bytes);FObjectAndNameAsStringProxyArchive In(Reader,false);
 FFMCodexSetPieceSelectionPresentation::StaticStruct()->SerializeItem(In,&Copy,nullptr);
 TestTrue(TEXT("Reflected candidate facts round-trip exactly"),FFMCodexSetPieceSelectionPresentation::StaticStruct()->CompareScriptStruct(&Original,&Copy,0));
 Copy.DisableActions();TestTrue(TEXT("Disabled/waiting actions clear candidate facts"),Copy.NearTakerEligibility.IsEmpty());
 return true;
}

#endif
