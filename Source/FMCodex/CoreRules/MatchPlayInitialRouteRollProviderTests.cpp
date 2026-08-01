#if WITH_DEV_AUTOMATION_TESTS

#include "MatchPlayInitialRouteRollProvider.h"

#include "MatchPlayState.h"
#include "Misc/AutomationTest.h"

#include <type_traits>

namespace MatchPlayInitialRouteRollProviderTests
{
	const TCHAR* InvalidPurposeMessage =
		TEXT("Initial Route roll provider requires InitialRoute purpose.");
	const TCHAR* EmptyQueueMessage =
		TEXT("Initial Route roll provider has no configured result.");

	FMatchPlayInitialRouteRollProviderResult MakeSuccess(
		const int32 RawD6)
	{
		FMatchPlayInitialRouteRollProviderResult Result;
		Result.bSuccess = true;
		Result.RawD6 = RawD6;
		return Result;
	}

	FMatchPlayInitialRouteRollProviderResult MakeFailure(
		const FString& ErrorMessage)
	{
		FMatchPlayInitialRouteRollProviderResult Result;
		Result.ErrorCode =
			EMatchPlayInitialRouteRollProviderErrorCode::ProviderFailure;
		Result.ErrorMessage = ErrorMessage;
		return Result;
	}

	class FDeterministicFakeInitialRouteRollProvider final
		: public IMatchPlayInitialRouteRollProvider
	{
	public:
		void Enqueue(
			const FMatchPlayInitialRouteRollProviderResult& Result)
		{
			ConfiguredResults.Add(Result);
		}

		virtual FMatchPlayInitialRouteRollProviderResult RollD6(
			const EMatchPlayCurrentAttackResolutionRollPurpose Purpose)
			override
		{
			++CallCount;
			PurposeHistory.Add(Purpose);

			if (Purpose
				!= EMatchPlayCurrentAttackResolutionRollPurpose::InitialRoute)
			{
				FMatchPlayInitialRouteRollProviderResult Result;
				Result.ErrorCode =
					EMatchPlayInitialRouteRollProviderErrorCode
						::InvalidPurpose;
				Result.ErrorMessage = InvalidPurposeMessage;
				return Result;
			}

			if (NextResultIndex >= ConfiguredResults.Num())
			{
				return MakeFailure(EmptyQueueMessage);
			}

			return ConfiguredResults[NextResultIndex++];
		}

		int32 GetCallCount() const
		{
			return CallCount;
		}

		const TArray<EMatchPlayCurrentAttackResolutionRollPurpose>&
		GetPurposeHistory() const
		{
			return PurposeHistory;
		}

		int32 GetRemainingResultCount() const
		{
			return ConfiguredResults.Num() - NextResultIndex;
		}

	private:
		TArray<FMatchPlayInitialRouteRollProviderResult> ConfiguredResults;
		int32 NextResultIndex = 0;
		int32 CallCount = 0;
		TArray<EMatchPlayCurrentAttackResolutionRollPurpose> PurposeHistory;
	};

	bool AreResultsEqual(
		const FMatchPlayInitialRouteRollProviderResult& Left,
		const FMatchPlayInitialRouteRollProviderResult& Right)
	{
		return Left.bSuccess == Right.bSuccess
			&& Left.RawD6 == Right.RawD6
			&& Left.ErrorCode == Right.ErrorCode
			&& Left.ErrorMessage == Right.ErrorMessage;
	}

