#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "FMCodexGuidedLesson1.h"
#include "FMCodexLocalMatchHostGameMode.h"
#include "FMCodexLocalMatchDemoConfiguration.h"
#include "FMCodexLocalMatchResolutionFeedback.h"
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
		if (L) { L->Update(View(), true, 2.f); L->bDispatchingOpponent = Script; }
		const bool Ok = Host->SubmitPlayerIntent(FMatchPlayPlayerIntent::Create(K,R)).bSuccess;
		if (L) { L->bDispatchingOpponent = false; if (ObserveAfter) Observe(); }
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
	TestTrue(TEXT("Range explanation"),L->Instruction().ToString().Contains(TEXT("远射 3–5")));
	TestTrue(TEXT("Range explanation is modal"),L->IsExplanationMode());
	L->Primary();
	TestEqual(TEXT("Deployment follows explanation"),L->GetStep(),EFMCodexLesson1Step::Deploy);
	auto Invalid=F.Request<FMatchPlayAuthoritativeDeployOrdinaryRequest>(); Invalid.CardId=Lesson::Odegaard(); Invalid.SlotId=Lesson::AttackerSlot();
	TestFalse(TEXT("Odegaard not yet playable"),F.Submit(Kind::DeployOrdinary,Invalid));
	Invalid.CardId=Lesson::Gyokeres(); Invalid.SlotId=TEXT("Demo.Slot.NearB.01");
	TestFalse(TEXT("Other cell gated without changing production legality"),F.Submit(Kind::DeployOrdinary,Invalid));
	if (!F.Attack(*this,Lesson::Gyokeres())) return false;
	F.Observe(false);
	TestFalse(TEXT("Rewind waits for visible result"),L->GetStep()==EFMCodexLesson1Step::Rewind);
	F.Observe();
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
	TestTrue(TEXT("Formal-match warning"),L->Explanation().ToString().Contains(TEXT("正式比赛")));
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
	L->EnterComparison(); L->Update(F.View(),true,12.f); TestEqual(TEXT("Hint 1"),L->GetHintLevel(),1);
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
	Lesson L;
	FFMCodexMatchScreenRequest Roll; Roll.Kind=Gesture::TacticalPoints;
	FFMCodexMatchScreenRequest Deploy; Deploy.Kind=Gesture::DeployOrdinary; Deploy.OptionId=Lesson::Gyokeres(); Deploy.SlotId=Lesson::AttackerSlot();
	TestTrue(TEXT("Intro is explanatory"),L.IsExplanationMode());
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
	L.Primary();
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
	L.Update(V,true,.1f); L.Primary();
	FFMCodexMatchScreenRequest Method; Method.Kind=Gesture::Branch; Method.Branch=EFMCodexUMGBranchIntent::DirectShot;
	TestTrue(TEXT("Direct shot allowed"),L.AllowsScreen(Method));
	Method.Branch=EFMCodexUMGBranchIntent::DeadCorner; TestFalse(TEXT("Other method blocked"),L.AllowsScreen(Method));
	TestTrue(TEXT("No old secondary method lecture"),L.Explanation().IsEmpty());
	return true;
}
#endif
