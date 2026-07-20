#include "MatchPlayAttackRequest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

namespace MatchPlayAttackRequestTests
{
	const FName CardA1(TEXT("CardA1"));
	const FName CardA2(TEXT("CardA2"));
	const FName CardB1(TEXT("CardB1"));
	const FName SharedCard(TEXT("SharedCard"));
	const FName MissingCard(TEXT("MissingCard"));

	FMatchPlayState MakeState(
		const EInitialTurnOrderPlayer CurrentPlayer =
			EInitialTurnOrderPlayer::PlayerA,
		const int32 PlayerATotal = 3,
		const int32 PlayerAUsed = 0,
		const int32 PlayerBTotal = 3,
		const int32 PlayerBUsed = 0)
	{
		FMatchRuntimeState RuntimeState;
		RuntimeState.bIsInitialized = true;
		RuntimeState.PlayerAState.TotalAttackCount = PlayerATotal;
		RuntimeState.PlayerAState.UsedAttackCount = PlayerAUsed;
		RuntimeState.PlayerAState.Score = 2;
		RuntimeState.PlayerBState.TotalAttackCount = PlayerBTotal;
		RuntimeState.PlayerBState.UsedAttackCount = PlayerBUsed;
		RuntimeState.PlayerBState.Score = 1;
		RuntimeState.FirstPlayer = EInitialTurnOrderPlayer::PlayerA;
		RuntimeState.SecondPlayer = EInitialTurnOrderPlayer::PlayerB;
		RuntimeState.CurrentAttackingPlayer = CurrentPlayer;
		RuntimeState.OpeningResultSnapshot.bSuccess = true;
		RuntimeState.OpeningResultSnapshot.AttackCountResult
			.PlayerAAttackCount = PlayerATotal;
		RuntimeState.OpeningResultSnapshot.AttackCountResult
			.PlayerBAttackCount = PlayerBTotal;

		FMatchCardUsageState CardUsageState;
		CardUsageState.PlayerACardUsageState.AvailableCardIds =
			{ CardA1, CardA2 };
		CardUsageState.PlayerBCardUsageState.AvailableCardIds =
			{ CardB1 };
		FMatchPlayState State;
		State.RuntimeState = RuntimeState;
		State.CardUsageState = CardUsageState;
		FMatchPlayDeploymentSlotDefinition Slot;
		Slot.SlotId = TEXT("TestDeploymentSlot");
		Slot.NeutralSide = EMatchPlayNeutralSlotSide::NearPlayerA;
		State.DeploymentSlotCatalog.Slots.Add(Slot);
		return State;
	}

