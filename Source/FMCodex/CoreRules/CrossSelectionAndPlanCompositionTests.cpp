#if WITH_DEV_AUTOMATION_TESTS

#include "CrossPlanQuery.h"
#include "CrossSelectionQuery.h"

#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace CrossSelectionAndPlanCompositionTests
{
	const FName SkillId(TEXT("Skill.Cross.Composition"));
	const FName CarrierCardId(TEXT("CrossComposition.Carrier"));
	const FName RunnerCardId(TEXT("CrossComposition.Runner"));
	const FName MarkerCardId(TEXT("CrossComposition.Marker"));
	const FName HelperCardId(TEXT("CrossComposition.Helper"));
	const FName GoalkeeperCardId(TEXT("CrossComposition.Goalkeeper"));
	const FName CarrierPlayerId(TEXT("Player.CrossComposition.Carrier"));
	const FName RunnerPlayerId(TEXT("Player.CrossComposition.Runner"));
	const FName MarkerPlayerId(TEXT("Player.CrossComposition.Marker"));
	const FName HelperPlayerId(TEXT("Player.CrossComposition.Helper"));
	const FName GoalkeeperPlayerId(TEXT("Player.CrossComposition.Goalkeeper"));
	const FGuid LogId(841, 842, 843, 844);

	FPlayerAttributes MakeAttributes(
		const int32 Passing,
		const int32 Shooting,
		const int32 Strength,
		const int32 Marking,
		const int32 Tackling)
	{
		FPlayerAttributes Attributes;
		Attributes.Shooting = Shooting;
		Attributes.Dribbling = 1;
		Attributes.Passing = Passing;
		Attributes.OffBall = 1;
		Attributes.Marking = Marking;
		Attributes.Tackling = Tackling;
		Attributes.Speed = 1;
		Attributes.Strength = Strength;
		Attributes.Stamina = 1;
		Attributes.LongShot = 1;
		return Attributes;
	}

	FPlayerCardRuleSnapshot MakeOutfieldCard(
		const FName CardId,
		const EPlayerPositionType Position,
		const FPlayerAttributes& Attributes,
		const bool bOwnsSkill = false)
	{
		FPlayerCardRuleSnapshot Card;
		Card.CardId = CardId;
		Card.PositionTypes = { Position };
		Card.Attributes = Attributes;
		Card.SkillIds = bOwnsSkill ? TArray<FName>{ SkillId } : TArray<FName>{};
		return Card;
	}

	FPlayerCardRuleSnapshot MakeGoalkeeperCard()
	{
		FPlayerCardRuleSnapshot Card;
		Card.CardId = GoalkeeperCardId;
		Card.PositionTypes = { EPlayerPositionType::Goalkeeper };
		Card.Attributes = MakeAttributes(1, 1, 1, 1, 1);
		Card.bIsGoalkeeper = true;
		Card.bHasGoalkeeperAttributes = true;
		Card.GoalkeeperAttributes.Handling = 1;
		Card.GoalkeeperAttributes.Positioning = 1;
		Card.GoalkeeperAttributes.Reflex = 6;
		Card.GoalkeeperAttributes.Aerial = 5;
		Card.GoalkeeperAttributes.Anticipation = 1;
		Card.GoalkeeperAttributes.OneOnOne = 1;
		return Card;
	}

	FPlayerCardRuleSnapshotSet MakeSnapshots()
	{
		FPlayerCardRuleSnapshotSet Snapshots;
		Snapshots.Cards = {
			MakeOutfieldCard(
				CarrierCardId,
				EPlayerPositionType::Midfield,
				MakeAttributes(3, 1, 1, 1, 1),
				true),
			MakeOutfieldCard(
				RunnerCardId,
				EPlayerPositionType::Attack,
				MakeAttributes(1, 6, 4, 1, 1)),
			MakeOutfieldCard(
				MarkerCardId,
				EPlayerPositionType::Defense,
				MakeAttributes(1, 1, 1, 1, 5)),
			MakeOutfieldCard(
				HelperCardId,
				EPlayerPositionType::Midfield,
				MakeAttributes(1, 1, 6, 4, 1)),
			MakeGoalkeeperCard()
		};
		return Snapshots;
	}

	FSkillRuleSnapshotSet MakeSkillRules()
	{
		FSkillRuleSnapshot Rule;
		Rule.SkillId = SkillId;
		Rule.SkillType = ESkillRuleType::Cross;
		Rule.MinTriggerActionPoint = 3;
		Rule.MaxTriggerActionPoint = 6;
		FSkillRuleSnapshotSet SkillRules;
		SkillRules.SkillRules = { Rule };
		return SkillRules;
	}

	FCrossSelectionQueryInput MakeSelectionInput(
		const ECrossIntentType IntentType,
		const int32 SelectionD6)
	{
		FCrossSelectionQueryInput Input;
		Input.IntendedCrossType = IntentType;
		Input.bHasExternalSelectionD6 = true;
		Input.ExternalSelectionD6 = SelectionD6;
		return Input;
	}

	FCrossPlanQueryInput MakePlanInput(
		const bool bHasHelper = false,
		const bool bUseGoalkeeper = false)
	{
		FCrossPlanQueryInput Input;
		Input.SkillId = SkillId;
		Input.CarrierCardId = CarrierCardId;
		Input.CarrierPlayerId = CarrierPlayerId;
		Input.RunnerCardId = RunnerCardId;
		Input.RunnerPlayerId = RunnerPlayerId;
		Input.MarkerCardId = MarkerCardId;
		Input.MarkerPlayerId = MarkerPlayerId;
		Input.bHasHelper = bHasHelper;
		Input.HelperCardId = bHasHelper ? HelperCardId : NAME_None;
		Input.HelperPlayerId = bHasHelper ? HelperPlayerId : NAME_None;
		Input.bUseGoalkeeper = bUseGoalkeeper;
		Input.GoalkeeperCardId = bUseGoalkeeper ? GoalkeeperCardId : NAME_None;
		Input.GoalkeeperPlayerId = bUseGoalkeeper
			? GoalkeeperPlayerId
			: NAME_None;
		Input.CurrentActionPoint = 4;
		Input.AttackD6 = 2;
		Input.DefenseD6 = 5;
		Input.LogId = LogId;
		Input.TurnIndex = 24;
		return Input;
	}

	bool MapSelectionActualTypeToPlanActualType(
		const ECrossActualType SelectionActualType,
		ECrossPlanActualType& OutPlanActualType)
	{
		if (SelectionActualType == ECrossActualType::High)
		{
			OutPlanActualType = ECrossPlanActualType::High;
			return true;
		}
		if (SelectionActualType == ECrossActualType::Low)
		{
			OutPlanActualType = ECrossPlanActualType::Low;
			return true;
		}
		return false;
	}

	bool AreSelectionInputsEqual(
		const FCrossSelectionQueryInput& Left,
		const FCrossSelectionQueryInput& Right)
	{
		return Left.IntendedCrossType == Right.IntendedCrossType
			&& Left.bHasExternalSelectionD6
				== Right.bHasExternalSelectionD6
			&& Left.ExternalSelectionD6 == Right.ExternalSelectionD6;
	}

	bool ArePlanInputsEqual(
		const FCrossPlanQueryInput& Left,
		const FCrossPlanQueryInput& Right)
	{
		return Left.SkillId == Right.SkillId
			&& Left.ActualCrossType == Right.ActualCrossType
			&& Left.CarrierCardId == Right.CarrierCardId
			&& Left.CarrierPlayerId == Right.CarrierPlayerId
			&& Left.RunnerCardId == Right.RunnerCardId
			&& Left.RunnerPlayerId == Right.RunnerPlayerId
			&& Left.MarkerCardId == Right.MarkerCardId
			&& Left.MarkerPlayerId == Right.MarkerPlayerId
			&& Left.bHasHelper == Right.bHasHelper
			&& Left.HelperCardId == Right.HelperCardId
			&& Left.HelperPlayerId == Right.HelperPlayerId
			&& Left.bUseGoalkeeper == Right.bUseGoalkeeper
			&& Left.GoalkeeperCardId == Right.GoalkeeperCardId
			&& Left.GoalkeeperPlayerId == Right.GoalkeeperPlayerId
			&& Left.CurrentActionPoint == Right.CurrentActionPoint
			&& Left.AttackD6 == Right.AttackD6
			&& Left.DefenseD6 == Right.DefenseD6
			&& Left.LogId == Right.LogId
			&& Left.TurnIndex == Right.TurnIndex;
	}

	bool AreSnapshotSetsEqual(
		const FPlayerCardRuleSnapshotSet& Left,
		const FPlayerCardRuleSnapshotSet& Right)
	{
		if (Left.Cards.Num() != Right.Cards.Num())
		{
			return false;
		}
		for (int32 Index = 0; Index < Left.Cards.Num(); ++Index)
		{
			const FPlayerCardRuleSnapshot& LeftCard = Left.Cards[Index];
			const FPlayerCardRuleSnapshot& RightCard = Right.Cards[Index];
			if (LeftCard.CardId != RightCard.CardId
				|| LeftCard.PositionTypes != RightCard.PositionTypes
				|| LeftCard.bIsGoalkeeper != RightCard.bIsGoalkeeper
				|| LeftCard.bHasGoalkeeperAttributes
					!= RightCard.bHasGoalkeeperAttributes
				|| LeftCard.SkillIds != RightCard.SkillIds
				|| FMemory::Memcmp(
					&LeftCard.Attributes,
					&RightCard.Attributes,
					sizeof(FPlayerAttributes)) != 0
				|| FMemory::Memcmp(
					&LeftCard.GoalkeeperAttributes,
					&RightCard.GoalkeeperAttributes,
					sizeof(FGoalkeeperAttributes)) != 0)
			{
				return false;
			}
		}
		return true;
	}

	struct FTestSideCrossPlanView
	{
		ECrossPlanActualType ActualCrossType = ECrossPlanActualType::None;
		EFormulaType AttackerFormulaType = EFormulaType::None;
		EFormulaType DefenderFormulaType = EFormulaType::None;
		ESingleCardFormulaAttribute AttackerAttribute =
			ESingleCardFormulaAttribute::None;
		ESingleCardFormulaAttribute DefenderAttribute =
			ESingleCardFormulaAttribute::None;
		float DefenderModifier = 0.0f;
		FName HelperPlayerId = NAME_None;
		FName GoalkeeperPlayerId = NAME_None;
	};

	bool TryConsumeCrossFormulaPlan(
		const FCrossSelectionQueryResult& SelectionResult,
		const FCrossPlanQueryResult& PlanResult,
		FTestSideCrossPlanView& OutView)
	{
		if (!SelectionResult.bSuccess
			|| !PlanResult.bSuccess
			|| !PlanResult.bHasFormulaPlan)
		{
			return false;
		}

		OutView.ActualCrossType = PlanResult.FormulaPlan.ActualCrossType;
		OutView.AttackerFormulaType =
			PlanResult.FormulaPlan.AttackerQueryInput.FormulaType;
		OutView.DefenderFormulaType =
			PlanResult.FormulaPlan.DefenderQueryInput.FormulaType;
		OutView.AttackerAttribute =
			PlanResult.FormulaPlan.AttackerQueryInput.Attribute;
		OutView.DefenderAttribute =
			PlanResult.FormulaPlan.DefenderQueryInput.Attribute;
		OutView.DefenderModifier =
			PlanResult.FormulaPlan.DefenderQueryInput.ExternalModifier;
		OutView.HelperPlayerId = PlanResult.FormulaPlan.HelperPlayerId;
		OutView.GoalkeeperPlayerId =
			PlanResult.FormulaPlan.GoalkeeperPlayerId;
		return true;
	}

	bool BuildFromSelection(
		const FCrossSelectionQueryInput& SelectionInput,
		FCrossPlanQueryInput PlanInput,
		const FPlayerCardRuleSnapshotSet& Snapshots,
		const FSkillRuleSnapshotSet& SkillRules,
		FCrossSelectionQueryResult& OutSelectionResult,
		FCrossPlanQueryResult& OutPlanResult,
		bool& bOutPlanWasBuilt)
	{
		bOutPlanWasBuilt = false;
		OutSelectionResult = FCrossSelectionQuery::Select(SelectionInput);
		if (!OutSelectionResult.bSuccess
			|| !OutSelectionResult.bHasActualCrossType)
		{
			return false;
		}

		ECrossPlanActualType PlanActualType = ECrossPlanActualType::None;
		if (!MapSelectionActualTypeToPlanActualType(
				OutSelectionResult.ActualCrossType,
				PlanActualType))
		{
			return false;
		}

		PlanInput.ActualCrossType = PlanActualType;
		bOutPlanWasBuilt = true;
		OutPlanResult = FCrossPlanQuery::BuildPlan(
			Snapshots,
			SkillRules,
			PlanInput);
		return OutPlanResult.bSuccess && OutPlanResult.bHasFormulaPlan;
	}

	bool LoadTestSource(FString& OutSource)
	{
		const FString SourcePath = FPaths::Combine(
			FPaths::ProjectDir(),
			TEXT("Source/FMCodex/CoreRules/CrossSelectionAndPlanCompositionTests.cpp"));
		return FFileHelper::LoadFileToString(OutSource, *SourcePath);
	}
}

