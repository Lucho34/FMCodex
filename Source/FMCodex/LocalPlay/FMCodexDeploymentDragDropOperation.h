#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"

#include "FMCodexLocalMatchUMGPresentation.h"

#include "FMCodexDeploymentDragDropOperation.generated.h"

/**
 * Presentation-only payload for a deployment drag. The authoritative command
 * is emitted only after a valid pitch slot accepts the drop.
 */
UCLASS(BlueprintType)
class FMCODEX_API UFMCodexDeploymentDragDropOperation
	: public UDragDropOperation
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Local Match|Deployment Drag")
	FName CardId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Local Match|Deployment Drag")
	bool bGoalkeeper = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Local Match|Deployment Drag")
	FFMCodexUMGCardViewModel CardPresentation;
};
