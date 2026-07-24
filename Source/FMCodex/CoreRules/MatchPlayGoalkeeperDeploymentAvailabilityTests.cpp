#include "MatchPlayGoalkeeperDeploymentAvailability.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

namespace FMCodex::Tests::MatchPlayGoalkeeperDeploymentAvailability
{
	constexpr int64 ValidAttackSequence = 17;
	const FName PlayerAGoalkeeperCardId(TEXT("PlayerA.GK"));
	const FName PlayerAOutfieldCardId(TEXT("PlayerA.Outfield"));
	const FName PlayerBGoalkeeperCardId(TEXT("PlayerB.GK"));
	const FName PlayerBOutfieldCardId(TEXT("PlayerB.Outfield"));
	const FName BackfieldFirstSlotId(TEXT("Slot.Backfield.First"));
	const FName MidfieldSecondSlotId(TEXT("Slot.Midfield.Second"));
	const FName BackfieldThirdSlotId(TEXT("Slot.Backfield.Third"));

	FPlayerCardRuleSnapshot MakeGoalkeeperAvailabilitySnapshot(
		const FName CardId,
		const bool bIsGoalkeeper)
	{
		FPlayerCardRuleSnapshot Snapshot;
		Snapshot.CardId = CardId;
		Snapshot.PositionTypes = {
			bIsGoalkeeper
				? EPlayerPositionType::Goalkeeper
				: EPlayerPositionType::Defense
		};
		Snapshot.bIsGoalkeeper = bIsGoalkeeper;
		Snapshot.bHasGoalkeeperAttributes = bIsGoalkeeper;
		return Snapshot;
	}

	FMatchPlayDeploymentSlotDefinition MakeGoalkeeperAvailabilitySlot(
		const FName SlotId,
		const EMatchPlayNeutralSlotSide NeutralSide)
	{
		FMatchPlayDeploymentSlotDefinition Slot;
		Slot.SlotId = SlotId;
		Slot.NeutralSide = NeutralSide;
		return Slot;
	}

	FMatchPlayDeploymentPlacement MakeGoalkeeperAvailabilityPlacement(
		const EInitialTurnOrderPlayer PlayerSide,
		const FName CardId,
		const FName SlotId)
	{
		FMatchPlayDeploymentPlacement Placement;
		Placement.PlayerSide = PlayerSide;
		Placement.CardId = CardId;
		Placement.SlotId = SlotId;
		return Placement;
	}

	FMatchPlayState MakeGoalkeeperAvailabilityState()
	{
		FMatchPlayState State;
		State.RuntimeState.bIsInitialized = true;
		State.RuntimeState.CurrentAttackingPlayer =
			EInitialTurnOrderPlayer::PlayerA;
		State.RuntimeState.PlayerAState.TotalAttackCount = 3;
		State.RuntimeState.PlayerBState.TotalAttackCount = 3;
		State.CardUsageState.PlayerACardUsageState.AvailableCardIds = {
			PlayerAGoalkeeperCardId,
			PlayerAOutfieldCardId
		};
		State.CardUsageState.PlayerBCardUsageState.AvailableCardIds = {
			PlayerBGoalkeeperCardId,
			PlayerBOutfieldCardId
		};
		State.CardSnapshotAuthority.PlayerACardSnapshots.Cards = {
			MakeGoalkeeperAvailabilitySnapshot(
				PlayerAGoalkeeperCardId,
				true),
			MakeGoalkeeperAvailabilitySnapshot(
				PlayerAOutfieldCardId,
				false)
		};
		State.CardSnapshotAuthority.PlayerBCardSnapshots.Cards = {
			MakeGoalkeeperAvailabilitySnapshot(
				PlayerBGoalkeeperCardId,
				true),
			MakeGoalkeeperAvailabilitySnapshot(
				PlayerBOutfieldCardId,
				false)
		};
		State.DeploymentSlotCatalog.Slots = {
			MakeGoalkeeperAvailabilitySlot(
				BackfieldFirstSlotId,
				EMatchPlayNeutralSlotSide::NearPlayerB),
			MakeGoalkeeperAvailabilitySlot(
				MidfieldSecondSlotId,
				EMatchPlayNeutralSlotSide::NearPlayerA),
			MakeGoalkeeperAvailabilitySlot(
				BackfieldThirdSlotId,
				EMatchPlayNeutralSlotSide::NearPlayerB)
		};
		State.bHasCurrentAttack = true;
		State.CurrentAttack.Phase =
			EMatchPlayCurrentAttackPhase::Deployment;
		State.CurrentAttack.AttackSequence = ValidAttackSequence;
		State.CurrentAttack.CurrentLegalDeploymentSide =
			EInitialTurnOrderPlayer::PlayerB;
		return State;
	}

