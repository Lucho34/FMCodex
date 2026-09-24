#include "FMCodexFormulaBroadcastPrototype.h"

#if !UE_BUILD_SHIPPING
#include "FMCodexLocalMatchUMGPresentation.h"
#include "FMCodexMatchFlowPanel.h"
#include "FMCodexOutcomePresentation.h"
#include "FMCodexPlayerUIPresentationText.h"
#include "FMCodexPlayerUIStyle.h"
#include "FMCodexRollReelWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/ButtonSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/NativeWidgetHost.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/ScaleBox.h"
#include "Components/ScaleBoxSlot.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/RichTextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/WrapBox.h"
#include "HAL/IConsoleManager.h"
#include "Rendering/DrawElements.h"
#include "Widgets/SLeafWidget.h"

#define LOCTEXT_NAMESPACE "FMCodexBroadcastPrototype"
namespace FMCodexFormulaBroadcastPrototype
{
namespace
{
	TAutoConsoleVariable<int32> Mode(TEXT("fm.UI.FormulaV2"), 0,
		TEXT("DEV only: 1 previews Stage 8.8C on standalone Cross.High Formula; 0 restores accepted UI."), ECVF_Default);
	using K = EFMCodexUMGInlineFormulaTermKind;
	using E = EFMCodexFormulaEmphasis;
	FLinearColor Color(uint8 R, uint8 G, uint8 B) { return FLinearColor::FromSRGBColor(FColor(R,G,B)); }
	const FLinearColor Ink = Color(7,22,34), White = Color(240,247,252), Quiet = Color(167,193,209);
	// Action cyan is deliberately separate from the accepted Goal mint token.
	const FLinearColor Aqua = Color(66,231,229), Gold = Color(233,204,143);
	FLinearColor Alpha(FLinearColor C, float A) { C.A = A; return C; }

	// One gradient per card protects the reading column while the surrounding shell
	// stays open to the real pitch. It is paint only: no material, blur or tick.
	class SBroadcastReadingWash final : public SLeafWidget
	{
	public:
		SLATE_BEGIN_ARGS(SBroadcastReadingWash) {}
		SLATE_END_ARGS()
		void Construct(const FArguments&) {}
		FVector2D ComputeDesiredSize(float) const override { return FVector2D::ZeroVector; }
		int32 OnPaint(const FPaintArgs&, const FGeometry& G, const FSlateRect&, FSlateWindowElementList& Out,
			int32 Layer, const FWidgetStyle& Style, bool) const override
		{
			const float W = G.GetLocalSize().X;
			TArray<FSlateGradientStop> Stops;
			Stops.Emplace(FVector2D(0,0),Alpha(Ink,.65f)*Style.GetColorAndOpacityTint());
			Stops.Emplace(FVector2D(W*.60f,0),Alpha(Ink,.52f)*Style.GetColorAndOpacityTint());
			Stops.Emplace(FVector2D(W,0),Alpha(Ink,0.f));
			FSlateDrawElement::MakeGradient(Out,Layer,G.ToPaintGeometry(),Stops,Orient_Horizontal,
				ESlateDrawEffect::None,FVector4f(10.f));
			return Layer;
		}
	};

