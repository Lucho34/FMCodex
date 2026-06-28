#include "MatchInitializer.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

namespace MatchInitializerTests
{
	const FName PlayerAId(TEXT("PlayerA"));
	const FName PlayerBId(TEXT("PlayerB"));

	FPlayerCardData MakeCard(
		const FString& CardId,
		const ECardRarity Rarity,
		const bool bIsGoalkeeper)
	{
		FPlayerCardData Card;
		Card.CardId = FName(*CardId);
		Card.Rarity = Rarity;
		Card.bIsGoalkeeper = bIsGoalkeeper;
		Card.PositionTypes = {
			bIsGoalkeeper
				? EPlayerPositionType::Goalkeeper
				: EPlayerPositionType::Attack
		};
		return Card;
	}

	TArray<FPlayerCardData> MakeValidDeck(
		const FString& Prefix,
		const ECardRarity Rarity)
	{
		TArray<FPlayerCardData> Deck;
		Deck.Reserve(20);

		for (int32 Index = 0; Index < 19; ++Index)
		{
			Deck.Add(MakeCard(
				FString::Printf(TEXT("%s_OUT_%02d"), *Prefix, Index),
				Rarity,
				false));
		}

		Deck.Add(MakeCard(
			FString::Printf(TEXT("%s_GK"), *Prefix),
			Rarity,
			true));
		return Deck;
	}

