#include "MatchPlaySkillNoSelectionNoGoal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "UObject/UnrealType.h"

#include <type_traits>
#include <utility>

namespace SkillNoSelectionNoGoalAuthorityTests
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
			EMatchPlaySkillNoSelectionNoGoalReason::None)
	DECLARE_PUBLIC_MEMBER_TRAIT(
		THasPublicSourceField,
		std::declval<T&>().Source =
			EMatchPlaySkillNoSelectionNoGoalSource::None)
	DECLARE_PUBLIC_MEMBER_TRAIT(
		THasPublicAuthorityResultField,
		std::declval<T&>().AuthorityResult =
			std::declval<
				FMatchPlayCurrentAttackSkillSelectionAvailabilityResult>())
	DECLARE_PUBLIC_MEMBER_TRAIT(
		THasPublicSetAttackSequence,
		std::declval<T&>().SetAttackSequence(int64{}))
	DECLARE_PUBLIC_MEMBER_TRAIT(
		THasPublicSetReason,
		std::declval<T&>().SetReason(
			EMatchPlaySkillNoSelectionNoGoalReason::None))
	DECLARE_PUBLIC_MEMBER_TRAIT(
		THasPublicSetSource,
		std::declval<T&>().SetSource(
			EMatchPlaySkillNoSelectionNoGoalSource::None))
	DECLARE_PUBLIC_MEMBER_TRAIT(
		THasPublicSetAuthorityResult,
		std::declval<T&>().SetAuthorityResult(
			std::declval<
				const FMatchPlayCurrentAttackSkillSelectionAvailabilityResult&>()))
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
		const int32 End =
			Source.Find(
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
	FSkillNoSelectionNoGoalCapabilityAuthorityTest,
	"FMCodex.CoreRules.MatchPlaySkillNoSelectionNoGoal.Authority.CapabilityAndPublicContracts",
	EAutomationTestFlags::EditorContext
		| EAutomationTestFlags::EngineFilter)

