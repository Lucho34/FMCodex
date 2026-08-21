#include "FMCodexPrototypeTeamContent.h"

#include "Dom/JsonObject.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

DEFINE_LOG_CATEGORY_STATIC(LogFMCodexCanonicalPlayerContent, Log, All);

namespace FMCodexPrototypeTeamContent
{
	const FName ArsenalId(TEXT("Prototype.Team.Arsenal"));
	const FName ManchesterCityId(TEXT("Prototype.Team.ManchesterCity"));
	constexpr int32 CanonicalPlayersPerTeam = 20;
	constexpr int32 CanonicalPlayerCount = CanonicalPlayersPerTeam * 2;
	constexpr int32 RuntimeSchemaVersion = 2;

	struct FCatalog
	{
		FString BalanceContentVersion;
		TArray<FFMCodexPrototypePlayerDefinition> Definitions;
		TArray<FString> Errors;
	};

	bool TryReadString(
		const TSharedPtr<FJsonObject>& Object,
		const TCHAR* Field,
		FString& OutValue,
		TArray<FString>& OutErrors,
		const FString& Context,
		const bool bAllowEmpty = false)
	{
		if (!Object.IsValid() || !Object->TryGetStringField(Field, OutValue)
			|| (!bAllowEmpty && OutValue.IsEmpty()))
		{
			OutErrors.Add(FString::Printf(
				TEXT("%s: required string field '%s' is missing or empty"),
				*Context, Field));
			return false;
		}
		return true;
	}

	bool TryReadInt(
		const TSharedPtr<FJsonObject>& Object,
		const TCHAR* Field,
		int32& OutValue,
		TArray<FString>& OutErrors,
		const FString& Context)
	{
		double Number = 0.0;
		if (!Object.IsValid() || !Object->TryGetNumberField(Field, Number)
			|| !FMath::IsFinite(Number)
			|| !FMath::IsNearlyEqual(Number, FMath::RoundToDouble(Number)))
		{
			OutErrors.Add(FString::Printf(
				TEXT("%s: integer field '%s' is missing or invalid"),
				*Context, Field));
			return false;
		}
		OutValue = FMath::RoundToInt(Number);
		return true;
	}

	bool IsInAttributeRange(const int32 Value)
	{
		return Value >= 1 && Value <= 6;
	}

	bool AttributesInRange(const FPlayerAttributes& Attributes)
	{
		return IsInAttributeRange(Attributes.Shooting)
			&& IsInAttributeRange(Attributes.Dribbling)
			&& IsInAttributeRange(Attributes.Passing)
			&& IsInAttributeRange(Attributes.OffBall)
			&& IsInAttributeRange(Attributes.Marking)
			&& IsInAttributeRange(Attributes.Tackling)
			&& IsInAttributeRange(Attributes.Speed)
			&& IsInAttributeRange(Attributes.Strength)
			&& IsInAttributeRange(Attributes.Stamina)
			&& IsInAttributeRange(Attributes.LongShot);
	}

	bool GoalkeeperAttributesInRange(
		const FGoalkeeperAttributes& Attributes)
	{
		return IsInAttributeRange(Attributes.Handling)
			&& IsInAttributeRange(Attributes.Positioning)
			&& IsInAttributeRange(Attributes.Reflex)
			&& IsInAttributeRange(Attributes.Aerial)
			&& IsInAttributeRange(Attributes.Anticipation)
			&& IsInAttributeRange(Attributes.OneOnOne);
	}

	bool TryParseRarity(const FString& Value, ECardRarity& OutRarity)
	{
		if (Value == TEXT("Common"))
		{
			OutRarity = ECardRarity::Common;
			return true;
		}
		if (Value == TEXT("Regional"))
		{
			OutRarity = ECardRarity::Regional;
			return true;
		}
		if (Value == TEXT("National"))
		{
			OutRarity = ECardRarity::National;
			return true;
		}
		if (Value == TEXT("Continental"))
		{
			OutRarity = ECardRarity::Continental;
			return true;
		}
		if (Value == TEXT("WorldClass"))
		{
			OutRarity = ECardRarity::WorldClass;
			return true;
		}
		return false;
	}

