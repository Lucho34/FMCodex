#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && !UE_BUILD_SHIPPING
#include "FMCodexFormulaBroadcastPrototype.h"
#include "FMCodexInlineResolutionFormulaSurfaceWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/RichTextBlock.h"
#include "Components/WrapBox.h"
#include "Engine/TextureRenderTarget2D.h"
#include "HAL/FileManager.h"
#include "HAL/IConsoleManager.h"
#include "ImageUtils.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Slate/WidgetRenderer.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SDPIScaler.h"

namespace
{
using K = EFMCodexUMGInlineFormulaTermKind;
FFMCodexUMGInlineFormulaSurfaceViewModel BroadcastFixture()
{
	FFMCodexUMGInlineFormulaSurfaceViewModel P;
	P.bVisible = P.bShowFormulaRows = P.bShowAttackRow = P.bShowDefenseRow = true;
	P.ContestId = TEXT("Cross.High"); P.ContestLabel = TEXT("高球传中");
	P.StatusLabel = TEXT("等待进攻方掷点"); P.bAttackRowActive = true;
	P.PrimaryAction.bVisible = P.PrimaryAction.Action.bAvailable = true;
	P.PrimaryAction.Action.Label = TEXT("进攻方掷点");
	P.PrimaryAction.Action.Category = EFMCodexUMGInteractionCategory::RollCrossAttack;
	for (auto* Row : {&P.AttackRow,&P.DefenseRow})
	{
		const bool A = Row == &P.AttackRow;
		Row->SideLabel = A ? TEXT("进攻") : TEXT("防守");
		Row->DisplayedResultLabel = A ? TEXT("3.5") : TEXT("6"); Row->bDisplayedResultResolved = true;
		Row->FinalValue = 999.f; // Not displayable, even in a visual experiment.
		for (int32 I=0;I<2;++I)
		{
			FFMCodexUMGInlineFormulaParticipantViewModel Person;
			Person.RoleLabel = A ? I ? TEXT("跑位") : TEXT("持球") : I ? TEXT("协防") : TEXT("盯人");
			Person.PlayerName = A ? I ? TEXT("特罗萨德") : TEXT("萨卡") : I ? TEXT("斯通斯") : TEXT("努内斯");
			Row->Participants.Add(Person);
			FFMCodexUMGInlineFormulaTermViewModel Term;
			Term.Kind = K::Attribute; Term.ContributorDisplayName = Person.PlayerName;
			Term.DisplayLabel = I ? TEXT("力量 2 ×0.5") : TEXT("传球 5 ×0.5"); Row->Terms.Add(Term);
		}
		FFMCodexUMGInlineFormulaTermViewModel Roll; Roll.Kind = K::RawRoll; Roll.DisplayLabel = TEXT("掷点 ?");
		Roll.bNextPendingRoll = A; Row->Terms.Add(Roll);
		if (!A) { FFMCodexUMGInlineFormulaTermViewModel M; M.Kind = K::FixedModifier; M.DisplayLabel = TEXT("+2"); Row->Terms.Add(M); }
	}
	return P;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFormulaBroadcastIsolationTest,
	"FMCodex.LocalPlay.FormulaBroadcastPrototype.IsolationContentAndGates",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFormulaBroadcastIsolationTest::RunTest(const FString&)
{
	auto* Mode = IConsoleManager::Get().FindConsoleVariable(TEXT("fm.UI.FormulaV2"));
	if (!TestNotNull(TEXT("DEV compare switch exists"),Mode)) return false;
	const int32 Previous = Mode->GetInt(); Mode->Set(0,ECVF_SetByCode);
	auto* W = NewObject<UFMCodexInlineResolutionFormulaSurfaceWidget>(); W->TakeWidget();
	auto P = BroadcastFixture(); W->RefreshFromPresentation(P);
	TestFalse(TEXT("Opt-out preserves accepted subtree with no prototype allocation"),W->IsBroadcastPrototypeVisible());
	TestNull(TEXT("Prototype is lazy"),W->GetWidgetFromName(TEXT("BroadcastPrototype")));
	Mode->Set(1,ECVF_SetByCode); W->RefreshFromPresentation(P);
	TestTrue(TEXT("Only one surface is visible"),W->IsBroadcastPrototypeVisible()
		&& W->GetWidgetFromName(TEXT("InlineFormulaSurfaceBounds"))->GetVisibility() == ESlateVisibility::Collapsed);
	auto Label = [&](const TCHAR* Name) { return CastChecked<UTextBlock>(W->GetWidgetFromName(Name))->GetText().ToString(); };
	TestEqual(TEXT("Displayed decimal is verbatim; raw final is not used"),Label(TEXT("BroadcastAttackNumber")),FString(TEXT("3.5")));
	TestEqual(TEXT("Ordered identity permits name-free equation"),Label(TEXT("BroadcastAttackTerm0")),P.AttackRow.Terms[0].DisplayLabel);
	TestEqual(TEXT("Second name remains prominent once"),Label(TEXT("BroadcastAttackName1")),P.AttackRow.Participants[1].PlayerName);
	TestEqual(TEXT("Modifier survives unchanged"),Label(TEXT("BroadcastDefenseTerm3")),FString(TEXT("+2")));
	TestTrue(TEXT("Pending relationship remains explicit"),W->GetWidgetFromName(TEXT("BroadcastAttackPending"))->GetVisibility()!=ESlateVisibility::Collapsed);
	TestEqual(TEXT("Broadcast subtitle describes the contest"),Label(TEXT("BroadcastStatus")),FString(TEXT("进球判定")));
	TestEqual(TEXT("Typed roll action gets concise CTA"),Label(TEXT("BroadcastContinueLabel")),FString(TEXT("掷点")));
	TestEqual(TEXT("Typed attack owner is explicit above CTA"),Label(TEXT("BroadcastActionContext")),FString(TEXT("轮到进攻方掷点")));
	P.PrimaryAction.Action.Category = EFMCodexUMGInteractionCategory::RollCrossDefense;
	P.PrimaryAction.Action.Label = TEXT("任意投影文案"); W->RefreshFromPresentation(P);
	TestEqual(TEXT("Owner is never parsed from CTA wording"),Label(TEXT("BroadcastActionContext")),FString(TEXT("轮到防守方掷点")));
	P.PrimaryAction.Action.Category = EFMCodexUMGInteractionCategory::AdvanceAfterTerminal;
	P.PrimaryAction.Action.Label = TEXT("下一回合"); W->RefreshFromPresentation(P);
	TestEqual(TEXT("Progression keeps projected label"),Label(TEXT("BroadcastContinueLabel")),P.PrimaryAction.Action.Label);
	TestEqual(TEXT("Progression has no misleading dice icon"),W->GetWidgetFromName(TEXT("BroadcastDiceIcon"))->GetVisibility(),ESlateVisibility::Collapsed);
	P.AttackRow.Participants[1].PlayerName = P.AttackRow.Participants[0].PlayerName;
	W->RefreshFromPresentation(P);
	TestTrue(TEXT("Ambiguous identity keeps contributor text"),Label(TEXT("BroadcastAttackTerm0")).StartsWith(TEXT("萨卡 ")));
	P = BroadcastFixture(); Swap(P.AttackRow.Participants[0],P.AttackRow.Participants[1]);
	TestFalse(TEXT("Order mismatch cannot remove names"),FMCodexFormulaBroadcastPrototype::CanOmitContributorNames(P.AttackRow));
	P = BroadcastFixture(); P.AttackRow.Terms[0].ContributorDisplayName.Empty();
	TestFalse(TEXT("Missing identity cannot justify omission"),FMCodexFormulaBroadcastPrototype::CanOmitContributorNames(P.AttackRow));
	P = BroadcastFixture();
	for (int32 State=0; State<3; ++State)
	{
		P.bAttackRowActive = State == 0; P.bDefenseRowActive = State == 1;
		P.bNarrativeAvailable = State == 2; P.PrimaryAction.bVisible = P.PrimaryAction.Action.bAvailable = false;
		P.StatusLabel = TEXT("等待玩家 B 操作");
		W->RefreshFromPresentation(P);
		TestTrue(TEXT("Active mark follows owner even with no viewer permission"),
			(W->GetWidgetFromName(TEXT("BroadcastAttackActive"))->GetVisibility()==ESlateVisibility::HitTestInvisible)==(State==0)
			&& (W->GetWidgetFromName(TEXT("BroadcastDefenseActive"))->GetVisibility()==ESlateVisibility::HitTestInvisible)==(State==1));
		TestTrue(TEXT("Waiting viewer has no action button"),W->GetWidgetFromName(TEXT("BroadcastActionBounds"))->GetVisibility()==ESlateVisibility::Collapsed
			&& !CastChecked<UButton>(W->GetWidgetFromName(TEXT("BroadcastContinue")))->GetIsEnabled());
		TestEqual(TEXT("Waiting viewer retains safe projected owner text"),Label(TEXT("BroadcastActionContext")),P.StatusLabel);
	}
	P = BroadcastFixture(); P.bNarrativeAvailable = P.bDiceRevealVisible = true;
	P.ContestLabel = TEXT("不得提前显示的结论"); P.ResolutionContextLabel = TEXT("高球传中");
	W->RefreshFromPresentation(P);
	TestEqual(TEXT("ResultHold remains neutral"),Label(TEXT("BroadcastTitle")),FString(TEXT("高球传中")));
	TestTrue(TEXT("Outcome semantic headline not exposed in reel"),CastChecked<URichTextBlock>(W->GetWidgetFromName(TEXT("BroadcastOutcomeHeading")))->GetText().IsEmpty());
	P.bDiceRevealVisible = false;
	P.AttackRow.bDisplayedResultIsFinalValue = true; P.AttackRow.DisplayedResultLabel = TEXT("7.5");
	P.AttackRow.Terms[2].bResolved = true; P.AttackRow.Terms[2].DisplayLabel = TEXT("掷点 4");
	W->RefreshFromPresentation(P);
	TestEqual(TEXT("Final uses gated result"),Label(TEXT("BroadcastAttackNumber")),FString(TEXT("7.5")));
	TestTrue(TEXT("Final removes pending suffix and keeps actual roll in equation"),
		W->GetWidgetFromName(TEXT("BroadcastAttackPending"))->GetVisibility()==ESlateVisibility::Collapsed
		&& Label(TEXT("BroadcastAttackTerm2")).Contains(TEXT("掷点 4")));
	Mode->Set(0,ECVF_SetByCode); W->RefreshFromPresentation(P);
	TestFalse(TEXT("Live opt-out restores original result surface"),W->IsBroadcastPrototypeVisible());
	TestEqual(TEXT("Baseline final value is preserved"),Label(TEXT("InlineFormulaAttackFinalValue")),FString(TEXT("7.5")));
	Mode->Set(1,ECVF_SetByCode);
	for (auto Id : {TEXT("Cross.Low"),TEXT("SetPiece.Type"),TEXT("SetPiece.Opposed"),TEXT("LongShot.DirectShot"),TEXT("SetPiece.Compact")})
	{
		P = BroadcastFixture(); P.ContestId = Id; W->RefreshFromPresentation(P);
		TestFalse(TEXT("Other families cannot enter V2"),W->IsBroadcastPrototypeVisible());
	}
	P = BroadcastFixture(); W->SetEmbeddedFormulaLayout(true); W->RefreshFromPresentation(P);
	TestFalse(TEXT("Parent-owned embedded layout is protected"),W->IsBroadcastPrototypeVisible());
	Mode->Set(Previous,ECVF_SetByCode);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFormulaBroadcastLayoutTest,
	"FMCodex.LocalPlay.FormulaBroadcastPrototype.AtomicLayoutAndReuse",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFormulaBroadcastLayoutTest::RunTest(const FString&)
{
	auto* Mode = IConsoleManager::Get().FindConsoleVariable(TEXT("fm.UI.FormulaV2"));
	const int32 Previous = Mode->GetInt(); Mode->Set(1,ECVF_SetByCode);
	auto* W = NewObject<UFMCodexInlineResolutionFormulaSurfaceWidget>();
	auto Slate = W->TakeWidget();
	auto* Renderer = new FWidgetRenderer(true);
	// Same logical width under two DPI scales; exercise repeated live text changes.
	for (const float DPI : {1.f,.75f})
	{
		auto Host = SNew(SDPIScaler).DPIScale(DPI)[SNew(SBox).VAlign(VAlign_Top)[Slate]];
		const int32 Width = FMath::RoundToInt(808*DPI), Height = FMath::RoundToInt(1000*DPI);
		auto* Target = NewObject<UTextureRenderTarget2D>();
		Target->ClearColor=FLinearColor::Transparent;
		Target->InitCustomFormat(Width,Height,PF_B8G8R8A8,true); Target->UpdateResourceImmediate(true);
		for (int32 Case=0; Case<3; ++Case)
		{
			auto P = BroadcastFixture();
			P.RouteResultLabel = TEXT("路线掷点 2 → 判定为高球传中");
			P.AttackRow.Participants[0].PlayerName = Case==1 ? TEXT("亚历山大·阿诺德") : TEXT("马杜埃凯");
			P.AttackRow.Participants[1].PlayerName = TEXT("马丁内利");
			P.DefenseRow.Participants[0].PlayerName = TEXT("格瓦迪奥尔");
			for (auto* Row : {&P.AttackRow,&P.DefenseRow})
				for (int32 I=0; I<2; ++I) Row->Terms[I].ContributorDisplayName=Row->Participants[I].PlayerName;
			P.AttackRow.DisplayedResultLabel = TEXT("12.5"); P.DefenseRow.DisplayedResultLabel=TEXT("10");
			if (Case==1)
			{
				// Ordered-identity failure keeps the full long contributor, within the same column.
				Swap(P.AttackRow.Participants[0],P.AttackRow.Participants[1]);
				P.AttackRow.Terms[2].bResolved=true; P.AttackRow.Terms[2].DisplayLabel=TEXT("掷点 6");
				P.bAttackRowActive=false; P.bDefenseRowActive=true;
				P.PrimaryAction.Action.Category=EFMCodexUMGInteractionCategory::RollCrossDefense;
			}
			W->RefreshFromPresentation(P);
			for (int32 Frame=0; Frame<5; ++Frame) Renderer->DrawWidget(Target,Host,FVector2D(Width,Height),0.f);
			auto Inside = [&](UWidget* Child, UWidget* Parent)
			{
				const auto& C=Child->GetCachedGeometry(); const auto& R=Parent->GetCachedGeometry();
				return C.GetAbsolutePosition().X >= R.GetAbsolutePosition().X-1.f
					&& C.GetAbsolutePosition().X+C.GetAbsoluteSize().X <= R.GetAbsolutePosition().X+R.GetAbsoluteSize().X+1.f;
			};
			for (const FString Prefix : {FString(TEXT("BroadcastAttack")),FString(TEXT("BroadcastDefense"))})
			{
				auto* Card = W->GetWidgetFromName(FName(*(Prefix+TEXT("Card"))));
				auto* People = W->GetWidgetFromName(FName(*(Prefix+TEXT("People"))));
				for (int32 I=0; I<2; ++I)
				{
					auto* Name=W->GetWidgetFromName(FName(*(Prefix+FString::Printf(TEXT("Name%d"),I))));
					TestTrue(TEXT("Chinese name stays on one line and within identity column"),Name->GetCachedGeometry().GetLocalSize().Y<45.f && Inside(Name,People));
				}
				auto* Equation=CastChecked<UWrapBox>(W->GetWidgetFromName(FName(*(Prefix+TEXT("Equation")))));
				for (int32 I=0; I<Equation->GetChildrenCount(); ++I)
				{
					if (Equation->GetChildAt(I)->GetVisibility()==ESlateVisibility::Collapsed) continue;
					auto* Term=W->GetWidgetFromName(FName(*(Prefix+FString::Printf(TEXT("Term%d"),I))));
					TestTrue(TEXT("Complete formula operand retains one line including multiplier"),Term->GetCachedGeometry().GetLocalSize().Y<30.f && Inside(Term,Equation));
				}
				auto* ValueCard=W->GetWidgetFromName(FName(*(Prefix+TEXT("ValueCard"))));
				TestTrue(TEXT("Decimal/two-digit number stays in its hero area"),Inside(W->GetWidgetFromName(FName(*(Prefix+TEXT("Number")))),ValueCard) && Inside(ValueCard,Card));
				const auto& CG=Card->GetCachedGeometry(); const auto& VG=ValueCard->GetCachedGeometry();
				const float RightInset=CG.GetAbsolutePosition().X+CG.GetAbsoluteSize().X-VG.GetAbsolutePosition().X-VG.GetAbsoluteSize().X;
				TestTrue(TEXT("Numeric inset remains anchored to the right across changing names"),RightInset>10.f*DPI && RightInset<22.f*DPI);
			}
			TestTrue(TEXT("Dense prototype fits the existing logical height budget"),Slate->GetDesiredSize().Y<950.f);
			auto* VS=W->GetWidgetFromName(TEXT("BroadcastVSChip"));
			auto* Context=W->GetWidgetFromName(TEXT("BroadcastContextChip"));
			auto* Action=W->GetWidgetFromName(TEXT("BroadcastActionBounds"));
			const auto& AG=W->GetWidgetFromName(TEXT("BroadcastAttackCard"))->GetCachedGeometry();
			const auto& DG=W->GetWidgetFromName(TEXT("BroadcastDefenseCard"))->GetCachedGeometry();
			const auto& VG=VS->GetCachedGeometry();
			TestTrue(TEXT("VS has a centered carrier wholly between the opposed cards"),
				VG.GetAbsolutePosition().Y >= AG.GetAbsolutePosition().Y+AG.GetAbsoluteSize().Y
				&& VG.GetAbsolutePosition().Y+VG.GetAbsoluteSize().Y <= DG.GetAbsolutePosition().Y
				&& FMath::Abs(VG.GetAbsolutePosition().X+VG.GetAbsoluteSize().X*.5f-Width*.5f)<6.f);
			TestTrue(TEXT("Action context is compact and ends before the button"),
				Context->GetCachedGeometry().GetAbsoluteSize().X < Width*.8f
				&& Context->GetCachedGeometry().GetAbsolutePosition().Y+Context->GetCachedGeometry().GetAbsoluteSize().Y <= Action->GetCachedGeometry().GetAbsolutePosition().Y);
			if (DPI==1.f && Case<2)
			{
				TArray<FColor> Pixels; FReadSurfaceDataFlags Flags(RCM_UNorm); Flags.SetLinearToGamma(false);
				if (Target->GameThread_GetRenderTargetResource()->ReadPixels(Pixels,Flags))
				{
					// Test the open-shell requirement in actual paint, not just widget types.
					for (const TCHAR* Region : {TEXT("BroadcastHeader"),TEXT("BroadcastFooter")})
					{
						const auto& G=W->GetWidgetFromName(Region)->GetCachedGeometry();
						const int32 X=FMath::RoundToInt(G.GetAbsolutePosition().X+8);
						const int32 Y=FMath::RoundToInt(G.GetAbsolutePosition().Y+G.GetAbsoluteSize().Y*.5f);
						TestTrue(TEXT("Header/footer sides remain transparent to the real pitch"),
							X>=0 && X<Width && Y>=0 && Y<Height && Pixels[Y*Width+X].A<8);
					}
					const FString Dir=FPaths::ProjectSavedDir()/TEXT("Stage8_8C"); IFileManager::Get().MakeDirectory(*Dir,true);
					TArray64<uint8> PNG; FImageUtils::PNGCompressImageArray(Width,Height,Pixels,PNG);
					TestTrue(TEXT("Technical layout fixture saved"),FFileHelper::SaveArrayToFile(PNG,*(Dir/(Case==0 ? TEXT("LayoutFixture_Names.png") : TEXT("LayoutFixture_LongFallback.png")))));
				}
			}
		}
	}
	BeginCleanup(Renderer); Mode->Set(Previous,ECVF_SetByCode);
	return true;
}
#endif
