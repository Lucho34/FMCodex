#include "MatchPlayCurrentAttackResolveInitialRouteOrchestrator.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "MatchPlayCurrentAttackResolveInitialRouteOrchestrationTestFixtures.h"
#include "HAL/FileManager.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "UObject/UnrealType.h"

namespace OrchestrationFixtures =
	FMCodex::Tests::MatchPlayCurrentAttackResolveInitialRouteOrchestration;
namespace RouteFixtures =
	FMCodex::Tests::MatchPlayCurrentAttackResolveInitialRoute;

namespace MatchPlayCurrentAttackResolveInitialRouteOrchestrationTests
{
	FString LoadSource(const TCHAR* FileName)
	{
		FString Source;
		FFileHelper::LoadFileToString(
			Source,
			*FPaths::Combine(
				FPaths::ProjectDir(),
				TEXT("Source/FMCodex/CoreRules"),
				FileName));
		return Source;
	}

	int32 Count(const FString& Source, const TCHAR* Token)
	{
		int32 Result = 0;
		int32 From = 0;
		while ((From = Source.Find(
			Token,
			ESearchCase::CaseSensitive,
			ESearchDir::FromStart,
			From)) != INDEX_NONE)
		{
			++Result;
			From += FCString::Strlen(Token);
		}
		return Result;
	}

	bool ExpectProviderObservation(
		FAutomationTestBase& Test,
		const FString& Context,
		const RouteFixtures::FQueueRollProvider& Provider,
		const int32 ExpectedCalls,
		const int32 ExpectedRemaining)
	{
		bool bValid = true;
		bValid &= Test.TestEqual(
			*(Context + TEXT(" provider calls")),
			Provider.GetCallCount(),
			ExpectedCalls);
		bValid &= Test.TestEqual(
			*(Context + TEXT(" Purpose count")),
			Provider.GetPurposeHistory().Num(),
			ExpectedCalls);
		for (const auto Purpose : Provider.GetPurposeHistory())
		{
			bValid &= Test.TestEqual(
				*(Context + TEXT(" Purpose")),
				Purpose,
				EMatchPlayCurrentAttackResolutionRollPurpose::InitialRoute);
		}
		bValid &= Test.TestEqual(
			*(Context + TEXT(" remaining queue")),
			Provider.GetRemainingCount(),
			ExpectedRemaining);
		return bValid;
	}

	bool ExpectFirstSuccess(
		FAutomationTestBase& Test,
		const FString& Context,
		const FMatchPlayState& State,
		const ESkillRuleType ActionType,
		const EMatchPlayElectiveBranchIntent Intent,
		const int32 RawD6,
		const bool bExpectedNewSession,
		IMatchPlayInitialRouteRollProvider* Provider)
	{
		const FMatchPlayState Before = State;
		const auto Request = OrchestrationFixtures::MakeRequest(State);
		const auto Result =
			FMatchPlayCurrentAttackResolveInitialRouteOrchestrator::Resolve(
				State,
				Request,
				Provider);
		const auto ExpectedBranch = OrchestrationFixtures::MakeExpectedBranch(
			ActionType,
			Intent,
			RawD6);
		const int32 ExpectedRolls =
			OrchestrationFixtures::IsD6Action(ActionType) ? 1 : 0;
		bool bValid = true;
		bValid &= Test.TestTrue(*(Context + TEXT(" succeeds")), Result.bSuccess);
		bValid &= Test.TestEqual(
			*(Context + TEXT(" began-new flag")),
			Result.bBeganNewSession,
			bExpectedNewSession);
		bValid &= Test.TestTrue(
			*(Context + TEXT(" resolved-new flag")),
			Result.bResolvedNewRoute);
		bValid &= Test.TestEqual(
			*(Context + TEXT(" FailureStage None")),
			Result.FailureStage,
			EMatchPlayCurrentAttackInitialRouteOrchestrationFailureStage::None);
		bValid &= Test.TestEqual(
			*(Context + TEXT(" disposition None")),
			Result.FailureDisposition,
			EMatchPlayCurrentAttackResolveInitialRouteFailureDisposition::None);
		bValid &= Test.TestEqual(
			*(Context + TEXT(" ErrorCode None")),
			Result.ErrorCode,
			EMatchPlayCurrentAttackInitialRouteOrchestrationErrorCode::None);
		bValid &= Test.TestTrue(
			*(Context + TEXT(" Begin succeeds")),
			Result.BeginResult.bSuccess);
		bValid &= Test.TestTrue(
			*(Context + TEXT(" Route succeeds")),
			Result.RouteResult.bSuccess);
		bValid &= Test.TestTrue(
			*(Context + TEXT(" Begin input is orchestration input")),
			RouteFixtures::AreStatesEqual(
				Result.BeginResult.BeforeState,
				Before));
		bValid &= Test.TestTrue(
			*(Context + TEXT(" Route input is Begin AfterState")),
			RouteFixtures::AreStatesEqual(
				Result.RouteResult.BeforeState,
				Result.BeginResult.AfterState));
		bValid &= Test.TestTrue(
			*(Context + TEXT(" composite publishes Route AfterState")),
			RouteFixtures::AreStatesEqual(
				Result.AfterState,
				Result.RouteResult.AfterState));
		bValid &= Test.TestTrue(
			*(Context + TEXT(" input State unchanged")),
			RouteFixtures::AreStatesEqual(State, Before));
		bValid &= Test.TestEqual(
			*(Context + TEXT(" RouteResolved")),
			Result.AfterState.CurrentAttack.ResolutionSession.Stage,
			EMatchPlayCurrentAttackResolutionStage::RouteResolved);
		bValid &= Test.TestTrue(
			*(Context + TEXT(" actual branch")),
			RouteFixtures::AreBranchesEqual(
				Result.AfterState.CurrentAttack.ResolutionSession.ActualBranch,
				ExpectedBranch));
		bValid &= Test.TestEqual(
			*(Context + TEXT(" roll count")),
			Result.AfterState.CurrentAttack.ResolutionSession
				.InitialRouteRollRecords.Num(),
			ExpectedRolls);
		bValid &= Test.TestEqual(
			*(Context + TEXT(" provider-called mirror")),
			Result.bProviderCalled,
			ExpectedRolls == 1);
		bValid &= Test.TestTrue(
			*(Context + TEXT(" canonical final State")),
			FMatchPlayCurrentAttackResolutionSessionStateValidator::Validate(
				Result.AfterState).bIsCanonical);
		return bValid;
	}

	bool ExpectDuplicateResult(
		FAutomationTestBase& Test,
		const FString& Context,
		const FMatchPlayState& State,
		const FMatchPlayCurrentAttackResolveInitialRouteOrchestrationResult&
			Result,
		const RouteFixtures::FQueueRollProvider* Provider,
		const int32 ExpectedRemaining)
	{
		bool bValid = true;
		bValid &= Test.TestTrue(*(Context + TEXT(" succeeds")), Result.bSuccess);
		bValid &= Test.TestFalse(
			*(Context + TEXT(" no new Session")),
			Result.bBeganNewSession);
		bValid &= Test.TestFalse(
			*(Context + TEXT(" no new Route")),
			Result.bResolvedNewRoute);
		bValid &= Test.TestFalse(
			*(Context + TEXT(" provider not called")),
			Result.bProviderCalled);
		bValid &= Test.TestTrue(
			*(Context + TEXT(" State unchanged")),
			RouteFixtures::AreStatesEqual(Result.AfterState, State));
		bValid &= Test.TestEqual(
			*(Context + TEXT(" FailureStage None")),
			Result.FailureStage,
			EMatchPlayCurrentAttackInitialRouteOrchestrationFailureStage::None);
		bValid &= Test.TestEqual(
			*(Context + TEXT(" disposition None")),
			Result.FailureDisposition,
			EMatchPlayCurrentAttackResolveInitialRouteFailureDisposition::None);
		bValid &= Test.TestEqual(
			*(Context + TEXT(" ErrorCode None")),
			Result.ErrorCode,
			EMatchPlayCurrentAttackInitialRouteOrchestrationErrorCode::None);
		bValid &= Test.TestTrue(
			*(Context + TEXT(" Begin duplicate succeeds")),
			Result.BeginResult.bSuccess
				&& !Result.BeginResult.bCreatedNewSession);
		bValid &= Test.TestTrue(
			*(Context + TEXT(" Begin duplicate State unchanged")),
			RouteFixtures::AreStatesEqual(
				Result.BeginResult.AfterState,
				State));
		bValid &= Test.TestTrue(
			*(Context + TEXT(" Route duplicate succeeds")),
			Result.RouteResult.bSuccess
				&& !Result.RouteResult.bResolvedNewRoute);
		bValid &= Test.TestTrue(
			*(Context + TEXT(" Route consumes Begin AfterState")),
			RouteFixtures::AreStatesEqual(
				Result.RouteResult.BeforeState,
				Result.BeginResult.AfterState));
		bValid &= Test.TestTrue(
			*(Context + TEXT(" Route duplicate State unchanged")),
			RouteFixtures::AreStatesEqual(
				Result.RouteResult.AfterState,
				State));
		if (Provider != nullptr)
		{
			bValid &= ExpectProviderObservation(
				Test,
				Context,
				*Provider,
				0,
				ExpectedRemaining);
		}
		return bValid;
	}

