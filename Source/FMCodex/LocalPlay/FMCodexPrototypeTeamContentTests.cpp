#include "FMCodexPrototypeTeamContent.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "FMCodexDeploymentDragDropOperation.h"
#include "FMCodexInteractionPanelWidget.h"
#include "FMCodexLocalMatchDemoConfiguration.h"
#include "FMCodexLocalMatchHostGameMode.h"
#include "FMCodexLocalMatchPlayerController.h"
#include "FMCodexLocalMatchScreenWidget.h"
#include "FMCodexPitchSlotWidget.h"
#include "FMCodexPitchWidget.h"
#include "FMCodexPlayerCardWidget.h"
#include "FMCodexPlayerUIAssetReferences.h"
#include "FMCodexPlayerUIPresentationText.h"

#include "../CoreRules/DeckValidator.h"
#include "../CoreRules/MatchPlayOpeningInitializer.h"

#include "Engine/Engine.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "Misc/AutomationTest.h"
#include "Misc/Paths.h"

namespace FMCodexPrototypeTeamContentTests
{
	const TSet<FName> FrozenSkillIds = {
		TEXT("Demo.Skill.LongShot"),
		TEXT("Demo.Skill.CutInsideShot"),
		TEXT("Demo.Skill.PassControl"),
		TEXT("Demo.Skill.Cross"),
		TEXT("Demo.Skill.ThroughBall")
	};

	bool IsCodeSafeId(const FName Id)
	{
		const FString Value = Id.ToString();
		if (Value.IsEmpty())
		{
			return false;
		}
		for (const TCHAR Character : Value)
		{
			if (!FChar::IsAlnum(Character) && Character != TEXT('.'))
			{
				return false;
			}
		}
		return true;
	}

	bool AttributesInRange(const FPlayerAttributes& A)
	{
		return A.Shooting >= 1 && A.Shooting <= 6
			&& A.Dribbling >= 1 && A.Dribbling <= 6
			&& A.Passing >= 1 && A.Passing <= 6
			&& A.OffBall >= 1 && A.OffBall <= 6
			&& A.Marking >= 1 && A.Marking <= 6
			&& A.Tackling >= 1 && A.Tackling <= 6
			&& A.Speed >= 1 && A.Speed <= 6
			&& A.Strength >= 1 && A.Strength <= 6
			&& A.Stamina >= 1 && A.Stamina <= 6
			&& A.LongShot >= 1 && A.LongShot <= 6;
	}

	bool GoalkeeperAttributesInRange(const FGoalkeeperAttributes& A)
	{
		return A.Handling >= 1 && A.Handling <= 6
			&& A.Positioning >= 1 && A.Positioning <= 6
			&& A.Reflex >= 1 && A.Reflex <= 6
			&& A.Aerial >= 1 && A.Aerial <= 6
			&& A.Anticipation >= 1 && A.Anticipation <= 6
			&& A.OneOnOne >= 1 && A.OneOnOne <= 6;
	}

	class FScopedPlayableWorld final
	{
	public:
		FScopedPlayableWorld()
		{
			World = UWorld::CreateWorld(EWorldType::Game, false);
			if (World == nullptr || GEngine == nullptr)
			{
				return;
			}
			FWorldContext& Context =
				GEngine->CreateNewWorldContext(EWorldType::Game);
			Context.SetCurrentWorld(World);
			Host = World->SpawnActor<AFMCodexLocalMatchHostGameMode>();
			Controller = World->SpawnActor<AFMCodexLocalMatchPlayerController>();
			if (Controller != nullptr)
			{
				Controller->RefreshPresentation();
			}
		}

		~FScopedPlayableWorld()
		{
			if (World != nullptr)
			{
				if (GEngine != nullptr)
				{
					GEngine->DestroyWorldContext(World);
				}
				World->DestroyWorld(false);
			}
		}

		AFMCodexLocalMatchHostGameMode* Host = nullptr;
		AFMCodexLocalMatchPlayerController* Controller = nullptr;

	private:
		UWorld* World = nullptr;
	};

