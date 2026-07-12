#if WITH_DEV_AUTOMATION_TESTS

#include "CrossPlanQuery.h"

#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace CrossPlanQueryTests
{
	const FName SkillId(TEXT("Skill.Cross.Plan"));
	const FName CarrierCardId(TEXT("Cross.Carrier"));
	const FName RunnerCardId(TEXT("Cross.Runner"));
	const FName MarkerCardId(TEXT("Cross.Marker"));
	const FName HelperCardId(TEXT("Cross.Helper"));
	const FName GoalkeeperCardId(TEXT("Cross.Goalkeeper"));
	const FName CarrierPlayerId(TEXT("Player.Cross.Carrier"));
	const FName RunnerPlayerId(TEXT("Player.Cross.Runner"));
	const FName MarkerPlayerId(TEXT("Player.Cross.Marker"));
	const FName HelperPlayerId(TEXT("Player.Cross.Helper"));
	const FName GoalkeeperPlayerId(TEXT("Player.Cross.Goalkeeper"));
	const FGuid LogId(811, 812, 813, 814);

	FPlayerAttributes MakeAttributes(
		const int32 Passing,
		const int32 Shooting,
		const int32 Strength,
		const int32 Marking,
		const int32 Tackling)
	{
		FPlayerAttributes Attributes;
		Attributes.Shooting = Shooting;
		Attributes.Dribbling = 1;
		Attributes.Passing = Passing;
		Attributes.OffBall = 1;
		Attributes.Marking = Marking;
		Attributes.Tackling = Tackling;
		Attributes.Speed = 1;
		Attributes.Strength = Strength;
		Attributes.Stamina = 1;
		Attributes.LongShot = 1;
		return Attributes;
	}

	FGoalkeeperAttributes MakeGoalkeeperAttributes(
		const int32 Reflex,
		const int32 Aerial)
	{
		FGoalkeeperAttributes Attributes;
		Attributes.Handling = 1;
		Attributes.Positioning = 1;
		Attributes.Reflex = Reflex;
		Attributes.Aerial = Aerial;
		Attributes.Anticipation = 1;
		Attributes.OneOnOne = 1;
		return Attributes;
	}

	FPlayerCardRuleSnapshot MakeOutfieldCard(
		const FName CardId,
		const EPlayerPositionType Position,
		const FPlayerAttributes& Attributes,
		const bool bOwnsSkill = false)
	{
		FPlayerCardRuleSnapshot Card;
		Card.CardId = CardId;
		Card.PositionTypes = { Position };
		Card.Attributes = Attributes;
		Card.SkillIds = bOwnsSkill ? TArray<FName>{ SkillId } : TArray<FName>{};
		return Card;
	}

	FPlayerCardRuleSnapshot MakeGoalkeeperCard(
		const FName CardId = GoalkeeperCardId)
	{
		FPlayerCardRuleSnapshot Card;
		Card.CardId = CardId;
		Card.PositionTypes = { EPlayerPositionType::Goalkeeper };
		Card.Attributes = MakeAttributes(1, 1, 1, 1, 1);
		Card.bIsGoalkeeper = true;
		Card.bHasGoalkeeperAttributes = true;
		Card.GoalkeeperAttributes = MakeGoalkeeperAttributes(6, 5);
		return Card;
	}

	FPlayerCardRuleSnapshotSet MakePlayerCardSnapshots()
	{
		FPlayerCardRuleSnapshotSet SnapshotSet;
		SnapshotSet.Cards = {
			MakeOutfieldCard(
				CarrierCardId,
				EPlayerPositionType::Midfield,
				MakeAttributes(3, 1, 1, 1, 1),
				true),
			MakeOutfieldCard(
				RunnerCardId,
				EPlayerPositionType::Attack,
				MakeAttributes(1, 6, 4, 1, 1)),
			MakeOutfieldCard(
				MarkerCardId,
				EPlayerPositionType::Defense,
				MakeAttributes(1, 1, 1, 1, 5)),
			MakeOutfieldCard(
				HelperCardId,
				EPlayerPositionType::Midfield,
				MakeAttributes(1, 1, 6, 4, 1)),
			MakeGoalkeeperCard()
		};
		return SnapshotSet;
	}

	FSkillRuleSnapshotSet MakeSkillRules(
		const ESkillRuleType SkillType = ESkillRuleType::Cross)
	{
		FSkillRuleSnapshot Rule;
		Rule.SkillId = SkillId;
		Rule.SkillType = SkillType;
		Rule.MinTriggerActionPoint = 3;
		Rule.MaxTriggerActionPoint = 6;

		FSkillRuleSnapshotSet SnapshotSet;
		SnapshotSet.SkillRules = { Rule };
		return SnapshotSet;
	}

	FCrossPlanQueryInput MakeInput(
		const ECrossPlanActualType ActualType = ECrossPlanActualType::High,
		const bool bHasHelper = false,
		const bool bUseGoalkeeper = false)
	{
		FCrossPlanQueryInput Input;
		Input.SkillId = SkillId;
		Input.ActualCrossType = ActualType;
		Input.CarrierCardId = CarrierCardId;
		Input.CarrierPlayerId = CarrierPlayerId;
		Input.RunnerCardId = RunnerCardId;
		Input.RunnerPlayerId = RunnerPlayerId;
		Input.MarkerCardId = MarkerCardId;
		Input.MarkerPlayerId = MarkerPlayerId;
		Input.bHasHelper = bHasHelper;
		Input.HelperCardId = bHasHelper ? HelperCardId : NAME_None;
		Input.HelperPlayerId = bHasHelper ? HelperPlayerId : NAME_None;
		Input.bUseGoalkeeper = bUseGoalkeeper;
		Input.GoalkeeperCardId = bUseGoalkeeper ? GoalkeeperCardId : NAME_None;
		Input.GoalkeeperPlayerId = bUseGoalkeeper
			? GoalkeeperPlayerId
			: NAME_None;
		Input.CurrentActionPoint = 4;
		Input.AttackD6 = 2;
		Input.DefenseD6 = 5;
		Input.LogId = LogId;
		Input.TurnIndex = 15;
		return Input;
	}

	bool AreInputsEqual(
		const FCrossPlanQueryInput& Left,
		const FCrossPlanQueryInput& Right)
	{
		return Left.SkillId == Right.SkillId
			&& Left.ActualCrossType == Right.ActualCrossType
			&& Left.CarrierCardId == Right.CarrierCardId
			&& Left.CarrierPlayerId == Right.CarrierPlayerId
			&& Left.RunnerCardId == Right.RunnerCardId
			&& Left.RunnerPlayerId == Right.RunnerPlayerId
			&& Left.MarkerCardId == Right.MarkerCardId
			&& Left.MarkerPlayerId == Right.MarkerPlayerId
			&& Left.bHasHelper == Right.bHasHelper
			&& Left.HelperCardId == Right.HelperCardId
			&& Left.HelperPlayerId == Right.HelperPlayerId
			&& Left.bUseGoalkeeper == Right.bUseGoalkeeper
			&& Left.GoalkeeperCardId == Right.GoalkeeperCardId
			&& Left.GoalkeeperPlayerId == Right.GoalkeeperPlayerId
			&& Left.CurrentActionPoint == Right.CurrentActionPoint
			&& Left.AttackD6 == Right.AttackD6
			&& Left.DefenseD6 == Right.DefenseD6
			&& Left.LogId == Right.LogId
			&& Left.TurnIndex == Right.TurnIndex;
	}

	bool ExpectFailure(
		FAutomationTestBase& Test,
		const FCrossPlanQueryInput& Input,
		const FPlayerCardRuleSnapshotSet& PlayerCardSnapshots,
		const FSkillRuleSnapshotSet& SkillRules,
		const ECrossPlanQueryErrorCode ExpectedError,
		const FName ExpectedInvalidField)
	{
		const FCrossPlanQueryResult Result = FCrossPlanQuery::BuildPlan(
			PlayerCardSnapshots,
			SkillRules,
			Input);
		Test.TestFalse(TEXT("Plan Query fails"), Result.bSuccess);
		Test.TestFalse(
			TEXT("Failure has no Formula Plan"),
			Result.bHasFormulaPlan);
		Test.TestEqual(
			TEXT("Failure has no decision"),
			Result.Decision,
			ECrossPlanQueryDecision::None);
		Test.TestEqual(
			TEXT("Expected error is returned"),
			Result.ErrorCode,
			ExpectedError);
		Test.TestEqual(
			TEXT("Expected invalid field is returned"),
			Result.InvalidField,
			ExpectedInvalidField);
		Test.TestFalse(
			TEXT("Failure message is not empty"),
			Result.ErrorMessage.IsEmpty());
		Test.TestTrue(
			TEXT("Failure preserves the input"),
			AreInputsEqual(Result.Input, Input));
		return true;
	}

	bool ExpectSuccess(
		FAutomationTestBase& Test,
		const ECrossPlanActualType ActualType,
		const bool bHasHelper,
		const bool bUseGoalkeeper,
		const float ExpectedAttackerModifier,
		const float ExpectedDefenderModifier)
	{
		const FCrossPlanQueryInput Input =
			MakeInput(ActualType, bHasHelper, bUseGoalkeeper);
		const FCrossPlanQueryResult Result = FCrossPlanQuery::BuildPlan(
			MakePlayerCardSnapshots(),
			MakeSkillRules(),
			Input);
		Test.TestTrue(TEXT("Plan Query succeeds"), Result.bSuccess);
		Test.TestTrue(TEXT("Success has a Formula Plan"), Result.bHasFormulaPlan);
		Test.TestEqual(
			TEXT("Success requires Formula resolution"),
			Result.Decision,
			ECrossPlanQueryDecision::FormulaResolutionRequired);
		Test.TestEqual(
			TEXT("Success has no error"),
			Result.ErrorCode,
			ECrossPlanQueryErrorCode::None);
		Test.TestEqual(
			TEXT("Plan retains actual Cross type"),
			Result.FormulaPlan.ActualCrossType,
			ActualType);
		Test.TestEqual(
			TEXT("Attacker FormulaType is Finishing"),
			Result.FormulaPlan.AttackerQueryInput.FormulaType,
			EFormulaType::Finishing);
		Test.TestEqual(
			TEXT("Defender FormulaType is Finishing"),
			Result.FormulaPlan.DefenderQueryInput.FormulaType,
			EFormulaType::Finishing);
		Test.TestEqual(
			TEXT("Attacker uses Carrier Passing"),
			Result.FormulaPlan.AttackerQueryInput.Attribute,
			ESingleCardFormulaAttribute::Passing);
		Test.TestEqual(
			TEXT("Defender uses Marker Tackling"),
			Result.FormulaPlan.DefenderQueryInput.Attribute,
			ESingleCardFormulaAttribute::Tackling);
		Test.TestEqual(
			TEXT("Attack D6 is retained"),
			Result.FormulaPlan.AttackerQueryInput.ExternalD6ComparePoint,
			Input.AttackD6);
		Test.TestEqual(
			TEXT("Defense D6 is retained"),
			Result.FormulaPlan.DefenderQueryInput.ExternalD6ComparePoint,
			Input.DefenseD6);
		Test.TestEqual(
			TEXT("Attacker modifier preserves half steps"),
			Result.FormulaPlan.AttackerQueryInput.ExternalModifier,
			ExpectedAttackerModifier);
		Test.TestEqual(
			TEXT("Defender modifier preserves half steps"),
			Result.FormulaPlan.DefenderQueryInput.ExternalModifier,
			ExpectedDefenderModifier);
		Test.TestEqual(
			TEXT("Runner is the goal scorer card"),
			Result.FormulaPlan.GoalScorerCardId,
			Input.RunnerCardId);
		Test.TestEqual(
			TEXT("Runner is the goal scorer player"),
			Result.FormulaPlan.GoalScorerPlayerId,
			Input.RunnerPlayerId);
		Test.TestEqual(
			TEXT("Helper presence is retained"),
			Result.FormulaPlan.bHasHelper,
			bHasHelper);
		Test.TestEqual(
			TEXT("Carrier identity is retained"),
			Result.FormulaPlan.CarrierPlayerId,
			Input.CarrierPlayerId);
		Test.TestEqual(
			TEXT("Runner identity is retained"),
			Result.FormulaPlan.RunnerPlayerId,
			Input.RunnerPlayerId);
		Test.TestEqual(
			TEXT("Marker identity is retained"),
			Result.FormulaPlan.MarkerPlayerId,
			Input.MarkerPlayerId);
		Test.TestEqual(
			TEXT("Goalkeeper presence is retained"),
			Result.FormulaPlan.bUseGoalkeeper,
			bUseGoalkeeper);
		if (bHasHelper)
		{
			Test.TestTrue(
				TEXT("Selected Helper Snapshot query succeeds"),
				Result.HelperSnapshotQueryResult.bSuccess);
		}
		else
		{
			Test.TestFalse(
				TEXT("Unselected Helper Snapshot query is skipped"),
				Result.HelperSnapshotQueryResult.bSuccess);
		}
		if (bUseGoalkeeper)
		{
			Test.TestTrue(
				TEXT("Selected Goalkeeper Snapshot query succeeds"),
				Result.GoalkeeperSnapshotQueryResult.bSuccess);
		}
		else
		{
			Test.TestFalse(
				TEXT("Unselected Goalkeeper Snapshot query is skipped"),
				Result.GoalkeeperSnapshotQueryResult.bSuccess);
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
			*FPaths::Combine(Directory, TEXT("CrossPlanQuery.h")));
		const bool bImplementationLoaded = FFileHelper::LoadFileToString(
			Implementation,
			*FPaths::Combine(Directory, TEXT("CrossPlanQuery.cpp")));
		OutSource = Header + Implementation;
		return bHeaderLoaded && bImplementationLoaded;
	}
}

