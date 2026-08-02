#include "MatchPlayCurrentAttackResolveInitialRouteWriter.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "MatchPlayCurrentAttackResolveInitialRouteTestFixtures.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "UObject/UnrealType.h"

namespace ResolveFixtures =
	FMCodex::Tests::MatchPlayCurrentAttackResolveInitialRoute;
namespace SessionFixtures =
	FMCodex::Tests::MatchPlayCurrentAttackResolutionSession;

namespace MatchPlayCurrentAttackResolveInitialRouteTests
{
	bool IsD6Action(const ESkillRuleType ActionType)
	{
		return ActionType == ESkillRuleType::Cross
			|| ActionType == ESkillRuleType::PassControl
			|| ActionType == ESkillRuleType::ThroughBall;
	}

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

	FMatchPlayCurrentAttackActualBranch MakeExpectedBranch(
		const ESkillRuleType ActionType,
		const EMatchPlayElectiveBranchIntent Intent,
		const int32 RawD6)
	{
		FMatchPlayCurrentAttackActualBranch Branch;
		Branch.ActionType = ActionType;
		switch (ActionType)
		{
		case ESkillRuleType::LongShot:
			Branch.LongShot = Intent
				== EMatchPlayElectiveBranchIntent::DirectShot
					? EMatchPlayLongShotActualBranch::DirectShot
					: EMatchPlayLongShotActualBranch::DeadCorner;
			break;
		case ESkillRuleType::CutInsideShot:
			Branch.CutInsideShot = Intent
				== EMatchPlayElectiveBranchIntent::DirectShot
					? EMatchPlayCutInsideShotActualBranch::DirectShot
					: EMatchPlayCutInsideShotActualBranch::DeadCorner;
			break;
		case ESkillRuleType::Cross:
		{
			const bool bSameIntent = RawD6 <= 4;
			const bool bHigh = Intent
				== EMatchPlayElectiveBranchIntent::CrossHigh
					? bSameIntent
					: !bSameIntent;
			Branch.Cross = bHigh
				? EMatchPlayCrossActualBranch::High
				: EMatchPlayCrossActualBranch::Low;
			break;
		}
		case ESkillRuleType::PassControl:
			Branch.PassControl = RawD6 <= 2
				? EMatchPlayPassControlActualBranch::PassAdvance
				: RawD6 <= 4
					? EMatchPlayPassControlActualBranch::DribbleAdvance
					: EMatchPlayPassControlActualBranch::RunAdvance;
			break;
		case ESkillRuleType::ThroughBall:
			Branch.ThroughBall = RawD6 <= 2
				? EMatchPlayThroughBallActualBranch::Feet
				: RawD6 <= 4
					? EMatchPlayThroughBallActualBranch::BehindDefense
					: EMatchPlayThroughBallActualBranch::AntiOffside;
			break;
		case ESkillRuleType::None:
		default:
			break;
		}
		return Branch;
	}

	bool ExpectProviderObservation(
		FAutomationTestBase& Test,
		const FString& Context,
		const ResolveFixtures::FQueueRollProvider& Provider,
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
		for (const EMatchPlayCurrentAttackResolutionRollPurpose Purpose :
			Provider.GetPurposeHistory())
		{
			bValid &= Test.TestEqual(
				*(Context + TEXT(" Purpose")),
				Purpose,
				EMatchPlayCurrentAttackResolutionRollPurpose::InitialRoute);
		}
		bValid &= Test.TestEqual(
			*(Context + TEXT(" remaining")),
			Provider.GetRemainingCount(),
			ExpectedRemaining);
		return bValid;
	}

	bool ExpectGlobalFailureDeterministic(
		FAutomationTestBase& Test,
		const FString& Context,
		const FMatchPlayState& State,
		const FMatchPlayCurrentAttackResolveInitialRouteRequest& Request,
		const
			EMatchPlayCurrentAttackResolveInitialRouteGlobalContextErrorCode
				ExpectedError)
	{
		const FMatchPlayState BeforeState = State;
		const auto BeforeRequest = Request;
		const auto First =
			FMatchPlayCurrentAttackResolveInitialRouteGlobalContextQuery::Query(
				State,
				Request);
		const auto Second =
			FMatchPlayCurrentAttackResolveInitialRouteGlobalContextQuery::Query(
				State,
				Request);
		const auto Third =
			FMatchPlayCurrentAttackResolveInitialRouteGlobalContextQuery::Query(
				State,
				Request);
		const auto LegalFirst =
			FMatchPlayCurrentAttackResolveInitialRouteLegalityQuery::Query(
				State,
				Request);
		const auto LegalSecond =
			FMatchPlayCurrentAttackResolveInitialRouteLegalityQuery::Query(
				State,
				Request);
		const auto LegalThird =
			FMatchPlayCurrentAttackResolveInitialRouteLegalityQuery::Query(
				State,
				Request);
		bool bValid = true;
		bValid &= Test.TestFalse(*(Context + TEXT(" fails")), First.bSuccess);
		bValid &= Test.TestFalse(
			*(Context + TEXT(" not duplicate")),
			First.bIsCanonicalDuplicate);
		bValid &= Test.TestEqual(
			*(Context + TEXT(" exact error")),
			First.ErrorCode,
			ExpectedError);
		bValid &= Test.TestTrue(
			*(Context + TEXT(" run 1 equals run 2")),
			ResolveFixtures::AreGlobalResultsEqual(First, Second));
		bValid &= Test.TestTrue(
			*(Context + TEXT(" run 1 equals run 3")),
			ResolveFixtures::AreGlobalResultsEqual(First, Third));
		bValid &= Test.TestFalse(
			*(Context + TEXT(" illegal")),
			LegalFirst.bIsLegal);
		bValid &= Test.TestEqual(
			*(Context + TEXT(" legality error")),
			LegalFirst.ErrorCode,
			EMatchPlayCurrentAttackResolveInitialRouteLegalityErrorCode
				::GlobalContextFailed);
		bValid &= Test.TestTrue(
			*(Context + TEXT(" legality run 1 equals run 2")),
			ResolveFixtures::AreLegalityResultsEqual(
				LegalFirst,
				LegalSecond));
		bValid &= Test.TestTrue(
			*(Context + TEXT(" legality run 1 equals run 3")),
			ResolveFixtures::AreLegalityResultsEqual(
				LegalFirst,
				LegalThird));
		bValid &= Test.TestTrue(
			*(Context + TEXT(" State unchanged")),
			ResolveFixtures::AreStatesEqual(State, BeforeState));
		bValid &= Test.TestEqual(
			*(Context + TEXT(" Request unchanged")),
			Request.AttackSequence,
			BeforeRequest.AttackSequence);
		return bValid;
	}

