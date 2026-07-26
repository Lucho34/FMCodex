#include "MatchPlayCurrentAttackResolutionBinding.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "MatchPlayCurrentAttackCarrierSelectionTestFixtures.h"
#include "MatchPlayCurrentAttackCarrierSelectionWriter.h"
#include "MatchPlayCurrentAttackSkillSelectionTestFixtures.h"
#include "MatchPlayCurrentAttackSkillSelectionWriter.h"
#include "Misc/AutomationTest.h"

#include <type_traits>

namespace SkillFixtures =
	FMCodex::Tests::MatchPlayCurrentAttackSkillSelection;

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
	TestNotNull(TEXT("Binding exposes final marker"),
		FMatchPlayCurrentAttackResolutionBindingValue::StaticStruct()
			->FindPropertyByName(TEXT("MarkerCardId")));
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
	FResolutionBindingAwaitingSkillTest,
	"AwaitingSkillRejectedWithoutLeakingPreparation")

bool FResolutionBindingAwaitingSkillTest::RunTest(
	const FString& Parameters)
{
	using namespace CurrentAttackResolutionBindingTests;
	FMatchPlayState State = MakeState();
	State.CurrentAttack.ActionPreparation.CarrierCardId =
		CarrierOneId;
	State.CurrentAttack.ActionPreparation.MarkerCardId =
		TEXT("PlayerB.Marker");
	State.CurrentAttack.SelectionStage =
		EMatchPlayCurrentAttackSelectionStage::AwaitingSkill;
	const FMatchPlayState Original = State;
	const auto Result =
		FMatchPlayCurrentAttackResolutionBinding::Query(
			State,
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
	TestTrue(TEXT("State unchanged"), AreStatesEqual(State, Original));
	return true;
}

RESOLUTION_BINDING_TEST(
	FResolutionBindingCorruptMarkerTest,
	"CorruptAwaitingSkillMarkerRejected")

bool FResolutionBindingCorruptMarkerTest::RunTest(
	const FString& Parameters)
{
	using namespace CurrentAttackResolutionBindingTests;
	FMatchPlayState State = MakeState();
	State.CurrentAttack.ActionPreparation.CarrierCardId =
		CarrierOneId;
	State.CurrentAttack.SelectionStage =
		EMatchPlayCurrentAttackSelectionStage::AwaitingSkill;
	return ExpectFailure(
		*this,
		TEXT("Corrupt AwaitingSkill marker"),
		State,
		EMatchPlayCurrentAttackResolutionBindingErrorCode
			::InvalidSelectionState);
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

RESOLUTION_BINDING_TEST(
	FResolutionBindingAwaitingRunnerTest,
	"AwaitingRunnerRejected")

bool FResolutionBindingAwaitingRunnerTest::RunTest(
	const FString& Parameters)
{
	const auto WriterResult =
		FMatchPlayCurrentAttackSkillSelectionWriter::Select(
			SkillFixtures::MakeState(
				{SkillFixtures::CrossSkillId}),
			SkillFixtures::MakeRuleSet(),
			SkillFixtures::MakeRequest(
				SkillFixtures::CrossSkillId));
	TestTrue(TEXT("Runner skill setup succeeds"), WriterResult.bSuccess);
	const FMatchPlayState Original = WriterResult.AfterState;
	const auto Result =
		FMatchPlayCurrentAttackResolutionBinding::Query(
			WriterResult.AfterState,
			SkillFixtures::ValidAttackSequence);
	TestFalse(TEXT("AwaitingRunner rejected"), Result.bSuccess);
	TestEqual(
		TEXT("AwaitingRunner incomplete"),
		Result.ErrorCode,
		EMatchPlayCurrentAttackResolutionBindingErrorCode
			::SelectionNotComplete);
	TestTrue(
		TEXT("AwaitingRunner state unchanged"),
		SkillFixtures::AreStatesEqual(
			WriterResult.AfterState,
			Original));
	return true;
}

RESOLUTION_BINDING_TEST(
	FResolutionBindingReadyTest,
	"ReadyLongShotAndCutInsideAccepted")

bool FResolutionBindingReadyTest::RunTest(
	const FString& Parameters)
{
	struct FCase
	{
		FName SkillId;
		ESkillRuleType Type;
	};
	const FCase Cases[] = {
		{
			SkillFixtures::LongShotSkillId,
			ESkillRuleType::LongShot
		},
		{
			SkillFixtures::CutInsideSkillId,
			ESkillRuleType::CutInsideShot
		}
	};
	for (const FCase& Case : Cases)
	{
		const auto WriterResult =
			FMatchPlayCurrentAttackSkillSelectionWriter::Select(
				SkillFixtures::MakeState({Case.SkillId}),
				SkillFixtures::MakeRuleSet(),
				SkillFixtures::MakeRequest(Case.SkillId));
		TestTrue(TEXT("Ready setup succeeds"), WriterResult.bSuccess);
		const FMatchPlayState Original = WriterResult.AfterState;
		const auto Result =
			FMatchPlayCurrentAttackResolutionBinding::Query(
				WriterResult.AfterState,
				SkillFixtures::ValidAttackSequence);
		TestTrue(TEXT("Ready binding succeeds"), Result.bSuccess);
		TestEqual(
			TEXT("Bound sequence"),
			Result.Binding.AttackSequence,
			SkillFixtures::ValidAttackSequence);
		TestEqual(
			TEXT("Bound carrier"),
			Result.Binding.CarrierCardId,
			SkillFixtures::CarrierId);
		TestEqual(
			TEXT("Bound marker"),
			Result.Binding.MarkerCardId,
			SkillFixtures::MarkerId);
		TestEqual(
			TEXT("Bound skill"),
			Result.Binding.SkillId,
			Case.SkillId);
		TestEqual(
			TEXT("Bound type"),
			Result.Binding.ActionType,
			Case.Type);
		TestTrue(
			TEXT("Ready state unchanged"),
			SkillFixtures::AreStatesEqual(
				WriterResult.AfterState,
				Original));
	}
	return true;
}

RESOLUTION_BINDING_TEST(
	FResolutionBindingReadyCorruptionTest,
	"ReadyCorruptionRejected")

bool FResolutionBindingReadyCorruptionTest::RunTest(
	const FString& Parameters)
{
	const auto WriterResult =
		FMatchPlayCurrentAttackSkillSelectionWriter::Select(
			SkillFixtures::MakeState(
				{SkillFixtures::LongShotSkillId}),
			SkillFixtures::MakeRuleSet(),
			SkillFixtures::MakeRequest());
	TestTrue(TEXT("Ready setup succeeds"), WriterResult.bSuccess);

	FMatchPlayState Preparation = WriterResult.AfterState;
	Preparation.CurrentAttack.ActionPreparation.SkillId =
		SkillFixtures::LongShotSkillId;
	const auto PreparationResult =
		FMatchPlayCurrentAttackResolutionBinding::Query(
			Preparation,
			SkillFixtures::ValidAttackSequence);
	TestFalse(
		TEXT("Ready preparation rejected"),
		PreparationResult.bSuccess);
	TestEqual(
		TEXT("Ready preparation invalid state"),
		PreparationResult.ErrorCode,
		EMatchPlayCurrentAttackResolutionBindingErrorCode
			::InvalidSelectionState);

	FMatchPlayState MissingMarker = WriterResult.AfterState;
	MissingMarker.CurrentAttack.SelectedAction.MarkerCardId =
		NAME_None;
	const auto MarkerResult =
		FMatchPlayCurrentAttackResolutionBinding::Query(
			MissingMarker,
			SkillFixtures::ValidAttackSequence);
	TestFalse(TEXT("Missing marker rejected"), MarkerResult.bSuccess);

	FMatchPlayState RunnerType = WriterResult.AfterState;
	RunnerType.CurrentAttack.SelectedAction.ActionType =
		ESkillRuleType::Cross;
	const auto RunnerResult =
		FMatchPlayCurrentAttackResolutionBinding::Query(
			RunnerType,
			SkillFixtures::ValidAttackSequence);
	TestFalse(TEXT("Ready runner type rejected"), RunnerResult.bSuccess);
	return true;
}

#undef RESOLUTION_BINDING_TEST

#endif
