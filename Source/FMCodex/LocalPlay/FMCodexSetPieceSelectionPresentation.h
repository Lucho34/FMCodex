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
