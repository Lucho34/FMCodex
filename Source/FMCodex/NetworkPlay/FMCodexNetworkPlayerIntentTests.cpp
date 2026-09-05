#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "FMCodexNetworkMatchGameMode.h"
#include "FMCodexNetworkRNGTestEntropy.h"
#include "FMCodexNetworkMatchPlayerController.h"
#include "FMCodexNetworkMatchPlayerState.h"
#include "../MatchPlayRuntime/MatchPlayAuthoritativeSession.h"
#include "../MatchPlayRuntime/MatchPlayFullD12PlayerIntentPort.h"
#include "../MatchPlayRuntime/MatchPlayServerCoordinator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "UObject/UnrealType.h"

struct FFMCodexNetworkIntentTestAccess
{
	static bool Configure(AFMCodexNetworkMatchGameMode& Mode,
		AFMCodexNetworkMatchPlayerController* A, AFMCodexNetworkMatchPlayerState* PS_A,
		AFMCodexNetworkMatchPlayerController* B, AFMCodexNetworkMatchPlayerState* PS_B,
		TUniquePtr<IMatchPlayAttackEntryRollProvider> Provider)
	{
		Mode.MatchInstanceId = FGuid::NewGuid();
		Mode.BootstrapConfiguration = FFMCodexNetworkBootstrapConfigurationFactory::CreatePrototypeMatch();
		Mode.ParticipantRegistry.Admit(A, PS_A);
		Mode.ParticipantRegistry.Admit(B, PS_B);
		Mode.MatchRuntime = MakeUnique<FFMCodexNetworkMatchRuntime>(
			Mode.MatchInstanceId, MakeUnique<FFMCodexNetworkScriptedEntropy>(), MoveTemp(Provider));
		const bool bReady = Mode.MatchRuntime->InitializeOnce(Mode.BootstrapConfiguration).bSuccess;
		Mode.PublishOwnerViews(EFMCodexNetworkBootstrapState::MatchReady);
		return bReady;
	}
	static bool UseEntropy(AFMCodexNetworkMatchGameMode& Mode, TUniquePtr<IFMCodexNetworkEntropySource> Entropy)
	{
		Mode.MatchRuntime = MakeUnique<FFMCodexNetworkMatchRuntime>(Mode.MatchInstanceId, MoveTemp(Entropy));
		const bool bReady = Mode.MatchRuntime->InitializeOnce(Mode.BootstrapConfiguration).bSuccess;
		Mode.PublishOwnerViews(EFMCodexNetworkBootstrapState::MatchReady);
		return bReady;
	}
	static FFMCodexNetworkMatchRuntime& Runtime(AFMCodexNetworkMatchGameMode& Mode)
	{
		return *Mode.MatchRuntime;
	}
	static FMatchPlayAuthoritativeSession& Session(AFMCodexNetworkMatchGameMode& Mode)
	{
		return *Mode.MatchRuntime->AuthoritativeSession;
	}
	static int32 Revision(AFMCodexNetworkMatchGameMode& Mode) { return Mode.ViewRevision; }
	static int32 Ledgers(AFMCodexNetworkMatchGameMode& Mode) { return Mode.IntentLedgers.Num(); }
	static void Publish(AFMCodexNetworkMatchGameMode& Mode)
	{
		Mode.PublishOwnerViews(EFMCodexNetworkBootstrapState::MatchReady);
	}
};

namespace FMCodexNetworkPlayerIntentTests
{
	using Access = FFMCodexNetworkIntentTestAccess;
	using Code = EFMCodexNetworkIntentAckCode;
	using Side = EInitialTurnOrderPlayer;
	class FEntryProvider final : public IMatchPlayAttackEntryRollProvider
	{
	public:
		explicit FEntryProvider(int32 InD12, bool bInFailSelection = false)
			: D12(InD12), bFailSelection(bInFailSelection) {}
		virtual FMatchPlayAttackEntryRollProviderResult RollD12(EMatchPlayAttackEntryRollPurpose) override
		{
			FMatchPlayAttackEntryRollProviderResult Result;
			Result.bSuccess = D12 > 0;
			Result.RawRoll = D12;
			return Result;
		}
		virtual FMatchPlayAttackEntryRollProviderResult RollD6(EMatchPlayAttackEntryRollPurpose) override
		{
			return {}; // This vertical slice must never request a type D6.
		}
		virtual FMatchPlayAttackEntrySelectionProviderResult SelectUniformIndex(
			EMatchPlayAttackEntryRollPurpose, int32 CandidateCount) override
		{
			FMatchPlayAttackEntrySelectionProviderResult Result;
			Result.bSuccess = !bFailSelection && CandidateCount > 0;
			Result.SelectedIndex = Result.bSuccess ? 0 : INDEX_NONE;
			return Result;
		}
		int32 D12;
		bool bFailSelection;
	};

