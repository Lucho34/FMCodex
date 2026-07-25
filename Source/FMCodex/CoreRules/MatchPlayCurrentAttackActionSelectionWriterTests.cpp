#include "MatchPlayCurrentAttackActionSelectionWriter.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

#include <type_traits>

namespace FMCodex::Tests::CurrentAttackActionSelection
{
	FSkillRuleSnapshotSet MakeSkillRules();
	FMatchPlayState MakeState();
	FMatchPlayCurrentAttackActionSelectionRequest MakeRequest(
		FName CarrierCardId,
		FName SkillId);
	FMatchPlayDeploymentPlacement MakePlacement(
		EInitialTurnOrderPlayer PlayerSide,
		FName CardId,
		const TCHAR* SlotId);
	bool AreStatesEqual(
		const FMatchPlayState& Left,
		const FMatchPlayState& Right);
}

namespace FMCodex::Tests::CurrentAttackActionSelectionWriter
{
	namespace Foundation =
		FMCodex::Tests::CurrentAttackActionSelection;

	constexpr int64 ValidAttackSequence = 11;
	const FName CarrierOneId(TEXT("PlayerA.CarrierOne"));
	const FName CarrierTwoId(TEXT("PlayerA.CarrierTwo"));
	const FName GoalkeeperId(TEXT("PlayerA.Goalkeeper"));
	const FName LongShotSkillId(TEXT("Skill.LongShot"));
	const FName CutInsideSkillId(TEXT("Skill.CutInside"));
	const FName PassControlSkillId(TEXT("Skill.PassControl"));
	const FName CrossSkillId(TEXT("Skill.Cross"));
	const FName ThroughBallSkillId(TEXT("Skill.ThroughBall"));

	FSkillRuleSnapshotSet MakeSkillRules()
	{
		return Foundation::MakeSkillRules();
	}

	FMatchPlayState MakeState()
	{
		return Foundation::MakeState();
	}

	FMatchPlayCurrentAttackActionSelectionRequest MakeRequest(
		const FName CarrierCardId = CarrierOneId,
		const FName SkillId = LongShotSkillId)
	{
		return Foundation::MakeRequest(CarrierCardId, SkillId);
	}

	FMatchPlayDeploymentPlacement MakePlacement(
		const EInitialTurnOrderPlayer PlayerSide,
		const FName CardId,
		const TCHAR* SlotId)
	{
		return Foundation::MakePlacement(PlayerSide, CardId, SlotId);
	}

	bool AreStatesEqual(
		const FMatchPlayState& Left,
		const FMatchPlayState& Right)
	{
		return Foundation::AreStatesEqual(Left, Right);
	}

	bool ExpectWriterFailure(
		FAutomationTestBase& Test,
		const TCHAR* Context,
		const FMatchPlayState& BeforeState,
		const FMatchPlayCurrentAttackActionSelectionRequest& Request,
		const FSkillRuleSnapshotSet& SkillRules,
		const EMatchPlayCurrentAttackActionSelectionErrorCode
			ExpectedLegalityError)
	{
		const FMatchPlayState OriginalState = BeforeState;
		const FMatchPlayCurrentAttackActionSelectionWriterResult Result =
			FMatchPlayCurrentAttackActionSelectionWriter::Select(
				BeforeState,
				Request,
				SkillRules);
		Test.TestFalse(
			*FString::Printf(TEXT("%s fails"), Context),
			Result.bSuccess);
		Test.TestEqual(
			*FString::Printf(TEXT("%s returns writer failure"), Context),
			Result.ErrorCode,
			EMatchPlayCurrentAttackActionSelectionWriterErrorCode
				::LegalityFailed);
		Test.TestEqual(
			*FString::Printf(TEXT("%s propagates legality error"), Context),
			Result.LegalityResult.ErrorCode,
			ExpectedLegalityError);
		Test.TestTrue(
			*FString::Printf(TEXT("%s preserves result BeforeState"), Context),
			AreStatesEqual(Result.BeforeState, OriginalState));
		Test.TestTrue(
			*FString::Printf(TEXT("%s preserves result AfterState"), Context),
			AreStatesEqual(Result.AfterState, OriginalState));
		Test.TestTrue(
			*FString::Printf(TEXT("%s does not mutate input"), Context),
			AreStatesEqual(BeforeState, OriginalState));
		Test.TestFalse(
			*FString::Printf(TEXT("%s writes no selected flag"), Context),
			Result.AfterState.CurrentAttack.bHasSelectedAction);
		return true;
	}
}

