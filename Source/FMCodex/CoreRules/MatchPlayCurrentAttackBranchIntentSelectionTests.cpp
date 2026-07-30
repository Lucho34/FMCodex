#include "MatchPlayCurrentAttackBranchIntentSelectionWriter.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "MatchPlayBoundActionParticipantNormalizationTestFixtures.h"
#include "MatchPlayCurrentAttackBranchIntentSelectionTestFixtures.h"
#include "MatchPlayCurrentAttackResolutionBinding.h"
#include "MatchPlayHelperAbsence.h"
#include "Misc/AutomationTest.h"
#include "UObject/UnrealType.h"

#include <type_traits>

namespace BranchIntentSelectionContractTests
{
	namespace Fixtures =
		FMCodex::Tests::MatchPlayCurrentAttackBranchIntentSelection;
	namespace HelperFixtures =
		FMCodex::Tests::MatchPlayCurrentAttackHelperSelection;
	namespace NormalizationFixtures =
		FMCodex::Tests::MatchPlayBoundActionParticipantNormalization;

	bool IsLegal(
		const FMatchPlayState& State,
		const EMatchPlayElectiveBranchIntent Intent)
	{
		return
			FMatchPlayCurrentAttackBranchIntentSelectionLegalityEvaluator
				::Evaluate(
					State,
					Fixtures::MakeRequest(State, Intent))
				.bIsLegal;
	}

	bool AreGlobalResultsEqual(
		const FMatchPlayCurrentAttackBranchIntentSelectionGlobalContextResult&
			Left,
		const FMatchPlayCurrentAttackBranchIntentSelectionGlobalContextResult&
			Right)
	{
		return Left.bSuccess == Right.bSuccess
			&& Left.RequestedAttackSequence
				== Right.RequestedAttackSequence
			&& Left.AuthoritativeAttackSequence
				== Right.AuthoritativeAttackSequence
			&& Left.RequestingSide == Right.RequestingSide
			&& Left.CurrentAttackingPlayer
				== Right.CurrentAttackingPlayer
			&& Left.CurrentDefendingPlayer
				== Right.CurrentDefendingPlayer
			&& FMatchPlayCurrentAttackSelectionStateValidationResult
				::StaticStruct()->CompareScriptStruct(
					&Left.SelectionStateValidationResult,
					&Right.SelectionStateValidationResult,
					0)
			&& FMatchPlayCurrentAttackActionPreparationState
				::StaticStruct()->CompareScriptStruct(
					&Left.Preparation,
					&Right.Preparation,
					0)
			&& Left.FrozenActionType == Right.FrozenActionType
			&& Left.MatchingCarrierPlacementCount
				== Right.MatchingCarrierPlacementCount
			&& Left.MatchingMarkerPlacementCount
				== Right.MatchingMarkerPlacementCount
			&& Left.MatchingRunnerPlacementCount
				== Right.MatchingRunnerPlacementCount
			&& NormalizationFixtures::AreAuthorityQueryResultsEqual(
				Left.CarrierSnapshotQueryResult,
				Right.CarrierSnapshotQueryResult)
			&& NormalizationFixtures::AreAuthorityQueryResultsEqual(
				Left.MarkerSnapshotQueryResult,
				Right.MarkerSnapshotQueryResult)
			&& NormalizationFixtures::AreAuthorityQueryResultsEqual(
				Left.RunnerSnapshotQueryResult,
				Right.RunnerSnapshotQueryResult)
			&& NormalizationFixtures::AreHelperAuthorityResultsEqual(
				Left.HelperAuthorityResult,
				Right.HelperAuthorityResult)
			&& Left.ErrorCode == Right.ErrorCode
			&& Left.ErrorMessage == Right.ErrorMessage;
	}

	bool AreLegalityResultsEqual(
		const FMatchPlayCurrentAttackBranchIntentSelectionLegalityResult&
			Left,
		const FMatchPlayCurrentAttackBranchIntentSelectionLegalityResult&
			Right)
	{
		return FMatchPlayCurrentAttackBranchIntentSelectionLegalityResult
				::StaticStruct()->CompareScriptStruct(
					&Left,
					&Right,
					0)
			&& AreGlobalResultsEqual(
				Left.GlobalContextResult,
				Right.GlobalContextResult);
	}
}

