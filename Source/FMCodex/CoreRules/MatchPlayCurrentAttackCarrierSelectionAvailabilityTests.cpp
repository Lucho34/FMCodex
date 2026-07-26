#include "MatchPlayCurrentAttackCarrierSelectionAvailability.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "MatchPlayCurrentAttackCarrierSelectionTestFixtures.h"
#include "Misc/AutomationTest.h"

#include <type_traits>

namespace CarrierSelectionAvailabilityTests
{
	using namespace
		FMCodex::Tests::MatchPlayCurrentAttackCarrierSelection;

	FMatchPlayCurrentAttackCarrierSelectionAvailabilityResult Query(
		const FMatchPlayState& State,
		const int64 AttackSequence = ValidAttackSequence,
		const EInitialTurnOrderPlayer RequestingSide =
			EInitialTurnOrderPlayer::PlayerA)
	{
		return FMatchPlayCurrentAttackCarrierSelectionAvailability
			::Query(State, AttackSequence, RequestingSide);
	}
}

#define CARRIER_AVAILABILITY_TEST(TestClass, TestName) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
		TestClass, \
		"FMCodex.CoreRules.MatchPlayCurrentAttackCarrierSelection.Availability." TestName, \
		EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

CARRIER_AVAILABILITY_TEST(
	FCarrierAvailabilityContractTest,
	"DefaultsReflectionAndSignature")

bool FCarrierAvailabilityContractTest::RunTest(const FString& Parameters)
{
	const FMatchPlayCurrentAttackCarrierSelectionAvailabilityResult Result;
	TestFalse(TEXT("Default query not succeeded"),
		Result.bQuerySucceeded);
	TestFalse(TEXT("Default cannot select"),
		Result.bCanSelectAnyCarrier);
	TestTrue(TEXT("Default candidates empty"), Result.Candidates.IsEmpty());
	TestNotNull(TEXT("Result reflected"),
		FMatchPlayCurrentAttackCarrierSelectionAvailabilityResult
			::StaticStruct());
	using FQuerySignature =
		FMatchPlayCurrentAttackCarrierSelectionAvailabilityResult (*)(
			const FMatchPlayState&,
			int64,
			EInitialTurnOrderPlayer);
	TestTrue(TEXT("Single query signature"),
		(std::is_same_v<
			decltype(
				&FMatchPlayCurrentAttackCarrierSelectionAvailability
					::Query),
			FQuerySignature>));
	return true;
}

CARRIER_AVAILABILITY_TEST(
	FCarrierAvailabilityOrderTest,
	"PreservesAttackerPlacementOrder")

bool FCarrierAvailabilityOrderTest::RunTest(const FString& Parameters)
{
	using namespace CarrierSelectionAvailabilityTests;
	const FMatchPlayState State = MakeState();
	const FMatchPlayState Original = State;
	const auto Result = Query(State);
	TestTrue(TEXT("Query succeeds"), Result.bQuerySucceeded);
	TestTrue(TEXT("At least one carrier legal"),
		Result.bCanSelectAnyCarrier);
	TestEqual(TEXT("Defender excluded"), Result.Candidates.Num(), 2);
	TestEqual(TEXT("First attacker placement retained"),
		Result.Candidates[0].CarrierCardId, CarrierOneId);
	TestEqual(TEXT("Second attacker placement retained"),
		Result.Candidates[1].CarrierCardId, CarrierTwoId);
	TestTrue(TEXT("Both candidates legal"),
		Result.Candidates[0].LegalityResult.bIsLegal
			&& Result.Candidates[1].LegalityResult.bIsLegal);
	TestTrue(TEXT("State unchanged"), AreStatesEqual(State, Original));
	return true;
}

CARRIER_AVAILABILITY_TEST(
	FCarrierAvailabilityGoalkeeperCandidateTest,
	"RetainsGoalkeeperWithIllegalReason")

bool FCarrierAvailabilityGoalkeeperCandidateTest::RunTest(
	const FString& Parameters)
{
	using namespace CarrierSelectionAvailabilityTests;
	FMatchPlayState State = MakeState();
	State.CurrentAttack.DeploymentPlacements.Add(
		MakePlacement(
			EInitialTurnOrderPlayer::PlayerA,
			GoalkeeperId,
			TEXT("Slot.GK")));
	const auto Result = Query(State);
	TestEqual(TEXT("All attacker placements represented"),
		Result.Candidates.Num(), 3);
	TestEqual(TEXT("GK keeps original order"),
		Result.Candidates[2].CarrierCardId, GoalkeeperId);
	TestEqual(TEXT("GK reason retained"),
		Result.Candidates[2].LegalityResult.ErrorCode,
		EMatchPlayCurrentAttackCarrierSelectionErrorCode
			::CarrierIsGoalkeeper);
	return true;
}

CARRIER_AVAILABILITY_TEST(
	FCarrierAvailabilityDuplicateTest,
	"DuplicatePlacementsAreNotSilentlyDeduplicated")

bool FCarrierAvailabilityDuplicateTest::RunTest(
	const FString& Parameters)
{
	using namespace CarrierSelectionAvailabilityTests;
	FMatchPlayState State = MakeState();
	State.CurrentAttack.DeploymentPlacements.Add(
		MakePlacement(
			EInitialTurnOrderPlayer::PlayerA,
			CarrierOneId,
			TEXT("Slot.A3")));
	const auto Result = Query(State);
	TestEqual(TEXT("Duplicate placement remains visible"),
		Result.Candidates.Num(), 3);
	TestEqual(TEXT("First duplicate is ambiguous"),
		Result.Candidates[0].LegalityResult.ErrorCode,
		EMatchPlayCurrentAttackCarrierSelectionErrorCode
			::CarrierDeploymentAmbiguous);
	TestEqual(TEXT("Second duplicate is ambiguous"),
		Result.Candidates[2].LegalityResult.ErrorCode,
		EMatchPlayCurrentAttackCarrierSelectionErrorCode
			::CarrierDeploymentAmbiguous);
	TestTrue(TEXT("Other carrier remains selectable"),
		Result.bCanSelectAnyCarrier);
	return true;
}