	// Local, asset-free broadcast motifs. No dependency on the shared V1 skin.
	class SBroadcastIcon final : public SLeafWidget
	{
	public:
		SLATE_BEGIN_ARGS(SBroadcastIcon) : _Dice(false), _Tint(FLinearColor::White) {}
			SLATE_ARGUMENT(bool, Dice)
			SLATE_ARGUMENT(FLinearColor, Tint)
		SLATE_END_ARGS()
		void Construct(const FArguments& Args) { bDice = Args._Dice; Tint = Args._Tint; }
		FVector2D ComputeDesiredSize(float) const override { return FVector2D(40,40); }
		int32 OnPaint(const FPaintArgs&, const FGeometry& G, const FSlateRect&, FSlateWindowElementList& Out,
			int32 Layer, const FWidgetStyle& Style, bool) const override
		{
			const FVector2D Scale = G.GetLocalSize() / 40.f;
			auto Line = [&](TArray<FVector2D> Points, float Width = 1.3f)
			{
				for (auto& P : Points) P *= Scale;
				FSlateDrawElement::MakeLines(Out,Layer,G.ToPaintGeometry(),Points,ESlateDrawEffect::None,
					Tint * Style.GetColorAndOpacityTint(),true,Width);
			};
			auto Ring = [&](FVector2D Center, float Radius, int32 Segments)
			{
				TArray<FVector2D> Points;
				for (int32 I=0; I<=Segments; ++I)
				{
					const float A = -PI/2.f + 2.f*PI*I/Segments;
					Points.Add(Center + FVector2D(FMath::Cos(A),FMath::Sin(A))*Radius);
				}
				Line(Points);
			};
			if (bDice)
			{
				Line({{20,3},{37,12},{37,29},{20,38},{3,29},{3,12},{20,3}},2.f);
				Line({{3,12},{20,22},{37,12}},2.f); Line({{20,22},{20,38}},2.f);
				for (const FVector2D P : {FVector2D(20,12),FVector2D(9,21),FVector2D(14,30),FVector2D(26,30),FVector2D(31,22)}) Ring(P,1.2f,10);
			}
			else
			{
				Ring({20,20},18,36); Ring({20,20},7.5f,5);
				for (int32 I=0; I<5; ++I)
				{
					const float A = -PI/2.f + 2.f*PI*I/5.f;
					const FVector2D D(FMath::Cos(A),FMath::Sin(A));
					Line({FVector2D(20,20)+D*7.5f,FVector2D(20,20)+D*18.f});
				}
			}
			return Layer;
		}
	private:
		bool bDice = false;
		FLinearColor Tint;
	};
	USizeBox* Icon(UWidgetTree& Tree, FName Name, float Size, bool bDice, FLinearColor Tint)
	{
		auto* Host = Tree.ConstructWidget<UNativeWidgetHost>();
		Host->SetContent(SNew(SBroadcastIcon).Dice(bDice).Tint(Tint));
		auto* Bounds = Tree.ConstructWidget<USizeBox>(USizeBox::StaticClass(),Name);
		Bounds->SetWidthOverride(Size); Bounds->SetHeightOverride(Size); Bounds->AddChild(Host);
		return Bounds;
	}
	USizeBox* Rule(UWidgetTree& Tree, float Width, FLinearColor Tint = Quiet)
	{
		auto* Bounds = Tree.ConstructWidget<USizeBox>(); Bounds->SetHeightOverride(1.f);
		if (Width > 0) Bounds->SetWidthOverride(Width);
		auto* Line = Tree.ConstructWidget<UBorder>(); Line->SetPadding(FMargin(0)); Line->SetBrushColor(Alpha(Tint,.45f));
		Bounds->AddChild(Line); return Bounds;
	}
	UScaleBox* Fit(UWidgetTree& Tree, UWidget* Content)
	{
		auto* Scale = Tree.ConstructWidget<UScaleBox>(); Scale->SetStretch(EStretch::ScaleToFit);
		Scale->SetStretchDirection(EStretchDirection::DownOnly);
		CastChecked<UScaleBoxSlot>(Scale->AddChild(Content))->SetHorizontalAlignment(HAlign_Left);
		return Scale;
	}
	USizeBox* Atomic(UWidgetTree& Tree, UWidget* Content)
	{
		// Wrap complete names/operands, never individual CJK glyphs or multipliers.
		// The cap also contains an unusually long contributor fallback without clipping.
		auto* Bounds = Tree.ConstructWidget<USizeBox>(); Bounds->SetMaxDesiredWidth(430.f);
		Bounds->AddChild(Fit(Tree,Content)); return Bounds;
	}
	FName Named(const FString& Prefix, const TCHAR* Suffix) { return FName(*(Prefix + Suffix)); }
	template<typename T> T* Find(UWidgetTree& Tree, FName Name) { return CastChecked<T>(Tree.FindWidget(Name)); }
	UTextBlock* Text(UWidgetTree& Tree, FName Name, int32 Size, FLinearColor Tint = White)
	{
		auto* W = Tree.ConstructWidget<UTextBlock>(UTextBlock::StaticClass(),Name);
		FFMCodexPlayerUIStyle::Get().ApplyFlowText(*W, Size);
		W->SetColorAndOpacity(Tint);
		W->SetAutoWrapText(false);
		return W;
	}
	void Weight(UTextBlock& W, FLinearColor Tint)
	{
		// Existing CJK fallback has no bold face. A restrained same-color stroke
		// supplies local broadcast weight without importing or changing shared fonts.
		auto Font = W.GetFont(); Font.OutlineSettings.OutlineSize = 1;
		Font.OutlineSettings.OutlineColor = Alpha(Tint,Font.Size >= 36 ? .75f : Font.Size >= 24 ? .40f : .25f); W.SetFont(Font);
		W.SetColorAndOpacity(Tint);
	}
	void Shadow(UTextBlock& W)
	{
		W.SetShadowOffset(FVector2D(0,2)); W.SetShadowColorAndOpacity(Alpha(Ink,.85f));
	}
	void Rounded(UBorder& Border, FLinearColor Fill, FLinearColor Edge, float Radius = 10.f)
	{
		Border.SetBrush(FSlateRoundedBoxBrush(Fill,Radius,Edge,1.f));
		Border.SetBrushColor(FLinearColor::White);
	}
	void SetText(UWidgetTree& Tree, FName Name, const FString& Label)
	{
		Find<UTextBlock>(Tree,Name)->SetText(FText::FromString(Label));
	}
	void Show(UWidget& W, bool bShow)
	{
		W.SetVisibility(bShow ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}
	void BuildRow(UWidgetTree& Tree, UVerticalBox& Body, const FString& Prefix)
	{
		auto* Card = Tree.ConstructWidget<UBorder>(UBorder::StaticClass(),Named(Prefix,TEXT("Card")));
		Rounded(*Card, Ink, Color(55,85,103)); Card->SetPadding(FMargin(0));
		auto* Layers = Tree.ConstructWidget<UOverlay>(); Card->AddChild(Layers);
		auto* Wash = Tree.ConstructWidget<UNativeWidgetHost>(); Wash->SetContent(SNew(SBroadcastReadingWash));
		Wash->SetVisibility(ESlateVisibility::HitTestInvisible);
		auto* WashSlot = Layers->AddChildToOverlay(Wash); WashSlot->SetPadding(FMargin(1));
		WashSlot->SetHorizontalAlignment(HAlign_Fill); WashSlot->SetVerticalAlignment(VAlign_Fill);
		auto* Columns = Tree.ConstructWidget<UHorizontalBox>();
		auto* ColumnLayer = Layers->AddChildToOverlay(Columns);
		ColumnLayer->SetHorizontalAlignment(HAlign_Fill); ColumnLayer->SetVerticalAlignment(VAlign_Fill);
		auto* Rail = Tree.ConstructWidget<UBorder>(UBorder::StaticClass(),Named(Prefix,TEXT("Rail")));
		Rounded(*Rail,FLinearColor::White,FLinearColor::Transparent,4.f); Rail->SetPadding(FMargin(0));
		auto* RailBounds = Tree.ConstructWidget<USizeBox>(); RailBounds->SetWidthOverride(9.f); RailBounds->AddChild(Rail);
		Columns->AddChildToHorizontalBox(RailBounds)->SetPadding(FMargin(1,1,0,1));
		auto* Left = Tree.ConstructWidget<UVerticalBox>();
		auto* LeftSlot = Columns->AddChildToHorizontalBox(Left);
		LeftSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill)); LeftSlot->SetPadding(FMargin(23,16,18,16));
		LeftSlot->SetVerticalAlignment(VAlign_Center);
		auto* Heading = Tree.ConstructWidget<UHorizontalBox>();
		Heading->AddChildToHorizontalBox(Text(Tree,Named(Prefix,TEXT("Side")),36))->SetVerticalAlignment(VAlign_Center);
		auto* Badge = Tree.ConstructWidget<UBorder>(UBorder::StaticClass(),Named(Prefix,TEXT("Active")));
		Rounded(*Badge,Aqua,Aqua,5.f); Badge->SetPadding(FMargin(10,3));
		auto* BadgeLabel = Text(Tree,NAME_None,16,Ink); Weight(*BadgeLabel,Ink);
		BadgeLabel->SetText(LOCTEXT("Current","当前")); Badge->AddChild(BadgeLabel);
		auto* BadgeSlot = Heading->AddChildToHorizontalBox(Badge); BadgeSlot->SetPadding(FMargin(12,0,0,0)); BadgeSlot->SetVerticalAlignment(VAlign_Center);
		Left->AddChildToVerticalBox(Heading)->SetPadding(FMargin(0,0,0,12));
		auto* Identities = Tree.ConstructWidget<UWrapBox>(UWrapBox::StaticClass(),Named(Prefix,TEXT("People")));
		Identities->SetInnerSlotPadding(FVector2D(24,8));
		Identities->SetExplicitWrapSize(true); Identities->SetWrapSize(430.f);
		Left->AddChildToVerticalBox(Identities);
		Left->AddChildToVerticalBox(Rule(Tree,0))->SetPadding(FMargin(0,12,0,10));
		auto* Equation = Tree.ConstructWidget<UWrapBox>(UWrapBox::StaticClass(),Named(Prefix,TEXT("Equation")));
		Equation->SetInnerSlotPadding(FVector2D(7,5));
		Equation->SetExplicitWrapSize(true); Equation->SetWrapSize(430.f);
		Left->AddChildToVerticalBox(Equation);

