#include "FMCodexPlayerUIPresentationText.h"

#include "FMCodexPrototypeTeamContent.h"

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
		if (Token == TEXT("OFF")) return LOCTEXT("AttributeOffBall", "跑位");
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

FText FFMCodexPlayerUIPresentationText::PlayerName(
	const FName CardId,
	const FString& FallbackLabel)
{
	const FText PrototypeName =
		FFMCodexPrototypeTeamContent::PlayerDisplayName(CardId);
	if (!PrototypeName.IsEmpty())
	{
		return PrototypeName;
	}
	if (FallbackLabel.StartsWith(TEXT("Card "))
		|| FallbackLabel.Contains(TEXT("Demo."))
		|| FallbackLabel.Contains(TEXT("Prototype."))
		|| FallbackLabel.Contains(TEXT("CardId"))
		|| FallbackLabel.Equals(TEXT("UNKNOWN CARD"), ESearchCase::IgnoreCase)
		|| FallbackLabel.Contains(TEXT("reference"), ESearchCase::IgnoreCase))
	{
		// Hand-Micro-only aliases are visual validation labels, not canonical
		// Full Card identity data. Missing player-facing identity stays omitted.
		return FText::GetEmpty();
	}
	return FallbackLabel.IsEmpty()
		? FText::GetEmpty() : FText::FromString(FallbackLabel);
}

FText FFMCodexPlayerUIPresentationText::InMatchShortPlayerName(
	const FName CardId,
	const FString& FallbackLabel)
{
	// The production roster resolves its explicit presentation name from data.
	// PlayerName also supplies the defensive non-production fallback contract.
	return PlayerName(CardId, FallbackLabel);
}

