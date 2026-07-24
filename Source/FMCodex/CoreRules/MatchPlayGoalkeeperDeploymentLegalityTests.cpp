#include "MatchPlayGoalkeeperDeploymentLegality.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

#include <type_traits>

namespace FMCodex::Tests::MatchPlayGoalkeeperDeploymentLegality
{
	constexpr int64 ValidAttackSequence = 11;
	const FName PlayerAGoalkeeperCardId(TEXT("PlayerA.GK"));
	const FName PlayerAOutfieldCardId(TEXT("PlayerA.Outfield"));
	const FName PlayerBGoalkeeperCardId(TEXT("PlayerB.GK"));
	const FName PlayerBOutfieldCardId(TEXT("PlayerB.Outfield"));
	const FName NearPlayerASlotId(TEXT("Slot.NearPlayerA"));
	const FName NearPlayerBSlotId(TEXT("Slot.NearPlayerB"));
	const FName OtherNearPlayerBSlotId(TEXT("Slot.NearPlayerB.Other"));

	FPlayerCardRuleSnapshot MakeGoalkeeperDeploymentSnapshot(
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

	FMatchPlayDeploymentSlotDefinition MakeGoalkeeperDeploymentSlot(
		const FName SlotId,
		const EMatchPlayNeutralSlotSide NeutralSide)
	{
		FMatchPlayDeploymentSlotDefinition Slot;
		Slot.SlotId = SlotId;
		Slot.NeutralSide = NeutralSide;
		return Slot;
	}

	FMatchPlayState MakeGoalkeeperDeploymentState(
		const EInitialTurnOrderPlayer CurrentAttacker =
			EInitialTurnOrderPlayer::PlayerA)
	{
		FMatchPlayState State;
		State.RuntimeState.bIsInitialized = true;
		State.RuntimeState.CurrentAttackingPlayer = CurrentAttacker;
		State.RuntimeState.PlayerAState.TotalAttackCount = 4;
		State.RuntimeState.PlayerBState.TotalAttackCount = 4;
		State.CardUsageState.PlayerACardUsageState.AvailableCardIds = {
			PlayerAGoalkeeperCardId,
			PlayerAOutfieldCardId
		};
		State.CardUsageState.PlayerBCardUsageState.AvailableCardIds = {
			PlayerBGoalkeeperCardId,
			PlayerBOutfieldCardId
		};
		State.CardSnapshotAuthority.PlayerACardSnapshots.Cards = {
			MakeGoalkeeperDeploymentSnapshot(
				PlayerAGoalkeeperCardId,
				true),
			MakeGoalkeeperDeploymentSnapshot(
				PlayerAOutfieldCardId,
				false)
		};
		State.CardSnapshotAuthority.PlayerBCardSnapshots.Cards = {
			MakeGoalkeeperDeploymentSnapshot(
				PlayerBGoalkeeperCardId,
				true),
			MakeGoalkeeperDeploymentSnapshot(
				PlayerBOutfieldCardId,
				false)
		};
		State.DeploymentSlotCatalog.Slots = {
			MakeGoalkeeperDeploymentSlot(
				NearPlayerASlotId,
				EMatchPlayNeutralSlotSide::NearPlayerA),
			MakeGoalkeeperDeploymentSlot(
				NearPlayerBSlotId,
				EMatchPlayNeutralSlotSide::NearPlayerB),
			MakeGoalkeeperDeploymentSlot(
				OtherNearPlayerBSlotId,
				EMatchPlayNeutralSlotSide::NearPlayerB)
		};
		State.bHasCurrentAttack = true;
		State.CurrentAttack.Phase =
			EMatchPlayCurrentAttackPhase::Deployment;
		State.CurrentAttack.AttackSequence = ValidAttackSequence;
		State.CurrentAttack.CurrentLegalDeploymentSide =
			CurrentAttacker == EInitialTurnOrderPlayer::PlayerA
				? EInitialTurnOrderPlayer::PlayerB
				: EInitialTurnOrderPlayer::PlayerA;
		return State;
	}

	FMatchPlayGoalkeeperDeploymentRequest MakeGoalkeeperDeploymentRequest(
		const EInitialTurnOrderPlayer CurrentAttacker =
			EInitialTurnOrderPlayer::PlayerA)
	{
		FMatchPlayGoalkeeperDeploymentRequest Request;
		Request.AttackSequence = ValidAttackSequence;
		Request.RequestingSide =
			CurrentAttacker == EInitialTurnOrderPlayer::PlayerA
				? EInitialTurnOrderPlayer::PlayerB
				: EInitialTurnOrderPlayer::PlayerA;
		Request.CardId =
			Request.RequestingSide == EInitialTurnOrderPlayer::PlayerA
				? PlayerAGoalkeeperCardId
				: PlayerBGoalkeeperCardId;
		Request.SlotId =
			Request.RequestingSide == EInitialTurnOrderPlayer::PlayerA
				? NearPlayerASlotId
				: NearPlayerBSlotId;
		return Request;
	}

	FMatchPlayDeploymentPlacement MakeGoalkeeperDeploymentPlacement(
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

	bool AreGoalkeeperDeploymentStatesEqual(
		const FMatchPlayState& Left,
		const FMatchPlayState& Right)
	{
		return FMatchPlayState::StaticStruct()->CompareScriptStruct(
			&Left,
			&Right,
			0);
	}

	bool ExpectGoalkeeperDeploymentFailure(
		FAutomationTestBase& Test,
		const TCHAR* Context,
		const FMatchPlayState& State,
		const FMatchPlayGoalkeeperDeploymentRequest& Request,
		const EMatchPlayGoalkeeperDeploymentErrorCode ExpectedError)
	{
		const FMatchPlayState OriginalState = State;
		const FMatchPlayGoalkeeperDeploymentLegalityResult Result =
			FMatchPlayGoalkeeperDeploymentLegalityEvaluator::Evaluate(
				State,
				Request);
		Test.TestFalse(
			*FString::Printf(TEXT("%s is illegal"), Context),
			Result.bIsLegal);
		Test.TestEqual(
			*FString::Printf(TEXT("%s returns the exact error"), Context),
			Result.ErrorCode,
			ExpectedError);
		Test.TestTrue(
			*FString::Printf(TEXT("%s has diagnostics"), Context),
			!Result.ErrorMessage.IsEmpty());
		Test.TestTrue(
			*FString::Printf(TEXT("%s does not mutate State"), Context),
			AreGoalkeeperDeploymentStatesEqual(State, OriginalState));
		return true;
	}
}

#define MATCH_PLAY_GOALKEEPER_DEPLOYMENT_LEGALITY_TEST(TestClass, TestName) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
		TestClass, \
		"FMCodex.CoreRules.MatchPlayGoalkeeperDeployment.Legality." TestName, \
		EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

MATCH_PLAY_GOALKEEPER_DEPLOYMENT_LEGALITY_TEST(
	FMatchPlayGoalkeeperDeploymentDefaultsTest,
	"DefaultsAndPublicContract")

bool FMatchPlayGoalkeeperDeploymentDefaultsTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayGoalkeeperDeploymentRequest Request;
	const FMatchPlayGoalkeeperDeploymentLegalityResult Result;
	TestEqual(TEXT("Default sequence is zero"), Request.AttackSequence, int64{0});
	TestEqual(TEXT("Default side is None"), Request.RequestingSide,
		EInitialTurnOrderPlayer::None);
	TestTrue(TEXT("Default CardId is empty"), Request.CardId.IsNone());
	TestTrue(TEXT("Default SlotId is empty"), Request.SlotId.IsNone());
	TestFalse(TEXT("Default result is illegal"), Result.bIsLegal);
	TestNotNull(TEXT("Request is reflected"),
		FMatchPlayGoalkeeperDeploymentRequest::StaticStruct());
	TestNotNull(TEXT("Result is reflected"),
		FMatchPlayGoalkeeperDeploymentLegalityResult::StaticStruct());
	using FEvaluateSignature =
		FMatchPlayGoalkeeperDeploymentLegalityResult (*)(
			const FMatchPlayState&,
			const FMatchPlayGoalkeeperDeploymentRequest&);
	TestTrue(TEXT("Evaluate signature is frozen"),
		(std::is_same_v<
			decltype(
				&FMatchPlayGoalkeeperDeploymentLegalityEvaluator::Evaluate),
			FEvaluateSignature>));
	return true;
}

MATCH_PLAY_GOALKEEPER_DEPLOYMENT_LEGALITY_TEST(
	FMatchPlayGoalkeeperDeploymentSuccessTest,
	"PlayerADefendsInBackfieldWithCleanDiagnostics")

bool FMatchPlayGoalkeeperDeploymentSuccessTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayGoalkeeperDeploymentLegality;
	const FMatchPlayState State = MakeGoalkeeperDeploymentState(
		EInitialTurnOrderPlayer::PlayerB);
	const FMatchPlayState OriginalState = State;
	const FMatchPlayGoalkeeperDeploymentLegalityResult Result =
		FMatchPlayGoalkeeperDeploymentLegalityEvaluator::Evaluate(
			State,
			MakeGoalkeeperDeploymentRequest(
				EInitialTurnOrderPlayer::PlayerB));
	TestTrue(TEXT("Defender goalkeeper deployment is legal"), Result.bIsLegal);
	TestEqual(TEXT("Top error is clean"), Result.ErrorCode,
		EMatchPlayGoalkeeperDeploymentErrorCode::None);
	TestEqual(TEXT("Snapshot error is clean"),
		Result.UnderlyingSnapshotAuthorityQueryErrorCode,
		EMatchPlayCardSnapshotAuthorityQueryErrorCode::None);
	TestEqual(TEXT("PlayCard error is clean"),
		Result.UnderlyingPlayCardErrorCode,
		EPlayCardResolveErrorCode::None);
	TestEqual(TEXT("CardUsage error is clean"),
		Result.UnderlyingCardUsageErrorCode,
		ECardUsageResolveErrorCode::None);
	TestEqual(TEXT("Usage error is clean"),
		Result.UnderlyingGoalkeeperUsageErrorCode,
		EMatchPlayGoalkeeperUsageErrorCode::None);
	TestEqual(TEXT("Slot error is clean"),
		Result.UnderlyingSlotCatalogQueryErrorCode,
		EMatchPlayDeploymentSlotCatalogQueryErrorCode::None);
	TestEqual(TEXT("Zone error is clean"),
		Result.UnderlyingRelativeZoneResolutionErrorCode,
		EMatchPlayRelativeDeploymentZoneResolveErrorCode::None);
	TestEqual(TEXT("Zone is Backfield"), Result.ResolvedRelativeZone,
		EMatchPlayRelativeDeploymentZone::Backfield);
	TestTrue(TEXT("Success does not mutate State"),
		AreGoalkeeperDeploymentStatesEqual(State, OriginalState));
	return true;
}

