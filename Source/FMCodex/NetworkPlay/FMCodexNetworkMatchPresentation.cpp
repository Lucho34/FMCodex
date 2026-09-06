#include "FMCodexNetworkMatchPresentation.h"
#include "FMCodexNetworkMatchTypes.h"
#include "../LocalPlay/FMCodexPlayerUIPresentationText.h"
#include "../LocalPlay/FMCodexLocalMatchInteractionView.h"
#include "../LocalPlay/FMCodexLocalMatchResolutionFeedback.h"

namespace
{
	void CopyStaticCard(FFMCodexUMGCardViewModel& Target, const FFMCodexUMGCardViewModel& Source)
	{
		Target.IdentityLabel = Source.IdentityLabel;
		Target.EnglishIdentityLabel = Source.EnglishIdentityLabel;
		Target.NationalityLabel = Source.NationalityLabel;
		Target.ClubLabel = Source.ClubLabel;
		Target.RoleLabel = Source.RoleLabel;
		Target.OverallRating = Source.OverallRating;
		Target.bHasOverallRating = Source.bHasOverallRating;
		Target.BirthDate = Source.BirthDate;
		Target.HeightCm = Source.HeightCm;
		Target.WeightKg = Source.WeightKg;
		Target.AttributeValues = Source.AttributeValues;
		Target.Skills = Source.Skills;
		Target.PlayerFacingSerialLabel = Source.PlayerFacingSerialLabel;
		Target.SkillLabels = Source.SkillLabels;
		Target.SkillSummaryLabel = Source.SkillSummaryLabel;
		Target.CompactAttributeSummary = Source.CompactAttributeSummary;
		Target.FullAttributeSummary = Source.FullAttributeSummary;
		Target.RarityLabel = Source.RarityLabel;
		Target.bGoalkeeper = Source.bGoalkeeper;
	}
	template<typename F> void VisitCards(FFMCodexUMGMatchScreenViewModel& M, F&& Visitor)
	{
		for (auto& Cell : M.LocalRack.Cells) Visitor(Cell.Card);
		for (auto& Cell : M.OpponentRack.Cells) Visitor(Cell.Card);
		for (auto& Region : M.PitchRegions) for (auto& Slot : Region.Slots) Visitor(Slot.Card);
		for (auto& Choice : M.Interaction.DeploymentChoices) Visitor(Choice.Card);
		for (auto& Choice : M.Interaction.SelectionChoices) if (Choice.bHasCard) Visitor(Choice.Card);
	}
	bool WithinBounds(const FFMCodexUMGMatchScreenViewModel& M)
	{
		if (M.LocalRack.Cells.Num() > FFMCodexNetworkMatchPresentationAdapter::MaxCardsPerSide
			|| M.OpponentRack.Cells.Num() > FFMCodexNetworkMatchPresentationAdapter::MaxCardsPerSide
			|| M.PitchRegions.Num() > FFMCodexNetworkMatchPresentationAdapter::MaxPitchRegions
			|| M.Interaction.DeploymentChoices.Num() > 20 || M.Interaction.SelectionChoices.Num() > 20
			|| M.Interaction.BranchChoices.Num() > 2 || M.Interaction.OneOnOneChoices.Num() > 2
			|| M.ThroughBallResolution.OneOnOneChoices.Num() > 2 || M.LongShotResolution.BranchChoices.Num() > 2) return false;
		for (const auto& Region : M.PitchRegions)
			if (Region.Slots.Num() > FFMCodexNetworkMatchPresentationAdapter::MaxSlotsPerRegion) return false;
		for (const auto& Choice : M.Interaction.DeploymentChoices)
			if (Choice.Destinations.Num() > 80) return false;
		return true;
	}
}

