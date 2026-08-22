#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

#include "FMCodexPlayerUIAssetReferences.h"
#include "FMCodexPrototypeTeamContent.h"

#include "Engine/Texture2D.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace FMCodexSharedPortraitGoldenSampleTests
{
	const TArray<FName> FirstBatchPlayerKeys = {
		TEXT("Prototype.Arsenal.DavidRaya"),
		TEXT("Prototype.Arsenal.MylesLewisSkelly"),
		TEXT("Prototype.Arsenal.GabrielMartinelli"),
		TEXT("Prototype.Arsenal.ViktorGyokeres"),
		TEXT("Prototype.ManchesterCity.JoskoGvardiol"),
		TEXT("Prototype.ManchesterCity.BernardoSilva"),
		TEXT("Prototype.ManchesterCity.JeremyDoku"),
		TEXT("Prototype.ManchesterCity.RayanAitNouri")
	};

	const TSet<FName> FirstBatchExistingDedicatedVariants = {
		TEXT("Prototype.Arsenal.DavidRaya"),
		TEXT("Prototype.Arsenal.GabrielMartinelli"),
		TEXT("Prototype.ManchesterCity.JoskoGvardiol"),
		TEXT("Prototype.ManchesterCity.BernardoSilva"),
		TEXT("Prototype.ManchesterCity.JeremyDoku")
	};

	const TArray<FName> SecondBatchPlayerKeys = {
		TEXT("Prototype.Arsenal.MikelMerino"),
		TEXT("Prototype.Arsenal.BenWhite"),
		TEXT("Prototype.Arsenal.PieroHincapie"),
		TEXT("Prototype.Arsenal.JurrienTimber"),
		TEXT("Prototype.Arsenal.RiccardoCalafiori"),
		TEXT("Prototype.Arsenal.EberechiEze"),
		TEXT("Prototype.Arsenal.MartinZubimendi"),
		TEXT("Prototype.Arsenal.ChristianNorgaard"),
		TEXT("Prototype.Arsenal.LeandroTrossard"),
		TEXT("Prototype.Arsenal.NoniMadueke")
	};

	const TSet<FName> SecondBatchExistingDedicatedVariants = {
		TEXT("Prototype.Arsenal.MikelMerino")
	};

	bool HasRuntimeTextureContract(const UTexture2D* Texture)
	{
		return Texture != nullptr
			&& Texture->GetImportedSize() == FIntPoint(512, 768)
			&& Texture->LODGroup == TEXTUREGROUP_UI
			&& Texture->CompressionSettings == TC_BC7
			&& Texture->MipGenSettings == TMGS_Sharpen1
			&& Texture->Filter == TF_Trilinear
			&& Texture->LODBias == 0
			&& Texture->NeverStream
			&& Texture->SRGB;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexSharedPortraitFirstBatchRoutingTest,
	"FMCodex.LocalPlay.SharedPortraitGoldenSample.03.FirstBatchRoutingTextureAndIsolation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexSharedPortraitFirstBatchRoutingTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexSharedPortraitGoldenSampleTests;
	const FFMCodexPlayerUIAssetReferences& References =
		FFMCodexPlayerUIAssetReferences::Get();
	for (const FName PlayerKey : FirstBatchPlayerKeys)
	{
		const FFMCodexPlayerUICardArtReferences Art =
			References.ResolveCardArt(PlayerKey);
		const FString SharedPath = Art.Portrait.ToSoftObjectPath().ToString();
		const FString PlayerSuffix = PlayerKey.ToString().RightChop(
			PlayerKey.ToString().Find(TEXT("."), ESearchCase::CaseSensitive,
				ESearchDir::FromEnd) + 1);
		const FString ExpectedTeam = PlayerKey.ToString().Contains(
			TEXT(".Arsenal.")) ? TEXT("Arsenal") : TEXT("ManchesterCity");
		const FString ExpectedAsset = FString::Printf(
			TEXT("T_Prototype_%s_%s_01"), *ExpectedTeam, *PlayerSuffix);
		TestTrue(TEXT("First-batch Shared Portrait resolves its stable PlayerKey path"),
			SharedPath.Contains(ExpectedAsset)
				&& SharedPath.StartsWith(FString::Printf(
					TEXT("/Game/UI/Portraits/PrototypeTeams/%s/"), *ExpectedTeam)));
		TestTrue(TEXT("First-batch Shared Portrait uses the runtime texture contract"),
			HasRuntimeTextureContract(Art.Portrait.LoadSynchronous()));

		if (FirstBatchExistingDedicatedVariants.Contains(PlayerKey))
		{
			TestTrue(TEXT("Existing Full Card and Hand Micro routes remain isolated"),
				!Art.FullCardPortrait.IsNull()
					&& !Art.HandMicroPortrait.IsNull()
					&& Art.FullCardPortrait.ToSoftObjectPath()
						!= Art.Portrait.ToSoftObjectPath()
					&& Art.HandMicroPortrait.ToSoftObjectPath()
						!= Art.Portrait.ToSoftObjectPath());
		}
		else
		{
			TestTrue(TEXT("Shared-only batch players do not gain cross-Variant routes"),
				Art.FullCardPortrait.IsNull()
					&& Art.HandMicroPortrait.IsNull());
		}
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexSharedPortraitSecondBatchRoutingTest,
	"FMCodex.LocalPlay.SharedPortraitGoldenSample.04.SecondBatchRoutingTextureAndIsolation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexSharedPortraitSecondBatchRoutingTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexSharedPortraitGoldenSampleTests;
	const FFMCodexPlayerUIAssetReferences& References =
		FFMCodexPlayerUIAssetReferences::Get();
	for (const FName PlayerKey : SecondBatchPlayerKeys)
	{
		const FFMCodexPlayerUICardArtReferences Art =
			References.ResolveCardArt(PlayerKey);
		const FString SharedPath = Art.Portrait.ToSoftObjectPath().ToString();
		const FString PlayerSuffix = PlayerKey.ToString().RightChop(
			PlayerKey.ToString().Find(TEXT("."), ESearchCase::CaseSensitive,
				ESearchDir::FromEnd) + 1);
		const FString ExpectedAsset = FString::Printf(
			TEXT("T_Prototype_Arsenal_%s_01"), *PlayerSuffix);
		TestTrue(TEXT("Second-batch Shared Portrait resolves its stable PlayerKey path"),
			SharedPath.Contains(ExpectedAsset)
				&& SharedPath.StartsWith(
					TEXT("/Game/UI/Portraits/PrototypeTeams/Arsenal/")));
		TestTrue(TEXT("Second-batch Shared Portrait uses the runtime texture contract"),
			HasRuntimeTextureContract(Art.Portrait.LoadSynchronous()));

		if (SecondBatchExistingDedicatedVariants.Contains(PlayerKey))
		{
			TestTrue(TEXT("Mikel Merino dedicated variants remain isolated"),
				!Art.FullCardPortrait.IsNull()
					&& !Art.HandMicroPortrait.IsNull()
					&& Art.FullCardPortrait.ToSoftObjectPath()
						!= Art.Portrait.ToSoftObjectPath()
					&& Art.HandMicroPortrait.ToSoftObjectPath()
						!= Art.Portrait.ToSoftObjectPath());
		}
		else
		{
			TestTrue(TEXT("Shared-only second-batch players do not gain cross-Variant routes"),
				Art.FullCardPortrait.IsNull()
					&& Art.HandMicroPortrait.IsNull());
		}
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexSharedPortraitGoldenSampleRoutingTest,
	"FMCodex.LocalPlay.SharedPortraitGoldenSample.01.RoutingTextureAndIsolation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexSharedPortraitGoldenSampleRoutingTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexSharedPortraitGoldenSampleTests;
	const FFMCodexPlayerUIAssetReferences& References =
		FFMCodexPlayerUIAssetReferences::Get();
	const FFMCodexPlayerUICardArtReferences Gabriel = References.ResolveCardArt(
		TEXT("Prototype.Arsenal.GabrielMagalhaes"));
	const FFMCodexPlayerUICardArtReferences Haaland = References.ResolveCardArt(
		TEXT("Prototype.ManchesterCity.ErlingHaaland"));

	const FString GabrielSharedPath =
		Gabriel.Portrait.ToSoftObjectPath().ToString();
	const FString HaalandSharedPath =
		Haaland.Portrait.ToSoftObjectPath().ToString();
	TestEqual(TEXT("Gabriel resolves the stable Shared Portrait path"),
		GabrielSharedPath,
		FString(TEXT("/Game/UI/Portraits/PrototypeTeams/Arsenal/"
			"T_Prototype_Arsenal_GabrielMagalhaes_01."
			"T_Prototype_Arsenal_GabrielMagalhaes_01")));
	TestEqual(TEXT("Haaland retains the stable Shared Portrait path"),
		HaalandSharedPath,
		FString(TEXT("/Game/UI/Portraits/PrototypeTeams/ManchesterCity/"
			"T_Prototype_ManchesterCity_ErlingHaaland_01."
			"T_Prototype_ManchesterCity_ErlingHaaland_01")));

	UTexture2D* GabrielTexture = Gabriel.Portrait.LoadSynchronous();
	UTexture2D* HaalandTexture = Haaland.Portrait.LoadSynchronous();
	TestTrue(TEXT("Gabriel v3 and Haaland v2 use the runtime texture contract"),
		HasRuntimeTextureContract(GabrielTexture)
			&& HasRuntimeTextureContract(HaalandTexture));
	if (GabrielTexture != nullptr && HaalandTexture != nullptr)
	{
		AddInfo(FString::Printf(
			TEXT("Shared runtime editor-resource audit (zero means unavailable before resource initialization): "
				"Gabriel mips=%d memory=%llu; "
				"Haaland mips=%d memory=%llu"),
			GabrielTexture->GetNumMips(),
			static_cast<uint64>(GabrielTexture->CalcTextureMemorySizeEnum(TMC_AllMips)),
			HaalandTexture->GetNumMips(),
			static_cast<uint64>(HaalandTexture->CalcTextureMemorySizeEnum(TMC_AllMips))));
	}

	TestTrue(TEXT("Gabriel dedicated Full Card route remains isolated"),
		Gabriel.FullCardPortrait.ToSoftObjectPath().ToString().Contains(
			TEXT("T_Prototype_Arsenal_GabrielMagalhaes_FullCardHeroBust_01"))
			&& Gabriel.FullCardPortrait.ToSoftObjectPath()
				!= Gabriel.Portrait.ToSoftObjectPath());
	TestTrue(TEXT("Haaland dedicated Full Card route remains isolated"),
		Haaland.FullCardPortrait.ToSoftObjectPath().ToString().Contains(
			TEXT("T_Prototype_ManchesterCity_ErlingHaaland_FullCardHeroBust_01"))
			&& Haaland.FullCardPortrait.ToSoftObjectPath()
				!= Haaland.Portrait.ToSoftObjectPath());
	TestTrue(TEXT("Both Hand Micro and Drag Proxy sources remain isolated"),
		Gabriel.HandMicroPortrait.ToSoftObjectPath().ToString().Contains(
			TEXT("GabrielMagalhaes_HandMicro_ApprovedRuntime192"))
			&& Haaland.HandMicroPortrait.ToSoftObjectPath().ToString().Contains(
				TEXT("ErlingHaaland_HandMicro_ApprovedRuntime192"))
			&& Gabriel.HandMicroPortrait.ToSoftObjectPath()
				!= Gabriel.Portrait.ToSoftObjectPath()
			&& Haaland.HandMicroPortrait.ToSoftObjectPath()
				!= Haaland.Portrait.ToSoftObjectPath());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexSharedPortraitGoldenSampleCoverageAndCookTest,
	"FMCodex.LocalPlay.SharedPortraitGoldenSample.02.CoverageRosterAndCook",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexSharedPortraitGoldenSampleCoverageAndCookTest::RunTest(
	const FString& Parameters)
{
	const TArray<FFMCodexPrototypePlayerDefinition>& Definitions =
		FFMCodexPrototypeTeamContent::GetDefinitions();
	int32 SharedPortraitCount = 0;
	int32 FullCardPortraitCount = 0;
	int32 HandMicroPortraitCount = 0;
	for (const FFMCodexPrototypePlayerDefinition& Definition : Definitions)
	{
		const FFMCodexPlayerUICardArtReferences Art =
			FFMCodexPlayerUIAssetReferences::Get().ResolveCardArt(
				Definition.PlayerKey);
		SharedPortraitCount += Art.Portrait.IsNull() ? 0 : 1;
		FullCardPortraitCount += Art.FullCardPortrait.IsNull() ? 0 : 1;
		HandMicroPortraitCount += Art.HandMicroPortrait.IsNull() ? 0 : 1;
	}
	TestEqual(TEXT("Canonical roster remains exactly 40 players"),
		Definitions.Num(), 40);
	TestEqual(TEXT("Second production batch increases technical Shared coverage to 28"),
		SharedPortraitCount, 28);
	TestEqual(TEXT("Dedicated Full Card coverage remains unchanged"),
		FullCardPortraitCount, 16);
	TestEqual(TEXT("Dedicated Hand Micro coverage remains unchanged"),
		HandMicroPortraitCount, 16);

	FString DefaultGameConfig;
	const FString ConfigPath = FPaths::Combine(
		FPaths::ProjectConfigDir(), TEXT("DefaultGame.ini"));
	TestTrue(TEXT("DefaultGame.ini remains readable"),
		FFileHelper::LoadFileToString(DefaultGameConfig, *ConfigPath));
	TestTrue(TEXT("Shared Portrait packages have an explicit cook contract"),
		DefaultGameConfig.Contains(TEXT(
			"+DirectoriesToAlwaysCook=(Path=\"/Game/UI/Portraits/PrototypeTeams\")")));
	TestFalse(TEXT("ArtSource Masters are not part of the cook contract"),
		DefaultGameConfig.Contains(TEXT("ArtSource")));

	FString ProvenanceSource;
	const FString ProvenancePath = FPaths::Combine(
		FPaths::ProjectDir(),
		TEXT("ContentSource/UI/SharedPortraitRuntime/"
			"SharedPortraitRuntimeProvenance.json"));
	TestTrue(TEXT("Runtime derivative provenance remains readable"),
		FFileHelper::LoadFileToString(ProvenanceSource, *ProvenancePath));
	TestTrue(TEXT("Provenance records Gabriel v3, Haaland v2, and replacement history"),
		ProvenanceSource.Contains(TEXT("Prototype.Arsenal.GabrielMagalhaes"))
			&& ProvenanceSource.Contains(
				TEXT("Prototype.ManchesterCity.ErlingHaaland"))
			&& ProvenanceSource.Contains(TEXT("512"))
			&& ProvenanceSource.Contains(TEXT("768"))
			&& ProvenanceSource.Contains(TEXT(
				"V3 REFINEMENT CANDIDATE IMPORTED"))
			&& ProvenanceSource.Contains(TEXT(
				"V2 CANDIDATE IMPORTED"))
			&& ProvenanceSource.Contains(TEXT(
				"SUPERSEDED BY V3"))
			&& ProvenanceSource.Contains(
				TEXT("VISUAL CONFORMANCE FAIL")));
	for (const FName PlayerKey
		: FMCodexSharedPortraitGoldenSampleTests::FirstBatchPlayerKeys)
	{
		TestTrue(TEXT("Provenance records every first-batch PlayerKey"),
			ProvenanceSource.Contains(PlayerKey.ToString()));
	}
	for (const FName PlayerKey
		: FMCodexSharedPortraitGoldenSampleTests::SecondBatchPlayerKeys)
	{
		TestTrue(TEXT("Provenance records every second-batch PlayerKey"),
			ProvenanceSource.Contains(PlayerKey.ToString()));
	}
	TestTrue(TEXT("Every production-batch record remains pending the manual PIE gate"),
		ProvenanceSource.Contains(TEXT(
			"BATCH CANDIDATE — PENDING MANUAL PIE GATE")));

	FString CardWidgetSource;
	const FString CardWidgetPath = FPaths::Combine(
		FPaths::ProjectDir(),
		TEXT("Source/FMCodex/LocalPlay/FMCodexPlayerCardWidget.cpp"));
	TestTrue(TEXT("Frozen Pitch Mini implementation remains readable"),
		FFileHelper::LoadFileToString(CardWidgetSource, *CardWidgetPath));
	TestTrue(TEXT("Frozen Pitch Mini crop and geometry remain unchanged"),
		CardWidgetSource.Contains(TEXT("PitchMiniInteriorWidth = 130.0f"))
			&& CardWidgetSource.Contains(TEXT("PitchMiniInteriorHeight = 134.0f"))
			&& CardWidgetSource.Contains(TEXT("PitchMiniPortraitHeight = 112.0f"))
			&& CardWidgetSource.Contains(TEXT("PitchMiniIdentityHeight = 22.0f"))
			&& CardWidgetSource.Contains(TEXT("PitchMiniHeroZoom = 1.08f")));
	return true;
}

#endif
