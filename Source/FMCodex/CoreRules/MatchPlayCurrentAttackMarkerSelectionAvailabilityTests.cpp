#include "MatchPlayCurrentAttackMarkerSelectionAvailability.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "MatchPlayCurrentAttackMarkerSelectionTestFixtures.h"
#include "Misc/AutomationTest.h"

#include <type_traits>

namespace MarkerSelectionAvailabilityTests
{
	using namespace
		FMCodex::Tests::MatchPlayCurrentAttackMarkerSelection;

	FMatchPlayCurrentAttackMarkerSelectionAvailabilityResult Query(
		const FMatchPlayState& State,
		const int64 Sequence = ValidAttackSequence,
		const EInitialTurnOrderPlayer Side =
			EInitialTurnOrderPlayer::PlayerB)
	{
		return FMatchPlayCurrentAttackMarkerSelectionAvailability::Query(
			State,
			Sequence,
			Side);
	}

	bool ExpectGlobalBlocker(
		FAutomationTestBase& Test,
		const TCHAR* Context,
		const FMatchPlayState& State,
		const int64 Sequence,
		const EInitialTurnOrderPlayer Side,
		const EMatchPlayCurrentAttackMarkerSelectionErrorCode Error)
	{
		const FMatchPlayState Original = State;
		const auto Result = Query(State, Sequence, Side);
		Test.TestTrue(
			*FString::Printf(TEXT("%s query succeeds"), Context),
			Result.bQuerySucceeded);
		Test.TestTrue(
			*FString::Printf(TEXT("%s has global blocker"), Context),
			Result.bHasGlobalBlockingLegalityResult);
		Test.TestEqual(
			*FString::Printf(TEXT("%s exact blocker"), Context),
			Result.GlobalBlockingLegalityResult.ErrorCode,
			Error);
		Test.TestFalse(
			*FString::Printf(TEXT("%s cannot select"), Context),
			Result.bCanSelectAnyMarker);
		Test.TestEqual(
			*FString::Printf(TEXT("%s has no candidates"), Context),
			Result.Candidates.Num(),
			0);
		Test.TestTrue(
			*FString::Printf(TEXT("%s is read-only"), Context),
			AreStatesEqual(State, Original));
		return true;
	}
}

#define MARKER_AVAILABILITY_TEST(TestClass, TestName) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
		TestClass, \
		"FMCodex.CoreRules.MatchPlayCurrentAttackMarkerSelection.Availability." TestName, \
		EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

MARKER_AVAILABILITY_TEST(
	FMarkerAvailabilityContractTest,
	"DefaultsReflectionAndSignature")

bool FMarkerAvailabilityContractTest::RunTest(const FString& Parameters)
{
	const FMatchPlayCurrentAttackMarkerSelectionAvailabilityResult Result;
	TestFalse(TEXT("Default query not run"), Result.bQuerySucceeded);
	TestFalse(TEXT("Default cannot select"), Result.bCanSelectAnyMarker);
	TestNotNull(TEXT("Candidate reflected"),
		FMatchPlayCurrentAttackMarkerSelectionCandidateAvailability
			::StaticStruct());
	TestNotNull(TEXT("Result reflected"),
		FMatchPlayCurrentAttackMarkerSelectionAvailabilityResult
			::StaticStruct());
	using FQuerySignature =
		FMatchPlayCurrentAttackMarkerSelectionAvailabilityResult (*)(
			const FMatchPlayState&,
			int64,
			EInitialTurnOrderPlayer);
	TestTrue(TEXT("Single query signature"),
		(std::is_same_v<
			decltype(
				&FMatchPlayCurrentAttackMarkerSelectionAvailability
					::Query),
			FQuerySignature>));
	return true;
}

MARKER_AVAILABILITY_TEST(
	FMarkerAvailabilityCandidateOrderTest,
	"DefenderPlacementsPreserveOrderAndErrors")

