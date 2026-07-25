#include "MatchPlayCurrentAttackActionSelectionLegality.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

#include <type_traits>

namespace FMCodex::Tests::CurrentAttackActionSelection
{
	constexpr int64 ValidAttackSequence = 11;
	const FName CarrierOneId(TEXT("PlayerA.CarrierOne"));
	const FName CarrierTwoId(TEXT("PlayerA.CarrierTwo"));
	const FName GoalkeeperId(TEXT("PlayerA.Goalkeeper"));
	const FName DefenderSameId(TEXT("PlayerA.CarrierOne"));
	const FName LongShotSkillId(TEXT("Skill.LongShot"));
	const FName CutInsideSkillId(TEXT("Skill.CutInside"));
	const FName PassControlSkillId(TEXT("Skill.PassControl"));
	const FName CrossSkillId(TEXT("Skill.Cross"));
	const FName ThroughBallSkillId(TEXT("Skill.ThroughBall"));

	void SetBaseAttributes(FPlayerAttributes& Attributes)
	{
		Attributes.Shooting = 3;
		Attributes.Dribbling = 3;
		Attributes.Passing = 3;
		Attributes.OffBall = 3;
		Attributes.Marking = 3;
		Attributes.Tackling = 3;
		Attributes.Speed = 3;
		Attributes.Strength = 3;
		Attributes.Stamina = 3;
		Attributes.LongShot = 3;
	}

	void SetGoalkeeperAttributes(FGoalkeeperAttributes& Attributes)
	{
		Attributes.Handling = 3;
		Attributes.Positioning = 3;
		Attributes.Reflex = 3;
		Attributes.Aerial = 3;
		Attributes.Anticipation = 3;
		Attributes.OneOnOne = 3;
	}

	FPlayerCardRuleSnapshot MakeCard(
		const FName CardId,
		const TArray<FName>& SkillIds,
		const bool bGoalkeeper = false)
	{
		FPlayerCardRuleSnapshot Snapshot;
		Snapshot.CardId = CardId;
		Snapshot.PositionTypes = {
			bGoalkeeper
				? EPlayerPositionType::Goalkeeper
				: EPlayerPositionType::Attack
		};
		SetBaseAttributes(Snapshot.Attributes);
		Snapshot.bIsGoalkeeper = bGoalkeeper;
		Snapshot.bHasGoalkeeperAttributes = bGoalkeeper;
		if (bGoalkeeper)
		{
			SetGoalkeeperAttributes(Snapshot.GoalkeeperAttributes);
		}
		Snapshot.SkillIds = SkillIds;
		return Snapshot;
	}

	FSkillRuleSnapshot MakeSkillRule(
		const FName SkillId,
		const ESkillRuleType SkillType,
		const int32 MinActionPoint,
		const int32 MaxActionPoint)
	{
		FSkillRuleSnapshot Rule;
		Rule.SkillId = SkillId;
		Rule.SkillType = SkillType;
		Rule.MinTriggerActionPoint = MinActionPoint;
		Rule.MaxTriggerActionPoint = MaxActionPoint;
		return Rule;
	}

	FSkillRuleSnapshotSet MakeSkillRules()
	{
		FSkillRuleSnapshotSet Rules;
		Rules.SkillRules = {
			MakeSkillRule(
				LongShotSkillId,
				ESkillRuleType::LongShot,
				2,
				8),
			MakeSkillRule(
				CutInsideSkillId,
				ESkillRuleType::CutInsideShot,
				3,
				6),
			MakeSkillRule(
				PassControlSkillId,
				ESkillRuleType::PassControl,
				2,
				4),
			MakeSkillRule(
				CrossSkillId,
				ESkillRuleType::Cross,
				5,
				8),
			MakeSkillRule(
				ThroughBallSkillId,
				ESkillRuleType::ThroughBall,
				4,
				7)
		};
		return Rules;
	}