	FMatchPlayGoalkeeperDeploymentAvailabilityResult QueryGoalkeeperAvailability(
		const FMatchPlayState& State,
		const int64 AttackSequence = ValidAttackSequence,
		const EInitialTurnOrderPlayer RequestingSide =
			EInitialTurnOrderPlayer::PlayerB,
		const FName CardId = PlayerBGoalkeeperCardId)
	{
		return FMatchPlayGoalkeeperDeploymentAvailability::Query(
			State,
			AttackSequence,
			RequestingSide,
			CardId);
	}

	bool AreGoalkeeperAvailabilityStatesEqual(
		const FMatchPlayState& Left,
		const FMatchPlayState& Right)
	{
		return FMatchPlayState::StaticStruct()->CompareScriptStruct(
			&Left,
			&Right,
			0);
	}

	bool AreGoalkeeperLegalityResultsEqual(
		const FMatchPlayGoalkeeperDeploymentLegalityResult& Left,
		const FMatchPlayGoalkeeperDeploymentLegalityResult& Right)
	{
		return FMatchPlayGoalkeeperDeploymentLegalityResult::StaticStruct()
				->CompareScriptStruct(&Left, &Right, 0)
			&& Left.UnderlyingSnapshotAuthorityQueryErrorCode
				== Right.UnderlyingSnapshotAuthorityQueryErrorCode;
	}

	bool ExpectGoalkeeperAvailabilityGlobalBlocker(
		FAutomationTestBase& Test,
		const FMatchPlayGoalkeeperDeploymentAvailabilityResult& Result,
		const EMatchPlayGoalkeeperDeploymentErrorCode ExpectedError)
	{
		Test.TestTrue(TEXT("Global blocker still completes the query"),
			Result.bQuerySucceeded);
		Test.TestFalse(TEXT("Global blocker yields no legal slot"),
			Result.bCanDeployToAnySlot);
		Test.TestEqual(TEXT("Global blocker avoids fabricated per-slot results"),
			Result.SlotResults.Num(), 0);
		Test.TestTrue(TEXT("Global blocker is retained"),
			Result.bHasFirstBlockingLegalityResult);
		Test.TestEqual(TEXT("Global blocker error is exact"),
			Result.FirstBlockingLegalityResult.ErrorCode,
			ExpectedError);
		return true;
	}
}

#define MATCH_PLAY_GOALKEEPER_DEPLOYMENT_AVAILABILITY_TEST( \
	TestClass, TestName) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
		TestClass, \
		"FMCodex.CoreRules.MatchPlayGoalkeeperDeployment.Availability." \
			TestName, \
		EAutomationTestFlags::EditorContext \
			| EAutomationTestFlags::EngineFilter)

MATCH_PLAY_GOALKEEPER_DEPLOYMENT_AVAILABILITY_TEST(
	FMatchPlayGoalkeeperAvailabilityOrderTest,
	"CatalogOrderAndBackfieldLegalSlots")

bool FMatchPlayGoalkeeperAvailabilityOrderTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayGoalkeeperDeploymentAvailability;
	const FMatchPlayState State = MakeGoalkeeperAvailabilityState();
	const FMatchPlayGoalkeeperDeploymentAvailabilityResult Result =
		QueryGoalkeeperAvailability(State);
	TestTrue(TEXT("Query succeeds"), Result.bQuerySucceeded);
	TestTrue(TEXT("At least one slot is legal"),
		Result.bCanDeployToAnySlot);
	TestEqual(TEXT("Every Catalog slot has a result"),
		Result.SlotResults.Num(), 3);
	for (int32 Index = 0; Index < Result.SlotResults.Num(); ++Index)
	{
		TestEqual(TEXT("Slot results preserve Catalog order"),
			Result.SlotResults[Index].SlotId,
			State.DeploymentSlotCatalog.Slots[Index].SlotId);
	}
	TestEqual(TEXT("Only the two Backfield slots are legal"),
		Result.LegalSlotIds.Num(), 2);
	TestEqual(TEXT("First Backfield slot preserves order"),
		Result.LegalSlotIds[0], BackfieldFirstSlotId);
	TestEqual(TEXT("Second Backfield slot preserves order"),
		Result.LegalSlotIds[1], BackfieldThirdSlotId);
	TestEqual(TEXT("Midfield candidate is precisely rejected"),
		Result.SlotResults[1].LegalityResult.ErrorCode,
		EMatchPlayGoalkeeperDeploymentErrorCode
			::GoalkeeperSlotNotInDefenderBackfield);
	return true;
}

