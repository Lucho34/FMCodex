#include "MatchPlayCurrentAttackSkillSelectionLegality.h"

namespace MatchPlayCurrentAttackSkillSelectionLegalityImplementation
{
	void SetError(
		FMatchPlayCurrentAttackSkillSelectionLegalityResult& Result,
		const EMatchPlayCurrentAttackSkillSelectionErrorCode ErrorCode,
		const FString& ErrorMessage)
	{
		Result.ErrorCode = ErrorCode;
		Result.ErrorMessage = ErrorMessage;
	}

	void CopyGlobalDiagnostics(
		FMatchPlayCurrentAttackSkillSelectionLegalityResult& Result)
	{
		const FMatchPlayCurrentAttackSkillSelectionGlobalContextResult&
			Context = Result.GlobalContextResult;
		Result.SelectionStateValidationResult =
			Context.SelectionStateValidationResult;
		Result.FrozenCarrierCardId = Context.FrozenCarrierCardId;
		Result.FrozenMarkerCardId = Context.FrozenMarkerCardId;
		Result.MatchingFrozenCarrierPlacementCount =
			Context.MatchingFrozenCarrierPlacementCount;
		Result.MatchingFrozenMarkerPlacementCount =
			Context.MatchingFrozenMarkerPlacementCount;
		Result.FrozenCarrierPlacement =
			Context.FrozenCarrierPlacement;
		Result.FrozenMarkerPlacement =
			Context.FrozenMarkerPlacement;
		Result.CarrierSnapshotQueryResult =
			Context.CarrierSnapshotQueryResult;
	}
}

FMatchPlayCurrentAttackSkillSelectionLegalityResult
FMatchPlayCurrentAttackSkillSelectionLegalityEvaluator::Evaluate(
	const FMatchPlayState& BeforeState,
	const FSkillRuleSnapshotSet& SkillRuleSet,
	const FMatchPlayCurrentAttackSkillSelectionRequest& Request)
{
	using namespace
		MatchPlayCurrentAttackSkillSelectionLegalityImplementation;

	FMatchPlayCurrentAttackSkillSelectionLegalityResult Result;
	Result.Request = Request;
	Result.GlobalContextResult =
		FMatchPlayCurrentAttackSkillSelectionGlobalContextQuery::Query(
			BeforeState,
			Request.AttackSequence,
			Request.RequestingSide,
			SkillRuleSet);
	CopyGlobalDiagnostics(Result);
	if (!Result.GlobalContextResult.bSuccess)
	{
		SetError(
			Result,
			Result.GlobalContextResult.ErrorCode,
			Result.GlobalContextResult.ErrorMessage);
		return Result;
	}

	if (Request.SkillId.IsNone())
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackSkillSelectionErrorCode
				::InvalidSkillId,
			TEXT("SkillId must be non-empty."));
		return Result;
	}

	if (!Result.GlobalContextResult.ResolvedCarrierSnapshot
			.SkillIds.Contains(Request.SkillId))
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackSkillSelectionErrorCode
				::CarrierDoesNotOwnSkill,
			TEXT("The frozen carrier does not own the requested skill."));
		return Result;
	}

	FSkillRuleSnapshotQueryInput SkillRuleQueryInput;
	SkillRuleQueryInput.SkillId = Request.SkillId;
	Result.SkillRuleQueryResult =
		FSkillRuleSnapshotQuery::FindBySkillId(
			SkillRuleSet,
			SkillRuleQueryInput);
	if (!Result.SkillRuleQueryResult.bSuccess)
	{
		EMatchPlayCurrentAttackSkillSelectionErrorCode ErrorCode =
			EMatchPlayCurrentAttackSkillSelectionErrorCode
				::InvalidSkillRuleSet;
		if (Result.SkillRuleQueryResult.ErrorCode
			== ESkillRuleSnapshotQueryErrorCode::SkillRuleNotFound)
		{
			ErrorCode =
				EMatchPlayCurrentAttackSkillSelectionErrorCode
					::SkillRuleNotFound;
		}
		else if (
			Result.SkillRuleQueryResult.ErrorCode
				== ESkillRuleSnapshotQueryErrorCode
					::SnapshotSetValidationFailed
			&& Result.SkillRuleQueryResult.ValidationResult.ErrorCode
				== ESkillRuleSnapshotValidationErrorCode
					::DuplicateSkillId)
		{
			ErrorCode =
				EMatchPlayCurrentAttackSkillSelectionErrorCode
					::SkillRuleAmbiguous;
		}
		else if (
			Result.SkillRuleQueryResult.ErrorCode
				== ESkillRuleSnapshotQueryErrorCode
					::SnapshotSetValidationFailed
			&& Result.SkillRuleQueryResult.ValidationResult.InvalidSkillId
				== Request.SkillId
			&& (Result.SkillRuleQueryResult.ValidationResult.ErrorCode
					== ESkillRuleSnapshotValidationErrorCode
						::InvalidSkillType
				|| Result.SkillRuleQueryResult.ValidationResult.ErrorCode
					== ESkillRuleSnapshotValidationErrorCode
						::UnsupportedSkillType))
		{
			ErrorCode =
				EMatchPlayCurrentAttackSkillSelectionErrorCode
					::UnsupportedSkillRuleType;
		}

		SetError(
			Result,
			ErrorCode,
			Result.SkillRuleQueryResult.ErrorMessage);
		return Result;
	}

	Result.ResolvedSkillRule =
		Result.SkillRuleQueryResult.Snapshot;
	Result.ResolvedActionType =
		Result.ResolvedSkillRule.SkillType;
	Result.ParticipantRequirementResult =
		FMatchPlaySkillParticipantRequirementQuery::Query(
			Result.ResolvedActionType);
	if (!Result.ParticipantRequirementResult.bSuccess)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackSkillSelectionErrorCode
				::ParticipantRequirementResolutionFailed,
			Result.ParticipantRequirementResult.ErrorMessage);
		return Result;
	}

	const int32 ValidatedActionPoint =
		Result.GlobalContextResult.ValidatedActionPoint;
	if (ValidatedActionPoint
			< Result.ResolvedSkillRule.MinTriggerActionPoint
		|| ValidatedActionPoint
			> Result.ResolvedSkillRule.MaxTriggerActionPoint)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackSkillSelectionErrorCode
				::ActionPointOutsideSkillRange,
			TEXT("Current attack ActionPoint is outside the selected skill trigger range."));
		return Result;
	}

	Result.bIsLegal = true;
	Result.ErrorCode =
		EMatchPlayCurrentAttackSkillSelectionErrorCode::None;
	Result.ErrorMessage.Empty();
	return Result;
}
