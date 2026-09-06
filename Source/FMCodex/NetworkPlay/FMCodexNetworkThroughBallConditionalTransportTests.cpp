#if WITH_DEV_AUTOMATION_TESTS
#include "FMCodexNetworkThroughBallConditionalTestFixture.h"

IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFMCodexThroughBallConditionalMatrix,"FMCodex.NetworkPlay.ThroughBallConditionalTransport.Matrix",
    EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
void FFMCodexThroughBallConditionalMatrix::GetTests(TArray<FString>& N,TArray<FString>& C) const
{
    for(const TCHAR* S:{TEXT("A"),TEXT("B")})
    {
        auto Add=[&](const FString& P){N.Add(P);C.Add(P);};
        for(int32 A=1;A<=6;++A)for(int32 D=1;D<=(A<=2?1:6);++D)Add(FString::Printf(TEXT("%s.Behind.%d.%d"),S,A,D));
        for(int32 A=1;A<=6;++A)Add(FString::Printf(TEXT("%s.Anti.%d.0"),S,A));
        for(const TCHAR* Entry:{TEXT("Behind"),TEXT("Anti")})
        {
            for(int32 GK=0;GK<=1;++GK)for(int32 A=1;A<=6;++A)for(int32 D=1;D<=6;++D)
                Add(FString::Printf(TEXT("%s.%sDirect.%d.%d.%d"),S,Entry,A,D,GK));
            for(int32 A=1;A<=6;++A)Add(FString::Printf(TEXT("%s.%sChip.%d.0"),S,Entry,A));
        }
    }
}
bool FFMCodexThroughBallConditionalMatrix::RunTest(const FString& P)
{
    using namespace FMCodexThroughBallConditionalTests;
    TArray<FString> Part;P.ParseIntoArray(Part,TEXT("."));const bool B=Part[0]==TEXT("B"),Direct=Part[1].Contains(TEXT("Direct")),Chip=Part[1].Contains(TEXT("Chip"));
    const bool Anti=Part[1].StartsWith(TEXT("Anti")),GK=Part.Num()>4&&Part[4]==TEXT("1");
    const int32 AD6=FCString::Atoi(*Part[2]),DD6=FCString::Atoi(*Part[3]);FConditionalFixture F(B),Local(B);
    auto Step=[&](Kind K,int32 D6,Shot Choice=Shot::None)
    {
        auto* PC=IsDefense(K)?F.Defender():F.Attacker();auto E=F.For(K,Choice,PC);
        const auto Intent=Canonical(E,PC->GetOwnerView().ViewerSide);const FFrozen Before(F);
        TestEqual(TEXT("Genuine canonical PlayerIntent"),FMatchPlayAuthoritativeCommandClassification::OriginOf(Intent.CommandKind),EMatchPlayAuthoritativeCommandOrigin::PlayerIntent);
        F.Entropy->Word=Local.Entropy->Word=D6-1;
        const auto Oracle=Access::Runtime(*Local.Mode).SubmitPlayerIntent(Intent);const auto Ack=F.Mode->SubmitConnectionPlayerIntent(PC,E);
        if(!TestTrue(TEXT("Connection and direct typed port accept"),Oracle.bSuccess&&Ack.Code==Code::Accepted))return false;
        TestTrue(TEXT("Full State equals unchanged canonical typed path"),SameState(Access::Session(*F.Mode).GetStateSnapshot(),Access::Session(*Local.Mode).GetStateSnapshot()));
        const int32 Draw=K==Kind::SubmitThroughBallOneOnOneShotChoice?0:1;
        TestEqual(TEXT("Exact entropy consumption"),F.Entropy->Calls,Before.Entropy+Draw);
        TestEqual(TEXT("Exact provider consumption"),Access::Runtime(*F.Mode).GetPostRouteProviderInvocationCount(),Before.Post+Draw);
        TestEqual(TEXT("One Coordinator pass"),F.Calls(),Before.Coordinator+1);TestEqual(TEXT("One stable revision"),Ack.ViewRevision,Before.Revision+1);
        const auto State=Access::Session(*F.Mode).GetStateSnapshot();const auto& Records=State.CurrentAttack.ResolutionSession.PostRouteRollProgress.RollRecords;
        for(auto* Viewer:{F.A,F.B})TestEqual(TEXT("Complete accepted prefix, no future values"),Viewer->GetOwnerView().AcceptedContestRolls.Num(),Records.Num());
        AddInfo(FString::Printf(TEXT("STEP %s Kind=%d D6=%d Draw=%d Internal=%d Terminal=%d Prefix=%d"),*P,int32(K),D6,Draw,Oracle.CoordinatorResult.Steps.Num(),int32(State.CurrentAttack.LifecycleState),Records.Num()));
        const FFrozen After(F);
        TestEqual(TEXT("Same ID duplicate"),F.Mode->SubmitConnectionPlayerIntent(PC,E).Code,Code::DuplicateOrAlreadyResolved);After.Verify(*this,F);
        TestEqual(TEXT("Fresh ID cannot reroll/rechoose"),F.Mode->SubmitConnectionPlayerIntent(PC,F.For(K,Choice,PC)).Code,Code::AuthorityRejected);After.Verify(*this,F);
        return true;
    };
    if(!TestTrue(TEXT("Canonical real route for both adapters"),F.Route(Anti?5:3,GK)&&Local.Route(Anti?5:3,GK)))return false;
    if(!Step(Anti?Kind::ThroughBallAntiOffsideAttackRoll:Kind::ThroughBallBehindDefenseP1AttackRoll,Direct||Chip?6:AD6))return false;
    if(!Anti&&(Direct||Chip||AD6>=3))
    {
        TestEqual(TEXT("Actual conditional defense actor"),F.Attacker()->GetOwnerView().ExpectedActingSide,F.Defender()->GetOwnerView().ViewerSide);
        if(!Step(Kind::ThroughBallBehindDefenseP1DefenseRoll,Direct||Chip?1:DD6))return false;
    }
    if(Direct||Chip)
    {
        TestEqual(TEXT("OneOnOne offers exact two canonical choices"),F.Attacker()->GetOwnerView().OneOnOneOptions.Num(),2);
        TestEqual(TEXT("No premature terminal"),F.Attacker()->GetOwnerView().Terminal.Outcome,Outcome::None);
        TestEqual(TEXT("No premature history"),F.Attacker()->GetOwnerView().PublicGoalHistory.Num(),0);
        if(!Step(Kind::SubmitThroughBallOneOnOneShotChoice,1,Direct?Shot::DirectShot:Shot::ChipShot))return false;
        if(!Step(Direct?Kind::ThroughBallOneOnOneDirectShotAttackRoll:Kind::ThroughBallOneOnOneChipShotAttackRoll,AD6))return false;
        if(Direct&&!Step(Kind::ThroughBallOneOnOneDirectShotDefenseRoll,DD6))return false;
    }
    const auto State=Access::Session(*F.Mode).GetStateSnapshot();const auto View=F.Attacker()->GetOwnerView();
    const bool Terminal=State.CurrentAttack.LifecycleState==EMatchPlayCurrentAttackLifecycleState::TerminalPendingAdvance;
    TestEqual(TEXT("Public terminal matches persisted lifecycle"),View.Terminal.Outcome!=Outcome::None,Terminal);
    TestEqual(TEXT("Only terminal permits Advance"),View.bCanAdvance,Terminal);
    if(!Terminal)
    {
        TestEqual(TEXT("Only OneOnOne continuation remains"),View.OneOnOneOptions.Num(),2);
        const FFrozen Before(F);TestEqual(TEXT("No advance from OneOnOne"),F.Roll(Kind::AdvanceAfterTerminal).Code,Code::AuthorityRejected);Before.Verify(*this,F);return true;
    }
    if(!Direct&&!Chip)TestEqual(TEXT("Exact conditional terminal semantic"),View.Terminal.Outcome,Anti?Outcome::Offside:AD6<=2?Outcome::OutOfPlay:Outcome::DefenderStoppedAttack);
    if(Chip)TestEqual(TEXT("Canonical Chip mapping"),View.Terminal.Outcome,AD6>=4?Outcome::Goal:Outcome::NoGoal);
    const bool Goal=View.Terminal.Outcome==Outcome::Goal;
    TestEqual(TEXT("Exactly one history entry iff goal"),State.GoalHistory.Num(),int32(Goal));
    if(Goal)TestEqual(TEXT("Canonical Runner scorer"),State.GoalHistory[0].ScorerCardId,State.CurrentAttack.SelectedAction.RunnerCardId);
    const auto Hidden=F.Safe(4,false);TestTrue(TEXT("Independent permission conceals current history"),Hidden.GoalHistory.IsEmpty());
    TestEqual(TEXT("No hidden score A"),Hidden.PlayerAScore,0);TestEqual(TEXT("No hidden score B"),Hidden.PlayerBScore,0);
    const auto HiddenNetwork=FFMCodexNetworkClientViewSnapshotFactory::Build(Hidden,F.Mode->GetMatchInstanceId(),1,Side::PlayerA,EFMCodexNetworkBootstrapState::MatchReady);
    TestEqual(TEXT("No hidden terminal result"),HiddenNetwork.Terminal.Outcome,Outcome::None);
    auto* OldActor=F.Attacker();auto* Next=F.Defender();const int32 Post=Access::Runtime(*F.Mode).GetPostRouteProviderInvocationCount();
    TestEqual(TEXT("Explicit shared Advance"),F.Roll(Kind::AdvanceAfterTerminal).Code,Code::Accepted);
    TestEqual(TEXT("Advance never draws extra post-route RNG"),Access::Runtime(*F.Mode).GetPostRouteProviderInvocationCount(),Post);
    TestFalse(TEXT("Old scope cleared"),Access::Session(*F.Mode).GetStateSnapshot().bHasCurrentAttack);
    TestTrue(TEXT("Next actor is previous defender"),F.Attacker()==Next&&F.Attacker()!=OldActor);
    F.Entropy->Word=5;F.RequestedKind=Kind::RequestInitialActionPointRoll;
    TestEqual(TEXT("Next genuine Full D12"),F.Mode->SubmitConnectionPlayerIntent(Next,F.Request(Next)).Code,Code::Accepted);
    return true;
}
#endif
