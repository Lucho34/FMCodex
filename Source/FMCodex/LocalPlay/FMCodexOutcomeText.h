#pragma once

#include "CoreMinimal.h"
#include "FMCodexOutcomeText.generated.h"

/** NoGoal means the attacking attempt ended without scoring, not every non-Goal event. */
UENUM()
enum class EFMCodexOutcomeAccent : uint8 { Neutral, Goal, NoGoal };

/** Authored alongside the canonical sentence, never extracted from that sentence. */
USTRUCT()
struct FMCODEX_API FFMCodexOutcomeText
{
	GENERATED_BODY()

	UPROPERTY() FText Prefix;
	UPROPERTY() FText Keyword;
	UPROPERTY() FText Suffix;
	UPROPERTY() EFMCodexOutcomeAccent Accent = EFMCodexOutcomeAccent::Neutral;

	FFMCodexOutcomeText() = default;
	FFMCodexOutcomeText(const FText& InPrefix, const FText& InKeyword,
		const FText& InSuffix, EFMCodexOutcomeAccent InAccent)
		: Prefix(InPrefix), Keyword(InKeyword), Suffix(InSuffix), Accent(InAccent) {}

	FText ToText() const
	{
		return FText::Format(NSLOCTEXT("FMCodexOutcome", "Sentence", "{0}{1}{2}"), Prefix, Keyword, Suffix);
	}
};

namespace FMCodexOutcomeText
{
	inline FText PairedRollDetail(int32 First, int32 Second)
	{
		// Same disclosed pair and display sum as PairedRollResult; no RNG or outcome evaluation.
		return FText::Format(NSLOCTEXT("FMCodexOutcome", "PairDetail", "首次掷点 {0} + 第二次掷点 {1} = {2}"),
			FText::AsNumber(First), FText::AsNumber(Second), FText::AsNumber(First + Second));
	}
}
