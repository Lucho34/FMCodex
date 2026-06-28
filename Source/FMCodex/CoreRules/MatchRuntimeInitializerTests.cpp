#include "MatchRuntimeInitializer.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

namespace MatchRuntimeInitializerTests
{
	FMatchOpeningResolveResult MakeSuccessfulOpeningResult(
		const EInitialTurnOrderPlayer FirstPlayer = EInitialTurnOrderPlayer::PlayerB)
	{
		FMatchOpeningResolveResult OpeningResult;
		OpeningResult.bSuccess = true;

		OpeningResult.MatchInitializationResult.bSuccess = true;
		OpeningResult.MatchInitializationResult.PlayerADeckValidation.bIsValid = true;
		OpeningResult.MatchInitializationResult.PlayerADeckValidation.InitialDeckRarityScore = 72;
		OpeningResult.MatchInitializationResult.PlayerADeckValidation.GoalkeeperCardId =
			TEXT("A_GK");
		OpeningResult.MatchInitializationResult.PlayerBDeckValidation.bIsValid = true;
		OpeningResult.MatchInitializationResult.PlayerBDeckValidation.InitialDeckRarityScore = 60;
		OpeningResult.MatchInitializationResult.PlayerBDeckValidation.GoalkeeperCardId =
			TEXT("B_GK");

		OpeningResult.AttackCountResult.bSuccess = true;
		OpeningResult.AttackCountResult.PlayerAAttackCount = 5;
		OpeningResult.AttackCountResult.PlayerBAttackCount = 6;

		OpeningResult.InitialTurnOrderResult.bSuccess = true;
		OpeningResult.InitialTurnOrderResult.FirstPlayer = FirstPlayer;
		return OpeningResult;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchRuntimeInitializerSuccessfulStateTest,
	"FMCodex.CoreRules.MatchRuntimeInitializer.SuccessfulState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchRuntimeInitializerSuccessfulStateTest::RunTest(const FString& Parameters)
{
	const FMatchOpeningResolveResult OpeningResult =
		MatchRuntimeInitializerTests::MakeSuccessfulOpeningResult();
	const FMatchRuntimeInitializeResult Result =
		FMatchRuntimeInitializer::InitializeRuntimeState(OpeningResult);

	TestTrue(TEXT("Successful opening initializes runtime state"), Result.bSuccess);
	TestTrue(TEXT("Runtime state is marked initialized"), Result.RuntimeState.bIsInitialized);
	TestTrue(TEXT("Successful runtime initialization has no errors"), Result.ErrorCodes.IsEmpty());
	TestEqual(TEXT("PlayerA total attack count is copied"), Result.RuntimeState.PlayerAState.TotalAttackCount, 5);
	TestEqual(TEXT("PlayerB total attack count is copied"), Result.RuntimeState.PlayerBState.TotalAttackCount, 6);
	TestEqual(TEXT("PlayerA used attacks start at zero"), Result.RuntimeState.PlayerAState.UsedAttackCount, 0);
	TestEqual(TEXT("PlayerB used attacks start at zero"), Result.RuntimeState.PlayerBState.UsedAttackCount, 0);
	TestEqual(TEXT("PlayerA score starts at zero"), Result.RuntimeState.PlayerAState.Score, 0);
	TestEqual(TEXT("PlayerB score starts at zero"), Result.RuntimeState.PlayerBState.Score, 0);
	TestEqual(
		TEXT("PlayerA goalkeeper ID is copied"),
		Result.RuntimeState.PlayerAState.GoalkeeperCardId,
		FString(TEXT("A_GK")));
	TestEqual(
		TEXT("PlayerB goalkeeper ID is copied"),
		Result.RuntimeState.PlayerBState.GoalkeeperCardId,
		FString(TEXT("B_GK")));
	TestEqual(
		TEXT("PlayerA initial rarity score is copied"),
		Result.RuntimeState.PlayerAState.InitialDeckRarityScore,
		72);
	TestEqual(
		TEXT("PlayerB initial rarity score is copied"),
		Result.RuntimeState.PlayerBState.InitialDeckRarityScore,
		60);
	TestEqual(
		TEXT("First player is copied"),
		Result.RuntimeState.FirstPlayer,
		EInitialTurnOrderPlayer::PlayerB);
	TestEqual(
		TEXT("Second player is the other player"),
		Result.RuntimeState.SecondPlayer,
		EInitialTurnOrderPlayer::PlayerA);
	TestEqual(
		TEXT("Current attacker starts as first player"),
		Result.RuntimeState.CurrentAttackingPlayer,
		EInitialTurnOrderPlayer::PlayerB);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchRuntimeInitializerFailedOpeningTest,
	"FMCodex.CoreRules.MatchRuntimeInitializer.FailedOpening",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchRuntimeInitializerFailedOpeningTest::RunTest(const FString& Parameters)
{
	FMatchOpeningResolveResult OpeningResult;
	OpeningResult.ErrorMessages.Add(TEXT("Tie breaker reroll is required."));

	const FMatchRuntimeInitializeResult Result =
		FMatchRuntimeInitializer::InitializeRuntimeState(OpeningResult);

	TestFalse(TEXT("Failed opening cannot initialize runtime"), Result.bSuccess);
	TestFalse(TEXT("Failed runtime state is not marked initialized"), Result.RuntimeState.bIsInitialized);
	TestTrue(
		TEXT("Failed opening reports structured error"),
		Result.ErrorCodes.Contains(
			EMatchRuntimeInitializeErrorCode::OpeningResultFailed));
	TestTrue(TEXT("Failed opening retains readable error"), !Result.ErrorMessages.IsEmpty());
	TestFalse(
		TEXT("Failed opening snapshot is retained"),
		Result.RuntimeState.OpeningResultSnapshot.bSuccess);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchRuntimeInitializerPlayerOrderTest,
	"FMCodex.CoreRules.MatchRuntimeInitializer.PlayerOrder",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchRuntimeInitializerPlayerOrderTest::RunTest(const FString& Parameters)
{
	const FMatchOpeningResolveResult OpeningResult =
		MatchRuntimeInitializerTests::MakeSuccessfulOpeningResult(
			EInitialTurnOrderPlayer::PlayerA);
	const FMatchRuntimeInitializeResult Result =
		FMatchRuntimeInitializer::InitializeRuntimeState(OpeningResult);

	TestTrue(TEXT("PlayerA-first opening initializes"), Result.bSuccess);
	TestEqual(
		TEXT("PlayerA remains first"),
		Result.RuntimeState.FirstPlayer,
		EInitialTurnOrderPlayer::PlayerA);
	TestEqual(
		TEXT("PlayerB becomes second"),
		Result.RuntimeState.SecondPlayer,
		EInitialTurnOrderPlayer::PlayerB);
	TestEqual(
		TEXT("PlayerA starts as current attacker"),
		Result.RuntimeState.CurrentAttackingPlayer,
		EInitialTurnOrderPlayer::PlayerA);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchRuntimeInitializerSnapshotTest,
	"FMCodex.CoreRules.MatchRuntimeInitializer.Snapshot",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchRuntimeInitializerSnapshotTest::RunTest(const FString& Parameters)
{
	FMatchOpeningResolveResult OpeningResult =
		MatchRuntimeInitializerTests::MakeSuccessfulOpeningResult();
	const FMatchRuntimeInitializeResult Result =
		FMatchRuntimeInitializer::InitializeRuntimeState(OpeningResult);

	OpeningResult.bSuccess = false;
	OpeningResult.AttackCountResult.PlayerAAttackCount = 99;
	OpeningResult.MatchInitializationResult.PlayerADeckValidation.GoalkeeperCardId =
		TEXT("CHANGED_GK");

	TestTrue(TEXT("Snapshot initialization succeeds"), Result.bSuccess);
	TestTrue(TEXT("Opening snapshot retains success"), Result.RuntimeState.OpeningResultSnapshot.bSuccess);
	TestEqual(
		TEXT("Opening snapshot keeps original attack count"),
		Result.RuntimeState.OpeningResultSnapshot.AttackCountResult.PlayerAAttackCount,
		5);
	TestEqual(
		TEXT("Opening snapshot keeps original goalkeeper"),
		Result.RuntimeState.OpeningResultSnapshot.MatchInitializationResult
			.PlayerADeckValidation.GoalkeeperCardId,
		FString(TEXT("A_GK")));
	TestEqual(
		TEXT("Runtime state remains independent from source result"),
		Result.RuntimeState.PlayerAState.TotalAttackCount,
		5);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchRuntimeInitializerInvalidFirstPlayerTest,
	"FMCodex.CoreRules.MatchRuntimeInitializer.InvalidFirstPlayer",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchRuntimeInitializerInvalidFirstPlayerTest::RunTest(const FString& Parameters)
{
	const FMatchOpeningResolveResult OpeningResult =
		MatchRuntimeInitializerTests::MakeSuccessfulOpeningResult(
			EInitialTurnOrderPlayer::None);
	const FMatchRuntimeInitializeResult Result =
		FMatchRuntimeInitializer::InitializeRuntimeState(OpeningResult);

	TestFalse(TEXT("Successful opening without first player is rejected"), Result.bSuccess);
	TestFalse(TEXT("Invalid player runtime state is not initialized"), Result.RuntimeState.bIsInitialized);
	TestTrue(
		TEXT("Invalid first player reports structured error"),
		Result.ErrorCodes.Contains(
			EMatchRuntimeInitializeErrorCode::InvalidFirstPlayer));
	return true;
}

#endif
