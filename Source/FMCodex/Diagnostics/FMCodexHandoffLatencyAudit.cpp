#include "FMCodexHandoffLatencyAudit.h"
#if !UE_BUILD_SHIPPING
#include "HAL/PlatformProcess.h"
#include "HAL/PlatformTime.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
namespace FMCodexHandoffAudit
{
	namespace
	{
		thread_local const FContext* ActiveServerContext = nullptr;
#if WITH_DEV_AUTOMATION_TESTS
		TArray<FString>* TestSink = nullptr;
		bool bTestEnabled = false;
#endif
	}
	bool EnabledForCommandLine(const TCHAR* CommandLine) { return FParse::Param(CommandLine,TEXT("HandoffLatencyAudit")); }
	bool Enabled()
	{
#if WITH_DEV_AUTOMATION_TESTS
		if (TestSink) return bTestEnabled;
#endif
		static const bool bEnabled = EnabledForCommandLine(FCommandLine::Get());
		return bEnabled;
	}
	FString Quote(const FString& V) { return TEXT("\"") + V.ReplaceCharWithEscapedChar() + TEXT("\""); }
	void Emit(const TCHAR* Event, const FContext& C, const FString& Fields)
	{
		if (!Enabled()) return;
		const int64 Utc = FDateTime::UtcNow().GetTicks();
		const double Mono = FPlatformTime::Seconds();
		const FString Json = FString::Printf(TEXT("{\"Event\":%s,\"Match\":%s,\"AttackSequence\":%lld,\"IntentKind\":%s,\"RequestId\":%lld,\"ViewRevision\":%d,\"SourceSide\":%d,\"ViewerSide\":%d,\"TargetSide\":%d,\"Role\":%s,\"PID\":%u,\"UtcTicks\":%lld,\"MonoSeconds\":%.9f%s}"),
			*Quote(Event),*Quote(C.Match.ToString(EGuidFormats::DigitsWithHyphensLower)),C.Sequence,*Quote(C.Intent),C.Request,C.Revision,C.Source,C.Viewer,C.Target,*Quote(C.Role),FPlatformProcess::GetCurrentProcessId(),Utc,Mono,*Fields);
#if WITH_DEV_AUTOMATION_TESTS
		if (TestSink) { TestSink->Add(Json); return; }
#endif
		UE_LOG(LogTemp,Log,TEXT("FMCODEX_HANDOFF %s"),*Json);
	}
	const FContext* ServerContext() { return Enabled() ? ActiveServerContext : nullptr; }
	FServerScope::FServerScope(const FContext& In) : Context(In)
	{
		bActive=Enabled();
		if(bActive) { Previous=ActiveServerContext; ActiveServerContext=&Context; }
	}
	FServerScope::~FServerScope() { if(bActive) ActiveServerContext=Previous; }
	void AuthorityAccepted() { if(const auto* C=ServerContext()) Emit(TEXT("T1A.ServerAccepted"),*C); }
	void FPresentationObserver::ViewApplied(const FContext& In,const FString& Fields)
	{
		if(!Enabled()) return;
		if(bHasView && Context.Match==In.Match && In.Revision<=Context.Revision) return;
		Context=In; bHasView=true; LastPhase=INDEX_NONE; bReleased=bVisibleEmitted=false; LastBlockReason=TEXT("None");
		Emit(TEXT("T3.TargetClientViewApplied"),Context,Fields);
	}
	void FPresentationObserver::Observe(int32 Phase,const FString& Reason,bool bBlocked,bool bAfterRender,bool bVisible,const FString& Fields)
	{
		if(!Enabled() || !bHasView) return;
		const FString State=FString::Printf(TEXT(",\"Phase\":%d,\"BlockReason\":%s,\"Blocked\":%s"),Phase,*Quote(Reason),bBlocked?TEXT("true"):TEXT("false"));
		if(!bAfterRender)
		{
			if(LastPhase!=Phase)
			{
				LastPhase=Phase;
				Emit(TEXT("PresentationPhase"),Context,State);
			}
			if(bBlocked) LastBlockReason=Reason;
			if(!bBlocked && !bReleased)
			{
				bReleased=true;
				Emit(TEXT("T4.PresentationGateReleased"),Context,State+TEXT(",\"PreviousBlockReason\":")+Quote(LastBlockReason));
			}
		}
		else if(bReleased && !bBlocked && bVisible && !bVisibleEmitted)
		{
			bVisibleEmitted=true;
			Emit(TEXT("T5.ActionPromptVisible"),Context,State+Fields);
		}
	}
#if WITH_DEV_AUTOMATION_TESTS
	void SetTestCapture(TArray<FString>* Sink,bool bEnable) { TestSink=Sink; bTestEnabled=bEnable; }
#endif
}
#endif