void FFMCodexNetworkMatchPresentationAdapter::DisableActions(FFMCodexUMGMatchScreenViewModel& M)
{
	auto& I = M.Interaction;
	I.bCanStartNewMatch = I.bCanRollTacticalPoints = I.bCanFinishDeployment = false;
	I.bCanDecline = I.bCanResolveNoLegal = I.bCanContinue = false;
	I.PrimaryAction.bAvailable = false;
	I.bUseOnPitchPlayerSelection = false;
	I.DeploymentChoices.Reset(); I.SelectionChoices.Reset(); I.BranchChoices.Reset(); I.OneOnOneChoices.Reset();
	for (auto& Cell : M.LocalRack.Cells) Cell.bDeploymentDraggable = Cell.bSetPieceSelectable = false;
	for (auto& Cell : M.OpponentRack.Cells) Cell.bDeploymentDraggable = Cell.bSetPieceSelectable = false;
	for (auto& Region : M.PitchRegions) for (auto& Slot : Region.Slots)
	{
		Slot.bSelectableForCurrentPrompt = false;
		Slot.OnPitchSelectionIntent = EFMCodexUMGOnPitchSelectionIntent::None;
	}
	M.InlineFormula.PrimaryAction.bVisible = M.InlineFormula.PrimaryAction.Action.bAvailable = false;
	M.InlineFormula.bCanContinue = false;
	M.LongShotResolution.PrimaryAction.bVisible = M.LongShotResolution.PrimaryAction.Action.bAvailable = false;
	M.LongShotResolution.bCanContinue = false;
	M.LongShotResolution.BranchChoices.Reset();
	M.LongShotResolution.Formula.PrimaryAction.bVisible = M.LongShotResolution.Formula.PrimaryAction.Action.bAvailable = false;
	M.LongShotResolution.Formula.bCanContinue = false;
	M.ThroughBallResolution.PrimaryAction.bVisible = M.ThroughBallResolution.PrimaryAction.Action.bAvailable = false;
	M.ThroughBallResolution.Formula.PrimaryAction.bVisible = M.ThroughBallResolution.Formula.PrimaryAction.Action.bAvailable = false;
	M.ThroughBallResolution.bCanContinue = M.ThroughBallResolution.Formula.bCanContinue = false;
	M.ThroughBallResolution.OneOnOneChoices.Reset();
}

