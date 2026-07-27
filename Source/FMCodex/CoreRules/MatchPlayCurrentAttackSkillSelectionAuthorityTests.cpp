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
	const FString GlobalContextHeader =
		Load(TEXT("MatchPlayCurrentAttackSkillSelectionGlobalContextQuery.h"));
	const FString GlobalContextSource =
		Load(TEXT("MatchPlayCurrentAttackSkillSelectionGlobalContextQuery.cpp"));
	const FString SelectionTypesHeader =
		Load(TEXT("MatchPlayCurrentAttackSkillSelectionTypes.h"));
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
	const FString ValidatorSource =
		Load(TEXT("MatchPlayCurrentAttackSelectionStateValidator.cpp"));
	const FString BindingSource =
		Load(TEXT("MatchPlayCurrentAttackResolutionBinding.cpp"));
	const FString SkillProductionSources =
		RequirementSource + GlobalContextSource + LegalitySource
		+ AvailabilitySource + WriterSource + ValidatorSource;

	TestEqual(
		TEXT("One participant requirement authority"),
		CountOccurrences(
			RequirementHeader,
			TEXT("FMatchPlaySkillParticipantRequirementQuery final")),
		1);
	TestEqual(
		TEXT("One public global context authority"),
		CountOccurrences(
			GlobalContextHeader,
			TEXT("FMatchPlayCurrentAttackSkillSelectionGlobalContextQuery final")),
		1);
	TestEqual(
		TEXT("One global context implementation"),
		CountOccurrences(
			GlobalContextSource,
			TEXT("FMatchPlayCurrentAttackSkillSelectionGlobalContextQuery::Query")),
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
		TEXT("Availability calls candidate legality at one loop site"),
		CountOccurrences(
			AvailabilitySource,
			TEXT("FMatchPlayCurrentAttackSkillSelectionLegalityEvaluator")),
		1);
	TestEqual(
		TEXT("Availability calls global context exactly once"),
		CountOccurrences(
			AvailabilitySource,
			TEXT("FMatchPlayCurrentAttackSkillSelectionGlobalContextQuery")),
		1);
	TestEqual(
		TEXT("Legality calls global context exactly once"),
		CountOccurrences(
			LegalitySource,
			TEXT("FMatchPlayCurrentAttackSkillSelectionGlobalContextQuery")),
		1);
	TestEqual(
		TEXT("Global AP validation has one implementation site"),
		CountOccurrences(
			GlobalContextSource,
			TEXT("::InvalidCurrentAttackActionPoint")),
		1);
	TestEqual(
		TEXT("Legality has no duplicate global AP validation"),
		CountOccurrences(
			LegalitySource,
			TEXT("::InvalidCurrentAttackActionPoint")),
		0);
	TestEqual(
		TEXT("Availability has no duplicate global AP validation"),
		CountOccurrences(
			AvailabilitySource,
			TEXT("::InvalidCurrentAttackActionPoint")),
		0);
	TestFalse(
		TEXT("Availability has no fake or probe SkillId"),
		AvailabilitySource.Contains(TEXT("ProbeSkillId"))
			|| AvailabilitySource.Contains(TEXT("AvailabilityProbe")));
	TestFalse(
		TEXT("Availability has no global error classification"),
		AvailabilitySource.Contains(TEXT("IsGlobalBlocker"))
			|| AvailabilitySource.Contains(TEXT("switch (")));
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
	TestTrue(
		TEXT("Request contract moved intact to shared types"),
		SelectionTypesHeader.Contains(
			TEXT("FMatchPlayCurrentAttackSkillSelectionRequest")));
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
