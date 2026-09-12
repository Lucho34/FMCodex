#include "FMCodexPlayerCardWidget.h"
#include "FMCodexFullCardSurface.h"
#include "FMCodexPrototypeTeamContent.h"
#include "FMCodexPlayerUIAssetReferences.h"
#include "FMCodexLocalMatchInteractionView.h"
#include "FMCodexLocalMatchUMGPresentation.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexPlayerCardFamilyContractTest,
    "FMCodex.LocalPlay.UI.PlayerCardFamily.GlobalDataAndFallback",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexPlayerCardFamilyContractTest::RunTest(const FString&)
{
    using EMode = EFMCodexPlayerCardPresentationMode;
    FFMCodexLocalMatchCardView Input;
    Input.CardId=TEXT("Prototype.Arsenal.BukayoSaka"); Input.PlayerFacingSerialLabel=TEXT("015");
    auto Model=FFMCodexLocalMatchUMGPresentationBuilder::BuildCard(Input);
    TestEqual(TEXT("Configured default resolves independently of serial"),Model.AssignedPlayerNumber,FString(TEXT("7")));
    Input.AssignedPlayerNumber=TEXT("98");
    Model=FFMCodexLocalMatchUMGPresentationBuilder::BuildCard(Input);
    TestEqual(TEXT("Explicit runtime assignment wins"),Model.AssignedPlayerNumber,FString(TEXT("98")));
    TestEqual(TEXT("Resolution never changes collection serial"),Model.PlayerFacingSerialLabel,FString(TEXT("015")));
    Input.CardId=TEXT("Test.NoDefault");Input.AssignedPlayerNumber.Empty();
    TestTrue(TEXT("No default leaves number empty, never uses serial"),FFMCodexLocalMatchUMGPresentationBuilder::BuildCard(Input).AssignedPlayerNumber.IsEmpty());
    UWorld* World=UWorld::CreateWorld(EWorldType::Game,false);
    if (!TestNotNull(TEXT("Family widget world"),World)) return false;
    GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
    auto* Widget=CreateWidget<UFMCodexPlayerCardWidget>(World);
    if (!TestNotNull(TEXT("Family widget"),Widget)) { GEngine->DestroyWorldContext(World);World->DestroyWorld(false);return false; }
    Widget->TakeWidget();
    auto Text=[&](const TCHAR* Name) { return CastChecked<UTextBlock>(Widget->GetWidgetFromName(Name))->GetText().ToString(); };
    for (const TCHAR* Key : {TEXT("Prototype.Arsenal.BukayoSaka"),TEXT("Prototype.Arsenal.MylesLewisSkelly"),TEXT("Prototype.ManchesterCity.NathanAke")})
    {
        const auto* D=FFMCodexPrototypeTeamContent::Find(FName(Key));
        if (!TestNotNull(TEXT("Source data exists"),D)) continue;
        FFMCodexLocalMatchCardView View;
        View.CardId=D->PlayerKey;View.DisplayLabel=D->PreferredDisplayName.ToString();
        View.BirthDate=D->Card.BirthDate;View.HeightCm=D->Card.HeightCm;View.WeightKg=D->Card.WeightKg;
        View.CompactRoleLabel=TEXT("D");View.PlayerFacingSerialLabel=D->PlayerFacingSerial;
        const auto Card=FFMCodexLocalMatchUMGPresentationBuilder::BuildCard(View);
        Widget->RefreshFromPresentation(Card,EMode::InteractionChoice);
        TestEqual(TEXT("Full always renders four biography rows"),Widget->GetRenderedBiographyRowCount(),4);
        if (View.BirthDate.IsEmpty())
        {
            TestEqual(TEXT("Absent date has neutral placeholder"),Text(TEXT("BiographyBirthDateValue")),FString(TEXT("—")));
            TestEqual(TEXT("Absent height has neutral placeholder"),Text(TEXT("BiographyHeightValue")),FString(TEXT("—")));
            TestEqual(TEXT("Absent weight has neutral placeholder"),Text(TEXT("BiographyWeightValue")),FString(TEXT("—")));
            TestTrue(TEXT("Missing data is not written back into DTO"),Widget->GetPresentation().BirthDate.IsEmpty());
        }
        else
        {
            FString Expected=View.BirthDate;Expected.ReplaceInline(TEXT("-"),TEXT("."));
            TestEqual(TEXT("Known date retains its actual value"),Text(TEXT("BiographyBirthDateValue")),Expected);
        }
        TestEqual(TEXT("Full plate consumes resolved field for canonical and legacy cards"),Text(TEXT("FullCardAssignedNumber")),Card.AssignedPlayerNumber);
        TestTrue(TEXT("Configured Full plate visible"),Widget->GetWidgetFromName(TEXT("FullCardNumberPlateBounds"))->GetVisibility()!=ESlateVisibility::Collapsed);
    }
    FFMCodexLocalMatchCardView Missing;Missing.CardId=TEXT("Prototype.ManchesterCity.NathanAke");
    Missing.DisplayLabel=FFMCodexPrototypeTeamContent::PlayerDisplayName(Missing.CardId).ToString();Missing.CompactRoleLabel=TEXT("D");
    const auto MissingModel=FFMCodexLocalMatchUMGPresentationBuilder::BuildCard(Missing);
    TestFalse(TEXT("Fallback does not activate a fake canonical Master"),FFMCodexPlayerUIAssetReferences::Get().ResolveCardArt(Missing.CardId).bCanonicalPlayerArt);
    for (const auto Mode : {EMode::HandMicro,EMode::PitchMini,EMode::InteractionChoice})
    {
        Widget->RefreshFromPresentation(MissingModel,Mode);
        const TCHAR* Node=Mode==EMode::HandMicro ? TEXT("HandMicroPortraitFallback") : Mode==EMode::PitchMini ? TEXT("PitchMiniPortraitFallback") : TEXT("FullPortraitFallback");
        const auto* Fallback=CastChecked<UFMCodexFullCardSurface>(Widget->GetWidgetFromName(Node));
        TestTrue(TEXT("Same neutral mechanism in all purposes"),Fallback->GetSurface()==EFMCodexFullCardSurface::MissingPortrait);
        TestTrue(TEXT("Missing-art fallback visible"),Fallback->GetVisibility()!=ESlateVisibility::Collapsed);
        TestNull(TEXT("Missing art does not borrow a large portrait"),Widget->GetResolvedPortraitTexture());
        TestNull(TEXT("Missing art does not borrow a Hand portrait"),Widget->GetResolvedHandMicroPortraitTexture());
        TestEqual(TEXT("Fallback keeps correct Chinese identity"),Widget->GetPresentation().IdentityLabel,Missing.DisplayLabel);
        if (Mode==EMode::PitchMini)
        {
            TestTrue(TEXT("Pitch keeps shirt number hidden"),Widget->GetWidgetFromName(TEXT("AssignedPlayerNumber"))->GetVisibility()==ESlateVisibility::Collapsed);
            TestTrue(TEXT("Pitch keeps Full number hidden"),Widget->GetWidgetFromName(TEXT("FullCardNumberPlateBounds"))->GetVisibility()==ESlateVisibility::Collapsed);
        }
    }
    // Rebinding a populated card must remove the fallback and stale purpose brushes.
    Widget->RefreshFromPresentation(Model,EMode::HandMicro);
    TestTrue(TEXT("Real Hand hides silhouette"),Widget->GetWidgetFromName(TEXT("HandMicroPortraitFallback"))->GetVisibility()==ESlateVisibility::Collapsed);
    Widget->RemoveFromParent();GEngine->DestroyWorldContext(World);World->DestroyWorld(false);
    return true;
}
#endif
