#include "SkillRuleSnapshotQuery.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace SkillRuleSnapshotQueryTests
{
	const FName PrimarySkillId(TEXT("Skill.LongShot.Primary"));
	const FName SecondarySkillId(TEXT("Skill.LongShot.Secondary"));
	const FName MissingSkillId(TEXT("Skill.LongShot.Missing"));
	const FName CrossSkillId(TEXT("Skill.Cross.Primary"));
	const FName MissingCrossSkillId(TEXT("Skill.Cross.Missing"));
	const FName ThroughBallSkillId(TEXT("Skill.ThroughBall.Primary"));
	const FName MissingThroughBallSkillId(TEXT("Skill.ThroughBall.Missing"));
	const FName CutInsideShotSkillId(TEXT("Skill.CutInsideShot.Primary"));
	const FName PassControlSkillId(TEXT("Skill.PassControl.Primary"));

	FSkillRuleSnapshot MakeLongShotRule(
		const FName SkillId,
		const int32 MinActionPoint,
		const int32 MaxActionPoint)
	{
		FSkillRuleSnapshot SkillRule;
		SkillRule.SkillId = SkillId;
		SkillRule.SkillType = ESkillRuleType::LongShot;
		SkillRule.MinTriggerActionPoint = MinActionPoint;
		SkillRule.MaxTriggerActionPoint = MaxActionPoint;
		return SkillRule;
	}

	FSkillRuleSnapshot MakeCrossRule(
		const FName SkillId,
		const int32 MinActionPoint,
		const int32 MaxActionPoint)
	{
		FSkillRuleSnapshot SkillRule;
		SkillRule.SkillId = SkillId;
		SkillRule.SkillType = ESkillRuleType::Cross;
		SkillRule.MinTriggerActionPoint = MinActionPoint;
		SkillRule.MaxTriggerActionPoint = MaxActionPoint;
		return SkillRule;
	}

	FSkillRuleSnapshot MakeRule(
		const FName SkillId,
		const ESkillRuleType SkillType,
		const int32 MinActionPoint,
		const int32 MaxActionPoint)
	{
		FSkillRuleSnapshot SkillRule;
		SkillRule.SkillId = SkillId;
		SkillRule.SkillType = SkillType;
		SkillRule.MinTriggerActionPoint = MinActionPoint;
		SkillRule.MaxTriggerActionPoint = MaxActionPoint;
		return SkillRule;
	}

	FSkillRuleSnapshot MakeThroughBallRule(
		const FName SkillId = ThroughBallSkillId,
		const int32 MinActionPoint = 2,
		const int32 MaxActionPoint = 8)
	{
		return MakeRule(
			SkillId,
			ESkillRuleType::ThroughBall,
			MinActionPoint,
			MaxActionPoint);
	}

	FSkillRuleSnapshotSet MakeValidSnapshotSet()
	{
		FSkillRuleSnapshotSet SnapshotSet;
		SnapshotSet.SkillRules =
		{
			MakeLongShotRule(PrimarySkillId, 2, 5),
			MakeLongShotRule(SecondarySkillId, 6, 8),
			MakeCrossRule(CrossSkillId, 3, 6),
			MakeThroughBallRule()
		};
		return SnapshotSet;
	}

	bool AreSkillRulesEqual(
		const FSkillRuleSnapshot& Left,
		const FSkillRuleSnapshot& Right)
	{
		return Left.SkillId == Right.SkillId
			&& Left.SkillType == Right.SkillType
			&& Left.MinTriggerActionPoint
				== Right.MinTriggerActionPoint
			&& Left.MaxTriggerActionPoint
				== Right.MaxTriggerActionPoint;
	}

	bool AreSnapshotSetsEqual(
		const FSkillRuleSnapshotSet& Left,
		const FSkillRuleSnapshotSet& Right)
	{
		if (Left.SkillRules.Num() != Right.SkillRules.Num())
		{
			return false;
		}

		for (int32 Index = 0; Index < Left.SkillRules.Num(); ++Index)
		{
			if (!AreSkillRulesEqual(
					Left.SkillRules[Index],
					Right.SkillRules[Index]))
			{
				return false;
			}
		}
		return true;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSkillRuleSnapshotQueryFindsExistingTest,
	"FMCodex.CoreRules.SkillRuleSnapshotQuery.FindsExistingSkillRuleBySkillId",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSkillRuleSnapshotQueryFindsExistingTest::RunTest(
	const FString& Parameters)
{
	FSkillRuleSnapshotQueryInput Input;
	Input.SkillId = SkillRuleSnapshotQueryTests::SecondarySkillId;

	const FSkillRuleSnapshotQueryResult Result =
		FSkillRuleSnapshotQuery::FindBySkillId(
			SkillRuleSnapshotQueryTests::MakeValidSnapshotSet(),
			Input);

	TestTrue(TEXT("Query succeeds"), Result.bSuccess);
	TestTrue(TEXT("Skill Rule is found"), Result.bFound);
	TestEqual(
		TEXT("Query has no error"),
		Result.ErrorCode,
		ESkillRuleSnapshotQueryErrorCode::None);
	TestEqual(
		TEXT("Requested SkillId is returned"),
		Result.Snapshot.SkillId,
		Input.SkillId);
	TestEqual(
		TEXT("SkillType is copied"),
		Result.Snapshot.SkillType,
		ESkillRuleType::LongShot);
	TestEqual(
		TEXT("Minimum action point is copied"),
		Result.Snapshot.MinTriggerActionPoint,
		6);
	TestEqual(
		TEXT("Maximum action point is copied"),
		Result.Snapshot.MaxTriggerActionPoint,
		8);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSkillRuleSnapshotQueryFindsCrossTest,
	"FMCodex.CoreRules.SkillRuleSnapshotQuery.FindsCrossSkillRuleBySkillId",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSkillRuleSnapshotQueryFindsCrossTest::RunTest(
	const FString& Parameters)
{
	FSkillRuleSnapshotQueryInput Input;
	Input.SkillId = SkillRuleSnapshotQueryTests::CrossSkillId;

	const FSkillRuleSnapshotQueryResult Result =
		FSkillRuleSnapshotQuery::FindBySkillId(
			SkillRuleSnapshotQueryTests::MakeValidSnapshotSet(),
			Input);

	TestTrue(TEXT("Cross query succeeds"), Result.bSuccess);
	TestTrue(TEXT("Cross Skill Rule is found"), Result.bFound);
	TestEqual(
		TEXT("Cross query has no error"),
		Result.ErrorCode,
		ESkillRuleSnapshotQueryErrorCode::None);
	TestEqual(
		TEXT("Cross SkillId is returned"),
		Result.Snapshot.SkillId,
		SkillRuleSnapshotQueryTests::CrossSkillId);
	TestEqual(
		TEXT("Cross SkillType is returned"),
		Result.Snapshot.SkillType,
		ESkillRuleType::Cross);
	TestEqual(
		TEXT("Cross minimum action point is preserved"),
		Result.Snapshot.MinTriggerActionPoint,
		3);
	TestEqual(
		TEXT("Cross maximum action point is preserved"),
		Result.Snapshot.MaxTriggerActionPoint,
		6);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSkillRuleSnapshotQueryFindsThroughBallTest,
	"FMCodex.CoreRules.SkillRuleSnapshotQuery.FindsThroughBallSkillRuleBySkillId",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSkillRuleSnapshotQueryFindsThroughBallTest::RunTest(
	const FString& Parameters)
{
	FSkillRuleSnapshotSet SnapshotSet;
	SnapshotSet.SkillRules =
	{
		SkillRuleSnapshotQueryTests::MakeThroughBallRule(
			SkillRuleSnapshotQueryTests::ThroughBallSkillId,
			3,
			7)
	};
	const FSkillRuleSnapshotSet SnapshotSetBefore = SnapshotSet;
	const FSkillRuleSnapshotQueryInput Input =
		{ SkillRuleSnapshotQueryTests::ThroughBallSkillId };
	const FSkillRuleSnapshotQueryInput InputBefore = Input;

	const FSkillRuleSnapshotQueryResult Result =
		FSkillRuleSnapshotQuery::FindBySkillId(SnapshotSet, Input);

	TestTrue(TEXT("ThroughBall query succeeds"), Result.bSuccess);
	TestTrue(TEXT("ThroughBall Skill Rule is found"), Result.bFound);
	TestEqual(
		TEXT("ThroughBall query has no error"),
		Result.ErrorCode,
		ESkillRuleSnapshotQueryErrorCode::None);
	TestTrue(
		TEXT("ThroughBall query has no error message"),
		Result.ErrorMessage.IsEmpty());
	TestTrue(
		TEXT("ThroughBall query has no invalid field"),
		Result.InvalidField.IsNone());
	TestEqual(
		TEXT("ThroughBall SkillId is copied"),
		Result.Snapshot.SkillId,
		SkillRuleSnapshotQueryTests::ThroughBallSkillId);
	TestEqual(
		TEXT("ThroughBall SkillType is copied"),
		Result.Snapshot.SkillType,
		ESkillRuleType::ThroughBall);
	TestEqual(
		TEXT("ThroughBall minimum action point is copied"),
		Result.Snapshot.MinTriggerActionPoint,
		3);
	TestEqual(
		TEXT("ThroughBall maximum action point is copied"),
		Result.Snapshot.MaxTriggerActionPoint,
		7);
	TestTrue(
		TEXT("ThroughBall query does not mutate the Snapshot Set"),
		SkillRuleSnapshotQueryTests::AreSnapshotSetsEqual(
			SnapshotSet,
			SnapshotSetBefore));
	TestEqual(
		TEXT("ThroughBall query does not mutate its Input"),
		Input.SkillId,
		InputBefore.SkillId);

	SnapshotSet.SkillRules[0].MinTriggerActionPoint = 8;
	TestEqual(
		TEXT("ThroughBall query returns a Snapshot value copy"),
		Result.Snapshot.MinTriggerActionPoint,
		3);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSkillRuleSnapshotQueryMissingThroughBallTest,
	"FMCodex.CoreRules.SkillRuleSnapshotQuery.ReturnsNotFoundForMissingThroughBallSkillId",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSkillRuleSnapshotQueryMissingThroughBallTest::RunTest(
	const FString& Parameters)
{
	FSkillRuleSnapshotSet SnapshotSet;
	SnapshotSet.SkillRules =
	{
		SkillRuleSnapshotQueryTests::MakeThroughBallRule()
	};
	FSkillRuleSnapshotQueryInput Input;
	Input.SkillId =
		SkillRuleSnapshotQueryTests::MissingThroughBallSkillId;

	const FSkillRuleSnapshotQueryResult Result =
		FSkillRuleSnapshotQuery::FindBySkillId(SnapshotSet, Input);

	TestFalse(TEXT("Missing ThroughBall query fails"), Result.bSuccess);
	TestFalse(TEXT("Missing ThroughBall is not found"), Result.bFound);
	TestEqual(
		TEXT("Missing ThroughBall uses the existing not-found error"),
		Result.ErrorCode,
		ESkillRuleSnapshotQueryErrorCode::SkillRuleNotFound);
	TestEqual(
		TEXT("Missing ThroughBall SkillId is retained"),
		Result.InvalidSkillId,
		Input.SkillId);
	TestTrue(
		TEXT("Valid ThroughBall set passes validation before lookup"),
		Result.ValidationResult.bSuccess);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSkillRuleSnapshotQueryInvalidThroughBallSnapshotTest,
	"FMCodex.CoreRules.SkillRuleSnapshotQuery.RejectsInvalidThroughBallSnapshotSet",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSkillRuleSnapshotQueryInvalidThroughBallSnapshotTest::RunTest(
	const FString& Parameters)
{
	FSkillRuleSnapshotSet SnapshotSet;
	SnapshotSet.SkillRules =
	{
		SkillRuleSnapshotQueryTests::MakeThroughBallRule(NAME_None)
	};
	const FSkillRuleSnapshotSet SnapshotSetBefore = SnapshotSet;
	const FSkillRuleSnapshotQueryInput Input =
		{ SkillRuleSnapshotQueryTests::ThroughBallSkillId };

	const FSkillRuleSnapshotQueryResult Result =
		FSkillRuleSnapshotQuery::FindBySkillId(SnapshotSet, Input);

	TestFalse(TEXT("Invalid ThroughBall set query fails"), Result.bSuccess);
	TestFalse(TEXT("Invalid ThroughBall is not found"), Result.bFound);
	TestEqual(
		TEXT("Invalid ThroughBall set uses the existing query error"),
		Result.ErrorCode,
		ESkillRuleSnapshotQueryErrorCode::SnapshotSetValidationFailed);
	TestEqual(
		TEXT("Invalid ThroughBall retains the validator error"),
		Result.ValidationResult.ErrorCode,
		ESkillRuleSnapshotValidationErrorCode::InvalidSkillId);
	TestTrue(
		TEXT("Invalid ThroughBall failure keeps the default Snapshot"),
		Result.Snapshot.SkillId.IsNone());
	TestEqual(
		TEXT("Invalid ThroughBall failure keeps the default type"),
		Result.Snapshot.SkillType,
		ESkillRuleType::None);
	TestTrue(
		TEXT("Invalid ThroughBall query does not mutate the Snapshot Set"),
		SkillRuleSnapshotQueryTests::AreSnapshotSetsEqual(
			SnapshotSet,
			SnapshotSetBefore));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSkillRuleSnapshotQueryMixedTypesFindsThroughBallTest,
	"FMCodex.CoreRules.SkillRuleSnapshotQuery.FindsThroughBallAmongAllSupportedTypes",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSkillRuleSnapshotQueryMixedTypesFindsThroughBallTest::RunTest(
	const FString& Parameters)
{
	FSkillRuleSnapshotSet SnapshotSet;
	SnapshotSet.SkillRules =
	{
		SkillRuleSnapshotQueryTests::MakeLongShotRule(
			SkillRuleSnapshotQueryTests::PrimarySkillId,
			2,
			5),
		SkillRuleSnapshotQueryTests::MakeRule(
			SkillRuleSnapshotQueryTests::CutInsideShotSkillId,
			ESkillRuleType::CutInsideShot,
			3,
			6),
		SkillRuleSnapshotQueryTests::MakeRule(
			SkillRuleSnapshotQueryTests::PassControlSkillId,
			ESkillRuleType::PassControl,
			4,
			7),
		SkillRuleSnapshotQueryTests::MakeCrossRule(
			SkillRuleSnapshotQueryTests::CrossSkillId,
			3,
			6),
		SkillRuleSnapshotQueryTests::MakeThroughBallRule()
	};
	const FSkillRuleSnapshotSet SnapshotSetBefore = SnapshotSet;
	const FSkillRuleSnapshotQueryInput Input =
		{ SkillRuleSnapshotQueryTests::ThroughBallSkillId };
	const FSkillRuleSnapshotQueryInput InputBefore = Input;

	const FSkillRuleSnapshotQueryResult Result =
		FSkillRuleSnapshotQuery::FindBySkillId(SnapshotSet, Input);

	TestTrue(TEXT("Mixed-type ThroughBall query succeeds"), Result.bSuccess);
	TestTrue(TEXT("Mixed-type ThroughBall is found"), Result.bFound);
	TestEqual(
		TEXT("Mixed-type query returns ThroughBall"),
		Result.Snapshot.SkillType,
		ESkillRuleType::ThroughBall);
	TestTrue(
		TEXT("Mixed-type query does not mutate the Snapshot Set"),
		SkillRuleSnapshotQueryTests::AreSnapshotSetsEqual(
			SnapshotSet,
			SnapshotSetBefore));
	TestEqual(
		TEXT("Mixed-type query does not mutate its Input"),
		Input.SkillId,
		InputBefore.SkillId);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSkillRuleSnapshotQueryDuplicateThroughBallSkillIdTest,
	"FMCodex.CoreRules.SkillRuleSnapshotQuery.RejectsDuplicateThroughBallSkillIdBeforeLookup",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSkillRuleSnapshotQueryDuplicateThroughBallSkillIdTest::RunTest(
	const FString& Parameters)
{
	FSkillRuleSnapshotSet SnapshotSet;
	SnapshotSet.SkillRules =
	{
		SkillRuleSnapshotQueryTests::MakeLongShotRule(
			SkillRuleSnapshotQueryTests::ThroughBallSkillId,
			2,
			5),
		SkillRuleSnapshotQueryTests::MakeThroughBallRule()
	};
	const FSkillRuleSnapshotQueryInput Input =
		{ SkillRuleSnapshotQueryTests::ThroughBallSkillId };

	const FSkillRuleSnapshotQueryResult Result =
		FSkillRuleSnapshotQuery::FindBySkillId(SnapshotSet, Input);

	TestFalse(TEXT("Duplicate ThroughBall query fails"), Result.bSuccess);
	TestFalse(TEXT("Duplicate ThroughBall is not found"), Result.bFound);
	TestEqual(
		TEXT("Duplicate ThroughBall set fails before lookup"),
		Result.ErrorCode,
		ESkillRuleSnapshotQueryErrorCode::SnapshotSetValidationFailed);
	TestEqual(
		TEXT("Duplicate ThroughBall retains the validator error"),
		Result.ValidationResult.ErrorCode,
		ESkillRuleSnapshotValidationErrorCode::DuplicateSkillId);
	TestEqual(
		TEXT("Duplicate ThroughBall retains the invalid SkillId"),
		Result.InvalidSkillId,
		SkillRuleSnapshotQueryTests::ThroughBallSkillId);
	TestTrue(
		TEXT("Duplicate ThroughBall failure returns no Snapshot"),
		Result.Snapshot.SkillId.IsNone());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSkillRuleSnapshotQueryEmptySkillIdTest,
	"FMCodex.CoreRules.SkillRuleSnapshotQuery.RejectsEmptyQuerySkillId",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSkillRuleSnapshotQueryEmptySkillIdTest::RunTest(
	const FString& Parameters)
{
	const FSkillRuleSnapshotQueryInput Input;
	const FSkillRuleSnapshotQueryResult Result =
		FSkillRuleSnapshotQuery::FindBySkillId(
			SkillRuleSnapshotQueryTests::MakeValidSnapshotSet(),
			Input);

	TestFalse(TEXT("Empty SkillId query fails"), Result.bSuccess);
	TestFalse(TEXT("Empty SkillId is not found"), Result.bFound);
	TestEqual(
		TEXT("Invalid SkillId error is structured"),
		Result.ErrorCode,
		ESkillRuleSnapshotQueryErrorCode::InvalidSkillId);
	TestTrue(
		TEXT("Invalid SkillId remains None"),
		Result.InvalidSkillId.IsNone());
	TestEqual(
		TEXT("SkillId field is reported"),
		Result.InvalidField,
		FName(TEXT("SkillId")));
	TestFalse(TEXT("Error message is not empty"), Result.ErrorMessage.IsEmpty());
	TestFalse(
		TEXT("Snapshot Set validation does not run"),
		Result.ValidationResult.bSuccess);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSkillRuleSnapshotQueryValidationFailureTest,
	"FMCodex.CoreRules.SkillRuleSnapshotQuery.PropagatesSnapshotSetValidationFailure",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSkillRuleSnapshotQueryValidationFailureTest::RunTest(
	const FString& Parameters)
{
	FSkillRuleSnapshotSet SnapshotSet =
		SkillRuleSnapshotQueryTests::MakeValidSnapshotSet();
	SnapshotSet.SkillRules[1].SkillId =
		SkillRuleSnapshotQueryTests::PrimarySkillId;
	FSkillRuleSnapshotQueryInput Input;
	Input.SkillId = SkillRuleSnapshotQueryTests::PrimarySkillId;

	const FSkillRuleSnapshotQueryResult Result =
		FSkillRuleSnapshotQuery::FindBySkillId(
			SnapshotSet,
			Input);

	TestFalse(TEXT("Invalid Snapshot Set fails"), Result.bSuccess);
	TestFalse(TEXT("Ambiguous Skill Rule is not found"), Result.bFound);
	TestEqual(
		TEXT("Snapshot Set failure is structured"),
		Result.ErrorCode,
		ESkillRuleSnapshotQueryErrorCode
			::SnapshotSetValidationFailed);
	TestEqual(
		TEXT("Validator error code is retained"),
		Result.ValidationResult.ErrorCode,
		ESkillRuleSnapshotValidationErrorCode::DuplicateSkillId);
	TestEqual(
		TEXT("Validator error message is retained"),
		Result.ErrorMessage,
		Result.ValidationResult.ErrorMessage);
	TestEqual(
		TEXT("Validator invalid SkillId is retained"),
		Result.InvalidSkillId,
		Result.ValidationResult.InvalidSkillId);
	TestEqual(
		TEXT("Validator invalid field is retained"),
		Result.InvalidField,
		Result.ValidationResult.InvalidField);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSkillRuleSnapshotQueryCrossValidationFailureTest,
	"FMCodex.CoreRules.SkillRuleSnapshotQuery.PropagatesValidationFailureWhenQueryingCross",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSkillRuleSnapshotQueryCrossValidationFailureTest::RunTest(
	const FString& Parameters)
{
	FSkillRuleSnapshotSet SnapshotSet =
		SkillRuleSnapshotQueryTests::MakeValidSnapshotSet();
	SnapshotSet.SkillRules[1].SkillId =
		SkillRuleSnapshotQueryTests::PrimarySkillId;
	FSkillRuleSnapshotQueryInput Input;
	Input.SkillId = SkillRuleSnapshotQueryTests::CrossSkillId;

	const FSkillRuleSnapshotQueryResult Result =
		FSkillRuleSnapshotQuery::FindBySkillId(SnapshotSet, Input);

	TestFalse(TEXT("Invalid Cross Snapshot Set query fails"), Result.bSuccess);
	TestFalse(TEXT("Cross is not found when set validation fails"), Result.bFound);
	TestEqual(
		TEXT("Snapshot Set failure is structured for Cross"),
		Result.ErrorCode,
		ESkillRuleSnapshotQueryErrorCode
			::SnapshotSetValidationFailed);
	TestEqual(
		TEXT("Cross query retains the validator error"),
		Result.ValidationResult.ErrorCode,
		ESkillRuleSnapshotValidationErrorCode::DuplicateSkillId);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSkillRuleSnapshotQueryMissingSkillTest,
	"FMCodex.CoreRules.SkillRuleSnapshotQuery.ReturnsNotFoundForMissingSkillId",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSkillRuleSnapshotQueryMissingSkillTest::RunTest(
	const FString& Parameters)
{
	FSkillRuleSnapshotQueryInput Input;
	Input.SkillId = SkillRuleSnapshotQueryTests::MissingSkillId;

	const FSkillRuleSnapshotQueryResult Result =
		FSkillRuleSnapshotQuery::FindBySkillId(
			SkillRuleSnapshotQueryTests::MakeValidSnapshotSet(),
			Input);

	TestFalse(TEXT("Missing Skill Rule query fails"), Result.bSuccess);
	TestFalse(TEXT("Missing Skill Rule is not found"), Result.bFound);
	TestEqual(
		TEXT("Not-found error is structured"),
		Result.ErrorCode,
		ESkillRuleSnapshotQueryErrorCode::SkillRuleNotFound);
	TestEqual(
		TEXT("Missing SkillId is retained"),
		Result.InvalidSkillId,
		Input.SkillId);
	TestEqual(
		TEXT("SkillId field is reported"),
		Result.InvalidField,
		FName(TEXT("SkillId")));
	TestFalse(TEXT("Error message is not empty"), Result.ErrorMessage.IsEmpty());
	TestTrue(
		TEXT("Snapshot Set validation succeeds before lookup"),
		Result.ValidationResult.bSuccess);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSkillRuleSnapshotQueryMissingCrossSkillTest,
	"FMCodex.CoreRules.SkillRuleSnapshotQuery.ReturnsNotFoundForMissingCrossSkillId",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSkillRuleSnapshotQueryMissingCrossSkillTest::RunTest(
	const FString& Parameters)
{
	FSkillRuleSnapshotQueryInput Input;
	Input.SkillId = SkillRuleSnapshotQueryTests::MissingCrossSkillId;

	const FSkillRuleSnapshotQueryResult Result =
		FSkillRuleSnapshotQuery::FindBySkillId(
			SkillRuleSnapshotQueryTests::MakeValidSnapshotSet(),
			Input);

	TestFalse(TEXT("Missing Cross Skill Rule query fails"), Result.bSuccess);
	TestFalse(TEXT("Missing Cross Skill Rule is not found"), Result.bFound);
	TestEqual(
		TEXT("Missing Cross uses the existing not-found error"),
		Result.ErrorCode,
		ESkillRuleSnapshotQueryErrorCode::SkillRuleNotFound);
	TestEqual(
		TEXT("Missing Cross SkillId is retained"),
		Result.InvalidSkillId,
		Input.SkillId);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSkillRuleSnapshotQueryCrossCoexistsWithLongShotTest,
	"FMCodex.CoreRules.SkillRuleSnapshotQuery.CrossAndLongShotSnapshotsCoexist",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSkillRuleSnapshotQueryCrossCoexistsWithLongShotTest::RunTest(
	const FString& Parameters)
{
	const FSkillRuleSnapshotSet SnapshotSet =
		SkillRuleSnapshotQueryTests::MakeValidSnapshotSet();
	const FSkillRuleSnapshotQueryInput CrossInput =
		{ SkillRuleSnapshotQueryTests::CrossSkillId };
	const FSkillRuleSnapshotQueryInput LongShotInput =
		{ SkillRuleSnapshotQueryTests::PrimarySkillId };

	const FSkillRuleSnapshotQueryResult CrossResult =
		FSkillRuleSnapshotQuery::FindBySkillId(SnapshotSet, CrossInput);
	const FSkillRuleSnapshotQueryResult LongShotResult =
		FSkillRuleSnapshotQuery::FindBySkillId(SnapshotSet, LongShotInput);

	TestTrue(TEXT("Cross succeeds beside LongShot"), CrossResult.bSuccess);
	TestEqual(
		TEXT("Cross retains its type beside LongShot"),
		CrossResult.Snapshot.SkillType,
		ESkillRuleType::Cross);
	TestTrue(TEXT("LongShot succeeds beside Cross"), LongShotResult.bSuccess);
	TestEqual(
		TEXT("LongShot retains its type beside Cross"),
		LongShotResult.Snapshot.SkillType,
		ESkillRuleType::LongShot);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSkillRuleSnapshotQueryPreservesValidationTest,
	"FMCodex.CoreRules.SkillRuleSnapshotQuery.PreservesValidationResultOnSuccess",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSkillRuleSnapshotQueryPreservesValidationTest::RunTest(
	const FString& Parameters)
{
	FSkillRuleSnapshotQueryInput Input;
	Input.SkillId = SkillRuleSnapshotQueryTests::CrossSkillId;

	const FSkillRuleSnapshotQueryResult Result =
		FSkillRuleSnapshotQuery::FindBySkillId(
			SkillRuleSnapshotQueryTests::MakeValidSnapshotSet(),
			Input);

	TestTrue(TEXT("Query succeeds"), Result.bSuccess);
	TestTrue(
		TEXT("Validation success is retained"),
		Result.ValidationResult.bSuccess);
	TestTrue(
		TEXT("Validation validity is retained"),
		Result.ValidationResult.bIsValid);
	TestEqual(
		TEXT("Validation has no error"),
		Result.ValidationResult.ErrorCode,
		ESkillRuleSnapshotValidationErrorCode::None);
	TestTrue(
		TEXT("Validation error message remains empty"),
		Result.ValidationResult.ErrorMessage.IsEmpty());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSkillRuleSnapshotQuerySnapshotInputUnchangedTest,
	"FMCodex.CoreRules.SkillRuleSnapshotQuery.DoesNotMutateInputSnapshotSet",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSkillRuleSnapshotQuerySnapshotInputUnchangedTest::RunTest(
	const FString& Parameters)
{
	const FSkillRuleSnapshotSet SnapshotSet =
		SkillRuleSnapshotQueryTests::MakeValidSnapshotSet();
	const FSkillRuleSnapshotSet SnapshotSetBefore = SnapshotSet;
	FSkillRuleSnapshotQueryInput Input;
	Input.SkillId = SkillRuleSnapshotQueryTests::PrimarySkillId;

	const FSkillRuleSnapshotQueryResult Result =
		FSkillRuleSnapshotQuery::FindBySkillId(
			SnapshotSet,
			Input);

	TestTrue(TEXT("Query succeeds"), Result.bSuccess);
	TestTrue(
		TEXT("Snapshot Set remains unchanged"),
		SkillRuleSnapshotQueryTests::AreSnapshotSetsEqual(
			SnapshotSet,
			SnapshotSetBefore));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSkillRuleSnapshotQueryQueryInputUnchangedTest,
	"FMCodex.CoreRules.SkillRuleSnapshotQuery.DoesNotMutateQueryInput",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSkillRuleSnapshotQueryQueryInputUnchangedTest::RunTest(
	const FString& Parameters)
{
	const FSkillRuleSnapshotQueryInput Input =
		{ SkillRuleSnapshotQueryTests::CrossSkillId };
	const FSkillRuleSnapshotQueryInput InputBefore = Input;

	const FSkillRuleSnapshotQueryResult Result =
		FSkillRuleSnapshotQuery::FindBySkillId(
			SkillRuleSnapshotQueryTests::MakeValidSnapshotSet(),
			Input);

	TestTrue(TEXT("Query succeeds"), Result.bSuccess);
	TestEqual(
		TEXT("Query Input SkillId remains unchanged"),
		Input.SkillId,
		InputBefore.SkillId);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSkillRuleSnapshotQueryNoForbiddenDependenciesTest,
	"FMCodex.CoreRules.SkillRuleSnapshotQuery.HasNoForbiddenDependencies",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSkillRuleSnapshotQueryNoForbiddenDependenciesTest::RunTest(
	const FString& Parameters)
{
	const FString CoreRulesDirectory =
		FPaths::Combine(
			FPaths::ProjectDir(),
			TEXT("Source/FMCodex/CoreRules"));
	const TArray<FString> SourceFiles =
	{
		TEXT("SkillRuleSnapshotQuery.h"),
		TEXT("SkillRuleSnapshotQuery.cpp")
	};

	FString CombinedSource;
	for (const FString& SourceFile : SourceFiles)
	{
		FString SourceText;
		const FString SourcePath =
			FPaths::Combine(CoreRulesDirectory, SourceFile);
		TestTrue(
			*FString::Printf(TEXT("Reads %s"), *SourceFile),
			FFileHelper::LoadFileToString(SourceText, *SourcePath));
		CombinedSource += SourceText;
	}

	TestFalse(
		TEXT("Does not reference LongShot Direct Shot Plan"),
		CombinedSource.Contains(TEXT("LongShotDirectShot")));
	TestFalse(
		TEXT("Does not reference Player Card ownership"),
		CombinedSource.Contains(TEXT("PlayerCardRuleSnapshot"))
			|| CombinedSource.Contains(TEXT("SkillIds")));
	TestFalse(
		TEXT("Does not reference action-point eligibility"),
		CombinedSource.Contains(TEXT("CurrentActionPoint")));
	TestFalse(
		TEXT("Does not reference FormulaResolver"),
		CombinedSource.Contains(TEXT("FormulaResolver"))
			|| CombinedSource.Contains(TEXT("SingleCardFormula")));
	TestFalse(
		TEXT("Does not reference MatchPlay"),
		CombinedSource.Contains(TEXT("MatchPlay")));
	TestFalse(
		TEXT("Does not reference DataTable or Provider"),
		CombinedSource.Contains(TEXT("DataTable"))
			|| CombinedSource.Contains(TEXT("Provider")));
	TestFalse(
		TEXT("Does not reference random APIs"),
		CombinedSource.Contains(TEXT("FMath::Rand"))
			|| CombinedSource.Contains(TEXT("FRandomStream")));
	TestFalse(
		TEXT("Does not define SkillEffect or SkillPipeline"),
		CombinedSource.Contains(TEXT("SkillEffect"))
			|| CombinedSource.Contains(TEXT("SkillPipeline")));
	return true;
}

#endif