bool FMarkerAvailabilityCandidateOrderTest::RunTest(
	const FString& Parameters)
{
	using namespace
		FMCodex::Tests::MatchPlayCurrentAttackMarkerSelection;
	const FMatchPlayState State = MakeState();
	const FMatchPlayState Original = State;
	const auto Result =
		MarkerSelectionAvailabilityTests::Query(State);
	TestTrue(TEXT("Query succeeds"), Result.bQuerySucceeded);
	TestFalse(TEXT("No global blocker"),
		Result.bHasGlobalBlockingLegalityResult);
	TestTrue(TEXT("At least one marker legal"),
		Result.bCanSelectAnyMarker);
	TestEqual(TEXT("Only defender placements become candidates"),
		Result.Candidates.Num(), 4);
	if (Result.Candidates.Num() == 4)
	{
		TestEqual(TEXT("First candidate original order"),
			Result.Candidates[0].MarkerCardId, MarkerOneId);
		TestTrue(TEXT("First candidate legal"),
			Result.Candidates[0].LegalityResult.bIsLegal);
		TestEqual(TEXT("Different area retained"),
			Result.Candidates[1].LegalityResult.ErrorCode,
			EMatchPlayCurrentAttackMarkerSelectionErrorCode
				::MarkerNotInCarrierPhysicalArea);
		TestEqual(TEXT("Goalkeeper retained"),
			Result.Candidates[2].LegalityResult.ErrorCode,
			EMatchPlayCurrentAttackMarkerSelectionErrorCode
				::MarkerIsGoalkeeper);
		TestEqual(TEXT("Last candidate original order"),
			Result.Candidates[3].MarkerCardId, MarkerTwoId);
		TestTrue(TEXT("Last candidate legal"),
			Result.Candidates[3].LegalityResult.bIsLegal);
	}
	TestTrue(TEXT("State unchanged"), AreStatesEqual(State, Original));
	return true;
}

MARKER_AVAILABILITY_TEST(
	FMarkerAvailabilityMissingSnapshotCandidateTest,
	"SnapshotFailureCandidateRetained")

bool FMarkerAvailabilityMissingSnapshotCandidateTest::RunTest(
	const FString& Parameters)
{
	using namespace
		FMCodex::Tests::MatchPlayCurrentAttackMarkerSelection;
	FMatchPlayState State = MakeState();
	State.CurrentAttack.DeploymentPlacements.Add(
		MakePlacement(
			EInitialTurnOrderPlayer::PlayerB,
			MissingSnapshotMarkerId,
			MissingSnapshotSlotId));
	const auto Result =
		MarkerSelectionAvailabilityTests::Query(State);
	TestTrue(TEXT("Query succeeds"), Result.bQuerySucceeded);
	TestEqual(TEXT("Candidate retained"), Result.Candidates.Num(), 5);
	if (Result.Candidates.Num() == 5)
	{
		TestEqual(TEXT("Snapshot failure preserved"),
			Result.Candidates[4].LegalityResult.ErrorCode,
			EMatchPlayCurrentAttackMarkerSelectionErrorCode
				::MarkerSnapshotQueryFailed);
	}
	return true;
}

MARKER_AVAILABILITY_TEST(
	FMarkerAvailabilityDuplicatePlacementTest,
	"DuplicatePlacementsAreNotDeduplicated")

bool FMarkerAvailabilityDuplicatePlacementTest::RunTest(
	const FString& Parameters)
{
	using namespace
		FMCodex::Tests::MatchPlayCurrentAttackMarkerSelection;
	FMatchPlayState State = MakeState();
	State.CurrentAttack.DeploymentPlacements.Add(
		MakePlacement(
			EInitialTurnOrderPlayer::PlayerB,
			MarkerOneId,
			MarkerTwoSlotId));
	const auto Result =
		MarkerSelectionAvailabilityTests::Query(State);
	TestEqual(TEXT("All defender placements retained"),
		Result.Candidates.Num(), 5);
	if (Result.Candidates.Num() == 5)
	{
		TestEqual(TEXT("First duplicate is ambiguous"),
			Result.Candidates[0].LegalityResult.ErrorCode,
			EMatchPlayCurrentAttackMarkerSelectionErrorCode
				::MarkerDeploymentAmbiguous);
		TestEqual(TEXT("Second duplicate is ambiguous"),
			Result.Candidates[4].LegalityResult.ErrorCode,
			EMatchPlayCurrentAttackMarkerSelectionErrorCode
				::MarkerDeploymentAmbiguous);
	}
	return true;
}

MARKER_AVAILABILITY_TEST(
	FMarkerAvailabilityZeroLegalTest,
	"ZeroLegalMarkersStillSuccessful")

