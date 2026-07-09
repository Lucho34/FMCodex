#if WITH_DEV_AUTOMATION_TESTS

#include "PassControlAdvanceSelectionQuery.h"

#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace PassControlAdvanceSelectionQueryTests
{
	const FName SkillId(TEXT("Skill.PassControl.AdvanceSelection"));
	const FName CarrierCardId(TEXT("PassControl.Carrier"));
	const FName AttackerPlayerId(TEXT("Player.PassControl.Attacker"));
	const FGuid LogId(910, 920, 930, 940);

	FPlayerCardRuleSnapshot MakeCarrier(
		const bool bOwnsSkill = true,
		const bool bGoalkeeper = false)
	{
		FPlayerCardRuleSnapshot Card;
		Card.CardId = CarrierCardId;
		Card.SkillIds = bOwnsSkill
			? TArray<FName>{ SkillId }
			: TArray<FName>{};
		if (bGoalkeeper)
		{
			Card.PositionTypes = {
				EPlayerPositionType::Goalkeeper
			};
			Card.bIsGoalkeeper = true;
			Card.bHasGoalkeeperAttributes = true;
		}
		else
		{
			Card.PositionTypes = {
				EPlayerPositionType::Midfield
			};
		}
		return Card;
	}

	FPlayerCardRuleSnapshotSet MakePlayerCardSnapshots(
		const bool bOwnsSkill = true,
		const bool bGoalkeeper = false)
	{
		FPlayerCardRuleSnapshotSet SnapshotSet;
		SnapshotSet.Cards = {
			MakeCarrier(bOwnsSkill, bGoalkeeper)
		};
		return SnapshotSet;
	}

	FSkillRuleSnapshotSet MakeSkillRules()
	{
		FSkillRuleSnapshot Rule;
		Rule.SkillId = SkillId;
		Rule.SkillType = ESkillRuleType::PassControl;
		Rule.MinTriggerActionPoint = 3;
		Rule.MaxTriggerActionPoint = 6;

		FSkillRuleSnapshotSet SnapshotSet;
		SnapshotSet.SkillRules = { Rule };
		return SnapshotSet;
	}

	FPassControlAdvanceSelectionQueryInput MakeInput(
		const int32 D6 = 1)
	{
		FPassControlAdvanceSelectionQueryInput Input;
		Input.SkillId = SkillId;
		Input.CarrierCardId = CarrierCardId;
		Input.CurrentActionPoint = 4;
		Input.bHasExternalAdvanceD6 = true;
		Input.ExternalAdvanceD6 = D6;
		Input.LogId = LogId;
		Input.TurnIndex = 19;
		Input.AttackerPlayerId = AttackerPlayerId;
		return Input;
	}

	bool ExpectError(
		FAutomationTestBase& Test,
		const FPassControlAdvanceSelectionQueryInput& Input,
		const FPlayerCardRuleSnapshotSet& PlayerCardSnapshots,
		const FSkillRuleSnapshotSet& SkillRules,
		const EPassControlAdvanceSelectionQueryErrorCode ErrorCode,
		const FName InvalidField)
	{
		const FPassControlAdvanceSelectionQueryResult Result =
			FPassControlAdvanceSelectionQuery::Select(
				PlayerCardSnapshots,
				SkillRules,
				Input);
		Test.TestFalse(TEXT("Advance Selection Query fails"), Result.bSuccess);
		Test.TestEqual(
			TEXT("Expected error code is returned"),
			Result.ErrorCode,
			ErrorCode);
		Test.TestEqual(
			TEXT("Expected invalid field is returned"),
			Result.InvalidField,
			InvalidField);
		Test.TestFalse(
			TEXT("Failure message is not empty"),
			Result.ErrorMessage.IsEmpty());
		Test.TestEqual(
			TEXT("Failure produces no advance type"),
			Result.AdvanceType,
			EPassControlAdvanceType::None);
		return true;
	}

	bool AreInputsEqual(
		const FPassControlAdvanceSelectionQueryInput& Left,
		const FPassControlAdvanceSelectionQueryInput& Right)
	{
		return Left.SkillId == Right.SkillId
			&& Left.CarrierCardId == Right.CarrierCardId
			&& Left.CurrentActionPoint == Right.CurrentActionPoint
			&& Left.bHasExternalAdvanceD6
				== Right.bHasExternalAdvanceD6
			&& Left.ExternalAdvanceD6 == Right.ExternalAdvanceD6
			&& Left.LogId == Right.LogId
			&& Left.TurnIndex == Right.TurnIndex
			&& Left.AttackerPlayerId == Right.AttackerPlayerId;
	}

	bool ArePlayerAttributesEqual(
		const FPlayerAttributes& Left,
		const FPlayerAttributes& Right)
	{
		return Left.Shooting == Right.Shooting
			&& Left.Dribbling == Right.Dribbling
			&& Left.Passing == Right.Passing
			&& Left.OffBall == Right.OffBall
			&& Left.Marking == Right.Marking
			&& Left.Tackling == Right.Tackling
			&& Left.Speed == Right.Speed
			&& Left.Strength == Right.Strength
			&& Left.Stamina == Right.Stamina
			&& Left.LongShot == Right.LongShot;
	}

	bool AreGoalkeeperAttributesEqual(
		const FGoalkeeperAttributes& Left,
		const FGoalkeeperAttributes& Right)
	{
		return Left.Handling == Right.Handling
			&& Left.Positioning == Right.Positioning
			&& Left.Reflex == Right.Reflex
			&& Left.Aerial == Right.Aerial
			&& Left.Anticipation == Right.Anticipation
			&& Left.OneOnOne == Right.OneOnOne;
	}

	bool ArePlayerSnapshotSetsEqual(
		const FPlayerCardRuleSnapshotSet& Left,
		const FPlayerCardRuleSnapshotSet& Right)
	{
		if (Left.Cards.Num() != Right.Cards.Num())
		{
			return false;
		}
		for (int32 Index = 0; Index < Left.Cards.Num(); ++Index)
		{
			const FPlayerCardRuleSnapshot& LeftCard =
				Left.Cards[Index];
			const FPlayerCardRuleSnapshot& RightCard =
				Right.Cards[Index];
			if (LeftCard.CardId != RightCard.CardId
				|| LeftCard.PositionTypes != RightCard.PositionTypes
				|| !ArePlayerAttributesEqual(
					LeftCard.Attributes,
					RightCard.Attributes)
				|| LeftCard.bIsGoalkeeper
					!= RightCard.bIsGoalkeeper
				|| LeftCard.bHasGoalkeeperAttributes
					!= RightCard.bHasGoalkeeperAttributes
				|| !AreGoalkeeperAttributesEqual(
					LeftCard.GoalkeeperAttributes,
					RightCard.GoalkeeperAttributes)
				|| LeftCard.Rarity != RightCard.Rarity
				|| LeftCard.SkillIds != RightCard.SkillIds)
			{
				return false;
			}
		}
		return true;
	}

	bool AreSkillRuleSnapshotSetsEqual(
		const FSkillRuleSnapshotSet& Left,
		const FSkillRuleSnapshotSet& Right)
	{
		if (Left.SkillRules.Num() != Right.SkillRules.Num())
		{
			return false;
		}
		for (int32 Index = 0; Index < Left.SkillRules.Num(); ++Index)
		{
			const FSkillRuleSnapshot& LeftRule =
				Left.SkillRules[Index];
			const FSkillRuleSnapshot& RightRule =
				Right.SkillRules[Index];
			if (LeftRule.SkillId != RightRule.SkillId
				|| LeftRule.SkillType != RightRule.SkillType
				|| LeftRule.MinTriggerActionPoint
					!= RightRule.MinTriggerActionPoint
				|| LeftRule.MaxTriggerActionPoint
					!= RightRule.MaxTriggerActionPoint)
			{
				return false;
			}
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
				TEXT("PassControlAdvanceSelectionQuery.h")));
		const bool bImplementationLoaded =
			FFileHelper::LoadFileToString(
				Implementation,
				*FPaths::Combine(
					Directory,
					TEXT("PassControlAdvanceSelectionQuery.cpp")));
		OutSource = Header + Implementation;
		return bHeaderLoaded && bImplementationLoaded;
	}
}

