#include "MatchPlayDeploymentPhysicalAreaMatchQuery.h"

namespace MatchPlayDeploymentPhysicalAreaMatchQueryImplementation
{
	bool IsPlayerSide(const EInitialTurnOrderPlayer PlayerSide)
	{
		return PlayerSide == EInitialTurnOrderPlayer::PlayerA
			|| PlayerSide == EInitialTurnOrderPlayer::PlayerB;
	}

	void SetError(
		FMatchPlayDeploymentPhysicalAreaMatchResult& Result,
		const EMatchPlayDeploymentPhysicalAreaMatchErrorCode ErrorCode,
		const FString& ErrorMessage)
	{
		Result.ErrorCode = ErrorCode;
		Result.ErrorMessage = ErrorMessage;
	}

	void PopulateResolverDiagnostic(
		const FMatchPlayDeploymentSlotCatalog& SlotCatalog,
		const EInitialTurnOrderPlayer CurrentAttackingPlayer,
		FMatchPlayDeploymentPhysicalAreaDiagnostic& Diagnostic)
	{
		Diagnostic.RelativeZoneResolveResult =
			FMatchPlayRelativeDeploymentZoneResolver::Resolve(
				SlotCatalog,
				Diagnostic.SlotId,
				CurrentAttackingPlayer,
				Diagnostic.PlayerSide);
		if (Diagnostic.RelativeZoneResolveResult.bSuccess)
		{
			Diagnostic.NeutralSide =
				Diagnostic.RelativeZoneResolveResult.NeutralSide;
			Diagnostic.RelativeZone =
				Diagnostic.RelativeZoneResolveResult.RelativeZone;
		}
	}

	bool ResolvePlacement(
		const FMatchPlayDeploymentSlotCatalog& SlotCatalog,
		const EInitialTurnOrderPlayer CurrentAttackingPlayer,
		const bool bFirstPlacement,
		FMatchPlayDeploymentPhysicalAreaDiagnostic& Diagnostic,
		FMatchPlayDeploymentPhysicalAreaMatchResult& Result)
	{
		if (!IsPlayerSide(Diagnostic.PlayerSide))
		{
			PopulateResolverDiagnostic(
				SlotCatalog,
				CurrentAttackingPlayer,
				Diagnostic);
			SetError(
				Result,
				bFirstPlacement
					? EMatchPlayDeploymentPhysicalAreaMatchErrorCode
						::InvalidFirstPlayerSide
					: EMatchPlayDeploymentPhysicalAreaMatchErrorCode
						::InvalidSecondPlayerSide,
				bFirstPlacement
					? TEXT("First placement PlayerSide must be PlayerA or PlayerB.")
					: TEXT("Second placement PlayerSide must be PlayerA or PlayerB."));
			return false;
		}

		if (Diagnostic.SlotId.IsNone())
		{
			PopulateResolverDiagnostic(
				SlotCatalog,
				CurrentAttackingPlayer,
				Diagnostic);
			SetError(
				Result,
				bFirstPlacement
					? EMatchPlayDeploymentPhysicalAreaMatchErrorCode
						::InvalidFirstSlotId
					: EMatchPlayDeploymentPhysicalAreaMatchErrorCode
						::InvalidSecondSlotId,
				bFirstPlacement
					? TEXT("First placement SlotId must be non-empty.")
					: TEXT("Second placement SlotId must be non-empty."));
			return false;
		}

		Diagnostic.SlotQueryResult =
			FMatchPlayDeploymentSlotCatalogQuery::FindSlot(
				SlotCatalog,
				Diagnostic.SlotId);
		if (!Diagnostic.SlotQueryResult.bSuccess)
		{
			PopulateResolverDiagnostic(
				SlotCatalog,
				CurrentAttackingPlayer,
				Diagnostic);
			const bool bSlotNotFound =
				Diagnostic.SlotQueryResult.ErrorCode
				== EMatchPlayDeploymentSlotCatalogQueryErrorCode
					::SlotNotFound;
			SetError(
				Result,
				bFirstPlacement
					? (bSlotNotFound
						? EMatchPlayDeploymentPhysicalAreaMatchErrorCode
							::FirstSlotNotFound
						: EMatchPlayDeploymentPhysicalAreaMatchErrorCode
							::FirstSlotLookupFailed)
					: (bSlotNotFound
						? EMatchPlayDeploymentPhysicalAreaMatchErrorCode
							::SecondSlotNotFound
						: EMatchPlayDeploymentPhysicalAreaMatchErrorCode
							::SecondSlotLookupFailed),
				Diagnostic.SlotQueryResult.ErrorMessage);
			return false;
		}

		PopulateResolverDiagnostic(
			SlotCatalog,
			CurrentAttackingPlayer,
			Diagnostic);
		if (!Diagnostic.RelativeZoneResolveResult.bSuccess)
		{
			SetError(
				Result,
				bFirstPlacement
					? EMatchPlayDeploymentPhysicalAreaMatchErrorCode
						::FirstRelativeZoneResolutionFailed
					: EMatchPlayDeploymentPhysicalAreaMatchErrorCode
						::SecondRelativeZoneResolutionFailed,
				Diagnostic.RelativeZoneResolveResult.ErrorMessage);
			return false;
		}

		Diagnostic.NeutralSide =
			Diagnostic.SlotQueryResult.SlotDefinition.NeutralSide;
		return true;
	}
}

