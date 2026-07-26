#include "MatchPlayCurrentAttackCarrierSelectionWriter.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "MatchPlayCurrentAttackCarrierSelectionTestFixtures.h"
#include "Misc/AutomationTest.h"

#include <type_traits>

namespace CarrierSelectionWriterTests
{
	using namespace
		FMCodex::Tests::MatchPlayCurrentAttackCarrierSelection;

	bool ExpectFailure(
		FAutomationTestBase& Test,
		const TCHAR* Context,
		const FMatchPlayState& State,
		const FMatchPlayCurrentAttackCarrierSelectionRequest& Request,
		const EMatchPlayCurrentAttackCarrierSelectionErrorCode
			ExpectedLegalityError)
	{
		const FMatchPlayState Original = State;
		const auto Result =
			FMatchPlayCurrentAttackCarrierSelectionWriter::Select(
				State,
				Request);
		Test.TestFalse(
			*FString::Printf(TEXT("%s fails"), Context),
			Result.bSuccess);
		Test.TestEqual(
			*FString::Printf(TEXT("%s top-level error"), Context),
			Result.ErrorCode,
			EMatchPlayCurrentAttackCarrierSelectionWriterErrorCode
				::LegalityFailed);
		Test.TestEqual(
			*FString::Printf(TEXT("%s underlying error"), Context),
			Result.LegalityResult.ErrorCode,
			ExpectedLegalityError);
		Test.TestTrue(
			*FString::Printf(TEXT("%s BeforeState preserved"), Context),
			AreStatesEqual(Result.BeforeState, Original));
		Test.TestTrue(
			*FString::Printf(TEXT("%s AfterState atomic"), Context),
			AreStatesEqual(Result.AfterState, Original));
		Test.TestTrue(
			*FString::Printf(TEXT("%s input unchanged"), Context),
			AreStatesEqual(State, Original));
		return true;
	}
}

#define CARRIER_WRITER_TEST(TestClass, TestName) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
		TestClass, \
		"FMCodex.CoreRules.MatchPlayCurrentAttackCarrierSelection.Writer." TestName, \
		EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

CARRIER_WRITER_TEST(
	FCarrierWriterContractTest,
	"DefaultsReflectionAndSignature")

bool FCarrierWriterContractTest::RunTest(const FString& Parameters)
{
	const FMatchPlayCurrentAttackCarrierSelectionWriterResult Result;
	TestFalse(TEXT("Default fails"), Result.bSuccess);
	TestNotNull(TEXT("Result reflected"),
		FMatchPlayCurrentAttackCarrierSelectionWriterResult
			::StaticStruct());
	using FSelectSignature =
		FMatchPlayCurrentAttackCarrierSelectionWriterResult (*)(
			const FMatchPlayState&,
			const FMatchPlayCurrentAttackCarrierSelectionRequest&);
	TestTrue(TEXT("Single writer signature"),
		(std::is_same_v<
			decltype(
				&FMatchPlayCurrentAttackCarrierSelectionWriter::Select),
			FSelectSignature>));
	return true;
}

CARRIER_WRITER_TEST(
	FCarrierWriterSuccessTest,
	"FreezesCarrierAndEntersAwaitingMarker")

bool FCarrierWriterSuccessTest::RunTest(const FString& Parameters)
{
	using namespace
		FMCodex::Tests::MatchPlayCurrentAttackCarrierSelection;
	const FMatchPlayState BeforeState = MakeState();
	FMatchPlayState ExpectedState = BeforeState;
	ExpectedState.CurrentAttack.ActionPreparation.CarrierCardId =
		CarrierOneId;
	ExpectedState.CurrentAttack.SelectionStage =
		EMatchPlayCurrentAttackSelectionStage::AwaitingMarker;
	const auto Result =
		FMatchPlayCurrentAttackCarrierSelectionWriter::Select(
			BeforeState,
			MakeRequest());
	TestTrue(TEXT("Writer succeeds"), Result.bSuccess);
	TestTrue(TEXT("Legality succeeds"), Result.LegalityResult.bIsLegal);
	TestEqual(TEXT("Selected carrier echoed"),
		Result.SelectedCarrierCardId, CarrierOneId);
	TestTrue(TEXT("Only expected state fields change"),
		AreStatesEqual(Result.AfterState, ExpectedState));
	TestTrue(TEXT("Input state unchanged"),
		AreStatesEqual(Result.BeforeState, BeforeState));
	TestEqual(TEXT("Phase remains Resolution"),
		Result.AfterState.CurrentAttack.Phase,
		EMatchPlayCurrentAttackPhase::Resolution);
	TestTrue(TEXT("Marker preparation remains empty"),
		Result.AfterState.CurrentAttack.ActionPreparation
			.MarkerCardId.IsNone());
	TestFalse(TEXT("Final action remains absent"),
		Result.AfterState.CurrentAttack.bHasSelectedAction);
	const FMatchPlayCurrentAttackSelectedAction EmptySelectedAction;
	TestTrue(TEXT("Final selected payload remains empty"),
		FMatchPlayCurrentAttackSelectedAction::StaticStruct()
			->CompareScriptStruct(
				&Result.AfterState.CurrentAttack.SelectedAction,
				&EmptySelectedAction,
				0));
	return true;
}

