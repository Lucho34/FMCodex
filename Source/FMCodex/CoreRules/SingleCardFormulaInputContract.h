#pragma once

#include "CoreMinimal.h"

#include "CoreRuleEnums.h"

enum class ESingleCardFormulaParticipantRole : uint8
{
	None,
	Attacker,
	Defender,
	Goalkeeper
};

enum class ESingleCardFormulaAttribute : uint8
{
	None,
	Shooting,
	Dribbling,
	Passing,
	OffBall,
	Marking,
	Tackling,
	Speed,
	Strength,
	Stamina,
	LongShot,
	GoalkeeperHandling,
	GoalkeeperPositioning,
	GoalkeeperReflex,
	GoalkeeperAerial,
	GoalkeeperAnticipation,
	GoalkeeperOneOnOne
};

struct FMCODEX_API FSingleCardFormulaInputContract
{
	FName CardId = NAME_None;
	EFormulaType FormulaType = EFormulaType::None;
	ESingleCardFormulaParticipantRole ParticipantRole =
		ESingleCardFormulaParticipantRole::None;
	ESingleCardFormulaAttribute Attribute =
		ESingleCardFormulaAttribute::None;
	bool bHasExternalD6ComparePoint = false;
	int32 ExternalD6ComparePoint = 0;
	bool bHasExternalModifier = false;
	float ExternalModifier = 0.0f;
	FGuid LogId;
	int32 TurnIndex = 0;
	FName ContextTag = NAME_None;
};
