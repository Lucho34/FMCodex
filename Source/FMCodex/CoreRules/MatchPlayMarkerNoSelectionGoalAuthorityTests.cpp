#include "MatchPlayMarkerNoSelectionGoal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "UObject/UnrealType.h"

#include <type_traits>

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMarkerNoSelectionGoalAuthorityIsolationTest,
	"FMCodex.CoreRules.MatchPlayMarkerNoSelectionGoal.AuthorityIsolation.SinglePublicAuthorityShape",
	EAutomationTestFlags::EditorContext
		| EAutomationTestFlags::EngineFilter)

bool FMarkerNoSelectionGoalAuthorityIsolationTest::RunTest(
	const FString& Parameters)
{
	using FNoLegalSignature =
		FMatchPlayResolveNoLegalMarkerResult (*)(
			const FMatchPlayState&,
			const FMatchPlayResolveNoLegalMarkerRequest&);
	using FDeclineSignature =
		FMatchPlayMarkerDeclineResult (*)(
			const FMatchPlayState&,
			const FMatchPlayMarkerDeclineRequest&);
	using FCompletionSignature =
		FMatchPlayCurrentAttackCompletionResult (*)(
			const FMatchPlayState&,
			const FMatchPlayMarkerNoSelectionGoalProjection&);
	TestTrue(TEXT("No-legal-marker has one public signature"),
		(std::is_same_v<
			decltype(&FMatchPlayResolveNoLegalMarker::Resolve),
			FNoLegalSignature>));
	TestTrue(TEXT("Marker decline has one public signature"),
		(std::is_same_v<
			decltype(&FMatchPlayMarkerDecline::Decline),
			FDeclineSignature>));
	TestTrue(TEXT("Completion has one public signature"),
		(std::is_same_v<
			decltype(&FMatchPlayCurrentAttackCompletion::Complete),
			FCompletionSignature>));

	int32 NoLegalRequestPropertyCount = 0;
	for (TFieldIterator<FProperty> Property(
		FMatchPlayResolveNoLegalMarkerRequest::StaticStruct());
		Property;
		++Property)
	{
		++NoLegalRequestPropertyCount;
	}
	TestEqual(TEXT("System request contains only AttackSequence"),
		NoLegalRequestPropertyCount, 1);
	TestNotNull(TEXT("System request exposes AttackSequence"),
		FMatchPlayResolveNoLegalMarkerRequest::StaticStruct()
			->FindPropertyByName(GET_MEMBER_NAME_CHECKED(
				FMatchPlayResolveNoLegalMarkerRequest,
				AttackSequence)));

	int32 DeclineRequestPropertyCount = 0;
	for (TFieldIterator<FProperty> Property(
		FMatchPlayMarkerDeclineRequest::StaticStruct());
		Property;
		++Property)
	{
		++DeclineRequestPropertyCount;
	}
	TestEqual(TEXT("Decline request contains exactly two fields"),
		DeclineRequestPropertyCount, 2);
	TestNotNull(TEXT("Decline request exposes sequence"),
		FMatchPlayMarkerDeclineRequest::StaticStruct()
			->FindPropertyByName(GET_MEMBER_NAME_CHECKED(
				FMatchPlayMarkerDeclineRequest,
				AttackSequence)));
	TestNotNull(TEXT("Decline request exposes requesting side"),
		FMatchPlayMarkerDeclineRequest::StaticStruct()
			->FindPropertyByName(GET_MEMBER_NAME_CHECKED(
				FMatchPlayMarkerDeclineRequest,
				RequestingSide)));

	const UScriptStruct* StateStruct = FMatchPlayState::StaticStruct();
	TestNull(TEXT("State has no persistent goal projection"),
		StateStruct->FindPropertyByName(TEXT("GoalProjection")));
	TestNull(TEXT("State has no persistent outcome"),
		StateStruct->FindPropertyByName(TEXT("Outcome")));
	TestNull(TEXT("State has no persistent completion stage"),
		StateStruct->FindPropertyByName(TEXT("CompletionStage")));
	return true;
}

#endif
