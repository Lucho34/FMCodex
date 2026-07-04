#include "SkillRuleSnapshotValidator.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace SkillRuleSnapshotValidatorTests
{
	const FName LongShotSkillId(TEXT("Skill.LongShot.Primary"));

	FSkillRuleSnapshot MakeValidLongShotRule(
		const FName SkillId = LongShotSkillId,
		const int32 MinActionPoint = 3,
		const int32 MaxActionPoint = 6)
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
			MakeValidLongShotRule()
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
	FSkillRuleSnapshotValidatorValidLongShotTest,
	"FMCodex.CoreRules.SkillRuleSnapshotValidator.ValidLongShotSkillRuleSnapshotPasses",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSkillRuleSnapshotValidatorValidLongShotTest::RunTest(
	const FString& Parameters)
{
	const FSkillRuleSnapshotValidationResult Result =
		FSkillRuleSnapshotValidator::Validate(
			SkillRuleSnapshotValidatorTests::MakeValidSnapshotSet());

	TestTrue(TEXT("Valid LongShot snapshot succeeds"), Result.bSuccess);
	TestTrue(TEXT("Valid LongShot snapshot is valid"), Result.bIsValid);
	TestEqual(
		TEXT("Valid snapshot has no error"),
		Result.ErrorCode,
		ESkillRuleSnapshotValidationErrorCode::None);
	TestTrue(TEXT("Valid snapshot has no error message"), Result.ErrorMessage.IsEmpty());
	TestTrue(TEXT("Valid snapshot has no invalid SkillId"), Result.InvalidSkillId.IsNone());
	TestTrue(TEXT("Valid snapshot has no invalid field"), Result.InvalidField.IsNone());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSkillRuleSnapshotValidatorEmptySkillIdTest,
	"FMCodex.CoreRules.SkillRuleSnapshotValidator.RejectsEmptySkillId",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSkillRuleSnapshotValidatorEmptySkillIdTest::RunTest(
	const FString& Parameters)
{
	FSkillRuleSnapshotSet SnapshotSet =
		SkillRuleSnapshotValidatorTests::MakeValidSnapshotSet();
	SnapshotSet.SkillRules[0].SkillId = NAME_None;

	const FSkillRuleSnapshotValidationResult Result =
		FSkillRuleSnapshotValidator::Validate(SnapshotSet);

	TestFalse(TEXT("Empty SkillId fails"), Result.bSuccess);
	TestFalse(TEXT("Empty SkillId is invalid"), Result.bIsValid);
	TestEqual(
		TEXT("Empty SkillId error is structured"),
		Result.ErrorCode,
		ESkillRuleSnapshotValidationErrorCode::InvalidSkillId);
	TestEqual(
		TEXT("SkillId field is reported"),
		Result.InvalidField,
		FName(TEXT("SkillId")));
	TestFalse(TEXT("Error message is not empty"), Result.ErrorMessage.IsEmpty());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSkillRuleSnapshotValidatorDuplicateSkillIdTest,
	"FMCodex.CoreRules.SkillRuleSnapshotValidator.RejectsDuplicateSkillId",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSkillRuleSnapshotValidatorDuplicateSkillIdTest::RunTest(
	const FString& Parameters)
{
	FSkillRuleSnapshotSet SnapshotSet =
		SkillRuleSnapshotValidatorTests::MakeValidSnapshotSet();
	SnapshotSet.SkillRules.Add(
		SkillRuleSnapshotValidatorTests::MakeValidLongShotRule(
			SkillRuleSnapshotValidatorTests::LongShotSkillId,
			2,
			4));

	const FSkillRuleSnapshotValidationResult Result =
		FSkillRuleSnapshotValidator::Validate(SnapshotSet);

	TestFalse(TEXT("Duplicate SkillId fails"), Result.bSuccess);
	TestEqual(
		TEXT("Duplicate SkillId error is structured"),
		Result.ErrorCode,
		ESkillRuleSnapshotValidationErrorCode::DuplicateSkillId);
	TestEqual(
		TEXT("Duplicate SkillId is retained"),
		Result.InvalidSkillId,
		SkillRuleSnapshotValidatorTests::LongShotSkillId);
	TestEqual(
		TEXT("SkillId field is reported"),
		Result.InvalidField,
		FName(TEXT("SkillId")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSkillRuleSnapshotValidatorNoneSkillTypeTest,
	"FMCodex.CoreRules.SkillRuleSnapshotValidator.RejectsNoneSkillType",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSkillRuleSnapshotValidatorNoneSkillTypeTest::RunTest(
	const FString& Parameters)
{
	FSkillRuleSnapshotSet SnapshotSet =
		SkillRuleSnapshotValidatorTests::MakeValidSnapshotSet();
	SnapshotSet.SkillRules[0].SkillType = ESkillRuleType::None;

	const FSkillRuleSnapshotValidationResult Result =
		FSkillRuleSnapshotValidator::Validate(SnapshotSet);

	TestFalse(TEXT("None SkillType fails"), Result.bSuccess);
	TestEqual(
		TEXT("None SkillType error is structured"),
		Result.ErrorCode,
		ESkillRuleSnapshotValidationErrorCode::InvalidSkillType);
	TestEqual(
		TEXT("SkillType field is reported"),
		Result.InvalidField,
		FName(TEXT("SkillType")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSkillRuleSnapshotValidatorUnsupportedSkillTypeTest,
	"FMCodex.CoreRules.SkillRuleSnapshotValidator.RejectsUnsupportedSkillType",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSkillRuleSnapshotValidatorUnsupportedSkillTypeTest::RunTest(
	const FString& Parameters)
{
	FSkillRuleSnapshotSet SnapshotSet =
		SkillRuleSnapshotValidatorTests::MakeValidSnapshotSet();
	SnapshotSet.SkillRules[0].SkillType =
		static_cast<ESkillRuleType>(255);

	const FSkillRuleSnapshotValidationResult Result =
		FSkillRuleSnapshotValidator::Validate(SnapshotSet);

	TestFalse(TEXT("Unknown SkillType fails"), Result.bSuccess);
	TestEqual(
		TEXT("Unknown SkillType error is structured"),
		Result.ErrorCode,
		ESkillRuleSnapshotValidationErrorCode
			::UnsupportedSkillType);
	TestEqual(
		TEXT("SkillType field is reported"),
		Result.InvalidField,
		FName(TEXT("SkillType")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSkillRuleSnapshotValidatorBelowMinimumTest,
	"FMCodex.CoreRules.SkillRuleSnapshotValidator.RejectsTriggerRangeBelowTwo",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSkillRuleSnapshotValidatorBelowMinimumTest::RunTest(
	const FString& Parameters)
{
	FSkillRuleSnapshotSet SnapshotSet =
		SkillRuleSnapshotValidatorTests::MakeValidSnapshotSet();
	SnapshotSet.SkillRules[0].MinTriggerActionPoint = 1;

	const FSkillRuleSnapshotValidationResult Result =
		FSkillRuleSnapshotValidator::Validate(SnapshotSet);

	TestFalse(TEXT("Trigger range below two fails"), Result.bSuccess);
	TestEqual(
		TEXT("Below-minimum error is structured"),
		Result.ErrorCode,
		ESkillRuleSnapshotValidationErrorCode
			::TriggerActionPointBelowMinimum);
	TestEqual(
		TEXT("Minimum field is reported"),
		Result.InvalidField,
		FName(TEXT("MinTriggerActionPoint")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSkillRuleSnapshotValidatorAboveMaximumTest,
	"FMCodex.CoreRules.SkillRuleSnapshotValidator.RejectsTriggerRangeAboveEight",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSkillRuleSnapshotValidatorAboveMaximumTest::RunTest(
	const FString& Parameters)
{
	FSkillRuleSnapshotSet SnapshotSet =
		SkillRuleSnapshotValidatorTests::MakeValidSnapshotSet();
	SnapshotSet.SkillRules[0].MaxTriggerActionPoint = 9;

	const FSkillRuleSnapshotValidationResult Result =
		FSkillRuleSnapshotValidator::Validate(SnapshotSet);

	TestFalse(TEXT("Trigger range above eight fails"), Result.bSuccess);
	TestEqual(
		TEXT("Above-maximum error is structured"),
		Result.ErrorCode,
		ESkillRuleSnapshotValidationErrorCode
			::TriggerActionPointAboveMaximum);
	TestEqual(
		TEXT("Maximum field is reported"),
		Result.InvalidField,
		FName(TEXT("MaxTriggerActionPoint")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSkillRuleSnapshotValidatorInvalidRangeTest,
	"FMCodex.CoreRules.SkillRuleSnapshotValidator.RejectsMinGreaterThanMax",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSkillRuleSnapshotValidatorInvalidRangeTest::RunTest(
	const FString& Parameters)
{
	FSkillRuleSnapshotSet SnapshotSet =
		SkillRuleSnapshotValidatorTests::MakeValidSnapshotSet();
	SnapshotSet.SkillRules[0].MinTriggerActionPoint = 7;
	SnapshotSet.SkillRules[0].MaxTriggerActionPoint = 3;

	const FSkillRuleSnapshotValidationResult Result =
		FSkillRuleSnapshotValidator::Validate(SnapshotSet);

	TestFalse(TEXT("Min greater than Max fails"), Result.bSuccess);
	TestEqual(
		TEXT("Invalid range error is structured"),
		Result.ErrorCode,
		ESkillRuleSnapshotValidationErrorCode
			::InvalidTriggerActionPointRange);
	TestEqual(
		TEXT("Minimum field is reported"),
		Result.InvalidField,
		FName(TEXT("MinTriggerActionPoint")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSkillRuleSnapshotValidatorMultipleLongShotRulesTest,
	"FMCodex.CoreRules.SkillRuleSnapshotValidator.AllowsMultipleLongShotRulesWithDifferentSkillIds",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSkillRuleSnapshotValidatorMultipleLongShotRulesTest::RunTest(
	const FString& Parameters)
{
	FSkillRuleSnapshotSet SnapshotSet =
		SkillRuleSnapshotValidatorTests::MakeValidSnapshotSet();
	SnapshotSet.SkillRules.Add(
		SkillRuleSnapshotValidatorTests::MakeValidLongShotRule(
			TEXT("Skill.LongShot.Secondary"),
			2,
			4));
	SnapshotSet.SkillRules.Add(
		SkillRuleSnapshotValidatorTests::MakeValidLongShotRule(
			TEXT("Skill.LongShot.Late"),
			7,
			8));

	const FSkillRuleSnapshotValidationResult Result =
		FSkillRuleSnapshotValidator::Validate(SnapshotSet);

	TestTrue(TEXT("Distinct LongShot rules succeed"), Result.bSuccess);
	TestTrue(TEXT("Distinct LongShot rules are valid"), Result.bIsValid);
	TestEqual(
		TEXT("Distinct LongShot rules have no error"),
		Result.ErrorCode,
		ESkillRuleSnapshotValidationErrorCode::None);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSkillRuleSnapshotValidatorInputUnchangedTest,
	"FMCodex.CoreRules.SkillRuleSnapshotValidator.DoesNotMutateInputSnapshotSet",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSkillRuleSnapshotValidatorInputUnchangedTest::RunTest(
	const FString& Parameters)
{
	const FSkillRuleSnapshotSet SnapshotSet =
		SkillRuleSnapshotValidatorTests::MakeValidSnapshotSet();
	const FSkillRuleSnapshotSet SnapshotSetBefore = SnapshotSet;

	const FSkillRuleSnapshotValidationResult Result =
		FSkillRuleSnapshotValidator::Validate(SnapshotSet);

	TestTrue(TEXT("Validation succeeds"), Result.bSuccess);
	TestTrue(
		TEXT("Snapshot Set remains unchanged"),
		SkillRuleSnapshotValidatorTests::AreSnapshotSetsEqual(
			SnapshotSet,
			SnapshotSetBefore));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSkillRuleSnapshotValidatorNoForbiddenDependenciesTest,
	"FMCodex.CoreRules.SkillRuleSnapshotValidator.HasNoForbiddenDependencies",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSkillRuleSnapshotValidatorNoForbiddenDependenciesTest::RunTest(
	const FString& Parameters)
{
	const FString CoreRulesDirectory =
		FPaths::Combine(
			FPaths::ProjectDir(),
			TEXT("Source/FMCodex/CoreRules"));
	const TArray<FString> SourceFiles =
	{
		TEXT("SkillRuleSnapshot.h"),
		TEXT("SkillRuleSnapshotValidator.h"),
		TEXT("SkillRuleSnapshotValidator.cpp")
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
		TEXT("Does not reference SkillRuleSnapshotQuery"),
		CombinedSource.Contains(TEXT("SkillRuleSnapshotQuery")));
	TestFalse(
		TEXT("Does not reference LongShot Direct Shot Plan"),
		CombinedSource.Contains(TEXT("LongShotDirectShot")));
	TestFalse(
		TEXT("Does not reference MatchPlay"),
		CombinedSource.Contains(TEXT("MatchPlay")));
	TestFalse(
		TEXT("Does not reference FormulaResolver"),
		CombinedSource.Contains(TEXT("FormulaResolver")));
	TestFalse(
		TEXT("Does not reference DataTable"),
		CombinedSource.Contains(TEXT("DataTable")));
	TestFalse(
		TEXT("Does not reference Provider"),
		CombinedSource.Contains(TEXT("Provider")));
	TestFalse(
		TEXT("Does not reference random APIs"),
		CombinedSource.Contains(TEXT("FMath::Rand"))
			|| CombinedSource.Contains(TEXT("FRandomStream")));
	TestFalse(
		TEXT("Does not define a generic SkillEffect"),
		CombinedSource.Contains(TEXT("SkillEffect"))
			|| CombinedSource.Contains(TEXT("SkillPipeline")));
	return true;
}

#endif
