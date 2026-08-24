#include "TacticalRuleDescription.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

namespace TacticalRuleDescriptionTests
{
	using EAttribute = EMatchPlayResolutionFormulaAttribute;
	using EKind = EMatchPlayResolutionFormulaTermKind;
	using ERole = EMatchPlayResolutionParticipantRole;
	using ESemantics = EMatchPlayResolutionRollSemantics;

	const FTacticalRuleDescriptionBranch* Branch(
		const FTacticalRuleDescription& Description,
		const FName BranchId)
	{
		return Description.Branches.FindByPredicate(
			[BranchId](const FTacticalRuleDescriptionBranch& Candidate)
			{
				return Candidate.BranchId == BranchId;
			});
	}

	bool HasTerm(
		const TArray<FTacticalRuleDescriptionTerm>& Terms,
		const EKind Kind,
		const ERole Role = ERole::None,
		const EAttribute Attribute = EAttribute::None,
		const float Multiplier = 1.0f,
		const int32 FixedModifier = 0)
	{
		return Terms.ContainsByPredicate(
			[=](const FTacticalRuleDescriptionTerm& Term)
			{
				return Term.Kind == Kind
					&& (Role == ERole::None || Term.ParticipantRole == Role)
					&& (Attribute == EAttribute::None || Term.Attribute == Attribute)
					&& FMath::IsNearlyEqual(Term.Multiplier, Multiplier)
					&& (Kind != EKind::FixedModifier
						|| Term.FixedModifier == FixedModifier);
			});
	}