	bool TryParsePosition(
		const FString& Value,
		TArray<EPlayerPositionType>& OutPositions,
		bool& bOutGoalkeeper)
	{
		bOutGoalkeeper = false;
		if (Value == TEXT("A"))
		{
			OutPositions = { EPlayerPositionType::Attack };
			return true;
		}
		if (Value == TEXT("M"))
		{
			OutPositions = { EPlayerPositionType::Midfield };
			return true;
		}
		if (Value == TEXT("D"))
		{
			OutPositions = { EPlayerPositionType::Defense };
			return true;
		}
		if (Value == TEXT("A/M"))
		{
			OutPositions = {
				EPlayerPositionType::Attack,
				EPlayerPositionType::Midfield
			};
			return true;
		}
		if (Value == TEXT("M/D"))
		{
			OutPositions = {
				EPlayerPositionType::Midfield,
				EPlayerPositionType::Defense
			};
			return true;
		}
		if (Value == TEXT("GK"))
		{
			OutPositions = { EPlayerPositionType::Goalkeeper };
			bOutGoalkeeper = true;
			return true;
		}
		return false;
	}

	bool TryParseSkillType(
		const FString& Value,
		ESkillRuleType& OutType)
	{
		if (Value == TEXT("LongShot"))
		{
			OutType = ESkillRuleType::LongShot;
			return true;
		}
		if (Value == TEXT("CutInsideShot"))
		{
			OutType = ESkillRuleType::CutInsideShot;
			return true;
		}
		if (Value == TEXT("PassControl"))
		{
			OutType = ESkillRuleType::PassControl;
			return true;
		}
		if (Value == TEXT("Cross"))
		{
			OutType = ESkillRuleType::Cross;
			return true;
		}
		if (Value == TEXT("ThroughBall"))
		{
			OutType = ESkillRuleType::ThroughBall;
			return true;
		}
		return false;
	}

	FName MakeRuleId(
		const FString& SkillId,
		const int32 MinTacticalPoint,
		const int32 MaxTacticalPoint)
	{
		return FName(*FString::Printf(
			TEXT("Canonical.Skill.%s.%d.%d"),
			*SkillId, MinTacticalPoint, MaxTacticalPoint));
	}

	bool TryReadOutfieldAttributes(
		const TSharedPtr<FJsonObject>& Object,
		FPlayerAttributes& OutAttributes,
		TArray<FString>& OutErrors,
		const FString& Context)
	{
		if (!Object.IsValid() || Object->Values.Num() != 10)
		{
			OutErrors.Add(Context + TEXT(": exactly 10 outfield attributes are required"));
			return false;
		}
		bool bSuccess = true;
		bSuccess &= TryReadInt(Object, TEXT("SHO"), OutAttributes.Shooting, OutErrors, Context);
		bSuccess &= TryReadInt(Object, TEXT("DRI"), OutAttributes.Dribbling, OutErrors, Context);
		bSuccess &= TryReadInt(Object, TEXT("PAS"), OutAttributes.Passing, OutErrors, Context);
		bSuccess &= TryReadInt(Object, TEXT("OFF"), OutAttributes.OffBall, OutErrors, Context);
		bSuccess &= TryReadInt(Object, TEXT("MRK"), OutAttributes.Marking, OutErrors, Context);
		bSuccess &= TryReadInt(Object, TEXT("TKL"), OutAttributes.Tackling, OutErrors, Context);
		bSuccess &= TryReadInt(Object, TEXT("SPD"), OutAttributes.Speed, OutErrors, Context);
		bSuccess &= TryReadInt(Object, TEXT("STR"), OutAttributes.Strength, OutErrors, Context);
		bSuccess &= TryReadInt(Object, TEXT("STA"), OutAttributes.Stamina, OutErrors, Context);
		bSuccess &= TryReadInt(Object, TEXT("LS"), OutAttributes.LongShot, OutErrors, Context);
		if (bSuccess && !AttributesInRange(OutAttributes))
		{
			OutErrors.Add(Context + TEXT(": every outfield attribute must be 1-6"));
			bSuccess = false;
		}
		return bSuccess;
	}

