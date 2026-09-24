#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && !UE_BUILD_SHIPPING
#include "FMCodexInlineResolutionFormulaSurfaceWidget.h"
#include "FMCodexLocalMatchPlayerController.h"
#include "FMCodexLocalMatchScreenWidget.h"
#include "Editor.h"
#include "Components/Button.h"
#include "UObject/UnrealType.h"
#include "HAL/IConsoleManager.h"
#include "Engine/World.h"
#include "Engine/GameViewportClient.h"
#include "Framework/Application/SlateApplication.h"
#include "HAL/FileManager.h"
#include "ImageUtils.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "PlayInEditorDataTypes.h"
#include "Settings/LevelEditorPlaySettings.h"
#include "Tests/AutomationCommon.h"
#include "Tests/AutomationEditorCommon.h"
#include "Widgets/SWindow.h"
namespace
{
 TSharedPtr<SWindow> BroadcastPIEWindow;

	bool DeployNextOrdinary(
		AFMCodexLocalMatchPlayerController& Controller,
		const FString& SlotFragment)
	{
		Controller.RefreshPresentation();
		const FFMCodexLocalMatchInteractionView& View =
			Controller.GetInteractionView();
		const bool bDeployingAttacker =
			View.CurrentLegalDeploymentSide == View.CurrentAttackingPlayer;
		FName PreferredCardId = NAME_None;
		if (bDeployingAttacker)
		{
			const FName RequiredCarrierId = View.CurrentAttackingPlayer
				== EInitialTurnOrderPlayer::PlayerA
					? FName(TEXT("Prototype.Arsenal.BukayoSaka"))
					: FName(TEXT("Prototype.ManchesterCity.RayanAitNouri"));
			const FName RequiredRunnerId = View.CurrentAttackingPlayer
				== EInitialTurnOrderPlayer::PlayerA
					? FName(TEXT("Prototype.Arsenal.ViktorGyokeres"))
					: FName(TEXT("Prototype.ManchesterCity.ErlingHaaland"));
			const TArray<FFMCodexLocalMatchCardView>& Roster =
				View.CurrentAttackingPlayer == EInitialTurnOrderPlayer::PlayerA
					? View.PlayerACardRoster : View.PlayerBCardRoster;
			const bool bCarrierDeployed = Roster.ContainsByPredicate(
				[RequiredCarrierId](const FFMCodexLocalMatchCardView& Card)
				{
					return Card.bDeployed && Card.CardId == RequiredCarrierId;
				});
			PreferredCardId = bCarrierDeployed
				? RequiredRunnerId : RequiredCarrierId;
		}

		else
		{
			// Legal, reference-comparable participants; only existing deployment intents.
			const bool bDefenderA = View.CurrentLegalDeploymentSide == EInitialTurnOrderPlayer::PlayerA;
			const FName Marker = bDefenderA ? FName(TEXT("Prototype.Arsenal.WilliamSaliba")) : FName(TEXT("Prototype.ManchesterCity.JohnStones"));
			const FName Helper = bDefenderA ? FName(TEXT("Prototype.Arsenal.BenWhite")) : FName(TEXT("Prototype.ManchesterCity.NathanAke"));
			const auto& Roster = bDefenderA ? View.PlayerACardRoster : View.PlayerBCardRoster;
			PreferredCardId = Roster.ContainsByPredicate([Marker](const FFMCodexLocalMatchCardView& Card)
				{ return Card.bDeployed && Card.CardId==Marker; }) ? Helper : Marker;
		}

		const FFMCodexLocalMatchDeploymentOption* Option =
			View.DeploymentOptions.FindByPredicate(
				[PreferredCardId, &SlotFragment](
					const FFMCodexLocalMatchDeploymentOption& Candidate)
				{
					return !Candidate.bGoalkeeper
						&& Candidate.SlotId.ToString().Contains(SlotFragment)
						&& (PreferredCardId.IsNone()
							|| Candidate.CardId == PreferredCardId);
				});
		if (Option == nullptr && !PreferredCardId.IsNone())
		{
			Option = View.DeploymentOptions.FindByPredicate(
				[&SlotFragment](
					const FFMCodexLocalMatchDeploymentOption& Candidate)
				{
					return !Candidate.bGoalkeeper
						&& Candidate.SlotId.ToString().Contains(SlotFragment);
				});
		}
		if (Option == nullptr)
		{
			return false;
		}
		Controller.DeployOrdinary(Option->CardId, Option->SlotId);
		return Controller.GetLastDiagnostic().bHostSuccess;
	}