#define CROSS_PLAN_TEST(ClassName, TestName) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
		ClassName, \
		"FMCodex.CoreRules.CrossPlanQuery." TestName, \
		EAutomationTestFlags::EditorContext \
			| EAutomationTestFlags::EngineFilter)

#define CROSS_PLAN_SUCCESS_TEST( \
	ClassName, TestName, ActualType, HasHelper, UseGoalkeeper, AttackModifier, DefenseModifier) \
	CROSS_PLAN_TEST(ClassName, TestName) \
	bool ClassName::RunTest(const FString& Parameters) \
	{ \
		return CrossPlanQueryTests::ExpectSuccess( \
			*this, ActualType, HasHelper, UseGoalkeeper, AttackModifier, DefenseModifier); \
	}

CROSS_PLAN_SUCCESS_TEST(
	FCrossPlanHighNoHelperNoGoalkeeperTest,
	"HighWithoutHelperOrGoalkeeperBuildsPlan",
	ECrossPlanActualType::High,
	false,
	false,
	0.5f,
	-0.5f)

CROSS_PLAN_SUCCESS_TEST(
	FCrossPlanHighHelperNoGoalkeeperTest,
	"HighWithHelperWithoutGoalkeeperBuildsPlan",
	ECrossPlanActualType::High,
	true,
	false,
	0.5f,
	2.5f)