	bool TryReadGoalkeeperAttributes(
		const TSharedPtr<FJsonObject>& Object,
		FGoalkeeperAttributes& OutAttributes,
		TArray<FString>& OutErrors,
		const FString& Context)
	{
		if (!Object.IsValid() || Object->Values.Num() != 6)
		{
			OutErrors.Add(Context + TEXT(": exactly 6 goalkeeper attributes are required"));
			return false;
		}
		bool bSuccess = true;
		bSuccess &= TryReadInt(Object, TEXT("HAN"), OutAttributes.Handling, OutErrors, Context);
		bSuccess &= TryReadInt(Object, TEXT("POS_GK"), OutAttributes.Positioning, OutErrors, Context);
		bSuccess &= TryReadInt(Object, TEXT("REF"), OutAttributes.Reflex, OutErrors, Context);
		bSuccess &= TryReadInt(Object, TEXT("AER"), OutAttributes.Aerial, OutErrors, Context);
		bSuccess &= TryReadInt(Object, TEXT("ANT"), OutAttributes.Anticipation, OutErrors, Context);
		bSuccess &= TryReadInt(Object, TEXT("1V1"), OutAttributes.OneOnOne, OutErrors, Context);
		if (bSuccess && !GoalkeeperAttributesInRange(OutAttributes))
		{
			OutErrors.Add(Context + TEXT(": every goalkeeper attribute must be 1-6"));
			bSuccess = false;
		}
		return bSuccess;
	}

	FName TeamIdFromSource(const FString& Team)
	{
		if (Team == TEXT("Arsenal"))
		{
			return ArsenalId;
		}
		if (Team == TEXT("Manchester City"))
		{
			return ManchesterCityId;
		}
		return NAME_None;
	}

	FText TeamDisplayName(const FName TeamId)
	{
		if (TeamId == ArsenalId)
		{
			return NSLOCTEXT(
				"FMCodexPrototypeTeamContent", "TeamArsenal", "阿森纳");
		}
		if (TeamId == ManchesterCityId)
		{
			return NSLOCTEXT(
				"FMCodexPrototypeTeamContent", "TeamManchesterCity", "曼彻斯特城");
		}
		return FText::GetEmpty();
	}

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

