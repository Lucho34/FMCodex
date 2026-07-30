#include "MatchPlayCurrentAttackBranchIntentSelectionWriter.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "MatchPlayCurrentAttackBranchIntentSelectionTestFixtures.h"
#include "Misc/AutomationTest.h"

namespace BranchIntentSelectionFailureTests
{
	namespace Fixtures =
		FMCodex::Tests::MatchPlayCurrentAttackBranchIntentSelection;
	namespace HelperFixtures =
		FMCodex::Tests::MatchPlayCurrentAttackHelperSelection;

	bool IsStateEqual(
		const FMatchPlayState& Left,
		const FMatchPlayState& Right)
	{
		return Fixtures::AreStatesEqual(Left, Right);
	}

	bool ReportMismatch(
		FString& OutMismatch,
		const FString& FieldPath,
		const FString& ExpectedValue,
		const FString& ActualValue)
	{
		OutMismatch = FString::Printf(
			TEXT("%s differs: Expected=%s, Actual=%s"),
			*FieldPath,
			*ExpectedValue,
			*ActualValue);
		return false;
	}

	bool CompareBoolValue(
		const bool bExpected,
		const bool bActual,
		const FString& FieldPath,
		FString& OutMismatch)
	{
		if (bExpected == bActual)
		{
			return true;
		}
		return ReportMismatch(
			OutMismatch,
			FieldPath,
			bExpected ? TEXT("true") : TEXT("false"),
			bActual ? TEXT("true") : TEXT("false"));
	}

	template <typename TValue>
	bool CompareScalarValue(
		const TValue Expected,
		const TValue Actual,
		const FString& FieldPath,
		FString& OutMismatch)
	{
		if (Expected == Actual)
		{
			return true;
		}
		return ReportMismatch(
			OutMismatch,
			FieldPath,
			FString::Printf(
				TEXT("%lld"),
				static_cast<int64>(Expected)),
			FString::Printf(
				TEXT("%lld"),
				static_cast<int64>(Actual)));
	}

	bool CompareNameValue(
		const FName Expected,
		const FName Actual,
		const FString& FieldPath,
		FString& OutMismatch)
	{
		if (Expected == Actual)
		{
			return true;
		}
		return ReportMismatch(
			OutMismatch,
			FieldPath,
			Expected.ToString(),
			Actual.ToString());
	}

	bool CompareStringValue(
		const FString& Expected,
		const FString& Actual,
		const FString& FieldPath,
		FString& OutMismatch)
	{
		if (Expected == Actual)
		{
			return true;
		}
		return ReportMismatch(
			OutMismatch,
			FieldPath,
			FString::Printf(TEXT("\"%s\""), *Expected),
			FString::Printf(TEXT("\"%s\""), *Actual));
	}

	FString DescribeNameArray(const TArray<FName>& Values)
	{
		TArray<FString> Names;
		Names.Reserve(Values.Num());
		for (const FName Value : Values)
		{
			Names.Add(Value.ToString());
		}
		return FString::Printf(
			TEXT("[%s]"),
			*FString::Join(Names, TEXT(", ")));
	}

	bool CompareNameArrayValue(
		const TArray<FName>& Expected,
		const TArray<FName>& Actual,
		const FString& FieldPath,
		FString& OutMismatch)
	{
		if (Expected == Actual)
		{
			return true;
		}

		int32 FirstDifferenceIndex = INDEX_NONE;
		const int32 SharedCount =
			FMath::Min(Expected.Num(), Actual.Num());
		for (int32 Index = 0; Index < SharedCount; ++Index)
		{
			if (Expected[Index] != Actual[Index])
			{
				FirstDifferenceIndex = Index;
				break;
			}
		}
		const FString DiagnosticPath =
			FirstDifferenceIndex == INDEX_NONE
				? FString::Printf(
					TEXT("%s (length Expected=%d, Actual=%d)"),
					*FieldPath,
					Expected.Num(),
					Actual.Num())
				: FString::Printf(
					TEXT("%s (first differing index %d)"),
					*FieldPath,
					FirstDifferenceIndex);
		return ReportMismatch(
			OutMismatch,
			DiagnosticPath,
			DescribeNameArray(Expected),
			DescribeNameArray(Actual));
	}

	bool CompareScriptStructValue(
		const UScriptStruct* ScriptStruct,
		const void* Expected,
		const void* Actual,
		const FString& FieldPath,
		FString& OutMismatch)
	{
		if (ScriptStruct->CompareScriptStruct(
			Expected,
			Actual,
			0))
		{
			return true;
		}
		return ReportMismatch(
			OutMismatch,
			FieldPath,
			TEXT("<USTRUCT value>"),
			TEXT("<different USTRUCT value>"));
	}

	bool CompareSnapshotValidationResults(
		const FPlayerCardRuleSnapshotValidationResult& Expected,
		const FPlayerCardRuleSnapshotValidationResult& Actual,
		const FString& FieldPath,
		FString& OutMismatch)
	{
		return CompareBoolValue(
				Expected.bSuccess,
				Actual.bSuccess,
				FieldPath + TEXT(".bSuccess"),
				OutMismatch)
			&& CompareBoolValue(
				Expected.bIsValid,
				Actual.bIsValid,
				FieldPath + TEXT(".bIsValid"),
				OutMismatch)
			&& CompareScalarValue(
				Expected.ErrorCode,
				Actual.ErrorCode,
				FieldPath + TEXT(".ErrorCode"),
				OutMismatch)
			&& CompareStringValue(
				Expected.ErrorMessage,
				Actual.ErrorMessage,
				FieldPath + TEXT(".ErrorMessage"),
				OutMismatch)
			&& CompareNameValue(
				Expected.InvalidCardId,
				Actual.InvalidCardId,
				FieldPath + TEXT(".InvalidCardId"),
				OutMismatch)
			&& CompareNameArrayValue(
				Expected.DuplicateCardIds,
				Actual.DuplicateCardIds,
				FieldPath + TEXT(".DuplicateCardIds"),
				OutMismatch);
	}

	bool CompareSnapshotQueryResults(
		const FPlayerCardRuleSnapshotQueryResult& Expected,
		const FPlayerCardRuleSnapshotQueryResult& Actual,
		const FString& FieldPath,
		FString& OutMismatch)
	{
		return CompareBoolValue(
				Expected.bSuccess,
				Actual.bSuccess,
				FieldPath + TEXT(".bSuccess"),
				OutMismatch)
			&& CompareBoolValue(
				Expected.bFound,
				Actual.bFound,
				FieldPath + TEXT(".bFound"),
				OutMismatch)
			&& CompareScalarValue(
				Expected.ErrorCode,
				Actual.ErrorCode,
				FieldPath + TEXT(".ErrorCode"),
				OutMismatch)
			&& CompareStringValue(
				Expected.ErrorMessage,
				Actual.ErrorMessage,
				FieldPath + TEXT(".ErrorMessage"),
				OutMismatch)
			&& CompareNameValue(
				Expected.CardId,
				Actual.CardId,
				FieldPath + TEXT(".CardId"),
				OutMismatch)
			&& CompareScriptStructValue(
				FPlayerCardRuleSnapshot::StaticStruct(),
				&Expected.Snapshot,
				&Actual.Snapshot,
				FieldPath + TEXT(".Snapshot"),
				OutMismatch)
			&& CompareSnapshotValidationResults(
				Expected.ValidationResult,
				Actual.ValidationResult,
				FieldPath + TEXT(".ValidationResult"),
				OutMismatch);
	}

