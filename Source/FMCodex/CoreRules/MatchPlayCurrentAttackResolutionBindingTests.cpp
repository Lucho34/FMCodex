#include "MatchPlayCurrentAttackResolutionBinding.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "MatchPlayCurrentAttackActionSelectionWriter.h"
#include "Misc/AutomationTest.h"

#include <type_traits>

namespace FMCodex::Tests::CurrentAttackActionSelection
{
	FSkillRuleSnapshotSet MakeSkillRules();
	FMatchPlayState MakeState();
	FMatchPlayCurrentAttackActionSelectionRequest MakeRequest(
		FName CarrierCardId,
		FName SkillId);
	bool AreStatesEqual(
		const FMatchPlayState& Left,
		const FMatchPlayState& Right);
}

namespace FMCodex::Tests::CurrentAttackResolutionBinding
{
	namespace Foundation =
		FMCodex::Tests::CurrentAttackActionSelection;

	constexpr int64 ValidAttackSequence = 11;
	const FName CarrierId(TEXT("PlayerA.CarrierOne"));
	const FName SkillId(TEXT("Skill.LongShot"));

	FMatchPlayState MakeUnselectedState()
	{
		return Foundation::MakeState();
	}

	FMatchPlayState MakeSelectedState()
	{
		const auto WriterResult =
			FMatchPlayCurrentAttackActionSelectionWriter::Select(
				Foundation::MakeState(),
				Foundation::MakeRequest(CarrierId, SkillId),
				Foundation::MakeSkillRules());
		return WriterResult.AfterState;
	}

	bool AreStatesEqual(
		const FMatchPlayState& Left,
		const FMatchPlayState& Right)
	{
		return Foundation::AreStatesEqual(Left, Right);
	}

	bool ExpectBindingFailure(
		FAutomationTestBase& Test,
		const TCHAR* Context,
		const FMatchPlayState& State,
		const int64 AttackSequence,
		const EMatchPlayCurrentAttackResolutionBindingErrorCode
			ExpectedError)
	{
		const FMatchPlayState OriginalState = State;
		const auto Result =
			FMatchPlayCurrentAttackResolutionBinding::Query(
				State,
				AttackSequence);
		Test.TestFalse(
			*FString::Printf(TEXT("%s fails"), Context),
			Result.bSuccess);
		Test.TestEqual(
			*FString::Printf(TEXT("%s returns exact error"), Context),
			Result.ErrorCode,
			ExpectedError);
		Test.TestTrue(
			*FString::Printf(TEXT("%s has diagnostics"), Context),
			!Result.ErrorMessage.IsEmpty());
		Test.TestTrue(
			*FString::Printf(TEXT("%s is read-only"), Context),
			AreStatesEqual(State, OriginalState));
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
	TestEqual(TEXT("Default requested sequence is zero"),
		Result.RequestedAttackSequence, int64{0});
	TestNotNull(TEXT("Binding value is reflected"),
		FMatchPlayCurrentAttackResolutionBindingValue::StaticStruct());
	TestNotNull(TEXT("Binding result is reflected"),
		FMatchPlayCurrentAttackResolutionBindingResult::StaticStruct());
	using FQuerySignature =
		FMatchPlayCurrentAttackResolutionBindingResult (*)(
			const FMatchPlayState&,
			int64);
	TestTrue(TEXT("Query signature frozen"),
		(std::is_same_v<
			decltype(&FMatchPlayCurrentAttackResolutionBinding::Query),
			FQuerySignature>));
	return true;
}

RESOLUTION_BINDING_TEST(
	FResolutionBindingUnselectedTest,
	"CanonicalEmptyIsNotReady")

bool FResolutionBindingUnselectedTest::RunTest(
	const FString& Parameters)
{
	using namespace
		FMCodex::Tests::CurrentAttackResolutionBinding;
	return ExpectBindingFailure(
		*this,
		TEXT("Canonical empty action"),
		MakeUnselectedState(),
		ValidAttackSequence,
		EMatchPlayCurrentAttackResolutionBindingErrorCode
			::ActionNotSelected);
}

RESOLUTION_BINDING_TEST(
	FResolutionBindingWriterSuccessTest,
	"WriterSelectionBindsFrozenIdentity")

bool FResolutionBindingWriterSuccessTest::RunTest(
	const FString& Parameters)
{
	using namespace
		FMCodex::Tests::CurrentAttackResolutionBinding;
	const FMatchPlayState State = MakeSelectedState();
	const FMatchPlayState OriginalState = State;
	const auto Result =
		FMatchPlayCurrentAttackResolutionBinding::Query(
			State,
			ValidAttackSequence);
	TestTrue(TEXT("Binding succeeds"), Result.bSuccess);
	TestEqual(TEXT("Error is clean"), Result.ErrorCode,
		EMatchPlayCurrentAttackResolutionBindingErrorCode::None);
	TestEqual(TEXT("Sequence bound"), Result.Binding.AttackSequence,
		ValidAttackSequence);
	TestEqual(TEXT("Carrier bound"), Result.Binding.CarrierCardId,
		CarrierId);
	TestEqual(TEXT("Skill bound"), Result.Binding.SkillId, SkillId);
	TestEqual(TEXT("Action type bound"), Result.Binding.ActionType,
		ESkillRuleType::LongShot);
	TestTrue(TEXT("Binding is read-only"),
		AreStatesEqual(State, OriginalState));
	return true;
}

RESOLUTION_BINDING_TEST(
	FResolutionBindingRepeatedTest,
	"RepeatedQueryIsStableAndReadOnly")

bool FResolutionBindingRepeatedTest::RunTest(
	const FString& Parameters)
{
	using namespace
		FMCodex::Tests::CurrentAttackResolutionBinding;
	const FMatchPlayState State = MakeSelectedState();
	const FMatchPlayState OriginalState = State;
	const auto First =
		FMatchPlayCurrentAttackResolutionBinding::Query(
			State,
			ValidAttackSequence);
	const auto Second =
		FMatchPlayCurrentAttackResolutionBinding::Query(
			State,
			ValidAttackSequence);
	TestTrue(TEXT("Both queries succeed"),
		First.bSuccess && Second.bSuccess);
	TestTrue(TEXT("Binding values are identical"),
		FMatchPlayCurrentAttackResolutionBindingValue::StaticStruct()
			->CompareScriptStruct(
				&First.Binding,
				&Second.Binding,
				0));
	TestTrue(TEXT("State remains unchanged"),
		AreStatesEqual(State, OriginalState));
	return true;
}

#define SIMPLE_BINDING_FAILURE( \
	TestClass, TestName, StateFactory, StateMutation, Sequence, \
	ExpectedError) \
	RESOLUTION_BINDING_TEST(TestClass, TestName) \
	bool TestClass::RunTest(const FString& Parameters) \
	{ \
		using namespace \
			FMCodex::Tests::CurrentAttackResolutionBinding; \
		FMatchPlayState State = StateFactory; \
		StateMutation; \
		return ExpectBindingFailure( \
			*this, TEXT(TestName), State, Sequence, ExpectedError); \
	}

