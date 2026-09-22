#pragma once

#include "CoreMinimal.h"
#include "FMCodexOutcomeText.h"

class UWidgetTree;
class URichTextBlock;
class UVerticalBox;
class UButton;
class USizeBox;
class UFMCodexMatchFlowPanel;
struct FFMCodexUMGInlineFormulaSurfaceViewModel;

/** Read-only composition for already-disclosed prose. No outcome classification or actions. */
namespace FMCodexOutcomePresentation
{
	void Build(UWidgetTree& Tree, UVerticalBox& Parent);
	URichTextBlock* BuildPrimary(UWidgetTree& Tree, FName Name);
	bool IsFinalReady(bool bNarrativeAvailable, bool bDiceRevealVisible);
	void RefreshIntermediate(UWidgetTree& Tree, bool bVisible, const FString& Context, const FString& Detail);
	void Refresh(UWidgetTree& Tree, bool bVisible, const FString& Primary,
		const FString& Context, const FString& Detail, const FString& Supporting,
		bool bShowFooter, const FFMCodexOutcomeText& Segments);
	bool OwnsInlineSurface(const FFMCodexUMGInlineFormulaSurfaceViewModel& Presentation);
	FString PrimaryMarkup(const FString& CanonicalText, const FFMCodexOutcomeText& Segments);
	FLinearColor AccentColor(EFMCodexOutcomeAccent Accent);
	void ApplyActionStyle(UButton& Button);
	void ApplyFrameStyle(UFMCodexMatchFlowPanel& Frame, USizeBox& Bounds, bool bEmbedded = false);
	FString JoinContext(const TArray<FString>& Labels);
}
