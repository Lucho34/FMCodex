#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
#include "FMCodexInteractionPanelWidget.h"
#include "FMCodexMatchHeaderWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Internationalization/Internationalization.h"
#include "Internationalization/TextLocalizationManager.h"
#include "Internationalization/Text.h"
#include "LocalizationSettings.h"
#include "LocalizationConfigurationScript.h"
#include "HAL/FileManager.h"
#include "Misc/AutomationTest.h"
#include "Misc/Paths.h"
#include "Misc/ScopeExit.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexLocalizationConfigurationTest,
 "FMCodex.Localization.FoundationA.Configuration",
 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexLocalizationConfigurationTest::RunTest(const FString&)
{
 auto* Targets=ULocalizationSettings::GetGameTargetSet();
 if(!TestNotNull(TEXT("Native Dashboard target set"),Targets))return false;
 TestEqual(TEXT("One Game target"),Targets->TargetObjects.Num(),1);
 const auto* Found=Targets->TargetObjects.FindByPredicate([](const auto& T){return T->Settings.Name==TEXT("Game");});
 if(!TestNotNull(TEXT("Game recognized by UE Localization"),Found))return false;
 auto* Target=Found->Get();
 const auto& Settings=Target->Settings;
 TestEqual(TEXT("Chinese native index"),Settings.NativeCultureIndex,0);
 if(!TestEqual(TEXT("Two cultures"),Settings.SupportedCulturesStatistics.Num(),2))return false;
 TestEqual(TEXT("Native simplified Chinese"),Settings.SupportedCulturesStatistics[0].CultureName,FString(TEXT("zh-Hans")));
 TestEqual(TEXT("Initial translation English"),Settings.SupportedCulturesStatistics[1].CultureName,FString(TEXT("en")));
 for(const auto& C:Settings.SupportedCulturesStatistics)
  TestTrue(TEXT("Culture recognized by this engine"),FInternationalization::Get().GetCulture(C.CultureName).IsValid());
 FText Error;
 TestTrue(TEXT("Source gather settings valid"),Settings.GatherFromTextFiles.Validate(false,Error));
 TestTrue(TEXT("Package gather settings valid"),Settings.GatherFromPackages.Validate(false,Error));
 TArray<FString> Paths;
 GConfig->GetArray(TEXT("Internationalization"),TEXT("LocalizationPaths"),Paths,GGameIni);
 TestTrue(TEXT("Game runtime loading path"),Paths.ContainsByPredicate([](const auto& P){return P.EndsWith(TEXT("Content/Localization/Game"));}));
 FString DefaultCulture;
 GConfig->GetString(TEXT("Internationalization"),TEXT("Culture"),DefaultCulture,GGameIni);
 TestEqual(TEXT("Game default stays Chinese"),DefaultCulture,FString(TEXT("zh-Hans")));
 // Native UE generator, no SCC. Review/copy these files into Config/Localization.
 // Ordinary runs only write to Saved, never overwrite tracked configuration.
 const FString Out=FPaths::ProjectSavedDir()/TEXT("LocalizationFoundation/GeneratedConfigs");
 IFileManager::Get().MakeDirectory(*Out,true);
 using namespace LocalizationConfigurationScript;
 auto Save=[&](FLocalizationConfigurationScript Script,const TCHAR* Name)
 {TestTrue(*FString::Printf(TEXT("Generate %s with UE tooling"),Name),Script.Write(Out/Name));};
 Save(GenerateGatherTextConfigFile(Target),TEXT("Game_Gather.ini"));
 Save(GenerateExportTextConfigFile(Target),TEXT("Game_Export.ini"));
 Save(GenerateImportTextConfigFile(Target),TEXT("Game_Import.ini"));
 Save(GenerateCompileTextConfigFile(Target),TEXT("Game_Compile.ini"));
 return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexLocalizationVisibleUITest,
 "FMCodex.Localization.FoundationA.VisibleUI",
 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexLocalizationVisibleUITest::RunTest(const FString&)
{
 auto& Loc=FTextLocalizationManager::Get();
 const bool WasPreview=Loc.IsGameLocalizationPreviewEnabled();
 const FString PreviousPreview=Loc.GetConfiguredGameLocalizationPreviewLanguage();
 ON_SCOPE_EXIT {if(WasPreview)Loc.EnableGameLocalizationPreview(PreviousPreview);else Loc.DisableGameLocalizationPreview();Loc.WaitForAsyncTasks();};
 // The supported Editor game-preview mechanism loads compiled Game locres.
 // No direct resource injection, hardcoded runtime English branch, or source mutation.
 for(const bool English:{false,true,false})
 {
  Loc.EnableGameLocalizationPreview(English?TEXT("en"):TEXT("zh-Hans"));
  Loc.WaitForAsyncTasks(); // UE 5.3 loads game-preview resources asynchronously.
  auto* Panel=NewObject<UFMCodexInteractionPanelWidget>();auto PanelSlate=Panel->TakeWidget();
  FFMCodexUMGInteractionViewModel View;
  View.Category=EFMCodexUMGInteractionCategory::Deploy;
  View.bCanFinishDeployment=true;View.PrimaryAction.bAvailable=true;
  View.PrimaryAction.Label=TEXT("FINISH DEPLOYMENT");
  Panel->RefreshFromPresentation(View);
  auto* Header=NewObject<UFMCodexMatchHeaderWidget>();auto HeaderSlate=Header->TakeWidget();
  FFMCodexUMGMatchHeaderViewModel H;
  H.bHasCurrentAttacker=true;H.CurrentAttackerLabel=TEXT("玩家 A");
  H.CurrentAttackerAttackIndex=2;H.CurrentAttackerMaxAttackTurns=6;
  Header->RefreshFromPresentation(H);
  auto Check=[&](UTextBlock* Text,const TCHAR* Expected,const TCHAR* Namespace,const TCHAR* Key)
  {
   for(const UWidget* W=Text;W;W=W->GetParent())TestTrue(TEXT("Final text and ancestors visible"),W->IsVisible());
   TestEqual(TEXT("Actual displayed UTextBlock"),Text->GetText().ToString(),FString(Expected));
   if(Key)
   {
    const auto NS=FTextInspector::GetNamespace(Text->GetText());
    const auto K=FTextInspector::GetKey(Text->GetText());
    TestTrue(TEXT("FText identity survives to final widget"),NS.IsSet() && NS.GetValue()==Namespace && K.IsSet() && K.GetValue()==Key);
   }
   AddInfo(FString::Printf(TEXT("%s %s = %s"),English?TEXT("en"):TEXT("zh-Hans"),*Text->GetName(),*Text->GetText().ToString()));
  };
  auto* Finish=CastChecked<UButton>(Panel->GetWidgetFromName(TEXT("InteractionFinishDeploymentButton")));
  TestTrue(TEXT("Existing finish action stays enabled"),Finish->GetIsEnabled());
  Check(CastChecked<UTextBlock>(Finish->GetChildAt(0)),English?TEXT("Finish Deployment"):TEXT("结束部署"),TEXT("FMCodexMatchShell"),TEXT("FinishDeployment"));
  auto* Reference=CastChecked<UButton>(Panel->GetWidgetFromName(TEXT("DeploymentTacticalReferenceEntryButton")));
  Check(CastChecked<UTextBlock>(Reference->GetChildAt(0)),English?TEXT("Tactical Guide"):TEXT("战术说明"),TEXT("FMCodexPlayerUIPresentationText"),TEXT("TacticalReference"));
  Check(CastChecked<UTextBlock>(Header->GetWidgetFromName(TEXT("LeftAttackTurnHeading"))),English?TEXT("Attack Turns"):TEXT("进攻回合"),TEXT("FMCodexPlayerUIPresentationText"),TEXT("AttackTurnHeading"));
  Check(CastChecked<UTextBlock>(Header->GetWidgetFromName(TEXT("CurrentAttackProgressLabel"))),English?TEXT("玩家 A · Attack 2/6"):TEXT("玩家 A 第 2/6 次进攻"),nullptr,nullptr);
 }
 return true;
}
#endif
