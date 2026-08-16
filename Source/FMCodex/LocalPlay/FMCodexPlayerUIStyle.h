#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateTypes.h"

class UBorder;
class UButton;
class UTextBlock;

enum class EFMCodexPlayerUIColorRole : uint8
{
	ScreenBackground,
	PanelBackground,
	PanelRaised,
	PanelInset,
	PlayerAAccent,
	PlayerBAccent,
	NeutralAccent,
	PitchBackground,
	PitchHalf,
	PitchAttackingHalf,
	PitchCenterLine,
	EmptyPitchSlot,
	OccupiedPitchSlot,
	CardFrame,
	GoalkeeperCardFrame,
	SkillBadge,
	AttributeCell,
	StatusAvailable,
	StatusUsed,
	StatusActive,
	ActionPrimary,
	ActionSecondary,
	ActionDecline,
	ActionDisabled,
	SystemStatus,
	Success,
	Warning,
	Danger,
	TerminalNeutral
};

enum class EFMCodexPlayerUITextRole : uint8
{
	Secondary,
	Body,
	Kicker,
	SectionHeading,
	Identity,
	ActionTitle,
	Score,
	Status,
	DiceValue,
	TerminalResult,
	HandoffTitle,
	HandoffPlayer
};

enum class EFMCodexPlayerUIActionRole : uint8
{
	Primary,
	Secondary,
	Decline,
	Disabled
};

/**
 * Immutable, presentation-only visual vocabulary shared by LocalPlay UMG.
 * It owns no gameplay state and performs no gameplay queries.
 */
struct FMCODEX_API FFMCodexPlayerUIStyle
{
	static const FFMCodexPlayerUIStyle& Get();

	float GetPanelMinWidth() const;
	float GetPanelMaxWidth() const;
	FMargin GetOuterPadding() const;
	FMargin GetPanelPadding() const;
	FMargin GetSectionPadding() const;
	FMargin GetCompactPadding() const;
	FVector2D GetControlGap() const;

	FLinearColor GetColor(EFMCodexPlayerUIColorRole Role) const;
	int32 GetFontSize(EFMCodexPlayerUITextRole Role) const;
	FLinearColor GetPlayerAccentColor(const FString& PresentationLabel) const;
	FLinearColor GetRarityAccentColor(const FString& CanonicalLabel) const;
	FLinearColor GetStatusBadgeColor(const FString& PresentationLabel) const;
	FLinearColor GetTerminalColor(const FString& PresentationLabel) const;
	FButtonStyle MakeButtonStyle(EFMCodexPlayerUIActionRole Role) const;

	void ApplyText(UTextBlock& Text, EFMCodexPlayerUITextRole Role) const;
	void ApplyBorder(
		UBorder& Border,
		EFMCodexPlayerUIColorRole Role,
		const FMargin& Padding) const;
	void ApplyButton(UButton& Button, EFMCodexPlayerUIActionRole Role) const;

	bool HasValidDefaults() const;
};
