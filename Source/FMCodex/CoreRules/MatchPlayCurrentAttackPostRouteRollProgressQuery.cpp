#include "MatchPlayCurrentAttackPostRouteRollProgressQuery.h"

namespace MatchPlayCurrentAttackPostRouteRollProgressQuery
{
	using EPurpose = EMatchPlayCurrentAttackPostRouteRollPurpose;
	using EError = EMatchPlayCurrentAttackPostRouteRollProgressErrorCode;
	using FRecord = FMatchPlayCurrentAttackPostRouteRollRecord;
	using FResult = FMatchPlayCurrentAttackPostRouteRollProgressResult;

	constexpr int32 ConditionalContinuationMinD6 = 3;
	constexpr int32 AntiOffsideOneOnOneD6 = 6;
	constexpr int32 BehindDefenseP2OneOnOneMaxD6 = 3;

	void SetFailure(
		FResult& Result,
		const EError ErrorCode,
		const FString& ErrorMessage)
	{
		Result.ErrorCode = ErrorCode;
		Result.ErrorMessage = ErrorMessage;
	}

	void SetSuccess(
		FResult& Result,
		const bool bContractComplete,
		const EPurpose NextPurpose = EPurpose::None)
	{
		Result.bIsCanonical = true;
		Result.bContractComplete = bContractComplete;
		Result.bHasNextPurpose = NextPurpose != EPurpose::None;
		Result.NextPurpose = NextPurpose;
	}

	bool ValidateRecordValuesAndDuplicates(
		const TArray<FRecord>& Records,
		FResult& Result)
	{
		TSet<EPurpose> SeenPurposes;
		for (const FRecord& Record : Records)
		{
			if (Record.Purpose == EPurpose::None)
			{
				SetFailure(
					Result,
					EError::InvalidPurpose,
					TEXT("Post-route roll records require a non-None purpose."));
				return false;
			}
			if (Record.RawD6 < 1 || Record.RawD6 > 6)
			{
				SetFailure(
					Result,
					EError::InvalidD6,
					TEXT("Post-route roll RawD6 must be in range [1, 6]."));
				return false;
			}
			if (SeenPurposes.Contains(Record.Purpose))
			{
				SetFailure(
					Result,
					EError::DuplicatePurpose,
					TEXT("A post-route semantic roll purpose may occur only once in one Resolution Session."));
				return false;
			}
			SeenPurposes.Add(Record.Purpose);
		}
		return true;
	}

	bool ValidateOrderedPrefix(
		const TArray<FRecord>& Records,
		const TArray<EPurpose>& Expected,
		const int32 MinimumCount,
		FResult& Result)
	{
		if (Records.Num() < MinimumCount)
		{
			SetFailure(
				Result,
				EError::MissingRequiredPhasePrefix,
				TEXT("The post-route roll phase is missing its required prior roll prefix."));
			return false;
		}
		if (Records.Num() > Expected.Num())
		{
			SetFailure(
				Result,
				EError::TooManyRolls,
				TEXT("The post-route roll contract contains too many records."));
			return false;
		}
		for (int32 Index = 0; Index < Records.Num(); ++Index)
		{
			if (Records[Index].Purpose != Expected[Index])
			{
				SetFailure(
					Result,
					EError::InvalidRollOrder,
					TEXT("Post-route roll purposes do not match canonical acquisition order."));
				return false;
			}
		}
		return true;
	}

	FResult ValidateFixedContract(
		const TArray<FRecord>& Records,
		const TArray<EPurpose>& Expected)
	{
		FResult Result;
		if (!ValidateRecordValuesAndDuplicates(Records, Result)
			|| !ValidateOrderedPrefix(Records, Expected, 0, Result))
		{
			return Result;
		}
		if (Records.Num() == Expected.Num())
		{
			SetSuccess(Result, true);
		}
		else
		{
			SetSuccess(Result, false, Expected[Records.Num()]);
		}
		return Result;
	}

