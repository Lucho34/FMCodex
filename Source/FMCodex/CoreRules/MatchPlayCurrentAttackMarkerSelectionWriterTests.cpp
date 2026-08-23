#include "MatchPlayCurrentAttackMarkerSelectionWriter.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "MatchPlayCurrentAttackMarkerSelectionTestFixtures.h"
#include "MatchPlayCurrentAttackHelperSelectionWriter.h"
#include "MatchPlayCurrentAttackRunnerSelectionWriter.h"
#include "MatchPlayCurrentAttackSkillSelectionWriter.h"
#include "Misc/AutomationTest.h"

#include <type_traits>

namespace MarkerSelectionWriterTests
{
	using namespace
		FMCodex::Tests::MatchPlayCurrentAttackMarkerSelection;

	bool ExpectFailure(
		FAutomationTestBase& Test,
		const TCHAR* Context,
		const FMatchPlayState& State,
		const FMatchPlayCurrentAttackMarkerSelectionRequest& Request,
		const EMatchPlayCurrentAttackMarkerSelectionErrorCode
			ExpectedLegalityError)
	{
		const FMatchPlayState Original = State;
		const auto Result =
			FMatchPlayCurrentAttackMarkerSelectionWriter::Select(
				State,
				Request);
		Test.TestFalse(
			*FString::Printf(TEXT("%s fails"), Context),
			Result.bSuccess);
		Test.TestEqual(
			*FString::Printf(TEXT("%s top-level error"), Context),
			Result.ErrorCode,
			EMatchPlayCurrentAttackMarkerSelectionWriterErrorCode
				::LegalityFailed);
		Test.TestEqual(
			*FString::Printf(TEXT("%s underlying error"), Context),
			Result.LegalityResult.ErrorCode,
			ExpectedLegalityError);
		Test.TestTrue(
			*FString::Printf(TEXT("%s BeforeState preserved"), Context),
			AreStatesEqual(Result.BeforeState, Original));
		Test.TestTrue(
			*FString::Printf(TEXT("%s AfterState atomic"), Context),
			AreStatesEqual(Result.AfterState, Original));
		Test.TestTrue(
			*FString::Printf(TEXT("%s input unchanged"), Context),
			AreStatesEqual(State, Original));
		return true;
	}
}

#define MARKER_WRITER_TEST(TestClass, TestName) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
		TestClass, \
		"FMCodex.CoreRules.MatchPlayCurrentAttackMarkerSelection.Writer." TestName, \
		EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

MARKER_WRITER_TEST(
	FMarkerWriterContractTest,
	"DefaultsReflectionAndSignature")

bool FMarkerWriterContractTest::RunTest(const FString& Parameters)
{
	const FMatchPlayCurrentAttackMarkerSelectionWriterResult Result;
	TestFalse(TEXT("Default fails"), Result.bSuccess);
	TestTrue(TEXT("Default selected marker empty"),
		Result.SelectedMarkerCardId.IsNone());
	TestNotNull(TEXT("Result reflected"),
		FMatchPlayCurrentAttackMarkerSelectionWriterResult
			::StaticStruct());
	using FSelectSignature =
		FMatchPlayCurrentAttackMarkerSelectionWriterResult (*)(
			const FMatchPlayState&,
			const FMatchPlayCurrentAttackMarkerSelectionRequest&);
	TestTrue(TEXT("Single writer signature"),
		(std::is_same_v<
			decltype(
				&FMatchPlayCurrentAttackMarkerSelectionWriter::Select),
			FSelectSignature>));
	return true;
}

MARKER_WRITER_TEST(
	FMarkerWriterSuccessTest,
	"FreezesMarkerAndEntersAwaitingRunner")

