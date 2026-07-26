#include "MatchPlayMarkerNoSelectionGoal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "MatchPlayMarkerNoSelectionGoalTestFixtures.h"
#include "Misc/AutomationTest.h"
#include "UObject/UnrealType.h"

#include <type_traits>
#include <utility>

namespace MatchPlayMarkerNoSelectionGoalAuthorityTests
{
	template <typename T, typename = void>
	struct THasPublicComplete : std::false_type
	{
	};

	template <typename T>
	struct THasPublicComplete<
		T,
		std::void_t<decltype(&T::Complete)>> : std::true_type
	{
	};

	template <typename T, typename = void>
	struct THasStaticStruct : std::false_type
	{
	};

	template <typename T>
	struct THasStaticStruct<
		T,
		std::void_t<decltype(T::StaticStruct())>> : std::true_type
	{
	};

	template <typename T, typename = void>
	struct THasPublicWritableAuthorityFields : std::false_type
	{
	};

	template <typename T>
	struct THasPublicWritableAuthorityFields<
		T,
		std::void_t<
			decltype(std::declval<T&>().AttackSequence = int64{}),
			decltype(
				std::declval<T&>().Reason =
					EMatchPlayMarkerNoSelectionGoalReason::None),
			decltype(
				std::declval<T&>().Source =
					EMatchPlayMarkerNoSelectionGoalSource::None),
			decltype(
				std::declval<T&>().AuthorityResult =
					std::declval<
						FMatchPlayCurrentAttackMarkerSelectionAvailabilityResult>())>>
		: std::true_type
	{
	};

	template <typename T, typename = void>
	struct THasPublicCapabilityFactory : std::false_type
	{
	};

	template <typename T>
	struct THasPublicCapabilityFactory<
		T,
		std::void_t<decltype(&T::Create)>> : std::true_type
	{
	};

#define DECLARE_PUBLIC_SETTER_TRAIT(TraitName, Expression) \
	template <typename T, typename = void> \
	struct TraitName : std::false_type \
	{ \
	}; \
	template <typename T> \
	struct TraitName<T, std::void_t<decltype(Expression)>> \
		: std::true_type \
	{ \
	};

	DECLARE_PUBLIC_SETTER_TRAIT(
		THasPublicSetAttackSequence,
		std::declval<T&>().SetAttackSequence(int64{}))
	DECLARE_PUBLIC_SETTER_TRAIT(
		THasPublicSetReason,
		std::declval<T&>().SetReason(
			EMatchPlayMarkerNoSelectionGoalReason::None))
	DECLARE_PUBLIC_SETTER_TRAIT(
		THasPublicSetSource,
		std::declval<T&>().SetSource(
			EMatchPlayMarkerNoSelectionGoalSource::None))
	DECLARE_PUBLIC_SETTER_TRAIT(
		THasPublicSetAuthorityResult,
		std::declval<T&>().SetAuthorityResult(
			std::declval<
				const FMatchPlayCurrentAttackMarkerSelectionAvailabilityResult&>()))

#undef DECLARE_PUBLIC_SETTER_TRAIT
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMarkerNoSelectionGoalAuthorityIsolationTest,
	"FMCodex.CoreRules.MatchPlayMarkerNoSelectionGoal.AuthorityHardening.PublicBoundary",
	EAutomationTestFlags::EditorContext
		| EAutomationTestFlags::EngineFilter)

