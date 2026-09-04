#include "FMCodexLocalMatchHostGameMode.h"
#include "FMCodexInteractionPanelWidget.h"
#include "FMCodexInlineResolutionFormulaSurfaceWidget.h"
#include "FMCodexLocalMatchPlayerController.h"
#include "FMCodexLocalMatchScreenWidget.h"
#include "FMCodexLocalMatchUMGPresentation.h"
#include "FMCodexRollReelWidget.h"
#include "FMCodexTacticalDetailPresentation.h"
#include "FMCodexMatchHeaderWidget.h"
#include "../CoreRules/CrossSelectionQuery.h"
#include "../CoreRules/TacticalRuleDescription.h"

#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/ScaleBox.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/Widget.h"
#include "Misc/AutomationTest.h"
#include "Serialization/MemoryWriter.h"

namespace FMCodexFullD12SetPieceProductionTests
{
	using ECategory = EFMCodexLocalMatchInteractionCategory;
	using ETarget = EFMCodexLocalDevRollTarget;

	class FScopedWorld final
	{
	public:
		FScopedWorld()
		{
			World = UWorld::CreateWorld(EWorldType::Game, false);
			if (World == nullptr || GEngine == nullptr)
			{
				return;
			}
			FWorldContext& Context =
				GEngine->CreateNewWorldContext(EWorldType::Game);
			Context.SetCurrentWorld(World);
			Host = World->SpawnActor<AFMCodexLocalMatchHostGameMode>();
			Controller =
				World->SpawnActor<AFMCodexLocalMatchPlayerController>();
			if (Controller != nullptr)
			{
				Controller->RefreshPresentation();
			}
		}

		~FScopedWorld()
		{
			if (World != nullptr)
			{
				if (GEngine != nullptr)
				{
					GEngine->DestroyWorldContext(World);
				}
				World->DestroyWorld(false);
			}
		}

		AFMCodexLocalMatchHostGameMode* Host = nullptr;
		AFMCodexLocalMatchPlayerController* Controller = nullptr;

	private:
		UWorld* World = nullptr;
	};

	bool SetOverride(
		AFMCodexLocalMatchPlayerController& Controller,
		const ETarget Target,
		const int32 Value)
	{
		FFMCodexLocalDevRollOverrideRequest Request;
		Request.Target = Target;
		Request.Value = Value;
		return Controller.SetLocalDevRollOverride(Request).bSuccess;
	}

	TArray<uint8> SerializeState(const FMatchPlayState& State)
	{
		TArray<uint8> Bytes;
		FMemoryWriter Writer(Bytes);
		FMatchPlayState Copy = State;
		FMatchPlayState::StaticStruct()->SerializeItem(Writer, &Copy, nullptr);
		return Bytes;
	}

	bool StartSetPiece(
		FAutomationTestBase& Test,
		FScopedWorld& World,
		const int32 TypeD6,
		const ESetPieceSelectedType ExpectedType)
	{
		if (World.Controller == nullptr || World.Host == nullptr)
		{
			Test.AddError(TEXT("LocalPlay world was unavailable."));
			return false;
		}
		World.Controller->StartNewDemoMatch();
		if (!World.Controller->GetLastDiagnostic().bHostSuccess
			|| !SetOverride(*World.Controller, ETarget::FullD12, 9))
		{
			Test.AddError(TEXT("Set Piece fixture could not start Full D12."));
			return false;
		}
		World.Controller->RollDemoTacticalPoints();
		const auto& Entry = World.Controller->GetInteractionView();
		if (!World.Controller->GetLastDiagnostic().bHostSuccess
			|| Entry.RouteKind != EMatchPlayCurrentAttackRouteKind::SetPiece
			|| Entry.RawInitialD12 != 9
			|| Entry.InteractionCategory != ECategory::RollSetPieceType
			|| !SetOverride(*World.Controller, ETarget::SetPieceType, TypeD6))
		{
			Test.AddError(TEXT("Full D12 did not project the Set Piece type CTA."));
			return false;
		}
		World.Controller->SubmitProjectedPrimaryPlayerIntent();
		if (!World.Controller->GetLastDiagnostic().bHostSuccess
			|| World.Controller->GetInteractionView().SetPieceType
				!= ExpectedType
			|| World.Controller->GetInteractionView().RawSetPieceTypeD6
				!= TypeD6)
		{
			Test.AddError(TEXT("Set Piece type D6 did not reach the expected route."));
			return false;
		}
		return true;
	}

	int32 AttributeValue(
		const FFMCodexLocalMatchCardView& Card,
		const FString& Label)
	{
		const FFMCodexLocalMatchCardView::FAttribute* Attribute =
			Card.AttributeValues.FindByPredicate(
				[&Label](const FFMCodexLocalMatchCardView::FAttribute& Candidate)
				{
					return Candidate.CanonicalLabel == Label;
				});
		return Attribute == nullptr ? 0 : Attribute->Value;
	}

	FString CardDisplayName(
		const FFMCodexLocalMatchInteractionView& View,
		const EInitialTurnOrderPlayer Side,
		const FName CardId)
	{
		const TArray<FFMCodexLocalMatchCardView>& Roster =
			Side == EInitialTurnOrderPlayer::PlayerA
				? View.PlayerACardRoster : View.PlayerBCardRoster;
		const FFMCodexLocalMatchCardView* Card = Roster.FindByPredicate(
			[CardId](const FFMCodexLocalMatchCardView& Candidate)
			{
				return Candidate.CardId == CardId;
			});
		return Card == nullptr ? FString() : Card->DisplayLabel;
	}

	FString GoalkeeperDisplayName(
		const FFMCodexLocalMatchInteractionView& View,
		const EInitialTurnOrderPlayer Side)
	{
		const TArray<FFMCodexLocalMatchCardView>& Roster =
			Side == EInitialTurnOrderPlayer::PlayerA
				? View.PlayerACardRoster : View.PlayerBCardRoster;
		const FFMCodexLocalMatchCardView* Goalkeeper = Roster.FindByPredicate(
			[](const FFMCodexLocalMatchCardView& Card)
			{
				return Card.bGoalkeeper;
			});
		return Goalkeeper == nullptr ? FString() : Goalkeeper->DisplayLabel;
	}

	FName ChooseCarrier(
		AFMCodexLocalMatchPlayerController& Controller,
		const bool bRequireShortAngled)
	{
		const auto& View = Controller.GetInteractionView();
		const TArray<FFMCodexLocalMatchCardView>& Roster =
			View.CurrentAttackingPlayer == EInitialTurnOrderPlayer::PlayerA
				? View.PlayerACardRoster : View.PlayerBCardRoster;
		for (const FName CardId : View.LegalSetPieceCardIds)
		{
			const FFMCodexLocalMatchCardView* Card = Roster.FindByPredicate(
				[CardId](const FFMCodexLocalMatchCardView& Candidate)
				{
					return Candidate.CardId == CardId;
				});
			if (Card != nullptr && (!bRequireShortAngled
				|| AttributeValue(*Card, TEXT("SHO"))
					+ AttributeValue(*Card, TEXT("PAS")) >= 8))
			{
				return CardId;
			}
		}
		return NAME_None;
	}

	bool BindCarrier(
		FAutomationTestBase& Test,
		AFMCodexLocalMatchPlayerController& Controller,
		const bool bRequireShortAngled)
	{
		const FName CardId = ChooseCarrier(Controller, bRequireShortAngled);
		if (CardId.IsNone())
		{
			Test.AddError(TEXT("No required legal Set Piece Carrier was projected."));
			return false;
		}
		Controller.ToggleSetPieceDraftCard(CardId);
		if (Controller.GetInteractionView().DraftSetPieceCarrierCardId != CardId
			|| Controller.GetInteractionView().InteractionCategory
				!= ECategory::ConfirmSetPieceCarrier)
		{
			Test.AddError(TEXT("Carrier click did not remain a local correlated draft."));
			return false;
		}
		Controller.ConfirmSetPieceDraft();
		if (!Controller.GetLastDiagnostic().bHostSuccess
			|| Controller.GetInteractionView().InteractionCategory
				!= ECategory::SelectSetPieceMethod
			|| Controller.GetInteractionView().SetPieceCarrier.CardId != CardId)
		{
			Test.AddError(TEXT("Carrier confirmation did not reach Authority."));
			return false;
		}
		return true;
	}

	void CheckDecisiveScoreReveal(FAutomationTestBase& Test, UFMCodexLocalMatchScreenWidget& Screen,
		const FFMCodexUMGMatchHeaderViewModel& Before, const FFMCodexUMGMatchScreenViewModel& After)
	{
		auto CheckOld = [&](const TCHAR* Label)
		{
			const auto& Header = Screen.GetMatchHeader()->GetPresentation();
			Test.TestTrue(Label, Header.PlayerAScoreLabel == Before.PlayerAScoreLabel
				&& Header.PlayerBScoreLabel == Before.PlayerBScoreLabel
				&& !Screen.GetInlineFormulaSurface()->GetPresentation().bNarrativeAvailable);
		};
		CheckOld(TEXT("Cycling keeps the last disclosed score"));
		Screen.AdvanceInlineFormulaRevealForTesting(1.35f);
		Test.TestEqual(TEXT("Decisive reel is still settling"), Screen.GetInlineFormulaRevealPhase(),
			EFMCodexUMGInlineFormulaRevealPhase::Settling);
		CheckOld(TEXT("Settling cannot disclose authoritative score"));
		Screen.AdvanceInlineFormulaRevealForTesting(0.12f);
		CheckOld(TEXT("Early ResultHold still conceals score and narrative"));
		Screen.AdvanceInlineFormulaRevealForTesting(0.23f);
		Screen.RefreshFromPresentation(After);
		CheckOld(TEXT("Formula disclosure and repeated snapshot do not disclose score early"));
		Screen.AdvanceInlineFormulaRevealForTesting(0.15f);
		const auto& Header = Screen.GetMatchHeader()->GetPresentation();
		Test.TestTrue(TEXT("Goal narrative and authoritative score are disclosed in the same refresh"),
			Screen.GetInlineFormulaSurface()->GetPresentation().bNarrativeAvailable
				&& Header.PlayerAScoreLabel == After.Header.PlayerAScoreLabel
				&& Header.PlayerBScoreLabel == After.Header.PlayerBScoreLabel);
		Screen.AdvanceInlineFormulaRevealForTesting(5.0f);
	}

	enum class ECarrierScenario : uint8
	{
		ShortDirect,
		ShortAngled,
		LongDirect,
		LongPower,
		PenaltyDirect,
		PenaltyPanenka
	};

	bool RunCarrierScenario(
		FAutomationTestBase& Test,
		const ECarrierScenario Scenario)
	{
		const bool bShort = Scenario == ECarrierScenario::ShortDirect
			|| Scenario == ECarrierScenario::ShortAngled;
		const bool bLong = Scenario == ECarrierScenario::LongDirect
			|| Scenario == ECarrierScenario::LongPower;
		const int32 TypeD6 = bShort ? 5 : bLong ? 3 : 6;
		const ESetPieceSelectedType Type = bShort
			? ESetPieceSelectedType::ShortFreeKick
			: bLong ? ESetPieceSelectedType::LongFreeKick
				: ESetPieceSelectedType::Penalty;
		FScopedWorld World;
		if (!StartSetPiece(Test, World, TypeD6, Type)
			|| !BindCarrier(Test, *World.Controller,
				Scenario == ECarrierScenario::ShortAngled))
		{
			return false;
		}

		switch (Scenario)
		{
		case ECarrierScenario::ShortDirect:
		case ECarrierScenario::ShortAngled:
			World.Controller->SubmitShortFreeKickMethod(
				Scenario == ECarrierScenario::ShortDirect
					? EMatchPlayShortFreeKickMethod::Direct
					: EMatchPlayShortFreeKickMethod::Angled);
			break;
		case ECarrierScenario::LongDirect:
		case ECarrierScenario::LongPower:
			World.Controller->SubmitLongFreeKickMethod(
				Scenario == ECarrierScenario::LongDirect
					? EMatchPlayLongFreeKickMethod::Direct
					: EMatchPlayLongFreeKickMethod::Power);
			break;
		case ECarrierScenario::PenaltyDirect:
		case ECarrierScenario::PenaltyPanenka:
			World.Controller->SubmitPenaltyMethod(
				Scenario == ECarrierScenario::PenaltyDirect
					? EMatchPlayPenaltyMethod::Direct
					: EMatchPlayPenaltyMethod::Panenka);
			break;
		}
		if (!World.Controller->GetLastDiagnostic().bHostSuccess)
		{
			Test.AddError(TEXT("Set Piece method selection failed."));
			return false;
		}

		ETarget FirstTarget = ETarget::None;
		ETarget SecondTarget = ETarget::None;
		if (Scenario == ECarrierScenario::ShortDirect)
		{
			FirstTarget = ETarget::ShortFreeKickDirectAttack;
			SecondTarget = ETarget::ShortFreeKickDirectDefense;
		}
		else if (Scenario == ECarrierScenario::ShortAngled)
		{
			FirstTarget = ETarget::ShortFreeKickAngledA;
			SecondTarget = ETarget::ShortFreeKickAngledB;
		}
		else if (Scenario == ECarrierScenario::LongDirect)
		{
			FirstTarget = ETarget::LongFreeKickDirectAttack;
			SecondTarget = ETarget::LongFreeKickDirectDefense;
		}
		else if (Scenario == ECarrierScenario::LongPower)
		{
			FirstTarget = ETarget::LongFreeKickPowerA;
			SecondTarget = ETarget::LongFreeKickPowerB;
		}
		else if (Scenario == ECarrierScenario::PenaltyDirect)
		{
			FirstTarget = ETarget::PenaltyDirectAttack;
			SecondTarget = ETarget::PenaltyDirectDefense;
		}
		else
		{
			FirstTarget = ETarget::PenaltyPanenka;
		}
		if (!SetOverride(*World.Controller, FirstTarget, 6)
			|| (SecondTarget != ETarget::None
				&& !SetOverride(*World.Controller, SecondTarget, 1)))
		{
			Test.AddError(TEXT("Set Piece method roll overrides were rejected."));
			return false;
		}
		World.Controller->SubmitProjectedPrimaryPlayerIntent();
		if (!World.Controller->GetLastDiagnostic().bHostSuccess)
		{
			Test.AddError(TEXT("First Set Piece method roll failed."));
			return false;
		}
		if (Scenario == ECarrierScenario::ShortDirect
			|| Scenario == ECarrierScenario::LongDirect
			|| Scenario == ECarrierScenario::PenaltyDirect)
		{
			if (World.Controller->GetInteractionView().bTerminalPendingAdvance)
			{
				Test.AddError(TEXT("High direct attack roll skipped the defense roll."));
				return false;
			}
			World.Controller->SubmitProjectedPrimaryPlayerIntent();
		}
		const auto& Terminal = World.Controller->GetInteractionView();
		if (!World.Controller->GetLastDiagnostic().bHostSuccess
			|| !Terminal.bTerminalPendingAdvance
			|| !Terminal.bHasSetPieceOutcome
			|| Terminal.InteractionCategory != ECategory::AdvanceAfterTerminal)
		{
			Test.AddError(TEXT("Set Piece method did not reach explicit terminal."));
			return false;
		}
		return true;
	}