#define CROSS_SELECTION_PLAN_COMPOSITION_TEST(ClassName, TestName) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
		ClassName, \
		"FMCodex.CoreRules.CrossSelectionAndPlanComposition." TestName, \
		EAutomationTestFlags::EditorContext \
			| EAutomationTestFlags::EngineFilter)

#define CROSS_SELECTION_PLAN_BRIDGE_TEST( \
	ClassName, TestName, IntentType, D6, ExpectedSelectionType, ExpectedPlanType) \
	CROSS_SELECTION_PLAN_COMPOSITION_TEST(ClassName, TestName) \
	bool ClassName::RunTest(const FString& Parameters) \
	{ \
		using namespace CrossSelectionAndPlanCompositionTests; \
		FCrossSelectionQueryResult SelectionResult; \
		FCrossPlanQueryResult PlanResult; \
		bool bPlanWasBuilt = false; \
		const bool bBuildSucceeded = BuildFromSelection( \
			MakeSelectionInput(IntentType, D6), MakePlanInput(), MakeSnapshots(), MakeSkillRules(), \
			SelectionResult, PlanResult, bPlanWasBuilt); \
		TestTrue(TEXT("Selection succeeds"), SelectionResult.bSuccess); \
		TestEqual(TEXT("Selection produces the expected actual type"), SelectionResult.ActualCrossType, ExpectedSelectionType); \
		TestTrue(TEXT("Plan is built after Selection success"), bPlanWasBuilt); \
		TestTrue(TEXT("Bridge builds a Formula Plan"), bBuildSucceeded); \
		FTestSideCrossPlanView View; \
		TestTrue(TEXT("Bridge Formula Plan is test-side consumable"), TryConsumeCrossFormulaPlan(SelectionResult, PlanResult, View)); \
		TestEqual(TEXT("Plan receives the expected actual type"), View.ActualCrossType, ExpectedPlanType); \
		TestEqual(TEXT("Attacker FormulaType is Finishing"), View.AttackerFormulaType, EFormulaType::Finishing); \
		TestEqual(TEXT("Defender FormulaType is Finishing"), View.DefenderFormulaType, EFormulaType::Finishing); \
		return true; \
	}