MATCH_PLAY_GOALKEEPER_DEPLOYMENT_LEGALITY_TEST(
	FMatchPlayGoalkeeperDeploymentMirrorSuccessTest,
	"PlayerBDefendsInMirroredBackfield")

bool FMatchPlayGoalkeeperDeploymentMirrorSuccessTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayGoalkeeperDeploymentLegality;
	const FMatchPlayGoalkeeperDeploymentLegalityResult Result =
		FMatchPlayGoalkeeperDeploymentLegalityEvaluator::Evaluate(
			MakeGoalkeeperDeploymentState(
				EInitialTurnOrderPlayer::PlayerA),
			MakeGoalkeeperDeploymentRequest(
				EInitialTurnOrderPlayer::PlayerA));
	TestTrue(TEXT("Mirrored defender deployment is legal"), Result.bIsLegal);
	TestEqual(TEXT("Mirrored zone is Backfield"),
		Result.ResolvedRelativeZone,
		EMatchPlayRelativeDeploymentZone::Backfield);
	return true;
}

#define DEFINE_GOALKEEPER_SIMPLE_FAILURE_TEST( \
	TestClass, TestName, StateMutation, RequestMutation, ExpectedError) \
	MATCH_PLAY_GOALKEEPER_DEPLOYMENT_LEGALITY_TEST(TestClass, TestName) \
	bool TestClass::RunTest(const FString& Parameters) \
	{ \
		using namespace FMCodex::Tests::MatchPlayGoalkeeperDeploymentLegality; \
		FMatchPlayState State = MakeGoalkeeperDeploymentState(); \
		FMatchPlayGoalkeeperDeploymentRequest Request = \
			MakeGoalkeeperDeploymentRequest(); \
		StateMutation; \
		RequestMutation; \
		return ExpectGoalkeeperDeploymentFailure( \
			*this, TEXT(TestName), State, Request, ExpectedError); \
	}