	bool ExpectWriterFailure(
		FAutomationTestBase& Test,
		const FString& Context,
		const FMatchPlayState& State,
		const FMatchPlayCurrentAttackResolveInitialRouteRequest& Request,
		ResolveFixtures::FQueueRollProvider* Provider,
		const EMatchPlayCurrentAttackResolveInitialRouteWriterErrorCode
			ExpectedError,
		const EMatchPlayCurrentAttackResolveInitialRouteFailureDisposition
			ExpectedDisposition,
		const int32 ExpectedProviderCalls,
		const FMatchPlayInitialRouteRollProviderResult*
			ExpectedProviderResult = nullptr)
	{
		const FMatchPlayState BeforeState = State;
		const auto BeforeRequest = Request;
		const auto Result =
			FMatchPlayCurrentAttackResolveInitialRouteWriter::Resolve(
				State,
				Request,
				Provider);
		bool bValid = true;
		bValid &= Test.TestFalse(*(Context + TEXT(" fails")), Result.bSuccess);
		bValid &= Test.TestFalse(
			*(Context + TEXT(" no new route")),
			Result.bResolvedNewRoute);
		bValid &= Test.TestEqual(
			*(Context + TEXT(" exact error")),
			Result.ErrorCode,
			ExpectedError);
		bValid &= Test.TestEqual(
			*(Context + TEXT(" disposition")),
			Result.FailureDisposition,
			ExpectedDisposition);
		bValid &= Test.TestFalse(
			*(Context + TEXT(" diagnostic is non-empty")),
			Result.ErrorMessage.IsEmpty());
		bValid &= Test.TestEqual(
			*(Context + TEXT(" bProviderCalled")),
			Result.bProviderCalled,
			ExpectedProviderCalls == 1);
		bValid &= Test.TestTrue(
			*(Context + TEXT(" input State unchanged")),
			ResolveFixtures::AreStatesEqual(State, BeforeState));
		bValid &= Test.TestTrue(
			*(Context + TEXT(" BeforeState exact")),
			ResolveFixtures::AreStatesEqual(Result.BeforeState, BeforeState));
		bValid &= Test.TestTrue(
			*(Context + TEXT(" AfterState atomic")),
			ResolveFixtures::AreStatesEqual(Result.AfterState, BeforeState));
		bValid &= Test.TestEqual(
			*(Context + TEXT(" Request unchanged")),
			Request.AttackSequence,
			BeforeRequest.AttackSequence);
		if (Provider != nullptr)
		{
			bValid &= Test.TestEqual(
				*(Context + TEXT(" provider call count")),
				Provider->GetCallCount(),
				ExpectedProviderCalls);
			bValid &= Test.TestEqual(
				*(Context + TEXT(" provider Purpose count")),
				Provider->GetPurposeHistory().Num(),
				ExpectedProviderCalls);
			for (const auto Purpose : Provider->GetPurposeHistory())
			{
				bValid &= Test.TestEqual(
					*(Context + TEXT(" provider Purpose")),
					Purpose,
					EMatchPlayCurrentAttackResolutionRollPurpose
						::InitialRoute);
			}
		}
		if (ExpectedProviderResult != nullptr)
		{
			bValid &= Test.TestTrue(
				*(Context + TEXT(" ProviderResult snapshot")),
				ResolveFixtures::AreProviderResultsEqual(
					Result.ProviderResult,
					*ExpectedProviderResult));
			if (ExpectedError
				== EMatchPlayCurrentAttackResolveInitialRouteWriterErrorCode
					::RngProviderFailed)
			{
				bValid &= Test.TestEqual(
					*(Context + TEXT(" provider failure diagnostic")),
					Result.ErrorMessage,
					ExpectedProviderResult->ErrorMessage);
			}
		}
		return bValid;
	}

	bool ExpectSuccessfulResolution(
		FAutomationTestBase& Test,
		const FString& Context,
		const ESkillRuleType ActionType,
		const EMatchPlayElectiveBranchIntent Intent,
		const int32 RawD6)
	{
		const FMatchPlayState BeforeState =
			ResolveFixtures::MakeAwaitingState(ActionType, Intent);
		const auto Request = ResolveFixtures::MakeRequest(BeforeState);
		ResolveFixtures::FQueueRollProvider Provider;
		if (IsD6Action(ActionType))
		{
			Provider.Enqueue(ResolveFixtures::MakeSuccess(RawD6));
		}
		else
		{
			Provider.Enqueue(ResolveFixtures::MakeFailure(
				TEXT("Deterministic path must not consume provider.")));
		}
		const auto Result =
			FMatchPlayCurrentAttackResolveInitialRouteWriter::Resolve(
				BeforeState,
				Request,
				&Provider);
		const FMatchPlayCurrentAttackActualBranch ExpectedBranch =
			MakeExpectedBranch(ActionType, Intent, RawD6);
		const int32 ExpectedCalls = IsD6Action(ActionType) ? 1 : 0;
		const int32 ExpectedRollCount = ExpectedCalls;
		bool bValid = true;
		bValid &= Test.TestTrue(*(Context + TEXT(" succeeds")), Result.bSuccess);
		bValid &= Test.TestTrue(
			*(Context + TEXT(" resolves new route")),
			Result.bResolvedNewRoute);
		bValid &= Test.TestEqual(
			*(Context + TEXT(" writer error None")),
			Result.ErrorCode,
			EMatchPlayCurrentAttackResolveInitialRouteWriterErrorCode::None);
		bValid &= Test.TestEqual(
			*(Context + TEXT(" disposition None")),
			Result.FailureDisposition,
			EMatchPlayCurrentAttackResolveInitialRouteFailureDisposition::None);
		bValid &= Test.TestEqual(
			*(Context + TEXT(" bProviderCalled")),
			Result.bProviderCalled,
			ExpectedCalls == 1);
		bValid &= Test.TestTrue(
			*(Context + TEXT(" Global succeeded")),
			Result.GlobalContextResult.bSuccess);
		bValid &= Test.TestTrue(
			*(Context + TEXT(" Legality succeeded")),
			Result.LegalityResult.bIsLegal);
		bValid &= Test.TestTrue(
			*(Context + TEXT(" Mapping succeeded")),
			Result.MappingResult.bSuccess);
		bValid &= ExpectProviderObservation(
			Test,
			Context,
			Provider,
			ExpectedCalls,
			1 - ExpectedCalls);
		bValid &= Test.TestTrue(
			*(Context + TEXT(" expected branch")),
			ResolveFixtures::AreBranchesEqual(
				Result.ActualBranch,
				ExpectedBranch));
		const auto& Session =
			Result.AfterState.CurrentAttack.ResolutionSession;
		bValid &= Test.TestEqual(
			*(Context + TEXT(" RouteResolved")),
			Session.Stage,
			EMatchPlayCurrentAttackResolutionStage::RouteResolved);
		bValid &= Test.TestTrue(
			*(Context + TEXT(" branch presence")),
			Session.bHasActualBranch);
		bValid &= Test.TestTrue(
			*(Context + TEXT(" stored branch")),
			ResolveFixtures::AreBranchesEqual(
				Session.ActualBranch,
				ExpectedBranch));
		bValid &= Test.TestEqual(
			*(Context + TEXT(" roll count")),
			Session.InitialRouteRollRecords.Num(),
			ExpectedRollCount);
		bValid &= Test.TestTrue(
			*(Context + TEXT(" result roll snapshot")),
			ResolveFixtures::AreRollRecordsEqual(
				Result.InitialRouteRollRecords,
				Session.InitialRouteRollRecords));
		if (ExpectedRollCount == 1)
		{
			bValid &= Test.TestTrue(
				*(Context + TEXT(" ProviderResult snapshot")),
				ResolveFixtures::AreProviderResultsEqual(
					Result.ProviderResult,
					ResolveFixtures::MakeSuccess(RawD6)));
			bValid &= Test.TestEqual(
				*(Context + TEXT(" record Purpose")),
				Session.InitialRouteRollRecords[0].Purpose,
				EMatchPlayCurrentAttackResolutionRollPurpose::InitialRoute);
			bValid &= Test.TestEqual(
				*(Context + TEXT(" record RawD6")),
				Session.InitialRouteRollRecords[0].RawD6,
				RawD6);
		}
		bValid &= Test.TestTrue(
			*(Context + TEXT(" candidate canonical")),
			Result.CandidateValidationResult.bIsCanonical);
		bValid &= Test.TestTrue(
			*(Context + TEXT(" final canonical")),
			FMatchPlayCurrentAttackResolutionSessionStateValidator::Validate(
				Result.AfterState).bIsCanonical);
		bValid &= Test.TestTrue(
			*(Context + TEXT(" only route fields changed")),
			ResolveFixtures::OnlyRouteFieldsChanged(
				BeforeState,
				Result.AfterState));
		bValid &= Test.TestTrue(
			*(Context + TEXT(" input State unchanged")),
			ResolveFixtures::AreStatesEqual(
				BeforeState,
				Result.BeforeState));
		return bValid;
	}
}

