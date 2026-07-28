#include "MatchPlayCurrentAttackSelectionStateValidator.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

#include <type_traits>

namespace SelectionStateValidatorTests
{
	const FName CarrierId(TEXT("PlayerA.Carrier"));
	const FName MarkerId(TEXT("PlayerB.Marker"));
	const FName SkillId(TEXT("Skill.Selection"));
	const FName RunnerId(TEXT("PlayerA.Runner"));

	FMatchPlayCurrentAttackState MakeAwaitingCarrier()
	{
		FMatchPlayCurrentAttackState State;
		State.Phase = EMatchPlayCurrentAttackPhase::Resolution;
		State.SelectionStage =
			EMatchPlayCurrentAttackSelectionStage::AwaitingCarrier;
		return State;
	}

	FMatchPlayCurrentAttackState MakeAwaitingMarker()
	{
		FMatchPlayCurrentAttackState State = MakeAwaitingCarrier();
		State.SelectionStage =
			EMatchPlayCurrentAttackSelectionStage::AwaitingMarker;
		State.ActionPreparation.CarrierCardId = CarrierId;
		return State;
	}

	FMatchPlayCurrentAttackState MakeAwaitingSkill()
	{
		FMatchPlayCurrentAttackState State = MakeAwaitingMarker();
		State.SelectionStage =
			EMatchPlayCurrentAttackSelectionStage::AwaitingSkill;
		State.ActionPreparation.MarkerCardId = MarkerId;
		return State;
	}

	FMatchPlayCurrentAttackState MakeAwaitingRunner(
		const ESkillRuleType ActionType =
			ESkillRuleType::Cross)
	{
		FMatchPlayCurrentAttackState State = MakeAwaitingSkill();
		State.SelectionStage =
			EMatchPlayCurrentAttackSelectionStage::AwaitingRunner;
		State.ActionPreparation.SkillId = SkillId;
		State.ActionPreparation.ActionType = ActionType;
		State.ActionPoint = 5;
		State.bAttackerDeploymentFinished = true;
		State.bDefenderDeploymentFinished = true;
		State.CurrentLegalDeploymentSide =
			EInitialTurnOrderPlayer::None;
		return State;
	}

	FMatchPlayCurrentAttackState MakeAwaitingHelper(
		const ESkillRuleType ActionType =
			ESkillRuleType::Cross)
	{
		FMatchPlayCurrentAttackState State =
			MakeAwaitingRunner(ActionType);
		State.SelectionStage =
			EMatchPlayCurrentAttackSelectionStage::AwaitingHelper;
		State.ActionPreparation.RunnerCardId = RunnerId;
		return State;
	}

	FMatchPlayCurrentAttackState MakeReady(
		const ESkillRuleType ActionType =
			ESkillRuleType::LongShot)
	{
		FMatchPlayCurrentAttackState State;
		State.Phase = EMatchPlayCurrentAttackPhase::Resolution;
		State.SelectionStage =
			EMatchPlayCurrentAttackSelectionStage
				::ReadyForResolution;
		State.bHasSelectedAction = true;
		State.SelectedAction.CarrierCardId = CarrierId;
		State.SelectedAction.MarkerCardId = MarkerId;
		State.SelectedAction.SkillId = SkillId;
		State.SelectedAction.ActionType = ActionType;
		return State;
	}

	bool ExpectFailure(
		FAutomationTestBase& Test,
		const TCHAR* Context,
		const FMatchPlayCurrentAttackState& State,
		const EMatchPlayCurrentAttackSelectionStateValidationErrorCode
			ExpectedError)
	{
		const FMatchPlayCurrentAttackState Original = State;
		const auto Result =
			FMatchPlayCurrentAttackSelectionStateValidator::Validate(
				State);
		Test.TestFalse(
			*FString::Printf(TEXT("%s is not canonical"), Context),
			Result.bIsCanonical);
		Test.TestEqual(
			*FString::Printf(TEXT("%s exact error"), Context),
			Result.ErrorCode,
			ExpectedError);
		Test.TestTrue(
			*FString::Printf(TEXT("%s has diagnostics"), Context),
			!Result.ErrorMessage.IsEmpty());
		Test.TestTrue(
			*FString::Printf(TEXT("%s is read-only"), Context),
			FMatchPlayCurrentAttackState::StaticStruct()
				->CompareScriptStruct(&State, &Original, 0));
		return true;
	}
}

