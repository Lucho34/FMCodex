#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCurrentAttackCarrierSelectionLegality.h"

#include "MatchPlayCurrentAttackCarrierSelectionWriter.generated.h"

UENUM(BlueprintType)
enum class EMatchPlayCurrentAttackCarrierSelectionWriterErrorCode : uint8
{
	None UMETA(DisplayName = "None"),
	LegalityFailed UMETA(DisplayName = "Legality Failed")
};

USTRUCT(BlueprintType)
struct FMCODEX_API
	FMatchPlayCurrentAttackCarrierSelectionWriterResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Carrier Selection Writer")
	bool bSuccess = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Carrier Selection Writer")
	FMatchPlayCurrentAttackCarrierSelectionRequest Request;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Carrier Selection Writer")
	FMatchPlayState BeforeState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Carrier Selection Writer")
	FMatchPlayState AfterState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Carrier Selection Writer")
	EMatchPlayCurrentAttackCarrierSelectionWriterErrorCode ErrorCode =
		EMatchPlayCurrentAttackCarrierSelectionWriterErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Carrier Selection Writer")
	FMatchPlayCurrentAttackCarrierSelectionLegalityResult LegalityResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Carrier Selection Writer")
	FName SelectedCarrierCardId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Carrier Selection Writer")
	FString ErrorMessage;
};

class FMCODEX_API FMatchPlayCurrentAttackCarrierSelectionWriter final
{
public:
	static FMatchPlayCurrentAttackCarrierSelectionWriterResult Select(
		const FMatchPlayState& BeforeState,
		const FMatchPlayCurrentAttackCarrierSelectionRequest& Request);
};
