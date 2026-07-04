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

	FSkillRuleSnapshotSet MakeValidSnapshotSet()
	{
		FSkillRuleSnapshotSet SnapshotSet;
		SnapshotSet.SkillRules =
		{
			MakeLongShotRule(PrimarySkillId, 2, 5),
			MakeLongShotRule(SecondarySkillId, 6, 8)
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
	FSkillRuleSnapshotQueryPreservesValidationTest,
	"FMCodex.CoreRules.SkillRuleSnapshotQuery.PreservesValidationResultOnSuccess",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSkillRuleSnapshotQueryPreservesValidationTest::RunTest(
	const FString& Parameters)
{
	FSkillRuleSnapshotQueryInput Input;
	Input.SkillId = SkillRuleSnapshotQueryTests::PrimarySkillId;

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
		{ SkillRuleSnapshotQueryTests::PrimarySkillId };
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