#define INITIAL_ROUTE_RESOLVER_TEST(ClassName, TestName) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
		ClassName, \
		"FMCodex.CoreRules.MatchPlayCurrentAttackResolveInitialRoute." TestName, \
		EAutomationTestFlags::EditorContext \
			| EAutomationTestFlags::EngineFilter)

INITIAL_ROUTE_RESOLVER_TEST(
	FInitialRouteResolverSurfaceTest,
	"01.RequestAndProductionSurface")

bool FInitialRouteResolverSurfaceTest::RunTest(const FString& Parameters)
{
	using namespace MatchPlayCurrentAttackResolveInitialRouteTests;
	int32 PropertyCount = 0;
	FName OnlyProperty = NAME_None;
	for (TFieldIterator<FProperty> It(
		FMatchPlayCurrentAttackResolveInitialRouteRequest::StaticStruct());
		It;
		++It)
	{
		++PropertyCount;
		OnlyProperty = It->GetFName();
	}
	TestEqual(TEXT("Request has one field"), PropertyCount, 1);
	TestEqual(
		TEXT("Request field is AttackSequence"),
		OnlyProperty,
		FName(TEXT("AttackSequence")));

	const FString Types = LoadSource(
		TEXT("MatchPlayCurrentAttackResolveInitialRouteTypes.h"));
	const FString GlobalHeader = LoadSource(
		TEXT("MatchPlayCurrentAttackResolveInitialRouteGlobalContextQuery.h"));
	const FString GlobalCpp = LoadSource(
		TEXT("MatchPlayCurrentAttackResolveInitialRouteGlobalContextQuery.cpp"));
	const FString LegalityHeader = LoadSource(
		TEXT("MatchPlayCurrentAttackResolveInitialRouteLegalityQuery.h"));
	const FString LegalityCpp = LoadSource(
		TEXT("MatchPlayCurrentAttackResolveInitialRouteLegalityQuery.cpp"));
	const FString WriterHeader = LoadSource(
		TEXT("MatchPlayCurrentAttackResolveInitialRouteWriter.h"));
	const FString WriterCpp = LoadSource(
		TEXT("MatchPlayCurrentAttackResolveInitialRouteWriter.cpp"));
	TestFalse(TEXT("Request is not BlueprintType"),
		Types.Contains(TEXT("USTRUCT(BlueprintType)")));
	TestFalse(TEXT("No Blueprint API"),
		GlobalHeader.Contains(TEXT("UFUNCTION"))
			|| LegalityHeader.Contains(TEXT("UFUNCTION"))
			|| WriterHeader.Contains(TEXT("UFUNCTION"))
			|| WriterHeader.Contains(TEXT("Blueprint")));
	TestFalse(TEXT("Global has no provider"),
		GlobalHeader.Contains(TEXT("RollProvider"))
			|| GlobalCpp.Contains(TEXT("RollD6(")));
	TestFalse(TEXT("Legality has no provider"),
		LegalityHeader.Contains(TEXT("RollProvider"))
			|| LegalityCpp.Contains(TEXT("RollD6(")));
	TestEqual(TEXT("Writer calls provider at one site"),
		Count(WriterCpp, TEXT("RollD6(")), 1);
	TestEqual(TEXT("Writer calls unified mapper at one site"),
		Count(WriterCpp,
			TEXT("FMatchPlayCurrentAttackInitialRouteMappingQuery::Map")),
		1);
	TestTrue(TEXT("Writer uses WorkingState"),
		WriterCpp.Contains(TEXT("FMatchPlayState WorkingState = BeforeState")));
	TestTrue(TEXT("Writer validates candidate"),
		WriterCpp.Contains(
			TEXT("ResolutionSessionStateValidator::Validate")));
	TestFalse(TEXT("No concrete RNG"),
		WriterCpp.Contains(TEXT("FMath::Rand"))
			|| WriterCpp.Contains(TEXT("FRandomStream"))
			|| WriterCpp.Contains(TEXT("RandomSeed")));
	TestFalse(TEXT("No downstream implementation"),
		WriterCpp.Contains(TEXT("FormulaResolver"))
			|| WriterCpp.Contains(TEXT("OutcomeQuery"))
			|| WriterCpp.Contains(TEXT("Completion"))
			|| WriterCpp.Contains(TEXT("BehindDefenseP1")));
	return true;
}

INITIAL_ROUTE_RESOLVER_TEST(
	FInitialRouteResolverGlobalSuccessTest,
	"02.GlobalAndLegalitySuccessDeterminism")

bool FInitialRouteResolverGlobalSuccessTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState Awaiting = ResolveFixtures::MakeAwaitingState(
		ESkillRuleType::Cross,
		EMatchPlayElectiveBranchIntent::CrossLow);
	const auto Request = ResolveFixtures::MakeRequest(Awaiting);
	const FMatchPlayState BeforeState = Awaiting;
	const auto First =
		FMatchPlayCurrentAttackResolveInitialRouteGlobalContextQuery::Query(
			Awaiting,
			Request);
	const auto Second =
		FMatchPlayCurrentAttackResolveInitialRouteGlobalContextQuery::Query(
			Awaiting,
			Request);
	const auto Third =
		FMatchPlayCurrentAttackResolveInitialRouteGlobalContextQuery::Query(
			Awaiting,
			Request);
	TestTrue(TEXT("Awaiting Global succeeds"), First.bSuccess);
	TestFalse(TEXT("Awaiting is first resolution"),
		First.bIsCanonicalDuplicate);
	TestEqual(TEXT("Global Action"), First.ActionType, ESkillRuleType::Cross);
	TestEqual(TEXT("Global Intent"), First.Intent,
		EMatchPlayElectiveBranchIntent::CrossLow);
	TestTrue(TEXT("Cross requires D6"), First.bRequiresInitialRouteD6);
	TestEqual(TEXT("Expected Purpose"), First.ExpectedRollPurpose,
		EMatchPlayCurrentAttackResolutionRollPurpose::InitialRoute);
	TestTrue(TEXT("Awaiting existing branch default"),
		ResolveFixtures::AreBranchesEqual(
			First.ExistingActualBranch,
			FMatchPlayCurrentAttackActualBranch()));
	TestTrue(TEXT("Awaiting existing rolls empty"),
		First.ExistingInitialRouteRollRecords.IsEmpty());
	TestTrue(TEXT("Global run 1 equals 2"),
		ResolveFixtures::AreGlobalResultsEqual(First, Second));
	TestTrue(TEXT("Global run 1 equals 3"),
		ResolveFixtures::AreGlobalResultsEqual(First, Third));

	const auto LegalFirst =
		FMatchPlayCurrentAttackResolveInitialRouteLegalityQuery::Query(
			Awaiting,
			Request);
	const auto LegalSecond =
		FMatchPlayCurrentAttackResolveInitialRouteLegalityQuery::Query(
			Awaiting,
			Request);
	const auto LegalThird =
		FMatchPlayCurrentAttackResolveInitialRouteLegalityQuery::Query(
			Awaiting,
			Request);
	TestTrue(TEXT("Awaiting legal"), LegalFirst.bIsLegal);
	TestTrue(TEXT("Legality run 1 equals 2"),
		ResolveFixtures::AreLegalityResultsEqual(LegalFirst, LegalSecond));
	TestTrue(TEXT("Legality run 1 equals 3"),
		ResolveFixtures::AreLegalityResultsEqual(LegalFirst, LegalThird));
	TestTrue(TEXT("Queries do not mutate State"),
		ResolveFixtures::AreStatesEqual(Awaiting, BeforeState));

	ResolveFixtures::FQueueRollProvider ResolveProvider;
	ResolveProvider.Enqueue(ResolveFixtures::MakeSuccess(5));
	const auto ResolvedWriter =
		FMatchPlayCurrentAttackResolveInitialRouteWriter::Resolve(
			Awaiting,
			Request,
			&ResolveProvider);
	const FMatchPlayState Resolved = ResolvedWriter.AfterState;
	const auto DuplicateRequest = ResolveFixtures::MakeRequest(Resolved);
	const auto DuplicateFirst =
		FMatchPlayCurrentAttackResolveInitialRouteGlobalContextQuery::Query(
			Resolved,
			DuplicateRequest);
	const auto DuplicateSecond =
		FMatchPlayCurrentAttackResolveInitialRouteGlobalContextQuery::Query(
			Resolved,
			DuplicateRequest);
	const auto DuplicateThird =
		FMatchPlayCurrentAttackResolveInitialRouteGlobalContextQuery::Query(
			Resolved,
			DuplicateRequest);
	TestTrue(TEXT("Duplicate Global succeeds"), DuplicateFirst.bSuccess);
	TestTrue(TEXT("Duplicate classified"),
		DuplicateFirst.bIsCanonicalDuplicate);
	TestTrue(TEXT("Duplicate branch snapshot"),
		ResolveFixtures::AreBranchesEqual(
			DuplicateFirst.ExistingActualBranch,
			Resolved.CurrentAttack.ResolutionSession.ActualBranch));
	TestTrue(TEXT("Duplicate roll snapshot"),
		ResolveFixtures::AreRollRecordsEqual(
			DuplicateFirst.ExistingInitialRouteRollRecords,
			Resolved.CurrentAttack.ResolutionSession
				.InitialRouteRollRecords));
	TestTrue(TEXT("Duplicate Global run 1 equals 2"),
		ResolveFixtures::AreGlobalResultsEqual(
			DuplicateFirst,
			DuplicateSecond));
	TestTrue(TEXT("Duplicate Global run 1 equals 3"),
		ResolveFixtures::AreGlobalResultsEqual(
			DuplicateFirst,
			DuplicateThird));
	const auto DuplicateLegalFirst =
		FMatchPlayCurrentAttackResolveInitialRouteLegalityQuery::Query(
			Resolved,
			DuplicateRequest);
	const auto DuplicateLegalSecond =
		FMatchPlayCurrentAttackResolveInitialRouteLegalityQuery::Query(
			Resolved,
			DuplicateRequest);
	const auto DuplicateLegalThird =
		FMatchPlayCurrentAttackResolveInitialRouteLegalityQuery::Query(
			Resolved,
			DuplicateRequest);
	TestTrue(TEXT("Duplicate legal"), DuplicateLegalFirst.bIsLegal);
	TestTrue(TEXT("Duplicate Legality run 1 equals 2"),
		ResolveFixtures::AreLegalityResultsEqual(
			DuplicateLegalFirst,
			DuplicateLegalSecond));
	TestTrue(TEXT("Duplicate Legality run 1 equals 3"),
		ResolveFixtures::AreLegalityResultsEqual(
			DuplicateLegalFirst,
			DuplicateLegalThird));
	return true;
}

INITIAL_ROUTE_RESOLVER_TEST(
	FInitialRouteResolverGlobalFailureTest,
	"03.GlobalFailurePriorityAndDeterminism")

bool FInitialRouteResolverGlobalFailureTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayCurrentAttackResolveInitialRouteTests;
	bool bValid = true;
	const FMatchPlayState Canonical = ResolveFixtures::MakeAwaitingState(
		ESkillRuleType::Cross,
		EMatchPlayElectiveBranchIntent::CrossHigh);
	const auto CanonicalRequest = ResolveFixtures::MakeRequest(Canonical);

	FMatchPlayState State = Canonical;
	State.RuntimeState.bIsInitialized = false;
	bValid &= ExpectGlobalFailureDeterministic(
		*this, TEXT("Uninitialized"), State, CanonicalRequest,
		EMatchPlayCurrentAttackResolveInitialRouteGlobalContextErrorCode
			::MatchPlayNotInitialized);
	State = Canonical;
	State.bHasCurrentAttack = false;
	bValid &= ExpectGlobalFailureDeterministic(
		*this, TEXT("No CurrentAttack"), State, CanonicalRequest,
		EMatchPlayCurrentAttackResolveInitialRouteGlobalContextErrorCode
			::NoCurrentAttack);
	State = Canonical;
	State.CurrentAttack.AttackSequence = 0;
	bValid &= ExpectGlobalFailureDeterministic(
		*this, TEXT("Invalid current sequence"), State, CanonicalRequest,
		EMatchPlayCurrentAttackResolveInitialRouteGlobalContextErrorCode
			::InvalidCurrentAttackAttackSequence);
	auto Request = CanonicalRequest;
	Request.AttackSequence = 0;
	bValid &= ExpectGlobalFailureDeterministic(
		*this, TEXT("Invalid request sequence"), Canonical, Request,
		EMatchPlayCurrentAttackResolveInitialRouteGlobalContextErrorCode
			::InvalidRequestAttackSequence);
	Request = CanonicalRequest;
	--Request.AttackSequence;
	State = Canonical;
	State.CurrentAttack.ResolutionSession.bHasActualBranch = true;
	bValid &= ExpectGlobalFailureDeterministic(
		*this, TEXT("Stale precedes corruption"), State, Request,
		EMatchPlayCurrentAttackResolveInitialRouteGlobalContextErrorCode
			::AttackSequenceMismatch);
	Request = CanonicalRequest;
	++Request.AttackSequence;
	bValid &= ExpectGlobalFailureDeterministic(
		*this, TEXT("Ahead sequence"), Canonical, Request,
		EMatchPlayCurrentAttackResolveInitialRouteGlobalContextErrorCode
			::AttackSequenceMismatch);
	State = Canonical;
	State.CurrentAttack.Phase = EMatchPlayCurrentAttackPhase::Deployment;
	State.CurrentAttack.ResolutionSession.bHasActualBranch = true;
	bValid &= ExpectGlobalFailureDeterministic(
		*this, TEXT("Wrong Phase precedes corruption"), State,
		CanonicalRequest,
		EMatchPlayCurrentAttackResolveInitialRouteGlobalContextErrorCode
			::CurrentAttackNotInResolution);
	State = SessionFixtures::MakeReadyState(ESkillRuleType::LongShot);
	bValid &= ExpectGlobalFailureDeterministic(
		*this, TEXT("No Session"), State,
		ResolveFixtures::MakeRequest(State),
		EMatchPlayCurrentAttackResolveInitialRouteGlobalContextErrorCode
			::NoResolutionSession);
	State = Canonical;
	State.CurrentAttack.ResolutionSession.bHasActualBranch = true;
	bValid &= ExpectGlobalFailureDeterministic(
		*this, TEXT("Malformed AwaitingRoute"), State, CanonicalRequest,
		EMatchPlayCurrentAttackResolveInitialRouteGlobalContextErrorCode
			::InvalidResolutionSessionState);
	State = Canonical;
	++State.CurrentAttack.ResolutionSession.AttackSequence;
	bValid &= ExpectGlobalFailureDeterministic(
		*this, TEXT("Session sequence provenance"), State,
		CanonicalRequest,
		EMatchPlayCurrentAttackResolveInitialRouteGlobalContextErrorCode
			::SessionAttackSequenceMismatch);
	ResolveFixtures::FQueueRollProvider ResolveProvider;
	ResolveProvider.Enqueue(ResolveFixtures::MakeSuccess(4));
	const auto ResolvedWriter =
		FMatchPlayCurrentAttackResolveInitialRouteWriter::Resolve(
			Canonical,
			CanonicalRequest,
			&ResolveProvider);
	State = ResolvedWriter.AfterState;
	State.CurrentAttack.ResolutionSession.InitialRouteRollRecords[0].RawD6 = 0;
	bValid &= ExpectGlobalFailureDeterministic(
		*this, TEXT("Malformed RouteResolved is not duplicate"), State,
		ResolveFixtures::MakeRequest(State),
		EMatchPlayCurrentAttackResolveInitialRouteGlobalContextErrorCode
			::InvalidResolutionSessionState);
	State = Canonical;
	State.CurrentAttack.SelectedAction.ActionType = ESkillRuleType::None;
	State.CurrentAttack.ResolutionSession.Bundle.Binding.ActionType =
		ESkillRuleType::None;
	bValid &= ExpectGlobalFailureDeterministic(
		*this, TEXT("Unsupported Action is invalid canonical Session"),
		State, CanonicalRequest,
		EMatchPlayCurrentAttackResolveInitialRouteGlobalContextErrorCode
			::InvalidResolutionSessionState);
	State = Canonical;
	State.CurrentAttack.SelectedAction.ElectiveBranchIntent =
		EMatchPlayElectiveBranchIntent::DirectShot;
	State.CurrentAttack.ResolutionSession.Bundle.Binding
		.ElectiveBranchIntent =
			EMatchPlayElectiveBranchIntent::DirectShot;
	bValid &= ExpectGlobalFailureDeterministic(
		*this, TEXT("Invalid Intent is invalid canonical Session"),
		State, CanonicalRequest,
		EMatchPlayCurrentAttackResolveInitialRouteGlobalContextErrorCode
			::InvalidResolutionSessionState);
	return bValid;
}