	bool CompareAuthorityQueryResults(
		const FMatchPlayCardSnapshotAuthorityQueryResult& Expected,
		const FMatchPlayCardSnapshotAuthorityQueryResult& Actual,
		const FString& FieldPath,
		FString& OutMismatch)
	{
		return CompareBoolValue(
				Expected.bSuccess,
				Actual.bSuccess,
				FieldPath + TEXT(".bSuccess"),
				OutMismatch)
			&& CompareScalarValue(
				Expected.ErrorCode,
				Actual.ErrorCode,
				FieldPath + TEXT(".ErrorCode"),
				OutMismatch)
			&& CompareScalarValue(
				Expected.PlayerSide,
				Actual.PlayerSide,
				FieldPath + TEXT(".PlayerSide"),
				OutMismatch)
			&& CompareNameValue(
				Expected.CardId,
				Actual.CardId,
				FieldPath + TEXT(".CardId"),
				OutMismatch)
			&& CompareScriptStructValue(
				FPlayerCardRuleSnapshot::StaticStruct(),
				&Expected.Snapshot,
				&Actual.Snapshot,
				FieldPath + TEXT(".Snapshot"),
				OutMismatch)
			&& CompareSnapshotQueryResults(
				Expected.UnderlyingQueryResult,
				Actual.UnderlyingQueryResult,
				FieldPath + TEXT(".UnderlyingQueryResult"),
				OutMismatch)
			&& CompareStringValue(
				Expected.ErrorMessage,
				Actual.ErrorMessage,
				FieldPath + TEXT(".ErrorMessage"),
				OutMismatch);
	}

	bool CompareHelperAuthorityResults(
		const FMatchPlayCurrentAttackHelperParticipantAuthorityResult&
			Expected,
		const FMatchPlayCurrentAttackHelperParticipantAuthorityResult&
			Actual,
		const FString& FieldPath,
		FString& OutMismatch)
	{
		return CompareBoolValue(
				Expected.bSuccess,
				Actual.bSuccess,
				FieldPath + TEXT(".bSuccess"),
				OutMismatch)
			&& CompareScalarValue(
				Expected.MatchingPlacementCount,
				Actual.MatchingPlacementCount,
				FieldPath + TEXT(".MatchingPlacementCount"),
				OutMismatch)
			&& CompareScriptStructValue(
				FMatchPlayDeploymentPlacement::StaticStruct(),
				&Expected.Placement,
				&Actual.Placement,
				FieldPath + TEXT(".Placement"),
				OutMismatch)
			&& CompareAuthorityQueryResults(
				Expected.SnapshotQueryResult,
				Actual.SnapshotQueryResult,
				FieldPath + TEXT(".SnapshotQueryResult"),
				OutMismatch)
			&& CompareScriptStructValue(
				FPlayerCardRuleSnapshot::StaticStruct(),
				&Expected.Snapshot,
				&Actual.Snapshot,
				FieldPath + TEXT(".Snapshot"),
				OutMismatch)
			&& CompareScalarValue(
				Expected.ErrorCode,
				Actual.ErrorCode,
				FieldPath + TEXT(".ErrorCode"),
				OutMismatch)
			&& CompareStringValue(
				Expected.ErrorMessage,
				Actual.ErrorMessage,
				FieldPath + TEXT(".ErrorMessage"),
				OutMismatch);
	}

	bool CompareGlobalContextResults(
		const FMatchPlayCurrentAttackBranchIntentSelectionGlobalContextResult&
			Expected,
		const FMatchPlayCurrentAttackBranchIntentSelectionGlobalContextResult&
			Actual,
		FString& OutMismatch)
	{
		OutMismatch.Reset();
		return CompareBoolValue(
				Expected.bSuccess,
				Actual.bSuccess,
				TEXT("bSuccess"),
				OutMismatch)
			&& CompareScalarValue(
				Expected.RequestedAttackSequence,
				Actual.RequestedAttackSequence,
				TEXT("RequestedAttackSequence"),
				OutMismatch)
			&& CompareScalarValue(
				Expected.AuthoritativeAttackSequence,
				Actual.AuthoritativeAttackSequence,
				TEXT("AuthoritativeAttackSequence"),
				OutMismatch)
			&& CompareScalarValue(
				Expected.RequestingSide,
				Actual.RequestingSide,
				TEXT("RequestingSide"),
				OutMismatch)
			&& CompareScalarValue(
				Expected.CurrentAttackingPlayer,
				Actual.CurrentAttackingPlayer,
				TEXT("CurrentAttackingPlayer"),
				OutMismatch)
			&& CompareScalarValue(
				Expected.CurrentDefendingPlayer,
				Actual.CurrentDefendingPlayer,
				TEXT("CurrentDefendingPlayer"),
				OutMismatch)
			&& CompareScriptStructValue(
				FMatchPlayCurrentAttackSelectionStateValidationResult
					::StaticStruct(),
				&Expected.SelectionStateValidationResult,
				&Actual.SelectionStateValidationResult,
				TEXT("SelectionStateValidationResult"),
				OutMismatch)
			&& CompareScriptStructValue(
				FMatchPlayCurrentAttackActionPreparationState
					::StaticStruct(),
				&Expected.Preparation,
				&Actual.Preparation,
				TEXT("Preparation"),
				OutMismatch)
			&& CompareScalarValue(
				Expected.FrozenActionType,
				Actual.FrozenActionType,
				TEXT("FrozenActionType"),
				OutMismatch)
			&& CompareScalarValue(
				Expected.MatchingCarrierPlacementCount,
				Actual.MatchingCarrierPlacementCount,
				TEXT("MatchingCarrierPlacementCount"),
				OutMismatch)
			&& CompareScalarValue(
				Expected.MatchingMarkerPlacementCount,
				Actual.MatchingMarkerPlacementCount,
				TEXT("MatchingMarkerPlacementCount"),
				OutMismatch)
			&& CompareScalarValue(
				Expected.MatchingRunnerPlacementCount,
				Actual.MatchingRunnerPlacementCount,
				TEXT("MatchingRunnerPlacementCount"),
				OutMismatch)
			&& CompareAuthorityQueryResults(
				Expected.CarrierSnapshotQueryResult,
				Actual.CarrierSnapshotQueryResult,
				TEXT("CarrierSnapshotQueryResult"),
				OutMismatch)
			&& CompareAuthorityQueryResults(
				Expected.MarkerSnapshotQueryResult,
				Actual.MarkerSnapshotQueryResult,
				TEXT("MarkerSnapshotQueryResult"),
				OutMismatch)
			&& CompareAuthorityQueryResults(
				Expected.RunnerSnapshotQueryResult,
				Actual.RunnerSnapshotQueryResult,
				TEXT("RunnerSnapshotQueryResult"),
				OutMismatch)
			&& CompareHelperAuthorityResults(
				Expected.HelperAuthorityResult,
				Actual.HelperAuthorityResult,
				TEXT("HelperAuthorityResult"),
				OutMismatch)
			&& CompareScalarValue(
				Expected.ErrorCode,
				Actual.ErrorCode,
				TEXT("ErrorCode"),
				OutMismatch)
			&& CompareStringValue(
				Expected.ErrorMessage,
				Actual.ErrorMessage,
				TEXT("ErrorMessage"),
				OutMismatch);
	}

