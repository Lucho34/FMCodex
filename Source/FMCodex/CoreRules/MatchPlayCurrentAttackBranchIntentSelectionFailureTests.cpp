#include "MatchPlayCurrentAttackBranchIntentSelectionWriter.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "MatchPlayCurrentAttackBranchIntentSelectionTestFixtures.h"
#include "Misc/AutomationTest.h"

namespace BranchIntentSelectionFailureTests
{
	namespace Fixtures =
		FMCodex::Tests::MatchPlayCurrentAttackBranchIntentSelection;
	namespace HelperFixtures =
		FMCodex::Tests::MatchPlayCurrentAttackHelperSelection;

	bool IsStateEqual(
		const FMatchPlayState& Left,
		const FMatchPlayState& Right)
	{
		return Fixtures::AreStatesEqual(Left, Right);
	}
}

#define BRANCH_INTENT_FAILURE_TEST(TestClass, TestName) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
		TestClass, \
		"FMCodex.CoreRules.MatchPlayCurrentAttackBranchIntentSelection.Failures." TestName, \
		EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

BRANCH_INTENT_FAILURE_TEST(
	FBranchIntentGlobalContextFailureMatrixTest,
	"GlobalContextMatrix")

bool FBranchIntentGlobalContextFailureMatrixTest::RunTest(
	const FString& Parameters)
{
	using namespace BranchIntentSelectionFailureTests;
	const FMatchPlayState Base =
		Fixtures::MakeAwaitingShotIntent(
			ESkillRuleType::LongShot);
	auto Query = [](const FMatchPlayState& State,
		const int64 Sequence,
		const EInitialTurnOrderPlayer Side)
	{
		return
			FMatchPlayCurrentAttackBranchIntentSelectionGlobalContextQuery
				::Query(State, Sequence, Side);
	};

	const auto Stale = Query(
		Base,
		Base.CurrentAttack.AttackSequence - 1,
		EInitialTurnOrderPlayer::PlayerB);
	TestFalse(TEXT("Stale sequence fails"), Stale.bSuccess);
	TestEqual(TEXT("Stale sequence is first error"),
		Stale.ErrorCode,
		EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
			::AttackSequenceMismatch);
	const auto Ahead = Query(
		Base,
		Base.CurrentAttack.AttackSequence + 1,
		EInitialTurnOrderPlayer::PlayerA);
	TestFalse(TEXT("Ahead sequence fails"), Ahead.bSuccess);
	TestEqual(TEXT("Ahead sequence exact error"),
		Ahead.ErrorCode,
		EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
			::AttackSequenceMismatch);
	const auto WrongSide = Query(
		Base,
		Base.CurrentAttack.AttackSequence,
		EInitialTurnOrderPlayer::PlayerB);
	TestFalse(TEXT("Defender request fails"),
		WrongSide.bSuccess);
	TestEqual(TEXT("Defender request exact error"),
		WrongSide.ErrorCode,
		EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
			::RequestingSideIsNotCurrentAttacker);
	const auto InvalidSide = Query(
		Base,
		Base.CurrentAttack.AttackSequence,
		EInitialTurnOrderPlayer::None);
	TestFalse(TEXT("Invalid side fails"),
		InvalidSide.bSuccess);
	TestEqual(TEXT("Invalid side exact error"),
		InvalidSide.ErrorCode,
		EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
			::InvalidRequestingSide);

	FMatchPlayState NoAttack = Base;
	NoAttack.bHasCurrentAttack = false;
	TestEqual(TEXT("No attack exact error"),
		Query(
			NoAttack,
			Base.CurrentAttack.AttackSequence,
			EInitialTurnOrderPlayer::PlayerA)
			.ErrorCode,
		EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
			::NoCurrentAttack);

	FMatchPlayState WrongPhase = Base;
	WrongPhase.CurrentAttack.Phase =
		EMatchPlayCurrentAttackPhase::Deployment;
	TestEqual(TEXT("Wrong phase exact error"),
		Query(
			WrongPhase,
			Base.CurrentAttack.AttackSequence,
			EInitialTurnOrderPlayer::PlayerA)
			.ErrorCode,
		EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
			::CurrentAttackNotInResolution);

	for (const EMatchPlayCurrentAttackSelectionStage Stage : {
		EMatchPlayCurrentAttackSelectionStage::AwaitingSkill,
		EMatchPlayCurrentAttackSelectionStage::AwaitingRunner,
		EMatchPlayCurrentAttackSelectionStage::AwaitingHelper,
		EMatchPlayCurrentAttackSelectionStage::ReadyForResolution})
	{
		FMatchPlayState WrongStage = Base;
		WrongStage.CurrentAttack.SelectionStage = Stage;
		const auto Result = Query(
			WrongStage,
			Base.CurrentAttack.AttackSequence,
			EInitialTurnOrderPlayer::PlayerA);
		TestFalse(TEXT("Wrong stage fails"), Result.bSuccess);
		TestTrue(TEXT("Wrong stage has stable error"),
			Result.ErrorCode
					== EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
						::InvalidSelectionState
				|| Result.ErrorCode
					== EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
						::WrongSelectionStage);
	}

	for (const ESkillRuleType ActionType : {
		ESkillRuleType::None,
		ESkillRuleType::PassControl,
		ESkillRuleType::ThroughBall})
	{
		FMatchPlayState InvalidSkill = Base;
		InvalidSkill.CurrentAttack.ActionPreparation.ActionType =
			ActionType;
		const auto Result = Query(
			InvalidSkill,
			Base.CurrentAttack.AttackSequence,
			EInitialTurnOrderPlayer::PlayerA);
		TestFalse(TEXT("Unsupported frozen skill fails"),
			Result.bSuccess);
		TestTrue(TEXT("Unsupported skill has diagnostics"),
			!Result.ErrorMessage.IsEmpty());
	}

	FMatchPlayState MissingSkill = Base;
	MissingSkill.CurrentAttack.ActionPreparation.SkillId =
		NAME_None;
	TestFalse(TEXT("Missing Skill fails"),
		Query(
			MissingSkill,
			Base.CurrentAttack.AttackSequence,
			EInitialTurnOrderPlayer::PlayerA)
			.bSuccess);

	FMatchPlayState CarrierDamage = Base;
	CarrierDamage.CurrentAttack.DeploymentPlacements.RemoveAll(
		[&CarrierDamage](
			const FMatchPlayDeploymentPlacement& Placement)
		{
			return Placement.PlayerSide
					== EInitialTurnOrderPlayer::PlayerA
				&& Placement.CardId
					== CarrierDamage.CurrentAttack
						.ActionPreparation.CarrierCardId;
		});
	TestEqual(TEXT("Carrier damage exact error"),
		Query(
			CarrierDamage,
			Base.CurrentAttack.AttackSequence,
			EInitialTurnOrderPlayer::PlayerA)
			.ErrorCode,
		EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
			::CarrierDeploymentInvalid);

	FMatchPlayState MarkerDamage = Base;
	MarkerDamage.CurrentAttack.DeploymentPlacements.RemoveAll(
		[&MarkerDamage](
			const FMatchPlayDeploymentPlacement& Placement)
		{
			return Placement.PlayerSide
					== EInitialTurnOrderPlayer::PlayerB
				&& Placement.CardId
					== MarkerDamage.CurrentAttack
						.ActionPreparation.MarkerCardId;
		});
	TestEqual(TEXT("Marker damage exact error"),
		Query(
			MarkerDamage,
			Base.CurrentAttack.AttackSequence,
			EInitialTurnOrderPlayer::PlayerA)
			.ErrorCode,
		EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
			::MarkerDeploymentInvalid);

	const FMatchPlayState Cross =
		Fixtures::MakeAwaitingCrossIntent(
			Fixtures::ECrossHelperPath::Selected);
	FMatchPlayState MissingRunner = Cross;
	MissingRunner.CurrentAttack.ActionPreparation.RunnerCardId =
		NAME_None;
	TestFalse(TEXT("Missing Cross Runner fails"),
		Query(
			MissingRunner,
			Cross.CurrentAttack.AttackSequence,
			EInitialTurnOrderPlayer::PlayerA)
			.bSuccess);

	FMatchPlayState DamagedHelper = Cross;
	DamagedHelper.CurrentAttack.ActionPreparation.bHasHelper =
		false;
	TestFalse(TEXT("Damaged Helper presence fails"),
		Query(
			DamagedHelper,
			Cross.CurrentAttack.AttackSequence,
			EInitialTurnOrderPlayer::PlayerA)
			.bSuccess);

	TestTrue(TEXT("Global Context is read-only"),
		IsStateEqual(
			Base,
			Fixtures::MakeAwaitingShotIntent(
				ESkillRuleType::LongShot)));
	return true;
}