CROSS_PLAN_SUCCESS_TEST(
	FCrossPlanHighNoHelperGoalkeeperTest,
	"HighWithoutHelperWithGoalkeeperBuildsPlan",
	ECrossPlanActualType::High,
	false,
	true,
	0.5f,
	2.0f)

CROSS_PLAN_SUCCESS_TEST(
	FCrossPlanHighHelperGoalkeeperTest,
	"HighWithHelperAndGoalkeeperBuildsPlan",
	ECrossPlanActualType::High,
	true,
	true,
	0.5f,
	5.0f)

CROSS_PLAN_SUCCESS_TEST(
	FCrossPlanLowNoHelperNoGoalkeeperTest,
	"LowWithoutHelperOrGoalkeeperBuildsPlan",
	ECrossPlanActualType::Low,
	false,
	false,
	1.5f,
	-0.5f)

CROSS_PLAN_SUCCESS_TEST(
	FCrossPlanLowHelperNoGoalkeeperTest,
	"LowWithHelperWithoutGoalkeeperBuildsPlan",
	ECrossPlanActualType::Low,
	true,
	false,
	1.5f,
	1.5f)

CROSS_PLAN_SUCCESS_TEST(
	FCrossPlanLowNoHelperGoalkeeperTest,
	"LowWithoutHelperWithGoalkeeperBuildsPlan",
	ECrossPlanActualType::Low,
	false,
	true,
	1.5f,
	2.5f)