#define ACTION_SELECTION_WRITER_TEST(TestClass, TestName) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
		TestClass, \
		"FMCodex.CoreRules.MatchPlayCurrentAttackActionSelection.Writer." TestName, \
		EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

ACTION_SELECTION_WRITER_TEST(
	FActionSelectionWriterContractTest,
	"DefaultsReflectionAndSignature")

bool FActionSelectionWriterContractTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayCurrentAttackActionSelectionWriterResult Result;
	TestFalse(TEXT("Default result fails"), Result.bSuccess);
	TestEqual(TEXT("Default writer error is None"), Result.ErrorCode,
		EMatchPlayCurrentAttackActionSelectionWriterErrorCode::None);
	TestNotNull(TEXT("Writer result is reflected"),
		FMatchPlayCurrentAttackActionSelectionWriterResult::StaticStruct());
	using FSelectSignature =
		FMatchPlayCurrentAttackActionSelectionWriterResult (*)(
			const FMatchPlayState&,
			const FMatchPlayCurrentAttackActionSelectionRequest&,
			const FSkillRuleSnapshotSet&);
	TestTrue(TEXT("Writer has one frozen public signature"),
		(std::is_same_v<
			decltype(
				&FMatchPlayCurrentAttackActionSelectionWriter::Select),
			FSelectSignature>));
	return true;
}

ACTION_SELECTION_WRITER_TEST(
	FActionSelectionWriterSupportedTypesTest,
	"WritesAllSupportedAuthoritativeTypes")

bool FActionSelectionWriterSupportedTypesTest::RunTest(
	const FString& Parameters)
{
	using namespace
		FMCodex::Tests::CurrentAttackActionSelectionWriter;
	struct FCase
	{
		FName CarrierCardId;
		FName SkillId;
		ESkillRuleType ExpectedType;
		int32 ActionPoint;
	};
	const TArray<FCase> Cases = {
		{ CarrierOneId, LongShotSkillId, ESkillRuleType::LongShot, 2 },
		{ CarrierTwoId, CutInsideSkillId,
			ESkillRuleType::CutInsideShot, 3 },
		{ CarrierTwoId, PassControlSkillId,
			ESkillRuleType::PassControl, 4 },
		{ CarrierOneId, CrossSkillId, ESkillRuleType::Cross, 8 },
		{ CarrierTwoId, ThroughBallSkillId,
			ESkillRuleType::ThroughBall, 7 }
	};

	for (const FCase& Case : Cases)
	{
		FMatchPlayState BeforeState = MakeState();
		BeforeState.CurrentAttack.ActionPoint = Case.ActionPoint;
		FMatchPlayState ExpectedState = BeforeState;
		ExpectedState.CurrentAttack.bHasSelectedAction = true;
		ExpectedState.CurrentAttack.SelectedAction.CarrierCardId =
			Case.CarrierCardId;
		ExpectedState.CurrentAttack.SelectedAction.SkillId =
			Case.SkillId;
		ExpectedState.CurrentAttack.SelectedAction.ActionType =
			Case.ExpectedType;

		const FMatchPlayCurrentAttackActionSelectionWriterResult Result =
			FMatchPlayCurrentAttackActionSelectionWriter::Select(
				BeforeState,
				MakeRequest(Case.CarrierCardId, Case.SkillId),
				MakeSkillRules());
		TestTrue(TEXT("Writer succeeds"), Result.bSuccess);
		TestEqual(TEXT("Writer top error is clean"), Result.ErrorCode,
			EMatchPlayCurrentAttackActionSelectionWriterErrorCode::None);
		TestTrue(TEXT("Legality succeeds"),
			Result.LegalityResult.bIsLegal);
		TestEqual(TEXT("Legality resolves expected type"),
			Result.LegalityResult.ResolvedActionType,
			Case.ExpectedType);
		TestEqual(TEXT("Result selected type is expected"),
			Result.SelectedAction.ActionType,
			Case.ExpectedType);
		TestTrue(TEXT("Only expected State fields change"),
			AreStatesEqual(Result.AfterState, ExpectedState));
		TestTrue(TEXT("Input remains unchanged"),
			AreStatesEqual(Result.BeforeState, BeforeState));
	}
	return true;
}