FFMCodexNetworkMatchPresentation FFMCodexNetworkMatchPresentationAdapter::Project(
	const FFMCodexLocalMatchInteractionView& SafeView, EInitialTurnOrderPlayer Viewer)
{
	FFMCodexNetworkMatchPresentation Result;
	// This builder only formats already-projected public facts and canonical static descriptions.
	auto M = FFMCodexLocalMatchUMGPresentationBuilder::Build(SafeView,
		FFMCodexLocalMatchResolutionFeedbackBuilder::BuildFromTerminalSnapshot(SafeView), FString(), Viewer);
	if (!WithinBounds(M)) return Result; // Never silently truncate legal options.
	M.Interaction.bCanStartNewMatch = M.Interaction.bCanDecline = M.Interaction.bCanResolveNoLegal = false;
	M.Interaction.CandidateCards.Reset();
	M.Interaction.LegalActionLabels.Reset();
	M.Interaction.ClassificationLabel.Reset();
	M.Interaction.CategoryLabel.Reset();
	// Capability is separate from server legality: retain unsupported options with an explicit disabled state.
	for (auto& Choice : M.Interaction.SelectionChoices)
		if (M.Interaction.Category == EFMCodexUMGInteractionCategory::SelectSkill
			&& Choice.SkillType != ESkillRuleType::Cross && Choice.SkillType != ESkillRuleType::PassControl
			&& Choice.SkillType != ESkillRuleType::ThroughBall
			&& Choice.SkillType != ESkillRuleType::LongShot && Choice.SkillType != ESkillRuleType::CutInsideShot)
		{
			Choice.bEnabled = false;
			Choice.SecondaryLabel = TEXT("此联网演示暂未支持");
		}
	if (!SafeView.bHumanInteraction || SafeView.ExpectedActingPlayer != Viewer)
		DisableActions(M);
	VisitCards(M, [&Result](FFMCodexUMGCardViewModel& Card)
	{
		Card.DeveloperReferenceLabel.Reset();
		if (Card.CardId.IsNone()) return;
		if (!Result.CardCatalog.ContainsByPredicate([&Card](const auto& Existing) { return Existing.CardId == Card.CardId; }))
		{
			auto& Static = Result.CardCatalog.AddDefaulted_GetRef();
			Static.CardId = Card.CardId;
			CopyStaticCard(Static, Card);
		}
		CopyStaticCard(Card, FFMCodexUMGCardViewModel());
	});
	if (Result.CardCatalog.Num() > MaxCardsPerSide * 2) return {};
	const auto& Facts = M.Resolution.FormulaFacts;
	if (Facts.bSuccess && Facts.bHasFacts)
	{
		for (const auto& Roll : Facts.Rolls)
		{
			if (!Roll.bResolved) continue;
			FFMCodexUMGResolvedRollViewModel Event;
			Event.AttackSequence = Facts.AttackSequence;
			Event.OwnerSide = Roll.OwningSide; Event.SequenceIndex = Roll.SequenceIndex; Event.RawD6 = Roll.RawD6;
			if (Roll.bInitialRoute && Roll.Semantics == EMatchPlayResolutionRollSemantics::BranchSelection)
			{
				Event.Kind = EFMCodexUMGCrossRollRevealKind::InitialRoute;
				switch (Facts.ActualBranch.ActionType)
				{
				case ESkillRuleType::Cross: Event.ContestId = TEXT("Cross.Route"); break;
				case ESkillRuleType::PassControl: Event.Kind = EFMCodexUMGCrossRollRevealKind::PassControlInitialRoute; Event.ContestId = TEXT("PassControl.Route"); break;
				case ESkillRuleType::ThroughBall: Event.Kind = EFMCodexUMGCrossRollRevealKind::ThroughBallInitialRoute; Event.ContestId = TEXT("ThroughBall.Route"); break;
				default: continue;
				}
			}
			else
			{
				using P = EMatchPlayCurrentAttackPostRouteRollPurpose;
				const auto Purpose = Roll.PostRoutePurpose;
				Event.Kind = Purpose == P::PrimaryAttack || Purpose == P::OneOnOneDirectShotAttack || Purpose == P::OneOnOneChipShotAttack
					? EFMCodexUMGCrossRollRevealKind::Attack : Purpose == P::PrimaryDefense || Purpose == P::OneOnOneDirectShotDefense
						? EFMCodexUMGCrossRollRevealKind::Defense : EFMCodexUMGCrossRollRevealKind::None;
				const bool LongShot = Facts.ActualBranch.ActionType == ESkillRuleType::LongShot;
				const bool CutInside = Facts.ActualBranch.ActionType == ESkillRuleType::CutInsideShot;
				if ((LongShot || CutInside) && (Purpose == P::PairedAttackA || Purpose == P::PairedAttackB))
				{
					const bool First = Purpose == P::PairedAttackA;
					Event.Kind = LongShot ? (First ? EFMCodexUMGCrossRollRevealKind::LongShotDeadCornerA : EFMCodexUMGCrossRollRevealKind::LongShotDeadCornerB)
						: (First ? EFMCodexUMGCrossRollRevealKind::CutInsideShotDeadCornerA : EFMCodexUMGCrossRollRevealKind::CutInsideShotDeadCornerB);
					Event.ContestId = LongShot ? TEXT("LongShot.DeadCorner") : TEXT("CutInsideShot.DeadCorner");
				}
				else if (LongShot || CutInside)
					Event.ContestId = LongShot ? TEXT("LongShot.DirectShot") : TEXT("CutInsideShot.DirectShot");
				else if (Purpose == P::OneOnOneChipShotAttack) Event.ContestId = TEXT("ThroughBall.OneOnOne.ChipShot");
				else if (Purpose == P::OneOnOneDirectShotAttack || Purpose == P::OneOnOneDirectShotDefense)
					Event.ContestId = TEXT("ThroughBall.OneOnOne.DirectShot");
				else if (Facts.ActualBranch.ActionType == ESkillRuleType::ThroughBall
					&& Facts.ActualBranch.ThroughBall == EMatchPlayThroughBallActualBranch::AntiOffside)
					Event.ContestId = TEXT("ThroughBall.AntiOffside");
				else if (Facts.FormulaContests.Num() > 0) Event.ContestId = Facts.FormulaContests[0].ContestId;
				if (Event.ContestId.IsNone()) continue;
			}
			if (Event.Kind != EFMCodexUMGCrossRollRevealKind::None) Result.ResolvedRolls.Add(Event);
		}
	}
	if (Result.ResolvedRolls.Num() > 5 || M.FullTime.PlayerA.Goals.Num() > 6 || M.FullTime.PlayerB.Goals.Num() > 6) return {};
	Result.bAvailable = true;
	Result.Header = MoveTemp(M.Header);
	Result.LocalRack = MoveTemp(M.LocalRack); Result.OpponentRack = MoveTemp(M.OpponentRack);
	Result.PitchRegions = MoveTemp(M.PitchRegions);
	Result.Interaction = MoveTemp(M.Interaction);
	Result.InlineFormula = MoveTemp(M.InlineFormula);
	Result.BranchSurface = MoveTemp(M.LongShotResolution);
	Result.ThroughBallSurface = MoveTemp(M.ThroughBallResolution);
	Result.FullTime = MoveTemp(M.FullTime);
	return Result;
}

