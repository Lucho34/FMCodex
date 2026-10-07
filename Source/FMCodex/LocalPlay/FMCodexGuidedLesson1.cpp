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
	bOpponentMoveStarted = false; bOpponentMoveArrived = false;
	bExitConfirmation = false; bOpponentSubmitted = false; OpponentSettledSeconds = 0.f;
	Step = NewStep; StepSeconds = 0; HintLevel = 0; Feedback = FText();
	UE_LOG(LogTemp, Display, TEXT("LESSON1 attempt=%d step=%d"), bComparison ? 2 : 1, static_cast<int32>(Step));
}

bool FFMCodexGuidedLesson1::Update(const FFMCodexLocalMatchInteractionView& V, bool bRevealReady, float DeltaSeconds)
{
	if (bExitConfirmation) return false;
	if (bOpponentSubmitted && Step != StepType::OpponentDefense)
	{
		OpponentSettledSeconds += FMath::Max(0.f, DeltaSeconds);
		if (OpponentSettledSeconds < OpponentPostActionHold()) return false;
	}
	const auto Previous = Step;
	const int32 PreviousHint = HintLevel;
	StepSeconds += FMath::Max(0.f, DeltaSeconds);
	FeedbackSeconds = FMath::Max(0.f, FeedbackSeconds - DeltaSeconds);
	if (FeedbackSeconds == 0.f) Feedback = FText();
	if (Step == StepType::FormulaHover || Step == StepType::FormulaExplanation) return false;
	if (IsExplanationMode() || Step == StepType::InspectGyokeres || Step == StepType::InspectOdegaard || Step == StepType::RewindTransition) return false;
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
			bResolvedGoal = Contest.ResolvedResult.Winner == EFormulaWinner::Attacker;
			if (!bComparison && !bResolvedGoal)
			{
				// Start only after the disclosed production result reached its ready boundary.
				if (Step != StepType::FailurePause) SetStep(StepType::FailurePause);
				else if (StepSeconds >= FailureFollowupDelay) SetStep(StepType::Rewind);
			}
			else SetStep(bComparison ? StepType::Summary : StepType::Rewind);
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
	case Category::RollLongShotDirectAttack: SetStep(!bComparison && !bFormulaInspected ? StepType::FormulaHover : StepType::AttackRoll); break;
	case Category::RollLongShotDirectDefense: SetStep(StepType::OpponentDefense); break;
	default: break;
	}
	if (bComparison && Step == StepType::Deploy)
		HintLevel = StepSeconds >= 25.f ? 2 : StepSeconds >= 12.f ? 1 : 0;
	return Previous != Step || PreviousHint != HintLevel;
}