FText FFMCodexPlayerUIPresentationText::MatchScreenLabel(
	const FString& CanonicalLabel)
{
	if (CanonicalLabel.IsEmpty()) return FText::GetEmpty();
	if (CanonicalLabel == TEXT("LOCAL MATCH")) return LOCTEXT("MatchLocal", "\u672C\u5730\u5BF9\u6218");
	if (CanonicalLabel == TEXT("LIVE MATCH")) return LOCTEXT("MatchLive", "\u6BD4\u8D5B\u8FDB\u884C\u4E2D");
	if (CanonicalLabel == TEXT("PLAYER ACTION") || CanonicalLabel == TEXT("HUMAN INPUT")) return LOCTEXT("PlayerAction", "\u73A9\u5BB6\u64CD\u4F5C");
	if (CanonicalLabel == TEXT("SYSTEM") || CanonicalLabel == TEXT("SYSTEM RESOLUTION")) return LOCTEXT("SystemResolution", "\u7CFB\u7EDF\u7ED3\u7B97");
	if (CanonicalLabel == TEXT("INFORMATION")) return LOCTEXT("Information", "\u4FE1\u606F");
	if (CanonicalLabel == TEXT("MATCH COMPLETE") || CanonicalLabel == TEXT("MATCH ENDED")) return LOCTEXT("MatchComplete", "\u6BD4\u8D5B\u7ED3\u675F");
	if (CanonicalLabel == TEXT("MATCH IN PROGRESS")) return LOCTEXT("MatchInProgress", "\u6BD4\u8D5B\u8FDB\u884C\u4E2D");
	if (CanonicalLabel == TEXT("READY TO PLAY")) return LOCTEXT("ReadyToPlay", "\u51C6\u5907\u5F00\u59CB");
	if (CanonicalLabel == TEXT("Start a Local Match") || CanonicalLabel == TEXT("START LOCAL MATCH")) return LOCTEXT("StartLocalMatch", "\u5F00\u59CB\u672C\u5730\u5BF9\u6218");
	if (CanonicalLabel == TEXT("Start Match")) return LOCTEXT("StartMatch", "\u5F00\u59CB\u6BD4\u8D5B");
	if (CanonicalLabel == TEXT("Roll Tactical Points") || CanonicalLabel == TEXT("ROLL TACTICAL POINTS") || CanonicalLabel == TEXT("Tactical Point Roll")) return LOCTEXT("RollTacticalPoints", "\u63B7\u6218\u672F\u70B9");
	if (CanonicalLabel == TEXT("Deployment")) return LOCTEXT("DeploymentPhase", "\u90E8\u7F72\u9636\u6BB5");
	if (CanonicalLabel == TEXT("Selection")) return LOCTEXT("SelectionPhase", "\u9009\u62E9\u9636\u6BB5");
	if (CanonicalLabel == TEXT("Resolution")) return LOCTEXT("ResolutionPhase", "\u7ED3\u7B97\u9636\u6BB5");
	if (CanonicalLabel == TEXT("Deploy Your Cards")) return LOCTEXT("DeployCards", "\u90E8\u7F72\u7403\u5458");
	if (CanonicalLabel == TEXT("FINISH DEPLOYMENT") || CanonicalLabel == TEXT("Deploy / Finish Deployment")) return LOCTEXT("FinishDeployment", "\u5B8C\u6210\u90E8\u7F72");
	if (CanonicalLabel == TEXT("TACTICAL REFERENCE")) return LOCTEXT("TacticalReference", "\u6218\u672F\u8BF4\u660E");
	if (CanonicalLabel == TEXT("CLOSE TACTICAL REFERENCE")) return LOCTEXT("CloseTacticalReference", "\u5173\u95ED\u6218\u672F\u8BF4\u660E");
	if (CanonicalLabel == TEXT("Select Carrier")) return LOCTEXT("SelectCarrier", "\u9009\u62E9\u6301\u7403\u7403\u5458");
	if (CanonicalLabel == TEXT("Click a player on the pitch")) return LOCTEXT("ClickPitchPlayer", "\u70B9\u51FB\u573A\u4E0A\u7403\u5458\u9009\u62E9");
	if (CanonicalLabel == TEXT("Select Marker")) return LOCTEXT("SelectMarker", "\u9009\u62E9\u76EF\u4EBA\u7403\u5458");
	if (CanonicalLabel == TEXT("DECLINE MARKER") || CanonicalLabel == TEXT("Decline Marker")) return LOCTEXT("DeclineMarker", "\u653E\u5F03\u76EF\u4EBA");
	if (CanonicalLabel == TEXT("Choose Skill") || CanonicalLabel == TEXT("Select Skill")) return LOCTEXT("ChooseTactical", "\u9009\u62E9\u6218\u672F");
	if (CanonicalLabel == TEXT("DECLINE SKILL") || CanonicalLabel == TEXT("Decline Skill")) return LOCTEXT("DeclineTactical", "\u4E0D\u4F7F\u7528\u6218\u672F");
	if (CanonicalLabel == TEXT("Select Runner")) return LOCTEXT("SelectRunner", "\u9009\u62E9\u8DD1\u4F4D\u7403\u5458");
	if (CanonicalLabel == TEXT("DECLINE RUNNER") || CanonicalLabel == TEXT("Decline Runner")) return LOCTEXT("DeclineRunner", "\u4E0D\u9009\u62E9\u8DD1\u4F4D\u7403\u5458");
	if (CanonicalLabel == TEXT("Select Helper")) return LOCTEXT("SelectHelper", "\u9009\u62E9\u534F\u9632\u7403\u5458");
	if (CanonicalLabel == TEXT("DECLINE HELPER") || CanonicalLabel == TEXT("Decline Helper")) return LOCTEXT("DeclineHelper", "\u653E\u5F03\u534F\u9632");
	if (CanonicalLabel == TEXT("Choose Cross Type")) return LOCTEXT("ChooseCrossType", "\u9009\u62E9\u4F20\u4E2D\u65B9\u5F0F");
	if (CanonicalLabel == TEXT("Choose Shot Type")) return LOCTEXT("ChooseShotType", "\u9009\u62E9\u5C04\u95E8\u65B9\u5F0F");
	if (CanonicalLabel == TEXT("Choose One-on-One Shot")) return LOCTEXT("ChooseOneOnOneShot", "\u9009\u62E9\u5355\u5200\u5C04\u95E8");
	if (CanonicalLabel == TEXT("Continue Resolution") || CanonicalLabel == TEXT("CONTINUE")) return LOCTEXT("Continue", "\u7EE7\u7EED");
	if (CanonicalLabel.StartsWith(TEXT("Continue -"))) return LOCTEXT("ContinueStep", "\u7EE7\u7EED\u7ED3\u7B97");
	if (CanonicalLabel == TEXT("No player action is available.")) return LOCTEXT("NoPlayerAction", "\u5F53\u524D\u65E0\u73A9\u5BB6\u64CD\u4F5C");
	if (CanonicalLabel == TEXT("Attack Complete") || CanonicalLabel == TEXT("ATTACK COMPLETE")) return LOCTEXT("AttackComplete", "\u8FDB\u653B\u7ED3\u675F");
	if (CanonicalLabel == TEXT("Unable to Load Match") || CanonicalLabel == TEXT("Interaction unavailable") || CanonicalLabel == TEXT("Interaction unavailable.")) return LOCTEXT("InteractionUnavailable", "\u5F53\u524D\u65E0\u53EF\u7528\u64CD\u4F5C");
	if (CanonicalLabel == TEXT("LEGAL OPTIONS")) return LOCTEXT("LegalOptions", "\u53EF\u9009\u64CD\u4F5C");
	if (CanonicalLabel == TEXT("CROSS TYPE")) return LOCTEXT("CrossType", "\u4F20\u4E2D\u65B9\u5F0F");
	if (CanonicalLabel == TEXT("SHOT TYPE")) return LOCTEXT("ShotType", "\u5C04\u95E8\u65B9\u5F0F");
	if (CanonicalLabel == TEXT("ONE-ON-ONE | CHOOSE SHOT")) return LOCTEXT("OneOnOneChooseShot", "\u5355\u5200\uFF5C\u9009\u62E9\u5C04\u95E8");
	if (CanonicalLabel == TEXT("Direct Shot")) return LOCTEXT("DirectShot", "\u76F4\u63A5\u5C04\u95E8");
	if (CanonicalLabel == TEXT("Dead Corner")) return LOCTEXT("DeadCorner", "\u5C04\u5411\u6B7B\u89D2");
	if (CanonicalLabel == TEXT("Cross High")) return LOCTEXT("CrossHigh", "\u9AD8\u7403\u4F20\u4E2D");
	if (CanonicalLabel == TEXT("Cross Low")) return LOCTEXT("CrossLow", "\u4F4E\u5E73\u7403\u4F20\u4E2D");
	if (CanonicalLabel == TEXT("Chip Shot")) return LOCTEXT("ChipShot", "\u6311\u5C04");
	if (CanonicalLabel == TEXT("RESOLVE NO LEGAL CARRIER")) return LOCTEXT("NoLegalCarrier", "\u65E0\u53EF\u7528\u6301\u7403\u7403\u5458\uFF0C\u7EE7\u7EED\u7ED3\u7B97");
	if (CanonicalLabel == TEXT("RESOLVE NO LEGAL MARKER")) return LOCTEXT("NoLegalMarker", "\u65E0\u53EF\u7528\u76EF\u4EBA\u7403\u5458\uFF0C\u7EE7\u7EED\u7ED3\u7B97");
	if (CanonicalLabel == TEXT("RESOLVE NO LEGAL SKILL")) return LOCTEXT("NoLegalTactical", "\u65E0\u53EF\u7528\u6218\u672F\uFF0C\u7EE7\u7EED\u7ED3\u7B97");
	if (CanonicalLabel == TEXT("RESOLVE NO LEGAL RUNNER")) return LOCTEXT("NoLegalRunner", "\u4E0D\u9009\u62E9\u8DD1\u4F4D\u7403\u5458");
	if (CanonicalLabel == TEXT("RESOLVE NO LEGAL HELPER")) return LOCTEXT("NoLegalHelper", "\u65E0\u53EF\u7528\u534F\u9632\u7403\u5458\uFF0C\u7EE7\u7EED\u7ED3\u7B97");
	if (CanonicalLabel == TEXT("MATCH COMPLETE | No gameplay progression is available.")) return LOCTEXT("MatchCompleteNoAction", "\u6BD4\u8D5B\u7ED3\u675F\uFF5C\u5F53\u524D\u65E0\u53EF\u7EE7\u7EED\u7684\u6BD4\u8D5B\u64CD\u4F5C");
	if (CanonicalLabel == TEXT("ATTACK COMPLETE | Waiting for the next match state.")) return LOCTEXT("AttackCompleteWaiting", "\u8FDB\u653B\u7ED3\u675F\uFF5C\u7B49\u5F85\u4E0B\u4E00\u6BD4\u8D5B\u72B6\u6001");
	if (CanonicalLabel == TEXT("Player A Win")) return LOCTEXT("PlayerAWin", "\u73A9\u5BB6 A \u83B7\u80DC");
	if (CanonicalLabel == TEXT("Player B Win")) return LOCTEXT("PlayerBWin", "\u73A9\u5BB6 B \u83B7\u80DC");
	if (CanonicalLabel == TEXT("Draw")) return LOCTEXT("MatchDraw", "\u5E73\u5C40");
	if (CanonicalLabel == TEXT("Player A")) return LOCTEXT("PlayerA", "\u73A9\u5BB6 A");
	if (CanonicalLabel == TEXT("Player B")) return LOCTEXT("PlayerB", "\u73A9\u5BB6 B");
	if (CanonicalLabel == TEXT("PLAYER A TO ACT")) return LOCTEXT("PlayerAToAct", "\u8BF7\u73A9\u5BB6 A \u64CD\u4F5C");
	if (CanonicalLabel == TEXT("PLAYER B TO ACT")) return LOCTEXT("PlayerBToAct", "\u8BF7\u73A9\u5BB6 B \u64CD\u4F5C");
	if (CanonicalLabel == TEXT("FINAL RESULT")) return LOCTEXT("FinalResult", "\u6700\u7EC8\u7ED3\u679C");
	if (CanonicalLabel == TEXT("WAITING TO START")) return LOCTEXT("WaitingToStart", "\u7B49\u5F85\u5F00\u59CB");
	if (CanonicalLabel.StartsWith(TEXT("Action Points: ")))
	{
		return FText::Format(LOCTEXT("ActionPoints", "\u884C\u52A8\u70B9 {0}"),
			FText::FromString(CanonicalLabel.RightChop(15)));
	}
	return FText::FromString(CanonicalLabel);
}

