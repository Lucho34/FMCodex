#if WITH_DEV_AUTOMATION_TESTS

#include "PassControlPassAdvancePlanQuery.h"

#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace PassControlPassAdvanceCompositionTests
{
	const FName SkillId(TEXT("Skill.PassControl.Composition"));
	const FName CarrierCardId(TEXT("PassAdvance.Composition.Carrier"));
	const FName RunnerCardId(TEXT("PassAdvance.Composition.Runner"));
	const FName MarkerCardId(TEXT("PassAdvance.Composition.Marker"));
	const FName HelperCardId(TEXT("PassAdvance.Composition.Helper"));
	const FName CarrierPlayerId(TEXT("Player.Composition.Carrier"));
	const FName RunnerPlayerId(TEXT("Player.Composition.Runner"));
	const FName MarkerPlayerId(TEXT("Player.Composition.Marker"));
	const FName HelperPlayerId(TEXT("Player.Composition.Helper"));
	const FGuid LogId(4101, 4102, 4103, 4104);

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
		const int32 CarrierPassing = 3,
		const int32 RunnerPassing = 6,
		const int32 MarkerTackling = 4,
		const int32 HelperMarking = 5,
		const bool bCarrierOwnsSkill = true,
		const bool bCarrierGoalkeeper = false,
		const bool bRunnerMidfield = true)
	{
		FPlayerCardRuleSnapshot Carrier = MakeCard(
			CarrierCardId,
			bCarrierGoalkeeper
				? EPlayerPositionType::Goalkeeper
				: EPlayerPositionType::Midfield,
			bCarrierOwnsSkill
				? TArray<FName>{ SkillId }
				: TArray<FName>{});
		Carrier.Attributes.Passing = CarrierPassing;
		Carrier.bIsGoalkeeper = bCarrierGoalkeeper;
		Carrier.bHasGoalkeeperAttributes = bCarrierGoalkeeper;

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

	FSkillRuleSnapshotSet MakeSkillRules(
		const ESkillRuleType SkillType = ESkillRuleType::PassControl)
	{
		FSkillRuleSnapshot Rule;
		Rule.SkillId = SkillId;
		Rule.SkillType = SkillType;
		Rule.MinTriggerActionPoint = 3;
		Rule.MaxTriggerActionPoint = 6;

		FSkillRuleSnapshotSet SnapshotSet;
		SnapshotSet.SkillRules = { Rule };
		return SnapshotSet;
	}

	FPassControlPassAdvancePlanQueryInput MakeValidInput(
		const int32 AttackD6 = 4,
		const int32 DefenseD6 = 2)
	{
		FPassControlPassAdvancePlanQueryInput Input;
		Input.SkillId = SkillId;
		Input.AdvanceType = EPassControlAdvanceType::PassAdvance;
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
		Input.TurnIndex = 41;
		Input.CarrierPlayerId = CarrierPlayerId;
		Input.RunnerPlayerId = RunnerPlayerId;
		Input.MarkerPlayerId = MarkerPlayerId;
		Input.HelperPlayerId = HelperPlayerId;
		return Input;
	}

	struct FTestSidePassAdvancePlanView
	{
		EFormulaType AttackerFormulaType = EFormulaType::None;
		EFormulaType DefenderFormulaType = EFormulaType::None;
		FName AttackerCardId = NAME_None;
		FName DefenderCardId = NAME_None;
		int32 AttackD6 = 0;
		int32 DefenseD6 = 0;
		float AttackerValue = 0.0f;
		float DefenderValue = 0.0f;
	};

	// This test-side projection reads the Plan only; it does not execute it.
	bool TryConsumeFormulaPlan(
		const FPassControlPassAdvancePlanQueryResult& Result,
		FTestSidePassAdvancePlanView& OutView)
	{
		if (!Result.bSuccess || !Result.bHasFormulaPlan)
		{
			return false;
		}

		const FPassControlPassAdvanceFormulaPlan& Plan = Result.FormulaPlan;
		OutView.AttackerFormulaType =
			Plan.AttackerQueryInput.FormulaType;
		OutView.DefenderFormulaType =
			Plan.DefenderQueryInput.FormulaType;
		OutView.AttackerCardId = Plan.AttackerQueryInput.CardId;
		OutView.DefenderCardId = Plan.DefenderQueryInput.CardId;
		OutView.AttackD6 = Plan.AttackerQueryInput.ExternalD6ComparePoint;
		OutView.DefenseD6 = Plan.DefenderQueryInput.ExternalD6ComparePoint;
		OutView.AttackerValue = static_cast<float>(
			Result.CarrierSnapshotQueryResult.Snapshot.Attributes.Passing)
			+ Plan.AttackerQueryInput.ExternalModifier;
		OutView.DefenderValue = static_cast<float>(
			Result.MarkerSnapshotQueryResult.Snapshot.Attributes.Tackling)
			+ Plan.DefenderQueryInput.ExternalModifier;
		return true;
	}

	FPassControlPassAdvancePlanQueryResult BuildValidPlan(
		const FPlayerCardRuleSnapshotSet& PlayerCardSnapshots,
		const FSkillRuleSnapshotSet& SkillRules,
		const FPassControlPassAdvancePlanQueryInput& Input)
	{
		return FPassControlPassAdvancePlanQuery::BuildPlan(
			PlayerCardSnapshots,
			SkillRules,
			Input);
	}

	bool TestFailureIsNotConsumable(
		FAutomationTestBase& Test,
		const FPassControlPassAdvancePlanQueryResult& Result)
	{
		FTestSidePassAdvancePlanView View;
		Test.TestFalse(TEXT("Failure result is unsuccessful"), Result.bSuccess);
		Test.TestFalse(TEXT("Failure result has no Formula Plan"), Result.bHasFormulaPlan);
		Test.TestFalse(
			TEXT("Failure result cannot be consumed as a Formula Plan"),
			TryConsumeFormulaPlan(Result, View));
		return true;
	}
}

