#include "MatchPlayAuthoritativeSession.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

#include <type_traits>

namespace MatchPlayAuthoritativeSessionTests
{
	FPlayerCardData MakeDeckCard(
		const FString& CardId,
		const ECardRarity Rarity,
		const bool bIsGoalkeeper)
	{
		FPlayerCardData Card;
		Card.CardId = FName(*CardId);
		Card.Rarity = Rarity;
		Card.bIsGoalkeeper = bIsGoalkeeper;
		Card.PositionTypes = {
			bIsGoalkeeper
				? EPlayerPositionType::Goalkeeper
				: EPlayerPositionType::Attack
		};
		return Card;
	}

	TArray<FPlayerCardData> MakeValidDeck(
		const FString& Prefix,
		const ECardRarity Rarity)
	{
		TArray<FPlayerCardData> Deck;
		Deck.Reserve(20);
		for (int32 Index = 0; Index < 19; ++Index)
		{
			Deck.Add(MakeDeckCard(
				FString::Printf(
					TEXT("%s_OUT_%02d"),
					*Prefix,
					Index),
				Rarity,
				false));
		}
		Deck.Add(MakeDeckCard(
			FString::Printf(TEXT("%s_GK"), *Prefix),
			Rarity,
			true));
		return Deck;
	}

	FMatchPlayDeploymentSlotCatalog MakeDeploymentSlotCatalog(
		const TCHAR* Prefix)
	{
		FMatchPlayDeploymentSlotDefinition PlayerASlot;
		PlayerASlot.SlotId = FName(*FString::Printf(
			TEXT("%s_SlotA"),
			Prefix));
		PlayerASlot.NeutralSide =
			EMatchPlayNeutralSlotSide::NearPlayerA;

		FMatchPlayDeploymentSlotDefinition PlayerBSlot;
		PlayerBSlot.SlotId = FName(*FString::Printf(
			TEXT("%s_SlotB"),
			Prefix));
		PlayerBSlot.NeutralSide =
			EMatchPlayNeutralSlotSide::NearPlayerB;

		FMatchPlayDeploymentSlotCatalog Catalog;
		Catalog.Slots = { PlayerASlot, PlayerBSlot };
		return Catalog;
	}

	FMatchPlayOpeningInitializeInput MakeValidInput(
		const FString& Prefix = TEXT("Session"),
		const ECardRarity PlayerARarity = ECardRarity::Common,
		const ECardRarity PlayerBRarity = ECardRarity::Common)
	{
		FMatchPlayOpeningInitializeInput Input;
		Input.OpeningInput.PlayerADeck = MakeValidDeck(
			Prefix + TEXT("_A"),
			PlayerARarity);
		Input.OpeningInput.PlayerBDeck = MakeValidDeck(
			Prefix + TEXT("_B"),
			PlayerBRarity);
		Input.OpeningInput.PlayerAAttackCountD6Roll = 1;
		Input.OpeningInput.PlayerBAttackCountD6Roll = 1;
		Input.OpeningInput.PlayerATieBreakerRoll = 2;
		Input.OpeningInput.PlayerBTieBreakerRoll = 6;
		Input.DeploymentSlotCatalog =
			MakeDeploymentSlotCatalog(*Prefix);
		return Input;
	}

	FMatchPlayOpeningInitializeInput MakeInvalidInput()
	{
		FMatchPlayOpeningInitializeInput Input =
			MakeValidInput(TEXT("Invalid"));
		Input.OpeningInput.PlayerAAttackCountD6Roll = 0;
		return Input;
	}

	bool AreStatesEqual(
		const FMatchPlayState& Left,
		const FMatchPlayState& Right)
	{
		return FMatchPlayState::StaticStruct()->CompareScriptStruct(
			&Left,
			&Right,
			0);
	}

	bool AreSelectionValidationResultsEqual(
		const FMatchPlayCurrentAttackSelectionStateValidationResult& Left,
		const FMatchPlayCurrentAttackSelectionStateValidationResult& Right)
	{
		return Left.bIsCanonical == Right.bIsCanonical
			&& Left.ErrorCode == Right.ErrorCode
			&& Left.ErrorMessage == Right.ErrorMessage;
	}

	bool AreEnvelopesEqual(
		const FMatchPlayAuthoritativeRuntimeEnvelope& Left,
		const FMatchPlayAuthoritativeRuntimeEnvelope& Right)
	{
		return Left.bAccepted == Right.bAccepted
			&& Left.bDomainSuccess == Right.bDomainSuccess
			&& Left.bStateAdvanced == Right.bStateAdvanced
			&& Left.StateDisposition == Right.StateDisposition
			&& Left.bRuntimeFault == Right.bRuntimeFault
			&& AreStatesEqual(Left.BeforeState, Right.BeforeState)
			&& AreStatesEqual(Left.AfterState, Right.AfterState)
			&& Left.CommandKind == Right.CommandKind
			&& Left.AttackSequence == Right.AttackSequence
			&& Left.FailureDisposition == Right.FailureDisposition
			&& Left.RuntimeFailureCode == Right.RuntimeFailureCode
			&& Left.ErrorMessage == Right.ErrorMessage;
	}

	bool AreOpeningResultsEqual(
		const FMatchPlayOpeningInitializeResult& Left,
		const FMatchPlayOpeningInitializeResult& Right)
	{
		return Left.bSuccess == Right.bSuccess
			&& AreStatesEqual(Left.MatchPlayState, Right.MatchPlayState)
			&& Left.ErrorCode == Right.ErrorCode
			&& Left.UnderlyingOpeningErrorCodes
				== Right.UnderlyingOpeningErrorCodes
			&& Left.UnderlyingRuntimeInitializeErrorCodes
				== Right.UnderlyingRuntimeInitializeErrorCodes
			&& Left.UnderlyingPlayStateInitializeErrorCode
				== Right.UnderlyingPlayStateInitializeErrorCode
			&& Left.UnderlyingCardUsageErrorCode
				== Right.UnderlyingCardUsageErrorCode
			&& Left.UnderlyingDeploymentSlotCatalogValidationErrorCode
				== Right.UnderlyingDeploymentSlotCatalogValidationErrorCode
			&& Left.UnderlyingCardSnapshotAuthorityBuildErrorCode
				== Right.UnderlyingCardSnapshotAuthorityBuildErrorCode
			&& Left.UnderlyingCardSnapshotAuthorityFailingPlayerSide
				== Right.UnderlyingCardSnapshotAuthorityFailingPlayerSide
			&& Left.UnderlyingDeckValidationErrorCode
				== Right.UnderlyingDeckValidationErrorCode
			&& Left.UnderlyingPlayerCardRuleSnapshotValidationErrorCode
				== Right.UnderlyingPlayerCardRuleSnapshotValidationErrorCode
			&& Left.bRequiresTieBreakerReroll
				== Right.bRequiresTieBreakerReroll
			&& Left.ErrorMessage == Right.ErrorMessage;
	}

	bool AreBeginResultsEqual(
		const FMatchPlayBeginOrdinaryAttackResult& Left,
		const FMatchPlayBeginOrdinaryAttackResult& Right)
	{
		return Left.bSuccess == Right.bSuccess
			&& AreStatesEqual(Left.BeforeState, Right.BeforeState)
			&& AreStatesEqual(Left.AfterState, Right.AfterState)
			&& Left.ActionPoint == Right.ActionPoint
			&& Left.ErrorCode == Right.ErrorCode
			&& Left.ErrorMessage == Right.ErrorMessage;
	}