bool FMarkerNoSelectionGoalAuthorityIsolationTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayMarkerNoSelectionGoalAuthorityTests;
	using FCapability =
		FMatchPlayMarkerNoSelectionGoalCapability;
	using FNoLegalSignature =
		FMatchPlayResolveNoLegalMarkerResult (*)(
			const FMatchPlayState&,
			const FMatchPlayResolveNoLegalMarkerRequest&);
	using FDeclineSignature =
		FMatchPlayMarkerDeclineResult (*)(
			const FMatchPlayState&,
			const FMatchPlayMarkerDeclineRequest&);

	static_assert(
		!THasPublicComplete<FMatchPlayCurrentAttackCompletion>::value,
		"Completion must not be a public callable business entry.");
	static_assert(
		!std::is_default_constructible_v<FCapability>,
		"Capability must not be default constructible.");
	static_assert(
		!std::is_aggregate_v<FCapability>,
		"Capability must not be aggregate constructible.");
	static_assert(
		!std::is_constructible_v<
			FCapability,
			int64,
			EMatchPlayMarkerNoSelectionGoalReason,
			EMatchPlayMarkerNoSelectionGoalSource,
			const FMatchPlayCurrentAttackMarkerSelectionAvailabilityResult&>,
		"Raw authority fields must not publicly construct a capability.");
	static_assert(
		!THasStaticStruct<FCapability>::value,
		"Capability must not be reflected.");
	static_assert(
		!THasPublicWritableAuthorityFields<FCapability>::value,
		"Capability authority fields must not be publicly writable.");
	static_assert(
		!THasPublicCapabilityFactory<FCapability>::value,
		"Capability must not expose a public factory.");
	static_assert(
		!THasPublicSetAttackSequence<FCapability>::value
			&& !THasPublicSetReason<FCapability>::value
			&& !THasPublicSetSource<FCapability>::value
			&& !THasPublicSetAuthorityResult<FCapability>::value,
		"Capability must not expose public authority setters.");

	TestTrue(TEXT("No-legal-marker has its one public signature"),
		(std::is_same_v<
			decltype(&FMatchPlayResolveNoLegalMarker::Resolve),
			FNoLegalSignature>));
	TestTrue(TEXT("Marker decline has its one public signature"),
		(std::is_same_v<
			decltype(&FMatchPlayMarkerDecline::Decline),
			FDeclineSignature>));
	TestFalse(TEXT("Completion is not publicly callable"),
		THasPublicComplete<FMatchPlayCurrentAttackCompletion>::value);
	TestFalse(TEXT("Capability is not default constructible"),
		std::is_default_constructible_v<FCapability>);
	TestFalse(TEXT("Capability is not aggregate"),
		std::is_aggregate_v<FCapability>);
	TestFalse(TEXT("Capability is not reflected"),
		THasStaticStruct<FCapability>::value);
	TestFalse(TEXT("Capability fields are not publicly writable"),
		THasPublicWritableAuthorityFields<FCapability>::value);
	TestFalse(TEXT("Capability has no public factory"),
		THasPublicCapabilityFactory<FCapability>::value);
	TestFalse(TEXT("Capability has no public authority setters"),
		THasPublicSetAttackSequence<FCapability>::value
			|| THasPublicSetReason<FCapability>::value
			|| THasPublicSetSource<FCapability>::value
			|| THasPublicSetAuthorityResult<FCapability>::value);

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

	const TArray<const UScriptStruct*> PublicResultStructs = {
		FMatchPlayResolveNoLegalMarkerResult::StaticStruct(),
		FMatchPlayMarkerDeclineResult::StaticStruct(),
		FMatchPlayCurrentAttackCompletionResult::StaticStruct()
	};
	for (const UScriptStruct* ResultStruct : PublicResultStructs)
	{
		TestNull(TEXT("Public result exposes no capability"),
			ResultStruct->FindPropertyByName(TEXT("Capability")));
		TestNull(TEXT("Public result exposes no projection"),
			ResultStruct->FindPropertyByName(TEXT("GoalProjection")));
		TestNull(TEXT("Public result exposes no legacy projection"),
			ResultStruct->FindPropertyByName(TEXT("Projection")));
	}

	const TArray<FName> ForbiddenRequestFields = {
		TEXT("Availability"),
		TEXT("MarkerAvailabilityResult"),
		TEXT("Reason"),
		TEXT("Source"),
		TEXT("Goal"),
		TEXT("bIsGoal"),
		TEXT("bCanSelectAnyMarker"),
		TEXT("Score"),
		TEXT("ScoringSide")
	};
	for (const FName FieldName : ForbiddenRequestFields)
	{
		TestNull(TEXT("System request carries no authority data"),
			FMatchPlayResolveNoLegalMarkerRequest::StaticStruct()
				->FindPropertyByName(FieldName));
		TestNull(TEXT("Decline request carries no authority data"),
			FMatchPlayMarkerDeclineRequest::StaticStruct()
				->FindPropertyByName(FieldName));
	}

	const UScriptStruct* StateStruct = FMatchPlayState::StaticStruct();
	TestNull(TEXT("State has no persistent goal projection"),
		StateStruct->FindPropertyByName(TEXT("GoalProjection")));
	TestNull(TEXT("State has no persistent outcome"),
		StateStruct->FindPropertyByName(TEXT("Outcome")));
	TestNull(TEXT("State has no persistent completion stage"),
		StateStruct->FindPropertyByName(TEXT("CompletionStage")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMarkerNoSelectionGoalLegalCandidateForgeryBoundaryTest,
	"FMCodex.CoreRules.MatchPlayMarkerNoSelectionGoal.AuthorityHardening.AdversarialPublicBoundary.LegalCandidateCannotBecomeNoLegalGoal",
	EAutomationTestFlags::EditorContext
		| EAutomationTestFlags::EngineFilter)

bool FMarkerNoSelectionGoalLegalCandidateForgeryBoundaryTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayMarkerNoSelectionGoal;
	const FMatchPlayState State = MakeState();
	const FMatchPlayResolveNoLegalMarkerResult Result =
		FMatchPlayResolveNoLegalMarker::Resolve(
			State,
			MakeNoLegalRequest());
	TestFalse(TEXT("System path rejects when a legal marker exists"),
		Result.bSuccess);
	TestEqual(TEXT("Exact public error is LegalMarkerExists"),
		Result.ErrorCode,
		EMatchPlayResolveNoLegalMarkerErrorCode::LegalMarkerExists);
	TestTrue(TEXT("Entire state remains unchanged"),
		AreStatesEqual(Result.AfterState, State));
	TestEqual(TEXT("Score remains unchanged"),
		Result.AfterState.RuntimeState.PlayerAState.Score,
		State.RuntimeState.PlayerAState.Score);
	TestTrue(TEXT("CardUsage remains unchanged"),
		FMatchCardUsageState::StaticStruct()->CompareScriptStruct(
			&Result.AfterState.CardUsageState,
			&State.CardUsageState,
			0));
	TestEqual(TEXT("Opportunity remains unchanged"),
		Result.AfterState.RuntimeState.PlayerAState.UsedAttackCount,
		State.RuntimeState.PlayerAState.UsedAttackCount);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMarkerNoSelectionGoalIllegalCandidateForgeryBoundaryTest,
	"FMCodex.CoreRules.MatchPlayMarkerNoSelectionGoal.AuthorityHardening.AdversarialPublicBoundary.IllegalCandidateCannotBecomeDeclineGoal",
	EAutomationTestFlags::EditorContext
		| EAutomationTestFlags::EngineFilter)