	bool ExpectDuplicate(
		FAutomationTestBase& Test,
		const FString& Context,
		const FMatchPlayState& State,
		RouteFixtures::FQueueRollProvider* Provider)
	{
		const int32 ExpectedRemaining = Provider != nullptr
			? Provider->GetRemainingCount()
			: 0;
		const auto Result =
			FMatchPlayCurrentAttackResolveInitialRouteOrchestrator::Resolve(
				State,
				OrchestrationFixtures::MakeRequest(State),
				Provider);
		return ExpectDuplicateResult(
			Test,
			Context,
			State,
			Result,
			Provider,
			ExpectedRemaining);
	}

	bool ExpectBeginFailure(
		FAutomationTestBase& Test,
		const FString& Context,
		const FMatchPlayState& State,
		const FMatchPlayCurrentAttackResolveInitialRouteOrchestrationRequest&
			Request,
		RouteFixtures::FQueueRollProvider* Provider,
		const EMatchPlayCurrentAttackBeginResolutionSessionErrorCode
			ExpectedBeginError)
	{
		const auto Result =
			FMatchPlayCurrentAttackResolveInitialRouteOrchestrator::Resolve(
				State,
				Request,
				Provider);
		bool bValid = true;
		bValid &= Test.TestFalse(*(Context + TEXT(" fails")), Result.bSuccess);
		bValid &= Test.TestFalse(
			*(Context + TEXT(" no new Session")),
			Result.bBeganNewSession);
		bValid &= Test.TestFalse(
			*(Context + TEXT(" no new Route")),
			Result.bResolvedNewRoute);
		bValid &= Test.TestEqual(
			*(Context + TEXT(" Begin failure stage")),
			Result.FailureStage,
			EMatchPlayCurrentAttackInitialRouteOrchestrationFailureStage
				::BeginResolutionSession);
		bValid &= Test.TestEqual(
			*(Context + TEXT(" Begin top-level error")),
			Result.ErrorCode,
			EMatchPlayCurrentAttackInitialRouteOrchestrationErrorCode
				::BeginResolutionSessionFailed);
		bValid &= Test.TestEqual(
			*(Context + TEXT(" disposition None")),
			Result.FailureDisposition,
			EMatchPlayCurrentAttackResolveInitialRouteFailureDisposition::None);
		bValid &= Test.TestFalse(
			*(Context + TEXT(" provider not marked called")),
			Result.bProviderCalled);
		bValid &= Test.TestTrue(
			*(Context + TEXT(" AfterState unchanged")),
			RouteFixtures::AreStatesEqual(Result.AfterState, State));
		bValid &= Test.TestFalse(
			*(Context + TEXT(" Begin nested failure")),
			Result.BeginResult.bSuccess);
		bValid &= Test.TestEqual(
			*(Context + TEXT(" nested Begin error")),
			Result.BeginResult.ErrorCode,
			ExpectedBeginError);
		const FMatchPlayCurrentAttackResolveInitialRouteWriterResult
			DefaultRouteResult;
		bValid &= Test.TestTrue(
			*(Context + TEXT(" Route Result remains fully default")),
			RouteFixtures::AreWriterResultsEqual(
				Result.RouteResult,
				DefaultRouteResult));
		if (Provider != nullptr)
		{
			bValid &= Test.TestEqual(
				*(Context + TEXT(" provider calls")),
				Provider->GetCallCount(),
				0);
		}
		return bValid;
	}

	bool ExpectRouteFailureResult(
		FAutomationTestBase& Test,
		const FString& Context,
		const FMatchPlayState& State,
		const FMatchPlayCurrentAttackResolveInitialRouteOrchestrationResult&
			Result,
		const EMatchPlayCurrentAttackResolveInitialRouteWriterErrorCode
			ExpectedError,
		const EMatchPlayCurrentAttackResolveInitialRouteFailureDisposition
			ExpectedDisposition,
		const bool bExpectedProviderCalled)
	{
		bool bValid = true;
		bValid &= Test.TestFalse(*(Context + TEXT(" fails")), Result.bSuccess);
		bValid &= Test.TestTrue(
			*(Context + TEXT(" Begin succeeds")),
			Result.BeginResult.bSuccess);
		bValid &= Test.TestFalse(
			*(Context + TEXT(" Route fails")),
			Result.RouteResult.bSuccess);
		bValid &= Test.TestFalse(
			*(Context + TEXT(" no new Route")),
			Result.bResolvedNewRoute);
		bValid &= Test.TestEqual(
			*(Context + TEXT(" Route nested error")),
			Result.RouteResult.ErrorCode,
			ExpectedError);
		bValid &= Test.TestEqual(
			*(Context + TEXT(" failure stage")),
			Result.FailureStage,
			EMatchPlayCurrentAttackInitialRouteOrchestrationFailureStage
				::ResolveInitialRoute);
		bValid &= Test.TestEqual(
			*(Context + TEXT(" top-level error")),
			Result.ErrorCode,
			EMatchPlayCurrentAttackInitialRouteOrchestrationErrorCode
				::InitialRouteResolutionFailed);
		bValid &= Test.TestEqual(
			*(Context + TEXT(" disposition mirror")),
			Result.FailureDisposition,
			ExpectedDisposition);
		bValid &= Test.TestEqual(
			*(Context + TEXT(" nested disposition")),
			Result.RouteResult.FailureDisposition,
			ExpectedDisposition);
		bValid &= Test.TestEqual(
			*(Context + TEXT(" provider-called mirror")),
			Result.bProviderCalled,
			bExpectedProviderCalled);
		bValid &= Test.TestEqual(
			*(Context + TEXT(" nested provider-called")),
			Result.RouteResult.bProviderCalled,
			bExpectedProviderCalled);
		bValid &= Test.TestTrue(
			*(Context + TEXT(" publishes Begin AfterState")),
			RouteFixtures::AreStatesEqual(
				Result.AfterState,
				Result.BeginResult.AfterState));
		bValid &= Test.TestEqual(
			*(Context + TEXT(" AwaitingRoute preserved")),
			Result.AfterState.CurrentAttack.ResolutionSession.Stage,
			EMatchPlayCurrentAttackResolutionStage::AwaitingRoute);
		bValid &= Test.TestFalse(
			*(Context + TEXT(" no ActualBranch")),
			Result.AfterState.CurrentAttack.ResolutionSession
				.bHasActualBranch);
		bValid &= Test.TestTrue(
			*(Context + TEXT(" no roll record")),
			Result.AfterState.CurrentAttack.ResolutionSession
				.InitialRouteRollRecords.IsEmpty());
		bValid &= Test.TestTrue(
			*(Context + TEXT(" AwaitingRoute canonical")),
			FMatchPlayCurrentAttackResolutionSessionStateValidator::Validate(
				Result.AfterState).bIsCanonical);
		return bValid;
	}