INITIAL_ROUTE_RESOLVER_TEST(
	FInitialRouteResolverFiveActionTest,
	"04.FiveActionFirstResolutionMatrix")

bool FInitialRouteResolverFiveActionTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayCurrentAttackResolveInitialRouteTests;
	bool bValid = true;
	for (const ESkillRuleType ActionType : {
		ESkillRuleType::LongShot,
		ESkillRuleType::CutInsideShot})
	{
		for (const EMatchPlayElectiveBranchIntent Intent : {
			EMatchPlayElectiveBranchIntent::DirectShot,
			EMatchPlayElectiveBranchIntent::DeadCorner})
		{
			bValid &= ExpectSuccessfulResolution(
				*this,
				FString::Printf(
					TEXT("Shot action %d intent %d"),
					static_cast<int32>(ActionType),
					static_cast<int32>(Intent)),
				ActionType,
				Intent,
				0);
		}
	}
	for (const EMatchPlayElectiveBranchIntent Intent : {
		EMatchPlayElectiveBranchIntent::CrossHigh,
		EMatchPlayElectiveBranchIntent::CrossLow})
	{
		for (int32 D6 = 1; D6 <= 6; ++D6)
		{
			bValid &= ExpectSuccessfulResolution(
				*this,
				FString::Printf(
					TEXT("Cross intent %d D6 %d"),
					static_cast<int32>(Intent),
					D6),
				ESkillRuleType::Cross,
				Intent,
				D6);
		}
	}
	for (const ESkillRuleType ActionType : {
		ESkillRuleType::PassControl,
		ESkillRuleType::ThroughBall})
	{
		for (int32 D6 = 1; D6 <= 6; ++D6)
		{
			bValid &= ExpectSuccessfulResolution(
				*this,
				FString::Printf(
					TEXT("D6 action %d D6 %d"),
					static_cast<int32>(ActionType),
					D6),
				ActionType,
				EMatchPlayElectiveBranchIntent::None,
				D6);
		}
	}
	return bValid;
}

INITIAL_ROUTE_RESOLVER_TEST(
	FInitialRouteResolverProviderFailureTest,
	"05.ProviderFailureAndRawShapeMatrix")

bool FInitialRouteResolverProviderFailureTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayCurrentAttackResolveInitialRouteTests;
	const FMatchPlayState State = ResolveFixtures::MakeAwaitingState(
		ESkillRuleType::Cross,
		EMatchPlayElectiveBranchIntent::CrossHigh);
	const auto Request = ResolveFixtures::MakeRequest(State);
	bool bValid = ExpectWriterFailure(
		*this, TEXT("Missing provider"), State, Request, nullptr,
		EMatchPlayCurrentAttackResolveInitialRouteWriterErrorCode
			::RngProviderUnavailable,
		EMatchPlayCurrentAttackResolveInitialRouteFailureDisposition
			::RetryableExecutionFailure,
		0);

	auto RunProviderCase = [this, &State, &Request, &bValid](
		const TCHAR* Label,
		const FMatchPlayInitialRouteRollProviderResult& ProviderResult,
		const EMatchPlayCurrentAttackResolveInitialRouteWriterErrorCode Error,
		const EMatchPlayCurrentAttackResolveInitialRouteFailureDisposition
			Disposition)
	{
		ResolveFixtures::FQueueRollProvider Provider;
		Provider.Enqueue(ProviderResult);
		bValid &= ExpectWriterFailure(
			*this,
			Label,
			State,
			Request,
			&Provider,
			Error,
			Disposition,
			1,
			&ProviderResult);
		bValid &= TestEqual(
			*FString::Printf(TEXT("%s queue consumed once"), Label),
			Provider.GetRemainingCount(),
			0);
	};

	RunProviderCase(
		TEXT("Canonical provider failure"),
		ResolveFixtures::MakeFailure(),
		EMatchPlayCurrentAttackResolveInitialRouteWriterErrorCode
			::RngProviderFailed,
		EMatchPlayCurrentAttackResolveInitialRouteFailureDisposition
			::RetryableExecutionFailure);
	RunProviderCase(
		TEXT("RawD6 zero"),
		ResolveFixtures::MakeSuccess(0),
		EMatchPlayCurrentAttackResolveInitialRouteWriterErrorCode
			::InvalidRngResult,
		EMatchPlayCurrentAttackResolveInitialRouteFailureDisposition
			::NonRetryableExecutionFailure);
	RunProviderCase(
		TEXT("RawD6 seven"),
		ResolveFixtures::MakeSuccess(7),
		EMatchPlayCurrentAttackResolveInitialRouteWriterErrorCode
			::InvalidRngResult,
		EMatchPlayCurrentAttackResolveInitialRouteFailureDisposition
			::NonRetryableExecutionFailure);

	FMatchPlayInitialRouteRollProviderResult Malformed;
	RunProviderCase(
		TEXT("Failure with None error"),
		Malformed,
		EMatchPlayCurrentAttackResolveInitialRouteWriterErrorCode
			::InvalidRngResult,
		EMatchPlayCurrentAttackResolveInitialRouteFailureDisposition
			::NonRetryableExecutionFailure);
	Malformed = ResolveFixtures::MakeFailure();
	Malformed.RawD6 = 4;
	RunProviderCase(
		TEXT("Failure with nonzero D6"),
		Malformed,
		EMatchPlayCurrentAttackResolveInitialRouteWriterErrorCode
			::InvalidRngResult,
		EMatchPlayCurrentAttackResolveInitialRouteFailureDisposition
			::NonRetryableExecutionFailure);
	Malformed = ResolveFixtures::MakeSuccess(4);
	Malformed.ErrorCode =
		EMatchPlayInitialRouteRollProviderErrorCode::ProviderFailure;
	RunProviderCase(
		TEXT("Success with failure code"),
		Malformed,
		EMatchPlayCurrentAttackResolveInitialRouteWriterErrorCode
			::InvalidRngResult,
		EMatchPlayCurrentAttackResolveInitialRouteFailureDisposition
			::NonRetryableExecutionFailure);
	Malformed = ResolveFixtures::MakeSuccess(4);
	Malformed.ErrorMessage = TEXT("Unexpected success diagnostic.");
	RunProviderCase(
		TEXT("Success with message"),
		Malformed,
		EMatchPlayCurrentAttackResolveInitialRouteWriterErrorCode
			::InvalidRngResult,
		EMatchPlayCurrentAttackResolveInitialRouteFailureDisposition
			::NonRetryableExecutionFailure);
	return bValid;
}