MATCH_PLAY_GOALKEEPER_DEPLOYMENT_AVAILABILITY_TEST(
	FMatchPlayGoalkeeperAvailabilityEvaluatorReuseTest,
	"EveryCandidateRetainsCompleteEvaluatorResult")

bool FMatchPlayGoalkeeperAvailabilityEvaluatorReuseTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayGoalkeeperDeploymentAvailability;
	const FMatchPlayState State = MakeGoalkeeperAvailabilityState();
	const FMatchPlayGoalkeeperDeploymentAvailabilityResult Result =
		QueryGoalkeeperAvailability(State);
	for (const FMatchPlayGoalkeeperDeploymentSlotAvailability& SlotResult :
		Result.SlotResults)
	{
		FMatchPlayGoalkeeperDeploymentRequest Request;
		Request.AttackSequence = ValidAttackSequence;
		Request.RequestingSide = EInitialTurnOrderPlayer::PlayerB;
		Request.CardId = PlayerBGoalkeeperCardId;
		Request.SlotId = SlotResult.SlotId;
		const FMatchPlayGoalkeeperDeploymentLegalityResult Direct =
			FMatchPlayGoalkeeperDeploymentLegalityEvaluator::Evaluate(
				State,
				Request);
		TestTrue(TEXT("Availability retains the complete evaluator result"),
			AreGoalkeeperLegalityResultsEqual(
				SlotResult.LegalityResult,
				Direct));
	}
	return true;
}

MATCH_PLAY_GOALKEEPER_DEPLOYMENT_AVAILABILITY_TEST(
	FMatchPlayGoalkeeperAvailabilityAttackerTest,
	"AttackerRequestIsGlobalBlocker")

bool FMatchPlayGoalkeeperAvailabilityAttackerTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayGoalkeeperDeploymentAvailability;
	FMatchPlayState State = MakeGoalkeeperAvailabilityState();
	State.CurrentAttack.CurrentLegalDeploymentSide =
		EInitialTurnOrderPlayer::PlayerA;
	return ExpectGoalkeeperAvailabilityGlobalBlocker(
		*this,
		QueryGoalkeeperAvailability(
			State,
			ValidAttackSequence,
			EInitialTurnOrderPlayer::PlayerA,
			PlayerAGoalkeeperCardId),
		EMatchPlayGoalkeeperDeploymentErrorCode
			::RequestingSideIsNotDefender);
}

MATCH_PLAY_GOALKEEPER_DEPLOYMENT_AVAILABILITY_TEST(
	FMatchPlayGoalkeeperAvailabilityWrongLegalSideTest,
	"WrongLegalSideIsGlobalBlocker")

bool FMatchPlayGoalkeeperAvailabilityWrongLegalSideTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayGoalkeeperDeploymentAvailability;
	FMatchPlayState State = MakeGoalkeeperAvailabilityState();
	State.CurrentAttack.CurrentLegalDeploymentSide =
		EInitialTurnOrderPlayer::PlayerA;
	return ExpectGoalkeeperAvailabilityGlobalBlocker(
		*this,
		QueryGoalkeeperAvailability(State),
		EMatchPlayGoalkeeperDeploymentErrorCode
			::RequestingSideNotCurrentLegalDeploymentSide);
}

MATCH_PLAY_GOALKEEPER_DEPLOYMENT_AVAILABILITY_TEST(
	FMatchPlayGoalkeeperAvailabilityFinishedTest,
	"FinishedDefenderIsGlobalBlocker")