	bool ExpectResultsEqual(
		FAutomationTestBase& Test,
		const FString& Context,
		const FMatchPlayInitialRouteRollProviderResult& Left,
		const FMatchPlayInitialRouteRollProviderResult& Right)
	{
		bool bValid = true;
		bValid &= Test.TestEqual(
			*FString::Printf(TEXT("%s bSuccess"), *Context),
			Left.bSuccess,
			Right.bSuccess);
		bValid &= Test.TestEqual(
			*FString::Printf(TEXT("%s RawD6"), *Context),
			Left.RawD6,
			Right.RawD6);
		bValid &= Test.TestEqual(
			*FString::Printf(TEXT("%s ErrorCode"), *Context),
			Left.ErrorCode,
			Right.ErrorCode);
		bValid &= Test.TestEqual(
			*FString::Printf(TEXT("%s ErrorMessage"), *Context),
			Left.ErrorMessage,
			Right.ErrorMessage);
		bValid &= Test.TestTrue(
			*FString::Printf(TEXT("%s complete comparator"), *Context),
			AreResultsEqual(Left, Right));
		return bValid;
	}

	bool ExpectSuccess(
		FAutomationTestBase& Test,
		const FString& Context,
		const FMatchPlayInitialRouteRollProviderResult& Result,
		const int32 ExpectedD6)
	{
		bool bValid = true;
		bValid &= Test.TestTrue(*Context, Result.bSuccess);
		bValid &= Test.TestEqual(
			*FString::Printf(TEXT("%s RawD6"), *Context),
			Result.RawD6,
			ExpectedD6);
		bValid &= Test.TestEqual(
			*FString::Printf(TEXT("%s ErrorCode"), *Context),
			Result.ErrorCode,
			EMatchPlayInitialRouteRollProviderErrorCode::None);
		bValid &= Test.TestTrue(
			*FString::Printf(TEXT("%s ErrorMessage empty"), *Context),
			Result.ErrorMessage.IsEmpty());
		return bValid;
	}

	bool ExpectFailure(
		FAutomationTestBase& Test,
		const FString& Context,
		const FMatchPlayInitialRouteRollProviderResult& Result,
		const EMatchPlayInitialRouteRollProviderErrorCode ExpectedError,
		const FString& ExpectedMessage)
	{
		bool bValid = true;
		bValid &= Test.TestFalse(*Context, Result.bSuccess);
		bValid &= Test.TestEqual(
			*FString::Printf(TEXT("%s RawD6"), *Context),
			Result.RawD6,
			0);
		bValid &= Test.TestEqual(
			*FString::Printf(TEXT("%s ErrorCode"), *Context),
			Result.ErrorCode,
			ExpectedError);
		bValid &= Test.TestEqual(
			*FString::Printf(TEXT("%s ErrorMessage"), *Context),
			Result.ErrorMessage,
			ExpectedMessage);
		return bValid;
	}

	struct FFakeRunObservation
	{
		TArray<FMatchPlayInitialRouteRollProviderResult> Results;
		int32 CallCount = 0;
		TArray<EMatchPlayCurrentAttackResolutionRollPurpose> PurposeHistory;
		int32 RemainingResultCount = 0;
	};

	FFakeRunObservation RunSingleConfiguredResult(
		const FMatchPlayInitialRouteRollProviderResult& ConfiguredResult)
	{
		FDeterministicFakeInitialRouteRollProvider Provider;
		Provider.Enqueue(ConfiguredResult);

		FFakeRunObservation Observation;
		Observation.Results.Add(Provider.RollD6(
			EMatchPlayCurrentAttackResolutionRollPurpose::InitialRoute));
		Observation.CallCount = Provider.GetCallCount();
		Observation.PurposeHistory = Provider.GetPurposeHistory();
		Observation.RemainingResultCount =
			Provider.GetRemainingResultCount();
		return Observation;
	}

	FFakeRunObservation RunConfiguredSequence()
	{
		FDeterministicFakeInitialRouteRollProvider Provider;
		Provider.Enqueue(MakeSuccess(2));
		Provider.Enqueue(MakeFailure(TEXT("Configured provider failure.")));
		Provider.Enqueue(MakeSuccess(6));
		Provider.Enqueue(MakeSuccess(1));

		FFakeRunObservation Observation;
		for (int32 Index = 0; Index < 4; ++Index)
		{
			Observation.Results.Add(Provider.RollD6(
				EMatchPlayCurrentAttackResolutionRollPurpose::InitialRoute));
		}
		Observation.CallCount = Provider.GetCallCount();
		Observation.PurposeHistory = Provider.GetPurposeHistory();
		Observation.RemainingResultCount =
			Provider.GetRemainingResultCount();
		return Observation;
	}

