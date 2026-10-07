#include "FMCodexGuidedMatchContent.h"
#if !UE_BUILD_SHIPPING
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace FMCodexGuideContentPrivate
{
const TMap<FString, EFMCodexGuideVariable> Variables = {
 {TEXT("Carrier.DisplayName"),EFMCodexGuideVariable::CarrierName}, {TEXT("Carrier.Shooting"),EFMCodexGuideVariable::CarrierShooting},
 {TEXT("FirstCarrier.DisplayName"),EFMCodexGuideVariable::FirstCarrierName}, {TEXT("FirstCarrier.Shooting"),EFMCodexGuideVariable::FirstCarrierShooting},
 {TEXT("ComparisonCarrier.DisplayName"),EFMCodexGuideVariable::ComparisonCarrierName}, {TEXT("ComparisonCarrier.Shooting"),EFMCodexGuideVariable::ComparisonCarrierShooting},
 {TEXT("Marker.DisplayName"),EFMCodexGuideVariable::MarkerName}, {TEXT("CurrentAttackTP"),EFMCodexGuideVariable::CurrentAttackTP},
 {TEXT("LongShot.SkillMin"),EFMCodexGuideVariable::SkillMin}, {TEXT("LongShot.SkillMax"),EFMCodexGuideVariable::SkillMax},
 {TEXT("LongShot.TraitName"),EFMCodexGuideVariable::TraitName}, {TEXT("Formula.AttackBaseValue"),EFMCodexGuideVariable::FormulaAttackBase}
};
const TMap<FString, EFMCodexGuideTarget> Targets = {
 {TEXT("None"),EFMCodexGuideTarget::None}, {TEXT("Button.TacticalPoint"),EFMCodexGuideTarget::TacticPoint},
 {TEXT("PlayerHand.Carrier"),EFMCodexGuideTarget::HandCard}, {TEXT("Deployment.Carrier"),EFMCodexGuideTarget::Deployment},
 {TEXT("Button.EndDeployment"),EFMCodexGuideTarget::FinishDeployment}, {TEXT("Field.Carrier"),EFMCodexGuideTarget::Carrier},
 {TEXT("Button.LongShot"),EFMCodexGuideTarget::Skill}, {TEXT("Theater.DirectShot"),EFMCodexGuideTarget::DirectShot},
 {TEXT("Formula.AttackBaseValue"),EFMCodexGuideTarget::FormulaValue}, {TEXT("Button.AttackRoll"),EFMCodexGuideTarget::AttackRoll},
 {TEXT("FullCard.Skill.LongShot"),EFMCodexGuideTarget::FullCardSkill}, {TEXT("FullCard.Attribute.SHO"),EFMCodexGuideTarget::FullCardShooting},
 {TEXT("FullCard.Trait.LongShotCarrier"),EFMCodexGuideTarget::FullCardTrait}, {TEXT("Opponent.Deployment"),EFMCodexGuideTarget::OpponentDeployment},
 {TEXT("Header.Opponent"),EFMCodexGuideTarget::OpponentStatus}, {TEXT("Field.Marker"),EFMCodexGuideTarget::OpponentMarker}
};
bool Fail(FString& Error, const FString& Message) { Error=TEXT("Guided content: ")+Message; return false; }
bool Identifier(const FString& S)
{
 if(S.IsEmpty() || !FChar::IsAlpha(S[0]) || S[0]>127) return false;
 for(TCHAR C:S) if(C>127 || (!FChar::IsAlnum(C) && C!='.')) return false;
 return true;
}
bool TemplateKeys(const FString& Text, TArray<FString>& Keys, FString& Error)
{
 for(int32 I=0;I<Text.Len();++I)
 {
  if(Text[I]=='{' )
  {
   const int32 End=Text.Find(TEXT("}"),ESearchCase::CaseSensitive,ESearchDir::FromStart,I+1);
   if(End==INDEX_NONE) return Fail(Error,TEXT("Unclosed placeholder: ")+Text);
   const FString Key=Text.Mid(I+1,End-I-1);
   if(!Variables.Contains(Key)) return Fail(Error,TEXT("Unknown placeholder: ")+Key);
   Keys.Add(Key); I=End;
  }
  else if(Text[I]=='}' || Text[I]=='<' || Text[I]=='>' || Text[I]=='&') return Fail(Error,TEXT("Malformed copy/markup: ")+Text);
 }
 return true;
}
bool SpanRange(const FString& Text,const FFMCodexGuideEmphasis& Span,FIntPoint& Out,FString& Error)
{
 if(Span.MatchText.IsEmpty() || Span.Occurrence<1 || Span.Occurrence>20) return Fail(Error,TEXT("Invalid emphasis occurrence"));
 int32 Start=0,Found=INDEX_NONE;
 for(int32 I=0;I<Span.Occurrence;++I)
 {
  Found=Text.Find(Span.MatchText,ESearchCase::CaseSensitive,ESearchDir::FromStart,Start);
  if(Found==INDEX_NONE) return Fail(Error,TEXT("Emphasis not found: ")+Span.MatchText);
  Start=Found+Span.MatchText.Len();
 }
 // An emphasis token can contain a whole placeholder, but cannot cut one.
 for(int32 I=0;I<Text.Len();++I) if(Text[I]=='{')
 {
  const int32 End=Text.Find(TEXT("}"),ESearchCase::CaseSensitive,ESearchDir::FromStart,I)+1;
  if(Found<End && Start>I && !(Found<=I && Start>=End)) return Fail(Error,TEXT("Emphasis cuts placeholder"));
  I=End-1;
 }
 Out=FIntPoint(Found,Start); return true;
}
bool Fields(const TSharedPtr<FJsonObject>& O,const TCHAR* Required,FString& Error)
{
 if(!O.IsValid()) return Fail(Error,TEXT("Expected object"));
 TArray<FString> Names; FString(Required).ParseIntoArrayWS(Names);
 if(O->Values.Num()!=Names.Num()) return Fail(Error,TEXT("Missing or unknown fields"));
 for(const auto& Name:Names) if(!O->HasField(Name)) return Fail(Error,TEXT("Missing field: ")+Name);
 return true;
}
bool ReadText(const TSharedPtr<FJsonObject>& O,const TCHAR* Key,FString& Out,FString& Error,bool Required=false)
{
 if(!O->TryGetStringField(Key,Out) || (Required && Out.IsEmpty())) return Fail(Error,FString(TEXT("Missing text: "))+Key);
 TArray<FString> Keys; return TemplateKeys(Out,Keys,Error);
}
bool ExactKeys(const TSet<FString>& Actual,const TCHAR* Expected,FString& Error)
{
 TArray<FString> Keys; FString(Expected).ParseIntoArrayWS(Keys);
 if(Keys.Num()!=Actual.Num()) return Fail(Error,TEXT("Lesson01 missing/unknown code binding"));
 for(const auto& Key:Keys) if(!Actual.Contains(Key)) return Fail(Error,TEXT("Missing Lesson01 binding: ")+Key);
 return true;
}
template<typename T> TSet<FString> KeysOf(const TMap<FString,T>& Map)
{ TSet<FString> Keys; for(const auto& Pair:Map) Keys.Add(Pair.Key); return Keys; }
}

