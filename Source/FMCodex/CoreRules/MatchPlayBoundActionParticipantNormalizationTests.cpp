#include "MatchPlayBoundActionParticipantNormalizationQuery.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "MatchPlayBoundActionParticipantNormalizationTestFixtures.h"
#include "Misc/AutomationTest.h"
#include "UObject/UnrealType.h"

#include <type_traits>

namespace NormalizationFixtures =
	FMCodex::Tests::MatchPlayBoundActionParticipantNormalization;

#define NORMALIZATION_TEST(TestClass, TestName) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
		TestClass, \
		"FMCodex.CoreRules.MatchPlayBoundActionParticipantNormalization." TestName, \
		EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

NORMALIZATION_TEST(
	FBoundActionNormalizationContractTest,
	"Contract.ReflectionDefaultsAndSignature")

bool FBoundActionNormalizationContractTest::RunTest(
	const FString& Parameters)
{
	const UScriptStruct* Request =
		FMatchPlayBoundActionParticipantNormalizationRequest::StaticStruct();
	int32 RequestFieldCount = 0;
	TArray<FName> RequestFieldNames;
	for (TFieldIterator<FProperty> It(Request); It; ++It)
	{
		++RequestFieldCount;
		RequestFieldNames.Add(It->GetFName());
	}
	TestEqual(TEXT("Request has exactly one reflected field"),
		RequestFieldCount, 1);
	TestEqual(TEXT("Only field is AttackSequence"),
		RequestFieldNames[0], FName(TEXT("AttackSequence")));
	TestNotNull(TEXT("Result is reflected"),
		FMatchPlayBoundActionParticipantNormalizationResult::StaticStruct());
	TestNotNull(TEXT("Bundle is reflected"),
		FMatchPlayBoundActionNormalizedParticipantBundle::StaticStruct());
	TestNotNull(TEXT("Participant is reflected"),
		FMatchPlayBoundActionNormalizedParticipant::StaticStruct());
	TestNotNull(TEXT("Values are reflected"),
		FMatchPlayBoundActionNormalizedParticipantValues::StaticStruct());

	using FExpectedSignature =
		FMatchPlayBoundActionParticipantNormalizationResult (*)(
			const FMatchPlayState&,
			const FMatchPlayBoundActionParticipantNormalizationRequest&);
	TestTrue(TEXT("Query signature is exact"),
		(std::is_same_v<
			decltype(
				&FMatchPlayBoundActionParticipantNormalizationQuery::Query),
			FExpectedSignature>));

	const FMatchPlayBoundActionNormalizedParticipantValues Values;
	TestTrue(TEXT("Default values are all zero"),
		NormalizationFixtures::ValuesAreZero(Values));
	return true;
}

NORMALIZATION_TEST(
	FBoundActionNormalizationAllActionsTest,
	"Success.AllFiveActionTypes")

bool FBoundActionNormalizationAllActionsTest::RunTest(
	const FString& Parameters)
{
	const ESkillRuleType Types[] = {
		ESkillRuleType::LongShot,
		ESkillRuleType::CutInsideShot,
		ESkillRuleType::PassControl,
		ESkillRuleType::Cross,
		ESkillRuleType::ThroughBall
	};
	for (const ESkillRuleType Type : Types)
	{
		const FMatchPlayState State =
			NormalizationFixtures::MakeReadyState(Type, true);
		const auto Result =
			FMatchPlayBoundActionParticipantNormalizationQuery::Query(
				State,
				NormalizationFixtures::MakeRequest(State));
		TestTrue(TEXT("Action normalization succeeds"), Result.bSuccess);
		TestEqual(TEXT("Action type preserved"), Result.ActionType, Type);
		TestEqual(TEXT("Bundle sequence preserved"),
			Result.Bundle.AttackSequence,
			State.CurrentAttack.AttackSequence);
		TestEqual(TEXT("Binding provenance preserved"),
			Result.Bundle.Binding.ActionType, Type);
		TestTrue(TEXT("Carrier present"), Result.Bundle.Carrier.bIsPresent);
		TestTrue(TEXT("Marker present"), Result.Bundle.Marker.bIsPresent);
	}
	return true;
}