#define BRANCH_INTENT_TEST(TestClass, TestName) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
		TestClass, \
		"FMCodex.CoreRules.MatchPlayCurrentAttackBranchIntentSelection." TestName, \
		EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

BRANCH_INTENT_TEST(
	FBranchIntentTypeContractTest,
	"Types.EnumsAndReflection")

bool FBranchIntentTypeContractTest::RunTest(
	const FString& Parameters)
{
	TestEqual(TEXT("AwaitingBranchIntent is appended at 7"),
		static_cast<uint8>(
			EMatchPlayCurrentAttackSelectionStage
				::AwaitingBranchIntent),
		uint8{7});
	TestEqual(TEXT("Intent None value"),
		static_cast<uint8>(
			EMatchPlayElectiveBranchIntent::None),
		uint8{0});
	TestEqual(TEXT("Intent DirectShot value"),
		static_cast<uint8>(
			EMatchPlayElectiveBranchIntent::DirectShot),
		uint8{1});
	TestEqual(TEXT("Intent DeadCorner value"),
		static_cast<uint8>(
			EMatchPlayElectiveBranchIntent::DeadCorner),
		uint8{2});
	TestEqual(TEXT("Intent CrossHigh value"),
		static_cast<uint8>(
			EMatchPlayElectiveBranchIntent::CrossHigh),
		uint8{3});
	TestEqual(TEXT("Intent CrossLow value"),
		static_cast<uint8>(
			EMatchPlayElectiveBranchIntent::CrossLow),
		uint8{4});
	TestEqual(TEXT("Intent enum has exactly five business values"),
		StaticEnum<EMatchPlayElectiveBranchIntent>()->NumEnums(),
		6);
	TestEqual(TEXT("Intent default is None"),
		FMatchPlayCurrentAttackSelectedAction()
			.ElectiveBranchIntent,
		EMatchPlayElectiveBranchIntent::None);

	int32 RequestPropertyCount = 0;
	for (TFieldIterator<FProperty> It(
		FMatchPlayCurrentAttackBranchIntentSelectionRequest
			::StaticStruct(),
		EFieldIteratorFlags::ExcludeSuper);
		It;
		++It)
	{
		++RequestPropertyCount;
	}
	TestEqual(TEXT("Request has exactly three business fields"),
		RequestPropertyCount,
		3);
	TestNotNull(TEXT("Request AttackSequence reflected"),
		FMatchPlayCurrentAttackBranchIntentSelectionRequest
			::StaticStruct()
			->FindPropertyByName(TEXT("AttackSequence")));
	TestNotNull(TEXT("Request RequestingSide reflected"),
		FMatchPlayCurrentAttackBranchIntentSelectionRequest
			::StaticStruct()
			->FindPropertyByName(TEXT("RequestingSide")));
	TestNotNull(TEXT("Request Intent reflected"),
		FMatchPlayCurrentAttackBranchIntentSelectionRequest
			::StaticStruct()
			->FindPropertyByName(TEXT("Intent")));
	TestNotNull(TEXT("Preparation bHasHelper reflected"),
		FMatchPlayCurrentAttackActionPreparationState::StaticStruct()
			->FindPropertyByName(TEXT("bHasHelper")));
	TestNotNull(TEXT("Preparation HelperCardId reflected"),
		FMatchPlayCurrentAttackActionPreparationState::StaticStruct()
			->FindPropertyByName(TEXT("HelperCardId")));
	TestNotNull(TEXT("SelectedAction Intent reflected"),
		FMatchPlayCurrentAttackSelectedAction::StaticStruct()
			->FindPropertyByName(TEXT("ElectiveBranchIntent")));

	using FLegalitySignature =
		FMatchPlayCurrentAttackBranchIntentSelectionLegalityResult (*)(
			const FMatchPlayState&,
			const FMatchPlayCurrentAttackBranchIntentSelectionRequest&);
	using FWriterSignature =
		FMatchPlayCurrentAttackBranchIntentSelectionWriterResult (*)(
			const FMatchPlayState&,
			const FMatchPlayCurrentAttackBranchIntentSelectionRequest&);
	TestTrue(TEXT("One public Legality signature"),
		(std::is_same_v<
			decltype(
				&FMatchPlayCurrentAttackBranchIntentSelectionLegalityEvaluator
					::Evaluate),
			FLegalitySignature>));
	TestTrue(TEXT("One public Writer signature"),
		(std::is_same_v<
			decltype(
				&FMatchPlayCurrentAttackBranchIntentSelectionWriter
					::Select),
			FWriterSignature>));
	return true;
}

