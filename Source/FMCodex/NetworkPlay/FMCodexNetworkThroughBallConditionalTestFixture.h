#pragma once
#if WITH_DEV_AUTOMATION_TESTS
#include "FMCodexNetworkContestTestFixture.h"
namespace FMCodexThroughBallConditionalTests
{
using namespace FMCodexNetworkInitialRouteTests;
using Shot = EMatchPlayThroughBallOneOnOneShotChoice;
using Command = EMatchPlayAuthoritativeCommandKind;
using Outcome = EFMCodexNetworkTerminalOutcome;
inline const TArray<Kind>& Commands()
{
    static const TArray<Kind> K = { Kind::ThroughBallBehindDefenseP1AttackRoll, Kind::ThroughBallBehindDefenseP1DefenseRoll,
        Kind::ThroughBallAntiOffsideAttackRoll, Kind::SubmitThroughBallOneOnOneShotChoice,
        Kind::ThroughBallOneOnOneDirectShotAttackRoll, Kind::ThroughBallOneOnOneDirectShotDefenseRoll,
        Kind::ThroughBallOneOnOneChipShotAttackRoll };
    return K;
}
inline bool IsDefense(Kind K)
{
    return K==Kind::ThroughBallBehindDefenseP1DefenseRoll || K==Kind::ThroughBallOneOnOneDirectShotDefenseRoll;
}
inline FMatchPlayPlayerIntent Canonical(const Envelope& E, Side Player)
{
    auto Make = [&](auto R, Command C) { R.AttackSequence=E.ExpectedAttackSequence;R.RequestingSide=Player;return FMatchPlayPlayerIntent::Create(C,R); };
    switch(E.IntentKind)
    {
    case Kind::ThroughBallBehindDefenseP1AttackRoll: return Make(FMatchPlayAuthoritativeResolveThroughBallBehindDefenseP1AttackRollRequest{},Command::ResolveThroughBallBehindDefenseP1AttackRoll);
    case Kind::ThroughBallBehindDefenseP1DefenseRoll: return Make(FMatchPlayAuthoritativeResolveThroughBallBehindDefenseP1DefenseRollRequest{},Command::ResolveThroughBallBehindDefenseP1DefenseRoll);
    case Kind::ThroughBallAntiOffsideAttackRoll: return Make(FMatchPlayAuthoritativeResolveThroughBallAntiOffsideAttackRollRequest{},Command::ResolveThroughBallAntiOffsideAttackRoll);
    case Kind::ThroughBallOneOnOneDirectShotAttackRoll: return Make(FMatchPlayAuthoritativeResolveThroughBallOneOnOneDirectShotAttackRollRequest{},Command::ResolveThroughBallOneOnOneDirectShotAttackRoll);
    case Kind::ThroughBallOneOnOneDirectShotDefenseRoll: return Make(FMatchPlayAuthoritativeResolveThroughBallOneOnOneDirectShotDefenseRollRequest{},Command::ResolveThroughBallOneOnOneDirectShotDefenseRoll);
    case Kind::ThroughBallOneOnOneChipShotAttackRoll: return Make(FMatchPlayAuthoritativeResolveThroughBallOneOnOneChipShotAttackRollRequest{},Command::ResolveThroughBallOneOnOneChipShotAttackRoll);
    default:
        FMatchPlayAuthoritativeSubmitThroughBallOneOnOneShotChoiceRequest R;
        R.ExpectedAttackSequence=E.ExpectedAttackSequence;R.RequestingSide=Player;R.Choice=E.OneOnOneChoice;
        return FMatchPlayPlayerIntent::Create(Command::SubmitThroughBallOneOnOneShotChoice,R);
    }
}
struct FConditionalFixture : FFixture
{
    using FFixture::FFixture;
    bool Route(int32 D6=3,bool GK=false)
    {
        Entropy->Word=5;
        return ReachRoute(ESkillRuleType::ThroughBall,D6,true,false,GK)&&Send(Attacker(),Kind::ThroughBallInitialRouteRoll);
    }
    Envelope For(Kind K,Shot Choice=Shot::None,AFMCodexNetworkMatchPlayerController* PC=nullptr)
    {
        RequestedKind=K;auto E=Request(PC?PC:IsDefense(K)?Defender():Attacker());E.OneOnOneChoice=Choice;return E;
    }
    FFMCodexNetworkPlayerIntentAck Roll(Kind K,int32 D6=6,Shot Choice=Shot::None)
    {
        Entropy->Word=D6-1;auto* PC=IsDefense(K)?Defender():Attacker();Envelope E;auto& C=Client(PC);
        const bool Began=K==Kind::SubmitThroughBallOneOnOneShotChoice?C.BeginOneOnOne(PC->GetOwnerView(),Choice,E)
            :K==Kind::AdvanceAfterTerminal?C.BeginAdvance(PC->GetOwnerView(),E):C.BeginOrdinaryContest(PC->GetOwnerView(),K,E);
        if(!Began||E.RequestId<Next(PC))
        {
            if(Began){FFMCodexNetworkPlayerIntentAck Rejected;Rejected.MatchInstanceId=E.MatchInstanceId;Rejected.RequestId=E.RequestId;Rejected.Code=Code::InvalidPayload;C.ObserveAck(Rejected);}
            return Mode->SubmitConnectionPlayerIntent(PC,For(K,Choice,PC));
        }
        Next(PC)=E.RequestId+1;auto Ack=Mode->SubmitConnectionPlayerIntent(PC,E);C.ObserveView(PC->GetOwnerView());C.ObserveAck(Ack);return Ack;
    }
    bool Entry(bool Anti=false,bool GK=false)
    {
        if(!Route(Anti?5:3,GK))return false;
        if(Anti) return Roll(Kind::ThroughBallAntiOffsideAttackRoll).Code==Code::Accepted&&!Attacker()->GetOwnerView().OneOnOneOptions.IsEmpty();
        return Roll(Kind::ThroughBallBehindDefenseP1AttackRoll).Code==Code::Accepted
            &&Roll(Kind::ThroughBallBehindDefenseP1DefenseRoll,1).Code==Code::Accepted&&!Attacker()->GetOwnerView().OneOnOneOptions.IsEmpty();
    }
    bool Ready(Kind K)
    {
        if(K==Kind::ThroughBallBehindDefenseP1AttackRoll)return Route();
        if(K==Kind::ThroughBallBehindDefenseP1DefenseRoll)return Route()&&Roll(Kind::ThroughBallBehindDefenseP1AttackRoll).Code==Code::Accepted;
        if(K==Kind::ThroughBallAntiOffsideAttackRoll)return Route(5);
        if(!Entry())return false;
        if(K==Kind::SubmitThroughBallOneOnOneShotChoice)return true;
        if(Roll(Kind::SubmitThroughBallOneOnOneShotChoice,6,K==Kind::ThroughBallOneOnOneChipShotAttackRoll?Shot::ChipShot:Shot::DirectShot).Code!=Code::Accepted)return false;
        return K!=Kind::ThroughBallOneOnOneDirectShotDefenseRoll||Roll(Kind::ThroughBallOneOnOneDirectShotAttackRoll,3).Code==Code::Accepted;
    }
    FFMCodexLocalMatchInteractionView Safe(int32 Count,bool Terminal=true,bool Pending=true,bool Route=true)
    {
        FFMCodexLocalMatchViewerDisclosure D;D.bRevealInitialActionPointRoll=true;D.bRevealRouteRoll=Route;
        D.RevealedContestD6Count=Count;D.bRevealTerminalOutcome=Terminal;D.bPreservePendingOrdinaryFormula=Pending;
        return FFMCodexLocalMatchInteractionViewBuilder::BuildForViewer(Access::Session(*Mode).GetStateSnapshot(),Access::CallerRules(*Mode),Side::PlayerA,D);
    }
};
}
#endif