FText FFMCodexPlayerUIPresentationText::ThroughBallTitle()
{
	return LOCTEXT("ThroughBallProductionTitle", "直塞");
}

FText FFMCodexPlayerUIPresentationText::ThroughBallDirectChoiceHint()
{
	return LOCTEXT(
		"ThroughBallDirectChoiceHint", "（看射门、门将单刀）");
}

FText FFMCodexPlayerUIPresentationText::ThroughBallChipChoiceHint()
{
	return LOCTEXT("ThroughBallChipChoiceHint", "（只看掷点）");
}

FText FFMCodexPlayerUIPresentationText::ThroughBallInitialRouteStage()
{
	return LOCTEXT("ThroughBallInitialRouteStage", "判定直塞路线");
}

FText FFMCodexPlayerUIPresentationText::ThroughBallInitialRouteAction()
{
	return LOCTEXT("ThroughBallInitialRouteAction", "掷点判定路线");
}

FText FFMCodexPlayerUIPresentationText::ThroughBallRoute(
	const EMatchPlayThroughBallActualBranch Route)
{
	switch (Route)
	{
	case EMatchPlayThroughBallActualBranch::Feet:
		return LOCTEXT("ThroughBallRouteFeet", "脚下球");
	case EMatchPlayThroughBallActualBranch::BehindDefense:
		return LOCTEXT("ThroughBallRouteBehindDefense", "身后球");
	case EMatchPlayThroughBallActualBranch::AntiOffside:
		return LOCTEXT("ThroughBallRouteAntiOffside", "反越位");
	default:
		return FText::GetEmpty();
	}
}

FText FFMCodexPlayerUIPresentationText::ThroughBallFeetStage()
{
	return LOCTEXT("ThroughBallFeetStage", "属性对抗");
}

FText FFMCodexPlayerUIPresentationText::ThroughBallBehindDefenseStage()
{
	return LOCTEXT("ThroughBallBehindDefenseStage", "第一阶段");
}

FText FFMCodexPlayerUIPresentationText::ThroughBallAntiOffsideStage()
{
	return LOCTEXT("ThroughBallAntiOffsideStage", "越位判定");
}

FText FFMCodexPlayerUIPresentationText::ThroughBallOneOnOneStage()
{
	return LOCTEXT("ThroughBallOneOnOneStage", "选择单刀方式");
}

FText FFMCodexPlayerUIPresentationText::ThroughBallOneOnOnePrompt()
{
	return LOCTEXT(
		"ThroughBallOneOnOnePrompt", "直接射门或挑射");
}

FText FFMCodexPlayerUIPresentationText::ThroughBallRouteResult(
	const int32 RawD6,
	const EMatchPlayThroughBallActualBranch Route)
{
	return FText::Format(
		LOCTEXT("ThroughBallRouteResult", "路线掷点 {0} → 判定为{1}"),
		FText::AsNumber(RawD6),
		ThroughBallRoute(Route));
}

