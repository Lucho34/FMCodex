#include "FMCodexLocalMatchPlayerController.h"

#include "FMCodexLocalMatchDemoConfiguration.h"
#include "FMCodexLocalMatchHostGameMode.h"

#include "../CoreRules/MatchPlayCurrentAttackPostRouteRollProgressQuery.h"
#include "../CoreRules/MatchPlayCurrentAttackResolveThroughBallBehindDefenseP1FormulaOrchestrator.h"

#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "EngineUtils.h"
#endif
#include "Engine/World.h"
#include "Framework/Application/SlateApplication.h"
#include "GameFramework/GameModeBase.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

namespace FMCodexLocalMatchPlayerController
{
	FString IntentLabel(const EMatchPlayElectiveBranchIntent Intent)
	{
		switch (Intent)
		{
		case EMatchPlayElectiveBranchIntent::DirectShot: return TEXT("Direct Shot");
		case EMatchPlayElectiveBranchIntent::DeadCorner: return TEXT("Dead Corner");
		case EMatchPlayElectiveBranchIntent::CrossHigh: return TEXT("Cross High");
		case EMatchPlayElectiveBranchIntent::CrossLow: return TEXT("Cross Low");
		default: return TEXT("Unknown Intent");
		}
	}

	FString ShotChoiceLabel(
		const EMatchPlayThroughBallOneOnOneShotChoice Choice)
	{
		return Choice == EMatchPlayThroughBallOneOnOneShotChoice::ChipShot
			? TEXT("Chip Shot")
			: TEXT("Direct Shot");
	}

	TSharedRef<SWidget> MakeButton(
		const FString& Label,
		TFunction<void()> Action)
	{
		return SNew(SButton)
			.ContentPadding(FMargin(8.0f, 4.0f))
			.OnClicked_Lambda([Action = MoveTemp(Action)]()
			{
				Action();
				return FReply::Handled();
			})
			[
				SNew(STextBlock).Text(FText::FromString(Label))
			];
	}
}

AFMCodexLocalMatchPlayerController::AFMCodexLocalMatchPlayerController()
{
	bShowMouseCursor = true;
}

const FFMCodexLocalMatchInteractionView&
AFMCodexLocalMatchPlayerController::GetInteractionView() const
{
	return InteractionView;
}

const FFMCodexLocalMatchCommandDiagnostic&
AFMCodexLocalMatchPlayerController::GetLastDiagnostic() const
{
	return LastDiagnostic;
}

void AFMCodexLocalMatchPlayerController::BeginPlay()
{
	Super::BeginPlay();
	RefreshPresentation();

	if (IsLocalController() && GEngine != nullptr
		&& GEngine->GameViewport != nullptr)
	{
		SAssignNew(SurfaceContainer, SBox)
		[
			BuildControlSurface()
		];
		ViewportWidget = SNew(SBorder)
			.Padding(12.0f)
			.BorderBackgroundColor(FLinearColor(0.02f, 0.02f, 0.02f, 0.94f))
			[
				SurfaceContainer.ToSharedRef()
			];
		GEngine->GameViewport->AddViewportWidgetContent(
			ViewportWidget.ToSharedRef(), 100);
		FInputModeUIOnly InputMode;
		SetInputMode(InputMode);
	}
}

void AFMCodexLocalMatchPlayerController::EndPlay(
	const EEndPlayReason::Type EndPlayReason)
{
	if (ViewportWidget.IsValid() && GEngine != nullptr
		&& GEngine->GameViewport != nullptr)
	{
		GEngine->GameViewport->RemoveViewportWidgetContent(
			ViewportWidget.ToSharedRef());
	}
	SurfaceContainer.Reset();
	ViewportWidget.Reset();
	Super::EndPlay(EndPlayReason);
}

AFMCodexLocalMatchHostGameMode*
AFMCodexLocalMatchPlayerController::FindLocalMatchHost() const
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return nullptr;
	}
	if (AFMCodexLocalMatchHostGameMode* Host =
		Cast<AFMCodexLocalMatchHostGameMode>(World->GetAuthGameMode()))
	{
		return Host;
	}
#if WITH_DEV_AUTOMATION_TESTS
	for (TActorIterator<AFMCodexLocalMatchHostGameMode> It(World); It; ++It)
	{
		return *It;
	}
#endif
	return nullptr;
}

