#if WITH_DEV_AUTOMATION_TESTS

#include "ThroughBallBranchSelectionQuery.h"

#include "Misc/AutomationTest.h"

namespace ThroughBallBranchSelectionQueryTests
{
	FThroughBallBranchSelectionQueryInput MakeInput(const int32 D6)
	{
		FThroughBallBranchSelectionQueryInput Input;
		Input.bHasExternalSelectionD6 = true;
		Input.ExternalSelectionD6 = D6;
		return Input;
	}

	bool AreInputsEqual(
		const FThroughBallBranchSelectionQueryInput& Left,
		const FThroughBallBranchSelectionQueryInput& Right)
	{
		return Left.bHasExternalSelectionD6
				== Right.bHasExternalSelectionD6
			&& Left.ExternalSelectionD6 == Right.ExternalSelectionD6;
	}

	bool AreResultsEqual(
		const FThroughBallBranchSelectionQueryResult& Left,
		const FThroughBallBranchSelectionQueryResult& Right)
	{
		return Left.bSuccess == Right.bSuccess
			&& Left.bHasSelectedThroughBallBranch
				== Right.bHasSelectedThroughBallBranch
			&& Left.SelectedThroughBallBranch
				== Right.SelectedThroughBallBranch
			&& Left.ErrorCode == Right.ErrorCode
			&& Left.ErrorMessage == Right.ErrorMessage
			&& Left.InvalidField == Right.InvalidField
			&& AreInputsEqual(Left.Input, Right.Input);
	}

	bool ExpectSuccess(
		FAutomationTestBase& Test,
		const FThroughBallBranchSelectionQueryInput& Input,
		const EThroughBallSelectedBranch ExpectedBranch)
	{
		const FThroughBallBranchSelectionQueryResult Result =
			FThroughBallBranchSelectionQuery::Select(Input);
		Test.TestTrue(TEXT("Selection succeeds"), Result.bSuccess);
		Test.TestTrue(
			TEXT("Success has a selected Through Ball branch"),
			Result.bHasSelectedThroughBallBranch);
		Test.TestEqual(
			TEXT("Selected branch matches"),
			Result.SelectedThroughBallBranch,
			ExpectedBranch);
		Test.TestNotEqual(
			TEXT("Successful branch is not None"),
			Result.SelectedThroughBallBranch,
			EThroughBallSelectedBranch::None);
		Test.TestEqual(
			TEXT("Success has no error"),
			Result.ErrorCode,
			EThroughBallBranchSelectionQueryErrorCode::None);
		Test.TestTrue(
			TEXT("Success error message is empty"),
			Result.ErrorMessage.IsEmpty());
		Test.TestEqual(
			TEXT("Success has no invalid field"),
			Result.InvalidField,
			FName(NAME_None));
		Test.TestTrue(
			TEXT("Success preserves input"),
			AreInputsEqual(Result.Input, Input));
		return true;
	}

	bool ExpectFailure(
		FAutomationTestBase& Test,
		const FThroughBallBranchSelectionQueryInput& Input,
		const EThroughBallBranchSelectionQueryErrorCode ExpectedError)
	{
		const FThroughBallBranchSelectionQueryResult Result =
			FThroughBallBranchSelectionQuery::Select(Input);
		Test.TestFalse(TEXT("Selection fails"), Result.bSuccess);
		Test.TestFalse(
			TEXT("Failure has no consumable selected branch"),
			Result.bHasSelectedThroughBallBranch);
		Test.TestEqual(
			TEXT("Failure retains the default selected branch"),
			Result.SelectedThroughBallBranch,
			EThroughBallSelectedBranch::None);
		Test.TestEqual(
			TEXT("Expected error is returned"),
			Result.ErrorCode,
			ExpectedError);
		Test.TestFalse(
			TEXT("Failure error message is not empty"),
			Result.ErrorMessage.IsEmpty());
		Test.TestEqual(
			TEXT("ExternalSelectionD6 is the invalid field"),
			Result.InvalidField,
			FName(TEXT("ExternalSelectionD6")));
		Test.TestTrue(
			TEXT("Failure preserves input"),
			AreInputsEqual(Result.Input, Input));
		return true;
	}
}

