#include "MatchPlayCurrentAttackBeginResolutionSessionWriter.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "MatchPlayCurrentAttackResolutionSessionTestFixtures.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "UObject/UnrealType.h"

namespace SessionFixtures =
	FMCodex::Tests::MatchPlayCurrentAttackResolutionSession;
namespace NormalizationFixtures =
	FMCodex::Tests::MatchPlayBoundActionParticipantNormalization;

namespace MatchPlayCurrentAttackResolutionSessionTests
{
	FString LoadProductionSource(const TCHAR* FileName)
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

	bool ExpectWriterFailure(
		FAutomationTestBase& Test,
		const TCHAR* Label,
		const FMatchPlayState& State,
		const FMatchPlayCurrentAttackBeginResolutionSessionRequest&
			Request,
		const EMatchPlayCurrentAttackBeginResolutionSessionErrorCode
			ExpectedError)
	{
		const FMatchPlayState Original = State;
		const auto Result =
			FMatchPlayCurrentAttackBeginResolutionSessionWriter::Begin(
				State,
				Request);
		Test.TestFalse(
			*FString::Printf(TEXT("%s fails"), Label),
			Result.bSuccess);
		Test.TestEqual(
			*FString::Printf(TEXT("%s exact error"), Label),
			Result.ErrorCode,
			ExpectedError);
		Test.TestTrue(
			*FString::Printf(TEXT("%s input remains unchanged"), Label),
			SessionFixtures::AreStatesEqual(State, Original));
		Test.TestTrue(
			*FString::Printf(TEXT("%s BeforeState exact"), Label),
			SessionFixtures::AreStatesEqual(
				Result.BeforeState,
				Original));
		Test.TestTrue(
			*FString::Printf(TEXT("%s AfterState exact"), Label),
			SessionFixtures::AreStatesEqual(
				Result.AfterState,
				Original));
		return !Result.bSuccess
			&& Result.ErrorCode == ExpectedError;
	}

	bool ExpectMalformedDuplicateFailure(
		FAutomationTestBase& Test,
		const TCHAR* Label,
		const FMatchPlayState& State,
		const
			EMatchPlayCurrentAttackResolutionSessionStateValidationErrorCode
				ExpectedValidatorError)
	{
		const FMatchPlayState Original = State;
		const auto Request = SessionFixtures::MakeRequest(State);
		const auto Global =
			FMatchPlayCurrentAttackBeginResolutionSessionGlobalContextQuery
				::Query(State, Request);
		const auto Legality =
			FMatchPlayCurrentAttackBeginResolutionSessionLegalityEvaluator
				::Evaluate(State, Request);
		const auto Writer =
			FMatchPlayCurrentAttackBeginResolutionSessionWriter::Begin(
				State,
				Request);

		Test.TestFalse(
			*FString::Printf(TEXT("%s Global fails"), Label),
			Global.bSuccess);
		Test.TestFalse(
			*FString::Printf(TEXT("%s Global is not duplicate"), Label),
			Global.bSessionAlreadyExists);
		Test.TestEqual(
			*FString::Printf(TEXT("%s Global exact error"), Label),
			Global.ErrorCode,
			EMatchPlayCurrentAttackBeginResolutionSessionErrorCode
				::InvalidExistingSessionState);
		Test.TestEqual(
			*FString::Printf(TEXT("%s Global nested reason"), Label),
			Global.SessionStateValidationResult.ErrorCode,
			ExpectedValidatorError);

		Test.TestFalse(
			*FString::Printf(TEXT("%s Legality fails"), Label),
			Legality.bIsLegal);
		Test.TestFalse(
			*FString::Printf(TEXT("%s Legality is not duplicate"), Label),
			Legality.bSessionAlreadyExists);
		Test.TestEqual(
			*FString::Printf(TEXT("%s Legality exact error"), Label),
			Legality.ErrorCode,
			EMatchPlayCurrentAttackBeginResolutionSessionErrorCode
				::InvalidExistingSessionState);
		Test.TestEqual(
			*FString::Printf(TEXT("%s Legality nested reason"), Label),
			Legality.GlobalContextResult.SessionStateValidationResult
				.ErrorCode,
			ExpectedValidatorError);

		Test.TestFalse(
			*FString::Printf(TEXT("%s Writer fails"), Label),
			Writer.bSuccess);
		Test.TestFalse(
			*FString::Printf(TEXT("%s Writer creates nothing"), Label),
			Writer.bCreatedNewSession);
		Test.TestEqual(
			*FString::Printf(TEXT("%s Writer exact error"), Label),
			Writer.ErrorCode,
			EMatchPlayCurrentAttackBeginResolutionSessionErrorCode
				::InvalidExistingSessionState);
		Test.TestEqual(
			*FString::Printf(TEXT("%s Writer nested reason"), Label),
			Writer.LegalityResult.GlobalContextResult
				.SessionStateValidationResult.ErrorCode,
			ExpectedValidatorError);
		Test.TestTrue(
			*FString::Printf(TEXT("%s input unchanged"), Label),
			SessionFixtures::AreStatesEqual(State, Original));
		Test.TestTrue(
			*FString::Printf(TEXT("%s BeforeState exact"), Label),
			SessionFixtures::AreStatesEqual(
				Writer.BeforeState,
				Original));
		Test.TestTrue(
			*FString::Printf(TEXT("%s AfterState exact"), Label),
			SessionFixtures::AreStatesEqual(
				Writer.AfterState,
				Original));
		return !Global.bSuccess
			&& !Legality.bIsLegal
			&& !Writer.bSuccess
			&& Global.ErrorCode
				== EMatchPlayCurrentAttackBeginResolutionSessionErrorCode
					::InvalidExistingSessionState
			&& Global.SessionStateValidationResult.ErrorCode
				== ExpectedValidatorError;
	}

	FMatchPlayState MakeBegunState(const ESkillRuleType ActionType)
	{
		const FMatchPlayState Ready =
			SessionFixtures::MakeReadyState(ActionType);
		return
			FMatchPlayCurrentAttackBeginResolutionSessionWriter::Begin(
				Ready,
				SessionFixtures::MakeRequest(Ready))
			.AfterState;
	}
}

#define RESOLUTION_SESSION_TEST(TestClass, TestName) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
		TestClass, \
		"FMCodex.CoreRules.MatchPlayCurrentAttackResolutionSession." TestName, \
		EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

RESOLUTION_SESSION_TEST(
	FResolutionSessionDefaultStateTest,
	"01DefaultStateAndRequestSurface")

bool FResolutionSessionDefaultStateTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayCurrentAttackState CurrentAttack;
	TestFalse(TEXT("Default CurrentAttack has no Session"),
		CurrentAttack.bHasResolutionSession);
	TestEqual(TEXT("Default Session sequence"),
		CurrentAttack.ResolutionSession.AttackSequence, int64{0});
	TestEqual(TEXT("Default Session stage"),
		CurrentAttack.ResolutionSession.Stage,
		EMatchPlayCurrentAttackResolutionStage::None);
	const FMatchPlayCurrentAttackResolutionSessionBundle DefaultBundle;
	TestTrue(TEXT("Default Session bundle is empty"),
		FMatchPlayCurrentAttackResolutionSessionBundle::StaticStruct()
			->CompareScriptStruct(
				&CurrentAttack.ResolutionSession.Bundle,
				&DefaultBundle,
				0));

	int32 PropertyCount = 0;
	FName OnlyProperty = NAME_None;
	for (TFieldIterator<FProperty> It(
			FMatchPlayCurrentAttackBeginResolutionSessionRequest
				::StaticStruct());
		It;
		++It)
	{
		++PropertyCount;
		OnlyProperty = It->GetFName();
	}
	TestEqual(TEXT("Request has exactly one reflected field"),
		PropertyCount, 1);
	TestEqual(TEXT("Only Request field is AttackSequence"),
		OnlyProperty, FName(TEXT("AttackSequence")));
	return true;
}

RESOLUTION_SESSION_TEST(
	FResolutionSessionFiveActionBeginTest,
	"02FiveActionsFirstBegin")

bool FResolutionSessionFiveActionBeginTest::RunTest(
	const FString& Parameters)
{
	for (const ESkillRuleType ActionType : {
		ESkillRuleType::LongShot,
		ESkillRuleType::CutInsideShot,
		ESkillRuleType::Cross,
		ESkillRuleType::PassControl,
		ESkillRuleType::ThroughBall})
	{
		const FMatchPlayState Ready =
			SessionFixtures::MakeReadyState(ActionType);
		const auto Normalization =
			FMatchPlayBoundActionParticipantNormalizationQuery::Query(
				Ready,
				NormalizationFixtures::MakeRequest(Ready));
		const auto Result =
			FMatchPlayCurrentAttackBeginResolutionSessionWriter::Begin(
				Ready,
				SessionFixtures::MakeRequest(Ready));
		const auto SessionValidation =
			FMatchPlayCurrentAttackResolutionSessionStateValidator::Validate(
				Result.AfterState);
		const EInitialTurnOrderPlayer RuntimeAttacker =
			Result.AfterState.RuntimeState.CurrentAttackingPlayer;
		const EInitialTurnOrderPlayer RuntimeDefender =
			RuntimeAttacker == EInitialTurnOrderPlayer::PlayerA
				? EInitialTurnOrderPlayer::PlayerB
				: EInitialTurnOrderPlayer::PlayerA;
		const FMatchPlayCurrentAttackActionPreparationState
			DefaultPreparation;
		TestTrue(TEXT("Formal Ready state normalizes"),
			Normalization.bSuccess);
		TestTrue(TEXT("First Begin succeeds"), Result.bSuccess);
		TestTrue(TEXT("First Begin reports created"),
			Result.bCreatedNewSession);
		TestTrue(TEXT("Session presence"),
			Result.AfterState.CurrentAttack.bHasResolutionSession);
		TestEqual(TEXT("Session sequence"),
			Result.Session.AttackSequence,
			Ready.CurrentAttack.AttackSequence);
		TestEqual(TEXT("Session starts AwaitingRoute"),
			Result.Session.Stage,
			EMatchPlayCurrentAttackResolutionStage::AwaitingRoute);
		TestTrue(TEXT("Session matches formal normalization"),
			SessionFixtures::SessionMatchesNormalization(
				Result.Session,
				Normalization));
		TestTrue(TEXT("Published Session is canonical"),
			SessionValidation.bIsCanonical);
		TestEqual(TEXT("Bundle attacker matches runtime authority"),
			Result.Session.Bundle.CurrentAttackingPlayer,
			RuntimeAttacker);
		TestEqual(TEXT("Bundle defender matches runtime authority"),
			Result.Session.Bundle.CurrentDefendingPlayer,
			RuntimeDefender);
		TestTrue(TEXT("SelectedAction presence remains canonical"),
			Result.AfterState.CurrentAttack.bHasSelectedAction);
		TestTrue(TEXT("ActionPreparation remains default"),
			FMatchPlayCurrentAttackActionPreparationState::StaticStruct()
				->CompareScriptStruct(
					&Result.AfterState.CurrentAttack.ActionPreparation,
					&DefaultPreparation,
					0));
		TestTrue(TEXT("Only Session authority changed"),
			SessionFixtures::OnlySessionChanged(
				Ready,
				Result.AfterState));
		TestTrue(TEXT("Published Session matches Result"),
			SessionFixtures::AreSessionsEqual(
				Result.Session,
				Result.AfterState.CurrentAttack.ResolutionSession));
	}
	return true;
}

RESOLUTION_SESSION_TEST(
	FResolutionSessionDuplicateBeginTest,
	"03DuplicateBeginIsIdempotent")

bool FResolutionSessionDuplicateBeginTest::RunTest(
	const FString& Parameters)
{
	for (const ESkillRuleType ActionType : {
		ESkillRuleType::LongShot,
		ESkillRuleType::ThroughBall})
	{
		const FMatchPlayState Ready =
			SessionFixtures::MakeReadyState(ActionType);
		const auto Request = SessionFixtures::MakeRequest(Ready);
		const auto First =
			FMatchPlayCurrentAttackBeginResolutionSessionWriter::Begin(
				Ready,
				Request);
		const auto Second =
			FMatchPlayCurrentAttackBeginResolutionSessionWriter::Begin(
				First.AfterState,
				Request);
		TestTrue(TEXT("First succeeds"), First.bSuccess);
		TestTrue(TEXT("Second succeeds"), Second.bSuccess);
		TestFalse(TEXT("Second does not create"),
			Second.bCreatedNewSession);
		TestTrue(TEXT("Duplicate state is byte-for-value unchanged"),
			SessionFixtures::AreStatesEqual(
				Second.BeforeState,
				Second.AfterState));
		TestTrue(TEXT("Duplicate Session snapshot unchanged"),
			SessionFixtures::AreSessionsEqual(
				First.Session,
				Second.Session));
		TestTrue(TEXT("Duplicate Session validator succeeds"),
			Second.LegalityResult.GlobalContextResult
				.SessionStateValidationResult.bIsCanonical);
		TestTrue(TEXT("Duplicate Global Context bypasses Ready"),
			!Second.LegalityResult.GlobalContextResult
				.ReadyValidationResult.bSuccess);
		TestTrue(TEXT("Duplicate Global Context bypasses Binding"),
			!Second.LegalityResult.GlobalContextResult
				.BindingResult.bSuccess);
		TestTrue(TEXT("Duplicate Global Context bypasses Normalization"),
			!Second.LegalityResult.GlobalContextResult
				.NormalizationResult.bSuccess);
	}
	return true;
}