	bool ValidateDefinitions(
		const FString& BalanceContentVersion,
		const TArray<FFMCodexPrototypePlayerDefinition>& Definitions,
		TArray<FString>& OutErrors)
	{
		if (BalanceContentVersion.IsEmpty())
		{
			OutErrors.Add(TEXT("BalanceContentVersion is required"));
		}
		if (Definitions.Num() != CanonicalPlayerCount)
		{
			OutErrors.Add(FString::Printf(
				TEXT("Expected exactly %d players, found %d"),
				CanonicalPlayerCount, Definitions.Num()));
		}

		TSet<FName> PlayerKeys;
		TSet<int32> DisplaySerials;
		TMap<FName, int32> TeamCounts;
		TMap<FName, int32> GoalkeeperCounts;
		TMap<FName, TSet<int32>> RosterSlots;
		for (const FFMCodexPrototypePlayerDefinition& Definition : Definitions)
		{
			const FString Context = Definition.PlayerKey.IsNone()
				? TEXT("Unknown player") : Definition.PlayerKey.ToString();
			if (!IsCodeSafeId(Definition.PlayerKey)
				|| Definition.PlayerKey != Definition.Card.CardId)
			{
				OutErrors.Add(Context + TEXT(": invalid or mismatched PlayerKey/CardId"));
			}
			if (PlayerKeys.Contains(Definition.PlayerKey))
			{
				OutErrors.Add(Context + TEXT(": duplicate PlayerKey"));
			}
			PlayerKeys.Add(Definition.PlayerKey);
			if (Definition.TeamId != ArsenalId
				&& Definition.TeamId != ManchesterCityId)
			{
				OutErrors.Add(Context + TEXT(": invalid TeamId"));
			}
			TeamCounts.FindOrAdd(Definition.TeamId)++;
			GoalkeeperCounts.FindOrAdd(Definition.TeamId) +=
				Definition.Card.bIsGoalkeeper ? 1 : 0;
			if (Definition.RosterSlot < 1
				|| Definition.RosterSlot > CanonicalPlayersPerTeam
				|| RosterSlots.FindOrAdd(Definition.TeamId).Contains(
					Definition.RosterSlot))
			{
				OutErrors.Add(Context + TEXT(": invalid or duplicate RosterSlot"));
			}
			RosterSlots.FindOrAdd(Definition.TeamId).Add(Definition.RosterSlot);
			if (Definition.DisplaySerial <= 0
				|| DisplaySerials.Contains(Definition.DisplaySerial))
			{
				OutErrors.Add(Context + TEXT(": invalid or duplicate DisplaySerial"));
			}
			DisplaySerials.Add(Definition.DisplaySerial);
			if (Definition.PlayerFacingSerial
				!= FString::Printf(TEXT("%03d"), Definition.DisplaySerial))
			{
				OutErrors.Add(Context + TEXT(": Full Card serial format is not derived from DisplaySerial"));
			}
			if (Definition.Card.DisplayName.IsEmpty()
				|| Definition.CanonicalChineseDisplayName.IsEmpty()
				|| Definition.PreferredDisplayName.IsEmpty()
				|| Definition.EnglishDisplayName.IsEmpty()
				|| Definition.TeamDisplayName.IsEmpty())
			{
				OutErrors.Add(Context + TEXT(": required identity/display field is empty"));
			}
			if (!Definition.Card.DisplayName.EqualTo(
				Definition.CanonicalChineseDisplayName))
			{
				OutErrors.Add(Context + TEXT(": canonical Chinese identity fields diverge"));
			}
			if (Definition.Card.PositionTypes.IsEmpty())
			{
				OutErrors.Add(Context + TEXT(": Position is empty"));
			}
			if (Definition.Card.bIsGoalkeeper)
			{
				if (Definition.Card.PositionTypes.Num() != 1
					|| Definition.Card.PositionTypes[0]
						!= EPlayerPositionType::Goalkeeper
					|| !GoalkeeperAttributesInRange(
						Definition.Card.GoalkeeperAttributes))
				{
					OutErrors.Add(Context + TEXT(": invalid goalkeeper schema"));
				}
			}
			else if (!AttributesInRange(Definition.Card.Attributes)
				|| Definition.Card.PositionTypes.Contains(
					EPlayerPositionType::Goalkeeper))
			{
				OutErrors.Add(Context + TEXT(": invalid outfield schema"));
			}

			if (Definition.SkillAssignments.Num() > 3
				|| Definition.Card.AttackSkillIds.Num()
					!= Definition.SkillAssignments.Num())
			{
				OutErrors.Add(Context + TEXT(": Skill count must be 0-3 and match Card skill references"));
			}
			TSet<FName> CanonicalSkillIds;
			for (int32 Index = 0; Index < Definition.SkillAssignments.Num(); ++Index)
			{
				const FFMCodexPrototypeSkillAssignment& Skill =
					Definition.SkillAssignments[Index];
				if (Skill.SkillId.IsNone() || Skill.RuleId.IsNone()
					|| Skill.SkillType == ESkillRuleType::None
					|| Skill.MinTacticalPoint < 2
					|| Skill.MinTacticalPoint > Skill.MaxTacticalPoint
					|| Skill.MaxTacticalPoint > 8
					|| Definition.Card.AttackSkillIds[Index] != Skill.RuleId)
				{
					OutErrors.Add(Context + TEXT(": invalid Skill assignment"));
				}
				if (CanonicalSkillIds.Contains(Skill.SkillId))
				{
					OutErrors.Add(Context + TEXT(": duplicate canonical SkillId"));
				}
				CanonicalSkillIds.Add(Skill.SkillId);
			}
			for (int32 TacticalPoint = 2; TacticalPoint <= 8; ++TacticalPoint)
			{
				int32 EligibleSkillCount = 0;
				for (const FFMCodexPrototypeSkillAssignment& Skill
					: Definition.SkillAssignments)
				{
					EligibleSkillCount +=
						TacticalPoint >= Skill.MinTacticalPoint
						&& TacticalPoint <= Skill.MaxTacticalPoint ? 1 : 0;
				}
				if (EligibleSkillCount > 2)
				{
					OutErrors.Add(FString::Printf(
						TEXT("%s: TP %d has %d eligible Skills"),
						*Context, TacticalPoint, EligibleSkillCount));
				}
			}
		}

		for (const FName TeamId : { ArsenalId, ManchesterCityId })
		{
			if (TeamCounts.FindRef(TeamId) != CanonicalPlayersPerTeam)
			{
				OutErrors.Add(FString::Printf(
					TEXT("%s: expected 20 players, found %d"),
					*TeamId.ToString(), TeamCounts.FindRef(TeamId)));
			}
			if (GoalkeeperCounts.FindRef(TeamId) != 1)
			{
				OutErrors.Add(FString::Printf(
					TEXT("%s: expected exactly one goalkeeper, found %d"),
					*TeamId.ToString(), GoalkeeperCounts.FindRef(TeamId)));
			}
			if (RosterSlots.FindRef(TeamId).Num() != CanonicalPlayersPerTeam)
			{
				OutErrors.Add(TeamId.ToString() + TEXT(": RosterSlot set is not exactly 1-20"));
			}
		}
		return OutErrors.IsEmpty();
	}

