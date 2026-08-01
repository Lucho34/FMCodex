#include "MatchPlayCurrentAttackInitialRouteMappingQuery.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "MatchPlayCurrentAttackResolutionSessionTestFixtures.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace SessionFixtures =
	FMCodex::Tests::MatchPlayCurrentAttackResolutionSession;

namespace MatchPlayCurrentAttackInitialRouteStateTests
{
	struct FMappingCase
	{
		const TCHAR* Label = TEXT("");
		ESkillRuleType ActionType = ESkillRuleType::None;
		EMatchPlayElectiveBranchIntent Intent =
			EMatchPlayElectiveBranchIntent::None;
		bool bHasD6 = false;
		int32 D6 = 0;
		FMatchPlayCurrentAttackActualBranch Expected;
	};

	FMatchPlayCurrentAttackActualBranch MakeLongShotBranch(
		const EMatchPlayLongShotActualBranch Branch)
	{
		FMatchPlayCurrentAttackActualBranch Result;
		Result.ActionType = ESkillRuleType::LongShot;
		Result.LongShot = Branch;
		return Result;
	}

	FMatchPlayCurrentAttackActualBranch MakeCutInsideShotBranch(
		const EMatchPlayCutInsideShotActualBranch Branch)
	{
		FMatchPlayCurrentAttackActualBranch Result;
		Result.ActionType = ESkillRuleType::CutInsideShot;
		Result.CutInsideShot = Branch;
		return Result;
	}

	FMatchPlayCurrentAttackActualBranch MakeCrossBranch(
		const EMatchPlayCrossActualBranch Branch)
	{
		FMatchPlayCurrentAttackActualBranch Result;
		Result.ActionType = ESkillRuleType::Cross;
		Result.Cross = Branch;
		return Result;
	}

	FMatchPlayCurrentAttackActualBranch MakePassControlBranch(
		const EMatchPlayPassControlActualBranch Branch)
	{
		FMatchPlayCurrentAttackActualBranch Result;
		Result.ActionType = ESkillRuleType::PassControl;
		Result.PassControl = Branch;
		return Result;
	}

	FMatchPlayCurrentAttackActualBranch MakeThroughBallBranch(
		const EMatchPlayThroughBallActualBranch Branch)
	{
		FMatchPlayCurrentAttackActualBranch Result;
		Result.ActionType = ESkillRuleType::ThroughBall;
		Result.ThroughBall = Branch;
		return Result;
	}