#define PASS_CONTROL_ADVANCE_SELECTION_TEST(ClassName, TestName) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
		ClassName, \
		"FMCodex.CoreRules.PassControlAdvanceSelectionQuery." TestName, \
		EAutomationTestFlags::EditorContext \
			| EAutomationTestFlags::EngineFilter)

PASS_CONTROL_ADVANCE_SELECTION_TEST(
	FPassControlAdvanceD6OnePassTest,
	"D6OneReturnsPassAdvance")

bool FPassControlAdvanceD6OnePassTest::RunTest(const FString& Parameters)
{
	using namespace PassControlAdvanceSelectionQueryTests;
	const FPassControlAdvanceSelectionQueryResult Result =
		FPassControlAdvanceSelectionQuery::Select(
			MakePlayerCardSnapshots(),
			MakeSkillRules(),
			MakeInput(1));
	TestTrue(TEXT("D6 one succeeds"), Result.bSuccess);
	TestEqual(
		TEXT("D6 one maps to PassAdvance"),
		Result.AdvanceType,
		EPassControlAdvanceType::PassAdvance);
	return true;
}

PASS_CONTROL_ADVANCE_SELECTION_TEST(
	FPassControlAdvanceD6TwoPassTest,
	"D6TwoReturnsPassAdvance")

