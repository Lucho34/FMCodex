#include "DeckValidator.h"

#if WITH_DEV_AUTOMATION_TESTS

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

	TArray<FPlayerCardData> ThreeGoalkeepers = DeckValidatorTests::MakeValidCommonDeck();
	ThreeGoalkeepers[0] = DeckValidatorTests::MakeGoalkeeperCard(TEXT("GK_02"));
	ThreeGoalkeepers[1] = DeckValidatorTests::MakeGoalkeeperCard(TEXT("GK_03"));
	const FDeckValidationResult ThreeGoalkeepersResult =
		FDeckValidator::ValidateDeck(ThreeGoalkeepers);
	TestFalse(TEXT("Deck with three goalkeepers is illegal"), ThreeGoalkeepersResult.bIsValid);
	TestTrue(
		TEXT("Three goalkeepers reports multiple goalkeeper error"),
		DeckValidatorTests::HasError(
			ThreeGoalkeepersResult,
			EDeckValidationErrorCode::MultipleGoalkeepers));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDeckValidatorGoalkeeperPositionsTest,
	"FMCodex.CoreRules.DeckValidator.GoalkeeperPositions",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDeckValidatorGoalkeeperPositionsTest::RunTest(const FString& Parameters)
{
	struct FMixedGoalkeeperPositionCase
	{
		EPlayerPositionType SecondaryPosition;
		const TCHAR* Label;
	};

	const TArray<FMixedGoalkeeperPositionCase> MixedPositionCases = {
		{ EPlayerPositionType::Attack, TEXT("GK/A") },
		{ EPlayerPositionType::Midfield, TEXT("GK/M") },
		{ EPlayerPositionType::Defense, TEXT("GK/D") }
	};

	for (const FMixedGoalkeeperPositionCase& TestCase : MixedPositionCases)
	{
		TArray<FPlayerCardData> Deck = DeckValidatorTests::MakeValidCommonDeck();
		Deck.Last().PositionTypes = {
			EPlayerPositionType::Goalkeeper,
			TestCase.SecondaryPosition
		};
		const FDeckValidationResult Result = FDeckValidator::ValidateDeck(Deck);
		const FString InvalidMessage = FString::Printf(
			TEXT("%s goalkeeper position is illegal"),
			TestCase.Label);
		const FString ErrorMessage = FString::Printf(
			TEXT("%s reports mixed goalkeeper position error"),
			TestCase.Label);
		TestFalse(InvalidMessage, Result.bIsValid);
		TestTrue(
			ErrorMessage,
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

	TArray<FPlayerCardData> OnlyGkOnNonGoalkeeper = DeckValidatorTests::MakeValidCommonDeck();
	OnlyGkOnNonGoalkeeper[0].PositionTypes = { EPlayerPositionType::Goalkeeper };
	const FDeckValidationResult OnlyGkOnNonGoalkeeperResult =
		FDeckValidator::ValidateDeck(OnlyGkOnNonGoalkeeper);
	TestFalse(
		TEXT("Non-goalkeeper with only GK position is illegal"),
		OnlyGkOnNonGoalkeeperResult.bIsValid);
	TestTrue(
		TEXT("Only GK on non-goalkeeper reports non-goalkeeper GK error"),
		DeckValidatorTests::HasError(
			OnlyGkOnNonGoalkeeperResult,
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

	// DeckValidator only calculates the supplied initial deck. Persistence across
	// match-state zone changes belongs to the future match setup layer.
	return true;
}

#endif