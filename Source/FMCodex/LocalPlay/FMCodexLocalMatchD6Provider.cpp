#include "FMCodexLocalMatchD6Provider.h"

namespace FMCodexLocalMatchD6Provider
{
	bool IsValidPostRoutePurpose(
		const EMatchPlayCurrentAttackPostRouteRollPurpose Purpose)
	{
		switch (Purpose)
		{
		case EMatchPlayCurrentAttackPostRouteRollPurpose::PrimaryAttack:
		case EMatchPlayCurrentAttackPostRouteRollPurpose::PrimaryDefense:
		case EMatchPlayCurrentAttackPostRouteRollPurpose::PairedAttackA:
		case EMatchPlayCurrentAttackPostRouteRollPurpose::PairedAttackB:
		case EMatchPlayCurrentAttackPostRouteRollPurpose
			::BehindDefenseP2Defense:
		case EMatchPlayCurrentAttackPostRouteRollPurpose
			::OneOnOneChipShotAttack:
		case EMatchPlayCurrentAttackPostRouteRollPurpose
			::OneOnOneDirectShotAttack:
		case EMatchPlayCurrentAttackPostRouteRollPurpose
			::OneOnOneDirectShotDefense:
		case EMatchPlayCurrentAttackPostRouteRollPurpose
			::ShortFreeKickDirectAttack:
		case EMatchPlayCurrentAttackPostRouteRollPurpose
			::ShortFreeKickDirectDefense:
		case EMatchPlayCurrentAttackPostRouteRollPurpose
			::ShortFreeKickAngledA:
		case EMatchPlayCurrentAttackPostRouteRollPurpose
			::ShortFreeKickAngledB:
		case EMatchPlayCurrentAttackPostRouteRollPurpose
			::LongFreeKickDirectAttack:
		case EMatchPlayCurrentAttackPostRouteRollPurpose
			::LongFreeKickDirectDefense:
		case EMatchPlayCurrentAttackPostRouteRollPurpose
			::LongFreeKickPowerA:
		case EMatchPlayCurrentAttackPostRouteRollPurpose
			::LongFreeKickPowerB:
		case EMatchPlayCurrentAttackPostRouteRollPurpose
			::PenaltyDirectAttack:
		case EMatchPlayCurrentAttackPostRouteRollPurpose
			::PenaltyDirectDefense:
		case EMatchPlayCurrentAttackPostRouteRollPurpose::PenaltyPanenka:
		case EMatchPlayCurrentAttackPostRouteRollPurpose
			::CornerParticipantSelection:
		case EMatchPlayCurrentAttackPostRouteRollPurpose::CornerRoute:
		case EMatchPlayCurrentAttackPostRouteRollPurpose::CornerAttack:
		case EMatchPlayCurrentAttackPostRouteRollPurpose::CornerDefense:
		case EMatchPlayCurrentAttackPostRouteRollPurpose::CornerAutomaticScorer:
			return true;
		case EMatchPlayCurrentAttackPostRouteRollPurpose::None:
		default:
			return false;
		}
	}
}

FFMCodexLocalMatchD6Provider::FFMCodexLocalMatchD6Provider(
	const int32 Seed)
	: RandomStream(Seed)
{
}

FMatchPlayAttackEntryRollProviderResult
FFMCodexLocalMatchD6Provider::RollD12(
	const EMatchPlayAttackEntryRollPurpose Purpose)
{
	FMatchPlayAttackEntryRollProviderResult Result;
	if (Purpose != EMatchPlayAttackEntryRollPurpose::InitialActionPoint)
	{
		Result.ErrorCode =
			EMatchPlayAttackEntryRollProviderErrorCode::InvalidPurpose;
		Result.ErrorMessage =
			TEXT("Local D12 entry roll requires InitialActionPoint purpose.");
		return Result;
	}
	Result.bSuccess = true;
	Result.RawRoll = RandomStream.RandRange(1, 12);
	return Result;
}

FMatchPlayAttackEntryRollProviderResult
FFMCodexLocalMatchD6Provider::RollD6(
	const EMatchPlayAttackEntryRollPurpose Purpose)
{
	FMatchPlayAttackEntryRollProviderResult Result;
	if (Purpose != EMatchPlayAttackEntryRollPurpose::SetPieceType)
	{
		Result.ErrorCode =
			EMatchPlayAttackEntryRollProviderErrorCode::InvalidPurpose;
		Result.ErrorMessage =
			TEXT("Local attack-entry D6 requires SetPieceType purpose.");
		return Result;
	}
	Result.bSuccess = true;
	Result.RawRoll = RollCanonicalD6();
	return Result;
}

FMatchPlayAttackEntrySelectionProviderResult
FFMCodexLocalMatchD6Provider::SelectUniformIndex(
	const EMatchPlayAttackEntryRollPurpose Purpose,
	const int32 CandidateCount)
{
	FMatchPlayAttackEntrySelectionProviderResult Result;
	if (Purpose != EMatchPlayAttackEntryRollPurpose::SendingOffSelection
		|| CandidateCount <= 0)
	{
		Result.ErrorCode =
			EMatchPlayAttackEntryRollProviderErrorCode::InvalidPurpose;
		Result.ErrorMessage =
			TEXT("Local uniform selection requires SendingOffSelection and candidates.");
		return Result;
	}
	Result.bSuccess = true;
	Result.SelectedIndex = RandomStream.RandRange(0, CandidateCount - 1);
	return Result;
}

