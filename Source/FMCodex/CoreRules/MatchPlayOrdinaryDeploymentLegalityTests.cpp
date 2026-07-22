#include "MatchPlayOrdinaryDeploymentLegality.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

#include <type_traits>

namespace FMCodex::Tests::MatchPlayOrdinaryDeploymentLegality
{
	constexpr int64 ValidAttackSequence = 7;
	const FName SharedCardId(TEXT("Shared01"));
	const FName PlayerAOtherCardId(TEXT("PlayerA.Other"));
	const FName PlayerAGoalkeeperCardId(TEXT("PlayerA.GK"));
	const FName PlayerBOtherCardId(TEXT("PlayerB.Other"));
	const FName PlayerBGoalkeeperCardId(TEXT("PlayerB.GK"));
	const FName NearPlayerASlotId(TEXT("Slot.NearPlayerA"));
	const FName NearPlayerBSlotId(TEXT("Slot.NearPlayerB"));
	const FName OtherNearPlayerBSlotId(TEXT("Slot.NearPlayerB.Other"));

	FPlayerCardRuleSnapshot MakeSnapshot(
		const FName CardId,
		const TArray<EPlayerPositionType>& PositionTypes,
		const bool bIsGoalkeeper = false)
	{
		FPlayerCardRuleSnapshot Snapshot;
		Snapshot.CardId = CardId;
		Snapshot.PositionTypes = PositionTypes;
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
		State.RuntimeState.PlayerAState.TotalAttackCount = 4;
		State.RuntimeState.PlayerAState.UsedAttackCount = 1;
		State.RuntimeState.PlayerBState.TotalAttackCount = 4;
		State.RuntimeState.PlayerBState.UsedAttackCount = 1;

		State.CardUsageState.PlayerACardUsageState.AvailableCardIds = {
			SharedCardId,
			PlayerAOtherCardId,
			PlayerAGoalkeeperCardId
		};
		State.CardUsageState.PlayerBCardUsageState.AvailableCardIds = {
			SharedCardId,
			PlayerBOtherCardId,
			PlayerBGoalkeeperCardId
		};

		State.CardSnapshotAuthority.PlayerACardSnapshots.Cards = {
			MakeSnapshot(
				SharedCardId,
				{ EPlayerPositionType::Attack }),
			MakeSnapshot(
				PlayerAOtherCardId,
				{ EPlayerPositionType::Defense }),
			MakeSnapshot(
				PlayerAGoalkeeperCardId,
				{ EPlayerPositionType::Goalkeeper },
				true)
		};
		State.CardSnapshotAuthority.PlayerBCardSnapshots.Cards = {
			MakeSnapshot(
				SharedCardId,
				{ EPlayerPositionType::Defense }),
			MakeSnapshot(
				PlayerBOtherCardId,
				{ EPlayerPositionType::Attack }),
			MakeSnapshot(
				PlayerBGoalkeeperCardId,
				{ EPlayerPositionType::Goalkeeper },
				true)
		};

		State.DeploymentSlotCatalog.Slots = {
			MakeSlot(
				NearPlayerASlotId,
				EMatchPlayNeutralSlotSide::NearPlayerA),
			MakeSlot(
				NearPlayerBSlotId,
				EMatchPlayNeutralSlotSide::NearPlayerB),
			MakeSlot(
				OtherNearPlayerBSlotId,
				EMatchPlayNeutralSlotSide::NearPlayerB)
		};

		State.bHasCurrentAttack = true;
		State.CurrentAttack.Phase =
			EMatchPlayCurrentAttackPhase::Deployment;
		State.CurrentAttack.AttackSequence = ValidAttackSequence;
		State.CurrentAttack.ActionPoint = 5;
		State.CurrentAttack.CurrentLegalDeploymentSide =
			EInitialTurnOrderPlayer::PlayerA;
		return State;
	}