	bool ExpectRouteFailure(
		FAutomationTestBase& Test,
		const FString& Context,
		const FMatchPlayState& State,
		IMatchPlayInitialRouteRollProvider* Provider,
		const EMatchPlayCurrentAttackResolveInitialRouteWriterErrorCode
			ExpectedError,
		const EMatchPlayCurrentAttackResolveInitialRouteFailureDisposition
			ExpectedDisposition,
		const bool bExpectedProviderCalled)
	{
		const auto Result =
			FMatchPlayCurrentAttackResolveInitialRouteOrchestrator::Resolve(
				State,
				OrchestrationFixtures::MakeRequest(State),
				Provider);
		return ExpectRouteFailureResult(
			Test,
			Context,
			State,
			Result,
			ExpectedError,
			ExpectedDisposition,
			bExpectedProviderCalled);
	}
}

#define INITIAL_ROUTE_ORCHESTRATION_TEST(ClassName, TestName) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
		ClassName, \
		"FMCodex.CoreRules.MatchPlayCurrentAttackResolveInitialRouteOrchestration." TestName, \
		EAutomationTestFlags::EditorContext \
			| EAutomationTestFlags::EngineFilter)

INITIAL_ROUTE_ORCHESTRATION_TEST(
	FInitialRouteOrchestrationSurfaceTest,
	"01.RequestAndProductionSurface")

bool FInitialRouteOrchestrationSurfaceTest::RunTest(const FString& Parameters)
{
	using namespace MatchPlayCurrentAttackResolveInitialRouteOrchestrationTests;
	int32 PropertyCount = 0;
	FName OnlyProperty = NAME_None;
	for (TFieldIterator<FProperty> It(
		FMatchPlayCurrentAttackResolveInitialRouteOrchestrationRequest
			::StaticStruct());
		It;
		++It)
	{
		++PropertyCount;
		OnlyProperty = It->GetFName();
	}
	TestEqual(TEXT("Request has one field"), PropertyCount, 1);
	TestEqual(TEXT("Request field is AttackSequence"),
		OnlyProperty, FName(TEXT("AttackSequence")));

	const FString Types = LoadSource(
		TEXT("MatchPlayCurrentAttackResolveInitialRouteOrchestrationTypes.h"));
	const FString Header = LoadSource(
		TEXT("MatchPlayCurrentAttackResolveInitialRouteOrchestrator.h"));
	const FString Source = LoadSource(
		TEXT("MatchPlayCurrentAttackResolveInitialRouteOrchestrator.cpp"));
	TestFalse(TEXT("Request is not BlueprintType"),
		Types.Contains(TEXT("USTRUCT(BlueprintType)")));
	TestFalse(TEXT("No Blueprint API"),
		Header.Contains(TEXT("UFUNCTION"))
			|| Header.Contains(TEXT("Blueprint")));
	TestEqual(TEXT("One Begin Writer call"),
		Count(Source,
			TEXT("FMatchPlayCurrentAttackBeginResolutionSessionWriter::Begin")),
		1);
	TestEqual(TEXT("One Route Writer call"),
		Count(Source,
			TEXT("FMatchPlayCurrentAttackResolveInitialRouteWriter::Resolve")),
		1);
	TestFalse(TEXT("No direct provider call"),
		Source.Contains(TEXT("RollD6(")));
	TestFalse(TEXT("No mapper call"),
		Source.Contains(TEXT("InitialRouteMappingQuery::Map")));
	TestFalse(TEXT("No direct Session authority"),
		Source.Contains(TEXT("ResolutionSession ="))
			|| Source.Contains(TEXT("bHasActualBranch ="))
			|| Source.Contains(TEXT("ActualBranch ="))
			|| Source.Contains(TEXT("InitialRouteRollRecords.Add"))
			|| Source.Contains(TEXT("Stage = EMatchPlayCurrentAttackResolutionStage::RouteResolved"))
			|| Source.Contains(TEXT("SelectionStage =")));
	TestFalse(TEXT("No retry loop"),
		Source.Contains(TEXT("while ("))
			|| Source.Contains(TEXT("for ("))
			|| Source.Contains(TEXT("Resolve(BeforeState")));
	TestFalse(TEXT("No concrete RNG"),
		Source.Contains(TEXT("FMath::Rand"))
			|| Source.Contains(TEXT("FRandomStream"))
			|| Source.Contains(TEXT("RandomSeed")));
	return true;
}

INITIAL_ROUTE_ORCHESTRATION_TEST(
	FInitialRouteOrchestrationDeterministicActionsTest,
	"02.FirstResolutionDeterministicActions")

bool FInitialRouteOrchestrationDeterministicActionsTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayCurrentAttackResolveInitialRouteOrchestrationTests;
	bool bValid = true;
	for (const ESkillRuleType ActionType : {
		ESkillRuleType::LongShot,
		ESkillRuleType::CutInsideShot})
	{
		for (const EMatchPlayElectiveBranchIntent Intent : {
			EMatchPlayElectiveBranchIntent::DirectShot,
			EMatchPlayElectiveBranchIntent::DeadCorner})
		{
			const FMatchPlayState State =
				OrchestrationFixtures::MakeReadyState(ActionType, Intent);
			bValid &= ExpectFirstSuccess(
				*this,
				FString::Printf(TEXT("Action %d Intent %d"),
					static_cast<int32>(ActionType),
					static_cast<int32>(Intent)),
				State,
				ActionType,
				Intent,
				0,
				true,
				nullptr);
		}
	}
	return bValid;
}

INITIAL_ROUTE_ORCHESTRATION_TEST(
	FInitialRouteOrchestrationD6ActionsTest,
	"03.FirstResolutionD6Actions")

bool FInitialRouteOrchestrationD6ActionsTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayCurrentAttackResolveInitialRouteOrchestrationTests;
	bool bValid = true;
	for (const EMatchPlayElectiveBranchIntent Intent : {
		EMatchPlayElectiveBranchIntent::CrossHigh,
		EMatchPlayElectiveBranchIntent::CrossLow})
	{
		for (int32 D6 = 1; D6 <= 6; ++D6)
		{
			const FMatchPlayState State = OrchestrationFixtures::MakeReadyState(
				ESkillRuleType::Cross,
				Intent);
			RouteFixtures::FQueueRollProvider Provider;
			Provider.Enqueue(RouteFixtures::MakeSuccess(D6));
			const FString Context = FString::Printf(
				TEXT("Cross Intent %d D6 %d"),
				static_cast<int32>(Intent),
				D6);
			bValid &= ExpectFirstSuccess(
				*this, Context, State, ESkillRuleType::Cross,
				Intent, D6, true, &Provider);
			bValid &= ExpectProviderObservation(
				*this, Context, Provider, 1, 0);
		}
	}
	for (const ESkillRuleType ActionType : {
		ESkillRuleType::PassControl,
		ESkillRuleType::ThroughBall})
	{
		for (int32 D6 = 1; D6 <= 6; ++D6)
		{
			const FMatchPlayState State =
				OrchestrationFixtures::MakeReadyState(ActionType);
			RouteFixtures::FQueueRollProvider Provider;
			Provider.Enqueue(RouteFixtures::MakeSuccess(D6));
			const FString Context = FString::Printf(
				TEXT("Action %d D6 %d"),
				static_cast<int32>(ActionType),
				D6);
			bValid &= ExpectFirstSuccess(
				*this, Context, State, ActionType,
				EMatchPlayElectiveBranchIntent::None,
				D6, true, &Provider);
			bValid &= ExpectProviderObservation(
				*this, Context, Provider, 1, 0);
		}
	}
	return bValid;
}