	bool ResolveScenarioPresentation(
		FAutomationTestBase& Test,
		const ECarrierScenario Scenario,
		const int32 FirstD6,
		const int32 SecondD6,
		FFMCodexUMGMatchScreenViewModel& OutPresentation)
	{
		const bool bShort = Scenario == ECarrierScenario::ShortDirect
			|| Scenario == ECarrierScenario::ShortAngled;
		const bool bLong = Scenario == ECarrierScenario::LongDirect
			|| Scenario == ECarrierScenario::LongPower;
		FScopedWorld World;
		if (!StartSetPiece(Test, World, bShort ? 5 : bLong ? 3 : 6,
			bShort ? ESetPieceSelectedType::ShortFreeKick
				: bLong ? ESetPieceSelectedType::LongFreeKick
					: ESetPieceSelectedType::Penalty)
			|| !BindCarrier(Test, *World.Controller,
				Scenario == ECarrierScenario::ShortAngled))
		{
			return false;
		}

		ETarget FirstTarget = ETarget::None;
		ETarget SecondTarget = ETarget::None;
		bool bDirect = false;
		switch (Scenario)
		{
		case ECarrierScenario::ShortDirect:
			World.Controller->SubmitShortFreeKickMethod(
				EMatchPlayShortFreeKickMethod::Direct);
			FirstTarget = ETarget::ShortFreeKickDirectAttack;
			SecondTarget = ETarget::ShortFreeKickDirectDefense;
			bDirect = true;
			break;
		case ECarrierScenario::ShortAngled:
			World.Controller->SubmitShortFreeKickMethod(
				EMatchPlayShortFreeKickMethod::Angled);
			FirstTarget = ETarget::ShortFreeKickAngledA;
			SecondTarget = ETarget::ShortFreeKickAngledB;
			break;
		case ECarrierScenario::LongDirect:
			World.Controller->SubmitLongFreeKickMethod(
				EMatchPlayLongFreeKickMethod::Direct);
			FirstTarget = ETarget::LongFreeKickDirectAttack;
			SecondTarget = ETarget::LongFreeKickDirectDefense;
			bDirect = true;
			break;
		case ECarrierScenario::LongPower:
			World.Controller->SubmitLongFreeKickMethod(
				EMatchPlayLongFreeKickMethod::Power);
			FirstTarget = ETarget::LongFreeKickPowerA;
			SecondTarget = ETarget::LongFreeKickPowerB;
			break;
		case ECarrierScenario::PenaltyDirect:
			World.Controller->SubmitPenaltyMethod(
				EMatchPlayPenaltyMethod::Direct);
			FirstTarget = ETarget::PenaltyDirectAttack;
			SecondTarget = ETarget::PenaltyDirectDefense;
			bDirect = true;
			break;
		case ECarrierScenario::PenaltyPanenka:
			World.Controller->SubmitPenaltyMethod(
				EMatchPlayPenaltyMethod::Panenka);
			FirstTarget = ETarget::PenaltyPanenka;
			break;
		}
		if (!World.Controller->GetLastDiagnostic().bHostSuccess
			|| !SetOverride(*World.Controller, FirstTarget, FirstD6)
			|| (SecondTarget != ETarget::None && SecondD6 > 0
				&& !SetOverride(*World.Controller, SecondTarget, SecondD6)))
		{
			Test.AddError(TEXT("Presentation fixture could not configure method rolls."));
			return false;
		}
		World.Controller->SubmitProjectedPrimaryPlayerIntent();
		if (!World.Controller->GetLastDiagnostic().bHostSuccess)
		{
			Test.AddError(TEXT("Presentation fixture first roll failed."));
			return false;
		}
		if (bDirect
			&& !World.Controller->GetInteractionView().bTerminalPendingAdvance)
		{
			if (SecondD6 <= 0)
			{
				Test.AddError(TEXT("Opposed presentation fixture omitted defense D6."));
				return false;
			}
			World.Controller->SubmitProjectedPrimaryPlayerIntent();
		}
		if (!World.Controller->GetLastDiagnostic().bHostSuccess
			|| !World.Controller->GetInteractionView().bTerminalPendingAdvance)
		{
			Test.AddError(TEXT("Presentation fixture did not reach terminal."));
			return false;
		}
		OutPresentation = FFMCodexLocalMatchUMGPresentationBuilder::Build(
			World.Controller->GetInteractionView(),
			World.Controller->GetResolutionFeedback(), FString());
		return true;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexFullD12EntryAndAP1ProductionTest,
	"FMCodex.LocalPlay.FullD12SetPieceProduction.01.EntryAndAP1",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexFullD12EntryAndAP1ProductionTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexFullD12SetPieceProductionTests;
	(void)Parameters;

	FScopedWorld OrdinaryWorld;
	if (OrdinaryWorld.Controller == nullptr)
	{
		return false;
	}
	OrdinaryWorld.Controller->StartNewDemoMatch();
	TestTrue(TEXT("Ordinary D12 override accepted"),
		SetOverride(*OrdinaryWorld.Controller, ETarget::FullD12, 6));
	OrdinaryWorld.Controller->RollDemoTacticalPoints();
	const auto& Ordinary = OrdinaryWorld.Controller->GetInteractionView();
	TestTrue(TEXT("D12 2-8 enters the existing ordinary deployment path"),
		OrdinaryWorld.Controller->GetLastDiagnostic().bHostSuccess
			&& Ordinary.RawInitialD12 == 6
			&& Ordinary.ActionPoint == 6
			&& Ordinary.RouteKind == EMatchPlayCurrentAttackRouteKind::Ordinary
			&& Ordinary.InteractionCategory == ECategory::Deploy);

	FScopedWorld AP1World;
	if (AP1World.Controller == nullptr || AP1World.Host == nullptr)
	{
		return false;
	}
	AP1World.Controller->StartNewDemoMatch();
	TestTrue(TEXT("AP1 D12 and ejection index overrides accepted"),
		SetOverride(*AP1World.Controller, ETarget::FullD12, 1)
			&& SetOverride(*AP1World.Controller,
				ETarget::SendingOffSelection, 0));
	AP1World.Controller->RollDemoTacticalPoints();
	TestTrue(TEXT("D12 1 automatically resolves AP1 before presentation reveal"),
		AP1World.Controller->GetInteractionView().RawInitialD12 == 1
			&& AP1World.Controller->GetInteractionView().RouteKind
				== EMatchPlayCurrentAttackRouteKind::SendingOff
			&& AP1World.Controller->GetInteractionView().bTerminalPendingAdvance);
	const int32 AP1ScoreA = AP1World.Controller->GetInteractionView().PlayerAScore;
	const int32 AP1ScoreB = AP1World.Controller->GetInteractionView().PlayerBScore;
	const FMatchPlayState TerminalSnapshot =
		AP1World.Host->GetMatchSnapshot().Snapshot;
	TestTrue(TEXT("Player entry owns the command while coordinator owns AP1"),
		AP1World.Controller->GetLastDiagnostic().bHostSuccess
			&& AP1World.Controller->GetLastDiagnostic().CommandName
				== TEXT("RequestInitialActionPointRoll")
			&& AP1World.Controller->GetInteractionView().bTerminalPendingAdvance
			&& AP1World.Controller->GetInteractionView().PlayerAScore == AP1ScoreA
			&& AP1World.Controller->GetInteractionView().PlayerBScore == AP1ScoreB
			&& !AP1World.Controller->GetInteractionView()
				.SendingOffEjectedCardId.IsNone());
	AP1World.Controller->NotifyEntryRevealComplete();
	AP1World.Controller->NotifyEntryRevealComplete();
	TestTrue(TEXT("Reveal notifications are presentation-only and do not dispatch AP1"),
		AP1World.Controller->GetLastDiagnostic().CommandName
			== TEXT("RequestInitialActionPointRoll")
			&& AP1World.Host->GetMatchSnapshot().Snapshot
				.CurrentAttack.SendingOffRoute.EjectedCardId
				== TerminalSnapshot.CurrentAttack.SendingOffRoute.EjectedCardId);

	const auto AP1Presentation = FFMCodexLocalMatchUMGPresentationBuilder::Build(
		AP1World.Controller->GetInteractionView(),
		AP1World.Controller->GetResolutionFeedback(), FString());
	auto HasNoTacticalPips = [](const FFMCodexUMGCardRackViewModel& Rack)
	{
		return !Rack.Cells.ContainsByPredicate(
			[](const FFMCodexUMGCardRackCellViewModel& Cell)
			{
				return Cell.Card.bHasHandMicroTacticalMatch
					|| Cell.Card.HandMicroTacticalMatchCount != 0;
			});
	};
	const FName EjectedCardId = AP1World.Controller->GetInteractionView()
		.SendingOffEjectedCardId;
	const bool bEjectedCardIsGhostedOutOfHand =
		AP1Presentation.LocalRack.Cells.ContainsByPredicate(
			[EjectedCardId](const FFMCodexUMGCardRackCellViewModel& Cell)
			{
				return Cell.Card.CardId == EjectedCardId && Cell.bPlayed;
			})
		|| AP1Presentation.OpponentRack.Cells.ContainsByPredicate(
			[EjectedCardId](const FFMCodexUMGCardRackCellViewModel& Cell)
			{
				return Cell.Card.CardId == EjectedCardId && Cell.bPlayed;
			});
	TestTrue(TEXT("Ejected card is already removed from its visible hand"),
		bEjectedCardIsGhostedOutOfHand);
	UFMCodexLocalMatchScreenWidget* AP1Screen =
		NewObject<UFMCodexLocalMatchScreenWidget>(GetTransientPackage());
	AP1Screen->TakeWidget();
	AP1Screen->SetMatchController(AP1World.Controller);
	AP1Screen->RefreshFromPresentation(AP1Presentation);
	const UTextBlock* AP1Title = Cast<UTextBlock>(AP1Screen->GetWidgetFromName(
		TEXT("SetPieceProductionTitle")));
	const UTextBlock* AP1Body = Cast<UTextBlock>(AP1Screen->GetWidgetFromName(
		TEXT("SetPieceProductionDetail")));
	const UButton* AP1Primary = Cast<UButton>(AP1Screen->GetWidgetFromName(
		TEXT("SetPieceProductionPrimaryAction")));
	const UTextBlock* AP1PrimaryLabel = AP1Primary != nullptr
		? Cast<UTextBlock>(AP1Primary->GetChildAt(0)) : nullptr;
	TestTrue(TEXT("AP1 terminal uses red-card identity copy and explicit advance"),
		AP1Title != nullptr && AP1Title->GetText().ToString() == TEXT("红牌")
			&& AP1Body != nullptr
			&& (AP1Body->GetText().ToString().StartsWith(TEXT("玩家A · "))
				|| AP1Body->GetText().ToString().StartsWith(TEXT("玩家B · ")))
			&& AP1Body->GetText().ToString().EndsWith(TEXT(" 被罚下"))
			&& !AP1Body->GetText().ToString().Contains(TEXT("Prototype."))
			&& AP1Primary != nullptr
			&& AP1Primary->GetVisibility() == ESlateVisibility::Visible
			&& AP1PrimaryLabel != nullptr
			&& AP1PrimaryLabel->GetText().ToString() == TEXT("下一回合"));
	TestTrue(TEXT("AP1 route never fabricates ordinary Hand Tactical Pips"),
		HasNoTacticalPips(AP1Presentation.LocalRack)
			&& HasNoTacticalPips(AP1Presentation.OpponentRack));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexCarrierSetPieceProductionMatrixTest,
	"FMCodex.LocalPlay.FullD12SetPieceProduction.02.CarrierMethodMatrix",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexCarrierSetPieceProductionMatrixTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexFullD12SetPieceProductionTests;
	(void)Parameters;
	bool bAllPassed = true;
	for (const ECarrierScenario Scenario : {
		ECarrierScenario::ShortDirect,
		ECarrierScenario::ShortAngled,
		ECarrierScenario::LongDirect,
		ECarrierScenario::LongPower,
		ECarrierScenario::PenaltyDirect,
		ECarrierScenario::PenaltyPanenka })
	{
		bAllPassed &= RunCarrierScenario(*this, Scenario);
	}
	return bAllPassed;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexCornerDraftAndResolutionProductionTest,
	"FMCodex.LocalPlay.FullD12SetPieceProduction.03.CornerDraftSealAndResolution",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexCornerDraftAndResolutionProductionTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexFullD12SetPieceProductionTests;
	(void)Parameters;
	FScopedWorld World;
	if (!StartSetPiece(*this, World, 1, ESetPieceSelectedType::Corner))
	{
		return false;
	}
	auto* Controller = World.Controller;
	const TArray<FName> AttackerLegal =
		Controller->GetInteractionView().LegalSetPieceCardIds;
	TestTrue(TEXT("Corner attacker projects at least four legal hand nominees"),
		AttackerLegal.Num() >= 4);
	if (AttackerLegal.Num() < 4)
	{
		return false;
	}
	const TArray<uint8> BeforeAttackerDraft = SerializeState(
		World.Host->GetMatchSnapshot().Snapshot);
	Controller->ToggleSetPieceDraftCard(AttackerLegal[0]);
	Controller->ToggleSetPieceDraftCard(AttackerLegal[1]);
	Controller->ToggleSetPieceDraftCard(AttackerLegal[2]);
	TestTrue(TEXT("Corner attacker click order remains local before lock"),
		Controller->GetInteractionView().DraftCornerNomineeCardIds
			== TArray<FName>({ AttackerLegal[0], AttackerLegal[1],
				AttackerLegal[2] })
			&& BeforeAttackerDraft == SerializeState(
				World.Host->GetMatchSnapshot().Snapshot));
	Controller->ToggleSetPieceDraftCard(AttackerLegal[1]);
	TestTrue(TEXT("Removing a Corner draft nominee renumbers later choices"),
		Controller->GetInteractionView().DraftCornerNomineeCardIds
			== TArray<FName>({ AttackerLegal[0], AttackerLegal[2] }));
	Controller->ToggleSetPieceDraftCard(AttackerLegal[1]);
	Controller->ToggleSetPieceDraftCard(AttackerLegal[3]);
	TestTrue(TEXT("Corner draft caps at three and keeps the reordered sequence"),
		Controller->GetInteractionView().DraftCornerNomineeCardIds
			== TArray<FName>({ AttackerLegal[0], AttackerLegal[2],
				AttackerLegal[1] })
			&& BeforeAttackerDraft == SerializeState(
				World.Host->GetMatchSnapshot().Snapshot));
	Controller->ConfirmSetPieceDraft();
	const auto& DefenderDraft = Controller->GetInteractionView();
	TestTrue(TEXT("Defender draft receives only sealed attacker acknowledgement"),
		Controller->GetLastDiagnostic().bHostSuccess
			&& DefenderDraft.InteractionCategory == ECategory::DraftCornerDefender
			&& DefenderDraft.bCornerAttackerNominationsLocked
			&& DefenderDraft.bHideCornerAttackerNomineeDetails
			&& DefenderDraft.CornerAttackerNominees.IsEmpty()
			&& DefenderDraft.DraftCornerNomineeCardIds.IsEmpty());
	const TArray<FName> DefenderLegal = DefenderDraft.LegalSetPieceCardIds;
	TestTrue(TEXT("Corner defender projects at least one legal hand nominee"),
		DefenderLegal.Num() >= 1);
	if (DefenderLegal.IsEmpty())
	{
		return false;
	}
	Controller->ToggleSetPieceDraftCard(DefenderLegal[0]);
	const auto BeforeDefenderLock = SerializeState(World.Host->GetMatchSnapshot().Snapshot);
	Controller->ConfirmSetPieceDraft();
	TestTrue(TEXT("Underfilled first lock is presentation-only confirmation"),
		Controller->GetInteractionView().bCornerLockConfirmationPending
			&& BeforeDefenderLock == SerializeState(World.Host->GetMatchSnapshot().Snapshot));
	Controller->ConfirmSetPieceDraft();
	TestTrue(TEXT("Both locked lists reveal only after defender confirmation"),
		Controller->GetLastDiagnostic().bHostSuccess
			&& Controller->GetInteractionView().InteractionCategory
				== ECategory::RollCornerParticipantSelection
			&& Controller->GetInteractionView().CornerAttackerNominees.Num() == 3
			&& Controller->GetInteractionView().CornerDefenderNominees.Num() == 1);
	TestTrue(TEXT("Each board side receives its own canonical D6 ranges"),
		Controller->GetInteractionView().CornerAttackerNomineeRollLabels
			== TArray<FString>({ TEXT("1–2"), TEXT("3–4"), TEXT("5–6") })
			&& Controller->GetInteractionView().CornerDefenderNomineeRollLabels
				== TArray<FString>({ TEXT("1–6") }));

	UFMCodexLocalMatchScreenWidget* Screen =
		NewObject<UFMCodexLocalMatchScreenWidget>(GetTransientPackage());
	Screen->TakeWidget();
	Screen->SetMatchController(Controller);
	Screen->RefreshFromPresentation(
		FFMCodexLocalMatchUMGPresentationBuilder::Build(
			Controller->GetInteractionView(),
			Controller->GetResolutionFeedback(), FString()));
	UBorder* CornerBoard = Cast<UBorder>(Screen->GetWidgetFromName(
		TEXT("CornerConfrontationBoard")));
	UTextBlock* SharedD6Value = Cast<UTextBlock>(Screen->GetWidgetFromName(
		TEXT("CornerSharedD6Value")));
	TestTrue(TEXT("Unequal nominee groups face one shared central D6 before roll"),
		CornerBoard != nullptr
			&& CornerBoard->GetVisibility() == ESlateVisibility::HitTestInvisible
			&& SharedD6Value != nullptr
			&& SharedD6Value->GetText().ToString() == TEXT("待掷")
			&& Screen->GetWidgetFromName(TEXT("CornerAttackerNomineeCard1"))
				->GetVisibility() == ESlateVisibility::HitTestInvisible
			&& Screen->GetWidgetFromName(TEXT("CornerAttackerNomineeCard2"))
				->GetVisibility() == ESlateVisibility::HitTestInvisible
			&& Screen->GetWidgetFromName(TEXT("CornerAttackerNomineeCard3"))
				->GetVisibility() == ESlateVisibility::HitTestInvisible
			&& Screen->GetWidgetFromName(TEXT("CornerDefenderNomineeCard1"))
				->GetVisibility() == ESlateVisibility::HitTestInvisible
			&& Screen->GetWidgetFromName(TEXT("CornerDefenderNomineeCard2"))
				->GetVisibility() == ESlateVisibility::Collapsed
			&& Screen->GetInteractionPanel()->GetVisibility()
				== ESlateVisibility::Collapsed);

	TestTrue(TEXT("Corner shared participant override accepted"),
		SetOverride(*Controller, ETarget::CornerParticipantSelection, 3));
	Controller->SubmitProjectedPrimaryPlayerIntent();
	TestTrue(TEXT("Shared D6 binds both participants and reaches intent"),
		Controller->GetLastDiagnostic().bHostSuccess
			&& Controller->GetInteractionView().bHasCornerSharedParticipantD6
			&& Controller->GetInteractionView().CornerSharedParticipantD6 == 3
			&& Controller->GetInteractionView().CornerRunner.bIsBound
			&& Controller->GetInteractionView().CornerHelper.bIsBound
			&& Controller->GetInteractionView().InteractionCategory
				== ECategory::SelectCornerIntent);
	Screen->RefreshFromPresentation(
		FFMCodexLocalMatchUMGPresentationBuilder::Build(
			Controller->GetInteractionView(),
			Controller->GetResolutionFeedback(), FString()));
	Screen->PauseInlineFormulaRevealTimerForTesting();
	auto* ParticipantReel = Cast<UFMCodexRollReelWidget>(Screen->GetWidgetFromName(TEXT("CornerParticipantRollReel")));
	TestTrue(TEXT("Shared selection cycles on the visible board without revealing a selected row"),
		ParticipantReel && ParticipantReel->GetVisibility() != ESlateVisibility::Collapsed
			&& CornerBoard->GetVisibility() == ESlateVisibility::HitTestInvisible
			&& Cast<UBorder>(Screen->GetWidgetFromName(TEXT("CornerAttackerNomineeCard1")))->GetRenderOpacity() == 1.0f
			&& Screen->GetWidgetFromName(TEXT("CornerHighIntent"))->GetVisibility() == ESlateVisibility::Collapsed);
	const float BeforeReelOffset = ParticipantReel ? ParticipantReel->GetCenterVerticalOffset() : 0.0f;
	Screen->AdvanceInlineFormulaRevealForTesting(0.013f);
	TestTrue(TEXT("Shared board Reel refreshes within the same cycling phase"),
		ParticipantReel && !FMath::IsNearlyEqual(BeforeReelOffset, ParticipantReel->GetCenterVerticalOffset()));
	Screen->AdvanceInlineFormulaRevealForTesting(1.457f);
	TestTrue(TEXT("Early ResultHold has the accepted tile but no selected-row disclosure yet"),
		ParticipantReel && ParticipantReel->IsStaticResultTileVisible()
			&& SharedD6Value->GetText().ToString() == TEXT("待掷")
			&& Cast<UBorder>(Screen->GetWidgetFromName(TEXT("CornerAttackerNomineeCard1")))->GetRenderOpacity() == 1.0f);
	Screen->AdvanceInlineFormulaRevealForTesting(0.33f);
	SharedD6Value = Cast<UTextBlock>(Screen->GetWidgetFromName(
		TEXT("CornerSharedD6Value")));
	const auto& ParticipantView = Controller->GetInteractionView();
	const int32 RunnerIndex = ParticipantView.CornerAttackerNominees.IndexOfByPredicate(
		[&ParticipantView](const FMatchPlaySetPieceParticipantBinding& Binding)
		{
			return Binding.CardId == ParticipantView.CornerRunner.CardId
				&& Binding.OwnerSide == ParticipantView.CornerRunner.OwnerSide;
		});
	const int32 HelperIndex = ParticipantView.CornerDefenderNominees.IndexOfByPredicate(
		[&ParticipantView](const FMatchPlaySetPieceParticipantBinding& Binding)
		{
			return Binding.CardId == ParticipantView.CornerHelper.CardId
				&& Binding.OwnerSide == ParticipantView.CornerHelper.OwnerSide;
		});
	UBorder* RunnerCard = RunnerIndex != INDEX_NONE
		? Cast<UBorder>(Screen->GetWidgetFromName(FName(*FString::Printf(
			TEXT("CornerAttackerNomineeCard%d"), RunnerIndex + 1)))) : nullptr;
	UBorder* HelperCard = HelperIndex != INDEX_NONE
		? Cast<UBorder>(Screen->GetWidgetFromName(FName(*FString::Printf(
			TEXT("CornerDefenderNomineeCard%d"), HelperIndex + 1)))) : nullptr;
	UBorder* NonRunnerCard = Cast<UBorder>(Screen->GetWidgetFromName(
		RunnerIndex == 0 ? TEXT("CornerAttackerNomineeCard2")
			: TEXT("CornerAttackerNomineeCard1")));
	UTextBlock* BonusText = Cast<UTextBlock>(Screen->GetWidgetFromName(
		TEXT("CornerCandidateBonusText")));
	TestTrue(TEXT("Shared D6 result highlights Runner and Helper and dims others"),
		SharedD6Value != nullptr
			&& SharedD6Value->GetText().ToString() == TEXT("3")
			&& Screen->IsInlineFormulaRevealInputBlocked()
			&& ParticipantReel && ParticipantReel->IsStaticResultTileVisible()
			&& RunnerCard != nullptr && RunnerCard->GetRenderOpacity() == 1.0f
			&& HelperCard != nullptr && HelperCard->GetRenderOpacity() == 1.0f
			&& NonRunnerCard != nullptr
			&& NonRunnerCard->GetRenderOpacity() < 0.5f
			&& BonusText != nullptr
			&& BonusText->GetVisibility() == ESlateVisibility::HitTestInvisible
			&& BonusText->GetText().ToString().Contains(TEXT("+")));
	Screen->AdvanceInlineFormulaRevealForTesting(2.0f);
	const auto* Compact = Cast<UTextBlock>(Screen->GetWidgetFromName(TEXT("SetPieceProductionDetail")));
	TestTrue(TEXT("Stable High/Low removes candidates and raw shared D6 after the result hold"),
		CornerBoard->GetVisibility() == ESlateVisibility::Collapsed
			&& Compact && Compact->GetText().ToString().Contains(TEXT("进攻人选"))
			&& !Compact->GetText().ToString().Contains(TEXT("D6"))
			&& !Compact->GetText().ToString().Contains(TEXT("候选")));
	const auto* Fit = Cast<UScaleBox>(Screen->GetWidgetFromName(TEXT("CornerAttackerNomineeCard1NameFit")));
	TestTrue(TEXT("Nominee name fitting only shrinks, keeping normal names at their intended scale"),
		Fit && Fit->GetStretch() == EStretch::ScaleToFit && Fit->GetStretchDirection() == EStretchDirection::DownOnly);
	Controller->SubmitCornerIntent(EMatchPlayCornerRouteIntent::High);
	Screen->RefreshFromPresentation(FFMCodexLocalMatchUMGPresentationBuilder::Build(
		Controller->GetInteractionView(), Controller->GetResolutionFeedback(), FString()));
	const FString CornerRouteHelper = Screen->GetInlineFormulaSurface()->GetPresentation().RollHelperLabel;
	TestEqual(TEXT("Corner pending route helper retains canonical ranges"), CornerRouteHelper,
		FString(TEXT("1–4：高球｜5–6：低平球")));
	TestTrue(TEXT("Corner route override accepted"),
		SetOverride(*Controller, ETarget::CornerRoute, 5));
	Controller->SubmitProjectedPrimaryPlayerIntent();
	TestTrue(TEXT("Corner route roll persists intended and actual route"),
		Controller->GetLastDiagnostic().bHostSuccess
			&& Controller->GetInteractionView().bHasCornerRouteD6
			&& Controller->GetInteractionView().CornerRouteD6 == 5
			&& Controller->GetInteractionView().CornerIntendedRoute
				== EMatchPlayCornerRouteIntent::High
			&& Controller->GetInteractionView().CornerActualRoute
				!= EMatchPlayCornerRouteIntent::None
			&& Controller->GetInteractionView().InteractionCategory
				== ECategory::RollCornerAttack);
	Screen->RefreshFromPresentation(FFMCodexLocalMatchUMGPresentationBuilder::Build(
		Controller->GetInteractionView(), Controller->GetResolutionFeedback(), FString()));
	Screen->PauseInlineFormulaRevealTimerForTesting();
	TestTrue(TEXT("Corner route cycles on the shared inline surface without premature Formula rows"),
		Screen->GetInlineFormulaSurface()->GetPresentation().bVisible
			&& !Screen->GetInlineFormulaSurface()->GetPresentation().bShowFormulaRows
			&& Screen->IsInlineFormulaRevealInputBlocked());
	auto CheckCornerRouteHelper = [&]()
	{
		const auto* Helper = Cast<UTextBlock>(Screen->GetInlineFormulaSurface()->GetWidgetFromName(TEXT("InlineFormulaRollHelper")));
		TestTrue(TEXT("Active Corner route reel preserves the pending range helper in DTO and widget"),
			Screen->GetInlineFormulaSurface()->GetPresentation().RollHelperLabel == CornerRouteHelper
				&& Helper && Helper->GetText().ToString() == CornerRouteHelper
				&& Helper->GetVisibility() != ESlateVisibility::Collapsed);
	};
	CheckCornerRouteHelper();
	Screen->AdvanceInlineFormulaRevealForTesting(1.35f);
	TestEqual(TEXT("Corner helper is sampled during actual Settling"), Screen->GetInlineFormulaRevealPhase(),
		EFMCodexUMGInlineFormulaRevealPhase::Settling);
	Screen->RefreshFromPresentation(FFMCodexLocalMatchUMGPresentationBuilder::Build(
		Controller->GetInteractionView(), Controller->GetResolutionFeedback(), FString()));
	CheckCornerRouteHelper();
	Screen->AdvanceInlineFormulaRevealForTesting(0.12f);
	CheckCornerRouteHelper(); // Corner's existing route disclosure is delayed within ResultHold.
	Screen->AdvanceInlineFormulaRevealForTesting(0.23f);
	TestTrue(TEXT("Corner route result hold discloses route and D6, not Formula early"),
		Screen->GetInlineFormulaSurface()->GetPresentation().RouteResultLabel.Contains(TEXT("5"))
			&& !Screen->GetInlineFormulaSurface()->GetPresentation().bShowFormulaRows
			&& Screen->GetInlineFormulaSurface()->GetPresentation().RollHelperLabel.IsEmpty());
	Screen->AdvanceInlineFormulaRevealForTesting(2.0f);
	TestTrue(TEXT("Corner Formula cannot retain a stale route helper"),
		Screen->GetInlineFormulaSurface()->GetPresentation().RollHelperLabel.IsEmpty());
	const float KnownAttack = Controller->GetInteractionView().SetPieceAttackKnownSubtotal;
	TestTrue(TEXT("Route completion projects numeric known and current totals before either duel roll"),
		Controller->GetInteractionView().bHasSetPieceAttackKnownSubtotal
			&& Controller->GetInteractionView().bHasSetPieceDefenseKnownSubtotal
			&& Controller->GetInteractionView().bHasSetPieceAttackCurrentTotal
			&& Controller->GetInteractionView().bHasSetPieceDefenseCurrentTotal);
	TestTrue(TEXT("Corner attack/defense overrides accepted"),
		SetOverride(*Controller, ETarget::CornerAttack, 6)
			&& SetOverride(*Controller, ETarget::CornerDefense, 1));
	Controller->SubmitProjectedPrimaryPlayerIntent();
	TestTrue(TEXT("Corner attack roll hands off to defending side"),
		Controller->GetLastDiagnostic().bHostSuccess
			&& Controller->GetInteractionView().bHasSetPieceAttackD6
			&& Controller->GetInteractionView().SetPieceAttackD6 == 6
			&& Controller->GetInteractionView().InteractionCategory
				== ECategory::RollCornerDefense);
	TestEqual(TEXT("Attack-only prefix publishes authoritative current total"),
		Controller->GetInteractionView().SetPieceAttackCurrentTotal, KnownAttack + 6.0f);
	Screen->RefreshFromPresentation(FFMCodexLocalMatchUMGPresentationBuilder::Build(
		Controller->GetInteractionView(), Controller->GetResolutionFeedback(), FString()));
	Screen->PauseInlineFormulaRevealTimerForTesting();
	Screen->AdvanceInlineFormulaRevealForTesting(5.0f);
	const auto BeforeDecisiveHeader = Screen->GetMatchHeader()->GetPresentation();
	Controller->SubmitProjectedPrimaryPlayerIntent();
	TestTrue(TEXT("Corner defense roll reaches Formula and explicit terminal"),
		Controller->GetLastDiagnostic().bHostSuccess
			&& Controller->GetInteractionView().bHasSetPieceDefenseD6
			&& Controller->GetInteractionView().SetPieceDefenseD6 == 1
			&& Controller->GetInteractionView().bHasSetPieceFormula
			&& Controller->GetInteractionView().bHasSetPieceOutcome
			&& Controller->GetInteractionView().bTerminalPendingAdvance
			&& Controller->GetInteractionView().InteractionCategory
				== ECategory::AdvanceAfterTerminal);
	const auto CornerTerminalPresentation =
		FFMCodexLocalMatchUMGPresentationBuilder::Build(
			Controller->GetInteractionView(),
			Controller->GetResolutionFeedback(), FString());
	TestTrue(TEXT("Corner returns to the ordinary Formula/terminal surface"),
		CornerTerminalPresentation.InlineFormula.bVisible
			&& CornerTerminalPresentation.InlineFormula.bShowAttackRow
			&& CornerTerminalPresentation.InlineFormula.bShowDefenseRow
			&& CornerTerminalPresentation.InlineFormula.RouteResultLabel.Contains(
				TEXT(" vs 防守"))
			&& CornerTerminalPresentation.InlineFormula.PrimaryAction.Action.Label
				== TEXT("下一回合"));
	TestTrue(TEXT("Corner score fixture is an authoritative Goal"), Controller->GetInteractionView().bSetPieceGoal);
	Screen->RefreshFromPresentation(CornerTerminalPresentation);
	Screen->PauseInlineFormulaRevealTimerForTesting();
	CheckDecisiveScoreReveal(*this, *Screen, BeforeDecisiveHeader, CornerTerminalPresentation);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexFullD12RevealAndCentralOwnershipTest,
	"FMCodex.LocalPlay.FullD12SetPieceProduction.04.RevealAndCentralOwnership",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexFullD12RevealAndCentralOwnershipTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexFullD12SetPieceProductionTests;
	(void)Parameters;
	FScopedWorld World;
	if (World.Controller == nullptr)
	{
		return false;
	}
	World.Controller->StartNewDemoMatch();
	const FFMCodexLocalMatchResolutionFeedback EmptyFeedback;
	const auto Pending = FFMCodexLocalMatchUMGPresentationBuilder::Build(
		World.Controller->GetInteractionView(), EmptyFeedback, FString());
	UFMCodexLocalMatchScreenWidget* Screen =
		NewObject<UFMCodexLocalMatchScreenWidget>(GetTransientPackage());
	Screen->TakeWidget();
	Screen->SetMatchController(World.Controller);
	Screen->RefreshFromPresentation(Pending);
	Screen->BeginPendingTacticalPointRevealForTesting();
	Screen->PauseInlineFormulaRevealTimerForTesting();
	TestTrue(TEXT("Set Piece reveal fixture accepts Full D12 9"),
		SetOverride(*World.Controller, ETarget::FullD12, 9));
	World.Controller->RollDemoTacticalPoints();
	const auto Resolved = FFMCodexLocalMatchUMGPresentationBuilder::Build(
		World.Controller->GetInteractionView(),
		World.Controller->GetResolutionFeedback(), FString());
	Screen->RefreshFromPresentation(Resolved);
	Screen->PauseInlineFormulaRevealTimerForTesting();
	const UWidget* SetPieceSurface = Screen->GetWidgetFromName(
		TEXT("SetPieceProductionResolutionSurface"));
	UFMCodexRollReelWidget* Reel = Screen->GetTacticalPointRollReel();
	TestTrue(TEXT("D12 cycles in 1-12 domain without route surface leakage"),
		Screen->IsInlineFormulaRevealInputBlocked()
			&& Screen->GetInlineFormulaRevealPhase()
				== EFMCodexUMGInlineFormulaRevealPhase::Cycling
			&& Reel != nullptr
			&& Reel->GetPresentation().DomainMinimum == 1
			&& Reel->GetPresentation().DomainMaximum == 12
			&& SetPieceSurface != nullptr
			&& SetPieceSurface->GetVisibility() == ESlateVisibility::Collapsed
			&& Screen->GetInlineFormulaSurface()->GetVisibility()
				== ESlateVisibility::Collapsed);

	Screen->AdvanceInlineFormulaRevealForTesting(1.30f);
	Screen->AdvanceInlineFormulaRevealForTesting(0.16f);
	const UTextBlock* ResultText = Cast<UTextBlock>(Screen->GetWidgetFromName(
		TEXT("TacticalPointRollRevealResult")));
	TestTrue(TEXT("Raw D12 appears before semantic Set Piece disclosure"),
		Screen->GetInlineFormulaRevealPhase()
			== EFMCodexUMGInlineFormulaRevealPhase::ResultHold
			&& ResultText != nullptr
			&& ResultText->GetText().ToString().Contains(TEXT("9"))
			&& !ResultText->GetText().ToString().Contains(TEXT("定位球"))
			&& SetPieceSurface->GetVisibility() == ESlateVisibility::Collapsed
			&& Screen->GetInlineFormulaSurface()->GetVisibility()
				== ESlateVisibility::Collapsed);
	Screen->AdvanceInlineFormulaRevealForTesting(0.21f);
	ResultText = Cast<UTextBlock>(Screen->GetWidgetFromName(
		TEXT("TacticalPointRollRevealResult")));
	SetPieceSurface = Screen->GetWidgetFromName(
		TEXT("SetPieceProductionResolutionSurface"));
	TestTrue(TEXT("ResultHold discloses the authoritative route, not a local map"),
		ResultText != nullptr
			&& ResultText->GetText().ToString().Contains(TEXT("定位球"))
			&& SetPieceSurface != nullptr
			&& SetPieceSurface->GetVisibility() == ESlateVisibility::Collapsed
			&& Screen->GetInlineFormulaSurface()->GetVisibility()
				== ESlateVisibility::Collapsed);
	Screen->AdvanceInlineFormulaRevealForTesting(2.35f);
	Screen->AdvanceInlineFormulaRevealForTesting(0.03f);
	SetPieceSurface = Screen->GetWidgetFromName(
		TEXT("SetPieceProductionResolutionSurface"));
	auto* FormulaSurface = Screen->GetInlineFormulaSurface();
	TestTrue(TEXT("After entry reveal the ordinary central surface owns the type CTA"),
		!Screen->IsInlineFormulaRevealInputBlocked()
			&& FormulaSurface != nullptr
			&& FormulaSurface->GetVisibility()
				== ESlateVisibility::SelfHitTestInvisible
			&& FormulaSurface->GetPresentation().ContestLabel
				== TEXT("定位球类型")
			&& FormulaSurface->GetPresentation().TacticalPlayerSummaryLabel
				== TEXT("1–2：角球  3–4：远距离任意球  5：近距离任意球  6：点球")
			&& FormulaSurface->GetPresentation().PrimaryAction.Action.Label
				== TEXT("掷定位球类型")
			&& SetPieceSurface->GetVisibility() == ESlateVisibility::Collapsed
			&& Screen->GetInteractionPanel()->GetVisibility()
				== ESlateVisibility::Collapsed);

	TestTrue(TEXT("Type reveal fixture accepts Short Free Kick D6 5"),
		SetOverride(*World.Controller, ETarget::SetPieceType, 5));
	Screen->ResetPrimaryActionDispatchForTesting();
	Screen->RequestContinueResolution();
	Screen->PauseInlineFormulaRevealTimerForTesting();
	Screen->RefreshFromPresentation(
		FFMCodexLocalMatchUMGPresentationBuilder::Build(
			World.Controller->GetInteractionView(),
			World.Controller->GetResolutionFeedback(), FString()));
	Screen->PauseInlineFormulaRevealTimerForTesting();
	FormulaSurface = Screen->GetInlineFormulaSurface();
	TestTrue(TEXT("Set Piece Type uses the ordinary moving D6 Reel"),
		Screen->GetPrimaryActionDispatchCountForTesting() == 1
			&& Screen->GetLastPrimaryActionDispatchForTesting()
				== EFMCodexUMGInteractionCategory::RollSetPieceType
			&& Screen->GetInlineFormulaRevealPhase()
				== EFMCodexUMGInlineFormulaRevealPhase::Cycling
			&& FormulaSurface != nullptr
			&& FormulaSurface->GetPresentation().bDiceRevealVisible
			&& FormulaSurface->GetPresentation().RollReel.bMoving
			&& FormulaSurface->GetPresentation().RollReel.DomainMinimum == 1
			&& FormulaSurface->GetPresentation().RollReel.DomainMaximum == 6
			&& Screen->GetWidgetFromName(
				TEXT("TacticalPointRollRevealSurface"))->GetVisibility()
					== ESlateVisibility::Collapsed
			&& SetPieceSurface->GetVisibility() == ESlateVisibility::Collapsed);
	Screen->RequestContinueResolution();
	TestEqual(TEXT("Reveal gate prevents duplicate type command dispatch"),
		Screen->GetPrimaryActionDispatchCountForTesting(), 1);
	Screen->AdvanceInlineFormulaRevealForTesting(1.46f);
	Screen->AdvanceInlineFormulaRevealForTesting(0.21f);
	FormulaSurface = Screen->GetInlineFormulaSurface();
	TestTrue(TEXT("Type ResultHold discloses the authoritative D6 and route"),
		Screen->GetInlineFormulaRevealPhase()
			== EFMCodexUMGInlineFormulaRevealPhase::ResultHold
			&& FormulaSurface->GetPresentation().ContestLabel
				== TEXT("定位球类型")
			&& FormulaSurface->GetPresentation().RouteResultLabel
				== TEXT("掷点 5 → 近距离任意球")
			&& FormulaSurface->GetPresentation().RollReel.bStaticResult
			&& FormulaSurface->GetPresentation().RollReel.CenterValue == 5);
	Screen->AdvanceInlineFormulaRevealForTesting(2.40f);
	SetPieceSurface = Screen->GetWidgetFromName(
		TEXT("SetPieceProductionResolutionSurface"));
	TestTrue(TEXT("Type reveal settles into the unique carrier-selection surface"),
		!Screen->IsInlineFormulaRevealInputBlocked()
			&& SetPieceSurface->GetVisibility()
				== ESlateVisibility::SelfHitTestInvisible
			&& World.Controller->GetInteractionView().InteractionCategory
				== ECategory::SelectSetPieceCarrier);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexSetPieceCarrierCopyAndLocalizationRepairTest,
	"FMCodex.LocalPlay.FullD12SetPieceProduction.05.CarrierCopyAndLocalizationRepair",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexSetPieceCarrierCopyAndLocalizationRepairTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexFullD12SetPieceProductionTests;
	(void)Parameters;
	struct FCase
	{
		int32 TypeD6;
		ESetPieceSelectedType Type;
		const TCHAR* FirstHint;
		const TCHAR* SecondHint;
		const TCHAR* FirstHelperName;
		const TCHAR* SecondHelperName;
		const TCHAR* FirstHelper;
		const TCHAR* SecondHelper;
	};
	for (const FCase& Case : {
		FCase{ 5, ESetPieceSelectedType::ShortFreeKick,
			TEXT("直接射门：使用较高的射门/传球"),
			TEXT("战术配合：需射门+传球≥8"),
			TEXT("ShortDirectMethodHelper"),
			TEXT("ShortAngledMethodHelper"),
			TEXT("看较高射门/传球"), TEXT("需射门+传球≥8") },
		FCase{ 3, ESetPieceSelectedType::LongFreeKick,
			TEXT("直接射门：使用远射，对抗门将站位"),
			TEXT("重炮轰门：只看两次掷点"),
			TEXT("LongDirectMethodHelper"),
			TEXT("LongPowerMethodHelper"),
			TEXT("看远射 / 门将站位"), TEXT("只看两枚掷点") },
		FCase{ 6, ESetPieceSelectedType::Penalty,
			TEXT("常规点球：使用较高的射门/传球"),
			TEXT("勺子点球：只看一次掷点"),
			TEXT("PenaltyDirectMethodHelper"),
			TEXT("PenaltyPanenkaMethodHelper"),
			TEXT("看较高射门/传球"), TEXT("只看一枚掷点") } })
	{
		FScopedWorld World;
		if (!StartSetPiece(*this, World, Case.TypeD6, Case.Type))
		{
			return false;
		}
		UFMCodexLocalMatchScreenWidget* Screen =
			NewObject<UFMCodexLocalMatchScreenWidget>(GetTransientPackage());
		Screen->TakeWidget();
		Screen->SetMatchController(World.Controller);
		auto RefreshScreen = [&]()
		{
			Screen->RefreshFromPresentation(
				FFMCodexLocalMatchUMGPresentationBuilder::Build(
					World.Controller->GetInteractionView(),
					World.Controller->GetResolutionFeedback(), FString()));
		};
		RefreshScreen();
		UTextBlock* Detail = Cast<UTextBlock>(Screen->GetWidgetFromName(
			TEXT("SetPieceProductionDetail")));
		UTextBlock* Status = Cast<UTextBlock>(Screen->GetWidgetFromName(
			TEXT("SetPieceProductionStatus")));
		const FString InitialCopy = Detail == nullptr
			? FString() : Detail->GetText().ToString();
		TestTrue(TEXT("Carrier panel explains select-then-confirm in natural Chinese"),
			InitialCopy.Contains(TEXT("请选择主罚球员"))
				&& InitialCopy.Contains(TEXT("单击本方手牌只会选中球员"))
				&& InitialCopy.Contains(TEXT("确认主罚球员"))
				&& !InitialCopy.Contains(TEXT("本地草稿")));
		TestTrue(TEXT("Carrier panel shows the route-specific concise hints"),
			InitialCopy.Contains(Case.FirstHint)
				&& InitialCopy.Contains(Case.SecondHint));
		TestTrue(TEXT("Carrier panel removes upstream D12 clutter and names the player side"),
			Status != nullptr
				&& Status->GetText().ToString().Contains(TEXT("当前操作：玩家"))
				&& !Status->GetText().ToString().Contains(TEXT("D12"))
				&& !Status->GetText().ToString().Contains(TEXT("进攻方")));

		const FName Carrier = ChooseCarrier(*World.Controller, false);
		const TArray<uint8> BeforeClick = SerializeState(
			World.Host->GetMatchSnapshot().Snapshot);
		World.Controller->ToggleSetPieceDraftCard(Carrier);
		TestTrue(TEXT("First carrier click remains presentation-only selection"),
			!Carrier.IsNone()
				&& World.Controller->GetInteractionView()
					.DraftSetPieceCarrierCardId == Carrier
				&& BeforeClick == SerializeState(
					World.Host->GetMatchSnapshot().Snapshot));
		RefreshScreen();
		Detail = Cast<UTextBlock>(Screen->GetWidgetFromName(
			TEXT("SetPieceProductionDetail")));
		UButton* Primary = Cast<UButton>(Screen->GetWidgetFromName(
			TEXT("SetPieceProductionPrimaryAction")));
		UTextBlock* PrimaryLabel = Primary == nullptr
			? nullptr : Cast<UTextBlock>(Primary->GetChildAt(0));
		const FString SelectedCopy = Detail == nullptr
			? FString() : Detail->GetText().ToString();
		Status = Cast<UTextBlock>(Screen->GetWidgetFromName(
			TEXT("SetPieceProductionStatus")));
		TestTrue(TEXT("Selected carrier copy and CTA state the lock boundary without duplication"),
			!SelectedCopy.Contains(TEXT("已选中主罚球员"))
				&& SelectedCopy.Contains(TEXT("点击“确认主罚球员”后继续"))
				&& SelectedCopy.Contains(Case.FirstHint)
				&& SelectedCopy.Contains(Case.SecondHint)
				&& Primary != nullptr
				&& Primary->GetVisibility() == ESlateVisibility::Visible
				&& PrimaryLabel != nullptr
				&& PrimaryLabel->GetText().ToString() == TEXT("确认主罚球员"));
		TestTrue(TEXT("Draft carrier identity is promoted above the instructions"),
			Status != nullptr
				&& Status->GetText().ToString().Contains(
					TEXT("主罚球员（待确认）："))
				&& !Status->GetText().ToString().Contains(TEXT("D12")));
		TestFalse(TEXT("Active Set Piece copy exposes no English continue fallback"),
			InitialCopy.Contains(TEXT("Continue Resolution"))
				|| SelectedCopy.Contains(TEXT("Continue Resolution"))
				|| (PrimaryLabel != nullptr
					&& PrimaryLabel->GetText().ToString().Contains(TEXT("Continue"))));

		World.Controller->ConfirmSetPieceDraft();
		RefreshScreen();
		Detail = Cast<UTextBlock>(Screen->GetWidgetFromName(
			TEXT("SetPieceProductionDetail")));
		const FString MethodCopy = Detail == nullptr
			? FString() : Detail->GetText().ToString();
		Status = Cast<UTextBlock>(Screen->GetWidgetFromName(
			TEXT("SetPieceProductionStatus")));
		UHorizontalBox* MethodRow = Cast<UHorizontalBox>(Screen->GetWidgetFromName(
			TEXT("SetPieceMethodChoiceRow")));
		const UVerticalBoxSlot* MethodSlot = MethodRow == nullptr
			? nullptr : Cast<UVerticalBoxSlot>(MethodRow->Slot);
		TestTrue(TEXT("Method panel removes rules already carried by its buttons"),
			MethodCopy.Contains(TEXT("请选择结算方式"))
				&& !MethodCopy.Contains(Case.FirstHint)
				&& !MethodCopy.Contains(Case.SecondHint));
		TestTrue(TEXT("Method panel clearly names the confirmed taker"),
			Status != nullptr
				&& Status->GetText().ToString().Contains(TEXT("主罚球员："))
				&& !Status->GetText().ToString().Contains(TEXT("D12")));
		TestTrue(TEXT("Method choices reuse the centered ordinary branch rhythm"),
			MethodSlot != nullptr
				&& MethodSlot->GetHorizontalAlignment() == HAlign_Center);
		const UTextBlock* FirstHelper = Cast<UTextBlock>(
			Screen->GetWidgetFromName(FName(Case.FirstHelperName)));
		const UTextBlock* SecondHelper = Cast<UTextBlock>(
			Screen->GetWidgetFromName(FName(Case.SecondHelperName)));
		TestTrue(TEXT("Method buttons retain route-specific secondary help"),
			FirstHelper != nullptr
				&& FirstHelper->GetText().ToString() == Case.FirstHelper
				&& SecondHelper != nullptr
				&& SecondHelper->GetText().ToString() == Case.SecondHelper);
		if (Case.Type == ESetPieceSelectedType::LongFreeKick)
		{
			const auto* PowerLabel = Cast<UTextBlock>(Screen->GetWidgetFromName(TEXT("LongPowerMethodLabel")));
			const auto* DirectLabel = Cast<UTextBlock>(Screen->GetWidgetFromName(TEXT("LongDirectMethodLabel")));
			TestTrue(TEXT("Long Free Kick method buttons use Direct and centrally named Power"),
				PowerLabel && PowerLabel->GetText().ToString() == TEXT("重炮轰门")
					&& DirectLabel && DirectLabel->GetText().ToString() == TEXT("直接射门")
					&& !InitialCopy.Contains(TEXT("大力轰门")) && !SelectedCopy.Contains(TEXT("大力轰门")));
		}
		if (Case.Type == ESetPieceSelectedType::ShortFreeKick)
		{
			const UTextBlock* Title = Cast<UTextBlock>(
				Screen->GetWidgetFromName(TEXT("SetPieceProductionTitle")));
			TestTrue(TEXT("Near Free Kick player-facing route title uses the frozen name"),
				Title != nullptr
					&& Title->GetText().ToString() == TEXT("近距离任意球")
					&& !MethodCopy.Contains(TEXT("短任意球")));
		}
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexShortSetPieceOrdinarySurfaceRepairTest,
	"FMCodex.LocalPlay.FullD12SetPieceProduction.06.ShortOrdinarySurfaceRepair",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexShortSetPieceOrdinarySurfaceRepairTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexFullD12SetPieceProductionTests;
	(void)Parameters;
	FScopedWorld DirectWorld;
	if (!StartSetPiece(*this, DirectWorld, 5,
			ESetPieceSelectedType::ShortFreeKick)
		|| !BindCarrier(*this, *DirectWorld.Controller, false))
	{
		return false;
	}
	DirectWorld.Controller->SubmitShortFreeKickMethod(
		EMatchPlayShortFreeKickMethod::Direct);
	auto ProjectDirect = [&]()
	{
		return FFMCodexLocalMatchUMGPresentationBuilder::Build(
			DirectWorld.Controller->GetInteractionView(),
			DirectWorld.Controller->GetResolutionFeedback(), FString());
	};
	const auto AttackPending = ProjectDirect();
	TestTrue(TEXT("Short Direct attack uses the ordinary central formula surface"),
		AttackPending.InlineFormula.bVisible
			&& AttackPending.InlineFormula.bSuppressLegacyResolution
			&& AttackPending.InlineFormula.bShowFormulaRows
			&& AttackPending.InlineFormula.bShowAttackRow
			&& AttackPending.InlineFormula.bShowDefenseRow
			&& AttackPending.InlineFormula.bAttackRowActive
			&& AttackPending.InlineFormula.AttackRow.Terms.Num() == 2
			&& AttackPending.InlineFormula.DefenseRow.Terms.Num() == 3
			&& AttackPending.InlineFormula.AttackRow.Terms[0].DisplayLabel
				.Contains(TEXT("射门"))
			&& AttackPending.InlineFormula.AttackRow.Terms[0].DisplayLabel
				.Contains(TEXT("传球"))
			&& AttackPending.InlineFormula.AttackRow.Terms[0].DisplayLabel
				.Contains(TEXT("取较高"))
			&& AttackPending.InlineFormula.DefenseRow.Terms[0].DisplayLabel
				.Contains(TEXT("手控球"))
			&& AttackPending.InlineFormula.DefenseRow.Terms[1].DisplayLabel
				== TEXT("防守加成 1")
			&& AttackPending.InlineFormula.AttackRow
				.bKnownNonRollSubtotalResolved
			&& AttackPending.InlineFormula.DefenseRow
				.bKnownNonRollSubtotalResolved
			&& AttackPending.InlineFormula.AttackRow.bDisplayedResultResolved
			&& AttackPending.InlineFormula.DefenseRow.bDisplayedResultResolved
			&& FMath::IsNearlyEqual(
				AttackPending.InlineFormula.AttackRow.DisplayedResult,
				DirectWorld.Controller->GetInteractionView()
					.SetPieceAttackKnownSubtotal)
			&& FMath::IsNearlyEqual(
				AttackPending.InlineFormula.DefenseRow.DisplayedResult,
				DirectWorld.Controller->GetInteractionView()
					.SetPieceDefenseKnownSubtotal)
			&& !AttackPending.InlineFormula.AttackRow.KnownNonRollSubtotalLabel
				.Contains(TEXT("由权威结果计入"))
			&& AttackPending.InlineFormula.RouteResultLabel.IsEmpty()
			&& !AttackPending.InlineFormula.ContestLabel.Contains(TEXT("短任意球"))
			&& AttackPending.InlineFormula.PrimaryAction.Action.Label
				== TEXT("进攻方掷点"));

	TestTrue(TEXT("Short Direct overrides accepted"),
		SetOverride(*DirectWorld.Controller,
			ETarget::ShortFreeKickDirectAttack, 6)
			&& SetOverride(*DirectWorld.Controller,
				ETarget::ShortFreeKickDirectDefense, 1));
	UFMCodexLocalMatchScreenWidget* RevealScreen =
		NewObject<UFMCodexLocalMatchScreenWidget>(GetTransientPackage());
	RevealScreen->TakeWidget();
	RevealScreen->SetMatchController(DirectWorld.Controller);
	RevealScreen->RefreshFromPresentation(AttackPending);
	RevealScreen->RequestContinueResolution();
	RevealScreen->PauseInlineFormulaRevealTimerForTesting();
	RevealScreen->RefreshFromPresentation(ProjectDirect());
	auto* RevealFormula = Cast<UFMCodexInlineResolutionFormulaSurfaceWidget>(
		RevealScreen->GetWidgetFromName(TEXT("InlineResolutionFormulaSurface")));
	TestTrue(TEXT("Short method D6 reuses ordinary Reel without stacked reveal UI"),
		RevealScreen->IsInlineFormulaRevealInputBlocked()
			&& RevealFormula != nullptr
			&& RevealFormula->GetVisibility()
				== ESlateVisibility::SelfHitTestInvisible
			&& RevealFormula->GetPresentation().bDiceRevealVisible
			&& RevealFormula->GetPresentation().ContestLabel
				== TEXT("近距离任意球 · 直接射门")
			&& RevealFormula->GetPresentation().RouteResultLabel.IsEmpty()
			&& RevealScreen->GetWidgetFromName(
				TEXT("TacticalPointRollRevealSurface"))->GetVisibility()
				== ESlateVisibility::Collapsed
			&& RevealScreen->GetWidgetFromName(
				TEXT("SetPieceProductionResolutionSurface"))->GetVisibility()
				== ESlateVisibility::Collapsed);
	RevealScreen->AdvanceInlineFormulaRevealForTesting(5.0f);
	const auto DefensePending = ProjectDirect();
	TestTrue(TEXT("Accepted Short attack roll remains visible before defense"),
		DefensePending.InlineFormula.bVisible
			&& DefensePending.InlineFormula.StatusLabel.Contains(
				TEXT("进攻方掷点 6 已确认"))
			&& DefensePending.InlineFormula.StatusLabel.Contains(
				TEXT("等待防守方掷点"))
			&& DefensePending.InlineFormula.AttackRow.Terms[1].bResolved
			&& DefensePending.InlineFormula.AttackRow.Terms[1].RawD6 == 6
			&& DefensePending.InlineFormula.AttackRow.Terms[1].DisplayLabel
				== TEXT("掷点 6")
			&& DefensePending.InlineFormula.AttackRow.bDisplayedResultResolved
			&& FMath::IsNearlyEqual(
				DefensePending.InlineFormula.AttackRow.DisplayedResult,
				DefensePending.InlineFormula.AttackRow.KnownNonRollSubtotal + 6.0f)
			&& DefensePending.InlineFormula.DefenseRow.bDisplayedResultResolved
			&& FMath::IsNearlyEqual(
				DefensePending.InlineFormula.DefenseRow.DisplayedResult,
				DefensePending.InlineFormula.DefenseRow.KnownNonRollSubtotal)
			&& DefensePending.InlineFormula.RouteResultLabel.IsEmpty()
			&& DefensePending.InlineFormula.bDefenseRowActive
			&& DefensePending.InlineFormula.PrimaryAction.Action.Label
				== TEXT("防守方掷点"));
	DirectWorld.Controller->SubmitProjectedPrimaryPlayerIntent();
	const auto DirectTerminal = ProjectDirect();
	const auto& DirectTerminalView = DirectWorld.Controller->GetInteractionView();
	const FString DirectTakerName = CardDisplayName(
		DirectTerminalView, DirectTerminalView.SetPieceCarrier.OwnerSide,
		DirectTerminalView.SetPieceCarrier.CardId);
	TestTrue(TEXT("Short Direct terminal keeps formula, result and next CTA together"),
		DirectTerminal.InlineFormula.bVisible
			&& DirectTerminal.InlineFormula.bShowAttackRow
			&& DirectTerminal.InlineFormula.bShowDefenseRow
			&& DirectTerminal.InlineFormula.AttackRow.bFinalValueResolved
			&& DirectTerminal.InlineFormula.DefenseRow.bFinalValueResolved
			&& FMath::IsNearlyEqual(
				DirectTerminal.InlineFormula.AttackRow.DisplayedResult,
				DirectTerminal.InlineFormula.AttackRow.FinalValue)
			&& FMath::IsNearlyEqual(
				DirectTerminal.InlineFormula.DefenseRow.DisplayedResult,
				DirectTerminal.InlineFormula.DefenseRow.FinalValue)
			&& DirectTerminal.InlineFormula.RouteResultLabel.Contains(
				TEXT("进攻"))
			&& DirectTerminal.InlineFormula.bNarrativeAvailable
			&& DirectTerminalView.bSetPieceGoal
			&& !DirectTakerName.IsEmpty()
			&& DirectTerminal.InlineFormula.ContestLabel
				== DirectTerminal.InlineFormula.NarrativeHeadline
			&& DirectTerminal.InlineFormula.ContestLabel.Contains(DirectTakerName)
			&& DirectTerminal.InlineFormula.ContestLabel.Contains(
				TEXT("近距离任意球直接破门！"))
			&& DirectTerminal.InlineFormula.ResultTitle == TEXT("进球")
			&& DirectTerminal.InlineFormula.StatusLabel
				== TEXT("近距离任意球 · 直接射门 · 进球")
			&& !DirectTerminal.InlineFormula.ContestLabel.Contains(TEXT("短任意球"))
			&& DirectTerminal.InlineFormula.PrimaryAction.Action.Label
				== TEXT("下一回合"));

	UFMCodexLocalMatchScreenWidget* DirectScreen =
		NewObject<UFMCodexLocalMatchScreenWidget>(GetTransientPackage());
	DirectScreen->TakeWidget();
	DirectScreen->SetMatchController(DirectWorld.Controller);
	DirectScreen->RefreshFromPresentation(DirectTerminal);
	auto* FormulaSurface = Cast<UFMCodexInlineResolutionFormulaSurfaceWidget>(
		DirectScreen->GetWidgetFromName(TEXT("InlineResolutionFormulaSurface")));
	const UWidget* SetPieceSurface = DirectScreen->GetWidgetFromName(
		TEXT("SetPieceProductionResolutionSurface"));
	TestTrue(TEXT("Short Direct renders one ordinary shell without Set Piece stacking"),
		FormulaSurface != nullptr
			&& FormulaSurface->GetVisibility()
				== ESlateVisibility::SelfHitTestInvisible
			&& SetPieceSurface != nullptr
			&& SetPieceSurface->GetVisibility() == ESlateVisibility::Collapsed
			&& FormulaSurface->GetWidgetFromName(
				TEXT("InlineFormulaAttackRegion"))->GetVisibility()
				== ESlateVisibility::SelfHitTestInvisible
			&& FormulaSurface->GetWidgetFromName(
				TEXT("InlineFormulaDefenseRegion"))->GetVisibility()
				== ESlateVisibility::SelfHitTestInvisible);

	FScopedWorld MissWorld;
	if (!StartSetPiece(*this, MissWorld, 5,
			ESetPieceSelectedType::ShortFreeKick)
		|| !BindCarrier(*this, *MissWorld.Controller, false))
	{
		return false;
	}
	MissWorld.Controller->SubmitShortFreeKickMethod(
		EMatchPlayShortFreeKickMethod::Direct);
	TestTrue(TEXT("Short Direct miss overrides accepted"),
		SetOverride(*MissWorld.Controller,
			ETarget::ShortFreeKickDirectAttack, 1)
			&& SetOverride(*MissWorld.Controller,
				ETarget::ShortFreeKickDirectDefense, 6));
	MissWorld.Controller->SubmitProjectedPrimaryPlayerIntent();
	MissWorld.Controller->SubmitProjectedPrimaryPlayerIntent();
	const auto MissTerminal = FFMCodexLocalMatchUMGPresentationBuilder::Build(
		MissWorld.Controller->GetInteractionView(),
		MissWorld.Controller->GetResolutionFeedback(), FString());
	const auto& MissView = MissWorld.Controller->GetInteractionView();
	const FString MissTakerName = CardDisplayName(
		MissView, MissView.SetPieceCarrier.OwnerSide,
		MissView.SetPieceCarrier.CardId);
	const FString MissGoalkeeperName = GoalkeeperDisplayName(
		MissView, MissView.CurrentAttackingPlayer
			== EInitialTurnOrderPlayer::PlayerA
				? EInitialTurnOrderPlayer::PlayerB
				: EInitialTurnOrderPlayer::PlayerA);
	TestTrue(TEXT("Near Free Kick miss names the taker and goalkeeper as a save"),
		!MissView.bSetPieceGoal
			&& !MissTakerName.IsEmpty()
			&& !MissGoalkeeperName.IsEmpty()
			&& MissTerminal.InlineFormula.ContestLabel
				== MissTerminal.InlineFormula.NarrativeHeadline
			&& MissTerminal.InlineFormula.ContestLabel.Contains(MissTakerName)
			&& MissTerminal.InlineFormula.ContestLabel.Contains(MissGoalkeeperName)
			&& MissTerminal.InlineFormula.ContestLabel.Contains(
				TEXT("近距离任意球被"))
			&& MissTerminal.InlineFormula.ContestLabel.Contains(TEXT("扑出！"))
			&& MissTerminal.InlineFormula.ResultTitle == TEXT("未进球")
			&& MissTerminal.InlineFormula.StatusLabel
				== TEXT("近距离任意球 · 直接射门 · 未进球")
			&& MissTerminal.InlineFormula.RouteResultLabel.Contains(
				TEXT("进攻"))
			&& !MissTerminal.InlineFormula.ContestLabel.Contains(TEXT("短任意球")));

	FScopedWorld AngledWorld;
	if (!StartSetPiece(*this, AngledWorld, 5,
			ESetPieceSelectedType::ShortFreeKick)
		|| !BindCarrier(*this, *AngledWorld.Controller, true))
	{
		return false;
	}
	AngledWorld.Controller->SubmitShortFreeKickMethod(
		EMatchPlayShortFreeKickMethod::Angled);
	auto ProjectAngled = [&]()
	{
		return FFMCodexLocalMatchUMGPresentationBuilder::Build(
			AngledWorld.Controller->GetInteractionView(),
			AngledWorld.Controller->GetResolutionFeedback(), FString());
	};
	const auto AngledPending = ProjectAngled();
	TestTrue(TEXT("Short Angled starts in the same shell without compare rows"),
		AngledPending.InlineFormula.bVisible
			&& !AngledPending.InlineFormula.bShowFormulaRows
			&& AngledPending.InlineFormula.PrimaryAction.Action.Label
				== TEXT("掷战术配合双骰"));
	TestTrue(TEXT("Short Angled pair overrides accepted"),
		SetOverride(*AngledWorld.Controller, ETarget::ShortFreeKickAngledA, 4)
			&& SetOverride(*AngledWorld.Controller,
				ETarget::ShortFreeKickAngledB, 5));
	AngledWorld.Controller->SubmitProjectedPrimaryPlayerIntent();
	const auto AngledTerminal = ProjectAngled();
	const auto& AngledView = AngledWorld.Controller->GetInteractionView();
	const FString AngledTakerName = CardDisplayName(
		AngledView, AngledView.SetPieceCarrier.OwnerSide,
		AngledView.SetPieceCarrier.CardId);
	TestTrue(TEXT("Short Angled terminal is compact and has no fake defense row"),
		AngledTerminal.InlineFormula.bVisible
			&& !AngledTerminal.InlineFormula.bShowFormulaRows
			&& AngledTerminal.InlineFormula.RouteResultLabel
				== TEXT("D6 4 + D6 5 = 9")
			&& AngledWorld.Controller->GetInteractionView()
				.SetPiecePairedD6Total == 9
			&& AngledTerminal.InlineFormula.ContestLabel
				== AngledTerminal.InlineFormula.NarrativeHeadline
			&& AngledTerminal.InlineFormula.ContestLabel.Contains(AngledTakerName)
			&& AngledTerminal.InlineFormula.ContestLabel.Contains(
				TEXT("近距离任意球战术配合破门！"))
			&& !AngledTerminal.InlineFormula.ContestLabel.Contains(TEXT("扑出"))
			&& AngledTerminal.InlineFormula.StatusLabel
				== TEXT("近距离任意球 · 战术配合 · 进球")
			&& AngledTerminal.InlineFormula.PrimaryAction.Action.Label
				== TEXT("下一回合"));
	UFMCodexInlineResolutionFormulaSurfaceWidget* AngledSurface =
		NewObject<UFMCodexInlineResolutionFormulaSurfaceWidget>(
			GetTransientPackage());
	AngledSurface->TakeWidget();
	AngledSurface->RefreshFromPresentation(AngledTerminal.InlineFormula);
	TestTrue(TEXT("Shared widget structurally collapses both unused compact rows"),
		AngledSurface->GetWidgetFromName(TEXT("InlineFormulaAttackRegion"))
			->GetVisibility() == ESlateVisibility::Collapsed
			&& AngledSurface->GetWidgetFromName(TEXT("InlineFormulaDefenseRegion"))
				->GetVisibility() == ESlateVisibility::Collapsed);

	FFMCodexUMGMatchScreenViewModel AngledMiss;
	TestTrue(TEXT("Short Angled miss fixture reaches terminal"),
		ResolveScenarioPresentation(*this, ECarrierScenario::ShortAngled,
			4, 4, AngledMiss));
	TestTrue(TEXT("Short Angled miss stays outcome-first without fake goalkeeper causality"),
		AngledMiss.InlineFormula.bNarrativeAvailable
			&& AngledMiss.InlineFormula.ContestLabel
				== AngledMiss.InlineFormula.NarrativeHeadline
			&& AngledMiss.InlineFormula.ContestLabel.Contains(
				TEXT("近距离任意球战术配合未能形成进球。"))
			&& !AngledMiss.InlineFormula.ContestLabel.Contains(TEXT("门将"))
			&& !AngledMiss.InlineFormula.ContestLabel.Contains(TEXT("扑出"))
			&& AngledMiss.InlineFormula.RouteResultLabel
				== TEXT("D6 4 + D6 4 = 8")
			&& AngledMiss.InlineFormula.StatusLabel
				== TEXT("近距离任意球 · 战术配合 · 未进球"));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexSetPiecePresentationFamilyMatrixRepairTest,
	"FMCodex.LocalPlay.FullD12SetPieceProduction.07.PresentationFamilyMatrixRepair",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexSetPiecePresentationFamilyMatrixRepairTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexFullD12SetPieceProductionTests;
	(void)Parameters;
	FFMCodexUMGMatchScreenViewModel LongImmediate;
	FFMCodexUMGMatchScreenViewModel LongOpposed;
	FFMCodexUMGMatchScreenViewModel LongOpposedMiss;
	FFMCodexUMGMatchScreenViewModel LongPower;
	FFMCodexUMGMatchScreenViewModel PenaltyDirect;
	FFMCodexUMGMatchScreenViewModel PenaltyDirectMiss;
	FFMCodexUMGMatchScreenViewModel Panenka;
	if (!ResolveScenarioPresentation(*this, ECarrierScenario::LongDirect,
			1, 0, LongImmediate)
		|| !ResolveScenarioPresentation(*this, ECarrierScenario::LongDirect,
			6, 1, LongOpposed)
		|| !ResolveScenarioPresentation(*this, ECarrierScenario::LongDirect,
			3, 6, LongOpposedMiss)
		|| !ResolveScenarioPresentation(*this, ECarrierScenario::LongPower,
			6, 5, LongPower)
		|| !ResolveScenarioPresentation(*this, ECarrierScenario::PenaltyDirect,
			6, 1, PenaltyDirect)
		|| !ResolveScenarioPresentation(*this, ECarrierScenario::PenaltyDirect,
			1, 6, PenaltyDirectMiss)
		|| !ResolveScenarioPresentation(*this, ECarrierScenario::PenaltyPanenka,
			6, 0, Panenka))
	{
		return false;
	}
	TestTrue(TEXT("Long Direct immediate miss hides the nonexistent defense row"),
		LongImmediate.InlineFormula.bVisible
			&& LongImmediate.InlineFormula.bShowFormulaRows
			&& LongImmediate.InlineFormula.bShowAttackRow
			&& !LongImmediate.InlineFormula.bShowDefenseRow
			&& LongImmediate.InlineFormula.ContestLabel
				== LongImmediate.InlineFormula.NarrativeHeadline
			&& LongImmediate.InlineFormula.ContestLabel.Contains(
				TEXT("远距离任意球直接射偏。"))
			&& !LongImmediate.InlineFormula.ContestLabel.Contains(TEXT("门将"))
			&& !LongImmediate.InlineFormula.ContestLabel.Contains(TEXT("扑出"))
			&& LongImmediate.InlineFormula.StatusLabel
				== TEXT("远距离任意球 · 直接射门 · 未进球")
			&& LongImmediate.InlineFormula.RouteResultLabel
				== TEXT("进攻方掷点：1"));
	TestTrue(TEXT("Long Direct opposed result uses both shared formula rows"),
		LongOpposed.InlineFormula.bVisible
			&& LongOpposed.InlineFormula.bShowAttackRow
			&& LongOpposed.InlineFormula.bShowDefenseRow
			&& LongOpposed.InlineFormula.AttackRow.bFinalValueResolved
			&& LongOpposed.InlineFormula.DefenseRow.bFinalValueResolved
			&& LongOpposed.InlineFormula.RouteResultLabel.Contains(
				TEXT(" vs 防守"))
			&& LongOpposed.InlineFormula.ContestLabel
				== LongOpposed.InlineFormula.NarrativeHeadline
			&& (LongOpposed.InlineFormula.ResultTitle == TEXT("进球")
				? LongOpposed.InlineFormula.ContestLabel.Contains(
						TEXT("远距离任意球直接破门！"))
					&& LongOpposed.InlineFormula.StatusLabel
						== TEXT("远距离任意球 · 直接射门 · 进球")
				: LongOpposed.InlineFormula.ContestLabel.Contains(
						TEXT("远距离任意球被"))
					&& LongOpposed.InlineFormula.ContestLabel.Contains(TEXT("化解。"))
					&& LongOpposed.InlineFormula.StatusLabel
						== TEXT("远距离任意球 · 直接射门 · 未进球")));
	TestTrue(TEXT("Long Direct opposed miss names a GK-backed football stop"),
		LongOpposedMiss.InlineFormula.bShowAttackRow
			&& LongOpposedMiss.InlineFormula.bShowDefenseRow
			&& LongOpposedMiss.InlineFormula.ContestLabel.Contains(
				TEXT("远距离任意球被"))
			&& LongOpposedMiss.InlineFormula.ContestLabel.Contains(TEXT("化解。"))
			&& LongOpposedMiss.InlineFormula.RouteResultLabel.Contains(
				TEXT(" vs 防守")));
	TestTrue(TEXT("Long Power uses the compact shared result flow"),
		LongPower.InlineFormula.bVisible
			&& !LongPower.InlineFormula.bShowFormulaRows
			&& LongPower.InlineFormula.ContestLabel.Contains(
				TEXT("远距离任意球重炮轰门得手！"))
			&& LongPower.InlineFormula.StatusLabel
				== TEXT("远距离任意球 · 重炮轰门 · 进球")
			&& LongPower.InlineFormula.RouteResultLabel
				== TEXT("D6 6 + D6 5 = 11"));
	FFMCodexUMGMatchScreenViewModel LongPowerMiss;
	if (!ResolveScenarioPresentation(*this, ECarrierScenario::LongPower, 1, 1, LongPowerMiss)) return false;
	TestTrue(TEXT("Long Power NoGoal uses the same method name and remains outcome-only"),
		LongPowerMiss.InlineFormula.ContestLabel.Contains(TEXT("远距离任意球重炮轰门未能得分。"))
			&& LongPowerMiss.InlineFormula.StatusLabel == TEXT("远距离任意球 · 重炮轰门 · 未进球")
			&& LongPowerMiss.InlineFormula.RouteResultLabel == TEXT("D6 1 + D6 1 = 2")
			&& !LongPowerMiss.InlineFormula.bShowFormulaRows);
	TestTrue(TEXT("Penalty Direct uses the opposed shared formula flow"),
		PenaltyDirect.InlineFormula.bVisible
			&& PenaltyDirect.InlineFormula.bShowAttackRow
			&& PenaltyDirect.InlineFormula.bShowDefenseRow
			&& PenaltyDirect.InlineFormula.ContestLabel.Contains(
				TEXT("主罚点球命中！"))
			&& PenaltyDirect.InlineFormula.StatusLabel
				== TEXT("点球 · 常规点球 · 进球")
			&& PenaltyDirect.InlineFormula.RouteResultLabel.Contains(
				TEXT(" vs 防守"))
			&& PenaltyDirect.InlineFormula.PrimaryAction.Action.Label
				== TEXT("下一回合"));
	TestTrue(TEXT("Penalty Direct miss uses the goalkeeper only on its opposed route"),
		PenaltyDirectMiss.InlineFormula.bShowAttackRow
			&& PenaltyDirectMiss.InlineFormula.bShowDefenseRow
			&& PenaltyDirectMiss.InlineFormula.ContestLabel.Contains(TEXT("点球被"))
			&& PenaltyDirectMiss.InlineFormula.ContestLabel.Contains(TEXT("扑出！"))
			&& PenaltyDirectMiss.InlineFormula.StatusLabel
				== TEXT("点球 · 常规点球 · 未进球")
			&& PenaltyDirectMiss.InlineFormula.RouteResultLabel.Contains(
				TEXT(" vs 防守")));
	TestTrue(TEXT("Penalty -3 label describes the fixed defense adjustment, not goalkeeper guessing"),
		PenaltyDirect.InlineFormula.DefenseRow.Terms.ContainsByPredicate([](const auto& Term)
			{ return Term.DisplayLabel == TEXT("点球防守调整 -3"); })
			&& !PenaltyDirect.InlineFormula.DefenseRow.Terms.ContainsByPredicate([](const auto& Term)
				{ return Term.DisplayLabel.Contains(TEXT("门将预判")); }));
	TestTrue(TEXT("Panenka uses one compact roll without compare scaffolding"),
		Panenka.InlineFormula.bVisible
			&& !Panenka.InlineFormula.bShowFormulaRows
			&& Panenka.InlineFormula.ContestLabel.Contains(
				TEXT("勺子点球命中！"))
			&& !Panenka.InlineFormula.ContestLabel.Contains(TEXT("扑出"))
			&& Panenka.InlineFormula.StatusLabel
				== TEXT("点球 · 勺子点球 · 进球")
			&& Panenka.InlineFormula.RouteResultLabel == TEXT("掷点结果：6")
			&& !Panenka.InlineFormula.PrimaryAction.Action.Label.Contains(
				TEXT("Continue")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexSetPieceP0HelpersAndPairDisclosureTest,
	"FMCodex.LocalPlay.FullD12SetPieceProduction.08.P0HelpersAndPairDisclosure",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexSetPieceP0HelpersAndPairDisclosureTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexFullD12SetPieceProductionTests;
	(void)Parameters;
	for (const bool bShort : { false, true })
	{
		FScopedWorld World;
		if (!StartSetPiece(*this, World, bShort ? 5 : 3,
				bShort ? ESetPieceSelectedType::ShortFreeKick
					: ESetPieceSelectedType::LongFreeKick)
			|| !BindCarrier(*this, *World.Controller, bShort))
		{
			return false;
		}
		if (bShort)
		{
			World.Controller->SubmitShortFreeKickMethod(
				EMatchPlayShortFreeKickMethod::Angled);
		}
		else
		{
			World.Controller->SubmitLongFreeKickMethod(
				EMatchPlayLongFreeKickMethod::Power);
		}
		auto Project = [&]()
		{
			return FFMCodexLocalMatchUMGPresentationBuilder::Build(
				World.Controller->GetInteractionView(),
				World.Controller->GetResolutionFeedback(), FString());
		};
		const FString Target = bShort
			? TEXT("两枚点数总和达到 9 或以上：进球")
			: TEXT("两枚点数总和达到 11 或以上：进球");
		const auto Pending = Project();
		TestEqual(TEXT("Paired pre-roll helper states the canonical target"),
			Pending.InlineFormula.RollHelperLabel, Target);
		if (!bShort)
		{
			TestTrue(TEXT("Long Power pending title and CTA share the new method name without old wording"),
				Pending.InlineFormula.ContestLabel == TEXT("远距离任意球 · 重炮轰门")
					&& World.Controller->GetInteractionView().ContinueActionLabel == TEXT("掷重炮轰门双骰")
					&& !Pending.InlineFormula.ContestLabel.Contains(TEXT("大力轰门")));
		}
		TestTrue(TEXT("Paired pre-roll has no premature result or formula"),
			Pending.InlineFormula.RouteResultLabel.IsEmpty()
				&& !Pending.InlineFormula.bShowFormulaRows);
		if (!SetOverride(*World.Controller, bShort
				? ETarget::ShortFreeKickAngledA : ETarget::LongFreeKickPowerA, 6)
			|| !SetOverride(*World.Controller, bShort
				? ETarget::ShortFreeKickAngledB : ETarget::LongFreeKickPowerB, 5))
		{
			AddError(TEXT("Paired overrides were unavailable."));
			return false;
		}
		auto* Screen = NewObject<UFMCodexLocalMatchScreenWidget>(GetTransientPackage());
		Screen->TakeWidget();
		Screen->SetMatchController(World.Controller);
		Screen->RefreshFromPresentation(Pending);
		Screen->RequestContinueResolution();
		Screen->PauseInlineFormulaRevealTimerForTesting();
		Screen->RefreshFromPresentation(Project());
		TestTrue(TEXT("One request atomically resolves both accepted dice"),
			World.Controller->GetInteractionView().bTerminalPendingAdvance
				&& World.Controller->GetInteractionView().SetPiecePairedD6A == 6
				&& World.Controller->GetInteractionView().SetPiecePairedD6B == 5);
		TestTrue(TEXT("Authority already scored while first reel still shows the old score"),
			World.Controller->GetInteractionView().bSetPieceGoal
				&& Project().Header.ScoreLabel != Pending.Header.ScoreLabel
				&& Screen->GetMatchHeader()->GetDisplayedScoreLabel() == Pending.Header.ScoreLabel);
		const auto StateAfterRequest = SerializeState(World.Host->GetMatchSnapshot().Snapshot);
		auto* Formula = Screen->GetInlineFormulaSurface();
		auto* Helper = Cast<UTextBlock>(Formula->GetWidgetFromName(TEXT("InlineFormulaRollHelper")));
		TestTrue(TEXT("Shared widget renders the pending helper text"),
			Helper != nullptr && Helper->GetVisibility() != ESlateVisibility::Collapsed
				&& Helper->GetText().ToString() == Target);
		TestTrue(TEXT("First cycling shows target without either accepted value"),
			Formula->GetPresentation().RollHelperLabel == Target
				&& Formula->GetPresentation().RouteResultLabel.IsEmpty());
		Screen->AdvanceInlineFormulaRevealForTesting(1.8f);
		TestEqual(TEXT("First hold displays only first accepted die"),
			Formula->GetPresentation().RouteResultLabel,
			FString(TEXT("第一枚 D6：6")));
		TestEqual(TEXT("First hold cannot reveal the final score"),
			Screen->GetMatchHeader()->GetDisplayedScoreLabel(), Pending.Header.ScoreLabel);
		// A completes at 1.46 + 2.58 = 4.04; sample B at elapsed 0.01.
		Screen->AdvanceInlineFormulaRevealForTesting(2.25f);
		TestTrue(TEXT("Second cycling retains first die and target, without a second CTA"),
			Formula->GetPresentation().RouteResultLabel == TEXT("第一枚 D6：6")
				&& Formula->GetPresentation().RollHelperLabel == Target
				&& !Formula->GetPresentation().PrimaryAction.bVisible);
		TestEqual(TEXT("A-to-B handoff never captures the already-updated authority score"),
			Screen->GetMatchHeader()->GetDisplayedScoreLabel(), Pending.Header.ScoreLabel);
		Screen->AdvanceInlineFormulaRevealForTesting(1.34f);
		TestTrue(TEXT("Second reel settling conceals score"),
			Screen->GetInlineFormulaRevealPhase() == EFMCodexUMGInlineFormulaRevealPhase::Settling
				&& Screen->GetMatchHeader()->GetDisplayedScoreLabel() == Pending.Header.ScoreLabel);
		Screen->AdvanceInlineFormulaRevealForTesting(0.45f);
		TestTrue(TEXT("Second hold replaces helper with accepted arithmetic"),
			Formula->GetPresentation().RouteResultLabel == TEXT("D6 6 + D6 5 = 11")
				&& Formula->GetPresentation().RollHelperLabel.IsEmpty());
		TestTrue(TEXT("Shared widget collapses the disclosed helper"),
			Helper != nullptr && Helper->GetVisibility() == ESlateVisibility::Collapsed);
		TestTrue(TEXT("Second arithmetic disclosure is not yet Goal narrative or score disclosure"),
			!Formula->GetPresentation().bNarrativeAvailable
				&& Screen->GetMatchHeader()->GetDisplayedScoreLabel() == Pending.Header.ScoreLabel);
		Screen->AdvanceInlineFormulaRevealForTesting(0.05f);
		TestTrue(TEXT("Second Goal narrative and score arrive together"),
			Formula->GetPresentation().bNarrativeAvailable
				&& Screen->GetMatchHeader()->GetDisplayedScoreLabel() == Project().Header.ScoreLabel);
		Screen->AdvanceInlineFormulaRevealForTesting(3.0f);
		TestTrue(TEXT("Settled pair retains arithmetic and central Next Round"),
			Formula->GetPresentation().RouteResultLabel == TEXT("D6 6 + D6 5 = 11")
				&& Formula->GetPresentation().RollHelperLabel.IsEmpty()
				&& Formula->GetPresentation().PrimaryAction.Action.Label == TEXT("下一回合"));
		TestTrue(TEXT("Sequential display does not change accepted authority state"),
			StateAfterRequest == SerializeState(World.Host->GetMatchSnapshot().Snapshot));
		World.Controller->StartNewDemoMatch();
		Screen->RefreshFromPresentation(Project());
		TestTrue(TEXT("New match cannot retain the old terminal score"),
			!Screen->IsInlineFormulaRevealInputBlocked()
				&& Screen->GetMatchHeader()->GetDisplayedScoreLabel() == Project().Header.ScoreLabel
				&& Project().Header.PlayerAScoreLabel == TEXT("0")
				&& Project().Header.PlayerBScoreLabel == TEXT("0"));
	}
	for (const int32 AttackD6 : { 1, 6 })
	{
		FScopedWorld World;
		if (!StartSetPiece(*this, World, 3, ESetPieceSelectedType::LongFreeKick)
			|| !BindCarrier(*this, *World.Controller, false))
		{
			return false;
		}
		World.Controller->SubmitLongFreeKickMethod(EMatchPlayLongFreeKickMethod::Direct);
		auto Project = [&]()
		{
			return FFMCodexLocalMatchUMGPresentationBuilder::Build(
				World.Controller->GetInteractionView(),
				World.Controller->GetResolutionFeedback(), FString());
		};
		TestTrue(TEXT("Long Direct pending uses canonical name and early-out helper"),
			Project().InlineFormula.ContestLabel == TEXT("远距离任意球 · 直接射门")
				&& Project().InlineFormula.RollHelperLabel == TEXT("1–2：直接射偏"));
		TestTrue(TEXT("Long attack override accepted"),
			SetOverride(*World.Controller, ETarget::LongFreeKickDirectAttack, AttackD6));
		World.Controller->SubmitProjectedPrimaryPlayerIntent();
		TestTrue(TEXT("Accepted Long attack clears helper for defense or immediate terminal"),
			Project().InlineFormula.RollHelperLabel.IsEmpty()
				&& World.Controller->GetInteractionView().InteractionCategory
					== (AttackD6 == 1 ? ECategory::AdvanceAfterTerminal
						: ECategory::RollLongFreeKickDirectDefense));
	}
	for (const int32 PanenkaD6 : { 1, 6 })
	{
		FScopedWorld World;
		if (!StartSetPiece(*this, World, 6, ESetPieceSelectedType::Penalty)
			|| !BindCarrier(*this, *World.Controller, false))
		{
			return false;
		}
		World.Controller->SubmitPenaltyMethod(EMatchPlayPenaltyMethod::Panenka);
		auto Project = [&]()
		{
			return FFMCodexLocalMatchUMGPresentationBuilder::Build(
				World.Controller->GetInteractionView(),
				World.Controller->GetResolutionFeedback(), FString());
		};
		TestTrue(TEXT("Panenka helper belongs to a single-die non-formula surface"),
			Project().InlineFormula.RollHelperLabel == TEXT("1：射失｜2–6：进球")
				&& !Project().InlineFormula.bShowFormulaRows);
		TestTrue(TEXT("Panenka override accepted"),
			SetOverride(*World.Controller, ETarget::PenaltyPanenka, PanenkaD6));
		World.Controller->SubmitProjectedPrimaryPlayerIntent();
		TestTrue(TEXT("Panenka terminal clears helper without fabricating a pair or formula"),
			Project().InlineFormula.RollHelperLabel.IsEmpty()
				&& !Project().InlineFormula.bShowFormulaRows
				&& World.Controller->GetInteractionView().SetPiecePairedD6A == PanenkaD6
				&& World.Controller->GetInteractionView().SetPiecePairedD6B == 0
				&& Project().InlineFormula.RouteResultLabel
					== FString::Printf(TEXT("掷点结果：%d"), PanenkaD6)
				&& World.Controller->GetInteractionView().bTerminalPendingAdvance
				&& World.Controller->GetInteractionView().bSetPieceGoal == (PanenkaD6 != 1));
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexCornerFocusedProductionRepairTest,
	"FMCodex.LocalPlay.FullD12SetPieceProduction.09.CornerFocusedProductionRepair",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexCornerFocusedProductionRepairTest::RunTest(const FString& Parameters)
{
	using namespace FMCodexFullD12SetPieceProductionTests;
	const FString HighHint = FFMCodexTacticalDetailPresentationBuilder::BuildCornerChoiceHint(EMatchPlayCornerRouteIntent::High).ToString();
	const FString LowHint = FFMCodexTacticalDetailPresentationBuilder::BuildCornerChoiceHint(EMatchPlayCornerRouteIntent::Low).ToString();
	TestTrue(TEXT("Corner canonical educational helpers explain distinct attributes and defense average"),
		HighHint.Contains(TEXT("力量")) && HighHint.Contains(TEXT("制空")) && HighHint.Contains(TEXT("取平均"))
			&& LowHint.Contains(TEXT("射门")) && LowHint.Contains(TEXT("盯防")) && LowHint.Contains(TEXT("反应")));
	for (int32 Count = 0; Count <= 3; ++Count)
	{
		FScopedWorld World;
		if (!StartSetPiece(*this, World, 1, ESetPieceSelectedType::Corner)) return false;
		auto* Controller = World.Controller;
		const auto Legal = Controller->GetInteractionView().LegalSetPieceCardIds;
		if (!TestTrue(TEXT("Fixture has three nominees"), Legal.Num() >= 3)) return false;
		for (int32 Index = 0; Index < Count; ++Index) Controller->ToggleSetPieceDraftCard(Legal[Index]);
		const auto Draft = Controller->GetInteractionView().DraftCornerNomineeCardIds;
		const auto BeforeLock = SerializeState(World.Host->GetMatchSnapshot().Snapshot);
		auto* Screen = NewObject<UFMCodexLocalMatchScreenWidget>(GetTransientPackage());
		Screen->TakeWidget();
		Screen->SetMatchController(Controller);
		auto Refresh = [&]() { Screen->RefreshFromPresentation(FFMCodexLocalMatchUMGPresentationBuilder::Build(
			Controller->GetInteractionView(), Controller->GetResolutionFeedback(), FString())); };
		Refresh();
		const auto* Detail = Cast<UTextBlock>(Screen->GetWidgetFromName(TEXT("SetPieceProductionDetail")));
		TestTrue(TEXT("Draft uses player-facing candidate language"), Detail
			&& Detail->GetText().ToString().Contains(FString::Printf(TEXT("已选：%d/3"), Count))
			&& !Detail->GetText().ToString().Contains(TEXT("当前草稿")));
		auto CheckOrderedNames = [&]()
		{
			const auto& Editing = Controller->GetInteractionView();
			for (int32 Index = 0; Index < Editing.DraftCornerNomineeCardIds.Num(); ++Index)
			{
				const FName Id = Editing.DraftCornerNomineeCardIds[Index];
				const auto* Name = Cast<UTextBlock>(Screen->GetWidgetFromName(
					FName(*FString::Printf(TEXT("CornerDraftCandidate%d"), Index + 1))));
				TestTrue(TEXT("Editing visibly preserves compact order and canonical player name without raw ID"),
					Name && Name->GetText().ToString() == FString::Printf(TEXT("%d. %s"), Index + 1,
						*CardDisplayName(Editing, Editing.ExpectedActingPlayer, Id))
						&& !Name->GetText().ToString().Contains(Id.ToString())
						&& Name->GetParent()->GetVisibility() != ESlateVisibility::Collapsed);
			}
		};
		CheckOrderedNames();
		if (Count == 3)
		{
			Controller->ToggleSetPieceDraftCard(Legal[1]);
			Refresh();
			CheckOrderedNames();
			Controller->ToggleSetPieceDraftCard(Legal[1]);
			Refresh();
			CheckOrderedNames();
			TestTrue(TEXT("Reselected player appends after remaining candidates"),
				Controller->GetInteractionView().DraftCornerNomineeCardIds
					== TArray<FName>({Legal[0], Legal[2], Legal[1]}));
			Controller->ToggleSetPieceDraftCard(Legal[2]);
			Controller->ToggleSetPieceDraftCard(Legal[2]);
			Refresh();
		}
		auto* Primary = Cast<UButton>(Screen->GetWidgetFromName(TEXT("SetPieceProductionPrimaryAction")));
		auto* Return = Cast<UButton>(Screen->GetWidgetFromName(TEXT("CornerReturnToNominations")));
		auto* PrimaryBounds = Cast<USizeBox>(Screen->GetWidgetFromName(TEXT("SetPiecePrimaryActionBounds")));
		auto* ReturnBounds = Cast<USizeBox>(Screen->GetWidgetFromName(TEXT("CornerReturnActionBounds")));
		if (!Primary || !Return || !PrimaryBounds || !ReturnBounds) return false;
		auto* PrimaryLabel = CastChecked<UTextBlock>(Primary->GetChildAt(0));
		TestTrue(TEXT("Normal lock is single-line in a stable horizontal action row"),
			PrimaryLabel->GetText().ToString() == TEXT("锁定进攻候选") && !PrimaryLabel->GetAutoWrapText()
				&& Cast<UHorizontalBox>(PrimaryBounds->GetParent())
				&& PrimaryBounds->GetParent() == ReturnBounds->GetParent()
				&& PrimaryBounds->GetMinDesiredWidth() == 200.0f);
		Controller->ConfirmSetPieceDraft();
		if (Count < 3)
		{
			Refresh();
			const auto* ReturnButton = Screen->GetWidgetFromName(TEXT("CornerReturnToNominations"));
			TestTrue(TEXT("First underfilled click preserves sealed authority state"),
				BeforeLock == SerializeState(World.Host->GetMatchSnapshot().Snapshot)
					&& Controller->GetInteractionView().bCornerLockConfirmationPending
					&& ReturnButton && ReturnButton->GetVisibility() == ESlateVisibility::Visible);
			TestTrue(TEXT("Confirmation actions remain horizontal and both labels single-line"),
				PrimaryLabel->GetText().ToString() == TEXT("继续锁定")
					&& !PrimaryLabel->GetAutoWrapText()
					&& !CastChecked<UTextBlock>(Return->GetChildAt(0))->GetAutoWrapText()
					&& ReturnBounds->GetVisibility() == ESlateVisibility::SelfHitTestInvisible
					&& PrimaryBounds->GetParent() == ReturnBounds->GetParent());
			Controller->CancelCornerLockConfirmation();
			Refresh();
			TestTrue(TEXT("Return restores normal label without reparenting or leaking confirmation dimensions"),
				PrimaryLabel->GetText().ToString() == TEXT("锁定进攻候选")
					&& !PrimaryLabel->GetAutoWrapText() && Primary->GetParent() == PrimaryBounds
					&& PrimaryBounds->GetMinDesiredWidth() == 200.0f
					&& ReturnBounds->GetVisibility() == ESlateVisibility::Collapsed);
			TestTrue(TEXT("Return to nomination preserves order and remains local"),
				!Controller->GetInteractionView().bCornerLockConfirmationPending
					&& Controller->GetInteractionView().DraftCornerNomineeCardIds == Draft
					&& BeforeLock == SerializeState(World.Host->GetMatchSnapshot().Snapshot));
			Controller->ConfirmSetPieceDraft();
			Controller->ConfirmSetPieceDraft();
		}
		TestTrue(TEXT("Confirmed lock (or full-three direct lock) advances once"),
			Controller->GetInteractionView().InteractionCategory == ECategory::DraftCornerDefender
				&& !Controller->GetInteractionView().bCornerLockConfirmationPending);
		Refresh();
		TestTrue(TEXT("Sealed attacker list is removed from the editing presentation"),
			PrimaryLabel->GetText().ToString() == TEXT("锁定防守候选")
				&& Screen->GetWidgetFromName(TEXT("CornerDraftCandidateList"))->GetVisibility() == ESlateVisibility::Collapsed
				&& CastChecked<UTextBlock>(Screen->GetWidgetFromName(TEXT("CornerDraftCandidate1")))->GetText().IsEmpty());
		const auto BeforeAutomaticHeader = Screen->GetMatchHeader()->GetPresentation();
		if (Count > 1) TestTrue(TEXT("Automatic scorer deterministic typed backend override accepted"),
			SetOverride(*Controller, ETarget::CornerAutomaticScorer, 6));
		Controller->ConfirmSetPieceDraft();
		Controller->ConfirmSetPieceDraft();
		TestEqual(TEXT("No-roll authority terminal cannot mutate a header before its surface refresh"),
			Screen->GetMatchHeader()->GetPresentation().ScoreLabel, BeforeAutomaticHeader.ScoreLabel);
		Refresh();
		const auto& View = Controller->GetInteractionView();
		const auto& Formula = Screen->GetInlineFormulaSurface()->GetPresentation();
		TestTrue(TEXT("Zero defender resolves directly to explicit terminal, never to a visible roll or Formula"),
			View.bTerminalPendingAdvance && View.bSetPieceGoal == (Count > 0)
				&& !View.bHasCornerSharedParticipantD6 && !View.bHasCornerRouteD6
				&& !View.bHasSetPieceAttackD6 && !View.bHasSetPieceDefenseD6
				&& !View.bHasSetPieceFormula && !Formula.bShowFormulaRows
				&& !Screen->IsInlineFormulaRevealInputBlocked()
				&& Formula.PrimaryAction.Action.Label == TEXT("下一回合")
				&& !Formula.ContestLabel.Contains(TEXT("系统进球")));
		TestTrue(TEXT("No-roll terminal discloses its outcome and authoritative score together without a timer"),
			Formula.bNarrativeAvailable
				&& Screen->GetMatchHeader()->GetPresentation().ScoreLabel
					== FFMCodexLocalMatchUMGPresentationBuilder::Build(View, Controller->GetResolutionFeedback(), FString()).Header.ScoreLabel);
		if (Count > 0)
		{
			TestTrue(TEXT("Automatic terminal projects actual nominated scorer, not a raw hidden draw"),
				View.SetPieceGoalScorerCardId == Draft.Last() && View.CornerRunner.bIsBound);
			TestTrue(TEXT("Automatic Goal names its real scorer in football-event language"),
				Formula.ContestLabel == CardDisplayName(View, View.CornerRunner.OwnerSide, View.CornerRunner.CardId) + TEXT("角球破门！")
					&& Formula.StatusLabel == TEXT("角球 · 进球"));
		}
		else
		{
			TestTrue(TEXT("Zero versus zero explicitly means nobody reached the ball, not a generic system NoGoal"),
				Formula.ContestLabel == TEXT("进攻方无人抢到点") && Formula.StatusLabel == TEXT("角球 · 未进球"));
		}
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexCornerRouteCopyAndNarrativeTest,
	"FMCodex.LocalPlay.FullD12SetPieceProduction.10.CornerRouteBonusAndNarrative",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexCornerRouteCopyAndNarrativeTest::RunTest(const FString& Parameters)
{
	using namespace FMCodexFullD12SetPieceProductionTests;
	for (const bool bHigh : {true, false})
	{
		const auto Intent = bHigh ? EMatchPlayCornerRouteIntent::High : EMatchPlayCornerRouteIntent::Low;
		TestEqual(TEXT("Corner pending helper states both complete route ranges"),
			FFMCodexTacticalDetailPresentationBuilder::BuildCornerRouteHint(Intent).ToString(),
			FString(bHigh ? TEXT("1–4：高球｜5–6：低平球") : TEXT("1–4：低平球｜5–6：高球")));
		TestEqual(TEXT("Cross pending helper states both complete route ranges"),
			FFMCodexTacticalDetailPresentationBuilder::BuildCrossRouteHint(bHigh
				? EMatchPlayElectiveBranchIntent::CrossHigh : EMatchPlayElectiveBranchIntent::CrossLow).ToString(),
			FString(bHigh ? TEXT("1–4：高球传中｜5–6：低球传中") : TEXT("1–4：低球传中｜5–6：高球传中")));
		const auto* Cross = FTacticalRuleDescriptionCatalog::FindBySkillType(ESkillRuleType::Cross);
		if (!Cross) return false;
		for (int32 D6 = 1; D6 <= 6; ++D6)
		{
			FCrossSelectionQueryInput Input;
			Input.IntendedCrossType = bHigh ? ECrossIntentType::High : ECrossIntentType::Low;
			Input.bHasExternalSelectionD6 = true;
			Input.ExternalSelectionD6 = D6;
			const auto Result = FCrossSelectionQuery::Select(Input);
			const auto* Range = Cross->InitialRouteOutcomes.FindByPredicate([D6](const FTacticalRuleDescriptionOutcome& O)
				{ return D6 >= O.Minimum && D6 <= O.Maximum; });
			TestTrue(TEXT("Every canonical Cross educational range matches the authoritative query"),
				Result.bSuccess && Range && (Range->OutcomeId == TEXT("Cross.PreferredRoute"))
					== ((Result.ActualCrossType == ECrossActualType::High) == bHigh));
		}
		for (const auto Counts : {TPair<int32, int32>(2, 1), {3, 1}, {1, 2}, {1, 3}})
		{
			FScopedWorld World;
			if (!StartSetPiece(*this, World, 1, ESetPieceSelectedType::Corner)) return false;
			auto* Controller = World.Controller;
			for (const int32 Count : {Counts.Key, Counts.Value})
			{
				const auto Legal = Controller->GetInteractionView().LegalSetPieceCardIds;
				if (Legal.Num() < Count) return false;
				for (int32 Index = 0; Index < Count; ++Index) Controller->ToggleSetPieceDraftCard(Legal[Index]);
				Controller->ConfirmSetPieceDraft();
				if (Count < 3) Controller->ConfirmSetPieceDraft();
			}
			SetOverride(*Controller, ETarget::CornerParticipantSelection, 1);
			Controller->SubmitProjectedPrimaryPlayerIntent();
			Controller->SubmitCornerIntent(Intent);
			auto Project = [&]() { return FFMCodexLocalMatchUMGPresentationBuilder::Build(
				Controller->GetInteractionView(), Controller->GetResolutionFeedback(), FString()); };
			TestEqual(TEXT("Only route-pending surface exposes the canonical Corner mapping"),
				Project().InlineFormula.RollHelperLabel,
				FFMCodexTacticalDetailPresentationBuilder::BuildCornerRouteHint(Intent).ToString());
			SetOverride(*Controller, ETarget::CornerRoute, 5);
			Controller->SubmitProjectedPrimaryPlayerIntent();
			const auto Formula = Project().InlineFormula;
			const int32 Bonus = Controller->GetInteractionView().CornerCandidateBonus;
			const auto& LargerRow = Counts.Key > Counts.Value ? Formula.AttackRow : Formula.DefenseRow;
			const auto* BonusTerm = LargerRow.Terms.FindByPredicate([Bonus](const FFMCodexUMGInlineFormulaTermViewModel& Term)
				{ return Term.DisplayLabel == FString::Printf(TEXT("候选人数占优加成 %d"), Bonus); });
			TestTrue(TEXT("Authoritative +2/+3 candidate advantage remains distinct from fixed defense +2"),
				BonusTerm && Bonus == (FMath::Abs(Counts.Key - Counts.Value) == 1 ? 2 : 3)
					&& Formula.DefenseRow.Terms.ContainsByPredicate([](const FFMCodexUMGInlineFormulaTermViewModel& Term)
						{ return Term.DisplayLabel == TEXT("防守加成 2"); })
					&& Formula.RollHelperLabel.IsEmpty());
			const bool bGoal = Counts.Key > Counts.Value;
			SetOverride(*Controller, ETarget::CornerAttack, bGoal ? 6 : 1);
			SetOverride(*Controller, ETarget::CornerDefense, bGoal ? 1 : 6);
			Controller->SubmitProjectedPrimaryPlayerIntent();
			Controller->SubmitProjectedPrimaryPlayerIntent();
			const auto& View = Controller->GetInteractionView();
			const auto Terminal = Project().InlineFormula;
			TestTrue(TEXT("Corner narrative uses actual Runner and Goal/NoGoal without invented save or miss cause"),
				View.bHasSetPieceOutcome && View.bSetPieceGoal == bGoal
					&& Terminal.ContestLabel.Contains(CardDisplayName(View, View.CornerRunner.OwnerSide, View.CornerRunner.CardId))
					&& Terminal.ContestLabel.Contains(bGoal ? TEXT("角球破门") : TEXT("未能得分"))
					&& !Terminal.ContestLabel.Contains(TEXT("扑出")) && !Terminal.ContestLabel.Contains(TEXT("偏出"))
					&& !Terminal.ContestLabel.Contains(TEXT("头球")) && !Terminal.ContestLabel.Contains(TEXT("凌空"))
					&& Terminal.RollHelperLabel.IsEmpty());
			if (!bGoal) TestTrue(TEXT("NoGoal copy follows actual switched route, not selected intent"),
				Terminal.ContestLabel.Contains(bHigh ? TEXT("低平球攻门") : TEXT("高球攻门")));
		}
	}
	for (int32 DefenderCount = 1; DefenderCount <= 3; ++DefenderCount)
	{
		FScopedWorld World;
		if (!StartSetPiece(*this, World, 1, ESetPieceSelectedType::Corner)) return false;
		auto* Controller = World.Controller;
		Controller->ConfirmSetPieceDraft();
		Controller->ConfirmSetPieceDraft();
		const auto Legal = Controller->GetInteractionView().LegalSetPieceCardIds;
		for (int32 Index = 0; Index < DefenderCount; ++Index) Controller->ToggleSetPieceDraftCard(Legal[Index]);
		Controller->ConfirmSetPieceDraft();
		if (DefenderCount < 3) Controller->ConfirmSetPieceDraft();
		const auto& View = Controller->GetInteractionView();
		const auto Terminal = FFMCodexLocalMatchUMGPresentationBuilder::Build(View, Controller->GetResolutionFeedback(), FString());
		TestTrue(TEXT("Attacker zero versus nonzero defender remains no-roll NoGoal with explicit football language"),
			View.bHasSetPieceOutcome && !View.bSetPieceGoal && !View.bHasCornerSharedParticipantD6
				&& !View.bHasCornerRouteD6 && !View.bHasSetPieceAttackD6 && !View.bHasSetPieceDefenseD6
				&& Terminal.InlineFormula.ContestLabel == TEXT("进攻方无人抢到点")
				&& Terminal.InlineFormula.StatusLabel == TEXT("角球 · 未进球")
				&& Terminal.Header.PlayerAScoreLabel == TEXT("0") && Terminal.Header.PlayerBScoreLabel == TEXT("0"));
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexPenaltyScoreDisclosureTest,
	"FMCodex.LocalPlay.FullD12SetPieceProduction.11.PenaltyScoreDisclosureAndReset",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexPenaltyScoreDisclosureTest::RunTest(const FString& Parameters)
{
	using namespace FMCodexFullD12SetPieceProductionTests;
	for (const bool bGoal : {true, false})
	{
		FScopedWorld World;
		if (!StartSetPiece(*this, World, 6, ESetPieceSelectedType::Penalty)
			|| !BindCarrier(*this, *World.Controller, false)) return false;
		auto* Controller = World.Controller;
		Controller->SubmitPenaltyMethod(EMatchPlayPenaltyMethod::Direct);
		SetOverride(*Controller, ETarget::PenaltyDirectAttack, bGoal ? 6 : 1);
		SetOverride(*Controller, ETarget::PenaltyDirectDefense, bGoal ? 1 : 6);
		Controller->SubmitProjectedPrimaryPlayerIntent();
		auto Project = [&]() { return FFMCodexLocalMatchUMGPresentationBuilder::Build(
			Controller->GetInteractionView(), Controller->GetResolutionFeedback(), FString(), EInitialTurnOrderPlayer::PlayerB); };
		auto* Screen = NewObject<UFMCodexLocalMatchScreenWidget>(GetTransientPackage());
		Screen->TakeWidget();
		Screen->SetMatchController(Controller);
		Screen->RefreshFromPresentation(Project());
		const auto Before = Screen->GetMatchHeader()->GetPresentation();
		Controller->SubmitProjectedPrimaryPlayerIntent();
		const auto Terminal = Project();
		TestEqual(TEXT("Penalty fixture proves both authoritative Goal and NoGoal"), Controller->GetInteractionView().bSetPieceGoal, bGoal);
		if (!bGoal) TestEqual(TEXT("NoGoal does not replay a prior score delta"), Terminal.Header.ScoreLabel, Before.ScoreLabel);
		Screen->RefreshFromPresentation(Terminal);
		Screen->PauseInlineFormulaRevealTimerForTesting();
		const auto Frozen = SerializeState(World.Host->GetMatchSnapshot().Snapshot);
		CheckDecisiveScoreReveal(*this, *Screen, Before, Terminal);
		TestTrue(TEXT("Player B orientation attaches each score to its typed owner"),
			Screen->GetMatchHeader()->GetPresentation().LeftPlayerSide == EInitialTurnOrderPlayer::PlayerB
				&& Screen->GetMatchHeader()->GetPresentation().LeftScoreLabel == Terminal.Header.PlayerBScoreLabel
				&& Screen->GetMatchHeader()->GetPresentation().RightScoreLabel == Terminal.Header.PlayerAScoreLabel);
		TestTrue(TEXT("Score disclosure has no authority mutations"), Frozen == SerializeState(World.Host->GetMatchSnapshot().Snapshot));
		auto* Reconstructed = NewObject<UFMCodexLocalMatchScreenWidget>(GetTransientPackage());
		Reconstructed->TakeWidget();
		Reconstructed->SetMatchController(Controller);
		Reconstructed->RefreshFromPresentation(Terminal);
		TestTrue(TEXT("Fresh terminal reconstruction immediately renders disclosed outcome and score without invented reel"),
			!Reconstructed->IsInlineFormulaRevealInputBlocked()
				&& Reconstructed->GetInlineFormulaSurface()->GetPresentation().bNarrativeAvailable
				&& Reconstructed->GetMatchHeader()->GetDisplayedScoreLabel() == Terminal.Header.ScoreLabel);
		Controller->SubmitProjectedPrimaryPlayerIntent();
		Screen->RefreshFromPresentation(Project());
		TestTrue(TEXT("Advance releases presentation without dropping the disclosed goal"),
			!Screen->IsInlineFormulaRevealInputBlocked()
				&& Screen->GetMatchHeader()->GetPresentation().PlayerAScoreLabel == Terminal.Header.PlayerAScoreLabel
				&& Screen->GetMatchHeader()->GetPresentation().PlayerBScoreLabel == Terminal.Header.PlayerBScoreLabel);
		Controller->StartNewDemoMatch();
		Screen->RefreshFromPresentation(Project());
		TestTrue(TEXT("Reset clears score history and terminal narrative"),
			!Screen->IsInlineFormulaRevealInputBlocked() && !Screen->GetInlineFormulaSurface()->GetPresentation().bNarrativeAvailable
				&& Screen->GetMatchHeader()->GetDisplayedScoreLabel() == TEXT("0 - 0"));
		if (bGoal)
		{
			if (!StartSetPiece(*this, World, 6, ESetPieceSelectedType::Penalty)
				|| !BindCarrier(*this, *Controller, false)) return false;
			Controller->SubmitPenaltyMethod(EMatchPlayPenaltyMethod::Panenka);
			Screen->RefreshFromPresentation(Project());
			SetOverride(*Controller, ETarget::PenaltyPanenka, 6);
			Controller->SubmitProjectedPrimaryPlayerIntent();
			Screen->RefreshFromPresentation(Project());
			Screen->PauseInlineFormulaRevealTimerForTesting();
			TestTrue(TEXT("New scoring branch is active with score concealed before reset"),
				Screen->IsInlineFormulaRevealInputBlocked() && Controller->GetInteractionView().bSetPieceGoal
					&& Screen->GetMatchHeader()->GetDisplayedScoreLabel() == TEXT("0 - 0"));
			Controller->StartNewDemoMatch();
			Screen->RefreshFromPresentation(Project());
			Screen->AdvanceInlineFormulaRevealForTesting(5.0f);
			TestTrue(TEXT("Reset during decisive reel cancels stale disclosure rather than leaking the old Goal"),
				!Screen->IsInlineFormulaRevealInputBlocked()
					&& Screen->GetMatchHeader()->GetDisplayedScoreLabel() == TEXT("0 - 0")
					&& !Screen->GetInlineFormulaSurface()->GetPresentation().bNarrativeAvailable);
		}
	}
	return true;
}

#endif