	FMatchPlayOrdinaryDeploymentRequest MakeRequest(
		const EInitialTurnOrderPlayer RequestingSide =
			EInitialTurnOrderPlayer::PlayerA,
		const FName CardId = SharedCardId,
		const FName SlotId = NearPlayerASlotId,
		const int64 AttackSequence = ValidAttackSequence)
	{
		FMatchPlayOrdinaryDeploymentRequest Request;
		Request.AttackSequence = AttackSequence;
		Request.RequestingSide = RequestingSide;
		Request.CardId = CardId;
		Request.SlotId = SlotId;
		return Request;
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

	bool TestFailure(
		FAutomationTestBase& Test,
		const TCHAR* Context,
		const FMatchPlayState& State,
		const FMatchPlayOrdinaryDeploymentRequest& Request,
		const EMatchPlayOrdinaryDeploymentErrorCode ExpectedError)
	{
		const FMatchPlayState OriginalState = State;
		const FMatchPlayOrdinaryDeploymentLegalityResult Result =
			FMatchPlayOrdinaryDeploymentLegalityEvaluator::Evaluate(
				State,
				Request);
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
			*FString::Printf(TEXT("%s does not mutate State"), Context),
			AreStatesEqual(State, OriginalState));
		return true;
	}
}

#define MATCH_PLAY_ORDINARY_DEPLOYMENT_LEGALITY_TEST(TestClass, TestName) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
		TestClass, \
		"FMCodex.CoreRules.MatchPlayOrdinaryDeployment.Legality." TestName, \
		EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

MATCH_PLAY_ORDINARY_DEPLOYMENT_LEGALITY_TEST(
	FMatchPlayOrdinaryDeploymentDefaultsTest,
	"DefaultsAndPublicContract")

bool FMatchPlayOrdinaryDeploymentDefaultsTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayOrdinaryDeploymentLegality;
	const FMatchPlayOrdinaryDeploymentRequest Request;
	const FMatchPlayOrdinaryDeploymentLegalityResult Result;
	TestEqual(TEXT("Default sequence is zero"), Request.AttackSequence, int64{0});
	TestEqual(TEXT("Default side is None"), Request.RequestingSide,
		EInitialTurnOrderPlayer::None);
	TestTrue(TEXT("Default CardId is empty"), Request.CardId.IsNone());
	TestTrue(TEXT("Default SlotId is empty"), Request.SlotId.IsNone());
	TestFalse(TEXT("Default result is illegal"), Result.bIsLegal);
	TestEqual(TEXT("Default error is None"), Result.ErrorCode,
		EMatchPlayOrdinaryDeploymentErrorCode::None);
	TestNotNull(TEXT("Request is reflected"),
		FMatchPlayOrdinaryDeploymentRequest::StaticStruct());
	TestNotNull(TEXT("Result is reflected"),
		FMatchPlayOrdinaryDeploymentLegalityResult::StaticStruct());
	using EvaluateSignature = FMatchPlayOrdinaryDeploymentLegalityResult (*)(
		const FMatchPlayState&,
		const FMatchPlayOrdinaryDeploymentRequest&);
	TestTrue(TEXT("Evaluate signature is frozen"),
		(std::is_same_v<
			decltype(&FMatchPlayOrdinaryDeploymentLegalityEvaluator::Evaluate),
			EvaluateSignature>));
	return true;
}

MATCH_PLAY_ORDINARY_DEPLOYMENT_LEGALITY_TEST(
	FMatchPlayOrdinaryDeploymentSuccessTest,
	"SuccessHasCleanDiagnostics")