#define PASS_CONTROL_PASS_ADVANCE_COMPOSITION_TEST(ClassName, TestName) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
		ClassName, \
		"FMCodex.CoreRules.PassControlPassAdvanceComposition." TestName, \
		EAutomationTestFlags::EditorContext \
			| EAutomationTestFlags::EngineFilter)

PASS_CONTROL_PASS_ADVANCE_COMPOSITION_TEST(
	FPassControlPassAdvanceCompositionConsumesPlanTest,
	"BuildPlanProducesTestSideConsumablePlan")

bool FPassControlPassAdvanceCompositionConsumesPlanTest::RunTest(
	const FString& Parameters)
{
	using namespace PassControlPassAdvanceCompositionTests;

	const FPassControlPassAdvancePlanQueryResult Result = BuildValidPlan(
		MakePlayerCardSnapshots(),
		MakeSkillRules(),
		MakeValidInput());
	FTestSidePassAdvancePlanView View;

	TestTrue(TEXT("Plan Query succeeds"), Result.bSuccess);
	TestEqual(
		TEXT("Plan decision requires formula resolution"),
		Result.Decision,
		EPassControlPassAdvancePlanDecision::FormulaResolutionRequired);
	TestTrue(TEXT("Formula Plan is generated"), Result.bHasFormulaPlan);
	TestTrue(
		TEXT("Formula Plan is consumed by the test-side view"),
		TryConsumeFormulaPlan(Result, View));
	return true;
}

PASS_CONTROL_PASS_ADVANCE_COMPOSITION_TEST(
	FPassControlPassAdvanceCompositionConsumesPlanWithoutHelperTest,
	"BuildPlanWithoutHelperProducesTestSideConsumablePlan")

