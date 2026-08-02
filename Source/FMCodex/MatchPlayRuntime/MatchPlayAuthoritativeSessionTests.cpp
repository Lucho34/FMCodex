#include "MatchPlayAuthoritativeSession.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "../CoreRules/MatchPlayCurrentAttackCarrierSelectionAvailability.h"
#include "../CoreRules/MatchPlayOrdinaryDeploymentAvailability.h"

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

	bool AreDeploymentRequestsEqual(
		const FMatchPlayOrdinaryDeploymentRequest& Left,
		const FMatchPlayOrdinaryDeploymentRequest& Right)
	{
		return Left.AttackSequence == Right.AttackSequence
			&& Left.RequestingSide == Right.RequestingSide
			&& Left.CardId == Right.CardId
			&& Left.SlotId == Right.SlotId;
	}

	bool AreDeploymentLegalityResultsEqual(
		const FMatchPlayOrdinaryDeploymentLegalityResult& Left,
		const FMatchPlayOrdinaryDeploymentLegalityResult& Right)
	{
		return Left.bIsLegal == Right.bIsLegal
			&& AreDeploymentRequestsEqual(Left.Request, Right.Request)
			&& Left.ErrorCode == Right.ErrorCode
			&& Left.UnderlyingSnapshotAuthorityQueryErrorCode
				== Right.UnderlyingSnapshotAuthorityQueryErrorCode
			&& Left.UnderlyingPlayCardErrorCode
				== Right.UnderlyingPlayCardErrorCode
			&& Left.UnderlyingCardUsageErrorCode
				== Right.UnderlyingCardUsageErrorCode
			&& Left.UnderlyingSlotCatalogQueryErrorCode
				== Right.UnderlyingSlotCatalogQueryErrorCode
			&& Left.UnderlyingRelativeZoneResolutionErrorCode
				== Right.UnderlyingRelativeZoneResolutionErrorCode
			&& Left.ResolvedRelativeZone == Right.ResolvedRelativeZone
			&& Left.ErrorMessage == Right.ErrorMessage;
	}

	bool AreDeploymentWriterResultsEqual(
		const FMatchPlayOrdinaryDeploymentWriterResult& Left,
		const FMatchPlayOrdinaryDeploymentWriterResult& Right)
	{
		return Left.bSuccess == Right.bSuccess
			&& AreDeploymentRequestsEqual(Left.Request, Right.Request)
			&& AreStatesEqual(Left.BeforeState, Right.BeforeState)
			&& AreStatesEqual(Left.AfterState, Right.AfterState)
			&& Left.ErrorCode == Right.ErrorCode
			&& AreDeploymentLegalityResultsEqual(
				Left.LegalityResult,
				Right.LegalityResult)
			&& Left.UnderlyingTurnRotationErrorCode
				== Right.UnderlyingTurnRotationErrorCode
			&& Left.ErrorMessage == Right.ErrorMessage;
	}

	bool AreAuthoritativeDeployOrdinaryResultsEqual(
		const FMatchPlayAuthoritativeDeployOrdinaryResult& Left,
		const FMatchPlayAuthoritativeDeployOrdinaryResult& Right)
	{
		return AreEnvelopesEqual(
				Left.RuntimeEnvelope,
				Right.RuntimeEnvelope)
			&& AreDeploymentWriterResultsEqual(
				Left.DeploymentResult,
				Right.DeploymentResult);
	}

	bool AreCarrierRequestsEqual(
		const FMatchPlayCurrentAttackCarrierSelectionRequest& Left,
		const FMatchPlayCurrentAttackCarrierSelectionRequest& Right)
	{
		return Left.AttackSequence == Right.AttackSequence
			&& Left.RequestingSide == Right.RequestingSide
			&& Left.CarrierCardId == Right.CarrierCardId;
	}

	bool AreCarrierLegalityResultsEqual(
		const FMatchPlayCurrentAttackCarrierSelectionLegalityResult& Left,
		const FMatchPlayCurrentAttackCarrierSelectionLegalityResult& Right)
	{
		return Left.bIsLegal == Right.bIsLegal
			&& AreCarrierRequestsEqual(Left.Request, Right.Request)
			&& Left.ErrorCode == Right.ErrorCode
			&& AreSelectionValidationResultsEqual(
				Left.SelectionStateValidationResult,
				Right.SelectionStateValidationResult)
			&& Left.UnderlyingSnapshotAuthorityQueryErrorCode
				== Right.UnderlyingSnapshotAuthorityQueryErrorCode
			&& Left.MatchingCarrierPlacementCount
				== Right.MatchingCarrierPlacementCount
			&& Left.ErrorMessage == Right.ErrorMessage;
	}

	bool AreCarrierWriterResultsEqual(
		const FMatchPlayCurrentAttackCarrierSelectionWriterResult& Left,
		const FMatchPlayCurrentAttackCarrierSelectionWriterResult& Right)
	{
		return Left.bSuccess == Right.bSuccess
			&& AreCarrierRequestsEqual(Left.Request, Right.Request)
			&& AreStatesEqual(Left.BeforeState, Right.BeforeState)
			&& AreStatesEqual(Left.AfterState, Right.AfterState)
			&& Left.ErrorCode == Right.ErrorCode
			&& AreCarrierLegalityResultsEqual(
				Left.LegalityResult,
				Right.LegalityResult)
			&& Left.SelectedCarrierCardId == Right.SelectedCarrierCardId
			&& Left.ErrorMessage == Right.ErrorMessage;
	}

	bool AreAuthoritativeSubmitCarrierResultsEqual(
		const FMatchPlayAuthoritativeSubmitCarrierResult& Left,
		const FMatchPlayAuthoritativeSubmitCarrierResult& Right)
	{
		return AreEnvelopesEqual(
				Left.RuntimeEnvelope,
				Right.RuntimeEnvelope)
			&& AreCarrierWriterResultsEqual(
				Left.CarrierResult,
				Right.CarrierResult);
	}

	EInitialTurnOrderPlayer OtherPlayer(
		const EInitialTurnOrderPlayer Player)
	{
		return Player == EInitialTurnOrderPlayer::PlayerA
			? EInitialTurnOrderPlayer::PlayerB
			: EInitialTurnOrderPlayer::PlayerA;
	}

	const TArray<FName>& AvailableCardIdsForSide(
		const FMatchPlayState& State,
		const EInitialTurnOrderPlayer Side)
	{
		return Side == EInitialTurnOrderPlayer::PlayerA
			? State.CardUsageState.PlayerACardUsageState.AvailableCardIds
			: State.CardUsageState.PlayerBCardUsageState.AvailableCardIds;
	}

	struct FDeploymentChoice
	{
		EInitialTurnOrderPlayer Side = EInitialTurnOrderPlayer::None;
		FName CardId = NAME_None;
		FName SlotId = NAME_None;
		EMatchPlayRelativeDeploymentZone ResolvedRelativeZone =
			EMatchPlayRelativeDeploymentZone::None;
	};

	bool FindLegalDeployment(
		const FMatchPlayState& State,
		const EMatchPlayRelativeDeploymentZone PreferredZone,
		FDeploymentChoice& OutChoice)
	{
		if (!State.bHasCurrentAttack)
		{
			return false;
		}

		const EInitialTurnOrderPlayer Side =
			State.CurrentAttack.CurrentLegalDeploymentSide;
		for (const FName CardId : AvailableCardIdsForSide(State, Side))
		{
			const FMatchPlayOrdinaryDeploymentAvailabilityResult Availability =
				FMatchPlayOrdinaryDeploymentAvailability::Query(
					State,
					State.CurrentAttack.AttackSequence,
					Side,
					CardId);
			for (const FMatchPlayOrdinaryDeploymentSlotAvailability& Slot :
				Availability.SlotResults)
			{
				if (Slot.LegalityResult.bIsLegal
					&& Slot.LegalityResult.ResolvedRelativeZone == PreferredZone)
				{
					OutChoice.Side = Side;
					OutChoice.CardId = CardId;
					OutChoice.SlotId = Slot.SlotId;
					OutChoice.ResolvedRelativeZone =
						Slot.LegalityResult.ResolvedRelativeZone;
					return true;
				}
			}
		}
		return false;
	}

	FMatchPlayAuthoritativeDeployOrdinaryRequest MakeDeployRequest(
		const FDeploymentChoice& Choice)
	{
		FMatchPlayAuthoritativeDeployOrdinaryRequest Request;
		Request.RequestingSide = Choice.Side;
		Request.CardId = Choice.CardId;
		Request.SlotId = Choice.SlotId;
		return Request;
	}

	struct FReachabilityTrace
	{
		FMatchPlayAuthoritativeInitializeMatchResult Initialize;
		FMatchPlayAuthoritativeBeginOrdinaryAttackResult Begin;
		FDeploymentChoice FirstChoice;
		FMatchPlayAuthoritativeDeployOrdinaryResult FirstDeploy;
		FMatchPlayAuthoritativeFinishDeploymentResult FirstFinish;
		FDeploymentChoice SecondChoice;
		FMatchPlayAuthoritativeDeployOrdinaryResult SecondDeploy;
		FMatchPlayAuthoritativeFinishDeploymentResult SecondFinish;
		EInitialTurnOrderPlayer AttackingSide =
			EInitialTurnOrderPlayer::None;
		FName CarrierCardId = NAME_None;
	};

	bool BuildToAwaitingCarrier(
		FMatchPlayAuthoritativeSession& Session,
		const FString& Prefix,
		FReachabilityTrace& OutTrace)
	{
		OutTrace.Initialize = Session.InitializeMatch(MakeValidInput(Prefix));
		OutTrace.Begin = Session.BeginOrdinaryAttack(6);
		if (!OutTrace.Initialize.OpeningResult.bSuccess
			|| !OutTrace.Begin.BeginResult.bSuccess)
		{
			return false;
		}

		FMatchPlayState State = Session.GetStateSnapshot();
		OutTrace.AttackingSide = State.RuntimeState.CurrentAttackingPlayer;
		if (!FindLegalDeployment(
			State,
			EMatchPlayRelativeDeploymentZone::Forward,
			OutTrace.FirstChoice))
		{
			return false;
		}
		OutTrace.FirstDeploy = Session.DeployOrdinary(
			MakeDeployRequest(OutTrace.FirstChoice));
		if (!OutTrace.FirstDeploy.DeploymentResult.bSuccess)
		{
			return false;
		}

		State = Session.GetStateSnapshot();
		if (!FindLegalDeployment(
			State,
			EMatchPlayRelativeDeploymentZone::Midfield,
			OutTrace.SecondChoice))
		{
			return false;
		}
		OutTrace.SecondDeploy = Session.DeployOrdinary(
			MakeDeployRequest(OutTrace.SecondChoice));
		if (!OutTrace.SecondDeploy.DeploymentResult.bSuccess)
		{
			return false;
		}

		State = Session.GetStateSnapshot();
		OutTrace.FirstFinish = Session.FinishDeployment(
			State.CurrentAttack.AttackSequence,
			State.CurrentAttack.CurrentLegalDeploymentSide);
		if (!OutTrace.FirstFinish.FinishResult.bSuccess)
		{
			return false;
		}

		State = Session.GetStateSnapshot();
		OutTrace.SecondFinish = Session.FinishDeployment(
			State.CurrentAttack.AttackSequence,
			State.CurrentAttack.CurrentLegalDeploymentSide);
		if (!OutTrace.SecondFinish.FinishResult.bSuccess)
		{
			return false;
		}

		State = Session.GetStateSnapshot();
		const FMatchPlayCurrentAttackCarrierSelectionAvailabilityResult
			Availability =
				FMatchPlayCurrentAttackCarrierSelectionAvailability::Query(
					State,
					State.CurrentAttack.AttackSequence,
					OutTrace.AttackingSide);
		for (const auto& Candidate : Availability.Candidates)
		{
			if (Candidate.LegalityResult.bIsLegal)
			{
				OutTrace.CarrierCardId = Candidate.CarrierCardId;
				return true;
			}
		}
		return false;
	}

	FMatchPlayAuthoritativeSubmitCarrierRequest MakeCarrierRequest(
		const FReachabilityTrace& Trace)
	{
		FMatchPlayAuthoritativeSubmitCarrierRequest Request;
		Request.RequestingSide = Trace.AttackingSide;
		Request.CarrierCardId = Trace.CarrierCardId;
		return Request;
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

	void TestDeployMutationCoverage(
		FAutomationTestBase& Test,
		const FMatchPlayAuthoritativeDeployOrdinaryResult& Baseline)
	{
		auto RejectEqual = [&Test, &Baseline](
			const TCHAR* Field,
			const FMatchPlayAuthoritativeDeployOrdinaryResult& Mutated)
		{
			Test.TestFalse(
				Field,
				AreAuthoritativeDeployOrdinaryResultsEqual(Baseline, Mutated));
		};

		FMatchPlayAuthoritativeDeployOrdinaryResult Mutated = Baseline;
		Mutated.RuntimeEnvelope.bRuntimeFault = true;
		RejectEqual(TEXT("Deploy comparator covers envelope"), Mutated);
		Mutated = Baseline;
		Mutated.DeploymentResult.bSuccess = !Mutated.DeploymentResult.bSuccess;
		RejectEqual(TEXT("Deploy comparator covers bSuccess"), Mutated);
		Mutated = Baseline;
		++Mutated.DeploymentResult.Request.AttackSequence;
		RejectEqual(TEXT("Deploy comparator covers request sequence"), Mutated);
		Mutated = Baseline;
		Mutated.DeploymentResult.Request.RequestingSide =
			OtherPlayer(Mutated.DeploymentResult.Request.RequestingSide);
		RejectEqual(TEXT("Deploy comparator covers request side"), Mutated);
		Mutated = Baseline;
		Mutated.DeploymentResult.Request.CardId = TEXT("Mutated.Card");
		RejectEqual(TEXT("Deploy comparator covers request CardId"), Mutated);
		Mutated = Baseline;
		Mutated.DeploymentResult.Request.SlotId = TEXT("Mutated.Slot");
		RejectEqual(TEXT("Deploy comparator covers request SlotId"), Mutated);
		Mutated = Baseline;
		Mutated.DeploymentResult.BeforeState.bHasCurrentAttack = false;
		RejectEqual(TEXT("Deploy comparator covers BeforeState"), Mutated);
		Mutated = Baseline;
		Mutated.DeploymentResult.AfterState.bHasCurrentAttack = false;
		RejectEqual(TEXT("Deploy comparator covers AfterState"), Mutated);
		Mutated = Baseline;
		Mutated.DeploymentResult.ErrorCode =
			EMatchPlayOrdinaryDeploymentWriterErrorCode::LegalityFailed;
		RejectEqual(TEXT("Deploy comparator covers ErrorCode"), Mutated);
		Mutated = Baseline;
		Mutated.DeploymentResult.LegalityResult.bIsLegal =
			!Mutated.DeploymentResult.LegalityResult.bIsLegal;
		RejectEqual(TEXT("Deploy comparator covers legality flag"), Mutated);
		Mutated = Baseline;
		++Mutated.DeploymentResult.LegalityResult.Request.AttackSequence;
		RejectEqual(TEXT("Deploy comparator covers legality request sequence"), Mutated);
		Mutated = Baseline;
		Mutated.DeploymentResult.LegalityResult.Request.RequestingSide =
			OtherPlayer(Mutated.DeploymentResult.LegalityResult.Request.RequestingSide);
		RejectEqual(TEXT("Deploy comparator covers legality request side"), Mutated);
		Mutated = Baseline;
		Mutated.DeploymentResult.LegalityResult.Request.CardId =
			TEXT("Mutated.Legality.Card");
		RejectEqual(TEXT("Deploy comparator covers legality request CardId"), Mutated);
		Mutated = Baseline;
		Mutated.DeploymentResult.LegalityResult.Request.SlotId =
			TEXT("Mutated.Legality.Slot");
		RejectEqual(TEXT("Deploy comparator covers legality request SlotId"), Mutated);
		Mutated = Baseline;
		Mutated.DeploymentResult.LegalityResult.ErrorCode =
			EMatchPlayOrdinaryDeploymentErrorCode::InvalidCardId;
		RejectEqual(TEXT("Deploy comparator covers legality ErrorCode"), Mutated);
		Mutated = Baseline;
		Mutated.DeploymentResult.LegalityResult
			.UnderlyingSnapshotAuthorityQueryErrorCode =
				static_cast<EMatchPlayCardSnapshotAuthorityQueryErrorCode>(1);
		RejectEqual(TEXT("Deploy comparator covers authority query error"), Mutated);
		Mutated = Baseline;
		Mutated.DeploymentResult.LegalityResult.UnderlyingPlayCardErrorCode =
			static_cast<EPlayCardResolveErrorCode>(1);
		RejectEqual(TEXT("Deploy comparator covers play-card error"), Mutated);
		Mutated = Baseline;
		Mutated.DeploymentResult.LegalityResult.UnderlyingCardUsageErrorCode =
			static_cast<ECardUsageResolveErrorCode>(1);
		RejectEqual(TEXT("Deploy comparator covers card-usage error"), Mutated);
		Mutated = Baseline;
		Mutated.DeploymentResult.LegalityResult
			.UnderlyingSlotCatalogQueryErrorCode =
				static_cast<EMatchPlayDeploymentSlotCatalogQueryErrorCode>(1);
		RejectEqual(TEXT("Deploy comparator covers slot-catalog error"), Mutated);
		Mutated = Baseline;
		Mutated.DeploymentResult.LegalityResult
			.UnderlyingRelativeZoneResolutionErrorCode =
				static_cast<EMatchPlayRelativeDeploymentZoneResolveErrorCode>(1);
		RejectEqual(TEXT("Deploy comparator covers relative-zone error"), Mutated);
		Mutated = Baseline;
		Mutated.DeploymentResult.LegalityResult.ResolvedRelativeZone =
			EMatchPlayRelativeDeploymentZone::None;
		RejectEqual(TEXT("Deploy comparator covers resolved zone"), Mutated);
		Mutated = Baseline;
		Mutated.DeploymentResult.LegalityResult.ErrorMessage =
			TEXT("mutated deploy legality message");
		RejectEqual(TEXT("Deploy comparator covers legality message"), Mutated);
		Mutated = Baseline;
		Mutated.DeploymentResult.UnderlyingTurnRotationErrorCode =
			static_cast<EMatchPlayDeploymentTurnRotationErrorCode>(1);
		RejectEqual(TEXT("Deploy comparator covers rotation error"), Mutated);
		Mutated = Baseline;
		Mutated.DeploymentResult.ErrorMessage = TEXT("mutated deploy message");
		RejectEqual(TEXT("Deploy comparator covers message"), Mutated);
	}

	void TestCarrierMutationCoverage(
		FAutomationTestBase& Test,
		const FMatchPlayAuthoritativeSubmitCarrierResult& Baseline)
	{
		auto RejectEqual = [&Test, &Baseline](
			const TCHAR* Field,
			const FMatchPlayAuthoritativeSubmitCarrierResult& Mutated)
		{
			Test.TestFalse(
				Field,
				AreAuthoritativeSubmitCarrierResultsEqual(Baseline, Mutated));
		};

		FMatchPlayAuthoritativeSubmitCarrierResult Mutated = Baseline;
		Mutated.RuntimeEnvelope.bRuntimeFault = true;
		RejectEqual(TEXT("Carrier comparator covers envelope"), Mutated);
		Mutated = Baseline;
		Mutated.CarrierResult.bSuccess = !Mutated.CarrierResult.bSuccess;
		RejectEqual(TEXT("Carrier comparator covers bSuccess"), Mutated);
		Mutated = Baseline;
		++Mutated.CarrierResult.Request.AttackSequence;
		RejectEqual(TEXT("Carrier comparator covers request sequence"), Mutated);
		Mutated = Baseline;
		Mutated.CarrierResult.Request.RequestingSide =
			OtherPlayer(Mutated.CarrierResult.Request.RequestingSide);
		RejectEqual(TEXT("Carrier comparator covers request side"), Mutated);
		Mutated = Baseline;
		Mutated.CarrierResult.Request.CarrierCardId = TEXT("Mutated.Carrier");
		RejectEqual(TEXT("Carrier comparator covers request CardId"), Mutated);
		Mutated = Baseline;
		Mutated.CarrierResult.BeforeState.bHasCurrentAttack = false;
		RejectEqual(TEXT("Carrier comparator covers BeforeState"), Mutated);
		Mutated = Baseline;
		Mutated.CarrierResult.AfterState.bHasCurrentAttack = false;
		RejectEqual(TEXT("Carrier comparator covers AfterState"), Mutated);
		Mutated = Baseline;
		Mutated.CarrierResult.ErrorCode =
			EMatchPlayCurrentAttackCarrierSelectionWriterErrorCode::LegalityFailed;
		RejectEqual(TEXT("Carrier comparator covers ErrorCode"), Mutated);
		Mutated = Baseline;
		Mutated.CarrierResult.LegalityResult.bIsLegal =
			!Mutated.CarrierResult.LegalityResult.bIsLegal;
		RejectEqual(TEXT("Carrier comparator covers legality flag"), Mutated);
		Mutated = Baseline;
		++Mutated.CarrierResult.LegalityResult.Request.AttackSequence;
		RejectEqual(TEXT("Carrier comparator covers legality request sequence"), Mutated);
		Mutated = Baseline;
		Mutated.CarrierResult.LegalityResult.Request.RequestingSide =
			OtherPlayer(Mutated.CarrierResult.LegalityResult.Request.RequestingSide);
		RejectEqual(TEXT("Carrier comparator covers legality request side"), Mutated);
		Mutated = Baseline;
		Mutated.CarrierResult.LegalityResult.Request.CarrierCardId =
			TEXT("Mutated.Legality.Carrier");
		RejectEqual(TEXT("Carrier comparator covers legality request CardId"), Mutated);
		Mutated = Baseline;
		Mutated.CarrierResult.LegalityResult.ErrorCode =
			EMatchPlayCurrentAttackCarrierSelectionErrorCode::InvalidCarrierCardId;
		RejectEqual(TEXT("Carrier comparator covers legality ErrorCode"), Mutated);
		Mutated = Baseline;
		Mutated.CarrierResult.LegalityResult.SelectionStateValidationResult
			.bIsCanonical = !Mutated.CarrierResult.LegalityResult
				.SelectionStateValidationResult.bIsCanonical;
		RejectEqual(TEXT("Carrier comparator covers selection canonical flag"), Mutated);
		Mutated = Baseline;
		Mutated.CarrierResult.LegalityResult.SelectionStateValidationResult
			.ErrorCode = EMatchPlayCurrentAttackSelectionStateValidationErrorCode
				::UnsupportedCurrentAttackPhase;
		RejectEqual(TEXT("Carrier comparator covers selection error"), Mutated);
		Mutated = Baseline;
		Mutated.CarrierResult.LegalityResult.SelectionStateValidationResult
			.ErrorMessage = TEXT("mutated selection validation message");
		RejectEqual(TEXT("Carrier comparator covers selection message"), Mutated);
		Mutated = Baseline;
		Mutated.CarrierResult.LegalityResult
			.UnderlyingSnapshotAuthorityQueryErrorCode =
				static_cast<EMatchPlayCardSnapshotAuthorityQueryErrorCode>(1);
		RejectEqual(TEXT("Carrier comparator covers authority query error"), Mutated);
		Mutated = Baseline;
		++Mutated.CarrierResult.LegalityResult.MatchingCarrierPlacementCount;
		RejectEqual(TEXT("Carrier comparator covers matching placement count"), Mutated);
		Mutated = Baseline;
		Mutated.CarrierResult.LegalityResult.ErrorMessage =
			TEXT("mutated carrier legality message");
		RejectEqual(TEXT("Carrier comparator covers legality message"), Mutated);
		Mutated = Baseline;
		Mutated.CarrierResult.SelectedCarrierCardId = TEXT("Mutated.Selected");
		RejectEqual(TEXT("Carrier comparator covers selected CardId"), Mutated);
		Mutated = Baseline;
		Mutated.CarrierResult.ErrorMessage = TEXT("mutated carrier message");
		RejectEqual(TEXT("Carrier comparator covers message"), Mutated);
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
		EMatchPlayAuthoritativeCommandKind::DeployOrdinary) == 4);
	static_assert(static_cast<uint8>(
		EMatchPlayAuthoritativeCommandKind::SubmitCarrier) == 5);
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
	TestEqual(TEXT("All five mutations use the gate"),
		MatchPlayAuthoritativeSessionTests::CountOccurrences(
			Implementation,
			TEXT("ExecuteSerialized<")),
		5);
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
	TestFalse(TEXT("Direct CurrentAttack authority replacement absent"),
		Implementation.Contains(TEXT("AuthoritativeState.CurrentAttack =")));
	TestFalse(TEXT("Direct Deployment placement authority write absent"),
		Implementation.Contains(TEXT("AuthoritativeState.CurrentAttack.DeploymentPlacements")));
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
	const FMatchPlayState DefaultState;
	const FMatchPlayState InitialASnapshot = SessionA.GetStateSnapshot();
	const FMatchPlayState InitialBSnapshot = SessionB.GetStateSnapshot();
	TestTrue(TEXT("SessionA starts with the complete default state"),
		MatchPlayAuthoritativeSessionTests::AreStatesEqual(
			InitialASnapshot,
			DefaultState));
	TestTrue(TEXT("SessionB starts with the complete default state"),
		MatchPlayAuthoritativeSessionTests::AreStatesEqual(
			InitialBSnapshot,
			DefaultState));

	const FMatchPlayAuthoritativeInitializeMatchResult AInitialize =
		SessionA.InitializeMatch(
		MatchPlayAuthoritativeSessionTests::MakeValidInput(
			TEXT("IsolationA"), ECardRarity::Common, ECardRarity::Regional));
	const FMatchPlayState AAfterInitialize = SessionA.GetStateSnapshot();
	const FMatchPlayState BAfterAInitialize = SessionB.GetStateSnapshot();
	TestTrue(TEXT("SessionA initialization succeeds"),
		AInitialize.OpeningResult.bSuccess);
	TestTrue(TEXT("SessionA adopts its initialization result"),
		MatchPlayAuthoritativeSessionTests::AreStatesEqual(
			AAfterInitialize,
			AInitialize.RuntimeEnvelope.AfterState));
	TestTrue(TEXT("SessionA is initialized canonically"),
		AAfterInitialize.RuntimeState.bIsInitialized);
	TestTrue(TEXT("SessionA initialization leaves SessionB fully default"),
		MatchPlayAuthoritativeSessionTests::AreStatesEqual(
			BAfterAInitialize,
			DefaultState));
	TestFalse(TEXT("SessionA initialization leaves SessionB uninitialized"),
		BAfterAInitialize.RuntimeState.bIsInitialized);

	const FMatchPlayState ABeforeBInitialize = SessionA.GetStateSnapshot();
	const FMatchPlayAuthoritativeInitializeMatchResult BInitialize =
		SessionB.InitializeMatch(
		MatchPlayAuthoritativeSessionTests::MakeValidInput(
			TEXT("IsolationB"), ECardRarity::Regional, ECardRarity::Common));
	const FMatchPlayState AAfterBInitialize = SessionA.GetStateSnapshot();
	const FMatchPlayState BAfterInitialize = SessionB.GetStateSnapshot();
	TestTrue(TEXT("SessionB initialization succeeds"),
		BInitialize.OpeningResult.bSuccess);
	TestTrue(TEXT("SessionB adopts its initialization result"),
		MatchPlayAuthoritativeSessionTests::AreStatesEqual(
			BAfterInitialize,
			BInitialize.RuntimeEnvelope.AfterState));
	TestTrue(TEXT("SessionB is initialized canonically"),
		BAfterInitialize.RuntimeState.bIsInitialized);
	TestTrue(TEXT("SessionB initialization leaves SessionA frozen"),
		MatchPlayAuthoritativeSessionTests::AreStatesEqual(
			AAfterBInitialize,
			ABeforeBInitialize));
	TestTrue(TEXT("SessionB initialization preserves SessionA result"),
		MatchPlayAuthoritativeSessionTests::AreStatesEqual(
			AAfterBInitialize,
			AAfterInitialize));

	const FMatchPlayState BBeforeAProgress = BAfterInitialize;
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

	FMatchPlayAuthoritativeSession WrongFinishSessions[3];
	FMatchPlayAuthoritativeFinishDeploymentResult WrongFinishResults[3];
	FMatchPlayState WrongFinishBeforeStates[3];
	FMatchPlayState WrongFinishFinalStates[3];
	int64 CommonAttackSequence = 0;
	EInitialTurnOrderPlayer CommonLegalSide =
		EInitialTurnOrderPlayer::None;
	EInitialTurnOrderPlayer CommonWrongSide =
		EInitialTurnOrderPlayer::None;
	for (int32 Index = 0; Index < 3; ++Index)
	{
		const FMatchPlayAuthoritativeInitializeMatchResult InitializeResult =
			WrongFinishSessions[Index].InitializeMatch(Input);
		const FMatchPlayAuthoritativeBeginOrdinaryAttackResult BeginResult =
			WrongFinishSessions[Index].BeginOrdinaryAttack(6);
		TestTrue(TEXT("Wrong-side fixture initialization succeeds"),
			InitializeResult.OpeningResult.bSuccess);
		TestTrue(TEXT("Wrong-side fixture Begin succeeds"),
			BeginResult.BeginResult.bSuccess);

		WrongFinishBeforeStates[Index] =
			WrongFinishSessions[Index].GetStateSnapshot();
		const FMatchPlayState& BeforeState =
			WrongFinishBeforeStates[Index];
		TestTrue(TEXT("Wrong-side fixture is initialized"),
			BeforeState.RuntimeState.bIsInitialized);
		TestTrue(TEXT("Wrong-side fixture has a current attack"),
			BeforeState.bHasCurrentAttack);
		const FMatchPlayCurrentAttackSelectionStateValidationResult
			SelectionValidation =
				FMatchPlayCurrentAttackSelectionStateValidator::Validate(
					BeforeState.CurrentAttack);
		TestTrue(TEXT("Wrong-side fixture current attack is canonical"),
			SelectionValidation.bIsCanonical);

		if (Index == 0)
		{
			CommonAttackSequence =
				BeforeState.CurrentAttack.AttackSequence;
			CommonLegalSide =
				BeforeState.CurrentAttack.CurrentLegalDeploymentSide;
			CommonWrongSide =
				MatchPlayAuthoritativeSessionTests::OtherPlayer(
					CommonLegalSide);
		}
		else
		{
			TestTrue(TEXT("Wrong-side fixture BeforeStates are identical"),
				MatchPlayAuthoritativeSessionTests::AreStatesEqual(
					WrongFinishBeforeStates[0],
					BeforeState));
			TestEqual(TEXT("Wrong-side fixture sequences are identical"),
				BeforeState.CurrentAttack.AttackSequence,
				CommonAttackSequence);
			TestEqual(TEXT("Wrong-side fixture legal sides are identical"),
				BeforeState.CurrentAttack.CurrentLegalDeploymentSide,
				CommonLegalSide);
		}
		TestTrue(TEXT("Computed wrong side is a player"),
			CommonWrongSide == EInitialTurnOrderPlayer::PlayerA
				|| CommonWrongSide == EInitialTurnOrderPlayer::PlayerB);
		TestTrue(TEXT("Computed wrong side is actually illegal"),
			CommonWrongSide !=
				BeforeState.CurrentAttack.CurrentLegalDeploymentSide);

		WrongFinishResults[Index] =
			WrongFinishSessions[Index].FinishDeployment(
				CommonAttackSequence,
				CommonWrongSide);
		const FMatchPlayAuthoritativeRuntimeEnvelope& Envelope =
			WrongFinishResults[Index].RuntimeEnvelope;
		TestTrue(TEXT("Wrong-side Finish reaches the domain"),
			Envelope.bAccepted);
		TestFalse(TEXT("Wrong-side Finish domain fails"),
			Envelope.bDomainSuccess);
		TestEqual(TEXT("Wrong-side Finish does not adopt"),
			Envelope.StateDisposition,
			EMatchPlayAuthoritativeStateDisposition::DoNotAdopt);
		TestFalse(TEXT("Wrong-side Finish does not advance"),
			Envelope.bStateAdvanced);
		TestFalse(TEXT("Wrong-side Finish is not a runtime fault"),
			Envelope.bRuntimeFault);
		TestEqual(TEXT("Wrong-side Finish has exact command kind"),
			Envelope.CommandKind,
			EMatchPlayAuthoritativeCommandKind::FinishDeployment);
		TestEqual(TEXT("Wrong-side Finish has no failure disposition"),
			Envelope.FailureDisposition,
			EMatchPlayAuthoritativeFailureDisposition::None);
		TestEqual(TEXT("Wrong-side Finish has no runtime failure"),
			Envelope.RuntimeFailureCode,
			EMatchPlayAuthoritativeRuntimeFailureCode::None);
		TestTrue(TEXT("Wrong-side Finish runtime message is empty"),
			Envelope.ErrorMessage.IsEmpty());
		TestFalse(TEXT("Wrong-side nested Finish fails"),
			WrongFinishResults[Index].FinishResult.bSuccess);
		TestEqual(TEXT("Wrong-side nested Finish has exact error"),
			WrongFinishResults[Index].FinishResult.ErrorCode,
			EMatchPlayFinishDeploymentErrorCode
				::RequestingSideNotCurrentLegalDeploymentSide);
		TestTrue(TEXT("Wrong-side envelope BeforeState is exact"),
			MatchPlayAuthoritativeSessionTests::AreStatesEqual(
				Envelope.BeforeState,
				BeforeState));
		TestTrue(TEXT("Wrong-side envelope AfterState is unchanged"),
			MatchPlayAuthoritativeSessionTests::AreStatesEqual(
				Envelope.AfterState,
				BeforeState));

		WrongFinishFinalStates[Index] =
			WrongFinishSessions[Index].GetStateSnapshot();
		TestTrue(TEXT("Wrong-side Finish preserves its Session"),
			MatchPlayAuthoritativeSessionTests::AreStatesEqual(
				WrongFinishFinalStates[Index],
				BeforeState));
	}
	for (int32 Index = 1; Index < 3; ++Index)
	{
		TestTrue(TEXT("Wrong-side typed Finish result is deterministic"),
			MatchPlayAuthoritativeSessionTests
				::AreAuthoritativeFinishResultsEqual(
					WrongFinishResults[0],
					WrongFinishResults[Index]));
		TestTrue(TEXT("Wrong-side runtime envelope is deterministic"),
			MatchPlayAuthoritativeSessionTests::AreEnvelopesEqual(
				WrongFinishResults[0].RuntimeEnvelope,
				WrongFinishResults[Index].RuntimeEnvelope));
		TestTrue(TEXT("Wrong-side nested Finish result is deterministic"),
			MatchPlayAuthoritativeSessionTests::AreFinishResultsEqual(
				WrongFinishResults[0].FinishResult,
				WrongFinishResults[Index].FinishResult));
		TestTrue(TEXT("Wrong-side final snapshot is deterministic"),
			MatchPlayAuthoritativeSessionTests::AreStatesEqual(
				WrongFinishFinalStates[0],
				WrongFinishFinalStates[Index]));
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

MATCH_PLAY_AUTHORITATIVE_SESSION_TEST(
	FMatchPlayAuthoritativeSessionDeployOrdinaryTypesTest,
	"12.DeployOrdinaryTypesAndSurface")

bool FMatchPlayAuthoritativeSessionDeployOrdinaryTypesTest::RunTest(
	const FString& Parameters)
{
	using FDeploySignature = FMatchPlayAuthoritativeDeployOrdinaryResult
		(FMatchPlayAuthoritativeSession::*)(
			const FMatchPlayAuthoritativeDeployOrdinaryRequest&);
	using FCarrierSignature = FMatchPlayAuthoritativeSubmitCarrierResult
		(FMatchPlayAuthoritativeSession::*)(
			const FMatchPlayAuthoritativeSubmitCarrierRequest&);
	static_assert(std::is_same_v<
		decltype(&FMatchPlayAuthoritativeSession::DeployOrdinary),
		FDeploySignature>);
	static_assert(std::is_same_v<
		decltype(&FMatchPlayAuthoritativeSession::SubmitCarrier),
		FCarrierSignature>);

	const FMatchPlayAuthoritativeDeployOrdinaryRequest DeployRequest;
	TestEqual(TEXT("Deploy request defaults side"),
		DeployRequest.RequestingSide, EInitialTurnOrderPlayer::None);
	TestTrue(TEXT("Deploy request defaults CardId"),
		DeployRequest.CardId.IsNone());
	TestTrue(TEXT("Deploy request defaults SlotId"),
		DeployRequest.SlotId.IsNone());
	const FMatchPlayAuthoritativeSubmitCarrierRequest CarrierRequest;
	TestEqual(TEXT("Carrier request defaults side"),
		CarrierRequest.RequestingSide, EInitialTurnOrderPlayer::None);
	TestTrue(TEXT("Carrier request defaults CardId"),
		CarrierRequest.CarrierCardId.IsNone());
	TestNotNull(TEXT("Deploy nested type remains exact reflected writer result"),
		FMatchPlayOrdinaryDeploymentWriterResult::StaticStruct());
	TestNotNull(TEXT("Carrier nested type remains exact reflected writer result"),
		FMatchPlayCurrentAttackCarrierSelectionWriterResult::StaticStruct());
	return true;
}

MATCH_PLAY_AUTHORITATIVE_SESSION_TEST(
	FMatchPlayAuthoritativeSessionDeployOrdinaryMatrixTest,
	"13.DeployOrdinarySuccessAndFailure")

bool FMatchPlayAuthoritativeSessionDeployOrdinaryMatrixTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayAuthoritativeSessionTests;
	FMatchPlayAuthoritativeSession Uninitialized;
	const FMatchPlayState DefaultState;
	const FMatchPlayAuthoritativeDeployOrdinaryResult Gated =
		Uninitialized.DeployOrdinary({});
	TestFalse(TEXT("Uninitialized Deploy is rejected"),
		Gated.RuntimeEnvelope.bAccepted);
	TestEqual(TEXT("Uninitialized Deploy exact runtime code"),
		Gated.RuntimeEnvelope.RuntimeFailureCode,
		EMatchPlayAuthoritativeRuntimeFailureCode::NotInitialized);
	TestEqual(TEXT("Uninitialized Deploy trusted sequence is zero"),
		Gated.RuntimeEnvelope.AttackSequence, int64{ 0 });
	TestTrue(TEXT("Uninitialized Deploy nested result is default"),
		AreDeploymentWriterResultsEqual(
			Gated.DeploymentResult,
			FMatchPlayOrdinaryDeploymentWriterResult{}));
	TestTrue(TEXT("Uninitialized Deploy preserves state"),
		AreStatesEqual(Uninitialized.GetStateSnapshot(), DefaultState));

	FMatchPlayAuthoritativeSession Session;
	Session.InitializeMatch(MakeValidInput(TEXT("DeployMatrix")));
	Session.BeginOrdinaryAttack(6);
	const FMatchPlayState Active = Session.GetStateSnapshot();
	FDeploymentChoice Choice;
	TestTrue(TEXT("Availability finds real forward deployment"),
		FindLegalDeployment(
			Active,
			EMatchPlayRelativeDeploymentZone::Forward,
			Choice));

	FMatchPlayAuthoritativeDeployOrdinaryRequest WrongSide =
		MakeDeployRequest(Choice);
	WrongSide.RequestingSide = OtherPlayer(Choice.Side);
	const FMatchPlayAuthoritativeDeployOrdinaryResult Wrong =
		Session.DeployOrdinary(WrongSide);
	TestEqual(TEXT("Wrong-side Deploy exact domain error"),
		Wrong.DeploymentResult.LegalityResult.ErrorCode,
		EMatchPlayOrdinaryDeploymentErrorCode
			::RequestingSideNotCurrentLegalDeploymentSide);
	TestNoAdoptDomainFailure(
		*this, TEXT("Wrong-side Deploy"), Wrong.RuntimeEnvelope, Active);
	TestEqual(TEXT("Wrong-side sequence is state-derived"),
		Wrong.RuntimeEnvelope.AttackSequence,
		Active.CurrentAttack.AttackSequence);

	FMatchPlayAuthoritativeDeployOrdinaryRequest InvalidSlot =
		MakeDeployRequest(Choice);
	InvalidSlot.SlotId = NAME_None;
	const FMatchPlayAuthoritativeDeployOrdinaryResult Invalid =
		Session.DeployOrdinary(InvalidSlot);
	TestEqual(TEXT("Invalid-slot Deploy exact domain error"),
		Invalid.DeploymentResult.LegalityResult.ErrorCode,
		EMatchPlayOrdinaryDeploymentErrorCode::InvalidSlotId);
	TestNoAdoptDomainFailure(
		*this, TEXT("Invalid-slot Deploy"), Invalid.RuntimeEnvelope, Active);

	const FMatchPlayAuthoritativeDeployOrdinaryResult Valid =
		Session.DeployOrdinary(MakeDeployRequest(Choice));
	TestTrue(TEXT("Valid Deploy reaches writer success"),
		Valid.DeploymentResult.bSuccess);
	TestEqual(TEXT("Valid Deploy command kind"),
		Valid.RuntimeEnvelope.CommandKind,
		EMatchPlayAuthoritativeCommandKind::DeployOrdinary);
	TestAdoptedEnvelope(
		*this,
		TEXT("Valid Deploy"),
		Valid.RuntimeEnvelope,
		Active,
		Valid.DeploymentResult.AfterState);
	TestTrue(TEXT("Valid Deploy request sequence is state-derived"),
		Valid.DeploymentResult.Request.AttackSequence
			== Active.CurrentAttack.AttackSequence);
	TestEqual(TEXT("Real writer records resolved zone"),
		Valid.DeploymentResult.LegalityResult.ResolvedRelativeZone,
		Choice.ResolvedRelativeZone);

	const FMatchPlayState AfterValid = Session.GetStateSnapshot();
	const FMatchPlayAuthoritativeDeployOrdinaryResult Replay =
		Session.DeployOrdinary(MakeDeployRequest(Choice));
	TestFalse(TEXT("Same placement replay reaches domain failure"),
		Replay.DeploymentResult.bSuccess);
	TestNoAdoptDomainFailure(
		*this, TEXT("Deploy replay"), Replay.RuntimeEnvelope, AfterValid);
	TestTrue(TEXT("Deploy replay preserves session"),
		AreStatesEqual(Session.GetStateSnapshot(), AfterValid));
	return true;
}

