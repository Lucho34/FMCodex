#include "MatchPlayCurrentAttackSkillSelectionAvailability.h"
#include "MatchPlayCurrentAttackSkillSelectionWriter.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "UObject/UnrealType.h"

namespace SkillSelectionAuthorityTests
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
	FSkillSelectionAuthorityTest,
	"FMCodex.CoreRules.MatchPlayCurrentAttackSkillSelection.Authority.SingleAuthorityAndScopeIsolation",
	EAutomationTestFlags::EditorContext
		| EAutomationTestFlags::EngineFilter)

bool FSkillSelectionAuthorityTest::RunTest(
	const FString& Parameters)
{
	using namespace SkillSelectionAuthorityTests;
	const FString RequirementHeader =
		Load(TEXT("MatchPlaySkillParticipantRequirementQuery.h"));
	const FString RequirementSource =
		Load(TEXT("MatchPlaySkillParticipantRequirementQuery.cpp"));
	const FString LegalityHeader =
		Load(TEXT("MatchPlayCurrentAttackSkillSelectionLegality.h"));
	const FString LegalitySource =
		Load(TEXT("MatchPlayCurrentAttackSkillSelectionLegality.cpp"));
	const FString AvailabilityHeader =
		Load(TEXT("MatchPlayCurrentAttackSkillSelectionAvailability.h"));
	const FString AvailabilitySource =
		Load(TEXT("MatchPlayCurrentAttackSkillSelectionAvailability.cpp"));
	const FString WriterHeader =
		Load(TEXT("MatchPlayCurrentAttackSkillSelectionWriter.h"));
	const FString WriterSource =
		Load(TEXT("MatchPlayCurrentAttackSkillSelectionWriter.cpp"));
	const FString BindingSource =
		Load(TEXT("MatchPlayCurrentAttackResolutionBinding.cpp"));
	const FString SkillProductionSources =
		RequirementSource + LegalitySource + AvailabilitySource
		+ WriterSource;

	TestEqual(
		TEXT("One participant requirement authority"),
		CountOccurrences(
			RequirementHeader,
			TEXT("FMatchPlaySkillParticipantRequirementQuery final")),
		1);
	TestEqual(
		TEXT("One public legality authority"),
		CountOccurrences(
			LegalityHeader,
			TEXT("FMatchPlayCurrentAttackSkillSelectionLegalityEvaluator final")),
		1);
	TestEqual(
		TEXT("One public availability authority"),
		CountOccurrences(
			AvailabilityHeader,
			TEXT("FMatchPlayCurrentAttackSkillSelectionAvailability final")),
		1);
	TestEqual(
		TEXT("One public writer authority"),
		CountOccurrences(
			WriterHeader,
			TEXT("FMatchPlayCurrentAttackSkillSelectionWriter final")),
		1);
	TestEqual(
		TEXT("Availability calls legality at probe and candidate sites"),
		CountOccurrences(
			AvailabilitySource,
			TEXT("FMatchPlayCurrentAttackSkillSelectionLegalityEvaluator")),
		2);
	TestEqual(
		TEXT("Writer calls legality exactly once"),
		CountOccurrences(
			WriterSource,
			TEXT("FMatchPlayCurrentAttackSkillSelectionLegalityEvaluator")),
		1);
	TestEqual(
		TEXT("Writer has no additional rule query"),
		CountOccurrences(
			WriterSource,
			TEXT("FSkillRuleSnapshotQuery")),
		0);
	TestEqual(
		TEXT("Writer has no additional requirement query"),
		CountOccurrences(
			WriterSource,
			TEXT("FMatchPlaySkillParticipantRequirementQuery")),
		0);
	TestEqual(
		TEXT("One RuleType participant mapping switch"),
		CountOccurrences(
			SkillProductionSources,
			TEXT("switch (SkillRuleType)")),
		1);

	const UScriptStruct* RequestStruct =
		FMatchPlayCurrentAttackSkillSelectionRequest::StaticStruct();
	int32 RequestFieldCount = 0;
	for (TFieldIterator<FProperty> It(RequestStruct); It; ++It)
	{
		++RequestFieldCount;
	}
	TestEqual(TEXT("Request exactly three fields"), RequestFieldCount, 3);
	TestNotNull(
		TEXT("Request AttackSequence"),
		RequestStruct->FindPropertyByName(TEXT("AttackSequence")));
	TestNotNull(
		TEXT("Request RequestingSide"),
		RequestStruct->FindPropertyByName(TEXT("RequestingSide")));
	TestNotNull(
		TEXT("Request SkillId"),
		RequestStruct->FindPropertyByName(TEXT("SkillId")));
	TestNull(
		TEXT("Request has no Carrier"),
		RequestStruct->FindPropertyByName(TEXT("CarrierCardId")));
	TestNull(
		TEXT("Request has no Marker"),
		RequestStruct->FindPropertyByName(TEXT("MarkerCardId")));
	TestNull(
		TEXT("Request has no ActionType"),
		RequestStruct->FindPropertyByName(TEXT("ActionType")));
	TestNull(
		TEXT("Request has no ActionPoint"),
		RequestStruct->FindPropertyByName(TEXT("ActionPoint")));
	TestNull(
		TEXT("Request has no decline"),
		RequestStruct->FindPropertyByName(TEXT("bDecline")));

	const UScriptStruct* SelectedActionStruct =
		FMatchPlayCurrentAttackSelectedAction::StaticStruct();
	TestNull(
		TEXT("No Runner selected action field"),
		SelectedActionStruct->FindPropertyByName(TEXT("RunnerCardId")));
	TestNull(
		TEXT("No Helper selected action field"),
		SelectedActionStruct->FindPropertyByName(TEXT("HelperCardId")));
	TestNull(
		TEXT("No Helper presence field"),
		SelectedActionStruct->FindPropertyByName(TEXT("bHasHelper")));

	TestFalse(
		TEXT("No old combined action selection"),
		SkillProductionSources.Contains(
			TEXT("CurrentAttackActionSelection")));
	TestFalse(
		TEXT("No MatchPlayAttackFlow"),
		SkillProductionSources.Contains(TEXT("MatchPlayAttackFlow")));
	TestFalse(
		TEXT("No Formula"),
		SkillProductionSources.Contains(TEXT("Formula")));
	TestFalse(
		TEXT("No D6"),
		SkillProductionSources.Contains(TEXT("D6")));
	TestFalse(
		TEXT("No Branch"),
		SkillProductionSources.Contains(TEXT("Branch")));
	TestFalse(
		TEXT("No CardUsage"),
		SkillProductionSources.Contains(TEXT("CardUsage")));
	TestFalse(
		TEXT("No Score"),
		SkillProductionSources.Contains(TEXT("Score")));
	TestFalse(
		TEXT("No Opportunity"),
		SkillProductionSources.Contains(TEXT("Opportunity")));
	TestFalse(
		TEXT("No goalkeeper access"),
		SkillProductionSources.Contains(TEXT("Goalkeeper")));
	TestFalse(
		TEXT("No automatic Begin"),
		SkillProductionSources.Contains(TEXT("BeginOrdinaryAttack")));
	TestFalse(
		TEXT("Binding does not query skill rules"),
		BindingSource.Contains(TEXT("SkillRuleSnapshot")));
	TestFalse(
		TEXT("Binding does not inspect ActionPoint"),
		BindingSource.Contains(TEXT("ActionPoint")));
	TestFalse(
		TEXT("Binding does not select Branch"),
		BindingSource.Contains(TEXT("Branch")));
	TestFalse(
		TEXT("Binding does not construct Formula"),
		BindingSource.Contains(TEXT("Formula")));
	TestFalse(
		TEXT("Binding does not use D6"),
		BindingSource.Contains(TEXT("D6")));
	return true;
}

#endif