bool FMarkerAvailabilityZeroLegalTest::RunTest(const FString& Parameters)
{
	using namespace
		FMCodex::Tests::MatchPlayCurrentAttackMarkerSelection;
	FMatchPlayState State = MakeState();
	State.CurrentAttack.DeploymentPlacements.RemoveAll(
		[](const FMatchPlayDeploymentPlacement& Placement)
		{
			return Placement.PlayerSide
					== EInitialTurnOrderPlayer::PlayerB
				&& (Placement.CardId == MarkerOneId
					|| Placement.CardId == MarkerTwoId);
		});
	const auto Result =
		MarkerSelectionAvailabilityTests::Query(State);
	TestTrue(TEXT("Query succeeds"), Result.bQuerySucceeded);
	TestFalse(TEXT("No legal marker"), Result.bCanSelectAnyMarker);
	TestEqual(TEXT("Illegal candidates retained"),
		Result.Candidates.Num(), 2);
	return true;
}

MARKER_AVAILABILITY_TEST(
	FMarkerAvailabilityZeroDefendersTest,
	"ZeroDefendingPlacementsStillSuccessful")

bool FMarkerAvailabilityZeroDefendersTest::RunTest(
	const FString& Parameters)
{
	using namespace
		FMCodex::Tests::MatchPlayCurrentAttackMarkerSelection;
	FMatchPlayState State = MakeState();
	State.CurrentAttack.DeploymentPlacements.RemoveAll(
		[](const FMatchPlayDeploymentPlacement& Placement)
		{
			return Placement.PlayerSide
				== EInitialTurnOrderPlayer::PlayerB;
		});
	const auto Result =
		MarkerSelectionAvailabilityTests::Query(State);
	TestTrue(TEXT("Query succeeds"), Result.bQuerySucceeded);
	TestFalse(TEXT("No marker selectable"), Result.bCanSelectAnyMarker);
	TestEqual(TEXT("No candidates"), Result.Candidates.Num(), 0);
	return true;
}

MARKER_AVAILABILITY_TEST(
	FMarkerAvailabilityWrongStageTest,
	"WrongStageGlobalBlocker")

bool FMarkerAvailabilityWrongStageTest::RunTest(
	const FString& Parameters)
{
	using namespace
		FMCodex::Tests::MatchPlayCurrentAttackMarkerSelection;
	FMatchPlayState State = MakeState();
	State.CurrentAttack.ActionPreparation.MarkerCardId = MarkerOneId;
	State.CurrentAttack.SelectionStage =
		EMatchPlayCurrentAttackSelectionStage::AwaitingSkill;
	return MarkerSelectionAvailabilityTests::ExpectGlobalBlocker(
		*this,
		TEXT("Wrong stage"),
		State,
		ValidAttackSequence,
		EInitialTurnOrderPlayer::PlayerB,
		EMatchPlayCurrentAttackMarkerSelectionErrorCode
			::WrongSelectionStage);
}

MARKER_AVAILABILITY_TEST(
	FMarkerAvailabilityStaleSequenceTest,
	"StaleSequenceGlobalBlocker")

bool FMarkerAvailabilityStaleSequenceTest::RunTest(
	const FString& Parameters)
{
	using namespace
		FMCodex::Tests::MatchPlayCurrentAttackMarkerSelection;
	const FMatchPlayState State = MakeState();
	return MarkerSelectionAvailabilityTests::ExpectGlobalBlocker(
		*this,
		TEXT("Stale sequence"),
		State,
		ValidAttackSequence + 1,
		EInitialTurnOrderPlayer::PlayerB,
		EMatchPlayCurrentAttackMarkerSelectionErrorCode
			::AttackSequenceMismatch);
}

MARKER_AVAILABILITY_TEST(
	FMarkerAvailabilityCorruptStateTest,
	"CorruptStateGlobalBlocker")

bool FMarkerAvailabilityCorruptStateTest::RunTest(
	const FString& Parameters)
{
	using namespace
		FMCodex::Tests::MatchPlayCurrentAttackMarkerSelection;
	FMatchPlayState State = MakeState();
	State.CurrentAttack.ActionPreparation.MarkerCardId = MarkerOneId;
	return MarkerSelectionAvailabilityTests::ExpectGlobalBlocker(
		*this,
		TEXT("Corrupt state"),
		State,
		ValidAttackSequence,
		EInitialTurnOrderPlayer::PlayerB,
		EMatchPlayCurrentAttackMarkerSelectionErrorCode
			::InvalidSelectionState);
}