bool FPassControlAdvanceD6TwoPassTest::RunTest(const FString& Parameters)
{
	using namespace PassControlAdvanceSelectionQueryTests;
	const FPassControlAdvanceSelectionQueryResult Result =
		FPassControlAdvanceSelectionQuery::Select(
			MakePlayerCardSnapshots(),
			MakeSkillRules(),
			MakeInput(2));
	TestTrue(TEXT("D6 two succeeds"), Result.bSuccess);
	TestEqual(
		TEXT("D6 two maps to PassAdvance"),
		Result.AdvanceType,
		EPassControlAdvanceType::PassAdvance);
	return true;
}

PASS_CONTROL_ADVANCE_SELECTION_TEST(
	FPassControlAdvanceD6ThreeDribbleTest,
	"D6ThreeReturnsDribbleAdvance")

bool FPassControlAdvanceD6ThreeDribbleTest::RunTest(
	const FString& Parameters)
{
	using namespace PassControlAdvanceSelectionQueryTests;
	const FPassControlAdvanceSelectionQueryResult Result =
		FPassControlAdvanceSelectionQuery::Select(
			MakePlayerCardSnapshots(),
			MakeSkillRules(),
			MakeInput(3));
	TestTrue(TEXT("D6 three succeeds"), Result.bSuccess);
	TestEqual(
		TEXT("D6 three maps to DribbleAdvance"),
		Result.AdvanceType,
		EPassControlAdvanceType::DribbleAdvance);
	return true;
}

PASS_CONTROL_ADVANCE_SELECTION_TEST(
	FPassControlAdvanceD6FourDribbleTest,
	"D6FourReturnsDribbleAdvance")

bool FPassControlAdvanceD6FourDribbleTest::RunTest(
	const FString& Parameters)
{
	using namespace PassControlAdvanceSelectionQueryTests;
	const FPassControlAdvanceSelectionQueryResult Result =
		FPassControlAdvanceSelectionQuery::Select(
			MakePlayerCardSnapshots(),
			MakeSkillRules(),
			MakeInput(4));
	TestTrue(TEXT("D6 four succeeds"), Result.bSuccess);
	TestEqual(
		TEXT("D6 four maps to DribbleAdvance"),
		Result.AdvanceType,
		EPassControlAdvanceType::DribbleAdvance);
	return true;
}

PASS_CONTROL_ADVANCE_SELECTION_TEST(
	FPassControlAdvanceD6FiveRunTest,
	"D6FiveReturnsRunAdvance")

bool FPassControlAdvanceD6FiveRunTest::RunTest(const FString& Parameters)
{
	using namespace PassControlAdvanceSelectionQueryTests;
	const FPassControlAdvanceSelectionQueryResult Result =
		FPassControlAdvanceSelectionQuery::Select(
			MakePlayerCardSnapshots(),
			MakeSkillRules(),
			MakeInput(5));
	TestTrue(TEXT("D6 five succeeds"), Result.bSuccess);
	TestEqual(
		TEXT("D6 five maps to RunAdvance"),
		Result.AdvanceType,
		EPassControlAdvanceType::RunAdvance);
	return true;
}

