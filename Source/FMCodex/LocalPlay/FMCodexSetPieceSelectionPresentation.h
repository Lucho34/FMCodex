#pragma once
#include "CoreMinimal.h"
#include "../CoreRules/MatchPlayState.h"
#include "FMCodexSetPieceSelectionPresentation.generated.h"
struct FFMCodexLocalMatchInteractionView;

/** Bounded public entry/selection values only. Never participant snapshots or future resolution facts. */
USTRUCT()
struct FMCODEX_API FFMCodexSetPieceSelectionPresentation
{
 GENERATED_BODY()
 UPROPERTY() bool bVisible = false;
 UPROPERTY() bool bSelectionSupported = false;
 UPROPERTY() bool bOptionsUnavailable = false;
 UPROPERTY() bool bTypeWait = false;
 UPROPERTY() bool bTakerWait = false;
 UPROPERTY() bool bMethodWait = false;
 UPROPERTY() bool bCanRollType = false;
 UPROPERTY() bool bNoTakerNoGoal = false;
 UPROPERTY() EInitialTurnOrderPlayer AttackingSide = EInitialTurnOrderPlayer::None;
 UPROPERTY() EInitialTurnOrderPlayer ActingSide = EInitialTurnOrderPlayer::None;
 UPROPERTY() int32 TypeD6 = 0;
 UPROPERTY() ESetPieceSelectedType Type = ESetPieceSelectedType::None;
 UPROPERTY() EMatchPlaySetPieceCarrierRouteStage CarrierStage = EMatchPlaySetPieceCarrierRouteStage::None;
 UPROPERTY() EMatchPlaySetPieceCornerRouteStage CornerStage = EMatchPlaySetPieceCornerRouteStage::None;
 UPROPERTY() bool bCornerDraft = false;
 UPROPERTY() bool bCornerIntentWait = false;
 UPROPERTY() bool bCornerAttackerLocked = false;
 UPROPERTY() bool bCornerDefenderLocked = false;
 UPROPERTY() bool bHideCornerAttackerDetails = false;
 UPROPERTY() TArray<FName> CornerOptions;
 UPROPERTY() TArray<FName> CornerAttackers;
 UPROPERTY() TArray<FName> CornerDefenders;
 UPROPERTY() TArray<FString> CornerAttackerRollLabels;
 UPROPERTY() TArray<FString> CornerDefenderRollLabels;
 UPROPERTY() FName CornerRunner = NAME_None;
 UPROPERTY() FName CornerHelper = NAME_None;
 UPROPERTY() int32 CornerSharedD6 = 0;
 UPROPERTY() int32 CornerRouteD6 = 0;
 UPROPERTY() EMatchPlayCornerRouteIntent CornerIntendedRoute = EMatchPlayCornerRouteIntent::None;
 UPROPERTY() EMatchPlayCornerRouteIntent CornerActualRoute = EMatchPlayCornerRouteIntent::None;
 UPROPERTY() EInitialTurnOrderPlayer CornerBonusSide = EInitialTurnOrderPlayer::None;
 UPROPERTY() int32 CornerBonus = 0;
 UPROPERTY() FText TypeLabel;
 UPROPERTY() FName TakerCardId = NAME_None;
 UPROPERTY() FText TakerLabel;
 UPROPERTY() TArray<FName> TakerOptions;
 UPROPERTY() TArray<EMatchPlayShortFreeKickMethod> NearMethods;
 UPROPERTY() TArray<EMatchPlayLongFreeKickMethod> LongMethods;
 UPROPERTY() TArray<EMatchPlayPenaltyMethod> PenaltyMethods;
 UPROPERTY() EMatchPlayShortFreeKickMethod NearMethod = EMatchPlayShortFreeKickMethod::None;
 UPROPERTY() EMatchPlayLongFreeKickMethod LongMethod = EMatchPlayLongFreeKickMethod::None;
 UPROPERTY() EMatchPlayPenaltyMethod PenaltyMethod = EMatchPlayPenaltyMethod::None;
 static constexpr int32 MaxTakers = 19; // Admitted 20-card roster, including its unique GK.
 static FFMCodexSetPieceSelectionPresentation Build(const FFMCodexLocalMatchInteractionView& SafeView,
  EInitialTurnOrderPlayer Viewer);
 void DisableActions();
};
