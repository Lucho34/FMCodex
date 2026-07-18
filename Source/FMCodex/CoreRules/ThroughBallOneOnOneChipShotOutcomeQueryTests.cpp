#include "ThroughBallOneOnOneChipShotOutcomeQuery.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

#include <type_traits>

namespace ThroughBallOneOnOneChipShotOutcomeQueryTests
{
	const FName AttackingOwnerId(TEXT("Player.ChipShot.Attacking"));
	const FName DefendingOwnerId(TEXT("Player.ChipShot.Defending"));
	const FName ShooterCardId(TEXT("Card.ChipShot.Shooter"));
	const FGuid LogId(0x12345678, 0x23456789, 0x34567890, 0x45678901);

	FThroughBallOneOnOneHandoffCreationResult MakeHandoffCreationResult()
	{
		FThroughBallOneOnOneHandoffCreationResult Result;
		Result.bSuccess = true;
		Result.bHasHandoff = true;
		Result.Handoff.AttackingOwnerId = AttackingOwnerId;
		Result.Handoff.DefendingOwnerId = DefendingOwnerId;
		Result.Handoff.ShooterCardId = ShooterCardId;
		return Result;
	}

	FThroughBallOneOnOneChipShotOutcomeQueryInput MakeInput(
		const int32 ChipShotAttackD6 = 4)
	{
		FThroughBallOneOnOneChipShotOutcomeQueryInput Input;
		Input.HandoffCreationResult = MakeHandoffCreationResult();
		Input.bHasChipShotAttackD6 = true;
		Input.ChipShotAttackD6 = ChipShotAttackD6;
		Input.LogId = LogId;
		Input.TurnIndex = 7;
		return Input;
	}

	bool AreHandoffsEqual(
		const FThroughBallOneOnOneHandoff& Left,
		const FThroughBallOneOnOneHandoff& Right)
	{
		return Left.AttackingOwnerId == Right.AttackingOwnerId
			&& Left.DefendingOwnerId == Right.DefendingOwnerId
			&& Left.ShooterCardId == Right.ShooterCardId;
	}

	bool AreHandoffResultsEqual(
		const FThroughBallOneOnOneHandoffCreationResult& Left,
		const FThroughBallOneOnOneHandoffCreationResult& Right)
	{
		return Left.bSuccess == Right.bSuccess
			&& Left.ErrorCode == Right.ErrorCode
			&& Left.ErrorMessage == Right.ErrorMessage
			&& Left.InvalidField == Right.InvalidField
			&& Left.bHasHandoff == Right.bHasHandoff
			&& AreHandoffsEqual(Left.Handoff, Right.Handoff);
	}

	bool AreInputsEqual(
		const FThroughBallOneOnOneChipShotOutcomeQueryInput& Left,
		const FThroughBallOneOnOneChipShotOutcomeQueryInput& Right)
	{
		return AreHandoffResultsEqual(
				Left.HandoffCreationResult,
				Right.HandoffCreationResult)
			&& Left.bHasChipShotAttackD6 == Right.bHasChipShotAttackD6
			&& Left.ChipShotAttackD6 == Right.ChipShotAttackD6
			&& Left.LogId == Right.LogId
			&& Left.TurnIndex == Right.TurnIndex;
	}

	bool AreResultsEqual(
		const FThroughBallOneOnOneChipShotOutcomeQueryResult& Left,
		const FThroughBallOneOnOneChipShotOutcomeQueryResult& Right)
	{
		return Left.bSuccess == Right.bSuccess
			&& Left.ErrorCode == Right.ErrorCode
			&& Left.ErrorMessage == Right.ErrorMessage
			&& Left.InvalidField == Right.InvalidField
			&& AreInputsEqual(Left.Input, Right.Input)
			&& Left.Decision == Right.Decision
			&& Left.bAttackEnded == Right.bAttackEnded
			&& Left.bContinueResolution == Right.bContinueResolution
			&& Left.bIsGoal == Right.bIsGoal;
	}

