#include "FMCodexGoalCelebration.h"
#include "FMCodexMatchShellStyle.h"
#include "Framework/Application/SlateApplication.h"
#include "Fonts/FontMeasure.h"
#include "Layout/Geometry.h"
#include "Misc/Paths.h"
#include "Rendering/DrawElements.h"
#include "Rendering/SlateRenderer.h"
#include "Styling/CoreStyle.h"

namespace FMCodexGoalCelebration
{
bool FState::Start(bool bDisclosedGoal)
{
 if(!bDisclosedGoal || bConsumed)return false;
 bConsumed=true;Elapsed=0;return true;
}
void FState::Tick(float DeltaSeconds)
{Elapsed=FMath::Min(Duration,Elapsed+FMath::Max(0.f,DeltaSeconds));}
bool FState::Skip()
{if(!IsActive())return false;Elapsed=Duration;return true;}

namespace
{
// Celebration-specific typeface: ordinary HUD text keeps its existing weight.
FSlateFontInfo GoalFont()
{
 static const TSharedPtr<const FCompositeFont> Typeface=[]()
 {
  auto Result=MakeShared<FCompositeFont>();
  Result->DefaultTypeface.Fonts.Emplace(TEXT("Black"),
   FPaths::ProjectContentDir()/TEXT("Slate/Fonts/MatchShell/NotoSansSC-Black.otf"),
   EFontHinting::Default,EFontLoadingPolicy::LazyLoad);
  return Result;
 }();
 FSlateFontInfo Face(Typeface,164,TEXT("Black"));
 Face.SkewAmount=.22f;
 Face.LetterSpacing=-35;
 return Face;
}

FVector2D Bezier(const FVector2D* P,float T)
{
 const float U=1-T;
 return P[0]*(U*U*U)+P[1]*(3*U*U*T)+P[2]*(3*U*T*T)+P[3]*(T*T*T);
}

// Vertex gradients provide soft light and ribbon shading without a translucent
// full-screen rectangle, texture sampling seams, or per-frame asset loads.
struct FArtwork
{
 const FGeometry& Geometry;
 FVector2D Center;
 float Scale,Alpha;
 TArray<FSlateVertex> Vertices;
 TArray<SlateIndex> Indices;

 void Vertex(FVector2D P,FLinearColor Ink)
 {
  Ink.A*=Alpha;
  Vertices.Add(FSlateVertex::Make<ESlateVertexRounding::Disabled>(
   Geometry.GetAccumulatedRenderTransform(),FVector2f(Center+P*Scale),
   FVector2f::ZeroVector,Ink.ToFColor(true)));
 }
 void Quad(int32 A,int32 B,int32 C,int32 D)
 {Indices.Append({SlateIndex(A),SlateIndex(B),SlateIndex(C),SlateIndex(A),SlateIndex(C),SlateIndex(D)});}

 void Glow(FVector2D Position,FVector2D Radius,FLinearColor Ink)
 {
  constexpr int32 Sides=64,Rings=8;
  const int32 Start=Vertices.Num();
  for(int32 R=0;R<=Rings;++R)
  {
   const float U=float(R)/Rings;
   auto Tint=Ink;Tint.A*=FMath::Square(1-FMath::SmoothStep(0.f,1.f,U));
   for(int32 I=0;I<=Sides;++I)
   {
    const float A=I*(2*PI/Sides);
    Vertex(Position+FVector2D(FMath::Cos(A)*Radius.X,FMath::Sin(A)*Radius.Y)*U,Tint);
    if(R>0 && I>0)
    {
     const int32 N=Start+R*(Sides+1)+I;
     Quad(N-Sides-2,N-Sides-1,N,N-1);
    }
   }
  }
 }

 void Ribbon(const FVector2D* Top,const FVector2D* Bottom,FLinearColor Edge,FLinearColor Core,float Opacity)
 {
  constexpr int32 Steps=64,Rows=7;
  const float Across[Rows]={-.025f,0.f,.035f,.43f,.86f,1.f,1.025f};
  const float Brightness[Rows]={0.f,.87f,1.f,.89f,.79f,.73f,0.f};
  const int32 Start=Vertices.Num();
  for(int32 I=0;I<=Steps;++I)
  {
   const float T=float(I)/Steps;
   const auto A=Bezier(Top,T),B=Bezier(Bottom,T);
   const float Ends=FMath::SmoothStep(0.f,.13f,T)*(1-FMath::SmoothStep(.88f,1.f,T));
   for(int32 J=0;J<Rows;++J)
   {
    const float Along=.38f+.62f*FMath::Sin(PI*T);
    auto Tint=FMath::Lerp(Edge,Core,Brightness[J]*Along);
    Tint.A=Opacity*Ends*(J==0 || J==Rows-1?0.f:1.f);
    Vertex(FMath::Lerp(A,B,Across[J]),Tint);
    if(I>0 && J>0)
    {
     const int32 N=Start+I*Rows+J;
     Quad(N-Rows-1,N-Rows,N,N-1);
    }
   }
  }
 }

 void Shard(FVector2D P,float Size,FLinearColor Ink)
 {
  const int32 N=Vertices.Num();
  Vertex(P+FVector2D(-Size*.45f,0),Ink);
  Vertex(P+FVector2D(Size*.38f,-Size),Ink);
  Vertex(P+FVector2D(Size*.55f,Size*.80f),Ink);
  Indices.Append({SlateIndex(N),SlateIndex(N+1),SlateIndex(N+2)});
 }

