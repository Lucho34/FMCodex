#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && !UE_BUILD_SHIPPING
#include "FMCodexTacticalScene.h"
#include "FMCodexMatchShellStyle.h"
#include "FMCodexResolutionTheaterPrototype.h"
#include "FMCodexLocalMatchInteractionView.h"
#include "FMCodexLocalMatchResolutionFeedback.h"
#include "Serialization/MemoryWriter.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
#include "Misc/AutomationTest.h"
using namespace FMCodexTacticalScene;
namespace
{
FFMCodexUMGMatchScreenViewModel SceneFixture(bool Keeper, EInitialTurnOrderPlayer Attacker)
{
 FFMCodexLocalMatchInteractionView V;
 V.bCurrentAttackActive=true; V.AttackSequence=11;
 V.CurrentAttackingPlayer=Attacker; V.ExpectedActingPlayer=Attacker;
 V.SelectedCarrierCardId=TEXT("Prototype.Arsenal.DeclanRice");
 V.SelectedMarkerCardId=TEXT("Prototype.ManchesterCity.RubenDias");
 FFMCodexLocalMatchCardView GK;GK.CardId=TEXT("Prototype.ManchesterCity.GianluigiDonnarumma");GK.bGoalkeeper=true;
 (Attacker==EInitialTurnOrderPlayer::PlayerA?V.PlayerBCardRoster:V.PlayerACardRoster).Add(GK);
 V.PresentedActionType=ESkillRuleType::LongShot;
 V.MajorPhase=EFMCodexLocalMatchMajorPhase::Resolution;
 V.InteractionCategory=EFMCodexLocalMatchInteractionCategory::RollLongShotDirectAttack;
 auto& F=V.ResolutionFacts;F.bSuccess=true;F.bHasFacts=true;F.AttackSequence=11;
 F.ActionType=ESkillRuleType::LongShot;F.bHasActualBranch=true;
 F.ActualBranch.ActionType=ESkillRuleType::LongShot;F.ActualBranch.LongShot=EMatchPlayLongShotActualBranch::DirectShot;
 FMatchPlayResolutionFormulaContestFact C;C.ContestId=TEXT("LongShot.DirectShot");
 C.AttackRow.Side=Attacker;C.DefenseRow.Side=Attacker==EInitialTurnOrderPlayer::PlayerA?EInitialTurnOrderPlayer::PlayerB:EInitialTurnOrderPlayer::PlayerA;
 auto Add=[](auto& Row,FName Id,EMatchPlayResolutionParticipantRole Role,EMatchPlayResolutionFormulaAttribute Attribute)
 {
  FMatchPlayResolutionFormulaTermFact T;T.CardId=Id;T.Side=Row.Side;T.ParticipantRole=Role;T.Attribute=Attribute;
  T.Kind=Role==EMatchPlayResolutionParticipantRole::Goalkeeper?EMatchPlayResolutionFormulaTermKind::GoalkeeperContribution:EMatchPlayResolutionFormulaTermKind::Attribute;
  T.SourceValue=4;T.Contribution=4;Row.Terms.Add(T);
 };
 Add(C.AttackRow,TEXT("Prototype.Arsenal.DeclanRice"),EMatchPlayResolutionParticipantRole::Carrier,EMatchPlayResolutionFormulaAttribute::Shooting);
 Add(C.DefenseRow,TEXT("Prototype.ManchesterCity.RubenDias"),EMatchPlayResolutionParticipantRole::Marker,EMatchPlayResolutionFormulaAttribute::Defense);
 if(Keeper) Add(C.DefenseRow,TEXT("Prototype.ManchesterCity.GianluigiDonnarumma"),EMatchPlayResolutionParticipantRole::Goalkeeper,EMatchPlayResolutionFormulaAttribute::GoalkeeperPositioning);
 int32 Sequence=0;
 for(auto* Row:{&C.AttackRow,&C.DefenseRow})
 {
  FMatchPlayResolutionRollFact Roll;Roll.SequenceIndex=Sequence;Roll.OwningSide=Row->Side;
  Roll.Semantics=EMatchPlayResolutionRollSemantics::ArithmeticContest;
  Roll.PostRoutePurpose=Sequence==0?EMatchPlayCurrentAttackPostRouteRollPurpose::PrimaryAttack:EMatchPlayCurrentAttackPostRouteRollPurpose::PrimaryDefense;
  F.Rolls.Add(Roll);
  FMatchPlayResolutionFormulaTermFact Term;Term.Kind=EMatchPlayResolutionFormulaTermKind::RawRoll;Term.RollSequenceIndex=Sequence++;
  Row->Terms.Add(Term);
 }
 F.FormulaContests.Add(C);
 return FFMCodexLocalMatchUMGPresentationBuilder::Build(V,FFMCodexLocalMatchResolutionFeedback(),FString());
}
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSceneFactsTest,"FMCodex.LocalPlay.TacticalScene.LongShotSceneVisualFacts",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FSceneFactsTest::RunTest(const FString&)
{
 for(auto Side:{EInitialTurnOrderPlayer::PlayerA,EInitialTurnOrderPlayer::PlayerB}) for(bool Keeper:{false,true})
 {
  auto M=SceneFixture(Keeper,Side);
  M.Header.LeftAttackTurnTracker.PrimarySideColor=FLinearColor(.4f,.1f,.6f);
  M.Header.RightAttackTurnTracker.PrimarySideColor=FLinearColor(.8f,.4f,.1f);
  // Reflection serialization is the same boundary used by the nested network presentation DTO.
  TArray<uint8> Bytes;FMemoryWriter Writer(Bytes);FObjectAndNameAsStringProxyArchive Save(Writer,false);
  FFMCodexUMGMatchScreenViewModel::StaticStruct()->SerializeItem(Save,&M,nullptr);
  TestFalse(TEXT("Safe model serializes"),Save.IsError());
  FFMCodexUMGMatchScreenViewModel Copy;
  FMemoryReader Reader(Bytes);FObjectAndNameAsStringProxyArchive Load(Reader,true);
  FFMCodexUMGMatchScreenViewModel::StaticStruct()->SerializeItem(Load,&Copy,nullptr);
  TestFalse(TEXT("Safe model roundtrip"),Load.IsError());
  const auto F=Project(Copy.LongShotResolution,Copy.Header,true);
  TestTrue(TEXT("LongShot enabled"),F.bActive);
  TestEqual(TEXT("Spatial GK is present even without formula activation"),F.Participants.Num(),3);
  TestEqual(TEXT("Only actual formula participants"),F.Participants.FilterByPredicate([](const auto& P){return P.bFormulaActive;}).Num(),Keeper?3:2);
  TestEqual(TEXT("Formula defense unchanged"),Copy.LongShotResolution.Formula.DefenseRow.Participants.Num(),Keeper?2:1);
  TestTrue(TEXT("No invented runner or helper"),!F.Participants.ContainsByPredicate([](const auto& P){return P.Role==EMatchPlayResolutionParticipantRole::Runner || P.Role==EMatchPlayResolutionParticipantRole::Helper;}));
  if(F.Participants.Num()<2) continue;
  TestEqual(TEXT("Identity survives projection and serialization"),F.Participants[0].CardId,FName(TEXT("Prototype.Arsenal.DeclanRice")));
  TestEqual(TEXT("Marker role survives"),F.Participants[1].Role,EMatchPlayResolutionParticipantRole::Marker);
  const auto Expected=Side==Copy.Header.LeftPlayerSide?Copy.Header.LeftAttackTurnTracker.PrimarySideColor:Copy.Header.RightAttackTurnTracker.PrimarySideColor;
  TestTrue(TEXT("Accent follows ownership, not attack direction"),F.Participants[0].Accent.Equals(Expected));
  TestTrue(TEXT("Both sides attack right"),CarrierAnchor.X<MarkerAnchor.X && MarkerAnchor.X<GoalAnchor.X);
  TestFalse(TEXT("Lesson / disabled surface excluded"),Project(Copy.LongShotResolution,Copy.Header,false).bActive);
  TArray<uint8> After;FMemoryWriter AfterWriter(After);FObjectAndNameAsStringProxyArchive AfterSave(AfterWriter,false);
  FFMCodexUMGMatchScreenViewModel::StaticStruct()->SerializeItem(AfterSave,&M,nullptr);
  TestTrue(TEXT("Presentation leaves input untouched"),After==Bytes);
  Copy.LongShotResolution.SkillType=ESkillRuleType::CutInsideShot;
  TestFalse(TEXT("Other tactics untouched"),Project(Copy.LongShotResolution,Copy.Header,true).bActive);
 }
 return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSceneStoryboardTest,"FMCodex.LocalPlay.TacticalScene.LongShotStoryboard",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FSceneStoryboardTest::RunTest(const FString&)
{
 for(auto Outcome:{EMatchPlayResolutionDecisionOutcome::Goal,EMatchPlayResolutionDecisionOutcome::Miss,EMatchPlayResolutionDecisionOutcome::ImmediateMiss})
 {
  auto M=SceneFixture(false,EInitialTurnOrderPlayer::PlayerA);auto& V=M.LongShotResolution.Formula;
  FState S;S.Sync(Project(M.LongShotResolution,M.Header,true));
  TestTrue(TEXT("Setup starts once"),S.Phase==EPhase::Setup);
  S.Tick(SetupSeconds*.5f);S.Sync(Project(M.LongShotResolution,M.Header,true));TestEqual(TEXT("Repeated view does not reset"),S.Elapsed,SetupSeconds*.5f);
  S.Skip();TestTrue(TEXT("Skip only advances to intent"),S.Phase==EPhase::Intent);
  S.Tick(IntentSeconds);TestTrue(TEXT("Intent holds for formula"),S.Phase==EPhase::FormulaHold);
  TestFalse(TEXT("Skip cannot bypass formula"),S.Skip());
  V.bDefenseRowActive=true;V.bAttackRowActive=false;V.bShowFormulaRows=true;
  S.Sync(Project(M.LongShotResolution,M.Header,true));
  TestEqual(TEXT("Defense reveal links to marker"),S.Facts.Highlight,EMatchPlayResolutionParticipantRole::Marker);
  const auto* GK=S.Facts.Participants.FindByPredicate([](const auto& P){return P.Role==EMatchPlayResolutionParticipantRole::Goalkeeper;});
  TestTrue(TEXT("Visual-only GK excluded from formula emphasis"),GK && !GK->bFormulaActive);
  V.NarrativeHeadline=TEXT("已授权的远射结果");
  V.SpatialOutcome=Outcome;V.bNarrativeAvailable=true;V.bDiceRevealVisible=true;
  S.Sync(Project(M.LongShotResolution,M.Header,true));TestTrue(TEXT("Disclosed server result does not leak through active reel"),S.Facts.Outcome==EOutcome::None);
  V.bDiceRevealVisible=false;S.Sync(Project(M.LongShotResolution,M.Header,true));
  TestTrue(TEXT("Outcome follows formula"),S.Phase==EPhase::Outcome);
  TestTrue(TEXT("Typed decision maps without arithmetic"),S.Facts.Outcome==(Outcome==EMatchPlayResolutionDecisionOutcome::Goal?EOutcome::Goal:Outcome==EMatchPlayResolutionDecisionOutcome::Miss?EOutcome::DefensiveSuccess:EOutcome::ImmediateMiss));
  auto Gated=V;S.Gate(Gated);TestFalse(TEXT("Terminal text waits for ball"),Gated.bNarrativeAvailable);
  TestEqual(TEXT("Formula values unchanged"),Gated.AttackRow.FinalValue,V.AttackRow.FinalValue);
  S.Skip();TestTrue(TEXT("Outcome skip finishes locally"),S.Phase==EPhase::ResultHold);
  for(auto Method:{EMethod::Direct,EMethod::DeadCorner})
   TestEqual(TEXT("Goal safely inside mouth; misses clearly outside"),IsInsideGoal(OutcomeTarget(S.Facts.Outcome,Method)),Outcome==EMatchPlayResolutionDecisionOutcome::Goal);
  auto Final=V;S.Gate(Final);TestEqual(TEXT("Final narrative is original authoritative text"),Final.NarrativeHeadline,V.NarrativeHeadline);
  S.Sync(Project(M.LongShotResolution,M.Header,true));TestTrue(TEXT("Duplicate result does not replay"),S.Phase==EPhase::ResultHold);
  FState Reopened;Reopened.Sync(Project(M.LongShotResolution,M.Header,true));TestTrue(TEXT("Reconstructed resolved state snaps"),Reopened.Phase==EPhase::ResultHold);
  TestFalse(TEXT("Result skip does not continue gameplay"),S.Skip());
 }
 return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSceneMethodTest,"FMCodex.LocalPlay.TacticalScene.LongShotMethodPreviewAndFlow",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FSceneMethodTest::RunTest(const FString&)
{
 FFMCodexLocalMatchInteractionView V;
 V.bCurrentAttackActive=true;V.AttackSequence=12;V.CurrentAttackingPlayer=EInitialTurnOrderPlayer::PlayerA;
 V.ExpectedActingPlayer=EInitialTurnOrderPlayer::PlayerA;V.PresentedActionType=ESkillRuleType::LongShot;
 V.SelectedCarrierCardId=TEXT("Prototype.Arsenal.DeclanRice");V.SelectedMarkerCardId=TEXT("Prototype.ManchesterCity.RubenDias");
 V.InteractionCategory=EFMCodexLocalMatchInteractionCategory::SelectLongShotBranch;
 V.BranchIntentOptions={EMatchPlayElectiveBranchIntent::DirectShot,EMatchPlayElectiveBranchIntent::DeadCorner};
 auto M=FFMCodexLocalMatchUMGPresentationBuilder::Build(V,FFMCodexLocalMatchResolutionFeedback(),FString());
 FState S;S.Sync(Project(M.LongShotResolution,M.Header,true));
 TestEqual(TEXT("Method choice opens persistent scene"),S.Phase,EPhase::Preview);
 TestFalse(TEXT("Preview does not block actual method selection"),S.IsAnimating());
 S.Preview(EMethod::DeadCorner);S.Tick(.15f);
 TestEqual(TEXT("Corner hover/focus transitions in 150ms"),S.CornerBlend,1.f);
 S.Sync(Project(M.LongShotResolution,M.Header,true));
 TestEqual(TEXT("Repeated view retains hovered method"),S.PreviewMethod,EMethod::DeadCorner);
 S.Preview(EMethod::Direct);S.Tick(.15f);TestEqual(TEXT("Direct preview returns to normal target"),S.CornerBlend,0.f);
 TestEqual(TEXT("Preview has no method intent mutation"),V.ElectiveBranchIntent,EMatchPlayElectiveBranchIntent::None);
 TestEqual(TEXT("No future outcome in preview"),S.Facts.Outcome,EOutcome::None);
 TestFalse(TEXT("Space cannot commit preview"),S.Skip());
 // Production must provide a successful safe projection; intent/category alone
 // cannot substitute for missing or failed authoritative facts.
 V.InteractionCategory=EFMCodexLocalMatchInteractionCategory::RollLongShotDeadCorner;
 V.ElectiveBranchIntent=EMatchPlayElectiveBranchIntent::DeadCorner;
 V.ResolutionFacts.bSuccess=V.ResolutionFacts.bHasFacts=true;
 V.ResolutionFacts.bHasActualBranch=true;
 V.ResolutionFacts.ActualBranch.ActionType=ESkillRuleType::LongShot;
 V.ResolutionFacts.ActualBranch.LongShot=EMatchPlayLongShotActualBranch::DeadCorner;
 M=FFMCodexLocalMatchUMGPresentationBuilder::Build(V,FFMCodexLocalMatchResolutionFeedback(),FString());
 TestEqual(TEXT("Accepted pending pair owns DeadCorner surface"),M.LongShotResolution.Stage,EFMCodexUMGLongShotStage::DeadCorner);
 TestTrue(TEXT("Valid safe facts own commercial Theater"),M.LongShotResolution.bSuppressLegacyResolution && FMCodexResolutionTheaterPrototype::WantsTheater(M,M.InlineFormula));
 TestFalse(TEXT("No invented attribute formula"),M.LongShotResolution.Formula.bVisible);
 S.Sync(Project(M.LongShotResolution,M.Header,true));TestEqual(TEXT("Commit starts local storyboard"),S.Phase,EPhase::Setup);
 S.Skip();S.Skip();
 V.InteractionCategory=EFMCodexLocalMatchInteractionCategory::AdvanceAfterTerminal;
 V.bTerminalPendingAdvance=true;
 FMatchPlayResolutionDecisionFact Decision;
 Decision.DecisionId=TEXT("DeadCorner.Outcome");Decision.bResolved=true;Decision.Outcome=EMatchPlayResolutionDecisionOutcome::Goal;
 V.ResolutionFacts.Decisions={Decision};
 M=FFMCodexLocalMatchUMGPresentationBuilder::Build(V,FFMCodexLocalMatchResolutionFeedback(),FString());
 TestTrue(TEXT("Valid terminal retains commercial ownership"),M.LongShotResolution.bVisible && M.LongShotResolution.bSuppressLegacyResolution);
 TestEqual(TEXT("Terminal preserves typed procedural outcome"),M.LongShotResolution.Formula.SpatialOutcome,Decision.Outcome);
 M.LongShotResolution.Formula.SpatialOutcome=EMatchPlayResolutionDecisionOutcome::Goal;
 M.LongShotResolution.bNarrativeAvailable=true;M.LongShotResolution.bDiceRevealVisible=true;
 S.Sync(Project(M.LongShotResolution,M.Header,true));TestEqual(TEXT("Sequential pair still holds outcome"),S.Facts.Outcome,EOutcome::None);
 M.LongShotResolution.bDiceRevealVisible=false;
 S.Sync(Project(M.LongShotResolution,M.Header,true));TestEqual(TEXT("Procedural goal uses same scene"),S.Phase,EPhase::Outcome);
 auto Gated=M.LongShotResolution;S.Gate(Gated);TestFalse(TEXT("Outer procedural narrative gated too"),Gated.bNarrativeAvailable);
 TestFalse(TEXT("Procedural continuation waits for spatial result"),Gated.PrimaryAction.bVisible);
 V.ResolutionFacts.bSuccess=false;
 V.ResolutionFacts.ErrorMessage=TEXT("CardId must not be None.");
 const auto Invalid=FFMCodexLocalMatchUMGPresentationBuilder::Build(V,FFMCodexLocalMatchResolutionFeedback(),FString());
 TestFalse(TEXT("Failed authority projection must not be masked by intent or category"),Invalid.LongShotResolution.bVisible);
 return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSceneRhythmTest,"FMCodex.LocalPlay.TacticalScene.LongShotRhythmContinuity",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FSceneRhythmTest::RunTest(const FString&)
{
 FFacts F;F.bActive=true;F.AttackSequence=41;F.bMethodChoice=true;
 FState S;S.Sync(F);F.bMethodChoice=false;S.Sync(F);
 TestTrue(TEXT("Commit retains already-visible preview identities and intent"),S.bEnteredFromPreview);
 S.Tick(SetupSeconds+.07f);
 TestEqual(TEXT("Long frame advances to intent"),S.Phase,EPhase::Intent);
 TestTrue(TEXT("Overshoot is retained as actual elapsed time"),FMath::IsNearlyEqual(S.Elapsed,.07f));
 S.Sync(F);TestTrue(TEXT("Repeated view retains presentation continuity"),S.bEnteredFromPreview);
 S.Tick(4.f);
 TestEqual(TEXT("Even a stalled frame must stop for the real roll"),S.Phase,EPhase::FormulaHold);
 TestEqual(TEXT("No invented outcome at roll boundary"),S.Facts.Outcome,EOutcome::None);
 F.Outcome=EOutcome::Goal;S.Sync(F);
 S.Tick(OutcomeSeconds-.01f);
 FFMCodexUMGLongShotResolutionViewModel V;V.bNarrativeAvailable=true;V.Formula.bNarrativeAvailable=true;
 V.PrimaryAction.bVisible=true;S.Gate(V);
 TestFalse(TEXT("Terminal text remains gated until full spatial budget elapsed"),V.bNarrativeAvailable);
 TestFalse(TEXT("No early Continue while ball is arriving"),V.PrimaryAction.bVisible);
 S.Tick(.011f);TestEqual(TEXT("No extra hold after arrival"),S.Phase,EPhase::ResultHold);
 S.Sync(F);TestEqual(TEXT("Duplicate result never restarts ball flight"),S.Phase,EPhase::ResultHold);
 F.AttackSequence++;F.Outcome=EOutcome::None;S.Sync(F);
 TestFalse(TEXT("New attack cannot inherit preview continuity"),S.bEnteredFromPreview);
 const float Before=S.Elapsed;S.Tick(-1.f);TestEqual(TEXT("Negative delta never rewinds"),S.Elapsed,Before);
 S.Sync(FFacts{});TestFalse(TEXT("Session/surface reset clears presentation state"),S.bEnteredFromPreview);
 // Same wall time at different frame sizes reaches the same local boundary.
 for(float Step:{1.f/30.f,1.f/60.f,.1f})
 {
  FState R;R.Sync(F);
  const float Total=SetupSeconds+IntentSeconds+.03f;
  for(float T=0;T<Total;T+=Step) R.Tick(FMath::Min(Step,Total-T));
  TestEqual(TEXT("Pacing independent of frame partition"),R.Phase,EPhase::FormulaHold);
 }
 return true;
}
#endif