INITIAL_ROUTE_ORCHESTRATION_TEST(
	FInitialRouteOrchestrationBeginFailureTest,
	"04.BeginFailurePriority")

bool FInitialRouteOrchestrationBeginFailureTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayCurrentAttackResolveInitialRouteOrchestrationTests;
	bool bValid = true;
	const FMatchPlayState Ready = OrchestrationFixtures::MakeReadyState(
		ESkillRuleType::Cross,
		EMatchPlayElectiveBranchIntent::CrossHigh);
	RouteFixtures::FQueueRollProvider Provider;
	Provider.Enqueue(RouteFixtures::MakeSuccess(4));

	auto InvalidRequest = OrchestrationFixtures::MakeRequest(Ready);
	InvalidRequest.AttackSequence = 0;
	bValid &= ExpectBeginFailure(
		*this, TEXT("Invalid request sequence precedes provider"),
		Ready, InvalidRequest, &Provider,
		EMatchPlayCurrentAttackBeginResolutionSessionErrorCode
			::InvalidRequestedAttackSequence);

	auto StaleRequest = OrchestrationFixtures::MakeRequest(Ready);
	--StaleRequest.AttackSequence;
	bValid &= ExpectBeginFailure(
		*this, TEXT("Stale precedes missing provider"),
		Ready, StaleRequest, &Provider,
		EMatchPlayCurrentAttackBeginResolutionSessionErrorCode
			::AttackSequenceMismatch);

	auto AheadRequest = OrchestrationFixtures::MakeRequest(Ready);
	++AheadRequest.AttackSequence;
	bValid &= ExpectBeginFailure(
		*this, TEXT("Ahead sequence precedes provider"),
		Ready, AheadRequest, &Provider,
		EMatchPlayCurrentAttackBeginResolutionSessionErrorCode
			::AttackSequenceMismatch);

	FMatchPlayState InvalidAuthoritative = Ready;
	InvalidAuthoritative.CurrentAttack.AttackSequence = 0;
	bValid &= ExpectBeginFailure(
		*this, TEXT("Invalid authoritative sequence precedes provider"),
		InvalidAuthoritative,
		OrchestrationFixtures::MakeRequest(Ready),
		&Provider,
		EMatchPlayCurrentAttackBeginResolutionSessionErrorCode
			::InvalidCurrentAttackSequence);

	FMatchPlayState WrongPhase = Ready;
	WrongPhase.CurrentAttack.Phase =
		EMatchPlayCurrentAttackPhase::Deployment;
	bValid &= ExpectBeginFailure(
		*this, TEXT("Wrong phase precedes provider"),
		WrongPhase,
		OrchestrationFixtures::MakeRequest(WrongPhase),
		&Provider,
		EMatchPlayCurrentAttackBeginResolutionSessionErrorCode
			::CurrentAttackNotInResolution);

	FMatchPlayState MissingAttack = Ready;
	MissingAttack.bHasCurrentAttack = false;
	MissingAttack.CurrentAttack = FMatchPlayCurrentAttackState();
	const auto MissingAttackRequest =
		OrchestrationFixtures::MakeRequest(Ready);
	bValid &= TestTrue(
		TEXT("Missing CurrentAttack Request sequence is positive"),
		MissingAttackRequest.AttackSequence > 0);
	bValid &= ExpectBeginFailure(
		*this, TEXT("Missing CurrentAttack precedes provider"),
		MissingAttack,
		MissingAttackRequest,
		&Provider,
		EMatchPlayCurrentAttackBeginResolutionSessionErrorCode
			::NoCurrentAttack);

	FMatchPlayState Malformed = OrchestrationFixtures::MakeAwaitingState(
		ESkillRuleType::Cross,
		EMatchPlayElectiveBranchIntent::CrossHigh);
	Malformed.CurrentAttack.ResolutionSession.bHasActualBranch = true;
	bValid &= ExpectBeginFailure(
		*this, TEXT("Malformed Session precedes provider"),
		Malformed,
		OrchestrationFixtures::MakeRequest(Malformed),
		&Provider,
		EMatchPlayCurrentAttackBeginResolutionSessionErrorCode
			::InvalidExistingSessionState);
	bValid &= TestEqual(TEXT("Configured provider queue untouched"),
		Provider.GetRemainingCount(), 1);

	const FMatchPlayState AwaitingPassControl =
		OrchestrationFixtures::MakeAwaitingState(
			ESkillRuleType::PassControl);
	FMatchPlayInitialRouteRollProviderResult MalformedProviderResult =
		RouteFixtures::MakeSuccess(4);
	MalformedProviderResult.ErrorCode =
		EMatchPlayInitialRouteRollProviderErrorCode::ProviderFailure;
	MalformedProviderResult.ErrorMessage =
		TEXT("Malformed success shape must be rejected.");
	RouteFixtures::FQueueRollProvider MalformedProvider;
	MalformedProvider.Enqueue(MalformedProviderResult);
	const auto RouteFailure =
		FMatchPlayCurrentAttackResolveInitialRouteOrchestrator::Resolve(
			AwaitingPassControl,
			OrchestrationFixtures::MakeRequest(AwaitingPassControl),
			&MalformedProvider);
	bValid &= ExpectRouteFailureResult(
		*this,
		TEXT("Duplicate AwaitingRoute plus malformed provider"),
		AwaitingPassControl,
		RouteFailure,
		EMatchPlayCurrentAttackResolveInitialRouteWriterErrorCode
			::InvalidRngResult,
		EMatchPlayCurrentAttackResolveInitialRouteFailureDisposition
			::NonRetryableExecutionFailure,
		true);
	bValid &= TestTrue(
		TEXT("AwaitingRoute Begin is duplicate success"),
		RouteFailure.BeginResult.bSuccess
			&& !RouteFailure.BeginResult.bCreatedNewSession);
	bValid &= TestFalse(
		TEXT("AwaitingRoute composite did not begin a new Session"),
		RouteFailure.bBeganNewSession);
	bValid &= TestTrue(
		TEXT("AwaitingRoute Begin leaves State unchanged"),
		RouteFixtures::AreStatesEqual(
			RouteFailure.BeginResult.AfterState,
			AwaitingPassControl));
	bValid &= TestTrue(
		TEXT("Malformed ProviderResult retained"),
		RouteFixtures::AreProviderResultsEqual(
			RouteFailure.RouteResult.ProviderResult,
			MalformedProviderResult));
	bValid &= TestTrue(
		TEXT("Malformed provider composite leaves State unchanged"),
		RouteFixtures::AreStatesEqual(
			RouteFailure.AfterState,
			AwaitingPassControl));
	bValid &= ExpectProviderObservation(
		*this,
		TEXT("Duplicate AwaitingRoute plus malformed provider"),
		MalformedProvider,
		1,
		0);
	return bValid;
}

INITIAL_ROUTE_ORCHESTRATION_TEST(
	FInitialRouteOrchestrationMissingProviderRecoveryTest,
	"05.RetryableMissingProviderRecovery")

