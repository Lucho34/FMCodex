#include "AttackCountResolver.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAttackCountResolverBaseAndRarityTest,
	"FMCodex.CoreRules.AttackCountResolver.BaseAndRarity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAttackCountResolverBaseAndRarityTest::RunTest(const FString& Parameters)
{
	const FAttackCountResolveResult PlayerALead =
		FAttackCountResolver::ResolveAttackCounts(72, 60, 1, 1);
	TestTrue(TEXT("PlayerA lead resolves successfully"), PlayerALead.bSuccess);
	TestEqual(TEXT("PlayerA base attack count is three"), PlayerALead.PlayerABaseAttackCount, 3);
	TestEqual(TEXT("PlayerB base attack count is three"), PlayerALead.PlayerBBaseAttackCount, 3);
	TestEqual(TEXT("PlayerA receives rarity lead bonus"), PlayerALead.PlayerARarityBonusAttackCount, 1);
	TestEqual(TEXT("PlayerB receives no rarity bonus"), PlayerALead.PlayerBRarityBonusAttackCount, 0);

	const FAttackCountResolveResult PlayerBLead =
		FAttackCountResolver::ResolveAttackCounts(60, 72, 1, 1);
	TestEqual(TEXT("PlayerA receives no rarity bonus"), PlayerBLead.PlayerARarityBonusAttackCount, 0);
	TestEqual(TEXT("PlayerB receives rarity lead bonus"), PlayerBLead.PlayerBRarityBonusAttackCount, 1);

	const FAttackCountResolveResult EqualScores =
		FAttackCountResolver::ResolveAttackCounts(72, 72, 1, 1);
	TestEqual(TEXT("Equal score gives PlayerA no rarity bonus"), EqualScores.PlayerARarityBonusAttackCount, 0);
	TestEqual(TEXT("Equal score gives PlayerB no rarity bonus"), EqualScores.PlayerBRarityBonusAttackCount, 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAttackCountResolverD6MappingTest,
	"FMCodex.CoreRules.AttackCountResolver.D6Mapping",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAttackCountResolverD6MappingTest::RunTest(const FString& Parameters)
{
	const int32 ExpectedBonuses[] = { 1, 1, 2, 2, 3, 3 };

	for (int32 Roll = 1; Roll <= 6; ++Roll)
	{
		const FAttackCountResolveResult Result =
			FAttackCountResolver::ResolveAttackCounts(50, 50, Roll, Roll);
		const FString SuccessMessage =
			FString::Printf(TEXT("D6 %d resolves successfully"), Roll);
		const FString PlayerAMessage =
			FString::Printf(TEXT("PlayerA D6 %d maps to expected bonus"), Roll);
		const FString PlayerBMessage =
			FString::Printf(TEXT("PlayerB D6 %d maps to expected bonus"), Roll);

		TestTrue(*SuccessMessage, Result.bSuccess);
		TestEqual(
			*PlayerAMessage,
			Result.PlayerARandomBonusAttackCount,
			ExpectedBonuses[Roll - 1]);
		TestEqual(
			*PlayerBMessage,
			Result.PlayerBRandomBonusAttackCount,
			ExpectedBonuses[Roll - 1]);
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAttackCountResolverInvalidRollTest,
	"FMCodex.CoreRules.AttackCountResolver.InvalidRoll",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAttackCountResolverInvalidRollTest::RunTest(const FString& Parameters)
{
	const FAttackCountResolveResult PlayerAZero =
		FAttackCountResolver::ResolveAttackCounts(50, 50, 0, 1);
	TestFalse(TEXT("PlayerA D6 zero fails"), PlayerAZero.bSuccess);
	TestTrue(
		TEXT("PlayerA D6 zero reports PlayerA error code"),
		PlayerAZero.ErrorCodes.Contains(
			EAttackCountResolveErrorCode::PlayerAD6RollOutOfRange));
	TestTrue(TEXT("PlayerA D6 zero has readable error"), !PlayerAZero.ErrorMessages.IsEmpty());

	const FAttackCountResolveResult PlayerBSeven =
		FAttackCountResolver::ResolveAttackCounts(50, 50, 1, 7);
	TestFalse(TEXT("PlayerB D6 seven fails"), PlayerBSeven.bSuccess);
	TestTrue(
		TEXT("PlayerB D6 seven reports PlayerB error code"),
		PlayerBSeven.ErrorCodes.Contains(
			EAttackCountResolveErrorCode::PlayerBD6RollOutOfRange));
	TestTrue(TEXT("PlayerB D6 seven has readable error"), !PlayerBSeven.ErrorMessages.IsEmpty());

	const FAttackCountResolveResult BothInvalid =
		FAttackCountResolver::ResolveAttackCounts(50, 50, 0, 7);
	TestFalse(TEXT("Two invalid rolls fail"), BothInvalid.bSuccess);
	TestEqual(TEXT("Two invalid rolls retain two error codes"), BothInvalid.ErrorCodes.Num(), 2);
	TestEqual(TEXT("Two invalid rolls retain two messages"), BothInvalid.ErrorMessages.Num(), 2);
	TestEqual(TEXT("Invalid result does not publish PlayerA final count"), BothInvalid.PlayerAAttackCount, 0);
	TestEqual(TEXT("Invalid result does not publish PlayerB final count"), BothInvalid.PlayerBAttackCount, 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAttackCountResolverCanonicalExampleTest,
	"FMCodex.CoreRules.AttackCountResolver.CanonicalExample",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAttackCountResolverCanonicalExampleTest::RunTest(const FString& Parameters)
{
	const FAttackCountResolveResult Result =
		FAttackCountResolver::ResolveAttackCounts(72, 60, 1, 6);

	TestTrue(TEXT("Canonical example resolves successfully"), Result.bSuccess);
	TestEqual(TEXT("PlayerA final attack count is five"), Result.PlayerAAttackCount, 5);
	TestEqual(TEXT("PlayerB final attack count is six"), Result.PlayerBAttackCount, 6);
	TestEqual(TEXT("PlayerA rarity bonus is one"), Result.PlayerARarityBonusAttackCount, 1);
	TestEqual(TEXT("PlayerA D6 bonus is one"), Result.PlayerARandomBonusAttackCount, 1);
	TestEqual(TEXT("PlayerB rarity bonus is zero"), Result.PlayerBRarityBonusAttackCount, 0);
	TestEqual(TEXT("PlayerB D6 bonus is three"), Result.PlayerBRandomBonusAttackCount, 3);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAttackCountResolverDeterministicInputTest,
	"FMCodex.CoreRules.AttackCountResolver.DeterministicInput",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAttackCountResolverDeterministicInputTest::RunTest(const FString& Parameters)
{
	const FAttackCountResolveResult First =
		FAttackCountResolver::ResolveAttackCounts(65, 70, 4, 5);
	const FAttackCountResolveResult Second =
		FAttackCountResolver::ResolveAttackCounts(65, 70, 4, 5);

	TestTrue(TEXT("Repeated input resolves successfully"), First.bSuccess && Second.bSuccess);
	TestEqual(TEXT("Repeated input keeps PlayerA final count"), First.PlayerAAttackCount, Second.PlayerAAttackCount);
	TestEqual(TEXT("Repeated input keeps PlayerB final count"), First.PlayerBAttackCount, Second.PlayerBAttackCount);
	TestEqual(
		TEXT("Repeated input keeps PlayerA D6 bonus"),
		First.PlayerARandomBonusAttackCount,
		Second.PlayerARandomBonusAttackCount);
	TestEqual(
		TEXT("Repeated input keeps PlayerB D6 bonus"),
		First.PlayerBRandomBonusAttackCount,
		Second.PlayerBRandomBonusAttackCount);
	return true;
}

#endif