PASS_CONTROL_ADVANCE_SELECTION_TEST(
	FPassControlAdvanceD6SixRunTest,
	"D6SixReturnsRunAdvance")

bool FPassControlAdvanceD6SixRunTest::RunTest(const FString& Parameters)
{
	using namespace PassControlAdvanceSelectionQueryTests;
	const FPassControlAdvanceSelectionQueryResult Result =
		FPassControlAdvanceSelectionQuery::Select(
			MakePlayerCardSnapshots(),
			MakeSkillRules(),
			MakeInput(6));
	TestTrue(TEXT("D6 six succeeds"), Result.bSuccess);
	TestEqual(
		TEXT("D6 six maps to RunAdvance"),
		Result.AdvanceType,
		EPassControlAdvanceType::RunAdvance);
	return true;
}

PASS_CONTROL_ADVANCE_SELECTION_TEST(
	FPassControlAdvanceRejectsMissingD6Test,
	"RejectsMissingExternalAdvanceD6")

bool FPassControlAdvanceRejectsMissingD6Test::RunTest(
	const FString& Parameters)
{
	using namespace PassControlAdvanceSelectionQueryTests;
	FPassControlAdvanceSelectionQueryInput Input = MakeInput();
	Input.bHasExternalAdvanceD6 = false;
	return ExpectError(
		*this,
		Input,
		MakePlayerCardSnapshots(),
		MakeSkillRules(),
		EPassControlAdvanceSelectionQueryErrorCode
			::MissingExternalAdvanceD6,
		TEXT("ExternalAdvanceD6"));
}

PASS_CONTROL_ADVANCE_SELECTION_TEST(
	FPassControlAdvanceRejectsLowD6Test,
	"RejectsExternalAdvanceD6BelowRange")

bool FPassControlAdvanceRejectsLowD6Test::RunTest(
	const FString& Parameters)
{
	using namespace PassControlAdvanceSelectionQueryTests;
	return ExpectError(
		*this,
		MakeInput(0),
		MakePlayerCardSnapshots(),
		MakeSkillRules(),
		EPassControlAdvanceSelectionQueryErrorCode::InvalidAdvanceD6,
		TEXT("ExternalAdvanceD6"));
}

PASS_CONTROL_ADVANCE_SELECTION_TEST(
	FPassControlAdvanceRejectsHighD6Test,
	"RejectsExternalAdvanceD6AboveRange")

bool FPassControlAdvanceRejectsHighD6Test::RunTest(
	const FString& Parameters)
{
	using namespace PassControlAdvanceSelectionQueryTests;
	return ExpectError(
		*this,
		MakeInput(7),
		MakePlayerCardSnapshots(),
		MakeSkillRules(),
		EPassControlAdvanceSelectionQueryErrorCode::InvalidAdvanceD6,
		TEXT("ExternalAdvanceD6"));
}

PASS_CONTROL_ADVANCE_SELECTION_TEST(
	FPassControlAdvanceRejectsEmptySkillIdTest,
	"RejectsEmptySkillId")

bool FPassControlAdvanceRejectsEmptySkillIdTest::RunTest(
	const FString& Parameters)
{
	using namespace PassControlAdvanceSelectionQueryTests;
	FPassControlAdvanceSelectionQueryInput Input = MakeInput();
	Input.SkillId = NAME_None;
	return ExpectError(
		*this,
		Input,
		MakePlayerCardSnapshots(),
		MakeSkillRules(),
		EPassControlAdvanceSelectionQueryErrorCode::MissingSkillId,
		TEXT("SkillId"));
}

PASS_CONTROL_ADVANCE_SELECTION_TEST(
	FPassControlAdvanceRejectsSkillTypeMismatchTest,
	"RejectsSkillTypeMismatch")

bool FPassControlAdvanceRejectsSkillTypeMismatchTest::RunTest(
	const FString& Parameters)
{
	using namespace PassControlAdvanceSelectionQueryTests;
	FSkillRuleSnapshotSet SkillRules = MakeSkillRules();
	SkillRules.SkillRules[0].SkillType = ESkillRuleType::LongShot;
	return ExpectError(
		*this,
		MakeInput(),
		MakePlayerCardSnapshots(),
		SkillRules,
		EPassControlAdvanceSelectionQueryErrorCode::SkillTypeMismatch,
		TEXT("SkillType"));
}