RESOLUTION_SESSION_TEST(
	FResolutionSessionSequenceFailureTest,
	"04SequenceFailuresAreExactAndAtomic")

bool FResolutionSessionSequenceFailureTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayState Ready =
		SessionFixtures::MakeReadyState(ESkillRuleType::LongShot);
	auto Request = SessionFixtures::MakeRequest(Ready);

	Request.AttackSequence = 0;
	MatchPlayCurrentAttackResolutionSessionTests::ExpectWriterFailure(
		*this,
		TEXT("Invalid request"),
		Ready,
		Request,
		EMatchPlayCurrentAttackBeginResolutionSessionErrorCode
			::InvalidRequestedAttackSequence);

	Request.AttackSequence =
		Ready.CurrentAttack.AttackSequence - 1;
	MatchPlayCurrentAttackResolutionSessionTests::ExpectWriterFailure(
		*this,
		TEXT("Stale request"),
		Ready,
		Request,
		EMatchPlayCurrentAttackBeginResolutionSessionErrorCode
			::AttackSequenceMismatch);

	Request.AttackSequence =
		Ready.CurrentAttack.AttackSequence + 1;
	MatchPlayCurrentAttackResolutionSessionTests::ExpectWriterFailure(
		*this,
		TEXT("Ahead request"),
		Ready,
		Request,
		EMatchPlayCurrentAttackBeginResolutionSessionErrorCode
			::AttackSequenceMismatch);

	const int64 OriginalSequence = Ready.CurrentAttack.AttackSequence;
	Ready.CurrentAttack.AttackSequence = 0;
	Request.AttackSequence = OriginalSequence;
	MatchPlayCurrentAttackResolutionSessionTests::ExpectWriterFailure(
		*this,
		TEXT("Invalid authoritative sequence"),
		Ready,
		Request,
		EMatchPlayCurrentAttackBeginResolutionSessionErrorCode
			::InvalidCurrentAttackSequence);
	return true;
}

RESOLUTION_SESSION_TEST(
	FResolutionSessionLifecycleFailureTest,
	"05LifecycleAndSelectionStageFailures")

bool FResolutionSessionLifecycleFailureTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState Ready =
		SessionFixtures::MakeReadyState(ESkillRuleType::ThroughBall);
	const auto Request = SessionFixtures::MakeRequest(Ready);

	FMatchPlayState NoAttack = Ready;
	NoAttack.bHasCurrentAttack = false;
	NoAttack.CurrentAttack = FMatchPlayCurrentAttackState();
	MatchPlayCurrentAttackResolutionSessionTests::ExpectWriterFailure(
		*this,
		TEXT("No CurrentAttack"),
		NoAttack,
		Request,
		EMatchPlayCurrentAttackBeginResolutionSessionErrorCode
			::NoCurrentAttack);

	FMatchPlayState WrongPhase = Ready;
	WrongPhase.CurrentAttack.Phase =
		EMatchPlayCurrentAttackPhase::Deployment;
	MatchPlayCurrentAttackResolutionSessionTests::ExpectWriterFailure(
		*this,
		TEXT("Wrong outer phase"),
		WrongPhase,
		Request,
		EMatchPlayCurrentAttackBeginResolutionSessionErrorCode
			::CurrentAttackNotInResolution);

	for (const EMatchPlayCurrentAttackSelectionStage Stage : {
		EMatchPlayCurrentAttackSelectionStage::None,
		EMatchPlayCurrentAttackSelectionStage::AwaitingCarrier,
		EMatchPlayCurrentAttackSelectionStage::AwaitingMarker,
		EMatchPlayCurrentAttackSelectionStage::AwaitingSkill,
		EMatchPlayCurrentAttackSelectionStage::AwaitingRunner,
		EMatchPlayCurrentAttackSelectionStage::AwaitingHelper,
		EMatchPlayCurrentAttackSelectionStage::AwaitingBranchIntent})
	{
		FMatchPlayState WrongStage = Ready;
		WrongStage.CurrentAttack.SelectionStage = Stage;
		MatchPlayCurrentAttackResolutionSessionTests::ExpectWriterFailure(
			*this,
			TEXT("Wrong SelectionStage"),
			WrongStage,
			Request,
			EMatchPlayCurrentAttackBeginResolutionSessionErrorCode
				::WrongSelectionStage);
	}
	return true;
}

RESOLUTION_SESSION_TEST(
	FResolutionSessionReadyFailureTest,
	"06ReadyValidationFailure")

bool FResolutionSessionReadyFailureTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayState Damaged =
		SessionFixtures::MakeReadyState(ESkillRuleType::LongShot);
	Damaged.CurrentAttack.SelectedAction.ElectiveBranchIntent =
		EMatchPlayElectiveBranchIntent::CrossHigh;
	return MatchPlayCurrentAttackResolutionSessionTests
		::ExpectWriterFailure(
			*this,
			TEXT("Damaged Ready intent"),
			Damaged,
			SessionFixtures::MakeRequest(Damaged),
			EMatchPlayCurrentAttackBeginResolutionSessionErrorCode
				::ReadyValidationFailed);
}

RESOLUTION_SESSION_TEST(
	FResolutionSessionMalformedExistingTest,
	"07MalformedExistingSessionRejected")

bool FResolutionSessionMalformedExistingTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState Ready =
		SessionFixtures::MakeReadyState(ESkillRuleType::PassControl);
	const auto Request = SessionFixtures::MakeRequest(Ready);
	const FMatchPlayState Begun =
		MatchPlayCurrentAttackResolutionSessionTests::MakeBegunState(
			ESkillRuleType::PassControl);

	TArray<FMatchPlayState> DamagedStates;
	FMatchPlayState AbsentPayload = Ready;
	AbsentPayload.CurrentAttack.ResolutionSession =
		Begun.CurrentAttack.ResolutionSession;
	DamagedStates.Add(AbsentPayload);

	FMatchPlayState StageNone = Begun;
	StageNone.CurrentAttack.ResolutionSession.Stage =
		EMatchPlayCurrentAttackResolutionStage::None;
	DamagedStates.Add(StageNone);

	FMatchPlayState InvalidSequence = Begun;
	InvalidSequence.CurrentAttack.ResolutionSession.AttackSequence = 0;
	DamagedStates.Add(InvalidSequence);

	FMatchPlayState Mismatch = Begun;
	++Mismatch.CurrentAttack.ResolutionSession.AttackSequence;
	DamagedStates.Add(Mismatch);

	FMatchPlayState InvalidBundle = Begun;
	InvalidBundle.CurrentAttack.ResolutionSession.Bundle.Binding.SkillId =
		TEXT("Corrupt.Skill");
	DamagedStates.Add(InvalidBundle);

	for (const FMatchPlayState& Damaged : DamagedStates)
	{
		MatchPlayCurrentAttackResolutionSessionTests::ExpectWriterFailure(
			*this,
			TEXT("Malformed existing Session"),
			Damaged,
			Request,
			EMatchPlayCurrentAttackBeginResolutionSessionErrorCode
				::InvalidExistingSessionState);
	}

	FMatchPlayState MissingSelectedAction = Begun;
	MissingSelectedAction.CurrentAttack.bHasSelectedAction = false;
	MatchPlayCurrentAttackResolutionSessionTests
		::ExpectMalformedDuplicateFailure(
			*this,
			TEXT("Missing SelectedAction presence"),
			MissingSelectedAction,
			EMatchPlayCurrentAttackResolutionSessionStateValidationErrorCode
				::MissingSelectedAction);

	FMatchPlayState NonDefaultPreparation = Begun;
	NonDefaultPreparation.CurrentAttack.ActionPreparation.CarrierCardId =
		TEXT("Corrupt.Preparation.Carrier");
	MatchPlayCurrentAttackResolutionSessionTests
		::ExpectMalformedDuplicateFailure(
			*this,
			TEXT("Non-default ActionPreparation"),
			NonDefaultPreparation,
			EMatchPlayCurrentAttackResolutionSessionStateValidationErrorCode
				::NonDefaultActionPreparation);

	FMatchPlayState RuntimeAttackerMismatch = Begun;
	RuntimeAttackerMismatch.RuntimeState.CurrentAttackingPlayer =
		RuntimeAttackerMismatch.RuntimeState.CurrentAttackingPlayer
			== EInitialTurnOrderPlayer::PlayerA
			? EInitialTurnOrderPlayer::PlayerB
			: EInitialTurnOrderPlayer::PlayerA;
	MatchPlayCurrentAttackResolutionSessionTests
		::ExpectMalformedDuplicateFailure(
			*this,
			TEXT("Runtime attacker mismatch"),
			RuntimeAttackerMismatch,
			EMatchPlayCurrentAttackResolutionSessionStateValidationErrorCode
				::RuntimeAttackerMismatch);

	FMatchPlayState RuntimeDefenderMismatch = Begun;
	RuntimeDefenderMismatch.CurrentAttack.ResolutionSession.Bundle
		.CurrentDefendingPlayer =
			RuntimeDefenderMismatch.CurrentAttack.ResolutionSession.Bundle
				.CurrentAttackingPlayer;
	MatchPlayCurrentAttackResolutionSessionTests
		::ExpectMalformedDuplicateFailure(
			*this,
			TEXT("Runtime defender mismatch"),
			RuntimeDefenderMismatch,
			EMatchPlayCurrentAttackResolutionSessionStateValidationErrorCode
				::RuntimeDefenderMismatch);
	return true;
}

RESOLUTION_SESSION_TEST(
	FResolutionSessionFirstErrorTest,
	"08FirstErrorCombinations")

bool FResolutionSessionFirstErrorTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState Ready =
		SessionFixtures::MakeReadyState(ESkillRuleType::LongShot);
	const auto ValidRequest = SessionFixtures::MakeRequest(Ready);

	FMatchPlayState StaleAndSelectionCorruption = Ready;
	StaleAndSelectionCorruption.CurrentAttack.bHasSelectedAction = false;
	auto StaleRequest = ValidRequest;
	--StaleRequest.AttackSequence;
	MatchPlayCurrentAttackResolutionSessionTests::ExpectWriterFailure(
		*this,
		TEXT("Stale outranks selection authority corruption"),
		StaleAndSelectionCorruption,
		StaleRequest,
		EMatchPlayCurrentAttackBeginResolutionSessionErrorCode
			::AttackSequenceMismatch);

	FMatchPlayState PhaseAndRuntime =
		MatchPlayCurrentAttackResolutionSessionTests::MakeBegunState(
			ESkillRuleType::LongShot);
	PhaseAndRuntime.CurrentAttack.Phase =
		EMatchPlayCurrentAttackPhase::Deployment;
	PhaseAndRuntime.RuntimeState.CurrentAttackingPlayer =
		PhaseAndRuntime.RuntimeState.CurrentAttackingPlayer
			== EInitialTurnOrderPlayer::PlayerA
			? EInitialTurnOrderPlayer::PlayerB
			: EInitialTurnOrderPlayer::PlayerA;
	MatchPlayCurrentAttackResolutionSessionTests::ExpectWriterFailure(
		*this,
		TEXT("Phase outranks runtime authority corruption"),
		PhaseAndRuntime,
		ValidRequest,
		EMatchPlayCurrentAttackBeginResolutionSessionErrorCode
			::CurrentAttackNotInResolution);

	FMatchPlayState SessionAndStage =
		MatchPlayCurrentAttackResolutionSessionTests::MakeBegunState(
			ESkillRuleType::LongShot);
	SessionAndStage.CurrentAttack.ResolutionSession.Stage =
		EMatchPlayCurrentAttackResolutionStage::None;
	SessionAndStage.CurrentAttack.SelectionStage =
		EMatchPlayCurrentAttackSelectionStage::AwaitingSkill;
	MatchPlayCurrentAttackResolutionSessionTests::ExpectWriterFailure(
		*this,
		TEXT("Malformed Session outranks stage"),
		SessionAndStage,
		ValidRequest,
		EMatchPlayCurrentAttackBeginResolutionSessionErrorCode
			::InvalidExistingSessionState);

	FMatchPlayState ReadyAndSnapshot = Ready;
	ReadyAndSnapshot.CurrentAttack.SelectedAction.ElectiveBranchIntent =
		EMatchPlayElectiveBranchIntent::CrossHigh;
	ReadyAndSnapshot.CardSnapshotAuthority
		.PlayerACardSnapshots.Cards.Empty();
	MatchPlayCurrentAttackResolutionSessionTests::ExpectWriterFailure(
		*this,
		TEXT("Ready intent outranks downstream provenance"),
		ReadyAndSnapshot,
		ValidRequest,
		EMatchPlayCurrentAttackBeginResolutionSessionErrorCode
			::ReadyValidationFailed);
	return true;
}