void AFMCodexLocalMatchPlayerController::RefreshPresentation()
{
	AFMCodexLocalMatchHostGameMode* Host = FindLocalMatchHost();
	if (Host == nullptr || !Host->HasActiveLocalMatch())
	{
		InteractionView =
			FFMCodexLocalMatchInteractionViewBuilder::BuildNoActiveMatch();
		RebuildControlSurface();
		return;
	}

	const FFMCodexLocalMatchSnapshotResult State = Host->GetMatchSnapshot();
	const FFMCodexLocalMatchSkillRuleSnapshotResult Rules =
		Host->GetSkillRuleSnapshot();
	if (!State.bSuccess || !Rules.bSuccess)
	{
		InteractionView =
			FFMCodexLocalMatchInteractionViewBuilder::BuildNoActiveMatch();
		InteractionView.Diagnostic = !State.bSuccess
			? State.ErrorMessage
			: Rules.ErrorMessage;
		RebuildControlSurface();
		return;
	}

	InteractionView = FFMCodexLocalMatchInteractionViewBuilder::Build(
		State.Snapshot, Rules.Snapshot);
	RebuildControlSurface();
}

void AFMCodexLocalMatchPlayerController::RecordLocalFailure(
	const FString& CommandName,
	const FString& Message)
{
	LastDiagnostic.CommandName = CommandName;
	LastDiagnostic.bHostSuccess = false;
	LastDiagnostic.bAuthoritativeAccepted = false;
	LastDiagnostic.bAuthoritativeSuccess = false;
	LastDiagnostic.Message = Message;
	RefreshPresentation();
}

void AFMCodexLocalMatchPlayerController::StartNewDemoMatch()
{
	AFMCodexLocalMatchHostGameMode* Host = FindLocalMatchHost();
	if (Host == nullptr)
	{
		RecordLocalFailure(TEXT("StartNewLocalMatch"),
			TEXT("The local authoritative GameMode Host is unavailable."));
		return;
	}
	const FFMCodexLocalMatchDemoConfiguration Demo =
		FFMCodexLocalMatchDemoConfigurationFactory::Create();
	RecordCommandResult(
		TEXT("StartNewLocalMatch"),
		Host->StartNewLocalMatch(Demo.OpeningInput, Demo.SkillRuleSet));
}

void AFMCodexLocalMatchPlayerController::BeginDemoOrdinaryAttack()
{
	AFMCodexLocalMatchHostGameMode* Host = FindLocalMatchHost();
	if (Host == nullptr)
	{
		RecordLocalFailure(TEXT("BeginOrdinaryAttack"), TEXT("Host unavailable."));
		return;
	}
	const int32 ActionPoint =
		FFMCodexLocalMatchDemoConfigurationFactory::Create()
			.OrdinaryAttackActionPoint;
	RecordCommandResult(
		TEXT("BeginOrdinaryAttack"), Host->BeginOrdinaryAttack(ActionPoint));
}

void AFMCodexLocalMatchPlayerController::DeployOrdinary(
	const FName CardId,
	const FName SlotId)
{
	AFMCodexLocalMatchHostGameMode* Host = FindLocalMatchHost();
	if (Host == nullptr)
	{
		RecordLocalFailure(TEXT("DeployOrdinary"), TEXT("Host unavailable."));
		return;
	}
	FMatchPlayAuthoritativeDeployOrdinaryRequest Request;
	Request.RequestingSide = InteractionView.ExpectedActingPlayer;
	Request.CardId = CardId;
	Request.SlotId = SlotId;
	RecordCommandResult(TEXT("DeployOrdinary"), Host->DeployOrdinary(Request));
}

void AFMCodexLocalMatchPlayerController::DeployGoalkeeper(const FName SlotId)
{
	AFMCodexLocalMatchHostGameMode* Host = FindLocalMatchHost();
	if (Host == nullptr)
	{
		RecordLocalFailure(TEXT("DeployGoalkeeper"), TEXT("Host unavailable."));
		return;
	}
	FMatchPlayAuthoritativeDeployGoalkeeperRequest Request;
	Request.SlotId = SlotId;
	RecordCommandResult(
		TEXT("DeployGoalkeeper"), Host->DeployGoalkeeper(Request));
}

void AFMCodexLocalMatchPlayerController::FinishDeployment()
{
	AFMCodexLocalMatchHostGameMode* Host = FindLocalMatchHost();
	if (Host == nullptr)
	{
		RecordLocalFailure(TEXT("FinishDeployment"), TEXT("Host unavailable."));
		return;
	}
	RecordCommandResult(
		TEXT("FinishDeployment"),
		Host->FinishDeployment(
			InteractionView.AttackSequence,
			InteractionView.ExpectedActingPlayer));
}

