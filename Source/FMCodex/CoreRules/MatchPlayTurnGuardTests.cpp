#include "MatchPlayTurnGuard.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace MatchPlayTurnGuardTests
{
	const FName CardA1(TEXT("CardA1"));
	const FName CardA2(TEXT("CardA2"));
	const FName CardB1(TEXT("CardB1"));
	const FName SharedCard(TEXT("SharedCard"));

	FMatchPlayState MakeState(
		const EInitialTurnOrderPlayer CurrentPlayer =
			EInitialTurnOrderPlayer::PlayerA,
		const int32 PlayerAScore = 0,
		const int32 PlayerBScore = 0,
		const int32 PlayerATotal = 3,
		const int32 PlayerAUsed = 0,
		const int32 PlayerBTotal = 3,
		const int32 PlayerBUsed = 0,
		const bool bPlayerAHasCards = true,
		const bool bPlayerBHasCards = true)
	{
		FMatchRuntimeState RuntimeState;
		RuntimeState.bIsInitialized = true;
		RuntimeState.PlayerAState.TotalAttackCount = PlayerATotal;
		RuntimeState.PlayerAState.UsedAttackCount = PlayerAUsed;
		RuntimeState.PlayerAState.Score = PlayerAScore;
		RuntimeState.PlayerBState.TotalAttackCount = PlayerBTotal;
		RuntimeState.PlayerBState.UsedAttackCount = PlayerBUsed;
		RuntimeState.PlayerBState.Score = PlayerBScore;
		RuntimeState.FirstPlayer = EInitialTurnOrderPlayer::PlayerA;
		RuntimeState.SecondPlayer = EInitialTurnOrderPlayer::PlayerB;
		RuntimeState.CurrentAttackingPlayer = CurrentPlayer;
		RuntimeState.OpeningResultSnapshot.bSuccess = true;
		RuntimeState.OpeningResultSnapshot.AttackCountResult
			.PlayerAAttackCount = PlayerATotal;
		RuntimeState.OpeningResultSnapshot.AttackCountResult
			.PlayerBAttackCount = PlayerBTotal;

		FMatchCardUsageState CardUsageState;
		if (bPlayerAHasCards)
		{
			CardUsageState.PlayerACardUsageState.AvailableCardIds =
				{ CardA1, CardA2, SharedCard };
		}
		if (bPlayerBHasCards)
		{
			CardUsageState.PlayerBCardUsageState.AvailableCardIds =
				{ CardB1, SharedCard };
		}
		FMatchPlayState State;
		State.RuntimeState = RuntimeState;
		State.CardUsageState = CardUsageState;
		FMatchPlayDeploymentSlotDefinition Slot;
		Slot.SlotId = TEXT("TestDeploymentSlot");
		Slot.NeutralSide = EMatchPlayNeutralSlotSide::NearPlayerA;
		State.DeploymentSlotCatalog.Slots.Add(Slot);
		return State;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayTurnGuardCanSubmitTest,
	"FMCodex.CoreRules.MatchPlayTurnGuard.CanSubmitWhenCurrentAttackerHasOpportunityAndCard",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayTurnGuardCanSubmitTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayTurnGuardResult Result =
		FMatchPlayTurnGuard::QueryCanSubmitAttackRequest(
			MatchPlayTurnGuardTests::MakeState());

	TestTrue(TEXT("Can submit attack request"), Result.bCanSubmitAttackRequest);
	TestTrue(TEXT("Should wait for external request"), Result.bShouldWaitForExternalAttackRequest);
	TestEqual(TEXT("No error"), Result.ErrorCode, EMatchPlayTurnGuardErrorCode::None);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayTurnGuardNoRemainingTest,
	"FMCodex.CoreRules.MatchPlayTurnGuard.CannotSubmitWhenBothPlayersHaveNoRemainingOpportunities",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayTurnGuardNoRemainingTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayTurnGuardResult Result =
		FMatchPlayTurnGuard::QueryCanSubmitAttackRequest(
			MatchPlayTurnGuardTests::MakeState(
				EInitialTurnOrderPlayer::PlayerA,
				0,
				0,
				1,
				1,
				1,
				1));

	TestFalse(TEXT("Cannot submit"), Result.bCanSubmitAttackRequest);
	TestTrue(TEXT("No remaining flag is set"), Result.bNoRemainingAttackOpportunities);
	TestEqual(TEXT("No remaining error"), Result.ErrorCode, EMatchPlayTurnGuardErrorCode::NoRemainingAttackOpportunities);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayTurnGuardFinishedNoneAttackerTest,
	"FMCodex.CoreRules.MatchPlayTurnGuard.NoneAttackerWithNoRemainingOpportunitiesIsFinished",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayTurnGuardFinishedNoneAttackerTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayTurnGuardResult Result =
		FMatchPlayTurnGuard::QueryCanSubmitAttackRequest(
			MatchPlayTurnGuardTests::MakeState(
				EInitialTurnOrderPlayer::None,
				0,
				0,
				1,
				1,
				1,
				1));

	TestFalse(TEXT("Finished state cannot submit"), Result.bCanSubmitAttackRequest);
	TestTrue(
		TEXT("No remaining flag is set"),
		Result.bNoRemainingAttackOpportunities);
	TestEqual(
		TEXT("Finished state reports no remaining opportunities"),
		Result.ErrorCode,
		EMatchPlayTurnGuardErrorCode::NoRemainingAttackOpportunities);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayTurnGuardInvalidNoneAttackerTest,
	"FMCodex.CoreRules.MatchPlayTurnGuard.NoneAttackerWithRemainingOpportunitiesIsNotReady",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayTurnGuardInvalidNoneAttackerTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayTurnGuardResult Result =
		FMatchPlayTurnGuard::QueryCanSubmitAttackRequest(
			MatchPlayTurnGuardTests::MakeState(
				EInitialTurnOrderPlayer::None,
				0,
				0,
				1,
				0,
				1,
				0));

	TestFalse(TEXT("Invalid state cannot submit"), Result.bCanSubmitAttackRequest);
	TestFalse(
		TEXT("Remaining opportunities do not imply finished"),
		Result.bNoRemainingAttackOpportunities);
	TestEqual(
		TEXT("None attacker remains an invalid non-terminal state"),
		Result.ErrorCode,
		EMatchPlayTurnGuardErrorCode::MatchStateNotInitialized);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayTurnGuardCurrentNoRemainingTest,
	"FMCodex.CoreRules.MatchPlayTurnGuard.CannotSubmitWhenCurrentAttackerHasNoRemainingOpportunity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayTurnGuardCurrentNoRemainingTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayTurnGuardResult Result =
		FMatchPlayTurnGuard::QueryCanSubmitAttackRequest(
			MatchPlayTurnGuardTests::MakeState(
				EInitialTurnOrderPlayer::PlayerA,
				0,
				0,
				1,
				1,
				2,
				0));

	TestFalse(TEXT("Cannot submit"), Result.bCanSubmitAttackRequest);
	TestEqual(TEXT("Current attacker no remaining error"), Result.ErrorCode, EMatchPlayTurnGuardErrorCode::CurrentAttackerHasNoRemainingAttackOpportunity);
	TestEqual(TEXT("Current attacker remaining is zero"), Result.CurrentAttackerRemainingAttackCount, 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayTurnGuardNoCardsTest,
	"FMCodex.CoreRules.MatchPlayTurnGuard.CannotSubmitWhenCurrentAttackerHasNoAvailableCards",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayTurnGuardNoCardsTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayTurnGuardResult Result =
		FMatchPlayTurnGuard::QueryCanSubmitAttackRequest(
			MatchPlayTurnGuardTests::MakeState(
				EInitialTurnOrderPlayer::PlayerA,
				0,
				0,
				3,
				0,
				3,
				0,
				false,
				true));

	TestFalse(TEXT("Cannot submit"), Result.bCanSubmitAttackRequest);
	TestEqual(TEXT("No-card error"), Result.ErrorCode, EMatchPlayTurnGuardErrorCode::CurrentAttackerHasNoAvailableCards);
	TestEqual(TEXT("Current attacker available card count is zero"), Result.CurrentAttackerAvailableCardCount, 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayTurnGuardPlayerAStateTest,
	"FMCodex.CoreRules.MatchPlayTurnGuard.PlayerACurrentAttackerReadsPlayerAState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayTurnGuardPlayerAStateTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayTurnGuardResult Result =
		FMatchPlayTurnGuard::QueryCanSubmitAttackRequest(
			MatchPlayTurnGuardTests::MakeState(
				EInitialTurnOrderPlayer::PlayerA,
				0,
				0,
				4,
				1,
				5,
				0));

	TestEqual(TEXT("Current attacker is PlayerA"), Result.CurrentAttacker, EInitialTurnOrderPlayer::PlayerA);
	TestEqual(TEXT("PlayerA remaining count is used"), Result.CurrentAttackerRemainingAttackCount, 3);
	TestEqual(TEXT("PlayerA card count is used"), Result.CurrentAttackerAvailableCardCount, 3);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayTurnGuardPlayerBStateTest,
	"FMCodex.CoreRules.MatchPlayTurnGuard.PlayerBCurrentAttackerReadsPlayerBState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayTurnGuardPlayerBStateTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayTurnGuardResult Result =
		FMatchPlayTurnGuard::QueryCanSubmitAttackRequest(
			MatchPlayTurnGuardTests::MakeState(
				EInitialTurnOrderPlayer::PlayerB,
				0,
				0,
				4,
				1,
				5,
				2));

	TestEqual(TEXT("Current attacker is PlayerB"), Result.CurrentAttacker, EInitialTurnOrderPlayer::PlayerB);
	TestEqual(TEXT("PlayerB remaining count is used"), Result.CurrentAttackerRemainingAttackCount, 3);
	TestEqual(TEXT("PlayerB card count is used"), Result.CurrentAttackerAvailableCardCount, 2);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayTurnGuardSharedCardNoSpecificCardTest,
	"FMCodex.CoreRules.MatchPlayTurnGuard.SameCardIdAcrossPlayersDoesNotAffectGuard",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayTurnGuardSharedCardNoSpecificCardTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayTurnGuardResult PlayerAResult =
		FMatchPlayTurnGuard::QueryCanSubmitAttackRequest(
			MatchPlayTurnGuardTests::MakeState(
				EInitialTurnOrderPlayer::PlayerA));
	const FMatchPlayTurnGuardResult PlayerBResult =
		FMatchPlayTurnGuard::QueryCanSubmitAttackRequest(
			MatchPlayTurnGuardTests::MakeState(
				EInitialTurnOrderPlayer::PlayerB));

	TestTrue(TEXT("PlayerA can submit despite shared CardId"), PlayerAResult.bCanSubmitAttackRequest);
	TestTrue(TEXT("PlayerB can submit despite shared CardId"), PlayerBResult.bCanSubmitAttackRequest);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayTurnGuardInputUnchangedTest,
	"FMCodex.CoreRules.MatchPlayTurnGuard.DoesNotModifyInputMatchPlayState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayTurnGuardInputUnchangedTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State =
		MatchPlayTurnGuardTests::MakeState(
			EInitialTurnOrderPlayer::PlayerA,
			2,
			1);
	const TArray<FName> Available =
		State.CardUsageState.PlayerACardUsageState.AvailableCardIds;

	const FMatchPlayTurnGuardResult Result =
		FMatchPlayTurnGuard::QueryCanSubmitAttackRequest(State);

	TestTrue(TEXT("Guard can submit"), Result.bCanSubmitAttackRequest);
	TestEqual(TEXT("Input score remains unchanged"), State.RuntimeState.PlayerAState.Score, 2);
	TestEqual(TEXT("Input used attacks remain unchanged"), State.RuntimeState.PlayerAState.UsedAttackCount, 0);
	TestTrue(TEXT("Input cards remain unchanged"), State.CardUsageState.PlayerACardUsageState.AvailableCardIds == Available);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayTurnGuardNoCardConsumptionTest,
	"FMCodex.CoreRules.MatchPlayTurnGuard.DoesNotConsumeCard",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayTurnGuardNoCardConsumptionTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State = MatchPlayTurnGuardTests::MakeState();
	const FMatchPlayTurnGuardResult Result =
		FMatchPlayTurnGuard::QueryCanSubmitAttackRequest(State);

	TestTrue(TEXT("Guard can submit"), Result.bCanSubmitAttackRequest);
	TestEqual(TEXT("Available card count remains three"), State.CardUsageState.PlayerACardUsageState.AvailableCardIds.Num(), 3);
	TestEqual(TEXT("Used card count remains zero"), State.CardUsageState.PlayerACardUsageState.UsedCardIds.Num(), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayTurnGuardNoOpportunityConsumptionTest,
	"FMCodex.CoreRules.MatchPlayTurnGuard.DoesNotConsumeAttackOpportunity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayTurnGuardNoOpportunityConsumptionTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State = MatchPlayTurnGuardTests::MakeState();
	const FMatchPlayTurnGuardResult Result =
		FMatchPlayTurnGuard::QueryCanSubmitAttackRequest(State);

	TestTrue(TEXT("Guard can submit"), Result.bCanSubmitAttackRequest);
	TestEqual(TEXT("Used attack count remains zero"), State.RuntimeState.PlayerAState.UsedAttackCount, 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayTurnGuardNoScoreChangeTest,
	"FMCodex.CoreRules.MatchPlayTurnGuard.DoesNotChangeScore",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayTurnGuardNoScoreChangeTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State =
		MatchPlayTurnGuardTests::MakeState(
			EInitialTurnOrderPlayer::PlayerA,
			2,
			1);
	const FMatchPlayTurnGuardResult Result =
		FMatchPlayTurnGuard::QueryCanSubmitAttackRequest(State);

	TestTrue(TEXT("Guard can submit"), Result.bCanSubmitAttackRequest);
	TestEqual(TEXT("PlayerA score remains unchanged"), State.RuntimeState.PlayerAState.Score, 2);
	TestEqual(TEXT("PlayerB score remains unchanged"), State.RuntimeState.PlayerBState.Score, 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayTurnGuardNoFlowCallsTest,
	"FMCodex.CoreRules.MatchPlayTurnGuard.ProductionCodeDoesNotCallAttackOrFormulaFlows",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayTurnGuardNoFlowCallsTest::RunTest(
	const FString& Parameters)
{
	FString Source;
	const bool bLoaded = FFileHelper::LoadFileToString(
		Source,
		*(FPaths::ProjectDir()
			/ TEXT("Source/FMCodex/CoreRules/MatchPlayTurnGuard.cpp")));

	TestTrue(TEXT("Production source can be loaded"), bLoaded);
	TestFalse(TEXT("No AttackStep call"), Source.Contains(TEXT("FMatchPlayAttackStep::")));
	TestFalse(TEXT("No Executor call"), Source.Contains(TEXT("FMatchPlayAttackExecutor::")));
	TestFalse(TEXT("No MatchPlayAttackFlow call"), Source.Contains(TEXT("FMatchPlayAttackFlow::")));
	TestFalse(TEXT("No FormulaAttackFlow call"), Source.Contains(TEXT("FFormulaAttackFlow::")));
	TestFalse(TEXT("No FormulaResolver call"), Source.Contains(TEXT("UFormulaResolver::")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayTurnGuardNoResultResolverTest,
	"FMCodex.CoreRules.MatchPlayTurnGuard.ProductionCodeDoesNotRecalculateMatchResult",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayTurnGuardNoResultResolverTest::RunTest(
	const FString& Parameters)
{
	FString Source;
	const bool bLoaded = FFileHelper::LoadFileToString(
		Source,
		*(FPaths::ProjectDir()
			/ TEXT("Source/FMCodex/CoreRules/MatchPlayTurnGuard.cpp")));

	TestTrue(TEXT("Production source can be loaded"), bLoaded);
	TestFalse(TEXT("No MatchEndResolver call"), Source.Contains(TEXT("FMatchEndResolver::")));
	TestFalse(TEXT("No MatchResultResolver call"), Source.Contains(TEXT("FMatchResultResolver::")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayTurnGuardRepeatedTest,
	"FMCodex.CoreRules.MatchPlayTurnGuard.RepeatedQueryIsConsistent",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayTurnGuardRepeatedTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State = MatchPlayTurnGuardTests::MakeState();
	const FMatchPlayTurnGuardResult First =
		FMatchPlayTurnGuard::QueryCanSubmitAttackRequest(State);
	const FMatchPlayTurnGuardResult Second =
		FMatchPlayTurnGuard::QueryCanSubmitAttackRequest(State);

	TestEqual(TEXT("Can-submit result is consistent"), First.bCanSubmitAttackRequest, Second.bCanSubmitAttackRequest);
	TestEqual(TEXT("Error code is consistent"), First.ErrorCode, Second.ErrorCode);
	TestEqual(TEXT("Current attacker remaining count is consistent"), First.CurrentAttackerRemainingAttackCount, Second.CurrentAttackerRemainingAttackCount);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayTurnGuardCurrentAttackTest,
	"FMCodex.CoreRules.MatchPlayTurnGuard.ActiveCurrentAttackBlocksLegacySubmission",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayTurnGuardCurrentAttackTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayState State = MatchPlayTurnGuardTests::MakeState();
	State.bHasCurrentAttack = true;
	State.CurrentAttack.AttackSequence = 1;
	State.CurrentAttack.ActionPoint = 5;
	State.CurrentAttack.CurrentLegalDeploymentSide =
		State.RuntimeState.CurrentAttackingPlayer;

	const FMatchPlayTurnGuardResult Result =
		FMatchPlayTurnGuard::QueryCanSubmitAttackRequest(State);

	TestFalse(TEXT("Legacy attack request is blocked"),
		Result.bCanSubmitAttackRequest);
	TestFalse(TEXT("Guard does not wait for a legacy request"),
		Result.bShouldWaitForExternalAttackRequest);
	TestEqual(TEXT("Active-attack error is exact"),
		Result.ErrorCode,
		EMatchPlayTurnGuardErrorCode::CurrentAttackInProgress);
	return true;
}

#endif