bool FMatchPlayGoalkeeperAvailabilityFinishedTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayGoalkeeperDeploymentAvailability;
	FMatchPlayState State = MakeGoalkeeperAvailabilityState();
	State.CurrentAttack.bDefenderDeploymentFinished = true;
	return ExpectGoalkeeperAvailabilityGlobalBlocker(
		*this,
		QueryGoalkeeperAvailability(State),
		EMatchPlayGoalkeeperDeploymentErrorCode
			::RequestingSideAlreadyFinished);
}

MATCH_PLAY_GOALKEEPER_DEPLOYMENT_AVAILABILITY_TEST(
	FMatchPlayGoalkeeperAvailabilityStaleTest,
	"StaleSequenceIsGlobalBlocker")

bool FMatchPlayGoalkeeperAvailabilityStaleTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayGoalkeeperDeploymentAvailability;
	return ExpectGoalkeeperAvailabilityGlobalBlocker(
		*this,
		QueryGoalkeeperAvailability(
			MakeGoalkeeperAvailabilityState(),
			ValidAttackSequence + 1),
		EMatchPlayGoalkeeperDeploymentErrorCode::AttackSequenceMismatch);
}

MATCH_PLAY_GOALKEEPER_DEPLOYMENT_AVAILABILITY_TEST(
	FMatchPlayGoalkeeperAvailabilityNonGoalkeeperTest,
	"NonGoalkeeperIsGlobalBlocker")

bool FMatchPlayGoalkeeperAvailabilityNonGoalkeeperTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayGoalkeeperDeploymentAvailability;
	return ExpectGoalkeeperAvailabilityGlobalBlocker(
		*this,
		QueryGoalkeeperAvailability(
			MakeGoalkeeperAvailabilityState(),
			ValidAttackSequence,
			EInitialTurnOrderPlayer::PlayerB,
			PlayerBOutfieldCardId),
		EMatchPlayGoalkeeperDeploymentErrorCode::CardIsNotGoalkeeper);
}

MATCH_PLAY_GOALKEEPER_DEPLOYMENT_AVAILABILITY_TEST(
	FMatchPlayGoalkeeperAvailabilityUnavailableTest,
	"UnavailableGoalkeeperIsGlobalBlocker")

bool FMatchPlayGoalkeeperAvailabilityUnavailableTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayGoalkeeperDeploymentAvailability;
	FMatchPlayState State = MakeGoalkeeperAvailabilityState();
	State.CardUsageState.PlayerBCardUsageState.AvailableCardIds.Remove(
		PlayerBGoalkeeperCardId);
	return ExpectGoalkeeperAvailabilityGlobalBlocker(
		*this,
		QueryGoalkeeperAvailability(State),
		EMatchPlayGoalkeeperDeploymentErrorCode::CardNotAvailable);
}

MATCH_PLAY_GOALKEEPER_DEPLOYMENT_AVAILABILITY_TEST(
	FMatchPlayGoalkeeperAvailabilityUsedTest,
	"PersistentUseIsGlobalBlocker")

bool FMatchPlayGoalkeeperAvailabilityUsedTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayGoalkeeperDeploymentAvailability;
	FMatchPlayState State = MakeGoalkeeperAvailabilityState();
	State.GoalkeeperUsageState.bPlayerBGoalkeeperCardUsed = true;
	return ExpectGoalkeeperAvailabilityGlobalBlocker(
		*this,
		QueryGoalkeeperAvailability(State),
		EMatchPlayGoalkeeperDeploymentErrorCode
			::GoalkeeperAlreadyUsedThisMatch);
}

MATCH_PLAY_GOALKEEPER_DEPLOYMENT_AVAILABILITY_TEST(
	FMatchPlayGoalkeeperAvailabilityActivatedTest,
	"CurrentActivationIsGlobalBlocker")

bool FMatchPlayGoalkeeperAvailabilityActivatedTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayGoalkeeperDeploymentAvailability;
	FMatchPlayState State = MakeGoalkeeperAvailabilityState();
	State.GoalkeeperUsageState.bPlayerBGoalkeeperCardUsed = true;
	State.CurrentAttack.bCurrentDefenseGoalkeeperActivated = true;
	State.CurrentAttack.DeploymentPlacements.Add(
		MakeGoalkeeperAvailabilityPlacement(
			EInitialTurnOrderPlayer::PlayerB,
			PlayerBGoalkeeperCardId,
			BackfieldFirstSlotId));
	return ExpectGoalkeeperAvailabilityGlobalBlocker(
		*this,
		QueryGoalkeeperAvailability(State),
		EMatchPlayGoalkeeperDeploymentErrorCode
			::GoalkeeperAlreadyActivatedThisAttack);
}