void AFMCodexLocalMatchPlayerController::SubmitCarrier(const FName CardId)
{
	AFMCodexLocalMatchHostGameMode* Host = FindLocalMatchHost();
	if (Host == nullptr)
	{
		RecordLocalFailure(TEXT("SubmitCarrier"), TEXT("Host unavailable."));
		return;
	}
	FMatchPlayAuthoritativeSubmitCarrierRequest Request;
	Request.RequestingSide = InteractionView.ExpectedActingPlayer;
	Request.CarrierCardId = CardId;
	RecordCommandResult(TEXT("SubmitCarrier"), Host->SubmitCarrier(Request));
}

void AFMCodexLocalMatchPlayerController::SubmitMarker(const FName CardId)
{
	AFMCodexLocalMatchHostGameMode* Host = FindLocalMatchHost();
	if (Host == nullptr)
	{
		RecordLocalFailure(TEXT("SubmitMarker"), TEXT("Host unavailable."));
		return;
	}
	FMatchPlayAuthoritativeSubmitMarkerRequest Request;
	Request.RequestingSide = InteractionView.ExpectedActingPlayer;
	Request.MarkerCardId = CardId;
	RecordCommandResult(TEXT("SubmitMarker"), Host->SubmitMarker(Request));
}

void AFMCodexLocalMatchPlayerController::SubmitSkill(const FName SkillId)
{
	AFMCodexLocalMatchHostGameMode* Host = FindLocalMatchHost();
	if (Host == nullptr)
	{
		RecordLocalFailure(TEXT("SubmitSkill"), TEXT("Host unavailable."));
		return;
	}
	const auto Rules = Host->GetSkillRuleSnapshot();
	if (!Rules.bSuccess)
	{
		RecordLocalFailure(TEXT("SubmitSkill"), Rules.ErrorMessage);
		return;
	}
	FMatchPlayAuthoritativeSubmitSkillRequest Request;
	Request.RequestingSide = InteractionView.ExpectedActingPlayer;
	Request.SkillId = SkillId;
	RecordCommandResult(
		TEXT("SubmitSkill"), Host->SubmitSkill(Rules.Snapshot, Request));
}

void AFMCodexLocalMatchPlayerController::SubmitRunner(const FName CardId)
{
	AFMCodexLocalMatchHostGameMode* Host = FindLocalMatchHost();
	if (Host == nullptr)
	{
		RecordLocalFailure(TEXT("SubmitRunner"), TEXT("Host unavailable."));
		return;
	}
	FMatchPlayAuthoritativeSubmitRunnerRequest Request;
	Request.RequestingSide = InteractionView.ExpectedActingPlayer;
	Request.RunnerCardId = CardId;
	RecordCommandResult(TEXT("SubmitRunner"), Host->SubmitRunner(Request));
}

void AFMCodexLocalMatchPlayerController::SubmitHelper(const FName CardId)
{
	AFMCodexLocalMatchHostGameMode* Host = FindLocalMatchHost();
	if (Host == nullptr)
	{
		RecordLocalFailure(TEXT("SubmitHelper"), TEXT("Host unavailable."));
		return;
	}
	FMatchPlayAuthoritativeSubmitHelperRequest Request;
	Request.RequestingSide = InteractionView.ExpectedActingPlayer;
	Request.HelperCardId = CardId;
	RecordCommandResult(TEXT("SubmitHelper"), Host->SubmitHelper(Request));
}

