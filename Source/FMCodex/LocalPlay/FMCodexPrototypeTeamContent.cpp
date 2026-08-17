#include "FMCodexPrototypeTeamContent.h"

namespace FMCodexPrototypeTeamContent
{
	const FName ArsenalId(TEXT("Prototype.Team.Arsenal"));
	const FName ManchesterCityId(TEXT("Prototype.Team.ManchesterCity"));
	constexpr int32 PrototypeCardsPerTeam = 8;

	const FName LongShotSkillId(TEXT("Demo.Skill.LongShot"));
	const FName CutInsideShotSkillId(TEXT("Demo.Skill.CutInsideShot"));
	const FName PassControlSkillId(TEXT("Demo.Skill.PassControl"));
	const FName CrossSkillId(TEXT("Demo.Skill.Cross"));
	const FName ThroughBallSkillId(TEXT("Demo.Skill.ThroughBall"));

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

	FGoalkeeperAttributes MakeGoalkeeperAttributes(
		const int32 Handling,
		const int32 Positioning,
		const int32 Reflex,
		const int32 Aerial,
		const int32 Anticipation,
		const int32 OneOnOne)
	{
		FGoalkeeperAttributes Result;
		Result.Handling = Handling;
		Result.Positioning = Positioning;
		Result.Reflex = Reflex;
		Result.Aerial = Aerial;
		Result.Anticipation = Anticipation;
		Result.OneOnOne = OneOnOne;
		return Result;
	}

	FFMCodexPrototypePlayerDefinition MakeOutfield(
		const FName TeamId,
		const FText& TeamName,
		const TCHAR* CardId,
		const FText& PlayerName,
		const FText& EnglishName,
		const FText& NationalityName,
		const TCHAR* BirthDate,
		const int32 HeightCm,
		const int32 WeightKg,
		const TCHAR* Serial,
		const TArray<EPlayerPositionType>& Positions,
		const FPlayerAttributes& Attributes,
		const ECardRarity Rarity,
		const FName SkillId)
	{
		FFMCodexPrototypePlayerDefinition Result;
		Result.TeamId = TeamId;
		Result.TeamDisplayName = TeamName;
		Result.EnglishDisplayName = EnglishName;
		Result.NationalityDisplayName = NationalityName;
		Result.PlayerFacingSerial = Serial;
		Result.Card.CardId = FName(CardId);
		Result.Card.DisplayName = PlayerName;
		Result.Card.BirthDate = BirthDate;
		Result.Card.HeightCm = HeightCm;
		Result.Card.WeightKg = WeightKg;
		Result.Card.Rarity = Rarity;
		Result.Card.PositionTypes = Positions;
		Result.Card.Attributes = Attributes;
		if (!SkillId.IsNone())
		{
			Result.Card.AttackSkillIds = { SkillId };
		}
		return Result;
	}

	FFMCodexPrototypePlayerDefinition MakeGoalkeeper(
		const FName TeamId,
		const FText& TeamName,
		const TCHAR* CardId,
		const FText& PlayerName,
		const FText& EnglishName,
		const FText& NationalityName,
		const TCHAR* BirthDate,
		const int32 HeightCm,
		const int32 WeightKg,
		const TCHAR* Serial,
		const FPlayerAttributes& BaseAttributes,
		const FGoalkeeperAttributes& GoalkeeperAttributes,
		const ECardRarity Rarity)
	{
		FFMCodexPrototypePlayerDefinition Result;
		Result.TeamId = TeamId;
		Result.TeamDisplayName = TeamName;
		Result.EnglishDisplayName = EnglishName;
		Result.NationalityDisplayName = NationalityName;
		Result.PlayerFacingSerial = Serial;
		Result.Card.CardId = FName(CardId);
		Result.Card.DisplayName = PlayerName;
		Result.Card.BirthDate = BirthDate;
		Result.Card.HeightCm = HeightCm;
		Result.Card.WeightKg = WeightKg;
		Result.Card.Rarity = Rarity;
		Result.Card.PositionTypes = { EPlayerPositionType::Goalkeeper };
		Result.Card.Attributes = BaseAttributes;
		Result.Card.GoalkeeperAttributes = GoalkeeperAttributes;
		Result.Card.bIsGoalkeeper = true;
		return Result;
	}