	FThroughBallOneOnOneChipShotOutcomeQueryResult TestFailure(
		FAutomationTestBase& Test,
		const FThroughBallOneOnOneChipShotOutcomeQueryInput& Input,
		const EThroughBallOneOnOneChipShotOutcomeQueryErrorCode ErrorCode,
		const FName InvalidField)
	{
		const FThroughBallOneOnOneChipShotOutcomeQueryResult Result =
			FThroughBallOneOnOneChipShotOutcomeQuery::Evaluate(Input);
		Test.TestFalse(TEXT("Failure is not successful"), Result.bSuccess);
		Test.TestEqual(TEXT("Failure error code"), Result.ErrorCode, ErrorCode);
		Test.TestFalse(TEXT("Failure message is non-empty"), Result.ErrorMessage.IsEmpty());
		Test.TestEqual(TEXT("Failure invalid field"), Result.InvalidField, InvalidField);
		Test.TestEqual(
			TEXT("Failure decision remains None"),
			Result.Decision,
			EThroughBallOneOnOneChipShotOutcomeDecision::None);
		Test.TestFalse(TEXT("Failure does not end attack"), Result.bAttackEnded);
		Test.TestFalse(TEXT("Failure does not continue"), Result.bContinueResolution);
		Test.TestFalse(TEXT("Failure is not a goal"), Result.bIsGoal);
		Test.TestTrue(TEXT("Failure preserves complete Input"), AreInputsEqual(Result.Input, Input));
		return Result;
	}

	FThroughBallOneOnOneChipShotOutcomeQueryResult TestSuccess(
		FAutomationTestBase& Test,
		const FThroughBallOneOnOneChipShotOutcomeQueryInput& Input,
		const EThroughBallOneOnOneChipShotOutcomeDecision Decision)
	{
		const FThroughBallOneOnOneChipShotOutcomeQueryResult Result =
			FThroughBallOneOnOneChipShotOutcomeQuery::Evaluate(Input);
		Test.TestTrue(TEXT("Success is successful"), Result.bSuccess);
		Test.TestEqual(
			TEXT("Success error code is None"),
			Result.ErrorCode,
			EThroughBallOneOnOneChipShotOutcomeQueryErrorCode::None);
		Test.TestTrue(TEXT("Success message is empty"), Result.ErrorMessage.IsEmpty());
		Test.TestTrue(TEXT("Success invalid field is None"), Result.InvalidField.IsNone());
		Test.TestEqual(TEXT("Success decision"), Result.Decision, Decision);
		Test.TestTrue(TEXT("Success ends attack"), Result.bAttackEnded);
		Test.TestFalse(TEXT("Success does not continue"), Result.bContinueResolution);
		Test.TestEqual(
			TEXT("Success goal flag matches decision"),
			Result.bIsGoal,
			Decision == EThroughBallOneOnOneChipShotOutcomeDecision::Goal);
		Test.TestTrue(TEXT("Success preserves complete Input"), AreInputsEqual(Result.Input, Input));
		return Result;
	}