FMatchPlayInitialRouteRollProviderResult
FFMCodexLocalMatchD6Provider::RollD6(
	const EMatchPlayCurrentAttackResolutionRollPurpose Purpose)
{
	FMatchPlayInitialRouteRollProviderResult Result;
	if (Purpose
		!= EMatchPlayCurrentAttackResolutionRollPurpose::InitialRoute)
	{
		Result.ErrorCode =
			EMatchPlayInitialRouteRollProviderErrorCode::InvalidPurpose;
		Result.ErrorMessage =
			TEXT("Local D6 initial-route roll requires InitialRoute purpose.");
		return Result;
	}

	Result.bSuccess = true;
	Result.RawD6 = RollCanonicalD6();
	return Result;
}

FMatchPlayPostRouteRollProviderResult
FFMCodexLocalMatchD6Provider::RollD6(
	const EMatchPlayCurrentAttackPostRouteRollPurpose Purpose)
{
	using namespace FMCodexLocalMatchD6Provider;

	FMatchPlayPostRouteRollProviderResult Result;
	if (!IsValidPostRoutePurpose(Purpose))
	{
		Result.ErrorCode =
			EMatchPlayPostRouteRollProviderErrorCode::InvalidPurpose;
		Result.ErrorMessage =
			TEXT("Local D6 post-route roll requires a valid purpose.");
		return Result;
	}

	Result.bSuccess = true;
	Result.RawD6 = RollCanonicalD6();
	return Result;
}

FMatchPlayRecoveryProviderResult
FFMCodexLocalMatchD6Provider::DrawWeightedWithoutReplacement(
	const EMatchPlayRecoveryPurpose Purpose,
	const TArray<FMatchPlayRecoveryCandidate>& OrderedCandidates,
	const int32 ReturnCount)
{
	FMatchPlayRecoveryProviderResult Result;
	if (Purpose != EMatchPlayRecoveryPurpose::ConsumedRecovery
		|| OrderedCandidates.Num() < 2
		|| ReturnCount != 2)
	{
		Result.ErrorCode = EMatchPlayRecoveryProviderErrorCode::InvalidPurpose;
		Result.ErrorMessage =
			TEXT("Local Recovery requires ConsumedRecovery, at least two candidates, and ReturnCount 2.");
		return Result;
	}

	TArray<int32> RemainingIndices;
	RemainingIndices.Reserve(OrderedCandidates.Num());
	for (int32 Index = 0; Index < OrderedCandidates.Num(); ++Index)
	{
		const int32 Weight = OrderedCandidates[Index].StaminaWeight;
		if (Weight < 1 || Weight > 6)
		{
			Result.ErrorCode =
				EMatchPlayRecoveryProviderErrorCode::ProviderFailure;
			Result.ErrorMessage =
				TEXT("Local Recovery received an invalid Stamina weight.");
			return Result;
		}
		RemainingIndices.Add(Index);
	}

	for (int32 Draw = 0; Draw < ReturnCount; ++Draw)
	{
		int32 TotalWeight = 0;
		for (const int32 Index : RemainingIndices)
		{
			TotalWeight += OrderedCandidates[Index].StaminaWeight;
		}
		if (TotalWeight <= 0)
		{
			Result.ErrorCode =
				EMatchPlayRecoveryProviderErrorCode::ProviderFailure;
			Result.ErrorMessage = TEXT("Local Recovery has no positive weight.");
			Result.SelectedCandidateIndices.Reset();
			return Result;
		}

		const int32 Ticket = RandomStream.RandRange(1, TotalWeight);
		int32 CumulativeWeight = 0;
		int32 ChosenRemainingIndex = INDEX_NONE;
		for (int32 RemainingIndex = 0;
			RemainingIndex < RemainingIndices.Num();
			++RemainingIndex)
		{
			CumulativeWeight += OrderedCandidates[
				RemainingIndices[RemainingIndex]].StaminaWeight;
			if (Ticket <= CumulativeWeight)
			{
				ChosenRemainingIndex = RemainingIndex;
				break;
			}
		}
		if (ChosenRemainingIndex == INDEX_NONE)
		{
			Result.ErrorCode =
				EMatchPlayRecoveryProviderErrorCode::ProviderFailure;
			Result.ErrorMessage =
				TEXT("Local Recovery failed to resolve a weighted ticket.");
			Result.SelectedCandidateIndices.Reset();
			return Result;
		}
		Result.SelectedCandidateIndices.Add(
			RemainingIndices[ChosenRemainingIndex]);
		RemainingIndices.RemoveAt(ChosenRemainingIndex);
	}

	Result.bSuccess = true;
	return Result;
}

int32 FFMCodexLocalMatchD6Provider::RollOrdinaryTacticalPoint()
{
	return RandomStream.RandRange(2, 8);
}

int32 FFMCodexLocalMatchD6Provider::RollCanonicalD6()
{
	return RandomStream.RandRange(1, 6);
}