FText FFMCodexPlayerUIPresentationText::LongShotTitle()
{
	return LOCTEXT("LongShotProductionTitle", "远射");
}

FText FFMCodexPlayerUIPresentationText::LongShotBranchChoiceStage()
{
	return LOCTEXT("LongShotBranchChoiceStage", "选择远射方式");
}

FText FFMCodexPlayerUIPresentationText::LongShotDirectChoiceHint()
{
	return LOCTEXT(
		"LongShotDirectChoiceHint", "（看远射、抢断、门将站位）");
}

FText FFMCodexPlayerUIPresentationText::LongShotDeadCornerChoiceHint()
{
	return LOCTEXT(
		"LongShotDeadCornerChoiceHint", "（只看两枚掷点）");
}

FText FFMCodexPlayerUIPresentationText::LongShotDirectStage()
{
	return LOCTEXT("LongShotDirectStage", "直接射门");
}

FText FFMCodexPlayerUIPresentationText::LongShotDeadCornerStage()
{
	return LOCTEXT("LongShotDeadCornerStage", "射向死角");
}

FText FFMCodexPlayerUIPresentationText::LongShotDirectOutcomeHint()
{
	return LOCTEXT(
		"LongShotDirectOutcomeHint",
		"1–2：射门偏出 ｜ 3–6：进入攻防结算");
}

FText FFMCodexPlayerUIPresentationText::LongShotDeadCornerOutcomeHint()
{
	return LOCTEXT(
		"LongShotDeadCornerOutcomeHint",
		"合计 11–12：进球 ｜ 2–10：未进");
}

FText FFMCodexPlayerUIPresentationText::SelectedRoleTag(
	const FString& CanonicalRole)
{
	if (CanonicalRole == TEXT("Carrier")) return LOCTEXT("SelectedRoleCarrier", "\u6301\u7403");
	if (CanonicalRole == TEXT("Runner")) return LOCTEXT("SelectedRoleRunner", "\u8DD1\u4F4D");
	if (CanonicalRole == TEXT("Marker")) return LOCTEXT("SelectedRoleMarker", "\u76EF\u4EBA");
	if (CanonicalRole == TEXT("Helper")) return LOCTEXT("SelectedRoleHelper", "\u534F\u9632");
	return FText::GetEmpty();
}

FText FFMCodexPlayerUIPresentationText::SelectionFeedback(
	const FString& CanonicalReason)
{
	if (CanonicalReason == TEXT("MarkerWrongPhysicalArea"))
	{
		return LOCTEXT("MarkerWrongPhysicalArea",
			"\u76EF\u4EBA\u7403\u5458\u5FC5\u987B\u4E0E\u6301\u7403\u7403\u5458\u4F4D\u4E8E\u540C\u4E00\u534A\u533A");
	}
	if (CanonicalReason == TEXT("RunnerIsGoalkeeper"))
	{
		return LOCTEXT("RunnerIsGoalkeeper",
			"\u95E8\u5C06\u4E0D\u80FD\u4F5C\u4E3A\u8DD1\u4F4D\u7403\u5458");
	}
	if (CanonicalReason == TEXT("RunnerMatchesCarrier"))
	{
		return LOCTEXT("RunnerMatchesCarrier",
			"\u8DD1\u4F4D\u7403\u5458\u4E0D\u80FD\u4E0E\u6301\u7403\u7403\u5458\u76F8\u540C");
	}
	if (CanonicalReason == TEXT("RunnerMissingRequiredPositionType"))
	{
		return LOCTEXT("RunnerMissingRequiredPositionType",
			"\u8BE5\u7403\u5458\u4E0D\u7B26\u5408\u5F53\u524D\u8DD1\u4F4D\u4F4D\u7F6E\u8981\u6C42");
	}
	if (CanonicalReason == TEXT("RunnerNotInAttackingForwardArea"))
	{
		return LOCTEXT("RunnerNotInAttackingForwardArea",
			"\u76F4\u585E\u8981\u6C42\u8DD1\u4F4D\u7403\u5458\u4F4D\u4E8E\u524D\u573A");
	}
	if (CanonicalReason == TEXT("HelperIsGoalkeeper"))
	{
		return LOCTEXT("HelperIsGoalkeeper",
			"\u95E8\u5C06\u4E0D\u80FD\u4F5C\u4E3A\u534F\u9632\u7403\u5458");
	}
	if (CanonicalReason == TEXT("HelperMatchesMarker"))
	{
		return LOCTEXT("HelperMatchesMarker",
			"\u8BE5\u7403\u5458\u5DF2\u88AB\u6307\u5B9A\u4E3A\u76EF\u4EBA\u7403\u5458\uFF0C\u8BF7\u9009\u62E9\u5176\u4ED6\u534F\u9632\u7403\u5458");
	}
	if (CanonicalReason == TEXT("HelperWrongPhysicalArea"))
	{
		return LOCTEXT("HelperWrongPhysicalArea",
			"\u534F\u9632\u7403\u5458\u5FC5\u987B\u4E0E\u8DD1\u4F4D\u7403\u5458\u4F4D\u4E8E\u540C\u4E00\u534A\u533A");
	}
	return FText::GetEmpty();
}

