#include "FMCodexNetworkRandomProvider.h"
#include "IPlatformCrypto.h"
#include "PlatformCryptoTypes.h"

namespace
{
	class FPlatformNetworkEntropy final : public IFMCodexNetworkEntropySource
	{
	public:
		FPlatformNetworkEntropy()
		{
			if (IPlatformCrypto* Crypto = FModuleManager::LoadModulePtr<IPlatformCrypto>("PlatformCrypto"))
			{
				Context = Crypto->CreateContext();
			}
		}
		virtual bool Fill(TArrayView<uint8> Bytes) override
		{
			return Context && Context->CreateRandomBytes(Bytes) == EPlatformCryptoResult::Success;
		}
	private:
		TUniquePtr<FEncryptionContext> Context;
	};
}

namespace FMCodexNetworkRandomProvider
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

FFMCodexNetworkRandomProvider::FFMCodexNetworkRandomProvider()
	: Entropy(MakeUnique<FPlatformNetworkEntropy>())
{
}

FFMCodexNetworkRandomProvider::~FFMCodexNetworkRandomProvider() = default;

#if WITH_DEV_AUTOMATION_TESTS
FFMCodexNetworkRandomProvider::FFMCodexNetworkRandomProvider(
	TUniquePtr<IFMCodexNetworkEntropySource> TestEntropy)
	: Entropy(MoveTemp(TestEntropy))
{
}
#endif

bool FFMCodexNetworkRandomProvider::SampleIndex(const int32 Count, int32& OutIndex)
{
	OutIndex = INDEX_NONE;
	if (Count <= 0 || !Entropy)
	{
		return false;
	}
	// Keep a source interval whose size is an exact multiple of Count.
	// Each residue then has exactly Limit / Count representatives.
	const uint64 Domain = uint64(1) << 32;
	const uint64 Limit = Domain - Domain % static_cast<uint32>(Count);
	for (int32 Attempt = 0; Attempt < 64; ++Attempt)
	{
		uint32 Sample = 0;
		const bool bRead = Entropy->Fill(MakeArrayView(
			reinterpret_cast<uint8*>(&Sample), static_cast<int32>(sizeof(Sample))));
		if (!bRead)
		{
			FMemory::Memzero(&Sample, sizeof(Sample));
			return false;
		}
		if (static_cast<uint64>(Sample) < Limit)
		{
			OutIndex = static_cast<int32>(Sample % static_cast<uint32>(Count));
			FMemory::Memzero(&Sample, sizeof(Sample));
			return true;
		}
		FMemory::Memzero(&Sample, sizeof(Sample));
	}
	// A defective source must not hang the game thread or trigger a predictable fallback.
	return false;
}

FMatchPlayAttackEntryRollProviderResult
FFMCodexNetworkRandomProvider::RollD12(
	const EMatchPlayAttackEntryRollPurpose Purpose)
{
	FMatchPlayAttackEntryRollProviderResult Result;
	if (Purpose != EMatchPlayAttackEntryRollPurpose::InitialActionPoint)
	{
		Result.ErrorCode =
			EMatchPlayAttackEntryRollProviderErrorCode::InvalidPurpose;
		Result.ErrorMessage =
			TEXT("Network D12 entry roll requires InitialActionPoint purpose.");
		return Result;
	}
	int32 Index = INDEX_NONE;
	if (!SampleIndex(12, Index))
	{
		Result.ErrorCode = EMatchPlayAttackEntryRollProviderErrorCode::ProviderFailure;
		Result.ErrorMessage = TEXT("Network secure randomness unavailable.");
		return Result;
	}
	Result.bSuccess = true;
	Result.RawRoll = Index + 1;
	return Result;
}

FMatchPlayAttackEntryRollProviderResult
FFMCodexNetworkRandomProvider::RollD6(
	const EMatchPlayAttackEntryRollPurpose Purpose)
{
	FMatchPlayAttackEntryRollProviderResult Result;
	if (Purpose != EMatchPlayAttackEntryRollPurpose::SetPieceType)
	{
		Result.ErrorCode =
			EMatchPlayAttackEntryRollProviderErrorCode::InvalidPurpose;
		Result.ErrorMessage =
			TEXT("Network attack-entry D6 requires SetPieceType purpose.");
		return Result;
	}
	int32 Index = INDEX_NONE;
	if (!SampleIndex(6, Index))
	{
		Result.ErrorCode = EMatchPlayAttackEntryRollProviderErrorCode::ProviderFailure;
		Result.ErrorMessage = TEXT("Network secure randomness unavailable.");
		return Result;
	}
	Result.bSuccess = true;
	Result.RawRoll = Index + 1;
	return Result;
}