bool FPassControlPassAdvanceCompositionConsumesPlanWithoutHelperTest::RunTest(
	const FString& Parameters)
{
	using namespace PassControlPassAdvanceCompositionTests;

	FPassControlPassAdvancePlanQueryInput Input = MakeValidInput();
	Input.bHasHelper = false;
	Input.HelperCardId = NAME_None;
	Input.HelperPlayerId = NAME_None;
	FPlayerCardRuleSnapshotSet PlayerCardSnapshots = MakePlayerCardSnapshots();
	PlayerCardSnapshots.Cards.RemoveAll([](const FPlayerCardRuleSnapshot& Card)
	{
		return Card.CardId == HelperCardId;
	});
	const FPassControlPassAdvancePlanQueryResult Result = BuildValidPlan(
		PlayerCardSnapshots,
		MakeSkillRules(),
		Input);
	FTestSidePassAdvancePlanView View;

	TestTrue(TEXT("No Helper Plan Query succeeds"), Result.bSuccess);
	TestFalse(TEXT("No Helper is recorded as unselected"), Result.bHasHelper);
	TestTrue(TEXT("No Helper Formula Plan is generated"), Result.bHasFormulaPlan);
	TestFalse(TEXT("No Helper snapshot query is performed"),
		Result.HelperSnapshotQueryResult.bSuccess);
	TestTrue(TEXT("No Helper Formula Plan is consumable"),
		TryConsumeFormulaPlan(Result, View));
	TestEqual(TEXT("No Helper defender modifier uses zero Marking"),
		Result.FormulaPlan.DefenderQueryInput.ExternalModifier,
		0.0f);
	return true;
}

PASS_CONTROL_PASS_ADVANCE_COMPOSITION_TEST(
	FPassControlPassAdvanceCompositionFinishingFieldsTest,
	"PlanUsesFinishingCarrierPassingMarkerTacklingAndExternalD6")

bool FPassControlPassAdvanceCompositionFinishingFieldsTest::RunTest(
	const FString& Parameters)
{
	using namespace PassControlPassAdvanceCompositionTests;

	const FPassControlPassAdvancePlanQueryResult Result = BuildValidPlan(
		MakePlayerCardSnapshots(),
		MakeSkillRules(),
		MakeValidInput(6, 1));
	FTestSidePassAdvancePlanView View;
	TestTrue(TEXT("Plan is consumable"), TryConsumeFormulaPlan(Result, View));

	TestEqual(TEXT("Attacker FormulaType is Finishing"),
		View.AttackerFormulaType, EFormulaType::Finishing);
	TestEqual(TEXT("Defender FormulaType is Finishing"),
		View.DefenderFormulaType, EFormulaType::Finishing);
	TestEqual(TEXT("Attacker uses Carrier card"), View.AttackerCardId, CarrierCardId);
	TestEqual(TEXT("Defender uses Marker card"), View.DefenderCardId, MarkerCardId);
	TestEqual(TEXT("Attacker uses Passing"),
		Result.FormulaPlan.AttackerQueryInput.Attribute,
		ESingleCardFormulaAttribute::Passing);
	TestEqual(TEXT("Defender uses Tackling"),
		Result.FormulaPlan.DefenderQueryInput.Attribute,
		ESingleCardFormulaAttribute::Tackling);
	TestEqual(TEXT("Attack D6 is externally preserved"), View.AttackD6, 6);
	TestEqual(TEXT("Defense D6 is externally preserved"), View.DefenseD6, 1);
	return true;
}

PASS_CONTROL_PASS_ADVANCE_COMPOSITION_TEST(
	FPassControlPassAdvanceCompositionHalfValueMappingTest,
	"PlanPreservesHalfValueAttributeMappings")

bool FPassControlPassAdvanceCompositionHalfValueMappingTest::RunTest(
	const FString& Parameters)
{
	using namespace PassControlPassAdvanceCompositionTests;

	const FPassControlPassAdvancePlanQueryResult Result = BuildValidPlan(
		MakePlayerCardSnapshots(3, 6, 4, 5),
		MakeSkillRules(),
		MakeValidInput());
	FTestSidePassAdvancePlanView View;
	TestTrue(TEXT("Plan is consumable"), TryConsumeFormulaPlan(Result, View));

	TestEqual(TEXT("Attacker modifier preserves .5"),
		Result.FormulaPlan.AttackerQueryInput.ExternalModifier, 1.5f);
	TestEqual(TEXT("Defender modifier preserves .5 plus two"),
		Result.FormulaPlan.DefenderQueryInput.ExternalModifier, 2.5f);
	TestEqual(TEXT("Carrier and Runner Passing average is 4.5"),
		View.AttackerValue, 4.5f);
	TestEqual(TEXT("Marker Tackling and Helper Marking average plus two is 6.5"),
		View.DefenderValue, 6.5f);
	return true;
}

PASS_CONTROL_PASS_ADVANCE_COMPOSITION_TEST(
	FPassControlPassAdvanceCompositionParticipantTrackingTest,
	"ResultTracksAllFourParticipants")

