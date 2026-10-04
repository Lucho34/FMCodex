#include "FMCodexGuidedLesson1.h"
#if !UE_BUILD_SHIPPING
#define LOCTEXT_NAMESPACE "FMCodexGuidedLesson1"

using StepType = EFMCodexLesson1Step;
using Gesture = EFMCodexMatchScreenIntent;
using Category = EFMCodexLocalMatchInteractionCategory;

FName FFMCodexGuidedLesson1::Gyokeres() { return TEXT("Prototype.Arsenal.ViktorGyokeres"); }
FName FFMCodexGuidedLesson1::Odegaard() { return TEXT("Prototype.Arsenal.MartinOdegaard"); }
FName FFMCodexGuidedLesson1::Stones() { return TEXT("Prototype.ManchesterCity.JohnStones"); }
FName FFMCodexGuidedLesson1::AttackerSlot() { return TEXT("Demo.Slot.NearB.03"); }
FName FFMCodexGuidedLesson1::DefenderSlot() { return TEXT("Demo.Slot.NearB.04"); }
FName FFMCodexGuidedLesson1::Skill() { return TEXT("Canonical.Skill.LongShot.3.5"); }

void FFMCodexGuidedLesson1::SetStep(StepType NewStep)
{
	if (Step == NewStep) return;
	Step = NewStep; StepSeconds = 0; HintLevel = 0; Feedback = FText();
	UE_LOG(LogTemp, Display, TEXT("LESSON1 attempt=%d step=%d"), bComparison ? 2 : 1, static_cast<int32>(Step));
}

bool FFMCodexGuidedLesson1::Update(const FFMCodexLocalMatchInteractionView& V, bool bRevealReady, float DeltaSeconds)
{
	const auto Previous = Step;
	const int32 PreviousHint = HintLevel;
	StepSeconds += FMath::Max(0.f, DeltaSeconds);
	FeedbackSeconds = FMath::Max(0.f, FeedbackSeconds - DeltaSeconds);
	if (FeedbackSeconds == 0.f) Feedback = FText();
	if (IsExplanationMode() || Step == StepType::InspectGyokeres || Step == StepType::RewindTransition) return false;
	if (!bRevealReady)
	{
		if (V.bTerminalPendingAdvance) SetStep(StepType::ResultReveal);
		return Previous != Step;
	}
	if (V.bTerminalPendingAdvance)
	{
		for (const auto& Contest : V.ResolutionFacts.FormulaContests)
		{
			if (!Contest.bHasResolvedFormula) continue;
			ResolvedComparison = FText::Format(LOCTEXT("Totals", "进攻 {0}，防守 {1}。"),
				FText::AsNumber(Contest.AttackRow.FinalValue), FText::AsNumber(Contest.DefenseRow.FinalValue));
			bResolvedGoal = Contest.ResolvedResult.Winner == EFormulaWinner::Attacker;
			SetStep(bComparison ? StepType::Summary : StepType::Rewind);
			break;
		}
	}
	else switch (V.InteractionCategory)
	{
	case Category::TacticalPointRoll: SetStep(StepType::TacticPoint); break;
	case Category::Deploy:
	{
		bool bAttackerPlaced = false;
		for (const auto& Region : V.PitchRegions)
			for (const auto& Slot : Region.Slots)
				bAttackerPlaced |= Slot.bOccupied && Slot.Card.CardId == Attacker();
		if (V.ExpectedActingPlayer == EInitialTurnOrderPlayer::PlayerB)
			SetStep(V.bPlayerADeploymentFinished ? StepType::OpponentFinish : StepType::OpponentDeploy);
		else if (bAttackerPlaced)
		{
			if (Step != StepType::FinishDeployment)
				SetStep(bComparison ? StepType::FinishDeployment : StepType::FinishExplanation);
		}
		else if (!bComparison && Step == StepType::TacticPoint) SetStep(StepType::TacticPointExplanation);
		else SetStep(StepType::Deploy);
		break;
	}
	case Category::SelectCarrier:
		if (Step != StepType::Carrier) SetStep(bComparison ? StepType::Carrier : StepType::CarrierExplanation); break;
	case Category::SelectMarker: SetStep(StepType::OpponentMarker); break;
	case Category::SelectSkill:
		if (Step != StepType::Skill) SetStep(bComparison ? StepType::Skill : StepType::SkillExplanation); break;
	case Category::SelectLongShotBranch:
	case Category::SelectBranchIntent:
		if (Step != StepType::DirectShot) SetStep(bComparison ? StepType::DirectShot : StepType::DirectExplanation); break;
	case Category::RollLongShotDirectAttack: SetStep(StepType::AttackRoll); break;
	case Category::RollLongShotDirectDefense: SetStep(StepType::OpponentDefense); break;
	default: break;
	}
	if (bComparison && Step == StepType::Deploy)
		HintLevel = StepSeconds >= 25.f ? 2 : StepSeconds >= 12.f ? 1 : 0;
	return Previous != Step || PreviousHint != HintLevel;
}

