#pragma once
#include "CoreMinimal.h"
#include "FMCodexNetworkDeploymentPayload.generated.h"

class UPackageMap;

/** Canonical identities. Each UTF-8 name is bounded before receive-side allocation. */
USTRUCT(BlueprintType)
struct FMCODEX_API FFMCodexNetworkDeployOrdinaryPayload
{
	GENERATED_BODY()
	static constexpr int32 MaxIdentityUtf8Bytes = 128;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Play")
	FName CardId = NAME_None;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Play")
	FName SlotId = NAME_None;

	bool IsEmpty() const { return CardId.IsNone() && SlotId.IsNone(); }
	bool IsValidShape() const;
	bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess);
};
template<> struct TStructOpsTypeTraits<FFMCodexNetworkDeployOrdinaryPayload>
	: TStructOpsTypeTraitsBase2<FFMCodexNetworkDeployOrdinaryPayload>
{
	enum { WithNetSerializer = true };
};
