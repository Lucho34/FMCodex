#pragma once
#include "CoreMinimal.h"

#if !UE_BUILD_SHIPPING
/** DEV observer only. Never replicated, and never consumed by gameplay or presentation gates. */
namespace FMCodexHandoffAudit
{
	struct FContext
	{
		FGuid Match;
		int64 Sequence = 0, Request = 0;
		int32 Revision = 0, Source = 0, Viewer = 0, Target = 0;
		FString Role, Intent;
	};
	FMCODEX_API bool Enabled();
	FMCODEX_API bool EnabledForCommandLine(const TCHAR* CommandLine);
	FMCODEX_API FString Quote(const FString& Value);
	FMCODEX_API void Emit(const TCHAR* Event, const FContext& C, const FString& Fields = FString());
	FMCODEX_API const FContext* ServerContext();
	struct FMCODEX_API FServerScope
	{
		explicit FServerScope(const FContext& In);
		~FServerScope();
	private:
		FContext Context;
		const FContext* Previous = nullptr;
		bool bActive = false;
	};
	FMCODEX_API void AuthorityAccepted();
	/** Only trace dedupe; observes the existing gate instead of driving a second timing state machine. */
	struct FMCODEX_API FPresentationObserver
	{
		void ViewApplied(const FContext& In, const FString& Fields);
		void Observe(int32 Phase, const FString& Reason, bool bBlocked, bool bAfterRender,
			bool bVisible, const FString& Fields);
	private:
		FContext Context;
		int32 LastPhase = INDEX_NONE;
		bool bHasView = false, bReleased = false, bVisibleEmitted = false;
		FString LastBlockReason = TEXT("None");
	};
#if WITH_DEV_AUTOMATION_TESTS
	FMCODEX_API void SetTestCapture(TArray<FString>* Sink, bool bEnable);
#endif
}
#endif
