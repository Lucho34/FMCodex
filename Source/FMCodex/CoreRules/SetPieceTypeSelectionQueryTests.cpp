#if WITH_DEV_AUTOMATION_TESTS

#include "SetPieceTypeSelectionQuery.h"

#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace SetPieceTypeSelectionQueryTests
{
	FSetPieceTypeSelectionQueryInput MakeInput(
		const int32 CurrentActionPoint,
		const int32 D6)
	{
		FSetPieceTypeSelectionQueryInput Input;
		Input.CurrentActionPoint = CurrentActionPoint;
		Input.bHasExternalSelectionD6 = true;
		Input.ExternalSelectionD6 = D6;
		return Input;
	}

	bool AreInputsEqual(
		const FSetPieceTypeSelectionQueryInput& Left,
		const FSetPieceTypeSelectionQueryInput& Right)
	{
		return Left.CurrentActionPoint == Right.CurrentActionPoint
			&& Left.bHasExternalSelectionD6
				== Right.bHasExternalSelectionD6
			&& Left.ExternalSelectionD6 == Right.ExternalSelectionD6;
	}

	bool AreResultsEqual(
		const FSetPieceTypeSelectionQueryResult& Left,
		const FSetPieceTypeSelectionQueryResult& Right)
	{
		return Left.bSuccess == Right.bSuccess
			&& Left.bHasSelectedSetPieceType
				== Right.bHasSelectedSetPieceType
			&& Left.SelectedSetPieceType == Right.SelectedSetPieceType
			&& Left.ErrorCode == Right.ErrorCode
			&& Left.ErrorMessage == Right.ErrorMessage
			&& Left.InvalidField == Right.InvalidField
			&& AreInputsEqual(Left.Input, Right.Input);
	}

	ESetPieceSelectedType ExpectedTypeForD6(const int32 D6)
	{
		switch (D6)
		{
		case 1:
		case 2:
			return ESetPieceSelectedType::Corner;
		case 3:
		case 4:
			return ESetPieceSelectedType::LongFreeKick;
		case 5:
			return ESetPieceSelectedType::ShortFreeKick;
		case 6:
			return ESetPieceSelectedType::Penalty;
		default:
			return ESetPieceSelectedType::None;
		}
	}

	bool ExpectSuccess(
		FAutomationTestBase& Test,
		const FSetPieceTypeSelectionQueryInput& Input,
		const ESetPieceSelectedType ExpectedType,
		const FString& Context = FString())
	{
		const FSetPieceTypeSelectionQueryResult Result =
			FSetPieceTypeSelectionQuery::Select(Input);
		Test.TestTrue(
			*FString::Printf(TEXT("%s selection succeeds"), *Context),
			Result.bSuccess);
		Test.TestTrue(
			*FString::Printf(TEXT("%s success has a selected type"), *Context),
			Result.bHasSelectedSetPieceType);
		Test.TestEqual(
			*FString::Printf(TEXT("%s selected type matches"), *Context),
			Result.SelectedSetPieceType,
			ExpectedType);
		Test.TestEqual(
			*FString::Printf(TEXT("%s success has no error"), *Context),
			Result.ErrorCode,
			ESetPieceTypeSelectionQueryErrorCode::None);
		Test.TestTrue(
			*FString::Printf(TEXT("%s success message is empty"), *Context),
			Result.ErrorMessage.IsEmpty());
		Test.TestEqual(
			*FString::Printf(TEXT("%s success has no invalid field"), *Context),
			Result.InvalidField,
			FName(NAME_None));
		Test.TestTrue(
			*FString::Printf(TEXT("%s success preserves input"), *Context),
			AreInputsEqual(Result.Input, Input));
		return true;
	}

	bool ExpectFailure(
		FAutomationTestBase& Test,
		const FSetPieceTypeSelectionQueryInput& Input,
		const ESetPieceTypeSelectionQueryErrorCode ExpectedError,
		const FName ExpectedInvalidField)
	{
		const FSetPieceTypeSelectionQueryResult Result =
			FSetPieceTypeSelectionQuery::Select(Input);
		Test.TestFalse(TEXT("Selection fails"), Result.bSuccess);
		Test.TestFalse(
			TEXT("Failure has no consumable selected type"),
			Result.bHasSelectedSetPieceType);
		Test.TestEqual(
			TEXT("Failure retains the default selected type"),
			Result.SelectedSetPieceType,
			ESetPieceSelectedType::None);
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
			TEXT("Failure preserves input"),
			AreInputsEqual(Result.Input, Input));
		return true;
	}

	bool ExpectAllMappingsForActionPoint(
		FAutomationTestBase& Test,
		const int32 CurrentActionPoint)
	{
		for (int32 D6 = 1; D6 <= 6; ++D6)
		{
			const FString Context = FString::Printf(
				TEXT("AP=%d D6=%d"),
				CurrentActionPoint,
				D6);
			ExpectSuccess(
				Test,
				MakeInput(CurrentActionPoint, D6),
				ExpectedTypeForD6(D6),
				Context);
		}
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
			*FPaths::Combine(
				Directory,
				TEXT("SetPieceTypeSelectionQuery.h")));
		const bool bImplementationLoaded = FFileHelper::LoadFileToString(
			Implementation,
			*FPaths::Combine(
				Directory,
				TEXT("SetPieceTypeSelectionQuery.cpp")));
		OutSource = Header + Implementation;
		return bHeaderLoaded && bImplementationLoaded;
	}
}