	bool ExpectObservationsEqual(
		FAutomationTestBase& Test,
		const FString& Context,
		const FFakeRunObservation& Left,
		const FFakeRunObservation& Right)
	{
		bool bValid = true;
		bValid &= Test.TestEqual(
			*FString::Printf(TEXT("%s result count"), *Context),
			Left.Results.Num(),
			Right.Results.Num());
		const int32 ResultCount = FMath::Min(
			Left.Results.Num(),
			Right.Results.Num());
		for (int32 Index = 0; Index < ResultCount; ++Index)
		{
			bValid &= ExpectResultsEqual(
				Test,
				FString::Printf(TEXT("%s result %d"), *Context, Index),
				Left.Results[Index],
				Right.Results[Index]);
		}
		bValid &= Test.TestEqual(
			*FString::Printf(TEXT("%s call count"), *Context),
			Left.CallCount,
			Right.CallCount);
		bValid &= Test.TestEqual(
			*FString::Printf(TEXT("%s Purpose history count"), *Context),
			Left.PurposeHistory.Num(),
			Right.PurposeHistory.Num());
		const int32 PurposeCount = FMath::Min(
			Left.PurposeHistory.Num(),
			Right.PurposeHistory.Num());
		for (int32 Index = 0; Index < PurposeCount; ++Index)
		{
			bValid &= Test.TestEqual(
				*FString::Printf(
					TEXT("%s Purpose %d"),
					*Context,
					Index),
				Left.PurposeHistory[Index],
				Right.PurposeHistory[Index]);
		}
		bValid &= Test.TestEqual(
			*FString::Printf(TEXT("%s remaining"), *Context),
			Left.RemainingResultCount,
			Right.RemainingResultCount);
		return bValid;
	}
}

#define INITIAL_ROUTE_ROLL_PROVIDER_TEST(ClassName, TestName) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
		ClassName, \
		"FMCodex.CoreRules.MatchPlayInitialRouteRollProvider." TestName, \
		EAutomationTestFlags::EditorContext \
			| EAutomationTestFlags::EngineFilter)

INITIAL_ROUTE_ROLL_PROVIDER_TEST(
	FMatchPlayInitialRouteRollProviderSurfaceTest,
	"01.Surface")

