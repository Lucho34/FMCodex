#include "FMCodexPlayerUIPresentationText.h"

#define LOCTEXT_NAMESPACE "FMCodexPlayerUIPresentationText"

namespace FMCodexPlayerUIPresentationText
{
	FText MapExact(const FString& Label)
	{
		if (Label == TEXT("Long Shot")) return LOCTEXT("SkillLongShot", "远射");
		if (Label == TEXT("Cut Inside")) return LOCTEXT("SkillCutInside", "内切");
		if (Label == TEXT("Pass Control")) return LOCTEXT("SkillPassControl", "控球推进");
		if (Label == TEXT("Cross")) return LOCTEXT("SkillCross", "传中");
		if (Label == TEXT("Through Ball")) return LOCTEXT("SkillThroughBall", "直塞");
		if (Label == TEXT("Direct Shot")) return LOCTEXT("ChoiceDirectShot", "直接射门");
		if (Label == TEXT("Dead Corner")) return LOCTEXT("ChoiceDeadCorner", "直射死角");
		if (Label == TEXT("Chip Shot")) return LOCTEXT("ChoiceChipShot", "挑射");
		if (Label == TEXT("Goal")) return LOCTEXT("TerminalGoal", "进球");
		if (Label == TEXT("Miss") || Label == TEXT("No Goal")) return LOCTEXT("TerminalNoGoal", "未进球");
		if (Label == TEXT("Offside")) return LOCTEXT("TerminalOffside", "越位");
		if (Label == TEXT("Out of Play")) return LOCTEXT("TerminalOutOfPlay", "出界");
		if (Label == TEXT("Defender Stopped Attack")) return LOCTEXT("TerminalDefenderStopped", "防守成功");
		return FText::FromString(Label);
	}

	FText MapRoleToken(const FString& Token)
	{
		if (Token == TEXT("FW")) return LOCTEXT("RoleForward", "前锋");
		if (Token == TEXT("MF")) return LOCTEXT("RoleMidfielder", "中场");
		if (Token == TEXT("DF")) return LOCTEXT("RoleDefender", "后卫");
		if (Token == TEXT("GK")) return LOCTEXT("RoleGoalkeeper", "门将");
		return FText::FromString(Token);
	}

	FText MapAttributeToken(const FString& Token)
	{
		if (Token == TEXT("SHO")) return LOCTEXT("AttributeShooting", "射门");
		if (Token == TEXT("DRI")) return LOCTEXT("AttributeDribbling", "盘带");
		if (Token == TEXT("PAS")) return LOCTEXT("AttributePassing", "传球");
		if (Token == TEXT("OFF")) return LOCTEXT("AttributeOffBall", "无球");
		if (Token == TEXT("MRK")) return LOCTEXT("AttributeMarking", "盯防");
		if (Token == TEXT("TKL")) return LOCTEXT("AttributeTackling", "抢断");
		if (Token == TEXT("SPD")) return LOCTEXT("AttributeSpeed", "速度");
		if (Token == TEXT("STR")) return LOCTEXT("AttributeStrength", "力量");
		if (Token == TEXT("STA")) return LOCTEXT("AttributeStamina", "体力");
		if (Token == TEXT("LS")) return LOCTEXT("AttributeLongShot", "远射");
		if (Token == TEXT("HAN")) return LOCTEXT("AttributeHandling", "手控球");
		if (Token == TEXT("POS")) return LOCTEXT("AttributePositioning", "站位");
		if (Token == TEXT("REF")) return LOCTEXT("AttributeReflexes", "反应");
		if (Token == TEXT("AER")) return LOCTEXT("AttributeAerial", "制空");
		if (Token == TEXT("ANT")) return LOCTEXT("AttributeAnticipation", "预判");
		if (Token == TEXT("1V1")) return LOCTEXT("AttributeOneOnOne", "单刀");
		return FText::FromString(Token);
	}
}

FText FFMCodexPlayerUIPresentationText::Role(const FString& CanonicalLabel)
{
	if (CanonicalLabel.IsEmpty() || CanonicalLabel == TEXT("ROLE N/A"))
	{
		return UnknownRole();
	}
	TArray<FString> Tokens;
	CanonicalLabel.ParseIntoArray(Tokens, TEXT("/"), true);
	TArray<FString> Localized;
	for (FString Token : Tokens)
	{
		Token.TrimStartAndEndInline();
		Localized.Add(FMCodexPlayerUIPresentationText::MapRoleToken(Token).ToString());
	}
	return FText::FromString(FString::Join(Localized, TEXT(" / ")));
}

