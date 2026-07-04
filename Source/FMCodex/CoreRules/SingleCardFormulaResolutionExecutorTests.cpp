#include "SingleCardFormulaResolutionExecutor.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

#include <limits>

namespace SingleCardFormulaResolutionExecutorTests
{
	const FName AttackerCardId(TEXT("Card.Attacker"));
	const FName DefenderCardId(TEXT("Card.Defender"));
	const FGuid SharedLogId(11, 22, 33, 44);

	FFormulaResolverInput MakeValidInput(
		const EFormulaType FormulaType = EFormulaType::Finishing)
	{
		FFormulaResolverInput Input;
		Input.FormulaType = FormulaType;
		Input.Attacker.BaseValue = 5.0f;
		Input.Attacker.Modifier = 1.0f;
		Input.Attacker.ComparePoint = 4;
		Input.Attacker.bComparePointWasRolledOnD6 = true;
		Input.Attacker.ParticipatingStamina.Add(5);
		Input.Defender.BaseValue = 4.0f;
		Input.Defender.Modifier = 0.0f;
		Input.Defender.ComparePoint = 2;
		Input.Defender.bComparePointWasRolledOnD6 = true;
		Input.Defender.ParticipatingStamina.Add(4);
		Input.LogId = SharedLogId;
		Input.TurnIndex = 3;
		Input.AttackerPlayerId = TEXT("PlayerA");
		Input.DefenderPlayerId = TEXT("PlayerB");
		Input.InvolvedCardIds =
		{
			AttackerCardId,
			DefenderCardId
		};
		return Input;
	}

