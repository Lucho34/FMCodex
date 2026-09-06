#if WITH_DEV_AUTOMATION_TESTS
#include "FMCodexNetworkContestTestFixture.h"

namespace FMCodexOrdinaryContestTests
{
using namespace FMCodexNetworkInitialRouteTests;
using Command = EMatchPlayAuthoritativeCommandKind;
using Outcome = EFMCodexNetworkTerminalOutcome;
inline Kind RollKind(bool Feet, bool Attack)
{
    return Feet ? (Attack ? Kind::ThroughBallFeetAttackRoll : Kind::ThroughBallFeetDefenseRoll)
        : (Attack ? Kind::PassControlAttackRoll : Kind::PassControlDefenseRoll);
}
inline FMatchPlayPlayerIntent Canonical(Kind K, int64 Sequence, Side Player)
{
    auto Make = [&](auto R, Command C) { R.AttackSequence=Sequence; R.RequestingSide=Player; return FMatchPlayPlayerIntent::Create(C,R); };
    switch(K)
    {
    case Kind::PassControlAttackRoll: return Make(FMatchPlayAuthoritativeResolvePassControlAttackRollRequest{},Command::ResolvePassControlAttackRoll);
    case Kind::PassControlDefenseRoll: return Make(FMatchPlayAuthoritativeResolvePassControlDefenseRollRequest{},Command::ResolvePassControlDefenseRoll);
    case Kind::ThroughBallFeetAttackRoll: return Make(FMatchPlayAuthoritativeResolveThroughBallFeetAttackRollRequest{},Command::ResolveThroughBallFeetAttackRoll);
    default: return Make(FMatchPlayAuthoritativeResolveThroughBallFeetDefenseRollRequest{},Command::ResolveThroughBallFeetDefenseRoll);
    }
}
struct FOrdinaryFixture : FFixture
{
    using FFixture::FFixture;
    bool Prepare(bool Feet, int32 Route=1)
    {
        return ReachRoute(Feet?ESkillRuleType::ThroughBall:ESkillRuleType::PassControl,Route)
            && Send(Attacker(),RequestedKind);
    }
    FFMCodexNetworkPlayerIntentAck Roll(bool Feet,bool Attack,int32 D6)
    {
        Entropy->Word=D6-1; RequestedKind=RollKind(Feet,Attack);
        auto* PC=Attack?Attacker():Defender(); return Mode->SubmitConnectionPlayerIntent(PC,Request(PC));
    }
    FFMCodexNetworkPlayerIntentAck Advance(AFMCodexNetworkMatchPlayerController* PC)
    {
        RequestedKind=Kind::AdvanceAfterTerminal; return Mode->SubmitConnectionPlayerIntent(PC,Request(PC));
    }
    FFMCodexLocalMatchInteractionView Safe(int32 Count,bool RevealTerminal) const
    {
        FFMCodexLocalMatchViewerDisclosure D; D.bRevealInitialActionPointRoll=D.bRevealRouteRoll=true;
        D.RevealedContestD6Count=Count; D.bRevealTerminalOutcome=RevealTerminal;
        return FFMCodexLocalMatchInteractionViewBuilder::BuildForViewer(Access::Session(*Mode).GetStateSnapshot(),
            Access::CallerRules(*Mode),Side::PlayerA,D);
    }
};
}

IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFMCodexOrdinaryPairs,"FMCodex.NetworkPlay.OrdinarySequentialContestTransport.01.AllPairs",
    EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
