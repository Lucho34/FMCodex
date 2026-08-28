#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"

#include "FMCodexLocalMatchInteractionView.h"
#include "FMCodexLocalMatchResolutionFeedback.h"
#if !UE_BUILD_SHIPPING
#include "FMCodexLocalDevRollOverride.h"
#endif

#include "FMCodexLocalMatchPlayerController.generated.h"

class AFMCodexLocalMatchHostGameMode;
class UFMCodexLocalMatchScreenWidget;
class SBox;
class SWidget;

struct FMCODEX_API FFMCodexLocalMatchCommandDiagnostic
{
	FString CommandName = TEXT("None");
	bool bHostSuccess = false;
	bool bAuthoritativeAccepted = false;
	bool bAuthoritativeSuccess = false;
	FString Message = TEXT("No command has been submitted.");
	FString PresentationSummary = TEXT("Waiting for match input.");
};

UCLASS()
class FMCODEX_API AFMCodexLocalMatchPlayerController final
	: public APlayerController
{
	GENERATED_BODY()

public:
	AFMCodexLocalMatchPlayerController();

	const FFMCodexLocalMatchInteractionView& GetInteractionView() const;
	const FFMCodexLocalMatchCommandDiagnostic& GetLastDiagnostic() const;
	const FFMCodexLocalMatchResolutionFeedback&
		GetResolutionFeedback() const;
	UFMCodexLocalMatchScreenWidget* GetPlayerMatchScreen() const;

	void InitializePlayerFacingUI();
	void RefreshPresentation();
	void StartNewDemoMatch();
#if WITH_DEV_AUTOMATION_TESTS
	void SetNextDemoMatchSeedForTesting(int32 Seed);
#endif
	void RollDemoTacticalPoints();
	void DeployOrdinary(FName CardId, FName SlotId);
	void DeployGoalkeeper(FName SlotId);
	void FinishDeployment();
	void SubmitCarrier(FName CardId);
	void SubmitMarker(FName CardId);
	void SubmitSkill(FName SkillId);
	void SubmitRunner(FName CardId);
	void SubmitHelper(FName CardId);
	/**
	 * Unified player-facing "do not use tactical" intent. The projected
	 * authoritative capability selects DeclineSkill or ResolveNoLegalSkill;
	 * the typed authority commands remain mutually exclusive.
	 */
	void AbandonCurrentTacticalSelection();
	void DeclineCurrentSelection();
	void ResolveNoLegalCurrentSelection();
	void SubmitBranchIntent(EMatchPlayElectiveBranchIntent Intent);
	void SubmitOneOnOneShotChoice(
		EMatchPlayThroughBallOneOnOneShotChoice Choice);
	void RollCrossAttack();
	void RollCrossDefense();
	void ApplyCrossTerminalResolution();
	/** Legacy compatibility wrapper; production does not expose AndAdvance. */
	void CompleteCrossAndAdvance();
	void RollThroughBallFeetAttack();
	void RollThroughBallFeetDefense();
	void RollThroughBallAntiOffsideAttack();
	void RollThroughBallOneOnOneChipShotAttack();
	void RollThroughBallOneOnOneDirectShotAttack();
	void RollThroughBallOneOnOneDirectShotDefense();
	void RollThroughBallBehindDefenseAttack();
	void RollThroughBallBehindDefenseDefense();
	void ApplyThroughBallFeetTerminalResolution();
	/** Legacy compatibility wrapper; production does not expose AndAdvance. */
	void CompleteThroughBallFeetAndAdvance();
	void AdvanceAfterTerminal();
	void ContinueResolution();

#if !UE_BUILD_SHIPPING
	FFMCodexLocalDevRollOverrideCommandResult SetLocalDevRollOverride(
		const FFMCodexLocalDevRollOverrideRequest& Request);
	void ClearLocalDevRollOverride(EFMCodexLocalDevRollTarget Target);
	void ClearAllLocalDevRollOverrides();
	TArray<FFMCodexLocalDevPendingRollOverride>
		GetLocalDevPendingRollOverrides() const;
#endif

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	AFMCodexLocalMatchHostGameMode* FindLocalMatchHost() const;
	void InitializeDeveloperSlateSurface();
#if !UE_BUILD_SHIPPING
	void InitializeLocalDevRollOverrideSurface();
#endif
	void RefreshPlayerMatchScreen();
	void RebuildControlSurface();
	TSharedRef<SWidget> BuildControlSurface();
	void RecordLocalFailure(const FString& CommandName, const FString& Message);
	void ResolveAutomaticNoLegalHelperIfNeeded();

	template <typename TResult>
	void RecordCommandResult(const FString& CommandName, const TResult& Result)
	{
		const FFMCodexLocalMatchInteractionView PreviousView = InteractionView;
		const FFMCodexLocalMatchResolutionFeedback PreviousFeedback =
			ResolutionFeedback;
		LastDiagnostic.CommandName = CommandName;
		LastDiagnostic.bHostSuccess = Result.bSuccess;
		LastDiagnostic.bAuthoritativeAccepted =
			Result.AuthoritativeResult.RuntimeEnvelope.bAccepted;
		LastDiagnostic.bAuthoritativeSuccess =
			Result.AuthoritativeResult.RuntimeEnvelope.bDomainSuccess;
		LastDiagnostic.Message = Result.bSuccess
			? TEXT("Accepted by the authoritative local Host.")
			: !Result.ErrorMessage.IsEmpty()
				? Result.ErrorMessage
				: Result.AuthoritativeResult.RuntimeEnvelope.ErrorMessage;
		RefreshPresentation();
		if (!Result.bSuccess)
		{
			ResolutionFeedback =
				FFMCodexLocalMatchResolutionFeedbackBuilder::BuildRejected(
					CommandName,
					LastDiagnostic.Message,
					LastDiagnostic.bAuthoritativeAccepted);
			LastDiagnostic.PresentationSummary = TEXT("Command rejected");
		}
		else
		{
			ResolutionFeedback = CommandName == TEXT("AdvanceAfterTerminal")
				&& PreviousFeedback.bTerminal
					? PreviousFeedback
					: FFMCodexLocalMatchResolutionFeedbackBuilder::Build(
						CommandName, Result, PreviousView, InteractionView);
			if (ResolutionFeedback.bTerminal)
			{
				LastDiagnostic.PresentationSummary =
					ResolutionFeedback.TerminalSummary == TEXT("RESULT: GOAL")
						? TEXT("GOAL")
						: TEXT("Attack complete - no goal");
			}
			else
			{
				LastDiagnostic.PresentationSummary =
					ResolutionFeedback.StepSummary;
			}
		}
		RebuildControlSurface();
	}

	FFMCodexLocalMatchInteractionView InteractionView;
	FFMCodexLocalMatchCommandDiagnostic LastDiagnostic;
	FFMCodexLocalMatchResolutionFeedback ResolutionFeedback;
	bool bCrossRouteCommandInFlight = false;
	bool bCrossRollCommandInFlight = false;
	bool bThroughBallFeetRollCommandInFlight = false;
	bool bThroughBallAntiOffsideRollCommandInFlight = false;
	bool bThroughBallOneOnOneRollCommandInFlight = false;
	bool bThroughBallBehindDefenseRollCommandInFlight = false;

#if WITH_DEV_AUTOMATION_TESTS
	int32 NextDemoMatchSeedForTesting = INDEX_NONE;
#endif

	UPROPERTY(EditDefaultsOnly, Category = "Local Match|UI")
	TSubclassOf<UFMCodexLocalMatchScreenWidget> PlayerMatchScreenClass;

	UPROPERTY(EditDefaultsOnly, Category = "Local Match|UI")
	bool bEnablePlayerUMGSurface = true;

	UPROPERTY(EditDefaultsOnly, Category = "Local Match|UI")
	bool bEnableDeveloperSlateSurface = false;

	UPROPERTY(Transient)
	TObjectPtr<UFMCodexLocalMatchScreenWidget> PlayerMatchScreen;

	TSharedPtr<SWidget> ViewportWidget;
	TSharedPtr<SBox> SurfaceContainer;
#if !UE_BUILD_SHIPPING
	TSharedPtr<SWidget> DevRollOverrideViewportWidget;
#endif
};