MATCH_PLAY_AUTHORITATIVE_SESSION_TEST(
	FMatchPlayAuthoritativeSessionDeployDeterminismTest,
	"14.DeployOrdinaryReplayIsolationAndDeterminism")

bool FMatchPlayAuthoritativeSessionDeployDeterminismTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayAuthoritativeSessionTests;
	FMatchPlayAuthoritativeSession Sessions[3];
	FMatchPlayAuthoritativeDeployOrdinaryResult Wrong[3];
	FMatchPlayAuthoritativeDeployOrdinaryResult Valid[3];
	FMatchPlayAuthoritativeDeployOrdinaryResult Replay[3];
	FMatchPlayState Final[3];
	for (int32 Index = 0; Index < 3; ++Index)
	{
		Sessions[Index].InitializeMatch(MakeValidInput(TEXT("DeployDet")));
		Sessions[Index].BeginOrdinaryAttack(6);
		FDeploymentChoice Choice;
		TestTrue(TEXT("Determinism fixture finds legal deployment"),
			FindLegalDeployment(
				Sessions[Index].GetStateSnapshot(),
				EMatchPlayRelativeDeploymentZone::Forward,
				Choice));
		FMatchPlayAuthoritativeDeployOrdinaryRequest WrongRequest =
			MakeDeployRequest(Choice);
		WrongRequest.RequestingSide = OtherPlayer(Choice.Side);
		Wrong[Index] = Sessions[Index].DeployOrdinary(WrongRequest);
		Valid[Index] = Sessions[Index].DeployOrdinary(MakeDeployRequest(Choice));
		Replay[Index] = Sessions[Index].DeployOrdinary(MakeDeployRequest(Choice));
		Final[Index] = Sessions[Index].GetStateSnapshot();
	}
	for (int32 Index = 1; Index < 3; ++Index)
	{
		TestTrue(TEXT("Wrong-side Deploy typed result deterministic"),
			AreAuthoritativeDeployOrdinaryResultsEqual(Wrong[0], Wrong[Index]));
		TestTrue(TEXT("Valid Deploy typed result deterministic"),
			AreAuthoritativeDeployOrdinaryResultsEqual(Valid[0], Valid[Index]));
		TestTrue(TEXT("Deploy replay typed result deterministic"),
			AreAuthoritativeDeployOrdinaryResultsEqual(Replay[0], Replay[Index]));
		TestTrue(TEXT("Deploy final snapshot deterministic"),
			AreStatesEqual(Final[0], Final[Index]));
	}

	FMatchPlayAuthoritativeSession SessionA;
	FMatchPlayAuthoritativeSession SessionB;
	SessionA.InitializeMatch(MakeValidInput(TEXT("DeployIsolationA")));
	SessionB.InitializeMatch(MakeValidInput(TEXT("DeployIsolationB")));
	SessionA.BeginOrdinaryAttack(6);
	SessionB.BeginOrdinaryAttack(6);
	const FMatchPlayState BBeforeA = SessionB.GetStateSnapshot();
	FDeploymentChoice AChoice;
	FindLegalDeployment(
		SessionA.GetStateSnapshot(),
		EMatchPlayRelativeDeploymentZone::Forward,
		AChoice);
	SessionA.DeployOrdinary(MakeDeployRequest(AChoice));
	TestTrue(TEXT("A Deploy cannot change B"),
		AreStatesEqual(SessionB.GetStateSnapshot(), BBeforeA));
	const FMatchPlayState ABeforeB = SessionA.GetStateSnapshot();
	FDeploymentChoice BChoice;
	FindLegalDeployment(
		SessionB.GetStateSnapshot(),
		EMatchPlayRelativeDeploymentZone::Forward,
		BChoice);
	SessionB.DeployOrdinary(MakeDeployRequest(BChoice));
	TestTrue(TEXT("B Deploy cannot change A"),
		AreStatesEqual(SessionA.GetStateSnapshot(), ABeforeB));
	return true;
}