BRANCH_INTENT_TEST(
	FBranchIntentShotTransitionTest,
	"ShotTransitionsAndIntentMatrix")

bool FBranchIntentShotTransitionTest::RunTest(
	const FString& Parameters)
{
	using namespace BranchIntentSelectionContractTests;
	for (const ESkillRuleType ActionType : {
		ESkillRuleType::LongShot,
		ESkillRuleType::CutInsideShot})
	{
		const FMatchPlayState Awaiting =
			Fixtures::MakeAwaitingShotIntent(ActionType);
		TestEqual(TEXT("Shot awaits intent"),
			Awaiting.CurrentAttack.SelectionStage,
			EMatchPlayCurrentAttackSelectionStage
				::AwaitingBranchIntent);
		TestFalse(TEXT("Shot is not Ready"),
			Awaiting.CurrentAttack.bHasSelectedAction);
		TestEqual(TEXT("Preparation intent authority is None"),
			Awaiting.CurrentAttack.SelectedAction
				.ElectiveBranchIntent,
			EMatchPlayElectiveBranchIntent::None);
		TestTrue(TEXT("DirectShot legal"),
			IsLegal(
				Awaiting,
				EMatchPlayElectiveBranchIntent::DirectShot));
		TestTrue(TEXT("DeadCorner legal"),
			IsLegal(
				Awaiting,
				EMatchPlayElectiveBranchIntent::DeadCorner));
		TestFalse(TEXT("CrossHigh illegal"),
			IsLegal(
				Awaiting,
				EMatchPlayElectiveBranchIntent::CrossHigh));
		TestFalse(TEXT("CrossLow illegal"),
			IsLegal(
				Awaiting,
				EMatchPlayElectiveBranchIntent::CrossLow));
		TestFalse(TEXT("None illegal"),
			IsLegal(
				Awaiting,
				EMatchPlayElectiveBranchIntent::None));

		for (const EMatchPlayElectiveBranchIntent Intent : {
			EMatchPlayElectiveBranchIntent::DirectShot,
			EMatchPlayElectiveBranchIntent::DeadCorner})
		{
			const auto Result =
				FMatchPlayCurrentAttackBranchIntentSelectionWriter
					::Select(
						Awaiting,
						Fixtures::MakeRequest(Awaiting, Intent));
			TestTrue(TEXT("Shot Writer succeeds"),
				Result.bSuccess);
			TestTrue(TEXT("Shot Ready validates"),
				Result.ReadyValidationResult.bSuccess);
			TestEqual(TEXT("Shot enters Ready"),
				Result.AfterState.CurrentAttack.SelectionStage,
				EMatchPlayCurrentAttackSelectionStage
					::ReadyForResolution);
			TestEqual(TEXT("Shot final Intent matches"),
				Result.AfterState.CurrentAttack.SelectedAction
					.ElectiveBranchIntent,
				Intent);

			const auto Repeat =
				FMatchPlayCurrentAttackBranchIntentSelectionWriter
					::Select(
						Result.AfterState,
						Fixtures::MakeRequest(
							Result.AfterState,
							Intent));
			TestFalse(TEXT("Duplicate Writer fails"),
				Repeat.bSuccess);
			TestTrue(TEXT("Duplicate Writer atomic"),
				Fixtures::AreStatesEqual(
					Repeat.BeforeState,
					Repeat.AfterState));
		}
	}
	return true;
}

BRANCH_INTENT_TEST(
	FBranchIntentCrossTransitionTest,
	"CrossHelperPathsAndIntentMatrix")