	bool AreFinishResultsEqual(
		const FMatchPlayFinishDeploymentResult& Left,
		const FMatchPlayFinishDeploymentResult& Right)
	{
		return Left.bSuccess == Right.bSuccess
			&& AreStatesEqual(Left.BeforeState, Right.BeforeState)
			&& AreStatesEqual(Left.AfterState, Right.AfterState)
			&& Left.AttackSequence == Right.AttackSequence
			&& Left.RequestingSide == Right.RequestingSide
			&& Left.ErrorCode == Right.ErrorCode
			&& AreSelectionValidationResultsEqual(
				Left.SelectionStateValidationResult,
				Right.SelectionStateValidationResult)
			&& Left.ErrorMessage == Right.ErrorMessage;
	}

	bool AreInitializeResultsEqual(
		const FMatchPlayAuthoritativeInitializeMatchResult& Left,
		const FMatchPlayAuthoritativeInitializeMatchResult& Right)
	{
		return AreEnvelopesEqual(
				Left.RuntimeEnvelope,
				Right.RuntimeEnvelope)
			&& AreOpeningResultsEqual(
				Left.OpeningResult,
				Right.OpeningResult);
	}

	bool AreAuthoritativeBeginResultsEqual(
		const FMatchPlayAuthoritativeBeginOrdinaryAttackResult& Left,
		const FMatchPlayAuthoritativeBeginOrdinaryAttackResult& Right)
	{
		return AreEnvelopesEqual(
				Left.RuntimeEnvelope,
				Right.RuntimeEnvelope)
			&& AreBeginResultsEqual(Left.BeginResult, Right.BeginResult);
	}

	bool AreAuthoritativeFinishResultsEqual(
		const FMatchPlayAuthoritativeFinishDeploymentResult& Left,
		const FMatchPlayAuthoritativeFinishDeploymentResult& Right)
	{
		return AreEnvelopesEqual(
				Left.RuntimeEnvelope,
				Right.RuntimeEnvelope)
			&& AreFinishResultsEqual(
				Left.FinishResult,
				Right.FinishResult);
	}

	EInitialTurnOrderPlayer OtherPlayer(
		const EInitialTurnOrderPlayer Player)
	{
		return Player == EInitialTurnOrderPlayer::PlayerA
			? EInitialTurnOrderPlayer::PlayerB
			: EInitialTurnOrderPlayer::PlayerA;
	}

	int32 CountOccurrences(
		const FString& Source,
		const FString& Needle)
	{
		int32 Count = 0;
		int32 SearchFrom = 0;
		while (true)
		{
			const int32 FoundAt = Source.Find(
				Needle,
				ESearchCase::CaseSensitive,
				ESearchDir::FromStart,
				SearchFrom);
			if (FoundAt == INDEX_NONE)
			{
				return Count;
			}
			++Count;
			SearchFrom = FoundAt + Needle.Len();
		}
	}

	bool LoadProductionSource(
		const TCHAR* RelativePath,
		FString& OutSource)
	{
		return FFileHelper::LoadFileToString(
			OutSource,
			*FPaths::Combine(FPaths::ProjectDir(), RelativePath));
	}

	void TestEnvelopeMutationCoverage(
		FAutomationTestBase& Test,
		const FMatchPlayAuthoritativeRuntimeEnvelope& Baseline)
	{
		auto RejectEqual = [&Test, &Baseline](
			const TCHAR* Field,
			const FMatchPlayAuthoritativeRuntimeEnvelope& Mutated)
		{
			Test.TestFalse(Field, AreEnvelopesEqual(Baseline, Mutated));
		};

		FMatchPlayAuthoritativeRuntimeEnvelope Mutated = Baseline;
		Mutated.bAccepted = !Mutated.bAccepted;
		RejectEqual(TEXT("Envelope comparator covers bAccepted"), Mutated);
		Mutated = Baseline;
		Mutated.bDomainSuccess = !Mutated.bDomainSuccess;
		RejectEqual(TEXT("Envelope comparator covers bDomainSuccess"), Mutated);
		Mutated = Baseline;
		Mutated.bStateAdvanced = !Mutated.bStateAdvanced;
		RejectEqual(TEXT("Envelope comparator covers bStateAdvanced"), Mutated);
		Mutated = Baseline;
		Mutated.StateDisposition =
			EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
		RejectEqual(TEXT("Envelope comparator covers StateDisposition"), Mutated);
		Mutated = Baseline;
		Mutated.bRuntimeFault = !Mutated.bRuntimeFault;
		RejectEqual(TEXT("Envelope comparator covers bRuntimeFault"), Mutated);
		Mutated = Baseline;
		Mutated.BeforeState.RuntimeState.bIsInitialized =
			!Mutated.BeforeState.RuntimeState.bIsInitialized;
		RejectEqual(TEXT("Envelope comparator covers BeforeState"), Mutated);
		Mutated = Baseline;
		Mutated.AfterState.bHasCurrentAttack =
			!Mutated.AfterState.bHasCurrentAttack;
		RejectEqual(TEXT("Envelope comparator covers AfterState"), Mutated);
		Mutated = Baseline;
		Mutated.CommandKind = EMatchPlayAuthoritativeCommandKind::None;
		RejectEqual(TEXT("Envelope comparator covers CommandKind"), Mutated);
		Mutated = Baseline;
		++Mutated.AttackSequence;
		RejectEqual(TEXT("Envelope comparator covers AttackSequence"), Mutated);
		Mutated = Baseline;
		Mutated.FailureDisposition =
			EMatchPlayAuthoritativeFailureDisposition
				::RetryableExecutionFailure;
		RejectEqual(TEXT("Envelope comparator covers FailureDisposition"), Mutated);
		Mutated = Baseline;
		Mutated.RuntimeFailureCode =
			EMatchPlayAuthoritativeRuntimeFailureCode::NotInitialized;
		RejectEqual(TEXT("Envelope comparator covers RuntimeFailureCode"), Mutated);
		Mutated = Baseline;
		Mutated.ErrorMessage = TEXT("mutated runtime message");
		RejectEqual(TEXT("Envelope comparator covers ErrorMessage"), Mutated);
	}

