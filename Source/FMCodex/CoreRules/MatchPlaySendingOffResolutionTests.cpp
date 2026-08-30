#include "MatchPlaySendingOffResolution.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

namespace MatchPlaySendingOffResolutionTests
{
	const FName A1(TEXT("A1"));
	const FName A2(TEXT("A2"));
	const FName A3(TEXT("A3"));
	const FName AGK(TEXT("AGK"));
	const FName B1(TEXT("B1"));
	const FName BGK(TEXT("BGK"));

	FPlayerCardRuleSnapshot MakeSnapshot(
		const FName CardId,
		const bool bGoalkeeper = false)
	{
		FPlayerCardRuleSnapshot Snapshot;
		Snapshot.CardId = CardId;
		Snapshot.bIsGoalkeeper = bGoalkeeper;
		Snapshot.bHasGoalkeeperAttributes = bGoalkeeper;
		Snapshot.PositionTypes = {
			bGoalkeeper
				? EPlayerPositionType::Goalkeeper
				: EPlayerPositionType::Attack
		};
		return Snapshot;
	}

	FMatchPlayState MakePendingState(
		const TArray<FName>& Available,
		const TArray<FName>& Used = {},
		const TArray<FName>& Ejected = {},
		const int32 PlayerATotalAttacks = 2,
		const int32 PlayerBTotalAttacks = 2)
	{
		FMatchPlayState State;
		State.RuntimeState.bIsInitialized = true;
		State.RuntimeState.CurrentAttackingPlayer =
			EInitialTurnOrderPlayer::PlayerA;
		State.RuntimeState.PlayerAState.TotalAttackCount =
			PlayerATotalAttacks;
		State.RuntimeState.PlayerBState.TotalAttackCount =
			PlayerBTotalAttacks;
		State.CardUsageState.PlayerACardUsageState.AvailableCardIds =
			Available;
		State.CardUsageState.PlayerACardUsageState.UsedCardIds = Used;
		State.CardUsageState.PlayerACardUsageState.EjectedCardIds =
			Ejected;
		State.CardUsageState.PlayerBCardUsageState.AvailableCardIds =
			{ B1, BGK };
		State.CardSnapshotAuthority.PlayerACardSnapshots.Cards = {
			MakeSnapshot(A1),
			MakeSnapshot(A2),
			MakeSnapshot(A3),
			MakeSnapshot(AGK, true)
		};
		State.CardSnapshotAuthority.PlayerBCardSnapshots.Cards = {
			MakeSnapshot(B1),
			MakeSnapshot(BGK, true)
		};
		State.bHasCurrentAttack = true;
		State.CurrentAttack.AttackSequence = 1;
		State.CurrentAttack.ActionPoint = 1;
		State.CurrentAttack.RawInitialD12 = 1;
		State.CurrentAttack.Phase =
			EMatchPlayCurrentAttackPhase::RoutePending;
		State.CurrentAttack.RouteKind =
			EMatchPlayCurrentAttackRouteKind::SendingOff;
		State.CurrentAttack.SendingOffRoute.Stage =
			EMatchPlaySendingOffRouteStage::AwaitingResolution;
		return State;
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

	class FSelectionProvider final : public IMatchPlayAttackEntryRollProvider
	{
	public:
		virtual FMatchPlayAttackEntryRollProviderResult RollD12(
			EMatchPlayAttackEntryRollPurpose Purpose) override
		{
			return FMatchPlayAttackEntryRollProviderResult();
		}

		virtual FMatchPlayAttackEntryRollProviderResult RollD6(
			EMatchPlayAttackEntryRollPurpose Purpose) override
		{
			return FMatchPlayAttackEntryRollProviderResult();
		}

		virtual FMatchPlayAttackEntrySelectionProviderResult SelectUniformIndex(
			const EMatchPlayAttackEntryRollPurpose Purpose,
			const int32 CandidateCount) override
		{
			++CallCount;
			LastPurpose = Purpose;
			LastCandidateCount = CandidateCount;
			return NextResult;
		}

		int32 CallCount = 0;
		int32 LastCandidateCount = 0;
		EMatchPlayAttackEntryRollPurpose LastPurpose =
			EMatchPlayAttackEntryRollPurpose::None;
		FMatchPlayAttackEntrySelectionProviderResult NextResult;
	};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayEjectedStateInvariantTest,
	"FMCodex.CoreRules.MatchPlaySendingOff.EjectedStateInvariants",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayEjectedStateInvariantTest::RunTest(const FString& Parameters)
{
	using namespace MatchPlaySendingOffResolutionTests;

	const FCardUsageState DefaultUsage;
	TestTrue(TEXT("Default Ejected is empty"),
		DefaultUsage.EjectedCardIds.IsEmpty());

	const FMatchPlayState Valid = MakePendingState(
		{ A1, AGK },
		{ A2 },
		{ A3 });
	TestTrue(TEXT("Valid three-zone state passes"),
		FMatchPlayCardUsageStateValidator::Validate(
			Valid.CardUsageState,
			Valid.CardSnapshotAuthority).bIsCanonical);
	TestTrue(TEXT("PlayerA ejection is side-owned"),
		Valid.CardUsageState.PlayerACardUsageState.EjectedCardIds.Contains(A3));
	TestTrue(TEXT("PlayerB ejection remains independent"),
		Valid.CardUsageState.PlayerBCardUsageState.EjectedCardIds.IsEmpty());
	TestEqual(TEXT("Ordinary play rejects an Ejected card explicitly"),
		FPlayCardResolver::ValidateCanPlayCard(
			Valid.CardUsageState,
			EInitialTurnOrderPlayer::PlayerA,
			A3).ErrorCode,
		EPlayCardResolveErrorCode::CardAlreadyEjected);

	FMatchPlayState Duplicate = Valid;
	Duplicate.CardUsageState.PlayerACardUsageState.EjectedCardIds.Add(A3);
	TestEqual(TEXT("Duplicate Ejected is rejected"),
		FMatchPlayCardUsageStateValidator::Validate(
			Duplicate.CardUsageState,
			Duplicate.CardSnapshotAuthority).UnderlyingCardUsageErrorCode,
		ECardUsageResolveErrorCode::DuplicateCardInEjected);

	FMatchPlayState AvailableOverlap = Valid;
	AvailableOverlap.CardUsageState.PlayerACardUsageState
		.AvailableCardIds.Add(A3);
	TestEqual(TEXT("Available/Ejected overlap is rejected"),
		FMatchPlayCardUsageStateValidator::Validate(
			AvailableOverlap.CardUsageState,
			AvailableOverlap.CardSnapshotAuthority).UnderlyingCardUsageErrorCode,
		ECardUsageResolveErrorCode::CardExistsInBothAvailableAndEjected);

	FMatchPlayState UsedOverlap = Valid;
	UsedOverlap.CardUsageState.PlayerACardUsageState.UsedCardIds.Add(A3);
	TestEqual(TEXT("Used/Ejected overlap is rejected"),
		FMatchPlayCardUsageStateValidator::Validate(
			UsedOverlap.CardUsageState,
			UsedOverlap.CardSnapshotAuthority).UnderlyingCardUsageErrorCode,
		ECardUsageResolveErrorCode::CardExistsInBothUsedAndEjected);

	FMatchPlayState Missing = Valid;
	Missing.CardUsageState.PlayerACardUsageState.EjectedCardIds =
		{ FName(TEXT("UNKNOWN")) };
	TestEqual(TEXT("Missing provenance is rejected"),
		FMatchPlayCardUsageStateValidator::Validate(
			Missing.CardUsageState,
			Missing.CardSnapshotAuthority).ErrorCode,
		EMatchPlayCardUsageStateValidationErrorCode::CardSnapshotQueryFailed);

	FMatchPlayState Goalkeeper = Valid;
	Goalkeeper.CardUsageState.PlayerACardUsageState.AvailableCardIds.Remove(AGK);
	Goalkeeper.CardUsageState.PlayerACardUsageState.EjectedCardIds = { AGK };
	TestEqual(TEXT("GK in Ejected is rejected"),
		FMatchPlayCardUsageStateValidator::Validate(
			Goalkeeper.CardUsageState,
			Goalkeeper.CardSnapshotAuthority).ErrorCode,
		EMatchPlayCardUsageStateValidationErrorCode::GoalkeeperInEjectedZone);

	const FMatchPlayState Copy = Valid;
	TestTrue(TEXT("Reflected/value copy preserves Ejected"),
		AreStatesEqual(Copy, Valid));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlaySendingOffCandidateQueryTest,
	"FMCodex.CoreRules.MatchPlaySendingOff.CandidateQuery",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlaySendingOffCandidateQueryTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlaySendingOffResolutionTests;

	const FMatchPlayState State = MakePendingState(
		{ A2, AGK, A1 },
		{},
		{ A3 });
	const FMatchPlayState Before = State;
	const FMatchPlaySendingOffCandidateQueryResult Query =
		FMatchPlaySendingOffCandidateQuery::Query(State);
	TestTrue(TEXT("Candidate query succeeds"), Query.bSuccess);
	TestEqual(TEXT("Only Available non-GK cards are included"),
		Query.CandidateCardIds.Num(), 2);
	TestEqual(TEXT("Available order is stable at index 0"),
		Query.CandidateCardIds[0], A2);
	TestEqual(TEXT("Available order is stable at index 1"),
		Query.CandidateCardIds[1], A1);
	TestFalse(TEXT("GK is excluded"), Query.CandidateCardIds.Contains(AGK));
	TestFalse(TEXT("Ejected is excluded"), Query.CandidateCardIds.Contains(A3));
	TestFalse(TEXT("Opponent card is excluded"), Query.CandidateCardIds.Contains(B1));
	TestTrue(TEXT("Candidate query is pure"), AreStatesEqual(State, Before));

	const FMatchPlaySendingOffCandidateQueryResult Empty =
		FMatchPlaySendingOffCandidateQuery::Query(
			MakePendingState({ AGK }, { A1, A2 }, { A3 }));
	TestTrue(TEXT("Zero pool is a successful query"), Empty.bSuccess);
	TestTrue(TEXT("Zero pool is explicit empty"),
		Empty.CandidateCardIds.IsEmpty());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlaySendingOffResolutionLifecycleTest,
	"FMCodex.CoreRules.MatchPlaySendingOff.ResolutionTerminalAdvance",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlaySendingOffResolutionLifecycleTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlaySendingOffResolutionTests;

	FSelectionProvider Provider;
	FMatchPlaySendingOffResolutionRequest Request;
	Request.AttackSequence = 1;
	FSelectionProvider RejectedProvider;
	RejectedProvider.NextResult.bSuccess = true;
	RejectedProvider.NextResult.SelectedIndex = 0;
	FMatchPlaySendingOffResolutionRequest StaleResolveRequest;
	StaleResolveRequest.AttackSequence = 2;
	const FMatchPlayState RejectedBefore =
		MakePendingState({ A1, A2 });
	const auto RejectedResolve = FMatchPlaySendingOffResolution::Resolve(
		RejectedBefore,
		StaleResolveRequest,
		&RejectedProvider);
	TestEqual(TEXT("Stale AP1 request consumes no provider"),
		RejectedProvider.CallCount, 0);
	TestTrue(TEXT("Stale AP1 request preserves state"),
		AreStatesEqual(RejectedResolve.AfterState, RejectedBefore));

	const FMatchPlayState Pool0Before =
		MakePendingState({ AGK }, { A1, A2 }, { A3 });
	const FMatchPlaySendingOffResolutionResult Pool0 =
		FMatchPlaySendingOffResolution::Resolve(
			Pool0Before,
			Request,
			&Provider);
	TestTrue(TEXT("Pool0 resolves"), Pool0.bSuccess);
	TestEqual(TEXT("Pool0 consumes zero selection RNG"), Provider.CallCount, 0);
	TestEqual(TEXT("Pool0 stores NoEligibleCandidate"),
		Pool0.AfterState.CurrentAttack.SendingOffRoute.SelectionOutcome,
		EMatchPlaySendingOffSelectionOutcome::NoEligibleCandidate);
	TestEqual(TEXT("Pool0 stores NoGoal"),
		Pool0.AfterState.CurrentAttack.SendingOffRoute.GameplayOutcome,
		EMatchPlaySendingOffGameplayOutcome::NoGoal);
	TestEqual(TEXT("Pool0 is terminal"),
		Pool0.AfterState.CurrentAttack.LifecycleState,
		EMatchPlayCurrentAttackLifecycleState::TerminalPendingAdvance);
	TestEqual(TEXT("Pool0 score unchanged"),
		Pool0.AfterState.RuntimeState.PlayerAState.Score,
		Pool0Before.RuntimeState.PlayerAState.Score);
	TestEqual(TEXT("Pool0 opportunity unchanged before advance"),
		Pool0.AfterState.RuntimeState.PlayerAState.UsedAttackCount, 0);

	const FMatchPlayState Pool1Before =
		MakePendingState({ AGK, A2 }, { A1 }, { A3 });
	const FMatchPlaySendingOffResolutionResult Pool1 =
		FMatchPlaySendingOffResolution::Resolve(
			Pool1Before,
			Request,
			&Provider);
	TestTrue(TEXT("Pool1 resolves"), Pool1.bSuccess);
	TestEqual(TEXT("Pool1 consumes zero selection RNG"), Provider.CallCount, 0);
	TestEqual(TEXT("Only candidate is ejected"), Pool1.EjectedCardId, A2);
	TestFalse(TEXT("Ejected card leaves Available"),
		Pool1.AfterState.CardUsageState.PlayerACardUsageState
			.AvailableCardIds.Contains(A2));
	TestTrue(TEXT("Ejected card enters Ejected"),
		Pool1.AfterState.CardUsageState.PlayerACardUsageState
			.EjectedCardIds.Contains(A2));
	TestFalse(TEXT("Ejected card never enters Used"),
		Pool1.AfterState.CardUsageState.PlayerACardUsageState
			.UsedCardIds.Contains(A2));

	Provider.NextResult.bSuccess = true;
	Provider.NextResult.SelectedIndex = 1;
	const FMatchPlayState PoolManyBefore =
		MakePendingState({ A1, AGK, A2, A3 });
	const FMatchPlaySendingOffResolutionResult PoolMany =
		FMatchPlaySendingOffResolution::Resolve(
			PoolManyBefore,
			Request,
			&Provider);
	TestTrue(TEXT("Pool2+ resolves"), PoolMany.bSuccess);
	TestEqual(TEXT("Pool2+ consumes exactly one selection RNG"),
		Provider.CallCount, 1);
	TestEqual(TEXT("Provider receives eligible count only"),
		Provider.LastCandidateCount, 3);
	TestEqual(TEXT("Provider receives semantic purpose"),
		Provider.LastPurpose,
		EMatchPlayAttackEntryRollPurpose::SendingOffSelection);
	TestEqual(TEXT("Provider-selected legal card is ejected"),
		PoolMany.EjectedCardId, A2);
	TestTrue(TEXT("Terminal route validates"),
		FMatchPlayCurrentAttackRouteStateValidator::Validate(
			PoolMany.AfterState).bIsCanonical);

	const FMatchPlayCurrentAttackCompletionResult WrongAdvance =
		FMatchPlayCurrentAttackCompletion::AdvanceAfterTerminal(
			PoolMany.AfterState,
			1,
			EInitialTurnOrderPlayer::PlayerB);
	TestEqual(TEXT("Wrong-side advance is rejected"),
		WrongAdvance.ErrorCode,
		EMatchPlayCurrentAttackCompletionErrorCode::UnauthorizedAdvanceRequester);
	const FMatchPlayCurrentAttackCompletionResult StaleAdvance =
		FMatchPlayCurrentAttackCompletion::AdvanceAfterTerminal(
			PoolMany.AfterState,
			2,
			EInitialTurnOrderPlayer::PlayerA);
	TestEqual(TEXT("Stale advance is rejected"),
		StaleAdvance.ErrorCode,
		EMatchPlayCurrentAttackCompletionErrorCode::CapabilitySequenceMismatch);
	const FMatchPlayCurrentAttackCompletionResult Advance =
		FMatchPlayCurrentAttackCompletion::AdvanceAfterTerminal(
			PoolMany.AfterState,
			1,
			EInitialTurnOrderPlayer::PlayerA);
	TestTrue(TEXT("AP1 shared advance succeeds"), Advance.bSuccess);
	TestEqual(TEXT("AP1 consumes exactly one opportunity"),
		Advance.AfterState.RuntimeState.PlayerAState.UsedAttackCount, 1);
	TestFalse(TEXT("Advance clears current attack"),
		Advance.AfterState.bHasCurrentAttack);
	TestTrue(TEXT("Advance preserves Ejected"),
		Advance.AfterState.CardUsageState.PlayerACardUsageState
			.EjectedCardIds.Contains(A2));
	TestEqual(TEXT("Advance consumes no ordinary placements"),
		Advance.OrdinaryCardUsageResults.Num(), 0);
	const FMatchPlayCurrentAttackCompletionResult DuplicateAdvance =
		FMatchPlayCurrentAttackCompletion::AdvanceAfterTerminal(
			Advance.AfterState,
			1,
			EInitialTurnOrderPlayer::PlayerA);
	TestEqual(TEXT("Duplicate advance cannot consume twice"),
		DuplicateAdvance.ErrorCode,
		EMatchPlayCurrentAttackCompletionErrorCode::NoCurrentAttack);

	FSelectionProvider FailureProvider;
	FailureProvider.NextResult.ErrorCode =
		EMatchPlayAttackEntryRollProviderErrorCode::ProviderFailure;
	FailureProvider.NextResult.ErrorMessage = TEXT("retryable");
	const FMatchPlaySendingOffResolutionResult Failed =
		FMatchPlaySendingOffResolution::Resolve(
			PoolManyBefore,
			Request,
			&FailureProvider);
	TestEqual(TEXT("Provider failure is explicit"), Failed.ErrorCode,
		EMatchPlaySendingOffResolutionErrorCode::SelectionProviderFailure);
	TestTrue(TEXT("Provider failure is atomic"),
		AreStatesEqual(Failed.AfterState, PoolManyBefore));

	FSelectionProvider MalformedProvider;
	MalformedProvider.NextResult.bSuccess = true;
	MalformedProvider.NextResult.SelectedIndex = 99;
	const FMatchPlaySendingOffResolutionResult Malformed =
		FMatchPlaySendingOffResolution::Resolve(
			PoolManyBefore,
			Request,
			&MalformedProvider);
	TestEqual(TEXT("Malformed index is rejected"), Malformed.ErrorCode,
		EMatchPlaySendingOffResolutionErrorCode
			::MalformedSelectionProviderResult);
	TestTrue(TEXT("Malformed selection is atomic"),
		AreStatesEqual(Malformed.AfterState, PoolManyBefore));

	FSelectionProvider FinalProvider;
	FinalProvider.NextResult.bSuccess = true;
	FinalProvider.NextResult.SelectedIndex = 0;
	const FMatchPlayState FinalBefore =
		MakePendingState({ A1, A2 }, {}, {}, 1, 0);
	const auto FinalTerminal = FMatchPlaySendingOffResolution::Resolve(
		FinalBefore, Request, &FinalProvider);
	const auto FinalAdvance =
		FMatchPlayCurrentAttackCompletion::AdvanceAfterTerminal(
			FinalTerminal.AfterState,
			1,
			EInitialTurnOrderPlayer::PlayerA);
	TestTrue(TEXT("Final AP1 advance succeeds"), FinalAdvance.bSuccess);
	TestTrue(TEXT("Final AP1 ends match"), FinalAdvance.bMatchEnded);
	TestTrue(TEXT("Final match result derives successfully"),
		FinalAdvance.MatchResultResolveResult.bSuccess);
	TestEqual(TEXT("Final winner derives only from existing score"),
		FinalAdvance.MatchResultResolveResult.ResultType,
		EMatchResultType::Draw);
	return true;
}

#endif