	TArray<FMappingCase> MakeMappingCases()
	{
		return {
			{TEXT("LongShot DirectShot"), ESkillRuleType::LongShot,
				EMatchPlayElectiveBranchIntent::DirectShot, false, 0,
				MakeLongShotBranch(
					EMatchPlayLongShotActualBranch::DirectShot)},
			{TEXT("LongShot DeadCorner"), ESkillRuleType::LongShot,
				EMatchPlayElectiveBranchIntent::DeadCorner, false, 0,
				MakeLongShotBranch(
					EMatchPlayLongShotActualBranch::DeadCorner)},
			{TEXT("CutInside DirectShot"), ESkillRuleType::CutInsideShot,
				EMatchPlayElectiveBranchIntent::DirectShot, false, 0,
				MakeCutInsideShotBranch(
					EMatchPlayCutInsideShotActualBranch::DirectShot)},
			{TEXT("CutInside DeadCorner"), ESkillRuleType::CutInsideShot,
				EMatchPlayElectiveBranchIntent::DeadCorner, false, 0,
				MakeCutInsideShotBranch(
					EMatchPlayCutInsideShotActualBranch::DeadCorner)},
			{TEXT("Cross High"), ESkillRuleType::Cross,
				EMatchPlayElectiveBranchIntent::CrossHigh, true, 1,
				MakeCrossBranch(EMatchPlayCrossActualBranch::High)},
			{TEXT("Cross Low"), ESkillRuleType::Cross,
				EMatchPlayElectiveBranchIntent::CrossHigh, true, 5,
				MakeCrossBranch(EMatchPlayCrossActualBranch::Low)},
			{TEXT("PassAdvance"), ESkillRuleType::PassControl,
				EMatchPlayElectiveBranchIntent::None, true, 1,
				MakePassControlBranch(
					EMatchPlayPassControlActualBranch::PassAdvance)},
			{TEXT("DribbleAdvance"), ESkillRuleType::PassControl,
				EMatchPlayElectiveBranchIntent::None, true, 3,
				MakePassControlBranch(
					EMatchPlayPassControlActualBranch::DribbleAdvance)},
			{TEXT("RunAdvance"), ESkillRuleType::PassControl,
				EMatchPlayElectiveBranchIntent::None, true, 5,
				MakePassControlBranch(
					EMatchPlayPassControlActualBranch::RunAdvance)},
			{TEXT("Feet"), ESkillRuleType::ThroughBall,
				EMatchPlayElectiveBranchIntent::None, true, 1,
				MakeThroughBallBranch(
					EMatchPlayThroughBallActualBranch::Feet)},
			{TEXT("BehindDefense"), ESkillRuleType::ThroughBall,
				EMatchPlayElectiveBranchIntent::None, true, 3,
				MakeThroughBallBranch(
					EMatchPlayThroughBallActualBranch::BehindDefense)},
			{TEXT("AntiOffside"), ESkillRuleType::ThroughBall,
				EMatchPlayElectiveBranchIntent::None, true, 5,
				MakeThroughBallBranch(
					EMatchPlayThroughBallActualBranch::AntiOffside)},
			{TEXT("Cross High D6 2"), ESkillRuleType::Cross,
				EMatchPlayElectiveBranchIntent::CrossHigh, true, 2,
				MakeCrossBranch(EMatchPlayCrossActualBranch::High)},
			{TEXT("Cross High D6 3"), ESkillRuleType::Cross,
				EMatchPlayElectiveBranchIntent::CrossHigh, true, 3,
				MakeCrossBranch(EMatchPlayCrossActualBranch::High)},
			{TEXT("Cross High D6 4"), ESkillRuleType::Cross,
				EMatchPlayElectiveBranchIntent::CrossHigh, true, 4,
				MakeCrossBranch(EMatchPlayCrossActualBranch::High)},
			{TEXT("Cross High D6 6"), ESkillRuleType::Cross,
				EMatchPlayElectiveBranchIntent::CrossHigh, true, 6,
				MakeCrossBranch(EMatchPlayCrossActualBranch::Low)},
			{TEXT("Cross Low D6 1"), ESkillRuleType::Cross,
				EMatchPlayElectiveBranchIntent::CrossLow, true, 1,
				MakeCrossBranch(EMatchPlayCrossActualBranch::Low)},
			{TEXT("Cross Low D6 2"), ESkillRuleType::Cross,
				EMatchPlayElectiveBranchIntent::CrossLow, true, 2,
				MakeCrossBranch(EMatchPlayCrossActualBranch::Low)},
			{TEXT("Cross Low D6 3"), ESkillRuleType::Cross,
				EMatchPlayElectiveBranchIntent::CrossLow, true, 3,
				MakeCrossBranch(EMatchPlayCrossActualBranch::Low)},
			{TEXT("Cross Low D6 4"), ESkillRuleType::Cross,
				EMatchPlayElectiveBranchIntent::CrossLow, true, 4,
				MakeCrossBranch(EMatchPlayCrossActualBranch::Low)},
			{TEXT("Cross Low D6 5"), ESkillRuleType::Cross,
				EMatchPlayElectiveBranchIntent::CrossLow, true, 5,
				MakeCrossBranch(EMatchPlayCrossActualBranch::High)},
			{TEXT("Cross Low D6 6"), ESkillRuleType::Cross,
				EMatchPlayElectiveBranchIntent::CrossLow, true, 6,
				MakeCrossBranch(EMatchPlayCrossActualBranch::High)},
			{TEXT("PassAdvance D6 2"), ESkillRuleType::PassControl,
				EMatchPlayElectiveBranchIntent::None, true, 2,
				MakePassControlBranch(
					EMatchPlayPassControlActualBranch::PassAdvance)},
			{TEXT("DribbleAdvance D6 4"), ESkillRuleType::PassControl,
				EMatchPlayElectiveBranchIntent::None, true, 4,
				MakePassControlBranch(
					EMatchPlayPassControlActualBranch::DribbleAdvance)},
			{TEXT("RunAdvance D6 6"), ESkillRuleType::PassControl,
				EMatchPlayElectiveBranchIntent::None, true, 6,
				MakePassControlBranch(
					EMatchPlayPassControlActualBranch::RunAdvance)},
			{TEXT("Feet D6 2"), ESkillRuleType::ThroughBall,
				EMatchPlayElectiveBranchIntent::None, true, 2,
				MakeThroughBallBranch(
					EMatchPlayThroughBallActualBranch::Feet)},
			{TEXT("BehindDefense D6 4"), ESkillRuleType::ThroughBall,
				EMatchPlayElectiveBranchIntent::None, true, 4,
				MakeThroughBallBranch(
					EMatchPlayThroughBallActualBranch::BehindDefense)},
			{TEXT("AntiOffside D6 6"), ESkillRuleType::ThroughBall,
				EMatchPlayElectiveBranchIntent::None, true, 6,
				MakeThroughBallBranch(
					EMatchPlayThroughBallActualBranch::AntiOffside)}
		};
	}

