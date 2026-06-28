#include "MatchOpeningResolver.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

namespace MatchOpeningResolverTests
{
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

	FMatchOpeningResolveInput MakeInput(
		const ECardRarity PlayerARarity,
		const ECardRarity PlayerBRarity)
	{
		FMatchOpeningResolveInput Input;
		Input.PlayerADeck = MakeValidDeck(TEXT("A"), PlayerARarity);
		Input.PlayerBDeck = MakeValidDeck(TEXT("B"), PlayerBRarity);
		Input.PlayerAAttackCountD6Roll = 1;
		Input.PlayerBAttackCountD6Roll = 1;
		return Input;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchOpeningSuccessfulHigherAttackCountTest,
	"FMCodex.CoreRules.MatchOpeningResolver.SuccessfulHigherAttackCount",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchOpeningSuccessfulHigherAttackCountTest::RunTest(const FString& Parameters)
{
	FMatchOpeningResolveInput Input =
		MatchOpeningResolverTests::MakeInput(ECardRarity::WorldClass, ECardRarity::Common);
	Input.PlayerAAttackCountD6Roll = 1;
	Input.PlayerBAttackCountD6Roll = 6;

	const FMatchOpeningResolveResult Result =
		FMatchOpeningResolver::ResolveMatchOpening(Input);

	TestTrue(TEXT("Legal opening resolves successfully"), Result.bSuccess);
	TestTrue(TEXT("Match initialization child result is retained"), Result.MatchInitializationResult.bSuccess);
	TestTrue(TEXT("Attack count child result is retained"), Result.AttackCountResult.bSuccess);
	TestTrue(TEXT("Turn order child result is retained"), Result.InitialTurnOrderResult.bSuccess);
	TestTrue(TEXT("Successful opening has no stage errors"), Result.ErrorCodes.IsEmpty());
	TestEqual(
		TEXT("PlayerA initial rarity score remains available"),
		Result.MatchInitializationResult.PlayerADeckValidation.InitialDeckRarityScore,
		140);
	TestEqual(
		TEXT("PlayerB initial rarity score remains available"),
		Result.MatchInitializationResult.PlayerBDeckValidation.InitialDeckRarityScore,
		20);
	TestEqual(
		TEXT("PlayerA goalkeeper remains available"),
		Result.MatchInitializationResult.PlayerADeckValidation.GoalkeeperCardId,
		FString(TEXT("A_GK")));
	TestEqual(
		TEXT("PlayerB goalkeeper remains available"),
		Result.MatchInitializationResult.PlayerBDeckValidation.GoalkeeperCardId,
		FString(TEXT("B_GK")));
	TestEqual(TEXT("PlayerA total attack count is five"), Result.AttackCountResult.PlayerAAttackCount, 5);
	TestEqual(TEXT("PlayerB total attack count is six"), Result.AttackCountResult.PlayerBAttackCount, 6);
	TestEqual(
		TEXT("PlayerB attacks first with the higher attack count"),
		Result.InitialTurnOrderResult.FirstPlayer,
		EInitialTurnOrderPlayer::PlayerB);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchOpeningInvalidDecksTest,
	"FMCodex.CoreRules.MatchOpeningResolver.InvalidDecks",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchOpeningInvalidDecksTest::RunTest(const FString& Parameters)
{
	FMatchOpeningResolveInput PlayerAInvalid =
		MatchOpeningResolverTests::MakeInput(ECardRarity::Common, ECardRarity::Common);
	PlayerAInvalid.PlayerADeck.RemoveAt(0);
	const FMatchOpeningResolveResult PlayerAResult =
		FMatchOpeningResolver::ResolveMatchOpening(PlayerAInvalid);
	TestFalse(TEXT("Invalid PlayerA deck fails opening"), PlayerAResult.bSuccess);
	TestFalse(
		TEXT("PlayerA deck validation error is retained"),
		PlayerAResult.MatchInitializationResult.PlayerADeckValidation.bIsValid);
	TestTrue(
		TEXT("PlayerA failure reports initialization stage"),
		PlayerAResult.ErrorCodes.Contains(
			EMatchOpeningResolveErrorCode::MatchInitializationFailed));

	FMatchOpeningResolveInput PlayerBInvalid =
		MatchOpeningResolverTests::MakeInput(ECardRarity::Common, ECardRarity::Common);
	PlayerBInvalid.PlayerBDeck.RemoveAt(0);
	const FMatchOpeningResolveResult PlayerBResult =
		FMatchOpeningResolver::ResolveMatchOpening(PlayerBInvalid);
	TestFalse(TEXT("Invalid PlayerB deck fails opening"), PlayerBResult.bSuccess);
	TestFalse(
		TEXT("PlayerB deck validation error is retained"),
		PlayerBResult.MatchInitializationResult.PlayerBDeckValidation.bIsValid);

	FMatchOpeningResolveInput BothInvalid =
		MatchOpeningResolverTests::MakeInput(ECardRarity::Common, ECardRarity::Common);
	BothInvalid.PlayerADeck.RemoveAt(0);
	BothInvalid.PlayerBDeck.RemoveAt(0);
	const FMatchOpeningResolveResult BothResult =
		FMatchOpeningResolver::ResolveMatchOpening(BothInvalid);
	TestFalse(TEXT("Two invalid decks fail opening"), BothResult.bSuccess);
	TestFalse(
		TEXT("Both invalid result retains PlayerA validation"),
		BothResult.MatchInitializationResult.PlayerADeckValidation.bIsValid);
	TestFalse(
		TEXT("Both invalid result retains PlayerB validation"),
		BothResult.MatchInitializationResult.PlayerBDeckValidation.bIsValid);
	TestTrue(
		TEXT("Both invalid result retains readable errors"),
		BothResult.ErrorMessages.Num() >= 2);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchOpeningInvalidAttackRollTest,
	"FMCodex.CoreRules.MatchOpeningResolver.InvalidAttackRoll",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchOpeningInvalidAttackRollTest::RunTest(const FString& Parameters)
{
	FMatchOpeningResolveInput Input =
		MatchOpeningResolverTests::MakeInput(ECardRarity::Common, ECardRarity::Common);
	Input.PlayerAAttackCountD6Roll = 0;
	Input.PlayerBAttackCountD6Roll = 7;

	const FMatchOpeningResolveResult Result =
		FMatchOpeningResolver::ResolveMatchOpening(Input);

	TestFalse(TEXT("Invalid attack count rolls fail opening"), Result.bSuccess);
	TestTrue(TEXT("Initialization succeeds before attack count failure"), Result.MatchInitializationResult.bSuccess);
	TestFalse(TEXT("Attack count child result retains failure"), Result.AttackCountResult.bSuccess);
	TestTrue(
		TEXT("PlayerA attack roll error is retained"),
		Result.AttackCountResult.ErrorCodes.Contains(
			EAttackCountResolveErrorCode::PlayerAD6RollOutOfRange));
	TestTrue(
		TEXT("PlayerB attack roll error is retained"),
		Result.AttackCountResult.ErrorCodes.Contains(
			EAttackCountResolveErrorCode::PlayerBD6RollOutOfRange));
	TestTrue(
		TEXT("Opening reports attack count failure stage"),
		Result.ErrorCodes.Contains(
			EMatchOpeningResolveErrorCode::AttackCountResolutionFailed));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchOpeningLowerRarityFirstTest,
	"FMCodex.CoreRules.MatchOpeningResolver.LowerRarityFirst",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchOpeningLowerRarityFirstTest::RunTest(const FString& Parameters)
{
	FMatchOpeningResolveInput Input =
		MatchOpeningResolverTests::MakeInput(ECardRarity::WorldClass, ECardRarity::Common);
	Input.PlayerAAttackCountD6Roll = 1;
	Input.PlayerBAttackCountD6Roll = 3;

	const FMatchOpeningResolveResult Result =
		FMatchOpeningResolver::ResolveMatchOpening(Input);

	TestTrue(TEXT("Equal attack counts with different rarity resolve"), Result.bSuccess);
	TestEqual(TEXT("PlayerA total attack count is five"), Result.AttackCountResult.PlayerAAttackCount, 5);
	TestEqual(TEXT("PlayerB total attack count is five"), Result.AttackCountResult.PlayerBAttackCount, 5);
	TestEqual(
		TEXT("Lower rarity PlayerB attacks first"),
		Result.InitialTurnOrderResult.FirstPlayer,
		EInitialTurnOrderPlayer::PlayerB);
	TestEqual(
		TEXT("Opening retains lower rarity reason"),
		Result.InitialTurnOrderResult.Reason,
		EInitialTurnOrderReason::LowerInitialDeckRarityScore);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchOpeningLowerTieBreakerFirstTest,
	"FMCodex.CoreRules.MatchOpeningResolver.LowerTieBreakerFirst",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchOpeningLowerTieBreakerFirstTest::RunTest(const FString& Parameters)
{
	FMatchOpeningResolveInput Input =
		MatchOpeningResolverTests::MakeInput(ECardRarity::Common, ECardRarity::Common);
	Input.PlayerATieBreakerRoll = 2;
	Input.PlayerBTieBreakerRoll = 6;

	const FMatchOpeningResolveResult Result =
		FMatchOpeningResolver::ResolveMatchOpening(Input);

	TestTrue(TEXT("Valid tie breaker resolves opening"), Result.bSuccess);
	TestEqual(
		TEXT("Lower tie breaker PlayerA attacks first"),
		Result.InitialTurnOrderResult.FirstPlayer,
		EInitialTurnOrderPlayer::PlayerA);
	TestEqual(
		TEXT("Opening retains lower tie breaker reason"),
		Result.InitialTurnOrderResult.Reason,
		EInitialTurnOrderReason::LowerTieBreakerRoll);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchOpeningTieBreakerRerollTest,
	"FMCodex.CoreRules.MatchOpeningResolver.TieBreakerReroll",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchOpeningTieBreakerRerollTest::RunTest(const FString& Parameters)
{
	FMatchOpeningResolveInput Input =
		MatchOpeningResolverTests::MakeInput(ECardRarity::Common, ECardRarity::Common);
	Input.PlayerATieBreakerRoll = 4;
	Input.PlayerBTieBreakerRoll = 4;

	const FMatchOpeningResolveResult Result =
		FMatchOpeningResolver::ResolveMatchOpening(Input);

	TestFalse(TEXT("Tied tie breaker does not finish opening"), Result.bSuccess);
	TestTrue(TEXT("Initialization remains successful"), Result.MatchInitializationResult.bSuccess);
	TestTrue(TEXT("Attack count remains successful"), Result.AttackCountResult.bSuccess);
	TestFalse(TEXT("Turn order child result retains failure"), Result.InitialTurnOrderResult.bSuccess);
	TestTrue(
		TEXT("Opening retains external reroll requirement"),
		Result.InitialTurnOrderResult.bRequiresTieBreakerReroll);
	TestTrue(
		TEXT("Opening reports turn order failure stage"),
		Result.ErrorCodes.Contains(
			EMatchOpeningResolveErrorCode::InitialTurnOrderResolutionFailed));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchOpeningDeterministicCompositionTest,
	"FMCodex.CoreRules.MatchOpeningResolver.DeterministicComposition",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchOpeningDeterministicCompositionTest::RunTest(const FString& Parameters)
{
	FMatchOpeningResolveInput Input =
		MatchOpeningResolverTests::MakeInput(ECardRarity::National, ECardRarity::National);
	Input.PlayerAAttackCountD6Roll = 4;
	Input.PlayerBAttackCountD6Roll = 4;
	Input.PlayerATieBreakerRoll = 5;
	Input.PlayerBTieBreakerRoll = 1;

	const FMatchOpeningResolveResult First =
		FMatchOpeningResolver::ResolveMatchOpening(Input);
	const FMatchOpeningResolveResult Second =
		FMatchOpeningResolver::ResolveMatchOpening(Input);

	TestTrue(TEXT("Repeated opening input succeeds"), First.bSuccess && Second.bSuccess);
	TestEqual(
		TEXT("Repeated input keeps PlayerA attack count"),
		First.AttackCountResult.PlayerAAttackCount,
		Second.AttackCountResult.PlayerAAttackCount);
	TestEqual(
		TEXT("Repeated input keeps PlayerB attack count"),
		First.AttackCountResult.PlayerBAttackCount,
		Second.AttackCountResult.PlayerBAttackCount);
	TestEqual(
		TEXT("Repeated input keeps first player"),
		First.InitialTurnOrderResult.FirstPlayer,
		Second.InitialTurnOrderResult.FirstPlayer);
	return true;
}

#endif
