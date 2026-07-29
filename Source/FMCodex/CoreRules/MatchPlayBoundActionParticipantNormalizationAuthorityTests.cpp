#include "MatchPlayBoundActionParticipantNormalizationQuery.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "MatchPlayBoundActionParticipantNormalizationTestFixtures.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace NormalizationAuthorityFixtures =
	FMCodex::Tests::MatchPlayBoundActionParticipantNormalization;

namespace BoundActionNormalizationAuthorityTests
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
}

#define NORMALIZATION_AUTHORITY_TEST(TestClass, TestName) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
		TestClass, \
		"FMCodex.CoreRules.MatchPlayBoundActionParticipantNormalization.Authority." TestName, \
		EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

NORMALIZATION_AUTHORITY_TEST(
	FBoundActionNormalizationStaticAuthorityTest,
	"SingleEntryBindingAndIsolation")

bool FBoundActionNormalizationStaticAuthorityTest::RunTest(
	const FString& Parameters)
{
	using namespace BoundActionNormalizationAuthorityTests;
	const FString Header = LoadProductionSource(
		TEXT("MatchPlayBoundActionParticipantNormalizationQuery.h"));
	const FString Source = LoadProductionSource(
		TEXT("MatchPlayBoundActionParticipantNormalizationQuery.cpp"));
	TestEqual(TEXT("One public Query declaration"),
		Count(Header, TEXT("static FMatchPlayBoundActionParticipantNormalizationResult Query(")),
		1);
	TestEqual(TEXT("Binding called exactly once"),
		Count(Source,
			TEXT("FMatchPlayCurrentAttackResolutionBinding::Query(")),
		1);
	TestEqual(TEXT("Ready validator not called directly"),
		Count(Source,
			TEXT("FMatchPlayCurrentAttackReadyForResolutionValidator::Validate")),
		0);
	TestEqual(TEXT("Selection validator not called directly"),
		Count(Source,
			TEXT("FMatchPlayCurrentAttackSelectionStateValidator::Validate")),
		0);
	TestEqual(TEXT("No direct SelectedAction reads"),
		Count(Source, TEXT("SelectedAction")), 0);
	TestEqual(TEXT("No direct ActionPreparation reads"),
		Count(Source, TEXT("ActionPreparation")), 0);
	TestEqual(TEXT("No second snapshot authority query"),
		Count(Source, TEXT("FindByPlayerSideAndCardId")), 0);
	TestEqual(TEXT("Carrier Ready query result reused once"),
		Count(Source, TEXT("CarrierSnapshotQueryResult;")), 1);
	TestEqual(TEXT("Marker Ready query result reused once"),
		Count(Source, TEXT("MarkerSnapshotQueryResult;")), 1);
	TestEqual(TEXT("Runner Ready query result reused conditionally once"),
		Count(Source, TEXT("RunnerSnapshotQueryResult;")), 1);
	TestEqual(TEXT("Helper Ready query result reused conditionally once"),
		Count(Source, TEXT("HelperAuthorityResult.SnapshotQueryResult;")), 1);
	return true;
}

NORMALIZATION_AUTHORITY_TEST(
	FBoundActionNormalizationForbiddenDependencyTest,
	"NoMutationFormulaRandomOrOldFlow")

bool FBoundActionNormalizationForbiddenDependencyTest::RunTest(
	const FString& Parameters)
{
	using namespace BoundActionNormalizationAuthorityTests;
	const FString Header = LoadProductionSource(
		TEXT("MatchPlayBoundActionParticipantNormalizationQuery.h"));
	const FString Source = LoadProductionSource(
		TEXT("MatchPlayBoundActionParticipantNormalizationQuery.cpp"));
	const FString Production = Header + Source;
	const TCHAR* ForbiddenTokens[] = {
		TEXT("Goalkeeper"),
		TEXT("ActionPoint"),
		TEXT("D6"),
		TEXT("Random"),
		TEXT("BaseValue"),
		TEXT("FormulaResolver"),
		TEXT("Outcome"),
		TEXT("Completion"),
		TEXT("Score"),
		TEXT("CardUsage"),
		TEXT("Opportunity"),
		TEXT("MatchPlayAttackFlow"),
		TEXT("FormulaAttackFlow"),
		TEXT("AttackResolutionFlow"),
		TEXT("BeginOrdinaryAttack"),
		TEXT("ResolutionSession"),
		TEXT("Capability")
	};
	for (const TCHAR* Token : ForbiddenTokens)
	{
		TestEqual(
			*FString::Printf(TEXT("Forbidden token absent: %s"), Token),
			Count(Production, Token),
			0);
	}
	return true;
}

NORMALIZATION_AUTHORITY_TEST(
	FBoundActionNormalizationCrossSideSameIdTest,
	"SameCardIdAcrossSides")

