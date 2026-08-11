#pragma once

#include "CoreMinimal.h"
#include "UObject/SoftObjectPtr.h"

class UTexture2D;

struct FFMCodexPlayerUICardArtReferences
{
	FName ArtIdentity = NAME_None;
	TSoftObjectPtr<UTexture2D> CardFrame;
	TSoftObjectPtr<UTexture2D> Portrait;
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

private:
	FFMCodexPlayerUIAssetReferences();

	TSoftObjectPtr<UTexture2D> PilotCardFrame;
	TSoftObjectPtr<UTexture2D> PilotPortrait;
};
