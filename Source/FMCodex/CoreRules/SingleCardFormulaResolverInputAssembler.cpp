#include "SingleCardFormulaResolverInputAssembler.h"

namespace SingleCardFormulaResolverInputAssembler
{
	const FName AttackerSide(TEXT("Attacker"));
	const FName DefenderSide(TEXT("Defender"));

	void SetFailure(
		FSingleCardFormulaResolverInputAssemblyResult& Result,
		const ESingleCardFormulaResolverInputAssemblyErrorCode ErrorCode,
		const FString& ErrorMessage,
		const FName InvalidSide,
		const FName InvalidField)
	{
		Result.ErrorCode = ErrorCode;
		Result.ErrorMessage = ErrorMessage;
		Result.InvalidSide = InvalidSide;
		Result.InvalidField = InvalidField;
	}

	bool IsSupportedFormulaType(const EFormulaType FormulaType)
	{
		return FormulaType == EFormulaType::Transition
			|| FormulaType == EFormulaType::Finishing;
	}

	bool HasValidNestedResults(
		const FSingleCardFormulaInputAssemblyQueryResult& QueryResult)
	{
		return QueryResult.ContractValidationResult.bSuccess
			&& QueryResult.SnapshotQueryResult.bSuccess
			&& QueryResult.SnapshotQueryResult.bFound;
	}

	bool HasMatchingCardIds(
		const FSingleCardFormulaInputAssemblyQueryResult& QueryResult)
	{
		const FName CardId = QueryResult.Contract.CardId;
		return !CardId.IsNone()
			&& QueryResult.CardId == CardId
			&& QueryResult.SnapshotQueryResult.CardId == CardId
			&& QueryResult.SnapshotQueryResult.Snapshot.CardId == CardId;
	}

	bool TryGetAttributeValue(
		const ESingleCardFormulaAttribute Attribute,
		const FPlayerCardRuleSnapshot& Snapshot,
		float& OutValue)
	{
		switch (Attribute)
		{
		case ESingleCardFormulaAttribute::Shooting:
			OutValue = Snapshot.Attributes.Shooting;
			return true;
		case ESingleCardFormulaAttribute::Dribbling:
			OutValue = Snapshot.Attributes.Dribbling;
			return true;
		case ESingleCardFormulaAttribute::Passing:
			OutValue = Snapshot.Attributes.Passing;
			return true;
		case ESingleCardFormulaAttribute::OffBall:
			OutValue = Snapshot.Attributes.OffBall;
			return true;
		case ESingleCardFormulaAttribute::Marking:
			OutValue = Snapshot.Attributes.Marking;
			return true;
		case ESingleCardFormulaAttribute::Tackling:
			OutValue = Snapshot.Attributes.Tackling;
			return true;
		case ESingleCardFormulaAttribute::Speed:
			OutValue = Snapshot.Attributes.Speed;
			return true;
		case ESingleCardFormulaAttribute::Strength:
			OutValue = Snapshot.Attributes.Strength;
			return true;
		case ESingleCardFormulaAttribute::Stamina:
			OutValue = Snapshot.Attributes.Stamina;
			return true;
		case ESingleCardFormulaAttribute::LongShot:
			OutValue = Snapshot.Attributes.LongShot;
			return true;
		case ESingleCardFormulaAttribute::GoalkeeperHandling:
			OutValue = Snapshot.GoalkeeperAttributes.Handling;
			return true;
		case ESingleCardFormulaAttribute::GoalkeeperPositioning:
			OutValue = Snapshot.GoalkeeperAttributes.Positioning;
			return true;
		case ESingleCardFormulaAttribute::GoalkeeperReflex:
			OutValue = Snapshot.GoalkeeperAttributes.Reflex;
			return true;
		case ESingleCardFormulaAttribute::GoalkeeperAerial:
			OutValue = Snapshot.GoalkeeperAttributes.Aerial;
			return true;
		case ESingleCardFormulaAttribute::GoalkeeperAnticipation:
			OutValue = Snapshot.GoalkeeperAttributes.Anticipation;
			return true;
		case ESingleCardFormulaAttribute::GoalkeeperOneOnOne:
			OutValue = Snapshot.GoalkeeperAttributes.OneOnOne;
			return true;
		default:
			return false;
		}
	}

	FFormulaSideInput MakeSideInput(
		const FSingleCardFormulaInputContract& Contract,
		const FPlayerCardRuleSnapshot& Snapshot,
		const float BaseValue)
	{
		FFormulaSideInput SideInput;
		SideInput.BaseValue = BaseValue;
		SideInput.Modifier = Contract.ExternalModifier;
		SideInput.ComparePoint = Contract.ExternalD6ComparePoint;
		SideInput.bComparePointWasRolledOnD6 =
			Contract.bHasExternalD6ComparePoint;
		SideInput.ParticipatingStamina.Add(
			Snapshot.Attributes.Stamina);
		return SideInput;
	}
}

