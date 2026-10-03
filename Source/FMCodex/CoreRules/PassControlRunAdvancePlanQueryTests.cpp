#include "PassControlRunAdvancePlanQuery.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"

// Stage 8.20 withdraws this formula. The prior positive composition contract
// is superseded by rejection even when callers supply complete legacy context.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPassControlRunAdvancePlanQueryUnavailableTest,
    "FMCodex.CoreRules.PassControlRunAdvancePlanQuery.Unavailable",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPassControlRunAdvancePlanQueryUnavailableTest::RunTest(const FString&)
{
    FPlayerCardRuleSnapshotSet Cards;
    FSkillRuleSnapshotSet Rules;
    FPassControlRunAdvancePlanQueryInput Input;
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