INITIAL_ROUTE_RESOLVER_TEST(
	FInitialRouteResolverStaticFailureTest,
	"06.StaticFailureProviderCallMatrix")

bool FInitialRouteResolverStaticFailureTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayCurrentAttackResolveInitialRouteTests;
	const FMatchPlayState Canonical = ResolveFixtures::MakeAwaitingState(
		ESkillRuleType::Cross,
		EMatchPlayElectiveBranchIntent::CrossHigh);
	const auto CanonicalRequest = ResolveFixtures::MakeRequest(Canonical);
	bool bValid = true;
	auto Run = [this, &bValid](
		const TCHAR* Label,
		const FMatchPlayState& State,
		const FMatchPlayCurrentAttackResolveInitialRouteRequest& Request)
	{
		ResolveFixtures::FQueueRollProvider Provider;
		Provider.Enqueue(ResolveFixtures::MakeSuccess(3));
		bValid &= ExpectWriterFailure(
			*this,
			Label,
			State,
			Request,
			&Provider,
			EMatchPlayCurrentAttackResolveInitialRouteWriterErrorCode
				::GlobalContextFailed,
			EMatchPlayCurrentAttackResolveInitialRouteFailureDisposition
				::None,
			0);
		bValid &= TestEqual(
			*FString::Printf(TEXT("%s queue untouched"), Label),
			Provider.GetRemainingCount(),
			1);
	};

	FMatchPlayState State = Canonical;
	State.RuntimeState.bIsInitialized = false;
	Run(TEXT("Uninitialized"), State, CanonicalRequest);
	State = Canonical;
	State.bHasCurrentAttack = false;
	Run(TEXT("No CurrentAttack"), State, CanonicalRequest);
	State = Canonical;
	State.CurrentAttack.AttackSequence = 0;
	Run(TEXT("Invalid authoritative sequence"), State, CanonicalRequest);
	auto Request = CanonicalRequest;
	Request.AttackSequence = 0;
	Run(TEXT("Invalid request sequence"), Canonical, Request);
	Request = CanonicalRequest;
	--Request.AttackSequence;
	Run(TEXT("Stale sequence"), Canonical, Request);
	Request = CanonicalRequest;
	++Request.AttackSequence;
	Run(TEXT("Ahead sequence"), Canonical, Request);
	State = Canonical;
	State.CurrentAttack.Phase = EMatchPlayCurrentAttackPhase::Deployment;
	Run(TEXT("Wrong Phase"), State, CanonicalRequest);
	State = SessionFixtures::MakeReadyState(ESkillRuleType::LongShot);
	Run(TEXT("No Session"), State, ResolveFixtures::MakeRequest(State));
	State = Canonical;
	State.CurrentAttack.ResolutionSession.bHasActualBranch = true;
	Run(TEXT("Malformed AwaitingRoute"), State, CanonicalRequest);
	State = Canonical;
	State.CurrentAttack.SelectedAction.ActionType = ESkillRuleType::None;
	State.CurrentAttack.ResolutionSession.Bundle.Binding.ActionType =
		ESkillRuleType::None;
	Run(TEXT("Unsupported Action"), State, CanonicalRequest);
	State = Canonical;
	State.CurrentAttack.SelectedAction.ElectiveBranchIntent =
		EMatchPlayElectiveBranchIntent::DirectShot;
	State.CurrentAttack.ResolutionSession.Bundle.Binding
		.ElectiveBranchIntent =
			EMatchPlayElectiveBranchIntent::DirectShot;
	Run(TEXT("Invalid Intent"), State, CanonicalRequest);

	ResolveFixtures::FQueueRollProvider FirstProvider;
	FirstProvider.Enqueue(ResolveFixtures::MakeSuccess(4));
	const auto First =
		FMatchPlayCurrentAttackResolveInitialRouteWriter::Resolve(
			Canonical,
			CanonicalRequest,
			&FirstProvider);
	State = First.AfterState;
	State.CurrentAttack.ResolutionSession.InitialRouteRollRecords[0].RawD6 = 0;
	Run(TEXT("Malformed RouteResolved"), State,
		ResolveFixtures::MakeRequest(State));
	return bValid;
}

INITIAL_ROUTE_RESOLVER_TEST(
	FInitialRouteResolverDuplicateTest,
	"07.FiveActionCanonicalDuplicateMatrix")

