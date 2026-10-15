#include "FMCodexDefensiveCardContext.h"
#include "FMCodexAttackingCardContext.h"
#include "FMCodexLocalMatchInteractionView.h"
#include "FMCodexLocalMatchUMGPresentation.h"

namespace
{
using Skill = ESkillRuleType;
using Role = EMatchPlayResolutionParticipantRole;
struct FCounter { FName Id; Skill Family; Role Participant; FName Attribute; FText Label; };
const TArray<FCounter>& Counters()
{
    // Stable presentation ordering and names, not a Formula/bonus evaluator.
    static const TArray<FCounter> Values = {
        {TEXT("Trait.LongShotBlocker"),Skill::LongShot,Role::Marker,TEXT("DEF"),NSLOCTEXT("FMCodexCounters","LongShot","防远射")},
        {TEXT("Trait.CutInsideStopper"),Skill::CutInsideShot,Role::Marker,TEXT("DEF"),NSLOCTEXT("FMCodexCounters","CutInside","防内切")},
        {TEXT("Trait.CrossMarkerBlocker"),Skill::Cross,Role::Marker,TEXT("DEF"),NSLOCTEXT("FMCodexCounters","Cross","防传中")},
        {TEXT("Trait.CrossHighHelperDefense"),Skill::Cross,Role::Helper,TEXT("STR"),NSLOCTEXT("FMCodexCounters","CrossHigh","高球协防")},
        {TEXT("Trait.CrossLowHelperDefense"),Skill::Cross,Role::Helper,TEXT("SPD"),NSLOCTEXT("FMCodexCounters","CrossLow","低球协防")},
        {TEXT("Trait.ThroughBallMarkerDefense"),Skill::ThroughBall,Role::Marker,TEXT("DEF"),NSLOCTEXT("FMCodexCounters","ThroughBall","防直塞")},
        {TEXT("Trait.ThroughBallFeetHelperDefense"),Skill::ThroughBall,Role::Helper,TEXT("DEF"),NSLOCTEXT("FMCodexCounters","Feet","脚下协防")},
        {TEXT("Trait.ThroughBallBehindHelperDefense"),Skill::ThroughBall,Role::Helper,TEXT("SPD"),NSLOCTEXT("FMCodexCounters","Behind","身后协防")}
    };
    return Values;
}

bool RouteAllows(const FCounter& Counter, const FFMCodexLocalMatchInteractionView& View)
{
    const auto& Facts = View.ResolutionFacts;
    if (!Facts.bSuccess || !Facts.bHasFacts || !Facts.bHasActualBranch) return true;
    const auto& Branch = Facts.ActualBranch;
    if (Branch.ActionType != Counter.Family) return false;
    switch (Counter.Family)
    {
    case Skill::LongShot: return Branch.LongShot == EMatchPlayLongShotActualBranch::DirectShot;
    case Skill::CutInsideShot: return Branch.CutInsideShot == EMatchPlayCutInsideShotActualBranch::DirectShot;
    case Skill::Cross:
        return Counter.Participant == Role::Marker
            || (Counter.Attribute == TEXT("STR") && Branch.Cross == EMatchPlayCrossActualBranch::High)
            || (Counter.Attribute == TEXT("SPD") && Branch.Cross == EMatchPlayCrossActualBranch::Low);
    case Skill::ThroughBall:
        if (Branch.ThroughBall == EMatchPlayThroughBallActualBranch::AntiOffside) return false;
        // Once the public first contest is over, its counters do not carry into OneOnOne.
        if (Facts.FormulaContests.ContainsByPredicate([](const auto& C)
            { return C.ContestId == TEXT("ThroughBall.BehindDefense.P1") && (C.bHasResolvedFormula
                || C.Application == EMatchPlayResolutionFormulaApplication::SkippedByAuthoritativeGate); })) return false;
        return Counter.Participant == Role::Marker
            || (Counter.Attribute == TEXT("DEF") && Branch.ThroughBall == EMatchPlayThroughBallActualBranch::Feet)
            || (Counter.Attribute == TEXT("SPD") && Branch.ThroughBall == EMatchPlayThroughBallActualBranch::BehindDefense);
    default: return false;
    }
}
}