	FMatchPlayDeploymentPlacement MakePlacement(
		const EInitialTurnOrderPlayer PlayerSide,
		const FName CardId,
		const TCHAR* SlotId)
	{
		FMatchPlayDeploymentPlacement Placement;
		Placement.PlayerSide = PlayerSide;
		Placement.CardId = CardId;
		Placement.SlotId = FName(SlotId);
		return Placement;
	}

	FMatchPlayState MakeState()
	{
		FMatchPlayState State;
		State.RuntimeState.bIsInitialized = true;
		State.RuntimeState.CurrentAttackingPlayer =
			EInitialTurnOrderPlayer::PlayerA;
		State.RuntimeState.PlayerAState.TotalAttackCount = 4;
		State.RuntimeState.PlayerBState.TotalAttackCount = 4;
		State.CardSnapshotAuthority.PlayerACardSnapshots.Cards = {
			MakeCard(
				CarrierOneId,
				{ LongShotSkillId, CrossSkillId }),
			MakeCard(
				CarrierTwoId,
				{ CutInsideSkillId, ThroughBallSkillId,
					PassControlSkillId }),
			MakeCard(
				GoalkeeperId,
				{ LongShotSkillId },
				true)
		};
		State.CardSnapshotAuthority.PlayerBCardSnapshots.Cards = {
			MakeCard(
				DefenderSameId,
				{ LongShotSkillId })
		};
		State.bHasCurrentAttack = true;
		State.CurrentAttack.Phase =
			EMatchPlayCurrentAttackPhase::Resolution;
		State.CurrentAttack.AttackSequence = ValidAttackSequence;
		State.CurrentAttack.ActionPoint = 5;
		State.CurrentAttack.CurrentLegalDeploymentSide =
			EInitialTurnOrderPlayer::None;
		State.CurrentAttack.bAttackerDeploymentFinished = true;
		State.CurrentAttack.bDefenderDeploymentFinished = true;
		State.CurrentAttack.DeploymentPlacements = {
			MakePlacement(
				EInitialTurnOrderPlayer::PlayerA,
				CarrierOneId,
				TEXT("Slot.A1")),
			MakePlacement(
				EInitialTurnOrderPlayer::PlayerB,
				DefenderSameId,
				TEXT("Slot.B1")),
			MakePlacement(
				EInitialTurnOrderPlayer::PlayerA,
				CarrierTwoId,
				TEXT("Slot.A2"))
		};
		return State;
	}

	FMatchPlayCurrentAttackActionSelectionRequest MakeRequest(
		const FName CarrierCardId = CarrierOneId,
		const FName SkillId = LongShotSkillId)
	{
		FMatchPlayCurrentAttackActionSelectionRequest Request;
		Request.AttackSequence = ValidAttackSequence;
		Request.RequestingSide = EInitialTurnOrderPlayer::PlayerA;
		Request.CarrierCardId = CarrierCardId;
		Request.SkillId = SkillId;
		return Request;
	}

	bool AreStatesEqual(
		const FMatchPlayState& Left,
		const FMatchPlayState& Right)
	{
		return FMatchPlayState::StaticStruct()->CompareScriptStruct(
			&Left,
			&Right,
			0);
	}

	bool ExpectFailure(
		FAutomationTestBase& Test,
		const TCHAR* Context,
		const FMatchPlayState& State,
		const FMatchPlayCurrentAttackActionSelectionRequest& Request,
		const FSkillRuleSnapshotSet& SkillRules,
		const EMatchPlayCurrentAttackActionSelectionErrorCode ExpectedError)
	{
		const FMatchPlayState OriginalState = State;
		const FSkillRuleSnapshotSet OriginalRules = SkillRules;
		const FMatchPlayCurrentAttackActionSelectionLegalityResult Result =
			FMatchPlayCurrentAttackActionSelectionLegalityEvaluator
				::Evaluate(State, Request, SkillRules);
		Test.TestFalse(
			*FString::Printf(TEXT("%s is illegal"), Context),
			Result.bIsLegal);
		Test.TestEqual(
			*FString::Printf(TEXT("%s returns exact error"), Context),
			Result.ErrorCode,
			ExpectedError);
		Test.TestTrue(
			*FString::Printf(TEXT("%s has diagnostics"), Context),
			!Result.ErrorMessage.IsEmpty());
		Test.TestTrue(
			*FString::Printf(TEXT("%s does not mutate State"), Context),
			AreStatesEqual(State, OriginalState));
		Test.TestTrue(
			*FString::Printf(TEXT("%s does not mutate SkillRules"), Context),
			SkillRules.SkillRules.Num()
				== OriginalRules.SkillRules.Num());
		return true;
	}
}