MATCH_PLAY_GOALKEEPER_DEPLOYMENT_AVAILABILITY_TEST(
	FMatchPlayGoalkeeperAvailabilityInconsistentUsageTest,
	"InconsistentUsageIsGlobalBlocker")

bool FMatchPlayGoalkeeperAvailabilityInconsistentUsageTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayGoalkeeperDeploymentAvailability;
	FMatchPlayState State = MakeGoalkeeperAvailabilityState();
	State.CurrentAttack.bCurrentDefenseGoalkeeperActivated = true;
	return ExpectGoalkeeperAvailabilityGlobalBlocker(
		*this,
		QueryGoalkeeperAvailability(State),
		EMatchPlayGoalkeeperDeploymentErrorCode
			::InvalidGoalkeeperUsageState);
}

MATCH_PLAY_GOALKEEPER_DEPLOYMENT_AVAILABILITY_TEST(
	FMatchPlayGoalkeeperAvailabilityOccupiedTypesTest,
	"OrdinaryAndGoalkeeperOccupantsShareSlots")

bool FMatchPlayGoalkeeperAvailabilityOccupiedTypesTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayGoalkeeperDeploymentAvailability;
	FMatchPlayState State = MakeGoalkeeperAvailabilityState();
	State.CurrentAttack.DeploymentPlacements.Add(
		MakeGoalkeeperAvailabilityPlacement(
			EInitialTurnOrderPlayer::PlayerA,
			PlayerAOutfieldCardId,
			BackfieldFirstSlotId));
	State.CurrentAttack.DeploymentPlacements.Add(
		MakeGoalkeeperAvailabilityPlacement(
			EInitialTurnOrderPlayer::PlayerA,
			PlayerAGoalkeeperCardId,
			BackfieldThirdSlotId));
	const FMatchPlayGoalkeeperDeploymentAvailabilityResult Result =
		QueryGoalkeeperAvailability(State);
	TestTrue(TEXT("Zero legal slots is still a successful query"),
		Result.bQuerySucceeded);
	TestFalse(TEXT("Both occupied Backfield slots are excluded"),
		Result.bCanDeployToAnySlot);
	TestEqual(TEXT("All Catalog candidates are retained"),
		Result.SlotResults.Num(), 3);
	TestEqual(TEXT("Ordinary occupant blocks globally"),
		Result.SlotResults[0].LegalityResult.ErrorCode,
		EMatchPlayGoalkeeperDeploymentErrorCode::SlotAlreadyOccupied);
	TestEqual(TEXT("Goalkeeper occupant blocks globally"),
		Result.SlotResults[2].LegalityResult.ErrorCode,
		EMatchPlayGoalkeeperDeploymentErrorCode::SlotAlreadyOccupied);
	return true;
}

MATCH_PLAY_GOALKEEPER_DEPLOYMENT_AVAILABILITY_TEST(
	FMatchPlayGoalkeeperAvailabilityPartialOccupancyTest,
	"OccupiedBackfieldIsRemovedWithoutReordering")

bool FMatchPlayGoalkeeperAvailabilityPartialOccupancyTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayGoalkeeperDeploymentAvailability;
	FMatchPlayState State = MakeGoalkeeperAvailabilityState();
	State.CurrentAttack.DeploymentPlacements.Add(
		MakeGoalkeeperAvailabilityPlacement(
			EInitialTurnOrderPlayer::PlayerA,
			PlayerAOutfieldCardId,
			BackfieldFirstSlotId));
	const FMatchPlayGoalkeeperDeploymentAvailabilityResult Result =
		QueryGoalkeeperAvailability(State);
	TestTrue(TEXT("Query succeeds"), Result.bQuerySucceeded);
	TestTrue(TEXT("One Backfield slot remains"), Result.bCanDeployToAnySlot);
	TestEqual(TEXT("Only one slot is legal"), Result.LegalSlotIds.Num(), 1);
	TestEqual(TEXT("Remaining slot preserves Catalog order"),
		Result.LegalSlotIds[0], BackfieldThirdSlotId);
	return true;
}

