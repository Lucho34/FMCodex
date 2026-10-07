#include "FMCodexGuidedLesson1.h"
#if !UE_BUILD_SHIPPING
#include "FMCodexPrototypeTeamContent.h"
#include "../CoreRules/PlayerTraitFormula.h"

using StepType = EFMCodexLesson1Step;
using Gesture = EFMCodexMatchScreenIntent;
using Category = EFMCodexLocalMatchInteractionCategory;

FFMCodexGuidedLesson1::FFMCodexGuidedLesson1()
{
 Content=FFMCodexGuidedMatchContent::Load(ContentError);
 if(Content) BindStaticContent();
}
FFMCodexGuidedLesson1::FFMCodexGuidedLesson1(TSharedPtr<const FFMCodexGuidedMatchContent> InContent):Content(MoveTemp(InContent))
{
 if(Content) BindStaticContent(); else ContentError=TEXT("Missing canonical tutorial content");
}
void FFMCodexGuidedLesson1::BindStaticContent()
{
 using Var=EFMCodexGuideVariable;
 auto BindPlayer=[&](FName Id,Var Name,Var Shooting)->bool
 {
  const auto* Player=FFMCodexPrototypeTeamContent::Find(Id);
  if(!Player || Player->PreferredDisplayName.IsEmpty()) return false;
  StaticBindings.Add(Name,Player->PreferredDisplayName.ToString());
  StaticBindings.Add(Shooting,FString::FromInt(Player->Card.Attributes.Shooting)); return true;
 };
 if(!BindPlayer(Gyokeres(),Var::FirstCarrierName,Var::FirstCarrierShooting)
  || !BindPlayer(Odegaard(),Var::ComparisonCarrierName,Var::ComparisonCarrierShooting))
 { ContentError=TEXT("Missing tutorial player content binding"); return; }
 const auto* Marker=FFMCodexPrototypeTeamContent::Find(Stones());
 if(!Marker || Marker->PreferredDisplayName.IsEmpty()) { ContentError=TEXT("Missing marker display name"); return; }
 StaticBindings.Add(Var::MarkerName,Marker->PreferredDisplayName.ToString());
 const auto* Comparison=FFMCodexPrototypeTeamContent::Find(Odegaard());
 const auto* Trait=Comparison->Card.RankedTraits.FindByPredicate([](const auto& T){return T.TraitId==FName(TEXT("Trait.LongShotCarrier"));});
 if(!Trait || Trait->Rank==EPlayerTraitRank::None) { ContentError=TEXT("Missing tutorial trait display binding"); return; }
 const FString Rank=Trait->Rank==EPlayerTraitRank::S?TEXT("S"):Trait->Rank==EPlayerTraitRank::A?TEXT("A"):TEXT("B");
 StaticBindings.Add(Var::TraitName,FPlayerTraitFormula::DisplayName(Trait->TraitId).ToString()+TEXT(" ")+Rank);
}
TMap<EFMCodexGuideVariable,FString> FFMCodexGuidedLesson1::Bindings() const
{
 using Var=EFMCodexGuideVariable;
 auto Result=StaticBindings;
 if(const auto* Player=FFMCodexPrototypeTeamContent::Find(Attacker()))
 {
  Result.Add(Var::CarrierName,Player->PreferredDisplayName.ToString());
  Result.Add(Var::CarrierShooting,FString::FromInt(Player->Card.Attributes.Shooting));
  const auto* Range=Player->SkillAssignments.FindByPredicate([](const auto& S){return S.RuleId==Skill();});
  if(Range) { Result.Add(Var::SkillMin,FString::FromInt(Range->MinTacticalPoint)); Result.Add(Var::SkillMax,FString::FromInt(Range->MaxTacticalPoint)); }
 }
 if(CurrentTP.IsSet()) Result.Add(Var::CurrentAttackTP,FString::FromInt(CurrentTP.GetValue()));
 if(DisplayedAttackBase.IsSet()) Result.Add(Var::FormulaAttackBase,FText::AsNumber(DisplayedAttackBase.GetValue()).ToString());
 return Result;
}
FText FFMCodexGuidedLesson1::Resolve(const FString& Template) const
{
 FString Text,Error;
 if(IsContentReady() && FFMCodexGuidedMatchContent::Interpolate(Template,Bindings(),Text,Error)) return FText::FromString(Text);
 if(ContentError.IsEmpty()) { ContentError=Error; UE_LOG(LogTemp,Error,TEXT("Tutorial content binding failed: %s"),*ContentError); }
 // Bootstrap failure message remains code-owned; never fall back to old lesson copy.
 return NSLOCTEXT("FMCodexGuidedContent","LoadFailure","教学内容加载失败，请退出教学并查看日志。");
}
FString FFMCodexGuidedLesson1::ContentStepId() const
{
 static const TCHAR* Ids[]={TEXT("Intro"),TEXT("TacticPoint"),TEXT("TacticPointExplanation"),TEXT("InspectGyokeres"),TEXT("InspectOdegaard"),TEXT("ShootingExplanation"),TEXT("SkillRangeExplanation"),TEXT("TraitExplanation"),
  TEXT("Deploy"),TEXT("OpponentDeploy"),TEXT("FinishExplanation"),TEXT("FinishDeployment"),TEXT("OpponentFinish"),TEXT("CarrierExplanation"),TEXT("Carrier"),TEXT("OpponentMarker"),TEXT("SkillExplanation"),TEXT("Skill"),
  TEXT("DirectExplanation"),TEXT("DirectShot"),TEXT("FormulaHover"),TEXT("FormulaExplanation"),TEXT("AttackRoll"),TEXT("OpponentDefense"),TEXT("ResultReveal"),TEXT("FailurePause"),TEXT("Rewind"),TEXT("RewindTransition"),TEXT("Summary"),TEXT("Complete")};
 static_assert(UE_ARRAY_COUNT(Ids)==static_cast<int32>(StepType::Complete)+1,"Each code step needs a stable content key");
 FString Id=Ids[static_cast<int32>(Step)];
 if(Step==StepType::Deploy && bComparison) Id+=HintLevel==2?TEXT(".Hint2"):HintLevel==1?TEXT(".Hint1"):TEXT(".Comparison");
 if(bOpponentSubmitted && (Step==StepType::OpponentDeploy || Step==StepType::OpponentFinish || Step==StepType::OpponentMarker)) Id+=TEXT(".After");
 if((Step==StepType::Rewind && bResolvedGoal) || (Step==StepType::Summary && !bResolvedGoal)) Id+=TEXT(".Unexpected");
 return Id;
}
const FFMCodexGuideStepPresentation& FFMCodexGuidedLesson1::Presentation() const
{
 static const FFMCodexGuideStepPresentation Empty;
 if(!Content) return Empty;
 FString Error; const auto* P=Content->FindStep(TEXT("Lesson01"),ContentStepId(),Error);
 if(!P) { ContentError=Error; return Empty; } return *P;
}
float FFMCodexGuidedLesson1::Timing(const TCHAR* Key) const
{
 const auto* L=Content?Content->FindLesson(TEXT("Lesson01")):nullptr;
 const float* Value=L?L->Timings.Find(Key):nullptr;
 if(!Value) { ContentError=FString(TEXT("Missing tutorial timing: "))+Key; return 0.f; }
 return *Value;
}
FText FFMCodexGuidedLesson1::Label(const TCHAR* Key) const
{
 const auto* L=Content?Content->FindLesson(TEXT("Lesson01")):nullptr;
 const FString* Text=L?L->Labels.Find(Key):nullptr;
 if(!Text) ContentError=FString(TEXT("Missing tutorial label: "))+Key;
 return Resolve(Text?*Text:FString());
}
FText FFMCodexGuidedLesson1::Heading() const { return Resolve(Presentation().Title); }
FText FFMCodexGuidedLesson1::PointerLabel() const { return Resolve(Presentation().Pointer); }
FText FFMCodexGuidedLesson1::Eyebrow() const
{
 const auto* L=Content?Content->FindLesson(TEXT("Lesson01")):nullptr;
 return Resolve(L?(bComparison?L->ComparisonTitle:L->Title):FString());
}

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
 if (!IsContentReady() || bExitConfirmation) return false;
 CurrentTP=V.ActionPoint;
 DisplayedAttackBase.Reset();
 for(const auto& Contest:V.ResolutionFacts.FormulaContests)
  if(Contest.AttackRow.bKnownNonRollSubtotalResolved) { DisplayedAttackBase=Contest.AttackRow.KnownNonRollSubtotal; break; }
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
	if (RequiresAcknowledgement() || Step == StepType::InspectGyokeres || Step == StepType::InspectOdegaard || Step == StepType::RewindTransition) return false;
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
				else if (StepSeconds >= Timing(TEXT("FailureFollowupDelay"))) SetStep(StepType::Rewind);
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
		HintLevel = StepSeconds >= Timing(TEXT("HintSecondDelay")) ? 2 : StepSeconds >= Timing(TEXT("HintFirstDelay")) ? 1 : 0;
	return Previous != Step || PreviousHint != HintLevel;
}