#define ACTION_SELECTION_LEGALITY_TEST(TestClass, TestName) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
		TestClass, \
		"FMCodex.CoreRules.MatchPlayCurrentAttackActionSelection.Legality." TestName, \
		EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

ACTION_SELECTION_LEGALITY_TEST(
	FActionSelectionContractDefaultsTest,
	"DefaultsReflectionAndSignature")

bool FActionSelectionContractDefaultsTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayCurrentAttackActionSelectionRequest Request;
	const FMatchPlayCurrentAttackActionSelectionLegalityResult Result;
	TestEqual(TEXT("Default sequence"), Request.AttackSequence, int64{0});
	TestEqual(TEXT("Default requesting side"), Request.RequestingSide,
		EInitialTurnOrderPlayer::None);
	TestTrue(TEXT("Default carrier is empty"),
		Request.CarrierCardId.IsNone());
	TestTrue(TEXT("Default skill is empty"), Request.SkillId.IsNone());
	TestFalse(TEXT("Default result is illegal"), Result.bIsLegal);
	TestNotNull(TEXT("Request is reflected"),
		FMatchPlayCurrentAttackActionSelectionRequest::StaticStruct());
	TestNotNull(TEXT("Result is reflected"),
		FMatchPlayCurrentAttackActionSelectionLegalityResult::StaticStruct());
	using FEvaluateSignature =
		FMatchPlayCurrentAttackActionSelectionLegalityResult (*)(
			const FMatchPlayState&,
			const FMatchPlayCurrentAttackActionSelectionRequest&,
			const FSkillRuleSnapshotSet&);
	TestTrue(TEXT("Single evaluator signature is frozen"),
		(std::is_same_v<
			decltype(
				&FMatchPlayCurrentAttackActionSelectionLegalityEvaluator
					::Evaluate),
			FEvaluateSignature>));
	TestEqual(TEXT("LongShot enum value is unchanged"),
		static_cast<uint8>(ESkillRuleType::LongShot), uint8{1});
	TestEqual(TEXT("ThroughBall enum value is unchanged"),
		static_cast<uint8>(ESkillRuleType::ThroughBall), uint8{5});
	TestNotNull(TEXT("Skill type is reflected"),
		StaticEnum<ESkillRuleType>());
	return true;
}

ACTION_SELECTION_LEGALITY_TEST(
	FActionSelectionSupportedTypesTest,
	"SupportedTypesResolveAuthoritatively")

bool FActionSelectionSupportedTypesTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::CurrentAttackActionSelection;
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
		FMatchPlayState State = MakeState();
		State.CurrentAttack.ActionPoint = Case.ActionPoint;
		const FMatchPlayState OriginalState = State;
		const FMatchPlayCurrentAttackActionSelectionLegalityResult Result =
			FMatchPlayCurrentAttackActionSelectionLegalityEvaluator
				::Evaluate(
					State,
					MakeRequest(Case.CarrierCardId, Case.SkillId),
					MakeSkillRules());
		TestTrue(
			*FString::Printf(
				TEXT("%s selection is legal"),
				*Case.SkillId.ToString()),
			Result.bIsLegal);
		TestEqual(TEXT("Resolved type is authoritative"),
			Result.ResolvedActionType, Case.ExpectedType);
		TestEqual(TEXT("Top error is clean"), Result.ErrorCode,
			EMatchPlayCurrentAttackActionSelectionErrorCode::None);
		TestEqual(TEXT("Snapshot diagnostic is clean"),
			Result.UnderlyingSnapshotAuthorityQueryErrorCode,
			EMatchPlayCardSnapshotAuthorityQueryErrorCode::None);
		TestEqual(TEXT("Rule validation diagnostic is clean"),
			Result.UnderlyingSkillRuleSetValidationErrorCode,
			ESkillRuleSnapshotValidationErrorCode::None);
		TestEqual(TEXT("Rule query diagnostic is clean"),
			Result.UnderlyingSkillRuleQueryErrorCode,
			ESkillRuleSnapshotQueryErrorCode::None);
		TestTrue(TEXT("Evaluation is read-only"),
			AreStatesEqual(State, OriginalState));
	}
	return true;
}