MARKER_AVAILABILITY_TEST(
	FMarkerAvailabilityCatalogBlockerTest,
	"InvalidCatalogGlobalBlocker")

bool FMarkerAvailabilityCatalogBlockerTest::RunTest(
	const FString& Parameters)
{
	using namespace
		FMCodex::Tests::MatchPlayCurrentAttackMarkerSelection;
	FMatchPlayState State = MakeState();
	State.DeploymentSlotCatalog.Slots.Reset();
	return MarkerSelectionAvailabilityTests::ExpectGlobalBlocker(
		*this,
		TEXT("Invalid catalog"),
		State,
		ValidAttackSequence,
		EInitialTurnOrderPlayer::PlayerB,
		EMatchPlayCurrentAttackMarkerSelectionErrorCode
			::PhysicalAreaQueryFailed);
}

MARKER_AVAILABILITY_TEST(
	FMarkerAvailabilitySnapshotAuthorityBlockerTest,
	"InvalidSnapshotAuthorityGlobalBlocker")

bool FMarkerAvailabilitySnapshotAuthorityBlockerTest::RunTest(
	const FString& Parameters)
{
	using namespace
		FMCodex::Tests::MatchPlayCurrentAttackMarkerSelection;
	FMatchPlayState State = MakeState();
	State.CardSnapshotAuthority.PlayerBCardSnapshots.Cards.Add(
		MakeCard(MarkerOneId));
	return MarkerSelectionAvailabilityTests::ExpectGlobalBlocker(
		*this,
		TEXT("Invalid snapshot authority"),
		State,
		ValidAttackSequence,
		EInitialTurnOrderPlayer::PlayerB,
		EMatchPlayCurrentAttackMarkerSelectionErrorCode
			::MarkerSnapshotQueryFailed);
}

MARKER_AVAILABILITY_TEST(
	FMarkerAvailabilityCarrierSlotBlockerTest,
	"InvalidFrozenCarrierSlotGlobalBlocker")

bool FMarkerAvailabilityCarrierSlotBlockerTest::RunTest(
	const FString& Parameters)
{
	using namespace
		FMCodex::Tests::MatchPlayCurrentAttackMarkerSelection;
	FMatchPlayState State = MakeState();
	State.CurrentAttack.DeploymentPlacements[0].SlotId =
		FName(TEXT("Physical.UnknownCarrier"));
	return MarkerSelectionAvailabilityTests::ExpectGlobalBlocker(
		*this,
		TEXT("Invalid frozen carrier slot"),
		State,
		ValidAttackSequence,
		EInitialTurnOrderPlayer::PlayerB,
		EMatchPlayCurrentAttackMarkerSelectionErrorCode
			::PhysicalAreaQueryFailed);
}

MARKER_AVAILABILITY_TEST(
	FMarkerAvailabilityRepeatedQueryTest,
	"RepeatedQueryDeterministicAndReadOnly")

bool FMarkerAvailabilityRepeatedQueryTest::RunTest(
	const FString& Parameters)
{
	using namespace
		FMCodex::Tests::MatchPlayCurrentAttackMarkerSelection;
	const FMatchPlayState State = MakeState();
	const FMatchPlayState Original = State;
	const auto First =
		MarkerSelectionAvailabilityTests::Query(State);
	const auto Second =
		MarkerSelectionAvailabilityTests::Query(State);
	TestEqual(TEXT("Candidate count stable"),
		First.Candidates.Num(), Second.Candidates.Num());
	TestEqual(TEXT("Aggregate stable"),
		First.bCanSelectAnyMarker, Second.bCanSelectAnyMarker);
	for (int32 Index = 0;
		Index < First.Candidates.Num()
			&& Index < Second.Candidates.Num();
		++Index)
	{
		TestEqual(TEXT("Candidate identity stable"),
			First.Candidates[Index].MarkerCardId,
			Second.Candidates[Index].MarkerCardId);
		TestEqual(TEXT("Candidate result stable"),
			First.Candidates[Index].LegalityResult.ErrorCode,
			Second.Candidates[Index].LegalityResult.ErrorCode);
	}
	TestTrue(TEXT("State, catalog, and snapshots unchanged"),
		AreStatesEqual(State, Original));
	return true;
}

#undef MARKER_AVAILABILITY_TEST

#endif
