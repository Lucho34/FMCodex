#if WITH_DEV_AUTOMATION_TESTS

#include "PassControlRunAdvancePlanQuery.h"

#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace PassControlRunAdvanceCompositionTests
{
	const FName SkillId(TEXT("Skill.PassControl.RunAdvance"));
	const FName CarrierCardId(TEXT("RunAdvanceComposition.Carrier"));
	const FName RunnerCardId(TEXT("RunAdvanceComposition.Runner"));
	const FName MarkerCardId(TEXT("RunAdvanceComposition.Marker"));
	const FName HelperCardId(TEXT("RunAdvanceComposition.Helper"));
	const FName CarrierPlayerId(TEXT("Player.RunAdvanceComposition.Carrier"));
	const FName RunnerPlayerId(TEXT("Player.RunAdvanceComposition.Runner"));
	const FName MarkerPlayerId(TEXT("Player.RunAdvanceComposition.Marker"));
	const FName HelperPlayerId(TEXT("Player.RunAdvanceComposition.Helper"));
	const FGuid LogId(6561, 6562, 6563, 6564);

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

	FPlayerCardRuleSnapshotSet MakeSnapshots(
		const int32 CarrierOffBall = 3,
		const int32 RunnerDribbling = 6,
		const int32 MarkerMarking = 4,
		const int32 HelperMarking = 5,
		const bool bRunnerMidfield = true)
	{
		FPlayerCardRuleSnapshot Carrier = MakeCard(
			CarrierCardId, EPlayerPositionType::Midfield, { SkillId });
		Carrier.Attributes.OffBall = CarrierOffBall;

		FPlayerCardRuleSnapshot Runner = MakeCard(
			RunnerCardId,
			bRunnerMidfield ? EPlayerPositionType::Midfield : EPlayerPositionType::Attack);
		Runner.Attributes.Dribbling = RunnerDribbling;

		FPlayerCardRuleSnapshot Marker = MakeCard(
			MarkerCardId, EPlayerPositionType::Defense);
		Marker.Attributes.Marking = MarkerMarking;

		FPlayerCardRuleSnapshot Helper = MakeCard(
			HelperCardId, EPlayerPositionType::Defense);
		Helper.Attributes.Marking = HelperMarking;

		FPlayerCardRuleSnapshotSet Set;
		Set.Cards = { Carrier, Runner, Marker, Helper };
		return Set;
	}

	FSkillRuleSnapshotSet MakeSkillRules()
	{
		FSkillRuleSnapshot Rule;
		Rule.SkillId = SkillId;
		Rule.SkillType = ESkillRuleType::PassControl;
		Rule.MinTriggerActionPoint = 3;
		Rule.MaxTriggerActionPoint = 6;
		FSkillRuleSnapshotSet Set;
		Set.SkillRules = { Rule };
		return Set;
	}

	FPassControlRunAdvancePlanQueryInput MakeValidInput(
		const int32 AttackD6 = 4,
		const int32 DefenseD6 = 2)
	{
		FPassControlRunAdvancePlanQueryInput Input;
		Input.SkillId = SkillId;
		Input.AdvanceType = EPassControlAdvanceType::RunAdvance;
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
		Input.TurnIndex = 23;
		Input.CarrierPlayerId = CarrierPlayerId;
		Input.RunnerPlayerId = RunnerPlayerId;
		Input.MarkerPlayerId = MarkerPlayerId;
		Input.HelperPlayerId = HelperPlayerId;
		return Input;
	}

	FPassControlRunAdvancePlanQueryInput MakeInputWithoutHelper()
	{
		FPassControlRunAdvancePlanQueryInput Input = MakeValidInput();
		Input.bHasHelper = false;
		Input.HelperCardId = NAME_None;
		Input.HelperPlayerId = NAME_None;
		return Input;
	}

	FPassControlRunAdvancePlanQueryResult BuildPlan(
		const FPlayerCardRuleSnapshotSet& Snapshots,
		const FPassControlRunAdvancePlanQueryInput& Input)
	{
		return FPassControlRunAdvancePlanQuery::BuildPlan(
			Snapshots, MakeSkillRules(), Input);
	}

	struct FTestSideRunAdvancePlanView
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

	// Test-side projection only reads an already-built dedicated plan. It does not
	// resolve a comparison or create a match result.
	bool TryConsumeRunAdvanceFormulaPlan(
		const FPassControlRunAdvancePlanQueryResult& Result,
		FTestSideRunAdvancePlanView& OutView)
	{
		if (!Result.bSuccess || !Result.bHasFormulaPlan)
		{
			return false;
		}

		const FPassControlRunAdvanceFormulaPlan& Plan = Result.FormulaPlan;
		OutView.AttackerFormulaType = Plan.AttackerQueryInput.FormulaType;
		OutView.DefenderFormulaType = Plan.DefenderQueryInput.FormulaType;
		OutView.AttackerCardId = Plan.AttackerQueryInput.CardId;
		OutView.DefenderCardId = Plan.DefenderQueryInput.CardId;
		OutView.AttackerAttribute = Plan.AttackerQueryInput.Attribute;
		OutView.DefenderAttribute = Plan.DefenderQueryInput.Attribute;
		OutView.AttackD6 = Plan.AttackerQueryInput.ExternalD6ComparePoint;
		OutView.DefenseD6 = Plan.DefenderQueryInput.ExternalD6ComparePoint;
		OutView.AttackerProjectedValue = static_cast<float>(
			Result.CarrierSnapshotQueryResult.Snapshot.Attributes.OffBall)
			+ Plan.AttackerQueryInput.ExternalModifier;
		OutView.DefenderProjectedValue = static_cast<float>(
			Result.MarkerSnapshotQueryResult.Snapshot.Attributes.Marking)
			+ Plan.DefenderQueryInput.ExternalModifier;
		return true;
	}

	bool ExpectNotConsumable(
		FAutomationTestBase& Test,
		const FPassControlRunAdvancePlanQueryResult& Result)
	{
		FTestSideRunAdvancePlanView View;
		Test.TestFalse(TEXT("Failure result is unsuccessful"), Result.bSuccess);
		Test.TestFalse(TEXT("Failure result has no Formula Plan"), Result.bHasFormulaPlan);
		Test.TestFalse(TEXT("Failure result is not test-side consumable"),
			TryConsumeRunAdvanceFormulaPlan(Result, View));
		return true;
	}

	bool LoadTestSource(FString& OutSource)
	{
		const FString FilePath = FPaths::Combine(
			FPaths::ProjectDir(),
			TEXT("Source/FMCodex/CoreRules/PassControlRunAdvanceCompositionTests.cpp"));
		return FFileHelper::LoadFileToString(OutSource, *FilePath);
	}
}