bool FPassControlPassAdvanceCompositionParticipantTrackingTest::RunTest(
	const FString& Parameters)
{
	using namespace PassControlPassAdvanceCompositionTests;

	const FPassControlPassAdvancePlanQueryResult Result = BuildValidPlan(
		MakePlayerCardSnapshots(),
		MakeSkillRules(),
		MakeValidInput());

	TestTrue(TEXT("Plan is generated"), Result.bHasFormulaPlan);
	TestEqual(TEXT("Carrier PlayerId is retained"),
		Result.FormulaPlan.CarrierPlayerId, CarrierPlayerId);
	TestEqual(TEXT("Runner PlayerId is retained"),
		Result.FormulaPlan.RunnerPlayerId, RunnerPlayerId);
	TestEqual(TEXT("Marker PlayerId is retained"),
		Result.FormulaPlan.MarkerPlayerId, MarkerPlayerId);
	TestEqual(TEXT("Helper PlayerId is retained"),
		Result.FormulaPlan.HelperPlayerId, HelperPlayerId);
	TestEqual(TEXT("Runner CardId is retained"),
		Result.FormulaPlan.RunnerCardId, RunnerCardId);
	TestEqual(TEXT("Helper CardId is retained"),
		Result.FormulaPlan.HelperCardId, HelperCardId);
	TestTrue(TEXT("Carrier snapshot trace succeeds"),
		Result.CarrierSnapshotQueryResult.bSuccess);
	TestTrue(TEXT("Runner snapshot trace succeeds"),
		Result.RunnerSnapshotQueryResult.bSuccess);
	TestTrue(TEXT("Marker snapshot trace succeeds"),
		Result.MarkerSnapshotQueryResult.bSuccess);
	TestTrue(TEXT("Helper snapshot trace succeeds"),
		Result.HelperSnapshotQueryResult.bSuccess);
	return true;
}

PASS_CONTROL_PASS_ADVANCE_COMPOSITION_TEST(
	FPassControlPassAdvanceCompositionExternalD6Test,
	"PlanPreservesDistinctExternalD6Values")

bool FPassControlPassAdvanceCompositionExternalD6Test::RunTest(
	const FString& Parameters)
{
	using namespace PassControlPassAdvanceCompositionTests;

	const FPassControlPassAdvancePlanQueryResult Result = BuildValidPlan(
		MakePlayerCardSnapshots(),
		MakeSkillRules(),
		MakeValidInput(1, 6));
	FTestSidePassAdvancePlanView View;
	TestTrue(TEXT("Plan is consumable"), TryConsumeFormulaPlan(Result, View));
	TestEqual(TEXT("Attack D6 remains the explicit input"), View.AttackD6, 1);
	TestEqual(TEXT("Defense D6 remains the explicit input"), View.DefenseD6, 6);
	return true;
}

PASS_CONTROL_PASS_ADVANCE_COMPOSITION_TEST(
	FPassControlPassAdvanceCompositionRejectsNonMidfieldRunnerTest,
	"NonMidfieldRunnerProducesNoConsumablePlan")

bool FPassControlPassAdvanceCompositionRejectsNonMidfieldRunnerTest::RunTest(
	const FString& Parameters)
{
	using namespace PassControlPassAdvanceCompositionTests;

	const FPassControlPassAdvancePlanQueryResult Result = BuildValidPlan(
		MakePlayerCardSnapshots(3, 6, 4, 5, true, false, false),
		MakeSkillRules(),
		MakeValidInput());
	TestEqual(TEXT("Runner position error is retained"), Result.ErrorCode,
		EPassControlPassAdvancePlanQueryErrorCode::RunnerPositionMismatch);
	return TestFailureIsNotConsumable(*this, Result);
}

PASS_CONTROL_PASS_ADVANCE_COMPOSITION_TEST(
	FPassControlPassAdvanceCompositionRejectsOtherSkillTypeTest,
	"NonPassControlSkillProducesNoConsumablePlan")

