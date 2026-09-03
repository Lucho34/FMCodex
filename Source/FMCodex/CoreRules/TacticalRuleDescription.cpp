#include "TacticalRuleDescription.h"

namespace TacticalRuleDescription
{
	using EAttribute = EMatchPlayResolutionFormulaAttribute;
	using EKind = EMatchPlayResolutionFormulaTermKind;
	using ERole = EMatchPlayResolutionParticipantRole;
	using ESemantics = EMatchPlayResolutionRollSemantics;

	FTacticalRuleDescriptionTerm Attribute(
		const ERole Role,
		const EAttribute AttributeId,
		const float Multiplier = 1.0f,
		const bool bOptional = false,
		const EKind Kind = EKind::Attribute)
	{
		FTacticalRuleDescriptionTerm Result;
		Result.Kind = Kind;
		Result.ParticipantRole = Role;
		Result.Attribute = AttributeId;
		Result.Multiplier = Multiplier;
		Result.bOptional = bOptional;
		return Result;
	}

	FTacticalRuleDescriptionTerm Roll()
	{
		FTacticalRuleDescriptionTerm Result;
		Result.Kind = EKind::RawRoll;
		return Result;
	}

	FTacticalRuleDescriptionTerm Fixed(const int32 Value)
	{
		FTacticalRuleDescriptionTerm Result;
		Result.Kind = EKind::FixedModifier;
		Result.FixedModifier = Value;
		return Result;
	}

	FTacticalRuleDescriptionTerm TacticalPlayer()
	{
		FTacticalRuleDescriptionTerm Result;
		Result.Kind = EKind::TacticalPlayerAdvantage;
		return Result;
	}

	FTacticalRuleDescriptionOutcome Outcome(
		const int32 Minimum,
		const int32 Maximum,
		const TCHAR* OutcomeId)
	{
		return { Minimum, Maximum, FName(OutcomeId) };
	}

	FTacticalRuleDescriptionBranch Arithmetic(
		const TCHAR* BranchId,
		TArray<FTacticalRuleDescriptionTerm> AttackTerms,
		TArray<FTacticalRuleDescriptionTerm> DefenseTerms,
		const bool bUsesTacticalPlayer,
		const TCHAR* SpecialRuleId = TEXT(""),
		const bool bConditional = false)
	{
		FTacticalRuleDescriptionBranch Result;
		Result.BranchId = BranchId;
		Result.RollSemantics = ESemantics::ArithmeticContest;
		Result.AttackTerms = MoveTemp(AttackTerms);
		Result.DefenseTerms = MoveTemp(DefenseTerms);
		Result.bConditional = bConditional;
		Result.bUsesTacticalPlayerAdvantage = bUsesTacticalPlayer;
		Result.SpecialRuleId = SpecialRuleId;
		if (bUsesTacticalPlayer)
		{
			Result.AttackTerms.Add(TacticalPlayer());
			Result.DefenseTerms.Add(TacticalPlayer());
		}
		return Result;
	}

	FTacticalRuleDescriptionBranch Decision(
		const TCHAR* BranchId,
		TArray<FTacticalRuleDescriptionOutcome> Outcomes,
		const int32 RollCount = 1,
		const bool bUsesTotal = false,
		const TCHAR* SpecialRuleId = TEXT(""))
	{
		FTacticalRuleDescriptionBranch Result;
		Result.BranchId = BranchId;
		Result.RollSemantics = ESemantics::OutcomeDecision;
		Result.Outcomes = MoveTemp(Outcomes);
		Result.OutcomeRollCount = RollCount;
		Result.bOutcomeUsesRollTotal = bUsesTotal;
		Result.SpecialRuleId = SpecialRuleId;
		return Result;
	}

