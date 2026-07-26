#include "MatchPlaySkillParticipantRequirementQuery.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlaySkillParticipantRequirementQueryTest,
	"FMCodex.CoreRules.MatchPlaySkillParticipantRequirement.AllTypesAndUnknown",
	EAutomationTestFlags::EditorContext
		| EAutomationTestFlags::EngineFilter)

bool FMatchPlaySkillParticipantRequirementQueryTest::RunTest(
	const FString& Parameters)
{
	struct FCase
	{
		ESkillRuleType Type;
		bool bRunner;
		bool bHelper;
		bool bReady;
	};
	const FCase Cases[] = {
		{ESkillRuleType::LongShot, false, false, true},
		{ESkillRuleType::CutInsideShot, false, false, true},
		{ESkillRuleType::PassControl, true, true, false},
		{ESkillRuleType::Cross, true, true, false},
		{ESkillRuleType::ThroughBall, true, true, false}
	};

	for (const FCase& Case : Cases)
	{
		const auto First =
			FMatchPlaySkillParticipantRequirementQuery::Query(
				Case.Type);
		const auto Second =
			FMatchPlaySkillParticipantRequirementQuery::Query(
				Case.Type);
		TestTrue(TEXT("Supported type succeeds"), First.bSuccess);
		TestEqual(
			TEXT("RequiresRunner exact"),
			First.bRequiresRunner,
			Case.bRunner);
		TestEqual(
			TEXT("RequiresHelperStage exact"),
			First.bRequiresHelperStage,
			Case.bHelper);
		TestEqual(
			TEXT("CanBecomeReadyImmediately exact"),
			First.bCanBecomeReadyImmediately,
			Case.bReady);
		TestEqual(
			TEXT("Repeated query deterministic"),
			First.bSuccess,
			Second.bSuccess);
		TestEqual(
			TEXT("Repeated runner result deterministic"),
			First.bRequiresRunner,
			Second.bRequiresRunner);
	}

	const auto NoneResult =
		FMatchPlaySkillParticipantRequirementQuery::Query(
			ESkillRuleType::None);
	TestFalse(TEXT("None rejected"), NoneResult.bSuccess);
	TestEqual(
		TEXT("None structured error"),
		NoneResult.ErrorCode,
		EMatchPlaySkillParticipantRequirementErrorCode
			::UnsupportedSkillRuleType);

	const auto UnknownResult =
		FMatchPlaySkillParticipantRequirementQuery::Query(
			static_cast<ESkillRuleType>(255));
	TestFalse(TEXT("Unknown rejected"), UnknownResult.bSuccess);
	TestTrue(
		TEXT("Unknown diagnostics"),
		!UnknownResult.ErrorMessage.IsEmpty());
	return true;
}

#endif
