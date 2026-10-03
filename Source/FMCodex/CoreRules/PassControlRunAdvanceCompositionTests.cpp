#include "PassControlRunAdvancePlanQuery.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"

// Stage 8.20 withdraws this formula. The prior positive composition contract
// is superseded by rejection even when callers supply complete legacy context.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPassControlRunAdvanceCompositionUnavailableTest,
    "FMCodex.CoreRules.PassControlRunAdvanceComposition.Unavailable",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPassControlRunAdvanceCompositionUnavailableTest::RunTest(const FString&)
{
    FPlayerCardRuleSnapshotSet Cards;
    FSkillRuleSnapshotSet Rules;
    FPassControlRunAdvancePlanQueryInput Input;
    Input.SkillId = TEXT("Withdrawn.PassControl");
    Input.CarrierCardId = TEXT("Carrier"); Input.RunnerCardId = TEXT("Runner");
    Input.MarkerCardId = TEXT("Marker"); Input.HelperCardId = TEXT("Helper");
    Input.bHasHelper = true; Input.CurrentActionPoint = 4;
    Input.AdvanceType = EPassControlAdvanceType::RunAdvance;
    Input.bHasExternalAttackD6 = Input.bHasExternalDefenseD6 = true;
    Input.ExternalAttackD6 = 6; Input.ExternalDefenseD6 = 1;
    Input.LogId = FGuid(1, 2, 3, 4); Input.TurnIndex = 1;
    Input.CarrierPlayerId = TEXT("A"); Input.RunnerPlayerId = TEXT("A");
    Input.MarkerPlayerId = TEXT("B"); Input.HelperPlayerId = TEXT("B");
    for (const FName Id : {Input.CarrierCardId, Input.RunnerCardId, Input.MarkerCardId, Input.HelperCardId})
    {
        FPlayerCardRuleSnapshot Card; Card.CardId = Id;
        Card.PositionTypes = {EPlayerPositionType::Midfield};
        Card.SkillIds = {Input.SkillId}; Cards.Cards.Add(Card);
    }
    FSkillRuleSnapshot Rule; Rule.SkillId = Input.SkillId;
    Rule.SkillType = ESkillRuleType::PassControl;
    Rule.MinTriggerActionPoint = 2; Rule.MaxTriggerActionPoint = 8;
    Rules.SkillRules.Add(Rule);
    const auto Result = FPassControlRunAdvancePlanQuery::BuildPlan(Cards, Rules, Input);
    TestFalse(TEXT("Withdrawn tactic cannot resolve"), Result.bSuccess);
    TestFalse(TEXT("No invented replacement formula"), Result.bHasFormulaPlan);
    TestEqual(TEXT("Explicit withdrawal error"), Result.ErrorCode,
        EPassControlRunAdvancePlanQueryErrorCode::TacticUnavailable);
    TestFalse(TEXT("Rejection explains unavailability"), Result.ErrorMessage.IsEmpty());
    TestEqual(TEXT("Diagnostic input retained"), Result.Input.SkillId, Input.SkillId);
    return true;
}
#endif
