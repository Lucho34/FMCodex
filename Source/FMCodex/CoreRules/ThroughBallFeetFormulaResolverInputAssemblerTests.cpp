#if WITH_DEV_AUTOMATION_TESTS

#include "ThroughBallFeetFormulaResolverInputAssembler.h"

#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

#include <limits>

namespace ThroughBallFeetFormulaResolverInputAssemblerTests
{
	const FName CarrierId(TEXT("ThroughBall.Carrier"));
	const FName RunnerId(TEXT("ThroughBall.Runner"));
	const FName MarkerId(TEXT("ThroughBall.Marker"));
	const FName HelperId(TEXT("ThroughBall.Helper"));
	const FName GoalkeeperId(TEXT("ThroughBall.Goalkeeper"));
	const FName AttackingOwnerId(TEXT("Player.Attacking"));
	const FName DefendingOwnerId(TEXT("Player.Defending"));
	const FGuid LogId(221, 222, 223, 224);

	FThroughBallFeetFormulaPlan MakePlan(
		const bool bHasHelper = false,
		const bool bHasGoalkeeper = false)
	{
		FThroughBallFeetFormulaPlan Plan;
		Plan.FormulaType = EFormulaType::Finishing;
		Plan.CarrierId = CarrierId;
		Plan.CarrierPassing = 4;
		Plan.CarrierStamina = 5;
		Plan.RunnerId = RunnerId;
		Plan.RunnerOffBall = 5;
		Plan.RunnerStamina = 4;
		Plan.AttackD6 = 3;
		Plan.AttackBaseValue = 4.5f;
		Plan.AttackExternalModifier = 0.0f;
		Plan.AttackParticipatingStamina = {5, 4};

		Plan.MarkerId = MarkerId;
		Plan.MarkerTackling = 5;
		Plan.MarkerStamina = 3;
		Plan.bHasHelper = bHasHelper;
		if (bHasHelper)
		{
			Plan.HelperId = HelperId;
			Plan.HelperMarking = 4;
			Plan.HelperStamina = 2;
		}

		Plan.bHasActiveGoalkeeper = bHasGoalkeeper;
		if (bHasGoalkeeper)
		{
			Plan.ActiveGoalkeeperId = GoalkeeperId;
			Plan.GoalkeeperOneOnOne = 5;
			Plan.GoalkeeperStamina = 6;
		}

		Plan.DefenseD6 = 4;
		Plan.DefenseBaseValue = bHasHelper ? 4.5f : 2.5f;
		Plan.DefenseExternalModifier = bHasGoalkeeper ? 4.5f : 2.0f;
		Plan.DefenseParticipatingStamina.Add(Plan.MarkerStamina);
		if (bHasHelper)
		{
			Plan.DefenseParticipatingStamina.Add(Plan.HelperStamina);
		}
		if (bHasGoalkeeper)
		{
			Plan.DefenseParticipatingStamina.Add(Plan.GoalkeeperStamina);
		}

		Plan.LogId = LogId;
		Plan.TurnIndex = 7;
		Plan.AttackingOwnerId = AttackingOwnerId;
		Plan.DefendingOwnerId = DefendingOwnerId;
		Plan.InvolvedCardIds = {CarrierId, RunnerId, MarkerId};
		if (bHasHelper)
		{
			Plan.InvolvedCardIds.Add(HelperId);
		}
		if (bHasGoalkeeper)
		{
			Plan.InvolvedCardIds.Add(GoalkeeperId);
		}

		Plan.GoalScorerCardId = RunnerId;
		Plan.bAttackVictoryIsGoal = true;
		Plan.bDefenderVictoryIsMiss = true;
		Plan.bAttackEndsAfterResolution = true;
		Plan.bContinueResolution = false;
		return Plan;
	}

	FThroughBallFeetFormulaResolverInputAssemblyInput MakeInput(
		const bool bHasHelper = false,
		const bool bHasGoalkeeper = false)
	{
		FThroughBallFeetFormulaResolverInputAssemblyInput Input;
		Input.FormulaPlan = MakePlan(bHasHelper, bHasGoalkeeper);
		return Input;
	}