bool FMatchPlayInitialRouteRollProviderSurfaceTest::RunTest(
	const FString& Parameters)
{
	using FExpectedRollSignature =
		FMatchPlayInitialRouteRollProviderResult
		(IMatchPlayInitialRouteRollProvider::*)(
			EMatchPlayCurrentAttackResolutionRollPurpose);

	TestTrue(
		TEXT("Provider is an abstract C++ interface"),
		std::is_abstract_v<IMatchPlayInitialRouteRollProvider>);
	TestFalse(
		TEXT("Provider is not a UObject"),
		std::is_base_of_v<UObject, IMatchPlayInitialRouteRollProvider>);
	TestTrue(
		TEXT("RollD6 accepts only Purpose and returns structured Result"),
		(std::is_same_v<
			decltype(&IMatchPlayInitialRouteRollProvider::RollD6),
			FExpectedRollSignature>));
	TestTrue(
		TEXT("Result is a value type"),
		std::is_copy_constructible_v<
			FMatchPlayInitialRouteRollProviderResult>
			&& std::is_copy_assignable_v<
				FMatchPlayInitialRouteRollProviderResult>);

	const FMatchPlayInitialRouteRollProviderResult DefaultResult;
	TestFalse(TEXT("Default Result is not success"), DefaultResult.bSuccess);
	TestEqual(TEXT("Default RawD6 is zero"), DefaultResult.RawD6, 0);
	TestEqual(
		TEXT("Default ErrorCode is None"),
		DefaultResult.ErrorCode,
		EMatchPlayInitialRouteRollProviderErrorCode::None);
	TestTrue(
		TEXT("Default ErrorMessage is empty"),
		DefaultResult.ErrorMessage.IsEmpty());

	const FMatchPlayInitialRouteRollProviderResult ComparatorBaseline =
		MatchPlayInitialRouteRollProviderTests::MakeSuccess(3);
	FMatchPlayInitialRouteRollProviderResult EqualComparatorValue =
		ComparatorBaseline;
	TestTrue(
		TEXT("Comparator accepts independent equal values"),
		MatchPlayInitialRouteRollProviderTests::AreResultsEqual(
			ComparatorBaseline,
			EqualComparatorValue));
	FMatchPlayInitialRouteRollProviderResult DifferentSuccess =
		ComparatorBaseline;
	DifferentSuccess.bSuccess = false;
	TestFalse(
		TEXT("Comparator covers bSuccess"),
		MatchPlayInitialRouteRollProviderTests::AreResultsEqual(
			ComparatorBaseline,
			DifferentSuccess));
	FMatchPlayInitialRouteRollProviderResult DifferentRawD6 =
		ComparatorBaseline;
	DifferentRawD6.RawD6 = 4;
	TestFalse(
		TEXT("Comparator covers RawD6"),
		MatchPlayInitialRouteRollProviderTests::AreResultsEqual(
			ComparatorBaseline,
			DifferentRawD6));
	FMatchPlayInitialRouteRollProviderResult DifferentErrorCode =
		ComparatorBaseline;
	DifferentErrorCode.ErrorCode =
		EMatchPlayInitialRouteRollProviderErrorCode::ProviderFailure;
	TestFalse(
		TEXT("Comparator covers ErrorCode"),
		MatchPlayInitialRouteRollProviderTests::AreResultsEqual(
			ComparatorBaseline,
			DifferentErrorCode));
	FMatchPlayInitialRouteRollProviderResult DifferentErrorMessage =
		ComparatorBaseline;
	DifferentErrorMessage.ErrorMessage = TEXT("Different diagnostic.");
	TestFalse(
		TEXT("Comparator covers ErrorMessage"),
		MatchPlayInitialRouteRollProviderTests::AreResultsEqual(
			ComparatorBaseline,
			DifferentErrorMessage));
	return true;
}

INITIAL_ROUTE_ROLL_PROVIDER_TEST(
	FMatchPlayInitialRouteRollProviderLegalD6Test,
	"02.LegalD6OneToSix")

bool FMatchPlayInitialRouteRollProviderLegalD6Test::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayInitialRouteRollProviderTests;
	bool bValid = true;
	for (int32 D6 = 1; D6 <= 6; ++D6)
	{
		FMatchPlayInitialRouteRollProviderResult Baseline;
		for (int32 RunIndex = 0; RunIndex < 3; ++RunIndex)
		{
			FDeterministicFakeInitialRouteRollProvider Provider;
			Provider.Enqueue(MakeSuccess(D6));
			const FMatchPlayInitialRouteRollProviderResult Result =
				Provider.RollD6(
					EMatchPlayCurrentAttackResolutionRollPurpose
						::InitialRoute);
			const FString Context = FString::Printf(
				TEXT("D6 %d run %d"),
				D6,
				RunIndex + 1);
			bValid &= ExpectSuccess(*this, Context, Result, D6);
			bValid &= TestEqual(
				*FString::Printf(TEXT("%s call count"), *Context),
				Provider.GetCallCount(),
				1);
			bValid &= TestEqual(
				*FString::Printf(TEXT("%s history count"), *Context),
				Provider.GetPurposeHistory().Num(),
				1);
			bValid &= TestEqual(
				*FString::Printf(TEXT("%s Purpose"), *Context),
				Provider.GetPurposeHistory()[0],
				EMatchPlayCurrentAttackResolutionRollPurpose
					::InitialRoute);
			bValid &= TestEqual(
				*FString::Printf(TEXT("%s remaining"), *Context),
				Provider.GetRemainingResultCount(),
				0);
			if (RunIndex == 0)
			{
				Baseline = Result;
			}
			else
			{
				bValid &= ExpectResultsEqual(
					*this,
					Context + TEXT(" equals baseline"),
					Baseline,
					Result);
			}
		}
	}
	return bValid;
}