#define SIMPLE_ACTION_SELECTION_FAILURE( \
	TestClass, TestName, StateMutation, RequestMutation, RulesMutation, \
	ExpectedError) \
	ACTION_SELECTION_LEGALITY_TEST(TestClass, TestName) \
	bool TestClass::RunTest(const FString& Parameters) \
	{ \
		using namespace FMCodex::Tests::CurrentAttackActionSelection; \
		FMatchPlayState State = MakeState(); \
		FMatchPlayCurrentAttackActionSelectionRequest Request = \
			MakeRequest(); \
		FSkillRuleSnapshotSet Rules = MakeSkillRules(); \
		StateMutation; \
		RequestMutation; \
		RulesMutation; \
		return ExpectFailure( \
			*this, TEXT(TestName), State, Request, Rules, ExpectedError); \
	}

SIMPLE_ACTION_SELECTION_FAILURE(
	FActionSelectionUninitializedTest,
	"RejectsUninitializedBeforeOtherErrors",
	State.RuntimeState.bIsInitialized = false,
	Request.CarrierCardId = NAME_None,
	Rules.SkillRules.Reset(),
	EMatchPlayCurrentAttackActionSelectionErrorCode
		::MatchPlayStateNotInitialized)

SIMPLE_ACTION_SELECTION_FAILURE(
	FActionSelectionNoAttackTest,
	"RejectsMissingCurrentAttack",
	State.bHasCurrentAttack = false,
	Request.CarrierCardId = NAME_None,
	Rules.SkillRules.Reset(),
	EMatchPlayCurrentAttackActionSelectionErrorCode::NoCurrentAttack)

SIMPLE_ACTION_SELECTION_FAILURE(
	FActionSelectionInvalidSequenceTest,
	"RejectsInvalidAuthoritativeSequence",
	State.CurrentAttack.AttackSequence = 0,
	Request.AttackSequence = 0,
	Rules.SkillRules.Reset(),
	EMatchPlayCurrentAttackActionSelectionErrorCode
		::InvalidCurrentAttackSequence)

SIMPLE_ACTION_SELECTION_FAILURE(
	FActionSelectionStaleTest,
	"RejectsStaleSequence",
	State.RuntimeState.bIsInitialized = true,
	Request.AttackSequence = ValidAttackSequence + 1,
	Rules.SkillRules.Reset(),
	EMatchPlayCurrentAttackActionSelectionErrorCode
		::AttackSequenceMismatch)

SIMPLE_ACTION_SELECTION_FAILURE(
	FActionSelectionPhaseTest,
	"RejectsDeploymentPhase",
	State.CurrentAttack.Phase =
		EMatchPlayCurrentAttackPhase::Deployment,
	Request.RequestingSide = EInitialTurnOrderPlayer::None,
	Rules.SkillRules.Reset(),
	EMatchPlayCurrentAttackActionSelectionErrorCode
		::CurrentAttackNotInResolution)

SIMPLE_ACTION_SELECTION_FAILURE(
	FActionSelectionInvalidAttackerTest,
	"RejectsInvalidAttacker",
	State.RuntimeState.CurrentAttackingPlayer =
		EInitialTurnOrderPlayer::None,
	Request.RequestingSide = EInitialTurnOrderPlayer::None,
	Rules.SkillRules.Reset(),
	EMatchPlayCurrentAttackActionSelectionErrorCode
		::InvalidCurrentAttackingPlayer)

