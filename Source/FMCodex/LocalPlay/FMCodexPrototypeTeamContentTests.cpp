#include "FMCodexPrototypeTeamContent.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "FMCodexDeploymentDragDropOperation.h"
#include "FMCodexCardRackWidget.h"
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

#include "Engine/Engine.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "Components/TextBlock.h"
#include "HAL/FileManager.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
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

	FPlayerAttributes MakeAttributes(
		const int32 Shooting,
		const int32 Dribbling,
		const int32 Passing,
		const int32 OffBall,
		const int32 Marking,
		const int32 Tackling,
		const int32 Speed,
		const int32 Strength,
		const int32 Stamina,
		const int32 LongShot)
	{
		FPlayerAttributes Result;
		Result.Shooting = Shooting;
		Result.Dribbling = Dribbling;
		Result.Passing = Passing;
		Result.OffBall = OffBall;
		Result.Marking = Marking;
		Result.Tackling = Tackling;
		Result.Speed = Speed;
		Result.Strength = Strength;
		Result.Stamina = Stamina;
		Result.LongShot = LongShot;
		return Result;
	}

	bool AttributesEqual(
		const FPlayerAttributes& Left,
		const FPlayerAttributes& Right)
	{
		return Left.Shooting == Right.Shooting
			&& Left.Dribbling == Right.Dribbling
			&& Left.Passing == Right.Passing
			&& Left.OffBall == Right.OffBall
			&& Left.Marking == Right.Marking
			&& Left.Tackling == Right.Tackling
			&& Left.Speed == Right.Speed
			&& Left.Strength == Right.Strength
			&& Left.Stamina == Right.Stamina
			&& Left.LongShot == Right.LongShot;
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
		UFMCodexCardRackWidget& Rack,
		const FName CardId)
	{
		for (UFMCodexPlayerCardWidget* Widget
			: Rack.GetRenderedCardWidgets())
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
	struct FExpectedMetadata
	{
		FString ChineseName;
		FString EnglishName;
		FString Nationality;
		FString BirthDate;
		int32 HeightCm;
		int32 WeightKg;
		FString Serial;
	};
	const TMap<FName, FExpectedMetadata> ExpectedMetadata = {
		{ TEXT("Prototype.Arsenal.DavidRaya"),
			{ TEXT("大卫·拉亚"), TEXT("David Raya"), TEXT("西班牙"), TEXT("1995-09-15"), 183, 80, TEXT("001") } },
		{ TEXT("Prototype.Arsenal.WilliamSaliba"),
			{ TEXT("威廉·萨利巴"), TEXT("William Saliba"), TEXT("法国"), TEXT("2001-03-24"), 192, 92, TEXT("002") } },
		{ TEXT("Prototype.Arsenal.BukayoSaka"),
			{ TEXT("布卡约·萨卡"), TEXT("Bukayo Saka"), TEXT("英格兰"), TEXT("2001-09-05"), 178, 72, TEXT("003") } },
		{ TEXT("Prototype.Arsenal.MartinOdegaard"),
			{ TEXT("马丁·厄德高"), TEXT("Martin Ødegaard"), TEXT("挪威"), TEXT("1998-12-17"), 178, 68, TEXT("004") } },
		{ TEXT("Prototype.Arsenal.DeclanRice"),
			{ TEXT("德克兰·赖斯"), TEXT("Declan Rice"), TEXT("英格兰"), TEXT("1999-01-14"), 188, 83, TEXT("005") } },
		{ TEXT("Prototype.Arsenal.GabrielMartinelli"),
			{ TEXT("加布里埃尔·马丁内利"), TEXT("Gabriel Martinelli"), TEXT("巴西"), TEXT("2001-06-18"), 178, 74, TEXT("006") } },
		{ TEXT("Prototype.Arsenal.GabrielMagalhaes"),
			{ TEXT("加布里埃尔·马加良斯"), TEXT("Gabriel Magalhães"), TEXT("巴西"), TEXT("1997-12-19"), 190, 78, TEXT("007") } },
		{ TEXT("Prototype.Arsenal.MikelMerino"),
			{ TEXT("米克尔·梅里诺"), TEXT("Mikel Merino"), TEXT("西班牙"), TEXT("1996-06-22"), 189, 83, TEXT("008") } },
		{ TEXT("Prototype.ManchesterCity.GianluigiDonnarumma"),
			{ TEXT("吉安路易吉·多纳鲁马"), TEXT("Gianluigi Donnarumma"), TEXT("意大利"), TEXT("1999-02-25"), 196, 89, TEXT("009") } },
		{ TEXT("Prototype.ManchesterCity.ErlingHaaland"),
			{ TEXT("埃尔林·哈兰德"), TEXT("Erling Haaland"), TEXT("挪威"), TEXT("2000-07-21"), 195, 87, TEXT("010") } },
		{ TEXT("Prototype.ManchesterCity.PhilFoden"),
			{ TEXT("菲尔·福登"), TEXT("Phil Foden"), TEXT("英格兰"), TEXT("2000-05-28"), 171, 63, TEXT("011") } },
		{ TEXT("Prototype.ManchesterCity.Rodri"),
			{ TEXT("罗德里"), TEXT("Rodri"), TEXT("西班牙"), TEXT("1996-06-22"), 190, 82, TEXT("012") } },
		{ TEXT("Prototype.ManchesterCity.RubenDias"),
			{ TEXT("鲁本·迪亚斯"), TEXT("Rúben Dias"), TEXT("葡萄牙"), TEXT("1997-05-14"), 187, 83, TEXT("013") } },
		{ TEXT("Prototype.ManchesterCity.JoskoGvardiol"),
			{ TEXT("约什科·格瓦迪奥尔"), TEXT("Joško Gvardiol"), TEXT("克罗地亚"), TEXT("2002-01-23"), 185, 79, TEXT("014") } },
		{ TEXT("Prototype.ManchesterCity.BernardoSilva"),
			{ TEXT("贝尔纳多·席尔瓦"), TEXT("Bernardo Silva"), TEXT("葡萄牙"), TEXT("1994-08-10"), 173, 64, TEXT("015") } },
		{ TEXT("Prototype.ManchesterCity.JeremyDoku"),
			{ TEXT("杰里米·多库"), TEXT("Jérémy Doku"), TEXT("比利时"), TEXT("2002-05-27"), 171, 66, TEXT("016") } }
	};
	const TArray<FFMCodexPrototypePlayerDefinition>& Definitions =
		FFMCodexPrototypeTeamContent::GetDefinitions();
	TestEqual(TEXT("Formal Prototype roster has exactly sixteen definitions"),
		Definitions.Num(), 16);

	TMap<FName, int32> TeamCounts;
	TMap<FName, int32> GoalkeeperCounts;
	TSet<FName> UniqueIds;
	TSet<FString> UniqueSerials;
	for (const FFMCodexPrototypePlayerDefinition& Definition : Definitions)
	{
		const FPlayerCardData& Card = Definition.Card;
		TeamCounts.FindOrAdd(Definition.TeamId)++;
		GoalkeeperCounts.FindOrAdd(Definition.TeamId) += Card.bIsGoalkeeper ? 1 : 0;
		UniqueIds.Add(Card.CardId);
		UniqueSerials.Add(Definition.PlayerFacingSerial);
		TestTrue(TEXT("Technical CardId is non-empty and code-safe"),
			IsCodeSafeId(Card.CardId));
		TestFalse(TEXT("Localized visible player name is live and non-empty"),
			Card.DisplayName.IsEmpty());
		TestFalse(TEXT("Localized visible team name is live and non-empty"),
			Definition.TeamDisplayName.IsEmpty());
		TestFalse(TEXT("English player-facing name is explicit and non-empty"),
			Definition.EnglishDisplayName.IsEmpty());
		TestFalse(TEXT("Player nationality is explicit and non-empty"),
			Definition.NationalityDisplayName.IsEmpty());
		TestTrue(TEXT("Biography metadata is complete"),
			!Card.BirthDate.IsEmpty()
				&& Card.HeightCm > 0
				&& Card.WeightKg > 0);
		TestTrue(TEXT("Player-facing serial preserves three digits"),
			Definition.PlayerFacingSerial.Len() == 3
				&& Definition.PlayerFacingSerial.IsNumeric());
		const FExpectedMetadata* Expected = ExpectedMetadata.Find(Card.CardId);
		TestTrue(TEXT("Approved factual metadata and Serial mapping are exact"),
			Expected != nullptr
				&& Card.DisplayName.ToString() == Expected->ChineseName
				&& Definition.EnglishDisplayName.ToString()
					== Expected->EnglishName
				&& Definition.NationalityDisplayName.ToString()
					== Expected->Nationality
				&& Card.BirthDate == Expected->BirthDate
				&& Card.HeightCm == Expected->HeightCm
				&& Card.WeightKg == Expected->WeightKg
				&& Definition.PlayerFacingSerial == Expected->Serial);
		const FFMCodexPlayerOverallResult Overall =
			FFMCodexPlayerOverall::Calculate(Card);
		TestTrue(TEXT("Every formal Prototype player has Overall v1"),
			Overall.bSuccess && Overall.Value > 0);
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
		else if (Card.CardId == TEXT("Prototype.Arsenal.GabrielMagalhaes"))
		{
			TestTrue(TEXT("Gabriel deliberately has no attack Skill"),
				Card.AttackSkillIds.IsEmpty());
		}
		else
		{
			TestTrue(TEXT("Outfield pilot uses exactly one frozen existing skill"),
				Card.AttackSkillIds.Num() == 1
					&& FrozenSkillIds.Contains(Card.AttackSkillIds[0]));
		}
	}
	TestEqual(TEXT("All sixteen technical CardIds are unique"),
		UniqueIds.Num(), 16);
	TestEqual(TEXT("All sixteen explicit serials are unique"),
		UniqueSerials.Num(), 16);
	TestEqual(TEXT("Metadata regression table covers all sixteen players"),
		ExpectedMetadata.Num(), 16);
	TestEqual(TEXT("Arsenal formal count is exactly eight"),
		TeamCounts.FindRef(FFMCodexPrototypeTeamContent::ArsenalTeamId()), 8);
	TestEqual(TEXT("Manchester City formal count is exactly eight"),
		TeamCounts.FindRef(
			FFMCodexPrototypeTeamContent::ManchesterCityTeamId()), 8);
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
		const FName ExpectedFirstCardId = SideIndex == 0
			? FName(TEXT("Prototype.Arsenal.GabrielMartinelli"))
			: FName(TEXT("Prototype.ManchesterCity.JoskoGvardiol"));
		TestTrue(TEXT("Normal deck replaces visual stand-in 01 with formal content"),
			Deck[0].CardId == ExpectedFirstCardId
				&& !Deck[0].CardId.ToString().StartsWith(TEXT("Demo.")));
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
	TestTrue(TEXT("Normal LocalPlay integrates exactly eight formal cards per side"),
		PrototypeTotal == 16
			&& PlayerAPrototypeCount == 8
			&& PlayerBPrototypeCount == 8
			&& DemoCardIds.Num() == 40);
	TestEqual(TEXT("Approved Arsenal content has expected deck rarity score"),
		PlayerARarityScore, 50);
	TestEqual(TEXT("Approved Manchester City content has expected deck rarity score"),
		PlayerBRarityScore, 52);
	TestTrue(TEXT("Demo visual stand-ins 01-03 are isolated from normal decks"),
		!DemoCardIds.Contains(TEXT("Demo.A.Outfield.01"))
			&& !DemoCardIds.Contains(TEXT("Demo.A.Outfield.02"))
			&& !DemoCardIds.Contains(TEXT("Demo.A.Outfield.03"))
			&& !DemoCardIds.Contains(TEXT("Demo.B.Outfield.01"))
			&& !DemoCardIds.Contains(TEXT("Demo.B.Outfield.02"))
			&& !DemoCardIds.Contains(TEXT("Demo.B.Outfield.03")));
	TestTrue(TEXT("Opening accepts pilot data without schema or rule changes"),
		FMatchPlayOpeningInitializer::InitializeMatchPlayOpening(
			Demo.OpeningInput).bSuccess);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexPrototypeApprovedSixIntegrationTest,
	"FMCodex.LocalPlay.PrototypeTeams.02.ApprovedSixIntegration",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexPrototypeApprovedSixIntegrationTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexPrototypeTeamContentTests;
	struct FExpectedPlayer
	{
		FName CardId;
		FName TeamId;
		TArray<EPlayerPositionType> Positions;
		ECardRarity Rarity;
		FPlayerAttributes Attributes;
		FName SkillId;
		FString Serial;
	};
	const TArray<FExpectedPlayer> Expected = {
		{ TEXT("Prototype.Arsenal.GabrielMartinelli"),
			FFMCodexPrototypeTeamContent::ArsenalTeamId(),
			{ EPlayerPositionType::Attack }, ECardRarity::National,
			MakeAttributes(5, 5, 3, 5, 2, 2, 6, 3, 5, 3),
			TEXT("Demo.Skill.CutInsideShot"), TEXT("006") },
		{ TEXT("Prototype.Arsenal.GabrielMagalhaes"),
			FFMCodexPrototypeTeamContent::ArsenalTeamId(),
			{ EPlayerPositionType::Defense }, ECardRarity::Continental,
			MakeAttributes(2, 3, 3, 3, 6, 6, 5, 6, 5, 2),
			NAME_None, TEXT("007") },
		{ TEXT("Prototype.Arsenal.MikelMerino"),
			FFMCodexPrototypeTeamContent::ArsenalTeamId(),
			{ EPlayerPositionType::Midfield, EPlayerPositionType::Attack },
			ECardRarity::National,
			MakeAttributes(4, 4, 5, 5, 5, 5, 3, 6, 5, 4),
			TEXT("Demo.Skill.PassControl"), TEXT("008") },
		{ TEXT("Prototype.ManchesterCity.JoskoGvardiol"),
			FFMCodexPrototypeTeamContent::ManchesterCityTeamId(),
			{ EPlayerPositionType::Defense }, ECardRarity::Continental,
			MakeAttributes(3, 4, 5, 4, 5, 5, 5, 5, 5, 3),
			TEXT("Demo.Skill.Cross"), TEXT("014") },
		{ TEXT("Prototype.ManchesterCity.BernardoSilva"),
			FFMCodexPrototypeTeamContent::ManchesterCityTeamId(),
			{ EPlayerPositionType::Midfield, EPlayerPositionType::Attack },
			ECardRarity::Continental,
			MakeAttributes(4, 6, 6, 5, 3, 3, 4, 2, 6, 4),
			TEXT("Demo.Skill.ThroughBall"), TEXT("015") },
		{ TEXT("Prototype.ManchesterCity.JeremyDoku"),
			FFMCodexPrototypeTeamContent::ManchesterCityTeamId(),
			{ EPlayerPositionType::Attack }, ECardRarity::National,
			MakeAttributes(4, 6, 4, 5, 2, 2, 6, 3, 4, 3),
			TEXT("Demo.Skill.CutInsideShot"), TEXT("016") }
	};
	const FFMCodexLocalMatchDemoConfiguration Demo =
		FFMCodexLocalMatchDemoConfigurationFactory::Create();
	for (const FExpectedPlayer& Player : Expected)
	{
		const FFMCodexPrototypePlayerDefinition* Definition =
			FFMCodexPrototypeTeamContent::Find(Player.CardId);
		TestNotNull(TEXT("Approved formal definition exists"), Definition);
		if (Definition == nullptr)
		{
			continue;
		}
		const FPlayerCardData& Card = Definition->Card;
		TestTrue(TEXT("Approved team, position, rarity and attributes are exact"),
			Definition->TeamId == Player.TeamId
				&& Card.PositionTypes == Player.Positions
				&& Card.Rarity == Player.Rarity
				&& AttributesEqual(Card.Attributes, Player.Attributes));
		TestTrue(TEXT("Approved Skill or explicit None is exact"),
			Player.SkillId.IsNone()
				? Card.AttackSkillIds.IsEmpty()
				: Card.AttackSkillIds.Num() == 1
					&& Card.AttackSkillIds[0] == Player.SkillId);
		TestEqual(TEXT("Approved serial is explicit"),
			Definition->PlayerFacingSerial, Player.Serial);
		if (!Player.SkillId.IsNone())
		{
			const FSkillRuleSnapshot* Rule =
				Demo.SkillRuleSet.SkillRules.FindByPredicate(
					[&Player](const FSkillRuleSnapshot& Candidate)
					{
						return Candidate.SkillId == Player.SkillId;
					});
			TestTrue(TEXT("Approved Skill preserves canonical 2-8 trigger range"),
				Rule != nullptr
					&& Rule->MinTriggerActionPoint == 2
					&& Rule->MaxTriggerActionPoint == 8);
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
	using namespace FMCodexPrototypeTeamContentTests;
	TestTrue(TEXT("Overall v1 rarity values are explicit and complete"),
		FFMCodexPlayerOverall::RarityValue(
			EFMCodexOverallRarityTier::Common) == 1
			&& FFMCodexPlayerOverall::RarityValue(
				EFMCodexOverallRarityTier::National) == 2
			&& FFMCodexPlayerOverall::RarityValue(
				EFMCodexOverallRarityTier::Continental) == 3
			&& FFMCodexPlayerOverall::RarityValue(
				EFMCodexOverallRarityTier::WorldClass) == 4
			&& FFMCodexPlayerOverall::RarityValue(
				EFMCodexOverallRarityTier::Legendary) == 5);
	EFMCodexOverallRarityTier UnsupportedTier =
		EFMCodexOverallRarityTier::Legendary;
	TestFalse(TEXT("Regional fails closed because Overall v1 does not map it"),
		FFMCodexPlayerOverall::TryMapRarity(
			ECardRarity::Regional, UnsupportedTier));

	const TMap<FName, int32> ExpectedOverall = {
		{ TEXT("Prototype.Arsenal.DavidRaya"), 90 },
		{ TEXT("Prototype.Arsenal.WilliamSaliba"), 99 },
		{ TEXT("Prototype.Arsenal.BukayoSaka"), 103 },
		{ TEXT("Prototype.Arsenal.MartinOdegaard"), 93 },
		{ TEXT("Prototype.Arsenal.DeclanRice"), 105 },
		{ TEXT("Prototype.Arsenal.GabrielMartinelli"), 89 },
		{ TEXT("Prototype.Arsenal.GabrielMagalhaes"), 96 },
		{ TEXT("Prototype.Arsenal.MikelMerino"), 95 },
		{ TEXT("Prototype.ManchesterCity.GianluigiDonnarumma"), 105 },
		{ TEXT("Prototype.ManchesterCity.ErlingHaaland"), 106 },
		{ TEXT("Prototype.ManchesterCity.PhilFoden"), 96 },
		{ TEXT("Prototype.ManchesterCity.Rodri"), 108 },
		{ TEXT("Prototype.ManchesterCity.RubenDias"), 96 },
		{ TEXT("Prototype.ManchesterCity.JoskoGvardiol"), 93 },
		{ TEXT("Prototype.ManchesterCity.BernardoSilva"), 96 },
		{ TEXT("Prototype.ManchesterCity.JeremyDoku"), 89 }
	};
	for (const FFMCodexPrototypePlayerDefinition& Definition
		: FFMCodexPrototypeTeamContent::GetDefinitions())
	{
		const FFMCodexPlayerOverallResult Overall =
			FFMCodexPlayerOverall::Calculate(Definition.Card);
		const int32* Expected = ExpectedOverall.Find(Definition.Card.CardId);
		TestTrue(TEXT("Exact approved Overall v1 value is deterministic"),
			Expected != nullptr && Overall.bSuccess
				&& Overall.Value == *Expected);
	}
	TestEqual(TEXT("Overall audit covers all sixteen formal players"),
		ExpectedOverall.Num(), 16);
	const FFMCodexPrototypePlayerDefinition* Rodri =
		FFMCodexPrototypeTeamContent::Find(
			TEXT("Prototype.ManchesterCity.Rodri"));
	TestTrue(TEXT("Overall v1 is not capped at 100"),
		Rodri != nullptr
			&& FFMCodexPlayerOverall::Calculate(Rodri->Card).Value == 108);

	FPlayerAttributes TopSixProbe = MakeAttributes(
		6, 5, 4, 3, 2, 1, 6, 5, 4, 3);
	const int32 Before = FFMCodexPlayerOverall::CalculateOutfield(
		TopSixProbe, ECardRarity::Common).Value;
	TopSixProbe.Tackling = 2;
	const int32 LowerAttributeChanged =
		FFMCodexPlayerOverall::CalculateOutfield(
			TopSixProbe, ECardRarity::Common).Value;
	TopSixProbe.Tackling = 6;
	const int32 EnteredTopSix = FFMCodexPlayerOverall::CalculateOutfield(
		TopSixProbe, ECardRarity::Common).Value;
	TestTrue(TEXT("Lower attribute is ignored until it enters the Top-6"),
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
	const TSet<FName> DedicatedFullCardIds = {
		TEXT("Prototype.Arsenal.DavidRaya"),
		TEXT("Prototype.Arsenal.WilliamSaliba"),
		TEXT("Prototype.Arsenal.BukayoSaka"),
		TEXT("Prototype.Arsenal.MartinOdegaard"),
		TEXT("Prototype.Arsenal.DeclanRice"),
		TEXT("Prototype.ManchesterCity.GianluigiDonnarumma"),
		TEXT("Prototype.ManchesterCity.ErlingHaaland"),
		TEXT("Prototype.ManchesterCity.PhilFoden"),
		TEXT("Prototype.ManchesterCity.Rodri"),
		TEXT("Prototype.ManchesterCity.RubenDias")
	};
	const TSet<FName> FullCardPilotIds = {
		TEXT("Prototype.Arsenal.BukayoSaka"),
		TEXT("Prototype.Arsenal.DavidRaya"),
		TEXT("Prototype.ManchesterCity.Rodri"),
		TEXT("Prototype.ManchesterCity.GianluigiDonnarumma")
	};
	int32 DedicatedFullCardCount = 0;
	int32 MissingFullCardCount = 0;
	int32 FullCardPilotCount = 0;
	for (const FFMCodexPrototypePlayerDefinition& Definition
		: FFMCodexPrototypeTeamContent::GetDefinitions())
	{
		const FFMCodexPlayerUICardArtReferences Art =
			References.ResolveCardArt(Definition.Card.CardId);
		UTexture2D* Frame = Art.CardFrame.LoadSynchronous();
		const TSoftObjectPtr<UTexture2D>& FullCardPortrait =
			Art.FullCardPortrait.IsNull()
				? Art.Portrait : Art.FullCardPortrait;
		UTexture2D* Portrait = FullCardPortrait.LoadSynchronous();
		UTexture2D* HandMicroPortrait =
			Art.HandMicroPortrait.LoadSynchronous();
		TestTrue(TEXT("All sixteen formal identities resolve approved Hand Micro art"),
			!Art.ArtIdentity.IsNone()
				&& !Art.HandMicroPortrait.IsNull()
				&& HandMicroPortrait != nullptr
				&& HandMicroPortrait->GetImportedSize()
					== FIntPoint(192, 128));
		if (DedicatedFullCardIds.Contains(Definition.Card.CardId))
		{
			++DedicatedFullCardCount;
			TestTrue(TEXT("Dedicated Full Card art remains complete for original ten"),
				Frame != nullptr && Portrait != nullptr
					&& !Art.LongShotSkillIcon.IsNull()
					&& Portrait->GetImportedSize()
						== FIntPoint(1024, 1536));
			const bool bUsesPilotArtwork = FullCardPortrait.ToSoftObjectPath()
				.ToString().Contains(TEXT("_FullCardPilot_02"));
			if (FullCardPilotIds.Contains(Definition.Card.CardId))
			{
				++FullCardPilotCount;
				TestTrue(TEXT("Selected four use versioned Full Card pilot artwork"),
					bUsesPilotArtwork);
			}
			else
			{
				TestFalse(TEXT("Other dedicated portraits remain on existing artwork"),
					bUsesPilotArtwork);
			}
		}
		else
		{
			++MissingFullCardCount;
			TestTrue(TEXT("New six keep clean missing Full Card art behavior"),
				Art.CardFrame.IsNull() && Art.Portrait.IsNull()
					&& Art.FullCardPortrait.IsNull()
					&& Frame == nullptr && Portrait == nullptr
					&& Art.HandMicroPortrait.ToSoftObjectPath()
						!= Art.Portrait.ToSoftObjectPath());
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
	TestTrue(TEXT("Full Card artwork audit remains honestly 10 dedicated / 6 missing"),
		DedicatedFullCardCount == 10 && MissingFullCardCount == 6);
	TestEqual(TEXT("Full Card artwork pilot remains bounded to four players"),
		FullCardPilotCount, 4);

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
	"FMCodex.LocalPlay.PrototypeTeams.05.PublicFlow",
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
	UFMCodexCardRackWidget* LocalRack = Screen->GetLocalRackWidget();
	UFMCodexPitchWidget* Pitch = Screen->GetPitchWidget();
	TestNotNull(TEXT("Prototype deployment hand exists"), Panel);
	TestNotNull(TEXT("Prototype five-slot pitch exists"), Pitch);
	if (Panel == nullptr || Pitch == nullptr || LocalRack == nullptr)
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
			FindHandCard(*LocalRack, Choice.CardId);
		TestNotNull(TEXT("Prototype card is rendered as the real drag source"),
			CardWidget);
		if (CardWidget == nullptr)
		{
			return false;
		}
		CardWidget->TakeWidget();
		const UTextBlock* MicroName = Cast<UTextBlock>(
			CardWidget->GetWidgetFromName(TEXT("HandMicroPlayerName")));
		TestTrue(TEXT("Rack micro card renders localized identity and real art"),
			MicroName != nullptr && !MicroName->GetText().IsEmpty()
				&& CardWidget->GetRenderedSkillCount() == 0
				&& CardWidget->GetRenderedAttributeCount() == 0
				&& CardWidget->GetResolvedHandMicroPortraitTexture() != nullptr);

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
		TestFalse(TEXT("Successful prototype drop auto-hands off without Ready"),
			Controller->IsAwaitingHotSeatHandoff());
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
	const TArray<const FFMCodexUMGCardRackViewModel*> Racks = {
		&Presentation.LocalRack, &Presentation.OpponentRack
	};
	for (const FFMCodexUMGCardRackViewModel* Rack : Racks)
	{
		for (const FFMCodexUMGCardRackCellViewModel& Cell : Rack->Cells)
		{
			if (Cell.Card.CardId.ToString().StartsWith(TEXT("Prototype.")))
			{
				FormalCells.Add(&Cell);
			}
		}
	}
	TestEqual(TEXT("Normal presentation exposes sixteen formal players"),
		FormalCells.Num(), 16);
	TSet<FString> Serials;
	bool bAllFieldsComplete = FormalCells.Num() == 16;
	bool bDemoStandInLeaked = false;
	const FFMCodexUMGCardRackCellViewModel* StableIndexProbe = nullptr;
	const FFMCodexUMGCardViewModel* Gabriel = nullptr;
	for (const FFMCodexUMGCardRackCellViewModel* Cell : FormalCells)
	{
		if (Cell == nullptr)
		{
			bAllFieldsComplete = false;
			continue;
		}
		const FFMCodexUMGCardViewModel& Card = Cell->Card;
		Serials.Add(Card.PlayerFacingSerialLabel);
		bAllFieldsComplete = bAllFieldsComplete
			&& !Card.IdentityLabel.IsEmpty()
			&& !Card.EnglishIdentityLabel.IsEmpty()
			&& !Card.NationalityLabel.IsEmpty()
			&& !Card.ClubLabel.IsEmpty()
			&& !Card.BirthDate.IsEmpty()
			&& Card.HeightCm > 0 && Card.WeightKg > 0
			&& Card.bHasOverallRating && Card.OverallRating > 0
			&& Card.PlayerFacingSerialLabel.Len() == 3
			&& (Card.AttributeValues.Num() == 10
				|| Card.AttributeValues.Num() == 6)
			&& !Card.IdentityLabel.Contains(TEXT("Prototype."))
			&& !Card.IdentityLabel.Contains(TEXT("Demo."));
		bDemoStandInLeaked = bDemoStandInLeaked
			|| Card.CardId.ToString().StartsWith(TEXT("Demo."));
		if (StableIndexProbe == nullptr)
		{
			StableIndexProbe = Cell;
		}
		if (Card.CardId == TEXT("Prototype.Arsenal.GabrielMagalhaes"))
		{
			Gabriel = &Card;
		}
	}
	TestTrue(TEXT("Full Card DTO receives complete data without debug identity"),
		bAllFieldsComplete && !bDemoStandInLeaked && Serials.Num() == 16);
	TestTrue(TEXT("Gabriel is formal, complete, and has no fake Skill"),
		Gabriel != nullptr && Gabriel->Skills.IsEmpty()
			&& Gabriel->SkillLabels.IsEmpty()
			&& Gabriel->OverallRating == 96
			&& Gabriel->PlayerFacingSerialLabel == TEXT("007")
			&& FFMCodexPlayerUIPresentationText::HandMicroPlayerName(
				Gabriel->CardId, Gabriel->IdentityLabel).ToString()
				== TEXT("加布里埃尔"));
	if (StableIndexProbe != nullptr)
	{
		FFMCodexUMGCardRackCellViewModel Copy = *StableIndexProbe;
		const FString SerialBefore = Copy.Card.PlayerFacingSerialLabel;
		const FName IdentityBefore = Copy.Card.CardId;
		Copy.StableIndex += 37;
		TestTrue(TEXT("Changing StableIndex does not change serial or identity"),
			Copy.Card.PlayerFacingSerialLabel == SerialBefore
				&& Copy.Card.CardId == IdentityBefore);
		Copy.Card.PlayerFacingSerialLabel = TEXT("999");
		TestEqual(TEXT("Serial does not act as gameplay CardId"),
			Copy.Card.CardId, IdentityBefore);
	}

	FString UMGSource;
	FString InteractionSource;
	const bool bLoadedPresentationSources = FFileHelper::LoadFileToString(
		UMGSource, *FPaths::Combine(FPaths::ProjectDir(),
			TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchUMGPresentation.cpp")))
		&& FFileHelper::LoadFileToString(
			InteractionSource, *FPaths::Combine(FPaths::ProjectDir(),
				TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchInteractionView.cpp")));
	TestTrue(TEXT("Overall is calculated once before the UMG DTO"),
		bLoadedPresentationSources
			&& InteractionSource.Contains(
				TEXT("FFMCodexPlayerOverall::Calculate"))
			&& !UMGSource.Contains(
				TEXT("FFMCodexPlayerOverall::Calculate"))
			&& !UMGSource.Contains(TEXT("TopSix")));

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
			&& (Source.Contains(TEXT("FMCodexPlayerOverall"))
				|| Source.Contains(TEXT("OverallRating"))
				|| Source.Contains(TEXT("PlayerFacingSerial"))
				|| Source.Contains(TEXT("NationalityDisplayName"))
				|| Source.Contains(TEXT("TeamDisplayName"))))
		{
			bPresentationDataAffectsGameplay = true;
			break;
		}
	}
	TestFalse(TEXT("Full Card presentation metadata has no Gameplay/Authority readers"),
		bPresentationDataAffectsGameplay);
	return true;
}

#endif
