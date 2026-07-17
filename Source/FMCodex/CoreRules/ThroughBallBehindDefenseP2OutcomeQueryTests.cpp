#if WITH_DEV_AUTOMATION_TESTS

#include "ThroughBallBehindDefenseP2OutcomeQuery.h"

#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace ThroughBallBehindDefenseP2OutcomeQueryTests
{
	const FName SkillId(TEXT("Skill.ThroughBall.BehindDefenseP2"));
	const FName CarrierId(TEXT("ThroughBall.P2.Carrier"));
	const FName RunnerId(TEXT("ThroughBall.P2.Runner"));
	const FName MarkerId(TEXT("ThroughBall.P2.Marker"));
	const FName AttackingOwnerId(TEXT("Player.P2.Attacking"));
	const FName DefendingOwnerId(TEXT("Player.P2.Defending"));
	const FGuid LogId(751, 752, 753, 754);

	using EDecision = EThroughBallBehindDefenseP2OutcomeDecision;
	using EError = EThroughBallBehindDefenseP2OutcomeQueryErrorCode;

	FPlayerCardRuleSnapshot MakeOutfield(
		const FName CardId,
		const EPlayerPositionType Position,
		const int32 Passing,
		const int32 Speed,
		const int32 Marking,
		const int32 Stamina)
	{
		FPlayerCardRuleSnapshot Snapshot;
		Snapshot.CardId = CardId;
		Snapshot.PositionTypes = {Position};
		Snapshot.Attributes.Passing = Passing;
		Snapshot.Attributes.Speed = Speed;
		Snapshot.Attributes.Marking = Marking;
		Snapshot.Attributes.Stamina = Stamina;
		return Snapshot;
	}

	FSkillRuleSnapshotSet MakeSkillRules()
	{
		FSkillRuleSnapshot Rule;
		Rule.SkillId = SkillId;
		Rule.SkillType = ESkillRuleType::ThroughBall;
		Rule.MinTriggerActionPoint = 2;
		Rule.MaxTriggerActionPoint = 8;

		FSkillRuleSnapshotSet Rules;
		Rules.SkillRules = {Rule};
		return Rules;
	}

	FThroughBallParticipantEligibilityQueryResult MakeEligibilityResult()
	{
		FThroughBallParticipantEligibilityQueryInput Input;
		Input.SelectedSkillId = SkillId;
		Input.CurrentActionPoint = 4;
		Input.AttackingOwnerId = AttackingOwnerId;
		Input.DefendingOwnerId = DefendingOwnerId;
		Input.CarrierSnapshot = MakeOutfield(
			CarrierId, EPlayerPositionType::Midfield, 6, 2, 1, 6);
		Input.CarrierSnapshot.SkillIds = {SkillId};
		Input.RunnerSnapshot = MakeOutfield(
			RunnerId, EPlayerPositionType::Attack, 1, 6, 1, 5);
		Input.MarkerSnapshot = MakeOutfield(
			MarkerId, EPlayerPositionType::Defense, 1, 1, 2, 4);
		Input.bIsRunnerInAttackingForwardArea = true;
		return FThroughBallParticipantEligibilityQuery::Evaluate(
			MakeSkillRules(),
			Input);
	}

	FThroughBallBehindDefenseP1FormulaResolutionExecutionResult
	MakeP1ExecutionResult(const int32 P1DefenseD6 = 4)
	{
		FThroughBallBehindDefenseP1PlanQueryInput PlanInput;
		PlanInput.ParticipantEligibilityResult = MakeEligibilityResult();
		PlanInput.SelectedBranch = EThroughBallSelectedBranch::BehindDefense;
		PlanInput.bHasAttackD6 = true;
		PlanInput.AttackD6 = 6;
		PlanInput.bHasDefenseD6 = true;
		PlanInput.DefenseD6 = P1DefenseD6;
		PlanInput.LogId = LogId;
		PlanInput.TurnIndex = 11;

		FThroughBallBehindDefenseP1FormulaResolverInputAssemblyInput
			AssemblyInput;
		AssemblyInput.PlanQueryResult =
			FThroughBallBehindDefenseP1PlanQuery::Evaluate(PlanInput);

		FThroughBallBehindDefenseP1FormulaResolutionExecutionInput
			ExecutionInput;
		ExecutionInput.ResolverInputAssemblyResult =
			FThroughBallBehindDefenseP1FormulaResolverInputAssembler::Assemble(
				AssemblyInput);
		return FThroughBallBehindDefenseP1FormulaResolutionExecutor::Execute(
			ExecutionInput);
	}

	FThroughBallBehindDefenseP2OutcomeQueryInput MakeInput(
		const int32 P2DefenseD6 = 1,
		const int32 P1DefenseD6 = 4)
	{
		FThroughBallBehindDefenseP2OutcomeQueryInput Input;
		Input.P1ExecutionResult = MakeP1ExecutionResult(P1DefenseD6);
		Input.bHasP2DefenseD6 = true;
		Input.P2DefenseD6 = P2DefenseD6;
		return Input;
	}

	void ExpectFailureDefaults(
		FAutomationTestBase& Test,
		const FThroughBallBehindDefenseP2OutcomeQueryResult& Result,
		const EError ErrorCode,
		const FName InvalidField)
	{
		Test.TestFalse(TEXT("Failure is not successful"), Result.bSuccess);
		Test.TestEqual(TEXT("Failure error"), Result.ErrorCode, ErrorCode);
		Test.TestFalse(TEXT("Failure message is populated"), Result.ErrorMessage.IsEmpty());
		Test.TestEqual(TEXT("Failure invalid field"), Result.InvalidField, InvalidField);
		Test.TestEqual(TEXT("Failure decision is None"), Result.Decision, EDecision::None);
		Test.TestFalse(TEXT("Failure does not end attack"), Result.bAttackEnded);
		Test.TestFalse(TEXT("Failure does not continue"), Result.bContinueResolution);
		Test.TestFalse(TEXT("Failure does not require One-on-One"), Result.bRequiresOneOnOne);
		Test.TestTrue(TEXT("Failure RunnerId is default"), Result.RunnerId.IsNone());
	}

	FThroughBallBehindDefenseP2OutcomeQueryResult ExpectFailure(
		FAutomationTestBase& Test,
		const FThroughBallBehindDefenseP2OutcomeQueryInput& Input,
		const EError ErrorCode,
		const FName InvalidField)
	{
		const FThroughBallBehindDefenseP2OutcomeQueryResult Result =
			FThroughBallBehindDefenseP2OutcomeQuery::Evaluate(Input);
		ExpectFailureDefaults(Test, Result, ErrorCode, InvalidField);
		return Result;
	}

	FThroughBallBehindDefenseP2OutcomeQueryResult ExpectSuccess(
		FAutomationTestBase& Test,
		const FThroughBallBehindDefenseP2OutcomeQueryInput& Input)
	{
		const FThroughBallBehindDefenseP2OutcomeQueryResult Result =
			FThroughBallBehindDefenseP2OutcomeQuery::Evaluate(Input);
		Test.TestTrue(TEXT("P2 Query succeeds"), Result.bSuccess);
		Test.TestEqual(TEXT("Success error is None"), Result.ErrorCode, EError::None);
		Test.TestTrue(TEXT("Success message is empty"), Result.ErrorMessage.IsEmpty());
		Test.TestTrue(TEXT("Success InvalidField is None"), Result.InvalidField.IsNone());
		return Result;
	}

	void ExpectOneOnOne(
		FAutomationTestBase& Test,
		const FThroughBallBehindDefenseP2OutcomeQueryResult& Result)
	{
		Test.TestEqual(TEXT("One-on-One decision"), Result.Decision, EDecision::OneOnOneRequired);
		Test.TestFalse(TEXT("One-on-One does not end attack"), Result.bAttackEnded);
		Test.TestTrue(TEXT("One-on-One continues resolution"), Result.bContinueResolution);
		Test.TestTrue(TEXT("One-on-One is required"), Result.bRequiresOneOnOne);
		Test.TestEqual(TEXT("One-on-One preserves RunnerId"), Result.RunnerId, RunnerId);
	}

	void ExpectOffside(
		FAutomationTestBase& Test,
		const FThroughBallBehindDefenseP2OutcomeQueryResult& Result)
	{
		Test.TestEqual(TEXT("Offside decision"), Result.Decision, EDecision::Offside);
		Test.TestTrue(TEXT("Offside ends attack"), Result.bAttackEnded);
		Test.TestFalse(TEXT("Offside does not continue"), Result.bContinueResolution);
		Test.TestFalse(TEXT("Offside does not require One-on-One"), Result.bRequiresOneOnOne);
		Test.TestTrue(TEXT("Offside RunnerId remains default"), Result.RunnerId.IsNone());
	}

	bool RunCase(FAutomationTestBase& Test, const int32 Case)
	{
		FThroughBallBehindDefenseP2OutcomeQueryInput Input;
		FThroughBallBehindDefenseP2OutcomeQueryResult Result;
		switch (Case)
		{
		case 1:
			Result = FThroughBallBehindDefenseP2OutcomeQuery::Evaluate(Input);
			ExpectFailureDefaults(Test, Result, EError::P1ExecutionFailed, TEXT("P1ExecutionResult"));
			Test.TestFalse(TEXT("Default P2 D6 presence preserved"), Result.Input.bHasP2DefenseD6);
			Test.TestEqual(TEXT("Default P2 D6 preserved"), Result.Input.P2DefenseD6, 0);
			break;
		case 2:
			Test.TestFalse(TEXT("Default Result fails safely"), Result.bSuccess);
			Test.TestEqual(TEXT("Default Result error"), Result.ErrorCode, EError::None);
			Test.TestEqual(TEXT("Default Result decision"), Result.Decision, EDecision::None);
			Test.TestTrue(TEXT("Default Result RunnerId"), Result.RunnerId.IsNone());
			break;
		case 3:
			ExpectOneOnOne(Test, ExpectSuccess(Test, MakeInput(1)));
			break;
		case 4:
			ExpectOneOnOne(Test, ExpectSuccess(Test, MakeInput(3)));
			break;
		case 5:
			ExpectOffside(Test, ExpectSuccess(Test, MakeInput(4)));
			break;
		case 6:
			ExpectOffside(Test, ExpectSuccess(Test, MakeInput(6)));
			break;
		case 7:
			Input = MakeInput(3, 5);
			Result = ExpectSuccess(Test, Input);
			Test.TestEqual(TEXT("Selected P1 decision preserved"), Result.Input.P1ExecutionResult.Decision, Input.P1ExecutionResult.Decision);
			Test.TestEqual(TEXT("Selected P1 RunnerId preserved"), Result.Input.P1ExecutionResult.RunnerId, Input.P1ExecutionResult.RunnerId);
			Test.TestEqual(TEXT("Selected nested P1 DefenseD6 preserved"), Result.Input.P1ExecutionResult.Input.ResolverInputAssemblyResult.Input.PlanQueryResult.FormulaPlan.DefenseD6, 5);
			Test.TestEqual(TEXT("P2 presence preserved"), Result.Input.bHasP2DefenseD6, Input.bHasP2DefenseD6);
			Test.TestEqual(TEXT("P2 D6 preserved"), Result.Input.P2DefenseD6, Input.P2DefenseD6);
			break;
		case 8:
			Input = MakeInput();
			Input.P1ExecutionResult.bSuccess = false;
			Input.P1ExecutionResult.ErrorCode = EThroughBallBehindDefenseP1FormulaResolutionExecutionErrorCode::InvalidFormulaResolutionResult;
			ExpectFailure(Test, Input, EError::P1ExecutionFailed, TEXT("P1ExecutionResult"));
			break;
		case 9:
			Input = MakeInput();
			Input.P1ExecutionResult.ErrorCode = EThroughBallBehindDefenseP1FormulaResolutionExecutionErrorCode::InvalidFormulaResolutionResult;
			ExpectFailure(Test, Input, EError::InvalidP1ExecutionResult, TEXT("P1ExecutionResult.ErrorCode"));
			break;
		case 10:
			Input = MakeInput();
			Input.P1ExecutionResult.ErrorMessage = TEXT("Forged");
			ExpectFailure(Test, Input, EError::InvalidP1ExecutionResult, TEXT("P1ExecutionResult.ErrorMessage"));
			break;
		case 11:
			Input = MakeInput();
			Input.P1ExecutionResult.InvalidField = TEXT("ForgedField");
			ExpectFailure(Test, Input, EError::InvalidP1ExecutionResult, TEXT("P1ExecutionResult.InvalidField"));
			break;
		case 12:
			Input = MakeInput();
			Input.P1ExecutionResult.Decision = EThroughBallBehindDefenseP1FormulaResolutionExecutionDecision::DefenderStoppedAttack;
			ExpectFailure(Test, Input, EError::InvalidP1ExecutionResult, TEXT("P1ExecutionResult.Decision"));
			break;
		case 13:
			Input = MakeInput();
			Input.P1ExecutionResult.bAttackEnded = true;
			ExpectFailure(Test, Input, EError::InvalidP1ExecutionResult, TEXT("P1ExecutionResult.bAttackEnded"));
			break;
		case 14:
			Input = MakeInput();
			Input.P1ExecutionResult.bContinueResolution = false;
			ExpectFailure(Test, Input, EError::InvalidP1ExecutionResult, TEXT("P1ExecutionResult.bContinueResolution"));
			break;
		case 15:
			Input = MakeInput();
			Input.P1ExecutionResult.bRequiresP2 = false;
			ExpectFailure(Test, Input, EError::InvalidP1ExecutionResult, TEXT("P1ExecutionResult.bRequiresP2"));
			break;
		case 16:
			Input = MakeInput();
			Input.P1ExecutionResult.Decision = EThroughBallBehindDefenseP1FormulaResolutionExecutionDecision::None;
			Input.bHasP2DefenseD6 = false;
			ExpectFailure(Test, Input, EError::InvalidP1ExecutionResult, TEXT("P1ExecutionResult.Decision"));
			break;
		case 17:
			Input = MakeInput();
			Input.P1ExecutionResult.bHasFormulaResolution = false;
			ExpectFailure(Test, Input, EError::InvalidP1ExecutionResult, TEXT("P1ExecutionResult.bHasFormulaResolution"));
			break;
		case 18:
			Input = MakeInput();
			Input.P1ExecutionResult.FormulaResolutionResult.FormulaType = EFormulaType::Finishing;
			ExpectFailure(Test, Input, EError::InvalidP1ExecutionResult, TEXT("P1ExecutionResult.FormulaResolutionResult.FormulaType"));
			break;
		case 19:
			Input = MakeInput();
			Input.P1ExecutionResult.FormulaResolutionResult.Winner = EFormulaWinner::Defender;
			ExpectFailure(Test, Input, EError::InvalidP1ExecutionResult, TEXT("P1ExecutionResult.FormulaResolutionResult.Winner"));
			break;
		case 20:
			Input = MakeInput();
			Input.P1ExecutionResult.FormulaResolutionResult.WinReason = EFormulaWinReason::None;
			ExpectFailure(Test, Input, EError::InvalidP1ExecutionResult, TEXT("P1ExecutionResult.FormulaResolutionResult.WinReason"));
			break;
		case 21:
			Input = MakeInput();
			Input.P1ExecutionResult.FormulaResolutionResult.bIsGoal = true;
			ExpectFailure(Test, Input, EError::InvalidP1ExecutionResult, TEXT("P1ExecutionResult.FormulaResolutionResult.bIsGoal"));
			break;
		case 22:
			Input = MakeInput();
			Input.P1ExecutionResult.FormulaResolutionResult.bAttackEnded = true;
			ExpectFailure(Test, Input, EError::InvalidP1ExecutionResult, TEXT("P1ExecutionResult.FormulaResolutionResult.bAttackEnded"));
			Input = MakeInput();
			Input.P1ExecutionResult.FormulaResolutionResult.bContinueResolution = false;
			ExpectFailure(Test, Input, EError::InvalidP1ExecutionResult, TEXT("P1ExecutionResult.FormulaResolutionResult.bContinueResolution"));
			break;
		case 23:
			Input = MakeInput();
			Input.P1ExecutionResult.Input.ResolverInputAssemblyResult.bSuccess = false;
			ExpectFailure(Test, Input, EError::InvalidP1ExecutionResult, TEXT("P1ExecutionResult.Input.ResolverInputAssemblyResult.bSuccess"));
			Input = MakeInput();
			Input.P1ExecutionResult.Input.ResolverInputAssemblyResult.ErrorCode = EThroughBallBehindDefenseP1FormulaResolverInputAssemblyErrorCode::InvalidPlanQueryResult;
			ExpectFailure(Test, Input, EError::InvalidP1ExecutionResult, TEXT("P1ExecutionResult.Input.ResolverInputAssemblyResult.ErrorCode"));
			Input = MakeInput();
			Input.P1ExecutionResult.Input.ResolverInputAssemblyResult.ErrorMessage = TEXT("Forged");
			ExpectFailure(Test, Input, EError::InvalidP1ExecutionResult, TEXT("P1ExecutionResult.Input.ResolverInputAssemblyResult.ErrorMessage"));
			Input = MakeInput();
			Input.P1ExecutionResult.Input.ResolverInputAssemblyResult.InvalidField = TEXT("ForgedField");
			ExpectFailure(Test, Input, EError::InvalidP1ExecutionResult, TEXT("P1ExecutionResult.Input.ResolverInputAssemblyResult.InvalidField"));
			break;
		case 24:
			Input = MakeInput();
			Input.P1ExecutionResult.Input.ResolverInputAssemblyResult.bHasResolverInput = false;
			ExpectFailure(Test, Input, EError::InvalidP1ExecutionResult, TEXT("P1ExecutionResult.Input.ResolverInputAssemblyResult.bHasResolverInput"));
			break;
		case 25:
			Input = MakeInput();
			Input.P1ExecutionResult.Input.ResolverInputAssemblyResult.Input.PlanQueryResult.bSuccess = false;
			ExpectFailure(Test, Input, EError::InvalidP1ExecutionResult, TEXT("P1ExecutionResult.Input.ResolverInputAssemblyResult.Input.PlanQueryResult.bSuccess"));
			Input = MakeInput();
			Input.P1ExecutionResult.Input.ResolverInputAssemblyResult.Input.PlanQueryResult.ErrorCode = EThroughBallBehindDefenseP1PlanQueryErrorCode::InvalidDefenseD6;
			ExpectFailure(Test, Input, EError::InvalidP1ExecutionResult, TEXT("P1ExecutionResult.Input.ResolverInputAssemblyResult.Input.PlanQueryResult.ErrorCode"));
			Input = MakeInput();
			Input.P1ExecutionResult.Input.ResolverInputAssemblyResult.Input.PlanQueryResult.ErrorMessage = TEXT("Forged");
			ExpectFailure(Test, Input, EError::InvalidP1ExecutionResult, TEXT("P1ExecutionResult.Input.ResolverInputAssemblyResult.Input.PlanQueryResult.ErrorMessage"));
			Input = MakeInput();
			Input.P1ExecutionResult.Input.ResolverInputAssemblyResult.Input.PlanQueryResult.InvalidField = TEXT("ForgedField");
			ExpectFailure(Test, Input, EError::InvalidP1ExecutionResult, TEXT("P1ExecutionResult.Input.ResolverInputAssemblyResult.Input.PlanQueryResult.InvalidField"));
			Input = MakeInput();
			Input.P1ExecutionResult.Input.ResolverInputAssemblyResult.Input.PlanQueryResult.Decision = EThroughBallBehindDefenseP1PlanQueryDecision::OutOfPlay;
			ExpectFailure(Test, Input, EError::InvalidP1ExecutionResult, TEXT("P1ExecutionResult.Input.ResolverInputAssemblyResult.Input.PlanQueryResult.Decision"));
			Input = MakeInput();
			Input.P1ExecutionResult.Input.ResolverInputAssemblyResult.Input.PlanQueryResult.bHasFormulaPlan = false;
			ExpectFailure(Test, Input, EError::InvalidP1ExecutionResult, TEXT("P1ExecutionResult.Input.ResolverInputAssemblyResult.Input.PlanQueryResult.bHasFormulaPlan"));
			break;
		case 26:
			Input = MakeInput();
			Input.P1ExecutionResult.RunnerId = NAME_None;
			ExpectFailure(Test, Input, EError::InvalidP1ExecutionResult, TEXT("P1ExecutionResult.RunnerId"));
			Input = MakeInput();
			Input.P1ExecutionResult.Input.ResolverInputAssemblyResult.Input.PlanQueryResult.FormulaPlan.RunnerId = NAME_None;
			ExpectFailure(Test, Input, EError::InvalidP1ExecutionResult, TEXT("P1ExecutionResult.Input.ResolverInputAssemblyResult.Input.PlanQueryResult.FormulaPlan.RunnerId"));
			Input = MakeInput();
			Input.P1ExecutionResult.RunnerId = TEXT("ThroughBall.P2.ForgedRunner");
			ExpectFailure(Test, Input, EError::InvalidP1ExecutionResult, TEXT("P1ExecutionResult.RunnerId"));
			break;
		case 27:
			Input = MakeInput();
			Input.bHasP2DefenseD6 = false;
			ExpectFailure(Test, Input, EError::MissingP2DefenseD6, TEXT("P2DefenseD6"));
			break;
		case 28:
			ExpectFailure(Test, MakeInput(0), EError::InvalidP2DefenseD6, TEXT("P2DefenseD6"));
			break;
		case 29:
			ExpectFailure(Test, MakeInput(7), EError::InvalidP2DefenseD6, TEXT("P2DefenseD6"));
			break;
		case 30:
			{
				const auto LowP1Defense = ExpectSuccess(Test, MakeInput(2, 1));
				const auto HighP1Defense = ExpectSuccess(Test, MakeInput(2, 6));
				Test.TestEqual(TEXT("Different P1 DefenseD6 values are preserved"), LowP1Defense.Input.P1ExecutionResult.Input.ResolverInputAssemblyResult.Input.PlanQueryResult.FormulaPlan.DefenseD6, 1);
				Test.TestEqual(TEXT("Second P1 DefenseD6 is preserved"), HighP1Defense.Input.P1ExecutionResult.Input.ResolverInputAssemblyResult.Input.PlanQueryResult.FormulaPlan.DefenseD6, 6);
				Test.TestEqual(TEXT("Same P2 D6 produces same decision"), LowP1Defense.Decision, HighP1Defense.Decision);
				ExpectOneOnOne(Test, LowP1Defense);
				ExpectOneOnOne(Test, HighP1Defense);
			}
			break;
		case 31:
			Input = MakeInput(3);
			{
				const auto OneOnOne = ExpectSuccess(Test, Input);
				Input.P2DefenseD6 = 4;
				const auto Offside = ExpectSuccess(Test, Input);
				ExpectOneOnOne(Test, OneOnOne);
				ExpectOffside(Test, Offside);
			}
			break;
		case 32:
			Result = ExpectSuccess(Test, MakeInput(2));
			Test.TestEqual(TEXT("One-on-One copies the real P1 RunnerId"), Result.RunnerId, Result.Input.P1ExecutionResult.RunnerId);
			ExpectOneOnOne(Test, Result);
			break;
		case 33:
			Result = ExpectSuccess(Test, MakeInput(5));
			ExpectOffside(Test, Result);
			break;
		case 34:
			Input = MakeInput(3, 5);
			{
				const auto First = FThroughBallBehindDefenseP2OutcomeQuery::Evaluate(Input);
				const auto Second = FThroughBallBehindDefenseP2OutcomeQuery::Evaluate(Input);
				Test.TestEqual(TEXT("Selected success deterministic"), First.bSuccess, Second.bSuccess);
				Test.TestEqual(TEXT("Selected error deterministic"), First.ErrorCode, Second.ErrorCode);
				Test.TestEqual(TEXT("Selected message deterministic"), First.ErrorMessage, Second.ErrorMessage);
				Test.TestEqual(TEXT("Selected invalid field deterministic"), First.InvalidField, Second.InvalidField);
				Test.TestEqual(TEXT("Selected decision deterministic"), First.Decision, Second.Decision);
				Test.TestEqual(TEXT("Attack-ended deterministic"), First.bAttackEnded, Second.bAttackEnded);
				Test.TestEqual(TEXT("Continue deterministic"), First.bContinueResolution, Second.bContinueResolution);
				Test.TestEqual(TEXT("One-on-One requirement deterministic"), First.bRequiresOneOnOne, Second.bRequiresOneOnOne);
				Test.TestEqual(TEXT("Runner deterministic"), First.RunnerId, Second.RunnerId);
				Test.TestEqual(TEXT("Selected P2 D6 input deterministic"), First.Input.P2DefenseD6, Second.Input.P2DefenseD6);

				FString Implementation;
				const FString Path = FPaths::Combine(
					FPaths::ProjectDir(),
					TEXT("Source/FMCodex/CoreRules/ThroughBallBehindDefenseP2OutcomeQuery.cpp"));
				Test.TestTrue(TEXT("Production source is readable for auxiliary dependency audit"), FFileHelper::LoadFileToString(Implementation, *Path));
				Test.TestFalse(TEXT("Does not call P1 Plan Query"), Implementation.Contains(TEXT("FThroughBallBehindDefenseP1PlanQuery::Evaluate")));
				Test.TestFalse(TEXT("Does not call P1 Assembler"), Implementation.Contains(TEXT("FThroughBallBehindDefenseP1FormulaResolverInputAssembler::Assemble")));
				Test.TestFalse(TEXT("Does not call P1 Executor"), Implementation.Contains(TEXT("FThroughBallBehindDefenseP1FormulaResolutionExecutor::Execute")));
				Test.TestFalse(TEXT("Does not call FormulaResolver"), Implementation.Contains(TEXT("UFormulaResolver::ResolveFormula")));
				Test.TestFalse(TEXT("Does not use Active Goalkeeper"), Implementation.Contains(TEXT("ActiveGoalkeeper")));
				Test.TestFalse(TEXT("Does not use RNG"), Implementation.Contains(TEXT("FMath::Rand")) || Implementation.Contains(TEXT("FRandomStream")));
				Test.TestFalse(TEXT("Does not access Match State"), Implementation.Contains(TEXT("MatchState")));
				Test.TestFalse(TEXT("Does not call FormulaAttackFlow"), Implementation.Contains(TEXT("FormulaAttackFlow")));
				Test.TestFalse(TEXT("Does not call MatchPlay"), Implementation.Contains(TEXT("MatchPlay")));
				Test.TestFalse(TEXT("Does not create Handoff"), Implementation.Contains(TEXT("Handoff")));
			}
			break;
		default:
			Test.AddError(TEXT("Unknown P2 Outcome Query test case."));
			return false;
		}
		return true;
	}
}

