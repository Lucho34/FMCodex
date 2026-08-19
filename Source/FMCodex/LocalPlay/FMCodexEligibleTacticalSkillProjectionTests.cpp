#include "FMCodexLocalMatchInteractionView.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "FMCodexLocalMatchDemoConfiguration.h"
#include "FMCodexLocalMatchResolutionFeedback.h"
#include "FMCodexLocalMatchUMGPresentation.h"
#include "FMCodexPrototypeTeamContent.h"

#include "../MatchPlayRuntime/MatchPlayAuthoritativeSession.h"

#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace FMCodexEligibleTacticalSkillProjectionTests
{
	FFMCodexLocalMatchCardView::FSkill MakeSkill(
		const TCHAR* SkillId,
		const TCHAR* Label,
		const int32 MinTacticalPoint,
		const int32 MaxTacticalPoint)
	{
		FFMCodexLocalMatchCardView::FSkill Result;
		Result.SkillId = FName(SkillId);
		Result.CanonicalLabel = Label;
		Result.MinTriggerActionPoint = MinTacticalPoint;
		Result.MaxTriggerActionPoint = MaxTacticalPoint;
		return Result;
	}

	const FFMCodexLocalMatchCardView* FindCard(
		const FFMCodexLocalMatchInteractionView& View,
		const FName CardId)
	{
		if (const FFMCodexLocalMatchCardView* Card =
			View.PlayerACardRoster.FindByPredicate(
				[CardId](const FFMCodexLocalMatchCardView& Candidate)
				{
					return Candidate.CardId == CardId;
				}))
		{
			return Card;
		}
		return View.PlayerBCardRoster.FindByPredicate(
			[CardId](const FFMCodexLocalMatchCardView& Candidate)
			{
				return Candidate.CardId == CardId;
			});
	}

	bool BuildAtTacticalPoint(
		const FFMCodexLocalMatchDemoConfiguration& Demo,
		const int32 TacticalPoint,
		FMatchPlayState& OutState,
		FFMCodexLocalMatchInteractionView& OutView)
	{
		FMatchPlayAuthoritativeSession Session;
		if (!Session.InitializeMatch(Demo.OpeningInput).OpeningResult.bSuccess
			|| !Session.BeginOrdinaryAttack(TacticalPoint).BeginResult.bSuccess)
		{
			return false;
		}
		OutState = Session.GetStateSnapshot();
		OutView = FFMCodexLocalMatchInteractionViewBuilder::Build(
			OutState, Demo.SkillRuleSet);
		return true;
	}

	bool LoadProjectSource(const TCHAR* RelativePath, FString& OutSource)
	{
		return FFileHelper::LoadFileToString(
			OutSource,
			*FPaths::Combine(FPaths::ProjectDir(), RelativePath));
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexEligibleTacticalSkillProjectionRangeTest,
	"FMCodex.LocalPlay.TacticalSkillProjection.01.RangeAndOrder",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexEligibleTacticalSkillProjectionRangeTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexEligibleTacticalSkillProjectionTests;
	const TArray<FFMCodexLocalMatchCardView::FSkill> OneSkill = {
		MakeSkill(TEXT("Rule.Range3To5"), TEXT("Range 3-5"), 3, 5)
	};
	const TArray<int32> ExpectedCounts = { 0, 1, 1, 1, 0 };
	for (int32 TacticalPoint = 2; TacticalPoint <= 6; ++TacticalPoint)
	{
		const TArray<FFMCodexLocalMatchCardView::FSkill> Eligible =
			FFMCodexLocalMatchInteractionViewBuilder::
				ProjectEligibleTacticalSkills(OneSkill, TacticalPoint);
		TestEqual(
			*FString::Printf(TEXT("Range 3-5 eligibility is inclusive at TP %d"),
				TacticalPoint),
			Eligible.Num(), ExpectedCounts[TacticalPoint - 2]);
	}

	const TArray<FFMCodexLocalMatchCardView::FSkill> MultipleSkills = {
		MakeSkill(TEXT("Rule.AuthoredFirst"), TEXT("Authored First"), 3, 4),
		MakeSkill(TEXT("Rule.AuthoredSecond"), TEXT("Authored Second"), 4, 5),
		MakeSkill(TEXT("Rule.AuthoredThird"), TEXT("Authored Third"), 6, 6)
	};
	TestTrue(TEXT("Zero owned Skills projects zero eligible Skills"),
		FFMCodexLocalMatchInteractionViewBuilder::
			ProjectEligibleTacticalSkills({}, 4).IsEmpty());
	TestEqual(TEXT("Multiple Skills fixture can project zero eligible Skills"),
		FFMCodexLocalMatchInteractionViewBuilder::
			ProjectEligibleTacticalSkills(MultipleSkills, 2).Num(), 0);
	TestEqual(TEXT("Multiple Skills fixture can project one eligible Skill"),
		FFMCodexLocalMatchInteractionViewBuilder::
			ProjectEligibleTacticalSkills(MultipleSkills, 3).Num(), 1);
	const TArray<FFMCodexLocalMatchCardView::FSkill> TwoEligible =
		FFMCodexLocalMatchInteractionViewBuilder::
			ProjectEligibleTacticalSkills(MultipleSkills, 4);
	TestTrue(TEXT("Multiple Skills fixture projects two in authored order"),
		TwoEligible.Num() == 2
			&& TwoEligible[0].SkillId == TEXT("Rule.AuthoredFirst")
			&& TwoEligible[1].SkillId == TEXT("Rule.AuthoredSecond"));
	TestTrue(TEXT("Projection does not clamp values outside authored domain"),
		FFMCodexLocalMatchInteractionViewBuilder::
			ProjectEligibleTacticalSkills(MultipleSkills, 1).IsEmpty()
			&& FFMCodexLocalMatchInteractionViewBuilder::
				ProjectEligibleTacticalSkills(MultipleSkills, 9).IsEmpty());
	TestEqual(TEXT("Projection leaves the static owned collection intact"),
		MultipleSkills.Num(), 3);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexEligibleTacticalSkillAuthorityProjectionTest,
	"FMCodex.LocalPlay.TacticalSkillProjection.02.AuthorityAndDTO",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexEligibleTacticalSkillAuthorityProjectionTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexEligibleTacticalSkillProjectionTests;
	const FFMCodexLocalMatchDemoConfiguration Demo =
		FFMCodexLocalMatchDemoConfigurationFactory::Create();
	const FFMCodexPrototypePlayerDefinition* SakaDefinition =
		FFMCodexPrototypeTeamContent::Find(
			TEXT("Prototype.Arsenal.BukayoSaka"));
	if (!TestNotNull(TEXT("Canonical Saka fixture exists"), SakaDefinition))
	{
		return false;
	}

	FMatchPlayState StateAtFour;
	FFMCodexLocalMatchInteractionView ViewAtFour;
	if (!TestTrue(TEXT("Authority accepts ordinary attack TP 4"),
		BuildAtTacticalPoint(Demo, 4, StateAtFour, ViewAtFour)))
	{
		return false;
	}
	TestTrue(TEXT("Projection source is the authoritative current attack"),
		StateAtFour.bHasCurrentAttack
			&& StateAtFour.CurrentAttack.ActionPoint == 4
			&& ViewAtFour.bCurrentAttackActive
			&& ViewAtFour.ActionPoint == 4);
	const FFMCodexLocalMatchCardView* SakaAtFour = FindCard(
		ViewAtFour, SakaDefinition->PlayerKey);
	if (!TestNotNull(TEXT("Saka card is projected at TP 4"), SakaAtFour))
	{
		return false;
	}
	TestTrue(TEXT("Static and eligible collections remain separate at TP 4"),
		SakaAtFour->Skills.Num() == 2
			&& SakaAtFour->EligibleTacticalSkills.Num() == 2
			&& SakaAtFour->EligibleTacticalSkills[0].SkillId
				== SakaDefinition->SkillAssignments[0].RuleId
			&& SakaAtFour->EligibleTacticalSkills[1].SkillId
				== SakaDefinition->SkillAssignments[1].RuleId);

	const FFMCodexUMGCardViewModel SakaUMG =
		FFMCodexLocalMatchUMGPresentationBuilder::BuildCard(*SakaAtFour);
	TestTrue(TEXT("UMG copies static and already-resolved eligible Skills"),
		SakaUMG.Skills.Num() == 2
			&& SakaUMG.EligibleTacticalSkills.Num() == 2
			&& SakaUMG.EligibleTacticalSkills[0].SkillId
				== SakaAtFour->EligibleTacticalSkills[0].SkillId);
	const FFMCodexUMGMatchScreenViewModel ScreenAtFour =
		FFMCodexLocalMatchUMGPresentationBuilder::Build(
			ViewAtFour,
			FFMCodexLocalMatchResolutionFeedback(),
			FString(),
			false,
			FString());
	TestTrue(TEXT("Header and Skill projection share the same authority path"),
		ScreenAtFour.Header.CurrentAttackerTacticalPoints == 4
			&& ScreenAtFour.Header.CurrentAttackerTacticalPointsLabel
				.Contains(TEXT("4")));

	const FFMCodexPrototypePlayerDefinition* SavinhoDefinition =
		FFMCodexPrototypeTeamContent::Find(
			TEXT("Prototype.ManchesterCity.Savinho"));
	const FFMCodexLocalMatchCardView* SavinhoAtFour = SavinhoDefinition == nullptr
		? nullptr : FindCard(ViewAtFour, SavinhoDefinition->PlayerKey);
	TestTrue(TEXT("The current attack TP applies consistently across both side rosters"),
		SavinhoAtFour != nullptr
			&& SavinhoAtFour->EligibleTacticalSkills.Num() == 2);

	FMatchPlayState StateAtFive;
	FFMCodexLocalMatchInteractionView ViewAtFive;
	if (!TestTrue(TEXT("Authority accepts a fresh ordinary attack TP 5"),
		BuildAtTacticalPoint(Demo, 5, StateAtFive, ViewAtFive)))
	{
		return false;
	}
	const FFMCodexLocalMatchCardView* SakaAtFive = FindCard(
		ViewAtFive, SakaDefinition->PlayerKey);
	TestTrue(TEXT("Authoritative TP change deterministically updates eligibility"),
		StateAtFive.CurrentAttack.ActionPoint == 5
			&& ViewAtFive.ActionPoint == 5
			&& SakaAtFive != nullptr
			&& SakaAtFive->Skills.Num() == 2
			&& SakaAtFive->EligibleTacticalSkills.Num() == 1
			&& SakaAtFive->EligibleTacticalSkills[0].SkillId
				== SakaDefinition->SkillAssignments[0].RuleId);

	FMatchPlayAuthoritativeSession BetweenAttacksSession;
	TestTrue(TEXT("Authority initializes a between-attacks state"),
		BetweenAttacksSession.InitializeMatch(Demo.OpeningInput)
			.OpeningResult.bSuccess);
	const FFMCodexLocalMatchInteractionView BetweenAttacksView =
		FFMCodexLocalMatchInteractionViewBuilder::Build(
			BetweenAttacksSession.GetStateSnapshot(), Demo.SkillRuleSet);
	const FFMCodexLocalMatchCardView* SakaBetweenAttacks = FindCard(
		BetweenAttacksView, SakaDefinition->PlayerKey);
	TestTrue(TEXT("No active attack preserves static Skills but projects no eligibility"),
		SakaBetweenAttacks != nullptr
			&& SakaBetweenAttacks->Skills.Num() == 2
			&& SakaBetweenAttacks->EligibleTacticalSkills.IsEmpty());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexEligibleTacticalSkillCanonicalInvariantTest,
	"FMCodex.LocalPlay.TacticalSkillProjection.03.Canonical40Invariant",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexEligibleTacticalSkillCanonicalInvariantTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexEligibleTacticalSkillProjectionTests;
	TArray<FString> ValidationErrors;
	TestTrue(TEXT("Canonical 40-player content validation passes"),
		FFMCodexPrototypeTeamContent::Validate(ValidationErrors));
	TestEqual(TEXT("Canonical validation has zero errors"),
		ValidationErrors.Num(), 0);
	const FFMCodexLocalMatchDemoConfiguration Demo =
		FFMCodexLocalMatchDemoConfigurationFactory::Create();
	int32 PlayerTacticalPointChecks = 0;
	for (int32 TacticalPoint = 2; TacticalPoint <= 8; ++TacticalPoint)
	{
		FMatchPlayState State;
		FFMCodexLocalMatchInteractionView View;
		if (!TestTrue(
			*FString::Printf(TEXT("Authority builds projection at TP %d"),
				TacticalPoint),
			BuildAtTacticalPoint(Demo, TacticalPoint, State, View)))
		{
			return false;
		}
		TArray<FFMCodexLocalMatchCardView> Cards = View.PlayerACardRoster;
		Cards.Append(View.PlayerBCardRoster);
		TestEqual(TEXT("Each TP projection contains all forty players"),
			Cards.Num(), 40);
		for (const FFMCodexLocalMatchCardView& Card : Cards)
		{
			TestTrue(TEXT("Projected eligible Skill count never exceeds two"),
				Card.EligibleTacticalSkills.Num() <= 2);
			for (const FFMCodexLocalMatchCardView::FSkill& Eligible
				: Card.EligibleTacticalSkills)
			{
				TestTrue(TEXT("Every eligible Skill remains an exact static Skill entry"),
					Card.Skills.ContainsByPredicate(
						[&Eligible](
							const FFMCodexLocalMatchCardView::FSkill& StaticSkill)
						{
							return StaticSkill.SkillId == Eligible.SkillId
								&& StaticSkill.MinTriggerActionPoint
									== Eligible.MinTriggerActionPoint
								&& StaticSkill.MaxTriggerActionPoint
									== Eligible.MaxTriggerActionPoint;
						}));
			}
			++PlayerTacticalPointChecks;
		}
	}
	TestEqual(TEXT("Full canonical overlap projection repeats 40 x 7 checks"),
		PlayerTacticalPointChecks, 280);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexEligibleTacticalSkillLayerContractTest,
	"FMCodex.LocalPlay.TacticalSkillProjection.04.LayerContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexEligibleTacticalSkillLayerContractTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexEligibleTacticalSkillProjectionTests;
	FString InteractionSource;
	FString UMGSource;
	FString CardWidgetSource;
	FString PitchSlotSource;
	TestTrue(TEXT("InteractionView source is readable"),
		LoadProjectSource(
			TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchInteractionView.cpp"),
			InteractionSource));
	TestTrue(TEXT("UMG presentation source is readable"),
		LoadProjectSource(
			TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchUMGPresentation.cpp"),
			UMGSource));
	TestTrue(TEXT("Player Card Widget source is readable"),
		LoadProjectSource(
			TEXT("Source/FMCodex/LocalPlay/FMCodexPlayerCardWidget.cpp"),
			CardWidgetSource));
	TestTrue(TEXT("Pitch Slot Widget source is readable"),
		LoadProjectSource(
			TEXT("Source/FMCodex/LocalPlay/FMCodexPitchSlotWidget.cpp"),
			PitchSlotSource));

	TestTrue(TEXT("Eligibility is evaluated in InteractionView from current attack authority"),
		InteractionSource.Contains(TEXT("State.CurrentAttack.ActionPoint"))
			&& InteractionSource.Contains(TEXT("ProjectEligibleTacticalSkills"))
			&& InteractionSource.Contains(
				TEXT("CurrentTacticalPoint >= Skill.MinTriggerActionPoint"))
			&& InteractionSource.Contains(
				TEXT("CurrentTacticalPoint <= Skill.MaxTriggerActionPoint")));
	TestTrue(TEXT("Projection reports the at-most-two invariant without truncating"),
		InteractionSource.Contains(TEXT("ensureAlwaysMsgf(Result.Num() <= 2"))
			&& !InteractionSource.Contains(TEXT("Result.SetNum(2)")));
	TestTrue(TEXT("UMG only copies the pre-resolved eligible collection"),
		UMGSource.Contains(TEXT("Card.EligibleTacticalSkills"))
			&& UMGSource.Contains(
				TEXT("Result.EligibleTacticalSkills.Add(MakeSkill(Skill))"))
			&& !UMGSource.Contains(TEXT("CurrentTacticalPoint")));
	TestTrue(TEXT("Pitch Mini and Pitch Slot perform no eligibility calculation"),
		!CardWidgetSource.Contains(TEXT("EligibleTacticalSkills"))
			&& !PitchSlotSource.Contains(TEXT("EligibleTacticalSkills")));
	TestTrue(TEXT("Full Card remains bound to the complete static Skills collection"),
		CardWidgetSource.Contains(
			TEXT("TArray<FFMCodexUMGSkillViewModel> Skills = Presentation.Skills"))
			&& CardWidgetSource.Contains(
				TEXT("PresentationMode == EFMCodexPlayerCardPresentationMode::PitchMini")));
	return true;
}

#endif