#define SET_PIECE_SELECTION_TEST(ClassName, TestName) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
		ClassName, \
		"FMCodex.CoreRules.SetPieceTypeSelectionQuery." TestName, \
		EAutomationTestFlags::EditorContext \
			| EAutomationTestFlags::EngineFilter)

#define SET_PIECE_MAPPING_TEST(ClassName, TestName, D6, SelectedType) \
	SET_PIECE_SELECTION_TEST(ClassName, TestName) \
	bool ClassName::RunTest(const FString& Parameters) \
	{ \
		return SetPieceTypeSelectionQueryTests::ExpectSuccess( \
			*this, \
			SetPieceTypeSelectionQueryTests::MakeInput(9, D6), \
			SelectedType, \
			TEXT("AP=9 D6=" #D6)); \
	}

SET_PIECE_MAPPING_TEST(
	FSetPieceSelectionD6OneTest,
	"D6OneReturnsCorner",
	1,
	ESetPieceSelectedType::Corner)

SET_PIECE_MAPPING_TEST(
	FSetPieceSelectionD6TwoTest,
	"D6TwoReturnsCorner",
	2,
	ESetPieceSelectedType::Corner)

SET_PIECE_MAPPING_TEST(
	FSetPieceSelectionD6ThreeTest,
	"D6ThreeReturnsLongFreeKick",
	3,
	ESetPieceSelectedType::LongFreeKick)

SET_PIECE_MAPPING_TEST(
	FSetPieceSelectionD6FourTest,
	"D6FourReturnsLongFreeKick",
	4,
	ESetPieceSelectedType::LongFreeKick)

SET_PIECE_MAPPING_TEST(
	FSetPieceSelectionD6FiveTest,
	"D6FiveReturnsShortFreeKick",
	5,
	ESetPieceSelectedType::ShortFreeKick)

SET_PIECE_MAPPING_TEST(
	FSetPieceSelectionD6SixTest,
	"D6SixReturnsPenalty",
	6,
	ESetPieceSelectedType::Penalty)

#define SET_PIECE_ACTION_POINT_TEST(ClassName, TestName, ActionPoint) \
	SET_PIECE_SELECTION_TEST(ClassName, TestName) \
	bool ClassName::RunTest(const FString& Parameters) \
	{ \
		return SetPieceTypeSelectionQueryTests:: \
			ExpectAllMappingsForActionPoint(*this, ActionPoint); \
	}

SET_PIECE_ACTION_POINT_TEST(
	FSetPieceSelectionActionPointNineTest,
	"ActionPointNineUsesCompleteMapping",
	9)

SET_PIECE_ACTION_POINT_TEST(
	FSetPieceSelectionActionPointTenTest,
	"ActionPointTenUsesCompleteMapping",
	10)

SET_PIECE_ACTION_POINT_TEST(
	FSetPieceSelectionActionPointElevenTest,
	"ActionPointElevenUsesCompleteMapping",
	11)

SET_PIECE_ACTION_POINT_TEST(
	FSetPieceSelectionActionPointTwelveTest,
	"ActionPointTwelveUsesCompleteMapping",
	12)

SET_PIECE_SELECTION_TEST(
	FSetPieceSelectionRejectsMissingDefaultD6Test,
	"RejectsMissingSelectionD6WithDefaultValue")

bool FSetPieceSelectionRejectsMissingDefaultD6Test::RunTest(
	const FString& Parameters)
{
	FSetPieceTypeSelectionQueryInput Input;
	Input.CurrentActionPoint = 9;
	return SetPieceTypeSelectionQueryTests::ExpectFailure(
		*this,
		Input,
		ESetPieceTypeSelectionQueryErrorCode::MissingSelectionD6,
		TEXT("ExternalSelectionD6"));
}

SET_PIECE_SELECTION_TEST(
	FSetPieceSelectionRejectsMissingValidD6Test,
	"RejectsMissingSelectionD6WithValidValue")

bool FSetPieceSelectionRejectsMissingValidD6Test::RunTest(
	const FString& Parameters)
{
	FSetPieceTypeSelectionQueryInput Input;
	Input.CurrentActionPoint = 9;
	Input.ExternalSelectionD6 = 4;
	return SetPieceTypeSelectionQueryTests::ExpectFailure(
		*this,
		Input,
		ESetPieceTypeSelectionQueryErrorCode::MissingSelectionD6,
		TEXT("ExternalSelectionD6"));
}

#define SET_PIECE_INVALID_D6_TEST(ClassName, TestName, D6) \
	SET_PIECE_SELECTION_TEST(ClassName, TestName) \
	bool ClassName::RunTest(const FString& Parameters) \
	{ \
		return SetPieceTypeSelectionQueryTests::ExpectFailure( \
			*this, \
			SetPieceTypeSelectionQueryTests::MakeInput(9, D6), \
			ESetPieceTypeSelectionQueryErrorCode::InvalidSelectionD6, \
			TEXT("ExternalSelectionD6")); \
	}

SET_PIECE_INVALID_D6_TEST(
	FSetPieceSelectionRejectsZeroD6Test,
	"RejectsSelectionD6Zero",
	0)

SET_PIECE_INVALID_D6_TEST(
	FSetPieceSelectionRejectsSevenD6Test,
	"RejectsSelectionD6Seven",
	7)

SET_PIECE_INVALID_D6_TEST(
	FSetPieceSelectionRejectsNegativeD6Test,
	"RejectsNegativeSelectionD6",
	-1)

SET_PIECE_SELECTION_TEST(
	FSetPieceSelectionRejectsLargeD6Test,
	"RejectsLargeSelectionD6")

bool FSetPieceSelectionRejectsLargeD6Test::RunTest(
	const FString& Parameters)
{
	return SetPieceTypeSelectionQueryTests::ExpectFailure(
		*this,
		SetPieceTypeSelectionQueryTests::MakeInput(
			9,
			TNumericLimits<int32>::Max()),
		ESetPieceTypeSelectionQueryErrorCode::InvalidSelectionD6,
		TEXT("ExternalSelectionD6"));
}

#define SET_PIECE_INVALID_ACTION_POINT_TEST( \
	ClassName, TestName, ActionPoint) \
	SET_PIECE_SELECTION_TEST(ClassName, TestName) \
	bool ClassName::RunTest(const FString& Parameters) \
	{ \
		return SetPieceTypeSelectionQueryTests::ExpectFailure( \
			*this, \
			SetPieceTypeSelectionQueryTests::MakeInput(ActionPoint, 1), \
			ESetPieceTypeSelectionQueryErrorCode:: \
				ActionPointNotEligibleForSetPiece, \
			TEXT("CurrentActionPoint")); \
	}

SET_PIECE_INVALID_ACTION_POINT_TEST(
	FSetPieceSelectionRejectsActionPointEightTest,
	"RejectsActionPointEight",
	8)

SET_PIECE_INVALID_ACTION_POINT_TEST(
	FSetPieceSelectionRejectsActionPointThirteenTest,
	"RejectsActionPointThirteen",
	13)

SET_PIECE_INVALID_ACTION_POINT_TEST(
	FSetPieceSelectionRejectsActionPointZeroTest,
	"RejectsActionPointZero",
	0)

SET_PIECE_INVALID_ACTION_POINT_TEST(
	FSetPieceSelectionRejectsNegativeActionPointTest,
	"RejectsNegativeActionPoint",
	-1)

SET_PIECE_SELECTION_TEST(
	FSetPieceSelectionRejectsLargeActionPointTest,
	"RejectsLargeActionPoint")

bool FSetPieceSelectionRejectsLargeActionPointTest::RunTest(
	const FString& Parameters)
{
	return SetPieceTypeSelectionQueryTests::ExpectFailure(
		*this,
		SetPieceTypeSelectionQueryTests::MakeInput(
			TNumericLimits<int32>::Max(),
			1),
		ESetPieceTypeSelectionQueryErrorCode::
			ActionPointNotEligibleForSetPiece,
		TEXT("CurrentActionPoint"));
}

SET_PIECE_SELECTION_TEST(
	FSetPieceSelectionActionPointBeforeMissingD6Test,
	"ValidatesActionPointBeforeMissingSelectionD6")

bool FSetPieceSelectionActionPointBeforeMissingD6Test::RunTest(
	const FString& Parameters)
{
	FSetPieceTypeSelectionQueryInput Input;
	Input.CurrentActionPoint = 8;
	return SetPieceTypeSelectionQueryTests::ExpectFailure(
		*this,
		Input,
		ESetPieceTypeSelectionQueryErrorCode::
			ActionPointNotEligibleForSetPiece,
		TEXT("CurrentActionPoint"));
}

SET_PIECE_SELECTION_TEST(
	FSetPieceSelectionActionPointBeforeInvalidD6Test,
	"ValidatesActionPointBeforeInvalidSelectionD6")

bool FSetPieceSelectionActionPointBeforeInvalidD6Test::RunTest(
	const FString& Parameters)
{
	return SetPieceTypeSelectionQueryTests::ExpectFailure(
		*this,
		SetPieceTypeSelectionQueryTests::MakeInput(13, 7),
		ESetPieceTypeSelectionQueryErrorCode::
			ActionPointNotEligibleForSetPiece,
		TEXT("CurrentActionPoint"));
}

SET_PIECE_SELECTION_TEST(
	FSetPieceSelectionMissingBeforeRangeTest,
	"ValidatesSelectionD6PresenceBeforeRange")

bool FSetPieceSelectionMissingBeforeRangeTest::RunTest(
	const FString& Parameters)
{
	FSetPieceTypeSelectionQueryInput Input;
	Input.CurrentActionPoint = 9;
	Input.ExternalSelectionD6 = 7;
	return SetPieceTypeSelectionQueryTests::ExpectFailure(
		*this,
		Input,
		ESetPieceTypeSelectionQueryErrorCode::MissingSelectionD6,
		TEXT("ExternalSelectionD6"));
}

SET_PIECE_SELECTION_TEST(
	FSetPieceSelectionValidActionPointInvalidD6Test,
	"ReturnsRangeErrorAfterValidActionPointAndPresence")

bool FSetPieceSelectionValidActionPointInvalidD6Test::RunTest(
	const FString& Parameters)
{
	return SetPieceTypeSelectionQueryTests::ExpectFailure(
		*this,
		SetPieceTypeSelectionQueryTests::MakeInput(9, 7),
		ESetPieceTypeSelectionQueryErrorCode::InvalidSelectionD6,
		TEXT("ExternalSelectionD6"));
}

SET_PIECE_SELECTION_TEST(
	FSetPieceSelectionDeterminismTest,
	"ProducesDeterministicResults")

bool FSetPieceSelectionDeterminismTest::RunTest(
	const FString& Parameters)
{
	const FSetPieceTypeSelectionQueryInput Input =
		SetPieceTypeSelectionQueryTests::MakeInput(11, 6);
	const FSetPieceTypeSelectionQueryResult FirstResult =
		FSetPieceTypeSelectionQuery::Select(Input);
	const FSetPieceTypeSelectionQueryResult SecondResult =
		FSetPieceTypeSelectionQuery::Select(Input);
	TestTrue(
		TEXT("Repeated selection returns an identical result"),
		SetPieceTypeSelectionQueryTests::AreResultsEqual(
			FirstResult,
			SecondResult));
	return true;
}

SET_PIECE_SELECTION_TEST(
	FSetPieceSelectionInputImmutabilityTest,
	"DoesNotMutateInput")

bool FSetPieceSelectionInputImmutabilityTest::RunTest(
	const FString& Parameters)
{
	const FSetPieceTypeSelectionQueryInput Input =
		SetPieceTypeSelectionQueryTests::MakeInput(12, 5);
	const FSetPieceTypeSelectionQueryInput InputBefore = Input;
	const FSetPieceTypeSelectionQueryResult Result =
		FSetPieceTypeSelectionQuery::Select(Input);
	TestTrue(
		TEXT("Input remains unchanged"),
		SetPieceTypeSelectionQueryTests::AreInputsEqual(
			Input,
			InputBefore));
	TestTrue(
		TEXT("Result preserves an input copy"),
		SetPieceTypeSelectionQueryTests::AreInputsEqual(
			Result.Input,
			InputBefore));
	return true;
}

SET_PIECE_SELECTION_TEST(
	FSetPieceSelectionForbiddenDependenciesTest,
	"HasNoForbiddenDependencies")

bool FSetPieceSelectionForbiddenDependenciesTest::RunTest(
	const FString& Parameters)
{
	FString Source;
	TestTrue(
		TEXT("Set Piece Type Selection source loads"),
		SetPieceTypeSelectionQueryTests::LoadQuerySource(Source));
	const TArray<FString> ForbiddenTerms = {
		TEXT("PlayerSnapshot"),
		TEXT("PlayerCardRuleSnapshot"),
		TEXT("SkillRuleSnapshot"),
		TEXT("FormulaPlan"),
		TEXT("SingleCardFormulaInputContract"),
		TEXT("FormulaResolver"),
		TEXT("FormulaAttackFlow"),
		TEXT("ResolutionExecutor"),
		TEXT("InputAssemblyQuery"),
		TEXT("MatchPlay"),
		TEXT("ExternalApi"),
		TEXT("ExternalAPI"),
		TEXT("FMath::Rand"),
		TEXT("FRandomStream"),
		TEXT("Random"),
		TEXT("RNG"),
		TEXT("Hand"),
		TEXT("Deck"),
		TEXT("Draw"),
		TEXT("Shuffle"),
		TEXT("DataTable"),
		TEXT("Provider"),
		TEXT("SkillPipeline"),
		TEXT("SkillEffect"),
		TEXT("GenericSelection"),
		TEXT("GenericBranch"),
		TEXT("GenericEligibility"),
		TEXT("GenericComposition")
	};
	for (const FString& Term : ForbiddenTerms)
	{
		TestFalse(
			*FString::Printf(
				TEXT("Forbidden dependency '%s' is absent"),
				*Term),
			Source.Contains(Term));
	}
	return true;
}

#undef SET_PIECE_INVALID_ACTION_POINT_TEST
#undef SET_PIECE_INVALID_D6_TEST
#undef SET_PIECE_ACTION_POINT_TEST
#undef SET_PIECE_MAPPING_TEST
#undef SET_PIECE_SELECTION_TEST

#endif
