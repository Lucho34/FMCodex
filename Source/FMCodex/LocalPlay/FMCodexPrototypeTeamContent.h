#pragma once

#include "CoreMinimal.h"

#include "../CoreRules/InitialTurnOrderResolver.h"
#include "../CoreRules/PlayerCardTypes.h"

struct FFMCodexPrototypePlayerDefinition
{
	FName TeamId = NAME_None;
	FText TeamDisplayName;
	FText EnglishDisplayName;
	FText NationalityDisplayName;
	FString PlayerFacingSerial;
	FPlayerCardData Card;
};

/**
 * Formal Prototype content catalog. It supplies exactly eight real-player
 * identities per team while continuing to use the existing player-card
 * gameplay schema and the existing twenty-card LocalPlay decks.
 */
class FMCODEX_API FFMCodexPrototypeTeamContent final
{
public:
	static const TArray<FFMCodexPrototypePlayerDefinition>& GetDefinitions();
	static const FFMCodexPrototypePlayerDefinition* Find(FName CardId);
	static bool IsPrototypeCard(FName CardId);
	static FName TeamIdForCard(FName CardId);
	static FText PlayerDisplayName(FName CardId);
	static FText TeamDisplayName(FName CardId);

	static FName ArsenalTeamId();
	static FName ManchesterCityTeamId();
	static int32 CardsPerTeam();

	static void IntegrateIntoDemoDeck(
		EInitialTurnOrderPlayer Side,
		TArray<FPlayerCardData>& Deck);
};
