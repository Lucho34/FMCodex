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
	for (const FCase& Case : ValidCases)
	{
		const auto Result =
			FMatchPlayCurrentAttackReadyForResolutionValidator
				::Validate(MakeReady(Case));
		TestTrue(TEXT("Legal Ready combination succeeds"),
			Result.bSuccess);
	}

	struct FInvalidCase
	{
		FCase Base;
		EMatchPlayElectiveBranchIntent InvalidIntent =
			EMatchPlayElectiveBranchIntent::None;
	};
	const FInvalidCase InvalidCases[] = {
		{{ESkillRuleType::LongShot,
			EMatchPlayElectiveBranchIntent::DirectShot},
			EMatchPlayElectiveBranchIntent::None},
		{{ESkillRuleType::LongShot,
			EMatchPlayElectiveBranchIntent::DirectShot},
			EMatchPlayElectiveBranchIntent::CrossHigh},
		{{ESkillRuleType::LongShot,
			EMatchPlayElectiveBranchIntent::DirectShot},
			EMatchPlayElectiveBranchIntent::CrossLow},
		{{ESkillRuleType::CutInsideShot,
			EMatchPlayElectiveBranchIntent::DirectShot},
			EMatchPlayElectiveBranchIntent::None},
		{{ESkillRuleType::CutInsideShot,
			EMatchPlayElectiveBranchIntent::DirectShot},
			EMatchPlayElectiveBranchIntent::CrossHigh},
		{{ESkillRuleType::CutInsideShot,
			EMatchPlayElectiveBranchIntent::DirectShot},
			EMatchPlayElectiveBranchIntent::CrossLow},
		{{ESkillRuleType::Cross,
			EMatchPlayElectiveBranchIntent::CrossHigh},
			EMatchPlayElectiveBranchIntent::None},
		{{ESkillRuleType::Cross,
			EMatchPlayElectiveBranchIntent::CrossHigh},
			EMatchPlayElectiveBranchIntent::DirectShot},
		{{ESkillRuleType::Cross,
			EMatchPlayElectiveBranchIntent::CrossHigh},
			EMatchPlayElectiveBranchIntent::DeadCorner},
		{{ESkillRuleType::PassControl,
			EMatchPlayElectiveBranchIntent::None},
			EMatchPlayElectiveBranchIntent::DirectShot},
		{{ESkillRuleType::PassControl,
			EMatchPlayElectiveBranchIntent::None},
			EMatchPlayElectiveBranchIntent::DeadCorner},
		{{ESkillRuleType::PassControl,
			EMatchPlayElectiveBranchIntent::None},
			EMatchPlayElectiveBranchIntent::CrossHigh},
		{{ESkillRuleType::PassControl,
			EMatchPlayElectiveBranchIntent::None},
			EMatchPlayElectiveBranchIntent::CrossLow},
		{{ESkillRuleType::ThroughBall,
			EMatchPlayElectiveBranchIntent::None},
			EMatchPlayElectiveBranchIntent::DirectShot},
		{{ESkillRuleType::ThroughBall,
			EMatchPlayElectiveBranchIntent::None},
			EMatchPlayElectiveBranchIntent::DeadCorner},
		{{ESkillRuleType::ThroughBall,
			EMatchPlayElectiveBranchIntent::None},
			EMatchPlayElectiveBranchIntent::CrossHigh},
		{{ESkillRuleType::ThroughBall,
			EMatchPlayElectiveBranchIntent::None},
			EMatchPlayElectiveBranchIntent::CrossLow}
	};
	for (const FInvalidCase& Case : InvalidCases)
	{
		FMatchPlayState State = MakeReady(Case.Base);
		State.CurrentAttack.SelectedAction
			.ElectiveBranchIntent = Case.InvalidIntent;
		const auto Result =
			FMatchPlayCurrentAttackReadyForResolutionValidator
				::Validate(State);
		TestFalse(TEXT("Illegal Ready combination fails"),
			Result.bSuccess);
		TestEqual(TEXT("Illegal Intent exact Ready error"),
			Result.ErrorCode,
			EMatchPlayCurrentAttackReadyValidationErrorCode
				::InvalidElectiveBranchIntent);
	}

	FMatchPlayState MissingHelper = MakeReady({
		ESkillRuleType::Cross,
		EMatchPlayElectiveBranchIntent::CrossHigh});
	MissingHelper.CurrentAttack.SelectedAction.HelperCardId =
		NAME_None;
	TestFalse(TEXT("Present Helper with empty id fails"),
		FMatchPlayCurrentAttackReadyForResolutionValidator
			::Validate(MissingHelper)
			.bSuccess);

	FMatchPlayState UnexpectedHelper =
		Fixtures::MakeReadyState(
			ESkillRuleType::Cross,
			EMatchPlayElectiveBranchIntent::CrossLow,
			false);
	UnexpectedHelper.CurrentAttack.SelectedAction.HelperCardId =
		HelperFixtures::HelperId;
	TestFalse(TEXT("Absent Helper with non-empty id fails"),
		FMatchPlayCurrentAttackReadyForResolutionValidator
			::Validate(UnexpectedHelper)
			.bSuccess);
	return true;
}

#undef INTENT_PROPAGATION_TEST

#endif