	bool ArePlansEqual(
		const FThroughBallFeetFormulaPlan& Left,
		const FThroughBallFeetFormulaPlan& Right)
	{
		return Left.FormulaType == Right.FormulaType
			&& Left.CarrierId == Right.CarrierId
			&& Left.CarrierPassing == Right.CarrierPassing
			&& Left.CarrierStamina == Right.CarrierStamina
			&& Left.RunnerId == Right.RunnerId
			&& Left.RunnerOffBall == Right.RunnerOffBall
			&& Left.RunnerStamina == Right.RunnerStamina
			&& Left.AttackD6 == Right.AttackD6
			&& Left.AttackBaseValue == Right.AttackBaseValue
			&& Left.AttackExternalModifier == Right.AttackExternalModifier
			&& Left.AttackParticipatingStamina
				== Right.AttackParticipatingStamina
			&& Left.MarkerId == Right.MarkerId
			&& Left.MarkerTackling == Right.MarkerTackling
			&& Left.MarkerStamina == Right.MarkerStamina
			&& Left.bHasHelper == Right.bHasHelper
			&& Left.HelperId == Right.HelperId
			&& Left.HelperMarking == Right.HelperMarking
			&& Left.HelperStamina == Right.HelperStamina
			&& Left.bHasActiveGoalkeeper == Right.bHasActiveGoalkeeper
			&& Left.ActiveGoalkeeperId == Right.ActiveGoalkeeperId
			&& Left.GoalkeeperOneOnOne == Right.GoalkeeperOneOnOne
			&& Left.GoalkeeperStamina == Right.GoalkeeperStamina
			&& Left.DefenseD6 == Right.DefenseD6
			&& Left.DefenseBaseValue == Right.DefenseBaseValue
			&& Left.DefenseExternalModifier == Right.DefenseExternalModifier
			&& Left.DefenseParticipatingStamina
				== Right.DefenseParticipatingStamina
			&& Left.LogId == Right.LogId
			&& Left.TurnIndex == Right.TurnIndex
			&& Left.AttackingOwnerId == Right.AttackingOwnerId
			&& Left.DefendingOwnerId == Right.DefendingOwnerId
			&& Left.InvolvedCardIds == Right.InvolvedCardIds
			&& Left.GoalScorerCardId == Right.GoalScorerCardId
			&& Left.bAttackVictoryIsGoal == Right.bAttackVictoryIsGoal
			&& Left.bDefenderVictoryIsMiss == Right.bDefenderVictoryIsMiss
			&& Left.bAttackEndsAfterResolution
				== Right.bAttackEndsAfterResolution
			&& Left.bContinueResolution == Right.bContinueResolution;
	}

	bool AreSideInputsEqual(
		const FFormulaSideInput& Left,
		const FFormulaSideInput& Right)
	{
		return Left.BaseValue == Right.BaseValue
			&& Left.Modifier == Right.Modifier
			&& Left.ComparePoint == Right.ComparePoint
			&& Left.bComparePointWasRolledOnD6
				== Right.bComparePointWasRolledOnD6
			&& Left.ParticipatingStamina == Right.ParticipatingStamina;
	}

	bool AreResolverInputsEqual(
		const FFormulaResolverInput& Left,
		const FFormulaResolverInput& Right)
	{
		return Left.FormulaType == Right.FormulaType
			&& AreSideInputsEqual(Left.Attacker, Right.Attacker)
			&& AreSideInputsEqual(Left.Defender, Right.Defender)
			&& Left.bGoalkeeperParticipated
				== Right.bGoalkeeperParticipated
			&& Left.LogId == Right.LogId
			&& Left.TurnIndex == Right.TurnIndex
			&& Left.AttackerPlayerId == Right.AttackerPlayerId
			&& Left.DefenderPlayerId == Right.DefenderPlayerId
			&& Left.InvolvedCardIds == Right.InvolvedCardIds;
	}

	bool IsDefaultResolverInput(const FFormulaResolverInput& Input)
	{
		return AreResolverInputsEqual(Input, FFormulaResolverInput{});
	}

	FThroughBallFeetFormulaResolverInputAssemblyResult AssembleSuccess(
		FAutomationTestBase& Test,
		const FThroughBallFeetFormulaResolverInputAssemblyInput& Input)
	{
		const FThroughBallFeetFormulaResolverInputAssemblyResult Result =
			FThroughBallFeetFormulaResolverInputAssembler::Assemble(Input);
		Test.TestTrue(TEXT("Assembly succeeds"), Result.bSuccess);
		Test.TestEqual(
			TEXT("Success has no error"),
			Result.ErrorCode,
			EThroughBallFeetFormulaResolverInputAssemblyErrorCode::None);
		Test.TestTrue(
			TEXT("Success message is empty"),
			Result.ErrorMessage.IsEmpty());
		Test.TestTrue(
			TEXT("Success invalid field is empty"),
			Result.InvalidField.IsNone());
		Test.TestTrue(TEXT("Success has Resolver Input"), Result.bHasResolverInput);
		return Result;
	}