	struct FFixture
	{
		UWorld* World = nullptr;
		AFMCodexNetworkMatchGameMode* Mode = nullptr;
		AFMCodexNetworkMatchPlayerController* A = nullptr;
		AFMCodexNetworkMatchPlayerController* B = nullptr;
		bool bReady = false;
		explicit FFixture(int32 D12 = 4, bool bFailSelection = false)
		{
			World = UWorld::CreateWorld(EWorldType::Game, false);
			GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
			Mode = World->SpawnActor<AFMCodexNetworkMatchGameMode>();
			A = World->SpawnActor<AFMCodexNetworkMatchPlayerController>();
			B = World->SpawnActor<AFMCodexNetworkMatchPlayerController>();
			World->AddController(A);
			World->AddController(B);
			bReady = Access::Configure(*Mode, A,
				World->SpawnActor<AFMCodexNetworkMatchPlayerState>(), B,
				World->SpawnActor<AFMCodexNetworkMatchPlayerState>(),
				MakeUnique<FEntryProvider>(D12, bFailSelection));
		}
		~FFixture()
		{
			GEngine->DestroyWorldContext(World);
			World->DestroyWorld(false);
		}
		FFMCodexNetworkPlayerIntentEnvelope Request(int64 Id = 1) const
		{
			FFMCodexNetworkPlayerIntentEnvelope Result;
			Result.MatchInstanceId = Mode->GetMatchInstanceId();
			Result.RequestId = Id;
			Result.ExpectedAttackSequence = A->GetOwnerView().AttackSequence;
			Result.IntentKind = EFMCodexNetworkPlayerIntentKind::RequestInitialActionPointRoll;
			return Result;
		}
	};
	bool SameState(const FMatchPlayState& A, const FMatchPlayState& B)
	{
		return FMatchPlayState::StaticStruct()->CompareScriptStruct(&A, &B, 0);
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexNetworkIntentRejectionTest,
	"FMCodex.NetworkPlay.PlayerIntentTransport.01.RejectionBoundary",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexNetworkIntentRejectionTest::RunTest(const FString&)
{
	using namespace FMCodexNetworkPlayerIntentTests;
	FFixture F;
	TestTrue(TEXT("Initialized"), F.bReady);
	const auto Before = Access::Session(*F.Mode).GetStateSnapshot();
	const int32 Revision = Access::Revision(*F.Mode);
	auto Reject = [&](const TCHAR* Name, AFMCodexNetworkMatchPlayerController* PC,
		const FFMCodexNetworkPlayerIntentEnvelope& Request, Code Expected)
	{
		const auto Ack = F.Mode->SubmitConnectionPlayerIntent(PC, Request);
		TestEqual(Name, Ack.Code, Expected);
		TestEqual(TEXT("ACK echoes submitted match"), Ack.MatchInstanceId, Request.MatchInstanceId);
		TestEqual(TEXT("ACK echoes request"), Ack.RequestId, Request.RequestId);
		TestEqual(TEXT("Rejected revision unchanged"), Ack.ViewRevision, Revision);
		TestEqual(TEXT("Rejected provider untouched"),
			Access::Runtime(*F.Mode).GetEntryProviderInvocationCount(), 0);
		TestTrue(TEXT("Entire reflected authoritative state unchanged"),
			SameState(Before, Access::Session(*F.Mode).GetStateSnapshot()));
	};
	auto E = F.Request();
	auto* Outsider = F.World->SpawnActor<AFMCodexNetworkMatchPlayerController>();
	Reject(TEXT("Nonparticipant"), Outsider, E, Code::NotParticipant);
	Reject(TEXT("Null"), nullptr, E, Code::NotParticipant);
	TestEqual(TEXT("Nonparticipant allocates no ledger"), Access::Ledgers(*F.Mode), 0);
	E.MatchInstanceId = FGuid::NewGuid();
	Reject(TEXT("Wrong match"), F.A, E, Code::MatchMismatch);
	E = F.Request(0);
	Reject(TEXT("Malformed request ID"), F.A, E, Code::InvalidPayload);
	E = F.Request(1); E.ExpectedAttackSequence = 0;
	Reject(TEXT("Malformed sequence"), F.A, E, Code::InvalidPayload);
	E = F.Request(1); E.IntentKind = EFMCodexNetworkPlayerIntentKind::None;
	Reject(TEXT("No intent/internal action has no wire representation"), F.A, E, Code::NotPlayerIntent);
	E = F.Request(2); E.IntentKind = static_cast<EFMCodexNetworkPlayerIntentKind>(255);
	Reject(TEXT("Unknown intent"), F.A, E, Code::NotPlayerIntent);
	E = F.Request(3); ++E.ExpectedAttackSequence;
	Reject(TEXT("Stale sequence"), F.A, E, Code::StaleAttackSequence);
	E = F.Request(1);
	Reject(TEXT("Wrong connection side"), F.B, E, Code::WrongSide);
	Reject(TEXT("Rejected request replay"), F.B, E, Code::DuplicateOrAlreadyResolved);
	E = F.Request(4);
	TestEqual(TEXT("Valid A accepted"), F.Mode->SubmitConnectionPlayerIntent(F.A, E).Code, Code::Accepted);
	const auto After = Access::Session(*F.Mode).GetStateSnapshot();
	const int32 AfterRevision = Access::Revision(*F.Mode);
	TestEqual(TEXT("Accepted request replay"), F.Mode->SubmitConnectionPlayerIntent(F.A, E).Code,
		Code::DuplicateOrAlreadyResolved);
	E.RequestId = 5;
	TestEqual(TEXT("New ID cannot reroll active attack"),
		F.Mode->SubmitConnectionPlayerIntent(F.A, E).Code, Code::InvalidPhase);
	TestEqual(TEXT("No repeated D12"), Access::Runtime(*F.Mode).GetD12ProviderInvocationCount(), 1);
	TestEqual(TEXT("No duplicate publication"), Access::Revision(*F.Mode), AfterRevision);
	TestTrue(TEXT("Post-accept rejects preserve full state"), SameState(After,
		Access::Session(*F.Mode).GetStateSnapshot()));
	return true;
}

IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFMCodexNetworkIntentBranchTest,
	"FMCodex.NetworkPlay.PlayerIntentTransport.02.FullD12Branches",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
void FFMCodexNetworkIntentBranchTest::GetTests(TArray<FString>& Names, TArray<FString>& Commands) const
{
	for (const TCHAR* Value : { TEXT("1"), TEXT("4"), TEXT("10") })
	{
		Names.Add(Value); Commands.Add(Value);
	}
}
bool FFMCodexNetworkIntentBranchTest::RunTest(const FString& Parameters)
{
	using namespace FMCodexNetworkPlayerIntentTests;
	const int32 D12 = FCString::Atoi(*Parameters);
	FFixture F(D12);
	TestTrue(TEXT("Initialized"), F.bReady);
	TestEqual(TEXT("A before hides roll"), F.A->GetOwnerView().DisclosedInitialD12, 0);
	TestEqual(TEXT("B before hides roll"), F.B->GetOwnerView().DisclosedInitialD12, 0);
	TestEqual(TEXT("B waits for opponent"), F.B->GetOwnerView().InteractionState,
		EFMCodexNetworkClientInteractionState::WaitingForOpponentInitialActionPoint);
	const int32 BeforeRevision = Access::Revision(*F.Mode);
	const auto Ack = F.Mode->SubmitConnectionPlayerIntent(F.A, F.Request());
	TestEqual(TEXT("Accepted"), Ack.Code, Code::Accepted);
	TestEqual(TEXT("Exactly one publication"), Ack.ViewRevision, BeforeRevision + 1);
	for (auto* PC : { F.A, F.B })
	{
		const auto& View = PC->GetOwnerView();
		TestEqual(TEXT("Both viewers see saved D12"), View.DisclosedInitialD12, D12);
		TestEqual(TEXT("Both viewers have ACK revision"), View.ViewRevision, Ack.ViewRevision);
		TestEqual(TEXT("High-level branch only"), View.EntryBranch, D12 == 1
			? EFMCodexNetworkEntryBranch::SendingOff : D12 == 4
			? EFMCodexNetworkEntryBranch::Ordinary : EFMCodexNetworkEntryBranch::SetPiece);
		TestEqual(TEXT("Coordinator reaches correct stable wait"), View.EntryWait, D12 == 1
			? EFMCodexNetworkEntryWait::TerminalPendingAdvance : D12 == 4
			? EFMCodexNetworkEntryWait::Deployment : EFMCodexNetworkEntryWait::SetPieceTypeRoll);
	}
	TestEqual(TEXT("Only one D12 call"), Access::Runtime(*F.Mode).GetD12ProviderInvocationCount(), 1);
	TestEqual(TEXT("AP1 selection automatic; no type D6"),
		Access::Runtime(*F.Mode).GetEntryProviderInvocationCount(), D12 == 1 ? 2 : 1);
	const auto State = Access::Session(*F.Mode).GetStateSnapshot();
	TestEqual(TEXT("No opportunity consumed before advance"), State.RuntimeState.PlayerAState.UsedAttackCount, 0);
	TestEqual(TEXT("No fabricated goal"), State.RuntimeState.PlayerAState.Score, 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexNetworkIntentAckOrderTest,
	"FMCodex.NetworkPlay.PlayerIntentTransport.03.AckViewOrdering",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexNetworkIntentAckOrderTest::RunTest(const FString&)
{
	using namespace FMCodexNetworkPlayerIntentTests;
	for (bool bAckFirst : { true, false })
	{
		FFixture F;
		FFMCodexNetworkIntentClientState Client;
		FFMCodexNetworkPlayerIntentEnvelope E;
		TestTrue(TEXT("Owner can begin"), Client.Begin(F.A->GetOwnerView(), E));
		TestFalse(TEXT("Double-click cannot emit another request"), Client.Begin(F.A->GetOwnerView(), E));
		const auto Ack = F.Mode->SubmitConnectionPlayerIntent(F.A, E);
		if (bAckFirst)
		{
			TestTrue(TEXT("Accepted ACK correlates"), Client.ObserveAck(Ack));
			TestTrue(TEXT("ACK alone keeps pending/read-only"), Client.IsPending());
			Client.ObserveView(F.A->GetOwnerView());
		}
		else
		{
			Client.ObserveView(F.A->GetOwnerView());
			TestTrue(TEXT("View alone keeps pending"), Client.IsPending());
			TestTrue(TEXT("Later ACK correlates"), Client.ObserveAck(Ack));
		}
		TestFalse(TEXT("Both facts release pending"), Client.IsPending());
		TestFalse(TEXT("Repeated ACK is harmless"), Client.ObserveAck(Ack));
		TestFalse(TEXT("Read-only next stage emits no new request"), Client.Begin(F.A->GetOwnerView(), E));
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexNetworkIntentCorrelationTest,
	"FMCodex.NetworkPlay.PlayerIntentTransport.04.CorrelationAndMatchReset",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexNetworkIntentCorrelationTest::RunTest(const FString&)
{
	using namespace FMCodexNetworkPlayerIntentTests;
	FFixture F;
	FFMCodexNetworkIntentClientState Client;
	FFMCodexNetworkPlayerIntentEnvelope E;
	TestFalse(TEXT("Nonacting remote cannot begin"), Client.Begin(F.B->GetOwnerView(), E));
	TestTrue(TEXT("Acting owner begins"), Client.Begin(F.A->GetOwnerView(), E));
	FFMCodexNetworkPlayerIntentAck Ack;
	Ack.MatchInstanceId = E.MatchInstanceId; Ack.RequestId = E.RequestId + 1;
	Ack.Code = Code::Accepted; Ack.ViewRevision = 2;
	TestFalse(TEXT("Wrong request ACK ignored"), Client.ObserveAck(Ack));
	Ack.RequestId = E.RequestId; Ack.MatchInstanceId = FGuid::NewGuid();
	TestFalse(TEXT("Wrong match ACK ignored"), Client.ObserveAck(Ack));
	Ack.MatchInstanceId = E.MatchInstanceId; Ack.Code = Code::WrongSide;
	TestTrue(TEXT("Rejection correlates"), Client.ObserveAck(Ack));
	TestFalse(TEXT("Rejection releases pending without new view"), Client.IsPending());
	TestTrue(TEXT("Retry uses new ID"), Client.Begin(F.A->GetOwnerView(), E));
	TestEqual(TEXT("Monotonic retry"), E.RequestId, int64(2));
	TestFalse(TEXT("Stale ACK cannot clear next request"), Client.ObserveAck(Ack));
	auto NextMatch = F.A->GetOwnerView(); NextMatch.MatchInstanceId = FGuid::NewGuid();
	Client.ObserveView(NextMatch);
	TestFalse(TEXT("New authoritative match clears old pending"), Client.IsPending());
	TestTrue(TEXT("New match can submit"), Client.Begin(NextMatch, E));
	TestEqual(TEXT("IDs stay monotonic across matches"), E.RequestId, int64(3));
	TestFalse(TEXT("Old match ACK harmless"), Client.ObserveAck(Ack));
	TestTrue(TEXT("New match still pending"), Client.IsPending());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexNetworkIntentLedgerTest,
	"FMCodex.NetworkPlay.PlayerIntentTransport.05.BoundedLedger",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexNetworkIntentLedgerTest::RunTest(const FString&)
{
	FFMCodexNetworkIntentLedger Ledger;
	FGuid Match = FGuid::NewGuid();
	FFMCodexNetworkPlayerIntentEnvelope E;
	E.MatchInstanceId = Match; E.RequestId = 100;
	TestTrue(TEXT("First positive ID"), Ledger.Consume(Match, E));
	E.RequestId = 99;
	TestFalse(TEXT("Lower out-of-order ID denied"), Ledger.Consume(Match, E));
	E.MatchInstanceId = FGuid::NewGuid(); E.RequestId = 1000;
	TestFalse(TEXT("Fabricated match cannot reset high-water"), Ledger.Consume(Match, E));
	E.MatchInstanceId = Match; E.RequestId = 100;
	TestFalse(TEXT("Duplicate remains blocked"), Ledger.Consume(Match, E));
	Match = FGuid::NewGuid(); E.MatchInstanceId = Match; E.RequestId = 1;
	TestTrue(TEXT("Only server-selected new match resets ledger"), Ledger.Consume(Match, E));
	TestTrue(TEXT("Constant-size receipt storage"), sizeof(Ledger) <= 32);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexNetworkIntentStaleAttackTest,
	"FMCodex.NetworkPlay.PlayerIntentTransport.06.StaleAttackAndSideParity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexNetworkIntentStaleAttackTest::RunTest(const FString&)
{
	using namespace FMCodexNetworkPlayerIntentTests;
	FFixture F(1);
	const auto Old = F.Request();
	TestEqual(TEXT("Host-side A accepted"), F.Mode->SubmitConnectionPlayerIntent(F.A, Old).Code, Code::Accepted);
	FMatchPlayAuthoritativeAdvanceAfterTerminalRequest Advance;
	Advance.AttackSequence = Old.ExpectedAttackSequence;
	Advance.RequestingSide = Side::PlayerA;
	TestTrue(TEXT("Server-only fixture advances AP1 canonically"),
		Access::Session(*F.Mode).AdvanceAfterTerminal(Advance).RuntimeEnvelope.bDomainSuccess);
	Access::Publish(*F.Mode);
	const auto Before = Access::Session(*F.Mode).GetStateSnapshot();
	auto Stale = Old; Stale.RequestId = 2;
	TestEqual(TEXT("Old attack cannot enter N+1"), F.Mode->SubmitConnectionPlayerIntent(F.A, Stale).Code,
		Code::StaleAttackSequence);
	TestTrue(TEXT("Stale preserves entire state"), SameState(Before, Access::Session(*F.Mode).GetStateSnapshot()));
	TestEqual(TEXT("No stale RNG"), Access::Runtime(*F.Mode).GetD12ProviderInvocationCount(), 1);
	auto Remote = F.Request(1);
	TestEqual(TEXT("Now remote B is acting"), F.B->GetOwnerView().ExpectedActingSide, Side::PlayerB);
	TestEqual(TEXT("Remote-side B uses same boundary and port"),
		F.Mode->SubmitConnectionPlayerIntent(F.B, Remote).Code, Code::Accepted);
	TestEqual(TEXT("Remote accepted exactly one further D12"), Access::Runtime(*F.Mode).GetD12ProviderInvocationCount(), 2);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexNetworkIntentFailureTest,
	"FMCodex.NetworkPlay.PlayerIntentTransport.07.AuthorityAndCoordinatorFailure",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexNetworkIntentFailureTest::RunTest(const FString&)
{
	using namespace FMCodexNetworkPlayerIntentTests;
	{
		FFixture F(0);
		const auto Before = Access::Session(*F.Mode).GetStateSnapshot();
		const int32 Revision = Access::Revision(*F.Mode);
		const auto E = F.Request();
		TestEqual(TEXT("Provider failure typed rejection"),
			F.Mode->SubmitConnectionPlayerIntent(F.A, E).Code, Code::AuthorityRejected);
		TestEqual(TEXT("No failure publication"), Access::Revision(*F.Mode), Revision);
		TestTrue(TEXT("No authority adoption"), SameState(Before, Access::Session(*F.Mode).GetStateSnapshot()));
		TestEqual(TEXT("Retry of same ID blocked"), F.Mode->SubmitConnectionPlayerIntent(F.A, E).Code,
			Code::DuplicateOrAlreadyResolved);
		TestEqual(TEXT("Failed request does not reroll"), Access::Runtime(*F.Mode).GetD12ProviderInvocationCount(), 1);
	}
	{
		FFixture F(1, true);
		const int32 Revision = Access::Revision(*F.Mode);
		TestEqual(TEXT("Failed AP1 coordinator reported explicitly"),
			F.Mode->SubmitConnectionPlayerIntent(F.A, F.Request()).Code, Code::InternalFailure);
		TestEqual(TEXT("Committed entry publishes failure view"), Access::Revision(*F.Mode), Revision + 1);
		TestEqual(TEXT("Failed world read-only"), F.A->GetOwnerView().BootstrapState,
			EFMCodexNetworkBootstrapState::BootstrapFailed);
		TestEqual(TEXT("Saved D12 still visible"), F.B->GetOwnerView().DisclosedInitialD12, 1);
		TestEqual(TEXT("No further submission during fault"),
			F.Mode->SubmitConnectionPlayerIntent(F.A, F.Request(2)).Code, Code::InvalidPhase);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexNetworkIntentSurfaceTest,
	"FMCodex.NetworkPlay.PlayerIntentTransport.08.TransportSecrecyAndSharedPort",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexNetworkIntentSurfaceTest::RunTest(const FString&)
{
	using namespace FMCodexNetworkPlayerIntentTests;
	auto ExactFields = [&](UScriptStruct* Struct, const TArray<FName>& Allowed)
	{
		int32 Count = 0;
		for (TFieldIterator<FProperty> It(Struct); It; ++It)
		{
			++Count;
			TestTrue(TEXT("Only allowlisted wire field"), Allowed.Contains(It->GetFName()));
		}
		TestEqual(TEXT("Exact field count"), Count, Allowed.Num());
	};
	ExactFields(FFMCodexNetworkPlayerIntentEnvelope::StaticStruct(),
		{TEXT("MatchInstanceId"), TEXT("RequestId"), TEXT("ExpectedAttackSequence"), TEXT("IntentKind"), TEXT("Deployment")});
	ExactFields(FFMCodexNetworkDeployOrdinaryPayload::StaticStruct(), {TEXT("CardId"), TEXT("SlotId")});
	ExactFields(FFMCodexNetworkDeploymentOption::StaticStruct(), {TEXT("Choice"), TEXT("CardLabel"), TEXT("SlotLabel")});
	ExactFields(FFMCodexNetworkDeploymentSummary::StaticStruct(), {TEXT("Side"), TEXT("Placement")});
	ExactFields(FFMCodexNetworkPlayerIntentAck::StaticStruct(),
		{TEXT("MatchInstanceId"), TEXT("RequestId"), TEXT("Code"), TEXT("ViewRevision")});
	ExactFields(FFMCodexNetworkClientViewSnapshot::StaticStruct(),
		{TEXT("MatchInstanceId"), TEXT("ViewRevision"), TEXT("ViewerSide"), TEXT("BootstrapState"),
		TEXT("InteractionState"), TEXT("bMatchInitialized"), TEXT("bMatchEnded"), TEXT("AttackSequence"),
		TEXT("CurrentAttackingSide"), TEXT("ExpectedActingSide"), TEXT("PlayerAScore"), TEXT("PlayerBScore"),
		TEXT("PlayerAMaxAttackOpportunities"), TEXT("PlayerBMaxAttackOpportunities"),
		TEXT("DisclosedInitialD12"), TEXT("EntryBranch"), TEXT("EntryWait"), TEXT("DeploymentOptions"),
		TEXT("DeploymentCount"), TEXT("LastDeployment")});
	const auto* Class = AFMCodexNetworkMatchPlayerController::StaticClass();
	const auto* Server = Class->FindFunctionByName(TEXT("ServerSubmitPlayerIntent"));
	const auto* Client = Class->FindFunctionByName(TEXT("ClientReceivePlayerIntentAck"));
	TestTrue(TEXT("Reliable server RPC"), Server && Server->HasAllFunctionFlags(FUNC_Net | FUNC_NetServer | FUNC_NetReliable));
	TestTrue(TEXT("Owner client ACK, no multicast"), Client && Client->HasAllFunctionFlags(FUNC_Net | FUNC_NetClient | FUNC_NetReliable)
		&& !Client->HasAnyFunctionFlags(FUNC_NetMulticast));
	for (const TCHAR* Path : { TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchHostGameMode.cpp"),
		TEXT("Source/FMCodex/NetworkPlay/FMCodexNetworkMatchRuntime.cpp") })
	{
		FString Source;
		TestTrue(TEXT("Shared port consumer loads"), FFileHelper::LoadFileToString(Source, *(FPaths::ProjectDir() / Path)));
		TestTrue(TEXT("Both hosts use same entry/deployment port"), Source.Contains(TEXT("FMatchPlayEntryDeploymentPlayerIntentPort("))
			|| Source.Contains(TEXT("FMatchPlayEntryDeploymentPlayerIntentPort Port(")));
	}
	FFixture F;
	auto& Runtime = Access::Runtime(*F.Mode);
	FMatchPlayPlayerIntent Internal;
	Internal.CommandKind = EMatchPlayAuthoritativeCommandKind::ResolveSendingOff;
	TestEqual(TEXT("ServerInternalAction denied by shared port"), Runtime.SubmitPlayerIntent(Internal).ErrorCode,
		EMatchPlayPlayerIntentPortErrorCode::NotPlayerIntent);
	Internal.CommandKind = EMatchPlayAuthoritativeCommandKind::RequestInitialActionPointRoll;
	TestEqual(TEXT("Mismatched variant denied by shared port"), Runtime.SubmitPlayerIntent(Internal).ErrorCode,
		EMatchPlayPlayerIntentPortErrorCode::PayloadTypeMismatch);
	TestEqual(TEXT("Port rejects before RNG"), Runtime.GetEntryProviderInvocationCount(), 0);
	return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexNetworkSecureEntropyAckFailureTest,
	"FMCodex.NetworkPlay.RNGPrivacy.07.EntropyFailureAckAndDuplicate",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexNetworkSecureEntropyAckFailureTest::RunTest(const FString&)
{
	using namespace FMCodexNetworkPlayerIntentTests;
	FFixture F;
	auto Source = MakeUnique<FFMCodexNetworkScriptedEntropy>();
	auto* Calls = Source.Get();
	TestTrue(TEXT("Secure provider with failing injected entropy initializes"),
		Access::UseEntropy(*F.Mode, MoveTemp(Source)));
	const auto Before = Access::Session(*F.Mode).GetStateSnapshot();
	const int32 Revision = Access::Revision(*F.Mode);
	const auto Request = F.Request();
	const auto Ack = F.Mode->SubmitConnectionPlayerIntent(F.A, Request);
	TestEqual(TEXT("Typed rejection from real secure-provider failure"), Ack.Code, Code::AuthorityRejected);
	TestEqual(TEXT("ACK retains request"), Ack.RequestId, Request.RequestId);
	TestEqual(TEXT("ACK retains match"), Ack.MatchInstanceId, Request.MatchInstanceId);
	TestEqual(TEXT("No extra publication"), Access::Revision(*F.Mode), Revision);
	TestTrue(TEXT("Whole state preserved"), SameState(Before, Access::Session(*F.Mode).GetStateSnapshot()));
	TestEqual(TEXT("Duplicate rejected"), F.Mode->SubmitConnectionPlayerIntent(F.A, Request).Code, Code::DuplicateOrAlreadyResolved);
	TestEqual(TEXT("No second entropy call"), Calls->Calls, 1);
	TestEqual(TEXT("New-ID retry is a new attempt"), F.Mode->SubmitConnectionPlayerIntent(F.A, F.Request(2)).Code, Code::AuthorityRejected);
	TestEqual(TEXT("No automatic retries"), Calls->Calls, 2);
	return true;
}
#endif