MATCH_PLAY_AUTHORITATIVE_SESSION_TEST(
	FMatchPlayAuthoritativeSessionCarrierFailureMatrixTest,
	"15.SubmitCarrierStageAndLegalityFailures")

bool FMatchPlayAuthoritativeSessionCarrierFailureMatrixTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayAuthoritativeSessionTests;
	FMatchPlayAuthoritativeSession Uninitialized;
	const FMatchPlayAuthoritativeSubmitCarrierResult Gated =
		Uninitialized.SubmitCarrier({});
	TestFalse(TEXT("Uninitialized Carrier is rejected"),
		Gated.RuntimeEnvelope.bAccepted);
	TestEqual(TEXT("Uninitialized Carrier exact runtime code"),
		Gated.RuntimeEnvelope.RuntimeFailureCode,
		EMatchPlayAuthoritativeRuntimeFailureCode::NotInitialized);
	TestTrue(TEXT("Uninitialized Carrier nested result is default"),
		AreCarrierWriterResultsEqual(
			Gated.CarrierResult,
			FMatchPlayCurrentAttackCarrierSelectionWriterResult{}));

	FMatchPlayAuthoritativeSession EarlySession;
	EarlySession.InitializeMatch(MakeValidInput(TEXT("CarrierEarly")));
	EarlySession.BeginOrdinaryAttack(6);
	const FMatchPlayState EarlyState = EarlySession.GetStateSnapshot();
	FMatchPlayAuthoritativeSubmitCarrierRequest EarlyRequest;
	EarlyRequest.RequestingSide = EarlyState.RuntimeState.CurrentAttackingPlayer;
	EarlyRequest.CarrierCardId = TEXT("Not.Deployed");
	const FMatchPlayAuthoritativeSubmitCarrierResult Early =
		EarlySession.SubmitCarrier(EarlyRequest);
	TestEqual(TEXT("Carrier before Resolution exact error"),
		Early.CarrierResult.LegalityResult.ErrorCode,
		EMatchPlayCurrentAttackCarrierSelectionErrorCode
			::CurrentAttackNotInResolution);
	TestNoAdoptDomainFailure(
		*this, TEXT("Early Carrier"), Early.RuntimeEnvelope, EarlyState);

	FMatchPlayAuthoritativeSession Session;
	FReachabilityTrace Trace;
	TestTrue(TEXT("Carrier failure fixture reaches AwaitingCarrier"),
		BuildToAwaitingCarrier(Session, TEXT("CarrierFailures"), Trace));
	const FMatchPlayState AwaitingCarrier = Session.GetStateSnapshot();
	FMatchPlayAuthoritativeSubmitCarrierRequest WrongSide =
		MakeCarrierRequest(Trace);
	WrongSide.RequestingSide = OtherPlayer(Trace.AttackingSide);
	const FMatchPlayAuthoritativeSubmitCarrierResult Wrong =
		Session.SubmitCarrier(WrongSide);
	TestEqual(TEXT("Wrong-side Carrier exact error"),
		Wrong.CarrierResult.LegalityResult.ErrorCode,
		EMatchPlayCurrentAttackCarrierSelectionErrorCode
			::RequestingSideIsNotCurrentAttacker);
	TestNoAdoptDomainFailure(
		*this, TEXT("Wrong-side Carrier"), Wrong.RuntimeEnvelope, AwaitingCarrier);

	FMatchPlayAuthoritativeSubmitCarrierRequest Invalid =
		MakeCarrierRequest(Trace);
	Invalid.CarrierCardId = TEXT("Not.Deployed.Carrier");
	const FMatchPlayAuthoritativeSubmitCarrierResult Unavailable =
		Session.SubmitCarrier(Invalid);
	TestEqual(TEXT("Unavailable Carrier exact error"),
		Unavailable.CarrierResult.LegalityResult.ErrorCode,
		EMatchPlayCurrentAttackCarrierSelectionErrorCode::CarrierNotDeployed);
	TestNoAdoptDomainFailure(
		*this,
		TEXT("Unavailable Carrier"),
		Unavailable.RuntimeEnvelope,
		AwaitingCarrier);
	return true;
}