	void TestOpeningMutationCoverage(
		FAutomationTestBase& Test,
		const FMatchPlayAuthoritativeInitializeMatchResult& Baseline)
	{
		auto RejectEqual = [&Test, &Baseline](
			const TCHAR* Field,
			const FMatchPlayAuthoritativeInitializeMatchResult& Mutated)
		{
			Test.TestFalse(
				Field,
				AreInitializeResultsEqual(Baseline, Mutated));
		};

		FMatchPlayAuthoritativeInitializeMatchResult Mutated = Baseline;
		Mutated.RuntimeEnvelope.bRuntimeFault = true;
		RejectEqual(TEXT("Initialize comparator covers envelope"), Mutated);
		Mutated = Baseline;
		Mutated.OpeningResult.bSuccess = !Mutated.OpeningResult.bSuccess;
		RejectEqual(TEXT("Opening comparator covers bSuccess"), Mutated);
		Mutated = Baseline;
		Mutated.OpeningResult.MatchPlayState.bHasCurrentAttack = true;
		RejectEqual(TEXT("Opening comparator covers MatchPlayState"), Mutated);
		Mutated = Baseline;
		Mutated.OpeningResult.ErrorCode =
			EMatchPlayOpeningInitializeErrorCode::OpeningResolveFailed;
		RejectEqual(TEXT("Opening comparator covers ErrorCode"), Mutated);
		Mutated = Baseline;
		Mutated.OpeningResult.UnderlyingOpeningErrorCodes.Add(
			static_cast<EMatchOpeningResolveErrorCode>(1));
		RejectEqual(TEXT("Opening comparator covers opening errors"), Mutated);
		Mutated = Baseline;
		Mutated.OpeningResult.UnderlyingRuntimeInitializeErrorCodes.Add(
			static_cast<EMatchRuntimeInitializeErrorCode>(1));
		RejectEqual(TEXT("Opening comparator covers runtime errors"), Mutated);
		Mutated = Baseline;
		Mutated.OpeningResult.UnderlyingPlayStateInitializeErrorCode =
			static_cast<EMatchPlayStateInitializeErrorCode>(1);
		RejectEqual(TEXT("Opening comparator covers play-state error"), Mutated);
		Mutated = Baseline;
		Mutated.OpeningResult.UnderlyingCardUsageErrorCode =
			static_cast<EMatchCardUsageInitializeErrorCode>(1);
		RejectEqual(TEXT("Opening comparator covers card-usage error"), Mutated);
		Mutated = Baseline;
		Mutated.OpeningResult
			.UnderlyingDeploymentSlotCatalogValidationErrorCode =
				static_cast<
					EMatchPlayDeploymentSlotCatalogValidationErrorCode>(1);
		RejectEqual(TEXT("Opening comparator covers catalog error"), Mutated);
		Mutated = Baseline;
		Mutated.OpeningResult
			.UnderlyingCardSnapshotAuthorityBuildErrorCode =
				static_cast<EMatchPlayCardSnapshotAuthorityBuildErrorCode>(1);
		RejectEqual(TEXT("Opening comparator covers authority error"), Mutated);
		Mutated = Baseline;
		Mutated.OpeningResult
			.UnderlyingCardSnapshotAuthorityFailingPlayerSide =
				EInitialTurnOrderPlayer::PlayerA;
		RejectEqual(TEXT("Opening comparator covers failing side"), Mutated);
		Mutated = Baseline;
		Mutated.OpeningResult.UnderlyingDeckValidationErrorCode =
			static_cast<EDeckValidationErrorCode>(1);
		RejectEqual(TEXT("Opening comparator covers deck error"), Mutated);
		Mutated = Baseline;
		Mutated.OpeningResult
			.UnderlyingPlayerCardRuleSnapshotValidationErrorCode =
				static_cast<
					EPlayerCardRuleSnapshotValidationErrorCode>(1);
		RejectEqual(TEXT("Opening comparator covers snapshot error"), Mutated);
		Mutated = Baseline;
		Mutated.OpeningResult.bRequiresTieBreakerReroll =
			!Mutated.OpeningResult.bRequiresTieBreakerReroll;
		RejectEqual(TEXT("Opening comparator covers reroll flag"), Mutated);
		Mutated = Baseline;
		Mutated.OpeningResult.ErrorMessage = TEXT("mutated opening message");
		RejectEqual(TEXT("Opening comparator covers message"), Mutated);
	}

	void TestBeginMutationCoverage(
		FAutomationTestBase& Test,
		const FMatchPlayAuthoritativeBeginOrdinaryAttackResult& Baseline)
	{
		auto RejectEqual = [&Test, &Baseline](
			const TCHAR* Field,
			const FMatchPlayAuthoritativeBeginOrdinaryAttackResult& Mutated)
		{
			Test.TestFalse(
				Field,
				AreAuthoritativeBeginResultsEqual(Baseline, Mutated));
		};

		FMatchPlayAuthoritativeBeginOrdinaryAttackResult Mutated = Baseline;
		Mutated.RuntimeEnvelope.bRuntimeFault = true;
		RejectEqual(TEXT("Begin comparator covers envelope"), Mutated);
		Mutated = Baseline;
		Mutated.BeginResult.bSuccess = !Mutated.BeginResult.bSuccess;
		RejectEqual(TEXT("Begin comparator covers bSuccess"), Mutated);
		Mutated = Baseline;
		Mutated.BeginResult.BeforeState.bHasCurrentAttack = true;
		RejectEqual(TEXT("Begin comparator covers BeforeState"), Mutated);
		Mutated = Baseline;
		Mutated.BeginResult.AfterState.bHasCurrentAttack = false;
		RejectEqual(TEXT("Begin comparator covers AfterState"), Mutated);
		Mutated = Baseline;
		++Mutated.BeginResult.ActionPoint;
		RejectEqual(TEXT("Begin comparator covers ActionPoint"), Mutated);
		Mutated = Baseline;
		Mutated.BeginResult.ErrorCode =
			EMatchPlayBeginOrdinaryAttackErrorCode::InvalidActionPoint;
		RejectEqual(TEXT("Begin comparator covers ErrorCode"), Mutated);
		Mutated = Baseline;
		Mutated.BeginResult.ErrorMessage = TEXT("mutated begin message");
		RejectEqual(TEXT("Begin comparator covers message"), Mutated);
	}

	void TestFinishMutationCoverage(
		FAutomationTestBase& Test,
		const FMatchPlayAuthoritativeFinishDeploymentResult& Baseline)
	{
		auto RejectEqual = [&Test, &Baseline](
			const TCHAR* Field,
			const FMatchPlayAuthoritativeFinishDeploymentResult& Mutated)
		{
			Test.TestFalse(
				Field,
				AreAuthoritativeFinishResultsEqual(Baseline, Mutated));
		};

		FMatchPlayAuthoritativeFinishDeploymentResult Mutated = Baseline;
		Mutated.RuntimeEnvelope.bRuntimeFault = true;
		RejectEqual(TEXT("Finish comparator covers envelope"), Mutated);
		Mutated = Baseline;
		Mutated.FinishResult.bSuccess = !Mutated.FinishResult.bSuccess;
		RejectEqual(TEXT("Finish comparator covers bSuccess"), Mutated);
		Mutated = Baseline;
		Mutated.FinishResult.BeforeState.bHasCurrentAttack = false;
		RejectEqual(TEXT("Finish comparator covers BeforeState"), Mutated);
		Mutated = Baseline;
		Mutated.FinishResult.AfterState.bHasCurrentAttack = false;
		RejectEqual(TEXT("Finish comparator covers AfterState"), Mutated);
		Mutated = Baseline;
		++Mutated.FinishResult.AttackSequence;
		RejectEqual(TEXT("Finish comparator covers AttackSequence"), Mutated);
		Mutated = Baseline;
		Mutated.FinishResult.RequestingSide =
			OtherPlayer(Mutated.FinishResult.RequestingSide);
		RejectEqual(TEXT("Finish comparator covers RequestingSide"), Mutated);
		Mutated = Baseline;
		Mutated.FinishResult.ErrorCode =
			EMatchPlayFinishDeploymentErrorCode::NoCurrentAttack;
		RejectEqual(TEXT("Finish comparator covers ErrorCode"), Mutated);
		Mutated = Baseline;
		Mutated.FinishResult.SelectionStateValidationResult.bIsCanonical =
			!Mutated.FinishResult.SelectionStateValidationResult.bIsCanonical;
		RejectEqual(TEXT("Finish comparator covers selection canonical flag"), Mutated);
		Mutated = Baseline;
		Mutated.FinishResult.SelectionStateValidationResult.ErrorCode =
			EMatchPlayCurrentAttackSelectionStateValidationErrorCode
				::UnsupportedCurrentAttackPhase;
		RejectEqual(TEXT("Finish comparator covers selection error code"), Mutated);
		Mutated = Baseline;
		Mutated.FinishResult.SelectionStateValidationResult.ErrorMessage =
			TEXT("mutated selection message");
		RejectEqual(TEXT("Finish comparator covers selection message"), Mutated);
		Mutated = Baseline;
		Mutated.FinishResult.ErrorMessage = TEXT("mutated finish message");
		RejectEqual(TEXT("Finish comparator covers message"), Mutated);
	}