FString FFMCodexGuidedMatchContent::RuntimePath() { return FPaths::Combine(FPaths::ProjectContentDir(),TEXT("Data/CanonicalGuidedMatchContent.json")); }
TSharedPtr<const FFMCodexGuidedMatchContent> FFMCodexGuidedMatchContent::Load(FString& Error)
{
 FString Json;
 if(!FFileHelper::LoadFileToString(Json,*RuntimePath())) { Error=TEXT("Cannot read canonical tutorial: ")+RuntimePath(); return nullptr; }
 return Parse(Json,Error);
}
bool FFMCodexGuidedMatchContent::Variable(const FString& Key,EFMCodexGuideVariable& Out)
{
 if(const auto* V=FMCodexGuideContentPrivate::Variables.Find(Key)) { Out=*V; return true; } return false;
}
TSharedPtr<const FFMCodexGuidedMatchContent> FFMCodexGuidedMatchContent::Parse(const FString& Json,FString& Error)
{
 using namespace FMCodexGuideContentPrivate;
 Error=TEXT("Invalid canonical tutorial field or binding");
 TSharedPtr<FJsonObject> Root;
 if(!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Json),Root) || !Fields(Root,TEXT("schemaVersion sourceWorkbook sourceWorkbookSha256 lessons steps emphasis timings labels"),Error))
 { Fail(Error,TEXT("Invalid JSON root/schema fields")); return nullptr; }
 double Version=0;
 if(!Root->TryGetNumberField(TEXT("schemaVersion"),Version) || Version!=1) { Fail(Error,TEXT("Unsupported schemaVersion")); return nullptr; }
 auto Result=MakeShared<FFMCodexGuidedMatchContent>();
 FString Source;
 if(!ReadText(Root,TEXT("sourceWorkbook"),Source,Error,true) || !ReadText(Root,TEXT("sourceWorkbookSha256"),Result->WorkbookHash,Error,true)) return nullptr;
 if(Result->WorkbookHash.Len()!=64) { Fail(Error,TEXT("Invalid workbook SHA256")); return nullptr; }
 const TArray<TSharedPtr<FJsonValue>> *LessonRows=nullptr,*StepRows=nullptr,*EmphasisRows=nullptr,*TimingRows=nullptr,*LabelRows=nullptr;
 if(!Root->TryGetArrayField(TEXT("lessons"),LessonRows) || !Root->TryGetArrayField(TEXT("steps"),StepRows) || !Root->TryGetArrayField(TEXT("emphasis"),EmphasisRows)
  || !Root->TryGetArrayField(TEXT("timings"),TimingRows) || !Root->TryGetArrayField(TEXT("labels"),LabelRows)) { Fail(Error,TEXT("Required arrays missing")); return nullptr; }
 for(const auto& V:*LessonRows)
 {
  const auto O=V->Type==EJson::Object?V->AsObject():nullptr;
  if(!Fields(O,TEXT("LessonId SchemaVersion Enabled TitleCN ComparisonTitleCN Notes"),Error)) return nullptr;
  FString Id,Notes; FFMCodexGuideLessonDefinition L;
  if(!ReadText(O,TEXT("LessonId"),Id,Error,true) || !Identifier(Id) || Result->Lessons.Contains(Id)
   || !O->TryGetNumberField(TEXT("SchemaVersion"),Version) || Version!=1 || !O->TryGetBoolField(TEXT("Enabled"),L.bEnabled)
   || !ReadText(O,TEXT("TitleCN"),L.Title,Error,true) || !ReadText(O,TEXT("ComparisonTitleCN"),L.ComparisonTitle,Error,true)
   || !ReadText(O,TEXT("Notes"),Notes,Error)) { Fail(Error,TEXT("Invalid/duplicate lesson: ")+Id); return nullptr; }
  Result->Lessons.Add(Id,MoveTemp(L));
 }
 const TSet<FString> Ack={TEXT("Intro"),TEXT("TacticPointExplanation"),TEXT("ShootingExplanation"),TEXT("SkillRangeExplanation"),TEXT("TraitExplanation"),TEXT("FinishExplanation"),
  TEXT("CarrierExplanation"),TEXT("SkillExplanation"),TEXT("DirectExplanation"),TEXT("FormulaExplanation"),TEXT("Rewind"),TEXT("Rewind.Unexpected"),TEXT("Summary"),TEXT("Summary.Unexpected"),TEXT("Complete")};
 for(const auto& V:*StepRows)
 {
  const auto O=V->Type==EJson::Object?V->AsObject():nullptr;
  if(!Fields(O,TEXT("LessonId StepId SurfaceType TitleCN BodyCN SecondaryCN CTA_CN FocusTargetId ArrowPlacement ShowExit PointerCN Notes"),Error)) return nullptr;
  FString Id,Surface,Target,Arrow,Notes; FFMCodexGuideStepPresentation P;
  if(!ReadText(O,TEXT("LessonId"),Id,Error,true) || !ReadText(O,TEXT("StepId"),P.StepId,Error,true)
   || !ReadText(O,TEXT("SurfaceType"),Surface,Error,true) || !ReadText(O,TEXT("FocusTargetId"),Target,Error,true)
   || !ReadText(O,TEXT("ArrowPlacement"),Arrow,Error,true) || !O->TryGetBoolField(TEXT("ShowExit"),P.bShowExit)
   || !ReadText(O,TEXT("TitleCN"),P.Title,Error,true) || !ReadText(O,TEXT("BodyCN"),P.Body,Error,P.StepId!=TEXT("FailurePause"))
   || !ReadText(O,TEXT("SecondaryCN"),P.Secondary,Error) || !ReadText(O,TEXT("CTA_CN"),P.CTA,Error)
   || !ReadText(O,TEXT("PointerCN"),P.Pointer,Error) || !ReadText(O,TEXT("Notes"),Notes,Error)) return nullptr;
  auto* L=Result->Lessons.Find(Id);
  if(!L || !Identifier(P.StepId) || L->Steps.Contains(P.StepId) || !Targets.Contains(Target)
   || (Surface!=TEXT("ExplanationPanel") && Surface!=TEXT("CompactActionBar"))
   || (Id==TEXT("Lesson01") && (Ack.Contains(P.StepId)==P.CTA.IsEmpty()))
   || (Id==TEXT("Lesson01") && !Ack.Contains(P.StepId) && Surface!=TEXT("CompactActionBar"))) { Fail(Error,TEXT("Invalid/duplicate step/surface/target/CTA: ")+P.StepId); return nullptr; }
  P.Surface=Surface==TEXT("ExplanationPanel")?EFMCodexGuideSurface::ExplanationPanel:EFMCodexGuideSurface::CompactActionBar;
  P.Target=Targets[Target];
  if(Arrow==TEXT("Auto")) P.Arrow=EFMCodexGuideArrow::Auto;
  else if(Arrow==TEXT("Above")) P.Arrow=EFMCodexGuideArrow::Above;
  else if(Arrow==TEXT("Side")) P.Arrow=EFMCodexGuideArrow::Side;
  else if(Arrow==TEXT("None")) P.Arrow=EFMCodexGuideArrow::None;
  else { Fail(Error,TEXT("Unknown arrow preference")); return nullptr; }
  TArray<FString> BoundKeys;
  for(const auto& Text:{P.Title,P.Body,P.Secondary,P.CTA,P.Pointer}) if(!TemplateKeys(Text,BoundKeys,Error)) return nullptr;
  if(Id==TEXT("Lesson01"))
  {
   const bool BeforeTP=P.StepId==TEXT("Intro") || P.StepId==TEXT("TacticPoint");
   const TSet<FString> FormulaSteps={TEXT("FormulaHover"),TEXT("FormulaExplanation"),TEXT("AttackRoll"),TEXT("OpponentDefense"),TEXT("ResultReveal"),TEXT("FailurePause"),TEXT("Rewind"),TEXT("Rewind.Unexpected"),TEXT("Summary"),TEXT("Summary.Unexpected"),TEXT("Complete")};
   if((BeforeTP && BoundKeys.Contains(TEXT("CurrentAttackTP"))) || (!FormulaSteps.Contains(P.StepId) && BoundKeys.Contains(TEXT("Formula.AttackBaseValue"))))
   { Fail(Error,TEXT("Binding unavailable at step: ")+P.StepId); return nullptr; }
  }
  const FString StepId=P.StepId; L->Steps.Add(StepId,MoveTemp(P));
 }
 for(const auto& V:*EmphasisRows)
 {
  const auto O=V->Type==EJson::Object?V->AsObject():nullptr;
  if(!Fields(O,TEXT("LessonId StepId Field MatchText Occurrence Style"),Error)) return nullptr;
  FString Id,StepId,Style; FFMCodexGuideEmphasis E; double Occurrence=0;
  if(!ReadText(O,TEXT("LessonId"),Id,Error,true) || !ReadText(O,TEXT("StepId"),StepId,Error,true)
   || !ReadText(O,TEXT("Field"),E.Field,Error,true) || !ReadText(O,TEXT("MatchText"),E.MatchText,Error,true)
   || !ReadText(O,TEXT("Style"),Style,Error,true) || !O->TryGetNumberField(TEXT("Occurrence"),Occurrence)
   || !FMath::IsFinite(Occurrence) || Occurrence<1 || Occurrence>20 || Occurrence!=FMath::FloorToDouble(Occurrence)) return nullptr;
  E.Occurrence=static_cast<int32>(Occurrence);
  auto* L=Result->Lessons.Find(Id); auto* P=L?L->Steps.Find(StepId):nullptr;
  if(!P || (E.Field!=TEXT("TitleCN") && E.Field!=TEXT("BodyCN")) || Style!=TEXT("PerGlyphDots")) { Fail(Error,TEXT("Invalid emphasis binding")); return nullptr; }
  const FString& Text=E.Field==TEXT("TitleCN")?P->Title:P->Body; FIntPoint Span;
  if(!SpanRange(Text,E,Span,Error)) return nullptr;
  for(const auto& Existing:P->Emphasis) if(Existing.Field==E.Field)
  { FIntPoint Other; if(!SpanRange(Text,Existing,Other,Error)) return nullptr;
    if(Span.X<Other.Y && Span.Y>Other.X) { Fail(Error,TEXT("Duplicate/overlapping emphasis")); return nullptr; } }
  P->Emphasis.Add(E);
 }
 for(const auto& V:*TimingRows)
 {
  const auto O=V->Type==EJson::Object?V->AsObject():nullptr;
  if(!Fields(O,TEXT("LessonId TimingId Value Notes"),Error)) return nullptr;
  FString Id,Key,Notes; double Value=0;
  if(!ReadText(O,TEXT("LessonId"),Id,Error,true) || !ReadText(O,TEXT("TimingId"),Key,Error,true) || !ReadText(O,TEXT("Notes"),Notes,Error)
   || !O->TryGetNumberField(TEXT("Value"),Value) || !FMath::IsFinite(Value) || Value<0 || Value>60) { Fail(Error,TEXT("Invalid timing")); return nullptr; }
  auto* L=Result->Lessons.Find(Id);
  if(!L || !Identifier(Key) || L->Timings.Contains(Key) || ((Key==TEXT("OpponentDeployMove") || Key==TEXT("ComparisonPace")) && (Value<.05 || Value>(Key==TEXT("ComparisonPace")?2:10))))
  { Fail(Error,TEXT("Invalid/duplicate timing: ")+Key); return nullptr; }
  L->Timings.Add(Key,static_cast<float>(Value));
 }
 for(const auto& V:*LabelRows)
 {
  const auto O=V->Type==EJson::Object?V->AsObject():nullptr;
  if(!Fields(O,TEXT("LessonId LabelId TextCN Notes"),Error)) return nullptr;
  FString Id,Key,Text,Notes;
  if(!ReadText(O,TEXT("LessonId"),Id,Error,true) || !ReadText(O,TEXT("LabelId"),Key,Error,true) || !ReadText(O,TEXT("TextCN"),Text,Error,true) || !ReadText(O,TEXT("Notes"),Notes,Error)) return nullptr;
  auto* L=Result->Lessons.Find(Id);
  if(!L || !Identifier(Key) || L->Labels.Contains(Key)) { Fail(Error,TEXT("Invalid/duplicate label: ")+Key); return nullptr; }
  TArray<FString> BoundKeys; TemplateKeys(Text,BoundKeys,Error);
  if(BoundKeys.Contains(TEXT("CurrentAttackTP")) || BoundKeys.Contains(TEXT("Formula.AttackBaseValue"))) { Fail(Error,TEXT("Global labels require static bindings")); return nullptr; }
  L->Labels.Add(Key,Text);
 }
 const auto* L=Result->Lessons.Find(TEXT("Lesson01"));
 if(!L || !L->bEnabled) { Fail(Error,TEXT("Enabled Lesson01 required")); return nullptr; }
 if(!ExactKeys(KeysOf(L->Steps),TEXT("Intro TacticPoint TacticPointExplanation InspectGyokeres InspectOdegaard ShootingExplanation SkillRangeExplanation TraitExplanation Deploy Deploy.Comparison Deploy.Hint1 Deploy.Hint2 OpponentDeploy OpponentDeploy.After FinishExplanation FinishDeployment OpponentFinish OpponentFinish.After CarrierExplanation Carrier OpponentMarker OpponentMarker.After SkillExplanation Skill DirectExplanation DirectShot FormulaHover FormulaExplanation AttackRoll OpponentDefense ResultReveal FailurePause Rewind Rewind.Unexpected RewindTransition Summary Summary.Unexpected Complete"),Error)
  || !ExactKeys(KeysOf(L->Timings),TEXT("OpponentPreAction OpponentDeployAttention OpponentDeployMove OpponentDeploySettledHold OpponentChoiceFocus OpponentFinishFocus OpponentDestinationHold OpponentSettledHold OpponentDefenseLead FailureFollowupDelay ComparisonPace RewindDelay HintFirstDelay HintSecondDelay FeedbackDuration"),Error)
  || !ExactKeys(KeysOf(L->Labels),TEXT("ExitTitle ExitBody ExitCancel ExitConfirm OpponentPointer LocalRack OpponentRack UnavailableChoice DeploymentTarget FeedbackCompare FeedbackUnavailable ProgressFirst ProgressComparison"),Error)) return nullptr;
 if(L->Timings[TEXT("HintFirstDelay")]>L->Timings[TEXT("HintSecondDelay")]) { Fail(Error,TEXT("Hint delays out of order")); return nullptr; }
 for(const auto& Pair:Result->Lessons)
 {
  TArray<FString> BoundKeys; TemplateKeys(Pair.Value.Title,BoundKeys,Error); TemplateKeys(Pair.Value.ComparisonTitle,BoundKeys,Error);
  if(!BoundKeys.IsEmpty()) { Fail(Error,TEXT("Lesson titles do not support dynamic bindings")); return nullptr; }
 }
 Error.Reset(); return Result;
}
const FFMCodexGuideStepPresentation* FFMCodexGuidedMatchContent::FindStep(const FString& LessonId,const FString& StepId,FString& Error) const
{
 const auto* L=FindLesson(LessonId); const auto* P=L?L->Steps.Find(StepId):nullptr;
 if(!P) Error=TEXT("Missing tutorial step: ")+LessonId+TEXT("/")+StepId;
 return P;
}
bool FFMCodexGuidedMatchContent::Interpolate(const FString& Template,const TMap<EFMCodexGuideVariable,FString>& Bindings,FString& Out,FString& Error)
{
 using namespace FMCodexGuideContentPrivate;
 TArray<FString> Keys; if(!TemplateKeys(Template,Keys,Error)) return false;
 Out.Reset();
 for(int32 I=0;I<Template.Len();++I)
 {
  if(Template[I]!='{') { Out.AppendChar(Template[I]); continue; }
  const int32 End=Template.Find(TEXT("}"),ESearchCase::CaseSensitive,ESearchDir::FromStart,I+1);
  const FString Key=Template.Mid(I+1,End-I-1); const auto* Value=Bindings.Find(Variables[Key]);
  if(!Value || Value->IsEmpty()) return Fail(Error,TEXT("Missing required binding: ")+Key);
  Out+=*Value; I=End;
 }
 return true;
}
bool FFMCodexGuidedMatchContent::Render(const FString& Template,const TArray<FFMCodexGuideEmphasis>& Spans,const FString& Field,
 const TMap<EFMCodexGuideVariable,FString>& Bindings,FString& Out,FString& Error)
{
 using namespace FMCodexGuideContentPrivate;
 TArray<FIntPoint> Ranges;
 for(const auto& S:Spans) if(S.Field==Field) { FIntPoint R; if(!SpanRange(Template,S,R,Error)) return false; Ranges.Add(R); }
 Ranges.Sort([](const FIntPoint& A,const FIntPoint& B){ return A.X<B.X; });
 auto Append=[&](const FString& Part)->bool { FString Text; if(!Interpolate(Part,Bindings,Text,Error)) return false;
  Text.ReplaceInline(TEXT("&"),TEXT("&amp;")); Text.ReplaceInline(TEXT("<"),TEXT("&lt;")); Text.ReplaceInline(TEXT(">"),TEXT("&gt;")); Out+=Text; return true; };
 Out.Reset(); int32 Offset=0;
 for(const auto& R:Ranges)
 {
  if(R.X<Offset) return Fail(Error,TEXT("Overlapping rendered emphasis"));
  if(!Append(Template.Mid(Offset,R.X-Offset))) return false;
  Out+=TEXT("<concept>"); if(!Append(Template.Mid(R.X,R.Y-R.X))) return false; Out+=TEXT("</>"); Offset=R.Y;
 }
 return Append(Template.Mid(Offset));
}
#endif
