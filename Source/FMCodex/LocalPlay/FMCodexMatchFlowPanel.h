#pragma once

#include "CoreMinimal.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "FMCodexMatchFlowPanel.generated.h"

/** Opt-in decoration for type information and method choices. No input or flow state. */
UCLASS()
class FMCODEX_API UFMCodexMatchFlowPanel final : public UBorder
{
	GENERATED_BODY()
public:
	void SetFlowStyleEnabled(bool bEnabled);
	bool IsFlowStyleEnabled() const { return bFlowStyleEnabled; }
	void SetRuleCardStyle() { bFlowStyleEnabled = true; bRuleCard = true; }
	bool IsRuleCard() const { return bRuleCard; }
protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
private:
	bool bFlowStyleEnabled = false;
	bool bRuleCard = false;
};

/** The original UButton input contract with opt-in procedural decoration. */
UCLASS()
class FMCODEX_API UFMCodexMatchFlowButton final : public UButton
{
	GENERATED_BODY()
public:
	void SetFlowStyleEnabled(bool bEnabled);
	bool IsFlowStyleEnabled() const { return bFlowStyleEnabled; }
protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
private:
	bool bFlowStyleEnabled = false;
};

// Static pictograms only: no selected state, result, legality or match data.
enum class EFMCodexFlowDiagram : uint8
{
	Corner, LongFreeKick, ShortFreeKick, Penalty,
	Direct, Combination, Power, Panenka, HighCross, LowCross
};

UCLASS()
class FMCODEX_API UFMCodexMatchFlowDiagram final : public UWidget
{
	GENERATED_BODY()
public:
	void SetDiagram(EFMCodexFlowDiagram InDiagram) { Diagram = InDiagram; }
	EFMCodexFlowDiagram GetDiagram() const { return Diagram; }
	static FVector2D ViewportSize() { return FVector2D(72, 52); }
protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
private:
	EFMCodexFlowDiagram Diagram = EFMCodexFlowDiagram::Corner;
};