		auto* Total = Tree.ConstructWidget<UVerticalBox>();
		auto* TotalLabel = Text(Tree,Named(Prefix,TEXT("TotalLabel")),14,Quiet);
		TotalLabel->SetJustification(ETextJustify::Center); Total->AddChildToVerticalBox(TotalLabel);
		auto* NumberLine = Tree.ConstructWidget<UHorizontalBox>();
		auto* Number = Text(Tree,Named(Prefix,TEXT("Number")),64);
		NumberLine->AddChildToHorizontalBox(Number)->SetVerticalAlignment(VAlign_Center);
		auto* Pending = Tree.ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(),Named(Prefix,TEXT("Pending")));
		auto* Plus = Text(Tree,NAME_None,30,Quiet); Plus->SetText(FText::FromString(TEXT("+")));
		auto* PlusSlot = Pending->AddChildToHorizontalBox(Plus); PlusSlot->SetVerticalAlignment(VAlign_Center); PlusSlot->SetPadding(FMargin(10,0));
		auto* Unknown = Text(Tree,NAME_None,56,Quiet); Unknown->SetText(FText::FromString(TEXT("?")));
		Pending->AddChildToHorizontalBox(Unknown)->SetVerticalAlignment(VAlign_Center);
		NumberLine->AddChildToHorizontalBox(Pending)->SetVerticalAlignment(VAlign_Center);
		auto* ValueFit = Fit(Tree,NumberLine);
		CastChecked<UScaleBoxSlot>(ValueFit->GetContent()->Slot)->SetHorizontalAlignment(HAlign_Center);
		Total->AddChildToVerticalBox(ValueFit)->SetPadding(FMargin(0,6,0,3));
		auto* PendingLabel = Text(Tree,Named(Prefix,TEXT("PendingLabel")),12,Quiet);
		PendingLabel->SetJustification(ETextJustify::Center); Total->AddChildToVerticalBox(PendingLabel);
		auto* ValueCard = Tree.ConstructWidget<UBorder>(UBorder::StaticClass(),Named(Prefix,TEXT("ValueCard")));
		Rounded(*ValueCard,Alpha(Ink,.80f),Alpha(Color(100,177,198),.75f),10.f);
		ValueCard->SetPadding(FMargin(14,12)); ValueCard->SetVerticalAlignment(VAlign_Center); ValueCard->AddChild(Total);
		auto* TotalBounds = Tree.ConstructWidget<USizeBox>(); TotalBounds->SetWidthOverride(250.f);
		TotalBounds->SetMinDesiredHeight(164.f); TotalBounds->AddChild(ValueCard);
		auto* TotalSlot = Columns->AddChildToHorizontalBox(TotalBounds);
		TotalSlot->SetVerticalAlignment(VAlign_Fill); TotalSlot->SetPadding(FMargin(0,17,16,17));
		Body.AddChildToVerticalBox(Card);
	}

	void RefreshRow(UWidgetTree& Tree, const FString& Prefix,
		const FFMCodexUMGInlineFormulaRowViewModel& Row, E Emphasis, bool bRolling)
	{
		const bool bActive = Emphasis == E::Active;
		const bool bFinal = Row.bDisplayedResultResolved && Row.bDisplayedResultIsFinalValue;
		Rounded(*Find<UBorder>(Tree,Named(Prefix,TEXT("Card"))),
			Alpha(bActive ? Color(12,36,50) : Color(10,28,42),.80f),
			Alpha(bActive ? Color(119,219,226) : Color(111,157,183),.85f));
		Find<UBorder>(Tree,Named(Prefix,TEXT("Rail")))->SetBrushColor(bActive ? Aqua : Color(58,78,95));
		Find<UBorder>(Tree,Named(Prefix,TEXT("Active")))->SetVisibility(bActive ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
		SetText(Tree,Named(Prefix,TEXT("Side")),Row.SideLabel);
		Weight(*Find<UTextBlock>(Tree,Named(Prefix,TEXT("Side"))),bActive ? White : Color(195,217,231));
		auto* People = Find<UWrapBox>(Tree,Named(Prefix,TEXT("People")));
		while (People->GetChildrenCount() < Row.Participants.Num())
		{
			const int32 I = People->GetChildrenCount();
			auto* Person = Tree.ConstructWidget<UVerticalBox>();
			Person->AddChildToVerticalBox(Text(Tree,FName(*(Prefix+FString::Printf(TEXT("Role%d"),I))),11,Quiet));
			auto* Name = Text(Tree,FName(*(Prefix+FString::Printf(TEXT("Name%d"),I))),26); Weight(*Name,White);
			Person->AddChildToVerticalBox(Name);
			People->AddChildToWrapBox(Atomic(Tree,Person));
		}
		for (int32 I=0; I<People->GetChildrenCount(); ++I)
		{
			Show(*People->GetChildAt(I),Row.Participants.IsValidIndex(I));
			if (!Row.Participants.IsValidIndex(I)) continue;
			SetText(Tree,FName(*(Prefix+FString::Printf(TEXT("Role%d"),I))),Row.Participants[I].RoleLabel);
			SetText(Tree,FName(*(Prefix+FString::Printf(TEXT("Name%d"),I))),Row.Participants[I].PlayerName);
		}
		// Move a single unresolved roll to the large value anchor. Multiple-roll or
		// already-final shapes retain all terms rather than imply a false equation.
		int32 RollCount = 0; const FFMCodexUMGInlineFormulaTermViewModel* Roll = nullptr;
		for (const auto& Term : Row.Terms) if (Term.Kind == K::RawRoll) { ++RollCount; Roll = &Term; }
		const bool bPending = RollCount == 1 && !Roll->bResolved && !bFinal;
		const bool bCompact = CanOmitContributorNames(Row);
		auto* Equation = Find<UWrapBox>(Tree,Named(Prefix,TEXT("Equation")));
		while (Equation->GetChildrenCount() < Row.Terms.Num())
		{
			auto* Operand = Text(Tree,FName(*(Prefix+FString::Printf(TEXT("Term%d"),Equation->GetChildrenCount()))),17,Quiet);
			Equation->AddChildToWrapBox(Atomic(Tree,Operand));
		}
		bool bFirst = true;
		for (int32 I=0; I<Equation->GetChildrenCount(); ++I)
		{
			auto* Operand = Find<UTextBlock>(Tree,FName(*(Prefix+FString::Printf(TEXT("Term%d"),I))));
			const bool bVisible = Row.Terms.IsValidIndex(I) && !(bPending && Row.Terms[I].Kind == K::RawRoll);
			Show(*Equation->GetChildAt(I),bVisible); if (!bVisible) { Operand->SetText(FText::GetEmpty()); continue; }
			const auto& Term = Row.Terms[I];
			const FString Name = bCompact || Term.ContributorDisplayName.IsEmpty() ? FString() : Term.ContributorDisplayName + TEXT(" ");
			// Use projection-formatted operands verbatim. No term arithmetic or parsing.
			const FString Plus = bFirst || Term.Kind == K::FixedModifier ? TEXT("") : TEXT("+ ");
			Operand->SetText(FText::FromString(Plus + Name + Term.DisplayLabel));
			Operand->SetColorAndOpacity(Term.Kind == K::RawRoll && Term.bResolved ? Gold : Quiet);
			bFirst = false;
		}
		SetText(Tree,Named(Prefix,TEXT("Number")),Row.DisplayedResultLabel);
		Find<UTextBlock>(Tree,Named(Prefix,TEXT("Number")))->SetColorAndOpacity(bFinal ? Gold : White);
		Find<UTextBlock>(Tree,Named(Prefix,TEXT("TotalLabel")))->SetText(bFinal ? LOCTEXT("Final","最终值") : LOCTEXT("CurrentValue","当前值"));
		Show(*Tree.FindWidget(Named(Prefix,TEXT("Pending"))),bPending);
		auto* PendingLabel = Find<UTextBlock>(Tree,Named(Prefix,TEXT("PendingLabel")));
		PendingLabel->SetText(bPending ? bActive ? bRolling ? LOCTEXT("Rolling","掷点中") : LOCTEXT("Pending","待掷点") : LOCTEXT("Waiting","等待掷点") : FText::GetEmpty());
		PendingLabel->SetColorAndOpacity(bActive ? Aqua : Quiet);
	}
}

