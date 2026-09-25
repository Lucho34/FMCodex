#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && !UE_BUILD_SHIPPING
#include "FMCodexInlineResolutionFormulaSurfaceWidget.h"
#include "FMCodexLocalMatchPlayerController.h"
#include "FMCodexLocalMatchScreenWidget.h"
#include "Editor.h"
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
	bool DeployHierarchyOrdinary(
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

	bool SubmitHierarchyFirst(
		AFMCodexLocalMatchPlayerController& Controller,
		const EFMCodexLocalMatchInteractionCategory Expected)
	{
		Controller.RefreshPresentation();
		const FFMCodexLocalMatchInteractionView& View =
			Controller.GetInteractionView();
		if (View.InteractionCategory != Expected
			|| View.SelectionOptions.IsEmpty())
		{
			return false;
		}
		const FName Id = View.SelectionOptions[0].Id;
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


class FStartHierarchyPIE final : public IAutomationLatentCommand
{
public:
 bool Update() override
 {
  auto* Settings=DuplicateObject<ULevelEditorPlaySettings>(GetDefault<ULevelEditorPlaySettings>(),GetTransientPackage());
  Settings->NewWindowWidth=1920; Settings->NewWindowHeight=1080;
  Settings->SetPlayNetMode(EPlayNetMode::PIE_Standalone); Settings->SetPlayNumberOfClients(1);
  FRequestPlaySessionParams Params; Params.EditorPlaySettings=Settings; GEditor->RequestPlaySession(Params); return true;
 }
};
class FHierarchyPIE final : public IAutomationLatentCommand
{
public:
 explicit FHierarchyPIE(FAutomationTestBase* InTest):Test(InTest) {}
 bool Update() override
 {
  if (FPlatformTime::Seconds()-Start>120) { Test->AddError(TEXT("Formula hierarchy PIE timed out")); return true; }
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
   for (int32 I=0;I<4;++I) if (!DeployHierarchyOrdinary(*C,Forward)) { Test->AddError(TEXT("Legal deployment failed")); return true; }
   C->FinishDeployment(); C->FinishDeployment();
   S->RequestSubmitCarrier(Attacker==EInitialTurnOrderPlayer::PlayerA ? FName(TEXT("Prototype.Arsenal.BukayoSaka")) : FName(TEXT("Prototype.ManchesterCity.RayanAitNouri")));
   if (!SubmitHierarchyFirst(*C,EFMCodexLocalMatchInteractionCategory::SelectMarker)
    || !SubmitHierarchyFirst(*C,EFMCodexLocalMatchInteractionCategory::SelectRunner)) { Test->AddError(TEXT("Legal role selection failed")); return true; }
   if (!C->GetInteractionView().SelectionOptions.IsEmpty())
   {
    if (!SubmitHierarchyFirst(*C,EFMCodexLocalMatchInteractionCategory::SelectHelper)) { Test->AddError(TEXT("Legal helper selection failed")); return true; }
   }
   else if (C->GetInteractionView().bCanResolveNoLegalChoice) C->ResolveNoLegalCurrentSelection();
   else C->DeclineCurrentSelection();
   C->SubmitSkill(TEXT("Canonical.Skill.Cross.4.6"));
   C->SubmitBranchIntent(EMatchPlayElectiveBranchIntent::CrossLow);
   if (!Test->TestTrue(TEXT("Natural Low Cross route entry"),C->GetLastDiagnostic().bHostSuccess
    && C->GetInteractionView().InteractionCategory==EFMCodexLocalMatchInteractionCategory::RollCrossRoute)) return true;
   if (!Override(*C,EFMCodexLocalDevRollTarget::CrossRoute,2)
    || !Override(*C,EFMCodexLocalDevRollTarget::CrossLowAttack,4)
    || !Override(*C,EFMCodexLocalDevRollTarget::CrossLowDefense,3)) return true;
   Next(); return false;
  }
  if (Step==3) { S->RequestContinueResolution(); Next(); return false; }
  auto* F=S->GetInlineFormulaSurface(); const auto& P=F->GetPresentation();
  using E=EFMCodexFormulaEmphasis;
  if (Step>=4 && Step<=6)
  {
   const E A=Step==4 ? E::Active : Step==5 ? E::Context : E::Resolved;
   const E D=Step==4 ? E::Context : Step==5 ? E::Active : E::Resolved;
   if (!Test->TestTrue(TEXT("Real Low Cross A -> D -> resolved visual owners"),P.ContestId==TEXT("Cross.Low")
     && F->GetRowEmphasis(true)==A && F->GetRowEmphasis(false)==D)) return true;
   Test->TestTrue(TEXT("Real high-density facts include both attack participants and full defense context"),
    P.AttackRow.Participants.Num()==2 && P.DefenseRow.Participants.Num()>=2 && P.DefenseRow.Terms.Num()>=4);
   Test->AddInfo(FString::Printf(TEXT("HIERARCHY_PIE step=%d A=%d D=%d values=%s/%s game=%.3f"),Step,
    static_cast<int32>(A),static_cast<int32>(D),*P.AttackRow.DisplayedResultLabel,*P.DefenseRow.DisplayedResultLabel,GEditor->PlayWorld->GetTimeSeconds()));
   Capture(Step==4 ? TEXT("Formula_AttackActive.png") : Step==5 ? TEXT("Formula_DefenseActive.png") : TEXT("Formula_Resolved.png"));
   F->RequestContinue(); Next(); return false;
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
  TArray<FColor> Pixels; FIntVector Size;
  if (!Test->TestTrue(TEXT("Actual viewport capture"),FSlateApplication::Get().TakeScreenshot(Window->GetContent(),Pixels,Size))) return;
  const FString Dir=FPaths::ProjectSavedDir()/TEXT("Stage8_7F"); IFileManager::Get().MakeDirectory(*Dir,true);
  TArray64<uint8> PNG; FImageUtils::PNGCompressImageArray(Size.X,Size.Y,Pixels,PNG);
  Test->TestTrue(TEXT("Whole viewport evidence saved"),FFileHelper::SaveArrayToFile(PNG,*(Dir/File)));
  Test->AddInfo(FString::Printf(TEXT("HIERARCHY_VIEWPORT %s %dx%d"),File,Size.X,Size.Y));
 }
 FAutomationTestBase* Test;
 double Start=FPlatformTime::Seconds(),Changed=0;
 int32 Step=0;
};
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFormulaHierarchyPIETest,"FMCodex.PIE.FormulaHierarchy.LowCross",
 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFormulaHierarchyPIETest::RunTest(const FString&)
{
 FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShareable(new FStartHierarchyPIE()));
 ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(2.f));
 FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShareable(new FHierarchyPIE(this)));
 ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
 return true;
}
#endif
