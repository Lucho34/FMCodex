#include "MatchPlayCurrentAttackSkillSelectionGlobalContextQuery.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "MatchPlayCurrentAttackSkillSelectionTestFixtures.h"
#include "Misc/AutomationTest.h"

namespace SkillSelectionGlobalContextTests
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

	bool ArePlacementsEqual(
		const FMatchPlayDeploymentPlacement& Left,
		const FMatchPlayDeploymentPlacement& Right)
	{
		return FMatchPlayDeploymentPlacement::StaticStruct()
			->CompareScriptStruct(&Left, &Right, 0);
	}

	bool AreSnapshotsEqual(
		const FPlayerCardRuleSnapshot& Left,
		const FPlayerCardRuleSnapshot& Right)
	{
		return FPlayerCardRuleSnapshot::StaticStruct()
			->CompareScriptStruct(&Left, &Right, 0);
	}

	bool ArePlayerSnapshotValidationResultsEqual(
		const FPlayerCardRuleSnapshotValidationResult& Left,
		const FPlayerCardRuleSnapshotValidationResult& Right)
	{
		return Left.bSuccess == Right.bSuccess
			&& Left.bIsValid == Right.bIsValid
			&& Left.ErrorCode == Right.ErrorCode
			&& Left.ErrorMessage == Right.ErrorMessage
			&& Left.InvalidCardId == Right.InvalidCardId
			&& Left.DuplicateCardIds == Right.DuplicateCardIds;
	}

	bool ArePlayerSnapshotQueryResultsEqual(
		const FPlayerCardRuleSnapshotQueryResult& Left,
		const FPlayerCardRuleSnapshotQueryResult& Right)
	{
		return Left.bSuccess == Right.bSuccess
			&& Left.bFound == Right.bFound
			&& Left.ErrorCode == Right.ErrorCode
			&& Left.ErrorMessage == Right.ErrorMessage
			&& Left.CardId == Right.CardId
			&& AreSnapshotsEqual(Left.Snapshot, Right.Snapshot)
			&& ArePlayerSnapshotValidationResultsEqual(
				Left.ValidationResult,
				Right.ValidationResult);
	}

	bool AreCarrierSnapshotQueryResultsEqual(
		const FMatchPlayCardSnapshotAuthorityQueryResult& Left,
		const FMatchPlayCardSnapshotAuthorityQueryResult& Right)
	{
		return Left.bSuccess == Right.bSuccess
			&& Left.ErrorCode == Right.ErrorCode
			&& Left.PlayerSide == Right.PlayerSide
			&& Left.CardId == Right.CardId
			&& AreSnapshotsEqual(Left.Snapshot, Right.Snapshot)
			&& ArePlayerSnapshotQueryResultsEqual(
				Left.UnderlyingQueryResult,
				Right.UnderlyingQueryResult)
			&& Left.ErrorMessage == Right.ErrorMessage;
	}

	bool AreSkillValidationResultsEqual(
		const FSkillRuleSnapshotValidationResult& Left,
		const FSkillRuleSnapshotValidationResult& Right)
	{
		return Left.bSuccess == Right.bSuccess
			&& Left.bIsValid == Right.bIsValid
			&& Left.ErrorCode == Right.ErrorCode
			&& Left.ErrorMessage == Right.ErrorMessage
			&& Left.InvalidSkillId == Right.InvalidSkillId
			&& Left.InvalidField == Right.InvalidField;
	}

	bool AreResultsEqual(
		const FMatchPlayCurrentAttackSkillSelectionGlobalContextResult&
			Left,
		const FMatchPlayCurrentAttackSkillSelectionGlobalContextResult&
			Right)
	{
		return Left.bSuccess == Right.bSuccess
			&& Left.RequestedAttackSequence
				== Right.RequestedAttackSequence
			&& Left.RequestingSide == Right.RequestingSide
			&& Left.ErrorCode == Right.ErrorCode
			&& Left.AuthoritativeAttackSequence
				== Right.AuthoritativeAttackSequence
			&& Left.CurrentAttackingPlayer
				== Right.CurrentAttackingPlayer
			&& Left.CurrentDefendingPlayer
				== Right.CurrentDefendingPlayer
			&& Left.SelectionStateValidationResult.bIsCanonical
				== Right.SelectionStateValidationResult.bIsCanonical
			&& Left.SelectionStateValidationResult.ErrorCode
				== Right.SelectionStateValidationResult.ErrorCode
			&& Left.SelectionStateValidationResult.ErrorMessage
				== Right.SelectionStateValidationResult.ErrorMessage
			&& Left.FrozenCarrierCardId
				== Right.FrozenCarrierCardId
			&& Left.FrozenMarkerCardId
				== Right.FrozenMarkerCardId
			&& Left.MatchingFrozenCarrierPlacementCount
				== Right.MatchingFrozenCarrierPlacementCount
			&& Left.MatchingFrozenMarkerPlacementCount
				== Right.MatchingFrozenMarkerPlacementCount
			&& ArePlacementsEqual(
				Left.FrozenCarrierPlacement,
				Right.FrozenCarrierPlacement)
			&& ArePlacementsEqual(
				Left.FrozenMarkerPlacement,
				Right.FrozenMarkerPlacement)
			&& AreCarrierSnapshotQueryResultsEqual(
				Left.CarrierSnapshotQueryResult,
				Right.CarrierSnapshotQueryResult)
			&& AreSnapshotsEqual(
				Left.ResolvedCarrierSnapshot,
				Right.ResolvedCarrierSnapshot)
			&& AreSkillValidationResultsEqual(
				Left.SkillRuleSetValidationResult,
				Right.SkillRuleSetValidationResult)
			&& Left.ValidatedActionPoint
				== Right.ValidatedActionPoint
			&& Left.ErrorMessage == Right.ErrorMessage;
	}

	bool ExpectError(
		FAutomationTestBase& Test,
		const TCHAR* Context,
		const FMatchPlayState& State,
		const int64 AttackSequence,
		const EInitialTurnOrderPlayer RequestingSide,
		const FSkillRuleSnapshotSet& Rules,
		const EMatchPlayCurrentAttackSkillSelectionErrorCode
			ExpectedError)
	{
		const auto Result =
			FMatchPlayCurrentAttackSkillSelectionGlobalContextQuery
				::Query(
					State,
					AttackSequence,
					RequestingSide,
					Rules);
		Test.TestFalse(Context, Result.bSuccess);
		Test.TestEqual(
			*FString::Printf(TEXT("%s exact error"), Context),
			Result.ErrorCode,
			ExpectedError);
		Test.TestTrue(
			*FString::Printf(TEXT("%s diagnostics"), Context),
			!Result.ErrorMessage.IsEmpty());
		return true;
	}
}