bool IsEnabledFor(const FFMCodexUMGInlineFormulaSurfaceViewModel& P, bool bEmbedded)
{
	return Mode.GetValueOnGameThread() == 1 && !bEmbedded && P.bVisible && P.bShowFormulaRows
		&& P.ContestId == FName(TEXT("Cross.High"));
}

bool CanOmitContributorNames(const FFMCodexUMGInlineFormulaRowViewModel& Row)
{
	// Identity order must explain attribute order one-to-one. Display-name equality
	// is only a redundancy guard, never gameplay identity, ownership or inference.
	int32 Index = 0;
	for (const auto& Term : Row.Terms)
	{
		if (Term.Kind != K::Attribute) continue;
		if (!Row.Participants.IsValidIndex(Index) || Term.ContributorDisplayName.IsEmpty()
			|| Row.Participants[Index].PlayerName != Term.ContributorDisplayName) return false;
		for (int32 Other=0; Other<Row.Participants.Num(); ++Other)
			if (Other != Index && Row.Participants[Other].PlayerName == Term.ContributorDisplayName) return false;
		++Index;
	}
	return Index > 0 && Index == Row.Participants.Num();
}

USizeBox* Build(UWidgetTree& Tree, UButton*& OutContinue)
{
	auto* Bounds = Tree.ConstructWidget<USizeBox>(USizeBox::StaticClass(),TEXT("BroadcastPrototype"));
	Bounds->SetMinDesiredWidth(800.f); Bounds->SetMaxDesiredWidth(820.f);
	// Open broadcast shell: no outer, header or footer plate. The title sits on
	// the real pitch; only the cards and content-sized context chips have backing.
	auto* Body = Tree.ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(),TEXT("BroadcastShell"));
	// The pitch overlay can allocate less height than this content desires. Keep
	// the complete prototype, including the clickable footer, inside that space.
	auto* ShellWidth = Tree.ConstructWidget<USizeBox>(); ShellWidth->SetWidthOverride(800.f); ShellWidth->AddChild(Body);
	auto* ShellFit = Fit(Tree,ShellWidth);
	CastChecked<UScaleBoxSlot>(ShellWidth->Slot)->SetHorizontalAlignment(HAlign_Center);
	Bounds->AddChild(ShellFit);
	auto* Heading = Tree.ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(),TEXT("BroadcastHeader"));
	auto* Crest = Tree.ConstructWidget<UHorizontalBox>();
	Crest->AddChildToHorizontalBox(Rule(Tree,84,White))->SetVerticalAlignment(VAlign_Center);
	Crest->AddChildToHorizontalBox(Icon(Tree,TEXT("BroadcastFootball"),30.f,false,White))->SetPadding(FMargin(14,0));
	Crest->AddChildToHorizontalBox(Rule(Tree,84,White))->SetVerticalAlignment(VAlign_Center);
	Heading->AddChildToVerticalBox(Crest)->SetHorizontalAlignment(HAlign_Center);
	auto* Title = Text(Tree,TEXT("BroadcastTitle"),46); Weight(*Title,White); Shadow(*Title); Title->SetJustification(ETextJustify::Center);
	Heading->AddChildToVerticalBox(Title)->SetPadding(FMargin(0,5,0,0));
	auto* Outcome = FMCodexOutcomePresentation::BuildPrimary(Tree,TEXT("BroadcastOutcomeHeading"));
	Outcome->SetDefaultShadowOffset(FVector2D(0,2)); Outcome->SetDefaultShadowColorAndOpacity(Alpha(Ink,.85f));
	Heading->AddChildToVerticalBox(Outcome);
	auto* Status = Text(Tree,TEXT("BroadcastStatus"),22,Quiet); Shadow(*Status); Status->SetJustification(ETextJustify::Center);
	Heading->AddChildToVerticalBox(Status)->SetPadding(FMargin(0,3,0,0));
	Body->AddChildToVerticalBox(Heading)->SetPadding(FMargin(0,0,0,18));
	BuildRow(Tree,*Body,TEXT("BroadcastAttack"));
	auto* Versus = Tree.ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(),TEXT("BroadcastVersus"));
	Versus->AddChildToHorizontalBox(Rule(Tree,160,White))->SetVerticalAlignment(VAlign_Center);
	auto* VSPlate = Tree.ConstructWidget<UBorder>(UBorder::StaticClass(),TEXT("BroadcastVSChip"));
	Rounded(*VSPlate,Alpha(Ink,.88f),Alpha(Color(116,181,195),.75f),9.f); VSPlate->SetPadding(FMargin(0));
	VSPlate->SetHorizontalAlignment(HAlign_Center); VSPlate->SetVerticalAlignment(VAlign_Center);
	auto* VS = Text(Tree,NAME_None,21); VS->SetText(FText::FromString(TEXT("VS"))); VSPlate->AddChild(VS);
	auto* VSBounds = Tree.ConstructWidget<USizeBox>(); VSBounds->SetWidthOverride(82.f); VSBounds->SetHeightOverride(36.f); VSBounds->AddChild(VSPlate);
	Versus->AddChildToHorizontalBox(VSBounds)->SetPadding(FMargin(14,0));
	Versus->AddChildToHorizontalBox(Rule(Tree,160,White))->SetVerticalAlignment(VAlign_Center);
	auto* VersusSlot = Body->AddChildToVerticalBox(Versus); VersusSlot->SetHorizontalAlignment(HAlign_Center); VersusSlot->SetPadding(FMargin(0,7));
	BuildRow(Tree,*Body,TEXT("BroadcastDefense"));

	auto* Footer = Tree.ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(),TEXT("BroadcastFooter"));
	Body->AddChildToVerticalBox(Footer)->SetPadding(FMargin(0,12,0,0));
	auto* Detail = Text(Tree,TEXT("BroadcastDetail"),11,Quiet); Shadow(*Detail); Detail->SetJustification(ETextJustify::Center); Detail->SetAutoWrapText(true);
	Footer->AddChildToVerticalBox(Detail)->SetPadding(FMargin(0,0,0,5));
	auto* Context = Text(Tree,TEXT("BroadcastActionContext"),18); Weight(*Context,White); Context->SetJustification(ETextJustify::Center);
	auto* ContextPlate = Tree.ConstructWidget<UBorder>(UBorder::StaticClass(),TEXT("BroadcastContextChip"));
	Rounded(*ContextPlate,Alpha(Ink,.87f),Alpha(Color(116,181,195),.65f),10.f); ContextPlate->SetPadding(FMargin(18,7));
	// The short typed/projected status sizes its own chip. Avoid nesting another
	// content-sized ScaleBox inside the shell's viewport fit.
	ContextPlate->AddChild(Context);
	auto* ContextSlot = Footer->AddChildToVerticalBox(ContextPlate); ContextSlot->SetHorizontalAlignment(HAlign_Center); ContextSlot->SetPadding(FMargin(0,0,0,9));
	auto* ReelLine = Tree.ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(),TEXT("BroadcastRollHost"));
	auto* Owner = Text(Tree,TEXT("BroadcastRollOwner"),16,Quiet);
	auto* OwnerSlot = ReelLine->AddChildToHorizontalBox(Owner); OwnerSlot->SetVerticalAlignment(VAlign_Center); OwnerSlot->SetPadding(FMargin(0,0,15,0));
	auto* ReelBounds = Tree.ConstructWidget<USizeBox>(); ReelBounds->SetWidthOverride(68.f); ReelBounds->SetHeightOverride(76.f);
	auto* Reel = Tree.ConstructWidget<UFMCodexRollReelWidget>(UFMCodexRollReelWidget::StaticClass(),TEXT("BroadcastReel")); ReelBounds->AddChild(Reel);
	ReelLine->AddChildToHorizontalBox(ReelBounds); Footer->AddChildToVerticalBox(ReelLine)->SetHorizontalAlignment(HAlign_Center);
	auto* ActionBounds = Tree.ConstructWidget<USizeBox>(USizeBox::StaticClass(),TEXT("BroadcastActionBounds"));
	ActionBounds->SetWidthOverride(300.f); ActionBounds->SetHeightOverride(72.f);
	OutContinue = Tree.ConstructWidget<UButton>(UButton::StaticClass(),TEXT("BroadcastContinue"));
	FButtonStyle ButtonStyle;
	ButtonStyle.SetNormal(FSlateRoundedBoxBrush(Aqua,8.f));
	ButtonStyle.SetHovered(FSlateRoundedBoxBrush(Color(133,248,244),8.f,White,1.f));
	ButtonStyle.SetPressed(FSlateRoundedBoxBrush(Color(45,179,180),8.f));
	ButtonStyle.SetDisabled(FSlateRoundedBoxBrush(Color(47,75,83),8.f));
	ButtonStyle.SetNormalPadding(FMargin(16,6)); ButtonStyle.SetPressedPadding(FMargin(16,8,16,4));
	OutContinue->SetStyle(ButtonStyle);
	auto* ButtonContent = Tree.ConstructWidget<UHorizontalBox>();
	auto* DiceGroup = Tree.ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(),TEXT("BroadcastDiceIcon"));
	DiceGroup->AddChildToHorizontalBox(Icon(Tree,NAME_None,40.f,true,Ink))->SetVerticalAlignment(VAlign_Center);
	auto* Divider = Tree.ConstructWidget<UBorder>(); Divider->SetPadding(FMargin(0)); Divider->SetBrushColor(Alpha(Ink,.65f));
	auto* DividerBounds = Tree.ConstructWidget<USizeBox>(); DividerBounds->SetWidthOverride(1.f); DividerBounds->SetHeightOverride(36.f); DividerBounds->AddChild(Divider);
	auto* DividerSlot = DiceGroup->AddChildToHorizontalBox(DividerBounds); DividerSlot->SetVerticalAlignment(VAlign_Center); DividerSlot->SetPadding(FMargin(24,0));
	ButtonContent->AddChildToHorizontalBox(DiceGroup);
	auto* Label = Text(Tree,TEXT("BroadcastContinueLabel"),30,Ink); Weight(*Label,Ink); Label->SetJustification(ETextJustify::Center);
	ButtonContent->AddChildToHorizontalBox(Label)->SetVerticalAlignment(VAlign_Center);
	auto* ButtonSlot = CastChecked<UButtonSlot>(OutContinue->AddChild(ButtonContent));
	ButtonSlot->SetHorizontalAlignment(HAlign_Center); ButtonSlot->SetVerticalAlignment(VAlign_Center);
	ActionBounds->AddChild(OutContinue);
	Footer->AddChildToVerticalBox(ActionBounds)->SetHorizontalAlignment(HAlign_Center);
	return Bounds;
}

