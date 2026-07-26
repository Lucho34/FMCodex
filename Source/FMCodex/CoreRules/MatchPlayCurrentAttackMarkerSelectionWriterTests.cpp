#include "MatchPlayCurrentAttackMarkerSelectionWriter.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "MatchPlayCurrentAttackMarkerSelectionTestFixtures.h"
#include "Misc/AutomationTest.h"

#include <type_traits>

namespace MarkerSelectionWriterTests
{
	using namespace
		FMCodex::Tests::MatchPlayCurrentAttackMarkerSelection;

	bool ExpectFailure(
		FAutomationTestBase& Test,
		const TCHAR* Context,
		const FMatchPlayState& State,
		const FMatchPlayCurrentAttackMarkerSelectionRequest& Request,
		const EMatchPlayCurrentAttackMarkerSelectionErrorCode
			ExpectedLegalityError)
	{
		const FMatchPlayState Original = State;
		const auto Result =
			FMatchPlayCurrentAttackMarkerSelectionWriter::Select(
				State,
				Request);
		Test.TestFalse(
			*FString::Printf(TEXT("%s fails"), Context),
			Result.bSuccess);
		Test.TestEqual(
			*FString::Printf(TEXT("%s top-level error"), Context),
			Result.ErrorCode,
			EMatchPlayCurrentAttackMarkerSelectionWriterErrorCode
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

#define MARKER_WRITER_TEST(TestClass, TestName) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
		TestClass, \
		"FMCodex.CoreRules.MatchPlayCurrentAttackMarkerSelection.Writer." TestName, \
		EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

MARKER_WRITER_TEST(
	FMarkerWriterContractTest,
	"DefaultsReflectionAndSignature")

bool FMarkerWriterContractTest::RunTest(const FString& Parameters)
{
	const FMatchPlayCurrentAttackMarkerSelectionWriterResult Result;
	TestFalse(TEXT("Default fails"), Result.bSuccess);
	TestTrue(TEXT("Default selected marker empty"),
		Result.SelectedMarkerCardId.IsNone());
	TestNotNull(TEXT("Result reflected"),
		FMatchPlayCurrentAttackMarkerSelectionWriterResult
			::StaticStruct());
	using FSelectSignature =
		FMatchPlayCurrentAttackMarkerSelectionWriterResult (*)(
			const FMatchPlayState&,
			const FMatchPlayCurrentAttackMarkerSelectionRequest&);
	TestTrue(TEXT("Single writer signature"),
		(std::is_same_v<
			decltype(
				&FMatchPlayCurrentAttackMarkerSelectionWriter::Select),
			FSelectSignature>));
	return true;
}

MARKER_WRITER_TEST(
	FMarkerWriterSuccessTest,
	"FreezesMarkerAndEntersAwaitingSkill")

bool FMarkerWriterSuccessTest::RunTest(const FString& Parameters)
{
	using namespace
		FMCodex::Tests::MatchPlayCurrentAttackMarkerSelection;
	const FMatchPlayState BeforeState = MakeState();
	FMatchPlayState ExpectedState = BeforeState;
	ExpectedState.CurrentAttack.ActionPreparation.MarkerCardId =
		MarkerOneId;
	ExpectedState.CurrentAttack.SelectionStage =
		EMatchPlayCurrentAttackSelectionStage::AwaitingSkill;
	const auto Result =
		FMatchPlayCurrentAttackMarkerSelectionWriter::Select(
			BeforeState,
			MakeRequest());
	TestTrue(TEXT("Writer succeeds"), Result.bSuccess);
	TestTrue(TEXT("Legality succeeds"), Result.LegalityResult.bIsLegal);
	TestEqual(TEXT("Selected marker echoed"),
		Result.SelectedMarkerCardId, MarkerOneId);
	TestTrue(TEXT("Only marker and stage change"),
		AreStatesEqual(Result.AfterState, ExpectedState));
	TestTrue(TEXT("Input state unchanged"),
		AreStatesEqual(Result.BeforeState, BeforeState));
	TestEqual(TEXT("Carrier preserved"),
		Result.AfterState.CurrentAttack.ActionPreparation.CarrierCardId,
		CarrierId);
	TestEqual(TEXT("Phase remains Resolution"),
		Result.AfterState.CurrentAttack.Phase,
		EMatchPlayCurrentAttackPhase::Resolution);
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

#define MARKER_WRITER_FAILURE_TEST( \
	TestClass, TestName, Mutation, ExpectedError) \
	MARKER_WRITER_TEST(TestClass, TestName) \
	bool TestClass::RunTest(const FString& Parameters) \
	{ \
		using namespace \
			FMCodex::Tests::MatchPlayCurrentAttackMarkerSelection; \
		FMatchPlayState State = MakeState(); \
		FMatchPlayCurrentAttackMarkerSelectionRequest Request = \
			MakeRequest(); \
		Mutation; \
		return MarkerSelectionWriterTests::ExpectFailure( \
			*this, TEXT(TestName), State, Request, ExpectedError); \
	}

MARKER_WRITER_FAILURE_TEST(
	FMarkerWriterStaleSequenceTest,
	"StaleSequenceAtomic",
	Request.AttackSequence = ValidAttackSequence + 1,
	EMatchPlayCurrentAttackMarkerSelectionErrorCode
		::AttackSequenceMismatch)

MARKER_WRITER_FAILURE_TEST(
	FMarkerWriterWrongSideTest,
	"WrongSideAtomic",
	Request.RequestingSide = EInitialTurnOrderPlayer::PlayerA,
	EMatchPlayCurrentAttackMarkerSelectionErrorCode
		::RequestingSideIsNotCurrentDefender)

MARKER_WRITER_FAILURE_TEST(
	FMarkerWriterWrongStageTest,
	"WrongStageAtomic",
	State.CurrentAttack.ActionPreparation.MarkerCardId = MarkerOneId;
	State.CurrentAttack.SelectionStage =
		EMatchPlayCurrentAttackSelectionStage::AwaitingSkill,
	EMatchPlayCurrentAttackMarkerSelectionErrorCode
		::WrongSelectionStage)

MARKER_WRITER_FAILURE_TEST(
	FMarkerWriterDifferentAreaTest,
	"DifferentAreaAtomic",
	Request.MarkerCardId = OtherAreaMarkerId,
	EMatchPlayCurrentAttackMarkerSelectionErrorCode
		::MarkerNotInCarrierPhysicalArea)

MARKER_WRITER_FAILURE_TEST(
	FMarkerWriterGoalkeeperTest,
	"GoalkeeperAtomic",
	Request.MarkerCardId = GoalkeeperMarkerId,
	EMatchPlayCurrentAttackMarkerSelectionErrorCode
		::MarkerIsGoalkeeper)

MARKER_WRITER_FAILURE_TEST(
	FMarkerWriterSnapshotFailureTest,
	"SnapshotFailureAtomic",
	State.CurrentAttack.DeploymentPlacements.Add(
		MakePlacement(
			EInitialTurnOrderPlayer::PlayerB,
			MissingSnapshotMarkerId,
			MissingSnapshotSlotId));
	Request.MarkerCardId = MissingSnapshotMarkerId,
	EMatchPlayCurrentAttackMarkerSelectionErrorCode
		::MarkerSnapshotQueryFailed)

MARKER_WRITER_FAILURE_TEST(
	FMarkerWriterAreaFailureTest,
	"AreaFailureAtomic",
	State.CurrentAttack.DeploymentPlacements[1].SlotId =
		FName(TEXT("Physical.Unknown")),
	EMatchPlayCurrentAttackMarkerSelectionErrorCode
		::PhysicalAreaQueryFailed)

MARKER_WRITER_TEST(
	FMarkerWriterRepeatedSubmissionTest,
	"RepeatedAndReplacementSubmissionsRejected")

bool FMarkerWriterRepeatedSubmissionTest::RunTest(
	const FString& Parameters)
{
	using namespace
		FMCodex::Tests::MatchPlayCurrentAttackMarkerSelection;
	const FMatchPlayState Initial = MakeState();
	const auto First =
		FMatchPlayCurrentAttackMarkerSelectionWriter::Select(
			Initial,
			MakeRequest());
	TestTrue(TEXT("First selection succeeds"), First.bSuccess);
	const FMatchPlayState Frozen = First.AfterState;

	const auto Repeat =
		FMatchPlayCurrentAttackMarkerSelectionWriter::Select(
			Frozen,
			MakeRequest());
	TestFalse(TEXT("Same marker repeat rejected"), Repeat.bSuccess);
	TestEqual(TEXT("Repeat rejected by stage"),
		Repeat.LegalityResult.ErrorCode,
		EMatchPlayCurrentAttackMarkerSelectionErrorCode
			::WrongSelectionStage);
	TestTrue(TEXT("Repeat atomic"),
		AreStatesEqual(Repeat.AfterState, Frozen));

	FMatchPlayCurrentAttackMarkerSelectionRequest Replacement =
		MakeRequest(MarkerTwoId);
	const auto Replace =
		FMatchPlayCurrentAttackMarkerSelectionWriter::Select(
			Frozen,
			Replacement);
	TestFalse(TEXT("Different marker replacement rejected"),
		Replace.bSuccess);
	TestEqual(TEXT("Replacement rejected by stage"),
		Replace.LegalityResult.ErrorCode,
		EMatchPlayCurrentAttackMarkerSelectionErrorCode
			::WrongSelectionStage);
	TestTrue(TEXT("Replacement atomic"),
		AreStatesEqual(Replace.AfterState, Frozen));
	return true;
}

MARKER_WRITER_TEST(
	FMarkerWriterAuthorityIsolationTest,
	"PreservesAllUnrelatedAuthorities")

bool FMarkerWriterAuthorityIsolationTest::RunTest(
	const FString& Parameters)
{
	using namespace
		FMCodex::Tests::MatchPlayCurrentAttackMarkerSelection;
	const FMatchPlayState BeforeState = MakeState();
	const auto Result =
		FMatchPlayCurrentAttackMarkerSelectionWriter::Select(
			BeforeState,
			MakeRequest());
	TestTrue(TEXT("Writer succeeds"), Result.bSuccess);
	TestTrue(TEXT("Runtime and scores unchanged"),
		FMatchRuntimeState::StaticStruct()->CompareScriptStruct(
			&Result.AfterState.RuntimeState,
			&BeforeState.RuntimeState,
			0));
	TestTrue(TEXT("CardUsage unchanged"),
		FMatchCardUsageState::StaticStruct()->CompareScriptStruct(
			&Result.AfterState.CardUsageState,
			&BeforeState.CardUsageState,
			0));
	bool bPlacementsUnchanged =
		Result.AfterState.CurrentAttack.DeploymentPlacements.Num()
		== BeforeState.CurrentAttack.DeploymentPlacements.Num();
	for (int32 Index = 0;
		bPlacementsUnchanged
			&& Index
				< BeforeState.CurrentAttack.DeploymentPlacements.Num();
		++Index)
	{
		bPlacementsUnchanged =
			FMatchPlayDeploymentPlacement::StaticStruct()
				->CompareScriptStruct(
					&Result.AfterState.CurrentAttack
						.DeploymentPlacements[Index],
					&BeforeState.CurrentAttack
						.DeploymentPlacements[Index],
					0);
	}
	TestTrue(TEXT("Deployment placements unchanged"),
		bPlacementsUnchanged);
	TestEqual(TEXT("ActionPoint unchanged"),
		Result.AfterState.CurrentAttack.ActionPoint,
		BeforeState.CurrentAttack.ActionPoint);
	TestEqual(TEXT("Goalkeeper current state unchanged"),
		Result.AfterState.CurrentAttack
			.bCurrentDefenseGoalkeeperActivated,
		BeforeState.CurrentAttack
			.bCurrentDefenseGoalkeeperActivated);
	TestTrue(TEXT("Goalkeeper persistent state unchanged"),
		FMatchPlayGoalkeeperUsageState::StaticStruct()
			->CompareScriptStruct(
				&Result.AfterState.GoalkeeperUsageState,
				&BeforeState.GoalkeeperUsageState,
				0));
	return true;
}

#undef MARKER_WRITER_FAILURE_TEST
#undef MARKER_WRITER_TEST

#endif
