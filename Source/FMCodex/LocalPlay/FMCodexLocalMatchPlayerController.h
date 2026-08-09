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

	void RefreshPresentation();
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
	void RecordLocalFailure(const FString& CommandName, const FString& Message);

	template <typename TResult>
	void RecordCommandResult(const FString& CommandName, const TResult& Result)
	{
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
	}

	FFMCodexLocalMatchInteractionView InteractionView;
	FFMCodexLocalMatchCommandDiagnostic LastDiagnostic;
	TSharedPtr<SWidget> ViewportWidget;
	TSharedPtr<SBox> SurfaceContainer;
};