bool FPassControlPassAdvanceCompositionRejectsOtherSkillTypeTest::RunTest(
	const FString& Parameters)
{
	using namespace PassControlPassAdvanceCompositionTests;

	const FPassControlPassAdvancePlanQueryResult Result = BuildValidPlan(
		MakePlayerCardSnapshots(),
		MakeSkillRules(ESkillRuleType::LongShot),
		MakeValidInput());
	TestEqual(TEXT("Skill type error is retained"), Result.ErrorCode,
		EPassControlPassAdvancePlanQueryErrorCode::SkillTypeMismatch);
	return TestFailureIsNotConsumable(*this, Result);
}

PASS_CONTROL_PASS_ADVANCE_COMPOSITION_TEST(
	FPassControlPassAdvanceCompositionRejectsUnownedSkillTest,
	"UnownedCarrierSkillProducesNoConsumablePlan")

bool FPassControlPassAdvanceCompositionRejectsUnownedSkillTest::RunTest(
	const FString& Parameters)
{
	using namespace PassControlPassAdvanceCompositionTests;

	const FPassControlPassAdvancePlanQueryResult Result = BuildValidPlan(
		MakePlayerCardSnapshots(3, 6, 4, 5, false),
		MakeSkillRules(),
		MakeValidInput());
	TestEqual(TEXT("Ownership error is retained"), Result.ErrorCode,
		EPassControlPassAdvancePlanQueryErrorCode::SkillNotOwnedByCarrier);
	return TestFailureIsNotConsumable(*this, Result);
}

PASS_CONTROL_PASS_ADVANCE_COMPOSITION_TEST(
	FPassControlPassAdvanceCompositionRejectsGoalkeeperCarrierTest,
	"GoalkeeperCarrierProducesNoConsumablePlan")

bool FPassControlPassAdvanceCompositionRejectsGoalkeeperCarrierTest::RunTest(
	const FString& Parameters)
{
	using namespace PassControlPassAdvanceCompositionTests;

	const FPassControlPassAdvancePlanQueryResult Result = BuildValidPlan(
		MakePlayerCardSnapshots(3, 6, 4, 5, true, true),
		MakeSkillRules(),
		MakeValidInput());
	TestEqual(TEXT("Goalkeeper error is retained"), Result.ErrorCode,
		EPassControlPassAdvancePlanQueryErrorCode::UnsupportedGoalkeeperParticipant);
	return TestFailureIsNotConsumable(*this, Result);
}

PASS_CONTROL_PASS_ADVANCE_COMPOSITION_TEST(
	FPassControlPassAdvanceCompositionRejectsMissingParticipantTest,
	"MissingAnyParticipantProducesNoConsumablePlan")

bool FPassControlPassAdvanceCompositionRejectsMissingParticipantTest::RunTest(
	const FString& Parameters)
{
	using namespace PassControlPassAdvanceCompositionTests;

	const TArray<FName> MissingCardIds = {
		CarrierCardId, RunnerCardId, MarkerCardId, HelperCardId
	};
	for (const FName MissingCardId : MissingCardIds)
	{
		FPlayerCardRuleSnapshotSet PlayerCardSnapshots =
			MakePlayerCardSnapshots();
		PlayerCardSnapshots.Cards.RemoveAll([MissingCardId](
			const FPlayerCardRuleSnapshot& Card)
		{
			return Card.CardId == MissingCardId;
		});

		const FPassControlPassAdvancePlanQueryResult Result = BuildValidPlan(
			PlayerCardSnapshots,
			MakeSkillRules(),
			MakeValidInput());
		TestFailureIsNotConsumable(*this, Result);
	}
	return true;
}

PASS_CONTROL_PASS_ADVANCE_COMPOSITION_TEST(
	FPassControlPassAdvanceCompositionForbiddenDependenciesTest,
	"HasNoForbiddenDependencies")

bool FPassControlPassAdvanceCompositionForbiddenDependenciesTest::RunTest(
	const FString& Parameters)
{
	const FString SourcePath = FPaths::Combine(
		FPaths::ProjectDir(),
		TEXT("Source/FMCodex/CoreRules/PassControlPassAdvanceCompositionTests.cpp"));
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
		FString(TEXT("Pro")) + TEXT("vider")
	};
	for (const FString& ForbiddenTerm : ForbiddenTerms)
	{
		TestFalse(TEXT("Forbidden dependency is absent"),
			Source.Contains(ForbiddenTerm));
	}
	return true;
}

#undef PASS_CONTROL_PASS_ADVANCE_COMPOSITION_TEST

#endif
