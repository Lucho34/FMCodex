#include "FMCodexPlayerUIStyle.h"

#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

namespace FMCodexPlayerUIStyle
{
	FSlateBrush MakeSolidBrush(const FLinearColor& Color)
	{
		FSlateBrush Brush;
		Brush.DrawAs = ESlateBrushDrawType::Box;
		Brush.Margin = FMargin(0.18f);
		Brush.TintColor = FSlateColor(Color);
		return Brush;
	}

	FLinearColor Emphasize(const FLinearColor& Color, const float Multiplier)
	{
		return FLinearColor(
			FMath::Clamp(Color.R * Multiplier, 0.0f, 1.0f),
			FMath::Clamp(Color.G * Multiplier, 0.0f, 1.0f),
			FMath::Clamp(Color.B * Multiplier, 0.0f, 1.0f),
			Color.A);
	}
}

const FFMCodexPlayerUIStyle& FFMCodexPlayerUIStyle::Get()
{
	static const FFMCodexPlayerUIStyle Style;
	return Style;
}

float FFMCodexPlayerUIStyle::GetPanelMinWidth() const
{
	return 840.0f;
}

float FFMCodexPlayerUIStyle::GetPanelMaxWidth() const
{
	return 1120.0f;
}

FMargin FFMCodexPlayerUIStyle::GetOuterPadding() const
{
	return FMargin(10.0f);
}

FMargin FFMCodexPlayerUIStyle::GetPanelPadding() const
{
	return FMargin(12.0f);
}

FMargin FFMCodexPlayerUIStyle::GetSectionPadding() const
{
	return FMargin(8.0f);
}

FMargin FFMCodexPlayerUIStyle::GetCompactPadding() const
{
	return FMargin(4.0f);
}

FVector2D FFMCodexPlayerUIStyle::GetControlGap() const
{
	return FVector2D(6.0f, 6.0f);
}

FLinearColor FFMCodexPlayerUIStyle::GetColor(
	const EFMCodexPlayerUIColorRole Role) const
{
	switch (Role)
	{
	case EFMCodexPlayerUIColorRole::ScreenBackground:
		return FLinearColor(0.008f, 0.015f, 0.025f, 1.0f);
	case EFMCodexPlayerUIColorRole::PanelBackground:
		return FLinearColor(0.018f, 0.035f, 0.055f, 0.985f);
	case EFMCodexPlayerUIColorRole::PanelRaised:
		return FLinearColor(0.035f, 0.075f, 0.105f, 1.0f);
	case EFMCodexPlayerUIColorRole::PanelInset:
		return FLinearColor(0.025f, 0.065f, 0.085f, 1.0f);
	case EFMCodexPlayerUIColorRole::PlayerAAccent:
		return FLinearColor(0.10f, 0.34f, 0.56f, 1.0f);
	case EFMCodexPlayerUIColorRole::PlayerBAccent:
		return FLinearColor(0.55f, 0.16f, 0.23f, 1.0f);
	case EFMCodexPlayerUIColorRole::NeutralAccent:
		return FLinearColor(0.12f, 0.24f, 0.32f, 1.0f);
	case EFMCodexPlayerUIColorRole::PitchBackground:
		return FLinearColor(0.015f, 0.18f, 0.072f, 1.0f);
	case EFMCodexPlayerUIColorRole::PitchHalf:
		return FLinearColor(0.025f, 0.25f, 0.10f, 0.96f);
	case EFMCodexPlayerUIColorRole::PitchAttackingHalf:
		return FLinearColor(0.32f, 0.27f, 0.045f, 0.98f);
	case EFMCodexPlayerUIColorRole::PitchCenterLine:
		return FLinearColor(0.72f, 0.80f, 0.70f, 0.94f);
	case EFMCodexPlayerUIColorRole::EmptyPitchSlot:
		return FLinearColor(0.025f, 0.105f, 0.072f, 0.88f);
	case EFMCodexPlayerUIColorRole::OccupiedPitchSlot:
		return FLinearColor(0.025f, 0.09f, 0.14f, 0.98f);
	case EFMCodexPlayerUIColorRole::CardFrame:
		return FLinearColor(0.025f, 0.055f, 0.085f, 0.995f);
	case EFMCodexPlayerUIColorRole::GoalkeeperCardFrame:
		return FLinearColor(0.20f, 0.135f, 0.022f, 0.995f);
	case EFMCodexPlayerUIColorRole::SkillBadge:
		return FLinearColor(0.07f, 0.25f, 0.27f, 1.0f);
	case EFMCodexPlayerUIColorRole::AttributeCell:
		return FLinearColor(0.04f, 0.125f, 0.17f, 1.0f);
	case EFMCodexPlayerUIColorRole::StatusAvailable:
		return FLinearColor(0.06f, 0.34f, 0.18f, 1.0f);
	case EFMCodexPlayerUIColorRole::StatusUsed:
		return FLinearColor(0.34f, 0.075f, 0.075f, 1.0f);
	case EFMCodexPlayerUIColorRole::StatusActive:
		return FLinearColor(0.38f, 0.26f, 0.035f, 1.0f);
	case EFMCodexPlayerUIColorRole::ActionPrimary:
		return FLinearColor(0.035f, 0.36f, 0.22f, 1.0f);
	case EFMCodexPlayerUIColorRole::ActionSecondary:
		return FLinearColor(0.055f, 0.18f, 0.28f, 1.0f);
	case EFMCodexPlayerUIColorRole::ActionDecline:
		return FLinearColor(0.36f, 0.055f, 0.07f, 1.0f);
	case EFMCodexPlayerUIColorRole::ActionDisabled:
		return FLinearColor(0.075f, 0.085f, 0.10f, 0.80f);
	case EFMCodexPlayerUIColorRole::SystemStatus:
		return FLinearColor(0.25f, 0.15f, 0.43f, 1.0f);
	case EFMCodexPlayerUIColorRole::Success:
		return FLinearColor(0.045f, 0.37f, 0.17f, 1.0f);
	case EFMCodexPlayerUIColorRole::Warning:
		return FLinearColor(0.42f, 0.26f, 0.035f, 1.0f);
	case EFMCodexPlayerUIColorRole::Danger:
		return FLinearColor(0.42f, 0.045f, 0.055f, 1.0f);
	case EFMCodexPlayerUIColorRole::TerminalNeutral:
	default:
		return FLinearColor(0.14f, 0.17f, 0.21f, 1.0f);
	}
}