#define CARRIER_WRITER_FAILURE_TEST( \
	TestClass, TestName, Mutation, ExpectedError) \
	CARRIER_WRITER_TEST(TestClass, TestName) \
	bool TestClass::RunTest(const FString& Parameters) \
	{ \
		using namespace \
			FMCodex::Tests::MatchPlayCurrentAttackCarrierSelection; \
		FMatchPlayState State = MakeState(); \
		FMatchPlayCurrentAttackCarrierSelectionRequest Request = \
			MakeRequest(); \
		Mutation; \
		return CarrierSelectionWriterTests::ExpectFailure( \
			*this, \
			TEXT(TestName), \
			State, \
			Request, \
			ExpectedError); \
	}

CARRIER_WRITER_FAILURE_TEST(
	FCarrierWriterStaleSequenceTest,
	"StaleSequenceAtomic",
	Request.AttackSequence = ValidAttackSequence + 1,
	EMatchPlayCurrentAttackCarrierSelectionErrorCode
		::AttackSequenceMismatch)

CARRIER_WRITER_FAILURE_TEST(
	FCarrierWriterWrongSideTest,
	"WrongSideAtomic",
	Request.RequestingSide = EInitialTurnOrderPlayer::PlayerB,
	EMatchPlayCurrentAttackCarrierSelectionErrorCode
		::RequestingSideIsNotCurrentAttacker)

CARRIER_WRITER_FAILURE_TEST(
	FCarrierWriterWrongStageSameCarrierTest,
	"SameCarrierRepeatRejected",
	State.CurrentAttack.ActionPreparation.CarrierCardId =
		CarrierOneId;
	State.CurrentAttack.SelectionStage =
		EMatchPlayCurrentAttackSelectionStage::AwaitingMarker,
	EMatchPlayCurrentAttackCarrierSelectionErrorCode
		::WrongSelectionStage)

CARRIER_WRITER_FAILURE_TEST(
	FCarrierWriterWrongStageDifferentCarrierTest,
	"DifferentCarrierReplacementRejected",
	State.CurrentAttack.ActionPreparation.CarrierCardId =
		CarrierOneId;
	State.CurrentAttack.SelectionStage =
		EMatchPlayCurrentAttackSelectionStage::AwaitingMarker;
	Request.CarrierCardId = CarrierTwoId,
	EMatchPlayCurrentAttackCarrierSelectionErrorCode
		::WrongSelectionStage)

CARRIER_WRITER_FAILURE_TEST(
	FCarrierWriterGoalkeeperTest,
	"GoalkeeperAtomic",
	State.CurrentAttack.DeploymentPlacements[0].CardId =
		GoalkeeperId;
	Request.CarrierCardId = GoalkeeperId,
	EMatchPlayCurrentAttackCarrierSelectionErrorCode
		::CarrierIsGoalkeeper)

CARRIER_WRITER_FAILURE_TEST(
	FCarrierWriterSnapshotFailureTest,
	"SnapshotFailureAtomic",
	State.CardSnapshotAuthority.PlayerACardSnapshots.Cards.RemoveAt(0),
	EMatchPlayCurrentAttackCarrierSelectionErrorCode
		::CarrierSnapshotLookupFailed)