FMatchPlayAttackEntrySelectionProviderResult
FFMCodexNetworkRandomProvider::SelectUniformIndex(
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
			TEXT("Network uniform selection requires SendingOffSelection and candidates.");
		return Result;
	}
	if (!SampleIndex(CandidateCount, Result.SelectedIndex))
	{
		Result.ErrorCode = EMatchPlayAttackEntryRollProviderErrorCode::ProviderFailure;
		Result.ErrorMessage = TEXT("Network secure randomness unavailable.");
		return Result;
	}
	Result.bSuccess = true;
	return Result;
}

FMatchPlayInitialRouteRollProviderResult
FFMCodexNetworkRandomProvider::RollD6(
	const EMatchPlayCurrentAttackResolutionRollPurpose Purpose)
{
	FMatchPlayInitialRouteRollProviderResult Result;
	if (Purpose
		!= EMatchPlayCurrentAttackResolutionRollPurpose::InitialRoute)
	{
		Result.ErrorCode =
			EMatchPlayInitialRouteRollProviderErrorCode::InvalidPurpose;
		Result.ErrorMessage =
			TEXT("Network D6 initial-route roll requires InitialRoute purpose.");
		return Result;
	}

	int32 Index = INDEX_NONE;
	if (!SampleIndex(6, Index))
	{
		Result.ErrorCode = EMatchPlayInitialRouteRollProviderErrorCode::ProviderFailure;
		Result.ErrorMessage = TEXT("Network secure randomness unavailable.");
		return Result;
	}
	Result.bSuccess = true;
	Result.RawD6 = Index + 1;
	return Result;
}

FMatchPlayPostRouteRollProviderResult
FFMCodexNetworkRandomProvider::RollD6(
	const EMatchPlayCurrentAttackPostRouteRollPurpose Purpose)
{
	using namespace FMCodexNetworkRandomProvider;

	FMatchPlayPostRouteRollProviderResult Result;
	if (!IsValidPostRoutePurpose(Purpose))
	{
		Result.ErrorCode =
			EMatchPlayPostRouteRollProviderErrorCode::InvalidPurpose;
		Result.ErrorMessage =
			TEXT("Network D6 post-route roll requires a valid purpose.");
		return Result;
	}

	int32 Index = INDEX_NONE;
	if (!SampleIndex(6, Index))
	{
		Result.ErrorCode = EMatchPlayPostRouteRollProviderErrorCode::ProviderFailure;
		Result.ErrorMessage = TEXT("Network secure randomness unavailable.");
		return Result;
	}
	Result.bSuccess = true;
	Result.RawD6 = Index + 1;
	return Result;
}

FMatchPlayRecoveryProviderResult
FFMCodexNetworkRandomProvider::DrawWeightedWithoutReplacement(
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
			TEXT("Network Recovery requires ConsumedRecovery, at least two candidates, and ReturnCount 2.");
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
				TEXT("Network Recovery received an invalid Stamina weight.");
			return Result;
		}
		RemainingIndices.Add(Index);
	}

	for (int32 Draw = 0; Draw < ReturnCount; ++Draw)
	{
		int64 TotalWeight = 0;
		for (const int32 Index : RemainingIndices)
		{
			TotalWeight += OrderedCandidates[Index].StaminaWeight;
		}
		if (TotalWeight <= 0 || TotalWeight > MAX_int32)
		{
			Result.ErrorCode =
				EMatchPlayRecoveryProviderErrorCode::ProviderFailure;
			Result.ErrorMessage = TEXT("Network Recovery has no positive weight.");
			Result.SelectedCandidateIndices.Reset();
			return Result;
		}

		int32 TicketIndex = INDEX_NONE;
		if (!SampleIndex(static_cast<int32>(TotalWeight), TicketIndex))
		{
			Result.ErrorCode = EMatchPlayRecoveryProviderErrorCode::ProviderFailure;
			Result.ErrorMessage = TEXT("Network secure randomness unavailable.");
			Result.SelectedCandidateIndices.Reset();
			return Result;
		}
		const int32 Ticket = TicketIndex + 1;
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
				TEXT("Network Recovery failed to resolve a weighted ticket.");
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
