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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexCanonicalArtBatch2Test,
    "FMCodex.LocalPlay.UI.CanonicalArtBatch2.RoutesIsolationAndDataContract",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexCanonicalArtBatch2Test::RunTest(const FString&)
{
    using EMode = EFMCodexPlayerCardPresentationMode;
    UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
    if (!TestNotNull(TEXT("Batch 2 widget world"), World)) return false;
    GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
    auto* Widget = CreateWidget<UFMCodexPlayerCardWidget>(World);
    if (!TestNotNull(TEXT("Batch 2 widget"), Widget))
    { GEngine->DestroyWorldContext(World); World->DestroyWorld(false); return false; }
    Widget->TakeWidget();
    auto Text = [&](const TCHAR* Name) {
        return CastChecked<UTextBlock>(Widget->GetWidgetFromName(Name))->GetText().ToString();
    };
    const TMap<FName, int32> Players = {
        {TEXT("Prototype.Arsenal.WilliamSaliba"),3}, {TEXT("Prototype.Arsenal.JurrienTimber"),6},
        {TEXT("Prototype.Arsenal.MartinOdegaard"),10}, {TEXT("Prototype.Arsenal.DeclanRice"),14},
        {TEXT("Prototype.ManchesterCity.RubenDias"),2}, {TEXT("Prototype.ManchesterCity.BernardoSilva"),10},
        {TEXT("Prototype.ManchesterCity.PhilFoden"),11},
        {TEXT("Prototype.ManchesterCity.NathanAke"),5}, {TEXT("Prototype.ManchesterCity.RayanCherki"),12}};
    for (const auto& Pair : Players)
    {
        const auto* D = FFMCodexPrototypeTeamContent::Find(Pair.Key);
        if (!TestNotNull(TEXT("Canonical roster definition"), D)) continue;
        FFMCodexLocalMatchCardView View;
        View.CardId = D->PlayerKey; View.DisplayLabel = D->PreferredDisplayName.ToString();
        View.PlayerFacingSerialLabel = D->PlayerFacingSerial;
        View.BirthDate = D->Card.BirthDate; View.HeightCm = D->Card.HeightCm; View.WeightKg = D->Card.WeightKg;
        const auto Model = FFMCodexLocalMatchUMGPresentationBuilder::BuildCard(View);
        const auto Art = FFMCodexPlayerUIAssetReferences::Get().ResolveCardArt(Pair.Key);
        TestTrue(TEXT("Source-gated Batch 2 key explicitly enabled"), Art.bCanonicalPlayerArt);
        TestEqual(TEXT("Configured number preserved"), D->DefaultShirtNumber, Pair.Value);
        TestEqual(TEXT("Default number projected"), Model.AssignedPlayerNumber, FString::FromInt(Pair.Value));
        View.AssignedPlayerNumber = TEXT("98");
        TestEqual(TEXT("Explicit assignment still takes precedence"),
            FFMCodexLocalMatchUMGPresentationBuilder::BuildCard(View).AssignedPlayerNumber, FString(TEXT("98")));
        const FString Token = Pair.Key.ToString().Replace(TEXT("."), TEXT("_"));
        auto Path = [&Token](const TCHAR* Role) {
            const FString Name = FString::Printf(TEXT("T_%s_%s"), *Token, Role);
            return FString::Printf(TEXT("/Game/UI/Portraits/PrototypeTeams/Canonical/%s/%s.%s"), *Token, *Name, *Name);
        };
        TestEqual(TEXT("Hand route"), Art.HandMicroPortrait.ToSoftObjectPath().ToString(), Path(TEXT("Hand")));
        TestEqual(TEXT("Pitch route"), Art.PitchMiniPortrait.ToSoftObjectPath().ToString(), Path(TEXT("Shared")));
        TestEqual(TEXT("Full route"), Art.FullCardPortrait.ToSoftObjectPath().ToString(), Path(TEXT("Full")));
        for (const auto Mode : {EMode::HandMicro, EMode::PitchMini, EMode::InteractionChoice, EMode::HandMicro})
        {
            Widget->RefreshFromPresentation(Model, Mode);
            const bool Hand = Mode == EMode::HandMicro, Pitch = Mode == EMode::PitchMini;
            auto* Active = Hand ? Widget->GetResolvedHandMicroPortraitTexture() : Widget->GetResolvedPortraitTexture();
            if (TestNotNull(TEXT("Requested purpose loads"), Active))
            {
                TestEqual(TEXT("No legacy or cross-player texture"), Active->GetPathName(), Path(Hand ? TEXT("Hand") : Pitch ? TEXT("Shared") : TEXT("Full")));
                TestEqual(TEXT("Frozen derivative size"), Active->GetImportedSize(), Hand ? FIntPoint(192,128) : Pitch ? FIntPoint(512,768) : FIntPoint(768,1152));
            }
            TestEqual(TEXT("Frozen family surface size"), Widget->GetConfiguredDimensions(), Hand ? FVector2D(220,68) : Pitch ? FVector2D(136,140) : FVector2D(360,540));
            if (Hand) TestNull(TEXT("Hand retains no large texture"), Widget->GetResolvedPortraitTexture());
            else TestNull(TEXT("Large purpose retains no Hand texture"), Widget->GetResolvedHandMicroPortraitTexture());
            const TCHAR* ActiveNode = Hand ? TEXT("HandMicroFaceSafePortrait") : Pitch ? TEXT("PitchMiniPortraitImage") : TEXT("PortraitAssetImage");
            for (const TCHAR* Node : {TEXT("HandMicroFaceSafePortrait"),TEXT("PitchMiniPortraitImage"),TEXT("PortraitAssetImage")})
                if (FString(Node) != ActiveNode)
                    TestNull(TEXT("Inactive purpose brush cleared on rebind"), CastChecked<UImage>(Widget->GetWidgetFromName(Node))->GetBrush().GetResourceObject());
            TestEqual(TEXT("Chinese display name survives migration"), Widget->GetPresentation().IdentityLabel, D->PreferredDisplayName.ToString());
            TestEqual(TEXT("Collection serial is independent"), Widget->GetPresentation().PlayerFacingSerialLabel, D->PlayerFacingSerial);
            if (Pitch)
                TestTrue(TEXT("Pitch number stays hidden"), Widget->GetWidgetFromName(TEXT("AssignedPlayerNumber"))->GetVisibility() == ESlateVisibility::Collapsed);
            if (!Hand && !Pitch)
            {
                TestEqual(TEXT("Fixed biography geometry"), Widget->GetRenderedBiographyRowCount(), 4);
                FString Date = D->Card.BirthDate; Date.ReplaceInline(TEXT("-"), TEXT("."));
                TestEqual(TEXT("Authored date or shared placeholder"), Text(TEXT("BiographyBirthDateValue")), Date.IsEmpty() ? FString(TEXT("—")) : Date);
                if (D->Card.BirthDate.IsEmpty())
                {
                    TestEqual(TEXT("Missing height placeholder"), Text(TEXT("BiographyHeightValue")), FString(TEXT("—")));
                    TestEqual(TEXT("Missing weight placeholder"), Text(TEXT("BiographyWeightValue")), FString(TEXT("—")));
                }
                TestEqual(TEXT("Full number plate uses configured field"), Text(TEXT("FullCardAssignedNumber")), Model.AssignedPlayerNumber);
            }
        }
    }
    for (const TCHAR* Key : {TEXT("Prototype.ManchesterCity.JohnStones")})
    {
        FFMCodexLocalMatchCardView View; View.CardId = FName(Key);
        View.DisplayLabel = FFMCodexPrototypeTeamContent::PlayerDisplayName(View.CardId).ToString();
        TestFalse(TEXT("Missing source remains non-canonical"), FFMCodexPlayerUIAssetReferences::Get().ResolveCardArt(View.CardId).bCanonicalPlayerArt);
        for (const auto Mode : {EMode::HandMicro, EMode::PitchMini, EMode::InteractionChoice})
        {
            Widget->RefreshFromPresentation(FFMCodexLocalMatchUMGPresentationBuilder::BuildCard(View), Mode);
            const TCHAR* Node = Mode == EMode::HandMicro ? TEXT("HandMicroPortraitFallback") : Mode == EMode::PitchMini ? TEXT("PitchMiniPortraitFallback") : TEXT("FullPortraitFallback");
            auto* Fallback = CastChecked<UFMCodexFullCardSurface>(Widget->GetWidgetFromName(Node));
            TestTrue(TEXT("Shared neutral fallback mechanism"), Fallback->GetSurface() == EFMCodexFullCardSurface::MissingPortrait && Fallback->GetVisibility() != ESlateVisibility::Collapsed);
            TestNull(TEXT("No borrowed large portrait"), Widget->GetResolvedPortraitTexture());
            TestNull(TEXT("No borrowed Hand portrait"), Widget->GetResolvedHandMicroPortraitTexture());
            TestEqual(TEXT("Missing art keeps correct name"), Widget->GetPresentation().IdentityLabel, View.DisplayLabel);
        }
    }
    const auto Legacy = FFMCodexPlayerUIAssetReferences::Get().ResolveCardArt(TEXT("Prototype.Arsenal.MikelMerino"));
    TestFalse(TEXT("Non-migrated legacy stays non-canonical"), Legacy.bCanonicalPlayerArt);
    TestTrue(TEXT("Retained legacy resources remain routed"), !Legacy.HandMicroPortrait.IsNull() && !Legacy.FullCardPortrait.IsNull()
        && !Legacy.FullCardPortrait.ToSoftObjectPath().ToString().Contains(TEXT("/Canonical/")));
    Widget->RemoveFromParent(); GEngine->DestroyWorldContext(World); World->DestroyWorld(false);
    return true;
}
#endif