bool FMarkerWriterSuccessTest::RunTest(const FString& Parameters)
{
	using namespace
		FMCodex::Tests::MatchPlayCurrentAttackMarkerSelection;
	const FMatchPlayState BeforeState = MakeState();
	FMatchPlayState ExpectedState = BeforeState;
	ExpectedState.CurrentAttack.ActionPreparation.MarkerCardId =
		MarkerOneId;
	ExpectedState.CurrentAttack.ActionPreparation.bSkillSelectionDeferred =
		true;
	ExpectedState.CurrentAttack.SelectionStage =
		EMatchPlayCurrentAttackSelectionStage::AwaitingRunner;
	const auto Result =
		FMatchPlayCurrentAttackMarkerSelectionWriter::Select(
			BeforeState,
			MakeRequest());
	TestTrue(TEXT("Writer succeeds"), Result.bSuccess);
	TestTrue(TEXT("Legality succeeds"), Result.LegalityResult.bIsLegal);
	TestEqual(TEXT("Selected marker echoed"),
		Result.SelectedMarkerCardId, MarkerOneId);
	TestTrue(TEXT("Only marker, participant-first flag, and stage change"),
		AreStatesEqual(Result.AfterState, ExpectedState));
	TestTrue(TEXT("Skill remains unselected"),
		Result.AfterState.CurrentAttack.ActionPreparation.SkillId.IsNone());
	TestEqual(TEXT("Action family remains unfrozen"),
		Result.AfterState.CurrentAttack.ActionPreparation.ActionType,
		ESkillRuleType::None);
	TestTrue(TEXT("Input state unchanged"),
		AreStatesEqual(Result.BeforeState, BeforeState));
	TestEqual(TEXT("Carrier preserved"),
		Result.AfterState.CurrentAttack.ActionPreparation.CarrierCardId,
		CarrierId);
	TestEqual(TEXT("Phase remains Resolution"),
		Result.AfterState.CurrentAttack.Phase,
		EMatchPlayCurrentAttackPhase::Resolution);
	TestFalse(TEXT("Final action remains absent"),
		Result.AfterState.CurrentAttack.bHasSelectedAction);
	const FMatchPlayCurrentAttackSelectedAction EmptySelectedAction;
	TestTrue(TEXT("Final selected payload remains empty"),
		FMatchPlayCurrentAttackSelectedAction::StaticStruct()
			->CompareScriptStruct(
				&Result.AfterState.CurrentAttack.SelectedAction,
				&EmptySelectedAction,
				0));
	return true;
}

MARKER_WRITER_TEST(
	FMarkerWriterCrossDefersSkillTest,
	"CrossOnlyDefersSkillUntilAfterParticipants")

bool FMarkerWriterCrossDefersSkillTest::RunTest(const FString& Parameters)
{
	using namespace
		FMCodex::Tests::MatchPlayCurrentAttackMarkerSelection;
	const FName CrossSkillId(TEXT("Skill.Cross.Deferred"));
	const FName AlternateCrossSkillId(TEXT("Skill.Cross.Deferred.Alternate"));
	FMatchPlayState BeforeState = MakeState();
	BeforeState.CardSnapshotAuthority.PlayerACardSnapshots.Cards[0]
		.SkillIds = { CrossSkillId, AlternateCrossSkillId };
	FSkillRuleSnapshot CrossRule;
	CrossRule.SkillId = CrossSkillId;
	CrossRule.SkillType = ESkillRuleType::Cross;
	CrossRule.MinTriggerActionPoint = 4;
	CrossRule.MaxTriggerActionPoint = 6;
	FSkillRuleSnapshot AlternateCrossRule = CrossRule;
	AlternateCrossRule.SkillId = AlternateCrossSkillId;
	FSkillRuleSnapshotSet Rules;
	Rules.SkillRules = { CrossRule, AlternateCrossRule };

	const auto Result =
		FMatchPlayCurrentAttackMarkerSelectionWriter::SelectWithSkillRules(
			BeforeState,
			Rules,
			MakeRequest());
	TestTrue(TEXT("Cross marker succeeds"), Result.bSuccess);
	TestTrue(TEXT("Deferred path is explicit"), Result.bDeferredSkillSelection);
	TestEqual(TEXT("Action family is not frozen before Skill selection"),
		Result.DeferredActionType, ESkillRuleType::None);
	TestEqual(TEXT("Marker advances to Runner, not Skill"),
		Result.AfterState.CurrentAttack.SelectionStage,
		EMatchPlayCurrentAttackSelectionStage::AwaitingRunner);
	TestTrue(TEXT("Authority marks deferred Skill selection"),
		Result.AfterState.CurrentAttack.ActionPreparation
			.bSkillSelectionDeferred);
	TestTrue(TEXT("Specific Skill remains unselected"),
		Result.AfterState.CurrentAttack.ActionPreparation.SkillId.IsNone());
	TestEqual(TEXT("Action family remains unfrozen"),
		Result.AfterState.CurrentAttack.ActionPreparation.ActionType,
		ESkillRuleType::None);
	return true;
}