	FMatchPlayCurrentAttackInitialRouteMappingInput MakeMappingInput(
		const FMappingCase& Case)
	{
		FMatchPlayCurrentAttackInitialRouteMappingInput Input;
		Input.ActionType = Case.ActionType;
		Input.Intent = Case.Intent;
		Input.bHasInitialRouteD6 = Case.bHasD6;
		Input.InitialRouteD6 = Case.D6;
		return Input;
	}

	bool AreInputsEqual(
		const FMatchPlayCurrentAttackInitialRouteMappingInput& Left,
		const FMatchPlayCurrentAttackInitialRouteMappingInput& Right)
	{
		return Left.ActionType == Right.ActionType
			&& Left.Intent == Right.Intent
			&& Left.bHasInitialRouteD6 == Right.bHasInitialRouteD6
			&& Left.InitialRouteD6 == Right.InitialRouteD6;
	}

	bool AreResultsEqual(
		const FMatchPlayCurrentAttackInitialRouteMappingResult& Left,
		const FMatchPlayCurrentAttackInitialRouteMappingResult& Right)
	{
		return Left.bSuccess == Right.bSuccess
			&& FMatchPlayCurrentAttackActualBranch::StaticStruct()
				->CompareScriptStruct(
					&Left.ActualBranch,
					&Right.ActualBranch,
					0)
			&& Left.ErrorCode == Right.ErrorCode
			&& Left.ErrorMessage == Right.ErrorMessage
			&& Left.InvalidField == Right.InvalidField
			&& AreInputsEqual(Left.Input, Right.Input);
	}

	void SetIntent(
		FMatchPlayState& State,
		const EMatchPlayElectiveBranchIntent Intent)
	{
		State.CurrentAttack.SelectedAction.ElectiveBranchIntent = Intent;
		if (State.CurrentAttack.bHasResolutionSession)
		{
			State.CurrentAttack.ResolutionSession.Bundle.Binding
				.ElectiveBranchIntent = Intent;
		}
	}

	FMatchPlayState MakeBegunState(
		const ESkillRuleType ActionType,
		const EMatchPlayElectiveBranchIntent Intent)
	{
		FMatchPlayState ReadyState =
			SessionFixtures::MakeReadyState(ActionType);
		ReadyState.CurrentAttack.SelectedAction.ElectiveBranchIntent =
			Intent;
		return FMatchPlayCurrentAttackBeginResolutionSessionWriter::Begin(
			ReadyState,
			SessionFixtures::MakeRequest(ReadyState)).AfterState;
	}

	FMatchPlayState MakeRouteResolvedState(const FMappingCase& Case)
	{
		FMatchPlayState State = MakeBegunState(Case.ActionType, Case.Intent);
		FMatchPlayCurrentAttackResolutionSession& Session =
			State.CurrentAttack.ResolutionSession;
		const FMatchPlayCurrentAttackInitialRouteMappingResult Mapping =
			FMatchPlayCurrentAttackInitialRouteMappingQuery::Map(
				MakeMappingInput(Case));
		Session.Stage = EMatchPlayCurrentAttackResolutionStage::RouteResolved;
		Session.bHasActualBranch = true;
		Session.ActualBranch = Mapping.ActualBranch;
		if (Case.bHasD6)
		{
			FMatchPlayCurrentAttackResolutionRollRecord Record;
			Record.Purpose =
				EMatchPlayCurrentAttackResolutionRollPurpose::InitialRoute;
			Record.RawD6 = Case.D6;
			Session.InitialRouteRollRecords.Add(Record);
		}
		return State;
	}

	bool ExpectValidationError(
		FAutomationTestBase& Test,
		const TCHAR* Label,
		const FMatchPlayState& State,
		const EMatchPlayCurrentAttackResolutionSessionStateValidationErrorCode
			ExpectedError)
	{
		const auto Result =
			FMatchPlayCurrentAttackResolutionSessionStateValidator::Validate(
				State);
		Test.TestFalse(
			*FString::Printf(TEXT("%s is rejected"), Label),
			Result.bIsCanonical);
		Test.TestEqual(
			*FString::Printf(TEXT("%s exact error"), Label),
			Result.ErrorCode,
			ExpectedError);
		return !Result.bIsCanonical && Result.ErrorCode == ExpectedError;
	}
}

#define INITIAL_ROUTE_STATE_TEST(ClassName, TestName) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
		ClassName, \
		"FMCodex.CoreRules.MatchPlayCurrentAttackInitialRouteState." TestName, \
		EAutomationTestFlags::EditorContext \
			| EAutomationTestFlags::EngineFilter)

INITIAL_ROUTE_STATE_TEST(
	FInitialRouteDefaultAndBeginRegressionTest,
	"01DefaultAndBeginRegression")

