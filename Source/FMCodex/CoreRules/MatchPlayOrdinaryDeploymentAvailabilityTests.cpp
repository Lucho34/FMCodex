#include "MatchPlayOrdinaryDeploymentAvailability.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

namespace FMCodex::Tests::MatchPlayOrdinaryDeploymentAvailability
{
	constexpr int64 ValidAttackSequence = 9;
	const FName PlayerACardId(TEXT("PlayerA.Defender"));
	const FName PlayerAGoalkeeperCardId(TEXT("PlayerA.GK"));
	const FName PlayerBCardId(TEXT("PlayerB.Attacker"));
	const FName NearPlayerBFirst(TEXT("Slot.Forward.First"));
	const FName NearPlayerASecond(TEXT("Slot.Midfield.Second"));
	const FName NearPlayerBThird(TEXT("Slot.Forward.Third"));

	FPlayerCardRuleSnapshot MakeSnapshot(
		const FName CardId,
		const EPlayerPositionType Position,
		const bool bIsGoalkeeper = false)
	{
		FPlayerCardRuleSnapshot Snapshot;
		Snapshot.CardId = CardId;
		Snapshot.PositionTypes = { Position };
		Snapshot.bIsGoalkeeper = bIsGoalkeeper;
		Snapshot.bHasGoalkeeperAttributes = bIsGoalkeeper;
		return Snapshot;
	}

	FMatchPlayDeploymentSlotDefinition MakeSlot(
		const FName SlotId,
		const EMatchPlayNeutralSlotSide NeutralSide)
	{
		FMatchPlayDeploymentSlotDefinition Slot;
		Slot.SlotId = SlotId;
		Slot.NeutralSide = NeutralSide;
		return Slot;
	}

	FMatchPlayState MakeState()
	{
		FMatchPlayState State;
		State.RuntimeState.bIsInitialized = true;
		State.RuntimeState.CurrentAttackingPlayer =
			EInitialTurnOrderPlayer::PlayerA;
		State.RuntimeState.PlayerAState.TotalAttackCount = 3;
		State.RuntimeState.PlayerBState.TotalAttackCount = 3;
		State.CardUsageState.PlayerACardUsageState.AvailableCardIds = {
			PlayerACardId,
			PlayerAGoalkeeperCardId
		};
		State.CardUsageState.PlayerBCardUsageState.AvailableCardIds = {
			PlayerBCardId
		};
		State.CardSnapshotAuthority.PlayerACardSnapshots.Cards = {
			MakeSnapshot(PlayerACardId, EPlayerPositionType::Defense),
			MakeSnapshot(
				PlayerAGoalkeeperCardId,
				EPlayerPositionType::Goalkeeper,
				true)
		};
		State.CardSnapshotAuthority.PlayerBCardSnapshots.Cards = {
			MakeSnapshot(PlayerBCardId, EPlayerPositionType::Attack)
		};
		State.DeploymentSlotCatalog.Slots = {
			MakeSlot(
				NearPlayerBFirst,
				EMatchPlayNeutralSlotSide::NearPlayerB),
			MakeSlot(
				NearPlayerASecond,
				EMatchPlayNeutralSlotSide::NearPlayerA),
			MakeSlot(
				NearPlayerBThird,
				EMatchPlayNeutralSlotSide::NearPlayerB)
		};
		State.bHasCurrentAttack = true;
		State.CurrentAttack.Phase =
			EMatchPlayCurrentAttackPhase::Deployment;
		State.CurrentAttack.AttackSequence = ValidAttackSequence;
		State.CurrentAttack.CurrentLegalDeploymentSide =
			EInitialTurnOrderPlayer::PlayerA;
		return State;
	}

	bool AreStatesEqual(
		const FMatchPlayState& Left,
		const FMatchPlayState& Right)
	{
		return FMatchPlayState::StaticStruct()->CompareScriptStruct(
			&Left,
			&Right,
			0);
	}

	FMatchPlayOrdinaryDeploymentAvailabilityResult Query(
		const FMatchPlayState& State,
		const int64 AttackSequence = ValidAttackSequence,
		const EInitialTurnOrderPlayer Side =
			EInitialTurnOrderPlayer::PlayerA,
		const FName CardId = PlayerACardId)
	{
		return FMatchPlayOrdinaryDeploymentAvailability::Query(
			State,
			AttackSequence,
			Side,
			CardId);
	}
}

