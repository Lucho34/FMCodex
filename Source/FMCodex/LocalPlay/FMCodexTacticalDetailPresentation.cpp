#include "FMCodexTacticalDetailPresentation.h"

#include "FMCodexPlayerUIPresentationText.h"
#include "../CoreRules/TacticalRuleDescription.h"

namespace FMCodexTacticalDetailPresentation
{
	FString SkillName(const ESkillRuleType SkillType)
	{
		switch (SkillType)
		{
		case ESkillRuleType::LongShot: return TEXT("远射");
		case ESkillRuleType::CutInsideShot: return TEXT("内切");
		case ESkillRuleType::PassControl: return TEXT("控球推进");
		case ESkillRuleType::Cross: return TEXT("传中");
		case ESkillRuleType::ThroughBall: return TEXT("直塞");
		default: return FString();
		}
	}

	FString CardHint(const ESkillRuleType SkillType)
	{
		switch (SkillType)
		{
		case ESkillRuleType::LongShot: return TEXT("远射 / 死角");
		case ESkillRuleType::CutInsideShot: return TEXT("射门 · 盘带");
		case ESkillRuleType::PassControl: return TEXT("传球 / 盘带 / 跑动");
		case ESkillRuleType::Cross: return TEXT("高球 / 低球");
		case ESkillRuleType::ThroughBall:
			return TEXT("脚下球 · 身后球 · 反越位");
		default: return FString();
		}
	}

	FString BranchLabel(const FName Id)
	{
		if (Id == TEXT("LongShot.Direct") || Id == TEXT("CutInside.Direct"))
			return TEXT("直接射门");
		if (Id == TEXT("LongShot.DeadCorner") || Id == TEXT("CutInside.DeadCorner"))
			return TEXT("直射死角");
		if (Id == TEXT("PassControl.Pass")) return TEXT("传球推进");
		if (Id == TEXT("PassControl.Dribble")) return TEXT("盘带推进");
		if (Id == TEXT("PassControl.Run")) return TEXT("跑动推进");
		if (Id == TEXT("Cross.High")) return TEXT("高球传中");
		if (Id == TEXT("Cross.Low")) return TEXT("低球传中");
		if (Id == TEXT("ThroughBall.Feet")) return TEXT("脚下球");
		if (Id == TEXT("ThroughBall.BehindDefenseP1")) return TEXT("第一阶段");
		if (Id == TEXT("ThroughBall.AntiOffside")) return TEXT("越位判定");
		if (Id == TEXT("ThroughBall.OneOnOneDirect")) return TEXT("直接射门");
		if (Id == TEXT("ThroughBall.OneOnOneChip")) return TEXT("挑射");
		return FString();
	}

	TArray<FString> PrimaryRouteLabels(const FName Id)
	{
		if (Id == TEXT("ThroughBall.Feet")) return { TEXT("脚下球") };
		if (Id == TEXT("ThroughBall.BehindDefenseP1"))
		{
			return { TEXT("身后球") };
		}
		if (Id == TEXT("ThroughBall.AntiOffside"))
		{
			return { TEXT("反越位") };
		}
		if (Id == TEXT("ThroughBall.OneOnOneDirect")
			|| Id == TEXT("ThroughBall.OneOnOneChip"))
		{
			return { TEXT("身后球"), TEXT("反越位") };
		}
		return {};
	}

	FString RouteStepLabel(const FName Id)
	{
		if (Id == TEXT("ThroughBall.Feet")) return TEXT("属性对抗");
		if (Id == TEXT("ThroughBall.BehindDefenseP1")) return TEXT("第一阶段");
		if (Id == TEXT("ThroughBall.AntiOffside")) return TEXT("越位判定");
		if (Id == TEXT("ThroughBall.OneOnOneDirect"))
			return TEXT("直接射门");
		if (Id == TEXT("ThroughBall.OneOnOneChip"))
			return TEXT("挑射");
		return FString();
	}

	FString RouteStageLabel(const FName Id)
	{
		return Id == TEXT("ThroughBall.OneOnOneDirect")
			|| Id == TEXT("ThroughBall.OneOnOneChip")
				? TEXT("成功后：单刀")
				: FString();
	}