int32 FFMCodexPlayerUIStyle::GetFontSize(
	const EFMCodexPlayerUITextRole Role) const
{
	switch (Role)
	{
	case EFMCodexPlayerUITextRole::Secondary: return 11;
	case EFMCodexPlayerUITextRole::Body: return 13;
	case EFMCodexPlayerUITextRole::Kicker: return 12;
	case EFMCodexPlayerUITextRole::SectionHeading: return 14;
	case EFMCodexPlayerUITextRole::Identity: return 18;
	case EFMCodexPlayerUITextRole::ActionTitle: return 22;
	case EFMCodexPlayerUITextRole::Score: return 36;
	case EFMCodexPlayerUITextRole::Status: return 17;
	case EFMCodexPlayerUITextRole::DiceValue: return 38;
	case EFMCodexPlayerUITextRole::TerminalResult: return 28;
	case EFMCodexPlayerUITextRole::HandoffTitle: return 30;
	case EFMCodexPlayerUITextRole::HandoffPlayer: return 22;
	default: return 13;
	}
}

FLinearColor FFMCodexPlayerUIStyle::GetPlayerAccentColor(
	const FString& PresentationLabel) const
{
	const FString Label = PresentationLabel.ToUpper();
	if (Label.Contains(TEXT("PLAYER A")))
	{
		return GetColor(EFMCodexPlayerUIColorRole::PlayerAAccent);
	}
	if (Label.Contains(TEXT("PLAYER B")))
	{
		return GetColor(EFMCodexPlayerUIColorRole::PlayerBAccent);
	}
	return GetColor(EFMCodexPlayerUIColorRole::NeutralAccent);
}

FLinearColor FFMCodexPlayerUIStyle::GetStatusBadgeColor(
	const FString& PresentationLabel) const
{
	const FString Label = PresentationLabel.ToUpper();
	if (Label.Contains(TEXT("ACTIVE")))
	{
		return GetColor(EFMCodexPlayerUIColorRole::StatusActive);
	}
	if (Label.Contains(TEXT("USED")))
	{
		return GetColor(EFMCodexPlayerUIColorRole::StatusUsed);
	}
	if (Label.Contains(TEXT("AVAILABLE"))
		|| Label.Contains(TEXT("DEPLOYED")))
	{
		return GetColor(EFMCodexPlayerUIColorRole::StatusAvailable);
	}
	return GetColor(EFMCodexPlayerUIColorRole::NeutralAccent);
}