CARRIER_WRITER_FAILURE_TEST(
	FCarrierWriterCorruptStateTest,
	"CorruptStateAtomic",
	State.CurrentAttack.ActionPreparation.CarrierCardId =
		CarrierOneId,
	EMatchPlayCurrentAttackCarrierSelectionErrorCode
		::InvalidSelectionState)

CARRIER_WRITER_TEST(
	FCarrierWriterAuthorityIsolationTest,
	"PreservesAllUnrelatedAuthorities")

bool FCarrierWriterAuthorityIsolationTest::RunTest(
	const FString& Parameters)
{
	using namespace
		FMCodex::Tests::MatchPlayCurrentAttackCarrierSelection;
	const FMatchPlayState BeforeState = MakeState();
	const auto Result =
		FMatchPlayCurrentAttackCarrierSelectionWriter::Select(
			BeforeState,
			MakeRequest());
	TestTrue(TEXT("Writer succeeds"), Result.bSuccess);
	TestTrue(TEXT("Runtime unchanged"),
		FMatchRuntimeState::StaticStruct()->CompareScriptStruct(
			&Result.AfterState.RuntimeState,
			&BeforeState.RuntimeState,
			0));
	TestTrue(TEXT("CardUsage unchanged"),
		FMatchCardUsageState::StaticStruct()->CompareScriptStruct(
			&Result.AfterState.CardUsageState,
			&BeforeState.CardUsageState,
			0));
	TestEqual(TEXT("Placement count unchanged"),
		Result.AfterState.CurrentAttack.DeploymentPlacements.Num(),
		BeforeState.CurrentAttack.DeploymentPlacements.Num());
	for (int32 Index = 0;
		Index
			< BeforeState.CurrentAttack.DeploymentPlacements.Num();
		++Index)
	{
		TestTrue(TEXT("Placement unchanged"),
			FMatchPlayDeploymentPlacement::StaticStruct()
				->CompareScriptStruct(
					&Result.AfterState.CurrentAttack
						.DeploymentPlacements[Index],
					&BeforeState.CurrentAttack
						.DeploymentPlacements[Index],
					0));
	}
	TestTrue(TEXT("GK authority unchanged"),
		FMatchPlayGoalkeeperUsageState::StaticStruct()
			->CompareScriptStruct(
				&Result.AfterState.GoalkeeperUsageState,
				&BeforeState.GoalkeeperUsageState,
				0));
	TestEqual(TEXT("GK activation unchanged"),
		Result.AfterState.CurrentAttack
			.bCurrentDefenseGoalkeeperActivated,
		BeforeState.CurrentAttack
			.bCurrentDefenseGoalkeeperActivated);
	TestEqual(TEXT("ActionPoint unchanged"),
		Result.AfterState.CurrentAttack.ActionPoint,
		BeforeState.CurrentAttack.ActionPoint);
	TestEqual(TEXT("PlayerA score unchanged"),
		Result.AfterState.RuntimeState.PlayerAState.Score,
		BeforeState.RuntimeState.PlayerAState.Score);
	TestEqual(TEXT("PlayerA used attacks unchanged"),
		Result.AfterState.RuntimeState.PlayerAState.UsedAttackCount,
		BeforeState.RuntimeState.PlayerAState.UsedAttackCount);
	return true;
}

CARRIER_WRITER_TEST(
	FCarrierWriterDeterministicTest,
	"RepeatedEvaluationStable")

bool FCarrierWriterDeterministicTest::RunTest(
	const FString& Parameters)
{
	using namespace
		FMCodex::Tests::MatchPlayCurrentAttackCarrierSelection;
	const FMatchPlayState State = MakeState();
	const auto First =
		FMatchPlayCurrentAttackCarrierSelectionWriter::Select(
			State,
			MakeRequest());
	const auto Second =
		FMatchPlayCurrentAttackCarrierSelectionWriter::Select(
			State,
			MakeRequest());
	TestTrue(TEXT("Results identical"),
		FMatchPlayCurrentAttackCarrierSelectionWriterResult
			::StaticStruct()->CompareScriptStruct(
				&First,
				&Second,
				0));
	return true;
}

#undef CARRIER_WRITER_FAILURE_TEST
#undef CARRIER_WRITER_TEST

#endif
