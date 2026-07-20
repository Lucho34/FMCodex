#include "MatchPlayFinishDeployment.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

#include <type_traits>

namespace MatchPlayFinishDeploymentTests
{
	constexpr int64 ValidAttackSequence = 7;
	const FName PlayerAAvailableCard(TEXT("PlayerA.Available"));
	const FName PlayerAUsedCard(TEXT("PlayerA.Used"));
	const FName PlayerBAvailableCard(TEXT("PlayerB.Available"));
	const FName PlayerBUsedCard(TEXT("PlayerB.Used"));

	FMatchPlayState MakeState(
		const EInitialTurnOrderPlayer CurrentAttacker =
			EInitialTurnOrderPlayer::PlayerA,
		const EInitialTurnOrderPlayer CurrentLegalSide =
			EInitialTurnOrderPlayer::PlayerA,
		const bool bAttackerFinished = false,
		const bool bDefenderFinished = false,
		const EMatchPlayCurrentAttackPhase Phase =
			EMatchPlayCurrentAttackPhase::Deployment,
		const int64 AttackSequence = ValidAttackSequence)
	{
		FMatchPlayState State;
		State.RuntimeState.bIsInitialized = true;
		State.RuntimeState.PlayerAState.TotalAttackCount = 4;
		State.RuntimeState.PlayerAState.UsedAttackCount = 1;
		State.RuntimeState.PlayerAState.Score = 2;
		State.RuntimeState.PlayerAState.GoalkeeperCardId =
			TEXT("PlayerA.Goalkeeper");
		State.RuntimeState.PlayerAState.InitialDeckRarityScore = 11;
		State.RuntimeState.PlayerBState.TotalAttackCount = 3;
		State.RuntimeState.PlayerBState.UsedAttackCount = 1;
		State.RuntimeState.PlayerBState.Score = 1;
		State.RuntimeState.PlayerBState.GoalkeeperCardId =
			TEXT("PlayerB.Goalkeeper");
		State.RuntimeState.PlayerBState.InitialDeckRarityScore = 9;
		State.RuntimeState.FirstPlayer = EInitialTurnOrderPlayer::PlayerA;
		State.RuntimeState.SecondPlayer = EInitialTurnOrderPlayer::PlayerB;
		State.RuntimeState.CurrentAttackingPlayer = CurrentAttacker;
		State.RuntimeState.OpeningResultSnapshot.bSuccess = true;
		State.RuntimeState.OpeningResultSnapshot.AttackCountResult
			.PlayerAAttackCount = 4;
		State.RuntimeState.OpeningResultSnapshot.AttackCountResult
			.PlayerBAttackCount = 3;
		State.CardUsageState.PlayerACardUsageState.AvailableCardIds =
			{ PlayerAAvailableCard };
		State.CardUsageState.PlayerACardUsageState.UsedCardIds =
			{ PlayerAUsedCard };
		State.CardUsageState.PlayerBCardUsageState.AvailableCardIds =
			{ PlayerBAvailableCard };
		State.CardUsageState.PlayerBCardUsageState.UsedCardIds =
			{ PlayerBUsedCard };
		FMatchPlayDeploymentSlotDefinition Slot;
		Slot.SlotId = TEXT("SharedFinishSlot");
		Slot.NeutralSide = EMatchPlayNeutralSlotSide::NearPlayerB;
		State.DeploymentSlotCatalog.Slots.Add(Slot);
		State.bHasCurrentAttack = true;
		State.CurrentAttack.Phase = Phase;
		State.CurrentAttack.AttackSequence = AttackSequence;
		State.CurrentAttack.ActionPoint = 6;
		State.CurrentAttack.CurrentLegalDeploymentSide = CurrentLegalSide;
		State.CurrentAttack.bAttackerDeploymentFinished =
			bAttackerFinished;
		State.CurrentAttack.bDefenderDeploymentFinished =
			bDefenderFinished;
		FMatchPlayDeploymentPlacement Placement;
		Placement.PlayerSide = EInitialTurnOrderPlayer::PlayerB;
		Placement.CardId = TEXT("Existing.Placement");
		Placement.SlotId = TEXT("Existing.Slot");
		State.CurrentAttack.DeploymentPlacements.Add(Placement);
		State.CurrentAttack.bCurrentDefenseGoalkeeperActivated = true;
		return State;
	}