CROSS_SELECTION_PLAN_BRIDGE_TEST(
	FCrossCompositionHighNormalBridgeTest,
	"HighIntentD6OneBridgesToHighPlan",
	ECrossIntentType::High,
	1,
	ECrossActualType::High,
	ECrossPlanActualType::High)

CROSS_SELECTION_PLAN_BRIDGE_TEST(
	FCrossCompositionHighReverseBridgeTest,
	"HighIntentD6FiveBridgesToLowPlan",
	ECrossIntentType::High,
	5,
	ECrossActualType::Low,
	ECrossPlanActualType::Low)

CROSS_SELECTION_PLAN_BRIDGE_TEST(
	FCrossCompositionLowNormalBridgeTest,
	"LowIntentD6OneBridgesToLowPlan",
	ECrossIntentType::Low,
	1,
	ECrossActualType::Low,
	ECrossPlanActualType::Low)

CROSS_SELECTION_PLAN_BRIDGE_TEST(
	FCrossCompositionLowReverseBridgeTest,
	"LowIntentD6FiveBridgesToHighPlan",
	ECrossIntentType::Low,
	5,
	ECrossActualType::High,
	ECrossPlanActualType::High)

CROSS_SELECTION_PLAN_COMPOSITION_TEST(
	FCrossCompositionHighHelperGoalkeeperTrackingTest,
	"HighPlanPreservesHelperGoalkeeperAndAerialModifier")

