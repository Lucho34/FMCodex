#pragma once
#if WITH_DEV_AUTOMATION_TESTS
#include "FMCodexNetworkNearFreeKickTestFixture.h"
namespace FMCodexPenaltyTests
{
using namespace FMCodexSetPieceSelectionTests;
using Outcome = EFMCodexNetworkTerminalOutcome;
using FBoundary = FMCodexNearFreeKickTests::FBoundary;
constexpr Kind Attack = Kind::ResolvePenaltyDirectAttackRoll;
constexpr Kind Defense = Kind::ResolvePenaltyDirectDefenseRoll;
constexpr Kind Panenka = Kind::ResolvePenaltyPanenkaRoll;
inline bool PreparePenalty(FFixture& F, bool Chip)
{
 return Prepare(F,Kind::SubmitPenaltyMethod,6) && Send(F,F.Attacker(),Request(F,F.Attacker(),Kind::SubmitPenaltyMethod,Chip))==Code::Accepted;
}
inline Code Roll(FFixture& F,Kind K,int32 D6)
{
 F.Entropy->PendingWords={uint32(D6-1)};
 auto* PC=K==Defense?F.Defender():F.Attacker();
 const auto Ack=Send(F,PC,Request(F,PC,K));
 F.Entropy->PendingWords.Reset();return Ack;
}
}
#endif
