#pragma once
#if WITH_DEV_AUTOMATION_TESTS
#include "FMCodexNetworkNearFreeKickTestFixture.h"
namespace FMCodexLongFreeKickTests
{
using namespace FMCodexSetPieceSelectionTests;
using Outcome = EFMCodexNetworkTerminalOutcome;
using FBoundary = FMCodexNearFreeKickTests::FBoundary;
constexpr Kind Attack = Kind::ResolveLongFreeKickDirectAttackRoll;
constexpr Kind Defense = Kind::ResolveLongFreeKickDirectDefenseRoll;
constexpr Kind Pair = Kind::ResolveLongFreeKickPowerRoll;
inline bool PrepareLong(FFixture& F, bool Power)
{
 return Prepare(F,Kind::SubmitLongFreeKickMethod,3) && Send(F,F.Attacker(),Request(F,F.Attacker(),Kind::SubmitLongFreeKickMethod,Power))==Code::Accepted;
}
inline Code Roll(FFixture& F,Kind K,int32 A=6,int32 B=6)
{
 F.Entropy->PendingWords={uint32(A-1)};
 if(K==Pair)F.Entropy->PendingWords.Add(uint32(B-1));
 const auto Code=Send(F,K==Defense?F.Defender():F.Attacker(),Request(F,K==Defense?F.Defender():F.Attacker(),K));
 F.Entropy->PendingWords.Reset();return Code;
}
}
#endif
