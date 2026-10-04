#pragma once
#include "CoreMinimal.h"
#include "Fonts/CompositeFont.h"
#include "Fonts/SlateFontInfo.h"
#include "Misc/Paths.h"

// Match shell only. Player/card rarity and resolution-family themes stay independent.
namespace FMCodexMatchShellStyle
{
inline FLinearColor Color(uint8 R,uint8 G,uint8 B,uint8 A=255)
{ return FLinearColor::FromSRGBColor(FColor(R,G,B,A)); }
inline FLinearColor Navy() { return Color(5,23,37); }
inline FLinearColor Border() { return Color(61,105,129); }
inline FLinearColor Mint() { return Color(43,225,200); }
inline FLinearColor Ink() { return Color(4,29,38); }
inline FLinearColor Text() { return Color(240,247,252); }
inline FLinearColor Secondary() { return Color(160,188,204); }
// Explicit Chinese weights for Header / Dock only; other UI keeps its existing font.
inline FSlateFontInfo Font(float Size, bool bBold = false)
{
	static const TSharedPtr<const FCompositeFont> Typeface = []()
	{
		auto Result = MakeShared<FCompositeFont>();
		const FString Root = FPaths::ProjectContentDir() / TEXT("Slate/Fonts/MatchShell");
		Result->DefaultTypeface.Fonts.Emplace(TEXT("Regular"), Root / TEXT("NotoSansSC-Regular.otf"), EFontHinting::Default, EFontLoadingPolicy::LazyLoad);
		Result->DefaultTypeface.Fonts.Emplace(TEXT("Bold"), Root / TEXT("NotoSansSC-Bold.otf"), EFontHinting::Default, EFontLoadingPolicy::LazyLoad);
		return Result;
	}();
	return FSlateFontInfo(Typeface, Size, bBold ? TEXT("Bold") : TEXT("Regular"));
}
inline FLinearColor HeaderAccent(const FLinearColor& Source)
{
	// Restyle only the default shell identity inks, not the projected palette or cards.
	if (Source.Equals(Color(79,120,146))) return Color(8,127,245);
	if (Source.Equals(Color(164,71,79))) return Color(218,25,50);
	return Source;
}
inline FLinearColor DisplayAccent(FLinearColor Color)
{
	// Preserve hue; constrain the rendered edge luminance, never the stored identity.
	Color=Color.GetClamped();
	FLinearColor HSV=Color.LinearRGBToHSV();
	HSV.B=FMath::Clamp(HSV.B,.38f,.85f);
	FLinearColor Result=HSV.HSVToLinearRGB(); Result.A=1.f;
	return Result;
}
inline FLinearColor CurrentTurnFill(const FLinearColor& Source)
{
	if (Source.Equals(Color(79,120,146))) return Color(110,152,185);
	return FMath::Lerp(DisplayAccent(HeaderAccent(Source)), FLinearColor::White, .18f);
}
}
