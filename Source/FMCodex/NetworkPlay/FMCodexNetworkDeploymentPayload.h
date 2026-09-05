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
/** The player's only goalkeeper choice is a slot. Session derives the unique GK card. */
USTRUCT(BlueprintType)
struct FMCODEX_API FFMCodexNetworkDeployGoalkeeperPayload
{
	GENERATED_BODY()
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Play")
	FName SlotId = NAME_None;
	bool IsEmpty() const { return SlotId.IsNone(); }
	bool IsValidShape() const;
	bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess);
};
/** Carrier choice reuses the bounded identity codec; no Side, slot or display text. */
USTRUCT(BlueprintType)
struct FMCODEX_API FFMCodexNetworkSubmitCarrierPayload
{
	GENERATED_BODY()
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Play")
	FName CarrierCardId = NAME_None;
	bool IsEmpty() const { return CarrierCardId.IsNone(); }
	bool IsValidShape() const;
	bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess);
};
/** Marker choice uses the same bounded identity codec; legality stays in Session. */
USTRUCT(BlueprintType)
struct FMCODEX_API FFMCodexNetworkSubmitMarkerPayload
{
	GENERATED_BODY()
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Play")
	FName MarkerCardId = NAME_None;
	bool IsEmpty() const { return MarkerCardId.IsNone(); }
	bool IsValidShape() const;
	bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess);
};
template<> struct TStructOpsTypeTraits<FFMCodexNetworkSubmitMarkerPayload>
	: TStructOpsTypeTraitsBase2<FFMCodexNetworkSubmitMarkerPayload>
{
	enum { WithNetSerializer = true };
};
/** Runner choice uses the same bounded identity codec; legality stays in Session. */
USTRUCT(BlueprintType)
struct FMCODEX_API FFMCodexNetworkSubmitRunnerPayload
{
	GENERATED_BODY()
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Play")
	FName RunnerCardId = NAME_None;
	bool IsEmpty() const { return RunnerCardId.IsNone(); }
	bool IsValidShape() const;
	bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess);
};
template<> struct TStructOpsTypeTraits<FFMCodexNetworkSubmitRunnerPayload>
	: TStructOpsTypeTraitsBase2<FFMCodexNetworkSubmitRunnerPayload>
{
	enum { WithNetSerializer = true };
};
/** Helper choice uses the same bounded identity codec; legality stays in Session. */
USTRUCT(BlueprintType)
struct FMCODEX_API FFMCodexNetworkSubmitHelperPayload
{
	GENERATED_BODY()
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Play")
	FName HelperCardId = NAME_None;
	bool IsEmpty() const { return HelperCardId.IsNone(); }
	bool IsValidShape() const;
	bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess);
};
template<> struct TStructOpsTypeTraits<FFMCodexNetworkSubmitHelperPayload>
	: TStructOpsTypeTraitsBase2<FFMCodexNetworkSubmitHelperPayload>
{
	enum { WithNetSerializer = true };
};
template<> struct TStructOpsTypeTraits<FFMCodexNetworkSubmitCarrierPayload>
	: TStructOpsTypeTraitsBase2<FFMCodexNetworkSubmitCarrierPayload>
{
	enum { WithNetSerializer = true };
};
template<> struct TStructOpsTypeTraits<FFMCodexNetworkDeployGoalkeeperPayload>
	: TStructOpsTypeTraitsBase2<FFMCodexNetworkDeployGoalkeeperPayload>
{
	enum { WithNetSerializer = true };
};
template<> struct TStructOpsTypeTraits<FFMCodexNetworkDeployOrdinaryPayload>
	: TStructOpsTypeTraitsBase2<FFMCodexNetworkDeployOrdinaryPayload>
{
	enum { WithNetSerializer = true };
};