	bool ExpectGlobalError(
		FAutomationTestBase& Test,
		const TCHAR* Label,
		const FMatchPlayState& State,
		const int64 AttackSequence,
		const EInitialTurnOrderPlayer RequestingSide,
		const EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
			ExpectedError)
	{
		const FMatchPlayState OriginalState = State;
		FMatchPlayCurrentAttackBranchIntentSelectionRequest Request;
		Request.AttackSequence = AttackSequence;
		Request.RequestingSide = RequestingSide;
		Request.Intent = EMatchPlayElectiveBranchIntent::None;
		const FMatchPlayCurrentAttackBranchIntentSelectionRequest
			OriginalRequest = Request;

		TArray<
			FMatchPlayCurrentAttackBranchIntentSelectionGlobalContextResult>
			Results;
		Results.Reserve(3);
		for (int32 Iteration = 1; Iteration <= 3; ++Iteration)
		{
			Results.Add(
				FMatchPlayCurrentAttackBranchIntentSelectionGlobalContextQuery
					::Query(
						State,
						Request.AttackSequence,
						Request.RequestingSide));
			const auto& Result = Results.Last();
			Test.TestFalse(
				*FString::Printf(
					TEXT("%s iteration %d fails"),
					Label,
					Iteration),
				Result.bSuccess);
			Test.TestEqual(
				*FString::Printf(
					TEXT("%s iteration %d exact error"),
					Label,
					Iteration),
				Result.ErrorCode,
				ExpectedError);
			Test.TestTrue(
				*FString::Printf(
					TEXT("%s iteration %d has diagnostics"),
					Label,
					Iteration),
				!Result.ErrorMessage.IsEmpty());
			if (Iteration > 1)
			{
				FString Mismatch;
				const bool bResultMatches =
					CompareGlobalContextResults(
						Results[0],
						Result,
						Mismatch);
				const FString ComparisonDiagnostic =
					bResultMatches
						? FString::Printf(
							TEXT("%s iteration %d full Result matches iteration 1"),
							Label,
							Iteration)
						: FString::Printf(
							TEXT("%s iteration %d Result mismatch against iteration 1: %s"),
							Label,
							Iteration,
							*Mismatch);
				Test.TestTrue(
					*ComparisonDiagnostic,
					bResultMatches);
			}
		}

		Test.TestTrue(
			*FString::Printf(
				TEXT("%s State unchanged after all Query calls"),
				Label),
			IsStateEqual(State, OriginalState));
		Test.TestTrue(
			*FString::Printf(
				TEXT("%s Request unchanged after all Query calls"),
				Label),
			FMatchPlayCurrentAttackBranchIntentSelectionRequest
				::StaticStruct()
				->CompareScriptStruct(
					&Request,
					&OriginalRequest,
					0));
		return true;
	}

	bool ExpectWriterFailureAtomic(
		FAutomationTestBase& Test,
		const TCHAR* Label,
		const FMatchPlayState& State,
		const FMatchPlayCurrentAttackBranchIntentSelectionRequest&
			Request,
		const EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
			ExpectedLegalityError,
		const EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
			ExpectedGlobalError =
				EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
					::None)
	{
		const auto Result =
			FMatchPlayCurrentAttackBranchIntentSelectionWriter::Select(
				State,
				Request);
		Test.TestFalse(
			*FString::Printf(TEXT("%s Writer fails"), Label),
			Result.bSuccess);
		Test.TestEqual(
			*FString::Printf(TEXT("%s exact Writer error"), Label),
			Result.ErrorCode,
			EMatchPlayCurrentAttackBranchIntentSelectionWriterErrorCode
				::LegalityFailed);
		Test.TestEqual(
			*FString::Printf(TEXT("%s exact Legality error"), Label),
			Result.LegalityResult.ErrorCode,
			ExpectedLegalityError);
		if (ExpectedLegalityError
			== EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
				::GlobalContextFailed)
		{
			Test.TestEqual(
				*FString::Printf(
					TEXT("%s exact Global Context error"),
					Label),
				Result.LegalityResult.GlobalContextResult.ErrorCode,
				ExpectedGlobalError);
		}
		Test.TestTrue(
			*FString::Printf(TEXT("%s BeforeState preserved"), Label),
			IsStateEqual(Result.BeforeState, State));
		Test.TestTrue(
			*FString::Printf(TEXT("%s failure is atomic"), Label),
			IsStateEqual(Result.AfterState, State));
		return true;
	}
}

#define BRANCH_INTENT_FAILURE_TEST(TestClass, TestName) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
		TestClass, \
		"FMCodex.CoreRules.MatchPlayCurrentAttackBranchIntentSelection.Failures." TestName, \
		EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

BRANCH_INTENT_FAILURE_TEST(
	FBranchIntentGlobalContextResultComparatorCoverageTest,
	"GlobalContextResultComparatorCoverage")