void FFMCodexGuidedLesson1::Primary()
{
	if (Step == StepType::Intro) SetStep(StepType::TacticPoint);
	else if (Step == StepType::TacticPointExplanation) SetStep(StepType::InspectGyokeres);
	else if (Step == StepType::SkillRangeExplanation) SetStep(StepType::Deploy);
	else if (Step == StepType::FinishExplanation) SetStep(StepType::FinishDeployment);
	else if (Step == StepType::CarrierExplanation) SetStep(StepType::Carrier);
	else if (Step == StepType::SkillExplanation) SetStep(StepType::Skill);
	else if (Step == StepType::DirectExplanation) SetStep(StepType::DirectShot);
	else if (Step == StepType::Rewind) SetStep(StepType::RewindTransition);
	else if (Step == StepType::Summary) SetStep(StepType::Complete);
}
void FFMCodexGuidedLesson1::EnterComparison()
{
	bComparison = true; bDispatchingOpponent = false; ResolvedComparison = FText();
	SetStep(StepType::Deploy);
}
bool FFMCodexGuidedLesson1::IsCheckpointDue() const { return Step == StepType::RewindTransition && StepSeconds >= .7f; }
bool FFMCodexGuidedLesson1::IsOpponentActionDue() const
{
	return StepSeconds >= (bComparison ? .6f : 1.6f) &&
		(Step == StepType::OpponentDeploy || Step == StepType::OpponentFinish
			|| Step == StepType::OpponentMarker || Step == StepType::OpponentDefense);
}
FFMCodexMatchScreenRequest FFMCodexGuidedLesson1::OpponentAction() const
{
	FFMCodexMatchScreenRequest R;
	if (Step == StepType::OpponentDeploy) { R.Kind = Gesture::DeployOrdinary; R.OptionId = Stones(); R.SlotId = DefenderSlot(); }
	else if (Step == StepType::OpponentFinish) R.Kind = Gesture::FinishDeployment;
	else if (Step == StepType::OpponentMarker) { R.Kind = Gesture::Marker; R.OptionId = Stones(); }
	else { R.Kind = Gesture::Continue; R.Category = EFMCodexUMGInteractionCategory::RollLongShotDirectDefense; }
	return R;
}

bool FFMCodexGuidedLesson1::AllowsScreen(const FFMCodexMatchScreenRequest& R) const
{
	if (bDispatchingOpponent)
	{
		const auto Expected = OpponentAction();
		return IsOpponentActionDue() && R.Kind == Expected.Kind && R.OptionId == Expected.OptionId
			&& R.SlotId == Expected.SlotId
			&& (R.Kind != Gesture::Continue || R.Category == Expected.Category);
	}
	switch (Step)
	{
	case StepType::TacticPoint: return R.Kind == Gesture::TacticalPoints;
	case StepType::Deploy: return R.Kind == Gesture::DeployOrdinary && R.OptionId == Attacker() && R.SlotId == AttackerSlot();
	case StepType::FinishDeployment: return R.Kind == Gesture::FinishDeployment;
	case StepType::Carrier: return R.Kind == Gesture::Carrier && R.OptionId == Attacker();
	case StepType::Skill: return R.Kind == Gesture::Skill && R.OptionId == Skill();
	case StepType::DirectShot: return R.Kind == Gesture::Branch && R.Branch == EFMCodexUMGBranchIntent::DirectShot;
	case StepType::AttackRoll: return R.Kind == Gesture::Continue && R.Category == EFMCodexUMGInteractionCategory::RollLongShotDirectAttack;
	default: return false;
	}
}

