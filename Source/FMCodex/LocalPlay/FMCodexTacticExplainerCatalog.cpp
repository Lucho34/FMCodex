#include "FMCodexTacticExplainerCatalog.h"
#include "FMCodexPlayerUIPresentationText.h"
#include "../CoreRules/PlayerTraitFormula.h"

namespace
{
using K=EMatchPlayResolutionFormulaTermKind;
using R=EMatchPlayResolutionParticipantRole;
using A=EMatchPlayResolutionFormulaAttribute;
FText Text(const FString& S) { return FText::FromString(S); }
FString Role(R Value)
{
 FString Name=FFMCodexPlayerUIPresentationText::TacticalDetailParticipantRole(Value).ToString();
 if(Value==R::Carrier || Value==R::Marker || Value==R::Helper) Name+=TEXT("球员");
 return Name;
}
FString Attr(A Value) { return FFMCodexPlayerUIPresentationText::ResolutionAttribute(Value).ToString(); }
void Project(const TArray<FTacticalRuleDescriptionTerm>& Terms,TArray<FFMCodexExplainerRole>& Rows,TArray<FFMCodexExplainerCalculationTerm>& Calculation)
{
 TArray<R> Roles;
 for(const auto& T:Terms)
 {
  if(T.Attribute!=A::None)
  {
   FString Label=Role(T.ParticipantRole);
   if(T.ParticipantRole==R::Goalkeeper && T.bOptional) Label+=TEXT("（参与时）");
   const int32 Existing=Roles.Find(T.ParticipantRole);
   if(Existing==INDEX_NONE) { Roles.Add(T.ParticipantRole); Rows.Add({Text(Label),Text(Attr(T.Attribute))}); }
   else if(!Rows[Existing].Attributes.ToString().Contains(Attr(T.Attribute))) Rows[Existing].Attributes=Text(Rows[Existing].Attributes.ToString()+TEXT("、")+Attr(T.Attribute));
  }
  // Common match modifiers belong to live Formula disclosure, not this core calculation.
  if(T.Kind!=K::TacticalPlayerAdvantage) Calculation.Add({T,Text(Role(T.ParticipantRole)),Text(Attr(T.Attribute)),false});
 }
}
void Arithmetic(FFMCodexExplainerRoute& P)
{
 Project(P.Rule.AttackTerms,P.Attack,P.AttackCalculation);
 Project(P.Rule.DefenseTerms,P.Defense,P.DefenseCalculation);
 P.Result=Text(TEXT("比较双方最终值，数值较高的一方获胜。"));
}
FString Range(int32 Minimum,int32 Maximum)
{ return Minimum==Maximum?FString::FromInt(Minimum):FString::Printf(TEXT("%d–%d"),Minimum,Maximum); }
void DiceProcedure(FFMCodexExplainerRoute& P,int32 Count,bool Total,int32 WinMin,int32 WinMax,int32 LoseMin,int32 LoseMax)
{
 const FString Prefix=Total?TEXT("骰点合计 "):TEXT("骰点 ");
 P.Procedures.Add({P.Name,Text(FString::Printf(TEXT("投掷 %d 次骰子。"),Count)),
  Text(Prefix+Range(WinMin,WinMax)+TEXT("，进球。")),Text(Prefix+Range(LoseMin,LoseMax)+TEXT("，不进球。"))});
}
FFMCodexExplainerRoute Base(ESkillRuleType Skill,const TCHAR* Id,const TCHAR* Name,const TCHAR* Memory,std::initializer_list<const TCHAR*> Traits)
{
 FFMCodexExplainerRoute P; P.Id=Id; P.Name=Text(Name); P.Memory=Text(Memory);
 const auto* Family=FTacticalRuleDescriptionCatalog::FindBySkillType(Skill);
 if(Family) if(const auto* B=Family->Branches.FindByPredicate([&](const auto& X){return X.BranchId==P.Id;})) P.Rule=*B;
 P.bProcedural=P.Rule.RollSemantics==EMatchPlayResolutionRollSemantics::OutcomeDecision;
 for(auto IdValue:Traits) P.Traits.Add(IdValue);
 if(!P.bProcedural) Arithmetic(P);
 if(P.Rule.SpecialRuleId==TEXT("AttackRollOneTwoImmediateMiss")) P.Precondition=Text(TEXT("进攻骰点为 1–2 时，直接射偏，本次进攻结束。"));
 if(P.Id==TEXT("ThroughBall.BehindDefenseP1")) P.Precondition=Text(TEXT("进攻骰点为 1–2 时，出界，本次进攻结束。"));
 if(P.Id.ToString().Contains(TEXT("DeadCorner")) || P.Id==TEXT("ThroughBall.OneOnOneChip"))
 {
  P.Attack={{Text(P.Id==TEXT("ThroughBall.OneOnOneChip")?TEXT("跑位球员"):TEXT("持球球员")),Text(TEXT("只看掷点"))}};
  const auto& Lose=P.Rule.Outcomes[0]; const auto& Win=P.Rule.Outcomes[1];
  DiceProcedure(P,P.Rule.OutcomeRollCount,P.Rule.bOutcomeUsesRollTotal,Win.Minimum,Win.Maximum,Lose.Minimum,Lose.Maximum);
 }
 if(P.Id==TEXT("ThroughBall.AntiOffside"))
 {
  P.Attack={{Text(TEXT("跑位球员")),Text(TEXT("反越位判定"))}};
  P.Procedures={
   {Text(TEXT("普通球员")),Text(TEXT("投掷 1 次骰子。")),Text(TEXT("掷出 6，反越位成功。")),Text(TEXT("未掷出 6，越位。"))},
   {Text(TEXT("反越位专家")),Text(TEXT("投掷 2 次骰子。")),Text(TEXT("任意一次掷出 6，反越位成功。")),Text(TEXT("两次都未掷出 6，越位。"))}};
 }
 return P;
}
FFMCodexExplainerRoute Corner(bool High)
{
 FFMCodexExplainerRoute P; P.Id=High?TEXT("Corner.High"):TEXT("Corner.Low"); P.Name=Text(High?TEXT("高球"):TEXT("低球"));
 P.Memory=Text(High?TEXT("高球角球强调力量。"):TEXT("进攻看控球，防守看防守。"));
 P.Rule=*FTacticalRuleDescriptionCatalog::FindCornerRoute(High?EMatchPlayCornerRouteIntent::High:EMatchPlayCornerRouteIntent::Low);
 P.Traits=High?TArray<FName>{TEXT("Trait.CornerHighThreat"),TEXT("Trait.CornerHighDefense")}:TArray<FName>{TEXT("Trait.CornerLowThreat"),TEXT("Trait.CornerLowDefense")};
 Arithmetic(P);
 // This is Corner's specific modifier, separate from the common tactical-player modifier.
 P.CalculationNote=Text(TEXT("角球人数修正：双方均有候选时，多 1 人的一方最终值 +2，多 2 人 +3，同数不加。"));
 return P;
}
FFMCodexExplainerRoute SetPiece(const TCHAR* Id,const TCHAR* Name,const TCHAR* Memory,const TCHAR* GK,int32 Fixed,const TCHAR* Trait,bool Max)
{
 FFMCodexExplainerRoute P; P.Id=Id; P.Name=Text(Name); P.Memory=Text(Memory); P.bMaxAttributes=Max; P.Traits={FName(Trait)};
 P.Attack={{Text(TEXT("主罚球员")),Text(Max?TEXT("射门 / 传球\n取较高值"):TEXT("射门"))}};
 P.Defense={{Text(TEXT("门将")),Text(GK)}};
 FTacticalRuleDescriptionTerm Shooting; Shooting.Kind=K::Attribute; Shooting.ParticipantRole=R::Taker; Shooting.Attribute=A::Shooting;
 FTacticalRuleDescriptionTerm Roll; Roll.Kind=K::RawRoll;
 FTacticalRuleDescriptionTerm Keeper; Keeper.Kind=K::GoalkeeperContribution; Keeper.ParticipantRole=R::Goalkeeper; Keeper.Attribute=Max?(P.Id==TEXT("NearFreeKick.Direct")?A::GoalkeeperHandling:A::None):A::GoalkeeperPositioning;
 FTacticalRuleDescriptionTerm Modifier; Modifier.Kind=K::FixedModifier; Modifier.FixedModifier=Fixed;
 P.AttackCalculation={{Shooting,Text(Max?TEXT("主罚能力"):TEXT("主罚球员")),Text(Max?TEXT("射门 与 传球 取较高值"):TEXT("射门")),Max},{Roll,FText(),FText(),false}};
 P.DefenseCalculation={{Keeper,Text(TEXT("门将")),Text(GK),false},{Roll,FText(),FText(),false},{Modifier,FText(),FText(),false}};
 P.Result=Text(TEXT("比较双方最终值，数值较高的一方获胜。"));
 if(!Max) P.Precondition=Text(TEXT("进攻骰点为 1–2 时，直接射偏，本次进攻结束。"));
 return P;
}
FFMCodexExplainerRoute Procedural(const TCHAR* Id,const TCHAR* Name,const TCHAR* Memory,int32 Count,int32 WinMin)
{
 FFMCodexExplainerRoute P; P.Id=Id; P.Name=Text(Name); P.Memory=Text(Memory); P.bProcedural=true;
 P.Attack={{Text(TEXT("主罚球员")),Text(TEXT("只看掷点"))}};
 DiceProcedure(P,Count,Count==2,WinMin,Count*6,Count,WinMin-1);
 if(P.Id==TEXT("NearFreeKick.Angled")) P.Precondition=Text(TEXT("射门 + 传球 ≥ 8 时可用。"));
 return P;
}
}
const TArray<FFMCodexExplainerTactic>& FFMCodexTacticExplainerCatalog::Get()
{
 static const TArray<FFMCodexExplainerTactic> Catalog=[]()
 {
  using S=ESkillRuleType;
  TArray<FFMCodexExplainerTactic> C;
  C.Add({TEXT("LongShot"),Text(TEXT("远射")),Text(TEXT("持球球员从远处起脚，直接威胁球门。")),S::LongShot,false,{
   Base(S::LongShot,TEXT("LongShot.Direct"),TEXT("直接射门"),TEXT("持球球员看射门，盯人球员看防守。"),{TEXT("Trait.LongShotCarrier"),TEXT("Trait.LongShotBlocker")}),
   Base(S::LongShot,TEXT("LongShot.DeadCorner"),TEXT("射向死角"),TEXT("瞄准死角，只看两枚骰子的合计。"),{})}});
  C.Add({TEXT("CutInside"),Text(TEXT("内切")),Text(TEXT("持球球员向中路摆脱防守，寻找射门机会。")),S::CutInsideShot,false,{
   Base(S::CutInsideShot,TEXT("CutInside.Direct"),TEXT("直接射门"),TEXT("进攻看控球与射门，防守看防守与速度。"),{TEXT("Trait.CutInsideCarrier"),TEXT("Trait.CutInsideStopper")}),
   Base(S::CutInsideShot,TEXT("CutInside.DeadCorner"),TEXT("射向死角"),TEXT("瞄准死角，只看两枚骰子的合计。"),{})}});
  C.Add({TEXT("Cross"),Text(TEXT("传中")),Text(TEXT("将球送入危险区域，由跑位球员完成接应。")),S::Cross,false,{
   Base(S::Cross,TEXT("Cross.High"),TEXT("高球"),TEXT("持球球员看传球，跑位球员看力量。"),{TEXT("Trait.CrossCarrier"),TEXT("Trait.CrossHighRunner"),TEXT("Trait.CrossMarkerBlocker"),TEXT("Trait.CrossHighHelperDefense")}),
   Base(S::Cross,TEXT("Cross.Low"),TEXT("低球"),TEXT("持球球员看传球，跑位球员看速度。"),{TEXT("Trait.CrossCarrier"),TEXT("Trait.CrossLowRunner"),TEXT("Trait.CrossMarkerBlocker"),TEXT("Trait.CrossLowHelperDefense")})}});
  C.Add({TEXT("ThroughBall"),Text(TEXT("直塞")),Text(TEXT("将球送入防线空当，为跑位球员制造机会。")),S::ThroughBall,false,{
   Base(S::ThroughBall,TEXT("ThroughBall.Feet"),TEXT("脚下球"),TEXT("持球球员看传球，跑位球员看控球。"),{TEXT("Trait.ThroughBallCarrier"),TEXT("Trait.ThroughBallFeetRunner"),TEXT("Trait.ThroughBallMarkerDefense"),TEXT("Trait.ThroughBallFeetHelperDefense")}),
   Base(S::ThroughBall,TEXT("ThroughBall.BehindDefenseP1"),TEXT("身后球"),TEXT("持球球员看传球，跑位球员看速度。"),{TEXT("Trait.ThroughBallCarrier"),TEXT("Trait.ThroughBallBehindRunner"),TEXT("Trait.ThroughBallMarkerDefense"),TEXT("Trait.ThroughBallBehindHelperDefense")}),
   Base(S::ThroughBall,TEXT("ThroughBall.AntiOffside"),TEXT("反越位"),TEXT("跑位球员通过反越位判定寻找单刀机会。"),{TEXT("Trait.ThroughBallAntiRunner")}),
   Base(S::ThroughBall,TEXT("ThroughBall.OneOnOneDirect"),TEXT("单刀·直接射门"),TEXT("跑位球员看射门，门将看单刀。"),{}),
   Base(S::ThroughBall,TEXT("ThroughBall.OneOnOneChip"),TEXT("单刀·挑射"),TEXT("越过门将的挑射，只看掷点。"),{})}});
  C.Add({TEXT("Corner"),Text(TEXT("角球")),Text(TEXT("从角旗区发起进攻，由选中的球员争抢机会。")),S::None,true,{Corner(true),Corner(false)}});
  C.Add({TEXT("NearFreeKick"),Text(TEXT("近距离任意球")),Text(TEXT("把握球门附近的定位球机会。")),S::None,true,{
   SetPiece(TEXT("NearFreeKick.Direct"),TEXT("直接射门"),TEXT("射门与传球取较高值。"),TEXT("手控球"),1,TEXT("Trait.NearFreeKickTaker"),true),
   Procedural(TEXT("NearFreeKick.Angled"),TEXT("战术配合"),TEXT("射门与传球之和达标后，尝试战术配合。"),2,9)}});
  C.Add({TEXT("LongFreeKick"),Text(TEXT("远距离任意球")),Text(TEXT("从较远位置主罚，选择射门方式。")),S::None,true,{
   SetPiece(TEXT("LongFreeKick.Direct"),TEXT("直接射门"),TEXT("主罚球员看射门。"),TEXT("站位"),2,TEXT("Trait.LongFreeKickTaker"),false),
   Procedural(TEXT("LongFreeKick.Power"),TEXT("重炮轰门"),TEXT("重炮轰门，只看两枚骰子的合计。"),2,11)}});
  C.Add({TEXT("Penalty"),Text(TEXT("点球")),Text(TEXT("主罚球员与门将在点球点前对决。")),S::None,true,{
   SetPiece(TEXT("Penalty.Direct"),TEXT("常规点球"),TEXT("射门与传球取较高值。"),TEXT("预判"),-3,TEXT("Trait.PenaltyTaker"),true),
   Procedural(TEXT("Penalty.Panenka"),TEXT("勺子点球"),TEXT("轻巧挑射，只看一枚骰子。"),1,2)}});
  return C;
 }();
 return Catalog;
}
const FFMCodexExplainerTactic* FFMCodexTacticExplainerCatalog::Find(FName Id)
{ return Get().FindByPredicate([Id](const auto& T){return T.Id==Id;}); }
FText FFMCodexTacticExplainerCatalog::SkillNote()
{ return NSLOCTEXT("FMCodexExplainer","SkillNote","基础进攻是否可用，由持球球员卡牌上的技能范围与本回合战术点决定。"); }

FText FFMCodexTacticExplainerCatalog::CalculationTermText(const FFMCodexExplainerCalculationTerm& Term,bool bAttack)
{
 const auto& T=Term.Source;
 if(T.Kind==K::RawRoll) return Text(bAttack?TEXT("进攻骰点"):TEXT("防守骰点"));
 if(T.Kind==K::FixedModifier) return Text(FString::Printf(TEXT("固定%s修正 %s%d"),bAttack?TEXT("进攻"):TEXT("防守"),T.FixedModifier>=0?TEXT("+"):TEXT("−"),FMath::Abs(T.FixedModifier)));
 FString S=Term.Role.ToString()+TEXT(" · ")+Term.Attribute.ToString()+FString::Printf(TEXT(" × %g"),T.Multiplier);
 if(T.bOptional) S+=TEXT("（参与时）");
 return Text(S);
}