bool FBranchIntentGlobalContextResultComparatorCoverageTest::RunTest(
	const FString& Parameters)
{
	using namespace BranchIntentSelectionFailureTests;
	using FGlobalContextResult =
		FMatchPlayCurrentAttackBranchIntentSelectionGlobalContextResult;

	auto ExpectMismatch = [this](
		const TCHAR* CaseLabel,
		const FGlobalContextResult& Expected,
		const FGlobalContextResult& Actual,
		const TCHAR* ExpectedPath,
		const TCHAR* ExpectedField)
	{
		FString Mismatch;
		const bool bEqual = CompareGlobalContextResults(
			Expected,
			Actual,
			Mismatch);
		TestFalse(
			*FString::Printf(
				TEXT("%s - comparator rejects difference"),
				CaseLabel),
			bEqual);
		TestTrue(
			*FString::Printf(
				TEXT("%s - mismatch contains top-level path"),
				CaseLabel),
			Mismatch.Contains(ExpectedPath));
		TestTrue(
			*FString::Printf(
				TEXT("%s - mismatch contains field"),
				CaseLabel),
			Mismatch.Contains(ExpectedField));
		TestTrue(
			*FString::Printf(
				TEXT("%s - mismatch contains Expected value"),
				CaseLabel),
			Mismatch.Contains(TEXT("Expected=")));
		TestTrue(
			*FString::Printf(
				TEXT("%s - mismatch contains Actual value"),
				CaseLabel),
			Mismatch.Contains(TEXT("Actual=")));
	};

	const FName CardA(TEXT("Card.A"));
	const FName CardB(TEXT("Card.B"));
	const FName InvalidCard(TEXT("Card.Invalid"));
	const FGlobalContextResult Baseline;
	FString EqualMismatch(TEXT("sentinel"));
	TestTrue(
		TEXT("ComparatorCoverage.Equal - equal Results compare equal"),
		CompareGlobalContextResults(
			Baseline,
			Baseline,
			EqualMismatch));
	TestTrue(
		TEXT("ComparatorCoverage.Equal - equal mismatch is empty"),
		EqualMismatch.IsEmpty());

	FGlobalContextResult Actual = Baseline;
	Actual.CarrierSnapshotQueryResult.UnderlyingQueryResult
		.ValidationResult.InvalidCardId = InvalidCard;
	ExpectMismatch(
		TEXT("ComparatorCoverage.Carrier.InvalidCardId"),
		Baseline,
		Actual,
		TEXT("CarrierSnapshotQueryResult"),
		TEXT("InvalidCardId"));

	Actual = Baseline;
	Actual.MarkerSnapshotQueryResult.UnderlyingQueryResult
		.ValidationResult.InvalidCardId = InvalidCard;
	ExpectMismatch(
		TEXT("ComparatorCoverage.Marker.InvalidCardId"),
		Baseline,
		Actual,
		TEXT("MarkerSnapshotQueryResult"),
		TEXT("InvalidCardId"));

	Actual = Baseline;
	Actual.RunnerSnapshotQueryResult.UnderlyingQueryResult
		.ValidationResult.InvalidCardId = InvalidCard;
	ExpectMismatch(
		TEXT("ComparatorCoverage.Runner.InvalidCardId"),
		Baseline,
		Actual,
		TEXT("RunnerSnapshotQueryResult"),
		TEXT("InvalidCardId"));

	Actual = Baseline;
	Actual.HelperAuthorityResult.SnapshotQueryResult
		.UnderlyingQueryResult.ValidationResult.InvalidCardId =
			InvalidCard;
	ExpectMismatch(
		TEXT("ComparatorCoverage.Helper.InvalidCardId"),
		Baseline,
		Actual,
		TEXT("HelperAuthorityResult"),
		TEXT("InvalidCardId"));

	Actual = Baseline;
	Actual.CarrierSnapshotQueryResult.UnderlyingQueryResult
		.ValidationResult.DuplicateCardIds.Add(CardA);
	ExpectMismatch(
		TEXT("ComparatorCoverage.Carrier.DuplicateCardIds"),
		Baseline,
		Actual,
		TEXT("CarrierSnapshotQueryResult"),
		TEXT("DuplicateCardIds"));

	Actual = Baseline;
	Actual.MarkerSnapshotQueryResult.UnderlyingQueryResult
		.ValidationResult.DuplicateCardIds.Add(CardA);
	ExpectMismatch(
		TEXT("ComparatorCoverage.Marker.DuplicateCardIds"),
		Baseline,
		Actual,
		TEXT("MarkerSnapshotQueryResult"),
		TEXT("DuplicateCardIds"));

	Actual = Baseline;
	Actual.RunnerSnapshotQueryResult.UnderlyingQueryResult
		.ValidationResult.DuplicateCardIds.Add(CardA);
	ExpectMismatch(
		TEXT("ComparatorCoverage.Runner.DuplicateCardIds"),
		Baseline,
		Actual,
		TEXT("RunnerSnapshotQueryResult"),
		TEXT("DuplicateCardIds"));

	Actual = Baseline;
	Actual.HelperAuthorityResult.SnapshotQueryResult
		.UnderlyingQueryResult.ValidationResult.DuplicateCardIds
		.Add(CardA);
	ExpectMismatch(
		TEXT("ComparatorCoverage.Helper.DuplicateCardIds"),
		Baseline,
		Actual,
		TEXT("HelperAuthorityResult"),
		TEXT("DuplicateCardIds"));

	FGlobalContextResult ExpectedOrder = Baseline;
	ExpectedOrder.CarrierSnapshotQueryResult.UnderlyingQueryResult
		.ValidationResult.DuplicateCardIds.Add(CardA);
	ExpectedOrder.CarrierSnapshotQueryResult.UnderlyingQueryResult
		.ValidationResult.DuplicateCardIds.Add(CardB);
	FGlobalContextResult ActualOrder = Baseline;
	ActualOrder.CarrierSnapshotQueryResult.UnderlyingQueryResult
		.ValidationResult.DuplicateCardIds.Add(CardB);
	ActualOrder.CarrierSnapshotQueryResult.UnderlyingQueryResult
		.ValidationResult.DuplicateCardIds.Add(CardA);
	ExpectMismatch(
		TEXT("ComparatorCoverage.DuplicateCardIds.Order"),
		ExpectedOrder,
		ActualOrder,
		TEXT("CarrierSnapshotQueryResult"),
		TEXT("DuplicateCardIds"));

	FGlobalContextResult ExpectedDuplicates = Baseline;
	ExpectedDuplicates.HelperAuthorityResult.SnapshotQueryResult
		.UnderlyingQueryResult.ValidationResult.DuplicateCardIds
		.Add(CardA);
	ExpectedDuplicates.HelperAuthorityResult.SnapshotQueryResult
		.UnderlyingQueryResult.ValidationResult.DuplicateCardIds
		.Add(CardA);
	FGlobalContextResult ActualDuplicates = Baseline;
	ActualDuplicates.HelperAuthorityResult.SnapshotQueryResult
		.UnderlyingQueryResult.ValidationResult.DuplicateCardIds
		.Add(CardA);
	ExpectMismatch(
		TEXT("ComparatorCoverage.DuplicateCardIds.DuplicateCount"),
		ExpectedDuplicates,
		ActualDuplicates,
		TEXT("HelperAuthorityResult"),
		TEXT("DuplicateCardIds"));
	return true;
}

BRANCH_INTENT_FAILURE_TEST(
	FBranchIntentGlobalContextFailureMatrixTest,
	"GlobalContextMatrix")

