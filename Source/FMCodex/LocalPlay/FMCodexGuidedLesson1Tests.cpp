#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "FMCodexGuidedLesson1.h"
#include "FMCodexGuidedLesson1Focus.h"
#include "Styling/CoreStyle.h"
#include "FMCodexLocalMatchHostGameMode.h"
#include "FMCodexLocalMatchDemoConfiguration.h"
#include "FMCodexLocalMatchResolutionFeedback.h"
#include "FMCodexPlayerCardWidget.h"
#include "FMCodexLocalMatchScreenWidget.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Misc/AutomationTest.h"

namespace
{
using Lesson = FFMCodexGuidedLesson1;
using Side = EInitialTurnOrderPlayer;
using Kind = EMatchPlayAuthoritativeCommandKind;
struct FLessonWorld
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	AFMCodexLocalMatchHostGameMode* Host = nullptr;
	FLessonWorld()
	{
		GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
		Host = World->SpawnActor<AFMCodexLocalMatchHostGameMode>();
	}
	~FLessonWorld() { GEngine->DestroyWorldContext(World); World->DestroyWorld(false); }
	FFMCodexLocalMatchInteractionView View() const
	{
		FFMCodexMatchClientViewRequest R; R.ViewerSide = Side::PlayerA;
		R.Disclosure = FFMCodexLocalMatchViewerDisclosure::FullyDisclosed();
		return Host->GetViewForViewer(R).View;
	}
	void Observe(bool Ready = true, float Seconds = 2.f) { Host->GetGuidedLesson1()->Update(View(), Ready, Seconds); }
	template<class T> T Request(Side S = Side::PlayerA)
	{
		T R; R.RequestingSide = S;
		if constexpr (std::is_same_v<T, FMatchPlayFinishDeploymentIntent>
			|| std::is_same_v<T, FMatchPlayAuthoritativeSubmitBranchIntentRequest>
			|| std::is_same_v<T, FMatchPlayAuthoritativeResolveLongShotDirectAttackRollRequest>
			|| std::is_same_v<T, FMatchPlayAuthoritativeResolveLongShotDirectDefenseRollRequest>)
			R.AttackSequence = View().AttackSequence;
		else R.ExpectedAttackSequence = View().AttackSequence;
		return R;
	}
	template<class T> bool Submit(Kind K, T R, bool Script = false, bool ObserveAfter = true)
	{
		auto* L = Host->GetGuidedLesson1();
		if (L)
		{
			L->Update(View(), true, 2.f); L->bDispatchingOpponent = Script;
			// Headless contract fixture acknowledges the visual seam; real motion is exercised in PIE.
			if (Script && L->CanStartOpponentDeploymentMove()) { L->BeginOpponentDeploymentMove(); L->CompleteOpponentDeploymentMove(); }
		}
		const bool Ok = Host->SubmitPlayerIntent(FMatchPlayPlayerIntent::Create(K,R)).bSuccess;
		if (L) { L->bDispatchingOpponent = false; if (Script && Ok) L->OpponentActionSubmitted(); if (ObserveAfter) Observe(true,Script ? L->OpponentPostActionHold() : 2.f); }
		return Ok;
	}
	bool Attack(FAutomationTestBase& T, FName Card)
	{
		auto Deploy = Request<FMatchPlayAuthoritativeDeployOrdinaryRequest>(); Deploy.CardId=Card; Deploy.SlotId=Lesson::AttackerSlot();
		if (!T.TestTrue(TEXT("Player uses authoritative deployment"),Submit(Kind::DeployOrdinary,Deploy))) return false;
		Deploy=Request<FMatchPlayAuthoritativeDeployOrdinaryRequest>(Side::PlayerB); Deploy.CardId=Lesson::Stones(); Deploy.SlotId=Lesson::DefenderSlot();
		if (!T.TestTrue(TEXT("Script uses authoritative opponent deployment"),Submit(Kind::DeployOrdinary,Deploy,true))) return false;
		T.TestEqual(TEXT("Finish concept only in first attempt"),Host->GetGuidedLesson1()->IsExplanationMode(),!Host->GetGuidedLesson1()->IsComparison());
		Host->GetGuidedLesson1()->Primary();
		T.TestEqual(TEXT("Finish button focus"),Host->GetGuidedLesson1()->FocusTarget(),EFMCodexLesson1Focus::FinishDeployment);
		T.TestTrue(TEXT("Unused production availability remains for voluntary finish"),View().DeploymentOptions.Num()>1);
		if (!T.TestTrue(TEXT("Player voluntarily finishes"),Submit(Kind::FinishDeployment,Request<FMatchPlayFinishDeploymentIntent>()))) return false;
		if (!T.TestTrue(TEXT("Opponent finishes"),Submit(Kind::FinishDeployment,Request<FMatchPlayFinishDeploymentIntent>(Side::PlayerB),true))) return false;
		Host->GetGuidedLesson1()->Primary();
		T.TestEqual(TEXT("Pitch carrier focus"),Host->GetGuidedLesson1()->FocusTarget(),EFMCodexLesson1Focus::Carrier);
		auto Carrier=Request<FMatchPlayAuthoritativeSubmitCarrierRequest>(); Carrier.CarrierCardId=Card;
		if (!T.TestTrue(TEXT("Actual Carrier"),Submit(Kind::SubmitCarrier,Carrier))) return false;
		auto Marker=Request<FMatchPlayAuthoritativeSubmitMarkerRequest>(Side::PlayerB); Marker.MarkerCardId=Lesson::Stones();
		if (!T.TestTrue(TEXT("Actual Marker"),Submit(Kind::SubmitMarker,Marker,true))) return false;
		T.TestEqual(TEXT("Production automatically skips missing Runner"),View().InteractionCategory,EFMCodexLocalMatchInteractionCategory::SelectSkill);
		T.TestTrue(TEXT("No Runner or Helper"),View().SelectedRunnerCardId.IsNone() && View().SelectedHelperCardId.IsNone());
		Host->GetGuidedLesson1()->Primary();
		T.TestEqual(TEXT("Real LongShot focus"),Host->GetGuidedLesson1()->FocusTarget(),EFMCodexLesson1Focus::Skill);
		auto Skill=Request<FMatchPlayAuthoritativeSubmitSkillRequest>(); Skill.SkillId=Lesson::Skill();
		if (!T.TestTrue(TEXT("Production LongShot Skill"),Submit(Kind::SubmitSkill,Skill))) return false;
		Host->GetGuidedLesson1()->Primary();
		T.TestEqual(TEXT("Direct focus"),Host->GetGuidedLesson1()->FocusTarget(),EFMCodexLesson1Focus::DirectShot);
		T.TestTrue(TEXT("Removed secondary direct lecture"),Host->GetGuidedLesson1()->Explanation().IsEmpty());
		auto Branch=Request<FMatchPlayAuthoritativeSubmitBranchIntentRequest>(); Branch.Intent=EMatchPlayElectiveBranchIntent::DirectShot;
		if (!T.TestTrue(TEXT("Production Direct method"),Submit(Kind::SubmitBranchIntent,Branch))) return false;
		if (!Host->GetGuidedLesson1()->IsComparison())
		{
			T.TestTrue(TEXT("First formula requires inspection"),Host->GetGuidedLesson1()->InspectFormula(true));
			Host->GetGuidedLesson1()->Primary();
		}
		if (!T.TestTrue(TEXT("Authoritative attack roll"),Submit(Kind::ResolveLongShotDirectAttackRoll,Request<FMatchPlayAuthoritativeResolveLongShotDirectAttackRollRequest>()))) return false;
		if (!T.TestTrue(TEXT("Authoritative defense roll"),Submit(Kind::ResolveLongShotDirectDefenseRoll,Request<FMatchPlayAuthoritativeResolveLongShotDirectDefenseRollRequest>(Side::PlayerB),true,false))) return false;
		return true;
	}
};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLessonFlowTest,"FMCodex.LocalPlay.GuidedLesson1.FlowAndCheckpoint",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FLessonFlowTest::RunTest(const FString&)
{
	FLessonWorld F;
	if (!TestTrue(TEXT("Lesson production assumptions hold"),F.Host->StartGuidedLesson1())) return false;
	auto* L=F.Host->GetGuidedLesson1();
	TestEqual(TEXT("Intro"),L->GetStep(),EFMCodexLesson1Step::Intro);
	TestEqual(TEXT("Intro body gives the first action; LongShot objective is in the visual heading"),L->Instruction().ToString(),FString(TEXT("先掷出本回合的进攻战术点。")));
	TestEqual(TEXT("Intro CTA starts the action"),L->PrimaryLabel().ToString(),FString(TEXT("开始操作")));
	TestTrue(TEXT("Centered explanation contract"),L->IsExplanationMode());
	TestEqual(TEXT("No field focus"),L->FocusTarget(),EFMCodexLesson1Focus::None);
	TestEqual(TEXT("Initial score"),F.View().PlayerAScore,0);
	TestTrue(TEXT("Empty pitch"),F.View().DeploymentPlacements.IsEmpty());
	auto Model=FFMCodexLocalMatchUMGPresentationBuilder::Build(F.View(),{},TEXT(""),Side::PlayerA); L->ApplyPresentation(Model);
	TestEqual(TEXT("Only Gyokeres in lesson hand"),Model.LocalRack.Cells.Num(),1);
	TestEqual(TEXT("First card"),Model.LocalRack.Cells[0].Card.CardId,Lesson::Gyokeres());
	TestFalse(TEXT("Intro gates production TP command"),F.Submit(Kind::RequestInitialActionPointRoll,F.Request<FMatchPlayFullD12EntryRequest>()));
	L->Primary(); F.Observe();
	if (!TestTrue(TEXT("Player-triggered production Full D12"),F.Submit(Kind::RequestInitialActionPointRoll,F.Request<FMatchPlayFullD12EntryRequest>()))) return false;
	TestEqual(TEXT("TP = 3"),F.View().ActionPoint,3);
	TestEqual(TEXT("Explain TP before deployment"),L->GetStep(),EFMCodexLesson1Step::TacticPointExplanation);
	L->Primary();
	TestEqual(TEXT("Inspect real card"),L->GetStep(),EFMCodexLesson1Step::InspectGyokeres);
	F.Observe(true,60.f);
	TestEqual(TEXT("Timer cannot bypass inspection"),L->GetStep(),EFMCodexLesson1Step::InspectGyokeres);
	TestFalse(TEXT("Unrelated hover cannot advance"),L->InspectCard(Lesson::Stones(),true));
	TestFalse(TEXT("Invisible Full Card cannot advance"),L->InspectCard(Lesson::Gyokeres(),false));
	auto EarlyDeploy=F.Request<FMatchPlayAuthoritativeDeployOrdinaryRequest>(); EarlyDeploy.CardId=Lesson::Gyokeres(); EarlyDeploy.SlotId=Lesson::AttackerSlot();
	TestFalse(TEXT("Even correct deployment blocked during inspection"),F.Submit(Kind::DeployOrdinary,EarlyDeploy));
	TestTrue(TEXT("Real intended card visible"),L->InspectCard(Lesson::Gyokeres(),true));
	TestEqual(TEXT("Skill taught first"),L->GetStep(),EFMCodexLesson1Step::SkillRangeExplanation);
	TestTrue(TEXT("Range explanation"),L->Instruction().ToString().Contains(TEXT("远射技能范围为 3–5")));
	TestTrue(TEXT("Range explanation is modal"),L->IsExplanationMode());
	L->Primary();
	TestEqual(TEXT("First inspection proceeds straight from skill to deployment"),L->GetStep(),EFMCodexLesson1Step::Deploy);
	auto Invalid=F.Request<FMatchPlayAuthoritativeDeployOrdinaryRequest>(); Invalid.CardId=Lesson::Odegaard(); Invalid.SlotId=Lesson::AttackerSlot();
	TestFalse(TEXT("Odegaard not yet playable"),F.Submit(Kind::DeployOrdinary,Invalid));
	Invalid.CardId=Lesson::Gyokeres(); Invalid.SlotId=TEXT("Demo.Slot.NearB.01");
	TestFalse(TEXT("Other cell gated without changing production legality"),F.Submit(Kind::DeployOrdinary,Invalid));
	if (!F.Attack(*this,Lesson::Gyokeres())) return false;
	F.Observe(false);
	TestFalse(TEXT("Rewind waits for visible result"),L->GetStep()==EFMCodexLesson1Step::Rewind);
	F.Observe();
	TestEqual(TEXT("Failure gets a clean production hold first"),L->GetStep(),EFMCodexLesson1Step::FailurePause);
	F.Observe(true,Lesson::FailureFollowupDelay);
	if (!TestEqual(TEXT("One real contest"),F.View().ResolutionFacts.FormulaContests.Num(),1)) return false;
	const auto First=F.View().ResolutionFacts.FormulaContests[0];
	TestEqual(TEXT("First authoritative attack die"),First.ResolvedInput.Attacker.ComparePoint,5);
	TestEqual(TEXT("First authoritative defense die"),First.ResolvedInput.Defender.ComparePoint,3);
	TestEqual(TEXT("Production fixed defense modifier"),First.ResolvedInput.Defender.Modifier,3.f);
	TestFalse(TEXT("No goalkeeper Formula contribution"),First.bGoalkeeperParticipated);
	TestEqual(TEXT("Actual first attack"),First.AttackRow.FinalValue,9.f);
	TestEqual(TEXT("Actual first defense"),First.DefenseRow.FinalValue,10.f);
	TestEqual(TEXT("Production miss"),First.ResolvedResult.Winner,EFormulaWinner::Defender);
	TestEqual(TEXT("Miss score unchanged"),F.View().PlayerAScore,0);
	TestTrue(TEXT("Rewind omits noisy secondary paragraph"),L->Explanation().IsEmpty());
	L->Primary(); F.Observe(true,1.f);
	if (!TestTrue(TEXT("Predefined teaching checkpoint"),F.Host->RebuildLesson1DeploymentCheckpoint())) return false;
	const auto Clean=F.View();
	TestEqual(TEXT("TP preserved without player reroll"),Clean.ActionPoint,3);
	TestTrue(TEXT("No stale pitch/roles/selected tactic/rolls/formula/result"),Clean.DeploymentPlacements.IsEmpty()
		&& Clean.SelectedCarrierCardId.IsNone() && Clean.SelectedMarkerCardId.IsNone() && Clean.SelectedRunnerCardId.IsNone()
		&& Clean.SelectedHelperCardId.IsNone() && Clean.SelectedSkillId.IsNone() && Clean.AcceptedRolls.IsEmpty()
		&& Clean.ResolutionFacts.FormulaContests.IsEmpty() && Clean.GoalHistory.IsEmpty()
		&& !Clean.bTerminalPendingAdvance && Clean.PlayerAScore==0 && Clean.PlayerBScore==0);
	Model=FFMCodexLocalMatchUMGPresentationBuilder::Build(Clean,{},TEXT(""),Side::PlayerA); L->ApplyPresentation(Model);
	TestEqual(TEXT("Two real comparison cards unlocked"),Model.LocalRack.Cells.Num(),2);
	TestEqual(TEXT("Comparison requires real card inspection"),L->GetStep(),EFMCodexLesson1Step::InspectOdegaard);
	TestFalse(TEXT("Gyokeres cannot satisfy Odegaard inspection"),L->InspectCard(Lesson::Gyokeres(),true));
	TestTrue(TEXT("Odegaard inspected"),L->InspectCard(Lesson::Odegaard(),true));
	L->Primary(); L->Primary();
	TestEqual(TEXT("Trait teaching before deployment"),L->GetStep(),EFMCodexLesson1Step::TraitExplanation);
	L->Primary();
	Invalid=F.Request<FMatchPlayAuthoritativeDeployOrdinaryRequest>(); Invalid.CardId=Lesson::Gyokeres(); Invalid.SlotId=Lesson::AttackerSlot();
	TestFalse(TEXT("Repeated Gyokeres does not consume another attempt"),F.Submit(Kind::DeployOrdinary,Invalid));
	TestTrue(TEXT("Comparison stays empty"),F.View().DeploymentPlacements.IsEmpty());
	if (!F.Attack(*this,Lesson::Odegaard())) return false;
	F.Observe();
	const auto Second=F.View().ResolutionFacts.FormulaContests[0];
	TestEqual(TEXT("Same authoritative attack die"),Second.ResolvedInput.Attacker.ComparePoint,5);
	TestEqual(TEXT("Same authoritative defense die"),Second.ResolvedInput.Defender.ComparePoint,3);
	TestEqual(TEXT("Effective Shooting comes from production Trait"),Second.ResolvedInput.Attacker.BaseValue,6.f);
	TestTrue(TEXT("Authoritative operand preserves base, rank and role"),Second.AttackRow.Terms.ContainsByPredicate([](const auto& Term)
	{
		const auto& Op=Term.AttributeOperand;
		return Term.CardId==Lesson::Odegaard() && Term.ParticipantRole==EMatchPlayResolutionParticipantRole::Carrier
			&& Op.BaseValue==4 && Op.Bonus==2 && Op.EffectiveValue==6
			&& Op.TraitId==FName(TEXT("Trait.LongShotCarrier"));
	}));
	TestEqual(TEXT("Production Trait produces 11"),Second.AttackRow.FinalValue,11.f);
	TestEqual(TEXT("Same defense produces 10"),Second.DefenseRow.FinalValue,10.f);
	TestEqual(TEXT("Production goal"),Second.ResolvedResult.Winner,EFormulaWinner::Attacker);
	TestEqual(TEXT("Normal score lifecycle"),F.View().PlayerAScore,1);
	TestEqual(TEXT("Lesson summary"),L->GetStep(),EFMCodexLesson1Step::Summary);
	L->Primary(); TestEqual(TEXT("Explicit completion"),L->GetStep(),EFMCodexLesson1Step::Complete);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLessonIsolationTest,"FMCodex.LocalPlay.GuidedLesson1.IsolationAndHints",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FLessonIsolationTest::RunTest(const FString&)
{
	FLessonWorld F;
	TestTrue(TEXT("Start"),F.Host->StartGuidedLesson1());
	auto* L=F.Host->GetGuidedLesson1(); L->Primary(); F.Observe();
	TestTrue(TEXT("Consume tutorial TP"),F.Submit(Kind::RequestInitialActionPointRoll,F.Request<FMatchPlayFullD12EntryRequest>()));
	// Hint timing is lesson state only, driven by real elapsed game time in the controller.
	L->EnterComparison(); L->InspectCard(Lesson::Odegaard(),true); L->Primary(); L->Primary(); L->Primary(); L->Update(F.View(),true,12.f); TestEqual(TEXT("Hint 1"),L->GetHintLevel(),1);
	L->Update(F.View(),true,13.f); TestEqual(TEXT("Hint 2"),L->GetHintLevel(),2);
	TestTrue(TEXT("Hint names Trait"),L->Instruction().ToString().Contains(TEXT("远射专家")));
	auto Demo=FFMCodexLocalMatchDemoConfigurationFactory::Create();
	TestTrue(TEXT("Normal match replaces lesson runtime"),F.Host->StartNewLocalMatch(Demo.OpeningInput,Demo.SkillRuleSet,1234).bSuccess);
	TestNull(TEXT("No tutorial state/rewind"),F.Host->GetGuidedLesson1());
	TestTrue(TEXT("No tutorial RNG values survive"),F.Host->GetLocalDevPendingRollOverrides().IsEmpty());
	auto R=F.Request<FMatchPlayFullD12EntryRequest>(F.View().ExpectedActingPlayer);
	TestTrue(TEXT("Normal entry ungated"),F.Submit(Kind::RequestInitialActionPointRoll,R));
	const int32 NormalRoll=F.View().RawInitialD12;
	F.Host->StartNewLocalMatch(Demo.OpeningInput,Demo.SkillRuleSet,1234);
	R=F.Request<FMatchPlayFullD12EntryRequest>(F.View().ExpectedActingPlayer);
	F.Submit(Kind::RequestInitialActionPointRoll,R);
	TestEqual(TEXT("Normal seeded provider unchanged by prior lesson"),F.View().RawInitialD12,NormalRoll);
	const auto P=FFMCodexLocalMatchUMGPresentationBuilder::Build(F.View(),{},TEXT(""),Side::PlayerA);
	TestEqual(TEXT("Normal roster still full"),P.LocalRack.Cells.Num(),20);
	const auto NormalView=F.View();
	const auto* Other=NormalView.DeploymentOptions.FindByPredicate([](const auto& Option)
	{
		return !Option.bGoalkeeper && !Option.SlotId.IsNone()
			&& Option.CardId!=Lesson::Gyokeres() && Option.CardId!=Lesson::Odegaard() && Option.CardId!=Lesson::Stones();
	});
	if (TestNotNull(TEXT("Normal unrelated card available"),Other))
	{
		auto Deploy=F.Request<FMatchPlayAuthoritativeDeployOrdinaryRequest>(NormalView.ExpectedActingPlayer);
		Deploy.CardId=Other->CardId; Deploy.SlotId=Other->SlotId;
		TestTrue(TEXT("Normal deployment has no lesson gate"),F.Submit(Kind::DeployOrdinary,Deploy));
	}
	return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLessonFocusTest,"FMCodex.LocalPlay.GuidedLesson1.FocusModeAndGating",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FLessonFocusTest::RunTest(const FString&)
{
	using Step=EFMCodexLesson1Step; using Focus=EFMCodexLesson1Focus; using Gesture=EFMCodexMatchScreenIntent;
	using Surface=EFMCodexLesson1CopySurface;
	using namespace FMCodexLesson1Focus;
	const auto SmallFont=FCoreStyle::GetDefaultFontStyle("Regular",17);
	const auto TitleFont=FCoreStyle::GetDefaultFontStyle("Bold",34);
	TestEqual(TEXT("One dot for each of two Han glyphs"),MeasureGlyphDots(TEXT("远射"),TitleFont).Centers.Num(),2);
	TestEqual(TEXT("TP uses five glyph markers"),MeasureGlyphDots(TEXT("进攻战术点"),SmallFont).Centers.Num(),5);
	const auto Rank=MeasureGlyphDots(TEXT("远射专家 A。"),SmallFont);
	TestEqual(TEXT("Rank glyph gets a dot, whitespace/punctuation do not"),Rank.Centers.Num(),5);
	TestTrue(TEXT("Whitespace advance retained before Latin rank"),Rank.Centers.Last()-Rank.Centers[3]>0.f);
	TestTrue(TEXT("Title dots scale with measured font height"),MeasureGlyphDots(TEXT("远射"),TitleFont).Diameter>MeasureGlyphDots(TEXT("远射"),SmallFont).Diameter);
	const auto Mixed=MeasureGlyphDots(TEXT("iWi"),SmallFont);
	const auto Single=MeasureGlyphDots(TEXT("W"),SmallFont);
	TestTrue(TEXT("Single glyph dot is centered inside its advance, not on the leading edge"),Single.Centers.Num()==1 && FMath::IsNearlyEqual(Single.Centers[0],Single.TextSize.X*.5f) && Single.Centers[0]>0.f);
	TestTrue(TEXT("Different glyph advances are not uniform word/count spacing"),Mixed.Centers.Num()==3 && !FMath::IsNearlyEqual(Mixed.Centers[0]*2.f,Mixed.Centers[1]-Mixed.Centers[0]));
	// Same conversion under window translation, nested layout scaling and render scaling.
	auto MakeOverlay=[](FVector2D Origin){return FGeometry::MakeRoot(FVector2D(1200,800),FSlateLayoutTransform(1.25f,Origin));};
	auto MakeTarget=[](const FGeometry& G){return G.MakeChild(FVector2D(160,60),FSlateLayoutTransform(.8f,FVector2D(120,80)),FSlateRenderTransform(FScale2D(1.15f,.9f)),FVector2D(.5,.5));};
	const auto G=MakeOverlay(FVector2D(180,95)), Moved=MakeOverlay(FVector2D(440,220));
	const auto R=ConvertTargetGeometry(MakeTarget(G),G,ETargetShape::Button);
	const auto Shifted=ConvertTargetGeometry(MakeTarget(Moved),Moved,ETargetShape::Button);
	if (!TestTrue(TEXT("Valid scaled target geometry"),R.IsSet() && Shifted.IsSet())) return false;
	TestTrue(TEXT("Window desktop origin cannot offset the frame"),FMath::IsNearlyEqual(R->Left,Shifted->Left,.001f) && FMath::IsNearlyEqual(R->Top,Shifted->Top,.001f));
	const auto Center=G.AbsoluteToLocal(MakeTarget(G).LocalToAbsolute(FVector2D(80,30)));
	TestTrue(TEXT("Symmetric frame follows transformed target center"),FMath::IsNearlyEqual((R->Left+R->Right)*.5,Center.X,.001) && FMath::IsNearlyEqual((R->Top+R->Bottom)*.5,Center.Y,.001));
	TestFalse(TEXT("Unarranged target has no fallback frame"),ConvertTargetGeometry(FGeometry(),G,ETargetShape::Card).IsSet());
	Lesson L;
	FFMCodexMatchScreenRequest Roll; Roll.Kind=Gesture::TacticalPoints;
	FFMCodexMatchScreenRequest Deploy; Deploy.Kind=Gesture::DeployOrdinary; Deploy.OptionId=Lesson::Gyokeres(); Deploy.SlotId=Lesson::AttackerSlot();
	TestTrue(TEXT("Intro is explanatory"),L.IsExplanationMode());
	TestEqual(TEXT("Concept belongs to the actual sentence"),L.EmphasizeKeywords(L.Instruction()).ToString(),FString(TEXT("先掷出本回合的<concept>进攻战术点</>。")));
	TestEqual(TEXT("Title has its own deliberate phrase"),L.EmphasizeKeywords(FText::FromString(TEXT("进行一次远射")),Surface::Heading).ToString(),FString(TEXT("进行一次<concept>远射</>")));
	TestEqual(TEXT("Title keyword does not spill into body"),L.EmphasizeKeywords(FText::FromString(TEXT("远射"))).ToString(),FString(TEXT("远射")));
	TestEqual(TEXT("Blue section label never gets dots"),L.EmphasizeKeywords(FText::FromString(TEXT("进攻战术点")),Surface::Section).ToString(),FString(TEXT("进攻战术点")));
	TestEqual(TEXT("Blue secondary copy never gets dots"),L.EmphasizeKeywords(L.Instruction(),Surface::Secondary).ToString(),L.Instruction().ToString());
	TestEqual(TEXT("Approved phrase marks only its first occurrence"),L.EmphasizeKeywords(FText::FromString(TEXT("进攻战术点，进攻战术点"))).ToString(),FString(TEXT("<concept>进攻战术点</>，进攻战术点")));
	TestEqual(TEXT("Ordinary words stay ordinary"),L.EmphasizeKeywords(FText::FromString(TEXT("开始操作"))).ToString(),FString(TEXT("开始操作")));
	TestFalse(TEXT("Modal blocks real roll even via keyboard/backend"),L.AllowsScreen(Roll));
	L.Primary();
	TestEqual(TEXT("Focus is real TP control"),L.FocusTarget(),Focus::TacticPoint);
	TestTrue(TEXT("Only requested roll allowed"),L.AllowsScreen(Roll));
	TestFalse(TEXT("Unrelated deployment blocked"),L.AllowsScreen(Deploy));
	FFMCodexLocalMatchInteractionView V; V.InteractionCategory=EFMCodexLocalMatchInteractionCategory::Deploy; V.ExpectedActingPlayer=Side::PlayerA;
	L.Update(V,true,.1f);
	TestEqual(TEXT("TP concept has no active target"),L.FocusTarget(),Focus::None);
	TestFalse(TEXT("Explanation blocks deployment"),L.AllowsScreen(Deploy));
	L.Primary();
	TestEqual(TEXT("Hover semantic target"),L.FocusTarget(),Focus::HandCard);
	TestTrue(TEXT("Intended card inspection allowed"),L.AllowsInspection(Lesson::Gyokeres()));
	TestFalse(TEXT("Unrelated card inspection gated"),L.AllowsInspection(Lesson::Stones()));
	TestFalse(TEXT("Inspection cannot submit gameplay"),L.AllowsScreen(Deploy));
	L.InspectCard(Lesson::Gyokeres(),true);
	TestEqual(TEXT("Hover target cleared on inspection"),L.FocusTarget(),Focus::None);
	TestEqual(TEXT("Skill before Shooting"),L.GetStep(),Step::SkillRangeExplanation);
	TestEqual(TEXT("Skill phrase is deliberate, not separate substring rules"),L.ConceptKeywords()[0].ToString(),FString(TEXT("远射技能")));
	TestEqual(TEXT("Range explains current TP explicitly"),L.Instruction().ToString(),FString(TEXT("远射技能范围为 3–5。当前进攻战术点为 3，落在范围内，因此可以使用远射。")));
	L.Primary();
	TestEqual(TEXT("First pass skips standalone Shooting teaching"),L.GetStep(),Step::Deploy);
	TestEqual(TEXT("Drag semantic group"),L.FocusTarget(),Focus::Deployment);
	TestTrue(TEXT("Intended drag/drop allowed"),L.AllowsScreen(Deploy));
	Deploy.SlotId=Lesson::DefenderSlot();
	TestFalse(TEXT("Other drop rejected independent of coordinates"),L.AllowsScreen(Deploy));
	V.InteractionCategory=EFMCodexLocalMatchInteractionCategory::SelectSkill;
	L.Update(V,true,.1f);
	TestEqual(TEXT("No Runner modal; next concept is tactic"),L.GetStep(),Step::SkillExplanation);
	L.Primary();
	FFMCodexMatchScreenRequest Skill; Skill.Kind=Gesture::Skill; Skill.OptionId=Lesson::Skill();
	TestTrue(TEXT("Semantic LongShot allowed"),L.AllowsScreen(Skill));
	Skill.OptionId=TEXT("Other.Skill"); TestFalse(TEXT("Other Skill blocked"),L.AllowsScreen(Skill));
	V.InteractionCategory=EFMCodexLocalMatchInteractionCategory::SelectLongShotBranch;
	L.Update(V,true,.1f);
	TestEqual(TEXT("Branch guidance copy"),L.Instruction().ToString(),FString(TEXT("进攻分支下有具体说明，这次请选择‘直接射门’。")));
	L.Primary();
	FFMCodexMatchScreenRequest Method; Method.Kind=Gesture::Branch; Method.Branch=EFMCodexUMGBranchIntent::DirectShot;
	TestTrue(TEXT("Direct shot allowed"),L.AllowsScreen(Method));
	Method.Branch=EFMCodexUMGBranchIntent::DeadCorner; TestFalse(TEXT("Other method blocked"),L.AllowsScreen(Method));
	TestTrue(TEXT("No old secondary method lecture"),L.Explanation().IsEmpty());
	V.InteractionCategory=EFMCodexLocalMatchInteractionCategory::RollLongShotDirectAttack;
	L.Update(V,false,5.f);
	TestEqual(TEXT("Formula teaching waits for production readiness"),L.GetStep(),Step::DirectShot);
	L.Update(V,true,.1f);
	TestEqual(TEXT("First formula focuses real value"),L.FocusTarget(),Focus::FormulaValue);
	FFMCodexMatchScreenRequest Attack; Attack.Kind=Gesture::Continue; Attack.Category=EFMCodexUMGInteractionCategory::RollLongShotDirectAttack;
	TestFalse(TEXT("Cannot roll before real source hover"),L.AllowsScreen(Attack));
	L.Primary(); TestEqual(TEXT("Primary cannot bypass hover"),L.GetStep(),Step::FormulaHover);
	TestFalse(TEXT("Hidden tooltip cannot satisfy inspection"),L.InspectFormula(false));
	L.RequestExitConfirmation(); TestFalse(TEXT("Confirmation cannot consume hover"),L.InspectFormula(true)); L.CancelExitConfirmation();
	TestTrue(TEXT("Visible production source satisfies hover"),L.InspectFormula(true));
	TestEqual(TEXT("Source explanation precedes roll"),L.GetStep(),Step::FormulaExplanation);
	TestTrue(TEXT("Formula shares canonical explanation mode"),L.IsExplanationMode());
	TestEqual(TEXT("Formula explains source, sum and this attack's sole attribute"),L.Instruction().ToString(),FString(TEXT("悬停属性值，可查看它的来源。\n公式总值由属性值与掷骰值相加得到。\n本次属性值仅来自哲凯赖什的射门 4。")));
	TestEqual(TEXT("Formula body deliberately marks each selected phrase once"),L.EmphasizeKeywords(L.Instruction()).ToString(),FString(TEXT("悬停<concept>属性值</>，可查看它的来源。\n<concept>公式总值</>由属性值与<concept>掷骰值</>相加得到。\n本次属性值仅来自哲凯赖什的射门 4。")));
	L.Update(V,true,10.f); TestEqual(TEXT("Explanation waits for acknowledgement"),L.GetStep(),Step::FormulaExplanation);
	TestFalse(TEXT("Explanation still gates roll"),L.AllowsScreen(Attack));
	FMCodexTacticalScene::FState Scene; Scene.Facts.bActive=true; Scene.Facts.Method=FMCodexTacticalScene::EMethod::Direct;
	Scene.Phase=FMCodexTacticalScene::EPhase::FormulaHold;
	TestFalse(TEXT("Stable formula permits real teaching"),L.YieldsToProduction(Scene,false,V.InteractionCategory));
	TestTrue(TEXT("Reel still owns visible reveal"),L.YieldsToProduction(Scene,true,V.InteractionCategory));
	Scene.Phase=FMCodexTacticalScene::EPhase::Outcome;
	TestTrue(TEXT("Animating scene always wins"),L.YieldsToProduction(Scene,false,V.InteractionCategory));
	L.Primary(); TestTrue(TEXT("Only after explanation may player roll"),L.AllowsScreen(Attack));
	Scene.Phase=FMCodexTacticalScene::EPhase::FormulaHold;
	V.InteractionCategory=EFMCodexLocalMatchInteractionCategory::RollLongShotDirectDefense;
	TestEqual(TEXT("Reproduce timer lag: Lesson is still AttackRoll"),L.GetStep(),Step::AttackRoll);
	TestTrue(TEXT("Live defense action suppresses stale tutorial immediately after reel"),L.YieldsToProduction(Scene,false,V.InteractionCategory));
	FFMCodexUMGMatchScreenViewModel Pending;
	Pending.Interaction.Category=EFMCodexUMGInteractionCategory::RollLongShotDirectDefense;
	Pending.Interaction.bCanContinue=true; Pending.LongShotResolution.bCanContinue=true;
	L.ApplyPresentation(Pending);
	TestFalse(TEXT("Stale player step cannot expose opponent CTA"),Pending.Interaction.bCanContinue || Pending.LongShotResolution.bCanContinue);
	V.bTerminalPendingAdvance=true;
	V.ResolutionFacts.FormulaContests.AddDefaulted();
	V.ResolutionFacts.FormulaContests[0].bHasResolvedFormula=true;
	V.ResolutionFacts.FormulaContests[0].ResolvedResult.Winner=EFormulaWinner::Defender;
	L.Update(V,false,5.f); TestEqual(TEXT("Authority resolution cannot show teaching early"),L.GetStep(),Step::ResultReveal);
	L.Update(V,true,2.f); TestEqual(TEXT("First ready tick starts delay from zero"),L.GetStep(),Step::FailurePause);
	Scene.Facts.Outcome=FMCodexTacticalScene::EOutcome::DefensiveSuccess; Scene.Phase=FMCodexTacticalScene::EPhase::ResultHold;
	TestTrue(TEXT("Clean failure moment has no tutorial chrome"),L.YieldsToProduction(Scene,false,V.InteractionCategory));
	L.Update(V,true,Lesson::FailureFollowupDelay-.1f); TestEqual(TEXT("No popup before extended elapsed hold"),L.GetStep(),Step::FailurePause);
	L.Update(V,true,.11f); TestEqual(TEXT("Teaching follows 1.6 seconds of visible failure"),L.GetStep(),Step::Rewind);
	TestEqual(TEXT("Rewind distinguishes skill and trait concisely"),L.EmphasizeKeywords(L.Instruction()).ToString(),FString(TEXT("哲凯赖什可以使用<concept>远射技能</>，但他没有<concept>远射特性</>。让我们换个人试试吧。")));
	TestTrue(TEXT("No secondary rewind warning paragraph"),L.Explanation().IsEmpty());
	TestFalse(TEXT("Follow-up can now render"),L.YieldsToProduction(Scene,false,V.InteractionCategory));
	V.bTerminalPendingAdvance=false; V.ResolutionFacts.FormulaContests.Reset();
	L.EnterComparison();
	L.InspectCard(Lesson::Odegaard(),true);
	TestEqual(TEXT("Comparison skill first"),L.GetStep(),Step::SkillRangeExplanation);
	L.Primary(); TestEqual(TEXT("Comparison Shooting second"),L.GetStep(),Step::ShootingExplanation);
	L.Primary(); TestEqual(TEXT("Comparison Trait third"),L.GetStep(),Step::TraitExplanation);
	TestEqual(TEXT("Trait concept"),L.ConceptKeywords()[0].ToString(),FString(TEXT("特性")));
	TestEqual(TEXT("Expert concept"),L.ConceptKeywords()[1].ToString(),FString(TEXT("远射专家 A")));
	const auto Before=L.GetStep();
	L.RequestExitConfirmation(); L.Primary(); L.Update(V,true,10.f);
	TestEqual(TEXT("Confirmation freezes teaching only"),L.GetStep(),Before);
	TestFalse(TEXT("Confirmation blocks typed gameplay input"),L.AllowsScreen(Method));
	L.CancelExitConfirmation(); TestEqual(TEXT("Cancel retains exact substep"),L.GetStep(),Before);
	L.RequestExitConfirmation(); TestTrue(TEXT("Explicit second confirmation accepted"),L.ConfirmExit());
	TestFalse(TEXT("No implicit exit"),L.ConfirmExit());
	L.RequestExitConfirmation(); L.EnterComparison();
	TestFalse(TEXT("Rewind clears confirmation"),L.IsExitConfirmationOpen());
	TestFalse(TEXT("Rewind clears opponent presentation"),L.IsOpponentSettling());
	// Accepted opponent commands settle before the next teaching step; never replay.
	Lesson P; P.Primary();
	V.InteractionCategory=EFMCodexLocalMatchInteractionCategory::Deploy; V.ExpectedActingPlayer=Side::PlayerB;
	P.Update(V,true,0.f);
	P.Update(V,true,Lesson::OpponentDeployAttention-.01f);
	TestTrue(TEXT("Opponent source visibly focused before dispatch"),P.IsOpponentFocusVisible());
	TestEqual(TEXT("Attention targets actual source"),P.OpponentTarget(),EFMCodexLesson1OpponentTarget::HandCard);
	TestFalse(TEXT("Pre-action is not dispatch"),P.IsOpponentActionDue());
	TestFalse(TEXT("Attention cannot start movement early"),P.CanStartOpponentDeploymentMove());
	P.Update(V,true,.02f); P.BeginOpponentDeploymentMove();
	TestEqual(TEXT("Moving proxy is the sole opponent pointer target"),P.OpponentTarget(),EFMCodexLesson1OpponentTarget::MovingCard);
	P.Update(V,true,10.f);
	TestFalse(TEXT("Elapsed time alone cannot deploy before visual arrival"),P.IsOpponentActionDue());
	P.RequestExitConfirmation(); P.CompleteOpponentDeploymentMove();
	TestFalse(TEXT("Exit confirmation freezes arrival"),P.IsOpponentActionDue()); P.CancelExitConfirmation();
	P.CompleteOpponentDeploymentMove();
	TestTrue(TEXT("Only visual arrival makes original deployment due"),P.IsOpponentActionDue());
	TestEqual(TEXT("Same authoritative deployment gesture"),P.OpponentAction().Kind,Gesture::DeployOrdinary);
	P.OpponentActionSubmitted();
	TestEqual(TEXT("Accepted deployment targets actual field card"),P.OpponentTarget(),EFMCodexLesson1OpponentTarget::FieldCard);
	V.InteractionCategory=EFMCodexLocalMatchInteractionCategory::SelectCarrier; V.ExpectedActingPlayer=Side::PlayerA;
	P.Update(V,true,.3f); TestEqual(TEXT("Placed card settles before instruction changes"),P.GetStep(),Step::OpponentDeploy);
	TestFalse(TEXT("No duplicate opponent dispatch while settling"),P.IsOpponentActionDue());
	P.Update(V,true,Lesson::OpponentDestinationHold-.3f+.01f);
	TestTrue(TEXT("Final board hold is a separate phase"),P.IsOpponentFinalHold());
	TestFalse(TEXT("No extra arrow distracts from settled board"),P.IsOpponentFocusVisible());
	TestFalse(TEXT("Final board hold cannot repeat command"),P.IsOpponentActionDue());
	P.Update(V,true,Lesson::OpponentDeploySettledHold); TestEqual(TEXT("Next instruction only after destination and final hold"),P.GetStep(),Step::CarrierExplanation);
	FFMCodexMatchScreenRequest Carrier; Carrier.Kind=Gesture::Carrier; Carrier.OptionId=Lesson::Gyokeres();
	TestFalse(TEXT("Carrier concept still gates selection"),P.AllowsScreen(Carrier)); P.Primary();
	TestEqual(TEXT("Exact approved carrier instruction"),P.Instruction().ToString(),FString(TEXT("点击场上的哲凯赖什，将他选中为本进攻回合的持球队员。")));
	TestTrue(TEXT("Carrier action unlocks only after explanation"),P.AllowsScreen(Carrier));
	Lesson Repeat; Repeat.EnterComparison(); Repeat.InspectCard(Lesson::Odegaard(),true); Repeat.Primary(); Repeat.Primary(); Repeat.Primary();
	V.InteractionCategory=EFMCodexLocalMatchInteractionCategory::Deploy; V.ExpectedActingPlayer=Side::PlayerB;
	Repeat.Update(V,true,0.f);
	Repeat.Update(V,true,Lesson::OpponentDeployAttention*.80f+.01f);
	TestTrue(TEXT("Repeated deployment uses 80 percent attention pace"),Repeat.CanStartOpponentDeploymentMove());
	Repeat.BeginOpponentDeploymentMove(); Repeat.CompleteOpponentDeploymentMove();
	TestTrue(TEXT("Repeated deployment still requires visual arrival"),Repeat.IsOpponentActionDue());
	TestTrue(TEXT("Repeated gesture retains full readable end hold"),FMath::IsNearlyEqual(Repeat.OpponentPostActionHold(),Lesson::OpponentDestinationHold*.80f+Lesson::OpponentDeploySettledHold));
	Repeat.EnterComparison(); TestFalse(TEXT("Rewind clears moving proxy state"),Repeat.IsOpponentDeploymentMoving());
	// Finish button wording is a whole phrase; generic deployment remains ordinary.
	Lesson Finish; Finish.Primary();
	V.ExpectedActingPlayer=Side::PlayerA; V.PitchRegions.AddDefaulted();
	auto& Placed=V.PitchRegions[0].Slots.AddDefaulted_GetRef(); Placed.bOccupied=true; Placed.Card.CardId=Lesson::Gyokeres();
	Finish.Update(V,true,0.f);
	TestEqual(TEXT("Placed attacker introduces finish concept"),Finish.GetStep(),Step::FinishExplanation);
	TestEqual(TEXT("Only finish button phrase emphasized in explanation"),Finish.EmphasizeKeywords(Finish.Instruction()).ToString(),FString(TEXT("斯通斯已上场。此次远射只需一名进攻球员，可以<concept>结束部署</>。")));
	Finish.Primary();
	TestEqual(TEXT("Trailing generic deployment has no dots"),Finish.EmphasizeKeywords(Finish.Instruction()).ToString(),FString(TEXT("点击“<concept>结束部署</>”，结束本次部署。")));
	V.PitchRegions.Reset();
	// Both non-deployment opponent actions preserve the full accepted-action hold.
	for (const bool bFinish : {true,false})
	{
		Lesson Opponent; Opponent.Primary();
		V.InteractionCategory=bFinish ? EFMCodexLocalMatchInteractionCategory::Deploy : EFMCodexLocalMatchInteractionCategory::SelectMarker;
		V.ExpectedActingPlayer=Side::PlayerB; V.bPlayerADeploymentFinished=bFinish;
		Opponent.Update(V,true,0.f); Opponent.Update(V,true,2.f);
		TestEqual(TEXT("Finish targets side status; marker targets deployed Stones"),Opponent.OpponentTarget(),bFinish?EFMCodexLesson1OpponentTarget::SideStatus:EFMCodexLesson1OpponentTarget::FieldCard);
		TestTrue(TEXT("Original opponent action becomes due"),Opponent.IsOpponentActionDue());
		Opponent.OpponentActionSubmitted();
		TestEqual(TEXT("Opponent result prose has no incidental dots"),Opponent.EmphasizeKeywords(Opponent.Instruction()).ToString(),Opponent.Instruction().ToString());
		V.InteractionCategory=bFinish ? EFMCodexLocalMatchInteractionCategory::SelectCarrier : EFMCodexLocalMatchInteractionCategory::SelectSkill;
		Opponent.Update(V,true,Lesson::OpponentSettledHold-.1f);
		TestEqual(TEXT("Accepted action remains visible during end hold"),Opponent.GetStep(),bFinish ? Step::OpponentFinish : Step::OpponentMarker);
		TestFalse(TEXT("Accepted opponent action cannot dispatch twice"),Opponent.IsOpponentActionDue());
		Opponent.Update(V,true,.11f);
		TestEqual(TEXT("Next instruction follows full end hold"),Opponent.GetStep(),bFinish ? Step::CarrierExplanation : Step::SkillExplanation);
	}
	// Render real production cards, including reordering and refresh/absence semantics.
	FLessonWorld F; F.Host->StartGuidedLesson1();
	auto* Detail=NewObject<UFMCodexPlayerCardWidget>(F.World); Detail->TakeWidget();
	auto Model=FFMCodexLocalMatchUMGPresentationBuilder::Build(F.View(),{},TEXT(""),Side::PlayerA);
	for (const auto Id : {Lesson::Gyokeres(),Lesson::Odegaard()})
	{
	 const auto* Cell=Model.LocalRack.Cells.FindByPredicate([Id](const auto& C){return C.Card.CardId==Id;});
	 if (!TestNotNull(TEXT("Real roster card"),Cell)) return false;
	 auto Card=Cell->Card;
	 Detail->RefreshFromPresentation(Card,EFMCodexPlayerCardPresentationMode::InteractionChoice);
	 TestNotNull(TEXT("Shooting canonical anchor"),Detail->FindAttributePresentationWidget(TEXT("SHO")));
	 TestNotNull(TEXT("LongShot stable skill anchor"),Detail->FindSkillPresentationWidget(Lesson::Skill()));
	 TestEqual(TEXT("Trait anchor reflects real assignment"),Detail->FindTraitPresentationWidget(TEXT("Trait.LongShotCarrier"))!=nullptr,Id==Lesson::Odegaard());
	 TestNull(TEXT("Unknown trait has no anchor"),Detail->FindTraitPresentationWidget(TEXT("Missing")));
	 if(Card.Skills.Num()>1) Swap(Card.Skills[0],Card.Skills.Last());
	 Detail->RefreshFromPresentation(Card,EFMCodexPlayerCardPresentationMode::InteractionChoice);
	 TestNotNull(TEXT("Skill anchor survives reorder"),Detail->FindSkillPresentationWidget(Lesson::Skill()));
	 Detail->RefreshFromPresentation(Card,EFMCodexPlayerCardPresentationMode::HandMicro);
	 TestNull(TEXT("Non Full Card has no stale attribute anchor"),Detail->FindAttributePresentationWidget(TEXT("SHO")));
	 TestNull(TEXT("Non Full Card has no stale skill anchor"),Detail->FindSkillPresentationWidget(Lesson::Skill()));
	 TestNull(TEXT("Non Full Card has no stale trait anchor"),Detail->FindTraitPresentationWidget(TEXT("Trait.LongShotCarrier")));
	}

	return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLessonProductionTest,"FMCodex.LocalPlay.GuidedLesson1.ProductionLifecycleAndReset",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FLessonProductionTest::RunTest(const FString&)
{
 using namespace FMCodexTacticalScene;
 Lesson L; FFMCodexLocalMatchInteractionView V; V.AttackSequence=7; V.bTerminalPendingAdvance=true;
 FFacts Facts; Facts.bActive=true; Facts.AttackSequence=7; FState Scene; Scene.Sync(Facts);
 auto Ready=[&](){return Lesson::CanProgress(V,Scene,false);};
 TestFalse(TEXT("Setup is not a result"),Ready());
 TestTrue(TEXT("Overlay yields during production"),L.YieldsToProduction(Scene,false,V.InteractionCategory));
 const auto Before=L.GetStep(); Scene.Skip(); TestEqual(TEXT("Production skip cannot advance Lesson"),L.GetStep(),Before);
 Scene.Tick(Scene.PhaseSeconds()); TestFalse(TEXT("FormulaHold is not result completion"),Ready());
 Facts.Outcome=EOutcome::DefensiveSuccess; Scene.Sync(Facts);
 TestFalse(TEXT("Disclosed facts alone cannot interrupt Outcome"),Ready());
 Scene.Tick(Scene.PhaseSeconds()); TestTrue(TEXT("Disclosed failure in ResultHold is ready"),Ready());
 TestFalse(TEXT("Mandatory reel remains a gate"),Lesson::CanProgress(V,Scene,true));
 V.AttackSequence=8; TestFalse(TEXT("Old event cannot complete current lesson"),Ready()); V.AttackSequence=7;
 Scene={}; Facts.Outcome=EOutcome::None; Scene.Sync(Facts); Scene.Tick(1.f);
 Facts.Outcome=EOutcome::Goal; Scene.Sync(Facts); Scene.Tick(Scene.PhaseSeconds());
 TestTrue(TEXT("Natural production Goal celebrates"),Scene.Celebration.IsActive());
 TestFalse(TEXT("Summary waits past spatial ResultHold for celebration"),Ready());
 Scene.Tick(FMCodexGoalCelebration::Duration); TestTrue(TEXT("Summary allowed after celebration"),Ready());
 Scene.Sync(Facts); TestFalse(TEXT("Duplicate result cannot replay celebration"),Scene.Celebration.IsActive());
 FLessonWorld F; auto* Screen=NewObject<UFMCodexLocalMatchScreenWidget>(F.World); Screen->TakeWidget();
 Screen->ResetPresentationSession();
 TestEqual(TEXT("New screen scene reset"),Screen->GetTacticalScene().Phase,EPhase::Hidden);
 TestFalse(TEXT("New screen has no celebration"),Screen->GetTacticalScene().Celebration.bConsumed);
 TestFalse(TEXT("New screen has no stale reveal"),Screen->IsInlineFormulaRevealInputBlocked());
 L.EnterComparison(); TestEqual(TEXT("Rewind starts explicit comparison inspection"),L.GetStep(),EFMCodexLesson1Step::InspectOdegaard);
 TestFalse(TEXT("Old row not pinned across rewind"),L.KeepsFullCardOpen());
 return true;
}
#endif
