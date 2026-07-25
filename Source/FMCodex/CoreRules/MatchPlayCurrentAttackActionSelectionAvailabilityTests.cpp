#include "MatchPlayCurrentAttackActionSelectionAvailability.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

#include <type_traits>

namespace FMCodex::Tests::CurrentAttackActionSelection
{
	FSkillRuleSnapshotSet MakeSkillRules();
	FMatchPlayState MakeState();
	FMatchPlayDeploymentPlacement MakePlacement(
		EInitialTurnOrderPlayer PlayerSide,
		FName CardId,
		const TCHAR* SlotId);
	bool AreStatesEqual(
		const FMatchPlayState& Left,
		const FMatchPlayState& Right);
}

namespace FMCodex::Tests::CurrentAttackActionSelectionAvailability
{
	constexpr int64 ValidAttackSequence = 11;
	const FName CarrierOneId(TEXT("PlayerA.CarrierOne"));
	const FName CarrierTwoId(TEXT("PlayerA.CarrierTwo"));
	const FName GoalkeeperId(TEXT("PlayerA.Goalkeeper"));
	const FName LongShotSkillId(TEXT("Skill.LongShot"));
	const FName CutInsideSkillId(TEXT("Skill.CutInside"));
	const FName PassControlSkillId(TEXT("Skill.PassControl"));
	const FName CrossSkillId(TEXT("Skill.Cross"));
	const FName ThroughBallSkillId(TEXT("Skill.ThroughBall"));

	FSkillRuleSnapshotSet MakeSkillRules()
	{
		return FMCodex::Tests::CurrentAttackActionSelection
			::MakeSkillRules();
	}

	FMatchPlayState MakeState()
	{
		return FMCodex::Tests::CurrentAttackActionSelection::MakeState();
	}

	FMatchPlayDeploymentPlacement MakePlacement(
		const EInitialTurnOrderPlayer PlayerSide,
		const FName CardId,
		const TCHAR* SlotId)
	{
		return FMCodex::Tests::CurrentAttackActionSelection::MakePlacement(
			PlayerSide,
			CardId,
			SlotId);
	}

	bool AreStatesEqual(
		const FMatchPlayState& Left,
		const FMatchPlayState& Right)
	{
		return FMCodex::Tests::CurrentAttackActionSelection::AreStatesEqual(
			Left,
			Right);
	}

	FMatchPlayCurrentAttackActionSelectionAvailabilityResult Query(
		const FMatchPlayState& State,
		const FSkillRuleSnapshotSet& Rules = MakeSkillRules(),
		const int64 AttackSequence = ValidAttackSequence,
		const EInitialTurnOrderPlayer RequestingSide =
			EInitialTurnOrderPlayer::PlayerA)
	{
		return FMatchPlayCurrentAttackActionSelectionAvailability::Query(
			State,
			AttackSequence,
			RequestingSide,
			Rules);
	}
}

#define ACTION_SELECTION_AVAILABILITY_TEST(TestClass, TestName) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
		TestClass, \
		"FMCodex.CoreRules.MatchPlayCurrentAttackActionSelection.Availability." TestName, \
		EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

ACTION_SELECTION_AVAILABILITY_TEST(
	FActionSelectionAvailabilityContractTest,
	"DefaultsReflectionAndSignature")

bool FActionSelectionAvailabilityContractTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayCurrentAttackActionSelectionAvailabilityResult Result;
	TestFalse(TEXT("Default query failed"), Result.bQuerySucceeded);
	TestFalse(TEXT("Default has no action"), Result.bCanSelectAnyAction);
	TestTrue(TEXT("Default candidates empty"), Result.Candidates.IsEmpty());
	TestNotNull(TEXT("Result reflected"),
		FMatchPlayCurrentAttackActionSelectionAvailabilityResult
			::StaticStruct());
	using FQuerySignature =
		FMatchPlayCurrentAttackActionSelectionAvailabilityResult (*)(
			const FMatchPlayState&,
			int64,
			EInitialTurnOrderPlayer,
			const FSkillRuleSnapshotSet&);
	TestTrue(TEXT("Query signature frozen"),
		(std::is_same_v<
			decltype(
				&FMatchPlayCurrentAttackActionSelectionAvailability
					::Query),
			FQuerySignature>));
	return true;
}

ACTION_SELECTION_AVAILABILITY_TEST(
	FActionSelectionAvailabilityOrderTest,
	"PreservesPlacementAndSkillOrder")

