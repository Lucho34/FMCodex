#pragma once

#include "CoreMinimal.h"

/**
 * Bounded, presentation-only zh-CN terminology for player-card UMG.
 * Canonical gameplay values remain English and never depend on these labels.
 */
class FMCODEX_API FFMCodexPlayerUIPresentationText final
{
public:
	static FText Role(const FString& CanonicalLabel);
	static FText Skill(const FString& CanonicalLabel);
	static FText Attribute(const FString& CanonicalEntry);
	static FText Status(const FString& CanonicalLabel);
	static FText Rarity(const FString& CanonicalLabel);
	static FText Owner(const FString& CanonicalLabel);

	static FText UnknownCard();
	static FText UnknownRole();
	static FText UnknownRarity();
	static FText NoSkill();
	static FText AttributesUnavailable();
	static FText Unavailable();
	static FText PortraitPlaceholder();
	static FText SkillsHeading();
	static FText AttributesHeading();
	static FText DeploymentHandInstruction();
	static FText EmptyPitchSlot();
	static FText ValidDeploymentTarget();
	static FText InvalidDeploymentTarget();
	static FText OccupiedDeploymentTarget();
	static FText UnavailableDeploymentTarget();
};
