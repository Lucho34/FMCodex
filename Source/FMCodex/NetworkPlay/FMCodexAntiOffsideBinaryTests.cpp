#if WITH_DEV_AUTOMATION_TESTS
#include "FMCodexNetworkThroughBallConditionalTestFixture.h"
#include "FMCodexNetworkMatchPresentation.h"
#include "../CoreRules/MatchPlayCurrentAttackResolveThroughBallAntiOffsideDecisionOrchestrator.h"

namespace AntiOffsideBinaryTests
{
using namespace FMCodexThroughBallConditionalTests;
const FName Expert(TEXT("Prototype.Arsenal.MikelMerino"));
bool Reach(FConditionalFixture& F, bool Binary, int32 Route = 5)
{
    F.Entropy->Word = 5;
    if (!F.ReachSkill(NAME_None, Binary ? Expert : NAME_None, true)) return false;
    const auto View = Access::Safe(*F.Mode, F.Attacker()->GetOwnerView().ViewerSide);
    const auto* Skill = View.SelectionOptions.FindByPredicate([](const auto& O) { return O.SkillType == ESkillRuleType::ThroughBall; });
    if (!Skill) return false;
    SkillPayload P; P.SkillId = Skill->Id;
    if (!F.Send(F.Attacker(), Kind::SubmitSkill, {}, {}, {}, {}, {}, P)) return false;
    F.Entropy->Word = Route - 1;
    return F.Send(F.Attacker(), Kind::ThroughBallInitialRouteRoll);
}
template<typename T> T RoundTrip(T Value)
{
    TArray<uint8> Bytes;
    FMemoryWriter Writer(Bytes); FObjectAndNameAsStringProxyArchive Save(Writer, false);
    T::StaticStruct()->SerializeItem(Save, &Value, nullptr);
    T Result; FMemoryReader Reader(Bytes); FObjectAndNameAsStringProxyArchive Load(Reader, false);
    T::StaticStruct()->SerializeItem(Load, &Result, nullptr);
    return Result;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAntiOffsideBinaryLifecycle,
    "FMCodex.NetworkPlay.AntiOffsideBinary.LifecycleAndProjection",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FAntiOffsideBinaryLifecycle::RunTest(const FString&)
{
    using namespace AntiOffsideBinaryTests;
    struct FCase { bool Binary; int32 A, B; };
    for (const FCase Case : {FCase{false, 3, 0}, {false, 6, 0}, {true, 6, 1}, {true, 3, 6}, {true, 6, 6}, {true, 5, 5}})
    {
        FConditionalFixture F;
        if (!TestTrue(TEXT("Production content and typed setup reach AntiOffside"), Reach(F, Case.Binary))) return false;
        const auto Before = Access::Session(*F.Mode).GetStateSnapshot();
        TestTrue(TEXT("Only the selected Runner authorizes the pair"),
            (Before.CurrentAttack.ResolutionSession.Bundle.Runner.CardId == Expert) == Case.Binary);
        // Merino remains a candidate in the authoritative roster even for ordinary runners.
        const int32 Calls = F.Entropy->Calls, Revision = Access::Revision(*F.Mode), Coordinator = F.Calls();
        F.Entropy->PendingWords = Case.Binary ? TArray<uint32>{uint32(Case.A - 1), uint32(Case.B - 1)} : TArray<uint32>{uint32(Case.A - 1)};
        const auto Request = F.For(Kind::ThroughBallAntiOffsideAttackRoll);
        TestEqual(TEXT("One typed command accepted"), F.Mode->SubmitConnectionPlayerIntent(F.Attacker(), Request).Code, Code::Accepted);
        TestEqual(TEXT("Exact authoritative sample count, including first six"), F.Entropy->Calls, Calls + (Case.Binary ? 2 : 1));
        TestEqual(TEXT("One publication"), Access::Revision(*F.Mode), Revision + 1);
        TestEqual(TEXT("One coordinator continuation"), F.Calls(), Coordinator + 1);
        const auto State = Access::Session(*F.Mode).GetStateSnapshot();
        const auto& Records = State.CurrentAttack.ResolutionSession.PostRouteRollProgress.RollRecords;
        if (!TestEqual(TEXT("One atomic event record"), Records.Num(), 1)) return false;
        TestEqual(TEXT("First persisted die"), Records[0].RawD6, Case.A);
        TestEqual(TEXT("Optional second persisted die"), Records[0].AntiOffsideSecondD6, Case.B);
        const bool Success = Case.A == 6 || Case.B == 6;
        TestEqual(TEXT("Same OneOnOne continuation"), !F.Attacker()->GetOwnerView().OneOnOneOptions.IsEmpty(), Success);
        TestEqual(TEXT("Same offside terminal"), F.Attacker()->GetOwnerView().bCanAdvance, !Success);
        const FFrozen Resolved(F);
        TestEqual(TEXT("Deduped same ID cannot reroll"), F.Mode->SubmitConnectionPlayerIntent(F.Attacker(), Request).Code, Code::DuplicateOrAlreadyResolved);
        Resolved.Verify(*this, F);
        TestEqual(TEXT("New ID at completed step rejected"), F.Roll(Kind::ThroughBallAntiOffsideAttackRoll).Code, Code::AuthorityRejected);
        Resolved.Verify(*this, F);
        auto Stale = F.For(Kind::ThroughBallAntiOffsideAttackRoll); ++Stale.ExpectedAttackSequence;
        TestEqual(TEXT("Stale sequence rejected"), F.Mode->SubmitConnectionPlayerIntent(F.Attacker(), Stale).Code, Code::StaleAttackSequence);
        Resolved.Verify(*this, F);

        const auto RebuiltState = RoundTrip(State);
        TestTrue(TEXT("Reflection persistence preserves entire state"), SameState(State, RebuiltState));
        FMatchPlayCurrentAttackResolveThroughBallAntiOffsideDecisionRequest Rebuild;
        Rebuild.AttackSequence = State.CurrentAttack.AttackSequence;
        Rebuild.Mode = decltype(Rebuild)::EMode::RegenerateCompletedDecision;
        const auto Replay = FMatchPlayCurrentAttackResolveThroughBallAntiOffsideDecisionOrchestrator::Resolve(
            RebuiltState, Rebuild, &Access::CallerRules(*F.Mode), nullptr);
        TestTrue(TEXT("Reconstruction needs no RNG provider"), Replay.bSuccess && Replay.ProviderCallCount == 0 && Replay.bReplayedCompleteRolls);
        TestEqual(TEXT("Reconstructed outcome"), Replay.OutcomeResult.bRequiresOneOnOne, Success);

        for (bool Reveal : {false, true})
        {
            const auto Safe = F.Safe(Reveal ? 1 : 0, Reveal);
            const auto DTO = RoundTrip(FFMCodexNetworkMatchPresentationAdapter::Project(Safe, Side::PlayerA));
            const auto* Event = DTO.ResolvedRolls.FindByPredicate([](const auto& E) { return E.ContestId == TEXT("ThroughBall.AntiOffside"); });
            TestEqual(TEXT("Event disclosure is atomic"), Event != nullptr, Reveal);
            if (Event) { TestEqual(TEXT("Safe first die"), Event->RawD6, Case.A); TestEqual(TEXT("Safe second die"), Event->AntiOffsideSecondD6, Case.B); }
            TestEqual(TEXT("Shared UI gets only disclosed pair"), DTO.ThroughBallSurface.AntiOffsideSecondD6, Reveal ? Case.B : 0);
            if (!Reveal) TestTrue(TEXT("No hidden Trait attribution"), DTO.ThroughBallSurface.AntiOffsideTraitHint.IsEmpty());
            TestTrue(TEXT("AntiOffside stays procedural"), Safe.ResolutionFacts.FormulaContests.IsEmpty());
        }
        const auto OpponentDTO = FFMCodexNetworkMatchPresentationAdapter::Project(
            FFMCodexLocalMatchInteractionViewBuilder::BuildForViewer(State, Access::CallerRules(*F.Mode), Side::PlayerB,
                [] { FFMCodexLocalMatchViewerDisclosure D; D.bRevealInitialActionPointRoll = D.bRevealRouteRoll = D.bRevealTerminalOutcome = true; D.RevealedContestD6Count = 1; return D; }()), Side::PlayerB);
        const auto* OpponentEvent = OpponentDTO.ResolvedRolls.FindByPredicate([](const auto& E) { return E.ContestId == TEXT("ThroughBall.AntiOffside"); });
        TestTrue(TEXT("Opponent receives same public dice"), OpponentEvent && OpponentEvent->RawD6 == Case.A && OpponentEvent->AntiOffsideSecondD6 == Case.B);
        if (Case.Binary && Success)
        {
            const bool Chip = Case.A == 6;
            TestEqual(TEXT("Existing single shot choice"), F.Roll(Kind::SubmitThroughBallOneOnOneShotChoice, 6, Chip ? Shot::ChipShot : Shot::DirectShot).Code, Code::Accepted);
            const int32 BeforeShot = F.Entropy->Calls;
            TestEqual(TEXT("Existing shot roll"), F.Roll(Chip ? Kind::ThroughBallOneOnOneChipShotAttackRoll : Kind::ThroughBallOneOnOneDirectShotAttackRoll, 3).Code, Code::Accepted);
            TestEqual(TEXT("Binary never enhances OneOnOne die count"), F.Entropy->Calls, BeforeShot + 1);
            if (!Chip) TestEqual(TEXT("Second-die success supports Direct defense continuation"), F.Roll(Kind::ThroughBallOneOnOneDirectShotDefenseRoll, 6).Code, Code::Accepted);
            TestTrue(TEXT("Downstream normal terminal"), F.Attacker()->GetOwnerView().bCanAdvance);
        }
    }
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAntiOffsideBinaryAtomicity,
    "FMCodex.NetworkPlay.AntiOffsideBinary.AtomicityAndIsolation",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FAntiOffsideBinaryAtomicity::RunTest(const FString&)
{
    using namespace AntiOffsideBinaryTests;
    FConditionalFixture F;
    if (!TestTrue(TEXT("Expert route"), Reach(F, true))) return false;
    for (bool Stale : {false, true})
    {
        const FFrozen Frozen(F);
        auto Request = F.For(Kind::ThroughBallAntiOffsideAttackRoll, Shot::None, Stale ? F.Attacker() : F.Defender());
        if (Stale) ++Request.ExpectedAttackSequence;
        TestEqual(TEXT("Unauthorized pair rejected before RNG"), F.Mode->SubmitConnectionPlayerIntent(Stale ? F.Attacker() : F.Defender(), Request).Code,
            Stale ? Code::StaleAttackSequence : Code::AuthorityRejected);
        Frozen.Verify(*this, F);
    }
    const FFrozen Frozen(F);
    F.Entropy->FailOnCall = F.Entropy->Calls + 2;
    const auto Request = F.For(Kind::ThroughBallAntiOffsideAttackRoll);
    TestEqual(TEXT("Second provider failure rejects whole event"), F.Mode->SubmitConnectionPlayerIntent(F.Attacker(), Request).Code, Code::AuthorityRejected);
    TestEqual(TEXT("Both sample attempts accounted"), F.Entropy->Calls, Frozen.Entropy + 2);
    TestTrue(TEXT("No partial authoritative record or state mutation"), SameState(Frozen.State, Access::Session(*F.Mode).GetStateSnapshot()));
    TestEqual(TEXT("No partial publication"), Access::Revision(*F.Mode), Frozen.Revision);
    TestEqual(TEXT("No continuation on provider failure"), F.Calls(), Frozen.Coordinator);
    const FFrozen Failed(F);
    F.Mode->SubmitConnectionPlayerIntent(F.Attacker(), Request);
    Failed.Verify(*this, F);
    F.Entropy->FailOnCall = 0;
    F.Entropy->PendingWords = {2, 5};
    TestEqual(TEXT("Explicit new request can retry failed transaction"), F.Roll(Kind::ThroughBallAntiOffsideAttackRoll).Code, Code::Accepted);
    TestTrue(TEXT("Fresh complete pair succeeds"), !F.Attacker()->GetOwnerView().OneOnOneOptions.IsEmpty());

    auto Malformed = Access::Session(*F.Mode).GetStateSnapshot();
    Malformed.CurrentAttack.ResolutionSession.PostRouteRollProgress.RollRecords[0].AntiOffsideSecondD6 = 0;
    TestFalse(TEXT("Persisted expert single die rejected"), FMatchPlayCurrentAttackResolutionSessionStateValidator::Validate(Malformed).bIsCanonical);
    for (int32 Route : {1, 3})
    {
        FConditionalFixture Other;
        if (!TestTrue(TEXT("Expert selected in other ThroughBall route"), Reach(Other, true, Route))) return false;
        const int32 Calls = Other.Entropy->Calls;
        const auto K = Route == 1 ? Kind::ThroughBallFeetAttackRoll : Kind::ThroughBallBehindDefenseP1AttackRoll;
        TestEqual(TEXT("Other route normal attack accepted"), Other.Roll(K, 6).Code, Code::Accepted);
        TestEqual(TEXT("Other route uses exactly one die"), Other.Entropy->Calls, Calls + 1);
        auto State = Access::Session(*Other.Mode).GetStateSnapshot();
        TestEqual(TEXT("No stale pair payload in other route"), State.CurrentAttack.ResolutionSession.PostRouteRollProgress.RollRecords[0].AntiOffsideSecondD6, 0);
        State.CurrentAttack.ResolutionSession.PostRouteRollProgress.RollRecords[0].AntiOffsideSecondD6 = 6;
        TestFalse(TEXT("Cross-route second die rejected"), FMatchPlayCurrentAttackResolutionSessionStateValidator::Validate(State).bIsCanonical);
    }
    return true;
}
#endif
