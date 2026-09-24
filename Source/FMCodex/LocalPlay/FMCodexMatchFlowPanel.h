#pragma once

#include "CoreMinimal.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "FMCodexMatchFlowPanel.generated.h"

// Paint roles for read-only formula furniture, separate from A/C decoration.
enum class EFMCodexFormulaPanelRole : uint8 { None, Section, Value, RollHost };
// Read-only attention roles. None preserves accepted non-formula paint.
enum class EFMCodexFormulaEmphasis : uint8 { None, Active, Context, Resolved };

/** Opt-in decoration for information, choice and formula surfaces. No gameplay state. */
UCLASS()
class FMCODEX_API UFMCodexMatchFlowPanel final : public UBorder
{
	GENERATED_BODY()
public:
	void SetFlowStyleEnabled(bool bEnabled);
	bool IsFlowStyleEnabled() const { return bFlowStyleEnabled; }
	void SetRuleCardStyle() { bFlowStyleEnabled = true; bRuleCard = true; }
	void SetContestRowStyle(bool bActive);
	bool IsContestRow() const { return bContestRow; }
	bool IsActiveContestRow() const { return bActiveContestRow; }
	bool IsRuleCard() const { return bRuleCard; }
	void SetFormulaRole(EFMCodexFormulaPanelRole Role, bool bFinal = false);
	EFMCodexFormulaPanelRole GetFormulaRole() const { return FormulaRole; }
	bool IsFinalFormulaValue() const { return bFinalFormulaValue; }
	void SetFormulaEmphasis(EFMCodexFormulaEmphasis InEmphasis);
	EFMCodexFormulaEmphasis GetFormulaEmphasis() const { return FormulaEmphasis; }
protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
private:
	bool bFlowStyleEnabled = false;
	bool bRuleCard = false;
	bool bContestRow = false;
	bool bActiveContestRow = false;
	EFMCodexFormulaPanelRole FormulaRole = EFMCodexFormulaPanelRole::None;
	bool bFinalFormulaValue = false;
	EFMCodexFormulaEmphasis FormulaEmphasis = EFMCodexFormulaEmphasis::None;
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
	Direct, Combination, Power, Panenka, HighCross, LowCross,
	FormulaAttack, FormulaDefense
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
