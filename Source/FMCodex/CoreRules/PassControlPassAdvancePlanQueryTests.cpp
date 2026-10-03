#include "PassControlPassAdvancePlanQuery.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"

// Stage 8.20 withdraws this formula. The prior positive composition contract
// is superseded by rejection even when callers supply complete legacy context.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPassControlPassAdvancePlanQueryUnavailableTest,
    "FMCodex.CoreRules.PassControlPassAdvancePlanQuery.Unavailable",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPassControlPassAdvancePlanQueryUnavailableTest::RunTest(const FString&)
{
    FPlayerCardRuleSnapshotSet Cards;
    FSkillRuleSnapshotSet Rules;
    FPassControlPassAdvancePlanQueryInput Input;
    const auto Result = FPassControlPassAdvancePlanQuery::BuildPlan(Cards, Rules, Input);
    TestFalse(TEXT("Withdrawn tactic cannot resolve"), Result.bSuccess);
    TestFalse(TEXT("No invented replacement formula"), Result.bHasFormulaPlan);
    TestEqual(TEXT("Explicit withdrawal error"), Result.ErrorCode,
        EPassControlPassAdvancePlanQueryErrorCode::TacticUnavailable);
    TestFalse(TEXT("Rejection explains unavailability"), Result.ErrorMessage.IsEmpty());
    TestEqual(TEXT("Diagnostic input retained"), Result.Input.SkillId, Input.SkillId);
    return true;
}
#endif
