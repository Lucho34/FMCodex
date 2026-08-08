#include "MatchPlayRunnerNoSelectionNoGoal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "UObject/UnrealType.h"

#include <type_traits>
#include <utility>

namespace RunnerNoSelectionNoGoalAuthorityTests
{
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

#define DECLARE_PUBLIC_MEMBER_TRAIT(TraitName, Expression) \
	template <typename T, typename = void> \
	struct TraitName : std::false_type \
	{ \
	}; \
	template <typename T> \
	struct TraitName<T, std::void_t<decltype(Expression)>> \
		: std::true_type \
	{ \
	};

	DECLARE_PUBLIC_MEMBER_TRAIT(
		THasPublicAttackSequenceField,
		std::declval<T&>().AttackSequence = int64{})
	DECLARE_PUBLIC_MEMBER_TRAIT(
		THasPublicReasonField,
		std::declval<T&>().Reason =
			EMatchPlayRunnerNoSelectionNoGoalReason::None)
	DECLARE_PUBLIC_MEMBER_TRAIT(
		THasPublicSourceField,
		std::declval<T&>().Source =
			EMatchPlayRunnerNoSelectionNoGoalSource::None)
	DECLARE_PUBLIC_MEMBER_TRAIT(
		THasPublicAuthorityResultField,
		std::declval<T&>().AuthorityResult =
			std::declval<
				FMatchPlayCurrentAttackRunnerSelectionAvailabilityResult>())
	DECLARE_PUBLIC_MEMBER_TRAIT(
		THasPublicSetAttackSequence,
		std::declval<T&>().SetAttackSequence(int64{}))
	DECLARE_PUBLIC_MEMBER_TRAIT(
		THasPublicSetReason,
		std::declval<T&>().SetReason(
			EMatchPlayRunnerNoSelectionNoGoalReason::None))
	DECLARE_PUBLIC_MEMBER_TRAIT(
		THasPublicSetSource,
		std::declval<T&>().SetSource(
			EMatchPlayRunnerNoSelectionNoGoalSource::None))
	DECLARE_PUBLIC_MEMBER_TRAIT(
		THasPublicSetAuthorityResult,
		std::declval<T&>().SetAuthorityResult(
			std::declval<
				const FMatchPlayCurrentAttackRunnerSelectionAvailabilityResult&>()))
	DECLARE_PUBLIC_MEMBER_TRAIT(
		THasPublicMutableAuthorityGetter,
		std::declval<T&>().GetAuthorityResult().bQuerySucceeded = true)
	DECLARE_PUBLIC_MEMBER_TRAIT(
		THasPublicCreate,
		&T::Create)
	DECLARE_PUBLIC_MEMBER_TRAIT(
		THasPublicCompleteMarkerGoal,
		&T::CompleteMarkerGoal)
	DECLARE_PUBLIC_MEMBER_TRAIT(
		THasPublicCompleteSkillNoGoal,
		&T::CompleteSkillNoGoal)
	DECLARE_PUBLIC_MEMBER_TRAIT(
		THasPublicCompleteRunnerNoGoal,
		&T::CompleteRunnerNoGoal)
	DECLARE_PUBLIC_MEMBER_TRAIT(
		THasPublicTerminalMutation,
		&T::ApplyCurrentAttackTerminalMutation)

#undef DECLARE_PUBLIC_MEMBER_TRAIT

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