bool FInitialRouteDefaultAndBeginRegressionTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayCurrentAttackInitialRouteStateTests;
	const FMatchPlayCurrentAttackActualBranch DefaultBranch;
	const FMatchPlayCurrentAttackResolutionRollRecord DefaultRecord;
	const FMatchPlayCurrentAttackResolutionSession DefaultSession;
	TestEqual(TEXT("LongShot default"),
		DefaultBranch.LongShot, EMatchPlayLongShotActualBranch::None);
	TestEqual(TEXT("CutInside default"), DefaultBranch.CutInsideShot,
		EMatchPlayCutInsideShotActualBranch::None);
	TestEqual(TEXT("Cross default"), DefaultBranch.Cross,
		EMatchPlayCrossActualBranch::None);
	TestEqual(TEXT("PassControl default"), DefaultBranch.PassControl,
		EMatchPlayPassControlActualBranch::None);
	TestEqual(TEXT("ThroughBall default"), DefaultBranch.ThroughBall,
		EMatchPlayThroughBallActualBranch::None);
	TestEqual(TEXT("Roll Purpose default"), DefaultRecord.Purpose,
		EMatchPlayCurrentAttackResolutionRollPurpose::None);
	TestEqual(TEXT("RawD6 default"), DefaultRecord.RawD6, 0);
	TestFalse(TEXT("Session branch absent"), DefaultSession.bHasActualBranch);
	TestTrue(TEXT("Session rolls empty"),
		DefaultSession.InitialRouteRollRecords.IsEmpty());

	const TArray<TPair<ESkillRuleType, EMatchPlayElectiveBranchIntent>> Cases = {
		{ESkillRuleType::LongShot,
			EMatchPlayElectiveBranchIntent::DirectShot},
		{ESkillRuleType::CutInsideShot,
			EMatchPlayElectiveBranchIntent::DirectShot},
		{ESkillRuleType::Cross,
			EMatchPlayElectiveBranchIntent::CrossHigh},
		{ESkillRuleType::PassControl,
			EMatchPlayElectiveBranchIntent::None},
		{ESkillRuleType::ThroughBall,
			EMatchPlayElectiveBranchIntent::None}
	};
	for (const auto& Case : Cases)
	{
		FMatchPlayState Ready = SessionFixtures::MakeReadyState(Case.Key);
		Ready.CurrentAttack.SelectedAction.ElectiveBranchIntent = Case.Value;
		const auto First =
			FMatchPlayCurrentAttackBeginResolutionSessionWriter::Begin(
				Ready,
				SessionFixtures::MakeRequest(Ready));
		TestTrue(TEXT("Begin succeeds"), First.bSuccess);
		TestEqual(TEXT("Begin stage AwaitingRoute"),
			First.AfterState.CurrentAttack.ResolutionSession.Stage,
			EMatchPlayCurrentAttackResolutionStage::AwaitingRoute);
		TestFalse(TEXT("Begin branch absent"),
			First.AfterState.CurrentAttack.ResolutionSession.bHasActualBranch);
		TestTrue(TEXT("Begin branch default"),
			FMatchPlayCurrentAttackActualBranch::StaticStruct()
				->CompareScriptStruct(
					&First.AfterState.CurrentAttack.ResolutionSession.ActualBranch,
					&DefaultBranch,
					0));
		TestTrue(TEXT("Begin rolls empty"), First.AfterState.CurrentAttack
			.ResolutionSession.InitialRouteRollRecords.IsEmpty());
		const auto Duplicate =
			FMatchPlayCurrentAttackBeginResolutionSessionWriter::Begin(
				First.AfterState,
				SessionFixtures::MakeRequest(First.AfterState));
		TestTrue(TEXT("Duplicate succeeds"), Duplicate.bSuccess);
		TestFalse(TEXT("Duplicate creates nothing"),
			Duplicate.bCreatedNewSession);
		TestTrue(TEXT("Duplicate leaves State unchanged"),
			SessionFixtures::AreStatesEqual(
				Duplicate.AfterState,
				First.AfterState));
	}

	return true;
}

INITIAL_ROUTE_STATE_TEST(
	FInitialRouteMappingAndDeterminismTest,
	"02MappingAndDeterminism")

bool FInitialRouteMappingAndDeterminismTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayCurrentAttackInitialRouteStateTests;
	for (const FMappingCase& Case : MakeMappingCases())
	{
		const FMatchPlayCurrentAttackInitialRouteMappingInput Input =
			MakeMappingInput(Case);
		const auto Before = Input;
		const auto First =
			FMatchPlayCurrentAttackInitialRouteMappingQuery::Map(Input);
		const auto Second =
			FMatchPlayCurrentAttackInitialRouteMappingQuery::Map(Input);
		const auto Third =
			FMatchPlayCurrentAttackInitialRouteMappingQuery::Map(Input);
		TestTrue(*FString::Printf(TEXT("%s succeeds"), Case.Label),
			First.bSuccess);
		TestTrue(*FString::Printf(TEXT("%s exact branch"), Case.Label),
			FMatchPlayCurrentAttackActualBranch::StaticStruct()
				->CompareScriptStruct(
					&First.ActualBranch,
					&Case.Expected,
					0));
		TestTrue(*FString::Printf(TEXT("%s deterministic 2"), Case.Label),
			AreResultsEqual(First, Second));
		TestTrue(*FString::Printf(TEXT("%s deterministic 3"), Case.Label),
			AreResultsEqual(First, Third));
		TestTrue(*FString::Printf(TEXT("%s input unchanged"), Case.Label),
			AreInputsEqual(Input, Before));
	}
	return true;
}

