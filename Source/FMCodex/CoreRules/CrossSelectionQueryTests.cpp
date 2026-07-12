#if WITH_DEV_AUTOMATION_TESTS

#include "CrossSelectionQuery.h"

#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace CrossSelectionQueryTests
{
	FCrossSelectionQueryInput MakeInput(
		const ECrossIntentType IntentType,
		const int32 D6)
	{
		FCrossSelectionQueryInput Input;
		Input.IntendedCrossType = IntentType;
		Input.bHasExternalSelectionD6 = true;
		Input.ExternalSelectionD6 = D6;
		return Input;
	}

	bool AreInputsEqual(
		const FCrossSelectionQueryInput& Left,
		const FCrossSelectionQueryInput& Right)
	{
		return Left.IntendedCrossType == Right.IntendedCrossType
			&& Left.bHasExternalSelectionD6
				== Right.bHasExternalSelectionD6
			&& Left.ExternalSelectionD6 == Right.ExternalSelectionD6;
	}

	bool AreResultsEqual(
		const FCrossSelectionQueryResult& Left,
		const FCrossSelectionQueryResult& Right)
	{
		return Left.bSuccess == Right.bSuccess
			&& Left.bHasActualCrossType == Right.bHasActualCrossType
			&& Left.ActualCrossType == Right.ActualCrossType
			&& Left.ErrorCode == Right.ErrorCode
			&& Left.ErrorMessage == Right.ErrorMessage
			&& Left.InvalidField == Right.InvalidField
			&& AreInputsEqual(Left.Input, Right.Input);
	}

	bool ExpectSuccess(
		FAutomationTestBase& Test,
		const FCrossSelectionQueryInput& Input,
		const ECrossActualType ExpectedActualType)
	{
		const FCrossSelectionQueryResult Result =
			FCrossSelectionQuery::Select(Input);
		Test.TestTrue(TEXT("Selection succeeds"), Result.bSuccess);
		Test.TestTrue(
			TEXT("Success has an actual Cross type"),
			Result.bHasActualCrossType);
		Test.TestEqual(
			TEXT("Actual Cross type matches Canonical mapping"),
			Result.ActualCrossType,
			ExpectedActualType);
		Test.TestEqual(
			TEXT("Success has no error"),
			Result.ErrorCode,
			ECrossSelectionQueryErrorCode::None);
		Test.TestTrue(
			TEXT("Success error message is empty"),
			Result.ErrorMessage.IsEmpty());
		Test.TestTrue(
			TEXT("Success preserves the input"),
			AreInputsEqual(Result.Input, Input));
		return true;
	}

	bool ExpectFailure(
		FAutomationTestBase& Test,
		const FCrossSelectionQueryInput& Input,
		const ECrossSelectionQueryErrorCode ExpectedError,
		const FName ExpectedInvalidField)
	{
		const FCrossSelectionQueryResult Result =
			FCrossSelectionQuery::Select(Input);
		Test.TestFalse(TEXT("Selection fails"), Result.bSuccess);
		Test.TestFalse(
			TEXT("Failure has no consumable actual Cross type"),
			Result.bHasActualCrossType);
		Test.TestEqual(
			TEXT("Failure retains the default actual Cross type"),
			Result.ActualCrossType,
			ECrossActualType::None);
		Test.TestEqual(
			TEXT("Expected error is returned"),
			Result.ErrorCode,
			ExpectedError);
		Test.TestEqual(
			TEXT("Expected invalid field is returned"),
			Result.InvalidField,
			ExpectedInvalidField);
		Test.TestFalse(
			TEXT("Failure error message is not empty"),
			Result.ErrorMessage.IsEmpty());
		Test.TestTrue(
			TEXT("Failure preserves the input"),
			AreInputsEqual(Result.Input, Input));
		return true;
	}

	bool LoadQuerySource(FString& OutSource)
	{
		const FString Directory = FPaths::Combine(
			FPaths::ProjectDir(),
			TEXT("Source/FMCodex/CoreRules"));
		FString Header;
		FString Implementation;
		const bool bHeaderLoaded = FFileHelper::LoadFileToString(
			Header,
			*FPaths::Combine(Directory, TEXT("CrossSelectionQuery.h")));
		const bool bImplementationLoaded = FFileHelper::LoadFileToString(
			Implementation,
			*FPaths::Combine(Directory, TEXT("CrossSelectionQuery.cpp")));
		OutSource = Header + Implementation;
		return bHeaderLoaded && bImplementationLoaded;
	}
}