	FResult ValidateConditionalPrimaryContract(
		const TArray<FRecord>& Records)
	{
		FResult Result;
		const TArray<EPurpose> Expected = {
			EPurpose::PrimaryAttack,
			EPurpose::PrimaryDefense
		};
		if (!ValidateRecordValuesAndDuplicates(Records, Result)
			|| !ValidateOrderedPrefix(Records, Expected, 0, Result))
		{
			return Result;
		}
		if (Records.IsEmpty())
		{
			SetSuccess(Result, false, EPurpose::PrimaryAttack);
			return Result;
		}

		const bool bRequiresDefense =
			Records[0].RawD6 >= ConditionalContinuationMinD6;
		if (!bRequiresDefense)
		{
			if (Records.Num() != 1)
			{
				SetFailure(
					Result,
					EError::ConditionalDefenseNotAllowed,
					TEXT("This primary Attack D6 completes the conditional post-route roll contract without Defense."));
				return Result;
			}
			SetSuccess(Result, true);
			return Result;
		}

		if (Records.Num() == 1)
		{
			SetSuccess(Result, false, EPurpose::PrimaryDefense);
		}
		else
		{
			SetSuccess(Result, true);
		}
		return Result;
	}

	FResult ValidatePrimaryBranch(
		const FMatchPlayCurrentAttackActualBranch& Branch,
		const TArray<FRecord>& Records)
	{
		switch (Branch.ActionType)
		{
		case ESkillRuleType::LongShot:
			if (Branch.LongShot
				== EMatchPlayLongShotActualBranch::DirectShot)
			{
				return ValidateConditionalPrimaryContract(Records);
			}
			if (Branch.LongShot
				== EMatchPlayLongShotActualBranch::DeadCorner)
			{
				return ValidateFixedContract(
					Records,
					{ EPurpose::PairedAttackA, EPurpose::PairedAttackB });
			}
			break;

		case ESkillRuleType::CutInsideShot:
			if (Branch.CutInsideShot
				== EMatchPlayCutInsideShotActualBranch::DirectShot)
			{
				return ValidateConditionalPrimaryContract(Records);
			}
			if (Branch.CutInsideShot
				== EMatchPlayCutInsideShotActualBranch::DeadCorner)
			{
				return ValidateFixedContract(
					Records,
					{ EPurpose::PairedAttackA, EPurpose::PairedAttackB });
			}
			break;

		case ESkillRuleType::Cross:
			if (Branch.Cross == EMatchPlayCrossActualBranch::High
				|| Branch.Cross == EMatchPlayCrossActualBranch::Low)
			{
				return ValidateFixedContract(
					Records,
					{ EPurpose::PrimaryAttack, EPurpose::PrimaryDefense });
			}
			break;

		case ESkillRuleType::PassControl:
			if (Branch.PassControl
					== EMatchPlayPassControlActualBranch::PassAdvance
				|| Branch.PassControl
					== EMatchPlayPassControlActualBranch::DribbleAdvance
				|| Branch.PassControl
					== EMatchPlayPassControlActualBranch::RunAdvance)
			{
				return ValidateFixedContract(
					Records,
					{ EPurpose::PrimaryAttack, EPurpose::PrimaryDefense });
			}
			break;

		case ESkillRuleType::ThroughBall:
			if (Branch.ThroughBall
				== EMatchPlayThroughBallActualBranch::Feet)
			{
				return ValidateFixedContract(
					Records,
					{ EPurpose::PrimaryAttack, EPurpose::PrimaryDefense });
			}
			if (Branch.ThroughBall
				== EMatchPlayThroughBallActualBranch::BehindDefense)
			{
				return ValidateConditionalPrimaryContract(Records);
			}
			if (Branch.ThroughBall
				== EMatchPlayThroughBallActualBranch::AntiOffside)
			{
				return ValidateFixedContract(
					Records,
					{ EPurpose::PrimaryAttack });
			}
			break;

		case ESkillRuleType::None:
		default:
			break;
		}

		FResult Result;
		SetFailure(
			Result,
			EError::UnsupportedPhaseForBranch,
			TEXT("The Actual Branch does not define a primary post-route roll contract."));
		return Result;
	}

