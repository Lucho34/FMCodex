#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCurrentAttackMarkerSelectionLegality.h"

#include "MatchPlayCurrentAttackMarkerSelectionWriter.generated.h"

UENUM(BlueprintType)
enum class EMatchPlayCurrentAttackMarkerSelectionWriterErrorCode : uint8
{
	None UMETA(DisplayName = "None"),
	LegalityFailed UMETA(DisplayName = "Legality Failed")
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayCurrentAttackMarkerSelectionWriterResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Marker Selection Writer")
	bool bSuccess = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Marker Selection Writer")
	FMatchPlayCurrentAttackMarkerSelectionRequest Request;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Marker Selection Writer")
	FMatchPlayState BeforeState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Marker Selection Writer")
	FMatchPlayState AfterState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Marker Selection Writer")
	EMatchPlayCurrentAttackMarkerSelectionWriterErrorCode ErrorCode =
		EMatchPlayCurrentAttackMarkerSelectionWriterErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Marker Selection Writer")
	FMatchPlayCurrentAttackMarkerSelectionLegalityResult LegalityResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Marker Selection Writer")
	FName SelectedMarkerCardId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Marker Selection Writer")
	FString ErrorMessage;
};

class FMCODEX_API FMatchPlayCurrentAttackMarkerSelectionWriter final
{
public:
	static FMatchPlayCurrentAttackMarkerSelectionWriterResult Select(
		const FMatchPlayState& BeforeState,
		const FMatchPlayCurrentAttackMarkerSelectionRequest& Request);
};