RESOLUTION_SESSION_TEST(
	FResolutionSessionStateValidatorTest,
	"09SingleCanonicalValidator")

bool FResolutionSessionStateValidatorTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState Ready =
		SessionFixtures::MakeReadyState(ESkillRuleType::Cross);
	auto Validation =
		FMatchPlayCurrentAttackResolutionSessionStateValidator::Validate(
			Ready);
	TestTrue(TEXT("Canonical absence accepted"),
		Validation.bIsCanonical);

	FMatchPlayState Begun =
		MatchPlayCurrentAttackResolutionSessionTests::MakeBegunState(
			ESkillRuleType::Cross);
	Validation =
		FMatchPlayCurrentAttackResolutionSessionStateValidator::Validate(
			Begun);
	TestTrue(TEXT("Canonical presence accepted"),
		Validation.bIsCanonical);

	Begun.CurrentAttack.ResolutionSession.Bundle.Carrier.Values.Passing = 0;
	Validation =
		FMatchPlayCurrentAttackResolutionSessionStateValidator::Validate(
			Begun);
	TestFalse(TEXT("Invalid normalized value rejected"),
		Validation.bIsCanonical);
	TestEqual(TEXT("Exact invalid bundle error"),
		Validation.ErrorCode,
		EMatchPlayCurrentAttackResolutionSessionStateValidationErrorCode
			::InvalidSessionBundle);
	return true;
}

RESOLUTION_SESSION_TEST(
	FResolutionSessionGlobalContextDeterminismTest,
	"10GlobalContextDeterminism")

bool FResolutionSessionGlobalContextDeterminismTest::RunTest(
	const FString& Parameters)
{
	TArray<TPair<FMatchPlayState,
		FMatchPlayCurrentAttackBeginResolutionSessionRequest>> Cases;
	TArray<EMatchPlayCurrentAttackBeginResolutionSessionErrorCode>
		ExpectedErrors;
	auto AddCase = [&Cases, &ExpectedErrors](
		const FMatchPlayState& State,
		const FMatchPlayCurrentAttackBeginResolutionSessionRequest& Request,
		const EMatchPlayCurrentAttackBeginResolutionSessionErrorCode
			ExpectedError)
	{
		Cases.Emplace(State, Request);
		ExpectedErrors.Add(ExpectedError);
	};
	const FMatchPlayState Ready =
		SessionFixtures::MakeReadyState(ESkillRuleType::LongShot);
	AddCase(
		Ready,
		SessionFixtures::MakeRequest(Ready),
		EMatchPlayCurrentAttackBeginResolutionSessionErrorCode::None);

	const FMatchPlayState Begun =
		MatchPlayCurrentAttackResolutionSessionTests::MakeBegunState(
			ESkillRuleType::LongShot);
	AddCase(
		Begun,
		SessionFixtures::MakeRequest(Begun),
		EMatchPlayCurrentAttackBeginResolutionSessionErrorCode::None);

	auto StaleRequest = SessionFixtures::MakeRequest(Ready);
	--StaleRequest.AttackSequence;
	AddCase(
		Ready,
		StaleRequest,
		EMatchPlayCurrentAttackBeginResolutionSessionErrorCode
			::AttackSequenceMismatch);

	FMatchPlayState WrongPhase = Ready;
	WrongPhase.CurrentAttack.Phase =
		EMatchPlayCurrentAttackPhase::Deployment;
	AddCase(
		WrongPhase,
		SessionFixtures::MakeRequest(WrongPhase),
		EMatchPlayCurrentAttackBeginResolutionSessionErrorCode
			::CurrentAttackNotInResolution);

	FMatchPlayState ReadyFailure = Ready;
	ReadyFailure.CurrentAttack.SelectedAction.ElectiveBranchIntent =
		EMatchPlayElectiveBranchIntent::CrossHigh;
	AddCase(
		ReadyFailure,
		SessionFixtures::MakeRequest(ReadyFailure),
		EMatchPlayCurrentAttackBeginResolutionSessionErrorCode
			::ReadyValidationFailed);

	FMatchPlayState Malformed = Begun;
	Malformed.CurrentAttack.ResolutionSession.AttackSequence = 0;
	AddCase(
		Malformed,
		SessionFixtures::MakeRequest(Malformed),
		EMatchPlayCurrentAttackBeginResolutionSessionErrorCode
			::InvalidExistingSessionState);

	FMatchPlayState MissingSelectedAction = Begun;
	MissingSelectedAction.CurrentAttack.bHasSelectedAction = false;
	AddCase(
		MissingSelectedAction,
		SessionFixtures::MakeRequest(MissingSelectedAction),
		EMatchPlayCurrentAttackBeginResolutionSessionErrorCode
			::InvalidExistingSessionState);

	FMatchPlayState NonDefaultPreparation = Begun;
	NonDefaultPreparation.CurrentAttack.ActionPreparation.CarrierCardId =
		TEXT("Corrupt.Preparation.Carrier");
	AddCase(
		NonDefaultPreparation,
		SessionFixtures::MakeRequest(NonDefaultPreparation),
		EMatchPlayCurrentAttackBeginResolutionSessionErrorCode
			::InvalidExistingSessionState);

	FMatchPlayState RuntimeAttackerMismatch = Begun;
	RuntimeAttackerMismatch.RuntimeState.CurrentAttackingPlayer =
		RuntimeAttackerMismatch.RuntimeState.CurrentAttackingPlayer
			== EInitialTurnOrderPlayer::PlayerA
			? EInitialTurnOrderPlayer::PlayerB
			: EInitialTurnOrderPlayer::PlayerA;
	AddCase(
		RuntimeAttackerMismatch,
		SessionFixtures::MakeRequest(RuntimeAttackerMismatch),
		EMatchPlayCurrentAttackBeginResolutionSessionErrorCode
			::InvalidExistingSessionState);

	for (int32 CaseIndex = 0; CaseIndex < Cases.Num(); ++CaseIndex)
	{
		const auto& Case = Cases[CaseIndex];
		const FMatchPlayState Original = Case.Key;
		const auto OriginalRequest = Case.Value;
		const auto First =
			FMatchPlayCurrentAttackBeginResolutionSessionGlobalContextQuery
				::Query(Case.Key, Case.Value);
		const auto Second =
			FMatchPlayCurrentAttackBeginResolutionSessionGlobalContextQuery
				::Query(Case.Key, Case.Value);
		const auto Third =
			FMatchPlayCurrentAttackBeginResolutionSessionGlobalContextQuery
				::Query(Case.Key, Case.Value);
		TestTrue(TEXT("First equals second"),
			SessionFixtures::AreGlobalContextResultsEqual(
				First,
				Second));
		TestTrue(TEXT("Second equals third"),
			SessionFixtures::AreGlobalContextResultsEqual(
				Second,
				Third));
		TestTrue(TEXT("Global Context is read-only"),
			SessionFixtures::AreStatesEqual(Case.Key, Original));
		TestTrue(TEXT("Global Context Request is unchanged"),
			FMatchPlayCurrentAttackBeginResolutionSessionRequest
				::StaticStruct()->CompareScriptStruct(
					&Case.Value,
					&OriginalRequest,
					0));
		TestEqual(TEXT("Global Context exact deterministic error"),
			First.ErrorCode,
			ExpectedErrors[CaseIndex]);
	}
	return true;
}

