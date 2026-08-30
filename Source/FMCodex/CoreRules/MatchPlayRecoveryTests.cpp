#include "MatchPlayRecovery.h"
#include "MatchPlayState.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

namespace MatchPlayRecoveryTests
{
	const FName A1(TEXT("Recovery.A1"));
	const FName A2(TEXT("Recovery.A2"));
	const FName A3(TEXT("Recovery.A3"));
	const FName AGK(TEXT("Recovery.AGK"));
	const FName B1(TEXT("Recovery.B1"));
	const FName B2(TEXT("Recovery.B2"));

	FPlayerCardRuleSnapshot Snapshot(
		const FName CardId,
		const int32 Stamina,
		const bool bGoalkeeper = false)
	{
		FPlayerCardRuleSnapshot Result;
		Result.CardId = CardId;
		Result.PositionTypes = { bGoalkeeper
			? EPlayerPositionType::Goalkeeper
			: EPlayerPositionType::Attack };
		Result.bIsGoalkeeper = bGoalkeeper;
		Result.bHasGoalkeeperAttributes = bGoalkeeper;
		Result.Attributes.Stamina = Stamina;
		return Result;
	}

	FMatchPlayPerSideCardSnapshotAuthority Authority()
	{
		FMatchPlayPerSideCardSnapshotAuthority Result;
		Result.PlayerACardSnapshots.Cards = {
			Snapshot(A1, 1), Snapshot(A2, 2), Snapshot(A3, 3),
			Snapshot(AGK, 1, true) };
		Result.PlayerBCardSnapshots.Cards = {
			Snapshot(B1, 4), Snapshot(B2, 6) };
		return Result;
	}

	class FProvider final : public IMatchPlayRecoveryProvider
	{
	public:
		virtual FMatchPlayRecoveryProviderResult
		DrawWeightedWithoutReplacement(
			const EMatchPlayRecoveryPurpose Purpose,
			const TArray<FMatchPlayRecoveryCandidate>& OrderedCandidates,
			const int32 ReturnCount) override
		{
			++CallCount;
			LastPurpose = Purpose;
			LastCandidates = OrderedCandidates;
			LastReturnCount = ReturnCount;
			return NextResult;
		}

		int32 CallCount = 0;
		int32 LastReturnCount = 0;
		EMatchPlayRecoveryPurpose LastPurpose = EMatchPlayRecoveryPurpose::None;
		TArray<FMatchPlayRecoveryCandidate> LastCandidates;
		FMatchPlayRecoveryProviderResult NextResult;
	};

	FMatchPlayRecoveryProviderResult Success(
		const int32 First,
		const int32 Second)
	{
		FMatchPlayRecoveryProviderResult Result;
		Result.bSuccess = true;
		Result.SelectedCandidateIndices = { First, Second };
		return Result;
	}

	bool SameUsage(
		const FMatchCardUsageState& Left,
		const FMatchCardUsageState& Right)
	{
		return FMatchCardUsageState::StaticStruct()->CompareScriptStruct(
			&Left, &Right, 0);
	}
}

#define RECOVERY_TEST(TestClass, TestName) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
		TestClass, \
		"FMCodex.CoreRules.MatchPlayRecovery." TestName, \
		EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

RECOVERY_TEST(FMatchPlayRecoveryCandidateQueryTest,
	"CandidateQueryStableCombinedOrderAndWeight")

