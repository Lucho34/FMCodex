#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCurrentAttackActionSelectionLegality.h"

#include "MatchPlayCurrentAttackActionSelectionWriter.generated.h"

UENUM(BlueprintType)
enum class EMatchPlayCurrentAttackActionSelectionWriterErrorCode : uint8
{
	None UMETA(DisplayName = "None"),
	LegalityFailed UMETA(DisplayName = "Legality Failed")
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayCurrentAttackActionSelectionWriterResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Action Selection Writer")
	bool bSuccess = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Action Selection Writer")
	FMatchPlayCurrentAttackActionSelectionRequest Request;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Action Selection Writer")
	FMatchPlayState BeforeState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Action Selection Writer")
	FMatchPlayState AfterState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Action Selection Writer")
	EMatchPlayCurrentAttackActionSelectionWriterErrorCode ErrorCode =
		EMatchPlayCurrentAttackActionSelectionWriterErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Action Selection Writer")
	FMatchPlayCurrentAttackActionSelectionLegalityResult LegalityResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Action Selection Writer")
	FMatchPlayCurrentAttackSelectedAction SelectedAction;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Action Selection Writer")
	FString ErrorMessage;
};

class FMCODEX_API FMatchPlayCurrentAttackActionSelectionWriter final
{
public:
	static FMatchPlayCurrentAttackActionSelectionWriterResult Select(
		const FMatchPlayState& BeforeState,
		const FMatchPlayCurrentAttackActionSelectionRequest& Request,
		const FSkillRuleSnapshotSet& SkillRules);
};