SIMPLE_ACTION_SELECTION_FAILURE(
	FActionSelectionInvalidRequesterTest,
	"RejectsInvalidRequester",
	State.RuntimeState.bIsInitialized = true,
	Request.RequestingSide = EInitialTurnOrderPlayer::None,
	Rules.SkillRules.Reset(),
	EMatchPlayCurrentAttackActionSelectionErrorCode
		::InvalidRequestingSide)

SIMPLE_ACTION_SELECTION_FAILURE(
	FActionSelectionDefenderRequesterTest,
	"RejectsDefenderRequester",
	State.RuntimeState.bIsInitialized = true,
	Request.RequestingSide = EInitialTurnOrderPlayer::PlayerB,
	Rules.SkillRules.Reset(),
	EMatchPlayCurrentAttackActionSelectionErrorCode
		::RequestingSideIsNotCurrentAttacker)

SIMPLE_ACTION_SELECTION_FAILURE(
	FActionSelectionNotFinishedTest,
	"RejectsPartiallyFinishedDeployment",
	State.CurrentAttack.bDefenderDeploymentFinished = false,
	Request.CarrierCardId = NAME_None,
	Rules.SkillRules.Reset(),
	EMatchPlayCurrentAttackActionSelectionErrorCode
		::DeploymentNotFullyFinished)

SIMPLE_ACTION_SELECTION_FAILURE(
	FActionSelectionLegalSideTest,
	"RejectsLegalDeploymentSideInResolution",
	State.CurrentAttack.CurrentLegalDeploymentSide =
		EInitialTurnOrderPlayer::PlayerA,
	Request.CarrierCardId = NAME_None,
	Rules.SkillRules.Reset(),
	EMatchPlayCurrentAttackActionSelectionErrorCode
		::InvalidCurrentLegalDeploymentSide)

SIMPLE_ACTION_SELECTION_FAILURE(
	FActionSelectionCorruptEmptyTest,
	"RejectsCorruptEmptySelectedAction",
	State.CurrentAttack.SelectedAction.CarrierCardId =
		CarrierOneId,
	Request.CarrierCardId = NAME_None,
	Rules.SkillRules.Reset(),
	EMatchPlayCurrentAttackActionSelectionErrorCode
		::InvalidSelectedActionState)

SIMPLE_ACTION_SELECTION_FAILURE(
	FActionSelectionCorruptSelectedTest,
	"RejectsCorruptSelectedAction",
	State.CurrentAttack.bHasSelectedAction = true;
	State.CurrentAttack.SelectedAction.CarrierCardId =
		CarrierOneId;
	State.CurrentAttack.SelectedAction.SkillId =
		LongShotSkillId;
	State.CurrentAttack.SelectedAction.ActionType =
		ESkillRuleType::None,
	Request.CarrierCardId = NAME_None,
	Rules.SkillRules.Reset(),
	EMatchPlayCurrentAttackActionSelectionErrorCode
		::InvalidSelectedActionState)

SIMPLE_ACTION_SELECTION_FAILURE(
	FActionSelectionAlreadySelectedTest,
	"RejectsCanonicalAlreadySelectedAction",
	State.CurrentAttack.bHasSelectedAction = true;
	State.CurrentAttack.SelectedAction.CarrierCardId =
		CarrierOneId;
	State.CurrentAttack.SelectedAction.SkillId =
		LongShotSkillId;
	State.CurrentAttack.SelectedAction.ActionType =
		ESkillRuleType::LongShot,
	Request.CarrierCardId = NAME_None,
	Rules.SkillRules.Reset(),
	EMatchPlayCurrentAttackActionSelectionErrorCode
		::ActionAlreadySelected)

SIMPLE_ACTION_SELECTION_FAILURE(
	FActionSelectionEmptyCarrierTest,
	"RejectsEmptyCarrierBeforeEmptySkill",
	State.RuntimeState.bIsInitialized = true,
	Request.CarrierCardId = NAME_None;
	Request.SkillId = NAME_None,
	Rules.SkillRules.Reset(),
	EMatchPlayCurrentAttackActionSelectionErrorCode
		::InvalidCarrierCardId)

