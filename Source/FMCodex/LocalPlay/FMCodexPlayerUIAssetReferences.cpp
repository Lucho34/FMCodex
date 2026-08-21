#include "FMCodexPlayerUIAssetReferences.h"

#include "Engine/Texture2D.h"

namespace FMCodexPlayerUIAssetReferences
{
	const FName PilotCardId(TEXT("Demo.A.Outfield.01"));
	const FName PilotArtIdentity(TEXT("Pilot.PlayerCard.01"));
	const FName GoldenSampleCardId(TEXT("Demo.A.Outfield.02"));
	const FName GoldenSampleArtIdentity(TEXT("GoldenSample.PlayerCard.01"));

	const FSoftObjectPath PilotCardFramePath(
		TEXT("/Game/UI/Cards/T_Pilot_CardFrame_01.T_Pilot_CardFrame_01"));
	const FSoftObjectPath PilotPortraitPath(
		TEXT("/Game/UI/Portraits/T_Pilot_PlayerPortrait_01.T_Pilot_PlayerPortrait_01"));
	const FSoftObjectPath GoldenCardFramePath(TEXT(
		"/Game/UI/Cards/GoldenSample/T_Golden_CardFrame_01.T_Golden_CardFrame_01"));
	const FSoftObjectPath GoldenPortraitPath(TEXT(
		"/Game/UI/Portraits/GoldenSample/T_Golden_PlayerPortrait_01.T_Golden_PlayerPortrait_01"));
	const FSoftObjectPath GoldenRoleIconPath(TEXT(
		"/Game/UI/Icons/GoldenSample/T_Golden_Role_Forward_01.T_Golden_Role_Forward_01"));
	const FSoftObjectPath GoldenLongShotSkillIconPath(TEXT(
		"/Game/UI/Icons/GoldenSample/T_Golden_Skill_LongShot_01.T_Golden_Skill_LongShot_01"));
}

const FFMCodexPlayerUIAssetReferences&
FFMCodexPlayerUIAssetReferences::Get()
{
	static const FFMCodexPlayerUIAssetReferences References;
	return References;
}

