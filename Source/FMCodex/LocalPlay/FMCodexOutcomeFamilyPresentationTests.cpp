#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING

#include "FMCodexInlineResolutionFormulaSurfaceWidget.h"
#include "FMCodexLongShotResolutionSurfaceWidget.h"
#include "FMCodexThroughBallResolutionSurfaceWidget.h"
#include "FMCodexMatchFlowPanel.h"
#include "FMCodexOutcomePresentation.h"
#include "FMCodexLocalMatchInteractionView.h"
#include "FMCodexLocalMatchResolutionFeedback.h"
#include "FMCodexTacticalResolutionNarrativePresentation.h"
#include "Components/RichTextBlock.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Misc/AutomationTest.h"
#include "Slate/WidgetRenderer.h"
#include "Engine/TextureRenderTarget2D.h"

namespace FMCodexOutcomeTests
{
UTextBlock* Text(UUserWidget& W, const TCHAR* Name)
{
	return CastChecked<UTextBlock>(W.GetWidgetFromName(Name));
}
URichTextBlock* Primary(UUserWidget& W)
{
	return CastChecked<URichTextBlock>(W.GetWidgetFromName(TEXT("OutcomePrimary")));
}
FFMCodexUMGInlineFormulaSurfaceViewModel Detail()
{
	FFMCodexUMGInlineFormulaSurfaceViewModel P;
	P.bVisible = P.bNarrativeAvailable = true;
	P.bShowFormulaRows = false;
	P.ContestId = TEXT("SetPiece.Compact");
	P.ResolutionContextLabel = TEXT("远距离任意球 · 重炮轰门");
	P.ContestLabel = TEXT("诺尔高远距离任意球重炮轰门未能得分。");
	P.StatusLabel = TEXT("远距离任意球 · 重炮轰门 · 未进球");
	P.RouteResultLabel = TEXT("D6 3 + D6 1 = 4");
	P.OutcomeRollDetail = FMCodexOutcomeText::PairedRollDetail(3, 1).ToString();
	P.OutcomeText = FFMCodexOutcomeText(FText::FromString(TEXT("诺尔高远距离任意球重炮轰门")),
		FText::FromString(TEXT("未能得分")), FText::FromString(TEXT("。")), EFMCodexOutcomeAccent::NoGoal);
	P.PrimaryAction.bVisible = P.PrimaryAction.Action.bAvailable = true;
	P.PrimaryAction.Action.Label = TEXT("下一回合");
	return P;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexOutcomeContentTest,
	"FMCodex.LocalPlay.OutcomeFamily.ContentHierarchyAndReuse",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexOutcomeContentTest::RunTest(const FString&)
{
	using namespace FMCodexOutcomeTests;
	auto* W = NewObject<UFMCodexInlineResolutionFormulaSurfaceWidget>();
	const auto Slate = W->TakeWidget();
	auto P = Detail();
	W->RefreshFromPresentation(P);
	auto* PrimaryText = Primary(*W);
	auto* Context = Text(*W, TEXT("OutcomeContext"));
	auto* KeyDetail = Text(*W, TEXT("OutcomeDetail"));
	auto* Button = CastChecked<UFMCodexMatchFlowButton>(W->GetWidgetFromName(TEXT("InlineFormulaContinueButton")));
	TestEqual(TEXT("Complete canonical sentence preserved"), PrimaryText->GetText().ToString(), FMCodexOutcomePresentation::PrimaryMarkup(P.ContestLabel, P.OutcomeText));
	TestEqual(TEXT("Context preserved without semantic rewrite"), Context->GetText().ToString(), P.StatusLabel);
	TestEqual(TEXT("Structured pair has natural wording"), KeyDetail->GetText().ToString(), FString(TEXT("首次掷点 3 + 第二次掷点 1 = 4")));
	TestTrue(TEXT("Prose has stronger hierarchy than context and detail"), PrimaryText->GetCurrentDefaultTextStyle().Font.Size > Context->GetFont().Size && PrimaryText->GetCurrentDefaultTextStyle().Font.Size > KeyDetail->GetFont().Size);
	TestTrue(TEXT("Result uses flow button with original bound delegate"), Button->IsFlowStyleEnabled() && Button->OnClicked.IsBound() && Button->GetIsEnabled());
	TestFalse(TEXT("Short CTA stays one unbroken action label across DPI changes"), CastChecked<UTextBlock>(Button->GetChildAt(0))->GetAutoWrapText());
	const auto NeutralProse = PrimaryText->GetCurrentDefaultTextStyle().ColorAndOpacity.GetSpecifiedColor();
	const auto NeutralFrame = CastChecked<UBorder>(W->GetWidgetFromName(TEXT("InlineFormulaSurfaceFrame")))->GetBrushColor();
	P.ContestLabel = TEXT("加布里埃尔远距离任意球重炮轰门破门！");
	P.bNarrativeAttackSuccess = true;
	P.OutcomeText = FFMCodexOutcomeText(FText::FromString(TEXT("加布里埃尔远距离任意球重炮轰门")),
		FText::FromString(TEXT("破门")), FText::FromString(TEXT("！")), EFMCodexOutcomeAccent::Goal);
	P.RouteResultLabel = TEXT("D6 6 + D6 5 = 11");
	W->RefreshFromPresentation(P);
	TestEqual(TEXT("Goal has no prose rewrite"), PrimaryText->GetText().ToString(), FMCodexOutcomePresentation::PrimaryMarkup(P.ContestLabel, P.OutcomeText));
	TestTrue(TEXT("Only keyword changes color; default prose and frame stay neutral"), NeutralProse.Equals(PrimaryText->GetCurrentDefaultTextStyle().ColorAndOpacity.GetSpecifiedColor())
		&& NeutralFrame.Equals(CastChecked<UBorder>(W->GetWidgetFromName(TEXT("InlineFormulaSurfaceFrame")))->GetBrushColor()));
	P.ContestLabel = TEXT("福登远射偏出。");
	P.StatusLabel.Empty(); P.RouteResultLabel.Empty(); P.RollHelperLabel.Empty(); P.OutcomeRollDetail.Empty(); P.OutcomeText = {};
	P.PrimaryAction.Action.Label = TEXT("继续"); P.PrimaryAction.Action.bAvailable = false;
	W->RefreshFromPresentation(P);
	TestEqual(TEXT("Reuse replaces old result"), PrimaryText->GetText().ToString(), FMCodexOutcomePresentation::PrimaryMarkup(P.ContestLabel, P.OutcomeText));
	TestTrue(TEXT("Missing context and detail collapse without stale text"), Context->GetVisibility() == ESlateVisibility::Collapsed
		&& Context->GetText().IsEmpty() && KeyDetail->GetText().IsEmpty()
		&& W->GetWidgetFromName(TEXT("OutcomeDetailRegion"))->GetVisibility() == ESlateVisibility::Collapsed);
	TestTrue(TEXT("Unavailable action stays disabled and hidden with no orphan separator"), !Button->GetIsEnabled()
		&& Button->GetParent()->GetVisibility() == ESlateVisibility::Collapsed
		&& W->GetWidgetFromName(TEXT("OutcomeFooterRule"))->GetVisibility() == ESlateVisibility::Collapsed);
	TestEqual(TEXT("CTA label refreshes even while hidden"), CastChecked<UTextBlock>(Button->GetChildAt(0))->GetText().ToString(), P.PrimaryAction.Action.Label);
	P.PrimaryAction.Action.bAvailable = true;
	W->RefreshFromPresentation(P);
	auto* Renderer = new FWidgetRenderer(true);
	auto* Target = Renderer->DrawWidget(Slate, FVector2D(808, 500));
	Renderer->DrawWidget(Target, Slate, FVector2D(808, 500), 0.f);
	const float ShortHeight = PrimaryText->GetDesiredSize().Y;
	P.ContestLabel = TEXT("亚历山大·阿诺德与刘易斯·斯凯利的远距离任意球进攻被防守方化解，本次进攻未能得分。");
	P.RouteResultLabel = TEXT("已经揭示的完整详细信息保持原文，以可换行的辅助细节显示，不裁掉末尾的结果文字。");
	P.PrimaryAction.Action.Label = TEXT("继续进入下一回合");
	W->RefreshFromPresentation(P);
	Renderer->DrawWidget(Target, Slate, FVector2D(808, 500), 0.f);
	Renderer->DrawWidget(Target, Slate, FVector2D(808, 500), 0.f);
	TestTrue(TEXT("Long canonical prose wraps naturally without shrinking font"), PrimaryText->GetDesiredSize().Y > ShortHeight
		&& PrimaryText->GetAutoWrapText() && PrimaryText->GetCurrentDefaultTextStyle().Font.Size > Context->GetFont().Size);
	TestEqual(TEXT("Long prose remains complete"), PrimaryText->GetText().ToString(), FMCodexOutcomePresentation::PrimaryMarkup(P.ContestLabel, P.OutcomeText));
	BeginCleanup(Renderer);
	P = Detail(); P.bNarrativeAvailable = false; P.bDiceRevealVisible = true;
	W->RefreshFromPresentation(P);
	TestTrue(TEXT("Unrevealed reuse clears outcome text and hides composition"), PrimaryText->GetText().IsEmpty()
		&& W->GetWidgetFromName(TEXT("OutcomeHierarchy"))->GetVisibility() == ESlateVisibility::Collapsed);
	P = Detail(); P.bShowFormulaRows = true;
	W->RefreshFromPresentation(P);
	TestTrue(TEXT("Accepted Formula result stays in its original hierarchy"), PrimaryText->GetText().IsEmpty()
		&& W->GetWidgetFromName(TEXT("InlineFormulaOutcomeHeading"))->GetVisibility() != ESlateVisibility::Collapsed
		&& W->GetWidgetFromName(TEXT("InlineFormulaResultBadge"))->GetVisibility() != ESlateVisibility::Collapsed);
	P = Detail(); P.ContestId = TEXT("SetPiece.Opposed");
	W->RefreshFromPresentation(P);
	TestTrue(TEXT("Rare no-row Opposed mode stays deferred without parsing its prose"),
		PrimaryText->GetText().IsEmpty()
		&& !CastChecked<UFMCodexMatchFlowPanel>(W->GetWidgetFromName(TEXT("InlineFormulaSurfaceFrame")))->IsFlowStyleEnabled()
		&& Text(*W, TEXT("InlineFormulaContestHeading"))->GetVisibility() != ESlateVisibility::Collapsed
		&& !Button->IsFlowStyleEnabled());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexOutcomeHostTest,
	"FMCodex.LocalPlay.OutcomeFamily.SharedHostsAndIsolation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexOutcomeHostTest::RunTest(const FString&)
{
	using namespace FMCodexOutcomeTests;
	auto* Shot = NewObject<UFMCodexLongShotResolutionSurfaceWidget>(); Shot->TakeWidget();
	auto* Through = NewObject<UFMCodexThroughBallResolutionSurfaceWidget>(); Through->TakeWidget();
	FFMCodexUMGLongShotResolutionViewModel S;
	S.bVisible = S.bNarrativeAvailable = true;
	S.TitleLabel = TEXT("远射"); S.BranchLabel = TEXT("直射死角"); S.ResultTitle = TEXT("射门未进");
	S.NarrativeHeadline = TEXT("诺尔高直射死角未能得分。");
	S.PairedRollResultLabel = TEXT("D6 2 + D6 1 = 3");
	S.OutcomeRollDetail = FMCodexOutcomeText::PairedRollDetail(2, 1).ToString();
	S.PrimaryAction.bVisible = S.PrimaryAction.Action.bAvailable = true;
	S.ContinueActionLabel = TEXT("下一回合");
	Shot->RefreshFromPresentation(S);
	TestEqual(TEXT("Outer paired result uses canonical primary"), Primary(*Shot)->GetText().ToString(), S.NarrativeHeadline);
	TestEqual(TEXT("Outer paired detail uses structured projection"), Text(*Shot,TEXT("OutcomeDetail"))->GetText().ToString(), S.OutcomeRollDetail);
	FFMCodexUMGThroughBallResolutionViewModel T;
	T.bVisible = T.bNarrativeAvailable = true;
	T.TitleLabel = TEXT("直塞"); T.RouteLabel = TEXT("单刀"); T.StageLabel = TEXT("挑射");
	T.ResultTitle = TEXT("进球"); T.NarrativeHeadline = TEXT("哈弗茨挑射破门！");
	T.PrimaryAction = S.PrimaryAction; T.PrimaryAction.Action.Label = TEXT("下一回合");
	Through->RefreshFromPresentation(T);
	TestEqual(TEXT("Chip goal is never converted to goalkeeper prose"), Primary(*Through)->GetText().ToString(), T.NarrativeHeadline);
	TestTrue(TEXT("Shared hosts use identical primary role"), Primary(*Shot)->GetCurrentDefaultTextStyle().Font.Size == Primary(*Through)->GetCurrentDefaultTextStyle().Font.Size);
	T.ResultTitle = TEXT("越位"); T.NarrativeHeadline = TEXT("厄德高送出直塞，哈弗茨越位。");
	T.PrimaryAction.Action.bAvailable = false;
	Through->RefreshFromPresentation(T);
	TestEqual(TEXT("Different canonical terminal replaces old goal"), Primary(*Through)->GetText().ToString(), T.NarrativeHeadline);
	TestFalse(TEXT("ThroughBall retains unavailable gate"), CastChecked<UButton>(Through->GetWidgetFromName(TEXT("ThroughBallPrimaryActionButton")))->GetIsEnabled());
	S.Formula = T.Formula = Detail(); S.bNarrativeAvailable = T.bNarrativeAvailable = false;
	Shot->RefreshFromPresentation(S); Through->RefreshFromPresentation(T);
	for (auto* Host : {static_cast<UUserWidget*>(Shot), static_cast<UUserWidget*>(Through)})
	{
		auto* Inner = Host == Shot ? Shot->GetFormulaSurface() : Through->GetFormulaSurface();
		TestTrue(TEXT("Embedded outcome has one outer shell and clears outer stale prose"),
			!CastChecked<UFMCodexMatchFlowPanel>(Inner->GetWidgetFromName(TEXT("InlineFormulaSurfaceFrame")))->IsFlowStyleEnabled()
			&& Primary(*Host)->GetText().IsEmpty());
		TestEqual(TEXT("Both embedded consumers share primary role"), Primary(*Inner)->GetText().ToString(), FMCodexOutcomePresentation::PrimaryMarkup(S.Formula.ContestLabel, S.Formula.OutcomeText));
	}
	S = {}; T = {}; S.bVisible = T.bVisible = true;
	Shot->RefreshFromPresentation(S); Through->RefreshFromPresentation(T);
	TestTrue(TEXT("Unrelated choice/reveal modes keep prior shell"),
		!CastChecked<UFMCodexMatchFlowPanel>(Shot->GetWidgetFromName(TEXT("LongShotProductionSurfaceFrame")))->IsFlowStyleEnabled()
		&& !CastChecked<UFMCodexMatchFlowPanel>(Through->GetWidgetFromName(TEXT("ThroughBallProductionSurfaceFrame")))->IsFlowStyleEnabled());
	TestTrue(TEXT("Prior CTA treatment is restored on reuse"),
		!CastChecked<UFMCodexMatchFlowButton>(Shot->GetWidgetFromName(TEXT("LongShotPrimaryActionButton")))->IsFlowStyleEnabled()
		&& !CastChecked<UFMCodexMatchFlowButton>(Through->GetWidgetFromName(TEXT("ThroughBallPrimaryActionButton")))->IsFlowStyleEnabled());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexOutcomeSemanticTest,
	"FMCodex.LocalPlay.OutcomeFamily.SemanticSegmentsAndReuse",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexOutcomeSemanticTest::RunTest(const FString&)
{
	using namespace FMCodexOutcomeTests;
	using EBranch = EFMCodexTacticalNarrativeBranch;
	using EOutcome = EMatchPlayResolutionDecisionOutcome;
	using EAccent = EFMCodexOutcomeAccent;
	struct FCase { EBranch Branch; EOutcome Outcome; EAccent Accent; const TCHAR* Sentence; const TCHAR* Keyword; };
	const FCase Cases[] = {
		{ EBranch::LongShotDeadCorner, EOutcome::Goal, EAccent::Goal, TEXT("福登射向死角破门！"), TEXT("破门") },
		{ EBranch::LongShotDeadCorner, EOutcome::Miss, EAccent::NoGoal, TEXT("福登射向死角未能得分。"), TEXT("未能得分") },
		{ EBranch::ThroughBallAntiOffside, EOutcome::Offside, EAccent::NoGoal, TEXT("福登送出直塞，哈兰德越位。"), TEXT("越位") },
		{ EBranch::ThroughBallOneOnOneChip, EOutcome::Goal, EAccent::Goal, TEXT("哈兰德挑射破门！"), TEXT("破门") },
		{ EBranch::ThroughBallOneOnOneChip, EOutcome::Miss, EAccent::NoGoal, TEXT("哈兰德挑射未能得分。"), TEXT("未能得分") },
		{ EBranch::LongShotDirect, EOutcome::ImmediateMiss, EAccent::NoGoal, TEXT("福登远射偏出。"), TEXT("偏出") },
		{ EBranch::CutInsideDirect, EOutcome::ImmediateMiss, EAccent::NoGoal, TEXT("福登内切后射门偏出。"), TEXT("偏出") },
		{ EBranch::CutInsideDeadCorner, EOutcome::Goal, EAccent::Goal, TEXT("福登内切射向死角破门！"), TEXT("破门") },
		{ EBranch::CutInsideDeadCorner, EOutcome::Miss, EAccent::NoGoal, TEXT("福登内切射向死角未能得分。"), TEXT("未能得分") },
		{ EBranch::ThroughBallBehindDefense, EOutcome::OutOfPlay, EAccent::NoGoal, TEXT("福登直塞传出界外。"), TEXT("出界外") },
		{ EBranch::ThroughBallAntiOffside, EOutcome::OneOnOneRequired, EAccent::Neutral, TEXT("福登送出直塞，哈兰德反越位成功，形成单刀！"), TEXT("") }
	};
	auto* W = NewObject<UFMCodexInlineResolutionFormulaSurfaceWidget>(); W->TakeWidget();
	auto P = Detail();
	for (const auto& C : Cases)
	{
		FFMCodexTacticalNarrativePresentationInput Input;
		Input.Branch = C.Branch; Input.AuthorityOutcome = C.Outcome; Input.bAttackEnded = true;
		Input.Carrier.DisplayName = FText::FromString(TEXT("福登"));
		Input.Runner.DisplayName = FText::FromString(TEXT("哈兰德"));
		Input.Goalkeeper.DisplayName = FText::FromString(TEXT("多纳鲁马"));
		const auto N = FFMCodexTacticalResolutionNarrativePresentationBuilder::Build(Input);
		TestEqual(TEXT("Canonical sentence unchanged by segmentation"), N.NarrativeText.ToString(), FString(C.Sentence));
		TestTrue(TEXT("Typed outcome sets expected local accent"), N.OutcomeText.Accent == C.Accent);
		TestEqual(TEXT("Only explicitly authored keyword is highlighted"), N.OutcomeText.Keyword.ToString(), FString(C.Keyword));
		TestTrue(TEXT("No invented keeper causality"), N.DefensivePerformerRole != EMatchPlayResolutionParticipantRole::Goalkeeper);
		if (C.Accent != EAccent::Neutral)
			TestEqual(TEXT("Segments join to the entire canonical sentence"), N.OutcomeText.ToText().ToString(), N.NarrativeText.ToString());
		P.ContestLabel = N.NarrativeText.ToString(); P.OutcomeText = N.OutcomeText;
		W->RefreshFromPresentation(P);
		const auto Markup = Primary(*W)->GetText().ToString();
		TestEqual(TEXT("Goal to NoGoal to neutral reuse clears prior run roles"), Markup,
			FMCodexOutcomePresentation::PrimaryMarkup(P.ContestLabel, N.OutcomeText));
		TestTrue(TEXT("Neutral has no stale color tag"), C.Accent != EAccent::Neutral || Markup == C.Sentence);
		Input.bAttackEnded = false;
		const auto Continuing = FFMCodexTacticalResolutionNarrativePresentationBuilder::Build(Input);
		TestTrue(TEXT("A non-goal without a terminal fact never gets attack-end color"),
			C.Accent == EAccent::Goal || Continuing.OutcomeText.Accent == EAccent::Neutral);
	}
	// Read-only safe-view fixtures exercise all compact projection consumers;
	// supplied Goal facts are not recomputed from the sample dice.
	for (const auto Type : {ESetPieceSelectedType::ShortFreeKick, ESetPieceSelectedType::LongFreeKick, ESetPieceSelectedType::Penalty})
	{
		for (const bool bGoal : {false, true})
		{
			FFMCodexLocalMatchInteractionView V;
			V.bMatchActive = V.bCurrentAttackActive = V.bTerminalPendingAdvance = true;
			V.RouteKind = EMatchPlayCurrentAttackRouteKind::SetPiece;
			V.SetPieceType = Type; V.bHasSetPieceOutcome = V.bHasSetPiecePairedD6 = true;
			V.bSetPieceGoal = bGoal; V.SetPiecePairedD6A = 3; V.SetPiecePairedD6B = 1;
			V.CurrentAttackingPlayer = EInitialTurnOrderPlayer::PlayerA;
			V.SetPieceCarrier.bIsBound = true; V.SetPieceCarrier.OwnerSide = V.CurrentAttackingPlayer;
			V.SetPieceCarrier.CardId = TEXT("OutcomeFixture");
			FFMCodexLocalMatchCardView Card; Card.CardId = V.SetPieceCarrier.CardId; Card.DisplayLabel = TEXT("诺尔高");
			V.PlayerACardRoster.Add(Card);
			const auto Projected = FFMCodexLocalMatchUMGPresentationBuilder::Build(V, {}, FString()).InlineFormula;
			TestTrue(TEXT("Compact canonical projection is available"), Projected.bNarrativeAvailable && !Projected.bShowFormulaRows);
			TestTrue(TEXT("Compact semantic comes from supplied outcome, not dice arithmetic"), Projected.OutcomeText.Accent == (bGoal ? EAccent::Goal : EAccent::NoGoal));
			TestEqual(TEXT("Compact segments preserve whole canonical output"), Projected.OutcomeText.ToText().ToString(), Projected.NarrativeHeadline);
			const FString Expected = Type == ESetPieceSelectedType::ShortFreeKick
				? bGoal ? TEXT("诺尔高近距离任意球战术配合破门！") : TEXT("诺尔高近距离任意球战术配合未能形成进球。")
				: Type == ESetPieceSelectedType::LongFreeKick
					? bGoal ? TEXT("诺尔高远距离任意球重炮轰门得手！") : TEXT("诺尔高远距离任意球重炮轰门未能得分。")
					: bGoal ? TEXT("诺尔高勺子点球命中！") : TEXT("诺尔高勺子点球未能命中。");
			TestEqual(TEXT("Compact sentence keeps prior canonical wording"), Projected.NarrativeHeadline, Expected);
			if (Type != ESetPieceSelectedType::Penalty)
				TestEqual(TEXT("Compact pair detail uses same family formatter"), Projected.OutcomeRollDetail, FString(TEXT("首次掷点 3 + 第二次掷点 1 = 4")));
			else TestTrue(TEXT("Panenka does not invent a second die"), Projected.OutcomeRollDetail.IsEmpty());
		}
	}
	TestEqual(TEXT("Pair presentation uses structured values"), FMCodexOutcomeText::PairedRollDetail(3, 1).ToString(),
		FString(TEXT("首次掷点 3 + 第二次掷点 1 = 4")));
	TestEqual(TEXT("Words alone never classify outcome"), FMCodexOutcomePresentation::PrimaryMarkup(TEXT("进球未能得分偏出扑救"), {}), FString(TEXT("进球未能得分偏出扑救")));
	P.OutcomeText = FFMCodexOutcomeText(FText::FromString(TEXT("<Goal> & 福登远射")),
		FText::FromString(TEXT("偏出")), FText::FromString(TEXT("。")), EAccent::NoGoal);
	TestEqual(TEXT("Actor text cannot introduce rich-text roles"), FMCodexOutcomePresentation::PrimaryMarkup(P.OutcomeText.ToText().ToString(), P.OutcomeText),
		FString(TEXT("&lt;Goal&gt; &amp; 福登远射<NoGoal>偏出</>。")));
	TestEqual(TEXT("Mismatched reused fragments fall back to complete neutral text"),
		FMCodexOutcomePresentation::PrimaryMarkup(TEXT("其他规范结果。"), P.OutcomeText), FString(TEXT("其他规范结果。")));
	auto* Styles = Primary(*W)->GetTextStyleSet();
	const auto* GoalStyle = Styles->FindRow<FRichTextStyleRow>(TEXT("Goal"), TEXT("Outcome test"));
	const auto* MissStyle = Styles->FindRow<FRichTextStyleRow>(TEXT("NoGoal"), TEXT("Outcome test"));
	TestTrue(TEXT("Distinct restrained semantic colors installed"), GoalStyle && MissStyle
		&& GoalStyle->TextStyle.ColorAndOpacity.GetSpecifiedColor().Equals(FLinearColor::FromSRGBColor(FColor(102,217,183)))
		&& MissStyle->TextStyle.ColorAndOpacity.GetSpecifiedColor().Equals(FLinearColor::FromSRGBColor(FColor(232,163,109))));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexOutcomeTransitionTest,
	"FMCodex.LocalPlay.OutcomeFamily.TransitionOwnership",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexOutcomeTransitionTest::RunTest(const FString&)
{
	using namespace FMCodexOutcomeTests;
	auto* Inline = NewObject<UFMCodexInlineResolutionFormulaSurfaceWidget>(); Inline->TakeWidget();
	auto* Shot = NewObject<UFMCodexLongShotResolutionSurfaceWidget>(); Shot->TakeWidget();
	auto* Through = NewObject<UFMCodexThroughBallResolutionSurfaceWidget>(); Through->TakeWidget();
	// Exact former gap: narrative is disclosed while ResultHold still displays a reel.
	// Check each refresh result; there is no frame rate, wall-clock delay or new timer here.
	for (int32 State = 0; State < 4; ++State)
	{
		const bool bNarrative = State == 1 || State == 2, bRoll = State != 2;
		auto P = Detail(); P.bNarrativeAvailable = bNarrative; P.bDiceRevealVisible = bRoll;
		P.PrimaryAction.bVisible = true; // Even a stale available CTA must be hidden during a reel.
		for (const auto Id : {TEXT("SetPiece.Compact"), TEXT("LongShot.DirectShot"), TEXT("CutInsideShot.DirectShot"), TEXT("ThroughBall.BehindDefense.P1")})
		{
			P.ContestId = Id; Inline->RefreshFromPresentation(P);
			TestTrue(TEXT("Inline has one flow frame throughout transition"), CastChecked<UFMCodexMatchFlowPanel>(Inline->GetWidgetFromName(TEXT("InlineFormulaSurfaceFrame")))->IsFlowStyleEnabled());
			TestTrue(TEXT("Inline final content waits until existing reel is gone"), !Primary(*Inline)->GetText().IsEmpty() == (bNarrative && !bRoll));
			TestTrue(TEXT("Intermediate ownership and CTA cannot leak final content"),
				(Inline->GetWidgetFromName(TEXT("OutcomeIntermediate"))->GetVisibility() != ESlateVisibility::Collapsed) == bRoll
				&& (Inline->GetWidgetFromName(TEXT("InlineFormulaContinueButton"))->GetParent()->GetVisibility() == ESlateVisibility::Collapsed) == bRoll);
			TestTrue(TEXT("Inline keeps the existing reel lifetime"), (Inline->GetWidgetFromName(TEXT("InlineFormulaDiceRevealRegion"))->GetVisibility() != ESlateVisibility::Collapsed) == bRoll);
			if (bNarrative) TestTrue(TEXT("No legacy inline result is exposed during hold"), Text(*Inline,TEXT("InlineFormulaContestHeading"))->GetVisibility() == ESlateVisibility::Collapsed);
		}
		FFMCodexUMGLongShotResolutionViewModel S;
		S.bVisible = true; S.bNarrativeAvailable = bNarrative; S.bDiceRevealVisible = bRoll;
		S.NarrativeHeadline = P.ContestLabel; S.OutcomeText = P.OutcomeText; S.PrimaryAction = P.PrimaryAction;
		S.OutcomeRollDetail = P.OutcomeRollDetail;
		Shot->RefreshFromPresentation(S);
		TestTrue(TEXT("DeadCorner clears prior final title throughout roll"), Primary(*Shot)->GetText().IsEmpty() == bRoll);
		TestTrue(TEXT("DeadCorner has continuous family shell"), CastChecked<UFMCodexMatchFlowPanel>(Shot->GetWidgetFromName(TEXT("LongShotProductionSurfaceFrame")))->IsFlowStyleEnabled());
		TestTrue(TEXT("DeadCorner reel is neither dropped early nor left stale"), (Shot->GetWidgetFromName(TEXT("LongShotDiceRevealRegion"))->GetVisibility() != ESlateVisibility::Collapsed) == bRoll);
		if (bNarrative) TestTrue(TEXT("DeadCorner original result text stays collapsed"), Text(*Shot,TEXT("LongShotNarrative"))->GetVisibility() == ESlateVisibility::Collapsed);
		FFMCodexUMGThroughBallResolutionViewModel T;
		T.bVisible = true; T.bNarrativeAvailable = bNarrative; T.bDiceRevealVisible = bRoll;
		T.NarrativeHeadline = P.ContestLabel; T.OutcomeText = P.OutcomeText; T.PrimaryAction = P.PrimaryAction;
		for (const auto Stage : {EFMCodexUMGThroughBallStage::AntiOffsideCheck, EFMCodexUMGThroughBallStage::OneOnOneResolution})
		{
			T.Stage = Stage; Through->RefreshFromPresentation(T);
			TestTrue(TEXT("ThroughBall clears prior final title throughout roll"), Primary(*Through)->GetText().IsEmpty() == bRoll);
			TestTrue(TEXT("AntiOffside and Chip have continuous family shell"), CastChecked<UFMCodexMatchFlowPanel>(Through->GetWidgetFromName(TEXT("ThroughBallProductionSurfaceFrame")))->IsFlowStyleEnabled());
			TestTrue(TEXT("ThroughBall keeps existing reel lifetime"), (Through->GetWidgetFromName(TEXT("ThroughBallInitialRouteRevealRegion"))->GetVisibility() != ESlateVisibility::Collapsed) == bRoll);
			if (bNarrative) TestTrue(TEXT("ThroughBall old result stays collapsed"), Text(*Through,TEXT("ThroughBallOutcomeNarrative"))->GetVisibility() == ESlateVisibility::Collapsed);
		}
		S.Formula = T.Formula = P; S.bNarrativeAvailable = T.bNarrativeAvailable = false;
		T.RouteResultLabel = TEXT("路线掷点 3");
		Shot->RefreshFromPresentation(S); Through->RefreshFromPresentation(T);
		TestTrue(TEXT("Retained parent route context does not expose a legacy gray band"),
			CastChecked<UFMCodexMatchFlowPanel>(Through->GetWidgetFromName(TEXT("ThroughBallInitialRouteRevealRegion")))->GetFormulaRole() == EFMCodexFormulaPanelRole::RollHost);
		for (auto* Host : {static_cast<UUserWidget*>(Shot), static_cast<UUserWidget*>(Through)})
		{
			auto* Child = Host == Shot ? Shot->GetFormulaSurface() : Through->GetFormulaSurface();
			TestTrue(TEXT("Nested outcome has no duplicate outer prose"), Primary(*Host)->GetText().IsEmpty());
			TestFalse(TEXT("Nested outcome retains exactly one outer shell"), CastChecked<UFMCodexMatchFlowPanel>(Child->GetWidgetFromName(TEXT("InlineFormulaSurfaceFrame")))->IsFlowStyleEnabled());
			TestTrue(TEXT("Nested outcome observes same local final ownership"), !Primary(*Child)->GetText().IsEmpty() == (bNarrative && !bRoll));
		}
	}
	for (const auto Id : {TEXT("SetPiece.Type"), TEXT("Cross.Route"), TEXT("Corner.Route"), TEXT("SetPiece.Opposed")})
	{
		auto P = Detail(); P.ContestId = Id; P.bNarrativeAvailable = false; P.bDiceRevealVisible = true;
		TestFalse(TEXT("Initial route and exceptional hosts are outside migration"), FMCodexOutcomePresentation::OwnsInlineSurface(P));
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexOutcomeCenterAxisTest,
	"FMCodex.LocalPlay.OutcomeFamily.CenterAxis",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexOutcomeCenterAxisTest::RunTest(const FString&)
{
	using namespace FMCodexOutcomeTests;
	auto* W = NewObject<UFMCodexInlineResolutionFormulaSurfaceWidget>();
	const auto Slate = W->TakeWidget();
	auto* Renderer = new FWidgetRenderer(true);
	for (const float Width : {808.f, 960.f})
	{
		for (const bool bLong : {false, true})
		{
			auto P = Detail();
			if (bLong) { P.ContestLabel = TEXT("亚历山大·阿诺德与刘易斯·斯凯利的远距离任意球进攻被防守方化解，本次进攻未能得分。"); P.OutcomeText = {}; }
			W->RefreshFromPresentation(P);
			auto* Target = Renderer->DrawWidget(Slate, FVector2D(Width, 700));
			for (int32 Pass=0; Pass<3; ++Pass) Renderer->DrawWidget(Target, Slate, FVector2D(Width, 700), 0.f);
			auto CenterX = [](UWidget* Widget)
			{
				const auto& G = Widget->GetCachedGeometry();
				return G.LocalToAbsolute(G.GetLocalSize() * .5f).X;
			};
			const float Axis = CenterX(W->GetWidgetFromName(TEXT("InlineFormulaSurfaceBounds")));
			for (const auto Name : {TEXT("OutcomePrimary"), TEXT("OutcomeContext"), TEXT("OutcomeDetailRegion"), TEXT("InlineFormulaContinueButton")})
			{
				const float Delta = FMath::Abs(CenterX(W->GetWidgetFromName(Name)) - Axis);
				TestTrue(FString::Printf(TEXT("%s center agrees with frame at width %.0f long=%d (delta %.3f)"), Name, Width, bLong, Delta), Delta <= 1.f);
				AddInfo(FString::Printf(TEXT("OUTCOME_AXIS width=%.0f long=%d item=%s delta=%.3f"), Width, bLong, Name, Delta));
			}
		}
	}
	BeginCleanup(Renderer);
	return true;
}

#endif

#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && !UE_BUILD_SHIPPING
#include "HAL/FileManager.h"
#include "ImageUtils.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

// Static review of the shared final layout. No world, controller, RNG or PIE.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexOutcomeStaticReviewTest,
	"FMCodex.LocalPlay.OutcomeFamily.StaticReview",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexOutcomeStaticReviewTest::RunTest(const FString&)
{
	auto* Renderer = new FWidgetRenderer(true);
	auto Capture = [&](UUserWidget& W, const TCHAR* File)
	{
		// Render the actual widget at its full desired size. Desktop window bounds
		// can crop a tall two-row Formula; fixture evidence must include its CTA.
		const auto Slate = W.TakeWidget();
		auto MakeTarget = [](int32 Width, int32 Height)
		{
			auto* Target = NewObject<UTextureRenderTarget2D>();
			Target->ClearColor = FLinearColor::Transparent;
			// Slate applies display gamma once. A linear render target prevents
			// the GPU target from applying a second sRGB conversion.
			Target->InitCustomFormat(Width, Height, PF_B8G8R8A8, true);
			Target->UpdateResourceImmediate(true);
			return Target;
		};
		auto* MeasureTarget = MakeTarget(808, 1000);
		for (int32 Pass=0; Pass<3; ++Pass)
			Renderer->DrawWidget(MeasureTarget, Slate, FVector2D(808, 1000), 0.f);
		const FIntVector Size(808, FMath::CeilToInt(Slate->GetDesiredSize().Y), 1);
		auto* Target = MakeTarget(Size.X, Size.Y);
		Renderer->DrawWidget(Target, Slate, FVector2D(Size.X, Size.Y), 0.f);
		TArray<FColor> Pixels;
		// Match the native Slate capture path; keep display-space colors unchanged.
		FReadSurfaceDataFlags ReadFlags(RCM_UNorm); ReadFlags.SetLinearToGamma(false);
		if (!TestTrue(TEXT("Complete native widget rendered"), Target->GameThread_GetRenderTargetResource()->ReadPixels(Pixels, ReadFlags))) return;
		TestEqual(TEXT("Static evidence includes every row and footer"), Pixels.Num(), Size.X * Size.Y);
		const FString Dir = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()/TEXT("Stage8_7E_2"));
		IFileManager::Get().MakeDirectory(*Dir, true);
		TArray64<uint8> PNG; FImageUtils::PNGCompressImageArray(Size.X, Size.Y, Pixels, PNG);
		TestTrue(TEXT("Final static evidence saved"), FFileHelper::SaveArrayToFile(PNG, *(Dir/File)));
		AddInfo(FString::Printf(TEXT("OUTCOME_STATIC %s %dx%d (fixture, not PIE)"), File, Size.X, Size.Y));
	};
	auto* W = NewObject<UFMCodexInlineResolutionFormulaSurfaceWidget>(); W->TakeWidget();
	auto P = FMCodexOutcomeTests::Detail(); W->RefreshFromPresentation(P);

	P.ContestLabel = TEXT("福登远射偏出。"); P.StatusLabel = TEXT("直接射门 · 射门偏出"); P.RouteResultLabel.Empty(); P.OutcomeRollDetail.Empty();
	FFMCodexTacticalNarrativePresentationInput Input;
	Input.Branch = EFMCodexTacticalNarrativeBranch::LongShotDirect;
	Input.AuthorityOutcome = EMatchPlayResolutionDecisionOutcome::ImmediateMiss;
	Input.bAttackEnded = true;
	Input.Carrier.DisplayName = FText::FromString(TEXT("福登"));
	P.OutcomeText = FFMCodexTacticalResolutionNarrativePresentationBuilder::Build(Input).OutcomeText;
	W->RefreshFromPresentation(P); Capture(*W, TEXT("Outcome_SimpleMiss_After.png"));
	auto* T = NewObject<UFMCodexThroughBallResolutionSurfaceWidget>(); T->TakeWidget();
	FFMCodexUMGThroughBallResolutionViewModel TP;
	TP.bVisible = TP.bNarrativeAvailable = true;
	TP.TitleLabel = TEXT("直塞"); TP.RouteLabel = TEXT("单刀"); TP.StageLabel = TEXT("挑射");
	Input.Branch = EFMCodexTacticalNarrativeBranch::ThroughBallOneOnOneChip;
	Input.AuthorityOutcome = EMatchPlayResolutionDecisionOutcome::Goal;
	TP.OutcomeText = FFMCodexTacticalResolutionNarrativePresentationBuilder::Build(Input).OutcomeText;
	TP.NarrativeHeadline = TP.OutcomeText.ToText().ToString(); TP.ResultTitle = TEXT("进球"); TP.PrimaryAction = P.PrimaryAction;
	T->RefreshFromPresentation(TP); Capture(*T, TEXT("Outcome_Goal_Fixture.png"));
	Input.Branch = EFMCodexTacticalNarrativeBranch::ThroughBallAntiOffside;
	Input.AuthorityOutcome = EMatchPlayResolutionDecisionOutcome::Offside;
	Input.Runner.DisplayName = FText::FromString(TEXT("哈兰德"));
	const auto Offside = FFMCodexTacticalResolutionNarrativePresentationBuilder::Build(Input);
	P.ContestLabel = Offside.NarrativeText.ToString(); P.OutcomeText = Offside.OutcomeText;
	P.StatusLabel = TEXT("直塞 · 反越位 · 越位");
	W->RefreshFromPresentation(P); Capture(*W, TEXT("Outcome_OffsideOrOut_Fixture.png"));

	Input.Branch = EFMCodexTacticalNarrativeBranch::LongShotDirect;
	Input.AuthorityOutcome = EMatchPlayResolutionDecisionOutcome::Miss;
	Input.Carrier.DisplayName = FText::FromString(TEXT("哲凯赖什"));
	Input.Marker.DisplayName = FText::FromString(TEXT("斯通斯"));
	const auto Defense = FFMCodexTacticalResolutionNarrativePresentationBuilder::Build(Input);
	TestEqual(TEXT("Formula sentence remains canonical"), Defense.NarrativeText.ToString(), FString(TEXT("斯通斯完成抢断，哲凯赖什的远射未能破门。")));
	P.ContestLabel = Defense.NarrativeText.ToString(); P.OutcomeText = Defense.OutcomeText;
	P.StatusLabel = TEXT("直接射门 · 防守成功"); P.bShowFormulaRows = true;
	P.AttackRow.SideLabel = TEXT("进攻"); P.DefenseRow.SideLabel = TEXT("防守");
	for (auto* Row : {&P.AttackRow, &P.DefenseRow})
	{
		const bool bAttack = Row == &P.AttackRow;
		Row->bDisplayedResultResolved = Row->bDisplayedResultIsFinalValue = true;
		Row->KnownNonRollSubtotalLabel = bAttack ? TEXT("基础值 4") : TEXT("基础值 7");
		Row->DisplayedResultLabel = bAttack ? TEXT("7") : TEXT("11");
		FFMCodexUMGInlineFormulaParticipantViewModel Actor;
		Actor.RoleLabel = bAttack ? TEXT("持球") : TEXT("盯人");
		Actor.PlayerName = bAttack ? TEXT("哲凯赖什") : TEXT("斯通斯"); Row->Participants.Add(Actor);
		FFMCodexUMGInlineFormulaTermViewModel Attribute;
		Attribute.DisplayLabel = bAttack ? TEXT("哲凯赖什 远射 4") : TEXT("斯通斯 抢断 5"); Row->Terms.Add(Attribute);
		FFMCodexUMGInlineFormulaTermViewModel Roll;
		Roll.Kind = EFMCodexUMGInlineFormulaTermKind::RawRoll;
		Roll.DisplayLabel = bAttack ? TEXT("掷点 3") : TEXT("掷点 4"); Roll.bResolved = true; Row->Terms.Add(Roll);
		if (!bAttack) { Attribute.DisplayLabel = TEXT("+2"); Row->Terms.Add(Attribute); }
	}
	W->RefreshFromPresentation(P);
	TestEqual(TEXT("Formula-linked uses the shared keyword renderer"),
		CastChecked<URichTextBlock>(W->GetWidgetFromName(TEXT("InlineFormulaOutcomeHeading")))->GetText().ToString(),
		FString(TEXT("斯通斯完成抢断，哲凯赖什的远射<NoGoal>未能破门</>。")));
	Capture(*W, TEXT("Outcome_FormulaLinked_NoGoal.png"));
	P.bNarrativeAvailable = false; P.bDiceRevealVisible = true;
	W->RefreshFromPresentation(P);
	TestTrue(TEXT("Formula headline clears on next reveal without changing rows"),
		CastChecked<URichTextBlock>(W->GetWidgetFromName(TEXT("InlineFormulaOutcomeHeading")))->GetText().IsEmpty()
		&& W->GetRenderedAttackTermCount() == 2 && W->GetRenderedDefenseTermCount() == 3);

	BeginCleanup(Renderer);
	return true;
}
#endif