INITIAL_ROUTE_STATE_TEST(
	FInitialRouteMapperFailureTest,
	"03MapperFailures")

bool FInitialRouteMapperFailureTest::RunTest(const FString& Parameters)
{
	using namespace MatchPlayCurrentAttackInitialRouteStateTests;
	auto ExpectFailure = [this](
		const TCHAR* Label,
		const FMatchPlayCurrentAttackInitialRouteMappingInput& Input,
		const EMatchPlayCurrentAttackInitialRouteMappingErrorCode Error)
	{
		const auto Result =
			FMatchPlayCurrentAttackInitialRouteMappingQuery::Map(Input);
		TestFalse(Label, Result.bSuccess);
		TestEqual(*FString::Printf(TEXT("%s exact error"), Label),
			Result.ErrorCode, Error);
	};

	FMatchPlayCurrentAttackInitialRouteMappingInput Input;
	ExpectFailure(TEXT("Unsupported Action"), Input,
		EMatchPlayCurrentAttackInitialRouteMappingErrorCode
			::UnsupportedActionType);
	Input.ActionType = ESkillRuleType::LongShot;
	Input.Intent = EMatchPlayElectiveBranchIntent::CrossHigh;
	ExpectFailure(TEXT("LongShot wrong Intent"), Input,
		EMatchPlayCurrentAttackInitialRouteMappingErrorCode::InvalidIntent);
	Input.Intent = EMatchPlayElectiveBranchIntent::DirectShot;
	Input.bHasInitialRouteD6 = true;
	Input.InitialRouteD6 = 1;
	ExpectFailure(TEXT("LongShot unexpected D6"), Input,
		EMatchPlayCurrentAttackInitialRouteMappingErrorCode
			::UnexpectedInitialRouteD6);
	Input.ActionType = ESkillRuleType::Cross;
	Input.Intent = EMatchPlayElectiveBranchIntent::None;
	Input.bHasInitialRouteD6 = false;
	Input.InitialRouteD6 = 0;
	ExpectFailure(TEXT("Cross wrong Intent"), Input,
		EMatchPlayCurrentAttackInitialRouteMappingErrorCode::InvalidIntent);
	Input.Intent = EMatchPlayElectiveBranchIntent::CrossHigh;
	ExpectFailure(TEXT("Cross missing D6"), Input,
		EMatchPlayCurrentAttackInitialRouteMappingErrorCode
			::MissingInitialRouteD6);
	Input.bHasInitialRouteD6 = true;
	ExpectFailure(TEXT("Cross D6 zero"), Input,
		EMatchPlayCurrentAttackInitialRouteMappingErrorCode
			::InvalidInitialRouteD6);
	Input.InitialRouteD6 = 7;
	ExpectFailure(TEXT("Cross D6 seven"), Input,
		EMatchPlayCurrentAttackInitialRouteMappingErrorCode
			::InvalidInitialRouteD6);
	Input.ActionType = ESkillRuleType::PassControl;
	Input.Intent = EMatchPlayElectiveBranchIntent::DirectShot;
	Input.InitialRouteD6 = 1;
	ExpectFailure(TEXT("PassControl wrong Intent"), Input,
		EMatchPlayCurrentAttackInitialRouteMappingErrorCode::InvalidIntent);
	Input.ActionType = ESkillRuleType::ThroughBall;
	ExpectFailure(TEXT("ThroughBall wrong Intent"), Input,
		EMatchPlayCurrentAttackInitialRouteMappingErrorCode::InvalidIntent);
	return true;
}

INITIAL_ROUTE_STATE_TEST(
	FInitialRouteCanonicalMatrixTest,
	"04CanonicalFiveActionMatrix")

bool FInitialRouteCanonicalMatrixTest::RunTest(const FString& Parameters)
{
	using namespace MatchPlayCurrentAttackInitialRouteStateTests;
	for (const FMappingCase& Case : MakeMappingCases())
	{
		const FMatchPlayState State = MakeRouteResolvedState(Case);
		const auto Validation =
			FMatchPlayCurrentAttackResolutionSessionStateValidator::Validate(
				State);
		TestTrue(*FString::Printf(TEXT("%s canonical"), Case.Label),
			Validation.bIsCanonical);
		TestEqual(*FString::Printf(TEXT("%s roll count"), Case.Label),
			State.CurrentAttack.ResolutionSession.InitialRouteRollRecords.Num(),
			Case.bHasD6 ? 1 : 0);
	}
	return true;
}