FFMCodexUMGMatchScreenViewModel FFMCodexNetworkMatchPresentationAdapter::Read(
	const FFMCodexNetworkMatchPresentation& View, bool bPending)
{
	FFMCodexUMGMatchScreenViewModel M;
	if (!View.bAvailable)
	{
		M.Interaction.TitleLabel = TEXT("等待网络比赛就绪");
		M.Header.PlayerALabel = M.Header.LeftPlayerLabel = TEXT("玩家 A");
		M.Header.PlayerBLabel = M.Header.RightPlayerLabel = TEXT("玩家 B");
		DisableActions(M);
		return M;
	}
	M.Header = View.Header; M.LocalRack = View.LocalRack; M.OpponentRack = View.OpponentRack;
	M.LocalPlayerLabel = View.LocalRack.SideLabel;
	M.PitchRegions = View.PitchRegions; M.Interaction = View.Interaction;
	M.InlineFormula = View.InlineFormula; M.LongShotResolution = View.BranchSurface;
	M.ThroughBallResolution = View.ThroughBallSurface;
	M.FullTime = View.FullTime;
	M.ResolvedRolls = View.ResolvedRolls;
	VisitCards(M, [&View](FFMCodexUMGCardViewModel& Card)
	{
		if (const auto* Static = View.CardCatalog.FindByPredicate([&Card](const auto& C) { return C.CardId == Card.CardId; }))
			CopyStaticCard(Card, *Static);
	});
	if (bPending)
	{
		DisableActions(M);
		M.Interaction.EmptyStateLabel = TEXT("正在提交，请稍候");
	}
	return M;
}