bool FActionSelectionAvailabilityOrderTest::RunTest(
	const FString& Parameters)
{
	using namespace
		FMCodex::Tests::CurrentAttackActionSelectionAvailability;
	const FMatchPlayState State = MakeState();
	const FMatchPlayState OriginalState = State;
	const FMatchPlayCurrentAttackActionSelectionAvailabilityResult Result =
		Query(State);
	TestTrue(TEXT("Query succeeds"), Result.bQuerySucceeded);
	TestTrue(TEXT("At least one action is legal"),
		Result.bCanSelectAnyAction);
	TestEqual(TEXT("All attacker ordinary skills are candidates"),
		Result.Candidates.Num(), 5);
	const TArray<FName> ExpectedCards = {
		CarrierOneId,
		CarrierOneId,
		CarrierTwoId,
		CarrierTwoId,
		CarrierTwoId
	};
	const TArray<FName> ExpectedSkills = {
		LongShotSkillId,
		CrossSkillId,
		CutInsideSkillId,
		ThroughBallSkillId,
		PassControlSkillId
	};
	for (int32 Index = 0; Index < Result.Candidates.Num(); ++Index)
	{
		TestEqual(TEXT("Carrier order is stable"),
			Result.Candidates[Index].CarrierCardId,
			ExpectedCards[Index]);
		TestEqual(TEXT("Skill order is stable"),
			Result.Candidates[Index].SkillId,
			ExpectedSkills[Index]);
		TestEqual(TEXT("Candidate request carrier matches"),
			Result.Candidates[Index].LegalityResult.Request.CarrierCardId,
			ExpectedCards[Index]);
		TestEqual(TEXT("Candidate request skill matches"),
			Result.Candidates[Index].LegalityResult.Request.SkillId,
			ExpectedSkills[Index]);
	}
	TestTrue(TEXT("Query is read-only"), AreStatesEqual(State, OriginalState));
	return true;
}

ACTION_SELECTION_AVAILABILITY_TEST(
	FActionSelectionAvailabilityFilteringTest,
	"FiltersDefenderAndGoalkeeperPlacements")

bool FActionSelectionAvailabilityFilteringTest::RunTest(
	const FString& Parameters)
{
	using namespace
		FMCodex::Tests::CurrentAttackActionSelectionAvailability;
	FMatchPlayState State = MakeState();
	State.CurrentAttack.DeploymentPlacements.Add(
		MakePlacement(
			EInitialTurnOrderPlayer::PlayerA,
			GoalkeeperId,
			TEXT("Slot.GK")));
	const FMatchPlayCurrentAttackActionSelectionAvailabilityResult Result =
		Query(State);
	TestTrue(TEXT("Query succeeds"), Result.bQuerySucceeded);
	for (const auto& Candidate : Result.Candidates)
	{
		TestTrue(TEXT("No goalkeeper candidate"),
			Candidate.CarrierCardId != GoalkeeperId);
		TestTrue(TEXT("No defender placement candidate"),
			Candidate.LegalityResult.Request.RequestingSide
				== EInitialTurnOrderPlayer::PlayerA);
	}
	return true;
}

ACTION_SELECTION_AVAILABILITY_TEST(
	FActionSelectionAvailabilityZeroNoPlacementTest,
	"NoAttackerPlacementIsSuccessfulEmptyResult")

bool FActionSelectionAvailabilityZeroNoPlacementTest::RunTest(
	const FString& Parameters)
{
	using namespace
		FMCodex::Tests::CurrentAttackActionSelectionAvailability;
	FMatchPlayState State = MakeState();
	State.CurrentAttack.DeploymentPlacements.RemoveAll(
		[](const FMatchPlayDeploymentPlacement& Placement)
		{
			return Placement.PlayerSide
				== EInitialTurnOrderPlayer::PlayerA;
		});
	const auto Result = Query(State);
	TestTrue(TEXT("Query succeeds"), Result.bQuerySucceeded);
	TestFalse(TEXT("No action is legal"), Result.bCanSelectAnyAction);
	TestTrue(TEXT("No candidate fabricated"), Result.Candidates.IsEmpty());
	return true;
}

ACTION_SELECTION_AVAILABILITY_TEST(
	FActionSelectionAvailabilityZeroGoalkeeperOnlyTest,
	"GoalkeeperOnlyIsSuccessfulEmptyResult")

bool FActionSelectionAvailabilityZeroGoalkeeperOnlyTest::RunTest(
	const FString& Parameters)
{
	using namespace
		FMCodex::Tests::CurrentAttackActionSelectionAvailability;
	FMatchPlayState State = MakeState();
	State.CurrentAttack.DeploymentPlacements.RemoveAll(
		[](const FMatchPlayDeploymentPlacement& Placement)
		{
			return Placement.PlayerSide
				== EInitialTurnOrderPlayer::PlayerA;
		});
	State.CurrentAttack.DeploymentPlacements.Add(
		MakePlacement(
			EInitialTurnOrderPlayer::PlayerA,
			GoalkeeperId,
			TEXT("Slot.GK")));
	const auto Result = Query(State);
	TestTrue(TEXT("Query succeeds"), Result.bQuerySucceeded);
	TestFalse(TEXT("GK cannot produce legal action"),
		Result.bCanSelectAnyAction);
	TestTrue(TEXT("GK skills are not candidates"),
		Result.Candidates.IsEmpty());
	return true;
}

