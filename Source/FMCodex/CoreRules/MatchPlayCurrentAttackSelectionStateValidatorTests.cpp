#include "MatchPlayCurrentAttackSelectionStateValidator.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

#include <type_traits>

namespace SelectionStateValidatorTests
{
	const FName CarrierId(TEXT("PlayerA.Carrier"));
	const FName MarkerId(TEXT("PlayerB.Marker"));

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

#undef SELECTION_STATE_TEST

#endif
