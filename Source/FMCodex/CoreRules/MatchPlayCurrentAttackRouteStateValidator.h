#pragma once

#include "CoreMinimal.h"

#include "MatchPlayState.h"

#include "MatchPlayCurrentAttackRouteStateValidator.generated.h"

UENUM(BlueprintType)
enum class EMatchPlayCurrentAttackRouteStateValidationErrorCode : uint8
{
	None UMETA(DisplayName = "None"),
	InactiveStateHasPayload UMETA(DisplayName = "Inactive State Has Payload"),
	InvalidAttackSequence UMETA(DisplayName = "Invalid Attack Sequence"),
	AttackSequenceMismatch UMETA(DisplayName = "Attack Sequence Mismatch"),
	InvalidRawInitialD12 UMETA(DisplayName = "Invalid Raw Initial D12"),
	ActionPointMismatch UMETA(DisplayName = "Action Point Mismatch"),
	InvalidRouteKind UMETA(DisplayName = "Invalid Route Kind"),
	OrdinaryD12Mismatch UMETA(DisplayName = "Ordinary D12 Mismatch"),
	SendingOffD12Mismatch UMETA(DisplayName = "Sending Off D12 Mismatch"),
	SetPieceD12Mismatch UMETA(DisplayName = "Set Piece D12 Mismatch"),
	WrongRoutePhase UMETA(DisplayName = "Wrong Route Phase"),
	WrongRouteLifecycle UMETA(DisplayName = "Wrong Route Lifecycle"),
	InvalidSendingOffStage UMETA(DisplayName = "Invalid Sending Off Stage"),
	InvalidSendingOffPayload UMETA(DisplayName = "Invalid Sending Off Payload"),
	SendingOffEjectionMismatch UMETA(DisplayName = "Sending Off Ejection Mismatch"),
	InvalidSetPieceStage UMETA(DisplayName = "Invalid Set Piece Stage"),
	UnexpectedSendingOffPayload UMETA(DisplayName = "Unexpected Sending Off Payload"),
	UnexpectedSetPiecePayload UMETA(DisplayName = "Unexpected Set Piece Payload"),
	UnexpectedOrdinaryPayload UMETA(DisplayName = "Unexpected Ordinary Payload"),
	AwaitingTypeHasResolvedPayload UMETA(DisplayName = "Awaiting Type Has Resolved Payload"),
	AwaitingTypeHasConcretePayload UMETA(DisplayName = "Awaiting Type Has Concrete Payload"),
	TypeResolvedMissingRoll UMETA(DisplayName = "Type Resolved Missing Roll"),
	InvalidSetPieceTypeRoll UMETA(DisplayName = "Invalid Set Piece Type Roll"),
	InvalidSetPieceType UMETA(DisplayName = "Invalid Set Piece Type"),
	SetPieceTypeMappingMismatch UMETA(DisplayName = "Set Piece Type Mapping Mismatch"),
	ConcretePayloadCountMismatch UMETA(DisplayName = "Concrete Payload Count Mismatch"),
	ConcretePayloadTypeMismatch UMETA(DisplayName = "Concrete Payload Type Mismatch"),
	InvalidConcreteSetPieceStage UMETA(DisplayName = "Invalid Concrete Set Piece Stage"),
	AwaitingCarrierHasBoundCarrier UMETA(DisplayName = "Awaiting Carrier Has Bound Carrier"),
	AwaitingMethodMissingCarrier UMETA(DisplayName = "Awaiting Method Missing Carrier"),
	CarrierOwnerSideMismatch UMETA(DisplayName = "Carrier Owner Side Mismatch"),
	CarrierEligibilityFailed UMETA(DisplayName = "Carrier Eligibility Failed"),
	CarrierSnapshotBindingMismatch UMETA(DisplayName = "Carrier Snapshot Binding Mismatch"),
	InvalidCornerNominationState UMETA(DisplayName = "Invalid Corner Nomination State"),
	CornerNomineeEligibilityFailed UMETA(DisplayName = "Corner Nominee Eligibility Failed"),
	CornerParticipantMappingMismatch UMETA(DisplayName = "Corner Participant Mapping Mismatch"),
	CornerFormulaMismatch UMETA(DisplayName = "Corner Formula Mismatch")
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayCurrentAttackRouteStateValidationResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Route")
	bool bIsCanonical = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Route")
	EMatchPlayCurrentAttackRouteStateValidationErrorCode ErrorCode =
		EMatchPlayCurrentAttackRouteStateValidationErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Route")
	FString ErrorMessage;
};

class FMCODEX_API FMatchPlayCurrentAttackRouteStateValidator final
{
public:
	static FMatchPlayCurrentAttackRouteStateValidationResult Validate(
		const FMatchPlayState& State);
};