	FMatchPlayAttackRequest MakeRequest(
		const EInitialTurnOrderPlayer PlayerSide =
			EInitialTurnOrderPlayer::PlayerA,
		const FName CardId = CardA1,
		const bool bHasFormulaInput = true)
	{
		FMatchPlayAttackRequest Request;
		Request.PlayerSide = PlayerSide;
		Request.CardId = CardId;
		Request.bHasExternalFormulaInput = bHasFormulaInput;
		Request.FormulaInput.FormulaType = EFormulaType::Finishing;
		Request.FormulaInput.Attacker.BaseValue = 6.0f;
		Request.FormulaInput.Defender.BaseValue = 4.0f;
		Request.FormulaInput.AttackerPlayerId = TEXT("Attacker");
		Request.FormulaInput.DefenderPlayerId = TEXT("Defender");
		return Request;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAttackRequestValidTest,
	"FMCodex.CoreRules.MatchPlayAttackRequest.ValidRequestWhenCardPlayableAndFormulaInputProvided",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAttackRequestValidTest::RunTest(const FString& Parameters)
{
	const FMatchPlayAttackRequestValidationResult Result =
		FMatchPlayAttackRequestValidator::ValidateRequest(
			MatchPlayAttackRequestTests::MakeState(),
			MatchPlayAttackRequestTests::MakeRequest());

	TestTrue(TEXT("Playable request with formula input is valid"), Result.bValid);
	TestTrue(TEXT("Valid request can play"), Result.bCanPlay);
	TestTrue(TEXT("Formula input presence is retained"), Result.bHasExternalFormulaInput);
	TestEqual(TEXT("Valid request has no error"), Result.ErrorCode, EMatchPlayAttackRequestValidationErrorCode::None);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAttackRequestInvalidPlayerTest,
	"FMCodex.CoreRules.MatchPlayAttackRequest.InvalidWhenRequestPlayerInvalid",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAttackRequestInvalidPlayerTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayAttackRequestValidationResult Result =
		FMatchPlayAttackRequestValidator::ValidateRequest(
			MatchPlayAttackRequestTests::MakeState(),
			MatchPlayAttackRequestTests::MakeRequest(
				EInitialTurnOrderPlayer::None));

	TestFalse(TEXT("Invalid player request fails"), Result.bValid);
	TestEqual(
		TEXT("Invalid request player error is returned"),
		Result.ErrorCode,
		EMatchPlayAttackRequestValidationErrorCode::InvalidRequestPlayer);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAttackRequestInvalidCardTest,
	"FMCodex.CoreRules.MatchPlayAttackRequest.InvalidWhenCardIdInvalid",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAttackRequestInvalidCardTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayAttackRequestValidationResult Result =
		FMatchPlayAttackRequestValidator::ValidateRequest(
			MatchPlayAttackRequestTests::MakeState(),
			MatchPlayAttackRequestTests::MakeRequest(
				EInitialTurnOrderPlayer::PlayerA,
				NAME_None));

	TestFalse(TEXT("None CardId request fails"), Result.bValid);
	TestEqual(
		TEXT("Invalid CardId error is returned"),
		Result.ErrorCode,
		EMatchPlayAttackRequestValidationErrorCode::InvalidCardId);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAttackRequestNotCurrentTest,
	"FMCodex.CoreRules.MatchPlayAttackRequest.InvalidWhenNotCurrentAttacker",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAttackRequestNotCurrentTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayAttackRequestValidationResult Result =
		FMatchPlayAttackRequestValidator::ValidateRequest(
			MatchPlayAttackRequestTests::MakeState(),
			MatchPlayAttackRequestTests::MakeRequest(
				EInitialTurnOrderPlayer::PlayerB,
				MatchPlayAttackRequestTests::CardB1));

	TestFalse(TEXT("Non-current attacker request fails"), Result.bValid);
	TestEqual(
		TEXT("Request maps to card not playable"),
		Result.ErrorCode,
		EMatchPlayAttackRequestValidationErrorCode::CardNotPlayable);
	TestEqual(
		TEXT("Availability reason is retained"),
		Result.AvailabilityResult.ErrorCode,
		EMatchPlayAvailabilityErrorCode::NotCurrentAttacker);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAttackRequestNoOpportunityTest,
	"FMCodex.CoreRules.MatchPlayAttackRequest.InvalidWhenNoRemainingAttackOpportunity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAttackRequestNoOpportunityTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayAttackRequestValidationResult Result =
		FMatchPlayAttackRequestValidator::ValidateRequest(
			MatchPlayAttackRequestTests::MakeState(
				EInitialTurnOrderPlayer::PlayerA,
				1,
				1,
				2,
				0),
			MatchPlayAttackRequestTests::MakeRequest());

	TestFalse(TEXT("Request without opportunity fails"), Result.bValid);
	TestEqual(
		TEXT("No-opportunity reason is retained"),
		Result.AvailabilityResult.ErrorCode,
		EMatchPlayAvailabilityErrorCode::NoRemainingAttackOpportunity);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAttackRequestUnavailableCardTest,
	"FMCodex.CoreRules.MatchPlayAttackRequest.InvalidWhenCardUnavailable",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAttackRequestUnavailableCardTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayAttackRequestValidationResult Result =
		FMatchPlayAttackRequestValidator::ValidateRequest(
			MatchPlayAttackRequestTests::MakeState(),
			MatchPlayAttackRequestTests::MakeRequest(
				EInitialTurnOrderPlayer::PlayerA,
				MatchPlayAttackRequestTests::MissingCard));

	TestFalse(TEXT("Unavailable card request fails"), Result.bValid);
	TestEqual(
		TEXT("Unavailable-card reason is retained"),
		Result.AvailabilityResult.ErrorCode,
		EMatchPlayAvailabilityErrorCode::CardNotAvailable);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAttackRequestUsedCardTest,
	"FMCodex.CoreRules.MatchPlayAttackRequest.InvalidWhenCardAlreadyUsed",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAttackRequestUsedCardTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayState State =
		MatchPlayAttackRequestTests::MakeState();
	State.CardUsageState.PlayerACardUsageState.AvailableCardIds.Remove(
		MatchPlayAttackRequestTests::CardA1);
	State.CardUsageState.PlayerACardUsageState.UsedCardIds.Add(
		MatchPlayAttackRequestTests::CardA1);

	const FMatchPlayAttackRequestValidationResult Result =
		FMatchPlayAttackRequestValidator::ValidateRequest(
			State,
			MatchPlayAttackRequestTests::MakeRequest());

	TestFalse(TEXT("Used card request fails"), Result.bValid);
	TestEqual(
		TEXT("Already-used reason is retained"),
		Result.AvailabilityResult.ErrorCode,
		EMatchPlayAvailabilityErrorCode::CardAlreadyUsed);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAttackRequestMissingFormulaTest,
	"FMCodex.CoreRules.MatchPlayAttackRequest.InvalidWhenFormulaInputMissing",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAttackRequestMissingFormulaTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayAttackRequestValidationResult Result =
		FMatchPlayAttackRequestValidator::ValidateRequest(
			MatchPlayAttackRequestTests::MakeState(),
			MatchPlayAttackRequestTests::MakeRequest(
				EInitialTurnOrderPlayer::PlayerA,
				MatchPlayAttackRequestTests::CardA1,
				false));

	TestFalse(TEXT("Request without formula input fails"), Result.bValid);
	TestFalse(TEXT("Formula input absence is retained"), Result.bHasExternalFormulaInput);
	TestEqual(
		TEXT("Missing formula input error is returned"),
		Result.ErrorCode,
		EMatchPlayAttackRequestValidationErrorCode::MissingFormulaInput);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAttackRequestAvailabilityResultTest,
	"FMCodex.CoreRules.MatchPlayAttackRequest.ValidationIncludesAvailabilityResult",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAttackRequestAvailabilityResultTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayAttackRequestValidationResult Result =
		FMatchPlayAttackRequestValidator::ValidateRequest(
			MatchPlayAttackRequestTests::MakeState(),
			MatchPlayAttackRequestTests::MakeRequest(
				EInitialTurnOrderPlayer::PlayerA,
				MatchPlayAttackRequestTests::MissingCard));

	TestFalse(TEXT("Unavailable request fails"), Result.bValid);
	TestEqual(
		TEXT("Availability CardId is retained"),
		Result.AvailabilityResult.CardId,
		MatchPlayAttackRequestTests::MissingCard);
	TestEqual(
		TEXT("Availability error is retained"),
		Result.AvailabilityResult.ErrorCode,
		EMatchPlayAvailabilityErrorCode::CardNotAvailable);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAttackRequestActionPreviewTest,
	"FMCodex.CoreRules.MatchPlayAttackRequest.ValidationCanUseActionPreview",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAttackRequestActionPreviewTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayAttackRequestValidationResult Result =
		FMatchPlayAttackRequestValidator::ValidateRequest(
			MatchPlayAttackRequestTests::MakeState(),
			MatchPlayAttackRequestTests::MakeRequest());

	TestTrue(TEXT("Request is valid"), Result.bValid);
	TestTrue(
		TEXT("Action preview is retained"),
		Result.ActionPreviewResult.bPreviewAvailable);
	TestEqual(
		TEXT("Action preview retains current attacker"),
		Result.ActionPreviewResult.StatusSnapshot.CurrentAttackingPlayer,
		EInitialTurnOrderPlayer::PlayerA);
	TestTrue(
		TEXT("Action preview retains external formula requirement"),
		Result.ActionPreviewResult.bRequiresExternalFormulaInput);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAttackRequestInputUnchangedTest,
	"FMCodex.CoreRules.MatchPlayAttackRequest.ValidationDoesNotModifyInput",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAttackRequestInputUnchangedTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State =
		MatchPlayAttackRequestTests::MakeState();
	const FMatchPlayAttackRequest Request =
		MatchPlayAttackRequestTests::MakeRequest();
	const TArray<FName> PlayerAAvailable =
		State.CardUsageState.PlayerACardUsageState.AvailableCardIds;

	const FMatchPlayAttackRequestValidationResult Result =
		FMatchPlayAttackRequestValidator::ValidateRequest(State, Request);

	TestTrue(TEXT("Request validates"), Result.bValid);
	TestEqual(TEXT("Input score is unchanged"), State.RuntimeState.PlayerAState.Score, 2);
	TestEqual(TEXT("Input used attacks are unchanged"), State.RuntimeState.PlayerAState.UsedAttackCount, 0);
	TestTrue(
		TEXT("Input available cards are unchanged"),
		State.CardUsageState.PlayerACardUsageState.AvailableCardIds
			== PlayerAAvailable);
	TestEqual(TEXT("Request CardId is unchanged"), Request.CardId, MatchPlayAttackRequestTests::CardA1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAttackRequestDoesNotConsumeCardTest,
	"FMCodex.CoreRules.MatchPlayAttackRequest.ValidationDoesNotConsumeCard",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAttackRequestDoesNotConsumeCardTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State =
		MatchPlayAttackRequestTests::MakeState();

	const FMatchPlayAttackRequestValidationResult Result =
		FMatchPlayAttackRequestValidator::ValidateRequest(
			State,
			MatchPlayAttackRequestTests::MakeRequest());

	TestTrue(TEXT("Request validates"), Result.bValid);
	TestTrue(
		TEXT("Requested card remains available"),
		State.CardUsageState.PlayerACardUsageState.AvailableCardIds.Contains(
			MatchPlayAttackRequestTests::CardA1));
	TestFalse(
		TEXT("Requested card does not enter used cards"),
		State.CardUsageState.PlayerACardUsageState.UsedCardIds.Contains(
			MatchPlayAttackRequestTests::CardA1));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAttackRequestDoesNotConsumeOpportunityTest,
	"FMCodex.CoreRules.MatchPlayAttackRequest.ValidationDoesNotConsumeAttackOpportunity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAttackRequestDoesNotConsumeOpportunityTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State =
		MatchPlayAttackRequestTests::MakeState();

	const FMatchPlayAttackRequestValidationResult Result =
		FMatchPlayAttackRequestValidator::ValidateRequest(
			State,
			MatchPlayAttackRequestTests::MakeRequest());

	TestTrue(TEXT("Request validates"), Result.bValid);
	TestEqual(
		TEXT("Used attack count remains unchanged"),
		State.RuntimeState.PlayerAState.UsedAttackCount,
		0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAttackRequestDoesNotChangeScoreTest,
	"FMCodex.CoreRules.MatchPlayAttackRequest.ValidationDoesNotChangeScore",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAttackRequestDoesNotChangeScoreTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State =
		MatchPlayAttackRequestTests::MakeState();

	const FMatchPlayAttackRequestValidationResult Result =
		FMatchPlayAttackRequestValidator::ValidateRequest(
			State,
			MatchPlayAttackRequestTests::MakeRequest());

	TestTrue(TEXT("Request validates"), Result.bValid);
	TestEqual(TEXT("PlayerA score remains unchanged"), State.RuntimeState.PlayerAState.Score, 2);
	TestEqual(TEXT("PlayerB score remains unchanged"), State.RuntimeState.PlayerBState.Score, 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAttackRequestNoAttackFlowTest,
	"FMCodex.CoreRules.MatchPlayAttackRequest.ValidationDoesNotCallAttackFlow",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAttackRequestNoAttackFlowTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State =
		MatchPlayAttackRequestTests::MakeState();
	const FMatchPlayAttackRequestValidationResult Result =
		FMatchPlayAttackRequestValidator::ValidateRequest(
			State,
			MatchPlayAttackRequestTests::MakeRequest());

	TestTrue(TEXT("Validation succeeds without executing attack"), Result.bValid);
	TestEqual(TEXT("Score is not updated"), State.RuntimeState.PlayerAState.Score, 2);
	TestEqual(TEXT("Attack opportunity is not consumed"), State.RuntimeState.PlayerAState.UsedAttackCount, 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAttackRequestNoFormulaFlowTest,
	"FMCodex.CoreRules.MatchPlayAttackRequest.ValidationDoesNotCallFormulaFlow",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAttackRequestNoFormulaFlowTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayAttackRequest Request =
		MatchPlayAttackRequestTests::MakeRequest();
	const FMatchPlayAttackRequestValidationResult Result =
		FMatchPlayAttackRequestValidator::ValidateRequest(
			MatchPlayAttackRequestTests::MakeState(),
			Request);

	TestTrue(TEXT("Validation succeeds with supplied input"), Result.bValid);
	TestEqual(
		TEXT("Formula input is copied without resolution"),
		Result.AttackRequest.FormulaInput.Attacker.BaseValue,
		Request.FormulaInput.Attacker.BaseValue);
	TestEqual(
		TEXT("Formula type is unchanged"),
		Result.AttackRequest.FormulaInput.FormulaType,
		EFormulaType::Finishing);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAttackRequestPlayerATest,
	"FMCodex.CoreRules.MatchPlayAttackRequest.PlayerARequestUsesPlayerAState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAttackRequestPlayerATest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayAttackRequestValidationResult Result =
		FMatchPlayAttackRequestValidator::ValidateRequest(
			MatchPlayAttackRequestTests::MakeState(),
			MatchPlayAttackRequestTests::MakeRequest(
				EInitialTurnOrderPlayer::PlayerA,
				MatchPlayAttackRequestTests::CardB1));

	TestFalse(TEXT("PlayerA cannot request PlayerB-only card"), Result.bValid);
	TestEqual(
		TEXT("PlayerA availability reports unavailable"),
		Result.AvailabilityResult.ErrorCode,
		EMatchPlayAvailabilityErrorCode::CardNotAvailable);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAttackRequestPlayerBTest,
	"FMCodex.CoreRules.MatchPlayAttackRequest.PlayerBRequestUsesPlayerBState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAttackRequestPlayerBTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayAttackRequestValidationResult Result =
		FMatchPlayAttackRequestValidator::ValidateRequest(
			MatchPlayAttackRequestTests::MakeState(
				EInitialTurnOrderPlayer::PlayerB),
			MatchPlayAttackRequestTests::MakeRequest(
				EInitialTurnOrderPlayer::PlayerB,
				MatchPlayAttackRequestTests::CardB1));

	TestTrue(TEXT("PlayerB request uses PlayerB available cards"), Result.bValid);
	TestEqual(
		TEXT("PlayerB is retained"),
		Result.AvailabilityResult.PlayerSide,
		EInitialTurnOrderPlayer::PlayerB);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAttackRequestSharedCardTest,
	"FMCodex.CoreRules.MatchPlayAttackRequest.SameCardIdAcrossPlayersHandledCorrectly",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAttackRequestSharedCardTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayState PlayerAState =
		MatchPlayAttackRequestTests::MakeState();
	PlayerAState.CardUsageState.PlayerACardUsageState.AvailableCardIds.Add(
		MatchPlayAttackRequestTests::SharedCard);
	PlayerAState.CardUsageState.PlayerBCardUsageState.AvailableCardIds.Add(
		MatchPlayAttackRequestTests::SharedCard);
	FMatchPlayState PlayerBState = PlayerAState;
	PlayerBState.RuntimeState.CurrentAttackingPlayer =
		EInitialTurnOrderPlayer::PlayerB;

	const FMatchPlayAttackRequestValidationResult PlayerAResult =
		FMatchPlayAttackRequestValidator::ValidateRequest(
			PlayerAState,
			MatchPlayAttackRequestTests::MakeRequest(
				EInitialTurnOrderPlayer::PlayerA,
				MatchPlayAttackRequestTests::SharedCard));
	const FMatchPlayAttackRequestValidationResult PlayerBResult =
		FMatchPlayAttackRequestValidator::ValidateRequest(
			PlayerBState,
			MatchPlayAttackRequestTests::MakeRequest(
				EInitialTurnOrderPlayer::PlayerB,
				MatchPlayAttackRequestTests::SharedCard));

	TestTrue(TEXT("PlayerA shared card request is valid"), PlayerAResult.bValid);
	TestTrue(TEXT("PlayerB shared card request is valid"), PlayerBResult.bValid);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAttackRequestRepeatedTest,
	"FMCodex.CoreRules.MatchPlayAttackRequest.RepeatedValidationIsConsistent",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAttackRequestRepeatedTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State =
		MatchPlayAttackRequestTests::MakeState();
	const FMatchPlayAttackRequest Request =
		MatchPlayAttackRequestTests::MakeRequest();

	const FMatchPlayAttackRequestValidationResult First =
		FMatchPlayAttackRequestValidator::ValidateRequest(State, Request);
	const FMatchPlayAttackRequestValidationResult Second =
		FMatchPlayAttackRequestValidator::ValidateRequest(State, Request);

	TestEqual(TEXT("Repeated validity is stable"), First.bValid, Second.bValid);
	TestEqual(TEXT("Repeated bCanPlay is stable"), First.bCanPlay, Second.bCanPlay);
	TestEqual(TEXT("Repeated error code is stable"), First.ErrorCode, Second.ErrorCode);
	return true;
}

#endif