MATCH_PLAY_AUTHORITATIVE_SESSION_TEST(
	FMatchPlayAuthoritativeSessionEndToEndReachabilityTest,
	"16.EndToEndDeploymentToCarrierReachability")

bool FMatchPlayAuthoritativeSessionEndToEndReachabilityTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayAuthoritativeSessionTests;
	FMatchPlayAuthoritativeSession Session;
	FReachabilityTrace Trace;
	TestTrue(TEXT("Public Session chain reaches AwaitingCarrier"),
		BuildToAwaitingCarrier(Session, TEXT("EndToEnd"), Trace));
	TestEqual(TEXT("First legal side is current attacker"),
		Trace.FirstChoice.Side, Trace.AttackingSide);
	TestEqual(TEXT("Second legal side is defender"),
		Trace.SecondChoice.Side, OtherPlayer(Trace.AttackingSide));
	TestEqual(TEXT("Attacker uses real Forward slot"),
		Trace.FirstChoice.ResolvedRelativeZone,
		EMatchPlayRelativeDeploymentZone::Forward);
	TestEqual(TEXT("Defender uses real Midfield slot"),
		Trace.SecondChoice.ResolvedRelativeZone,
		EMatchPlayRelativeDeploymentZone::Midfield);
	TestEqual(TEXT("Carrier candidate is attacking deployment"),
		Trace.CarrierCardId, Trace.FirstChoice.CardId);

	TestTrue(TEXT("Begin to Deploy1 continuity"),
		AreStatesEqual(
			Trace.FirstDeploy.RuntimeEnvelope.BeforeState,
			Trace.Begin.RuntimeEnvelope.AfterState));
	TestTrue(TEXT("Deploy1 to Deploy2 continuity"),
		AreStatesEqual(
			Trace.SecondDeploy.RuntimeEnvelope.BeforeState,
			Trace.FirstDeploy.RuntimeEnvelope.AfterState));
	TestTrue(TEXT("Deploy2 to Finish1 continuity"),
		AreStatesEqual(
			Trace.FirstFinish.RuntimeEnvelope.BeforeState,
			Trace.SecondDeploy.RuntimeEnvelope.AfterState));
	TestTrue(TEXT("Finish1 to Finish2 continuity"),
		AreStatesEqual(
			Trace.SecondFinish.RuntimeEnvelope.BeforeState,
			Trace.FirstFinish.RuntimeEnvelope.AfterState));

	const FMatchPlayState BeforeCarrier = Session.GetStateSnapshot();
	const FMatchPlayAuthoritativeSubmitCarrierResult Carrier =
		Session.SubmitCarrier(MakeCarrierRequest(Trace));
	TestTrue(TEXT("Real Carrier writer succeeds"),
		Carrier.CarrierResult.bSuccess);
	TestEqual(TEXT("Carrier command kind"),
		Carrier.RuntimeEnvelope.CommandKind,
		EMatchPlayAuthoritativeCommandKind::SubmitCarrier);
	TestAdoptedEnvelope(
		*this,
		TEXT("Carrier"),
		Carrier.RuntimeEnvelope,
		BeforeCarrier,
		Carrier.CarrierResult.AfterState);
	TestTrue(TEXT("Finish2 to Carrier continuity"),
		AreStatesEqual(
			Carrier.RuntimeEnvelope.BeforeState,
			Trace.SecondFinish.RuntimeEnvelope.AfterState));

	const FMatchPlayState Final = Session.GetStateSnapshot();
	TestEqual(TEXT("Final phase is Resolution"),
		Final.CurrentAttack.Phase,
		EMatchPlayCurrentAttackPhase::Resolution);
	TestEqual(TEXT("Final stage is AwaitingMarker"),
		Final.CurrentAttack.SelectionStage,
		EMatchPlayCurrentAttackSelectionStage::AwaitingMarker);
	TestEqual(TEXT("Carrier is recorded"),
		Final.CurrentAttack.ActionPreparation.CarrierCardId,
		Trace.CarrierCardId);
	TestTrue(TEXT("Marker remains absent"),
		Final.CurrentAttack.ActionPreparation.MarkerCardId.IsNone());
	TestFalse(TEXT("Resolution Session remains absent"),
		Final.CurrentAttack.bHasResolutionSession);
	TestFalse(TEXT("Actual Branch remains absent"),
		Final.CurrentAttack.ResolutionSession.bHasActualBranch);
	TestTrue(TEXT("Initial Route rolls remain empty"),
		Final.CurrentAttack.ResolutionSession.InitialRouteRollRecords.IsEmpty());
	return true;
}

