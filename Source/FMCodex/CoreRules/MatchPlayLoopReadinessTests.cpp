#include "MatchPlayLoopReadiness.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace MatchPlayLoopReadinessTests
{
	const FName CardA1(TEXT("CardA1"));
	const FName CardB1(TEXT("CardB1"));

	FMatchPlayState MakeState(
		const EInitialTurnOrderPlayer CurrentPlayer =
			EInitialTurnOrderPlayer::PlayerA,
		const int32 PlayerAScore = 0,
		const int32 PlayerBScore = 0,
		const int32 PlayerATotal = 3,
		const int32 PlayerAUsed = 0,
		const int32 PlayerBTotal = 3,
		const int32 PlayerBUsed = 0,
		const bool bPlayerAHasCards = true,
		const bool bPlayerBHasCards = true)
	{
		FMatchRuntimeState RuntimeState;
		RuntimeState.bIsInitialized = true;
		RuntimeState.PlayerAState.TotalAttackCount = PlayerATotal;
		RuntimeState.PlayerAState.UsedAttackCount = PlayerAUsed;
		RuntimeState.PlayerAState.Score = PlayerAScore;
		RuntimeState.PlayerBState.TotalAttackCount = PlayerBTotal;
		RuntimeState.PlayerBState.UsedAttackCount = PlayerBUsed;
		RuntimeState.PlayerBState.Score = PlayerBScore;
		RuntimeState.FirstPlayer = EInitialTurnOrderPlayer::PlayerA;
		RuntimeState.SecondPlayer = EInitialTurnOrderPlayer::PlayerB;
		RuntimeState.CurrentAttackingPlayer = CurrentPlayer;
		RuntimeState.OpeningResultSnapshot.bSuccess = true;
		RuntimeState.OpeningResultSnapshot.AttackCountResult
			.PlayerAAttackCount = PlayerATotal;
		RuntimeState.OpeningResultSnapshot.AttackCountResult
			.PlayerBAttackCount = PlayerBTotal;

		FMatchCardUsageState CardUsageState;
		if (bPlayerAHasCards)
		{
			CardUsageState.PlayerACardUsageState.AvailableCardIds =
				{ CardA1 };
		}
		if (bPlayerBHasCards)
		{
			CardUsageState.PlayerBCardUsageState.AvailableCardIds =
				{ CardB1 };
		}
		FMatchPlayState State;
		State.RuntimeState = RuntimeState;
		State.CardUsageState = CardUsageState;
		FMatchPlayDeploymentSlotDefinition Slot;
		Slot.SlotId = TEXT("TestDeploymentSlot");
		Slot.NeutralSide = EMatchPlayNeutralSlotSide::NearPlayerA;
		State.DeploymentSlotCatalog.Slots.Add(Slot);
		return State;
	}

	FString LoadProductionSource()
	{
		FString Source;
		FFileHelper::LoadFileToString(
			Source,
			*(FPaths::ProjectDir()
				/ TEXT(
					"Source/FMCodex/CoreRules/MatchPlayLoopReadiness.cpp")));
		return Source;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayLoopReadinessReadyTest,
	"FMCodex.CoreRules.MatchPlayLoopReadiness.ReadyStateAcceptsExternalAttackRequest",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayLoopReadinessReadyTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayLoopReadinessResult Result =
		FMatchPlayLoopReadiness::Evaluate(
			MatchPlayLoopReadinessTests::MakeState());

	TestTrue(TEXT("Can accept external attack request"), Result.bCanAcceptExternalAttackRequest);
	TestTrue(TEXT("Should wait for external attack request"), Result.bShouldWaitForExternalAttackRequest);
	TestFalse(TEXT("Never enters automatic loop"), Result.bShouldEnterAutomaticLoop);
	TestEqual(TEXT("Ready code is returned"), Result.Code, EMatchPlayLoopReadinessCode::ReadyForExternalAttackRequest);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayLoopReadinessEndedTest,
	"FMCodex.CoreRules.MatchPlayLoopReadiness.EndedMatchCannotAcceptExternalAttackRequest",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayLoopReadinessEndedTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayLoopReadinessResult Result =
		FMatchPlayLoopReadiness::Evaluate(
			MatchPlayLoopReadinessTests::MakeState(
				EInitialTurnOrderPlayer::PlayerA,
				1,
				0,
				1,
				1,
				1,
				1));

	TestFalse(TEXT("Cannot accept when all opportunities are consumed"), Result.bCanAcceptExternalAttackRequest);
	TestFalse(TEXT("Should not wait for request"), Result.bShouldWaitForExternalAttackRequest);
	TestFalse(TEXT("Does not enter automatic loop"), Result.bShouldEnterAutomaticLoop);
	TestEqual(TEXT("No remaining opportunities code is returned"), Result.Code, EMatchPlayLoopReadinessCode::NoRemainingAttackOpportunities);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayLoopReadinessNoRemainingTest,
	"FMCodex.CoreRules.MatchPlayLoopReadiness.NoRemainingAttackOpportunitiesCannotAcceptExternalAttackRequest",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayLoopReadinessNoRemainingTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayLoopReadinessResult Result =
		FMatchPlayLoopReadiness::Evaluate(
			MatchPlayLoopReadinessTests::MakeState(
				EInitialTurnOrderPlayer::PlayerB,
				0,
				0,
				2,
				2,
				2,
				2));

	TestFalse(TEXT("Cannot accept when neither player has attacks"), Result.bCanAcceptExternalAttackRequest);
	TestEqual(TEXT("No remaining opportunities code is returned"), Result.Code, EMatchPlayLoopReadinessCode::NoRemainingAttackOpportunities);
	TestTrue(TEXT("Underlying guard marks no remaining opportunities"), Result.TurnGuardResult.bNoRemainingAttackOpportunities);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayLoopReadinessNoAutomaticLoopTest,
	"FMCodex.CoreRules.MatchPlayLoopReadiness.AutomaticLoopFlagAlwaysFalse",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayLoopReadinessNoAutomaticLoopTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayLoopReadinessResult Ready =
		FMatchPlayLoopReadiness::Evaluate(
			MatchPlayLoopReadinessTests::MakeState());
	const FMatchPlayLoopReadinessResult NoCards =
		FMatchPlayLoopReadiness::Evaluate(
			MatchPlayLoopReadinessTests::MakeState(
				EInitialTurnOrderPlayer::PlayerA,
				0,
				0,
				3,
				0,
				3,
				0,
				false,
				true));
	const FMatchPlayLoopReadinessResult NoRemaining =
		FMatchPlayLoopReadiness::Evaluate(
			MatchPlayLoopReadinessTests::MakeState(
				EInitialTurnOrderPlayer::PlayerA,
				0,
				0,
				1,
				1,
				1,
				1));

	TestFalse(TEXT("Ready path does not enter automatic loop"), Ready.bShouldEnterAutomaticLoop);
	TestFalse(TEXT("No-card path does not enter automatic loop"), NoCards.bShouldEnterAutomaticLoop);
	TestFalse(TEXT("No-remaining path does not enter automatic loop"), NoRemaining.bShouldEnterAutomaticLoop);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayLoopReadinessInputUnchangedTest,
	"FMCodex.CoreRules.MatchPlayLoopReadiness.EvaluateDoesNotModifyInputState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayLoopReadinessInputUnchangedTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State =
		MatchPlayLoopReadinessTests::MakeState(
			EInitialTurnOrderPlayer::PlayerA,
			2,
			1);
	const TArray<FName> AvailableCards =
		State.CardUsageState.PlayerACardUsageState.AvailableCardIds;

	const FMatchPlayLoopReadinessResult Result =
		FMatchPlayLoopReadiness::Evaluate(State);

	TestTrue(TEXT("State is ready"), Result.bCanAcceptExternalAttackRequest);
	TestEqual(TEXT("Input PlayerA score remains unchanged"), State.RuntimeState.PlayerAState.Score, 2);
	TestEqual(TEXT("Input PlayerB score remains unchanged"), State.RuntimeState.PlayerBState.Score, 1);
	TestEqual(TEXT("Input used attack count remains unchanged"), State.RuntimeState.PlayerAState.UsedAttackCount, 0);
	TestTrue(TEXT("Input available cards remain unchanged"), State.CardUsageState.PlayerACardUsageState.AvailableCardIds == AvailableCards);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayLoopReadinessNoRandomnessTest,
	"FMCodex.CoreRules.MatchPlayLoopReadiness.ProductionCodeDoesNotUseRandomness",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayLoopReadinessNoRandomnessTest::RunTest(
	const FString& Parameters)
{
	const FString Source =
		MatchPlayLoopReadinessTests::LoadProductionSource();

	TestFalse(TEXT("No Rand call"), Source.Contains(TEXT("Rand(")));
	TestFalse(TEXT("No RandRange call"), Source.Contains(TEXT("RandRange")));
	TestFalse(TEXT("No RandomStream usage"), Source.Contains(TEXT("RandomStream")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayLoopReadinessNoExternalFeatureDependenciesTest,
	"FMCodex.CoreRules.MatchPlayLoopReadiness.ProductionCodeDoesNotRequireExternalFeatureSystems",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayLoopReadinessNoExternalFeatureDependenciesTest::RunTest(
	const FString& Parameters)
{
	const FString Source =
		MatchPlayLoopReadinessTests::LoadProductionSource();

	TestFalse(TEXT("No skill system dependency"), Source.Contains(TEXT("Skill")));
	TestFalse(TEXT("No card database dependency"), Source.Contains(TEXT("Database")));
	TestFalse(TEXT("No UI dependency"), Source.Contains(TEXT("UI")));
	TestFalse(TEXT("No Blueprint dependency"), Source.Contains(TEXT("Blueprint")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayLoopReadinessNoAttackExecutionDependenciesTest,
	"FMCodex.CoreRules.MatchPlayLoopReadiness.ProductionCodeDoesNotCallAttackExecutionFlows",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayLoopReadinessNoAttackExecutionDependenciesTest::RunTest(
	const FString& Parameters)
{
	const FString Source =
		MatchPlayLoopReadinessTests::LoadProductionSource();

	TestFalse(TEXT("No AttackExecutor call"), Source.Contains(TEXT("FMatchPlayAttackExecutor::")));
	TestFalse(TEXT("No AttackStep call"), Source.Contains(TEXT("FMatchPlayAttackStep::")));
	TestFalse(TEXT("No MatchPlayAttackFlow call"), Source.Contains(TEXT("FMatchPlayAttackFlow::")));
	TestFalse(TEXT("No FormulaAttackFlow call"), Source.Contains(TEXT("FFormulaAttackFlow::")));
	TestFalse(TEXT("No FormulaResolver call"), Source.Contains(TEXT("UFormulaResolver::")));
	return true;
}

#endif
