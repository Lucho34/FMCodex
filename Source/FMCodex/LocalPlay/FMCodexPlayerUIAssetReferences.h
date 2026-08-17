#pragma once

#include "CoreMinimal.h"
#include "UObject/SoftObjectPtr.h"

class UTexture2D;

struct FFMCodexPlayerUICardArtReferences
{
	FName ArtIdentity = NAME_None;
	TSoftObjectPtr<UTexture2D> CardFrame;
	TSoftObjectPtr<UTexture2D> Portrait;
	/** Optional vertical portrait override used only by the in-match Full Card. */
	TSoftObjectPtr<UTexture2D> FullCardPortrait;
	/** Dedicated horizontal portrait variant used only by Hand Micro. */
	TSoftObjectPtr<UTexture2D> HandMicroPortrait;
	TSoftObjectPtr<UTexture2D> RoleIcon;
	TSoftObjectPtr<UTexture2D> LongShotSkillIcon;
	/** Presentation-only top edge for the Hand Micro portrait UV window. */
	float HandMicroPortraitTop = 0.06f;
	/** Presentation-only Hand Micro crop height; width remains the full asset. */
	float HandMicroPortraitUVHeight = 0.426667f;
};

/**
 * Presentation-only pilot catalog. Gameplay snapshots and authoritative
 * sessions deliberately know nothing about these cosmetic references.
 */
class FMCODEX_API FFMCodexPlayerUIAssetReferences final
{
public:
	static const FFMCodexPlayerUIAssetReferences& Get();

	FFMCodexPlayerUICardArtReferences ResolveCardArt(FName CardId) const;
	FName GetPilotCardId() const;
	FName GetPilotArtIdentity() const;
	FName GetGoldenSampleCardId() const;
	FName GetGoldenSampleArtIdentity() const;

private:
	FFMCodexPlayerUIAssetReferences();

	TSoftObjectPtr<UTexture2D> PilotCardFrame;
	TSoftObjectPtr<UTexture2D> PilotPortrait;
	TSoftObjectPtr<UTexture2D> GoldenCardFrame;
	TSoftObjectPtr<UTexture2D> GoldenPortrait;
	TSoftObjectPtr<UTexture2D> GoldenRoleIcon;
	TSoftObjectPtr<UTexture2D> GoldenLongShotSkillIcon;
	TMap<FName, TSoftObjectPtr<UTexture2D>> PrototypePortraits;
	TMap<FName, TSoftObjectPtr<UTexture2D>> PrototypeFullCardPortraits;
	TMap<FName, TSoftObjectPtr<UTexture2D>> PrototypeHandMicroPortraits;
	TMap<FName, TSoftObjectPtr<UTexture2D>> HandMicroValidationPortraits;
	TMap<FName, float> HandMicroPortraitTops;
	TSet<FName> PrototypeForwardRoleCards;
};