BRANCH_INTENT_FAILURE_TEST(
	FBranchIntentCombinationFailureMatrixTest,
	"IntentCombinationMatrix")

bool FBranchIntentCombinationFailureMatrixTest::RunTest(
	const FString& Parameters)
{
	using namespace BranchIntentSelectionFailureTests;
	for (const ESkillRuleType ActionType : {
		ESkillRuleType::LongShot,
		ESkillRuleType::CutInsideShot})
	{
		const FMatchPlayState State =
			Fixtures::MakeAwaitingShotIntent(ActionType);
		for (const EMatchPlayElectiveBranchIntent Intent : {
			EMatchPlayElectiveBranchIntent::None,
			EMatchPlayElectiveBranchIntent::CrossHigh,
			EMatchPlayElectiveBranchIntent::CrossLow})
		{
			const auto Result =
				FMatchPlayCurrentAttackBranchIntentSelectionLegalityEvaluator
					::Evaluate(
						State,
						Fixtures::MakeRequest(State, Intent));
			TestFalse(TEXT("Shot invalid Intent fails"),
				Result.bIsLegal);
		}
	}

	const FMatchPlayState Cross =
		Fixtures::MakeAwaitingCrossIntent(
			Fixtures::ECrossHelperPath::Selected);
	for (const EMatchPlayElectiveBranchIntent Intent : {
		EMatchPlayElectiveBranchIntent::None,
		EMatchPlayElectiveBranchIntent::DirectShot,
		EMatchPlayElectiveBranchIntent::DeadCorner})
	{
		const auto Result =
			FMatchPlayCurrentAttackBranchIntentSelectionLegalityEvaluator
				::Evaluate(
					Cross,
					Fixtures::MakeRequest(Cross, Intent));
		TestFalse(TEXT("Cross invalid Intent fails"),
			Result.bIsLegal);
	}

	const auto UnknownIntent =
		static_cast<EMatchPlayElectiveBranchIntent>(255);
	const FMatchPlayState Shot =
		Fixtures::MakeAwaitingShotIntent(
			ESkillRuleType::LongShot);
	const auto Unknown =
		FMatchPlayCurrentAttackBranchIntentSelectionLegalityEvaluator
			::Evaluate(
				Shot,
				Fixtures::MakeRequest(Shot, UnknownIntent));
	TestFalse(TEXT("Unknown Intent safely fails"),
		Unknown.bIsLegal);
	TestEqual(TEXT("Unknown Intent exact error"),
		Unknown.ErrorCode,
		EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
			::InvalidIntent);
	return true;
}

