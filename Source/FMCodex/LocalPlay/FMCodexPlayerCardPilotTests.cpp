#include "FMCodexPlayerCardWidget.h"
#include "FMCodexFullCardSurface.h"
#include "FMCodexPlayerUIAssetReferences.h"
#include "FMCodexPrototypeTeamContent.h"
#include "FMCodexLocalMatchInteractionView.h"
#include "FMCodexLocalMatchUMGPresentation.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/Texture2D.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "FMCodexPlayerCardSurface.h"
#include "FMCodexDeploymentDragDropOperation.h"
#include "Components/SizeBox.h"
#include "Components/OverlaySlot.h"
#include "UObject/UObjectIterator.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexHandCompactPilotTest,
    "FMCodex.LocalPlay.UI.HandCompactPilot.DataRarityAndRoleIsolation",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexHandCompactPilotTest::RunTest(const FString&)
{
    // Explicit DEV verification of the already-running player-facing Hand.
    // Uses its existing legal drag configuration; never fabricates availability.
    if (FParse::Param(FCommandLine::Get(), TEXT("HandCloseoutRuntime")))
    {
        UFMCodexPlayerCardWidget* LiveHand = nullptr;
        for (TObjectIterator<UFMCodexPlayerCardWidget> It; It; ++It)
        {
            if (!It->IsTemplate() && It->GetWorld() && It->GetWorld()->WorldType == EWorldType::Game
                && It->GetPresentationMode() == EFMCodexPlayerCardPresentationMode::HandMicro
                && It->IsCanonicalCardFamily() && It->IsDeploymentDragEnabled())
            { LiveHand = *It; break; }
        }
        if (!TestNotNull(TEXT("Fresh runtime has an existing legally draggable Hand"),LiveHand)) return false;
        UTexture2D* HandTexture = LiveHand->GetResolvedHandMicroPortraitTexture();
        auto* LiveDrag = LiveHand->BeginDeploymentDrag();
        if (!TestNotNull(TEXT("Live production Hand starts its configured drag"),LiveDrag)) return false;
        auto* Proxy = Cast<UFMCodexPlayerCardWidget>(LiveDrag->DefaultDragVisual);
        if (Proxy) Proxy->TakeWidget();
        TestTrue(TEXT("Live Hand proxy acquires only the same small portrait"),Proxy
            && Proxy->GetResolvedHandMicroPortraitTexture()==HandTexture && !Proxy->GetResolvedPortraitTexture());
        TestTrue(TEXT("Live source enters drag state"),LiveHand->IsDragSourcePresentationActive());
        LiveDrag->DragCancelled(FPointerEvent());
        TestFalse(TEXT("Live cancel restores source state"),LiveHand->IsDragSourcePresentationActive());
        TestEqual(TEXT("Live cancel restores opacity"),LiveHand->GetRenderOpacity(),1.f);
        TestNull(TEXT("Live Hand return retains no large portrait"),LiveHand->GetResolvedPortraitTexture());
        AddInfo(TEXT("HAND_CLOSEOUT_LIVE_DRAG_CHECK_COMPLETED: existing legal source, native drag/cancel, Hand-only proxy"));
    }

    UWorld* World=UWorld::CreateWorld(EWorldType::Game,false);
    if (!TestNotNull(TEXT("Hand test world"),World)) return false;
    GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
    UFMCodexPlayerCardWidget* Card=CreateWidget<UFMCodexPlayerCardWidget>(World);
    if (!Card) { GEngine->DestroyWorldContext(World); World->DestroyWorld(false); return false; }
    Card->TakeWidget();
    for (const TCHAR* Key : {TEXT("Prototype.Arsenal.BukayoSaka"),TEXT("Prototype.Arsenal.DavidRaya"),
        TEXT("Prototype.ManchesterCity.Rodri"),TEXT("Prototype.ManchesterCity.ErlingHaaland")})
    {
        const FString ExpectedName=FString(Key).EndsWith(TEXT("BukayoSaka")) ? TEXT("萨卡")
            : FString(Key).EndsWith(TEXT("DavidRaya")) ? TEXT("拉亚")
            : FString(Key).EndsWith(TEXT("Rodri")) ? TEXT("罗德里") : TEXT("哈兰德");
        FFMCodexLocalMatchCardView View; View.CardId=Key; View.DisplayLabel=ExpectedName;
        View.CompactRoleLabel=TEXT("GK"); View.AssignedPlayerNumber=TEXT("7");
        View.PlayerFacingSerialLabel=TEXT("015"); View.bAvailable=true;
        auto Model=FFMCodexLocalMatchUMGPresentationBuilder::BuildCard(View);
        Card->RefreshFromPresentation(Model,EFMCodexPlayerCardPresentationMode::HandMicro);
        UTexture2D* Texture=Card->GetResolvedHandMicroPortraitTexture();
        if (TestNotNull(TEXT("Hand derivative exists"),Texture))
        {
            TestEqual(TEXT("Hand texture is purpose sized"),Texture->GetImportedSize(),FIntPoint(192,128));
            TestTrue(TEXT("Correct identity route"),Texture->GetPathName().Contains(FString(Key).Replace(TEXT("."),TEXT("_"))));
        }
        TestNull(TEXT("Hand does not retain larger portrait"),Card->GetResolvedPortraitTexture());
        TestEqual(TEXT("Catalog serial is independent of shirt number"), Model.PlayerFacingSerialLabel, FString(TEXT("015")));
        for (const TCHAR* ImageName : {TEXT("PortraitAssetImage"), TEXT("PitchMiniPortraitImage"), TEXT("CardFrameAssetImage")})
        {
            const UImage* Image = Cast<UImage>(Card->GetWidgetFromName(ImageName));
            if (Image) TestNull(TEXT("Hand clears inactive image brush"), Image->GetBrush().GetResourceObject());
        }
        TestNull(TEXT("Hand does not acquire unused frame"),Card->GetResolvedCardFrameTexture());
        TestNull(TEXT("Hand does not acquire unused role icon"),Card->GetResolvedRoleIconTexture());
        TestNull(TEXT("Hand does not acquire unused skill icon"),Card->GetResolvedLongShotSkillIconTexture());
        const auto Art = FFMCodexPlayerUIAssetReferences::Get().ResolveCardArt(Model.CardId);
        TestFalse(TEXT("Shared route remains legacy"), Art.Portrait.ToSoftObjectPath().ToString().Contains(TEXT("/Canonical/")));
        TestTrue(TEXT("Full pilot has its own canonical route"), Art.FullCardPortrait.ToSoftObjectPath().ToString().Contains(TEXT("/Canonical/")));
        for (const auto Mode : {EFMCodexPlayerCardPresentationMode::PitchMini, EFMCodexPlayerCardPresentationMode::InteractionChoice})
        {
            Card->RefreshFromPresentation(Model,Mode);
            const auto Expected = Mode == EFMCodexPlayerCardPresentationMode::PitchMini ? Art.PitchMiniPortrait : Art.FullCardPortrait;
            if (TestNotNull(TEXT("Existing non-Hand portrait resolves"),Card->GetResolvedPortraitTexture()))
                TestEqual(TEXT("Non-Hand uses retained production route"),Card->GetResolvedPortraitTexture()->GetPathName(),Expected.ToSoftObjectPath().ToString());
            TestFalse(TEXT("Hand surface does not activate for other modes"),Card->IsCanonicalCardFamily());
            TestTrue(TEXT("Hand number node stays hidden outside Hand"),Card->GetWidgetFromName(TEXT("AssignedPlayerNumber"))->GetVisibility()==ESlateVisibility::Collapsed);
            Card->RefreshFromPresentation(Model,EFMCodexPlayerCardPresentationMode::HandMicro);
            TestEqual(TEXT("Returning to Hand restores original small texture"),Card->GetResolvedHandMicroPortraitTexture(),Texture);
            TestNull(TEXT("Returning to Hand clears larger member"),Card->GetResolvedPortraitTexture());
            for (const TCHAR* ImageName : {TEXT("PortraitAssetImage"),TEXT("PitchMiniPortraitImage")})
                TestNull(TEXT("Returning to Hand clears larger brush"),CastChecked<UImage>(Card->GetWidgetFromName(ImageName))->GetBrush().GetResourceObject());
            const auto* Surface=Cast<UFMCodexPlayerCardSurface>(Card->GetWidgetFromName(TEXT("HandMicroIdentitySurface")));
            TestTrue(TEXT("Returning to Hand restores accepted material"),Surface && Surface->bPremium);
        }

        TestEqual(TEXT("Current card hit geometry"),Card->GetConfiguredDimensions(),FVector2D(220,68));
        TestEqual(TEXT("Single active localized name"),CastChecked<UTextBlock>(Card->GetWidgetFromName(TEXT("HandMicroPlayerName")))->GetText().ToString(),ExpectedName);
        TestEqual(TEXT("Role supplied by presentation"),CastChecked<UTextBlock>(Card->GetWidgetFromName(TEXT("HandMicroPosition")))->GetText().ToString(),FString(TEXT("GK")));
        Model.AssignedPlayerNumber=TEXT("99");
        Card->RefreshFromPresentation(Model,EFMCodexPlayerCardPresentationMode::HandMicro);
        TestEqual(TEXT("Mutable shirt number"),Card->GetRenderedAssignedNumber().ToString(),FString(TEXT("99")));
        TestEqual(TEXT("Number update reuses portrait"),Card->GetResolvedHandMicroPortraitTexture(),Texture);
        const UTextBlock* Number=CastChecked<UTextBlock>(Card->GetWidgetFromName(TEXT("AssignedPlayerNumber")));
        const UTextBlock* Position=CastChecked<UTextBlock>(Card->GetWidgetFromName(TEXT("HandMicroPosition")));
        TestTrue(TEXT("Present Hand number has visible presentation state"),
            Number->GetVisibility()==ESlateVisibility::HitTestInvisible);
        const FLinearColor NumberColor=Number->GetColorAndOpacity().GetSpecifiedColor();
        const FLinearColor PositionColor=Position->GetColorAndOpacity().GetSpecifiedColor();
        TestTrue(TEXT("Number is opaque and readable, below position hierarchy"),
            NumberColor.A==1.f && NumberColor.GetLuminance()>.3f
            && NumberColor.GetLuminance()<PositionColor.GetLuminance());
        TestTrue(TEXT("Pilot no longer paints a separate thick rarity strip"),
            Card->GetWidgetFromName(TEXT("HandMicroRarityAccent"))->GetVisibility()==ESlateVisibility::Hidden);
        TestEqual(TEXT("Removing strip paint preserves content width"),
            CastChecked<USizeBox>(Card->GetWidgetFromName(TEXT("HandMicroIdentityBounds")))->GetWidthOverride(),120.f);
        Model.AssignedPlayerNumber.Empty();
        Card->RefreshFromPresentation(Model,EFMCodexPlayerCardPresentationMode::HandMicro);
        TestTrue(TEXT("Empty shirt number does not invent one from serial"),Card->GetWidgetFromName(TEXT("AssignedPlayerNumber"))->GetVisibility()==ESlateVisibility::Collapsed);
        Card->ConfigureDeploymentDrag(Model.CardId,false);
        auto* Drag=Card->BeginDeploymentDrag();
        if (TestNotNull(TEXT("Existing permitted drag still starts"),Drag))
        {
            auto* Proxy=Cast<UFMCodexPlayerCardWidget>(Drag->DefaultDragVisual);
            if (Proxy) Proxy->TakeWidget(); // Slate constructs the production drag proxy when drag starts.
            TestTrue(TEXT("Drag reuses Hand derivative"),Proxy && Proxy->GetResolvedHandMicroPortraitTexture()==Texture && !Proxy->GetResolvedPortraitTexture());
            TestFalse(TEXT("Full hover suppressed during drag"),Card->CanExposeFullCardDetail());
            Drag->DragCancelled(FPointerEvent());
            TestFalse(TEXT("Cancel clears drag presentation"),Card->IsDragSourcePresentationActive());
            TestEqual(TEXT("Cancel restores opacity"),Card->GetRenderOpacity(),1.f);
        }
        Card->ClearDeploymentDrag();
        TestNull(TEXT("Non-draggable Hand cannot start drag"),Card->BeginDeploymentDrag());
        Card->ConfigureOnPitchSelection(TEXT("ExistingOption"),true);
        TestTrue(TEXT("Existing selection path remains available"),Card->RequestOnPitchSelection());
        Card->ClearOnPitchSelection();
        TestFalse(TEXT("Unavailable selection is rejected"),Card->RequestOnPitchSelection());
    }
    // The shared widget must restore the untouched legacy Hand when rebound.
    {
        auto Legacy=Card->GetPresentation(); Legacy.CardId=TEXT("Prototype.Arsenal.MikelMerino");
        Card->RefreshFromPresentation(Legacy,EFMCodexPlayerCardPresentationMode::HandMicro);
        TestTrue(TEXT("Legacy Hand keeps its existing strip"),
            Card->GetWidgetFromName(TEXT("HandMicroRarityAccent"))->GetVisibility()==ESlateVisibility::HitTestInvisible);
        Legacy.CardId=TEXT("Prototype.Arsenal.BukayoSaka");
        Card->RefreshFromPresentation(Legacy,EFMCodexPlayerCardPresentationMode::HandMicro);
        TestTrue(TEXT("Rebinding to pilot removes legacy strip again"),
            Card->GetWidgetFromName(TEXT("HandMicroRarityAccent"))->GetVisibility()==ESlateVisibility::Hidden);
    }
    const TArray<TPair<FString,FColor>> Rarities={
        {TEXT("Common"),FColor(255,255,255)},{TEXT("Regional"),FColor(0x1E,0xFF,0)},
        {TEXT("National"),FColor(0,0x70,0xDD)},{TEXT("Continental"),FColor(0xA3,0x35,0xEE)},
        {TEXT("World Class"),FColor(0xFF,0x80,0)}};
    for (const auto& Rarity : Rarities)
    {
        auto Model=Card->GetPresentation(); Model.RarityLabel=Rarity.Key;
        Model.PitchMiniOwnershipAccentColor=FLinearColor::Red;
        Card->RefreshFromPresentation(Model,EFMCodexPlayerCardPresentationMode::HandMicro);
        FColor Actual=CastChecked<UBorder>(Card->GetWidgetFromName(TEXT("HandMicroRarityAccent")))->GetBrushColor().ToFColorSRGB(); Actual.A=255;
        TestEqual(TEXT("Canonical Hand rarity hue, independent of side"),Actual,Rarity.Value);
    }
    Card->RemoveFromParent(); GEngine->DestroyWorldContext(World); World->DestroyWorld(false);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexFullCardUnifiedPilotTest,
    "FMCodex.LocalPlay.UI.FullCardUnifiedPilot.NumberAndPurposeIsolation",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexFullCardUnifiedPilotTest::RunTest(const FString&)
{
    UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
    if (!TestNotNull(TEXT("Full pilot world"), World)) return false;
    GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
    UFMCodexPlayerCardWidget* Card = CreateWidget<UFMCodexPlayerCardWidget>(World);
    if (!Card) { GEngine->DestroyWorldContext(World); World->DestroyWorld(false); return false; }
    Card->TakeWidget();
    for (const TCHAR* Key : {TEXT("Prototype.Arsenal.BukayoSaka"),TEXT("Prototype.Arsenal.DavidRaya"),
        TEXT("Prototype.ManchesterCity.Rodri"),TEXT("Prototype.ManchesterCity.ErlingHaaland")})
    {
        FFMCodexUMGCardViewModel Model;
        Model.CardId = Key; Model.IdentityLabel = TEXT("测试姓名");
        Model.PlayerFacingSerialLabel = TEXT("015");
        Model.RarityLabel = TEXT("Legendary");
        FFMCodexUMGAttributeViewModel Stat; Stat.CanonicalLabel = TEXT("SHO"); Stat.Value = 5;
        Model.AttributeValues.Add(Stat);
        FFMCodexUMGSkillViewModel Skill; Skill.CanonicalLabel = TEXT("Crossing");
        Skill.MinTriggerActionPoint = 4; Skill.MaxTriggerActionPoint = 6;
        Model.Skills.Add(Skill);
        Card->RefreshFromPresentation(Model, EFMCodexPlayerCardPresentationMode::InteractionChoice);
        auto CheckSurface = [this,Card](const TCHAR* Name, EFMCodexFullCardSurface Kind)
        {
            auto* Surface = Cast<UFMCodexFullCardSurface>(Card->GetWidgetFromName(Name));
            TestTrue(FString::Printf(TEXT("Full geometry contract: %s"), Name),
                Surface != nullptr && Surface->GetSurface() == Kind);
        };
        CheckSurface(TEXT("InMatchFullCardBiographyRegion"), EFMCodexFullCardSurface::Biography);
        CheckSurface(TEXT("CardIdentityRegion"), EFMCodexFullCardSurface::Identity);
        CheckSurface(TEXT("AttributePresentationRegion"), EFMCodexFullCardSurface::Attributes);
        CheckSurface(TEXT("AttributeCell0"), EFMCodexFullCardSurface::AttributeRow);
        CheckSurface(TEXT("AttributeTierBadge0"), EFMCodexFullCardSurface::Value);
        CheckSurface(TEXT("FullCardSkillRow0"), EFMCodexFullCardSurface::SkillRow);
        CheckSurface(TEXT("FullCardSkillRange0"), EFMCodexFullCardSurface::Range);
        TestTrue(TEXT("Empty shirt number hides its entire plate"),
            Card->GetWidgetFromName(TEXT("FullCardNumberPlateBounds"))->GetVisibility() == ESlateVisibility::Collapsed);
        auto* Full = Card->GetResolvedPortraitTexture();
        if (TestNotNull(TEXT("Full has a dedicated derivative"), Full))
            TestEqual(TEXT("Full runtime resolution"), Full->GetImportedSize(), FIntPoint(768,1152));
        TestNull(TEXT("Full does not acquire Hand"), Card->GetResolvedHandMicroPortraitTexture());
        TestNull(TEXT("Full does not acquire unused frame"), Card->GetResolvedCardFrameTexture());
        TestNull(TEXT("Full does not acquire unused role icon"), Card->GetResolvedRoleIconTexture());
        TestNull(TEXT("Full does not acquire unused skill icon"), Card->GetResolvedLongShotSkillIconTexture());
        for (const TCHAR* Node : {TEXT("HandMicroFaceSafePortrait"),TEXT("PitchMiniPortraitImage")})
            TestNull(TEXT("Full clears inactive purpose brushes"),CastChecked<UImage>(Card->GetWidgetFromName(Node))->GetBrush().GetResourceObject());
        auto* Number = CastChecked<UTextBlock>(Card->GetWidgetFromName(TEXT("FullCardAssignedNumber")));
        auto* Collection = CastChecked<UTextBlock>(Card->GetWidgetFromName(TEXT("FullCardCollectionLine")));
        TestTrue(TEXT("Unassigned number stays hidden despite serial"),Number->GetVisibility()==ESlateVisibility::Collapsed);
        TestTrue(TEXT("Collection uses real serial without invented denominator"),Collection->GetText().ToString().EndsWith(TEXT("015"))
            && !Collection->GetText().ToString().Contains(TEXT("/290")));
        Model.AssignedPlayerNumber = TEXT("99");
        Card->RefreshFromPresentation(Model,EFMCodexPlayerCardPresentationMode::InteractionChoice);
        TestEqual(TEXT("Independent number update"),Number->GetText().ToString(),FString(TEXT("99")));
        TestTrue(TEXT("Assigned number becomes visible"),Number->GetVisibility()==ESlateVisibility::HitTestInvisible);
        TestEqual(TEXT("Number updates reuse Full art"),Card->GetResolvedPortraitTexture(),Full);
        const TSharedRef<SWidget> NumberSlate = Card->TakeWidget();
        NumberSlate->SlatePrepass();
        TestTrue(TEXT("Two-digit kit number fits its actual text lane"),
            Number->GetDesiredSize().X > 0 && Number->GetDesiredSize().X <= 43.f);
        TestTrue(TEXT("Assigned number reveals its independent plate"),
            Card->GetWidgetFromName(TEXT("FullCardNumberPlateBounds"))->GetVisibility() == ESlateVisibility::HitTestInvisible);
        CheckSurface(TEXT("FullCardNumberPlate"), EFMCodexFullCardSurface::Number);
        for (const auto Mode : {EFMCodexPlayerCardPresentationMode::HandMicro,EFMCodexPlayerCardPresentationMode::PitchMini})
        {
            Card->RefreshFromPresentation(Model,Mode);
            TestTrue(TEXT("Full footer hides on mode switch"),Collection->GetVisibility()==ESlateVisibility::Collapsed);
            TestTrue(TEXT("Full number hides on mode switch"),Number->GetVisibility()==ESlateVisibility::Collapsed);
            for (const TCHAR* Node : {TEXT("InMatchFullCardBiographyRegion"), TEXT("CardIdentityRegion"),
                TEXT("AttributePresentationRegion"), TEXT("SkillPresentationRegion"), TEXT("FullCardNumberPlate")})
                CheckSurface(Node, EFMCodexFullCardSurface::None);
            TestTrue(TEXT("Other purpose hides the complete number plate"),
                Card->GetWidgetFromName(TEXT("FullCardNumberPlateBounds"))->GetVisibility() == ESlateVisibility::Collapsed);
            TestNull(TEXT("Full portrait brush released by other purposes"),CastChecked<UImage>(Card->GetWidgetFromName(TEXT("PortraitAssetImage")))->GetBrush().GetResourceObject());
            TestTrue(TEXT("Other purpose never holds Full member"),Card->GetResolvedPortraitTexture()!=Full);
        }
    }
    // Rebind the same live widget: geometry must not depend on the digit/player.
    auto Precision = Card->GetPresentation();
    Precision.bHasOverallRating = true;
    float RatingModuleWidth = 0.f;
    FMargin NumberPadding;
    FVector2D NumberTranslation;
    const TCHAR* KitNumbers[] = {TEXT("1"),TEXT("7"),TEXT("9"),TEXT("16")};
    const int32 Ratings[] = {93,97,99,100};
    const TCHAR* Dates[] = {TEXT("1995.09.15"),TEXT("2000.07.21"),TEXT("1996.06.22"),TEXT("2001.09.05")};
    for (int32 Case = 0; Case < 4; ++Case)
    {
        Precision.AssignedPlayerNumber = KitNumbers[Case];
        Precision.OverallRating = Ratings[Case];
        Precision.BirthDate = Dates[Case];
        Precision.HeightCm = 195; Precision.WeightKg = 87; Precision.RoleLabel = TEXT("M/D");
        Card->RefreshFromPresentation(Precision,EFMCodexPlayerCardPresentationMode::InteractionChoice);
        const TSharedRef<SWidget> Slate = Card->TakeWidget(); Slate->SlatePrepass();
        const auto* Number = CastChecked<UTextBlock>(Card->GetWidgetFromName(TEXT("FullCardAssignedNumber")));
        const auto* Plate = CastChecked<UBorder>(Card->GetWidgetFromName(TEXT("FullCardNumberPlate")));
        const float Lane = CastChecked<USizeBox>(Card->GetWidgetFromName(TEXT("FullCardNumberPlateBounds")))->GetWidthOverride()
            - Plate->GetPadding().Left - Plate->GetPadding().Right;
        const auto* Rating = CastChecked<UTextBlock>(Card->GetWidgetFromName(TEXT("OverallNumber")));
        const float GroupWidth = Card->GetWidgetFromName(TEXT("InMatchFullCardOverallGroup"))->GetDesiredSize().X;
        if (Case == 0) { RatingModuleWidth=GroupWidth; NumberPadding=Plate->GetPadding(); NumberTranslation=Number->GetRenderTransform().Translation; }
        TestEqual(TEXT("Rating module stays fixed through 93/97/99/100"),GroupWidth,RatingModuleWidth);
        const auto* BioBounds = CastChecked<USizeBox>(Card->GetWidgetFromName(TEXT("InMatchFullCardBiographyBounds")));
        const auto* BioSlot = CastChecked<UOverlaySlot>(BioBounds->Slot);
        const auto* Bio = CastChecked<UBorder>(Card->GetWidgetFromName(TEXT("InMatchFullCardBiographyRegion")));
        const float BioTextWidth = BioBounds->GetWidthOverride() - Bio->GetPadding().Left - Bio->GetPadding().Right;
        TestTrue(TEXT("Bio keeps its top anchor and approved right safety margin"),
            BioBounds->GetWidthOverride() == 96.f && BioSlot->GetHorizontalAlignment() == HAlign_Right
            && BioSlot->GetPadding().Top == 18.f && BioSlot->GetPadding().Right == 10.f
            && Bio->GetPadding().Left == 6.f && Bio->GetPadding().Right == 6.f);
        for (const TCHAR* Fact : {TEXT("BiographyBirthDate"),TEXT("BiographyHeight"),TEXT("BiographyWeight"),TEXT("BiographyPosition")})
        for (const TCHAR* Part : {TEXT("Label"),TEXT("Value")})
        {
            const auto* Text = CastChecked<UTextBlock>(Card->GetWidgetFromName(FName(*(FString(Fact)+Part))));
            TestTrue(FString::Printf(TEXT("Bio %s%s fits %.2f within %.2f"),Fact,Part,Text->GetDesiredSize().X,BioTextWidth),
                !Text->GetAutoWrapText() && Text->GetDesiredSize().X > 0.f && Text->GetDesiredSize().X <= BioTextWidth);
        }
        const auto* Date = CastChecked<UTextBlock>(Card->GetWidgetFromName(TEXT("BiographyBirthDateValue")));
        AddInfo(FString::Printf(TEXT("Bio date %s: measured %.2f / %.2f units, font %.1f"),
            Dates[Case], Date->GetDesiredSize().X, BioTextWidth, Date->GetFont().Size));
        TestTrue(TEXT("Legal rating is readable inside the common module"),Rating->GetDesiredSize().X <= GroupWidth
            && Rating->GetText().ToString() == FString::FromInt(Ratings[Case]));
        TestTrue(TEXT("All kit numbers fit a centered lane with rendering slack"),
            Number->GetDesiredSize().X > 0 && Number->GetDesiredSize().X + 4.f <= Lane
            && Plate->GetVerticalAlignment() == VAlign_Center);
        TestTrue(TEXT("Kit alignment has no digit-specific padding/translation"),
            Plate->GetPadding()==NumberPadding && Number->GetRenderTransform().Translation.Equals(NumberTranslation));
    }
    for (int32 Digit = 1; Digit <= 6; ++Digit)
    {
        Precision.AttributeValues[0].Value = Digit;
        Card->RefreshFromPresentation(Precision,EFMCodexPlayerCardPresentationMode::InteractionChoice);
        const TSharedRef<SWidget> Slate = Card->TakeWidget(); Slate->SlatePrepass();
        const auto* Value = CastChecked<UTextBlock>(Card->GetWidgetFromName(TEXT("AttributeValue0")));
        const auto* Badge = CastChecked<UBorder>(Card->GetWidgetFromName(TEXT("AttributeTierBadge0")));
        const auto* Bounds = CastChecked<USizeBox>(Card->GetWidgetFromName(TEXT("AttributeValueBounds0")));
        TestTrue(TEXT("Every tier digit fits the same centered value body"),
            Value->GetDesiredSize().X + 4.f <= Bounds->GetWidthOverride()
            && Value->GetDesiredSize().Y <= Bounds->GetHeightOverride()
            && Badge->GetVerticalAlignment()==VAlign_Center);
    }
    auto Legacy = Card->GetPresentation(); Legacy.CardId=TEXT("Prototype.Arsenal.MikelMerino");
    Card->RefreshFromPresentation(Legacy,EFMCodexPlayerCardPresentationMode::InteractionChoice);
    for (const TCHAR* Node : {TEXT("InMatchFullCardBiographyRegion"), TEXT("CardIdentityRegion"),
        TEXT("AttributePresentationRegion"), TEXT("SkillPresentationRegion"), TEXT("FullCardNumberPlate"),
        TEXT("AttributeTierBadge0"), TEXT("FullCardSkillRange0")})
    {
        const auto* Surface = Cast<UFMCodexFullCardSurface>(Card->GetWidgetFromName(Node));
        TestTrue(TEXT("Legacy rebind restores ordinary Border fallback"),
            Surface != nullptr && Surface->GetSurface() == EFMCodexFullCardSurface::None);
    }
    TestEqual(TEXT("Full-only caption protection clears on legacy rebind"),
        CastChecked<UTextBlock>(Card->GetWidgetFromName(TEXT("OverallLabel")))->GetFont().OutlineSettings.OutlineSize,0);
    TestEqual(TEXT("Legacy rating typography restored"),
        CastChecked<UTextBlock>(Card->GetWidgetFromName(TEXT("OverallNumber")))->GetFont().Size,44.f);
    TestEqual(TEXT("Legacy Full hero is restored"),CastChecked<USizeBox>(Card->GetWidgetFromName(TEXT("PortraitAssetBounds")))->GetHeightOverride(),320.f);
    TestTrue(TEXT("Legacy Full footer remains unchanged"),Card->GetWidgetFromName(TEXT("FullCardCollectionLine"))->GetVisibility()==ESlateVisibility::Collapsed);
    Card->RemoveFromParent();GEngine->DestroyWorldContext(World);World->DestroyWorld(false);
    return true;
}


// Batch migration changes asset enablement, not any card-surface implementation.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexCanonicalArtBatch1Test,
    "FMCodex.LocalPlay.UI.CanonicalArtBatch1.RoutesPurposeIsolationAndGoalkeeper",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexCanonicalArtBatch1Test::RunTest(const FString&)
{
    UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
    if (!TestNotNull(TEXT("Batch route world"), World)) return false;
    GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
    auto* Widget = CreateWidget<UFMCodexPlayerCardWidget>(World);
    if (!Widget) { GEngine->DestroyWorldContext(World); World->DestroyWorld(false); return false; }
    Widget->TakeWidget();
    for (const TCHAR* Key : {
        TEXT("Prototype.ManchesterCity.GianluigiDonnarumma"),
        TEXT("Prototype.Arsenal.GabrielMagalhaes"),
        TEXT("Prototype.Arsenal.MylesLewisSkelly"),
        TEXT("Prototype.Arsenal.RiccardoCalafiori"),
        TEXT("Prototype.ManchesterCity.JoskoGvardiol"),
        TEXT("Prototype.ManchesterCity.JeremyDoku"),
        TEXT("Prototype.Arsenal.GabrielMartinelli")})
    {
        const auto* Definition = FFMCodexPrototypeTeamContent::Find(FName(Key));
        if (!TestNotNull(FString::Printf(TEXT("Canonical data exists: %s"),Key),Definition)) continue;
        FFMCodexLocalMatchCardView View;
        View.CardId = Definition->PlayerKey;
        View.DisplayLabel = Definition->PreferredDisplayName.ToString();
        View.PlayerFacingSerialLabel = Definition->PlayerFacingSerial;
        View.bGoalkeeper = Definition->Card.bIsGoalkeeper;
        if (View.bGoalkeeper)
        {
            View.CompactRoleLabel = TEXT("GK");
            const auto& G = Definition->Card.GoalkeeperAttributes;
            View.AttributeValues = {{TEXT("HAN"),G.Handling},{TEXT("POS"),G.Positioning},
                {TEXT("REF"),G.Reflex},{TEXT("AER"),G.Aerial},{TEXT("ANT"),G.Anticipation},{TEXT("1V1"),G.OneOnOne}};
        }
        const auto Model = FFMCodexLocalMatchUMGPresentationBuilder::BuildCard(View);
        const auto Art = FFMCodexPlayerUIAssetReferences::Get().ResolveCardArt(Model.CardId);
        TestTrue(TEXT("Source-gated key is explicitly enabled"),Art.bCanonicalPlayerArt);
        const FString Token = FString(Key).Replace(TEXT("."),TEXT("_"));
        const auto ExpectedPath = [&Token](const TCHAR* Role) {
            const FString Name = FString::Printf(TEXT("T_%s_%s"),*Token,Role);
            return FString::Printf(TEXT("/Game/UI/Portraits/PrototypeTeams/Canonical/%s/%s.%s"),*Token,*Name,*Name);
        };
        TestEqual(TEXT("Exact Hand identity route"),Art.HandMicroPortrait.ToSoftObjectPath().ToString(),ExpectedPath(TEXT("Hand")));
        TestEqual(TEXT("Exact Pitch identity route"),Art.PitchMiniPortrait.ToSoftObjectPath().ToString(),ExpectedPath(TEXT("Shared")));
        TestEqual(TEXT("Exact Full identity route"),Art.FullCardPortrait.ToSoftObjectPath().ToString(),ExpectedPath(TEXT("Full")));
        // Reuse one real widget so stale textures/brushes across purpose and player changes are observable.
        for (const auto Mode : {EFMCodexPlayerCardPresentationMode::HandMicro,
            EFMCodexPlayerCardPresentationMode::PitchMini, EFMCodexPlayerCardPresentationMode::InteractionChoice,
            EFMCodexPlayerCardPresentationMode::HandMicro})
        {
            Widget->RefreshFromPresentation(Model,Mode);
            const bool Hand = Mode == EFMCodexPlayerCardPresentationMode::HandMicro;
            const bool Pitch = Mode == EFMCodexPlayerCardPresentationMode::PitchMini;
            const TCHAR* Role = Hand ? TEXT("Hand") : Pitch ? TEXT("Shared") : TEXT("Full");
            UTexture2D* Active = Hand ? Widget->GetResolvedHandMicroPortraitTexture() : Widget->GetResolvedPortraitTexture();
            if (TestNotNull(TEXT("Selected purpose texture loads"),Active))
            {
                TestEqual(TEXT("Active texture has exact key and purpose"),Active->GetPathName(),ExpectedPath(Role));
                TestEqual(TEXT("Purpose dimensions"),Active->GetImportedSize(),Hand ? FIntPoint(192,128) : Pitch ? FIntPoint(512,768) : FIntPoint(768,1152));
            }
            TestEqual(TEXT("Frozen card size"),Widget->GetConfiguredDimensions(),Hand ? FVector2D(220,68) : Pitch ? FVector2D(136,140) : FVector2D(360,540));
            if (Hand) TestNull(TEXT("Hand retains no large texture member"),Widget->GetResolvedPortraitTexture());
            else TestNull(TEXT("Pitch/Full retains no Hand texture member"),Widget->GetResolvedHandMicroPortraitTexture());
            for (const TCHAR* Node : {TEXT("HandMicroFaceSafePortrait"),TEXT("PitchMiniPortraitImage"),TEXT("PortraitAssetImage")})
            {
                const bool IsActive = FString(Node) == (Hand ? TEXT("HandMicroFaceSafePortrait") : Pitch ? TEXT("PitchMiniPortraitImage") : TEXT("PortraitAssetImage"));
                if (!IsActive)
                    TestNull(TEXT("Inactive purpose brush cleared"),CastChecked<UImage>(Widget->GetWidgetFromName(Node))->GetBrush().GetResourceObject());
            }
            TestNull(TEXT("No unused frame texture acquired"),Widget->GetResolvedCardFrameTexture());
            TestNull(TEXT("No unused role texture acquired"),Widget->GetResolvedRoleIconTexture());
            TestNull(TEXT("No unused skill texture acquired"),Widget->GetResolvedLongShotSkillIconTexture());
            TestEqual(TEXT("Presentation preserves canonical Chinese identity"),Widget->GetPresentation().IdentityLabel,Definition->PreferredDisplayName.ToString());
            TestEqual(TEXT("Collection serial preserved"),Widget->GetPresentation().PlayerFacingSerialLabel,Definition->PlayerFacingSerial);
            TestEqual(TEXT("Configured default shirt number is resolved"),Widget->GetPresentation().AssignedPlayerNumber,FString::FromInt(Definition->DefaultShirtNumber));
            if (!Hand && !Pitch && View.bGoalkeeper)
            {
                TestEqual(TEXT("Donnarumma retains six real GK attributes"),Widget->GetRenderedAttributeCount(),6);
                TestEqual(TEXT("GK has no invented skill"),Widget->GetRenderedSkillCount(),0);
                TestEqual(TEXT("GK Handling comes from canonical data"),Widget->GetPresentation().AttributeValues[0].Value,Definition->Card.GoalkeeperAttributes.Handling);
            }
        }
    }
    for (const TCHAR* Key : {TEXT("Prototype.Arsenal.MikelMerino"),TEXT("Prototype.ManchesterCity.NathanAke")})
    {
        const auto Art = FFMCodexPlayerUIAssetReferences::Get().ResolveCardArt(FName(Key));
        TestFalse(TEXT("Non-migrated and source-missing keys stay inactive"),Art.bCanonicalPlayerArt);
        for (const auto& Path : {Art.HandMicroPortrait.ToSoftObjectPath(),Art.PitchMiniPortrait.ToSoftObjectPath(),Art.FullCardPortrait.ToSoftObjectPath()})
            TestFalse(TEXT("No nonexistent canonical fallback"),Path.ToString().Contains(TEXT("/Canonical/")));
    }
    FFMCodexUMGCardViewModel Legacy; Legacy.CardId=TEXT("Prototype.Arsenal.MikelMerino");
    const auto LegacyArt=FFMCodexPlayerUIAssetReferences::Get().ResolveCardArt(Legacy.CardId);
    Widget->RefreshFromPresentation(Legacy,EFMCodexPlayerCardPresentationMode::HandMicro);
    if (TestNotNull(TEXT("Legacy Hand still loads"),Widget->GetResolvedHandMicroPortraitTexture()))
        TestEqual(TEXT("Legacy rebind keeps original Hand route"),Widget->GetResolvedHandMicroPortraitTexture()->GetPathName(),LegacyArt.HandMicroPortrait.ToSoftObjectPath().ToString());
    Widget->RefreshFromPresentation(Legacy,EFMCodexPlayerCardPresentationMode::PitchMini);
    if (TestNotNull(TEXT("Legacy Pitch still loads"),Widget->GetResolvedPortraitTexture()))
        TestEqual(TEXT("Legacy rebind keeps original Pitch route"),Widget->GetResolvedPortraitTexture()->GetPathName(),LegacyArt.Portrait.ToSoftObjectPath().ToString());
    Widget->RefreshFromPresentation(Legacy,EFMCodexPlayerCardPresentationMode::InteractionChoice);
    if (TestNotNull(TEXT("Legacy Full still loads"),Widget->GetResolvedPortraitTexture()))
        TestEqual(TEXT("Legacy rebind keeps original Full route"),Widget->GetResolvedPortraitTexture()->GetPathName(),LegacyArt.FullCardPortrait.ToSoftObjectPath().ToString());
    Widget->RemoveFromParent(); GEngine->DestroyWorldContext(World); World->DestroyWorld(false);
    return true;
}

#endif