INITIAL_ROUTE_STATE_TEST(
	FInitialRouteWrapperAndAwaitingCorruptionTest,
	"05WrapperAndAwaitingCorruption")

bool FInitialRouteWrapperAndAwaitingCorruptionTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayCurrentAttackInitialRouteStateTests;
	const FMappingCase LongCase = MakeMappingCases()[0];
	FMatchPlayState State = MakeRouteResolvedState(LongCase);
	State.CurrentAttack.ResolutionSession.bHasActualBranch = false;
	ExpectValidationError(*this, TEXT("Missing branch"), State,
		EMatchPlayCurrentAttackResolutionSessionStateValidationErrorCode
			::MissingActualBranchForRouteResolved);
	State = MakeRouteResolvedState(LongCase);
	State.CurrentAttack.ResolutionSession.ActualBranch = {};
	ExpectValidationError(*this, TEXT("Default branch"), State,
		EMatchPlayCurrentAttackResolutionSessionStateValidationErrorCode
			::DefaultActualBranchForRouteResolved);
	State = MakeRouteResolvedState(LongCase);
	State.CurrentAttack.ResolutionSession.ActualBranch.ActionType =
		ESkillRuleType::Cross;
	ExpectValidationError(*this, TEXT("Action mismatch"), State,
		EMatchPlayCurrentAttackResolutionSessionStateValidationErrorCode
			::ActualBranchActionMismatch);
	State = MakeRouteResolvedState(LongCase);
	State.CurrentAttack.ResolutionSession.ActualBranch.LongShot =
		EMatchPlayLongShotActualBranch::None;
	ExpectValidationError(*this, TEXT("Active None"), State,
		EMatchPlayCurrentAttackResolutionSessionStateValidationErrorCode
			::InvalidActiveBranch);
	State = MakeRouteResolvedState(LongCase);
	State.CurrentAttack.ResolutionSession.ActualBranch.Cross =
		EMatchPlayCrossActualBranch::High;
	ExpectValidationError(*this, TEXT("Inactive nondefault"), State,
		EMatchPlayCurrentAttackResolutionSessionStateValidationErrorCode
			::NonDefaultInactiveBranchPayload);

	State = MakeBegunState(
		ESkillRuleType::LongShot,
		EMatchPlayElectiveBranchIntent::DirectShot);
	State.CurrentAttack.ResolutionSession.bHasActualBranch = true;
	ExpectValidationError(*this, TEXT("Awaiting presence"), State,
		EMatchPlayCurrentAttackResolutionSessionStateValidationErrorCode
			::UnexpectedActualBranchWhileAwaitingRoute);
	State = MakeBegunState(
		ESkillRuleType::LongShot,
		EMatchPlayElectiveBranchIntent::DirectShot);
	State.CurrentAttack.ResolutionSession.ActualBranch =
		MakeLongShotBranch(EMatchPlayLongShotActualBranch::DirectShot);
	ExpectValidationError(*this, TEXT("Awaiting payload"), State,
		EMatchPlayCurrentAttackResolutionSessionStateValidationErrorCode
			::UnexpectedActualBranchWhileAwaitingRoute);
	State = MakeBegunState(
		ESkillRuleType::Cross,
		EMatchPlayElectiveBranchIntent::CrossHigh);
	FMatchPlayCurrentAttackResolutionRollRecord Record;
	Record.Purpose =
		EMatchPlayCurrentAttackResolutionRollPurpose::InitialRoute;
	Record.RawD6 = 1;
	State.CurrentAttack.ResolutionSession.InitialRouteRollRecords.Add(Record);
	ExpectValidationError(*this, TEXT("Awaiting roll"), State,
		EMatchPlayCurrentAttackResolutionSessionStateValidationErrorCode
			::UnexpectedInitialRouteRollWhileAwaitingRoute);
	return true;
}

INITIAL_ROUTE_STATE_TEST(
	FInitialRouteRollAndMappingCorruptionTest,
	"06RollAndMappingCorruption")

