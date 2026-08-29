#include "MatchPlayCurrentAttackSkillSelectionTestFixtures.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

namespace SkillSelectionLegalityTests
{
	using namespace
		FMCodex::Tests::MatchPlayCurrentAttackSkillSelection;

	bool AreRuleSetsEqual(
		const FSkillRuleSnapshotSet& Left,
		const FSkillRuleSnapshotSet& Right)
	{
		if (Left.SkillRules.Num() != Right.SkillRules.Num())
		{
			return false;
		}
		for (int32 Index = 0; Index < Left.SkillRules.Num(); ++Index)
		{
			const FSkillRuleSnapshot& LeftRule =
				Left.SkillRules[Index];
			const FSkillRuleSnapshot& RightRule =
				Right.SkillRules[Index];
			if (LeftRule.SkillId != RightRule.SkillId
				|| LeftRule.SkillType != RightRule.SkillType
				|| LeftRule.MinTriggerActionPoint
					!= RightRule.MinTriggerActionPoint
				|| LeftRule.MaxTriggerActionPoint
					!= RightRule.MaxTriggerActionPoint)
			{
				return false;
			}
		}
		return true;
	}

	bool ExpectError(
		FAutomationTestBase& Test,
		const TCHAR* Context,
		const FMatchPlayState& State,
		const FSkillRuleSnapshotSet& Rules,
		const FMatchPlayCurrentAttackSkillSelectionRequest& Request,
		const EMatchPlayCurrentAttackSkillSelectionErrorCode Error)
	{
		const FMatchPlayState OriginalState = State;
		const FSkillRuleSnapshotSet OriginalRules = Rules;
		const auto Result =
			FMatchPlayCurrentAttackSkillSelectionLegalityEvaluator
				::Evaluate(State, Rules, Request);
		Test.TestFalse(Context, Result.bIsLegal);
		Test.TestEqual(
			*FString::Printf(TEXT("%s exact error"), Context),
			Result.ErrorCode,
			Error);
		Test.TestTrue(
			*FString::Printf(TEXT("%s state unchanged"), Context),
			AreStatesEqual(State, OriginalState));
		Test.TestTrue(
			*FString::Printf(TEXT("%s rules unchanged"), Context),
			AreRuleSetsEqual(Rules, OriginalRules));
		return true;
	}

}

#define SKILL_LEGALITY_TEST(TestClass, TestName) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
		TestClass, \
		"FMCodex.CoreRules.MatchPlayCurrentAttackSkillSelectionLegality." TestName, \
		EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

SKILL_LEGALITY_TEST(
	FSkillSelectionAllTypesTest,
	"AllFiveTypesAndClosedRange")

bool FSkillSelectionAllTypesTest::RunTest(const FString& Parameters)
{
	using namespace
		FMCodex::Tests::MatchPlayCurrentAttackSkillSelection;
	struct FCase
	{
		FName SkillId;
		ESkillRuleType Type;
	};
	const FCase Cases[] = {
		{LongShotSkillId, ESkillRuleType::LongShot},
		{CutInsideSkillId, ESkillRuleType::CutInsideShot},
		{PassControlSkillId, ESkillRuleType::PassControl},
		{CrossSkillId, ESkillRuleType::Cross},
		{ThroughBallSkillId, ESkillRuleType::ThroughBall}
	};
	const FSkillRuleSnapshotSet Rules = MakeRuleSet();
	for (const FCase& Case : Cases)
	{
		FMatchPlayState State = MakeState({Case.SkillId});
		State.CurrentAttack.ActionPoint = 2;
		const auto Minimum =
			FMatchPlayCurrentAttackSkillSelectionLegalityEvaluator
				::Evaluate(
					State,
					Rules,
					MakeRequest(Case.SkillId));
		TestTrue(TEXT("AP2 supported type legal"), Minimum.bIsLegal);
		TestEqual(
			TEXT("ActionType derived"),
			Minimum.ResolvedActionType,
			Case.Type);
		State.CurrentAttack.ActionPoint = 8;
		const auto Maximum =
			FMatchPlayCurrentAttackSkillSelectionLegalityEvaluator
				::Evaluate(
					State,
					Rules,
					MakeRequest(Case.SkillId));
		TestTrue(TEXT("AP8 supported type legal"), Maximum.bIsLegal);
	}
	return true;
}

