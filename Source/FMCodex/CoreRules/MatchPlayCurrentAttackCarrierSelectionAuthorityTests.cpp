#include "MatchPlayCurrentAttackCarrierSelectionAvailability.h"
#include "MatchPlayCurrentAttackCarrierSelectionWriter.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "HAL/FileManager.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace CarrierSelectionAuthorityTests
{
	FString GetSourceDirectory()
	{
		return FPaths::Combine(
			FPaths::ProjectDir(),
			TEXT("Source/FMCodex/CoreRules"));
	}

	bool LoadSourceFile(
		const TCHAR* FileName,
		FString& OutSource)
	{
		return FFileHelper::LoadFileToString(
			OutSource,
			*FPaths::Combine(GetSourceDirectory(), FileName));
	}

	int32 CountOccurrences(
		const FString& Source,
		const TCHAR* Token)
	{
		int32 Count = 0;
		int32 SearchFrom = 0;
		while (true)
		{
			const int32 FoundAt =
				Source.Find(
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

#define CARRIER_AUTHORITY_TEST(TestClass, TestName) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
		TestClass, \
		"FMCodex.CoreRules.MatchPlayCurrentAttackCarrierSelection.Authority." TestName, \
		EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

CARRIER_AUTHORITY_TEST(
	FCarrierAuthorityLegacyFilesRemovedTest,
	"LegacySynchronizedEntryFilesRemoved")

bool FCarrierAuthorityLegacyFilesRemovedTest::RunTest(
	const FString& Parameters)
{
	const FString Directory =
		CarrierSelectionAuthorityTests::GetSourceDirectory();
	const TArray<FString> LegacyFiles = {
		TEXT("MatchPlayCurrentAttackActionSelectionLegality.h"),
		TEXT("MatchPlayCurrentAttackActionSelectionLegality.cpp"),
		TEXT("MatchPlayCurrentAttackActionSelectionAvailability.h"),
		TEXT("MatchPlayCurrentAttackActionSelectionAvailability.cpp"),
		TEXT("MatchPlayCurrentAttackActionSelectionWriter.h"),
		TEXT("MatchPlayCurrentAttackActionSelectionWriter.cpp")
	};
	for (const FString& LegacyFile : LegacyFiles)
	{
		TestFalse(
			*FString::Printf(
				TEXT("%s no longer exists"),
				*LegacyFile),
			IFileManager::Get().FileExists(
				*FPaths::Combine(Directory, LegacyFile)));
	}
	return true;
}

CARRIER_AUTHORITY_TEST(
	FCarrierAuthorityCallGraphTest,
	"SingleLegalityAndWriterCallGraph")

bool FCarrierAuthorityCallGraphTest::RunTest(
	const FString& Parameters)
{
	FString LegalityHeader;
	FString AvailabilitySource;
	FString WriterHeader;
	FString WriterSource;
	TestTrue(TEXT("Legality header loads"),
		CarrierSelectionAuthorityTests::LoadSourceFile(
			TEXT("MatchPlayCurrentAttackCarrierSelectionLegality.h"),
			LegalityHeader));
	TestTrue(TEXT("Availability source loads"),
		CarrierSelectionAuthorityTests::LoadSourceFile(
			TEXT("MatchPlayCurrentAttackCarrierSelectionAvailability.cpp"),
			AvailabilitySource));
	TestTrue(TEXT("Writer header loads"),
		CarrierSelectionAuthorityTests::LoadSourceFile(
			TEXT("MatchPlayCurrentAttackCarrierSelectionWriter.h"),
			WriterHeader));
	TestTrue(TEXT("Writer source loads"),
		CarrierSelectionAuthorityTests::LoadSourceFile(
			TEXT("MatchPlayCurrentAttackCarrierSelectionWriter.cpp"),
			WriterSource));

	TestEqual(TEXT("One public legality evaluator class"),
		CarrierSelectionAuthorityTests::CountOccurrences(
			LegalityHeader,
			TEXT("FMatchPlayCurrentAttackCarrierSelectionLegalityEvaluator final")),
		1);
	TestEqual(TEXT("One public writer class"),
		CarrierSelectionAuthorityTests::CountOccurrences(
			WriterHeader,
			TEXT("FMatchPlayCurrentAttackCarrierSelectionWriter final")),
		1);
	TestEqual(TEXT("Availability calls sole evaluator twice"),
		CarrierSelectionAuthorityTests::CountOccurrences(
			AvailabilitySource,
			TEXT("FMatchPlayCurrentAttackCarrierSelectionLegalityEvaluator")),
		2);
	TestEqual(TEXT("Writer calls sole evaluator once"),
		CarrierSelectionAuthorityTests::CountOccurrences(
			WriterSource,
			TEXT("FMatchPlayCurrentAttackCarrierSelectionLegalityEvaluator")),
		1);
	TestFalse(TEXT("Writer never writes SelectedAction"),
		WriterSource.Contains(
			TEXT("SelectedAction"),
			ESearchCase::CaseSensitive));
	TestFalse(TEXT("Writer has no CardUsage mutation"),
		WriterSource.Contains(
			TEXT("CardUsage"),
			ESearchCase::CaseSensitive));
	TestFalse(TEXT("Writer has no Skill Rule query"),
		WriterSource.Contains(
			TEXT("SkillRule"),
			ESearchCase::CaseSensitive));
	TestFalse(TEXT("Writer has no ActionPoint check"),
		WriterSource.Contains(
			TEXT("ActionPoint"),
			ESearchCase::CaseSensitive));
	TestFalse(TEXT("Writer has no MatchPlayAttackFlow reference"),
		WriterSource.Contains(
			TEXT("MatchPlayAttackFlow"),
			ESearchCase::CaseSensitive));
	return true;
}

#undef CARRIER_AUTHORITY_TEST

#endif