	void TestRejected(
		FAutomationTestBase& Test,
		const FString& Label,
		const FFormulaResolverInput& Input,
		const FName ExpectedSide,
		const FName ExpectedField)
	{
		const FSingleCardFormulaResolutionExecutionResult Result =
			FSingleCardFormulaResolutionExecutor::Execute(Input);

		Test.TestFalse(*FString::Printf(TEXT("%s fails"), *Label), Result.bSuccess);
		Test.TestFalse(
			*FString::Printf(TEXT("%s does not execute Resolver"), *Label),
			Result.bExecuted);
		Test.TestEqual(
			*FString::Printf(TEXT("%s uses InvalidInput"), *Label),
			Result.ErrorCode,
			ESingleCardFormulaResolutionExecutionErrorCode::InvalidInput);
		Test.TestEqual(
			*FString::Printf(TEXT("%s reports side"), *Label),
			Result.InvalidSide,
			ExpectedSide);
		Test.TestEqual(
			*FString::Printf(TEXT("%s reports field"), *Label),
			Result.InvalidField,
			ExpectedField);
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSingleCardFormulaResolutionExecutorFinishingTest,
	"FMCodex.CoreRules.SingleCardFormulaResolutionExecutor.ExecutesFinishing",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSingleCardFormulaResolutionExecutorFinishingTest::RunTest(
	const FString& Parameters)
{
	const FFormulaResolverInput Input =
		SingleCardFormulaResolutionExecutorTests::MakeValidInput();
	const FSingleCardFormulaResolutionExecutionResult Result =
		FSingleCardFormulaResolutionExecutor::Execute(Input);

	TestTrue(TEXT("Execution succeeds"), Result.bSuccess);
	TestTrue(TEXT("Resolver executes"), Result.bExecuted);
	TestEqual(
		TEXT("ErrorCode is None"),
		Result.ErrorCode,
		ESingleCardFormulaResolutionExecutionErrorCode::None);
	TestEqual(
		TEXT("Resolver result is retained"),
		Result.FormulaResolutionResult.FormulaType,
		EFormulaType::Finishing);
	TestEqual(
		TEXT("Attacker final value is retained"),
		Result.FormulaResolutionResult.AttackerFinalValue,
		10.0f);
	TestEqual(
		TEXT("Defender final value is retained"),
		Result.FormulaResolutionResult.DefenderFinalValue,
		6.0f);
	TestEqual(
		TEXT("Winner is retained"),
		Result.FormulaResolutionResult.Winner,
		EFormulaWinner::Attacker);
	TestTrue(
		TEXT("Finishing goal is retained"),
		Result.FormulaResolutionResult.bIsGoal);
	TestEqual(
		TEXT("Input LogId is retained"),
		Result.ResolverInput.LogId,
		Input.LogId);
	TestEqual(
		TEXT("Input TurnIndex is retained"),
		Result.ResolverInput.TurnIndex,
		Input.TurnIndex);
	TestEqual(
		TEXT("Attacker CardId remains first"),
		Result.ResolverInput.InvolvedCardIds[0],
		SingleCardFormulaResolutionExecutorTests::AttackerCardId);
	TestEqual(
		TEXT("Defender CardId remains second"),
		Result.ResolverInput.InvolvedCardIds[1],
		SingleCardFormulaResolutionExecutorTests::DefenderCardId);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSingleCardFormulaResolutionExecutorTransitionTest,
	"FMCodex.CoreRules.SingleCardFormulaResolutionExecutor.ExecutesTransition",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSingleCardFormulaResolutionExecutorTransitionTest::RunTest(
	const FString& Parameters)
{
	const FFormulaResolverInput Input =
		SingleCardFormulaResolutionExecutorTests::MakeValidInput(
			EFormulaType::Transition);
	const FSingleCardFormulaResolutionExecutionResult Result =
		FSingleCardFormulaResolutionExecutor::Execute(Input);

	TestTrue(TEXT("Transition execution succeeds"), Result.bSuccess);
	TestTrue(TEXT("Transition executes Resolver"), Result.bExecuted);
	TestEqual(
		TEXT("Transition result is retained"),
		Result.FormulaResolutionResult.FormulaType,
		EFormulaType::Transition);
	TestTrue(
		TEXT("Transition attacker win continues resolution"),
		Result.FormulaResolutionResult.bContinueResolution);
	TestFalse(
		TEXT("Transition is not a goal"),
		Result.FormulaResolutionResult.bIsGoal);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSingleCardFormulaResolutionExecutorUnsupportedFormulaTest,
	"FMCodex.CoreRules.SingleCardFormulaResolutionExecutor.RejectsUnsupportedFormula",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSingleCardFormulaResolutionExecutorUnsupportedFormulaTest::RunTest(
	const FString& Parameters)
{
	FFormulaResolverInput Input =
		SingleCardFormulaResolutionExecutorTests::MakeValidInput();
	Input.FormulaType = EFormulaType::Determination;

	const FSingleCardFormulaResolutionExecutionResult Result =
		FSingleCardFormulaResolutionExecutor::Execute(Input);

	TestFalse(TEXT("Determination fails"), Result.bSuccess);
	TestFalse(TEXT("Determination does not execute Resolver"), Result.bExecuted);
	TestEqual(
		TEXT("Unsupported formula error is structured"),
		Result.ErrorCode,
		ESingleCardFormulaResolutionExecutionErrorCode
			::UnsupportedFormulaType);
	TestEqual(
		TEXT("FormulaType field is reported"),
		Result.InvalidField,
		FName(TEXT("FormulaType")));
	TestEqual(
		TEXT("Original FormulaType is retained"),
		Result.ResolverInput.FormulaType,
		EFormulaType::Determination);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSingleCardFormulaResolutionExecutorSideValidationTest,
	"FMCodex.CoreRules.SingleCardFormulaResolutionExecutor.ValidatesSideInputs",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSingleCardFormulaResolutionExecutorSideValidationTest::RunTest(
	const FString& Parameters)
{
	FFormulaResolverInput BaseValueInput =
		SingleCardFormulaResolutionExecutorTests::MakeValidInput();
	BaseValueInput.Attacker.BaseValue =
		std::numeric_limits<float>::quiet_NaN();
	SingleCardFormulaResolutionExecutorTests::TestRejected(
		*this,
		TEXT("Non-finite attacker BaseValue"),
		BaseValueInput,
		TEXT("Attacker"),
		TEXT("BaseValue"));

	FFormulaResolverInput ModifierInput =
		SingleCardFormulaResolutionExecutorTests::MakeValidInput();
	ModifierInput.Defender.Modifier =
		std::numeric_limits<float>::infinity();
	SingleCardFormulaResolutionExecutorTests::TestRejected(
		*this,
		TEXT("Non-finite defender Modifier"),
		ModifierInput,
		TEXT("Defender"),
		TEXT("Modifier"));

	FFormulaResolverInput D6SourceInput =
		SingleCardFormulaResolutionExecutorTests::MakeValidInput();
	D6SourceInput.Attacker.bComparePointWasRolledOnD6 = false;
	SingleCardFormulaResolutionExecutorTests::TestRejected(
		*this,
		TEXT("Missing external D6 source"),
		D6SourceInput,
		TEXT("Attacker"),
		TEXT("bComparePointWasRolledOnD6"));

	FFormulaResolverInput D6RangeInput =
		SingleCardFormulaResolutionExecutorTests::MakeValidInput();
	D6RangeInput.Defender.ComparePoint = 7;
	SingleCardFormulaResolutionExecutorTests::TestRejected(
		*this,
		TEXT("Out-of-range D6"),
		D6RangeInput,
		TEXT("Defender"),
		TEXT("ComparePoint"));

	FFormulaResolverInput StaminaInput =
		SingleCardFormulaResolutionExecutorTests::MakeValidInput();
	StaminaInput.Attacker.ParticipatingStamina.Add(3);
	SingleCardFormulaResolutionExecutorTests::TestRejected(
		*this,
		TEXT("Multiple attacker Stamina values"),
		StaminaInput,
		TEXT("Attacker"),
		TEXT("ParticipatingStamina"));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSingleCardFormulaResolutionExecutorContextValidationTest,
	"FMCodex.CoreRules.SingleCardFormulaResolutionExecutor.ValidatesContext",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSingleCardFormulaResolutionExecutorContextValidationTest::RunTest(
	const FString& Parameters)
{
	FFormulaResolverInput TurnInput =
		SingleCardFormulaResolutionExecutorTests::MakeValidInput();
	TurnInput.TurnIndex = -1;
	SingleCardFormulaResolutionExecutorTests::TestRejected(
		*this,
		TEXT("Negative TurnIndex"),
		TurnInput,
		NAME_None,
		TEXT("TurnIndex"));

	FFormulaResolverInput CardCountInput =
		SingleCardFormulaResolutionExecutorTests::MakeValidInput();
	CardCountInput.InvolvedCardIds.Pop();
	SingleCardFormulaResolutionExecutorTests::TestRejected(
		*this,
		TEXT("Missing defender CardId"),
		CardCountInput,
		NAME_None,
		TEXT("InvolvedCardIds"));

	FFormulaResolverInput EmptyCardInput =
		SingleCardFormulaResolutionExecutorTests::MakeValidInput();
	EmptyCardInput.InvolvedCardIds[0] = NAME_None;
	SingleCardFormulaResolutionExecutorTests::TestRejected(
		*this,
		TEXT("Empty attacker CardId"),
		EmptyCardInput,
		NAME_None,
		TEXT("InvolvedCardIds"));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSingleCardFormulaResolutionExecutorInputUnchangedTest,
	"FMCodex.CoreRules.SingleCardFormulaResolutionExecutor.InputIsUnchanged",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSingleCardFormulaResolutionExecutorInputUnchangedTest::RunTest(
	const FString& Parameters)
{
	const FFormulaResolverInput Input =
		SingleCardFormulaResolutionExecutorTests::MakeValidInput();
	const FFormulaResolverInput Before = Input;

	const FSingleCardFormulaResolutionExecutionResult Result =
		FSingleCardFormulaResolutionExecutor::Execute(Input);

	TestTrue(TEXT("Execution succeeds"), Result.bSuccess);
	TestEqual(
		TEXT("FormulaType is unchanged"),
		Input.FormulaType,
		Before.FormulaType);
	TestEqual(
		TEXT("Attacker BaseValue is unchanged"),
		Input.Attacker.BaseValue,
		Before.Attacker.BaseValue);
	TestEqual(
		TEXT("Defender Modifier is unchanged"),
		Input.Defender.Modifier,
		Before.Defender.Modifier);
	TestEqual(
		TEXT("TurnIndex is unchanged"),
		Input.TurnIndex,
		Before.TurnIndex);
	TestEqual(
		TEXT("CardId count is unchanged"),
		Input.InvolvedCardIds.Num(),
		Before.InvolvedCardIds.Num());
	TestEqual(
		TEXT("Attacker CardId is unchanged"),
		Input.InvolvedCardIds[0],
		Before.InvolvedCardIds[0]);
	TestEqual(
		TEXT("Defender CardId is unchanged"),
		Input.InvolvedCardIds[1],
		Before.InvolvedCardIds[1]);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSingleCardFormulaResolutionExecutorDependencyTest,
	"FMCodex.CoreRules.SingleCardFormulaResolutionExecutor.HasOnlyAllowedDependencies",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSingleCardFormulaResolutionExecutorDependencyTest::RunTest(
	const FString& Parameters)
{
	const TArray<FString> FileNames =
	{
		TEXT("SingleCardFormulaResolutionExecutor.h"),
		TEXT("SingleCardFormulaResolutionExecutor.cpp")
	};
	FString CombinedSource;
	for (const FString& FileName : FileNames)
	{
		FString Source;
		const bool bLoaded = FFileHelper::LoadFileToString(
			Source,
			*(FPaths::ProjectDir()
				/ TEXT("Source/FMCodex/CoreRules")
				/ FileName));
		TestTrue(
			*FString::Printf(TEXT("%s can be loaded"), *FileName),
			bLoaded);
		CombinedSource += Source;
	}

	int32 ResolverCallCount = 0;
	FString RemainingSource = CombinedSource;
	const FString ResolverCall(TEXT("UFormulaResolver::ResolveFormula("));
	while (RemainingSource.Split(
		ResolverCall,
		nullptr,
		&RemainingSource))
	{
		++ResolverCallCount;
	}
	TestEqual(
		TEXT("FormulaResolver is called exactly once in production source"),
		ResolverCallCount,
		1);
	TestFalse(
		TEXT("No Query dependency"),
		CombinedSource.Contains(TEXT("QueryResult"))
			|| CombinedSource.Contains(TEXT("AssemblyQuery"))
			|| CombinedSource.Contains(
				TEXT("FPlayerCardRuleSnapshotQuery")));
	TestFalse(
		TEXT("No Assembler dependency"),
		CombinedSource.Contains(
			TEXT("SingleCardFormulaResolverInputAssembler")));
	TestFalse(
		TEXT("No Flow, MatchPlay, or External API integration"),
		CombinedSource.Contains(TEXT("FormulaAttackFlow"))
			|| CombinedSource.Contains(TEXT("MatchPlay"))
			|| CombinedSource.Contains(TEXT("ExternalTurn"))
			|| CombinedSource.Contains(TEXT("ExternalApi")));
	TestFalse(
		TEXT("No random generation"),
		CombinedSource.Contains(TEXT("FMath::Rand"))
			|| CombinedSource.Contains(TEXT("RandomStream")));
	TestFalse(
		TEXT("No TieBreaker semantics"),
		CombinedSource.Contains(TEXT("TieBreaker")));
	TestFalse(
		TEXT("No skill effect"),
		CombinedSource.Contains(TEXT("Skill")));
	TestFalse(
		TEXT("No Provider, DataTable, or database dependency"),
		CombinedSource.Contains(TEXT("Provider"))
			|| CombinedSource.Contains(TEXT("DataTable"))
			|| CombinedSource.Contains(TEXT("Database")));
	TestFalse(
		TEXT("No deck-zone semantics"),
		CombinedSource.Contains(TEXT("DrawPile"))
			|| CombinedSource.Contains(TEXT("Shuffle"))
			|| CombinedSource.Contains(TEXT("HandCards"))
			|| CombinedSource.Contains(TEXT("Deck")));
	return true;
}

#endif