	bool TestAdoptedEnvelope(
		FAutomationTestBase& Test,
		const TCHAR* Context,
		const FMatchPlayAuthoritativeRuntimeEnvelope& Envelope,
		const FMatchPlayState& BeforeState,
		const FMatchPlayState& AfterState)
	{
		Test.TestTrue(*FString::Printf(TEXT("%s accepted"), Context),
			Envelope.bAccepted);
		Test.TestTrue(*FString::Printf(TEXT("%s domain succeeds"), Context),
			Envelope.bDomainSuccess);
		Test.TestEqual(
			*FString::Printf(TEXT("%s adopts"), Context),
			Envelope.StateDisposition,
			EMatchPlayAuthoritativeStateDisposition::Adopt);
		Test.TestTrue(*FString::Printf(TEXT("%s advances"), Context),
			Envelope.bStateAdvanced);
		Test.TestFalse(*FString::Printf(TEXT("%s has no runtime fault"), Context),
			Envelope.bRuntimeFault);
		Test.TestEqual(
			*FString::Printf(TEXT("%s has no runtime failure"), Context),
			Envelope.RuntimeFailureCode,
			EMatchPlayAuthoritativeRuntimeFailureCode::None);
		Test.TestEqual(
			*FString::Printf(TEXT("%s has no failure disposition"), Context),
			Envelope.FailureDisposition,
			EMatchPlayAuthoritativeFailureDisposition::None);
		Test.TestTrue(*FString::Printf(TEXT("%s before matches"), Context),
			AreStatesEqual(Envelope.BeforeState, BeforeState));
		Test.TestTrue(*FString::Printf(TEXT("%s after matches"), Context),
			AreStatesEqual(Envelope.AfterState, AfterState));
		Test.TestTrue(*FString::Printf(TEXT("%s runtime message is empty"), Context),
			Envelope.ErrorMessage.IsEmpty());
		return true;
	}

	bool TestNoAdoptDomainFailure(
		FAutomationTestBase& Test,
		const TCHAR* Context,
		const FMatchPlayAuthoritativeRuntimeEnvelope& Envelope,
		const FMatchPlayState& State)
	{
		Test.TestTrue(*FString::Printf(TEXT("%s accepted"), Context),
			Envelope.bAccepted);
		Test.TestFalse(*FString::Printf(TEXT("%s domain fails"), Context),
			Envelope.bDomainSuccess);
		Test.TestEqual(
			*FString::Printf(TEXT("%s does not adopt"), Context),
			Envelope.StateDisposition,
			EMatchPlayAuthoritativeStateDisposition::DoNotAdopt);
		Test.TestFalse(*FString::Printf(TEXT("%s does not advance"), Context),
			Envelope.bStateAdvanced);
		Test.TestEqual(
			*FString::Printf(TEXT("%s is not a runtime rejection"), Context),
			Envelope.RuntimeFailureCode,
			EMatchPlayAuthoritativeRuntimeFailureCode::None);
		Test.TestTrue(*FString::Printf(TEXT("%s before is authoritative"), Context),
			AreStatesEqual(Envelope.BeforeState, State));
		Test.TestTrue(*FString::Printf(TEXT("%s after is unchanged"), Context),
			AreStatesEqual(Envelope.AfterState, State));
		Test.TestTrue(*FString::Printf(TEXT("%s runtime message is empty"), Context),
			Envelope.ErrorMessage.IsEmpty());
		return true;
	}
}

#define MATCH_PLAY_AUTHORITATIVE_SESSION_TEST(TestClass, TestName) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
		TestClass, \
		"FMCodex.CoreRules.MatchPlayAuthoritativeSession." TestName, \
		EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

MATCH_PLAY_AUTHORITATIVE_SESSION_TEST(
	FMatchPlayAuthoritativeSessionTypesAndSurfaceTest,
	"01.TypesAndProductionSurface")

bool FMatchPlayAuthoritativeSessionTypesAndSurfaceTest::RunTest(
	const FString& Parameters)
{
	static_assert(!std::is_copy_constructible_v<
		FMatchPlayAuthoritativeSession>);
	static_assert(!std::is_copy_assignable_v<
		FMatchPlayAuthoritativeSession>);
	static_assert(!std::is_move_constructible_v<
		FMatchPlayAuthoritativeSession>);
	static_assert(!std::is_move_assignable_v<
		FMatchPlayAuthoritativeSession>);
	static_assert(static_cast<uint8>(
		EMatchPlayAuthoritativeStateDisposition::DoNotAdopt) == 0);
	static_assert(static_cast<uint8>(
		EMatchPlayAuthoritativeStateDisposition::Adopt) == 1);
	static_assert(static_cast<uint8>(
		EMatchPlayAuthoritativeCommandKind::FinishDeployment) == 3);
	static_assert(static_cast<uint8>(
		EMatchPlayAuthoritativeRuntimeFailureCode::ReentrantCommand) == 3);
	static_assert(static_cast<uint8>(
		EMatchPlayAuthoritativeFailureDisposition
			::NonRetryableInvariantFailure) == 3);

	FString Header;
	FString Implementation;
	FString Types;
	TestTrue(TEXT("Session header loads"),
		MatchPlayAuthoritativeSessionTests::LoadProductionSource(
			TEXT("Source/FMCodex/MatchPlayRuntime/MatchPlayAuthoritativeSession.h"),
			Header));
	TestTrue(TEXT("Session implementation loads"),
		MatchPlayAuthoritativeSessionTests::LoadProductionSource(
			TEXT("Source/FMCodex/MatchPlayRuntime/MatchPlayAuthoritativeSession.cpp"),
			Implementation));
	TestTrue(TEXT("Session types load"),
		MatchPlayAuthoritativeSessionTests::LoadProductionSource(
			TEXT("Source/FMCodex/MatchPlayRuntime/MatchPlayAuthoritativeSessionTypes.h"),
			Types));

	const FString Production = Header + Implementation + Types;
	const int32 PrivateAt = Header.Find(TEXT("private:"));
	const FString PublicSurface = PrivateAt == INDEX_NONE
		? Header
		: Header.Left(PrivateAt);
	TestEqual(TEXT("Authoritative Session definitions"),
		MatchPlayAuthoritativeSessionTests::CountOccurrences(
			Header,
			TEXT("class FMCODEX_API FMatchPlayAuthoritativeSession final")),
		1);
	TestEqual(TEXT("Private FMatchPlayState owner fields"),
		MatchPlayAuthoritativeSessionTests::CountOccurrences(
			Header,
			TEXT("FMatchPlayState AuthoritativeState;")),
		1);
	TestEqual(TEXT("Serialized gate declarations"),
		MatchPlayAuthoritativeSessionTests::CountOccurrences(
			Header,
			TEXT("ExecuteSerialized(")),
		1);
	TestEqual(TEXT("All three mutations use the gate"),
		MatchPlayAuthoritativeSessionTests::CountOccurrences(
			Implementation,
			TEXT("ExecuteSerialized<")),
		3);
	TestEqual(TEXT("Instance execution guard fields"),
		MatchPlayAuthoritativeSessionTests::CountOccurrences(
			Header,
			TEXT("bool bExecutingCommand = false;")),
		1);
	TestEqual(TEXT("Single authoritative replacement site"),
		MatchPlayAuthoritativeSessionTests::CountOccurrences(
			Implementation,
			TEXT("AuthoritativeState = Adoption.AdoptedAfterState;")),
		1);
	TestFalse(TEXT("Public mutable State references absent"),
		PublicSurface.Contains(TEXT("FMatchPlayState&")));
	TestFalse(TEXT("Public State pointers absent"),
		PublicSurface.Contains(TEXT("FMatchPlayState*")));
	TestFalse(TEXT("SetState absent"), Production.Contains(TEXT("SetState")));
	TestFalse(TEXT("RestoreState absent"), Production.Contains(TEXT("RestoreState")));
	TestFalse(TEXT("Public generic Execute absent"),
		PublicSurface.Contains(TEXT("Execute")));
	TestFalse(TEXT("Public Writer access absent"),
		PublicSurface.Contains(TEXT("Writer")));
	TestFalse(TEXT("UObject absent"), Production.Contains(TEXT("UObject")));
	TestFalse(TEXT("UInterface absent"), Production.Contains(TEXT("UInterface")));
	TestFalse(TEXT("USTRUCT absent"), Production.Contains(TEXT("USTRUCT")));
	TestFalse(TEXT("UENUM absent"), Production.Contains(TEXT("UENUM")));
	TestFalse(TEXT("UFUNCTION absent"), Production.Contains(TEXT("UFUNCTION")));
	TestFalse(TEXT("RPC absent"), Production.Contains(TEXT("RPC")));
	TestFalse(TEXT("Tick absent"), Production.Contains(TEXT("Tick")));
	TestFalse(TEXT("Mutex absent"), Production.Contains(TEXT("Mutex")));
	TestFalse(TEXT("Async absent"), Production.Contains(TEXT("Async")));
	TestFalse(TEXT("Future absent"), Production.Contains(TEXT("Future")));
	TestFalse(TEXT("Provider absent"), Production.Contains(TEXT("Provider")));
	TestFalse(TEXT("Initial Route Orchestrator absent"),
		Production.Contains(TEXT("ResolveInitialRouteOrchestrator")));
	TestFalse(TEXT("Direct CurrentAttack authority write absent"),
		Implementation.Contains(TEXT("AuthoritativeState.CurrentAttack")));
	TestFalse(TEXT("Direct Phase authority write absent"),
		Implementation.Contains(TEXT("AuthoritativeState.Phase")));
	TestFalse(TEXT("Static mutable State absent"),
		Production.Contains(TEXT("static FMatchPlayState")));
	return true;
}

