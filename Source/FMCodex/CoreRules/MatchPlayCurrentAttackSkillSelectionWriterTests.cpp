#include "MatchPlayCurrentAttackSkillSelectionWriter.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "MatchPlayCurrentAttackBranchIntentSelectionWriter.h"
#include "MatchPlayCurrentAttackSkillSelectionTestFixtures.h"
#include "Misc/AutomationTest.h"

#define SKILL_WRITER_TEST(TestClass, TestName) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
		TestClass, \
		"FMCodex.CoreRules.MatchPlayCurrentAttackSkillSelectionWriter." TestName, \
		EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

SKILL_WRITER_TEST(
	FSkillWriterNoRunnerFinalizationTest,
	"LongShotAndCutInsideAwaitBranchIntent")

bool FSkillWriterNoRunnerFinalizationTest::RunTest(
	const FString& Parameters)
{
	using namespace
		FMCodex::Tests::MatchPlayCurrentAttackSkillSelection;
	struct FCase
	{
		FName SkillId;
		ESkillRuleType Type;
	};
	const FCase Cases[] = {
		{LongShotSkillId, ESkillRuleType::LongShot},
		{CutInsideSkillId, ESkillRuleType::CutInsideShot}
	};
	const FSkillRuleSnapshotSet Rules = MakeRuleSet();
	for (const FCase& Case : Cases)
	{
		const FMatchPlayState Before = MakeState({Case.SkillId});
		const auto Result =
			FMatchPlayCurrentAttackSkillSelectionWriter::Select(
				Before,
				Rules,
				MakeRequest(Case.SkillId));
		const auto& Attack = Result.AfterState.CurrentAttack;
		TestTrue(TEXT("Writer succeeds"), Result.bSuccess);
		TestTrue(
			TEXT("Input unchanged"),
			AreStatesEqual(Before, Result.BeforeState));
		TestEqual(
			TEXT("Awaiting branch intent stage"),
			Attack.SelectionStage,
			EMatchPlayCurrentAttackSelectionStage
				::AwaitingBranchIntent);
		TestFalse(
			TEXT("No selected action yet"),
			Attack.bHasSelectedAction);
		TestEqual(
			TEXT("Frozen carrier"),
			Attack.ActionPreparation.CarrierCardId,
			CarrierId);
		TestEqual(
			TEXT("Frozen marker"),
			Attack.ActionPreparation.MarkerCardId,
			MarkerId);
		TestEqual(
			TEXT("Frozen skill"),
			Attack.ActionPreparation.SkillId,
			Case.SkillId);
		TestEqual(
			TEXT("Frozen type"),
			Attack.ActionPreparation.ActionType,
			Case.Type);
		TestTrue(
			TEXT("Selected action remains empty"),
			Attack.SelectedAction.CarrierCardId.IsNone());
		TestEqual(
			TEXT("ActionPoint unchanged"),
			Attack.ActionPoint,
			Before.CurrentAttack.ActionPoint);
		TestTrue(
			TEXT("Awaiting intent state canonical"),
			FMatchPlayCurrentAttackSelectionStateValidator::Validate(
				Attack)
				.bIsCanonical);
	}
	return true;
}

SKILL_WRITER_TEST(
	FSkillWriterParticipantFirstShotDropsIrrelevantRolesTest,
	"ParticipantFirstShotDropsIrrelevantRunnerAndHelper")

