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
	if (CardId == GetPilotCardId())
	{
		Result.ArtIdentity = GetPilotArtIdentity();
		Result.CardFrame = PilotCardFrame;
		Result.Portrait = PilotPortrait;
		return Result;
	}
	if (const TSoftObjectPtr<UTexture2D>* PrototypePortrait =
			PrototypePortraits.Find(CardId))
	{
		Result.ArtIdentity = FName(*FString::Printf(
			TEXT("PrototypeTeam.PlayerCard.%s"), *CardId.ToString()));
		Result.CardFrame = GoldenCardFrame;
		Result.Portrait = *PrototypePortrait;
		Result.LongShotSkillIcon = GoldenLongShotSkillIcon;
		if (PrototypeForwardRoleCards.Contains(CardId))
		{
			Result.RoleIcon = GoldenRoleIcon;
		}
		return Result;
	}
	if (CardId != GetGoldenSampleCardId())
	{
		return Result;
	}

	Result.ArtIdentity = GetGoldenSampleArtIdentity();
	Result.CardFrame = GoldenCardFrame;
	Result.Portrait = GoldenPortrait;
	Result.RoleIcon = GoldenRoleIcon;
	Result.LongShotSkillIcon = GoldenLongShotSkillIcon;
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