bool FBoundActionNormalizationCrossSideSameIdTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayState State =
		NormalizationAuthorityFixtures::MakeReadyState(
			ESkillRuleType::PassControl,
			false);
	const FName SharedId(TEXT("Normalization.Shared"));
	const FName OriginalCarrier =
		State.CurrentAttack.SelectedAction.CarrierCardId;
	const FName OriginalMarker =
		State.CurrentAttack.SelectedAction.MarkerCardId;
	State.CurrentAttack.SelectedAction.CarrierCardId = SharedId;
	State.CurrentAttack.SelectedAction.MarkerCardId = SharedId;
	for (FMatchPlayDeploymentPlacement& Placement :
		State.CurrentAttack.DeploymentPlacements)
	{
		if (Placement.CardId == OriginalCarrier
			|| Placement.CardId == OriginalMarker)
		{
			Placement.CardId = SharedId;
		}
	}
	FPlayerCardRuleSnapshot* Carrier =
		NormalizationAuthorityFixtures::FindSnapshot(
			State,
			EInitialTurnOrderPlayer::PlayerA,
			OriginalCarrier);
	FPlayerCardRuleSnapshot* Marker =
		NormalizationAuthorityFixtures::FindSnapshot(
			State,
			EInitialTurnOrderPlayer::PlayerB,
			OriginalMarker);
	TestNotNull(TEXT("Carrier source exists"), Carrier);
	TestNotNull(TEXT("Marker source exists"), Marker);
	if (Carrier == nullptr || Marker == nullptr)
	{
		return false;
	}
	Carrier->CardId = SharedId;
	Marker->CardId = SharedId;
	Carrier->Attributes.Passing = 6;
	Marker->Attributes.Passing = 2;

	const auto Result =
		FMatchPlayBoundActionParticipantNormalizationQuery::Query(
			State,
			NormalizationAuthorityFixtures::MakeRequest(State));
	TestTrue(TEXT("Cross-side same CardId succeeds"), Result.bSuccess);
	TestEqual(TEXT("Carrier reads attacker value"),
		Result.Bundle.Carrier.Values.Passing, 6);
	TestEqual(TEXT("Marker reads defender value"),
		Result.Bundle.Marker.Values.Passing, 2);
	TestEqual(TEXT("Carrier side"), Result.Bundle.Carrier.Side,
		EInitialTurnOrderPlayer::PlayerA);
	TestEqual(TEXT("Marker side"), Result.Bundle.Marker.Side,
		EInitialTurnOrderPlayer::PlayerB);
	return true;
}

NORMALIZATION_AUTHORITY_TEST(
	FBoundActionNormalizationMissingCarrierTest,
	"BindingOwnsMissingCarrierSnapshotFirstError")

bool FBoundActionNormalizationMissingCarrierTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayState State =
		NormalizationAuthorityFixtures::MakeReadyState(
			ESkillRuleType::LongShot);
	State.CardSnapshotAuthority.PlayerACardSnapshots.Cards.Empty();
	const FMatchPlayState Original = State;
	const auto Result =
		FMatchPlayBoundActionParticipantNormalizationQuery::Query(
			State,
			NormalizationAuthorityFixtures::MakeRequest(State));
	TestFalse(TEXT("Missing Carrier fails"), Result.bSuccess);
	TestEqual(TEXT("Binding is first error"), Result.ErrorCode,
		EMatchPlayBoundActionParticipantNormalizationErrorCode
			::BindingFailed);
	TestEqual(TEXT("Ready preserves concrete Carrier failure"),
		Result.BindingResult.ReadyValidationResult.ErrorCode,
		EMatchPlayCurrentAttackReadyValidationErrorCode
			::CarrierSnapshotQueryFailed);
	TestFalse(TEXT("Normalization bundle query not attempted"),
		Result.Bundle.Carrier.bSnapshotQueryAttempted);
	TestTrue(TEXT("State unchanged"),
		NormalizationAuthorityFixtures::AreStatesEqual(State, Original));
	return true;
}

NORMALIZATION_AUTHORITY_TEST(
	FBoundActionNormalizationMissingMarkerTest,
	"BindingOwnsMissingMarkerSnapshotFirstError")

bool FBoundActionNormalizationMissingMarkerTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayState State =
		NormalizationAuthorityFixtures::MakeReadyState(
			ESkillRuleType::LongShot);
	State.CardSnapshotAuthority.PlayerBCardSnapshots.Cards.Empty();
	const auto Result =
		FMatchPlayBoundActionParticipantNormalizationQuery::Query(
			State,
			NormalizationAuthorityFixtures::MakeRequest(State));
	TestEqual(TEXT("Binding is first error"), Result.ErrorCode,
		EMatchPlayBoundActionParticipantNormalizationErrorCode
			::BindingFailed);
	TestEqual(TEXT("Ready preserves concrete Marker failure"),
		Result.BindingResult.ReadyValidationResult.ErrorCode,
		EMatchPlayCurrentAttackReadyValidationErrorCode
			::MarkerSnapshotQueryFailed);
	TestFalse(TEXT("Carrier bundle remains unconstructed"),
		Result.Bundle.Carrier.bSnapshotQueryAttempted);
	return true;
}

