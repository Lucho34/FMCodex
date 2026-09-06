#if WITH_DEV_AUTOMATION_TESTS
#include "FMCodexNetworkPlayerFacingTestFixture.h"
#include "FMCodexNetworkOptionalDeclineTestFixture.h"
IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFMCodexOptionalDeclineUI,"FMCodex.NetworkPlay.OptionalDeclineUI.SharedControls",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
void FFMCodexOptionalDeclineUI::GetTests(TArray<FString>& N,TArray<FString>& C) const
{
 for(const TCHAR* S:{TEXT("A"),TEXT("B")})for(const TCHAR* K:{TEXT("Runner"),TEXT("Helper"),TEXT("Skill"),TEXT("Marker")})for(const TCHAR* End:{TEXT("Next"),TEXT("Final")})
 {const auto P=FString::Printf(TEXT("%s.%s.%s"),S,K,End);N.Add(P);C.Add(P);}
}
bool FFMCodexOptionalDeclineUI::RunTest(const FString& P)
{
 using namespace FMCodexPlayerFacingOrdinaryUITests;
 using namespace FMCodexOptionalDeclineTests;
 TArray<FString> Parts;P.ParseIntoArray(Parts,TEXT("."));const Kind K=Command(Parts[1]);const bool Final=Parts[2]==TEXT("Final");
 FUIFixture F(((Parts[0]==TEXT("B"))!=(K==Kind::DeclineHelper||K==Kind::DeclineMarker))!=Final,Final);
 if(!TestTrue(TEXT("Canonical milestone"),Prepare(F,K,Final)))return false;if(!F.A->GetOwnerView().bMatchEnded)F.Settle();
 auto* PC=Actor(F,K);auto* Other=PC==F.A?F.B:F.A;auto* S=PC->GetPlayerMatchScreen();
 const auto BeforeView=PC->GetOwnerView();const auto BeforeModel=S->GetPresentation();
 TestTrue(TEXT("Existing Local screen reused"),S->GetClass()==UFMCodexLocalMatchScreenWidget::StaticClass());
 TestTrue(TEXT("Canonical actor has positive choices and decline"),BeforeModel.Interaction.bCanDecline&&!BeforeModel.Interaction.SelectionChoices.IsEmpty());
 TestFalse(TEXT("Waiting viewer has no decline"),Other->GetPlayerMatchScreen()->GetPresentation().Interaction.bCanDecline);
 auto* Button=Cast<UButton>(S->GetInteractionPanel()->GetWidgetFromName(TEXT("InteractionDeclineButton")));
 TestEqual(TEXT("Existing shared decline button visible"),Button->GetVisibility(),ESlateVisibility::Visible);
 const FString Label=K==Kind::DeclineMarker?TEXT("放弃盯人"):K==Kind::DeclineRunner?TEXT("不选择跑位球员"):K==Kind::DeclineHelper?TEXT("放弃协防"):TEXT("不使用战术");
 TestEqual(TEXT("Existing Chinese label"),Cast<UTextBlock>(Button->GetChildAt(0))->GetText().ToString(),Label);
 const FString Prompt=K==Kind::DeclineMarker?TEXT("选择盯人球员"):K==Kind::DeclineRunner?TEXT("选择跑位球员"):K==Kind::DeclineHelper?TEXT("选择协防球员"):TEXT("选择战术");
 CheckBothPrompts(*this,F,Prompt);
 // Hold a request before transport to inspect the same generic pending read adapter.
 Envelope Pending;TestTrue(TEXT("Generic pending begins"),F.Client(PC).BeginDecline(BeforeView,K,Pending));
 const FUnchanged Unchanged(F);
 S->RefreshFromPresentation(FFMCodexNetworkMatchPresentationAdapter::Read(BeforeView,true));
 const auto& PendingModel=S->GetPresentation();
 TestFalse(TEXT("Pending disables decline"),PendingModel.Interaction.bCanDecline);
 TestEqual(TEXT("Pending keeps authoritative candidates"),PendingModel.Interaction.SelectionChoices.Num(),BeforeModel.Interaction.SelectionChoices.Num());
 for(int32 I=0;I<PendingModel.Interaction.SelectionChoices.Num();++I)
 {TestFalse(TEXT("Pending option disabled"),PendingModel.Interaction.SelectionChoices[I].bEnabled);TestEqual(TEXT("Pending retains exact safe ID"),PendingModel.Interaction.SelectionChoices[I].OptionId,BeforeModel.Interaction.SelectionChoices[I].OptionId);}
 const int32 Sends=F.Backend(PC).Sends;S->RequestDeclineSelection();TestEqual(TEXT("Duplicate UI click sends nothing"),F.Backend(PC).Sends,Sends);Unchanged.Verify(*this,F);
 auto Bad=Pending;Bad.IntentKind=K==Kind::DeclineRunner?Kind::DeclineHelper:Kind::DeclineRunner;
 const auto Reject=F.Mode->SubmitConnectionPlayerIntent(PC,Bad);F.Client(PC).ObserveAck(Reject);PC->RefreshPlayerFacingUI();
 TestEqual(TEXT("Wrong phase rejection"),Reject.Code,Code::InvalidPhase);TestTrue(TEXT("Rejection restores safe decline"),S->GetPresentation().Interaction.bCanDecline);
 const FUnchanged Before(F);S->RequestDeclineSelection();
 TestEqual(TEXT("Shared callback sends once"),F.Backend(PC).Sends,Sends+1);TestEqual(TEXT("Exact typed decline"),F.Backend(PC).Last.IntentKind,K);TestEqual(TEXT("Accepted"),F.Backend(PC).LastCode,Code::Accepted);
 Adopted(*this,F,K,Before);if(!F.A->GetOwnerView().bMatchEnded)F.Settle();
 if(K!=Kind::DeclineSkill&&K!=Kind::DeclineMarker)
 {
  CheckBothPrompts(*this,F,TEXT("选择战术"));auto* Attacker=F.Attacker();
  Attacker->GetPlayerMatchScreen()->RequestDeclineSelection();TestEqual(TEXT("Next genuine Skill decline"),F.Backend(Attacker).Last.IntentKind,Kind::DeclineSkill);TestEqual(TEXT("Finish canonically"),F.Backend(Attacker).LastCode,Code::Accepted);if(!F.A->GetOwnerView().bMatchEnded)F.Settle();
 }
 for(auto* Viewer:{F.A,F.B})
 {
  const auto& M=Viewer->GetPlayerMatchScreen()->GetPresentation();
  TestEqual(TEXT("Final decline reaches shared Full Time directly"),M.FullTime.bVisible,Final);
  TestFalse(TEXT("No invented decline terminal Advance"),Viewer->GetOwnerView().bCanAdvance);
  TestTrue(TEXT("No stale optional candidates"),M.Interaction.SelectionChoices.IsEmpty());
  TestFalse(TEXT("No stale decline CTA"),M.Interaction.bCanDecline);
  if(K==Kind::DeclineMarker)
  {
   TestTrue(TEXT("No fake accepted roll"),M.ResolvedRolls.IsEmpty());
   TestEqual(TEXT("No-roll Goal notice shares first stable score refresh"),M.Resolution.bVisible,!Final);
   if(!Final){TestTrue(TEXT("Existing nonblocking surface"),M.Resolution.bNonBlockingNotification);TestEqual(TEXT("Goal title"),M.Resolution.StepLabel,FString(TEXT("进球")));TestTrue(TEXT("Authoritative system award label"),M.Resolution.StepSummaryLabel.Contains(TEXT("规则判定进球")));}
   const auto Summary=M.Resolution.StepSummaryLabel;Viewer->RefreshPlayerFacingUI();TestEqual(TEXT("Duplicate refresh remains same notification"),Viewer->GetPlayerMatchScreen()->GetPresentation().Resolution.StepSummaryLabel,Summary);
   TestEqual(TEXT("Early closure has no Recovery"),Viewer->GetOwnerView().Recovery.SourceAttackSequence,int64(0));
  }
  if(Final)TestFalse(TEXT("No final D12"),M.Interaction.bCanRollTacticalPoints);
 }
 if(Final&&K==Kind::DeclineMarker)
 {
  const FUnchanged Ended(F);auto Fresh=Request(F,PC,K);Fresh.RequestId=100;
  TestEqual(TEXT("Fresh Marker decline after MatchEnded rejected"),F.Mode->SubmitConnectionPlayerIntent(PC,Fresh).Code,Code::InvalidPhase);Ended.Verify(*this,F);
 }
 if(!Final)
 {
  CheckBothPrompts(*this,F,TEXT("掷战术点"));auto* Next=F.Attacker();
  TestTrue(TEXT("Next attacker has canonical D12"),Next->GetPlayerMatchScreen()->GetPresentation().Interaction.bCanRollTacticalPoints);
  Next->GetPlayerMatchScreen()->RequestRollTacticalPoints();TestEqual(TEXT("Existing next D12 transport"),F.Backend(Next).LastCode,Code::Accepted);
 }
 return true;
}
#endif