MATCH_PLAY_AUTHORITATIVE_SESSION_TEST(
	FMatchPlayAuthoritativeSessionCarrierDeterminismTest,
	"17.SubmitCarrierReplayIsolationAndDeterminism")

bool FMatchPlayAuthoritativeSessionCarrierDeterminismTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayAuthoritativeSessionTests;
	FMatchPlayAuthoritativeSession Sessions[3];
	FMatchPlayAuthoritativeSubmitCarrierResult Wrong[3];
	FMatchPlayAuthoritativeSubmitCarrierResult Success[3];
	FMatchPlayAuthoritativeSubmitCarrierResult Replay[3];
	FMatchPlayState Final[3];
	for (int32 Index = 0; Index < 3; ++Index)
	{
		FReachabilityTrace Trace;
		TestTrue(TEXT("Carrier determinism fixture succeeds"),
			BuildToAwaitingCarrier(Sessions[Index], TEXT("CarrierDet"), Trace));
		FMatchPlayAuthoritativeSubmitCarrierRequest WrongRequest =
			MakeCarrierRequest(Trace);
		WrongRequest.RequestingSide = OtherPlayer(Trace.AttackingSide);
		Wrong[Index] = Sessions[Index].SubmitCarrier(WrongRequest);
		Success[Index] = Sessions[Index].SubmitCarrier(MakeCarrierRequest(Trace));
		Replay[Index] = Sessions[Index].SubmitCarrier(MakeCarrierRequest(Trace));
		Final[Index] = Sessions[Index].GetStateSnapshot();
		TestEqual(TEXT("Replay fails at actual selection stage"),
			Replay[Index].CarrierResult.LegalityResult.ErrorCode,
			EMatchPlayCurrentAttackCarrierSelectionErrorCode::WrongSelectionStage);
		TestFalse(TEXT("Replay does not adopt"),
			Replay[Index].RuntimeEnvelope.bStateAdvanced);
	}
	for (int32 Index = 1; Index < 3; ++Index)
	{
		TestTrue(TEXT("Wrong-side Carrier deterministic"),
			AreAuthoritativeSubmitCarrierResultsEqual(Wrong[0], Wrong[Index]));
		TestTrue(TEXT("Successful Carrier deterministic"),
			AreAuthoritativeSubmitCarrierResultsEqual(Success[0], Success[Index]));
		TestTrue(TEXT("Carrier replay deterministic"),
			AreAuthoritativeSubmitCarrierResultsEqual(Replay[0], Replay[Index]));
		TestTrue(TEXT("Carrier final snapshot deterministic"),
			AreStatesEqual(Final[0], Final[Index]));
	}

	FMatchPlayAuthoritativeSession SessionA;
	FMatchPlayAuthoritativeSession SessionB;
	FReachabilityTrace TraceA;
	FReachabilityTrace TraceB;
	BuildToAwaitingCarrier(SessionA, TEXT("CarrierIsolationA"), TraceA);
	BuildToAwaitingCarrier(SessionB, TEXT("CarrierIsolationB"), TraceB);
	const FMatchPlayState BBeforeA = SessionB.GetStateSnapshot();
	SessionA.SubmitCarrier(MakeCarrierRequest(TraceA));
	TestTrue(TEXT("A Carrier cannot change B"),
		AreStatesEqual(SessionB.GetStateSnapshot(), BBeforeA));
	const FMatchPlayState ABeforeB = SessionA.GetStateSnapshot();
	SessionB.SubmitCarrier(MakeCarrierRequest(TraceB));
	TestTrue(TEXT("B Carrier cannot change A"),
		AreStatesEqual(SessionA.GetStateSnapshot(), ABeforeB));
	return true;
}