	FString ExtractFunction(
		const FString& Source,
		const TCHAR* StartToken,
		const TCHAR* EndToken)
	{
		const int32 Start = Source.Find(StartToken);
		if (Start == INDEX_NONE)
		{
			return FString();
		}
		const int32 End = Source.Find(
			EndToken,
			ESearchCase::CaseSensitive,
			ESearchDir::FromStart,
			Start + 1);
		return End == INDEX_NONE
			? Source.Mid(Start)
			: Source.Mid(Start, End - Start);
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FRunnerNoSelectionNoGoalCapabilityAuthorityTest,
	"FMCodex.CoreRules.MatchPlayRunnerNoSelectionNoGoal.Authority.CapabilityAndPublicContracts",
	EAutomationTestFlags::EditorContext
		| EAutomationTestFlags::EngineFilter)

bool FRunnerNoSelectionNoGoalCapabilityAuthorityTest::RunTest(
	const FString& Parameters)
{
	using namespace RunnerNoSelectionNoGoalAuthorityTests;
	using FCapability =
		FMatchPlayRunnerNoSelectionNoGoalCapability;
	using FResolveSignature =
		FMatchPlayResolveNoLegalRunnerResult (*)(
			const FMatchPlayState&,
			const FMatchPlayResolveNoLegalRunnerRequest&);
	using FDeclineSignature =
		FMatchPlayRunnerDeclineResult (*)(
			const FMatchPlayState&,
			const FMatchPlayRunnerDeclineRequest&);

	static_assert(!std::is_default_constructible_v<FCapability>);
	static_assert(!std::is_aggregate_v<FCapability>);
	static_assert(!std::is_copy_constructible_v<FCapability>);
	static_assert(!std::is_move_constructible_v<FCapability>);
	static_assert(!std::is_copy_assignable_v<FCapability>);
	static_assert(!std::is_move_assignable_v<FCapability>);
	static_assert(!THasStaticStruct<FCapability>::value);
	static_assert(
		!std::is_constructible_v<
			FCapability,
			int64,
			EMatchPlayRunnerNoSelectionNoGoalSource,
			EMatchPlayRunnerNoSelectionNoGoalReason,
			const FMatchPlayCurrentAttackRunnerSelectionAvailabilityResult&>);
	static_assert(
		!THasPublicAttackSequenceField<FCapability>::value
			&& !THasPublicReasonField<FCapability>::value
			&& !THasPublicSourceField<FCapability>::value
			&& !THasPublicAuthorityResultField<FCapability>::value);
	static_assert(
		!THasPublicSetAttackSequence<FCapability>::value
			&& !THasPublicSetReason<FCapability>::value
			&& !THasPublicSetSource<FCapability>::value
			&& !THasPublicSetAuthorityResult<FCapability>::value
			&& !THasPublicMutableAuthorityGetter<FCapability>::value
			&& !THasPublicCreate<FCapability>::value);
	static_assert(
		!THasPublicCompleteMarkerGoal<
			FMatchPlayCurrentAttackCompletion>::value
			&& !THasPublicCompleteSkillNoGoal<
				FMatchPlayCurrentAttackCompletion>::value
			&& !THasPublicCompleteRunnerNoGoal<
				FMatchPlayCurrentAttackCompletion>::value
			&& !THasPublicTerminalMutation<
				FMatchPlayCurrentAttackCompletion>::value);

	TestTrue(TEXT("Resolve has one exact public signature"),
		(std::is_same_v<
			decltype(&FMatchPlayResolveNoLegalRunner::Resolve),
			FResolveSignature>));
	TestTrue(TEXT("Decline has one exact public signature"),
		(std::is_same_v<
			decltype(&FMatchPlayRunnerDecline::Decline),
			FDeclineSignature>));
	TestFalse(TEXT("Capability is not reflected"),
		THasStaticStruct<FCapability>::value);
	TestFalse(TEXT("Capability is not default constructible"),
		std::is_default_constructible_v<FCapability>);
	TestFalse(TEXT("Capability is not aggregate"),
		std::is_aggregate_v<FCapability>);
	TestFalse(TEXT("Capability is neither copyable nor movable"),
		std::is_copy_constructible_v<FCapability>
			|| std::is_move_constructible_v<FCapability>
			|| std::is_copy_assignable_v<FCapability>
			|| std::is_move_assignable_v<FCapability>);
	TestFalse(TEXT("Capability has no mutable authority getter"),
		THasPublicMutableAuthorityGetter<FCapability>::value);
	TestFalse(TEXT("Completion has no public callable entry"),
		THasPublicCompleteMarkerGoal<
			FMatchPlayCurrentAttackCompletion>::value
			|| THasPublicCompleteSkillNoGoal<
				FMatchPlayCurrentAttackCompletion>::value
			|| THasPublicCompleteRunnerNoGoal<
				FMatchPlayCurrentAttackCompletion>::value
			|| THasPublicTerminalMutation<
				FMatchPlayCurrentAttackCompletion>::value);

	const UScriptStruct* ResolveRequest =
		FMatchPlayResolveNoLegalRunnerRequest::StaticStruct();
	int32 ResolveFieldCount = 0;
	for (TFieldIterator<FProperty> It(ResolveRequest); It; ++It)
	{
		++ResolveFieldCount;
	}
	TestEqual(TEXT("Resolve request has one field"),
		ResolveFieldCount, 1);
	TestNotNull(TEXT("Resolve request AttackSequence"),
		ResolveRequest->FindPropertyByName(TEXT("AttackSequence")));

	const UScriptStruct* DeclineRequest =
		FMatchPlayRunnerDeclineRequest::StaticStruct();
	int32 DeclineFieldCount = 0;
	for (TFieldIterator<FProperty> It(DeclineRequest); It; ++It)
	{
		++DeclineFieldCount;
	}
	TestEqual(TEXT("Decline request has two fields"),
		DeclineFieldCount, 2);
	TestNotNull(TEXT("Decline request AttackSequence"),
		DeclineRequest->FindPropertyByName(TEXT("AttackSequence")));
	TestNotNull(TEXT("Decline request RequestingSide"),
		DeclineRequest->FindPropertyByName(TEXT("RequestingSide")));

	const TArray<FName> ForbiddenRequestFields = {
		TEXT("RunnerCardId"),
		TEXT("CarrierCardId"),
		TEXT("MarkerCardId"),
		TEXT("SkillId"),
		TEXT("bDecline"),
		TEXT("Availability"),
		TEXT("RunnerAvailabilityResult"),
		TEXT("NoGoal"),
		TEXT("Score"),
		TEXT("Outcome"),
		TEXT("Capability")
	};
	for (const FName Field : ForbiddenRequestFields)
	{
		TestNull(TEXT("Resolve request has no authority field"),
			ResolveRequest->FindPropertyByName(Field));
		TestNull(TEXT("Decline request has no authority field"),
			DeclineRequest->FindPropertyByName(Field));
	}

	for (const UScriptStruct* ResultStruct :
		{FMatchPlayResolveNoLegalRunnerResult::StaticStruct(),
			FMatchPlayRunnerDeclineResult::StaticStruct()})
	{
		TestNull(TEXT("Public result exposes no capability"),
			ResultStruct->FindPropertyByName(TEXT("Capability")));
		TestNull(TEXT("Public result exposes no token"),
			ResultStruct->FindPropertyByName(TEXT("CapabilityToken")));
		TestNull(TEXT("Public result exposes no outcome"),
			ResultStruct->FindPropertyByName(TEXT("Outcome")));
		TestNotNull(TEXT("Public result includes availability"),
			ResultStruct->FindPropertyByName(
				TEXT("RunnerAvailabilityResult")));
		TestNotNull(TEXT("Public result includes completion"),
			ResultStruct->FindPropertyByName(
				TEXT("CompletionResult")));
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FRunnerNoSelectionNoGoalStaticAuthorityTest,
	"FMCodex.CoreRules.MatchPlayRunnerNoSelectionNoGoal.Authority.StaticIsolation",
	EAutomationTestFlags::EditorContext
		| EAutomationTestFlags::EngineFilter)

bool FRunnerNoSelectionNoGoalStaticAuthorityTest::RunTest(
	const FString& Parameters)
{
	using namespace RunnerNoSelectionNoGoalAuthorityTests;
	const FString EntrySource =
		Load(TEXT("MatchPlayRunnerNoSelectionNoGoal.cpp"));
	const FString CapabilityHeader =
		Load(TEXT("MatchPlayRunnerNoSelectionNoGoalCapability.h"));
	const FString CompletionHeader =
		Load(TEXT("MatchPlayCurrentAttackCompletion.h"));
	const FString CompletionSource =
		Load(TEXT("MatchPlayCurrentAttackCompletion.cpp"));
	const FString ResolveBody = ExtractFunction(
		EntrySource,
		TEXT("FMatchPlayResolveNoLegalRunner::Resolve"),
		TEXT("FMatchPlayRunnerDeclineResult FMatchPlayRunnerDecline::Decline"));
	const FString DeclineBody = ExtractFunction(
		EntrySource,
		TEXT("FMatchPlayRunnerDecline::Decline"),
		TEXT("__END_OF_FILE__"));
	const FString RunnerOuterBody = ExtractFunction(
		CompletionSource,
		TEXT("FMatchPlayCurrentAttackCompletion::CompleteRunnerNoGoal"),
		TEXT("FMatchPlayCurrentAttackCompletion\n\t::ApplyCurrentAttackTerminalMutation"));

	TestEqual(TEXT("Two issuer tags exist"),
		CountOccurrences(CapabilityHeader, TEXT("IssuerTag final")),
		2);
	TestTrue(TEXT("Resolve uses only Resolve issuer tag"),
		ResolveBody.Contains(TEXT("FResolveNoLegalRunnerIssuerTag"))
			&& !ResolveBody.Contains(TEXT("FRunnerDeclineIssuerTag")));
	TestTrue(TEXT("Decline uses only Decline issuer tag"),
		DeclineBody.Contains(TEXT("FRunnerDeclineIssuerTag"))
			&& !DeclineBody.Contains(
				TEXT("FResolveNoLegalRunnerIssuerTag")));
	TestFalse(TEXT("Capability has no arbitrary raw Source parameter"),
		CapabilityHeader.Contains(TEXT("InSource"))
			|| CapabilityHeader.Contains(TEXT("ArbitrarySource")));
	TestFalse(TEXT("Capability has no arbitrary raw Reason parameter"),
		CapabilityHeader.Contains(TEXT("InReason"))
			|| CapabilityHeader.Contains(TEXT("ArbitraryReason")));

	TestEqual(TEXT("Each Runner entry has one Availability call"),
		CountOccurrences(
			EntrySource,
			TEXT("FMatchPlayCurrentAttackRunnerSelectionAvailability::Query")),
		2);
	TestEqual(TEXT("Completion has zero Runner Availability calls"),
		CountOccurrences(
			CompletionSource,
			TEXT("FMatchPlayCurrentAttackRunnerSelectionAvailability::Query")),
		0);
	TestEqual(TEXT("One private Marker outer declaration"),
		CountOccurrences(
			CompletionHeader,
			TEXT("CompleteMarkerGoal(")),
		1);
	TestEqual(TEXT("One private Skill outer declaration"),
		CountOccurrences(
			CompletionHeader,
			TEXT("CompleteSkillNoGoal(")),
		1);
	TestEqual(TEXT("One private Runner outer declaration"),
		CountOccurrences(
			CompletionHeader,
			TEXT("CompleteRunnerNoGoal(")),
		1);
	TestEqual(TEXT("One private shared terminal core declaration"),
		CountOccurrences(
			CompletionHeader,
			TEXT("ApplyCurrentAttackTerminalMutation(")),
		1);
	TestTrue(TEXT("Runner-aware stage diagnostic exists"),
		CompletionSource.Contains(
			TEXT("Runner no-selection completion requires AwaitingRunner stage.")));
	TestEqual(TEXT("GoalResolver remains bounded to Marker and ThroughBall Goal"),
		CountOccurrences(
			CompletionSource,
			TEXT("FGoalResolver::RecordGoal")),
		2);
	TestFalse(TEXT("Runner entry never calls GoalResolver"),
		EntrySource.Contains(TEXT("GoalResolver")));
	TestFalse(TEXT("Runner outer never calls GoalResolver"),
		RunnerOuterBody.Contains(TEXT("GoalResolver")));
	TestFalse(TEXT("Runner outer never writes Score"),
		RunnerOuterBody.Contains(TEXT(".Score ="))
			|| RunnerOuterBody.Contains(TEXT(".Score +=")));
	TestEqual(TEXT("Shared ordinary CardUsage loop remains unique"),
		CountOccurrences(
			CompletionSource,
			TEXT("FPlayCardResolver::PlayCard")),
		1);
	TestEqual(TEXT("Opportunity resolver remains unique"),
		CountOccurrences(
			CompletionSource,
			TEXT("FAttackOpportunityResolver::ConsumeCurrentAttackOpportunity")),
		1);
	TestEqual(TEXT("MatchEnd resolver remains unique"),
		CountOccurrences(
			CompletionSource,
			TEXT("FMatchEndResolver::ResolveMatchEnd")),
		2);
	TestEqual(TEXT("MatchResult resolver remains unique"),
		CountOccurrences(
			CompletionSource,
			TEXT("FMatchResultResolver::ResolveMatchResult")),
		1);
	TestFalse(TEXT("No old MatchPlayAttackFlow reuse"),
		EntrySource.Contains(TEXT("MatchPlayAttackFlow"))
			|| CompletionSource.Contains(
				TEXT("MatchPlayAttackFlow")));
	TestFalse(TEXT("No automatic Begin"),
		EntrySource.Contains(TEXT("BeginOrdinaryAttack"))
			|| CompletionSource.Contains(
				TEXT("BeginOrdinaryAttack")));
	TestFalse(TEXT("No direct Score writes in Runner entry"),
		EntrySource.Contains(TEXT(".Score ="))
			|| EntrySource.Contains(TEXT(".Score +=")));
	TestFalse(TEXT("No helper implementation in Runner entry"),
		EntrySource.Contains(TEXT("HelperCardId"))
			|| EntrySource.Contains(TEXT("AwaitingHelper"))
			|| EntrySource.Contains(TEXT("ReadyForResolution")));
	return true;
}

#endif
