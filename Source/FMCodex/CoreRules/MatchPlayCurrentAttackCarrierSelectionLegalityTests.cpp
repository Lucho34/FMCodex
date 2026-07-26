#include "MatchPlayCurrentAttackCarrierSelectionLegality.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "MatchPlayCurrentAttackCarrierSelectionTestFixtures.h"
#include "Misc/AutomationTest.h"

#include <type_traits>

namespace CarrierSelectionLegalityTests
{
	using namespace
		FMCodex::Tests::MatchPlayCurrentAttackCarrierSelection;

	bool ExpectFailure(
		FAutomationTestBase& Test,
		const TCHAR* Context,
		const FMatchPlayState& State,
		const FMatchPlayCurrentAttackCarrierSelectionRequest& Request,
		const EMatchPlayCurrentAttackCarrierSelectionErrorCode ExpectedError)
	{
		const FMatchPlayState OriginalState = State;
		const auto Result =
			FMatchPlayCurrentAttackCarrierSelectionLegalityEvaluator
				::Evaluate(State, Request);
		Test.TestFalse(
			*FString::Printf(TEXT("%s is illegal"), Context),
			Result.bIsLegal);
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

#define CARRIER_LEGALITY_TEST(TestClass, TestName) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
		TestClass, \
		"FMCodex.CoreRules.MatchPlayCurrentAttackCarrierSelection.Legality." TestName, \
		EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

CARRIER_LEGALITY_TEST(
	FCarrierLegalityContractTest,
	"DefaultsReflectionAndSignature")

bool FCarrierLegalityContractTest::RunTest(const FString& Parameters)
{
	const FMatchPlayCurrentAttackCarrierSelectionRequest Request;
	const FMatchPlayCurrentAttackCarrierSelectionLegalityResult Result;
	TestEqual(TEXT("Default sequence"), Request.AttackSequence, int64{0});
	TestEqual(TEXT("Default side"), Request.RequestingSide,
		EInitialTurnOrderPlayer::None);
	TestTrue(TEXT("Default carrier empty"), Request.CarrierCardId.IsNone());
	TestFalse(TEXT("Default result illegal"), Result.bIsLegal);
	TestNotNull(TEXT("Request reflected"),
		FMatchPlayCurrentAttackCarrierSelectionRequest::StaticStruct());
	TestNotNull(TEXT("Result reflected"),
		FMatchPlayCurrentAttackCarrierSelectionLegalityResult
			::StaticStruct());
	using FEvaluateSignature =
		FMatchPlayCurrentAttackCarrierSelectionLegalityResult (*)(
			const FMatchPlayState&,
			const FMatchPlayCurrentAttackCarrierSelectionRequest&);
	TestTrue(TEXT("Single evaluator signature"),
		(std::is_same_v<
			decltype(
				&FMatchPlayCurrentAttackCarrierSelectionLegalityEvaluator
					::Evaluate),
			FEvaluateSignature>));
	return true;
}

CARRIER_LEGALITY_TEST(FCarrierLegalitySuccessTest, "ValidCarrier")

bool FCarrierLegalitySuccessTest::RunTest(const FString& Parameters)
{
	using namespace
		FMCodex::Tests::MatchPlayCurrentAttackCarrierSelection;
	const FMatchPlayState State = MakeState();
	const FMatchPlayState Original = State;
	const auto Result =
		FMatchPlayCurrentAttackCarrierSelectionLegalityEvaluator
			::Evaluate(State, MakeRequest());
	TestTrue(TEXT("Carrier is legal"), Result.bIsLegal);
	TestEqual(TEXT("Error clean"), Result.ErrorCode,
		EMatchPlayCurrentAttackCarrierSelectionErrorCode::None);
	TestEqual(TEXT("Exactly one placement"),
		Result.MatchingCarrierPlacementCount, 1);
	TestTrue(TEXT("Selection state canonical"),
		Result.SelectionStateValidationResult.bIsCanonical);
	TestTrue(TEXT("Input unchanged"), AreStatesEqual(State, Original));
	return true;
}

#define CARRIER_LEGALITY_FAILURE_TEST( \
	TestClass, TestName, Mutation, ExpectedError) \
	CARRIER_LEGALITY_TEST(TestClass, TestName) \
	bool TestClass::RunTest(const FString& Parameters) \
	{ \
		using namespace \
			FMCodex::Tests::MatchPlayCurrentAttackCarrierSelection; \
		FMatchPlayState State = MakeState(); \
		FMatchPlayCurrentAttackCarrierSelectionRequest Request = \
			MakeRequest(); \
		Mutation; \
		return CarrierSelectionLegalityTests::ExpectFailure( \
			*this, \
			TEXT(TestName), \
			State, \
			Request, \
			ExpectedError); \
	}

CARRIER_LEGALITY_FAILURE_TEST(
	FCarrierLegalityUninitializedTest,
	"UninitializedRejectedFirst",
	State.RuntimeState.bIsInitialized = false,
	EMatchPlayCurrentAttackCarrierSelectionErrorCode
		::MatchPlayStateNotInitialized)

CARRIER_LEGALITY_FAILURE_TEST(
	FCarrierLegalityNoAttackTest,
	"NoCurrentAttack",
	State.bHasCurrentAttack = false,
	EMatchPlayCurrentAttackCarrierSelectionErrorCode::NoCurrentAttack)

CARRIER_LEGALITY_FAILURE_TEST(
	FCarrierLegalityInvalidSequenceTest,
	"InvalidAuthoritativeSequence",
	State.CurrentAttack.AttackSequence = 0,
	EMatchPlayCurrentAttackCarrierSelectionErrorCode
		::InvalidCurrentAttackSequence)

CARRIER_LEGALITY_FAILURE_TEST(
	FCarrierLegalityStaleSequenceTest,
	"StaleSequence",
	Request.AttackSequence = ValidAttackSequence + 1,
	EMatchPlayCurrentAttackCarrierSelectionErrorCode
		::AttackSequenceMismatch)

CARRIER_LEGALITY_FAILURE_TEST(
	FCarrierLegalityWrongPhaseTest,
	"WrongPhase",
	State.CurrentAttack.Phase =
		EMatchPlayCurrentAttackPhase::Deployment,
	EMatchPlayCurrentAttackCarrierSelectionErrorCode
		::CurrentAttackNotInResolution)

CARRIER_LEGALITY_FAILURE_TEST(
	FCarrierLegalityInvalidAttackerTest,
	"InvalidAttacker",
	State.RuntimeState.CurrentAttackingPlayer =
		EInitialTurnOrderPlayer::None,
	EMatchPlayCurrentAttackCarrierSelectionErrorCode
		::InvalidCurrentAttackingPlayer)

CARRIER_LEGALITY_FAILURE_TEST(
	FCarrierLegalityCorruptStateTest,
	"CorruptSelectionState",
	State.CurrentAttack.ActionPreparation.CarrierCardId =
		CarrierOneId,
	EMatchPlayCurrentAttackCarrierSelectionErrorCode
		::InvalidSelectionState)

CARRIER_LEGALITY_FAILURE_TEST(
	FCarrierLegalityWrongStageTest,
	"WrongStage",
	State.CurrentAttack.ActionPreparation.CarrierCardId =
		CarrierOneId;
	State.CurrentAttack.SelectionStage =
		EMatchPlayCurrentAttackSelectionStage::AwaitingMarker,
	EMatchPlayCurrentAttackCarrierSelectionErrorCode
		::WrongSelectionStage)

CARRIER_LEGALITY_FAILURE_TEST(
	FCarrierLegalityInvalidRequesterTest,
	"InvalidRequestingSide",
	Request.RequestingSide = EInitialTurnOrderPlayer::None,
	EMatchPlayCurrentAttackCarrierSelectionErrorCode
		::InvalidRequestingSide)

CARRIER_LEGALITY_FAILURE_TEST(
	FCarrierLegalityDefenderRequesterTest,
	"DefenderCannotSelect",
	Request.RequestingSide = EInitialTurnOrderPlayer::PlayerB,
	EMatchPlayCurrentAttackCarrierSelectionErrorCode
		::RequestingSideIsNotCurrentAttacker)

CARRIER_LEGALITY_FAILURE_TEST(
	FCarrierLegalityEmptyCardTest,
	"EmptyCarrierId",
	Request.CarrierCardId = NAME_None,
	EMatchPlayCurrentAttackCarrierSelectionErrorCode
		::InvalidCarrierCardId)

CARRIER_LEGALITY_FAILURE_TEST(
	FCarrierLegalityDefenderFallbackTest,
	"DefenderSameIdDoesNotFallback",
	State.CurrentAttack.DeploymentPlacements.RemoveAt(0);
	State.CurrentAttack.DeploymentPlacements.RemoveAt(1),
	EMatchPlayCurrentAttackCarrierSelectionErrorCode
		::CarrierNotDeployed)

CARRIER_LEGALITY_FAILURE_TEST(
	FCarrierLegalityAmbiguousTest,
	"DuplicateAttackerPlacement",
	State.CurrentAttack.DeploymentPlacements.Add(
		MakePlacement(
			EInitialTurnOrderPlayer::PlayerA,
			CarrierOneId,
			TEXT("Slot.A3"))),
	EMatchPlayCurrentAttackCarrierSelectionErrorCode
		::CarrierDeploymentAmbiguous)

CARRIER_LEGALITY_FAILURE_TEST(
	FCarrierLegalityMissingSnapshotTest,
	"MissingSnapshot",
	State.CardSnapshotAuthority.PlayerACardSnapshots.Cards.RemoveAt(0),
	EMatchPlayCurrentAttackCarrierSelectionErrorCode
		::CarrierSnapshotLookupFailed)

CARRIER_LEGALITY_FAILURE_TEST(
	FCarrierLegalityInvalidSnapshotAuthorityTest,
	"InvalidSnapshotAuthority",
	State.CardSnapshotAuthority.PlayerACardSnapshots.Cards.Add(
		MakeCard(CarrierOneId)),
	EMatchPlayCurrentAttackCarrierSelectionErrorCode
		::CarrierSnapshotLookupFailed)

CARRIER_LEGALITY_FAILURE_TEST(
	FCarrierLegalityGoalkeeperTest,
	"GoalkeeperCarrier",
	State.CurrentAttack.DeploymentPlacements[0].CardId =
		GoalkeeperId;
	Request.CarrierCardId = GoalkeeperId,
	EMatchPlayCurrentAttackCarrierSelectionErrorCode
		::CarrierIsGoalkeeper)

CARRIER_LEGALITY_TEST(
	FCarrierLegalityFirstErrorTest,
	"MixedInvalidUsesFrozenFirstError")

bool FCarrierLegalityFirstErrorTest::RunTest(const FString& Parameters)
{
	using namespace
		FMCodex::Tests::MatchPlayCurrentAttackCarrierSelection;
	FMatchPlayState State = MakeState();
	State.RuntimeState.bIsInitialized = false;
	State.CurrentAttack.AttackSequence = 0;
	FMatchPlayCurrentAttackCarrierSelectionRequest Request = MakeRequest();
	Request.RequestingSide = EInitialTurnOrderPlayer::None;
	Request.CarrierCardId = NAME_None;
	return CarrierSelectionLegalityTests::ExpectFailure(
		*this,
		TEXT("Mixed invalid"),
		State,
		Request,
		EMatchPlayCurrentAttackCarrierSelectionErrorCode
			::MatchPlayStateNotInitialized);
}

CARRIER_LEGALITY_TEST(
	FCarrierLegalityDeterministicTest,
	"RepeatedEvaluationStable")

bool FCarrierLegalityDeterministicTest::RunTest(
	const FString& Parameters)
{
	using namespace
		FMCodex::Tests::MatchPlayCurrentAttackCarrierSelection;
	const FMatchPlayState State = MakeState();
	const auto First =
		FMatchPlayCurrentAttackCarrierSelectionLegalityEvaluator
			::Evaluate(State, MakeRequest());
	const auto Second =
		FMatchPlayCurrentAttackCarrierSelectionLegalityEvaluator
			::Evaluate(State, MakeRequest());
	TestTrue(TEXT("Results are identical"),
		FMatchPlayCurrentAttackCarrierSelectionLegalityResult
			::StaticStruct()->CompareScriptStruct(
				&First,
				&Second,
				0));
	return true;
}

#undef CARRIER_LEGALITY_FAILURE_TEST
#undef CARRIER_LEGALITY_TEST

#endif