void FFMCodexGuidedLesson1::Primary()
{
	if (!IsContentReady() || bExitConfirmation) return;
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
	DisplayedAttackBase.Reset();
	bComparison = true; bDispatchingOpponent = false; bResolvedGoal = false; bFormulaInspected = false;
	bExitConfirmation = false; bOpponentSubmitted = false; OpponentSettledSeconds = 0.f;
	FeedbackSeconds = 0; Feedback = FText();
	SetStep(StepType::InspectOdegaard);
}
bool FFMCodexGuidedLesson1::IsCheckpointDue() const { return Step == StepType::RewindTransition && StepSeconds >= Timing(TEXT("RewindDelay")); }
bool FFMCodexGuidedLesson1::IsOpponentPresenting() const
{
	return Step == StepType::OpponentDeploy || Step == StepType::OpponentFinish
		|| Step == StepType::OpponentMarker || Step == StepType::OpponentDefense;
}
bool FFMCodexGuidedLesson1::IsOpponentActionDue() const
{
	if (!IsOpponentPresenting() || bOpponentSubmitted || bExitConfirmation) return false;
	if (Step == StepType::OpponentDeploy) return bOpponentMoveArrived;
	const float Lead = Step == StepType::OpponentDefense ? Timing(TEXT("OpponentDefenseLead"))
		: Timing(TEXT("OpponentPreAction")) + (Step == StepType::OpponentFinish ? Timing(TEXT("OpponentFinishFocus")) : Timing(TEXT("OpponentChoiceFocus")));
	return StepSeconds >= Lead * OpponentPace();
}
float FFMCodexGuidedLesson1::OpponentPostActionHold() const
{
	// Repeated gestures are slightly quicker; preserve the full readable end hold.
	return Step==StepType::OpponentDeploy ? Timing(TEXT("OpponentDestinationHold"))*OpponentPace()+Timing(TEXT("OpponentDeploySettledHold")) : Timing(TEXT("OpponentSettledHold"));
}
bool FFMCodexGuidedLesson1::CanStartOpponentDeploymentMove() const
{
	return Step==StepType::OpponentDeploy && !bOpponentMoveStarted && !bOpponentSubmitted && !bExitConfirmation
		&& StepSeconds>=Timing(TEXT("OpponentDeployAttention"))*OpponentPace();
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
	if (FocusTarget()==EFMCodexGuideTarget::OpponentStatus) return Target::SideStatus;
	if (FocusTarget()==EFMCodexGuideTarget::OpponentDeployment)
		return bOpponentSubmitted ? Target::FieldCard : bOpponentMoveStarted ? Target::MovingCard : Target::HandCard;
	return FocusTarget()==EFMCodexGuideTarget::OpponentMarker ? Target::FieldCard : Target::None;
}
bool FFMCodexGuidedLesson1::IsOpponentFinalHold() const
{
	return Step==StepType::OpponentDeploy && bOpponentSubmitted && OpponentSettledSeconds>=Timing(TEXT("OpponentDestinationHold"))*OpponentPace();
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
	if (!IsContentReady() || bExitConfirmation) return false;
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
	FeedbackSeconds = Timing(TEXT("FeedbackDuration"));
 Feedback = Label(bComparison && Step == StepType::Deploy && R.OptionId == Gyokeres()?TEXT("FeedbackCompare"):TEXT("FeedbackUnavailable"));
}