bool FCrossCompositionHighHelperGoalkeeperTrackingTest::RunTest(
	const FString& Parameters)
{
	using namespace CrossSelectionAndPlanCompositionTests;
	FCrossSelectionQueryResult SelectionResult;
	FCrossPlanQueryResult PlanResult;
	bool bPlanWasBuilt = false;
	const bool bBuildSucceeded = BuildFromSelection(
		MakeSelectionInput(ECrossIntentType::High, 1),
		MakePlanInput(true, true),
		MakeSnapshots(),
		MakeSkillRules(),
		SelectionResult,
		PlanResult,
		bPlanWasBuilt);
	FTestSideCrossPlanView View;
	TestTrue(TEXT("High bridge builds a Formula Plan"), bBuildSucceeded);
	TestTrue(TEXT("High Plan is consumable"), TryConsumeCrossFormulaPlan(
		SelectionResult, PlanResult, View));
	TestEqual(TEXT("Helper identity is retained"), View.HelperPlayerId, HelperPlayerId);
	TestEqual(
		TEXT("Goalkeeper identity is retained"),
		View.GoalkeeperPlayerId,
		GoalkeeperPlayerId);
	TestEqual(TEXT("Aerial half modifier is preserved"), View.DefenderModifier, 5.0f);
	return true;
}