 void Draw(FSlateWindowElementList& Out,int32 Layer)
 {
  const auto& Brush=*FCoreStyle::Get().GetBrush(TEXT("WhiteBrush"));
  FSlateDrawElement::MakeCustomVerts(Out,Layer,FSlateApplication::Get().GetRenderer()->GetResourceHandle(Brush),
   Vertices,Indices,nullptr,0,0);
  Vertices.Reset();Indices.Reset();
 }
};
}

int32 Paint(const FState& State,const FGeometry& G,FSlateWindowElementList& Out,int32 Layer)
{
 if(!State.IsActive())return Layer;
 using namespace FMCodexMatchShellStyle;
 const float T=State.Elapsed;
 const float Enter=FMath::SmoothStep(0.f,.20f,T),Exit=1.f-FMath::SmoothStep(1.10f,Duration,T);
 const float Alpha=Enter*Exit;
 const FVector2D C=G.GetLocalSize()*.5f;
 const float Fit=FMath::Min(float(G.GetLocalSize().X)/1324.f,float(G.GetLocalSize().Y)/380.f);
 const float Sweep=-120*(1-Enter)+90*(1-Exit);
 FArtwork Art{G,C+FVector2D(Sweep*Fit,0),Fit,Alpha};
 Art.Vertices.Reserve(7000);Art.Indices.Reserve(36000);
 Art.Glow(FVector2D(0,8),FVector2D(655,184),Color(2,14,22).CopyWithNewOpacity(.82f));
 const FVector2D BandTop[]={{-620,-34},{-255,-125},{300,-116},{624,-12}};
 const FVector2D BandBottom[]={{-620,51},{-210,136},{272,135},{624,42}};
 Art.Ribbon(BandTop,BandBottom,Color(3,20,29),Color(8,31,43),.92f);
 Art.Draw(Out,Layer);

 // Local illumination follows the brightest bends, not the entire UI frame.
 Art.Glow(FVector2D(-370,-62),FVector2D(225,89),Mint().CopyWithNewOpacity(.32f));
 Art.Glow(FVector2D(360,78),FVector2D(242,83),Mint().CopyWithNewOpacity(.30f));
 Art.Draw(Out,Layer+1);
 const FVector2D LeftTop[]={{-648,-15},{-489,-113},{-379,-133},{-108,-51}};
 const FVector2D LeftBottom[]={{-648,12},{-442,-66},{-371,-84},{-108,-51}};
 const FVector2D RightTop[]={{18,70},{285,191},{472,24},{648,1}};
 const FVector2D RightBottom[]={{18,70},{244,224},{484,88},{648,25}};
 const FVector2D EchoTop[]={{135,62},{365,109},{508,-25},{644,-48}};
 const FVector2D EchoBottom[]={{135,65},{370,135},{511,-7},{644,-42}};
 const FVector2D UnderTop[]={{-608,101},{-414,53},{-204,74},{-60,94}};
 const FVector2D UnderBottom[]={{-608,102},{-425,73},{-187,81},{-60,94}};
 Art.Ribbon(EchoTop,EchoBottom,Color(7,65,69),Color(31,169,153),.42f);
 Art.Ribbon(UnderTop,UnderBottom,Color(10,53,65),Color(47,128,139),.45f);
 Art.Ribbon(LeftTop,LeftBottom,Color(8,106,101),Color(54,247,219),.97f);
 Art.Ribbon(RightTop,RightBottom,Color(8,98,100),Color(46,239,211),.97f);
 Art.Draw(Out,Layer+2);
 // Stable decoration only: no gameplay RNG or outcome-dependent particles.
 for(int32 I=0;I<26;++I)
 {
  const float Side=I%2?-1.f:1.f;
  const float X=Side*(305+(I*47)%277+T*13);
  const float Y=-111+(I*67)%224;
  const float Radius=I%7==0?5.f:1.f+(I%3)*.65f;
  const auto Ink=(I%7==0?Text():Mint()).CopyWithNewOpacity(I%7==0?.9f:.25f+(I%4)*.1f);
  Art.Shard(FVector2D(X,Y),Radius,Ink);
 }
 Art.Draw(Out,Layer+3);
 const float TextEnter=FMath::SmoothStep(.20f,.40f,T);
 if(TextEnter>0)
 {
  // Narrow punctuation keeps the optical gap beside 球 close to the reference.
  const FText Title=NSLOCTEXT("GoalCelebration","Goal","进球!");
  const float Scale=FMath::Lerp(.88f,1.f,TextEnter)+.09f*FMath::Sin(TextEnter*PI);
  const auto Face=GoalFont();
  const auto Extent=FSlateApplication::Get().GetRenderer()->GetFontMeasureService()->Measure(Title,Face);
  const float TextScale=Scale*Fit;
  const auto TextGeometry=G.MakeChild(Extent,FSlateLayoutTransform(TextScale,
   C-Extent*TextScale*.5f+FVector2D(-10,8+10*(1-TextEnter))*Fit));
  FSlateDrawElement::MakeText(Out,Layer+4,TextGeometry.ToPaintGeometry(),Title,Face,ESlateDrawEffect::None,Text().CopyWithNewOpacity(TextEnter*Exit));
 }
 return Layer+5;
}
}