DEFINE_GOALKEEPER_SIMPLE_FAILURE_TEST(
	FMatchPlayGoalkeeperDeploymentUninitializedTest,
	"RejectsUninitializedState",
	State.RuntimeState.bIsInitialized = false,
	Request.CardId = NAME_None,
	EMatchPlayGoalkeeperDeploymentErrorCode::MatchPlayStateNotInitialized)

DEFINE_GOALKEEPER_SIMPLE_FAILURE_TEST(
	FMatchPlayGoalkeeperDeploymentNoAttackTest,
	"RejectsMissingCurrentAttack",
	State.bHasCurrentAttack = false,
	Request.CardId = NAME_None,
	EMatchPlayGoalkeeperDeploymentErrorCode::NoCurrentAttack)

DEFINE_GOALKEEPER_SIMPLE_FAILURE_TEST(
	FMatchPlayGoalkeeperDeploymentInvalidSequenceTest,
	"RejectsInvalidAuthoritativeSequence",
	State.CurrentAttack.AttackSequence = 0,
	Request.AttackSequence = 0,
	EMatchPlayGoalkeeperDeploymentErrorCode::InvalidCurrentAttackSequence)

DEFINE_GOALKEEPER_SIMPLE_FAILURE_TEST(
	FMatchPlayGoalkeeperDeploymentStaleSequenceTest,
	"RejectsStaleSequence",
	State.RuntimeState.bIsInitialized = true,
	Request.AttackSequence = ValidAttackSequence + 1,
	EMatchPlayGoalkeeperDeploymentErrorCode::AttackSequenceMismatch)