PASS_CONTROL_ADVANCE_SELECTION_TEST(
	FPassControlAdvanceRejectsMissingSkillRuleTest,
	"RejectsSkillRuleNotFound")

bool FPassControlAdvanceRejectsMissingSkillRuleTest::RunTest(
	const FString& Parameters)
{
	using namespace PassControlAdvanceSelectionQueryTests;
	return ExpectError(
		*this,
		MakeInput(),
		MakePlayerCardSnapshots(),
		FSkillRuleSnapshotSet{},
		EPassControlAdvanceSelectionQueryErrorCode
			::SkillRuleQueryFailed,
		TEXT("SkillId"));
}

PASS_CONTROL_ADVANCE_SELECTION_TEST(
	FPassControlAdvanceRejectsMissingCarrierTest,
	"RejectsMissingCarrierSnapshot")

bool FPassControlAdvanceRejectsMissingCarrierTest::RunTest(
	const FString& Parameters)
{
	using namespace PassControlAdvanceSelectionQueryTests;
	FPassControlAdvanceSelectionQueryInput Input = MakeInput();
	Input.CarrierCardId = TEXT("PassControl.Missing");
	return ExpectError(
		*this,
		Input,
		MakePlayerCardSnapshots(),
		MakeSkillRules(),
		EPassControlAdvanceSelectionQueryErrorCode
			::CarrierSnapshotQueryFailed,
		TEXT("CarrierCardId"));
}

PASS_CONTROL_ADVANCE_SELECTION_TEST(
	FPassControlAdvanceRejectsUnownedSkillTest,
	"RejectsSkillNotOwnedByCarrier")

bool FPassControlAdvanceRejectsUnownedSkillTest::RunTest(
	const FString& Parameters)
{
	using namespace PassControlAdvanceSelectionQueryTests;
	return ExpectError(
		*this,
		MakeInput(),
		MakePlayerCardSnapshots(false),
		MakeSkillRules(),
		EPassControlAdvanceSelectionQueryErrorCode
			::SkillNotOwnedByCarrier,
		TEXT("SkillId"));
}

PASS_CONTROL_ADVANCE_SELECTION_TEST(
	FPassControlAdvanceRejectsGoalkeeperTest,
	"RejectsGoalkeeperCarrier")

bool FPassControlAdvanceRejectsGoalkeeperTest::RunTest(
	const FString& Parameters)
{
	using namespace PassControlAdvanceSelectionQueryTests;
	return ExpectError(
		*this,
		MakeInput(),
		MakePlayerCardSnapshots(true, true),
		MakeSkillRules(),
		EPassControlAdvanceSelectionQueryErrorCode
			::UnsupportedGoalkeeperParticipant,
		TEXT("CarrierCardId"));
}

PASS_CONTROL_ADVANCE_SELECTION_TEST(
	FPassControlAdvanceRejectsInvalidActionPointTest,
	"RejectsInvalidActionPoint")

bool FPassControlAdvanceRejectsInvalidActionPointTest::RunTest(
	const FString& Parameters)
{
	using namespace PassControlAdvanceSelectionQueryTests;
	FPassControlAdvanceSelectionQueryInput Input = MakeInput();
	Input.CurrentActionPoint = 1;
	return ExpectError(
		*this,
		Input,
		MakePlayerCardSnapshots(),
		MakeSkillRules(),
		EPassControlAdvanceSelectionQueryErrorCode::InvalidActionPoint,
		TEXT("CurrentActionPoint"));
}

PASS_CONTROL_ADVANCE_SELECTION_TEST(
	FPassControlAdvanceRejectsActionPointBelowRangeTest,
	"RejectsActionPointBelowSkillRange")

bool FPassControlAdvanceRejectsActionPointBelowRangeTest::RunTest(
	const FString& Parameters)
{
	using namespace PassControlAdvanceSelectionQueryTests;
	FPassControlAdvanceSelectionQueryInput Input = MakeInput();
	Input.CurrentActionPoint = 2;
	return ExpectError(
		*this,
		Input,
		MakePlayerCardSnapshots(),
		MakeSkillRules(),
		EPassControlAdvanceSelectionQueryErrorCode
			::ActionPointOutOfRange,
		TEXT("CurrentActionPoint"));
}