ACTION_SELECTION_WRITER_TEST(
	FActionSelectionWriterAuthoritySourceTest,
	"ActionTypeComesFromRuleNotSkillName")

bool FActionSelectionWriterAuthoritySourceTest::RunTest(
	const FString& Parameters)
{
	using namespace
		FMCodex::Tests::CurrentAttackActionSelectionWriter;
	FSkillRuleSnapshotSet Rules = MakeSkillRules();
	Rules.SkillRules[0].SkillType = ESkillRuleType::Cross;
	const auto Result =
		FMatchPlayCurrentAttackActionSelectionWriter::Select(
			MakeState(),
			MakeRequest(CarrierOneId, LongShotSkillId),
			Rules);
	TestTrue(TEXT("Writer succeeds"), Result.bSuccess);
	TestEqual(TEXT("Renamed rule type is authoritative"),
		Result.AfterState.CurrentAttack.SelectedAction.ActionType,
		ESkillRuleType::Cross);
	return true;
}

ACTION_SELECTION_WRITER_TEST(
	FActionSelectionWriterPreservationTest,
	"SuccessPreservesAllNonSelectedState")

bool FActionSelectionWriterPreservationTest::RunTest(
	const FString& Parameters)
{
	using namespace
		FMCodex::Tests::CurrentAttackActionSelectionWriter;
	const FMatchPlayState BeforeState = MakeState();
	const auto Result =
		FMatchPlayCurrentAttackActionSelectionWriter::Select(
			BeforeState,
			MakeRequest(),
			MakeSkillRules());
	TestTrue(TEXT("Writer succeeds"), Result.bSuccess);
	TestEqual(TEXT("Phase remains Resolution"),
		Result.AfterState.CurrentAttack.Phase,
		EMatchPlayCurrentAttackPhase::Resolution);
	TestEqual(TEXT("Legal side remains None"),
		Result.AfterState.CurrentAttack.CurrentLegalDeploymentSide,
		EInitialTurnOrderPlayer::None);
	TestEqual(TEXT("Sequence preserved"),
		Result.AfterState.CurrentAttack.AttackSequence,
		BeforeState.CurrentAttack.AttackSequence);
	TestEqual(TEXT("ActionPoint preserved"),
		Result.AfterState.CurrentAttack.ActionPoint,
		BeforeState.CurrentAttack.ActionPoint);
	TestEqual(TEXT("Finished flags preserved"),
		Result.AfterState.CurrentAttack.bAttackerDeploymentFinished,
		BeforeState.CurrentAttack.bAttackerDeploymentFinished);
	TestEqual(TEXT("Placement count preserved"),
		Result.AfterState.CurrentAttack.DeploymentPlacements.Num(),
		BeforeState.CurrentAttack.DeploymentPlacements.Num());
	for (int32 Index = 0;
		Index < BeforeState.CurrentAttack.DeploymentPlacements.Num();
		++Index)
	{
		TestTrue(TEXT("Placement preserved field-for-field"),
			FMatchPlayDeploymentPlacement::StaticStruct()
				->CompareScriptStruct(
					&Result.AfterState.CurrentAttack
						.DeploymentPlacements[Index],
					&BeforeState.CurrentAttack
						.DeploymentPlacements[Index],
					0));
	}
	TestTrue(TEXT("Snapshots preserved"),
		FMatchPlayPerSideCardSnapshotAuthority::StaticStruct()
			->CompareScriptStruct(
				&Result.AfterState.CardSnapshotAuthority,
				&BeforeState.CardSnapshotAuthority,
				0));
	TestTrue(TEXT("CardUsage preserved"),
		FMatchCardUsageState::StaticStruct()->CompareScriptStruct(
			&Result.AfterState.CardUsageState,
			&BeforeState.CardUsageState,
			0));
	TestTrue(TEXT("Runtime including score/opportunity preserved"),
		FMatchRuntimeState::StaticStruct()->CompareScriptStruct(
			&Result.AfterState.RuntimeState,
			&BeforeState.RuntimeState,
			0));
	TestTrue(TEXT("GK usage preserved"),
		FMatchPlayGoalkeeperUsageState::StaticStruct()
			->CompareScriptStruct(
				&Result.AfterState.GoalkeeperUsageState,
				&BeforeState.GoalkeeperUsageState,
				0));
	TestEqual(TEXT("GK activation preserved"),
		Result.AfterState.CurrentAttack
			.bCurrentDefenseGoalkeeperActivated,
		BeforeState.CurrentAttack
			.bCurrentDefenseGoalkeeperActivated);
	return true;
}