MARKER_WRITER_TEST(
	FMarkerWriterMixedFamiliesRemainParticipantFirstTest,
	"MixedFamiliesRemainParticipantFirst")

bool FMarkerWriterMixedFamiliesRemainParticipantFirstTest::RunTest(
	const FString& Parameters)
{
	using namespace
		FMCodex::Tests::MatchPlayCurrentAttackMarkerSelection;
	const FName CrossSkillId(TEXT("Skill.Cross.Ambiguous"));
	const FName LongShotSkillId(TEXT("Skill.LongShot.Ambiguous"));
	FMatchPlayState BeforeState = MakeState();
	BeforeState.CardSnapshotAuthority.PlayerACardSnapshots.Cards[0]
		.SkillIds = { CrossSkillId, LongShotSkillId };
	FSkillRuleSnapshot CrossRule;
	CrossRule.SkillId = CrossSkillId;
	CrossRule.SkillType = ESkillRuleType::Cross;
	CrossRule.MinTriggerActionPoint = 4;
	CrossRule.MaxTriggerActionPoint = 6;
	FSkillRuleSnapshot LongShotRule = CrossRule;
	LongShotRule.SkillId = LongShotSkillId;
	LongShotRule.SkillType = ESkillRuleType::LongShot;
	FSkillRuleSnapshotSet Rules;
	Rules.SkillRules = { CrossRule, LongShotRule };

	const auto Result =
		FMatchPlayCurrentAttackMarkerSelectionWriter::SelectWithSkillRules(
			BeforeState,
			Rules,
			MakeRequest());
	TestTrue(TEXT("Ambiguous marker still succeeds"), Result.bSuccess);
	TestTrue(TEXT("Mixed action families use participant-first flow"),
		Result.bDeferredSkillSelection);
	TestEqual(TEXT("Mixed action families advance to Runner"),
		Result.AfterState.CurrentAttack.SelectionStage,
		EMatchPlayCurrentAttackSelectionStage::AwaitingRunner);
	TestTrue(TEXT("Participant-first marker is set"),
		Result.AfterState.CurrentAttack.ActionPreparation
			.bSkillSelectionDeferred);
	TestEqual(TEXT("No action family is guessed"),
		Result.AfterState.CurrentAttack.ActionPreparation.ActionType,
		ESkillRuleType::None);
	TestTrue(TEXT("Specific Skill remains unselected"),
		Result.AfterState.CurrentAttack.ActionPreparation.SkillId.IsNone());
	return true;
}

MARKER_WRITER_TEST(
	FCrossParticipantFirstAuthoritySequenceTest,
	"CrossAuthoritySequenceMarkerRunnerHelperSkill")