PASS_CONTROL_ADVANCE_SELECTION_TEST(
	FPassControlAdvanceRejectsActionPointAboveRangeTest,
	"RejectsActionPointAboveSkillRange")

bool FPassControlAdvanceRejectsActionPointAboveRangeTest::RunTest(
	const FString& Parameters)
{
	using namespace PassControlAdvanceSelectionQueryTests;
	FPassControlAdvanceSelectionQueryInput Input = MakeInput();
	Input.CurrentActionPoint = 7;
	return ExpectError(
		*this,
		Input,
		MakePlayerCardSnapshots(),
		MakeSkillRules(),
		EPassControlAdvanceSelectionQueryErrorCode
			::ActionPointOutOfRange,
		TEXT("CurrentActionPoint"));
}

PASS_CONTROL_ADVANCE_SELECTION_TEST(
	FPassControlAdvanceRejectsInvalidLogIdTest,
	"RejectsInvalidLogId")

bool FPassControlAdvanceRejectsInvalidLogIdTest::RunTest(
	const FString& Parameters)
{
	using namespace PassControlAdvanceSelectionQueryTests;
	FPassControlAdvanceSelectionQueryInput Input = MakeInput();
	Input.LogId.Invalidate();
	return ExpectError(
		*this,
		Input,
		MakePlayerCardSnapshots(),
		MakeSkillRules(),
		EPassControlAdvanceSelectionQueryErrorCode::InvalidLogContext,
		TEXT("LogId"));
}

PASS_CONTROL_ADVANCE_SELECTION_TEST(
	FPassControlAdvanceRejectsInvalidTurnIndexTest,
	"RejectsInvalidTurnIndex")

bool FPassControlAdvanceRejectsInvalidTurnIndexTest::RunTest(
	const FString& Parameters)
{
	using namespace PassControlAdvanceSelectionQueryTests;
	FPassControlAdvanceSelectionQueryInput Input = MakeInput();
	Input.TurnIndex = -1;
	return ExpectError(
		*this,
		Input,
		MakePlayerCardSnapshots(),
		MakeSkillRules(),
		EPassControlAdvanceSelectionQueryErrorCode::InvalidLogContext,
		TEXT("TurnIndex"));
}

PASS_CONTROL_ADVANCE_SELECTION_TEST(
	FPassControlAdvanceRejectsMissingAttackerPlayerIdTest,
	"RejectsMissingAttackerPlayerId")

bool FPassControlAdvanceRejectsMissingAttackerPlayerIdTest::RunTest(
	const FString& Parameters)
{
	using namespace PassControlAdvanceSelectionQueryTests;
	FPassControlAdvanceSelectionQueryInput Input = MakeInput();
	Input.AttackerPlayerId = NAME_None;
	return ExpectError(
		*this,
		Input,
		MakePlayerCardSnapshots(),
		MakeSkillRules(),
		EPassControlAdvanceSelectionQueryErrorCode::InvalidLogContext,
		TEXT("AttackerPlayerId"));
}

PASS_CONTROL_ADVANCE_SELECTION_TEST(
	FPassControlAdvanceInputImmutabilityTest,
	"DoesNotMutateInput")

bool FPassControlAdvanceInputImmutabilityTest::RunTest(
	const FString& Parameters)
{
	using namespace PassControlAdvanceSelectionQueryTests;
	const FPassControlAdvanceSelectionQueryInput Input = MakeInput();
	const FPassControlAdvanceSelectionQueryInput Before = Input;
	const FPassControlAdvanceSelectionQueryResult Result =
		FPassControlAdvanceSelectionQuery::Select(
			MakePlayerCardSnapshots(),
			MakeSkillRules(),
			Input);
	TestTrue(
		TEXT("Input remains unchanged"),
		AreInputsEqual(Input, Before));
	TestTrue(
		TEXT("Result preserves input copy"),
		AreInputsEqual(Result.Input, Before));
	return true;
}