bool FInitialRouteRollAndMappingCorruptionTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayCurrentAttackInitialRouteStateTests;
	const TArray<FMappingCase> Cases = MakeMappingCases();
	FMatchPlayCurrentAttackResolutionRollRecord Record;
	Record.Purpose =
		EMatchPlayCurrentAttackResolutionRollPurpose::InitialRoute;
	Record.RawD6 = 1;

	FMatchPlayState State = MakeRouteResolvedState(Cases[0]);
	State.CurrentAttack.ResolutionSession.InitialRouteRollRecords.Add(Record);
	ExpectValidationError(*this, TEXT("LongShot unexpected roll"), State,
		EMatchPlayCurrentAttackResolutionSessionStateValidationErrorCode
			::UnexpectedInitialRouteRollCount);
	State = MakeRouteResolvedState(Cases[4]);
	State.CurrentAttack.ResolutionSession.InitialRouteRollRecords.Empty();
	ExpectValidationError(*this, TEXT("Cross missing roll"), State,
		EMatchPlayCurrentAttackResolutionSessionStateValidationErrorCode
			::UnexpectedInitialRouteRollCount);
	State = MakeRouteResolvedState(Cases[4]);
	State.CurrentAttack.ResolutionSession.InitialRouteRollRecords.Add(Record);
	ExpectValidationError(*this, TEXT("Two initial rolls"), State,
		EMatchPlayCurrentAttackResolutionSessionStateValidationErrorCode
			::UnexpectedInitialRouteRollCount);
	State = MakeRouteResolvedState(Cases[4]);
	State.CurrentAttack.ResolutionSession.InitialRouteRollRecords[0].Purpose =
		EMatchPlayCurrentAttackResolutionRollPurpose::None;
	ExpectValidationError(*this, TEXT("Purpose None"), State,
		EMatchPlayCurrentAttackResolutionSessionStateValidationErrorCode
			::InvalidRollPurpose);
	State = MakeRouteResolvedState(Cases[4]);
	State.CurrentAttack.ResolutionSession.InitialRouteRollRecords[0].RawD6 = 0;
	ExpectValidationError(*this, TEXT("D6 zero"), State,
		EMatchPlayCurrentAttackResolutionSessionStateValidationErrorCode
			::InvalidD6);
	State = MakeRouteResolvedState(Cases[4]);
	State.CurrentAttack.ResolutionSession.InitialRouteRollRecords[0].RawD6 = 7;
	ExpectValidationError(*this, TEXT("D6 seven"), State,
		EMatchPlayCurrentAttackResolutionSessionStateValidationErrorCode
			::InvalidD6);

	State = MakeRouteResolvedState(Cases[0]);
	State.CurrentAttack.ResolutionSession.ActualBranch.LongShot =
		EMatchPlayLongShotActualBranch::DeadCorner;
	ExpectValidationError(*this, TEXT("LongShot mapping mismatch"), State,
		EMatchPlayCurrentAttackResolutionSessionStateValidationErrorCode
			::InitialRouteMappingMismatch);
	State = MakeRouteResolvedState(Cases[2]);
	State.CurrentAttack.ResolutionSession.ActualBranch.CutInsideShot =
		EMatchPlayCutInsideShotActualBranch::DeadCorner;
	ExpectValidationError(*this, TEXT("CIS mapping mismatch"), State,
		EMatchPlayCurrentAttackResolutionSessionStateValidationErrorCode
			::InitialRouteMappingMismatch);
	State = MakeRouteResolvedState(Cases[4]);
	State.CurrentAttack.ResolutionSession.ActualBranch.Cross =
		EMatchPlayCrossActualBranch::Low;
	ExpectValidationError(*this, TEXT("Cross D6 mismatch"), State,
		EMatchPlayCurrentAttackResolutionSessionStateValidationErrorCode
			::InitialRouteMappingMismatch);
	State = MakeRouteResolvedState(Cases[4]);
	SetIntent(State, EMatchPlayElectiveBranchIntent::CrossLow);
	ExpectValidationError(*this, TEXT("Cross Intent mismatch"), State,
		EMatchPlayCurrentAttackResolutionSessionStateValidationErrorCode
			::InitialRouteMappingMismatch);
	State = MakeRouteResolvedState(Cases[6]);
	State.CurrentAttack.ResolutionSession.ActualBranch.PassControl =
		EMatchPlayPassControlActualBranch::RunAdvance;
	ExpectValidationError(*this, TEXT("Pass mapping mismatch"), State,
		EMatchPlayCurrentAttackResolutionSessionStateValidationErrorCode
			::InitialRouteMappingMismatch);
	State = MakeRouteResolvedState(Cases[9]);
	State.CurrentAttack.ResolutionSession.ActualBranch.ThroughBall =
		EMatchPlayThroughBallActualBranch::AntiOffside;
	ExpectValidationError(*this, TEXT("ThroughBall mapping mismatch"), State,
		EMatchPlayCurrentAttackResolutionSessionStateValidationErrorCode
			::InitialRouteMappingMismatch);
	return true;
}

INITIAL_ROUTE_STATE_TEST(
	FInitialRouteProposedSessionTest,
	"07ProposedSessionMatrix")