	FThroughBallFeetFormulaResolverInputAssemblyResult AssembleFailure(
		FAutomationTestBase& Test,
		const FThroughBallFeetFormulaResolverInputAssemblyInput& Input,
		const EThroughBallFeetFormulaResolverInputAssemblyErrorCode ExpectedError,
		const FName ExpectedField)
	{
		const FThroughBallFeetFormulaResolverInputAssemblyResult Result =
			FThroughBallFeetFormulaResolverInputAssembler::Assemble(Input);
		Test.TestFalse(TEXT("Assembly fails"), Result.bSuccess);
		Test.TestFalse(
			TEXT("Failure has no Resolver Input"),
			Result.bHasResolverInput);
		Test.TestEqual(TEXT("Expected error"), Result.ErrorCode, ExpectedError);
		Test.TestEqual(
			TEXT("Expected invalid field"),
			Result.InvalidField,
			ExpectedField);
		Test.TestFalse(
			TEXT("Failure message is non-empty"),
			Result.ErrorMessage.IsEmpty());
		Test.TestTrue(
			TEXT("Failure Resolver Input is default"),
			IsDefaultResolverInput(Result.ResolverInput));
		return Result;
	}

	bool LoadProductionSource(
		FAutomationTestBase& Test,
		FString& OutHeader,
		FString& OutImplementation)
	{
		const FString Root = FPaths::Combine(
			FPaths::ProjectDir(),
			TEXT("Source/FMCodex/CoreRules"));
		const bool bHeaderLoaded = FFileHelper::LoadFileToString(
			OutHeader,
			*FPaths::Combine(
				Root,
				TEXT("ThroughBallFeetFormulaResolverInputAssembler.h")));
		const bool bImplementationLoaded = FFileHelper::LoadFileToString(
			OutImplementation,
			*FPaths::Combine(
				Root,
				TEXT("ThroughBallFeetFormulaResolverInputAssembler.cpp")));
		Test.TestTrue(TEXT("Assembler header loaded"), bHeaderLoaded);
		Test.TestTrue(
			TEXT("Assembler implementation loaded"),
			bImplementationLoaded);
		return bHeaderLoaded && bImplementationLoaded;
	}