	bool AreStatesEqual(
		const FMatchPlayState& Left,
		const FMatchPlayState& Right)
	{
		return FMatchPlayState::StaticStruct()->CompareScriptStruct(
			&Left,
			&Right,
			0);
	}

	bool TestAtomicFailure(
		FAutomationTestBase& Test,
		const TCHAR* Context,
		const FMatchPlayState& BeforeState,
		const int64 AttackSequence,
		const EInitialTurnOrderPlayer RequestingSide,
		const EMatchPlayFinishDeploymentErrorCode ExpectedErrorCode)
	{
		const FMatchPlayState OriginalInput = BeforeState;
		const FMatchPlayFinishDeploymentResult Result =
			FMatchPlayFinishDeployment::Finish(
				BeforeState,
				AttackSequence,
				RequestingSide);
		Test.TestFalse(
			*FString::Printf(TEXT("%s fails"), Context),
			Result.bSuccess);
		Test.TestEqual(
			*FString::Printf(TEXT("%s returns exact error"), Context),
			Result.ErrorCode,
			ExpectedErrorCode);
		Test.TestTrue(
			*FString::Printf(TEXT("%s returns a readable error"), Context),
			!Result.ErrorMessage.IsEmpty());
		Test.TestTrue(
			*FString::Printf(TEXT("%s preserves BeforeState"), Context),
			AreStatesEqual(Result.BeforeState, BeforeState));
		Test.TestTrue(
			*FString::Printf(TEXT("%s preserves AfterState atomically"), Context),
			AreStatesEqual(Result.AfterState, BeforeState));
		Test.TestTrue(
			*FString::Printf(TEXT("%s does not mutate input"), Context),
			AreStatesEqual(BeforeState, OriginalInput));
		Test.TestEqual(
			*FString::Printf(TEXT("%s echoes AttackSequence"), Context),
			Result.AttackSequence,
			AttackSequence);
		Test.TestEqual(
			*FString::Printf(TEXT("%s echoes RequestingSide"), Context),
			Result.RequestingSide,
			RequestingSide);
		return true;
	}
}

#define MATCH_PLAY_FINISH_DEPLOYMENT_TEST(TestClass, TestName) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
		TestClass, \
		"FMCodex.CoreRules.MatchPlayFinishDeployment." TestName, \
		EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

MATCH_PLAY_FINISH_DEPLOYMENT_TEST(
	FMatchPlayFinishDeploymentPlayerAAttackerFirstTest,
	"PlayerAAttackerFirstFinishRotatesToPlayerB")

bool FMatchPlayFinishDeploymentPlayerAAttackerFirstTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayFinishDeploymentResult Result =
		FMatchPlayFinishDeployment::Finish(
			MatchPlayFinishDeploymentTests::MakeState(),
			MatchPlayFinishDeploymentTests::ValidAttackSequence,
			EInitialTurnOrderPlayer::PlayerA);
	TestTrue(TEXT("Finish succeeds"), Result.bSuccess);
	TestTrue(TEXT("Attacker is finished"),
		Result.AfterState.CurrentAttack.bAttackerDeploymentFinished);
	TestFalse(TEXT("Defender remains unfinished"),
		Result.AfterState.CurrentAttack.bDefenderDeploymentFinished);
	TestEqual(TEXT("Phase remains Deployment"),
		Result.AfterState.CurrentAttack.Phase,
		EMatchPlayCurrentAttackPhase::Deployment);
	TestEqual(TEXT("Legal side rotates to PlayerB"),
		Result.AfterState.CurrentAttack.CurrentLegalDeploymentSide,
		EInitialTurnOrderPlayer::PlayerB);
	return true;
}

MATCH_PLAY_FINISH_DEPLOYMENT_TEST(
	FMatchPlayFinishDeploymentPlayerBAttackerFirstTest,
	"PlayerBAttackerFirstFinishRotatesToPlayerA")