void Refresh(UWidgetTree& Tree, const FFMCodexUMGInlineFormulaSurfaceViewModel& P, E Attack, E Defense)
{
	const bool bFinal = FMCodexOutcomePresentation::IsFinalReady(P.bNarrativeAvailable,P.bDiceRevealVisible);
	SetText(Tree,TEXT("BroadcastTitle"), P.bNarrativeAvailable ? P.ResolutionContextLabel.IsEmpty()
		? FFMCodexPlayerUIPresentationText::ResolutionContest(P.ContestId).ToString() : P.ResolutionContextLabel : P.ContestLabel);
	Show(*Tree.FindWidget(TEXT("BroadcastTitle")),!bFinal);
	auto* Headline = Find<URichTextBlock>(Tree,TEXT("BroadcastOutcomeHeading"));
	Headline->SetText(bFinal ? FText::FromString(FMCodexOutcomePresentation::PrimaryMarkup(P.ContestLabel,P.OutcomeText)) : FText::GetEmpty()); Show(*Headline,bFinal);
	Find<UTextBlock>(Tree,TEXT("BroadcastStatus"))->SetText(bFinal ? LOCTEXT("Result","结算结果") : LOCTEXT("GoalContest","进球判定"));
	TArray<FString> Details;
	if (!P.bParentOwnsRouteContext && !P.RouteResultLabel.IsEmpty()) Details.Add(P.RouteResultLabel);
	if (!P.RollHelperLabel.IsEmpty()) Details.Add(P.RollHelperLabel);
	if (!P.TacticalPlayerSummaryLabel.IsEmpty()) Details.Add(P.TacticalPlayerSummaryLabel);
	SetText(Tree,TEXT("BroadcastDetail"),FString::Join(Details,TEXT(" · ")));
	RefreshRow(Tree,TEXT("BroadcastAttack"),P.AttackRow,Attack,P.bDiceRevealVisible);
	RefreshRow(Tree,TEXT("BroadcastDefense"),P.DefenseRow,Defense,P.bDiceRevealVisible);
	Show(*Tree.FindWidget(TEXT("BroadcastAttackCard")),P.bShowAttackRow);
	Show(*Tree.FindWidget(TEXT("BroadcastDefenseCard")),P.bShowDefenseRow);
	Show(*Tree.FindWidget(TEXT("BroadcastVersus")),P.bShowAttackRow && P.bShowDefenseRow);
	Show(*Tree.FindWidget(TEXT("BroadcastRollHost")),P.bDiceRevealVisible);
	SetText(Tree,TEXT("BroadcastRollOwner"),P.DiceOwnerLabel);
	Find<UFMCodexRollReelWidget>(Tree,TEXT("BroadcastReel"))->RefreshFromPresentation(P.RollReel);
	const bool bAction = P.bVisible && P.PrimaryAction.bVisible && P.PrimaryAction.Action.bAvailable;
	Show(*Tree.FindWidget(TEXT("BroadcastActionBounds")),bAction);
	Find<UButton>(Tree,TEXT("BroadcastContinue"))->SetIsEnabled(P.PrimaryAction.Action.bAvailable);
	using C = EFMCodexUMGInteractionCategory;
	const C Category = P.PrimaryAction.Action.Category;
	const bool bRoll = Category == C::RollCrossAttack || Category == C::RollCrossDefense;
	// Typed action owns the CTA; waiting viewers retain the safe projected status.
	Find<UTextBlock>(Tree,TEXT("BroadcastActionContext"))->SetText(bAction && bRoll
		? Category == C::RollCrossAttack ? LOCTEXT("AttackTurn","轮到进攻方掷点") : LOCTEXT("DefenseTurn","轮到防守方掷点")
		: P.bDiceRevealVisible ? LOCTEXT("Processing","正在结算") : FText::FromString(P.StatusLabel));
	Find<UTextBlock>(Tree,TEXT("BroadcastContinueLabel"))->SetText(bRoll ? LOCTEXT("RollCTA","掷点") : FText::FromString(P.PrimaryAction.Action.Label));
	Show(*Tree.FindWidget(TEXT("BroadcastDiceIcon")),bRoll);
}
}
#undef LOCTEXT_NAMESPACE
#endif