#define THROUGH_BALL_SELECTION_TEST(ClassName, TestName) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
		ClassName, \
		"FMCodex.CoreRules.ThroughBallBranchSelectionQuery." TestName, \
		EAutomationTestFlags::EditorContext \
			| EAutomationTestFlags::EngineFilter)

#define THROUGH_BALL_MAPPING_TEST(ClassName, TestName, D6, Branch) \
	THROUGH_BALL_SELECTION_TEST(ClassName, TestName) \
	bool ClassName::RunTest(const FString& Parameters) \
	{ \
		return ThroughBallBranchSelectionQueryTests::ExpectSuccess( \
			*this, \
			ThroughBallBranchSelectionQueryTests::MakeInput(D6), \
			Branch); \
	}

THROUGH_BALL_MAPPING_TEST(
	FThroughBallSelectionD6OneTest,
	"D6OneReturnsFeet",
	1,
	EThroughBallSelectedBranch::Feet)

THROUGH_BALL_MAPPING_TEST(
	FThroughBallSelectionD6TwoTest,
	"D6TwoReturnsFeet",
	2,
	EThroughBallSelectedBranch::Feet)

THROUGH_BALL_MAPPING_TEST(
	FThroughBallSelectionD6ThreeTest,
	"D6ThreeReturnsBehindDefense",
	3,
	EThroughBallSelectedBranch::BehindDefense)

THROUGH_BALL_MAPPING_TEST(
	FThroughBallSelectionD6FourTest,
	"D6FourReturnsBehindDefense",
	4,
	EThroughBallSelectedBranch::BehindDefense)

THROUGH_BALL_MAPPING_TEST(
	FThroughBallSelectionD6FiveTest,
	"D6FiveReturnsAntiOffside",
	5,
	EThroughBallSelectedBranch::AntiOffside)

THROUGH_BALL_MAPPING_TEST(
	FThroughBallSelectionD6SixTest,
	"D6SixReturnsAntiOffside",
	6,
	EThroughBallSelectedBranch::AntiOffside)

THROUGH_BALL_SELECTION_TEST(
	FThroughBallSelectionMissingDefaultD6Test,
	"RejectsMissingSelectionD6WithDefaultValue")

bool FThroughBallSelectionMissingDefaultD6Test::RunTest(
	const FString& Parameters)
{
	FThroughBallBranchSelectionQueryInput Input;
	return ThroughBallBranchSelectionQueryTests::ExpectFailure(
		*this,
		Input,
		EThroughBallBranchSelectionQueryErrorCode::MissingSelectionD6);
}

THROUGH_BALL_SELECTION_TEST(
	FThroughBallSelectionMissingLegalD6Test,
	"RejectsMissingSelectionD6WithLegalValue")

bool FThroughBallSelectionMissingLegalD6Test::RunTest(
	const FString& Parameters)
{
	FThroughBallBranchSelectionQueryInput Input;
	Input.ExternalSelectionD6 = 3;
	return ThroughBallBranchSelectionQueryTests::ExpectFailure(
		*this,
		Input,
		EThroughBallBranchSelectionQueryErrorCode::MissingSelectionD6);
}

THROUGH_BALL_SELECTION_TEST(
	FThroughBallSelectionMissingInvalidD6Test,
	"ValidatesPresenceBeforeRange")

bool FThroughBallSelectionMissingInvalidD6Test::RunTest(
	const FString& Parameters)
{
	FThroughBallBranchSelectionQueryInput Input;
	Input.ExternalSelectionD6 = 99;
	return ThroughBallBranchSelectionQueryTests::ExpectFailure(
		*this,
		Input,
		EThroughBallBranchSelectionQueryErrorCode::MissingSelectionD6);
}

