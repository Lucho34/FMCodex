#pragma once

#include "CoreMinimal.h"

#include "MatchPlayDeploymentSlotCatalog.h"
#include "MatchPlayState.h"

#include "MatchPlayDeploymentPhysicalAreaMatchQuery.generated.h"

UENUM(BlueprintType)
enum class EMatchPlayDeploymentPhysicalAreaMatchErrorCode : uint8
{
	None UMETA(DisplayName = "None"),
	InvalidSlotCatalog UMETA(DisplayName = "Invalid Slot Catalog"),
	InvalidCurrentAttackingPlayer
		UMETA(DisplayName = "Invalid Current Attacking Player"),
	InvalidFirstPlayerSide
		UMETA(DisplayName = "Invalid First Player Side"),
	InvalidFirstSlotId UMETA(DisplayName = "Invalid First Slot Id"),
	FirstSlotNotFound UMETA(DisplayName = "First Slot Not Found"),
	FirstSlotLookupFailed
		UMETA(DisplayName = "First Slot Lookup Failed"),
	FirstRelativeZoneResolutionFailed
		UMETA(DisplayName = "First Relative Zone Resolution Failed"),
	InvalidSecondPlayerSide
		UMETA(DisplayName = "Invalid Second Player Side"),
	InvalidSecondSlotId UMETA(DisplayName = "Invalid Second Slot Id"),
	SecondSlotNotFound UMETA(DisplayName = "Second Slot Not Found"),
	SecondSlotLookupFailed
		UMETA(DisplayName = "Second Slot Lookup Failed"),
	SecondRelativeZoneResolutionFailed
		UMETA(DisplayName = "Second Relative Zone Resolution Failed")
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayDeploymentPhysicalAreaDiagnostic
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Deployment Physical Area")
	EInitialTurnOrderPlayer PlayerSide =
		EInitialTurnOrderPlayer::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Deployment Physical Area")
	FName SlotId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Deployment Physical Area")
	EMatchPlayNeutralSlotSide NeutralSide =
		EMatchPlayNeutralSlotSide::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Deployment Physical Area")
	EMatchPlayRelativeDeploymentZone RelativeZone =
		EMatchPlayRelativeDeploymentZone::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Deployment Physical Area")
	FMatchPlayDeploymentSlotCatalogQueryResult SlotQueryResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Deployment Physical Area")
	FMatchPlayRelativeDeploymentZoneResolveResult
		RelativeZoneResolveResult;
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayDeploymentPhysicalAreaMatchResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Deployment Physical Area")
	bool bSuccess = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Deployment Physical Area")
	bool bSamePhysicalArea = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Deployment Physical Area")
	EInitialTurnOrderPlayer CurrentAttackingPlayer =
		EInitialTurnOrderPlayer::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Deployment Physical Area")
	FMatchPlayDeploymentPhysicalAreaDiagnostic FirstDiagnostic;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Deployment Physical Area")
	FMatchPlayDeploymentPhysicalAreaDiagnostic SecondDiagnostic;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Deployment Physical Area")
	FMatchPlayDeploymentSlotCatalogValidationResult
		SlotCatalogValidationResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Deployment Physical Area")
	EMatchPlayDeploymentPhysicalAreaMatchErrorCode ErrorCode =
		EMatchPlayDeploymentPhysicalAreaMatchErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Deployment Physical Area")
	FString ErrorMessage;
};

class FMCODEX_API FMatchPlayDeploymentPhysicalAreaMatchQuery final
{
public:
	static FMatchPlayDeploymentPhysicalAreaMatchResult Query(
		const FMatchPlayDeploymentSlotCatalog& SlotCatalog,
		EInitialTurnOrderPlayer CurrentAttackingPlayer,
		const FMatchPlayDeploymentPlacement& FirstPlacement,
		const FMatchPlayDeploymentPlacement& SecondPlacement);
};