void FFMCodexDefensiveCardContext::Apply(const FFMCodexLocalMatchInteractionView& View, FFMCodexUMGMatchScreenViewModel& Model)
{
    TMap<FName,TArray<FName>> Matches;
    const bool bValidSide = View.CurrentAttackingPlayer == EInitialTurnOrderPlayer::PlayerA
        || View.CurrentAttackingPlayer == EInitialTurnOrderPlayer::PlayerB;
    if (bValidSide && View.bMatchActive && !View.bMatchEnded && View.bCurrentAttackActive
        && View.RouteKind == EMatchPlayCurrentAttackRouteKind::Ordinary && View.ActionPoint > 0
        && !View.bTerminalPendingAdvance && View.MajorPhase != EFMCodexLocalMatchMajorPhase::Complete)
    {
        const bool bA = View.CurrentAttackingPlayer == EInitialTurnOrderPlayer::PlayerA;
        const auto& Attackers = bA ? View.PlayerACardRoster : View.PlayerBCardRoster;
        const auto& Defenders = bA ? View.PlayerBCardRoster : View.PlayerACardRoster;
        const bool bAttackDeploymentLocked = bA ? View.bPlayerADeploymentFinished : View.bPlayerBDeploymentFinished;
        TSet<Skill> Families;
        if (!View.SelectedSkillId.IsNone())
            Families.Add(View.PresentedActionType);
        else
            for (const auto& Card : Attackers)
            {
                if (Card.bGoalkeeper || Card.bUsed || Card.bEjected || (!Card.bAvailable && !Card.bDeployed)
                    || (bAttackDeploymentLocked && !Card.bDeployed)
                    || (!View.SelectedCarrierCardId.IsNone() && Card.CardId != View.SelectedCarrierCardId)) continue;
                for (const auto& Match : Card.EligibleTacticalSkills)
                    Families.Add(FFMCodexAttackingCardContext::SkillType(Match.SkillId));
            }
        for (const auto& Card : Defenders)
        {
            if (Card.bGoalkeeper || Card.bUsed || Card.bEjected || (!Card.bAvailable && !Card.bDeployed)) continue;
            for (const auto& Counter : Counters())
            {
                if (!Families.Contains(Counter.Family) || !RouteAllows(Counter,View)) continue;
                const FName Bound = Counter.Participant == Role::Marker ? View.SelectedMarkerCardId : View.SelectedHelperCardId;
                const FName Other = Counter.Participant == Role::Marker ? View.SelectedHelperCardId : View.SelectedMarkerCardId;
                if ((!Bound.IsNone() && Bound != Card.CardId) || Other == Card.CardId) continue;
                // A published resolution bundle fixes participants, including an absent Helper.
                if (View.ResolutionFacts.bSuccess && View.ResolutionFacts.bHasFacts
                    && !View.ResolutionFacts.Participants.ContainsByPredicate([&](const auto& P)
                        { return P.CardId == Card.CardId && P.Role == Counter.Participant; })) continue;
                if (Card.RankedTraits.ContainsByPredicate([&](const auto& T)
                    { return T.TraitId == Counter.Id && T.Rank != EPlayerTraitRank::None; }))
                    Matches.FindOrAdd(Card.CardId).Add(Counter.Id);
            }
        }
    }
    auto ApplyCard = [&](FFMCodexUMGCardViewModel& Card)
    {
        Card.DefensiveCounterTraits = Matches.FindRef(Card.CardId);
    };
    for (auto& Cell : Model.LocalRack.Cells) ApplyCard(Cell.Card);
    for (auto& Cell : Model.OpponentRack.Cells) ApplyCard(Cell.Card);
    for (auto& Region : Model.PitchRegions) for (auto& Slot : Region.Slots) ApplyCard(Slot.Card);
    for (auto& Choice : Model.Interaction.DeploymentChoices) ApplyCard(Choice.Card);
    for (auto& Choice : Model.Interaction.SelectionChoices) if (Choice.bHasCard) ApplyCard(Choice.Card);
    for (auto& Card : Model.Interaction.CandidateCards) ApplyCard(Card);
}

FName FFMCodexDefensiveCardContext::Attribute(FName Trait)
{
    const auto* Counter = Counters().FindByPredicate([&](const auto& C){return C.Id == Trait;});
    return Counter ? Counter->Attribute : NAME_None;
}

FText FFMCodexDefensiveCardContext::CompactLabels(const TArray<FName>& Traits, TFunctionRef<bool(const FText&)> Fits,
    bool bAllowPartialPrefix)
{
    TArray<FText> Labels;
    for (const auto& Counter : Counters()) if (Traits.Contains(Counter.Id)) Labels.Add(Counter.Label);
    for (int32 Keep = Labels.Num(); Keep > 0; --Keep)
    {
        if (!bAllowPartialPrefix && Keep < Labels.Num()) break;
        TArray<FText> Visible;
        for (int32 I = 0; I < Keep; ++I) Visible.Add(Labels[I]);
        if (Keep < Labels.Num()) Visible.Add(FText::Format(NSLOCTEXT("FMCodexCounters","Remaining","+{0}"),FText::AsNumber(Labels.Num()-Keep)));
        const FText Text = FText::Join(FText::FromString(TEXT("·")),Visible);
        if (Fits(Text)) return Text;
    }
    if (Labels.IsEmpty()) return FText::GetEmpty();
    const FText Summary = FText::Format(NSLOCTEXT("FMCodexCounters","Count","防守{0}项"),FText::AsNumber(Labels.Num()));
    return !bAllowPartialPrefix || Fits(Summary) ? Summary
        : FText::Format(NSLOCTEXT("FMCodexCounters","Remaining","+{0}"),FText::AsNumber(Labels.Num()));
}