INITIAL_ROUTE_ROLL_PROVIDER_TEST(
	FMatchPlayInitialRouteRollProviderInvalidRawD6Test,
	"03.InvalidRawD6Injection")

bool FMatchPlayInitialRouteRollProviderInvalidRawD6Test::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayInitialRouteRollProviderTests;
	bool bValid = true;
	for (const int32 RawD6 : {0, 7})
	{
		const FFakeRunObservation First =
			RunSingleConfiguredResult(MakeSuccess(RawD6));
		const FFakeRunObservation Second =
			RunSingleConfiguredResult(MakeSuccess(RawD6));
		const FFakeRunObservation Third =
			RunSingleConfiguredResult(MakeSuccess(RawD6));
		const FString CaseContext = FString::Printf(
			TEXT("RawD6=%d"),
			RawD6);
		const FFakeRunObservation* Runs[] = {&First, &Second, &Third};
		for (int32 RunIndex = 0; RunIndex < 3; ++RunIndex)
		{
			const FFakeRunObservation& Observation = *Runs[RunIndex];
			const FString RunContext = FString::Printf(
				TEXT("%s run %d"),
				*CaseContext,
				RunIndex + 1);
			bValid &= TestEqual(
				*(RunContext + TEXT(" result count")),
				Observation.Results.Num(),
				1);
			if (Observation.Results.Num() == 1)
			{
				bValid &= ExpectSuccess(
					*this,
					RunContext,
					Observation.Results[0],
					RawD6);
			}
			bValid &= TestEqual(
				*(RunContext + TEXT(" call count")),
				Observation.CallCount,
				1);
			bValid &= TestEqual(
				*(RunContext + TEXT(" Purpose history count")),
				Observation.PurposeHistory.Num(),
				1);
			if (Observation.PurposeHistory.Num() == 1)
			{
				bValid &= TestEqual(
					*(RunContext + TEXT(" Purpose")),
					Observation.PurposeHistory[0],
					EMatchPlayCurrentAttackResolutionRollPurpose
						::InitialRoute);
			}
			bValid &= TestEqual(
				*(RunContext + TEXT(" remaining")),
				Observation.RemainingResultCount,
				0);
		}
		bValid &= ExpectObservationsEqual(
			*this,
			CaseContext + TEXT(" run 1 versus run 2"),
			First,
			Second);
		bValid &= ExpectObservationsEqual(
			*this,
			CaseContext + TEXT(" run 1 versus run 3"),
			First,
			Third);
	}
	return bValid;
}

INITIAL_ROUTE_ROLL_PROVIDER_TEST(
	FMatchPlayInitialRouteRollProviderFailureTest,
	"04.ProviderFailure")