	FCatalog BuildCatalog()
	{
		FCatalog Result;
		const FString ContentPath = FFMCodexPrototypeTeamContent::GetRuntimeContentPath();
		FString JsonText;
		if (!FFileHelper::LoadFileToString(JsonText, *ContentPath))
		{
			Result.Errors.Add(TEXT("Unable to read canonical runtime player content: ") + ContentPath);
			return Result;
		}

		TSharedPtr<FJsonObject> Root;
		const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonText);
		if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
		{
			Result.Errors.Add(TEXT("Canonical runtime player content is not valid JSON"));
			return Result;
		}

		int32 SchemaVersion = 0;
		TryReadInt(Root, TEXT("schemaVersion"), SchemaVersion,
			Result.Errors, TEXT("catalog"));
		if (SchemaVersion != RuntimeSchemaVersion)
		{
			Result.Errors.Add(FString::Printf(
				TEXT("Unsupported canonical player schemaVersion %d"), SchemaVersion));
		}
		TryReadString(Root, TEXT("balanceContentVersion"),
			Result.BalanceContentVersion, Result.Errors, TEXT("catalog"));

		const TArray<TSharedPtr<FJsonValue>>* Players = nullptr;
		if (!Root->TryGetArrayField(TEXT("players"), Players) || Players == nullptr)
		{
			Result.Errors.Add(TEXT("catalog: players array is required"));
			return Result;
		}