bool FFMCodexGuidedLesson1::AllowsIntent(const FMatchPlayPlayerIntent& I) const
{
	FFMCodexMatchScreenRequest R;
	EInitialTurnOrderPlayer Side = EInitialTurnOrderPlayer::None;
#define LESSON_PAYLOAD(Type, Body) if (!I.Payload.IsType<Type>()) return false; { const auto& P = I.Payload.Get<Type>(); Side = P.RequestingSide; Body; } break
	switch (I.CommandKind)
	{
	case EMatchPlayAuthoritativeCommandKind::RequestInitialActionPointRoll:
		LESSON_PAYLOAD(FMatchPlayFullD12EntryRequest, R.Kind = Gesture::TacticalPoints);
	case EMatchPlayAuthoritativeCommandKind::DeployOrdinary:
		LESSON_PAYLOAD(FMatchPlayAuthoritativeDeployOrdinaryRequest, R.Kind = Gesture::DeployOrdinary; R.OptionId = P.CardId; R.SlotId = P.SlotId);
	case EMatchPlayAuthoritativeCommandKind::FinishDeployment:
		LESSON_PAYLOAD(FMatchPlayFinishDeploymentIntent, R.Kind = Gesture::FinishDeployment);
	case EMatchPlayAuthoritativeCommandKind::SubmitCarrier:
		LESSON_PAYLOAD(FMatchPlayAuthoritativeSubmitCarrierRequest, R.Kind = Gesture::Carrier; R.OptionId = P.CarrierCardId);
	case EMatchPlayAuthoritativeCommandKind::SubmitMarker:
		LESSON_PAYLOAD(FMatchPlayAuthoritativeSubmitMarkerRequest, R.Kind = Gesture::Marker; R.OptionId = P.MarkerCardId);
	case EMatchPlayAuthoritativeCommandKind::SubmitSkill:
		LESSON_PAYLOAD(FMatchPlayAuthoritativeSubmitSkillRequest, R.Kind = Gesture::Skill; R.OptionId = P.SkillId);
	case EMatchPlayAuthoritativeCommandKind::SubmitBranchIntent:
		LESSON_PAYLOAD(FMatchPlayAuthoritativeSubmitBranchIntentRequest, R.Kind = Gesture::Branch; R.Branch = P.Intent == EMatchPlayElectiveBranchIntent::DirectShot ? EFMCodexUMGBranchIntent::DirectShot : EFMCodexUMGBranchIntent::None);
	case EMatchPlayAuthoritativeCommandKind::ResolveLongShotDirectAttackRoll:
		LESSON_PAYLOAD(FMatchPlayAuthoritativeResolveLongShotDirectAttackRollRequest, R.Kind = Gesture::Continue; R.Category = EFMCodexUMGInteractionCategory::RollLongShotDirectAttack);
	case EMatchPlayAuthoritativeCommandKind::ResolveLongShotDirectDefenseRoll:
		LESSON_PAYLOAD(FMatchPlayAuthoritativeResolveLongShotDirectDefenseRollRequest, R.Kind = Gesture::Continue; R.Category = EFMCodexUMGInteractionCategory::RollLongShotDirectDefense);
	default: return false;
	}
#undef LESSON_PAYLOAD
	return Side == (bDispatchingOpponent ? EInitialTurnOrderPlayer::PlayerB : EInitialTurnOrderPlayer::PlayerA) && AllowsScreen(R);
}