bool FCrossParticipantFirstAuthoritySequenceTest::RunTest(
	const FString& Parameters)
{
	using namespace
		FMCodex::Tests::MatchPlayCurrentAttackMarkerSelection;
	const FName CrossSkillId(TEXT("Skill.Cross.Sequence"));
	const FName RunnerId(TEXT("PlayerA.CrossRunner"));
	const FName HelperId(TEXT("PlayerB.CrossHelper"));
	const FName RunnerSlot(TEXT("Physical.NearB.Runner"));
	const FName HelperSlot(TEXT("Physical.NearB.Helper"));
	FMatchPlayState State = MakeState();
	State.CurrentAttack.DeploymentPlacements.RemoveAll(
		[](const FMatchPlayDeploymentPlacement& Placement)
		{
			return Placement.CardId == AttackerOnlyMarkerId;
		});
	State.CardSnapshotAuthority.PlayerACardSnapshots.Cards[0]
		.SkillIds = { CrossSkillId };
	FPlayerCardRuleSnapshot Runner = MakeCard(RunnerId);
	Runner.PositionTypes = { EPlayerPositionType::Attack };
	State.CardSnapshotAuthority.PlayerACardSnapshots.Cards.Add(Runner);
	State.CardSnapshotAuthority.PlayerBCardSnapshots.Cards.Add(
		MakeCard(HelperId));
	State.DeploymentSlotCatalog.Slots.Add(
		MakeSlot(RunnerSlot, EMatchPlayNeutralSlotSide::NearPlayerB));
	State.DeploymentSlotCatalog.Slots.Add(
		MakeSlot(HelperSlot, EMatchPlayNeutralSlotSide::NearPlayerB));
	State.CurrentAttack.DeploymentPlacements.Add(MakePlacement(
		EInitialTurnOrderPlayer::PlayerA, RunnerId, RunnerSlot));
	State.CurrentAttack.DeploymentPlacements.Add(MakePlacement(
		EInitialTurnOrderPlayer::PlayerB, HelperId, HelperSlot));
	FSkillRuleSnapshot CrossRule;
	CrossRule.SkillId = CrossSkillId;
	CrossRule.SkillType = ESkillRuleType::Cross;
	CrossRule.MinTriggerActionPoint = 4;
	CrossRule.MaxTriggerActionPoint = 6;
	FSkillRuleSnapshotSet Rules;
	Rules.SkillRules = { CrossRule };

	const auto Marker =
		FMatchPlayCurrentAttackMarkerSelectionWriter::SelectWithSkillRules(
			State, Rules, MakeRequest());
	TestTrue(TEXT("Marker succeeds"), Marker.bSuccess);
	TestEqual(TEXT("After Marker awaits Runner"),
		Marker.AfterState.CurrentAttack.SelectionStage,
		EMatchPlayCurrentAttackSelectionStage::AwaitingRunner);

	FMatchPlayCurrentAttackRunnerSelectionRequest RunnerRequest;
	RunnerRequest.AttackSequence = ValidAttackSequence;
	RunnerRequest.RequestingSide = EInitialTurnOrderPlayer::PlayerA;
	RunnerRequest.RunnerCardId = RunnerId;
	const auto RunnerResult =
		FMatchPlayCurrentAttackRunnerSelectionWriter::Select(
			Marker.AfterState, RunnerRequest);
	if (!RunnerResult.bSuccess)
	{
		AddError(FString::Printf(TEXT("Runner failure: %s"),
			*RunnerResult.ErrorMessage));
		return false;
	}
	TestTrue(TEXT("Runner succeeds"), RunnerResult.bSuccess);
	TestEqual(TEXT("After Runner awaits Helper"),
		RunnerResult.AfterState.CurrentAttack.SelectionStage,
		EMatchPlayCurrentAttackSelectionStage::AwaitingHelper);

	FMatchPlayCurrentAttackHelperSelectionRequest HelperRequest;
	HelperRequest.AttackSequence = ValidAttackSequence;
	HelperRequest.RequestingSide = EInitialTurnOrderPlayer::PlayerB;
	HelperRequest.HelperCardId = HelperId;
	const auto HelperResult =
		FMatchPlayCurrentAttackHelperSelectionWriter::Select(
			RunnerResult.AfterState, HelperRequest);
	if (!HelperResult.bSuccess)
	{
		AddError(FString::Printf(TEXT("Helper failure: %s"),
			*HelperResult.ErrorMessage));
		return false;
	}
	TestTrue(TEXT("Helper succeeds"), HelperResult.bSuccess);
	TestEqual(TEXT("After Helper finally awaits Skill"),
		HelperResult.AfterState.CurrentAttack.SelectionStage,
		EMatchPlayCurrentAttackSelectionStage::AwaitingSkill);

	FMatchPlayCurrentAttackSkillSelectionRequest SkillRequest;
	SkillRequest.AttackSequence = ValidAttackSequence;
	SkillRequest.RequestingSide = EInitialTurnOrderPlayer::PlayerA;
	SkillRequest.SkillId = CrossSkillId;
	const auto SkillResult =
		FMatchPlayCurrentAttackSkillSelectionWriter::Select(
			HelperResult.AfterState, Rules, SkillRequest);
	TestTrue(TEXT("Final Cross Skill succeeds"), SkillResult.bSuccess);
	TestEqual(TEXT("Cross then awaits branch intent"),
		SkillResult.AfterState.CurrentAttack.SelectionStage,
		EMatchPlayCurrentAttackSelectionStage::AwaitingBranchIntent);
	TestEqual(TEXT("Runner remains authoritative"),
		SkillResult.AfterState.CurrentAttack.ActionPreparation.RunnerCardId,
		RunnerId);
	TestEqual(TEXT("Helper remains authoritative"),
		SkillResult.AfterState.CurrentAttack.ActionPreparation.HelperCardId,
		HelperId);
	return true;
}

