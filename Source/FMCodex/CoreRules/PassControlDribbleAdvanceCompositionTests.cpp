#if WITH_DEV_AUTOMATION_TESTS

#include "PassControlDribbleAdvancePlanQuery.h"

#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace PassControlDribbleAdvanceCompositionTests
{
	const FName SkillId(TEXT("Skill.PassControl.DribbleComposition"));
	const FName CarrierCardId(TEXT("DribbleComposition.Carrier"));
	const FName RunnerCardId(TEXT("DribbleComposition.Runner"));
	const FName MarkerCardId(TEXT("DribbleComposition.Marker"));
	const FName HelperCardId(TEXT("DribbleComposition.Helper"));
	const FName CarrierPlayerId(TEXT("Player.DribbleComposition.Carrier"));
	const FName RunnerPlayerId(TEXT("Player.DribbleComposition.Runner"));
	const FName MarkerPlayerId(TEXT("Player.DribbleComposition.Marker"));
	const FName HelperPlayerId(TEXT("Player.DribbleComposition.Helper"));
	const FGuid LogId(6571, 6572, 6573, 6574);

	FPlayerCardRuleSnapshot MakeCard(
		const FName CardId,
		const EPlayerPositionType Position,
		const TArray<FName>& SkillIds = {})
	{
		FPlayerCardRuleSnapshot Card;
		Card.CardId = CardId;
		Card.PositionTypes = { Position };
		Card.SkillIds = SkillIds;
		return Card;
	}

	FPlayerCardRuleSnapshotSet MakePlayerCardSnapshots(
		const int32 CarrierDribbling = 3,
		const int32 RunnerPassing = 6,
		const int32 MarkerTackling = 4,
		const int32 HelperMarking = 5,
		const bool bRunnerMidfield = true)
	{
		FPlayerCardRuleSnapshot Carrier = MakeCard(
			CarrierCardId,
			EPlayerPositionType::Midfield,
			TArray<FName>{ SkillId });
		Carrier.Attributes.Dribbling = CarrierDribbling;

		FPlayerCardRuleSnapshot Runner = MakeCard(
			RunnerCardId,
			bRunnerMidfield
				? EPlayerPositionType::Midfield
				: EPlayerPositionType::Attack);
		Runner.Attributes.Passing = RunnerPassing;

		FPlayerCardRuleSnapshot Marker = MakeCard(
			MarkerCardId,
			EPlayerPositionType::Defense);
		Marker.Attributes.Tackling = MarkerTackling;

		FPlayerCardRuleSnapshot Helper = MakeCard(
			HelperCardId,
			EPlayerPositionType::Defense);
		Helper.Attributes.Marking = HelperMarking;

		FPlayerCardRuleSnapshotSet SnapshotSet;
		SnapshotSet.Cards = { Carrier, Runner, Marker, Helper };
		return SnapshotSet;
	}

	FSkillRuleSnapshotSet MakeSkillRules()
	{
		FSkillRuleSnapshot Rule;
		Rule.SkillId = SkillId;
		Rule.SkillType = ESkillRuleType::PassControl;
		Rule.MinTriggerActionPoint = 3;
		Rule.MaxTriggerActionPoint = 6;
		FSkillRuleSnapshotSet SnapshotSet;
		SnapshotSet.SkillRules = { Rule };
		return SnapshotSet;
	}

	FPassControlDribbleAdvancePlanQueryInput MakeValidInput(
		const int32 AttackD6 = 4,
		const int32 DefenseD6 = 2)
	{
		FPassControlDribbleAdvancePlanQueryInput Input;
		Input.SkillId = SkillId;
		Input.AdvanceType = EPassControlAdvanceType::DribbleAdvance;
		Input.CarrierCardId = CarrierCardId;
		Input.RunnerCardId = RunnerCardId;
		Input.MarkerCardId = MarkerCardId;
		Input.bHasHelper = true;
		Input.HelperCardId = HelperCardId;
		Input.CurrentActionPoint = 4;
		Input.bHasExternalAttackD6 = true;
		Input.ExternalAttackD6 = AttackD6;
		Input.bHasExternalDefenseD6 = true;
		Input.ExternalDefenseD6 = DefenseD6;
		Input.LogId = LogId;
		Input.TurnIndex = 57;
		Input.CarrierPlayerId = CarrierPlayerId;
		Input.RunnerPlayerId = RunnerPlayerId;
		Input.MarkerPlayerId = MarkerPlayerId;
		Input.HelperPlayerId = HelperPlayerId;
		return Input;
	}

	FPassControlDribbleAdvancePlanQueryInput MakeInputWithoutHelper()
	{
		FPassControlDribbleAdvancePlanQueryInput Input = MakeValidInput();
		Input.bHasHelper = false;
		Input.HelperCardId = NAME_None;
		Input.HelperPlayerId = NAME_None;
		return Input;
	}

	FPassControlDribbleAdvancePlanQueryResult BuildPlan(
		const FPlayerCardRuleSnapshotSet& PlayerCardSnapshots,
		const FPassControlDribbleAdvancePlanQueryInput& Input)
	{
		return FPassControlDribbleAdvancePlanQuery::BuildPlan(
			PlayerCardSnapshots,
			MakeSkillRules(),
			Input);
	}

	struct FTestSideDribbleAdvancePlanView
	{
		EFormulaType AttackerFormulaType = EFormulaType::None;
		EFormulaType DefenderFormulaType = EFormulaType::None;
		FName AttackerCardId = NAME_None;
		FName DefenderCardId = NAME_None;
		ESingleCardFormulaAttribute AttackerAttribute =
			ESingleCardFormulaAttribute::None;
		ESingleCardFormulaAttribute DefenderAttribute =
			ESingleCardFormulaAttribute::None;
		int32 AttackD6 = 0;
		int32 DefenseD6 = 0;
		float AttackerProjectedValue = 0.0f;
		float DefenderProjectedValue = 0.0f;
	};

	// This test-side projection reads an already-built plan only. It does not
	// resolve the comparison or produce any match outcome.
	bool TryConsumeDribbleAdvanceFormulaPlan(
		const FPassControlDribbleAdvancePlanQueryResult& Result,
		FTestSideDribbleAdvancePlanView& OutView)
	{
		if (!Result.bSuccess || !Result.bHasFormulaPlan)
		{
			return false;
		}

		const FPassControlDribbleAdvanceFormulaPlan& Plan = Result.FormulaPlan;
		OutView.AttackerFormulaType = Plan.AttackerQueryInput.FormulaType;
		OutView.DefenderFormulaType = Plan.DefenderQueryInput.FormulaType;
		OutView.AttackerCardId = Plan.AttackerQueryInput.CardId;
		OutView.DefenderCardId = Plan.DefenderQueryInput.CardId;
		OutView.AttackerAttribute = Plan.AttackerQueryInput.Attribute;
		OutView.DefenderAttribute = Plan.DefenderQueryInput.Attribute;
		OutView.AttackD6 = Plan.AttackerQueryInput.ExternalD6ComparePoint;
		OutView.DefenseD6 = Plan.DefenderQueryInput.ExternalD6ComparePoint;
		OutView.AttackerProjectedValue = static_cast<float>(
			Result.CarrierSnapshotQueryResult.Snapshot.Attributes.Dribbling)
			+ Plan.AttackerQueryInput.ExternalModifier;
		OutView.DefenderProjectedValue = static_cast<float>(
			Result.MarkerSnapshotQueryResult.Snapshot.Attributes.Tackling)
			+ Plan.DefenderQueryInput.ExternalModifier;
		return true;
	}

	bool TestFailureIsNotConsumable(
		FAutomationTestBase& Test,
		const FPassControlDribbleAdvancePlanQueryResult& Result)
	{
		FTestSideDribbleAdvancePlanView View;
		Test.TestFalse(TEXT("Failure result is unsuccessful"), Result.bSuccess);
		Test.TestFalse(TEXT("Failure result has no Formula Plan"), Result.bHasFormulaPlan);
		Test.TestFalse(TEXT("Failure result is not test-side consumable"),
			TryConsumeDribbleAdvanceFormulaPlan(Result, View));
		return true;
	}
}

