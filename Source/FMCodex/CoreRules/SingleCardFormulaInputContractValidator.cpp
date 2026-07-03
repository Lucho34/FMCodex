#include "SingleCardFormulaInputContractValidator.h"

namespace SingleCardFormulaInputContractValidator
{
	void SetFailure(
		FSingleCardFormulaInputContractValidationResult& Result,
		const ESingleCardFormulaInputContractValidationErrorCode ErrorCode,
		const FString& ErrorMessage,
		const FName InvalidCardId,
		const FName InvalidField)
	{
		Result.ErrorCode = ErrorCode;
		Result.ErrorMessage = ErrorMessage;
		Result.InvalidCardId = InvalidCardId;
		Result.InvalidField = InvalidField;
	}

	bool IsSupportedFormulaType(const EFormulaType FormulaType)
	{
		return FormulaType == EFormulaType::Transition
			|| FormulaType == EFormulaType::Finishing;
	}

	bool IsValidParticipantRole(
		const ESingleCardFormulaParticipantRole ParticipantRole)
	{
		switch (ParticipantRole)
		{
		case ESingleCardFormulaParticipantRole::Attacker:
		case ESingleCardFormulaParticipantRole::Defender:
		case ESingleCardFormulaParticipantRole::Goalkeeper:
			return true;
		default:
			return false;
		}
	}

	bool IsValidAttribute(const ESingleCardFormulaAttribute Attribute)
	{
		switch (Attribute)
		{
		case ESingleCardFormulaAttribute::Shooting:
		case ESingleCardFormulaAttribute::Dribbling:
		case ESingleCardFormulaAttribute::Passing:
		case ESingleCardFormulaAttribute::OffBall:
		case ESingleCardFormulaAttribute::Marking:
		case ESingleCardFormulaAttribute::Tackling:
		case ESingleCardFormulaAttribute::Speed:
		case ESingleCardFormulaAttribute::Strength:
		case ESingleCardFormulaAttribute::Stamina:
		case ESingleCardFormulaAttribute::LongShot:
		case ESingleCardFormulaAttribute::GoalkeeperHandling:
		case ESingleCardFormulaAttribute::GoalkeeperPositioning:
		case ESingleCardFormulaAttribute::GoalkeeperReflex:
		case ESingleCardFormulaAttribute::GoalkeeperAerial:
		case ESingleCardFormulaAttribute::GoalkeeperAnticipation:
		case ESingleCardFormulaAttribute::GoalkeeperOneOnOne:
			return true;
		default:
			return false;
		}
	}

	bool IsGoalkeeperAttribute(
		const ESingleCardFormulaAttribute Attribute)
	{
		switch (Attribute)
		{
		case ESingleCardFormulaAttribute::GoalkeeperHandling:
		case ESingleCardFormulaAttribute::GoalkeeperPositioning:
		case ESingleCardFormulaAttribute::GoalkeeperReflex:
		case ESingleCardFormulaAttribute::GoalkeeperAerial:
		case ESingleCardFormulaAttribute::GoalkeeperAnticipation:
		case ESingleCardFormulaAttribute::GoalkeeperOneOnOne:
			return true;
		default:
			return false;
		}
	}
}