ACTION_SELECTION_AVAILABILITY_TEST(
	FActionSelectionAvailabilityAllAPFilteredTest,
	"AllActionPointMismatchesAreSuccessfulZeroResult")

bool FActionSelectionAvailabilityAllAPFilteredTest::RunTest(
	const FString& Parameters)
{
	using namespace
		FMCodex::Tests::CurrentAttackActionSelectionAvailability;
	FMatchPlayState State = MakeState();
	State.CurrentAttack.DeploymentPlacements = {
		MakePlacement(
			EInitialTurnOrderPlayer::PlayerA,
			CarrierOneId,
			TEXT("Slot.A1"))
	};
	State.CardSnapshotAuthority.PlayerACardSnapshots.Cards[0].SkillIds = {
		CrossSkillId
	};
	State.CurrentAttack.ActionPoint = 4;
	const auto Result = Query(State);
	TestTrue(TEXT("Query succeeds"), Result.bQuerySucceeded);
	TestFalse(TEXT("No action passes AP"), Result.bCanSelectAnyAction);
	TestEqual(TEXT("Real candidate retained"), Result.Candidates.Num(), 1);
	TestEqual(TEXT("Exact candidate error"),
		Result.Candidates[0].LegalityResult.ErrorCode,
		EMatchPlayCurrentAttackActionSelectionErrorCode
			::ActionPointOutsideSkillRange);
	return true;
}

ACTION_SELECTION_AVAILABILITY_TEST(
	FActionSelectionAvailabilityGlobalBlockerTest,
	"StaleSequenceIsExplicitGlobalBlocker")

bool FActionSelectionAvailabilityGlobalBlockerTest::RunTest(
	const FString& Parameters)
{
	using namespace
		FMCodex::Tests::CurrentAttackActionSelectionAvailability;
	const auto Result = Query(
		MakeState(),
		MakeSkillRules(),
		ValidAttackSequence + 1);
	TestTrue(TEXT("Global blocker query completes"),
		Result.bQuerySucceeded);
	TestFalse(TEXT("No action claimed"), Result.bCanSelectAnyAction);
	TestTrue(TEXT("Blocker exposed"),
		Result.bHasFirstBlockingLegalityResult);
	TestEqual(TEXT("Exact blocker"),
		Result.FirstBlockingLegalityResult.ErrorCode,
		EMatchPlayCurrentAttackActionSelectionErrorCode
			::AttackSequenceMismatch);
	TestTrue(TEXT("No candidates enumerated"),
		Result.Candidates.IsEmpty());
	return true;
}

ACTION_SELECTION_AVAILABILITY_TEST(
	FActionSelectionAvailabilitySelectedBlockerTest,
	"AlreadySelectedIsExplicitGlobalBlocker")

bool FActionSelectionAvailabilitySelectedBlockerTest::RunTest(
	const FString& Parameters)
{
	using namespace
		FMCodex::Tests::CurrentAttackActionSelectionAvailability;
	FMatchPlayState State = MakeState();
	State.CurrentAttack.bHasSelectedAction = true;
	State.CurrentAttack.SelectedAction.CarrierCardId = CarrierOneId;
	State.CurrentAttack.SelectedAction.SkillId = LongShotSkillId;
	State.CurrentAttack.SelectedAction.ActionType =
		ESkillRuleType::LongShot;
	const auto Result = Query(State);
	TestTrue(TEXT("Global blocker query completes"),
		Result.bQuerySucceeded);
	TestEqual(TEXT("Exact selected blocker"),
		Result.FirstBlockingLegalityResult.ErrorCode,
		EMatchPlayCurrentAttackActionSelectionErrorCode
			::ActionAlreadySelected);
	return true;
}

ACTION_SELECTION_AVAILABILITY_TEST(
	FActionSelectionAvailabilityInvalidRulesTest,
	"InvalidSkillRuleSetFailsEnumeration")

bool FActionSelectionAvailabilityInvalidRulesTest::RunTest(
	const FString& Parameters)
{
	using namespace
		FMCodex::Tests::CurrentAttackActionSelectionAvailability;
	FSkillRuleSnapshotSet Rules = MakeSkillRules();
	const FSkillRuleSnapshot DuplicateRule = Rules.SkillRules[0];
	Rules.SkillRules.Add(DuplicateRule);
	const auto Result = Query(MakeState(), Rules);
	TestFalse(TEXT("Query fails"), Result.bQuerySucceeded);
	TestEqual(TEXT("Top availability error"),
		Result.ErrorCode,
		EMatchPlayCurrentAttackActionSelectionAvailabilityErrorCode
			::SkillRuleSetValidationFailed);
	TestEqual(TEXT("Underlying validation error"),
		Result.UnderlyingSkillRuleSetValidationErrorCode,
		ESkillRuleSnapshotValidationErrorCode::DuplicateSkillId);
	TestTrue(TEXT("No candidates enumerated"),
		Result.Candidates.IsEmpty());
	return true;
}

