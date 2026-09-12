#pragma once

#include "CoreMinimal.h"
#include "Components/Border.h"
#include "FMCodexFullCardSurface.generated.h"

// Full-only drawing vocabulary. None preserves the ordinary Border fallback.
enum class EFMCodexFullCardSurface : uint8
{
    None, Frame, Body, Biography, Identity, Number, Section, Attributes, AttributeRow, Value, SkillRow, Range, RuleLeft, RuleRight, Footer
};

// Scaled from the target's nested 45-degree frame and cut panel family.
namespace FMCodexFullCardGeometry
{
    constexpr float OuterInset = 2.f;
    constexpr float OuterCut = 16.f;
    constexpr float ChannelInset = 5.f;
    constexpr float ChannelCut = 13.f;
    constexpr float InnerInset = 8.f;
    constexpr float InnerCut = 10.f;
    constexpr float PanelCut = 10.f;
    constexpr float RowCut = 5.f;
    constexpr float MainStroke = 1.4f;
    constexpr float DetailStroke = .65f;
    // One neutral metal identity for all interior structure, independent of rarity/tier.
    inline FLinearColor StructuralInk() { return FLinearColor(.22f,.32f,.40f,1.f); }
}

/** Small convex Slate panels; no assets, materials, animation or gameplay data. */
UCLASS()
class FMCODEX_API UFMCodexFullCardSurface : public UBorder
{
    GENERATED_BODY()
public:
    void SetSurface(EFMCodexFullCardSurface InSurface, const FLinearColor& InAccent);
    EFMCodexFullCardSurface GetSurface() const { return Surface; }
    FLinearColor GetAccent() const { return Accent; }
protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;
private:
    EFMCodexFullCardSurface Surface = EFMCodexFullCardSurface::None;
    FLinearColor Accent = FLinearColor::White;
};