bool FInitialRouteResolverDuplicateTest::RunTest(
	const FString& Parameters)
{
	struct FDuplicateCase
	{
		ESkillRuleType ActionType;
		EMatchPlayElectiveBranchIntent Intent;
		int32 D6;
	};
	const FDuplicateCase Cases[] = {
		{ESkillRuleType::LongShot,
			EMatchPlayElectiveBranchIntent::DirectShot, 0},
		{ESkillRuleType::CutInsideShot,
			EMatchPlayElectiveBranchIntent::DeadCorner, 0},
		{ESkillRuleType::Cross,
			EMatchPlayElectiveBranchIntent::CrossHigh, 5},
		{ESkillRuleType::PassControl,
			EMatchPlayElectiveBranchIntent::None, 3},
		{ESkillRuleType::ThroughBall,
			EMatchPlayElectiveBranchIntent::None, 1}
	};
	bool bValid = true;
	for (const FDuplicateCase& Case : Cases)
	{
		const FMatchPlayState Awaiting = ResolveFixtures::MakeAwaitingState(
			Case.ActionType,
			Case.Intent);
		const auto Request = ResolveFixtures::MakeRequest(Awaiting);
		ResolveFixtures::FQueueRollProvider FirstProvider;
		if (MatchPlayCurrentAttackResolveInitialRouteTests::IsD6Action(
			Case.ActionType))
		{
			FirstProvider.Enqueue(ResolveFixtures::MakeSuccess(Case.D6));
		}
		const auto First =
			FMatchPlayCurrentAttackResolveInitialRouteWriter::Resolve(
				Awaiting,
				Request,
				&FirstProvider);
		const FMatchPlayState Resolved = First.AfterState;
		const auto DuplicateRequest = ResolveFixtures::MakeRequest(Resolved);
		ResolveFixtures::FQueueRollProvider DuplicateProvider;
		DuplicateProvider.Enqueue(ResolveFixtures::MakeFailure(
			TEXT("Duplicate must not call provider.")));
		const auto Duplicate =
			FMatchPlayCurrentAttackResolveInitialRouteWriter::Resolve(
				Resolved,
				DuplicateRequest,
				&DuplicateProvider);
		ResolveFixtures::FQueueRollProvider RepeatedProvider;
		RepeatedProvider.Enqueue(ResolveFixtures::MakeSuccess(6));
		const auto Repeated =
			FMatchPlayCurrentAttackResolveInitialRouteWriter::Resolve(
				Resolved,
				DuplicateRequest,
				&RepeatedProvider);
		const FString Context = FString::Printf(
			TEXT("Duplicate action %d"),
			static_cast<int32>(Case.ActionType));
		bValid &= TestTrue(*(Context + TEXT(" succeeds")), Duplicate.bSuccess);
		bValid &= TestFalse(
			*(Context + TEXT(" no new route")),
			Duplicate.bResolvedNewRoute);
		bValid &= TestTrue(
			*(Context + TEXT(" State unchanged")),
			ResolveFixtures::AreStatesEqual(
				Duplicate.AfterState,
				Resolved));
		bValid &= TestTrue(
			*(Context + TEXT(" branch snapshot")),
			ResolveFixtures::AreBranchesEqual(
				Duplicate.ActualBranch,
				Resolved.CurrentAttack.ResolutionSession.ActualBranch));
		bValid &= TestTrue(
			*(Context + TEXT(" roll snapshot")),
			ResolveFixtures::AreRollRecordsEqual(
				Duplicate.InitialRouteRollRecords,
				Resolved.CurrentAttack.ResolutionSession
					.InitialRouteRollRecords));
		bValid &= TestEqual(
			*(Context + TEXT(" provider calls")),
			DuplicateProvider.GetCallCount(),
			0);
		bValid &= TestFalse(
			*(Context + TEXT(" provider not marked called")),
			Duplicate.bProviderCalled);
		bValid &= TestEqual(
			*(Context + TEXT(" repeated provider calls")),
			RepeatedProvider.GetCallCount(),
			0);
		bValid &= TestTrue(
			*(Context + TEXT(" repeated result deterministic")),
			ResolveFixtures::AreWriterResultsEqual(Duplicate, Repeated));
	}
	return bValid;
}

INITIAL_ROUTE_RESOLVER_TEST(
	FInitialRouteResolverWriterDeterminismTest,
	"08.WriterIndependentProviderDeterminism")

bool FInitialRouteResolverWriterDeterminismTest::RunTest(
	const FString& Parameters)
{
	bool bValid = true;
	const FMatchPlayState Shot = ResolveFixtures::MakeAwaitingState(
		ESkillRuleType::LongShot,
		EMatchPlayElectiveBranchIntent::DirectShot);
	const auto ShotRequest = ResolveFixtures::MakeRequest(Shot);
	ResolveFixtures::FQueueRollProvider ShotProviders[3];
	FMatchPlayCurrentAttackResolveInitialRouteWriterResult ShotResults[3];
	for (int32 Index = 0; Index < 3; ++Index)
	{
		ShotProviders[Index].Enqueue(ResolveFixtures::MakeSuccess(6));
		ShotResults[Index] =
			FMatchPlayCurrentAttackResolveInitialRouteWriter::Resolve(
				Shot,
				ShotRequest,
				&ShotProviders[Index]);
		bValid &= TestEqual(TEXT("Shot provider unused"),
			ShotProviders[Index].GetCallCount(), 0);
	}
	bValid &= TestTrue(TEXT("Shot run 1 equals 2"),
		ResolveFixtures::AreWriterResultsEqual(
			ShotResults[0], ShotResults[1]));
	bValid &= TestTrue(TEXT("Shot run 1 equals 3"),
		ResolveFixtures::AreWriterResultsEqual(
			ShotResults[0], ShotResults[2]));
	const auto NullProviderShot =
		FMatchPlayCurrentAttackResolveInitialRouteWriter::Resolve(
			Shot,
			ShotRequest,
			nullptr);
	bValid &= TestTrue(TEXT("Shot accepts null provider"),
		ResolveFixtures::AreWriterResultsEqual(
			ShotResults[0],
			NullProviderShot));

	const FMatchPlayState D6State = ResolveFixtures::MakeAwaitingState(
		ESkillRuleType::ThroughBall,
		EMatchPlayElectiveBranchIntent::None);
	const auto D6Request = ResolveFixtures::MakeRequest(D6State);
	ResolveFixtures::FQueueRollProvider D6Providers[3];
	FMatchPlayCurrentAttackResolveInitialRouteWriterResult D6Results[3];
	for (int32 Index = 0; Index < 3; ++Index)
	{
		D6Providers[Index].Enqueue(ResolveFixtures::MakeSuccess(4));
		D6Results[Index] =
			FMatchPlayCurrentAttackResolveInitialRouteWriter::Resolve(
				D6State,
				D6Request,
				&D6Providers[Index]);
		bValid &= MatchPlayCurrentAttackResolveInitialRouteTests
			::ExpectProviderObservation(
				*this,
				FString::Printf(TEXT("D6 run %d"), Index + 1),
				D6Providers[Index],
				1,
				0);
	}
	bValid &= TestTrue(TEXT("D6 run 1 equals 2"),
		ResolveFixtures::AreWriterResultsEqual(D6Results[0], D6Results[1]));
	bValid &= TestTrue(TEXT("D6 run 1 equals 3"),
		ResolveFixtures::AreWriterResultsEqual(D6Results[0], D6Results[2]));
	return bValid;
}

INITIAL_ROUTE_RESOLVER_TEST(
	FInitialRouteResolverComparatorTest,
	"09.WriterResultComparatorCoverage")

bool FInitialRouteResolverComparatorTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State = ResolveFixtures::MakeAwaitingState(
		ESkillRuleType::Cross,
		EMatchPlayElectiveBranchIntent::CrossHigh);
	const auto Request = ResolveFixtures::MakeRequest(State);
	ResolveFixtures::FQueueRollProvider FirstProvider;
	ResolveFixtures::FQueueRollProvider SecondProvider;
	FirstProvider.Enqueue(ResolveFixtures::MakeSuccess(3));
	SecondProvider.Enqueue(ResolveFixtures::MakeSuccess(3));
	const auto Baseline =
		FMatchPlayCurrentAttackResolveInitialRouteWriter::Resolve(
			State, Request, &FirstProvider);
	const auto IndependentEqual =
		FMatchPlayCurrentAttackResolveInitialRouteWriter::Resolve(
			State, Request, &SecondProvider);
	bool bValid = TestTrue(TEXT("Comparator accepts independent equal results"),
		ResolveFixtures::AreWriterResultsEqual(Baseline, IndependentEqual));
	auto ExpectDifferent = [this, &bValid, &Baseline](
		const TCHAR* Label,
		auto Mutator)
	{
		auto Different = Baseline;
		Mutator(Different);
		bValid &= TestFalse(
			Label,
			ResolveFixtures::AreWriterResultsEqual(Baseline, Different));
	};
	ExpectDifferent(TEXT("Comparator covers bSuccess"),
		[](auto& Value){ Value.bSuccess = false; });
	ExpectDifferent(TEXT("Comparator covers bResolvedNewRoute"),
		[](auto& Value){ Value.bResolvedNewRoute = false; });
	ExpectDifferent(TEXT("Comparator covers Request"),
		[](auto& Value){ ++Value.Request.AttackSequence; });
	ExpectDifferent(TEXT("Comparator covers BeforeState"),
		[](auto& Value){ ++Value.BeforeState.CurrentAttack.AttackSequence; });
	ExpectDifferent(TEXT("Comparator covers AfterState"),
		[](auto& Value){ ++Value.AfterState.CurrentAttack.AttackSequence; });
	ExpectDifferent(TEXT("Comparator covers ActualBranch"),
		[](auto& Value){ Value.ActualBranch.Cross = EMatchPlayCrossActualBranch::Low; });
	ExpectDifferent(TEXT("Comparator covers roll records"),
		[](auto& Value){ ++Value.InitialRouteRollRecords[0].RawD6; });
	ExpectDifferent(TEXT("Comparator covers ErrorCode"),
		[](auto& Value){ Value.ErrorCode = EMatchPlayCurrentAttackResolveInitialRouteWriterErrorCode::InvalidRngResult; });
	ExpectDifferent(TEXT("Comparator covers ErrorMessage"),
		[](auto& Value){ Value.ErrorMessage = TEXT("Different"); });
	ExpectDifferent(TEXT("Comparator covers disposition"),
		[](auto& Value){ Value.FailureDisposition = EMatchPlayCurrentAttackResolveInitialRouteFailureDisposition::RetryableExecutionFailure; });
	ExpectDifferent(TEXT("Comparator covers Global"),
		[](auto& Value){ Value.GlobalContextResult.bSuccess = false; });
	ExpectDifferent(TEXT("Comparator covers Legality"),
		[](auto& Value){ Value.LegalityResult.bIsLegal = false; });
	ExpectDifferent(TEXT("Comparator covers provider-called"),
		[](auto& Value){ Value.bProviderCalled = false; });
	ExpectDifferent(TEXT("Comparator covers ProviderResult"),
		[](auto& Value){ ++Value.ProviderResult.RawD6; });
	ExpectDifferent(TEXT("Comparator covers MappingResult"),
		[](auto& Value){ ++Value.MappingResult.Input.InitialRouteD6; });
	ExpectDifferent(TEXT("Comparator covers candidate validation"),
		[](auto& Value){ Value.CandidateValidationResult.bIsCanonical = false; });
	return bValid;
}

INITIAL_ROUTE_RESOLVER_TEST(
	FInitialRouteResolverThroughBallTest,
	"10.ThroughBallInitialRouteBoundary")

bool FInitialRouteResolverThroughBallTest::RunTest(
	const FString& Parameters)
{
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
		const FMatchPlayState State = ResolveFixtures::MakeAwaitingState(
			ESkillRuleType::ThroughBall);
		ResolveFixtures::FQueueRollProvider Provider;
		Provider.Enqueue(ResolveFixtures::MakeSuccess(D6));
		const auto Result =
			FMatchPlayCurrentAttackResolveInitialRouteWriter::Resolve(
				State,
				ResolveFixtures::MakeRequest(State),
				&Provider);
		bValid &= TestTrue(
			*FString::Printf(TEXT("ThroughBall D6 %d succeeds"), D6),
			Result.bSuccess);
		bValid &= TestEqual(
			*FString::Printf(TEXT("ThroughBall D6 %d branch"), D6),
			Result.ActualBranch.ThroughBall,
			Expected[D6 - 1]);
		bValid &= TestEqual(
			*FString::Printf(TEXT("ThroughBall D6 %d one call"), D6),
			Provider.GetCallCount(),
			1);
	}
	return bValid;
}

INITIAL_ROUTE_RESOLVER_TEST(
	FInitialRouteResolverAuthorityTest,
	"11.AuthorityAndDefensiveBoundary")

bool FInitialRouteResolverAuthorityTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayCurrentAttackResolveInitialRouteTests;
	const FString Writer = LoadSource(
		TEXT("MatchPlayCurrentAttackResolveInitialRouteWriter.cpp"));
	const FString Begin = LoadSource(
		TEXT("MatchPlayCurrentAttackBeginResolutionSessionWriter.cpp"));
	TestEqual(TEXT("Single bHasActualBranch writer"),
		Count(Writer, TEXT("bHasActualBranch = true")), 1);
	TestEqual(TEXT("Single ActualBranch writer"),
		Count(Writer, TEXT("WorkingSession.ActualBranch =")), 1);
	TestEqual(TEXT("Single roll append writer"),
		Count(Writer, TEXT("InitialRouteRollRecords.Add")), 1);
	TestEqual(TEXT("Single RouteResolved transition"),
		Count(Writer,
			TEXT("EMatchPlayCurrentAttackResolutionStage::RouteResolved")),
		1);
	TestTrue(TEXT("Mapping invariant defensive branch exists"),
		Writer.Contains(TEXT("InitialRouteMappingInvariantViolation"))
			&& Writer.Contains(TEXT("NonRetryableInvariantFailure")));
	TestTrue(TEXT("Candidate invariant defensive branch exists"),
		Writer.Contains(TEXT("CandidateInvariantViolation"))
			&& Writer.Contains(TEXT("NonRetryableInvariantFailure")));
	const int32 LegalityIndex = Writer.Find(
		TEXT("ResolveInitialRouteLegalityQuery::Query"));
	const int32 DuplicateIndex = Writer.Find(
		TEXT("bIsCanonicalDuplicate"));
	const int32 ProviderRequirementIndex = Writer.Find(
		TEXT("RollProvider == nullptr"));
	const int32 ProviderCallIndex = Writer.Find(TEXT("RollD6("));
	const int32 MappingIndex = Writer.Find(
		TEXT("InitialRouteMappingQuery::Map"));
	const int32 WorkingStateIndex = Writer.Find(
		TEXT("FMatchPlayState WorkingState = BeforeState"));
	const int32 CandidateIndex = Writer.Find(
		TEXT("ResolutionSessionStateValidator::Validate"));
	TestTrue(TEXT("Writer execution order is frozen"),
		LegalityIndex != INDEX_NONE
			&& LegalityIndex < DuplicateIndex
			&& DuplicateIndex < ProviderRequirementIndex
			&& ProviderRequirementIndex < ProviderCallIndex
			&& ProviderCallIndex < MappingIndex
			&& MappingIndex < WorkingStateIndex
			&& WorkingStateIndex < CandidateIndex);
	TestFalse(TEXT("No pending roll"),
		Writer.Contains(TEXT("PendingRoll")));
	TestFalse(TEXT("Begin remains Route-free"),
		Begin.Contains(TEXT("RouteResolved"))
			|| Begin.Contains(TEXT("InitialRouteRollRecords.Add"))
			|| Begin.Contains(TEXT("bHasActualBranch = true")));
	return true;
}

#undef INITIAL_ROUTE_RESOLVER_TEST

#endif