ACTION_SELECTION_WRITER_TEST(
	FActionSelectionWriterRepeatedSelectionTest,
	"SecondSelectionCannotOverwriteFirst")

bool FActionSelectionWriterRepeatedSelectionTest::RunTest(
	const FString& Parameters)
{
	using namespace
		FMCodex::Tests::CurrentAttackActionSelectionWriter;
	const auto First =
		FMatchPlayCurrentAttackActionSelectionWriter::Select(
			MakeState(),
			MakeRequest(),
			MakeSkillRules());
	TestTrue(TEXT("First selection succeeds"), First.bSuccess);
	const FMatchPlayState FirstSelectedState = First.AfterState;
	const auto Second =
		FMatchPlayCurrentAttackActionSelectionWriter::Select(
			FirstSelectedState,
			MakeRequest(CarrierOneId, CrossSkillId),
			MakeSkillRules());
	TestFalse(TEXT("Second selection fails"), Second.bSuccess);
	TestEqual(TEXT("Exact repeated-selection error"),
		Second.LegalityResult.ErrorCode,
		EMatchPlayCurrentAttackActionSelectionErrorCode
			::ActionAlreadySelected);
	TestTrue(TEXT("First selection remains field-for-field"),
		AreStatesEqual(Second.AfterState, FirstSelectedState));
	TestEqual(TEXT("Original skill is not overwritten"),
		Second.AfterState.CurrentAttack.SelectedAction.SkillId,
		LongShotSkillId);
	return true;
}

#define SIMPLE_WRITER_FAILURE( \
	TestClass, TestName, StateMutation, RequestMutation, RulesMutation, \
	ExpectedError) \
	ACTION_SELECTION_WRITER_TEST(TestClass, TestName) \
	bool TestClass::RunTest(const FString& Parameters) \
	{ \
		using namespace \
			FMCodex::Tests::CurrentAttackActionSelectionWriter; \
		FMatchPlayState State = MakeState(); \
		FMatchPlayCurrentAttackActionSelectionRequest Request = \
			MakeRequest(); \
		FSkillRuleSnapshotSet Rules = MakeSkillRules(); \
		StateMutation; \
		RequestMutation; \
		RulesMutation; \
		return ExpectWriterFailure( \
			*this, TEXT(TestName), State, Request, Rules, ExpectedError); \
	}

SIMPLE_WRITER_FAILURE(
	FActionSelectionWriterStaleTest,
	"StaleSequenceIsAtomic",
	State.RuntimeState.bIsInitialized = true,
	Request.AttackSequence = ValidAttackSequence + 1,
	Rules.SkillRules.Reset(),
	EMatchPlayCurrentAttackActionSelectionErrorCode
		::AttackSequenceMismatch)

SIMPLE_WRITER_FAILURE(
	FActionSelectionWriterWrongSideTest,
	"WrongSideIsAtomic",
	State.RuntimeState.bIsInitialized = true,
	Request.RequestingSide = EInitialTurnOrderPlayer::PlayerB,
	Rules.SkillRules.Reset(),
	EMatchPlayCurrentAttackActionSelectionErrorCode
		::RequestingSideIsNotCurrentAttacker)