MATCH_PLAY_AUTHORITATIVE_SESSION_TEST(
	FMatchPlayAuthoritativeSessionNewComparatorCoverageTest,
	"18.NewTypedResultComparatorCoverage")

bool FMatchPlayAuthoritativeSessionNewComparatorCoverageTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayAuthoritativeSessionTests;
	FMatchPlayAuthoritativeSession Session;
	Session.InitializeMatch(MakeValidInput(TEXT("NewComparators")));
	Session.BeginOrdinaryAttack(6);
	FDeploymentChoice Choice;
	TestTrue(TEXT("Comparator fixture finds real deployment"),
		FindLegalDeployment(
			Session.GetStateSnapshot(),
			EMatchPlayRelativeDeploymentZone::Forward,
			Choice));
	const FMatchPlayAuthoritativeDeployOrdinaryResult Deploy =
		Session.DeployOrdinary(MakeDeployRequest(Choice));
	TestTrue(TEXT("Deploy comparator accepts equal values"),
		AreAuthoritativeDeployOrdinaryResultsEqual(Deploy, Deploy));
	TestDeployMutationCoverage(*this, Deploy);

	FMatchPlayAuthoritativeSession CarrierSession;
	FReachabilityTrace Trace;
	TestTrue(TEXT("Carrier comparator fixture reaches stage"),
		BuildToAwaitingCarrier(
			CarrierSession,
			TEXT("CarrierComparator"),
			Trace));
	const FMatchPlayAuthoritativeSubmitCarrierResult Carrier =
		CarrierSession.SubmitCarrier(MakeCarrierRequest(Trace));
	TestTrue(TEXT("Carrier comparator accepts equal values"),
		AreAuthoritativeSubmitCarrierResultsEqual(Carrier, Carrier));
	TestCarrierMutationCoverage(*this, Carrier);
	return true;
}