SIMPLE_ACTION_SELECTION_FAILURE(
	FActionSelectionEmptySkillTest,
	"RejectsEmptySkill",
	State.RuntimeState.bIsInitialized = true,
	Request.SkillId = NAME_None,
	Rules.SkillRules.Reset(),
	EMatchPlayCurrentAttackActionSelectionErrorCode::InvalidSkillId)

SIMPLE_ACTION_SELECTION_FAILURE(
	FActionSelectionUndeployedTest,
	"RejectsUndeployedCarrier",
	State.RuntimeState.bIsInitialized = true,
	Request.CarrierCardId = TEXT("PlayerA.NotDeployed"),
	Rules.SkillRules.Reset(),
	EMatchPlayCurrentAttackActionSelectionErrorCode
		::CarrierNotDeployed)

SIMPLE_ACTION_SELECTION_FAILURE(
	FActionSelectionDefenderOnlyTest,
	"DoesNotUseDefenderPlacement",
	State.CurrentAttack.DeploymentPlacements.RemoveAt(0);
	State.CurrentAttack.DeploymentPlacements.RemoveAt(1),
	Request.CarrierCardId = DefenderSameId,
	Rules.SkillRules.Reset(),
	EMatchPlayCurrentAttackActionSelectionErrorCode
		::CarrierNotDeployed)

SIMPLE_ACTION_SELECTION_FAILURE(
	FActionSelectionAmbiguousTest,
	"RejectsDuplicateAttackerPlacement",
	State.CurrentAttack.DeploymentPlacements.Add(
		MakePlacement(
			EInitialTurnOrderPlayer::PlayerA,
			CarrierOneId,
			TEXT("Slot.A3"))),
	Request.SkillId = LongShotSkillId,
	Rules.SkillRules.Reset(),
	EMatchPlayCurrentAttackActionSelectionErrorCode
		::CarrierDeploymentAmbiguous)

SIMPLE_ACTION_SELECTION_FAILURE(
	FActionSelectionMissingSnapshotTest,
	"RejectsMissingSideAwareSnapshot",
	State.CardSnapshotAuthority.PlayerACardSnapshots.Cards.RemoveAt(0),
	Request.SkillId = LongShotSkillId,
	Rules.SkillRules.Reset(),
	EMatchPlayCurrentAttackActionSelectionErrorCode
		::CarrierSnapshotLookupFailed)

SIMPLE_ACTION_SELECTION_FAILURE(
	FActionSelectionInvalidSnapshotAuthorityTest,
	"RejectsInvalidSideAwareSnapshotAuthority",
	const FPlayerCardRuleSnapshot DuplicateSnapshot =
		State.CardSnapshotAuthority.PlayerACardSnapshots.Cards[0];
	State.CardSnapshotAuthority.PlayerACardSnapshots.Cards.Add(
		DuplicateSnapshot),
	Request.SkillId = LongShotSkillId,
	Rules.SkillRules.Reset(),
	EMatchPlayCurrentAttackActionSelectionErrorCode
		::CarrierSnapshotLookupFailed)

