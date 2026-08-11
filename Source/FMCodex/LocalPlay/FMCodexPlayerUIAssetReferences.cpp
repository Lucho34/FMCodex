#include "FMCodexPlayerUIAssetReferences.h"

#include "Engine/Texture2D.h"

namespace FMCodexPlayerUIAssetReferences
{
	const FName PilotCardId(TEXT("Demo.A.Outfield.01"));
	const FName PilotArtIdentity(TEXT("Pilot.PlayerCard.01"));

	const FSoftObjectPath PilotCardFramePath(
		TEXT("/Game/UI/Cards/T_Pilot_CardFrame_01.T_Pilot_CardFrame_01"));
	const FSoftObjectPath PilotPortraitPath(
		TEXT("/Game/UI/Portraits/T_Pilot_PlayerPortrait_01.T_Pilot_PlayerPortrait_01"));
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
{
}

FFMCodexPlayerUICardArtReferences
FFMCodexPlayerUIAssetReferences::ResolveCardArt(const FName CardId) const
{
	FFMCodexPlayerUICardArtReferences Result;
	if (CardId != GetPilotCardId())
	{
		return Result;
	}

	Result.ArtIdentity = GetPilotArtIdentity();
	Result.CardFrame = PilotCardFrame;
	Result.Portrait = PilotPortrait;
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