MATCH_PLAY_AUTHORITATIVE_SESSION_TEST(
	FMatchPlayAuthoritativeSessionDefaultAndSnapshotTest,
	"02.DefaultSessionAndSnapshotIsolation")

bool FMatchPlayAuthoritativeSessionDefaultAndSnapshotTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayAuthoritativeSession Session;
	const FMatchPlayState DefaultState;
	TestTrue(TEXT("Default session exposes complete default state"),
		MatchPlayAuthoritativeSessionTests::AreStatesEqual(
			Session.GetStateSnapshot(),
			DefaultState));

	FMatchPlayState Snapshot = Session.GetStateSnapshot();
	Snapshot.RuntimeState.bIsInitialized = true;
	Snapshot.bHasCurrentAttack = true;
	TestTrue(TEXT("Mutating a snapshot cannot mutate the session"),
		MatchPlayAuthoritativeSessionTests::AreStatesEqual(
			Session.GetStateSnapshot(),
			DefaultState));
	return true;
}

MATCH_PLAY_AUTHORITATIVE_SESSION_TEST(
	FMatchPlayAuthoritativeSessionInitializationSuccessTest,
	"03.InitializationSuccessDeterminism")

bool FMatchPlayAuthoritativeSessionInitializationSuccessTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayOpeningInitializeInput Input =
		MatchPlayAuthoritativeSessionTests::MakeValidInput();
	FMatchPlayAuthoritativeSession Sessions[3];
	FMatchPlayAuthoritativeInitializeMatchResult Results[3] = {
		Sessions[0].InitializeMatch(Input),
		Sessions[1].InitializeMatch(Input),
		Sessions[2].InitializeMatch(Input)
	};

	const FMatchPlayState DefaultState;
	for (int32 Index = 0; Index < 3; ++Index)
	{
		TestTrue(TEXT("Opening domain succeeds"), Results[Index].OpeningResult.bSuccess);
		TestEqual(TEXT("Initialize command kind"),
			Results[Index].RuntimeEnvelope.CommandKind,
			EMatchPlayAuthoritativeCommandKind::InitializeMatch);
		TestEqual(TEXT("Initialize attack sequence is zero"),
			Results[Index].RuntimeEnvelope.AttackSequence,
			int64{ 0 });
		MatchPlayAuthoritativeSessionTests::TestAdoptedEnvelope(
			*this,
			TEXT("Initialize"),
			Results[Index].RuntimeEnvelope,
			DefaultState,
			Results[Index].OpeningResult.MatchPlayState);
		TestTrue(TEXT("Session adopts exact opening state"),
			MatchPlayAuthoritativeSessionTests::AreStatesEqual(
				Sessions[Index].GetStateSnapshot(),
				Results[Index].OpeningResult.MatchPlayState));
	}
	TestTrue(TEXT("Initialize result deterministic 1/2"),
		MatchPlayAuthoritativeSessionTests::AreInitializeResultsEqual(
			Results[0], Results[1]));
	TestTrue(TEXT("Initialize result deterministic 1/3"),
		MatchPlayAuthoritativeSessionTests::AreInitializeResultsEqual(
			Results[0], Results[2]));
	return true;
}

MATCH_PLAY_AUTHORITATIVE_SESSION_TEST(
	FMatchPlayAuthoritativeSessionInitializationFailureTest,
	"04.InitializationFailureAndRepeatGate")

bool FMatchPlayAuthoritativeSessionInitializationFailureTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayAuthoritativeSession FailedSession;
	const FMatchPlayState DefaultState;
	const FMatchPlayAuthoritativeInitializeMatchResult Failed =
		FailedSession.InitializeMatch(
			MatchPlayAuthoritativeSessionTests::MakeInvalidInput());
	TestFalse(TEXT("Invalid external D6 reaches real domain failure"),
		Failed.OpeningResult.bSuccess);
	TestEqual(TEXT("Invalid D6 exact wrapper failure"),
		Failed.OpeningResult.ErrorCode,
		EMatchPlayOpeningInitializeErrorCode::OpeningResolveFailed);
	MatchPlayAuthoritativeSessionTests::TestNoAdoptDomainFailure(
		*this,
		TEXT("Initialize failure"),
		Failed.RuntimeEnvelope,
		DefaultState);
	TestTrue(TEXT("Failed initialization keeps default session"),
		MatchPlayAuthoritativeSessionTests::AreStatesEqual(
			FailedSession.GetStateSnapshot(),
			DefaultState));

	FMatchPlayAuthoritativeSession Session;
	Session.InitializeMatch(
		MatchPlayAuthoritativeSessionTests::MakeValidInput());
	const FMatchPlayState Initialized = Session.GetStateSnapshot();
	const FMatchPlayAuthoritativeInitializeMatchResult Repeated =
		Session.InitializeMatch(
			MatchPlayAuthoritativeSessionTests::MakeInvalidInput());
	TestFalse(TEXT("Repeated initialization is gate-rejected"),
		Repeated.RuntimeEnvelope.bAccepted);
	TestEqual(TEXT("Repeated initialization exact runtime code"),
		Repeated.RuntimeEnvelope.RuntimeFailureCode,
		EMatchPlayAuthoritativeRuntimeFailureCode::AlreadyInitialized);
	TestTrue(TEXT("Repeated initialization has deterministic message"),
		!Repeated.RuntimeEnvelope.ErrorMessage.IsEmpty());
	TestTrue(TEXT("Repeated initialization nested result is default"),
		MatchPlayAuthoritativeSessionTests::AreOpeningResultsEqual(
			Repeated.OpeningResult,
			FMatchPlayOpeningInitializeResult{}));
	TestTrue(TEXT("Repeated initialization preserves session"),
		MatchPlayAuthoritativeSessionTests::AreStatesEqual(
			Session.GetStateSnapshot(),
			Initialized));
	return true;
}

