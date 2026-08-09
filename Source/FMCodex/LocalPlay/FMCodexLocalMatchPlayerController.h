#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"

#include "FMCodexLocalMatchInteractionView.h"

#include "FMCodexLocalMatchPlayerController.generated.h"

class AFMCodexLocalMatchHostGameMode;
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

struct FMCODEX_API FFMCodexLocalMatchHotSeatHandoffState
{
	bool bAwaitingAcknowledgement = false;
	EInitialTurnOrderPlayer PendingPlayer = EInitialTurnOrderPlayer::None;
	EFMCodexLocalMatchInteractionCategory PendingInteraction =
		EFMCodexLocalMatchInteractionCategory::None;
	EInitialTurnOrderPlayer LastRevealedHumanPlayer =
		EInitialTurnOrderPlayer::None;
};

class FMCODEX_API FFMCodexLocalMatchHotSeatHandoffPolicy final
{
public:
	static void Reconcile(
		const FFMCodexLocalMatchInteractionView& InteractionView,
		FFMCodexLocalMatchHotSeatHandoffState& HandoffState);

	static bool Acknowledge(
		const FFMCodexLocalMatchInteractionView& InteractionView,
		FFMCodexLocalMatchHotSeatHandoffState& HandoffState);
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
	const FFMCodexLocalMatchHotSeatHandoffState& GetHotSeatHandoffState() const;
	bool IsAwaitingHotSeatHandoff() const;

	void RefreshPresentation();
	void AcknowledgeHotSeatHandoff();
	void StartNewDemoMatch();
	void BeginDemoOrdinaryAttack();
	void DeployOrdinary(FName CardId, FName SlotId);
	void DeployGoalkeeper(FName SlotId);
	void FinishDeployment();
	void SubmitCarrier(FName CardId);
	void SubmitMarker(FName CardId);
	void SubmitSkill(FName SkillId);
	void SubmitRunner(FName CardId);
	void SubmitHelper(FName CardId);
	void DeclineCurrentSelection();
	void ResolveNoLegalCurrentSelection();
	void SubmitBranchIntent(EMatchPlayElectiveBranchIntent Intent);
	void SubmitOneOnOneShotChoice(
		EMatchPlayThroughBallOneOnOneShotChoice Choice);
	void ContinueResolution();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	AFMCodexLocalMatchHostGameMode* FindLocalMatchHost() const;
	void RebuildControlSurface();
	TSharedRef<SWidget> BuildControlSurface();
	void UpdateHotSeatHandoff(
		const FFMCodexLocalMatchInteractionView& NewInteractionView);
	bool AllowGameplayCommand(const FString& CommandName);
	void RecordLocalFailure(const FString& CommandName, const FString& Message);

	template <typename TResult>
	void RecordCommandResult(const FString& CommandName, const TResult& Result)
	{
		const int32 PreviousPlayerAScore = InteractionView.PlayerAScore;
		const int32 PreviousPlayerBScore = InteractionView.PlayerBScore;
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
			LastDiagnostic.PresentationSummary = TEXT("Command rejected");
		}
		else if (CommandName.StartsWith(TEXT("Apply")))
		{
			LastDiagnostic.PresentationSummary =
				InteractionView.PlayerAScore > PreviousPlayerAScore
					|| InteractionView.PlayerBScore > PreviousPlayerBScore
						? TEXT("GOAL")
						: TEXT("Attack complete - no goal");
		}
		else if (CommandName.Contains(TEXT("Route")))
		{
			LastDiagnostic.PresentationSummary = TEXT("Route resolved");
		}
		else if (CommandName.StartsWith(TEXT("Resolve")))
		{
			LastDiagnostic.PresentationSummary =
				TEXT("Resolution step accepted");
		}
		else if (CommandName.StartsWith(TEXT("Deploy")))
		{
			LastDiagnostic.PresentationSummary = TEXT("Card deployed");
		}
		else if (CommandName.StartsWith(TEXT("Submit")))
		{
			LastDiagnostic.PresentationSummary = TEXT("Selection accepted");
		}
		else
		{
			LastDiagnostic.PresentationSummary =
				TEXT("Authoritative command accepted");
		}
		RebuildControlSurface();
	}

	FFMCodexLocalMatchInteractionView InteractionView;
	FFMCodexLocalMatchHotSeatHandoffState HotSeatHandoffState;
	FFMCodexLocalMatchCommandDiagnostic LastDiagnostic;
	TSharedPtr<SWidget> ViewportWidget;
	TSharedPtr<SBox> SurfaceContainer;
};