bool FMatchPlayFinishDeploymentPlayerBAttackerFirstTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayFinishDeploymentResult Result =
		FMatchPlayFinishDeployment::Finish(
			MatchPlayFinishDeploymentTests::MakeState(
				EInitialTurnOrderPlayer::PlayerB,
				EInitialTurnOrderPlayer::PlayerB),
			MatchPlayFinishDeploymentTests::ValidAttackSequence,
			EInitialTurnOrderPlayer::PlayerB);
	TestTrue(TEXT("Finish succeeds"), Result.bSuccess);
	TestTrue(TEXT("Attacker is finished"),
		Result.AfterState.CurrentAttack.bAttackerDeploymentFinished);
	TestFalse(TEXT("Defender remains unfinished"),
		Result.AfterState.CurrentAttack.bDefenderDeploymentFinished);
	TestEqual(TEXT("Legal side rotates to PlayerA"),
		Result.AfterState.CurrentAttack.CurrentLegalDeploymentSide,
		EInitialTurnOrderPlayer::PlayerA);
	return true;
}

MATCH_PLAY_FINISH_DEPLOYMENT_TEST(
	FMatchPlayFinishDeploymentPlayerADefenderFirstTest,
	"PlayerADefenderFirstFinishRotatesToPlayerB")

bool FMatchPlayFinishDeploymentPlayerADefenderFirstTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayFinishDeploymentResult Result =
		FMatchPlayFinishDeployment::Finish(
			MatchPlayFinishDeploymentTests::MakeState(
				EInitialTurnOrderPlayer::PlayerB,
				EInitialTurnOrderPlayer::PlayerA),
			MatchPlayFinishDeploymentTests::ValidAttackSequence,
			EInitialTurnOrderPlayer::PlayerA);
	TestTrue(TEXT("Finish succeeds"), Result.bSuccess);
	TestFalse(TEXT("Attacker remains unfinished"),
		Result.AfterState.CurrentAttack.bAttackerDeploymentFinished);
	TestTrue(TEXT("Defender is finished"),
		Result.AfterState.CurrentAttack.bDefenderDeploymentFinished);
	TestEqual(TEXT("Legal side rotates to PlayerB"),
		Result.AfterState.CurrentAttack.CurrentLegalDeploymentSide,
		EInitialTurnOrderPlayer::PlayerB);
	return true;
}

MATCH_PLAY_FINISH_DEPLOYMENT_TEST(
	FMatchPlayFinishDeploymentPlayerBDefenderFirstTest,
	"PlayerBDefenderFirstFinishRotatesToPlayerA")

bool FMatchPlayFinishDeploymentPlayerBDefenderFirstTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayFinishDeploymentResult Result =
		FMatchPlayFinishDeployment::Finish(
			MatchPlayFinishDeploymentTests::MakeState(
				EInitialTurnOrderPlayer::PlayerA,
				EInitialTurnOrderPlayer::PlayerB),
			MatchPlayFinishDeploymentTests::ValidAttackSequence,
			EInitialTurnOrderPlayer::PlayerB);
	TestTrue(TEXT("Finish succeeds"), Result.bSuccess);
	TestFalse(TEXT("Attacker remains unfinished"),
		Result.AfterState.CurrentAttack.bAttackerDeploymentFinished);
	TestTrue(TEXT("Defender is finished"),
		Result.AfterState.CurrentAttack.bDefenderDeploymentFinished);
	TestEqual(TEXT("Legal side rotates to PlayerA"),
		Result.AfterState.CurrentAttack.CurrentLegalDeploymentSide,
		EInitialTurnOrderPlayer::PlayerA);
	return true;
}

MATCH_PLAY_FINISH_DEPLOYMENT_TEST(
	FMatchPlayFinishDeploymentPlayerASecondTest,
	"PlayerASecondFinishEntersResolution")