bool FMatchPlayOrdinaryDeploymentSuccessTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayOrdinaryDeploymentLegality;
	const FMatchPlayState State = MakeState();
	const FMatchPlayState OriginalState = State;
	const FMatchPlayOrdinaryDeploymentLegalityResult Result =
		FMatchPlayOrdinaryDeploymentLegalityEvaluator::Evaluate(
			State,
			MakeRequest());
	TestTrue(TEXT("Request is legal"), Result.bIsLegal);
	TestEqual(TEXT("Top error is clean"), Result.ErrorCode,
		EMatchPlayOrdinaryDeploymentErrorCode::None);
	TestEqual(TEXT("Snapshot error is clean"),
		Result.UnderlyingSnapshotAuthorityQueryErrorCode,
		EMatchPlayCardSnapshotAuthorityQueryErrorCode::None);
	TestEqual(TEXT("PlayCard error is clean"),
		Result.UnderlyingPlayCardErrorCode,
		EPlayCardResolveErrorCode::None);
	TestEqual(TEXT("CardUsage error is clean"),
		Result.UnderlyingCardUsageErrorCode,
		ECardUsageResolveErrorCode::None);
	TestEqual(TEXT("Slot error is clean"),
		Result.UnderlyingSlotCatalogQueryErrorCode,
		EMatchPlayDeploymentSlotCatalogQueryErrorCode::None);
	TestEqual(TEXT("Zone error is clean"),
		Result.UnderlyingRelativeZoneResolutionErrorCode,
		EMatchPlayRelativeDeploymentZoneResolveErrorCode::None);
	TestEqual(TEXT("Near attacker resolves midfield"),
		Result.ResolvedRelativeZone,
		EMatchPlayRelativeDeploymentZone::Midfield);
	TestTrue(TEXT("Success does not mutate State"),
		AreStatesEqual(State, OriginalState));
	return true;
}

#define SIMPLE_FAILURE_TEST(TestClass, TestName, Mutation, ExpectedError) \
	MATCH_PLAY_ORDINARY_DEPLOYMENT_LEGALITY_TEST(TestClass, TestName) \
	bool TestClass::RunTest(const FString& Parameters) \
	{ \
		using namespace FMCodex::Tests::MatchPlayOrdinaryDeploymentLegality; \
		FMatchPlayState State = MakeState(); \
		FMatchPlayOrdinaryDeploymentRequest Request = MakeRequest(); \
		Mutation; \
		return TestFailure(*this, TEXT(TestName), State, Request, ExpectedError); \
	}

SIMPLE_FAILURE_TEST(
	FMatchPlayOrdinaryDeploymentUninitializedTest,
	"UninitializedRejectedFirst",
	State.RuntimeState.bIsInitialized = false,
	EMatchPlayOrdinaryDeploymentErrorCode::MatchPlayStateNotInitialized)

SIMPLE_FAILURE_TEST(
	FMatchPlayOrdinaryDeploymentNoAttackTest,
	"NoCurrentAttackRejected",
	State.bHasCurrentAttack = false,
	EMatchPlayOrdinaryDeploymentErrorCode::NoCurrentAttack)

SIMPLE_FAILURE_TEST(
	FMatchPlayOrdinaryDeploymentInvalidSequenceTest,
	"InvalidCurrentSequenceRejected",
	State.CurrentAttack.AttackSequence = 0,
	EMatchPlayOrdinaryDeploymentErrorCode::InvalidCurrentAttackSequence)

SIMPLE_FAILURE_TEST(
	FMatchPlayOrdinaryDeploymentSequenceMismatchTest,
	"StaleSequenceRejected",
	Request.AttackSequence = ValidAttackSequence + 1,
	EMatchPlayOrdinaryDeploymentErrorCode::AttackSequenceMismatch)

SIMPLE_FAILURE_TEST(
	FMatchPlayOrdinaryDeploymentWrongPhaseTest,
	"ResolutionPhaseRejected",
	State.CurrentAttack.Phase = EMatchPlayCurrentAttackPhase::Resolution,
	EMatchPlayOrdinaryDeploymentErrorCode::CurrentAttackNotInDeployment)

SIMPLE_FAILURE_TEST(
	FMatchPlayOrdinaryDeploymentInvalidAttackerTest,
	"InvalidCurrentAttackerRejected",
	State.RuntimeState.CurrentAttackingPlayer = EInitialTurnOrderPlayer::None,
	EMatchPlayOrdinaryDeploymentErrorCode::InvalidCurrentAttackingPlayer)

SIMPLE_FAILURE_TEST(
	FMatchPlayOrdinaryDeploymentInvalidRequesterTest,
	"InvalidRequesterRejected",
	Request.RequestingSide = EInitialTurnOrderPlayer::None,
	EMatchPlayOrdinaryDeploymentErrorCode::InvalidRequestingSide)