NORMALIZATION_TEST(
	FBoundActionNormalizationNoRunnerTest,
	"Presence.NoRunnerActionsUseFormalAbsence")

bool FBoundActionNormalizationNoRunnerTest::RunTest(
	const FString& Parameters)
{
	const ESkillRuleType Types[] = {
		ESkillRuleType::LongShot,
		ESkillRuleType::CutInsideShot
	};
	for (const ESkillRuleType Type : Types)
	{
		const FMatchPlayState State =
			NormalizationFixtures::MakeReadyState(Type);
		const auto Result =
			FMatchPlayBoundActionParticipantNormalizationQuery::Query(
				State,
				NormalizationFixtures::MakeRequest(State));
		TestTrue(TEXT("No-runner action succeeds"), Result.bSuccess);
		TestFalse(TEXT("Runner absent"), Result.Bundle.bHasRunner);
		TestFalse(TEXT("Runner is not present"),
			Result.Bundle.Runner.bIsPresent);
		TestFalse(TEXT("Runner query not attempted"),
			Result.Bundle.Runner.bSnapshotQueryAttempted);
		TestTrue(TEXT("Runner CardId remains None"),
			Result.Bundle.Runner.CardId.IsNone());
		TestTrue(TEXT("Runner values stay zero"),
			NormalizationFixtures::ValuesAreZero(
				Result.Bundle.Runner.Values));
		TestFalse(TEXT("Helper absent"), Result.Bundle.bHasHelper);
		TestFalse(TEXT("Helper query not attempted"),
			Result.Bundle.Helper.bSnapshotQueryAttempted);
	}
	return true;
}

NORMALIZATION_TEST(
	FBoundActionNormalizationRunnerActionsTest,
	"Presence.RunnerRequiredForThreeActions")

bool FBoundActionNormalizationRunnerActionsTest::RunTest(
	const FString& Parameters)
{
	const ESkillRuleType Types[] = {
		ESkillRuleType::PassControl,
		ESkillRuleType::Cross,
		ESkillRuleType::ThroughBall
	};
	for (const ESkillRuleType Type : Types)
	{
		const FMatchPlayState State =
			NormalizationFixtures::MakeReadyState(Type, false);
		const auto Result =
			FMatchPlayBoundActionParticipantNormalizationQuery::Query(
				State,
				NormalizationFixtures::MakeRequest(State));
		TestTrue(TEXT("Runner action succeeds"), Result.bSuccess);
		TestTrue(TEXT("Runner presence set"), Result.Bundle.bHasRunner);
		TestTrue(TEXT("Runner is present"),
			Result.Bundle.Runner.bIsPresent);
		TestTrue(TEXT("Runner query attempted"),
			Result.Bundle.Runner.bSnapshotQueryAttempted);
		TestTrue(TEXT("Runner query succeeded"),
			Result.Bundle.Runner.bSnapshotQuerySucceeded);
		TestEqual(TEXT("Runner side is attacker"),
			Result.Bundle.Runner.Side,
			Result.CurrentAttackingPlayer);
	}
	return true;
}

NORMALIZATION_TEST(
	FBoundActionNormalizationHelperPresentTest,
	"Presence.HelperPresentUsesAuthoritativeValues")