bool FMarkerNoSelectionGoalIllegalCandidateForgeryBoundaryTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayMarkerNoSelectionGoal;
	FMatchPlayState State = MakeState();
	MakeOnlyOtherAreaDefender(State);
	const FMatchPlayMarkerDeclineResult Result =
		FMatchPlayMarkerDecline::Decline(
			State,
			MakeDeclineRequest());
	TestFalse(TEXT("Decline rejects when no legal marker exists"),
		Result.bSuccess);
	TestEqual(TEXT("Exact public error is NoLegalMarkerToDecline"),
		Result.ErrorCode,
		EMatchPlayMarkerDeclineErrorCode::NoLegalMarkerToDecline);
	TestTrue(TEXT("Entire state remains unchanged"),
		AreStatesEqual(Result.AfterState, State));
	TestEqual(TEXT("Score remains unchanged"),
		Result.AfterState.RuntimeState.PlayerAState.Score,
		State.RuntimeState.PlayerAState.Score);
	TestTrue(TEXT("CardUsage remains unchanged"),
		FMatchCardUsageState::StaticStruct()->CompareScriptStruct(
			&Result.AfterState.CardUsageState,
			&State.CardUsageState,
			0));
	TestEqual(TEXT("Opportunity remains unchanged"),
		Result.AfterState.RuntimeState.PlayerAState.UsedAttackCount,
		State.RuntimeState.PlayerAState.UsedAttackCount);
	return true;
}

#endif