CROSS_SELECTION_PLAN_COMPOSITION_TEST(
	FCrossCompositionLowHelperGoalkeeperTrackingTest,
	"LowPlanPreservesHelperGoalkeeperAndReflexModifier")

bool FCrossCompositionLowHelperGoalkeeperTrackingTest::RunTest(
	const FString& Parameters)
{
	using namespace CrossSelectionAndPlanCompositionTests;
	FCrossSelectionQueryResult SelectionResult;
	FCrossPlanQueryResult PlanResult;
	bool bPlanWasBuilt = false;
	const bool bBuildSucceeded = BuildFromSelection(
		MakeSelectionInput(ECrossIntentType::Low, 1),
		MakePlanInput(true, true),
		MakeSnapshots(),
		MakeSkillRules(),
		SelectionResult,
		PlanResult,
		bPlanWasBuilt);
	FTestSideCrossPlanView View;
	TestTrue(TEXT("Low bridge builds a Formula Plan"), bBuildSucceeded);
	TestTrue(TEXT("Low Plan is consumable"), TryConsumeCrossFormulaPlan(
		SelectionResult, PlanResult, View));
	TestEqual(TEXT("Helper identity is retained"), View.HelperPlayerId, HelperPlayerId);
	TestEqual(
		TEXT("Goalkeeper identity is retained"),
		View.GoalkeeperPlayerId,
		GoalkeeperPlayerId);
	TestEqual(TEXT("Reflex half modifier is preserved"), View.DefenderModifier, 4.5f);
	return true;
}

CROSS_SELECTION_PLAN_COMPOSITION_TEST(
	FCrossCompositionSelectionFailureShortCircuitTest,
	"SelectionFailureDoesNotBuildOrConsumePlan")

bool FCrossCompositionSelectionFailureShortCircuitTest::RunTest(
	const FString& Parameters)
{
	using namespace CrossSelectionAndPlanCompositionTests;
	FCrossSelectionQueryInput SelectionInput;
	SelectionInput.IntendedCrossType = ECrossIntentType::High;
	FCrossSelectionQueryResult SelectionResult;
	FCrossPlanQueryResult PlanResult;
	bool bPlanWasBuilt = false;
	const bool bBuildSucceeded = BuildFromSelection(
		SelectionInput,
		MakePlanInput(),
		MakeSnapshots(),
		MakeSkillRules(),
		SelectionResult,
		PlanResult,
		bPlanWasBuilt);
	FTestSideCrossPlanView View;
	TestFalse(TEXT("Selection fails"), SelectionResult.bSuccess);
	TestFalse(TEXT("Plan is not built"), bPlanWasBuilt);
	TestFalse(TEXT("Bridge does not succeed"), bBuildSucceeded);
	TestFalse(TEXT("No Plan is test-side consumable"), TryConsumeCrossFormulaPlan(
		SelectionResult, PlanResult, View));
	return true;
}

CROSS_SELECTION_PLAN_COMPOSITION_TEST(
	FCrossCompositionPlanFailureShortCircuitTest,
	"PlanFailureIsNotConsumable")

