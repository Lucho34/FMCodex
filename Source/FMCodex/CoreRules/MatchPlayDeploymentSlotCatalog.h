#pragma once

#include "CoreMinimal.h"

#include "InitialTurnOrderResolver.h"

#include "MatchPlayDeploymentSlotCatalog.generated.h"

UENUM(BlueprintType)
enum class EMatchPlayNeutralSlotSide : uint8
{
	None UMETA(DisplayName = "None"),
	NearPlayerA UMETA(DisplayName = "Near Player A"),
	NearPlayerB UMETA(DisplayName = "Near Player B")
};

UENUM(BlueprintType)
enum class EMatchPlayRelativeDeploymentZone : uint8
{
	None UMETA(DisplayName = "None"),
	Forward UMETA(DisplayName = "Forward"),
	Midfield UMETA(DisplayName = "Midfield"),
	Backfield UMETA(DisplayName = "Backfield")
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayDeploymentSlotDefinition
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Deployment Slots")
	FName SlotId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Deployment Slots")
	EMatchPlayNeutralSlotSide NeutralSide =
		EMatchPlayNeutralSlotSide::None;
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayDeploymentSlotCatalog
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Deployment Slots")
	TArray<FMatchPlayDeploymentSlotDefinition> Slots;
};

UENUM(BlueprintType)
enum class EMatchPlayDeploymentSlotCatalogValidationErrorCode : uint8
{
	None UMETA(DisplayName = "None"),
	EmptyCatalog UMETA(DisplayName = "Empty Catalog"),
	EmptySlotId UMETA(DisplayName = "Empty Slot Id"),
	DuplicateSlotId UMETA(DisplayName = "Duplicate Slot Id"),
	InvalidNeutralSide UMETA(DisplayName = "Invalid Neutral Side")
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayDeploymentSlotCatalogValidationResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Deployment Slots")
	bool bSuccess = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Deployment Slots")
	EMatchPlayDeploymentSlotCatalogValidationErrorCode ErrorCode =
		EMatchPlayDeploymentSlotCatalogValidationErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Deployment Slots")
	FString ErrorMessage;
};

class FMCODEX_API FMatchPlayDeploymentSlotCatalogValidator final
{
public:
	static FMatchPlayDeploymentSlotCatalogValidationResult Validate(
		const FMatchPlayDeploymentSlotCatalog& SlotCatalog);
};

UENUM(BlueprintType)
enum class EMatchPlayDeploymentSlotCatalogQueryErrorCode : uint8
{
	None UMETA(DisplayName = "None"),
	InvalidSlotId UMETA(DisplayName = "Invalid Slot Id"),
	InvalidCatalog UMETA(DisplayName = "Invalid Catalog"),
	SlotNotFound UMETA(DisplayName = "Slot Not Found")
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayDeploymentSlotCatalogQueryResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Deployment Slots")
	bool bSuccess = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Deployment Slots")
	FName SlotId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Deployment Slots")
	FMatchPlayDeploymentSlotDefinition SlotDefinition;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Deployment Slots")
	EMatchPlayDeploymentSlotCatalogQueryErrorCode ErrorCode =
		EMatchPlayDeploymentSlotCatalogQueryErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Deployment Slots")
	FString ErrorMessage;
};

class FMCODEX_API FMatchPlayDeploymentSlotCatalogQuery final
{
public:
	static FMatchPlayDeploymentSlotCatalogQueryResult FindSlot(
		const FMatchPlayDeploymentSlotCatalog& SlotCatalog,
		FName SlotId);
};

UENUM(BlueprintType)
enum class EMatchPlayRelativeDeploymentZoneResolveErrorCode : uint8
{
	None UMETA(DisplayName = "None"),
	InvalidSlotId UMETA(DisplayName = "Invalid Slot Id"),
	InvalidCurrentAttackingPlayer
		UMETA(DisplayName = "Invalid Current Attacking Player"),
	InvalidEvaluatedPlayerSide
		UMETA(DisplayName = "Invalid Evaluated Player Side"),
	InvalidCatalog UMETA(DisplayName = "Invalid Catalog"),
	SlotNotFound UMETA(DisplayName = "Slot Not Found")
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayRelativeDeploymentZoneResolveResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Deployment Slots")
	bool bSuccess = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Deployment Slots")
	FName SlotId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Deployment Slots")
	EInitialTurnOrderPlayer CurrentAttackingPlayer =
		EInitialTurnOrderPlayer::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Deployment Slots")
	EInitialTurnOrderPlayer EvaluatedPlayerSide =
		EInitialTurnOrderPlayer::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Deployment Slots")
	EMatchPlayNeutralSlotSide NeutralSide =
		EMatchPlayNeutralSlotSide::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Deployment Slots")
	EMatchPlayRelativeDeploymentZone RelativeZone =
		EMatchPlayRelativeDeploymentZone::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Deployment Slots")
	EMatchPlayRelativeDeploymentZoneResolveErrorCode ErrorCode =
		EMatchPlayRelativeDeploymentZoneResolveErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Deployment Slots")
	FString ErrorMessage;
};

class FMCODEX_API FMatchPlayRelativeDeploymentZoneResolver final
{
public:
	static FMatchPlayRelativeDeploymentZoneResolveResult Resolve(
		const FMatchPlayDeploymentSlotCatalog& SlotCatalog,
		FName SlotId,
		EInitialTurnOrderPlayer CurrentAttackingPlayer,
		EInitialTurnOrderPlayer EvaluatedPlayerSide);
};