bool FSkillWriterParticipantFirstShotDropsIrrelevantRolesTest::RunTest(
	const FString& Parameters)
{
	using namespace
		FMCodex::Tests::MatchPlayCurrentAttackSkillSelection;
	const FName PreparedRunnerId(TEXT("PlayerA.PreparedRunner"));
	const FName PreparedHelperId(TEXT("PlayerB.PreparedHelper"));
	FMatchPlayState Before = MakeState({CrossSkillId, CutInsideSkillId});
	Before.CardSnapshotAuthority.PlayerACardSnapshots.Cards.Add(
		MakeCard(PreparedRunnerId));
	Before.CardSnapshotAuthority.PlayerBCardSnapshots.Cards.Add(
		MakeCard(PreparedHelperId));
	Before.CurrentAttack.DeploymentPlacements.Add(MakePlacement(
		EInitialTurnOrderPlayer::PlayerA,
		PreparedRunnerId,
		TEXT("Slot.PreparedRunner")));
	Before.CurrentAttack.DeploymentPlacements.Add(MakePlacement(
		EInitialTurnOrderPlayer::PlayerB,
		PreparedHelperId,
		TEXT("Slot.PreparedHelper")));
	Before.CurrentAttack.ActionPreparation.RunnerCardId = PreparedRunnerId;
	Before.CurrentAttack.ActionPreparation.bHasHelper = true;
	Before.CurrentAttack.ActionPreparation.HelperCardId = PreparedHelperId;
	Before.CurrentAttack.ActionPreparation.bSkillSelectionDeferred = true;
	Before.CurrentAttack.ActionPreparation.SkillId = NAME_None;
	Before.CurrentAttack.ActionPreparation.ActionType = ESkillRuleType::None;

	const auto SkillResult =
		FMatchPlayCurrentAttackSkillSelectionWriter::Select(
			Before,
			MakeRuleSet(),
			MakeRequest(CutInsideSkillId));
	TestTrue(TEXT("Participant-first Cut Inside selection succeeds"),
		SkillResult.bSuccess);
	TestEqual(TEXT("Cut Inside awaits intent"),
		SkillResult.AfterState.CurrentAttack.SelectionStage,
		EMatchPlayCurrentAttackSelectionStage::AwaitingBranchIntent);
	TestTrue(TEXT("Irrelevant prepared Runner is cleared"),
		SkillResult.AfterState.CurrentAttack.ActionPreparation
			.RunnerCardId.IsNone());
	TestFalse(TEXT("Irrelevant prepared Helper presence is cleared"),
		SkillResult.AfterState.CurrentAttack.ActionPreparation.bHasHelper);
	TestTrue(TEXT("Irrelevant prepared Helper id is cleared"),
		SkillResult.AfterState.CurrentAttack.ActionPreparation
			.HelperCardId.IsNone());
	if (!SkillResult.bSuccess)
	{
		return false;
	}

	FMatchPlayCurrentAttackBranchIntentSelectionRequest IntentRequest;
	IntentRequest.AttackSequence = ValidAttackSequence;
	IntentRequest.RequestingSide = EInitialTurnOrderPlayer::PlayerA;
	IntentRequest.Intent = EMatchPlayElectiveBranchIntent::DirectShot;
	const auto IntentResult =
		FMatchPlayCurrentAttackBranchIntentSelectionWriter::Select(
			SkillResult.AfterState,
			IntentRequest);
	TestTrue(TEXT("Cut Inside intent finalizes"), IntentResult.bSuccess);
	TestTrue(TEXT("Final selected action exists"),
		IntentResult.AfterState.CurrentAttack.bHasSelectedAction);
	TestTrue(TEXT("Final selected action omits irrelevant Runner"),
		IntentResult.AfterState.CurrentAttack.SelectedAction
			.RunnerCardId.IsNone());
	TestFalse(TEXT("Final selected action omits irrelevant Helper presence"),
		IntentResult.AfterState.CurrentAttack.SelectedAction.bHasHelper);
	TestTrue(TEXT("Final selected action omits irrelevant Helper id"),
		IntentResult.AfterState.CurrentAttack.SelectedAction
			.HelperCardId.IsNone());
	return true;
}

SKILL_WRITER_TEST(
	FSkillWriterRunnerTransitionTest,
	"RunnerSkillsEnterAwaitingRunner")

bool FSkillWriterRunnerTransitionTest::RunTest(
	const FString& Parameters)
{
	using namespace
		FMCodex::Tests::MatchPlayCurrentAttackSkillSelection;
	struct FCase
	{
		FName SkillId;
		ESkillRuleType Type;
	};
	const FCase Cases[] = {
		{PassControlSkillId, ESkillRuleType::PassControl},
		{CrossSkillId, ESkillRuleType::Cross},
		{ThroughBallSkillId, ESkillRuleType::ThroughBall}
	};
	const FSkillRuleSnapshotSet Rules = MakeRuleSet();
	for (const FCase& Case : Cases)
	{
		const FMatchPlayState Before = MakeState({Case.SkillId});
		const auto Result =
			FMatchPlayCurrentAttackSkillSelectionWriter::Select(
				Before,
				Rules,
				MakeRequest(Case.SkillId));
		const auto& Attack = Result.AfterState.CurrentAttack;
		TestTrue(TEXT("Writer succeeds"), Result.bSuccess);
		TestEqual(
			TEXT("AwaitingRunner stage"),
			Attack.SelectionStage,
			EMatchPlayCurrentAttackSelectionStage
				::AwaitingRunner);
		TestEqual(
			TEXT("Carrier preserved"),
			Attack.ActionPreparation.CarrierCardId,
			CarrierId);
		TestEqual(
			TEXT("Marker preserved"),
			Attack.ActionPreparation.MarkerCardId,
			MarkerId);
		TestEqual(
			TEXT("Skill frozen"),
			Attack.ActionPreparation.SkillId,
			Case.SkillId);
		TestEqual(
			TEXT("Action type frozen"),
			Attack.ActionPreparation.ActionType,
			Case.Type);
		TestFalse(
			TEXT("No selected action"),
			Attack.bHasSelectedAction);
		TestTrue(
			TEXT("Selected carrier empty"),
			Attack.SelectedAction.CarrierCardId.IsNone());
		TestEqual(
			TEXT("ActionPoint unchanged"),
			Attack.ActionPoint,
			Before.CurrentAttack.ActionPoint);
		TestTrue(
			TEXT("AwaitingRunner canonical"),
			FMatchPlayCurrentAttackSelectionStateValidator::Validate(
				Attack)
				.bIsCanonical);
	}
	return true;
}