BRANCH_INTENT_FAILURE_TEST(
	FBranchIntentWriterAtomicityTest,
	"WriterAtomicity")

bool FBranchIntentWriterAtomicityTest::RunTest(
	const FString& Parameters)
{
	using namespace BranchIntentSelectionFailureTests;
	const FMatchPlayState Shot =
		Fixtures::MakeAwaitingShotIntent(
			ESkillRuleType::LongShot);

	TArray<FMatchPlayCurrentAttackBranchIntentSelectionRequest>
		Requests;
	auto WrongSequence = Fixtures::MakeRequest(
		Shot,
		EMatchPlayElectiveBranchIntent::DirectShot);
	WrongSequence.AttackSequence++;
	Requests.Add(WrongSequence);
	auto WrongSide = Fixtures::MakeRequest(
		Shot,
		EMatchPlayElectiveBranchIntent::DirectShot);
	WrongSide.RequestingSide =
		EInitialTurnOrderPlayer::PlayerB;
	Requests.Add(WrongSide);
	Requests.Add(Fixtures::MakeRequest(
		Shot,
		EMatchPlayElectiveBranchIntent::CrossHigh));
	Requests.Add(Fixtures::MakeRequest(
		Shot,
		EMatchPlayElectiveBranchIntent::None));

	for (const auto& Request : Requests)
	{
		const auto Result =
			FMatchPlayCurrentAttackBranchIntentSelectionWriter
				::Select(Shot, Request);
		TestFalse(TEXT("Invalid Writer request fails"),
			Result.bSuccess);
		TestTrue(TEXT("Invalid Writer request atomic"),
			IsStateEqual(
				Result.BeforeState,
				Result.AfterState));
	}

	FMatchPlayState WrongStage = Shot;
	WrongStage.CurrentAttack.SelectionStage =
		EMatchPlayCurrentAttackSelectionStage::AwaitingSkill;
	const auto WrongStageResult =
		FMatchPlayCurrentAttackBranchIntentSelectionWriter::Select(
			WrongStage,
			Fixtures::MakeRequest(
				WrongStage,
				EMatchPlayElectiveBranchIntent::DirectShot));
	TestFalse(TEXT("Wrong stage Writer fails"),
		WrongStageResult.bSuccess);
	TestTrue(TEXT("Wrong stage Writer atomic"),
		IsStateEqual(
			WrongStageResult.BeforeState,
			WrongStageResult.AfterState));

	FMatchPlayState DamagedHelper =
		Fixtures::MakeAwaitingCrossIntent(
			Fixtures::ECrossHelperPath::Selected);
	DamagedHelper.CurrentAttack.ActionPreparation
		.HelperCardId = NAME_None;
	const auto DamagedHelperResult =
		FMatchPlayCurrentAttackBranchIntentSelectionWriter::Select(
			DamagedHelper,
			Fixtures::MakeRequest(
				DamagedHelper,
				EMatchPlayElectiveBranchIntent::CrossHigh));
	TestFalse(TEXT("Damaged Helper Writer fails"),
		DamagedHelperResult.bSuccess);
	TestTrue(TEXT("Damaged Helper Writer atomic"),
		IsStateEqual(
			DamagedHelperResult.BeforeState,
			DamagedHelperResult.AfterState));
	return true;
}

#undef BRANCH_INTENT_FAILURE_TEST

#endif