#define THROUGH_BALL_INVALID_D6_TEST(ClassName, TestName, D6) \
	THROUGH_BALL_SELECTION_TEST(ClassName, TestName) \
	bool ClassName::RunTest(const FString& Parameters) \
	{ \
		return ThroughBallBranchSelectionQueryTests::ExpectFailure( \
			*this, \
			ThroughBallBranchSelectionQueryTests::MakeInput(D6), \
			EThroughBallBranchSelectionQueryErrorCode::InvalidSelectionD6); \
	}

THROUGH_BALL_INVALID_D6_TEST(
	FThroughBallSelectionRejectsZeroTest,
	"RejectsSelectionD6Zero",
	0)

THROUGH_BALL_INVALID_D6_TEST(
	FThroughBallSelectionRejectsNegativeOneTest,
	"RejectsSelectionD6NegativeOne",
	-1)

THROUGH_BALL_INVALID_D6_TEST(
	FThroughBallSelectionRejectsMinimumIntTest,
	"RejectsSelectionD6MinimumInt",
	MIN_int32)

THROUGH_BALL_INVALID_D6_TEST(
	FThroughBallSelectionRejectsSevenTest,
	"RejectsSelectionD6Seven",
	7)

THROUGH_BALL_INVALID_D6_TEST(
	FThroughBallSelectionRejectsEightTest,
	"RejectsSelectionD6Eight",
	8)

THROUGH_BALL_INVALID_D6_TEST(
	FThroughBallSelectionRejectsMaximumIntTest,
	"RejectsSelectionD6MaximumInt",
	MAX_int32)

THROUGH_BALL_SELECTION_TEST(
	FThroughBallSelectionDeterminismTest,
	"ProducesDeterministicResults")

bool FThroughBallSelectionDeterminismTest::RunTest(
	const FString& Parameters)
{
	const FThroughBallBranchSelectionQueryInput Input =
		ThroughBallBranchSelectionQueryTests::MakeInput(5);
	const FThroughBallBranchSelectionQueryResult FirstResult =
		FThroughBallBranchSelectionQuery::Select(Input);
	const FThroughBallBranchSelectionQueryResult SecondResult =
		FThroughBallBranchSelectionQuery::Select(Input);
	TestTrue(
		TEXT("Repeated selection returns an identical result"),
		ThroughBallBranchSelectionQueryTests::AreResultsEqual(
			FirstResult,
			SecondResult));
	return true;
}

THROUGH_BALL_SELECTION_TEST(
	FThroughBallSelectionInputImmutabilityTest,
	"DoesNotMutateInput")

bool FThroughBallSelectionInputImmutabilityTest::RunTest(
	const FString& Parameters)
{
	const FThroughBallBranchSelectionQueryInput Input =
		ThroughBallBranchSelectionQueryTests::MakeInput(4);
	const FThroughBallBranchSelectionQueryInput InputBefore = Input;
	const FThroughBallBranchSelectionQueryResult Result =
		FThroughBallBranchSelectionQuery::Select(Input);
	TestTrue(
		TEXT("Input remains unchanged"),
		ThroughBallBranchSelectionQueryTests::AreInputsEqual(
			Input,
			InputBefore));
	TestTrue(
		TEXT("Result preserves an input copy"),
		ThroughBallBranchSelectionQueryTests::AreInputsEqual(
			Result.Input,
			InputBefore));
	return true;
}

THROUGH_BALL_SELECTION_TEST(
	FThroughBallSelectionBoundaryIsolationTest,
	"SucceedsWithOnlyExternalSelectionD6")

bool FThroughBallSelectionBoundaryIsolationTest::RunTest(
	const FString& Parameters)
{
	FThroughBallBranchSelectionQueryInput Input;
	Input.bHasExternalSelectionD6 = true;
	Input.ExternalSelectionD6 = 2;
	return ThroughBallBranchSelectionQueryTests::ExpectSuccess(
		*this,
		Input,
		EThroughBallSelectedBranch::Feet);
}

#undef THROUGH_BALL_INVALID_D6_TEST
#undef THROUGH_BALL_MAPPING_TEST
#undef THROUGH_BALL_SELECTION_TEST

#endif