void AFMCodexLocalMatchPlayerController::DeclineCurrentSelection()
{
	AFMCodexLocalMatchHostGameMode* Host = FindLocalMatchHost();
	if (Host == nullptr)
	{
		RecordLocalFailure(TEXT("DeclineSelection"), TEXT("Host unavailable."));
		return;
	}

	switch (InteractionView.InteractionCategory)
	{
	case EFMCodexLocalMatchInteractionCategory::SelectMarker:
	{
		FMatchPlayAuthoritativeDeclineMarkerRequest Request;
		Request.RequestingSide = InteractionView.ExpectedActingPlayer;
		RecordCommandResult(TEXT("DeclineMarker"), Host->DeclineMarker(Request));
		break;
	}
	case EFMCodexLocalMatchInteractionCategory::SelectSkill:
	{
		const auto Rules = Host->GetSkillRuleSnapshot();
		if (!Rules.bSuccess)
		{
			RecordLocalFailure(TEXT("DeclineSkill"), Rules.ErrorMessage);
			return;
		}
		FMatchPlayAuthoritativeDeclineSkillRequest Request;
		Request.RequestingSide = InteractionView.ExpectedActingPlayer;
		RecordCommandResult(
			TEXT("DeclineSkill"),
			Host->DeclineSkill(Rules.Snapshot, Request));
		break;
	}
	case EFMCodexLocalMatchInteractionCategory::SelectRunner:
	{
		FMatchPlayAuthoritativeDeclineRunnerRequest Request;
		Request.RequestingSide = InteractionView.ExpectedActingPlayer;
		RecordCommandResult(TEXT("DeclineRunner"), Host->DeclineRunner(Request));
		break;
	}
	case EFMCodexLocalMatchInteractionCategory::SelectHelper:
	{
		FMatchPlayAuthoritativeDeclineHelperRequest Request;
		Request.RequestingSide = InteractionView.ExpectedActingPlayer;
		RecordCommandResult(TEXT("DeclineHelper"), Host->DeclineHelper(Request));
		break;
	}
	default:
		RecordLocalFailure(
			TEXT("DeclineSelection"),
			TEXT("The current authoritative stage has no decline contract."));
		break;
	}
}

void AFMCodexLocalMatchPlayerController::ResolveNoLegalCurrentSelection()
{
	AFMCodexLocalMatchHostGameMode* Host = FindLocalMatchHost();
	if (Host == nullptr)
	{
		RecordLocalFailure(TEXT("ResolveNoLegalSelection"), TEXT("Host unavailable."));
		return;
	}

	switch (InteractionView.InteractionCategory)
	{
	case EFMCodexLocalMatchInteractionCategory::SelectCarrier:
		RecordCommandResult(
			TEXT("ResolveNoLegalCarrier"), Host->ResolveNoLegalCarrier());
		break;
	case EFMCodexLocalMatchInteractionCategory::SelectMarker:
		RecordCommandResult(
			TEXT("ResolveNoLegalMarker"), Host->ResolveNoLegalMarker());
		break;
	case EFMCodexLocalMatchInteractionCategory::SelectSkill:
	{
		const auto Rules = Host->GetSkillRuleSnapshot();
		if (!Rules.bSuccess)
		{
			RecordLocalFailure(TEXT("ResolveNoLegalSkill"), Rules.ErrorMessage);
			return;
		}
		RecordCommandResult(
			TEXT("ResolveNoLegalSkill"),
			Host->ResolveNoLegalSkill(Rules.Snapshot));
		break;
	}
	case EFMCodexLocalMatchInteractionCategory::SelectRunner:
		RecordCommandResult(
			TEXT("ResolveNoLegalRunner"), Host->ResolveNoLegalRunner());
		break;
	case EFMCodexLocalMatchInteractionCategory::SelectHelper:
		RecordCommandResult(
			TEXT("ResolveNoLegalHelper"), Host->ResolveNoLegalHelper());
		break;
	default:
		RecordLocalFailure(
			TEXT("ResolveNoLegalSelection"),
			TEXT("No authoritative no-legal operation is available now."));
		break;
	}
}

void AFMCodexLocalMatchPlayerController::SubmitBranchIntent(
	const EMatchPlayElectiveBranchIntent Intent)
{
	AFMCodexLocalMatchHostGameMode* Host = FindLocalMatchHost();
	if (Host == nullptr)
	{
		RecordLocalFailure(TEXT("SubmitBranchIntent"), TEXT("Host unavailable."));
		return;
	}
	FMatchPlayAuthoritativeSubmitBranchIntentRequest Request;
	Request.RequestingSide = InteractionView.ExpectedActingPlayer;
	Request.Intent = Intent;
	RecordCommandResult(
		TEXT("SubmitBranchIntent"), Host->SubmitBranchIntent(Request));
}

