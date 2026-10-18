#pragma once

#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"

namespace FMCodexFormulaTooltipTestUtils
{
// Assert player-visible content without coupling tests to the tooltip's child layout.
inline FString Text(const UWidget* Widget)
{
 if(!Widget || Widget->GetVisibility()==ESlateVisibility::Collapsed || Widget->GetVisibility()==ESlateVisibility::Hidden)return {};
 if(const auto* Label=Cast<UTextBlock>(Widget))return Label->GetText().ToString();
 TArray<FString> Parts;
 if(const auto* Panel=Cast<UPanelWidget>(Widget))
  for(int32 I=0;I<Panel->GetChildrenCount();++I)
  {
   const auto Part=Text(Panel->GetChildAt(I));
   if(!Part.IsEmpty())Parts.Add(Part);
  }
 return FString::Join(Parts,TEXT(" "));
}
}
#endif