DEFINE_GOALKEEPER_SIMPLE_FAILURE_TEST(
	FMatchPlayGoalkeeperDeploymentWrongPhaseTest,
	"RejectsResolutionPhase",
	State.CurrentAttack.Phase = EMatchPlayCurrentAttackPhase::Resolution,
	Request.CardId = NAME_None,
	EMatchPlayGoalkeeperDeploymentErrorCode::CurrentAttackNotInDeployment)

DEFINE_GOALKEEPER_SIMPLE_FAILURE_TEST(
	FMatchPlayGoalkeeperDeploymentInvalidAttackerTest,
	"RejectsInvalidCurrentAttacker",
	State.RuntimeState.CurrentAttackingPlayer =
		EInitialTurnOrderPlayer::None,
	Request.CardId = NAME_None,
	EMatchPlayGoalkeeperDeploymentErrorCode::InvalidCurrentAttackingPlayer)

DEFINE_GOALKEEPER_SIMPLE_FAILURE_TEST(
	FMatchPlayGoalkeeperDeploymentInvalidRequesterTest,
	"RejectsInvalidRequestingSide",
	State.CurrentAttack.CurrentLegalDeploymentSide =
		EInitialTurnOrderPlayer::None,
	Request.RequestingSide = EInitialTurnOrderPlayer::None,
	EMatchPlayGoalkeeperDeploymentErrorCode::InvalidRequestingSide)

DEFINE_GOALKEEPER_SIMPLE_FAILURE_TEST(
	FMatchPlayGoalkeeperDeploymentInvalidLegalSideTest,
	"RejectsInvalidLegalSide",
	State.CurrentAttack.CurrentLegalDeploymentSide =
		EInitialTurnOrderPlayer::None,
	Request.CardId = NAME_None,
	EMatchPlayGoalkeeperDeploymentErrorCode
		::InvalidCurrentLegalDeploymentSide)