	bool RunCase(FAutomationTestBase& Test, const int32 CaseNumber)
	{
		switch (CaseNumber)
		{
		case 1:
		{
			const FThroughBallOneOnOneChipShotOutcomeQueryResult Result;
			Test.TestFalse(TEXT("Default is not successful"), Result.bSuccess);
			Test.TestEqual(TEXT("Default error is None"), Result.ErrorCode, EThroughBallOneOnOneChipShotOutcomeQueryErrorCode::None);
			Test.TestEqual(TEXT("Default decision is None"), Result.Decision, EThroughBallOneOnOneChipShotOutcomeDecision::None);
			Test.TestFalse(TEXT("Default does not end attack"), Result.bAttackEnded);
			Test.TestFalse(TEXT("Default does not continue"), Result.bContinueResolution);
			Test.TestFalse(TEXT("Default is not goal"), Result.bIsGoal);
			return true;
		}
		case 2:
			for (int32 D6 = 1; D6 <= 3; ++D6)
			{
				TestSuccess(Test, MakeInput(D6), EThroughBallOneOnOneChipShotOutcomeDecision::Miss);
			}
			return true;
		case 3:
			for (int32 D6 = 4; D6 <= 6; ++D6)
			{
				TestSuccess(Test, MakeInput(D6), EThroughBallOneOnOneChipShotOutcomeDecision::Goal);
			}
			return true;
		case 4:
			TestSuccess(Test, MakeInput(3), EThroughBallOneOnOneChipShotOutcomeDecision::Miss);
			TestSuccess(Test, MakeInput(4), EThroughBallOneOnOneChipShotOutcomeDecision::Goal);
			return true;
		case 5:
		{
			FThroughBallOneOnOneChipShotOutcomeQueryInput Input = MakeInput();
			Input.HandoffCreationResult.bSuccess = false;
			Input.HandoffCreationResult.ErrorCode = EThroughBallOneOnOneHandoffCreationErrorCode::SourceOutcomeFailed;
			Input.bHasChipShotAttackD6 = false;
			Input.LogId.Invalidate();
			TestFailure(Test, Input, EThroughBallOneOnOneChipShotOutcomeQueryErrorCode::HandoffCreationFailed, TEXT("HandoffCreationResult"));
			return true;
		}
		case 6:
		{
			FThroughBallOneOnOneChipShotOutcomeQueryInput Input = MakeInput();
			Input.HandoffCreationResult.ErrorCode = EThroughBallOneOnOneHandoffCreationErrorCode::InvalidSourceOutcomeResult;
			TestFailure(Test, Input, EThroughBallOneOnOneChipShotOutcomeQueryErrorCode::InvalidHandoffCreationResult, TEXT("HandoffCreationResult.ErrorCode"));
			Input = MakeInput();
			Input.HandoffCreationResult.ErrorMessage = TEXT("Dirty diagnostics");
			TestFailure(Test, Input, EThroughBallOneOnOneChipShotOutcomeQueryErrorCode::InvalidHandoffCreationResult, TEXT("HandoffCreationResult.ErrorMessage"));
			Input = MakeInput();
			Input.HandoffCreationResult.InvalidField = TEXT("DirtyField");
			TestFailure(Test, Input, EThroughBallOneOnOneChipShotOutcomeQueryErrorCode::InvalidHandoffCreationResult, TEXT("HandoffCreationResult.InvalidField"));
			return true;
		}
		case 7:
		{
			FThroughBallOneOnOneChipShotOutcomeQueryInput Input = MakeInput();
			Input.HandoffCreationResult.bHasHandoff = false;
			TestFailure(Test, Input, EThroughBallOneOnOneChipShotOutcomeQueryErrorCode::InvalidHandoffCreationResult, TEXT("HandoffCreationResult.bHasHandoff"));
			return true;
		}
		case 8:
		{
			FThroughBallOneOnOneChipShotOutcomeQueryInput Input = MakeInput();
			Input.HandoffCreationResult.Handoff.AttackingOwnerId = NAME_None;
			TestFailure(Test, Input, EThroughBallOneOnOneChipShotOutcomeQueryErrorCode::InvalidHandoffCreationResult, TEXT("Handoff.AttackingOwnerId"));
			Input = MakeInput();
			Input.HandoffCreationResult.Handoff.DefendingOwnerId = NAME_None;
			TestFailure(Test, Input, EThroughBallOneOnOneChipShotOutcomeQueryErrorCode::InvalidHandoffCreationResult, TEXT("Handoff.DefendingOwnerId"));
			Input = MakeInput();
			Input.HandoffCreationResult.Handoff.DefendingOwnerId = AttackingOwnerId;
			TestFailure(Test, Input, EThroughBallOneOnOneChipShotOutcomeQueryErrorCode::InvalidHandoffCreationResult, TEXT("Handoff.DefendingOwnerId"));
			Input = MakeInput();
			Input.HandoffCreationResult.Handoff.ShooterCardId = NAME_None;
			TestFailure(Test, Input, EThroughBallOneOnOneChipShotOutcomeQueryErrorCode::InvalidHandoffCreationResult, TEXT("Handoff.ShooterCardId"));
			return true;
		}
		case 9:
		{
			FThroughBallOneOnOneChipShotOutcomeQueryInput Input = MakeInput();
			Input.bHasChipShotAttackD6 = false;
			TestFailure(Test, Input, EThroughBallOneOnOneChipShotOutcomeQueryErrorCode::MissingChipShotAttackD6, TEXT("ChipShotAttackD6"));
			return true;
		}
		case 10:
			for (const int32 D6 : {0, -1})
			{
				TestFailure(Test, MakeInput(D6), EThroughBallOneOnOneChipShotOutcomeQueryErrorCode::InvalidChipShotAttackD6, TEXT("ChipShotAttackD6"));
			}
			return true;
		case 11:
			for (const int32 D6 : {7, 100})
			{
				TestFailure(Test, MakeInput(D6), EThroughBallOneOnOneChipShotOutcomeQueryErrorCode::InvalidChipShotAttackD6, TEXT("ChipShotAttackD6"));
			}
			return true;
		case 12:
		{
			FThroughBallOneOnOneChipShotOutcomeQueryInput Input = MakeInput();
			Input.LogId.Invalidate();
			TestFailure(Test, Input, EThroughBallOneOnOneChipShotOutcomeQueryErrorCode::InvalidLogContext, TEXT("LogId"));
			return true;
		}
		case 13:
		{
			FThroughBallOneOnOneChipShotOutcomeQueryInput Input = MakeInput();
			Input.TurnIndex = -1;
			TestFailure(Test, Input, EThroughBallOneOnOneChipShotOutcomeQueryErrorCode::InvalidLogContext, TEXT("TurnIndex"));
			return true;
		}
		case 14:
		{
			FThroughBallOneOnOneChipShotOutcomeQueryInput Input = MakeInput();
			Input.HandoffCreationResult.bSuccess = false;
			Input.bHasChipShotAttackD6 = false;
			Input.LogId.Invalidate();
			TestFailure(Test, Input, EThroughBallOneOnOneChipShotOutcomeQueryErrorCode::HandoffCreationFailed, TEXT("HandoffCreationResult"));
			Input = MakeInput();
			Input.bHasChipShotAttackD6 = false;
			Input.LogId.Invalidate();
			TestFailure(Test, Input, EThroughBallOneOnOneChipShotOutcomeQueryErrorCode::MissingChipShotAttackD6, TEXT("ChipShotAttackD6"));
			Input = MakeInput(7);
			Input.LogId.Invalidate();
			TestFailure(Test, Input, EThroughBallOneOnOneChipShotOutcomeQueryErrorCode::InvalidChipShotAttackD6, TEXT("ChipShotAttackD6"));
			return true;
		}
		case 15:
		{
			FThroughBallOneOnOneChipShotOutcomeQueryInput Input = MakeInput();
			Input.LogId.Invalidate();
			Input.TurnIndex = -1;
			TestFailure(Test, Input, EThroughBallOneOnOneChipShotOutcomeQueryErrorCode::InvalidLogContext, TEXT("LogId"));
			return true;
		}
		case 16:
		{
			const FThroughBallOneOnOneChipShotOutcomeQueryInput SuccessInput = MakeInput(6);
			const FThroughBallOneOnOneChipShotOutcomeQueryResult Success =
				TestSuccess(Test, SuccessInput, EThroughBallOneOnOneChipShotOutcomeDecision::Goal);
			Test.TestTrue(TEXT("Success explicitly preserves Input"), AreInputsEqual(Success.Input, SuccessInput));
			FThroughBallOneOnOneChipShotOutcomeQueryInput FailureInput = MakeInput();
			FailureInput.TurnIndex = -9;
			const FThroughBallOneOnOneChipShotOutcomeQueryResult Failure =
				TestFailure(Test, FailureInput, EThroughBallOneOnOneChipShotOutcomeQueryErrorCode::InvalidLogContext, TEXT("TurnIndex"));
			Test.TestTrue(TEXT("Failure explicitly preserves Input"), AreInputsEqual(Failure.Input, FailureInput));
			return true;
		}
		case 17:
		{
			const FThroughBallOneOnOneChipShotOutcomeQueryInput Input = MakeInput(5);
			const FThroughBallOneOnOneChipShotOutcomeQueryInput Before = Input;
			const FThroughBallOneOnOneChipShotOutcomeQueryResult First = FThroughBallOneOnOneChipShotOutcomeQuery::Evaluate(Input);
			const FThroughBallOneOnOneChipShotOutcomeQueryResult Second = FThroughBallOneOnOneChipShotOutcomeQuery::Evaluate(Input);
			Test.TestTrue(TEXT("Evaluation is deterministic"), AreResultsEqual(First, Second));
			Test.TestTrue(TEXT("Evaluation does not mutate Input"), AreInputsEqual(Input, Before));
			return true;
		}
		case 18:
		{
			using FExpectedEvaluate = FThroughBallOneOnOneChipShotOutcomeQueryResult (*)(
				const FThroughBallOneOnOneChipShotOutcomeQueryInput&);
			static_assert(
				std::is_same_v<
					decltype(&FThroughBallOneOnOneChipShotOutcomeQuery::Evaluate),
					FExpectedEvaluate>,
				"Evaluate must expose only the frozen formal Input signature.");
			static_assert(
				std::is_same_v<
					decltype(FThroughBallOneOnOneChipShotOutcomeQueryInput::HandoffCreationResult),
					FThroughBallOneOnOneHandoffCreationResult>,
				"Input must own the formal Handoff Creation Result by value.");
			const FThroughBallOneOnOneChipShotOutcomeQueryResult Result =
				FThroughBallOneOnOneChipShotOutcomeQuery::Evaluate(MakeInput());
			Test.TestTrue(TEXT("Frozen public surface remains usable"), Result.bSuccess);
			return true;
		}
		default:
			Test.AddError(TEXT("Unknown Chip Shot Outcome Query test case."));
			return false;
		}
	}
}