#define CROSS_SELECTION_TEST(ClassName, TestName) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
		ClassName, \
		"FMCodex.CoreRules.CrossSelectionQuery." TestName, \
		EAutomationTestFlags::EditorContext \
			| EAutomationTestFlags::EngineFilter)

#define CROSS_SELECTION_MAPPING_TEST( \
	ClassName, TestName, IntentType, D6, ActualType) \
	CROSS_SELECTION_TEST(ClassName, TestName) \
	bool ClassName::RunTest(const FString& Parameters) \
	{ \
		return CrossSelectionQueryTests::ExpectSuccess( \
			*this, \
			CrossSelectionQueryTests::MakeInput(IntentType, D6), \
			ActualType); \
	}

CROSS_SELECTION_MAPPING_TEST(
	FCrossSelectionHighD6OneTest,
	"HighIntentD6OneReturnsHigh",
	ECrossIntentType::High,
	1,
	ECrossActualType::High)

CROSS_SELECTION_MAPPING_TEST(
	FCrossSelectionHighD6TwoTest,
	"HighIntentD6TwoReturnsHigh",
	ECrossIntentType::High,
	2,
	ECrossActualType::High)

CROSS_SELECTION_MAPPING_TEST(
	FCrossSelectionHighD6ThreeTest,
	"HighIntentD6ThreeReturnsHigh",
	ECrossIntentType::High,
	3,
	ECrossActualType::High)

CROSS_SELECTION_MAPPING_TEST(
	FCrossSelectionHighD6FourTest,
	"HighIntentD6FourReturnsHigh",
	ECrossIntentType::High,
	4,
	ECrossActualType::High)

CROSS_SELECTION_MAPPING_TEST(
	FCrossSelectionHighD6FiveTest,
	"HighIntentD6FiveReturnsLow",
	ECrossIntentType::High,
	5,
	ECrossActualType::Low)

CROSS_SELECTION_MAPPING_TEST(
	FCrossSelectionHighD6SixTest,
	"HighIntentD6SixReturnsLow",
	ECrossIntentType::High,
	6,
	ECrossActualType::Low)

CROSS_SELECTION_MAPPING_TEST(
	FCrossSelectionLowD6OneTest,
	"LowIntentD6OneReturnsLow",
	ECrossIntentType::Low,
	1,
	ECrossActualType::Low)

CROSS_SELECTION_MAPPING_TEST(
	FCrossSelectionLowD6TwoTest,
	"LowIntentD6TwoReturnsLow",
	ECrossIntentType::Low,
	2,
	ECrossActualType::Low)

CROSS_SELECTION_MAPPING_TEST(
	FCrossSelectionLowD6ThreeTest,
	"LowIntentD6ThreeReturnsLow",
	ECrossIntentType::Low,
	3,
	ECrossActualType::Low)

CROSS_SELECTION_MAPPING_TEST(
	FCrossSelectionLowD6FourTest,
	"LowIntentD6FourReturnsLow",
	ECrossIntentType::Low,
	4,
	ECrossActualType::Low)

CROSS_SELECTION_MAPPING_TEST(
	FCrossSelectionLowD6FiveTest,
	"LowIntentD6FiveReturnsHigh",
	ECrossIntentType::Low,
	5,
	ECrossActualType::High)