	FTacticalRuleDescription MakeLongShot()
	{
		FTacticalRuleDescription Result;
		Result.SkillType = ESkillRuleType::LongShot;
		Result.SummaryId = TEXT("Tactical.LongShot");
		Result.RequiredRoles = { ERole::Carrier, ERole::Marker };
		Result.AutomaticRoles = { ERole::Goalkeeper };
		Result.Branches.Add(Arithmetic(TEXT("LongShot.Direct"),
			{ Attribute(ERole::Carrier, EAttribute::LongShot), Roll() },
			{ Attribute(ERole::Marker, EAttribute::Tackling), Roll(), Fixed(2),
				Attribute(ERole::Goalkeeper, EAttribute::GoalkeeperPositioning,
					0.5f, true, EKind::GoalkeeperContribution) },
			true, TEXT("AttackRollOneTwoImmediateMiss")));
		Result.Branches.Add(Decision(TEXT("LongShot.DeadCorner"),
			{ Outcome(2, 10, TEXT("Miss")), Outcome(11, 12, TEXT("Goal")) },
			2, true, TEXT("AttackerRollsTwice")));
		return Result;
	}

	FTacticalRuleDescription MakeCutInside()
	{
		FTacticalRuleDescription Result;
		Result.SkillType = ESkillRuleType::CutInsideShot;
		Result.SummaryId = TEXT("Tactical.CutInside");
		Result.RequiredRoles = { ERole::Carrier, ERole::Marker };
		Result.AutomaticRoles = { ERole::Goalkeeper };
		Result.Branches.Add(Arithmetic(TEXT("CutInside.Direct"),
			{ Attribute(ERole::Carrier, EAttribute::Shooting, 0.5f),
				Attribute(ERole::Carrier, EAttribute::Dribbling, 0.5f), Roll() },
			{ Attribute(ERole::Marker, EAttribute::Tackling), Roll(), Fixed(2),
				Attribute(ERole::Goalkeeper, EAttribute::GoalkeeperHandling,
					0.5f, true, EKind::GoalkeeperContribution) },
			true, TEXT("AttackRollOneTwoImmediateMiss")));
		Result.Branches.Add(Decision(TEXT("CutInside.DeadCorner"),
			{ Outcome(2, 10, TEXT("Miss")), Outcome(11, 12, TEXT("Goal")) },
			2, true, TEXT("AttackerRollsTwice")));
		return Result;
	}

	TArray<FTacticalRuleDescriptionTerm> SharedControlDefense(
		const EAttribute MarkerAttribute)
	{
		return {
			Attribute(ERole::Marker, MarkerAttribute, 0.5f),
			Attribute(ERole::Helper, EAttribute::Marking, 0.5f, true),
			Roll(), Fixed(2),
			Attribute(ERole::Goalkeeper, EAttribute::GoalkeeperHandling,
				0.5f, true, EKind::GoalkeeperContribution)
		};
	}

	FTacticalRuleDescription MakePassControl()
	{
		FTacticalRuleDescription Result;
		Result.SkillType = ESkillRuleType::PassControl;
		Result.SummaryId = TEXT("Tactical.PassControl");
		Result.RequiredRoles = { ERole::Carrier, ERole::Runner, ERole::Marker };
		Result.OptionalRoles = { ERole::Helper };
		Result.AutomaticRoles = { ERole::Goalkeeper };
		Result.InitialRouteRuleId = TEXT("PassControl.Route");
		Result.InitialRouteOutcomes = {
			Outcome(1, 2, TEXT("PassControl.Pass")),
			Outcome(3, 4, TEXT("PassControl.Dribble")),
			Outcome(5, 6, TEXT("PassControl.Run"))
		};
		Result.Branches.Add(Arithmetic(TEXT("PassControl.Pass"),
			{ Attribute(ERole::Carrier, EAttribute::Passing, 0.5f),
				Attribute(ERole::Runner, EAttribute::Passing, 0.5f), Roll() },
			SharedControlDefense(EAttribute::Tackling), true));
		Result.Branches.Add(Arithmetic(TEXT("PassControl.Dribble"),
			{ Attribute(ERole::Carrier, EAttribute::Dribbling, 0.5f),
				Attribute(ERole::Runner, EAttribute::Passing, 0.5f), Roll() },
			SharedControlDefense(EAttribute::Tackling), true));
		Result.Branches.Add(Arithmetic(TEXT("PassControl.Run"),
			{ Attribute(ERole::Carrier, EAttribute::OffBall, 0.5f),
				Attribute(ERole::Runner, EAttribute::Dribbling, 0.5f), Roll() },
			SharedControlDefense(EAttribute::Marking), true));
		return Result;
	}