SKILL_LEGALITY_TEST(
	FSkillSelectionStateFirstErrorsTest,
	"StateRequestAndStableFirstErrors")

bool FSkillSelectionStateFirstErrorsTest::RunTest(
	const FString& Parameters)
{
	using namespace
		FMCodex::Tests::MatchPlayCurrentAttackSkillSelection;
	using SkillSelectionLegalityTests::ExpectError;
	const FSkillRuleSnapshotSet Rules = MakeRuleSet();

	{
		FMatchPlayState State = MakeState();
		State.RuntimeState.bIsInitialized = false;
		ExpectError(
			*this,
			TEXT("Uninitialized"),
			State,
			Rules,
			MakeRequest(),
			EMatchPlayCurrentAttackSkillSelectionErrorCode
				::MatchPlayStateNotInitialized);
	}
	{
		FMatchPlayState State = MakeState();
		State.bHasCurrentAttack = false;
		ExpectError(
			*this,
			TEXT("No current attack"),
			State,
			Rules,
			MakeRequest(),
			EMatchPlayCurrentAttackSkillSelectionErrorCode
				::NoCurrentAttack);
	}
	{
		FMatchPlayState State = MakeState();
		State.CurrentAttack.AttackSequence = 0;
		ExpectError(
			*this,
			TEXT("Invalid sequence"),
			State,
			Rules,
			MakeRequest(),
			EMatchPlayCurrentAttackSkillSelectionErrorCode
				::InvalidCurrentAttackSequence);
	}
	{
		auto Request = MakeRequest();
		Request.AttackSequence = ValidAttackSequence + 1;
		Request.RequestingSide = EInitialTurnOrderPlayer::PlayerB;
		ExpectError(
			*this,
			TEXT("Stale sequence wins mixed invalid"),
			MakeState(),
			Rules,
			Request,
			EMatchPlayCurrentAttackSkillSelectionErrorCode
				::AttackSequenceMismatch);
	}
	{
		FMatchPlayState State = MakeState();
		State.CurrentAttack.Phase =
			EMatchPlayCurrentAttackPhase::Deployment;
		ExpectError(
			*this,
			TEXT("Wrong phase"),
			State,
			Rules,
			MakeRequest(),
			EMatchPlayCurrentAttackSkillSelectionErrorCode
				::CurrentAttackNotInResolution);
	}
	{
		FMatchPlayState State = MakeState();
		State.CurrentAttack.ActionPreparation.SkillId =
			LongShotSkillId;
		ExpectError(
			*this,
			TEXT("Corrupt canonical"),
			State,
			Rules,
			MakeRequest(),
			EMatchPlayCurrentAttackSkillSelectionErrorCode
				::InvalidSelectionState);
	}
	{
		FMatchPlayState State = MakeState();
		State.CurrentAttack.SelectionStage =
			EMatchPlayCurrentAttackSelectionStage::AwaitingRunner;
		State.CurrentAttack.ActionPreparation.SkillId =
			CrossSkillId;
		State.CurrentAttack.ActionPreparation.ActionType =
			ESkillRuleType::Cross;
		ExpectError(
			*this,
			TEXT("Wrong stage"),
			State,
			Rules,
			MakeRequest(),
			EMatchPlayCurrentAttackSkillSelectionErrorCode
				::WrongSelectionStage);
	}
	{
		auto Request = MakeRequest();
		Request.RequestingSide = EInitialTurnOrderPlayer::PlayerB;
		ExpectError(
			*this,
			TEXT("Wrong side"),
			MakeState(),
			Rules,
			Request,
			EMatchPlayCurrentAttackSkillSelectionErrorCode
				::RequestingSideIsNotCurrentAttacker);
	}
	{
		auto Request = MakeRequest();
		Request.SkillId = NAME_None;
		ExpectError(
			*this,
			TEXT("Empty skill"),
			MakeState(),
			Rules,
			Request,
			EMatchPlayCurrentAttackSkillSelectionErrorCode
				::InvalidSkillId);
	}
	return true;
}

SKILL_LEGALITY_TEST(
	FSkillSelectionFrozenAuthorityTest,
	"FrozenPlacementsSnapshotAndOwnership")