	TArray<FFMCodexPrototypePlayerDefinition> BuildDefinitions()
	{
		const FText ArsenalName = NSLOCTEXT(
			"FMCodexPrototypeTeamContent", "TeamArsenal", "阿森纳");
		const FText ManchesterCityName = NSLOCTEXT(
			"FMCodexPrototypeTeamContent", "TeamManchesterCity", "曼彻斯特城");

		TArray<FFMCodexPrototypePlayerDefinition> Result;
		Result.Reserve(PrototypeCardsPerTeam * 2);

		Result.Add(MakeGoalkeeper(
			ArsenalId, ArsenalName,
			TEXT("Prototype.Arsenal.DavidRaya"),
			NSLOCTEXT("FMCodexPrototypeTeamContent", "PlayerDavidRaya", "大卫·拉亚"),
			NSLOCTEXT("FMCodexPrototypeTeamContent", "EnglishDavidRaya", "David Raya"),
			NSLOCTEXT("FMCodexPrototypeTeamContent", "NationalityDavidRaya", "西班牙"),
			TEXT("1995-09-15"), 183, 80, TEXT("001"),
			MakeAttributes(1, 2, 5, 2, 1, 1, 4, 3, 5, 1),
			MakeGoalkeeperAttributes(5, 5, 5, 4, 5, 5),
			ECardRarity::Continental));
		Result.Add(MakeOutfield(
			ArsenalId, ArsenalName,
			TEXT("Prototype.Arsenal.WilliamSaliba"),
			NSLOCTEXT("FMCodexPrototypeTeamContent", "PlayerWilliamSaliba", "威廉·萨利巴"),
			NSLOCTEXT("FMCodexPrototypeTeamContent", "EnglishWilliamSaliba", "William Saliba"),
			NSLOCTEXT("FMCodexPrototypeTeamContent", "NationalityWilliamSaliba", "法国"),
			TEXT("2001-03-24"), 192, 92, TEXT("002"),
			{ EPlayerPositionType::Defense },
			MakeAttributes(2, 3, 4, 2, 6, 6, 5, 6, 5, 2),
			ECardRarity::Continental, CrossSkillId));
		Result.Add(MakeOutfield(
			ArsenalId, ArsenalName,
			TEXT("Prototype.Arsenal.BukayoSaka"),
			NSLOCTEXT("FMCodexPrototypeTeamContent", "PlayerBukayoSaka", "布卡约·萨卡"),
			NSLOCTEXT("FMCodexPrototypeTeamContent", "EnglishBukayoSaka", "Bukayo Saka"),
			NSLOCTEXT("FMCodexPrototypeTeamContent", "NationalityBukayoSaka", "英格兰"),
			TEXT("2001-09-05"), 178, 72, TEXT("003"),
			{ EPlayerPositionType::Attack, EPlayerPositionType::Midfield },
			MakeAttributes(5, 6, 5, 6, 2, 2, 6, 3, 5, 4),
			ECardRarity::WorldClass, CutInsideShotSkillId));
		Result.Add(MakeOutfield(
			ArsenalId, ArsenalName,
			TEXT("Prototype.Arsenal.MartinOdegaard"),
			NSLOCTEXT("FMCodexPrototypeTeamContent", "PlayerMartinOdegaard", "马丁·厄德高"),
			NSLOCTEXT("FMCodexPrototypeTeamContent", "EnglishMartinOdegaard", "Martin Ødegaard"),
			NSLOCTEXT("FMCodexPrototypeTeamContent", "NationalityMartinOdegaard", "挪威"),
			TEXT("1998-12-17"), 178, 68, TEXT("004"),
			{ EPlayerPositionType::Midfield, EPlayerPositionType::Attack },
			MakeAttributes(4, 5, 6, 5, 3, 3, 4, 3, 5, 5),
			ECardRarity::Continental, PassControlSkillId));
		Result.Add(MakeOutfield(
			ArsenalId, ArsenalName,
			TEXT("Prototype.Arsenal.DeclanRice"),
			NSLOCTEXT("FMCodexPrototypeTeamContent", "PlayerDeclanRice", "德克兰·赖斯"),
			NSLOCTEXT("FMCodexPrototypeTeamContent", "EnglishDeclanRice", "Declan Rice"),
			NSLOCTEXT("FMCodexPrototypeTeamContent", "NationalityDeclanRice", "英格兰"),
			TEXT("1999-01-14"), 188, 83, TEXT("005"),
			{ EPlayerPositionType::Midfield, EPlayerPositionType::Defense },
			MakeAttributes(4, 4, 5, 4, 6, 6, 4, 6, 6, 5),
			ECardRarity::Continental, LongShotSkillId));
		Result.Add(MakeOutfield(
			ArsenalId, ArsenalName,
			TEXT("Prototype.Arsenal.GabrielMartinelli"),
			NSLOCTEXT("FMCodexPrototypeTeamContent", "PlayerGabrielMartinelli", "加布里埃尔·马丁内利"),
			NSLOCTEXT("FMCodexPrototypeTeamContent", "EnglishGabrielMartinelli", "Gabriel Martinelli"),
			NSLOCTEXT("FMCodexPrototypeTeamContent", "NationalityGabrielMartinelli", "巴西"),
			TEXT("2001-06-18"), 178, 74, TEXT("006"),
			{ EPlayerPositionType::Attack },
			MakeAttributes(5, 5, 3, 5, 2, 2, 6, 3, 5, 3),
			ECardRarity::National, CutInsideShotSkillId));
		Result.Add(MakeOutfield(
			ArsenalId, ArsenalName,
			TEXT("Prototype.Arsenal.GabrielMagalhaes"),
			NSLOCTEXT("FMCodexPrototypeTeamContent", "PlayerGabrielMagalhaes", "加布里埃尔·马加良斯"),
			NSLOCTEXT("FMCodexPrototypeTeamContent", "EnglishGabrielMagalhaes", "Gabriel Magalhães"),
			NSLOCTEXT("FMCodexPrototypeTeamContent", "NationalityGabrielMagalhaes", "巴西"),
			TEXT("1997-12-19"), 190, 78, TEXT("007"),
			{ EPlayerPositionType::Defense },
			MakeAttributes(2, 3, 3, 3, 6, 6, 5, 6, 5, 2),
			ECardRarity::Continental, NAME_None));
		Result.Add(MakeOutfield(
			ArsenalId, ArsenalName,
			TEXT("Prototype.Arsenal.MikelMerino"),
			NSLOCTEXT("FMCodexPrototypeTeamContent", "PlayerMikelMerino", "米克尔·梅里诺"),
			NSLOCTEXT("FMCodexPrototypeTeamContent", "EnglishMikelMerino", "Mikel Merino"),
			NSLOCTEXT("FMCodexPrototypeTeamContent", "NationalityMikelMerino", "西班牙"),
			TEXT("1996-06-22"), 189, 83, TEXT("008"),
			{ EPlayerPositionType::Midfield, EPlayerPositionType::Attack },
			MakeAttributes(4, 4, 5, 5, 5, 5, 3, 6, 5, 4),
			ECardRarity::National, PassControlSkillId));

		Result.Add(MakeGoalkeeper(
			ManchesterCityId, ManchesterCityName,
			TEXT("Prototype.ManchesterCity.GianluigiDonnarumma"),
			NSLOCTEXT("FMCodexPrototypeTeamContent", "PlayerGianluigiDonnarumma", "吉安路易吉·多纳鲁马"),
			NSLOCTEXT("FMCodexPrototypeTeamContent", "EnglishGianluigiDonnarumma", "Gianluigi Donnarumma"),
			NSLOCTEXT("FMCodexPrototypeTeamContent", "NationalityGianluigiDonnarumma", "意大利"),
			TEXT("1999-02-25"), 196, 89, TEXT("009"),
			MakeAttributes(1, 1, 3, 1, 1, 1, 3, 6, 5, 1),
			MakeGoalkeeperAttributes(6, 5, 6, 6, 5, 6),
			ECardRarity::Continental));
		Result.Add(MakeOutfield(
			ManchesterCityId, ManchesterCityName,
			TEXT("Prototype.ManchesterCity.ErlingHaaland"),
			NSLOCTEXT("FMCodexPrototypeTeamContent", "PlayerErlingHaaland", "埃尔林·哈兰德"),
			NSLOCTEXT("FMCodexPrototypeTeamContent", "EnglishErlingHaaland", "Erling Haaland"),
			NSLOCTEXT("FMCodexPrototypeTeamContent", "NationalityErlingHaaland", "挪威"),
			TEXT("2000-07-21"), 195, 87, TEXT("010"),
			{ EPlayerPositionType::Attack },
			MakeAttributes(6, 4, 3, 6, 1, 2, 6, 6, 5, 5),
			ECardRarity::WorldClass, LongShotSkillId));
		Result.Add(MakeOutfield(
			ManchesterCityId, ManchesterCityName,
			TEXT("Prototype.ManchesterCity.PhilFoden"),
			NSLOCTEXT("FMCodexPrototypeTeamContent", "PlayerPhilFoden", "菲尔·福登"),
			NSLOCTEXT("FMCodexPrototypeTeamContent", "EnglishPhilFoden", "Phil Foden"),
			NSLOCTEXT("FMCodexPrototypeTeamContent", "NationalityPhilFoden", "英格兰"),
			TEXT("2000-05-28"), 171, 63, TEXT("011"),
			{ EPlayerPositionType::Attack, EPlayerPositionType::Midfield },
			MakeAttributes(5, 6, 5, 5, 2, 2, 5, 3, 5, 5),
			ECardRarity::Continental, CutInsideShotSkillId));
		Result.Add(MakeOutfield(
			ManchesterCityId, ManchesterCityName,
			TEXT("Prototype.ManchesterCity.Rodri"),
			NSLOCTEXT("FMCodexPrototypeTeamContent", "PlayerRodri", "罗德里"),
			NSLOCTEXT("FMCodexPrototypeTeamContent", "EnglishRodri", "Rodri"),
			NSLOCTEXT("FMCodexPrototypeTeamContent", "NationalityRodri", "西班牙"),
			TEXT("1996-06-22"), 190, 82, TEXT("012"),
			{ EPlayerPositionType::Midfield, EPlayerPositionType::Defense },
			MakeAttributes(4, 5, 6, 4, 6, 6, 3, 6, 6, 5),
			ECardRarity::Continental, PassControlSkillId));
		Result.Add(MakeOutfield(
			ManchesterCityId, ManchesterCityName,
			TEXT("Prototype.ManchesterCity.RubenDias"),
			NSLOCTEXT("FMCodexPrototypeTeamContent", "PlayerRubenDias", "鲁本·迪亚斯"),
			NSLOCTEXT("FMCodexPrototypeTeamContent", "EnglishRubenDias", "Rúben Dias"),
			NSLOCTEXT("FMCodexPrototypeTeamContent", "NationalityRubenDias", "葡萄牙"),
			TEXT("1997-05-14"), 187, 83, TEXT("013"),
			{ EPlayerPositionType::Defense },
			MakeAttributes(2, 3, 4, 2, 6, 6, 4, 6, 5, 2),
			ECardRarity::Continental, CrossSkillId));
		Result.Add(MakeOutfield(
			ManchesterCityId, ManchesterCityName,
			TEXT("Prototype.ManchesterCity.JoskoGvardiol"),
			NSLOCTEXT("FMCodexPrototypeTeamContent", "PlayerJoskoGvardiol", "约什科·格瓦迪奥尔"),
			NSLOCTEXT("FMCodexPrototypeTeamContent", "EnglishJoskoGvardiol", "Joško Gvardiol"),
			NSLOCTEXT("FMCodexPrototypeTeamContent", "NationalityJoskoGvardiol", "克罗地亚"),
			TEXT("2002-01-23"), 185, 79, TEXT("014"),
			{ EPlayerPositionType::Defense },
			MakeAttributes(3, 4, 5, 4, 5, 5, 5, 5, 5, 3),
			ECardRarity::Continental, CrossSkillId));
		Result.Add(MakeOutfield(
			ManchesterCityId, ManchesterCityName,
			TEXT("Prototype.ManchesterCity.BernardoSilva"),
			NSLOCTEXT("FMCodexPrototypeTeamContent", "PlayerBernardoSilva", "贝尔纳多·席尔瓦"),
			NSLOCTEXT("FMCodexPrototypeTeamContent", "EnglishBernardoSilva", "Bernardo Silva"),
			NSLOCTEXT("FMCodexPrototypeTeamContent", "NationalityBernardoSilva", "葡萄牙"),
			TEXT("1994-08-10"), 173, 64, TEXT("015"),
			{ EPlayerPositionType::Midfield, EPlayerPositionType::Attack },
			MakeAttributes(4, 6, 6, 5, 3, 3, 4, 2, 6, 4),
			ECardRarity::Continental, ThroughBallSkillId));
		Result.Add(MakeOutfield(
			ManchesterCityId, ManchesterCityName,
			TEXT("Prototype.ManchesterCity.JeremyDoku"),
			NSLOCTEXT("FMCodexPrototypeTeamContent", "PlayerJeremyDoku", "杰里米·多库"),
			NSLOCTEXT("FMCodexPrototypeTeamContent", "EnglishJeremyDoku", "Jérémy Doku"),
			NSLOCTEXT("FMCodexPrototypeTeamContent", "NationalityJeremyDoku", "比利时"),
			TEXT("2002-05-27"), 171, 66, TEXT("016"),
			{ EPlayerPositionType::Attack },
			MakeAttributes(4, 6, 4, 5, 2, 2, 6, 3, 4, 3),
			ECardRarity::National, CutInsideShotSkillId));

		return Result;
	}
}