void FFMCodexGuidedLesson1::Primary()
{
	if (bExitConfirmation) return;
	if (Step == StepType::Intro) SetStep(StepType::TacticPoint);
	else if (Step == StepType::TacticPointExplanation) SetStep(StepType::InspectGyokeres);
	else if (Step == StepType::SkillRangeExplanation) SetStep(bComparison ? StepType::ShootingExplanation : StepType::Deploy);
	else if (Step == StepType::ShootingExplanation) SetStep(StepType::TraitExplanation);
	else if (Step == StepType::TraitExplanation) SetStep(StepType::Deploy);
	else if (Step == StepType::FinishExplanation) SetStep(StepType::FinishDeployment);
	else if (Step == StepType::CarrierExplanation) SetStep(StepType::Carrier);
	else if (Step == StepType::SkillExplanation) SetStep(StepType::Skill);
	else if (Step == StepType::FormulaExplanation) SetStep(StepType::AttackRoll);
	else if (Step == StepType::DirectExplanation) SetStep(StepType::DirectShot);
	else if (Step == StepType::Rewind) SetStep(StepType::RewindTransition);
	else if (Step == StepType::Summary) SetStep(StepType::Complete);
}
void FFMCodexGuidedLesson1::EnterComparison()
{
	bComparison = true; bDispatchingOpponent = false; bResolvedGoal = false; bFormulaInspected = false;
	bExitConfirmation = false; bOpponentSubmitted = false; OpponentSettledSeconds = 0.f;
	FeedbackSeconds = 0; Feedback = FText();
	SetStep(StepType::InspectOdegaard);
}
bool FFMCodexGuidedLesson1::IsCheckpointDue() const { return Step == StepType::RewindTransition && StepSeconds >= .7f; }
bool FFMCodexGuidedLesson1::IsOpponentPresenting() const
{
	return Step == StepType::OpponentDeploy || Step == StepType::OpponentFinish
		|| Step == StepType::OpponentMarker || Step == StepType::OpponentDefense;
}
bool FFMCodexGuidedLesson1::IsOpponentActionDue() const
{
	if (!IsOpponentPresenting() || bOpponentSubmitted || bExitConfirmation) return false;
	if (Step == StepType::OpponentDeploy) return bOpponentMoveArrived;
	const float Lead = Step == StepType::OpponentDefense ? OpponentDefenseLead
		: OpponentPreAction + (Step == StepType::OpponentFinish ? OpponentFinishFocus : OpponentChoiceFocus);
	return StepSeconds >= Lead * OpponentPace();
}
float FFMCodexGuidedLesson1::OpponentPostActionHold() const
{
	// Repeated gestures are slightly quicker; preserve the full readable end hold.
	return Step==StepType::OpponentDeploy ? OpponentDestinationHold*OpponentPace()+OpponentDeploySettledHold : OpponentSettledHold;
}
bool FFMCodexGuidedLesson1::CanStartOpponentDeploymentMove() const
{
	return Step==StepType::OpponentDeploy && !bOpponentMoveStarted && !bOpponentSubmitted && !bExitConfirmation
		&& StepSeconds>=OpponentDeployAttention*OpponentPace();
}
void FFMCodexGuidedLesson1::BeginOpponentDeploymentMove()
{
	if (CanStartOpponentDeploymentMove()) bOpponentMoveStarted=true;
}
void FFMCodexGuidedLesson1::CompleteOpponentDeploymentMove()
{
	// The local Slate proxy reports visual arrival, never a gameplay result.
	if (IsOpponentDeploymentMoving() && !bExitConfirmation) bOpponentMoveArrived=true;
}
EFMCodexLesson1OpponentTarget FFMCodexGuidedLesson1::OpponentTarget() const
{
	using Target=EFMCodexLesson1OpponentTarget;
	if (!IsOpponentFocusVisible()) return Target::None;
	if (Step==StepType::OpponentFinish) return Target::SideStatus;
	if (Step==StepType::OpponentDeploy && !bOpponentSubmitted)
		return bOpponentMoveStarted ? Target::MovingCard : Target::HandCard;
	return Target::FieldCard;
}
bool FFMCodexGuidedLesson1::IsOpponentFinalHold() const
{
	return Step==StepType::OpponentDeploy && bOpponentSubmitted && OpponentSettledSeconds>=OpponentDestinationHold*OpponentPace();
}
bool FFMCodexGuidedLesson1::IsOpponentFocusVisible() const
{
	// Attention and selection keep the source visible; destination is the sole target after acceptance.
	return IsOpponentPresenting() && Step!=StepType::OpponentDefense && !IsOpponentFinalHold();
}
void FFMCodexGuidedLesson1::OpponentActionSubmitted()
{
	if (IsOpponentPresenting()) { bOpponentSubmitted = true; OpponentSettledSeconds = 0.f; }
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
	if (bExitConfirmation) return false;
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
	const bool bPlayerRoll = Step == StepType::AttackRoll
		&& P.Interaction.Category == EFMCodexUMGInteractionCategory::RollLongShotDirectAttack;
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

TArray<FText> FFMCodexGuidedLesson1::ConceptKeywords(EFMCodexLesson1CopySurface Surface) const
{
	using SurfaceType = EFMCodexLesson1CopySurface;
	// Copy/emphasis script: Docs/Tutorial/Guided_Match_Lesson_01_Copy_Script.md.
	// Blue labels and secondary copy never carry teaching emphasis.
	if (Surface==SurfaceType::Section || Surface==SurfaceType::Secondary) return {};
	const FText TP=LOCTEXT("ConceptTP","进攻战术点"), LongShot=LOCTEXT("ConceptLongShot","远射"),
		SkillWord=LOCTEXT("ConceptSkill","技能"), Shooting=LOCTEXT("ConceptShooting","射门"),
		Finish=LOCTEXT("ConceptFinish","结束部署"), Trait=LOCTEXT("ConceptTrait","特性"),
		Expert=LOCTEXT("ConceptExpert","远射专家 A"), Carrier=LOCTEXT("ConceptCarrier","持球队员"),
		Direct=LOCTEXT("ConceptDirect","直接射门"), Formula=LOCTEXT("ConceptFormula","公式"),
		Attribute=LOCTEXT("ConceptAttribute","属性值");
	if (Surface==SurfaceType::Heading)
	{
		switch (Step)
		{
		case StepType::Intro: case StepType::SkillRangeExplanation: case StepType::SkillExplanation: return {LongShot};
		case StepType::TacticPointExplanation: return {SkillWord};
		case StepType::ShootingExplanation: return {Shooting};
		case StepType::TraitExplanation: return {Expert};
		case StepType::CarrierExplanation: return {Carrier};
		case StepType::DirectExplanation: return {Direct};
		case StepType::FormulaExplanation: return {Formula};
		default: return {};
		}
	}
	if (!Feedback.IsEmpty()) return {};
	switch (Step)
	{
	case StepType::Intro: case StepType::TacticPoint: return {TP};
	case StepType::TacticPointExplanation: return {TP, SkillWord};
	case StepType::InspectGyokeres: case StepType::InspectOdegaard: return {SkillWord};
	case StepType::SkillRangeExplanation: return {LOCTEXT("ConceptLongShotSkill","远射技能"), TP};
	case StepType::ShootingExplanation: return {Shooting, Direct};
	case StepType::TraitExplanation: case StepType::Summary: return {Trait, Expert, Shooting};
	case StepType::Deploy:
		if (!bComparison) return {LOCTEXT("ConceptDeployPlayer","部署球员")};
		if (HintLevel==2) return {LOCTEXT("ConceptExpertHint","远射专家")};
		return HintLevel==1 ? TArray<FText>{Trait} : TArray<FText>{Shooting};
	case StepType::FinishExplanation: case StepType::FinishDeployment: return {Finish};
	case StepType::CarrierExplanation: case StepType::Carrier: return {Carrier};
	case StepType::SkillExplanation: case StepType::Skill: return {LongShot};
	case StepType::DirectExplanation: case StepType::DirectShot: return {Direct};
	case StepType::FormulaHover: return {Attribute};
	case StepType::FormulaExplanation:
		return {LOCTEXT("ConceptFormulaTotal","公式总值"), Attribute, LOCTEXT("ConceptRollValue","掷骰值")};
	case StepType::Rewind: return {LOCTEXT("ConceptLongShotSkill","远射技能"), LOCTEXT("ConceptLongShotTrait","远射特性")};
	default: return {};
	}
}

FText FFMCodexGuidedLesson1::EmphasizeKeywords(const FText& Copy, EFMCodexLesson1CopySurface Surface) const
{
	// Each approved phrase marks its first occurrence only, on its specified surface.
	// Longest phrase wins overlaps; a generic word is never an implicit fallback.
	TArray<FString> Words;
	for (const auto& Word : ConceptKeywords(Surface)) Words.Add(Word.ToString());
	Words.Sort([](const FString& A, const FString& B){ return A.Len() > B.Len(); });
	const FString Source = Copy.ToString();
	FString Result;
	for (int32 I=0; I<Source.Len();)
	{
		const int32 Match=Words.IndexOfByPredicate([&](const FString& W){ return !W.IsEmpty() && Source.Mid(I,W.Len())==W; });
		if (Match!=INDEX_NONE) { Result+=TEXT("<concept>")+Words[Match]+TEXT("</>"); I+=Words[Match].Len(); Words.RemoveAt(Match); }
		else { Result.AppendChar(Source[I]); ++I; }
	}
	return FText::FromString(Result);
}
bool FFMCodexGuidedLesson1::IsFormulaTeaching() const
{
	return Step == StepType::FormulaHover || Step == StepType::FormulaExplanation;
}
bool FFMCodexGuidedLesson1::InspectFormula(bool bProductionTooltipVisible)
{
	if (bExitConfirmation || Step != StepType::FormulaHover || !bProductionTooltipVisible) return false;
	bFormulaInspected=true; SetStep(StepType::FormulaExplanation); return true;
}

FText FFMCodexGuidedLesson1::ProgressLabel() const
{
	return bComparison ? LOCTEXT("Second", "教学 · 换个人试试") : LOCTEXT("First", "教学 · 第一次进攻");
}
FText FFMCodexGuidedLesson1::Instruction() const
{
	if (!Feedback.IsEmpty()) return Feedback;
	switch (Step)
	{
	case StepType::Intro: return LOCTEXT("IntroLongShot", "先掷出本回合的进攻战术点。");
	case StepType::TacticPointExplanation: return LOCTEXT("TPInspect", "本回合进攻战术点为 3。鼠标移动至本方球员区哲凯赖什处悬停，查看技能。");
	case StepType::InspectOdegaard: return LOCTEXT("InspectOdegaard", "鼠标移动至本方球员区厄德高处悬停，查看技能。");
	case StepType::ShootingExplanation: return LOCTEXT("SameShooting", "厄德高的射门也是 4。这是直接射门所使用的基础属性。");
	case StepType::TraitExplanation: return LOCTEXT("ExpertTeaching", "厄德高拥有「远射专家 A」。直接远射时，这项特性提供射门 +2。");
	case StepType::InspectGyokeres: return LOCTEXT("Inspect", "鼠标移动至本方球员区哲凯赖什处悬停，查看技能。");
	case StepType::SkillRangeExplanation: return LOCTEXT("SkillRange", "远射技能范围为 3–5。当前进攻战术点为 3，落在范围内，因此可以使用远射。");
	case StepType::FinishExplanation: return LOCTEXT("FinishConcept", "斯通斯已上场。此次远射只需一名进攻球员，可以结束部署。");
	case StepType::CarrierExplanation: return LOCTEXT("CarrierConcept", "点击场上的哲凯赖什，将他选中为本进攻回合的持球队员。");
	case StepType::SkillExplanation: return LOCTEXT("SkillConcept", "哲凯赖什当前可用的进攻技能是远射。接下来点击左下角“远射”。");
	case StepType::DirectExplanation: return LOCTEXT("DirectConcept", "进攻分支下有具体说明，这次请选择‘直接射门’。");
	case StepType::TacticPoint: return LOCTEXT("TP", "掷出本回合的进攻战术点。");
	case StepType::Deploy:
		if (!bComparison) return LOCTEXT("DeployFirst", "部署球员，点住鼠标左键将哲凯赖什拖到高亮位置。");
		if (HintLevel == 2) return LOCTEXT("Hint2", "“远射专家”可以提高直接远射时的射门能力。");
		if (HintLevel == 1) return LOCTEXT("Hint1", "打开球员卡，看看“特性”。");
		return LOCTEXT("Compare", "两人的射门都是 4。选择更适合远射的球员。");
	case StepType::OpponentDeploy: return bOpponentSubmitted ? LOCTEXT("OpponentPlaced", "斯通斯已部署到高亮位置。") : LOCTEXT("OpponentDeploy", "对手准备部署斯通斯。");
	case StepType::FinishDeployment: return LOCTEXT("Finish", "点击“结束部署”，结束本次部署。");
	case StepType::OpponentFinish: return bOpponentSubmitted ? LOCTEXT("FinishSettled", "对手已结束部署。") : LOCTEXT("OpponentFinishShort", "对手正在结束部署。");
	case StepType::Carrier: return bComparison ? LOCTEXT("CarrierSecond", "点击场上的厄德高，将他选中为本进攻回合的持球队员。") : LOCTEXT("CarrierFocus", "点击场上的哲凯赖什，将他选中为本进攻回合的持球队员。");
	case StepType::OpponentMarker: return bOpponentSubmitted ? LOCTEXT("MarkerSettled", "斯通斯已成为本回合的盯人球员。") : LOCTEXT("Marker", "对手选择斯通斯作为盯人球员。");
	case StepType::Skill: return LOCTEXT("Skill", "点击左下角的“远射”");
	case StepType::DirectShot: return LOCTEXT("Direct", "选择“直接射门”");
	case StepType::FormulaHover: return LOCTEXT("FormulaHover", "将鼠标悬停在进攻公式的数字 4 上，查看属性值的来源。");
	case StepType::FormulaExplanation: return LOCTEXT("FormulaExplain", "悬停属性值，可查看它的来源。\n公式总值由属性值与掷骰值相加得到。\n本次属性值仅来自哲凯赖什的射门 4。");
	case StepType::AttackRoll: return LOCTEXT("AttackRoll", "点击“进攻方掷远射点数”。");
	case StepType::OpponentDefense: return LOCTEXT("DefenseRoll", "对手正在进行防守判定。");
	case StepType::ResultReveal: return LOCTEXT("Reveal", "看看这次远射的结果。");
	case StepType::Rewind: return bResolvedGoal
		? LOCTEXT("UnexpectedGoal", "教学条件与预期不同，请退出教学并检查场景配置。")
		: LOCTEXT("FirstLesson", "哲凯赖什可以使用远射技能，但他没有远射特性。让我们换个人试试吧。");
	case StepType::RewindTransition: return LOCTEXT("Replay", "教学重演");
	case StepType::Summary: return bResolvedGoal
		? LOCTEXT("FitSummary", "相同的射门属性和骰点下，适合战术的特性改变了结果。\n先考虑战术，再选择适合它的球员。")
		: LOCTEXT("UnexpectedMiss", "教学条件与预期不同，请退出教学并检查场景配置。");
	case StepType::Complete: return LOCTEXT("Complete", "教学已完成。你已体验如何选择适合战术的球员。");
	default: return FText();
	}
}
FText FFMCodexGuidedLesson1::Explanation() const
{
	if (Step == StepType::Deploy && bComparison) return LOCTEXT("Unlock", "这一次，厄德高也可以上场。这是本课新增的选择；悬停卡牌查看特性。");
	if (Step == StepType::Summary) return LOCTEXT("Limits", "本次对照中，远射专家 A 提供了射门 +2。正式比赛中，结果仍会受到骰点和对手配置影响。");
	return FText();
}
FText FFMCodexGuidedLesson1::PrimaryLabel() const
{
	if (Step == StepType::Intro) return LOCTEXT("StartAction", "开始操作");
	if (Step == StepType::FormulaExplanation) return LOCTEXT("ContinueFormula", "继续");
	if (IsExplanationMode() && Step != StepType::Rewind && Step != StepType::Summary && Step != StepType::Complete) return LOCTEXT("Continue", "继续");
	if (Step == StepType::Rewind) return LOCTEXT("RewindCTA", "让时间倒流，换个人试试");
	if (Step == StepType::Summary) return LOCTEXT("FinishCTA", "完成教学");
	if (Step == StepType::Complete) return LOCTEXT("Return", "返回普通对局");
	return FText();
}
bool FFMCodexGuidedLesson1::IsExplanationMode() const
{
	switch (Step)
	{
	case StepType::Intro: case StepType::TacticPointExplanation: case StepType::ShootingExplanation: case StepType::SkillRangeExplanation: case StepType::TraitExplanation:
	case StepType::FinishExplanation: case StepType::CarrierExplanation: case StepType::SkillExplanation:
	case StepType::DirectExplanation: case StepType::FormulaExplanation: case StepType::Rewind: case StepType::Summary: case StepType::Complete: return true;
	default: return false;
	}
}
EFMCodexLesson1Focus FFMCodexGuidedLesson1::FocusTarget() const
{
	using F = EFMCodexLesson1Focus;
	switch (Step)
	{
	case StepType::TacticPoint: return F::TacticPoint;
	case StepType::InspectGyokeres: case StepType::InspectOdegaard: return F::HandCard;
	case StepType::Deploy: return F::Deployment;
	case StepType::FinishDeployment: return F::FinishDeployment;
	case StepType::Carrier: return F::Carrier;
	case StepType::Skill: return F::Skill;
	case StepType::DirectShot: return F::DirectShot;
	case StepType::FormulaHover: case StepType::FormulaExplanation: return F::FormulaValue;
	case StepType::AttackRoll: return F::AttackRoll;
	default: return F::None;
	}
}
bool FFMCodexGuidedLesson1::InspectCard(FName CardId, bool bFullCardVisible)
{
	if (bExitConfirmation) return false;
	if ((Step != StepType::InspectGyokeres && Step != StepType::InspectOdegaard) || CardId != Attacker() || !bFullCardVisible) return false;
	SetStep(StepType::SkillRangeExplanation);
	return true;
}
bool FFMCodexGuidedLesson1::AllowsInspection(FName CardId) const
{
	if (Step == StepType::InspectGyokeres || Step == StepType::InspectOdegaard || KeepsFullCardOpen()) return CardId == Attacker();
	if (Step == StepType::Deploy) return CardId == Gyokeres() || (bComparison && CardId == Odegaard());
	return Step == StepType::Carrier && CardId == Attacker();
}
bool FFMCodexGuidedLesson1::KeepsFullCardOpen() const
{
 return Step == StepType::ShootingExplanation || Step == StepType::SkillRangeExplanation || Step == StepType::TraitExplanation;
}
bool FFMCodexGuidedLesson1::CanProgress(const FFMCodexLocalMatchInteractionView& View,
 const FMCodexTacticalScene::FState& Scene, bool bRevealBlocked)
{
 if (bRevealBlocked || Scene.IsAnimating() || Scene.Celebration.IsActive()) return false;
 if (!View.bTerminalPendingAdvance) return true;
 // Observe the current disclosed production event, never totals, CTA availability or a delay.
 return Scene.Facts.bActive && Scene.Facts.AttackSequence == View.AttackSequence
  && Scene.Facts.Method == FMCodexTacticalScene::EMethod::Direct
  && Scene.Facts.Outcome != FMCodexTacticalScene::EOutcome::None
  && Scene.Phase == FMCodexTacticalScene::EPhase::ResultHold;
}
bool FFMCodexGuidedLesson1::YieldsToProduction(const FMCodexTacticalScene::FState& Scene, bool bRevealBlocked, Category CurrentAction) const
{
 if (bRevealBlocked || Scene.IsAnimating() || Scene.Celebration.IsActive()) return true;
 if (Scene.Facts.bActive && Scene.Facts.Method==FMCodexTacticalScene::EMethod::Direct
  && Scene.Facts.Outcome==FMCodexTacticalScene::EOutcome::None && Scene.Phase==FMCodexTacticalScene::EPhase::FormulaHold
  // The reveal clock can finish before the lesson's next 0.1s observation. Do not
  // reopen the stale AttackRoll strip/spotlight while the live action is defense.
  && CurrentAction==Category::RollLongShotDirectAttack
  && (IsFormulaTeaching() || Step==StepType::AttackRoll)) return false;
 return Scene.Facts.bActive && !Scene.Facts.bMethodChoice
  && Step != StepType::Rewind && Step != StepType::RewindTransition && Step != StepType::Summary && Step != StepType::Complete;
}
#undef LOCTEXT_NAMESPACE
#endif