		Result.Definitions.Reserve(Players->Num());
		for (int32 Index = 0; Index < Players->Num(); ++Index)
		{
			const TSharedPtr<FJsonObject> PlayerObject = (*Players)[Index]->AsObject();
			const FString Context = FString::Printf(TEXT("players[%d]"), Index);
			if (!PlayerObject.IsValid())
			{
				Result.Errors.Add(Context + TEXT(": object is required"));
				continue;
			}

			FFMCodexPrototypePlayerDefinition Definition;
			FString PlayerKey;
			FString Team;
			FString DisplayName;
			FString ChineseName;
			FString EnglishName;
			FString Position;
			FString Notes;
			TryReadString(PlayerObject, TEXT("playerKey"), PlayerKey,
				Result.Errors, Context);
			TryReadString(PlayerObject, TEXT("team"), Team,
				Result.Errors, Context);
			TryReadString(PlayerObject, TEXT("displayName"), DisplayName,
				Result.Errors, Context);
			TryReadString(PlayerObject, TEXT("chineseName"), ChineseName,
				Result.Errors, Context);
			TryReadString(PlayerObject, TEXT("englishName"), EnglishName,
				Result.Errors, Context);
			TryReadString(PlayerObject, TEXT("position"), Position,
				Result.Errors, Context);
			TryReadString(PlayerObject, TEXT("notes"), Notes,
				Result.Errors, Context, true);
			TryReadInt(PlayerObject, TEXT("rosterSlot"), Definition.RosterSlot,
				Result.Errors, Context);
			TryReadInt(PlayerObject, TEXT("displaySerial"), Definition.DisplaySerial,
				Result.Errors, Context);

			Definition.PlayerKey = FName(*PlayerKey);
			Definition.Card.CardId = Definition.PlayerKey;
			Definition.TeamId = TeamIdFromSource(Team);
			Definition.TeamDisplayName = TeamDisplayName(Definition.TeamId);
			Definition.Card.DisplayName = FText::FromString(ChineseName);
			Definition.CanonicalChineseDisplayName =
				FText::FromString(ChineseName);
			Definition.PreferredDisplayName = FText::FromString(DisplayName);
			Definition.EnglishDisplayName = FText::FromString(EnglishName);
			Definition.PlayerFacingSerial = FString::Printf(
				TEXT("%03d"), Definition.DisplaySerial);
			Definition.Card.Notes = Notes;

			if (!TryParsePosition(Position, Definition.Card.PositionTypes,
				Definition.Card.bIsGoalkeeper))
			{
				Result.Errors.Add(Context + TEXT(": unsupported Position '")
					+ Position + TEXT("'"));
			}

			const TSharedPtr<FJsonObject>* OutfieldAttributes = nullptr;
			const TSharedPtr<FJsonObject>* GoalkeeperAttributes = nullptr;
			const bool bHasOutfield = PlayerObject->TryGetObjectField(
				TEXT("outfieldAttributes"), OutfieldAttributes);
			const bool bHasGoalkeeper = PlayerObject->TryGetObjectField(
				TEXT("goalkeeperAttributes"), GoalkeeperAttributes);
			if (Definition.Card.bIsGoalkeeper)
			{
				if (bHasOutfield || !bHasGoalkeeper || GoalkeeperAttributes == nullptr)
				{
					Result.Errors.Add(Context + TEXT(": GK requires only goalkeeperAttributes"));
				}
				else
				{
					TryReadGoalkeeperAttributes(*GoalkeeperAttributes,
						Definition.Card.GoalkeeperAttributes,
						Result.Errors, Context);
				}
			}
			else
			{
				if (!bHasOutfield || OutfieldAttributes == nullptr || bHasGoalkeeper)
				{
					Result.Errors.Add(Context + TEXT(": outfield player requires only outfieldAttributes"));
				}
				else
				{
					TryReadOutfieldAttributes(*OutfieldAttributes,
						Definition.Card.Attributes, Result.Errors, Context);
				}
			}

			const TSharedPtr<FJsonObject>* Presentation = nullptr;
			if (!PlayerObject->TryGetObjectField(TEXT("presentation"), Presentation)
				|| Presentation == nullptr)
			{
				Result.Errors.Add(Context + TEXT(": presentation object is required"));
			}
			else
			{
				FString Nationality;
				FString BirthDate;
				FString Rarity;
				TryReadString(*Presentation, TEXT("nationality"), Nationality,
					Result.Errors, Context, true);
				TryReadString(*Presentation, TEXT("birthDate"), BirthDate,
					Result.Errors, Context, true);
				TryReadString(*Presentation, TEXT("rarity"), Rarity,
					Result.Errors, Context);
				TryReadInt(*Presentation, TEXT("heightCm"), Definition.Card.HeightCm,
					Result.Errors, Context);
				TryReadInt(*Presentation, TEXT("weightKg"), Definition.Card.WeightKg,
					Result.Errors, Context);
				Definition.NationalityDisplayName = FText::FromString(Nationality);
				Definition.Card.BirthDate = BirthDate;
				if (!TryParseRarity(Rarity, Definition.Card.Rarity))
				{
					Result.Errors.Add(Context + TEXT(": unsupported rarity '")
						+ Rarity + TEXT("'"));
				}
			}

			const TArray<TSharedPtr<FJsonValue>>* Skills = nullptr;
			if (!PlayerObject->TryGetArrayField(TEXT("skills"), Skills)
				|| Skills == nullptr)
			{
				Result.Errors.Add(Context + TEXT(": skills array is required"));
			}
			else
			{
				for (int32 SkillIndex = 0; SkillIndex < Skills->Num(); ++SkillIndex)
				{
					const TSharedPtr<FJsonObject> SkillObject =
						(*Skills)[SkillIndex]->AsObject();
					const FString SkillContext = FString::Printf(
						TEXT("%s.skills[%d]"), *Context, SkillIndex);
					if (!SkillObject.IsValid())
					{
						Result.Errors.Add(SkillContext + TEXT(": object is required"));
						continue;
					}
					FFMCodexPrototypeSkillAssignment Assignment;
					FString SkillId;
					TryReadString(SkillObject, TEXT("skillId"), SkillId,
						Result.Errors, SkillContext);
					TryReadInt(SkillObject, TEXT("minTP"),
						Assignment.MinTacticalPoint, Result.Errors, SkillContext);
					TryReadInt(SkillObject, TEXT("maxTP"),
						Assignment.MaxTacticalPoint, Result.Errors, SkillContext);
					Assignment.SkillId = FName(*SkillId);
					if (!TryParseSkillType(SkillId, Assignment.SkillType))
					{
						Result.Errors.Add(SkillContext + TEXT(": unsupported SkillId '")
							+ SkillId + TEXT("'"));
					}
					Assignment.RuleId = MakeRuleId(
						SkillId,
						Assignment.MinTacticalPoint,
						Assignment.MaxTacticalPoint);
					Definition.SkillAssignments.Add(Assignment);
					Definition.Card.AttackSkillIds.Add(Assignment.RuleId);
				}
			}
			Result.Definitions.Add(MoveTemp(Definition));
		}