DEFINE_GOALKEEPER_SIMPLE_FAILURE_TEST(
	FMatchPlayGoalkeeperDeploymentWrongLegalSideTest,
	"RejectsWrongLegalSide",
	State.CurrentAttack.CurrentLegalDeploymentSide =
		EInitialTurnOrderPlayer::PlayerA,
	Request.CardId = NAME_None,
	EMatchPlayGoalkeeperDeploymentErrorCode
		::RequestingSideNotCurrentLegalDeploymentSide)

DEFINE_GOALKEEPER_SIMPLE_FAILURE_TEST(
	FMatchPlayGoalkeeperDeploymentBothFinishedTest,
	"RejectsBothFinishedCorruption",
	State.CurrentAttack.bAttackerDeploymentFinished = true;
	State.CurrentAttack.bDefenderDeploymentFinished = true,
	Request.CardId = NAME_None,
	EMatchPlayGoalkeeperDeploymentErrorCode::InvalidDeploymentFinishedState)

DEFINE_GOALKEEPER_SIMPLE_FAILURE_TEST(
	FMatchPlayGoalkeeperDeploymentRequesterFinishedTest,
	"RejectsFinishedDefender",
	State.CurrentAttack.bDefenderDeploymentFinished = true,
	Request.CardId = NAME_None,
	EMatchPlayGoalkeeperDeploymentErrorCode::RequestingSideAlreadyFinished)

DEFINE_GOALKEEPER_SIMPLE_FAILURE_TEST(
	FMatchPlayGoalkeeperDeploymentAttackerRequestTest,
	"RejectsCurrentAttacker",
	State.CurrentAttack.CurrentLegalDeploymentSide =
		EInitialTurnOrderPlayer::PlayerA,
	Request.RequestingSide = EInitialTurnOrderPlayer::PlayerA;
	Request.CardId = PlayerAGoalkeeperCardId;
	Request.SlotId = NearPlayerASlotId,
	EMatchPlayGoalkeeperDeploymentErrorCode::RequestingSideIsNotDefender)

DEFINE_GOALKEEPER_SIMPLE_FAILURE_TEST(
	FMatchPlayGoalkeeperDeploymentEmptyCardTest,
	"RejectsEmptyCardId",
	State.RuntimeState.bIsInitialized = true,
	Request.CardId = NAME_None,
	EMatchPlayGoalkeeperDeploymentErrorCode::InvalidCardId)

DEFINE_GOALKEEPER_SIMPLE_FAILURE_TEST(
	FMatchPlayGoalkeeperDeploymentEmptySlotTest,
	"RejectsEmptySlotId",
	State.RuntimeState.bIsInitialized = true,
	Request.SlotId = NAME_None,
	EMatchPlayGoalkeeperDeploymentErrorCode::InvalidSlotId)

DEFINE_GOALKEEPER_SIMPLE_FAILURE_TEST(
	FMatchPlayGoalkeeperDeploymentMissingSnapshotTest,
	"RejectsMissingSnapshot",
	State.RuntimeState.bIsInitialized = true,
	Request.CardId = TEXT("Missing.GK"),
	EMatchPlayGoalkeeperDeploymentErrorCode::CardSnapshotLookupFailed)

DEFINE_GOALKEEPER_SIMPLE_FAILURE_TEST(
	FMatchPlayGoalkeeperDeploymentInvalidSnapshotAuthorityTest,
	"RejectsInvalidSnapshotAuthority",
	const FPlayerCardRuleSnapshot DuplicateSnapshot =
		State.CardSnapshotAuthority.PlayerBCardSnapshots.Cards[0];
	State.CardSnapshotAuthority.PlayerBCardSnapshots.Cards.Add(
		DuplicateSnapshot),
	Request.SlotId = NearPlayerBSlotId,
	EMatchPlayGoalkeeperDeploymentErrorCode::CardSnapshotLookupFailed)

