#include "MatchPlayCurrentAttackInitialRouteMappingQuery.h"

#include "CrossSelectionQuery.h"
#include "PassControlAdvanceSelectionQuery.h"
#include "ThroughBallBranchSelectionQuery.h"

namespace MatchPlayCurrentAttackInitialRouteMappingQuery
{
	void SetFailure(
		FMatchPlayCurrentAttackInitialRouteMappingResult& Result,
		const EMatchPlayCurrentAttackInitialRouteMappingErrorCode ErrorCode,
		const FString& ErrorMessage,
		const FName InvalidField)
	{
		Result.ErrorCode = ErrorCode;
		Result.ErrorMessage = ErrorMessage;
		Result.InvalidField = InvalidField;
	}

	bool ValidateNoInitialRouteD6(
		FMatchPlayCurrentAttackInitialRouteMappingResult& Result)
	{
		if (!Result.Input.bHasInitialRouteD6
			&& Result.Input.InitialRouteD6 == 0)
		{
			return true;
		}

		SetFailure(
			Result,
			EMatchPlayCurrentAttackInitialRouteMappingErrorCode
				::UnexpectedInitialRouteD6,
			TEXT("This deterministic initial route must not contain a D6."),
			TEXT("InitialRouteD6"));
		return false;
	}

	bool ValidateInitialRouteD6(
		FMatchPlayCurrentAttackInitialRouteMappingResult& Result)
	{
		if (!Result.Input.bHasInitialRouteD6)
		{
			SetFailure(
				Result,
				EMatchPlayCurrentAttackInitialRouteMappingErrorCode
					::MissingInitialRouteD6,
				TEXT("This initial route requires one D6."),
				TEXT("InitialRouteD6"));
			return false;
		}

		if (Result.Input.InitialRouteD6 < 1
			|| Result.Input.InitialRouteD6 > 6)
		{
			SetFailure(
				Result,
				EMatchPlayCurrentAttackInitialRouteMappingErrorCode
					::InvalidInitialRouteD6,
				TEXT("InitialRouteD6 must be in range [1, 6]."),
				TEXT("InitialRouteD6"));
			return false;
		}

		return true;
	}

	bool ValidateNoIntent(
		FMatchPlayCurrentAttackInitialRouteMappingResult& Result)
	{
		if (Result.Input.Intent == EMatchPlayElectiveBranchIntent::None)
		{
			return true;
		}

		SetFailure(
			Result,
			EMatchPlayCurrentAttackInitialRouteMappingErrorCode
				::InvalidIntent,
			TEXT("This initial route requires the default elective intent."),
			TEXT("Intent"));
		return false;
	}

	void SetSuccess(
		FMatchPlayCurrentAttackInitialRouteMappingResult& Result)
	{
		Result.bSuccess = true;
		Result.ActualBranch.ActionType = Result.Input.ActionType;
	}
}

