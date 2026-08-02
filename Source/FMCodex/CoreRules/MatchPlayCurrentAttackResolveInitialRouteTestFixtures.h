#pragma once

#if WITH_DEV_AUTOMATION_TESTS

#include "MatchPlayCurrentAttackBranchIntentSelectionTestFixtures.h"
#include "MatchPlayCurrentAttackResolutionSessionTestFixtures.h"
#include "MatchPlayCurrentAttackResolveInitialRouteWriter.h"

namespace FMCodex::Tests::MatchPlayCurrentAttackResolveInitialRoute
{
	namespace BranchIntentFixtures =
		FMCodex::Tests::MatchPlayCurrentAttackBranchIntentSelection;
	namespace SessionFixtures =
		FMCodex::Tests::MatchPlayCurrentAttackResolutionSession;

	inline FMatchPlayInitialRouteRollProviderResult MakeSuccess(
		const int32 RawD6)
	{
		FMatchPlayInitialRouteRollProviderResult Result;
		Result.bSuccess = true;
		Result.RawD6 = RawD6;
		return Result;
	}

	inline FMatchPlayInitialRouteRollProviderResult MakeFailure(
		const FString& ErrorMessage = TEXT("Configured provider failure."))
	{
		FMatchPlayInitialRouteRollProviderResult Result;
		Result.ErrorCode =
			EMatchPlayInitialRouteRollProviderErrorCode::ProviderFailure;
		Result.ErrorMessage = ErrorMessage;
		return Result;
	}

	class FQueueRollProvider final
		: public IMatchPlayInitialRouteRollProvider
	{
	public:
		void Enqueue(
			const FMatchPlayInitialRouteRollProviderResult& Result)
		{
			Results.Add(Result);
		}

		virtual FMatchPlayInitialRouteRollProviderResult RollD6(
			const EMatchPlayCurrentAttackResolutionRollPurpose Purpose)
			override
		{
			++CallCount;
			PurposeHistory.Add(Purpose);
			if (NextIndex >= Results.Num())
			{
				return MakeFailure(TEXT("Unexpected empty test queue."));
			}
			return Results[NextIndex++];
		}

		int32 GetCallCount() const
		{
			return CallCount;
		}

		int32 GetRemainingCount() const
		{
			return Results.Num() - NextIndex;
		}

		const TArray<EMatchPlayCurrentAttackResolutionRollPurpose>&
		GetPurposeHistory() const
		{
			return PurposeHistory;
		}

	private:
		TArray<FMatchPlayInitialRouteRollProviderResult> Results;
		int32 NextIndex = 0;
		int32 CallCount = 0;
		TArray<EMatchPlayCurrentAttackResolutionRollPurpose> PurposeHistory;
	};

	inline FMatchPlayState MakeAwaitingState(
		const ESkillRuleType ActionType,
		const EMatchPlayElectiveBranchIntent Intent =
			EMatchPlayElectiveBranchIntent::None)
	{
		FMatchPlayState ReadyState;
		if (ActionType == ESkillRuleType::LongShot
			|| ActionType == ESkillRuleType::CutInsideShot
			|| ActionType == ESkillRuleType::Cross)
		{
			ReadyState = BranchIntentFixtures::MakeReadyState(
				ActionType,
				Intent);
		}
		else
		{
			ReadyState = SessionFixtures::MakeReadyState(ActionType);
		}

		return
			FMatchPlayCurrentAttackBeginResolutionSessionWriter::Begin(
				ReadyState,
				SessionFixtures::MakeRequest(ReadyState))
			.AfterState;
	}

	inline FMatchPlayCurrentAttackResolveInitialRouteRequest MakeRequest(
		const FMatchPlayState& State)
	{
		FMatchPlayCurrentAttackResolveInitialRouteRequest Request;
		Request.AttackSequence = State.CurrentAttack.AttackSequence;
		return Request;
	}

	inline bool AreStatesEqual(
		const FMatchPlayState& Left,
		const FMatchPlayState& Right)
	{
		return FMatchPlayState::StaticStruct()->CompareScriptStruct(
			&Left,
			&Right,
			0);
	}

	inline bool AreBranchesEqual(
		const FMatchPlayCurrentAttackActualBranch& Left,
		const FMatchPlayCurrentAttackActualBranch& Right)
	{
		return FMatchPlayCurrentAttackActualBranch::StaticStruct()
			->CompareScriptStruct(&Left, &Right, 0);
	}

	inline bool AreBundlesEqual(
		const FMatchPlayCurrentAttackResolutionSessionBundle& Left,
		const FMatchPlayCurrentAttackResolutionSessionBundle& Right)
	{
		return FMatchPlayCurrentAttackResolutionSessionBundle::StaticStruct()
			->CompareScriptStruct(&Left, &Right, 0);
	}

	inline bool AreRollRecordsEqual(
		const TArray<FMatchPlayCurrentAttackResolutionRollRecord>& Left,
		const TArray<FMatchPlayCurrentAttackResolutionRollRecord>& Right)
	{
		if (Left.Num() != Right.Num())
		{
			return false;
		}
		for (int32 Index = 0; Index < Left.Num(); ++Index)
		{
			if (Left[Index].Purpose != Right[Index].Purpose
				|| Left[Index].RawD6 != Right[Index].RawD6)
			{
				return false;
			}
		}
		return true;
	}

	inline bool AreProviderResultsEqual(
		const FMatchPlayInitialRouteRollProviderResult& Left,
		const FMatchPlayInitialRouteRollProviderResult& Right)
	{
		return Left.bSuccess == Right.bSuccess
			&& Left.RawD6 == Right.RawD6
			&& Left.ErrorCode == Right.ErrorCode
			&& Left.ErrorMessage == Right.ErrorMessage;
	}

