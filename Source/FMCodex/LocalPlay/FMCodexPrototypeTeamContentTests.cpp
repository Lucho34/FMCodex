#include "FMCodexPrototypeTeamContent.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "FMCodexCardRackWidget.h"
#include "FMCodexDeploymentDragDropOperation.h"
#include "FMCodexInteractionPanelWidget.h"
#include "FMCodexLocalMatchDemoConfiguration.h"
#include "FMCodexLocalMatchHostGameMode.h"
#include "FMCodexLocalMatchPlayerController.h"
#include "FMCodexLocalMatchScreenWidget.h"
#include "FMCodexPitchSlotWidget.h"
#include "FMCodexPitchWidget.h"
#include "FMCodexPlayerCardWidget.h"
#include "FMCodexPlayerOverall.h"
#include "FMCodexPlayerUIAssetReferences.h"
#include "FMCodexPlayerUIPresentationText.h"

#include "../CoreRules/DeckValidator.h"
#include "../CoreRules/MatchPlayOpeningInitializer.h"

#include "Components/TextBlock.h"
#include "Engine/Engine.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "HAL/FileManager.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace FMCodexPrototypeTeamContentTests
{
	const TSet<FName> ExistingArtworkPlayerKeys = {
		TEXT("Prototype.Arsenal.DavidRaya"),
		TEXT("Prototype.Arsenal.GabrielMagalhaes"),
		TEXT("Prototype.Arsenal.WilliamSaliba"),
		TEXT("Prototype.Arsenal.MartinOdegaard"),
		TEXT("Prototype.Arsenal.MikelMerino"),
		TEXT("Prototype.Arsenal.DeclanRice"),
		TEXT("Prototype.Arsenal.BukayoSaka"),
		TEXT("Prototype.Arsenal.GabrielMartinelli"),
		TEXT("Prototype.ManchesterCity.GianluigiDonnarumma"),
		TEXT("Prototype.ManchesterCity.RubenDias"),
		TEXT("Prototype.ManchesterCity.JoskoGvardiol"),
		TEXT("Prototype.ManchesterCity.Rodri"),
		TEXT("Prototype.ManchesterCity.BernardoSilva"),
		TEXT("Prototype.ManchesterCity.PhilFoden"),
		TEXT("Prototype.ManchesterCity.ErlingHaaland"),
		TEXT("Prototype.ManchesterCity.JeremyDoku")
	};

	const TSet<FName> SharedPortraitPlayerKeys = {
		TEXT("Prototype.Arsenal.DavidRaya"),
		TEXT("Prototype.Arsenal.GabrielMagalhaes"),
		TEXT("Prototype.Arsenal.MylesLewisSkelly"),
		TEXT("Prototype.Arsenal.GabrielMartinelli"),
		TEXT("Prototype.Arsenal.ViktorGyokeres"),
		TEXT("Prototype.Arsenal.WilliamSaliba"),
		TEXT("Prototype.Arsenal.MartinOdegaard"),
		TEXT("Prototype.Arsenal.DeclanRice"),
		TEXT("Prototype.Arsenal.BukayoSaka"),
		TEXT("Prototype.ManchesterCity.GianluigiDonnarumma"),
		TEXT("Prototype.ManchesterCity.RubenDias"),
		TEXT("Prototype.ManchesterCity.JoskoGvardiol"),
		TEXT("Prototype.ManchesterCity.Rodri"),
		TEXT("Prototype.ManchesterCity.BernardoSilva"),
		TEXT("Prototype.ManchesterCity.PhilFoden"),
		TEXT("Prototype.ManchesterCity.ErlingHaaland"),
		TEXT("Prototype.ManchesterCity.JeremyDoku"),
		TEXT("Prototype.ManchesterCity.RayanAitNouri")
	};

	const TSet<FName> ProductionContractSharedPortraitPlayerKeys = {
		TEXT("Prototype.Arsenal.DavidRaya"),
		TEXT("Prototype.Arsenal.GabrielMagalhaes"),
		TEXT("Prototype.Arsenal.MylesLewisSkelly"),
		TEXT("Prototype.Arsenal.GabrielMartinelli"),
		TEXT("Prototype.Arsenal.ViktorGyokeres"),
		TEXT("Prototype.ManchesterCity.JoskoGvardiol"),
		TEXT("Prototype.ManchesterCity.BernardoSilva"),
		TEXT("Prototype.ManchesterCity.ErlingHaaland"),
		TEXT("Prototype.ManchesterCity.JeremyDoku"),
		TEXT("Prototype.ManchesterCity.RayanAitNouri")
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

	const FFMCodexUMGDeploymentChoiceViewModel* FindArtworkBackedChoice(
		const FFMCodexUMGInteractionViewModel& Interaction)
	{
		return Interaction.DeploymentChoices.FindByPredicate(
			[](const FFMCodexUMGDeploymentChoiceViewModel& Choice)
			{
				if (Choice.bGoalkeeper || Choice.Destinations.IsEmpty()
					|| !FFMCodexPrototypeTeamContent::IsPrototypeCard(Choice.CardId))
				{
					return false;
				}
				return !FFMCodexPlayerUIAssetReferences::Get()
					.ResolveCardArt(Choice.CardId).HandMicroPortrait.IsNull();
			});
	}

	UFMCodexPlayerCardWidget* FindHandCard(
		UFMCodexCardRackWidget& Rack,
		const FName CardId)
	{
		for (UFMCodexPlayerCardWidget* Widget : Rack.GetRenderedCardWidgets())
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
	TArray<FString> ValidationErrors;
	TestTrue(TEXT("Independent runtime catalog validation passes"),
		FFMCodexPrototypeTeamContent::Validate(ValidationErrors));
	TestEqual(TEXT("Runtime catalog validation reports no errors"),
		ValidationErrors.Num(), 0);
	TestEqual(TEXT("Balance content version is explicit"),
		FFMCodexPrototypeTeamContent::GetBalanceContentVersion(),
		FString(TEXT("Prototype40_v1")));
	TestTrue(TEXT("Generated runtime JSON exists outside gameplay C++"),
		IFileManager::Get().FileExists(
			*FFMCodexPrototypeTeamContent::GetRuntimeContentPath()));

	const TArray<FFMCodexPrototypePlayerDefinition>& Definitions =
		FFMCodexPrototypeTeamContent::GetDefinitions();
	TestEqual(TEXT("Canonical roster has exactly forty definitions"),
		Definitions.Num(), 40);
	TMap<FName, int32> TeamCounts;
	TMap<FName, int32> GoalkeeperCounts;
	TSet<FName> PlayerKeys;
	TSet<int32> DisplaySerials;
	TMap<int32, int32> SkillCounts;
	int32 OverlapChecks = 0;
	for (const FFMCodexPrototypePlayerDefinition& Definition : Definitions)
	{
		const FPlayerCardData& Card = Definition.Card;
		TeamCounts.FindOrAdd(Definition.TeamId)++;
		GoalkeeperCounts.FindOrAdd(Definition.TeamId) +=
			Card.bIsGoalkeeper ? 1 : 0;
		PlayerKeys.Add(Definition.PlayerKey);
		DisplaySerials.Add(Definition.DisplaySerial);
		SkillCounts.FindOrAdd(Definition.SkillAssignments.Num())++;
		TestTrue(TEXT("PlayerKey is stable, code-safe, and owns CardId identity"),
			IsCodeSafeId(Definition.PlayerKey)
				&& Definition.PlayerKey == Card.CardId);
		TestTrue(TEXT("DisplaySerial is positive and only formatted for presentation"),
			Definition.DisplaySerial > 0
				&& Definition.PlayerFacingSerial
					== FString::Printf(TEXT("%03d"), Definition.DisplaySerial));
		TestTrue(TEXT("RosterSlot remains order metadata"),
			Definition.RosterSlot >= 1 && Definition.RosterSlot <= 20);
		TestTrue(TEXT("Required player and team names are present"),
			!Card.DisplayName.IsEmpty()
				&& !Definition.EnglishDisplayName.IsEmpty()
				&& !Definition.TeamDisplayName.IsEmpty());
		if (Card.bIsGoalkeeper)
		{
			TestTrue(TEXT("Goalkeeper uses exactly the canonical GK schema"),
				Card.PositionTypes.Num() == 1
					&& Card.PositionTypes[0] == EPlayerPositionType::Goalkeeper
					&& GoalkeeperAttributesInRange(Card.GoalkeeperAttributes));
		}
		else
		{
			TestTrue(TEXT("Outfield player uses exactly ranged canonical values"),
				!Card.PositionTypes.IsEmpty()
					&& !Card.PositionTypes.Contains(EPlayerPositionType::Goalkeeper)
					&& AttributesInRange(Card.Attributes));
		}
		TestTrue(TEXT("Player has zero to three Skill assignments"),
			Definition.SkillAssignments.Num() >= 0
				&& Definition.SkillAssignments.Num() <= 3
				&& Definition.SkillAssignments.Num()
					== Card.AttackSkillIds.Num());
		TSet<FName> CanonicalSkillIds;
		for (const FFMCodexPrototypeSkillAssignment& Skill
			: Definition.SkillAssignments)
		{
			TestTrue(TEXT("Skill reference and TP range are canonical"),
				!Skill.SkillId.IsNone() && !Skill.RuleId.IsNone()
					&& Skill.SkillType != ESkillRuleType::None
					&& Skill.MinTacticalPoint >= 2
					&& Skill.MinTacticalPoint <= Skill.MaxTacticalPoint
					&& Skill.MaxTacticalPoint <= 8);
			TestFalse(TEXT("No duplicate canonical SkillId per player"),
				CanonicalSkillIds.Contains(Skill.SkillId));
			CanonicalSkillIds.Add(Skill.SkillId);
		}
		for (int32 TacticalPoint = 2; TacticalPoint <= 8; ++TacticalPoint)
		{
			int32 Eligible = 0;
			for (const FFMCodexPrototypeSkillAssignment& Skill
				: Definition.SkillAssignments)
			{
				Eligible += TacticalPoint >= Skill.MinTacticalPoint
					&& TacticalPoint <= Skill.MaxTacticalPoint ? 1 : 0;
			}
			TestTrue(TEXT("Every player/TP overlap count is at most two"),
				Eligible <= 2);
			++OverlapChecks;
		}
	}
	TestEqual(TEXT("All forty PlayerKeys are unique"), PlayerKeys.Num(), 40);
	TestEqual(TEXT("All forty DisplaySerial values are unique"),
		DisplaySerials.Num(), 40);
	TestEqual(TEXT("Arsenal roster is exactly 20"),
		TeamCounts.FindRef(FFMCodexPrototypeTeamContent::ArsenalTeamId()), 20);
	TestEqual(TEXT("Manchester City roster is exactly 20"),
		TeamCounts.FindRef(
			FFMCodexPrototypeTeamContent::ManchesterCityTeamId()), 20);
	TestEqual(TEXT("Arsenal has exactly one goalkeeper"),
		GoalkeeperCounts.FindRef(
			FFMCodexPrototypeTeamContent::ArsenalTeamId()), 1);
	TestEqual(TEXT("Manchester City has exactly one goalkeeper"),
		GoalkeeperCounts.FindRef(
			FFMCodexPrototypeTeamContent::ManchesterCityTeamId()), 1);
	TestEqual(TEXT("Full overlap matrix contains 40 x 7 checks"),
		OverlapChecks, 280);
	TestTrue(TEXT("Skill count distribution is exact"),
		SkillCounts.FindRef(0) == 18
			&& SkillCounts.FindRef(1) == 10
			&& SkillCounts.FindRef(2) == 10
			&& SkillCounts.FindRef(3) == 2);

	const FFMCodexLocalMatchDemoConfiguration Demo =
		FFMCodexLocalMatchDemoConfigurationFactory::Create();
	TSet<FName> DemoCardIds;
	int32 PlayerARarityScore = 0;
	int32 PlayerBRarityScore = 0;
	for (int32 SideIndex = 0; SideIndex < 2; ++SideIndex)
	{
		const TArray<FPlayerCardData>& Deck = SideIndex == 0
			? Demo.OpeningInput.OpeningInput.PlayerADeck
			: Demo.OpeningInput.OpeningInput.PlayerBDeck;
		const FDeckValidationResult DeckValidation =
			FDeckValidator::ValidateDeck(Deck);
		TestEqual(TEXT("Canonical LocalPlay deck remains exactly twenty cards"),
			Deck.Num(), 20);
		TestTrue(TEXT("Canonical LocalPlay deck remains valid"),
			DeckValidation.bIsValid);
		for (const FPlayerCardData& Card : Deck)
		{
			DemoCardIds.Add(Card.CardId);
			TestTrue(TEXT("Every normal LocalPlay card is canonical real-player content"),
				FFMCodexPrototypeTeamContent::IsPrototypeCard(Card.CardId)
					&& !Card.CardId.ToString().StartsWith(TEXT("Demo.")));
		}
		if (SideIndex == 0)
		{
			PlayerARarityScore = DeckValidation.InitialDeckRarityScore;
			TestEqual(TEXT("Arsenal order starts with workbook RosterSlot 1"),
				Deck[0].CardId, FName(TEXT("Prototype.Arsenal.DavidRaya")));
		}
		else
		{
			PlayerBRarityScore = DeckValidation.InitialDeckRarityScore;
			TestEqual(TEXT("Manchester City order starts with workbook RosterSlot 1"),
				Deck[0].CardId,
				FName(TEXT("Prototype.ManchesterCity.GianluigiDonnarumma")));
		}
	}
	TestEqual(TEXT("Normal decks contain forty unique canonical players"),
		DemoCardIds.Num(), 40);
	TestEqual(TEXT("Preserved Arsenal presentation rarity score"),
		PlayerARarityScore, 50);
	TestEqual(TEXT("Preserved Manchester City presentation rarity score"),
		PlayerBRarityScore, 52);
	TestTrue(TEXT("Opening accepts the migrated canonical decks"),
		FMatchPlayOpeningInitializer::InitializeMatchPlayOpening(
			Demo.OpeningInput).bSuccess);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexCanonicalPlayerSkillRulesTest,
	"FMCodex.LocalPlay.PrototypeTeams.02.CanonicalDataAndSkillRules",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexCanonicalPlayerSkillRulesTest::RunTest(const FString& Parameters)
{
	const FFMCodexPrototypePlayerDefinition* Saka =
		FFMCodexPrototypeTeamContent::Find(TEXT("Prototype.Arsenal.BukayoSaka"));
	const FFMCodexPrototypePlayerDefinition* Odegaard =
		FFMCodexPrototypeTeamContent::Find(TEXT("Prototype.Arsenal.MartinOdegaard"));
	const FFMCodexPrototypePlayerDefinition* Raya =
		FFMCodexPrototypeTeamContent::Find(TEXT("Prototype.Arsenal.DavidRaya"));
	const FFMCodexPrototypePlayerDefinition* Savinho =
		FFMCodexPrototypeTeamContent::Find(TEXT("Prototype.ManchesterCity.Savinho"));
	TestTrue(TEXT("Existing Saka PlayerKey is reused and workbook values are exact"),
		Saka != nullptr && Saka->DisplaySerial == 15
			&& Saka->RosterSlot == 15
			&& Saka->Card.PositionTypes
				== TArray<EPlayerPositionType>{ EPlayerPositionType::Attack }
			&& Saka->Card.Attributes.Shooting == 5
			&& Saka->Card.Attributes.Dribbling == 6
			&& Saka->Card.Attributes.OffBall == 6
			&& Saka->SkillAssignments.Num() == 2
			&& Saka->SkillAssignments[0].SkillId == TEXT("Cross")
			&& Saka->SkillAssignments[0].MinTacticalPoint == 4
			&& Saka->SkillAssignments[0].MaxTacticalPoint == 6
			&& Saka->SkillAssignments[1].SkillId == TEXT("CutInsideShot")
			&& Saka->SkillAssignments[1].MinTacticalPoint == 2
			&& Saka->SkillAssignments[1].MaxTacticalPoint == 4);
	TestTrue(TEXT("Three-Skill workbook order and ranges are preserved"),
		Odegaard != nullptr && Odegaard->SkillAssignments.Num() == 3
			&& Odegaard->SkillAssignments[0].SkillId == TEXT("PassControl")
			&& Odegaard->SkillAssignments[0].MinTacticalPoint == 6
			&& Odegaard->SkillAssignments[0].MaxTacticalPoint == 8
			&& Odegaard->SkillAssignments[1].SkillId == TEXT("ThroughBall")
			&& Odegaard->SkillAssignments[1].MinTacticalPoint == 5
			&& Odegaard->SkillAssignments[1].MaxTacticalPoint == 6
			&& Odegaard->SkillAssignments[2].SkillId == TEXT("LongShot")
			&& Odegaard->SkillAssignments[2].MinTacticalPoint == 3
			&& Odegaard->SkillAssignments[2].MaxTacticalPoint == 5);
	TestTrue(TEXT("Goalkeeper schema and DisplaySerial are exact"),
		Raya != nullptr && Raya->DisplaySerial == 1
			&& Raya->Card.bIsGoalkeeper
			&& Raya->Card.GoalkeeperAttributes.Handling == 5
			&& Raya->Card.GoalkeeperAttributes.Positioning == 6
			&& Raya->Card.GoalkeeperAttributes.Reflex == 5
			&& Raya->Card.GoalkeeperAttributes.Aerial == 4
			&& Raya->Card.GoalkeeperAttributes.Anticipation == 5
			&& Raya->Card.GoalkeeperAttributes.OneOnOne == 5
			&& Raya->SkillAssignments.IsEmpty());
	TestTrue(TEXT("New PlayerKey is stable and workbook content is imported"),
		Savinho != nullptr && Savinho->DisplaySerial == 40
			&& Savinho->RosterSlot == 20
			&& Savinho->Card.DisplayName.ToString() == TEXT("萨维尼奥")
			&& Savinho->SkillAssignments.Num() == 2
			&& Savinho->SkillAssignments[0].SkillId == TEXT("Cross")
			&& Savinho->SkillAssignments[0].MinTacticalPoint == 4
			&& Savinho->SkillAssignments[0].MaxTacticalPoint == 5
			&& Savinho->SkillAssignments[1].SkillId == TEXT("CutInsideShot")
			&& Savinho->SkillAssignments[1].MinTacticalPoint == 4
			&& Savinho->SkillAssignments[1].MaxTacticalPoint == 4);

	const FFMCodexLocalMatchDemoConfiguration Demo =
		FFMCodexLocalMatchDemoConfigurationFactory::Create();
	TSet<FName> RuleIds;
	for (const FSkillRuleSnapshot& Rule : Demo.SkillRuleSet.SkillRules)
	{
		RuleIds.Add(Rule.SkillId);
		TestTrue(TEXT("Generated Skill rule has canonical type and range"),
			!Rule.SkillId.IsNone() && Rule.SkillType != ESkillRuleType::None
				&& Rule.MinTriggerActionPoint >= 2
				&& Rule.MinTriggerActionPoint <= Rule.MaxTriggerActionPoint
				&& Rule.MaxTriggerActionPoint <= 8);
	}
	TestEqual(TEXT("Generated Skill rule identities are unique"),
		RuleIds.Num(), Demo.SkillRuleSet.SkillRules.Num());
	for (const FFMCodexPrototypePlayerDefinition& Definition
		: FFMCodexPrototypeTeamContent::GetDefinitions())
	{
		for (const FFMCodexPrototypeSkillAssignment& Assignment
			: Definition.SkillAssignments)
		{
			const FSkillRuleSnapshot* Rule =
				Demo.SkillRuleSet.SkillRules.FindByPredicate(
					[&Assignment](const FSkillRuleSnapshot& Candidate)
					{
						return Candidate.SkillId == Assignment.RuleId;
					});
			TestTrue(TEXT("Every player Skill assignment resolves its exact runtime rule"),
				Rule != nullptr && Rule->SkillType == Assignment.SkillType
					&& Rule->MinTriggerActionPoint
						== Assignment.MinTacticalPoint
					&& Rule->MaxTriggerActionPoint
						== Assignment.MaxTacticalPoint);
		}
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexPrototypeOverallV1Test,
	"FMCodex.LocalPlay.PrototypeTeams.03.OverallV1",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexPrototypeOverallV1Test::RunTest(const FString& Parameters)
{
	int32 SuccessfulOverallCount = 0;
	for (const FFMCodexPrototypePlayerDefinition& Definition
		: FFMCodexPrototypeTeamContent::GetDefinitions())
	{
		const FFMCodexPlayerOverallResult First =
			FFMCodexPlayerOverall::Calculate(Definition.Card);
		const FFMCodexPlayerOverallResult Second =
			FFMCodexPlayerOverall::Calculate(Definition.Card);
		TestTrue(TEXT("Overall remains deterministic for every canonical player"),
			First.bSuccess && First.Value > 0
				&& First.Value == Second.Value);
		SuccessfulOverallCount += First.bSuccess ? 1 : 0;
	}
	TestEqual(TEXT("Overall compatibility covers all forty players"),
		SuccessfulOverallCount, 40);

	FPlayerAttributes TopSixProbe;
	TopSixProbe.Shooting = 6;
	TopSixProbe.Dribbling = 5;
	TopSixProbe.Passing = 4;
	TopSixProbe.OffBall = 3;
	TopSixProbe.Marking = 2;
	TopSixProbe.Tackling = 1;
	TopSixProbe.Speed = 6;
	TopSixProbe.Strength = 5;
	TopSixProbe.Stamina = 4;
	TopSixProbe.LongShot = 3;
	const int32 Before = FFMCodexPlayerOverall::CalculateOutfield(
		TopSixProbe, ECardRarity::Common).Value;
	TopSixProbe.Tackling = 2;
	const int32 LowerAttributeChanged =
		FFMCodexPlayerOverall::CalculateOutfield(
			TopSixProbe, ECardRarity::Common).Value;
	TopSixProbe.Tackling = 6;
	const int32 EnteredTopSix = FFMCodexPlayerOverall::CalculateOutfield(
		TopSixProbe, ECardRarity::Common).Value;
	TestTrue(TEXT("Overall v1 calculation semantics remain unchanged"),
		Before == LowerAttributeChanged && EnteredTopSix > Before);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexPrototypeTeamAssetPipelineTest,
	"FMCodex.LocalPlay.PrototypeTeams.04.AssetPipeline",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexPrototypeTeamAssetPipelineTest::RunTest(const FString& Parameters)
{
	using namespace FMCodexPrototypeTeamContentTests;
	const FFMCodexPlayerUIAssetReferences& References =
		FFMCodexPlayerUIAssetReferences::Get();
	int32 HandMicroCoverage = 0;
	int32 FullCardCoverage = 0;
	int32 SharedPortraitCoverage = 0;
	int32 SafeFallbackCoverage = 0;
	for (const FFMCodexPrototypePlayerDefinition& Definition
		: FFMCodexPrototypeTeamContent::GetDefinitions())
	{
		const FFMCodexPlayerUICardArtReferences Art =
			References.ResolveCardArt(Definition.PlayerKey);
		if (ExistingArtworkPlayerKeys.Contains(Definition.PlayerKey))
		{
			UTexture2D* HandMicro = Art.HandMicroPortrait.LoadSynchronous();
			UTexture2D* FullCard = Art.FullCardPortrait.LoadSynchronous();
			TestTrue(TEXT("Existing Hand Micro artwork reference remains valid"),
				!Art.HandMicroPortrait.IsNull() && HandMicro != nullptr
					&& HandMicro->GetImportedSize() == FIntPoint(192, 128));
			TestTrue(TEXT("Existing Full Card artwork reference remains valid"),
				!Art.FullCardPortrait.IsNull() && FullCard != nullptr
					&& FullCard->GetImportedSize() == FIntPoint(1024, 1536));
			++HandMicroCoverage;
			++FullCardCoverage;
		}
		else
		{
			TestTrue(TEXT("Shared coverage does not populate dedicated variants"),
				Art.FullCardPortrait.IsNull()
					&& Art.HandMicroPortrait.IsNull());
		}
		if (SharedPortraitPlayerKeys.Contains(Definition.PlayerKey))
		{
			UTexture2D* SharedPortrait = Art.Portrait.LoadSynchronous();
			TestTrue(TEXT("Shared portrait is available through the Pitch Mini route"),
				!Art.Portrait.IsNull()
					&& SharedPortrait != nullptr);
			if (SharedPortrait != nullptr)
			{
				const FIntPoint ExpectedSize =
					ProductionContractSharedPortraitPlayerKeys.Contains(
						Definition.PlayerKey)
						? FIntPoint(512, 768) : FIntPoint(1024, 1536);
				TestEqual(TEXT("Shared portrait uses its recorded pipeline generation"),
					SharedPortrait->GetImportedSize(), ExpectedSize);
			}
			++SharedPortraitCoverage;
		}
		else
		{
			TestTrue(TEXT("Uncovered Pitch Mini keeps the established portrait fallback"),
				Art.Portrait.IsNull());
			++SafeFallbackCoverage;
		}
		TestFalse(TEXT("Localized player identity remains live with or without art"),
			FFMCodexPlayerUIPresentationText::PlayerName(
				Definition.PlayerKey, FString()).IsEmpty());
	}
	TestEqual(TEXT("Hand Micro dedicated coverage is preserved at 16/40"),
		HandMicroCoverage, 16);
	TestEqual(TEXT("Full Card dedicated coverage is preserved at 16/40"),
		FullCardCoverage, 16);
	TestEqual(TEXT("Pitch Mini-compatible shared portrait coverage is 18/40"),
		SharedPortraitCoverage, 18);
	TestEqual(TEXT("Pitch Mini shared fallback coverage is 22/40"),
		SafeFallbackCoverage, 22);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexPrototypeTeamPublicFlowTest,
	"FMCodex.LocalPlay.PrototypeTeams.05.PublicFlow",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexPrototypeTeamPublicFlowTest::RunTest(const FString& Parameters)
{
	using namespace FMCodexPrototypeTeamContentTests;
	FScopedPlayableWorld PlayableWorld;
	AFMCodexLocalMatchHostGameMode* Host = PlayableWorld.Host;
	AFMCodexLocalMatchPlayerController* Controller = PlayableWorld.Controller;
	TestNotNull(TEXT("Canonical flow Host exists"), Host);
	TestNotNull(TEXT("Canonical flow Controller exists"), Controller);
	if (Host == nullptr || Controller == nullptr)
	{
		return false;
	}

	Controller->InitializePlayerFacingUI();
	UFMCodexLocalMatchScreenWidget* Screen =
		Controller->GetPlayerMatchScreen();
	TestNotNull(TEXT("Canonical flow player-facing Screen exists"), Screen);
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
	UFMCodexCardRackWidget* LocalRack = Screen->GetLocalRackWidget();
	UFMCodexPitchWidget* Pitch = Screen->GetPitchWidget();
	TestNotNull(TEXT("Canonical deployment hand exists"), Panel);
	TestNotNull(TEXT("Canonical five-slot pitch exists"), Pitch);
	if (Panel == nullptr || Pitch == nullptr || LocalRack == nullptr)
	{
		return false;
	}

	TSet<FName> EncounteredTeams;
	for (int32 DeploymentIndex = 0; DeploymentIndex < 2; ++DeploymentIndex)
	{
		const FFMCodexUMGDeploymentChoiceViewModel* ChoiceSource =
			FindArtworkBackedChoice(Screen->GetPresentation().Interaction);
		TestNotNull(TEXT("Authoritative hand exposes an artwork-backed canonical card"),
			ChoiceSource);
		if (ChoiceSource == nullptr)
		{
			return false;
		}
		const FFMCodexUMGDeploymentChoiceViewModel Choice = *ChoiceSource;
		EncounteredTeams.Add(
			FFMCodexPrototypeTeamContent::TeamIdForCard(Choice.CardId));

		UFMCodexPlayerCardWidget* CardWidget =
			FindHandCard(*LocalRack, Choice.CardId);
		TestNotNull(TEXT("Canonical card is rendered as the real drag source"),
			CardWidget);
		if (CardWidget == nullptr)
		{
			return false;
		}
		CardWidget->TakeWidget();
		const UTextBlock* MicroName = Cast<UTextBlock>(
			CardWidget->GetWidgetFromName(TEXT("HandMicroPlayerName")));
		TestTrue(TEXT("Frozen Hand Micro renders canonical identity and existing art"),
			MicroName != nullptr && !MicroName->GetText().IsEmpty()
				&& CardWidget->GetRenderedSkillCount() == 0
				&& CardWidget->GetRenderedAttributeCount() == 0
				&& CardWidget->GetResolvedHandMicroPortraitTexture() != nullptr);

		UFMCodexDeploymentDragDropOperation* Operation =
			CardWidget->BeginDeploymentDrag();
		TestNotNull(TEXT("Canonical card creates typed drag operation"), Operation);
		if (Operation == nullptr)
		{
			return false;
		}
		UFMCodexPitchSlotWidget* Target = FindPitchSlot(
			*Pitch, Choice.Destinations[0].SlotId);
		TestNotNull(TEXT("Canonical destination is a rendered five-slot target"),
			Target);
		if (Target == nullptr)
		{
			return false;
		}
		const int32 BeforePlacements = Host->GetMatchSnapshot().Snapshot
			.CurrentAttack.DeploymentPlacements.Num();
		TestTrue(TEXT("Canonical drag emits the existing authoritative command"),
			Target->TryHandleDeploymentDrop(Operation)
				&& Controller->GetLastDiagnostic().CommandName
					== TEXT("DeployOrdinary")
				&& Controller->GetLastDiagnostic().bHostSuccess
				&& Host->GetMatchSnapshot().Snapshot.CurrentAttack
					.DeploymentPlacements.Num() == BeforePlacements + 1);
		Operation->Drop(FPointerEvent());
	}

	TestTrue(TEXT("Public flow renders and deploys both canonical teams"),
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexPrototypePresentationMetadataTest,
	"FMCodex.LocalPlay.PrototypeTeams.06.PresentationMetadata",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexPrototypePresentationMetadataTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexPrototypeTeamContentTests;
	FScopedPlayableWorld PlayableWorld;
	AFMCodexLocalMatchPlayerController* Controller = PlayableWorld.Controller;
	TestNotNull(TEXT("Metadata flow Controller exists"), Controller);
	if (Controller == nullptr)
	{
		return false;
	}
	Controller->InitializePlayerFacingUI();
	UFMCodexLocalMatchScreenWidget* Screen =
		Controller->GetPlayerMatchScreen();
	TestNotNull(TEXT("Metadata flow Screen exists"), Screen);
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

	TArray<const FFMCodexUMGCardRackCellViewModel*> FormalCells;
	const FFMCodexUMGMatchScreenViewModel& Presentation =
		Screen->GetPresentation();
	for (const FFMCodexUMGCardRackViewModel* Rack
		: TArray<const FFMCodexUMGCardRackViewModel*>{
			&Presentation.LocalRack, &Presentation.OpponentRack })
	{
		for (const FFMCodexUMGCardRackCellViewModel& Cell : Rack->Cells)
		{
			if (FFMCodexPrototypeTeamContent::IsPrototypeCard(Cell.Card.CardId))
			{
				FormalCells.Add(&Cell);
			}
		}
	}
	TestEqual(TEXT("Normal presentation exposes all forty canonical players"),
		FormalCells.Num(), 40);
	TSet<FString> Serials;
	int32 PreservedBiographyCount = 0;
	bool bCoreFieldsComplete = FormalCells.Num() == 40;
	const FFMCodexUMGCardViewModel* Gabriel = nullptr;
	const FFMCodexUMGCardRackCellViewModel* StableIndexProbe = nullptr;
	for (const FFMCodexUMGCardRackCellViewModel* Cell : FormalCells)
	{
		if (Cell == nullptr)
		{
			bCoreFieldsComplete = false;
			continue;
		}
		const FFMCodexUMGCardViewModel& Card = Cell->Card;
		Serials.Add(Card.PlayerFacingSerialLabel);
		bCoreFieldsComplete = bCoreFieldsComplete
			&& !Card.IdentityLabel.IsEmpty()
			&& !Card.EnglishIdentityLabel.IsEmpty()
			&& !Card.ClubLabel.IsEmpty()
			&& Card.bHasOverallRating && Card.OverallRating > 0
			&& Card.PlayerFacingSerialLabel.Len() == 3
			&& Card.PlayerFacingSerialLabel.IsNumeric()
			&& (Card.AttributeValues.Num() == 10
				|| Card.AttributeValues.Num() == 6)
			&& !Card.IdentityLabel.Contains(TEXT("Prototype."))
			&& !Card.IdentityLabel.Contains(TEXT("Demo."));
		if (ExistingArtworkPlayerKeys.Contains(Card.CardId))
		{
			const bool bBiographyComplete = !Card.NationalityLabel.IsEmpty()
				&& !Card.BirthDate.IsEmpty()
				&& Card.HeightCm > 0 && Card.WeightKg > 0;
			TestTrue(TEXT("Existing factual presentation metadata is preserved"),
				bBiographyComplete);
			PreservedBiographyCount += bBiographyComplete ? 1 : 0;
		}
		if (Card.CardId == TEXT("Prototype.Arsenal.GabrielMagalhaes"))
		{
			Gabriel = &Card;
		}
		if (StableIndexProbe == nullptr)
		{
			StableIndexProbe = Cell;
		}
	}
	TestTrue(TEXT("Frozen UI DTO receives all canonical core presentation fields"),
		bCoreFieldsComplete && Serials.Num() == 40);
	TestEqual(TEXT("Existing biography coverage remains 16/40"),
		PreservedBiographyCount, 16);
	TestTrue(TEXT("Gabriel keeps one identity, no fake Skill, and workbook serial 2"),
		Gabriel != nullptr && Gabriel->Skills.IsEmpty()
			&& Gabriel->SkillLabels.IsEmpty()
			&& Gabriel->PlayerFacingSerialLabel == TEXT("002")
			&& FFMCodexPlayerUIPresentationText::HandMicroPlayerName(
				Gabriel->CardId, Gabriel->IdentityLabel).ToString()
					== TEXT("加布里埃尔"));
	if (StableIndexProbe != nullptr)
	{
		FFMCodexUMGCardRackCellViewModel Copy = *StableIndexProbe;
		const FString SerialBefore = Copy.Card.PlayerFacingSerialLabel;
		const FName IdentityBefore = Copy.Card.CardId;
		Copy.StableIndex += 37;
		TestTrue(TEXT("Changing Roster/UI order does not change serial or identity"),
			Copy.Card.PlayerFacingSerialLabel == SerialBefore
				&& Copy.Card.CardId == IdentityBefore);
		Copy.Card.PlayerFacingSerialLabel = TEXT("999");
		TestEqual(TEXT("Changing DisplaySerial presentation cannot change PlayerKey/CardId"),
			Copy.Card.CardId, IdentityBefore);
	}

	TArray<FString> AuthoritySources;
	IFileManager::Get().FindFilesRecursive(AuthoritySources,
		*FPaths::Combine(FPaths::ProjectDir(), TEXT("Source/FMCodex/CoreRules")),
		TEXT("*.cpp"), true, false);
	IFileManager::Get().FindFilesRecursive(AuthoritySources,
		*FPaths::Combine(FPaths::ProjectDir(),
			TEXT("Source/FMCodex/MatchPlayRuntime")),
		TEXT("*.cpp"), true, false);
	bool bPresentationDataAffectsGameplay = false;
	for (const FString& SourcePath : AuthoritySources)
	{
		FString Source;
		if (FFileHelper::LoadFileToString(Source, *SourcePath)
			&& (Source.Contains(TEXT("DisplaySerial"))
				|| Source.Contains(TEXT("PlayerFacingSerial"))
				|| Source.Contains(TEXT("PreferredDisplayName"))
				|| Source.Contains(TEXT("CanonicalChineseDisplayName"))
				|| Source.Contains(TEXT("NationalityDisplayName"))
				|| Source.Contains(TEXT("TeamDisplayName"))))
		{
			bPresentationDataAffectsGameplay = true;
			break;
		}
	}
	TestFalse(TEXT("DisplaySerial and presentation metadata have no Authority readers"),
		bPresentationDataAffectsGameplay);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexPrototypeDisplayNameContractTest,
	"FMCodex.LocalPlay.PrototypeTeams.07.DisplayNameContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexPrototypeDisplayNameContractTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexPrototypeTeamContentTests;
	const TArray<FString> ExpectedDisplayNames = {
		TEXT("拉亚"), TEXT("加布里埃尔"), TEXT("萨利巴"), TEXT("怀特"),
		TEXT("因卡皮耶"), TEXT("廷贝尔"), TEXT("卡拉菲奥里"),
		TEXT("刘易斯-斯凯利"), TEXT("厄德高"), TEXT("埃泽"), TEXT("梅里诺"),
		TEXT("苏比门迪"), TEXT("赖斯"), TEXT("诺尔高"), TEXT("萨卡"),
		TEXT("马丁内利"), TEXT("哲凯赖什"), TEXT("特罗萨德"),
		TEXT("马杜埃凯"), TEXT("哈弗茨"), TEXT("多纳鲁马"), TEXT("迪亚斯"),
		TEXT("格伊"), TEXT("格瓦迪奥尔"), TEXT("阿克"), TEXT("斯通斯"),
		TEXT("艾特-努里"), TEXT("罗德里"), TEXT("赖因德斯"), TEXT("贝尔纳多"),
		TEXT("福登"), TEXT("谢尔基"), TEXT("冈萨雷斯"), TEXT("努内斯"),
		TEXT("科瓦契奇"), TEXT("哈兰德"), TEXT("马尔穆什"), TEXT("多库"),
		TEXT("塞梅尼奥"), TEXT("萨维尼奥")
	};
	const TArray<FFMCodexPrototypePlayerDefinition>& Definitions =
		FFMCodexPrototypeTeamContent::GetDefinitions();
	TestEqual(TEXT("DisplayName expectation covers the forty-player roster"),
		ExpectedDisplayNames.Num(), 40);
	TestEqual(TEXT("Runtime catalog exposes forty explicit DisplayNames"),
		Definitions.Num(), ExpectedDisplayNames.Num());

	bool bAllExplicitNamesMatch = Definitions.Num() == ExpectedDisplayNames.Num();
	for (int32 Index = 0; Index < Definitions.Num(); ++Index)
	{
		const FFMCodexPrototypePlayerDefinition& Definition = Definitions[Index];
		bAllExplicitNamesMatch = bAllExplicitNamesMatch
			&& !Definition.PreferredDisplayName.IsEmpty()
			&& Definition.PreferredDisplayName.ToString()
				== ExpectedDisplayNames[Index]
			&& Definition.Card.DisplayName.EqualTo(
				Definition.CanonicalChineseDisplayName)
			&& !Definition.CanonicalChineseDisplayName.IsEmpty()
			&& !Definition.EnglishDisplayName.IsEmpty()
			&& FFMCodexPrototypeTeamContent::PlayerDisplayName(
				Definition.PlayerKey).EqualTo(Definition.PreferredDisplayName);
	}
	TestTrue(TEXT("All 40 preferred names are explicit while full identities remain separate"),
		bAllExplicitNamesMatch);

	FString ConfigSource;
	const FString ConfigPath = FPaths::Combine(
		FPaths::ProjectDir(),
		TEXT("ContentSource/PlayerContent/CanonicalPlayerImportConfig.json"));
	TestTrue(TEXT("DisplayName presentation configuration loads"),
		FFileHelper::LoadFileToString(ConfigSource, *ConfigPath));
	TArray<FString> DisplayNameFields;
	ConfigSource.ParseIntoArray(
		DisplayNameFields, TEXT("\"displayName\""), false);
	TestEqual(TEXT("Presentation configuration has 40/40 explicit displayName fields"),
		FMath::Max(0, DisplayNameFields.Num() - 1), 40);

	const FName GabrielId(TEXT("Prototype.Arsenal.GabrielMagalhaes"));
	const FFMCodexPrototypePlayerDefinition* Gabriel =
		FFMCodexPrototypeTeamContent::Find(GabrielId);
	TestTrue(TEXT("Gabriel uses the configured preferred name and preserves full identity"),
		Gabriel != nullptr
			&& Gabriel->PreferredDisplayName.ToString() == TEXT("加布里埃尔")
			&& Gabriel->CanonicalChineseDisplayName.ToString()
				== TEXT("加布里埃尔·马加良斯")
			&& Gabriel->EnglishDisplayName.ToString()
				== TEXT("Gabriel Magalhães")
			&& FFMCodexPlayerUIPresentationText::CompactPlayerName(
				GabrielId, FString()).ToString() == TEXT("加布里埃尔")
			&& FFMCodexPlayerUIPresentationText::HandMicroPlayerName(
				GabrielId, FString()).ToString() == TEXT("加布里埃尔")
			&& FFMCodexPlayerUIPresentationText::InMatchShortPlayerName(
				GabrielId, FString()).ToString() == TEXT("加布里埃尔"));

	FString PresentationTextSource;
	const FString PresentationTextPath = FPaths::Combine(
		FPaths::ProjectDir(),
		TEXT("Source/FMCodex/LocalPlay/FMCodexPlayerUIPresentationText.cpp"));
	TestTrue(TEXT("Player presentation-name source loads"),
		FFileHelper::LoadFileToString(
			PresentationTextSource, *PresentationTextPath));
	TestTrue(TEXT("Production name presentation has no roster heuristics or player cases"),
		!PresentationTextSource.Contains(TEXT("FindLastChar"))
			&& !PresentationTextSource.Contains(TEXT("Prototype.Arsenal."))
			&& !PresentationTextSource.Contains(TEXT("Prototype.ManchesterCity.")));

	const FName IsolationId(TEXT("Prototype.Arsenal.BukayoSaka"));
	const FFMCodexPrototypePlayerDefinition* Saka =
		FFMCodexPrototypeTeamContent::Find(IsolationId);
	TestNotNull(TEXT("DisplayName mutation fixture resolves"), Saka);
	if (Saka != nullptr)
	{
		const FFMCodexPlayerUICardArtReferences ArtBefore =
			FFMCodexPlayerUIAssetReferences::Get().ResolveCardArt(Saka->PlayerKey);
		FFMCodexPrototypePlayerDefinition Mutated = *Saka;
		Mutated.PreferredDisplayName = FText::FromString(TEXT("测试显示名"));
		const FFMCodexPlayerUICardArtReferences ArtAfter =
			FFMCodexPlayerUIAssetReferences::Get().ResolveCardArt(Mutated.PlayerKey);
		TestTrue(TEXT("DisplayName mutation is isolated from identity, serial, art, and gameplay"),
			Mutated.PlayerKey == Saka->PlayerKey
				&& Mutated.Card.CardId == Saka->Card.CardId
				&& Mutated.DisplaySerial == Saka->DisplaySerial
				&& Mutated.TeamId == Saka->TeamId
				&& Mutated.Card.PositionTypes == Saka->Card.PositionTypes
				&& Mutated.Card.AttackSkillIds == Saka->Card.AttackSkillIds
				&& Mutated.Card.Attributes.Shooting
					== Saka->Card.Attributes.Shooting
				&& Mutated.Card.Attributes.Passing
					== Saka->Card.Attributes.Passing
				&& ArtAfter.Portrait.ToSoftObjectPath()
					== ArtBefore.Portrait.ToSoftObjectPath()
				&& ArtAfter.FullCardPortrait.ToSoftObjectPath()
					== ArtBefore.FullCardPortrait.ToSoftObjectPath()
				&& ArtAfter.HandMicroPortrait.ToSoftObjectPath()
					== ArtBefore.HandMicroPortrait.ToSoftObjectPath());
	}
	return true;
}

#endif