const TArray<FFMCodexPrototypePlayerDefinition>&
FFMCodexPrototypeTeamContent::GetDefinitions()
{
	static const TArray<FFMCodexPrototypePlayerDefinition> Definitions =
		FMCodexPrototypeTeamContent::BuildDefinitions();
	return Definitions;
}

const FFMCodexPrototypePlayerDefinition*
FFMCodexPrototypeTeamContent::Find(const FName CardId)
{
	return GetDefinitions().FindByPredicate(
		[CardId](const FFMCodexPrototypePlayerDefinition& Definition)
		{
			return Definition.Card.CardId == CardId;
		});
}

bool FFMCodexPrototypeTeamContent::IsPrototypeCard(const FName CardId)
{
	return Find(CardId) != nullptr;
}

FName FFMCodexPrototypeTeamContent::TeamIdForCard(const FName CardId)
{
	const FFMCodexPrototypePlayerDefinition* Definition = Find(CardId);
	return Definition == nullptr ? NAME_None : Definition->TeamId;
}

FText FFMCodexPrototypeTeamContent::PlayerDisplayName(const FName CardId)
{
	const FFMCodexPrototypePlayerDefinition* Definition = Find(CardId);
	return Definition == nullptr ? FText::GetEmpty() : Definition->Card.DisplayName;
}

