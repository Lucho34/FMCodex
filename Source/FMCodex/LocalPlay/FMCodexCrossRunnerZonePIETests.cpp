#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && !UE_BUILD_SHIPPING
#include "FMCodexLocalMatchPlayerController.h"
#include "FMCodexLocalMatchScreenWidget.h"
#include "FMCodexPrototypeTeamContent.h"
#include "Editor.h"
#include "Engine/World.h"
#include "Misc/AutomationTest.h"
#include "PlayInEditorDataTypes.h"
#include "Settings/LevelEditorPlaySettings.h"
#include "Tests/AutomationCommon.h"
#include "Tests/AutomationEditorCommon.h"

namespace
{
class FStartCrossRunnerZonePIE final : public IAutomationLatentCommand
{
public:
	bool Update() override
	{
		auto* Settings = DuplicateObject<ULevelEditorPlaySettings>(GetDefault<ULevelEditorPlaySettings>(), GetTransientPackage());
		Settings->NewWindowWidth = 1600; Settings->NewWindowHeight = 900;
		Settings->SetPlayNetMode(EPlayNetMode::PIE_Standalone); Settings->SetPlayNumberOfClients(1);
		FRequestPlaySessionParams Params; Params.EditorPlaySettings = Settings;
		GEditor->RequestPlaySession(Params); return true;
	}
};

// Real Local PIE, canonical Prototype40_v2, Screen typed actions and the existing
// DEV provider. No gameplay-state edits, fake candidates or reveal-clock jumps.
class FCrossRunnerZonePIE final : public IAutomationLatentCommand
{
public:
	explicit FCrossRunnerZonePIE(FAutomationTestBase* InTest) : Test(InTest) {}
	bool Update() override
	{
		if (FPlatformTime::Seconds() - Started > 120)
		{ Test->AddError(TEXT("Cross runner zone PIE timed out")); return true; }
		if (!GEditor || !GEditor->PlayWorld) return false;
		auto* C = Cast<AFMCodexLocalMatchPlayerController>(GEditor->PlayWorld->GetFirstPlayerController());
		auto* S = C ? C->GetPlayerMatchScreen() : nullptr;
		if (!S || FPlatformTime::Seconds() - Changed < .6) return false;
		if (Step == 0)
		{
			Test->TestEqual(TEXT("Committed four-tactic production content"), FFMCodexPrototypeTeamContent::GetBalanceContentVersion(), FString(TEXT("Prototype40_v2")));
			S->RequestStartNewMatch(); ++Step; Changed = FPlatformTime::Seconds(); return false;
		}
		if (S->IsInlineFormulaRevealInputBlocked()) return false;
		using Category = EFMCodexLocalMatchInteractionCategory;
		const auto& V = C->GetInteractionView();
		if (Step == 1)
		{
			if (!Override(*C, EFMCodexLocalDevRollTarget::FullD12, 4)) return true;
			S->RequestRollTacticalPoints(); ++Step; Changed = FPlatformTime::Seconds(); return false;
		}
		if (Step == 2)
		{
			if (!Test->TestEqual(TEXT("Natural TP4 reveal reaches deployment"), V.InteractionCategory, Category::Deploy)) return true;
			const bool IsA = V.CurrentAttackingPlayer == EInitialTurnOrderPlayer::PlayerA;
			Carrier = IsA ? TEXT("Prototype.Arsenal.BukayoSaka") : TEXT("Prototype.ManchesterCity.RayanAitNouri");
			Runner = IsA ? TEXT("Prototype.Arsenal.ChristianNorgaard") : TEXT("Prototype.ManchesterCity.Rodri");
			const auto* Definition = FFMCodexPrototypeTeamContent::Find(Runner);
			if (!Test->TestNotNull(TEXT("Runner exists in production content"), Definition)) return true;
			if (!Test->TestFalse(TEXT("Production runner has no authored A type"), Definition->Card.PositionTypes.Contains(EPlayerPositionType::Attack))) return true;
			const FString Own = IsA ? TEXT("NearA") : TEXT("NearB");
			const FString Forward = IsA ? TEXT("NearB") : TEXT("NearA");
			if (!Deploy(*C, *S, Own, Carrier) || !Deploy(*C, *S, Own)
				|| !Deploy(*C, *S, Forward, Runner) || !Deploy(*C, *S, Forward)) return true;
			S->RequestFinishDeployment();
			if (!Accepted(*C)) return true;
			if (!Deploy(*C, *S, Own) || !Deploy(*C, *S, Own)
				|| !Deploy(*C, *S, Forward) || !Deploy(*C, *S, Forward)) return true;
			S->RequestFinishDeployment();
			if (!Accepted(*C)) return true;
			Test->AddInfo(FString::Printf(TEXT("CROSS_ZONE_PIE world=%s TP=4 Carrier=%s Runner=%s authoredNonA=1 Forward=%s"),
				*GEditor->PlayWorld->GetPathName(), *Carrier.ToString(), *Runner.ToString(), *Forward));
			++Step; Changed = FPlatformTime::Seconds(); return false;
		}
		switch (V.InteractionCategory)
		{
		case Category::SelectCarrier: S->RequestSubmitCarrier(Carrier); break;
		case Category::SelectMarker:
			if (!Test->TestTrue(TEXT("A canonical Marker exists"), !V.SelectionOptions.IsEmpty())) return true;
			S->RequestSubmitMarker(V.SelectionOptions[0].Id); break;
		case Category::SelectRunner:
			if (!Test->TestTrue(TEXT("Non-A Forward runner appears in canonical selection"),
				V.SelectionOptions.ContainsByPredicate([&](const auto& O) { return O.Id == Runner; }))) return true;
			S->RequestSubmitRunner(Runner); bSelectedRunner = true; break;
		case Category::SelectHelper:
			if (V.bCanResolveNoLegalChoice) S->RequestResolveNoLegalSelection();
			else S->RequestDeclineSelection();
			break;
		case Category::SelectSkill:
		{
			const auto* Cross = V.SelectionOptions.FindByPredicate([](const auto& O) { return O.SkillType == ESkillRuleType::Cross; });
			if (!Test->TestNotNull(TEXT("Canonical TP4 Cross available with non-A Forward runner"), Cross)) return true;
			const FName SkillId = Cross->Id; S->RequestSubmitSkill(SkillId); bSelectedCross = true; break;
		}
		case Category::SelectBranchIntent:
			S->RequestSubmitBranchIntent(EFMCodexUMGBranchIntent::CrossHigh); break;
		case Category::RollCrossRoute:
			if (!Override(*C, EFMCodexLocalDevRollTarget::CrossRoute, 2)) return true;
			S->RequestContinueResolution(); break;
		case Category::RollCrossAttack:
			if (!Override(*C, EFMCodexLocalDevRollTarget::CrossHighAttack, 4)) return true;
			S->RequestContinueResolution(); break;
		case Category::RollCrossDefense:
			if (!Override(*C, EFMCodexLocalDevRollTarget::CrossHighDefense, 3)) return true;
			S->RequestContinueResolution(); break;
		case Category::AdvanceAfterTerminal:
			Test->TestTrue(TEXT("Original Cross flow reaches terminal after canonical selection"), bSelectedRunner && bSelectedCross && V.bTerminalPendingAdvance);
			Test->AddInfo(FString::Printf(TEXT("CROSS_ZONE_PIE PASS Runner=%s canonicalRunner=1 CrossAvailable=1 CrossTerminal=1 game=%.3f"),
				*Runner.ToString(), GEditor->PlayWorld->GetTimeSeconds()));
			return true;
		default:
			Test->AddError(FString::Printf(TEXT("Unexpected Cross zone PIE category %d"), static_cast<int32>(V.InteractionCategory))); return true;
		}
		if (!Accepted(*C)) return true;
		Changed = FPlatformTime::Seconds(); return false;
	}
private:
	bool Accepted(AFMCodexLocalMatchPlayerController& C)
	{
		return Test->TestTrue(*FString::Printf(TEXT("Screen typed action accepted: %s (%s)"),
			*C.GetLastDiagnostic().CommandName, *C.GetLastDiagnostic().Message), C.GetLastDiagnostic().bHostSuccess);
	}
	bool Override(AFMCodexLocalMatchPlayerController& C, EFMCodexLocalDevRollTarget Target, int32 Value)
	{
		FFMCodexLocalDevRollOverrideRequest R; R.Target = Target; R.Value = Value;
		return Test->TestTrue(TEXT("DEV provider accepts dice override"), C.SetLocalDevRollOverride(R).bSuccess);
	}
	bool Deploy(AFMCodexLocalMatchPlayerController& C, UFMCodexLocalMatchScreenWidget& S, const FString& Half, FName Card = NAME_None)
	{
		const auto* O = C.GetInteractionView().DeploymentOptions.FindByPredicate([&](const auto& Option)
		{
			return !Option.bGoalkeeper && (Card.IsNone() ? Option.CardId != Runner && Option.CardId != Carrier : Option.CardId == Card)
				&& Option.SlotId.ToString().Contains(Half);
		});
		if (!Test->TestNotNull(TEXT("Canonical deployment option"), O)) return false;
		const FName CardId = O->CardId, SlotId = O->SlotId;
		S.RequestDeployOrdinary(CardId, SlotId); return Accepted(C);
	}
	FAutomationTestBase* Test;
	double Started = FPlatformTime::Seconds(), Changed = 0;
	int32 Step = 0;
	FName Carrier, Runner;
	bool bSelectedRunner = false, bSelectedCross = false;
};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCrossRunnerZoneProductionPIETest,
	"FMCodex.PIE.CrossRunnerZone.Production", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FCrossRunnerZoneProductionPIETest::RunTest(const FString&)
{
	FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShareable(new FStartCrossRunnerZonePIE()));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(2.f));
	FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShareable(new FCrossRunnerZonePIE(this)));
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	return true;
}
#endif
