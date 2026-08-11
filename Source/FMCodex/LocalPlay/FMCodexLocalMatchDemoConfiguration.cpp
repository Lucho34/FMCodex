#include "FMCodexLocalMatchDemoConfiguration.h"

#include "FMCodexPrototypeTeamContent.h"

namespace FMCodexLocalMatchDemoConfiguration
{
	constexpr const TCHAR* LongShotSkillId = TEXT("Demo.Skill.LongShot");
	constexpr const TCHAR* CutInsideShotSkillId =
		TEXT("Demo.Skill.CutInsideShot");
	constexpr const TCHAR* PassControlSkillId = TEXT("Demo.Skill.PassControl");
	constexpr const TCHAR* CrossSkillId = TEXT("Demo.Skill.Cross");
	constexpr const TCHAR* ThroughBallSkillId = TEXT("Demo.Skill.ThroughBall");

	FName SkillIdForOutfieldCard(const int32 Index)
	{
		switch ((Index - 1) % 5)
		{
		case 0: return FName(CrossSkillId);
		case 1: return FName(LongShotSkillId);
		case 2: return FName(CutInsideShotSkillId);
		case 3: return FName(PassControlSkillId);
		default: return FName(ThroughBallSkillId);
		}
	}

	FSkillRuleSnapshot MakeSkillRule(
		const TCHAR* SkillId,
		const ESkillRuleType SkillType)
	{
		FSkillRuleSnapshot Rule;
		Rule.SkillId = FName(SkillId);
		Rule.SkillType = SkillType;
		Rule.MinTriggerActionPoint = 2;
		Rule.MaxTriggerActionPoint = 8;
		return Rule;
	}

	FPlayerCardData MakeOutfieldCard(
		const EInitialTurnOrderPlayer Side,
		const int32 Index)
	{
		FPlayerCardData Card;
		Card.CardId = FName(*FString::Printf(
			TEXT("Demo.%s.Outfield.%02d"),
			Side == EInitialTurnOrderPlayer::PlayerA ? TEXT("A") : TEXT("B"),
			Index));
		Card.DisplayName = FText::FromName(Card.CardId);
		Card.Rarity = ECardRarity::Common;
		Card.PositionTypes = {
			EPlayerPositionType::Attack,
			EPlayerPositionType::Midfield,
			EPlayerPositionType::Defense
		};
		Card.AttackSkillIds = { SkillIdForOutfieldCard(Index) };
		return Card;
	}

	FPlayerCardData MakeGoalkeeper(const EInitialTurnOrderPlayer Side)
	{
		FPlayerCardData Card;
		Card.CardId = FName(*FString::Printf(
			TEXT("Demo.%s.Goalkeeper"),
			Side == EInitialTurnOrderPlayer::PlayerA ? TEXT("A") : TEXT("B")));
		Card.DisplayName = FText::FromName(Card.CardId);
		Card.Rarity = ECardRarity::Common;
		Card.PositionTypes = { EPlayerPositionType::Goalkeeper };
		Card.bIsGoalkeeper = true;
		return Card;
	}

	TArray<FPlayerCardData> MakeDeck(const EInitialTurnOrderPlayer Side)
	{
		TArray<FPlayerCardData> Deck;
		Deck.Reserve(20);
		for (int32 Index = 0; Index < 19; ++Index)
		{
			Deck.Add(MakeOutfieldCard(Side, Index + 1));
		}
		Deck.Add(MakeGoalkeeper(Side));
		FFMCodexPrototypeTeamContent::IntegrateIntoDemoDeck(Side, Deck);
		return Deck;
	}
}

FFMCodexLocalMatchDemoConfiguration
FFMCodexLocalMatchDemoConfigurationFactory::Create()
{
	using namespace FMCodexLocalMatchDemoConfiguration;

	FFMCodexLocalMatchDemoConfiguration Result;
	Result.OpeningInput.OpeningInput.PlayerADeck =
		MakeDeck(EInitialTurnOrderPlayer::PlayerA);
	Result.OpeningInput.OpeningInput.PlayerBDeck =
		MakeDeck(EInitialTurnOrderPlayer::PlayerB);
	Result.OpeningInput.OpeningInput.PlayerAAttackCountD6Roll = 1;
	Result.OpeningInput.OpeningInput.PlayerBAttackCountD6Roll = 1;
	Result.OpeningInput.OpeningInput.PlayerATieBreakerRoll = 2;
	Result.OpeningInput.OpeningInput.PlayerBTieBreakerRoll = 6;

	for (int32 Index = 0; Index < 5; ++Index)
	{
		FMatchPlayDeploymentSlotDefinition NearA;
		NearA.SlotId = FName(*FString::Printf(
			TEXT("Demo.Slot.NearA.%02d"), Index + 1));
		NearA.NeutralSide = EMatchPlayNeutralSlotSide::NearPlayerA;
		Result.OpeningInput.DeploymentSlotCatalog.Slots.Add(NearA);

		FMatchPlayDeploymentSlotDefinition NearB;
		NearB.SlotId = FName(*FString::Printf(
			TEXT("Demo.Slot.NearB.%02d"), Index + 1));
		NearB.NeutralSide = EMatchPlayNeutralSlotSide::NearPlayerB;
		Result.OpeningInput.DeploymentSlotCatalog.Slots.Add(NearB);
	}

	Result.SkillRuleSet.SkillRules = {
		MakeSkillRule(LongShotSkillId, ESkillRuleType::LongShot),
		MakeSkillRule(CutInsideShotSkillId, ESkillRuleType::CutInsideShot),
		MakeSkillRule(PassControlSkillId, ESkillRuleType::PassControl),
		MakeSkillRule(CrossSkillId, ESkillRuleType::Cross),
		MakeSkillRule(ThroughBallSkillId, ESkillRuleType::ThroughBall)
	};
	return Result;
}
