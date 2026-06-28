#include "CardUsageResolver.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

namespace CardUsageResolverTests
{
	const FName CardA(TEXT("CardA"));
	const FName CardB(TEXT("CardB"));
	const FName CardC(TEXT("CardC"));
	const FName CardD(TEXT("CardD"));

	FCardUsageState MakeState(
		const TArray<FName>& AvailableCardIds = { CardA, CardB, CardC },
		const TArray<FName>& UsedCardIds = {})
	{
		FCardUsageState State;
		State.AvailableCardIds = AvailableCardIds;
		State.UsedCardIds = UsedCardIds;
		return State;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCardUsageSuccessfulUseTest,
	"FMCodex.CoreRules.CardUsageResolver.SuccessfulUse",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCardUsageSuccessfulUseTest::RunTest(const FString& Parameters)
{
	const FCardUsageState State =
		CardUsageResolverTests::MakeState();
	const FCardUsageResolveResult Result =
		FCardUsageResolver::UseCard(
			State,
			CardUsageResolverTests::CardB);

	TestTrue(TEXT("Available card can be used"), Result.bSuccess);
	TestEqual(TEXT("Resolved card is reported"), Result.CardId, CardUsageResolverTests::CardB);
	TestFalse(
		TEXT("Used card leaves available cards"),
		Result.UpdatedCardUsageState.AvailableCardIds.Contains(
			CardUsageResolverTests::CardB));
	TestTrue(
		TEXT("Used card enters used cards"),
		Result.UpdatedCardUsageState.UsedCardIds.Contains(
			CardUsageResolverTests::CardB));
	TestEqual(
		TEXT("Successful result has no error"),
		Result.ErrorCode,
		ECardUsageResolveErrorCode::None);
	TestTrue(TEXT("Successful result has no error message"), Result.ErrorMessage.IsEmpty());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCardUsageInputUnchangedTest,
	"FMCodex.CoreRules.CardUsageResolver.InputStateUnchanged",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCardUsageInputUnchangedTest::RunTest(const FString& Parameters)
{
	const FCardUsageState State =
		CardUsageResolverTests::MakeState();
	const FCardUsageResolveResult Result =
		FCardUsageResolver::UseCard(
			State,
			CardUsageResolverTests::CardA);

	TestTrue(TEXT("Card use succeeds"), Result.bSuccess);
	TestTrue(
		TEXT("Input available cards still contain CardA"),
		State.AvailableCardIds.Contains(CardUsageResolverTests::CardA));
	TestTrue(TEXT("Input used cards remain empty"), State.UsedCardIds.IsEmpty());
	TestFalse(
		TEXT("Updated available cards remove CardA"),
		Result.UpdatedCardUsageState.AvailableCardIds.Contains(
			CardUsageResolverTests::CardA));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCardUsageCardNotAvailableTest,
	"FMCodex.CoreRules.CardUsageResolver.CardNotAvailable",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCardUsageCardNotAvailableTest::RunTest(const FString& Parameters)
{
	const FCardUsageState State =
		CardUsageResolverTests::MakeState();
	const FCardUsageResolveResult Result =
		FCardUsageResolver::UseCard(
			State,
			CardUsageResolverTests::CardD);

	TestFalse(TEXT("Unknown card is rejected"), Result.bSuccess);
	TestEqual(
		TEXT("Unknown card reports not available"),
		Result.ErrorCode,
		ECardUsageResolveErrorCode::CardNotAvailable);
	TestTrue(TEXT("Failure has a readable error"), !Result.ErrorMessage.IsEmpty());
	TestTrue(
		TEXT("Failed result preserves available cards"),
		Result.UpdatedCardUsageState.AvailableCardIds
			== State.AvailableCardIds);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCardUsageAlreadyUsedTest,
	"FMCodex.CoreRules.CardUsageResolver.CardAlreadyUsed",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCardUsageAlreadyUsedTest::RunTest(const FString& Parameters)
{
	const FCardUsageState State =
		CardUsageResolverTests::MakeState(
			{ CardUsageResolverTests::CardA, CardUsageResolverTests::CardC },
			{ CardUsageResolverTests::CardB });
	const FCardUsageResolveResult Result =
		FCardUsageResolver::UseCard(
			State,
			CardUsageResolverTests::CardB);

	TestFalse(TEXT("Used card cannot be used again"), Result.bSuccess);
	TestEqual(
		TEXT("Used card reports already used"),
		Result.ErrorCode,
		ECardUsageResolveErrorCode::CardAlreadyUsed);
	TestEqual(TEXT("Used cards remain unchanged"), Result.UpdatedCardUsageState.UsedCardIds.Num(), 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCardUsageDuplicateAvailableTest,
	"FMCodex.CoreRules.CardUsageResolver.DuplicateAvailable",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCardUsageDuplicateAvailableTest::RunTest(const FString& Parameters)
{
	const FCardUsageState State =
		CardUsageResolverTests::MakeState(
			{ CardUsageResolverTests::CardA, CardUsageResolverTests::CardA });
	const FCardUsageResolveResult Result =
		FCardUsageResolver::UseCard(
			State,
			CardUsageResolverTests::CardA);

	TestFalse(TEXT("Duplicate available state is rejected"), Result.bSuccess);
	TestEqual(
		TEXT("Available duplicate reports its error"),
		Result.ErrorCode,
		ECardUsageResolveErrorCode::DuplicateCardInAvailable);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCardUsageDuplicateUsedTest,
	"FMCodex.CoreRules.CardUsageResolver.DuplicateUsed",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCardUsageDuplicateUsedTest::RunTest(const FString& Parameters)
{
	const FCardUsageState State =
		CardUsageResolverTests::MakeState(
			{ CardUsageResolverTests::CardA },
			{ CardUsageResolverTests::CardB, CardUsageResolverTests::CardB });
	const FCardUsageResolveResult Result =
		FCardUsageResolver::UseCard(
			State,
			CardUsageResolverTests::CardA);

	TestFalse(TEXT("Duplicate used state is rejected"), Result.bSuccess);
	TestEqual(
		TEXT("Used duplicate reports its error"),
		Result.ErrorCode,
		ECardUsageResolveErrorCode::DuplicateCardInUsed);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCardUsageCardInBothListsTest,
	"FMCodex.CoreRules.CardUsageResolver.CardInBothLists",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCardUsageCardInBothListsTest::RunTest(const FString& Parameters)
{
	const FCardUsageState State =
		CardUsageResolverTests::MakeState(
			{ CardUsageResolverTests::CardA, CardUsageResolverTests::CardB },
			{ CardUsageResolverTests::CardB });
	const FCardUsageResolveResult Result =
		FCardUsageResolver::UseCard(
			State,
			CardUsageResolverTests::CardA);

	TestFalse(TEXT("Card in both lists invalidates state"), Result.bSuccess);
	TestEqual(
		TEXT("Cross-list duplicate reports its error"),
		Result.ErrorCode,
		ECardUsageResolveErrorCode::CardExistsInBothAvailableAndUsed);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCardUsageInvalidCardIdTest,
	"FMCodex.CoreRules.CardUsageResolver.InvalidCardId",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCardUsageInvalidCardIdTest::RunTest(const FString& Parameters)
{
	const FCardUsageState State =
		CardUsageResolverTests::MakeState();
	const FCardUsageResolveResult Result =
		FCardUsageResolver::UseCard(State, NAME_None);

	TestFalse(TEXT("None CardId is rejected"), Result.bSuccess);
	TestEqual(
		TEXT("None CardId reports invalid ID"),
		Result.ErrorCode,
		ECardUsageResolveErrorCode::InvalidCardId);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCardUsageInvalidStateCardIdTest,
	"FMCodex.CoreRules.CardUsageResolver.InvalidStateCardId",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCardUsageInvalidStateCardIdTest::RunTest(const FString& Parameters)
{
	const FCardUsageState State =
		CardUsageResolverTests::MakeState(
			{ CardUsageResolverTests::CardA, NAME_None });
	const FCardUsageResolveResult Result =
		FCardUsageResolver::UseCard(
			State,
			CardUsageResolverTests::CardA);

	TestFalse(TEXT("State containing None is rejected"), Result.bSuccess);
	TestEqual(
		TEXT("Invalid list entry reports invalid state"),
		Result.ErrorCode,
		ECardUsageResolveErrorCode::InvalidCardUsageState);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCardUsagePreservesOrderTest,
	"FMCodex.CoreRules.CardUsageResolver.PreservesOtherCardsAndOrder",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCardUsagePreservesOrderTest::RunTest(const FString& Parameters)
{
	const FCardUsageState State =
		CardUsageResolverTests::MakeState(
			{
				CardUsageResolverTests::CardA,
				CardUsageResolverTests::CardB,
				CardUsageResolverTests::CardC,
				CardUsageResolverTests::CardD
			},
			{ FName(TEXT("PreviouslyUsed")) });
	const FCardUsageResolveResult Result =
		FCardUsageResolver::UseCard(
			State,
			CardUsageResolverTests::CardB);

	const TArray<FName> ExpectedAvailable =
		{
			CardUsageResolverTests::CardA,
			CardUsageResolverTests::CardC,
			CardUsageResolverTests::CardD
		};
	const TArray<FName> ExpectedUsed =
		{
			FName(TEXT("PreviouslyUsed")),
			CardUsageResolverTests::CardB
		};

	TestTrue(TEXT("Card use succeeds"), Result.bSuccess);
	TestTrue(
		TEXT("Other available cards retain order"),
		Result.UpdatedCardUsageState.AvailableCardIds
			== ExpectedAvailable);
	TestTrue(
		TEXT("Existing used cards retain order and used card appends"),
		Result.UpdatedCardUsageState.UsedCardIds
			== ExpectedUsed);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCardUsageConsecutiveUsesTest,
	"FMCodex.CoreRules.CardUsageResolver.ConsecutiveUses",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCardUsageConsecutiveUsesTest::RunTest(const FString& Parameters)
{
	const FCardUsageState State =
		CardUsageResolverTests::MakeState();
	const FCardUsageResolveResult FirstResult =
		FCardUsageResolver::UseCard(
			State,
			CardUsageResolverTests::CardA);
	const FCardUsageResolveResult SecondResult =
		FCardUsageResolver::UseCard(
			FirstResult.UpdatedCardUsageState,
			CardUsageResolverTests::CardC);

	TestTrue(TEXT("First card use succeeds"), FirstResult.bSuccess);
	TestTrue(TEXT("Second card use succeeds"), SecondResult.bSuccess);
	TestEqual(
		TEXT("Only CardB remains available"),
		SecondResult.UpdatedCardUsageState.AvailableCardIds.Num(),
		1);
	TestEqual(
		TEXT("CardB remains available"),
		SecondResult.UpdatedCardUsageState.AvailableCardIds[0],
		CardUsageResolverTests::CardB);
	TestEqual(
		TEXT("Two cards are used"),
		SecondResult.UpdatedCardUsageState.UsedCardIds.Num(),
		2);
	TestEqual(
		TEXT("First used card order is retained"),
		SecondResult.UpdatedCardUsageState.UsedCardIds[0],
		CardUsageResolverTests::CardA);
	TestEqual(
		TEXT("Second used card appends"),
		SecondResult.UpdatedCardUsageState.UsedCardIds[1],
		CardUsageResolverTests::CardC);
	return true;
}

#endif
