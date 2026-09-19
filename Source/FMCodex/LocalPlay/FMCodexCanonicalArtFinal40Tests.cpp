#include "FMCodexPlayerCardWidget.h"
#include "FMCodexPlayerUIAssetReferences.h"
#include "FMCodexPrototypeTeamContent.h"
#include "FMCodexLocalMatchInteractionView.h"
#include "FMCodexLocalMatchUMGPresentation.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/Texture2D.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexCanonicalArtFinal40Test,
    "FMCodex.LocalPlay.UI.CanonicalArtFinal40.RoutesAndRebinding",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexCanonicalArtFinal40Test::RunTest(const FString&)
{
    using EMode = EFMCodexPlayerCardPresentationMode;
    UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
    if (!TestNotNull(TEXT("Final 40 world"), World)) return false;
    GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
    auto* Widget = CreateWidget<UFMCodexPlayerCardWidget>(World);
    if (!TestNotNull(TEXT("Final 40 widget"), Widget))
    { GEngine->DestroyWorldContext(World); World->DestroyWorld(false); return false; }
    Widget->TakeWidget();
    const TCHAR* Keys[] = {
        TEXT("Prototype.Arsenal.BukayoSaka"),
        TEXT("Prototype.Arsenal.MartinOdegaard"),
        TEXT("Prototype.Arsenal.DeclanRice"),
        TEXT("Prototype.Arsenal.WilliamSaliba"),
        TEXT("Prototype.Arsenal.DavidRaya"),
        TEXT("Prototype.Arsenal.MylesLewisSkelly"),
        TEXT("Prototype.Arsenal.GabrielMartinelli"),
        TEXT("Prototype.Arsenal.ViktorGyokeres"),
        TEXT("Prototype.Arsenal.GabrielMagalhaes"),
        TEXT("Prototype.Arsenal.MikelMerino"),
        TEXT("Prototype.Arsenal.BenWhite"),
        TEXT("Prototype.Arsenal.PieroHincapie"),
        TEXT("Prototype.Arsenal.JurrienTimber"),
        TEXT("Prototype.Arsenal.RiccardoCalafiori"),
        TEXT("Prototype.Arsenal.EberechiEze"),
        TEXT("Prototype.Arsenal.MartinZubimendi"),
        TEXT("Prototype.Arsenal.ChristianNorgaard"),
        TEXT("Prototype.Arsenal.LeandroTrossard"),
        TEXT("Prototype.Arsenal.NoniMadueke"),
        TEXT("Prototype.ManchesterCity.ErlingHaaland"),
        TEXT("Prototype.ManchesterCity.JoskoGvardiol"),
        TEXT("Prototype.ManchesterCity.BernardoSilva"),
        TEXT("Prototype.ManchesterCity.JeremyDoku"),
        TEXT("Prototype.ManchesterCity.RayanAitNouri"),
        TEXT("Prototype.ManchesterCity.PhilFoden"),
        TEXT("Prototype.ManchesterCity.Rodri"),
        TEXT("Prototype.ManchesterCity.RubenDias"),
        TEXT("Prototype.ManchesterCity.GianluigiDonnarumma"),
        TEXT("Prototype.ManchesterCity.NathanAke"),
        TEXT("Prototype.ManchesterCity.RayanCherki"),
        TEXT("Prototype.Arsenal.KaiHavertz"),
        TEXT("Prototype.ManchesterCity.MarcGuehi"),
        TEXT("Prototype.ManchesterCity.OmarMarmoush"),
        TEXT("Prototype.ManchesterCity.TijjaniReijnders"),
        TEXT("Prototype.ManchesterCity.JohnStones"),
        TEXT("Prototype.ManchesterCity.NicoGonzalez"),
        TEXT("Prototype.ManchesterCity.MatheusNunes"),
        TEXT("Prototype.ManchesterCity.MateoKovacic"),
        TEXT("Prototype.ManchesterCity.AntoineSemenyo"),
        TEXT("Prototype.ManchesterCity.Savinho")};
    for (const TCHAR* Key : Keys)
    {
        const FName Id(Key);
        const auto* Definition = FFMCodexPrototypeTeamContent::Find(Id);
        if (!TestNotNull(TEXT("Final 40 roster identity"), Definition)) continue;
        FFMCodexLocalMatchCardView View;
        View.CardId = Id; View.DisplayLabel = Definition->PreferredDisplayName.ToString();
        const auto Model = FFMCodexLocalMatchUMGPresentationBuilder::BuildCard(View);
        const auto Art = FFMCodexPlayerUIAssetReferences::Get().ResolveCardArt(Id);
        TestTrue(TEXT("Final 40 explicitly canonical"), Art.bCanonicalPlayerArt);
        const FString Token = FString(Key).Replace(TEXT("."), TEXT("_"));
        for (const auto Mode : {EMode::HandMicro, EMode::PitchMini, EMode::InteractionChoice})
        {
            const bool Hand = Mode == EMode::HandMicro, Pitch = Mode == EMode::PitchMini;
            const TCHAR* Role = Hand ? TEXT("Hand") : Pitch ? TEXT("Shared") : TEXT("Full");
            const FString Name = FString::Printf(TEXT("T_%s_%s"), *Token, Role);
            const FString Expected = FString::Printf(TEXT("/Game/UI/Portraits/PrototypeTeams/Canonical/%s/%s.%s"), *Token, *Name, *Name);
            Widget->RefreshFromPresentation(Model, Mode);
            auto* Texture = Hand ? Widget->GetResolvedHandMicroPortraitTexture() : Widget->GetResolvedPortraitTexture();
            if (TestNotNull(TEXT("New requested texture loads"), Texture))
            {
                TestEqual(TEXT("Exact identity and purpose"), Texture->GetPathName(), Expected);
                TestEqual(TEXT("Derivative dimensions"), Texture->GetImportedSize(), Hand ? FIntPoint(192,128) : Pitch ? FIntPoint(512,768) : FIntPoint(768,1152));
            }
            if (Hand) TestNull(TEXT("Hand does not load large art"), Widget->GetResolvedPortraitTexture());
            else TestNull(TEXT("Large purpose clears Hand art"), Widget->GetResolvedHandMicroPortraitTexture());

            FFMCodexLocalMatchCardView Missing;
            Missing.CardId = TEXT("Test.FuturePlayer");
            Missing.DisplayLabel = FString(TEXT("未来球员"));
            TestFalse(TEXT("Future fixture remains non-canonical"), FFMCodexPlayerUIAssetReferences::Get().ResolveCardArt(Missing.CardId).bCanonicalPlayerArt);
            Widget->RefreshFromPresentation(FFMCodexLocalMatchUMGPresentationBuilder::BuildCard(Missing), Mode);
            TestNull(TEXT("Fallback clears previous large art"), Widget->GetResolvedPortraitTexture());
            TestNull(TEXT("Fallback clears previous Hand art"), Widget->GetResolvedHandMicroPortraitTexture());
            Widget->RefreshFromPresentation(Model, Mode);
            TestNotNull(TEXT("Canonical rebind after fallback"), Hand ? Widget->GetResolvedHandMicroPortraitTexture() : Widget->GetResolvedPortraitTexture());
        }
    }
    Widget->RemoveFromParent(); GEngine->DestroyWorldContext(World); World->DestroyWorld(false);
    return true;
}
#endif