CROSS_SELECTION_MAPPING_TEST(
	FCrossSelectionLowD6SixTest,
	"LowIntentD6SixReturnsHigh",
	ECrossIntentType::Low,
	6,
	ECrossActualType::High)

CROSS_SELECTION_TEST(
	FCrossSelectionRejectsMissingDefaultD6Test,
	"RejectsMissingSelectionD6WithDefaultValue")

bool FCrossSelectionRejectsMissingDefaultD6Test::RunTest(
	const FString& Parameters)
{
	FCrossSelectionQueryInput Input;
	Input.IntendedCrossType = ECrossIntentType::High;
	return CrossSelectionQueryTests::ExpectFailure(
		*this,
		Input,
		ECrossSelectionQueryErrorCode::MissingSelectionD6,
		TEXT("ExternalSelectionD6"));
}

CROSS_SELECTION_TEST(
	FCrossSelectionRejectsMissingValidD6Test,
	"RejectsMissingSelectionD6WithValidValue")

bool FCrossSelectionRejectsMissingValidD6Test::RunTest(
	const FString& Parameters)
{
	FCrossSelectionQueryInput Input;
	Input.IntendedCrossType = ECrossIntentType::Low;
	Input.ExternalSelectionD6 = 4;
	return CrossSelectionQueryTests::ExpectFailure(
		*this,
		Input,
		ECrossSelectionQueryErrorCode::MissingSelectionD6,
		TEXT("ExternalSelectionD6"));
}

CROSS_SELECTION_TEST(
	FCrossSelectionRejectsZeroD6Test,
	"RejectsSelectionD6Zero")

bool FCrossSelectionRejectsZeroD6Test::RunTest(const FString& Parameters)
{
	return CrossSelectionQueryTests::ExpectFailure(
		*this,
		CrossSelectionQueryTests::MakeInput(ECrossIntentType::High, 0),
		ECrossSelectionQueryErrorCode::InvalidSelectionD6,
		TEXT("ExternalSelectionD6"));
}

CROSS_SELECTION_TEST(
	FCrossSelectionRejectsSevenD6Test,
	"RejectsSelectionD6Seven")

bool FCrossSelectionRejectsSevenD6Test::RunTest(const FString& Parameters)
{
	return CrossSelectionQueryTests::ExpectFailure(
		*this,
		CrossSelectionQueryTests::MakeInput(ECrossIntentType::Low, 7),
		ECrossSelectionQueryErrorCode::InvalidSelectionD6,
		TEXT("ExternalSelectionD6"));
}

CROSS_SELECTION_TEST(
	FCrossSelectionRejectsNegativeD6Test,
	"RejectsNegativeSelectionD6")

bool FCrossSelectionRejectsNegativeD6Test::RunTest(
	const FString& Parameters)
{
	return CrossSelectionQueryTests::ExpectFailure(
		*this,
		CrossSelectionQueryTests::MakeInput(ECrossIntentType::High, -1),
		ECrossSelectionQueryErrorCode::InvalidSelectionD6,
		TEXT("ExternalSelectionD6"));
}

CROSS_SELECTION_TEST(
	FCrossSelectionRejectsLargeD6Test,
	"RejectsLargeSelectionD6")

bool FCrossSelectionRejectsLargeD6Test::RunTest(
	const FString& Parameters)
{
	return CrossSelectionQueryTests::ExpectFailure(
		*this,
		CrossSelectionQueryTests::MakeInput(
			ECrossIntentType::Low,
			TNumericLimits<int32>::Max()),
		ECrossSelectionQueryErrorCode::InvalidSelectionD6,
		TEXT("ExternalSelectionD6"));
}

CROSS_SELECTION_TEST(
	FCrossSelectionRejectsInvalidIntentTest,
	"RejectsInvalidIntendedCrossType")