MATCH_PLAY_AUTHORITATIVE_SESSION_TEST(
	FMatchPlayAuthoritativeSessionUninitializedGateTest,
	"05.UninitializedCommandGate")

bool FMatchPlayAuthoritativeSessionUninitializedGateTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayAuthoritativeSession Session;
	const FMatchPlayState DefaultState;
	const FMatchPlayAuthoritativeBeginOrdinaryAttackResult Begin =
		Session.BeginOrdinaryAttack(6);
	const FMatchPlayAuthoritativeFinishDeploymentResult Finish =
		Session.FinishDeployment(17, EInitialTurnOrderPlayer::PlayerA);

	for (const FMatchPlayAuthoritativeRuntimeEnvelope* Envelope :
		{ &Begin.RuntimeEnvelope, &Finish.RuntimeEnvelope })
	{
		TestFalse(TEXT("Uninitialized command is not accepted"),
			Envelope->bAccepted);
		TestFalse(TEXT("Uninitialized command has no domain success"),
			Envelope->bDomainSuccess);
		TestFalse(TEXT("Uninitialized command does not advance"),
			Envelope->bStateAdvanced);
		TestEqual(TEXT("Uninitialized command does not adopt"),
			Envelope->StateDisposition,
			EMatchPlayAuthoritativeStateDisposition::DoNotAdopt);
		TestFalse(TEXT("Uninitialized rejection is not a runtime fault"),
			Envelope->bRuntimeFault);
		TestEqual(TEXT("Uninitialized command exact runtime code"),
			Envelope->RuntimeFailureCode,
			EMatchPlayAuthoritativeRuntimeFailureCode::NotInitialized);
		TestTrue(TEXT("Uninitialized command has deterministic message"),
			!Envelope->ErrorMessage.IsEmpty());
		TestTrue(TEXT("Uninitialized command before is default"),
			MatchPlayAuthoritativeSessionTests::AreStatesEqual(
				Envelope->BeforeState, DefaultState));
		TestTrue(TEXT("Uninitialized command after is default"),
			MatchPlayAuthoritativeSessionTests::AreStatesEqual(
				Envelope->AfterState, DefaultState));
	}
	TestTrue(TEXT("Uninitialized Begin nested result is default"),
		MatchPlayAuthoritativeSessionTests::AreBeginResultsEqual(
			Begin.BeginResult,
			FMatchPlayBeginOrdinaryAttackResult{}));
	TestTrue(TEXT("Uninitialized Finish nested result is default"),
		MatchPlayAuthoritativeSessionTests::AreFinishResultsEqual(
			Finish.FinishResult,
			FMatchPlayFinishDeploymentResult{}));
	TestEqual(TEXT("Uninitialized Begin has no attack sequence"),
		Begin.RuntimeEnvelope.AttackSequence,
		int64{ 0 });
	TestEqual(TEXT("Uninitialized Finish echoes trusted attack sequence"),
		Finish.RuntimeEnvelope.AttackSequence,
		int64{ 17 });
	TestTrue(TEXT("Uninitialized gates preserve session"),
		MatchPlayAuthoritativeSessionTests::AreStatesEqual(
			Session.GetStateSnapshot(),
			DefaultState));
	return true;
}

MATCH_PLAY_AUTHORITATIVE_SESSION_TEST(
	FMatchPlayAuthoritativeSessionBeginAdoptionTest,
	"06.BeginOrdinaryAttackAdoption")

bool FMatchPlayAuthoritativeSessionBeginAdoptionTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayAuthoritativeSession Session;
	Session.InitializeMatch(
		MatchPlayAuthoritativeSessionTests::MakeValidInput());
	const FMatchPlayState Before = Session.GetStateSnapshot();
	const FMatchPlayAuthoritativeBeginOrdinaryAttackResult Result =
		Session.BeginOrdinaryAttack(6);
	TestTrue(TEXT("Real Begin succeeds"), Result.BeginResult.bSuccess);
	TestEqual(TEXT("Begin command kind"),
		Result.RuntimeEnvelope.CommandKind,
		EMatchPlayAuthoritativeCommandKind::BeginOrdinaryAttack);
	TestEqual(TEXT("Begin sequence comes from nested adopted state"),
		Result.RuntimeEnvelope.AttackSequence,
		Result.BeginResult.AfterState.CurrentAttack.AttackSequence);
	MatchPlayAuthoritativeSessionTests::TestAdoptedEnvelope(
		*this,
		TEXT("Begin"),
		Result.RuntimeEnvelope,
		Before,
		Result.BeginResult.AfterState);
	TestTrue(TEXT("Begin reads exact session state"),
		MatchPlayAuthoritativeSessionTests::AreStatesEqual(
			Result.BeginResult.BeforeState,
			Before));
	TestTrue(TEXT("Session adopts exact Begin AfterState"),
		MatchPlayAuthoritativeSessionTests::AreStatesEqual(
			Session.GetStateSnapshot(),
			Result.BeginResult.AfterState));
	return true;
}

MATCH_PLAY_AUTHORITATIVE_SESSION_TEST(
	FMatchPlayAuthoritativeSessionFinishSequentialTest,
	"07.FinishDeploymentSequentialAdoption")

bool FMatchPlayAuthoritativeSessionFinishSequentialTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayAuthoritativeSession Session;
	Session.InitializeMatch(
		MatchPlayAuthoritativeSessionTests::MakeValidInput());
	const FMatchPlayAuthoritativeBeginOrdinaryAttackResult Begin =
		Session.BeginOrdinaryAttack(6);
	const FMatchPlayState BeforeFirstFinish = Session.GetStateSnapshot();
	const int64 Sequence =
		BeforeFirstFinish.CurrentAttack.AttackSequence;
	const EInitialTurnOrderPlayer FirstSide =
		BeforeFirstFinish.CurrentAttack.CurrentLegalDeploymentSide;
	const FMatchPlayAuthoritativeFinishDeploymentResult First =
		Session.FinishDeployment(Sequence, FirstSide);
	const FMatchPlayState BeforeSecondFinish = Session.GetStateSnapshot();
	const EInitialTurnOrderPlayer SecondSide =
		BeforeSecondFinish.CurrentAttack.CurrentLegalDeploymentSide;
	const FMatchPlayAuthoritativeFinishDeploymentResult Second =
		Session.FinishDeployment(Sequence, SecondSide);

	TestTrue(TEXT("Begin succeeded before finish sequence"),
		Begin.BeginResult.bSuccess);
	TestTrue(TEXT("First Finish succeeds"), First.FinishResult.bSuccess);
	TestTrue(TEXT("Second Finish succeeds"), Second.FinishResult.bSuccess);
	MatchPlayAuthoritativeSessionTests::TestAdoptedEnvelope(
		*this, TEXT("First Finish"), First.RuntimeEnvelope,
		BeforeFirstFinish, First.FinishResult.AfterState);
	MatchPlayAuthoritativeSessionTests::TestAdoptedEnvelope(
		*this, TEXT("Second Finish"), Second.RuntimeEnvelope,
		BeforeSecondFinish, Second.FinishResult.AfterState);
	TestTrue(TEXT("First Finish domain reads latest Begin state"),
		MatchPlayAuthoritativeSessionTests::AreStatesEqual(
			First.FinishResult.BeforeState,
			Begin.BeginResult.AfterState));
	TestTrue(TEXT("Second Finish domain reads first Finish state"),
		MatchPlayAuthoritativeSessionTests::AreStatesEqual(
			Second.FinishResult.BeforeState,
			First.FinishResult.AfterState));
	TestEqual(TEXT("Second finish enters Resolution"),
		Session.GetStateSnapshot().CurrentAttack.Phase,
		EMatchPlayCurrentAttackPhase::Resolution);
	return true;
}

