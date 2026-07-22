#include "MatchPlayOpeningInitializer.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

namespace MatchPlayOpeningInitializerTests
{
	const FName PlayerACard1(TEXT("PlayerACard1"));
	const FName PlayerACard2(TEXT("PlayerACard2"));
	const FName PlayerBCard1(TEXT("PlayerBCard1"));
	const FName PlayerBCard2(TEXT("PlayerBCard2"));

	FPlayerCardData MakeDeckCard(
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
			Deck.Add(MakeDeckCard(
				FString::Printf(TEXT("%s_OUT_%02d"), *Prefix, Index),
				Rarity,
				false));
		}
		Deck.Add(MakeDeckCard(
			FString::Printf(TEXT("%s_GK"), *Prefix),
			Rarity,
			true));
		return Deck;
	}

	FMatchPlayDeploymentSlotCatalog MakeDeploymentSlotCatalog(
		const TCHAR* FirstSlotId = TEXT("SharedSlotA"))
	{
		FMatchPlayDeploymentSlotDefinition NearPlayerASlot;
		NearPlayerASlot.SlotId = FName(FirstSlotId);
		NearPlayerASlot.NeutralSide =
			EMatchPlayNeutralSlotSide::NearPlayerA;

		FMatchPlayDeploymentSlotDefinition NearPlayerBSlot;
		NearPlayerBSlot.SlotId = TEXT("SharedSlotB");
		NearPlayerBSlot.NeutralSide =
			EMatchPlayNeutralSlotSide::NearPlayerB;

		FMatchPlayDeploymentSlotCatalog Catalog;
		Catalog.Slots = { NearPlayerASlot, NearPlayerBSlot };
		return Catalog;
	}

	FMatchPlayOpeningInitializeInput MakeInput(
		const ECardRarity PlayerARarity = ECardRarity::Common,
		const ECardRarity PlayerBRarity = ECardRarity::Common)
	{
		FMatchPlayOpeningInitializeInput Input;
		Input.OpeningInput.PlayerADeck =
			MakeValidDeck(TEXT("A"), PlayerARarity);
		Input.OpeningInput.PlayerBDeck =
			MakeValidDeck(TEXT("B"), PlayerBRarity);
		Input.OpeningInput.PlayerAAttackCountD6Roll = 1;
		Input.OpeningInput.PlayerBAttackCountD6Roll = 1;
		Input.OpeningInput.PlayerATieBreakerRoll = 2;
		Input.OpeningInput.PlayerBTieBreakerRoll = 6;
		Input.DeploymentSlotCatalog = MakeDeploymentSlotCatalog();
		return Input;
	}

	TArray<FName> GetCardIds(const TArray<FPlayerCardData>& Deck)
	{
		TArray<FName> CardIds;
		CardIds.Reserve(Deck.Num());
		for (const FPlayerCardData& Card : Deck)
		{
			CardIds.Add(Card.CardId);
		}
		return CardIds;
	}

	bool IsMatchPlayStateEmpty(const FMatchPlayState& MatchPlayState)
	{
		return !MatchPlayState.RuntimeState.bIsInitialized
			&& MatchPlayState.CardUsageState.PlayerACardUsageState
				.AvailableCardIds.IsEmpty()
			&& MatchPlayState.CardUsageState.PlayerACardUsageState
				.UsedCardIds.IsEmpty()
			&& MatchPlayState.CardUsageState.PlayerBCardUsageState
				.AvailableCardIds.IsEmpty()
			&& MatchPlayState.CardUsageState.PlayerBCardUsageState
				.UsedCardIds.IsEmpty()
			&& MatchPlayState.DeploymentSlotCatalog.Slots.IsEmpty()
			&& MatchPlayState.CardSnapshotAuthority
				.PlayerACardSnapshots.Cards.IsEmpty()
			&& MatchPlayState.CardSnapshotAuthority
				.PlayerBCardSnapshots.Cards.IsEmpty();
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayOpeningInitializerSuccessTest,
	"FMCodex.CoreRules.MatchPlayOpeningInitializer.Success",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayOpeningInitializerSuccessTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayOpeningInitializeResult Result =
		FMatchPlayOpeningInitializer::InitializeMatchPlayOpening(
			MatchPlayOpeningInitializerTests::MakeInput());

	TestTrue(TEXT("Complete opening input succeeds"), Result.bSuccess);
	TestEqual(
		TEXT("Successful result has no wrapper error"),
		Result.ErrorCode,
		EMatchPlayOpeningInitializeErrorCode::None);
	TestTrue(
		TEXT("Successful result contains initialized runtime state"),
		Result.MatchPlayState.RuntimeState.bIsInitialized);
	TestEqual(
		TEXT("Successful result has no catalog error"),
		Result.UnderlyingDeploymentSlotCatalogValidationErrorCode,
		EMatchPlayDeploymentSlotCatalogValidationErrorCode::None);
	TestEqual(
		TEXT("Successful result has no snapshot authority build error"),
		Result.UnderlyingCardSnapshotAuthorityBuildErrorCode,
		EMatchPlayCardSnapshotAuthorityBuildErrorCode::None);
	TestEqual(
		TEXT("Successful result has no snapshot authority failing side"),
		Result.UnderlyingCardSnapshotAuthorityFailingPlayerSide,
		EInitialTurnOrderPlayer::None);
	TestEqual(
		TEXT("Successful result has no deck validation error"),
		Result.UnderlyingDeckValidationErrorCode,
		EDeckValidationErrorCode::None);
	TestEqual(
		TEXT("Successful result has no snapshot validation error"),
		Result.UnderlyingPlayerCardRuleSnapshotValidationErrorCode,
		EPlayerCardRuleSnapshotValidationErrorCode::None);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayOpeningInitializerRuntimeTest,
	"FMCodex.CoreRules.MatchPlayOpeningInitializer.SuccessKeepsOpeningRuntimeState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayOpeningInitializerRuntimeTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayOpeningInitializeInput Input =
		MatchPlayOpeningInitializerTests::MakeInput();
	const FMatchOpeningResolveResult ExpectedOpening =
		FMatchOpeningResolver::ResolveMatchOpening(Input.OpeningInput);
	const FMatchRuntimeInitializeResult ExpectedRuntime =
		FMatchRuntimeInitializer::InitializeRuntimeState(ExpectedOpening);
	const FMatchPlayOpeningInitializeResult Result =
		FMatchPlayOpeningInitializer::InitializeMatchPlayOpening(Input);

	TestTrue(TEXT("Expected runtime initialization succeeds"), ExpectedRuntime.bSuccess);
	TestEqual(
		TEXT("PlayerA total attacks match runtime initializer"),
		Result.MatchPlayState.RuntimeState.PlayerAState.TotalAttackCount,
		ExpectedRuntime.RuntimeState.PlayerAState.TotalAttackCount);
	TestEqual(
		TEXT("PlayerB total attacks match runtime initializer"),
		Result.MatchPlayState.RuntimeState.PlayerBState.TotalAttackCount,
		ExpectedRuntime.RuntimeState.PlayerBState.TotalAttackCount);
	TestEqual(
		TEXT("First player matches runtime initializer"),
		Result.MatchPlayState.RuntimeState.FirstPlayer,
		ExpectedRuntime.RuntimeState.FirstPlayer);
	TestEqual(
		TEXT("Current attacker matches runtime initializer"),
		Result.MatchPlayState.RuntimeState.CurrentAttackingPlayer,
		ExpectedRuntime.RuntimeState.CurrentAttackingPlayer);
	TestEqual(
		TEXT("PlayerA goalkeeper matches runtime initializer"),
		Result.MatchPlayState.RuntimeState.PlayerAState.GoalkeeperCardId,
		ExpectedRuntime.RuntimeState.PlayerAState.GoalkeeperCardId);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayOpeningInitializerPlayerACardsTest,
	"FMCodex.CoreRules.MatchPlayOpeningInitializer.SuccessInitializesPlayerACards",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayOpeningInitializerPlayerACardsTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayOpeningInitializeInput Input =
		MatchPlayOpeningInitializerTests::MakeInput();
	const FMatchPlayOpeningInitializeResult Result =
		FMatchPlayOpeningInitializer::InitializeMatchPlayOpening(Input);

	TestTrue(
		TEXT("PlayerA available cards preserve input"),
		Result.MatchPlayState.CardUsageState.PlayerACardUsageState
			.AvailableCardIds
			== MatchPlayOpeningInitializerTests::GetCardIds(
				Input.OpeningInput.PlayerADeck));
	TestTrue(
		TEXT("PlayerA used cards start empty"),
		Result.MatchPlayState.CardUsageState.PlayerACardUsageState
			.UsedCardIds.IsEmpty());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayOpeningInitializerPlayerBCardsTest,
	"FMCodex.CoreRules.MatchPlayOpeningInitializer.SuccessInitializesPlayerBCards",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayOpeningInitializerPlayerBCardsTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayOpeningInitializeInput Input =
		MatchPlayOpeningInitializerTests::MakeInput();
	const FMatchPlayOpeningInitializeResult Result =
		FMatchPlayOpeningInitializer::InitializeMatchPlayOpening(Input);

	TestTrue(
		TEXT("PlayerB available cards preserve input"),
		Result.MatchPlayState.CardUsageState.PlayerBCardUsageState
			.AvailableCardIds
			== MatchPlayOpeningInitializerTests::GetCardIds(
				Input.OpeningInput.PlayerBDeck));
	TestTrue(
		TEXT("PlayerB used cards start empty"),
		Result.MatchPlayState.CardUsageState.PlayerBCardUsageState
			.UsedCardIds.IsEmpty());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayOpeningInitializerHigherAttackTest,
	"FMCodex.CoreRules.MatchPlayOpeningInitializer.HigherTotalAttackCountActsFirst",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayOpeningInitializerHigherAttackTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayOpeningInitializeInput Input =
		MatchPlayOpeningInitializerTests::MakeInput();
	Input.OpeningInput.PlayerAAttackCountD6Roll = 6;
	Input.OpeningInput.PlayerBAttackCountD6Roll = 1;

	const FMatchPlayOpeningInitializeResult Result =
		FMatchPlayOpeningInitializer::InitializeMatchPlayOpening(Input);

	TestTrue(TEXT("Different attack totals resolve"), Result.bSuccess);
	TestTrue(
		TEXT("PlayerA has more total attacks"),
		Result.MatchPlayState.RuntimeState.PlayerAState.TotalAttackCount
			> Result.MatchPlayState.RuntimeState.PlayerBState.TotalAttackCount);
	TestEqual(
		TEXT("PlayerA acts first"),
		Result.MatchPlayState.RuntimeState.FirstPlayer,
		EInitialTurnOrderPlayer::PlayerA);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayOpeningInitializerLowerRarityTest,
	"FMCodex.CoreRules.MatchPlayOpeningInitializer.EqualAttackCountLowerRarityActsFirst",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayOpeningInitializerLowerRarityTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayOpeningInitializeInput Input =
		MatchPlayOpeningInitializerTests::MakeInput(
			ECardRarity::WorldClass,
			ECardRarity::Common);
	Input.OpeningInput.PlayerAAttackCountD6Roll = 1;
	Input.OpeningInput.PlayerBAttackCountD6Roll = 3;

	const FMatchPlayOpeningInitializeResult Result =
		FMatchPlayOpeningInitializer::InitializeMatchPlayOpening(Input);

	TestTrue(TEXT("Equal attacks with different rarity resolve"), Result.bSuccess);
	TestEqual(
		TEXT("Attack totals are equal"),
		Result.MatchPlayState.RuntimeState.PlayerAState.TotalAttackCount,
		Result.MatchPlayState.RuntimeState.PlayerBState.TotalAttackCount);
	TestEqual(
		TEXT("Lower rarity PlayerB acts first"),
		Result.MatchPlayState.RuntimeState.FirstPlayer,
		EInitialTurnOrderPlayer::PlayerB);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayOpeningInitializerTieBreakerTest,
	"FMCodex.CoreRules.MatchPlayOpeningInitializer.TieBreakerLowerRollActsFirst",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayOpeningInitializerTieBreakerTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayOpeningInitializeInput Input =
		MatchPlayOpeningInitializerTests::MakeInput();
	Input.OpeningInput.PlayerATieBreakerRoll = 2;
	Input.OpeningInput.PlayerBTieBreakerRoll = 6;

	const FMatchPlayOpeningInitializeResult Result =
		FMatchPlayOpeningInitializer::InitializeMatchPlayOpening(Input);

	TestTrue(TEXT("Valid external tie breaker resolves"), Result.bSuccess);
	TestEqual(
		TEXT("Lower tie breaker PlayerA acts first"),
		Result.MatchPlayState.RuntimeState.FirstPlayer,
		EInitialTurnOrderPlayer::PlayerA);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayOpeningInitializerTieBreakerDrawTest,
	"FMCodex.CoreRules.MatchPlayOpeningInitializer.TieBreakerDrawRequiresReroll",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayOpeningInitializerTieBreakerDrawTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayOpeningInitializeInput Input =
		MatchPlayOpeningInitializerTests::MakeInput();
	Input.OpeningInput.PlayerATieBreakerRoll = 4;
	Input.OpeningInput.PlayerBTieBreakerRoll = 4;

	const FMatchPlayOpeningInitializeResult Result =
		FMatchPlayOpeningInitializer::InitializeMatchPlayOpening(Input);

	TestFalse(TEXT("Tied tie breaker fails this attempt"), Result.bSuccess);
	TestEqual(
		TEXT("Tie is reported as opening failure"),
		Result.ErrorCode,
		EMatchPlayOpeningInitializeErrorCode::OpeningResolveFailed);
	TestTrue(
		TEXT("External tie breaker reroll is requested"),
		Result.bRequiresTieBreakerReroll);
	TestTrue(
		TEXT("Underlying turn order stage failure is retained"),
		Result.UnderlyingOpeningErrorCodes.Contains(
			EMatchOpeningResolveErrorCode::InitialTurnOrderResolutionFailed));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayOpeningInitializerInvalidD6Test,
	"FMCodex.CoreRules.MatchPlayOpeningInitializer.InvalidD6InputFails",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayOpeningInitializerInvalidD6Test::RunTest(
	const FString& Parameters)
{
	FMatchPlayOpeningInitializeInput Input =
		MatchPlayOpeningInitializerTests::MakeInput();
	Input.OpeningInput.PlayerAAttackCountD6Roll = 0;

	const FMatchPlayOpeningInitializeResult Result =
		FMatchPlayOpeningInitializer::InitializeMatchPlayOpening(Input);

	TestFalse(TEXT("Invalid external D6 input fails"), Result.bSuccess);
	TestEqual(
		TEXT("Invalid D6 is an opening failure"),
		Result.ErrorCode,
		EMatchPlayOpeningInitializeErrorCode::OpeningResolveFailed);
	TestTrue(
		TEXT("Attack count stage failure is retained"),
		Result.UnderlyingOpeningErrorCodes.Contains(
			EMatchOpeningResolveErrorCode::AttackCountResolutionFailed));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayOpeningInitializerPlayerAInvalidCardTest,
	"FMCodex.CoreRules.MatchPlayOpeningInitializer.InvalidPlayerACardFails",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayOpeningInitializerPlayerAInvalidCardTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayOpeningInitializeInput Input =
		MatchPlayOpeningInitializerTests::MakeInput();
	Input.OpeningInput.PlayerADeck[0].CardId = NAME_None;

	const FMatchPlayOpeningInitializeResult Result =
		FMatchPlayOpeningInitializer::InitializeMatchPlayOpening(Input);

	TestFalse(TEXT("Invalid PlayerA CardId fails"), Result.bSuccess);
	TestEqual(
		TEXT("Failure occurs during play state initialization"),
		Result.ErrorCode,
		EMatchPlayOpeningInitializeErrorCode::PlayStateInitializationFailed);
	TestEqual(
		TEXT("PlayerA snapshot error is preserved"),
		Result.UnderlyingPlayerCardRuleSnapshotValidationErrorCode,
		EPlayerCardRuleSnapshotValidationErrorCode::InvalidCardId);
	TestEqual(
		TEXT("PlayerA failure is classified as snapshot validation"),
		Result.UnderlyingCardSnapshotAuthorityBuildErrorCode,
		EMatchPlayCardSnapshotAuthorityBuildErrorCode
			::SnapshotValidationFailed);
	TestEqual(
		TEXT("PlayerA failing side is preserved"),
		Result.UnderlyingCardSnapshotAuthorityFailingPlayerSide,
		EInitialTurnOrderPlayer::PlayerA);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayOpeningInitializerPlayerBInvalidCardTest,
	"FMCodex.CoreRules.MatchPlayOpeningInitializer.InvalidPlayerBCardFails",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayOpeningInitializerPlayerBInvalidCardTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayOpeningInitializeInput Input =
		MatchPlayOpeningInitializerTests::MakeInput();
	Input.OpeningInput.PlayerBDeck[0].CardId = NAME_None;

	const FMatchPlayOpeningInitializeResult Result =
		FMatchPlayOpeningInitializer::InitializeMatchPlayOpening(Input);

	TestFalse(TEXT("Invalid PlayerB CardId fails"), Result.bSuccess);
	TestEqual(
		TEXT("PlayerB snapshot error is preserved"),
		Result.UnderlyingPlayerCardRuleSnapshotValidationErrorCode,
		EPlayerCardRuleSnapshotValidationErrorCode::InvalidCardId);
	TestEqual(
		TEXT("PlayerB failure is classified as snapshot validation"),
		Result.UnderlyingCardSnapshotAuthorityBuildErrorCode,
		EMatchPlayCardSnapshotAuthorityBuildErrorCode
			::SnapshotValidationFailed);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayOpeningInitializerPlayerADuplicateTest,
	"FMCodex.CoreRules.MatchPlayOpeningInitializer.DuplicatePlayerACardFails",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayOpeningInitializerPlayerADuplicateTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayOpeningInitializeInput Input =
		MatchPlayOpeningInitializerTests::MakeInput();
	Input.OpeningInput.PlayerADeck[1].CardId =
		Input.OpeningInput.PlayerADeck[0].CardId;

	const FMatchPlayOpeningInitializeResult Result =
		FMatchPlayOpeningInitializer::InitializeMatchPlayOpening(Input);

	TestFalse(TEXT("Duplicate PlayerA CardId fails"), Result.bSuccess);
	TestEqual(
		TEXT("PlayerA duplicate snapshot error is preserved"),
		Result.UnderlyingPlayerCardRuleSnapshotValidationErrorCode,
		EPlayerCardRuleSnapshotValidationErrorCode::DuplicateCardId);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayOpeningInitializerPlayerBDuplicateTest,
	"FMCodex.CoreRules.MatchPlayOpeningInitializer.DuplicatePlayerBCardFails",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayOpeningInitializerPlayerBDuplicateTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayOpeningInitializeInput Input =
		MatchPlayOpeningInitializerTests::MakeInput();
	Input.OpeningInput.PlayerBDeck[1].CardId =
		Input.OpeningInput.PlayerBDeck[0].CardId;

	const FMatchPlayOpeningInitializeResult Result =
		FMatchPlayOpeningInitializer::InitializeMatchPlayOpening(Input);

	TestFalse(TEXT("Duplicate PlayerB CardId fails"), Result.bSuccess);
	TestEqual(
		TEXT("PlayerB duplicate snapshot error is preserved"),
		Result.UnderlyingPlayerCardRuleSnapshotValidationErrorCode,
		EPlayerCardRuleSnapshotValidationErrorCode::DuplicateCardId);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayOpeningInitializerOpeningAtomicTest,
	"FMCodex.CoreRules.MatchPlayOpeningInitializer.FailureIsAtomicWhenOpeningFails",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayOpeningInitializerOpeningAtomicTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayOpeningInitializeInput Input =
		MatchPlayOpeningInitializerTests::MakeInput();
	Input.OpeningInput.PlayerBAttackCountD6Roll = 7;

	const FMatchPlayOpeningInitializeResult Result =
		FMatchPlayOpeningInitializer::InitializeMatchPlayOpening(Input);

	TestFalse(TEXT("Opening failure is not successful"), Result.bSuccess);
	TestTrue(
		TEXT("Opening failure returns no partial match play state"),
		MatchPlayOpeningInitializerTests::IsMatchPlayStateEmpty(
			Result.MatchPlayState));
	TestTrue(
		TEXT("Opening failure retains a diagnostic message"),
		!Result.ErrorMessage.IsEmpty());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayOpeningInitializerCardAtomicTest,
	"FMCodex.CoreRules.MatchPlayOpeningInitializer.FailureIsAtomicWhenCardUsageFails",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayOpeningInitializerCardAtomicTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayOpeningInitializeInput Input =
		MatchPlayOpeningInitializerTests::MakeInput();
	Input.OpeningInput.PlayerBDeck[0].CardId = NAME_None;

	const FMatchPlayOpeningInitializeResult Result =
		FMatchPlayOpeningInitializer::InitializeMatchPlayOpening(Input);

	TestFalse(TEXT("Card usage failure is not successful"), Result.bSuccess);
	TestTrue(
		TEXT("Card usage failure returns no partial match play state"),
		MatchPlayOpeningInitializerTests::IsMatchPlayStateEmpty(
			Result.MatchPlayState));
	TestEqual(
		TEXT("Snapshot failure retains play state layer error"),
		Result.UnderlyingPlayStateInitializeErrorCode,
		EMatchPlayStateInitializeErrorCode
			::CardSnapshotAuthorityInitializationFailed);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayOpeningInitializerInputsTest,
	"FMCodex.CoreRules.MatchPlayOpeningInitializer.InputsUnchanged",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayOpeningInitializerInputsTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayOpeningInitializeInput Input =
		MatchPlayOpeningInitializerTests::MakeInput();
	const int32 PlayerADeckCount = Input.OpeningInput.PlayerADeck.Num();
	const FName PlayerAFirstDeckCard =
		Input.OpeningInput.PlayerADeck[0].CardId;
	const int32 PlayerAD6 = Input.OpeningInput.PlayerAAttackCountD6Roll;
	const int32 PlayerATieBreaker =
		Input.OpeningInput.PlayerATieBreakerRoll;

	FMatchPlayOpeningInitializeResult Result =
		FMatchPlayOpeningInitializer::InitializeMatchPlayOpening(Input);
	Result.MatchPlayState.RuntimeState.PlayerAState.Score = 99;
	Result.MatchPlayState.CardUsageState.PlayerACardUsageState
		.AvailableCardIds.Reset();

	TestEqual(
		TEXT("PlayerA deck count is unchanged"),
		Input.OpeningInput.PlayerADeck.Num(),
		PlayerADeckCount);
	TestEqual(
		TEXT("PlayerA first deck CardId is unchanged"),
		Input.OpeningInput.PlayerADeck[0].CardId,
		PlayerAFirstDeckCard);
	TestEqual(
		TEXT("PlayerA external D6 input is unchanged"),
		Input.OpeningInput.PlayerAAttackCountD6Roll,
		PlayerAD6);
	TestEqual(
		TEXT("PlayerA external tie breaker input is unchanged"),
		Input.OpeningInput.PlayerATieBreakerRoll,
		PlayerATieBreaker);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayOpeningInitializerDeterministicTest,
	"FMCodex.CoreRules.MatchPlayOpeningInitializer.DeterministicExternalRollInputs",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayOpeningInitializerDeterministicTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayOpeningInitializeInput Input =
		MatchPlayOpeningInitializerTests::MakeInput();

	const FMatchPlayOpeningInitializeResult First =
		FMatchPlayOpeningInitializer::InitializeMatchPlayOpening(Input);
	const FMatchPlayOpeningInitializeResult Second =
		FMatchPlayOpeningInitializer::InitializeMatchPlayOpening(Input);

	TestTrue(TEXT("Repeated deterministic input succeeds"), First.bSuccess && Second.bSuccess);
	TestEqual(
		TEXT("Repeated input keeps PlayerA total attacks"),
		First.MatchPlayState.RuntimeState.PlayerAState.TotalAttackCount,
		Second.MatchPlayState.RuntimeState.PlayerAState.TotalAttackCount);
	TestEqual(
		TEXT("Repeated input keeps first player"),
		First.MatchPlayState.RuntimeState.FirstPlayer,
		Second.MatchPlayState.RuntimeState.FirstPlayer);
	TestTrue(
		TEXT("Repeated input keeps PlayerA available cards"),
		First.MatchPlayState.CardUsageState.PlayerACardUsageState
			.AvailableCardIds
			== Second.MatchPlayState.CardUsageState.PlayerACardUsageState
				.AvailableCardIds);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayOpeningInitializerCatalogPropagationTest,
	"FMCodex.CoreRules.MatchPlayOpeningInitializer.SuccessCopiesDeploymentSlotCatalog",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayOpeningInitializerCatalogPropagationTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayOpeningInitializeInput Input =
		MatchPlayOpeningInitializerTests::MakeInput();
	const FMatchPlayOpeningInitializeResult Result =
		FMatchPlayOpeningInitializer::InitializeMatchPlayOpening(Input);

	TestTrue(TEXT("Opening succeeds"), Result.bSuccess);
	TestTrue(
		TEXT("Opening propagates the catalog into match play state"),
		FMatchPlayDeploymentSlotCatalog::StaticStruct()
			->CompareScriptStruct(
				&Result.MatchPlayState.DeploymentSlotCatalog,
				&Input.DeploymentSlotCatalog,
				0));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayOpeningInitializerCatalogValueCopyTest,
	"FMCodex.CoreRules.MatchPlayOpeningInitializer.InputCatalogMutationDoesNotChangeState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayOpeningInitializerCatalogValueCopyTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayOpeningInitializeInput Input =
		MatchPlayOpeningInitializerTests::MakeInput();
	const FMatchPlayOpeningInitializeResult Result =
		FMatchPlayOpeningInitializer::InitializeMatchPlayOpening(Input);
	const FName StateFirstSlotId =
		Result.MatchPlayState.DeploymentSlotCatalog.Slots[0].SlotId;

	Input.DeploymentSlotCatalog.Slots[0].SlotId = TEXT("CallerMutation");
	Input.DeploymentSlotCatalog.Slots.Reset();

	TestTrue(TEXT("Opening succeeds"), Result.bSuccess);
	TestEqual(
		TEXT("State catalog is independent from the opening input"),
		Result.MatchPlayState.DeploymentSlotCatalog.Slots[0].SlotId,
		StateFirstSlotId);
	TestEqual(
		TEXT("State retains all copied slots"),
		Result.MatchPlayState.DeploymentSlotCatalog.Slots.Num(),
		2);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayOpeningInitializerEmptyCatalogTest,
	"FMCodex.CoreRules.MatchPlayOpeningInitializer.EmptyCatalogFailsWithNestedErrors",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayOpeningInitializerEmptyCatalogTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayOpeningInitializeInput Input =
		MatchPlayOpeningInitializerTests::MakeInput();
	Input.DeploymentSlotCatalog = FMatchPlayDeploymentSlotCatalog();
	const FMatchPlayOpeningInitializeResult Result =
		FMatchPlayOpeningInitializer::InitializeMatchPlayOpening(Input);

	TestFalse(TEXT("Empty catalog fails opening"), Result.bSuccess);
	TestEqual(
		TEXT("Opening maps the state initialization failure"),
		Result.ErrorCode,
		EMatchPlayOpeningInitializeErrorCode::PlayStateInitializationFailed);
	TestEqual(
		TEXT("State layer preserves catalog validation failure"),
		Result.UnderlyingPlayStateInitializeErrorCode,
		EMatchPlayStateInitializeErrorCode
			::DeploymentSlotCatalogValidationFailed);
	TestEqual(
		TEXT("Catalog layer preserves EmptyCatalog"),
		Result.UnderlyingDeploymentSlotCatalogValidationErrorCode,
		EMatchPlayDeploymentSlotCatalogValidationErrorCode::EmptyCatalog);
	TestTrue(
		TEXT("Catalog failure is atomic"),
		MatchPlayOpeningInitializerTests::IsMatchPlayStateEmpty(
			Result.MatchPlayState));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayOpeningInitializerDuplicateSlotTest,
	"FMCodex.CoreRules.MatchPlayOpeningInitializer.DuplicateSlotIdFailsWithNestedErrors",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayOpeningInitializerDuplicateSlotTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayOpeningInitializeInput Input =
		MatchPlayOpeningInitializerTests::MakeInput();
	Input.DeploymentSlotCatalog.Slots[1].SlotId =
		Input.DeploymentSlotCatalog.Slots[0].SlotId;
	const FMatchPlayOpeningInitializeResult Result =
		FMatchPlayOpeningInitializer::InitializeMatchPlayOpening(Input);

	TestFalse(TEXT("Duplicate SlotId fails opening"), Result.bSuccess);
	TestEqual(
		TEXT("Catalog layer preserves DuplicateSlotId"),
		Result.UnderlyingDeploymentSlotCatalogValidationErrorCode,
		EMatchPlayDeploymentSlotCatalogValidationErrorCode::DuplicateSlotId);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayOpeningInitializerOpeningPriorityTest,
	"FMCodex.CoreRules.MatchPlayOpeningInitializer.OpeningFailurePrecedesCatalogFailure",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayOpeningInitializerOpeningPriorityTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayOpeningInitializeInput Input =
		MatchPlayOpeningInitializerTests::MakeInput();
	Input.OpeningInput.PlayerAAttackCountD6Roll = 7;
	Input.DeploymentSlotCatalog = FMatchPlayDeploymentSlotCatalog();
	const FMatchPlayOpeningInitializeResult Result =
		FMatchPlayOpeningInitializer::InitializeMatchPlayOpening(Input);

	TestEqual(
		TEXT("Opening resolve failure wins"),
		Result.ErrorCode,
		EMatchPlayOpeningInitializeErrorCode::OpeningResolveFailed);
	TestEqual(
		TEXT("Catalog validation was not reached"),
		Result.UnderlyingDeploymentSlotCatalogValidationErrorCode,
		EMatchPlayDeploymentSlotCatalogValidationErrorCode::None);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayOpeningInitializerCatalogPriorityTest,
	"FMCodex.CoreRules.MatchPlayOpeningInitializer.CatalogFailurePrecedesCardUsageFailure",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayOpeningInitializerCatalogPriorityTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayOpeningInitializeInput Input =
		MatchPlayOpeningInitializerTests::MakeInput();
	Input.OpeningInput.PlayerADeck[0].CardId = NAME_None;
	Input.DeploymentSlotCatalog = FMatchPlayDeploymentSlotCatalog();
	const FMatchPlayOpeningInitializeResult Result =
		FMatchPlayOpeningInitializer::InitializeMatchPlayOpening(Input);

	TestEqual(
		TEXT("Catalog failure maps through the state layer"),
		Result.UnderlyingPlayStateInitializeErrorCode,
		EMatchPlayStateInitializeErrorCode
			::DeploymentSlotCatalogValidationFailed);
	TestEqual(
		TEXT("Snapshot authority validation was not reached"),
		Result.UnderlyingCardSnapshotAuthorityBuildErrorCode,
		EMatchPlayCardSnapshotAuthorityBuildErrorCode::None);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayOpeningInitializerNonCatalogDefaultsTest,
	"FMCodex.CoreRules.MatchPlayOpeningInitializer.NonCatalogFailureLeavesCatalogErrorNone",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayOpeningInitializerNonCatalogDefaultsTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayOpeningInitializeInput Input =
		MatchPlayOpeningInitializerTests::MakeInput();
	Input.OpeningInput.PlayerBDeck[0].CardId = NAME_None;
	const FMatchPlayOpeningInitializeResult Result =
		FMatchPlayOpeningInitializer::InitializeMatchPlayOpening(Input);

	TestEqual(
		TEXT("Snapshot failure reaches state initialization"),
		Result.UnderlyingPlayStateInitializeErrorCode,
		EMatchPlayStateInitializeErrorCode
			::CardSnapshotAuthorityInitializationFailed);
	TestEqual(
		TEXT("Non-catalog failure leaves catalog error None"),
		Result.UnderlyingDeploymentSlotCatalogValidationErrorCode,
		EMatchPlayDeploymentSlotCatalogValidationErrorCode::None);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayOpeningInitializerIndependentOpeningsTest,
	"FMCodex.CoreRules.MatchPlayOpeningInitializer.RepeatedOpeningsReturnIndependentStates",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayOpeningInitializerIndependentOpeningsTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayOpeningInitializeInput FirstInput =
		MatchPlayOpeningInitializerTests::MakeInput();
	FMatchPlayOpeningInitializeInput SecondInput =
		MatchPlayOpeningInitializerTests::MakeInput();
	SecondInput.DeploymentSlotCatalog =
		MatchPlayOpeningInitializerTests::MakeDeploymentSlotCatalog(
			TEXT("SecondMatchSlot"));

	const FMatchPlayOpeningInitializeResult First =
		FMatchPlayOpeningInitializer::InitializeMatchPlayOpening(FirstInput);
	FMatchPlayOpeningInitializeResult Second =
		FMatchPlayOpeningInitializer::InitializeMatchPlayOpening(SecondInput);
	const FName FirstSlotId =
		First.MatchPlayState.DeploymentSlotCatalog.Slots[0].SlotId;
	const FName FirstSnapshotCardId = First.MatchPlayState
		.CardSnapshotAuthority.PlayerACardSnapshots.Cards[0].CardId;
	Second.MatchPlayState.DeploymentSlotCatalog.Slots[0].SlotId =
		TEXT("SecondResultMutation");
	Second.MatchPlayState.CardSnapshotAuthority
		.PlayerACardSnapshots.Cards[0].CardId =
			TEXT("SecondSnapshotMutation");

	TestTrue(TEXT("First opening succeeds"), First.bSuccess);
	TestTrue(TEXT("Second opening succeeds"), Second.bSuccess);
	TestEqual(
		TEXT("Second opening does not modify the first state"),
		First.MatchPlayState.DeploymentSlotCatalog.Slots[0].SlotId,
		FirstSlotId);
	TestNotEqual(
		TEXT("Two match states own independent catalog values"),
		First.MatchPlayState.DeploymentSlotCatalog.Slots[0].SlotId,
		Second.MatchPlayState.DeploymentSlotCatalog.Slots[0].SlotId);
	TestEqual(
		TEXT("Second snapshot mutation does not modify first state"),
		First.MatchPlayState.CardSnapshotAuthority
			.PlayerACardSnapshots.Cards[0].CardId,
		FirstSnapshotCardId);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayOpeningInitializerDeckMutationIsolationTest,
	"FMCodex.CoreRules.MatchPlayOpeningInitializer.DeckMutationDoesNotChangeSnapshotAuthority",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayOpeningInitializerDeckMutationIsolationTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayOpeningInitializeInput Input =
		MatchPlayOpeningInitializerTests::MakeInput();
	const FMatchPlayOpeningInitializeResult Result =
		FMatchPlayOpeningInitializer::InitializeMatchPlayOpening(Input);
	const FName StateCardId = Result.MatchPlayState.CardSnapshotAuthority
		.PlayerACardSnapshots.Cards[0].CardId;
	Input.OpeningInput.PlayerADeck[0].CardId = TEXT("CallerMutation");
	Input.OpeningInput.PlayerADeck.Reset();

	TestTrue(TEXT("Opening succeeds"), Result.bSuccess);
	TestEqual(
		TEXT("State snapshot authority owns a value copy"),
		Result.MatchPlayState.CardSnapshotAuthority
			.PlayerACardSnapshots.Cards[0].CardId,
		StateCardId);
	TestEqual(
		TEXT("State snapshot count remains complete"),
		Result.MatchPlayState.CardSnapshotAuthority
			.PlayerACardSnapshots.Cards.Num(),
		20);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayOpeningInitializerSameCardIdAcrossSidesTest,
	"FMCodex.CoreRules.MatchPlayOpeningInitializer.SameCardIdAcrossSidesIsIsolated",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayOpeningInitializerSameCardIdAcrossSidesTest::RunTest(
	const FString& Parameters)
{
	const FName SharedCardId(TEXT("Shared01"));
	FMatchPlayOpeningInitializeInput Input =
		MatchPlayOpeningInitializerTests::MakeInput();
	Input.OpeningInput.PlayerADeck[0].CardId = SharedCardId;
	Input.OpeningInput.PlayerADeck[0].Attributes.LongShot = 6;
	Input.OpeningInput.PlayerBDeck[0].CardId = SharedCardId;
	Input.OpeningInput.PlayerBDeck[0].Attributes.LongShot = 3;

	const FMatchPlayOpeningInitializeResult Result =
		FMatchPlayOpeningInitializer::InitializeMatchPlayOpening(Input);
	const FMatchPlayCardSnapshotAuthorityQueryResult PlayerAQuery =
		FMatchPlayCardSnapshotAuthorityQuery::FindByPlayerSideAndCardId(
			Result.MatchPlayState.CardSnapshotAuthority,
			EInitialTurnOrderPlayer::PlayerA,
			SharedCardId);
	const FMatchPlayCardSnapshotAuthorityQueryResult PlayerBQuery =
		FMatchPlayCardSnapshotAuthorityQuery::FindByPlayerSideAndCardId(
			Result.MatchPlayState.CardSnapshotAuthority,
			EInitialTurnOrderPlayer::PlayerB,
			SharedCardId);

	TestTrue(TEXT("Cross-side same CardId opening succeeds"), Result.bSuccess);
	TestEqual(TEXT("PlayerA query returns PlayerA rules"),
		PlayerAQuery.Snapshot.Attributes.LongShot, 6);
	TestEqual(TEXT("PlayerB query returns PlayerB rules"),
		PlayerBQuery.Snapshot.Attributes.LongShot, 3);
	TestTrue(TEXT("PlayerA CardUsage includes shared CardId"),
		Result.MatchPlayState.CardUsageState.PlayerACardUsageState
			.AvailableCardIds.Contains(SharedCardId));
	TestTrue(TEXT("PlayerB CardUsage includes shared CardId"),
		Result.MatchPlayState.CardUsageState.PlayerBCardUsageState
			.AvailableCardIds.Contains(SharedCardId));
	return true;
}

#endif
