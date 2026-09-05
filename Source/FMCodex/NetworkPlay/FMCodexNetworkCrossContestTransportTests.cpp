#if WITH_DEV_AUTOMATION_TESTS
#include "FMCodexNetworkInitialRouteTestFixture.h"

struct FFMCodexNetworkCrossContestTestAccess
{
	static void WithholdContest(FFMCodexNetworkMatchRuntime& Runtime)
	{
		Runtime.DisclosedCrossContestAttackSequence = 0;
	}
};
namespace FMCodexNetworkCrossContestTests
{
	using namespace FMCodexNetworkInitialRouteTests;
	using Action = EFMCodexNetworkCrossContestAction;
	using Route = EMatchPlayCrossActualBranch;
	using Purpose = EMatchPlayCurrentAttackPostRouteRollPurpose;
	using Command = EMatchPlayAuthoritativeCommandKind;
	Kind RollKind(bool High, bool Attack)
	{
		return High ? (Attack ? Kind::CrossHighAttackRoll : Kind::CrossHighDefenseRoll)
			: (Attack ? Kind::CrossLowAttackRoll : Kind::CrossLowDefenseRoll);
	}
	Action RollAction(bool High, bool Attack)
	{
		return High ? (Attack ? Action::CrossHighAttackRoll : Action::CrossHighDefenseRoll)
			: (Attack ? Action::CrossLowAttackRoll : Action::CrossLowDefenseRoll);
	}
	FMatchPlayPlayerIntent Canonical(Kind K, int64 Sequence, Side Player)
	{
		auto Make = [&](auto Request, Command C)
		{
			Request.AttackSequence = Sequence; Request.RequestingSide = Player;
			return FMatchPlayPlayerIntent::Create(C, Request);
		};
		switch (K)
		{
		case Kind::CrossHighAttackRoll: return Make(FMatchPlayAuthoritativeResolveCrossHighAttackRollRequest{}, Command::ResolveCrossHighAttackRoll);
		case Kind::CrossHighDefenseRoll: return Make(FMatchPlayAuthoritativeResolveCrossHighDefenseRollRequest{}, Command::ResolveCrossHighDefenseRoll);
		case Kind::CrossLowAttackRoll: return Make(FMatchPlayAuthoritativeResolveCrossLowAttackRollRequest{}, Command::ResolveCrossLowAttackRoll);
		default: return Make(FMatchPlayAuthoritativeResolveCrossLowDefenseRollRequest{}, Command::ResolveCrossLowDefenseRoll);
		}
	}
	struct FCrossFixture : FFixture
	{
		using FFixture::FFixture;
		bool Prepare(bool High, bool Flip = false)
		{
			return ReachRoute(ESkillRuleType::Cross, Flip ? 5 : 1, High != Flip)
				&& Send(Attacker(), Kind::CrossInitialRouteRoll);
		}
		FFMCodexNetworkPlayerIntentAck Roll(bool High, bool Attack, int32 D6)
		{
			Entropy->Word = D6 - 1; RequestedKind = RollKind(High, Attack);
			auto* PC = Attack ? Attacker() : Defender();
			return Mode->SubmitConnectionPlayerIntent(PC, Request(PC));
		}
		FFMCodexLocalMatchInteractionView Safe(Side Player, int32 Count, bool Terminal = false, bool RouteReveal = true, bool EntryReveal = true)
		{
			FFMCodexLocalMatchViewerDisclosure D;
			D.bRevealInitialActionPointRoll = EntryReveal; D.bRevealRouteRoll = RouteReveal;
			D.RevealedContestD6Count = Count; D.bRevealTerminalOutcome = Terminal;
			return FFMCodexLocalMatchInteractionViewBuilder::BuildForViewer(
				Access::Session(*Mode).GetStateSnapshot(), Access::CallerRules(*Mode), Player, D);
		}
	};
	struct FFrozen
	{
		FMatchPlayState State;
		int32 Revision, Entropy, Post, Initial, Entry, D12, Coordinator;
		explicit FFrozen(FCrossFixture& F)
			: State(Access::Session(*F.Mode).GetStateSnapshot()), Revision(Access::Revision(*F.Mode)),
			Entropy(F.Entropy->Calls), Post(Access::Runtime(*F.Mode).GetPostRouteProviderInvocationCount()),
			Initial(Access::Runtime(*F.Mode).GetInitialRouteProviderInvocationCount()),
			Entry(Access::Runtime(*F.Mode).GetEntryProviderInvocationCount()), D12(Access::Runtime(*F.Mode).GetD12ProviderInvocationCount()), Coordinator(F.Calls()) {}
		void Verify(FAutomationTestBase& T, FCrossFixture& F, bool Failure = false) const
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
	FFMCodexNetworkClientViewSnapshot Project(const FFMCodexLocalMatchInteractionView& Safe, Side Viewer)
	{
		return FFMCodexNetworkClientViewSnapshotFactory::Build(Safe, FGuid::NewGuid(), 1, Viewer, EFMCodexNetworkBootstrapState::MatchReady);
	}
}

IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFMCodexCrossPairs,
	"FMCodex.NetworkPlay.CrossContestTransport.01.AllPairsBothActorsAndFlips",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
void FFMCodexCrossPairs::GetTests(TArray<FString>& N, TArray<FString>& C) const
{
	for (const TCHAR* Player : {TEXT("A"),TEXT("B")})
		for (const TCHAR* Route : {TEXT("High"),TEXT("Low")})
			for (const TCHAR* Flip : {TEXT("Keep"),TEXT("Flip")})
				for (int32 A=1;A<=6;++A) for(int32 D=1;D<=6;++D)
				{const auto S=FString::Printf(TEXT("%s.%s.%s.%d.%d"),Player,Route,Flip,A,D);N.Add(S);C.Add(S);}
}
bool FFMCodexCrossPairs::RunTest(const FString& P)
{
	using namespace FMCodexNetworkCrossContestTests;
	TArray<FString> Parts;P.ParseIntoArray(Parts,TEXT("."));
	const bool BFirst=Parts[0]==TEXT("B"),High=Parts[1]==TEXT("High"),Flip=Parts[2]==TEXT("Flip");
	const int32 AD6=FCString::Atoi(*Parts[3]),DD6=FCString::Atoi(*Parts[4]);
	FCrossFixture F(BFirst), Local(BFirst);
	if(!TestTrue(TEXT("Two canonical Cross fixtures"),F.Prepare(High,Flip) && Local.Prepare(High,Flip))) {return false;}
	const auto BeforeState=Access::Session(*F.Mode).GetStateSnapshot();
	const Side Attacker=F.Attacker()->GetOwnerView().ViewerSide,Defender=F.Defender()->GetOwnerView().ViewerSide;
	const auto Selected=BeforeState.CurrentAttack.SelectedAction;
	for(bool IsAttack:{true,false})
	{
		const int32 D6=IsAttack?AD6:DD6,ExpectedCount=IsAttack?1:2;
		const auto K=RollKind(High,IsAttack);auto* PC=IsAttack?F.Attacker():F.Defender();
		const auto Before=PC->GetOwnerView();const FFrozen Frozen(F);
		TestEqual(TEXT("Owner receives exact actual-route action"),Before.CrossContestAction,RollAction(High,IsAttack));
		TestEqual(TEXT("Other viewer receives no roll control"),(IsAttack?F.Defender():F.Attacker())->GetOwnerView().CrossContestAction,Action::None);
		const auto Intent=Canonical(K,Before.AttackSequence,Before.ViewerSide);
		TestEqual(TEXT("Structural PlayerIntent classification"),FMatchPlayAuthoritativeCommandClassification::OriginOf(Intent.CommandKind),EMatchPlayAuthoritativeCommandOrigin::PlayerIntent);
		Local.Entropy->Word=D6-1;
		const auto L=Access::Runtime(*Local.Mode).SubmitPlayerIntent(Intent);
		if(!TestTrue(TEXT("Shared port accepts"),L.bSuccess)) {return false;}
		const auto Ack=F.Roll(High,IsAttack,D6);
		if(!TestEqual(TEXT("Network empty typed request accepted"),Ack.Code,Code::Accepted)) {return false;}
		const auto S=Access::Session(*F.Mode).GetStateSnapshot();const auto& A=S.CurrentAttack;const auto& R=A.ResolutionSession;
		TestTrue(TEXT("Complete shared-port/network State equivalence"),SameState(S,Access::Session(*Local.Mode).GetStateSnapshot()));
		TestEqual(TEXT("One secure-provider entropy request for supplied uniform word"),F.Entropy->Calls,Frozen.Entropy+1);
		TestEqual(TEXT("Exactly one post-route provider invocation"),Access::Runtime(*F.Mode).GetPostRouteProviderInvocationCount(),Frozen.Post+1);
		TestEqual(TEXT("No route redraw"),Access::Runtime(*F.Mode).GetInitialRouteProviderInvocationCount(),Frozen.Initial);
		TestEqual(TEXT("No entry draw"),Access::Runtime(*F.Mode).GetEntryProviderInvocationCount(),Frozen.Entry);
		TestEqual(TEXT("Coordinator exactly once"),F.Calls(),Frozen.Coordinator+1);
		TestEqual(TEXT("One stable revision"),Ack.ViewRevision,Frozen.Revision+1);
		TestEqual(TEXT("Actual route frozen"),R.ActualBranch.Cross,High?Route::High:Route::Low);
		TestEqual(TEXT("Skill frozen"),A.SelectedAction.SkillId,Selected.SkillId);
		TestEqual(TEXT("Selected branch frozen independently"),A.SelectedAction.ElectiveBranchIntent,Selected.ElectiveBranchIntent);
		TestEqual(TEXT("Phase remains Resolution"),A.Phase,EMatchPlayCurrentAttackPhase::Resolution);
		TestEqual(TEXT("Selection remains ReadyForResolution"),A.SelectionStage,EMatchPlayCurrentAttackSelectionStage::ReadyForResolution);
		TestEqual(TEXT("Session stays RouteResolved"),R.Stage,EMatchPlayCurrentAttackResolutionStage::RouteResolved);
		TestEqual(TEXT("PrimaryBranch roll phase"),R.PostRouteRollProgress.Phase,EMatchPlayCurrentAttackPostRouteRollPhase::PrimaryBranch);
		if(!TestEqual(TEXT("Exact canonical roll prefix size"),R.PostRouteRollProgress.RollRecords.Num(),ExpectedCount)) {return false;}
		TestEqual(TEXT("Attack D6 committed once"),R.PostRouteRollProgress.RollRecords[0].RawD6,AD6);
		TestEqual(TEXT("First purpose"),R.PostRouteRollProgress.RollRecords[0].Purpose,Purpose::PrimaryAttack);
		TestEqual(TEXT("Internal step count"),L.CoordinatorResult.Steps.Num(),IsAttack?0:1);
		TestEqual(TEXT("Correct stable stop"),L.CoordinatorResult.StopReason,IsAttack?EMatchPlayServerCoordinatorStopReason::WaitingForPlayerIntent:EMatchPlayServerCoordinatorStopReason::TerminalPendingAdvance);
		TestEqual(TEXT("Only defense terminalizes"),A.LifecycleState,IsAttack?EMatchPlayCurrentAttackLifecycleState::Active:EMatchPlayCurrentAttackLifecycleState::TerminalPendingAdvance);
		if(!IsAttack)
		{
			TestEqual(TEXT("Only ApplyCrossTerminalResolution continues internally"),L.CoordinatorResult.Steps[0].CommandKind,Command::ApplyCrossTerminalResolution);
			TestEqual(TEXT("Defense D6 committed once"),R.PostRouteRollProgress.RollRecords[1].RawD6,DD6);
			TestEqual(TEXT("Second purpose"),R.PostRouteRollProgress.RollRecords[1].Purpose,Purpose::PrimaryDefense);
		}
		for(auto* Viewer:{F.A,F.B})
		{
			const auto V=Viewer->GetOwnerView();
			TestEqual(TEXT("Public Attack D6"),V.CrossContest.AttackD6,AD6);
			TestEqual(TEXT("Defense absent until actual roll"),V.CrossContest.DefenseD6,IsAttack?0:DD6);
			TestEqual(TEXT("Only completed pair reports Formula completion"),V.CrossContest.bFormulaResolved,!IsAttack);
			TestEqual(TEXT("Precise handoff/terminal wait"),V.EntryWait,IsAttack?EFMCodexNetworkEntryWait::CrossDefenseRoll:EFMCodexNetworkEntryWait::TerminalPendingAdvance);
			TestEqual(TEXT("Next actor from canonical safe view"),V.ExpectedActingSide,IsAttack?Defender:Attacker);
			TestEqual(TEXT("No attack reroll or premature next control"),V.CrossContestAction,IsAttack && V.ViewerSide==Defender?RollAction(High,false):Action::None);
			TestEqual(TEXT("Current goal score remains withheld A"),V.PlayerAScore,BeforeState.RuntimeState.PlayerAState.Score);
			TestEqual(TEXT("Current goal score remains withheld B"),V.PlayerBScore,BeforeState.RuntimeState.PlayerBState.Score);
			const auto Safe=F.Safe(V.ViewerSide,ExpectedCount);
			TestEqual(TEXT("Network exactly follows safe score"),V.PlayerAScore,Safe.PlayerAScore);
			TestEqual(TEXT("Network exactly follows safe score"),V.PlayerBScore,Safe.PlayerBScore);
			TestTrue(TEXT("Current scorer/GoalHistory withheld"),Safe.GoalHistory.IsEmpty());
			for(const auto& Decision:Safe.ResolutionFacts.Decisions)
				TestFalse(TEXT("No hidden Goal decision"),Decision.bResolved && Decision.Outcome==EMatchPlayResolutionDecisionOutcome::Goal);
		}
		TestEqual(TEXT("No early opportunity consumption A"),S.RuntimeState.PlayerAState.UsedAttackCount,BeforeState.RuntimeState.PlayerAState.UsedAttackCount);
		TestEqual(TEXT("No early opportunity consumption B"),S.RuntimeState.PlayerBState.UsedAttackCount,BeforeState.RuntimeState.PlayerBState.UsedAttackCount);
		TestEqual(TEXT("No attacker handoff until explicit advance"),S.RuntimeState.CurrentAttackingPlayer,Attacker);
	}
	const auto S=Access::Session(*F.Mode).GetStateSnapshot();
	const auto Full=F.Safe(Attacker,2,true);
	if(!TestEqual(TEXT("One canonical terminal Formula"),Full.ResolutionFacts.FormulaContests.Num(),1)) {return false;}
	const auto& Contest=Full.ResolutionFacts.FormulaContests[0];
	TestTrue(TEXT("Full authoritative Formula complete"),Contest.bHasResolvedFormula && Contest.ResolvedResult.bAttackEnded && !Contest.ResolvedResult.bContinueResolution);
	const bool Goal=Contest.ResolvedResult.bIsGoal;
	TestEqual(TEXT("Cross finishing goal means attacker winner"),Goal,Contest.ResolvedResult.Winner==EFormulaWinner::Attacker);
	TestEqual(TEXT("Score is committed only on authoritative Goal"),S.RuntimeState.PlayerAState.Score+S.RuntimeState.PlayerBState.Score,int32(Goal));
	TestEqual(TEXT("Goal history recorded exactly once"),S.GoalHistory.Num(),int32(Goal));
	if(Goal) {TestEqual(TEXT("Scorer is canonical Runner"),S.GoalHistory[0].ScorerCardId,Selected.RunnerCardId);}
	AddInfo(FString::Printf(TEXT("CrossPair Actor=%s Actual=%s Flip=%d AttackD6=%d DefenseD6=%d AttackTotal=%.2f DefenseTotal=%.2f Winner=%d Goal=%d Terminal=1 PublicScoreA=0 PublicScoreB=0 InternalSteps=1 AdditionalRNG=0"),
		*Parts[0],*Parts[1],Flip,AD6,DD6,Contest.AttackRow.FinalValue,Contest.DefenseRow.FinalValue,static_cast<int32>(Contest.ResolvedResult.Winner),Goal));
	return true;
}

IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFMCodexCrossSecurity,
	"FMCodex.NetworkPlay.CrossContestTransport.02.Security",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
void FFMCodexCrossSecurity::GetTests(TArray<FString>& N,TArray<FString>& C) const
{
	for(const TCHAR* K:{TEXT("HighAttack"),TEXT("HighDefense"),TEXT("LowAttack"),TEXT("LowDefense")})
		for(const TCHAR* Case:{TEXT("WrongSide"),TEXT("WrongRoute"),TEXT("BeforeSkill"),TEXT("BeforeBranch"),TEXT("BeforeRoute"),
			TEXT("WrongMatch"),TEXT("Stale"),TEXT("HugeId"),TEXT("ZeroId"),TEXT("ZeroSequence"),TEXT("Internal"),TEXT("Nonparticipant"),
			TEXT("Ordinary"),TEXT("Goalkeeper"),TEXT("Carrier"),TEXT("Marker"),TEXT("Runner"),TEXT("Helper"),TEXT("Skill"),TEXT("Branch")})
		{const auto S=FString::Printf(TEXT("%s.%s"),K,Case);N.Add(S);C.Add(S);}
}
bool FFMCodexCrossSecurity::RunTest(const FString& P)
{
	using namespace FMCodexNetworkCrossContestTests;FCrossFixture F;
	const bool High=P.StartsWith(TEXT("High")),Attack=P.Contains(TEXT("Attack"));
	FString K,Case;P.Split(TEXT("."),&K,&Case);bool Ready=false;
	if(Case==TEXT("BeforeSkill")) {Ready=F.ReachSkill();}
	else if(Case==TEXT("BeforeBranch")) {Ready=F.ReachRoute(ESkillRuleType::Cross,1,true,true);}
	else if(Case==TEXT("BeforeRoute")) {Ready=F.ReachRoute(ESkillRuleType::Cross);}
	else {Ready=F.Prepare(High,true);if(Ready && !Attack) {Ready=F.Roll(High,true,3).Code==Code::Accepted;}}
	if(!TestTrue(TEXT("Canonical requested security checkpoint"),Ready)) {return false;}
	auto* PC=Attack?F.Attacker():F.Defender();
	F.RequestedKind=RollKind(High,Attack);auto E=F.Request(PC);const int64 NormalId=E.RequestId;Code Expected=Code::AuthorityRejected;
	if(Case==TEXT("WrongSide")) {PC=Attack?F.Defender():F.Attacker();E.RequestId=F.Next(PC)++;}
	if(Case==TEXT("WrongRoute")) {E.IntentKind=RollKind(!High,Attack);}
	if(Case==TEXT("WrongMatch")) {E.MatchInstanceId=FGuid::NewGuid();E.RequestId=MAX_int64;Expected=Code::MatchMismatch;}
	if(Case==TEXT("Stale")) {++E.ExpectedAttackSequence;Expected=Code::StaleAttackSequence;}
	if(Case==TEXT("HugeId")) {E.RequestId=MAX_int64;Expected=Code::InvalidPayload;}
	if(Case==TEXT("ZeroId")) {E.RequestId=0;Expected=Code::InvalidPayload;}
	if(Case==TEXT("ZeroSequence")) {E.ExpectedAttackSequence=0;Expected=Code::InvalidPayload;}
	if(Case==TEXT("Internal")) {E.IntentKind=static_cast<Kind>(255);Expected=Code::NotPlayerIntent;}
	if(Case==TEXT("Nonparticipant")) {PC=F.World->SpawnActor<AFMCodexNetworkMatchPlayerController>();Expected=Code::NotParticipant;}
	if(Case==TEXT("Ordinary")) {E.Deployment.CardId=TEXT("Card");E.Deployment.SlotId=TEXT("Slot");Expected=Code::InvalidPayload;}
	if(Case==TEXT("Goalkeeper")) {E.Goalkeeper.SlotId=TEXT("Slot");Expected=Code::InvalidPayload;}
	if(Case==TEXT("Carrier")) {E.Carrier.CarrierCardId=TEXT("Card");Expected=Code::InvalidPayload;}
	if(Case==TEXT("Marker")) {E.Marker.MarkerCardId=TEXT("Card");Expected=Code::InvalidPayload;}
	if(Case==TEXT("Runner")) {E.Runner.RunnerCardId=TEXT("Card");Expected=Code::InvalidPayload;}
	if(Case==TEXT("Helper")) {E.Helper.HelperCardId=TEXT("Card");Expected=Code::InvalidPayload;}
	if(Case==TEXT("Skill")) {E.Skill.SkillId=TEXT("Skill");Expected=Code::InvalidPayload;}
	if(Case==TEXT("Branch")) {E.Branch.Intent=Branch::CrossHigh;Expected=Code::InvalidPayload;}
	const FFrozen Before(F);TestEqual(TEXT("Exact secure rejection"),F.Mode->SubmitConnectionPlayerIntent(PC,E).Code,Expected);Before.Verify(*this,F);
	if(!Case.StartsWith(TEXT("Before")))
	{
		auto* Correct=Attack?F.Attacker():F.Defender();auto Normal=F.Request(Correct);
		if(Case==TEXT("WrongMatch") || Case==TEXT("HugeId")) {Normal.RequestId=NormalId;}
		TestEqual(TEXT("Next valid input remains usable, no ledger poisoning"),F.Mode->SubmitConnectionPlayerIntent(Correct,Normal).Code,Code::Accepted);
	}
	return true;
}

IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFMCodexCrossReplay,
	"FMCodex.NetworkPlay.CrossContestTransport.03.OrderAndRerolls",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
void FFMCodexCrossReplay::GetTests(TArray<FString>& N,TArray<FString>& C) const
{for(const TCHAR* K:{TEXT("AHigh"),TEXT("ALow"),TEXT("BHigh"),TEXT("BLow")}) {N.Add(K);C.Add(K);}}
bool FFMCodexCrossReplay::RunTest(const FString& P)
{
	using namespace FMCodexNetworkCrossContestTests;FCrossFixture F(P.StartsWith(TEXT("B")));const bool High=P.Contains(TEXT("High"));
	if(!TestTrue(TEXT("Actual-route flip fixture"),F.Prepare(High,true))) {return false;}
	F.RequestedKind=RollKind(High,false);auto Early=F.Request(F.Defender());const FFrozen Before(F);
	TestEqual(TEXT("Defense cannot precede Attack"),F.Mode->SubmitConnectionPlayerIntent(F.Defender(),Early).Code,Code::AuthorityRejected);Before.Verify(*this,F);
	for(bool Attack:{true,false})
	{
		auto* PC=Attack?F.Attacker():F.Defender();F.RequestedKind=RollKind(High,Attack);auto E=F.Request(PC);
		TestEqual(TEXT("First accepted"),F.Mode->SubmitConnectionPlayerIntent(PC,E).Code,Code::Accepted);
		const FFrozen Frozen(F);
		TestEqual(TEXT("Same ID cannot redraw or reapply terminal"),F.Mode->SubmitConnectionPlayerIntent(PC,E).Code,Code::DuplicateOrAlreadyResolved);Frozen.Verify(*this,F);
		for(bool RequestHigh:{true,false})
		{
			E.RequestId=F.Next(PC)++;E.IntentKind=RollKind(RequestHigh,Attack);
			TestEqual(TEXT("Fresh same/opposite-route reroll rejected"),F.Mode->SubmitConnectionPlayerIntent(PC,E).Code,Code::AuthorityRejected);Frozen.Verify(*this,F);
		}
	}
	const FFrozen Terminal(F);
	for(bool HighRequest:{true,false}) for(bool AttackRequest:{true,false})
	{
		auto* PC=AttackRequest?F.Attacker():F.Defender();F.RequestedKind=RollKind(HighRequest,AttackRequest);
		TestEqual(TEXT("All four unavailable after terminal"),F.Mode->SubmitConnectionPlayerIntent(PC,F.Request(PC)).Code,Code::AuthorityRejected);Terminal.Verify(*this,F);
	}
	return true;
}

IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFMCodexCrossProviderFailure,
	"FMCodex.NetworkPlay.CrossContestTransport.04.ProviderFailure",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
void FFMCodexCrossProviderFailure::GetTests(TArray<FString>& N,TArray<FString>& C) const
{for(const TCHAR* K:{TEXT("HighAttack"),TEXT("HighDefense"),TEXT("LowAttack"),TEXT("LowDefense")}) {N.Add(K);C.Add(K);}}
bool FFMCodexCrossProviderFailure::RunTest(const FString& P)
{
	using namespace FMCodexNetworkCrossContestTests;FCrossFixture F;const bool High=P.StartsWith(TEXT("High")),Attack=P.Contains(TEXT("Attack"));
	if(!TestTrue(TEXT("Route ready"),F.Prepare(High))) {return false;}
	if(!Attack && !TestEqual(TEXT("Prior attack"),F.Roll(High,true,3).Code,Code::Accepted)) {return false;}
	auto* PC=Attack?F.Attacker():F.Defender();Envelope E;
	auto& Existing=F.Client(PC);Existing.ObserveView(PC->GetOwnerView());
	TestTrue(TEXT("Existing generic state begins"),Existing.BeginCrossContest(PC->GetOwnerView(),RollKind(High,Attack),E));
	F.Next(PC)=E.RequestId+1;
	const FFrozen Before(F);F.Entropy->bFail=true;
	const auto Ack=F.Mode->SubmitConnectionPlayerIntent(PC,E);
	TestEqual(TEXT("Failure is authority rejection"),Ack.Code,Code::AuthorityRejected);Before.Verify(*this,F,true);
	TestTrue(TEXT("Rejected ACK correlates"),Existing.ObserveAck(Ack));TestFalse(TEXT("Failure releases pending"),Existing.IsPending());
	Existing.ObserveView(PC->GetOwnerView());Before.Verify(*this,F,true);
	F.Entropy->bFail=false;
	TestTrue(TEXT("Explicit subsequent input may retry after repair"),F.Send(PC,RollKind(High,Attack)));
	return true;
}

IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFMCodexCrossOtherFamilies,
	"FMCodex.NetworkPlay.CrossContestTransport.05.WrongTacticalFamily",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
void FFMCodexCrossOtherFamilies::GetTests(TArray<FString>& N,TArray<FString>& C) const
{
	for(const TCHAR* Player:{TEXT("A"),TEXT("B")})
		for(const TCHAR* F:{TEXT("PassControl"),TEXT("ThroughBall"),TEXT("LongShot"),TEXT("CutInsideShot")})
			for(int32 K=14;K<=17;++K) {const auto S=FString::Printf(TEXT("%s.%s.%d"),Player,F,K);N.Add(S);C.Add(S);}
}
bool FFMCodexCrossOtherFamilies::RunTest(const FString& P)
{
	using namespace FMCodexNetworkCrossContestTests;TArray<FString> Parts;P.ParseIntoArray(Parts,TEXT("."));
	FCrossFixture F(Parts[0]==TEXT("B"));
	const auto Type=Parts[1]==TEXT("PassControl")?ESkillRuleType::PassControl:Parts[1]==TEXT("ThroughBall")?ESkillRuleType::ThroughBall
		:Parts[1]==TEXT("LongShot")?ESkillRuleType::LongShot:ESkillRuleType::CutInsideShot;
	F.Entropy->Word=(Type==ESkillRuleType::LongShot || Type==ESkillRuleType::CutInsideShot)?3:5;
	if(!TestTrue(TEXT("Actual other-family Skill binding"),F.ReachBranch(Type))) {return false;}
	if(Type==ESkillRuleType::LongShot || Type==ESkillRuleType::CutInsideShot)
	{
		Payload Choice;Choice.Intent=Branch::DirectShot;
		if(!TestTrue(TEXT("Canonical shot branch"),F.Send(F.Attacker(),Kind::SubmitBranchIntent,{},{},{},{},{},{},Choice))) {return false;}
	}
	else
	{
		if(!TestTrue(TEXT("Canonical other-family initial route"),F.Send(F.Attacker(),Type==ESkillRuleType::PassControl?Kind::PassControlInitialRouteRoll:Kind::ThroughBallInitialRouteRoll))) {return false;}
	}
	const auto K=static_cast<Kind>(FCString::Atoi(*Parts[2]));const bool Attack=K==Kind::CrossHighAttackRoll || K==Kind::CrossLowAttackRoll;
	auto* PC=Attack?F.Attacker():F.Defender();F.RequestedKind=K;const auto E=F.Request(PC);const FFrozen Before(F);
	TestEqual(TEXT("Every Cross roll rejected outside Cross"),F.Mode->SubmitConnectionPlayerIntent(PC,E).Code,Code::AuthorityRejected);Before.Verify(*this,F);return true;
}

IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFMCodexCrossAckOrder,
	"FMCodex.NetworkPlay.CrossContestTransport.06.AckViewOrder",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
void FFMCodexCrossAckOrder::GetTests(TArray<FString>& N,TArray<FString>& C) const
{
	for(const TCHAR* K:{TEXT("HighAttack"),TEXT("HighDefense"),TEXT("LowAttack"),TEXT("LowDefense")})
		for(const TCHAR* O:{TEXT("AckFirst"),TEXT("ViewFirst"),TEXT("Reject")})
		{const auto S=FString::Printf(TEXT("%s.%s"),K,O);N.Add(S);C.Add(S);}
}
bool FFMCodexCrossAckOrder::RunTest(const FString& P)
{
	using namespace FMCodexNetworkCrossContestTests;FCrossFixture F;const bool High=P.StartsWith(TEXT("High")),Attack=P.Contains(TEXT("Attack"));
	if(!TestTrue(TEXT("Canonical route"),F.Prepare(High))) {return false;}
	if(!Attack && !TestEqual(TEXT("Attack prefix"),F.Roll(High,true,4).Code,Code::Accepted)) {return false;}
	auto* PC=Attack?F.Attacker():F.Defender();auto& Client=F.Client(PC);const auto Before=PC->GetOwnerView();Envelope E;
	TestTrue(TEXT("Generic begin"),Client.BeginCrossContest(Before,RollKind(High,Attack),E));const auto Original=E;
	TestFalse(TEXT("Duplicate click cannot issue request"),Client.BeginCrossContest(Before,RollKind(High,Attack),E));
	TestEqual(TEXT("Duplicate click preserves envelope"),E.RequestId,Original.RequestId);
	FFMCodexNetworkPlayerIntentAck Wrong;Wrong.MatchInstanceId=E.MatchInstanceId;Wrong.RequestId=E.RequestId+1;Wrong.Code=Code::Accepted;Wrong.ViewRevision=Before.ViewRevision+1;
	TestFalse(TEXT("Unrelated ACK ignored"),Client.ObserveAck(Wrong));TestTrue(TEXT("Still pending"),Client.IsPending());
	if(P.EndsWith(TEXT("Reject"))) {E.IntentKind=RollKind(!High,Attack);}
	const auto Ack=F.Mode->SubmitConnectionPlayerIntent(PC,E);const auto After=PC->GetOwnerView();
	if(P.EndsWith(TEXT("Reject")))
	{
		TestEqual(TEXT("Wrong route rejected"),Ack.Code,Code::AuthorityRejected);
		TestTrue(TEXT("Rejected ACK correlated"),Client.ObserveAck(Ack));TestFalse(TEXT("No View required after rejection"),Client.IsPending());return true;
	}
	TestEqual(TEXT("Accepted"),Ack.Code,Code::Accepted);
	if(P.EndsWith(TEXT("AckFirst")))
	{
		Client.ObserveAck(Ack);TestTrue(TEXT("ACK alone cannot complete"),Client.IsPending());
		Client.ObserveView(Before);TestTrue(TEXT("Old view insufficient"),Client.IsPending());Client.ObserveView(After);
	}
	else
	{
		Client.ObserveView(After);TestTrue(TEXT("View alone cannot complete"),Client.IsPending());
		Client.ObserveView(Before);TestTrue(TEXT("Old view cannot finish"),Client.IsPending());Client.ObserveAck(Ack);
	}
	TestFalse(TEXT("Matching ACK and new View finish"),Client.IsPending());TestFalse(TEXT("Duplicate ACK ignored"),Client.ObserveAck(Ack));return true;
}

IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFMCodexCrossRealStale,
	"FMCodex.NetworkPlay.CrossContestTransport.07.RealAttackNToNPlusOne",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
void FFMCodexCrossRealStale::GetTests(TArray<FString>& N,TArray<FString>& C) const
{for(const TCHAR* K:{TEXT("HighAttack"),TEXT("HighDefense"),TEXT("LowAttack"),TEXT("LowDefense")}) {N.Add(K);C.Add(K);}}
bool FFMCodexCrossRealStale::RunTest(const FString& P)
{
	using namespace FMCodexNetworkCrossContestTests;FCrossFixture F;const bool High=P.StartsWith(TEXT("High")),Attack=P.Contains(TEXT("Attack"));
	if(!TestTrue(TEXT("Attack N Cross wait"),F.Prepare(High,true))) {return false;}
	if(!Attack && !TestTrue(TEXT("N Attack prefix"),F.Send(F.Attacker(),RollKind(High,true)))) {return false;}
	F.RequestedKind=RollKind(High,Attack);auto* OldPC=Attack?F.Attacker():F.Defender();auto Held=F.Request(OldPC);
	TestTrue(TEXT("N captured legal request through continuous client namespace"),F.Send(OldPC,F.RequestedKind));
	if(Attack && !TestTrue(TEXT("N Defense closes canonically"),F.Send(F.Defender(),RollKind(High,false)))) {return false;}
	auto& Session=Access::Session(*F.Mode);
	TestEqual(TEXT("N really terminal"),Session.GetStateSnapshot().CurrentAttack.LifecycleState,EMatchPlayCurrentAttackLifecycleState::TerminalPendingAdvance);
	FMatchPlayAuthoritativeAdvanceAfterTerminalRequest Advance;Advance.AttackSequence=Held.ExpectedAttackSequence;Advance.RequestingSide=Side::PlayerA;
	F.Entropy->Word=5;
	if(!TestTrue(TEXT("Explicit fixture-only next-turn command"),Session.AdvanceAfterTerminal(Advance).CompletionResult.bSuccess)) {return false;}
	TestTrue(TEXT("Next entry stable"),Access::Advance(*F.Mode).bSuccess);Access::Publish(*F.Mode);
	TestEqual(TEXT("True next sequence"),F.A->GetOwnerView().AttackSequence,Held.ExpectedAttackSequence+1);
	TestEqual(TEXT("Old contest disclosure cannot cross sequence"),F.A->GetOwnerView().CrossContest.AttackD6,0);
	if(!TestTrue(TEXT("Equivalent N+1 actual Cross route"),F.Prepare(High,true))) {return false;}
	if(!Attack && !TestTrue(TEXT("N+1 actual Attack prefix"),F.Send(F.Attacker(),RollKind(High,true)))) {return false;}
	auto* PC=Attack?F.Attacker():F.Defender();Held.RequestId=F.Next(PC)++;const FFrozen Before(F);
	TestEqual(TEXT("Fresh ID held old sequence rejected"),F.Mode->SubmitConnectionPlayerIntent(PC,Held).Code,Code::StaleAttackSequence);Before.Verify(*this,F);
	F.RequestedKind=RollKind(High,Attack);
	TestEqual(TEXT("N+1 current input remains legal"),F.Mode->SubmitConnectionPlayerIntent(PC,F.Request(PC)).Code,Code::Accepted);
	const FFrozen Current(F);auto Old=Held;Old.RequestId=F.Next(PC)++;
	TestEqual(TEXT("Closed prior attack replay still stale"),F.Mode->SubmitConnectionPlayerIntent(PC,Old).Code,Code::StaleAttackSequence);Current.Verify(*this,F);return true;
}

IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFMCodexCrossSafeProjection,
	"FMCodex.NetworkPlay.CrossContestTransport.08.SafeProjectionAndScoreGate",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
void FFMCodexCrossSafeProjection::GetTests(TArray<FString>& N,TArray<FString>& C) const
{for(const TCHAR* K:{TEXT("AHigh"),TEXT("ALow"),TEXT("BHigh"),TEXT("BLow")}) {N.Add(K);C.Add(K);}}
bool FFMCodexCrossSafeProjection::RunTest(const FString& P)
{
	using namespace FMCodexNetworkCrossContestTests;FCrossFixture F(P.StartsWith(TEXT("B")));const bool High=P.Contains(TEXT("High"));
	if(!TestTrue(TEXT("Canonical actual route"),F.Prepare(High,true))) {return false;}
	const auto Player=F.Attacker()->GetOwnerView().ViewerSide;
	for(bool RouteReveal:{true,false}) for(bool EntryReveal:{true,false})
	{
		const auto V=Project(F.Safe(Player,0,false,RouteReveal,EntryReveal),Player);
		TestEqual(TEXT("Action needs disclosed actual route and entry"),V.CrossContestAction,RouteReveal && EntryReveal?RollAction(High,true):Action::None);
	}
	TestEqual(TEXT("Null viewer has no action"),Project(F.Safe(Side::None,0),Side::None).CrossContestAction,Action::None);
	TestEqual(TEXT("Attack"),F.Roll(High,true,6).Code,Code::Accepted);
	TestEqual(TEXT("Defense"),F.Roll(High,false,1).Code,Code::Accepted);
	const auto State=Access::Session(*F.Mode).GetStateSnapshot();
	for(auto Viewer:{Side::PlayerA,Side::PlayerB})
	{
		for(int32 Count=0;Count<=2;++Count)
		{
			const auto Safe=F.Safe(Viewer,Count);const auto V=Project(Safe,Viewer);
			TestEqual(TEXT("Only explicitly disclosed Attack"),V.CrossContest.AttackD6,Count>=1?6:0);
			TestEqual(TEXT("Only explicitly disclosed Defense"),V.CrossContest.DefenseD6,Count>=2?1:0);
			TestEqual(TEXT("No Formula completion via hidden die"),V.CrossContest.bFormulaResolved,Count==2);
			TestEqual(TEXT("Terminal wait is high-level lifecycle only"),V.EntryWait,EFMCodexNetworkEntryWait::TerminalPendingAdvance);
			TestEqual(TEXT("No next-turn action added"),V.CrossContestAction,Action::None);
			TestEqual(TEXT("Withheld score A"),V.PlayerAScore,0);TestEqual(TEXT("Withheld score B"),V.PlayerBScore,0);
			TestTrue(TEXT("Goal history hidden before terminal reveal"),Safe.GoalHistory.IsEmpty());
		}
		const auto Revealed=Project(F.Safe(Viewer,2,true),Viewer);
		TestEqual(TEXT("Existing separate terminal reveal permits committed score A"),Revealed.PlayerAScore,State.RuntimeState.PlayerAState.Score);
		TestEqual(TEXT("Existing separate terminal reveal permits committed score B"),Revealed.PlayerBScore,State.RuntimeState.PlayerBState.Score);
		auto Safe=F.Safe(Viewer,2);
		const auto Copy=Safe.ResolutionFacts.Rolls.Last();Safe.ResolutionFacts.Rolls.Add(Copy);
		TestEqual(TEXT("Duplicate contest record fails whole projection"),Project(Safe,Viewer).CrossContest.AttackD6,0);
		Safe=F.Safe(Viewer,2);Safe.ResolutionFacts.Rolls[1].RawD6=7;
		TestEqual(TEXT("Invalid D6 fails whole projection"),Project(Safe,Viewer).CrossContest.AttackD6,0);
		Safe=F.Safe(Viewer,2);Safe.ResolutionFacts.Rolls.RemoveAt(1);
		TestEqual(TEXT("Defense without Attack cannot be disclosed"),Project(Safe,Viewer).CrossContest.DefenseD6,0);
		Safe=F.Safe(Viewer,2);Safe.ResolutionFacts.FormulaContests.Reset();
		TestFalse(TEXT("No fabricated Formula completion"),Project(Safe,Viewer).CrossContest.bFormulaResolved);
	}
	const FFrozen Before(F);
	FFMCodexNetworkCrossContestTestAccess::WithholdContest(Access::Runtime(*F.Mode));
	const auto V=Access::Runtime(*F.Mode).BuildClientView(Player,1,EFMCodexNetworkBootstrapState::MatchReady);
	TestEqual(TEXT("Runtime reveal belongs to exact sequence"),V.CrossContest.AttackD6,0);
	TestEqual(TEXT("Runtime also withholds Defense"),V.CrossContest.DefenseD6,0);
	Before.Verify(*this,F);return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexCrossClosedWire,
	"FMCodex.NetworkPlay.CrossContestTransport.09.ClosedWireNamespaceAndSharedPath",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexCrossClosedWire::RunTest(const FString&)
{
	using namespace FMCodexNetworkCrossContestTests;
	for(int32 Tag=1;Tag<=17;++Tag) for(int32 Mask=0;Mask<256;++Mask)
	{
		Envelope E;E.IntentKind=static_cast<Kind>(Tag);
		if(Mask&1){E.Deployment.CardId=TEXT("Card");E.Deployment.SlotId=TEXT("Slot");}
		if(Mask&2){E.Goalkeeper.SlotId=TEXT("Slot");} if(Mask&4){E.Carrier.CarrierCardId=TEXT("Card");}
		if(Mask&8){E.Marker.MarkerCardId=TEXT("Card");} if(Mask&16){E.Runner.RunnerCardId=TEXT("Card");}
		if(Mask&32){E.Helper.HelperCardId=TEXT("Card");} if(Mask&64){E.Skill.SkillId=TEXT("Skill");}
		if(Mask&128){E.Branch.Intent=Branch::CrossHigh;}
		const int32 Expected=Tag==1 || Tag==4 || Tag>=11?0:Tag==2?1:Tag==3?2:1<<(Tag-3);
		TestEqual(TEXT("17 closed tags x 256 member masks"),E.ValidatePayloadShape(),Mask==Expected?Code::None:Code::InvalidPayload);
	}
	for(int32 K=14;K<=17;++K)
	{
		Envelope Original;Original.MatchInstanceId=FGuid::NewGuid();Original.RequestId=19;Original.ExpectedAttackSequence=5;Original.IntentKind=static_cast<Kind>(K);
		TArray<uint8> Bytes;FMemoryWriter Output(Bytes);FObjectAndNameAsStringProxyArchive Writer(Output,false);
		Envelope::StaticStruct()->SerializeItem(Writer,&Original,nullptr);
		FMemoryReader Input(Bytes);FObjectAndNameAsStringProxyArchive Reader(Input,false);Envelope Decoded;
		Envelope::StaticStruct()->SerializeItem(Reader,&Decoded,nullptr);
		TestTrue(TEXT("Actual reflected empty Cross request roundtrip"),!Writer.IsError() && !Reader.IsError() && Envelope::StaticStruct()->CompareScriptStruct(&Original,&Decoded,0));
		TestEqual(TEXT("Decoded request remains no gameplay payload"),Decoded.ValidatePayloadShape(),Code::None);
	}
	FFMCodexNetworkIntentLedger Ledger;Envelope E;E.MatchInstanceId=FGuid::NewGuid();E.ExpectedAttackSequence=1;
	for(int32 I=1;I<=17;++I)
	{
		E.IntentKind=static_cast<Kind>(I);E.RequestId=I;TestTrue(TEXT("One continuous namespace"),Ledger.Consume(E.MatchInstanceId,E));
		E.IntentKind=Kind::CrossHighDefenseRoll;TestEqual(TEXT("Changing kind cannot reuse ID"),Ledger.Check(E.MatchInstanceId,E),Code::DuplicateOrAlreadyResolved);
	}
	E.RequestId=17+1025;TestEqual(TEXT("1025 denied"),Ledger.Check(E.MatchInstanceId,E),Code::InvalidPayload);
	E.RequestId=17+1024;TestEqual(TEXT("1024 allowed"),Ledger.Check(E.MatchInstanceId,E),Code::None);
	E.RequestId=18;TestTrue(TEXT("No poisoning"),Ledger.Consume(E.MatchInstanceId,E));
	FCrossFixture F;if(!TestTrue(TEXT("Canonical mismatch fixture"),F.Prepare(true))) {return false;}
	const FFrozen Before(F);
	for(int32 K=14;K<=17;++K)
	{
		auto Wrong=Canonical(static_cast<Kind>(K),1,Side::PlayerA);Wrong.Payload.Set<FMatchPlayFullD12EntryRequest>({});
		TestEqual(TEXT("Canonical variant mismatch rejected before Session"),Access::Runtime(*F.Mode).SubmitPlayerIntent(Wrong).ErrorCode,EMatchPlayPlayerIntentPortErrorCode::PayloadTypeMismatch);Before.Verify(*this,F);
	}
	FString Source;TestTrue(TEXT("Controller source"),FFileHelper::LoadFileToString(Source,*(FPaths::ProjectDir()/TEXT("Source/FMCodex/NetworkPlay/FMCodexNetworkMatchPlayerController.cpp"))));
	int32 Start=Source.Find(TEXT("void AFMCodexNetworkMatchPlayerController::SubmitCrossContest("));
	int32 End=Source.Find(TEXT("void AFMCodexNetworkMatchPlayerController::DevProbeWrongCrossContestRoute"),ESearchCase::CaseSensitive,ESearchDir::FromStart,Start);
	auto Block=Source.Mid(Start,End-Start);
	TestTrue(TEXT("Generated RPC for both roles"),Block.Contains(TEXT("ServerSubmitPlayerIntent(Envelope)")));
	TestFalse(TEXT("No implementation or Host bypass"),Block.Contains(TEXT("_Implementation")) || Block.Contains(TEXT("HasAuthority")) || Block.Contains(TEXT("HostPort")));
	TestTrue(TEXT("Local source"),FFileHelper::LoadFileToString(Source,*(FPaths::ProjectDir()/TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchHostGameMode.cpp"))));
	Start=Source.Find(TEXT("case EMatchPlayAuthoritativeCommandKind::ResolveCrossHighAttackRoll:"));
	End=Source.Find(TEXT("case EMatchPlayAuthoritativeCommandKind::ResolveThroughBallFeetAttackRoll:"),ESearchCase::CaseSensitive,ESearchDir::FromStart,Start);
	Block=Source.Mid(Start,End-Start);
	for(const TCHAR* Name:{TEXT("CrossHighAttack"),TEXT("CrossHighDefense"),TEXT("CrossLowAttack"),TEXT("CrossLowDefense")})
		TestTrue(TEXT("Local invocation-specific deterministic override retained"),Block.Contains(FString(TEXT("EFMCodexLocalDevRollInvocation::"))+Name));
	TestTrue(TEXT("Shared Local gameplay port"),Block.Contains(TEXT("FMatchPlayEntryDeploymentPlayerIntentPort")) && Block.Contains(TEXT("ExecuteProviderCall")));
	return true;
}

#endif
