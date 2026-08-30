#include "FMCodexLocalMatchD6Provider.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "FMCodexLocalMatchHostGameMode.h"

#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

#include <type_traits>

namespace FMCodexLocalMatchD6ProviderTests
{
	bool IsD6(const int32 Value)
	{
		return Value >= 1 && Value <= 6;
	}

	bool LoadProductionSource(
		const TCHAR* RelativePath,
		FString& OutSource)
	{
		return FFileHelper::LoadFileToString(
			OutSource,
			*FPaths::ConvertRelativePathToFull(
				FPaths::ProjectDir() / RelativePath));
	}

	int32 CountOccurrences(
		const FString& Text,
		const FString& Needle)
	{
		int32 Count = 0;
		int32 SearchFrom = 0;
		while ((SearchFrom = Text.Find(
			Needle,
			ESearchCase::CaseSensitive,
			ESearchDir::FromStart,
			SearchFrom)) != INDEX_NONE)
		{
			++Count;
			SearchFrom += Needle.Len();
		}
		return Count;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexLocalMatchD6ProviderContractTest,
	"FMCodex.LocalPlay.LocalD6Provider.01.ContractsAndDeterminism",
	EAutomationTestFlags::EditorContext
		| EAutomationTestFlags::EngineFilter)

bool FFMCodexLocalMatchD6ProviderContractTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexLocalMatchD6ProviderTests;
	TestTrue(TEXT("Provider implements Initial Route interface"),
		(std::is_base_of_v<
			IMatchPlayInitialRouteRollProvider,
			FFMCodexLocalMatchD6Provider>));
	TestTrue(TEXT("Provider implements post-route interface"),
		(std::is_base_of_v<
			IMatchPlayPostRouteRollProvider,
			FFMCodexLocalMatchD6Provider>));
	TestTrue(TEXT("Provider implements Recovery interface"),
		(std::is_base_of_v<
			IMatchPlayRecoveryProvider,
			FFMCodexLocalMatchD6Provider>));
	TestFalse(TEXT("Provider requires an explicit match seed"),
		std::is_default_constructible_v<FFMCodexLocalMatchD6Provider>);
	TestFalse(TEXT("Provider cannot be copied"),
		std::is_copy_constructible_v<FFMCodexLocalMatchD6Provider>);
	TestFalse(TEXT("Provider cannot be moved"),
		std::is_move_constructible_v<FFMCodexLocalMatchD6Provider>);

	constexpr int32 Seed = 0x5132A6;
	FFMCodexLocalMatchD6Provider ProviderA(Seed);
	FFMCodexLocalMatchD6Provider ProviderB(Seed);
	FFMCodexLocalMatchD6Provider TacticalProviderA(Seed);
	FFMCodexLocalMatchD6Provider TacticalProviderB(Seed);
	for (int32 Index = 0; Index < 128; ++Index)
	{
		const int32 TacticalPointA =
			TacticalProviderA.RollOrdinaryTacticalPoint();
		const int32 TacticalPointB =
			TacticalProviderB.RollOrdinaryTacticalPoint();
		TestTrue(TEXT("Ordinary Tactical Point stays in supported 2..8 slice"),
			TacticalPointA >= 2 && TacticalPointA <= 8);
		TestEqual(TEXT("Same seed Tactical Point sequence is deterministic"),
			TacticalPointA, TacticalPointB);
	}
	for (int32 Index = 0; Index < 128; ++Index)
	{
		if (Index % 3 == 0)
		{
			const auto ResultA = ProviderA.RollD6(
				EMatchPlayCurrentAttackResolutionRollPurpose::InitialRoute);
			const auto ResultB = ProviderB.RollD6(
				EMatchPlayCurrentAttackResolutionRollPurpose::InitialRoute);
			TestTrue(TEXT("Initial Route result is canonical success"),
				ResultA.bSuccess
					&& ResultA.ErrorCode
						== EMatchPlayInitialRouteRollProviderErrorCode::None
					&& ResultA.ErrorMessage.IsEmpty()
					&& IsD6(ResultA.RawD6));
			TestEqual(TEXT("Same seed Initial Route sequence is deterministic"),
				ResultA.RawD6, ResultB.RawD6);
		}
		else
		{
			const EMatchPlayCurrentAttackPostRouteRollPurpose Purpose =
				Index % 3 == 1
					? EMatchPlayCurrentAttackPostRouteRollPurpose
						::PrimaryAttack
					: EMatchPlayCurrentAttackPostRouteRollPurpose
						::OneOnOneDirectShotDefense;
			const auto ResultA = ProviderA.RollD6(Purpose);
			const auto ResultB = ProviderB.RollD6(Purpose);
			const auto Validation =
				FMatchPlayPostRouteRollProviderResultValidator::Validate(
					Purpose,
					ResultA);
			TestTrue(TEXT("Post-route result is canonical success"),
				ResultA.bSuccess
					&& ResultA.ErrorCode
						== EMatchPlayPostRouteRollProviderErrorCode::None
					&& ResultA.ErrorMessage.IsEmpty()
					&& IsD6(ResultA.RawD6)
					&& Validation.bIsCanonical);
			TestEqual(TEXT("Same seed post-route sequence is deterministic"),
				ResultA.RawD6, ResultB.RawD6);
		}
	}