FMatchPlayCurrentAttackInitialRouteMappingResult
FMatchPlayCurrentAttackInitialRouteMappingQuery::Map(
	const FMatchPlayCurrentAttackInitialRouteMappingInput& Input)
{
	using namespace MatchPlayCurrentAttackInitialRouteMappingQuery;

	FMatchPlayCurrentAttackInitialRouteMappingResult Result;
	Result.Input = Input;

	switch (Input.ActionType)
	{
	case ESkillRuleType::LongShot:
		if (!ValidateNoInitialRouteD6(Result))
		{
			return Result;
		}
		if (Input.Intent == EMatchPlayElectiveBranchIntent::DirectShot)
		{
			Result.ActualBranch.LongShot =
				EMatchPlayLongShotActualBranch::DirectShot;
		}
		else if (Input.Intent
			== EMatchPlayElectiveBranchIntent::DeadCorner)
		{
			Result.ActualBranch.LongShot =
				EMatchPlayLongShotActualBranch::DeadCorner;
		}
		else
		{
			SetFailure(
				Result,
				EMatchPlayCurrentAttackInitialRouteMappingErrorCode
					::InvalidIntent,
				TEXT("Long Shot requires DirectShot or DeadCorner intent."),
				TEXT("Intent"));
			return Result;
		}
		SetSuccess(Result);
		return Result;

	case ESkillRuleType::CutInsideShot:
		if (!ValidateNoInitialRouteD6(Result))
		{
			return Result;
		}
		if (Input.Intent == EMatchPlayElectiveBranchIntent::DirectShot)
		{
			Result.ActualBranch.CutInsideShot =
				EMatchPlayCutInsideShotActualBranch::DirectShot;
		}
		else if (Input.Intent
			== EMatchPlayElectiveBranchIntent::DeadCorner)
		{
			Result.ActualBranch.CutInsideShot =
				EMatchPlayCutInsideShotActualBranch::DeadCorner;
		}
		else
		{
			SetFailure(
				Result,
				EMatchPlayCurrentAttackInitialRouteMappingErrorCode
					::InvalidIntent,
				TEXT("Cut Inside Shot requires DirectShot or DeadCorner intent."),
				TEXT("Intent"));
			return Result;
		}
		SetSuccess(Result);
		return Result;

	case ESkillRuleType::Cross:
	{
		ECrossIntentType CrossIntent = ECrossIntentType::None;
		if (Input.Intent == EMatchPlayElectiveBranchIntent::CrossHigh)
		{
			CrossIntent = ECrossIntentType::High;
		}
		else if (Input.Intent == EMatchPlayElectiveBranchIntent::CrossLow)
		{
			CrossIntent = ECrossIntentType::Low;
		}
		else
		{
			SetFailure(
				Result,
				EMatchPlayCurrentAttackInitialRouteMappingErrorCode
					::InvalidIntent,
				TEXT("Cross requires CrossHigh or CrossLow intent."),
				TEXT("Intent"));
			return Result;
		}
		if (!ValidateInitialRouteD6(Result))
		{
			return Result;
		}

		FCrossSelectionQueryInput CrossInput;
		CrossInput.IntendedCrossType = CrossIntent;
		CrossInput.bHasExternalSelectionD6 = true;
		CrossInput.ExternalSelectionD6 = Input.InitialRouteD6;
		const FCrossSelectionQueryResult CrossResult =
			FCrossSelectionQuery::Select(CrossInput);
		if (!CrossResult.bSuccess || !CrossResult.bHasActualCrossType)
		{
			SetFailure(
				Result,
				EMatchPlayCurrentAttackInitialRouteMappingErrorCode
					::ActionSpecificMappingFailed,
				CrossResult.ErrorMessage,
				CrossResult.InvalidField);
			return Result;
		}

		Result.ActualBranch.Cross =
			CrossResult.ActualCrossType == ECrossActualType::High
				? EMatchPlayCrossActualBranch::High
				: EMatchPlayCrossActualBranch::Low;
		SetSuccess(Result);
		return Result;
	}

	case ESkillRuleType::PassControl:
	{
		if (!ValidateNoIntent(Result)
			|| !ValidateInitialRouteD6(Result))
		{
			return Result;
		}
		const FPassControlAdvanceTypeMappingResult PassResult =
			FPassControlAdvanceTypeMapper::Map(Input.InitialRouteD6);
		if (!PassResult.bSuccess)
		{
			SetFailure(
				Result,
				EMatchPlayCurrentAttackInitialRouteMappingErrorCode
					::ActionSpecificMappingFailed,
				TEXT("Pass Control D6 did not map to an advance type."),
				TEXT("InitialRouteD6"));
			return Result;
		}
		switch (PassResult.AdvanceType)
		{
		case EPassControlAdvanceType::PassAdvance:
			Result.ActualBranch.PassControl =
				EMatchPlayPassControlActualBranch::PassAdvance;
			break;
		case EPassControlAdvanceType::DribbleAdvance:
			Result.ActualBranch.PassControl =
				EMatchPlayPassControlActualBranch::DribbleAdvance;
			break;
		case EPassControlAdvanceType::RunAdvance:
			Result.ActualBranch.PassControl =
				EMatchPlayPassControlActualBranch::RunAdvance;
			break;
		case EPassControlAdvanceType::None:
		default:
			SetFailure(
				Result,
				EMatchPlayCurrentAttackInitialRouteMappingErrorCode
					::ActionSpecificMappingFailed,
				TEXT("Pass Control mapper returned an unsupported advance type."),
				TEXT("AdvanceType"));
			return Result;
		}
		SetSuccess(Result);
		return Result;
	}

	case ESkillRuleType::ThroughBall:
	{
		if (!ValidateNoIntent(Result)
			|| !ValidateInitialRouteD6(Result))
		{
			return Result;
		}
		FThroughBallBranchSelectionQueryInput ThroughBallInput;
		ThroughBallInput.bHasExternalSelectionD6 = true;
		ThroughBallInput.ExternalSelectionD6 = Input.InitialRouteD6;
		const FThroughBallBranchSelectionQueryResult ThroughBallResult =
			FThroughBallBranchSelectionQuery::Select(ThroughBallInput);
		if (!ThroughBallResult.bSuccess
			|| !ThroughBallResult.bHasSelectedThroughBallBranch)
		{
			SetFailure(
				Result,
				EMatchPlayCurrentAttackInitialRouteMappingErrorCode
					::ActionSpecificMappingFailed,
				ThroughBallResult.ErrorMessage,
				ThroughBallResult.InvalidField);
			return Result;
		}
		switch (ThroughBallResult.SelectedThroughBallBranch)
		{
		case EThroughBallSelectedBranch::Feet:
			Result.ActualBranch.ThroughBall =
				EMatchPlayThroughBallActualBranch::Feet;
			break;
		case EThroughBallSelectedBranch::BehindDefense:
			Result.ActualBranch.ThroughBall =
				EMatchPlayThroughBallActualBranch::BehindDefense;
			break;
		case EThroughBallSelectedBranch::AntiOffside:
			Result.ActualBranch.ThroughBall =
				EMatchPlayThroughBallActualBranch::AntiOffside;
			break;
		case EThroughBallSelectedBranch::None:
		default:
			SetFailure(
				Result,
				EMatchPlayCurrentAttackInitialRouteMappingErrorCode
					::ActionSpecificMappingFailed,
				TEXT("Through Ball mapper returned an unsupported branch."),
				TEXT("SelectedThroughBallBranch"));
			return Result;
		}
		SetSuccess(Result);
		return Result;
	}

	case ESkillRuleType::None:
	default:
		SetFailure(
			Result,
			EMatchPlayCurrentAttackInitialRouteMappingErrorCode
				::UnsupportedActionType,
			TEXT("Initial Route mapping requires a supported ActionType."),
			TEXT("ActionType"));
		return Result;
	}
}
