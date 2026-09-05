#include "FMCodexNetworkMatchRuntime.h"
#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "../LocalPlay/FMCodexLocalMatchInteractionView.h"
#include "../MatchPlayRuntime/MatchPlayAuthoritativeSession.h"

// Server-only canonical setup. Skill, Branch and Route remain genuine subsequent player inputs.
bool FFMCodexNetworkMatchRuntime::PrepareInitialRouteMilestone(ESkillRuleType Family)
{
	if (!bInitialized || AuthoritativeSession->GetStateSnapshot().bHasCurrentAttack
		|| (Family != ESkillRuleType::Cross && Family != ESkillRuleType::PassControl && Family != ESkillRuleType::ThroughBall)) { return false; }
	using Side = EInitialTurnOrderPlayer;
	using Command = EMatchPlayAuthoritativeCommandKind;
	auto View = [&](Side Player) { return BuildClientView(Player, 0, EFMCodexNetworkBootstrapState::MatchReady); };
	const auto Initial = View(Side::PlayerA);
	const Side Attack = Initial.CurrentAttackingSide;
	const Side Defense = Attack == Side::PlayerA ? Side::PlayerB : Side::PlayerA;
	const bool IsA = Attack == Side::PlayerA;
	auto Submit = [&](Command Kind, const auto& Request)
	{
		const auto Result = SubmitPlayerIntent(FMatchPlayPlayerIntent::Create(Kind, Request));
		if (!Result.bSuccess) { UE_LOG(LogFMCodexNetworkPlay, Error, TEXT("InitialRoute milestone canonical setup rejected: %s"), *Result.ErrorMessage); }
		return Result.bSuccess;
	};
	FMatchPlayFullD12EntryRequest Entry; Entry.ExpectedAttackSequence = Initial.AttackSequence; Entry.RequestingSide = Attack;
	if (!Submit(Command::RequestInitialActionPointRoll, Entry)) { return false; }
	const int64 Sequence = View(Attack).AttackSequence;
	const FName Carrier = FName(IsA
		? (Family == ESkillRuleType::Cross ? TEXT("Prototype.Arsenal.BukayoSaka") : TEXT("Prototype.Arsenal.MartinOdegaard"))
		: (Family == ESkillRuleType::Cross ? TEXT("Prototype.ManchesterCity.JeremyDoku") : TEXT("Prototype.ManchesterCity.Rodri")));
	const FName Runner = FName(IsA
		? (Family == ESkillRuleType::Cross ? TEXT("Prototype.Arsenal.KaiHavertz") : TEXT("Prototype.Arsenal.MylesLewisSkelly"))
		: (Family == ESkillRuleType::Cross ? TEXT("Prototype.ManchesterCity.ErlingHaaland") : TEXT("Prototype.ManchesterCity.RayanAitNouri")));
	const FString Half = IsA ? TEXT("NearA") : TEXT("NearB");
	const FString Other = IsA ? TEXT("NearB") : TEXT("NearA");
	auto Deploy = [&](Side Player, const FString& RequiredHalf, FName RequiredCard = NAME_None)
	{
		FFMCodexLocalMatchViewerDisclosure Disclosure; Disclosure.bRevealInitialActionPointRoll = true;
		const auto Safe = FFMCodexLocalMatchInteractionViewBuilder::BuildForViewer(
			AuthoritativeSession->GetStateSnapshot(), SkillRuleSet, Player, Disclosure);
		for (const auto& Option : Safe.DeploymentOptions)
		{
			if (Option.bGoalkeeper || !Option.SlotId.ToString().Contains(RequiredHalf)
				|| (!RequiredCard.IsNone() && RequiredCard != Option.CardId)) { continue; }
			FMatchPlayAuthoritativeDeployOrdinaryRequest Request;
			Request.ExpectedAttackSequence = Sequence; Request.RequestingSide = Player;
			Request.CardId = Option.CardId; Request.SlotId = Option.SlotId;
			return Submit(Command::DeployOrdinary, Request);
		}
		return false;
	};
	auto Finish = [&](Side Player)
	{
		FMatchPlayFinishDeploymentIntent Request; Request.AttackSequence = Sequence; Request.RequestingSide = Player;
		return Submit(Command::FinishDeployment, Request);
	};
	if (!Deploy(Attack, Half, Carrier) || !Deploy(Defense, Half) || !Deploy(Attack, Other, Runner)
		|| !Deploy(Defense, Other) || !Finish(Attack) || !Deploy(Defense, Other) || !Finish(Defense)) { return false; }
	FMatchPlayAuthoritativeSubmitCarrierRequest C;
	C.ExpectedAttackSequence = Sequence; C.RequestingSide = Attack; C.CarrierCardId = Carrier;
	if (!Submit(Command::SubmitCarrier, C)) { return false; }
	const auto MarkerView = View(Defense);
	if (MarkerView.MarkerOptions.IsEmpty()) { return false; }
	FMatchPlayAuthoritativeSubmitMarkerRequest M;
	M.ExpectedAttackSequence = Sequence; M.RequestingSide = Defense; M.MarkerCardId = MarkerView.MarkerOptions[0].Choice.MarkerCardId;
	if (!Submit(Command::SubmitMarker, M)) { return false; }
	FMatchPlayAuthoritativeSubmitRunnerRequest R;
	R.ExpectedAttackSequence = Sequence; R.RequestingSide = Attack; R.RunnerCardId = Runner;
	if (!Submit(Command::SubmitRunner, R)) { return false; }
	const auto HelperView = View(Defense);
	if (HelperView.HelperOptions.IsEmpty()) { return false; }
	FMatchPlayAuthoritativeSubmitHelperRequest H;
	H.ExpectedAttackSequence = Sequence; H.RequestingSide = Defense; H.HelperCardId = HelperView.HelperOptions[0].Choice.HelperCardId;
	if (!Submit(Command::SubmitHelper, H)) { return false; }
	FFMCodexLocalMatchViewerDisclosure Disclosure; Disclosure.bRevealInitialActionPointRoll = true;
	const auto Safe = FFMCodexLocalMatchInteractionViewBuilder::BuildForViewer(
		AuthoritativeSession->GetStateSnapshot(), SkillRuleSet, Attack, Disclosure);
	const bool Ready = View(Attack).EntryWait == EFMCodexNetworkEntryWait::SkillSelection
		&& Safe.SelectionOptions.ContainsByPredicate([&](const auto& O) { return O.SkillType == Family; });
	UE_LOG(LogFMCodexNetworkPlay, Log, TEXT("InitialRoute milestone: Ready=%d Attacker=%d Family=%d Sequence=%lld; Skill, Branch and Route remain player inputs."),
		Ready, static_cast<int32>(Attack), static_cast<int32>(Family), Sequence);
	return Ready;
}
#endif
