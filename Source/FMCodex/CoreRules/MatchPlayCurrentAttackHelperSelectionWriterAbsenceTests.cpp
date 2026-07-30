#include "MatchPlayCurrentAttackHelperSelectionWriter.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "MatchPlayCurrentAttackHelperSelectionTestFixtures.h"
#include "MatchPlayHelperAbsence.h"
#include "Misc/AutomationTest.h"

namespace HelperFixtures =
	FMCodex::Tests::MatchPlayCurrentAttackHelperSelection;

#define HELPER_WRITE_TEST(TestClass, TestName) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
		TestClass, \
		"FMCodex.CoreRules.MatchPlayCurrentAttackHelperSelection.Writer." TestName, \
		EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

HELPER_WRITE_TEST(FHelperSelectWriterSuccessTest, "FinalPayload")

bool FHelperSelectWriterSuccessTest::RunTest(
	const FString& Parameters)
{
	for (const ESkillRuleType Type : {
		ESkillRuleType::PassControl,
		ESkillRuleType::Cross,
		ESkillRuleType::ThroughBall})
	{
		const FMatchPlayState Before = HelperFixtures::MakeState(Type);
		const auto Result =
			FMatchPlayCurrentAttackHelperSelectionWriter::Select(
				Before,
				HelperFixtures::MakeRequest());
		TestTrue(TEXT("Select succeeds"), Result.bSuccess);
		const FMatchPlayCurrentAttackState& Attack =
			Result.AfterState.CurrentAttack;
		if (Type == ESkillRuleType::Cross)
		{
			TestFalse(TEXT("Cross is not Ready yet"),
				Result.ReadyValidationResult.bSuccess);
			TestEqual(TEXT("Cross awaits branch intent"),
				Attack.SelectionStage,
				EMatchPlayCurrentAttackSelectionStage
					::AwaitingBranchIntent);
			TestFalse(TEXT("Cross selected action absent"),
				Attack.bHasSelectedAction);
			TestTrue(TEXT("Cross preparation helper present"),
				Attack.ActionPreparation.bHasHelper);
			TestEqual(TEXT("Cross preparation helper copied"),
				Attack.ActionPreparation.HelperCardId,
				HelperFixtures::HelperId);
		}
		else
		{
			TestTrue(TEXT("Ready validation succeeds"),
				Result.ReadyValidationResult.bSuccess);
			TestEqual(TEXT("Carrier copied"),
				Attack.SelectedAction.CarrierCardId,
				HelperFixtures::CarrierId);
			TestEqual(TEXT("Marker copied"),
				Attack.SelectedAction.MarkerCardId,
				HelperFixtures::MarkerId);
			TestEqual(TEXT("Skill copied"),
				Attack.SelectedAction.SkillId,
				HelperFixtures::GetSkillId(Type));
			TestEqual(TEXT("Type copied"),
				Attack.SelectedAction.ActionType, Type);
			TestEqual(TEXT("Runner copied"),
				Attack.SelectedAction.RunnerCardId,
				HelperFixtures::RunnerId);
			TestTrue(TEXT("Helper present"),
				Attack.SelectedAction.bHasHelper);
			TestEqual(TEXT("Helper copied"),
				Attack.SelectedAction.HelperCardId,
				HelperFixtures::HelperId);
			TestTrue(TEXT("Selected action present"),
				Attack.bHasSelectedAction);
			TestEqual(TEXT("Ready stage"), Attack.SelectionStage,
				EMatchPlayCurrentAttackSelectionStage
					::ReadyForResolution);
			TestEqual(TEXT("Non-elective intent remains None"),
				Attack.SelectedAction.ElectiveBranchIntent,
				EMatchPlayElectiveBranchIntent::None);
			const FMatchPlayCurrentAttackActionPreparationState Empty;
			TestTrue(TEXT("Preparation cleared"),
				FMatchPlayCurrentAttackActionPreparationState::StaticStruct()
					->CompareScriptStruct(
						&Attack.ActionPreparation,
						&Empty,
						0));
		}

		FMatchPlayState Normalized = Result.AfterState;
		Normalized.CurrentAttack.SelectedAction =
			Before.CurrentAttack.SelectedAction;
		Normalized.CurrentAttack.ActionPreparation =
			Before.CurrentAttack.ActionPreparation;
		Normalized.CurrentAttack.bHasSelectedAction =
			Before.CurrentAttack.bHasSelectedAction;
		Normalized.CurrentAttack.SelectionStage =
			Before.CurrentAttack.SelectionStage;
		TestTrue(TEXT("Only final-selection fields changed"),
			HelperFixtures::AreStatesEqual(Normalized, Before));
	}
	return true;
}

