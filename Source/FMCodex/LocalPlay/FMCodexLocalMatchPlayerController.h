#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"

#include "FMCodexLocalMatchInteractionView.h"
#include "FMCodexMatchClientViewPort.h"
#include "FMCodexLocalMatchResolutionFeedback.h"
#include "../MatchPlayRuntime/MatchPlayHostPort.h"
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

#if !UE_BUILD_SHIPPING
	/** Explicit DEV restart; the regular start entry always keeps three per side. */
	void StartNewDevShortMatch();
#endif
#if WITH_DEV_AUTOMATION_TESTS
	void SetNextDemoMatchSeedForTesting(int32 Seed);
	bool IsRecoveryNotificationDismissScheduledForTesting() const;
	void ExpireRecoveryNotificationForTesting();
#endif
	void RollDemoTacticalPoints();
	void NotifyEntryRevealComplete();
	void ToggleSetPieceDraftCard(FName CardId);
	void ConfirmSetPieceDraft();
	void CancelCornerLockConfirmation();
	void SubmitShortFreeKickMethod(EMatchPlayShortFreeKickMethod Method);
	void SubmitLongFreeKickMethod(EMatchPlayLongFreeKickMethod Method);
	void SubmitPenaltyMethod(EMatchPlayPenaltyMethod Method);
	void SubmitCornerIntent(EMatchPlayCornerRouteIntent Intent);
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
	void RollCrossRoute();
	void RollCrossAttack();
	void RollCrossDefense();
	void RollLongShotDirectAttack();
	void RollLongShotDirectDefense();
	void RollLongShotDeadCorner();
	void RollCutInsideShotDirectAttack();
	void RollCutInsideShotDirectDefense();
	void RollCutInsideShotDeadCorner();
	void RollPassControlRoute();
	void RollPassControlAttack();
	void RollPassControlDefense();
	void ApplyCrossTerminalResolution();
	/** Legacy compatibility wrapper; production does not expose AndAdvance. */
	void CompleteCrossAndAdvance();
	void RollThroughBallInitialRoute();
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
	void SubmitProjectedPrimaryPlayerIntent();
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
	IMatchPlayPlayerIntentPort* FindPlayerIntentPort() const;
	const IFMCodexMatchClientViewPort* FindClientViewPort() const;
	void InitializeDeveloperSlateSurface();
#if !UE_BUILD_SHIPPING
	void InitializeLocalDevRollOverrideSurface();
#endif
	void RefreshPlayerMatchScreen();
	void RebuildControlSurface();
	TSharedRef<SWidget> BuildControlSurface();
	void RecordLocalFailure(const FString& CommandName, const FString& Message);
	void ResolveAutomaticNoLegalHelperIfNeeded();
	void ReconcileSetPieceDraft();
	void ResolveAutomaticSetPieceEntryIfNeeded();
	void ResetSetPieceDraft();
	void ScheduleRecoveryNotificationDismiss();
	void CancelRecoveryNotificationDismiss();
	void DismissRecoveryNotification();

	template <typename TRequest>
	FMatchPlayPlayerIntentSubmissionResult SubmitPlayerIntent(
		const FString& CommandName,
		const EMatchPlayAuthoritativeCommandKind CommandKind,
		const TRequest& Request)
	{
		IMatchPlayPlayerIntentPort* Port = FindPlayerIntentPort();
		if (Port == nullptr)
		{
			RecordLocalFailure(CommandName, TEXT("Host port unavailable."));
			return {};
		}
		const FMatchPlayPlayerIntentSubmissionResult Result =
			Port->SubmitPlayerIntent(
				FMatchPlayPlayerIntent::Create(CommandKind, Request));
		RecordCommandResult(CommandName, Result);
		return Result;
	}

	template <typename TResult>
	void RecordCommandResult(const FString& CommandName, const TResult& Result)
	{
		CancelRecoveryNotificationDismiss();
		const FFMCodexLocalMatchInteractionView PreviousView = InteractionView;
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
			if (CommandName == TEXT("AdvanceAfterTerminal"))
			{
				// The completed attack no longer owns the surface. A bounded Recovery
				// fact may supply a non-blocking transition message for the next state.
				ResolutionFeedback =
					FFMCodexLocalMatchResolutionFeedbackBuilder::BuildRecovery(
						InteractionView);
			}
			else if (InteractionView.bTerminalPendingAdvance)
			{
				// The refreshed authoritative terminal snapshot owns presentation.
				// A generic command acknowledgement must never replace its result.
				ResolutionFeedback =
					FFMCodexLocalMatchResolutionFeedbackBuilder
						::BuildFromTerminalSnapshot(InteractionView);
			}
			else
			{
				ResolutionFeedback =
					FFMCodexLocalMatchResolutionFeedbackBuilder::Build(
						CommandName, Result, PreviousView, InteractionView);
			}
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
		if (ResolutionFeedback.bNonBlockingNotification)
		{
			ScheduleRecoveryNotificationDismiss();
		}
	}

	FFMCodexLocalMatchInteractionView InteractionView;
	FFMCodexLocalMatchCommandDiagnostic LastDiagnostic;
	FFMCodexLocalMatchResolutionFeedback ResolutionFeedback;
	bool bCrossRouteCommandInFlight = false;
	bool bCrossRollCommandInFlight = false;
	bool bLongShotRollCommandInFlight = false;
	bool bCutInsideShotRollCommandInFlight = false;
	bool bPassControlRollCommandInFlight = false;
	bool bThroughBallRouteCommandInFlight = false;
	bool bThroughBallFeetRollCommandInFlight = false;
	bool bThroughBallAntiOffsideRollCommandInFlight = false;
	bool bThroughBallOneOnOneRollCommandInFlight = false;
	bool bThroughBallBehindDefenseRollCommandInFlight = false;
	struct FSetPieceLocalDraft
	{
		int64 AttackSequence = 0;
		EInitialTurnOrderPlayer Side = EInitialTurnOrderPlayer::None;
		ESetPieceSelectedType Type = ESetPieceSelectedType::None;
		EMatchPlaySetPieceCornerRouteStage CornerStage =
			EMatchPlaySetPieceCornerRouteStage::None;
		FName CarrierCardId = NAME_None;
		TArray<FName> CornerCardIds;
		bool bLockConfirmationPending = false;
	};
	FSetPieceLocalDraft SetPieceDraft;
	FTimerHandle RecoveryNotificationDismissTimerHandle;
	static constexpr float RecoveryNotificationDurationSeconds = 2.0f;

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