#define THROUGH_BALL_BEHIND_DEFENSE_P2_OUTCOME_TEST(Number, Name) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
		FThroughBallBehindDefenseP2OutcomeTest##Number, \
		"FMCodex.CoreRules.ThroughBallBehindDefenseP2OutcomeQuery." Name, \
		EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter) \
	bool FThroughBallBehindDefenseP2OutcomeTest##Number::RunTest( \
		const FString& Parameters) \
	{ \
		return ThroughBallBehindDefenseP2OutcomeQueryTests::RunCase(*this, Number); \
	}

THROUGH_BALL_BEHIND_DEFENSE_P2_OUTCOME_TEST(1, "01DefaultInputFailsSafely")
THROUGH_BALL_BEHIND_DEFENSE_P2_OUTCOME_TEST(2, "02DefaultResultIsFailureSafe")
THROUGH_BALL_BEHIND_DEFENSE_P2_OUTCOME_TEST(3, "03P2DefenseD6OneRequiresOneOnOne")
THROUGH_BALL_BEHIND_DEFENSE_P2_OUTCOME_TEST(4, "04P2DefenseD6ThreeRequiresOneOnOne")
THROUGH_BALL_BEHIND_DEFENSE_P2_OUTCOME_TEST(5, "05P2DefenseD6FourIsOffside")
THROUGH_BALL_BEHIND_DEFENSE_P2_OUTCOME_TEST(6, "06P2DefenseD6SixIsOffside")
THROUGH_BALL_BEHIND_DEFENSE_P2_OUTCOME_TEST(7, "07SelectedHighValueInputFieldsPreserved")
THROUGH_BALL_BEHIND_DEFENSE_P2_OUTCOME_TEST(8, "08FormalP1FailureMapsWithoutCopyingError")
THROUGH_BALL_BEHIND_DEFENSE_P2_OUTCOME_TEST(9, "09RejectsSuccessfulP1WithErrorCode")
THROUGH_BALL_BEHIND_DEFENSE_P2_OUTCOME_TEST(10, "10RejectsSuccessfulP1WithErrorMessage")
THROUGH_BALL_BEHIND_DEFENSE_P2_OUTCOME_TEST(11, "11RejectsSuccessfulP1WithInvalidField")
THROUGH_BALL_BEHIND_DEFENSE_P2_OUTCOME_TEST(12, "12RejectsDecisionOtherThanP2Required")
THROUGH_BALL_BEHIND_DEFENSE_P2_OUTCOME_TEST(13, "13RejectsEndedP1Attack")
THROUGH_BALL_BEHIND_DEFENSE_P2_OUTCOME_TEST(14, "14RejectsP1WithoutContinuation")
THROUGH_BALL_BEHIND_DEFENSE_P2_OUTCOME_TEST(15, "15RejectsP1WithoutP2Requirement")
THROUGH_BALL_BEHIND_DEFENSE_P2_OUTCOME_TEST(16, "16P1ErrorPrecedesMissingP2DefenseD6")
THROUGH_BALL_BEHIND_DEFENSE_P2_OUTCOME_TEST(17, "17RejectsMissingFormulaResolution")
THROUGH_BALL_BEHIND_DEFENSE_P2_OUTCOME_TEST(18, "18RejectsNonTransitionFormulaResult")
THROUGH_BALL_BEHIND_DEFENSE_P2_OUTCOME_TEST(19, "19RejectsNonAttackerFormulaWinner")
THROUGH_BALL_BEHIND_DEFENSE_P2_OUTCOME_TEST(20, "20RejectsMissingFormulaWinReason")
THROUGH_BALL_BEHIND_DEFENSE_P2_OUTCOME_TEST(21, "21RejectsGoalFormulaResult")
THROUGH_BALL_BEHIND_DEFENSE_P2_OUTCOME_TEST(22, "22RejectsFormulaTerminalMetadataMismatch")
THROUGH_BALL_BEHIND_DEFENSE_P2_OUTCOME_TEST(23, "23RejectsInvalidNestedAssemblyEnvelope")
THROUGH_BALL_BEHIND_DEFENSE_P2_OUTCOME_TEST(24, "24RejectsMissingNestedResolverInput")
THROUGH_BALL_BEHIND_DEFENSE_P2_OUTCOME_TEST(25, "25RejectsInvalidNestedPlanEnvelope")
THROUGH_BALL_BEHIND_DEFENSE_P2_OUTCOME_TEST(26, "26RejectsInvalidRunnerProvenance")
THROUGH_BALL_BEHIND_DEFENSE_P2_OUTCOME_TEST(27, "27RejectsMissingP2DefenseD6")
THROUGH_BALL_BEHIND_DEFENSE_P2_OUTCOME_TEST(28, "28RejectsP2DefenseD6BelowRange")
THROUGH_BALL_BEHIND_DEFENSE_P2_OUTCOME_TEST(29, "29RejectsP2DefenseD6AboveRange")
THROUGH_BALL_BEHIND_DEFENSE_P2_OUTCOME_TEST(30, "30P1DefenseD6DoesNotSelectP2Outcome")
THROUGH_BALL_BEHIND_DEFENSE_P2_OUTCOME_TEST(31, "31P2DefenseD6AloneSwitchesOutcome")
THROUGH_BALL_BEHIND_DEFENSE_P2_OUTCOME_TEST(32, "32OneOnOneCopiesRealRunnerId")
THROUGH_BALL_BEHIND_DEFENSE_P2_OUTCOME_TEST(33, "33OffsideKeepsTerminalMetadataAndNoRunner")
THROUGH_BALL_BEHIND_DEFENSE_P2_OUTCOME_TEST(34, "34SelectedObservedFieldsRepeatDeterministically")

#undef THROUGH_BALL_BEHIND_DEFENSE_P2_OUTCOME_TEST

#endif
