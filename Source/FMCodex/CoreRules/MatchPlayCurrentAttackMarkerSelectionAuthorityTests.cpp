#include "MatchPlayCurrentAttackMarkerSelectionAvailability.h"
#include "MatchPlayCurrentAttackMarkerSelectionWriter.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace MarkerSelectionAuthorityTests
{
	FString Load(const TCHAR* FileName)
	{
		FString Source;
		FFileHelper::LoadFileToString(
			Source,
			*FPaths::Combine(
				FPaths::ProjectDir(),
				TEXT("Source/FMCodex/CoreRules"),
				FileName));
		return Source;
	}

	int32 CountOccurrences(
		const FString& Source,
		const TCHAR* Token)
	{
		int32 Count = 0;
		int32 SearchFrom = 0;
		while (true)
		{
			const int32 FoundAt = Source.Find(
				Token,
				ESearchCase::CaseSensitive,
				ESearchDir::FromStart,
				SearchFrom);
			if (FoundAt == INDEX_NONE)
			{
				return Count;
			}
			++Count;
			SearchFrom = FoundAt + FCString::Strlen(Token);
		}
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMarkerSelectionAuthorityCallGraphTest,
	"FMCodex.CoreRules.MatchPlayCurrentAttackMarkerSelection.Authority.SingleAuthorityCallGraph",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMarkerSelectionAuthorityCallGraphTest::RunTest(
	const FString& Parameters)
{
	using namespace MarkerSelectionAuthorityTests;
	const FString LegalityHeader =
		Load(TEXT("MatchPlayCurrentAttackMarkerSelectionLegality.h"));
	const FString LegalitySource =
		Load(TEXT("MatchPlayCurrentAttackMarkerSelectionLegality.cpp"));
	const FString AvailabilityHeader =
		Load(TEXT("MatchPlayCurrentAttackMarkerSelectionAvailability.h"));
	const FString AvailabilitySource =
		Load(TEXT("MatchPlayCurrentAttackMarkerSelectionAvailability.cpp"));
	const FString WriterHeader =
		Load(TEXT("MatchPlayCurrentAttackMarkerSelectionWriter.h"));
	const FString WriterSource =
		Load(TEXT("MatchPlayCurrentAttackMarkerSelectionWriter.cpp"));
	const FString ProductionSources =
		LegalitySource + AvailabilitySource + WriterSource;

	TestEqual(TEXT("One public legality authority"),
		CountOccurrences(
			LegalityHeader,
			TEXT("FMatchPlayCurrentAttackMarkerSelectionLegalityEvaluator final")),
		1);
	TestEqual(TEXT("One public availability authority"),
		CountOccurrences(
			AvailabilityHeader,
			TEXT("FMatchPlayCurrentAttackMarkerSelectionAvailability final")),
		1);
	TestEqual(TEXT("One public writer authority"),
		CountOccurrences(
			WriterHeader,
			TEXT("FMatchPlayCurrentAttackMarkerSelectionWriter final")),
		1);
	TestEqual(TEXT("Availability reuses legality at two call sites"),
		CountOccurrences(
			AvailabilitySource,
			TEXT("FMatchPlayCurrentAttackMarkerSelectionLegalityEvaluator")),
		2);
	TestEqual(TEXT("Writer calls legality exactly once"),
		CountOccurrences(
			WriterSource,
			TEXT("FMatchPlayCurrentAttackMarkerSelectionLegalityEvaluator")),
		1);
	TestEqual(TEXT("Marker module calls physical authority once"),
		CountOccurrences(
			ProductionSources,
			TEXT("FMatchPlayDeploymentPhysicalAreaMatchQuery::Query")),
		1);
	TestFalse(TEXT("Marker modules do not compare NeutralSide"),
		ProductionSources.Contains(TEXT("NeutralSide")));
	TestFalse(TEXT("Marker modules do not compare RelativeZone"),
		ProductionSources.Contains(TEXT("RelativeZone")));
	TestFalse(TEXT("Marker modules do not query SkillRule"),
		ProductionSources.Contains(TEXT("SkillRule")));
	TestFalse(TEXT("Marker modules do not query ActionPoint"),
		ProductionSources.Contains(TEXT("ActionPoint")));
	TestFalse(TEXT("Marker modules do not use Formula"),
		ProductionSources.Contains(TEXT("Formula")));
	TestFalse(TEXT("Marker modules do not use D6"),
		ProductionSources.Contains(TEXT("D6")));
	TestFalse(TEXT("Marker modules do not write SelectedAction"),
		WriterSource.Contains(TEXT("SelectedAction")));
	TestFalse(TEXT("Marker modules do not write CardUsage"),
		ProductionSources.Contains(TEXT("CardUsage")));
	TestFalse(TEXT("Marker modules do not use MatchPlayAttackFlow"),
		ProductionSources.Contains(TEXT("MatchPlayAttackFlow")));
	return true;
}

#endif
