#include "FMCodexTacticExplainerWidget.h"
#include "FMCodexMatchShellStyle.h"
#include "../CoreRules/PlayerTraitFormula.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SWrapBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Styling/CoreStyle.h"

namespace
{
namespace Theme=FMCodexMatchShellStyle;
const FSlateBrush* Panel()
{ static FSlateRoundedBoxBrush B(Theme::Navy(),9.f,Theme::Border(),1.f); return &B; }
const FSlateBrush* Card()
{ static FSlateRoundedBoxBrush B(Theme::Color(9,33,49),7.f,Theme::Border(),1.f); return &B; }
const FButtonStyle* Button(bool Selected=false)
{
 static FButtonStyle Normal=[](){FButtonStyle B; B.SetNormal(*Panel()); B.SetHovered(FSlateRoundedBoxBrush(Theme::Color(18,58,73),7.f,Theme::Mint(),1.f)); B.SetPressed(FSlateRoundedBoxBrush(Theme::Color(24,74,83),7.f)); B.SetNormalPadding(FMargin(14,6)); B.SetPressedPadding(FMargin(14,6)); return B;}();
 static FButtonStyle Active=[](){auto B=Normal; B.SetNormal(FSlateRoundedBoxBrush(Theme::Mint(),18.f)); B.SetHovered(B.Normal); B.SetPressed(B.Normal); return B;}();
 return Selected?&Active:&Normal;
}
TSharedRef<STextBlock> Label(FText T,float Size=21,bool Bold=false,FLinearColor Color=Theme::Text())
{ return SNew(STextBlock).Text(T).Font(Theme::Font(Size,Bold)).ColorAndOpacity(Color).AutoWrapText(false); }
TSharedRef<STextBlock> Label(const TCHAR* T,float Size=21,bool Bold=false,FLinearColor Color=Theme::Text())
{ return Label(FText::FromString(T),Size,Bold,Color); }
TSharedRef<SWidget> Heading(const TCHAR* T)
{
 return SNew(SHorizontalBox)
 +SHorizontalBox::Slot().AutoWidth().Padding(0,4,14,4)[SNew(SBox).WidthOverride(6)[SNew(SBorder).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush")).BorderBackgroundColor(Theme::Mint())]]
 +SHorizontalBox::Slot().FillWidth(1)[Label(T,20,true)];
}
TSharedRef<SWidget> RoleCard(const TCHAR* Title,const TArray<FFMCodexExplainerRole>& Rows)
{
 auto Box=SNew(SVerticalBox);
 Box->AddSlot().AutoHeight().Padding(0,0,0,7)[Label(Title,21,true)];
 if(Rows.IsEmpty()) Box->AddSlot().AutoHeight().Padding(0,10)[Label(TEXT("无属性对抗"),20,false,Theme::Secondary())];
 for(const auto& Row:Rows)
 {
  Box->AddSlot().AutoHeight()[SNew(SBox).HeightOverride(1)[SNew(SBorder).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush")).BorderBackgroundColor(Theme::Border())]];
  // Max is deliberately a two-line attribute treatment, preserving both inputs.
  auto Values=Label(Row.Attributes,18,true,Theme::Mint());
  Values->SetAutoWrapText(true);
  Box->AddSlot().AutoHeight().Padding(8,4)[SNew(SHorizontalBox)
   +SHorizontalBox::Slot().FillWidth(.48f)[Label(Row.Label,18)]
   +SHorizontalBox::Slot().FillWidth(.52f)[Values]];
 }
 return SNew(SBorder).BorderImage(Card()).Padding(14)[Box];
}
TSharedRef<SWidget> CopyCard(const TCHAR* Title,const FText& Copy)
{
 auto Body=Label(Copy,19); Body->SetAutoWrapText(true);
 return SNew(SBorder).BorderImage(Card()).Padding(12)[SNew(SHorizontalBox)
  +SHorizontalBox::Slot().AutoWidth().Padding(0,0,14,0)[Label(Title,20,true)]
  +SHorizontalBox::Slot().FillWidth(1)[Body]];
}
TSharedRef<SWidget> FormulaCard(const TCHAR* Title,const TArray<FFMCodexExplainerCalculationTerm>& Terms,bool bAttack,float Width)
{
 auto Box=SNew(SVerticalBox);
 Box->AddSlot().AutoHeight().Padding(0,0,0,6)[Label(Title,21,true)];
 for(int32 I=0;I<Terms.Num();++I)
 {
  const auto& Term=Terms[I]; const auto& Source=Term.Source;
  auto Line=SNew(SWrapBox).PreferredSize(Width-46.f).InnerSlotPadding(FVector2D(5,0));
  using K=EMatchPlayResolutionFormulaTermKind;
  if(Source.Kind==K::RawRoll || Source.Kind==K::FixedModifier)
  {
   // Signed fixed adjustments carry their own source label and sign.
   Line->AddSlot()[Label(FFMCodexTacticExplainerCatalog::CalculationTermText(Term,bAttack),18,false,Theme::Secondary())];
  }
  else
  {
   Line->AddSlot()[Label(Term.Role,18)];
   Line->AddSlot()[Label(TEXT("·"),18,false,Theme::Secondary())];
   Line->AddSlot()[Label(Term.Attribute,20,true,Theme::Mint())];
   Line->AddSlot()[Label(FText::FromString(FString::Printf(TEXT("× %g"),Source.Multiplier)),18,false,Theme::Secondary())];
  }
  if(Source.bOptional) Line->AddSlot()[Label(TEXT("（参与时）"),16,false,Theme::Secondary())];
  Box->AddSlot().AutoHeight().Padding(0,1)[SNew(SHorizontalBox)
   +SHorizontalBox::Slot().AutoWidth()[SNew(SBox).WidthOverride(22)[Label(I==0 || Source.Kind==K::FixedModifier?TEXT(""):TEXT("+"),18,false,Theme::Secondary())]]
   +SHorizontalBox::Slot().FillWidth(1)[Line]];
 }
 return SNew(SBorder).BorderImage(Card()).Padding(12)[Box];
}
TSharedRef<SWidget> ProcedureCard(const FFMCodexExplainerProcedure& P)
{
 auto Success=Label(P.Success,20,true,Theme::Mint()); Success->SetAutoWrapText(true);
 auto Failure=Label(P.Failure,18,false,Theme::Secondary()); Failure->SetAutoWrapText(true);
 return SNew(SBorder).BorderImage(Card()).Padding(18)[SNew(SVerticalBox)
  +SVerticalBox::Slot().AutoHeight().Padding(0,0,0,16)[Label(P.Title,21,true)]
  +SVerticalBox::Slot().AutoHeight().Padding(0,0,0,12)[Label(P.Action,20)]
  +SVerticalBox::Slot().AutoHeight().Padding(0,0,0,8)[Success]
  +SVerticalBox::Slot().AutoHeight()[Failure]];
}
TSharedRef<SWidget> MemoryLine(const FText& Memory,float Width)
{
 // Longest token first; prose remains neutral, only attribute words carry accent.
 const TArray<FString> Keywords={TEXT("取较高值"),TEXT("射门"),TEXT("传球"),TEXT("控球"),TEXT("速度"),TEXT("力量"),TEXT("防守")};
 auto Line=SNew(SWrapBox).PreferredSize(Width).InnerSlotPadding(FVector2D(0,0));
 FString S=Memory.ToString(), Plain;
 auto Flush=[&](){if(!Plain.IsEmpty()){Line->AddSlot()[Label(FText::FromString(Plain),22,true)];Plain.Empty();}};
 for(int32 I=0;I<S.Len();)
 {
  const FString* Key=Keywords.FindByPredicate([&](const auto& K){return S.Mid(I,K.Len())==K;});
  // “防守看” is prose; the attribute after 看 is the emphasized occurrence.
  if(Key && !(*Key==TEXT("防守") && S.Mid(I+2,1)==TEXT("看")))
  { Flush(); Line->AddSlot()[Label(FText::FromString(*Key),22,true,Theme::Mint())]; I+=Key->Len(); }
  else Plain+=S[I++];
 }
 Flush(); return Line;
}
}
const FFMCodexExplainerRoute& UFMCodexTacticExplainerWidget::GetRoute() const
{
 const auto& T=*FFMCodexTacticExplainerCatalog::Find(TacticId);
 const auto* R=T.Routes.FindByPredicate([this](const auto& X){return X.Id==RouteId;});
 return R?*R:T.Routes[0];
}
bool UFMCodexTacticExplainerWidget::SelectTactic(FName Id)
{
 const auto* T=FFMCodexTacticExplainerCatalog::Find(Id); if(!T) return false;
 TacticId=Id; RouteId=T->Routes[0].Id; bDetails=false; RefreshContent(); return true;
}
bool UFMCodexTacticExplainerWidget::SelectRoute(FName Id)
{
 const auto* T=FFMCodexTacticExplainerCatalog::Find(TacticId);
 if(!T->Routes.ContainsByPredicate([Id](const auto& R){return R.Id==Id;})) return false;
 RouteId=Id; RefreshContent(); return true;
}
void UFMCodexTacticExplainerWidget::ShowDetails(bool bShow) { bDetails=bShow; RefreshContent(); }
void UFMCodexTacticExplainerWidget::ReleaseSlateResources(bool bReleaseChildren)
{ Super::ReleaseSlateResources(bReleaseChildren); Navigation.Reset(); Content.Reset(); Scroll.Reset(); }
TSharedRef<SWidget> UFMCodexTacticExplainerWidget::RebuildWidget()
{
 auto Body=SNew(SHorizontalBox)
 +SHorizontalBox::Slot().AutoWidth()[SNew(SBox).WidthOverride(330)[SNew(SBorder).BorderImage(FCoreStyle::Get().GetBrush("NoBrush")).Padding(FMargin(30,25))[SAssignNew(Navigation,SVerticalBox)]]]
 +SHorizontalBox::Slot().AutoWidth()[SNew(SBox).WidthOverride(1)[SNew(SBorder).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush")).BorderBackgroundColor(Theme::Border())]]
 +SHorizontalBox::Slot().FillWidth(1)[SAssignNew(Scroll,SScrollBox)+SScrollBox::Slot().Padding(38,18)[SAssignNew(Content,SVerticalBox)]];
 auto Modal=SNew(SBox).WidthOverride(1440).HeightOverride(880)
 [SNew(SBorder).BorderImage(Panel()).Padding(1)
  [SNew(SVerticalBox)
   +SVerticalBox::Slot().AutoHeight()[SNew(SBorder).BorderImage(Card()).Padding(FMargin(32,12))
    [SNew(SHorizontalBox)
     +SHorizontalBox::Slot().FillWidth(1)[SNew(SVerticalBox)
      +SVerticalBox::Slot().AutoHeight()[Label(TEXT("战术说明"),30,true)]
      +SVerticalBox::Slot().AutoHeight().Padding(0,4,0,0)[Label(TEXT("查看战术用途、角色需求与详细规则"),18,false,Theme::Secondary())]]
     +SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Top)[SNew(SButton).ButtonStyle(FCoreStyle::Get(),"NoBorder").ContentPadding(10).OnClicked_Lambda([this](){OnClose.Broadcast();return FReply::Handled();})[Label(TEXT("×"),32)]]]]
   +SVerticalBox::Slot().FillHeight(1)[Body]]];
 auto Result=SNew(SOverlay)
 +SOverlay::Slot()[SNew(SBorder).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush")).BorderBackgroundColor(FLinearColor(0,0,0,.62f)).OnMouseButtonDown_Lambda([](const FGeometry&,const FPointerEvent&){return FReply::Handled();})]
 +SOverlay::Slot()[SNew(SScaleBox).Stretch(EStretch::ScaleToFit)[SNew(SBox).WidthOverride(1920).HeightOverride(1080)
  [SNew(SOverlay)+SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)[Modal]]]];
 RefreshContent(); return Result;
}
void UFMCodexTacticExplainerWidget::RefreshContent()
{
 if(!Content || !Navigation) return;
 // Static navigation and shell stay mounted while only their selected attributes change.
 if(Navigation->GetChildren()->Num()==0)
 {
  for(int32 I=0;I<FFMCodexTacticExplainerCatalog::Get().Num();++I)
  {
   const auto& T=FFMCodexTacticExplainerCatalog::Get()[I];
   if(I==0 || I==4) Navigation->AddSlot().AutoHeight().Padding(0,I==0?0:30,0,16)[Heading(I==0?TEXT("基础进攻"):TEXT("定位球"))];
   const FName Id=T.Id;
   Navigation->AddSlot().AutoHeight().Padding(0,3)[SNew(SBox).HeightOverride(54)
    [SNew(SButton).ButtonStyle(Button()).HAlign(HAlign_Fill).OnClicked_Lambda([this,Id](){SelectTactic(Id);return FReply::Handled();})
     [SNew(SHorizontalBox)
      +SHorizontalBox::Slot().AutoWidth().Padding(0,0,12,0)
       [SNew(STextBlock).Text_Lambda([this,Id](){return FText::FromString(Id==TacticId?TEXT("▎"):TEXT(" "));}).Font(Theme::Font(23,true)).ColorAndOpacity(Theme::Mint())]
      +SHorizontalBox::Slot().FillWidth(1).VAlign(VAlign_Center)
       [SNew(STextBlock).Text(T.Name).Font_Lambda([this,Id](){return Theme::Font(19,Id==TacticId);})
        .ColorAndOpacity_Lambda([this,Id](){return FSlateColor(Id==TacticId?Theme::Mint():Theme::Text());})]
      +SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
       [SNew(STextBlock).Text_Lambda([this,Id](){return FText::FromString(Id==TacticId?TEXT("›"):TEXT(""));}).Font(Theme::Font(23)).ColorAndOpacity(Theme::Mint())]]]];
  }
 }
 // The accepted fixed-width modal is scaled as a whole. Seed wrapping from its
 // current logical content width instead of SWrapBox's first-frame default 100.
 const float Width=Content->GetCachedGeometry().GetLocalSize().X>0.f
  ? Content->GetCachedGeometry().GetLocalSize().X : 1022.f;
 const float ColumnWidth=(Width-24.f)*.5f;
 auto Page=SNew(SVerticalBox);
 const auto& T=*FFMCodexTacticExplainerCatalog::Find(TacticId); const auto& R=GetRoute();
 Page->AddSlot().AutoHeight()[Label(T.Name,32,true)];
 Page->AddSlot().AutoHeight().Padding(0,4,0,12)[Label(T.Purpose,19)];
 auto Pills=SNew(SWrapBox).PreferredSize(Width).InnerSlotPadding(FVector2D(8,8));
 for(const auto& P:T.Routes)
 {
  const auto Id=P.Id; const bool Active=Id==RouteId;
  Pills->AddSlot()[SNew(SButton).ButtonStyle(Button(Active)).OnClicked_Lambda([this,Id](){SelectRoute(Id);return FReply::Handled();})[Label(P.Name,18,Active,Active?Theme::Ink():Theme::Text())]];
 }
 Page->AddSlot().AutoHeight().Padding(0,0,0,20)[Pills];
 if(bDetails)
 {
  Page->AddSlot().AutoHeight().Padding(0,0,0,16)[Heading(TEXT("计算方式"))];
  if(!R.Precondition.IsEmpty()) Page->AddSlot().AutoHeight().Padding(0,0,0,16)[CopyCard(TEXT("前置判定"),R.Precondition)];
  if(R.bProcedural)
  {
   auto Cards=SNew(SHorizontalBox);
   for(int32 I=0;I<R.Procedures.Num();++I) Cards->AddSlot().FillWidth(1).Padding(I==0?0:12,0,I+1==R.Procedures.Num()?0:12,0)[ProcedureCard(R.Procedures[I])];
   Page->AddSlot().AutoHeight()[Cards];
  }
  else
  {
   Page->AddSlot().AutoHeight()[SNew(SHorizontalBox)
    +SHorizontalBox::Slot().FillWidth(1).Padding(0,0,12,0)[FormulaCard(TEXT("进攻"),R.AttackCalculation,true,ColumnWidth)]
    +SHorizontalBox::Slot().FillWidth(1).Padding(12,0,0,0)[FormulaCard(TEXT("防守"),R.DefenseCalculation,false,ColumnWidth)]];
   if(!R.CalculationNote.IsEmpty()) { auto Note=Label(R.CalculationNote,18,false,Theme::Secondary()); Note->SetAutoWrapText(true); Page->AddSlot().AutoHeight().Padding(0,12,0,0)[Note]; }
   Page->AddSlot().AutoHeight().Padding(0,16,0,0)[CopyCard(TEXT("结果"),R.Result)];
  }
 }
 else
 {
  Page->AddSlot().AutoHeight().Padding(0,2,0,12)[MemoryLine(R.Memory,Width)];
  Page->AddSlot().AutoHeight().Padding(0,0,0,12)[Heading(TEXT("角色与关键属性"))];
  Page->AddSlot().AutoHeight()[SNew(SHorizontalBox)
   +SHorizontalBox::Slot().FillWidth(1).Padding(0,0,12,0)[RoleCard(TEXT("进攻"),R.Attack)]
   +SHorizontalBox::Slot().FillWidth(1).Padding(12,0,0,0)[RoleCard(TEXT("防守"),R.Defense)]];
  Page->AddSlot().AutoHeight().Padding(0,14,0,10)[Heading(TEXT("相关特性"))];
  auto Chips=SNew(SWrapBox).PreferredSize(Width).InnerSlotPadding(FVector2D(12,8));
  for(FName Id:R.Traits) Chips->AddSlot()[SNew(SBorder).BorderImage(Panel()).Padding(FMargin(18,9))[Label(FPlayerTraitFormula::DisplayName(Id),18)]];
  if(R.Traits.IsEmpty()) Chips->AddSlot()[Label(TEXT("此方法无相关属性特性"),20,false,Theme::Secondary())];
  Page->AddSlot().AutoHeight()[Chips];
  Page->AddSlot().AutoHeight().Padding(0,12,0,0)[Label(FFMCodexTacticExplainerCatalog::SkillNote(),15,false,Theme::Secondary())];
 }
 Page->AddSlot().AutoHeight().HAlign(HAlign_Right).Padding(0,bDetails?6:10,0,0)
 [SNew(SButton).ButtonStyle(Button()).OnClicked_Lambda([this](){ShowDetails(!bDetails);return FReply::Handled();})[Label(bDetails?TEXT("‹  返回概览"):TEXT("查看计算方式  ›"),19,true)]];
 // Add the prepared page before removing the old one: the mounted host never
 // has an empty child list and no partially populated page is exposed.
 TSharedPtr<SWidget> Previous;
 if(Content->GetChildren()->Num()>0) Previous=Content->GetChildren()->GetChildAt(0);
 Content->AddSlot().AutoHeight()[Page];
 if(Previous) Content->RemoveSlot(Previous.ToSharedRef());
 Scroll->SetScrollOffset(0.f);

}
FString UFMCodexTacticExplainerWidget::CollectPlayerFacingText() const
{
 const auto& T=*FFMCodexTacticExplainerCatalog::Find(TacticId); const auto& R=GetRoute();
 FString Out=TEXT("战术说明\n查看战术用途、角色需求与详细规则\n基础进攻\n定位球\n")+T.Name.ToString()+TEXT("\n")+T.Purpose.ToString();
 for(const auto& P:T.Routes) Out+=TEXT("\n")+P.Name.ToString();
 if(bDetails)
 {
  Out+=TEXT("\n计算方式");
  if(!R.Precondition.IsEmpty()) Out+=TEXT("\n前置判定\n")+R.Precondition.ToString();
  if(R.bProcedural) for(const auto& P:R.Procedures) Out+=TEXT("\n")+P.Title.ToString()+TEXT("\n")+P.Action.ToString()+TEXT("\n")+P.Success.ToString()+TEXT("\n")+P.Failure.ToString();
  else
  {
   Out+=TEXT("\n进攻");
   for(const auto& Term:R.AttackCalculation) Out+=TEXT("\n")+FFMCodexTacticExplainerCatalog::CalculationTermText(Term,true).ToString();
   Out+=TEXT("\n防守");
   for(const auto& Term:R.DefenseCalculation) Out+=TEXT("\n")+FFMCodexTacticExplainerCatalog::CalculationTermText(Term,false).ToString();
   if(!R.CalculationNote.IsEmpty()) Out+=TEXT("\n")+R.CalculationNote.ToString();
   Out+=TEXT("\n结果\n")+R.Result.ToString();
  }
  return Out+TEXT("\n返回概览");
 }
 Out+=TEXT("\n")+R.Memory.ToString();
 for(const auto& Row:R.Attack) Out+=TEXT("\n")+Row.Label.ToString()+TEXT(" ")+Row.Attributes.ToString();
 for(const auto& Row:R.Defense) Out+=TEXT("\n")+Row.Label.ToString()+TEXT(" ")+Row.Attributes.ToString();
 for(FName Id:R.Traits) Out+=TEXT("\n")+FPlayerTraitFormula::DisplayName(Id).ToString();
 return Out+TEXT("\n")+FFMCodexTacticExplainerCatalog::SkillNote().ToString()+TEXT("\n查看计算方式");
}

#if WITH_DEV_AUTOMATION_TESTS
void UFMCodexTacticExplainerWidget::ScrollDetailsToEndForTest() { if(Scroll && bDetails) Scroll->ScrollToEnd(); }
float UFMCodexTacticExplainerWidget::GetScrollOffsetForTest() const { return Scroll?Scroll->GetScrollOffset():0.f; }
#endif
