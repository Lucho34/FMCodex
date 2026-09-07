#pragma once
#if WITH_DEV_AUTOMATION_TESTS
#include "FMCodexNetworkSetPieceSelectionTestFixture.h"
namespace FMCodexNearFreeKickTests
{
using namespace FMCodexSetPieceSelectionTests;
using Outcome = EFMCodexNetworkTerminalOutcome;
constexpr Kind Attack = Kind::ResolveShortFreeKickDirectAttackRoll;
constexpr Kind Defense = Kind::ResolveShortFreeKickDirectDefenseRoll;
constexpr Kind Pair = Kind::ResolveShortFreeKickAngledRoll;
inline bool PrepareNear(FFixture& F, bool Angled)
{
 return Prepare(F,Kind::SubmitShortFreeKickMethod) && Send(F,F.Attacker(),Request(F,F.Attacker(),Kind::SubmitShortFreeKickMethod,Angled))==Code::Accepted;
}
inline Code Roll(FFixture& F,Kind K,int32 A=6,int32 B=6)
{
 F.Entropy->PendingWords={uint32(A-1)};
 if(K==Pair) F.Entropy->PendingWords.Add(uint32(B-1));
 const auto Code=Send(F,K==Defense?F.Defender():F.Attacker(),Request(F,K==Defense?F.Defender():F.Attacker(),K));
 F.Entropy->PendingWords.Reset(); return Code;
}
struct FBoundary : FUnchanged
{
 int32 Post,Recovery;
 explicit FBoundary(FFixture& F):FUnchanged(F),Post(Access::Runtime(*F.Mode).GetPostRouteProviderInvocationCount()),Recovery(Access::Runtime(*F.Mode).GetRecoveryProviderInvocationCount()){}
 void VerifyFailure(FAutomationTestBase& T,FFixture& F,int32 Draws=0) const
 {
  T.TestTrue(TEXT("Complete authoritative state unchanged"),SameState(State,Access::Session(*F.Mode).GetStateSnapshot()));
  T.TestEqual(TEXT("No publication"),Access::Revision(*F.Mode),Revision);
  T.TestEqual(TEXT("Exact entropy attempts"),F.Entropy->Calls,EntropyCalls+Draws);
  T.TestEqual(TEXT("Exact post-route attempts"),Access::Runtime(*F.Mode).GetPostRouteProviderInvocationCount(),Post+Draws);
  T.TestEqual(TEXT("No coordinator on failure"),F.Calls(),CoordinatorCalls);
  T.TestEqual(TEXT("No entry RNG"),Access::Runtime(*F.Mode).GetEntryProviderInvocationCount(),EntryCalls);
  T.TestEqual(TEXT("No D12 RNG"),Access::Runtime(*F.Mode).GetD12ProviderInvocationCount(),D12Calls);
  T.TestEqual(TEXT("No initial route RNG"),Access::Runtime(*F.Mode).GetInitialRouteProviderInvocationCount(),RouteCalls);
  T.TestEqual(TEXT("No Recovery RNG"),Access::Runtime(*F.Mode).GetRecoveryProviderInvocationCount(),Recovery);
 }
};
}
#endif