	FResult ValidateBehindDefenseP2(
		const FMatchPlayCurrentAttackActualBranch& Branch,
		const TArray<FRecord>& Records)
	{
		FResult Result;
		if (Branch.ActionType != ESkillRuleType::ThroughBall
			|| Branch.ThroughBall
				!= EMatchPlayThroughBallActualBranch::BehindDefense)
		{
			SetFailure(
				Result,
				EError::UnsupportedPhaseForBranch,
				TEXT("BehindDefenseP2 roll acquisition requires the ThroughBall BehindDefense branch."));
			return Result;
		}

		const TArray<EPurpose> Expected = {
			EPurpose::PrimaryAttack,
			EPurpose::PrimaryDefense,
			EPurpose::BehindDefenseP2Defense
		};
		if (!ValidateRecordValuesAndDuplicates(Records, Result)
			|| !ValidateOrderedPrefix(Records, Expected, 2, Result))
		{
			return Result;
		}
		if (Records[0].RawD6 < ConditionalContinuationMinD6)
		{
			SetFailure(
				Result,
				EError::InvalidLaterPhasePrerequisite,
				TEXT("BehindDefenseP2 cannot follow an Attack D6 that completes P1 as OutOfPlay."));
			return Result;
		}
		if (Records.Num() == Expected.Num())
		{
			SetSuccess(Result, true);
		}
		else
		{
			SetSuccess(Result, false, EPurpose::BehindDefenseP2Defense);
		}
		return Result;
	}

	FResult ValidateOneOnOneChipShot(
		const FMatchPlayCurrentAttackActualBranch& Branch,
		const TArray<FRecord>& Records)
	{
		FResult Result;
		TArray<EPurpose> Expected;
		int32 MinimumCount = 0;
		if (Branch.ActionType == ESkillRuleType::ThroughBall
			&& Branch.ThroughBall
				== EMatchPlayThroughBallActualBranch::AntiOffside)
		{
			Expected = {
				EPurpose::PrimaryAttack,
				EPurpose::OneOnOneChipShotAttack
			};
			MinimumCount = 1;
		}
		else if (Branch.ActionType == ESkillRuleType::ThroughBall
			&& Branch.ThroughBall
				== EMatchPlayThroughBallActualBranch::BehindDefense)
		{
			Expected = {
				EPurpose::PrimaryAttack,
				EPurpose::PrimaryDefense,
				EPurpose::BehindDefenseP2Defense,
				EPurpose::OneOnOneChipShotAttack
			};
			MinimumCount = 3;
		}
		else
		{
			SetFailure(
				Result,
				EError::UnsupportedPhaseForBranch,
				TEXT("OneOnOneChipShot roll acquisition requires a supported ThroughBall branch."));
			return Result;
		}

		if (!ValidateRecordValuesAndDuplicates(Records, Result)
			|| !ValidateOrderedPrefix(
				Records,
				Expected,
				MinimumCount,
				Result))
		{
			return Result;
		}

		if (Branch.ThroughBall
			== EMatchPlayThroughBallActualBranch::AntiOffside)
		{
			if (Records[0].RawD6 != AntiOffsideOneOnOneD6)
			{
				SetFailure(
					Result,
					EError::InvalidLaterPhasePrerequisite,
					TEXT("AntiOffside OneOnOne requires its accepted Primary Attack roll to be 6."));
				return Result;
			}
		}
		else if (Records[0].RawD6 < ConditionalContinuationMinD6
			|| Records[2].RawD6 > BehindDefenseP2OneOnOneMaxD6)
		{
			SetFailure(
				Result,
				EError::InvalidLaterPhasePrerequisite,
				TEXT("BehindDefense OneOnOne roll prerequisites are inconsistent."));
			return Result;
		}

		if (Records.Num() == Expected.Num())
		{
			SetSuccess(Result, true);
		}
		else
		{
			SetSuccess(Result, false, EPurpose::OneOnOneChipShotAttack);
		}
		return Result;
	}

