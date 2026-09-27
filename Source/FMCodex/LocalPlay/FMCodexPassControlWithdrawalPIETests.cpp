#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && !UE_BUILD_SHIPPING
#include "FMCodexCardRackWidget.h"
#include "FMCodexLocalMatchPlayerController.h"
#include "FMCodexLocalMatchScreenWidget.h"
#include "FMCodexPlayerCardWidget.h"
#include "Components/HorizontalBox.h"
#include "Editor.h"
#include "Engine/World.h"
#include "Misc/AutomationTest.h"
#include "PlayInEditorDataTypes.h"
#include "Settings/LevelEditorPlaySettings.h"
#include "Tests/AutomationCommon.h"
#include "Tests/AutomationEditorCommon.h"

namespace
{
class FStartWithdrawalPIE final : public IAutomationLatentCommand
{
public:
	bool Update() override
	{
		auto* Settings = DuplicateObject<ULevelEditorPlaySettings>(
			GetDefault<ULevelEditorPlaySettings>(), GetTransientPackage());
		Settings->NewWindowWidth = 1600;
		Settings->NewWindowHeight = 900;
		Settings->SetPlayNetMode(EPlayNetMode::PIE_Standalone);
		Settings->SetPlayNumberOfClients(1);
		FRequestPlaySessionParams Params;
		Params.EditorPlaySettings = Settings;
		GEditor->RequestPlaySession(Params);
		return true;
	}
};

// One real Local PIE session: canonical content only, typed player requests and
// existing DEV dice provider. Never mutate State or advance a reveal clock.
class FWithdrawalPIE final : public IAutomationLatentCommand
{
public:
	explicit FWithdrawalPIE(FAutomationTestBase* InTest) : Test(InTest) {}
	bool Update() override
	{
		if (FPlatformTime::Seconds() - Started > 150)
		{
			Test->AddError(TEXT("PassControl withdrawal PIE timed out"));
			return true;
		}
		if (!GEditor || !GEditor->PlayWorld) return false;
		auto* C = Cast<AFMCodexLocalMatchPlayerController>(GEditor->PlayWorld->GetFirstPlayerController());
		auto* S = C ? C->GetPlayerMatchScreen() : nullptr;
		if (!S || FPlatformTime::Seconds() - Changed < .6) return false;
		if (Step == 0) { S->RequestStartNewMatch(); Advance(); return false; }
		if (S->IsInlineFormulaRevealInputBlocked()) return false;
		using Category = EFMCodexLocalMatchInteractionCategory;
		const auto& V = C->GetInteractionView();
		if (Step == 1)
		{
			if (!Override(*C, EFMCodexLocalDevRollTarget::FullD12, 8)) return true;
			S->RequestRollTacticalPoints(); Advance(); return false;
		}
		if (Step == 2)
		{
			if (!Test->TestEqual(TEXT("Natural D12 reveal reaches deployment"), V.InteractionCategory, Category::Deploy)) return true;
			if (Possession == 0 && !Inspect(*S)) return true;
			const bool IsA = V.CurrentAttackingPlayer == EInitialTurnOrderPlayer::PlayerA;
			Carrier = Possession == 0
				? FName(IsA ? TEXT("Prototype.Arsenal.MartinZubimendi") : TEXT("Prototype.ManchesterCity.BernardoSilva"))
				: FName(IsA ? TEXT("Prototype.Arsenal.MartinOdegaard") : TEXT("Prototype.ManchesterCity.Rodri"));
			const FName Runner(IsA ? TEXT("Prototype.Arsenal.MylesLewisSkelly") : TEXT("Prototype.ManchesterCity.RayanAitNouri"));
			const FString Own = IsA ? TEXT("NearA") : TEXT("NearB");
			const FString Forward = IsA ? TEXT("NearB") : TEXT("NearA");
			if (!Deploy(*C, *S, Own, Carrier) || !Deploy(*C, *S, Own)
				|| !Deploy(*C, *S, Forward, Runner) || !Deploy(*C, *S, Forward)) return true;
			S->RequestFinishDeployment();
			S->RequestFinishDeployment();
			if (!Test->TestTrue(TEXT("Legal deployment completes"), C->GetLastDiagnostic().bHostSuccess)) return true;
			for (const auto& Region : S->GetPresentation().PitchRegions)
				for (const auto& Slot : Region.Slots)
					if (Slot.bOccupied)
					{
						CheckCard(Slot.Card);
						if (Possession == 1 && Slot.Card.CardId == Carrier)
							bSawNoOption = Slot.Card.EligibleTacticalSkills.IsEmpty();
					}
			Advance(); return false;
		}
		if (V.InteractionCategory == Category::TacticalPointRoll && Step >= 3)
		{
			// Empty skill availability is completed automatically by the canonical
			// coordinator; unlike a rolled contest it has no terminal confirmation.
			Test->TestTrue(TEXT("Resolved attack returns to a usable roll action"),
				(Possession == 0 ? bSawTerminal : bSawNoOption)
				&& !V.bCurrentAttackActive && S->GetPresentation().Interaction.bCanRollTacticalPoints);
			Test->AddInfo(FString::Printf(TEXT("WITHDRAWAL_PIE possession=%d completed game=%.3f"), Possession, GEditor->PlayWorld->GetTimeSeconds()));
			if (++Possession == 2)
			{
				Test->TestTrue(TEXT("Production ThroughBall resolved and high-TP no-option progressed"), bSelectedThroughBall && bSawNoOption);
				return true;
			}
			Step = 1; bSawTerminal = false; Changed = FPlatformTime::Seconds(); return false;
		}
		Test->AddInfo(FString::Printf(TEXT("WITHDRAWAL_PIE possession=%d category=%d options=%d terminal=%d"),
			Possession, static_cast<int32>(V.InteractionCategory), V.SelectionOptions.Num(), V.bTerminalPendingAdvance));
		switch (V.InteractionCategory)
		{
		case Category::SelectCarrier: S->RequestSubmitCarrier(Carrier); break;
		case Category::SelectMarker:
		case Category::SelectRunner:
		case Category::SelectHelper:
			if (V.SelectionOptions.IsEmpty())
			{
				if (V.bCanResolveNoLegalChoice) S->RequestResolveNoLegalSelection();
				else S->RequestDeclineSelection();
			}
			else
			{
				const FName Id = V.SelectionOptions[0].Id;
				if (V.InteractionCategory == Category::SelectMarker) S->RequestSubmitMarker(Id);
				else if (V.InteractionCategory == Category::SelectRunner) S->RequestSubmitRunner(Id);
				else S->RequestSubmitHelper(Id);
			}
			break;
		case Category::SelectSkill:
			for (const auto& O : V.SelectionOptions) Test->TestTrue(TEXT("Production selection never offers PassControl"), O.SkillType != ESkillRuleType::PassControl);
			if (Possession == 1)
			{
				if (!Test->TestTrue(TEXT("TP8 Case A carrier has no matching tactics"), V.SelectionOptions.IsEmpty() && V.bCanResolveNoLegalChoice)) return true;
				bSawNoOption = true; S->RequestResolveNoLegalSelection();
			}
			else
			{
				const auto* Option = V.SelectionOptions.FindByPredicate([](const auto& O) { return O.SkillType == ESkillRuleType::ThroughBall; });
				if (!Test->TestNotNull(TEXT("TP8 canonical ThroughBall available"), Option)) return true;
				const FName Id = Option->Id;
				bSelectedThroughBall = true; S->RequestSubmitSkill(Id);
			}
			break;
		case Category::RollThroughBallInitialRoute:
			if (!Override(*C, EFMCodexLocalDevRollTarget::ThroughBallRoute, 1)) return true;
			S->RequestContinueResolution(); break;
		case Category::RollThroughBallFeetAttack:
			if (!Override(*C, EFMCodexLocalDevRollTarget::ThroughBallFeetAttack, 1)) return true;
			S->RequestContinueResolution(); break;
		case Category::RollThroughBallFeetDefense:
			if (!Override(*C, EFMCodexLocalDevRollTarget::ThroughBallFeetDefense, 6)) return true;
			S->RequestContinueResolution(); break;
		case Category::AdvanceAfterTerminal:
			bSawTerminal = true;
			if (Possession == 1) Test->TestTrue(TEXT("No-option path reached terminal with no replacement tactic"), bSawNoOption);
			S->RequestContinueResolution(); break;
		default:
			Test->AddError(TEXT("Unexpected withdrawal PIE action")); return true;
		}
		if (!Test->TestTrue(TEXT("Original Screen action succeeds"), C->GetLastDiagnostic().bHostSuccess)) return true;
		Changed = FPlatformTime::Seconds();
		return false;
	}
private:
	void Advance() { ++Step; Changed = FPlatformTime::Seconds(); }
	bool Override(AFMCodexLocalMatchPlayerController& C, EFMCodexLocalDevRollTarget Target, int32 Value)
	{
		FFMCodexLocalDevRollOverrideRequest R; R.Target = Target; R.Value = Value;
		return Test->TestTrue(TEXT("Existing DEV dice provider accepts override"), C.SetLocalDevRollOverride(R).bSuccess);
	}
	bool Deploy(AFMCodexLocalMatchPlayerController& C, UFMCodexLocalMatchScreenWidget& S, const FString& Half, FName Card = NAME_None)
	{
		const auto* O = C.GetInteractionView().DeploymentOptions.FindByPredicate([&](const auto& Option)
		{
			return !Option.bGoalkeeper && (Card.IsNone() || Option.CardId == Card) && Option.SlotId.ToString().Contains(Half);
		});
		if (!Test->TestNotNull(TEXT("Canonical legal deployment option"), O)) return false;
		const FName CardId = O->CardId, SlotId = O->SlotId;
		S.RequestDeployOrdinary(CardId, SlotId);
		return Test->TestTrue(TEXT("Screen deployment accepted"), C.GetLastDiagnostic().bHostSuccess);
	}
	void CheckCard(const FFMCodexUMGCardViewModel& Card)
	{
		for (const auto* Skills : { &Card.Skills, &Card.EligibleTacticalSkills, &Card.HandMicroVisibleTacticalSkills, &Card.PitchMiniVisibleTacticalSkills })
			for (const auto& Skill : *Skills)
				Test->TestFalse(TEXT("Full/Hand/Pitch content has no PassControl"), Skill.SkillId.ToString().Contains(TEXT("PassControl")));
	}
	bool Inspect(UFMCodexLocalMatchScreenWidget& S)
	{
		int32 Inspected = 0;
		for (auto* Rack : { S.GetLocalRackWidget(), S.GetOpponentRackWidget() })
		{
			if (!Test->TestNotNull(TEXT("Live hand rack"), Rack)) return false;
			for (auto Card : Rack->GetRenderedCardWidgets())
			{
				CheckCard(Card->GetPresentation());
				const FString Id = Card->GetPresentation().CardId.ToString();
				if (!Id.EndsWith(TEXT("MartinOdegaard")) && !Id.EndsWith(TEXT("MartinZubimendi"))) continue;
				Card->OnDetailHoverRequested.Broadcast(Card);
				if (!Test->TestTrue(TEXT("Real deployment Full Card opens"), S.IsDetailOverlayVisible())) return false;
				const auto& Full = S.GetDetailOverlayCard()->GetPresentation();
				const bool Z = Id.EndsWith(TEXT("MartinZubimendi"));
				Test->TestEqual(TEXT("Affected Full Card skill count"), Full.Skills.Num(), Z ? 1 : 2);
				if (Z && Full.Skills.Num() == 1)
					Test->TestTrue(TEXT("Zubimendi Full Card shows ThroughBall 7-8"), Full.Skills[0].SkillId == TEXT("Canonical.Skill.ThroughBall.7.8")
						&& Full.Skills[0].MinTriggerActionPoint == 7 && Full.Skills[0].MaxTriggerActionPoint == 8);
				CheckCard(Full); ++Inspected;
				Card->OnDetailHoverDismissed.Broadcast(Card);
			}
		}
		S.OpenDeploymentTacticalReference();
		auto* Selector = Cast<UHorizontalBox>(S.GetWidgetFromName(TEXT("DeploymentTacticalReferenceSelector")));
		Test->TestTrue(TEXT("Live deployment reference shows exactly four tactics"), S.IsDeploymentTacticalReferenceOpen() && Selector && Selector->GetChildrenCount() == 7);
		Test->TestNull(TEXT("No production PassControl reference entry"), S.GetWidgetFromName(TEXT("DeploymentReferencePassControlButton")));
		S.CloseDeploymentTacticalReference();
		return Test->TestEqual(TEXT("Case A and Zubimendi Full Cards inspected"), Inspected, 2);
	}
	FAutomationTestBase* Test;
	double Started = FPlatformTime::Seconds(), Changed = 0;
	int32 Step = 0, Possession = 0;
	FName Carrier;
	bool bSawTerminal = false, bSelectedThroughBall = false, bSawNoOption = false;
};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPassControlWithdrawalPIETest,
	"FMCodex.PIE.PassControlWithdrawal.Production", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPassControlWithdrawalPIETest::RunTest(const FString&)
{
	FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShareable(new FStartWithdrawalPIE()));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(2.f));
	FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShareable(new FWithdrawalPIE(this)));
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	return true;
}
#endif
