#include "MatchCardUsageInitializer.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

namespace MatchCardUsageInitializerTests
{
	const FName SharedCard(TEXT("SharedCard"));
	const FName PlayerACard1(TEXT("PlayerACard1"));
	const FName PlayerACard2(TEXT("PlayerACard2"));
	const FName PlayerBCard1(TEXT("PlayerBCard1"));
	const FName PlayerBCard2(TEXT("PlayerBCard2"));
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchCardUsageInitializeSuccessTest,
	"FMCodex.CoreRules.MatchCardUsageInitializer.Success",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchCardUsageInitializeSuccessTest::RunTest(
	const FString& Parameters)
{
	const TArray<FName> PlayerACardIds =
		{
			MatchCardUsageInitializerTests::PlayerACard1,
			MatchCardUsageInitializerTests::PlayerACard2
		};
	const TArray<FName> PlayerBCardIds =
		{
			MatchCardUsageInitializerTests::PlayerBCard1,
			MatchCardUsageInitializerTests::PlayerBCard2
		};
	const TArray<FName> PlayerACardIdsSnapshot = PlayerACardIds;
	const TArray<FName> PlayerBCardIdsSnapshot = PlayerBCardIds;

	const FMatchCardUsageInitializeResult Result =
		FMatchCardUsageInitializer::InitializeMatchCardUsageState(
			PlayerACardIds,
			PlayerBCardIds);

	TestTrue(TEXT("Initialization succeeds"), Result.bSuccess);
	TestTrue(
		TEXT("PlayerA available cards preserve the input"),
		Result.InitializedCardUsageState.PlayerACardUsageState.AvailableCardIds
			== PlayerACardIds);
	TestTrue(
		TEXT("PlayerB available cards preserve the input"),
		Result.InitializedCardUsageState.PlayerBCardUsageState.AvailableCardIds
			== PlayerBCardIds);
	TestTrue(
		TEXT("PlayerA used cards start empty"),
		Result.InitializedCardUsageState.PlayerACardUsageState.UsedCardIds.IsEmpty());
	TestTrue(
		TEXT("PlayerB used cards start empty"),
		Result.InitializedCardUsageState.PlayerBCardUsageState.UsedCardIds.IsEmpty());
	TestTrue(
		TEXT("PlayerA input remains unchanged"),
		PlayerACardIds == PlayerACardIdsSnapshot);
	TestTrue(
		TEXT("PlayerB input remains unchanged"),
		PlayerBCardIds == PlayerBCardIdsSnapshot);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchCardUsageInitializePlayerAInvalidTest,
	"FMCodex.CoreRules.MatchCardUsageInitializer.PlayerAInvalidCard",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchCardUsageInitializePlayerAInvalidTest::RunTest(
	const FString& Parameters)
{
	const TArray<FName> PlayerACardIds =
		{ MatchCardUsageInitializerTests::PlayerACard1, NAME_None };
	const TArray<FName> PlayerBCardIds =
		{ MatchCardUsageInitializerTests::PlayerBCard1 };

	const FMatchCardUsageInitializeResult Result =
		FMatchCardUsageInitializer::InitializeMatchCardUsageState(
			PlayerACardIds,
			PlayerBCardIds);

	TestFalse(TEXT("Invalid PlayerA CardId fails"), Result.bSuccess);
	TestEqual(
		TEXT("PlayerA invalid error is returned"),
		Result.ErrorCode,
		EMatchCardUsageInitializeErrorCode::InvalidPlayerACardId);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchCardUsageInitializePlayerBInvalidTest,
	"FMCodex.CoreRules.MatchCardUsageInitializer.PlayerBInvalidCard",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchCardUsageInitializePlayerBInvalidTest::RunTest(
	const FString& Parameters)
{
	const TArray<FName> PlayerACardIds =
		{ MatchCardUsageInitializerTests::PlayerACard1 };
	const TArray<FName> PlayerBCardIds =
		{ NAME_None };

	const FMatchCardUsageInitializeResult Result =
		FMatchCardUsageInitializer::InitializeMatchCardUsageState(
			PlayerACardIds,
			PlayerBCardIds);

	TestFalse(TEXT("Invalid PlayerB CardId fails"), Result.bSuccess);
	TestEqual(
		TEXT("PlayerB invalid error is returned"),
		Result.ErrorCode,
		EMatchCardUsageInitializeErrorCode::InvalidPlayerBCardId);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchCardUsageInitializePlayerADuplicateTest,
	"FMCodex.CoreRules.MatchCardUsageInitializer.PlayerADuplicateCard",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchCardUsageInitializePlayerADuplicateTest::RunTest(
	const FString& Parameters)
{
	const TArray<FName> PlayerACardIds =
		{
			MatchCardUsageInitializerTests::PlayerACard1,
			MatchCardUsageInitializerTests::PlayerACard1
		};
	const TArray<FName> PlayerBCardIds =
		{ MatchCardUsageInitializerTests::PlayerBCard1 };

	const FMatchCardUsageInitializeResult Result =
		FMatchCardUsageInitializer::InitializeMatchCardUsageState(
			PlayerACardIds,
			PlayerBCardIds);

	TestFalse(TEXT("Duplicate PlayerA CardId fails"), Result.bSuccess);
	TestEqual(
		TEXT("PlayerA duplicate error is returned"),
		Result.ErrorCode,
		EMatchCardUsageInitializeErrorCode::DuplicatePlayerACardId);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchCardUsageInitializePlayerBDuplicateTest,
	"FMCodex.CoreRules.MatchCardUsageInitializer.PlayerBDuplicateCard",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchCardUsageInitializePlayerBDuplicateTest::RunTest(
	const FString& Parameters)
{
	const TArray<FName> PlayerACardIds =
		{ MatchCardUsageInitializerTests::PlayerACard1 };
	const TArray<FName> PlayerBCardIds =
		{
			MatchCardUsageInitializerTests::PlayerBCard1,
			MatchCardUsageInitializerTests::PlayerBCard1
		};

	const FMatchCardUsageInitializeResult Result =
		FMatchCardUsageInitializer::InitializeMatchCardUsageState(
			PlayerACardIds,
			PlayerBCardIds);

	TestFalse(TEXT("Duplicate PlayerB CardId fails"), Result.bSuccess);
	TestEqual(
		TEXT("PlayerB duplicate error is returned"),
		Result.ErrorCode,
		EMatchCardUsageInitializeErrorCode::DuplicatePlayerBCardId);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchCardUsageInitializeSharedCardTest,
	"FMCodex.CoreRules.MatchCardUsageInitializer.SharedCardAllowed",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchCardUsageInitializeSharedCardTest::RunTest(
	const FString& Parameters)
{
	const TArray<FName> PlayerACardIds =
		{ MatchCardUsageInitializerTests::SharedCard };
	const TArray<FName> PlayerBCardIds =
		{ MatchCardUsageInitializerTests::SharedCard };

	const FMatchCardUsageInitializeResult Result =
		FMatchCardUsageInitializer::InitializeMatchCardUsageState(
			PlayerACardIds,
			PlayerBCardIds);

	TestTrue(TEXT("Shared CardId across players is allowed"), Result.bSuccess);
	TestEqual(
		TEXT("PlayerA keeps the shared CardId"),
		Result.InitializedCardUsageState.PlayerACardUsageState.AvailableCardIds[0],
		MatchCardUsageInitializerTests::SharedCard);
	TestEqual(
		TEXT("PlayerB keeps the shared CardId"),
		Result.InitializedCardUsageState.PlayerBCardUsageState.AvailableCardIds[0],
		MatchCardUsageInitializerTests::SharedCard);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchCardUsageInitializeFailureAtomicTest,
	"FMCodex.CoreRules.MatchCardUsageInitializer.FailureIsAtomic",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchCardUsageInitializeFailureAtomicTest::RunTest(
	const FString& Parameters)
{
	const TArray<FName> PlayerACardIds =
		{
			MatchCardUsageInitializerTests::PlayerACard1,
			MatchCardUsageInitializerTests::PlayerACard2
		};
	const TArray<FName> PlayerBCardIds =
		{ MatchCardUsageInitializerTests::PlayerBCard1, NAME_None };

	const FMatchCardUsageInitializeResult Result =
		FMatchCardUsageInitializer::InitializeMatchCardUsageState(
			PlayerACardIds,
			PlayerBCardIds);

	TestFalse(TEXT("Invalid PlayerB input fails"), Result.bSuccess);
	TestTrue(
		TEXT("PlayerA is not partially initialized"),
		Result.InitializedCardUsageState.PlayerACardUsageState.AvailableCardIds.IsEmpty());
	TestTrue(
		TEXT("PlayerB is not partially initialized"),
		Result.InitializedCardUsageState.PlayerBCardUsageState.AvailableCardIds.IsEmpty());
	TestTrue(
		TEXT("PlayerA used cards remain empty"),
		Result.InitializedCardUsageState.PlayerACardUsageState.UsedCardIds.IsEmpty());
	TestTrue(
		TEXT("PlayerB used cards remain empty"),
		Result.InitializedCardUsageState.PlayerBCardUsageState.UsedCardIds.IsEmpty());
	return true;
}

#endif