HELPER_WRITE_TEST(FHelperSelectWriterFailureAtomicTest, "FailureAtomicity")

bool FHelperSelectWriterFailureAtomicTest::RunTest(
	const FString& Parameters)
{
	for (const FName CardId : TArray<FName>{
		FName(),
		HelperFixtures::MissingId,
		HelperFixtures::MissingSnapshotId,
		HelperFixtures::GoalkeeperId,
		HelperFixtures::MarkerId})
	{
		const FMatchPlayState State = HelperFixtures::MakeState();
		const auto Result =
			FMatchPlayCurrentAttackHelperSelectionWriter::Select(
				State,
				HelperFixtures::MakeRequest(CardId));
		TestFalse(TEXT("Invalid Select fails"), Result.bSuccess);
		TestTrue(TEXT("Invalid Select atomic"),
			HelperFixtures::AreStatesEqual(
				Result.AfterState,
				State));
	}
	FMatchPlayCurrentAttackHelperSelectionRequest WrongSide =
		HelperFixtures::MakeRequest();
	WrongSide.RequestingSide = EInitialTurnOrderPlayer::PlayerA;
	const FMatchPlayState State = HelperFixtures::MakeState();
	const auto Result =
		FMatchPlayCurrentAttackHelperSelectionWriter::Select(
			State, WrongSide);
	TestFalse(TEXT("Wrong side fails"), Result.bSuccess);
	return TestTrue(TEXT("Wrong side atomic"),
		HelperFixtures::AreStatesEqual(Result.AfterState, State));
}

HELPER_WRITE_TEST(FHelperAbsenceMutualExclusionTest,
	"AbsenceMutualExclusion")

bool FHelperAbsenceMutualExclusionTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayResolveNoLegalHelperRequest ResolveRequest;
	ResolveRequest.AttackSequence =
		HelperFixtures::ValidAttackSequence;
	FMatchPlayHelperDeclineRequest DeclineRequest;
	DeclineRequest.AttackSequence =
		HelperFixtures::ValidAttackSequence;
	DeclineRequest.RequestingSide =
		EInitialTurnOrderPlayer::PlayerB;

	const FMatchPlayState Zero = HelperFixtures::MakeZeroLegalState();
	const auto Resolve =
		FMatchPlayResolveNoLegalHelper::Resolve(
			Zero,
			ResolveRequest);
	TestTrue(TEXT("Zero legal Resolve succeeds"), Resolve.bSuccess);
	TestEqual(TEXT("Resolve source"), Resolve.Source,
		EMatchPlayHelperAbsenceSource::ResolveNoLegalHelper);
	TestEqual(TEXT("Resolve reason"), Resolve.Reason,
		EMatchPlayHelperAbsenceReason::NoLegalHelper);
	TestFalse(TEXT("Resolve payload absent"),
		Resolve.AfterState.CurrentAttack.SelectedAction.bHasHelper);
	TestTrue(TEXT("Resolve HelperId empty"),
		Resolve.AfterState.CurrentAttack.SelectedAction
			.HelperCardId.IsNone());
	TestTrue(TEXT("Resolve Ready validation"),
		Resolve.FinalizationResult.ReadyValidationResult.bSuccess);

	const auto WrongDecline =
		FMatchPlayHelperDecline::Decline(Zero, DeclineRequest);
	TestFalse(TEXT("Zero legal Decline rejected"),
		WrongDecline.bSuccess);
	TestEqual(TEXT("Zero legal exact error"),
		WrongDecline.ErrorCode,
		EMatchPlayHelperAbsenceErrorCode
			::NoLegalHelperToDecline);
	TestTrue(TEXT("Zero legal Decline atomic"),
		HelperFixtures::AreStatesEqual(
			WrongDecline.AfterState,
			Zero));

	const FMatchPlayState Legal = HelperFixtures::MakeState();
	const auto Decline =
		FMatchPlayHelperDecline::Decline(Legal, DeclineRequest);
	TestTrue(TEXT("Legal Helper Decline succeeds"),
		Decline.bSuccess);
	TestEqual(TEXT("Decline source"), Decline.Source,
		EMatchPlayHelperAbsenceSource::HelperDecline);
	TestEqual(TEXT("Decline reason"), Decline.Reason,
		EMatchPlayHelperAbsenceReason::HelperDeclined);
	TestFalse(TEXT("Decline payload absent"),
		Decline.AfterState.CurrentAttack.SelectedAction.bHasHelper);

	const auto WrongResolve =
		FMatchPlayResolveNoLegalHelper::Resolve(
			Legal,
			ResolveRequest);
	TestFalse(TEXT("Legal Helper Resolve rejected"),
		WrongResolve.bSuccess);
	TestEqual(TEXT("Legal Helper exact error"),
		WrongResolve.ErrorCode,
		EMatchPlayHelperAbsenceErrorCode::LegalHelperExists);
	TestTrue(TEXT("Legal Helper Resolve atomic"),
		HelperFixtures::AreStatesEqual(
			WrongResolve.AfterState,
			Legal));
	return true;
}

