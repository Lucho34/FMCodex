#include "MatchPlayActionPreview.h"

FMatchPlayActionPreviewResult FMatchPlayActionPreview::PreviewAction(
	const FMatchPlayState& MatchPlayState,
	const EInitialTurnOrderPlayer PlayerSide,
	const FName CardId)
{
	FMatchPlayActionPreviewResult Result;
	Result.PlayerSide = PlayerSide;
	Result.CardId = CardId;
	Result.StatusSnapshot =
		FMatchPlayStatusQuery::QueryStatus(MatchPlayState);
	Result.AvailabilityResult =
		FMatchPlayAvailabilityQuery::QueryCanPlayCard(
			MatchPlayState,
			PlayerSide,
			CardId);
	Result.bCanPlay = Result.AvailabilityResult.bCanPlay;
	Result.bRequiresExternalFormulaInput = true;
	Result.bPreviewAvailable = true;

	if (!Result.bCanPlay)
	{
		Result.ErrorCode =
			EMatchPlayActionPreviewErrorCode::CannotPlaySelectedCard;
		Result.ErrorMessage =
			Result.AvailabilityResult.ErrorMessage;
	}

	return Result;
}