FLinearColor FFMCodexPlayerUIStyle::GetTerminalColor(
	const FString& PresentationLabel) const
{
	const FString Label = PresentationLabel.ToUpper();
	if (Label.Contains(TEXT("NO GOAL")) || Label.Contains(TEXT("MISS"))
		|| Label.Contains(TEXT("OFFSIDE"))
		|| Label.Contains(TEXT("OUT OF PLAY"))
		|| Label.Contains(TEXT("STOPPED")))
	{
		return GetColor(EFMCodexPlayerUIColorRole::Danger);
	}
	if (Label.Contains(TEXT("GOAL")))
	{
		return GetColor(EFMCodexPlayerUIColorRole::Success);
	}
	return GetColor(EFMCodexPlayerUIColorRole::TerminalNeutral);
}

FButtonStyle FFMCodexPlayerUIStyle::MakeButtonStyle(
	const EFMCodexPlayerUIActionRole Role) const
{
	using namespace FMCodexPlayerUIStyle;
	EFMCodexPlayerUIColorRole ColorRole =
		EFMCodexPlayerUIColorRole::ActionSecondary;
	switch (Role)
	{
	case EFMCodexPlayerUIActionRole::Primary:
		ColorRole = EFMCodexPlayerUIColorRole::ActionPrimary;
		break;
	case EFMCodexPlayerUIActionRole::Decline:
		ColorRole = EFMCodexPlayerUIColorRole::ActionDecline;
		break;
	case EFMCodexPlayerUIActionRole::Disabled:
		ColorRole = EFMCodexPlayerUIColorRole::ActionDisabled;
		break;
	default:
		break;
	}
	const FLinearColor Base = GetColor(ColorRole);
	FButtonStyle Result;
	Result.SetNormal(MakeSolidBrush(Base));
	Result.SetHovered(MakeSolidBrush(Emphasize(Base, 1.18f)));
	Result.SetPressed(MakeSolidBrush(Emphasize(Base, 0.72f)));
	Result.SetDisabled(MakeSolidBrush(
		GetColor(EFMCodexPlayerUIColorRole::ActionDisabled)));
	Result.SetNormalPadding(FMargin(12.0f, 8.0f));
	Result.SetPressedPadding(FMargin(12.0f, 9.0f, 12.0f, 7.0f));
	return Result;
}

void FFMCodexPlayerUIStyle::ApplyText(
	UTextBlock& Text,
	const EFMCodexPlayerUITextRole Role) const
{
	FSlateFontInfo Font = Text.GetFont();
	Font.Size = GetFontSize(Role);
	Text.SetFont(Font);
	Text.SetColorAndOpacity(Role == EFMCodexPlayerUITextRole::Secondary
		? FSlateColor(FLinearColor(0.62f, 0.68f, 0.73f, 1.0f))
		: FSlateColor(FLinearColor(0.93f, 0.96f, 0.98f, 1.0f)));
}

void FFMCodexPlayerUIStyle::ApplyBorder(
	UBorder& Border,
	const EFMCodexPlayerUIColorRole Role,
	const FMargin& Padding) const
{
	Border.SetPadding(Padding);
	Border.SetBrushColor(GetColor(Role));
}

void FFMCodexPlayerUIStyle::ApplyButton(
	UButton& Button,
	const EFMCodexPlayerUIActionRole Role) const
{
	Button.SetStyle(MakeButtonStyle(Role));
}

bool FFMCodexPlayerUIStyle::HasValidDefaults() const
{
	return GetPanelMinWidth() > 0.0f
		&& GetPanelMaxWidth() >= GetPanelMinWidth()
		&& GetFontSize(EFMCodexPlayerUITextRole::Score)
			> GetFontSize(EFMCodexPlayerUITextRole::Identity)
		&& GetFontSize(EFMCodexPlayerUITextRole::TerminalResult)
			> GetFontSize(EFMCodexPlayerUITextRole::Body)
		&& GetColor(EFMCodexPlayerUIColorRole::PlayerAAccent)
			!= GetColor(EFMCodexPlayerUIColorRole::PlayerBAccent)
		&& GetColor(EFMCodexPlayerUIColorRole::ActionPrimary).A > 0.0f;
}
