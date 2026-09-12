#include "FMCodexPlayerCardWidget.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "FMCodexPlayerUIAssetReferences.h"
#include "FMCodexPrototypeTeamContent.h"
#include "FMCodexLocalMatchInteractionView.h"
#include "FMCodexLocalMatchResolutionFeedback.h"
#include "FMCodexLocalMatchDemoConfiguration.h"
#include "FMCodexLocalMatchHostGameMode.h"
#include "FMCodexLocalMatchPlayerController.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

#include "Blueprint/WidgetTree.h"
#include "Fonts/FontMeasure.h"
#include "Framework/Application/SlateApplication.h"
#include "Rendering/SlateRenderer.h"
#include "Components/Border.h"
#include "Components/HorizontalBox.h"
#include "Components/Image.h"
#include "Components/OverlaySlot.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Engine/Engine.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include <limits>

namespace FMCodexPitchMiniPresentationTests
{
	class FScopedWidgetWorld final
	{
	public:
		FScopedWidgetWorld()
		{
			World = UWorld::CreateWorld(EWorldType::Game, false);
			if (World != nullptr && GEngine != nullptr)
			{
				FWorldContext& Context =
					GEngine->CreateNewWorldContext(EWorldType::Game);
				Context.SetCurrentWorld(World);
			}
		}

		~FScopedWidgetWorld()
		{
			if (World != nullptr)
			{
				if (GEngine != nullptr)
				{
					GEngine->DestroyWorldContext(World);
				}
				World->DestroyWorld(false);
			}
		}

		UWorld* Get() const { return World; }

	private:
		UWorld* World = nullptr;
	};

	FFMCodexUMGSkillViewModel MakeSkill(
		const TCHAR* Id,
		const TCHAR* Label,
		const int32 Minimum,
		const int32 Maximum)
	{
		FFMCodexUMGSkillViewModel Result;
		Result.SkillId = FName(Id);
		Result.CanonicalLabel = Label;
		Result.MinTriggerActionPoint = Minimum;
		Result.MaxTriggerActionPoint = Maximum;
		return Result;
	}

	FFMCodexUMGCardViewModel MakeCard()
	{
		FFMCodexUMGCardViewModel Result;
		Result.CardId = TEXT("Prototype.Arsenal.BukayoSaka");
		Result.IdentityLabel = TEXT("萨卡");
		Result.RoleLabel = TEXT("FW / MF");
		Result.RarityLabel = TEXT("World Class");
		Result.Skills = {
			MakeSkill(TEXT("Skill.Cross"), TEXT("Cross"), 4, 7),
			MakeSkill(TEXT("Skill.LongShot"), TEXT("Long Shot"), 6, 8)
		};
		Result.EligibleTacticalSkills = Result.Skills;
		return Result;
	}

	UFMCodexPlayerCardWidget* MakeWidget(
		UWorld& World,
		const FFMCodexUMGCardViewModel& Card,
		const EFMCodexPlayerCardPresentationMode Mode)
	{
		UFMCodexPlayerCardWidget* Widget =
			CreateWidget<UFMCodexPlayerCardWidget>(
				&World, UFMCodexPlayerCardWidget::StaticClass());
		if (Widget != nullptr)
		{
			Widget->RefreshFromPresentation(Card, Mode);
			Widget->TakeWidget();
		}
		return Widget;
	}

