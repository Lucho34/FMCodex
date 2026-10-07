#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "FMCodexGuidedMatchContent.h"
#include "FMCodexGuidedLesson1.h"
#include "FMCodexPrototypeTeamContent.h"
#include "Dom/JsonObject.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGuidedContentEquivalenceTest,"FMCodex.LocalPlay.GuidedContent.MigrationEquivalence",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FGuidedContentEquivalenceTest::RunTest(const FString&)
{
 FString Error; const auto Content=FFMCodexGuidedMatchContent::Load(Error);
 if(!TestTrue(TEXT("Canonical generated content loads: ")+Error,Content.IsValid())) return false;
 FFMCodexGuidedLesson1 L(Content);
 TestTrue(TEXT("Production player bindings available"),L.IsContentReady());
 TestEqual(TEXT("Accepted intro heading"),L.Heading().ToString(),FString(TEXT("进行一次远射")));
 TestEqual(TEXT("Accepted intro body"),L.Instruction().ToString(),FString(TEXT("先掷出本回合的进攻战术点。")));
 TestEqual(TEXT("Per-glyph span selection remains canonical"),L.EmphasizeKeywords(L.Instruction()).ToString(),FString(TEXT("先掷出本回合的<concept>进攻战术点</>。")));
 L.Primary(); TestEqual(TEXT("TP instruction"),L.Instruction().ToString(),FString(TEXT("掷出本回合的进攻战术点。")));
 FFMCodexLocalMatchInteractionView View; View.ActionPoint=3; View.InteractionCategory=EFMCodexLocalMatchInteractionCategory::Deploy;
 View.ExpectedActingPlayer=EInitialTurnOrderPlayer::PlayerA;
 L.Update(View,true,.1f); L.Primary(); L.InspectCard(L.Gyokeres(),true);
 TestEqual(TEXT("Skill range comes from production catalog and live TP"),L.Instruction().ToString(),FString(TEXT("远射技能范围为 3–5。当前进攻战术点为 3，落在范围内，因此可以使用远射。")));
 const auto* Lesson=Content->FindLesson(TEXT("Lesson01"));
 TestEqual(TEXT("End Deployment copy"),Lesson->Steps[TEXT("FinishDeployment")].Body,FString(TEXT("点击“结束部署”，结束本次部署。")));
 FString Rendered;
 const auto& Finish=Lesson->Steps[TEXT("FinishDeployment")];
 TestTrue(TEXT("Explicit occurrence renders"),FFMCodexGuidedMatchContent::Render(Finish.Body,Finish.Emphasis,TEXT("BodyCN"),{},Rendered,Error));
 TestEqual(TEXT("Trailing generic word receives no implicit emphasis"),Rendered,FString(TEXT("点击“<concept>结束部署</>”，结束本次部署。")));
 TestEqual(TEXT("Direct method copy"),Lesson->Steps[TEXT("DirectShot")].Body,FString(TEXT("选择“直接射门”")));
 TestTrue(TEXT("Formula explanatory sentence retained"),Lesson->Steps[TEXT("FormulaExplanation")].Body.Contains(TEXT("公式总值由属性值与掷骰值相加得到。")));
 TestEqual(TEXT("Rewind CTA"),Lesson->Steps[TEXT("Rewind")].CTA,FString(TEXT("让时间倒流，换个人试试")));
 TestEqual(TEXT("Carrier uses one dynamic accepted sentence"),Lesson->Steps[TEXT("Carrier")].Body,FString(TEXT("点击场上的{Carrier.DisplayName}，将他选中为本进攻回合的持球队员。")));
 TestEqual(TEXT("Failure delay"),L.Timing(TEXT("FailureFollowupDelay")),1.6f);
 TestEqual(TEXT("Proxy duration"),L.Timing(TEXT("OpponentDeployMove")),.8f);
 TestEqual(TEXT("Final board hold"),L.Timing(TEXT("OpponentDeploySettledHold")),1.2f);
 return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGuidedContentBindingTest,"FMCodex.LocalPlay.GuidedContent.PresentationBinding",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FGuidedContentBindingTest::RunTest(const FString&)
{
 FString Json,Error; FFileHelper::LoadFileToString(Json,*FFMCodexGuidedMatchContent::RuntimePath());
 auto ParseObject=[&]() { TSharedPtr<FJsonObject> Root; FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Json),Root); return Root; };
 auto Encode=[](const TSharedPtr<FJsonObject>& Root) { FString Text; FJsonSerializer::Serialize(Root.ToSharedRef(),TJsonWriterFactory<>::Create(&Text)); return Text; };
 auto Root=ParseObject(); if(!TestTrue(TEXT("Source JSON available"),Root.IsValid())) return false;
 auto Steps=Root->GetArrayField(TEXT("steps"));
 auto StepObject=[&](const TCHAR* Id) { return (*Steps.FindByPredicate([&](const auto& V){return V->AsObject()->GetStringField(TEXT("StepId"))==Id;}))->AsObject(); };
 StepObject(TEXT("Intro"))->SetStringField(TEXT("SurfaceType"),TEXT("CompactActionBar"));
 StepObject(TEXT("Intro"))->SetStringField(TEXT("FocusTargetId"),TEXT("Button.TacticalPoint"));
 StepObject(TEXT("Intro"))->SetStringField(TEXT("ArrowPlacement"),TEXT("Side"));
 StepObject(TEXT("Intro"))->SetBoolField(TEXT("ShowExit"),false);
 for(const auto& V:Root->GetArrayField(TEXT("timings"))) if(V->AsObject()->GetStringField(TEXT("TimingId"))==TEXT("OpponentDeployMove")) V->AsObject()->SetNumberField(TEXT("Value"),.9);
 auto Content=FFMCodexGuidedMatchContent::Parse(Encode(Root),Error);
 if(!TestTrue(TEXT("In-memory edited config validates"),Content.IsValid())) return false;
 FFMCodexGuidedLesson1 L(Content);
 TestEqual(TEXT("Code maps stable StepId"),L.ContentStepId(),FString(TEXT("Intro")));
 TestFalse(TEXT("Surface consumed from config"),L.IsExplanationMode());
 TestEqual(TEXT("Semantic target consumed from config"),L.FocusTarget(),EFMCodexGuideTarget::TacticPoint);
 TestEqual(TEXT("Arrow preference consumed"),L.Presentation().Arrow,EFMCodexGuideArrow::Side);
 TestFalse(TEXT("Tutorial display flag consumed"),L.Presentation().bShowExit);
 TestEqual(TEXT("Pacing consumed from edited content"),L.Timing(TEXT("OpponentDeployMove")),.9f);
 FFMCodexLocalMatchInteractionView V; V.InteractionCategory=EFMCodexLocalMatchInteractionCategory::TacticalPointRoll;
 L.Update(V,true,100.f);
 TestEqual(TEXT("Surface configuration cannot advance code-owned acknowledgement"),L.GetStep(),EFMCodexLesson1Step::Intro);
 L.Primary(); TestEqual(TEXT("Original explicit transition retained"),L.GetStep(),EFMCodexLesson1Step::TacticPoint);
 using Var=EFMCodexGuideVariable;
 TMap<Var,FString> Bindings={{Var::CarrierName,TEXT("测试球员")},{Var::CarrierShooting,TEXT("5")}};
 FString Text;
 TestTrue(TEXT("Whitelist interpolation responds to production binding values"),FFMCodexGuidedMatchContent::Interpolate(TEXT("{Carrier.DisplayName}的射门 {Carrier.Shooting}"),Bindings,Text,Error));
 TestEqual(TEXT("No stale numeric copy"),Text,FString(TEXT("测试球员的射门 5")));
 TestFalse(TEXT("Missing binding rejected"),FFMCodexGuidedMatchContent::Interpolate(TEXT("{CurrentAttackTP}"),Bindings,Text,Error));
 TestFalse(TEXT("Unknown placeholder rejected"),FFMCodexGuidedMatchContent::Interpolate(TEXT("{Unknown.Field}"),Bindings,Text,Error));
 TArray<FFMCodexGuideEmphasis> Spans={{TEXT("BodyCN"),TEXT("{Carrier.DisplayName}"),2}};
 TestTrue(TEXT("Occurrence belongs to template, not repeated interpolated text"),FFMCodexGuidedMatchContent::Render(TEXT("{Carrier.DisplayName}，{Carrier.DisplayName}"),Spans,TEXT("BodyCN"),Bindings,Text,Error));
 TestEqual(TEXT("Only explicit second occurrence"),Text,FString(TEXT("测试球员，<concept>测试球员</>")));
 TestNull(TEXT("Missing step returns no fallback"),Content->FindStep(TEXT("Lesson01"),TEXT("Missing"),Error));
 TestTrue(TEXT("Missing step names offending key"),Error.Contains(TEXT("Missing")));
 Root=ParseObject(); Steps=Root->GetArrayField(TEXT("steps")); Steps.Pop(); Root->SetArrayField(TEXT("steps"),Steps);
 TestFalse(TEXT("Incomplete catalog rejected before publication"),FFMCodexGuidedMatchContent::Parse(Encode(Root),Error).IsValid());
 Root=ParseObject(); Steps=Root->GetArrayField(TEXT("steps"));
 const auto DuplicateStep=Steps[0]; Steps.Add(DuplicateStep); Root->SetArrayField(TEXT("steps"),Steps);
 TestFalse(TEXT("Runtime duplicate rejection"),FFMCodexGuidedMatchContent::Parse(Encode(Root),Error).IsValid());
 for(const auto& Pair:{TPair<FString,FString>(TEXT("SurfaceType"),TEXT("Unknown")),{TEXT("FocusTargetId"),TEXT("Unknown")},{TEXT("BodyCN"),TEXT("{Unknown.Field}")}})
 {
  Root=ParseObject(); Root->GetArrayField(TEXT("steps"))[0]->AsObject()->SetStringField(Pair.Key,Pair.Value);
  TestFalse(TEXT("Runtime rejects malformed field: ")+Pair.Key,FFMCodexGuidedMatchContent::Parse(Encode(Root),Error).IsValid());
 }
 return true;
}
#endif