FFMCodexPlayerUIAssetReferences::FFMCodexPlayerUIAssetReferences()
	: PilotCardFrame(FMCodexPlayerUIAssetReferences::PilotCardFramePath)
	, PilotPortrait(FMCodexPlayerUIAssetReferences::PilotPortraitPath)
	, GoldenCardFrame(FMCodexPlayerUIAssetReferences::GoldenCardFramePath)
	, GoldenPortrait(FMCodexPlayerUIAssetReferences::GoldenPortraitPath)
	, GoldenRoleIcon(FMCodexPlayerUIAssetReferences::GoldenRoleIconPath)
	, GoldenLongShotSkillIcon(
		FMCodexPlayerUIAssetReferences::GoldenLongShotSkillIconPath)
{
	PrototypePortraits.Add(
		TEXT("Prototype.Arsenal.BukayoSaka"),
		TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT(
			"/Game/UI/Portraits/PrototypeTeams/Arsenal/T_Prototype_Arsenal_BukayoSaka_01.T_Prototype_Arsenal_BukayoSaka_01"))));
	PrototypePortraits.Add(
		TEXT("Prototype.Arsenal.MartinOdegaard"),
		TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT(
			"/Game/UI/Portraits/PrototypeTeams/Arsenal/T_Prototype_Arsenal_MartinOdegaard_01.T_Prototype_Arsenal_MartinOdegaard_01"))));
	PrototypePortraits.Add(
		TEXT("Prototype.Arsenal.DeclanRice"),
		TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT(
			"/Game/UI/Portraits/PrototypeTeams/Arsenal/T_Prototype_Arsenal_DeclanRice_01.T_Prototype_Arsenal_DeclanRice_01"))));
	PrototypePortraits.Add(
		TEXT("Prototype.Arsenal.WilliamSaliba"),
		TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT(
			"/Game/UI/Portraits/PrototypeTeams/Arsenal/T_Prototype_Arsenal_WilliamSaliba_01.T_Prototype_Arsenal_WilliamSaliba_01"))));
	PrototypePortraits.Add(
		TEXT("Prototype.Arsenal.DavidRaya"),
		TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT(
			"/Game/UI/Portraits/PrototypeTeams/Arsenal/T_Prototype_Arsenal_DavidRaya_01.T_Prototype_Arsenal_DavidRaya_01"))));
	PrototypePortraits.Add(
		TEXT("Prototype.Arsenal.GabrielMagalhaes"),
		TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT(
			"/Game/UI/Portraits/PrototypeTeams/Arsenal/T_Prototype_Arsenal_GabrielMagalhaes_01.T_Prototype_Arsenal_GabrielMagalhaes_01"))));
	PrototypePortraits.Add(
		TEXT("Prototype.ManchesterCity.ErlingHaaland"),
		TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT(
			"/Game/UI/Portraits/PrototypeTeams/ManchesterCity/T_Prototype_ManchesterCity_ErlingHaaland_01.T_Prototype_ManchesterCity_ErlingHaaland_01"))));
	PrototypePortraits.Add(
		TEXT("Prototype.ManchesterCity.PhilFoden"),
		TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT(
			"/Game/UI/Portraits/PrototypeTeams/ManchesterCity/T_Prototype_ManchesterCity_PhilFoden_01.T_Prototype_ManchesterCity_PhilFoden_01"))));
	PrototypePortraits.Add(
		TEXT("Prototype.ManchesterCity.Rodri"),
		TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT(
			"/Game/UI/Portraits/PrototypeTeams/ManchesterCity/T_Prototype_ManchesterCity_Rodri_01.T_Prototype_ManchesterCity_Rodri_01"))));
	PrototypePortraits.Add(
		TEXT("Prototype.ManchesterCity.RubenDias"),
		TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT(
			"/Game/UI/Portraits/PrototypeTeams/ManchesterCity/T_Prototype_ManchesterCity_RubenDias_01.T_Prototype_ManchesterCity_RubenDias_01"))));
	PrototypePortraits.Add(
		TEXT("Prototype.ManchesterCity.GianluigiDonnarumma"),
		TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT(
			"/Game/UI/Portraits/PrototypeTeams/ManchesterCity/T_Prototype_ManchesterCity_GianluigiDonnarumma_01.T_Prototype_ManchesterCity_GianluigiDonnarumma_01"))));

	PrototypeFullCardPortraits = {
		{ TEXT("Prototype.Arsenal.BukayoSaka"),
			TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT(
				"/Game/UI/Portraits/PrototypeTeams/Arsenal/T_Prototype_Arsenal_BukayoSaka_FullCardPilot_02.T_Prototype_Arsenal_BukayoSaka_FullCardPilot_02"))) },
		{ TEXT("Prototype.Arsenal.DavidRaya"),
			TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT(
				"/Game/UI/Portraits/PrototypeTeams/Arsenal/T_Prototype_Arsenal_DavidRaya_FullCardPilot_02.T_Prototype_Arsenal_DavidRaya_FullCardPilot_02"))) },
		{ TEXT("Prototype.ManchesterCity.Rodri"),
			TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT(
				"/Game/UI/Portraits/PrototypeTeams/ManchesterCity/T_Prototype_ManchesterCity_Rodri_FullCardPilot_02.T_Prototype_ManchesterCity_Rodri_FullCardPilot_02"))) },
		{ TEXT("Prototype.ManchesterCity.GianluigiDonnarumma"),
			TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT(
				"/Game/UI/Portraits/PrototypeTeams/ManchesterCity/T_Prototype_ManchesterCity_GianluigiDonnarumma_FullCardPilot_02.T_Prototype_ManchesterCity_GianluigiDonnarumma_FullCardPilot_02"))) },
		{ TEXT("Prototype.Arsenal.GabrielMartinelli"),
			TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT(
				"/Game/UI/Portraits/PrototypeTeams/Arsenal/T_Prototype_Arsenal_GabrielMartinelli_FullCardHeroBust_01.T_Prototype_Arsenal_GabrielMartinelli_FullCardHeroBust_01"))) },
		{ TEXT("Prototype.Arsenal.GabrielMagalhaes"),
			TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT(
				"/Game/UI/Portraits/PrototypeTeams/Arsenal/T_Prototype_Arsenal_GabrielMagalhaes_FullCardHeroBust_01.T_Prototype_Arsenal_GabrielMagalhaes_FullCardHeroBust_01"))) },
		{ TEXT("Prototype.Arsenal.MikelMerino"),
			TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT(
				"/Game/UI/Portraits/PrototypeTeams/Arsenal/T_Prototype_Arsenal_MikelMerino_FullCardHeroBust_01.T_Prototype_Arsenal_MikelMerino_FullCardHeroBust_01"))) },
		{ TEXT("Prototype.ManchesterCity.JoskoGvardiol"),
			TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT(
				"/Game/UI/Portraits/PrototypeTeams/ManchesterCity/T_Prototype_ManchesterCity_JoskoGvardiol_FullCardHeroBust_01.T_Prototype_ManchesterCity_JoskoGvardiol_FullCardHeroBust_01"))) },
		{ TEXT("Prototype.ManchesterCity.BernardoSilva"),
			TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT(
				"/Game/UI/Portraits/PrototypeTeams/ManchesterCity/T_Prototype_ManchesterCity_BernardoSilva_FullCardHeroBust_01.T_Prototype_ManchesterCity_BernardoSilva_FullCardHeroBust_01"))) },
		{ TEXT("Prototype.ManchesterCity.JeremyDoku"),
			TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT(
				"/Game/UI/Portraits/PrototypeTeams/ManchesterCity/T_Prototype_ManchesterCity_JeremyDoku_FullCardHeroBust_01.T_Prototype_ManchesterCity_JeremyDoku_FullCardHeroBust_01"))) },
		{ TEXT("Prototype.Arsenal.WilliamSaliba"),
			TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT(
				"/Game/UI/Portraits/PrototypeTeams/Arsenal/T_Prototype_Arsenal_WilliamSaliba_FullCardHeroBust_01.T_Prototype_Arsenal_WilliamSaliba_FullCardHeroBust_01"))) },
		{ TEXT("Prototype.Arsenal.MartinOdegaard"),
			TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT(
				"/Game/UI/Portraits/PrototypeTeams/Arsenal/T_Prototype_Arsenal_MartinOdegaard_FullCardHeroBust_01.T_Prototype_Arsenal_MartinOdegaard_FullCardHeroBust_01"))) },
		{ TEXT("Prototype.Arsenal.DeclanRice"),
			TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT(
				"/Game/UI/Portraits/PrototypeTeams/Arsenal/T_Prototype_Arsenal_DeclanRice_FullCardHeroBust_01.T_Prototype_Arsenal_DeclanRice_FullCardHeroBust_01"))) },
		{ TEXT("Prototype.ManchesterCity.ErlingHaaland"),
			TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT(
				"/Game/UI/Portraits/PrototypeTeams/ManchesterCity/T_Prototype_ManchesterCity_ErlingHaaland_FullCardHeroBust_01.T_Prototype_ManchesterCity_ErlingHaaland_FullCardHeroBust_01"))) },
		{ TEXT("Prototype.ManchesterCity.PhilFoden"),
			TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT(
				"/Game/UI/Portraits/PrototypeTeams/ManchesterCity/T_Prototype_ManchesterCity_PhilFoden_FullCardHeroBust_01.T_Prototype_ManchesterCity_PhilFoden_FullCardHeroBust_01"))) },
		{ TEXT("Prototype.ManchesterCity.RubenDias"),
			TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT(
				"/Game/UI/Portraits/PrototypeTeams/ManchesterCity/T_Prototype_ManchesterCity_RubenDias_FullCardHeroBust_01.T_Prototype_ManchesterCity_RubenDias_FullCardHeroBust_01"))) }
	};

	PrototypeHandMicroPortraits = {
		{ TEXT("Prototype.Arsenal.GabrielMartinelli"),
			TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT(
				"/Game/UI/Portraits/PrototypeTeams/HandMicroApprovedRollout/T_Prototype_Arsenal_GabrielMartinelli_HandMicro_ApprovedRuntime192.T_Prototype_Arsenal_GabrielMartinelli_HandMicro_ApprovedRuntime192"))) },
		{ TEXT("Prototype.Arsenal.GabrielMagalhaes"),
			TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT(
				"/Game/UI/Portraits/PrototypeTeams/HandMicroApprovedRollout/T_Prototype_Arsenal_GabrielMagalhaes_HandMicro_ApprovedRuntime192.T_Prototype_Arsenal_GabrielMagalhaes_HandMicro_ApprovedRuntime192"))) },
		{ TEXT("Prototype.Arsenal.MikelMerino"),
			TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT(
				"/Game/UI/Portraits/PrototypeTeams/HandMicroApprovedRollout/T_Prototype_Arsenal_MikelMerino_HandMicro_ApprovedRuntime192.T_Prototype_Arsenal_MikelMerino_HandMicro_ApprovedRuntime192"))) },
		{ TEXT("Prototype.Arsenal.BukayoSaka"),
			TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT(
				"/Game/UI/Portraits/PrototypeTeams/HandMicroApprovedRollout/T_Prototype_Arsenal_BukayoSaka_HandMicro_ApprovedRuntime192.T_Prototype_Arsenal_BukayoSaka_HandMicro_ApprovedRuntime192"))) },
		{ TEXT("Prototype.Arsenal.MartinOdegaard"),
			TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT(
				"/Game/UI/Portraits/PrototypeTeams/HandMicroApprovedRollout/T_Prototype_Arsenal_MartinOdegaard_HandMicro_ApprovedRuntime192.T_Prototype_Arsenal_MartinOdegaard_HandMicro_ApprovedRuntime192"))) },
		{ TEXT("Prototype.Arsenal.DeclanRice"),
			TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT(
				"/Game/UI/Portraits/PrototypeTeams/HandMicroApprovedRollout/T_Prototype_Arsenal_DeclanRice_HandMicro_ApprovedRuntime192.T_Prototype_Arsenal_DeclanRice_HandMicro_ApprovedRuntime192"))) },
		{ TEXT("Prototype.Arsenal.WilliamSaliba"),
			TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT(
				"/Game/UI/Portraits/PrototypeTeams/HandMicroApprovedRollout/T_Prototype_Arsenal_WilliamSaliba_HandMicro_ApprovedRuntime192.T_Prototype_Arsenal_WilliamSaliba_HandMicro_ApprovedRuntime192"))) },
		{ TEXT("Prototype.Arsenal.DavidRaya"),
			TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT(
				"/Game/UI/Portraits/PrototypeTeams/HandMicroApprovedRollout/T_Prototype_Arsenal_DavidRaya_HandMicro_ApprovedRuntime192.T_Prototype_Arsenal_DavidRaya_HandMicro_ApprovedRuntime192"))) },
		{ TEXT("Prototype.ManchesterCity.ErlingHaaland"),
			TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT(
				"/Game/UI/Portraits/PrototypeTeams/HandMicroApprovedRollout/T_Prototype_ManchesterCity_ErlingHaaland_HandMicro_ApprovedRuntime192.T_Prototype_ManchesterCity_ErlingHaaland_HandMicro_ApprovedRuntime192"))) },
		{ TEXT("Prototype.ManchesterCity.PhilFoden"),
			TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT(
				"/Game/UI/Portraits/PrototypeTeams/HandMicroApprovedRollout/T_Prototype_ManchesterCity_PhilFoden_HandMicro_ApprovedRuntime192.T_Prototype_ManchesterCity_PhilFoden_HandMicro_ApprovedRuntime192"))) },
		{ TEXT("Prototype.ManchesterCity.Rodri"),
			TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT(
				"/Game/UI/Portraits/PrototypeTeams/HandMicroApprovedRollout/T_Prototype_ManchesterCity_Rodri_HandMicro_ApprovedRuntime192.T_Prototype_ManchesterCity_Rodri_HandMicro_ApprovedRuntime192"))) },
		{ TEXT("Prototype.ManchesterCity.RubenDias"),
			TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT(
				"/Game/UI/Portraits/PrototypeTeams/HandMicroApprovedRollout/T_Prototype_ManchesterCity_RubenDias_HandMicro_ApprovedRuntime192.T_Prototype_ManchesterCity_RubenDias_HandMicro_ApprovedRuntime192"))) },
		{ TEXT("Prototype.ManchesterCity.GianluigiDonnarumma"),
			TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT(
				"/Game/UI/Portraits/PrototypeTeams/HandMicroApprovedRollout/T_Prototype_ManchesterCity_GianluigiDonnarumma_HandMicro_ApprovedRuntime192.T_Prototype_ManchesterCity_GianluigiDonnarumma_HandMicro_ApprovedRuntime192"))) },
		{ TEXT("Prototype.ManchesterCity.JoskoGvardiol"),
			TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT(
				"/Game/UI/Portraits/PrototypeTeams/HandMicroApprovedRollout/T_Prototype_ManchesterCity_JoskoGvardiol_HandMicro_ApprovedRuntime192.T_Prototype_ManchesterCity_JoskoGvardiol_HandMicro_ApprovedRuntime192"))) },
		{ TEXT("Prototype.ManchesterCity.BernardoSilva"),
			TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT(
				"/Game/UI/Portraits/PrototypeTeams/HandMicroApprovedRollout/T_Prototype_ManchesterCity_BernardoSilva_HandMicro_ApprovedRuntime192.T_Prototype_ManchesterCity_BernardoSilva_HandMicro_ApprovedRuntime192"))) },
		{ TEXT("Prototype.ManchesterCity.JeremyDoku"),
			TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT(
				"/Game/UI/Portraits/PrototypeTeams/HandMicroApprovedRollout/T_Prototype_ManchesterCity_JeremyDoku_HandMicro_ApprovedRuntime192.T_Prototype_ManchesterCity_JeremyDoku_HandMicro_ApprovedRuntime192"))) }
	};

	HandMicroValidationPortraits = {
		{ TEXT("Demo.A.Outfield.01"), TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT(
			"/Game/UI/Portraits/PrototypeTeams/HandMicroApprovedRollout/T_Prototype_Arsenal_GabrielMartinelli_HandMicro_ApprovedRuntime192.T_Prototype_Arsenal_GabrielMartinelli_HandMicro_ApprovedRuntime192"))) },
		{ TEXT("Demo.A.Outfield.02"), TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT(
			"/Game/UI/Portraits/PrototypeTeams/HandMicroApprovedRollout/T_Prototype_Arsenal_GabrielMagalhaes_HandMicro_ApprovedRuntime192.T_Prototype_Arsenal_GabrielMagalhaes_HandMicro_ApprovedRuntime192"))) },
		{ TEXT("Demo.A.Outfield.03"), TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT(
			"/Game/UI/Portraits/PrototypeTeams/HandMicroApprovedRollout/T_Prototype_Arsenal_MikelMerino_HandMicro_ApprovedRuntime192.T_Prototype_Arsenal_MikelMerino_HandMicro_ApprovedRuntime192"))) },
		{ TEXT("Demo.B.Outfield.01"), TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT(
			"/Game/UI/Portraits/PrototypeTeams/HandMicroApprovedRollout/T_Prototype_ManchesterCity_JoskoGvardiol_HandMicro_ApprovedRuntime192.T_Prototype_ManchesterCity_JoskoGvardiol_HandMicro_ApprovedRuntime192"))) },
		{ TEXT("Demo.B.Outfield.02"), TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT(
			"/Game/UI/Portraits/PrototypeTeams/HandMicroApprovedRollout/T_Prototype_ManchesterCity_BernardoSilva_HandMicro_ApprovedRuntime192.T_Prototype_ManchesterCity_BernardoSilva_HandMicro_ApprovedRuntime192"))) },
		{ TEXT("Demo.B.Outfield.03"), TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT(
			"/Game/UI/Portraits/PrototypeTeams/HandMicroApprovedRollout/T_Prototype_ManchesterCity_JeremyDoku_HandMicro_ApprovedRuntime192.T_Prototype_ManchesterCity_JeremyDoku_HandMicro_ApprovedRuntime192"))) }
	};

	// Presentation-only focal alignment. The crop height stays fixed and
	// aspect-ratio safe; only the vertical window moves to preserve the full
	// face and shoulders across the current representative portrait set.
	HandMicroPortraitTops = {
		{ TEXT("Prototype.Arsenal.BukayoSaka"), 0.08f },
		{ TEXT("Prototype.Arsenal.MartinOdegaard"), 0.03f },
		{ TEXT("Prototype.Arsenal.DeclanRice"), 0.06f },
		{ TEXT("Prototype.Arsenal.WilliamSaliba"), 0.04f },
		{ TEXT("Prototype.Arsenal.DavidRaya"), 0.06f },
		{ TEXT("Prototype.ManchesterCity.ErlingHaaland"), 0.04f },
		{ TEXT("Prototype.ManchesterCity.PhilFoden"), 0.03f },
		{ TEXT("Prototype.ManchesterCity.Rodri"), 0.05f },
		{ TEXT("Prototype.ManchesterCity.RubenDias"), 0.05f },
		{ TEXT("Prototype.ManchesterCity.GianluigiDonnarumma"), 0.03f }
	};

	PrototypeForwardRoleCards = {
		TEXT("Prototype.Arsenal.BukayoSaka"),
		TEXT("Prototype.Arsenal.MartinOdegaard"),
		TEXT("Prototype.ManchesterCity.ErlingHaaland"),
		TEXT("Prototype.ManchesterCity.PhilFoden")
	};
}

