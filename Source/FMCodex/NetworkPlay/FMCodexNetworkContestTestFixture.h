#pragma once
#if WITH_DEV_AUTOMATION_TESTS
#include "FMCodexNetworkInitialRouteTestFixture.h"
namespace FMCodexNetworkInitialRouteTests
{
	struct FFrozen
	{
		FMatchPlayState State;
		int32 Revision, Entropy, Post, Initial, Entry, D12, Coordinator;
		explicit FFrozen(FFixture& F)
			: State(Access::Session(*F.Mode).GetStateSnapshot()), Revision(Access::Revision(*F.Mode)),
			Entropy(F.Entropy->Calls), Post(Access::Runtime(*F.Mode).GetPostRouteProviderInvocationCount()),
			Initial(Access::Runtime(*F.Mode).GetInitialRouteProviderInvocationCount()),
			Entry(Access::Runtime(*F.Mode).GetEntryProviderInvocationCount()), D12(Access::Runtime(*F.Mode).GetD12ProviderInvocationCount()), Coordinator(F.Calls()) {}
		void Verify(FAutomationTestBase& T, FFixture& F, bool Failure = false) const
		{
			T.TestTrue(TEXT("Entire authoritative State unchanged"), SameState(State, Access::Session(*F.Mode).GetStateSnapshot()));
			T.TestEqual(TEXT("No gameplay publication"), Access::Revision(*F.Mode), Revision);
			T.TestEqual(TEXT("No other entropy or fallback"), F.Entropy->Calls, Entropy + int32(Failure));
			T.TestEqual(TEXT("Post-route provider boundary"), Access::Runtime(*F.Mode).GetPostRouteProviderInvocationCount(), Post + int32(Failure));
			T.TestEqual(TEXT("Initial route provider unchanged"), Access::Runtime(*F.Mode).GetInitialRouteProviderInvocationCount(), Initial);
			T.TestEqual(TEXT("Entry provider unchanged"), Access::Runtime(*F.Mode).GetEntryProviderInvocationCount(), Entry);
			T.TestEqual(TEXT("D12 provider unchanged"), Access::Runtime(*F.Mode).GetD12ProviderInvocationCount(), D12);
			T.TestEqual(TEXT("Rejected request never coordinates"), F.Calls(), Coordinator);
		}
	};
}
#endif
