#include "FMCodexHandMicroDiagnostics.h"

#include "HAL/IConsoleManager.h"

namespace
{
	TAutoConsoleVariable<int32> CVarFMCodexHandMicroFullNameCandidate(
		TEXT("FMCodex.UI.HandMicroFullNameCandidate"),
		1,
		TEXT("Golden Draft geometry: 0=188 legacy comparison, 1=220 preferred full-name Draft."),
		ECVF_Cheat);

	TAutoConsoleVariable<int32> CVarFMCodexHandMicroSharpnessDiagnostic(
		TEXT("FMCodex.UI.HandMicroSharpnessDiagnostic"),
		0,
		TEXT("Development-only Raya portrait comparison: 0=hidden, 1=show A/B/C surface."),
		ECVF_Cheat);

	TAutoConsoleVariable<int32> CVarFMCodexHandMicroSharpnessDiagnosticPage(
		TEXT("FMCodex.UI.HandMicroSharpnessDiagnosticPage"),
		0,
		TEXT("Development-only portrait comparison page: 0=Raya A/B/C, "
			"1=representative six-player B/C, 2=Raya/Saliba/Saka C/D, "
			"3=Odegaard/Donnarumma/Haaland C/D, "
			"4=Raya/Saliba/Saka D1/D2, 5=Odegaard/Donnarumma/Haaland D1/D2, "
			"6=Raya/Saliba D2/D3, 7=Saka/Odegaard D2/D3, "
			"8=Donnarumma/Haaland D2/D3, 9=Name typography, 10=64/68 height."),
		ECVF_Cheat);

	TAutoConsoleVariable<int32> CVarFMCodexHandMicroArtConformanceOverride(
		TEXT("FMCodex.UI.HandMicroArtConformanceOverride"),
		0,
		TEXT("Development-only Hand Micro rack portrait override: "
			"0=current production path, 1=previous D1 art-conformance candidate, "
			"2=rebalanced D2 candidate, 3=Reference-A D3 composition candidate."),
		ECVF_Cheat);

	TAutoConsoleVariable<int32> CVarFMCodexHandMicroUnifiedNameSize(
		TEXT("FMCodex.UI.HandMicroUnifiedNameSize"),
		0,
		TEXT("Development-only Hand Micro name rhythm: 0=existing maximize-to-fit, "
			"1=16px standard then shrink to the existing 12px floor."),
		ECVF_Cheat);

	TAutoConsoleVariable<int32> CVarFMCodexHandMicroHeight68(
		TEXT("FMCodex.UI.HandMicroHeight68"),
		0,
		TEXT("Development-only Hand Micro density candidate: 0=220x64, 1=220x68. "
			"Width and the 96/120/4 horizontal allocation remain unchanged."),
		ECVF_Cheat);
}

bool FMCodexHandMicroDiagnostics::IsFullNameCandidateEnabled()
{
#if UE_BUILD_SHIPPING
	return false;
#else
	return CVarFMCodexHandMicroFullNameCandidate.GetValueOnGameThread() != 0;
#endif
}

bool FMCodexHandMicroDiagnostics::IsUnifiedNameSizeCandidateEnabled()
{
#if UE_BUILD_SHIPPING
	return false;
#else
	return CVarFMCodexHandMicroUnifiedNameSize.GetValueOnGameThread() != 0;
#endif
}

bool FMCodexHandMicroDiagnostics::IsHeight68CandidateEnabled()
{
#if UE_BUILD_SHIPPING
	return false;
#else
	return CVarFMCodexHandMicroHeight68.GetValueOnGameThread() != 0;
#endif
}

float FMCodexHandMicroDiagnostics::GetCardHeight()
{
	return IsHeight68CandidateEnabled()
		? CandidateCardHeight : BaselineCardHeight;
}

bool FMCodexHandMicroDiagnostics::IsSharpnessComparisonEnabled()
{
#if UE_BUILD_SHIPPING
	return false;
#else
	return CVarFMCodexHandMicroSharpnessDiagnostic.GetValueOnGameThread() != 0;
#endif
}

