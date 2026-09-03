#include "MatchPlayMarkerNoSelectionGoal.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "MatchPlayMarkerNoSelectionGoalTestFixtures.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMatchPlayGoalHistoryAwardTest,
	"FMCodex.CoreRules.MatchPlayGoalHistory.SystemAwardAtomicity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMatchPlayGoalHistoryAwardTest::RunTest(const FString&)
{
	using namespace FMCodex::Tests::MatchPlayMarkerNoSelectionGoal;
	for (const auto Side : {EInitialTurnOrderPlayer::PlayerA, EInitialTurnOrderPlayer::PlayerB})
	{
		const auto Before = MakeState(Side);
		const auto Request = MakeDeclineRequest(GetDefender(Side));
		const auto Goal = FMatchPlayMarkerDecline::Decline(Before, Request);
		if (!TestTrue(TEXT("Real rule-awarded goal accepted"), Goal.bSuccess)) return false;
		if (!TestEqual(TEXT("Exactly one persistent event"), Goal.AfterState.GoalHistory.Num(), 1)) return false;
		const auto& Fact = Goal.AfterState.GoalHistory[0];
		TestTrue(TEXT("Actual sequence/side; no invented individual for marker decline"),
			Fact.AttackSequence == Before.CurrentAttack.AttackSequence && Fact.ScoringSide == Side
				&& Fact.bSystemAward && Fact.ScorerCardId.IsNone() && !Goal.AfterState.bHasCurrentAttack);
		const auto Duplicate = FMatchPlayMarkerDecline::Decline(Goal.AfterState, Request);
		TestTrue(TEXT("Duplicate cannot record or score again"), !Duplicate.bSuccess && AreStatesEqual(Duplicate.AfterState, Goal.AfterState));
		auto Invalid = Before;
		Invalid.RuntimeState.PlayerAState.Score = -1;
		const auto Failure = FMatchPlayMarkerDecline::Decline(Invalid, Request);
		TestTrue(TEXT("Failed completion cannot publish history"), !Failure.bSuccess && AreStatesEqual(Invalid, Failure.AfterState)
			&& Failure.AfterState.GoalHistory.IsEmpty());
	}
	return true;
}
#endif