#define SKILL_GLOBAL_CONTEXT_TEST(TestClass, TestName) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
		TestClass, \
		"FMCodex.CoreRules.MatchPlayCurrentAttackSkillSelectionGlobalContext." TestName, \
		EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

SKILL_GLOBAL_CONTEXT_TEST(
	FSkillGlobalContextStateContractTest,
	"ValidBoundariesAndStateFailures")

bool FSkillGlobalContextStateContractTest::RunTest(
	const FString& Parameters)
{
	using namespace
		FMCodex::Tests::MatchPlayCurrentAttackSkillSelection;
	using namespace SkillSelectionGlobalContextTests;
	const FSkillRuleSnapshotSet Rules = MakeRuleSet();

	for (const int32 ActionPoint : {2, 8})
	{
		FMatchPlayState State = MakeState({});
		State.CurrentAttack.ActionPoint = ActionPoint;
		const auto Result =
			FMatchPlayCurrentAttackSkillSelectionGlobalContextQuery
				::Query(
					State,
					ValidAttackSequence,
					EInitialTurnOrderPlayer::PlayerA,
					Rules);
		TestTrue(TEXT("Boundary context succeeds"), Result.bSuccess);
		TestEqual(
			TEXT("Validated AP retained"),
			Result.ValidatedActionPoint,
			ActionPoint);
		TestEqual(
			TEXT("Empty carrier skills retained"),
			Result.ResolvedCarrierSnapshot.SkillIds.Num(),
			0);
	}

	{
		FMatchPlayState State = MakeState();
		State.RuntimeState.bIsInitialized = false;
		ExpectError(
			*this,
			TEXT("Runtime invalid"),
			State,
			ValidAttackSequence,
			EInitialTurnOrderPlayer::PlayerA,
			Rules,
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
			ValidAttackSequence,
			EInitialTurnOrderPlayer::PlayerA,
			Rules,
			EMatchPlayCurrentAttackSkillSelectionErrorCode
				::NoCurrentAttack);
	}
	{
		FMatchPlayState State = MakeState();
		State.CurrentAttack.AttackSequence = 0;
		ExpectError(
			*this,
			TEXT("Invalid authoritative sequence"),
			State,
			ValidAttackSequence,
			EInitialTurnOrderPlayer::PlayerA,
			Rules,
			EMatchPlayCurrentAttackSkillSelectionErrorCode
				::InvalidCurrentAttackSequence);
	}
	ExpectError(
		*this,
		TEXT("Stale sequence"),
		MakeState(),
		ValidAttackSequence + 1,
		EInitialTurnOrderPlayer::PlayerA,
		Rules,
		EMatchPlayCurrentAttackSkillSelectionErrorCode
			::AttackSequenceMismatch);
	{
		FMatchPlayState State = MakeState();
		State.CurrentAttack.Phase =
			EMatchPlayCurrentAttackPhase::Deployment;
		ExpectError(
			*this,
			TEXT("Wrong phase"),
			State,
			ValidAttackSequence,
			EInitialTurnOrderPlayer::PlayerA,
			Rules,
			EMatchPlayCurrentAttackSkillSelectionErrorCode
				::CurrentAttackNotInResolution);
	}
	{
		FMatchPlayState State = MakeState();
		State.CurrentAttack.ActionPreparation.SkillId =
			LongShotSkillId;
		ExpectError(
			*this,
			TEXT("Invalid canonical state"),
			State,
			ValidAttackSequence,
			EInitialTurnOrderPlayer::PlayerA,
			Rules,
			EMatchPlayCurrentAttackSkillSelectionErrorCode
				::InvalidSelectionState);
	}
	{
		FMatchPlayState State = MakeState();
		State.CurrentAttack.SelectionStage =
			EMatchPlayCurrentAttackSelectionStage::AwaitingRunner;
		State.CurrentAttack.ActionPreparation.SkillId = CrossSkillId;
		State.CurrentAttack.ActionPreparation.ActionType =
			ESkillRuleType::Cross;
		ExpectError(
			*this,
			TEXT("Wrong stage"),
			State,
			ValidAttackSequence,
			EInitialTurnOrderPlayer::PlayerA,
			Rules,
			EMatchPlayCurrentAttackSkillSelectionErrorCode
				::WrongSelectionStage);
	}
	ExpectError(
		*this,
		TEXT("Invalid requesting side"),
		MakeState(),
		ValidAttackSequence,
		EInitialTurnOrderPlayer::None,
		Rules,
		EMatchPlayCurrentAttackSkillSelectionErrorCode
			::InvalidRequestingSide);
	ExpectError(
		*this,
		TEXT("Wrong requesting side"),
		MakeState(),
		ValidAttackSequence,
		EInitialTurnOrderPlayer::PlayerB,
		Rules,
		EMatchPlayCurrentAttackSkillSelectionErrorCode
			::RequestingSideIsNotCurrentAttacker);
	return true;
}

