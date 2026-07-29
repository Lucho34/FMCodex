#include "MatchPlayCurrentAttackHelperSelectionAvailability.h"
#include "MatchPlayCurrentAttackHelperSelectionWriter.h"
#include "MatchPlayHelperAbsence.h"
#include "MatchPlayHelperAbsenceCapability.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "UObject/UnrealType.h"

#include <type_traits>

namespace HelperAuthorityTests
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
			Token,
			ESearchCase::CaseSensitive,
			ESearchDir::FromStart,
			From)) != INDEX_NONE)
		{
			++Result;
			From += FCString::Strlen(Token);
		}
		return Result;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHelperSelectionAuthorityTest,
	"FMCodex.CoreRules.MatchPlayCurrentAttackHelperSelection.Authority.Contract",
	EAutomationTestFlags::EditorContext
		| EAutomationTestFlags::EngineFilter)

bool FHelperSelectionAuthorityTest::RunTest(
	const FString& Parameters)
{
	using namespace HelperAuthorityTests;
	const UScriptStruct* Request =
		FMatchPlayCurrentAttackHelperSelectionRequest::StaticStruct();
	int32 RequestFields = 0;
	for (TFieldIterator<FProperty> It(Request); It; ++It)
	{
		++RequestFields;
	}
	TestEqual(TEXT("Helper Request exactly 3 fields"),
		RequestFields, 3);
	TestNotNull(TEXT("AttackSequence"),
		Request->FindPropertyByName(TEXT("AttackSequence")));
	TestNotNull(TEXT("RequestingSide"),
		Request->FindPropertyByName(TEXT("RequestingSide")));
	TestNotNull(TEXT("HelperCardId"),
		Request->FindPropertyByName(TEXT("HelperCardId")));

	const UScriptStruct* ResolveRequest =
		FMatchPlayResolveNoLegalHelperRequest::StaticStruct();
	int32 ResolveFields = 0;
	for (TFieldIterator<FProperty> It(ResolveRequest); It; ++It)
	{
		++ResolveFields;
	}
	TestEqual(TEXT("Resolve Request exactly 1 field"),
		ResolveFields, 1);
	const UScriptStruct* DeclineRequest =
		FMatchPlayHelperDeclineRequest::StaticStruct();
	int32 DeclineFields = 0;
	for (TFieldIterator<FProperty> It(DeclineRequest); It; ++It)
	{
		++DeclineFields;
	}
	TestEqual(TEXT("Decline Request exactly 2 fields"),
		DeclineFields, 2);

	const UScriptStruct* Selected =
		FMatchPlayCurrentAttackSelectedAction::StaticStruct();
	TArray<FName> SelectedNames;
	for (TFieldIterator<FProperty> It(Selected); It; ++It)
	{
		SelectedNames.Add(It->GetFName());
	}
	const TArray<FName> ExpectedNames = {
		TEXT("CarrierCardId"),
		TEXT("MarkerCardId"),
		TEXT("SkillId"),
		TEXT("ActionType"),
		TEXT("RunnerCardId"),
		TEXT("bHasHelper"),
		TEXT("HelperCardId")
	};
	TestEqual(TEXT("SelectedAction exactly 7 fields"),
		SelectedNames.Num(), ExpectedNames.Num());
	for (int32 Index = 0;
		Index < SelectedNames.Num()
			&& Index < ExpectedNames.Num();
		++Index)
	{
		TestEqual(TEXT("SelectedAction field order"),
			SelectedNames[Index], ExpectedNames[Index]);
	}

	TestFalse(TEXT("Capability is not aggregate"),
		std::is_aggregate_v<FMatchPlayHelperAbsenceCapability>);
	TestFalse(TEXT("Capability not default constructible"),
		std::is_default_constructible_v<
			FMatchPlayHelperAbsenceCapability>);
	TestFalse(TEXT("Capability not copy constructible"),
		std::is_copy_constructible_v<
			FMatchPlayHelperAbsenceCapability>);
	TestFalse(TEXT("Capability not move constructible"),
		std::is_move_constructible_v<
			FMatchPlayHelperAbsenceCapability>);
	TestFalse(TEXT("Capability not publicly constructible"),
		std::is_constructible_v<
			FMatchPlayHelperAbsenceCapability,
			int64,
			const FMatchPlayCurrentAttackHelperSelectionAvailabilityResult&>);

	const UScriptStruct* ResolveResult =
		FMatchPlayResolveNoLegalHelperResult::StaticStruct();
	TestNull(TEXT("Resolve Result hides capability"),
		ResolveResult->FindPropertyByName(TEXT("Capability")));
	const UScriptStruct* DeclineResult =
		FMatchPlayHelperDeclineResult::StaticStruct();
	TestNull(TEXT("Decline Result hides capability"),
		DeclineResult->FindPropertyByName(TEXT("Capability")));

	const FString GlobalHeader = Load(
		TEXT("MatchPlayCurrentAttackHelperSelectionGlobalContextQuery.h"));
	const FString GlobalSource = Load(
		TEXT("MatchPlayCurrentAttackHelperSelectionGlobalContextQuery.cpp"));
	const FString LegalityHeader = Load(
		TEXT("MatchPlayCurrentAttackHelperSelectionLegality.h"));
	const FString LegalitySource = Load(
		TEXT("MatchPlayCurrentAttackHelperSelectionLegality.cpp"));
	const FString AvailabilityHeader = Load(
		TEXT("MatchPlayCurrentAttackHelperSelectionAvailability.h"));
	const FString AvailabilitySource = Load(
		TEXT("MatchPlayCurrentAttackHelperSelectionAvailability.cpp"));
	const FString WriterHeader = Load(
		TEXT("MatchPlayCurrentAttackHelperSelectionWriter.h"));
	const FString WriterSource = Load(
		TEXT("MatchPlayCurrentAttackHelperSelectionWriter.cpp"));
	const FString AbsenceSource = Load(
		TEXT("MatchPlayHelperAbsence.cpp"));
	const FString FinalizationSource = Load(
		TEXT("MatchPlayCurrentAttackHelperFinalization.cpp"));
	const FString Production = GlobalSource + LegalitySource
		+ AvailabilitySource + WriterSource + AbsenceSource
		+ FinalizationSource;

	TestEqual(TEXT("One Global public class"), Count(GlobalHeader,
		TEXT("FMatchPlayCurrentAttackHelperSelectionGlobalContextQuery final")), 1);
	TestEqual(TEXT("Global implementation once"), Count(GlobalSource,
		TEXT("FMatchPlayCurrentAttackHelperSelectionGlobalContextQuery::Query")), 1);
	TestEqual(TEXT("Global header has no HelperCardId"),
		Count(GlobalHeader, TEXT("HelperCardId")), 0);
	TestEqual(TEXT("One Legality class"), Count(LegalityHeader,
		TEXT("FMatchPlayCurrentAttackHelperSelectionLegalityEvaluator final")), 1);
	TestEqual(TEXT("One Availability class"), Count(AvailabilityHeader,
		TEXT("FMatchPlayCurrentAttackHelperSelectionAvailability final")), 1);
	TestEqual(TEXT("One Writer class"), Count(WriterHeader,
		TEXT("FMatchPlayCurrentAttackHelperSelectionWriter final")), 1);
	TestEqual(TEXT("Legality Global call once"), Count(LegalitySource,
		TEXT("FMatchPlayCurrentAttackHelperSelectionGlobalContextQuery::Query")), 1);
	TestEqual(TEXT("Availability Global call once"),
		Count(AvailabilitySource,
			TEXT("FMatchPlayCurrentAttackHelperSelectionGlobalContextQuery::Query")), 1);
	TestEqual(TEXT("Availability candidate core site once"),
		Count(AvailabilitySource, TEXT("::EvaluateWithGlobalContext")), 1);
	TestEqual(TEXT("Writer Legality once"), Count(WriterSource,
		TEXT("FMatchPlayCurrentAttackHelperSelectionLegalityEvaluator::Evaluate")), 1);
	TestEqual(TEXT("Writer Availability zero"),
		Count(WriterSource, TEXT("SelectionAvailability")), 0);
	TestEqual(TEXT("Resolve/Decline Availability two total"),
		Count(AbsenceSource,
			TEXT("FMatchPlayCurrentAttackHelperSelectionAvailability::Query")), 2);
	TestEqual(TEXT("One private absence finalizer"),
		Count(AbsenceSource, TEXT("FinalizeWithoutHelper(")), 3);
	TestEqual(TEXT("One final SelectedAction implementation"),
		Count(FinalizationSource, TEXT("ApplyFinalSelectedAction(")), 1);
	TestFalse(TEXT("No fake Helper probe"),
		Production.Contains(TEXT("ProbeHelperCardId"))
			|| Production.Contains(TEXT("HelperAvailabilityProbe"))
			|| Production.Contains(TEXT("DummyHelper"))
			|| Production.Contains(TEXT("SentinelHelper"))
			|| Production.Contains(TEXT("PlaceholderHelper"))
			|| Production.Contains(TEXT("SyntheticHelper")));
	TestFalse(TEXT("No manual global classification"),
		AvailabilitySource.Contains(TEXT("IsGlobalBlocker"))
			|| AvailabilitySource.Contains(TEXT("switch (")));
	for (const TCHAR* Forbidden : {
		TEXT("MatchPlayAttackFlow"),
		TEXT("CurrentAttackCompletion"),
		TEXT("GoalResolver"),
		TEXT("PlayCardResolver"),
		TEXT("OpportunityResolver"),
		TEXT("MatchEnd"),
		TEXT("MatchResult")})
	{
		TestFalse(TEXT("Forbidden production dependency absent"),
			Production.Contains(Forbidden));
	}
	return true;
}

#endif
