#pragma once

#include "CoreMinimal.h"

namespace FMCodexRollPresentationStyle
{
	// Shared production roll glyph colors only, never Formula totals or results.
	inline FLinearColor UnresolvedValue()
	{
		return FLinearColor::FromSRGBColor(FColor(224, 243, 249));
	}
	inline FLinearColor AuthoritativeLandedValue()
	{
		// Frozen sRGB token: #EED7A6. Convert once into Slate's linear color space.
		return FLinearColor::FromSRGBColor(FColor(238, 215, 166));
	}
}