		ValidateDefinitions(
			Result.BalanceContentVersion, Result.Definitions, Result.Errors);
		if (!Result.Errors.IsEmpty())
		{
			Result.Definitions.Reset();
		}
		return Result;
	}

	const FCatalog& GetCatalog()
	{
		static const FCatalog Catalog = []
		{
			FCatalog Built = BuildCatalog();
			for (const FString& Error : Built.Errors)
			{
				UE_LOG(LogFMCodexCanonicalPlayerContent, Error, TEXT("%s"), *Error);
			}
			return Built;
		}();
		return Catalog;
	}
}

const TArray<FFMCodexPrototypePlayerDefinition>&
FFMCodexPrototypeTeamContent::GetDefinitions()
{
	return FMCodexPrototypeTeamContent::GetCatalog().Definitions;
}

FString FFMCodexPrototypeTeamContent::GetBalanceContentVersion()
{
	return FMCodexPrototypeTeamContent::GetCatalog().BalanceContentVersion;
}

FString FFMCodexPrototypeTeamContent::GetRuntimeContentPath()
{
	return FPaths::Combine(
		FPaths::ProjectContentDir(),
		TEXT("Data"),
		TEXT("CanonicalPlayerContent.json"));
}

const FFMCodexPrototypePlayerDefinition*
FFMCodexPrototypeTeamContent::Find(const FName CardId)
{
	return GetDefinitions().FindByPredicate(
		[CardId](const FFMCodexPrototypePlayerDefinition& Definition)
		{
			return Definition.PlayerKey == CardId;
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
	return Definition == nullptr
		? FText::GetEmpty() : Definition->PreferredDisplayName;
}

FText FFMCodexPrototypeTeamContent::CanonicalChinesePlayerName(
	const FName CardId)
{
	const FFMCodexPrototypePlayerDefinition* Definition = Find(CardId);
	return Definition == nullptr
		? FText::GetEmpty() : Definition->CanonicalChineseDisplayName;
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
	return FMCodexPrototypeTeamContent::CanonicalPlayersPerTeam;
}

void FFMCodexPrototypeTeamContent::AppendSkillRules(
	FSkillRuleSnapshotSet& RuleSet)
{
	TSet<FName> ExistingRuleIds;
	for (const FSkillRuleSnapshot& Rule : RuleSet.SkillRules)
	{
		ExistingRuleIds.Add(Rule.SkillId);
	}
	for (const FFMCodexPrototypePlayerDefinition& Definition : GetDefinitions())
	{
		for (const FFMCodexPrototypeSkillAssignment& Assignment
			: Definition.SkillAssignments)
		{
			if (ExistingRuleIds.Contains(Assignment.RuleId))
			{
				continue;
			}
			FSkillRuleSnapshot Rule;
			Rule.SkillId = Assignment.RuleId;
			Rule.SkillType = Assignment.SkillType;
			Rule.MinTriggerActionPoint = Assignment.MinTacticalPoint;
			Rule.MaxTriggerActionPoint = Assignment.MaxTacticalPoint;
			RuleSet.SkillRules.Add(Rule);
			ExistingRuleIds.Add(Rule.SkillId);
		}
	}
}

bool FFMCodexPrototypeTeamContent::Validate(TArray<FString>& OutErrors)
{
	OutErrors = FMCodexPrototypeTeamContent::GetCatalog().Errors;
	if (!OutErrors.IsEmpty())
	{
		return false;
	}
	return FMCodexPrototypeTeamContent::ValidateDefinitions(
		GetBalanceContentVersion(), GetDefinitions(), OutErrors);
}

void FFMCodexPrototypeTeamContent::IntegrateIntoDemoDeck(
	const EInitialTurnOrderPlayer Side,
	TArray<FPlayerCardData>& Deck)
{
	if (Deck.Num() != CardsPerTeam())
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

	TArray<const FFMCodexPrototypePlayerDefinition*> TeamDefinitions;
	for (const FFMCodexPrototypePlayerDefinition& Definition : GetDefinitions())
	{
		if (Definition.TeamId == ExpectedTeam)
		{
			TeamDefinitions.Add(&Definition);
		}
	}
	TeamDefinitions.Sort(
		[](const FFMCodexPrototypePlayerDefinition& Left,
			const FFMCodexPrototypePlayerDefinition& Right)
		{
			return Left.RosterSlot < Right.RosterSlot;
		});
	if (TeamDefinitions.Num() != CardsPerTeam())
	{
		return;
	}
	for (int32 Index = 0; Index < TeamDefinitions.Num(); ++Index)
	{
		Deck[Index] = TeamDefinitions[Index]->Card;
	}
}