#define MARKER_WRITER_FAILURE_TEST( \
	TestClass, TestName, Mutation, ExpectedError) \
	MARKER_WRITER_TEST(TestClass, TestName) \
	bool TestClass::RunTest(const FString& Parameters) \
	{ \
		using namespace \
			FMCodex::Tests::MatchPlayCurrentAttackMarkerSelection; \
		FMatchPlayState State = MakeState(); \
		FMatchPlayCurrentAttackMarkerSelectionRequest Request = \
			MakeRequest(); \
		Mutation; \
		return MarkerSelectionWriterTests::ExpectFailure( \
			*this, TEXT(TestName), State, Request, ExpectedError); \
	}

MARKER_WRITER_FAILURE_TEST(
	FMarkerWriterStaleSequenceTest,
	"StaleSequenceAtomic",
	Request.AttackSequence = ValidAttackSequence + 1,
	EMatchPlayCurrentAttackMarkerSelectionErrorCode
		::AttackSequenceMismatch)

MARKER_WRITER_FAILURE_TEST(
	FMarkerWriterWrongSideTest,
	"WrongSideAtomic",
	Request.RequestingSide = EInitialTurnOrderPlayer::PlayerA,
	EMatchPlayCurrentAttackMarkerSelectionErrorCode
		::RequestingSideIsNotCurrentDefender)

MARKER_WRITER_FAILURE_TEST(
	FMarkerWriterWrongStageTest,
	"WrongStageAtomic",
	State.CurrentAttack.ActionPreparation.MarkerCardId = MarkerOneId;
	State.CurrentAttack.SelectionStage =
		EMatchPlayCurrentAttackSelectionStage::AwaitingSkill,
	EMatchPlayCurrentAttackMarkerSelectionErrorCode
		::WrongSelectionStage)

MARKER_WRITER_FAILURE_TEST(
	FMarkerWriterDifferentAreaTest,
	"DifferentAreaAtomic",
	Request.MarkerCardId = OtherAreaMarkerId,
	EMatchPlayCurrentAttackMarkerSelectionErrorCode
		::MarkerNotInCarrierPhysicalArea)

MARKER_WRITER_FAILURE_TEST(
	FMarkerWriterGoalkeeperTest,
	"GoalkeeperAtomic",
	Request.MarkerCardId = GoalkeeperMarkerId,
	EMatchPlayCurrentAttackMarkerSelectionErrorCode
		::MarkerIsGoalkeeper)

MARKER_WRITER_FAILURE_TEST(
	FMarkerWriterSnapshotFailureTest,
	"SnapshotFailureAtomic",
	State.CurrentAttack.DeploymentPlacements.Add(
		MakePlacement(
			EInitialTurnOrderPlayer::PlayerB,
			MissingSnapshotMarkerId,
			MissingSnapshotSlotId));
	Request.MarkerCardId = MissingSnapshotMarkerId,
	EMatchPlayCurrentAttackMarkerSelectionErrorCode
		::MarkerSnapshotQueryFailed)

MARKER_WRITER_FAILURE_TEST(
	FMarkerWriterAreaFailureTest,
	"AreaFailureAtomic",
	State.CurrentAttack.DeploymentPlacements[1].SlotId =
		FName(TEXT("Physical.Unknown")),
	EMatchPlayCurrentAttackMarkerSelectionErrorCode
		::PhysicalAreaQueryFailed)

MARKER_WRITER_TEST(
	FMarkerWriterRepeatedSubmissionTest,
	"RepeatedAndReplacementSubmissionsRejected")