bool FMatchPlayFinishDeploymentPlayerASecondTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayFinishDeploymentResult Result =
		FMatchPlayFinishDeployment::Finish(
			MatchPlayFinishDeploymentTests::MakeState(
				EInitialTurnOrderPlayer::PlayerB,
				EInitialTurnOrderPlayer::PlayerA,
				true,
				false),
			MatchPlayFinishDeploymentTests::ValidAttackSequence,
			EInitialTurnOrderPlayer::PlayerA);
	TestTrue(TEXT("Second Finish succeeds"), Result.bSuccess);
	TestTrue(TEXT("Attacker remains finished"),
		Result.AfterState.CurrentAttack.bAttackerDeploymentFinished);
	TestTrue(TEXT("Defender becomes finished"),
		Result.AfterState.CurrentAttack.bDefenderDeploymentFinished);
	TestEqual(TEXT("Phase enters Resolution"),
		Result.AfterState.CurrentAttack.Phase,
		EMatchPlayCurrentAttackPhase::Resolution);
	TestEqual(TEXT("Legal side is cleared"),
		Result.AfterState.CurrentAttack.CurrentLegalDeploymentSide,
		EInitialTurnOrderPlayer::None);
	return true;
}

MATCH_PLAY_FINISH_DEPLOYMENT_TEST(
	FMatchPlayFinishDeploymentPlayerBSecondTest,
	"PlayerBSecondFinishEntersResolution")

bool FMatchPlayFinishDeploymentPlayerBSecondTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayFinishDeploymentResult Result =
		FMatchPlayFinishDeployment::Finish(
			MatchPlayFinishDeploymentTests::MakeState(
				EInitialTurnOrderPlayer::PlayerA,
				EInitialTurnOrderPlayer::PlayerB,
				true,
				false),
			MatchPlayFinishDeploymentTests::ValidAttackSequence,
			EInitialTurnOrderPlayer::PlayerB);
	TestTrue(TEXT("Second Finish succeeds"), Result.bSuccess);
	TestTrue(TEXT("Both roles are finished"),
		Result.AfterState.CurrentAttack.bAttackerDeploymentFinished
			&& Result.AfterState.CurrentAttack.bDefenderDeploymentFinished);
	TestEqual(TEXT("Phase enters Resolution"),
		Result.AfterState.CurrentAttack.Phase,
		EMatchPlayCurrentAttackPhase::Resolution);
	TestEqual(TEXT("Legal side is None"),
		Result.AfterState.CurrentAttack.CurrentLegalDeploymentSide,
		EInitialTurnOrderPlayer::None);
	return true;
}

MATCH_PLAY_FINISH_DEPLOYMENT_TEST(
	FMatchPlayFinishDeploymentUninitializedTest,
	"UninitializedRuntimeRejectedFirst")

bool FMatchPlayFinishDeploymentUninitializedTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayState State = MatchPlayFinishDeploymentTests::MakeState();
	State.RuntimeState.bIsInitialized = false;
	State.bHasCurrentAttack = false;
	State.CurrentAttack.AttackSequence = 0;
	return MatchPlayFinishDeploymentTests::TestAtomicFailure(
		*this,
		TEXT("Uninitialized runtime"),
		State,
		0,
		EInitialTurnOrderPlayer::None,
		EMatchPlayFinishDeploymentErrorCode::MatchPlayStateNotInitialized);
}

MATCH_PLAY_FINISH_DEPLOYMENT_TEST(
	FMatchPlayFinishDeploymentNoCurrentAttackTest,
	"NoCurrentAttackRejectedBeforeSequence")

bool FMatchPlayFinishDeploymentNoCurrentAttackTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayState State = MatchPlayFinishDeploymentTests::MakeState();
	State.bHasCurrentAttack = false;
	State.CurrentAttack.AttackSequence = 0;
	return MatchPlayFinishDeploymentTests::TestAtomicFailure(
		*this,
		TEXT("No current attack"),
		State,
		0,
		EInitialTurnOrderPlayer::None,
		EMatchPlayFinishDeploymentErrorCode::NoCurrentAttack);
}

MATCH_PLAY_FINISH_DEPLOYMENT_TEST(
	FMatchPlayFinishDeploymentInvalidCurrentSequenceTest,
	"InvalidCurrentAttackSequenceRejected")

bool FMatchPlayFinishDeploymentInvalidCurrentSequenceTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State = MatchPlayFinishDeploymentTests::MakeState(
		EInitialTurnOrderPlayer::PlayerA,
		EInitialTurnOrderPlayer::PlayerA,
		false,
		false,
		EMatchPlayCurrentAttackPhase::Resolution,
		0);
	return MatchPlayFinishDeploymentTests::TestAtomicFailure(
		*this,
		TEXT("Invalid current sequence"),
		State,
		0,
		EInitialTurnOrderPlayer::PlayerA,
		EMatchPlayFinishDeploymentErrorCode::InvalidCurrentAttackSequence);
}