	FFMCodexLocalMatchD6Provider InvalidInitialProvider(Seed);
	FFMCodexLocalMatchD6Provider FreshInitialProvider(Seed);
	const auto InvalidInitial = InvalidInitialProvider.RollD6(
		EMatchPlayCurrentAttackResolutionRollPurpose::None);
	TestFalse(TEXT("Invalid Initial Route purpose fails"),
		InvalidInitial.bSuccess);
	TestEqual(TEXT("Invalid Initial Route exact error"),
		InvalidInitial.ErrorCode,
		EMatchPlayInitialRouteRollProviderErrorCode::InvalidPurpose);
	TestEqual(TEXT("Invalid Initial Route does not consume stream"),
		InvalidInitialProvider.RollD6(
			EMatchPlayCurrentAttackResolutionRollPurpose::InitialRoute).RawD6,
		FreshInitialProvider.RollD6(
			EMatchPlayCurrentAttackResolutionRollPurpose::InitialRoute).RawD6);

	FFMCodexLocalMatchD6Provider InvalidPostProvider(Seed);
	FFMCodexLocalMatchD6Provider FreshPostProvider(Seed);
	const auto InvalidPost = InvalidPostProvider.RollD6(
		EMatchPlayCurrentAttackPostRouteRollPurpose::None);
	TestFalse(TEXT("Invalid post-route purpose fails"),
		InvalidPost.bSuccess);
	TestEqual(TEXT("Invalid post-route exact error"),
		InvalidPost.ErrorCode,
		EMatchPlayPostRouteRollProviderErrorCode::InvalidPurpose);
	TestEqual(TEXT("Invalid post-route does not consume stream"),
		InvalidPostProvider.RollD6(
			EMatchPlayCurrentAttackPostRouteRollPurpose::PrimaryAttack).RawD6,
		FreshPostProvider.RollD6(
			EMatchPlayCurrentAttackPostRouteRollPurpose::PrimaryAttack).RawD6);

	TArray<FMatchPlayRecoveryCandidate> RecoveryCandidates;
	for (int32 Index = 0; Index < 4; ++Index)
	{
		FMatchPlayRecoveryCandidate Candidate;
		Candidate.OwnerSide = Index < 2
			? EInitialTurnOrderPlayer::PlayerA
			: EInitialTurnOrderPlayer::PlayerB;
		Candidate.CardId = FName(*FString::Printf(TEXT("Recovery.%d"), Index));
		Candidate.StaminaWeight = Index + 1;
		RecoveryCandidates.Add(Candidate);
	}
	FFMCodexLocalMatchD6Provider RecoveryA(Seed);
	FFMCodexLocalMatchD6Provider RecoveryB(Seed);
	const auto PairA = RecoveryA.DrawWeightedWithoutReplacement(
		EMatchPlayRecoveryPurpose::ConsumedRecovery,
		RecoveryCandidates,
		2);
	const auto PairB = RecoveryB.DrawWeightedWithoutReplacement(
		EMatchPlayRecoveryPurpose::ConsumedRecovery,
		RecoveryCandidates,
		2);
	TestTrue(TEXT("Recovery pair succeeds"), PairA.bSuccess);
	TestEqual(TEXT("Recovery pair returns exactly two"),
		PairA.SelectedCandidateIndices.Num(), 2);
	TestTrue(TEXT("Recovery pair is without replacement"),
		PairA.SelectedCandidateIndices.Num() == 2
			&& PairA.SelectedCandidateIndices[0]
				!= PairA.SelectedCandidateIndices[1]);
	TestTrue(TEXT("Same seed Recovery pair is deterministic"),
		PairA.SelectedCandidateIndices == PairB.SelectedCandidateIndices);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexLocalMatchD6ProviderChronologyAndAuthorityTest,
	"FMCodex.LocalPlay.LocalD6Provider.02.ChronologyIsolationAndAuthority",
	EAutomationTestFlags::EditorContext
		| EAutomationTestFlags::EngineFilter)

