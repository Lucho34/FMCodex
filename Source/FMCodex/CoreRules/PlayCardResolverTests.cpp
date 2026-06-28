#include "PlayCardResolver.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

namespace PlayCardResolverTests
{
	const FName CardA1(TEXT("CardA1"));
	const FName CardA2(TEXT("CardA2"));
	const FName CardA3(TEXT("CardA3"));
	const FName CardB1(TEXT("CardB1"));
	const FName CardB2(TEXT("CardB2"));
	const FName CardB3(TEXT("CardB3"));

	FMatchCardUsageState MakeState()
	{
		FMatchCardUsageState State;
		State.PlayerACardUsageState.AvailableCardIds =
			{ CardA1, CardA2, CardA3 };
		State.PlayerBCardUsageState.AvailableCardIds =
			{ CardB1, CardB2, CardB3 };
		return State;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlayCardPlayerASuccessTest,
	"FMCodex.CoreRules.PlayCardResolver.PlayerASuccess",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPlayCardPlayerASuccessTest::RunTest(const FString& Parameters)
{
	const FMatchCardUsageState State =
		PlayCardResolverTests::MakeState();
	const FPlayCardResolveResult Result =
		FPlayCardResolver::PlayCard(
			State,
			EInitialTurnOrderPlayer::PlayerA,
			PlayCardResolverTests::CardA1);

	TestTrue(TEXT("PlayerA can play an available card"), Result.bSuccess);
	TestEqual(TEXT("PlayerA is reported"), Result.PlayerSide, EInitialTurnOrderPlayer::PlayerA);
	TestEqual(TEXT("Played card is reported"), Result.CardId, PlayCardResolverTests::CardA1);
	TestFalse(
		TEXT("PlayerA card leaves available cards"),
		Result.UpdatedMatchCardUsageState.PlayerACardUsageState
			.AvailableCardIds.Contains(PlayCardResolverTests::CardA1));
	TestTrue(
		TEXT("PlayerA card enters used cards"),
		Result.UpdatedMatchCardUsageState.PlayerACardUsageState
			.UsedCardIds.Contains(PlayCardResolverTests::CardA1));
	TestTrue(TEXT("CardUsageResolver result is retained"), Result.CardUsageResult.bSuccess);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlayCardPlayerBSuccessTest,
	"FMCodex.CoreRules.PlayCardResolver.PlayerBSuccess",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPlayCardPlayerBSuccessTest::RunTest(const FString& Parameters)
{
	const FMatchCardUsageState State =
		PlayCardResolverTests::MakeState();
	const FPlayCardResolveResult Result =
		FPlayCardResolver::PlayCard(
			State,
			EInitialTurnOrderPlayer::PlayerB,
			PlayCardResolverTests::CardB1);

	TestTrue(TEXT("PlayerB can play an available card"), Result.bSuccess);
	TestEqual(TEXT("PlayerB is reported"), Result.PlayerSide, EInitialTurnOrderPlayer::PlayerB);
	TestFalse(
		TEXT("PlayerB card leaves available cards"),
		Result.UpdatedMatchCardUsageState.PlayerBCardUsageState
			.AvailableCardIds.Contains(PlayCardResolverTests::CardB1));
	TestTrue(
		TEXT("PlayerB card enters used cards"),
		Result.UpdatedMatchCardUsageState.PlayerBCardUsageState
			.UsedCardIds.Contains(PlayCardResolverTests::CardB1));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlayCardPlayerAIsolationTest,
	"FMCodex.CoreRules.PlayCardResolver.PlayerAIsolation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPlayCardPlayerAIsolationTest::RunTest(const FString& Parameters)
{
	const FMatchCardUsageState State =
		PlayCardResolverTests::MakeState();
	const FPlayCardResolveResult Result =
		FPlayCardResolver::PlayCard(
			State,
			EInitialTurnOrderPlayer::PlayerA,
			PlayCardResolverTests::CardA2);

	TestTrue(TEXT("PlayerA card play succeeds"), Result.bSuccess);
	TestTrue(
		TEXT("PlayerB available cards are unchanged"),
		Result.UpdatedMatchCardUsageState.PlayerBCardUsageState
			.AvailableCardIds
			== State.PlayerBCardUsageState.AvailableCardIds);
	TestTrue(
		TEXT("PlayerB used cards are unchanged"),
		Result.UpdatedMatchCardUsageState.PlayerBCardUsageState
			.UsedCardIds
			== State.PlayerBCardUsageState.UsedCardIds);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlayCardPlayerBIsolationTest,
	"FMCodex.CoreRules.PlayCardResolver.PlayerBIsolation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPlayCardPlayerBIsolationTest::RunTest(const FString& Parameters)
{
	const FMatchCardUsageState State =
		PlayCardResolverTests::MakeState();
	const FPlayCardResolveResult Result =
		FPlayCardResolver::PlayCard(
			State,
			EInitialTurnOrderPlayer::PlayerB,
			PlayCardResolverTests::CardB2);

	TestTrue(TEXT("PlayerB card play succeeds"), Result.bSuccess);
	TestTrue(
		TEXT("PlayerA available cards are unchanged"),
		Result.UpdatedMatchCardUsageState.PlayerACardUsageState
			.AvailableCardIds
			== State.PlayerACardUsageState.AvailableCardIds);
	TestTrue(
		TEXT("PlayerA used cards are unchanged"),
		Result.UpdatedMatchCardUsageState.PlayerACardUsageState
			.UsedCardIds
			== State.PlayerACardUsageState.UsedCardIds);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlayCardInputUnchangedTest,
	"FMCodex.CoreRules.PlayCardResolver.InputStateUnchanged",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPlayCardInputUnchangedTest::RunTest(const FString& Parameters)
{
	const FMatchCardUsageState State =
		PlayCardResolverTests::MakeState();
	const FPlayCardResolveResult Result =
		FPlayCardResolver::PlayCard(
			State,
			EInitialTurnOrderPlayer::PlayerA,
			PlayCardResolverTests::CardA1);

	TestTrue(TEXT("Card play succeeds"), Result.bSuccess);
	TestTrue(
		TEXT("Input PlayerA still has the card available"),
		State.PlayerACardUsageState.AvailableCardIds.Contains(
			PlayCardResolverTests::CardA1));
	TestTrue(
		TEXT("Input PlayerA used cards remain empty"),
		State.PlayerACardUsageState.UsedCardIds.IsEmpty());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlayCardInvalidCardIdTest,
	"FMCodex.CoreRules.PlayCardResolver.InvalidCardId",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPlayCardInvalidCardIdTest::RunTest(const FString& Parameters)
{
	const FMatchCardUsageState State =
		PlayCardResolverTests::MakeState();
	const FPlayCardResolveResult Result =
		FPlayCardResolver::PlayCard(
			State,
			EInitialTurnOrderPlayer::PlayerA,
			NAME_None);

	TestFalse(TEXT("None CardId is rejected"), Result.bSuccess);
	TestEqual(
		TEXT("None CardId reports invalid ID"),
		Result.ErrorCode,
		EPlayCardResolveErrorCode::InvalidCardId);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlayCardInvalidPlayerSideTest,
	"FMCodex.CoreRules.PlayCardResolver.InvalidPlayerSide",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPlayCardInvalidPlayerSideTest::RunTest(const FString& Parameters)
{
	const FMatchCardUsageState State =
		PlayCardResolverTests::MakeState();
	const FPlayCardResolveResult Result =
		FPlayCardResolver::PlayCard(
			State,
			EInitialTurnOrderPlayer::None,
			PlayCardResolverTests::CardA1);

	TestFalse(TEXT("None player side is rejected"), Result.bSuccess);
	TestEqual(
		TEXT("None player reports invalid side"),
		Result.ErrorCode,
		EPlayCardResolveErrorCode::InvalidPlayerSide);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlayCardNotAvailableTest,
	"FMCodex.CoreRules.PlayCardResolver.CardNotAvailable",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPlayCardNotAvailableTest::RunTest(const FString& Parameters)
{
	const FMatchCardUsageState State =
		PlayCardResolverTests::MakeState();
	const FPlayCardResolveResult Result =
		FPlayCardResolver::PlayCard(
			State,
			EInitialTurnOrderPlayer::PlayerA,
			FName(TEXT("UnknownCard")));

	TestFalse(TEXT("Unavailable card is rejected"), Result.bSuccess);
	TestEqual(
		TEXT("Unavailable card error is mapped"),
		Result.ErrorCode,
		EPlayCardResolveErrorCode::CardNotAvailable);
	TestEqual(
		TEXT("Underlying error is retained"),
		Result.CardUsageResult.ErrorCode,
		ECardUsageResolveErrorCode::CardNotAvailable);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlayCardAlreadyUsedTest,
	"FMCodex.CoreRules.PlayCardResolver.CardAlreadyUsed",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPlayCardAlreadyUsedTest::RunTest(const FString& Parameters)
{
	FMatchCardUsageState State =
		PlayCardResolverTests::MakeState();
	State.PlayerACardUsageState.AvailableCardIds.Remove(
		PlayCardResolverTests::CardA1);
	State.PlayerACardUsageState.UsedCardIds.Add(
		PlayCardResolverTests::CardA1);

	const FPlayCardResolveResult Result =
		FPlayCardResolver::PlayCard(
			State,
			EInitialTurnOrderPlayer::PlayerA,
			PlayCardResolverTests::CardA1);

	TestFalse(TEXT("Used card cannot be played again"), Result.bSuccess);
	TestEqual(
		TEXT("Used card error is mapped"),
		Result.ErrorCode,
		EPlayCardResolveErrorCode::CardAlreadyUsed);
	TestEqual(
		TEXT("Underlying used-card error is retained"),
		Result.CardUsageResult.ErrorCode,
		ECardUsageResolveErrorCode::CardAlreadyUsed);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlayCardPlayerACannotUsePlayerBCardTest,
	"FMCodex.CoreRules.PlayCardResolver.PlayerACannotUsePlayerBCard",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPlayCardPlayerACannotUsePlayerBCardTest::RunTest(
	const FString& Parameters)
{
	const FMatchCardUsageState State =
		PlayCardResolverTests::MakeState();
	const FPlayCardResolveResult Result =
		FPlayCardResolver::PlayCard(
			State,
			EInitialTurnOrderPlayer::PlayerA,
			PlayCardResolverTests::CardB1);

	TestFalse(TEXT("PlayerA cannot use PlayerB-only card"), Result.bSuccess);
	TestEqual(
		TEXT("Cross-player card is unavailable"),
		Result.ErrorCode,
		EPlayCardResolveErrorCode::CardNotAvailable);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlayCardPlayerBCannotUsePlayerACardTest,
	"FMCodex.CoreRules.PlayCardResolver.PlayerBCannotUsePlayerACard",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPlayCardPlayerBCannotUsePlayerACardTest::RunTest(
	const FString& Parameters)
{
	const FMatchCardUsageState State =
		PlayCardResolverTests::MakeState();
	const FPlayCardResolveResult Result =
		FPlayCardResolver::PlayCard(
			State,
			EInitialTurnOrderPlayer::PlayerB,
			PlayCardResolverTests::CardA1);

	TestFalse(TEXT("PlayerB cannot use PlayerA-only card"), Result.bSuccess);
	TestEqual(
		TEXT("Cross-player card is unavailable"),
		Result.ErrorCode,
		EPlayCardResolveErrorCode::CardNotAvailable);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlayCardInvalidSelectedUsageStateTest,
	"FMCodex.CoreRules.PlayCardResolver.InvalidSelectedUsageState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPlayCardInvalidSelectedUsageStateTest::RunTest(
	const FString& Parameters)
{
	FMatchCardUsageState State =
		PlayCardResolverTests::MakeState();
	State.PlayerBCardUsageState.AvailableCardIds.Add(
		PlayCardResolverTests::CardB1);

	const FPlayCardResolveResult Result =
		FPlayCardResolver::PlayCard(
			State,
			EInitialTurnOrderPlayer::PlayerB,
			PlayCardResolverTests::CardB2);

	TestFalse(TEXT("Invalid selected player state is rejected"), Result.bSuccess);
	TestEqual(
		TEXT("Invalid selected state is mapped"),
		Result.ErrorCode,
		EPlayCardResolveErrorCode::InvalidMatchCardUsageState);
	TestEqual(
		TEXT("Underlying duplicate error is retained"),
		Result.CardUsageResult.ErrorCode,
		ECardUsageResolveErrorCode::DuplicateCardInAvailable);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlayCardConsecutivePlaysTest,
	"FMCodex.CoreRules.PlayCardResolver.ConsecutivePlays",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPlayCardConsecutivePlaysTest::RunTest(const FString& Parameters)
{
	const FMatchCardUsageState State =
		PlayCardResolverTests::MakeState();
	const FPlayCardResolveResult FirstResult =
		FPlayCardResolver::PlayCard(
			State,
			EInitialTurnOrderPlayer::PlayerA,
			PlayCardResolverTests::CardA1);
	const FPlayCardResolveResult SecondResult =
		FPlayCardResolver::PlayCard(
			FirstResult.UpdatedMatchCardUsageState,
			EInitialTurnOrderPlayer::PlayerB,
			PlayCardResolverTests::CardB1);
	const FPlayCardResolveResult ThirdResult =
		FPlayCardResolver::PlayCard(
			SecondResult.UpdatedMatchCardUsageState,
			EInitialTurnOrderPlayer::PlayerA,
			PlayCardResolverTests::CardA3);

	TestTrue(TEXT("First play succeeds"), FirstResult.bSuccess);
	TestTrue(TEXT("Second play succeeds"), SecondResult.bSuccess);
	TestTrue(TEXT("Third play succeeds"), ThirdResult.bSuccess);
	TestEqual(
		TEXT("PlayerA has used two cards"),
		ThirdResult.UpdatedMatchCardUsageState.PlayerACardUsageState
			.UsedCardIds.Num(),
		2);
	TestEqual(
		TEXT("PlayerB has used one card"),
		ThirdResult.UpdatedMatchCardUsageState.PlayerBCardUsageState
			.UsedCardIds.Num(),
		1);
	TestTrue(
		TEXT("PlayerA unused card remains available"),
		ThirdResult.UpdatedMatchCardUsageState.PlayerACardUsageState
			.AvailableCardIds.Contains(PlayCardResolverTests::CardA2));
	return true;
}

#endif
