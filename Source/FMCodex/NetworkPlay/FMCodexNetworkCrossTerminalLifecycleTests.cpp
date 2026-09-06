#if WITH_DEV_AUTOMATION_TESTS
#include "FMCodexNetworkInitialRouteTestFixture.h"
#include "../CoreRules/PlayCardResolver.h"

struct FFMCodexNetworkCrossTerminalTestAccess
{
	static FInitialRouteEntropy* Milestone(AFMCodexNetworkMatchGameMode& Mode, bool Goal, bool Final)
	{
		Mode.BootstrapConfiguration = (!Goal != Final)
			? FFMCodexNetworkBootstrapConfigurationFactory::CreateBFirstAutomationMatch()
			: FFMCodexNetworkBootstrapConfigurationFactory::CreatePrototypeMatch();
		Mode.BootstrapConfiguration.MatchConfiguration.OpeningInput.OpeningInput.bUseDevOneAttackPerSide = Final;
		auto Entropy=MakeUnique<FInitialRouteEntropy>();auto* Source=Entropy.Get();
		Mode.MatchRuntime=MakeUnique<FFMCodexNetworkMatchRuntime>(Mode.MatchInstanceId,MoveTemp(Entropy));
		Mode.MatchRuntime->EnableCrossTerminalAutomation(Goal,Final);
		if(!Mode.MatchRuntime->InitializeOnce(Mode.BootstrapConfiguration).bSuccess || !Mode.MatchRuntime->PrepareCrossTerminalMilestone(Goal,Final)) {return nullptr;}
		Mode.PublishOwnerViews(EFMCodexNetworkBootstrapState::MatchReady);
		return Source;
	}
	static int64 SetReveal(FFMCodexNetworkMatchRuntime& Runtime, int64 Sequence)
	{
		const int64 Old = Runtime.DisclosedTerminalAttackSequence;
		Runtime.DisclosedTerminalAttackSequence = Sequence;
		return Old;
	}
};
namespace FMCodexNetworkCrossTerminalTests
{
	using namespace FMCodexNetworkInitialRouteTests;
	using Outcome = EFMCodexNetworkTerminalOutcome;
	using Command = EMatchPlayAuthoritativeCommandKind;
	template<class T> bool Same(const T& A, const T& B) { return T::StaticStruct()->CompareScriptStruct(&A, &B, 0); }
	Kind Roll(bool High, bool Attack)
	{
		return High ? (Attack ? Kind::CrossHighAttackRoll : Kind::CrossHighDefenseRoll)
			: (Attack ? Kind::CrossLowAttackRoll : Kind::CrossLowDefenseRoll);
	}
	FMatchPlayPlayerIntent Advance(int64 Sequence, Side Actor)
	{
		FMatchPlayAuthoritativeAdvanceAfterTerminalRequest R; R.AttackSequence = Sequence; R.RequestingSide = Actor;
		return FMatchPlayPlayerIntent::Create(Command::AdvanceAfterTerminal, R);
	}
	struct FTerminalFixture : FFixture
	{
		bool Final;
		FTerminalFixture(bool BFirst = false, bool InFinal = false) : FFixture(BFirst != InFinal, 6, InFinal), Final(InFinal) {}
		bool Prepare(bool High, bool Goal = true, bool GK = false, int32 Stop = 3)
		{
			if (Final)
			{
				Entropy->Word = 0;
				auto* PC = Attacker(); const auto V = PC->GetOwnerView();
				if (!Send(PC, Kind::RequestInitialActionPointRoll)) { return false; }
				// Only prelude setup bypasses transport; the Cross advance under test never does.
				if (!Access::Runtime(*Mode).SubmitPlayerIntent(Advance(V.AttackSequence, V.ViewerSide)).bSuccess) { return false; }
				Access::Publish(*Mode);
			}
			Entropy->Word = 5;
			if (!ReachRoute(ESkillRuleType::Cross, 5, !High, false, GK)) { return false; }
			if (Stop == 0) { return true; }
			if (!Send(Attacker(), Kind::CrossInitialRouteRoll)) { return false; }
			if (Stop == 1) { return true; }
			Entropy->Word = Goal ? 5 : 0;
			if (!Send(Attacker(), Roll(High, true))) { return false; }
			if (Stop == 2) { return true; }
			Entropy->Word = Goal ? 0 : 5;
			return Send(Defender(), Roll(High, false));
		}
		Envelope AdvanceRequest(AFMCodexNetworkMatchPlayerController* PC)
		{
			auto E = Request(PC); E.IntentKind = Kind::AdvanceAfterTerminal; return E;
		}
	};
	struct FFrozen
	{
		FMatchPlayState State;
		int32 Revision, Entropy, Entry, D12, Route, Post, Recovery, Coordinator;
		FFrozen(FFixture& F) : State(Access::Session(*F.Mode).GetStateSnapshot()), Revision(Access::Revision(*F.Mode)),
			Entropy(F.Entropy->Calls), Entry(Access::Runtime(*F.Mode).GetEntryProviderInvocationCount()),
			D12(Access::Runtime(*F.Mode).GetD12ProviderInvocationCount()), Route(Access::Runtime(*F.Mode).GetInitialRouteProviderInvocationCount()),
			Post(Access::Runtime(*F.Mode).GetPostRouteProviderInvocationCount()), Recovery(Access::Runtime(*F.Mode).GetRecoveryProviderInvocationCount()), Coordinator(F.Calls()) {}
		void Verify(FAutomationTestBase& T, FFixture& F, int32 FailedDraws = 0) const
		{
			T.TestTrue(TEXT("Entire authoritative state unchanged"), Same(State, Access::Session(*F.Mode).GetStateSnapshot()));
			T.TestEqual(TEXT("No stable publication"), Access::Revision(*F.Mode), Revision);
			T.TestEqual(TEXT("No hidden entropy/fallback"), F.Entropy->Calls, Entropy + FailedDraws);
			T.TestEqual(TEXT("Entry unchanged"), Access::Runtime(*F.Mode).GetEntryProviderInvocationCount(), Entry);
			T.TestEqual(TEXT("D12 unchanged"), Access::Runtime(*F.Mode).GetD12ProviderInvocationCount(), D12);
			T.TestEqual(TEXT("Route unchanged"), Access::Runtime(*F.Mode).GetInitialRouteProviderInvocationCount(), Route);
			T.TestEqual(TEXT("Contest unchanged"), Access::Runtime(*F.Mode).GetPostRouteProviderInvocationCount(), Post);
			T.TestEqual(TEXT("Recovery provider boundary"), Access::Runtime(*F.Mode).GetRecoveryProviderInvocationCount(), Recovery + int32(FailedDraws > 0));
			T.TestEqual(TEXT("Rejected input never coordinates"), F.Calls(), Coordinator);
		}
	};
}

IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFMCodexCrossTerminalLifecycle,
	"FMCodex.NetworkPlay.CrossTerminalLifecycle.01.RevealAndAdvance",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
void FFMCodexCrossTerminalLifecycle::GetTests(TArray<FString>& N,TArray<FString>& C) const
{
	for (const TCHAR* A : {TEXT("A"), TEXT("B")}) for (const TCHAR* R : {TEXT("High"), TEXT("Low")})
		for (const TCHAR* G : {TEXT("Goal"), TEXT("NoGoal")}) for (const TCHAR* F : {TEXT("Next"), TEXT("Final")})
			for (bool GK : {false, true})
			{
				if (GK && FString(G) == TEXT("Goal")) { continue; }
				const auto S = FString::Printf(TEXT("%s.%s.%s.%s.%s"), A,R,G,F,GK?TEXT("GK"):TEXT("NoGK")); N.Add(S); C.Add(S);
			}
}
bool FFMCodexCrossTerminalLifecycle::RunTest(const FString& P)
{
	using namespace FMCodexNetworkCrossTerminalTests;
	TArray<FString> Parts; P.ParseIntoArray(Parts,TEXT("."));
	const bool B = Parts[0]==TEXT("B"), High=Parts[1]==TEXT("High"), Goal=Parts[2]==TEXT("Goal"), Final=Parts[3]==TEXT("Final"), GK=Parts[4]==TEXT("GK");
	FTerminalFixture F(B,Final), Local(B,Final);
	if (!TestTrue(TEXT("Complete typed ordinary Cross setup"),F.Prepare(High,Goal,GK) && Local.Prepare(High,Goal,GK))) { return false; }
	auto* Actor = F.Attacker(); const Side ActorSide=Actor->GetOwnerView().ViewerSide;
	const FFrozen Before(F); const auto Selected=Before.State.CurrentAttack.SelectedAction;
	TestEqual(TEXT("Canonical Goal result/history"),Before.State.GoalHistory.Num(),int32(Goal));
	if (Goal) { TestEqual(TEXT("Scorer comes from persisted history"),Before.State.GoalHistory[0].ScorerCardId,Selected.RunnerCardId); }
	auto& Runtime=Access::Runtime(*F.Mode);
	const auto RevealSequence=FFMCodexNetworkCrossTerminalTestAccess::SetReveal(Runtime,0);
	for (auto* PC : {F.A,F.B})
	{
		const auto V=Runtime.BuildClientView(PC->GetOwnerView().ViewerSide,Before.Revision,EFMCodexNetworkBootstrapState::MatchReady);
		TestEqual(TEXT("Persisted authority Goal concealed A"),V.PlayerAScore,0);
		TestEqual(TEXT("Persisted authority Goal concealed B"),V.PlayerBScore,0);
		TestEqual(TEXT("Concealed terminal outcome"),V.CrossTerminal.Outcome,Outcome::None);
		TestTrue(TEXT("Concealed scorer/history"),V.CrossTerminal.Goal.ScorerCardId.IsNone() && V.PublicGoalHistory.IsEmpty());
		TestFalse(TEXT("Normal client cannot advance concealed terminal"),V.bCanAdvance);
	}
	Before.Verify(*this,F);
	FFMCodexNetworkCrossTerminalTestAccess::SetReveal(Runtime,RevealSequence);
	for (auto* PC : {F.A,F.B})
	{
		const auto V=Runtime.BuildClientView(PC->GetOwnerView().ViewerSide,Before.Revision,EFMCodexNetworkBootstrapState::MatchReady);
		TestEqual(TEXT("One coherent terminal outcome"),V.CrossTerminal.Outcome,Goal?Outcome::Goal:Outcome::NoGoal);
		TestEqual(TEXT("A score orientation"),V.PlayerAScore,Goal&&!B?1:0);
		TestEqual(TEXT("B score orientation"),V.PlayerBScore,Goal&&B?1:0);
		TestEqual(TEXT("Exactly public persisted history"),V.PublicGoalHistory.Num(),int32(Goal));
		TestEqual(TEXT("Only current attacker advances"),V.bCanAdvance,PC==Actor);
		if (Goal)
		{
			TestEqual(TEXT("Canonical scorer identity, not inferred Runner"),V.CrossTerminal.Goal.ScorerCardId,Before.State.GoalHistory[0].ScorerCardId);
			TestFalse(TEXT("Chinese roster scorer label"),V.CrossTerminal.Goal.ScorerLabel.IsEmpty());
			TestTrue(TEXT("Current Goal is exact public history entry"),Same(V.CrossTerminal.Goal,V.PublicGoalHistory[0]));
		}
		else { TestTrue(TEXT("No placeholder scorer on NoGoal"),V.CrossTerminal.Goal.ScorerCardId.IsNone() && V.CrossTerminal.Goal.ScorerLabel.IsEmpty()); }
	}
	Before.Verify(*this,F);
	F.Entropy->Word=0; Local.Entropy->Word=0;
	const auto Reference=FMatchPlayEntryDeploymentPlayerIntentPort(Access::Session(*Local.Mode),Access::Coordinator(*Local.Mode)).SubmitPlayerIntent(Advance(Before.State.CurrentAttack.AttackSequence,ActorSide));
	if (!TestTrue(TEXT("Shared authoritative Advance accepts"),Reference.bSuccess)) { return false; }
	if (!TestTrue(TEXT("Existing client state submits Advance"),F.Send(Actor,Kind::AdvanceAfterTerminal))) { return false; }
	const auto After=Access::Session(*F.Mode).GetStateSnapshot();
	TestTrue(TEXT("Local/shared and Network complete State parity"),Same(After,Access::Session(*Local.Mode).GetStateSnapshot()));
	TestFalse(TEXT("Advance clears current attack, next D12 will create it"),After.bHasCurrentAttack);
	TestTrue(TEXT("Current attack transient data fully cleared"),Same(After.CurrentAttack,FMatchPlayCurrentAttackState{}));
	TestTrue(TEXT("Persistent GK usage preserved"),Same(After.GoalkeeperUsageState,Before.State.GoalkeeperUsageState));
	TestEqual(TEXT("Used A once"),After.RuntimeState.PlayerAState.UsedAttackCount,Before.State.RuntimeState.PlayerAState.UsedAttackCount+int32(!B));
	TestEqual(TEXT("Used B once"),After.RuntimeState.PlayerBState.UsedAttackCount,Before.State.RuntimeState.PlayerBState.UsedAttackCount+int32(B));
	TestEqual(TEXT("One Advance revision"),Access::Revision(*F.Mode),Before.Revision+1);
	TestEqual(TEXT("One Coordinator invocation"),F.Calls(),Before.Coordinator+1);
	TestEqual(TEXT("Advance Coordinator has no internal steps"),Reference.CoordinatorResult.Steps.Num(),0);
	TestEqual(TEXT("Canonical stop"),Reference.CoordinatorResult.StopReason,Final?EMatchPlayServerCoordinatorStopReason::MatchEnded:EMatchPlayServerCoordinatorStopReason::WaitingForPlayerIntent);
	TestEqual(TEXT("Recovery runs only on non-final advance"),Runtime.GetRecoveryProviderInvocationCount(),Before.Recovery+int32(!Final));
	TestEqual(TEXT("Two private weighted draws or zero on final"),F.Entropy->Calls,Before.Entropy+(Final?0:2));
	TestEqual(TEXT("No post-route redraw"),Runtime.GetPostRouteProviderInvocationCount(),Before.Post);
	TestEqual(TEXT("No new entry draw until player asks"),Runtime.GetEntryProviderInvocationCount(),Before.Entry);
	for (const auto& Placement : Before.State.CurrentAttack.DeploymentPlacements)
	{
		const bool IsGK=FMatchPlayCardSnapshotAuthorityQuery::FindByPlayerSideAndCardId(Before.State.CardSnapshotAuthority,Placement.PlayerSide,Placement.CardId).Snapshot.bIsGoalkeeper;
		const bool Returned=After.LastRecoveryFact.ReturnedCards.ContainsByPredicate([&](const auto& C){return C.OwnerSide==Placement.PlayerSide && C.CardId==Placement.CardId;});
		TestEqual(TEXT("Consumed card is available only if recovered or GK"),FPlayCardResolver::ValidateCanPlayCard(After.CardUsageState,Placement.PlayerSide,Placement.CardId).bSuccess,IsGK||Returned);
	}
	const int64 Sequence=Before.State.CurrentAttack.AttackSequence;
	for (auto* PC : {F.A,F.B})
	{
		const auto V=PC->GetOwnerView();
		TestFalse(TEXT("Old Advance action disappears"),V.bCanAdvance);
		TestEqual(TEXT("Old current Goal disappears"),V.CrossTerminal.Outcome,Outcome::None);
		TestTrue(TEXT("No stale terminal scorer"),V.CrossTerminal.Goal.ScorerCardId.IsNone());
		TestTrue(TEXT("No stale Cross Skill/branch/rolls"),V.SelectedSkill.Choice.IsEmpty() && V.SelectedBranch.Choice.IsEmpty() && V.CrossContest.AttackD6==0 && V.CrossContest.DefenseD6==0 && V.InitialRoute.D6==0);
		TestEqual(TEXT("Public history persists"),V.PublicGoalHistory.Num(),int32(Goal));
		TestEqual(TEXT("Public A score persists"),V.PlayerAScore,Goal&&!B?1:0);
		TestEqual(TEXT("Public B score persists"),V.PlayerBScore,Goal&&B?1:0);
		TestEqual(TEXT("Match end state"),V.bMatchEnded,Final);
		if (Final)
		{
			TestEqual(TEXT("Final result from safe canonical MatchResult"),V.MatchResult,Goal?(B?EFMCodexNetworkMatchResult::PlayerBWins:EFMCodexNetworkMatchResult::PlayerAWins):EFMCodexNetworkMatchResult::Draw);
			TestEqual(TEXT("No actor after final"),V.ExpectedActingSide,Side::None);
			TestEqual(TEXT("Ended interaction"),V.InteractionState,EFMCodexNetworkClientInteractionState::MatchEnded);
			TestEqual(TEXT("Final advance clears Recovery event"),V.Recovery.SourceAttackSequence,int64(0));
		}
		else
		{
			TestEqual(TEXT("Next sequence correlation"),V.AttackSequence,Sequence+1);
			TestEqual(TEXT("Next canonical attacker"),V.CurrentAttackingSide,B?Side::PlayerA:Side::PlayerB);
			TestEqual(TEXT("Next Full D12 belongs only to next actor"),V.InteractionState,V.ViewerSide==V.CurrentAttackingSide?EFMCodexNetworkClientInteractionState::WaitingForOwnInitialActionPoint:EFMCodexNetworkClientInteractionState::WaitingForOpponentInitialActionPoint);
			TestEqual(TEXT("Safe latest Recovery source"),V.Recovery.SourceAttackSequence,Sequence);
			TestEqual(TEXT("Bounded public Recovery cards"),V.Recovery.Cards.Num(),2);
		}
	}
	if (!Final)
	{
		F.Entropy->Word=5;
		TestTrue(TEXT("Existing next Full D12 transport accepts"),F.Send(F.Attacker(),Kind::RequestInitialActionPointRoll));
		const auto Next=Access::Session(*F.Mode).GetStateSnapshot();
		TestTrue(TEXT("Next D12 creates exactly one current attack"),Next.bHasCurrentAttack);
		TestEqual(TEXT("Created sequence matches previously offered next sequence"),Next.CurrentAttack.AttackSequence,Sequence+1);
		TestEqual(TEXT("Fresh next D12"),Next.CurrentAttack.RawInitialD12,6);
		TestEqual(TEXT("History survives next entry"),Next.GoalHistory.Num(),int32(Goal));
	}
	AddInfo(FString::Printf(TEXT("TerminalLifecycle Actor=%s Actual=%s Goal=%d Final=%d GK=%d Sequence=%lld ScoreA=%d ScoreB=%d RecoveryCalls=%d RecoveryCards=%d HasCurrentAttackAfterAdvance=%d"),
		*Parts[0],*Parts[1],Goal,Final,GK,Sequence,After.RuntimeState.PlayerAState.Score,After.RuntimeState.PlayerBState.Score,Final?0:1,After.LastRecoveryFact.ReturnedCards.Num(),After.bHasCurrentAttack));
	return true;
}

IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFMCodexCrossTerminalSecurity,
	"FMCodex.NetworkPlay.CrossTerminalLifecycle.02.Security",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
void FFMCodexCrossTerminalSecurity::GetTests(TArray<FString>& N,TArray<FString>& C) const
{
	for(const TCHAR* A:{TEXT("A"),TEXT("B")})
		for(const TCHAR* K:{TEXT("WrongSide"),TEXT("BeforeRoute"),TEXT("BeforeAttack"),TEXT("BeforeDefense"),TEXT("Stale"),TEXT("WrongMatch"),TEXT("HugeId"),TEXT("ZeroId"),TEXT("ZeroSequence"),TEXT("Internal"),TEXT("Nonparticipant"),
			TEXT("Ordinary"),TEXT("Goalkeeper"),TEXT("Carrier"),TEXT("Marker"),TEXT("Runner"),TEXT("Helper"),TEXT("Skill"),TEXT("Branch")})
		{const auto S=FString::Printf(TEXT("%s.%s"),A,K);N.Add(S);C.Add(S);}
}
bool FFMCodexCrossTerminalSecurity::RunTest(const FString& P)
{
	using namespace FMCodexNetworkCrossTerminalTests;
	FString A,K;P.Split(TEXT("."),&A,&K);FTerminalFixture F(A==TEXT("B"));
	const int32 Stop=K==TEXT("BeforeRoute")?0:K==TEXT("BeforeAttack")?1:K==TEXT("BeforeDefense")?2:3;
	if(!TestTrue(TEXT("Canonical security checkpoint"),F.Prepare(true,true,false,Stop))) {return false;}
	auto* Correct=F.Attacker();auto* PC=K==TEXT("WrongSide")?F.Defender():Correct;
	auto E=F.AdvanceRequest(PC);const auto NormalId=E.RequestId;Code Expected=Code::AuthorityRejected;
	if(K==TEXT("Stale")) {++E.ExpectedAttackSequence;Expected=Code::StaleAttackSequence;}
	if(K==TEXT("WrongMatch")) {E.MatchInstanceId=FGuid::NewGuid();E.RequestId=MAX_int64;Expected=Code::MatchMismatch;}
	if(K==TEXT("HugeId")) {E.RequestId=MAX_int64;Expected=Code::InvalidPayload;}
	if(K==TEXT("ZeroId")) {E.RequestId=0;Expected=Code::InvalidPayload;}
	if(K==TEXT("ZeroSequence")) {E.ExpectedAttackSequence=0;Expected=Code::InvalidPayload;}
	if(K==TEXT("Internal")) {E.IntentKind=static_cast<Kind>(255);Expected=Code::NotPlayerIntent;}
	if(K==TEXT("Nonparticipant")) {PC=F.World->SpawnActor<AFMCodexNetworkMatchPlayerController>();Expected=Code::NotParticipant;}
	if(K==TEXT("Ordinary")) {E.Deployment.CardId=TEXT("Card");E.Deployment.SlotId=TEXT("Slot");Expected=Code::InvalidPayload;}
	if(K==TEXT("Goalkeeper")) {E.Goalkeeper.SlotId=TEXT("Slot");Expected=Code::InvalidPayload;}
	if(K==TEXT("Carrier")) {E.Carrier.CarrierCardId=TEXT("Card");Expected=Code::InvalidPayload;}
	if(K==TEXT("Marker")) {E.Marker.MarkerCardId=TEXT("Card");Expected=Code::InvalidPayload;}
	if(K==TEXT("Runner")) {E.Runner.RunnerCardId=TEXT("Card");Expected=Code::InvalidPayload;}
	if(K==TEXT("Helper")) {E.Helper.HelperCardId=TEXT("Card");Expected=Code::InvalidPayload;}
	if(K==TEXT("Skill")) {E.Skill.SkillId=TEXT("Skill");Expected=Code::InvalidPayload;}
	if(K==TEXT("Branch")) {E.Branch.Intent=Branch::CrossLow;Expected=Code::InvalidPayload;}
	const FFrozen Before(F);
	const auto Ack=F.Mode->SubmitConnectionPlayerIntent(PC,E);
	TestEqual(TEXT("Exact rejection ACK"),Ack.Code,Expected);
	TestEqual(TEXT("Rejected match correlation preserved"),Ack.MatchInstanceId,E.MatchInstanceId);
	TestEqual(TEXT("Rejected request correlation preserved"),Ack.RequestId,E.RequestId);
	Before.Verify(*this,F);
	if(Stop==3)
	{
		auto Normal=F.AdvanceRequest(Correct);
		if(K==TEXT("WrongMatch")||K==TEXT("HugeId")) {Normal.RequestId=NormalId;}
		TestEqual(TEXT("Next normal request remains usable"),F.Mode->SubmitConnectionPlayerIntent(Correct,Normal).Code,Code::Accepted);
	}
	return true;
}

IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFMCodexCrossTerminalReplay,
	"FMCodex.NetworkPlay.CrossTerminalLifecycle.03.DuplicateAndFreshAdvance",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
void FFMCodexCrossTerminalReplay::GetTests(TArray<FString>& N,TArray<FString>& C) const
{for(const TCHAR* S:{TEXT("A.Next"),TEXT("B.Next"),TEXT("A.Final"),TEXT("B.Final")}){N.Add(S);C.Add(S);}}
bool FFMCodexCrossTerminalReplay::RunTest(const FString& P)
{
	using namespace FMCodexNetworkCrossTerminalTests;
	FTerminalFixture F(P.StartsWith(TEXT("B")),P.EndsWith(TEXT("Final")));
	if(!TestTrue(TEXT("Terminal ready"),F.Prepare(false))) {return false;}
	auto* PC=F.Attacker();auto E=F.AdvanceRequest(PC);
	TestEqual(TEXT("First Advance accepted"),F.Mode->SubmitConnectionPlayerIntent(PC,E).Code,Code::Accepted);
	const FFrozen After(F);
	TestEqual(TEXT("Same accepted ID cannot finalize twice"),F.Mode->SubmitConnectionPlayerIntent(PC,E).Code,Code::DuplicateOrAlreadyResolved);
	After.Verify(*this,F);
	E.RequestId=F.Next(PC)++;
	TestEqual(TEXT("Old sequence cannot progress next lifecycle"),F.Mode->SubmitConnectionPlayerIntent(PC,E).Code,Code::StaleAttackSequence);
	After.Verify(*this,F);
	E.RequestId=F.Next(PC)++;E.ExpectedAttackSequence=PC->GetOwnerView().AttackSequence;
	TestEqual(TEXT("Fresh current-sequence Advance rejects without terminal"),F.Mode->SubmitConnectionPlayerIntent(PC,E).Code,Code::AuthorityRejected);
	After.Verify(*this,F);
	return true;
}

IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFMCodexCrossTerminalRecoveryFailure,
	"FMCodex.NetworkPlay.CrossTerminalLifecycle.04.RecoveryFailure",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
void FFMCodexCrossTerminalRecoveryFailure::GetTests(TArray<FString>& N,TArray<FString>& C) const
{for(const TCHAR* S:{TEXT("A.FirstDraw"),TEXT("B.FirstDraw"),TEXT("A.SecondDraw"),TEXT("B.SecondDraw")}){N.Add(S);C.Add(S);}}
bool FFMCodexCrossTerminalRecoveryFailure::RunTest(const FString& P)
{
	using namespace FMCodexNetworkCrossTerminalTests;FTerminalFixture F(P.StartsWith(TEXT("B")));
	if(!TestTrue(TEXT("Terminal ready"),F.Prepare(true))) {return false;}
	auto* PC=F.Attacker();const FFrozen Before(F);
	const int32 FailedDraws=P.EndsWith(TEXT("SecondDraw"))?2:1;
	F.Entropy->Word=0;F.Entropy->FailOnCall=F.Entropy->Calls+FailedDraws;
	TestFalse(TEXT("Recovery failure rejects Advance"),F.Send(PC,Kind::AdvanceAfterTerminal));
	TestFalse(TEXT("Rejected ACK releases the generic pending"),F.Client(PC).IsPending());
	Before.Verify(*this,F,FailedDraws);
	TestEqual(TEXT("Terminal result remains visible after failed Advance"),PC->GetOwnerView().CrossTerminal.Outcome,Outcome::Goal);
	F.Entropy->FailOnCall=0;
	TestTrue(TEXT("Only explicit retry advances after repair"),F.Send(PC,Kind::AdvanceAfterTerminal));
	TestEqual(TEXT("No lost or doubled opportunity on retry"),Access::Session(*F.Mode).GetStateSnapshot().RuntimeState.PlayerAState.UsedAttackCount+
		Access::Session(*F.Mode).GetStateSnapshot().RuntimeState.PlayerBState.UsedAttackCount,1);
	return true;
}

IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFMCodexCrossTerminalAck,
	"FMCodex.NetworkPlay.CrossTerminalLifecycle.05.AsyncAckView",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