DEFINE_GOALKEEPER_SIMPLE_FAILURE_TEST(
	FMatchPlayGoalkeeperDeploymentNonGoalkeeperTest,
	"RejectsNonGoalkeeper",
	State.RuntimeState.bIsInitialized = true,
	Request.CardId = PlayerBOutfieldCardId,
	EMatchPlayGoalkeeperDeploymentErrorCode::CardIsNotGoalkeeper)

DEFINE_GOALKEEPER_SIMPLE_FAILURE_TEST(
	FMatchPlayGoalkeeperDeploymentCorruptCardUsageTest,
	"RejectsCorruptCardUsage",
	State.CardUsageState.PlayerBCardUsageState.UsedCardIds.Add(
		PlayerBGoalkeeperCardId),
	Request.SlotId = NearPlayerBSlotId,
	EMatchPlayGoalkeeperDeploymentErrorCode::CardUsageValidationFailed)

DEFINE_GOALKEEPER_SIMPLE_FAILURE_TEST(
	FMatchPlayGoalkeeperDeploymentUsedCardTest,
	"RejectsUsedCard",
	State.CardUsageState.PlayerBCardUsageState.AvailableCardIds.Remove(
		PlayerBGoalkeeperCardId);
	State.CardUsageState.PlayerBCardUsageState.UsedCardIds.Add(
		PlayerBGoalkeeperCardId),
	Request.SlotId = NearPlayerBSlotId,
	EMatchPlayGoalkeeperDeploymentErrorCode::CardAlreadyUsed)

DEFINE_GOALKEEPER_SIMPLE_FAILURE_TEST(
	FMatchPlayGoalkeeperDeploymentUnavailableCardTest,
	"RejectsUnavailableCard",
	State.CardUsageState.PlayerBCardUsageState.AvailableCardIds.Remove(
		PlayerBGoalkeeperCardId),
	Request.SlotId = NearPlayerBSlotId,
	EMatchPlayGoalkeeperDeploymentErrorCode::CardNotAvailable)

DEFINE_GOALKEEPER_SIMPLE_FAILURE_TEST(
	FMatchPlayGoalkeeperDeploymentPersistentUsedTest,
	"RejectsPreviouslyUsedGoalkeeper",
	State.GoalkeeperUsageState.bPlayerBGoalkeeperCardUsed = true,
	Request.SlotId = NearPlayerBSlotId,
	EMatchPlayGoalkeeperDeploymentErrorCode::GoalkeeperAlreadyUsedThisMatch)

DEFINE_GOALKEEPER_SIMPLE_FAILURE_TEST(
	FMatchPlayGoalkeeperDeploymentActivatedTest,
	"PrioritizesAlreadyActivatedForConsistentCurrentUse",
	State.GoalkeeperUsageState.bPlayerBGoalkeeperCardUsed = true;
	State.CurrentAttack.bCurrentDefenseGoalkeeperActivated = true;
	State.CurrentAttack.DeploymentPlacements.Add(
		MakeGoalkeeperDeploymentPlacement(
			EInitialTurnOrderPlayer::PlayerB,
			PlayerBGoalkeeperCardId,
			NearPlayerBSlotId)),
	Request.SlotId = OtherNearPlayerBSlotId,
	EMatchPlayGoalkeeperDeploymentErrorCode
		::GoalkeeperAlreadyActivatedThisAttack)

DEFINE_GOALKEEPER_SIMPLE_FAILURE_TEST(
	FMatchPlayGoalkeeperDeploymentFalseTrueUsageTest,
	"RejectsActivationWithoutPersistentUse",
	State.CurrentAttack.bCurrentDefenseGoalkeeperActivated = true,
	Request.SlotId = NearPlayerBSlotId,
	EMatchPlayGoalkeeperDeploymentErrorCode::InvalidGoalkeeperUsageState)

DEFINE_GOALKEEPER_SIMPLE_FAILURE_TEST(
	FMatchPlayGoalkeeperDeploymentActivatedWithoutPlacementTest,
	"RejectsActivationWithoutPlacement",
	State.GoalkeeperUsageState.bPlayerBGoalkeeperCardUsed = true;
	State.CurrentAttack.bCurrentDefenseGoalkeeperActivated = true,
	Request.SlotId = NearPlayerBSlotId,
	EMatchPlayGoalkeeperDeploymentErrorCode::InvalidGoalkeeperUsageState)