	FString RoleLabel(const EMatchPlayResolutionParticipantRole Role)
	{
		return FFMCodexPlayerUIPresentationText::TacticalDetailParticipantRole(Role)
			.ToString();
	}

	bool IsAttributeTerm(const FTacticalRuleDescriptionTerm& Term)
	{
		return (Term.Kind == EMatchPlayResolutionFormulaTermKind::Attribute
				|| Term.Kind
					== EMatchPlayResolutionFormulaTermKind::GoalkeeperContribution)
			&& Term.ParticipantRole
				!= EMatchPlayResolutionParticipantRole::None
			&& Term.Attribute != EMatchPlayResolutionFormulaAttribute::None;
	}

	void AddRoleAttribute(
		FFMCodexUMGTacticalBranchViewModel& View,
		const FTacticalRuleDescription& Description,
		const FTacticalRuleDescriptionTerm& Term)
	{
		if (!IsAttributeTerm(Term))
		{
			return;
		}
		FFMCodexUMGTacticalRoleAttributeViewModel* Row =
			View.RoleAttributes.FindByPredicate(
				[&Term](const FFMCodexUMGTacticalRoleAttributeViewModel& Candidate)
				{
					return Candidate.Role == Term.ParticipantRole;
				});
		if (Row == nullptr)
		{
			Row = &View.RoleAttributes.AddDefaulted_GetRef();
			Row->Role = Term.ParticipantRole;
			Row->RoleLabel = RoleLabel(Term.ParticipantRole);
			Row->bOptional = Description.OptionalRoles.Contains(
				Term.ParticipantRole);
		}
		Row->Attributes.AddUnique(Term.Attribute);
	}

	void FinalizeAttributeLabels(
		FFMCodexUMGTacticalBranchViewModel& View)
	{
		for (FFMCodexUMGTacticalRoleAttributeViewModel& Row
			: View.RoleAttributes)
		{
			TArray<FString> Labels;
			for (const EMatchPlayResolutionFormulaAttribute Attribute
				: Row.Attributes)
			{
				const FString Label = FFMCodexPlayerUIPresentationText::
					ResolutionAttribute(Attribute).ToString();
				if (!Label.IsEmpty())
				{
					Labels.Add(Label);
				}
			}
			Row.AttributeLabel = FString::Join(Labels, TEXT("、"));
		}
		View.RoleAttributes.RemoveAll(
			[](const FFMCodexUMGTacticalRoleAttributeViewModel& Row)
			{
				return Row.RoleLabel.IsEmpty() || Row.AttributeLabel.IsEmpty();
			});
	}

	FFMCodexUMGTacticalDetailViewModel BuildDetail(
		const ESkillRuleType SkillType,
		const FName OnlyBranchId = NAME_None)
	{
		FFMCodexUMGTacticalDetailViewModel Result;
		const FTacticalRuleDescription* Description =
			FTacticalRuleDescriptionCatalog::FindBySkillType(SkillType);
		if (Description == nullptr)
		{
			return Result;
		}

		Result.bValid = true;
		Result.SkillType = SkillType;
		Result.DisplayName = OnlyBranchId.IsNone()
			? SkillName(SkillType) : BranchLabel(OnlyBranchId);
		Result.CardHint = OnlyBranchId.IsNone()
			? CardHint(SkillType) : TEXT("选择前快速了解参与角色与属性");

		for (const FTacticalRuleDescriptionBranch& Branch : Description->Branches)
		{
			if (!OnlyBranchId.IsNone() && Branch.BranchId != OnlyBranchId)
			{
				continue;
			}
			TArray<FString> RouteLabels = OnlyBranchId.IsNone()
				? PrimaryRouteLabels(Branch.BranchId) : TArray<FString>{ FString() };
			if (RouteLabels.IsEmpty())
			{
				RouteLabels.Add(FString());
			}
			for (const FString& RouteLabel : RouteLabels)
			{
				FFMCodexUMGTacticalBranchViewModel& View =
					Result.Branches.AddDefaulted_GetRef();
				View.Label = BranchLabel(Branch.BranchId);
				View.PrimaryRouteLabel = RouteLabel;
				View.RouteStepLabel = OnlyBranchId.IsNone()
					? RouteStepLabel(Branch.BranchId) : FString();
				View.RouteStageLabel = OnlyBranchId.IsNone()
					? RouteStageLabel(Branch.BranchId) : FString();
				View.bRollOnly = Branch.RollSemantics
					!= EMatchPlayResolutionRollSemantics::ArithmeticContest;
				for (const FTacticalRuleDescriptionTerm& TermValue : Branch.AttackTerms)
				{
					AddRoleAttribute(View, *Description, TermValue);
				}
				for (const FTacticalRuleDescriptionTerm& TermValue : Branch.DefenseTerms)
				{
					AddRoleAttribute(View, *Description, TermValue);
				}
				FinalizeAttributeLabels(View);
			}
		}
		Result.bValid = Result.bValid && !Result.Branches.IsEmpty();
		return Result;
	}
}