#define PASS_CONTROL_RUN_ADVANCE_COMPOSITION_TEST(ClassName, TestName) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
		ClassName, \
		"FMCodex.CoreRules.PassControlRunAdvanceComposition." TestName, \
		EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

PASS_CONTROL_RUN_ADVANCE_COMPOSITION_TEST(
	FPassControlRunAdvanceCompositionWithHelperTest,
	"ConsumesFinishingPlanWithHelper")
bool FPassControlRunAdvanceCompositionWithHelperTest::RunTest(const FString& Parameters)
{
	using namespace PassControlRunAdvanceCompositionTests;
	const FPassControlRunAdvancePlanQueryResult Result = BuildPlan(
		MakeSnapshots(), MakeValidInput());
	FTestSideRunAdvancePlanView View;
	TestTrue(TEXT("Plan query succeeds"), Result.bSuccess);
	TestTrue(TEXT("Formula Plan exists"), Result.bHasFormulaPlan);
	TestTrue(TEXT("Selected Helper is recorded"), Result.bHasHelper);
	TestTrue(TEXT("Test-side projection consumes the Plan"),
		TryConsumeRunAdvanceFormulaPlan(Result, View));
	TestEqual(TEXT("Attacker FormulaType is Finishing"),
		View.AttackerFormulaType, EFormulaType::Finishing);
	TestEqual(TEXT("Defender FormulaType is Finishing"),
		View.DefenderFormulaType, EFormulaType::Finishing);
	return true;
}

