#include "MatchPlayRequestValidationReport.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace MatchPlayRequestValidationReportTests
{
	const FName CardA1(TEXT("CardA1"));
	const FName CardA2(TEXT("CardA2"));
	const FName CardB1(TEXT("CardB1"));
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
		return FMatchPlayState::Create(RuntimeState, CardUsageState);
	}

	FMatchPlayAttackRequest MakeRequest(
		const EInitialTurnOrderPlayer PlayerSide =
			EInitialTurnOrderPlayer::PlayerA,
		const FName CardId = CardA1)
	{
		FMatchPlayAttackRequest Request;
		Request.PlayerSide = PlayerSide;
		Request.CardId = CardId;
		Request.bHasExternalFormulaInput = true;
		Request.FormulaInput.FormulaType = EFormulaType::Finishing;
		Request.FormulaInput.Attacker.BaseValue = 6.0f;
		Request.FormulaInput.Defender.BaseValue = 4.0f;
		Request.FormulaInput.AttackerPlayerId = TEXT("Attacker");
		Request.FormulaInput.DefenderPlayerId = TEXT("Defender");
		return Request;
	}

	FString LoadProductionSource()
	{
		FString Source;
		FFileHelper::LoadFileToString(
			Source,
			*(FPaths::ProjectDir()
				/ TEXT(
					"Source/FMCodex/CoreRules/MatchPlayRequestValidationReport.cpp")));
		return Source;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayRequestValidationReportCannotAcceptNowTest,
	"FMCodex.CoreRules.MatchPlayRequestValidationReport.UninitializedStateCannotAcceptRequestNow",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayRequestValidationReportCannotAcceptNowTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State;

	const FMatchPlayRequestValidationReport Report =
		FMatchPlayRequestValidationReportQuery::Validate(
			State,
			MatchPlayRequestValidationReportTests::MakeRequest());

	TestFalse(TEXT("Uninitialized state is invalid"), Report.bIsValid);
	TestFalse(
		TEXT("Uninitialized state cannot submit"),
		Report.bCanSubmitAttackRequest);
	TestEqual(
		TEXT("Cannot-accept-now code is returned"),
		Report.Code,
		EMatchPlayRequestValidationCode::CannotAcceptAttackRequestNow);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayRequestValidationReportValidTest,
	"FMCodex.CoreRules.MatchPlayRequestValidationReport.ValidAttackRequestCanSubmit",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayRequestValidationReportValidTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayRequestValidationReport Report =
		FMatchPlayRequestValidationReportQuery::Validate(
			MatchPlayRequestValidationReportTests::MakeState(),
			MatchPlayRequestValidationReportTests::MakeRequest());

	TestTrue(TEXT("Report is valid"), Report.bIsValid);
	TestTrue(
		TEXT("Report can submit attack request"),
		Report.bCanSubmitAttackRequest);
	TestEqual(
		TEXT("Valid code is returned"),
		Report.Code,
		EMatchPlayRequestValidationCode::Valid);
	TestTrue(
		TEXT("Loop readiness is retained"),
		Report.LoopReadinessResult.bCanAcceptExternalAttackRequest);
	TestTrue(
		TEXT("Request validation is retained"),
		Report.RequestValidationResult.bValid);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayRequestValidationReportFinishedTest,
	"FMCodex.CoreRules.MatchPlayRequestValidationReport.MatchAlreadyFinishedCannotSubmit",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayRequestValidationReportFinishedTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayRequestValidationReport Report =
		FMatchPlayRequestValidationReportQuery::Validate(
			MatchPlayRequestValidationReportTests::MakeState(
				EInitialTurnOrderPlayer::PlayerA,
				1,
				1,
				1,
				1),
			MatchPlayRequestValidationReportTests::MakeRequest());

	TestFalse(TEXT("Finished match is invalid"), Report.bIsValid);
	TestFalse(
		TEXT("Finished match cannot submit"),
		Report.bCanSubmitAttackRequest);
	TestEqual(
		TEXT("Finished code is returned"),
		Report.Code,
		EMatchPlayRequestValidationCode::MatchAlreadyFinished);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayRequestValidationReportNoOpportunityTest,
	"FMCodex.CoreRules.MatchPlayRequestValidationReport.NoRemainingOpportunityCannotSubmit",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayRequestValidationReportNoOpportunityTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayRequestValidationReport Report =
		FMatchPlayRequestValidationReportQuery::Validate(
			MatchPlayRequestValidationReportTests::MakeState(
				EInitialTurnOrderPlayer::PlayerA,
				1,
				1,
				2,
				0),
			MatchPlayRequestValidationReportTests::MakeRequest());

	TestFalse(TEXT("No opportunity is invalid"), Report.bIsValid);
	TestFalse(
		TEXT("No opportunity cannot submit"),
		Report.bCanSubmitAttackRequest);
	TestEqual(
		TEXT("No opportunity code is returned"),
		Report.Code,
		EMatchPlayRequestValidationCode::NoRemainingAttackOpportunity);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayRequestValidationReportNotCurrentTest,
	"FMCodex.CoreRules.MatchPlayRequestValidationReport.NotCurrentAttackerCannotSubmit",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayRequestValidationReportNotCurrentTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayRequestValidationReport Report =
		FMatchPlayRequestValidationReportQuery::Validate(
			MatchPlayRequestValidationReportTests::MakeState(),
			MatchPlayRequestValidationReportTests::MakeRequest(
				EInitialTurnOrderPlayer::PlayerB,
				MatchPlayRequestValidationReportTests::CardB1));

	TestFalse(TEXT("Non-current attacker is invalid"), Report.bIsValid);
	TestFalse(
		TEXT("Non-current attacker cannot submit"),
		Report.bCanSubmitAttackRequest);
	TestEqual(
		TEXT("Not-current code is returned"),
		Report.Code,
		EMatchPlayRequestValidationCode::NotCurrentAttacker);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayRequestValidationReportEmptyCardTest,
	"FMCodex.CoreRules.MatchPlayRequestValidationReport.EmptyCardIdCannotSubmit",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayRequestValidationReportEmptyCardTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayRequestValidationReport Report =
		FMatchPlayRequestValidationReportQuery::Validate(
			MatchPlayRequestValidationReportTests::MakeState(),
			MatchPlayRequestValidationReportTests::MakeRequest(
				EInitialTurnOrderPlayer::PlayerA,
				NAME_None));

	TestFalse(TEXT("Empty CardId is invalid"), Report.bIsValid);
	TestFalse(
		TEXT("Empty CardId cannot submit"),
		Report.bCanSubmitAttackRequest);
	TestEqual(
		TEXT("Empty CardId code is returned"),
		Report.Code,
		EMatchPlayRequestValidationCode::EmptyCardId);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayRequestValidationReportUnavailableCardTest,
	"FMCodex.CoreRules.MatchPlayRequestValidationReport.UnavailableCardCannotSubmit",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayRequestValidationReportUnavailableCardTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayRequestValidationReport Report =
		FMatchPlayRequestValidationReportQuery::Validate(
			MatchPlayRequestValidationReportTests::MakeState(),
			MatchPlayRequestValidationReportTests::MakeRequest(
				EInitialTurnOrderPlayer::PlayerA,
				MatchPlayRequestValidationReportTests::MissingCard));

	TestFalse(TEXT("Unavailable card is invalid"), Report.bIsValid);
	TestFalse(
		TEXT("Unavailable card cannot submit"),
		Report.bCanSubmitAttackRequest);
	TestEqual(
		TEXT("Unavailable-card code is returned"),
		Report.Code,
		EMatchPlayRequestValidationCode::CardNotAvailable);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayRequestValidationReportUsedCardTest,
	"FMCodex.CoreRules.MatchPlayRequestValidationReport.UsedCardCannotSubmit",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayRequestValidationReportUsedCardTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayState State =
		MatchPlayRequestValidationReportTests::MakeState();
	State.CardUsageState.PlayerACardUsageState.AvailableCardIds.Remove(
		MatchPlayRequestValidationReportTests::CardA1);
	State.CardUsageState.PlayerACardUsageState.UsedCardIds.Add(
		MatchPlayRequestValidationReportTests::CardA1);

	const FMatchPlayRequestValidationReport Report =
		FMatchPlayRequestValidationReportQuery::Validate(
			State,
			MatchPlayRequestValidationReportTests::MakeRequest());

	TestFalse(TEXT("Used card is invalid"), Report.bIsValid);
	TestFalse(
		TEXT("Used card cannot submit"),
		Report.bCanSubmitAttackRequest);
	TestEqual(
		TEXT("Used-card code is returned"),
		Report.Code,
		EMatchPlayRequestValidationCode::CardAlreadyUsed);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayRequestValidationReportInvalidRequestTest,
	"FMCodex.CoreRules.MatchPlayRequestValidationReport.MissingFormulaInputIsInvalidRequest",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayRequestValidationReportInvalidRequestTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayAttackRequest Request =
		MatchPlayRequestValidationReportTests::MakeRequest();
	Request.bHasExternalFormulaInput = false;

	const FMatchPlayRequestValidationReport Report =
		FMatchPlayRequestValidationReportQuery::Validate(
			MatchPlayRequestValidationReportTests::MakeState(),
			Request);

	TestFalse(TEXT("Missing formula input is invalid"), Report.bIsValid);
	TestFalse(
		TEXT("Missing formula input cannot submit"),
		Report.bCanSubmitAttackRequest);
	TestEqual(
		TEXT("Invalid-request code is returned"),
		Report.Code,
		EMatchPlayRequestValidationCode::InvalidRequest);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayRequestValidationReportInvalidPlayerTest,
	"FMCodex.CoreRules.MatchPlayRequestValidationReport.InvalidAttackingPlayerCannotSubmit",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayRequestValidationReportInvalidPlayerTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayRequestValidationReport Report =
		FMatchPlayRequestValidationReportQuery::Validate(
			MatchPlayRequestValidationReportTests::MakeState(),
			MatchPlayRequestValidationReportTests::MakeRequest(
				EInitialTurnOrderPlayer::None));

	TestFalse(TEXT("Invalid attacking player is invalid"), Report.bIsValid);
	TestFalse(
		TEXT("Invalid attacking player cannot submit"),
		Report.bCanSubmitAttackRequest);
	TestEqual(
		TEXT("Invalid attacking player code is returned"),
		Report.Code,
		EMatchPlayRequestValidationCode::InvalidAttackingPlayer);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayRequestValidationReportInputUnchangedTest,
	"FMCodex.CoreRules.MatchPlayRequestValidationReport.ValidateDoesNotModifyInputState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayRequestValidationReportInputUnchangedTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State =
		MatchPlayRequestValidationReportTests::MakeState();
	const FMatchPlayAttackRequest Request =
		MatchPlayRequestValidationReportTests::MakeRequest();
	const TArray<FName> PlayerAAvailable =
		State.CardUsageState.PlayerACardUsageState.AvailableCardIds;

	const FMatchPlayRequestValidationReport Report =
		FMatchPlayRequestValidationReportQuery::Validate(State, Request);

	TestTrue(TEXT("Validation succeeds"), Report.bIsValid);
	TestEqual(
		TEXT("PlayerA score remains unchanged"),
		State.RuntimeState.PlayerAState.Score,
		2);
	TestEqual(
		TEXT("PlayerB score remains unchanged"),
		State.RuntimeState.PlayerBState.Score,
		1);
	TestEqual(
		TEXT("Used attack count remains unchanged"),
		State.RuntimeState.PlayerAState.UsedAttackCount,
		0);
	TestTrue(
		TEXT("Available cards remain unchanged"),
		State.CardUsageState.PlayerACardUsageState.AvailableCardIds
			== PlayerAAvailable);
	TestEqual(
		TEXT("Request CardId remains unchanged"),
		Request.CardId,
		MatchPlayRequestValidationReportTests::CardA1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayRequestValidationReportNoStateChangeTest,
	"FMCodex.CoreRules.MatchPlayRequestValidationReport.ValidateDoesNotExecuteOrChangeState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayRequestValidationReportNoStateChangeTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State =
		MatchPlayRequestValidationReportTests::MakeState();

	const FMatchPlayRequestValidationReport Report =
		FMatchPlayRequestValidationReportQuery::Validate(
			State,
			MatchPlayRequestValidationReportTests::MakeRequest());

	TestTrue(TEXT("Validation succeeds"), Report.bIsValid);
	TestTrue(
		TEXT("Requested card remains available"),
		State.CardUsageState.PlayerACardUsageState.AvailableCardIds.Contains(
			MatchPlayRequestValidationReportTests::CardA1));
	TestFalse(
		TEXT("Requested card was not used"),
		State.CardUsageState.PlayerACardUsageState.UsedCardIds.Contains(
			MatchPlayRequestValidationReportTests::CardA1));
	TestEqual(
		TEXT("Attack opportunity was not consumed"),
		State.RuntimeState.PlayerAState.UsedAttackCount,
		0);
	TestEqual(
		TEXT("Score was not changed"),
		State.RuntimeState.PlayerAState.Score,
		2);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayRequestValidationReportNoExecutionDependenciesTest,
	"FMCodex.CoreRules.MatchPlayRequestValidationReport.ProductionCodeDoesNotCallExecutionFlows",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayRequestValidationReportNoExecutionDependenciesTest::RunTest(
	const FString& Parameters)
{
	const FString Source =
		MatchPlayRequestValidationReportTests::LoadProductionSource();

	TestFalse(
		TEXT("No AttackExecutor call"),
		Source.Contains(TEXT("FMatchPlayAttackExecutor::")));
	TestFalse(
		TEXT("No AttackStep call"),
		Source.Contains(TEXT("FMatchPlayAttackStep::")));
	TestFalse(
		TEXT("No MatchPlayAttackFlow call"),
		Source.Contains(TEXT("FMatchPlayAttackFlow::")));
	TestFalse(
		TEXT("No FormulaAttackFlow call"),
		Source.Contains(TEXT("FFormulaAttackFlow::")));
	TestFalse(
		TEXT("No FormulaResolver call"),
		Source.Contains(TEXT("UFormulaResolver::")));
	TestFalse(
		TEXT("No CardUsageResolver execution call"),
		Source.Contains(TEXT("FCardUsageResolver::")));
	TestFalse(
		TEXT("No PlayCardResolver execution call"),
		Source.Contains(TEXT("FPlayCardResolver::PlayCard")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayRequestValidationReportNoRandomnessTest,
	"FMCodex.CoreRules.MatchPlayRequestValidationReport.ProductionCodeDoesNotUseRandomness",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayRequestValidationReportNoRandomnessTest::RunTest(
	const FString& Parameters)
{
	const FString Source =
		MatchPlayRequestValidationReportTests::LoadProductionSource();

	TestFalse(TEXT("No Rand call"), Source.Contains(TEXT("Rand(")));
	TestFalse(TEXT("No RandRange call"), Source.Contains(TEXT("RandRange")));
	TestFalse(
		TEXT("No RandomStream usage"),
		Source.Contains(TEXT("RandomStream")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayRequestValidationReportNoExternalSystemsTest,
	"FMCodex.CoreRules.MatchPlayRequestValidationReport.ProductionCodeDoesNotRequireExternalFeatureSystems",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayRequestValidationReportNoExternalSystemsTest::RunTest(
	const FString& Parameters)
{
	const FString Source =
		MatchPlayRequestValidationReportTests::LoadProductionSource();

	TestFalse(TEXT("No skill system dependency"), Source.Contains(TEXT("Skill")));
	TestFalse(
		TEXT("No card database dependency"),
		Source.Contains(TEXT("Database")));
	TestFalse(TEXT("No UI dependency"), Source.Contains(TEXT("UI")));
	TestFalse(
		TEXT("No Blueprint dependency"),
		Source.Contains(TEXT("Blueprint")));
	return true;
}

#endif