void FFMCodexCrossTerminalAck::GetTests(TArray<FString>& N,TArray<FString>& C) const
{for(const TCHAR* A:{TEXT("A"),TEXT("B")})for(const TCHAR* K:{TEXT("AckFirst"),TEXT("ViewFirst"),TEXT("Reject")}){auto S=FString::Printf(TEXT("%s.%s"),A,K);N.Add(S);C.Add(S);}}
bool FFMCodexCrossTerminalAck::RunTest(const FString& P)
{
	using namespace FMCodexNetworkCrossTerminalTests;FTerminalFixture F(P.StartsWith(TEXT("B")));
	if(!TestTrue(TEXT("Terminal ready"),F.Prepare(true))) {return false;}
	auto* PC=F.Attacker();auto& Client=F.Client(PC);const auto Old=PC->GetOwnerView();Envelope E;
	TestTrue(TEXT("Generic Advance begins"),Client.BeginAdvance(Old,E));
	auto Duplicate=E;TestFalse(TEXT("Duplicate click cannot start another Advance"),Client.BeginAdvance(Old,Duplicate));
	FFMCodexNetworkPlayerIntentAck Unrelated;Unrelated.MatchInstanceId=E.MatchInstanceId;Unrelated.RequestId=E.RequestId+100;Unrelated.Code=Code::Accepted;Unrelated.ViewRevision=Old.ViewRevision+1;
	TestFalse(TEXT("Unrelated ACK ignored"),Client.ObserveAck(Unrelated));
	if(P.EndsWith(TEXT("Reject"))) {E.Branch.Intent=Branch::CrossHigh;}
	const auto Ack=F.Mode->SubmitConnectionPlayerIntent(PC,E);const auto New=PC->GetOwnerView();
	if(P.EndsWith(TEXT("Reject")))
	{
		TestEqual(TEXT("Correlated typed rejection"),Ack.Code,Code::InvalidPayload);
		TestTrue(TEXT("Rejected ACK observed"),Client.ObserveAck(Ack));TestFalse(TEXT("Rejection needs no new View"),Client.IsPending());return true;
	}
	TestEqual(TEXT("Advance accepted"),Ack.Code,Code::Accepted);
	if(P.EndsWith(TEXT("AckFirst")))
	{
		TestTrue(TEXT("Accepted ACK correlates"),Client.ObserveAck(Ack));TestTrue(TEXT("ACK alone stays pending"),Client.IsPending());
		Client.ObserveView(Old);TestTrue(TEXT("Old View cannot release"),Client.IsPending());Client.ObserveView(New);
	}
	else
	{
		Client.ObserveView(New);TestTrue(TEXT("View alone stays pending"),Client.IsPending());TestTrue(TEXT("ACK joins current View"),Client.ObserveAck(Ack));
	}
	TestFalse(TEXT("Both orders clear pending"),Client.IsPending());
	TestFalse(TEXT("Duplicate ACK ignored"),Client.ObserveAck(Ack));
	Client.ObserveView(Old);TestFalse(TEXT("Late old View cannot resurrect pending"),Client.IsPending());
	return true;
}

IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFMCodexCrossTerminalStale,
	"FMCodex.NetworkPlay.CrossTerminalLifecycle.06.ActualLaterTerminal",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
void FFMCodexCrossTerminalStale::GetTests(TArray<FString>& N,TArray<FString>& C) const
{N.Add(TEXT("A"));C.Add(TEXT("A"));N.Add(TEXT("B"));C.Add(TEXT("B"));}
bool FFMCodexCrossTerminalStale::RunTest(const FString& P)
{
	using namespace FMCodexNetworkCrossTerminalTests;FTerminalFixture F(P==TEXT("B"));
	if(!TestTrue(TEXT("Attack N terminal"),F.Prepare(true))) {return false;}
	const int64 OldSequence=F.Attacker()->GetOwnerView().AttackSequence;
	if(!TestTrue(TEXT("Genuine N Advance through shared transport"),F.Send(F.Attacker(),Kind::AdvanceAfterTerminal))) {return false;}
	if(!TestTrue(TEXT("Actual N+1 Cross terminal through existing full prefix"),F.Prepare(false,false))) {return false;}
	auto* Actor=F.Attacker();TestEqual(TEXT("Actual later sequence"),Actor->GetOwnerView().AttackSequence,OldSequence+1);
	auto E=F.AdvanceRequest(Actor);E.ExpectedAttackSequence=OldSequence;const FFrozen Before(F);
	TestEqual(TEXT("Fresh ID stale N cannot advance N+1 terminal"),F.Mode->SubmitConnectionPlayerIntent(Actor,E).Code,Code::StaleAttackSequence);Before.Verify(*this,F);
	auto Normal=F.AdvanceRequest(Actor);
	TestEqual(TEXT("New terminal still advances normally"),F.Mode->SubmitConnectionPlayerIntent(Actor,Normal).Code,Code::Accepted);
	return true;
}

IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFMCodexCrossTerminalDisclosureAuthority,
	"FMCodex.NetworkPlay.CrossTerminalLifecycle.07.DisclosureIsNotGameplay",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