	FTacticalRuleDescription MakeCross()
	{
		FTacticalRuleDescription Result;
		Result.SkillType = ESkillRuleType::Cross;
		Result.SummaryId = TEXT("Tactical.Cross");
		Result.RequiredRoles = { ERole::Carrier, ERole::Runner, ERole::Marker };
		Result.OptionalRoles = { ERole::Helper };
		Result.AutomaticRoles = { ERole::Goalkeeper };
		Result.InitialRouteRuleId = TEXT("Cross.RoutePreference");
		Result.InitialRouteOutcomes = {
			Outcome(1, 4, TEXT("Cross.PreferredRoute")),
			Outcome(5, 6, TEXT("Cross.AlternateRoute"))
		};
		Result.Branches.Add(Arithmetic(TEXT("Cross.High"),
			{ Attribute(ERole::Carrier, EAttribute::Passing, 0.5f),
				Attribute(ERole::Runner, EAttribute::Strength, 0.5f), Roll() },
			{ Attribute(ERole::Marker, EAttribute::Tackling, 0.5f),
				Attribute(ERole::Helper, EAttribute::Strength, 0.5f, true),
				Roll(), Fixed(2),
				Attribute(ERole::Goalkeeper, EAttribute::GoalkeeperAerial,
					0.5f, true, EKind::GoalkeeperContribution) }, true));
		Result.Branches.Add(Arithmetic(TEXT("Cross.Low"),
			{ Attribute(ERole::Carrier, EAttribute::Passing, 0.5f),
				Attribute(ERole::Runner, EAttribute::Shooting, 0.5f), Roll() },
			{ Attribute(ERole::Marker, EAttribute::Tackling, 0.5f),
				Attribute(ERole::Helper, EAttribute::Marking, 0.5f, true),
				Roll(), Fixed(2),
				Attribute(ERole::Goalkeeper, EAttribute::GoalkeeperReflex,
					0.5f, true, EKind::GoalkeeperContribution) }, true));
		return Result;
	}

	FTacticalRuleDescription MakeThroughBall()
	{
		FTacticalRuleDescription Result;
		Result.SkillType = ESkillRuleType::ThroughBall;
		Result.SummaryId = TEXT("Tactical.ThroughBall");
		Result.RequiredRoles = { ERole::Carrier, ERole::Runner, ERole::Marker };
		Result.OptionalRoles = { ERole::Helper };
		Result.AutomaticRoles = { ERole::Goalkeeper };
		Result.InitialRouteRuleId = TEXT("ThroughBall.Route");
		Result.InitialRouteOutcomes = {
			Outcome(1, 2, TEXT("ThroughBall.Feet")),
			Outcome(3, 4, TEXT("ThroughBall.BehindDefenseP1")),
			Outcome(5, 6, TEXT("ThroughBall.AntiOffside"))
		};
		Result.Branches.Add(Arithmetic(TEXT("ThroughBall.Feet"),
			{ Attribute(ERole::Carrier, EAttribute::Passing, 0.5f),
				Attribute(ERole::Runner, EAttribute::OffBall, 0.5f), Roll() },
			{ Attribute(ERole::Marker, EAttribute::Tackling, 0.5f),
				Attribute(ERole::Helper, EAttribute::Marking, 0.5f, true),
				Roll(), Fixed(2),
				Attribute(ERole::Goalkeeper, EAttribute::GoalkeeperOneOnOne,
					0.5f, true, EKind::GoalkeeperContribution) }, true));
		Result.Branches.Add(Arithmetic(TEXT("ThroughBall.BehindDefenseP1"),
			{ Attribute(ERole::Carrier, EAttribute::Passing, 0.5f),
				Attribute(ERole::Runner, EAttribute::Speed, 0.5f), Roll() },
			{ Attribute(ERole::Marker, EAttribute::Marking, 0.5f),
				Attribute(ERole::Helper, EAttribute::Speed, 0.5f, true),
				Roll(), Fixed(1) }, false,
			TEXT("BehindDefenseP1Conditional"), true));
		Result.Branches.Add(Decision(TEXT("ThroughBall.AntiOffside"),
			{ Outcome(1, 5, TEXT("Offside")),
				Outcome(6, 6, TEXT("OneOnOne")) }, 1, false,
			TEXT("AttackerRoll")));
		Result.Branches.Add(Arithmetic(TEXT("ThroughBall.OneOnOneDirect"),
			{ Attribute(ERole::Runner, EAttribute::Shooting), Roll(), Fixed(1) },
			{ Attribute(ERole::Goalkeeper, EAttribute::GoalkeeperOneOnOne,
					1.0f, false, EKind::GoalkeeperContribution), Roll(),
				Attribute(ERole::Goalkeeper, EAttribute::GoalkeeperOneOnOne,
					0.5f, true, EKind::GoalkeeperContribution) }, true,
			TEXT("ActiveGoalkeeperAddsHalf")));
		Result.Branches.Add(Decision(TEXT("ThroughBall.OneOnOneChip"),
			{ Outcome(1, 3, TEXT("Miss")), Outcome(4, 6, TEXT("Goal")) },
			1, false, TEXT("AttackerRoll")));
		return Result;
	}

