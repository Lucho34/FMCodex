#include "MatchPlayExternalAttackRequestPreflight.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace MatchPlayExternalAttackRequestPreflightTests
{
	const FName CardA1(TEXT("CardA1"));
	const FName CardA2(TEXT("CardA2"));
	const FName CardB1(TEXT("CardB1"));
	const FName MissingCard(TEXT("MissingCard"));

	FMatchPlayState MakeState(
		const EInitialTurnOrderPlayer CurrentPlayer =
			EInitialTurnOrderPlayer::PlayerA,
		const int32 HomeTotal = 3,
		const int32 HomeUsed = 0,
		const int32 AwayTotal = 3,
		const int32 AwayUsed = 0,
		const bool bHomeHasAvailableCards = true)
	{
		FMatchRuntimeState RuntimeState;
		RuntimeState.bIsInitialized = true;
		RuntimeState.PlayerAState.TotalAttackCount = HomeTotal;
		RuntimeState.PlayerAState.UsedAttackCount = HomeUsed;
		RuntimeState.PlayerAState.Score = 2;
		RuntimeState.PlayerBState.TotalAttackCount = AwayTotal;
		RuntimeState.PlayerBState.UsedAttackCount = AwayUsed;
		RuntimeState.PlayerBState.Score = 1;
		RuntimeState.FirstPlayer = EInitialTurnOrderPlayer::PlayerA;
		RuntimeState.SecondPlayer = EInitialTurnOrderPlayer::PlayerB;
		RuntimeState.CurrentAttackingPlayer = CurrentPlayer;
		RuntimeState.OpeningResultSnapshot.bSuccess = true;
		RuntimeState.OpeningResultSnapshot.AttackCountResult
			.PlayerAAttackCount = HomeTotal;
		RuntimeState.OpeningResultSnapshot.AttackCountResult
			.PlayerBAttackCount = AwayTotal;

		FMatchCardUsageState CardUsageState;
		if (bHomeHasAvailableCards)
		{
			CardUsageState.PlayerACardUsageState.AvailableCardIds =
				{ CardA1, CardA2 };
		}
		CardUsageState.PlayerBCardUsageState.AvailableCardIds =
			{ CardB1 };
		return FMatchPlayState::Create(RuntimeState, CardUsageState);
	}

	FMatchPlayAttackRequest MakeRequest(
		const EInitialTurnOrderPlayer Player =
			EInitialTurnOrderPlayer::PlayerA,
		const FName CardId = CardA1,
		const bool bHasFormulaInput = true)
	{
		FMatchPlayAttackRequest Request;
		Request.PlayerSide = Player;
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
					"Source/FMCodex/CoreRules/MatchPlayExternalAttackRequestPreflight.cpp")));
		return Source;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayExternalAttackRequestPreflightValidTest,
	"FMCodex.CoreRules.MatchPlayExternalAttackRequestPreflight.ValidRequestPasses",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayExternalAttackRequestPreflightValidTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayExternalAttackRequestPreflightView View =
		FMatchPlayExternalAttackRequestPreflight::Evaluate(
			MatchPlayExternalAttackRequestPreflightTests::MakeState(),
			MatchPlayExternalAttackRequestPreflightTests::MakeRequest());

	TestTrue(TEXT("State is ready"), View.bStateReadyForAttackRequest);
	TestTrue(TEXT("Gate accepts request"), View.bRequestAcceptedByGate);
	TestTrue(TEXT("Specific request can submit"), View.bCanSubmit);
	TestEqual(
		TEXT("Gate success code is retained"),
		View.GateCode,
		EMatchPlaySubmissionGateCode::CanSubmit);
	TestTrue(
		TEXT("Validation report is retained"),
		View.ValidationReport.bCanSubmitAttackRequest);
	TestTrue(
		TEXT("Successful preflight has no rejection reason"),
		View.CannotSubmitReason.IsEmpty());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayExternalAttackRequestPreflightInvalidReasonTest,
	"FMCodex.CoreRules.MatchPlayExternalAttackRequestPreflight.InvalidRequestReasonIsStable",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayExternalAttackRequestPreflightInvalidReasonTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State =
		MatchPlayExternalAttackRequestPreflightTests::MakeState();
	const FMatchPlayAttackRequest Request =
		MatchPlayExternalAttackRequestPreflightTests::MakeRequest(
			EInitialTurnOrderPlayer::PlayerA,
			MatchPlayExternalAttackRequestPreflightTests::CardA1,
			false);
	const FMatchPlayExternalAttackRequestPreflightView First =
		FMatchPlayExternalAttackRequestPreflight::Evaluate(
			State,
			Request);
	const FMatchPlayExternalAttackRequestPreflightView Second =
		FMatchPlayExternalAttackRequestPreflight::Evaluate(
			State,
			Request);

	TestFalse(TEXT("Invalid request fails"), First.bCanSubmit);
	TestEqual(
		TEXT("Invalid request code is retained"),
		First.GateCode,
		EMatchPlaySubmissionGateCode::InvalidRequest);
	TestFalse(
		TEXT("Failure reason is populated"),
		First.CannotSubmitReason.IsEmpty());
	TestEqual(
		TEXT("Failure reason is stable"),
		First.CannotSubmitReason,
		Second.CannotSubmitReason);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayExternalAttackRequestPreflightStateNotReadyTest,
	"FMCodex.CoreRules.MatchPlayExternalAttackRequestPreflight.StateNotReadyFails",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayExternalAttackRequestPreflightStateNotReadyTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayExternalAttackRequestPreflightView View =
		FMatchPlayExternalAttackRequestPreflight::Evaluate(
			MatchPlayExternalAttackRequestPreflightTests::MakeState(
				EInitialTurnOrderPlayer::PlayerA,
				3,
				0,
				3,
				0,
				false),
			MatchPlayExternalAttackRequestPreflightTests::MakeRequest());

	TestFalse(TEXT("State is not ready"), View.bStateReadyForAttackRequest);
	TestFalse(TEXT("Gate rejects request"), View.bRequestAcceptedByGate);
	TestFalse(TEXT("Preflight fails"), View.bCanSubmit);
	TestEqual(
		TEXT("State readiness reason is retained"),
		View.StateView.ReadinessCode,
		EMatchPlayLoopReadinessCode::CurrentAttackerHasNoAvailableCards);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayExternalAttackRequestPreflightWrongPlayerTest,
	"FMCodex.CoreRules.MatchPlayExternalAttackRequestPreflight.NonCurrentPlayerFails",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayExternalAttackRequestPreflightWrongPlayerTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayExternalAttackRequestPreflightView View =
		FMatchPlayExternalAttackRequestPreflight::Evaluate(
			MatchPlayExternalAttackRequestPreflightTests::MakeState(),
			MatchPlayExternalAttackRequestPreflightTests::MakeRequest(
				EInitialTurnOrderPlayer::PlayerB,
				MatchPlayExternalAttackRequestPreflightTests::CardB1));

	TestTrue(TEXT("State itself is ready"), View.bStateReadyForAttackRequest);
	TestFalse(TEXT("Wrong player request fails"), View.bCanSubmit);
	TestEqual(
		TEXT("Current attacker is retained"),
		View.CurrentAttackingPlayer,
		EInitialTurnOrderPlayer::PlayerA);
	TestEqual(
		TEXT("Request player is retained"),
		View.RequestPlayer,
		EInitialTurnOrderPlayer::PlayerB);
	TestEqual(
		TEXT("Wrong-player code is retained"),
		View.GateCode,
		EMatchPlaySubmissionGateCode::NotCurrentAttacker);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayExternalAttackRequestPreflightUnavailableCardTest,
	"FMCodex.CoreRules.MatchPlayExternalAttackRequestPreflight.UnavailableCardFails",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayExternalAttackRequestPreflightUnavailableCardTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayExternalAttackRequestPreflightView View =
		FMatchPlayExternalAttackRequestPreflight::Evaluate(
			MatchPlayExternalAttackRequestPreflightTests::MakeState(),
			MatchPlayExternalAttackRequestPreflightTests::MakeRequest(
				EInitialTurnOrderPlayer::PlayerA,
				MatchPlayExternalAttackRequestPreflightTests::MissingCard));

	TestTrue(TEXT("State itself is ready"), View.bStateReadyForAttackRequest);
	TestFalse(TEXT("Unavailable card request fails"), View.bCanSubmit);
	TestEqual(
		TEXT("CardId is retained"),
		View.CardId,
		MatchPlayExternalAttackRequestPreflightTests::MissingCard);
	TestEqual(
		TEXT("Unavailable-card code is retained"),
		View.GateCode,
		EMatchPlaySubmissionGateCode::CardNotAvailable);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayExternalAttackRequestPreflightFinishedTest,
	"FMCodex.CoreRules.MatchPlayExternalAttackRequestPreflight.FinishedMatchFails",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayExternalAttackRequestPreflightFinishedTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayExternalAttackRequestPreflightView View =
		FMatchPlayExternalAttackRequestPreflight::Evaluate(
			MatchPlayExternalAttackRequestPreflightTests::MakeState(
				EInitialTurnOrderPlayer::PlayerA,
				1,
				1,
				1,
				1),
			MatchPlayExternalAttackRequestPreflightTests::MakeRequest());

	TestTrue(TEXT("State view reports finished match"), View.StateView.bIsMatchFinished);
	TestFalse(TEXT("Finished state is not ready"), View.bStateReadyForAttackRequest);
	TestFalse(TEXT("Finished match preflight fails"), View.bCanSubmit);
	TestEqual(
		TEXT("Finished-match code is retained"),
		View.GateCode,
		EMatchPlaySubmissionGateCode::MatchAlreadyFinished);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayExternalAttackRequestPreflightInputUnchangedTest,
	"FMCodex.CoreRules.MatchPlayExternalAttackRequestPreflight.DoesNotModifyInputs",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayExternalAttackRequestPreflightInputUnchangedTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State =
		MatchPlayExternalAttackRequestPreflightTests::MakeState();
	const FMatchPlayAttackRequest Request =
		MatchPlayExternalAttackRequestPreflightTests::MakeRequest();
	const int32 HomeScore = State.RuntimeState.PlayerAState.Score;
	const int32 HomeUsedAttackCount =
		State.RuntimeState.PlayerAState.UsedAttackCount;
	const TArray<FName> HomeAvailable =
		State.CardUsageState.PlayerACardUsageState.AvailableCardIds;
	const FName RequestCardId = Request.CardId;
	const float AttackerValue =
		Request.FormulaInput.Attacker.BaseValue;

	const FMatchPlayExternalAttackRequestPreflightView View =
		FMatchPlayExternalAttackRequestPreflight::Evaluate(
			State,
			Request);

	TestTrue(TEXT("Preflight succeeds"), View.bCanSubmit);
	TestEqual(TEXT("Input score is unchanged"), State.RuntimeState.PlayerAState.Score, HomeScore);
	TestEqual(TEXT("Input opportunities are unchanged"), State.RuntimeState.PlayerAState.UsedAttackCount, HomeUsedAttackCount);
	TestTrue(TEXT("Input cards are unchanged"), State.CardUsageState.PlayerACardUsageState.AvailableCardIds == HomeAvailable);
	TestEqual(TEXT("Request CardId is unchanged"), Request.CardId, RequestCardId);
	TestEqual(TEXT("Request formula input is unchanged"), Request.FormulaInput.Attacker.BaseValue, AttackerValue);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayExternalAttackRequestPreflightNoConsumptionTest,
	"FMCodex.CoreRules.MatchPlayExternalAttackRequestPreflight.DoesNotConsumeOrAdvance",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayExternalAttackRequestPreflightNoConsumptionTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State =
		MatchPlayExternalAttackRequestPreflightTests::MakeState();

	const FMatchPlayExternalAttackRequestPreflightView View =
		FMatchPlayExternalAttackRequestPreflight::Evaluate(
			State,
			MatchPlayExternalAttackRequestPreflightTests::MakeRequest());

	TestTrue(TEXT("Specific request is accepted"), View.bCanSubmit);
	TestTrue(
		TEXT("Card remains available"),
		State.CardUsageState.PlayerACardUsageState.AvailableCardIds.Contains(
			MatchPlayExternalAttackRequestPreflightTests::CardA1));
	TestTrue(
		TEXT("Used cards remain empty"),
		State.CardUsageState.PlayerACardUsageState.UsedCardIds.IsEmpty());
	TestEqual(
		TEXT("Attack opportunity is not consumed"),
		State.RuntimeState.PlayerAState.UsedAttackCount,
		0);
	TestEqual(
		TEXT("Score is unchanged"),
		State.RuntimeState.PlayerAState.Score,
		2);
	TestEqual(
		TEXT("Current attacker is unchanged"),
		State.RuntimeState.CurrentAttackingPlayer,
		EInitialTurnOrderPlayer::PlayerA);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayExternalAttackRequestPreflightSpecificityTest,
	"FMCodex.CoreRules.MatchPlayExternalAttackRequestPreflight.StateReadinessIsNotRequestAcceptance",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayExternalAttackRequestPreflightSpecificityTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayExternalAttackRequestPreflightView View =
		FMatchPlayExternalAttackRequestPreflight::Evaluate(
			MatchPlayExternalAttackRequestPreflightTests::MakeState(),
			MatchPlayExternalAttackRequestPreflightTests::MakeRequest(
				EInitialTurnOrderPlayer::PlayerA,
				MatchPlayExternalAttackRequestPreflightTests::MissingCard));

	TestTrue(
		TEXT("State-level readiness is true"),
		View.StateView.bCanSubmitAttackRequest);
	TestTrue(
		TEXT("Preflight exposes state readiness"),
		View.bStateReadyForAttackRequest);
	TestFalse(
		TEXT("Concrete request is not accepted"),
		View.bRequestAcceptedByGate);
	TestFalse(
		TEXT("Concrete request cannot submit"),
		View.bCanSubmit);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayExternalAttackRequestPreflightDependencyTest,
	"FMCodex.CoreRules.MatchPlayExternalAttackRequestPreflight.ProductionUsesOnlyReadOnlyEntryPoints",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayExternalAttackRequestPreflightDependencyTest::RunTest(
	const FString& Parameters)
{
	const FString Source =
		MatchPlayExternalAttackRequestPreflightTests
			::LoadProductionSource();

	TestTrue(
		TEXT("Uses external state view"),
		Source.Contains(
			TEXT("FMatchPlayExternalStateViewQuery::BuildView")));
	TestTrue(
		TEXT("Uses submission gate"),
		Source.Contains(TEXT("FMatchPlaySubmissionGate::CanSubmit")));
	TestFalse(
		TEXT("Does not call external turn controller"),
		Source.Contains(TEXT("FMatchPlayExternalTurnController::")));
	TestFalse(
		TEXT("Does not call submit facade"),
		Source.Contains(TEXT("FMatchPlaySubmitAttackFacade::")));
	TestFalse(
		TEXT("Does not call AttackStep"),
		Source.Contains(TEXT("FMatchPlayAttackStep::")));
	TestFalse(TEXT("Does not call Flow"), Source.Contains(TEXT("Flow::")));
	TestFalse(
		TEXT("Does not call Executor"),
		Source.Contains(TEXT("Executor::")));
	TestFalse(TEXT("No while loop"), Source.Contains(TEXT("while (")));
	TestFalse(TEXT("No for loop"), Source.Contains(TEXT("for (")));
	TestFalse(TEXT("No Rand call"), Source.Contains(TEXT("Rand(")));
	TestFalse(
		TEXT("No automatic card selection"),
		Source.Contains(TEXT("SelectCard")));
	return true;
}

#endif
