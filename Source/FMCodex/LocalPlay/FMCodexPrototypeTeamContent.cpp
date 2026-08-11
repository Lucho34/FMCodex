#include "FMCodexPrototypeTeamContent.h"

namespace FMCodexPrototypeTeamContent
{
	const FName ArsenalId(TEXT("Prototype.Team.Arsenal"));
	const FName ManchesterCityId(TEXT("Prototype.Team.ManchesterCity"));
	constexpr int32 PilotCardsPerTeam = 5;

	const FName LongShotSkillId(TEXT("Demo.Skill.LongShot"));
	const FName CutInsideShotSkillId(TEXT("Demo.Skill.CutInsideShot"));
	const FName PassControlSkillId(TEXT("Demo.Skill.PassControl"));
	const FName CrossSkillId(TEXT("Demo.Skill.Cross"));

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
		const TArray<EPlayerPositionType>& Positions,
		const FPlayerAttributes& Attributes,
		const ECardRarity Rarity,
		const FName SkillId)
	{
		FFMCodexPrototypePlayerDefinition Result;
		Result.TeamId = TeamId;
		Result.TeamDisplayName = TeamName;
		Result.Card.CardId = FName(CardId);
		Result.Card.DisplayName = PlayerName;
		Result.Card.Rarity = Rarity;
		Result.Card.PositionTypes = Positions;
		Result.Card.Attributes = Attributes;
		Result.Card.AttackSkillIds = { SkillId };
		return Result;
	}

	FFMCodexPrototypePlayerDefinition MakeGoalkeeper(
		const FName TeamId,
		const FText& TeamName,
		const TCHAR* CardId,
		const FText& PlayerName,
		const FPlayerAttributes& BaseAttributes,
		const FGoalkeeperAttributes& GoalkeeperAttributes,
		const ECardRarity Rarity)
	{
		FFMCodexPrototypePlayerDefinition Result;
		Result.TeamId = TeamId;
		Result.TeamDisplayName = TeamName;
		Result.Card.CardId = FName(CardId);
		Result.Card.DisplayName = PlayerName;
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
		Result.Reserve(PilotCardsPerTeam * 2);

		Result.Add(MakeOutfield(
			ArsenalId, ArsenalName,
			TEXT("Prototype.Arsenal.WilliamSaliba"),
			NSLOCTEXT("FMCodexPrototypeTeamContent", "PlayerWilliamSaliba", "威廉·萨利巴"),
			{ EPlayerPositionType::Defense },
			MakeAttributes(2, 3, 4, 2, 6, 6, 5, 6, 5, 2),
			ECardRarity::Continental, CrossSkillId));
		Result.Add(MakeOutfield(
			ArsenalId, ArsenalName,
			TEXT("Prototype.Arsenal.DeclanRice"),
			NSLOCTEXT("FMCodexPrototypeTeamContent", "PlayerDeclanRice", "德克兰·赖斯"),
			{ EPlayerPositionType::Midfield, EPlayerPositionType::Defense },
			MakeAttributes(4, 4, 5, 4, 6, 6, 4, 6, 6, 5),
			ECardRarity::Continental, LongShotSkillId));
		Result.Add(MakeOutfield(
			ArsenalId, ArsenalName,
			TEXT("Prototype.Arsenal.BukayoSaka"),
			NSLOCTEXT("FMCodexPrototypeTeamContent", "PlayerBukayoSaka", "布卡约·萨卡"),
			{ EPlayerPositionType::Attack, EPlayerPositionType::Midfield },
			MakeAttributes(5, 6, 5, 6, 2, 2, 6, 3, 5, 4),
			ECardRarity::WorldClass, CutInsideShotSkillId));
		Result.Add(MakeOutfield(
			ArsenalId, ArsenalName,
			TEXT("Prototype.Arsenal.MartinOdegaard"),
			NSLOCTEXT("FMCodexPrototypeTeamContent", "PlayerMartinOdegaard", "马丁·厄德高"),
			{ EPlayerPositionType::Midfield, EPlayerPositionType::Attack },
			MakeAttributes(4, 5, 6, 5, 3, 3, 4, 3, 5, 5),
			ECardRarity::Continental, PassControlSkillId));
		Result.Add(MakeGoalkeeper(
			ArsenalId, ArsenalName,
			TEXT("Prototype.Arsenal.DavidRaya"),
			NSLOCTEXT("FMCodexPrototypeTeamContent", "PlayerDavidRaya", "大卫·拉亚"),
			MakeAttributes(1, 2, 5, 2, 1, 1, 4, 3, 5, 1),
			MakeGoalkeeperAttributes(5, 5, 5, 4, 5, 5),
			ECardRarity::Continental));

		Result.Add(MakeOutfield(
			ManchesterCityId, ManchesterCityName,
			TEXT("Prototype.ManchesterCity.RubenDias"),
			NSLOCTEXT("FMCodexPrototypeTeamContent", "PlayerRubenDias", "鲁本·迪亚斯"),
			{ EPlayerPositionType::Defense },
			MakeAttributes(2, 3, 4, 2, 6, 6, 4, 6, 5, 2),
			ECardRarity::Continental, CrossSkillId));
		Result.Add(MakeOutfield(
			ManchesterCityId, ManchesterCityName,
			TEXT("Prototype.ManchesterCity.ErlingHaaland"),
			NSLOCTEXT("FMCodexPrototypeTeamContent", "PlayerErlingHaaland", "埃尔林·哈兰德"),
			{ EPlayerPositionType::Attack },
			MakeAttributes(6, 4, 3, 6, 1, 2, 6, 6, 5, 5),
			ECardRarity::WorldClass, LongShotSkillId));
		Result.Add(MakeOutfield(
			ManchesterCityId, ManchesterCityName,
			TEXT("Prototype.ManchesterCity.PhilFoden"),
			NSLOCTEXT("FMCodexPrototypeTeamContent", "PlayerPhilFoden", "菲尔·福登"),
			{ EPlayerPositionType::Attack, EPlayerPositionType::Midfield },
			MakeAttributes(5, 6, 5, 5, 2, 2, 5, 3, 5, 5),
			ECardRarity::Continental, CutInsideShotSkillId));
		Result.Add(MakeOutfield(
			ManchesterCityId, ManchesterCityName,
			TEXT("Prototype.ManchesterCity.Rodri"),
			NSLOCTEXT("FMCodexPrototypeTeamContent", "PlayerRodri", "罗德里"),
			{ EPlayerPositionType::Midfield, EPlayerPositionType::Defense },
			MakeAttributes(4, 5, 6, 4, 6, 6, 3, 6, 6, 5),
			ECardRarity::Continental, PassControlSkillId));
		Result.Add(MakeGoalkeeper(
			ManchesterCityId, ManchesterCityName,
			TEXT("Prototype.ManchesterCity.GianluigiDonnarumma"),
			NSLOCTEXT("FMCodexPrototypeTeamContent", "PlayerGianluigiDonnarumma", "吉安路易吉·多纳鲁马"),
			MakeAttributes(1, 1, 3, 1, 1, 1, 3, 6, 5, 1),
			MakeGoalkeeperAttributes(6, 5, 6, 6, 5, 6),
			ECardRarity::Continental));

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
	return FMCodexPrototypeTeamContent::PilotCardsPerTeam;
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
			TEXT("Prototype.Arsenal.WilliamSaliba"),
			TEXT("Prototype.Arsenal.DeclanRice"),
			TEXT("Prototype.Arsenal.BukayoSaka"),
			TEXT("Prototype.Arsenal.MartinOdegaard"),
			TEXT("Prototype.Arsenal.DavidRaya") }
		: TArray<FName>{
			TEXT("Prototype.ManchesterCity.RubenDias"),
			TEXT("Prototype.ManchesterCity.ErlingHaaland"),
			TEXT("Prototype.ManchesterCity.PhilFoden"),
			TEXT("Prototype.ManchesterCity.Rodri"),
			TEXT("Prototype.ManchesterCity.GianluigiDonnarumma") };

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

	for (int32 Index = 0; Index < CardsPerTeam() - 1; ++Index)
	{
		// Preserve Demo.*.Outfield.01-05: existing public-flow fixtures use
		// those stable IDs to prove each authoritative skill family.
		Deck[10 + Index] = OrderedDefinitions[Index]->Card;
	}
	Deck.Last() = OrderedDefinitions.Last()->Card;

}