MATCH_PLAY_AUTHORITATIVE_SESSION_TEST(
	FMatchPlayAuthoritativeSessionResolutionFoundationBoundaryTest,
	"19.ResolutionFoundationAProductionBoundary")

bool FMatchPlayAuthoritativeSessionResolutionFoundationBoundaryTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayAuthoritativeSessionTests;
	FString Header;
	FString Implementation;
	FString Types;
	TestTrue(TEXT("Boundary header loads"), LoadProductionSource(
		TEXT("Source/FMCodex/MatchPlayRuntime/MatchPlayAuthoritativeSession.h"),
		Header));
	TestTrue(TEXT("Boundary implementation loads"), LoadProductionSource(
		TEXT("Source/FMCodex/MatchPlayRuntime/MatchPlayAuthoritativeSession.cpp"),
		Implementation));
	TestTrue(TEXT("Boundary types load"), LoadProductionSource(
		TEXT("Source/FMCodex/MatchPlayRuntime/MatchPlayAuthoritativeSessionTypes.h"),
		Types));
	const FString Production = Header + Implementation + Types;
	TestEqual(TEXT("Ordinary Writer has one Session call"),
		CountOccurrences(
			Implementation,
			TEXT("FMatchPlayOrdinaryDeploymentWriter::Deploy(")),
		1);
	TestEqual(TEXT("Carrier Writer has one Session call"),
		CountOccurrences(
			Implementation,
			TEXT("FMatchPlayCurrentAttackCarrierSelectionWriter::Select(")),
		1);
	TestEqual(TEXT("Session keeps one State replacement"),
		CountOccurrences(
			Implementation,
			TEXT("AuthoritativeState = Adoption.AdoptedAfterState;")),
		1);
	TestFalse(TEXT("SubmitAction remains absent"),
		Production.Contains(TEXT("SubmitAction")));
	TestFalse(TEXT("DeployGoalkeeper remains absent"),
		Production.Contains(TEXT("DeployGoalkeeper")));
	for (const TCHAR* Forbidden : {
		TEXT("SubmitMarker"),
		TEXT("SubmitSkill"),
		TEXT("SubmitRunner"),
		TEXT("SubmitHelper"),
		TEXT("SubmitBranchIntent"),
		TEXT("ResolveInitialRouteOrchestrator"),
		TEXT("IMatchPlayInitialRouteRollProvider"),
		TEXT("RollD6"),
		TEXT("UObject"),
		TEXT("RPC"),
		TEXT("Tick"),
		TEXT("SetState"),
		TEXT("RestoreState") })
	{
		TestFalse(*FString::Printf(TEXT("Forbidden production surface absent: %s"), Forbidden),
			Production.Contains(Forbidden));
	}
	TestFalse(TEXT("Session never appends Deployment directly"),
		Implementation.Contains(TEXT("DeploymentPlacements.Add")));
	TestFalse(TEXT("Session never writes authoritative Carrier directly"),
		Implementation.Contains(TEXT("AuthoritativeState.CurrentAttack.ActionPreparation.CarrierCardId")));
	return true;
}

#undef MATCH_PLAY_AUTHORITATIVE_SESSION_TEST

#endif