FText FFMCodexPlayerUIPresentationText::CompactPlayerName(
	const FName CardId,
	const FString& FallbackLabel)
{
	const FText PrototypeName =
		FFMCodexPrototypeTeamContent::PlayerDisplayName(CardId);
	if (!PrototypeName.IsEmpty())
	{
		return PrototypeName;
	}

	const FString Id = CardId.ToString();
	if (Id.StartsWith(TEXT("Demo.A.Outfield."))
		|| Id.StartsWith(TEXT("Demo.B.Outfield.")))
	{
		const bool bPlayerA = Id.StartsWith(TEXT("Demo.A."));
		FString Number;
		Id.Split(TEXT("."), nullptr, &Number, ESearchCase::CaseSensitive,
			ESearchDir::FromEnd);
		return FText::Format(LOCTEXT("CompactDemoPlayer", "{0}\u961F {1}\u53F7"),
			bPlayerA ? LOCTEXT("CompactDemoTeamA", "A")
				: LOCTEXT("CompactDemoTeamB", "B"),
			FText::FromString(Number));
	}
	if (Id == TEXT("Demo.A.Goalkeeper") || Id == TEXT("Demo.B.Goalkeeper"))
	{
		return Id.Contains(TEXT(".A."))
			? LOCTEXT("CompactDemoGoalkeeperA", "A\u961F \u95E8\u5C06")
			: LOCTEXT("CompactDemoGoalkeeperB", "B\u961F \u95E8\u5C06");
	}
	if (!FallbackLabel.IsEmpty()
		&& FallbackLabel != Id
		&& !FallbackLabel.Contains(TEXT("CardId"))
		&& !FallbackLabel.Contains(TEXT("Demo.")))
	{
		return FText::FromString(FallbackLabel);
	}
	return LOCTEXT("CompactAnonymousPlayer", "\u7403\u5458");
}

FText FFMCodexPlayerUIPresentationText::HandMicroPlayerName(
	const FName CardId,
	const FString& FallbackLabel)
{
	const FText PrototypeName =
		FFMCodexPrototypeTeamContent::PlayerDisplayName(CardId);
	if (!PrototypeName.IsEmpty())
	{
		return PrototypeName;
	}

	// Non-production validation aliases remain bounded defensive fixtures.
	if (CardId == TEXT("Demo.A.Outfield.01"))
	{
		return LOCTEXT("HandMicroMartinelli", "\u9A6C\u4E01\u5185\u5229");
	}
	if (CardId == TEXT("Demo.A.Outfield.02"))
	{
		return LOCTEXT("HandMicroGabriel", "\u52A0\u5E03\u91CC\u57C3\u5C14");
	}
	if (CardId == TEXT("Demo.A.Outfield.03"))
	{
		return LOCTEXT("HandMicroMerino", "\u6885\u91CC\u8BFA");
	}
	if (CardId == TEXT("Demo.B.Outfield.01"))
	{
		return LOCTEXT("HandMicroGvardiol", "\u683C\u74E6\u8FEA\u5965\u5C14");
	}
	if (CardId == TEXT("Demo.B.Outfield.02"))
	{
		return LOCTEXT("HandMicroBernardo", "\u8D1D\u5C14\u7EB3\u591A");
	}
	if (CardId == TEXT("Demo.B.Outfield.03"))
	{
		return LOCTEXT("HandMicroDoku", "\u591A\u5E93");
	}
	if (CardId == TEXT("Visual.HandMicro.Kvaratskhelia"))
	{
		return LOCTEXT(
			"HandMicroKvaratskhelia", "\u514B\u74E6\u62C9\u8328\u8D6B\u5229\u4E9A");
	}
	return CompactPlayerName(CardId, FallbackLabel);
}

FText FFMCodexPlayerUIPresentationText::HandMicroFallbackPlayerName(
	const FName CardId)
{
	// Exceptional, localization-ready presentation aliases. The Widget asks
	// for these only after the primary display name fails at the 12 px floor.
	if (CardId == TEXT("Demo.B.Outfield.01"))
	{
		return LOCTEXT("HandMicroFallbackGvardiol", "\u683C\u74E6");
	}
	if (CardId == TEXT("Visual.HandMicro.Kvaratskhelia"))
	{
		return LOCTEXT("HandMicroFallbackKvaratskhelia", "\u514B\u74E6\u62C9");
	}
	return FText::GetEmpty();
}