bool FBoundActionNormalizationHelperPresentTest::RunTest(
	const FString& Parameters)
{
	const ESkillRuleType Types[] = {
		ESkillRuleType::PassControl,
		ESkillRuleType::Cross,
		ESkillRuleType::ThroughBall
	};
	for (const ESkillRuleType Type : Types)
	{
		FMatchPlayState State =
			NormalizationFixtures::MakeReadyState(Type, true);
		const EInitialTurnOrderPlayer Defender =
			NormalizationFixtures::HelperFixtures::GetDefender(
				State.RuntimeState.CurrentAttackingPlayer);
		FPlayerCardRuleSnapshot* Helper =
			NormalizationFixtures::FindSnapshot(
				State,
				Defender,
				NormalizationFixtures::HelperFixtures::HelperId);
		TestNotNull(TEXT("Helper source exists"), Helper);
		if (Helper == nullptr)
		{
			continue;
		}
		NormalizationFixtures::SetDistinctValues(Helper->Attributes, 2);

		const auto Result =
			FMatchPlayBoundActionParticipantNormalizationQuery::Query(
				State,
				NormalizationFixtures::MakeRequest(State));
		TestTrue(TEXT("Helper-present normalization succeeds"),
			Result.bSuccess);
		TestTrue(TEXT("Helper presence preserved"),
			Result.Bundle.bHasHelper);
		TestTrue(TEXT("Helper participant present"),
			Result.Bundle.Helper.bIsPresent);
		TestTrue(TEXT("Helper query attempted"),
			Result.Bundle.Helper.bSnapshotQueryAttempted);
		TestTrue(TEXT("Helper values copied exactly"),
			NormalizationFixtures::ValuesMatch(
				Result.Bundle.Helper.Values,
				Helper->Attributes));
	}
	return true;
}

NORMALIZATION_TEST(
	FBoundActionNormalizationHelperAbsentTest,
	"Presence.HelperAbsentUsesSingleZeroAuthority")

bool FBoundActionNormalizationHelperAbsentTest::RunTest(
	const FString& Parameters)
{
	const ESkillRuleType Types[] = {
		ESkillRuleType::PassControl,
		ESkillRuleType::Cross,
		ESkillRuleType::ThroughBall
	};
	for (const ESkillRuleType Type : Types)
	{
		const FMatchPlayState State =
			NormalizationFixtures::MakeReadyState(Type, false);
		const auto Result =
			FMatchPlayBoundActionParticipantNormalizationQuery::Query(
				State,
				NormalizationFixtures::MakeRequest(State));
		TestTrue(TEXT("Helper-absent normalization succeeds"),
			Result.bSuccess);
		TestFalse(TEXT("Helper presence false"),
			Result.Bundle.bHasHelper);
		TestFalse(TEXT("Helper participant absent"),
			Result.Bundle.Helper.bIsPresent);
		TestFalse(TEXT("Helper query not attempted"),
			Result.Bundle.Helper.bSnapshotQueryAttempted);
		TestFalse(TEXT("Helper query not marked successful"),
			Result.Bundle.Helper.bSnapshotQuerySucceeded);
		TestTrue(TEXT("Helper CardId remains None"),
			Result.Bundle.Helper.CardId.IsNone());
		TestTrue(TEXT("Every helper numeric value is typed zero"),
			NormalizationFixtures::ValuesAreZero(
				Result.Bundle.Helper.Values));
		TestEqual(TEXT("Helper stamina is zero"),
			Result.Bundle.Helper.Values.Stamina, 0);
	}
	return true;
}

NORMALIZATION_TEST(
	FBoundActionNormalizationRawValuesTest,
	"Values.AllTenFieldsCopiedWithoutDerivation")

bool FBoundActionNormalizationRawValuesTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayState State =
		NormalizationFixtures::MakeReadyState(
			ESkillRuleType::ThroughBall,
			true);
	const EInitialTurnOrderPlayer Attacker =
		State.RuntimeState.CurrentAttackingPlayer;
	const EInitialTurnOrderPlayer Defender =
		NormalizationFixtures::HelperFixtures::GetDefender(Attacker);
	const FName Ids[] = {
		NormalizationFixtures::HelperFixtures::CarrierId,
		NormalizationFixtures::HelperFixtures::MarkerId,
		NormalizationFixtures::HelperFixtures::RunnerId,
		NormalizationFixtures::HelperFixtures::HelperId
	};
	const EInitialTurnOrderPlayer Sides[] = {
		Attacker,
		Defender,
		Attacker,
		Defender
	};
	for (int32 Index = 0; Index < 4; ++Index)
	{
		FPlayerCardRuleSnapshot* Snapshot =
			NormalizationFixtures::FindSnapshot(
				State,
				Sides[Index],
				Ids[Index]);
		TestNotNull(TEXT("Source snapshot exists"), Snapshot);
		if (Snapshot != nullptr)
		{
			NormalizationFixtures::SetDistinctValues(
				Snapshot->Attributes,
				Index);
		}
	}

	const auto Result =
		FMatchPlayBoundActionParticipantNormalizationQuery::Query(
			State,
			NormalizationFixtures::MakeRequest(State));
	TestTrue(TEXT("Normalization succeeds"), Result.bSuccess);
	const FMatchPlayBoundActionNormalizedParticipant Participants[] = {
		Result.Bundle.Carrier,
		Result.Bundle.Marker,
		Result.Bundle.Runner,
		Result.Bundle.Helper
	};
	for (int32 Index = 0; Index < 4; ++Index)
	{
		const FPlayerCardRuleSnapshot* Snapshot =
			NormalizationFixtures::FindSnapshot(
				State,
				Sides[Index],
				Ids[Index]);
		TestTrue(TEXT("All raw values copied exactly"),
			Snapshot != nullptr
				&& NormalizationFixtures::ValuesMatch(
					Participants[Index].Values,
					Snapshot->Attributes));
	}
	return true;
}

NORMALIZATION_TEST(
	FBoundActionNormalizationPlayerBAttackTest,
	"Authority.PlayerBAttackUsesCorrectSides")

bool FBoundActionNormalizationPlayerBAttackTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State =
		NormalizationFixtures::MakeReadyState(
			ESkillRuleType::Cross,
			true,
			EInitialTurnOrderPlayer::PlayerB);
	const auto Result =
		FMatchPlayBoundActionParticipantNormalizationQuery::Query(
			State,
			NormalizationFixtures::MakeRequest(State));
	TestTrue(TEXT("PlayerB attack succeeds"), Result.bSuccess);
	TestEqual(TEXT("Attacker preserved"),
		Result.CurrentAttackingPlayer,
		EInitialTurnOrderPlayer::PlayerB);
	TestEqual(TEXT("Defender derived by binding"),
		Result.CurrentDefendingPlayer,
		EInitialTurnOrderPlayer::PlayerA);
	TestEqual(TEXT("Carrier side"), Result.Bundle.Carrier.Side,
		EInitialTurnOrderPlayer::PlayerB);
	TestEqual(TEXT("Runner side"), Result.Bundle.Runner.Side,
		EInitialTurnOrderPlayer::PlayerB);
	TestEqual(TEXT("Marker side"), Result.Bundle.Marker.Side,
		EInitialTurnOrderPlayer::PlayerA);
	TestEqual(TEXT("Helper side"), Result.Bundle.Helper.Side,
		EInitialTurnOrderPlayer::PlayerA);
	return true;
}

NORMALIZATION_TEST(
	FBoundActionNormalizationStateReadOnlyTest,
	"ReadOnly.SuccessAndFailureLeaveStateUnchanged")