#define THROUGH_BALL_ONE_ON_ONE_CHIP_SHOT_OUTCOME_QUERY_TEST(Number, Name) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
		FThroughBallOneOnOneChipShotOutcomeQueryTest##Number, \
		"FMCodex.CoreRules.ThroughBallOneOnOneChipShotOutcomeQuery." Name, \
		EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter) \
	bool FThroughBallOneOnOneChipShotOutcomeQueryTest##Number::RunTest( \
		const FString& Parameters) \
	{ \
		return ThroughBallOneOnOneChipShotOutcomeQueryTests::RunCase(*this, Number); \
	}

THROUGH_BALL_ONE_ON_ONE_CHIP_SHOT_OUTCOME_QUERY_TEST(1, "01DefaultResultIsFailureSafe")
THROUGH_BALL_ONE_ON_ONE_CHIP_SHOT_OUTCOME_QUERY_TEST(2, "02ChipShotD6OneToThreeProducesMiss")
THROUGH_BALL_ONE_ON_ONE_CHIP_SHOT_OUTCOME_QUERY_TEST(3, "03ChipShotD6FourToSixProducesGoal")
THROUGH_BALL_ONE_ON_ONE_CHIP_SHOT_OUTCOME_QUERY_TEST(4, "04ChipShotDecisionBoundaryIsThreeMissFourGoal")
THROUGH_BALL_ONE_ON_ONE_CHIP_SHOT_OUTCOME_QUERY_TEST(5, "05FailedHandoffCreationResultIsRejectedBeforeResidualPayload")
THROUGH_BALL_ONE_ON_ONE_CHIP_SHOT_OUTCOME_QUERY_TEST(6, "06DirtySuccessfulHandoffDiagnosticsAreRejected")
THROUGH_BALL_ONE_ON_ONE_CHIP_SHOT_OUTCOME_QUERY_TEST(7, "07MissingHandoffPresenceIsRejected")
THROUGH_BALL_ONE_ON_ONE_CHIP_SHOT_OUTCOME_QUERY_TEST(8, "08InvalidHandoffIdentitiesAreRejected")
THROUGH_BALL_ONE_ON_ONE_CHIP_SHOT_OUTCOME_QUERY_TEST(9, "09MissingChipShotAttackD6IsRejected")
THROUGH_BALL_ONE_ON_ONE_CHIP_SHOT_OUTCOME_QUERY_TEST(10, "10ChipShotAttackD6BelowRangeIsRejected")
THROUGH_BALL_ONE_ON_ONE_CHIP_SHOT_OUTCOME_QUERY_TEST(11, "11ChipShotAttackD6AboveRangeIsRejected")
THROUGH_BALL_ONE_ON_ONE_CHIP_SHOT_OUTCOME_QUERY_TEST(12, "12InvalidLogIdIsRejected")
THROUGH_BALL_ONE_ON_ONE_CHIP_SHOT_OUTCOME_QUERY_TEST(13, "13NegativeTurnIndexIsRejected")
THROUGH_BALL_ONE_ON_ONE_CHIP_SHOT_OUTCOME_QUERY_TEST(14, "14HandoffAndD6ErrorsPrecedeLogErrors")
THROUGH_BALL_ONE_ON_ONE_CHIP_SHOT_OUTCOME_QUERY_TEST(15, "15LogIdErrorPrecedesTurnIndexError")
THROUGH_BALL_ONE_ON_ONE_CHIP_SHOT_OUTCOME_QUERY_TEST(16, "16InputIsPreservedOnSuccessAndFailure")
THROUGH_BALL_ONE_ON_ONE_CHIP_SHOT_OUTCOME_QUERY_TEST(17, "17EvaluationIsDeterministicAndDoesNotMutateInput")
THROUGH_BALL_ONE_ON_ONE_CHIP_SHOT_OUTCOME_QUERY_TEST(18, "18PublicSurfaceOnlyAcceptsFormalQueryInput")

#undef THROUGH_BALL_ONE_ON_ONE_CHIP_SHOT_OUTCOME_QUERY_TEST

#endif
