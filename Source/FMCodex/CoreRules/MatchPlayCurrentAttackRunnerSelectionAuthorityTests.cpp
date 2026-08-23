#include "MatchPlayCurrentAttackRunnerSelectionAvailability.h"
#include "MatchPlayCurrentAttackRunnerSelectionWriter.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "UObject/UnrealType.h"

namespace RunnerAuthorityTests
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

	int32 Count(const FString& Source, const TCHAR* Token)
	{
		int32 Result = 0;
		int32 From = 0;
		while ((From = Source.Find(
			Token, ESearchCase::CaseSensitive,
			ESearchDir::FromStart, From)) != INDEX_NONE)
		{
			++Result;
			From += FCString::Strlen(Token);
		}
		return Result;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FRunnerSelectionAuthorityTest,
	"FMCodex.CoreRules.MatchPlayCurrentAttackRunnerSelection.Authority.SingleAuthorityAndIsolation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FRunnerSelectionAuthorityTest::RunTest(const FString& Parameters)
{
	using namespace RunnerAuthorityTests;
	const FString GlobalHeader =
		Load(TEXT("MatchPlayCurrentAttackRunnerSelectionGlobalContextQuery.h"));
	const FString GlobalSource =
		Load(TEXT("MatchPlayCurrentAttackRunnerSelectionGlobalContextQuery.cpp"));
	const FString LegalityHeader =
		Load(TEXT("MatchPlayCurrentAttackRunnerSelectionLegality.h"));
	const FString LegalitySource =
		Load(TEXT("MatchPlayCurrentAttackRunnerSelectionLegality.cpp"));
	const FString AvailabilityHeader =
		Load(TEXT("MatchPlayCurrentAttackRunnerSelectionAvailability.h"));
	const FString AvailabilitySource =
		Load(TEXT("MatchPlayCurrentAttackRunnerSelectionAvailability.cpp"));
	const FString WriterHeader =
		Load(TEXT("MatchPlayCurrentAttackRunnerSelectionWriter.h"));
	const FString WriterSource =
		Load(TEXT("MatchPlayCurrentAttackRunnerSelectionWriter.cpp"));
	const FString ValidatorSource =
		Load(TEXT("MatchPlayCurrentAttackSelectionStateValidator.cpp"));
	const FString Production = GlobalSource + LegalitySource
		+ AvailabilitySource + WriterSource;

	const UScriptStruct* Request =
		FMatchPlayCurrentAttackRunnerSelectionRequest::StaticStruct();
	int32 FieldCount = 0;
	for (TFieldIterator<FProperty> It(Request); It; ++It)
	{
		++FieldCount;
	}
	TestEqual(TEXT("Request exactly three fields"), FieldCount, 3);
	TestNotNull(TEXT("AttackSequence field"),
		Request->FindPropertyByName(TEXT("AttackSequence")));
	TestNotNull(TEXT("RequestingSide field"),
		Request->FindPropertyByName(TEXT("RequestingSide")));
	TestNotNull(TEXT("RunnerCardId field"),
		Request->FindPropertyByName(TEXT("RunnerCardId")));
	for (const FName Forbidden : {
		FName(TEXT("CarrierCardId")), FName(TEXT("SkillId")),
		FName(TEXT("SlotId")), FName(TEXT("bDecline")),
		FName(TEXT("HelperCardId"))})
	{
		TestNull(TEXT("Forbidden request field absent"),
			Request->FindPropertyByName(Forbidden));
	}

	TestEqual(TEXT("One global class"), Count(GlobalHeader,
		TEXT("FMatchPlayCurrentAttackRunnerSelectionGlobalContextQuery final")), 1);
	TestEqual(TEXT("One global implementation"), Count(GlobalSource,
		TEXT("FMatchPlayCurrentAttackRunnerSelectionGlobalContextQuery::Query")), 1);
	TestEqual(TEXT("Global header has no runner candidate input"),
		Count(GlobalHeader, TEXT("RunnerCardId")), 0);
	TestEqual(TEXT("One legality class"), Count(LegalityHeader,
		TEXT("FMatchPlayCurrentAttackRunnerSelectionLegalityEvaluator final")), 1);
	TestEqual(TEXT("One availability class"), Count(AvailabilityHeader,
		TEXT("FMatchPlayCurrentAttackRunnerSelectionAvailability final")), 1);
	TestEqual(TEXT("One writer class"), Count(WriterHeader,
		TEXT("FMatchPlayCurrentAttackRunnerSelectionWriter final")), 1);
	TestEqual(TEXT("Legality one global call"), Count(LegalitySource,
		TEXT("FMatchPlayCurrentAttackRunnerSelectionGlobalContextQuery::Query")), 1);
	TestEqual(TEXT("Availability one global call"), Count(AvailabilitySource,
		TEXT("FMatchPlayCurrentAttackRunnerSelectionGlobalContextQuery::Query")), 1);
	TestEqual(TEXT("Availability one candidate legality site"),
		Count(AvailabilitySource,
			TEXT("FMatchPlayCurrentAttackRunnerSelectionLegalityEvaluator")), 1);
	TestEqual(TEXT("Writer one legality call"), Count(WriterSource,
		TEXT("FMatchPlayCurrentAttackRunnerSelectionLegalityEvaluator::Evaluate")), 1);
	TestEqual(TEXT("Writer no availability"), Count(WriterSource,
		TEXT("RunnerSelectionAvailability")), 0);
	TestEqual(TEXT("Writer no completion"), Count(WriterSource,
		TEXT("Completion")), 0);
	TestEqual(TEXT("Validator requirement calls cover legacy Runner Helper Ready boundaries"),
		Count(ValidatorSource,
			TEXT("FMatchPlaySkillParticipantRequirementQuery::Query")), 3);
	TestTrue(TEXT("Validator has AwaitingHelper"),
		ValidatorSource.Contains(TEXT("::AwaitingHelper:")));
	TestFalse(TEXT("No fake runner probe"),
		Production.Contains(TEXT("ProbeRunnerCardId"))
			|| Production.Contains(TEXT("RunnerAvailabilityProbe")));
	TestFalse(TEXT("No manual global classification"),
		AvailabilitySource.Contains(TEXT("IsGlobalBlocker"))
			|| AvailabilitySource.Contains(TEXT("switch (")));
	for (const TCHAR* Forbidden : {
		TEXT("MatchPlayAttackFlow"), TEXT("Completion"),
		TEXT("GoalResolver"), TEXT("PlayCardResolver"),
		TEXT("OpportunityResolver"), TEXT("MatchEnd"),
		TEXT("MatchResult")})
	{
		TestFalse(TEXT("Forbidden dependency absent"),
			Production.Contains(Forbidden));
	}
	return true;
}

#endif
