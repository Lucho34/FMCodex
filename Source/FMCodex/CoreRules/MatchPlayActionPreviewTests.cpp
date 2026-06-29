#include "MatchPlayActionPreview.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

namespace MatchPlayActionPreviewTests
{
	const FName CardA1(TEXT("CardA1"));
	const FName CardA2(TEXT("CardA2"));
	const FName CardAUsed(TEXT("CardAUsed"));
	const FName CardB1(TEXT("CardB1"));
	const FName CardBUsed(TEXT("CardBUsed"));
	const FName SharedCard(TEXT("SharedCard"));
	const FName MissingCard(TEXT("MissingCard"));

	FMatchPlayState MakeState(
		const EInitialTurnOrderPlayer CurrentPlayer =
			EInitialTurnOrderPlayer::PlayerA)
	{
		FMatchRuntimeState RuntimeState;
		RuntimeState.bIsInitialized = true;
		RuntimeState.PlayerAState.TotalAttackCount = 5;
		RuntimeState.PlayerAState.UsedAttackCount = 2;
		RuntimeState.PlayerAState.Score = 3;
		RuntimeState.PlayerBState.TotalAttackCount = 4;
		RuntimeState.PlayerBState.UsedAttackCount = 1;
		RuntimeState.PlayerBState.Score = 2;
		RuntimeState.FirstPlayer = EInitialTurnOrderPlayer::PlayerA;
		RuntimeState.SecondPlayer = EInitialTurnOrderPlayer::PlayerB;
		RuntimeState.CurrentAttackingPlayer = CurrentPlayer;
		RuntimeState.OpeningResultSnapshot.bSuccess = true;
		RuntimeState.OpeningResultSnapshot.AttackCountResult
			.PlayerAAttackCount = 5;
		RuntimeState.OpeningResultSnapshot.AttackCountResult
			.PlayerBAttackCount = 4;

		FMatchCardUsageState CardUsageState;
		CardUsageState.PlayerACardUsageState.AvailableCardIds =
			{ CardA1, CardA2 };
		CardUsageState.PlayerACardUsageState.UsedCardIds =
			{ CardAUsed };
		CardUsageState.PlayerBCardUsageState.AvailableCardIds =
			{ CardB1 };
		CardUsageState.PlayerBCardUsageState.UsedCardIds =
			{ CardBUsed };
		return FMatchPlayState::Create(RuntimeState, CardUsageState);
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayActionPreviewStatusTest,
	"FMCodex.CoreRules.MatchPlayActionPreview.PreviewReturnsStatusSnapshot",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayActionPreviewStatusTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayActionPreviewResult Result =
		FMatchPlayActionPreview::PreviewAction(
			MatchPlayActionPreviewTests::MakeState(),
			EInitialTurnOrderPlayer::PlayerA,
			MatchPlayActionPreviewTests::CardA1);

	TestTrue(TEXT("Preview snapshot is available"), Result.bPreviewAvailable);
	TestEqual(TEXT("PlayerA score is returned"), Result.StatusSnapshot.PlayerAScore, 3);
	TestEqual(TEXT("PlayerB score is returned"), Result.StatusSnapshot.PlayerBScore, 2);
	TestEqual(
		TEXT("Current attacker is returned"),
		Result.StatusSnapshot.CurrentAttackingPlayer,
		EInitialTurnOrderPlayer::PlayerA);
	TestEqual(
		TEXT("PlayerA remaining attacks are returned"),
		Result.StatusSnapshot.PlayerARemainingAttackCount,
		3);
	TestEqual(
		TEXT("PlayerB remaining attacks are returned"),
		Result.StatusSnapshot.PlayerBRemainingAttackCount,
		3);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayActionPreviewCardCountsTest,
	"FMCodex.CoreRules.MatchPlayActionPreview.PreviewReturnsCardCounts",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayActionPreviewCardCountsTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayActionPreviewResult Result =
		FMatchPlayActionPreview::PreviewAction(
			MatchPlayActionPreviewTests::MakeState(),
			EInitialTurnOrderPlayer::PlayerA,
			MatchPlayActionPreviewTests::CardA1);

	TestEqual(TEXT("PlayerA available count is returned"), Result.StatusSnapshot.PlayerAAvailableCardCount, 2);
	TestEqual(TEXT("PlayerA used count is returned"), Result.StatusSnapshot.PlayerAUsedCardCount, 1);
	TestEqual(TEXT("PlayerB available count is returned"), Result.StatusSnapshot.PlayerBAvailableCardCount, 1);
	TestEqual(TEXT("PlayerB used count is returned"), Result.StatusSnapshot.PlayerBUsedCardCount, 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayActionPreviewCanPlayTest,
	"FMCodex.CoreRules.MatchPlayActionPreview.PreviewCanPlayWhenAvailabilityAllows",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayActionPreviewCanPlayTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayActionPreviewResult Result =
		FMatchPlayActionPreview::PreviewAction(
			MatchPlayActionPreviewTests::MakeState(),
			EInitialTurnOrderPlayer::PlayerA,
			MatchPlayActionPreviewTests::CardA1);

	TestTrue(TEXT("Available action can be played"), Result.bCanPlay);
	TestTrue(TEXT("Availability result is preserved"), Result.AvailabilityResult.bCanPlay);
	TestEqual(TEXT("Successful preview has no error"), Result.ErrorCode, EMatchPlayActionPreviewErrorCode::None);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayActionPreviewNotCurrentTest,
	"FMCodex.CoreRules.MatchPlayActionPreview.PreviewCannotPlayWhenNotCurrentAttacker",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayActionPreviewNotCurrentTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayActionPreviewResult Result =
		FMatchPlayActionPreview::PreviewAction(
			MatchPlayActionPreviewTests::MakeState(),
			EInitialTurnOrderPlayer::PlayerB,
			MatchPlayActionPreviewTests::CardB1);

	TestTrue(TEXT("Preview still returns a snapshot"), Result.bPreviewAvailable);
	TestFalse(TEXT("Non-current player cannot play"), Result.bCanPlay);
	TestEqual(
		TEXT("Availability reason is preserved"),
		Result.AvailabilityResult.ErrorCode,
		EMatchPlayAvailabilityErrorCode::NotCurrentAttacker);
	TestEqual(
		TEXT("Preview maps unavailable action"),
		Result.ErrorCode,
		EMatchPlayActionPreviewErrorCode::CannotPlaySelectedCard);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayActionPreviewUnavailableTest,
	"FMCodex.CoreRules.MatchPlayActionPreview.PreviewCannotPlayWhenCardUnavailable",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayActionPreviewUnavailableTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayActionPreviewResult Result =
		FMatchPlayActionPreview::PreviewAction(
			MatchPlayActionPreviewTests::MakeState(),
			EInitialTurnOrderPlayer::PlayerA,
			MatchPlayActionPreviewTests::MissingCard);

	TestFalse(TEXT("Unavailable card cannot be played"), Result.bCanPlay);
	TestEqual(
		TEXT("Card unavailable reason is preserved"),
		Result.AvailabilityResult.ErrorCode,
		EMatchPlayAvailabilityErrorCode::CardNotAvailable);
	TestTrue(TEXT("Unavailable reason remains readable"), !Result.ErrorMessage.IsEmpty());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayActionPreviewUsedCardTest,
	"FMCodex.CoreRules.MatchPlayActionPreview.PreviewCannotPlayWhenCardAlreadyUsed",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayActionPreviewUsedCardTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayState State =
		MatchPlayActionPreviewTests::MakeState();
	State.CardUsageState.PlayerACardUsageState.AvailableCardIds.Remove(
		MatchPlayActionPreviewTests::CardA1);
	State.CardUsageState.PlayerACardUsageState.UsedCardIds.Add(
		MatchPlayActionPreviewTests::CardA1);

	const FMatchPlayActionPreviewResult Result =
		FMatchPlayActionPreview::PreviewAction(
			State,
			EInitialTurnOrderPlayer::PlayerA,
			MatchPlayActionPreviewTests::CardA1);

	TestFalse(TEXT("Used card cannot be played"), Result.bCanPlay);
	TestEqual(
		TEXT("Already-used reason is preserved"),
		Result.AvailabilityResult.ErrorCode,
		EMatchPlayAvailabilityErrorCode::CardAlreadyUsed);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayActionPreviewExternalFormulaTest,
	"FMCodex.CoreRules.MatchPlayActionPreview.PreviewIncludesRequiresExternalFormulaInputFlag",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayActionPreviewExternalFormulaTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayActionPreviewResult Result =
		FMatchPlayActionPreview::PreviewAction(
			MatchPlayActionPreviewTests::MakeState(),
			EInitialTurnOrderPlayer::PlayerA,
			MatchPlayActionPreviewTests::CardA1);

	TestTrue(
		TEXT("Real attack still requires external formula input"),
		Result.bRequiresExternalFormulaInput);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayActionPreviewInputUnchangedTest,
	"FMCodex.CoreRules.MatchPlayActionPreview.PreviewDoesNotModifyInput",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayActionPreviewInputUnchangedTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State =
		MatchPlayActionPreviewTests::MakeState();
	const TArray<FName> PlayerAAvailable =
		State.CardUsageState.PlayerACardUsageState.AvailableCardIds;

	const FMatchPlayActionPreviewResult Result =
		FMatchPlayActionPreview::PreviewAction(
			State,
			EInitialTurnOrderPlayer::PlayerA,
			MatchPlayActionPreviewTests::CardA1);

	TestTrue(TEXT("Preview succeeds"), Result.bPreviewAvailable);
	TestEqual(TEXT("Input PlayerA score is unchanged"), State.RuntimeState.PlayerAState.Score, 3);
	TestEqual(TEXT("Input PlayerA used attacks are unchanged"), State.RuntimeState.PlayerAState.UsedAttackCount, 2);
	TestTrue(
		TEXT("Input PlayerA available cards are unchanged"),
		State.CardUsageState.PlayerACardUsageState.AvailableCardIds
			== PlayerAAvailable);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayActionPreviewDoesNotConsumeCardTest,
	"FMCodex.CoreRules.MatchPlayActionPreview.PreviewDoesNotConsumeCard",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayActionPreviewDoesNotConsumeCardTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State =
		MatchPlayActionPreviewTests::MakeState();

	const FMatchPlayActionPreviewResult Result =
		FMatchPlayActionPreview::PreviewAction(
			State,
			EInitialTurnOrderPlayer::PlayerA,
			MatchPlayActionPreviewTests::CardA1);

	TestTrue(TEXT("Card is available for preview"), Result.bCanPlay);
	TestTrue(
		TEXT("Card remains available"),
		State.CardUsageState.PlayerACardUsageState.AvailableCardIds.Contains(
			MatchPlayActionPreviewTests::CardA1));
	TestFalse(
		TEXT("Card does not enter used cards"),
		State.CardUsageState.PlayerACardUsageState.UsedCardIds.Contains(
			MatchPlayActionPreviewTests::CardA1));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayActionPreviewDoesNotConsumeOpportunityTest,
	"FMCodex.CoreRules.MatchPlayActionPreview.PreviewDoesNotConsumeAttackOpportunity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayActionPreviewDoesNotConsumeOpportunityTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State =
		MatchPlayActionPreviewTests::MakeState();

	const FMatchPlayActionPreviewResult Result =
		FMatchPlayActionPreview::PreviewAction(
			State,
			EInitialTurnOrderPlayer::PlayerA,
			MatchPlayActionPreviewTests::CardA1);

	TestTrue(TEXT("Action is available"), Result.bCanPlay);
	TestEqual(
		TEXT("Used attack count is unchanged"),
		State.RuntimeState.PlayerAState.UsedAttackCount,
		2);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayActionPreviewDoesNotChangeScoreTest,
	"FMCodex.CoreRules.MatchPlayActionPreview.PreviewDoesNotChangeScore",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayActionPreviewDoesNotChangeScoreTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State =
		MatchPlayActionPreviewTests::MakeState();

	const FMatchPlayActionPreviewResult Result =
		FMatchPlayActionPreview::PreviewAction(
			State,
			EInitialTurnOrderPlayer::PlayerA,
			MatchPlayActionPreviewTests::CardA1);

	TestTrue(TEXT("Action is available"), Result.bCanPlay);
	TestEqual(TEXT("PlayerA score is unchanged"), State.RuntimeState.PlayerAState.Score, 3);
	TestEqual(TEXT("PlayerB score is unchanged"), State.RuntimeState.PlayerBState.Score, 2);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayActionPreviewNoOutcomeTest,
	"FMCodex.CoreRules.MatchPlayActionPreview.PreviewDoesNotResolveMatchEndOrResult",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayActionPreviewNoOutcomeTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayActionPreviewResult Result =
		FMatchPlayActionPreview::PreviewAction(
			MatchPlayActionPreviewTests::MakeState(),
			EInitialTurnOrderPlayer::PlayerA,
			MatchPlayActionPreviewTests::CardA1);

	TestFalse(
		TEXT("Preview does not synthesize a match-end state"),
		Result.StatusSnapshot.bHasStoredMatchEndState);
	TestFalse(
		TEXT("Preview does not synthesize a result type"),
		Result.StatusSnapshot.bHasStoredMatchResultType);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayActionPreviewRepeatedTest,
	"FMCodex.CoreRules.MatchPlayActionPreview.PreviewCanRunRepeatedlyConsistently",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayActionPreviewRepeatedTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State =
		MatchPlayActionPreviewTests::MakeState();
	const FMatchPlayActionPreviewResult First =
		FMatchPlayActionPreview::PreviewAction(
			State,
			EInitialTurnOrderPlayer::PlayerA,
			MatchPlayActionPreviewTests::CardA1);
	const FMatchPlayActionPreviewResult Second =
		FMatchPlayActionPreview::PreviewAction(
			State,
			EInitialTurnOrderPlayer::PlayerA,
			MatchPlayActionPreviewTests::CardA1);

	TestEqual(TEXT("Repeated bCanPlay is stable"), First.bCanPlay, Second.bCanPlay);
	TestEqual(TEXT("Repeated preview errors are stable"), First.ErrorCode, Second.ErrorCode);
	TestEqual(
		TEXT("Repeated status scores are stable"),
		First.StatusSnapshot.PlayerAScore,
		Second.StatusSnapshot.PlayerAScore);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayActionPreviewPlayerATest,
	"FMCodex.CoreRules.MatchPlayActionPreview.PlayerAPreviewUsesPlayerAState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayActionPreviewPlayerATest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayActionPreviewResult Result =
		FMatchPlayActionPreview::PreviewAction(
			MatchPlayActionPreviewTests::MakeState(),
			EInitialTurnOrderPlayer::PlayerA,
			MatchPlayActionPreviewTests::CardB1);

	TestFalse(TEXT("PlayerA cannot preview PlayerB-only card as playable"), Result.bCanPlay);
	TestEqual(
		TEXT("PlayerA-specific card state reports unavailable"),
		Result.AvailabilityResult.ErrorCode,
		EMatchPlayAvailabilityErrorCode::CardNotAvailable);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayActionPreviewPlayerBTest,
	"FMCodex.CoreRules.MatchPlayActionPreview.PlayerBPreviewUsesPlayerBState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayActionPreviewPlayerBTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayActionPreviewResult Result =
		FMatchPlayActionPreview::PreviewAction(
			MatchPlayActionPreviewTests::MakeState(
				EInitialTurnOrderPlayer::PlayerB),
			EInitialTurnOrderPlayer::PlayerB,
			MatchPlayActionPreviewTests::CardB1);

	TestTrue(TEXT("PlayerB card preview is playable"), Result.bCanPlay);
	TestEqual(
		TEXT("PlayerB is retained"),
		Result.AvailabilityResult.PlayerSide,
		EInitialTurnOrderPlayer::PlayerB);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayActionPreviewSharedCardTest,
	"FMCodex.CoreRules.MatchPlayActionPreview.SameCardIdAcrossPlayersHandledCorrectly",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayActionPreviewSharedCardTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayState PlayerAState =
		MatchPlayActionPreviewTests::MakeState();
	PlayerAState.CardUsageState.PlayerACardUsageState.AvailableCardIds.Add(
		MatchPlayActionPreviewTests::SharedCard);
	PlayerAState.CardUsageState.PlayerBCardUsageState.AvailableCardIds.Add(
		MatchPlayActionPreviewTests::SharedCard);
	FMatchPlayState PlayerBState = PlayerAState;
	PlayerBState.RuntimeState.CurrentAttackingPlayer =
		EInitialTurnOrderPlayer::PlayerB;

	const FMatchPlayActionPreviewResult PlayerAResult =
		FMatchPlayActionPreview::PreviewAction(
			PlayerAState,
			EInitialTurnOrderPlayer::PlayerA,
			MatchPlayActionPreviewTests::SharedCard);
	const FMatchPlayActionPreviewResult PlayerBResult =
		FMatchPlayActionPreview::PreviewAction(
			PlayerBState,
			EInitialTurnOrderPlayer::PlayerB,
			MatchPlayActionPreviewTests::SharedCard);

	TestTrue(TEXT("PlayerA shared card is playable"), PlayerAResult.bCanPlay);
	TestTrue(TEXT("PlayerB shared card is playable"), PlayerBResult.bCanPlay);
	return true;
}

#endif
