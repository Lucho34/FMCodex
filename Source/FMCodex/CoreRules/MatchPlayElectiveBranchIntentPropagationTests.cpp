#include "MatchPlayBoundActionParticipantNormalizationQuery.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "MatchPlayCurrentAttackBranchIntentSelectionTestFixtures.h"
#include "MatchPlayCurrentAttackReadyForResolutionValidator.h"
#include "MatchPlayCurrentAttackResolutionBinding.h"
#include "Misc/AutomationTest.h"

namespace ElectiveBranchIntentPropagationTests
{
	namespace Fixtures =
		FMCodex::Tests::MatchPlayCurrentAttackBranchIntentSelection;
	namespace HelperFixtures =
		FMCodex::Tests::MatchPlayCurrentAttackHelperSelection;

	struct FCase
	{
		ESkillRuleType ActionType = ESkillRuleType::None;
		EMatchPlayElectiveBranchIntent Intent =
			EMatchPlayElectiveBranchIntent::None;
	};

	FMatchPlayState MakeReady(const FCase& Case)
	{
		if (Case.ActionType == ESkillRuleType::PassControl
			|| Case.ActionType == ESkillRuleType::ThroughBall)
		{
			return HelperFixtures::MakeReadyState(
				Case.ActionType,
				true);
		}
		return Fixtures::MakeReadyState(
			Case.ActionType,
			Case.Intent,
			true);
	}

	const FCase ValidCases[] = {
		{ESkillRuleType::LongShot,
			EMatchPlayElectiveBranchIntent::DirectShot},
		{ESkillRuleType::LongShot,
			EMatchPlayElectiveBranchIntent::DeadCorner},
		{ESkillRuleType::CutInsideShot,
			EMatchPlayElectiveBranchIntent::DirectShot},
		{ESkillRuleType::CutInsideShot,
			EMatchPlayElectiveBranchIntent::DeadCorner},
		{ESkillRuleType::Cross,
			EMatchPlayElectiveBranchIntent::CrossHigh},
		{ESkillRuleType::Cross,
			EMatchPlayElectiveBranchIntent::CrossLow},
		{ESkillRuleType::PassControl,
			EMatchPlayElectiveBranchIntent::None},
		{ESkillRuleType::ThroughBall,
			EMatchPlayElectiveBranchIntent::None}
	};

	struct FReadyValidationCase
	{
		const TCHAR* CaseLabel = TEXT("");
		ESkillRuleType ActionType = ESkillRuleType::None;
		EMatchPlayElectiveBranchIntent SeedIntent =
			EMatchPlayElectiveBranchIntent::None;
		EMatchPlayElectiveBranchIntent Intent =
			EMatchPlayElectiveBranchIntent::None;
		bool bExpectedValid = false;
		EMatchPlayCurrentAttackReadyValidationErrorCode ExpectedError =
			EMatchPlayCurrentAttackReadyValidationErrorCode::None;
	};

