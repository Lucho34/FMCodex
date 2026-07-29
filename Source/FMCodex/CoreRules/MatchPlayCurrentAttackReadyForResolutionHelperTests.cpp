#include "MatchPlayCurrentAttackReadyForResolutionValidator.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "MatchPlayCurrentAttackHelperSelectionTestFixtures.h"
#include "MatchPlayCurrentAttackResolutionBinding.h"
#include "MatchPlayCurrentAttackSkillSelectionTestFixtures.h"
#include "MatchPlayCurrentAttackSkillSelectionWriter.h"
#include "Misc/AutomationTest.h"

namespace HelperFixtures =
	FMCodex::Tests::MatchPlayCurrentAttackHelperSelection;
namespace SkillFixtures =
	FMCodex::Tests::MatchPlayCurrentAttackSkillSelection;

#define HELPER_READY_TEST(TestClass, TestName) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
		TestClass, \
		"FMCodex.CoreRules.MatchPlayCurrentAttackHelperSelection.Ready." TestName, \
		EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

HELPER_READY_TEST(FHelperReadyPresentAbsentTest,
	"RunnerSkillsPresentAndAbsent")

bool FHelperReadyPresentAbsentTest::RunTest(
	const FString& Parameters)
{
	for (const ESkillRuleType Type : {
		ESkillRuleType::PassControl,
		ESkillRuleType::Cross,
		ESkillRuleType::ThroughBall})
	{
		for (const bool bHasHelper : {false, true})
		{
			const FMatchPlayState State =
				HelperFixtures::MakeReadyState(Type, bHasHelper);
			const FMatchPlayState Original = State;
			const auto Result =
				FMatchPlayCurrentAttackReadyForResolutionValidator
					::Validate(State);
			TestTrue(TEXT("Ready variant valid"), Result.bSuccess);
			if (!bHasHelper)
			{
				TestEqual(TEXT("Absent Helper authority not queried"),
					Result.HelperAuthorityResult
						.MatchingPlacementCount,
					0);
				TestTrue(TEXT("Absent Helper snapshot untouched"),
					Result.HelperAuthorityResult
						.SnapshotQueryResult.CardId.IsNone());
			}
			TestTrue(TEXT("Ready validation read-only"),
				HelperFixtures::AreStatesEqual(State, Original));
		}
	}
	return true;
}

HELPER_READY_TEST(FHelperReadyCorruptionTest,
	"StructuralAndAuthorityCorruption")

bool FHelperReadyCorruptionTest::RunTest(
	const FString& Parameters)
{
	auto ExpectFailure = [this](
		const TCHAR* Label,
		FMatchPlayState State)
	{
		const auto Result =
			FMatchPlayCurrentAttackReadyForResolutionValidator
				::Validate(State);
		TestFalse(Label, Result.bSuccess);
	};

	FMatchPlayState MissingRunner =
		HelperFixtures::MakeReadyState();
	MissingRunner.CurrentAttack.SelectedAction.RunnerCardId = NAME_None;
	ExpectFailure(TEXT("Missing Runner"), MissingRunner);

	FMatchPlayState MissingHelper =
		HelperFixtures::MakeReadyState();
	MissingHelper.CurrentAttack.SelectedAction.HelperCardId = NAME_None;
	ExpectFailure(TEXT("Missing Helper identity"), MissingHelper);

	FMatchPlayState AbsentGarbage =
		HelperFixtures::MakeReadyState(
			ESkillRuleType::PassControl,
			false);
	AbsentGarbage.CurrentAttack.SelectedAction.HelperCardId =
		HelperFixtures::HelperId;
	ExpectFailure(TEXT("Absent Helper garbage"), AbsentGarbage);

	FMatchPlayState Marker =
		HelperFixtures::MakeReadyState();
	Marker.CurrentAttack.SelectedAction.HelperCardId =
		HelperFixtures::MarkerId;
	ExpectFailure(TEXT("Helper matches Marker"), Marker);

	FMatchPlayState Goalkeeper =
		HelperFixtures::MakeReadyState();
	Goalkeeper.CurrentAttack.SelectedAction.HelperCardId =
		HelperFixtures::GoalkeeperId;
	ExpectFailure(TEXT("Helper is GK"), Goalkeeper);

	FMatchPlayState MissingSnapshot =
		HelperFixtures::MakeReadyState();
	MissingSnapshot.CurrentAttack.SelectedAction.HelperCardId =
		HelperFixtures::MissingSnapshotId;
	ExpectFailure(TEXT("Missing Helper snapshot"), MissingSnapshot);

	FMatchPlayState Preparation =
		HelperFixtures::MakeReadyState();
	Preparation.CurrentAttack.ActionPreparation.SkillId =
		HelperFixtures::PassControlSkillId;
	ExpectFailure(TEXT("Preparation coexistence"), Preparation);
	return true;
}