FSingleCardFormulaInputContractValidationResult
FSingleCardFormulaInputContractValidator::Validate(
	const FSingleCardFormulaInputContract& Contract)
{
	FSingleCardFormulaInputContractValidationResult Result;

	if (Contract.CardId.IsNone())
	{
		SingleCardFormulaInputContractValidator::SetFailure(
			Result,
			ESingleCardFormulaInputContractValidationErrorCode
				::InvalidCardId,
			TEXT("Single-card formula input contract requires a non-empty CardId."),
			Contract.CardId,
			TEXT("CardId"));
		return Result;
	}

	if (!SingleCardFormulaInputContractValidator::IsSupportedFormulaType(
			Contract.FormulaType))
	{
		SingleCardFormulaInputContractValidator::SetFailure(
			Result,
			ESingleCardFormulaInputContractValidationErrorCode
				::UnsupportedFormulaType,
			TEXT("FormulaType must be Transition or Finishing."),
			Contract.CardId,
			TEXT("FormulaType"));
		return Result;
	}

	if (!SingleCardFormulaInputContractValidator::IsValidParticipantRole(
			Contract.ParticipantRole))
	{
		SingleCardFormulaInputContractValidator::SetFailure(
			Result,
			ESingleCardFormulaInputContractValidationErrorCode
				::InvalidParticipantRole,
			TEXT("ParticipantRole must be Attacker, Defender, or Goalkeeper."),
			Contract.CardId,
			TEXT("ParticipantRole"));
		return Result;
	}

	if (!SingleCardFormulaInputContractValidator::IsValidAttribute(
			Contract.Attribute))
	{
		SingleCardFormulaInputContractValidator::SetFailure(
			Result,
			ESingleCardFormulaInputContractValidationErrorCode
				::InvalidAttribute,
			TEXT("Attribute must identify a supported player-card rule attribute."),
			Contract.CardId,
			TEXT("Attribute"));
		return Result;
	}

	const bool bGoalkeeperAttribute =
		SingleCardFormulaInputContractValidator::IsGoalkeeperAttribute(
			Contract.Attribute);
	if (Contract.ParticipantRole
			== ESingleCardFormulaParticipantRole::Goalkeeper
		&& !bGoalkeeperAttribute)
	{
		SingleCardFormulaInputContractValidator::SetFailure(
			Result,
			ESingleCardFormulaInputContractValidationErrorCode
				::GoalkeeperRoleRequiresGoalkeeperAttribute,
			TEXT("Goalkeeper participants must select a goalkeeper attribute."),
			Contract.CardId,
			TEXT("Attribute"));
		return Result;
	}

	if (Contract.ParticipantRole
			!= ESingleCardFormulaParticipantRole::Goalkeeper
		&& bGoalkeeperAttribute)
	{
		SingleCardFormulaInputContractValidator::SetFailure(
			Result,
			ESingleCardFormulaInputContractValidationErrorCode
				::NonGoalkeeperRoleCannotUseGoalkeeperAttribute,
			TEXT("Non-goalkeeper participants cannot select a goalkeeper attribute."),
			Contract.CardId,
			TEXT("Attribute"));
		return Result;
	}

	if (!Contract.bHasExternalD6ComparePoint)
	{
		SingleCardFormulaInputContractValidator::SetFailure(
			Result,
			ESingleCardFormulaInputContractValidationErrorCode
				::MissingExternalD6ComparePoint,
			TEXT("An externally supplied D6 compare point is required."),
			Contract.CardId,
			TEXT("ExternalD6ComparePoint"));
		return Result;
	}

	if (Contract.ExternalD6ComparePoint < MinD6ComparePoint
		|| Contract.ExternalD6ComparePoint > MaxD6ComparePoint)
	{
		SingleCardFormulaInputContractValidator::SetFailure(
			Result,
			ESingleCardFormulaInputContractValidationErrorCode
				::D6ComparePointOutOfRange,
			FString::Printf(
				TEXT("ExternalD6ComparePoint must be in range [%d, %d]."),
				MinD6ComparePoint,
				MaxD6ComparePoint),
			Contract.CardId,
			TEXT("ExternalD6ComparePoint"));
		return Result;
	}

	if (!Contract.bHasExternalModifier)
	{
		SingleCardFormulaInputContractValidator::SetFailure(
			Result,
			ESingleCardFormulaInputContractValidationErrorCode
				::MissingExternalModifier,
			TEXT("An externally supplied Modifier is required."),
			Contract.CardId,
			TEXT("ExternalModifier"));
		return Result;
	}

	if (!FMath::IsFinite(Contract.ExternalModifier))
	{
		SingleCardFormulaInputContractValidator::SetFailure(
			Result,
			ESingleCardFormulaInputContractValidationErrorCode
				::InvalidModifier,
			TEXT("ExternalModifier must be a finite value."),
			Contract.CardId,
			TEXT("ExternalModifier"));
		return Result;
	}

	if (Contract.TurnIndex < 0)
	{
		SingleCardFormulaInputContractValidator::SetFailure(
			Result,
			ESingleCardFormulaInputContractValidationErrorCode
				::InvalidLogContext,
			TEXT("TurnIndex cannot be negative."),
			Contract.CardId,
			TEXT("TurnIndex"));
		return Result;
	}

	Result.bSuccess = true;
	Result.bIsValid = true;
	return Result;
}