CROSS_PLAN_SUCCESS_TEST(
	FCrossPlanLowHelperGoalkeeperTest,
	"LowWithHelperAndGoalkeeperBuildsPlan",
	ECrossPlanActualType::Low,
	true,
	true,
	1.5f,
	4.5f)

CROSS_PLAN_TEST(
	FCrossPlanAttackD6BoundariesTest,
	"AcceptsAttackD6Boundaries")

bool FCrossPlanAttackD6BoundariesTest::RunTest(const FString& Parameters)
{
	using namespace CrossPlanQueryTests;
	for (const int32 AttackD6 : { 1, 6 })
	{
		FCrossPlanQueryInput Input = MakeInput();
		Input.AttackD6 = AttackD6;
		const FCrossPlanQueryResult Result = FCrossPlanQuery::BuildPlan(
			MakePlayerCardSnapshots(), MakeSkillRules(), Input);
		TestTrue(TEXT("Attack D6 boundary succeeds"), Result.bSuccess);
		TestEqual(
			TEXT("Attack D6 boundary is retained"),
			Result.FormulaPlan.AttackerQueryInput.ExternalD6ComparePoint,
			AttackD6);
	}
	return true;
}

CROSS_PLAN_TEST(
	FCrossPlanDefenseD6BoundariesTest,
	"AcceptsDefenseD6Boundaries")

bool FCrossPlanDefenseD6BoundariesTest::RunTest(const FString& Parameters)
{
	using namespace CrossPlanQueryTests;
	for (const int32 DefenseD6 : { 1, 6 })
	{
		FCrossPlanQueryInput Input = MakeInput();
		Input.DefenseD6 = DefenseD6;
		const FCrossPlanQueryResult Result = FCrossPlanQuery::BuildPlan(
			MakePlayerCardSnapshots(), MakeSkillRules(), Input);
		TestTrue(TEXT("Defense D6 boundary succeeds"), Result.bSuccess);
		TestEqual(
			TEXT("Defense D6 boundary is retained"),
			Result.FormulaPlan.DefenderQueryInput.ExternalD6ComparePoint,
			DefenseD6);
	}
	return true;
}

