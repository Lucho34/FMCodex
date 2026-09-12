#include "FMCodexPlayerCardWidget.h"
#include "FMCodexPlayerUIAssetReferences.h"
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
        TestFalse(TEXT("Full route remains legacy"), Art.FullCardPortrait.ToSoftObjectPath().ToString().Contains(TEXT("/Canonical/")));
        for (const auto Mode : {EFMCodexPlayerCardPresentationMode::PitchMini, EFMCodexPlayerCardPresentationMode::InteractionChoice})
        {
            Card->RefreshFromPresentation(Model,Mode);
            const auto Expected = Mode == EFMCodexPlayerCardPresentationMode::PitchMini ? Art.Portrait : Art.FullCardPortrait;
            if (TestNotNull(TEXT("Existing non-Hand portrait resolves"),Card->GetResolvedPortraitTexture()))
                TestEqual(TEXT("Non-Hand uses retained production route"),Card->GetResolvedPortraitTexture()->GetPathName(),Expected.ToSoftObjectPath().ToString());
            TestFalse(TEXT("Hand surface does not activate for other modes"),Card->IsCanonicalCardFamily());
            TestTrue(TEXT("No new number display outside Hand"),Card->GetWidgetFromName(TEXT("AssignedPlayerNumber"))->GetVisibility()==ESlateVisibility::Collapsed);
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
#endif