bool FMatchPlayInitialRouteRollProviderFailureTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayInitialRouteRollProviderTests;
	const FString ConfiguredMessage = TEXT("Configured provider failure.");
	bool bValid = true;
	FMatchPlayInitialRouteRollProviderResult ConfiguredBaseline;
	FMatchPlayInitialRouteRollProviderResult EmptyBaseline;
	for (int32 RunIndex = 0; RunIndex < 3; ++RunIndex)
	{
		const FString RunContext = FString::Printf(
			TEXT("run %d"),
			RunIndex + 1);
		FDeterministicFakeInitialRouteRollProvider ConfiguredProvider;
		ConfiguredProvider.Enqueue(MakeFailure(ConfiguredMessage));
		const FMatchPlayInitialRouteRollProviderResult ConfiguredResult =
			ConfiguredProvider.RollD6(
				EMatchPlayCurrentAttackResolutionRollPurpose::InitialRoute);
		bValid &= ExpectFailure(
			*this,
			TEXT("Explicit failure ") + RunContext,
			ConfiguredResult,
			EMatchPlayInitialRouteRollProviderErrorCode::ProviderFailure,
			ConfiguredMessage);
		bValid &= TestEqual(
			*(TEXT("Explicit failure call count ") + RunContext),
			ConfiguredProvider.GetCallCount(),
			1);
		bValid &= TestEqual(
			*(TEXT("Explicit failure Purpose history count ") + RunContext),
			ConfiguredProvider.GetPurposeHistory().Num(),
			1);
		if (ConfiguredProvider.GetPurposeHistory().Num() == 1)
		{
			bValid &= TestEqual(
				*(TEXT("Explicit failure Purpose ") + RunContext),
				ConfiguredProvider.GetPurposeHistory()[0],
				EMatchPlayCurrentAttackResolutionRollPurpose::InitialRoute);
		}
		bValid &= TestEqual(
			*(TEXT("Explicit failure consumed queue ") + RunContext),
			ConfiguredProvider.GetRemainingResultCount(),
			0);

		FDeterministicFakeInitialRouteRollProvider EmptyProvider;
		const FMatchPlayInitialRouteRollProviderResult EmptyResult =
			EmptyProvider.RollD6(
				EMatchPlayCurrentAttackResolutionRollPurpose::InitialRoute);
		bValid &= ExpectFailure(
			*this,
			TEXT("Empty queue failure ") + RunContext,
			EmptyResult,
			EMatchPlayInitialRouteRollProviderErrorCode::ProviderFailure,
			EmptyQueueMessage);
		bValid &= TestEqual(
			*(TEXT("Empty queue call count ") + RunContext),
			EmptyProvider.GetCallCount(),
			1);
		bValid &= TestEqual(
			*(TEXT("Empty queue Purpose history count ") + RunContext),
			EmptyProvider.GetPurposeHistory().Num(),
			1);
		if (EmptyProvider.GetPurposeHistory().Num() == 1)
		{
			bValid &= TestEqual(
				*(TEXT("Empty queue Purpose ") + RunContext),
				EmptyProvider.GetPurposeHistory()[0],
				EMatchPlayCurrentAttackResolutionRollPurpose::InitialRoute);
		}
		bValid &= TestEqual(
			*(TEXT("Empty queue remains empty ") + RunContext),
			EmptyProvider.GetRemainingResultCount(),
			0);

		if (RunIndex == 0)
		{
			ConfiguredBaseline = ConfiguredResult;
			EmptyBaseline = EmptyResult;
		}
		else
		{
			bValid &= ExpectResultsEqual(
				*this,
				TEXT("Explicit failure determinism ") + RunContext,
				ConfiguredBaseline,
				ConfiguredResult);
			bValid &= ExpectResultsEqual(
				*this,
				TEXT("Empty failure determinism ") + RunContext,
				EmptyBaseline,
				EmptyResult);
		}
	}
	return bValid;
}

INITIAL_ROUTE_ROLL_PROVIDER_TEST(
	FMatchPlayInitialRouteRollProviderInvalidPurposeTest,
	"05.InvalidPurposeDoesNotConsume")