MATCH_PLAY_AUTHORITATIVE_SESSION_TEST(
	FMatchPlayAuthoritativeSessionOrdinaryFailureTest,
	"08.OrdinaryFailureAndDuplicateNoAdopt")

bool FMatchPlayAuthoritativeSessionOrdinaryFailureTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayAuthoritativeSession InvalidBeginSession;
	InvalidBeginSession.InitializeMatch(
		MatchPlayAuthoritativeSessionTests::MakeValidInput(TEXT("InvalidBegin")));
	const FMatchPlayState BeforeInvalidBegin =
		InvalidBeginSession.GetStateSnapshot();
	const FMatchPlayAuthoritativeBeginOrdinaryAttackResult InvalidBegin =
		InvalidBeginSession.BeginOrdinaryAttack(1);
	TestEqual(TEXT("Invalid action point is real Begin domain failure"),
		InvalidBegin.BeginResult.ErrorCode,
		EMatchPlayBeginOrdinaryAttackErrorCode::InvalidActionPoint);
	MatchPlayAuthoritativeSessionTests::TestNoAdoptDomainFailure(
		*this, TEXT("Invalid Begin"), InvalidBegin.RuntimeEnvelope,
		BeforeInvalidBegin);
	TestTrue(TEXT("Invalid Begin preserves session"),
		MatchPlayAuthoritativeSessionTests::AreStatesEqual(
			InvalidBeginSession.GetStateSnapshot(),
			BeforeInvalidBegin));

	FMatchPlayAuthoritativeSession Session;
	Session.InitializeMatch(
		MatchPlayAuthoritativeSessionTests::MakeValidInput(TEXT("Failures")));
	const FMatchPlayAuthoritativeBeginOrdinaryAttackResult Begin =
		Session.BeginOrdinaryAttack(6);
	const FMatchPlayState ActiveAttack = Session.GetStateSnapshot();
	const FMatchPlayAuthoritativeBeginOrdinaryAttackResult Duplicate =
		Session.BeginOrdinaryAttack(6);
	TestEqual(TEXT("Duplicate Begin is actual active-attack domain failure"),
		Duplicate.BeginResult.ErrorCode,
		EMatchPlayBeginOrdinaryAttackErrorCode::CurrentAttackAlreadyActive);
	MatchPlayAuthoritativeSessionTests::TestNoAdoptDomainFailure(
		*this, TEXT("Duplicate Begin"), Duplicate.RuntimeEnvelope,
		ActiveAttack);

	const EInitialTurnOrderPlayer WrongSide =
		MatchPlayAuthoritativeSessionTests::OtherPlayer(
			ActiveAttack.CurrentAttack.CurrentLegalDeploymentSide);
	const FMatchPlayAuthoritativeFinishDeploymentResult WrongFinish =
		Session.FinishDeployment(
			ActiveAttack.CurrentAttack.AttackSequence,
			WrongSide);
	TestEqual(TEXT("Wrong-side Finish is real domain failure"),
		WrongFinish.FinishResult.ErrorCode,
		EMatchPlayFinishDeploymentErrorCode
			::RequestingSideNotCurrentLegalDeploymentSide);
	MatchPlayAuthoritativeSessionTests::TestNoAdoptDomainFailure(
		*this, TEXT("Wrong-side Finish"), WrongFinish.RuntimeEnvelope,
		ActiveAttack);
	TestTrue(TEXT("All ordinary failures preserve active attack"),
		MatchPlayAuthoritativeSessionTests::AreStatesEqual(
			Session.GetStateSnapshot(),
			ActiveAttack));
	TestTrue(TEXT("Initial Begin succeeded"), Begin.BeginResult.bSuccess);
	return true;
}

MATCH_PLAY_AUTHORITATIVE_SESSION_TEST(
	FMatchPlayAuthoritativeSessionAdoptionPolicyTest,
	"09.StateDispositionPolicy")

bool FMatchPlayAuthoritativeSessionAdoptionPolicyTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayOpeningInitializeResult Opening =
		FMatchPlayOpeningInitializer::InitializeMatchPlayOpening(
			MatchPlayAuthoritativeSessionTests::MakeValidInput(TEXT("Policy")));
	const FMatchPlayBeginOrdinaryAttackResult Begin =
		FMatchPlayBeginOrdinaryAttack::Begin(
			Opening.MatchPlayState,
			6);
	TestTrue(TEXT("Policy fixture opening is canonical"), Opening.bSuccess);
	TestTrue(TEXT("Policy candidate comes from real successful Begin"),
		Begin.bSuccess);

	const FMatchPlayAuthoritativeStateAdoptionResult DoNotAdopt =
		FMatchPlayAuthoritativeStateAdoptionPolicy::Apply(
			Opening.MatchPlayState,
			Begin.AfterState,
			EMatchPlayAuthoritativeStateDisposition::DoNotAdopt);
	TestFalse(TEXT("DoNotAdopt never advances"), DoNotAdopt.bStateAdvanced);
	TestTrue(TEXT("DoNotAdopt selects current canonical state"),
		MatchPlayAuthoritativeSessionTests::AreStatesEqual(
			DoNotAdopt.AdoptedAfterState,
			Opening.MatchPlayState));

	const FMatchPlayAuthoritativeStateAdoptionResult FailureWithProgress =
		FMatchPlayAuthoritativeStateAdoptionPolicy::Apply(
			Opening.MatchPlayState,
			Begin.AfterState,
			EMatchPlayAuthoritativeStateDisposition::Adopt);
	TestTrue(TEXT("Adopt advances independently of domain success"),
		FailureWithProgress.bStateAdvanced);
	TestTrue(TEXT("Adopt selects real canonical candidate"),
		MatchPlayAuthoritativeSessionTests::AreStatesEqual(
			FailureWithProgress.AdoptedAfterState,
			Begin.AfterState));
	return true;
}

MATCH_PLAY_AUTHORITATIVE_SESSION_TEST(
	FMatchPlayAuthoritativeSessionIsolationDeterminismTest,
	"10.SessionIsolationAndIndependentDeterminism")