HELPER_WRITE_TEST(FHelperAbsenceBothDefendersTest, "BothDefenders")

bool FHelperAbsenceBothDefendersTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayResolveNoLegalHelperRequest ResolveRequest;
	ResolveRequest.AttackSequence =
		HelperFixtures::ValidAttackSequence;
	for (const EInitialTurnOrderPlayer Attacker : {
		EInitialTurnOrderPlayer::PlayerA,
		EInitialTurnOrderPlayer::PlayerB})
	{
		const auto Resolve =
			FMatchPlayResolveNoLegalHelper::Resolve(
				HelperFixtures::MakeZeroLegalState(Attacker),
				ResolveRequest);
		TestTrue(TEXT("Resolve derives defender"),
			Resolve.bSuccess);

		FMatchPlayHelperDeclineRequest DeclineRequest;
		DeclineRequest.AttackSequence =
			HelperFixtures::ValidAttackSequence;
		DeclineRequest.RequestingSide =
			HelperFixtures::GetDefender(Attacker);
		const auto Decline =
			FMatchPlayHelperDecline::Decline(
				HelperFixtures::MakeState(
					ESkillRuleType::PassControl,
					Attacker),
				DeclineRequest);
		TestTrue(TEXT("Decline accepts current defender"),
			Decline.bSuccess);
	}
	return true;
}

HELPER_WRITE_TEST(FHelperRepeatedCrossCallsTest, "RepeatedCrossCalls")

bool FHelperRepeatedCrossCallsTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayResolveNoLegalHelperRequest ResolveRequest;
	ResolveRequest.AttackSequence =
		HelperFixtures::ValidAttackSequence;
	FMatchPlayHelperDeclineRequest DeclineRequest;
	DeclineRequest.AttackSequence =
		HelperFixtures::ValidAttackSequence;
	DeclineRequest.RequestingSide =
		EInitialTurnOrderPlayer::PlayerB;
	const auto SelectFirst =
		FMatchPlayCurrentAttackHelperSelectionWriter::Select(
			HelperFixtures::MakeState(),
			HelperFixtures::MakeRequest());
	const auto ResolveFirst =
		FMatchPlayResolveNoLegalHelper::Resolve(
			HelperFixtures::MakeZeroLegalState(),
			ResolveRequest);
	const auto DeclineFirst =
		FMatchPlayHelperDecline::Decline(
			HelperFixtures::MakeState(),
			DeclineRequest);
	TestTrue(TEXT("Select setup"), SelectFirst.bSuccess);
	TestTrue(TEXT("Resolve setup"), ResolveFirst.bSuccess);
	TestTrue(TEXT("Decline setup"), DeclineFirst.bSuccess);

	auto CheckThree = [this, &ResolveRequest, &DeclineRequest](
		const TCHAR* Label,
		const FMatchPlayState& Ready)
	{
		const FMatchPlayState Original = Ready;
		const auto Select =
			FMatchPlayCurrentAttackHelperSelectionWriter::Select(
				Ready,
				HelperFixtures::MakeRequest());
		const auto Resolve =
			FMatchPlayResolveNoLegalHelper::Resolve(
				Ready,
				ResolveRequest);
		const auto Decline =
			FMatchPlayHelperDecline::Decline(
				Ready,
				DeclineRequest);
		TestFalse(Label, Select.bSuccess);
		TestFalse(Label, Resolve.bSuccess);
		TestFalse(Label, Decline.bSuccess);
		TestTrue(TEXT("Repeated Select atomic"),
			HelperFixtures::AreStatesEqual(
				Select.AfterState,
				Original));
		TestTrue(TEXT("Repeated Resolve atomic"),
			HelperFixtures::AreStatesEqual(
				Resolve.AfterState,
				Original));
		TestTrue(TEXT("Repeated Decline atomic"),
			HelperFixtures::AreStatesEqual(
				Decline.AfterState,
				Original));
	};
	CheckThree(TEXT("Select-first repeats fail"),
		SelectFirst.AfterState);
	CheckThree(TEXT("Resolve-first repeats fail"),
		ResolveFirst.AfterState);
	CheckThree(TEXT("Decline-first repeats fail"),
		DeclineFirst.AfterState);
	return true;
}

#undef HELPER_WRITE_TEST

#endif