PASS_CONTROL_ADVANCE_SELECTION_TEST(
	FPassControlAdvancePlayerSnapshotsImmutabilityTest,
	"DoesNotMutatePlayerCardSnapshotSet")

bool FPassControlAdvancePlayerSnapshotsImmutabilityTest::RunTest(
	const FString& Parameters)
{
	using namespace PassControlAdvanceSelectionQueryTests;
	const FPlayerCardRuleSnapshotSet SnapshotSet =
		MakePlayerCardSnapshots();
	const FPlayerCardRuleSnapshotSet Before = SnapshotSet;
	FPassControlAdvanceSelectionQuery::Select(
		SnapshotSet,
		MakeSkillRules(),
		MakeInput());
	TestTrue(
		TEXT("Player Snapshot Set remains unchanged"),
		ArePlayerSnapshotSetsEqual(SnapshotSet, Before));
	return true;
}

PASS_CONTROL_ADVANCE_SELECTION_TEST(
	FPassControlAdvanceSkillRulesImmutabilityTest,
	"DoesNotMutateSkillRuleSnapshotSet")

bool FPassControlAdvanceSkillRulesImmutabilityTest::RunTest(
	const FString& Parameters)
{
	using namespace PassControlAdvanceSelectionQueryTests;
	const FSkillRuleSnapshotSet SnapshotSet = MakeSkillRules();
	const FSkillRuleSnapshotSet Before = SnapshotSet;
	FPassControlAdvanceSelectionQuery::Select(
		MakePlayerCardSnapshots(),
		SnapshotSet,
		MakeInput());
	TestTrue(
		TEXT("Skill Rule Snapshot Set remains unchanged"),
		AreSkillRuleSnapshotSetsEqual(SnapshotSet, Before));
	return true;
}

PASS_CONTROL_ADVANCE_SELECTION_TEST(
	FPassControlAdvanceNoFormulaPlanTest,
	"DoesNotGenerateFormulaPlan")

bool FPassControlAdvanceNoFormulaPlanTest::RunTest(
	const FString& Parameters)
{
	using namespace PassControlAdvanceSelectionQueryTests;
	FString Source;
	TestTrue(TEXT("Query source loads"), LoadQuerySource(Source));
	TestFalse(
		TEXT("Query has no Formula Plan"),
		Source.Contains(TEXT("FormulaPlan"))
			|| Source.Contains(TEXT("Formula Plan")));
	TestFalse(
		TEXT("Query does not freeze FormulaType"),
		Source.Contains(TEXT("FormulaType")));
	return true;
}

PASS_CONTROL_ADVANCE_SELECTION_TEST(
	FPassControlAdvanceNoFormulaChainTest,
	"DoesNotCallFormulaChain")

bool FPassControlAdvanceNoFormulaChainTest::RunTest(
	const FString& Parameters)
{
	using namespace PassControlAdvanceSelectionQueryTests;
	FString Source;
	TestTrue(TEXT("Query source loads"), LoadQuerySource(Source));
	TestFalse(
		TEXT("Query does not call Input Assembly"),
		Source.Contains(TEXT("SingleCardFormulaInputAssembly")));
	TestFalse(
		TEXT("Query does not call Resolver Input Assembler"),
		Source.Contains(TEXT("SingleCardFormulaResolverInputAssembler")));
	TestFalse(
		TEXT("Query does not call Executor"),
		Source.Contains(TEXT("SingleCardFormulaResolutionExecutor")));
	TestFalse(
		TEXT("Query does not call FormulaResolver"),
		Source.Contains(TEXT("FormulaResolver")));
	return true;
}

PASS_CONTROL_ADVANCE_SELECTION_TEST(
	FPassControlAdvanceNoPlayerAttributeReadsTest,
	"DoesNotReadPlayerAdvanceAttributes")

bool FPassControlAdvanceNoPlayerAttributeReadsTest::RunTest(
	const FString& Parameters)
{
	using namespace PassControlAdvanceSelectionQueryTests;
	FString Source;
	TestTrue(TEXT("Query source loads"), LoadQuerySource(Source));
	TestFalse(
		TEXT("Query does not read Passing"),
		Source.Contains(TEXT("Passing")));
	TestFalse(
		TEXT("Query does not read Dribbling"),
		Source.Contains(TEXT("Dribbling")));
	TestFalse(
		TEXT("Query does not read OffBall"),
		Source.Contains(TEXT("OffBall")));
	TestFalse(
		TEXT("Query does not read Tackling"),
		Source.Contains(TEXT("Tackling")));
	TestFalse(
		TEXT("Query does not read Marking"),
		Source.Contains(TEXT("Marking")));
	return true;
}