MATCH_PLAY_FINISH_DEPLOYMENT_TEST(
	FMatchPlayFinishDeploymentSequenceMismatchTest,
	"StaleSequenceRejectedBeforePhase")

bool FMatchPlayFinishDeploymentSequenceMismatchTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State = MatchPlayFinishDeploymentTests::MakeState(
		EInitialTurnOrderPlayer::PlayerA,
		EInitialTurnOrderPlayer::PlayerA,
		false,
		false,
		EMatchPlayCurrentAttackPhase::Resolution);
	return MatchPlayFinishDeploymentTests::TestAtomicFailure(
		*this,
		TEXT("Stale sequence"),
		State,
		MatchPlayFinishDeploymentTests::ValidAttackSequence - 1,
		EInitialTurnOrderPlayer::PlayerA,
		EMatchPlayFinishDeploymentErrorCode::AttackSequenceMismatch);
}

MATCH_PLAY_FINISH_DEPLOYMENT_TEST(
	FMatchPlayFinishDeploymentResolutionPhaseTest,
	"ResolutionPhaseRejected")

bool FMatchPlayFinishDeploymentResolutionPhaseTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State = MatchPlayFinishDeploymentTests::MakeState(
		EInitialTurnOrderPlayer::None,
		EInitialTurnOrderPlayer::None,
		false,
		false,
		EMatchPlayCurrentAttackPhase::Resolution);
	return MatchPlayFinishDeploymentTests::TestAtomicFailure(
		*this,
		TEXT("Resolution phase"),
		State,
		MatchPlayFinishDeploymentTests::ValidAttackSequence,
		EInitialTurnOrderPlayer::None,
		EMatchPlayFinishDeploymentErrorCode::CurrentAttackNotInDeployment);
}

MATCH_PLAY_FINISH_DEPLOYMENT_TEST(
	FMatchPlayFinishDeploymentInvalidAttackerTest,
	"InvalidCurrentAttackerRejected")

bool FMatchPlayFinishDeploymentInvalidAttackerTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State = MatchPlayFinishDeploymentTests::MakeState(
		EInitialTurnOrderPlayer::None,
		EInitialTurnOrderPlayer::PlayerA);
	return MatchPlayFinishDeploymentTests::TestAtomicFailure(
		*this,
		TEXT("Invalid current attacker"),
		State,
		MatchPlayFinishDeploymentTests::ValidAttackSequence,
		EInitialTurnOrderPlayer::PlayerA,
		EMatchPlayFinishDeploymentErrorCode::InvalidCurrentAttackingPlayer);
}

MATCH_PLAY_FINISH_DEPLOYMENT_TEST(
	FMatchPlayFinishDeploymentInvalidRequestingSideTest,
	"InvalidRequestingSideRejected")

bool FMatchPlayFinishDeploymentInvalidRequestingSideTest::RunTest(
	const FString& Parameters)
{
	return MatchPlayFinishDeploymentTests::TestAtomicFailure(
		*this,
		TEXT("Invalid requesting side"),
		MatchPlayFinishDeploymentTests::MakeState(),
		MatchPlayFinishDeploymentTests::ValidAttackSequence,
		EInitialTurnOrderPlayer::None,
		EMatchPlayFinishDeploymentErrorCode::InvalidRequestingSide);
}

MATCH_PLAY_FINISH_DEPLOYMENT_TEST(
	FMatchPlayFinishDeploymentInvalidLegalSideTest,
	"InvalidCurrentLegalSideRejected")

bool FMatchPlayFinishDeploymentInvalidLegalSideTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State = MatchPlayFinishDeploymentTests::MakeState(
		EInitialTurnOrderPlayer::PlayerA,
		EInitialTurnOrderPlayer::None);
	return MatchPlayFinishDeploymentTests::TestAtomicFailure(
		*this,
		TEXT("Invalid legal side"),
		State,
		MatchPlayFinishDeploymentTests::ValidAttackSequence,
		EInitialTurnOrderPlayer::PlayerA,
		EMatchPlayFinishDeploymentErrorCode
			::InvalidCurrentLegalDeploymentSide);
}