void FFMCodexGuidedLesson1::ApplyPresentation(FFMCodexUMGMatchScreenViewModel& P) const
{
	P.LocalRack.Cells.RemoveAll([&](const auto& C) { return C.Card.CardId != Gyokeres() && (!bComparison || C.Card.CardId != Odegaard()); });
	P.OpponentRack.Cells.RemoveAll([&](const auto& C) { return C.Card.CardId != Stones(); });
	for (auto& C : P.LocalRack.Cells) C.bDeploymentDraggable &= Step == StepType::Deploy;
	for (auto& C : P.OpponentRack.Cells) C.bDeploymentDraggable = false;
	P.LocalRack.SideLabel = Label(TEXT("LocalRack")).ToString();
	P.OpponentRack.SideLabel = Label(TEXT("OpponentRack")).ToString();
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
		if (Step != StepType::Skill || C.OptionId != Skill()) { C.bEnabled = false; C.SecondaryLabel = Label(TEXT("UnavailableChoice")).ToString(); }
	// The existing theater retains the absent method as a disabled option.
	P.Interaction.BranchChoices.RemoveAll([](const auto& C) { return C.Intent != EFMCodexUMGBranchIntent::DirectShot; });
	P.LongShotResolution.BranchChoices.RemoveAll([](const auto& C) { return C.Intent != EFMCodexUMGBranchIntent::DirectShot; });
	for (auto& Region : P.PitchRegions)
		for (auto& Slot : Region.Slots)
		{
			Slot.bSelectableForCurrentPrompt &= Step == StepType::Carrier && Slot.Card.CardId == Attacker();
			if (Step == StepType::Deploy && Slot.SlotId == AttackerSlot())
			{
				if (Step == StepType::Deploy) Slot.SlotLabel = Label(TEXT("DeploymentTarget")).ToString();
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
 if (Surface==EFMCodexLesson1CopySurface::Section || Surface==EFMCodexLesson1CopySurface::Secondary || !Feedback.IsEmpty()) return {};
 const FString Field=Surface==EFMCodexLesson1CopySurface::Heading?TEXT("TitleCN"):TEXT("BodyCN");
 TArray<FText> Result;
 for(const auto& Span:Presentation().Emphasis) if(Span.Field==Field) Result.Add(Resolve(Span.MatchText));
 return Result;
}
FText FFMCodexGuidedLesson1::EmphasizeKeywords(const FText& Copy, EFMCodexLesson1CopySurface Surface) const
{
 if(Surface==EFMCodexLesson1CopySurface::Section || Surface==EFMCodexLesson1CopySurface::Secondary || !Feedback.IsEmpty()) return Copy;
 const auto& P=Presentation(); FString Text,Error;
 const bool HeadingSurface=Surface==EFMCodexLesson1CopySurface::Heading;
 if(!FFMCodexGuidedMatchContent::Render(HeadingSurface?P.Title:P.Body,P.Emphasis,HeadingSurface?TEXT("TitleCN"):TEXT("BodyCN"),Bindings(),Text,Error))
 { Resolve(HeadingSurface?P.Title:P.Body); return Copy; }
 return FText::FromString(Text);
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

FText FFMCodexGuidedLesson1::ProgressLabel() const { return Label(bComparison?TEXT("ProgressComparison"):TEXT("ProgressFirst")); }
FText FFMCodexGuidedLesson1::Instruction() const { return Feedback.IsEmpty()?Resolve(Presentation().Body):Feedback; }
FText FFMCodexGuidedLesson1::Explanation() const { return Resolve(Presentation().Secondary); }
FText FFMCodexGuidedLesson1::PrimaryLabel() const { return Resolve(Presentation().CTA); }
bool FFMCodexGuidedLesson1::IsExplanationMode() const { return Presentation().Surface==EFMCodexGuideSurface::ExplanationPanel; }
bool FFMCodexGuidedLesson1::RequiresAcknowledgement() const
{
	switch (Step)
	{
	case StepType::Intro: case StepType::TacticPointExplanation: case StepType::ShootingExplanation: case StepType::SkillRangeExplanation: case StepType::TraitExplanation:
	case StepType::FinishExplanation: case StepType::CarrierExplanation: case StepType::SkillExplanation:
	case StepType::DirectExplanation: case StepType::FormulaExplanation: case StepType::Rewind: case StepType::Summary: case StepType::Complete: return true;
	default: return false;
	}
}
EFMCodexLesson1Focus FFMCodexGuidedLesson1::FocusTarget() const { return Presentation().Target; }
bool FFMCodexGuidedLesson1::InspectCard(FName CardId, bool bFullCardVisible)
{
	if (!IsContentReady() || bExitConfirmation) return false;
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
#endif