	TArray<FTacticalRuleDescription> BuildCatalog()
	{
		return {
			MakeLongShot(), MakeCutInside(), MakePassControl(), MakeCross(),
			MakeThroughBall()
		};
	}
}

const TArray<FTacticalRuleDescriptionOutcome>& FTacticalRuleDescriptionCatalog::GetCornerInitialRouteOutcomes()
{
	// Rules 13.1 / MatchPlayCornerResolution::SwitchRoute. Not a gameplay input.
	static const TArray<FTacticalRuleDescriptionOutcome> Outcomes = {
		TacticalRuleDescription::Outcome(1, 4, TEXT("Corner.PreferredRoute")),
		TacticalRuleDescription::Outcome(5, 6, TEXT("Corner.AlternateRoute")) };
	return Outcomes;
}

const FTacticalRuleDescriptionBranch* FTacticalRuleDescriptionCatalog::FindCornerRoute(
	const EMatchPlayCornerRouteIntent Route)
{
	using namespace TacticalRuleDescription;
	static const auto High = Arithmetic(TEXT("Corner.High"),
		{ Attribute(ERole::Runner, EAttribute::Strength), Roll() },
		{ Attribute(ERole::Helper, EAttribute::Strength, 0.5f),
			Attribute(ERole::Goalkeeper, EAttribute::GoalkeeperAerial, 0.5f), Fixed(2), Roll() },
		false, TEXT("CornerCandidateCountAdvantage"));
	static const auto Low = Arithmetic(TEXT("Corner.Low"),
		{ Attribute(ERole::Runner, EAttribute::Shooting), Roll() },
		{ Attribute(ERole::Helper, EAttribute::Marking, 0.5f),
			Attribute(ERole::Goalkeeper, EAttribute::GoalkeeperReflex, 0.5f), Fixed(2), Roll() },
		false, TEXT("CornerCandidateCountAdvantage"));
	return Route == EMatchPlayCornerRouteIntent::High ? &High
		: Route == EMatchPlayCornerRouteIntent::Low ? &Low : nullptr;
}

const TArray<FTacticalRuleDescription>&
FTacticalRuleDescriptionCatalog::GetAll()
{
	static const TArray<FTacticalRuleDescription> Catalog =
		TacticalRuleDescription::BuildCatalog();
	return Catalog;
}

const FTacticalRuleDescription*
FTacticalRuleDescriptionCatalog::FindBySkillType(
	const ESkillRuleType SkillType)
{
	return GetAll().FindByPredicate(
		[SkillType](const FTacticalRuleDescription& Candidate)
		{
			return Candidate.SkillType == SkillType;
		});
}
