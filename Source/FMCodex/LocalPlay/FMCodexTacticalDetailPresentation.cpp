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
		case ESkillRuleType::ThroughBall: return TEXT("脚下 / 身后 / 反越位");
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
		if (Id == TEXT("ThroughBall.BehindDefenseP1")) return TEXT("身后球 · 第一阶段");
		if (Id == TEXT("ThroughBall.BehindDefenseP2")) return TEXT("身后球 · 越位判定");
		if (Id == TEXT("ThroughBall.AntiOffside")) return TEXT("反越位");
		if (Id == TEXT("ThroughBall.OneOnOneDirect")) return TEXT("单刀 · 直接射门");
		if (Id == TEXT("ThroughBall.OneOnOneChip")) return TEXT("单刀 · 挑射");
		return FString();
	}

	FString RoleLabel(const EMatchPlayResolutionParticipantRole Role)
	{
		return FFMCodexPlayerUIPresentationText::ResolutionParticipantRole(Role)
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
}

FFMCodexUMGTacticalDetailViewModel
FFMCodexTacticalDetailPresentationBuilder::Build(
	const ESkillRuleType SkillType)
{
	using namespace FMCodexTacticalDetailPresentation;
	FFMCodexUMGTacticalDetailViewModel Result;
	const FTacticalRuleDescription* Description =
		FTacticalRuleDescriptionCatalog::FindBySkillType(SkillType);
	if (Description == nullptr)
	{
		return Result;
	}

	Result.bValid = true;
	Result.SkillType = SkillType;
	Result.DisplayName = SkillName(SkillType);
	Result.CardHint = CardHint(SkillType);

	for (const FTacticalRuleDescriptionBranch& Branch : Description->Branches)
	{
		FFMCodexUMGTacticalBranchViewModel& View =
			Result.Branches.AddDefaulted_GetRef();
		View.Label = BranchLabel(Branch.BranchId);
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
	return Result;
}