HELPER_READY_TEST(FHelperReadyNoRunnerCompatibilityTest,
	"LongShotCutInsideCompatibility")

bool FHelperReadyNoRunnerCompatibilityTest::RunTest(
	const FString& Parameters)
{
	struct FCase
	{
		FName SkillId;
		ESkillRuleType Type;
	};
	for (const FCase& Case : {
		FCase{
			SkillFixtures::LongShotSkillId,
			ESkillRuleType::LongShot},
		FCase{
			SkillFixtures::CutInsideSkillId,
			ESkillRuleType::CutInsideShot}})
	{
		const auto Writer =
			FMatchPlayCurrentAttackSkillSelectionWriter::Select(
				SkillFixtures::MakeState(
					{
						SkillFixtures::LongShotSkillId,
						SkillFixtures::CutInsideSkillId
					}),
				SkillFixtures::MakeRuleSet(),
				SkillFixtures::MakeRequest(Case.SkillId));
		TestTrue(TEXT("No-runner writer succeeds"), Writer.bSuccess);
		TestTrue(TEXT("Runner default empty"),
			Writer.AfterState.CurrentAttack.SelectedAction
				.RunnerCardId.IsNone());
		TestFalse(TEXT("Helper presence default false"),
			Writer.AfterState.CurrentAttack.SelectedAction.bHasHelper);
		TestTrue(TEXT("Helper default empty"),
			Writer.AfterState.CurrentAttack.SelectedAction
				.HelperCardId.IsNone());
		const auto Ready =
			FMatchPlayCurrentAttackReadyForResolutionValidator
				::Validate(Writer.AfterState);
		TestTrue(TEXT("No-runner Ready remains valid"),
			Ready.bSuccess);
	}
	return true;
}

HELPER_READY_TEST(FHelperResolutionBindingTest,
	"BindingPresentAbsentAndReadOnly")

bool FHelperResolutionBindingTest::RunTest(
	const FString& Parameters)
{
	for (const ESkillRuleType Type : {
		ESkillRuleType::PassControl,
		ESkillRuleType::Cross,
		ESkillRuleType::ThroughBall})
	{
		for (const bool bHasHelper : {false, true})
		{
			const FMatchPlayState State =
				HelperFixtures::MakeReadyState(Type, bHasHelper);
			const FMatchPlayState Original = State;
			const auto Binding =
				FMatchPlayCurrentAttackResolutionBinding::Query(
					State,
					HelperFixtures::ValidAttackSequence);
			TestTrue(TEXT("Runner Ready binds"), Binding.bSuccess);
			TestEqual(TEXT("Runner binds"),
				Binding.Binding.RunnerCardId,
				HelperFixtures::RunnerId);
			TestEqual(TEXT("Presence binds"),
				Binding.Binding.bHasHelper, bHasHelper);
			TestEqual(TEXT("Helper identity binds"),
				Binding.Binding.HelperCardId,
				bHasHelper ? HelperFixtures::HelperId : NAME_None);
			if (!bHasHelper)
			{
				TestTrue(TEXT("Binding skips absent Helper query"),
					Binding.ReadyValidationResult
						.HelperAuthorityResult
						.SnapshotQueryResult.CardId.IsNone());
			}
			TestTrue(TEXT("Binding read-only"),
				HelperFixtures::AreStatesEqual(State, Original));
		}
	}

	const FMatchPlayState Awaiting =
		HelperFixtures::MakeState();
	const auto Incomplete =
		FMatchPlayCurrentAttackResolutionBinding::Query(
			Awaiting,
			HelperFixtures::ValidAttackSequence);
	TestFalse(TEXT("AwaitingHelper rejected"),
		Incomplete.bSuccess);
	TestEqual(TEXT("AwaitingHelper exact error"),
		Incomplete.ErrorCode,
		EMatchPlayCurrentAttackResolutionBindingErrorCode
			::SelectionNotComplete);
	return true;
}

#undef HELPER_READY_TEST

#endif