#define SELECTION_STATE_TEST(TestClass, TestName) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
		TestClass, \
		"FMCodex.CoreRules.MatchPlayCurrentAttackSelectionState." TestName, \
		EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

SELECTION_STATE_TEST(
	FSelectionStateContractTest,
	"DefaultsReflectionAndSignature")

bool FSelectionStateContractTest::RunTest(const FString& Parameters)
{
	const FMatchPlayCurrentAttackState State;
	const auto Result =
		FMatchPlayCurrentAttackSelectionStateValidator::Validate(State);
	TestTrue(TEXT("Default Deployment state canonical"),
		Result.bIsCanonical);
	TestEqual(TEXT("Default stage None"), State.SelectionStage,
		EMatchPlayCurrentAttackSelectionStage::None);
	TestTrue(TEXT("Default preparation empty"),
		State.ActionPreparation.CarrierCardId.IsNone());
	TestTrue(TEXT("Default preparation marker empty"),
		State.ActionPreparation.MarkerCardId.IsNone());
	TestTrue(TEXT("Default preparation skill empty"),
		State.ActionPreparation.SkillId.IsNone());
	TestEqual(TEXT("Default preparation action type empty"),
		State.ActionPreparation.ActionType,
		ESkillRuleType::None);
	TestTrue(TEXT("Default preparation runner empty"),
		State.ActionPreparation.RunnerCardId.IsNone());
	TestNotNull(TEXT("Preparation RunnerCardId reflected"),
		FMatchPlayCurrentAttackActionPreparationState::StaticStruct()
			->FindPropertyByName(TEXT("RunnerCardId")));
	TestTrue(TEXT("Default selected marker empty"),
		State.SelectedAction.MarkerCardId.IsNone());
	TestNotNull(TEXT("Stage reflected"),
		StaticEnum<EMatchPlayCurrentAttackSelectionStage>());
	TestNotNull(TEXT("Preparation reflected"),
		FMatchPlayCurrentAttackActionPreparationState::StaticStruct());
	TestNotNull(TEXT("Validation result reflected"),
		FMatchPlayCurrentAttackSelectionStateValidationResult
			::StaticStruct());
	using FValidateSignature =
		FMatchPlayCurrentAttackSelectionStateValidationResult (*)(
			const FMatchPlayCurrentAttackState&);
	TestTrue(TEXT("Single validator signature"),
		(std::is_same_v<
			decltype(
				&FMatchPlayCurrentAttackSelectionStateValidator
					::Validate),
			FValidateSignature>));
	TestEqual(TEXT("Stage None value stable"),
		static_cast<uint8>(
			EMatchPlayCurrentAttackSelectionStage::None),
		uint8{0});
	TestEqual(TEXT("AwaitingCarrier value stable"),
		static_cast<uint8>(
			EMatchPlayCurrentAttackSelectionStage
				::AwaitingCarrier),
		uint8{1});
	TestEqual(TEXT("AwaitingMarker value stable"),
		static_cast<uint8>(
			EMatchPlayCurrentAttackSelectionStage
				::AwaitingMarker),
		uint8{2});
	TestEqual(TEXT("AwaitingSkill value appended"),
		static_cast<uint8>(
			EMatchPlayCurrentAttackSelectionStage
				::AwaitingSkill),
		uint8{3});
	TestEqual(TEXT("AwaitingRunner value appended"),
		static_cast<uint8>(
			EMatchPlayCurrentAttackSelectionStage
				::AwaitingRunner),
		uint8{4});
	TestEqual(TEXT("ReadyForResolution value appended"),
		static_cast<uint8>(
			EMatchPlayCurrentAttackSelectionStage
				::ReadyForResolution),
		uint8{5});
	TestEqual(TEXT("AwaitingHelper value appended"),
		static_cast<uint8>(
			EMatchPlayCurrentAttackSelectionStage
				::AwaitingHelper),
		uint8{6});
	return true;
}

SELECTION_STATE_TEST(
	FSelectionStateAwaitingSkillTest,
	"AwaitingSkillCanonical")