bool FInitialRouteOrchestrationMissingProviderRecoveryTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayCurrentAttackResolveInitialRouteOrchestrationTests;
	const FMatchPlayState Ready = OrchestrationFixtures::MakeReadyState(
		ESkillRuleType::Cross,
		EMatchPlayElectiveBranchIntent::CrossHigh);
	const auto Request = OrchestrationFixtures::MakeRequest(Ready);
	const auto First =
		FMatchPlayCurrentAttackResolveInitialRouteOrchestrator::Resolve(
			Ready, Request, nullptr);
	bool bValid = ExpectRouteFailureResult(
		*this, TEXT("Missing provider first call"), Ready, First,
		EMatchPlayCurrentAttackResolveInitialRouteWriterErrorCode
			::RngProviderUnavailable,
		EMatchPlayCurrentAttackResolveInitialRouteFailureDisposition
			::RetryableExecutionFailure,
		false);
	bValid &= TestTrue(TEXT("First call began new Session"),
		First.bBeganNewSession);
	bValid &= TestTrue(TEXT("First call publishes its Begin State"),
		RouteFixtures::AreStatesEqual(
			First.AfterState, First.BeginResult.AfterState));

	RouteFixtures::FQueueRollProvider RecoveryProvider;
	RecoveryProvider.Enqueue(RouteFixtures::MakeSuccess(4));
	const auto Recovered =
		FMatchPlayCurrentAttackResolveInitialRouteOrchestrator::Resolve(
			First.AfterState,
			OrchestrationFixtures::MakeRequest(First.AfterState),
			&RecoveryProvider);
	bValid &= TestTrue(TEXT("Explicit recovery succeeds"), Recovered.bSuccess);
	bValid &= TestFalse(TEXT("Recovery Begin is duplicate"),
		Recovered.bBeganNewSession);
	bValid &= TestTrue(TEXT("Recovery resolves new Route"),
		Recovered.bResolvedNewRoute);
	bValid &= ExpectProviderObservation(
		*this, TEXT("Missing provider recovery"), RecoveryProvider, 1, 0);

	RouteFixtures::FQueueRollProvider DuplicateProvider;
	DuplicateProvider.Enqueue(RouteFixtures::MakeFailure(
		TEXT("Duplicate must not consume provider.")));
	bValid &= ExpectDuplicate(
		*this, TEXT("Recovered duplicate"),
		Recovered.AfterState, &DuplicateProvider);
	bValid &= TestEqual(TEXT("Duplicate queue untouched"),
		DuplicateProvider.GetRemainingCount(), 1);
	return bValid;
}

INITIAL_ROUTE_ORCHESTRATION_TEST(
	FInitialRouteOrchestrationProviderFailureRecoveryTest,
	"06.RetryableProviderFailureRecovery")

bool FInitialRouteOrchestrationProviderFailureRecoveryTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayCurrentAttackResolveInitialRouteOrchestrationTests;
	const FMatchPlayState Ready = OrchestrationFixtures::MakeReadyState(
		ESkillRuleType::ThroughBall);
	RouteFixtures::FQueueRollProvider FirstProvider;
	FirstProvider.Enqueue(RouteFixtures::MakeFailure());
	const auto First =
		FMatchPlayCurrentAttackResolveInitialRouteOrchestrator::Resolve(
			Ready,
			OrchestrationFixtures::MakeRequest(Ready),
			&FirstProvider);
	bool bValid = ExpectProviderObservation(
		*this, TEXT("Provider failure first call"), FirstProvider, 1, 0);
	bValid &= TestFalse(TEXT("Provider failure composite fails"),
		First.bSuccess);
	bValid &= TestTrue(TEXT("Provider failure began new Session"),
		First.bBeganNewSession);
	bValid &= TestEqual(TEXT("Provider failure nested error"),
		First.RouteResult.ErrorCode,
		EMatchPlayCurrentAttackResolveInitialRouteWriterErrorCode
			::RngProviderFailed);
	bValid &= TestEqual(TEXT("Provider failure retryable"),
		First.FailureDisposition,
		EMatchPlayCurrentAttackResolveInitialRouteFailureDisposition
			::RetryableExecutionFailure);
	bValid &= TestTrue(TEXT("Provider failure publishes Begin State"),
		RouteFixtures::AreStatesEqual(
			First.AfterState, First.BeginResult.AfterState));

	RouteFixtures::FQueueRollProvider RecoveryProvider;
	RecoveryProvider.Enqueue(RouteFixtures::MakeSuccess(6));
	const auto Recovered =
		FMatchPlayCurrentAttackResolveInitialRouteOrchestrator::Resolve(
			First.AfterState,
			OrchestrationFixtures::MakeRequest(First.AfterState),
			&RecoveryProvider);
	bValid &= TestTrue(TEXT("Provider recovery succeeds"), Recovered.bSuccess);
	bValid &= TestFalse(TEXT("Provider recovery Begin duplicate"),
		Recovered.bBeganNewSession);
	bValid &= TestTrue(TEXT("Provider recovery resolves Route"),
		Recovered.bResolvedNewRoute);
	bValid &= ExpectProviderObservation(
		*this, TEXT("Provider recovery"), RecoveryProvider, 1, 0);
	RouteFixtures::FQueueRollProvider DuplicateProvider;
	DuplicateProvider.Enqueue(RouteFixtures::MakeFailure(
		TEXT("Recovered duplicate must not consume provider.")));
	bValid &= ExpectDuplicate(
		*this,
		TEXT("Provider recovery duplicate"),
		Recovered.AfterState,
		&DuplicateProvider);
	return bValid;
}

INITIAL_ROUTE_ORCHESTRATION_TEST(
	FInitialRouteOrchestrationNonRetryableTest,
	"07.NonRetryableProviderOutput")

bool FInitialRouteOrchestrationNonRetryableTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayCurrentAttackResolveInitialRouteOrchestrationTests;
	struct FCase
	{
		const TCHAR* Label;
		FMatchPlayInitialRouteRollProviderResult ProviderResult;
	};
	FMatchPlayInitialRouteRollProviderResult MalformedSuccess =
		RouteFixtures::MakeSuccess(4);
	MalformedSuccess.ErrorCode =
		EMatchPlayInitialRouteRollProviderErrorCode::ProviderFailure;
	FMatchPlayInitialRouteRollProviderResult MalformedFailure;
	const FCase Cases[] = {
		{TEXT("RawD6 zero"), RouteFixtures::MakeSuccess(0)},
		{TEXT("RawD6 seven"), RouteFixtures::MakeSuccess(7)},
		{TEXT("Malformed success"), MalformedSuccess},
		{TEXT("Malformed failure"), MalformedFailure}
	};
	bool bValid = true;
	for (const FCase& Case : Cases)
	{
		const FMatchPlayState Ready = OrchestrationFixtures::MakeReadyState(
			ESkillRuleType::PassControl);
		RouteFixtures::FQueueRollProvider Provider;
		Provider.Enqueue(Case.ProviderResult);
		const auto Result =
			FMatchPlayCurrentAttackResolveInitialRouteOrchestrator::Resolve(
				Ready,
				OrchestrationFixtures::MakeRequest(Ready),
				&Provider);
		const FString Context(Case.Label);
		bValid &= TestFalse(*(Context + TEXT(" fails")), Result.bSuccess);
		bValid &= TestTrue(*(Context + TEXT(" Begin succeeds")),
			Result.BeginResult.bSuccess);
		bValid &= TestTrue(*(Context + TEXT(" began Session")),
			Result.bBeganNewSession);
		bValid &= TestEqual(*(Context + TEXT(" nested error")),
			Result.RouteResult.ErrorCode,
			EMatchPlayCurrentAttackResolveInitialRouteWriterErrorCode
				::InvalidRngResult);
		bValid &= TestEqual(*(Context + TEXT(" nonretryable")),
			Result.FailureDisposition,
			EMatchPlayCurrentAttackResolveInitialRouteFailureDisposition
				::NonRetryableExecutionFailure);
		bValid &= TestTrue(*(Context + TEXT(" provider called")),
			Result.bProviderCalled);
		bValid &= TestTrue(*(Context + TEXT(" ProviderResult retained")),
			RouteFixtures::AreProviderResultsEqual(
				Result.RouteResult.ProviderResult,
				Case.ProviderResult));
		bValid &= TestTrue(*(Context + TEXT(" publishes Begin State")),
			RouteFixtures::AreStatesEqual(
				Result.AfterState,
				Result.BeginResult.AfterState));
		bValid &= TestFalse(*(Context + TEXT(" no partial branch")),
			Result.AfterState.CurrentAttack.ResolutionSession
				.bHasActualBranch);
		bValid &= ExpectProviderObservation(
			*this, Context, Provider, 1, 0);
	}
	return bValid;
}