bool FMatchPlayRecoveryCandidateQueryTest::RunTest(const FString& Parameters)
{
	using namespace MatchPlayRecoveryTests;
	FMatchCardUsageState Usage;
	Usage.PlayerACardUsageState.AvailableCardIds = { A3, AGK };
	Usage.PlayerACardUsageState.UsedCardIds = { A2, A1 };
	Usage.PlayerBCardUsageState.AvailableCardIds = { B2 };
	Usage.PlayerBCardUsageState.UsedCardIds = { B1 };
	const FMatchCardUsageState Before = Usage;
	const auto Query = FMatchPlayRecoveryCandidateQuery::Build(
		Usage, Authority());
	TestTrue(TEXT("Candidate query succeeds"), Query.bSuccess);
	TestEqual(TEXT("Combined pool count"), Query.Candidates.Num(), 3);
	if (Query.Candidates.Num() == 3)
	{
		TestEqual(TEXT("PlayerA Used order index 0"),
			Query.Candidates[0].CardId, A2);
		TestEqual(TEXT("PlayerA Used order index 1"),
			Query.Candidates[1].CardId, A1);
		TestEqual(TEXT("PlayerB follows PlayerA"),
			Query.Candidates[2].CardId, B1);
		TestEqual(TEXT("Stamina is authoritative"),
			Query.Candidates[2].StaminaWeight, 4);
	}
	TestTrue(TEXT("Query is pure"), SameUsage(Usage, Before));
	return true;
}

RECOVERY_TEST(FMatchPlayRecoveryZeroOneTest,
	"PoolZeroAndOneAreDeterministicWithoutProvider")

bool FMatchPlayRecoveryZeroOneTest::RunTest(const FString& Parameters)
{
	using namespace MatchPlayRecoveryTests;
	FProvider Provider;
	FMatchCardUsageState Empty;
	Empty.PlayerACardUsageState.AvailableCardIds = { A1, A2, A3, AGK };
	Empty.PlayerBCardUsageState.AvailableCardIds = { B1, B2 };
	const auto Zero = FMatchPlayRecoveryResolver::Resolve(
		Empty, Authority(), 7, &Provider);
	TestTrue(TEXT("Pool0 succeeds"), Zero.bSuccess);
	TestEqual(TEXT("Pool0 provider calls"), Provider.CallCount, 0);
	TestTrue(TEXT("Pool0 explicit fact"), Zero.RecoveryFact.bHasRecoveryFact);
	TestEqual(TEXT("Pool0 fact sequence"),
		Zero.RecoveryFact.SourceAttackSequence, int64(7));
	TestTrue(TEXT("Pool0 returned list empty"),
		Zero.RecoveryFact.ReturnedCards.IsEmpty());

	FMatchCardUsageState One = Empty;
	One.PlayerACardUsageState.AvailableCardIds.Remove(A2);
	One.PlayerACardUsageState.UsedCardIds.Add(A2);
	const auto Single = FMatchPlayRecoveryResolver::Resolve(
		One, Authority(), 8, &Provider);
	TestTrue(TEXT("Pool1 succeeds"), Single.bSuccess);
	TestEqual(TEXT("Pool1 provider calls"), Provider.CallCount, 0);
	TestEqual(TEXT("Pool1 returns one"),
		Single.RecoveryFact.ReturnedCards.Num(), 1);
	TestTrue(TEXT("Pool1 moves Used to Available"),
		Single.UpdatedCardUsageState.PlayerACardUsageState
			.AvailableCardIds.Contains(A2));
	TestFalse(TEXT("Pool1 removes Used"),
		Single.UpdatedCardUsageState.PlayerACardUsageState
			.UsedCardIds.Contains(A2));
	return true;
}

RECOVERY_TEST(FMatchPlayRecoveryPairTest,
	"PoolTwoUsesOneAtomicProviderAndPreservesDrawOrder")