	FResult ValidateOneOnOneDirectShot(
		const FMatchPlayCurrentAttackActualBranch& Branch,
		const TArray<FRecord>& Records)
	{
		FResult Result;
		TArray<EPurpose> Expected;
		int32 MinimumCount = 0;
		if (Branch.ActionType == ESkillRuleType::ThroughBall
			&& Branch.ThroughBall
				== EMatchPlayThroughBallActualBranch::AntiOffside)
		{
			Expected = {
				EPurpose::PrimaryAttack,
				EPurpose::OneOnOneDirectShotAttack,
				EPurpose::OneOnOneDirectShotDefense
			};
			MinimumCount = 1;
		}
		else if (Branch.ActionType == ESkillRuleType::ThroughBall
			&& Branch.ThroughBall
				== EMatchPlayThroughBallActualBranch::BehindDefense)
		{
			Expected = {
				EPurpose::PrimaryAttack,
				EPurpose::PrimaryDefense,
				EPurpose::BehindDefenseP2Defense,
				EPurpose::OneOnOneDirectShotAttack,
				EPurpose::OneOnOneDirectShotDefense
			};
			MinimumCount = 3;
		}
		else
		{
			SetFailure(
				Result,
				EError::UnsupportedPhaseForBranch,
				TEXT("OneOnOneDirectShot roll acquisition requires a supported ThroughBall branch."));
			return Result;
		}

		if (!ValidateRecordValuesAndDuplicates(Records, Result)
			|| !ValidateOrderedPrefix(Records, Expected, MinimumCount, Result))
		{
			return Result;
		}

		if (Branch.ThroughBall
			== EMatchPlayThroughBallActualBranch::AntiOffside)
		{
			if (Records[0].RawD6 != AntiOffsideOneOnOneD6)
			{
				SetFailure(
					Result,
					EError::InvalidLaterPhasePrerequisite,
					TEXT("AntiOffside OneOnOne DirectShot requires its accepted Primary Attack roll to be 6."));
				return Result;
			}
		}
		else if (Records[0].RawD6 < ConditionalContinuationMinD6
			|| Records[2].RawD6 > BehindDefenseP2OneOnOneMaxD6)
		{
			SetFailure(
				Result,
				EError::InvalidLaterPhasePrerequisite,
				TEXT("BehindDefense OneOnOne DirectShot roll prerequisites are inconsistent."));
			return Result;
		}

		if (Records.Num() == Expected.Num())
		{
			SetSuccess(Result, true);
		}
		else
		{
			SetSuccess(Result, false, Expected[Records.Num()]);
		}
		return Result;
	}
}

FMatchPlayCurrentAttackPostRouteRollProgressResult
FMatchPlayCurrentAttackPostRouteRollProgressQuery::Evaluate(
	const FMatchPlayCurrentAttackResolutionSession& Session)
{
	using namespace MatchPlayCurrentAttackPostRouteRollProgressQuery;

	FResult Result;
	if (Session.Stage
		!= EMatchPlayCurrentAttackResolutionStage::RouteResolved)
	{
		SetFailure(
			Result,
			EError::RouteNotResolved,
			TEXT("Post-route roll progress requires RouteResolved."));
		return Result;
	}
	if (!Session.bHasActualBranch
		|| Session.ActualBranch.ActionType == ESkillRuleType::None)
	{
		SetFailure(
			Result,
			EError::MissingActualBranch,
			TEXT("Post-route roll progress requires an Actual Branch."));
		return Result;
	}

	const FMatchPlayCurrentAttackPostRouteRollProgress& Progress =
		Session.PostRouteRollProgress;
	switch (Progress.Phase)
	{
	case EMatchPlayCurrentAttackPostRouteRollPhase::None:
		if (!Progress.RollRecords.IsEmpty())
		{
			SetFailure(
				Result,
				EError::RecordsWithoutPhase,
				TEXT("Post-route roll records require an active acquisition phase."));
			return Result;
		}
		SetSuccess(Result, false);
		return Result;

	case EMatchPlayCurrentAttackPostRouteRollPhase::PrimaryBranch:
		return ValidatePrimaryBranch(
			Session.ActualBranch,
			Progress.RollRecords);

	case EMatchPlayCurrentAttackPostRouteRollPhase::BehindDefenseP2:
		return ValidateBehindDefenseP2(
			Session.ActualBranch,
			Progress.RollRecords);

	case EMatchPlayCurrentAttackPostRouteRollPhase::OneOnOneChipShot:
		return ValidateOneOnOneChipShot(
			Session.ActualBranch,
			Progress.RollRecords);

	case EMatchPlayCurrentAttackPostRouteRollPhase::OneOnOneDirectShot:
		return ValidateOneOnOneDirectShot(
			Session.ActualBranch,
			Progress.RollRecords);

	default:
		SetFailure(
			Result,
			EError::UnsupportedPhaseForBranch,
			TEXT("Post-route roll progress contains an unsupported phase."));
		return Result;
	}
}