FMatchPlayDeploymentPhysicalAreaMatchResult
FMatchPlayDeploymentPhysicalAreaMatchQuery::Query(
	const FMatchPlayDeploymentSlotCatalog& SlotCatalog,
	const EInitialTurnOrderPlayer CurrentAttackingPlayer,
	const FMatchPlayDeploymentPlacement& FirstPlacement,
	const FMatchPlayDeploymentPlacement& SecondPlacement)
{
	using namespace
		MatchPlayDeploymentPhysicalAreaMatchQueryImplementation;

	FMatchPlayDeploymentPhysicalAreaMatchResult Result;
	Result.CurrentAttackingPlayer = CurrentAttackingPlayer;
	Result.FirstDiagnostic.PlayerSide = FirstPlacement.PlayerSide;
	Result.FirstDiagnostic.SlotId = FirstPlacement.SlotId;
	Result.SecondDiagnostic.PlayerSide = SecondPlacement.PlayerSide;
	Result.SecondDiagnostic.SlotId = SecondPlacement.SlotId;

	Result.SlotCatalogValidationResult =
		FMatchPlayDeploymentSlotCatalogValidator::Validate(SlotCatalog);
	if (!Result.SlotCatalogValidationResult.bSuccess)
	{
		SetError(
			Result,
			EMatchPlayDeploymentPhysicalAreaMatchErrorCode
				::InvalidSlotCatalog,
			Result.SlotCatalogValidationResult.ErrorMessage);
		return Result;
	}

	if (!IsPlayerSide(CurrentAttackingPlayer))
	{
		SetError(
			Result,
			EMatchPlayDeploymentPhysicalAreaMatchErrorCode
				::InvalidCurrentAttackingPlayer,
			TEXT("CurrentAttackingPlayer must be PlayerA or PlayerB."));
		return Result;
	}

	if (!ResolvePlacement(
		SlotCatalog,
		CurrentAttackingPlayer,
		true,
		Result.FirstDiagnostic,
		Result))
	{
		return Result;
	}

	if (!ResolvePlacement(
		SlotCatalog,
		CurrentAttackingPlayer,
		false,
		Result.SecondDiagnostic,
		Result))
	{
		return Result;
	}

	Result.bSamePhysicalArea =
		Result.FirstDiagnostic.NeutralSide
		== Result.SecondDiagnostic.NeutralSide;
	Result.bSuccess = true;
	Result.ErrorCode =
		EMatchPlayDeploymentPhysicalAreaMatchErrorCode::None;
	Result.ErrorMessage.Empty();
	return Result;
}
