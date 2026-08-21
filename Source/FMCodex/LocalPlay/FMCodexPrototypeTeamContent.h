#pragma once

#include "CoreMinimal.h"

#include "../CoreRules/InitialTurnOrderResolver.h"
#include "../CoreRules/PlayerCardTypes.h"
#include "../CoreRules/SkillRuleSnapshot.h"

struct FFMCodexPrototypeSkillAssignment
{
	/** Canonical workbook SkillId, for example Cross. */
	FName SkillId = NAME_None;
	/** Runtime rule identity for this exact SkillId + TP range tuple. */
	FName RuleId = NAME_None;
	ESkillRuleType SkillType = ESkillRuleType::None;
	int32 MinTacticalPoint = 0;
	int32 MaxTacticalPoint = 0;
};

struct FFMCodexPrototypePlayerDefinition
{
	/** Hidden stable content identity; identical to Card.CardId. */
	FName PlayerKey = NAME_None;
	FName TeamId = NAME_None;
	FText TeamDisplayName;
	/** Complete Chinese identity/provenance name from canonical balance content. */
	FText CanonicalChineseDisplayName;
	/** Explicit player-facing compact/title name from presentation configuration. */
	FText PreferredDisplayName;
	FText EnglishDisplayName;
	FText NationalityDisplayName;
	int32 RosterSlot = 0;
	/** Workbook PlayerId, intentionally presentation-only. */
	int32 DisplaySerial = 0;
	/** Three-digit formatting derived from DisplaySerial for the frozen Full Card. */
	FString PlayerFacingSerial;
	TArray<FFMCodexPrototypeSkillAssignment> SkillAssignments;
	FPlayerCardData Card;
};

/**
 * Data-driven canonical player catalog. The packaged runtime reads generated
 * JSON, never the XLSX authoring workbook. Stable PlayerKey/CardId identity is
 * separate from DisplaySerial and RosterSlot presentation/order metadata.
 */
class FMCODEX_API FFMCodexPrototypeTeamContent final
{
public:
	static const TArray<FFMCodexPrototypePlayerDefinition>& GetDefinitions();
	static FString GetBalanceContentVersion();
	static FString GetRuntimeContentPath();
	static const FFMCodexPrototypePlayerDefinition* Find(FName CardId);
	static bool IsPrototypeCard(FName CardId);
	static FName TeamIdForCard(FName CardId);
	static FText PlayerDisplayName(FName CardId);
	static FText CanonicalChinesePlayerName(FName CardId);
	static FText TeamDisplayName(FName CardId);

	static FName ArsenalTeamId();
	static FName ManchesterCityTeamId();
	static int32 CardsPerTeam();
	static void AppendSkillRules(FSkillRuleSnapshotSet& RuleSet);
	static bool Validate(TArray<FString>& OutErrors);

	static void IntegrateIntoDemoDeck(
		EInitialTurnOrderPlayer Side,
		TArray<FPlayerCardData>& Deck);
};
