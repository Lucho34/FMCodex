#include "FormulaResolver.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

namespace FormulaResolverTests
{
	FFormulaResolverInput MakeInput(const EFormulaType FormulaType)
	{
		FFormulaResolverInput Input;
		Input.FormulaType = FormulaType;
		Input.LogId = FGuid(1, 2, 3, 4);
		Input.TurnIndex = 1;
		Input.AttackerPlayerId = TEXT("PlayerA");
		Input.DefenderPlayerId = TEXT("PlayerB");
		return Input;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFormulaResolverTransitionTest,
	"FMCodex.CoreRules.FormulaResolver.Transition",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFormulaResolverTransitionTest::RunTest(const FString& Parameters)
{
	FFormulaResolverInput AttackerWinInput = FormulaResolverTests::MakeInput(EFormulaType::Transition);
	AttackerWinInput.Attacker.BaseValue = 5.0f;
	AttackerWinInput.Defender.BaseValue = 4.0f;

	const FFormulaResolutionResult AttackerWin = UFormulaResolver::ResolveFormula(AttackerWinInput);
	TestEqual(TEXT("Transition attacker winner"), AttackerWin.Winner, EFormulaWinner::Attacker);
	TestEqual(
		TEXT("Transition normal win reason"),
		AttackerWin.WinReason,
		EFormulaWinReason::HigherFinalValue);
	TestFalse(TEXT("Transition attacker win is not a goal"), AttackerWin.bIsGoal);
	TestFalse(TEXT("Transition attacker win continues resolution"), AttackerWin.bAttackEnded);
	TestTrue(TEXT("Transition attacker win sets continue resolution"), AttackerWin.bContinueResolution);
	TestTrue(
		TEXT("Transition log records higher final value reason"),
		AttackerWin.MatchLogEntry.FormulaResult.Contains(TEXT("WinReason=HigherFinalValue")));
	TestTrue(
		TEXT("Transition log records continue resolution"),
		AttackerWin.MatchLogEntry.FormulaResult.Contains(TEXT("ContinueResolution=true")));

	FFormulaResolverInput DefenderWinInput = FormulaResolverTests::MakeInput(EFormulaType::Transition);
	DefenderWinInput.Attacker.BaseValue = 3.0f;
	DefenderWinInput.Defender.BaseValue = 4.0f;

	const FFormulaResolutionResult DefenderWin = UFormulaResolver::ResolveFormula(DefenderWinInput);
	TestEqual(TEXT("Transition defender winner"), DefenderWin.Winner, EFormulaWinner::Defender);
	TestFalse(TEXT("Transition defender win is not a goal"), DefenderWin.bIsGoal);
	TestTrue(TEXT("Transition defender win ends attack"), DefenderWin.bAttackEnded);
	TestFalse(TEXT("Transition defender win does not continue"), DefenderWin.bContinueResolution);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFormulaResolverFinishingTest,
	"FMCodex.CoreRules.FormulaResolver.Finishing",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFormulaResolverFinishingTest::RunTest(const FString& Parameters)
{
	FFormulaResolverInput AttackerWinInput = FormulaResolverTests::MakeInput(EFormulaType::Finishing);
	AttackerWinInput.Attacker.BaseValue = 7.0f;
	AttackerWinInput.Defender.BaseValue = 6.0f;

	const FFormulaResolutionResult AttackerWin = UFormulaResolver::ResolveFormula(AttackerWinInput);
	TestEqual(TEXT("Finishing attacker winner"), AttackerWin.Winner, EFormulaWinner::Attacker);
	TestEqual(
		TEXT("Finishing normal win reason"),
		AttackerWin.WinReason,
		EFormulaWinReason::HigherFinalValue);
	TestTrue(TEXT("Finishing attacker win is a goal"), AttackerWin.bIsGoal);
	TestTrue(TEXT("Goal ends the current attack"), AttackerWin.bAttackEnded);
	TestFalse(TEXT("Finishing result does not continue resolution"), AttackerWin.bContinueResolution);

	FFormulaResolverInput DefenderWinInput = FormulaResolverTests::MakeInput(EFormulaType::Finishing);
	DefenderWinInput.Attacker.BaseValue = 5.0f;
	DefenderWinInput.Defender.BaseValue = 6.0f;

	const FFormulaResolutionResult DefenderWin = UFormulaResolver::ResolveFormula(DefenderWinInput);
	TestEqual(TEXT("Finishing defender winner"), DefenderWin.Winner, EFormulaWinner::Defender);
	TestFalse(TEXT("Finishing defender win is not a goal"), DefenderWin.bIsGoal);
	TestTrue(TEXT("Finishing defender win ends attack"), DefenderWin.bAttackEnded);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFormulaResolverTieBreakTest,
	"FMCodex.CoreRules.FormulaResolver.TieBreak",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFormulaResolverTieBreakTest::RunTest(const FString& Parameters)
{
	FFormulaResolverInput SinglePlayerInput = FormulaResolverTests::MakeInput(EFormulaType::Finishing);
	SinglePlayerInput.Attacker.BaseValue = 5.0f;
	SinglePlayerInput.Defender.BaseValue = 5.0f;
	SinglePlayerInput.Attacker.ParticipatingStamina = { 6 };
	SinglePlayerInput.Defender.ParticipatingStamina = { 4 };

	const FFormulaResolutionResult SinglePlayerResult =
		UFormulaResolver::ResolveFormula(SinglePlayerInput);
	TestEqual(
		TEXT("Higher single-player stamina wins a tie"),
		SinglePlayerResult.Winner,
		EFormulaWinner::Attacker);
	TestEqual(
		TEXT("Single-player stamina tie-break reason"),
		SinglePlayerResult.WinReason,
		EFormulaWinReason::StaminaTieBreaker);

	FFormulaResolverInput EqualStaminaInput = SinglePlayerInput;
	EqualStaminaInput.Attacker.ParticipatingStamina = { 4 };
	EqualStaminaInput.Defender.ParticipatingStamina = { 4 };
	const FFormulaResolutionResult EqualStaminaResult =
		UFormulaResolver::ResolveFormula(EqualStaminaInput);
	TestEqual(
		TEXT("Defender wins when stamina is equal"),
		EqualStaminaResult.Winner,
		EFormulaWinner::Defender);
	TestEqual(
		TEXT("Equal stamina defender win reason"),
		EqualStaminaResult.WinReason,
		EFormulaWinReason::DefenderWinsEqualStamina);

	FFormulaResolverInput GoalkeeperInput = SinglePlayerInput;
	GoalkeeperInput.bGoalkeeperParticipated = true;
	GoalkeeperInput.Attacker.ParticipatingStamina = { 6 };
	GoalkeeperInput.Defender.ParticipatingStamina = { 1 };
	const FFormulaResolutionResult GoalkeeperResult =
		UFormulaResolver::ResolveFormula(GoalkeeperInput);
	TestEqual(
		TEXT("Defender wins a tied formula when goalkeeper participates"),
		GoalkeeperResult.Winner,
		EFormulaWinner::Defender);
	TestEqual(
		TEXT("Goalkeeper tie win reason"),
		GoalkeeperResult.WinReason,
		EFormulaWinReason::DefenderWinsGoalkeeperTie);

	FFormulaResolverInput MultiPlayerInput = SinglePlayerInput;
	MultiPlayerInput.Attacker.ParticipatingStamina = { 2, 6 };
	MultiPlayerInput.Defender.ParticipatingStamina = { 4, 3 };
	const FFormulaResolutionResult MultiPlayerResult =
		UFormulaResolver::ResolveFormula(MultiPlayerInput);
	TestEqual(
		TEXT("Higher summed stamina wins a multi-player tie"),
		MultiPlayerResult.Winner,
		EFormulaWinner::Attacker);
	TestEqual(
		TEXT("Multi-player stamina tie-break reason"),
		MultiPlayerResult.WinReason,
		EFormulaWinReason::StaminaTieBreaker);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFormulaResolverFastSuppressionTest,
	"FMCodex.CoreRules.FormulaResolver.FastSuppression",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFormulaResolverFastSuppressionTest::RunTest(const FString& Parameters)
{
	FFormulaResolverInput Input = FormulaResolverTests::MakeInput(EFormulaType::Finishing);
	Input.Attacker.BaseValue = 0.0f;
	Input.Attacker.ComparePoint = 6;
	Input.Attacker.bComparePointWasRolledOnD6 = true;
	Input.Defender.BaseValue = 20.0f;
	Input.Defender.ComparePoint = 2;
	Input.Defender.bComparePointWasRolledOnD6 = true;

	const FFormulaResolutionResult Result = UFormulaResolver::ResolveFormula(Input);
	TestTrue(TEXT("Six against two triggers fast suppression"), Result.bFastSuppressionTriggered);
	TestEqual(TEXT("Side that rolled six wins directly"), Result.Winner, EFormulaWinner::Attacker);
	TestEqual(
		TEXT("Fast suppression win reason"),
		Result.WinReason,
		EFormulaWinReason::FastSuppression);
	TestEqual(TEXT("Attacker die is logged first"), Result.MatchLogEntry.DiceResults[0], 6);
	TestEqual(TEXT("Defender die is logged second"), Result.MatchLogEntry.DiceResults[1], 2);
	TestEqual(TEXT("Attacker log order"), Result.MatchLogEntry.DiceOrder[0], FName(TEXT("PlayerA")));
	TestEqual(TEXT("Defender log order"), Result.MatchLogEntry.DiceOrder[1], FName(TEXT("PlayerB")));
	TestTrue(
		TEXT("Structured result text records fast suppression"),
		Result.MatchLogEntry.FormulaResult.Contains(TEXT("FastSuppression=true")));
	TestTrue(
		TEXT("Formula log records win reason"),
		Result.MatchLogEntry.FormulaResult.Contains(TEXT("WinReason=FastSuppression")));
	TestTrue(
		TEXT("Formula log records continue resolution"),
		Result.MatchLogEntry.FormulaResult.Contains(TEXT("ContinueResolution=false")));

	Input.Defender.bComparePointWasRolledOnD6 = false;
	const FFormulaResolutionResult OneSidedRollResult = UFormulaResolver::ResolveFormula(Input);
	TestFalse(
		TEXT("One-sided D6 input does not trigger fast suppression"),
		OneSidedRollResult.bFastSuppressionTriggered);
	TestEqual(
		TEXT("Normal final-value comparison is used without fast suppression"),
		OneSidedRollResult.Winner,
		EFormulaWinner::Defender);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFormulaResolverDecimalTest,
	"FMCodex.CoreRules.FormulaResolver.DecimalRules",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFormulaResolverDecimalTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("Division by two keeps one decimal"), UFormulaResolver::DivideByTwo(5.0f), 2.5f);
	TestEqual(TEXT("Average keeps one decimal"), UFormulaResolver::Average(5.0f, 4.0f), 4.5f);
	TestEqual(
		TEXT("Goalkeeper half keeps one decimal"),
		UFormulaResolver::CalculateGoalkeeperHalf(5.0f),
		2.5f);

	FFormulaResolverInput Input = FormulaResolverTests::MakeInput(EFormulaType::Finishing);
	Input.Attacker.BaseValue = UFormulaResolver::Average(5.0f, 4.0f);
	Input.Attacker.ComparePoint = 3;
	Input.Defender.BaseValue = UFormulaResolver::CalculateGoalkeeperHalf(5.0f);
	Input.Defender.Modifier = 2.0f;
	Input.Defender.ComparePoint = 3;

	const FFormulaResolutionResult Result = UFormulaResolver::ResolveFormula(Input);
	TestEqual(TEXT("Attacker final value is kept to one decimal"), Result.AttackerFinalValue, 7.5f);
	TestEqual(TEXT("Defender final value is kept to one decimal"), Result.DefenderFinalValue, 7.5f);
	TestEqual(
		TEXT("One-decimal values are compared directly before tie-break"),
		Result.Winner,
		EFormulaWinner::Defender);
	return true;
}

#endif
