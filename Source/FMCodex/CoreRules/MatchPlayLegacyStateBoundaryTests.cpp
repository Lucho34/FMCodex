#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace MatchPlayLegacyStateBoundaryTests
{
	bool LoadCoreRulesFile(
		const TCHAR* RelativePath,
		FString& OutSource)
	{
		return FFileHelper::LoadFileToString(
			OutSource,
			*(FPaths::ProjectDir()
				/ TEXT("Source/FMCodex/CoreRules")
				/ RelativePath));
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayExternalHeadersUseCurrentStateTest,
	"FMCodex.CoreRules.MatchPlayLegacyStateBoundary.ExternalApiV1HeadersUseMatchPlayState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayExternalHeadersUseCurrentStateTest::RunTest(
	const FString& Parameters)
{
	const TArray<FString> HeaderNames =
	{
		TEXT("MatchPlayExternalMatchSetupView.h"),
		TEXT("MatchPlayExternalStateView.h"),
		TEXT("MatchPlayExternalAttackRequestPreflight.h"),
		TEXT("MatchPlayExternalTurnController.h")
	};

	for (const FString& HeaderName : HeaderNames)
	{
		FString Source;
		const bool bLoaded =
			MatchPlayLegacyStateBoundaryTests::LoadCoreRulesFile(
				*HeaderName,
				Source);
		TestTrue(
			*FString::Printf(TEXT("%s can be loaded"), *HeaderName),
			bLoaded);
		if (!bLoaded)
		{
			continue;
		}

		TestTrue(
			*FString::Printf(
				TEXT("%s uses FMatchPlayState"),
				*HeaderName),
			Source.Contains(TEXT("FMatchPlayState")));
		TestFalse(
			*FString::Printf(
				TEXT("%s does not use legacy FMatchState"),
				*HeaderName),
			Source.Contains(TEXT("FMatchState")));
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayLifecycleUsesCurrentStateTest,
	"FMCodex.CoreRules.MatchPlayLegacyStateBoundary.LifecycleRegressionUsesMatchPlayState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayLifecycleUsesCurrentStateTest::RunTest(
	const FString& Parameters)
{
	FString Source;
	const bool bLoaded =
		MatchPlayLegacyStateBoundaryTests::LoadCoreRulesFile(
			TEXT("MatchPlayExternalApiV1LifecycleTests.cpp"),
			Source);

	TestTrue(TEXT("Lifecycle regression source can be loaded"), bLoaded);
	TestTrue(
		TEXT("Lifecycle regression uses FMatchPlayState"),
		Source.Contains(TEXT("FMatchPlayState")));
	TestFalse(
		TEXT("Lifecycle regression does not use legacy FMatchState"),
		Source.Contains(TEXT("FMatchState")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayActiveMatchLogTypeTest,
	"FMCodex.CoreRules.MatchPlayLegacyStateBoundary.MatchLogEntryRemainsActive",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayActiveMatchLogTypeTest::RunTest(
	const FString& Parameters)
{
	FString StateTypesSource;
	FString FormulaSource;
	const bool bStateTypesLoaded =
		MatchPlayLegacyStateBoundaryTests::LoadCoreRulesFile(
			TEXT("MatchStateTypes.h"),
			StateTypesSource);
	const bool bFormulaLoaded =
		MatchPlayLegacyStateBoundaryTests::LoadCoreRulesFile(
			TEXT("FormulaResolver.h"),
			FormulaSource);

	TestTrue(TEXT("MatchStateTypes can be loaded"), bStateTypesLoaded);
	TestTrue(TEXT("FormulaResolver header can be loaded"), bFormulaLoaded);
	TestTrue(
		TEXT("FMatchLogEntry remains defined"),
		StateTypesSource.Contains(
			TEXT("struct FMCODEX_API FMatchLogEntry")));
	TestTrue(
		TEXT("FormulaResolver still exposes FMatchLogEntry"),
		FormulaSource.Contains(TEXT("FMatchLogEntry MatchLogEntry")));
	return true;
}

#endif