SIMPLE_FAILURE_TEST(
	FMatchPlayOrdinaryDeploymentInvalidLegalSideTest,
	"InvalidLegalSideRejected",
	State.CurrentAttack.CurrentLegalDeploymentSide =
		EInitialTurnOrderPlayer::None,
	EMatchPlayOrdinaryDeploymentErrorCode::InvalidCurrentLegalDeploymentSide)

SIMPLE_FAILURE_TEST(
	FMatchPlayOrdinaryDeploymentWrongLegalSideTest,
	"WrongLegalSideRejected",
	Request.RequestingSide = EInitialTurnOrderPlayer::PlayerB,
	EMatchPlayOrdinaryDeploymentErrorCode
		::RequestingSideNotCurrentLegalDeploymentSide)

SIMPLE_FAILURE_TEST(
	FMatchPlayOrdinaryDeploymentBothFinishedTest,
	"BothFinishedDeploymentCorruptionRejected",
	State.CurrentAttack.bAttackerDeploymentFinished = true;
	State.CurrentAttack.bDefenderDeploymentFinished = true,
	EMatchPlayOrdinaryDeploymentErrorCode::InvalidDeploymentFinishedState)

SIMPLE_FAILURE_TEST(
	FMatchPlayOrdinaryDeploymentRequesterFinishedTest,
	"RequesterFinishedRejected",
	State.CurrentAttack.bAttackerDeploymentFinished = true,
	EMatchPlayOrdinaryDeploymentErrorCode::RequestingSideAlreadyFinished)

SIMPLE_FAILURE_TEST(
	FMatchPlayOrdinaryDeploymentEmptyCardTest,
	"EmptyCardRejectedBeforeSlot",
	Request.CardId = NAME_None;
	Request.SlotId = NAME_None,
	EMatchPlayOrdinaryDeploymentErrorCode::InvalidCardId)

SIMPLE_FAILURE_TEST(
	FMatchPlayOrdinaryDeploymentEmptySlotTest,
	"EmptySlotRejectedBeforeAuthorityLookup",
	Request.SlotId = NAME_None,
	EMatchPlayOrdinaryDeploymentErrorCode::InvalidSlotId)

SIMPLE_FAILURE_TEST(
	FMatchPlayOrdinaryDeploymentSnapshotNotFoundTest,
	"SnapshotNotFound",
	Request.CardId = TEXT("Missing.Card"),
	EMatchPlayOrdinaryDeploymentErrorCode::CardSnapshotLookupFailed)

SIMPLE_FAILURE_TEST(
	FMatchPlayOrdinaryDeploymentSnapshotInvalidTest,
	"InvalidSnapshotAuthority",
	State.CardSnapshotAuthority.PlayerACardSnapshots.Cards[0]
		.PositionTypes.Reset(),
	EMatchPlayOrdinaryDeploymentErrorCode::CardSnapshotLookupFailed)

SIMPLE_FAILURE_TEST(
	FMatchPlayOrdinaryDeploymentCardUsageCorruptTest,
	"CorruptCardUsage",
	State.CardUsageState.PlayerACardUsageState.UsedCardIds.Add(
		SharedCardId),
	EMatchPlayOrdinaryDeploymentErrorCode::CardUsageValidationFailed)

SIMPLE_FAILURE_TEST(
	FMatchPlayOrdinaryDeploymentUnavailableTest,
	"UnavailableCard",
	State.CardUsageState.PlayerACardUsageState.AvailableCardIds.Remove(
		SharedCardId),
	EMatchPlayOrdinaryDeploymentErrorCode::CardNotAvailable)

SIMPLE_FAILURE_TEST(
	FMatchPlayOrdinaryDeploymentUsedTest,
	"UsedCard",
	State.CardUsageState.PlayerACardUsageState.AvailableCardIds.Remove(
		SharedCardId);
	State.CardUsageState.PlayerACardUsageState.UsedCardIds.Add(
		SharedCardId),
	EMatchPlayOrdinaryDeploymentErrorCode::CardAlreadyUsed)