	bool HasOutcome(
		const FTacticalRuleDescriptionBranch& BranchValue,
		const int32 Minimum,
		const int32 Maximum,
		const FName OutcomeId)
	{
		return BranchValue.Outcomes.ContainsByPredicate(
			[=](const FTacticalRuleDescriptionOutcome& Outcome)
			{
				return Outcome.Minimum == Minimum && Outcome.Maximum == Maximum
					&& Outcome.OutcomeId == OutcomeId;
			});
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FTacticalRuleDescriptionCompletenessTest,
	"FMCodex.CoreRules.TacticalRuleDescription.01.CompletenessAndStableIdentity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTacticalRuleDescriptionCompletenessTest::RunTest(const FString& Parameters)
{
	using namespace TacticalRuleDescriptionTests;
	const TArray<FTacticalRuleDescription>& All =
		FTacticalRuleDescriptionCatalog::GetAll();
	TestEqual(TEXT("Exactly five canonical tactical families"), All.Num(), 5);
	const TArray<ESkillRuleType> Expected = {
		ESkillRuleType::LongShot, ESkillRuleType::CutInsideShot,
		ESkillRuleType::PassControl, ESkillRuleType::Cross,
		ESkillRuleType::ThroughBall
	};
	TSet<uint8> StableTypes;
	for (const ESkillRuleType SkillType : Expected)
	{
		const FTacticalRuleDescription* Description =
			FTacticalRuleDescriptionCatalog::FindBySkillType(SkillType);
		TestNotNull(TEXT("Stable SkillType lookup succeeds"), Description);
		if (Description != nullptr)
		{
			StableTypes.Add(static_cast<uint8>(Description->SkillType));
			TestFalse(TEXT("Description has branches"),
				Description->Branches.IsEmpty());
			TestFalse(TEXT("Description has required participants"),
				Description->RequiredRoles.IsEmpty());
		}
	}
	TestEqual(TEXT("All stable identities are unique"), StableTypes.Num(), 5);
	TestNull(TEXT("None has no educational description"),
		FTacticalRuleDescriptionCatalog::FindBySkillType(ESkillRuleType::None));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FTacticalRuleDescriptionCanonicalSemanticsTest,
	"FMCodex.CoreRules.TacticalRuleDescription.02.CanonicalSemantics",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTacticalRuleDescriptionCanonicalSemanticsTest::RunTest(
	const FString& Parameters)
{
	using namespace TacticalRuleDescriptionTests;
	const auto* LongShot = FTacticalRuleDescriptionCatalog::FindBySkillType(
		ESkillRuleType::LongShot);
	const auto* CutInside = FTacticalRuleDescriptionCatalog::FindBySkillType(
		ESkillRuleType::CutInsideShot);
	const auto* PassControl = FTacticalRuleDescriptionCatalog::FindBySkillType(
		ESkillRuleType::PassControl);
	const auto* Cross = FTacticalRuleDescriptionCatalog::FindBySkillType(
		ESkillRuleType::Cross);
	const auto* ThroughBall = FTacticalRuleDescriptionCatalog::FindBySkillType(
		ESkillRuleType::ThroughBall);
	if (LongShot == nullptr || CutInside == nullptr || PassControl == nullptr
		|| Cross == nullptr || ThroughBall == nullptr)
	{
		AddError(TEXT("Catalog completeness prerequisite failed"));
		return false;
	}

	const auto* LongDirect = Branch(*LongShot, TEXT("LongShot.Direct"));
	const auto* LongDead = Branch(*LongShot, TEXT("LongShot.DeadCorner"));
	TestTrue(TEXT("Long Shot Direct uses LongShot/Tackling/+2/GK Positioning"),
		LongDirect != nullptr
			&& LongDirect->RollSemantics == ESemantics::ArithmeticContest
			&& HasTerm(LongDirect->AttackTerms, EKind::Attribute,
				ERole::Carrier, EAttribute::LongShot)
			&& HasTerm(LongDirect->DefenseTerms, EKind::Attribute,
				ERole::Marker, EAttribute::Tackling)
			&& HasTerm(LongDirect->DefenseTerms, EKind::FixedModifier,
				ERole::None, EAttribute::None, 1.0f, 2)
			&& HasTerm(LongDirect->DefenseTerms, EKind::GoalkeeperContribution,
				ERole::Goalkeeper, EAttribute::GoalkeeperPositioning, 0.5f)
			&& LongDirect->bUsesTacticalPlayerAdvantage);
	TestTrue(TEXT("Long Shot Dead Corner is paired outcome, not arithmetic"),
		LongDead != nullptr
			&& LongDead->RollSemantics == ESemantics::OutcomeDecision
			&& LongDead->OutcomeRollCount == 2 && LongDead->bOutcomeUsesRollTotal
			&& HasOutcome(*LongDead, 11, 12, TEXT("Goal")));

	const auto* CutDirect = Branch(*CutInside, TEXT("CutInside.Direct"));
	TestTrue(TEXT("Cut Inside Direct uses half Shooting/Dribbling and Handling"),
		CutDirect != nullptr
			&& HasTerm(CutDirect->AttackTerms, EKind::Attribute,
				ERole::Carrier, EAttribute::Shooting, 0.5f)
			&& HasTerm(CutDirect->AttackTerms, EKind::Attribute,
				ERole::Carrier, EAttribute::Dribbling, 0.5f)
			&& HasTerm(CutDirect->DefenseTerms, EKind::GoalkeeperContribution,
				ERole::Goalkeeper, EAttribute::GoalkeeperHandling, 0.5f));

	const auto* Pass = Branch(*PassControl, TEXT("PassControl.Pass"));
	const auto* Dribble = Branch(*PassControl, TEXT("PassControl.Dribble"));
	const auto* Run = Branch(*PassControl, TEXT("PassControl.Run"));
	TestTrue(TEXT("Pass Control documents all three distinct arithmetic branches"),
		Pass != nullptr && Dribble != nullptr && Run != nullptr
			&& PassControl->InitialRouteOutcomes.Num() == 3
			&& HasTerm(Pass->AttackTerms, EKind::Attribute,
				ERole::Carrier, EAttribute::Passing, 0.5f)
			&& HasTerm(Dribble->AttackTerms, EKind::Attribute,
				ERole::Carrier, EAttribute::Dribbling, 0.5f)
			&& HasTerm(Run->AttackTerms, EKind::Attribute,
				ERole::Carrier, EAttribute::OffBall, 0.5f)
			&& HasTerm(Run->DefenseTerms, EKind::Attribute,
				ERole::Marker, EAttribute::Marking, 0.5f));

	const auto* High = Branch(*Cross, TEXT("Cross.High"));
	const auto* Low = Branch(*Cross, TEXT("Cross.Low"));
	TestTrue(TEXT("Cross High/Low retain branch-specific roles, attributes and GK"),
		High != nullptr && Low != nullptr && Cross->InitialRouteOutcomes.Num() == 2
			&& Cross->OptionalRoles.Contains(ERole::Helper)
			&& HasTerm(High->AttackTerms, EKind::Attribute,
				ERole::Runner, EAttribute::Strength, 0.5f)
			&& HasTerm(High->DefenseTerms, EKind::GoalkeeperContribution,
				ERole::Goalkeeper, EAttribute::GoalkeeperAerial, 0.5f)
			&& HasTerm(Low->AttackTerms, EKind::Attribute,
				ERole::Runner, EAttribute::Shooting, 0.5f)
			&& HasTerm(Low->DefenseTerms, EKind::GoalkeeperContribution,
				ERole::Goalkeeper, EAttribute::GoalkeeperReflex, 0.5f));

	const auto* Feet = Branch(*ThroughBall, TEXT("ThroughBall.Feet"));
	const auto* BehindP1 = Branch(*ThroughBall, TEXT("ThroughBall.BehindDefenseP1"));
	const auto* BehindP2 = Branch(*ThroughBall, TEXT("ThroughBall.BehindDefenseP2"));
	const auto* AntiOffside = Branch(*ThroughBall, TEXT("ThroughBall.AntiOffside"));
	const auto* Direct = Branch(*ThroughBall, TEXT("ThroughBall.OneOnOneDirect"));
	const auto* Chip = Branch(*ThroughBall, TEXT("ThroughBall.OneOnOneChip"));
	TestTrue(TEXT("Through Ball distinguishes arithmetic, conditional and outcome branches"),
		Feet != nullptr && BehindP1 != nullptr && BehindP2 != nullptr
			&& AntiOffside != nullptr && Direct != nullptr && Chip != nullptr
			&& Feet->RollSemantics == ESemantics::ArithmeticContest
			&& BehindP1->bConditional && !BehindP1->bUsesTacticalPlayerAdvantage
			&& BehindP2->RollSemantics == ESemantics::OutcomeDecision
			&& HasOutcome(*BehindP2, 1, 3, TEXT("OneOnOne"))
			&& HasOutcome(*AntiOffside, 6, 6, TEXT("OneOnOne"))
			&& HasTerm(Direct->DefenseTerms, EKind::GoalkeeperContribution,
				ERole::Goalkeeper, EAttribute::GoalkeeperOneOnOne, 1.0f)
			&& HasOutcome(*Chip, 4, 6, TEXT("Goal")));
	return true;
}

#endif