bool FSelectionStateAwaitingSkillTest::RunTest(
	const FString& Parameters)
{
	const auto Result =
		FMatchPlayCurrentAttackSelectionStateValidator::Validate(
			SelectionStateValidatorTests::MakeAwaitingSkill());
	TestTrue(TEXT("AwaitingSkill canonical"), Result.bIsCanonical);
	return true;
}

SELECTION_STATE_TEST(
	FSelectionStateAwaitingCarrierTest,
	"AwaitingCarrierCanonical")

bool FSelectionStateAwaitingCarrierTest::RunTest(
	const FString& Parameters)
{
	const auto Result =
		FMatchPlayCurrentAttackSelectionStateValidator::Validate(
			SelectionStateValidatorTests::MakeAwaitingCarrier());
	TestTrue(TEXT("AwaitingCarrier canonical"), Result.bIsCanonical);
	return true;
}

SELECTION_STATE_TEST(
	FSelectionStateAwaitingMarkerTest,
	"AwaitingMarkerCanonical")

bool FSelectionStateAwaitingMarkerTest::RunTest(
	const FString& Parameters)
{
	const auto Result =
		FMatchPlayCurrentAttackSelectionStateValidator::Validate(
			SelectionStateValidatorTests::MakeAwaitingMarker());
	TestTrue(TEXT("AwaitingMarker canonical"), Result.bIsCanonical);
	return true;
}

SELECTION_STATE_TEST(
	FSelectionStateDeploymentStageTest,
	"DeploymentRejectsSelectionStage")

bool FSelectionStateDeploymentStageTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayCurrentAttackState State;
	State.SelectionStage =
		EMatchPlayCurrentAttackSelectionStage::AwaitingCarrier;
	return SelectionStateValidatorTests::ExpectFailure(
		*this,
		TEXT("Deployment stage"),
		State,
		EMatchPlayCurrentAttackSelectionStateValidationErrorCode
			::SelectionStageDoesNotMatchPhase);
}

SELECTION_STATE_TEST(
	FSelectionStateResolutionNoneTest,
	"ResolutionRejectsNoneStage")

bool FSelectionStateResolutionNoneTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayCurrentAttackState State;
	State.Phase = EMatchPlayCurrentAttackPhase::Resolution;
	return SelectionStateValidatorTests::ExpectFailure(
		*this,
		TEXT("Resolution None"),
		State,
		EMatchPlayCurrentAttackSelectionStateValidationErrorCode
			::SelectionStageDoesNotMatchPhase);
}

SELECTION_STATE_TEST(
	FSelectionStateAwaitingCarrierPayloadTest,
	"AwaitingCarrierRejectsPreparationCarrier")

bool FSelectionStateAwaitingCarrierPayloadTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayCurrentAttackState State =
		SelectionStateValidatorTests::MakeAwaitingCarrier();
	State.ActionPreparation.CarrierCardId =
		SelectionStateValidatorTests::CarrierId;
	return SelectionStateValidatorTests::ExpectFailure(
		*this,
		TEXT("AwaitingCarrier payload"),
		State,
		EMatchPlayCurrentAttackSelectionStateValidationErrorCode
			::UnexpectedPreparationCarrier);
}

SELECTION_STATE_TEST(
	FSelectionStateAwaitingCarrierMarkerTest,
	"AwaitingCarrierRejectsPreparationMarker")

bool FSelectionStateAwaitingCarrierMarkerTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayCurrentAttackState State =
		SelectionStateValidatorTests::MakeAwaitingCarrier();
	State.ActionPreparation.MarkerCardId =
		SelectionStateValidatorTests::MarkerId;
	return SelectionStateValidatorTests::ExpectFailure(
		*this,
		TEXT("AwaitingCarrier marker"),
		State,
		EMatchPlayCurrentAttackSelectionStateValidationErrorCode
			::UnexpectedPreparationMarker);
}

SELECTION_STATE_TEST(
	FSelectionStateAwaitingMarkerPayloadTest,
	"AwaitingMarkerRequiresPreparationCarrier")

