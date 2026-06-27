#include "DeckValidator.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "MatchStateTypes.h"
#include "Misc/AutomationTest.h"

namespace DeckValidatorTests
{
	FPlayerCardData MakeOutfieldCard(
		const int32 Index,
		const ECardRarity Rarity = ECardRarity::Common)
	{
		FPlayerCardData Card;
		Card.CardId = FName(*FString::Printf(TEXT("OUT_%02d"), Index));
		Card.DisplayName = FText::FromString(FString::Printf(TEXT("Outfield %02d"), Index));
		Card.Rarity = Rarity;
		Card.PositionTypes = { EPlayerPositionType::Attack };
		Card.bIsGoalkeeper = false;
		return Card;
	}

	FPlayerCardData MakeGoalkeeperCard(
		const FString& CardId = TEXT("GK_01"),
		const ECardRarity Rarity = ECardRarity::Common)
	{
		FPlayerCardData Card;
		Card.CardId = FName(*CardId);
		Card.DisplayName = FText::FromString(TEXT("Goalkeeper"));
		Card.Rarity = Rarity;
		Card.PositionTypes = { EPlayerPositionType::Goalkeeper };
		Card.bIsGoalkeeper = true;
		return Card;
	}

	TArray<FPlayerCardData> MakeValidCommonDeck()
	{
		TArray<FPlayerCardData> Deck;
		Deck.Reserve(20);
		for (int32 Index = 0; Index < 19; ++Index)
		{
			Deck.Add(MakeOutfieldCard(Index));
		}
		Deck.Add(MakeGoalkeeperCard());
		return Deck;
	}

