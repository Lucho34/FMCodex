#pragma once

#if WITH_DEV_AUTOMATION_TESTS
#include "FMCodexNetworkRandomProvider.h"

/** Exhaustion is failure: no automation fixture silently reaches OS randomness. */
class FFMCodexNetworkScriptedEntropy final : public IFMCodexNetworkEntropySource
{
public:
	explicit FFMCodexNetworkScriptedEntropy(TArray<uint32> InWords = {}) : Words(MoveTemp(InWords)) {}
	virtual bool Fill(TArrayView<uint8> Bytes) override
	{
		++Calls;
		if (Bytes.Num() != sizeof(uint32) || Next >= Words.Num())
		{
			return false;
		}
		FMemory::Memcpy(Bytes.GetData(), &Words[Next++], sizeof(uint32));
		return true;
	}
	int32 Calls = 0;
private:
	TArray<uint32> Words;
	int32 Next = 0;
};
#endif