#define PASS_CONTROL_DRIBBLE_ADVANCE_COMPOSITION_TEST(ClassName, TestName) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
		ClassName, \
		"FMCodex.CoreRules.PassControlDribbleAdvanceComposition." TestName, \
		EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

PASS_CONTROL_DRIBBLE_ADVANCE_COMPOSITION_TEST(
	FPassControlDribbleAdvanceCompositionConsumesHelperPlanTest,
	"HelperPlanIsTestSideConsumable")

bool FPassControlDribbleAdvanceCompositionConsumesHelperPlanTest::RunTest(
	const FString& Parameters)
{
	using namespace PassControlDribbleAdvanceCompositionTests;
	const FPassControlDribbleAdvancePlanQueryResult Result = BuildPlan(
		MakePlayerCardSnapshots(), MakeValidInput());
	FTestSideDribbleAdvancePlanView View;
	TestTrue(TEXT("Plan query succeeds"), Result.bSuccess);
	TestTrue(TEXT("Formula Plan exists"), Result.bHasFormulaPlan);
	TestTrue(TEXT("Selected Helper is recorded"), Result.bHasHelper);
	TestTrue(TEXT("Test-side projection consumes the Plan"),
		TryConsumeDribbleAdvanceFormulaPlan(Result, View));
	TestEqual(TEXT("Attacker FormulaType is Finishing"),
		View.AttackerFormulaType, EFormulaType::Finishing);
	TestEqual(TEXT("Defender FormulaType is Finishing"),
		View.DefenderFormulaType, EFormulaType::Finishing);
	return true;
}