CROSS_PLAN_TEST(FCrossPlanMissingSkillIdTest, "RejectsMissingSkillId")

bool FCrossPlanMissingSkillIdTest::RunTest(const FString& Parameters)
{
	using namespace CrossPlanQueryTests;
	FCrossPlanQueryInput Input = MakeInput();
	Input.SkillId = NAME_None;
	return ExpectFailure(
		*this, Input, MakePlayerCardSnapshots(), MakeSkillRules(),
		ECrossPlanQueryErrorCode::MissingSkillId, TEXT("SkillId"));
}

CROSS_PLAN_TEST(FCrossPlanInvalidActualTypeTest, "RejectsInvalidActualCrossType")

bool FCrossPlanInvalidActualTypeTest::RunTest(const FString& Parameters)
{
	using namespace CrossPlanQueryTests;
	FCrossPlanQueryInput Input = MakeInput();
	Input.ActualCrossType = static_cast<ECrossPlanActualType>(255);
	return ExpectFailure(
		*this, Input, MakePlayerCardSnapshots(), MakeSkillRules(),
		ECrossPlanQueryErrorCode::InvalidActualCrossType,
		TEXT("ActualCrossType"));
}

#define CROSS_PLAN_REQUIRED_IDENTITY_TEST( \
	ClassName, TestName, Field, ErrorCode, FieldName) \
	CROSS_PLAN_TEST(ClassName, TestName) \
	bool ClassName::RunTest(const FString& Parameters) \
	{ \
		using namespace CrossPlanQueryTests; \
		FCrossPlanQueryInput Input = MakeInput(); \
		Input.Field = NAME_None; \
		return ExpectFailure( \
			*this, Input, MakePlayerCardSnapshots(), MakeSkillRules(), ErrorCode, TEXT(FieldName)); \
	}

CROSS_PLAN_REQUIRED_IDENTITY_TEST(
	FCrossPlanMissingCarrierIdentityTest,
	"RejectsMissingCarrierIdentity",
	CarrierCardId,
	ECrossPlanQueryErrorCode::MissingCarrierIdentity,
	"CarrierCardId")

CROSS_PLAN_REQUIRED_IDENTITY_TEST(
	FCrossPlanMissingRunnerIdentityTest,
	"RejectsMissingRunnerIdentity",
	RunnerPlayerId,
	ECrossPlanQueryErrorCode::MissingRunnerIdentity,
	"RunnerPlayerId")

CROSS_PLAN_REQUIRED_IDENTITY_TEST(
	FCrossPlanMissingMarkerIdentityTest,
	"RejectsMissingMarkerIdentity",
	MarkerCardId,
	ECrossPlanQueryErrorCode::MissingMarkerIdentity,
	"MarkerCardId")

CROSS_PLAN_TEST(FCrossPlanHelperPresenceTest, "ValidatesHelperPresenceContract")

bool FCrossPlanHelperPresenceTest::RunTest(const FString& Parameters)
{
	using namespace CrossPlanQueryTests;
	FCrossPlanQueryInput UnexpectedInput = MakeInput();
	UnexpectedInput.HelperCardId = HelperCardId;
	ExpectFailure(
		*this, UnexpectedInput, MakePlayerCardSnapshots(), MakeSkillRules(),
		ECrossPlanQueryErrorCode::UnexpectedHelperIdentity,
		TEXT("HelperCardId"));
	FCrossPlanQueryInput MissingInput = MakeInput();
	MissingInput.bHasHelper = true;
	return ExpectFailure(
		*this, MissingInput, MakePlayerCardSnapshots(), MakeSkillRules(),
		ECrossPlanQueryErrorCode::MissingHelperIdentity,
		TEXT("HelperCardId"));
}

CROSS_PLAN_TEST(FCrossPlanGoalkeeperPresenceTest, "ValidatesGoalkeeperPresenceContract")

bool FCrossPlanGoalkeeperPresenceTest::RunTest(const FString& Parameters)
{
	using namespace CrossPlanQueryTests;
	FCrossPlanQueryInput UnexpectedInput = MakeInput();
	UnexpectedInput.GoalkeeperPlayerId = GoalkeeperPlayerId;
	ExpectFailure(
		*this, UnexpectedInput, MakePlayerCardSnapshots(), MakeSkillRules(),
		ECrossPlanQueryErrorCode::UnexpectedGoalkeeperIdentity,
		TEXT("GoalkeeperPlayerId"));
	FCrossPlanQueryInput MissingInput = MakeInput();
	MissingInput.bUseGoalkeeper = true;
	return ExpectFailure(
		*this, MissingInput, MakePlayerCardSnapshots(), MakeSkillRules(),
		ECrossPlanQueryErrorCode::MissingGoalkeeperIdentity,
		TEXT("GoalkeeperCardId"));
}