bool FBoundActionNormalizationStateReadOnlyTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayState SuccessState =
		NormalizationFixtures::MakeReadyState(
			ESkillRuleType::PassControl,
			true);
	const FMatchPlayState SuccessOriginal = SuccessState;
	const auto Success =
		FMatchPlayBoundActionParticipantNormalizationQuery::Query(
			SuccessState,
			NormalizationFixtures::MakeRequest(SuccessState));
	TestTrue(TEXT("Success setup succeeds"), Success.bSuccess);
	TestTrue(TEXT("Success leaves state unchanged"),
		NormalizationFixtures::AreStatesEqual(
			SuccessState,
			SuccessOriginal));

	FMatchPlayState FailureState = SuccessState;
	FMatchPlayBoundActionParticipantNormalizationRequest FailureRequest =
		NormalizationFixtures::MakeRequest(FailureState);
	++FailureRequest.AttackSequence;
	const FMatchPlayState FailureOriginal = FailureState;
	const auto Failure =
		FMatchPlayBoundActionParticipantNormalizationQuery::Query(
			FailureState,
			FailureRequest);
	TestFalse(TEXT("Stale request fails"), Failure.bSuccess);
	TestEqual(TEXT("Binding owns first error"), Failure.ErrorCode,
		EMatchPlayBoundActionParticipantNormalizationErrorCode
			::BindingFailed);
	TestTrue(TEXT("Failure leaves state unchanged"),
		NormalizationFixtures::AreStatesEqual(
			FailureState,
			FailureOriginal));
	return true;
}

NORMALIZATION_TEST(
	FBoundActionNormalizationDeterminismTest,
	"Determinism.CompleteResultStable")

bool FBoundActionNormalizationDeterminismTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State =
		NormalizationFixtures::MakeReadyState(
			ESkillRuleType::ThroughBall,
			true);
	const auto First =
		FMatchPlayBoundActionParticipantNormalizationQuery::Query(
			State,
			NormalizationFixtures::MakeRequest(State));
	const auto Second =
		FMatchPlayBoundActionParticipantNormalizationQuery::Query(
			State,
			NormalizationFixtures::MakeRequest(State));
	TestTrue(TEXT("First succeeds"), First.bSuccess);
	TestTrue(TEXT("Second succeeds"), Second.bSuccess);
	TestTrue(TEXT("Complete reflected result and diagnostics match"),
		NormalizationFixtures::AreResultsEqual(First, Second));
	return true;
}

NORMALIZATION_TEST(
	FBoundActionNormalizationBindingFailuresTest,
	"FirstError.BindingFailuresPrecedeNormalization")

bool FBoundActionNormalizationBindingFailuresTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayState State =
		NormalizationFixtures::MakeReadyState(
			ESkillRuleType::PassControl,
			true);
	const FMatchPlayBoundActionParticipantNormalizationRequest Request =
		NormalizationFixtures::MakeRequest(State);

	FMatchPlayState WrongStage = State;
	WrongStage.CurrentAttack.SelectionStage =
		EMatchPlayCurrentAttackSelectionStage::AwaitingHelper;
	const auto WrongStageResult =
		FMatchPlayBoundActionParticipantNormalizationQuery::Query(
			WrongStage,
			Request);
	TestEqual(TEXT("Wrong stage is BindingFailed"),
		WrongStageResult.ErrorCode,
		EMatchPlayBoundActionParticipantNormalizationErrorCode
			::BindingFailed);
	TestFalse(TEXT("Carrier normalization not attempted"),
		WrongStageResult.Bundle.Carrier.bSnapshotQueryAttempted);

	FMatchPlayState Corrupted = State;
	Corrupted.CurrentAttack.ActionPreparation.CarrierCardId =
		NormalizationFixtures::HelperFixtures::CarrierId;
	const auto CorruptedResult =
		FMatchPlayBoundActionParticipantNormalizationQuery::Query(
			Corrupted,
			Request);
	TestEqual(TEXT("Corruption is BindingFailed"),
		CorruptedResult.ErrorCode,
		EMatchPlayBoundActionParticipantNormalizationErrorCode
			::BindingFailed);
	TestFalse(TEXT("No participant bundle on Binding failure"),
		CorruptedResult.Bundle.Carrier.bSnapshotQueryAttempted);
	return true;
}

#undef NORMALIZATION_TEST

#endif
