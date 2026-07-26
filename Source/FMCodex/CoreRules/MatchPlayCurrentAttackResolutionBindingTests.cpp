#include "MatchPlayCurrentAttackResolutionBinding.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "MatchPlayCurrentAttackCarrierSelectionTestFixtures.h"
#include "MatchPlayCurrentAttackCarrierSelectionWriter.h"
#include "Misc/AutomationTest.h"

#include <type_traits>

namespace CurrentAttackResolutionBindingTests
{
	using namespace
		FMCodex::Tests::MatchPlayCurrentAttackCarrierSelection;

	bool ExpectFailure(
		FAutomationTestBase& Test,
		const TCHAR* Context,
		const FMatchPlayState& State,
		const EMatchPlayCurrentAttackResolutionBindingErrorCode
			ExpectedError)
	{
		const FMatchPlayState Original = State;
		const auto Result =
			FMatchPlayCurrentAttackResolutionBinding::Query(
				State,
				ValidAttackSequence);
		Test.TestFalse(
			*FString::Printf(TEXT("%s fails"), Context),
			Result.bSuccess);
		Test.TestEqual(
			*FString::Printf(TEXT("%s exact error"), Context),
			Result.ErrorCode,
			ExpectedError);
		Test.TestTrue(
			*FString::Printf(TEXT("%s has diagnostics"), Context),
			!Result.ErrorMessage.IsEmpty());
		Test.TestTrue(
			*FString::Printf(TEXT("%s is read-only"), Context),
			AreStatesEqual(State, Original));
		return true;
	}
}

#define RESOLUTION_BINDING_TEST(TestClass, TestName) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
		TestClass, \
		"FMCodex.CoreRules.MatchPlayCurrentAttackResolutionBinding." TestName, \
		EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

RESOLUTION_BINDING_TEST(
	FResolutionBindingContractTest,
	"DefaultsReflectionAndSignature")

bool FResolutionBindingContractTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayCurrentAttackResolutionBindingResult Result;
	TestFalse(TEXT("Default result fails"), Result.bSuccess);
	TestNotNull(TEXT("Binding reflected"),
		FMatchPlayCurrentAttackResolutionBindingValue::StaticStruct());
	TestNotNull(TEXT("Result reflected"),
		FMatchPlayCurrentAttackResolutionBindingResult::StaticStruct());
	using FQuerySignature =
		FMatchPlayCurrentAttackResolutionBindingResult (*)(
			const FMatchPlayState&,
			int64);
	TestTrue(TEXT("Query signature stable"),
		(std::is_same_v<
			decltype(&FMatchPlayCurrentAttackResolutionBinding::Query),
			FQuerySignature>));
	return true;
}

RESOLUTION_BINDING_TEST(
	FResolutionBindingAwaitingCarrierTest,
	"AwaitingCarrierRejected")

bool FResolutionBindingAwaitingCarrierTest::RunTest(
	const FString& Parameters)
{
	using namespace CurrentAttackResolutionBindingTests;
	const FMatchPlayState State = MakeState();
	const auto Result =
		FMatchPlayCurrentAttackResolutionBinding::Query(
			State,
			ValidAttackSequence);
	TestFalse(TEXT("Binding fails"), Result.bSuccess);
	TestEqual(TEXT("Incomplete selection error"), Result.ErrorCode,
		EMatchPlayCurrentAttackResolutionBindingErrorCode
			::SelectionNotComplete);
	TestTrue(TEXT("Canonical state retained"),
		Result.SelectionStateValidationResult.bIsCanonical);
	TestTrue(TEXT("Binding carrier remains empty"),
		Result.Binding.CarrierCardId.IsNone());
	return true;
}

RESOLUTION_BINDING_TEST(
	FResolutionBindingAwaitingMarkerTest,
	"AwaitingMarkerRejectedWithoutLeakingCarrier")

bool FResolutionBindingAwaitingMarkerTest::RunTest(
	const FString& Parameters)
{
	using namespace CurrentAttackResolutionBindingTests;
	const auto WriterResult =
		FMatchPlayCurrentAttackCarrierSelectionWriter::Select(
			MakeState(),
			MakeRequest());
	TestTrue(TEXT("Carrier setup succeeds"), WriterResult.bSuccess);
	const auto Result =
		FMatchPlayCurrentAttackResolutionBinding::Query(
			WriterResult.AfterState,
			ValidAttackSequence);
	TestFalse(TEXT("Binding fails"), Result.bSuccess);
	TestEqual(TEXT("Incomplete selection error"), Result.ErrorCode,
		EMatchPlayCurrentAttackResolutionBindingErrorCode
			::SelectionNotComplete);
	TestTrue(TEXT("Preparation carrier is not returned"),
		Result.Binding.CarrierCardId.IsNone());
	TestTrue(TEXT("Skill remains empty"),
		Result.Binding.SkillId.IsNone());
	TestEqual(TEXT("ActionType remains None"),
		Result.Binding.ActionType, ESkillRuleType::None);
	return true;
}

RESOLUTION_BINDING_TEST(
	FResolutionBindingCorruptStateTest,
	"CorruptSelectionStateRejected")

bool FResolutionBindingCorruptStateTest::RunTest(
	const FString& Parameters)
{
	using namespace CurrentAttackResolutionBindingTests;
	FMatchPlayState State = MakeState();
	State.CurrentAttack.ActionPreparation.CarrierCardId = CarrierOneId;
	return ExpectFailure(
		*this,
		TEXT("Corrupt selection"),
		State,
		EMatchPlayCurrentAttackResolutionBindingErrorCode
			::InvalidSelectionState);
}

RESOLUTION_BINDING_TEST(
	FResolutionBindingLegacyPayloadTest,
	"LegacySelectedPayloadRejected")

bool FResolutionBindingLegacyPayloadTest::RunTest(
	const FString& Parameters)
{
	using namespace CurrentAttackResolutionBindingTests;
	FMatchPlayState State = MakeState();
	State.CurrentAttack.bHasSelectedAction = true;
	State.CurrentAttack.SelectedAction.CarrierCardId = CarrierOneId;
	State.CurrentAttack.SelectedAction.SkillId = TEXT("Skill.Legacy");
	State.CurrentAttack.SelectedAction.ActionType =
		ESkillRuleType::LongShot;
	return ExpectFailure(
		*this,
		TEXT("Legacy payload"),
		State,
		EMatchPlayCurrentAttackResolutionBindingErrorCode
			::InvalidSelectionState);
}

RESOLUTION_BINDING_TEST(
	FResolutionBindingRepeatedTest,
	"RepeatedQueryStableAndReadOnly")

bool FResolutionBindingRepeatedTest::RunTest(
	const FString& Parameters)
{
	using namespace CurrentAttackResolutionBindingTests;
	const FMatchPlayState State = MakeState();
	const FMatchPlayState Original = State;
	const auto First =
		FMatchPlayCurrentAttackResolutionBinding::Query(
			State,
			ValidAttackSequence);
	const auto Second =
		FMatchPlayCurrentAttackResolutionBinding::Query(
			State,
			ValidAttackSequence);
	TestTrue(TEXT("Results identical"),
		FMatchPlayCurrentAttackResolutionBindingResult
			::StaticStruct()->CompareScriptStruct(
				&First,
				&Second,
				0));
	TestTrue(TEXT("Input unchanged"), AreStatesEqual(State, Original));
	return true;
}

#undef RESOLUTION_BINDING_TEST

#endif