void FFMCodexCrossTerminalDisclosureAuthority::GetTests(TArray<FString>& N,TArray<FString>& C) const
{N.Add(TEXT("A"));C.Add(TEXT("A"));N.Add(TEXT("B"));C.Add(TEXT("B"));}
bool FFMCodexCrossTerminalDisclosureAuthority::RunTest(const FString& P)
{
	using namespace FMCodexNetworkCrossTerminalTests;FTerminalFixture F(P==TEXT("B"));
	if(!TestTrue(TEXT("Authoritative terminal"),F.Prepare(true))) {return false;}
	FFMCodexNetworkCrossTerminalTestAccess::SetReveal(Access::Runtime(*F.Mode),0);
	auto* Actor=F.Attacker();auto E=F.AdvanceRequest(Actor);
	TestEqual(TEXT("Client viewing time is not a second gameplay authority"),F.Mode->SubmitConnectionPlayerIntent(Actor,E).Code,Code::Accepted);
	TestEqual(TEXT("Forged early viewing cannot forge score"),Actor->GetOwnerView().PlayerAScore+Actor->GetOwnerView().PlayerBScore,1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexCrossTerminalProjectionBounds,
	"FMCodex.NetworkPlay.CrossTerminalLifecycle.08.SafeProjectionBounds",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexCrossTerminalProjectionBounds::RunTest(const FString&)
{
	using namespace FMCodexNetworkCrossTerminalTests;FTerminalFixture F;
	if(!TestTrue(TEXT("Goal terminal"),F.Prepare(true))) {return false;}
	FFMCodexLocalMatchViewerDisclosure D;D.bRevealInitialActionPointRoll=true;D.bRevealRouteRoll=true;D.RevealedContestD6Count=2;D.bRevealTerminalOutcome=true;
	const auto Snapshot=Access::Session(*F.Mode).GetStateSnapshot();
	const auto Original=FFMCodexLocalMatchInteractionViewBuilder::BuildForViewer(Snapshot,Access::CallerRules(*F.Mode),Side::PlayerA,D);
	auto Project=[&](const auto& V,Side Viewer=Side::PlayerA){return FFMCodexNetworkClientViewSnapshotFactory::Build(V,F.Mode->GetMatchInstanceId(),9,Viewer,EFMCodexNetworkBootstrapState::MatchReady);};
	auto Safe=Original;const FFrozen Before(F);
	TestEqual(TEXT("Legitimate public goal"),Project(Safe).CrossTerminal.Outcome,Outcome::Goal);
	Safe.GoalHistory.Reset();
	TestEqual(TEXT("Goal without matching public history fails closed"),Project(Safe).CrossTerminal.Outcome,Outcome::None);
	Safe=Original;Safe.GoalHistory[0].ScoringSide=Side::PlayerB;
	TestEqual(TEXT("Mismatched scoring Side fails closed"),Project(Safe).CrossTerminal.Outcome,Outcome::None);
	Safe=Original;Safe.GoalHistory[0].ScorerCardId=NAME_None;
	TestEqual(TEXT("Cross Goal without canonical scorer fails closed"),Project(Safe).CrossTerminal.Outcome,Outcome::None);
	Safe=Original;const auto DuplicateGoal=Safe.GoalHistory[0];Safe.GoalHistory.Add(DuplicateGoal);
	TestTrue(TEXT("Duplicate history cannot be silently presented"),Project(Safe).bGoalHistoryUnavailable);
	TestTrue(TEXT("Duplicate history not partially copied"),Project(Safe).PublicGoalHistory.IsEmpty());
	Safe=Original;Safe.GoalHistory.Reset();
	for(int32 I=1;I<=FFMCodexNetworkClientViewSnapshot::MaxPublicGoals+1;++I)
	{auto G=Original.GoalHistory[0];G.AttackSequence=I;Safe.GoalHistory.Add(G);}
	TestTrue(TEXT("History bound rejects overlong list"),Project(Safe).bGoalHistoryUnavailable);
	TestTrue(TEXT("No unbounded replication"),Project(Safe).PublicGoalHistory.IsEmpty());
	Safe=Original;Safe.ResolutionFacts.Decisions.RemoveAll([](const auto& V){return V.DecisionId==FName(TEXT("Cross.High.Outcome"));});
	TestEqual(TEXT("No fabricated result without safe decision"),Project(Safe).CrossTerminal.Outcome,Outcome::None);
	Safe=Original;
	const auto* Decision=Safe.ResolutionFacts.Decisions.FindByPredicate([](const auto& V){return V.DecisionId==FName(TEXT("Cross.High.Outcome"));});
	if(!TestNotNull(TEXT("Canonical decision identity"),Decision)) {return false;}
	const auto DuplicateDecision=*Decision;Safe.ResolutionFacts.Decisions.Add(DuplicateDecision);
	TestEqual(TEXT("Ambiguous terminal facts fail closed"),Project(Safe).CrossTerminal.Outcome,Outcome::None);
	TestFalse(TEXT("No nonowner Advance"),Project(Original,Side::PlayerB).bCanAdvance);
	TestFalse(TEXT("No invalid-viewer Advance"),Project(Original,Side::None).bCanAdvance);
	Safe=Original;Safe.bHasRecoveryFact=true;Safe.RecoverySourceAttackSequence=1;
	FFMCodexLocalMatchRecoveryPresentationEntry Entry;Entry.OwnerSide=Side::PlayerA;Entry.CardId=TEXT("Card");Entry.PlayerDisplayName=TEXT("球员");
	Safe.RecoveryPresentationEntries={Entry,Entry};
	TestEqual(TEXT("Duplicate Recovery identities fail closed"),Project(Safe).Recovery.SourceAttackSequence,int64(0));
	Safe.RecoveryPresentationEntries.Add(Entry);
	TestEqual(TEXT("Overlong Recovery does not replicate"),Project(Safe).Recovery.SourceAttackSequence,int64(0));
	Before.Verify(*this,F);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexCrossTerminalWire,
	"FMCodex.NetworkPlay.CrossTerminalLifecycle.09.ClosedWireAndSharedNamespace",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexCrossTerminalWire::RunTest(const FString&)
{
	using namespace FMCodexNetworkCrossTerminalTests;
	for(int32 K=1;K<=18;++K) for(int32 Mask=0;Mask<256;++Mask)
	{
		Envelope E;E.IntentKind=static_cast<Kind>(K);
		if(Mask&1){E.Deployment.CardId=TEXT("Card");E.Deployment.SlotId=TEXT("Slot");}
		if(Mask&2){E.Goalkeeper.SlotId=TEXT("Slot");}
		if(Mask&4){E.Carrier.CarrierCardId=TEXT("Card");}
		if(Mask&8){E.Marker.MarkerCardId=TEXT("Card");}
		if(Mask&16){E.Runner.RunnerCardId=TEXT("Card");}
		if(Mask&32){E.Helper.HelperCardId=TEXT("Card");}
		if(Mask&64){E.Skill.SkillId=TEXT("Skill");}
		if(Mask&128){E.Branch.Intent=Branch::CrossHigh;}
		const int32 Expected=K==2?1:K==3?2:K>=5&&K<=10?1<<(K-3):0;
		TestEqual(TEXT("Eighteen closed tags x all payload shapes"),E.ValidatePayloadShape(),Mask==Expected?Code::None:Code::InvalidPayload);
	}
	FFMCodexNetworkIntentLedger Ledger;const FGuid Match=FGuid::NewGuid();Envelope E;E.MatchInstanceId=Match;E.ExpectedAttackSequence=1;
	for(int32 K=1;K<=18;++K){E.IntentKind=static_cast<Kind>(K);E.RequestId=K;TestEqual(TEXT("One shared namespace"),Ledger.Check(Match,E),Code::None);TestTrue(TEXT("Shared high-water consumption"),Ledger.Consume(Match,E));}
	E.RequestId=18+1025;TestEqual(TEXT("1025 forward delta rejected"),Ledger.Check(Match,E),Code::InvalidPayload);
	E.RequestId=18+1024;TestEqual(TEXT("1024 forward delta allowed"),Ledger.Check(Match,E),Code::None);
	E.IntentKind=Kind::AdvanceAfterTerminal;TArray<uint8> Bytes;
	{FMemoryWriter W(Bytes);FObjectAndNameAsStringProxyArchive A(W,false);FFMCodexNetworkPlayerIntentEnvelope::StaticStruct()->SerializeItem(A,&E,nullptr);}
	Envelope Restored;
	{FMemoryReader R(Bytes);FObjectAndNameAsStringProxyArchive A(R,false);FFMCodexNetworkPlayerIntentEnvelope::StaticStruct()->SerializeItem(A,&Restored,nullptr);}
	TestTrue(TEXT("Actual Advance wire serialization roundtrip"),Same(E,Restored));
	FTerminalFixture F;if(!TestTrue(TEXT("Terminal fixture"),F.Prepare(true))) {return false;}
	const FFrozen Before(F);
	FMatchPlayAuthoritativeSubmitSkillRequest WrongPayload;
	const auto Wrong=FMatchPlayEntryDeploymentPlayerIntentPort(Access::Session(*F.Mode),Access::Coordinator(*F.Mode)).SubmitPlayerIntent(FMatchPlayPlayerIntent::Create(Command::AdvanceAfterTerminal,WrongPayload));
	TestFalse(TEXT("Canonical port rejects wrong variant"),Wrong.bSuccess);Before.Verify(*this,F);
	return true;
}

IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFMCodexCrossTerminalMilestone,
	"FMCodex.NetworkPlay.CrossTerminalLifecycle.10.CanonicalMilestone",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
void FFMCodexCrossTerminalMilestone::GetTests(TArray<FString>& N,TArray<FString>& C) const
{for(const TCHAR* S:{TEXT("Goal"),TEXT("NoGoal"),TEXT("FinalGoal"),TEXT("FinalNoGoal")}){N.Add(S);C.Add(S);}}
bool FFMCodexCrossTerminalMilestone::RunTest(const FString& P)
{
	using namespace FMCodexNetworkCrossTerminalTests;FFixture F;
	const bool Goal=P==TEXT("Goal")||P==TEXT("FinalGoal"),Final=P.StartsWith(TEXT("Final"));
	F.Entropy=FFMCodexNetworkCrossTerminalTestAccess::Milestone(*F.Mode,Goal,Final);
	if(!TestNotNull(TEXT("Canonical server milestone ready"),F.Entropy)) {return false;}
	auto* Actor=F.Attacker();const auto V=Actor->GetOwnerView();
	TestEqual(TEXT("Goal fixture Host A; NoGoal fixture Remote B"),V.ViewerSide,Goal?Side::PlayerA:Side::PlayerB);
	TestEqual(TEXT("Fixture stops before actual Attack RPC"),V.CrossContest.AttackD6,0);
	TestEqual(TEXT("Short final fixture has one opportunity per side"),V.PlayerAMaxAttackOpportunities,Final?1:3);
	TestEqual(TEXT("Canonical final prelude sequence"),V.AttackSequence,int64(Final?2:1));
	TestTrue(TEXT("Actual Attack transport"),F.Send(Actor,Roll(Goal,true)));
	TestTrue(TEXT("Actual Defense transport"),F.Send(F.Defender(),Roll(Goal,false)));
	TestEqual(TEXT("Desired result is produced by canonical Formula"),Actor->GetOwnerView().CrossTerminal.Outcome,Goal?Outcome::Goal:Outcome::NoGoal);
	TestTrue(TEXT("Actual Advance transport"),F.Send(Actor,Kind::AdvanceAfterTerminal));
	TestEqual(TEXT("Expected final lifecycle"),Actor->GetOwnerView().bMatchEnded,Final);
	return true;
}
#endif