	inline bool AreMappingInputsEqual(
		const FMatchPlayCurrentAttackInitialRouteMappingInput& Left,
		const FMatchPlayCurrentAttackInitialRouteMappingInput& Right)
	{
		return Left.ActionType == Right.ActionType
			&& Left.Intent == Right.Intent
			&& Left.bHasInitialRouteD6 == Right.bHasInitialRouteD6
			&& Left.InitialRouteD6 == Right.InitialRouteD6;
	}

	inline bool AreMappingResultsEqual(
		const FMatchPlayCurrentAttackInitialRouteMappingResult& Left,
		const FMatchPlayCurrentAttackInitialRouteMappingResult& Right)
	{
		return Left.bSuccess == Right.bSuccess
			&& AreBranchesEqual(Left.ActualBranch, Right.ActualBranch)
			&& Left.ErrorCode == Right.ErrorCode
			&& Left.ErrorMessage == Right.ErrorMessage
			&& Left.InvalidField == Right.InvalidField
			&& AreMappingInputsEqual(Left.Input, Right.Input);
	}

	inline bool AreGlobalResultsEqual(
		const FMatchPlayCurrentAttackResolveInitialRouteGlobalContextResult&
			Left,
		const FMatchPlayCurrentAttackResolveInitialRouteGlobalContextResult&
			Right)
	{
		return Left.bSuccess == Right.bSuccess
			&& Left.bIsCanonicalDuplicate
				== Right.bIsCanonicalDuplicate
			&& Left.Request.AttackSequence
				== Right.Request.AttackSequence
			&& Left.ActionType == Right.ActionType
			&& Left.Intent == Right.Intent
			&& Left.bRequiresInitialRouteD6
				== Right.bRequiresInitialRouteD6
			&& Left.ExpectedRollPurpose
				== Right.ExpectedRollPurpose
			&& AreBundlesEqual(
				Left.ImmutableBundle,
				Right.ImmutableBundle)
			&& AreBranchesEqual(
				Left.ExistingActualBranch,
				Right.ExistingActualBranch)
			&& AreRollRecordsEqual(
				Left.ExistingInitialRouteRollRecords,
				Right.ExistingInitialRouteRollRecords)
			&& FMatchPlayCurrentAttackResolutionSessionStateValidationResult
				::StaticStruct()->CompareScriptStruct(
					&Left.SessionValidationResult,
					&Right.SessionValidationResult,
					0)
			&& Left.ErrorCode == Right.ErrorCode
			&& Left.ErrorMessage == Right.ErrorMessage;
	}

	inline bool AreLegalityResultsEqual(
		const FMatchPlayCurrentAttackResolveInitialRouteLegalityResult& Left,
		const FMatchPlayCurrentAttackResolveInitialRouteLegalityResult& Right)
	{
		return Left.bIsLegal == Right.bIsLegal
			&& Left.Request.AttackSequence
				== Right.Request.AttackSequence
			&& AreGlobalResultsEqual(
				Left.GlobalContextResult,
				Right.GlobalContextResult)
			&& Left.ErrorCode == Right.ErrorCode
			&& Left.ErrorMessage == Right.ErrorMessage;
	}

	inline bool AreWriterResultsEqual(
		const FMatchPlayCurrentAttackResolveInitialRouteWriterResult& Left,
		const FMatchPlayCurrentAttackResolveInitialRouteWriterResult& Right)
	{
		return Left.bSuccess == Right.bSuccess
			&& Left.bResolvedNewRoute == Right.bResolvedNewRoute
			&& Left.Request.AttackSequence
				== Right.Request.AttackSequence
			&& AreStatesEqual(Left.BeforeState, Right.BeforeState)
			&& AreStatesEqual(Left.AfterState, Right.AfterState)
			&& AreBranchesEqual(Left.ActualBranch, Right.ActualBranch)
			&& AreRollRecordsEqual(
				Left.InitialRouteRollRecords,
				Right.InitialRouteRollRecords)
			&& Left.ErrorCode == Right.ErrorCode
			&& Left.ErrorMessage == Right.ErrorMessage
			&& Left.FailureDisposition == Right.FailureDisposition
			&& AreGlobalResultsEqual(
				Left.GlobalContextResult,
				Right.GlobalContextResult)
			&& AreLegalityResultsEqual(
				Left.LegalityResult,
				Right.LegalityResult)
			&& Left.bProviderCalled == Right.bProviderCalled
			&& AreProviderResultsEqual(
				Left.ProviderResult,
				Right.ProviderResult)
			&& AreMappingResultsEqual(
				Left.MappingResult,
				Right.MappingResult)
			&& FMatchPlayCurrentAttackResolutionSessionStateValidationResult
				::StaticStruct()->CompareScriptStruct(
					&Left.CandidateValidationResult,
					&Right.CandidateValidationResult,
					0);
	}

	inline bool OnlyRouteFieldsChanged(
		const FMatchPlayState& Before,
		const FMatchPlayState& After)
	{
		FMatchPlayState NormalizedAfter = After;
		const FMatchPlayCurrentAttackResolutionSession& BeforeSession =
			Before.CurrentAttack.ResolutionSession;
		FMatchPlayCurrentAttackResolutionSession& AfterSession =
			NormalizedAfter.CurrentAttack.ResolutionSession;
		AfterSession.Stage = BeforeSession.Stage;
		AfterSession.bHasActualBranch = BeforeSession.bHasActualBranch;
		AfterSession.ActualBranch = BeforeSession.ActualBranch;
		AfterSession.InitialRouteRollRecords =
			BeforeSession.InitialRouteRollRecords;
		return AreStatesEqual(Before, NormalizedAfter);
	}
}

#endif