SKILL_WRITER_TEST(
	FSkillWriterFailureAndRepeatAtomicityTest,
	"FailuresAndRepeatedSelectionsAreAtomic")

bool FSkillWriterFailureAndRepeatAtomicityTest::RunTest(
	const FString& Parameters)
{
	using namespace
		FMCodex::Tests::MatchPlayCurrentAttackSkillSelection;
	const FSkillRuleSnapshotSet Rules = MakeRuleSet();
	const FMatchPlayState Initial =
		MakeState({LongShotSkillId, CrossSkillId});

	auto StaleRequest = MakeRequest();
	StaleRequest.AttackSequence = ValidAttackSequence + 1;
	const auto Stale =
		FMatchPlayCurrentAttackSkillSelectionWriter::Select(
			Initial,
			Rules,
			StaleRequest);
	TestFalse(TEXT("Stale fails"), Stale.bSuccess);
	TestTrue(
		TEXT("Stale atomic"),
		AreStatesEqual(Stale.BeforeState, Stale.AfterState));

	auto WrongSideRequest = MakeRequest();
	WrongSideRequest.RequestingSide =
		EInitialTurnOrderPlayer::PlayerB;
	const auto WrongSide =
		FMatchPlayCurrentAttackSkillSelectionWriter::Select(
			Initial,
			Rules,
			WrongSideRequest);
	TestFalse(TEXT("Wrong side fails"), WrongSide.bSuccess);
	TestTrue(
		TEXT("Wrong side atomic"),
		AreStatesEqual(
			WrongSide.BeforeState,
			WrongSide.AfterState));

	FMatchPlayState OwnershipState = MakeState({CrossSkillId});
	const auto Ownership =
		FMatchPlayCurrentAttackSkillSelectionWriter::Select(
			OwnershipState,
			Rules,
			MakeRequest(LongShotSkillId));
	TestFalse(TEXT("Ownership fails"), Ownership.bSuccess);
	TestTrue(
		TEXT("Ownership atomic"),
		AreStatesEqual(
			Ownership.BeforeState,
			Ownership.AfterState));

	FSkillRuleSnapshotSet InvalidRules = MakeRuleSet();
	InvalidRules.SkillRules[0].SkillId = NAME_None;
	const auto RuleFailure =
		FMatchPlayCurrentAttackSkillSelectionWriter::Select(
			Initial,
			InvalidRules,
			MakeRequest(LongShotSkillId));
	TestFalse(TEXT("Rule failure fails"), RuleFailure.bSuccess);
	TestTrue(
		TEXT("Rule failure atomic"),
		AreStatesEqual(
			RuleFailure.BeforeState,
			RuleFailure.AfterState));

	FMatchPlayState ApState = MakeState({LongShotSkillId});
	ApState.CurrentAttack.ActionPoint = 1;
	const auto ApFailure =
		FMatchPlayCurrentAttackSkillSelectionWriter::Select(
			ApState,
			Rules,
			MakeRequest());
	TestFalse(TEXT("AP fails"), ApFailure.bSuccess);
	TestTrue(
		TEXT("AP atomic"),
		AreStatesEqual(
			ApFailure.BeforeState,
			ApFailure.AfterState));

	FSkillRuleSnapshotSet RangeRules;
	RangeRules.SkillRules = {
		MakeRule(
			LongShotSkillId,
			ESkillRuleType::LongShot,
			6,
			8)
	};
	const FMatchPlayState RangeState =
		MakeState({LongShotSkillId});
	const auto RangeFailure =
		FMatchPlayCurrentAttackSkillSelectionWriter::Select(
			RangeState,
			RangeRules,
			MakeRequest());
	TestFalse(
		TEXT("Candidate AP range mismatch fails"),
		RangeFailure.bSuccess);
	TestEqual(
		TEXT("Candidate AP range exact error"),
		RangeFailure.LegalityResult.ErrorCode,
		EMatchPlayCurrentAttackSkillSelectionErrorCode
			::ActionPointOutsideSkillRange);
	TestTrue(
		TEXT("Candidate AP range failure atomic"),
		AreStatesEqual(
			RangeFailure.BeforeState,
			RangeFailure.AfterState));

	const auto First =
		FMatchPlayCurrentAttackSkillSelectionWriter::Select(
			Initial,
			Rules,
			MakeRequest(LongShotSkillId));
	TestTrue(TEXT("First succeeds"), First.bSuccess);
	const auto RepeatSame =
		FMatchPlayCurrentAttackSkillSelectionWriter::Select(
			First.AfterState,
			Rules,
			MakeRequest(LongShotSkillId));
	TestFalse(TEXT("Repeat same fails"), RepeatSame.bSuccess);
	TestEqual(
		TEXT("Repeat same wrong stage"),
		RepeatSame.LegalityResult.ErrorCode,
		EMatchPlayCurrentAttackSkillSelectionErrorCode
			::WrongSelectionStage);
	TestTrue(
		TEXT("Repeat same atomic"),
		AreStatesEqual(
			RepeatSame.BeforeState,
			RepeatSame.AfterState));

	const auto RepeatDifferent =
		FMatchPlayCurrentAttackSkillSelectionWriter::Select(
			First.AfterState,
			Rules,
			MakeRequest(CrossSkillId));
	TestFalse(
		TEXT("Repeat different fails"),
		RepeatDifferent.bSuccess);
	TestTrue(
		TEXT("Repeat different atomic"),
		AreStatesEqual(
			RepeatDifferent.BeforeState,
			RepeatDifferent.AfterState));
	TestTrue(
		TEXT("Score preserved"),
		Initial.RuntimeState.PlayerAState.Score
			== First.AfterState.RuntimeState.PlayerAState.Score);
	TestTrue(
		TEXT("Card usage preserved"),
		FMatchCardUsageState::StaticStruct()->CompareScriptStruct(
			&Initial.CardUsageState,
			&First.AfterState.CardUsageState,
			0));
	TestTrue(
		TEXT("GK usage preserved"),
		FMatchPlayGoalkeeperUsageState::StaticStruct()
			->CompareScriptStruct(
				&Initial.GoalkeeperUsageState,
				&First.AfterState.GoalkeeperUsageState,
				0));
	return true;
}