MATCH_PLAY_GOALKEEPER_DEPLOYMENT_AVAILABILITY_TEST(
	FMatchPlayGoalkeeperAvailabilityNoBackfieldTest,
	"NoBackfieldIsSuccessfulZeroSlotQuery")

bool FMatchPlayGoalkeeperAvailabilityNoBackfieldTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayGoalkeeperDeploymentAvailability;
	FMatchPlayState State = MakeGoalkeeperAvailabilityState();
	for (FMatchPlayDeploymentSlotDefinition& Slot :
		State.DeploymentSlotCatalog.Slots)
	{
		Slot.NeutralSide = EMatchPlayNeutralSlotSide::NearPlayerA;
	}
	const FMatchPlayGoalkeeperDeploymentAvailabilityResult Result =
		QueryGoalkeeperAvailability(State);
	TestTrue(TEXT("Valid Catalog enumeration succeeds"),
		Result.bQuerySucceeded);
	TestFalse(TEXT("No Backfield means no legal slot"),
		Result.bCanDeployToAnySlot);
	TestEqual(TEXT("No legal slots are fabricated"),
		Result.LegalSlotIds.Num(), 0);
	for (const FMatchPlayGoalkeeperDeploymentSlotAvailability& SlotResult :
		Result.SlotResults)
	{
		TestEqual(TEXT("Every non-Backfield slot has the exact error"),
			SlotResult.LegalityResult.ErrorCode,
			EMatchPlayGoalkeeperDeploymentErrorCode
				::GoalkeeperSlotNotInDefenderBackfield);
	}
	return true;
}

MATCH_PLAY_GOALKEEPER_DEPLOYMENT_AVAILABILITY_TEST(
	FMatchPlayGoalkeeperAvailabilityInvalidCatalogTest,
	"InvalidCatalogFailsEnumeration")

bool FMatchPlayGoalkeeperAvailabilityInvalidCatalogTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayGoalkeeperDeploymentAvailability;
	FMatchPlayState State = MakeGoalkeeperAvailabilityState();
	State.DeploymentSlotCatalog.Slots.Reset();
	const FMatchPlayGoalkeeperDeploymentAvailabilityResult Result =
		QueryGoalkeeperAvailability(State);
	TestFalse(TEXT("Unsafe Catalog enumeration fails"),
		Result.bQuerySucceeded);
	TestFalse(TEXT("Failed query claims no legal slot"),
		Result.bCanDeployToAnySlot);
	TestEqual(TEXT("Availability error is exact"), Result.ErrorCode,
		EMatchPlayGoalkeeperDeploymentAvailabilityErrorCode
			::CatalogEnumerationFailed);
	TestEqual(TEXT("Underlying Catalog error is exact"),
		Result.UnderlyingCatalogValidationErrorCode,
		EMatchPlayDeploymentSlotCatalogValidationErrorCode::EmptyCatalog);
	TestEqual(TEXT("No candidate SlotId is fabricated"),
		Result.SlotResults.Num(), 0);
	return true;
}

MATCH_PLAY_GOALKEEPER_DEPLOYMENT_AVAILABILITY_TEST(
	FMatchPlayGoalkeeperAvailabilityDeterminismTest,
	"RepeatedQueryIsDeterministicAndReadOnly")

bool FMatchPlayGoalkeeperAvailabilityDeterminismTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayGoalkeeperDeploymentAvailability;
	const FMatchPlayState State = MakeGoalkeeperAvailabilityState();
	const FMatchPlayState OriginalState = State;
	const FMatchPlayGoalkeeperDeploymentAvailabilityResult First =
		QueryGoalkeeperAvailability(State);
	const FMatchPlayGoalkeeperDeploymentAvailabilityResult Second =
		QueryGoalkeeperAvailability(State);
	TestTrue(TEXT("Query never mutates State"),
		AreGoalkeeperAvailabilityStatesEqual(State, OriginalState));
	TestTrue(TEXT("Repeated result is reflected-identical"),
		FMatchPlayGoalkeeperDeploymentAvailabilityResult::StaticStruct()
			->CompareScriptStruct(&First, &Second, 0));
	return true;
}

#undef MATCH_PLAY_GOALKEEPER_DEPLOYMENT_AVAILABILITY_TEST

#endif