bool FBranchIntentCrossTransitionTest::RunTest(
	const FString& Parameters)
{
	using namespace BranchIntentSelectionContractTests;
	for (const Fixtures::ECrossHelperPath Path : {
		Fixtures::ECrossHelperPath::Selected,
		Fixtures::ECrossHelperPath::Declined,
		Fixtures::ECrossHelperPath::NoLegal})
	{
		const FMatchPlayState Awaiting =
			Fixtures::MakeAwaitingCrossIntent(Path);
		TestEqual(TEXT("Cross awaits intent"),
			Awaiting.CurrentAttack.SelectionStage,
			EMatchPlayCurrentAttackSelectionStage
				::AwaitingBranchIntent);
		const bool bExpectedHelper =
			Path == Fixtures::ECrossHelperPath::Selected;
		TestEqual(TEXT("Cross Helper presence preserved"),
			Awaiting.CurrentAttack.ActionPreparation.bHasHelper,
			bExpectedHelper);
		TestEqual(TEXT("Cross Helper id preserved"),
			Awaiting.CurrentAttack.ActionPreparation.HelperCardId,
			bExpectedHelper
				? HelperFixtures::HelperId
				: NAME_None);
		TestFalse(TEXT("Cross selected action absent"),
			Awaiting.CurrentAttack.bHasSelectedAction);
		TestTrue(TEXT("CrossHigh legal"),
			IsLegal(
				Awaiting,
				EMatchPlayElectiveBranchIntent::CrossHigh));
		TestTrue(TEXT("CrossLow legal"),
			IsLegal(
				Awaiting,
				EMatchPlayElectiveBranchIntent::CrossLow));
		TestFalse(TEXT("DirectShot illegal"),
			IsLegal(
				Awaiting,
				EMatchPlayElectiveBranchIntent::DirectShot));
		TestFalse(TEXT("DeadCorner illegal"),
			IsLegal(
				Awaiting,
				EMatchPlayElectiveBranchIntent::DeadCorner));
		TestFalse(TEXT("None illegal"),
			IsLegal(
				Awaiting,
				EMatchPlayElectiveBranchIntent::None));

		for (const EMatchPlayElectiveBranchIntent Intent : {
			EMatchPlayElectiveBranchIntent::CrossHigh,
			EMatchPlayElectiveBranchIntent::CrossLow})
		{
			const auto Result =
				FMatchPlayCurrentAttackBranchIntentSelectionWriter
					::Select(
						Awaiting,
						Fixtures::MakeRequest(Awaiting, Intent));
			TestTrue(TEXT("Cross Writer succeeds"),
				Result.bSuccess);
			TestTrue(TEXT("Cross Ready validates"),
				Result.ReadyValidationResult.bSuccess);
			TestEqual(TEXT("Cross final Intent"),
				Result.AfterState.CurrentAttack.SelectedAction
					.ElectiveBranchIntent,
				Intent);
			TestEqual(TEXT("Cross final Helper presence"),
				Result.AfterState.CurrentAttack.SelectedAction
					.bHasHelper,
				bExpectedHelper);
			TestEqual(TEXT("Cross final Helper id"),
				Result.AfterState.CurrentAttack.SelectedAction
					.HelperCardId,
				bExpectedHelper
					? HelperFixtures::HelperId
					: NAME_None);
		}
	}
	return true;
}

BRANCH_INTENT_TEST(
	FBranchIntentNonElectiveRegressionTest,
	"PassControlAndThroughBallRemainAutomatic")