PASS_CONTROL_DRIBBLE_ADVANCE_COMPOSITION_TEST(
	FPassControlDribbleAdvanceCompositionConsumesNoHelperPlanTest,
	"NoHelperPlanIsTestSideConsumable")

bool FPassControlDribbleAdvanceCompositionConsumesNoHelperPlanTest::RunTest(
	const FString& Parameters)
{
	using namespace PassControlDribbleAdvanceCompositionTests;
	FPlayerCardRuleSnapshotSet Snapshots = MakePlayerCardSnapshots(3, 6, 4, 5);
	Snapshots.Cards.RemoveAll([](const FPlayerCardRuleSnapshot& Card)
	{
		return Card.CardId == HelperCardId;
	});
	const FPassControlDribbleAdvancePlanQueryResult Result = BuildPlan(
		Snapshots, MakeInputWithoutHelper());
	FTestSideDribbleAdvancePlanView View;
	TestTrue(TEXT("No Helper Plan query succeeds"), Result.bSuccess);
	TestFalse(TEXT("No Helper is recorded as unselected"), Result.bHasHelper);
	TestTrue(TEXT("Helper identity remains empty"),
		Result.FormulaPlan.HelperCardId.IsNone()
			&& Result.FormulaPlan.HelperPlayerId.IsNone());
	TestFalse(TEXT("No Helper Snapshot query is performed"),
		Result.HelperSnapshotQueryResult.bSuccess);
	TestTrue(TEXT("No Helper Snapshot diagnostic remains empty"),
		Result.HelperSnapshotQueryResult.CardId.IsNone());
	TestTrue(TEXT("No Helper Plan is test-side consumable"),
		TryConsumeDribbleAdvanceFormulaPlan(Result, View));
	TestEqual(TEXT("No Helper uses zero Marking with fixed bonus"),
		Result.FormulaPlan.DefenderQueryInput.ExternalModifier, 0.0f);
	return true;
}

PASS_CONTROL_DRIBBLE_ADVANCE_COMPOSITION_TEST(
	FPassControlDribbleAdvanceCompositionPreservesFieldsTest,
	"PlanPreservesParticipantsAttributesAndExternalD6")

bool FPassControlDribbleAdvanceCompositionPreservesFieldsTest::RunTest(
	const FString& Parameters)
{
	using namespace PassControlDribbleAdvanceCompositionTests;
	const FPassControlDribbleAdvancePlanQueryResult Result = BuildPlan(
		MakePlayerCardSnapshots(), MakeValidInput(6, 1));
	FTestSideDribbleAdvancePlanView View;
	TestTrue(TEXT("Plan is test-side consumable"),
		TryConsumeDribbleAdvanceFormulaPlan(Result, View));
	TestEqual(TEXT("Carrier is attacker"), View.AttackerCardId, CarrierCardId);
	TestEqual(TEXT("Marker is defender"), View.DefenderCardId, MarkerCardId);
	TestEqual(TEXT("Attacker uses Dribbling"), View.AttackerAttribute,
		ESingleCardFormulaAttribute::Dribbling);
	TestEqual(TEXT("Defender uses Tackling"), View.DefenderAttribute,
		ESingleCardFormulaAttribute::Tackling);
	TestEqual(TEXT("Attack D6 remains explicit"), View.AttackD6, 6);
	TestEqual(TEXT("Defense D6 remains explicit"), View.DefenseD6, 1);
	return true;
}

PASS_CONTROL_DRIBBLE_ADVANCE_COMPOSITION_TEST(
	FPassControlDribbleAdvanceCompositionPreservesHalfMappingTest,
	"ProjectionPreservesHalfValueMappings")

