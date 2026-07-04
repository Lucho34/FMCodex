#include "SingleCardFormulaInputAssemblyQuery.h"

namespace SingleCardFormulaInputAssemblyQuery
{
	FSingleCardFormulaInputContract MakeCandidateContract(
		const FSingleCardFormulaInputAssemblyQueryInput& Input)
	{
		FSingleCardFormulaInputContract Contract;
		Contract.CardId = Input.CardId;
		Contract.FormulaType = Input.FormulaType;
		Contract.ParticipantRole = Input.ParticipantRole;
		Contract.Attribute = Input.Attribute;
		Contract.bHasExternalD6ComparePoint =
			Input.bHasExternalD6ComparePoint;
		Contract.ExternalD6ComparePoint =
			Input.ExternalD6ComparePoint;
		Contract.bHasExternalModifier = Input.bHasExternalModifier;
		Contract.ExternalModifier = Input.ExternalModifier;
		Contract.LogId = Input.LogId;
		Contract.TurnIndex = Input.TurnIndex;
		Contract.ContextTag = Input.ContextTag;
		return Contract;
	}

	void SetFailure(
		FSingleCardFormulaInputAssemblyQueryResult& Result,
		const ESingleCardFormulaInputAssemblyQueryErrorCode ErrorCode,
		const FString& ErrorMessage,
		const FName InvalidField)
	{
		Result.ErrorCode = ErrorCode;
		Result.ErrorMessage = ErrorMessage;
		Result.InvalidField = InvalidField;
	}
}

FSingleCardFormulaInputAssemblyQueryResult
FSingleCardFormulaInputAssemblyQuery::Assemble(
	const FPlayerCardRuleSnapshotSet& SnapshotSet,
	const FSingleCardFormulaInputAssemblyQueryInput& Input)
{
	FSingleCardFormulaInputAssemblyQueryResult Result;
	Result.CardId = Input.CardId;

	const FSingleCardFormulaInputContract CandidateContract =
		SingleCardFormulaInputAssemblyQuery::MakeCandidateContract(Input);
	Result.ContractValidationResult =
		FSingleCardFormulaInputContractValidator::Validate(
			CandidateContract);
	if (!Result.ContractValidationResult.bSuccess)
	{
		SingleCardFormulaInputAssemblyQuery::SetFailure(
			Result,
			ESingleCardFormulaInputAssemblyQueryErrorCode
				::ContractValidationFailed,
			Result.ContractValidationResult.ErrorMessage,
			Result.ContractValidationResult.InvalidField);
		return Result;
	}

	Result.SnapshotQueryResult =
		FPlayerCardRuleSnapshotQuery::FindByCardId(
			SnapshotSet,
			Input.CardId);
	if (!Result.SnapshotQueryResult.bSuccess)
	{
		const bool bCardIdFailure =
			Result.SnapshotQueryResult.ErrorCode
				== EPlayerCardRuleSnapshotQueryErrorCode::InvalidCardId
			|| Result.SnapshotQueryResult.ErrorCode
				== EPlayerCardRuleSnapshotQueryErrorCode::CardNotFound;
		SingleCardFormulaInputAssemblyQuery::SetFailure(
			Result,
			ESingleCardFormulaInputAssemblyQueryErrorCode
				::SnapshotQueryFailed,
			Result.SnapshotQueryResult.ErrorMessage,
			bCardIdFailure ? FName(TEXT("CardId")) : NAME_None);
		return Result;
	}

	const bool bSnapshotIsGoalkeeper =
		Result.SnapshotQueryResult.Snapshot.bIsGoalkeeper;
	if (Input.ParticipantRole
			== ESingleCardFormulaParticipantRole::Goalkeeper
		&& !bSnapshotIsGoalkeeper)
	{
		SingleCardFormulaInputAssemblyQuery::SetFailure(
			Result,
			ESingleCardFormulaInputAssemblyQueryErrorCode
				::GoalkeeperRoleRequiresGoalkeeperSnapshot,
			TEXT("Goalkeeper participants require a goalkeeper player-card rule snapshot."),
			TEXT("ParticipantRole"));
		return Result;
	}

	if (Input.ParticipantRole
			!= ESingleCardFormulaParticipantRole::Goalkeeper
		&& bSnapshotIsGoalkeeper)
	{
		SingleCardFormulaInputAssemblyQuery::SetFailure(
			Result,
			ESingleCardFormulaInputAssemblyQueryErrorCode
				::NonGoalkeeperRoleCannotUseGoalkeeperSnapshot,
			TEXT("Attacker and defender participants cannot use a goalkeeper player-card rule snapshot."),
			TEXT("ParticipantRole"));
		return Result;
	}

	Result.bSuccess = true;
	Result.Contract = CandidateContract;
	return Result;
}
