#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && !UE_BUILD_SHIPPING

#include "FMCodexInlineResolutionFormulaSurfaceWidget.h"
#include "FMCodexMatchFlowPanel.h"
#include "Components/TextBlock.h"
#include "Components/WrapBox.h"
#include "Components/SizeBox.h"
#include "Misc/AutomationTest.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "HAL/FileManager.h"
#include "ImageUtils.h"
#include "Slate/WidgetRenderer.h"
#include "Engine/TextureRenderTarget2D.h"

namespace FMCodexFormulaHierarchyTests
{
using E = EFMCodexFormulaEmphasis;
using R = EFMCodexFormulaComponentRole;
using K = EFMCodexUMGInlineFormulaTermKind;
using W = UFMCodexInlineResolutionFormulaSurfaceWidget;
FFMCodexUMGInlineFormulaSurfaceViewModel Fixture(bool bDense = true)
{
	FFMCodexUMGInlineFormulaSurfaceViewModel P;
	P.bVisible = P.bShowFormulaRows = P.bShowAttackRow = P.bShowDefenseRow = true;
	P.ContestId = bDense ? TEXT("Cross.Low") : TEXT("SetPiece.Opposed");
	P.ContestLabel = bDense ? TEXT("低球传中") : TEXT("近距离任意球 · 直接射门");
	P.StatusLabel = TEXT("等待进攻方掷点");
	P.RouteResultLabel = bDense ? TEXT("路线掷点 2 → 判定为低球传中") : TEXT("");
	P.bAttackRowActive = true;
	P.PrimaryAction.bVisible = P.PrimaryAction.Action.bAvailable = true;
	P.PrimaryAction.Action.Label = TEXT("进攻方掷点");
	P.AttackRow.SideLabel = TEXT("进攻"); P.DefenseRow.SideLabel = TEXT("防守");
	for (auto* Row : {&P.AttackRow, &P.DefenseRow})
	{
		const bool bAttack = Row == &P.AttackRow;
		Row->bDisplayedResultResolved = Row->bKnownNonRollSubtotalResolved = true;
		Row->KnownNonRollSubtotalLabel = bDense ? bAttack ? TEXT("基础值 4.5") : TEXT("基础值 7")
			: bAttack ? TEXT("射门与传球取较高值") : TEXT("门将手控球与防守加成");
		Row->DisplayedResultLabel = bAttack ? bDense ? TEXT("4.5") : TEXT("5") : TEXT("7");
		Row->FinalValue = 999; // Hidden authority value must not influence paint.
		for (int32 I=0; I<(bDense ? 2 : 1); ++I)
		{
			FFMCodexUMGInlineFormulaParticipantViewModel Actor;
			Actor.RoleLabel = bAttack ? I ? TEXT("跑位") : TEXT("持球") : I ? TEXT("协防") : TEXT("盯人");
			Actor.PlayerName = bAttack ? I ? TEXT("萨卡") : TEXT("卡拉菲奥里") : I ? TEXT("斯通斯") : TEXT("格瓦迪奥尔");
			Row->Participants.Add(Actor);
			FFMCodexUMGInlineFormulaTermViewModel T;
			T.Kind = K::Attribute;
			T.ContributorDisplayName = bDense ? Actor.PlayerName : FString();
			T.DisplayLabel = bDense ? bAttack ? I ? TEXT("射门 5 ×0.5") : TEXT("传球 4 ×0.5")
				: I ? TEXT("盯防 5 ×0.5") : TEXT("抢断 5 ×0.5") : bAttack ? TEXT("射门 5") : TEXT("手控球 6");
			Row->Terms.Add(T);
		}
		FFMCodexUMGInlineFormulaTermViewModel Roll;
		Roll.Kind = K::RawRoll; Roll.DisplayLabel = TEXT("掷点 ?"); Roll.bNextPendingRoll = bAttack;
		Row->Terms.Add(Roll);
		if (!bAttack)
		{
			FFMCodexUMGInlineFormulaTermViewModel Modifier;
			Modifier.Kind = K::FixedModifier; Modifier.DisplayLabel = bDense ? TEXT("+2") : TEXT("防守加成 1");
			Row->Terms.Add(Modifier);
		}
	}
	return P;
}
UTextBlock* Text(W& Widget,const TCHAR* Name) { return CastChecked<UTextBlock>(Widget.GetWidgetFromName(Name)); }
UFMCodexMatchFlowPanel* Panel(W& Widget,const TCHAR* Name) { return CastChecked<UFMCodexMatchFlowPanel>(Widget.GetWidgetFromName(Name)); }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFormulaHierarchyRolesTest,
	"FMCodex.LocalPlay.FormulaHierarchy.RolesWaitingAndReuse", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFormulaHierarchyRolesTest::RunTest(const FString&)
{
	using namespace FMCodexFormulaHierarchyTests;
	auto* Widget = NewObject<W>(); Widget->TakeWidget();
	for (const auto Id : {TEXT("SetPiece.Opposed"), TEXT("Cross.High"), TEXT("Cross.Low"), TEXT("PassControl.Pass"),
		TEXT("ThroughBall.Feet"), TEXT("ThroughBall.BehindDefense.P1"), TEXT("LongShot.DirectShot"), TEXT("CutInsideShot.DirectShot")})
	{
		auto P=Fixture(); P.ContestId=Id;
		for (int32 State=0; State<4; ++State)
		{
			P.bAttackRowActive=State==0 || State==3; P.bDefenseRowActive=State==1;
			P.bNarrativeAvailable=State==2;
			P.AttackRow.bDisplayedResultIsFinalValue=P.DefenseRow.bDisplayedResultIsFinalValue=State==2;
			P.AttackRow.Terms[2].bNextPendingRoll=P.bAttackRowActive;
			P.DefenseRow.Terms[2].bNextPendingRoll=P.bDefenseRowActive;
			P.AttackRow.Terms[2].bResolved=P.DefenseRow.Terms[2].bResolved=State==2;
			// Deliberately contradictory text: no label is an ownership input.
			P.PrimaryAction.Action.Label=TEXT("无关标签");
			Widget->RefreshFromPresentation(P);
			const E A=State==2 ? E::Resolved : P.bAttackRowActive ? E::Active : E::Context;
			const E D=State==2 ? E::Resolved : P.bDefenseRowActive ? E::Active : E::Context;
			TestTrue(TEXT("Typed row owner drives both modules across reuse"), Widget->GetRowEmphasis(true)==A && Widget->GetRowEmphasis(false)==D);
			TestTrue(TEXT("Paint state follows role, including removal of old accent"),
				Panel(*Widget,TEXT("InlineFormulaAttackFinalValueRegion"))->GetFormulaEmphasis()==A
				&& Panel(*Widget,TEXT("InlineFormulaDefenseSectionHeader"))->GetFormulaEmphasis()==D);
			TestTrue(TEXT("Final never keeps an active section"), State!=2 ||
				Widget->GetWidgetFromName(TEXT("InlineFormulaAttackActiveMarker"))->GetVisibility()==ESlateVisibility::Collapsed);
			TestTrue(TEXT("Pending component follows typed roll and active owner"),
				W::GetComponentRole(P.AttackRow.Terms[2],A)==(A==E::Active ? R::PendingRoll : R::Roll));
			TestTrue(TEXT("Operand and modifier roles remain distinct"), W::GetComponentRole(P.DefenseRow.Terms[0],D)==R::Operand
				&& W::GetComponentRole(P.DefenseRow.Terms[3],D)==R::Modifier);
			P.PrimaryAction.bVisible=P.PrimaryAction.Action.bAvailable=false;
			Widget->RefreshFromPresentation(P);
			TestTrue(TEXT("Waiting viewer still sees true owner without CTA permission"), Widget->GetRowEmphasis(true)==A
				&& !CastChecked<UButton>(Widget->GetWidgetFromName(TEXT("InlineFormulaContinueButton")))->GetIsEnabled()
				&& Widget->GetWidgetFromName(TEXT("InlineFormulaContinueBounds"))->GetVisibility()==ESlateVisibility::Collapsed);
			TestEqual(TEXT("Context data is never disabled/dimmed as a group"),Widget->GetWidgetFromName(TEXT("InlineFormulaDefenseRegion"))->GetRenderOpacity(),1.f);
			P.PrimaryAction.bVisible=P.PrimaryAction.Action.bAvailable=true;
		}
	}
	auto P=Fixture(); P.bAttackRowActive=P.bDefenseRowActive=true; Widget->RefreshFromPresentation(P);
	TestTrue(TEXT("Ambiguous safe owner stays neutral"),Widget->GetRowEmphasis(true)==E::Context && Widget->GetRowEmphasis(false)==E::Context);
	for (const auto Id : {TEXT("SetPiece.Type"),TEXT("SetPiece.Compact")})
	{
		P=Fixture(); P.ContestId=Id; P.bShowFormulaRows=false; P.bNarrativeAvailable=Id==FName(TEXT("SetPiece.Compact"));
		Widget->RefreshFromPresentation(P);
		TestTrue(TEXT("Type and Outcome never inherit Formula emphasis"),Widget->GetRowEmphasis(true)==E::None
			&& Panel(*Widget,TEXT("InlineFormulaAttackFinalValueRegion"))->GetFormulaEmphasis()==E::None);
	}
	P=Fixture(); P.bNarrativeAvailable=true; P.bAttackRowActive=true; Widget->RefreshFromPresentation(P);
	TestTrue(TEXT("Final narrative overrides stale active flags"),Widget->GetRowEmphasis(true)==E::Resolved);
	TestEqual(TEXT("Accepted Formula-linked identity size restored"),Text(*Widget,TEXT("InlineFormulaAttackParticipantRole0"))->GetFont().Size,14.f);
	TestEqual(TEXT("Accepted Formula-linked base size restored"),Text(*Widget,TEXT("InlineFormulaAttackKnownSubtotal"))->GetFont().Size,17.f);
	P=Fixture(); P.AttackRow.Terms[0].Kind=K::FixedModifier; Widget->RefreshFromPresentation(P);
	const float ModifierSize=Text(*Widget,TEXT("InlineFormulaAttackOperandText0"))->GetFont().Size;
	P.AttackRow.Terms[0].Kind=K::Attribute; Widget->RefreshFromPresentation(P);
	TestTrue(TEXT("Pooled modifier restores full operand weight when its typed role changes"),
		Text(*Widget,TEXT("InlineFormulaAttackOperandText0"))->GetFont().Size>ModifierSize);
	TestTrue(TEXT("Active pending chip has stronger structure than contextual roll"),
		CastChecked<UBorder>(Widget->GetWidgetFromName(TEXT("InlineFormulaAttackOperand2")))->Background.OutlineSettings.Width
		> CastChecked<UBorder>(Widget->GetWidgetFromName(TEXT("InlineFormulaDefenseOperand2")))->Background.OutlineSettings.Width);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFormulaHierarchyDensityTest,
	"FMCodex.LocalPlay.FormulaHierarchy.DensityAndComponents", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFormulaHierarchyDensityTest::RunTest(const FString&)
{
	using namespace FMCodexFormulaHierarchyTests;
	auto* Widget=NewObject<W>(); const auto Slate=Widget->TakeWidget();
	auto* Renderer=new FWidgetRenderer(true);
	auto* Target=NewObject<UTextureRenderTarget2D>();
	Target->InitCustomFormat(808,950,PF_B8G8R8A8,true); Target->UpdateResourceImmediate(true);
	for (bool bDense : {true,false})
	{
		auto P=Fixture(bDense);
		if (bDense) { P.AttackRow.Participants[0].PlayerName=TEXT("亚历山大·阿诺德"); P.AttackRow.Terms[0].ContributorDisplayName=P.AttackRow.Participants[0].PlayerName; }
		Widget->RefreshFromPresentation(P);
		for (int32 I=0;I<3;++I) Renderer->DrawWidget(Target,Slate,FVector2D(808,950),0.f);
		TestTrue(TEXT("Header has three distinct text weights"),Text(*Widget,TEXT("InlineFormulaContestHeading"))->GetFont().Size
			> Text(*Widget,TEXT("InlineFormulaStatus"))->GetFont().Size && Text(*Widget,TEXT("InlineFormulaStatus"))->GetFont().Size
			> Text(*Widget,TEXT("InlineFormulaRouteResult"))->GetFont().Size);
		TestTrue(TEXT("Current value is larger than operand and base"),Text(*Widget,TEXT("InlineFormulaAttackFinalValue"))->GetFont().Size
			> Text(*Widget,TEXT("InlineFormulaAttackOperandText0"))->GetFont().Size && Text(*Widget,TEXT("InlineFormulaAttackFinalValue"))->GetFont().Size
			> Text(*Widget,TEXT("InlineFormulaAttackKnownSubtotal"))->GetFont().Size);
		TestEqual(TEXT("Decimal format preserved verbatim"),Text(*Widget,TEXT("InlineFormulaAttackFinalValue"))->GetText().ToString(),P.AttackRow.DisplayedResultLabel);
		for (const auto Prefix : {FString(TEXT("InlineFormulaAttack")),FString(TEXT("InlineFormulaDefense"))})
		{
			auto* Terms=CastChecked<UWrapBox>(Widget->GetWidgetFromName(FName(*(Prefix+TEXT("Terms")))));
			for (auto* Item : Terms->GetAllChildren())
				TestTrue(TEXT("Whole operands including multiplier fit the reading column"),Item->GetCachedGeometry().GetLocalSize().X <= Terms->GetCachedGeometry().GetLocalSize().X+.5f);
		}
		const float Height=Slate->GetDesiredSize().Y;
		P.bAttackRowActive=false; P.bDefenseRowActive=true;
		Widget->RefreshFromPresentation(P);
		for (int32 I=0;I<3;++I) Renderer->DrawWidget(Target,Slate,FVector2D(808,950),0.f);
		TestTrue(TEXT("Changing active owner alone does not change layout height"),FMath::Abs(Slate->GetDesiredSize().Y-Height)<.5f);
		P.bAttackRowActive=true; P.bDefenseRowActive=false; Widget->RefreshFromPresentation(P);
		for (int32 I=0;I<3;++I) Renderer->DrawWidget(Target,Slate,FVector2D(808,950),0.f);
		if (!bDense)
		{
			TestTrue(TEXT("Simple formula remains content-fit"),Height<720.f);
			auto* Capture=NewObject<UTextureRenderTarget2D>(); const int32 H=FMath::CeilToInt(Height);
			Capture->InitCustomFormat(808,H,PF_B8G8R8A8,true); Capture->UpdateResourceImmediate(true);
			Renderer->DrawWidget(Capture,Slate,FVector2D(808,H),0.f);
			TArray<FColor> Pixels; FReadSurfaceDataFlags Flags(RCM_UNorm); Flags.SetLinearToGamma(false);
			if (TestTrue(TEXT("Low-density fixture rendered"),Capture->GameThread_GetRenderTargetResource()->ReadPixels(Pixels,Flags)))
			{
				const FString Dir=FPaths::ProjectSavedDir()/TEXT("Stage8_7F"); IFileManager::Get().MakeDirectory(*Dir,true);
				TArray64<uint8> PNG; FImageUtils::PNGCompressImageArray(808,H,Pixels,PNG);
				TestTrue(TEXT("Low-density fixture saved"),FFileHelper::SaveArrayToFile(PNG,*(Dir/TEXT("Formula_LowDensity_Fixture.png"))));
			}
		}
	}
	BeginCleanup(Renderer);
	return true;
}
#endif