	const FReadyValidationCase ReadyValidationCases[] = {
		{TEXT("Ready.LongShot.DirectShot.Valid"),
			ESkillRuleType::LongShot,
			EMatchPlayElectiveBranchIntent::DirectShot,
			EMatchPlayElectiveBranchIntent::DirectShot,
			true,
			EMatchPlayCurrentAttackReadyValidationErrorCode::None},
		{TEXT("Ready.LongShot.DeadCorner.Valid"),
			ESkillRuleType::LongShot,
			EMatchPlayElectiveBranchIntent::DeadCorner,
			EMatchPlayElectiveBranchIntent::DeadCorner,
			true,
			EMatchPlayCurrentAttackReadyValidationErrorCode::None},
		{TEXT("Ready.CutInsideShot.DirectShot.Valid"),
			ESkillRuleType::CutInsideShot,
			EMatchPlayElectiveBranchIntent::DirectShot,
			EMatchPlayElectiveBranchIntent::DirectShot,
			true,
			EMatchPlayCurrentAttackReadyValidationErrorCode::None},
		{TEXT("Ready.CutInsideShot.DeadCorner.Valid"),
			ESkillRuleType::CutInsideShot,
			EMatchPlayElectiveBranchIntent::DeadCorner,
			EMatchPlayElectiveBranchIntent::DeadCorner,
			true,
			EMatchPlayCurrentAttackReadyValidationErrorCode::None},
		{TEXT("Ready.Cross.CrossHigh.Valid"),
			ESkillRuleType::Cross,
			EMatchPlayElectiveBranchIntent::CrossHigh,
			EMatchPlayElectiveBranchIntent::CrossHigh,
			true,
			EMatchPlayCurrentAttackReadyValidationErrorCode::None},
		{TEXT("Ready.Cross.CrossLow.Valid"),
			ESkillRuleType::Cross,
			EMatchPlayElectiveBranchIntent::CrossLow,
			EMatchPlayElectiveBranchIntent::CrossLow,
			true,
			EMatchPlayCurrentAttackReadyValidationErrorCode::None},
		{TEXT("Ready.PassControl.None.Valid"),
			ESkillRuleType::PassControl,
			EMatchPlayElectiveBranchIntent::None,
			EMatchPlayElectiveBranchIntent::None,
			true,
			EMatchPlayCurrentAttackReadyValidationErrorCode::None},
		{TEXT("Ready.ThroughBall.None.Valid"),
			ESkillRuleType::ThroughBall,
			EMatchPlayElectiveBranchIntent::None,
			EMatchPlayElectiveBranchIntent::None,
			true,
			EMatchPlayCurrentAttackReadyValidationErrorCode::None},
		{TEXT("Ready.LongShot.None.Invalid"),
			ESkillRuleType::LongShot,
			EMatchPlayElectiveBranchIntent::DirectShot,
			EMatchPlayElectiveBranchIntent::None,
			false,
			EMatchPlayCurrentAttackReadyValidationErrorCode
				::InvalidElectiveBranchIntent},
		{TEXT("Ready.LongShot.CrossHigh.Invalid"),
			ESkillRuleType::LongShot,
			EMatchPlayElectiveBranchIntent::DirectShot,
			EMatchPlayElectiveBranchIntent::CrossHigh,
			false,
			EMatchPlayCurrentAttackReadyValidationErrorCode
				::InvalidElectiveBranchIntent},
		{TEXT("Ready.LongShot.CrossLow.Invalid"),
			ESkillRuleType::LongShot,
			EMatchPlayElectiveBranchIntent::DirectShot,
			EMatchPlayElectiveBranchIntent::CrossLow,
			false,
			EMatchPlayCurrentAttackReadyValidationErrorCode
				::InvalidElectiveBranchIntent},
		{TEXT("Ready.CutInsideShot.None.Invalid"),
			ESkillRuleType::CutInsideShot,
			EMatchPlayElectiveBranchIntent::DirectShot,
			EMatchPlayElectiveBranchIntent::None,
			false,
			EMatchPlayCurrentAttackReadyValidationErrorCode
				::InvalidElectiveBranchIntent},
		{TEXT("Ready.CutInsideShot.CrossHigh.Invalid"),
			ESkillRuleType::CutInsideShot,
			EMatchPlayElectiveBranchIntent::DirectShot,
			EMatchPlayElectiveBranchIntent::CrossHigh,
			false,
			EMatchPlayCurrentAttackReadyValidationErrorCode
				::InvalidElectiveBranchIntent},
		{TEXT("Ready.CutInsideShot.CrossLow.Invalid"),
			ESkillRuleType::CutInsideShot,
			EMatchPlayElectiveBranchIntent::DirectShot,
			EMatchPlayElectiveBranchIntent::CrossLow,
			false,
			EMatchPlayCurrentAttackReadyValidationErrorCode
				::InvalidElectiveBranchIntent},
		{TEXT("Ready.Cross.None.Invalid"),
			ESkillRuleType::Cross,
			EMatchPlayElectiveBranchIntent::CrossHigh,
			EMatchPlayElectiveBranchIntent::None,
			false,
			EMatchPlayCurrentAttackReadyValidationErrorCode
				::InvalidElectiveBranchIntent},
		{TEXT("Ready.Cross.DirectShot.Invalid"),
			ESkillRuleType::Cross,
			EMatchPlayElectiveBranchIntent::CrossHigh,
			EMatchPlayElectiveBranchIntent::DirectShot,
			false,
			EMatchPlayCurrentAttackReadyValidationErrorCode
				::InvalidElectiveBranchIntent},
		{TEXT("Ready.Cross.DeadCorner.Invalid"),
			ESkillRuleType::Cross,
			EMatchPlayElectiveBranchIntent::CrossHigh,
			EMatchPlayElectiveBranchIntent::DeadCorner,
			false,
			EMatchPlayCurrentAttackReadyValidationErrorCode
				::InvalidElectiveBranchIntent},
		{TEXT("Ready.PassControl.DirectShot.Invalid"),
			ESkillRuleType::PassControl,
			EMatchPlayElectiveBranchIntent::None,
			EMatchPlayElectiveBranchIntent::DirectShot,
			false,
			EMatchPlayCurrentAttackReadyValidationErrorCode
				::InvalidElectiveBranchIntent},
		{TEXT("Ready.PassControl.DeadCorner.Invalid"),
			ESkillRuleType::PassControl,
			EMatchPlayElectiveBranchIntent::None,
			EMatchPlayElectiveBranchIntent::DeadCorner,
			false,
			EMatchPlayCurrentAttackReadyValidationErrorCode
				::InvalidElectiveBranchIntent},
		{TEXT("Ready.PassControl.CrossHigh.Invalid"),
			ESkillRuleType::PassControl,
			EMatchPlayElectiveBranchIntent::None,
			EMatchPlayElectiveBranchIntent::CrossHigh,
			false,
			EMatchPlayCurrentAttackReadyValidationErrorCode
				::InvalidElectiveBranchIntent},
		{TEXT("Ready.PassControl.CrossLow.Invalid"),
			ESkillRuleType::PassControl,
			EMatchPlayElectiveBranchIntent::None,
			EMatchPlayElectiveBranchIntent::CrossLow,
			false,
			EMatchPlayCurrentAttackReadyValidationErrorCode
				::InvalidElectiveBranchIntent},
		{TEXT("Ready.ThroughBall.DirectShot.Invalid"),
			ESkillRuleType::ThroughBall,
			EMatchPlayElectiveBranchIntent::None,
			EMatchPlayElectiveBranchIntent::DirectShot,
			false,
			EMatchPlayCurrentAttackReadyValidationErrorCode
				::InvalidElectiveBranchIntent},
		{TEXT("Ready.ThroughBall.DeadCorner.Invalid"),
			ESkillRuleType::ThroughBall,
			EMatchPlayElectiveBranchIntent::None,
			EMatchPlayElectiveBranchIntent::DeadCorner,
			false,
			EMatchPlayCurrentAttackReadyValidationErrorCode
				::InvalidElectiveBranchIntent},
		{TEXT("Ready.ThroughBall.CrossHigh.Invalid"),
			ESkillRuleType::ThroughBall,
			EMatchPlayElectiveBranchIntent::None,
			EMatchPlayElectiveBranchIntent::CrossHigh,
			false,
			EMatchPlayCurrentAttackReadyValidationErrorCode
				::InvalidElectiveBranchIntent},
		{TEXT("Ready.ThroughBall.CrossLow.Invalid"),
			ESkillRuleType::ThroughBall,
			EMatchPlayElectiveBranchIntent::None,
			EMatchPlayElectiveBranchIntent::CrossLow,
			false,
			EMatchPlayCurrentAttackReadyValidationErrorCode
				::InvalidElectiveBranchIntent}
	};