bool FSkillSelectionFrozenAuthorityTest::RunTest(
	const FString& Parameters)
{
	using namespace
		FMCodex::Tests::MatchPlayCurrentAttackSkillSelection;
	using SkillSelectionLegalityTests::ExpectError;
	const FSkillRuleSnapshotSet Rules = MakeRuleSet();

	{
		FMatchPlayState State = MakeState();
		State.CurrentAttack.ActionPreparation.CarrierCardId = NAME_None;
		ExpectError(
			*this,
			TEXT("Invalid carrier id"),
			State,
			Rules,
			MakeRequest(),
			EMatchPlayCurrentAttackSkillSelectionErrorCode
				::InvalidSelectionState);
	}
	{
		FMatchPlayState State = MakeState();
		State.CurrentAttack.ActionPreparation.MarkerCardId = NAME_None;
		ExpectError(
			*this,
			TEXT("Invalid marker id"),
			State,
			Rules,
			MakeRequest(),
			EMatchPlayCurrentAttackSkillSelectionErrorCode
				::InvalidSelectionState);
	}
	{
		FMatchPlayState State = MakeState();
		State.CurrentAttack.DeploymentPlacements.RemoveAt(0);
		ExpectError(
			*this,
			TEXT("Carrier missing"),
			State,
			Rules,
			MakeRequest(),
			EMatchPlayCurrentAttackSkillSelectionErrorCode
				::FrozenCarrierNotDeployed);
	}
	{
		FMatchPlayState State = MakeState();
		State.CurrentAttack.DeploymentPlacements.Add(
			MakePlacement(
				EInitialTurnOrderPlayer::PlayerA,
				CarrierId,
				TEXT("Slot.AttackerDuplicate")));
		ExpectError(
			*this,
			TEXT("Carrier ambiguous"),
			State,
			Rules,
			MakeRequest(),
			EMatchPlayCurrentAttackSkillSelectionErrorCode
				::FrozenCarrierDeploymentAmbiguous);
	}
	{
		FMatchPlayState State = MakeState();
		State.CurrentAttack.DeploymentPlacements.RemoveAt(1);
		ExpectError(
			*this,
			TEXT("Marker missing"),
			State,
			Rules,
			MakeRequest(),
			EMatchPlayCurrentAttackSkillSelectionErrorCode
				::FrozenMarkerNotDeployed);
	}
	{
		FMatchPlayState State = MakeState();
		State.CurrentAttack.DeploymentPlacements.Add(
			MakePlacement(
				EInitialTurnOrderPlayer::PlayerB,
				MarkerId,
				TEXT("Slot.DefenderDuplicate")));
		ExpectError(
			*this,
			TEXT("Marker ambiguous"),
			State,
			Rules,
			MakeRequest(),
			EMatchPlayCurrentAttackSkillSelectionErrorCode
				::FrozenMarkerDeploymentAmbiguous);
	}
	{
		FMatchPlayState State = MakeState();
		State.CardSnapshotAuthority.PlayerACardSnapshots.Cards.Reset();
		State.CardSnapshotAuthority.PlayerBCardSnapshots.Cards.Add(
			MakeCard(CarrierId, {LongShotSkillId}));
		ExpectError(
			*this,
			TEXT("No cross-side fallback"),
			State,
			Rules,
			MakeRequest(),
			EMatchPlayCurrentAttackSkillSelectionErrorCode
				::CarrierSnapshotQueryFailed);
	}
	{
		FMatchPlayState State = MakeState({CrossSkillId});
		ExpectError(
			*this,
			TEXT("Carrier does not own"),
			State,
			Rules,
			MakeRequest(LongShotSkillId),
			EMatchPlayCurrentAttackSkillSelectionErrorCode
				::CarrierDoesNotOwnSkill);
	}
	{
		FMatchPlayState State =
			MakeState({LongShotSkillId, LongShotSkillId});
		ExpectError(
			*this,
			TEXT("Duplicate carrier skill"),
			State,
			Rules,
			MakeRequest(),
			EMatchPlayCurrentAttackSkillSelectionErrorCode
				::DuplicateCarrierSkillId);
	}
	return true;
}

SKILL_LEGALITY_TEST(
	FSkillSelectionParticipantFirstThroughBallRunnerZoneTest,
	"ParticipantFirstThroughBallUsesRelativeForwardEligibility")