FText FFMCodexPlayerUIPresentationText::Skill(const FString& CanonicalLabel)
{
	return CanonicalLabel.IsEmpty() || CanonicalLabel == TEXT("NO SKILL")
		? NoSkill() : FMCodexPlayerUIPresentationText::MapExact(CanonicalLabel);
}

FText FFMCodexPlayerUIPresentationText::Attribute(const FString& CanonicalEntry)
{
	FString Entry = CanonicalEntry;
	Entry.TrimStartAndEndInline();
	if (Entry.IsEmpty() || Entry == TEXT("Attributes unavailable"))
	{
		return AttributesUnavailable();
	}
	FString Token;
	FString Value;
	if (!Entry.Split(TEXT(" "), &Token, &Value, ESearchCase::CaseSensitive,
		ESearchDir::FromStart))
	{
		return FText::FromString(Entry);
	}
	return FText::Format(LOCTEXT("AttributeValueFormat", "{0} {1}"),
		FMCodexPlayerUIPresentationText::MapAttributeToken(Token.ToUpper()),
		FText::FromString(Value));
}

FText FFMCodexPlayerUIPresentationText::Status(const FString& CanonicalLabel)
{
	const FString Label = CanonicalLabel.ToUpper();
	if (Label == TEXT("AVAILABLE")) return LOCTEXT("StatusAvailable", "可用");
	if (Label == TEXT("DEPLOYED")) return LOCTEXT("StatusDeployed", "已部署");
	if (Label == TEXT("USED")) return LOCTEXT("StatusUsed", "已使用");
	if (Label == TEXT("ACTIVE")) return LOCTEXT("StatusActive", "已激活");
	if (Label == TEXT("UNAVAILABLE")) return Unavailable();
	return FText::FromString(CanonicalLabel);
}

FText FFMCodexPlayerUIPresentationText::Rarity(const FString& CanonicalLabel)
{
	if (CanonicalLabel.IsEmpty() || CanonicalLabel == TEXT("RARITY N/A")) return UnknownRarity();
	if (CanonicalLabel == TEXT("Common")) return LOCTEXT("RarityCommon", "普通");
	if (CanonicalLabel == TEXT("Club")) return LOCTEXT("RarityClub", "俱乐部");
	if (CanonicalLabel == TEXT("National")) return LOCTEXT("RarityNational", "国家级");
	if (CanonicalLabel == TEXT("Pilot")) return LOCTEXT("RarityPilot", "试制");
	return FText::FromString(CanonicalLabel);
}

FText FFMCodexPlayerUIPresentationText::Owner(const FString& CanonicalLabel)
{
	if (CanonicalLabel == TEXT("Player A")) return LOCTEXT("OwnerPlayerA", "玩家 A");
	if (CanonicalLabel == TEXT("Player B")) return LOCTEXT("OwnerPlayerB", "玩家 B");
	return CanonicalLabel.IsEmpty()
		? LOCTEXT("OwnerUnavailable", "玩家未知") : FText::FromString(CanonicalLabel);
}

FText FFMCodexPlayerUIPresentationText::UnknownCard() { return LOCTEXT("UnknownCard", "未知球员卡"); }
FText FFMCodexPlayerUIPresentationText::UnknownRole() { return LOCTEXT("UnknownRole", "位置未知"); }
FText FFMCodexPlayerUIPresentationText::UnknownRarity() { return LOCTEXT("UnknownRarity", "稀有度未知"); }
FText FFMCodexPlayerUIPresentationText::NoSkill() { return LOCTEXT("NoSkill", "无技能"); }
FText FFMCodexPlayerUIPresentationText::AttributesUnavailable() { return LOCTEXT("AttributesUnavailable", "属性不可用"); }
FText FFMCodexPlayerUIPresentationText::Unavailable() { return LOCTEXT("Unavailable", "不可用"); }
FText FFMCodexPlayerUIPresentationText::PortraitPlaceholder() { return LOCTEXT("PortraitPlaceholder", "球员肖像\n素材待接入"); }
FText FFMCodexPlayerUIPresentationText::SkillsHeading() { return LOCTEXT("SkillsHeading", "技能"); }
FText FFMCodexPlayerUIPresentationText::AttributesHeading() { return LOCTEXT("AttributesHeading", "属性"); }

#undef LOCTEXT_NAMESPACE