bool FBranchIntentGlobalContextFailureMatrixTest::RunTest(
	const FString& Parameters)
{
	using namespace BranchIntentSelectionFailureTests;
	const FMatchPlayState Base =
		Fixtures::MakeAwaitingShotIntent(
			ESkillRuleType::LongShot);
	auto Query = [](const FMatchPlayState& State,
		const int64 Sequence,
		const EInitialTurnOrderPlayer Side)
	{
		return
			FMatchPlayCurrentAttackBranchIntentSelectionGlobalContextQuery
				::Query(State, Sequence, Side);
	};

	const auto Stale = Query(
		Base,
		Base.CurrentAttack.AttackSequence - 1,
		EInitialTurnOrderPlayer::PlayerB);
	TestFalse(TEXT("Stale sequence fails"), Stale.bSuccess);
	TestEqual(TEXT("Stale sequence is first error"),
		Stale.ErrorCode,
		EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
			::AttackSequenceMismatch);
	const auto Ahead = Query(
		Base,
		Base.CurrentAttack.AttackSequence + 1,
		EInitialTurnOrderPlayer::PlayerA);
	TestFalse(TEXT("Ahead sequence fails"), Ahead.bSuccess);
	TestEqual(TEXT("Ahead sequence exact error"),
		Ahead.ErrorCode,
		EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
			::AttackSequenceMismatch);
	const auto WrongSide = Query(
		Base,
		Base.CurrentAttack.AttackSequence,
		EInitialTurnOrderPlayer::PlayerB);
	TestFalse(TEXT("Defender request fails"),
		WrongSide.bSuccess);
	TestEqual(TEXT("Defender request exact error"),
		WrongSide.ErrorCode,
		EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
			::RequestingSideIsNotCurrentAttacker);
	const auto InvalidSide = Query(
		Base,
		Base.CurrentAttack.AttackSequence,
		EInitialTurnOrderPlayer::None);
	TestFalse(TEXT("Invalid side fails"),
		InvalidSide.bSuccess);
	TestEqual(TEXT("Invalid side exact error"),
		InvalidSide.ErrorCode,
		EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
			::InvalidRequestingSide);

	FMatchPlayState NoAttack = Base;
	NoAttack.bHasCurrentAttack = false;
	TestEqual(TEXT("No attack exact error"),
		Query(
			NoAttack,
			Base.CurrentAttack.AttackSequence,
			EInitialTurnOrderPlayer::PlayerA)
			.ErrorCode,
		EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
			::NoCurrentAttack);

	FMatchPlayState WrongPhase = Base;
	WrongPhase.CurrentAttack.Phase =
		EMatchPlayCurrentAttackPhase::Deployment;
	TestEqual(TEXT("Wrong phase exact error"),
		Query(
			WrongPhase,
			Base.CurrentAttack.AttackSequence,
			EInitialTurnOrderPlayer::PlayerA)
			.ErrorCode,
		EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
			::CurrentAttackNotInResolution);

	for (const EMatchPlayCurrentAttackSelectionStage Stage : {
		EMatchPlayCurrentAttackSelectionStage::AwaitingSkill,
		EMatchPlayCurrentAttackSelectionStage::AwaitingRunner,
		EMatchPlayCurrentAttackSelectionStage::AwaitingHelper,
		EMatchPlayCurrentAttackSelectionStage::ReadyForResolution})
	{
		FMatchPlayState WrongStage = Base;
		WrongStage.CurrentAttack.SelectionStage = Stage;
		const auto Result = Query(
			WrongStage,
			Base.CurrentAttack.AttackSequence,
			EInitialTurnOrderPlayer::PlayerA);
		TestFalse(TEXT("Wrong stage fails"), Result.bSuccess);
		TestEqual(TEXT("Wrong stage has exact error"),
			Result.ErrorCode,
			EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
				::WrongSelectionStage);
	}

	for (const ESkillRuleType ActionType : {
		ESkillRuleType::None,
		ESkillRuleType::PassControl,
		ESkillRuleType::ThroughBall})
	{
		FMatchPlayState InvalidSkill = Base;
		InvalidSkill.CurrentAttack.ActionPreparation.ActionType =
			ActionType;
		const auto Result = Query(
			InvalidSkill,
			Base.CurrentAttack.AttackSequence,
			EInitialTurnOrderPlayer::PlayerA);
		TestFalse(TEXT("Unsupported frozen skill fails"),
			Result.bSuccess);
		TestTrue(TEXT("Unsupported skill has diagnostics"),
			!Result.ErrorMessage.IsEmpty());
	}

	FMatchPlayState MissingSkill = Base;
	MissingSkill.CurrentAttack.ActionPreparation.SkillId =
		NAME_None;
	TestFalse(TEXT("Missing Skill fails"),
		Query(
			MissingSkill,
			Base.CurrentAttack.AttackSequence,
			EInitialTurnOrderPlayer::PlayerA)
			.bSuccess);

	FMatchPlayState CarrierDamage = Base;
	CarrierDamage.CurrentAttack.DeploymentPlacements.RemoveAll(
		[&CarrierDamage](
			const FMatchPlayDeploymentPlacement& Placement)
		{
			return Placement.PlayerSide
					== EInitialTurnOrderPlayer::PlayerA
				&& Placement.CardId
					== CarrierDamage.CurrentAttack
						.ActionPreparation.CarrierCardId;
		});
	TestEqual(TEXT("Carrier damage exact error"),
		Query(
			CarrierDamage,
			Base.CurrentAttack.AttackSequence,
			EInitialTurnOrderPlayer::PlayerA)
			.ErrorCode,
		EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
			::CarrierDeploymentInvalid);

	FMatchPlayState MarkerDamage = Base;
	MarkerDamage.CurrentAttack.DeploymentPlacements.RemoveAll(
		[&MarkerDamage](
			const FMatchPlayDeploymentPlacement& Placement)
		{
			return Placement.PlayerSide
					== EInitialTurnOrderPlayer::PlayerB
				&& Placement.CardId
					== MarkerDamage.CurrentAttack
						.ActionPreparation.MarkerCardId;
		});
	TestEqual(TEXT("Marker damage exact error"),
		Query(
			MarkerDamage,
			Base.CurrentAttack.AttackSequence,
			EInitialTurnOrderPlayer::PlayerA)
			.ErrorCode,
		EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
			::MarkerDeploymentInvalid);

	const FMatchPlayState Cross =
		Fixtures::MakeAwaitingCrossIntent(
			Fixtures::ECrossHelperPath::Selected);
	FMatchPlayState MissingRunner = Cross;
	MissingRunner.CurrentAttack.ActionPreparation.RunnerCardId =
		NAME_None;
	TestFalse(TEXT("Missing Cross Runner fails"),
		Query(
			MissingRunner,
			Cross.CurrentAttack.AttackSequence,
			EInitialTurnOrderPlayer::PlayerA)
			.bSuccess);

	FMatchPlayState DamagedHelper = Cross;
	DamagedHelper.CurrentAttack.ActionPreparation.bHasHelper =
		false;
	TestFalse(TEXT("Damaged Helper presence fails"),
		Query(
			DamagedHelper,
			Cross.CurrentAttack.AttackSequence,
			EInitialTurnOrderPlayer::PlayerA)
			.bSuccess);

	TestTrue(TEXT("Global Context is read-only"),
		IsStateEqual(
			Base,
			Fixtures::MakeAwaitingShotIntent(
				ESkillRuleType::LongShot)));
	return true;
}

BRANCH_INTENT_FAILURE_TEST(
	FBranchIntentGlobalContextFirstErrorCombinationTest,
	"GlobalContextFirstErrorCombinations")