bool FSkillSelectionParticipantFirstThroughBallRunnerZoneTest::RunTest(
	const FString& Parameters)
{
	using namespace
		FMCodex::Tests::MatchPlayCurrentAttackSkillSelection;
	using SkillSelectionLegalityTests::ExpectError;
	const FSkillRuleSnapshotSet Rules = MakeRuleSet();

	const FMatchPlayState Midfield =
		MakeParticipantFirstRunnerState(
			EMatchPlayNeutralSlotSide::NearPlayerA);
	ExpectError(
		*this,
		TEXT("Midfield Runner blocks ThroughBall only at Skill eligibility"),
		Midfield,
		Rules,
		MakeRequest(ThroughBallSkillId),
		EMatchPlayCurrentAttackSkillSelectionErrorCode::
			PreparedRunnerIncompatibleWithSkill);
	const auto PassControl =
		FMatchPlayCurrentAttackSkillSelectionLegalityEvaluator::Evaluate(
			Midfield, Rules, MakeRequest(PassControlSkillId));
	TestTrue(TEXT("Same midfield Runner remains legal for PassControl"),
		PassControl.bIsLegal);

	const FMatchPlayState Forward =
		MakeParticipantFirstRunnerState(
			EMatchPlayNeutralSlotSide::NearPlayerB);
	const auto ThroughBall =
		FMatchPlayCurrentAttackSkillSelectionLegalityEvaluator::Evaluate(
			Forward, Rules, MakeRequest(ThroughBallSkillId));
	TestTrue(TEXT("Relative-forward Runner keeps ThroughBall selectable"),
		ThroughBall.bIsLegal);
	TestEqual(TEXT("ThroughBall action type remains canonical"),
		ThroughBall.ResolvedActionType, ESkillRuleType::ThroughBall);
	return true;
}

SKILL_LEGALITY_TEST(
	FSkillSelectionRuleAndActionPointTest,
	"RuleFailuresAndActionPointBounds")

bool FSkillSelectionRuleAndActionPointTest::RunTest(
	const FString& Parameters)
{
	using namespace
		FMCodex::Tests::MatchPlayCurrentAttackSkillSelection;
	using SkillSelectionLegalityTests::ExpectError;

	{
		FSkillRuleSnapshotSet Rules = MakeRuleSet();
		Rules.SkillRules[0].SkillId = NAME_None;
		ExpectError(
			*this,
			TEXT("Invalid rules"),
			MakeState(),
			Rules,
			MakeRequest(),
			EMatchPlayCurrentAttackSkillSelectionErrorCode
				::InvalidSkillRuleSet);
	}
	{
		FSkillRuleSnapshotSet Rules = MakeRuleSet();
		const FSkillRuleSnapshot DuplicateRule =
			Rules.SkillRules[0];
		Rules.SkillRules.Add(DuplicateRule);
		ExpectError(
			*this,
			TEXT("Duplicate rules"),
			MakeState(),
			Rules,
			MakeRequest(),
			EMatchPlayCurrentAttackSkillSelectionErrorCode
				::SkillRuleAmbiguous);
	}
	{
		FSkillRuleSnapshotSet Rules = MakeRuleSet();
		Rules.SkillRules.RemoveAt(0);
		ExpectError(
			*this,
			TEXT("Missing rule"),
			MakeState(),
			Rules,
			MakeRequest(),
			EMatchPlayCurrentAttackSkillSelectionErrorCode
				::SkillRuleNotFound);
	}
	{
		FSkillRuleSnapshotSet Rules = MakeRuleSet();
		Rules.SkillRules[0].SkillType =
			static_cast<ESkillRuleType>(255);
		ExpectError(
			*this,
			TEXT("Unsupported rule"),
			MakeState(),
			Rules,
			MakeRequest(),
			EMatchPlayCurrentAttackSkillSelectionErrorCode
				::InvalidSkillRuleSet);
	}
	{
		FMatchPlayState State = MakeState();
		State.CurrentAttack.ActionPoint = 1;
		ExpectError(
			*this,
			TEXT("AP1"),
			State,
			MakeRuleSet(),
			MakeRequest(),
			EMatchPlayCurrentAttackSkillSelectionErrorCode
				::InvalidCurrentAttackActionPoint);
		State.CurrentAttack.ActionPoint = 9;
		ExpectError(
			*this,
			TEXT("AP9"),
			State,
			MakeRuleSet(),
			MakeRequest(),
			EMatchPlayCurrentAttackSkillSelectionErrorCode
				::InvalidCurrentAttackActionPoint);
	}
	{
		FMatchPlayState EmptySkillState = MakeState({});
		EmptySkillState.CurrentAttack.ActionPoint = 1;
		auto EmptyRequest = MakeRequest();
		EmptyRequest.SkillId = NAME_None;
		ExpectError(
			*this,
			TEXT("Global AP precedes empty SkillId"),
			EmptySkillState,
			MakeRuleSet(),
			EmptyRequest,
			EMatchPlayCurrentAttackSkillSelectionErrorCode
				::InvalidCurrentAttackActionPoint);

		FMatchPlayState MissingRuleState =
			MakeState({MissingSkillId});
		MissingRuleState.CurrentAttack.ActionPoint = 9;
		ExpectError(
			*this,
			TEXT("Global AP precedes missing Rule"),
			MissingRuleState,
			MakeRuleSet(),
			MakeRequest(MissingSkillId),
			EMatchPlayCurrentAttackSkillSelectionErrorCode
				::InvalidCurrentAttackActionPoint);
	}
	{
		FSkillRuleSnapshotSet Rules;
		Rules.SkillRules = {
			MakeRule(
				LongShotSkillId,
				ESkillRuleType::LongShot,
				4,
				6)
		};
		FMatchPlayState State = MakeState({LongShotSkillId});
		State.CurrentAttack.ActionPoint = 3;
		ExpectError(
			*this,
			TEXT("Below range"),
			State,
			Rules,
			MakeRequest(),
			EMatchPlayCurrentAttackSkillSelectionErrorCode
				::ActionPointOutsideSkillRange);
		State.CurrentAttack.ActionPoint = 4;
		TestTrue(
			TEXT("Equal minimum legal"),
			FMatchPlayCurrentAttackSkillSelectionLegalityEvaluator
				::Evaluate(State, Rules, MakeRequest())
				.bIsLegal);
		State.CurrentAttack.ActionPoint = 6;
		TestTrue(
			TEXT("Equal maximum legal"),
			FMatchPlayCurrentAttackSkillSelectionLegalityEvaluator
				::Evaluate(State, Rules, MakeRequest())
				.bIsLegal);
		State.CurrentAttack.ActionPoint = 7;
		ExpectError(
			*this,
			TEXT("Above range"),
			State,
			Rules,
			MakeRequest(),
			EMatchPlayCurrentAttackSkillSelectionErrorCode
				::ActionPointOutsideSkillRange);
	}
	return true;
}