void FFMCodexOrdinaryPairs::GetTests(TArray<FString>& N,TArray<FString>& C) const
{
    for(const TCHAR* S:{TEXT("A"),TEXT("B")}) for(const TCHAR* R:{TEXT("Pass"),TEXT("Dribble"),TEXT("Run"),TEXT("Feet")})
        for(int32 A=1;A<=6;++A) for(int32 D=1;D<=6;++D)
        {auto P=FString::Printf(TEXT("%s.%s.%d.%d"),S,R,A,D);N.Add(P);C.Add(P);}
}
bool FFMCodexOrdinaryPairs::RunTest(const FString& P)
{
    using namespace FMCodexOrdinaryContestTests;
    TArray<FString> Parts;P.ParseIntoArray(Parts,TEXT("."));const bool B=Parts[0]==TEXT("B"),Feet=Parts[1]==TEXT("Feet");
    const int32 Route=Parts[1]==TEXT("Dribble")?3:Parts[1]==TEXT("Run")?5:1;
    const int32 AD6=FCString::Atoi(*Parts[2]),DD6=FCString::Atoi(*Parts[3]);
    FOrdinaryFixture F(B),Local(B);
    if(!TestTrue(TEXT("Both fixtures reach canonical actual route"),F.Prepare(Feet,Route)&&Local.Prepare(Feet,Route))) return false;
    auto* Actor=F.Attacker();auto* Defender=F.Defender();const auto Initial=Access::Session(*F.Mode).GetStateSnapshot();
    for(bool Attack:{true,false})
    {
        const FFrozen Before(F);const int32 D6=Attack?AD6:DD6;
        const auto V=(Attack?Actor:Defender)->GetOwnerView();const auto Intent=Canonical(RollKind(Feet,Attack),V.AttackSequence,V.ViewerSide);
        TestEqual(TEXT("Exact command is PlayerIntent"),FMatchPlayAuthoritativeCommandClassification::OriginOf(Intent.CommandKind),EMatchPlayAuthoritativeCommandOrigin::PlayerIntent);
        Local.Entropy->Word=D6-1;const auto Direct=Access::Runtime(*Local.Mode).SubmitPlayerIntent(Intent);
        const auto Ack=F.Roll(Feet,Attack,D6);
        if(!TestTrue(TEXT("Network and shared port both accept"),Ack.Code==Code::Accepted&&Direct.bSuccess)) return false;
        const auto State=Access::Session(*F.Mode).GetStateSnapshot();const auto& Rolls=State.CurrentAttack.ResolutionSession.PostRouteRollProgress.RollRecords;
        TestTrue(TEXT("Full State equals direct authoritative DTO path"),SameState(State,Access::Session(*Local.Mode).GetStateSnapshot()));
        TestEqual(TEXT("Exactly one successful D6 draw"),F.Entropy->Calls,Before.Entropy+1);
        TestEqual(TEXT("Exactly one provider invocation"),Access::Runtime(*F.Mode).GetPostRouteProviderInvocationCount(),Before.Post+1);
        TestEqual(TEXT("One coordinator pass"),F.Calls(),Before.Coordinator+1);
        TestEqual(TEXT("One publication"),Ack.ViewRevision,Before.Revision+1);
        TestEqual(TEXT("Actual internal continuation preserved"),Direct.CoordinatorResult.Steps.Num(),!Attack&&Feet?1:0);
        TestEqual(TEXT("Persisted prefix length"),Rolls.Num(),Attack?1:2);
        TestEqual(TEXT("Persisted attack D6"),Rolls[0].RawD6,AD6);
        if(!Attack) TestEqual(TEXT("Persisted defense D6"),Rolls[1].RawD6,DD6);
        TestEqual(TEXT("Terminal only after defense"),State.CurrentAttack.LifecycleState,
            Attack?EMatchPlayCurrentAttackLifecycleState::Active:EMatchPlayCurrentAttackLifecycleState::TerminalPendingAdvance);
        TestEqual(TEXT("No early opportunity use"),State.RuntimeState.PlayerAState.UsedAttackCount+State.RuntimeState.PlayerBState.UsedAttackCount,0);
        const auto Safe=F.Safe(Attack?1:2,!Attack);
        for(auto* PC:{F.A,F.B})
        {
            const auto& Public=PC->GetOwnerView();
            TestEqual(TEXT("Exact public accepted prefix"),Public.Contest.AttackD6,AD6);
            TestEqual(TEXT("No future defender value"),Public.Contest.DefenseD6,Attack?0:DD6);
            TestEqual(TEXT("Formula requires both D6"),Public.Contest.bFormulaResolved,!Attack);
            TestEqual(TEXT("Same safe score A"),Public.PlayerAScore,Safe.PlayerAScore);
            TestEqual(TEXT("Same safe score B"),Public.PlayerBScore,Safe.PlayerBScore);
            TestEqual(TEXT("Canonical handoff actor"),Public.ExpectedActingSide,Attack?Defender->GetOwnerView().ViewerSide:Actor->GetOwnerView().ViewerSide);
        }
        if(!Attack)
        {
            if(!TestEqual(TEXT("One canonical finishing Formula"),Safe.ResolutionFacts.FormulaContests.Num(),1)) return false;
            const auto& Formula=Safe.ResolutionFacts.FormulaContests[0];
            TestTrue(TEXT("Formula is resolved authority fact"),Formula.bHasResolvedFormula);
            const bool Goal=Formula.ResolvedResult.bIsGoal;
            TestEqual(TEXT("Goal/NoGoal exactly follows Formula"),Actor->GetOwnerView().Terminal.Outcome,Goal?Outcome::Goal:Outcome::NoGoal);
            TestEqual(TEXT("Exactly one history entry per goal"),State.GoalHistory.Num(),int32(Goal));
            if(Goal) TestEqual(TEXT("Canonical Runner scorer"),State.GoalHistory[0].ScorerCardId,State.CurrentAttack.SelectedAction.RunnerCardId);
            const auto Hidden=F.Safe(2,false);
            TestTrue(TEXT("Independent terminal gate conceals scorer/history"),Hidden.GoalHistory.IsEmpty());
            TestEqual(TEXT("Authority score may precede public permission A"),Hidden.PlayerAScore,Initial.RuntimeState.PlayerAState.Score);
            TestEqual(TEXT("Authority score may precede public permission B"),Hidden.PlayerBScore,Initial.RuntimeState.PlayerBState.Score);
            AddInfo(FString::Printf(TEXT("PAIR %s AttackTotal=%.2f DefenseTotal=%.2f Goal=%d"),*P,Formula.ResolvedResult.AttackerFinalValue,Formula.ResolvedResult.DefenderFinalValue,Goal));
        }
    }
    const int32 Post=Access::Runtime(*F.Mode).GetPostRouteProviderInvocationCount();
    const auto OldSequence=Actor->GetOwnerView().AttackSequence;
    TestEqual(TEXT("Explicit advance accepted"),F.Advance(Actor).Code,Code::Accepted);
    TestEqual(TEXT("Advance performs no contest draw"),Access::Runtime(*F.Mode).GetPostRouteProviderInvocationCount(),Post);
    const auto After=Access::Session(*F.Mode).GetStateSnapshot();
    TestFalse(TEXT("Advance clears CurrentAttack"),After.bHasCurrentAttack);
    TestEqual(TEXT("One opportunity consumed"),After.RuntimeState.PlayerAState.UsedAttackCount+After.RuntimeState.PlayerBState.UsedAttackCount,1);
    const FFrozen Frozen(F);TestEqual(TEXT("Fresh advance retry rejected"),F.Advance(Actor).Code,Code::AuthorityRejected);Frozen.Verify(*this,F);
    F.Entropy->Word=5;F.RequestedKind=Kind::RequestInitialActionPointRoll;
    TestEqual(TEXT("Next actor requests genuine Full D12"),F.Mode->SubmitConnectionPlayerIntent(Defender,F.Request(Defender)).Code,Code::Accepted);
    TestTrue(TEXT("Next attack gets a new sequence"),Defender->GetOwnerView().AttackSequence>OldSequence);
    return true;
}

IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFMCodexOrdinarySecurity,"FMCodex.NetworkPlay.OrdinarySequentialContestTransport.02.Security",
    EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
void FFMCodexOrdinarySecurity::GetTests(TArray<FString>& N,TArray<FString>& C) const
{
    for(const TCHAR* S:{TEXT("A"),TEXT("B")}) for(const TCHAR* K:{TEXT("PassAttack"),TEXT("PassDefense"),TEXT("FeetAttack"),TEXT("FeetDefense")})
    for(const TCHAR* Case:{TEXT("WrongSide"),TEXT("WrongFamily"),TEXT("BeforeSkill"),TEXT("BeforeRoute"),TEXT("WrongMatch"),TEXT("Stale"),TEXT("HugeId"),TEXT("ZeroId"),TEXT("Internal"),TEXT("Nonparticipant"),TEXT("Ordinary"),TEXT("Goalkeeper"),TEXT("Carrier"),TEXT("Marker"),TEXT("Runner"),TEXT("Helper"),TEXT("Skill"),TEXT("Branch"),TEXT("ProviderFailure"),TEXT("AckFirst"),TEXT("ViewFirst")})
    {auto P=FString::Printf(TEXT("%s.%s.%s"),S,K,Case);N.Add(P);C.Add(P);}
}
bool FFMCodexOrdinarySecurity::RunTest(const FString& P)
{
    using namespace FMCodexOrdinaryContestTests;
    TArray<FString> Parts;P.ParseIntoArray(Parts,TEXT("."));const bool Feet=Parts[1].StartsWith(TEXT("Feet")),Attack=Parts[1].EndsWith(TEXT("Attack"));const auto Case=Parts[2];
    FOrdinaryFixture F(Parts[0]==TEXT("B"));bool Ready;
    if(Case==TEXT("BeforeSkill")) Ready=F.ReachSkill();
    else if(Case==TEXT("BeforeRoute")) Ready=F.ReachRoute(Feet?ESkillRuleType::ThroughBall:ESkillRuleType::PassControl);
    else {Ready=F.Prepare(Feet);if(Ready&&!Attack) Ready=F.Roll(Feet,true,3).Code==Code::Accepted;}
    if(!TestTrue(TEXT("Canonical checkpoint"),Ready)) return false;
    auto* PC=Attack?F.Attacker():F.Defender();F.RequestedKind=RollKind(Feet,Attack);auto E=F.Request(PC);const int64 Normal=E.RequestId;Code Expected=Code::AuthorityRejected;
    auto& Client=F.Client(PC);
    const bool AckOrder=Case==TEXT("AckFirst")||Case==TEXT("ViewFirst");
    if(AckOrder)
    {
        if(!TestTrue(TEXT("Generic client begins exact contest"),Client.BeginOrdinaryContest(PC->GetOwnerView(),E.IntentKind,E))) return false;
        const auto Ack=F.Mode->SubmitConnectionPlayerIntent(PC,E);
        TestEqual(TEXT("Real connection ACK accepted"),Ack.Code,Code::Accepted);
        const auto View=PC->GetOwnerView();
        if(Case==TEXT("AckFirst")) {TestTrue(TEXT("ACK correlates"),Client.ObserveAck(Ack));TestTrue(TEXT("ACK waits for view"),Client.IsPending());Client.ObserveView(View);}
        else {Client.ObserveView(View);TestTrue(TEXT("View waits for ACK"),Client.IsPending());Client.ObserveAck(Ack);}
        TestFalse(TEXT("Both signals release generic pending"),Client.IsPending());return true;
    }
    if(Case==TEXT("WrongSide")){PC=Attack?F.Defender():F.Attacker();E.RequestId=F.Next(PC)++;}
    if(Case==TEXT("WrongFamily"))E.IntentKind=RollKind(!Feet,Attack);
    if(Case==TEXT("WrongMatch")){E.MatchInstanceId=FGuid::NewGuid();E.RequestId=MAX_int64;Expected=Code::MatchMismatch;}
    if(Case==TEXT("Stale")){++E.ExpectedAttackSequence;Expected=Code::StaleAttackSequence;}
    if(Case==TEXT("HugeId")){E.RequestId=MAX_int64;Expected=Code::InvalidPayload;}
    if(Case==TEXT("ZeroId")){E.RequestId=0;Expected=Code::InvalidPayload;}
    if(Case==TEXT("Internal")){E.IntentKind=static_cast<Kind>(255);Expected=Code::NotPlayerIntent;}
    if(Case==TEXT("Nonparticipant")){PC=F.World->SpawnActor<AFMCodexNetworkMatchPlayerController>();Expected=Code::NotParticipant;}
    if(Case==TEXT("Ordinary")){E.Deployment.CardId=TEXT("Card");Expected=Code::InvalidPayload;}
    if(Case==TEXT("Goalkeeper")){E.Goalkeeper.SlotId=TEXT("Slot");Expected=Code::InvalidPayload;}
    if(Case==TEXT("Carrier")){E.Carrier.CarrierCardId=TEXT("Card");Expected=Code::InvalidPayload;}
    if(Case==TEXT("Marker")){E.Marker.MarkerCardId=TEXT("Card");Expected=Code::InvalidPayload;}
    if(Case==TEXT("Runner")){E.Runner.RunnerCardId=TEXT("Card");Expected=Code::InvalidPayload;}
    if(Case==TEXT("Helper")){E.Helper.HelperCardId=TEXT("Card");Expected=Code::InvalidPayload;}
    if(Case==TEXT("Skill")){E.Skill.SkillId=TEXT("Skill");Expected=Code::InvalidPayload;}
    if(Case==TEXT("Branch")){E.Branch.Intent=Branch::CrossHigh;Expected=Code::InvalidPayload;}
    F.Entropy->bFail=Case==TEXT("ProviderFailure");const FFrozen Before(F);
    TestEqual(TEXT("Exact rejection ACK"),F.Mode->SubmitConnectionPlayerIntent(PC,E).Code,Expected);Before.Verify(*this,F,F.Entropy->bFail);F.Entropy->bFail=false;
    if(!Case.StartsWith(TEXT("Before")))
    {
        auto* Correct=Attack?F.Attacker():F.Defender();auto Retry=F.Request(Correct);
        if(Case==TEXT("HugeId")||Case==TEXT("WrongMatch"))Retry.RequestId=Normal;
        TestEqual(TEXT("Valid retry remains usable"),F.Mode->SubmitConnectionPlayerIntent(Correct,Retry).Code,Code::Accepted);
    }
    return true;
}

IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFMCodexOrdinaryReplay,"FMCodex.NetworkPlay.OrdinarySequentialContestTransport.03.ReplayAndWrongRoutes",
    EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
void FFMCodexOrdinaryReplay::GetTests(TArray<FString>& N,TArray<FString>& C) const
{
    for(const TCHAR* S:{TEXT("A"),TEXT("B")}) for(const TCHAR* R:{TEXT("Pass"),TEXT("Dribble"),TEXT("Run"),TEXT("Feet"),TEXT("Behind"),TEXT("Anti"),TEXT("Cross"),TEXT("LongShot"),TEXT("CutInside")})
    {auto P=FString::Printf(TEXT("%s.%s"),S,R);N.Add(P);C.Add(P);}
}
bool FFMCodexOrdinaryReplay::RunTest(const FString& P)
{
    using namespace FMCodexOrdinaryContestTests;
    const bool Feet=P.Contains(TEXT("Feet"));FOrdinaryFixture F(P.StartsWith(TEXT("B")));
    const bool Wrong=P.Contains(TEXT("Behind"))||P.Contains(TEXT("Anti"))||P.Contains(TEXT("Cross"))||P.Contains(TEXT("LongShot"))||P.Contains(TEXT("CutInside"));
    if(Wrong)
    {
        const auto Family=P.Contains(TEXT("Cross"))?ESkillRuleType::Cross:P.Contains(TEXT("LongShot"))?ESkillRuleType::LongShot:P.Contains(TEXT("CutInside"))?ESkillRuleType::CutInsideShot:ESkillRuleType::ThroughBall;
        F.Entropy->Word=(Family==ESkillRuleType::LongShot||Family==ESkillRuleType::CutInsideShot)?3:5;
        bool Ready=Family==ESkillRuleType::ThroughBall?F.Prepare(true,P.Contains(TEXT("Behind"))?3:5):F.ReachBranch(Family);
        if(!TestTrue(TEXT("Real excluded family checkpoint"),Ready))return false;
        if(Family==ESkillRuleType::LongShot||Family==ESkillRuleType::CutInsideShot)
        {
            Payload Choice;Choice.Intent=Branch::DirectShot;
            if(!TestTrue(TEXT("Real excluded direct-shot wait"),F.Send(F.Attacker(),Kind::SubmitBranchIntent,{},{},{},{},{},{},Choice)))return false;
        }
        if(Family==ESkillRuleType::Cross)
        {
            Payload Choice;Choice.Intent=Branch::CrossHigh;
            if(!F.Send(F.Attacker(),Kind::SubmitBranchIntent,{},{},{},{},{},{},Choice)||!F.Send(F.Attacker(),Kind::CrossInitialRouteRoll))return false;
        }
        for(bool IsFeet:{false,true})for(bool Attack:{true,false})
        {
            auto* PC=Attack?F.Attacker():F.Defender();F.RequestedKind=RollKind(IsFeet,Attack);const FFrozen Before(F);
            TestEqual(TEXT("Excluded actual route/family rejects before draw"),F.Mode->SubmitConnectionPlayerIntent(PC,F.Request(PC)).Code,Code::AuthorityRejected);Before.Verify(*this,F);
        }
        if(Family==ESkillRuleType::ThroughBall) TestEqual(TEXT("Conditional route offers its own new action, never Feet"),F.Attacker()->GetOwnerView().ContestAction,
            P.Contains(TEXT("Behind"))?EFMCodexNetworkContestAction::ThroughBallBehindDefenseP1AttackRoll:EFMCodexNetworkContestAction::ThroughBallAntiOffsideAttackRoll);
        else if(Family!=ESkillRuleType::Cross) TestEqual(TEXT("Specialized shot offers its own typed action, never PassControl/Feet"),F.Attacker()->GetOwnerView().ContestAction,
            Family==ESkillRuleType::LongShot?EFMCodexNetworkContestAction::LongShotDirectAttackRoll:EFMCodexNetworkContestAction::CutInsideShotDirectAttackRoll);return true;
    }
    if(!TestTrue(TEXT("Canonical route"),F.Prepare(Feet,P.Contains(TEXT("Dribble"))?3:P.Contains(TEXT("Run"))?5:1)))return false;
    const FFrozen Early(F);TestEqual(TEXT("Defense before attack"),F.Roll(Feet,false,2).Code,Code::AuthorityRejected);Early.Verify(*this,F);
    for(bool Attack:{true,false})
    {
        auto* PC=Attack?F.Attacker():F.Defender();F.RequestedKind=RollKind(Feet,Attack);const auto E=F.Request(PC);
        TestEqual(TEXT("First roll accepted"),F.Mode->SubmitConnectionPlayerIntent(PC,E).Code,Code::Accepted);
        const FFrozen Before(F);
        TestEqual(TEXT("Duplicate same RequestId"),F.Mode->SubmitConnectionPlayerIntent(PC,E).Code,Code::DuplicateOrAlreadyResolved);Before.Verify(*this,F);
        TestEqual(TEXT("Fresh-ID reroll"),F.Mode->SubmitConnectionPlayerIntent(PC,F.Request(PC)).Code,Code::AuthorityRejected);Before.Verify(*this,F);
        F.RequestedKind=Attack?Kind::CrossHighAttackRoll:Kind::CrossHighDefenseRoll;
        TestEqual(TEXT("Cross cannot enter this family"),F.Mode->SubmitConnectionPlayerIntent(PC,F.Request(PC)).Code,Code::AuthorityRejected);Before.Verify(*this,F);
    }
    const FFrozen Terminal(F);for(bool Attack:{true,false}) {TestEqual(TEXT("No post-terminal contest"),F.Roll(Feet,Attack,2).Code,Code::AuthorityRejected);Terminal.Verify(*this,F);}
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexOrdinaryClosedWire,"FMCodex.NetworkPlay.OrdinarySequentialContestTransport.04.ClosedWire",
    EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FFMCodexOrdinaryClosedWire::RunTest(const FString&)
{
    using namespace FMCodexOrdinaryContestTests;
    FFMCodexNetworkIntentLedger Ledger;Envelope E;E.MatchInstanceId=FGuid::NewGuid();E.ExpectedAttackSequence=1;
    for(bool Feet:{false,true})for(bool Attack:{true,false})
    {
        const auto K=RollKind(Feet,Attack);
        for(int32 Mask=0;Mask<256;++Mask)
        {
            Envelope P;P.IntentKind=K;
            if(Mask&1)P.Deployment.CardId=TEXT("Card");if(Mask&2)P.Goalkeeper.SlotId=TEXT("Slot");
            if(Mask&4)P.Carrier.CarrierCardId=TEXT("Card");if(Mask&8)P.Marker.MarkerCardId=TEXT("Card");
            if(Mask&16)P.Runner.RunnerCardId=TEXT("Card");if(Mask&32)P.Helper.HelperCardId=TEXT("Card");
            if(Mask&64)P.Skill.SkillId=TEXT("Skill");if(Mask&128)P.Branch.Intent=Branch::CrossHigh;
            TestEqual(TEXT("Every mixed member mask is closed"),P.ValidatePayloadShape(),Mask==0?Code::None:Code::InvalidPayload);
        }
        E.IntentKind=K;++E.RequestId;TestTrue(TEXT("Same cross-kind ledger"),Ledger.Consume(E.MatchInstanceId,E));
    }
    E.RequestId+=1025;TestEqual(TEXT("Huge jump rejected"),Ledger.Check(E.MatchInstanceId,E),Code::InvalidPayload);
    E.RequestId=5;TestTrue(TEXT("Huge jump did not poison high-water"),Ledger.Consume(E.MatchInstanceId,E));
    E.RequestId=1029;TestEqual(TEXT("Exact 1024 window accepted"),Ledger.Check(E.MatchInstanceId,E),Code::None);
    return true;
}

IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFMCodexOrdinaryRealStale,"FMCodex.NetworkPlay.OrdinarySequentialContestTransport.05.RealAttackNToNPlusOne",
    EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
void FFMCodexOrdinaryRealStale::GetTests(TArray<FString>& N,TArray<FString>& C) const
{
    for(const TCHAR* S:{TEXT("A"),TEXT("B")})for(const TCHAR* K:{TEXT("PassAttack"),TEXT("PassDefense"),TEXT("FeetAttack"),TEXT("FeetDefense")})
    {auto P=FString::Printf(TEXT("%s.%s"),S,K);N.Add(P);C.Add(P);}
}
bool FFMCodexOrdinaryRealStale::RunTest(const FString& P)
{
    using namespace FMCodexOrdinaryContestTests;const bool Feet=P.Contains(TEXT("Feet")),Attack=P.EndsWith(TEXT("Attack"));
    FOrdinaryFixture F(P.StartsWith(TEXT("B")));
    if(!TestTrue(TEXT("Real attack N same-family route"),F.Prepare(Feet)))return false;
    if(!Attack&&!F.Send(F.Attacker(),RollKind(Feet,true)))return false;
    auto* Old=Attack?F.Attacker():F.Defender();F.RequestedKind=RollKind(Feet,Attack);auto Held=F.Request(Old);
    if(!F.Send(Old,F.RequestedKind)||(Attack&&!F.Send(F.Defender(),RollKind(Feet,false))))return false;
    if(!TestTrue(TEXT("Real explicit advance"),F.Send(F.Attacker(),Kind::AdvanceAfterTerminal)))return false;
    F.Entropy->Word=5;
    if(!TestTrue(TEXT("Real attack N+1 matching family route"),F.Prepare(Feet)))return false;
    if(!Attack&&!F.Send(F.Attacker(),RollKind(Feet,true)))return false;
    auto* PC=Attack?F.Attacker():F.Defender();Held.RequestId=F.Next(PC)++;const FFrozen Before(F);
    TestEqual(TEXT("Fresh ID old sequence rejected at matching legal checkpoint"),F.Mode->SubmitConnectionPlayerIntent(PC,Held).Code,Code::StaleAttackSequence);Before.Verify(*this,F);
    F.RequestedKind=RollKind(Feet,Attack);
    TestEqual(TEXT("Matching current sequence remains usable"),F.Mode->SubmitConnectionPlayerIntent(PC,F.Request(PC)).Code,Code::Accepted);
    return true;
}
#endif