bool FPassControlDribbleAdvanceCompositionPreservesHalfMappingTest::RunTest(
	const FString& Parameters)
{
	using namespace PassControlDribbleAdvanceCompositionTests;
	const FPassControlDribbleAdvancePlanQueryResult Result = BuildPlan(
		MakePlayerCardSnapshots(3, 6, 4, 5), MakeValidInput());
	FTestSideDribbleAdvancePlanView View;
	TestTrue(TEXT("Plan is test-side consumable"),
		TryConsumeDribbleAdvanceFormulaPlan(Result, View));
	TestEqual(TEXT("Attacker modifier preserves .5"),
		Result.FormulaPlan.AttackerQueryInput.ExternalModifier, 1.5f);
	TestEqual(TEXT("Defender modifier preserves .5 plus two"),
		Result.FormulaPlan.DefenderQueryInput.ExternalModifier, 2.5f);
	TestEqual(TEXT("Projection retains attacker half value"),
		View.AttackerProjectedValue, 4.5f);
	TestEqual(TEXT("Projection retains defender half value plus two"),
		View.DefenderProjectedValue, 6.5f);
	return true;
}

PASS_CONTROL_DRIBBLE_ADVANCE_COMPOSITION_TEST(
	FPassControlDribbleAdvanceCompositionTracksRunnerAndHelperTest,
	"ResultTracksRunnerAndHelperConsistently")

bool FPassControlDribbleAdvanceCompositionTracksRunnerAndHelperTest::RunTest(
	const FString& Parameters)
{
	using namespace PassControlDribbleAdvanceCompositionTests;
	const FPassControlDribbleAdvancePlanQueryResult WithHelper = BuildPlan(
		MakePlayerCardSnapshots(), MakeValidInput());
	const FPassControlDribbleAdvancePlanQueryResult WithoutHelper = BuildPlan(
		MakePlayerCardSnapshots(), MakeInputWithoutHelper());
	TestEqual(TEXT("Runner CardId is retained"),
		WithHelper.FormulaPlan.RunnerCardId, RunnerCardId);
	TestEqual(TEXT("Runner PlayerId is retained"),
		WithHelper.FormulaPlan.RunnerPlayerId, RunnerPlayerId);
	TestTrue(TEXT("With Helper Result agrees with Input"),
		WithHelper.bHasHelper == WithHelper.Input.bHasHelper);
	TestTrue(TEXT("With Helper Plan retains Helper identity"),
		WithHelper.FormulaPlan.HelperCardId == HelperCardId
			&& WithHelper.FormulaPlan.HelperPlayerId == HelperPlayerId);
	TestTrue(TEXT("No Helper Result agrees with Input"),
		WithoutHelper.bHasHelper == WithoutHelper.Input.bHasHelper);
	TestTrue(TEXT("No Helper Plan has no Helper identity"),
		WithoutHelper.FormulaPlan.HelperCardId.IsNone()
			&& WithoutHelper.FormulaPlan.HelperPlayerId.IsNone());
	return true;
}

PASS_CONTROL_DRIBBLE_ADVANCE_COMPOSITION_TEST(
	FPassControlDribbleAdvanceCompositionRejectsInvalidAdvanceTypeTest,
	"InvalidAdvanceTypeHasNoConsumablePlan")

bool FPassControlDribbleAdvanceCompositionRejectsInvalidAdvanceTypeTest::RunTest(
	const FString& Parameters)
{
	using namespace PassControlDribbleAdvanceCompositionTests;
	FPassControlDribbleAdvancePlanQueryInput Input = MakeValidInput();
	Input.AdvanceType = EPassControlAdvanceType::PassAdvance;
	const FPassControlDribbleAdvancePlanQueryResult Result = BuildPlan(
		MakePlayerCardSnapshots(), Input);
	TestEqual(TEXT("Advance type error is retained"), Result.ErrorCode,
		EPassControlDribbleAdvancePlanQueryErrorCode::InvalidAdvanceType);
	return TestFailureIsNotConsumable(*this, Result);
}

PASS_CONTROL_DRIBBLE_ADVANCE_COMPOSITION_TEST(
	FPassControlDribbleAdvanceCompositionRejectsNonMidfieldRunnerTest,
	"NonMidfieldRunnerHasNoConsumablePlan")

bool FPassControlDribbleAdvanceCompositionRejectsNonMidfieldRunnerTest::RunTest(
	const FString& Parameters)
{
	using namespace PassControlDribbleAdvanceCompositionTests;
	const FPassControlDribbleAdvancePlanQueryResult Result = BuildPlan(
		MakePlayerCardSnapshots(3, 6, 4, 5, false), MakeValidInput());
	TestEqual(TEXT("Runner position error is retained"), Result.ErrorCode,
		EPassControlDribbleAdvancePlanQueryErrorCode::RunnerNotMidfield);
	return TestFailureIsNotConsumable(*this, Result);
}