SIMPLE_ACTION_SELECTION_FAILURE(
	FActionSelectionGoalkeeperTest,
	"RejectsGoalkeeperCarrier",
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

SIMPLE_ACTION_SELECTION_FAILURE(
	FActionSelectionForeignSkillTest,
	"RejectsSkillNotOwnedByCarrier",
	State.RuntimeState.bIsInitialized = true,
	Request.SkillId = ThroughBallSkillId,
	Rules.SkillRules.Reset(),
	EMatchPlayCurrentAttackActionSelectionErrorCode
		::SkillNotOwnedByCarrier)

SIMPLE_ACTION_SELECTION_FAILURE(
	FActionSelectionInvalidRuleSetTest,
	"RejectsInvalidRuleSetAfterOwnership",
	State.RuntimeState.bIsInitialized = true,
	Request.SkillId = LongShotSkillId,
	Rules.SkillRules[0].SkillType = ESkillRuleType::None,
	EMatchPlayCurrentAttackActionSelectionErrorCode
		::SkillRuleSetValidationFailed)

SIMPLE_ACTION_SELECTION_FAILURE(
	FActionSelectionDuplicateRuleTest,
	"RejectsDuplicateRuleSet",
	State.RuntimeState.bIsInitialized = true,
	Request.SkillId = LongShotSkillId,
	const FSkillRuleSnapshot DuplicateRule = Rules.SkillRules[0];
	Rules.SkillRules.Add(DuplicateRule),
	EMatchPlayCurrentAttackActionSelectionErrorCode
		::SkillRuleSetValidationFailed)

SIMPLE_ACTION_SELECTION_FAILURE(
	FActionSelectionMissingRuleTest,
	"RejectsMissingOwnedSkillRule",
	State.RuntimeState.bIsInitialized = true,
	Request.SkillId = LongShotSkillId,
	Rules.SkillRules.RemoveAt(0),
	EMatchPlayCurrentAttackActionSelectionErrorCode
		::SkillRuleLookupFailed)

SIMPLE_ACTION_SELECTION_FAILURE(
	FActionSelectionInvalidActionPointTest,
	"RejectsAP1WithoutResolvingOpportunityDebt",
	State.CurrentAttack.ActionPoint = 1,
	Request.SkillId = LongShotSkillId,
	Rules.SkillRules[0].MinTriggerActionPoint = 2,
	EMatchPlayCurrentAttackActionSelectionErrorCode
		::InvalidCurrentActionPoint)

SIMPLE_ACTION_SELECTION_FAILURE(
	FActionSelectionBelowRangeTest,
	"RejectsActionPointBelowSkillRange",
	State.CurrentAttack.ActionPoint = 4,
	Request.SkillId = CrossSkillId,
	Rules.SkillRules[3].MinTriggerActionPoint = 5,
	EMatchPlayCurrentAttackActionSelectionErrorCode
		::ActionPointOutsideSkillRange)

SIMPLE_ACTION_SELECTION_FAILURE(
	FActionSelectionAboveRangeTest,
	"RejectsActionPointAboveSkillRange",
	State.CurrentAttack.ActionPoint = 5,
	Request.CarrierCardId = CarrierTwoId;
	Request.SkillId = PassControlSkillId,
	Rules.SkillRules[2].MaxTriggerActionPoint = 4,
	EMatchPlayCurrentAttackActionSelectionErrorCode
		::ActionPointOutsideSkillRange)

ACTION_SELECTION_LEGALITY_TEST(
	FActionSelectionUnderlyingDiagnosticsTest,
	"UnderlyingDiagnosticsAreScoped")

bool FActionSelectionUnderlyingDiagnosticsTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::CurrentAttackActionSelection;
	FMatchPlayState State = MakeState();
	State.CardSnapshotAuthority.PlayerACardSnapshots.Cards.RemoveAt(0);
	const FMatchPlayCurrentAttackActionSelectionLegalityResult Result =
		FMatchPlayCurrentAttackActionSelectionLegalityEvaluator::Evaluate(
			State,
			MakeRequest(),
			MakeSkillRules());
	TestEqual(TEXT("Snapshot top error"), Result.ErrorCode,
		EMatchPlayCurrentAttackActionSelectionErrorCode
			::CarrierSnapshotLookupFailed);
	TestEqual(TEXT("Snapshot underlying error"),
		Result.UnderlyingSnapshotAuthorityQueryErrorCode,
		EMatchPlayCardSnapshotAuthorityQueryErrorCode::SnapshotNotFound);
	TestEqual(TEXT("Unreached validation remains clean"),
		Result.UnderlyingSkillRuleSetValidationErrorCode,
		ESkillRuleSnapshotValidationErrorCode::None);
	TestEqual(TEXT("Unreached query remains clean"),
		Result.UnderlyingSkillRuleQueryErrorCode,
		ESkillRuleSnapshotQueryErrorCode::None);
	return true;
}

#undef SIMPLE_ACTION_SELECTION_FAILURE
#undef ACTION_SELECTION_LEGALITY_TEST

#endif
