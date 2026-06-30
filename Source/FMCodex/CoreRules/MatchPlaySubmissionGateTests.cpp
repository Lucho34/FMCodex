#include "MatchPlaySubmissionGate.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace MatchPlaySubmissionGateTests
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

	FString LoadProductionSource()
	{
		FString Source;
		FFileHelper::LoadFileToString(
			Source,
			*(FPaths::ProjectDir()
				/ TEXT(
					"Source/FMCodex/CoreRules/MatchPlaySubmissionGate.cpp")));
		return Source;
	}

	FString LoadProductionHeader()
	{
		FString Source;
		FFileHelper::LoadFileToString(
			Source,
			*(FPaths::ProjectDir()
				/ TEXT(
					"Source/FMCodex/CoreRules/MatchPlaySubmissionGate.h")));
		return Source;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlaySubmissionGateCanSubmitTest,
	"FMCodex.CoreRules.MatchPlaySubmissionGate.CanSubmitWhenReadinessAndRequestValidationAllow",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlaySubmissionGateCanSubmitTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlaySubmissionGateResult Result =
		FMatchPlaySubmissionGate::CanSubmit(
			MatchPlaySubmissionGateTests::MakeState(),
			MatchPlaySubmissionGateTests::MakeRequest());

	TestTrue(TEXT("Gate can submit"), Result.bCanSubmit);
	TestEqual(
		TEXT("CanSubmit code is returned"),
		Result.Code,
		EMatchPlaySubmissionGateCode::CanSubmit);
	TestTrue(
		TEXT("Readiness result is retained"),
		Result.LoopReadinessResult.bCanAcceptExternalAttackRequest);
	TestTrue(
		TEXT("Validation report is retained"),
		Result.RequestValidationReport.bCanSubmitAttackRequest);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlaySubmissionGateReadinessBlocksTest,
	"FMCodex.CoreRules.MatchPlaySubmissionGate.ReadinessBlocksSubmission",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlaySubmissionGateReadinessBlocksTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayState State =
		MatchPlaySubmissionGateTests::MakeState();
	State.CardUsageState.PlayerACardUsageState.AvailableCardIds.Reset();

	const FMatchPlaySubmissionGateResult Result =
		FMatchPlaySubmissionGate::CanSubmit(
			State,
			MatchPlaySubmissionGateTests::MakeRequest());

	TestFalse(TEXT("Gate cannot submit"), Result.bCanSubmit);
	TestEqual(
		TEXT("Cannot-accept code is returned"),
		Result.Code,
		EMatchPlaySubmissionGateCode::CannotAcceptAttackRequestNow);
	TestEqual(
		TEXT("Readiness code is retained"),
		Result.LoopReadinessResult.Code,
		EMatchPlayLoopReadinessCode::CurrentAttackerHasNoAvailableCards);
	TestFalse(
		TEXT("Request validation is not run when readiness blocks"),
		Result.RequestValidationReport.bCanSubmitAttackRequest);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlaySubmissionGateInvalidRequestTest,
	"FMCodex.CoreRules.MatchPlaySubmissionGate.RequestValidationBlocksSubmission",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlaySubmissionGateInvalidRequestTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlaySubmissionGateResult Result =
		FMatchPlaySubmissionGate::CanSubmit(
			MatchPlaySubmissionGateTests::MakeState(),
			MatchPlaySubmissionGateTests::MakeRequest(
				EInitialTurnOrderPlayer::PlayerA,
				MatchPlaySubmissionGateTests::CardA1,
				false));

	TestFalse(TEXT("Gate cannot submit invalid request"), Result.bCanSubmit);
	TestEqual(
		TEXT("Invalid-request code is returned"),
		Result.Code,
		EMatchPlaySubmissionGateCode::InvalidRequest);
	TestTrue(
		TEXT("Readiness still allows request intake"),
		Result.LoopReadinessResult.bCanAcceptExternalAttackRequest);
	TestEqual(
		TEXT("Validation report reason is retained"),
		Result.RequestValidationReport.Code,
		EMatchPlayRequestValidationCode::InvalidRequest);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlaySubmissionGateFinishedTest,
	"FMCodex.CoreRules.MatchPlaySubmissionGate.MatchAlreadyFinishedCannotSubmit",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlaySubmissionGateFinishedTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlaySubmissionGateResult Result =
		FMatchPlaySubmissionGate::CanSubmit(
			MatchPlaySubmissionGateTests::MakeState(
				EInitialTurnOrderPlayer::PlayerA,
				1,
				1,
				1,
				1),
			MatchPlaySubmissionGateTests::MakeRequest());

	TestFalse(TEXT("Finished match cannot submit"), Result.bCanSubmit);
	TestEqual(
		TEXT("Finished code is returned"),
		Result.Code,
		EMatchPlaySubmissionGateCode::MatchAlreadyFinished);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlaySubmissionGateNoOpportunityTest,
	"FMCodex.CoreRules.MatchPlaySubmissionGate.NoRemainingOpportunityCannotSubmit",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlaySubmissionGateNoOpportunityTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlaySubmissionGateResult Result =
		FMatchPlaySubmissionGate::CanSubmit(
			MatchPlaySubmissionGateTests::MakeState(
				EInitialTurnOrderPlayer::PlayerA,
				1,
				1,
				2,
				0),
			MatchPlaySubmissionGateTests::MakeRequest());

	TestFalse(TEXT("No opportunity cannot submit"), Result.bCanSubmit);
	TestEqual(
		TEXT("No-opportunity code is returned"),
		Result.Code,
		EMatchPlaySubmissionGateCode::NoRemainingAttackOpportunity);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlaySubmissionGateNotCurrentTest,
	"FMCodex.CoreRules.MatchPlaySubmissionGate.NotCurrentAttackerCannotSubmit",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlaySubmissionGateNotCurrentTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlaySubmissionGateResult Result =
		FMatchPlaySubmissionGate::CanSubmit(
			MatchPlaySubmissionGateTests::MakeState(),
			MatchPlaySubmissionGateTests::MakeRequest(
				EInitialTurnOrderPlayer::PlayerB,
				MatchPlaySubmissionGateTests::CardB1));

	TestFalse(TEXT("Non-current attacker cannot submit"), Result.bCanSubmit);
	TestEqual(
		TEXT("Not-current code is returned"),
		Result.Code,
		EMatchPlaySubmissionGateCode::NotCurrentAttacker);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlaySubmissionGateEmptyCardTest,
	"FMCodex.CoreRules.MatchPlaySubmissionGate.EmptyCardIdCannotSubmit",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlaySubmissionGateEmptyCardTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlaySubmissionGateResult Result =
		FMatchPlaySubmissionGate::CanSubmit(
			MatchPlaySubmissionGateTests::MakeState(),
			MatchPlaySubmissionGateTests::MakeRequest(
				EInitialTurnOrderPlayer::PlayerA,
				NAME_None));

	TestFalse(TEXT("Empty CardId cannot submit"), Result.bCanSubmit);
	TestEqual(
		TEXT("Empty CardId code is returned"),
		Result.Code,
		EMatchPlaySubmissionGateCode::EmptyCardId);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlaySubmissionGateUnavailableCardTest,
	"FMCodex.CoreRules.MatchPlaySubmissionGate.UnavailableCardCannotSubmit",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlaySubmissionGateUnavailableCardTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlaySubmissionGateResult Result =
		FMatchPlaySubmissionGate::CanSubmit(
			MatchPlaySubmissionGateTests::MakeState(),
			MatchPlaySubmissionGateTests::MakeRequest(
				EInitialTurnOrderPlayer::PlayerA,
				MatchPlaySubmissionGateTests::MissingCard));

	TestFalse(TEXT("Unavailable card cannot submit"), Result.bCanSubmit);
	TestEqual(
		TEXT("Unavailable-card code is returned"),
		Result.Code,
		EMatchPlaySubmissionGateCode::CardNotAvailable);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlaySubmissionGateUsedCardTest,
	"FMCodex.CoreRules.MatchPlaySubmissionGate.UsedCardCannotSubmit",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlaySubmissionGateUsedCardTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayState State =
		MatchPlaySubmissionGateTests::MakeState();
	State.CardUsageState.PlayerACardUsageState.AvailableCardIds.Remove(
		MatchPlaySubmissionGateTests::CardA1);
	State.CardUsageState.PlayerACardUsageState.UsedCardIds.Add(
		MatchPlaySubmissionGateTests::CardA1);

	const FMatchPlaySubmissionGateResult Result =
		FMatchPlaySubmissionGate::CanSubmit(
			State,
			MatchPlaySubmissionGateTests::MakeRequest());

	TestFalse(TEXT("Used card cannot submit"), Result.bCanSubmit);
	TestEqual(
		TEXT("Used-card code is returned"),
		Result.Code,
		EMatchPlaySubmissionGateCode::CardAlreadyUsed);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlaySubmissionGateInputUnchangedTest,
	"FMCodex.CoreRules.MatchPlaySubmissionGate.CanSubmitDoesNotModifyInputState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlaySubmissionGateInputUnchangedTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State =
		MatchPlaySubmissionGateTests::MakeState();
	const FMatchPlayAttackRequest Request =
		MatchPlaySubmissionGateTests::MakeRequest();
	const TArray<FName> PlayerAAvailable =
		State.CardUsageState.PlayerACardUsageState.AvailableCardIds;

	const FMatchPlaySubmissionGateResult Result =
		FMatchPlaySubmissionGate::CanSubmit(State, Request);

	TestTrue(TEXT("Gate allows submission"), Result.bCanSubmit);
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
		MatchPlaySubmissionGateTests::CardA1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlaySubmissionGateNoStateChangeTest,
	"FMCodex.CoreRules.MatchPlaySubmissionGate.CanSubmitDoesNotConsumeOrChangeState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlaySubmissionGateNoStateChangeTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State =
		MatchPlaySubmissionGateTests::MakeState();

	const FMatchPlaySubmissionGateResult Result =
		FMatchPlaySubmissionGate::CanSubmit(
			State,
			MatchPlaySubmissionGateTests::MakeRequest());

	TestTrue(TEXT("Gate allows submission"), Result.bCanSubmit);
	TestTrue(
		TEXT("Requested card remains available"),
		State.CardUsageState.PlayerACardUsageState.AvailableCardIds.Contains(
			MatchPlaySubmissionGateTests::CardA1));
	TestFalse(
		TEXT("Requested card was not used"),
		State.CardUsageState.PlayerACardUsageState.UsedCardIds.Contains(
			MatchPlaySubmissionGateTests::CardA1));
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
	FMatchPlaySubmissionGateNoUpdatedStateTest,
	"FMCodex.CoreRules.MatchPlaySubmissionGate.ProductionCodeDoesNotReturnUpdatedState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlaySubmissionGateNoUpdatedStateTest::RunTest(
	const FString& Parameters)
{
	const FString Header =
		MatchPlaySubmissionGateTests::LoadProductionHeader();
	const FString Source =
		MatchPlaySubmissionGateTests::LoadProductionSource();

	TestFalse(
		TEXT("Header has no UpdatedState field"),
		Header.Contains(TEXT("UpdatedState")));
	TestFalse(
		TEXT("Source has no UpdatedState field"),
		Source.Contains(TEXT("UpdatedState")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlaySubmissionGateNoExecutionDependenciesTest,
	"FMCodex.CoreRules.MatchPlaySubmissionGate.ProductionCodeDoesNotCallExecutionFlows",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlaySubmissionGateNoExecutionDependenciesTest::RunTest(
	const FString& Parameters)
{
	const FString Source =
		MatchPlaySubmissionGateTests::LoadProductionSource();

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
	FMatchPlaySubmissionGateOnlyReadOnlyAggregationTest,
	"FMCodex.CoreRules.MatchPlaySubmissionGate.ProductionCodeOnlyCallsReadOnlyAggregation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlaySubmissionGateOnlyReadOnlyAggregationTest::RunTest(
	const FString& Parameters)
{
	const FString Source =
		MatchPlaySubmissionGateTests::LoadProductionSource();

	TestTrue(
		TEXT("Uses loop readiness"),
		Source.Contains(TEXT("FMatchPlayLoopReadiness::Evaluate")));
	TestTrue(
		TEXT("Uses request validation report"),
		Source.Contains(
			TEXT("FMatchPlayRequestValidationReportQuery::Validate")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlaySubmissionGateNoRandomnessTest,
	"FMCodex.CoreRules.MatchPlaySubmissionGate.ProductionCodeDoesNotUseRandomness",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlaySubmissionGateNoRandomnessTest::RunTest(
	const FString& Parameters)
{
	const FString Source =
		MatchPlaySubmissionGateTests::LoadProductionSource();

	TestFalse(TEXT("No Rand call"), Source.Contains(TEXT("Rand(")));
	TestFalse(TEXT("No RandRange call"), Source.Contains(TEXT("RandRange")));
	TestFalse(
		TEXT("No RandomStream usage"),
		Source.Contains(TEXT("RandomStream")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlaySubmissionGateNoExternalSystemsTest,
	"FMCodex.CoreRules.MatchPlaySubmissionGate.ProductionCodeDoesNotRequireExternalFeatureSystems",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlaySubmissionGateNoExternalSystemsTest::RunTest(
	const FString& Parameters)
{
	const FString Source =
		MatchPlaySubmissionGateTests::LoadProductionSource();

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
