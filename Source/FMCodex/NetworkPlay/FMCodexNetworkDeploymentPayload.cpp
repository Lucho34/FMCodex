#include "FMCodexNetworkDeploymentPayload.h"
#include "Containers/StringConv.h"
#include "Serialization/Archive.h"

namespace FMCodexNetworkDeploymentPayload
{
	bool BoundedName(const FName Name)
	{
		const FTCHARToUTF8 Utf8(*Name.ToString());
		return !Name.IsNone() && Utf8.Length() <= FFMCodexNetworkDeployOrdinaryPayload::MaxIdentityUtf8Bytes;
	}
	bool SerializeName(FArchive& Ar, FName& Name)
	{
		constexpr int32 Limit = FFMCodexNetworkDeployOrdinaryPayload::MaxIdentityUtf8Bytes;
		uint8 Length = 0;
		ANSICHAR Bytes[Limit] = {};
		if (Ar.IsSaving() && !Name.IsNone())
		{
			const FTCHARToUTF8 Utf8(*Name.ToString());
			if (Utf8.Length() > Limit) { Ar.SetError(); return false; }
			Length = static_cast<uint8>(Utf8.Length());
			FMemory::Memcpy(Bytes, Utf8.Get(), Length);
		}
		Ar << Length;
		if (Ar.IsError() || Length > Limit) { Ar.SetError(); return false; }
		Ar.Serialize(Bytes, Length);
		if (Ar.IsError()) { return false; }
		if (Ar.IsLoading())
		{
			if (Length == 0) { Name = NAME_None; return true; }
			for (int32 I = 0; I < Length; ++I)
			{
				if (Bytes[I] == 0) { Ar.SetError(); return false; }
			}
			const FUTF8ToTCHAR Decoded(Bytes, Length);
			const FString Text(Decoded.Length(), Decoded.Get());
			const FTCHARToUTF8 RoundTrip(*Text);
			if (RoundTrip.Length() != Length || FMemory::Memcmp(RoundTrip.Get(), Bytes, Length) != 0)
			{
				Ar.SetError(); return false;
			}
			Name = FName(*Text);
		}
		return true;
	}
}
bool FFMCodexNetworkSubmitMarkerPayload::IsValidShape() const
{
	return FMCodexNetworkDeploymentPayload::BoundedName(MarkerCardId);
}
bool FFMCodexNetworkSubmitMarkerPayload::NetSerialize(FArchive& Ar, UPackageMap*, bool& bOutSuccess)
{
	bOutSuccess = FMCodexNetworkDeploymentPayload::SerializeName(Ar, MarkerCardId);
	return true;
}
bool FFMCodexNetworkSubmitRunnerPayload::IsValidShape() const
{
	return FMCodexNetworkDeploymentPayload::BoundedName(RunnerCardId);
}
bool FFMCodexNetworkSubmitRunnerPayload::NetSerialize(FArchive& Ar, UPackageMap*, bool& bOutSuccess)
{
	bOutSuccess = FMCodexNetworkDeploymentPayload::SerializeName(Ar, RunnerCardId);
	return true;
}
bool FFMCodexNetworkSubmitHelperPayload::IsValidShape() const
{
	return FMCodexNetworkDeploymentPayload::BoundedName(HelperCardId);
}
bool FFMCodexNetworkSubmitHelperPayload::NetSerialize(FArchive& Ar, UPackageMap*, bool& bOutSuccess)
{
	bOutSuccess = FMCodexNetworkDeploymentPayload::SerializeName(Ar, HelperCardId);
	return true;
}
bool FFMCodexNetworkSubmitSkillPayload::IsValidShape() const
{
	return FMCodexNetworkDeploymentPayload::BoundedName(SkillId);
}
bool FFMCodexNetworkSubmitSkillPayload::NetSerialize(FArchive& Ar, UPackageMap*, bool& bOutSuccess)
{
	bOutSuccess = FMCodexNetworkDeploymentPayload::SerializeName(Ar, SkillId);
	return true;
}
bool FFMCodexNetworkSubmitCarrierPayload::IsValidShape() const
{
	return FMCodexNetworkDeploymentPayload::BoundedName(CarrierCardId);
}
bool FFMCodexNetworkSubmitCarrierPayload::NetSerialize(FArchive& Ar, UPackageMap*, bool& bOutSuccess)
{
	bOutSuccess = FMCodexNetworkDeploymentPayload::SerializeName(Ar, CarrierCardId);
	return true;
}
bool FFMCodexNetworkDeployGoalkeeperPayload::IsValidShape() const
{
	return FMCodexNetworkDeploymentPayload::BoundedName(SlotId);
}
bool FFMCodexNetworkDeployGoalkeeperPayload::NetSerialize(FArchive& Ar, UPackageMap*, bool& bOutSuccess)
{
	bOutSuccess = FMCodexNetworkDeploymentPayload::SerializeName(Ar, SlotId);
	return true;
}
bool FFMCodexNetworkDeployOrdinaryPayload::IsValidShape() const
{
	return FMCodexNetworkDeploymentPayload::BoundedName(CardId)
		&& FMCodexNetworkDeploymentPayload::BoundedName(SlotId);
}
bool FFMCodexNetworkDeployOrdinaryPayload::NetSerialize(FArchive& Ar, UPackageMap*, bool& bOutSuccess)
{
	bOutSuccess = FMCodexNetworkDeploymentPayload::SerializeName(Ar, CardId)
		&& FMCodexNetworkDeploymentPayload::SerializeName(Ar, SlotId);
	return true;
}