bool FMatchPlayInitialRouteRollProviderInvalidPurposeTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayInitialRouteRollProviderTests;
	FDeterministicFakeInitialRouteRollProvider Provider;
	Provider.Enqueue(MakeSuccess(4));

	const FMatchPlayInitialRouteRollProviderResult InvalidResult =
		Provider.RollD6(
			EMatchPlayCurrentAttackResolutionRollPurpose::None);
	bool bValid = ExpectFailure(
		*this,
		TEXT("None Purpose"),
		InvalidResult,
		EMatchPlayInitialRouteRollProviderErrorCode::InvalidPurpose,
		InvalidPurposeMessage);
	bValid &= TestEqual(TEXT("None call counted"), Provider.GetCallCount(), 1);
	bValid &= TestEqual(
		TEXT("None Purpose recorded"),
		Provider.GetPurposeHistory()[0],
		EMatchPlayCurrentAttackResolutionRollPurpose::None);
	bValid &= TestEqual(
		TEXT("None does not consume queue"),
		Provider.GetRemainingResultCount(),
		1);

	const FMatchPlayInitialRouteRollProviderResult LegalResult =
		Provider.RollD6(
			EMatchPlayCurrentAttackResolutionRollPurpose::InitialRoute);
	bValid &= ExpectSuccess(
		*this,
		TEXT("Follow-up legal call"),
		LegalResult,
		4);
	bValid &= TestEqual(TEXT("Two attempted calls"), Provider.GetCallCount(), 2);
	bValid &= TestEqual(
		TEXT("Follow-up Purpose recorded"),
		Provider.GetPurposeHistory()[1],
		EMatchPlayCurrentAttackResolutionRollPurpose::InitialRoute);
	bValid &= TestEqual(
		TEXT("Follow-up consumes original head"),
		Provider.GetRemainingResultCount(),
		0);
	return bValid;
}

INITIAL_ROUTE_ROLL_PROVIDER_TEST(
	FMatchPlayInitialRouteRollProviderOrderedSequenceTest,
	"06.OrderedSequence")

bool FMatchPlayInitialRouteRollProviderOrderedSequenceTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayInitialRouteRollProviderTests;
	const FFakeRunObservation Observation = RunConfiguredSequence();
	bool bValid = true;
	bValid &= ExpectSuccess(*this, TEXT("First result"), Observation.Results[0], 2);
	bValid &= ExpectFailure(
		*this,
		TEXT("Second result"),
		Observation.Results[1],
		EMatchPlayInitialRouteRollProviderErrorCode::ProviderFailure,
		TEXT("Configured provider failure."));
	bValid &= ExpectSuccess(*this, TEXT("Third result"), Observation.Results[2], 6);
	bValid &= ExpectSuccess(*this, TEXT("Fourth result"), Observation.Results[3], 1);
	bValid &= TestEqual(TEXT("Ordered call count"), Observation.CallCount, 4);
	bValid &= TestEqual(
		TEXT("Ordered Purpose history length"),
		Observation.PurposeHistory.Num(),
		4);
	for (const EMatchPlayCurrentAttackResolutionRollPurpose Purpose :
		Observation.PurposeHistory)
	{
		bValid &= TestEqual(
			TEXT("Ordered Purpose is InitialRoute"),
			Purpose,
			EMatchPlayCurrentAttackResolutionRollPurpose::InitialRoute);
	}
	bValid &= TestEqual(
		TEXT("Ordered queue exhausted"),
		Observation.RemainingResultCount,
		0);
	return bValid;
}

INITIAL_ROUTE_ROLL_PROVIDER_TEST(
	FMatchPlayInitialRouteRollProviderIsolationTest,
	"07.InstanceIsolation")