RESOLUTION_SESSION_TEST(
	FResolutionSessionLegalityDeterminismTest,
	"11LegalityDeterminism")

bool FResolutionSessionLegalityDeterminismTest::RunTest(
	const FString& Parameters)
{
	TArray<TPair<FMatchPlayState,
		FMatchPlayCurrentAttackBeginResolutionSessionRequest>> Cases;
	TArray<EMatchPlayCurrentAttackBeginResolutionSessionErrorCode>
		ExpectedErrors;
	auto AddCase = [&Cases, &ExpectedErrors](
		const FMatchPlayState& State,
		const FMatchPlayCurrentAttackBeginResolutionSessionRequest& Request,
		const EMatchPlayCurrentAttackBeginResolutionSessionErrorCode
			ExpectedError)
	{
		Cases.Emplace(State, Request);
		ExpectedErrors.Add(ExpectedError);
	};

	const FMatchPlayState Ready =
		SessionFixtures::MakeReadyState(ESkillRuleType::ThroughBall);
	AddCase(
		Ready,
		SessionFixtures::MakeRequest(Ready),
		EMatchPlayCurrentAttackBeginResolutionSessionErrorCode::None);
	const FMatchPlayState Begun =
		MatchPlayCurrentAttackResolutionSessionTests::MakeBegunState(
			ESkillRuleType::ThroughBall);
	AddCase(
		Begun,
		SessionFixtures::MakeRequest(Begun),
		EMatchPlayCurrentAttackBeginResolutionSessionErrorCode::None);

	FMatchPlayState WrongPhase = Ready;
	WrongPhase.CurrentAttack.Phase =
		EMatchPlayCurrentAttackPhase::Deployment;
	AddCase(
		WrongPhase,
		SessionFixtures::MakeRequest(WrongPhase),
		EMatchPlayCurrentAttackBeginResolutionSessionErrorCode
			::CurrentAttackNotInResolution);

	FMatchPlayState Malformed = Begun;
	Malformed.CurrentAttack.ResolutionSession.Stage =
		EMatchPlayCurrentAttackResolutionStage::None;
	AddCase(
		Malformed,
		SessionFixtures::MakeRequest(Malformed),
		EMatchPlayCurrentAttackBeginResolutionSessionErrorCode
			::InvalidExistingSessionState);

	auto StaleRequest = SessionFixtures::MakeRequest(Ready);
	--StaleRequest.AttackSequence;
	AddCase(
		Ready,
		StaleRequest,
		EMatchPlayCurrentAttackBeginResolutionSessionErrorCode
			::AttackSequenceMismatch);

	FMatchPlayState ReadyFailure = Ready;
	ReadyFailure.CurrentAttack.SelectedAction.ElectiveBranchIntent =
		EMatchPlayElectiveBranchIntent::CrossHigh;
	AddCase(
		ReadyFailure,
		SessionFixtures::MakeRequest(ReadyFailure),
		EMatchPlayCurrentAttackBeginResolutionSessionErrorCode
			::ReadyValidationFailed);

	FMatchPlayState MissingSelectedAction = Begun;
	MissingSelectedAction.CurrentAttack.bHasSelectedAction = false;
	AddCase(
		MissingSelectedAction,
		SessionFixtures::MakeRequest(MissingSelectedAction),
		EMatchPlayCurrentAttackBeginResolutionSessionErrorCode
			::InvalidExistingSessionState);

	FMatchPlayState NonDefaultPreparation = Begun;
	NonDefaultPreparation.CurrentAttack.ActionPreparation.CarrierCardId =
		TEXT("Corrupt.Preparation.Carrier");
	AddCase(
		NonDefaultPreparation,
		SessionFixtures::MakeRequest(NonDefaultPreparation),
		EMatchPlayCurrentAttackBeginResolutionSessionErrorCode
			::InvalidExistingSessionState);

	FMatchPlayState RuntimeAttackerMismatch = Begun;
	RuntimeAttackerMismatch.RuntimeState.CurrentAttackingPlayer =
		RuntimeAttackerMismatch.RuntimeState.CurrentAttackingPlayer
			== EInitialTurnOrderPlayer::PlayerA
			? EInitialTurnOrderPlayer::PlayerB
			: EInitialTurnOrderPlayer::PlayerA;
	AddCase(
		RuntimeAttackerMismatch,
		SessionFixtures::MakeRequest(RuntimeAttackerMismatch),
		EMatchPlayCurrentAttackBeginResolutionSessionErrorCode
			::InvalidExistingSessionState);

	for (int32 CaseIndex = 0; CaseIndex < Cases.Num(); ++CaseIndex)
	{
		const auto& Case = Cases[CaseIndex];
		const FMatchPlayState OriginalState = Case.Key;
		const auto OriginalRequest = Case.Value;
		const auto First =
			FMatchPlayCurrentAttackBeginResolutionSessionLegalityEvaluator
				::Evaluate(Case.Key, Case.Value);
		const auto Second =
			FMatchPlayCurrentAttackBeginResolutionSessionLegalityEvaluator
				::Evaluate(Case.Key, Case.Value);
		const auto Third =
			FMatchPlayCurrentAttackBeginResolutionSessionLegalityEvaluator
				::Evaluate(Case.Key, Case.Value);
		TestTrue(TEXT("Legality first equals second"),
			SessionFixtures::AreLegalityResultsEqual(First, Second));
		TestTrue(TEXT("Legality second equals third"),
			SessionFixtures::AreLegalityResultsEqual(Second, Third));
		TestTrue(TEXT("Legality State is unchanged"),
			SessionFixtures::AreStatesEqual(
				Case.Key,
				OriginalState));
		TestTrue(TEXT("Legality Request is unchanged"),
			FMatchPlayCurrentAttackBeginResolutionSessionRequest
				::StaticStruct()->CompareScriptStruct(
					&Case.Value,
					&OriginalRequest,
					0));
		TestEqual(TEXT("Legality exact deterministic error"),
			First.ErrorCode,
			ExpectedErrors[CaseIndex]);
	}
	return true;
}