CARRIER_AVAILABILITY_TEST(
	FCarrierAvailabilityZeroLegalTest,
	"ZeroLegalCarriersIsSuccessfulQuery")

bool FCarrierAvailabilityZeroLegalTest::RunTest(
	const FString& Parameters)
{
	using namespace CarrierSelectionAvailabilityTests;
	FMatchPlayState State = MakeState();
	State.CurrentAttack.DeploymentPlacements = {
		MakePlacement(
			EInitialTurnOrderPlayer::PlayerA,
			GoalkeeperId,
			TEXT("Slot.GK")),
		MakePlacement(
			EInitialTurnOrderPlayer::PlayerB,
			DefenderSameId,
			TEXT("Slot.B1"))
	};
	const auto Result = Query(State);
	TestTrue(TEXT("Query succeeds"), Result.bQuerySucceeded);
	TestFalse(TEXT("No legal carrier"), Result.bCanSelectAnyCarrier);
	TestEqual(TEXT("GK candidate retained"), Result.Candidates.Num(), 1);
	return true;
}

#define CARRIER_AVAILABILITY_BLOCKER_TEST( \
	TestClass, TestName, Mutation, ExpectedError) \
	CARRIER_AVAILABILITY_TEST(TestClass, TestName) \
	bool TestClass::RunTest(const FString& Parameters) \
	{ \
		using namespace CarrierSelectionAvailabilityTests; \
		FMatchPlayState State = MakeState(); \
		int64 Sequence = ValidAttackSequence; \
		EInitialTurnOrderPlayer Side = \
			EInitialTurnOrderPlayer::PlayerA; \
		Mutation; \
		const auto Result = Query(State, Sequence, Side); \
		TestTrue(TEXT("Query completes"), Result.bQuerySucceeded); \
		TestFalse(TEXT("No carrier can be selected"), \
			Result.bCanSelectAnyCarrier); \
		TestTrue(TEXT("Global blocker retained"), \
			Result.bHasGlobalBlockingLegalityResult); \
		TestEqual(TEXT("Exact blocker retained"), \
			Result.GlobalBlockingLegalityResult.ErrorCode, \
			ExpectedError); \
		TestTrue(TEXT("No candidates enumerated"), \
			Result.Candidates.IsEmpty()); \
		return true; \
	}

CARRIER_AVAILABILITY_BLOCKER_TEST(
	FCarrierAvailabilityStaleSequenceTest,
	"StaleSequenceGlobalBlocker",
	Sequence = ValidAttackSequence + 1,
	EMatchPlayCurrentAttackCarrierSelectionErrorCode
		::AttackSequenceMismatch)

CARRIER_AVAILABILITY_BLOCKER_TEST(
	FCarrierAvailabilityWrongPhaseTest,
	"WrongPhaseGlobalBlocker",
	State.CurrentAttack.Phase =
		EMatchPlayCurrentAttackPhase::Deployment,
	EMatchPlayCurrentAttackCarrierSelectionErrorCode
		::CurrentAttackNotInResolution)

CARRIER_AVAILABILITY_BLOCKER_TEST(
	FCarrierAvailabilityWrongStageTest,
	"WrongStageGlobalBlocker",
	State.CurrentAttack.ActionPreparation.CarrierCardId =
		CarrierOneId;
	State.CurrentAttack.SelectionStage =
		EMatchPlayCurrentAttackSelectionStage::AwaitingMarker,
	EMatchPlayCurrentAttackCarrierSelectionErrorCode
		::WrongSelectionStage)

CARRIER_AVAILABILITY_BLOCKER_TEST(
	FCarrierAvailabilityCorruptStateTest,
	"CorruptStateGlobalBlocker",
	State.CurrentAttack.ActionPreparation.CarrierCardId =
		CarrierOneId,
	EMatchPlayCurrentAttackCarrierSelectionErrorCode
		::InvalidSelectionState)

CARRIER_AVAILABILITY_BLOCKER_TEST(
	FCarrierAvailabilityWrongSideTest,
	"WrongSideGlobalBlocker",
	Side = EInitialTurnOrderPlayer::PlayerB,
	EMatchPlayCurrentAttackCarrierSelectionErrorCode
		::RequestingSideIsNotCurrentAttacker)

CARRIER_AVAILABILITY_TEST(
	FCarrierAvailabilityDeterministicTest,
	"RepeatedQueryStableAndReadOnly")

bool FCarrierAvailabilityDeterministicTest::RunTest(
	const FString& Parameters)
{
	using namespace CarrierSelectionAvailabilityTests;
	const FMatchPlayState State = MakeState();
	const FMatchPlayState Original = State;
	const auto First = Query(State);
	const auto Second = Query(State);
	TestTrue(TEXT("Results identical"),
		FMatchPlayCurrentAttackCarrierSelectionAvailabilityResult
			::StaticStruct()->CompareScriptStruct(
				&First,
				&Second,
				0));
	TestTrue(TEXT("State unchanged"), AreStatesEqual(State, Original));
	return true;
}

#undef CARRIER_AVAILABILITY_BLOCKER_TEST
#undef CARRIER_AVAILABILITY_TEST

#endif