bool FSelectionStateAwaitingMarkerPayloadTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayCurrentAttackState State =
		SelectionStateValidatorTests::MakeAwaitingMarker();
	State.ActionPreparation.CarrierCardId = NAME_None;
	return SelectionStateValidatorTests::ExpectFailure(
		*this,
		TEXT("AwaitingMarker missing carrier"),
		State,
		EMatchPlayCurrentAttackSelectionStateValidationErrorCode
			::MissingPreparationCarrier);
}

SELECTION_STATE_TEST(
	FSelectionStateAwaitingMarkerMarkerTest,
	"AwaitingMarkerRejectsPreparationMarker")

bool FSelectionStateAwaitingMarkerMarkerTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayCurrentAttackState State =
		SelectionStateValidatorTests::MakeAwaitingMarker();
	State.ActionPreparation.MarkerCardId =
		SelectionStateValidatorTests::MarkerId;
	return SelectionStateValidatorTests::ExpectFailure(
		*this,
		TEXT("AwaitingMarker marker"),
		State,
		EMatchPlayCurrentAttackSelectionStateValidationErrorCode
			::UnexpectedPreparationMarker);
}

SELECTION_STATE_TEST(
	FSelectionStateAwaitingSkillCarrierTest,
	"AwaitingSkillRequiresPreparationCarrier")

bool FSelectionStateAwaitingSkillCarrierTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayCurrentAttackState State =
		SelectionStateValidatorTests::MakeAwaitingSkill();
	State.ActionPreparation.CarrierCardId = NAME_None;
	return SelectionStateValidatorTests::ExpectFailure(
		*this,
		TEXT("AwaitingSkill missing carrier"),
		State,
		EMatchPlayCurrentAttackSelectionStateValidationErrorCode
			::MissingPreparationCarrier);
}

SELECTION_STATE_TEST(
	FSelectionStateAwaitingSkillMarkerTest,
	"AwaitingSkillRequiresPreparationMarker")

bool FSelectionStateAwaitingSkillMarkerTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayCurrentAttackState State =
		SelectionStateValidatorTests::MakeAwaitingSkill();
	State.ActionPreparation.MarkerCardId = NAME_None;
	return SelectionStateValidatorTests::ExpectFailure(
		*this,
		TEXT("AwaitingSkill missing marker"),
		State,
		EMatchPlayCurrentAttackSelectionStateValidationErrorCode
			::MissingPreparationMarker);
}

SELECTION_STATE_TEST(
	FSelectionStateDeploymentMarkerTest,
	"DeploymentRejectsPreparationMarker")

bool FSelectionStateDeploymentMarkerTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayCurrentAttackState State;
	State.ActionPreparation.MarkerCardId =
		SelectionStateValidatorTests::MarkerId;
	return SelectionStateValidatorTests::ExpectFailure(
		*this,
		TEXT("Deployment marker"),
		State,
		EMatchPlayCurrentAttackSelectionStateValidationErrorCode
			::UnexpectedPreparationMarker);
}

SELECTION_STATE_TEST(
	FSelectionStateSelectedPresenceTest,
	"SelectedActionPresenceRejected")

bool FSelectionStateSelectedPresenceTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayCurrentAttackState State =
		SelectionStateValidatorTests::MakeAwaitingCarrier();
	State.bHasSelectedAction = true;
	return SelectionStateValidatorTests::ExpectFailure(
		*this,
		TEXT("Selected presence"),
		State,
		EMatchPlayCurrentAttackSelectionStateValidationErrorCode
			::SelectedActionUnexpectedlyPresent);
}

SELECTION_STATE_TEST(
	FSelectionStateSelectedPayloadTest,
	"SelectedActionPayloadRejected")

bool FSelectionStateSelectedPayloadTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayCurrentAttackState State =
		SelectionStateValidatorTests::MakeAwaitingCarrier();
	State.SelectedAction.SkillId = TEXT("Skill.Legacy");
	return SelectionStateValidatorTests::ExpectFailure(
		*this,
		TEXT("Selected payload"),
		State,
		EMatchPlayCurrentAttackSelectionStateValidationErrorCode
			::SelectedActionPayloadNotEmpty);
}

SELECTION_STATE_TEST(
	FSelectionStateDuplicateCarrierTest,
	"DuplicateCarrierAuthorityRejectedFirst")

bool FSelectionStateDuplicateCarrierTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayCurrentAttackState State =
		SelectionStateValidatorTests::MakeAwaitingMarker();
	State.SelectedAction.CarrierCardId =
		SelectionStateValidatorTests::CarrierId;
	return SelectionStateValidatorTests::ExpectFailure(
		*this,
		TEXT("Duplicate carrier"),
		State,
		EMatchPlayCurrentAttackSelectionStateValidationErrorCode
			::DuplicateCarrierAuthority);
}

SELECTION_STATE_TEST(
	FSelectionStateUnknownStageTest,
	"UnknownStageRejected")

bool FSelectionStateUnknownStageTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayCurrentAttackState State =
		SelectionStateValidatorTests::MakeAwaitingCarrier();
	State.SelectionStage =
		static_cast<EMatchPlayCurrentAttackSelectionStage>(255);
	return SelectionStateValidatorTests::ExpectFailure(
		*this,
		TEXT("Unknown stage"),
		State,
		EMatchPlayCurrentAttackSelectionStateValidationErrorCode
			::UnsupportedSelectionStage);
}

SELECTION_STATE_TEST(
	FSelectionStateExtendedCanonicalTest,
	"AwaitingRunnerAwaitingHelperAndReadyCanonical")

bool FSelectionStateExtendedCanonicalTest::RunTest(
	const FString& Parameters)
{
	const auto Runner =
		FMatchPlayCurrentAttackSelectionStateValidator::Validate(
			SelectionStateValidatorTests::MakeAwaitingRunner());
	TestTrue(TEXT("AwaitingRunner canonical"), Runner.bIsCanonical);
	const auto Helper =
		FMatchPlayCurrentAttackSelectionStateValidator::Validate(
			SelectionStateValidatorTests::MakeAwaitingHelper());
	TestTrue(TEXT("AwaitingHelper canonical"), Helper.bIsCanonical);
	const auto LongShot =
		FMatchPlayCurrentAttackSelectionStateValidator::Validate(
			SelectionStateValidatorTests::MakeReady());
	TestTrue(TEXT("Ready LongShot canonical"), LongShot.bIsCanonical);
	const auto CutInside =
		FMatchPlayCurrentAttackSelectionStateValidator::Validate(
			SelectionStateValidatorTests::MakeReady(
				ESkillRuleType::CutInsideShot));
	TestTrue(
		TEXT("Ready CutInside canonical"),
		CutInside.bIsCanonical);
	return true;
}

SELECTION_STATE_TEST(
	FSelectionStateAwaitingSkillExtendedPayloadTest,
	"AwaitingSkillRejectsSkillAndActionType")

bool FSelectionStateAwaitingSkillExtendedPayloadTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayCurrentAttackState SkillState =
		SelectionStateValidatorTests::MakeAwaitingSkill();
	SkillState.ActionPreparation.SkillId =
		SelectionStateValidatorTests::SkillId;
	SelectionStateValidatorTests::ExpectFailure(
		*this,
		TEXT("AwaitingSkill with skill"),
		SkillState,
		EMatchPlayCurrentAttackSelectionStateValidationErrorCode
			::UnexpectedPreparationSkill);

	FMatchPlayCurrentAttackState ActionState =
		SelectionStateValidatorTests::MakeAwaitingSkill();
	ActionState.ActionPreparation.ActionType =
		ESkillRuleType::Cross;
	return SelectionStateValidatorTests::ExpectFailure(
		*this,
		TEXT("AwaitingSkill with action type"),
		ActionState,
		EMatchPlayCurrentAttackSelectionStateValidationErrorCode
			::UnexpectedPreparationActionType);
}

SELECTION_STATE_TEST(
	FSelectionStateAwaitingRunnerCorruptionTest,
	"AwaitingRunnerRejectsMissingAndNoRunnerTypes")

