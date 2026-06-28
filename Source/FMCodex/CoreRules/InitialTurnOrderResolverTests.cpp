#include "InitialTurnOrderResolver.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FInitialTurnOrderHigherAttackCountTest,
	"FMCodex.CoreRules.InitialTurnOrderResolver.HigherAttackCount",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FInitialTurnOrderHigherAttackCountTest::RunTest(const FString& Parameters)
{
	const FInitialTurnOrderResult PlayerAFirst =
		FInitialTurnOrderResolver::ResolveInitialTurnOrder(6, 4, 80, 40, 0, 0);
	TestTrue(TEXT("Higher PlayerA attack count resolves successfully"), PlayerAFirst.bSuccess);
	TestEqual(
		TEXT("PlayerA attacks first with more attacks"),
		PlayerAFirst.FirstPlayer,
		EInitialTurnOrderPlayer::PlayerA);
	TestEqual(
		TEXT("PlayerA result records higher attack count reason"),
		PlayerAFirst.Reason,
		EInitialTurnOrderReason::HigherAttackCount);

	const FInitialTurnOrderResult PlayerBFirst =
		FInitialTurnOrderResolver::ResolveInitialTurnOrder(4, 6, 40, 80, 0, 0);
	TestTrue(TEXT("Higher PlayerB attack count resolves successfully"), PlayerBFirst.bSuccess);
	TestEqual(
		TEXT("PlayerB attacks first with more attacks"),
		PlayerBFirst.FirstPlayer,
		EInitialTurnOrderPlayer::PlayerB);
	TestEqual(
		TEXT("Unused tie breaker rolls are not validated"),
		PlayerBFirst.ErrorCodes.Num(),
		0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FInitialTurnOrderLowerRarityScoreTest,
	"FMCodex.CoreRules.InitialTurnOrderResolver.LowerRarityScore",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FInitialTurnOrderLowerRarityScoreTest::RunTest(const FString& Parameters)
{
	const FInitialTurnOrderResult PlayerAFirst =
		FInitialTurnOrderResolver::ResolveInitialTurnOrder(5, 5, 60, 72, 0, 0);
	TestTrue(TEXT("Lower PlayerA rarity score resolves successfully"), PlayerAFirst.bSuccess);
	TestEqual(
		TEXT("PlayerA attacks first with lower rarity score"),
		PlayerAFirst.FirstPlayer,
		EInitialTurnOrderPlayer::PlayerA);
	TestEqual(
		TEXT("PlayerA result records lower rarity score reason"),
		PlayerAFirst.Reason,
		EInitialTurnOrderReason::LowerInitialDeckRarityScore);

	const FInitialTurnOrderResult PlayerBFirst =
		FInitialTurnOrderResolver::ResolveInitialTurnOrder(5, 5, 72, 60, 0, 0);
	TestTrue(TEXT("Lower PlayerB rarity score resolves successfully"), PlayerBFirst.bSuccess);
	TestEqual(
		TEXT("PlayerB attacks first with lower rarity score"),
		PlayerBFirst.FirstPlayer,
		EInitialTurnOrderPlayer::PlayerB);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FInitialTurnOrderLowerTieBreakerRollTest,
	"FMCodex.CoreRules.InitialTurnOrderResolver.LowerTieBreakerRoll",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FInitialTurnOrderLowerTieBreakerRollTest::RunTest(const FString& Parameters)
{
	const FInitialTurnOrderResult PlayerAFirst =
		FInitialTurnOrderResolver::ResolveInitialTurnOrder(5, 5, 72, 72, 2, 6);
	TestTrue(TEXT("Lower PlayerA tie breaker resolves successfully"), PlayerAFirst.bSuccess);
	TestEqual(
		TEXT("PlayerA attacks first with lower tie breaker"),
		PlayerAFirst.FirstPlayer,
		EInitialTurnOrderPlayer::PlayerA);
	TestEqual(
		TEXT("PlayerA result records lower tie breaker reason"),
		PlayerAFirst.Reason,
		EInitialTurnOrderReason::LowerTieBreakerRoll);

	const FInitialTurnOrderResult PlayerBFirst =
		FInitialTurnOrderResolver::ResolveInitialTurnOrder(5, 5, 72, 72, 5, 1);
	TestTrue(TEXT("Lower PlayerB tie breaker resolves successfully"), PlayerBFirst.bSuccess);
	TestEqual(
		TEXT("PlayerB attacks first with lower tie breaker"),
		PlayerBFirst.FirstPlayer,
		EInitialTurnOrderPlayer::PlayerB);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FInitialTurnOrderTieBreakerStillTiedTest,
	"FMCodex.CoreRules.InitialTurnOrderResolver.TieBreakerStillTied",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FInitialTurnOrderTieBreakerStillTiedTest::RunTest(const FString& Parameters)
{
	const FInitialTurnOrderResult Result =
		FInitialTurnOrderResolver::ResolveInitialTurnOrder(5, 5, 72, 72, 4, 4);

	TestFalse(TEXT("Equal tie breaker rolls do not resolve"), Result.bSuccess);
	TestTrue(TEXT("Equal tie breaker rolls request external reroll"), Result.bRequiresTieBreakerReroll);
	TestEqual(
		TEXT("No first player is selected while tied"),
		Result.FirstPlayer,
		EInitialTurnOrderPlayer::None);
	TestEqual(
		TEXT("Tie result records still tied reason"),
		Result.Reason,
		EInitialTurnOrderReason::TieBreakerStillTied);
	TestTrue(
		TEXT("Tie result has structured error code"),
		Result.ErrorCodes.Contains(EInitialTurnOrderErrorCode::TieBreakerStillTied));
	TestTrue(TEXT("Tie result has readable error"), !Result.ErrorMessages.IsEmpty());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FInitialTurnOrderInvalidInputTest,
	"FMCodex.CoreRules.InitialTurnOrderResolver.InvalidInput",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FInitialTurnOrderInvalidInputTest::RunTest(const FString& Parameters)
{
	const FInitialTurnOrderResult InvalidRolls =
		FInitialTurnOrderResolver::ResolveInitialTurnOrder(5, 5, 72, 72, 0, 7);
	TestFalse(TEXT("Tie breaker rolls zero and seven fail"), InvalidRolls.bSuccess);
	TestTrue(
		TEXT("Tie breaker zero reports PlayerA roll error"),
		InvalidRolls.ErrorCodes.Contains(
			EInitialTurnOrderErrorCode::InvalidPlayerATieBreakerRoll));
	TestTrue(
		TEXT("Tie breaker seven reports PlayerB roll error"),
		InvalidRolls.ErrorCodes.Contains(
			EInitialTurnOrderErrorCode::InvalidPlayerBTieBreakerRoll));
	TestEqual(TEXT("Two invalid tie breaker rolls retain two messages"), InvalidRolls.ErrorMessages.Num(), 2);

	const FInitialTurnOrderResult InvalidAttackCounts =
		FInitialTurnOrderResolver::ResolveInitialTurnOrder(-1, -2, 72, 72, 1, 2);
	TestFalse(TEXT("Negative attack counts fail"), InvalidAttackCounts.bSuccess);
	TestTrue(
		TEXT("Negative PlayerA attack count reports error"),
		InvalidAttackCounts.ErrorCodes.Contains(
			EInitialTurnOrderErrorCode::InvalidPlayerAAttackCount));
	TestTrue(
		TEXT("Negative PlayerB attack count reports error"),
		InvalidAttackCounts.ErrorCodes.Contains(
			EInitialTurnOrderErrorCode::InvalidPlayerBAttackCount));

	const FInitialTurnOrderResult InvalidRarityScores =
		FInitialTurnOrderResolver::ResolveInitialTurnOrder(5, 5, -1, -2, 1, 2);
	TestFalse(TEXT("Negative rarity scores fail"), InvalidRarityScores.bSuccess);
	TestTrue(
		TEXT("Negative PlayerA rarity score reports error"),
		InvalidRarityScores.ErrorCodes.Contains(
			EInitialTurnOrderErrorCode::InvalidPlayerAInitialDeckRarityScore));
	TestTrue(
		TEXT("Negative PlayerB rarity score reports error"),
		InvalidRarityScores.ErrorCodes.Contains(
			EInitialTurnOrderErrorCode::InvalidPlayerBInitialDeckRarityScore));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FInitialTurnOrderDeterministicInputTest,
	"FMCodex.CoreRules.InitialTurnOrderResolver.DeterministicInput",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FInitialTurnOrderDeterministicInputTest::RunTest(const FString& Parameters)
{
	const FInitialTurnOrderResult First =
		FInitialTurnOrderResolver::ResolveInitialTurnOrder(5, 5, 72, 72, 3, 6);
	const FInitialTurnOrderResult Second =
		FInitialTurnOrderResolver::ResolveInitialTurnOrder(5, 5, 72, 72, 3, 6);

	TestTrue(TEXT("Repeated input resolves successfully"), First.bSuccess && Second.bSuccess);
	TestEqual(TEXT("Repeated input keeps first player"), First.FirstPlayer, Second.FirstPlayer);
	TestEqual(TEXT("Repeated input keeps reason"), First.Reason, Second.Reason);
	TestEqual(
		TEXT("Repeated input keeps reroll requirement"),
		First.bRequiresTieBreakerReroll,
		Second.bRequiresTieBreakerReroll);
	return true;
}

#endif