RESOLUTION_SESSION_TEST(
	FResolutionSessionWriterAtomicityTest,
	"12WriterFailureMatrixIsAtomic")

bool FResolutionSessionWriterAtomicityTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState Ready =
		SessionFixtures::MakeReadyState(ESkillRuleType::Cross);
	const auto Request = SessionFixtures::MakeRequest(Ready);

	FMatchPlayState Uninitialized = Ready;
	Uninitialized.RuntimeState.bIsInitialized = false;
	MatchPlayCurrentAttackResolutionSessionTests::ExpectWriterFailure(
		*this, TEXT("Uninitialized"), Uninitialized, Request,
		EMatchPlayCurrentAttackBeginResolutionSessionErrorCode
			::MatchPlayStateNotInitialized);

	FMatchPlayState NoAttack = Ready;
	NoAttack.bHasCurrentAttack = false;
	MatchPlayCurrentAttackResolutionSessionTests::ExpectWriterFailure(
		*this, TEXT("No attack"), NoAttack, Request,
		EMatchPlayCurrentAttackBeginResolutionSessionErrorCode
			::NoCurrentAttack);

	auto BadRequest = Request;
	BadRequest.AttackSequence = 0;
	MatchPlayCurrentAttackResolutionSessionTests::ExpectWriterFailure(
		*this, TEXT("Bad sequence"), Ready, BadRequest,
		EMatchPlayCurrentAttackBeginResolutionSessionErrorCode
			::InvalidRequestedAttackSequence);

	FMatchPlayState WrongPhase = Ready;
	WrongPhase.CurrentAttack.Phase =
		EMatchPlayCurrentAttackPhase::Deployment;
	MatchPlayCurrentAttackResolutionSessionTests::ExpectWriterFailure(
		*this, TEXT("Wrong phase"), WrongPhase, Request,
		EMatchPlayCurrentAttackBeginResolutionSessionErrorCode
			::CurrentAttackNotInResolution);

	FMatchPlayState WrongStage = Ready;
	WrongStage.CurrentAttack.SelectionStage =
		EMatchPlayCurrentAttackSelectionStage::AwaitingRunner;
	MatchPlayCurrentAttackResolutionSessionTests::ExpectWriterFailure(
		*this, TEXT("Wrong stage"), WrongStage, Request,
		EMatchPlayCurrentAttackBeginResolutionSessionErrorCode
			::WrongSelectionStage);

	FMatchPlayState ReadyFailure = Ready;
	ReadyFailure.CurrentAttack.SelectedAction.ElectiveBranchIntent =
		EMatchPlayElectiveBranchIntent::DirectShot;
	MatchPlayCurrentAttackResolutionSessionTests::ExpectWriterFailure(
		*this, TEXT("Ready failure"), ReadyFailure, Request,
		EMatchPlayCurrentAttackBeginResolutionSessionErrorCode
			::ReadyValidationFailed);

	FMatchPlayState Malformed =
		MatchPlayCurrentAttackResolutionSessionTests::MakeBegunState(
			ESkillRuleType::Cross);
	Malformed.CurrentAttack.ResolutionSession.Bundle.Binding.SkillId =
		TEXT("Malformed");
	MatchPlayCurrentAttackResolutionSessionTests::ExpectWriterFailure(
		*this, TEXT("Malformed Session"), Malformed, Request,
		EMatchPlayCurrentAttackBeginResolutionSessionErrorCode
			::InvalidExistingSessionState);
	return true;
}

RESOLUTION_SESSION_TEST(
	FResolutionSessionBundleValueCopyTest,
	"13BundleIsAuthorityValueCopy")

bool FResolutionSessionBundleValueCopyTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayState Ready =
		SessionFixtures::MakeReadyState(
			ESkillRuleType::ThroughBall,
			true);
	FPlayerCardRuleSnapshot* Carrier =
		NormalizationFixtures::FindSnapshot(
			Ready,
			EInitialTurnOrderPlayer::PlayerA,
			Ready.CurrentAttack.SelectedAction.CarrierCardId);
	TestNotNull(TEXT("Carrier snapshot exists"), Carrier);
	if (Carrier == nullptr)
	{
		return false;
	}
	NormalizationFixtures::SetDistinctValues(
		Carrier->Attributes,
		2);
	const auto Result =
		FMatchPlayCurrentAttackBeginResolutionSessionWriter::Begin(
			Ready,
			SessionFixtures::MakeRequest(Ready));
	TestTrue(TEXT("Begin succeeds"), Result.bSuccess);
	const int32 StoredPassing =
		Result.Session.Bundle.Carrier.Values.Passing;
	Carrier->Attributes.Passing =
		Carrier->Attributes.Passing == 6 ? 5 : 6;
	TestEqual(TEXT("Stored bundle remains a value copy"),
		Result.Session.Bundle.Carrier.Values.Passing,
		StoredPassing);
	TestTrue(TEXT("Binding provenance retained"),
		Result.Session.Bundle.Binding.AttackSequence > 0);
	TestTrue(TEXT("All participant payloads retained"),
		Result.Session.Bundle.Carrier.bIsPresent
			&& Result.Session.Bundle.Marker.bIsPresent
			&& Result.Session.Bundle.Runner.bIsPresent
			&& Result.Session.Bundle.Helper.bIsPresent);
	return true;
}

RESOLUTION_SESSION_TEST(
	FResolutionSessionStaticAuthorityTest,
	"14SinglePublicSurfaceAndWriter")

bool FResolutionSessionStaticAuthorityTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayCurrentAttackResolutionSessionTests;
	const FString GlobalHeader = LoadProductionSource(
		TEXT("MatchPlayCurrentAttackBeginResolutionSessionGlobalContextQuery.h"));
	const FString LegalityHeader = LoadProductionSource(
		TEXT("MatchPlayCurrentAttackBeginResolutionSessionLegality.h"));
	const FString WriterHeader = LoadProductionSource(
		TEXT("MatchPlayCurrentAttackBeginResolutionSessionWriter.h"));
	const FString GlobalSource = LoadProductionSource(
		TEXT("MatchPlayCurrentAttackBeginResolutionSessionGlobalContextQuery.cpp"));
	const FString WriterSource = LoadProductionSource(
		TEXT("MatchPlayCurrentAttackBeginResolutionSessionWriter.cpp"));
	TestEqual(TEXT("One Global Context Query"),
		Count(GlobalHeader,
			TEXT("FMatchPlayCurrentAttackBeginResolutionSessionGlobalContextResult")),
		1);
	TestEqual(TEXT("One Legality Evaluate"),
		Count(LegalityHeader, TEXT("Evaluate(")), 1);
	TestEqual(TEXT("One Writer Begin"),
		Count(WriterHeader, TEXT("Begin(")), 1);
	TestEqual(TEXT("One production presence write"),
		Count(WriterSource,
			TEXT("bHasResolutionSession = true")), 1);
	TestEqual(TEXT("One production Session payload write"),
		Count(WriterSource,
			TEXT("CurrentAttack.ResolutionSession =")), 1);
	TestEqual(TEXT("Global Context never writes Session presence"),
		Count(GlobalSource,
			TEXT("bHasResolutionSession = true")), 0);
	TestEqual(TEXT("Global Context never writes Session payload"),
		Count(GlobalSource,
			TEXT("CurrentAttack.ResolutionSession =")), 0);

	const FString AvailabilityPath = FPaths::Combine(
		FPaths::ProjectDir(),
		TEXT("Source/FMCodex/CoreRules"),
		TEXT("MatchPlayCurrentAttackBeginResolutionSessionAvailability.h"));
	TestFalse(TEXT("No Begin Availability"),
		FPaths::FileExists(AvailabilityPath));
	return true;
}

RESOLUTION_SESSION_TEST(
	FResolutionSessionForbiddenBoundaryTest,
	"15NoRouteRngOutcomeCompletionOrExternalApi")

bool FResolutionSessionForbiddenBoundaryTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayCurrentAttackResolutionSessionTests;
	const FString Production =
		LoadProductionSource(
			TEXT("MatchPlayCurrentAttackBeginResolutionSessionGlobalContextQuery.cpp"))
		+ LoadProductionSource(
			TEXT("MatchPlayCurrentAttackBeginResolutionSessionLegality.cpp"))
		+ LoadProductionSource(
			TEXT("MatchPlayCurrentAttackBeginResolutionSessionWriter.cpp"));
	for (const TCHAR* Token : {
		TEXT("FMath::Rand"),
		TEXT("FRandomStream"),
		TEXT("D6"),
		TEXT("ActualBranch"),
		TEXT("RouteResolver"),
		TEXT("FormulaResolver"),
		TEXT("Outcome"),
		TEXT("FMatchPlayCurrentAttackCompletion")})
	{
		TestEqual(
			*FString::Printf(TEXT("Forbidden production token absent: %s"),
				Token),
			Count(Production, Token),
			0);
	}

	const FString External =
		LoadProductionSource(TEXT("MatchPlayExternalStateView.h"))
		+ LoadProductionSource(TEXT("MatchPlayExternalStateView.cpp"))
		+ LoadProductionSource(TEXT("MatchPlaySubmissionGate.h"))
		+ LoadProductionSource(TEXT("MatchPlaySubmissionGate.cpp"));
	TestEqual(TEXT("No external Begin API"),
		Count(External, TEXT("BeginResolutionSession")), 0);
	return true;
}

RESOLUTION_SESSION_TEST(
	FResolutionSessionPipelineOrderAuditTest,
	"16PipelineOrderAndDownstreamFailureMapping")

bool FResolutionSessionPipelineOrderAuditTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayCurrentAttackResolutionSessionTests;
	const FString Source = LoadProductionSource(
		TEXT("MatchPlayCurrentAttackBeginResolutionSessionGlobalContextQuery.cpp"));
	const int32 ExistingSessionPosition = Source.Find(
		TEXT("if (CurrentAttack.bHasResolutionSession)"));
	const int32 ReadyPosition = Source.Find(
		TEXT("FMatchPlayCurrentAttackReadyForResolutionValidator::Validate"));
	const int32 BindingPosition = Source.Find(
		TEXT("FMatchPlayValidatedResolutionPreparationAccess::PopulateBinding"));
	const int32 BindingFailurePosition = Source.Find(
		TEXT("::BindingFailed"));
	const int32 NormalizationPosition = Source.Find(
		TEXT("FMatchPlayValidatedResolutionPreparationAccess::Normalize"));
	const int32 NormalizationFailurePosition = Source.Find(
		TEXT("::NormalizationFailed"));
	const int32 BuildSessionPosition = Source.Find(
		TEXT("Result.Session = BuildSession"));
	const int32 CanonicalValidationPosition = Source.Find(
		TEXT("&Result.Session"));

	TestTrue(TEXT("All audited pipeline symbols are present"),
		ExistingSessionPosition != INDEX_NONE
			&& ReadyPosition != INDEX_NONE
			&& BindingPosition != INDEX_NONE
			&& BindingFailurePosition != INDEX_NONE
			&& NormalizationPosition != INDEX_NONE
			&& NormalizationFailurePosition != INDEX_NONE
			&& BuildSessionPosition != INDEX_NONE
			&& CanonicalValidationPosition != INDEX_NONE);
	TestTrue(TEXT("Duplicate handling precedes Ready validation"),
		ExistingSessionPosition < ReadyPosition);
	TestTrue(TEXT("Ready precedes Binding"),
		ReadyPosition < BindingPosition);
	TestTrue(TEXT("Binding failure is mapped before Normalization"),
		BindingPosition < BindingFailurePosition
			&& BindingFailurePosition < NormalizationPosition);
	TestTrue(TEXT("Normalization failure is mapped before bundle build"),
		NormalizationPosition < NormalizationFailurePosition
			&& NormalizationFailurePosition < BuildSessionPosition);
	TestTrue(TEXT("Bundle is built before candidate validation"),
		BuildSessionPosition < CanonicalValidationPosition);
	return true;
}

#undef RESOLUTION_SESSION_TEST

#endif