DEFINE_GOALKEEPER_SIMPLE_FAILURE_TEST(
	FMatchPlayGoalkeeperDeploymentMultiplePlacementsTest,
	"RejectsMultipleMatchingGoalkeeperPlacements",
	State.GoalkeeperUsageState.bPlayerBGoalkeeperCardUsed = true;
	State.CurrentAttack.bCurrentDefenseGoalkeeperActivated = true;
	State.CurrentAttack.DeploymentPlacements.Add(
		MakeGoalkeeperDeploymentPlacement(
			EInitialTurnOrderPlayer::PlayerB,
			PlayerBGoalkeeperCardId,
			NearPlayerBSlotId));
	State.CurrentAttack.DeploymentPlacements.Add(
		MakeGoalkeeperDeploymentPlacement(
			EInitialTurnOrderPlayer::PlayerB,
			PlayerBGoalkeeperCardId,
			OtherNearPlayerBSlotId)),
	Request.SlotId = NearPlayerBSlotId,
	EMatchPlayGoalkeeperDeploymentErrorCode::InvalidGoalkeeperUsageState)

DEFINE_GOALKEEPER_SIMPLE_FAILURE_TEST(
	FMatchPlayGoalkeeperDeploymentPlacementWithoutActivationTest,
	"RejectsPlacementWithoutActivation",
	State.CurrentAttack.DeploymentPlacements.Add(
		MakeGoalkeeperDeploymentPlacement(
			EInitialTurnOrderPlayer::PlayerB,
			PlayerBGoalkeeperCardId,
			NearPlayerBSlotId)),
	Request.SlotId = OtherNearPlayerBSlotId,
	EMatchPlayGoalkeeperDeploymentErrorCode::InvalidGoalkeeperUsageState)

DEFINE_GOALKEEPER_SIMPLE_FAILURE_TEST(
	FMatchPlayGoalkeeperDeploymentInvalidCatalogTest,
	"RejectsInvalidCatalog",
	State.DeploymentSlotCatalog.Slots[1].SlotId =
		State.DeploymentSlotCatalog.Slots[0].SlotId,
	Request.SlotId = NearPlayerBSlotId,
	EMatchPlayGoalkeeperDeploymentErrorCode::SlotCatalogLookupFailed)

DEFINE_GOALKEEPER_SIMPLE_FAILURE_TEST(
	FMatchPlayGoalkeeperDeploymentMissingSlotTest,
	"RejectsMissingSlot",
	State.RuntimeState.bIsInitialized = true,
	Request.SlotId = TEXT("Missing.Slot"),
	EMatchPlayGoalkeeperDeploymentErrorCode::SlotNotFound)

DEFINE_GOALKEEPER_SIMPLE_FAILURE_TEST(
	FMatchPlayGoalkeeperDeploymentOccupiedSlotTest,
	"RejectsCrossSideGlobalOccupancy",
	State.CurrentAttack.DeploymentPlacements.Add(
		MakeGoalkeeperDeploymentPlacement(
			EInitialTurnOrderPlayer::PlayerA,
			PlayerAOutfieldCardId,
			NearPlayerBSlotId)),
	Request.SlotId = NearPlayerBSlotId,
	EMatchPlayGoalkeeperDeploymentErrorCode::SlotAlreadyOccupied)

DEFINE_GOALKEEPER_SIMPLE_FAILURE_TEST(
	FMatchPlayGoalkeeperDeploymentGoalkeeperOccupiedSlotTest,
	"RejectsGoalkeeperPlacementGlobalOccupancy",
	State.CurrentAttack.DeploymentPlacements.Add(
		MakeGoalkeeperDeploymentPlacement(
			EInitialTurnOrderPlayer::PlayerA,
			PlayerAGoalkeeperCardId,
			NearPlayerBSlotId)),
	Request.SlotId = NearPlayerBSlotId,
	EMatchPlayGoalkeeperDeploymentErrorCode::SlotAlreadyOccupied)