CROSS_PLAN_TEST(FCrossPlanSnapshotFailuresTest, "PropagatesParticipantSnapshotFailures")

bool FCrossPlanSnapshotFailuresTest::RunTest(const FString& Parameters)
{
	using namespace CrossPlanQueryTests;
	struct FSnapshotFailureCase
	{
		FName FCrossPlanQueryInput::* CardField;
		ECrossPlanQueryErrorCode ErrorCode;
		FName InvalidField;
		bool bHasHelper;
		bool bUseGoalkeeper;
	};
	const TArray<FSnapshotFailureCase> Cases = {
		{ &FCrossPlanQueryInput::CarrierCardId, ECrossPlanQueryErrorCode::CarrierSnapshotQueryFailed, TEXT("CarrierCardId"), false, false },
		{ &FCrossPlanQueryInput::RunnerCardId, ECrossPlanQueryErrorCode::RunnerSnapshotQueryFailed, TEXT("RunnerCardId"), false, false },
		{ &FCrossPlanQueryInput::MarkerCardId, ECrossPlanQueryErrorCode::MarkerSnapshotQueryFailed, TEXT("MarkerCardId"), false, false },
		{ &FCrossPlanQueryInput::HelperCardId, ECrossPlanQueryErrorCode::HelperSnapshotQueryFailed, TEXT("HelperCardId"), true, false },
		{ &FCrossPlanQueryInput::GoalkeeperCardId, ECrossPlanQueryErrorCode::GoalkeeperSnapshotQueryFailed, TEXT("GoalkeeperCardId"), false, true }
	};
	for (const FSnapshotFailureCase& FailureCase : Cases)
	{
		FCrossPlanQueryInput Input = MakeInput(
			ECrossPlanActualType::High,
			FailureCase.bHasHelper,
			FailureCase.bUseGoalkeeper);
		Input.*(FailureCase.CardField) = TEXT("Cross.Missing");
		ExpectFailure(
			*this, Input, MakePlayerCardSnapshots(), MakeSkillRules(),
			FailureCase.ErrorCode, FailureCase.InvalidField);
	}
	return true;
}

CROSS_PLAN_TEST(FCrossPlanSkillRuleFailuresTest, "ValidatesSkillRuleContract")

bool FCrossPlanSkillRuleFailuresTest::RunTest(const FString& Parameters)
{
	using namespace CrossPlanQueryTests;
	const FCrossPlanQueryInput Input = MakeInput();
	ExpectFailure(
		*this, Input, MakePlayerCardSnapshots(), FSkillRuleSnapshotSet{},
		ECrossPlanQueryErrorCode::SkillRuleQueryFailed, TEXT("SkillId"));
	ExpectFailure(
		*this, Input, MakePlayerCardSnapshots(), MakeSkillRules(ESkillRuleType::LongShot),
		ECrossPlanQueryErrorCode::SkillRuleTypeMismatch, TEXT("SkillType"));
	FPlayerCardRuleSnapshotSet NoSkillSnapshots = MakePlayerCardSnapshots();
	NoSkillSnapshots.Cards[0].SkillIds.Reset();
	return ExpectFailure(
		*this, Input, NoSkillSnapshots, MakeSkillRules(),
		ECrossPlanQueryErrorCode::CarrierDoesNotOwnSkill, TEXT("SkillId"));
}

CROSS_PLAN_TEST(FCrossPlanActionPointFailuresTest, "ValidatesActionPointContract")

bool FCrossPlanActionPointFailuresTest::RunTest(const FString& Parameters)
{
	using namespace CrossPlanQueryTests;
	FCrossPlanQueryInput InvalidInput = MakeInput();
	InvalidInput.CurrentActionPoint = 1;
	ExpectFailure(
		*this, InvalidInput, MakePlayerCardSnapshots(), MakeSkillRules(),
		ECrossPlanQueryErrorCode::InvalidCurrentActionPoint,
		TEXT("CurrentActionPoint"));
	FCrossPlanQueryInput OutOfRangeInput = MakeInput();
	OutOfRangeInput.CurrentActionPoint = 7;
	return ExpectFailure(
		*this, OutOfRangeInput, MakePlayerCardSnapshots(), MakeSkillRules(),
		ECrossPlanQueryErrorCode::ActionPointOutOfRange,
		TEXT("CurrentActionPoint"));
}

CROSS_PLAN_TEST(FCrossPlanGoalkeeperEligibilityTest, "ValidatesGoalkeeperEligibility")

