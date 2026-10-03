#include "FMCodexPrototypeTeamContent.h"
#include "FMCodexLocalMatchD6Provider.h"
#include "../CoreRules/MatchPlayCardSnapshotAuthority.h"
#include "../CoreRules/MatchPlayRecovery.h"
#include "../CoreRules/PlayerCardRuleSnapshotValidator.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexSimplifiedSnapshotTest,
    "FMCodex.LocalPlay.SimplifiedFoundation.PassiveSnapshotAndSchema",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexSimplifiedSnapshotTest::RunTest(const FString&)
{
    TArray<FPlayerCardData> A, B;
    A.SetNum(20); B.SetNum(20);
    FFMCodexPrototypeTeamContent::IntegrateIntoDemoDeck(EInitialTurnOrderPlayer::PlayerA, A);
    FFMCodexPrototypeTeamContent::IntegrateIntoDemoDeck(EInitialTurnOrderPlayer::PlayerB, B);
    const auto Built = FMatchPlayCardSnapshotAuthorityBuilder::Build(A, B);
    if (!TestTrue(TEXT("New canonical decks build authoritative snapshots"), Built.bSuccess)) return false;
    int32 Ranked = 0, Binary = 0;
    for (const auto& Definition : FFMCodexPrototypeTeamContent::GetDefinitions())
    {
        const auto Side = Definition.TeamId == FFMCodexPrototypeTeamContent::ArsenalTeamId()
            ? EInitialTurnOrderPlayer::PlayerA : EInitialTurnOrderPlayer::PlayerB;
        const auto Found = FMatchPlayCardSnapshotAuthorityQuery::FindByPlayerSideAndCardId(
            Built.CardSnapshotAuthority, Side, Definition.Card.CardId);
        TestTrue(TEXT("Every canonical snapshot is valid"), Found.bSuccess);
        TestTrue(TEXT("Passive Trait identity and rank survive authority copying"),
            Found.Snapshot.RankedTraits == Definition.Card.RankedTraits
            && Found.Snapshot.BinaryTraits == Definition.Card.BinaryTraits);
        TestEqual(TEXT("Semantic tier survives copying"), Found.Snapshot.Attributes.StaminaTier,
            Definition.Card.Attributes.StaminaTier);
        Ranked += Found.Snapshot.RankedTraits.Num(); Binary += Found.Snapshot.BinaryTraits.Num();
    }
    TestTrue(TEXT("Both approved passive payload kinds are loaded"), Ranked > 0 && Binary > 0);
    auto Invalid = Built.CardSnapshotAuthority.PlayerACardSnapshots;
    FPlayerRankedTrait Bad; Bad.TraitId = TEXT("Trait.Unknown"); Bad.Rank = EPlayerTraitRank::S;
    Invalid.Cards[1].RankedTraits.Add(Bad);
    TestFalse(TEXT("Unknown Trait cannot enter canonical snapshot"), FPlayerCardRuleSnapshotValidator::Validate(Invalid).bSuccess);
    Invalid = Built.CardSnapshotAuthority.PlayerACardSnapshots;
    Invalid.Cards[1].Attributes.Control = 7;
    TestFalse(TEXT("Base remains 1-6 despite future effective operands"), FPlayerCardRuleSnapshotValidator::Validate(Invalid).bSuccess);
    Invalid = Built.CardSnapshotAuthority.PlayerACardSnapshots;
    Invalid.Cards[1].Attributes.StaminaTier = EPlayerStaminaTier::None;
    TestFalse(TEXT("Outfield tier is required"), FPlayerCardRuleSnapshotValidator::Validate(Invalid).bSuccess);
    for (const FName OldField : {FName(TEXT("LongShot")), FName(TEXT("Tackling")), FName(TEXT("Marking")),
        FName(TEXT("Dribbling")), FName(TEXT("OffBall")), FName(TEXT("Stamina"))})
        TestNull(TEXT("Removed base field has no reflection alias"), FPlayerAttributes::StaticStruct()->FindPropertyByName(OldField));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexSimplifiedRecoveryTest,
    "FMCodex.LocalPlay.SimplifiedFoundation.TierWeightedRecovery",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexSimplifiedRecoveryTest::RunTest(const FString&)
{
    FMatchPlayPerSideCardSnapshotAuthority Authority;
    FMatchCardUsageState Usage;
    for (const auto Tier : {EPlayerStaminaTier::S, EPlayerStaminaTier::A, EPlayerStaminaTier::B})
    {
        FPlayerCardRuleSnapshot Card; Card.CardId = FName(PlayerStaminaTierLabel(Tier));
        Card.PositionTypes = {EPlayerPositionType::Attack}; Card.Attributes.StaminaTier = Tier;
        Authority.PlayerACardSnapshots.Cards.Add(Card);
        Usage.PlayerACardUsageState.UsedCardIds.Add(Card.CardId);
    }
    const auto Pool = FMatchPlayRecoveryCandidateQuery::Build(Usage, Authority);
    if (!TestTrue(TEXT("Three eligible tier candidates"), Pool.bSuccess && Pool.Candidates.Num() == 3)) return false;
    TestEqual(TEXT("S weight"), Pool.Candidates[0].StaminaWeight, 5);
    TestEqual(TEXT("A weight"), Pool.Candidates[1].StaminaWeight, 3);
    TestEqual(TEXT("B weight"), Pool.Candidates[2].StaminaWeight, 1);
    TestEqual(TEXT("This pool totals nine"), Pool.Candidates[0].StaminaWeight
        + Pool.Candidates[1].StaminaWeight + Pool.Candidates[2].StaminaWeight, 9);
    for (int32 Seed = 1; Seed <= 24; ++Seed)
    {
        FFMCodexLocalMatchD6Provider Provider(Seed);
        const auto Draw = Provider.DrawWeightedWithoutReplacement(
            EMatchPlayRecoveryPurpose::ConsumedRecovery, Pool.Candidates, 2);
        if (!TestTrue(TEXT("Weighted pair succeeds"), Draw.bSuccess && Draw.SelectedCandidateIndices.Num() == 2)) return false;
        FRandomStream Tickets(Seed);
        const int32 FirstTicket = Tickets.RandRange(1, 9);
        const int32 First = FirstTicket <= 5 ? 0 : FirstTicket <= 8 ? 1 : 2;
        TestEqual(TEXT("First draw uses the tier-weighted intervals"), Draw.SelectedCandidateIndices[0], First);
        const int32 RemainingTotal = 9 - Pool.Candidates[First].StaminaWeight;
        const int32 SecondTicket = Tickets.RandRange(1, RemainingTotal);
        const int32 Left = First == 0 ? 1 : 0;
        const int32 Right = First == 2 ? 1 : 2;
        TestEqual(TEXT("Next draw renormalizes only the remaining two candidates"), Draw.SelectedCandidateIndices[1],
            SecondTicket <= Pool.Candidates[Left].StaminaWeight ? Left : Right);
        TestTrue(TEXT("No replacement"), Draw.SelectedCandidateIndices[0] != Draw.SelectedCandidateIndices[1]);
    }
    return true;
}
#endif