bool FMatchPlayRecoveryPairTest::RunTest(const FString& Parameters)
{
	using namespace MatchPlayRecoveryTests;
	FMatchCardUsageState Usage;
	Usage.PlayerACardUsageState.AvailableCardIds = { A1, A3, AGK };
	Usage.PlayerACardUsageState.UsedCardIds = { A2 };
	Usage.PlayerBCardUsageState.AvailableCardIds = { B2 };
	Usage.PlayerBCardUsageState.UsedCardIds = { B1 };
	FProvider Provider;
	Provider.NextResult = Success(1, 0);
	const auto Result = FMatchPlayRecoveryResolver::Resolve(
		Usage, Authority(), 9, &Provider);
	TestTrue(TEXT("Pool2 succeeds"), Result.bSuccess);
	TestEqual(TEXT("Pool2 provider called once"), Provider.CallCount, 1);
	TestEqual(TEXT("Semantic purpose"), Provider.LastPurpose,
		EMatchPlayRecoveryPurpose::ConsumedRecovery);
	TestEqual(TEXT("ReturnCount is two"), Provider.LastReturnCount, 2);
	TestEqual(TEXT("Exactly two facts"),
		Result.RecoveryFact.ReturnedCards.Num(), 2);
	if (Result.RecoveryFact.ReturnedCards.Num() == 2)
	{
		TestEqual(TEXT("Provider order entry 0"),
			Result.RecoveryFact.ReturnedCards[0].CardId, B1);
		TestEqual(TEXT("Provider order entry 1"),
			Result.RecoveryFact.ReturnedCards[1].CardId, A2);
	}
	TestTrue(TEXT("PlayerA Available returns to roster order"),
		Result.UpdatedCardUsageState.PlayerACardUsageState.AvailableCardIds
			== TArray<FName>({ A1, A2, A3, AGK }));

	FMatchCardUsageState WeightedUsage;
	WeightedUsage.PlayerACardUsageState.AvailableCardIds = { A2, AGK };
	WeightedUsage.PlayerACardUsageState.UsedCardIds = { A1, A3 };
	WeightedUsage.PlayerBCardUsageState.AvailableCardIds = { B1 };
	WeightedUsage.PlayerBCardUsageState.UsedCardIds = { B2 };
	FProvider CrossSideProvider;
	CrossSideProvider.NextResult = Success(0, 2);
	const auto CrossSide = FMatchPlayRecoveryResolver::Resolve(
		WeightedUsage, Authority(), 10, &CrossSideProvider);
	TestTrue(TEXT("Pool3 cross-side pair succeeds"), CrossSide.bSuccess);
	TestEqual(TEXT("Pool3 uses one atomic provider operation"),
		CrossSideProvider.CallCount, 1);
	TestEqual(TEXT("Pool3 provider receives three candidates"),
		CrossSideProvider.LastCandidates.Num(), 3);
	if (CrossSideProvider.LastCandidates.Num() == 3)
	{
		TestEqual(TEXT("Pool3 weight 0 is STA1"),
			CrossSideProvider.LastCandidates[0].StaminaWeight, 1);
		TestEqual(TEXT("Pool3 weight 1 is STA3"),
			CrossSideProvider.LastCandidates[1].StaminaWeight, 3);
		TestEqual(TEXT("Pool3 weight 2 is STA6"),
			CrossSideProvider.LastCandidates[2].StaminaWeight, 6);
	}
	TestTrue(TEXT("Cross-side PlayerA card returns to PlayerA"),
		CrossSide.UpdatedCardUsageState.PlayerACardUsageState
			.AvailableCardIds.Contains(A1));
	TestTrue(TEXT("Cross-side PlayerB card returns to PlayerB"),
		CrossSide.UpdatedCardUsageState.PlayerBCardUsageState
			.AvailableCardIds.Contains(B2));

	FProvider SameSideProvider;
	SameSideProvider.NextResult = Success(0, 1);
	const auto SameSide = FMatchPlayRecoveryResolver::Resolve(
		WeightedUsage, Authority(), 11, &SameSideProvider);
	TestTrue(TEXT("Pool3 same-side pair succeeds"), SameSide.bSuccess);
	TestTrue(TEXT("Both PlayerA candidates may return"),
		SameSide.UpdatedCardUsageState.PlayerACardUsageState
			.AvailableCardIds.Contains(A1)
		&& SameSide.UpdatedCardUsageState.PlayerACardUsageState
			.AvailableCardIds.Contains(A3));
	TestTrue(TEXT("No per-side quota forces PlayerB return"),
		SameSide.UpdatedCardUsageState.PlayerBCardUsageState
			.UsedCardIds.Contains(B2));
	return true;
}

RECOVERY_TEST(FMatchPlayRecoveryFailureAtomicityTest,
	"ProviderFailureAndMalformedPairAreAtomic")