PASS_CONTROL_ADVANCE_SELECTION_TEST(
	FPassControlAdvanceSnapshotDiagnosticsTest,
	"PreservesSnapshotQueryDiagnostics")

bool FPassControlAdvanceSnapshotDiagnosticsTest::RunTest(
	const FString& Parameters)
{
	using namespace PassControlAdvanceSelectionQueryTests;
	FPassControlAdvanceSelectionQueryInput Input = MakeInput();
	Input.CarrierCardId = TEXT("PassControl.Missing");
	const FPassControlAdvanceSelectionQueryResult Result =
		FPassControlAdvanceSelectionQuery::Select(
			MakePlayerCardSnapshots(),
			MakeSkillRules(),
			Input);
	TestEqual(
		TEXT("Top-level carrier query error"),
		Result.ErrorCode,
		EPassControlAdvanceSelectionQueryErrorCode
			::CarrierSnapshotQueryFailed);
	TestEqual(
		TEXT("Nested CardNotFound is preserved"),
		Result.CarrierSnapshotQueryResult.ErrorCode,
		EPlayerCardRuleSnapshotQueryErrorCode::CardNotFound);
	TestEqual(
		TEXT("Requested missing CardId is preserved"),
		Result.CarrierSnapshotQueryResult.CardId,
		Input.CarrierCardId);
	return true;
}

PASS_CONTROL_ADVANCE_SELECTION_TEST(
	FPassControlAdvanceSkillDiagnosticsTest,
	"PreservesSkillRuleQueryDiagnostics")

bool FPassControlAdvanceSkillDiagnosticsTest::RunTest(
	const FString& Parameters)
{
	using namespace PassControlAdvanceSelectionQueryTests;
	const FPassControlAdvanceSelectionQueryResult Result =
		FPassControlAdvanceSelectionQuery::Select(
			MakePlayerCardSnapshots(),
			FSkillRuleSnapshotSet{},
			MakeInput());
	TestEqual(
		TEXT("Top-level skill query error"),
		Result.ErrorCode,
		EPassControlAdvanceSelectionQueryErrorCode
			::SkillRuleQueryFailed);
	TestEqual(
		TEXT("Nested SkillRuleNotFound is preserved"),
		Result.SkillRuleQueryResult.ErrorCode,
		ESkillRuleSnapshotQueryErrorCode::SkillRuleNotFound);
	TestEqual(
		TEXT("Missing SkillId is preserved"),
		Result.SkillRuleQueryResult.InvalidSkillId,
		SkillId);
	return true;
}

PASS_CONTROL_ADVANCE_SELECTION_TEST(
	FPassControlAdvanceForbiddenDependenciesTest,
	"HasNoForbiddenDependencies")

bool FPassControlAdvanceForbiddenDependenciesTest::RunTest(
	const FString& Parameters)
{
	using namespace PassControlAdvanceSelectionQueryTests;
	FString Source;
	TestTrue(TEXT("Query source loads"), LoadQuerySource(Source));
	const TArray<FString> ForbiddenTerms = {
		TEXT("MatchPlay"),
		TEXT("ExternalApi"),
		TEXT("FormulaAttackFlow"),
		TEXT("FormulaResolver"),
		TEXT("DataTable"),
		TEXT("Provider"),
		TEXT("CardDatabase"),
		TEXT("SkillEffect"),
		TEXT("SkillPipeline"),
		TEXT("SkillFramework"),
		TEXT("FMath::Rand"),
		TEXT("FRandomStream"),
		TEXT("PassControlPlanQuery"),
		TEXT("CutInsideShot"),
		TEXT("LongShot")
	};
	for (const FString& Term : ForbiddenTerms)
	{
		TestFalse(
			TEXT("Forbidden dependency is absent"),
			Source.Contains(Term));
	}
	return true;
}

#undef PASS_CONTROL_ADVANCE_SELECTION_TEST

#endif
