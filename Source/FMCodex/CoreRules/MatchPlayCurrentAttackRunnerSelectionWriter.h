#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCurrentAttackRunnerSelectionLegality.h"

#include "MatchPlayCurrentAttackRunnerSelectionWriter.generated.h"

UENUM(BlueprintType)
enum class EMatchPlayCurrentAttackRunnerSelectionWriterErrorCode : uint8
{
	None UMETA(DisplayName = "None"),
	LegalityFailed UMETA(DisplayName = "Legality Failed")
};

USTRUCT(BlueprintType)
struct FMCODEX_API
	FMatchPlayCurrentAttackRunnerSelectionWriterResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Runner Selection Writer")
	bool bSuccess = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Runner Selection Writer")
	FMatchPlayCurrentAttackRunnerSelectionRequest Request;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Runner Selection Writer")
	FMatchPlayState BeforeState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Runner Selection Writer")
	FMatchPlayState AfterState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Runner Selection Writer")
	FMatchPlayCurrentAttackRunnerSelectionLegalityResult
		LegalityResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Runner Selection Writer")
	FName SelectedRunnerCardId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Runner Selection Writer")
	EMatchPlayCurrentAttackRunnerSelectionWriterErrorCode ErrorCode =
		EMatchPlayCurrentAttackRunnerSelectionWriterErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Runner Selection Writer")
	FString ErrorMessage;
};

class FMCODEX_API
	FMatchPlayCurrentAttackRunnerSelectionWriter final
{
public:
	static FMatchPlayCurrentAttackRunnerSelectionWriterResult Select(
		const FMatchPlayState& BeforeState,
		const FMatchPlayCurrentAttackRunnerSelectionRequest& Request);
};