	struct FReadyHelperDamageCase
	{
		const TCHAR* CaseLabel = TEXT("");
		ESkillRuleType ActionType = ESkillRuleType::None;
		EMatchPlayElectiveBranchIntent Intent =
			EMatchPlayElectiveBranchIntent::None;
		bool bBaseHasHelper = false;
		bool bExpectedValid = false;
		EMatchPlayCurrentAttackReadyValidationErrorCode ExpectedError =
			EMatchPlayCurrentAttackReadyValidationErrorCode::None;
	};

	const FReadyHelperDamageCase ReadyHelperDamageCases[] = {
		{TEXT("Ready.Cross.CrossHigh.HelperPresentMissingId.Invalid"),
			ESkillRuleType::Cross,
			EMatchPlayElectiveBranchIntent::CrossHigh,
			true,
			false,
			EMatchPlayCurrentAttackReadyValidationErrorCode
				::InvalidSelectionState},
		{TEXT("Ready.Cross.CrossLow.HelperAbsentWithId.Invalid"),
			ESkillRuleType::Cross,
			EMatchPlayElectiveBranchIntent::CrossLow,
			false,
			false,
			EMatchPlayCurrentAttackReadyValidationErrorCode
				::InvalidSelectionState}
	};
}

#define INTENT_PROPAGATION_TEST(TestClass, TestName) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
		TestClass, \
		"FMCodex.CoreRules.MatchPlayElectiveBranchIntent." TestName, \
		EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

INTENT_PROPAGATION_TEST(
	FElectiveBranchIntentBindingTest,
	"BindingPropagation")

bool FElectiveBranchIntentBindingTest::RunTest(
	const FString& Parameters)
{
	using namespace ElectiveBranchIntentPropagationTests;
	for (const FCase& Case : ValidCases)
	{
		const FMatchPlayState State = MakeReady(Case);
		const FMatchPlayState Original = State;
		const auto Result =
			FMatchPlayCurrentAttackResolutionBinding::Query(
				State,
				State.CurrentAttack.AttackSequence);
		TestTrue(TEXT("Binding succeeds"), Result.bSuccess);
		TestEqual(TEXT("Binding preserves action type"),
			Result.Binding.ActionType,
			Case.ActionType);
		TestEqual(TEXT("Binding preserves Intent"),
			Result.Binding.ElectiveBranchIntent,
			Case.Intent);
		TestTrue(TEXT("Binding does not mutate state"),
			Fixtures::AreStatesEqual(State, Original));
	}

	const FMatchPlayState Awaiting =
		Fixtures::MakeAwaitingShotIntent(
			ESkillRuleType::LongShot);
	const auto Incomplete =
		FMatchPlayCurrentAttackResolutionBinding::Query(
			Awaiting,
			Awaiting.CurrentAttack.AttackSequence);
	TestFalse(TEXT("Binding rejects AwaitingBranchIntent"),
		Incomplete.bSuccess);
	TestEqual(TEXT("Binding incomplete exact error"),
		Incomplete.ErrorCode,
		EMatchPlayCurrentAttackResolutionBindingErrorCode
			::SelectionNotComplete);
	return true;
}