bool FInitialRouteProposedSessionTest::RunTest(const FString& Parameters)
{
	using namespace MatchPlayCurrentAttackInitialRouteStateTests;
	const TArray<FMappingCase> Cases = MakeMappingCases();
	auto ValidateProposed = [this](
		const TCHAR* Label,
		const FMatchPlayState& Source,
		FMatchPlayCurrentAttackResolutionSession Proposed,
		const bool bExpectedCanonical,
		const EMatchPlayCurrentAttackResolutionSessionStateValidationErrorCode
			ExpectedError)
	{
		FMatchPlayState State = Source;
		State.CurrentAttack.bHasResolutionSession = false;
		State.CurrentAttack.ResolutionSession = {};
		const FMatchPlayState Before = State;
		const auto Result =
			FMatchPlayCurrentAttackResolutionSessionStateValidator::Validate(
				State,
				&Proposed);
		TestEqual(*FString::Printf(TEXT("%s canonical"), Label),
			Result.bIsCanonical, bExpectedCanonical);
		TestEqual(*FString::Printf(TEXT("%s exact error"), Label),
			Result.ErrorCode, ExpectedError);
		TestTrue(*FString::Printf(TEXT("%s State unchanged"), Label),
			SessionFixtures::AreStatesEqual(State, Before));
	};

	FMatchPlayState Awaiting = MakeBegunState(
		ESkillRuleType::LongShot,
		EMatchPlayElectiveBranchIntent::DirectShot);
	ValidateProposed(
		TEXT("AwaitingRoute proposed"),
		Awaiting,
		Awaiting.CurrentAttack.ResolutionSession,
		true,
		EMatchPlayCurrentAttackResolutionSessionStateValidationErrorCode::None);
	FMatchPlayState Route = MakeRouteResolvedState(Cases[4]);
	ValidateProposed(
		TEXT("RouteResolved proposed"),
		Route,
		Route.CurrentAttack.ResolutionSession,
		true,
		EMatchPlayCurrentAttackResolutionSessionStateValidationErrorCode::None);
	FMatchPlayCurrentAttackResolutionSession Proposed =
		Route.CurrentAttack.ResolutionSession;
	Proposed.bHasActualBranch = false;
	ValidateProposed(
		TEXT("Malformed branch proposed"),
		Route,
		Proposed,
		false,
		EMatchPlayCurrentAttackResolutionSessionStateValidationErrorCode
			::MissingActualBranchForRouteResolved);
	Proposed = Route.CurrentAttack.ResolutionSession;
	Proposed.InitialRouteRollRecords[0].RawD6 = 0;
	ValidateProposed(
		TEXT("Malformed roll proposed"),
		Route,
		Proposed,
		false,
		EMatchPlayCurrentAttackResolutionSessionStateValidationErrorCode
			::InvalidD6);
	Proposed = Route.CurrentAttack.ResolutionSession;
	Proposed.ActualBranch.Cross = EMatchPlayCrossActualBranch::Low;
	ValidateProposed(
		TEXT("Mapping mismatch proposed"),
		Route,
		Proposed,
		false,
		EMatchPlayCurrentAttackResolutionSessionStateValidationErrorCode
			::InitialRouteMappingMismatch);
	return true;
}

INITIAL_ROUTE_STATE_TEST(
	FInitialRouteProductionBoundaryTest,
	"08ProductionBoundary")

bool FInitialRouteProductionBoundaryTest::RunTest(const FString& Parameters)
{
	const FString Directory = FPaths::Combine(
		FPaths::ProjectDir(),
		TEXT("Source/FMCodex/CoreRules"));
	auto Load = [&Directory](const TCHAR* FileName)
	{
		FString Source;
		FFileHelper::LoadFileToString(
			Source,
			*FPaths::Combine(Directory, FileName));
		return Source;
	};
	const FString Mapping =
		Load(TEXT("MatchPlayCurrentAttackInitialRouteMappingQuery.cpp"));
	TestFalse(TEXT("Pure mapper has no RNG"),
		Mapping.Contains(TEXT("FMath::Rand"))
			|| Mapping.Contains(TEXT("FRandomStream"))
			|| Mapping.Contains(TEXT("RandomSeed")));
	TestFalse(TEXT("Pure mapper has no Formula or Outcome"),
		Mapping.Contains(TEXT("FormulaResolver"))
			|| Mapping.Contains(TEXT("OutcomeQuery"))
			|| Mapping.Contains(TEXT("Completion")));
	const FString Begin =
		Load(TEXT("MatchPlayCurrentAttackBeginResolutionSessionWriter.cpp"));
	TestFalse(TEXT("Begin does not publish RouteResolved"),
		Begin.Contains(TEXT("RouteResolved")));
	TestFalse(TEXT("Begin does not publish Actual Branch"),
		Begin.Contains(TEXT("bHasActualBranch = true"))
			|| Begin.Contains(TEXT("InitialRouteRollRecords.Add")));
	TestFalse(TEXT("No Route Writer header"),
		FPaths::FileExists(FPaths::Combine(
			Directory,
			TEXT("MatchPlayCurrentAttackResolveInitialRouteWriter.h"))));
	return true;
}

#undef INITIAL_ROUTE_STATE_TEST

#endif