	bool RunCase(FAutomationTestBase& Test, const int32 Case)
	{
		FThroughBallFeetFormulaResolverInputAssemblyInput Input;
		FThroughBallFeetFormulaResolverInputAssemblyResult Result;
		switch (Case)
		{
		case 1:
			AssembleSuccess(Test, MakeInput(false, false));
			break;
		case 2:
			AssembleSuccess(Test, MakeInput(true, false));
			break;
		case 3:
			AssembleSuccess(Test, MakeInput(false, true));
			break;
		case 4:
			AssembleSuccess(Test, MakeInput(true, true));
			break;
		case 5:
			Result = AssembleSuccess(Test, MakeInput());
			Test.TestEqual(TEXT("Attack base mapped"), Result.ResolverInput.Attacker.BaseValue, 4.5f);
			Test.TestEqual(TEXT("Attack modifier mapped"), Result.ResolverInput.Attacker.Modifier, 0.0f);
			Test.TestEqual(TEXT("Attack D6 mapped"), Result.ResolverInput.Attacker.ComparePoint, 3);
			break;
		case 6:
			Result = AssembleSuccess(Test, MakeInput(true, true));
			Test.TestEqual(TEXT("Defense base mapped"), Result.ResolverInput.Defender.BaseValue, 4.5f);
			Test.TestEqual(TEXT("Defense modifier mapped"), Result.ResolverInput.Defender.Modifier, 4.5f);
			Test.TestEqual(TEXT("Defense D6 mapped"), Result.ResolverInput.Defender.ComparePoint, 4);
			break;
		case 7:
			Result = AssembleSuccess(Test, MakeInput(true, true));
			Test.TestEqual(TEXT("Attack stamina preserved"), Result.ResolverInput.Attacker.ParticipatingStamina, TArray<int32>({5, 4}));
			Test.TestEqual(TEXT("Defense stamina preserved"), Result.ResolverInput.Defender.ParticipatingStamina, TArray<int32>({3, 2, 6}));
			break;
		case 8:
			Test.TestFalse(TEXT("Absent GK maps false"), AssembleSuccess(Test, MakeInput()).ResolverInput.bGoalkeeperParticipated);
			Test.TestTrue(TEXT("Present GK maps true"), AssembleSuccess(Test, MakeInput(false, true)).ResolverInput.bGoalkeeperParticipated);
			break;
		case 9:
			Result = AssembleSuccess(Test, MakeInput());
			Test.TestEqual(TEXT("Log mapped"), Result.ResolverInput.LogId, LogId);
			Test.TestEqual(TEXT("Turn mapped"), Result.ResolverInput.TurnIndex, 7);
			Test.TestEqual(TEXT("Attacker owner mapped"), Result.ResolverInput.AttackerPlayerId, AttackingOwnerId);
			Test.TestEqual(TEXT("Defender owner mapped"), Result.ResolverInput.DefenderPlayerId, DefendingOwnerId);
			break;
		case 10:
			Result = AssembleSuccess(Test, MakeInput(true, true));
			Test.TestEqual(TEXT("Cards mapped"), Result.ResolverInput.InvolvedCardIds, TArray<FName>({CarrierId, RunnerId, MarkerId, HelperId, GoalkeeperId}));
			Test.TestTrue(TEXT("Attack D6 rolled flag set"), Result.ResolverInput.Attacker.bComparePointWasRolledOnD6);
			Test.TestTrue(TEXT("Defense D6 rolled flag set"), Result.ResolverInput.Defender.bComparePointWasRolledOnD6);
			break;
		case 11:
			Input = MakeInput(); Input.FormulaPlan.FormulaType = EFormulaType::Transition;
			AssembleFailure(Test, Input, EThroughBallFeetFormulaResolverInputAssemblyErrorCode::UnsupportedFormulaType, TEXT("FormulaType"));
			break;
		case 12:
			Input = MakeInput(); Input.FormulaPlan.CarrierId = NAME_None;
			AssembleFailure(Test, Input, EThroughBallFeetFormulaResolverInputAssemblyErrorCode::InvalidRequiredParticipantIdentity, TEXT("CarrierId"));
			break;
		case 13:
			Input = MakeInput(); Input.FormulaPlan.RunnerId = NAME_None;
			AssembleFailure(Test, Input, EThroughBallFeetFormulaResolverInputAssemblyErrorCode::InvalidRequiredParticipantIdentity, TEXT("RunnerId"));
			break;
		case 14:
			Input = MakeInput(); Input.FormulaPlan.MarkerId = NAME_None;
			AssembleFailure(Test, Input, EThroughBallFeetFormulaResolverInputAssemblyErrorCode::InvalidRequiredParticipantIdentity, TEXT("MarkerId"));
			break;
		case 15:
			Input = MakeInput(); Input.FormulaPlan.HelperId = HelperId;
			AssembleFailure(Test, Input, EThroughBallFeetFormulaResolverInputAssemblyErrorCode::InvalidOptionalParticipantState, TEXT("HelperId"));
			break;
		case 16:
			Input = MakeInput(); Input.FormulaPlan.HelperMarking = 1;
			AssembleFailure(Test, Input, EThroughBallFeetFormulaResolverInputAssemblyErrorCode::InvalidOptionalParticipantState, TEXT("HelperMarking"));
			Input = MakeInput(); Input.FormulaPlan.HelperStamina = 1;
			AssembleFailure(Test, Input, EThroughBallFeetFormulaResolverInputAssemblyErrorCode::InvalidOptionalParticipantState, TEXT("HelperStamina"));
			break;
		case 17:
			Input = MakeInput(true); Input.FormulaPlan.HelperId = NAME_None;
			AssembleFailure(Test, Input, EThroughBallFeetFormulaResolverInputAssemblyErrorCode::InvalidOptionalParticipantState, TEXT("HelperId"));
			break;
		case 18:
			Input = MakeInput(); Input.FormulaPlan.ActiveGoalkeeperId = GoalkeeperId;
			AssembleFailure(Test, Input, EThroughBallFeetFormulaResolverInputAssemblyErrorCode::InvalidOptionalParticipantState, TEXT("ActiveGoalkeeperId"));
			break;
		case 19:
			Input = MakeInput(); Input.FormulaPlan.GoalkeeperOneOnOne = 1;
			AssembleFailure(Test, Input, EThroughBallFeetFormulaResolverInputAssemblyErrorCode::InvalidOptionalParticipantState, TEXT("GoalkeeperOneOnOne"));
			Input = MakeInput(); Input.FormulaPlan.GoalkeeperStamina = 1;
			AssembleFailure(Test, Input, EThroughBallFeetFormulaResolverInputAssemblyErrorCode::InvalidOptionalParticipantState, TEXT("GoalkeeperStamina"));
			break;
		case 20:
			Input = MakeInput(false, true); Input.FormulaPlan.ActiveGoalkeeperId = NAME_None;
			AssembleFailure(Test, Input, EThroughBallFeetFormulaResolverInputAssemblyErrorCode::InvalidOptionalParticipantState, TEXT("ActiveGoalkeeperId"));
			break;
		case 21:
			Input = MakeInput(); Input.FormulaPlan.AttackBaseValue = std::numeric_limits<float>::quiet_NaN();
			AssembleFailure(Test, Input, EThroughBallFeetFormulaResolverInputAssemblyErrorCode::InvalidAttackFormulaData, TEXT("AttackBaseValue"));
			break;
		case 22:
			Input = MakeInput(); Input.FormulaPlan.AttackExternalModifier = std::numeric_limits<float>::infinity();
			AssembleFailure(Test, Input, EThroughBallFeetFormulaResolverInputAssemblyErrorCode::InvalidAttackFormulaData, TEXT("AttackExternalModifier"));
			break;
		case 23:
			Input = MakeInput(); Input.FormulaPlan.AttackD6 = 0;
			AssembleFailure(Test, Input, EThroughBallFeetFormulaResolverInputAssemblyErrorCode::InvalidAttackFormulaData, TEXT("AttackD6"));
			Input = MakeInput(); Input.FormulaPlan.AttackD6 = 7;
			AssembleFailure(Test, Input, EThroughBallFeetFormulaResolverInputAssemblyErrorCode::InvalidAttackFormulaData, TEXT("AttackD6"));
			break;
		case 24:
			Input = MakeInput(); Input.FormulaPlan.DefenseBaseValue = std::numeric_limits<float>::quiet_NaN();
			AssembleFailure(Test, Input, EThroughBallFeetFormulaResolverInputAssemblyErrorCode::InvalidDefenseFormulaData, TEXT("DefenseBaseValue"));
			break;
		case 25:
			Input = MakeInput(); Input.FormulaPlan.DefenseExternalModifier = std::numeric_limits<float>::infinity();
			AssembleFailure(Test, Input, EThroughBallFeetFormulaResolverInputAssemblyErrorCode::InvalidDefenseFormulaData, TEXT("DefenseExternalModifier"));
			break;
		case 26:
			Input = MakeInput(); Input.FormulaPlan.DefenseD6 = 0;
			AssembleFailure(Test, Input, EThroughBallFeetFormulaResolverInputAssemblyErrorCode::InvalidDefenseFormulaData, TEXT("DefenseD6"));
			Input = MakeInput(); Input.FormulaPlan.DefenseD6 = 7;
			AssembleFailure(Test, Input, EThroughBallFeetFormulaResolverInputAssemblyErrorCode::InvalidDefenseFormulaData, TEXT("DefenseD6"));
			break;
		case 27:
			Input = MakeInput(); Input.FormulaPlan.AttackParticipatingStamina = {5};
			AssembleFailure(Test, Input, EThroughBallFeetFormulaResolverInputAssemblyErrorCode::InvalidAttackParticipatingStamina, TEXT("AttackParticipatingStamina"));
			Input = MakeInput(); Input.FormulaPlan.AttackParticipatingStamina = {5, 4, 1};
			AssembleFailure(Test, Input, EThroughBallFeetFormulaResolverInputAssemblyErrorCode::InvalidAttackParticipatingStamina, TEXT("AttackParticipatingStamina"));
			break;
		case 28:
			Input = MakeInput(); Input.FormulaPlan.AttackParticipatingStamina = {4, 5};
			AssembleFailure(Test, Input, EThroughBallFeetFormulaResolverInputAssemblyErrorCode::InvalidAttackParticipatingStamina, TEXT("AttackParticipatingStamina"));
			Input = MakeInput(); Input.FormulaPlan.AttackParticipatingStamina = {5, 3};
			AssembleFailure(Test, Input, EThroughBallFeetFormulaResolverInputAssemblyErrorCode::InvalidAttackParticipatingStamina, TEXT("AttackParticipatingStamina"));
			break;
		case 29:
			Input = MakeInput(true, true); Input.FormulaPlan.DefenseParticipatingStamina = {3, 2};
			AssembleFailure(Test, Input, EThroughBallFeetFormulaResolverInputAssemblyErrorCode::InvalidDefenseParticipatingStamina, TEXT("DefenseParticipatingStamina"));
			Input = MakeInput(); Input.FormulaPlan.DefenseParticipatingStamina = {3, 0};
			AssembleFailure(Test, Input, EThroughBallFeetFormulaResolverInputAssemblyErrorCode::InvalidDefenseParticipatingStamina, TEXT("DefenseParticipatingStamina"));
			break;
		case 30:
			Input = MakeInput(true, true); Input.FormulaPlan.DefenseParticipatingStamina = {3, 6, 2};
			AssembleFailure(Test, Input, EThroughBallFeetFormulaResolverInputAssemblyErrorCode::InvalidDefenseParticipatingStamina, TEXT("DefenseParticipatingStamina"));
			Input = MakeInput(true); Input.FormulaPlan.DefenseParticipatingStamina = {3, 1};
			AssembleFailure(Test, Input, EThroughBallFeetFormulaResolverInputAssemblyErrorCode::InvalidDefenseParticipatingStamina, TEXT("DefenseParticipatingStamina"));
			break;
		case 31:
			Input = MakeInput(); Input.FormulaPlan.LogId.Invalidate();
			AssembleFailure(Test, Input, EThroughBallFeetFormulaResolverInputAssemblyErrorCode::InvalidLogContext, TEXT("LogId"));
			Input = MakeInput(); Input.FormulaPlan.TurnIndex = -1;
			AssembleFailure(Test, Input, EThroughBallFeetFormulaResolverInputAssemblyErrorCode::InvalidLogContext, TEXT("TurnIndex"));
			break;
		case 32:
			Input = MakeInput(); Input.FormulaPlan.AttackingOwnerId = NAME_None;
			AssembleFailure(Test, Input, EThroughBallFeetFormulaResolverInputAssemblyErrorCode::InvalidOwnerIdentity, TEXT("AttackingOwnerId"));
			Input = MakeInput(); Input.FormulaPlan.DefendingOwnerId = NAME_None;
			AssembleFailure(Test, Input, EThroughBallFeetFormulaResolverInputAssemblyErrorCode::InvalidOwnerIdentity, TEXT("DefendingOwnerId"));
			Input = MakeInput(); Input.FormulaPlan.DefendingOwnerId = AttackingOwnerId;
			AssembleFailure(Test, Input, EThroughBallFeetFormulaResolverInputAssemblyErrorCode::InvalidOwnerIdentity, TEXT("DefendingOwnerId"));
			break;
		case 33:
			Input = MakeInput(true, true); Input.FormulaPlan.InvolvedCardIds = {CarrierId, RunnerId, MarkerId, HelperId};
			AssembleFailure(Test, Input, EThroughBallFeetFormulaResolverInputAssemblyErrorCode::InvalidInvolvedCardIds, TEXT("InvolvedCardIds"));
			Input = MakeInput(); Input.FormulaPlan.InvolvedCardIds = {RunnerId, CarrierId, MarkerId};
			AssembleFailure(Test, Input, EThroughBallFeetFormulaResolverInputAssemblyErrorCode::InvalidInvolvedCardIds, TEXT("InvolvedCardIds"));
			Input = MakeInput(); Input.FormulaPlan.InvolvedCardIds.Add(HelperId);
			AssembleFailure(Test, Input, EThroughBallFeetFormulaResolverInputAssemblyErrorCode::InvalidInvolvedCardIds, TEXT("InvolvedCardIds"));
			break;
		case 34:
			Input = MakeInput(); Input.FormulaPlan.GoalScorerCardId = NAME_None;
			AssembleFailure(Test, Input, EThroughBallFeetFormulaResolverInputAssemblyErrorCode::InvalidGoalScorerIdentity, TEXT("GoalScorerCardId"));
			Input = MakeInput(); Input.FormulaPlan.GoalScorerCardId = CarrierId;
			AssembleFailure(Test, Input, EThroughBallFeetFormulaResolverInputAssemblyErrorCode::InvalidGoalScorerIdentity, TEXT("GoalScorerCardId"));
			break;
		case 35:
			Input = MakeInput(); Input.FormulaPlan.bAttackVictoryIsGoal = false;
			AssembleFailure(Test, Input, EThroughBallFeetFormulaResolverInputAssemblyErrorCode::InvalidTerminalMetadata, TEXT("bAttackVictoryIsGoal"));
			Input = MakeInput(); Input.FormulaPlan.bDefenderVictoryIsMiss = false;
			AssembleFailure(Test, Input, EThroughBallFeetFormulaResolverInputAssemblyErrorCode::InvalidTerminalMetadata, TEXT("bDefenderVictoryIsMiss"));
			Input = MakeInput(); Input.FormulaPlan.bAttackEndsAfterResolution = false;
			AssembleFailure(Test, Input, EThroughBallFeetFormulaResolverInputAssemblyErrorCode::InvalidTerminalMetadata, TEXT("bAttackEndsAfterResolution"));
			Input = MakeInput(); Input.FormulaPlan.bContinueResolution = true;
			AssembleFailure(Test, Input, EThroughBallFeetFormulaResolverInputAssemblyErrorCode::InvalidTerminalMetadata, TEXT("bContinueResolution"));
			break;
		case 36:
			Input = MakeInput();
			Input.FormulaPlan.FormulaType = EFormulaType::Transition;
			Input.FormulaPlan.CarrierId = NAME_None;
			Input.FormulaPlan.AttackD6 = 0;
			AssembleFailure(Test, Input, EThroughBallFeetFormulaResolverInputAssemblyErrorCode::UnsupportedFormulaType, TEXT("FormulaType"));
			Input = MakeInput();
			Input.FormulaPlan.CarrierId = NAME_None;
			Input.FormulaPlan.HelperId = HelperId;
			AssembleFailure(Test, Input, EThroughBallFeetFormulaResolverInputAssemblyErrorCode::InvalidRequiredParticipantIdentity, TEXT("CarrierId"));
			break;
		case 37:
			Input = MakeInput(); Input.FormulaPlan.AttackD6 = 0;
			Result = AssembleFailure(Test, Input, EThroughBallFeetFormulaResolverInputAssemblyErrorCode::InvalidAttackFormulaData, TEXT("AttackD6"));
			Test.TestTrue(TEXT("Every Resolver Input field stays default"), IsDefaultResolverInput(Result.ResolverInput));
			break;
		case 38:
			Input = MakeInput(true, true);
			{
				const FThroughBallFeetFormulaResolverInputAssemblyInput Before = Input;
				Result = AssembleSuccess(Test, Input);
				Test.TestTrue(TEXT("Caller Plan unchanged"), ArePlansEqual(Input.FormulaPlan, Before.FormulaPlan));
				Test.TestTrue(TEXT("Result copies Plan"), ArePlansEqual(Result.Input.FormulaPlan, Before.FormulaPlan));
			}
			break;
		case 39:
			Input = MakeInput(true, true);
			{
				const auto First = AssembleSuccess(Test, Input);
				const auto Second = AssembleSuccess(Test, Input);
				Test.TestEqual(TEXT("Deterministic success"), First.bSuccess, Second.bSuccess);
				Test.TestEqual(TEXT("Deterministic error"), First.ErrorCode, Second.ErrorCode);
				Test.TestTrue(TEXT("Deterministic Resolver Input"), AreResolverInputsEqual(First.ResolverInput, Second.ResolverInput));
			}
			break;
		case 40:
			{
				FString Header;
				FString Implementation;
				if (LoadProductionSource(Test, Header, Implementation))
				{
					Test.TestFalse(TEXT("Does not call FormulaResolver"), Implementation.Contains(TEXT("UFormulaResolver::ResolveFormula")));
					Test.TestFalse(TEXT("Does not call FormulaAttackFlow"), Implementation.Contains(TEXT("FormulaAttackFlow")));
					Test.TestFalse(TEXT("Does not call MatchPlay"), Implementation.Contains(TEXT("MatchPlay")));
				}
			}
			break;
		case 41:
			{
				FString Header;
				FString Implementation;
				if (LoadProductionSource(Test, Header, Implementation))
				{
					const FString Source = Header + Implementation;
					Test.TestFalse(TEXT("Does not use SingleCard Assembler"), Source.Contains(TEXT("SingleCardFormulaResolverInputAssembler")));
					Test.TestFalse(TEXT("Does not use SingleCard Input Assembly"), Source.Contains(TEXT("SingleCardFormulaInputAssembly")));
					Test.TestFalse(TEXT("Does not query snapshots"), Source.Contains(TEXT("PlayerCardRuleSnapshotQuery")));
					Test.TestFalse(TEXT("Does not execute Eligibility"), Source.Contains(TEXT("FThroughBallParticipantEligibilityQuery::Evaluate")));
					Test.TestFalse(TEXT("Does not add generic framework"), Source.Contains(TEXT("GenericFormulaAssembly")));
				}
			}
			break;
		default:
			Test.AddError(TEXT("Unknown Through Ball Feet Resolver Input Assembler test case."));
			break;
		}
		return !Test.HasAnyErrors();
	}
}