bool FCrossCompositionPlanFailureShortCircuitTest::RunTest(
	const FString& Parameters)
{
	using namespace CrossSelectionAndPlanCompositionTests;
	FCrossPlanQueryInput PlanInput = MakePlanInput();
	PlanInput.AttackD6 = 0;
	FCrossSelectionQueryResult SelectionResult;
	FCrossPlanQueryResult PlanResult;
	bool bPlanWasBuilt = false;
	const bool bBuildSucceeded = BuildFromSelection(
		MakeSelectionInput(ECrossIntentType::High, 1),
		PlanInput,
		MakeSnapshots(),
		MakeSkillRules(),
		SelectionResult,
		PlanResult,
		bPlanWasBuilt);
	FTestSideCrossPlanView View;
	TestTrue(TEXT("Selection succeeds before Plan failure"), SelectionResult.bSuccess);
	TestTrue(TEXT("Plan is built"), bPlanWasBuilt);
	TestFalse(TEXT("Bridge fails when Plan fails"), bBuildSucceeded);
	TestFalse(TEXT("Plan reports no Formula Plan"), PlanResult.bHasFormulaPlan);
	TestFalse(TEXT("Failed Plan is not consumable"), TryConsumeCrossFormulaPlan(
		SelectionResult, PlanResult, View));
	return true;
}

CROSS_SELECTION_PLAN_COMPOSITION_TEST(
	FCrossCompositionInputImmutabilityTest,
	"DoesNotMutateCompositionInputs")

bool FCrossCompositionInputImmutabilityTest::RunTest(
	const FString& Parameters)
{
	using namespace CrossSelectionAndPlanCompositionTests;
	const FCrossSelectionQueryInput SelectionInput =
		MakeSelectionInput(ECrossIntentType::Low, 5);
	const FCrossSelectionQueryInput SelectionInputBefore = SelectionInput;
	const FCrossPlanQueryInput PlanInput = MakePlanInput(true, true);
	const FCrossPlanQueryInput PlanInputBefore = PlanInput;
	const FPlayerCardRuleSnapshotSet Snapshots = MakeSnapshots();
	const FPlayerCardRuleSnapshotSet SnapshotsBefore = Snapshots;
	FCrossSelectionQueryResult SelectionResult;
	FCrossPlanQueryResult PlanResult;
	bool bPlanWasBuilt = false;
	BuildFromSelection(
		SelectionInput,
		PlanInput,
		Snapshots,
		MakeSkillRules(),
		SelectionResult,
		PlanResult,
		bPlanWasBuilt);
	TestTrue(
		TEXT("Selection Input remains unchanged"),
		AreSelectionInputsEqual(SelectionInput, SelectionInputBefore));
	TestTrue(
		TEXT("Plan Input remains unchanged"),
		ArePlanInputsEqual(PlanInput, PlanInputBefore));
	TestTrue(
		TEXT("Snapshot set remains unchanged"),
		AreSnapshotSetsEqual(Snapshots, SnapshotsBefore));
	return true;
}

CROSS_SELECTION_PLAN_COMPOSITION_TEST(
	FCrossCompositionForbiddenDependenciesTest,
	"HasNoForbiddenDependencies")

bool FCrossCompositionForbiddenDependenciesTest::RunTest(
	const FString& Parameters)
{
	using namespace CrossSelectionAndPlanCompositionTests;
	FString Source;
	TestTrue(TEXT("Composition test source loads"), LoadTestSource(Source));
	const TArray<FString> ForbiddenTerms = {
		FString(TEXT("Formula")) + TEXT("Resolver"),
		FString(TEXT("Formula")) + TEXT("AttackFlow"),
		FString(TEXT("Match")) + TEXT("Play"),
		FString(TEXT("External")) + TEXT("Api"),
		FString(TEXT("FMath::")) + TEXT("Rand"),
		FString(TEXT("FRandom")) + TEXT("Stream"),
		FString(TEXT("Resolution")) + TEXT("Executor"),
		FString(TEXT("Resolver")) + TEXT("InputAssembler"),
		FString(TEXT("Input")) + TEXT("AssemblyQuery"),
		FString(TEXT("Skill")) + TEXT("Pipeline"),
		FString(TEXT("Skill")) + TEXT("Effect"),
		FString(TEXT("Generic")) + TEXT("Composition")
	};
	for (const FString& Term : ForbiddenTerms)
	{
		TestFalse(TEXT("Forbidden dependency is absent"), Source.Contains(Term));
	}
	return true;
}

#undef CROSS_SELECTION_PLAN_BRIDGE_TEST
#undef CROSS_SELECTION_PLAN_COMPOSITION_TEST

#endif