#define LOCTEXT_NAMESPACE "FMCodexActionWaitPrompt"
FFMCodexUMGMatchScreenViewModel FFMCodexNetworkMatchPresentationAdapter::Read(
	const FFMCodexNetworkClientViewSnapshot& View, const bool bPending)
{
	auto M = Read(View.Presentation, bPending);
	using C = EFMCodexUMGInteractionCategory;
	using S = EInitialTurnOrderPlayer;
	if (!View.Presentation.bAvailable || View.bMatchEnded || M.FullTime.bVisible
		|| (View.ExpectedActingSide != S::PlayerA && View.ExpectedActingSide != S::PlayerB)
		|| (View.ViewerSide != S::PlayerA && View.ViewerSide != S::PlayerB)) return M;
	FText Action;
	switch (M.Interaction.Category)
	{
	case C::TacticalPointRoll: Action = LOCTEXT("TacticalPoints", "掷战术点"); break;
	case C::Deploy: Action = LOCTEXT("Deploy", "部署球员并完成部署"); break;
	case C::SelectCarrier: Action = FFMCodexPlayerUIPresentationText::MatchScreenLabel(TEXT("Select Carrier")); break;
	case C::SelectMarker: Action = FFMCodexPlayerUIPresentationText::MatchScreenLabel(TEXT("Select Marker")); break;
	case C::SelectRunner: Action = FFMCodexPlayerUIPresentationText::MatchScreenLabel(TEXT("Select Runner")); break;
	case C::SelectHelper: Action = FFMCodexPlayerUIPresentationText::MatchScreenLabel(TEXT("Select Helper")); break;
	case C::SelectSkill: Action = LOCTEXT("Skill", "选择战术"); break;
	case C::SelectBranchIntent:
	case C::SelectLongShotBranch:
		Action = M.LongShotResolution.SkillType == ESkillRuleType::Cross
			? LOCTEXT("CrossBranch", "选择传中方式") : LOCTEXT("ShotBranch", "选择射门方式"); break;
	case C::RollPassControlRoute: Action = LOCTEXT("PassControlRoute", "掷传控路线骰"); break;
	case C::RollThroughBallInitialRoute: Action = LOCTEXT("ThroughBallRoute", "掷直塞路线骰"); break;
	case C::RollCrossRoute: Action = LOCTEXT("CrossRoute", "掷传中路线骰"); break;
	case C::SelectOneOnOneShot: Action = LOCTEXT("OneOnOneChoice", "选择单刀射门方式"); break;
	case C::RollThroughBallAntiOffsideAttack: Action = LOCTEXT("AntiOffsideRoll", "掷反越位点数"); break;
	case C::RollThroughBallOneOnOneChipShotAttack: Action = LOCTEXT("ChipRoll", "掷挑射点数"); break;
	case C::RollLongShotDirectAttack:
	case C::RollCutInsideShotDirectAttack:
	case C::RollThroughBallBehindDefenseAttack:
	case C::RollThroughBallOneOnOneDirectShotAttack:
	case C::RollPassControlAttack:
	case C::RollThroughBallFeetAttack:
	case C::RollCrossAttack: Action = LOCTEXT("CrossAttack", "进攻方掷点"); break;
	case C::RollLongShotDirectDefense:
	case C::RollCutInsideShotDirectDefense:
	case C::RollThroughBallBehindDefenseDefense:
	case C::RollThroughBallOneOnOneDirectShotDefense:
	case C::RollPassControlDefense:
	case C::RollThroughBallFeetDefense:
	case C::RollCrossDefense: Action = LOCTEXT("CrossDefense", "防守方掷点"); break;
	case C::RollLongShotDeadCorner:
	case C::RollCutInsideShotDeadCorner: Action = LOCTEXT("DeadCornerPair", "进攻方掷两枚骰"); break;
	case C::AdvanceAfterTerminal: Action = LOCTEXT("Advance", "下一回合"); break;
	default: return M; // Unsupported families and non-player progression keep their existing presentation.
	}
	M.bMirrorActionWaitPrompt = true;
	M.bActionWaitPromptReadOnly = View.ExpectedActingSide != View.ViewerSide;
	const auto Actor = FFMCodexPlayerUIPresentationText::MatchScreenLabel(
		View.ExpectedActingSide == S::PlayerA ? TEXT("Player A") : TEXT("Player B"));
	M.ActionWaitActorText = M.bActionWaitPromptReadOnly
		? FText::Format(LOCTEXT("WaitingActor", "等待{0} 操作"), Actor)
		: bPending ? LOCTEXT("Submitting", "正在提交，请稍候")
		: LOCTEXT("YourTurn", "轮到你操作");
	M.ActionWaitActionText = !M.bActionWaitPromptReadOnly ? Action
		: M.Interaction.Category == C::AdvanceAfterTerminal
			? LOCTEXT("WaitingAdvance", "等待下一回合推进")
			: FText::Format(LOCTEXT("WaitingAction", "等待{0}"), Action);
	M.CentralActionPromptText = FText::Format(LOCTEXT("CentralActor", "当前操作：{0}"), Actor);
	if (bPending || (M.bActionWaitPromptReadOnly && M.Interaction.Category == C::AdvanceAfterTerminal))
	{
		M.CentralActionPromptText = FText::Format(LOCTEXT("CentralActorStatus", "{0}\n{1}"),
			M.CentralActionPromptText, bPending ? LOCTEXT("Submitting", "正在提交，请稍候") : M.ActionWaitActionText);
	}
	return M;
}
#undef LOCTEXT_NAMESPACE