SIMPLE_FAILURE_TEST(
	FMatchPlayOrdinaryDeploymentSameSideDuplicateTest,
	"SameSideCardDuplicate",
	FMatchPlayDeploymentPlacement Placement;
	Placement.PlayerSide = EInitialTurnOrderPlayer::PlayerA;
	Placement.CardId = SharedCardId;
	Placement.SlotId = NearPlayerBSlotId;
	State.CurrentAttack.DeploymentPlacements.Add(Placement),
	EMatchPlayOrdinaryDeploymentErrorCode::CardAlreadyPlacedBySide)

MATCH_PLAY_ORDINARY_DEPLOYMENT_LEGALITY_TEST(
	FMatchPlayOrdinaryDeploymentCrossSideSameCardTest,
	"CrossSideSameCardDoesNotConflict")

bool FMatchPlayOrdinaryDeploymentCrossSideSameCardTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayOrdinaryDeploymentLegality;
	FMatchPlayState State = MakeState();
	FMatchPlayDeploymentPlacement Placement;
	Placement.PlayerSide = EInitialTurnOrderPlayer::PlayerB;
	Placement.CardId = SharedCardId;
	Placement.SlotId = NearPlayerBSlotId;
	State.CurrentAttack.DeploymentPlacements.Add(Placement);
	const FMatchPlayOrdinaryDeploymentLegalityResult Result =
		FMatchPlayOrdinaryDeploymentLegalityEvaluator::Evaluate(
			State,
			MakeRequest());
	TestTrue(TEXT("Other side same CardId remains legal"), Result.bIsLegal);
	return true;
}

SIMPLE_FAILURE_TEST(
	FMatchPlayOrdinaryDeploymentGoalkeeperTest,
	"GoalkeeperRejected",
	Request.CardId = PlayerAGoalkeeperCardId,
	EMatchPlayOrdinaryDeploymentErrorCode::GoalkeeperNotAllowed)

SIMPLE_FAILURE_TEST(
	FMatchPlayOrdinaryDeploymentInvalidCatalogTest,
	"InvalidCatalog",
	State.DeploymentSlotCatalog.Slots.Reset(),
	EMatchPlayOrdinaryDeploymentErrorCode::SlotCatalogLookupFailed)

SIMPLE_FAILURE_TEST(
	FMatchPlayOrdinaryDeploymentMissingSlotTest,
	"MissingSlot",
	Request.SlotId = TEXT("Slot.Missing"),
	EMatchPlayOrdinaryDeploymentErrorCode::SlotNotFound)

SIMPLE_FAILURE_TEST(
	FMatchPlayOrdinaryDeploymentGlobalOccupancyTest,
	"GlobalOccupancyAcrossSides",
	FMatchPlayDeploymentPlacement Placement;
	Placement.PlayerSide = EInitialTurnOrderPlayer::PlayerB;
	Placement.CardId = PlayerBOtherCardId;
	Placement.SlotId = NearPlayerASlotId;
	State.CurrentAttack.DeploymentPlacements.Add(Placement),
	EMatchPlayOrdinaryDeploymentErrorCode::SlotAlreadyOccupied)

MATCH_PLAY_ORDINARY_DEPLOYMENT_LEGALITY_TEST(
	FMatchPlayOrdinaryDeploymentZoneMappingTest,
	"RelativeZoneUsesAttackerAndRequestingSide")

bool FMatchPlayOrdinaryDeploymentZoneMappingTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayOrdinaryDeploymentLegality;
	struct FCase
	{
		EInitialTurnOrderPlayer RequestingSide;
		FName CardId;
		FName SlotId;
		EMatchPlayRelativeDeploymentZone ExpectedZone;
	};
	const TArray<FCase> Cases = {
		{ EInitialTurnOrderPlayer::PlayerA, SharedCardId,
			NearPlayerASlotId, EMatchPlayRelativeDeploymentZone::Midfield },
		{ EInitialTurnOrderPlayer::PlayerB, SharedCardId,
			NearPlayerASlotId, EMatchPlayRelativeDeploymentZone::Midfield },
		{ EInitialTurnOrderPlayer::PlayerA, SharedCardId,
			NearPlayerBSlotId, EMatchPlayRelativeDeploymentZone::Forward },
		{ EInitialTurnOrderPlayer::PlayerB, SharedCardId,
			NearPlayerBSlotId, EMatchPlayRelativeDeploymentZone::Backfield }
	};

	for (const FCase& Case : Cases)
	{
		FMatchPlayState State = MakeState();
		State.CurrentAttack.CurrentLegalDeploymentSide = Case.RequestingSide;
		const FMatchPlayOrdinaryDeploymentLegalityResult Result =
			FMatchPlayOrdinaryDeploymentLegalityEvaluator::Evaluate(
				State,
				MakeRequest(
					Case.RequestingSide,
					Case.CardId,
					Case.SlotId));
		TestEqual(TEXT("Resolved zone matches case"),
			Result.ResolvedRelativeZone, Case.ExpectedZone);
	}
	return true;
}