bool FCrossPlanGoalkeeperEligibilityTest::RunTest(const FString& Parameters)
{
	using namespace CrossPlanQueryTests;
	FPlayerCardRuleSnapshotSet CarrierSnapshots = MakePlayerCardSnapshots();
	CarrierSnapshots.Cards[0] = MakeGoalkeeperCard(CarrierCardId);
	CarrierSnapshots.Cards[0].SkillIds = { SkillId };
	ExpectFailure(
		*this, MakeInput(), CarrierSnapshots, MakeSkillRules(),
		ECrossPlanQueryErrorCode::UnsupportedGoalkeeperCarrier,
		TEXT("CarrierCardId"));
	FPlayerCardRuleSnapshotSet RunnerSnapshots = MakePlayerCardSnapshots();
	RunnerSnapshots.Cards[1] = MakeGoalkeeperCard(RunnerCardId);
	ExpectFailure(
		*this, MakeInput(), RunnerSnapshots, MakeSkillRules(),
		ECrossPlanQueryErrorCode::UnsupportedGoalkeeperRunner,
		TEXT("RunnerCardId"));
	FPlayerCardRuleSnapshotSet MarkerSnapshots = MakePlayerCardSnapshots();
	MarkerSnapshots.Cards[2] = MakeGoalkeeperCard(MarkerCardId);
	ExpectFailure(
		*this, MakeInput(), MarkerSnapshots, MakeSkillRules(),
		ECrossPlanQueryErrorCode::UnsupportedGoalkeeperMarker,
		TEXT("MarkerCardId"));
	FPlayerCardRuleSnapshotSet HelperSnapshots = MakePlayerCardSnapshots();
	HelperSnapshots.Cards[3] = MakeGoalkeeperCard(HelperCardId);
	return ExpectFailure(
		*this, MakeInput(ECrossPlanActualType::High, true), HelperSnapshots, MakeSkillRules(),
		ECrossPlanQueryErrorCode::UnsupportedGoalkeeperHelper,
		TEXT("HelperCardId"));
}

CROSS_PLAN_TEST(FCrossPlanRunnerAndSelectedGoalkeeperEligibilityTest, "ValidatesRunnerAndSelectedGoalkeeperEligibility")

bool FCrossPlanRunnerAndSelectedGoalkeeperEligibilityTest::RunTest(
	const FString& Parameters)
{
	using namespace CrossPlanQueryTests;
	FPlayerCardRuleSnapshotSet RunnerSnapshots = MakePlayerCardSnapshots();
	RunnerSnapshots.Cards[1].PositionTypes = { EPlayerPositionType::Midfield };
	ExpectFailure(
		*this, MakeInput(), RunnerSnapshots, MakeSkillRules(),
		ECrossPlanQueryErrorCode::RunnerNotForward, TEXT("RunnerCardId"));
	FPlayerCardRuleSnapshotSet GoalkeeperSnapshots = MakePlayerCardSnapshots();
	GoalkeeperSnapshots.Cards[4] = MakeOutfieldCard(
		GoalkeeperCardId,
		EPlayerPositionType::Defense,
		MakeAttributes(1, 1, 1, 1, 1));
	return ExpectFailure(
		*this, MakeInput(ECrossPlanActualType::High, false, true),
		GoalkeeperSnapshots, MakeSkillRules(),
		ECrossPlanQueryErrorCode::GoalkeeperSnapshotNotGoalkeeper,
		TEXT("GoalkeeperCardId"));
}

CROSS_PLAN_TEST(FCrossPlanIdentityConflictsTest, "ValidatesCanonicalIdentityConflicts")

bool FCrossPlanIdentityConflictsTest::RunTest(const FString& Parameters)
{
	using namespace CrossPlanQueryTests;
	FCrossPlanQueryInput CarrierRunnerInput = MakeInput();
	CarrierRunnerInput.RunnerPlayerId = CarrierPlayerId;
	ExpectFailure(
		*this, CarrierRunnerInput, MakePlayerCardSnapshots(), MakeSkillRules(),
		ECrossPlanQueryErrorCode::CarrierRunnerIdentityConflict,
		TEXT("RunnerCardId"));
	FCrossPlanQueryInput MarkerHelperInput = MakeInput(ECrossPlanActualType::High, true);
	MarkerHelperInput.HelperPlayerId = MarkerPlayerId;
	ExpectFailure(
		*this, MarkerHelperInput, MakePlayerCardSnapshots(), MakeSkillRules(),
		ECrossPlanQueryErrorCode::MarkerHelperIdentityConflict,
		TEXT("HelperCardId"));
	FCrossPlanQueryInput MarkerGoalkeeperInput = MakeInput(
		ECrossPlanActualType::High, false, true);
	MarkerGoalkeeperInput.GoalkeeperPlayerId = MarkerPlayerId;
	ExpectFailure(
		*this, MarkerGoalkeeperInput, MakePlayerCardSnapshots(), MakeSkillRules(),
		ECrossPlanQueryErrorCode::MarkerGoalkeeperIdentityConflict,
		TEXT("GoalkeeperCardId"));
	FCrossPlanQueryInput HelperGoalkeeperInput = MakeInput(
		ECrossPlanActualType::High, true, true);
	HelperGoalkeeperInput.GoalkeeperPlayerId = HelperPlayerId;
	return ExpectFailure(
		*this, HelperGoalkeeperInput, MakePlayerCardSnapshots(), MakeSkillRules(),
		ECrossPlanQueryErrorCode::HelperGoalkeeperIdentityConflict,
		TEXT("GoalkeeperCardId"));
}