bool FBranchIntentGlobalContextFirstErrorCombinationTest::RunTest(
	const FString& Parameters)
{
	using namespace BranchIntentSelectionFailureTests;
	const FMatchPlayState Base =
		Fixtures::MakeAwaitingShotIntent(
			ESkillRuleType::LongShot);
	const int64 Sequence = Base.CurrentAttack.AttackSequence;

	FMatchPlayState DamagedIdentityState = Base;
	DamagedIdentityState.CurrentAttack.SelectionStage =
		EMatchPlayCurrentAttackSelectionStage::AwaitingSkill;
	DamagedIdentityState.CurrentAttack.ActionPreparation.CarrierCardId =
		NAME_None;
	ExpectGlobalError(
		*this,
		TEXT("Stale sequence outranks damaged selection state"),
		DamagedIdentityState,
		Sequence - 1,
		EInitialTurnOrderPlayer::PlayerB,
		EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
			::AttackSequenceMismatch);
	ExpectGlobalError(
		*this,
		TEXT("Ahead sequence outranks damaged selection state"),
		DamagedIdentityState,
		Sequence + 1,
		EInitialTurnOrderPlayer::PlayerB,
		EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
			::AttackSequenceMismatch);
	ExpectGlobalError(
		*this,
		TEXT("Wrong side outranks damaged selection state"),
		DamagedIdentityState,
		Sequence,
		EInitialTurnOrderPlayer::PlayerB,
		EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
			::RequestingSideIsNotCurrentAttacker);
	ExpectGlobalError(
		*this,
		TEXT("Invalid side outranks damaged selection state"),
		DamagedIdentityState,
		Sequence,
		EInitialTurnOrderPlayer::None,
		EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
			::InvalidRequestingSide);

	FMatchPlayState Completed = DamagedIdentityState;
	Completed.bHasCurrentAttack = false;
	ExpectGlobalError(
		*this,
		TEXT("Completed attack outranks damaged selection state"),
		Completed,
		Sequence,
		EInitialTurnOrderPlayer::PlayerA,
		EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
			::NoCurrentAttack);

	FMatchPlayState WrongLifecycle = DamagedIdentityState;
	WrongLifecycle.CurrentAttack.Phase =
		EMatchPlayCurrentAttackPhase::Deployment;
	ExpectGlobalError(
		*this,
		TEXT("Lifecycle outranks stage and preparation damage"),
		WrongLifecycle,
		Sequence,
		EInitialTurnOrderPlayer::PlayerA,
		EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
			::CurrentAttackNotInResolution);

	struct FStageCombinationCase
	{
		EMatchPlayCurrentAttackSelectionStage Stage =
			EMatchPlayCurrentAttackSelectionStage::None;
		const TCHAR* CaseLabel = TEXT("");
	};
	const FStageCombinationCase StageCases[] = {
		{EMatchPlayCurrentAttackSelectionStage::AwaitingSkill,
			TEXT("AwaitingSkill stage outranks preparation damage")},
		{EMatchPlayCurrentAttackSelectionStage::AwaitingRunner,
			TEXT("AwaitingRunner stage outranks preparation damage")},
		{EMatchPlayCurrentAttackSelectionStage::AwaitingHelper,
			TEXT("AwaitingHelper stage outranks preparation damage")},
		{EMatchPlayCurrentAttackSelectionStage::ReadyForResolution,
			TEXT("ReadyForResolution stage outranks preparation damage")}
	};
	for (const FStageCombinationCase& StageCase : StageCases)
	{
		FMatchPlayState WrongStage = Base;
		WrongStage.CurrentAttack.SelectionStage =
			StageCase.Stage;
		WrongStage.CurrentAttack.ActionPreparation.CarrierCardId =
			NAME_None;
		ExpectGlobalError(
			*this,
			StageCase.CaseLabel,
			WrongStage,
			Sequence,
			EInitialTurnOrderPlayer::PlayerA,
			EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
				::WrongSelectionStage);
	}

	FMatchPlayState MissingCarrier = Base;
	MissingCarrier.CurrentAttack.ActionPreparation.CarrierCardId =
		NAME_None;
	ExpectGlobalError(
		*this,
		TEXT("Missing Carrier follows identity lifecycle and stage"),
		MissingCarrier,
		Sequence,
		EInitialTurnOrderPlayer::PlayerA,
		EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
			::InvalidSelectionState);

	FMatchPlayState MissingMarker = Base;
	MissingMarker.CurrentAttack.ActionPreparation.MarkerCardId =
		NAME_None;
	ExpectGlobalError(
		*this,
		TEXT("Missing Marker follows identity lifecycle and stage"),
		MissingMarker,
		Sequence,
		EInitialTurnOrderPlayer::PlayerA,
		EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
			::InvalidSelectionState);

	FMatchPlayState MissingSkill = Base;
	MissingSkill.CurrentAttack.ActionPreparation.SkillId =
		NAME_None;
	ExpectGlobalError(
		*this,
		TEXT("Missing Skill follows identity lifecycle and stage"),
		MissingSkill,
		Sequence,
		EInitialTurnOrderPlayer::PlayerA,
		EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
			::InvalidSelectionState);

	const FMatchPlayState Cross =
		Fixtures::MakeAwaitingCrossIntent(
			Fixtures::ECrossHelperPath::Selected);
	FMatchPlayState MissingRunner = Cross;
	MissingRunner.CurrentAttack.ActionPreparation.RunnerCardId =
		NAME_None;
	ExpectGlobalError(
		*this,
		TEXT("Missing Cross Runner is a post-stage structure error"),
		MissingRunner,
		MissingRunner.CurrentAttack.AttackSequence,
		EInitialTurnOrderPlayer::PlayerA,
		EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
			::InvalidSelectionState);

	FMatchPlayState PresentWithoutIdentity = Cross;
	PresentWithoutIdentity.CurrentAttack.ActionPreparation.HelperCardId =
		NAME_None;
	ExpectGlobalError(
		*this,
		TEXT("Present Helper requires identity"),
		PresentWithoutIdentity,
		PresentWithoutIdentity.CurrentAttack.AttackSequence,
		EInitialTurnOrderPlayer::PlayerA,
		EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
			::InvalidSelectionState);

	FMatchPlayState AbsentWithIdentity =
		Fixtures::MakeAwaitingCrossIntent(
			Fixtures::ECrossHelperPath::Declined);
	AbsentWithIdentity.CurrentAttack.ActionPreparation.HelperCardId =
		HelperFixtures::HelperId;
	ExpectGlobalError(
		*this,
		TEXT("Absent Helper forbids identity"),
		AbsentWithIdentity,
		AbsentWithIdentity.CurrentAttack.AttackSequence,
		EInitialTurnOrderPlayer::PlayerA,
		EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
			::InvalidSelectionState);
	return true;
}

BRANCH_INTENT_FAILURE_TEST(
	FBranchIntentCombinationFailureMatrixTest,
	"IntentCombinationMatrix")