SIMPLE_BINDING_FAILURE(
	FResolutionBindingUninitializedTest,
	"RejectsUninitializedState",
	MakeSelectedState(),
	State.RuntimeState.bIsInitialized = false,
	0,
	EMatchPlayCurrentAttackResolutionBindingErrorCode
		::MatchPlayStateNotInitialized)

SIMPLE_BINDING_FAILURE(
	FResolutionBindingNoAttackTest,
	"RejectsMissingCurrentAttack",
	MakeSelectedState(),
	State.bHasCurrentAttack = false,
	0,
	EMatchPlayCurrentAttackResolutionBindingErrorCode::NoCurrentAttack)

SIMPLE_BINDING_FAILURE(
	FResolutionBindingInvalidSequenceTest,
	"RejectsInvalidAuthoritativeSequence",
	MakeSelectedState(),
	State.CurrentAttack.AttackSequence = 0,
	0,
	EMatchPlayCurrentAttackResolutionBindingErrorCode
		::InvalidCurrentAttackSequence)

SIMPLE_BINDING_FAILURE(
	FResolutionBindingStaleTest,
	"RejectsStaleSequence",
	MakeSelectedState(),
	State.RuntimeState.bIsInitialized = true,
	ValidAttackSequence + 1,
	EMatchPlayCurrentAttackResolutionBindingErrorCode
		::AttackSequenceMismatch)

SIMPLE_BINDING_FAILURE(
	FResolutionBindingPhaseTest,
	"RejectsNonResolutionPhase",
	MakeSelectedState(),
	State.CurrentAttack.Phase =
		EMatchPlayCurrentAttackPhase::Deployment,
	ValidAttackSequence,
	EMatchPlayCurrentAttackResolutionBindingErrorCode
		::CurrentAttackNotInResolution)

SIMPLE_BINDING_FAILURE(
	FResolutionBindingCorruptEmptyTest,
	"RejectsCorruptUnselectedPayload",
	MakeUnselectedState(),
	State.CurrentAttack.SelectedAction.SkillId = SkillId,
	ValidAttackSequence,
	EMatchPlayCurrentAttackResolutionBindingErrorCode
		::InvalidSelectedActionState)

SIMPLE_BINDING_FAILURE(
	FResolutionBindingMissingCarrierTest,
	"RejectsSelectedPayloadWithoutCarrier",
	MakeSelectedState(),
	State.CurrentAttack.SelectedAction.CarrierCardId = NAME_None,
	ValidAttackSequence,
	EMatchPlayCurrentAttackResolutionBindingErrorCode
		::InvalidSelectedActionState)

SIMPLE_BINDING_FAILURE(
	FResolutionBindingNoneTypeTest,
	"RejectsNoneActionType",
	MakeSelectedState(),
	State.CurrentAttack.SelectedAction.ActionType =
		ESkillRuleType::None,
	ValidAttackSequence,
	EMatchPlayCurrentAttackResolutionBindingErrorCode
		::UnsupportedActionType)

SIMPLE_BINDING_FAILURE(
	FResolutionBindingUnknownTypeTest,
	"RejectsUnknownActionType",
	MakeSelectedState(),
	State.CurrentAttack.SelectedAction.ActionType =
		static_cast<ESkillRuleType>(255),
	ValidAttackSequence,
	EMatchPlayCurrentAttackResolutionBindingErrorCode
		::UnsupportedActionType)

#undef SIMPLE_BINDING_FAILURE
#undef RESOLUTION_BINDING_TEST

#endif