SIMPLE_WRITER_FAILURE(
	FActionSelectionWriterWrongPhaseTest,
	"WrongPhaseIsAtomic",
	State.CurrentAttack.Phase =
		EMatchPlayCurrentAttackPhase::Deployment,
	Request.CarrierCardId = NAME_None,
	Rules.SkillRules.Reset(),
	EMatchPlayCurrentAttackActionSelectionErrorCode
		::CurrentAttackNotInResolution)

SIMPLE_WRITER_FAILURE(
	FActionSelectionWriterCorruptSelectedStateTest,
	"CorruptSelectedStateIsAtomic",
	State.CurrentAttack.SelectedAction.SkillId = LongShotSkillId,
	Request.CarrierCardId = NAME_None,
	Rules.SkillRules.Reset(),
	EMatchPlayCurrentAttackActionSelectionErrorCode
		::InvalidSelectedActionState)

SIMPLE_WRITER_FAILURE(
	FActionSelectionWriterGoalkeeperTest,
	"GoalkeeperCarrierIsAtomic",
	State.CurrentAttack.DeploymentPlacements.Add(
		MakePlacement(
			EInitialTurnOrderPlayer::PlayerA,
			GoalkeeperId,
			TEXT("Slot.GK"))),
	Request.CarrierCardId = GoalkeeperId;
	Request.SkillId = LongShotSkillId,
	Rules.SkillRules.Reset(),
	EMatchPlayCurrentAttackActionSelectionErrorCode
		::CarrierIsGoalkeeper)

SIMPLE_WRITER_FAILURE(
	FActionSelectionWriterForeignSkillTest,
	"ForeignSkillIsAtomic",
	State.RuntimeState.bIsInitialized = true,
	Request.SkillId = ThroughBallSkillId,
	Rules.SkillRules.Reset(),
	EMatchPlayCurrentAttackActionSelectionErrorCode
		::SkillNotOwnedByCarrier)

SIMPLE_WRITER_FAILURE(
	FActionSelectionWriterActionPointTest,
	"ActionPointMismatchIsAtomic",
	State.CurrentAttack.ActionPoint = 4,
	Request.SkillId = CrossSkillId,
	Rules.SkillRules[3].MinTriggerActionPoint = 5,
	EMatchPlayCurrentAttackActionSelectionErrorCode
		::ActionPointOutsideSkillRange)

SIMPLE_WRITER_FAILURE(
	FActionSelectionWriterInvalidRuleSetTest,
	"InvalidRuleSetDiagnosticsArePropagated",
	State.RuntimeState.bIsInitialized = true,
	Request.SkillId = LongShotSkillId,
	const FSkillRuleSnapshot DuplicateRule = Rules.SkillRules[0];
	Rules.SkillRules.Add(DuplicateRule),
	EMatchPlayCurrentAttackActionSelectionErrorCode
		::SkillRuleSetValidationFailed)

SIMPLE_WRITER_FAILURE(
	FActionSelectionWriterMissingRuleTest,
	"MissingRuleDiagnosticsArePropagated",
	State.RuntimeState.bIsInitialized = true,
	Request.SkillId = LongShotSkillId,
	Rules.SkillRules.RemoveAt(0),
	EMatchPlayCurrentAttackActionSelectionErrorCode
		::SkillRuleLookupFailed)

SIMPLE_WRITER_FAILURE(
	FActionSelectionWriterMixedPriorityTest,
	"FirstErrorOrderStillComesFromEvaluator",
	State.RuntimeState.bIsInitialized = false;
	State.bHasCurrentAttack = false,
	Request.AttackSequence = 0;
	Request.RequestingSide = EInitialTurnOrderPlayer::None;
	Request.CarrierCardId = NAME_None;
	Request.SkillId = NAME_None,
	Rules.SkillRules.Reset(),
	EMatchPlayCurrentAttackActionSelectionErrorCode
		::MatchPlayStateNotInitialized)

#undef SIMPLE_WRITER_FAILURE
#undef ACTION_SELECTION_WRITER_TEST

#endif
