#include "MatchPlayCurrentAttackRunnerSelectionWriter.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "MatchPlayCurrentAttackRunnerSelectionTestFixtures.h"
#include "Misc/AutomationTest.h"

namespace RunnerWriterFixtures =
	FMCodex::Tests::MatchPlayCurrentAttackRunnerSelection;

#define RUNNER_WRITER_TEST(TestClass, TestName) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
		TestClass, \
		"FMCodex.CoreRules.MatchPlayCurrentAttackRunnerSelection.Writer." TestName, \
		EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

RUNNER_WRITER_TEST(FRunnerWriterSuccessTest, "SuccessExactMutation")

bool FRunnerWriterSuccessTest::RunTest(const FString& Parameters)
{
	struct FCase
	{
		ESkillRuleType ActionType;
		FName RunnerCardId;
	};
	for (const FCase& Case : {
		FCase{ESkillRuleType::PassControl,
			RunnerWriterFixtures::MidfieldRunnerId},
		FCase{ESkillRuleType::Cross,
			RunnerWriterFixtures::AttackRunnerId},
		FCase{ESkillRuleType::ThroughBall,
			RunnerWriterFixtures::ForwardRunnerId}})
	{
		const FMatchPlayState Before =
			RunnerWriterFixtures::MakeState(Case.ActionType);
		const auto Result =
			FMatchPlayCurrentAttackRunnerSelectionWriter::Select(
				Before,
				RunnerWriterFixtures::MakeRequest(Case.RunnerCardId));
		TestTrue(TEXT("Valid writer succeeds"), Result.bSuccess);
		FMatchPlayState Expected = Before;
		Expected.CurrentAttack.ActionPreparation.RunnerCardId =
			Case.RunnerCardId;
		Expected.CurrentAttack.SelectionStage =
			EMatchPlayCurrentAttackSelectionStage::AwaitingHelper;
		TestTrue(TEXT("Only RunnerCardId and stage change"),
			RunnerWriterFixtures::AreStatesEqual(
				Result.AfterState, Expected));
		TestTrue(TEXT("Before state preserved"),
			RunnerWriterFixtures::AreStatesEqual(
				Result.BeforeState, Before));
		TestFalse(TEXT("No final SelectedAction"),
			Result.AfterState.CurrentAttack.bHasSelectedAction);
		TestTrue(TEXT("SelectedAction remains default"),
			Result.AfterState.CurrentAttack
				.SelectedAction.CarrierCardId.IsNone());
	}
	return true;
}

RUNNER_WRITER_TEST(FRunnerWriterFailureAtomicityTest, "FailureAtomicity")

bool FRunnerWriterFailureAtomicityTest::RunTest(
	const FString& Parameters)
{
	auto ExpectFailure = [this](
		const TCHAR* Label,
		const FMatchPlayState& State,
		FMatchPlayCurrentAttackRunnerSelectionRequest Request)
	{
		const auto Result =
			FMatchPlayCurrentAttackRunnerSelectionWriter::Select(
				State, Request);
		TestFalse(Label, Result.bSuccess);
		TestEqual(Label, Result.ErrorCode,
			EMatchPlayCurrentAttackRunnerSelectionWriterErrorCode
				::LegalityFailed);
		TestTrue(TEXT("Failure returns exact input"),
			RunnerWriterFixtures::AreStatesEqual(
				Result.AfterState, State));
	};

	auto Request = RunnerWriterFixtures::MakeRequest(
		RunnerWriterFixtures::MidfieldRunnerId);
	Request.AttackSequence++;
	ExpectFailure(TEXT("Stale sequence"),
		RunnerWriterFixtures::MakeState(), Request);
	Request = RunnerWriterFixtures::MakeRequest(
		RunnerWriterFixtures::MidfieldRunnerId);
	Request.RequestingSide = EInitialTurnOrderPlayer::PlayerB;
	ExpectFailure(TEXT("Wrong side"),
		RunnerWriterFixtures::MakeState(), Request);
	ExpectFailure(TEXT("Carrier rejected"),
		RunnerWriterFixtures::MakeState(),
		RunnerWriterFixtures::MakeRequest(
			RunnerWriterFixtures::CarrierId));
	ExpectFailure(TEXT("Empty runner rejected"),
		RunnerWriterFixtures::MakeState(),
		RunnerWriterFixtures::MakeRequest(NAME_None));
	ExpectFailure(TEXT("Not deployed rejected"),
		RunnerWriterFixtures::MakeState(),
		RunnerWriterFixtures::MakeRequest(
			RunnerWriterFixtures::MissingRunnerId));
	ExpectFailure(TEXT("GK rejected"),
		RunnerWriterFixtures::MakeState(),
		RunnerWriterFixtures::MakeRequest(
			RunnerWriterFixtures::GoalkeeperId));
	ExpectFailure(TEXT("Position rejected"),
		RunnerWriterFixtures::MakeState(),
		RunnerWriterFixtures::MakeRequest(
			RunnerWriterFixtures::DefenseRunnerId));
	ExpectFailure(TEXT("Non-forward rejected"),
		RunnerWriterFixtures::MakeState(ESkillRuleType::ThroughBall),
		RunnerWriterFixtures::MakeRequest(
			RunnerWriterFixtures::MidfieldRunnerId));

	FMatchPlayState InvalidSnapshot =
		RunnerWriterFixtures::MakeState();
	InvalidSnapshot.CardSnapshotAuthority
		.PlayerACardSnapshots.Cards[0].CardId = NAME_None;
	ExpectFailure(TEXT("Invalid snapshot authority"), InvalidSnapshot,
		RunnerWriterFixtures::MakeRequest(
			RunnerWriterFixtures::MidfieldRunnerId));

	FMatchPlayState InvalidCatalog =
		RunnerWriterFixtures::MakeState(ESkillRuleType::ThroughBall);
	InvalidCatalog.DeploymentSlotCatalog.Slots[0].SlotId = NAME_None;
	ExpectFailure(TEXT("Invalid slot authority"), InvalidCatalog,
		RunnerWriterFixtures::MakeRequest(
			RunnerWriterFixtures::ForwardRunnerId));
	return true;
}