bool FMarkerWriterRepeatedSubmissionTest::RunTest(
	const FString& Parameters)
{
	using namespace
		FMCodex::Tests::MatchPlayCurrentAttackMarkerSelection;
	const FMatchPlayState Initial = MakeState();
	const auto First =
		FMatchPlayCurrentAttackMarkerSelectionWriter::Select(
			Initial,
			MakeRequest());
	TestTrue(TEXT("First selection succeeds"), First.bSuccess);
	const FMatchPlayState Frozen = First.AfterState;

	const auto Repeat =
		FMatchPlayCurrentAttackMarkerSelectionWriter::Select(
			Frozen,
			MakeRequest());
	TestFalse(TEXT("Same marker repeat rejected"), Repeat.bSuccess);
	TestEqual(TEXT("Repeat rejected by stage"),
		Repeat.LegalityResult.ErrorCode,
		EMatchPlayCurrentAttackMarkerSelectionErrorCode
			::WrongSelectionStage);
	TestTrue(TEXT("Repeat atomic"),
		AreStatesEqual(Repeat.AfterState, Frozen));

	FMatchPlayCurrentAttackMarkerSelectionRequest Replacement =
		MakeRequest(MarkerTwoId);
	const auto Replace =
		FMatchPlayCurrentAttackMarkerSelectionWriter::Select(
			Frozen,
			Replacement);
	TestFalse(TEXT("Different marker replacement rejected"),
		Replace.bSuccess);
	TestEqual(TEXT("Replacement rejected by stage"),
		Replace.LegalityResult.ErrorCode,
		EMatchPlayCurrentAttackMarkerSelectionErrorCode
			::WrongSelectionStage);
	TestTrue(TEXT("Replacement atomic"),
		AreStatesEqual(Replace.AfterState, Frozen));
	return true;
}

MARKER_WRITER_TEST(
	FMarkerWriterAuthorityIsolationTest,
	"PreservesAllUnrelatedAuthorities")

bool FMarkerWriterAuthorityIsolationTest::RunTest(
	const FString& Parameters)
{
	using namespace
		FMCodex::Tests::MatchPlayCurrentAttackMarkerSelection;
	const FMatchPlayState BeforeState = MakeState();
	const auto Result =
		FMatchPlayCurrentAttackMarkerSelectionWriter::Select(
			BeforeState,
			MakeRequest());
	TestTrue(TEXT("Writer succeeds"), Result.bSuccess);
	TestTrue(TEXT("Runtime and scores unchanged"),
		FMatchRuntimeState::StaticStruct()->CompareScriptStruct(
			&Result.AfterState.RuntimeState,
			&BeforeState.RuntimeState,
			0));
	TestTrue(TEXT("CardUsage unchanged"),
		FMatchCardUsageState::StaticStruct()->CompareScriptStruct(
			&Result.AfterState.CardUsageState,
			&BeforeState.CardUsageState,
			0));
	bool bPlacementsUnchanged =
		Result.AfterState.CurrentAttack.DeploymentPlacements.Num()
		== BeforeState.CurrentAttack.DeploymentPlacements.Num();
	for (int32 Index = 0;
		bPlacementsUnchanged
			&& Index
				< BeforeState.CurrentAttack.DeploymentPlacements.Num();
		++Index)
	{
		bPlacementsUnchanged =
			FMatchPlayDeploymentPlacement::StaticStruct()
				->CompareScriptStruct(
					&Result.AfterState.CurrentAttack
						.DeploymentPlacements[Index],
					&BeforeState.CurrentAttack
						.DeploymentPlacements[Index],
					0);
	}
	TestTrue(TEXT("Deployment placements unchanged"),
		bPlacementsUnchanged);
	TestEqual(TEXT("ActionPoint unchanged"),
		Result.AfterState.CurrentAttack.ActionPoint,
		BeforeState.CurrentAttack.ActionPoint);
	TestEqual(TEXT("Goalkeeper current state unchanged"),
		Result.AfterState.CurrentAttack
			.bCurrentDefenseGoalkeeperActivated,
		BeforeState.CurrentAttack
			.bCurrentDefenseGoalkeeperActivated);
	TestTrue(TEXT("Goalkeeper persistent state unchanged"),
		FMatchPlayGoalkeeperUsageState::StaticStruct()
			->CompareScriptStruct(
				&Result.AfterState.GoalkeeperUsageState,
				&BeforeState.GoalkeeperUsageState,
				0));
	return true;
}

#undef MARKER_WRITER_FAILURE_TEST
#undef MARKER_WRITER_TEST

#endif