	const FFMCodexUMGDeploymentChoiceViewModel* FindPrototypeChoice(
		const FFMCodexUMGInteractionViewModel& Interaction)
	{
		return Interaction.DeploymentChoices.FindByPredicate(
			[](const FFMCodexUMGDeploymentChoiceViewModel& Choice)
			{
				return !Choice.bGoalkeeper
					&& !Choice.Destinations.IsEmpty()
					&& FFMCodexPrototypeTeamContent::IsPrototypeCard(
						Choice.CardId);
			});
	}

	UFMCodexPlayerCardWidget* FindHandCard(
		UFMCodexInteractionPanelWidget& Panel,
		const FName CardId)
	{
		for (UFMCodexPlayerCardWidget* Widget
			: Panel.GetRenderedCandidateCardWidgets())
		{
			if (Widget != nullptr
				&& Widget->GetDeploymentDragCardId() == CardId)
			{
				return Widget;
			}
		}
		return nullptr;
	}

	UFMCodexPitchSlotWidget* FindPitchSlot(
		UFMCodexPitchWidget& Pitch,
		const FName SlotId)
	{
		for (UFMCodexPitchSlotWidget* Widget : Pitch.GetRenderedSlotWidgets())
		{
			if (Widget != nullptr
				&& Widget->GetPresentation().SlotId == SlotId)
			{
				return Widget;
			}
		}
		return nullptr;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexPrototypeTeamCatalogTest,
	"FMCodex.LocalPlay.PrototypeTeams.01.ContentCatalog",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexPrototypeTeamCatalogTest::RunTest(const FString& Parameters)
{
	using namespace FMCodexPrototypeTeamContentTests;
	const TArray<FFMCodexPrototypePlayerDefinition>& Definitions =
		FFMCodexPrototypeTeamContent::GetDefinitions();
	TestEqual(TEXT("Pilot stops at exactly ten player definitions"),
		Definitions.Num(), 10);

	TMap<FName, int32> TeamCounts;
	TMap<FName, int32> GoalkeeperCounts;
	TSet<FName> UniqueIds;
	for (const FFMCodexPrototypePlayerDefinition& Definition : Definitions)
	{
		const FPlayerCardData& Card = Definition.Card;
		TeamCounts.FindOrAdd(Definition.TeamId)++;
		GoalkeeperCounts.FindOrAdd(Definition.TeamId) += Card.bIsGoalkeeper ? 1 : 0;
		UniqueIds.Add(Card.CardId);
		TestTrue(TEXT("Technical CardId is non-empty and code-safe"),
			IsCodeSafeId(Card.CardId));
		TestFalse(TEXT("Localized visible player name is live and non-empty"),
			Card.DisplayName.IsEmpty());
		TestFalse(TEXT("Localized visible team name is live and non-empty"),
			Definition.TeamDisplayName.IsEmpty());
		TestTrue(TEXT("All canonical outfield attributes use the existing range"),
			AttributesInRange(Card.Attributes));
		TestTrue(TEXT("Prototype card uses at least one existing role"),
			!Card.PositionTypes.IsEmpty());
		if (Card.bIsGoalkeeper)
		{
			TestTrue(TEXT("Goalkeeper uses only the existing GK position"),
				Card.PositionTypes.Num() == 1
					&& Card.PositionTypes[0]
						== EPlayerPositionType::Goalkeeper
					&& GoalkeeperAttributesInRange(
						Card.GoalkeeperAttributes)
					&& Card.AttackSkillIds.IsEmpty());
		}
		else
		{
			TestTrue(TEXT("Outfield pilot uses exactly one frozen existing skill"),
				Card.AttackSkillIds.Num() == 1
					&& FrozenSkillIds.Contains(Card.AttackSkillIds[0]));
		}
	}
	TestEqual(TEXT("All ten technical CardIds are unique"), UniqueIds.Num(), 10);
	TestEqual(TEXT("Arsenal pilot count is exactly five"),
		TeamCounts.FindRef(FFMCodexPrototypeTeamContent::ArsenalTeamId()), 5);
	TestEqual(TEXT("Manchester City pilot count is exactly five"),
		TeamCounts.FindRef(
			FFMCodexPrototypeTeamContent::ManchesterCityTeamId()), 5);
	TestEqual(TEXT("Arsenal pilot has exactly one goalkeeper"),
		GoalkeeperCounts.FindRef(
			FFMCodexPrototypeTeamContent::ArsenalTeamId()), 1);
	TestEqual(TEXT("Manchester City pilot has exactly one goalkeeper"),
		GoalkeeperCounts.FindRef(
			FFMCodexPrototypeTeamContent::ManchesterCityTeamId()), 1);
	TestEqual(TEXT("Chinese-first identity lookup is centralized"),
		FFMCodexPrototypeTeamContent::PlayerDisplayName(
			TEXT("Prototype.Arsenal.BukayoSaka")).ToString(),
		FString(TEXT("布卡约·萨卡")));
	TestEqual(TEXT("Chinese-first team lookup is centralized"),
		FFMCodexPrototypeTeamContent::TeamDisplayName(
			TEXT("Prototype.ManchesterCity.Rodri")).ToString(),
		FString(TEXT("曼彻斯特城")));

	const FFMCodexLocalMatchDemoConfiguration Demo =
		FFMCodexLocalMatchDemoConfigurationFactory::Create();
	TSet<FName> DemoCardIds;
	int32 PrototypeTotal = 0;
	int32 PlayerAPrototypeCount = 0;
	int32 PlayerBPrototypeCount = 0;
	int32 PlayerARarityScore = 0;
	int32 PlayerBRarityScore = 0;
	for (int32 SideIndex = 0; SideIndex < 2; ++SideIndex)
	{
		const TArray<FPlayerCardData>& Deck = SideIndex == 0
			? Demo.OpeningInput.OpeningInput.PlayerADeck
			: Demo.OpeningInput.OpeningInput.PlayerBDeck;
		const FDeckValidationResult Validation = FDeckValidator::ValidateDeck(Deck);
		TestEqual(TEXT("Integrated LocalPlay deck remains exactly twenty cards"),
			Deck.Num(), 20);
		TestTrue(TEXT("Integrated LocalPlay deck remains canonically valid"),
			Validation.bIsValid);
		const FName ExpectedFirstCardId(*FString::Printf(
			TEXT("Demo.%s.Outfield.01"), SideIndex == 0 ? TEXT("A") : TEXT("B")));
		TestTrue(TEXT("Pilot integration preserves the established Cross public-flow fixture"),
			Deck[0].CardId == ExpectedFirstCardId
				&& Deck[0].AttackSkillIds.Num() == 1
				&& Deck[0].AttackSkillIds[0] == TEXT("Demo.Skill.Cross")
				&& Deck[0].PositionTypes.Contains(EPlayerPositionType::Attack)
				&& Deck[1].PositionTypes.Contains(EPlayerPositionType::Attack));
		TestTrue(TEXT("The fifth generic fixture preserves the five-family demo mix"),
			Deck[4].AttackSkillIds.Num() == 1
				&& Deck[4].AttackSkillIds[0]
					== TEXT("Demo.Skill.ThroughBall"));
		if (SideIndex == 0)
		{
			PlayerARarityScore = Validation.InitialDeckRarityScore;
		}
		else
		{
			PlayerBRarityScore = Validation.InitialDeckRarityScore;
		}
		for (const FPlayerCardData& Card : Deck)
		{
			DemoCardIds.Add(Card.CardId);
			if (FFMCodexPrototypeTeamContent::IsPrototypeCard(Card.CardId))
			{
				++PrototypeTotal;
				if (SideIndex == 0)
				{
					++PlayerAPrototypeCount;
				}
				else
				{
					++PlayerBPrototypeCount;
				}
			}
		}
	}
	TestTrue(TEXT("Demo integrates exactly five plus five pilot cards"),
		PrototypeTotal == 10
			&& PlayerAPrototypeCount == 5
			&& PlayerBPrototypeCount == 5
			&& DemoCardIds.Num() == 40);
	TestEqual(TEXT("Pilot rarity remains mirrored and preserves tie-break fixture"),
		PlayerARarityScore, PlayerBRarityScore);
	TestTrue(TEXT("Opening accepts pilot data without schema or rule changes"),
		FMatchPlayOpeningInitializer::InitializeMatchPlayOpening(
			Demo.OpeningInput).bSuccess);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexPrototypeTeamAssetPipelineTest,
	"FMCodex.LocalPlay.PrototypeTeams.02.AssetPipeline",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexPrototypeTeamAssetPipelineTest::RunTest(const FString& Parameters)
{
	using namespace FMCodexPrototypeTeamContentTests;
	const FFMCodexPlayerUIAssetReferences& References =
		FFMCodexPlayerUIAssetReferences::Get();
	for (const FFMCodexPrototypePlayerDefinition& Definition
		: FFMCodexPrototypeTeamContent::GetDefinitions())
	{
		const FFMCodexPlayerUICardArtReferences Art =
			References.ResolveCardArt(Definition.Card.CardId);
		UTexture2D* Frame = Art.CardFrame.LoadSynchronous();
		UTexture2D* Portrait = Art.Portrait.LoadSynchronous();
		TestTrue(TEXT("Prototype portrait lookup is centralized and complete"),
			!Art.ArtIdentity.IsNone()
				&& !Art.CardFrame.IsNull()
				&& !Art.Portrait.IsNull()
				&& !Art.LongShotSkillIcon.IsNull());
		TestNotNull(TEXT("Shared Golden Sample frame loads"), Frame);
		TestNotNull(TEXT("Prototype Texture2D portrait loads"), Portrait);
		if (Portrait != nullptr)
		{
			TestTrue(TEXT("Prototype portrait has validated card dimensions"),
				Portrait->GetImportedSize() == FIntPoint(1024, 1536));
		}

		const FString TeamFolder = Definition.TeamId
			== FFMCodexPrototypeTeamContent::ArsenalTeamId()
				? TEXT("Arsenal") : TEXT("ManchesterCity");
		const FString SourceFolder = FPaths::Combine(
			FPaths::ProjectDir(), TEXT("ArtSource/UI/PrototypeTeams"),
			TeamFolder, TEXT("Portraits"));
		TestTrue(TEXT("Generated source PNG is preserved beside its prompt set"),
			IFileManager::Get().DirectoryExists(*SourceFolder));
		TestFalse(TEXT("Live localized identity does not come from image text"),
			FFMCodexPlayerUIPresentationText::PlayerName(
				Definition.Card.CardId, FString()).IsEmpty());
		TestFalse(TEXT("Live localized team label remains available"),
			FFMCodexPlayerUIPresentationText::TeamName(
				Definition.Card.CardId).IsEmpty());
	}

	const FFMCodexPlayerUICardArtReferences Missing =
		References.ResolveCardArt(TEXT("Prototype.Missing.Card"));
	TestTrue(TEXT("Unknown CardId keeps the established safe fallback"),
		Missing.ArtIdentity.IsNone()
			&& Missing.CardFrame.IsNull()
			&& Missing.Portrait.IsNull());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexPrototypeTeamPublicFlowTest,
	"FMCodex.LocalPlay.PrototypeTeams.03.PublicFlow",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexPrototypeTeamPublicFlowTest::RunTest(const FString& Parameters)
{
	using namespace FMCodexPrototypeTeamContentTests;
	FScopedPlayableWorld PlayableWorld;
	AFMCodexLocalMatchHostGameMode* Host = PlayableWorld.Host;
	AFMCodexLocalMatchPlayerController* Controller = PlayableWorld.Controller;
	TestNotNull(TEXT("Prototype flow Host exists"), Host);
	TestNotNull(TEXT("Prototype flow Controller exists"), Controller);
	if (Host == nullptr || Controller == nullptr)
	{
		return false;
	}

	Controller->InitializePlayerFacingUI();
	UFMCodexLocalMatchScreenWidget* Screen =
		Controller->GetPlayerMatchScreen();
	TestNotNull(TEXT("Prototype flow player-facing Screen exists"), Screen);
	if (Screen == nullptr)
	{
		return false;
	}
	Screen->TakeWidget();
	Screen->RequestStartNewMatch();
	if (Controller->IsAwaitingHotSeatHandoff())
	{
		Screen->RequestReady();
	}
	Screen->RequestBeginOrdinaryAttack();
	if (Controller->IsAwaitingHotSeatHandoff())
	{
		Screen->RequestReady();
	}

	UFMCodexInteractionPanelWidget* Panel = Screen->GetInteractionPanel();
	UFMCodexPitchWidget* Pitch = Screen->GetPitchWidget();
	TestNotNull(TEXT("Prototype deployment hand exists"), Panel);
	TestNotNull(TEXT("Prototype five-slot pitch exists"), Pitch);
	if (Panel == nullptr || Pitch == nullptr)
	{
		return false;
	}

	TSet<FName> EncounteredTeams;
	for (int32 DeploymentIndex = 0; DeploymentIndex < 2; ++DeploymentIndex)
	{
		const FFMCodexUMGDeploymentChoiceViewModel* ChoiceSource =
			FindPrototypeChoice(Screen->GetPresentation().Interaction);
		TestNotNull(TEXT("Authoritative hand exposes a prototype card"),
			ChoiceSource);
		if (ChoiceSource == nullptr)
		{
			return false;
		}
		const FFMCodexUMGDeploymentChoiceViewModel Choice = *ChoiceSource;
		EncounteredTeams.Add(
			FFMCodexPrototypeTeamContent::TeamIdForCard(Choice.CardId));

		UFMCodexPlayerCardWidget* CardWidget =
			FindHandCard(*Panel, Choice.CardId);
		TestNotNull(TEXT("Prototype card is rendered as the real drag source"),
			CardWidget);
		if (CardWidget == nullptr)
		{
			return false;
		}
		CardWidget->TakeWidget();
		TestTrue(TEXT("PlayerCard renders localized identity, team, skill and stats"),
			!CardWidget->GetRenderedIdentityText().IsEmpty()
				&& !CardWidget->GetRenderedTeamText().IsEmpty()
				&& CardWidget->GetRenderedSkillCount() == 1
				&& CardWidget->GetRenderedAttributeCount() > 0
				&& CardWidget->GetResolvedPortraitTexture() != nullptr
				&& CardWidget->GetResolvedCardFrameTexture() != nullptr);

		UFMCodexDeploymentDragDropOperation* Operation =
			CardWidget->BeginDeploymentDrag();
		TestNotNull(TEXT("Prototype card creates typed drag operation"), Operation);
		if (Operation == nullptr)
		{
			return false;
		}
		UFMCodexPitchSlotWidget* Target = FindPitchSlot(
			*Pitch, Choice.Destinations[0].SlotId);
		TestNotNull(TEXT("Prototype destination is a rendered five-slot target"),
			Target);
		if (Target == nullptr)
		{
			return false;
		}
		const int32 BeforePlacements = Host->GetMatchSnapshot().Snapshot
			.CurrentAttack.DeploymentPlacements.Num();
		TestTrue(TEXT("Prototype drag emits existing typed authoritative command"),
			Target->TryHandleDeploymentDrop(Operation)
				&& Controller->GetLastDiagnostic().CommandName
					== TEXT("DeployOrdinary")
				&& Controller->GetLastDiagnostic().bHostSuccess
				&& Host->GetMatchSnapshot().Snapshot.CurrentAttack
					.DeploymentPlacements.Num() == BeforePlacements + 1);
		Operation->Drop(FPointerEvent());
		if (Controller->IsAwaitingHotSeatHandoff())
		{
			Screen->RequestReady();
		}
	}

	TestTrue(TEXT("Public flow renders and deploys both prototype teams"),
		EncounteredTeams.Num() == 2
			&& EncounteredTeams.Contains(
				FFMCodexPrototypeTeamContent::ArsenalTeamId())
			&& EncounteredTeams.Contains(
				FFMCodexPrototypeTeamContent::ManchesterCityTeamId()));
	TestTrue(TEXT("Existing board remains exactly five slots per physical half"),
		Pitch->GetPresentation().Num() == 2
			&& Pitch->GetPresentation()[0].Slots.Num() == 5
			&& Pitch->GetPresentation()[1].Slots.Num() == 5
			&& Pitch->GetRenderedSlotWidgets().Num() == 10);
	return true;
}

#endif