MATCH_PLAY_FINISH_DEPLOYMENT_TEST(
	FMatchPlayFinishDeploymentWrongLegalSideTest,
	"WrongLegalSideRejected")

bool FMatchPlayFinishDeploymentWrongLegalSideTest::RunTest(
	const FString& Parameters)
{
	return MatchPlayFinishDeploymentTests::TestAtomicFailure(
		*this,
		TEXT("Wrong legal side"),
		MatchPlayFinishDeploymentTests::MakeState(),
		MatchPlayFinishDeploymentTests::ValidAttackSequence,
		EInitialTurnOrderPlayer::PlayerB,
		EMatchPlayFinishDeploymentErrorCode
			::RequestingSideNotCurrentLegalDeploymentSide);
}

MATCH_PLAY_FINISH_DEPLOYMENT_TEST(
	FMatchPlayFinishDeploymentAttackerAlreadyFinishedTest,
	"AttackerAlreadyFinishedRejected")

bool FMatchPlayFinishDeploymentAttackerAlreadyFinishedTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State = MatchPlayFinishDeploymentTests::MakeState(
		EInitialTurnOrderPlayer::PlayerA,
		EInitialTurnOrderPlayer::PlayerA,
		true,
		false);
	return MatchPlayFinishDeploymentTests::TestAtomicFailure(
		*this,
		TEXT("Attacker already finished"),
		State,
		MatchPlayFinishDeploymentTests::ValidAttackSequence,
		EInitialTurnOrderPlayer::PlayerA,
		EMatchPlayFinishDeploymentErrorCode::RequestingSideAlreadyFinished);
}

MATCH_PLAY_FINISH_DEPLOYMENT_TEST(
	FMatchPlayFinishDeploymentDefenderAlreadyFinishedTest,
	"DefenderAlreadyFinishedRejected")

bool FMatchPlayFinishDeploymentDefenderAlreadyFinishedTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State = MatchPlayFinishDeploymentTests::MakeState(
		EInitialTurnOrderPlayer::PlayerA,
		EInitialTurnOrderPlayer::PlayerB,
		false,
		true);
	return MatchPlayFinishDeploymentTests::TestAtomicFailure(
		*this,
		TEXT("Defender already finished"),
		State,
		MatchPlayFinishDeploymentTests::ValidAttackSequence,
		EInitialTurnOrderPlayer::PlayerB,
		EMatchPlayFinishDeploymentErrorCode::RequestingSideAlreadyFinished);
}

MATCH_PLAY_FINISH_DEPLOYMENT_TEST(
	FMatchPlayFinishDeploymentBothFinishedTest,
	"BothFinishedDeploymentStateRejected")

bool FMatchPlayFinishDeploymentBothFinishedTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State = MatchPlayFinishDeploymentTests::MakeState(
		EInitialTurnOrderPlayer::PlayerA,
		EInitialTurnOrderPlayer::PlayerA,
		true,
		true);
	return MatchPlayFinishDeploymentTests::TestAtomicFailure(
		*this,
		TEXT("Both roles already finished"),
		State,
		MatchPlayFinishDeploymentTests::ValidAttackSequence,
		EInitialTurnOrderPlayer::PlayerA,
		EMatchPlayFinishDeploymentErrorCode
			::InvalidDeploymentFinishedState);
}

MATCH_PLAY_FINISH_DEPLOYMENT_TEST(
	FMatchPlayFinishDeploymentPreservesAuthoritiesTest,
	"SuccessPreservesUnrelatedAuthorities")