	bool SubmitFirst(
		AFMCodexLocalMatchPlayerController& Controller,
		const EFMCodexLocalMatchInteractionCategory Expected, FName PreferredId = NAME_None)
	{
		Controller.RefreshPresentation();
		const FFMCodexLocalMatchInteractionView& View =
			Controller.GetInteractionView();
		if (View.InteractionCategory != Expected
			|| View.SelectionOptions.IsEmpty())
		{
			return false;
		}
		const auto* Preferred = View.SelectionOptions.FindByPredicate([PreferredId](const auto& Option) { return Option.Id == PreferredId; });
		const FName Id = Preferred ? Preferred->Id : View.SelectionOptions[0].Id;
		switch (Expected)
		{
		case EFMCodexLocalMatchInteractionCategory::SelectMarker:
			Controller.SubmitMarker(Id);
			break;
		case EFMCodexLocalMatchInteractionCategory::SelectRunner:
			Controller.SubmitRunner(Id);
			break;
		case EFMCodexLocalMatchInteractionCategory::SelectHelper:
			Controller.SubmitHelper(Id);
			break;
		default:
			return false;
		}
		return Controller.GetLastDiagnostic().bHostSuccess;
	}


class FStartBroadcastPIE final : public IAutomationLatentCommand
{
public:
 bool Update() override
 {
  auto* Settings=DuplicateObject<ULevelEditorPlaySettings>(GetDefault<ULevelEditorPlaySettings>(),GetTransientPackage());
  Settings->NewWindowWidth=1920; Settings->NewWindowHeight=1080;
  Settings->SetPlayNetMode(EPlayNetMode::PIE_Standalone); Settings->SetPlayNumberOfClients(1);
  BroadcastPIEWindow = SNew(SWindow).Title(FText::FromString(TEXT("Stage 8.8C PIE")))
   .ClientSize(FVector2D(1920,1080)).ScreenPosition(FVector2D(0,0)).AutoCenter(EAutoCenter::None)
   .SaneWindowPlacement(false).AdjustInitialSizeAndPositionForDPIScale(false).SizingRule(ESizingRule::UserSized);
  // Under -RenderOffscreen Slate still needs a shown logical window to arrange
  // the gameplay viewport. A hidden SWindow captures unarranged HUD geometry.
  FSlateApplication::Get().AddWindow(BroadcastPIEWindow.ToSharedRef());
  FRequestPlaySessionParams Params; Params.EditorPlaySettings=Settings; Params.CustomPIEWindow=BroadcastPIEWindow;
  GEditor->RequestPlaySession(Params); return true;
 }
};
class FBroadcastPIE final : public IAutomationLatentCommand
{
public:
 explicit FBroadcastPIE(FAutomationTestBase* InTest):Test(InTest)
 { Mode=IConsoleManager::Get().FindConsoleVariable(TEXT("fm.UI.FormulaV2")); PreviousMode=Mode->GetInt(); Mode->Set(0,ECVF_SetByCode); }
 ~FBroadcastPIE() { Mode->Set(PreviousMode,ECVF_SetByCode); }
 bool Update() override
 {
  if (FPlatformTime::Seconds()-Start>120) { Test->AddError(TEXT("Broadcast prototype PIE timed out")); return true; }
  if (!GEditor || !GEditor->PlayWorld) return false;
  auto* C=Cast<AFMCodexLocalMatchPlayerController>(GEditor->PlayWorld->GetFirstPlayerController());
  auto* S=C ? C->GetPlayerMatchScreen() : nullptr;
  if (!S || FPlatformTime::Seconds()-Changed<1.0) return false;
  if (Step==0) { S->RequestStartNewMatch(); Next(); return false; }
  if (Step==1) { if (!Override(*C,EFMCodexLocalDevRollTarget::FullD12,6)) return true; S->RequestRollTacticalPoints(); Next(); return false; }
  if (S->IsInlineFormulaRevealInputBlocked()) return false; // Natural game-time reveal only.
  if (Step==2)
  {
   const auto Attacker=C->GetInteractionView().CurrentAttackingPlayer;
   const FString Forward=Attacker==EInitialTurnOrderPlayer::PlayerA ? TEXT("NearB") : TEXT("NearA");
   for (int32 I=0;I<4;++I) if (!DeployNextOrdinary(*C,Forward)) { Test->AddError(TEXT("Legal deployment failed")); return true; }
   C->FinishDeployment(); C->FinishDeployment();
   S->RequestSubmitCarrier(Attacker==EInitialTurnOrderPlayer::PlayerA ? FName(TEXT("Prototype.Arsenal.BukayoSaka")) : FName(TEXT("Prototype.ManchesterCity.RayanAitNouri")));
   if (!SubmitFirst(*C,EFMCodexLocalMatchInteractionCategory::SelectMarker,Attacker==EInitialTurnOrderPlayer::PlayerA ? FName(TEXT("Prototype.ManchesterCity.JohnStones")) : FName(TEXT("Prototype.Arsenal.WilliamSaliba")))
    || !SubmitFirst(*C,EFMCodexLocalMatchInteractionCategory::SelectRunner)) { Test->AddError(TEXT("Legal role selection failed")); return true; }
   if (!C->GetInteractionView().SelectionOptions.IsEmpty())
   {
    if (!SubmitFirst(*C,EFMCodexLocalMatchInteractionCategory::SelectHelper)) { Test->AddError(TEXT("Legal helper selection failed")); return true; }
   }
   else if (C->GetInteractionView().bCanResolveNoLegalChoice) C->ResolveNoLegalCurrentSelection();
   else C->DeclineCurrentSelection();
   C->SubmitSkill(TEXT("Canonical.Skill.Cross.4.6"));
   C->SubmitBranchIntent(EMatchPlayElectiveBranchIntent::CrossHigh);
   if (!Test->TestTrue(TEXT("Natural High Cross route entry"),C->GetLastDiagnostic().bHostSuccess
    && C->GetInteractionView().InteractionCategory==EFMCodexLocalMatchInteractionCategory::RollCrossRoute)) return true;
   if (!Override(*C,EFMCodexLocalDevRollTarget::CrossRoute,2)
    || !Override(*C,EFMCodexLocalDevRollTarget::CrossHighAttack,4)
    || !Override(*C,EFMCodexLocalDevRollTarget::CrossHighDefense,3)) return true;
   Next(); return false;
  }
  if (Step==3) { S->RequestContinueResolution(); Next(); return false; }
  auto* F=S->GetInlineFormulaSurface(); const auto& P=F->GetPresentation();
  if (Step==4)
  {
   FString SafeFormula;
   FFMCodexUMGInlineFormulaSurfaceViewModel::StaticStruct()->ExportText(SafeFormula,&P,nullptr,nullptr,PPF_None,nullptr);
   const auto& V=C->GetInteractionView();
   const FString MatchIdentity=FString::Printf(TEXT("%lld/%d/%d/%d/%d/%d/%d/%d"),V.AttackSequence,V.ActionPoint,
    static_cast<int32>(V.CurrentAttackingPlayer),static_cast<int32>(V.ExpectedActingPlayer),
    static_cast<int32>(V.InteractionCategory),static_cast<int32>(V.RouteKind),V.PlayerAScore,V.PlayerBScore);
   if (CompareStep==0)
   {
    ComparedFormula=SafeFormula; ComparedIdentity=MatchIdentity;
    Test->TestFalse(TEXT("Comparison begins in accepted V1"),F->IsBroadcastPrototypeVisible());
    Capture(TEXT("HighCross_V1_Attack.png"));
   }
   else
   {
    Test->TestEqual(TEXT("A/B preserves all safe Formula facts, values, roles, action and reveal gates"),SafeFormula,ComparedFormula);
    Test->TestEqual(TEXT("A/B preserves sequence, ownership, route, score and typed next action"),MatchIdentity,ComparedIdentity);
    Test->TestEqual(TEXT("Native tick switches the visible subtree in both directions"),F->IsBroadcastPrototypeVisible(),CompareStep!=2);
   }
   if (CompareStep<3)
   {
    Mode->Set(CompareStep==1 ? 0 : 1,ECVF_SetByCode);
    ++CompareStep; Changed=FPlatformTime::Seconds(); return false;
   }
  }
  using E=EFMCodexFormulaEmphasis;
  if (Step>=4 && Step<=6)
  {
   const E A=Step==4 ? E::Active : Step==5 ? E::Context : E::Resolved;
   const E D=Step==4 ? E::Context : Step==5 ? E::Active : E::Resolved;
   if (!Test->TestTrue(TEXT("Real High Cross A -> D -> resolved visual owners"),P.ContestId==TEXT("Cross.High")
     && F->GetRowEmphasis(true)==A && F->GetRowEmphasis(false)==D)) return true;
   Test->TestTrue(TEXT("Real high-density facts include both attack participants and full defense context"),
    P.AttackRow.Participants.Num()==2 && P.DefenseRow.Participants.Num()>=2 && P.DefenseRow.Terms.Num()>=4);
   Test->AddInfo(FString::Printf(TEXT("BROADCAST_PIE step=%d A=%d D=%d values=%s/%s game=%.3f"),Step,
    static_cast<int32>(A),static_cast<int32>(D),*P.AttackRow.DisplayedResultLabel,*P.DefenseRow.DisplayedResultLabel,GEditor->PlayWorld->GetTimeSeconds()));
   Test->TestTrue(TEXT("Runtime opt-in only displays prototype"),F->IsBroadcastPrototypeVisible());
   Capture(Step==4 ? TEXT("HighCross_V2_Attack.png") : Step==5 ? TEXT("HighCross_V2_Defense.png") : TEXT("HighCross_V2_Resolved.png"));
   auto* Button=CastChecked<UButton>(F->GetWidgetFromName(TEXT("BroadcastContinue")));
   if (!Test->TestTrue(TEXT("Same projected action gate is enabled"),Button->GetIsEnabled() && P.PrimaryAction.bVisible)) return true;
   Button->OnClicked.Broadcast(); Next(); return false;
  }
  Test->TestTrue(TEXT("Original CTA completes natural terminal handoff"),C->GetLastDiagnostic().bHostSuccess && !C->GetInteractionView().bTerminalPendingAdvance);
  return true;
 }
private:
 void Next() { ++Step; Changed=FPlatformTime::Seconds(); }
 bool Override(AFMCodexLocalMatchPlayerController& C,EFMCodexLocalDevRollTarget Target,int32 Value)
 {
  FFMCodexLocalDevRollOverrideRequest R; R.Target=Target; R.Value=Value;
  return Test->TestTrue(TEXT("Existing DEV provider override accepted"),C.SetLocalDevRollOverride(R).bSuccess);
 }
 void Capture(const TCHAR* File)
 {
  // Entire real PIE window at actual gameplay size. No widget reparenting/resizing,
  // fabricated Formula, altered phase or manual reveal-clock advancement.
  auto Window=GEditor->PlayWorld->GetGameViewport()->GetWindow();
  if (!Window.IsValid()) { Test->AddError(TEXT("PIE window missing")); return; }
  auto* Controller=CastChecked<AFMCodexLocalMatchPlayerController>(GEditor->PlayWorld->GetFirstPlayerController());
  auto* Surface=Controller->GetPlayerMatchScreen()->GetInlineFormulaSurface();
  const auto& SurfaceGeometry=Surface->GetCachedGeometry();
  const FVector2D SurfaceSize=SurfaceGeometry.GetAbsoluteSize();
  if (!Test->TestTrue(TEXT("Formula is laid out at readable real viewport size"),SurfaceSize.X>300 && SurfaceSize.Y>150)) return;
  if (Surface->IsBroadcastPrototypeVisible())
  {
   const FVector2D SurfaceStart=SurfaceGeometry.GetAbsolutePosition(), SurfaceEnd=SurfaceStart+SurfaceSize;
   for (const TCHAR* Name : {TEXT("BroadcastHeader"),TEXT("BroadcastAttackCard"),TEXT("BroadcastDefenseCard"),TEXT("BroadcastActionBounds")})
   {
    const auto& Geometry=Surface->GetWidgetFromName(Name)->GetCachedGeometry();
    const FVector2D ChildStart=Geometry.GetAbsolutePosition(), ChildEnd=ChildStart+Geometry.GetAbsoluteSize();
    Test->TestTrue(FString::Printf(TEXT("%s stays inside the real pitch overlay allocation"),Name),
     ChildStart.X>=SurfaceStart.X-1 && ChildStart.Y>=SurfaceStart.Y-1 && ChildEnd.X<=SurfaceEnd.X+1 && ChildEnd.Y<=SurfaceEnd.Y+1);
   }
   Test->TestTrue(TEXT("Cards retain broadcast width after viewport fit"),
    Surface->GetWidgetFromName(TEXT("BroadcastAttackCard"))->GetCachedGeometry().GetAbsoluteSize().X>=SurfaceSize.X*.88f);
   Test->TestTrue(TEXT("Action context retains readable type after ownership changes"),
    Surface->GetWidgetFromName(TEXT("BroadcastActionContext"))->GetCachedGeometry().GetAbsoluteSize().Y>=20.f);
  }
  TArray<FColor> Pixels; FIntVector Size;
  if (!Test->TestTrue(TEXT("Actual viewport capture"),FSlateApplication::Get().TakeScreenshot(Window->GetContent(),Pixels,Size))) return;
  const FString Dir=FPaths::ProjectSavedDir()/TEXT("Stage8_8C"); IFileManager::Get().MakeDirectory(*Dir,true);
  TArray64<uint8> PNG; FImageUtils::PNGCompressImageArray(Size.X,Size.Y,Pixels,PNG);
  Test->TestTrue(TEXT("Whole viewport evidence saved"),FFileHelper::SaveArrayToFile(PNG,*(Dir/File)));
  if (FString(File)==TEXT("HighCross_V2_Attack.png"))
   Test->TestTrue(TEXT("Full context evidence saved from the same real frame"),FFileHelper::SaveArrayToFile(PNG,*(Dir/TEXT("HighCross_V2_FullContext.png"))));
  Test->AddInfo(FString::Printf(TEXT("BROADCAST_VIEWPORT %s %dx%d surface=%.1fx%.1f"),File,Size.X,Size.Y,SurfaceSize.X,SurfaceSize.Y));
 }
 FAutomationTestBase* Test;
 double Start=FPlatformTime::Seconds(),Changed=0;
 int32 Step=0, PreviousMode=0;
 IConsoleVariable* Mode=nullptr; int32 CompareStep=0; FString ComparedFormula,ComparedIdentity;
};
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFormulaBroadcastPIETest,"FMCodex.PIE.FormulaBroadcastPrototype.HighCross",
 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFormulaBroadcastPIETest::RunTest(const FString&)
{
 FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShareable(new FStartBroadcastPIE()));
 ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(2.f));
 FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShareable(new FBroadcastPIE(this)));
 ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
 return true;
}
#endif