bool FSkillNoSelectionNoGoalCapabilityAuthorityTest::RunTest(
	const FString& Parameters)
{
	using namespace SkillNoSelectionNoGoalAuthorityTests;
	using FCapability =
		FMatchPlaySkillNoSelectionNoGoalCapability;
	using FResolveSignature =
		FMatchPlayResolveNoLegalSkillResult (*)(
			const FMatchPlayState&,
			const FSkillRuleSnapshotSet&,
			const FMatchPlayResolveNoLegalSkillRequest&);
	using FDeclineSignature =
		FMatchPlaySkillDeclineResult (*)(
			const FMatchPlayState&,
			const FSkillRuleSnapshotSet&,
			const FMatchPlaySkillDeclineRequest&);

	static_assert(
		!std::is_default_constructible_v<FCapability>);
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
			EMatchPlaySkillNoSelectionNoGoalSource,
			EMatchPlaySkillNoSelectionNoGoalReason,
			const FMatchPlayCurrentAttackSkillSelectionAvailabilityResult&>);
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
			&& !THasPublicCreate<FCapability>::value);
	static_assert(
		!THasPublicCompleteMarkerGoal<
			FMatchPlayCurrentAttackCompletion>::value
			&& !THasPublicCompleteSkillNoGoal<
				FMatchPlayCurrentAttackCompletion>::value
			&& !THasPublicTerminalMutation<
				FMatchPlayCurrentAttackCompletion>::value);

	TestTrue(TEXT("Resolve has one exact public signature"),
		(std::is_same_v<
			decltype(&FMatchPlayResolveNoLegalSkill::Resolve),
			FResolveSignature>));
	TestTrue(TEXT("Decline has one exact public signature"),
		(std::is_same_v<
			decltype(&FMatchPlaySkillDecline::Decline),
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
	TestFalse(TEXT("Completion has no public callable entry"),
		THasPublicCompleteMarkerGoal<
			FMatchPlayCurrentAttackCompletion>::value
			|| THasPublicCompleteSkillNoGoal<
				FMatchPlayCurrentAttackCompletion>::value
			|| THasPublicTerminalMutation<
				FMatchPlayCurrentAttackCompletion>::value);

	const UScriptStruct* ResolveRequest =
		FMatchPlayResolveNoLegalSkillRequest::StaticStruct();
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
		FMatchPlaySkillDeclineRequest::StaticStruct();
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
		TEXT("SkillId"),
		TEXT("bDecline"),
		TEXT("Availability"),
		TEXT("SkillAvailabilityResult"),
		TEXT("NoGoal"),
		TEXT("Score"),
		TEXT("Outcome")
	};
	for (const FName Field : ForbiddenRequestFields)
	{
		TestNull(TEXT("Resolve request has no authority field"),
			ResolveRequest->FindPropertyByName(Field));
		TestNull(TEXT("Decline request has no authority field"),
			DeclineRequest->FindPropertyByName(Field));
	}

	for (const UScriptStruct* ResultStruct :
		{FMatchPlayResolveNoLegalSkillResult::StaticStruct(),
			FMatchPlaySkillDeclineResult::StaticStruct()})
	{
		TestNull(TEXT("Public result exposes no capability"),
			ResultStruct->FindPropertyByName(TEXT("Capability")));
		TestNull(TEXT("Public result exposes no token"),
			ResultStruct->FindPropertyByName(TEXT("CapabilityToken")));
		TestNull(TEXT("Public result exposes no outcome"),
			ResultStruct->FindPropertyByName(TEXT("Outcome")));
		TestNotNull(TEXT("Public result includes availability"),
			ResultStruct->FindPropertyByName(
				TEXT("SkillAvailabilityResult")));
		TestNotNull(TEXT("Public result includes completion"),
			ResultStruct->FindPropertyByName(
				TEXT("CompletionResult")));
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSkillNoSelectionNoGoalStaticAuthorityTest,
	"FMCodex.CoreRules.MatchPlaySkillNoSelectionNoGoal.Authority.StaticIsolation",
	EAutomationTestFlags::EditorContext
		| EAutomationTestFlags::EngineFilter)

bool FSkillNoSelectionNoGoalStaticAuthorityTest::RunTest(
	const FString& Parameters)
{
	using namespace SkillNoSelectionNoGoalAuthorityTests;
	const FString EntrySource =
		Load(TEXT("MatchPlaySkillNoSelectionNoGoal.cpp"));
	const FString CapabilityHeader =
		Load(TEXT("MatchPlaySkillNoSelectionNoGoalCapability.h"));
	const FString CompletionHeader =
		Load(TEXT("MatchPlayCurrentAttackCompletion.h"));
	const FString CompletionSource =
		Load(TEXT("MatchPlayCurrentAttackCompletion.cpp"));
	const FString AvailabilityHeader =
		Load(TEXT("MatchPlayCurrentAttackSkillSelectionAvailability.h"));
	const FString AvailabilitySource =
		Load(TEXT("MatchPlayCurrentAttackSkillSelectionAvailability.cpp"));
	const FString ResolveBody = ExtractFunction(
		EntrySource,
		TEXT("FMatchPlayResolveNoLegalSkill::Resolve"),
		TEXT("FMatchPlaySkillDeclineResult FMatchPlaySkillDecline::Decline"));
	const FString DeclineBody = ExtractFunction(
		EntrySource,
		TEXT("FMatchPlaySkillDecline::Decline"),
		TEXT("__END_OF_FILE__"));

	TestEqual(TEXT("Two issuer tags exist"),
		CountOccurrences(CapabilityHeader, TEXT("IssuerTag final")),
		2);
	TestTrue(TEXT("Resolve uses only Resolve issuer tag"),
		ResolveBody.Contains(TEXT("FResolveNoLegalSkillIssuerTag"))
			&& !ResolveBody.Contains(TEXT("FDeclineSkillIssuerTag")));
	TestTrue(TEXT("Decline uses only Decline issuer tag"),
		DeclineBody.Contains(TEXT("FDeclineSkillIssuerTag"))
			&& !DeclineBody.Contains(
				TEXT("FResolveNoLegalSkillIssuerTag")));
	TestFalse(TEXT("Capability has no arbitrary raw Source parameter"),
		CapabilityHeader.Contains(TEXT("InSource"))
			|| CapabilityHeader.Contains(TEXT("ArbitrarySource")));
	TestFalse(TEXT("Capability has no arbitrary raw Reason parameter"),
		CapabilityHeader.Contains(TEXT("InReason"))
			|| CapabilityHeader.Contains(TEXT("ArbitraryReason")));

	TestEqual(TEXT("Each Skill entry has one Availability call"),
		CountOccurrences(
			EntrySource,
			TEXT("FMatchPlayCurrentAttackSkillSelectionAvailability::Query")),
		2);
	TestEqual(TEXT("Completion has zero Skill Availability calls"),
		CountOccurrences(
			CompletionSource,
			TEXT("FMatchPlayCurrentAttackSkillSelectionAvailability::Query")),
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
	TestEqual(TEXT("One private shared advance core declaration"),
		CountOccurrences(
			CompletionHeader,
			TEXT("ApplyCurrentAttackAdvanceMutation(")),
		1);
	TestEqual(TEXT("GoalResolver has Marker, ThroughBall, Cross, PassControl, and Shot Goal call sites"),
		CountOccurrences(
			CompletionSource,
			TEXT("FGoalResolver::RecordGoal")),
		5);
	TestFalse(TEXT("Skill entry never calls GoalResolver"),
		EntrySource.Contains(TEXT("GoalResolver")));
	TestEqual(TEXT("Ordinary and Set Piece consumption each use PlayCard"),
		CountOccurrences(
			CompletionSource,
			TEXT("FPlayCardResolver::PlayCard")),
		2);
	TestEqual(TEXT("Set Piece participant extraction seam remains unique"),
		CountOccurrences(
			CompletionSource,
			TEXT("FMatchPlaySetPieceParticipantConsumption::Extract")),
		1);
	TestEqual(TEXT("Duplicate SlotId has one shared authority check"),
		CountOccurrences(
			CompletionSource,
			TEXT("SeenDeploymentSlots.Contains")),
		1);
	TestEqual(TEXT("Opportunity resolver has one call site"),
		CountOccurrences(
			CompletionSource,
			TEXT("FAttackOpportunityResolver::ConsumeCurrentAttackOpportunity")),
		1);
	TestEqual(TEXT("MatchResult resolver has one call site"),
		CountOccurrences(
			CompletionSource,
			TEXT("FMatchResultResolver::ResolveMatchResult")),
		1);
	TestFalse(TEXT("No legacy projection remains"),
		AvailabilityHeader.Contains(
			TEXT("bHasGlobalBlockingLegalityResult"))
			|| AvailabilityHeader.Contains(
				TEXT("GlobalBlockingLegalityResult"))
			|| AvailabilitySource.Contains(
				TEXT("GlobalBlockingLegalityResult")));
	TestFalse(TEXT("No old MatchPlayAttackFlow reuse"),
		EntrySource.Contains(TEXT("MatchPlayAttackFlow"))
			|| CompletionSource.Contains(
				TEXT("MatchPlayAttackFlow")));
	TestFalse(TEXT("No automatic Begin"),
		EntrySource.Contains(TEXT("BeginOrdinaryAttack"))
			|| CompletionSource.Contains(
				TEXT("BeginOrdinaryAttack")));
	TestFalse(TEXT("No direct Score writes in Skill entry"),
		EntrySource.Contains(TEXT(".Score ="))
			|| EntrySource.Contains(TEXT(".Score +=")));
	return true;
}

#endif