bool FMatchPlayFinishDeploymentPreservesAuthoritiesTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState BeforeState =
		MatchPlayFinishDeploymentTests::MakeState();
	const FMatchPlayFinishDeploymentResult Result =
		FMatchPlayFinishDeployment::Finish(
			BeforeState,
			MatchPlayFinishDeploymentTests::ValidAttackSequence,
			EInitialTurnOrderPlayer::PlayerA);
	TestTrue(TEXT("Finish succeeds"), Result.bSuccess);
	TestTrue(TEXT("Runtime is preserved field-for-field"),
		FMatchRuntimeState::StaticStruct()->CompareScriptStruct(
			&Result.AfterState.RuntimeState,
			&BeforeState.RuntimeState,
			0));
	TestTrue(TEXT("CardUsage is preserved field-for-field"),
		FMatchCardUsageState::StaticStruct()->CompareScriptStruct(
			&Result.AfterState.CardUsageState,
			&BeforeState.CardUsageState,
			0));
	TestTrue(TEXT("CurrentAttack remains present"),
		Result.AfterState.bHasCurrentAttack);
	TestEqual(TEXT("ActionPoint is preserved"),
		Result.AfterState.CurrentAttack.ActionPoint,
		BeforeState.CurrentAttack.ActionPoint);
	TestEqual(TEXT("AttackSequence is preserved"),
		Result.AfterState.CurrentAttack.AttackSequence,
		BeforeState.CurrentAttack.AttackSequence);
	TestEqual(TEXT("Placement count is preserved"),
		Result.AfterState.CurrentAttack.DeploymentPlacements.Num(),
		BeforeState.CurrentAttack.DeploymentPlacements.Num());
	TestTrue(TEXT("Placement is preserved field-for-field"),
		FMatchPlayDeploymentPlacement::StaticStruct()->CompareScriptStruct(
			&Result.AfterState.CurrentAttack.DeploymentPlacements[0],
			&BeforeState.CurrentAttack.DeploymentPlacements[0],
			0));
	TestEqual(TEXT("Temporary GK activation is preserved"),
		Result.AfterState.CurrentAttack.bCurrentDefenseGoalkeeperActivated,
		BeforeState.CurrentAttack.bCurrentDefenseGoalkeeperActivated);
	return true;
}

MATCH_PLAY_FINISH_DEPLOYMENT_TEST(
	FMatchPlayFinishDeploymentDeterministicTest,
	"RepeatedEvaluationIsDeterministic")

bool FMatchPlayFinishDeploymentDeterministicTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState BeforeState =
		MatchPlayFinishDeploymentTests::MakeState(
			EInitialTurnOrderPlayer::PlayerB,
			EInitialTurnOrderPlayer::PlayerA);
	const FMatchPlayFinishDeploymentResult First =
		FMatchPlayFinishDeployment::Finish(
			BeforeState,
			MatchPlayFinishDeploymentTests::ValidAttackSequence,
			EInitialTurnOrderPlayer::PlayerA);
	const FMatchPlayFinishDeploymentResult Second =
		FMatchPlayFinishDeployment::Finish(
			BeforeState,
			MatchPlayFinishDeploymentTests::ValidAttackSequence,
			EInitialTurnOrderPlayer::PlayerA);
	TestTrue(TEXT("Repeated Results are field-for-field identical"),
		FMatchPlayFinishDeploymentResult::StaticStruct()
			->CompareScriptStruct(&First, &Second, 0));
	return true;
}

MATCH_PLAY_FINISH_DEPLOYMENT_TEST(
	FMatchPlayFinishDeploymentPublicContractTest,
	"PublicContractDefaultsAndEcho")