	bool HasMessageForPlayer(
		const FMatchInitializationResult& Result,
		const FString& PlayerLabel)
	{
		return Result.ErrorMessages.ContainsByPredicate(
			[&PlayerLabel](const FString& Message)
			{
				return Message.StartsWith(PlayerLabel + TEXT(":"));
			});
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchInitializerValidDecksTest,
	"FMCodex.CoreRules.MatchInitializer.ValidDecks",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchInitializerValidDecksTest::RunTest(const FString& Parameters)
{
	const TArray<FPlayerCardData> PlayerADeck =
		MatchInitializerTests::MakeValidDeck(TEXT("A"), ECardRarity::Common);
	const TArray<FPlayerCardData> PlayerBDeck =
		MatchInitializerTests::MakeValidDeck(TEXT("B"), ECardRarity::Regional);

	const FMatchInitializationResult Result =
		FMatchInitializer::InitializeMatch(PlayerADeck, PlayerBDeck);

	TestTrue(TEXT("Two legal decks initialize successfully"), Result.bSuccess);
	TestTrue(TEXT("PlayerA validation is retained"), Result.PlayerADeckValidation.bIsValid);
	TestTrue(TEXT("PlayerB validation is retained"), Result.PlayerBDeckValidation.bIsValid);
	TestTrue(TEXT("Successful initialization has no errors"), Result.ErrorMessages.IsEmpty());
	TestEqual(
		TEXT("PlayerA initial rarity score is stored"),
		Result.InitialMatchState.InitialDeckRarityScores.FindRef(MatchInitializerTests::PlayerAId),
		20);
	TestEqual(
		TEXT("PlayerB initial rarity score is stored"),
		Result.InitialMatchState.InitialDeckRarityScores.FindRef(MatchInitializerTests::PlayerBId),
		40);
	TestEqual(
		TEXT("PlayerA goalkeeper ID is stored"),
		Result.InitialMatchState.InitialGoalkeeperCardIds.FindRef(MatchInitializerTests::PlayerAId),
		FName(TEXT("A_GK")));
	TestEqual(
		TEXT("PlayerB goalkeeper ID is stored"),
		Result.InitialMatchState.InitialGoalkeeperCardIds.FindRef(MatchInitializerTests::PlayerBId),
		FName(TEXT("B_GK")));
	TestEqual(TEXT("Two player states are initialized"), Result.InitialMatchState.PlayerStates.Num(), 2);
	TestEqual(
		TEXT("PlayerA state has a stable ID"),
		Result.InitialMatchState.PlayerStates[0].PlayerId,
		MatchInitializerTests::PlayerAId);
	TestEqual(
		TEXT("PlayerB state has a stable ID"),
		Result.InitialMatchState.PlayerStates[1].PlayerId,
		MatchInitializerTests::PlayerBId);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchInitializerPlayerAInvalidTest,
	"FMCodex.CoreRules.MatchInitializer.PlayerAInvalid",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchInitializerPlayerAInvalidTest::RunTest(const FString& Parameters)
{
	TArray<FPlayerCardData> PlayerADeck =
		MatchInitializerTests::MakeValidDeck(TEXT("A"), ECardRarity::Common);
	PlayerADeck.RemoveAt(0);
	const TArray<FPlayerCardData> PlayerBDeck =
		MatchInitializerTests::MakeValidDeck(TEXT("B"), ECardRarity::Common);

	const FMatchInitializationResult Result =
		FMatchInitializer::InitializeMatch(PlayerADeck, PlayerBDeck);

	TestFalse(TEXT("Invalid PlayerA deck prevents initialization"), Result.bSuccess);
	TestFalse(TEXT("PlayerA validation remains invalid"), Result.PlayerADeckValidation.bIsValid);
	TestTrue(TEXT("PlayerB validation remains valid"), Result.PlayerBDeckValidation.bIsValid);
	TestTrue(
		TEXT("PlayerA invalid card count is retained"),
		Result.PlayerADeckValidation.ErrorCodes.Contains(
			EDeckValidationErrorCode::InvalidCardCount));
	TestTrue(
		TEXT("Readable errors identify PlayerA"),
		MatchInitializerTests::HasMessageForPlayer(Result, TEXT("PlayerA")));
	TestFalse(
		TEXT("No PlayerB error is added for a legal deck"),
		MatchInitializerTests::HasMessageForPlayer(Result, TEXT("PlayerB")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchInitializerPlayerBInvalidTest,
	"FMCodex.CoreRules.MatchInitializer.PlayerBInvalid",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchInitializerPlayerBInvalidTest::RunTest(const FString& Parameters)
{
	const TArray<FPlayerCardData> PlayerADeck =
		MatchInitializerTests::MakeValidDeck(TEXT("A"), ECardRarity::Common);
	TArray<FPlayerCardData> PlayerBDeck =
		MatchInitializerTests::MakeValidDeck(TEXT("B"), ECardRarity::Common);
	PlayerBDeck.RemoveAt(0);

	const FMatchInitializationResult Result =
		FMatchInitializer::InitializeMatch(PlayerADeck, PlayerBDeck);

	TestFalse(TEXT("Invalid PlayerB deck prevents initialization"), Result.bSuccess);
	TestTrue(TEXT("PlayerA validation remains valid"), Result.PlayerADeckValidation.bIsValid);
	TestFalse(TEXT("PlayerB validation remains invalid"), Result.PlayerBDeckValidation.bIsValid);
	TestTrue(
		TEXT("PlayerB invalid card count is retained"),
		Result.PlayerBDeckValidation.ErrorCodes.Contains(
			EDeckValidationErrorCode::InvalidCardCount));
	TestFalse(
		TEXT("No PlayerA error is added for a legal deck"),
		MatchInitializerTests::HasMessageForPlayer(Result, TEXT("PlayerA")));
	TestTrue(
		TEXT("Readable errors identify PlayerB"),
		MatchInitializerTests::HasMessageForPlayer(Result, TEXT("PlayerB")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchInitializerBothInvalidTest,
	"FMCodex.CoreRules.MatchInitializer.BothInvalid",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchInitializerBothInvalidTest::RunTest(const FString& Parameters)
{
	TArray<FPlayerCardData> PlayerADeck =
		MatchInitializerTests::MakeValidDeck(TEXT("A"), ECardRarity::Common);
	PlayerADeck.RemoveAt(0);

	TArray<FPlayerCardData> PlayerBDeck =
		MatchInitializerTests::MakeValidDeck(TEXT("B"), ECardRarity::Common);
	PlayerBDeck.Last().bIsGoalkeeper = false;
	PlayerBDeck.Last().PositionTypes = { EPlayerPositionType::Attack };

	const FMatchInitializationResult Result =
		FMatchInitializer::InitializeMatch(PlayerADeck, PlayerBDeck);

	TestFalse(TEXT("Two invalid decks prevent initialization"), Result.bSuccess);
	TestFalse(TEXT("PlayerA invalid result is retained"), Result.PlayerADeckValidation.bIsValid);
	TestFalse(TEXT("PlayerB invalid result is retained"), Result.PlayerBDeckValidation.bIsValid);
	TestTrue(
		TEXT("PlayerA error is included"),
		MatchInitializerTests::HasMessageForPlayer(Result, TEXT("PlayerA")));
	TestTrue(
		TEXT("PlayerB error is included"),
		MatchInitializerTests::HasMessageForPlayer(Result, TEXT("PlayerB")));
	TestTrue(
		TEXT("PlayerB missing goalkeeper error is retained"),
		Result.PlayerBDeckValidation.ErrorCodes.Contains(
			EDeckValidationErrorCode::NoGoalkeeper));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchInitializerSnapshotTest,
	"FMCodex.CoreRules.MatchInitializer.Snapshot",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchInitializerSnapshotTest::RunTest(const FString& Parameters)
{
	TArray<FPlayerCardData> PlayerADeck =
		MatchInitializerTests::MakeValidDeck(TEXT("A"), ECardRarity::WorldClass);
	TArray<FPlayerCardData> PlayerBDeck =
		MatchInitializerTests::MakeValidDeck(TEXT("B"), ECardRarity::National);

	const FMatchInitializationResult Result =
		FMatchInitializer::InitializeMatch(PlayerADeck, PlayerBDeck);

	PlayerADeck.Empty();
	for (FPlayerCardData& Card : PlayerBDeck)
	{
		Card.Rarity = ECardRarity::Common;
	}
	PlayerBDeck.Last().CardId = FName(TEXT("B_REPLACED_GK"));

	TestTrue(TEXT("Snapshot was initialized successfully"), Result.bSuccess);
	TestEqual(
		TEXT("PlayerA stored score is independent from the source deck"),
		Result.InitialMatchState.InitialDeckRarityScores.FindRef(MatchInitializerTests::PlayerAId),
		140);
	TestEqual(
		TEXT("PlayerB stored score is independent from later rarity changes"),
		Result.InitialMatchState.InitialDeckRarityScores.FindRef(MatchInitializerTests::PlayerBId),
		60);
	TestEqual(
		TEXT("Stored goalkeeper ID is independent from later card changes"),
		Result.InitialMatchState.InitialGoalkeeperCardIds.FindRef(MatchInitializerTests::PlayerBId),
		FName(TEXT("B_GK")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchInitializerDelegatesValidationTest,
	"FMCodex.CoreRules.MatchInitializer.DelegatesValidation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchInitializerDelegatesValidationTest::RunTest(const FString& Parameters)
{
	TArray<FPlayerCardData> PlayerADeck =
		MatchInitializerTests::MakeValidDeck(TEXT("A"), ECardRarity::Continental);
	PlayerADeck.Last().PositionTypes.Add(EPlayerPositionType::Attack);
	const TArray<FPlayerCardData> PlayerBDeck =
		MatchInitializerTests::MakeValidDeck(TEXT("B"), ECardRarity::Common);

	const FDeckValidationResult DirectValidation = FDeckValidator::ValidateDeck(PlayerADeck);
	const FMatchInitializationResult Initialization =
		FMatchInitializer::InitializeMatch(PlayerADeck, PlayerBDeck);

	TestEqual(
		TEXT("Initializer keeps DeckValidator validity"),
		Initialization.PlayerADeckValidation.bIsValid,
		DirectValidation.bIsValid);
	TestTrue(
		TEXT("Initializer keeps DeckValidator error codes"),
		Initialization.PlayerADeckValidation.ErrorCodes == DirectValidation.ErrorCodes);
	TestTrue(
		TEXT("Initializer keeps DeckValidator error messages"),
		Initialization.PlayerADeckValidation.ErrorMessages == DirectValidation.ErrorMessages);
	TestEqual(
		TEXT("Initializer keeps DeckValidator rarity score"),
		Initialization.PlayerADeckValidation.InitialDeckRarityScore,
		DirectValidation.InitialDeckRarityScore);
	TestEqual(
		TEXT("Initializer keeps DeckValidator goalkeeper ID"),
		Initialization.PlayerADeckValidation.GoalkeeperCardId,
		DirectValidation.GoalkeeperCardId);
	return true;
}

#endif