INTENT_PROPAGATION_TEST(
	FElectiveBranchIntentNormalizationTest,
	"ParticipantNormalizationPropagation")

bool FElectiveBranchIntentNormalizationTest::RunTest(
	const FString& Parameters)
{
	using namespace ElectiveBranchIntentPropagationTests;
	for (const FCase& Case : ValidCases)
	{
		const FMatchPlayState State = MakeReady(Case);
		FMatchPlayBoundActionParticipantNormalizationRequest Request;
		Request.AttackSequence =
			State.CurrentAttack.AttackSequence;
		const auto Result =
			FMatchPlayBoundActionParticipantNormalizationQuery::Query(
				State,
				Request);
		TestTrue(TEXT("Normalization succeeds"),
			Result.bSuccess);
		TestEqual(TEXT("Normalization result preserves Intent"),
			Result.ElectiveBranchIntent,
			Case.Intent);
		TestEqual(TEXT("Normalization bundle preserves Intent"),
			Result.Bundle.ElectiveBranchIntent,
			Case.Intent);
		TestEqual(TEXT("Nested Binding preserves Intent"),
			Result.Bundle.Binding.ElectiveBranchIntent,
			Case.Intent);
	}
	return true;
}

INTENT_PROPAGATION_TEST(
	FElectiveBranchIntentReadyValidationTest,
	"ReadyValidationMatrix")

bool FElectiveBranchIntentReadyValidationTest::RunTest(
	const FString& Parameters)
{
	using namespace ElectiveBranchIntentPropagationTests;
	TSet<FString> SeenCaseLabels;
	for (const FReadyValidationCase& Case : ReadyValidationCases)
	{
		const FString CaseLabel(Case.CaseLabel);
		const bool bUniqueLabel =
			!SeenCaseLabels.Contains(CaseLabel);
		TestTrue(
			*FString::Printf(
				TEXT("%s - CaseLabel is unique"),
				Case.CaseLabel),
			bUniqueLabel);
		SeenCaseLabels.Add(CaseLabel);

		FMatchPlayState State = MakeReady({
			Case.ActionType,
			Case.SeedIntent});
		State.CurrentAttack.SelectedAction
			.ElectiveBranchIntent = Case.Intent;
		const auto Result =
			FMatchPlayCurrentAttackReadyForResolutionValidator
				::Validate(State);
		TestEqual(
			*FString::Printf(
				TEXT("%s - validation success matches expected"),
				Case.CaseLabel),
			Result.bSuccess,
			Case.bExpectedValid);
		TestEqual(
			*FString::Printf(
				TEXT("%s - exact Ready error matches expected"),
				Case.CaseLabel),
			Result.ErrorCode,
			Case.ExpectedError);
	}

	for (const FReadyHelperDamageCase& Case :
		ReadyHelperDamageCases)
	{
		const FString CaseLabel(Case.CaseLabel);
		const bool bUniqueLabel =
			!SeenCaseLabels.Contains(CaseLabel);
		TestTrue(
			*FString::Printf(
				TEXT("%s - CaseLabel is unique"),
				Case.CaseLabel),
			bUniqueLabel);
		SeenCaseLabels.Add(CaseLabel);

		FMatchPlayState State = Fixtures::MakeReadyState(
			Case.ActionType,
			Case.Intent,
			Case.bBaseHasHelper);
		State.CurrentAttack.SelectedAction.HelperCardId =
			Case.bBaseHasHelper
				? NAME_None
				: HelperFixtures::HelperId;
		const auto Result =
			FMatchPlayCurrentAttackReadyForResolutionValidator
				::Validate(State);
		TestEqual(
			*FString::Printf(
				TEXT("%s - validation success matches expected"),
				Case.CaseLabel),
			Result.bSuccess,
			Case.bExpectedValid);
		TestEqual(
			*FString::Printf(
				TEXT("%s - exact Ready error matches expected"),
				Case.CaseLabel),
			Result.ErrorCode,
			Case.ExpectedError);
	}
	return true;
}

#undef INTENT_PROPAGATION_TEST

#endif