SKILL_LEGALITY_TEST(
	FSkillSelectionFormalRunnerAbsenceTest,
	"FormalRunnerAbsenceAllowsNoRunnerSkillsOnly")

bool FSkillSelectionFormalRunnerAbsenceTest::RunTest(
	const FString& Parameters)
{
	using namespace
		FMCodex::Tests::MatchPlayCurrentAttackSkillSelection;
	const FSkillRuleSnapshotSet Rules = MakeRuleSet();
	FMatchPlayState State = MakeState(
		{LongShotSkillId, CutInsideSkillId, CrossSkillId});
	State.CurrentAttack.ActionPreparation.bSkillSelectionDeferred = true;

	const auto LongShot =
		FMatchPlayCurrentAttackSkillSelectionLegalityEvaluator::Evaluate(
			State, Rules, MakeRequest(LongShotSkillId));
	TestTrue(TEXT("LongShot is legal without a prepared Runner"),
		LongShot.bIsLegal);
	TestFalse(TEXT("LongShot requires no Runner"),
		LongShot.ParticipantRequirementResult.bRequiresRunner);

	const auto CutInside =
		FMatchPlayCurrentAttackSkillSelectionLegalityEvaluator::Evaluate(
			State, Rules, MakeRequest(CutInsideSkillId));
	TestTrue(TEXT("CutInside remains legal without a prepared Runner"),
		CutInside.bIsLegal);
	TestFalse(TEXT("CutInside requires no Runner"),
		CutInside.ParticipantRequirementResult.bRequiresRunner);

	return SkillSelectionLegalityTests::ExpectError(
		*this,
		TEXT("Cross still requires a prepared Runner"),
		State,
		Rules,
		MakeRequest(CrossSkillId),
		EMatchPlayCurrentAttackSkillSelectionErrorCode
			::PreparedRunnerIncompatibleWithSkill);
}

#undef SKILL_LEGALITY_TEST

#endif