bool FBranchIntentCombinationFailureMatrixTest::RunTest(
	const FString& Parameters)
{
	using namespace BranchIntentSelectionFailureTests;
	for (const ESkillRuleType ActionType : {
		ESkillRuleType::LongShot,
		ESkillRuleType::CutInsideShot})
	{
		const FMatchPlayState State =
			Fixtures::MakeAwaitingShotIntent(ActionType);
		for (const EMatchPlayElectiveBranchIntent Intent : {
			EMatchPlayElectiveBranchIntent::None,
			EMatchPlayElectiveBranchIntent::CrossHigh,
			EMatchPlayElectiveBranchIntent::CrossLow})
		{
			const auto Result =
				FMatchPlayCurrentAttackBranchIntentSelectionLegalityEvaluator
					::Evaluate(
						State,
						Fixtures::MakeRequest(State, Intent));
			TestFalse(TEXT("Shot invalid Intent fails"),
				Result.bIsLegal);
		}
	}

	const FMatchPlayState Cross =
		Fixtures::MakeAwaitingCrossIntent(
			Fixtures::ECrossHelperPath::Selected);
	for (const EMatchPlayElectiveBranchIntent Intent : {
		EMatchPlayElectiveBranchIntent::None,
		EMatchPlayElectiveBranchIntent::DirectShot,
		EMatchPlayElectiveBranchIntent::DeadCorner})
	{
		const auto Result =
			FMatchPlayCurrentAttackBranchIntentSelectionLegalityEvaluator
				::Evaluate(
					Cross,
					Fixtures::MakeRequest(Cross, Intent));
		TestFalse(TEXT("Cross invalid Intent fails"),
			Result.bIsLegal);
	}

	const auto UnknownIntent =
		static_cast<EMatchPlayElectiveBranchIntent>(255);
	const FMatchPlayState Shot =
		Fixtures::MakeAwaitingShotIntent(
			ESkillRuleType::LongShot);
	const auto Unknown =
		FMatchPlayCurrentAttackBranchIntentSelectionLegalityEvaluator
			::Evaluate(
				Shot,
				Fixtures::MakeRequest(Shot, UnknownIntent));
	TestFalse(TEXT("Unknown Intent safely fails"),
		Unknown.bIsLegal);
	TestEqual(TEXT("Unknown Intent exact error"),
		Unknown.ErrorCode,
		EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
			::InvalidIntent);
	return true;
}

BRANCH_INTENT_FAILURE_TEST(
	FBranchIntentWriterAtomicityTest,
	"WriterAtomicity")

bool FBranchIntentWriterAtomicityTest::RunTest(
	const FString& Parameters)
{
	using namespace BranchIntentSelectionFailureTests;
	const FMatchPlayState Shot =
		Fixtures::MakeAwaitingShotIntent(
			ESkillRuleType::LongShot);

	TArray<FMatchPlayCurrentAttackBranchIntentSelectionRequest>
		Requests;
	auto WrongSequence = Fixtures::MakeRequest(
		Shot,
		EMatchPlayElectiveBranchIntent::DirectShot);
	WrongSequence.AttackSequence++;
	Requests.Add(WrongSequence);
	auto WrongSide = Fixtures::MakeRequest(
		Shot,
		EMatchPlayElectiveBranchIntent::DirectShot);
	WrongSide.RequestingSide =
		EInitialTurnOrderPlayer::PlayerB;
	Requests.Add(WrongSide);
	Requests.Add(Fixtures::MakeRequest(
		Shot,
		EMatchPlayElectiveBranchIntent::CrossHigh));
	Requests.Add(Fixtures::MakeRequest(
		Shot,
		EMatchPlayElectiveBranchIntent::None));

	for (const auto& Request : Requests)
	{
		const auto Result =
			FMatchPlayCurrentAttackBranchIntentSelectionWriter
				::Select(Shot, Request);
		TestFalse(TEXT("Invalid Writer request fails"),
			Result.bSuccess);
		TestTrue(TEXT("Invalid Writer request atomic"),
			IsStateEqual(
				Result.BeforeState,
				Result.AfterState));
	}

	FMatchPlayState WrongStage = Shot;
	WrongStage.CurrentAttack.SelectionStage =
		EMatchPlayCurrentAttackSelectionStage::AwaitingSkill;
	const auto WrongStageResult =
		FMatchPlayCurrentAttackBranchIntentSelectionWriter::Select(
			WrongStage,
			Fixtures::MakeRequest(
				WrongStage,
				EMatchPlayElectiveBranchIntent::DirectShot));
	TestFalse(TEXT("Wrong stage Writer fails"),
		WrongStageResult.bSuccess);
	TestTrue(TEXT("Wrong stage Writer atomic"),
		IsStateEqual(
			WrongStageResult.BeforeState,
			WrongStageResult.AfterState));

	FMatchPlayState DamagedHelper =
		Fixtures::MakeAwaitingCrossIntent(
			Fixtures::ECrossHelperPath::Selected);
	DamagedHelper.CurrentAttack.ActionPreparation
		.HelperCardId = NAME_None;
	const auto DamagedHelperResult =
		FMatchPlayCurrentAttackBranchIntentSelectionWriter::Select(
			DamagedHelper,
			Fixtures::MakeRequest(
				DamagedHelper,
				EMatchPlayElectiveBranchIntent::CrossHigh));
	TestFalse(TEXT("Damaged Helper Writer fails"),
		DamagedHelperResult.bSuccess);
	TestTrue(TEXT("Damaged Helper Writer atomic"),
		IsStateEqual(
			DamagedHelperResult.BeforeState,
			DamagedHelperResult.AfterState));
	return true;
}

BRANCH_INTENT_FAILURE_TEST(
	FBranchIntentWriterAtomicityCompleteMatrixTest,
	"WriterAtomicityCompleteMatrix")