MATCH_PLAY_ORDINARY_DEPLOYMENT_LEGALITY_TEST(
	FMatchPlayOrdinaryDeploymentPositionMatrixTest,
	"PositionEligibilityMatrix")

bool FMatchPlayOrdinaryDeploymentPositionMatrixTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayOrdinaryDeploymentLegality;
	struct FZoneCase
	{
		EInitialTurnOrderPlayer Side;
		FName SlotId;
		EMatchPlayRelativeDeploymentZone Zone;
	};
	const TArray<FZoneCase> ZoneCases = {
		{ EInitialTurnOrderPlayer::PlayerA, NearPlayerASlotId,
			EMatchPlayRelativeDeploymentZone::Midfield },
		{ EInitialTurnOrderPlayer::PlayerA, NearPlayerBSlotId,
			EMatchPlayRelativeDeploymentZone::Forward },
		{ EInitialTurnOrderPlayer::PlayerB, NearPlayerBSlotId,
			EMatchPlayRelativeDeploymentZone::Backfield }
	};
	struct FPositionCase
	{
		EPlayerPositionType Position;
		bool bMidfieldEligible;
		bool bForwardEligible;
		bool bBackfieldEligible;
	};
	const TArray<FPositionCase> PositionCases = {
		{ EPlayerPositionType::Attack, true, true, false },
		{ EPlayerPositionType::Midfield, true, true, true },
		{ EPlayerPositionType::Defense, true, false, true },
		{ EPlayerPositionType::Goalkeeper, false, false, false }
	};

	for (const FZoneCase& ZoneCase : ZoneCases)
	{
		for (const FPositionCase& PositionCase : PositionCases)
		{
			FMatchPlayState State = MakeState();
			State.CurrentAttack.CurrentLegalDeploymentSide = ZoneCase.Side;
			FPlayerCardRuleSnapshot& Snapshot = ZoneCase.Side
				== EInitialTurnOrderPlayer::PlayerA
					? State.CardSnapshotAuthority.PlayerACardSnapshots.Cards[0]
					: State.CardSnapshotAuthority.PlayerBCardSnapshots.Cards[0];
			Snapshot.PositionTypes = { PositionCase.Position };
			Snapshot.bIsGoalkeeper =
				PositionCase.Position == EPlayerPositionType::Goalkeeper;
			Snapshot.bHasGoalkeeperAttributes = Snapshot.bIsGoalkeeper;

			const FMatchPlayOrdinaryDeploymentLegalityResult Result =
				FMatchPlayOrdinaryDeploymentLegalityEvaluator::Evaluate(
					State,
					MakeRequest(
						ZoneCase.Side,
						SharedCardId,
						ZoneCase.SlotId));
			bool bExpectedLegal = false;
			switch (ZoneCase.Zone)
			{
			case EMatchPlayRelativeDeploymentZone::Midfield:
				bExpectedLegal = PositionCase.bMidfieldEligible;
				break;
			case EMatchPlayRelativeDeploymentZone::Forward:
				bExpectedLegal = PositionCase.bForwardEligible;
				break;
			case EMatchPlayRelativeDeploymentZone::Backfield:
				bExpectedLegal = PositionCase.bBackfieldEligible;
				break;
			case EMatchPlayRelativeDeploymentZone::None:
			default:
				break;
			}
			TestEqual(TEXT("Matrix cell legality"),
				Result.bIsLegal, bExpectedLegal);
			if (PositionCase.Position == EPlayerPositionType::Goalkeeper)
			{
				TestEqual(TEXT("GK matrix cell uses explicit guard"),
					Result.ErrorCode,
					EMatchPlayOrdinaryDeploymentErrorCode::GoalkeeperNotAllowed);
			}
			else if (!bExpectedLegal)
			{
				TestEqual(TEXT("Invalid matrix cell uses position error"),
					Result.ErrorCode,
					EMatchPlayOrdinaryDeploymentErrorCode
						::PositionNotEligibleForZone);
			}
		}
	}
	return true;
}