SKILL_GLOBAL_CONTEXT_TEST(
	FSkillGlobalContextAuthorityFailureTest,
	"PlacementsSnapshotsRulesAndActionPoint")

bool FSkillGlobalContextAuthorityFailureTest::RunTest(
	const FString& Parameters)
{
	using namespace
		FMCodex::Tests::MatchPlayCurrentAttackSkillSelection;
	using namespace SkillSelectionGlobalContextTests;
	const FSkillRuleSnapshotSet Rules = MakeRuleSet();

	{
		FMatchPlayState State = MakeState();
		State.CurrentAttack.DeploymentPlacements.RemoveAt(0);
		ExpectError(
			*this,
			TEXT("Carrier missing"),
			State,
			ValidAttackSequence,
			EInitialTurnOrderPlayer::PlayerA,
			Rules,
			EMatchPlayCurrentAttackSkillSelectionErrorCode
				::FrozenCarrierNotDeployed);
	}
	{
		FMatchPlayState State = MakeState();
		State.CurrentAttack.DeploymentPlacements.Add(
			MakePlacement(
				EInitialTurnOrderPlayer::PlayerA,
				CarrierId,
				TEXT("Slot.CarrierDuplicate")));
		ExpectError(
			*this,
			TEXT("Carrier ambiguous"),
			State,
			ValidAttackSequence,
			EInitialTurnOrderPlayer::PlayerA,
			Rules,
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
			ValidAttackSequence,
			EInitialTurnOrderPlayer::PlayerA,
			Rules,
			EMatchPlayCurrentAttackSkillSelectionErrorCode
				::FrozenMarkerNotDeployed);
	}
	{
		FMatchPlayState State = MakeState();
		State.CurrentAttack.DeploymentPlacements.Add(
			MakePlacement(
				EInitialTurnOrderPlayer::PlayerB,
				MarkerId,
				TEXT("Slot.MarkerDuplicate")));
		ExpectError(
			*this,
			TEXT("Marker ambiguous"),
			State,
			ValidAttackSequence,
			EInitialTurnOrderPlayer::PlayerA,
			Rules,
			EMatchPlayCurrentAttackSkillSelectionErrorCode
				::FrozenMarkerDeploymentAmbiguous);
	}
	{
		FMatchPlayState State = MakeState();
		State.CardSnapshotAuthority.PlayerACardSnapshots.Cards.Reset();
		ExpectError(
			*this,
			TEXT("Carrier snapshot failure"),
			State,
			ValidAttackSequence,
			EInitialTurnOrderPlayer::PlayerA,
			Rules,
			EMatchPlayCurrentAttackSkillSelectionErrorCode
				::CarrierSnapshotQueryFailed);
	}
	ExpectError(
		*this,
		TEXT("Duplicate carrier skill"),
		MakeState({LongShotSkillId, LongShotSkillId}),
		ValidAttackSequence,
		EInitialTurnOrderPlayer::PlayerA,
		Rules,
		EMatchPlayCurrentAttackSkillSelectionErrorCode
			::DuplicateCarrierSkillId);
	{
		FSkillRuleSnapshotSet InvalidRules = MakeRuleSet();
		InvalidRules.SkillRules[0].SkillId = NAME_None;
		ExpectError(
			*this,
			TEXT("Invalid rule set"),
			MakeState(),
			ValidAttackSequence,
			EInitialTurnOrderPlayer::PlayerA,
			InvalidRules,
			EMatchPlayCurrentAttackSkillSelectionErrorCode
				::InvalidSkillRuleSet);
	}
	{
		FSkillRuleSnapshotSet DuplicateRules = MakeRuleSet();
		const FSkillRuleSnapshot DuplicateRule =
			DuplicateRules.SkillRules[0];
		DuplicateRules.SkillRules.Add(DuplicateRule);
		ExpectError(
			*this,
			TEXT("Duplicate rule"),
			MakeState(),
			ValidAttackSequence,
			EInitialTurnOrderPlayer::PlayerA,
			DuplicateRules,
			EMatchPlayCurrentAttackSkillSelectionErrorCode
				::SkillRuleAmbiguous);
	}
	for (const int32 ActionPoint : {1, 9})
	{
		FMatchPlayState State = MakeState({});
		State.CurrentAttack.ActionPoint = ActionPoint;
		ExpectError(
			*this,
			TEXT("Invalid global AP"),
			State,
			ValidAttackSequence,
			EInitialTurnOrderPlayer::PlayerA,
			Rules,
			EMatchPlayCurrentAttackSkillSelectionErrorCode
				::InvalidCurrentAttackActionPoint);
	}
	return true;
}