PASS_CONTROL_RUN_ADVANCE_COMPOSITION_TEST(
	FPassControlRunAdvanceCompositionWithoutHelperTest,
	"ConsumesFinishingPlanWithoutHelper")
bool FPassControlRunAdvanceCompositionWithoutHelperTest::RunTest(const FString& Parameters)
{
	using namespace PassControlRunAdvanceCompositionTests;
	const FPassControlRunAdvancePlanQueryResult Result = BuildPlan(
		MakeSnapshots(3, 6, 6, 5), MakeInputWithoutHelper());
	FTestSideRunAdvancePlanView View;
	TestTrue(TEXT("No Helper Plan query succeeds"), Result.bSuccess);
	TestFalse(TEXT("No Helper is recorded as unselected"), Result.bHasHelper);
	TestTrue(TEXT("Helper identity remains empty"),
		Result.FormulaPlan.HelperCardId.IsNone()
			&& Result.FormulaPlan.HelperPlayerId.IsNone());
	TestFalse(TEXT("No Helper Snapshot query is performed"),
		Result.HelperSnapshotQueryResult.bSuccess);
	TestTrue(TEXT("No Helper Snapshot diagnostic remains empty"),
		Result.HelperSnapshotQueryResult.CardId.IsNone());
	TestEqual(TEXT("No Helper uses zero Marking with fixed bonus"),
		Result.FormulaPlan.DefenderQueryInput.ExternalModifier, -1.0f);
	TestTrue(TEXT("No Helper Plan is test-side consumable"),
		TryConsumeRunAdvanceFormulaPlan(Result, View));
	return true;
}

PASS_CONTROL_RUN_ADVANCE_COMPOSITION_TEST(
	FPassControlRunAdvanceCompositionParticipantAndD6Test,
	"PreservesParticipantsAttributesAndExternalD6")
bool FPassControlRunAdvanceCompositionParticipantAndD6Test::RunTest(const FString& Parameters)
{
	using namespace PassControlRunAdvanceCompositionTests;
	const FPassControlRunAdvancePlanQueryResult Result = BuildPlan(
		MakeSnapshots(), MakeValidInput(6, 1));
	FTestSideRunAdvancePlanView View;
	TestTrue(TEXT("Plan is test-side consumable"),
		TryConsumeRunAdvanceFormulaPlan(Result, View));
	TestEqual(TEXT("Carrier is attacker"), View.AttackerCardId, CarrierCardId);
	TestEqual(TEXT("Marker is defender"), View.DefenderCardId, MarkerCardId);
	TestEqual(TEXT("Attacker uses OffBall"), View.AttackerAttribute,
		ESingleCardFormulaAttribute::OffBall);
	TestEqual(TEXT("Defender uses Marking"), View.DefenderAttribute,
		ESingleCardFormulaAttribute::Marking);
	TestTrue(TEXT("Attacker is not Passing or Dribbling"),
		View.AttackerAttribute != ESingleCardFormulaAttribute::Passing
			&& View.AttackerAttribute != ESingleCardFormulaAttribute::Dribbling);
	TestTrue(TEXT("Defender is not Tackling"),
		View.DefenderAttribute != ESingleCardFormulaAttribute::Tackling);
	TestEqual(TEXT("Attack D6 remains explicit"), View.AttackD6, 6);
	TestEqual(TEXT("Defense D6 remains explicit"), View.DefenseD6, 1);
	return true;
}

PASS_CONTROL_RUN_ADVANCE_COMPOSITION_TEST(
	FPassControlRunAdvanceCompositionHalfAndBonusTest,
	"PreservesHalfValuesAndDefenseBonus")