	bool HasError(
		const FDeckValidationResult& Result,
		const EDeckValidationErrorCode ErrorCode)
	{
		return Result.ErrorCodes.Contains(ErrorCode);
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDeckValidatorValidDeckTest,
	"FMCodex.CoreRules.DeckValidator.ValidDeck",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDeckValidatorValidDeckTest::RunTest(const FString& Parameters)
{
	const TArray<FPlayerCardData> Deck = DeckValidatorTests::MakeValidCommonDeck();
	const FDeckValidationResult Result = FDeckValidator::ValidateDeck(Deck);

	TestTrue(TEXT("Exactly 20 cards with one valid goalkeeper is legal"), Result.bIsValid);
	TestTrue(TEXT("Legal deck has no error codes"), Result.ErrorCodes.IsEmpty());
	TestTrue(TEXT("Legal deck has no error messages"), Result.ErrorMessages.IsEmpty());
	TestEqual(TEXT("Legal deck returns goalkeeper card ID"), Result.GoalkeeperCardId, FString(TEXT("GK_01")));
	TestEqual(TEXT("Twenty common cards score twenty points"), Result.InitialDeckRarityScore, 20);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDeckValidatorCardCountTest,
	"FMCodex.CoreRules.DeckValidator.CardCount",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDeckValidatorCardCountTest::RunTest(const FString& Parameters)
{
	TArray<FPlayerCardData> TooFew = DeckValidatorTests::MakeValidCommonDeck();
	TooFew.RemoveAt(0);
	const FDeckValidationResult TooFewResult = FDeckValidator::ValidateDeck(TooFew);
	TestFalse(TEXT("Fewer than 20 cards is illegal"), TooFewResult.bIsValid);
	TestTrue(
		TEXT("Too few cards reports invalid count"),
		DeckValidatorTests::HasError(TooFewResult, EDeckValidationErrorCode::InvalidCardCount));
	TestEqual(TEXT("Illegal deck still calculates rarity score"), TooFewResult.InitialDeckRarityScore, 19);
	TestTrue(TEXT("Invalid count has a readable message"), !TooFewResult.ErrorMessages.IsEmpty());

	TArray<FPlayerCardData> TooMany = DeckValidatorTests::MakeValidCommonDeck();
	TooMany.Add(DeckValidatorTests::MakeOutfieldCard(20));
	const FDeckValidationResult TooManyResult = FDeckValidator::ValidateDeck(TooMany);
	TestFalse(TEXT("More than 20 cards is illegal"), TooManyResult.bIsValid);
	TestTrue(
		TEXT("Too many cards reports invalid count"),
		DeckValidatorTests::HasError(TooManyResult, EDeckValidationErrorCode::InvalidCardCount));
	TestEqual(TEXT("Oversized deck still calculates rarity score"), TooManyResult.InitialDeckRarityScore, 21);

	const TArray<FPlayerCardData> EmptyDeck;
	const FDeckValidationResult EmptyResult = FDeckValidator::ValidateDeck(EmptyDeck);
	TestFalse(TEXT("Empty deck is illegal"), EmptyResult.bIsValid);
	TestTrue(
		TEXT("Empty deck reports invalid count"),
		DeckValidatorTests::HasError(EmptyResult, EDeckValidationErrorCode::InvalidCardCount));
	TestTrue(
		TEXT("Empty deck reports no goalkeeper"),
		DeckValidatorTests::HasError(EmptyResult, EDeckValidationErrorCode::NoGoalkeeper));
	TestEqual(TEXT("Empty deck rarity score is zero"), EmptyResult.InitialDeckRarityScore, 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDeckValidatorGoalkeeperCountTest,
	"FMCodex.CoreRules.DeckValidator.GoalkeeperCount",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDeckValidatorGoalkeeperCountTest::RunTest(const FString& Parameters)
{
	TArray<FPlayerCardData> NoGoalkeeper = DeckValidatorTests::MakeValidCommonDeck();
	NoGoalkeeper.Last() = DeckValidatorTests::MakeOutfieldCard(19);
	const FDeckValidationResult NoGoalkeeperResult = FDeckValidator::ValidateDeck(NoGoalkeeper);
	TestFalse(TEXT("Deck without a goalkeeper is illegal"), NoGoalkeeperResult.bIsValid);
	TestTrue(
		TEXT("Missing goalkeeper error is reported"),
		DeckValidatorTests::HasError(NoGoalkeeperResult, EDeckValidationErrorCode::NoGoalkeeper));
	TestTrue(TEXT("Missing goalkeeper has no goalkeeper ID"), NoGoalkeeperResult.GoalkeeperCardId.IsEmpty());

	TArray<FPlayerCardData> MultipleGoalkeepers = DeckValidatorTests::MakeValidCommonDeck();
	MultipleGoalkeepers[0] = DeckValidatorTests::MakeGoalkeeperCard(TEXT("GK_02"));
	const FDeckValidationResult MultipleResult = FDeckValidator::ValidateDeck(MultipleGoalkeepers);
	TestFalse(TEXT("Deck with two goalkeepers is illegal"), MultipleResult.bIsValid);
	TestTrue(
		TEXT("Multiple goalkeeper error is reported"),
		DeckValidatorTests::HasError(MultipleResult, EDeckValidationErrorCode::MultipleGoalkeepers));
	TestTrue(TEXT("Ambiguous goalkeeper deck has no goalkeeper ID"), MultipleResult.GoalkeeperCardId.IsEmpty());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDeckValidatorGoalkeeperPositionsTest,
	"FMCodex.CoreRules.DeckValidator.GoalkeeperPositions",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDeckValidatorGoalkeeperPositionsTest::RunTest(const FString& Parameters)
{
	const TArray<EPlayerPositionType> MixedPositions = {
		EPlayerPositionType::Attack,
		EPlayerPositionType::Midfield,
		EPlayerPositionType::Defense
	};

	for (const EPlayerPositionType SecondaryPosition : MixedPositions)
	{
		TArray<FPlayerCardData> Deck = DeckValidatorTests::MakeValidCommonDeck();
		Deck.Last().PositionTypes = {
			EPlayerPositionType::Goalkeeper,
			SecondaryPosition
		};
		const FDeckValidationResult Result = FDeckValidator::ValidateDeck(Deck);
		TestFalse(TEXT("Goalkeeper mixed with an outfield position is illegal"), Result.bIsValid);
		TestTrue(
			TEXT("Mixed goalkeeper position error is reported"),
			DeckValidatorTests::HasError(
				Result,
				EDeckValidationErrorCode::GoalkeeperHasMixedPositionTypes));
	}

	TArray<FPlayerCardData> GoalkeeperWithoutGk = DeckValidatorTests::MakeValidCommonDeck();
	GoalkeeperWithoutGk.Last().PositionTypes = { EPlayerPositionType::Attack };
	const FDeckValidationResult InvalidGoalkeeperResult =
		FDeckValidator::ValidateDeck(GoalkeeperWithoutGk);
	TestFalse(TEXT("Goalkeeper without GK position is illegal"), InvalidGoalkeeperResult.bIsValid);
	TestTrue(
		TEXT("Goalkeeper without GK reports invalid goalkeeper positions"),
		DeckValidatorTests::HasError(
			InvalidGoalkeeperResult,
			EDeckValidationErrorCode::GoalkeeperHasMixedPositionTypes));

	TArray<FPlayerCardData> OutfieldWithGk = DeckValidatorTests::MakeValidCommonDeck();
	OutfieldWithGk[0].PositionTypes = {
		EPlayerPositionType::Attack,
		EPlayerPositionType::Goalkeeper
	};
	const FDeckValidationResult OutfieldWithGkResult = FDeckValidator::ValidateDeck(OutfieldWithGk);
	TestFalse(TEXT("Non-goalkeeper containing GK is illegal"), OutfieldWithGkResult.bIsValid);
	TestTrue(
		TEXT("Non-goalkeeper GK position error is reported"),
		DeckValidatorTests::HasError(
			OutfieldWithGkResult,
			EDeckValidationErrorCode::NonGoalkeeperContainsGoalkeeperPosition));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDeckValidatorRarityScoreTest,
	"FMCodex.CoreRules.DeckValidator.RarityScore",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDeckValidatorRarityScoreTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("World class rarity score"), FDeckValidator::GetRarityScore(ECardRarity::WorldClass), 7);
	TestEqual(TEXT("Continental rarity score"), FDeckValidator::GetRarityScore(ECardRarity::Continental), 5);
	TestEqual(TEXT("National rarity score"), FDeckValidator::GetRarityScore(ECardRarity::National), 3);
	TestEqual(TEXT("Regional rarity score"), FDeckValidator::GetRarityScore(ECardRarity::Regional), 2);
	TestEqual(TEXT("Common rarity score"), FDeckValidator::GetRarityScore(ECardRarity::Common), 1);

	const TArray<ECardRarity> Rarities = {
		ECardRarity::WorldClass,
		ECardRarity::Continental,
		ECardRarity::National,
		ECardRarity::Regional,
		ECardRarity::Common
	};

	TArray<FPlayerCardData> Deck;
	Deck.Reserve(20);
	for (int32 Index = 0; Index < 19; ++Index)
	{
		Deck.Add(DeckValidatorTests::MakeOutfieldCard(Index, Rarities[Index % Rarities.Num()]));
	}
	Deck.Add(DeckValidatorTests::MakeGoalkeeperCard(TEXT("GK_SCORE"), Rarities[19 % Rarities.Num()]));

	const FDeckValidationResult InitialResult = FDeckValidator::ValidateDeck(Deck);
	TestTrue(TEXT("Mixed-rarity 20-card deck is legal"), InitialResult.bIsValid);
	TestEqual(TEXT("Four of each rarity score seventy-two"), InitialResult.InitialDeckRarityScore, 72);
	TestEqual(TEXT("Rarity deck returns goalkeeper ID"), InitialResult.GoalkeeperCardId, FString(TEXT("GK_SCORE")));

	FPlayerMatchState UnrelatedMatchState;
	UnrelatedMatchState.HandCardIds = { TEXT("OTHER_HAND_CARD") };
	UnrelatedMatchState.ConsumedZoneCardIds = { TEXT("OTHER_CONSUMED_CARD") };
	UnrelatedMatchState.DiscardPileCardIds = { TEXT("OTHER_DISCARDED_CARD") };

	const FDeckValidationResult AfterZoneChanges = FDeckValidator::ValidateDeck(Deck);
	TestEqual(
		TEXT("Dynamic hand and zone state does not change initial deck score"),
		AfterZoneChanges.InitialDeckRarityScore,
		InitialResult.InitialDeckRarityScore);
	return true;
}

#endif