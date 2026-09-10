// Explicit, removable DEV capture of the real LocalPlay viewport. No fake UI state.
#if WITH_EDITOR && WITH_DEV_AUTOMATION_TESTS
#include "FMCodexLocalMatchPlayerController.h"
#include "FMCodexLocalMatchScreenWidget.h"
#include "FMCodexLocalDevRollOverride.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "Misc/CommandLine.h"
#include "Misc/Paths.h"
#include "TimerManager.h"
#include "UnrealClient.h"

namespace
{
FAutoConsoleCommandWithWorld PreviewMatchShell(
	TEXT("FMCodex.Dev.PreviewMatchShell"),
	TEXT("Start a new LocalPlay demo through the normal screen intent. Optional -MatchShellScreenshot=<name> captures its real viewport."),
	FConsoleCommandWithWorldDelegate::CreateLambda([](UWorld* World)
	{
		if (!World || World->GetNetMode() != NM_Standalone) return;
		TWeakObjectPtr<UWorld> WeakWorld(World);
		FTimerHandle StartHandle;
		World->GetTimerManager().SetTimer(StartHandle, [WeakWorld]()
		{
			UWorld* LiveWorld = WeakWorld.Get();
			if (!LiveWorld) return;
			auto* Controller = Cast<AFMCodexLocalMatchPlayerController>(LiveWorld->GetFirstPlayerController());
			if (!Controller || !Controller->GetPlayerMatchScreen()) return;
			Controller->GetPlayerMatchScreen()->RequestStartNewMatch();
			const bool bDeployment = FParse::Param(FCommandLine::Get(), TEXT("MatchShellDeployment"));
			if (bDeployment)
			{
				FFMCodexLocalDevRollOverrideRequest Roll;
				Roll.Target = EFMCodexLocalDevRollTarget::FullD12;
				Roll.Value = 6;
				if (!Controller->SetLocalDevRollOverride(Roll).bSuccess) return;
				Controller->GetPlayerMatchScreen()->RequestRollTacticalPoints();
			}
			FString CaptureName;
			if (!FParse::Value(FCommandLine::Get(), TEXT("MatchShellScreenshot="), CaptureName)) return;
			// Allow actual frame ticks and the existing texture pipeline to settle.
			FTimerHandle CaptureHandle;
			LiveWorld->GetTimerManager().SetTimer(CaptureHandle, [WeakWorld, CaptureName]()
			{
				if (!WeakWorld.IsValid()) return;
				const FString Path = FPaths::ProjectSavedDir() / TEXT("Stage8_1A")
					/ (FPaths::GetCleanFilename(CaptureName) + TEXT(".png"));
				FScreenshotRequest::RequestScreenshot(Path, true, false);
				UE_LOG(LogTemp, Display, TEXT("MATCH_SHELL_VIEWPORT_CAPTURE %s"), *Path);
				if (FParse::Param(FCommandLine::Get(), TEXT("MatchShellExitAfterCapture")))
				{
					FTimerHandle ExitHandle;
					WeakWorld->GetTimerManager().SetTimer(ExitHandle, []()
					{
						FPlatformMisc::RequestExit(false);
					}, 2.0f, false);
				}
			}, 9.0f, false);
		}, 1.0f, false);
	}));
}
#endif