#define THROUGH_BALL_FEET_ASSEMBLER_TEST(Number, Name) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
		FThroughBallFeetAssemblerTest##Number, \
		"FMCodex.CoreRules.ThroughBallFeetFormulaResolverInputAssembler." Name, \
		EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter) \
	bool FThroughBallFeetAssemblerTest##Number::RunTest(const FString& Parameters) \
	{ \
		return ThroughBallFeetFormulaResolverInputAssemblerTests::RunCase(*this, Number); \
	}

THROUGH_BALL_FEET_ASSEMBLER_TEST(1, "01SuccessWithoutHelperOrGoalkeeper")
THROUGH_BALL_FEET_ASSEMBLER_TEST(2, "02SuccessWithHelperOnly")
THROUGH_BALL_FEET_ASSEMBLER_TEST(3, "03SuccessWithGoalkeeperOnly")
THROUGH_BALL_FEET_ASSEMBLER_TEST(4, "04SuccessWithHelperAndGoalkeeper")
THROUGH_BALL_FEET_ASSEMBLER_TEST(5, "05MapsAttackSide")
THROUGH_BALL_FEET_ASSEMBLER_TEST(6, "06MapsDefenseSide")
THROUGH_BALL_FEET_ASSEMBLER_TEST(7, "07PreservesBothStaminaArrays")
THROUGH_BALL_FEET_ASSEMBLER_TEST(8, "08MapsGoalkeeperParticipation")
THROUGH_BALL_FEET_ASSEMBLER_TEST(9, "09MapsLogTurnAndOwners")
THROUGH_BALL_FEET_ASSEMBLER_TEST(10, "10MapsCardsAndD6RolledFlags")
THROUGH_BALL_FEET_ASSEMBLER_TEST(11, "11RejectsUnsupportedFormulaType")
THROUGH_BALL_FEET_ASSEMBLER_TEST(12, "12RejectsMissingCarrierId")
THROUGH_BALL_FEET_ASSEMBLER_TEST(13, "13RejectsMissingRunnerId")
THROUGH_BALL_FEET_ASSEMBLER_TEST(14, "14RejectsMissingMarkerId")
THROUGH_BALL_FEET_ASSEMBLER_TEST(15, "15RejectsAbsentHelperIdentity")
THROUGH_BALL_FEET_ASSEMBLER_TEST(16, "16RejectsAbsentHelperValues")
THROUGH_BALL_FEET_ASSEMBLER_TEST(17, "17RejectsPresentHelperWithoutIdentity")
THROUGH_BALL_FEET_ASSEMBLER_TEST(18, "18RejectsAbsentGoalkeeperIdentity")
THROUGH_BALL_FEET_ASSEMBLER_TEST(19, "19RejectsAbsentGoalkeeperValues")
THROUGH_BALL_FEET_ASSEMBLER_TEST(20, "20RejectsPresentGoalkeeperWithoutIdentity")
THROUGH_BALL_FEET_ASSEMBLER_TEST(21, "21RejectsNonFiniteAttackBase")
THROUGH_BALL_FEET_ASSEMBLER_TEST(22, "22RejectsNonFiniteAttackModifier")
THROUGH_BALL_FEET_ASSEMBLER_TEST(23, "23RejectsInvalidAttackD6")
THROUGH_BALL_FEET_ASSEMBLER_TEST(24, "24RejectsNonFiniteDefenseBase")
THROUGH_BALL_FEET_ASSEMBLER_TEST(25, "25RejectsNonFiniteDefenseModifier")
THROUGH_BALL_FEET_ASSEMBLER_TEST(26, "26RejectsInvalidDefenseD6")
THROUGH_BALL_FEET_ASSEMBLER_TEST(27, "27RejectsAttackStaminaCount")
THROUGH_BALL_FEET_ASSEMBLER_TEST(28, "28RejectsAttackStaminaOrderOrValue")
THROUGH_BALL_FEET_ASSEMBLER_TEST(29, "29RejectsDefenseStaminaCount")
THROUGH_BALL_FEET_ASSEMBLER_TEST(30, "30RejectsDefenseStaminaOrderOrValue")
THROUGH_BALL_FEET_ASSEMBLER_TEST(31, "31RejectsInvalidLogContext")
THROUGH_BALL_FEET_ASSEMBLER_TEST(32, "32RejectsInvalidOwnerIdentity")
THROUGH_BALL_FEET_ASSEMBLER_TEST(33, "33RejectsInvalidInvolvedCardIds")
THROUGH_BALL_FEET_ASSEMBLER_TEST(34, "34RejectsInvalidGoalScorer")
THROUGH_BALL_FEET_ASSEMBLER_TEST(35, "35RejectsInvalidTerminalMetadata")
THROUGH_BALL_FEET_ASSEMBLER_TEST(36, "36UsesFrozenErrorPriority")
THROUGH_BALL_FEET_ASSEMBLER_TEST(37, "37FailureKeepsDefaultResolverInput")
THROUGH_BALL_FEET_ASSEMBLER_TEST(38, "38PreservesInput")
THROUGH_BALL_FEET_ASSEMBLER_TEST(39, "39IsDeterministic")
THROUGH_BALL_FEET_ASSEMBLER_TEST(40, "40DoesNotCallResolverOrFlows")
THROUGH_BALL_FEET_ASSEMBLER_TEST(41, "41UsesNoSingleCardOrGenericFramework")

#undef THROUGH_BALL_FEET_ASSEMBLER_TEST

#endif
