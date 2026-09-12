#include "FMCodexPlayerCardSurface.h"

#include "Components/BorderSlot.h"
#include "Rendering/DrawElements.h"
#include "Widgets/Layout/SBorder.h"

namespace
{
class SFMCodexPlayerCardSurface : public SBorder
{
public:
	SLATE_BEGIN_ARGS(SFMCodexPlayerCardSurface) {} SLATE_END_ARGS()
	void Construct(const FArguments&, UFMCodexPlayerCardSurface* InOwner)
	{
		Owner = InOwner;
		SBorder::Construct(SBorder::FArguments());
	}
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& G,
		const FSlateRect& Cull, FSlateWindowElementList& Out, int32 Layer,
		const FWidgetStyle& Style, bool bEnabled) const override
	{
		if (!Owner.IsValid() || !Owner->bPremium)
			return SBorder::OnPaint(Args,G,Cull,Out,Layer,Style,bEnabled);
		const FVector2f Size(G.GetLocalSize());
		const float W=Size.X,H=Size.Y;
		const FLinearColor Tint=Style.GetColorAndOpacityTint();
		const ESlateDrawEffect Effect=ShouldBeEnabled(bEnabled)
			? ESlateDrawEffect::None : ESlateDrawEffect::DisabledEffect;
        const bool Hand=Owner->GetFName()==TEXT("HandMicroIdentitySurface");
        if (Hand)
        {
            FSlateDrawElement::MakeGradient(Out,Layer,G.ToPaintGeometry(),
                {FSlateGradientStop(FVector2f(0,0),FLinearColor(.0055f,.018f,.034f,1)*Tint),
                 FSlateGradientStop(FVector2f(0,H*.42f),FLinearColor(.003f,.011f,.023f,1)*Tint),
                 FSlateGradientStop(FVector2f(0,H),FLinearColor(.0018f,.006f,.012f,1)*Tint)},
                Orient_Horizontal,Effect);
            FSlateDrawElement::MakeLines(Out,Layer+1,G.ToPaintGeometry(),
                TArray<FVector2f>{{W*.62f,H-3},{W-4,H*.22f}},Effect,FLinearColor(.09f,.22f,.36f,.12f)*Tint,true,1);
            FSlateDrawElement::MakeLines(Out,Layer+1,G.ToPaintGeometry(),
                TArray<FVector2f>{{W*.72f,H-3},{W-4,H*.40f}},Effect,FLinearColor(.09f,.22f,.36f,.055f)*Tint,true,1);
            FSlateDrawElement::MakeLines(Out,Layer+1,G.ToPaintGeometry(),
                TArray<FVector2f>{{5,5},{W-7,5}},Effect,FLinearColor(.17f,.32f,.45f,.14f)*Tint,true,1);
            return SCompoundWidget::OnPaint(Args,G,Cull,Out,Layer+2,Style,ShouldBeEnabled(bEnabled));
        }
		return SBorder::OnPaint(Args,G,Cull,Out,Layer,Style,bEnabled);
	}
private:
	TWeakObjectPtr<UFMCodexPlayerCardSurface> Owner;
};
}
TSharedRef<SWidget> UFMCodexPlayerCardSurface::RebuildWidget()
{
	MyBorder=SNew(SFMCodexPlayerCardSurface,this);
	if (GetChildrenCount()>0)
		CastChecked<UBorderSlot>(GetContentSlot())->BuildSlot(MyBorder.ToSharedRef());
	return MyBorder.ToSharedRef();
}
