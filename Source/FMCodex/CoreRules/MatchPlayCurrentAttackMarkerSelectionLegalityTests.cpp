#include "MatchPlayCurrentAttackMarkerSelectionLegality.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "MatchPlayCurrentAttackMarkerSelectionTestFixtures.h"
#include "Misc/AutomationTest.h"

#include <type_traits>

namespace MarkerSelectionLegalityTests
{
	using namespace
		FMCodex::Tests::MatchPlayCurrentAttackMarkerSelection;

	bool ExpectFailure(
		FAutomationTestBase& Test,
		const TCHAR* Context,
		const FMatchPlayState& State,
		const FMatchPlayCurrentAttackMarkerSelectionRequest& Request,
		const EMatchPlayCurrentAttackMarkerSelectionErrorCode ExpectedError)
	{
		const FMatchPlayState OriginalState = State;
		const auto Result =
			FMatchPlayCurrentAttackMarkerSelectionLegalityEvaluator
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

#define MARKER_LEGALITY_TEST(TestClass, TestName) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
		TestClass, \
		"FMCodex.CoreRules.MatchPlayCurrentAttackMarkerSelection.Legality." TestName, \
		EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

MARKER_LEGALITY_TEST(
	FMarkerLegalityContractTest,
	"DefaultsReflectionAndSignature")

bool FMarkerLegalityContractTest::RunTest(const FString& Parameters)
{
	const FMatchPlayCurrentAttackMarkerSelectionRequest Request;
	const FMatchPlayCurrentAttackMarkerSelectionLegalityResult Result;
	TestEqual(TEXT("Default sequence"), Request.AttackSequence, int64{0});
	TestEqual(TEXT("Default side"), Request.RequestingSide,
		EInitialTurnOrderPlayer::None);
	TestTrue(TEXT("Default marker empty"), Request.MarkerCardId.IsNone());
	TestFalse(TEXT("Default result illegal"), Result.bIsLegal);
	TestNotNull(TEXT("Request reflected"),
		FMatchPlayCurrentAttackMarkerSelectionRequest::StaticStruct());
	TestNotNull(TEXT("Result reflected"),
		FMatchPlayCurrentAttackMarkerSelectionLegalityResult
			::StaticStruct());
	using FEvaluateSignature =
		FMatchPlayCurrentAttackMarkerSelectionLegalityResult (*)(
			const FMatchPlayState&,
			const FMatchPlayCurrentAttackMarkerSelectionRequest&);
	TestTrue(TEXT("Single evaluator signature"),
		(std::is_same_v<
			decltype(
				&FMatchPlayCurrentAttackMarkerSelectionLegalityEvaluator
					::Evaluate),
			FEvaluateSignature>));
	return true;
}

MARKER_LEGALITY_TEST(FMarkerLegalitySuccessTest, "ValidMarker")

bool FMarkerLegalitySuccessTest::RunTest(const FString& Parameters)
{
	using namespace
		FMCodex::Tests::MatchPlayCurrentAttackMarkerSelection;
	const FMatchPlayState State = MakeState();
	const FMatchPlayState Original = State;
	const auto Result =
		FMatchPlayCurrentAttackMarkerSelectionLegalityEvaluator
			::Evaluate(State, MakeRequest());
	TestTrue(TEXT("Marker legal"), Result.bIsLegal);
	TestEqual(TEXT("No error"), Result.ErrorCode,
		EMatchPlayCurrentAttackMarkerSelectionErrorCode::None);
	TestEqual(TEXT("Frozen carrier echoed"),
		Result.FrozenCarrierCardId, CarrierId);
	TestEqual(TEXT("One carrier placement"),
		Result.MatchingFrozenCarrierPlacementCount, 1);
	TestEqual(TEXT("One marker placement"),
		Result.MatchingMarkerPlacementCount, 1);
	TestTrue(TEXT("Marker snapshot succeeds"),
		Result.MarkerSnapshotQueryResult.bSuccess);
	TestTrue(TEXT("Area query succeeds"),
		Result.PhysicalAreaMatchResult.bSuccess);
	TestTrue(TEXT("Different slots share physical area"),
		Result.PhysicalAreaMatchResult.bSamePhysicalArea);
	TestNotEqual(TEXT("Slots differ"),
		Result.FrozenCarrierPlacement.SlotId,
		Result.MarkerPlacement.SlotId);
	TestTrue(TEXT("Input unchanged"), AreStatesEqual(State, Original));
	return true;
}

#define MARKER_LEGALITY_FAILURE_TEST( \
	TestClass, TestName, Mutation, ExpectedError) \
	MARKER_LEGALITY_TEST(TestClass, TestName) \
	bool TestClass::RunTest(const FString& Parameters) \
	{ \
		using namespace \
			FMCodex::Tests::MatchPlayCurrentAttackMarkerSelection; \
		FMatchPlayState State = MakeState(); \
		FMatchPlayCurrentAttackMarkerSelectionRequest Request = \
			MakeRequest(); \
		Mutation; \
		return MarkerSelectionLegalityTests::ExpectFailure( \
			*this, TEXT(TestName), State, Request, ExpectedError); \
	}

MARKER_LEGALITY_FAILURE_TEST(
	FMarkerLegalityUninitializedTest,
	"UninitializedRejectedFirst",
	State.RuntimeState.bIsInitialized = false,
	EMatchPlayCurrentAttackMarkerSelectionErrorCode
		::MatchPlayStateNotInitialized)

MARKER_LEGALITY_FAILURE_TEST(
	FMarkerLegalityNoAttackTest,
	"NoCurrentAttack",
	State.bHasCurrentAttack = false,
	EMatchPlayCurrentAttackMarkerSelectionErrorCode::NoCurrentAttack)

MARKER_LEGALITY_FAILURE_TEST(
	FMarkerLegalityInvalidSequenceTest,
	"InvalidAuthoritativeSequence",
	State.CurrentAttack.AttackSequence = 0,
	EMatchPlayCurrentAttackMarkerSelectionErrorCode
		::InvalidCurrentAttackSequence)

MARKER_LEGALITY_FAILURE_TEST(
	FMarkerLegalityStaleSequenceTest,
	"StaleSequence",
	Request.AttackSequence = ValidAttackSequence + 1,
	EMatchPlayCurrentAttackMarkerSelectionErrorCode
		::AttackSequenceMismatch)

MARKER_LEGALITY_FAILURE_TEST(
	FMarkerLegalityWrongPhaseTest,
	"WrongPhase",
	State.CurrentAttack.Phase =
		EMatchPlayCurrentAttackPhase::Deployment,
	EMatchPlayCurrentAttackMarkerSelectionErrorCode
		::CurrentAttackNotInResolution)

MARKER_LEGALITY_FAILURE_TEST(
	FMarkerLegalityInvalidAttackerTest,
	"InvalidAttacker",
	State.RuntimeState.CurrentAttackingPlayer =
		EInitialTurnOrderPlayer::None,
	EMatchPlayCurrentAttackMarkerSelectionErrorCode
		::InvalidCurrentAttackingPlayer)

MARKER_LEGALITY_FAILURE_TEST(
	FMarkerLegalityCorruptStateTest,
	"CorruptSelectionState",
	State.CurrentAttack.ActionPreparation.MarkerCardId = MarkerOneId,
	EMatchPlayCurrentAttackMarkerSelectionErrorCode
		::InvalidSelectionState)

MARKER_LEGALITY_FAILURE_TEST(
	FMarkerLegalityWrongStageTest,
	"WrongStage",
	State.CurrentAttack.ActionPreparation.MarkerCardId = MarkerOneId;
	State.CurrentAttack.SelectionStage =
		EMatchPlayCurrentAttackSelectionStage::AwaitingSkill,
	EMatchPlayCurrentAttackMarkerSelectionErrorCode
		::WrongSelectionStage)

MARKER_LEGALITY_FAILURE_TEST(
	FMarkerLegalityInvalidRequesterTest,
	"InvalidRequestingSide",
	Request.RequestingSide = EInitialTurnOrderPlayer::None,
	EMatchPlayCurrentAttackMarkerSelectionErrorCode
		::InvalidRequestingSide)

MARKER_LEGALITY_FAILURE_TEST(
	FMarkerLegalityAttackerRequesterTest,
	"AttackerCannotSelect",
	Request.RequestingSide = EInitialTurnOrderPlayer::PlayerA,
	EMatchPlayCurrentAttackMarkerSelectionErrorCode
		::RequestingSideIsNotCurrentDefender)

MARKER_LEGALITY_FAILURE_TEST(
	FMarkerLegalityEmptyMarkerTest,
	"EmptyMarkerId",
	Request.MarkerCardId = NAME_None,
	EMatchPlayCurrentAttackMarkerSelectionErrorCode
		::InvalidMarkerCardId)

MARKER_LEGALITY_FAILURE_TEST(
	FMarkerLegalityMissingFrozenCarrierTest,
	"MissingFrozenCarrierIsCorruptState",
	State.CurrentAttack.ActionPreparation.CarrierCardId = NAME_None,
	EMatchPlayCurrentAttackMarkerSelectionErrorCode
		::InvalidSelectionState)

MARKER_LEGALITY_FAILURE_TEST(
	FMarkerLegalityCarrierNotDeployedTest,
	"FrozenCarrierNotDeployed",
	State.CurrentAttack.DeploymentPlacements.RemoveAt(0),
	EMatchPlayCurrentAttackMarkerSelectionErrorCode
		::FrozenCarrierNotDeployed)

MARKER_LEGALITY_FAILURE_TEST(
	FMarkerLegalityCarrierAmbiguousTest,
	"FrozenCarrierAmbiguous",
	State.CurrentAttack.DeploymentPlacements.Add(
		MakePlacement(
			EInitialTurnOrderPlayer::PlayerA,
			CarrierId,
			MarkerTwoSlotId)),
	EMatchPlayCurrentAttackMarkerSelectionErrorCode
		::FrozenCarrierDeploymentAmbiguous)

MARKER_LEGALITY_FAILURE_TEST(
	FMarkerLegalityMarkerNotDeployedTest,
	"MarkerNotDeployed",
	Request.MarkerCardId = FName(TEXT("PlayerB.Unknown")),
	EMatchPlayCurrentAttackMarkerSelectionErrorCode
		::MarkerNotDeployed)

MARKER_LEGALITY_FAILURE_TEST(
	FMarkerLegalityNoAttackerFallbackTest,
	"NoFallbackToAttackerSameId",
	Request.MarkerCardId = AttackerOnlyMarkerId,
	EMatchPlayCurrentAttackMarkerSelectionErrorCode
		::MarkerNotDeployed)

MARKER_LEGALITY_FAILURE_TEST(
	FMarkerLegalityMarkerAmbiguousTest,
	"MarkerPlacementAmbiguous",
	State.CurrentAttack.DeploymentPlacements.Add(
		MakePlacement(
			EInitialTurnOrderPlayer::PlayerB,
			MarkerOneId,
			MarkerTwoSlotId)),
	EMatchPlayCurrentAttackMarkerSelectionErrorCode
		::MarkerDeploymentAmbiguous)

MARKER_LEGALITY_FAILURE_TEST(
	FMarkerLegalityMissingSnapshotTest,
	"MissingMarkerSnapshot",
	State.CurrentAttack.DeploymentPlacements.Add(
		MakePlacement(
			EInitialTurnOrderPlayer::PlayerB,
			MissingSnapshotMarkerId,
			MissingSnapshotSlotId));
	Request.MarkerCardId = MissingSnapshotMarkerId,
	EMatchPlayCurrentAttackMarkerSelectionErrorCode
		::MarkerSnapshotQueryFailed)

MARKER_LEGALITY_FAILURE_TEST(
	FMarkerLegalityInvalidSnapshotAuthorityTest,
	"InvalidMarkerSnapshotAuthority",
	State.CardSnapshotAuthority.PlayerBCardSnapshots.Cards.Add(
		MakeCard(MarkerOneId)),
	EMatchPlayCurrentAttackMarkerSelectionErrorCode
		::MarkerSnapshotQueryFailed)

MARKER_LEGALITY_FAILURE_TEST(
	FMarkerLegalityAreaFailureTest,
	"PhysicalAreaQueryFailure",
	State.CurrentAttack.DeploymentPlacements[1].SlotId =
		FName(TEXT("Physical.Unknown")),
	EMatchPlayCurrentAttackMarkerSelectionErrorCode
		::PhysicalAreaQueryFailed)

MARKER_LEGALITY_FAILURE_TEST(
	FMarkerLegalityDifferentAreaTest,
	"DifferentPhysicalArea",
	Request.MarkerCardId = OtherAreaMarkerId,
	EMatchPlayCurrentAttackMarkerSelectionErrorCode
		::MarkerNotInCarrierPhysicalArea)

MARKER_LEGALITY_FAILURE_TEST(
	FMarkerLegalityGoalkeeperTest,
	"GoalkeeperMarker",
	Request.MarkerCardId = GoalkeeperMarkerId,
	EMatchPlayCurrentAttackMarkerSelectionErrorCode
		::MarkerIsGoalkeeper)

MARKER_LEGALITY_TEST(
	FMarkerLegalityFirstErrorTest,
	"MixedInvalidUsesFrozenFirstError")

bool FMarkerLegalityFirstErrorTest::RunTest(const FString& Parameters)
{
	using namespace
		FMCodex::Tests::MatchPlayCurrentAttackMarkerSelection;
	FMatchPlayState State = MakeState();
	State.RuntimeState.bIsInitialized = false;
	State.CurrentAttack.AttackSequence = 0;
	FMatchPlayCurrentAttackMarkerSelectionRequest Request = MakeRequest();
	Request.RequestingSide = EInitialTurnOrderPlayer::None;
	Request.MarkerCardId = NAME_None;
	return MarkerSelectionLegalityTests::ExpectFailure(
		*this,
		TEXT("Mixed invalid"),
		State,
		Request,
		EMatchPlayCurrentAttackMarkerSelectionErrorCode
			::MatchPlayStateNotInitialized);
}

#undef MARKER_LEGALITY_FAILURE_TEST
#undef MARKER_LEGALITY_TEST

#endif
