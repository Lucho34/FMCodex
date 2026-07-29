#include "MatchPlayCurrentAttackHelperSelectionAvailability.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "MatchPlayCurrentAttackHelperSelectionTestFixtures.h"
#include "Misc/AutomationTest.h"

namespace HelperFixtures =
	FMCodex::Tests::MatchPlayCurrentAttackHelperSelection;

#define HELPER_QUERY_TEST(TestClass, TestName) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
		TestClass, \
		"FMCodex.CoreRules.MatchPlayCurrentAttackHelperSelection.Query." TestName, \
		EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

HELPER_QUERY_TEST(FHelperLegalityCommonRulesTest, "CommonRules")

bool FHelperLegalityCommonRulesTest::RunTest(const FString& Parameters)
{
	for (const ESkillRuleType Type : {
		ESkillRuleType::PassControl,
		ESkillRuleType::Cross,
		ESkillRuleType::ThroughBall})
	{
		const FMatchPlayState State = HelperFixtures::MakeState(Type);
		const FMatchPlayState Original = State;
		const auto Legal =
			FMatchPlayCurrentAttackHelperSelectionLegalityEvaluator
				::Evaluate(State, HelperFixtures::MakeRequest());
		TestTrue(TEXT("Ordinary Helper legal"), Legal.bSuccess);
		TestEqual(TEXT("Action retained"),
			Legal.ResolvedActionType, Type);
		TestTrue(TEXT("No Helper position restriction"),
			Legal.ResolvedHelperSnapshot.PositionTypes.Contains(
				EPlayerPositionType::Attack));
		TestTrue(TEXT("Legality read-only"),
			HelperFixtures::AreStatesEqual(State, Original));
	}
	return true;
}

HELPER_QUERY_TEST(FHelperLegalityFailuresTest, "CandidateFailures")

bool FHelperLegalityFailuresTest::RunTest(const FString& Parameters)
{
	using E = EMatchPlayCurrentAttackHelperSelectionErrorCode;
	auto Expect = [this](
		const TCHAR* Label,
		const FName CardId,
		const E Expected)
	{
		const auto Result =
			FMatchPlayCurrentAttackHelperSelectionLegalityEvaluator
				::Evaluate(
					HelperFixtures::MakeState(),
					HelperFixtures::MakeRequest(CardId));
		TestFalse(Label, Result.bSuccess);
		TestEqual(Label, Result.ErrorCode, Expected);
	};
	Expect(TEXT("Empty"), NAME_None, E::InvalidHelperCardId);
	Expect(TEXT("Not deployed"), HelperFixtures::MissingId,
		E::HelperNotDeployed);
	Expect(TEXT("Missing snapshot"), HelperFixtures::MissingSnapshotId,
		E::HelperSnapshotQueryFailed);
	Expect(TEXT("Goalkeeper"), HelperFixtures::GoalkeeperId,
		E::HelperIsGoalkeeper);
	Expect(TEXT("Marker conflict"), HelperFixtures::MarkerId,
		E::HelperMatchesMarker);

	FMatchPlayState CrossSide = HelperFixtures::MakeState();
	CrossSide.CurrentAttack.DeploymentPlacements.Add(
		HelperFixtures::MakePlacement(
			EInitialTurnOrderPlayer::PlayerA,
			HelperFixtures::HelperId,
			TEXT("Slot.CrossSideSameCard")));
	CrossSide.DeploymentSlotCatalog.Slots.Add(
		HelperFixtures::MakeSlot(
			TEXT("Slot.CrossSideSameCard"),
			EMatchPlayNeutralSlotSide::NearPlayerA));
	const auto CrossSideResult =
		FMatchPlayCurrentAttackHelperSelectionLegalityEvaluator
			::Evaluate(CrossSide, HelperFixtures::MakeRequest());
	TestTrue(TEXT("Cross-side same CardId allowed"),
		CrossSideResult.bSuccess);
	return true;
}

HELPER_QUERY_TEST(FHelperAvailabilitySemanticsTest, "OrderAndRetention")

bool FHelperAvailabilitySemanticsTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State = HelperFixtures::MakeState();
	const FMatchPlayState Original = State;
	const auto First =
		FMatchPlayCurrentAttackHelperSelectionAvailability::Query(
			State,
			HelperFixtures::ValidAttackSequence,
			EInitialTurnOrderPlayer::PlayerB);
	const auto Second =
		FMatchPlayCurrentAttackHelperSelectionAvailability::Query(
			State,
			HelperFixtures::ValidAttackSequence,
			EInitialTurnOrderPlayer::PlayerB);
	TestTrue(TEXT("Availability succeeds"), First.bQuerySucceeded);
	TestTrue(TEXT("Has legal Helper"), First.bCanSelectAnyHelper);
	TestEqual(TEXT("All defender placements retained"),
		First.Candidates.Num(), 4);
	if (First.Candidates.Num() == 4)
	{
		TestEqual(TEXT("Marker first"),
			First.Candidates[0].HelperCardId, HelperFixtures::MarkerId);
		TestEqual(TEXT("Legal Helper second"),
			First.Candidates[1].HelperCardId, HelperFixtures::HelperId);
		TestEqual(TEXT("GK third"),
			First.Candidates[2].HelperCardId,
			HelperFixtures::GoalkeeperId);
		TestEqual(TEXT("Missing snapshot fourth"),
			First.Candidates[3].HelperCardId,
			HelperFixtures::MissingSnapshotId);
		TestEqual(TEXT("Marker diagnostic"),
			First.Candidates[0].LegalityResult.ErrorCode,
			EMatchPlayCurrentAttackHelperSelectionErrorCode
				::HelperMatchesMarker);
		TestTrue(TEXT("Legal candidate retained"),
			First.Candidates[1].LegalityResult.bSuccess);
		TestEqual(TEXT("GK diagnostic"),
			First.Candidates[2].LegalityResult.ErrorCode,
			EMatchPlayCurrentAttackHelperSelectionErrorCode
				::HelperIsGoalkeeper);
		TestEqual(TEXT("Missing diagnostic"),
			First.Candidates[3].LegalityResult.ErrorCode,
			EMatchPlayCurrentAttackHelperSelectionErrorCode
				::HelperSnapshotQueryFailed);
	}
	TestEqual(TEXT("Repeated candidate count"),
		First.Candidates.Num(), Second.Candidates.Num());
	TestTrue(TEXT("Complete reflected Availability deterministic"),
		FMatchPlayCurrentAttackHelperSelectionAvailabilityResult
			::StaticStruct()->CompareScriptStruct(
				&First,
				&Second,
				0));
	TestEqual(TEXT("Repeated Global error"),
		First.GlobalContextResult.ErrorCode,
		Second.GlobalContextResult.ErrorCode);
	TestEqual(TEXT("Repeated frozen Carrier"),
		First.GlobalContextResult.FrozenCarrierCardId,
		Second.GlobalContextResult.FrozenCarrierCardId);
	TestEqual(TEXT("Repeated frozen Marker"),
		First.GlobalContextResult.FrozenMarkerCardId,
		Second.GlobalContextResult.FrozenMarkerCardId);
	TestEqual(TEXT("Repeated frozen Runner"),
		First.GlobalContextResult.FrozenRunnerCardId,
		Second.GlobalContextResult.FrozenRunnerCardId);
	for (int32 Index = 0; Index < First.Candidates.Num(); ++Index)
	{
		TestEqual(TEXT("Repeated candidate order"),
			First.Candidates[Index].HelperCardId,
			Second.Candidates[Index].HelperCardId);
		TestEqual(TEXT("Repeated full candidate error"),
			First.Candidates[Index].LegalityResult.ErrorCode,
			Second.Candidates[Index].LegalityResult.ErrorCode);
		TestEqual(TEXT("Repeated placement count"),
			First.Candidates[Index].LegalityResult
				.MatchingHelperPlacementCount,
			Second.Candidates[Index].LegalityResult
				.MatchingHelperPlacementCount);
		TestEqual(TEXT("Repeated messages"),
			First.Candidates[Index].LegalityResult.ErrorMessage,
			Second.Candidates[Index].LegalityResult.ErrorMessage);
		TestEqual(TEXT("Repeated Snapshot query error"),
			First.Candidates[Index].LegalityResult
				.HelperSnapshotQueryResult.ErrorCode,
			Second.Candidates[Index].LegalityResult
				.HelperSnapshotQueryResult.ErrorCode);
		TestEqual(TEXT("Repeated Snapshot query CardId"),
			First.Candidates[Index].LegalityResult
				.HelperSnapshotQueryResult.CardId,
			Second.Candidates[Index].LegalityResult
				.HelperSnapshotQueryResult.CardId);
	}
	TestTrue(TEXT("Availability read-only"),
		HelperFixtures::AreStatesEqual(State, Original));
	return true;
}

HELPER_QUERY_TEST(FHelperAvailabilityZeroAndGlobalFailureTest,
	"ZeroAndGlobalFailure")

bool FHelperAvailabilityZeroAndGlobalFailureTest::RunTest(
	const FString& Parameters)
{
	const auto Zero =
		FMatchPlayCurrentAttackHelperSelectionAvailability::Query(
			HelperFixtures::MakeZeroLegalState(),
			HelperFixtures::ValidAttackSequence,
			EInitialTurnOrderPlayer::PlayerB);
	TestTrue(TEXT("Zero legal query succeeds"), Zero.bQuerySucceeded);
	TestFalse(TEXT("Zero legal flag"), Zero.bCanSelectAnyHelper);
	TestEqual(TEXT("Illegal candidates retained"),
		Zero.Candidates.Num(), 3);

	FMatchPlayState Corrupt = HelperFixtures::MakeState();
	Corrupt.CurrentAttack.DeploymentPlacements[0].SlotId = NAME_None;
	const auto Failure =
		FMatchPlayCurrentAttackHelperSelectionAvailability::Query(
			Corrupt,
			HelperFixtures::ValidAttackSequence,
			EInitialTurnOrderPlayer::PlayerB);
	TestFalse(TEXT("Global failure"), Failure.bQuerySucceeded);
	TestEqual(TEXT("No candidates on global failure"),
		Failure.Candidates.Num(), 0);
	return true;
}

#undef HELPER_QUERY_TEST

#endif