int32 FMCodexHandMicroDiagnostics::GetSharpnessComparisonPage()
{
#if UE_BUILD_SHIPPING
	return 0;
#else
	return FMath::Clamp(
		CVarFMCodexHandMicroSharpnessDiagnosticPage.GetValueOnGameThread(), 0, 10);
#endif
}

bool FMCodexHandMicroDiagnostics::IsArtConformanceOverrideEnabled()
{
#if UE_BUILD_SHIPPING
	return false;
#else
	return GetArtConformanceOverrideMode() != 0;
#endif
}

int32 FMCodexHandMicroDiagnostics::GetArtConformanceOverrideMode()
{
#if UE_BUILD_SHIPPING
	return 0;
#else
	return FMath::Clamp(
		CVarFMCodexHandMicroArtConformanceOverride.GetValueOnGameThread(), 0, 3);
#endif
}

FString FMCodexHandMicroDiagnostics::GetArtConformanceCandidateTexturePath(
	const FName CardId)
{
#if UE_BUILD_SHIPPING
	return FString();
#else
	if (!IsArtConformanceOverrideEnabled())
	{
		return FString();
	}
	const int32 OverrideMode = GetArtConformanceOverrideMode();
	const bool bRebalanced = OverrideMode == 2;
	const bool bReferenceA = OverrideMode == 3;
	const FString Root = bReferenceA
		? TEXT("/Game/Developers/FMCodex/HandMicroReferenceA/")
		: bRebalanced
			? TEXT("/Game/Developers/FMCodex/HandMicroPortraitRebalance/")
			: TEXT("/Game/Developers/FMCodex/HandMicroArtConformance/");
	const FString Variant = bReferenceA
		? TEXT("ReferenceARuntime192")
		: bRebalanced
			? TEXT("RebalancedRuntime192") : TEXT("ArtConformedRuntime192");
	if (CardId == TEXT("Prototype.Arsenal.DavidRaya"))
	{
		return Root + TEXT("T_Prototype_Arsenal_DavidRaya_HandMicro_")
			+ Variant + TEXT(".T_Prototype_Arsenal_DavidRaya_HandMicro_") + Variant;
	}
	if (CardId == TEXT("Prototype.Arsenal.WilliamSaliba"))
	{
		return Root + TEXT("T_Prototype_Arsenal_WilliamSaliba_HandMicro_")
			+ Variant + TEXT(".T_Prototype_Arsenal_WilliamSaliba_HandMicro_") + Variant;
	}
	if (CardId == TEXT("Prototype.Arsenal.BukayoSaka"))
	{
		return Root + TEXT("T_Prototype_Arsenal_BukayoSaka_HandMicro_")
			+ Variant + TEXT(".T_Prototype_Arsenal_BukayoSaka_HandMicro_") + Variant;
	}
	if (CardId == TEXT("Prototype.Arsenal.MartinOdegaard"))
	{
		return Root + TEXT("T_Prototype_Arsenal_MartinOdegaard_HandMicro_")
			+ Variant + TEXT(".T_Prototype_Arsenal_MartinOdegaard_HandMicro_") + Variant;
	}
	if (CardId == TEXT("Prototype.ManchesterCity.GianluigiDonnarumma"))
	{
		return Root + TEXT(
			"T_Prototype_ManchesterCity_GianluigiDonnarumma_HandMicro_")
			+ Variant + TEXT(".T_Prototype_ManchesterCity_GianluigiDonnarumma_HandMicro_")
			+ Variant;
	}
	if (CardId == TEXT("Prototype.ManchesterCity.ErlingHaaland"))
	{
		return Root + TEXT("T_Prototype_ManchesterCity_ErlingHaaland_HandMicro_")
			+ Variant + TEXT(".T_Prototype_ManchesterCity_ErlingHaaland_HandMicro_")
			+ Variant;
	}
	return FString();
#endif
}