bool FMatchPlayAuthoritativeSessionIsolationDeterminismTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayAuthoritativeSession SessionA;
	FMatchPlayAuthoritativeSession SessionB;
	SessionA.InitializeMatch(
		MatchPlayAuthoritativeSessionTests::MakeValidInput(
			TEXT("IsolationA"), ECardRarity::Common, ECardRarity::Regional));
	SessionB.InitializeMatch(
		MatchPlayAuthoritativeSessionTests::MakeValidInput(
			TEXT("IsolationB"), ECardRarity::Regional, ECardRarity::Common));
	const FMatchPlayState BBeforeAProgress = SessionB.GetStateSnapshot();
	SessionA.BeginOrdinaryAttack(6);
	TestTrue(TEXT("A progress cannot change B"),
		MatchPlayAuthoritativeSessionTests::AreStatesEqual(
			SessionB.GetStateSnapshot(),
			BBeforeAProgress));
	TestFalse(TEXT("A and B own independent states"),
		MatchPlayAuthoritativeSessionTests::AreStatesEqual(
			SessionA.GetStateSnapshot(),
			SessionB.GetStateSnapshot()));
	FMatchPlayState DetachedB = SessionB.GetStateSnapshot();
	DetachedB.bHasCurrentAttack = true;
	TestTrue(TEXT("B snapshot is detached"),
		MatchPlayAuthoritativeSessionTests::AreStatesEqual(
			SessionB.GetStateSnapshot(),
			BBeforeAProgress));
	const FMatchPlayState ABeforeBProgress = SessionA.GetStateSnapshot();
	SessionB.BeginOrdinaryAttack(6);
	TestTrue(TEXT("B progress cannot change A"),
		MatchPlayAuthoritativeSessionTests::AreStatesEqual(
			SessionA.GetStateSnapshot(),
			ABeforeBProgress));

	const FMatchPlayOpeningInitializeInput Input =
		MatchPlayAuthoritativeSessionTests::MakeValidInput(TEXT("Determinism"));
	FMatchPlayAuthoritativeSession Sessions[3];
	FMatchPlayAuthoritativeInitializeMatchResult Initialize[3];
	FMatchPlayAuthoritativeInitializeMatchResult Repeat[3];
	FMatchPlayAuthoritativeBeginOrdinaryAttackResult Begin[3];
	FMatchPlayAuthoritativeFinishDeploymentResult Finish[3];
	for (int32 Index = 0; Index < 3; ++Index)
	{
		Initialize[Index] = Sessions[Index].InitializeMatch(Input);
		Repeat[Index] = Sessions[Index].InitializeMatch(Input);
		Begin[Index] = Sessions[Index].BeginOrdinaryAttack(6);
		const FMatchPlayState Active = Sessions[Index].GetStateSnapshot();
		Finish[Index] = Sessions[Index].FinishDeployment(
			Active.CurrentAttack.AttackSequence,
			Active.CurrentAttack.CurrentLegalDeploymentSide);
	}
	for (int32 Index = 1; Index < 3; ++Index)
	{
		TestTrue(TEXT("Success initialize deterministic"),
			MatchPlayAuthoritativeSessionTests::AreInitializeResultsEqual(
				Initialize[0], Initialize[Index]));
		TestTrue(TEXT("Repeat gate deterministic"),
			MatchPlayAuthoritativeSessionTests::AreInitializeResultsEqual(
				Repeat[0], Repeat[Index]));
		TestTrue(TEXT("Legal Begin deterministic"),
			MatchPlayAuthoritativeSessionTests
				::AreAuthoritativeBeginResultsEqual(Begin[0], Begin[Index]));
		TestTrue(TEXT("Legal Finish deterministic"),
			MatchPlayAuthoritativeSessionTests
				::AreAuthoritativeFinishResultsEqual(Finish[0], Finish[Index]));
		TestTrue(TEXT("Final session snapshot deterministic"),
			MatchPlayAuthoritativeSessionTests::AreStatesEqual(
				Sessions[0].GetStateSnapshot(),
				Sessions[Index].GetStateSnapshot()));
	}

	FMatchPlayAuthoritativeInitializeMatchResult FailedInitialize[3];
	FMatchPlayAuthoritativeBeginOrdinaryAttackResult UninitializedBegin[3];
	FMatchPlayAuthoritativeBeginOrdinaryAttackResult DomainFailure[3];
	FMatchPlayAuthoritativeSession FailureSessions[3];
	FMatchPlayAuthoritativeSession UninitializedSessions[3];
	FMatchPlayAuthoritativeSession DomainFailureSessions[3];
	for (int32 Index = 0; Index < 3; ++Index)
	{
		FailedInitialize[Index] = FailureSessions[Index].InitializeMatch(
			MatchPlayAuthoritativeSessionTests::MakeInvalidInput());
		UninitializedBegin[Index] =
			UninitializedSessions[Index].BeginOrdinaryAttack(6);
		DomainFailureSessions[Index].InitializeMatch(Input);
		DomainFailure[Index] =
			DomainFailureSessions[Index].BeginOrdinaryAttack(1);
	}
	for (int32 Index = 1; Index < 3; ++Index)
	{
		TestTrue(TEXT("Failed initialize deterministic"),
			MatchPlayAuthoritativeSessionTests::AreInitializeResultsEqual(
				FailedInitialize[0], FailedInitialize[Index]));
		TestTrue(TEXT("Uninitialized Begin gate deterministic"),
			MatchPlayAuthoritativeSessionTests
				::AreAuthoritativeBeginResultsEqual(
					UninitializedBegin[0], UninitializedBegin[Index]));
		TestTrue(TEXT("Ordinary domain failure deterministic"),
			MatchPlayAuthoritativeSessionTests
				::AreAuthoritativeBeginResultsEqual(
					DomainFailure[0], DomainFailure[Index]));
	}
	return true;
}

MATCH_PLAY_AUTHORITATIVE_SESSION_TEST(
	FMatchPlayAuthoritativeSessionComparatorBoundaryTest,
	"11.ResultComparatorAndRuntimeBoundary")

bool FMatchPlayAuthoritativeSessionComparatorBoundaryTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayAuthoritativeSession Session;
	const FMatchPlayAuthoritativeInitializeMatchResult Initialize =
		Session.InitializeMatch(
			MatchPlayAuthoritativeSessionTests::MakeValidInput(TEXT("Comparator")));
	const FMatchPlayAuthoritativeBeginOrdinaryAttackResult Begin =
		Session.BeginOrdinaryAttack(6);
	const FMatchPlayState Active = Session.GetStateSnapshot();
	const FMatchPlayAuthoritativeFinishDeploymentResult Finish =
		Session.FinishDeployment(
			Active.CurrentAttack.AttackSequence,
			Active.CurrentAttack.CurrentLegalDeploymentSide);

	TestTrue(TEXT("Initialize comparator accepts equal value"),
		MatchPlayAuthoritativeSessionTests::AreInitializeResultsEqual(
			Initialize, Initialize));
	TestTrue(TEXT("Begin comparator accepts equal value"),
		MatchPlayAuthoritativeSessionTests
			::AreAuthoritativeBeginResultsEqual(Begin, Begin));
	TestTrue(TEXT("Finish comparator accepts equal value"),
		MatchPlayAuthoritativeSessionTests
			::AreAuthoritativeFinishResultsEqual(Finish, Finish));

	MatchPlayAuthoritativeSessionTests::TestEnvelopeMutationCoverage(
		*this,
		Initialize.RuntimeEnvelope);
	MatchPlayAuthoritativeSessionTests::TestOpeningMutationCoverage(
		*this,
		Initialize);
	MatchPlayAuthoritativeSessionTests::TestBeginMutationCoverage(
		*this,
		Begin);
	MatchPlayAuthoritativeSessionTests::TestFinishMutationCoverage(
		*this,
		Finish);
	return true;
}

#undef MATCH_PLAY_AUTHORITATIVE_SESSION_TEST

#endif