bool FMatchPlayRecoveryFailureAtomicityTest::RunTest(const FString& Parameters)
{
	using namespace MatchPlayRecoveryTests;
	FMatchCardUsageState Usage;
	Usage.PlayerACardUsageState.AvailableCardIds = { A3, AGK };
	Usage.PlayerACardUsageState.UsedCardIds = { A1, A2 };
	Usage.PlayerBCardUsageState.AvailableCardIds = { B2 };
	Usage.PlayerBCardUsageState.UsedCardIds = { B1 };

	FProvider Failure;
	Failure.NextResult.ErrorCode =
		EMatchPlayRecoveryProviderErrorCode::ProviderFailure;
	Failure.NextResult.ErrorMessage = TEXT("retryable");
	const auto Failed = FMatchPlayRecoveryResolver::Resolve(
		Usage, Authority(), 10, &Failure);
	TestEqual(TEXT("Provider failure is explicit"), Failed.ErrorCode,
		EMatchPlayRecoveryResolveErrorCode::ProviderFailure);
	TestTrue(TEXT("Provider failure preserves usage"),
		SameUsage(Failed.UpdatedCardUsageState, Usage));
	TestFalse(TEXT("Failed Recovery publishes no fact"),
		Failed.RecoveryFact.bHasRecoveryFact);

	FProvider Malformed;
	Malformed.NextResult = Success(0, 0);
	const auto Rejected = FMatchPlayRecoveryResolver::Resolve(
		Usage, Authority(), 10, &Malformed);
	TestEqual(TEXT("Duplicate pair is malformed"), Rejected.ErrorCode,
		EMatchPlayRecoveryResolveErrorCode::MalformedProviderResult);
	TestTrue(TEXT("Malformed pair preserves usage"),
		SameUsage(Rejected.UpdatedCardUsageState, Usage));
	return true;
}

RECOVERY_TEST(FMatchPlayRecoveryInvalidCandidateTest,
	"GoalkeeperAndInvalidStaminaCannotEnterPool")

bool FMatchPlayRecoveryInvalidCandidateTest::RunTest(const FString& Parameters)
{
	using namespace MatchPlayRecoveryTests;
	FMatchCardUsageState GoalkeeperUsage;
	GoalkeeperUsage.PlayerACardUsageState.AvailableCardIds = { A1, A2, A3 };
	GoalkeeperUsage.PlayerACardUsageState.UsedCardIds = { AGK };
	GoalkeeperUsage.PlayerBCardUsageState.AvailableCardIds = { B1, B2 };
	const auto Goalkeeper = FMatchPlayRecoveryCandidateQuery::Build(
		GoalkeeperUsage, Authority());
	TestEqual(TEXT("Goalkeeper Used is invalid"), Goalkeeper.ErrorCode,
		EMatchPlayRecoveryCandidateQueryErrorCode::GoalkeeperInUsedZone);

	auto InvalidAuthority = Authority();
	InvalidAuthority.PlayerACardSnapshots.Cards[0].Attributes.Stamina = 7;
	FMatchCardUsageState InvalidWeight;
	InvalidWeight.PlayerACardUsageState.UsedCardIds = { A1 };
	InvalidWeight.PlayerBCardUsageState.AvailableCardIds = { B1, B2 };
	const auto Weight = FMatchPlayRecoveryCandidateQuery::Build(
		InvalidWeight, InvalidAuthority);
	TestFalse(TEXT("Invalid snapshot/weight query fails"), Weight.bSuccess);

	FMatchCardUsageState FilteredUsage;
	FilteredUsage.PlayerACardUsageState.AvailableCardIds = { A1, AGK };
	FilteredUsage.PlayerACardUsageState.UsedCardIds = { A3 };
	FilteredUsage.PlayerACardUsageState.EjectedCardIds = { A2 };
	FilteredUsage.PlayerBCardUsageState.AvailableCardIds = { B2 };
	FilteredUsage.PlayerBCardUsageState.UsedCardIds = { B1 };
	const auto Filtered = FMatchPlayRecoveryCandidateQuery::Build(
		FilteredUsage, Authority());
	TestTrue(TEXT("Valid Used-only query succeeds"), Filtered.bSuccess);
	TestEqual(TEXT("Available, GK, and Ejected are excluded"),
		Filtered.Candidates.Num(), 2);
	if (Filtered.Candidates.Num() == 2)
	{
		TestEqual(TEXT("Eligible PlayerA Used remains"),
			Filtered.Candidates[0].CardId, A3);
		TestEqual(TEXT("Eligible PlayerB Used remains"),
			Filtered.Candidates[1].CardId, B1);
	}

	FMatchCardUsageState UnknownUsage = FilteredUsage;
	UnknownUsage.PlayerACardUsageState.UsedCardIds.Add(
		FName(TEXT("Recovery.Unknown")));
	const auto Unknown = FMatchPlayRecoveryCandidateQuery::Build(
		UnknownUsage, Authority());
	TestFalse(TEXT("Missing authoritative snapshot rejects query"),
		Unknown.bSuccess);
	return true;
}