void FFMCodexGuidedLesson1::ExplainUnavailable(const FFMCodexMatchScreenRequest& R)
{
	FeedbackSeconds = 6.f;
	Feedback = bComparison && Step == StepType::Deploy && R.OptionId == Gyokeres()
		? LOCTEXT("CompareAgain", "哲凯赖什的射门也是 4，但没有远射加成。再看看另一名球员的特性。")
		: LOCTEXT("Unavailable", "本课暂不练习这项操作，请按当前提示继续。");
}

void FFMCodexGuidedLesson1::ApplyPresentation(FFMCodexUMGMatchScreenViewModel& P) const
{
	P.LocalRack.Cells.RemoveAll([&](const auto& C) { return C.Card.CardId != Gyokeres() && (!bComparison || C.Card.CardId != Odegaard()); });
	P.OpponentRack.Cells.RemoveAll([&](const auto& C) { return C.Card.CardId != Stones(); });
	for (auto& C : P.LocalRack.Cells) C.bDeploymentDraggable &= Step == StepType::Deploy;
	for (auto& C : P.OpponentRack.Cells) C.bDeploymentDraggable = false;
	P.LocalRack.SideLabel = TEXT("阿森纳 · 本课选择");
	P.OpponentRack.SideLabel = TEXT("曼城 · 教学对手");
	P.Interaction.bCanStartNewMatch = false;
	P.Interaction.bCanRollTacticalPoints &= Step == StepType::TacticPoint;
	P.Interaction.bCanFinishDeployment &= Step == StepType::FinishDeployment;
	P.Interaction.bCanDecline = false;
	P.Interaction.bCanResolveNoLegal = false;
	P.Interaction.DeploymentChoices.RemoveAll([&](auto& C)
	{
		if (Step != StepType::Deploy || (C.CardId != Gyokeres() && (!bComparison || C.CardId != Odegaard()))) return true;
		C.Destinations.RemoveAll([](const auto& D) { return D.SlotId != AttackerSlot(); });
		return C.Destinations.IsEmpty();
	});
	for (auto& C : P.Interaction.SelectionChoices)
		if (Step != StepType::Skill || C.OptionId != Skill()) { C.bEnabled = false; C.SecondaryLabel = TEXT("本课暂不练习"); }
	// The existing theater retains the absent method as a disabled option.
	P.Interaction.BranchChoices.RemoveAll([](const auto& C) { return C.Intent != EFMCodexUMGBranchIntent::DirectShot; });
	P.LongShotResolution.BranchChoices.RemoveAll([](const auto& C) { return C.Intent != EFMCodexUMGBranchIntent::DirectShot; });
	for (auto& Region : P.PitchRegions)
		for (auto& Slot : Region.Slots)
		{
			Slot.bSelectableForCurrentPrompt &= Step == StepType::Carrier && Slot.Card.CardId == Attacker();
			if (Step == StepType::Deploy && Slot.SlotId == AttackerSlot())
			{
				if (Step == StepType::Deploy) Slot.SlotLabel = TEXT("部署到此处");
				Slot.DeploymentTargetState = EFMCodexUMGDeploymentTargetState::Valid;
			}
		}
	// The lesson owns terminal progression; keep all production result facts/timers.
	const bool bPlayerRoll = Step == StepType::AttackRoll;
	P.Interaction.PrimaryAction.bAvailable &= bPlayerRoll;
	P.Interaction.bCanContinue &= bPlayerRoll;
	P.LongShotResolution.PrimaryAction.Action.bAvailable &= bPlayerRoll;
	P.LongShotResolution.bCanContinue &= bPlayerRoll;
	P.InlineFormula.PrimaryAction.Action.bAvailable &= bPlayerRoll;
	P.InlineFormula.bCanContinue &= bPlayerRoll;
	P.LongShotResolution.Formula.PrimaryAction.Action.bAvailable &= bPlayerRoll;
	P.LongShotResolution.Formula.bCanContinue &= bPlayerRoll;
	P.Resolution.bCanContinue = false;
}