SKILL_WRITER_TEST(
	FSkillWriterRejectsParticipantFirstMidfieldThroughBallTest,
	"RejectsDirectMidfieldThroughBallAtomically")

bool FSkillWriterRejectsParticipantFirstMidfieldThroughBallTest::RunTest(
	const FString& Parameters)
{
	using namespace
		FMCodex::Tests::MatchPlayCurrentAttackSkillSelection;
	const FMatchPlayState Before = MakeParticipantFirstRunnerState(
		EMatchPlayNeutralSlotSide::NearPlayerA);
	const auto Result = FMatchPlayCurrentAttackSkillSelectionWriter::Select(
		Before,
		MakeRuleSet(),
		MakeRequest(ThroughBallSkillId));

	TestFalse(TEXT("Direct invalid ThroughBall selection is rejected"),
		Result.bSuccess);
	TestEqual(TEXT("Direct rejection keeps the specific tactical error"),
		Result.LegalityResult.ErrorCode,
		EMatchPlayCurrentAttackSkillSelectionErrorCode::
			PreparedRunnerIncompatibleWithSkill);
	TestTrue(TEXT("Direct rejection leaves authority state byte-equivalent"),
		AreStatesEqual(Result.BeforeState, Result.AfterState)
			&& AreStatesEqual(Before, Result.AfterState));
	TestFalse(TEXT("Direct rejection creates no resolution session or RNG facts"),
		Result.AfterState.CurrentAttack.bHasResolutionSession);

	const FMatchPlayState ForwardBefore = MakeParticipantFirstRunnerState(
		EMatchPlayNeutralSlotSide::NearPlayerB);
	const auto ForwardResult =
		FMatchPlayCurrentAttackSkillSelectionWriter::Select(
			ForwardBefore,
			MakeRuleSet(),
			MakeRequest(ThroughBallSkillId));
	TestTrue(TEXT("Relative-forward ThroughBall selection succeeds"),
		ForwardResult.bSuccess);
	TestTrue(TEXT("Valid ThroughBall freezes participants and becomes ready"),
		ForwardResult.AfterState.CurrentAttack.bHasSelectedAction
			&& ForwardResult.AfterState.CurrentAttack.SelectionStage
				== EMatchPlayCurrentAttackSelectionStage::ReadyForResolution
			&& !ForwardResult.AfterState.CurrentAttack.SelectedAction
				.RunnerCardId.IsNone());
	TestFalse(TEXT("Skill selection itself still consumes no resolution RNG"),
		ForwardResult.AfterState.CurrentAttack.bHasResolutionSession);
	return true;
}

#undef SKILL_WRITER_TEST

#endif