PASS_CONTROL_DRIBBLE_ADVANCE_COMPOSITION_TEST(
	FPassControlDribbleAdvanceCompositionRejectsUnexpectedHelperTest,
	"UnexpectedHelperIdentityHasNoConsumablePlan")

bool FPassControlDribbleAdvanceCompositionRejectsUnexpectedHelperTest::RunTest(
	const FString& Parameters)
{
	using namespace PassControlDribbleAdvanceCompositionTests;
	FPassControlDribbleAdvancePlanQueryInput Input = MakeInputWithoutHelper();
	Input.HelperCardId = HelperCardId;
	const FPassControlDribbleAdvancePlanQueryResult Result = BuildPlan(
		MakePlayerCardSnapshots(), Input);
	TestEqual(TEXT("Helper identity error is retained"), Result.ErrorCode,
		EPassControlDribbleAdvancePlanQueryErrorCode::UnexpectedHelperIdentity);
	return TestFailureIsNotConsumable(*this, Result);
}

PASS_CONTROL_DRIBBLE_ADVANCE_COMPOSITION_TEST(
	FPassControlDribbleAdvanceCompositionRejectsSnapshotFailuresTest,
	"SnapshotFailuresHaveNoConsumablePlan")

bool FPassControlDribbleAdvanceCompositionRejectsSnapshotFailuresTest::RunTest(
	const FString& Parameters)
{
	using namespace PassControlDribbleAdvanceCompositionTests;
	const TArray<FName> MissingCardIds = {
		CarrierCardId, RunnerCardId, MarkerCardId, HelperCardId
	};
	for (const FName MissingCardId : MissingCardIds)
	{
		FPlayerCardRuleSnapshotSet Snapshots = MakePlayerCardSnapshots();
		Snapshots.Cards.RemoveAll([MissingCardId](
			const FPlayerCardRuleSnapshot& Card)
		{
			return Card.CardId == MissingCardId;
		});
		const FPassControlDribbleAdvancePlanQueryResult Result = BuildPlan(
			Snapshots, MakeValidInput());
		TestFailureIsNotConsumable(*this, Result);
	}
	return true;
}

PASS_CONTROL_DRIBBLE_ADVANCE_COMPOSITION_TEST(
	FPassControlDribbleAdvanceCompositionForbiddenDependenciesTest,
	"HasNoForbiddenDependencies")

bool FPassControlDribbleAdvanceCompositionForbiddenDependenciesTest::RunTest(
	const FString& Parameters)
{
	const FString SourcePath = FPaths::Combine(
		FPaths::ProjectDir(),
		TEXT("Source/FMCodex/CoreRules/PassControlDribbleAdvanceCompositionTests.cpp"));
	FString Source;
	const bool bLoaded = FFileHelper::LoadFileToString(Source, *SourcePath);
	TestTrue(TEXT("Composition source loads"), bLoaded);
	if (!bLoaded)
	{
		return false;
	}

	const TArray<FString> ForbiddenTerms = {
		FString(TEXT("Input")) + TEXT("AssemblyQuery"),
		FString(TEXT("ResolverInput")) + TEXT("Assembler"),
		FString(TEXT("Resolution")) + TEXT("Executor"),
		FString(TEXT("Formula")) + TEXT("Resolver"),
		FString(TEXT("FormulaAttack")) + TEXT("Flow"),
		FString(TEXT("Match")) + TEXT("Play"),
		FString(TEXT("External")) + TEXT("Api"),
		FString(TEXT("Skill")) + TEXT("Pipeline"),
		FString(TEXT("Skill")) + TEXT("Effect"),
		FString(TEXT("Data")) + TEXT("Table"),
		FString(TEXT("Pro")) + TEXT("vider"),
		FString(TEXT("Outcome")) + TEXT("Owner"),
		FString(TEXT("Generic")) + TEXT("Consumer"),
		FString(TEXT("Generic")) + TEXT("Composition")
	};
	for (const FString& ForbiddenTerm : ForbiddenTerms)
	{
		TestFalse(TEXT("Forbidden dependency is absent"),
			Source.Contains(ForbiddenTerm));
	}
	const FString GeneratedValueTerm = FString(TEXT("Ra")) + TEXT("nd");
	TestFalse(TEXT("No generated-value API"),
		Source.Contains(GeneratedValueTerm, ESearchCase::CaseSensitive));
	return true;
}

#undef PASS_CONTROL_DRIBBLE_ADVANCE_COMPOSITION_TEST

#endif