RECOVERY_TEST(FMatchPlayLastRecoveryFactLifecycleTest,
	"LastRecoveryFactIsBoundedCopyableAndNonLegal")

bool FMatchPlayLastRecoveryFactLifecycleTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayRecoveryTests;
	FMatchPlayState State;
	TestFalse(TEXT("Default state has no Recovery fact"),
		State.LastRecoveryFact.bHasRecoveryFact);

	FProvider FirstProvider;
	FirstProvider.NextResult = Success(0, 1);
	FMatchCardUsageState FirstUsage;
	FirstUsage.PlayerACardUsageState.AvailableCardIds = { A3, AGK };
	FirstUsage.PlayerACardUsageState.UsedCardIds = { A1, A2 };
	FirstUsage.PlayerBCardUsageState.AvailableCardIds = { B1, B2 };
	const auto First = FMatchPlayRecoveryResolver::Resolve(
		FirstUsage, Authority(), 21, &FirstProvider);
	TestTrue(TEXT("First bounded Recovery succeeds"), First.bSuccess);
	State.CardUsageState = First.UpdatedCardUsageState;
	State.CardSnapshotAuthority = Authority();
	State.LastRecoveryFact = First.RecoveryFact;
	const FMatchPlayState Reconstructed = State;
	TestTrue(TEXT("Recovery fact survives snapshot copy"),
		FMatchPlayLastRecoveryFact::StaticStruct()->CompareScriptStruct(
			&State.LastRecoveryFact,
			&Reconstructed.LastRecoveryFact,
			0));
	TestEqual(TEXT("Copied fact keeps source sequence"),
		Reconstructed.LastRecoveryFact.SourceAttackSequence, int64(21));
	TestEqual(TEXT("Copied fact keeps ordered identities"),
		Reconstructed.LastRecoveryFact.ReturnedCards.Num(), 2);

	FMatchCardUsageState Empty;
	Empty.PlayerACardUsageState.AvailableCardIds = { A1, A2, A3, AGK };
	Empty.PlayerBCardUsageState.AvailableCardIds = { B1, B2 };
	const auto Replacement = FMatchPlayRecoveryResolver::Resolve(
		Empty, Authority(), 22, nullptr);
	TestTrue(TEXT("Replacement zero-return event succeeds"),
		Replacement.bSuccess);
	State.LastRecoveryFact = Replacement.RecoveryFact;
	TestEqual(TEXT("Latest event replaces source sequence"),
		State.LastRecoveryFact.SourceAttackSequence, int64(22));
	TestTrue(TEXT("Latest zero-return event replaces old identities"),
		State.LastRecoveryFact.ReturnedCards.IsEmpty());
	TestTrue(TEXT("Recovery fact replacement leaves CardUsage untouched"),
		SameUsage(State.CardUsageState,
			First.UpdatedCardUsageState));
	return true;
}

#undef RECOVERY_TEST

#endif