FText FFMCodexPlayerUIPresentationText::TeamName(const FName CardId)
{
	return FFMCodexPrototypeTeamContent::TeamDisplayName(CardId);
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

FText FFMCodexPlayerUIPresentationText::CompactRole(
	const FString& CanonicalLabel)
{
	if (CanonicalLabel.IsEmpty() || CanonicalLabel == TEXT("ROLE N/A"))
	{
		return LOCTEXT("CompactRoleUnknown", "?");
	}
	TArray<FString> Tokens;
	CanonicalLabel.ParseIntoArray(Tokens, TEXT("/"), true);
	TArray<FString> Abbreviations;
	for (FString Token : Tokens)
	{
		Token.TrimStartAndEndInline();
		if (Token == TEXT("FW")) Abbreviations.Add(TEXT("A"));
		else if (Token == TEXT("MF")) Abbreviations.Add(TEXT("M"));
		else if (Token == TEXT("DF")) Abbreviations.Add(TEXT("D"));
		else if (Token == TEXT("GK")) Abbreviations.Add(TEXT("GK"));
	}
	return Abbreviations.IsEmpty()
		? LOCTEXT("CompactRoleUnknownFallback", "?")
		: FText::FromString(FString::Join(Abbreviations, TEXT("")));
}

FText FFMCodexPlayerUIPresentationText::HandMicroCompactRole(
	const FString& CanonicalLabel)
{
	FString Compact = CompactRole(CanonicalLabel).ToString();
	if (Compact == TEXT("?"))
	{
		Compact = CanonicalLabel.ToUpper();
		Compact.ReplaceInline(TEXT(" "), TEXT(""));
		Compact.ReplaceInline(TEXT("/"), TEXT(""));
		if (Compact != TEXT("A") && Compact != TEXT("M")
			&& Compact != TEXT("D") && Compact != TEXT("AM")
			&& Compact != TEXT("MD") && Compact != TEXT("AD")
			&& Compact != TEXT("AMD") && Compact != TEXT("GK"))
		{
			return LOCTEXT("HandMicroCompactRoleUnknown", "?");
		}
	}
	if (Compact == TEXT("GK") || Compact.Len() <= 1)
	{
		return FText::FromString(Compact);
	}
	TArray<FString> RoleLetters;
	RoleLetters.Reserve(Compact.Len());
	for (const TCHAR Letter : Compact)
	{
		RoleLetters.Add(FString::Chr(Letter));
	}
	return FText::FromString(FString::Join(RoleLetters, TEXT("/")));
}

FText FFMCodexPlayerUIPresentationText::InMatchCompactRole(
	const FString& CanonicalLabel)
{
	return HandMicroCompactRole(CanonicalLabel);
}

FText FFMCodexPlayerUIPresentationText::PitchMiniCompactRole(
	const FString& CanonicalLabel)
{
	return HandMicroCompactRole(CanonicalLabel);
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

FText FFMCodexPlayerUIPresentationText::AttributeLabel(
	const FString& CanonicalToken)
{
	FString Token = CanonicalToken;
	Token.TrimStartAndEndInline();
	return Token.IsEmpty()
		? AttributesUnavailable()
		: FMCodexPlayerUIPresentationText::MapAttributeToken(Token.ToUpper());
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
	if (CanonicalLabel == TEXT("Regional") || CanonicalLabel == TEXT("Club")) return LOCTEXT("RarityRegional", "区域级");
	if (CanonicalLabel == TEXT("National")) return LOCTEXT("RarityNational", "国家级");
	if (CanonicalLabel == TEXT("Continental")) return LOCTEXT("RarityContinental", "洲际级");
	if (CanonicalLabel == TEXT("World Class")) return LOCTEXT("RarityWorldClass", "世界级");
	if (CanonicalLabel == TEXT("Pilot")) return LOCTEXT("RarityPilot", "试制");
	return FText::FromString(CanonicalLabel);
}

FText FFMCodexPlayerUIPresentationText::CompactRarity(
	const FString& CanonicalLabel)
{
	if (CanonicalLabel == TEXT("Common")) return LOCTEXT("CompactRarityCommon", "\u666E");
	if (CanonicalLabel == TEXT("Regional")) return LOCTEXT("CompactRarityRegional", "\u533A");
	if (CanonicalLabel == TEXT("National")) return LOCTEXT("CompactRarityNational", "\u56FD");
	if (CanonicalLabel == TEXT("Continental")) return LOCTEXT("CompactRarityContinental", "\u6D32");
	if (CanonicalLabel == TEXT("World Class")) return LOCTEXT("CompactRarityWorldClass", "\u4E16");
	if (CanonicalLabel == TEXT("Pilot")) return LOCTEXT("CompactRarityPilot", "\u8BD5");
	return LOCTEXT("CompactRarityUnknown", "?");
}

FText FFMCodexPlayerUIPresentationText::Owner(const FString& CanonicalLabel)
{
	if (CanonicalLabel == TEXT("Player A")) return LOCTEXT("OwnerPlayerA", "玩家 A");
	if (CanonicalLabel == TEXT("Player B")) return LOCTEXT("OwnerPlayerB", "玩家 B");
	return CanonicalLabel.IsEmpty()
		? LOCTEXT("OwnerUnavailable", "玩家未知") : FText::FromString(CanonicalLabel);
}

FText FFMCodexPlayerUIPresentationText::RackHeading(
	const FString& SideLabel,
	const bool bLocalRack)
{
	return bLocalRack
		? LOCTEXT("LocalRackHeading", "\u672C\u65B9")
		: LOCTEXT("OpponentRackHeading", "\u5BF9\u65B9");
}

FText FFMCodexPlayerUIPresentationText::TacticalRegion(
	const FString& CanonicalLabel)
{
	if (CanonicalLabel == TEXT("Forward")) return LOCTEXT("RegionForward", "\u524D\u573A");
	if (CanonicalLabel == TEXT("Midfield")) return LOCTEXT("RegionMidfield", "\u4E2D\u573A");
	if (CanonicalLabel == TEXT("Backfield")) return LOCTEXT("RegionBackfield", "\u540E\u573A");
	return FText::FromString(CanonicalLabel);
}

FText FFMCodexPlayerUIPresentationText::TacticalLaneHeading(
	const FString& CanonicalLabel,
	const bool bAttacking)
{
	return TacticalRegion(CanonicalLabel);
}

FText FFMCodexPlayerUIPresentationText::Turn(const int64 AttackSequence)
{
	return AttackSequence > 0
		? FText::Format(LOCTEXT("TurnFormat", "\u7B2C {0} \u8F6E\u8FDB\u653B"),
			FText::AsNumber(AttackSequence))
		: LOCTEXT("PreMatchTurn", "\u8D5B\u524D");
}

FText FFMCodexPlayerUIPresentationText::TacticalPoints(const int32 ActionPoint)
{
	return FText::Format(LOCTEXT("TacticalPointsFormat", "\u6218\u672F\u70B9  {0}"),
		FText::AsNumber(ActionPoint));
}

FText FFMCodexPlayerUIPresentationText::TacticalPointsHeading()
{
	return LOCTEXT("TacticalPointsHeading", "\u6218\u672F\u70B9");
}

FText FFMCodexPlayerUIPresentationText::AttackTurnHeading()
{
	return LOCTEXT("AttackTurnHeading", "\u8FDB\u653B\u56DE\u5408");
}

FText FFMCodexPlayerUIPresentationText::CurrentAttackProgress(
	const FString& PlayerLabel,
	const int32 AttackIndex,
	const int32 MaxAttackTurns)
{
	return FText::Format(
		LOCTEXT("CurrentAttackProgress", "{0} \u7B2C {1}/{2} \u6B21\u8FDB\u653B"),
		MatchScreenLabel(PlayerLabel), FText::AsNumber(AttackIndex),
		FText::AsNumber(MaxAttackTurns));
}

FText FFMCodexPlayerUIPresentationText::WaitingForTacticalPointRoll()
{
	return LOCTEXT("WaitingForTacticalPointRoll", "\u7B49\u5F85\u63B7\u51FA\u6218\u672F\u70B9");
}

FText FFMCodexPlayerUIPresentationText::ResolutionContest(
	const FName ContestId)
{
	if (ContestId == TEXT("Cross.High"))
	{
		return LOCTEXT("ResolutionCrossHigh", "\u9AD8\u7403\u4F20\u4E2D");
	}
	if (ContestId == TEXT("Cross.Low"))
	{
		return LOCTEXT("ResolutionCrossLow", "\u4F4E\u7403\u4F20\u4E2D");
	}
	if (ContestId == TEXT("ThroughBall.BehindDefense.P1"))
	{
		return LOCTEXT(
			"ResolutionThroughBallBehind", "\u8EAB\u540E\u7403\u5BF9\u6297");
	}
	if (ContestId == TEXT("ThroughBall.OneOnOne.DirectShot"))
	{
		return LOCTEXT(
			"ResolutionThroughBallOneOnOneDirect", "\u76F4\u63A5\u5C04\u95E8");
	}
	if (ContestId == TEXT("LongShot.DirectShot"))
	{
		return LOCTEXT("ResolutionLongShotDirect", "直接射门");
	}
	return FText::GetEmpty();
}

FText FFMCodexPlayerUIPresentationText::ResolutionParticipantRole(
	const EMatchPlayResolutionParticipantRole Role)
{
	switch (Role)
	{
	case EMatchPlayResolutionParticipantRole::Carrier:
		return LOCTEXT("ResolutionRoleCarrier", "\u6301\u7403");
	case EMatchPlayResolutionParticipantRole::Runner:
		return LOCTEXT("ResolutionRoleRunner", "\u8DD1\u4F4D");
	case EMatchPlayResolutionParticipantRole::Marker:
		return LOCTEXT("ResolutionRoleMarker", "\u76EF\u4EBA");
	case EMatchPlayResolutionParticipantRole::Helper:
		return LOCTEXT("ResolutionRoleHelper", "\u534F\u9632");
	case EMatchPlayResolutionParticipantRole::Goalkeeper:
		return LOCTEXT("ResolutionRoleGoalkeeper", "\u95E8\u5C06");
	default:
		return FText::GetEmpty();
	}
}

FText FFMCodexPlayerUIPresentationText::TacticalDetailParticipantRole(
	const EMatchPlayResolutionParticipantRole Role)
{
	if (Role == EMatchPlayResolutionParticipantRole::Runner)
	{
		return LOCTEXT("TacticalDetailRoleRunner", "\u8DD1\u4F4D\u7403\u5458");
	}
	return ResolutionParticipantRole(Role);
}

FText FFMCodexPlayerUIPresentationText::TacticalOutcome(
	const FName BranchId, const FName OutcomeId)
{
	if (BranchId == TEXT("ThroughBall.AntiOffside"))
	{
		if (OutcomeId == TEXT("Offside"))
		{
			return LOCTEXT("ThroughBallAntiOffsideOutcomeOffside", "\u8D8A\u4F4D");
		}
		if (OutcomeId == TEXT("OneOnOne"))
		{
			return LOCTEXT("ThroughBallAntiOffsideOutcomeSuccess", "\u53CD\u8D8A\u4F4D\u6210\u529F");
		}
	}
	if (BranchId == TEXT("ThroughBall.OneOnOneChip"))
	{
		if (OutcomeId == TEXT("Miss"))
		{
			return LOCTEXT("ThroughBallChipOutcomeMiss", "\u6311\u5C04\u672A\u8FDB");
		}
		if (OutcomeId == TEXT("Goal"))
		{
			return LOCTEXT("ThroughBallChipOutcomeGoal", "\u8FDB\u7403");
		}
	}
	return FText::GetEmpty();
}

FText FFMCodexPlayerUIPresentationText::TacticalOutcomeRange(
	const int32 Minimum, const int32 Maximum, const FText& OutcomeLabel)
{
	if (Minimum == Maximum)
	{
		return FText::Format(
			LOCTEXT("TacticalSingleOutcomeRange", "{0}\uFF1A{1}"),
			FText::AsNumber(Minimum), OutcomeLabel);
	}
	return FText::Format(
		LOCTEXT("TacticalOutcomeRange", "{0}\u2013{1}\uFF1A{2}"),
		FText::AsNumber(Minimum), FText::AsNumber(Maximum), OutcomeLabel);
}

FText FFMCodexPlayerUIPresentationText::ResolutionAttribute(
	const EMatchPlayResolutionFormulaAttribute Attribute)
{
	using EAttribute = EMatchPlayResolutionFormulaAttribute;
	switch (Attribute)
	{
	case EAttribute::Shooting: return AttributeLabel(TEXT("SHO"));
	case EAttribute::Dribbling: return AttributeLabel(TEXT("DRI"));
	case EAttribute::Passing: return AttributeLabel(TEXT("PAS"));
	case EAttribute::OffBall: return AttributeLabel(TEXT("OFF"));
	case EAttribute::Marking: return AttributeLabel(TEXT("MRK"));
	case EAttribute::Tackling: return AttributeLabel(TEXT("TKL"));
	case EAttribute::Speed: return AttributeLabel(TEXT("SPD"));
	case EAttribute::Strength: return AttributeLabel(TEXT("STR"));
	case EAttribute::LongShot: return AttributeLabel(TEXT("LS"));
	case EAttribute::GoalkeeperHandling: return AttributeLabel(TEXT("HAN"));
	case EAttribute::GoalkeeperPositioning: return AttributeLabel(TEXT("POS"));
	case EAttribute::GoalkeeperReflex: return AttributeLabel(TEXT("REF"));
	case EAttribute::GoalkeeperAerial: return AttributeLabel(TEXT("AER"));
	case EAttribute::GoalkeeperOneOnOne: return AttributeLabel(TEXT("1V1"));
	default: return FText::GetEmpty();
	}
}

FText FFMCodexPlayerUIPresentationText::ResolutionAttackRow()
{
	return LOCTEXT("ResolutionAttackRow", "\u8FDB\u653B");
}

FText FFMCodexPlayerUIPresentationText::ResolutionDefenseRow()
{
	return LOCTEXT("ResolutionDefenseRow", "\u9632\u5B88");
}

FText FFMCodexPlayerUIPresentationText::ResolutionPending()
{
	return LOCTEXT("ResolutionPending", "\u7B49\u5F85\u7ED3\u7B97");
}

FText FFMCodexPlayerUIPresentationText::ResolutionResolved()
{
	return LOCTEXT("ResolutionResolved", "\u516C\u5F0F\u5DF2\u7ED3\u7B97");
}

FText FFMCodexPlayerUIPresentationText::ResolutionAttackRoll()
{
	return LOCTEXT("ResolutionAttackRoll", "\u8FDB\u653B\u63B7\u70B9");
}

FText FFMCodexPlayerUIPresentationText::ResolutionAttackRollSettled()
{
	return LOCTEXT("ResolutionAttackRollSettled", "\u8FDB\u653B\u63B7\u70B9\u5DF2\u843D\u5B9A");
}

FText FFMCodexPlayerUIPresentationText::ResolutionDefenseRoll()
{
	return LOCTEXT("ResolutionDefenseRoll", "\u9632\u5B88\u63B7\u70B9");
}

FText FFMCodexPlayerUIPresentationText::ResolutionDefenseRollSettled()
{
	return LOCTEXT("ResolutionDefenseRollSettled", "\u9632\u5B88\u63B7\u70B9\u5DF2\u843D\u5B9A");
}

FText FFMCodexPlayerUIPresentationText::ContinueResolution()
{
	return LOCTEXT("ContinueResolution", "\u7EE7\u7EED\u7ED3\u7B97");
}

FText FFMCodexPlayerUIPresentationText::BroadcastStatus(
	const bool bMatchEnded,
	const bool bAttackActive,
	const FString& MatchResultLabel)
{
	if (bMatchEnded)
	{
		return FText::FromString(MatchResultLabel);
	}
	return bAttackActive
		? LOCTEXT("LiveMatch", "\u6BD4\u8D5B\u8FDB\u884C\u4E2D")
		: LOCTEXT("LocalMatch", "\u672C\u5730\u5BF9\u6218");
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
FText FFMCodexPlayerUIPresentationText::FullCardAttributesHeading() { return LOCTEXT("FullCardAttributesHeading", "球员属性"); }
FText FFMCodexPlayerUIPresentationText::OverallHeading() { return LOCTEXT("OverallHeading", "总能力值"); }
FText FFMCodexPlayerUIPresentationText::PositionHeading() { return LOCTEXT("PositionHeading", "位置"); }
FText FFMCodexPlayerUIPresentationText::FullCardPositionTypeHeading()
{
	return LOCTEXT("FullCardPositionTypeHeading", "位置类型");
}

FText FFMCodexPlayerUIPresentationText::FullCardIdentitySupplement(
	const FString& NationalityLabel,
	const FString& ClubLabel)
{
	if (!NationalityLabel.IsEmpty() && !ClubLabel.IsEmpty())
	{
		return FText::Format(
			LOCTEXT("FullCardIdentitySupplementBoth",
				"国籍：{0}  |  俱乐部：{1}"),
			FText::FromString(NationalityLabel), FText::FromString(ClubLabel));
	}
	if (!NationalityLabel.IsEmpty())
	{
		return FText::Format(
			LOCTEXT("FullCardIdentitySupplementNationality", "国籍：{0}"),
			FText::FromString(NationalityLabel));
	}
	if (!ClubLabel.IsEmpty())
	{
		return FText::Format(
			LOCTEXT("FullCardIdentitySupplementClub", "俱乐部：{0}"),
			FText::FromString(ClubLabel));
	}
	return FText::GetEmpty();
}

FText FFMCodexPlayerUIPresentationText::BirthDateHeading() { return LOCTEXT("BirthDateHeading", "出生日期"); }
FText FFMCodexPlayerUIPresentationText::HeightHeading() { return LOCTEXT("HeightHeading", "身高"); }
FText FFMCodexPlayerUIPresentationText::WeightHeading() { return LOCTEXT("WeightHeading", "体重"); }

FText FFMCodexPlayerUIPresentationText::DeploymentHandInstruction()
{
	return LOCTEXT("DeploymentHandInstruction",
		"\u62D6\u52A8\u7403\u5458\u5361\u8FDB\u884C\u90E8\u7F72");
}

FText FFMCodexPlayerUIPresentationText::EmptyPitchSlot()
{
	return LOCTEXT("EmptyPitchSlot", "\u7A7A\u4F4D\u7F6E");
}

FText FFMCodexPlayerUIPresentationText::ValidDeploymentTarget()
{
	return LOCTEXT("ValidDeploymentTarget",
		"\u53EF\u90E8\u7F72\u4F4D\u7F6E");
}

FText FFMCodexPlayerUIPresentationText::InvalidDeploymentTarget()
{
	return LOCTEXT("InvalidDeploymentTarget", "\u4E0D\u53EF\u90E8\u7F72");
}

FText FFMCodexPlayerUIPresentationText::OccupiedDeploymentTarget()
{
	return LOCTEXT("OccupiedDeploymentTarget", "\u5DF2\u5360\u7528");
}

FText FFMCodexPlayerUIPresentationText::UnavailableDeploymentTarget()
{
	return LOCTEXT("UnavailableDeploymentTarget",
		"\u4F4D\u7F6E\u4E0D\u53EF\u7528");
}

#undef LOCTEXT_NAMESPACE