#define MATCH_PLAY_ORDINARY_DEPLOYMENT_AVAILABILITY_TEST(TestClass, TestName) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
		TestClass, \
		"FMCodex.CoreRules.MatchPlayOrdinaryDeployment.Availability." TestName, \
		EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

MATCH_PLAY_ORDINARY_DEPLOYMENT_AVAILABILITY_TEST(
	FMatchPlayOrdinaryDeploymentAvailabilityOrderTest,
	"CatalogOrderAndLegalSlots")

bool FMatchPlayOrdinaryDeploymentAvailabilityOrderTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayOrdinaryDeploymentAvailability;
	const FMatchPlayState State = MakeState();
	const FMatchPlayOrdinaryDeploymentAvailabilityResult Result = Query(State);
	TestTrue(TEXT("Query succeeds"), Result.bQuerySucceeded);
	TestTrue(TEXT("At least one slot is legal"), Result.bCanDeployToAnySlot);
	TestEqual(TEXT("All Catalog slots have results"),
		Result.SlotResults.Num(), 3);
	for (int32 Index = 0; Index < Result.SlotResults.Num(); ++Index)
	{
		TestEqual(TEXT("Slot result preserves Catalog order"),
			Result.SlotResults[Index].SlotId,
			State.DeploymentSlotCatalog.Slots[Index].SlotId);
	}
	TestEqual(TEXT("Only one Slot is legal"), Result.LegalSlotIds.Num(), 1);
	TestEqual(TEXT("Midfield Slot remains legal for Defense"),
		Result.LegalSlotIds[0], NearPlayerASecond);
	return true;
}

MATCH_PLAY_ORDINARY_DEPLOYMENT_AVAILABILITY_TEST(
	FMatchPlayOrdinaryDeploymentAvailabilityEvaluatorReuseTest,
	"EveryCandidateMatchesEvaluator")

bool FMatchPlayOrdinaryDeploymentAvailabilityEvaluatorReuseTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayOrdinaryDeploymentAvailability;
	const FMatchPlayState State = MakeState();
	const FMatchPlayOrdinaryDeploymentAvailabilityResult Result = Query(State);
	for (const FMatchPlayOrdinaryDeploymentSlotAvailability& SlotResult :
		Result.SlotResults)
	{
		FMatchPlayOrdinaryDeploymentRequest Request;
		Request.AttackSequence = ValidAttackSequence;
		Request.RequestingSide = EInitialTurnOrderPlayer::PlayerA;
		Request.CardId = PlayerACardId;
		Request.SlotId = SlotResult.SlotId;
		const FMatchPlayOrdinaryDeploymentLegalityResult Direct =
			FMatchPlayOrdinaryDeploymentLegalityEvaluator::Evaluate(
				State,
				Request);
		TestEqual(TEXT("Availability legality matches evaluator"),
			SlotResult.LegalityResult.bIsLegal, Direct.bIsLegal);
		TestEqual(TEXT("Availability error matches evaluator"),
			SlotResult.LegalityResult.ErrorCode, Direct.ErrorCode);
		TestEqual(TEXT("Availability zone matches evaluator"),
			SlotResult.LegalityResult.ResolvedRelativeZone,
			Direct.ResolvedRelativeZone);
	}
	return true;
}

MATCH_PLAY_ORDINARY_DEPLOYMENT_AVAILABILITY_TEST(
	FMatchPlayOrdinaryDeploymentAvailabilityWrongSideTest,
	"WrongSideIsGlobalBlocker")

bool FMatchPlayOrdinaryDeploymentAvailabilityWrongSideTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayOrdinaryDeploymentAvailability;
	const FMatchPlayOrdinaryDeploymentAvailabilityResult Result = Query(
		MakeState(),
		ValidAttackSequence,
		EInitialTurnOrderPlayer::PlayerB,
		PlayerBCardId);
	TestTrue(TEXT("Global blocker is a completed query"),
		Result.bQuerySucceeded);
	TestFalse(TEXT("No slot is deployable"), Result.bCanDeployToAnySlot);
	TestEqual(TEXT("Global blocker avoids duplicated slot results"),
		Result.SlotResults.Num(), 0);
	TestTrue(TEXT("First blocker is present"),
		Result.bHasFirstBlockingLegalityResult);
	TestEqual(TEXT("Wrong side remains exact"),
		Result.FirstBlockingLegalityResult.ErrorCode,
		EMatchPlayOrdinaryDeploymentErrorCode
			::RequestingSideNotCurrentLegalDeploymentSide);
	return true;
}