ACTION_SELECTION_AVAILABILITY_TEST(
	FActionSelectionAvailabilitySnapshotFailureTest,
	"InvalidSnapshotAuthorityFailsEnumeration")

bool FActionSelectionAvailabilitySnapshotFailureTest::RunTest(
	const FString& Parameters)
{
	using namespace
		FMCodex::Tests::CurrentAttackActionSelectionAvailability;
	FMatchPlayState State = MakeState();
	const FPlayerCardRuleSnapshot DuplicateSnapshot =
		State.CardSnapshotAuthority.PlayerACardSnapshots.Cards[0];
	State.CardSnapshotAuthority.PlayerACardSnapshots.Cards.Add(
		DuplicateSnapshot);
	const auto Result = Query(State);
	TestFalse(TEXT("Query fails"), Result.bQuerySucceeded);
	TestEqual(TEXT("Top availability error"),
		Result.ErrorCode,
		EMatchPlayCurrentAttackActionSelectionAvailabilityErrorCode
			::CarrierSnapshotEnumerationFailed);
	TestEqual(TEXT("Underlying snapshot error"),
		Result.UnderlyingSnapshotAuthorityQueryErrorCode,
		EMatchPlayCardSnapshotAuthorityQueryErrorCode
			::SnapshotValidationFailed);
	return true;
}

ACTION_SELECTION_AVAILABILITY_TEST(
	FActionSelectionAvailabilityDuplicatePlacementTest,
	"DuplicatePlacementIsNotSilentlyDeduplicated")

bool FActionSelectionAvailabilityDuplicatePlacementTest::RunTest(
	const FString& Parameters)
{
	using namespace
		FMCodex::Tests::CurrentAttackActionSelectionAvailability;
	FMatchPlayState State = MakeState();
	State.CurrentAttack.DeploymentPlacements.Add(
		MakePlacement(
			EInitialTurnOrderPlayer::PlayerA,
			CarrierOneId,
			TEXT("Slot.A3")));
	const auto Result = Query(State);
	TestTrue(TEXT("Query completes"), Result.bQuerySucceeded);
	TestFalse(TEXT("Ambiguous carrier is not legal"),
		Result.Candidates[0].LegalityResult.bIsLegal);
	TestEqual(TEXT("Corruption is exposed"),
		Result.Candidates[0].LegalityResult.ErrorCode,
		EMatchPlayCurrentAttackActionSelectionErrorCode
			::CarrierDeploymentAmbiguous);
	TestEqual(TEXT("Duplicate placement remains observable"),
		Result.Candidates.Num(), 7);
	return true;
}

ACTION_SELECTION_AVAILABILITY_TEST(
	FActionSelectionAvailabilityDeterminismTest,
	"RepeatedQueryIsDeterministicAndReadOnly")

bool FActionSelectionAvailabilityDeterminismTest::RunTest(
	const FString& Parameters)
{
	using namespace
		FMCodex::Tests::CurrentAttackActionSelectionAvailability;
	const FMatchPlayState State = MakeState();
	const FMatchPlayState OriginalState = State;
	const auto First = Query(State);
	const auto Second = Query(State);
	TestEqual(TEXT("Success is deterministic"),
		First.bQuerySucceeded, Second.bQuerySucceeded);
	TestEqual(TEXT("Availability is deterministic"),
		First.bCanSelectAnyAction, Second.bCanSelectAnyAction);
	TestEqual(TEXT("Candidate count deterministic"),
		First.Candidates.Num(), Second.Candidates.Num());
	for (int32 Index = 0; Index < First.Candidates.Num(); ++Index)
	{
		TestEqual(TEXT("Carrier deterministic"),
			First.Candidates[Index].CarrierCardId,
			Second.Candidates[Index].CarrierCardId);
		TestEqual(TEXT("Skill deterministic"),
			First.Candidates[Index].SkillId,
			Second.Candidates[Index].SkillId);
		TestEqual(TEXT("Legality deterministic"),
			First.Candidates[Index].LegalityResult.ErrorCode,
			Second.Candidates[Index].LegalityResult.ErrorCode);
	}
	TestTrue(TEXT("State unchanged"), AreStatesEqual(State, OriginalState));
	return true;
}

#undef ACTION_SELECTION_AVAILABILITY_TEST

#endif