bool FBranchIntentNonElectiveRegressionTest::RunTest(
	const FString& Parameters)
{
	using namespace BranchIntentSelectionContractTests;
	for (const ESkillRuleType ActionType : {
		ESkillRuleType::PassControl,
		ESkillRuleType::ThroughBall})
	{
		for (const Fixtures::ECrossHelperPath Path : {
			Fixtures::ECrossHelperPath::Selected,
			Fixtures::ECrossHelperPath::Declined,
			Fixtures::ECrossHelperPath::NoLegal})
		{
			FMatchPlayState ResultState;
			if (Path == Fixtures::ECrossHelperPath::Selected)
			{
				ResultState =
					FMatchPlayCurrentAttackHelperSelectionWriter
						::Select(
							HelperFixtures::MakeState(ActionType),
							HelperFixtures::MakeRequest())
						.AfterState;
			}
			else if (Path == Fixtures::ECrossHelperPath::Declined)
			{
				FMatchPlayHelperDeclineRequest Request;
				Request.AttackSequence =
					HelperFixtures::ValidAttackSequence;
				Request.RequestingSide =
					EInitialTurnOrderPlayer::PlayerB;
				ResultState =
					FMatchPlayHelperDecline::Decline(
						HelperFixtures::MakeState(ActionType),
						Request)
						.AfterState;
			}
			else
			{
				FMatchPlayResolveNoLegalHelperRequest Request;
				Request.AttackSequence =
					HelperFixtures::ValidAttackSequence;
				ResultState =
					FMatchPlayResolveNoLegalHelper::Resolve(
						HelperFixtures
							::MakeZeroLegalStateForType(
								ActionType),
						Request)
						.AfterState;
			}
			TestEqual(TEXT("Non-elective path enters Ready"),
				ResultState.CurrentAttack.SelectionStage,
				EMatchPlayCurrentAttackSelectionStage
					::ReadyForResolution);
			TestEqual(TEXT("Non-elective Intent is None"),
				ResultState.CurrentAttack.SelectedAction
					.ElectiveBranchIntent,
				EMatchPlayElectiveBranchIntent::None);
		}

		FMatchPlayState InvalidAwaiting =
			HelperFixtures::MakeState(ActionType);
		InvalidAwaiting.CurrentAttack.SelectionStage =
			EMatchPlayCurrentAttackSelectionStage
				::AwaitingBranchIntent;
		const auto Legality =
			FMatchPlayCurrentAttackBranchIntentSelectionLegalityEvaluator
				::Evaluate(
					InvalidAwaiting,
					Fixtures::MakeRequest(
						InvalidAwaiting,
						EMatchPlayElectiveBranchIntent
							::DirectShot));
		TestFalse(TEXT("Non-elective Legality rejects"),
			Legality.bIsLegal);
		const auto Writer =
			FMatchPlayCurrentAttackBranchIntentSelectionWriter
				::Select(
					InvalidAwaiting,
					Fixtures::MakeRequest(
						InvalidAwaiting,
						EMatchPlayElectiveBranchIntent
							::DirectShot));
		TestFalse(TEXT("Non-elective Writer rejects"),
			Writer.bSuccess);
		TestTrue(TEXT("Non-elective Writer is atomic"),
			Fixtures::AreStatesEqual(
				Writer.BeforeState,
				Writer.AfterState));
	}
	return true;
}

BRANCH_INTENT_TEST(
	FBranchIntentLegalityDeterminismTest,
	"LegalityFullResultDeterminism")

bool FBranchIntentLegalityDeterminismTest::RunTest(
	const FString& Parameters)
{
	using namespace BranchIntentSelectionContractTests;
	for (const FMatchPlayState State : {
		Fixtures::MakeAwaitingShotIntent(
			ESkillRuleType::LongShot),
		Fixtures::MakeAwaitingCrossIntent(
			Fixtures::ECrossHelperPath::Selected)})
	{
		for (const EMatchPlayElectiveBranchIntent Intent : {
			EMatchPlayElectiveBranchIntent::DirectShot,
			EMatchPlayElectiveBranchIntent::CrossHigh,
			EMatchPlayElectiveBranchIntent::None})
		{
			const auto Request =
				Fixtures::MakeRequest(State, Intent);
			const auto First =
				FMatchPlayCurrentAttackBranchIntentSelectionLegalityEvaluator
					::Evaluate(State, Request);
			const auto Second =
				FMatchPlayCurrentAttackBranchIntentSelectionLegalityEvaluator
					::Evaluate(State, Request);
			TestTrue(TEXT("Complete Legality result deterministic"),
				AreLegalityResultsEqual(First, Second));
		}
	}
	return true;
}

#undef BRANCH_INTENT_TEST

#endif
