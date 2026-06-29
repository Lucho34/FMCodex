#pragma once

#include "CoreMinimal.h"

#include "MatchPlayAvailabilityQuery.h"
#include "MatchPlayStatusQuery.h"

#include "MatchPlayActionPreview.generated.h"

UENUM(BlueprintType)
enum class EMatchPlayActionPreviewErrorCode : uint8
{
	None UMETA(DisplayName = "None"),
	CannotPlaySelectedCard UMETA(DisplayName = "Cannot Play Selected Card")
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayActionPreviewResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Action Preview")
	bool bPreviewAvailable = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Action Preview")
	EInitialTurnOrderPlayer PlayerSide = EInitialTurnOrderPlayer::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Action Preview")
	FName CardId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Action Preview")
	FMatchPlayStatus StatusSnapshot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Action Preview")
	FMatchPlayAvailabilityResult AvailabilityResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Action Preview")
	bool bCanPlay = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Action Preview")
	bool bRequiresExternalFormulaInput = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Action Preview")
	EMatchPlayActionPreviewErrorCode ErrorCode =
		EMatchPlayActionPreviewErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Action Preview")
	FString ErrorMessage;
};

class FMCODEX_API FMatchPlayActionPreview final
{
public:
	static FMatchPlayActionPreviewResult PreviewAction(
		const FMatchPlayState& MatchPlayState,
		EInitialTurnOrderPlayer PlayerSide,
		FName CardId);
};