MATCH_PLAY_ORDINARY_DEPLOYMENT_LEGALITY_TEST(
	FMatchPlayOrdinaryDeploymentMultiPositionTest,
	"MultiPositionUsesOrSemantics")

bool FMatchPlayOrdinaryDeploymentMultiPositionTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayOrdinaryDeploymentLegality;
	FMatchPlayState State = MakeState();
	State.CardSnapshotAuthority.PlayerACardSnapshots.Cards[0]
		.PositionTypes = {
			EPlayerPositionType::Defense,
			EPlayerPositionType::Midfield
		};
	const FMatchPlayOrdinaryDeploymentLegalityResult Result =
		FMatchPlayOrdinaryDeploymentLegalityEvaluator::Evaluate(
			State,
			MakeRequest(
				EInitialTurnOrderPlayer::PlayerA,
				SharedCardId,
				NearPlayerBSlotId));
	TestTrue(TEXT("One legal position makes multi-position card legal"),
		Result.bIsLegal);
	return true;
}

MATCH_PLAY_ORDINARY_DEPLOYMENT_LEGALITY_TEST(
	FMatchPlayOrdinaryDeploymentFailureUnderlyingErrorsTest,
	"UnderlyingErrorsRemainScoped")

bool FMatchPlayOrdinaryDeploymentFailureUnderlyingErrorsTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayOrdinaryDeploymentLegality;
	FMatchPlayState State = MakeState();
	const FMatchPlayOrdinaryDeploymentLegalityResult SnapshotFailure =
		FMatchPlayOrdinaryDeploymentLegalityEvaluator::Evaluate(
			State,
			MakeRequest(
				EInitialTurnOrderPlayer::PlayerA,
				TEXT("Missing.Card")));
	TestEqual(TEXT("Snapshot failure preserves concrete query error"),
		SnapshotFailure.UnderlyingSnapshotAuthorityQueryErrorCode,
		EMatchPlayCardSnapshotAuthorityQueryErrorCode::SnapshotNotFound);
	TestEqual(TEXT("Later CardUsage error remains clean"),
		SnapshotFailure.UnderlyingPlayCardErrorCode,
		EPlayCardResolveErrorCode::None);

	State = MakeState();
	State.CardUsageState.PlayerACardUsageState.AvailableCardIds.Remove(
		SharedCardId);
	State.CardUsageState.PlayerACardUsageState.UsedCardIds.Add(SharedCardId);
	const FMatchPlayOrdinaryDeploymentLegalityResult UsedFailure =
		FMatchPlayOrdinaryDeploymentLegalityEvaluator::Evaluate(
			State,
			MakeRequest());
	TestEqual(TEXT("Used failure preserves PlayCard error"),
		UsedFailure.UnderlyingPlayCardErrorCode,
		EPlayCardResolveErrorCode::CardAlreadyUsed);
	TestEqual(TEXT("Used failure preserves CardUsage error"),
		UsedFailure.UnderlyingCardUsageErrorCode,
		ECardUsageResolveErrorCode::CardAlreadyUsed);
	TestEqual(TEXT("Later slot error remains clean"),
		UsedFailure.UnderlyingSlotCatalogQueryErrorCode,
		EMatchPlayDeploymentSlotCatalogQueryErrorCode::None);
	return true;
}

#undef SIMPLE_FAILURE_TEST
#undef MATCH_PLAY_ORDINARY_DEPLOYMENT_LEGALITY_TEST

#endif