void AFMCodexLocalMatchPlayerController::SubmitOneOnOneShotChoice(
	const EMatchPlayThroughBallOneOnOneShotChoice Choice)
{
	AFMCodexLocalMatchHostGameMode* Host = FindLocalMatchHost();
	if (Host == nullptr)
	{
		RecordLocalFailure(
			TEXT("SubmitThroughBallOneOnOneShotChoice"),
			TEXT("Host unavailable."));
		return;
	}
	FMatchPlayAuthoritativeSubmitThroughBallOneOnOneShotChoiceRequest Request;
	Request.RequestingSide = InteractionView.ExpectedActingPlayer;
	Request.Choice = Choice;
	RecordCommandResult(
		TEXT("SubmitThroughBallOneOnOneShotChoice"),
		Host->SubmitThroughBallOneOnOneShotChoice(Request));
}

void AFMCodexLocalMatchPlayerController::ContinueResolution()
{
	AFMCodexLocalMatchHostGameMode* Host = FindLocalMatchHost();
	if (Host == nullptr)
	{
		RecordLocalFailure(TEXT("ContinueResolution"), TEXT("Host unavailable."));
		return;
	}
	const auto Snapshot = Host->GetMatchSnapshot();
	const auto Rules = Host->GetSkillRuleSnapshot();
	if (!Snapshot.bSuccess || !Rules.bSuccess
		|| !Snapshot.Snapshot.bHasCurrentAttack)
	{
		RecordLocalFailure(
			TEXT("ContinueResolution"),
			!Snapshot.bSuccess ? Snapshot.ErrorMessage
				: !Rules.bSuccess ? Rules.ErrorMessage
					: TEXT("No current attack is available to continue."));
		return;
	}

	const FMatchPlayCurrentAttackState& Attack =
		Snapshot.Snapshot.CurrentAttack;
	if (!Attack.bHasResolutionSession)
	{
		RecordCommandResult(
			TEXT("BeginResolutionSession"), Host->BeginResolutionSession());
		return;
	}

	const FMatchPlayCurrentAttackResolutionSession& Session =
		Attack.ResolutionSession;
	if (Session.Stage == EMatchPlayCurrentAttackResolutionStage::AwaitingRoute)
	{
		if (Session.Bundle.Binding.ActionType == ESkillRuleType::LongShot
			|| Session.Bundle.Binding.ActionType
				== ESkillRuleType::CutInsideShot)
		{
			RecordCommandResult(
				TEXT("ResolveIntentDeterminedRoute"),
				Host->ResolveIntentDeterminedRoute());
		}
		else
		{
			RecordCommandResult(
				TEXT("ResolveInitialRoute"), Host->ResolveInitialRoute());
		}
		return;
	}

	if (Session.Stage != EMatchPlayCurrentAttackResolutionStage::RouteResolved
		|| !Session.bHasActualBranch)
	{
		RecordLocalFailure(
			TEXT("ContinueResolution"),
			TEXT("The snapshot does not identify one canonical resolution continuation."));
		return;
	}

	const auto Progress =
		FMatchPlayCurrentAttackPostRouteRollProgressQuery::Evaluate(Session);
	if (!Progress.bIsCanonical)
	{
		RecordLocalFailure(TEXT("ContinueResolution"), Progress.ErrorMessage);
		return;
	}

	switch (Session.ActualBranch.ActionType)
	{
	case ESkillRuleType::Cross:
		if (!Progress.bContractComplete)
		{
			RecordCommandResult(
				TEXT("ResolveCrossPostRoutePlan"),
				Host->ResolveCrossPostRoutePlan());
		}
		else
		{
			RecordCommandResult(
				TEXT("ApplyCrossTerminalResolution"),
				Host->ApplyCrossTerminalResolution());
		}
		return;

	case ESkillRuleType::PassControl:
		if (!Progress.bContractComplete)
		{
			RecordCommandResult(
				TEXT("ResolvePassControlPostRoutePlan"),
				Host->ResolvePassControlPostRoutePlan());
		}
		else
		{
			RecordCommandResult(
				TEXT("ApplyPassControlTerminalResolution"),
				Host->ApplyPassControlTerminalResolution());
		}
		return;

	case ESkillRuleType::LongShot:
	case ESkillRuleType::CutInsideShot:
		if (!Progress.bContractComplete)
		{
			const bool bDeadCorner =
				(Session.ActualBranch.ActionType == ESkillRuleType::LongShot
					&& Session.ActualBranch.LongShot
						== EMatchPlayLongShotActualBranch::DeadCorner)
				|| (Session.ActualBranch.ActionType
						== ESkillRuleType::CutInsideShot
					&& Session.ActualBranch.CutInsideShot
						== EMatchPlayCutInsideShotActualBranch::DeadCorner);
			if (bDeadCorner)
			{
				RecordCommandResult(
					TEXT("ResolveDeadCornerPostRouteDecision"),
					Host->ResolveDeadCornerPostRouteDecision());
			}
			else
			{
				RecordCommandResult(
					TEXT("ResolveDirectShotPostRouteDecisionOrPlan"),
					Host->ResolveDirectShotPostRouteDecisionOrPlan());
			}
		}
		else
		{
			RecordCommandResult(
				TEXT("ApplyShotTerminalResolution"),
				Host->ApplyShotTerminalResolution());
		}
		return;

	case ESkillRuleType::ThroughBall:
		break;

	default:
		RecordLocalFailure(
			TEXT("ContinueResolution"),
			TEXT("The resolved action family has no LocalPlay continuation mapping."));
		return;
	}

	if (Session.ActualBranch.ThroughBall
		== EMatchPlayThroughBallActualBranch::Feet)
	{
		if (!Progress.bContractComplete)
		{
			RecordCommandResult(
				TEXT("ResolveThroughBallFeetPostRoutePlan"),
				Host->ResolveThroughBallFeetPostRoutePlan());
		}
		else
		{
			RecordCommandResult(
				TEXT("ApplyThroughBallTerminalResolution"),
				Host->ApplyThroughBallTerminalResolution());
		}
		return;
	}

	const EMatchPlayThroughBallOneOnOneShotChoice Choice =
		Session.ThroughBallOneOnOneShotChoice;
	if (Choice == EMatchPlayThroughBallOneOnOneShotChoice::ChipShot)
	{
		if (Session.PostRouteRollProgress.Phase
				!= EMatchPlayCurrentAttackPostRouteRollPhase::OneOnOneChipShot
			|| !Progress.bContractComplete)
		{
			RecordCommandResult(
				TEXT("ResolveThroughBallOneOnOneChipShotDecision"),
				Host->ResolveThroughBallOneOnOneChipShotDecision());
		}
		else
		{
			RecordCommandResult(
				TEXT("ApplyThroughBallTerminalResolution"),
				Host->ApplyThroughBallTerminalResolution());
		}
		return;
	}
	if (Choice == EMatchPlayThroughBallOneOnOneShotChoice::DirectShot)
	{
		if (Session.PostRouteRollProgress.Phase
				!= EMatchPlayCurrentAttackPostRouteRollPhase::OneOnOneDirectShot
			|| !Progress.bContractComplete)
		{
			RecordCommandResult(
				TEXT("ResolveThroughBallOneOnOneDirectShotPostRoutePlan"),
				Host->ResolveThroughBallOneOnOneDirectShotPostRoutePlan());
		}
		else
		{
			RecordCommandResult(
				TEXT("ApplyThroughBallTerminalResolution"),
				Host->ApplyThroughBallTerminalResolution());
		}
		return;
	}

	if (Session.ActualBranch.ThroughBall
		== EMatchPlayThroughBallActualBranch::AntiOffside)
	{
		if (!Progress.bContractComplete)
		{
			RecordCommandResult(
				TEXT("ResolveThroughBallAntiOffsideDecision"),
				Host->ResolveThroughBallAntiOffsideDecision());
		}
		else
		{
			RecordCommandResult(
				TEXT("ApplyThroughBallTerminalResolution"),
				Host->ApplyThroughBallTerminalResolution());
		}
		return;
	}

	if (Session.ActualBranch.ThroughBall
		== EMatchPlayThroughBallActualBranch::BehindDefense)
	{
		if (!Progress.bContractComplete)
		{
			if (Session.PostRouteRollProgress.Phase
				== EMatchPlayCurrentAttackPostRouteRollPhase::BehindDefenseP2)
			{
				RecordCommandResult(
					TEXT("ResolveThroughBallBehindDefenseP2Decision"),
					Host->ResolveThroughBallBehindDefenseP2Decision());
			}
			else
			{
				RecordCommandResult(
					TEXT("ResolveThroughBallBehindDefenseP1DecisionOrPlan"),
					Host->ResolveThroughBallBehindDefenseP1DecisionOrPlan());
			}
			return;
		}

		if (Session.PostRouteRollProgress.Phase
			== EMatchPlayCurrentAttackPostRouteRollPhase::PrimaryBranch)
		{
			const auto Formula =
				FMatchPlayCurrentAttackResolveThroughBallBehindDefenseP1FormulaOrchestrator
					::Resolve(Snapshot.Snapshot, &Rules.Snapshot);
			if (Formula.bSuccess
				&& Formula.FormulaResolutionResult.bContinueResolution)
			{
				RecordCommandResult(
					TEXT("ResolveThroughBallBehindDefenseP2Decision"),
					Host->ResolveThroughBallBehindDefenseP2Decision());
			}
			else
			{
				RecordCommandResult(
					TEXT("ApplyThroughBallTerminalResolution"),
					Host->ApplyThroughBallTerminalResolution());
			}
			return;
		}

		RecordCommandResult(
			TEXT("ApplyThroughBallTerminalResolution"),
			Host->ApplyThroughBallTerminalResolution());
		return;
	}

	RecordLocalFailure(
		TEXT("ContinueResolution"),
		TEXT("The ThroughBall snapshot does not identify a supported branch."));
}