bool FBranchIntentWriterAtomicityCompleteMatrixTest::RunTest(
	const FString& Parameters)
{
	using namespace BranchIntentSelectionFailureTests;
	const FMatchPlayState Shot =
		Fixtures::MakeAwaitingShotIntent(
			ESkillRuleType::LongShot);
	const auto DirectShotRequest = Fixtures::MakeRequest(
		Shot,
		EMatchPlayElectiveBranchIntent::DirectShot);

	auto Stale = DirectShotRequest;
	--Stale.AttackSequence;
	ExpectWriterFailureAtomic(
		*this,
		TEXT("Stale AttackSequence"),
		Shot,
		Stale,
		EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
			::GlobalContextFailed,
		EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
			::AttackSequenceMismatch);

	auto Ahead = DirectShotRequest;
	++Ahead.AttackSequence;
	ExpectWriterFailureAtomic(
		*this,
		TEXT("Ahead AttackSequence"),
		Shot,
		Ahead,
		EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
			::GlobalContextFailed,
		EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
			::AttackSequenceMismatch);

	auto WrongSide = DirectShotRequest;
	WrongSide.RequestingSide = EInitialTurnOrderPlayer::PlayerB;
	ExpectWriterFailureAtomic(
		*this,
		TEXT("Wrong RequestingSide"),
		Shot,
		WrongSide,
		EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
			::GlobalContextFailed,
		EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
			::RequestingSideIsNotCurrentAttacker);

	auto InvalidSide = DirectShotRequest;
	InvalidSide.RequestingSide = EInitialTurnOrderPlayer::None;
	ExpectWriterFailureAtomic(
		*this,
		TEXT("Invalid RequestingSide"),
		Shot,
		InvalidSide,
		EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
			::GlobalContextFailed,
		EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
			::InvalidRequestingSide);

	FMatchPlayState Completed = Shot;
	Completed.bHasCurrentAttack = false;
	ExpectWriterFailureAtomic(
		*this,
		TEXT("Completed current attack"),
		Completed,
		DirectShotRequest,
		EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
			::GlobalContextFailed,
		EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
			::NoCurrentAttack);

	FMatchPlayState WrongLifecycle = Shot;
	WrongLifecycle.CurrentAttack.Phase =
		EMatchPlayCurrentAttackPhase::Deployment;
	ExpectWriterFailureAtomic(
		*this,
		TEXT("Current attack not in selection lifecycle"),
		WrongLifecycle,
		DirectShotRequest,
		EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
			::GlobalContextFailed,
		EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
			::CurrentAttackNotInResolution);

	FMatchPlayState NotYetAwaiting = Shot;
	NotYetAwaiting.CurrentAttack.SelectionStage =
		EMatchPlayCurrentAttackSelectionStage::AwaitingSkill;
	ExpectWriterFailureAtomic(
		*this,
		TEXT("Selection not yet awaiting Intent"),
		NotYetAwaiting,
		DirectShotRequest,
		EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
			::GlobalContextFailed,
		EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
			::WrongSelectionStage);

	const FMatchPlayState Ready = Fixtures::MakeReadyState(
		ESkillRuleType::LongShot,
		EMatchPlayElectiveBranchIntent::DirectShot);
	const auto RepeatRequest = Fixtures::MakeRequest(
		Ready,
		EMatchPlayElectiveBranchIntent::DirectShot);
	ExpectWriterFailureAtomic(
		*this,
		TEXT("Ready stage rejects Intent"),
		Ready,
		RepeatRequest,
		EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
			::GlobalContextFailed,
		EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
			::WrongSelectionStage);
	ExpectWriterFailureAtomic(
		*this,
		TEXT("Repeated Intent submission"),
		Ready,
		RepeatRequest,
		EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
			::GlobalContextFailed,
		EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
			::WrongSelectionStage);

	ExpectWriterFailureAtomic(
		*this,
		TEXT("Intent None"),
		Shot,
		Fixtures::MakeRequest(
			Shot,
			EMatchPlayElectiveBranchIntent::None),
		EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
			::InvalidIntent);
	ExpectWriterFailureAtomic(
		*this,
		TEXT("Shot rejects Cross family"),
		Shot,
		Fixtures::MakeRequest(
			Shot,
			EMatchPlayElectiveBranchIntent::CrossHigh),
		EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
			::IntentActionTypeMismatch);

	const FMatchPlayState Cross =
		Fixtures::MakeAwaitingCrossIntent(
			Fixtures::ECrossHelperPath::Selected);
	ExpectWriterFailureAtomic(
		*this,
		TEXT("Cross rejects Shot family"),
		Cross,
		Fixtures::MakeRequest(
			Cross,
			EMatchPlayElectiveBranchIntent::DirectShot),
		EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
			::IntentActionTypeMismatch);
	ExpectWriterFailureAtomic(
		*this,
		TEXT("Unknown Intent enum"),
		Shot,
		Fixtures::MakeRequest(
			Shot,
			static_cast<EMatchPlayElectiveBranchIntent>(255)),
		EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
			::InvalidIntent);

	FMatchPlayState MissingCarrier = Shot;
	MissingCarrier.CurrentAttack.ActionPreparation.CarrierCardId =
		NAME_None;
	ExpectWriterFailureAtomic(
		*this,
		TEXT("Missing Carrier"),
		MissingCarrier,
		DirectShotRequest,
		EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
			::GlobalContextFailed,
		EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
			::InvalidSelectionState);

	FMatchPlayState MissingMarker = Shot;
	MissingMarker.CurrentAttack.ActionPreparation.MarkerCardId =
		NAME_None;
	ExpectWriterFailureAtomic(
		*this,
		TEXT("Missing Marker"),
		MissingMarker,
		DirectShotRequest,
		EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
			::GlobalContextFailed,
		EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
			::InvalidSelectionState);

	FMatchPlayState MissingSkill = Shot;
	MissingSkill.CurrentAttack.ActionPreparation.SkillId =
		NAME_None;
	ExpectWriterFailureAtomic(
		*this,
		TEXT("Missing Skill"),
		MissingSkill,
		DirectShotRequest,
		EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
			::GlobalContextFailed,
		EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
			::InvalidSelectionState);

	FMatchPlayState MissingRunner = Cross;
	MissingRunner.CurrentAttack.ActionPreparation.RunnerCardId =
		NAME_None;
	ExpectWriterFailureAtomic(
		*this,
		TEXT("Missing Cross Runner"),
		MissingRunner,
		Fixtures::MakeRequest(
			MissingRunner,
			EMatchPlayElectiveBranchIntent::CrossHigh),
		EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
			::GlobalContextFailed,
		EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
			::InvalidSelectionState);

	FMatchPlayState PresentWithoutIdentity = Cross;
	PresentWithoutIdentity.CurrentAttack.ActionPreparation.HelperCardId =
		NAME_None;
	ExpectWriterFailureAtomic(
		*this,
		TEXT("Present Helper without identity"),
		PresentWithoutIdentity,
		Fixtures::MakeRequest(
			PresentWithoutIdentity,
			EMatchPlayElectiveBranchIntent::CrossHigh),
		EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
			::GlobalContextFailed,
		EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
			::InvalidSelectionState);

	FMatchPlayState AbsentWithIdentity =
		Fixtures::MakeAwaitingCrossIntent(
			Fixtures::ECrossHelperPath::Declined);
	AbsentWithIdentity.CurrentAttack.ActionPreparation.HelperCardId =
		HelperFixtures::HelperId;
	ExpectWriterFailureAtomic(
		*this,
		TEXT("Absent Helper with identity"),
		AbsentWithIdentity,
		Fixtures::MakeRequest(
			AbsentWithIdentity,
			EMatchPlayElectiveBranchIntent::CrossLow),
		EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
			::GlobalContextFailed,
		EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
			::InvalidSelectionState);

	for (const ESkillRuleType UnsupportedType : {
		ESkillRuleType::PassControl,
		ESkillRuleType::ThroughBall,
		static_cast<ESkillRuleType>(255)})
	{
		FMatchPlayState Unsupported =
			HelperFixtures::MakeState(
				UnsupportedType == static_cast<ESkillRuleType>(255)
					? ESkillRuleType::PassControl
					: UnsupportedType);
		Unsupported.CurrentAttack.SelectionStage =
			EMatchPlayCurrentAttackSelectionStage
				::AwaitingBranchIntent;
		Unsupported.CurrentAttack.ActionPreparation.ActionType =
			UnsupportedType;
		ExpectWriterFailureAtomic(
			*this,
			TEXT("Unsupported Skill"),
			Unsupported,
			Fixtures::MakeRequest(
				Unsupported,
				EMatchPlayElectiveBranchIntent::DirectShot),
			EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
				::GlobalContextFailed,
			EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
				::InvalidSelectionState);
	}
	return true;
}

#undef BRANCH_INTENT_FAILURE_TEST

#endif