FFMCodexPlayerUICardArtReferences
FFMCodexPlayerUIAssetReferences::ResolveCardArt(const FName CardId) const
{
	FFMCodexPlayerUICardArtReferences Result;
	const auto ApplyDedicatedHandMicroPortrait =
		[this, CardId](FFMCodexPlayerUICardArtReferences& Target)
		{
			const TSoftObjectPtr<UTexture2D>* HandMicroPortrait =
				PrototypeHandMicroPortraits.Find(CardId);
			if (HandMicroPortrait == nullptr)
			{
				HandMicroPortrait = HandMicroValidationPortraits.Find(CardId);
			}
			if (HandMicroPortrait != nullptr)
			{
				Target.HandMicroPortrait = *HandMicroPortrait;
				Target.HandMicroPortraitTop = 0.0f;
				Target.HandMicroPortraitUVHeight = 1.0f;
			}
		};
	const auto ApplyDedicatedFullCardPortrait =
		[this, CardId](FFMCodexPlayerUICardArtReferences& Target)
		{
			if (const TSoftObjectPtr<UTexture2D>* FullCardPortrait =
					PrototypeFullCardPortraits.Find(CardId))
			{
				Target.FullCardPortrait = *FullCardPortrait;
			}
		};
	if (CardId == GetPilotCardId())
	{
		Result.ArtIdentity = GetPilotArtIdentity();
		Result.CardFrame = PilotCardFrame;
		Result.Portrait = PilotPortrait;
		ApplyDedicatedHandMicroPortrait(Result);
		return Result;
	}
	if (const TSoftObjectPtr<UTexture2D>* PrototypePortrait =
			PrototypePortraits.Find(CardId))
	{
		Result.ArtIdentity = FName(*FString::Printf(
			TEXT("PrototypeTeam.PlayerCard.%s"), *CardId.ToString()));
		Result.CardFrame = GoldenCardFrame;
		Result.Portrait = *PrototypePortrait;
		ApplyDedicatedFullCardPortrait(Result);
		ApplyDedicatedHandMicroPortrait(Result);
		if (const float* PortraitTop = HandMicroPortraitTops.Find(CardId))
		{
			if (Result.HandMicroPortrait.IsNull())
			{
				Result.HandMicroPortraitTop = *PortraitTop;
			}
		}
		Result.LongShotSkillIcon = GoldenLongShotSkillIcon;
		if (PrototypeForwardRoleCards.Contains(CardId))
		{
			Result.RoleIcon = GoldenRoleIcon;
		}
		return Result;
	}
	if (CardId == GetGoldenSampleCardId())
	{
		Result.ArtIdentity = GetGoldenSampleArtIdentity();
		Result.CardFrame = GoldenCardFrame;
		Result.Portrait = GoldenPortrait;
		Result.RoleIcon = GoldenRoleIcon;
		Result.LongShotSkillIcon = GoldenLongShotSkillIcon;
		ApplyDedicatedHandMicroPortrait(Result);
		return Result;
	}
	if (PrototypeHandMicroPortraits.Contains(CardId)
		|| HandMicroValidationPortraits.Contains(CardId))
	{
		Result.ArtIdentity = FName(*FString::Printf(
			TEXT("HandMicroOnly.PlayerCard.%s"), *CardId.ToString()));
		ApplyDedicatedFullCardPortrait(Result);
		ApplyDedicatedHandMicroPortrait(Result);
	}
	return Result;
}

FName FFMCodexPlayerUIAssetReferences::GetPilotCardId() const
{
	return FMCodexPlayerUIAssetReferences::PilotCardId;
}

FName FFMCodexPlayerUIAssetReferences::GetPilotArtIdentity() const
{
	return FMCodexPlayerUIAssetReferences::PilotArtIdentity;
}

FName FFMCodexPlayerUIAssetReferences::GetGoldenSampleCardId() const
{
	return FMCodexPlayerUIAssetReferences::GoldenSampleCardId;
}

FName FFMCodexPlayerUIAssetReferences::GetGoldenSampleArtIdentity() const
{
	return FMCodexPlayerUIAssetReferences::GoldenSampleArtIdentity;
}