MATCH_PLAY_ORDINARY_DEPLOYMENT_AVAILABILITY_TEST(
	FMatchPlayOrdinaryDeploymentAvailabilityFinishedTest,
	"FinishedSideIsGlobalBlocker")

bool FMatchPlayOrdinaryDeploymentAvailabilityFinishedTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayOrdinaryDeploymentAvailability;
	FMatchPlayState State = MakeState();
	State.CurrentAttack.bAttackerDeploymentFinished = true;
	const FMatchPlayOrdinaryDeploymentAvailabilityResult Result = Query(State);
	TestTrue(TEXT("Query succeeds"), Result.bQuerySucceeded);
	TestFalse(TEXT("Finished side has no slot"), Result.bCanDeployToAnySlot);
	TestEqual(TEXT("Finished side blocker is exact"),
		Result.FirstBlockingLegalityResult.ErrorCode,
		EMatchPlayOrdinaryDeploymentErrorCode::RequestingSideAlreadyFinished);
	return true;
}

MATCH_PLAY_ORDINARY_DEPLOYMENT_AVAILABILITY_TEST(
	FMatchPlayOrdinaryDeploymentAvailabilityGoalkeeperTest,
	"GoalkeeperIsGlobalBlocker")

bool FMatchPlayOrdinaryDeploymentAvailabilityGoalkeeperTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayOrdinaryDeploymentAvailability;
	const FMatchPlayOrdinaryDeploymentAvailabilityResult Result = Query(
		MakeState(),
		ValidAttackSequence,
		EInitialTurnOrderPlayer::PlayerA,
		PlayerAGoalkeeperCardId);
	TestTrue(TEXT("Query succeeds"), Result.bQuerySucceeded);
	TestFalse(TEXT("GK has no ordinary deployment slot"),
		Result.bCanDeployToAnySlot);
	TestEqual(TEXT("GK blocker is exact"),
		Result.FirstBlockingLegalityResult.ErrorCode,
		EMatchPlayOrdinaryDeploymentErrorCode::GoalkeeperNotAllowed);
	return true;
}

MATCH_PLAY_ORDINARY_DEPLOYMENT_AVAILABILITY_TEST(
	FMatchPlayOrdinaryDeploymentAvailabilityStaleTest,
	"StaleSequenceIsGlobalBlocker")

bool FMatchPlayOrdinaryDeploymentAvailabilityStaleTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayOrdinaryDeploymentAvailability;
	const FMatchPlayOrdinaryDeploymentAvailabilityResult Result = Query(
		MakeState(),
		ValidAttackSequence + 1);
	TestTrue(TEXT("Query succeeds"), Result.bQuerySucceeded);
	TestFalse(TEXT("Stale request has no slot"),
		Result.bCanDeployToAnySlot);
	TestEqual(TEXT("Sequence blocker is exact"),
		Result.FirstBlockingLegalityResult.ErrorCode,
		EMatchPlayOrdinaryDeploymentErrorCode::AttackSequenceMismatch);
	return true;
}

MATCH_PLAY_ORDINARY_DEPLOYMENT_AVAILABILITY_TEST(
	FMatchPlayOrdinaryDeploymentAvailabilityAllOccupiedTest,
	"AllSlotsOccupied")

bool FMatchPlayOrdinaryDeploymentAvailabilityAllOccupiedTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayOrdinaryDeploymentAvailability;
	FMatchPlayState State = MakeState();
	for (const FMatchPlayDeploymentSlotDefinition& Slot :
		State.DeploymentSlotCatalog.Slots)
	{
		FMatchPlayDeploymentPlacement Placement;
		Placement.PlayerSide = EInitialTurnOrderPlayer::PlayerB;
		Placement.CardId = FName(*FString::Printf(
			TEXT("Occupant.%s"), *Slot.SlotId.ToString()));
		Placement.SlotId = Slot.SlotId;
		State.CurrentAttack.DeploymentPlacements.Add(Placement);
	}
	const FMatchPlayOrdinaryDeploymentAvailabilityResult Result = Query(State);
	TestTrue(TEXT("Query succeeds"), Result.bQuerySucceeded);
	TestFalse(TEXT("No legal slots remain"), Result.bCanDeployToAnySlot);
	TestEqual(TEXT("All candidates are returned"),
		Result.SlotResults.Num(), State.DeploymentSlotCatalog.Slots.Num());
	for (const FMatchPlayOrdinaryDeploymentSlotAvailability& SlotResult :
		Result.SlotResults)
	{
		TestEqual(TEXT("Every Slot reports occupancy"),
			SlotResult.LegalityResult.ErrorCode,
			EMatchPlayOrdinaryDeploymentErrorCode::SlotAlreadyOccupied);
	}
	return true;
}