CROSS_PLAN_TEST(FCrossPlanD6FailuresTest, "ValidatesExternalD6Ranges")

bool FCrossPlanD6FailuresTest::RunTest(const FString& Parameters)
{
	using namespace CrossPlanQueryTests;
	for (const int32 InvalidD6 : { 0, 7 })
	{
		FCrossPlanQueryInput AttackInput = MakeInput();
		AttackInput.AttackD6 = InvalidD6;
		ExpectFailure(
			*this, AttackInput, MakePlayerCardSnapshots(), MakeSkillRules(),
			ECrossPlanQueryErrorCode::InvalidAttackD6, TEXT("AttackD6"));
		FCrossPlanQueryInput DefenseInput = MakeInput();
		DefenseInput.DefenseD6 = InvalidD6;
		ExpectFailure(
			*this, DefenseInput, MakePlayerCardSnapshots(), MakeSkillRules(),
			ECrossPlanQueryErrorCode::InvalidDefenseD6, TEXT("DefenseD6"));
	}
	return true;
}

CROSS_PLAN_TEST(FCrossPlanLogContextFailuresTest, "ValidatesLogContext")

bool FCrossPlanLogContextFailuresTest::RunTest(const FString& Parameters)
{
	using namespace CrossPlanQueryTests;
	FCrossPlanQueryInput InvalidLogInput = MakeInput();
	InvalidLogInput.LogId.Invalidate();
	ExpectFailure(
		*this, InvalidLogInput, MakePlayerCardSnapshots(), MakeSkillRules(),
		ECrossPlanQueryErrorCode::InvalidLogContext, TEXT("LogId"));
	FCrossPlanQueryInput InvalidTurnInput = MakeInput();
	InvalidTurnInput.TurnIndex = -1;
	return ExpectFailure(
		*this, InvalidTurnInput, MakePlayerCardSnapshots(), MakeSkillRules(),
		ECrossPlanQueryErrorCode::InvalidLogContext, TEXT("TurnIndex"));
}

CROSS_PLAN_TEST(FCrossPlanInputImmutabilityTest, "DoesNotMutateInput")

bool FCrossPlanInputImmutabilityTest::RunTest(const FString& Parameters)
{
	using namespace CrossPlanQueryTests;
	const FCrossPlanQueryInput Input = MakeInput(
		ECrossPlanActualType::Low, true, true);
	const FCrossPlanQueryInput InputBefore = Input;
	const FCrossPlanQueryResult Result = FCrossPlanQuery::BuildPlan(
		MakePlayerCardSnapshots(), MakeSkillRules(), Input);
	TestTrue(TEXT("Plan Query succeeds"), Result.bSuccess);
	TestTrue(TEXT("Input remains unchanged"), AreInputsEqual(Input, InputBefore));
	TestTrue(TEXT("Result preserves input copy"), AreInputsEqual(Result.Input, InputBefore));
	return true;
}

CROSS_PLAN_TEST(FCrossPlanForbiddenDependenciesTest, "HasNoForbiddenDependencies")

bool FCrossPlanForbiddenDependenciesTest::RunTest(const FString& Parameters)
{
	using namespace CrossPlanQueryTests;
	FString Source;
	TestTrue(TEXT("Cross Plan source loads"), LoadQuerySource(Source));
	const TArray<FString> ForbiddenTerms = {
		TEXT("CrossSelectionQuery"),
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
		TEXT("GoalResult"),
		TEXT("ScoreUpdate"),
		TEXT("Composition")
	};
	for (const FString& Term : ForbiddenTerms)
	{
		TestFalse(TEXT("Forbidden dependency is absent"), Source.Contains(Term));
	}
	return true;
}

#undef CROSS_PLAN_REQUIRED_IDENTITY_TEST
#undef CROSS_PLAN_SUCCESS_TEST
#undef CROSS_PLAN_TEST

#endif