void AFMCodexLocalMatchPlayerController::RebuildControlSurface()
{
	if (SurfaceContainer.IsValid())
	{
		SurfaceContainer->SetContent(BuildControlSurface());
	}
}

TSharedRef<SWidget> AFMCodexLocalMatchPlayerController::BuildControlSurface()
{
	using namespace FMCodexLocalMatchPlayerController;

	TSharedRef<SVerticalBox> Content = SNew(SVerticalBox);
	auto AddText = [&Content](const FString& Text)
	{
		Content->AddSlot().AutoHeight().Padding(2.0f)
		[
			SNew(STextBlock)
			.Text(FText::FromString(Text))
			.ColorAndOpacity(FLinearColor::White)
		];
	};
	auto AddButton = [&Content](const TSharedRef<SWidget>& Button)
	{
		Content->AddSlot().AutoHeight().Padding(2.0f)[Button];
	};

	AddText(TEXT("FMCodex - Developer Hot-seat Control Surface"));
	AddText(FString::Printf(
		TEXT("Status: %s | Score A %d - %d B"),
		InteractionView.bMatchActive ? TEXT("Active") : TEXT("No Match"),
		InteractionView.PlayerAScore,
		InteractionView.PlayerBScore));
	AddText(FString::Printf(
		TEXT("Attacking: %s | Expected: %s"),
		*FFMCodexLocalMatchInteractionViewBuilder::ToString(
			InteractionView.CurrentAttackingPlayer),
		*FFMCodexLocalMatchInteractionViewBuilder::ToString(
			InteractionView.ExpectedActingPlayer)));
	AddText(FString::Printf(
		TEXT("Phase: %s | Interaction: %s | Attack #%lld | AP %d"),
		*FFMCodexLocalMatchInteractionViewBuilder::ToString(
			InteractionView.MajorPhase),
		*FFMCodexLocalMatchInteractionViewBuilder::ToString(
			InteractionView.InteractionCategory),
		InteractionView.AttackSequence,
		InteractionView.ActionPoint));
	AddText(FString::Printf(
		TEXT("CurrentAttack: %s"),
		InteractionView.bCurrentAttackActive ? TEXT("Active") : TEXT("None")));
	if (InteractionView.bMatchEnded)
	{
		AddText(FString(TEXT("Result: "))
			+ FFMCodexLocalMatchInteractionViewBuilder::ToString(
				InteractionView.MatchResult));
	}
	AddText(FString::Printf(
		TEXT("Last: %s | Host=%s | Accepted=%s | Domain=%s | %s"),
		*LastDiagnostic.CommandName,
		LastDiagnostic.bHostSuccess ? TEXT("OK") : TEXT("FAIL"),
		LastDiagnostic.bAuthoritativeAccepted ? TEXT("YES") : TEXT("NO"),
		LastDiagnostic.bAuthoritativeSuccess ? TEXT("OK") : TEXT("FAIL"),
		*LastDiagnostic.Message));
	if (!InteractionView.Diagnostic.IsEmpty())
	{
		AddText(TEXT("View diagnostic: ") + InteractionView.Diagnostic);
	}

	for (const FMatchPlayDeploymentPlacement& Placement
		: InteractionView.DeploymentPlacements)
	{
		AddText(FString::Printf(
			TEXT("Placed: %s / %s -> %s"),
			*FFMCodexLocalMatchInteractionViewBuilder::ToString(
				Placement.PlayerSide),
			*Placement.CardId.ToString(),
			*Placement.SlotId.ToString()));
	}
	for (const FFMCodexLocalMatchRollView& Roll : InteractionView.AcceptedRolls)
	{
		AddText(FString::Printf(
			TEXT("D6: %s = %d"), *Roll.Purpose, Roll.RawD6));
	}

	switch (InteractionView.InteractionCategory)
	{
	case EFMCodexLocalMatchInteractionCategory::StartMatch:
		AddButton(MakeButton(TEXT("Start New Local Match"), [this]()
		{
			StartNewDemoMatch();
		}));
		break;
	case EFMCodexLocalMatchInteractionCategory::BeginAttack:
		AddButton(MakeButton(TEXT("Begin Ordinary Attack (AP 6)"), [this]()
		{
			BeginDemoOrdinaryAttack();
		}));
		break;
	case EFMCodexLocalMatchInteractionCategory::Deploy:
		for (const FFMCodexLocalMatchDeploymentOption& Option
			: InteractionView.DeploymentOptions)
		{
			const FString Label = FString::Printf(
				TEXT("%s %s -> %s"),
				Option.bGoalkeeper ? TEXT("Deploy GK") : TEXT("Deploy"),
				*Option.CardId.ToString(),
				*Option.SlotId.ToString());
			AddButton(MakeButton(Label, [this, Option]()
			{
				if (Option.bGoalkeeper)
				{
					DeployGoalkeeper(Option.SlotId);
				}
				else
				{
					DeployOrdinary(Option.CardId, Option.SlotId);
				}
			}));
		}
		AddButton(MakeButton(TEXT("Finish Deployment"), [this]()
		{
			FinishDeployment();
		}));
		break;
	case EFMCodexLocalMatchInteractionCategory::SelectCarrier:
	case EFMCodexLocalMatchInteractionCategory::SelectMarker:
	case EFMCodexLocalMatchInteractionCategory::SelectSkill:
	case EFMCodexLocalMatchInteractionCategory::SelectRunner:
	case EFMCodexLocalMatchInteractionCategory::SelectHelper:
		for (const FFMCodexLocalMatchSelectionOption& Option
			: InteractionView.SelectionOptions)
		{
			AddButton(MakeButton(Option.Label, [this, Option]()
			{
				switch (InteractionView.InteractionCategory)
				{
				case EFMCodexLocalMatchInteractionCategory::SelectCarrier:
					SubmitCarrier(Option.Id); break;
				case EFMCodexLocalMatchInteractionCategory::SelectMarker:
					SubmitMarker(Option.Id); break;
				case EFMCodexLocalMatchInteractionCategory::SelectSkill:
					SubmitSkill(Option.Id); break;
				case EFMCodexLocalMatchInteractionCategory::SelectRunner:
					SubmitRunner(Option.Id); break;
				case EFMCodexLocalMatchInteractionCategory::SelectHelper:
					SubmitHelper(Option.Id); break;
				default: break;
				}
			}));
		}
		if (InteractionView.bCanDecline)
		{
			AddButton(MakeButton(TEXT("Decline"), [this]()
			{
				DeclineCurrentSelection();
			}));
		}
		if (InteractionView.bCanResolveNoLegalChoice)
		{
			AddButton(MakeButton(TEXT("Resolve No Legal Choice"), [this]()
			{
				ResolveNoLegalCurrentSelection();
			}));
		}
		break;
	case EFMCodexLocalMatchInteractionCategory::SelectBranchIntent:
		for (const EMatchPlayElectiveBranchIntent Intent
			: InteractionView.BranchIntentOptions)
		{
			AddButton(MakeButton(IntentLabel(Intent), [this, Intent]()
			{
				SubmitBranchIntent(Intent);
			}));
		}
		break;
	case EFMCodexLocalMatchInteractionCategory::SelectOneOnOneShot:
		for (const EMatchPlayThroughBallOneOnOneShotChoice Choice
			: InteractionView.OneOnOneOptions)
		{
			AddButton(MakeButton(ShotChoiceLabel(Choice), [this, Choice]()
			{
				SubmitOneOnOneShotChoice(Choice);
			}));
		}
		break;
	case EFMCodexLocalMatchInteractionCategory::ContinueResolution:
		AddButton(MakeButton(TEXT("Continue Resolution"), [this]()
		{
			ContinueResolution();
		}));
		break;
	default:
		break;
	}

	return SNew(SScrollBox)
		+ SScrollBox::Slot()
		[
			Content
		];
}
