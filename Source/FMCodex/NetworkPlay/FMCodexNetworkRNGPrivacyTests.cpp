#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "FMCodexNetworkRNGTestEntropy.h"
#include "FMCodexNetworkMatchRuntime.h"
#include "FMCodexNetworkPlayerIntent.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "UObject/UnrealType.h"

namespace FMCodexNetworkRNGPrivacyTests
{
	using Entry = EMatchPlayAttackEntryRollPurpose;
	using Side = EInitialTurnOrderPlayer;
	using Provider = FFMCodexNetworkRandomProvider;
	using Entropy = FFMCodexNetworkScriptedEntropy;
	FMatchPlayPlayerIntent FullD12(const FFMCodexNetworkClientViewSnapshot& View)
	{
		FMatchPlayFullD12EntryRequest Request;
		Request.RequestingSide = View.ExpectedActingSide;
		Request.ExpectedAttackSequence = View.AttackSequence;
		return FMatchPlayPlayerIntent::Create(
			EMatchPlayAuthoritativeCommandKind::RequestInitialActionPointRoll, Request);
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexNetworkBoundedRandomTest,
	"FMCodex.NetworkPlay.RNGPrivacy.01.UnbiasedBoundaries",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexNetworkBoundedRandomTest::RunTest(const FString&)
{
	using namespace FMCodexNetworkRNGPrivacyTests;
	for (const int32 Count : {1, 2, 3, 6, 12, 97, MAX_int32})
	{
		const uint64 Domain = uint64(1) << 32;
		const uint64 Limit = Domain - Domain % static_cast<uint32>(Count);
		auto Source = MakeUnique<Entropy>(TArray<uint32>{0, static_cast<uint32>(Limit - 1)});
		auto* Calls = Source.Get();
		Provider P(MoveTemp(Source));
		TestEqual(TEXT("Inclusive minimum index"), P.SelectUniformIndex(Entry::SendingOffSelection, Count).SelectedIndex, 0);
		TestEqual(TEXT("Inclusive maximum index"), P.SelectUniformIndex(Entry::SendingOffSelection, Count).SelectedIndex, Count - 1);
		TestEqual(TEXT("Accepted boundaries use one draw each"), Calls->Calls, 2);
		if (Limit < Domain)
		{
			auto RejectSource = MakeUnique<Entropy>(TArray<uint32>{static_cast<uint32>(Limit), MAX_uint32, 0});
			auto* RejectCalls = RejectSource.Get();
			Provider Reject(MoveTemp(RejectSource));
			TestEqual(TEXT("Upper tail rejected before modulo"), Reject.SelectUniformIndex(Entry::SendingOffSelection, Count).SelectedIndex, 0);
			TestEqual(TEXT("Both tail words discarded"), RejectCalls->Calls, 3);
		}
	}
	// Complete consecutive source blocks give every residue exactly once.
	for (const int32 Count : {2, 3, 6, 12, 97})
	{
		TArray<uint32> Words;
		for (int32 N = 0; N < Count * 2; ++N) { Words.Add(N); }
		Provider P(MakeUnique<Entropy>(MoveTemp(Words)));
		TArray<int32> Hits; Hits.Init(0, Count);
		for (int32 N = 0; N < Count * 2; ++N)
		{
			const auto Result = P.SelectUniformIndex(Entry::SendingOffSelection, Count);
			if (!TestTrue(TEXT("Canonical index"), Result.bSuccess && Hits.IsValidIndex(Result.SelectedIndex))) { return false; }
			++Hits[Result.SelectedIndex];
		}
		for (const int32 Hit : Hits) { TestEqual(TEXT("Equal multiplicity, no statistical assertion"), Hit, 2); }
	}
	TArray<uint32> Tail; Tail.Init(MAX_uint32, 64);
	auto Source = MakeUnique<Entropy>(MoveTemp(Tail)); auto* Calls = Source.Get();
	Provider Exhausted(MoveTemp(Source));
	TestFalse(TEXT("Rejection cannot loop indefinitely"), Exhausted.RollD12(Entry::InitialActionPoint).bSuccess);
	TestEqual(TEXT("Bounded attempts"), Calls->Calls, 64);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexNetworkRandomPurposesTest,
	"FMCodex.NetworkPlay.RNGPrivacy.02.AllProviderPurposes",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexNetworkRandomPurposesTest::RunTest(const FString&)
{
	using namespace FMCodexNetworkRNGPrivacyTests;
	Provider D12(MakeUnique<Entropy>(TArray<uint32>{0, 11}));
	TestEqual(TEXT("D12 minimum"), D12.RollD12(Entry::InitialActionPoint).RawRoll, 1);
	TestEqual(TEXT("D12 maximum"), D12.RollD12(Entry::InitialActionPoint).RawRoll, 12);
	Provider Type(MakeUnique<Entropy>(TArray<uint32>{0, 5}));
	TestEqual(TEXT("Type minimum"), Type.RollD6(Entry::SetPieceType).RawRoll, 1);
	TestEqual(TEXT("Type maximum"), Type.RollD6(Entry::SetPieceType).RawRoll, 6);
	Provider Route(MakeUnique<Entropy>(TArray<uint32>{0, 5}));
	TestEqual(TEXT("Route minimum"), Route.RollD6(EMatchPlayCurrentAttackResolutionRollPurpose::InitialRoute).RawD6, 1);
	TestEqual(TEXT("Route maximum"), Route.RollD6(EMatchPlayCurrentAttackResolutionRollPurpose::InitialRoute).RawD6, 6);
	const UEnum* Purposes = StaticEnum<EMatchPlayCurrentAttackPostRouteRollPurpose>();
	for (int32 Index = 0; Index < Purposes->NumEnums() - 1; ++Index)
	{
		const auto Purpose = static_cast<EMatchPlayCurrentAttackPostRouteRollPurpose>(Purposes->GetValueByIndex(Index));
		if (Purpose == EMatchPlayCurrentAttackPostRouteRollPurpose::None) { continue; }
		auto Source = MakeUnique<Entropy>(TArray<uint32>{0, 5}); auto* Calls = Source.Get();
		Provider P(MoveTemp(Source));
		TestEqual(Purposes->GetNameStringByIndex(Index) + TEXT(" minimum"), P.RollD6(Purpose).RawD6, 1);
		TestEqual(Purposes->GetNameStringByIndex(Index) + TEXT(" maximum"), P.RollD6(Purpose).RawD6, 6);
		TestEqual(TEXT("All post-route purposes reach injected entropy"), Calls->Calls, 2);
	}
	auto Source = MakeUnique<Entropy>(); auto* Calls = Source.Get(); Provider Invalid(MoveTemp(Source));
	TestFalse(TEXT("Invalid D12 purpose"), Invalid.RollD12(Entry::SetPieceType).bSuccess);
	TestFalse(TEXT("Invalid type purpose"), Invalid.RollD6(Entry::InitialActionPoint).bSuccess);
	TestFalse(TEXT("No candidates"), Invalid.SelectUniformIndex(Entry::SendingOffSelection, 0).bSuccess);
	TestFalse(TEXT("Invalid route purpose"), Invalid.RollD6(EMatchPlayCurrentAttackResolutionRollPurpose::None).bSuccess);
	TestFalse(TEXT("Unknown post-route purpose"), Invalid.RollD6(static_cast<EMatchPlayCurrentAttackPostRouteRollPurpose>(255)).bSuccess);
	TestEqual(TEXT("Invalid requests never consume entropy"), Calls->Calls, 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexNetworkRecoveryRandomTest,
	"FMCodex.NetworkPlay.RNGPrivacy.03.WeightedRecovery",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexNetworkRecoveryRandomTest::RunTest(const FString&)
{
	using namespace FMCodexNetworkRNGPrivacyTests;
	TArray<FMatchPlayRecoveryCandidate> Candidates;
	for (const int32 Weight : {1, 3, 6}) { FMatchPlayRecoveryCandidate C; C.StaminaWeight = Weight; Candidates.Add(C); }
	TArray<int32> Hits; Hits.Init(0, 3);
	for (uint32 Ticket = 0; Ticket < 10; ++Ticket)
	{
		Provider P(MakeUnique<Entropy>(TArray<uint32>{Ticket, 0}));
		const auto R = P.DrawWeightedWithoutReplacement(EMatchPlayRecoveryPurpose::ConsumedRecovery, Candidates, 2);
		if (!TestTrue(TEXT("Two returned indices"), R.bSuccess && R.SelectedCandidateIndices.Num() == 2)) { return false; }
		++Hits[R.SelectedCandidateIndices[0]];
		TestNotEqual(TEXT("Without replacement"), R.SelectedCandidateIndices[0], R.SelectedCandidateIndices[1]);
	}
	TestEqual(TEXT("Weight one"), Hits[0], 1); TestEqual(TEXT("Weight three"), Hits[1], 3); TestEqual(TEXT("Weight six"), Hits[2], 6);
	Hits.Init(0, 3);
	for (uint32 Ticket = 0; Ticket < 4; ++Ticket)
	{
		Provider P(MakeUnique<Entropy>(TArray<uint32>{4, Ticket}));
		const auto R = P.DrawWeightedWithoutReplacement(EMatchPlayRecoveryPurpose::ConsumedRecovery, Candidates, 2);
		TestEqual(TEXT("Remove weight-six candidate first"), R.SelectedCandidateIndices[0], 2);
		++Hits[R.SelectedCandidateIndices[1]];
	}
	TestEqual(TEXT("Remaining conditional weight one"), Hits[0], 1);
	TestEqual(TEXT("Remaining conditional weight three"), Hits[1], 3);
	Provider Failed(MakeUnique<Entropy>(TArray<uint32>{0}));
	const auto R = Failed.DrawWeightedWithoutReplacement(EMatchPlayRecoveryPurpose::ConsumedRecovery, Candidates, 2);
	TestFalse(TEXT("Second entropy failure is explicit"), R.bSuccess);
	TestTrue(TEXT("No partial Recovery result"), R.SelectedCandidateIndices.IsEmpty());
	auto Source = MakeUnique<Entropy>(); auto* Calls = Source.Get(); Provider Invalid(MoveTemp(Source));
	Candidates[0].StaminaWeight = 0;
	TestFalse(TEXT("Invalid weights rejected"), Invalid.DrawWeightedWithoutReplacement(EMatchPlayRecoveryPurpose::ConsumedRecovery, Candidates, 2).bSuccess);
	TestEqual(TEXT("No entropy for invalid weights"), Calls->Calls, 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexNetworkPublicMetadataRandomTest,
	"FMCodex.NetworkPlay.RNGPrivacy.04.PublicMetadataIndependenceAndBranches",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexNetworkPublicMetadataRandomTest::RunTest(const FString&)
{
	using namespace FMCodexNetworkRNGPrivacyTests;
	const FGuid MatchA(1, 2, 3, 4), MatchB(5, 6, 7, 8);
	const auto Config = FFMCodexNetworkBootstrapConfigurationFactory::CreatePrototypeMatch();
	for (int32 D12 = 1; D12 <= 12; ++D12)
	{
		for (const FGuid& Match : {MatchA, MatchB})
		{
			FFMCodexNetworkMatchRuntime Runtime(Match,
				MakeUnique<Entropy>(TArray<uint32>{static_cast<uint32>(D12 - 1), 0}));
			TestTrue(TEXT("Deterministic initialization"), Runtime.InitializeOnce(Config).bSuccess);
			const auto Before = Runtime.BuildClientView(Side::PlayerA, 7, EFMCodexNetworkBootstrapState::MatchReady);
			TestEqual(TEXT("Initial D12 withheld"), Before.DisclosedInitialD12, 0);
			TestTrue(TEXT("Same real HostPort"), Runtime.SubmitPlayerIntent(FullD12(Before)).bSuccess);
			const auto After = Runtime.BuildClientView(Side::PlayerB, 8, EFMCodexNetworkBootstrapState::MatchReady);
			TestEqual(TEXT("Entropy alone chooses result for both public MatchIds"), After.DisclosedInitialD12, D12);
			const auto Branch = D12 == 1 ? EFMCodexNetworkEntryBranch::SendingOff
				: D12 <= 8 ? EFMCodexNetworkEntryBranch::Ordinary : EFMCodexNetworkEntryBranch::SetPiece;
			TestEqual(TEXT("All twelve canonical branch mappings"), After.EntryBranch, Branch);
			TestEqual(TEXT("One D12"), Runtime.GetD12ProviderInvocationCount(), 1);
			TestEqual(TEXT("Only AP1 consumes selection entropy"), Runtime.GetEntryProviderInvocationCount(), D12 == 1 ? 2 : 1);
		}
	}
	const auto BConfig = FFMCodexNetworkBootstrapConfigurationFactory::CreateBFirstAutomationMatch();
	FFMCodexNetworkMatchRuntime BFirst(MatchA, MakeUnique<Entropy>(TArray<uint32>{3}));
	TestTrue(TEXT("Server opening fixture initializes"), BFirst.InitializeOnce(BConfig).bSuccess);
	TestEqual(TEXT("Canonical opening resolver chooses B"),
		BFirst.BuildClientView(Side::PlayerB, 1, EFMCodexNetworkBootstrapState::MatchReady).ExpectedActingSide, Side::PlayerB);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexNetworkRandomFailureTest,
	"FMCodex.NetworkPlay.RNGPrivacy.05.EntropyFailureNoStateAdoption",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexNetworkRandomFailureTest::RunTest(const FString&)
{
	using namespace FMCodexNetworkRNGPrivacyTests;
	auto Source = MakeUnique<Entropy>(); auto* Calls = Source.Get();
	FFMCodexNetworkMatchRuntime Runtime(FGuid(1, 2, 3, 4), MoveTemp(Source));
	TestTrue(TEXT("Initialization needs no random draw"), Runtime.InitializeOnce(
		FFMCodexNetworkBootstrapConfigurationFactory::CreatePrototypeMatch()).bSuccess);
	const auto Before = Runtime.BuildClientView(Side::PlayerA, 1, EFMCodexNetworkBootstrapState::MatchReady);
	const auto Result = Runtime.SubmitPlayerIntent(FullD12(Before));
	TestFalse(TEXT("Entropy failure rejected"), Result.bSuccess);
	TestEqual(TEXT("Existing authority rejection contract"), Result.ErrorCode, EMatchPlayPlayerIntentPortErrorCode::AuthoritativeCommandRejected);
	const auto& E = Result.AuthoritativeResult.RuntimeEnvelope;
	TestFalse(TEXT("No successful domain result"), E.bDomainSuccess);
	TestTrue(TEXT("Entire authoritative State unchanged"), FMatchPlayState::StaticStruct()->CompareScriptStruct(&E.BeforeState, &E.AfterState, 0));
	TestEqual(TEXT("No automatic retry or fallback"), Calls->Calls, 1);
	Provider Failed(MakeUnique<Entropy>());
	TestFalse(TEXT("Type failure"), Failed.RollD6(Entry::SetPieceType).bSuccess);
	TestFalse(TEXT("Selection failure"), Failed.SelectUniformIndex(Entry::SendingOffSelection, 3).bSuccess);
	TestFalse(TEXT("Route failure"), Failed.RollD6(EMatchPlayCurrentAttackResolutionRollPurpose::InitialRoute).bSuccess);
	TestFalse(TEXT("Post-route failure"), Failed.RollD6(EMatchPlayCurrentAttackPostRouteRollPurpose::CornerAutomaticScorer).bSuccess);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexNetworkRandomSourceBoundaryTest,
	"FMCodex.NetworkPlay.RNGPrivacy.06.ProductionSourceAndWireBoundary",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexNetworkRandomSourceBoundaryTest::RunTest(const FString&)
{
	FString ProviderSource, RuntimeSource, ModeSource;
	auto Read = [&](const TCHAR* Name, FString& Out) { return TestTrue(TEXT("Source available"),
		FFileHelper::LoadFileToString(Out, *(FPaths::ProjectDir() / TEXT("Source/FMCodex/NetworkPlay") / Name))); };
	if (!Read(TEXT("FMCodexNetworkRandomProvider.cpp"), ProviderSource)
		|| !Read(TEXT("FMCodexNetworkMatchRuntime.cpp"), RuntimeSource)
		|| !Read(TEXT("FMCodexNetworkMatchGameMode.cpp"), ModeSource)) { return false; }
	TestTrue(TEXT("Uses engine cryptographic byte API"), ProviderSource.Contains(TEXT("Context->CreateRandomBytes(Bytes)")));
	for (const TCHAR* Forbidden : {TEXT("MatchInstanceId"), TEXT("RequestId"), TEXT("AttackSequence"), TEXT("ViewRevision"),
		TEXT("FRandomStream"), TEXT("FMath::Rand"), TEXT("FDateTime"), TEXT("FPlatformTime"), TEXT("UE_LOG")})
	{
		TestFalse(FString(TEXT("Private provider has no public seed or secret logging: ")) + Forbidden, ProviderSource.Contains(Forbidden));
	}
	TestFalse(TEXT("Old public seed helper retired"), ModeSource.Contains(TEXT("GenerateServerSeed")));
	TestFalse(TEXT("Runtime does not construct Local seeded provider"), RuntimeSource.Contains(TEXT("FFMCodexLocalMatchD6Provider")));
	TestTrue(TEXT("Production constructor uses no entropy/seed argument"), RuntimeSource.Contains(TEXT("MakeUnique<FFMCodexNetworkRandomProvider>()")));
	for (UScriptStruct* Struct : {FFMCodexNetworkClientViewSnapshot::StaticStruct(),
		FFMCodexNetworkPlayerIntentEnvelope::StaticStruct(), FFMCodexNetworkPlayerIntentAck::StaticStruct()})
	{
		for (TFieldIterator<FProperty> It(Struct); It; ++It)
		{
			const FString Name = It->GetName();
			TestFalse(TEXT("No secret RNG state on wire"), Name.Contains(TEXT("Seed")) || Name.Contains(TEXT("Entropy"))
				|| Name.Contains(TEXT("Provider")) || Name.Contains(TEXT("RandomStream")));
		}
	}
	return true;
}
#endif