bool FSelectionStateAwaitingRunnerCorruptionTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayCurrentAttackState MissingSkill =
		SelectionStateValidatorTests::MakeAwaitingRunner();
	MissingSkill.ActionPreparation.SkillId = NAME_None;
	SelectionStateValidatorTests::ExpectFailure(
		*this,
		TEXT("AwaitingRunner missing skill"),
		MissingSkill,
		EMatchPlayCurrentAttackSelectionStateValidationErrorCode
			::MissingPreparationSkill);

	FMatchPlayCurrentAttackState MissingCarrier =
		SelectionStateValidatorTests::MakeAwaitingRunner();
	MissingCarrier.ActionPreparation.CarrierCardId = NAME_None;
	SelectionStateValidatorTests::ExpectFailure(
		*this,
		TEXT("AwaitingRunner missing carrier"),
		MissingCarrier,
		EMatchPlayCurrentAttackSelectionStateValidationErrorCode
			::MissingPreparationCarrier);

	FMatchPlayCurrentAttackState MissingMarker =
		SelectionStateValidatorTests::MakeAwaitingRunner();
	MissingMarker.ActionPreparation.MarkerCardId = NAME_None;
	SelectionStateValidatorTests::ExpectFailure(
		*this,
		TEXT("AwaitingRunner missing marker"),
		MissingMarker,
		EMatchPlayCurrentAttackSelectionStateValidationErrorCode
			::MissingPreparationMarker);

	FMatchPlayCurrentAttackState MissingType =
		SelectionStateValidatorTests::MakeAwaitingRunner();
	MissingType.ActionPreparation.ActionType =
		ESkillRuleType::None;
	SelectionStateValidatorTests::ExpectFailure(
		*this,
		TEXT("AwaitingRunner missing type"),
		MissingType,
		EMatchPlayCurrentAttackSelectionStateValidationErrorCode
			::MissingPreparationActionType);

	FMatchPlayCurrentAttackState LongShot =
		SelectionStateValidatorTests::MakeAwaitingRunner(
			ESkillRuleType::LongShot);
	SelectionStateValidatorTests::ExpectFailure(
		*this,
		TEXT("AwaitingRunner LongShot"),
		LongShot,
		EMatchPlayCurrentAttackSelectionStateValidationErrorCode
			::ActionTypeDoesNotMatchSelectionStage);

	FMatchPlayCurrentAttackState CutInside =
		SelectionStateValidatorTests::MakeAwaitingRunner(
			ESkillRuleType::CutInsideShot);
	SelectionStateValidatorTests::ExpectFailure(
		*this,
		TEXT("AwaitingRunner CutInside"),
		CutInside,
		EMatchPlayCurrentAttackSelectionStateValidationErrorCode
			::ActionTypeDoesNotMatchSelectionStage);

	FMatchPlayCurrentAttackState Prefilled =
		SelectionStateValidatorTests::MakeAwaitingRunner();
	Prefilled.ActionPreparation.RunnerCardId =
		SelectionStateValidatorTests::RunnerId;
	SelectionStateValidatorTests::ExpectFailure(
		*this,
		TEXT("AwaitingRunner prefilled runner"),
		Prefilled,
		EMatchPlayCurrentAttackSelectionStateValidationErrorCode
			::UnexpectedPreparationRunner);

	FMatchPlayCurrentAttackState Incomplete =
		SelectionStateValidatorTests::MakeAwaitingRunner();
	Incomplete.bAttackerDeploymentFinished = false;
	SelectionStateValidatorTests::ExpectFailure(
		*this,
		TEXT("AwaitingRunner incomplete deployment"),
		Incomplete,
		EMatchPlayCurrentAttackSelectionStateValidationErrorCode
			::ResolutionDeploymentNotComplete);

	FMatchPlayCurrentAttackState Selected =
		SelectionStateValidatorTests::MakeAwaitingRunner();
	Selected.SelectedAction.SkillId =
		SelectionStateValidatorTests::SkillId;
	return SelectionStateValidatorTests::ExpectFailure(
		*this,
		TEXT("AwaitingRunner selected payload"),
		Selected,
		EMatchPlayCurrentAttackSelectionStateValidationErrorCode
			::SelectedActionPayloadNotEmpty);
}

SELECTION_STATE_TEST(
	FSelectionStateAwaitingHelperCorruptionTest,
	"AwaitingHelperRejectsCorruption")