INITIAL_ROUTE_ORCHESTRATION_TEST(
	FInitialRouteOrchestrationDuplicateTest,
	"08.CanonicalDuplicateMatrix")

bool FInitialRouteOrchestrationDuplicateTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayCurrentAttackResolveInitialRouteOrchestrationTests;
	bool bValid = true;

	const FMatchPlayState ReadyShot = OrchestrationFixtures::MakeReadyState(
		ESkillRuleType::LongShot,
		EMatchPlayElectiveBranchIntent::DirectShot);
	const auto ShotFirst =
		FMatchPlayCurrentAttackResolveInitialRouteOrchestrator::Resolve(
			ReadyShot, OrchestrationFixtures::MakeRequest(ReadyShot), nullptr);
	bValid &= TestTrue(TEXT("No Session deterministic first succeeds"),
		ShotFirst.bSuccess && ShotFirst.bBeganNewSession);
	RouteFixtures::FQueueRollProvider ShotDuplicateProvider;
	ShotDuplicateProvider.Enqueue(RouteFixtures::MakeSuccess(6));
	bValid &= ExpectDuplicate(*this, TEXT("No Session deterministic duplicate"),
		ShotFirst.AfterState, &ShotDuplicateProvider);

	const FMatchPlayState ReadyD6 = OrchestrationFixtures::MakeReadyState(
		ESkillRuleType::Cross,
		EMatchPlayElectiveBranchIntent::CrossLow);
	RouteFixtures::FQueueRollProvider D6FirstProvider;
	D6FirstProvider.Enqueue(RouteFixtures::MakeSuccess(5));
	const auto D6First =
		FMatchPlayCurrentAttackResolveInitialRouteOrchestrator::Resolve(
			ReadyD6, OrchestrationFixtures::MakeRequest(ReadyD6),
			&D6FirstProvider);
	bValid &= TestTrue(TEXT("No Session D6 first succeeds"),
		D6First.bSuccess && D6First.bBeganNewSession);
	RouteFixtures::FQueueRollProvider D6DuplicateProvider;
	D6DuplicateProvider.Enqueue(RouteFixtures::MakeFailure());
	bValid &= ExpectDuplicate(*this, TEXT("No Session D6 duplicate"),
		D6First.AfterState, &D6DuplicateProvider);

	const FMatchPlayState ReadyPassControl =
		OrchestrationFixtures::MakeReadyState(
			ESkillRuleType::PassControl);
	RouteFixtures::FQueueRollProvider PassControlSeedProvider;
	PassControlSeedProvider.Enqueue(RouteFixtures::MakeSuccess(1));
	const auto PassControlResolved =
		FMatchPlayCurrentAttackResolveInitialRouteOrchestrator::Resolve(
			ReadyPassControl,
			OrchestrationFixtures::MakeRequest(ReadyPassControl),
			&PassControlSeedProvider);
	bValid &= TestTrue(
		TEXT("PassControl seed resolution succeeds"),
		PassControlResolved.bSuccess
			&& PassControlResolved.bBeganNewSession
			&& PassControlResolved.bResolvedNewRoute);
	bValid &= ExpectProviderObservation(
		*this,
		TEXT("PassControl seed resolution"),
		PassControlSeedProvider,
		1,
		0);

	RouteFixtures::FQueueRollProvider PassControlFirstDuplicateProvider;
	PassControlFirstDuplicateProvider.Enqueue(
		RouteFixtures::MakeSuccess(6));
	const auto PassControlFirstDuplicate =
		FMatchPlayCurrentAttackResolveInitialRouteOrchestrator::Resolve(
			PassControlResolved.AfterState,
			OrchestrationFixtures::MakeRequest(
				PassControlResolved.AfterState),
			&PassControlFirstDuplicateProvider);
	bValid &= ExpectDuplicateResult(
		*this,
		TEXT("PassControl first canonical duplicate"),
		PassControlResolved.AfterState,
		PassControlFirstDuplicate,
		&PassControlFirstDuplicateProvider,
		1);
	const FMatchPlayCurrentAttackResolutionSession& PassControlSession =
		PassControlFirstDuplicate.AfterState.CurrentAttack.ResolutionSession;
	bValid &= TestEqual(
		TEXT("PassControl duplicate returns PassAdvance"),
		PassControlSession.ActualBranch.PassControl,
		EMatchPlayPassControlActualBranch::PassAdvance);
	bValid &= TestEqual(
		TEXT("PassControl duplicate retains one roll"),
		PassControlSession.InitialRouteRollRecords.Num(),
		1);
	if (PassControlSession.InitialRouteRollRecords.Num() == 1)
	{
		bValid &= TestEqual(
			TEXT("PassControl duplicate roll Purpose"),
			PassControlSession.InitialRouteRollRecords[0].Purpose,
			EMatchPlayCurrentAttackResolutionRollPurpose::InitialRoute);
		bValid &= TestEqual(
			TEXT("PassControl duplicate roll RawD6"),
			PassControlSession.InitialRouteRollRecords[0].RawD6,
			1);
	}

	RouteFixtures::FQueueRollProvider PassControlSecondDuplicateProvider;
	PassControlSecondDuplicateProvider.Enqueue(
		RouteFixtures::MakeSuccess(3));
	const auto PassControlSecondDuplicate =
		FMatchPlayCurrentAttackResolveInitialRouteOrchestrator::Resolve(
			PassControlResolved.AfterState,
			OrchestrationFixtures::MakeRequest(
				PassControlResolved.AfterState),
			&PassControlSecondDuplicateProvider);
	bValid &= ExpectDuplicateResult(
		*this,
		TEXT("PassControl repeated canonical duplicate"),
		PassControlResolved.AfterState,
		PassControlSecondDuplicate,
		&PassControlSecondDuplicateProvider,
		1);
	bValid &= TestTrue(
		TEXT("PassControl duplicate Results are deterministic"),
		OrchestrationFixtures::AreResultsEqual(
			PassControlFirstDuplicate,
			PassControlSecondDuplicate));

	const FMatchPlayState AwaitingShot =
		OrchestrationFixtures::MakeAwaitingState(
			ESkillRuleType::CutInsideShot,
			EMatchPlayElectiveBranchIntent::DeadCorner);
	const auto AwaitingShotFirst =
		FMatchPlayCurrentAttackResolveInitialRouteOrchestrator::Resolve(
			AwaitingShot,
			OrchestrationFixtures::MakeRequest(AwaitingShot),
			nullptr);
	bValid &= TestTrue(TEXT("Awaiting deterministic succeeds"),
		AwaitingShotFirst.bSuccess
			&& !AwaitingShotFirst.bBeganNewSession
			&& AwaitingShotFirst.bResolvedNewRoute);
	bValid &= ExpectDuplicate(*this, TEXT("Awaiting deterministic duplicate"),
		AwaitingShotFirst.AfterState, nullptr);

	const FMatchPlayState AwaitingD6 =
		OrchestrationFixtures::MakeAwaitingState(
			ESkillRuleType::ThroughBall);
	RouteFixtures::FQueueRollProvider AwaitingD6Provider;
	AwaitingD6Provider.Enqueue(RouteFixtures::MakeSuccess(3));
	const auto AwaitingD6First =
		FMatchPlayCurrentAttackResolveInitialRouteOrchestrator::Resolve(
			AwaitingD6,
			OrchestrationFixtures::MakeRequest(AwaitingD6),
			&AwaitingD6Provider);
	bValid &= TestTrue(TEXT("Awaiting D6 succeeds"),
		AwaitingD6First.bSuccess
			&& !AwaitingD6First.bBeganNewSession
			&& AwaitingD6First.bResolvedNewRoute);
	RouteFixtures::FQueueRollProvider AwaitingDuplicateProvider;
	AwaitingDuplicateProvider.Enqueue(RouteFixtures::MakeSuccess(6));
	bValid &= ExpectDuplicate(*this, TEXT("Awaiting D6 duplicate"),
		AwaitingD6First.AfterState, &AwaitingDuplicateProvider);
	bValid &= ExpectDuplicate(*this, TEXT("RouteResolved repeated duplicate"),
		AwaitingD6First.AfterState, nullptr);
	return bValid;
}

