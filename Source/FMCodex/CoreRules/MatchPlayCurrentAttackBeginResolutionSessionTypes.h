#pragma once

#include "CoreMinimal.h"

#include "MatchPlayBoundActionParticipantNormalizationTypes.h"
#include "MatchPlayCurrentAttackResolutionSessionStateValidator.h"

#include "MatchPlayCurrentAttackBeginResolutionSessionTypes.generated.h"

USTRUCT()
struct FMCODEX_API
	FMatchPlayCurrentAttackBeginResolutionSessionRequest
{
	GENERATED_BODY()

	UPROPERTY()
	int64 AttackSequence = 0;
};

UENUM()
enum class EMatchPlayCurrentAttackBeginResolutionSessionErrorCode
	: uint8
{
	None,
	MatchPlayStateNotInitialized,
	NoCurrentAttack,
	InvalidCurrentAttackSequence,
	InvalidRequestedAttackSequence,
	AttackSequenceMismatch,
	CurrentAttackNotInResolution,
	InvalidExistingSessionState,
	WrongSelectionStage,
	ReadyValidationFailed,
	BindingFailed,
	NormalizationFailed,
	InvalidCanonicalBundle
};

USTRUCT()
struct FMCODEX_API
	FMatchPlayCurrentAttackBeginResolutionSessionGlobalContextResult
{
	GENERATED_BODY()

	UPROPERTY()
	bool bSuccess = false;

	UPROPERTY()
	bool bSessionAlreadyExists = false;

	UPROPERTY()
	FMatchPlayCurrentAttackBeginResolutionSessionRequest Request;

	UPROPERTY()
	EMatchPlayCurrentAttackBeginResolutionSessionErrorCode ErrorCode =
		EMatchPlayCurrentAttackBeginResolutionSessionErrorCode::None;

	UPROPERTY()
	FMatchPlayCurrentAttackResolutionSessionStateValidationResult
		SessionStateValidationResult;

	UPROPERTY()
	FMatchPlayCurrentAttackReadyValidationResult ReadyValidationResult;

	UPROPERTY()
	FMatchPlayCurrentAttackResolutionBindingResult BindingResult;

	UPROPERTY()
	FMatchPlayBoundActionParticipantNormalizationResult
		NormalizationResult;

	UPROPERTY()
	FMatchPlayCurrentAttackResolutionSession Session;

	UPROPERTY()
	FString ErrorMessage;
};

USTRUCT()
struct FMCODEX_API
	FMatchPlayCurrentAttackBeginResolutionSessionLegalityResult
{
	GENERATED_BODY()

	UPROPERTY()
	bool bIsLegal = false;

	UPROPERTY()
	bool bSessionAlreadyExists = false;

	UPROPERTY()
	FMatchPlayCurrentAttackBeginResolutionSessionRequest Request;

	UPROPERTY()
	EMatchPlayCurrentAttackBeginResolutionSessionErrorCode ErrorCode =
		EMatchPlayCurrentAttackBeginResolutionSessionErrorCode::None;

	UPROPERTY()
	FMatchPlayCurrentAttackBeginResolutionSessionGlobalContextResult
		GlobalContextResult;

	UPROPERTY()
	FMatchPlayCurrentAttackResolutionSession Session;

	UPROPERTY()
	FString ErrorMessage;
};

USTRUCT()
struct FMCODEX_API
	FMatchPlayCurrentAttackBeginResolutionSessionWriterResult
{
	GENERATED_BODY()

	UPROPERTY()
	bool bSuccess = false;

	UPROPERTY()
	bool bCreatedNewSession = false;

	UPROPERTY()
	FMatchPlayCurrentAttackBeginResolutionSessionRequest Request;

	UPROPERTY()
	FMatchPlayState BeforeState;

	UPROPERTY()
	FMatchPlayState AfterState;

	UPROPERTY()
	EMatchPlayCurrentAttackBeginResolutionSessionErrorCode ErrorCode =
		EMatchPlayCurrentAttackBeginResolutionSessionErrorCode::None;

	UPROPERTY()
	FMatchPlayCurrentAttackBeginResolutionSessionLegalityResult
		LegalityResult;

	UPROPERTY()
	FMatchPlayCurrentAttackResolutionSession Session;

	UPROPERTY()
	FString ErrorMessage;
};