bool FCrossSelectionRejectsInvalidIntentTest::RunTest(
	const FString& Parameters)
{
	FCrossSelectionQueryInput Input =
		CrossSelectionQueryTests::MakeInput(ECrossIntentType::High, 1);
	Input.IntendedCrossType = static_cast<ECrossIntentType>(255);
	return CrossSelectionQueryTests::ExpectFailure(
		*this,
		Input,
		ECrossSelectionQueryErrorCode::InvalidIntendedCrossType,
		TEXT("IntendedCrossType"));
}

CROSS_SELECTION_TEST(
	FCrossSelectionIntentValidationPriorityTest,
	"ValidatesIntentBeforeSelectionD6Presence")

bool FCrossSelectionIntentValidationPriorityTest::RunTest(
	const FString& Parameters)
{
	const FCrossSelectionQueryInput Input;
	return CrossSelectionQueryTests::ExpectFailure(
		*this,
		Input,
		ECrossSelectionQueryErrorCode::InvalidIntendedCrossType,
		TEXT("IntendedCrossType"));
}

CROSS_SELECTION_TEST(
	FCrossSelectionInputImmutabilityTest,
	"DoesNotMutateInput")

bool FCrossSelectionInputImmutabilityTest::RunTest(
	const FString& Parameters)
{
	const FCrossSelectionQueryInput Input =
		CrossSelectionQueryTests::MakeInput(ECrossIntentType::Low, 5);
	const FCrossSelectionQueryInput InputBefore = Input;
	const FCrossSelectionQueryResult Result =
		FCrossSelectionQuery::Select(Input);
	TestTrue(
		TEXT("Input remains unchanged"),
		CrossSelectionQueryTests::AreInputsEqual(Input, InputBefore));
	TestTrue(
		TEXT("Result preserves an input copy"),
		CrossSelectionQueryTests::AreInputsEqual(Result.Input, InputBefore));
	return true;
}

CROSS_SELECTION_TEST(
	FCrossSelectionDeterminismTest,
	"ProducesDeterministicResults")

bool FCrossSelectionDeterminismTest::RunTest(const FString& Parameters)
{
	const FCrossSelectionQueryInput Input =
		CrossSelectionQueryTests::MakeInput(ECrossIntentType::High, 5);
	const FCrossSelectionQueryResult FirstResult =
		FCrossSelectionQuery::Select(Input);
	const FCrossSelectionQueryResult SecondResult =
		FCrossSelectionQuery::Select(Input);
	TestTrue(
		TEXT("Repeated selection returns an identical result"),
		CrossSelectionQueryTests::AreResultsEqual(
			FirstResult,
			SecondResult));
	return true;
}

CROSS_SELECTION_TEST(
	FCrossSelectionForbiddenDependenciesTest,
	"HasNoForbiddenDependencies")

bool FCrossSelectionForbiddenDependenciesTest::RunTest(
	const FString& Parameters)
{
	FString Source;
	TestTrue(
		TEXT("Cross Selection source loads"),
		CrossSelectionQueryTests::LoadQuerySource(Source));
	const TArray<FString> ForbiddenTerms = {
		TEXT("SkillRuleSnapshot"),
		TEXT("PlayerCardRuleSnapshot"),
		TEXT("FormulaPlan"),
		TEXT("FormulaResolver"),
		TEXT("FormulaAttackFlow"),
		TEXT("MatchPlay"),
		TEXT("ExternalApi"),
		TEXT("SkillEffect"),
		TEXT("SkillPipeline"),
		TEXT("DataTable"),
		TEXT("Provider"),
		TEXT("FMath::Rand"),
		TEXT("FRandomStream"),
		TEXT("AttackD6"),
		TEXT("DefenseD6"),
		TEXT("Carrier"),
		TEXT("Runner"),
		TEXT("Marker"),
		TEXT("Helper"),
		TEXT("Goalkeeper")
	};
	for (const FString& Term : ForbiddenTerms)
	{
		TestFalse(
			TEXT("Forbidden dependency is absent"),
			Source.Contains(Term));
	}
	return true;
}

#undef CROSS_SELECTION_MAPPING_TEST
#undef CROSS_SELECTION_TEST

#endif