bool FMatchPlayInitialRouteRollProviderIsolationTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayInitialRouteRollProviderTests;
	FDeterministicFakeInitialRouteRollProvider ProviderA;
	ProviderA.Enqueue(MakeSuccess(1));
	ProviderA.Enqueue(MakeSuccess(2));
	FDeterministicFakeInitialRouteRollProvider ProviderB;
	ProviderB.Enqueue(MakeSuccess(5));
	ProviderB.Enqueue(MakeSuccess(6));

	const auto Purpose =
		EMatchPlayCurrentAttackResolutionRollPurpose::InitialRoute;
	bool bValid = true;
	bValid &= ExpectSuccess(*this, TEXT("A1"), ProviderA.RollD6(Purpose), 1);
	bValid &= ExpectSuccess(*this, TEXT("B1"), ProviderB.RollD6(Purpose), 5);
	bValid &= ExpectSuccess(*this, TEXT("A2"), ProviderA.RollD6(Purpose), 2);
	bValid &= TestEqual(TEXT("A call count"), ProviderA.GetCallCount(), 2);
	bValid &= TestEqual(TEXT("B call count"), ProviderB.GetCallCount(), 1);
	bValid &= TestEqual(TEXT("A remaining"), ProviderA.GetRemainingResultCount(), 0);
	bValid &= TestEqual(TEXT("B remaining"), ProviderB.GetRemainingResultCount(), 1);
	bValid &= ExpectSuccess(*this, TEXT("B2"), ProviderB.RollD6(Purpose), 6);
	bValid &= TestEqual(TEXT("B final call count"), ProviderB.GetCallCount(), 2);
	bValid &= TestEqual(TEXT("B final remaining"), ProviderB.GetRemainingResultCount(), 0);
	bValid &= TestEqual(
		TEXT("A Purpose history count"),
		ProviderA.GetPurposeHistory().Num(),
		2);
	for (const EMatchPlayCurrentAttackResolutionRollPurpose RecordedPurpose :
		ProviderA.GetPurposeHistory())
	{
		bValid &= TestEqual(
			TEXT("A Purpose history entry"),
			RecordedPurpose,
			EMatchPlayCurrentAttackResolutionRollPurpose::InitialRoute);
	}
	bValid &= TestEqual(
		TEXT("B Purpose history count"),
		ProviderB.GetPurposeHistory().Num(),
		2);
	for (const EMatchPlayCurrentAttackResolutionRollPurpose RecordedPurpose :
		ProviderB.GetPurposeHistory())
	{
		bValid &= TestEqual(
			TEXT("B Purpose history entry"),
			RecordedPurpose,
			EMatchPlayCurrentAttackResolutionRollPurpose::InitialRoute);
	}
	return bValid;
}

INITIAL_ROUTE_ROLL_PROVIDER_TEST(
	FMatchPlayInitialRouteRollProviderDeterminismTest,
	"08.IndependentRunDeterminism")

bool FMatchPlayInitialRouteRollProviderDeterminismTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayInitialRouteRollProviderTests;
	const FFakeRunObservation First = RunConfiguredSequence();
	const FFakeRunObservation Second = RunConfiguredSequence();
	const FFakeRunObservation Third = RunConfiguredSequence();
	bool bValid = ExpectObservationsEqual(
		*this,
		TEXT("First versus second"),
		First,
		Second);
	bValid &= ExpectObservationsEqual(
		*this,
		TEXT("First versus third"),
		First,
		Third);
	return bValid;
}

INITIAL_ROUTE_ROLL_PROVIDER_TEST(
	FMatchPlayInitialRouteRollProviderStateIsolationTest,
	"09.StateUnchanged")

bool FMatchPlayInitialRouteRollProviderStateIsolationTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayInitialRouteRollProviderTests;
	FMatchPlayState State;
	State.RuntimeState.bIsInitialized = true;
	State.bHasCurrentAttack = true;
	State.CurrentAttack.AttackSequence = 91;
	const FMatchPlayState BeforeState = State;

	FDeterministicFakeInitialRouteRollProvider Provider;
	Provider.Enqueue(MakeSuccess(3));
	const FMatchPlayInitialRouteRollProviderResult Result =
		Provider.RollD6(
			EMatchPlayCurrentAttackResolutionRollPurpose::InitialRoute);
	bool bValid = ExpectSuccess(*this, TEXT("State isolation roll"), Result, 3);
	bValid &= TestTrue(
		TEXT("Provider call cannot change unrelated MatchPlay State"),
		FMatchPlayState::StaticStruct()->CompareScriptStruct(
			&State,
			&BeforeState,
			0));
	return bValid;
}

#undef INITIAL_ROUTE_ROLL_PROVIDER_TEST

#endif