FSingleCardFormulaResolverInputAssemblyResult
FSingleCardFormulaResolverInputAssembler::Assemble(
	const FSingleCardFormulaResolverInputAssemblyInput& Input)
{
	FSingleCardFormulaResolverInputAssemblyResult Result;
	Result.AttackerQueryResult = Input.AttackerQueryResult;
	Result.DefenderQueryResult = Input.DefenderQueryResult;

	if (!Input.AttackerQueryResult.bSuccess)
	{
		SingleCardFormulaResolverInputAssembler::SetFailure(
			Result,
			ESingleCardFormulaResolverInputAssemblyErrorCode
				::AttackerQueryFailed,
			Input.AttackerQueryResult.ErrorMessage,
			SingleCardFormulaResolverInputAssembler::AttackerSide,
			Input.AttackerQueryResult.InvalidField);
		return Result;
	}

	if (!Input.DefenderQueryResult.bSuccess)
	{
		SingleCardFormulaResolverInputAssembler::SetFailure(
			Result,
			ESingleCardFormulaResolverInputAssemblyErrorCode
				::DefenderQueryFailed,
			Input.DefenderQueryResult.ErrorMessage,
			SingleCardFormulaResolverInputAssembler::DefenderSide,
			Input.DefenderQueryResult.InvalidField);
		return Result;
	}

	if (!SingleCardFormulaResolverInputAssembler::HasValidNestedResults(
			Input.AttackerQueryResult))
	{
		SingleCardFormulaResolverInputAssembler::SetFailure(
			Result,
			ESingleCardFormulaResolverInputAssemblyErrorCode
				::InvalidAttackerQueryResult,
			TEXT("Attacker query result must retain successful contract validation and snapshot lookup results."),
			SingleCardFormulaResolverInputAssembler::AttackerSide,
			TEXT("AttackerQueryResult"));
		return Result;
	}

	if (!SingleCardFormulaResolverInputAssembler::HasValidNestedResults(
			Input.DefenderQueryResult))
	{
		SingleCardFormulaResolverInputAssembler::SetFailure(
			Result,
			ESingleCardFormulaResolverInputAssemblyErrorCode
				::InvalidDefenderQueryResult,
			TEXT("Defender query result must retain successful contract validation and snapshot lookup results."),
			SingleCardFormulaResolverInputAssembler::DefenderSide,
			TEXT("DefenderQueryResult"));
		return Result;
	}

	if (!SingleCardFormulaResolverInputAssembler::HasMatchingCardIds(
			Input.AttackerQueryResult))
	{
		SingleCardFormulaResolverInputAssembler::SetFailure(
			Result,
			ESingleCardFormulaResolverInputAssemblyErrorCode
				::AttackerCardIdMismatch,
			TEXT("Attacker query result CardId values must match."),
			SingleCardFormulaResolverInputAssembler::AttackerSide,
			TEXT("CardId"));
		return Result;
	}

	if (!SingleCardFormulaResolverInputAssembler::HasMatchingCardIds(
			Input.DefenderQueryResult))
	{
		SingleCardFormulaResolverInputAssembler::SetFailure(
			Result,
			ESingleCardFormulaResolverInputAssemblyErrorCode
				::DefenderCardIdMismatch,
			TEXT("Defender query result CardId values must match."),
			SingleCardFormulaResolverInputAssembler::DefenderSide,
			TEXT("CardId"));
		return Result;
	}

	const FSingleCardFormulaInputContract& AttackerContract =
		Input.AttackerQueryResult.Contract;
	const FSingleCardFormulaInputContract& DefenderContract =
		Input.DefenderQueryResult.Contract;

	if (AttackerContract.ParticipantRole
		!= ESingleCardFormulaParticipantRole::Attacker)
	{
		SingleCardFormulaResolverInputAssembler::SetFailure(
			Result,
			ESingleCardFormulaResolverInputAssemblyErrorCode
				::InvalidAttackerRole,
			TEXT("Attacker-side contract must use the Attacker participant role."),
			SingleCardFormulaResolverInputAssembler::AttackerSide,
			TEXT("ParticipantRole"));
		return Result;
	}

	if (DefenderContract.ParticipantRole
			!= ESingleCardFormulaParticipantRole::Defender
		&& DefenderContract.ParticipantRole
			!= ESingleCardFormulaParticipantRole::Goalkeeper)
	{
		SingleCardFormulaResolverInputAssembler::SetFailure(
			Result,
			ESingleCardFormulaResolverInputAssemblyErrorCode
				::InvalidDefenderRole,
			TEXT("Defender-side contract must use the Defender or Goalkeeper participant role."),
			SingleCardFormulaResolverInputAssembler::DefenderSide,
			TEXT("ParticipantRole"));
		return Result;
	}

	if (!SingleCardFormulaResolverInputAssembler::IsSupportedFormulaType(
			AttackerContract.FormulaType)
		|| !SingleCardFormulaResolverInputAssembler::IsSupportedFormulaType(
			DefenderContract.FormulaType))
	{
		SingleCardFormulaResolverInputAssembler::SetFailure(
			Result,
			ESingleCardFormulaResolverInputAssemblyErrorCode
				::UnsupportedFormulaType,
			TEXT("Both contracts must use Transition or Finishing."),
			NAME_None,
			TEXT("FormulaType"));
		return Result;
	}

	if (AttackerContract.FormulaType != DefenderContract.FormulaType)
	{
		SingleCardFormulaResolverInputAssembler::SetFailure(
			Result,
			ESingleCardFormulaResolverInputAssemblyErrorCode
				::FormulaTypeMismatch,
			TEXT("Attacker and defender contracts must use the same FormulaType."),
			NAME_None,
			TEXT("FormulaType"));
		return Result;
	}

	if (AttackerContract.LogId != DefenderContract.LogId)
	{
		SingleCardFormulaResolverInputAssembler::SetFailure(
			Result,
			ESingleCardFormulaResolverInputAssemblyErrorCode
				::LogIdMismatch,
			TEXT("Attacker and defender contracts must use the same LogId."),
			NAME_None,
			TEXT("LogId"));
		return Result;
	}

	if (AttackerContract.TurnIndex != DefenderContract.TurnIndex)
	{
		SingleCardFormulaResolverInputAssembler::SetFailure(
			Result,
			ESingleCardFormulaResolverInputAssemblyErrorCode
				::TurnIndexMismatch,
			TEXT("Attacker and defender contracts must use the same TurnIndex."),
			NAME_None,
			TEXT("TurnIndex"));
		return Result;
	}

	float AttackerBaseValue = 0.0f;
	if (!SingleCardFormulaResolverInputAssembler::TryGetAttributeValue(
			AttackerContract.Attribute,
			Input.AttackerQueryResult.SnapshotQueryResult.Snapshot,
			AttackerBaseValue))
	{
		SingleCardFormulaResolverInputAssembler::SetFailure(
			Result,
			ESingleCardFormulaResolverInputAssemblyErrorCode
				::InvalidAttackerAttribute,
			TEXT("Attacker contract must select a supported direct attribute."),
			SingleCardFormulaResolverInputAssembler::AttackerSide,
			TEXT("Attribute"));
		return Result;
	}

	float DefenderBaseValue = 0.0f;
	if (!SingleCardFormulaResolverInputAssembler::TryGetAttributeValue(
			DefenderContract.Attribute,
			Input.DefenderQueryResult.SnapshotQueryResult.Snapshot,
			DefenderBaseValue))
	{
		SingleCardFormulaResolverInputAssembler::SetFailure(
			Result,
			ESingleCardFormulaResolverInputAssemblyErrorCode
				::InvalidDefenderAttribute,
			TEXT("Defender contract must select a supported direct attribute."),
			SingleCardFormulaResolverInputAssembler::DefenderSide,
			TEXT("Attribute"));
		return Result;
	}

	FFormulaResolverInput ResolverInput;
	ResolverInput.FormulaType = AttackerContract.FormulaType;
	ResolverInput.Attacker =
		SingleCardFormulaResolverInputAssembler::MakeSideInput(
			AttackerContract,
			Input.AttackerQueryResult.SnapshotQueryResult.Snapshot,
			AttackerBaseValue);
	ResolverInput.Defender =
		SingleCardFormulaResolverInputAssembler::MakeSideInput(
			DefenderContract,
			Input.DefenderQueryResult.SnapshotQueryResult.Snapshot,
			DefenderBaseValue);
	ResolverInput.bGoalkeeperParticipated =
		DefenderContract.ParticipantRole
			== ESingleCardFormulaParticipantRole::Goalkeeper;
	ResolverInput.LogId = AttackerContract.LogId;
	ResolverInput.TurnIndex = AttackerContract.TurnIndex;
	ResolverInput.AttackerPlayerId = Input.AttackerPlayerId;
	ResolverInput.DefenderPlayerId = Input.DefenderPlayerId;
	ResolverInput.InvolvedCardIds =
	{
		AttackerContract.CardId,
		DefenderContract.CardId
	};

	Result.bSuccess = true;
	Result.ResolverInput = ResolverInput;
	return Result;
}
