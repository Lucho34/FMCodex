#include "LongShotDirectShotPlanQuery.h"
#include "SingleCardFormulaResolutionExecutor.h"
#include "SingleCardFormulaResolverInputAssembler.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace LongShotDirectShotCompositionTests
{
	const FName SkillId(TEXT("Skill.LongShot.Composition"));
	const FName AttackerCardId(TEXT("LongShot.Attacker"));
	const FName DefenderCardId(TEXT("LongShot.Defender"));
	const FName AttackerPlayerId(TEXT("Player.Attacker"));
	const FName DefenderPlayerId(TEXT("Player.Defender"));
	const FGuid LogId(611, 622, 633, 644);
	const int32 TurnIndex = 12;

	FPlayerCardRuleSnapshotSet MakePlayerCardSnapshots()
	{
		FPlayerCardRuleSnapshot Attacker;
		Attacker.CardId = AttackerCardId;
		Attacker.PositionTypes = { EPlayerPositionType::Attack };
		Attacker.Attributes.LongShot = 5;
		Attacker.Attributes.Stamina = 5;
		Attacker.SkillIds = { SkillId };

		FPlayerCardRuleSnapshot Defender;
		Defender.CardId = DefenderCardId;
		Defender.PositionTypes = { EPlayerPositionType::Defense };
		Defender.Attributes.Tackling = 4;
		Defender.Attributes.Stamina = 3;

		FPlayerCardRuleSnapshotSet SnapshotSet;
		SnapshotSet.Cards = { Attacker, Defender };
		return SnapshotSet;
	}

	FSkillRuleSnapshotSet MakeSkillRules()
	{
		FSkillRuleSnapshot Rule;
		Rule.SkillId = SkillId;
		Rule.SkillType = ESkillRuleType::LongShot;
		Rule.MinTriggerActionPoint = 3;
		Rule.MaxTriggerActionPoint = 6;

		FSkillRuleSnapshotSet SnapshotSet;
		SnapshotSet.SkillRules = { Rule };
		return SnapshotSet;
	}

	FLongShotDirectShotPlanQueryInput MakePlanInput(
		const int32 AttackD6,
		const int32 DefenseD6)
	{
		FLongShotDirectShotPlanQueryInput Input;
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

	struct FCompositionResult
	{
		FLongShotDirectShotPlanQueryResult PlanResult;
		FSingleCardFormulaInputAssemblyQueryResult AttackerQueryResult;
		FSingleCardFormulaInputAssemblyQueryResult DefenderQueryResult;
		FSingleCardFormulaResolverInputAssemblyResult AssemblyResult;
		FSingleCardFormulaResolutionExecutionResult ExecutionResult;
	};

	FCompositionResult Compose(
		const FPlayerCardRuleSnapshotSet& PlayerCardSnapshots,
		const FSkillRuleSnapshotSet& SkillRules,
		const FLongShotDirectShotPlanQueryInput& Input)
	{
		FCompositionResult Result;
		Result.PlanResult =
			FLongShotDirectShotPlanQuery::BuildPlan(
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
		const FLongShotDirectShotFormulaPlan& Left,
		const FLongShotDirectShotFormulaPlan& Right)
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
		const FLongShotDirectShotPlanQueryInput& Left,
		const FLongShotDirectShotPlanQueryInput& Right)
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
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLongShotCompositionAttackD6ThreeTest,
	"FMCodex.CoreRules.LongShotDirectShotComposition.AttackD6ThreeComposesFormulaPlanEndToEnd",
	EAutomationTestFlags::EditorContext
		| EAutomationTestFlags::EngineFilter)

bool FLongShotCompositionAttackD6ThreeTest::RunTest(
	const FString& Parameters)
{
	using namespace LongShotDirectShotCompositionTests;

	const FCompositionResult Result = Compose(
		MakePlayerCardSnapshots(),
		MakeSkillRules(),
		MakePlanInput(3, 2));

	TestTrue(TEXT("Plan Query succeeds"), Result.PlanResult.bSuccess);
	TestEqual(
		TEXT("Formula resolution is required"),
		Result.PlanResult.Decision,
		ELongShotDirectShotDecision::FormulaResolutionRequired);
	TestTrue(
		TEXT("Formula Plan is generated"),
		Result.PlanResult.bHasFormulaPlan);
	TestTrue(
		TEXT("Attacker Input Assembly Query succeeds"),
		Result.AttackerQueryResult.bSuccess);
	TestTrue(
		TEXT("Defender Input Assembly Query succeeds"),
		Result.DefenderQueryResult.bSuccess);
	TestTrue(
		TEXT("Resolver Input assembly succeeds"),
		Result.AssemblyResult.bSuccess);
	TestTrue(
		TEXT("Executor succeeds"),
		Result.ExecutionResult.bSuccess);
	TestTrue(
		TEXT("Executor invokes the existing resolver"),
		Result.ExecutionResult.bExecuted);

	const FLongShotDirectShotFormulaPlan& Plan =
		Result.PlanResult.FormulaPlan;
	TestEqual(
		TEXT("Attacker FormulaType is Finishing"),
		Plan.AttackerQueryInput.FormulaType,
		EFormulaType::Finishing);
	TestEqual(
		TEXT("Defender FormulaType is Finishing"),
		Plan.DefenderQueryInput.FormulaType,
		EFormulaType::Finishing);
	TestEqual(
		TEXT("Attacker selects LongShot"),
		Plan.AttackerQueryInput.Attribute,
		ESingleCardFormulaAttribute::LongShot);
	TestEqual(
		TEXT("Defender selects Tackling"),
		Plan.DefenderQueryInput.Attribute,
		ESingleCardFormulaAttribute::Tackling);
	TestEqual(
		TEXT("Attacker Modifier is zero"),
		Plan.AttackerQueryInput.ExternalModifier,
		0.0f);
	TestEqual(
		TEXT("Defender Modifier is plus two"),
		Plan.DefenderQueryInput.ExternalModifier,
		2.0f);

	const FFormulaResolverInput& ResolverInput =
		Result.AssemblyResult.ResolverInput;
	TestEqual(
		TEXT("Resolver FormulaType remains Finishing"),
		ResolverInput.FormulaType,
		EFormulaType::Finishing);
	TestEqual(
		TEXT("Attacker LongShot becomes BaseValue"),
		ResolverInput.Attacker.BaseValue,
		5.0f);
	TestEqual(
		TEXT("Defender Tackling becomes BaseValue"),
		ResolverInput.Defender.BaseValue,
		4.0f);
	TestEqual(
		TEXT("Attacker D6 is preserved"),
		ResolverInput.Attacker.ComparePoint,
		3);
	TestEqual(
		TEXT("Defender D6 is preserved"),
		ResolverInput.Defender.ComparePoint,
		2);
	TestTrue(
		TEXT("Attacker D6 source flag is preserved"),
		ResolverInput.Attacker.bComparePointWasRolledOnD6);
	TestTrue(
		TEXT("Defender D6 source flag is preserved"),
		ResolverInput.Defender.bComparePointWasRolledOnD6);
	TestEqual(
		TEXT("Attacker Modifier reaches ResolverInput"),
		ResolverInput.Attacker.Modifier,
		0.0f);
	TestEqual(
		TEXT("Defender Modifier reaches ResolverInput"),
		ResolverInput.Defender.Modifier,
		2.0f);
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

	const FFormulaResolutionResult& FormulaResult =
		Result.ExecutionResult.FormulaResolutionResult;
	TestEqual(
		TEXT("Attacker final value follows existing rule"),
		FormulaResult.AttackerFinalValue,
		8.0f);
	TestEqual(
		TEXT("Defender final value follows existing rule"),
		FormulaResult.DefenderFinalValue,
		8.0f);
	TestEqual(
		TEXT("Attacker wins the stamina tie"),
		FormulaResult.Winner,
		EFormulaWinner::Attacker);
	TestEqual(
		TEXT("Existing stamina tie reason is retained"),
		FormulaResult.WinReason,
		EFormulaWinReason::StaminaTieBreaker);
	TestTrue(TEXT("Winning Finishing result is a goal"), FormulaResult.bIsGoal);
	TestTrue(TEXT("Finishing ends the attack"), FormulaResult.bAttackEnded);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLongShotCompositionAttackD6SixTest,
	"FMCodex.CoreRules.LongShotDirectShotComposition.AttackD6SixComposesFormulaPlanEndToEnd",
	EAutomationTestFlags::EditorContext
		| EAutomationTestFlags::EngineFilter)

bool FLongShotCompositionAttackD6SixTest::RunTest(
	const FString& Parameters)
{
	using namespace LongShotDirectShotCompositionTests;

	const FCompositionResult Result = Compose(
		MakePlayerCardSnapshots(),
		MakeSkillRules(),
		MakePlanInput(6, 2));

	TestTrue(TEXT("D6 six Plan succeeds"), Result.PlanResult.bSuccess);
	TestTrue(
		TEXT("D6 six Plan enters Input Assembly"),
		Result.AttackerQueryResult.bSuccess
			&& Result.DefenderQueryResult.bSuccess);
	TestTrue(
		TEXT("D6 six Plan enters Resolver assembly"),
		Result.AssemblyResult.bSuccess);
	TestTrue(
		TEXT("D6 six Plan executes"),
		Result.ExecutionResult.bSuccess
			&& Result.ExecutionResult.bExecuted);
	TestEqual(
		TEXT("High attack D6 is preserved"),
		Result.AssemblyResult.ResolverInput.Attacker.ComparePoint,
		6);
	TestEqual(
		TEXT("Defense D6 is preserved"),
		Result.AssemblyResult.ResolverInput.Defender.ComparePoint,
		2);

	const FFormulaResolutionResult& FormulaResult =
		Result.ExecutionResult.FormulaResolutionResult;
	TestEqual(
		TEXT("Attacker final value follows existing rule"),
		FormulaResult.AttackerFinalValue,
		11.0f);
	TestEqual(
		TEXT("Defender final value follows existing rule"),
		FormulaResult.DefenderFinalValue,
		8.0f);
	TestEqual(
		TEXT("Existing resolver selects attacker"),
		FormulaResult.Winner,
		EFormulaWinner::Attacker);
	TestEqual(
		TEXT("Existing fast suppression reason is retained"),
		FormulaResult.WinReason,
		EFormulaWinReason::FastSuppression);
	TestTrue(
		TEXT("Existing fast suppression is triggered"),
		FormulaResult.bFastSuppressionTriggered);
	TestTrue(TEXT("Attacker scores"), FormulaResult.bIsGoal);
	TestTrue(TEXT("Attack ends"), FormulaResult.bAttackEnded);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLongShotCompositionImmediateMissTest,
	"FMCodex.CoreRules.LongShotDirectShotComposition.ImmediateMissDoesNotEnterFormulaComposition",
	EAutomationTestFlags::EditorContext
		| EAutomationTestFlags::EngineFilter)

bool FLongShotCompositionImmediateMissTest::RunTest(
	const FString& Parameters)
{
	using namespace LongShotDirectShotCompositionTests;

	FLongShotDirectShotPlanQueryInput Input = MakePlanInput(1, 0);
	Input.bHasExternalDefenseD6 = false;
	const FLongShotDirectShotPlanQueryResult Result =
		FLongShotDirectShotPlanQuery::BuildPlan(
			MakePlayerCardSnapshots(),
			MakeSkillRules(),
			Input);

	TestTrue(TEXT("Immediate miss succeeds"), Result.bSuccess);
	TestEqual(
		TEXT("Decision is ImmediateMiss"),
		Result.Decision,
		ELongShotDirectShotDecision::ImmediateMiss);
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
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLongShotCompositionInputImmutabilityTest,
	"FMCodex.CoreRules.LongShotDirectShotComposition.FormulaPlanPreservesInputImmutabilityAcrossComposition",
	EAutomationTestFlags::EditorContext
		| EAutomationTestFlags::EngineFilter)

bool FLongShotCompositionInputImmutabilityTest::RunTest(
	const FString& Parameters)
{
	using namespace LongShotDirectShotCompositionTests;

	const FPlayerCardRuleSnapshotSet PlayerCardSnapshots =
		MakePlayerCardSnapshots();
	const FPlayerCardRuleSnapshotSet PlayerCardSnapshotsBefore =
		PlayerCardSnapshots;
	const FSkillRuleSnapshotSet SkillRules = MakeSkillRules();
	const FSkillRuleSnapshotSet SkillRulesBefore = SkillRules;
	const FLongShotDirectShotPlanQueryInput Input =
		MakePlanInput(4, 3);
	const FLongShotDirectShotPlanQueryInput InputBefore = Input;

	const FLongShotDirectShotPlanQueryResult PlanResult =
		FLongShotDirectShotPlanQuery::BuildPlan(
			PlayerCardSnapshots,
			SkillRules,
			Input);
	TestTrue(TEXT("Plan Query succeeds"), PlanResult.bSuccess);
	TestTrue(TEXT("Plan exists"), PlanResult.bHasFormulaPlan);
	const FLongShotDirectShotFormulaPlan PlanBefore =
		PlanResult.FormulaPlan;

	const FCompositionResult CompositionResult = Compose(
		PlayerCardSnapshots,
		SkillRules,
		Input);
	TestTrue(
		TEXT("Composition succeeds"),
		CompositionResult.ExecutionResult.bSuccess);
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
	FLongShotCompositionForbiddenDependenciesTest,
	"FMCodex.CoreRules.LongShotDirectShotComposition.HasNoForbiddenDependencies",
	EAutomationTestFlags::EditorContext
		| EAutomationTestFlags::EngineFilter)

bool FLongShotCompositionForbiddenDependenciesTest::RunTest(
	const FString& Parameters)
{
	const FString SourcePath = FPaths::Combine(
		FPaths::ProjectDir(),
		TEXT("Source/FMCodex/CoreRules/LongShotDirectShotCompositionTests.cpp"));
	FString Source;
	const bool bLoaded =
		FFileHelper::LoadFileToString(Source, *SourcePath);
	TestTrue(TEXT("Composition source loads"), bLoaded);
	if (!bLoaded)
	{
		return false;
	}

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
			TEXT("Forbidden dependency is absent"),
			Source.Contains(ForbiddenTerm));
	}
	return true;
}

#endif