INITIAL_ROUTE_ORCHESTRATION_TEST(
	FInitialRouteOrchestrationPartialPublicationTest,
	"09.PartialPublication")

bool FInitialRouteOrchestrationPartialPublicationTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayCurrentAttackResolveInitialRouteOrchestrationTests;
	const FMatchPlayState Ready = OrchestrationFixtures::MakeReadyState(
		ESkillRuleType::Cross,
		EMatchPlayElectiveBranchIntent::CrossHigh);
	bool bValid = ExpectRouteFailure(
		*this, TEXT("Missing provider partial publication"), Ready, nullptr,
		EMatchPlayCurrentAttackResolveInitialRouteWriterErrorCode
			::RngProviderUnavailable,
		EMatchPlayCurrentAttackResolveInitialRouteFailureDisposition
			::RetryableExecutionFailure,
		false);
	RouteFixtures::FQueueRollProvider FailureProvider;
	FailureProvider.Enqueue(RouteFixtures::MakeFailure());
	bValid &= ExpectRouteFailure(
		*this, TEXT("Provider failure partial publication"),
		Ready, &FailureProvider,
		EMatchPlayCurrentAttackResolveInitialRouteWriterErrorCode
			::RngProviderFailed,
		EMatchPlayCurrentAttackResolveInitialRouteFailureDisposition
			::RetryableExecutionFailure,
		true);
	RouteFixtures::FQueueRollProvider InvalidProvider;
	InvalidProvider.Enqueue(RouteFixtures::MakeSuccess(0));
	bValid &= ExpectRouteFailure(
		*this, TEXT("Invalid output partial publication"),
		Ready, &InvalidProvider,
		EMatchPlayCurrentAttackResolveInitialRouteWriterErrorCode
			::InvalidRngResult,
		EMatchPlayCurrentAttackResolveInitialRouteFailureDisposition
			::NonRetryableExecutionFailure,
		true);
	return bValid;
}

INITIAL_ROUTE_ORCHESTRATION_TEST(
	FInitialRouteOrchestrationDeterminismTest,
	"10.IndependentRunDeterminism")

bool FInitialRouteOrchestrationDeterminismTest::RunTest(
	const FString& Parameters)
{
	bool bValid = true;
	const FMatchPlayState Shot = OrchestrationFixtures::MakeReadyState(
		ESkillRuleType::LongShot,
		EMatchPlayElectiveBranchIntent::DirectShot);
	FMatchPlayCurrentAttackResolveInitialRouteOrchestrationResult
		ShotResults[3];
	for (int32 Index = 0; Index < 3; ++Index)
	{
		ShotResults[Index] =
			FMatchPlayCurrentAttackResolveInitialRouteOrchestrator::Resolve(
				Shot, OrchestrationFixtures::MakeRequest(Shot), nullptr);
	}
	bValid &= TestTrue(TEXT("Deterministic run 1 equals 2"),
		OrchestrationFixtures::AreResultsEqual(
			ShotResults[0], ShotResults[1]));
	bValid &= TestTrue(TEXT("Deterministic run 1 equals 3"),
		OrchestrationFixtures::AreResultsEqual(
			ShotResults[0], ShotResults[2]));

	const FMatchPlayState D6State = OrchestrationFixtures::MakeReadyState(
		ESkillRuleType::ThroughBall);
	RouteFixtures::FQueueRollProvider D6Providers[3];
	FMatchPlayCurrentAttackResolveInitialRouteOrchestrationResult
		D6Results[3];
	for (int32 Index = 0; Index < 3; ++Index)
	{
		D6Providers[Index].Enqueue(RouteFixtures::MakeSuccess(4));
		D6Results[Index] =
			FMatchPlayCurrentAttackResolveInitialRouteOrchestrator::Resolve(
				D6State,
				OrchestrationFixtures::MakeRequest(D6State),
				&D6Providers[Index]);
		bValid &= MatchPlayCurrentAttackResolveInitialRouteOrchestrationTests
			::ExpectProviderObservation(
				*this,
				FString::Printf(TEXT("D6 independent run %d"), Index + 1),
				D6Providers[Index], 1, 0);
	}
	bValid &= TestTrue(TEXT("D6 run 1 equals 2"),
		OrchestrationFixtures::AreResultsEqual(D6Results[0], D6Results[1]));
	bValid &= TestTrue(TEXT("D6 run 1 equals 3"),
		OrchestrationFixtures::AreResultsEqual(D6Results[0], D6Results[2]));

	RouteFixtures::FQueueRollProvider DuplicateProviders[3];
	FMatchPlayCurrentAttackResolveInitialRouteOrchestrationResult
		DuplicateResults[3];
	for (int32 Index = 0; Index < 3; ++Index)
	{
		DuplicateProviders[Index].Enqueue(RouteFixtures::MakeSuccess(6));
		DuplicateResults[Index] =
			FMatchPlayCurrentAttackResolveInitialRouteOrchestrator::Resolve(
				D6Results[0].AfterState,
				OrchestrationFixtures::MakeRequest(D6Results[0].AfterState),
				&DuplicateProviders[Index]);
		bValid &= TestEqual(TEXT("Duplicate provider unused"),
			DuplicateProviders[Index].GetCallCount(), 0);
	}
	bValid &= TestTrue(TEXT("Duplicate run 1 equals 2"),
		OrchestrationFixtures::AreResultsEqual(
			DuplicateResults[0], DuplicateResults[1]));
	bValid &= TestTrue(TEXT("Duplicate run 1 equals 3"),
		OrchestrationFixtures::AreResultsEqual(
			DuplicateResults[0], DuplicateResults[2]));

	const FMatchPlayState FailureState =
		OrchestrationFixtures::MakeReadyState(ESkillRuleType::Cross,
			EMatchPlayElectiveBranchIntent::CrossHigh);
	auto StaleRequest = OrchestrationFixtures::MakeRequest(FailureState);
	--StaleRequest.AttackSequence;
	FMatchPlayCurrentAttackResolveInitialRouteOrchestrationResult
		BeginFailures[3];
	FMatchPlayCurrentAttackResolveInitialRouteOrchestrationResult
		RetryableFailures[3];
	FMatchPlayCurrentAttackResolveInitialRouteOrchestrationResult
		NonRetryableFailures[3];
	RouteFixtures::FQueueRollProvider RetryableProviders[3];
	RouteFixtures::FQueueRollProvider NonRetryableProviders[3];
	for (int32 Index = 0; Index < 3; ++Index)
	{
		BeginFailures[Index] =
			FMatchPlayCurrentAttackResolveInitialRouteOrchestrator::Resolve(
				FailureState, StaleRequest, nullptr);
		RetryableProviders[Index].Enqueue(RouteFixtures::MakeFailure());
		RetryableFailures[Index] =
			FMatchPlayCurrentAttackResolveInitialRouteOrchestrator::Resolve(
				FailureState,
				OrchestrationFixtures::MakeRequest(FailureState),
				&RetryableProviders[Index]);
		NonRetryableProviders[Index].Enqueue(RouteFixtures::MakeSuccess(0));
		NonRetryableFailures[Index] =
			FMatchPlayCurrentAttackResolveInitialRouteOrchestrator::Resolve(
				FailureState,
				OrchestrationFixtures::MakeRequest(FailureState),
				&NonRetryableProviders[Index]);
	}
	bValid &= TestTrue(TEXT("Begin failures deterministic"),
		OrchestrationFixtures::AreResultsEqual(BeginFailures[0], BeginFailures[1])
			&& OrchestrationFixtures::AreResultsEqual(BeginFailures[0], BeginFailures[2]));
	bValid &= TestTrue(TEXT("Retryable failures deterministic"),
		OrchestrationFixtures::AreResultsEqual(RetryableFailures[0], RetryableFailures[1])
			&& OrchestrationFixtures::AreResultsEqual(RetryableFailures[0], RetryableFailures[2]));
	bValid &= TestTrue(TEXT("Nonretryable failures deterministic"),
		OrchestrationFixtures::AreResultsEqual(NonRetryableFailures[0], NonRetryableFailures[1])
			&& OrchestrationFixtures::AreResultsEqual(NonRetryableFailures[0], NonRetryableFailures[2]));
	return bValid;
}

