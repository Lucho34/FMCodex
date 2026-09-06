#pragma once
#include "CoreMinimal.h"
#include "../LocalPlay/FMCodexLocalMatchUMGPresentation.h"
#include "FMCodexNetworkMatchPresentation.generated.h"

struct FFMCodexLocalMatchInteractionView;
struct FFMCodexNetworkClientViewSnapshot;

/**
 * Owner-only presentation for the supported ordinary paths. Built exclusively
 * from BuildForViewer, in the same revision as the narrow action snapshot.
 * Static card display data is interned once per stable CardId; art stays local.
 * No State, Session, RNG provider, legality query, or presentation timer travels.
 */
USTRUCT()
struct FMCODEX_API FFMCodexNetworkMatchPresentation
{
	GENERATED_BODY()
	UPROPERTY() bool bAvailable = false;
	UPROPERTY() FFMCodexUMGMatchHeaderViewModel Header;
	UPROPERTY() FFMCodexUMGCardRackViewModel LocalRack;
	UPROPERTY() FFMCodexUMGCardRackViewModel OpponentRack;
	UPROPERTY() TArray<FFMCodexUMGPitchRegionViewModel> PitchRegions;
	UPROPERTY() TArray<FFMCodexUMGCardViewModel> CardCatalog;
	UPROPERTY() FFMCodexUMGInteractionViewModel Interaction;
	UPROPERTY() FFMCodexUMGInlineFormulaSurfaceViewModel InlineFormula;
	/** At most route + attacker + defender; only accepted, disclosed values. */
	UPROPERTY() TArray<FFMCodexUMGResolvedRollViewModel> ResolvedRolls;
	/** Existing shared branch surface, named for its first Local consumer. */
	UPROPERTY() FFMCodexUMGLongShotResolutionViewModel BranchSurface;
	/** Existing shared ThroughBall surface, restricted to initial route / Feet. */
	UPROPERTY() FFMCodexUMGThroughBallResolutionViewModel ThroughBallSurface;
	UPROPERTY() FFMCodexFullTimePresentation FullTime;
};

class FMCODEX_API FFMCodexNetworkMatchPresentationAdapter final
{
public:
	static constexpr int32 MaxCardsPerSide = 20;
	static constexpr int32 MaxPitchRegions = 8;
	static constexpr int32 MaxSlotsPerRegion = 20;
	static FFMCodexNetworkMatchPresentation Project(
		const FFMCodexLocalMatchInteractionView& SafeView, EInitialTurnOrderPlayer Viewer, bool bFeetMilestoneCapability = false);
	static FFMCodexUMGMatchScreenViewModel Read(
		const FFMCodexNetworkMatchPresentation& View, bool bPending);
	/** Uses existing typed owner/actor facts; adds no replicated prompt state. */
	static FFMCodexUMGMatchScreenViewModel Read(
		const FFMCodexNetworkClientViewSnapshot& View, bool bPending);
	static void DisableActions(FFMCodexUMGMatchScreenViewModel& Model);
};