MATCH_PLAY_ORDINARY_DEPLOYMENT_AVAILABILITY_TEST(
	FMatchPlayOrdinaryDeploymentAvailabilityAllPositionInvalidTest,
	"AllSlotsPositionIneligible")

bool FMatchPlayOrdinaryDeploymentAvailabilityAllPositionInvalidTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayOrdinaryDeploymentAvailability;
	FMatchPlayState State = MakeState();
	State.DeploymentSlotCatalog.Slots.RemoveAt(1);
	const FMatchPlayOrdinaryDeploymentAvailabilityResult Result = Query(State);
	TestTrue(TEXT("Query succeeds"), Result.bQuerySucceeded);
	TestFalse(TEXT("Defense card has no Forward slot"),
		Result.bCanDeployToAnySlot);
	for (const FMatchPlayOrdinaryDeploymentSlotAvailability& SlotResult :
		Result.SlotResults)
	{
		TestEqual(TEXT("Every Forward slot rejects Defense"),
			SlotResult.LegalityResult.ErrorCode,
			EMatchPlayOrdinaryDeploymentErrorCode
				::PositionNotEligibleForZone);
	}
	return true;
}

MATCH_PLAY_ORDINARY_DEPLOYMENT_AVAILABILITY_TEST(
	FMatchPlayOrdinaryDeploymentAvailabilityInvalidCatalogTest,
	"InvalidCatalogFailsQuery")

bool FMatchPlayOrdinaryDeploymentAvailabilityInvalidCatalogTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayOrdinaryDeploymentAvailability;
	FMatchPlayState State = MakeState();
	State.DeploymentSlotCatalog.Slots.Reset();
	const FMatchPlayOrdinaryDeploymentAvailabilityResult Result = Query(State);
	TestFalse(TEXT("Catalog enumeration fails"), Result.bQuerySucceeded);
	TestFalse(TEXT("No slot is claimed legal"), Result.bCanDeployToAnySlot);
	TestEqual(TEXT("Availability error is exact"), Result.ErrorCode,
		EMatchPlayOrdinaryDeploymentAvailabilityErrorCode
			::CatalogEnumerationFailed);
	TestEqual(TEXT("Underlying catalog error is exact"),
		Result.UnderlyingCatalogValidationErrorCode,
		EMatchPlayDeploymentSlotCatalogValidationErrorCode::EmptyCatalog);
	TestEqual(TEXT("No candidates are fabricated"), Result.SlotResults.Num(), 0);
	return true;
}

MATCH_PLAY_ORDINARY_DEPLOYMENT_AVAILABILITY_TEST(
	FMatchPlayOrdinaryDeploymentAvailabilityNoMutationTest,
	"QueryNeverMutatesState")

bool FMatchPlayOrdinaryDeploymentAvailabilityNoMutationTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayOrdinaryDeploymentAvailability;
	const FMatchPlayState State = MakeState();
	const FMatchPlayState OriginalState = State;
	const FMatchPlayOrdinaryDeploymentAvailabilityResult First = Query(State);
	const FMatchPlayOrdinaryDeploymentAvailabilityResult Second = Query(State);
	TestTrue(TEXT("Input State remains identical"),
		AreStatesEqual(State, OriginalState));
	TestEqual(TEXT("Repeated query has same success"),
		First.bQuerySucceeded, Second.bQuerySucceeded);
	TestEqual(TEXT("Repeated query has same legal count"),
		First.LegalSlotIds.Num(), Second.LegalSlotIds.Num());
	TestEqual(TEXT("Repeated query has same candidate count"),
		First.SlotResults.Num(), Second.SlotResults.Num());
	return true;
}

#undef MATCH_PLAY_ORDINARY_DEPLOYMENT_AVAILABILITY_TEST

#endif