NORMALIZATION_AUTHORITY_TEST(
	FBoundActionNormalizationMissingRunnerTest,
	"BindingOwnsMissingRunnerSnapshotFirstError")

bool FBoundActionNormalizationMissingRunnerTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayState State =
		NormalizationAuthorityFixtures::MakeReadyState(
			ESkillRuleType::Cross,
			false);
	State.CardSnapshotAuthority.PlayerACardSnapshots.Cards.RemoveAll(
		[](const FPlayerCardRuleSnapshot& Snapshot)
		{
			return Snapshot.CardId
				== NormalizationAuthorityFixtures::HelperFixtures::RunnerId;
		});
	const auto Result =
		FMatchPlayBoundActionParticipantNormalizationQuery::Query(
			State,
			NormalizationAuthorityFixtures::MakeRequest(State));
	TestEqual(TEXT("Binding is first error"), Result.ErrorCode,
		EMatchPlayBoundActionParticipantNormalizationErrorCode
			::BindingFailed);
	TestEqual(TEXT("Ready preserves concrete Runner failure"),
		Result.BindingResult.ReadyValidationResult.ErrorCode,
		EMatchPlayCurrentAttackReadyValidationErrorCode
			::RunnerSnapshotQueryFailed);
	TestFalse(TEXT("Normalization Runner not attempted"),
		Result.Bundle.Runner.bSnapshotQueryAttempted);
	return true;
}

NORMALIZATION_AUTHORITY_TEST(
	FBoundActionNormalizationMissingHelperTest,
	"BindingOwnsMissingPresentHelperSnapshotFirstError")

bool FBoundActionNormalizationMissingHelperTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayState State =
		NormalizationAuthorityFixtures::MakeReadyState(
			ESkillRuleType::ThroughBall,
			true);
	State.CardSnapshotAuthority.PlayerBCardSnapshots.Cards.RemoveAll(
		[](const FPlayerCardRuleSnapshot& Snapshot)
		{
			return Snapshot.CardId
				== NormalizationAuthorityFixtures::HelperFixtures::HelperId;
		});
	const auto Result =
		FMatchPlayBoundActionParticipantNormalizationQuery::Query(
			State,
			NormalizationAuthorityFixtures::MakeRequest(State));
	TestEqual(TEXT("Binding is first error"), Result.ErrorCode,
		EMatchPlayBoundActionParticipantNormalizationErrorCode
			::BindingFailed);
	TestEqual(TEXT("Ready preserves Helper authority failure"),
		Result.BindingResult.ReadyValidationResult.ErrorCode,
		EMatchPlayCurrentAttackReadyValidationErrorCode
			::HelperAuthorityFailed);
	TestFalse(TEXT("Normalization Helper not attempted"),
		Result.Bundle.Helper.bSnapshotQueryAttempted);
	return true;
}

NORMALIZATION_AUTHORITY_TEST(
	FBoundActionNormalizationCorruptPresenceTest,
	"BindingRejectsCorruptRunnerAndHelperPresence")

bool FBoundActionNormalizationCorruptPresenceTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayState NoRunner =
		NormalizationAuthorityFixtures::MakeReadyState(
			ESkillRuleType::LongShot);
	NoRunner.CurrentAttack.SelectedAction.RunnerCardId =
		NormalizationAuthorityFixtures::HelperFixtures::RunnerId;
	const auto UnexpectedRunner =
		FMatchPlayBoundActionParticipantNormalizationQuery::Query(
			NoRunner,
			NormalizationAuthorityFixtures::MakeRequest(NoRunner));
	TestEqual(TEXT("Unexpected Runner rejected by Binding first"),
		UnexpectedRunner.ErrorCode,
		EMatchPlayBoundActionParticipantNormalizationErrorCode
			::BindingFailed);

	FMatchPlayState InvalidHelper =
		NormalizationAuthorityFixtures::MakeReadyState(
			ESkillRuleType::PassControl,
			false);
	InvalidHelper.CurrentAttack.SelectedAction.HelperCardId =
		NormalizationAuthorityFixtures::HelperFixtures::HelperId;
	const auto UnexpectedHelper =
		FMatchPlayBoundActionParticipantNormalizationQuery::Query(
			InvalidHelper,
			NormalizationAuthorityFixtures::MakeRequest(InvalidHelper));
	TestEqual(TEXT("Invalid Helper presence rejected by Binding first"),
		UnexpectedHelper.ErrorCode,
		EMatchPlayBoundActionParticipantNormalizationErrorCode
			::BindingFailed);
	return true;
}

#undef NORMALIZATION_AUTHORITY_TEST

#endif
