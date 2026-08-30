#include "FMCodexLocalMatchInteractionView.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "FMCodexLocalMatchDemoConfiguration.h"
#include "FMCodexLocalMatchResolutionFeedback.h"
#include "FMCodexLocalMatchUMGPresentation.h"
#include "FMCodexPrototypeTeamContent.h"

#include "Misc/AutomationTest.h"

namespace FMCodexRecoveryPresentationTests
{
	FMatchPlayState MakeInitializedState(
		FAutomationTestBase& Test,
		FSkillRuleSnapshotSet& OutRules)
	{
		const FFMCodexLocalMatchDemoConfiguration Configuration =
			FFMCodexLocalMatchDemoConfigurationFactory::Create();
		OutRules = Configuration.SkillRuleSet;
		const FMatchPlayOpeningInitializeResult Initialized =
			FMatchPlayOpeningInitializer::InitializeMatchPlayOpening(
				Configuration.OpeningInput);
		Test.TestTrue(TEXT("Presentation fixture initializes"),
			Initialized.bSuccess);
		return Initialized.MatchPlayState;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexRecoveryPresentationProjectionTest,
	"FMCodex.LocalPlay.RecoveryPresentation.01.DataDrivenIdentityAndOrder",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexRecoveryPresentationProjectionTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexRecoveryPresentationTests;
	FSkillRuleSnapshotSet Rules;
	FMatchPlayState State = MakeInitializedState(*this, Rules);
	const FName PlayerACard =
		State.CardSnapshotAuthority.PlayerACardSnapshots.Cards[0].CardId;
	const FName PlayerBCard =
		State.CardSnapshotAuthority.PlayerBCardSnapshots.Cards[0].CardId;
	State.LastRecoveryFact.bHasRecoveryFact = true;
	State.LastRecoveryFact.SourceAttackSequence = 11;
	State.LastRecoveryFact.ReturnedCards = {
		{ EInitialTurnOrderPlayer::PlayerB, PlayerBCard },
		{ EInitialTurnOrderPlayer::PlayerA, PlayerACard }
	};

	const FFMCodexLocalMatchInteractionView View =
		FFMCodexLocalMatchInteractionViewBuilder::Build(State, Rules);
	TestTrue(TEXT("Recovery fact reconstructs"), View.bHasRecoveryFact);
	TestEqual(TEXT("Recovery source sequence reconstructs"),
		View.RecoverySourceAttackSequence, int64(11));
	TestEqual(TEXT("Two returned entries reconstruct"),
		View.RecoveryPresentationEntries.Num(), 2);
	if (View.RecoveryPresentationEntries.Num() == 2)
	{
		const FString ExpectedB = FString::Printf(
			TEXT("玩家B · %s"),
			*FFMCodexPrototypeTeamContent::PlayerDisplayName(PlayerBCard).ToString());
		const FString ExpectedA = FString::Printf(
			TEXT("玩家A · %s"),
			*FFMCodexPrototypeTeamContent::PlayerDisplayName(PlayerACard).ToString());
		TestEqual(TEXT("Authoritative order entry 0"),
			View.RecoveryPresentationEntries[0].PresentationLine, ExpectedB);
		TestEqual(TEXT("Authoritative order entry 1"),
			View.RecoveryPresentationEntries[1].PresentationLine, ExpectedA);
		TestEqual(TEXT("Entry 0 keeps actual owner side"),
			View.RecoveryPresentationEntries[0].OwnerSide,
			EInitialTurnOrderPlayer::PlayerB);
		TestEqual(TEXT("Entry 0 localizes actual owner side"),
			View.RecoveryPresentationEntries[0].OwnerDisplayName.ToString(),
			FString(TEXT("玩家B")));
		TestFalse(TEXT("Rows do not repeat the event title"),
			View.RecoveryPresentationEntries[0].PresentationLine.Contains(
				TEXT("返回手牌")));
		TestFalse(TEXT("Rows never use the roster club as ownership"),
			View.RecoveryPresentationEntries[0].PresentationLine.Contains(
				FFMCodexPrototypeTeamContent::TeamDisplayName(PlayerBCard).ToString()));
	}

	FMatchPlayState SameSide = State;
	const FName SecondPlayerBCard =
		State.CardSnapshotAuthority.PlayerBCardSnapshots.Cards[1].CardId;
	SameSide.LastRecoveryFact.ReturnedCards = {
		{ EInitialTurnOrderPlayer::PlayerB, PlayerBCard },
		{ EInitialTurnOrderPlayer::PlayerB, SecondPlayerBCard }
	};
	const FFMCodexLocalMatchInteractionView SameSideView =
		FFMCodexLocalMatchInteractionViewBuilder::Build(SameSide, Rules);
	TestEqual(TEXT("Two same-side returns preserve authoritative count"),
		SameSideView.RecoveryPresentationEntries.Num(), 2);
	if (SameSideView.RecoveryPresentationEntries.Num() == 2)
	{
		TestTrue(TEXT("Both same-side rows use PlayerB owner identity"),
			SameSideView.RecoveryPresentationEntries[0].PresentationLine
				.StartsWith(TEXT("玩家B · "))
			&& SameSideView.RecoveryPresentationEntries[1].PresentationLine
				.StartsWith(TEXT("玩家B · ")));
	}

	FMatchPlayState Swapped = State;
	Swap(
		Swapped.CardSnapshotAuthority.PlayerACardSnapshots,
		Swapped.CardSnapshotAuthority.PlayerBCardSnapshots);
	Swap(
		Swapped.CardUsageState.PlayerACardUsageState,
		Swapped.CardUsageState.PlayerBCardUsageState);
	Swapped.LastRecoveryFact.ReturnedCards.Reset();
	FMatchPlayRecoveredCardFactEntry SwappedEntry;
	SwappedEntry.OwnerSide = EInitialTurnOrderPlayer::PlayerA;
	SwappedEntry.CardId = PlayerBCard;
	Swapped.LastRecoveryFact.ReturnedCards.Add(SwappedEntry);
	const FFMCodexLocalMatchInteractionView SwappedView =
		FFMCodexLocalMatchInteractionViewBuilder::Build(Swapped, Rules);
	TestEqual(TEXT("Swapped side assignment still reconstructs one entry"),
		SwappedView.RecoveryPresentationEntries.Num(), 1);
	if (SwappedView.RecoveryPresentationEntries.Num() == 1)
	{
		const FString ExpectedSwapped = FString::Printf(
			TEXT("玩家A · %s"),
			*FFMCodexPrototypeTeamContent::PlayerDisplayName(PlayerBCard).ToString());
		TestEqual(TEXT("Owner follows fact side while name follows swapped roster"),
			SwappedView.RecoveryPresentationEntries[0].PresentationLine,
			ExpectedSwapped);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexRecoveryPresentationSurfaceTest,
	"FMCodex.LocalPlay.RecoveryPresentation.02.NonBlockingCentralFeedbackAndPool0",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexRecoveryPresentationSurfaceTest::RunTest(
	const FString& Parameters)
{
	FFMCodexLocalMatchInteractionView View;
	View.bMatchActive = true;
	View.MajorPhase = EFMCodexLocalMatchMajorPhase::BetweenAttacks;
	View.InteractionCategory =
		EFMCodexLocalMatchInteractionCategory::TacticalPointRoll;
	View.ExpectedActingPlayer = EInitialTurnOrderPlayer::PlayerA;
	View.bHasRecoveryFact = true;
	View.RecoverySourceAttackSequence = 12;
	FFMCodexLocalMatchRecoveryPresentationEntry First;
	First.PresentationLine = TEXT("玩家A · 球员甲");
	FFMCodexLocalMatchRecoveryPresentationEntry Second;
	Second.PresentationLine = TEXT("玩家B · 球员乙");
	View.RecoveryPresentationEntries = { First, Second };

	const FFMCodexLocalMatchResolutionFeedback Feedback =
		FFMCodexLocalMatchResolutionFeedbackBuilder::BuildRecovery(View);
	TestTrue(TEXT("Recovery feedback is visible"), Feedback.bVisible);
	TestFalse(TEXT("Recovery feedback is non-terminal"), Feedback.bTerminal);
	TestTrue(TEXT("Recovery requests explicit non-blocking notification mode"),
		Feedback.bNonBlockingNotification);
	TestEqual(TEXT("Recovery owns one title"), Feedback.StepTitle,
		FString(TEXT("球员返回手牌")));
	TestEqual(TEXT("Recovery lines preserve order"), Feedback.StepSummary,
		FString(TEXT("玩家A · 球员甲\n玩家B · 球员乙")));

	const FFMCodexUMGMatchScreenViewModel Presentation =
		FFMCodexLocalMatchUMGPresentationBuilder::Build(
			View,
			Feedback,
			FString(),
			EInitialTurnOrderPlayer::PlayerA);
	TestTrue(TEXT("Existing central Resolution surface presents Recovery"),
		Presentation.Resolution.bVisible);
	TestTrue(TEXT("UMG preserves non-blocking notification mode"),
		Presentation.Resolution.bNonBlockingNotification);
	TestEqual(TEXT("Recovery title has no generic category prefix"),
		Presentation.Resolution.StepLabel,
		FString(TEXT("球员返回手牌")));
	TestFalse(TEXT("Recovery player text contains no STEP label"),
		(Presentation.Resolution.StepLabel
			+ Presentation.Resolution.StepSummaryLabel).Contains(TEXT("STEP")));
	TestFalse(TEXT("Recovery adds no confirmation action"),
		Presentation.Resolution.bCanContinue);
	TestEqual(TEXT("Next attack input remains available"),
		Presentation.Interaction.Category,
		EFMCodexUMGInteractionCategory::TacticalPointRoll);

	View.RecoveryPresentationEntries.Reset();
	const auto Pool0 =
		FFMCodexLocalMatchResolutionFeedbackBuilder::BuildRecovery(View);
	TestFalse(TEXT("Pool0 emits no noisy popup"), Pool0.bVisible);
	return true;
}

#endif