RUNNER_WRITER_TEST(FRunnerWriterRepeatTest, "RepeatedSelectionRejected")

bool FRunnerWriterRepeatTest::RunTest(const FString& Parameters)
{
	const auto First =
		FMatchPlayCurrentAttackRunnerSelectionWriter::Select(
			RunnerWriterFixtures::MakeState(),
			RunnerWriterFixtures::MakeRequest(
				RunnerWriterFixtures::MidfieldRunnerId));
	TestTrue(TEXT("First succeeds"), First.bSuccess);
	for (const FName RunnerId : {
		RunnerWriterFixtures::MidfieldRunnerId,
		RunnerWriterFixtures::AttackRunnerId})
	{
		const auto Repeat =
			FMatchPlayCurrentAttackRunnerSelectionWriter::Select(
				First.AfterState,
				RunnerWriterFixtures::MakeRequest(RunnerId));
		TestFalse(TEXT("Repeat fails"), Repeat.bSuccess);
		TestTrue(TEXT("Repeat does not overwrite"),
			RunnerWriterFixtures::AreStatesEqual(
				Repeat.AfterState, First.AfterState));
	}
	return true;
}

RUNNER_WRITER_TEST(
	FRunnerWriterParticipantFirstMidfieldStructuralTest,
	"ParticipantFirstMidfieldRunnerRemainsStructurallyLegal")

bool FRunnerWriterParticipantFirstMidfieldStructuralTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayState Before = RunnerWriterFixtures::MakeState();
	Before.CurrentAttack.ActionPreparation.SkillId = NAME_None;
	Before.CurrentAttack.ActionPreparation.ActionType = ESkillRuleType::None;
	Before.CurrentAttack.ActionPreparation.bSkillSelectionDeferred = true;

	const auto Result =
		FMatchPlayCurrentAttackRunnerSelectionWriter::Select(
			Before,
			RunnerWriterFixtures::MakeRequest(
				RunnerWriterFixtures::MidfieldRunnerId));

	TestTrue(TEXT("Participant-first midfield Runner is structurally accepted"),
		Result.bSuccess && Result.LegalityResult.bIsLegal);
	TestEqual(TEXT("Accepted midfield Runner is persisted"),
		Result.AfterState.CurrentAttack.ActionPreparation.RunnerCardId,
		RunnerWriterFixtures::MidfieldRunnerId);
	TestEqual(TEXT("Participant-first flow continues to Helper"),
		Result.AfterState.CurrentAttack.SelectionStage,
		EMatchPlayCurrentAttackSelectionStage::AwaitingHelper);
	TestTrue(TEXT("Tactic remains deferred after structural Runner selection"),
		Result.AfterState.CurrentAttack.ActionPreparation.bSkillSelectionDeferred
			&& Result.AfterState.CurrentAttack.ActionPreparation.SkillId.IsNone()
			&& Result.AfterState.CurrentAttack.ActionPreparation.ActionType
				== ESkillRuleType::None);
	TestTrue(TEXT("Runner writer does not mutate input"),
		RunnerWriterFixtures::AreStatesEqual(Result.BeforeState, Before));
	return true;
}

#undef RUNNER_WRITER_TEST

#endif