bool FSelectionStateAwaitingHelperCorruptionTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayCurrentAttackState MissingRunner =
		SelectionStateValidatorTests::MakeAwaitingHelper();
	MissingRunner.ActionPreparation.RunnerCardId = NAME_None;
	SelectionStateValidatorTests::ExpectFailure(
		*this,
		TEXT("AwaitingHelper missing runner"),
		MissingRunner,
		EMatchPlayCurrentAttackSelectionStateValidationErrorCode
			::MissingPreparationRunner);

	FMatchPlayCurrentAttackState LongShot =
		SelectionStateValidatorTests::MakeAwaitingHelper(
			ESkillRuleType::LongShot);
	SelectionStateValidatorTests::ExpectFailure(
		*this,
		TEXT("AwaitingHelper LongShot"),
		LongShot,
		EMatchPlayCurrentAttackSelectionStateValidationErrorCode
			::ActionTypeDoesNotMatchSelectionStage);

	FMatchPlayCurrentAttackState Selected =
		SelectionStateValidatorTests::MakeAwaitingHelper();
	Selected.bHasSelectedAction = true;
	SelectionStateValidatorTests::ExpectFailure(
		*this,
		TEXT("AwaitingHelper selected action"),
		Selected,
		EMatchPlayCurrentAttackSelectionStateValidationErrorCode
			::SelectedActionUnexpectedlyPresent);

	FMatchPlayCurrentAttackState WrongPhase =
		SelectionStateValidatorTests::MakeAwaitingHelper();
	WrongPhase.Phase = EMatchPlayCurrentAttackPhase::Deployment;
	return SelectionStateValidatorTests::ExpectFailure(
		*this,
		TEXT("AwaitingHelper wrong phase"),
		WrongPhase,
		EMatchPlayCurrentAttackSelectionStateValidationErrorCode
			::SelectionStageDoesNotMatchPhase);
}

SELECTION_STATE_TEST(
	FSelectionStateReadyCorruptionTest,
	"ReadyRejectsRunnerPreparationAndMissingMarker")

bool FSelectionStateReadyCorruptionTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayCurrentAttackState Runner =
		SelectionStateValidatorTests::MakeReady(
			ESkillRuleType::Cross);
	SelectionStateValidatorTests::ExpectFailure(
		*this,
		TEXT("Ready runner skill"),
		Runner,
		EMatchPlayCurrentAttackSelectionStateValidationErrorCode
			::ActionTypeDoesNotMatchSelectionStage);

	FMatchPlayCurrentAttackState Preparation =
		SelectionStateValidatorTests::MakeReady();
	Preparation.ActionPreparation.SkillId =
		SelectionStateValidatorTests::SkillId;
	SelectionStateValidatorTests::ExpectFailure(
		*this,
		TEXT("Ready preparation coexist"),
		Preparation,
		EMatchPlayCurrentAttackSelectionStateValidationErrorCode
			::PreparationAndSelectedActionCoexist);

	FMatchPlayCurrentAttackState MissingMarker =
		SelectionStateValidatorTests::MakeReady();
	MissingMarker.SelectedAction.MarkerCardId = NAME_None;
	SelectionStateValidatorTests::ExpectFailure(
		*this,
		TEXT("Ready missing marker"),
		MissingMarker,
		EMatchPlayCurrentAttackSelectionStateValidationErrorCode
			::MissingSelectedActionMarker);

	FMatchPlayCurrentAttackState MissingPresence =
		SelectionStateValidatorTests::MakeReady();
	MissingPresence.bHasSelectedAction = false;
	return SelectionStateValidatorTests::ExpectFailure(
		*this,
		TEXT("Ready missing selected presence"),
		MissingPresence,
		EMatchPlayCurrentAttackSelectionStateValidationErrorCode
			::MissingSelectedAction);
}

SELECTION_STATE_TEST(
	FSelectionStateReadyPhaseMismatchTest,
	"ReadyRejectsDeploymentPhase")

bool FSelectionStateReadyPhaseMismatchTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayCurrentAttackState State =
		SelectionStateValidatorTests::MakeReady();
	State.Phase = EMatchPlayCurrentAttackPhase::Deployment;
	return SelectionStateValidatorTests::ExpectFailure(
		*this,
		TEXT("Ready deployment phase"),
		State,
		EMatchPlayCurrentAttackSelectionStateValidationErrorCode
			::SelectionStageDoesNotMatchPhase);
}

#undef SELECTION_STATE_TEST

#endif