bool FFMCodexLocalMatchD6ProviderChronologyAndAuthorityTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexLocalMatchD6ProviderTests;
	constexpr int32 Seed = 0x19A52;
	FFMCodexLocalMatchD6Provider ProviderA(Seed);
	FFMCodexLocalMatchD6Provider ProviderB(Seed);

	const int32 AInitial = ProviderA.RollD6(
		EMatchPlayCurrentAttackResolutionRollPurpose::InitialRoute).RawD6;
	const int32 BInitial = ProviderB.RollD6(
		EMatchPlayCurrentAttackResolutionRollPurpose::InitialRoute).RawD6;
	TestEqual(TEXT("Independent providers start at same deterministic position"),
		AInitial, BInitial);
	const int32 AAttack = ProviderA.RollD6(
		EMatchPlayCurrentAttackPostRouteRollPurpose::PrimaryAttack).RawD6;
	const int32 ADefense = ProviderA.RollD6(
		EMatchPlayCurrentAttackPostRouteRollPurpose::PrimaryDefense).RawD6;
	const int32 BAttack = ProviderB.RollD6(
		EMatchPlayCurrentAttackPostRouteRollPurpose::PrimaryAttack).RawD6;
	const int32 BDefense = ProviderB.RollD6(
		EMatchPlayCurrentAttackPostRouteRollPurpose::PrimaryDefense).RawD6;
	TestEqual(TEXT("Advancing A does not advance B attack position"),
		AAttack, BAttack);
	TestEqual(TEXT("Advancing A does not advance B defense position"),
		ADefense, BDefense);
	const int32 AOneOnOne = ProviderA.RollD6(
		EMatchPlayCurrentAttackPostRouteRollPurpose
			::OneOnOneDirectShotDefense).RawD6;
	const int32 BOneOnOne = ProviderB.RollD6(
		EMatchPlayCurrentAttackPostRouteRollPurpose
			::OneOnOneDirectShotDefense).RawD6;
	TestEqual(TEXT("Initial and post-route calls share one coherent stream"),
		AOneOnOne, BOneOnOne);

	FString ProviderHeader;
	FString ProviderSource;
	FString HostHeader;
	FString HostSource;
	TestTrue(TEXT("Provider header loads"), LoadProductionSource(
		TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchD6Provider.h"),
		ProviderHeader));
	TestTrue(TEXT("Provider source loads"), LoadProductionSource(
		TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchD6Provider.cpp"),
		ProviderSource));
	TestTrue(TEXT("Host header loads"), LoadProductionSource(
		TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchHostGameMode.h"),
		HostHeader));
	TestTrue(TEXT("Host source loads"), LoadProductionSource(
		TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchHostGameMode.cpp"),
		HostSource));
	TestEqual(TEXT("All successful rolls use one local RandRange callsite"),
		CountOccurrences(ProviderSource,
			TEXT("RandomStream.RandRange(1, 6)")),
		1);
	TestEqual(TEXT("Ordinary Tactical Point uses one supported-range callsite"),
		CountOccurrences(ProviderSource,
			TEXT("RandomStream.RandRange(2, 8)")),
		1);
	TestFalse(TEXT("No global gameplay RNG is used"),
		ProviderSource.Contains(TEXT("FMath::Rand"))
			|| ProviderSource.Contains(TEXT("FMath::RandRange"))
			|| ProviderSource.Contains(TEXT("srand")));
	TestFalse(TEXT("Provider stores no duplicate accepted-roll history"),
		ProviderHeader.Contains(TEXT("AllRolls"))
			|| ProviderHeader.Contains(TEXT("AcceptedRolls"))
			|| ProviderHeader.Contains(TEXT("GameplayRollHistory")));
	TestFalse(TEXT("Host exposes no provider or stream getter"),
		HostHeader.Contains(TEXT("GetD6Provider"))
			|| HostHeader.Contains(TEXT("GetRandomStream"))
			|| HostHeader.Contains(TEXT("GetSession")));
	TestFalse(TEXT("Host exposes no caller-supplied roll command"),
		HostHeader.Contains(TEXT("SetNextD6"))
			|| HostHeader.Contains(TEXT("InjectRoll"))
			|| HostHeader.Contains(TEXT("ResolveInitialRoute(int32")));
	TestEqual(TEXT("Runtime bundle owns one provider"),
		CountOccurrences(HostHeader,
			TEXT("FFMCodexLocalMatchD6Provider D6Provider;")),
		1);
	TestEqual(TEXT("Runtime bundle owns one Session"),
		CountOccurrences(HostHeader,
			TEXT("FMatchPlayAuthoritativeSession AuthoritativeSession;")),
		1);
	TestTrue(TEXT("Provider is declared before borrowing Session"),
		HostHeader.Find(TEXT("FFMCodexLocalMatchD6Provider D6Provider;"))
			< HostHeader.Find(
				TEXT("FMatchPlayAuthoritativeSession AuthoritativeSession;")));
	TestEqual(TEXT("Session borrows the same provider for all three interfaces"),
		CountOccurrences(HostSource,
			TEXT("\t\tD6Provider,")),
		3);
	TestEqual(TEXT("Candidate runtime is atomically adopted once"),
		CountOccurrences(HostSource,
			TEXT("ActiveMatchRuntime = MoveTemp(CandidateRuntime);")),
		1);
	TestEqual(TEXT("Every start creates one fresh runtime bundle"),
		CountOccurrences(HostSource,
			TEXT("MakeUnique<FLocalMatchRuntime>(")),
		1);
	return true;
}

#endif
