#include "CutInsideShotDirectShotPlanQuery.h"
#include "SingleCardFormulaResolutionExecutor.h"
#include "SingleCardFormulaResolverInputAssembler.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace CutInsideShotDirectShotCompositionTests
{
	const FName SkillId(TEXT("Skill.CutInsideShot.Composition"));
	const FName AttackerCardId(TEXT("CutInsideShot.Attacker"));
	const FName DefenderCardId(TEXT("CutInsideShot.Defender"));
	const FName AttackerPlayerId(TEXT("Player.CutInside.Attacker"));
	const FName DefenderPlayerId(TEXT("Player.CutInside.Defender"));
	const FGuid LogId(621, 622, 623, 624);
	const int32 TurnIndex = 13;

	FPlayerCardRuleSnapshotSet MakePlayerCardSnapshots(
		const int32 Shooting,
		const int32 Dribbling,
		const int32 Tackling)
	{
		FPlayerCardRuleSnapshot Attacker;
		Attacker.CardId = AttackerCardId;
		Attacker.PositionTypes = { EPlayerPositionType::Attack };
		Attacker.Attributes.Shooting = Shooting;
		Attacker.Attributes.Dribbling = Dribbling;
		Attacker.Attributes.Stamina = 5;
		Attacker.SkillIds = { SkillId };

		FPlayerCardRuleSnapshot Defender;
		Defender.CardId = DefenderCardId;
		Defender.PositionTypes = { EPlayerPositionType::Defense };
		Defender.Attributes.Tackling = Tackling;
		Defender.Attributes.Stamina = 3;

		FPlayerCardRuleSnapshotSet SnapshotSet;
		SnapshotSet.Cards = { Attacker, Defender };
		return SnapshotSet;
	}

	FSkillRuleSnapshotSet MakeSkillRules()
	{
		FSkillRuleSnapshot Rule;
		Rule.SkillId = SkillId;
		Rule.SkillType = ESkillRuleType::CutInsideShot;
		Rule.MinTriggerActionPoint = 3;
		Rule.MaxTriggerActionPoint = 6;

		FSkillRuleSnapshotSet SnapshotSet;
		SnapshotSet.SkillRules = { Rule };
		return SnapshotSet;
	}

	FCutInsideShotDirectShotPlanQueryInput MakePlanInput(
		const int32 AttackD6,
		const int32 DefenseD6)
	{
		FCutInsideShotDirectShotPlanQueryInput Input;
		Input.SkillId = SkillId;
		Input.AttackerCardId = AttackerCardId;
		Input.DefenderCardId = DefenderCardId;
		Input.CurrentActionPoint = 4;
		Input.bHasExternalAttackD6 = true;
		Input.ExternalAttackD6 = AttackD6;
		Input.bHasExternalDefenseD6 = true;
		Input.ExternalDefenseD6 = DefenseD6;
		Input.LogId = LogId;
		Input.TurnIndex = TurnIndex;
		Input.AttackerPlayerId = AttackerPlayerId;
		Input.DefenderPlayerId = DefenderPlayerId;
		return Input;
	}

	float ExpectedAverageValue(
		const int32 Shooting,
		const int32 Dribbling)
	{
		const int32 AverageTenths = (Shooting + Dribbling) * 5;
		return static_cast<float>(AverageTenths) / 10.0f;
	}

	float ExpectedDerivedModifier(
		const int32 Shooting,
		const int32 Dribbling)
	{
		const int32 ModifierTenths = (Dribbling - Shooting) * 5;
		return static_cast<float>(ModifierTenths) / 10.0f;
	}

	struct FCompositionResult
	{
		FCutInsideShotDirectShotPlanQueryResult PlanResult;
		FSingleCardFormulaInputAssemblyQueryResult AttackerQueryResult;
		FSingleCardFormulaInputAssemblyQueryResult DefenderQueryResult;
		FSingleCardFormulaResolverInputAssemblyResult AssemblyResult;
		FSingleCardFormulaResolutionExecutionResult ExecutionResult;
	};

	FCompositionResult Compose(
		const FPlayerCardRuleSnapshotSet& PlayerCardSnapshots,
		const FSkillRuleSnapshotSet& SkillRules,
		const FCutInsideShotDirectShotPlanQueryInput& Input)
	{
		FCompositionResult Result;
		Result.PlanResult =
			FCutInsideShotDirectShotPlanQuery::BuildPlan(
				PlayerCardSnapshots,
				SkillRules,
				Input);
		if (!Result.PlanResult.bSuccess
			|| !Result.PlanResult.bHasFormulaPlan)
		{
			return Result;
		}

		Result.AttackerQueryResult =
			FSingleCardFormulaInputAssemblyQuery::Assemble(
				PlayerCardSnapshots,
				Result.PlanResult.FormulaPlan.AttackerQueryInput);
		Result.DefenderQueryResult =
			FSingleCardFormulaInputAssemblyQuery::Assemble(
				PlayerCardSnapshots,
				Result.PlanResult.FormulaPlan.DefenderQueryInput);
		if (!Result.AttackerQueryResult.bSuccess
			|| !Result.DefenderQueryResult.bSuccess)
		{
			return Result;
		}

		FSingleCardFormulaResolverInputAssemblyInput AssemblyInput;
		AssemblyInput.AttackerQueryResult =
			Result.AttackerQueryResult;
		AssemblyInput.DefenderQueryResult =
			Result.DefenderQueryResult;
		AssemblyInput.AttackerPlayerId =
			Result.PlanResult.FormulaPlan.AttackerPlayerId;
		AssemblyInput.DefenderPlayerId =
			Result.PlanResult.FormulaPlan.DefenderPlayerId;
		Result.AssemblyResult =
			FSingleCardFormulaResolverInputAssembler::Assemble(
				AssemblyInput);
		if (!Result.AssemblyResult.bSuccess)
		{
			return Result;
		}

		Result.ExecutionResult =
			FSingleCardFormulaResolutionExecutor::Execute(
				Result.AssemblyResult.ResolverInput);
		return Result;
	}

	bool AreQueryInputsEqual(
		const FSingleCardFormulaInputAssemblyQueryInput& Left,
		const FSingleCardFormulaInputAssemblyQueryInput& Right)
	{
		return Left.CardId == Right.CardId
			&& Left.FormulaType == Right.FormulaType
			&& Left.ParticipantRole == Right.ParticipantRole
			&& Left.Attribute == Right.Attribute
			&& Left.bHasExternalD6ComparePoint
				== Right.bHasExternalD6ComparePoint
			&& Left.ExternalD6ComparePoint
				== Right.ExternalD6ComparePoint
			&& Left.bHasExternalModifier
				== Right.bHasExternalModifier
			&& Left.ExternalModifier == Right.ExternalModifier
			&& Left.LogId == Right.LogId
			&& Left.TurnIndex == Right.TurnIndex
			&& Left.ContextTag == Right.ContextTag;
	}

	bool AreFormulaPlansEqual(
		const FCutInsideShotDirectShotFormulaPlan& Left,
		const FCutInsideShotDirectShotFormulaPlan& Right)
	{
		return AreQueryInputsEqual(
				Left.AttackerQueryInput,
				Right.AttackerQueryInput)
			&& AreQueryInputsEqual(
				Left.DefenderQueryInput,
				Right.DefenderQueryInput)
			&& Left.AttackerPlayerId == Right.AttackerPlayerId
			&& Left.DefenderPlayerId == Right.DefenderPlayerId;
	}

	bool ArePlanInputsEqual(
		const FCutInsideShotDirectShotPlanQueryInput& Left,
		const FCutInsideShotDirectShotPlanQueryInput& Right)
	{
		return Left.SkillId == Right.SkillId
			&& Left.AttackerCardId == Right.AttackerCardId
			&& Left.DefenderCardId == Right.DefenderCardId
			&& Left.CurrentActionPoint == Right.CurrentActionPoint
			&& Left.bHasExternalAttackD6
				== Right.bHasExternalAttackD6
			&& Left.ExternalAttackD6 == Right.ExternalAttackD6
			&& Left.bHasExternalDefenseD6
				== Right.bHasExternalDefenseD6
			&& Left.ExternalDefenseD6 == Right.ExternalDefenseD6
			&& Left.LogId == Right.LogId
			&& Left.TurnIndex == Right.TurnIndex
			&& Left.AttackerPlayerId == Right.AttackerPlayerId
			&& Left.DefenderPlayerId == Right.DefenderPlayerId;
	}

	bool ArePlayerAttributesEqual(
		const FPlayerAttributes& Left,
		const FPlayerAttributes& Right)
	{
		return Left.Shooting == Right.Shooting
			&& Left.Dribbling == Right.Dribbling
			&& Left.Passing == Right.Passing
			&& Left.OffBall == Right.OffBall
			&& Left.Marking == Right.Marking
			&& Left.Tackling == Right.Tackling
			&& Left.Speed == Right.Speed
			&& Left.Strength == Right.Strength
			&& Left.Stamina == Right.Stamina
			&& Left.LongShot == Right.LongShot;
	}

	bool AreGoalkeeperAttributesEqual(
		const FGoalkeeperAttributes& Left,
		const FGoalkeeperAttributes& Right)
	{
		return Left.Handling == Right.Handling
			&& Left.Positioning == Right.Positioning
			&& Left.Reflex == Right.Reflex
			&& Left.Aerial == Right.Aerial
			&& Left.Anticipation == Right.Anticipation
			&& Left.OneOnOne == Right.OneOnOne;
	}

	bool ArePlayerSnapshotSetsEqual(
		const FPlayerCardRuleSnapshotSet& Left,
		const FPlayerCardRuleSnapshotSet& Right)
	{
		if (Left.Cards.Num() != Right.Cards.Num())
		{
			return false;
		}

		for (int32 Index = 0; Index < Left.Cards.Num(); ++Index)
		{
			const FPlayerCardRuleSnapshot& LeftCard =
				Left.Cards[Index];
			const FPlayerCardRuleSnapshot& RightCard =
				Right.Cards[Index];
			if (LeftCard.CardId != RightCard.CardId
				|| LeftCard.PositionTypes != RightCard.PositionTypes
				|| !ArePlayerAttributesEqual(
					LeftCard.Attributes,
					RightCard.Attributes)
				|| LeftCard.bIsGoalkeeper
					!= RightCard.bIsGoalkeeper
				|| LeftCard.bHasGoalkeeperAttributes
					!= RightCard.bHasGoalkeeperAttributes
				|| !AreGoalkeeperAttributesEqual(
					LeftCard.GoalkeeperAttributes,
					RightCard.GoalkeeperAttributes)
				|| LeftCard.Rarity != RightCard.Rarity
				|| LeftCard.SkillIds != RightCard.SkillIds)
			{
				return false;
			}
		}
		return true;
	}

	bool AreSkillRuleSnapshotSetsEqual(
		const FSkillRuleSnapshotSet& Left,
		const FSkillRuleSnapshotSet& Right)
	{
		if (Left.SkillRules.Num() != Right.SkillRules.Num())
		{
			return false;
		}

		for (int32 Index = 0; Index < Left.SkillRules.Num(); ++Index)
		{
			const FSkillRuleSnapshot& LeftRule =
				Left.SkillRules[Index];
			const FSkillRuleSnapshot& RightRule =
				Right.SkillRules[Index];
			if (LeftRule.SkillId != RightRule.SkillId
				|| LeftRule.SkillType != RightRule.SkillType
				|| LeftRule.MinTriggerActionPoint
					!= RightRule.MinTriggerActionPoint
				|| LeftRule.MaxTriggerActionPoint
					!= RightRule.MaxTriggerActionPoint)
			{
				return false;
			}
		}
		return true;
	}

	void TestSuccessfulComposition(
		FAutomationTestBase& Test,
		const FCompositionResult& Result,
		const int32 Shooting,
		const int32 Dribbling,
		const int32 Tackling,
		const int32 AttackD6,
		const int32 DefenseD6)
	{
		Test.TestTrue(TEXT("Plan Query succeeds"), Result.PlanResult.bSuccess);
		Test.TestEqual(
			TEXT("Formula resolution is required"),
			Result.PlanResult.Decision,
			ECutInsideShotDirectShotDecision
				::FormulaResolutionRequired);
		Test.TestTrue(
			TEXT("Formula Plan is generated"),
			Result.PlanResult.bHasFormulaPlan);
		Test.TestTrue(
			TEXT("Attacker Input Assembly Query succeeds"),
			Result.AttackerQueryResult.bSuccess);
		Test.TestTrue(
			TEXT("Defender Input Assembly Query succeeds"),
			Result.DefenderQueryResult.bSuccess);
		Test.TestTrue(
			TEXT("Resolver Input assembly succeeds"),
			Result.AssemblyResult.bSuccess);
		Test.TestTrue(
			TEXT("Executor succeeds"),
			Result.ExecutionResult.bSuccess);
		Test.TestTrue(
			TEXT("Executor invokes the existing resolver"),
			Result.ExecutionResult.bExecuted);

		const FCutInsideShotDirectShotFormulaPlan& Plan =
			Result.PlanResult.FormulaPlan;
		Test.TestEqual(
			TEXT("Attacker FormulaType is Finishing"),
			Plan.AttackerQueryInput.FormulaType,
			EFormulaType::Finishing);
		Test.TestEqual(
			TEXT("Defender FormulaType is Finishing"),
			Plan.DefenderQueryInput.FormulaType,
			EFormulaType::Finishing);
		Test.TestEqual(
			TEXT("Attacker selects Shooting"),
			Plan.AttackerQueryInput.Attribute,
			ESingleCardFormulaAttribute::Shooting);
		Test.TestEqual(
			TEXT("Defender selects Tackling"),
			Plan.DefenderQueryInput.Attribute,
			ESingleCardFormulaAttribute::Tackling);
		Test.TestEqual(
			TEXT("Attacker derived modifier"),
			Plan.AttackerQueryInput.ExternalModifier,
			ExpectedDerivedModifier(Shooting, Dribbling));
		Test.TestEqual(
			TEXT("Defender Modifier is plus two"),
			Plan.DefenderQueryInput.ExternalModifier,
			2.0f);

		const FFormulaResolverInput& ResolverInput =
			Result.AssemblyResult.ResolverInput;
		Test.TestEqual(
			TEXT("Resolver FormulaType remains Finishing"),
			ResolverInput.FormulaType,
			EFormulaType::Finishing);
		Test.TestEqual(
			TEXT("Attacker Shooting becomes BaseValue"),
			ResolverInput.Attacker.BaseValue,
			static_cast<float>(Shooting));
		Test.TestEqual(
			TEXT("Defender Tackling becomes BaseValue"),
			ResolverInput.Defender.BaseValue,
			static_cast<float>(Tackling));
		Test.TestEqual(
			TEXT("Attacker D6 is preserved"),
			ResolverInput.Attacker.ComparePoint,
			AttackD6);
		Test.TestEqual(
			TEXT("Defender D6 is preserved"),
			ResolverInput.Defender.ComparePoint,
			DefenseD6);
		Test.TestTrue(
			TEXT("Attacker D6 source flag is preserved"),
			ResolverInput.Attacker.bComparePointWasRolledOnD6);
		Test.TestTrue(
			TEXT("Defender D6 source flag is preserved"),
			ResolverInput.Defender.bComparePointWasRolledOnD6);
		Test.TestEqual(
			TEXT("Attacker Modifier reaches ResolverInput"),
			ResolverInput.Attacker.Modifier,
			ExpectedDerivedModifier(Shooting, Dribbling));
		Test.TestEqual(
			TEXT("Defender Modifier reaches ResolverInput"),
			ResolverInput.Defender.Modifier,
			2.0f);

		const FFormulaResolutionResult& FormulaResult =
			Result.ExecutionResult.FormulaResolutionResult;
		Test.TestEqual(
			TEXT("Attacker final value uses average plus D6"),
			FormulaResult.AttackerFinalValue,
			ExpectedAverageValue(Shooting, Dribbling)
				+ static_cast<float>(AttackD6));
		Test.TestEqual(
			TEXT("Defender final value uses Tackling plus two plus D6"),
			FormulaResult.DefenderFinalValue,
			static_cast<float>(Tackling + DefenseD6) + 2.0f);
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCutInsideShotCompositionEqualAttributesTest,
	"FMCodex.CoreRules.CutInsideShotDirectShotComposition.EqualAttributesComposesFormulaPlanEndToEnd",
	EAutomationTestFlags::EditorContext
		| EAutomationTestFlags::EngineFilter)

bool FCutInsideShotCompositionEqualAttributesTest::RunTest(
	const FString& Parameters)
{
	using namespace CutInsideShotDirectShotCompositionTests;

	const int32 Shooting = 5;
	const int32 Dribbling = 5;
	const int32 Tackling = 2;
	const int32 AttackD6 = 3;
	const int32 DefenseD6 = 2;
	const FCompositionResult Result = Compose(
		MakePlayerCardSnapshots(Shooting, Dribbling, Tackling),
		MakeSkillRules(),
		MakePlanInput(AttackD6, DefenseD6));

	TestSuccessfulComposition(
		*this,
		Result,
		Shooting,
		Dribbling,
		Tackling,
		AttackD6,
		DefenseD6);
	TestEqual(
		TEXT("Zero modifier is produced"),
		Result.AssemblyResult.ResolverInput.Attacker.Modifier,
		0.0f);
	TestEqual(
		TEXT("Existing resolver selects attacker"),
		Result.ExecutionResult.FormulaResolutionResult.Winner,
		EFormulaWinner::Attacker);
	TestTrue(
		TEXT("Winning Finishing result is a goal"),
		Result.ExecutionResult.FormulaResolutionResult.bIsGoal);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCutInsideShotCompositionPositiveModifierTest,
	"FMCodex.CoreRules.CutInsideShotDirectShotComposition.PositiveDerivedModifierComposesFormulaPlanEndToEnd",
	EAutomationTestFlags::EditorContext
		| EAutomationTestFlags::EngineFilter)

bool FCutInsideShotCompositionPositiveModifierTest::RunTest(
	const FString& Parameters)
{
	using namespace CutInsideShotDirectShotCompositionTests;

	const int32 Shooting = 3;
	const int32 Dribbling = 5;
	const int32 Tackling = 3;
	const int32 AttackD6 = 4;
	const int32 DefenseD6 = 2;
	const FCompositionResult Result = Compose(
		MakePlayerCardSnapshots(Shooting, Dribbling, Tackling),
		MakeSkillRules(),
		MakePlanInput(AttackD6, DefenseD6));

	TestSuccessfulComposition(
		*this,
		Result,
		Shooting,
		Dribbling,
		Tackling,
		AttackD6,
		DefenseD6);
	TestEqual(
		TEXT("Positive modifier is produced"),
		Result.AssemblyResult.ResolverInput.Attacker.Modifier,
		1.0f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCutInsideShotCompositionNegativeModifierTest,
	"FMCodex.CoreRules.CutInsideShotDirectShotComposition.NegativeDerivedModifierComposesFormulaPlanEndToEnd",
	EAutomationTestFlags::EditorContext
		| EAutomationTestFlags::EngineFilter)

bool FCutInsideShotCompositionNegativeModifierTest::RunTest(
	const FString& Parameters)
{
	using namespace CutInsideShotDirectShotCompositionTests;

	const int32 Shooting = 6;
	const int32 Dribbling = 2;
	const int32 Tackling = 4;
	const int32 AttackD6 = 5;
	const int32 DefenseD6 = 1;
	const FCompositionResult Result = Compose(
		MakePlayerCardSnapshots(Shooting, Dribbling, Tackling),
		MakeSkillRules(),
		MakePlanInput(AttackD6, DefenseD6));

	TestSuccessfulComposition(
		*this,
		Result,
		Shooting,
		Dribbling,
		Tackling,
		AttackD6,
		DefenseD6);
	TestEqual(
		TEXT("Negative modifier is produced"),
		Result.AssemblyResult.ResolverInput.Attacker.Modifier,
		-2.0f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCutInsideShotCompositionHalfAverageTest,
	"FMCodex.CoreRules.CutInsideShotDirectShotComposition.HalfAverageComposesFormulaPlanEndToEnd",
	EAutomationTestFlags::EditorContext
		| EAutomationTestFlags::EngineFilter)

bool FCutInsideShotCompositionHalfAverageTest::RunTest(
	const FString& Parameters)
{
	using namespace CutInsideShotDirectShotCompositionTests;

	const int32 Shooting = 3;
	const int32 Dribbling = 6;
	const int32 Tackling = 4;
	const int32 AttackD6 = 4;
	const int32 DefenseD6 = 2;
	const FCompositionResult Result = Compose(
		MakePlayerCardSnapshots(Shooting, Dribbling, Tackling),
		MakeSkillRules(),
		MakePlanInput(AttackD6, DefenseD6));

	TestSuccessfulComposition(
		*this,
		Result,
		Shooting,
		Dribbling,
		Tackling,
		AttackD6,
		DefenseD6);
	TestEqual(
		TEXT("Half-point modifier is produced"),
		Result.AssemblyResult.ResolverInput.Attacker.Modifier,
		1.5f);
	TestEqual(
		TEXT("Half-point final value reaches existing resolver"),
		Result.ExecutionResult.FormulaResolutionResult.AttackerFinalValue,
		8.5f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCutInsideShotCompositionContextAndImmutabilityTest,
	"FMCodex.CoreRules.CutInsideShotDirectShotComposition.FormulaPlanPreservesContextAndInputImmutabilityAcrossComposition",
	EAutomationTestFlags::EditorContext
		| EAutomationTestFlags::EngineFilter)

bool FCutInsideShotCompositionContextAndImmutabilityTest::RunTest(
	const FString& Parameters)
{
	using namespace CutInsideShotDirectShotCompositionTests;

	const FPlayerCardRuleSnapshotSet PlayerCardSnapshots =
		MakePlayerCardSnapshots(4, 6, 3);
	const FPlayerCardRuleSnapshotSet PlayerCardSnapshotsBefore =
		PlayerCardSnapshots;
	const FSkillRuleSnapshotSet SkillRules = MakeSkillRules();
	const FSkillRuleSnapshotSet SkillRulesBefore = SkillRules;
	const FCutInsideShotDirectShotPlanQueryInput Input =
		MakePlanInput(5, 2);
	const FCutInsideShotDirectShotPlanQueryInput InputBefore = Input;

	const FCutInsideShotDirectShotPlanQueryResult PlanResult =
		FCutInsideShotDirectShotPlanQuery::BuildPlan(
			PlayerCardSnapshots,
			SkillRules,
			Input);
	TestTrue(TEXT("Plan Query succeeds"), PlanResult.bSuccess);
	TestTrue(TEXT("Plan exists"), PlanResult.bHasFormulaPlan);
	const FCutInsideShotDirectShotFormulaPlan PlanBefore =
		PlanResult.FormulaPlan;

	const FCompositionResult CompositionResult = Compose(
		PlayerCardSnapshots,
		SkillRules,
		Input);
	TestTrue(
		TEXT("Composition succeeds"),
		CompositionResult.ExecutionResult.bSuccess);

	const FFormulaResolverInput& ResolverInput =
		CompositionResult.AssemblyResult.ResolverInput;
	TestEqual(TEXT("LogId is preserved"), ResolverInput.LogId, LogId);
	TestEqual(
		TEXT("TurnIndex is preserved"),
		ResolverInput.TurnIndex,
		TurnIndex);
	TestEqual(
		TEXT("Attacker PlayerId is preserved"),
		ResolverInput.AttackerPlayerId,
		AttackerPlayerId);
	TestEqual(
		TEXT("Defender PlayerId is preserved"),
		ResolverInput.DefenderPlayerId,
		DefenderPlayerId);
	TestEqual(
		TEXT("Two CardIds are preserved"),
		ResolverInput.InvolvedCardIds.Num(),
		2);
	TestEqual(
		TEXT("Attacker CardId remains first"),
		ResolverInput.InvolvedCardIds[0],
		AttackerCardId);
	TestEqual(
		TEXT("Defender CardId remains second"),
		ResolverInput.InvolvedCardIds[1],
		DefenderCardId);
	TestTrue(
		TEXT("Plan Query input remains unchanged"),
		ArePlanInputsEqual(Input, InputBefore));
	TestTrue(
		TEXT("Player Snapshot Set remains unchanged"),
		ArePlayerSnapshotSetsEqual(
			PlayerCardSnapshots,
			PlayerCardSnapshotsBefore));
	TestTrue(
		TEXT("Skill Rule Snapshot Set remains unchanged"),
		AreSkillRuleSnapshotSetsEqual(
			SkillRules,
			SkillRulesBefore));
	TestTrue(
		TEXT("Original Formula Plan remains unchanged"),
		AreFormulaPlansEqual(
			PlanResult.FormulaPlan,
			PlanBefore));
	TestTrue(
		TEXT("Composed Formula Plan remains field-equivalent"),
		AreFormulaPlansEqual(
			CompositionResult.PlanResult.FormulaPlan,
			PlanBefore));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCutInsideShotCompositionImmediateMissAndForbiddenDependenciesTest,
	"FMCodex.CoreRules.CutInsideShotDirectShotComposition.ImmediateMissDoesNotEnterCompositionAndHasNoForbiddenDependencies",
	EAutomationTestFlags::EditorContext
		| EAutomationTestFlags::EngineFilter)

bool FCutInsideShotCompositionImmediateMissAndForbiddenDependenciesTest::RunTest(
	const FString& Parameters)
{
	using namespace CutInsideShotDirectShotCompositionTests;

	FCutInsideShotDirectShotPlanQueryInput Input =
		MakePlanInput(1, 0);
	Input.bHasExternalDefenseD6 = false;
	const FCutInsideShotDirectShotPlanQueryResult Result =
		FCutInsideShotDirectShotPlanQuery::BuildPlan(
			MakePlayerCardSnapshots(4, 6, 3),
			MakeSkillRules(),
			Input);

	TestTrue(TEXT("Immediate miss succeeds"), Result.bSuccess);
	TestEqual(
		TEXT("Decision is ImmediateMiss"),
		Result.Decision,
		ECutInsideShotDirectShotDecision::ImmediateMiss);
	TestTrue(TEXT("Immediate miss ends attack"), Result.bAttackEnded);
	TestFalse(TEXT("Immediate miss is not a goal"), Result.bIsGoal);
	TestFalse(TEXT("Immediate miss has no Formula Plan"), Result.bHasFormulaPlan);
	TestEqual(
		TEXT("No attacker formula input is generated"),
		Result.FormulaPlan.AttackerQueryInput.FormulaType,
		EFormulaType::None);
	TestEqual(
		TEXT("No defender formula input is generated"),
		Result.FormulaPlan.DefenderQueryInput.FormulaType,
		EFormulaType::None);

	const FString SourceDirectory = FPaths::Combine(
		FPaths::ProjectDir(),
		TEXT("Source/FMCodex/CoreRules"));
	FString QueryHeader;
	FString QueryImplementation;
	FString CompositionSource;
	const bool bQueryHeaderLoaded = FFileHelper::LoadFileToString(
		QueryHeader,
		*FPaths::Combine(
			SourceDirectory,
			TEXT("CutInsideShotDirectShotPlanQuery.h")));
	const bool bQueryImplementationLoaded =
		FFileHelper::LoadFileToString(
			QueryImplementation,
			*FPaths::Combine(
				SourceDirectory,
				TEXT("CutInsideShotDirectShotPlanQuery.cpp")));
	const bool bCompositionLoaded = FFileHelper::LoadFileToString(
		CompositionSource,
		*FPaths::Combine(
			SourceDirectory,
			TEXT("CutInsideShotDirectShotCompositionTests.cpp")));
	TestTrue(TEXT("Query header loads"), bQueryHeaderLoaded);
	TestTrue(TEXT("Query implementation loads"), bQueryImplementationLoaded);
	TestTrue(TEXT("Composition source loads"), bCompositionLoaded);
	if (!bQueryHeaderLoaded
		|| !bQueryImplementationLoaded
		|| !bCompositionLoaded)
	{
		return false;
	}

	const FString QuerySource = QueryHeader + QueryImplementation;
	TestFalse(
		TEXT("Query does not execute Input Assembly Query"),
		QuerySource.Contains(
			TEXT("FSingleCardFormulaInputAssemblyQuery::Assemble")));
	TestFalse(
		TEXT("Query does not reference Resolver Input Assembler"),
		QuerySource.Contains(
			TEXT("SingleCardFormulaResolverInputAssembler")));
	TestFalse(
		TEXT("Query does not reference Executor"),
		QuerySource.Contains(
			TEXT("SingleCardFormulaResolutionExecutor")));
	TestFalse(
		TEXT("Query does not directly reference resolver UObject"),
		QuerySource.Contains(
			FString(TEXT("UFormula")) + TEXT("Resolver")));
	TestFalse(
		TEXT("Query does not directly call resolver function"),
		QuerySource.Contains(
			FString(TEXT("Resolve")) + TEXT("Formula")));

	const TArray<FString> ForbiddenTerms = {
		FString(TEXT("Match")) + TEXT("Play"),
		FString(TEXT("External")) + TEXT("Api"),
		FString(TEXT("FormulaAttack")) + TEXT("Flow"),
		FString(TEXT("Data")) + TEXT("Table"),
		FString(TEXT("Pro")) + TEXT("vider"),
		FString(TEXT("Pipe")) + TEXT("line"),
		FString(TEXT("Skill")) + TEXT("Effect"),
		FString(TEXT("FMath::")) + TEXT("Rand"),
		FString(TEXT("F")) + TEXT("RandomStream"),
		FString(TEXT("UFormula")) + TEXT("Resolver"),
		FString(TEXT("Resolve")) + TEXT("Formula")
	};
	for (const FString& ForbiddenTerm : ForbiddenTerms)
	{
		TestFalse(
			TEXT("Composition forbidden dependency is absent"),
			CompositionSource.Contains(ForbiddenTerm));
	}
	return true;
}

#endif
