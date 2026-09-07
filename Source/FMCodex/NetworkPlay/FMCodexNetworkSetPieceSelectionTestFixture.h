#pragma once
#if WITH_DEV_AUTOMATION_TESTS
#include "FMCodexNetworkPlayerFacingTestFixture.h"
namespace FMCodexSetPieceSelectionTests
{
using namespace FMCodexNetworkInitialRouteTests;
using Type = ESetPieceSelectedType;
using Stage = EMatchPlaySetPieceCarrierRouteStage;
using Wait = EFMCodexNetworkEntryWait;
inline Kind MethodKind(int32 D6) { return D6 == 5 ? Kind::SubmitShortFreeKickMethod : D6 == 6 ? Kind::SubmitPenaltyMethod : Kind::SubmitLongFreeKickMethod; }
inline Type ExpectedType(int32 D6) { return D6 <= 2 ? Type::Corner : D6 <= 4 ? Type::LongFreeKick : D6 == 5 ? Type::ShortFreeKick : Type::Penalty; }
inline Envelope Request(FFixture& F, AFMCodexNetworkMatchPlayerController* PC, Kind K, bool Alternate = false)
{
 auto E = F.Request(PC); E.IntentKind = K;
 if (K == Kind::SubmitSetPieceCarrier) E.SetPieceCardId = PC->GetOwnerView().SetPiece.TakerOptions.IsEmpty() ? FName(TEXT("Missing")) : PC->GetOwnerView().SetPiece.TakerOptions[0];
 if (K == Kind::SubmitShortFreeKickMethod) E.NearMethod = Alternate ? EMatchPlayShortFreeKickMethod::Angled : EMatchPlayShortFreeKickMethod::Direct;
 if (K == Kind::SubmitLongFreeKickMethod) E.LongMethod = Alternate ? EMatchPlayLongFreeKickMethod::Power : EMatchPlayLongFreeKickMethod::Direct;
 if (K == Kind::SubmitPenaltyMethod) E.PenaltyMethod = Alternate ? EMatchPlayPenaltyMethod::Panenka : EMatchPlayPenaltyMethod::Direct;
 return E;
}
inline Code Send(FFixture& F, AFMCodexNetworkMatchPlayerController* PC, const Envelope& E) { return F.Mode->SubmitConnectionPlayerIntent(PC,E).Code; }
inline bool Entry(FFixture& F, int32 D12 = 9)
{ F.Entropy->Word = D12-1; return F.Send(F.Attacker(),Kind::RequestInitialActionPointRoll) && F.Attacker()->GetOwnerView().EntryWait == Wait::SetPieceTypeRoll; }
inline bool TypeRoll(FFixture& F, int32 D6)
{ F.Entropy->Word = D6-1; return Send(F,F.Attacker(),Request(F,F.Attacker(),Kind::RequestSetPieceTypeRoll)) == Code::Accepted; }
inline FName Eligible(FFixture& F, bool Angled = true)
{
 const auto State = Access::Session(*F.Mode).GetStateSnapshot();
 const auto& Cards = F.Attacker()==F.A ? State.CardSnapshotAuthority.PlayerACardSnapshots.Cards : State.CardSnapshotAuthority.PlayerBCardSnapshots.Cards;
 for (auto Id : F.Attacker()->GetOwnerView().SetPiece.TakerOptions)
  if (const auto* C = Cards.FindByPredicate([&](const auto& Card){return Card.CardId==Id;}))
   if ((C->Attributes.Shooting+C->Attributes.Passing>=8)==Angled) return Id;
 return NAME_None;
}
inline bool Taker(FFixture& F, bool Angled = true)
{ auto E = Request(F,F.Attacker(),Kind::SubmitSetPieceCarrier); E.SetPieceCardId = Eligible(F,Angled); return Send(F,F.Attacker(),E)==Code::Accepted; }
inline bool Prepare(FFixture& F, Kind K, int32 D6 = 5)
{
 Access::SetPiecePresentation(*F.Mode);
 return Entry(F) && (K==Kind::RequestSetPieceTypeRoll || (TypeRoll(F,D6) && (K==Kind::SubmitSetPieceCarrier || Taker(F))));
}
inline bool BeginFresh(FFixture& F, FFMCodexNetworkIntentClientState& C, AFMCodexNetworkMatchPlayerController* PC, const Envelope& Payload, Envelope& E)
{
 auto Begin=[&]{return C.BeginSetPiece(PC->GetOwnerView(),Payload.IntentKind,E,Payload.SetPieceCardId,Payload.NearMethod,Payload.LongMethod,Payload.PenaltyMethod);};
 bool OK=Begin();
 while(OK && E.RequestId<F.Next(PC))
 { FFMCodexNetworkPlayerIntentAck A;A.MatchInstanceId=E.MatchInstanceId;A.RequestId=E.RequestId;A.Code=Code::InvalidPayload;C.ObserveAck(A);OK=Begin(); }
 if(OK)F.Next(PC)=E.RequestId+1;return OK;
}
inline void NoScoreChange(FAutomationTestBase& T,const FMatchPlayState& B,const FMatchPlayState& A)
{
 T.TestEqual(TEXT("No score A mutation"),A.RuntimeState.PlayerAState.Score,B.RuntimeState.PlayerAState.Score);
 T.TestEqual(TEXT("No score B mutation"),A.RuntimeState.PlayerBState.Score,B.RuntimeState.PlayerBState.Score);
 T.TestEqual(TEXT("No scorer/history mutation"),A.GoalHistory.Num(),B.GoalHistory.Num());
}
}
#endif