bool FPassControlRunAdvanceCompositionHalfAndBonusTest::RunTest(const FString& Parameters)
{
	using namespace PassControlRunAdvanceCompositionTests;
	const FPassControlRunAdvancePlanQueryResult Result = BuildPlan(
		MakeSnapshots(3, 6, 4, 5), MakeValidInput());
	FTestSideRunAdvancePlanView View;
	TestTrue(TEXT("Plan is test-side consumable"),
		TryConsumeRunAdvanceFormulaPlan(Result, View));
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

PASS_CONTROL_RUN_ADVANCE_COMPOSITION_TEST(
	FPassControlRunAdvanceCompositionRunnerAndHelperTrackingTest,
	"PreservesRunnerAndHelperTracking")
bool FPassControlRunAdvanceCompositionRunnerAndHelperTrackingTest::RunTest(const FString& Parameters)
{
	using namespace PassControlRunAdvanceCompositionTests;
	const FPassControlRunAdvancePlanQueryInput WithHelperInput = MakeValidInput();
	const FPassControlRunAdvancePlanQueryResult WithHelper = BuildPlan(
		MakeSnapshots(), WithHelperInput);
	const FPassControlRunAdvancePlanQueryInput WithoutHelperInput = MakeInputWithoutHelper();
	const FPassControlRunAdvancePlanQueryResult WithoutHelper = BuildPlan(
		MakeSnapshots(), WithoutHelperInput);
	TestEqual(TEXT("Runner CardId is preserved"),
		WithHelper.FormulaPlan.RunnerCardId, RunnerCardId);
	TestEqual(TEXT("Runner PlayerId is preserved"),
		WithHelper.FormulaPlan.RunnerPlayerId, RunnerPlayerId);
	TestTrue(TEXT("Selected Helper state agrees with input"),
		WithHelper.bHasHelper == WithHelperInput.bHasHelper);
	TestEqual(TEXT("Selected Helper CardId is preserved"),
		WithHelper.FormulaPlan.HelperCardId, HelperCardId);
	TestTrue(TEXT("No Helper state agrees with input"),
		WithoutHelper.bHasHelper == WithoutHelperInput.bHasHelper);
	TestTrue(TEXT("No Helper CardId remains empty"),
		WithoutHelper.FormulaPlan.HelperCardId.IsNone());
	return true;
}

PASS_CONTROL_RUN_ADVANCE_COMPOSITION_TEST(
	FPassControlRunAdvanceCompositionInvalidAdvanceTypeTest,
	"DoesNotConsumeInvalidAdvanceType")
bool FPassControlRunAdvanceCompositionInvalidAdvanceTypeTest::RunTest(const FString& Parameters)
{
	using namespace PassControlRunAdvanceCompositionTests;
	FPassControlRunAdvancePlanQueryInput Input = MakeValidInput();
	Input.AdvanceType = EPassControlAdvanceType::PassAdvance;
	const FPassControlRunAdvancePlanQueryResult Result = BuildPlan(MakeSnapshots(), Input);
	TestEqual(TEXT("Invalid AdvanceType is retained"), Result.ErrorCode,
		EPassControlRunAdvancePlanQueryErrorCode::InvalidAdvanceType);
	return ExpectNotConsumable(*this, Result);
}

PASS_CONTROL_RUN_ADVANCE_COMPOSITION_TEST(
	FPassControlRunAdvanceCompositionRunnerNotMidfieldTest,
	"DoesNotConsumeRunnerNotMidfield")
bool FPassControlRunAdvanceCompositionRunnerNotMidfieldTest::RunTest(const FString& Parameters)
{
	using namespace PassControlRunAdvanceCompositionTests;
	const FPassControlRunAdvancePlanQueryResult Result = BuildPlan(
		MakeSnapshots(3, 6, 4, 5, false), MakeValidInput());
	TestEqual(TEXT("Runner position failure is retained"), Result.ErrorCode,
		EPassControlRunAdvancePlanQueryErrorCode::RunnerNotMidfield);
	return ExpectNotConsumable(*this, Result);
}

PASS_CONTROL_RUN_ADVANCE_COMPOSITION_TEST(
	FPassControlRunAdvanceCompositionUnexpectedHelperTest,
	"DoesNotConsumeUnexpectedHelperIdentity")
bool FPassControlRunAdvanceCompositionUnexpectedHelperTest::RunTest(const FString& Parameters)
{
	using namespace PassControlRunAdvanceCompositionTests;
	FPassControlRunAdvancePlanQueryInput Input = MakeInputWithoutHelper();
	Input.HelperCardId = HelperCardId;
	const FPassControlRunAdvancePlanQueryResult Result = BuildPlan(MakeSnapshots(), Input);
	TestEqual(TEXT("Unexpected Helper identity is retained"), Result.ErrorCode,
		EPassControlRunAdvancePlanQueryErrorCode::UnexpectedHelperIdentity);
	return ExpectNotConsumable(*this, Result);
}

PASS_CONTROL_RUN_ADVANCE_COMPOSITION_TEST(
	FPassControlRunAdvanceCompositionSnapshotFailuresTest,
	"DoesNotConsumeSnapshotFailures")
bool FPassControlRunAdvanceCompositionSnapshotFailuresTest::RunTest(const FString& Parameters)
{
	using namespace PassControlRunAdvanceCompositionTests;
	const auto CheckFailure = [this](
		const FPassControlRunAdvancePlanQueryInput& Input,
		const EPassControlRunAdvancePlanQueryErrorCode ErrorCode)
	{
		const FPassControlRunAdvancePlanQueryResult Result = BuildPlan(MakeSnapshots(), Input);
		TestEqual(TEXT("Snapshot failure error is retained"), Result.ErrorCode, ErrorCode);
		ExpectNotConsumable(*this, Result);
	};

	FPassControlRunAdvancePlanQueryInput CarrierInput = MakeValidInput();
	CarrierInput.CarrierCardId = TEXT("Missing.Carrier");
	CheckFailure(CarrierInput,
		EPassControlRunAdvancePlanQueryErrorCode::CarrierSnapshotQueryFailed);
	FPassControlRunAdvancePlanQueryInput RunnerInput = MakeValidInput();
	RunnerInput.RunnerCardId = TEXT("Missing.Runner");
	CheckFailure(RunnerInput,
		EPassControlRunAdvancePlanQueryErrorCode::RunnerSnapshotQueryFailed);
	FPassControlRunAdvancePlanQueryInput MarkerInput = MakeValidInput();
	MarkerInput.MarkerCardId = TEXT("Missing.Marker");
	CheckFailure(MarkerInput,
		EPassControlRunAdvancePlanQueryErrorCode::MarkerSnapshotQueryFailed);
	FPassControlRunAdvancePlanQueryInput HelperInput = MakeValidInput();
	HelperInput.HelperCardId = TEXT("Missing.Helper");
	CheckFailure(HelperInput,
		EPassControlRunAdvancePlanQueryErrorCode::HelperSnapshotQueryFailed);
	return true;
}

PASS_CONTROL_RUN_ADVANCE_COMPOSITION_TEST(
	FPassControlRunAdvanceCompositionForbiddenDependenciesTest,
	"HasNoForbiddenDependencies")
bool FPassControlRunAdvanceCompositionForbiddenDependenciesTest::RunTest(const FString& Parameters)
{
	using namespace PassControlRunAdvanceCompositionTests;
	FString Source;
	TestTrue(TEXT("Composition source loads"), LoadTestSource(Source));
	const TArray<FString> Forbidden = {
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
		FString(TEXT("Ra")) + TEXT("nd"),
		FString(TEXT("Generic")) + TEXT("Consumer"),
		FString(TEXT("Generic")) + TEXT("Composition"),
		FString(TEXT("PassControl")) + TEXT("PlanQuery"),
		FString(TEXT("Outcome")) + TEXT("Owner"),
		FString(TEXT("Go")) + TEXT("al"),
		FString(TEXT("Sc")) + TEXT("ore"),
		FString(TEXT("Attack")) + TEXT("Ended"),
		FString(TEXT("De")) + TEXT("ck")
	};
	for (const FString& Token : Forbidden)
	{
		TestFalse(FString::Printf(TEXT("No forbidden token '%s'"), *Token),
			Source.Contains(Token, ESearchCase::CaseSensitive));
	}
	return true;
}

#undef PASS_CONTROL_RUN_ADVANCE_COMPOSITION_TEST

#endif
