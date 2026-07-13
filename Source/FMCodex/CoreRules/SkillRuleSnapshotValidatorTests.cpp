#include "SkillRuleSnapshotValidator.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace SkillRuleSnapshotValidatorTests
{
	const FName LongShotSkillId(TEXT("Skill.LongShot.Primary"));
	const FName CutInsideShotSkillId(
		TEXT("Skill.CutInsideShot.Primary"));
	const FName PassControlSkillId(
		TEXT("Skill.PassControl.Primary"));
	const FName CrossSkillId(TEXT("Skill.Cross.Primary"));
	const FName ThroughBallSkillId(TEXT("Skill.ThroughBall.Primary"));

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

	FSkillRuleSnapshot MakeValidCutInsideShotRule(
		const FName SkillId = CutInsideShotSkillId,
		const int32 MinActionPoint = 3,
		const int32 MaxActionPoint = 6)
	{
		FSkillRuleSnapshot SkillRule;
		SkillRule.SkillId = SkillId;
		SkillRule.SkillType = ESkillRuleType::CutInsideShot;
		SkillRule.MinTriggerActionPoint = MinActionPoint;
		SkillRule.MaxTriggerActionPoint = MaxActionPoint;
		return SkillRule;
	}

	FSkillRuleSnapshot MakeValidPassControlRule(
		const FName SkillId = PassControlSkillId,
		const int32 MinActionPoint = 3,
		const int32 MaxActionPoint = 6)
	{
		FSkillRuleSnapshot SkillRule;
		SkillRule.SkillId = SkillId;
		SkillRule.SkillType = ESkillRuleType::PassControl;
		SkillRule.MinTriggerActionPoint = MinActionPoint;
		SkillRule.MaxTriggerActionPoint = MaxActionPoint;
		return SkillRule;
	}

	FSkillRuleSnapshot MakeValidCrossRule(
		const FName SkillId = CrossSkillId,
		const int32 MinActionPoint = 3,
		const int32 MaxActionPoint = 6)
	{
		FSkillRuleSnapshot SkillRule;
		SkillRule.SkillId = SkillId;
		SkillRule.SkillType = ESkillRuleType::Cross;
		SkillRule.MinTriggerActionPoint = MinActionPoint;
		SkillRule.MaxTriggerActionPoint = MaxActionPoint;
		return SkillRule;
	}

	FSkillRuleSnapshot MakeValidThroughBallRule(
		const FName SkillId = ThroughBallSkillId,
		const int32 MinActionPoint = 3,
		const int32 MaxActionPoint = 6)
	{
		FSkillRuleSnapshot SkillRule;
		SkillRule.SkillId = SkillId;
		SkillRule.SkillType = ESkillRuleType::ThroughBall;
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
	FSkillRuleSnapshotValidatorValidCutInsideShotTest,
	"FMCodex.CoreRules.SkillRuleSnapshotValidator.ValidCutInsideShotSkillRuleSnapshotPasses",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSkillRuleSnapshotValidatorValidCutInsideShotTest::RunTest(
	const FString& Parameters)
{
	FSkillRuleSnapshotSet SnapshotSet;
	SnapshotSet.SkillRules =
	{
		SkillRuleSnapshotValidatorTests::MakeValidCutInsideShotRule()
	};

	const FSkillRuleSnapshotValidationResult Result =
		FSkillRuleSnapshotValidator::Validate(SnapshotSet);

	TestTrue(TEXT("Valid CutInsideShot snapshot succeeds"), Result.bSuccess);
	TestTrue(TEXT("Valid CutInsideShot snapshot is valid"), Result.bIsValid);
	TestEqual(
		TEXT("Valid snapshot has no error"),
		Result.ErrorCode,
		ESkillRuleSnapshotValidationErrorCode::None);
	TestTrue(
		TEXT("Valid snapshot has no error message"),
		Result.ErrorMessage.IsEmpty());
	TestTrue(
		TEXT("Valid snapshot has no invalid SkillId"),
		Result.InvalidSkillId.IsNone());
	TestTrue(
		TEXT("Valid snapshot has no invalid field"),
		Result.InvalidField.IsNone());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSkillRuleSnapshotValidatorValidPassControlTest,
	"FMCodex.CoreRules.SkillRuleSnapshotValidator.ValidPassControlSkillRuleSnapshotPasses",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSkillRuleSnapshotValidatorValidPassControlTest::RunTest(
	const FString& Parameters)
{
	FSkillRuleSnapshotSet SnapshotSet;
	SnapshotSet.SkillRules =
	{
		SkillRuleSnapshotValidatorTests::MakeValidPassControlRule()
	};

	const FSkillRuleSnapshotValidationResult Result =
		FSkillRuleSnapshotValidator::Validate(SnapshotSet);

	TestTrue(TEXT("Valid PassControl snapshot succeeds"), Result.bSuccess);
	TestTrue(TEXT("Valid PassControl snapshot is valid"), Result.bIsValid);
	TestEqual(
		TEXT("Valid snapshot has no error"),
		Result.ErrorCode,
		ESkillRuleSnapshotValidationErrorCode::None);
	TestTrue(
		TEXT("Valid snapshot has no error message"),
		Result.ErrorMessage.IsEmpty());
	TestTrue(
		TEXT("Valid snapshot has no invalid SkillId"),
		Result.InvalidSkillId.IsNone());
	TestTrue(
		TEXT("Valid snapshot has no invalid field"),
		Result.InvalidField.IsNone());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSkillRuleSnapshotValidatorValidCrossTest,
	"FMCodex.CoreRules.SkillRuleSnapshotValidator.ValidCrossSkillRuleSnapshotPasses",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSkillRuleSnapshotValidatorValidCrossTest::RunTest(
	const FString& Parameters)
{
	FSkillRuleSnapshotSet SnapshotSet;
	SnapshotSet.SkillRules =
	{
		SkillRuleSnapshotValidatorTests::MakeValidCrossRule()
	};

	const FSkillRuleSnapshotValidationResult Result =
		FSkillRuleSnapshotValidator::Validate(SnapshotSet);

	TestTrue(TEXT("Valid Cross snapshot succeeds"), Result.bSuccess);
	TestTrue(TEXT("Valid Cross snapshot is valid"), Result.bIsValid);
	TestEqual(
		TEXT("Valid Cross snapshot has no error"),
		Result.ErrorCode,
		ESkillRuleSnapshotValidationErrorCode::None);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSkillRuleSnapshotValidatorValidThroughBallTest,
	"FMCodex.CoreRules.SkillRuleSnapshotValidator.ValidThroughBallSkillRuleSnapshotPasses",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSkillRuleSnapshotValidatorValidThroughBallTest::RunTest(
	const FString& Parameters)
{
	FSkillRuleSnapshotSet SnapshotSet;
	SnapshotSet.SkillRules =
	{
		SkillRuleSnapshotValidatorTests::MakeValidThroughBallRule()
	};

	const FSkillRuleSnapshotValidationResult Result =
		FSkillRuleSnapshotValidator::Validate(SnapshotSet);

	TestTrue(TEXT("Valid ThroughBall snapshot succeeds"), Result.bSuccess);
	TestTrue(TEXT("Valid ThroughBall snapshot is valid"), Result.bIsValid);
	TestEqual(
		TEXT("Valid ThroughBall snapshot has no error"),
		Result.ErrorCode,
		ESkillRuleSnapshotValidationErrorCode::None);
	TestTrue(
		TEXT("Valid ThroughBall snapshot has no error message"),
		Result.ErrorMessage.IsEmpty());
	TestTrue(
		TEXT("Valid ThroughBall snapshot has no invalid SkillId"),
		Result.InvalidSkillId.IsNone());
	TestTrue(
		TEXT("Valid ThroughBall snapshot has no invalid field"),
		Result.InvalidField.IsNone());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSkillRuleSnapshotValidatorThroughBallActionPointBoundariesTest,
	"FMCodex.CoreRules.SkillRuleSnapshotValidator.ValidatesThroughBallActionPointBoundaries",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSkillRuleSnapshotValidatorThroughBallActionPointBoundariesTest::RunTest(
	const FString& Parameters)
{
	const TArray<TPair<int32, int32>> ValidRanges =
	{
		{ 2, 8 },
		{ 2, 2 },
		{ 8, 8 }
	};

	for (const TPair<int32, int32>& ValidRange : ValidRanges)
	{
		FSkillRuleSnapshotSet SnapshotSet;
		SnapshotSet.SkillRules =
		{
			SkillRuleSnapshotValidatorTests::MakeValidThroughBallRule(
				SkillRuleSnapshotValidatorTests::ThroughBallSkillId,
				ValidRange.Key,
				ValidRange.Value)
		};
		const FSkillRuleSnapshotValidationResult Result =
			FSkillRuleSnapshotValidator::Validate(SnapshotSet);
		TestTrue(
			*FString::Printf(
				TEXT("ThroughBall accepts action-point range [%d,%d]"),
				ValidRange.Key,
				ValidRange.Value),
			Result.bSuccess);
		TestEqual(
			TEXT("Valid ThroughBall boundary has no error"),
			Result.ErrorCode,
			ESkillRuleSnapshotValidationErrorCode::None);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSkillRuleSnapshotValidatorEmptyThroughBallSkillIdTest,
	"FMCodex.CoreRules.SkillRuleSnapshotValidator.RejectsEmptyThroughBallSkillId",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSkillRuleSnapshotValidatorEmptyThroughBallSkillIdTest::RunTest(
	const FString& Parameters)
{
	FSkillRuleSnapshotSet SnapshotSet;
	SnapshotSet.SkillRules =
	{
		SkillRuleSnapshotValidatorTests::MakeValidThroughBallRule(NAME_None)
	};

	const FSkillRuleSnapshotValidationResult Result =
		FSkillRuleSnapshotValidator::Validate(SnapshotSet);

	TestFalse(TEXT("Empty ThroughBall SkillId fails"), Result.bSuccess);
	TestEqual(
		TEXT("Empty ThroughBall SkillId uses the existing error"),
		Result.ErrorCode,
		ESkillRuleSnapshotValidationErrorCode::InvalidSkillId);
	TestEqual(
		TEXT("Empty ThroughBall SkillId reports the SkillId field"),
		Result.InvalidField,
		FName(TEXT("SkillId")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSkillRuleSnapshotValidatorInvalidThroughBallRangeTest,
	"FMCodex.CoreRules.SkillRuleSnapshotValidator.RejectsInvalidThroughBallActionPointRange",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSkillRuleSnapshotValidatorInvalidThroughBallRangeTest::RunTest(
	const FString& Parameters)
{
	FSkillRuleSnapshotSet SnapshotSet;
	SnapshotSet.SkillRules =
	{
		SkillRuleSnapshotValidatorTests::MakeValidThroughBallRule(
			SkillRuleSnapshotValidatorTests::ThroughBallSkillId,
			7,
			3)
	};

	const FSkillRuleSnapshotValidationResult Result =
		FSkillRuleSnapshotValidator::Validate(SnapshotSet);

	TestFalse(TEXT("Invalid ThroughBall range fails"), Result.bSuccess);
	TestEqual(
		TEXT("Invalid ThroughBall range uses the existing error"),
		Result.ErrorCode,
		ESkillRuleSnapshotValidationErrorCode
			::InvalidTriggerActionPointRange);
	TestEqual(
		TEXT("Invalid ThroughBall range reports the minimum field"),
		Result.InvalidField,
		FName(TEXT("MinTriggerActionPoint")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSkillRuleSnapshotValidatorDuplicateThroughBallSkillIdTest,
	"FMCodex.CoreRules.SkillRuleSnapshotValidator.RejectsDuplicateThroughBallSkillId",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSkillRuleSnapshotValidatorDuplicateThroughBallSkillIdTest::RunTest(
	const FString& Parameters)
{
	FSkillRuleSnapshotSet SnapshotSet;
	SnapshotSet.SkillRules =
	{
		SkillRuleSnapshotValidatorTests::MakeValidLongShotRule(
			SkillRuleSnapshotValidatorTests::ThroughBallSkillId),
		SkillRuleSnapshotValidatorTests::MakeValidThroughBallRule()
	};

	const FSkillRuleSnapshotValidationResult Result =
		FSkillRuleSnapshotValidator::Validate(SnapshotSet);

	TestFalse(TEXT("Duplicate ThroughBall SkillId fails"), Result.bSuccess);
	TestEqual(
		TEXT("Duplicate ThroughBall SkillId uses the existing error"),
		Result.ErrorCode,
		ESkillRuleSnapshotValidationErrorCode::DuplicateSkillId);
	TestEqual(
		TEXT("Duplicate ThroughBall SkillId is retained"),
		Result.InvalidSkillId,
		SkillRuleSnapshotValidatorTests::ThroughBallSkillId);
	TestEqual(
		TEXT("Duplicate ThroughBall SkillId reports the SkillId field"),
		Result.InvalidField,
		FName(TEXT("SkillId")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSkillRuleSnapshotValidatorCrossActionPointBoundariesTest,
	"FMCodex.CoreRules.SkillRuleSnapshotValidator.ValidatesCrossActionPointBoundaries",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSkillRuleSnapshotValidatorCrossActionPointBoundariesTest::RunTest(
	const FString& Parameters)
{
	FSkillRuleSnapshotSet MinimumSnapshotSet;
	MinimumSnapshotSet.SkillRules =
	{
		SkillRuleSnapshotValidatorTests::MakeValidCrossRule(
			SkillRuleSnapshotValidatorTests::CrossSkillId,
			2,
			2)
	};
	TestTrue(
		TEXT("Cross accepts the existing minimum action point"),
		FSkillRuleSnapshotValidator::Validate(MinimumSnapshotSet).bSuccess);

	FSkillRuleSnapshotSet MaximumSnapshotSet;
	MaximumSnapshotSet.SkillRules =
	{
		SkillRuleSnapshotValidatorTests::MakeValidCrossRule(
			SkillRuleSnapshotValidatorTests::CrossSkillId,
			8,
			8)
	};
	TestTrue(
		TEXT("Cross accepts the existing maximum action point"),
		FSkillRuleSnapshotValidator::Validate(MaximumSnapshotSet).bSuccess);

	FSkillRuleSnapshotSet BelowMinimumSnapshotSet;
	BelowMinimumSnapshotSet.SkillRules =
	{
		SkillRuleSnapshotValidatorTests::MakeValidCrossRule(
			SkillRuleSnapshotValidatorTests::CrossSkillId,
			1,
			6)
	};
	const FSkillRuleSnapshotValidationResult BelowMinimumResult =
		FSkillRuleSnapshotValidator::Validate(BelowMinimumSnapshotSet);
	TestEqual(
		TEXT("Cross rejects action points below the existing minimum"),
		BelowMinimumResult.ErrorCode,
		ESkillRuleSnapshotValidationErrorCode
			::TriggerActionPointBelowMinimum);

	FSkillRuleSnapshotSet AboveMaximumSnapshotSet;
	AboveMaximumSnapshotSet.SkillRules =
	{
		SkillRuleSnapshotValidatorTests::MakeValidCrossRule(
			SkillRuleSnapshotValidatorTests::CrossSkillId,
			3,
			9)
	};
	const FSkillRuleSnapshotValidationResult AboveMaximumResult =
		FSkillRuleSnapshotValidator::Validate(AboveMaximumSnapshotSet);
	TestEqual(
		TEXT("Cross rejects action points above the existing maximum"),
		AboveMaximumResult.ErrorCode,
		ESkillRuleSnapshotValidationErrorCode
			::TriggerActionPointAboveMaximum);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSkillRuleSnapshotValidatorCrossEmptySkillIdTest,
	"FMCodex.CoreRules.SkillRuleSnapshotValidator.RejectsEmptyCrossSkillId",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSkillRuleSnapshotValidatorCrossEmptySkillIdTest::RunTest(
	const FString& Parameters)
{
	FSkillRuleSnapshotSet SnapshotSet;
	SnapshotSet.SkillRules =
	{
		SkillRuleSnapshotValidatorTests::MakeValidCrossRule(NAME_None)
	};

	const FSkillRuleSnapshotValidationResult Result =
		FSkillRuleSnapshotValidator::Validate(SnapshotSet);

	TestFalse(TEXT("Empty Cross SkillId fails"), Result.bSuccess);
	TestEqual(
		TEXT("Empty Cross SkillId uses the existing structured error"),
		Result.ErrorCode,
		ESkillRuleSnapshotValidationErrorCode::InvalidSkillId);
	TestEqual(
		TEXT("SkillId field is reported"),
		Result.InvalidField,
		FName(TEXT("SkillId")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSkillRuleSnapshotValidatorDuplicateCrossSkillIdTest,
	"FMCodex.CoreRules.SkillRuleSnapshotValidator.RejectsDuplicateCrossSkillId",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSkillRuleSnapshotValidatorDuplicateCrossSkillIdTest::RunTest(
	const FString& Parameters)
{
	FSkillRuleSnapshotSet SnapshotSet =
		SkillRuleSnapshotValidatorTests::MakeValidSnapshotSet();
	SnapshotSet.SkillRules.Add(
		SkillRuleSnapshotValidatorTests::MakeValidCrossRule());
	SnapshotSet.SkillRules.Add(
		SkillRuleSnapshotValidatorTests::MakeValidCrossRule());

	const FSkillRuleSnapshotValidationResult Result =
		FSkillRuleSnapshotValidator::Validate(SnapshotSet);

	TestFalse(TEXT("Duplicate Cross SkillId fails"), Result.bSuccess);
	TestEqual(
		TEXT("Duplicate Cross SkillId uses the existing structured error"),
		Result.ErrorCode,
		ESkillRuleSnapshotValidationErrorCode::DuplicateSkillId);
	TestEqual(
		TEXT("Duplicate Cross SkillId is retained"),
		Result.InvalidSkillId,
		SkillRuleSnapshotValidatorTests::CrossSkillId);
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
	FSkillRuleSnapshotValidatorMixedSupportedRulesTest,
	"FMCodex.CoreRules.SkillRuleSnapshotValidator.AllowsAllSupportedRulesTogether",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSkillRuleSnapshotValidatorMixedSupportedRulesTest::RunTest(
	const FString& Parameters)
{
	FSkillRuleSnapshotSet SnapshotSet =
		SkillRuleSnapshotValidatorTests::MakeValidSnapshotSet();
	SnapshotSet.SkillRules.Add(
		SkillRuleSnapshotValidatorTests::MakeValidCutInsideShotRule(
			SkillRuleSnapshotValidatorTests::CutInsideShotSkillId,
			2,
			5));
	SnapshotSet.SkillRules.Add(
		SkillRuleSnapshotValidatorTests::MakeValidPassControlRule(
			SkillRuleSnapshotValidatorTests::PassControlSkillId,
			6,
			8));
	SnapshotSet.SkillRules.Add(
		SkillRuleSnapshotValidatorTests::MakeValidCrossRule(
			SkillRuleSnapshotValidatorTests::CrossSkillId,
			3,
			6));
	SnapshotSet.SkillRules.Add(
		SkillRuleSnapshotValidatorTests::MakeValidThroughBallRule(
			SkillRuleSnapshotValidatorTests::ThroughBallSkillId,
			2,
			8));

	const FSkillRuleSnapshotValidationResult Result =
		FSkillRuleSnapshotValidator::Validate(SnapshotSet);

	TestTrue(
		TEXT("Mixed supported rules succeed"),
		Result.bSuccess);
	TestTrue(
		TEXT("Mixed supported rules are valid"),
		Result.bIsValid);
	TestEqual(
		TEXT("Mixed supported rules have no error"),
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
		[]()
		{
			FSkillRuleSnapshotSet MixedSnapshotSet =
				SkillRuleSnapshotValidatorTests::MakeValidSnapshotSet();
			MixedSnapshotSet.SkillRules.Add(
				SkillRuleSnapshotValidatorTests
					::MakeValidCutInsideShotRule());
			MixedSnapshotSet.SkillRules.Add(
				SkillRuleSnapshotValidatorTests
					::MakeValidPassControlRule());
			MixedSnapshotSet.SkillRules.Add(
				SkillRuleSnapshotValidatorTests::MakeValidCrossRule());
			MixedSnapshotSet.SkillRules.Add(
				SkillRuleSnapshotValidatorTests
					::MakeValidThroughBallRule());
			return MixedSnapshotSet;
		}();
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