FText FFMCodexGuidedLesson1::ProgressLabel() const
{
	return bComparison ? LOCTEXT("Second", "第一课 · 换个人试试") : LOCTEXT("First", "第一课 · 第一次进攻");
}
FText FFMCodexGuidedLesson1::Instruction() const
{
	if (!Feedback.IsEmpty()) return Feedback;
	switch (Step)
	{
	case StepType::Intro: return LOCTEXT("IntroLongShot", "先掷出本回合的进攻战术点。");
	case StepType::TacticPointExplanation: return LOCTEXT("TPInspect", "本回合进攻战术点为 3。悬停哲凯赖什，查看技能。");
	case StepType::InspectGyokeres: return LOCTEXT("Inspect", "悬停哲凯赖什，查看技能");
	case StepType::SkillRangeExplanation: return LOCTEXT("SkillRange", "“远射 3–5”表示，当进攻战术点为 3、4 或 5 时，可以发动远射。");
	case StepType::FinishExplanation: return LOCTEXT("FinishConcept", "斯通斯已上场。此次远射只需一名进攻球员，可以结束部署。");
	case StepType::CarrierExplanation: return LOCTEXT("CarrierConcept", "点击场上的哲凯赖什，由他担任本回合持球队员。");
	case StepType::SkillExplanation: return LOCTEXT("SkillConcept", "哲凯赖什当前可用的进攻技能是远射。接下来点击左下角“远射”。");
	case StepType::DirectExplanation: return LOCTEXT("DirectConcept", "在下方查看说明，再选择“直接射门”。");
	case StepType::TacticPoint: return LOCTEXT("TP", "掷出本回合的进攻战术点。");
	case StepType::Deploy:
		if (!bComparison) return LOCTEXT("DeployFirst", "将哲凯赖什拖到高亮位置");
		if (HintLevel == 2) return LOCTEXT("Hint2", "“远射专家”可以提高直接远射时的射门能力。");
		if (HintLevel == 1) return LOCTEXT("Hint1", "打开球员卡，看看“特性”。");
		return LOCTEXT("Compare", "两人的射门都是 4。选择更适合远射的球员。");
	case StepType::OpponentDeploy: return LOCTEXT("OpponentDeploy", "现在轮到对手部署。");
	case StepType::FinishDeployment: return bComparison ? LOCTEXT("FinishShort", "球员已就位，点击“部署完毕”。") : LOCTEXT("Finish", "点击“部署完毕”，结束本次部署。");
	case StepType::OpponentFinish: return bComparison ? LOCTEXT("OpponentFinishShort", "对手正在结束部署。") : LOCTEXT("OpponentFinish", "你已结束部署，对手仍可继续部署。双方都结束后，开始选择进攻角色。");
	case StepType::Carrier: return bComparison ? LOCTEXT("CarrierSecond", "点击场上的厄德高，选择持球队员。") : LOCTEXT("CarrierFocus", "点击场上的哲凯赖什，选择持球队员。");
	case StepType::OpponentMarker: return LOCTEXT("Marker", "斯通斯负责盯防。");
	case StepType::Skill: return LOCTEXT("Skill", "点击左下角的“远射”");
	case StepType::DirectShot: return LOCTEXT("Direct", "选择“直接射门”");
	case StepType::AttackRoll: return LOCTEXT("AttackRoll", "点击进攻掷骰，完成远射判定");
	case StepType::OpponentDefense: return LOCTEXT("DefenseRoll", "对手正在进行防守判定。");
	case StepType::ResultReveal: return LOCTEXT("Reveal", "看看这次远射的结果。");
	case StepType::Rewind: return bResolvedGoal
		? FText::Format(LOCTEXT("UnexpectedGoal", "进球。{0}教学条件与预期不同，请退出教学并检查场景配置。"), ResolvedComparison)
		: FText::Format(LOCTEXT("Miss", "未能进球\n{0}这次远射未能突破防守。"), ResolvedComparison);
	case StepType::RewindTransition: return LOCTEXT("Replay", "教学重演");
	case StepType::Summary: return bResolvedGoal
		? FText::Format(LOCTEXT("Summary", "进球！{0}\n相同的战术和骰点，适合的球员改变了结果。\n先考虑战术，再选择适合它的球员。"), ResolvedComparison)
		: FText::Format(LOCTEXT("UnexpectedMiss", "未能进球。{0}教学条件与预期不同，请退出教学并检查场景配置。"), ResolvedComparison);
	case StepType::Complete: return LOCTEXT("Complete", "第一课已完成。你已体验如何选择适合战术的球员。");
	default: return FText();
	}
}
FText FFMCodexGuidedLesson1::Explanation() const
{
	if (Step == StepType::Rewind) return LOCTEXT("RewindWarning", "教学模式会回到进攻开始前，并保留相同条件用于比较。正式比赛中，已经完成的进攻不能撤销。");
	if (Step == StepType::Deploy && bComparison) return LOCTEXT("Unlock", "这一次，厄德高也可以上场。这是本课新增的选择；悬停卡牌查看特性。");
	if (Step == StepType::Summary) return LOCTEXT("Limits", "本次对照中，远射专家 A 提供了射门 +2。正式比赛中，结果仍会受到骰点和对手配置影响。");
	return FText();
}
FText FFMCodexGuidedLesson1::PrimaryLabel() const
{
	if (Step == StepType::Intro) return LOCTEXT("StartAction", "开始操作");
	if (IsExplanationMode() && Step != StepType::Rewind && Step != StepType::Summary && Step != StepType::Complete) return LOCTEXT("Continue", "继续");
	if (Step == StepType::Rewind) return LOCTEXT("RewindCTA", "让时间倒流，换个人试试");
	if (Step == StepType::Summary) return LOCTEXT("FinishCTA", "完成第一课");
	if (Step == StepType::Complete) return LOCTEXT("Return", "返回普通对局");
	return FText();
}
bool FFMCodexGuidedLesson1::IsExplanationMode() const
{
	switch (Step)
	{
	case StepType::Intro: case StepType::TacticPointExplanation: case StepType::SkillRangeExplanation:
	case StepType::FinishExplanation: case StepType::CarrierExplanation: case StepType::SkillExplanation:
	case StepType::DirectExplanation: case StepType::Rewind: case StepType::Summary: case StepType::Complete: return true;
	default: return false;
	}
}
EFMCodexLesson1Focus FFMCodexGuidedLesson1::FocusTarget() const
{
	using F = EFMCodexLesson1Focus;
	switch (Step)
	{
	case StepType::TacticPoint: return F::TacticPoint;
	case StepType::InspectGyokeres: return F::HandCard;
	case StepType::Deploy: return F::Deployment;
	case StepType::FinishDeployment: return F::FinishDeployment;
	case StepType::Carrier: return F::Carrier;
	case StepType::Skill: return F::Skill;
	case StepType::DirectShot: return F::DirectShot;
	case StepType::AttackRoll: return F::AttackRoll;
	default: return F::None;
	}
}
bool FFMCodexGuidedLesson1::InspectCard(FName CardId, bool bFullCardVisible)
{
	if (Step != StepType::InspectGyokeres || CardId != Gyokeres() || !bFullCardVisible) return false;
	SetStep(StepType::SkillRangeExplanation);
	return true;
}
bool FFMCodexGuidedLesson1::AllowsInspection(FName CardId) const
{
	if (Step == StepType::InspectGyokeres || Step == StepType::SkillRangeExplanation) return CardId == Gyokeres();
	if (Step == StepType::Deploy) return CardId == Gyokeres() || (bComparison && CardId == Odegaard());
	return Step == StepType::Carrier && CardId == Attacker();
}
#undef LOCTEXT_NAMESPACE
#endif