bool FMatchPlayFinishDeploymentPublicContractTest::RunTest(
	const FString& Parameters)
{
	using FExpectedFinishFunction = FMatchPlayFinishDeploymentResult (*)(
		const FMatchPlayState&,
		int64,
		EInitialTurnOrderPlayer);
	static_assert(
		std::is_same_v<
			decltype(&FMatchPlayFinishDeployment::Finish),
			FExpectedFinishFunction>,
		"Finish public signature must remain exact.");

	const FMatchPlayFinishDeploymentResult DefaultResult;
	TestFalse(TEXT("Default result is unsuccessful"), DefaultResult.bSuccess);
	TestEqual(TEXT("Default sequence is zero"),
		DefaultResult.AttackSequence,
		int64{ 0 });
	TestEqual(TEXT("Default requesting side is None"),
		DefaultResult.RequestingSide,
		EInitialTurnOrderPlayer::None);
	TestEqual(TEXT("Default error is None"),
		DefaultResult.ErrorCode,
		EMatchPlayFinishDeploymentErrorCode::None);
	TestTrue(TEXT("Default error message is empty"),
		DefaultResult.ErrorMessage.IsEmpty());

	const FMatchPlayState BeforeState =
		MatchPlayFinishDeploymentTests::MakeState();
	const FMatchPlayFinishDeploymentResult Success =
		FMatchPlayFinishDeployment::Finish(
			BeforeState,
			MatchPlayFinishDeploymentTests::ValidAttackSequence,
			EInitialTurnOrderPlayer::PlayerA);
	TestTrue(TEXT("Public API succeeds for valid input"), Success.bSuccess);
	TestTrue(TEXT("BeforeState is echoed field-for-field"),
		MatchPlayFinishDeploymentTests::AreStatesEqual(
			Success.BeforeState,
			BeforeState));
	TestEqual(TEXT("AttackSequence is echoed"),
		Success.AttackSequence,
		MatchPlayFinishDeploymentTests::ValidAttackSequence);
	TestEqual(TEXT("RequestingSide is echoed"),
		Success.RequestingSide,
		EInitialTurnOrderPlayer::PlayerA);
	TestEqual(TEXT("Successful error is None"),
		Success.ErrorCode,
		EMatchPlayFinishDeploymentErrorCode::None);
	TestTrue(TEXT("Successful error message is empty"),
		Success.ErrorMessage.IsEmpty());
	return true;
}

MATCH_PLAY_FINISH_DEPLOYMENT_TEST(
	FMatchPlayFinishDeploymentFirstFinishPreservesCatalogTest,
	"FirstFinishPreservesDeploymentSlotCatalog")

bool FMatchPlayFinishDeploymentFirstFinishPreservesCatalogTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState BeforeState =
		MatchPlayFinishDeploymentTests::MakeState();
	const FMatchPlayFinishDeploymentResult Result =
		FMatchPlayFinishDeployment::Finish(
			BeforeState,
			MatchPlayFinishDeploymentTests::ValidAttackSequence,
			EInitialTurnOrderPlayer::PlayerA);

	TestTrue(TEXT("First finish succeeds"), Result.bSuccess);
	TestEqual(
		TEXT("First finish remains in Deployment"),
		Result.AfterState.CurrentAttack.Phase,
		EMatchPlayCurrentAttackPhase::Deployment);
	TestTrue(
		TEXT("First finish preserves the deployment slot catalog"),
		FMatchPlayDeploymentSlotCatalog::StaticStruct()
			->CompareScriptStruct(
				&Result.AfterState.DeploymentSlotCatalog,
				&BeforeState.DeploymentSlotCatalog,
				0));
	return true;
}

MATCH_PLAY_FINISH_DEPLOYMENT_TEST(
	FMatchPlayFinishDeploymentResolutionPreservesCatalogTest,
	"ResolutionTransitionPreservesDeploymentSlotCatalog")

bool FMatchPlayFinishDeploymentResolutionPreservesCatalogTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState BeforeState =
		MatchPlayFinishDeploymentTests::MakeState(
			EInitialTurnOrderPlayer::PlayerA,
			EInitialTurnOrderPlayer::PlayerB,
			true,
			false);
	const FMatchPlayFinishDeploymentResult Result =
		FMatchPlayFinishDeployment::Finish(
			BeforeState,
			MatchPlayFinishDeploymentTests::ValidAttackSequence,
			EInitialTurnOrderPlayer::PlayerB);

	TestTrue(TEXT("Second finish succeeds"), Result.bSuccess);
	TestEqual(
		TEXT("Second finish enters Resolution"),
		Result.AfterState.CurrentAttack.Phase,
		EMatchPlayCurrentAttackPhase::Resolution);
	TestTrue(
		TEXT("Resolution transition preserves the deployment slot catalog"),
		FMatchPlayDeploymentSlotCatalog::StaticStruct()
			->CompareScriptStruct(
				&Result.AfterState.DeploymentSlotCatalog,
				&BeforeState.DeploymentSlotCatalog,
				0));
	return true;
}

#undef MATCH_PLAY_FINISH_DEPLOYMENT_TEST

#endif