FText FFMCodexPrototypeTeamContent::TeamDisplayName(const FName CardId)
{
	const FFMCodexPrototypePlayerDefinition* Definition = Find(CardId);
	return Definition == nullptr ? FText::GetEmpty() : Definition->TeamDisplayName;
}

FName FFMCodexPrototypeTeamContent::ArsenalTeamId()
{
	return FMCodexPrototypeTeamContent::ArsenalId;
}

FName FFMCodexPrototypeTeamContent::ManchesterCityTeamId()
{
	return FMCodexPrototypeTeamContent::ManchesterCityId;
}

int32 FFMCodexPrototypeTeamContent::CardsPerTeam()
{
	return FMCodexPrototypeTeamContent::PrototypeCardsPerTeam;
}

void FFMCodexPrototypeTeamContent::IntegrateIntoDemoDeck(
	const EInitialTurnOrderPlayer Side,
	TArray<FPlayerCardData>& Deck)
{
	if (Deck.Num() != 20)
	{
		return;
	}

	const FName ExpectedTeam = Side == EInitialTurnOrderPlayer::PlayerA
		? ArsenalTeamId() : Side == EInitialTurnOrderPlayer::PlayerB
			? ManchesterCityTeamId() : NAME_None;
	if (ExpectedTeam.IsNone())
	{
		return;
	}

	const TArray<FName> OrderedCardIds = ExpectedTeam == ArsenalTeamId()
		? TArray<FName>{
			TEXT("Prototype.Arsenal.DavidRaya"),
			TEXT("Prototype.Arsenal.WilliamSaliba"),
			TEXT("Prototype.Arsenal.BukayoSaka"),
			TEXT("Prototype.Arsenal.MartinOdegaard"),
			TEXT("Prototype.Arsenal.DeclanRice"),
			TEXT("Prototype.Arsenal.GabrielMartinelli"),
			TEXT("Prototype.Arsenal.GabrielMagalhaes"),
			TEXT("Prototype.Arsenal.MikelMerino") }
		: TArray<FName>{
			TEXT("Prototype.ManchesterCity.GianluigiDonnarumma"),
			TEXT("Prototype.ManchesterCity.ErlingHaaland"),
			TEXT("Prototype.ManchesterCity.PhilFoden"),
			TEXT("Prototype.ManchesterCity.Rodri"),
			TEXT("Prototype.ManchesterCity.RubenDias"),
			TEXT("Prototype.ManchesterCity.JoskoGvardiol"),
			TEXT("Prototype.ManchesterCity.BernardoSilva"),
			TEXT("Prototype.ManchesterCity.JeremyDoku") };
	// Keep the five previously shipped Prototype cards at their established
	// rack indices. The three newly approved players replace only the old
	// Demo.*.Outfield.01-03 presentation fixtures.
	const TArray<int32> DeckIndices = ExpectedTeam == ArsenalTeamId()
		? TArray<int32>{ 19, 10, 12, 13, 11, 0, 1, 2 }
		: TArray<int32>{ 19, 11, 12, 13, 10, 0, 1, 2 };

	TArray<const FFMCodexPrototypePlayerDefinition*> OrderedDefinitions;
	OrderedDefinitions.Reserve(OrderedCardIds.Num());
	for (const FName CardId : OrderedCardIds)
	{
		const FFMCodexPrototypePlayerDefinition* Definition = Find(CardId);
		if (Definition == nullptr || Definition->TeamId != ExpectedTeam)
		{
			return;
		}
		OrderedDefinitions.Add(Definition);
	}

	for (int32 Index = 0; Index < OrderedDefinitions.Num(); ++Index)
	{
		Deck[DeckIndices[Index]] = OrderedDefinitions[Index]->Card;
	}
}