INITIAL_ROUTE_ORCHESTRATION_TEST(
	FInitialRouteOrchestrationComparatorTest,
	"11.ResultComparatorCoverage")

bool FInitialRouteOrchestrationComparatorTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State = OrchestrationFixtures::MakeReadyState(
		ESkillRuleType::Cross,
		EMatchPlayElectiveBranchIntent::CrossHigh);
	RouteFixtures::FQueueRollProvider FirstProvider;
	RouteFixtures::FQueueRollProvider SecondProvider;
	FirstProvider.Enqueue(RouteFixtures::MakeSuccess(3));
	SecondProvider.Enqueue(RouteFixtures::MakeSuccess(3));
	const auto Baseline =
		FMatchPlayCurrentAttackResolveInitialRouteOrchestrator::Resolve(
			State, OrchestrationFixtures::MakeRequest(State), &FirstProvider);
	const auto IndependentEqual =
		FMatchPlayCurrentAttackResolveInitialRouteOrchestrator::Resolve(
			State, OrchestrationFixtures::MakeRequest(State), &SecondProvider);
	bool bValid = TestTrue(TEXT("Comparator accepts independent equality"),
		OrchestrationFixtures::AreResultsEqual(Baseline, IndependentEqual));
	auto ExpectDifferent = [this, &bValid, &Baseline](
		const TCHAR* Label,
		auto Mutator)
	{
		auto Different = Baseline;
		Mutator(Different);
		bValid &= TestFalse(
			Label,
			OrchestrationFixtures::AreResultsEqual(Baseline, Different));
	};
	ExpectDifferent(TEXT("Comparator covers bSuccess"),
		[](auto& Value){ Value.bSuccess = false; });
	ExpectDifferent(TEXT("Comparator covers bBeganNewSession"),
		[](auto& Value){ Value.bBeganNewSession = false; });
	ExpectDifferent(TEXT("Comparator covers bResolvedNewRoute"),
		[](auto& Value){ Value.bResolvedNewRoute = false; });
	ExpectDifferent(TEXT("Comparator covers Request"),
		[](auto& Value){ ++Value.Request.AttackSequence; });
	ExpectDifferent(TEXT("Comparator covers BeforeState"),
		[](auto& Value){ ++Value.BeforeState.CurrentAttack.AttackSequence; });
	ExpectDifferent(TEXT("Comparator covers AfterState"),
		[](auto& Value){ ++Value.AfterState.CurrentAttack.AttackSequence; });
	ExpectDifferent(TEXT("Comparator covers BeginResult"),
		[](auto& Value){ Value.BeginResult.bSuccess = false; });
	ExpectDifferent(TEXT("Comparator covers RouteResult"),
		[](auto& Value){ Value.RouteResult.bSuccess = false; });
	ExpectDifferent(TEXT("Comparator covers FailureStage"),
		[](auto& Value){ Value.FailureStage = EMatchPlayCurrentAttackInitialRouteOrchestrationFailureStage::ResolveInitialRoute; });
	ExpectDifferent(TEXT("Comparator covers FailureDisposition"),
		[](auto& Value){ Value.FailureDisposition = EMatchPlayCurrentAttackResolveInitialRouteFailureDisposition::RetryableExecutionFailure; });
	ExpectDifferent(TEXT("Comparator covers bProviderCalled"),
		[](auto& Value){ Value.bProviderCalled = false; });
	ExpectDifferent(TEXT("Comparator covers ErrorCode"),
		[](auto& Value){ Value.ErrorCode = EMatchPlayCurrentAttackInitialRouteOrchestrationErrorCode::InitialRouteResolutionFailed; });
	ExpectDifferent(TEXT("Comparator covers ErrorMessage"),
		[](auto& Value){ Value.ErrorMessage = TEXT("Different"); });
	return bValid;
}

INITIAL_ROUTE_ORCHESTRATION_TEST(
	FInitialRouteOrchestrationAuthorityTest,
	"12.AuthorityAndDownstreamBoundary")

bool FInitialRouteOrchestrationAuthorityTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayCurrentAttackResolveInitialRouteOrchestrationTests;
	const EMatchPlayThroughBallActualBranch Expected[] = {
		EMatchPlayThroughBallActualBranch::Feet,
		EMatchPlayThroughBallActualBranch::Feet,
		EMatchPlayThroughBallActualBranch::BehindDefense,
		EMatchPlayThroughBallActualBranch::BehindDefense,
		EMatchPlayThroughBallActualBranch::AntiOffside,
		EMatchPlayThroughBallActualBranch::AntiOffside
	};
	bool bValid = true;
	for (int32 D6 = 1; D6 <= 6; ++D6)
	{
		const FMatchPlayState State =
			OrchestrationFixtures::MakeReadyState(ESkillRuleType::ThroughBall);
		RouteFixtures::FQueueRollProvider Provider;
		Provider.Enqueue(RouteFixtures::MakeSuccess(D6));
		const auto Result =
			FMatchPlayCurrentAttackResolveInitialRouteOrchestrator::Resolve(
				State,
				OrchestrationFixtures::MakeRequest(State),
				&Provider);
		bValid &= TestTrue(
			*FString::Printf(TEXT("ThroughBall D6 %d succeeds"), D6),
			Result.bSuccess);
		bValid &= TestEqual(
			*FString::Printf(TEXT("ThroughBall D6 %d branch"), D6),
			Result.RouteResult.ActualBranch.ThroughBall,
			Expected[D6 - 1]);
		bValid &= TestEqual(
			*FString::Printf(TEXT("ThroughBall D6 %d terminal Stage"), D6),
			Result.AfterState.CurrentAttack.ResolutionSession.Stage,
			EMatchPlayCurrentAttackResolutionStage::RouteResolved);
	}

	const FString Orchestrator = LoadSource(
		TEXT("MatchPlayCurrentAttackResolveInitialRouteOrchestrator.cpp"));
	const FString Writer = LoadSource(
		TEXT("MatchPlayCurrentAttackResolveInitialRouteWriter.cpp"));
	bValid &= TestEqual(TEXT("Orchestrator has zero RollD6 calls"),
		Count(Orchestrator, TEXT("RollD6(")), 0);
	bValid &= TestEqual(TEXT("Writer remains sole RollD6 production site"),
		Count(Writer, TEXT("RollD6(")), 1);
	bValid &= TestFalse(TEXT("No provider Result inspection"),
		Orchestrator.Contains(TEXT("ProviderResult"))
			|| Orchestrator.Contains(TEXT("RawD6")));
	bValid &= TestFalse(TEXT("No pending roll or retry marker"),
		Orchestrator.Contains(TEXT("PendingRoll"))
			|| Orchestrator.Contains(TEXT("RetryCount"))
			|| Orchestrator.Contains(TEXT("ResolvingRoute")));
	bValid &= TestFalse(TEXT("No downstream execution"),
		Orchestrator.Contains(TEXT("Formula"))
			|| Orchestrator.Contains(TEXT("Outcome"))
			|| Orchestrator.Contains(TEXT("Completion"))
			|| Orchestrator.Contains(TEXT("BehindDefenseP1"))
			|| Orchestrator.Contains(TEXT("BehindDefenseP2")));
	return bValid;
}

#undef INITIAL_ROUTE_ORCHESTRATION_TEST

#endif
