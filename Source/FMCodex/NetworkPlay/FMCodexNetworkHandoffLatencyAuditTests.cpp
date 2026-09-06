#include "../Diagnostics/FMCodexHandoffLatencyAudit.h"
#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "Misc/AutomationTest.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
using namespace FMCodexHandoffAudit;
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexHandoffTraceFlagTest,"FMCodex.NetworkPlay.PlayerFacingHandoffLatency.FlagAndScope",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FFMCodexHandoffTraceFlagTest::RunTest(const FString&)
{
 TestFalse(TEXT("Default is silent"),EnabledForCommandLine(TEXT("-game")));
 TestTrue(TEXT("Explicit opt in"),EnabledForCommandLine(TEXT("-game -HandoffLatencyAudit")));
 TestFalse(TEXT("Other flag cannot enable audit"),EnabledForCommandLine(TEXT("-HandoffLatencyAuditOther")));
 TArray<FString> Lines; SetTestCapture(&Lines,false);
 FContext C; C.Request=7;
 { FServerScope Scope(C); AuthorityAccepted(); TestNull(TEXT("Disabled has no active context"),ServerContext()); }
 TestEqual(TEXT("Disabled emits nothing"),Lines.Num(),0);
 SetTestCapture(&Lines,true);
 { FServerScope Outer(C); AuthorityAccepted(); C.Request=8;
  { FServerScope Inner(C); TestEqual(TEXT("Inner context"),ServerContext()->Request,int64(8)); }
  TestEqual(TEXT("Restored outer context"),ServerContext()->Request,int64(7)); }
 TestNull(TEXT("Scope does not leak into LocalPlay"),ServerContext());
 TestEqual(TEXT("One accepted event"),Lines.Num(),1);
 SetTestCapture(nullptr,false); return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexHandoffTraceObserverTest,"FMCodex.NetworkPlay.PlayerFacingHandoffLatency.ObserverCorrelationAndDedupe",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FFMCodexHandoffTraceObserverTest::RunTest(const FString&)
{
 TArray<FString> Lines; SetTestCapture(&Lines,true);
 FContext C; C.Match=FGuid::NewGuid(); C.Sequence=1; C.Revision=7; C.Viewer=2; C.Target=2;
 FPresentationObserver Observer;
 Observer.ViewApplied(C,TEXT(",\"SafeExpectedSide\":2"));
 Observer.Observe(3,TEXT("Cycling"),true,false,false,TEXT(""));
 Observer.Observe(3,TEXT("Cycling"),true,true,true,TEXT(""));
 Observer.Observe(5,TEXT("ResultHold"),true,false,false,TEXT(""));
 Observer.Observe(6,TEXT("Settled"),false,false,false,TEXT(""));
 Observer.Observe(6,TEXT("Settled"),false,true,false,TEXT(""));
 TestEqual(TEXT("Not visible does not emit T5"),Lines.Num(),5);
 Observer.Observe(6,TEXT("Settled"),false,true,true,TEXT(",\"ActorPrompt\":\"轮到你操作\""));
 const int32 Count=Lines.Num();
 Observer.ViewApplied(C,TEXT(""));
 Observer.Observe(6,TEXT("Settled"),false,true,true,TEXT(""));
 --C.Revision; Observer.ViewApplied(C,TEXT(""));
 TestEqual(TEXT("Repeated and stale View do not duplicate T5"),Lines.Num(),Count);
 TestTrue(TEXT("Actual last blocking phase retained"),Lines[4].Contains(TEXT("\"PreviousBlockReason\":\"ResultHold\"")));
 for(const FString& Line:Lines)
 {
  TSharedPtr<FJsonObject> Json; TestTrue(TEXT("Machine readable JSON"),FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Line),Json));
  if(Json) { TestEqual(TEXT("Safe revision correlation"),int32(Json->GetNumberField(TEXT("ViewRevision"))),7);
   TestEqual(TEXT("No invented replicated request ID"),int64(Json->GetNumberField(TEXT("RequestId"))),int64(0));
   TestTrue(TEXT("UTC present"),Json->HasField(TEXT("UtcTicks"))); TestTrue(TEXT("Mono present"),Json->HasField(TEXT("MonoSeconds"))); }
 }
 TestTrue(TEXT("CTA emission requires no other-client ACK"),Lines.Last().Contains(TEXT("T5.ActionPromptVisible")));
 SetTestCapture(nullptr,false); return true;
}

#include "../LocalPlay/FMCodexLocalMatchScreenWidget.h"
#include "Engine/World.h"
struct FFMCodexHandoffTimerTestAccess
{
 static void BeginHold(UFMCodexLocalMatchScreenWidget& S)
 {
  S.InlineFormulaRevealPhase=EFMCodexUMGInlineFormulaRevealPhase::ResultHold;
  S.InlineFormulaRevealPhaseElapsed=0.f;
  S.ActiveCrossRollReveal.Kind=EFMCodexUMGCrossRollRevealKind::Attack;
  S.StartInlineFormulaRevealTimer();
 }
 static void Wake(UFMCodexLocalMatchScreenWidget& S) { S.HandleInlineFormulaRevealTimer(); }
};
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexHandoffHoldClockTest,"FMCodex.NetworkPlay.PlayerFacingHandoffLatency.HoldUsesElapsedGameTime",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FFMCodexHandoffHoldClockTest::RunTest(const FString&)
{
 // Synthetic presentation-only clock: no Session, RNG, network latency or gameplay result is mocked.
 // Sparse callbacks reproduce a 40 ms one-shot timer serviced every 100 ms.
 UWorld* World=UWorld::CreateWorld(EWorldType::Game,false);
 auto* Screen=NewObject<UFMCodexLocalMatchScreenWidget>(World);
 FFMCodexHandoffTimerTestAccess::BeginHold(*Screen);
 const double Start=World->GetTimeSeconds();
 for(int32 I=1;I<=25;++I)
 {
  World->TimeSeconds=Start+I*0.1;
  FFMCodexHandoffTimerTestAccess::Wake(*Screen);
 }
 TestEqual(TEXT("Does not shorten the configured 2.58 s hold"),Screen->GetInlineFormulaRevealPhase(),EFMCodexUMGInlineFormulaRevealPhase::ResultHold);
 for(int32 I=0;I<10;++I) FFMCodexHandoffTimerTestAccess::Wake(*Screen);
 TestEqual(TEXT("No elapsed game time means no hold progression"),Screen->GetInlineFormulaRevealPhase(),EFMCodexUMGInlineFormulaRevealPhase::ResultHold);
 World->TimeSeconds=Start+2.6;
 FFMCodexHandoffTimerTestAccess::Wake(*Screen);
 TestFalse(TEXT("Late callbacks do not extend the configured hold"),Screen->IsInlineFormulaRevealInputBlocked());
 World->DestroyWorld(false); return true;
}

#endif