DEFINE_GOALKEEPER_SIMPLE_FAILURE_TEST(
	FMatchPlayGoalkeeperDeploymentMidfieldTest,
	"RejectsDefenderMidfield",
	State.RuntimeState.bIsInitialized = true,
	Request.SlotId = NearPlayerASlotId,
	EMatchPlayGoalkeeperDeploymentErrorCode
		::GoalkeeperSlotNotInDefenderBackfield)

MATCH_PLAY_GOALKEEPER_DEPLOYMENT_LEGALITY_TEST(
	FMatchPlayGoalkeeperDeploymentSideAwareSnapshotTest,
	"SameCardIdUsesRequestingSideSnapshot")

bool FMatchPlayGoalkeeperDeploymentSideAwareSnapshotTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayGoalkeeperDeploymentLegality;
	const FName SharedCardId(TEXT("Shared.GK"));
	FMatchPlayState State = MakeGoalkeeperDeploymentState();
	State.CardSnapshotAuthority.PlayerACardSnapshots.Cards[0] =
		MakeGoalkeeperDeploymentSnapshot(SharedCardId, false);
	State.CardSnapshotAuthority.PlayerBCardSnapshots.Cards[0] =
		MakeGoalkeeperDeploymentSnapshot(SharedCardId, true);
	State.CardUsageState.PlayerACardUsageState.AvailableCardIds[0] =
		SharedCardId;
	State.CardUsageState.PlayerBCardUsageState.AvailableCardIds[0] =
		SharedCardId;
	FMatchPlayGoalkeeperDeploymentRequest Request =
		MakeGoalkeeperDeploymentRequest();
	Request.CardId = SharedCardId;

	const FMatchPlayGoalkeeperDeploymentLegalityResult Result =
		FMatchPlayGoalkeeperDeploymentLegalityEvaluator::Evaluate(
			State,
			Request);
	TestTrue(TEXT("Defender side snapshot is selected"), Result.bIsLegal);
	return true;
}

MATCH_PLAY_GOALKEEPER_DEPLOYMENT_LEGALITY_TEST(
	FMatchPlayGoalkeeperDeploymentUsageSideIsolationTest,
	"PersistentUsageIsSideIsolated")

bool FMatchPlayGoalkeeperDeploymentUsageSideIsolationTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayGoalkeeperDeploymentLegality;
	FMatchPlayState State = MakeGoalkeeperDeploymentState();
	State.GoalkeeperUsageState.bPlayerAGoalkeeperCardUsed = true;
	const FMatchPlayState OriginalState = State;

	const FMatchPlayGoalkeeperDeploymentLegalityResult Result =
		FMatchPlayGoalkeeperDeploymentLegalityEvaluator::Evaluate(
			State,
			MakeGoalkeeperDeploymentRequest());
	TestTrue(TEXT("Other-side usage does not block the defender"),
		Result.bIsLegal);
	TestTrue(TEXT("Side-isolation success remains read-only"),
		AreGoalkeeperDeploymentStatesEqual(State, OriginalState));
	return true;
}

MATCH_PLAY_GOALKEEPER_DEPLOYMENT_LEGALITY_TEST(
	FMatchPlayGoalkeeperDeploymentMixedPriorityTest,
	"FirstErrorOrderPrecedesRequestAndAuthorityErrors")

bool FMatchPlayGoalkeeperDeploymentMixedPriorityTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayGoalkeeperDeploymentLegality;
	FMatchPlayState State = MakeGoalkeeperDeploymentState();
	State.RuntimeState.bIsInitialized = false;
	State.bHasCurrentAttack = false;
	State.DeploymentSlotCatalog.Slots.Reset();
	FMatchPlayGoalkeeperDeploymentRequest Request;
	return ExpectGoalkeeperDeploymentFailure(
		*this,
		TEXT("Mixed invalid state"),
		State,
		Request,
		EMatchPlayGoalkeeperDeploymentErrorCode
			::MatchPlayStateNotInitialized);
}

#undef DEFINE_GOALKEEPER_SIMPLE_FAILURE_TEST
#undef MATCH_PLAY_GOALKEEPER_DEPLOYMENT_LEGALITY_TEST

#endif