	bool LoadSource(const TCHAR* RelativePath, FString& OutSource)
	{
		return FFileHelper::LoadFileToString(OutSource,
			*FPaths::Combine(FPaths::ProjectDir(), RelativePath));
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexPitchMiniGeometryIdentityPortraitTest,
	"FMCodex.LocalPlay.PitchMiniPresentation.01.GeometryIdentityPortraitRarity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexPitchMiniGeometryIdentityPortraitTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexPitchMiniPresentationTests;
	FScopedWidgetWorld World;
	if (!TestNotNull(TEXT("Pitch Mini test World exists"), World.Get()))
	{
		return false;
	}
	FFMCodexUMGCardViewModel Card = MakeCard();
	UFMCodexPlayerCardWidget* PitchMini = MakeWidget(
		*World.Get(), Card, EFMCodexPlayerCardPresentationMode::PitchMini);
	if (!TestNotNull(TEXT("Pitch Mini widget exists"), PitchMini))
	{
		return false;
	}

	const USizeBox* PortraitBounds = Cast<USizeBox>(
		PitchMini->GetWidgetFromName(TEXT("PitchMiniPortraitBounds")));
	const USizeBox* IdentityBounds = Cast<USizeBox>(
		PitchMini->GetWidgetFromName(TEXT("PitchMiniIdentityBounds")));
	TestTrue(TEXT("Pitch Mini fixed external and 130 x 134 interior geometry"),
		PitchMini->GetConfiguredDimensions() == FVector2D(136.0f, 140.0f)
			&& PortraitBounds != nullptr
			&& PortraitBounds->GetWidthOverride() == 130.0f
			&& PortraitBounds->GetHeightOverride() == 112.0f
			&& IdentityBounds != nullptr
			&& IdentityBounds->GetWidthOverride() == 130.0f
			&& IdentityBounds->GetHeightOverride() == 22.0f);
	TestTrue(TEXT("Pitch Mini dedicated Skill presentation is absent"),
		PitchMini->GetWidgetFromName(TEXT("PitchMiniSkillBandBounds")) == nullptr
			&& PitchMini->GetWidgetFromName(TEXT("PitchMiniSkillBand")) == nullptr
			&& PitchMini->GetWidgetFromName(TEXT("PitchMiniSkillRows")) == nullptr
			&& PitchMini->GetWidgetFromName(TEXT("PitchMiniSkillRowBounds0")) == nullptr
			&& PitchMini->GetWidgetFromName(TEXT("PitchMiniSkillName0")) == nullptr
			&& PitchMini->GetWidgetFromName(TEXT("PitchMiniSkillRange0")) == nullptr);

	const UHorizontalBox* IdentityRow = Cast<UHorizontalBox>(
		PitchMini->GetWidgetFromName(TEXT("PitchMiniIdentityRow")));
	const UTextBlock* Name = Cast<UTextBlock>(
		PitchMini->GetWidgetFromName(TEXT("PitchMiniPlayerName")));
	const UTextBlock* Separator = Cast<UTextBlock>(
		PitchMini->GetWidgetFromName(TEXT("PitchMiniIdentitySeparator")));
	const UTextBlock* Position = Cast<UTextBlock>(
		PitchMini->GetWidgetFromName(TEXT("PitchMiniPosition")));
	TestTrue(TEXT("Name, separator, and slash-preserving Position share one row"),
		IdentityRow != nullptr && Name != nullptr && Separator != nullptr
			&& Position != nullptr
			&& Name->GetParent() == IdentityRow
			&& Separator->GetParent() == IdentityRow
			&& Position->GetParent() == IdentityRow
			&& Separator->GetText().ToString() == TEXT("|")
			&& Position->GetText().ToString() == TEXT("A/M")
			&& Position->GetFont().Size == 10
			&& Name->GetFont().Size >= 12 && Name->GetFont().Size <= 15
			&& !Name->GetAutoWrapText());

	const UImage* Portrait = Cast<UImage>(
		PitchMini->GetWidgetFromName(TEXT("PitchMiniPortraitImage")));
	UTexture2D* Texture = PitchMini->GetResolvedPortraitTexture();
	const FBox2f UV = Portrait != nullptr
		? static_cast<FBox2f>(Portrait->GetBrush().GetUVRegion()) : FBox2f();
	float CroppedPixelAspect = 0.0f;
	float HorizontalSubjectScale = 0.0f;
	float VerticalSubjectScale = 0.0f;
	FBox2f ExpectedHeroCrop;
	if (Texture != nullptr && UV.bIsValid)
	{
		const FIntPoint ImportedSize = Texture->GetImportedSize();
		ExpectedHeroCrop =
			UFMCodexPlayerCardWidget::CalculatePitchMiniHeroCrop(ImportedSize);
		CroppedPixelAspect =
			((UV.Max.X - UV.Min.X) * ImportedSize.X)
			/ ((UV.Max.Y - UV.Min.Y) * ImportedSize.Y);
		const float SourceAspect = static_cast<float>(ImportedSize.X)
			/ static_cast<float>(ImportedSize.Y);
		const float TargetAspect = 130.0f / 112.0f;
		const float AspectFillWidth = SourceAspect > TargetAspect
			? TargetAspect / SourceAspect : 1.0f;
		const float AspectFillHeight = SourceAspect < TargetAspect
			? SourceAspect / TargetAspect : 1.0f;
		HorizontalSubjectScale = AspectFillWidth / (UV.Max.X - UV.Min.X);
		VerticalSubjectScale = AspectFillHeight / (UV.Max.Y - UV.Min.Y);
	}
	TestNotNull(TEXT("Pitch Mini portrait image exists"), Portrait);
	TestNotNull(TEXT("Pitch Mini shared portrait resolves"), Texture);
	TestTrue(TEXT("Pitch Mini portrait brush has an explicit fill UV"),
		UV.bIsValid);
	TestTrue(TEXT("Pitch Mini portrait bounds clip the fill crop"),
		PortraitBounds != nullptr
			&& PortraitBounds->GetClipping() == EWidgetClipping::ClipToBounds);
	TestTrue(TEXT("Pitch Mini portrait brush retains the resolved texture"),
		Portrait != nullptr && Portrait->GetBrush().GetResourceObject() == Texture);
	TestTrue(TEXT("Pitch Mini portrait crop has the 130 x 112 pixel aspect"),
		FMath::IsNearlyEqual(CroppedPixelAspect, 130.0f / 112.0f, 0.01f));
	TestTrue(TEXT("Pitch Mini hero crop is deterministic and matches the rendered UV"),
		ExpectedHeroCrop.bIsValid
			&& FMath::IsNearlyEqual(UV.Min.X, ExpectedHeroCrop.Min.X)
			&& FMath::IsNearlyEqual(UV.Min.Y, ExpectedHeroCrop.Min.Y)
			&& FMath::IsNearlyEqual(UV.Max.X, ExpectedHeroCrop.Max.X)
			&& FMath::IsNearlyEqual(UV.Max.Y, ExpectedHeroCrop.Max.Y));
	TestTrue(TEXT("Pitch Mini hero crop uses the retuned eight-percent subject scale"),
		FMath::IsNearlyEqual(HorizontalSubjectScale, 1.08f, 0.001f)
			&& FMath::IsNearlyEqual(VerticalSubjectScale, 1.08f, 0.001f)
			&& UV.Min.X > 0.0f && UV.Max.X < 1.0f
			&& UV.Min.Y >= 0.045f && UV.Min.Y <= 0.065f);
	const FBox2f LandscapeHeroCrop =
		UFMCodexPlayerCardWidget::CalculatePitchMiniHeroCrop(
			FIntPoint(1920, 1080));
	const float LandscapePixelAspect = LandscapeHeroCrop.bIsValid
		? ((LandscapeHeroCrop.Max.X - LandscapeHeroCrop.Min.X) * 1920.0f)
			/ ((LandscapeHeroCrop.Max.Y - LandscapeHeroCrop.Min.Y) * 1080.0f)
		: 0.0f;
	TestTrue(TEXT("Pitch Mini hero crop stays clamped and aspect-safe for landscape input"),
		LandscapeHeroCrop.bIsValid
			&& LandscapeHeroCrop.Min.X >= 0.0f
			&& LandscapeHeroCrop.Min.Y >= 0.0f
			&& LandscapeHeroCrop.Max.X <= 1.0f
			&& LandscapeHeroCrop.Max.Y <= 1.0f
			&& FMath::IsNearlyEqual(
				LandscapePixelAspect, 130.0f / 112.0f, 0.01f)
			&& !UFMCodexPlayerCardWidget::CalculatePitchMiniHeroCrop(
				FIntPoint::ZeroValue).bIsValid);
	TestTrue(TEXT("Pitch Mini portrait route excludes FullCardPortrait"),
		Texture != nullptr
			&& !Texture->GetPathName().Contains(TEXT("FullCardHeroBust"))
			&& !Texture->GetPathName().Contains(TEXT("ApprovedRuntime192")));

	const UBorder* Frame = Cast<UBorder>(
		PitchMini->GetWidgetFromName(TEXT("PlayerCardFrame")));
	const FLinearColor FirstFrameColor = Frame != nullptr
		? Frame->GetBrushColor() : FLinearColor::Transparent;
	Card.RarityLabel = TEXT("Common");
	PitchMini->RefreshFromPresentation(
		Card, EFMCodexPlayerCardPresentationMode::PitchMini);
	TestTrue(TEXT("Pitch Mini has no rarity chrome or rarity-driven frame tint"),
		Frame != nullptr && Frame->GetBrushColor().Equals(FirstFrameColor)
			&& PitchMini->GetWidgetFromName(TEXT("PitchMiniRarityAccent")) == nullptr
			&& PitchMini->GetWidgetFromName(
				TEXT("PitchMiniRarityAccentBounds")) == nullptr
			&& PitchMini->GetWidgetFromName(TEXT("PitchMiniRarityBadge")) == nullptr);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexPitchMiniSkillStateIsolationTest,
	"FMCodex.LocalPlay.PitchMiniPresentation.02.SkillStatesAndVariantIsolation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexPitchMiniSkillStateIsolationTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexPitchMiniPresentationTests;
	FScopedWidgetWorld World;
	if (!TestNotNull(TEXT("Pitch Mini test World exists"), World.Get()))
	{
		return false;
	}
	FFMCodexUMGCardViewModel Card = MakeCard();
	// Legacy art shares refined chrome and unchanged tactical pip semantics.
	Card.CardId = TEXT("Prototype.Arsenal.GabrielMartinelli");
	UFMCodexPlayerCardWidget* PitchMini = MakeWidget(
		*World.Get(), Card, EFMCodexPlayerCardPresentationMode::PitchMini);
	if (!TestNotNull(TEXT("Pitch Mini widget exists"), PitchMini))
	{
		return false;
	}
	const UBorder* StrokeTop = Cast<UBorder>(
		PitchMini->GetWidgetFromName(TEXT("PitchMiniTacticalMatchStrokeTop")));
	const UBorder* StrokeBottom = Cast<UBorder>(
		PitchMini->GetWidgetFromName(TEXT("PitchMiniTacticalMatchStrokeBottom")));
	const UBorder* StrokeLeft = Cast<UBorder>(
		PitchMini->GetWidgetFromName(TEXT("PitchMiniTacticalMatchStrokeLeft")));
	const UBorder* StrokeRight = Cast<UBorder>(
		PitchMini->GetWidgetFromName(TEXT("PitchMiniTacticalMatchStrokeRight")));
	const UBorder* GlowTop = Cast<UBorder>(
		PitchMini->GetWidgetFromName(TEXT("PitchMiniTacticalMatchGlowTop")));
	const USizeBox* StrokeTopBounds = Cast<USizeBox>(
		PitchMini->GetWidgetFromName(
			TEXT("PitchMiniTacticalMatchStrokeTopBounds")));
	const USizeBox* GlowTopBounds = Cast<USizeBox>(
		PitchMini->GetWidgetFromName(
			TEXT("PitchMiniTacticalMatchGlowTopBounds")));
	const UBorder* PipTop = Cast<UBorder>(
		PitchMini->GetWidgetFromName(
			TEXT("PitchMiniTacticalMatchPipTop")));
	const UBorder* PipBottom = Cast<UBorder>(
		PitchMini->GetWidgetFromName(
			TEXT("PitchMiniTacticalMatchPipBottom")));
	const USizeBox* PipTopBounds = Cast<USizeBox>(
		PitchMini->GetWidgetFromName(
			TEXT("PitchMiniTacticalMatchPipTopBounds")));
	const USizeBox* PipGroupBounds = Cast<USizeBox>(
		PitchMini->GetWidgetFromName(
			TEXT("PitchMiniTacticalMatchPipGroupBounds")));
	TestTrue(TEXT("Eligible Skill collections do not bypass the resolved OFF state"),
		Card.Skills.Num() == 2 && Card.EligibleTacticalSkills.Num() == 2
			&& Card.PitchMiniVisibleTacticalSkills.IsEmpty()
			&& Card.PitchMiniTacticalMatchCount == 0
			&& PitchMini->GetRenderedSkillCount() == 0
			&& StrokeTop != nullptr
			&& StrokeTop->GetVisibility() == ESlateVisibility::Collapsed
			&& PipTop != nullptr && PipBottom != nullptr
			&& PipTop->GetVisibility() == ESlateVisibility::Collapsed
			&& PipBottom->GetVisibility() == ESlateVisibility::Collapsed);

	Card.PitchMiniVisibleTacticalSkills = { Card.EligibleTacticalSkills[0] };
	Card.PitchMiniTacticalMatchCount = 1;
	Card.bHasPitchMiniTacticalMatch = true;
	PitchMini->RefreshFromPresentation(
		Card, EFMCodexPlayerCardPresentationMode::PitchMini);
	FLinearColor ExpectedAccent =
		FLinearColor::FromSRGBColor(FColor(0x8F, 0xE6, 0xC2));
	ExpectedAccent.A = 0.88f;
	TestTrue(TEXT("One eligible Skill keeps its upper-left pip without stacked tactical borders"),
		PitchMini->GetRenderedSkillCount() == 0
			&& StrokeTop != nullptr && StrokeBottom != nullptr
			&& StrokeLeft != nullptr && StrokeRight != nullptr
			&& StrokeTop->GetVisibility() == ESlateVisibility::Collapsed
			&& StrokeBottom->GetVisibility() == ESlateVisibility::Collapsed
			&& StrokeLeft->GetVisibility() == ESlateVisibility::Collapsed
			&& StrokeRight->GetVisibility() == ESlateVisibility::Collapsed
			&& StrokeTop->GetBrushColor().Equals(ExpectedAccent)
			&& StrokeTopBounds != nullptr
			&& StrokeTopBounds->GetHeightOverride() == 1.5f
			&& GlowTop != nullptr
			&& GlowTop->GetVisibility() == ESlateVisibility::Collapsed
			&& GlowTopBounds != nullptr
			&& GlowTopBounds->GetHeightOverride() == 3.0f
			&& PipTopBounds != nullptr
			&& PipTopBounds->GetWidthOverride() == 4.0f
			&& PipTopBounds->GetHeightOverride() == 4.0f
			&& PipGroupBounds != nullptr
			&& PipGroupBounds->GetWidthOverride() == 4.0f
			&& PipGroupBounds->GetHeightOverride() == 11.0f
			&& PipTop->GetVisibility() == ESlateVisibility::HitTestInvisible
			&& PipBottom->GetVisibility() == ESlateVisibility::Collapsed);

	Card.PitchMiniVisibleTacticalSkills = Card.EligibleTacticalSkills;
	Card.PitchMiniTacticalMatchCount = 2;
	PitchMini->RefreshFromPresentation(
		Card, EFMCodexPlayerCardPresentationMode::PitchMini);
	TestTrue(TEXT("Two eligible Skills render exactly two vertical pips without text"),
		PitchMini->GetRenderedSkillCount() == 0
			&& StrokeTop->GetVisibility() == ESlateVisibility::Collapsed
			&& PipTop->GetVisibility() == ESlateVisibility::HitTestInvisible
			&& PipBottom->GetVisibility() == ESlateVisibility::HitTestInvisible
			&& PitchMini->GetWidgetFromName(TEXT("PitchMiniSkillName0")) == nullptr
			&& PitchMini->GetWidgetFromName(TEXT("PitchMiniSkillRange0")) == nullptr);

	Card.PitchMiniVisibleTacticalSkills.Reset();
	UFMCodexPlayerCardWidget* FullCard = MakeWidget(
		*World.Get(), Card,
		EFMCodexPlayerCardPresentationMode::InteractionChoice);
	UFMCodexPlayerCardWidget* HandMicro = MakeWidget(
		*World.Get(), Card, EFMCodexPlayerCardPresentationMode::HandMicro);
	const UBorder* FullCardTacticalStroke = FullCard != nullptr
		? Cast<UBorder>(FullCard->GetWidgetFromName(
			TEXT("PitchMiniTacticalMatchStrokeTop"))) : nullptr;
	const UBorder* HandMicroTacticalStroke = HandMicro != nullptr
		? Cast<UBorder>(HandMicro->GetWidgetFromName(
			TEXT("PitchMiniTacticalMatchStrokeTop"))) : nullptr;
	USizeBox* HandMicroPipGroup = HandMicro != nullptr
		? Cast<USizeBox>(HandMicro->GetWidgetFromName(
			TEXT("PitchMiniTacticalMatchPipGroupBounds"))) : nullptr;
	UBorder* HandMicroPipTop = HandMicro != nullptr
		? Cast<UBorder>(HandMicro->GetWidgetFromName(
			TEXT("PitchMiniTacticalMatchPipTop"))) : nullptr;
	UBorder* HandMicroPipBottom = HandMicro != nullptr
		? Cast<UBorder>(HandMicro->GetWidgetFromName(
			TEXT("PitchMiniTacticalMatchPipBottom"))) : nullptr;
	const UOverlaySlot* HandMicroPipGroupSlot = HandMicroPipGroup != nullptr
		? Cast<UOverlaySlot>(HandMicroPipGroup->Slot) : nullptr;
	TestTrue(TEXT("Full Card keeps all static Skills while Hand Micro starts pip-free"),
		FullCard != nullptr && FullCard->GetRenderedSkillCount() == 2
			&& FullCard->GetPresentation().Skills.Num() == 2
			&& FullCardTacticalStroke != nullptr
			&& FullCardTacticalStroke->GetVisibility()
				== ESlateVisibility::Collapsed
			&& HandMicro != nullptr && HandMicro->GetRenderedSkillCount() == 0
			&& HandMicro->GetConfiguredDimensions() == FVector2D(220.0f, 68.0f)
			&& HandMicroTacticalStroke != nullptr
			&& HandMicroTacticalStroke->GetVisibility()
				== ESlateVisibility::Collapsed
			&& HandMicroPipGroup != nullptr
			&& HandMicroPipGroup->GetVisibility()
				== ESlateVisibility::Collapsed
			&& HandMicroPipTop != nullptr && HandMicroPipBottom != nullptr
			&& HandMicroPipTop->GetVisibility() == ESlateVisibility::Collapsed
			&& HandMicroPipBottom->GetVisibility() == ESlateVisibility::Collapsed);

	Card.HandMicroVisibleTacticalSkills = { Card.EligibleTacticalSkills[0] };
	Card.HandMicroTacticalMatchCount = 1;
	Card.bHasHandMicroTacticalMatch = true;
	HandMicro->RefreshFromPresentation(
		Card, EFMCodexPlayerCardPresentationMode::HandMicro);
	TestTrue(TEXT("Hand Micro one-match state uses one non-interactive shared-style pip"),
		HandMicroPipGroup->GetVisibility()
			== ESlateVisibility::HitTestInvisible
			&& HandMicroPipGroup->GetWidthOverride() == 4.0f
			&& HandMicroPipGroup->GetHeightOverride() == 11.0f
			&& HandMicroPipGroupSlot != nullptr
			&& HandMicroPipGroupSlot->GetPadding()
				== FMargin(9.0f, 8.0f, 0.0f, 0.0f)
			&& HandMicroPipTop->GetVisibility()
				== ESlateVisibility::HitTestInvisible
			&& HandMicroPipTop->GetBrushColor().Equals(PipTop->GetBrushColor())
			&& HandMicroPipBottom->GetVisibility()
				== ESlateVisibility::Collapsed
			&& HandMicroTacticalStroke->GetVisibility()
				== ESlateVisibility::Collapsed
			&& HandMicro->GetConfiguredDimensions() == FVector2D(220.0f, 68.0f));

	Card.HandMicroVisibleTacticalSkills = Card.EligibleTacticalSkills;
	Card.HandMicroTacticalMatchCount = 2;
	HandMicro->RefreshFromPresentation(
		Card, EFMCodexPlayerCardPresentationMode::HandMicro);
	TestTrue(TEXT("Hand Micro two-match state reveals exactly the second shared-style pip"),
		HandMicroPipTop->GetVisibility() == ESlateVisibility::HitTestInvisible
			&& HandMicroPipBottom->GetVisibility()
				== ESlateVisibility::HitTestInvisible
			&& HandMicroTacticalStroke->GetVisibility()
				== ESlateVisibility::Collapsed);

	Card.HandMicroVisibleTacticalSkills.Reset();
	Card.HandMicroTacticalMatchCount = 0;
	Card.bHasHandMicroTacticalMatch = false;
	HandMicro->RefreshFromPresentation(
		Card, EFMCodexPlayerCardPresentationMode::HandMicro);
	TestTrue(TEXT("Hand Micro zero-match state hides the entire pip group"),
		HandMicroPipGroup->GetVisibility() == ESlateVisibility::Collapsed
			&& HandMicroPipTop->GetVisibility() == ESlateVisibility::Collapsed
			&& HandMicroPipBottom->GetVisibility() == ESlateVisibility::Collapsed);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexPitchMiniLayerAndContentContractTest,
	"FMCodex.LocalPlay.PitchMiniPresentation.03.LayerGeometryAndContent",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexPitchMiniLayerAndContentContractTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexPitchMiniPresentationTests;
	FString CardSource;
	FString InteractionSource;
	FString PresentationSource;
	FString SlotSource;
	FString ScreenSource;
	TestTrue(TEXT("Pitch Mini production sources are readable"),
		LoadSource(TEXT("Source/FMCodex/LocalPlay/FMCodexPlayerCardWidget.cpp"),
			CardSource)
			&& LoadSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchInteractionView.cpp"),
				InteractionSource)
			&& LoadSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchUMGPresentation.cpp"),
				PresentationSource)
			&& LoadSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexPitchSlotWidget.cpp"),
				SlotSource)
			&& LoadSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchScreenWidget.cpp"),
				ScreenSource));
	TestTrue(TEXT("Pitch Mini consumes only the resolved tactical-match count"),
		CardSource.Contains(TEXT("Presentation.PitchMiniTacticalMatchCount"))
			&& CardSource.Contains(TEXT("Presentation.bHasPitchMiniTacticalMatch"))
			&& !CardSource.Contains(
				TEXT("Presentation.PitchMiniVisibleTacticalSkills"))
			&& !CardSource.Contains(TEXT("Presentation.EligibleTacticalSkills"))
			&& !CardSource.Contains(TEXT("CurrentTacticalPoint"))
			&& !CardSource.Contains(TEXT("CurrentAttackingPlayer"))
			&& !CardSource.Contains(TEXT("ProjectEligibleTacticalSkills"))
			&& InteractionSource.Contains(
				TEXT("View.PitchMiniTacticalMatchCount"))
			&& PresentationSource.Contains(
				TEXT("Result.PitchMiniTacticalMatchCount")));
	TestTrue(TEXT("Pitch Mini Skill band and rows are not constructed"),
		!CardSource.Contains(TEXT("PitchMiniSkillBand"))
			&& !CardSource.Contains(TEXT("PitchMiniSkillRows"))
			&& !CardSource.Contains(TEXT("PitchMiniSkillName"))
			&& !CardSource.Contains(TEXT("PitchMiniSkillRange")));
	TestTrue(TEXT("Pitch Mini consumes presentation-resolved ownership without side palette logic"),
		CardSource.Contains(TEXT("Presentation.PitchMiniOwnershipAccentColor"))
			&& CardSource.Contains(TEXT("PitchMiniOwnershipAccentEdge"))
			&& !CardSource.Contains(TEXT("PlayerAPrimaryColor"))
			&& !CardSource.Contains(TEXT("PlayerBPrimaryColor"))
			&& !CardSource.Contains(TEXT("0xA4, 0x47, 0x4F"))
			&& !CardSource.Contains(TEXT("0x4F, 0x78, 0x92"))
			&& PresentationSource.Contains(
				TEXT("ResolvePitchMiniOwnershipAccent")));
	TestTrue(TEXT("Pitch Mini rarity and old stretched portrait paths are absent"),
		!CardSource.Contains(TEXT("PitchMiniRarityAccent"))
			&& !CardSource.Contains(
				TEXT("FLinearColor::LerpUsingHSV(BaseFrame, RarityAccent, 0.10f)"))
			&& !CardSource.Contains(
				TEXT("PitchMiniPortraitImage->SetBrushFromTexture(ResolvedPortraitTexture, true)")));
	TestTrue(TEXT("Pitch Mini uses one deterministic generalized hero crop"),
		CardSource.Contains(TEXT("CalculatePitchMiniHeroCrop"))
			&& CardSource.Contains(TEXT("PitchMiniHeroZoom = 1.08f"))
			&& CardSource.Contains(TEXT("PitchMiniHeroFocalY = 0.278f"))
			&& CardSource.Contains(TEXT("PitchMiniHeroFocalFrameY = 0.42f"))
			&& !CardSource.Contains(TEXT("PitchMiniPortraitCrops")));
	TestTrue(TEXT("Tactical match retains its static cyan-mint pip geometry"),
		CardSource.Contains(TEXT("0x8F, 0xE6, 0xC2"))
			&& CardSource.Contains(
				TEXT("PitchMiniTacticalMatchStrokeThickness = 1.5f"))
			&& CardSource.Contains(
				TEXT("PitchMiniTacticalMatchGlowThickness = 3.0f"))
			&& CardSource.Contains(
				TEXT("PitchMiniTacticalMatchPipDiameter = 4.0f"))
			&& CardSource.Contains(
				TEXT("PitchMiniTacticalMatchPipGap = 3.0f"))
			&& CardSource.Contains(
				TEXT("PitchMiniTacticalMatchPipLeftInset = 9.0f"))
			&& CardSource.Contains(
				TEXT("PitchMiniTacticalMatchPipTopInset = 8.0f"))
			&& CardSource.Contains(TEXT("FSlateRoundedBoxBrush"))
			&& !CardSource.Contains(TEXT("PlayAnimation")));
	TestTrue(TEXT("Drag Proxy remains the unchanged Hand Micro presentation"),
		CardSource.Contains(
			TEXT("Presentation, EFMCodexPlayerCardPresentationMode::HandMicro"))
			&& CardSource.Contains(TEXT("Operation->DefaultDragVisual = DragVisual")));
	TestTrue(TEXT("Slot and pitch external geometry remain unchanged"),
		SlotSource.Contains(TEXT("SetWidthOverride(148.0f)"))
			&& SlotSource.Contains(TEXT("SetHeightOverride(148.0f)"))
			&& ScreenSource.Contains(TEXT("SetHeightOverride(828.0f)")));

	TArray<FString> ValidationErrors;
	TestTrue(TEXT("Canonical content validates without rebalance"),
		FFMCodexPrototypeTeamContent::Validate(ValidationErrors));
	TestEqual(TEXT("Canonical content validation reports no errors"),
		ValidationErrors.Num(), 0);
	TestEqual(TEXT("Canonical roster remains forty players"),
		FFMCodexPrototypeTeamContent::GetDefinitions().Num(), 40);
	TestEqual(TEXT("Canonical roster remains twenty players per team"),
		FFMCodexPrototypeTeamContent::CardsPerTeam(), 20);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexPitchMiniOwnershipAccentAndFallbackTest,
	"FMCodex.LocalPlay.PitchMiniPresentation.04.OwnershipAccentAndFallback",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexPitchMiniOwnershipAccentAndFallbackTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexPitchMiniPresentationTests;

	FFMCodexLocalMatchInteractionView View;
	View.bMatchActive = true;
	View.CurrentAttackingPlayer = EInitialTurnOrderPlayer::PlayerA;
	View.ExpectedActingPlayer = EInitialTurnOrderPlayer::PlayerA;
	View.bCurrentAttackActive = true;
	View.InteractionCategory =
		EFMCodexLocalMatchInteractionCategory::SelectSkill;

	auto MakePitchRegion = [](const EMatchPlayNeutralSlotSide NeutralSide,
		const EInitialTurnOrderPlayer CardSide,
		const FName CardId,
		const TCHAR* DisplayLabel)
	{
		FFMCodexLocalMatchPitchRegionView Region;
		Region.NeutralSide = NeutralSide;
		Region.Label = TEXT("Midfield");
		Region.PlayerARelativeZone =
			EMatchPlayRelativeDeploymentZone::Midfield;
		Region.PlayerBRelativeZone =
			EMatchPlayRelativeDeploymentZone::Midfield;
		FFMCodexLocalMatchPitchSlotView& Slot =
			Region.Slots.AddDefaulted_GetRef();
		Slot.SlotId = CardId;
		Slot.NeutralSide = NeutralSide;
		Slot.PlayerARelativeZone = Region.PlayerARelativeZone;
		Slot.PlayerBRelativeZone = Region.PlayerBRelativeZone;
		Slot.bOccupied = true;
		Slot.Card.Side = CardSide;
		Slot.Card.CardId = CardId;
		Slot.Card.DisplayLabel = DisplayLabel;
		Slot.Card.CompactRoleLabel = TEXT("MF");
		Slot.Card.bDeployed = true;
		FFMCodexLocalMatchCardView::FSkill& Skill =
			Slot.Card.Skills.AddDefaulted_GetRef();
		Skill.SkillId = TEXT("Skill.Cross");
		Skill.CanonicalLabel = TEXT("Cross");
		Skill.MinTriggerActionPoint = 4;
		Skill.MaxTriggerActionPoint = 7;
		Slot.Card.EligibleTacticalSkills = Slot.Card.Skills;
		return Region;
	};

	View.PitchRegions.Add(MakePitchRegion(
		EMatchPlayNeutralSlotSide::NearPlayerA,
		EInitialTurnOrderPlayer::PlayerA,
		TEXT("Prototype.Arsenal.BukayoSaka"), TEXT("萨卡")));
	View.PitchRegions.Add(MakePitchRegion(
		EMatchPlayNeutralSlotSide::NearPlayerB,
		EInitialTurnOrderPlayer::PlayerB,
		TEXT("Prototype.Fallback.Player"), TEXT("后备球员")));
	View.PitchRegions[0].Slots[0].Card.PitchMiniVisibleTacticalSkills =
		View.PitchRegions[0].Slots[0].Card.EligibleTacticalSkills;
	View.PitchRegions[0].Slots[0].Card.PitchMiniTacticalMatchCount = 1;
	View.PitchRegions[0].Slots[0].Card.bHasPitchMiniTacticalMatch = true;

	FFMCodexUMGSidePrimaryColors Palette;
	Palette.PlayerAPrimaryColor = FLinearColor(0.74f, 0.10f, 0.15f, 1.0f);
	Palette.PlayerBPrimaryColor = FLinearColor(0.12f, 0.42f, 0.74f, 1.0f);
	const FFMCodexLocalMatchResolutionFeedback Feedback;
	const FFMCodexUMGMatchScreenViewModel AttackerA =
		FFMCodexLocalMatchUMGPresentationBuilder::Build(View, Feedback,
			FString(), EInitialTurnOrderPlayer::PlayerA,
			Palette);
	if (!TestTrue(TEXT("Both occupied pitch cards are presented"),
		AttackerA.PitchRegions.Num() == 2
			&& AttackerA.PitchRegions[0].Slots.Num() == 1
			&& AttackerA.PitchRegions[1].Slots.Num() == 1))
	{
		return false;
	}

	const FFMCodexUMGCardViewModel& SelfCard =
		AttackerA.PitchRegions[0].Slots[0].Card;
	const FFMCodexUMGCardViewModel& OpponentCard =
		AttackerA.PitchRegions[1].Slots[0].Card;
	TestTrue(TEXT("Self gets deterministic Player A color on the left edge"),
		SelfCard.bHasPitchMiniOwnershipAccent
			&& SelfCard.PitchMiniOwnershipAccentEdge
				== EFMCodexUMGPitchMiniOwnershipEdge::Left
			&& SelfCard.PitchMiniOwnershipAccentColor.Equals(
				Palette.PlayerAPrimaryColor));
	TestTrue(TEXT("Opponent gets deterministic Player B color on the right edge"),
		OpponentCard.bHasPitchMiniOwnershipAccent
			&& OpponentCard.PitchMiniOwnershipAccentEdge
				== EFMCodexUMGPitchMiniOwnershipEdge::Right
			&& OpponentCard.PitchMiniOwnershipAccentColor.Equals(
				Palette.PlayerBPrimaryColor));
	TestTrue(TEXT("Defending card keeps ownership accent with tactical match OFF"),
		OpponentCard.PitchMiniVisibleTacticalSkills.IsEmpty()
			&& OpponentCard.PitchMiniTacticalMatchCount == 0
			&& !OpponentCard.bHasPitchMiniTacticalMatch
			&& OpponentCard.bHasPitchMiniOwnershipAccent);

	View.CurrentAttackingPlayer = EInitialTurnOrderPlayer::PlayerB;
	View.ExpectedActingPlayer = EInitialTurnOrderPlayer::PlayerB;
	View.PitchRegions[0].Slots[0].Card.PitchMiniVisibleTacticalSkills.Reset();
	View.PitchRegions[0].Slots[0].Card.PitchMiniTacticalMatchCount = 0;
	View.PitchRegions[0].Slots[0].Card.bHasPitchMiniTacticalMatch = false;
	View.PitchRegions[1].Slots[0].Card.PitchMiniVisibleTacticalSkills =
		View.PitchRegions[1].Slots[0].Card.EligibleTacticalSkills;
	View.PitchRegions[1].Slots[0].Card.PitchMiniTacticalMatchCount = 1;
	View.PitchRegions[1].Slots[0].Card.bHasPitchMiniTacticalMatch = true;
	const FFMCodexUMGMatchScreenViewModel AttackerB =
		FFMCodexLocalMatchUMGPresentationBuilder::Build(View, Feedback,
			FString(), EInitialTurnOrderPlayer::PlayerA,
			Palette);
	const FFMCodexUMGCardViewModel& SelfAfterTransition =
		AttackerB.PitchRegions[0].Slots[0].Card;
	const FFMCodexUMGCardViewModel& OpponentAfterTransition =
		AttackerB.PitchRegions[1].Slots[0].Card;
	TestTrue(TEXT("Attack transition moves tactical match without moving ownership accents"),
		SelfAfterTransition.PitchMiniVisibleTacticalSkills.IsEmpty()
			&& OpponentAfterTransition.PitchMiniVisibleTacticalSkills.Num() == 1
			&& SelfAfterTransition.PitchMiniTacticalMatchCount == 0
			&& OpponentAfterTransition.PitchMiniTacticalMatchCount == 1
			&& !SelfAfterTransition.bHasPitchMiniTacticalMatch
			&& OpponentAfterTransition.bHasPitchMiniTacticalMatch
			&& SelfAfterTransition.PitchMiniOwnershipAccentEdge
				== SelfCard.PitchMiniOwnershipAccentEdge
			&& OpponentAfterTransition.PitchMiniOwnershipAccentEdge
				== OpponentCard.PitchMiniOwnershipAccentEdge
			&& SelfAfterTransition.PitchMiniOwnershipAccentColor.Equals(
				SelfCard.PitchMiniOwnershipAccentColor)
			&& OpponentAfterTransition.PitchMiniOwnershipAccentColor.Equals(
				OpponentCard.PitchMiniOwnershipAccentColor));

	const FFMCodexUMGMatchScreenViewModel PlayerBViewer =
		FFMCodexLocalMatchUMGPresentationBuilder::Build(View, Feedback,
			FString(), EInitialTurnOrderPlayer::PlayerB,
			Palette);
	TestTrue(TEXT("Local-view relation flips rail edges but preserves side colors"),
		PlayerBViewer.PitchRegions[0].Slots[0].Card
			.PitchMiniOwnershipAccentEdge
				== EFMCodexUMGPitchMiniOwnershipEdge::Left
			&& PlayerBViewer.PitchRegions[0].Slots[0].Card
				.PitchMiniOwnershipAccentColor.Equals(Palette.PlayerBPrimaryColor)
			&& PlayerBViewer.PitchRegions[1].Slots[0].Card
				.PitchMiniOwnershipAccentEdge
				== EFMCodexUMGPitchMiniOwnershipEdge::Right
			&& PlayerBViewer.PitchRegions[1].Slots[0].Card
				.PitchMiniOwnershipAccentColor.Equals(Palette.PlayerAPrimaryColor));

	FScopedWidgetWorld World;
	if (!TestNotNull(TEXT("Pitch Mini ownership test World exists"), World.Get()))
	{
		return false;
	}
	UFMCodexPlayerCardWidget* SelfWidget = MakeWidget(*World.Get(), SelfCard,
		EFMCodexPlayerCardPresentationMode::PitchMini);
	UFMCodexPlayerCardWidget* FallbackWidget = MakeWidget(
		*World.Get(), OpponentAfterTransition,
		EFMCodexPlayerCardPresentationMode::PitchMini);
	if (!TestNotNull(TEXT("Ownership widgets exist"), SelfWidget)
		|| !TestNotNull(TEXT("Fallback widget exists"), FallbackWidget))
	{
		return false;
	}
	const UBorder* SelfLeftRail = Cast<UBorder>(
		SelfWidget->GetWidgetFromName(TEXT("PitchMiniOwnershipRailLeft")));
	const UBorder* SelfRightRail = Cast<UBorder>(
		SelfWidget->GetWidgetFromName(TEXT("PitchMiniOwnershipRailRight")));
	const USizeBox* SelfLeftRailBounds = Cast<USizeBox>(
		SelfWidget->GetWidgetFromName(
			TEXT("PitchMiniOwnershipRailLeftBounds")));
	TestTrue(TEXT("Resolved self accent uses shared chrome without an extra side rail"),
		SelfLeftRail != nullptr && SelfRightRail != nullptr
			&& SelfLeftRailBounds != nullptr
			&& SelfLeftRailBounds->GetWidthOverride() == 3.0f
			&& SelfLeftRail->GetVisibility()
				== ESlateVisibility::Collapsed
			&& SelfRightRail->GetVisibility() == ESlateVisibility::Collapsed
			&& UFMCodexPlayerCardWidget::ResolvePitchMiniOwnerColor(SelfCard).Equals(
				Palette.PlayerAPrimaryColor));

	const UBorder* FallbackRightRail = Cast<UBorder>(
		FallbackWidget->GetWidgetFromName(
			TEXT("PitchMiniOwnershipRailRight")));
	const UBorder* FallbackSurface = Cast<UBorder>(
		FallbackWidget->GetWidgetFromName(TEXT("PitchMiniPortraitFallback")));
	const UBorder* TonalWash = Cast<UBorder>(
		FallbackWidget->GetWidgetFromName(TEXT("PitchMiniPortraitTonalWash")));
	const UBorder* FallbackTacticalStroke = Cast<UBorder>(
		FallbackWidget->GetWidgetFromName(
			TEXT("PitchMiniTacticalMatchStrokeTop")));
	TestTrue(TEXT("Fallback portrait supports enlarged restrained ownership and tactical state"),
		FallbackWidget->GetResolvedPortraitTexture() == nullptr
			&& FallbackWidget->GetRenderedSkillCount() == 0
			&& FallbackRightRail != nullptr
			&& FallbackRightRail->GetVisibility()
				== ESlateVisibility::Collapsed
			&& FallbackSurface != nullptr
			&& FallbackSurface->GetVisibility()
				== ESlateVisibility::HitTestInvisible
			&& FallbackSurface->GetBrushColor().GetLuminance() < 0.20f
			&& TonalWash != nullptr
			&& FMath::IsNearlyEqual(
				TonalWash->GetBrushColor().A, 0.0f)
			&& FallbackTacticalStroke != nullptr
			&& FallbackTacticalStroke->GetVisibility()
				== ESlateVisibility::Collapsed);

	UFMCodexPlayerCardWidget* FullCard = MakeWidget(*World.Get(), SelfCard,
		EFMCodexPlayerCardPresentationMode::InteractionChoice);
	const UBorder* FullCardLeftRail = FullCard != nullptr
		? Cast<UBorder>(FullCard->GetWidgetFromName(
			TEXT("PitchMiniOwnershipRailLeft"))) : nullptr;
	TestTrue(TEXT("Pitch Mini ownership treatment is isolated from Full Card"),
		FullCard != nullptr && FullCardLeftRail != nullptr
			&& FullCardLeftRail->GetVisibility() == ESlateVisibility::Collapsed
			&& FullCard->GetPresentation().Skills.Num() == 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FFMCodexPitchMiniCanonicalPilotTest,
    "FMCodex.LocalPlay.PitchMiniPresentation.05.CanonicalPilotColorAndPurposeIsolation",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexPitchMiniCanonicalPilotTest::RunTest(const FString& Parameters)
{
    using namespace FMCodexPitchMiniPresentationTests;
    FScopedWidgetWorld World;
    if (!TestNotNull(TEXT("Pilot world"), World.Get())) return false;
    auto Card = MakeCard();
    Card.bHasPitchMiniOwnershipAccent = true;
    Card.PitchMiniOwnershipAccentColor = FLinearColor(.12f,.65f,.28f,1.f);
    Card.PitchMiniOwnershipAccentEdge = EFMCodexUMGPitchMiniOwnershipEdge::None;
    auto* Widget = MakeWidget(*World.Get(),Card,EFMCodexPlayerCardPresentationMode::PitchMini);
    if (!TestNotNull(TEXT("Pilot widget"),Widget)) return false;
    for (const TCHAR* Key : {TEXT("Prototype.Arsenal.BukayoSaka"),TEXT("Prototype.Arsenal.DavidRaya"),
        TEXT("Prototype.ManchesterCity.Rodri"),TEXT("Prototype.ManchesterCity.ErlingHaaland")})
    {
        Card.CardId = Key;
        Widget->RefreshFromPresentation(Card,EFMCodexPlayerCardPresentationMode::PitchMini);
        const auto Art=FFMCodexPlayerUIAssetReferences::Get().ResolveCardArt(Card.CardId);
        auto* Pitch=Widget->GetResolvedPortraitTexture();
        if (!TestNotNull(TEXT("Shared derivative"),Pitch)) continue;
        TestTrue(TEXT("Only four-player Pitch pilot activates"),Widget->IsPitchMiniPilot());
        TestEqual(TEXT("Pitch uses Shared route"),Pitch->GetPathName(),Art.PitchMiniPortrait.ToSoftObjectPath().ToString());
        TestEqual(TEXT("Shared role resolution"),Pitch->GetImportedSize(),FIntPoint(512,768));
        TestEqual(TEXT("Unchanged hit geometry"),Widget->GetConfiguredDimensions(),FVector2D(136,140));
        TestNull(TEXT("Pitch does not acquire Hand"),Widget->GetResolvedHandMicroPortraitTexture());
        TestNull(TEXT("Pitch does not acquire frame"),Widget->GetResolvedCardFrameTexture());
        TestNull(TEXT("Pitch does not acquire role icon"),Widget->GetResolvedRoleIconTexture());
        TestNull(TEXT("Pitch does not acquire skill icon"),Widget->GetResolvedLongShotSkillIconTexture());
        for (const TCHAR* Name : {TEXT("PortraitAssetImage"),TEXT("HandMicroFaceSafePortrait")})
            TestNull(TEXT("Inactive image releases resource"),CastChecked<UImage>(Widget->GetWidgetFromName(Name))->GetBrush().GetResourceObject());
        for (int32 Count=0;Count<=2;++Count)
        {
            Card.PitchMiniTacticalMatchCount=Count;Card.bHasPitchMiniTacticalMatch=Count>0;
            Widget->RefreshFromPresentation(Card,EFMCodexPlayerCardPresentationMode::PitchMini);
            TestEqual(TEXT("Data-driven top pip"),Widget->GetWidgetFromName(TEXT("PitchMiniTacticalMatchPipTop"))->GetVisibility(),
                Count>0 ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
            TestEqual(TEXT("Data-driven second pip"),Widget->GetWidgetFromName(TEXT("PitchMiniTacticalMatchPipBottom"))->GetVisibility(),
                Count==2 ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
            TestEqual(TEXT("No fragmented top line"),Widget->GetWidgetFromName(TEXT("PitchMiniTacticalMatchStrokeTop"))->GetVisibility(),ESlateVisibility::Collapsed);
        }
        TestEqual(TEXT("Owner color does not require an edge or infer rarity"),
            UFMCodexPlayerCardWidget::ResolvePitchMiniOwnerColor(Card),Card.PitchMiniOwnershipAccentColor);
        Widget->RefreshFromPresentation(Card,EFMCodexPlayerCardPresentationMode::HandMicro);
        TestFalse(TEXT("Hand has no Pitch pilot paint"),Widget->IsPitchMiniPilot());
        TestNull(TEXT("Hand return releases Shared"),Widget->GetResolvedPortraitTexture());
        TestEqual(TEXT("Hand route remains accepted"),Widget->GetResolvedHandMicroPortraitTexture()->GetPathName(),Art.HandMicroPortrait.ToSoftObjectPath().ToString());
        Widget->RefreshFromPresentation(Card,EFMCodexPlayerCardPresentationMode::InteractionChoice);
        TestFalse(TEXT("Full has no Pitch pilot paint"),Widget->IsPitchMiniPilot());
        TestEqual(TEXT("Full remains old production"),Widget->GetResolvedPortraitTexture()->GetPathName(),Art.FullCardPortrait.ToSoftObjectPath().ToString());
    }
    for (const FLinearColor Color : {FLinearColor(.1f,.7f,.3f,1.f),FLinearColor(.7f,.12f,.8f,1.f)})
    {
        Card.PitchMiniOwnershipAccentColor=Color;
        TestEqual(TEXT("Player palette is honored without A/B hard coding"),UFMCodexPlayerCardWidget::ResolvePitchMiniOwnerColor(Card),Color);
    }
    Card.bHasPitchMiniOwnershipAccent=false;
    const auto Fallback=UFMCodexPlayerCardWidget::ResolvePitchMiniOwnerColor(Card);
    TestEqual(TEXT("Missing color fallback"),Fallback,FLinearColor::FromSRGBColor(FColor(109,137,157)));
    Card.bHasPitchMiniOwnershipAccent=true;Card.PitchMiniOwnershipAccentColor=FLinearColor::Transparent;
    TestEqual(TEXT("Transparent color fallback"),UFMCodexPlayerCardWidget::ResolvePitchMiniOwnerColor(Card),Fallback);
    Card.CardId=TEXT("Prototype.Arsenal.MikelMerino");
    Widget->RefreshFromPresentation(Card,EFMCodexPlayerCardPresentationMode::PitchMini);
    TestFalse(TEXT("Unmigrated player stays legacy"),Widget->IsPitchMiniPilot());
    Card.CardId=TEXT("Prototype.ManchesterCity.RayanAitNouri");
    Card.IdentityLabel=TEXT("艾特-努里");
    Card.RoleLabel=TEXT("M/D");
    Widget->RefreshFromPresentation(Card,EFMCodexPlayerCardPresentationMode::PitchMini);
    const auto* Name=CastChecked<UTextBlock>(Widget->GetWidgetFromName(TEXT("PitchMiniPlayerName")));
    const auto* Role=CastChecked<UTextBlock>(Widget->GetWidgetFromName(TEXT("PitchMiniPosition")));
    const auto* Separator=CastChecked<UTextBlock>(Widget->GetWidgetFromName(TEXT("PitchMiniIdentitySeparator")));
    const auto Measure=FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
    const float Used=Measure->Measure(Name->GetText(),Name->GetFont()).X
        + Measure->Measure(Role->GetText(),Role->GetFont()).X
        + Measure->Measure(Separator->GetText(),Separator->GetFont()).X + 6.f + 4.f;
    TestEqual(TEXT("Preferred legacy name stays complete"),Name->GetText().ToString(),FString(TEXT("艾特-努里")));
    TestTrue(TEXT("Legacy name and position fit the fixed information row"),Used <= 130.f && Name->GetFont().Size >= 12);
    TestNull(TEXT("Legacy Pitch also skips obsolete frame texture"),Widget->GetResolvedCardFrameTexture());
    TestNull(TEXT("Legacy Pitch does not acquire Hand"),Widget->GetResolvedHandMicroPortraitTexture());

    TestEqual(TEXT("Legacy shares refined information-row surface"),CastChecked<UBorder>(Widget->GetWidgetFromName(TEXT("PitchMiniIdentitySurface")))->GetBrushColor(),
        FLinearColor::FromSRGBColor(FColor(7,21,34)));
    return true;
}
// Opt-in review fixture: real Local authority initializes from explicit opening dice.
// It does not alter the production demo's fixed turn contract or fabricate a View.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FFMCodexPitchMiniGoalkeeperReviewStart,
    "FMCodex.LocalPlay.PitchMiniPresentation.06.GoalkeeperReviewStart",
    EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexPitchMiniGoalkeeperReviewStart::RunTest(const FString& Parameters)
{
    if (!FParse::Param(FCommandLine::Get(), TEXT("PitchMiniGoalkeeperReview")))
    { AddInfo(TEXT("Explicit live review opt-in required; no match changed.")); return true; }
    for (const auto& Context : GEngine->GetWorldContexts())
    {
        UWorld* World=Context.World();
        if (!World || World->WorldType!=EWorldType::Game) continue;
        auto* Host=World->GetAuthGameMode<AFMCodexLocalMatchHostGameMode>();
        auto* Controller=Cast<AFMCodexLocalMatchPlayerController>(World->GetFirstPlayerController());
        if (!Host || !Controller) continue;
        auto Demo=FFMCodexLocalMatchDemoConfigurationFactory::Create();
        auto& Opening=Demo.OpeningInput.OpeningInput;
        Opening.bUseFixedPrototypeAttackTurnContract=false;
        const bool bBatch1 = FParse::Param(FCommandLine::Get(), TEXT("CanonicalArtBatch1Review"));
        Opening.PlayerAAttackCountD6Roll=bBatch1 ? 6 : 1;Opening.PlayerBAttackCountD6Roll=bBatch1 ? 1 : 6;
        Opening.PlayerATieBreakerRoll=1;Opening.PlayerBTieBreakerRoll=6;
        const auto Result=Host->StartNewLocalMatch(Demo.OpeningInput,Demo.SkillRuleSet);
        TestTrue(TEXT("Review initialized through Local authority"),Result.bSuccess);
        Controller->RefreshPresentation();
        TestTrue(TEXT("Review opening permits the requested opposing goalkeeper through legal deployment"),
            Controller->GetInteractionView().CurrentAttackingPlayer==(bBatch1 ? EInitialTurnOrderPlayer::PlayerA : EInitialTurnOrderPlayer::PlayerB));
        FFMCodexLocalDevRollOverrideRequest Roll;
        Roll.Target=EFMCodexLocalDevRollTarget::FullD12;Roll.Value=bBatch1 ? 8 : 4;
        TestTrue(TEXT("Ordinary deployment roll uses existing DEV provider seam"),Controller->SetLocalDevRollOverride(Roll).bSuccess);
        AddInfo(TEXT("PITCH_GOALKEEPER_REVIEW: canonical opening evaluation; explicit DEV dice; production defaults unchanged"));
        return true;
    }
    AddError(TEXT("No live LocalPlay match for goalkeeper review"));return false;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FFMCodexPitchMiniOwnerReadabilityTest,
    "FMCodex.LocalPlay.PitchMiniPresentation.07.OwnerReadability",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexPitchMiniOwnerReadabilityTest::RunTest(const FString& Parameters)
{
    FFMCodexUMGCardViewModel Card;
    Card.bHasPitchMiniOwnershipAccent = true;
    const TArray<FLinearColor> Samples = {
        FLinearColor::FromSRGBColor(FColor(0x4F,0x78,0x92)),
        FLinearColor(.001f,.005f,.055f), FLinearColor(.01f,.08f,.015f),
        FLinearColor(.035f,.003f,.07f), FLinearColor(.055f,.002f,.008f),
        FLinearColor(0,0,1), FLinearColor(.7f,.6f,0),
        FLinearColor(.7f,.8f,.9f), FLinearColor::Black, FLinearColor(.01f,.01f,.01f)
    };
    for (const FLinearColor Source : Samples)
    {
        Card.PitchMiniOwnershipAccentColor = Source;
        const FLinearColor Accent = UFMCodexPlayerCardWidget::ResolvePitchMiniRenderedOwnerColor(Card);
        const float Luminance = .2126f*Accent.R + .7152f*Accent.G + .0722f*Accent.B;
        TestTrue(TEXT("Arbitrary dark owner accents meet perceptual readability floor"), Luminance >= .22f-UE_KINDA_SMALL_NUMBER);
        TestTrue(TEXT("Rendered accent remains finite, in gamut and opaque"),
            FMath::IsFinite(Luminance) && Accent.R >= 0 && Accent.G >= 0 && Accent.B >= 0
            && Accent.R <= 1 && Accent.G <= 1 && Accent.B <= 1 && Accent.A == 1);
        TestEqual(TEXT("Stored/projected color is never mutated"), Card.PitchMiniOwnershipAccentColor, Source);
        const FLinearColor Before = Source.LinearRGBToHSV();
        const FLinearColor After = Accent.LinearRGBToHSV();
        if (Before.G > UE_KINDA_SMALL_NUMBER)
        {
            TestTrue(TEXT("Hue identity survives exposure and bounded neutral mix"),
                FMath::Abs(FMath::UnwindDegrees(After.R-Before.R)) < .02f);
            TestTrue(TEXT("Saturation remains recognizable without amplification"),
                After.G >= Before.G*.7f && After.G <= Before.G+UE_KINDA_SMALL_NUMBER);
        }
        else
        {
            TestTrue(TEXT("Valid achromatic choices stay achromatic"),
                FMath::IsNearlyEqual(Accent.R,Accent.G) && FMath::IsNearlyEqual(Accent.G,Accent.B));
        }
        Card.PitchMiniOwnershipAccentColor = Accent;
        TestTrue(TEXT("Repeated presentation normalization does not keep brightening"),
            UFMCodexPlayerCardWidget::ResolvePitchMiniRenderedOwnerColor(Card).Equals(Accent,UE_KINDA_SMALL_NUMBER));
    }
    Card.bHasPitchMiniOwnershipAccent = false;
    const FLinearColor Fallback = UFMCodexPlayerCardWidget::ResolvePitchMiniRenderedOwnerColor(Card);
    Card.bHasPitchMiniOwnershipAccent = true;
    for (const FLinearColor Invalid : {FLinearColor::Transparent,
        FLinearColor(std::numeric_limits<float>::quiet_NaN(),0,0),
        FLinearColor(0,std::numeric_limits<float>::infinity(),0)})
    {
        Card.PitchMiniOwnershipAccentColor = Invalid;
        TestEqual(TEXT("Invalid colors use the same normalized neutral fallback"),
            UFMCodexPlayerCardWidget::ResolvePitchMiniRenderedOwnerColor(Card),Fallback);
    }
    Card.PitchMiniOwnershipAccentColor = FLinearColor::Black;
    TestFalse(TEXT("Valid black is readable charcoal, not missing-color fallback"),
        UFMCodexPlayerCardWidget::ResolvePitchMiniRenderedOwnerColor(Card).Equals(Fallback));
    return true;
}

#endif