FFMCodexUMGTacticalDetailViewModel
FFMCodexTacticalDetailPresentationBuilder::Build(
	const ESkillRuleType SkillType)
{
	using namespace FMCodexTacticalDetailPresentation;
	return BuildDetail(SkillType);
}

FFMCodexUMGTacticalDetailViewModel
FFMCodexTacticalDetailPresentationBuilder::BuildOneOnOneChoice(
	const EFMCodexUMGOneOnOneChoice Choice)
{
	using namespace FMCodexTacticalDetailPresentation;
	const FName BranchId = Choice == EFMCodexUMGOneOnOneChoice::DirectShot
		? FName(TEXT("ThroughBall.OneOnOneDirect"))
		: Choice == EFMCodexUMGOneOnOneChoice::ChipShot
			? FName(TEXT("ThroughBall.OneOnOneChip")) : NAME_None;
	return BranchId.IsNone()
		? FFMCodexUMGTacticalDetailViewModel()
		: BuildDetail(ESkillRuleType::ThroughBall, BranchId);
}

FFMCodexUMGOutcomeRollHintViewModel
FFMCodexTacticalDetailPresentationBuilder::BuildOutcomeRollHint(
	const ESkillRuleType SkillType, const FName BranchId)
{
	FFMCodexUMGOutcomeRollHintViewModel Result;
	const FTacticalRuleDescription* Description =
		FTacticalRuleDescriptionCatalog::FindBySkillType(SkillType);
	if (Description == nullptr)
	{
		return Result;
	}
	const FTacticalRuleDescriptionBranch* Branch =
		Description->Branches.FindByPredicate(
			[BranchId](const FTacticalRuleDescriptionBranch& Candidate)
			{
				return Candidate.BranchId == BranchId;
			});
	if (Branch == nullptr
		|| Branch->RollSemantics
			!= EMatchPlayResolutionRollSemantics::OutcomeDecision
		|| Branch->OutcomeRollCount != 1 || Branch->bOutcomeUsesRollTotal
		|| Branch->Outcomes.IsEmpty())
	{
		return Result;
	}

	TArray<FString> RangeLabels;
	for (const FTacticalRuleDescriptionOutcome& Outcome : Branch->Outcomes)
	{
		const FText OutcomeLabel = FFMCodexPlayerUIPresentationText::
			TacticalOutcome(BranchId, Outcome.OutcomeId);
		if (Outcome.Minimum <= 0 || Outcome.Maximum < Outcome.Minimum
			|| OutcomeLabel.IsEmpty())
		{
			return FFMCodexUMGOutcomeRollHintViewModel();
		}
		FFMCodexUMGOutcomeRollHintEntryViewModel& Entry =
			Result.Entries.AddDefaulted_GetRef();
		Entry.Minimum = Outcome.Minimum;
		Entry.Maximum = Outcome.Maximum;
		Entry.OutcomeId = Outcome.OutcomeId;
		Entry.DisplayLabel = FFMCodexPlayerUIPresentationText::
			TacticalOutcomeRange(
				Outcome.Minimum, Outcome.Maximum, OutcomeLabel).ToString();
		RangeLabels.Add(Entry.DisplayLabel);
	}
	Result.bVisible = !RangeLabels.IsEmpty();
	Result.BranchId = BranchId;
	Result.DisplayLabel = FString::Join(RangeLabels, TEXT("\u3000\uFF5C\u3000"));
	return Result;
}