SKILL_GLOBAL_CONTEXT_TEST(
	FSkillGlobalContextDeterminismTest,
	"ReadOnlyAndFullResultDeterminism")

bool FSkillGlobalContextDeterminismTest::RunTest(
	const FString& Parameters)
{
	using namespace
		FMCodex::Tests::MatchPlayCurrentAttackSkillSelection;
	using namespace SkillSelectionGlobalContextTests;
	const FMatchPlayState State = MakeState({});
	const FMatchPlayState OriginalState = State;
	const FSkillRuleSnapshotSet Rules = MakeRuleSet();
	const FSkillRuleSnapshotSet OriginalRules = Rules;

	const auto First =
		FMatchPlayCurrentAttackSkillSelectionGlobalContextQuery::Query(
			State,
			ValidAttackSequence,
			EInitialTurnOrderPlayer::PlayerA,
			Rules);
	const auto Second =
		FMatchPlayCurrentAttackSkillSelectionGlobalContextQuery::Query(
			State,
			ValidAttackSequence,
			EInitialTurnOrderPlayer::PlayerA,
			Rules);
	TestTrue(
		TEXT("Full global result deterministic"),
		AreResultsEqual(First, Second));
	TestTrue(
		TEXT("State unchanged"),
		AreStatesEqual(State, OriginalState));
	TestTrue(
		TEXT("Rule set unchanged"),
		AreRuleSetsEqual(Rules, OriginalRules));
	return true;
}

#undef SKILL_GLOBAL_CONTEXT_TEST

#endif
