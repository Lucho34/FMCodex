#include "MatchPlayAuthoritativeSession.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "../CoreRules/MatchPlayCurrentAttackCarrierSelectionAvailability.h"
#include "../CoreRules/MatchPlayCurrentAttackMarkerSelectionAvailability.h"
#include "../CoreRules/MatchPlayCurrentAttackSkillSelectionAvailability.h"
#include "../CoreRules/MatchPlayOrdinaryDeploymentAvailability.h"

#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "UObject/UnrealType.h"

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

	FMatchPlayOpeningInitializeInput MakeFoundationBInput(
		const FString& Prefix,
		const TArray<FName>& CarrierSkillIds = {})
	{
		FMatchPlayOpeningInitializeInput Input = MakeValidInput(Prefix);
		for (FPlayerCardData& Card : Input.OpeningInput.PlayerADeck)
		{
			if (!Card.bIsGoalkeeper)
			{
				Card.PositionTypes = {
					EPlayerPositionType::Attack,
					EPlayerPositionType::Midfield,
					EPlayerPositionType::Defense
				};
				Card.AttackSkillIds = CarrierSkillIds;
			}
		}
		for (FPlayerCardData& Card : Input.OpeningInput.PlayerBDeck)
		{
			if (!Card.bIsGoalkeeper)
			{
				Card.PositionTypes = {
					EPlayerPositionType::Attack,
					EPlayerPositionType::Midfield,
					EPlayerPositionType::Defense
				};
			}
		}

		FMatchPlayDeploymentSlotDefinition NearBOne;
		NearBOne.SlotId = FName(*FString::Printf(
			TEXT("%s_NearB1"),
			*Prefix));
		NearBOne.NeutralSide = EMatchPlayNeutralSlotSide::NearPlayerB;
		FMatchPlayDeploymentSlotDefinition NearBTwo;
		NearBTwo.SlotId = FName(*FString::Printf(
			TEXT("%s_NearB2"),
			*Prefix));
		NearBTwo.NeutralSide = EMatchPlayNeutralSlotSide::NearPlayerB;
		FMatchPlayDeploymentSlotDefinition NearAOne;
		NearAOne.SlotId = FName(*FString::Printf(
			TEXT("%s_NearA1"),
			*Prefix));
		NearAOne.NeutralSide = EMatchPlayNeutralSlotSide::NearPlayerA;
		FMatchPlayDeploymentSlotDefinition NearATwo;
		NearATwo.SlotId = FName(*FString::Printf(
			TEXT("%s_NearA2"),
			*Prefix));
		NearATwo.NeutralSide = EMatchPlayNeutralSlotSide::NearPlayerA;
		Input.DeploymentSlotCatalog.Slots = {
			NearBOne,
			NearBTwo,
			NearAOne,
			NearATwo
		};
		return Input;
	}

	FSkillRuleSnapshot MakeSkillRule(
		const FName SkillId,
		const ESkillRuleType SkillType)
	{
		FSkillRuleSnapshot Rule;
		Rule.SkillId = SkillId;
		Rule.SkillType = SkillType;
		Rule.MinTriggerActionPoint = 2;
		Rule.MaxTriggerActionPoint = 8;
		return Rule;
	}

	FSkillRuleSnapshotSet MakeSkillRuleSet(
		const FName SkillId,
		const ESkillRuleType SkillType)
	{
		FSkillRuleSnapshotSet Rules;
		Rules.SkillRules = { MakeSkillRule(SkillId, SkillType) };
		return Rules;
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

	void TestAcceptedDomainFailureNoAdopt(
		FAutomationTestBase& Test,
		const FString& Context,
		const FMatchPlayAuthoritativeRuntimeEnvelope& Envelope,
		const FMatchPlayState& BeforeState,
		const FMatchPlayState& FinalState)
	{
		Test.TestTrue(*FString::Printf(TEXT("%s accepted"), *Context),
			Envelope.bAccepted);
		Test.TestFalse(*FString::Printf(TEXT("%s domain failure"), *Context),
			Envelope.bDomainSuccess);
		Test.TestFalse(*FString::Printf(TEXT("%s did not advance"), *Context),
			Envelope.bStateAdvanced);
		Test.TestEqual(*FString::Printf(TEXT("%s does not adopt"), *Context),
			Envelope.StateDisposition,
			EMatchPlayAuthoritativeStateDisposition::DoNotAdopt);
		Test.TestFalse(*FString::Printf(TEXT("%s is not runtime fault"), *Context),
			Envelope.bRuntimeFault);
		Test.TestEqual(*FString::Printf(TEXT("%s failure disposition"), *Context),
			Envelope.FailureDisposition,
			EMatchPlayAuthoritativeFailureDisposition::None);
		Test.TestEqual(*FString::Printf(TEXT("%s runtime code"), *Context),
			Envelope.RuntimeFailureCode,
			EMatchPlayAuthoritativeRuntimeFailureCode::None);
		Test.TestTrue(*FString::Printf(TEXT("%s runtime message empty"), *Context),
			Envelope.ErrorMessage.IsEmpty());
		Test.TestTrue(*FString::Printf(TEXT("%s BeforeState exact"), *Context),
			AreStatesEqual(BeforeState, Envelope.BeforeState));
		Test.TestTrue(*FString::Printf(TEXT("%s AfterState unchanged"), *Context),
			AreStatesEqual(BeforeState, Envelope.AfterState));
		Test.TestTrue(*FString::Printf(TEXT("%s Session state unchanged"), *Context),
			AreStatesEqual(BeforeState, FinalState));
	}

	void TestAdoptedSuccessEnvelope(
		FAutomationTestBase& Test,
		const FString& Context,
		const FMatchPlayAuthoritativeRuntimeEnvelope& Envelope,
		const FMatchPlayState& BeforeState,
		const FMatchPlayState& NestedAfterState,
		const FMatchPlayState& FinalState)
	{
		Test.TestTrue(*FString::Printf(TEXT("%s accepted"), *Context),
			Envelope.bAccepted);
		Test.TestTrue(*FString::Printf(TEXT("%s domain success"), *Context),
			Envelope.bDomainSuccess);
		Test.TestTrue(*FString::Printf(TEXT("%s advanced"), *Context),
			Envelope.bStateAdvanced);
		Test.TestEqual(*FString::Printf(TEXT("%s adopts"), *Context),
			Envelope.StateDisposition,
			EMatchPlayAuthoritativeStateDisposition::Adopt);
		Test.TestFalse(*FString::Printf(TEXT("%s is not runtime fault"), *Context),
			Envelope.bRuntimeFault);
		Test.TestEqual(*FString::Printf(TEXT("%s failure disposition"), *Context),
			Envelope.FailureDisposition,
			EMatchPlayAuthoritativeFailureDisposition::None);
		Test.TestEqual(*FString::Printf(TEXT("%s runtime code"), *Context),
			Envelope.RuntimeFailureCode,
			EMatchPlayAuthoritativeRuntimeFailureCode::None);
		Test.TestTrue(*FString::Printf(TEXT("%s runtime message empty"), *Context),
			Envelope.ErrorMessage.IsEmpty());
		Test.TestTrue(*FString::Printf(TEXT("%s BeforeState exact"), *Context),
			AreStatesEqual(BeforeState, Envelope.BeforeState));
		Test.TestTrue(*FString::Printf(TEXT("%s envelope adopts nested AfterState"), *Context),
			AreStatesEqual(NestedAfterState, Envelope.AfterState));
		Test.TestTrue(*FString::Printf(TEXT("%s Session adopts nested AfterState"), *Context),
			AreStatesEqual(NestedAfterState, FinalState));
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

	template <typename TStruct>
	bool AreReflectedValuesEqual(const TStruct& Left, const TStruct& Right)
	{
		return TStruct::StaticStruct()->CompareScriptStruct(
			&Left,
			&Right,
			0);
	}

	bool ArePlayerCardValidationResultsEqual(
		const FPlayerCardRuleSnapshotValidationResult& Left,
		const FPlayerCardRuleSnapshotValidationResult& Right)
	{
		return Left.bSuccess == Right.bSuccess
			&& Left.bIsValid == Right.bIsValid
			&& Left.ErrorCode == Right.ErrorCode
			&& Left.ErrorMessage == Right.ErrorMessage
			&& Left.InvalidCardId == Right.InvalidCardId
			&& Left.DuplicateCardIds == Right.DuplicateCardIds;
	}

	bool ArePlayerCardQueryResultsEqual(
		const FPlayerCardRuleSnapshotQueryResult& Left,
		const FPlayerCardRuleSnapshotQueryResult& Right)
	{
		return Left.bSuccess == Right.bSuccess
			&& Left.bFound == Right.bFound
			&& Left.ErrorCode == Right.ErrorCode
			&& Left.ErrorMessage == Right.ErrorMessage
			&& Left.CardId == Right.CardId
			&& AreReflectedValuesEqual(Left.Snapshot, Right.Snapshot)
			&& ArePlayerCardValidationResultsEqual(
				Left.ValidationResult,
				Right.ValidationResult);
	}

	bool AreCardSnapshotQueryResultsEqual(
		const FMatchPlayCardSnapshotAuthorityQueryResult& Left,
		const FMatchPlayCardSnapshotAuthorityQueryResult& Right)
	{
		return Left.bSuccess == Right.bSuccess
			&& Left.ErrorCode == Right.ErrorCode
			&& Left.PlayerSide == Right.PlayerSide
			&& Left.CardId == Right.CardId
			&& AreReflectedValuesEqual(Left.Snapshot, Right.Snapshot)
			&& ArePlayerCardQueryResultsEqual(
				Left.UnderlyingQueryResult,
				Right.UnderlyingQueryResult)
			&& Left.ErrorMessage == Right.ErrorMessage;
	}

	bool AreSkillRuleValidationResultsEqual(
		const FSkillRuleSnapshotValidationResult& Left,
		const FSkillRuleSnapshotValidationResult& Right)
	{
		return Left.bSuccess == Right.bSuccess
			&& Left.bIsValid == Right.bIsValid
			&& Left.ErrorCode == Right.ErrorCode
			&& Left.ErrorMessage == Right.ErrorMessage
			&& Left.InvalidSkillId == Right.InvalidSkillId
			&& Left.InvalidField == Right.InvalidField;
	}

	bool AreSkillRulesEqual(
		const FSkillRuleSnapshot& Left,
		const FSkillRuleSnapshot& Right)
	{
		return Left.SkillId == Right.SkillId
			&& Left.SkillType == Right.SkillType
			&& Left.MinTriggerActionPoint == Right.MinTriggerActionPoint
			&& Left.MaxTriggerActionPoint == Right.MaxTriggerActionPoint;
	}

	bool AreSkillRuleQueryResultsEqual(
		const FSkillRuleSnapshotQueryResult& Left,
		const FSkillRuleSnapshotQueryResult& Right)
	{
		return Left.bSuccess == Right.bSuccess
			&& Left.bFound == Right.bFound
			&& Left.ErrorCode == Right.ErrorCode
			&& Left.ErrorMessage == Right.ErrorMessage
			&& Left.InvalidSkillId == Right.InvalidSkillId
			&& Left.InvalidField == Right.InvalidField
			&& AreSkillRulesEqual(Left.Snapshot, Right.Snapshot)
			&& AreSkillRuleValidationResultsEqual(
				Left.ValidationResult,
				Right.ValidationResult);
	}

	bool AreSkillGlobalContextResultsEqual(
		const FMatchPlayCurrentAttackSkillSelectionGlobalContextResult& Left,
		const FMatchPlayCurrentAttackSkillSelectionGlobalContextResult& Right)
	{
		return Left.bSuccess == Right.bSuccess
			&& Left.RequestedAttackSequence == Right.RequestedAttackSequence
			&& Left.RequestingSide == Right.RequestingSide
			&& Left.ErrorCode == Right.ErrorCode
			&& Left.AuthoritativeAttackSequence
				== Right.AuthoritativeAttackSequence
			&& Left.CurrentAttackingPlayer == Right.CurrentAttackingPlayer
			&& Left.CurrentDefendingPlayer == Right.CurrentDefendingPlayer
			&& AreSelectionValidationResultsEqual(
				Left.SelectionStateValidationResult,
				Right.SelectionStateValidationResult)
			&& Left.FrozenCarrierCardId == Right.FrozenCarrierCardId
			&& Left.FrozenMarkerCardId == Right.FrozenMarkerCardId
			&& Left.MatchingFrozenCarrierPlacementCount
				== Right.MatchingFrozenCarrierPlacementCount
			&& Left.MatchingFrozenMarkerPlacementCount
				== Right.MatchingFrozenMarkerPlacementCount
			&& AreReflectedValuesEqual(
				Left.FrozenCarrierPlacement,
				Right.FrozenCarrierPlacement)
			&& AreReflectedValuesEqual(
				Left.FrozenMarkerPlacement,
				Right.FrozenMarkerPlacement)
			&& AreCardSnapshotQueryResultsEqual(
				Left.CarrierSnapshotQueryResult,
				Right.CarrierSnapshotQueryResult)
			&& AreReflectedValuesEqual(
				Left.ResolvedCarrierSnapshot,
				Right.ResolvedCarrierSnapshot)
			&& AreSkillRuleValidationResultsEqual(
				Left.SkillRuleSetValidationResult,
				Right.SkillRuleSetValidationResult)
			&& Left.ValidatedActionPoint == Right.ValidatedActionPoint
			&& Left.ErrorMessage == Right.ErrorMessage;
	}

	bool AreMarkerLegalityResultsEqual(
		const FMatchPlayCurrentAttackMarkerSelectionLegalityResult& Left,
		const FMatchPlayCurrentAttackMarkerSelectionLegalityResult& Right)
	{
		return AreReflectedValuesEqual(Left, Right)
			&& AreCardSnapshotQueryResultsEqual(
				Left.MarkerSnapshotQueryResult,
				Right.MarkerSnapshotQueryResult);
	}

	bool AreMarkerAvailabilityResultsEqual(
		const FMatchPlayCurrentAttackMarkerSelectionAvailabilityResult& Left,
		const FMatchPlayCurrentAttackMarkerSelectionAvailabilityResult& Right)
	{
		if (!AreReflectedValuesEqual(Left, Right)
			|| Left.Candidates.Num() != Right.Candidates.Num())
		{
			return false;
		}
		for (int32 Index = 0; Index < Left.Candidates.Num(); ++Index)
		{
			if (!AreMarkerLegalityResultsEqual(
				Left.Candidates[Index].LegalityResult,
				Right.Candidates[Index].LegalityResult))
			{
				return false;
			}
		}
		return AreMarkerLegalityResultsEqual(
			Left.GlobalBlockingLegalityResult,
			Right.GlobalBlockingLegalityResult);
	}

	bool AreMarkerWriterResultsEqual(
		const FMatchPlayCurrentAttackMarkerSelectionWriterResult& Left,
		const FMatchPlayCurrentAttackMarkerSelectionWriterResult& Right)
	{
		return AreReflectedValuesEqual(Left, Right)
			&& AreMarkerLegalityResultsEqual(
				Left.LegalityResult,
				Right.LegalityResult);
	}

	bool AreCompletionResultsEqual(
		const FMatchPlayCurrentAttackCompletionResult& Left,
		const FMatchPlayCurrentAttackCompletionResult& Right)
	{
		if (!AreReflectedValuesEqual(Left, Right)
			|| Left.DeploymentSnapshotQueryResults.Num()
				!= Right.DeploymentSnapshotQueryResults.Num())
		{
			return false;
		}
		for (int32 Index = 0;
			Index < Left.DeploymentSnapshotQueryResults.Num();
			++Index)
		{
			if (!AreCardSnapshotQueryResultsEqual(
				Left.DeploymentSnapshotQueryResults[Index],
				Right.DeploymentSnapshotQueryResults[Index]))
			{
				return false;
			}
		}
		return true;
	}

	bool AreSkillLegalityResultsEqual(
		const FMatchPlayCurrentAttackSkillSelectionLegalityResult& Left,
		const FMatchPlayCurrentAttackSkillSelectionLegalityResult& Right)
	{
		return AreReflectedValuesEqual(Left, Right)
			&& AreSkillGlobalContextResultsEqual(
				Left.GlobalContextResult,
				Right.GlobalContextResult)
			&& AreCardSnapshotQueryResultsEqual(
				Left.CarrierSnapshotQueryResult,
				Right.CarrierSnapshotQueryResult)
			&& AreSkillRuleQueryResultsEqual(
				Left.SkillRuleQueryResult,
				Right.SkillRuleQueryResult)
			&& AreSkillRulesEqual(
				Left.ResolvedSkillRule,
				Right.ResolvedSkillRule);
	}

	bool AreSkillAvailabilityResultsEqual(
		const FMatchPlayCurrentAttackSkillSelectionAvailabilityResult& Left,
		const FMatchPlayCurrentAttackSkillSelectionAvailabilityResult& Right)
	{
		if (!AreReflectedValuesEqual(Left, Right)
			|| Left.Candidates.Num() != Right.Candidates.Num())
		{
			return false;
		}
		for (int32 Index = 0; Index < Left.Candidates.Num(); ++Index)
		{
			if (!AreSkillLegalityResultsEqual(
				Left.Candidates[Index].LegalityResult,
				Right.Candidates[Index].LegalityResult))
			{
				return false;
			}
		}
		return AreSkillGlobalContextResultsEqual(
				Left.GlobalContextResult,
				Right.GlobalContextResult)
			&& AreCardSnapshotQueryResultsEqual(
				Left.CarrierSnapshotQueryResult,
				Right.CarrierSnapshotQueryResult)
			&& AreSkillRuleValidationResultsEqual(
				Left.SkillRuleSetValidationResult,
				Right.SkillRuleSetValidationResult);
	}

	bool AreSkillWriterResultsEqual(
		const FMatchPlayCurrentAttackSkillSelectionWriterResult& Left,
		const FMatchPlayCurrentAttackSkillSelectionWriterResult& Right)
	{
		return AreReflectedValuesEqual(Left, Right)
			&& AreSkillLegalityResultsEqual(
				Left.LegalityResult,
				Right.LegalityResult);
	}

	bool AreAuthoritativeSubmitMarkerResultsEqual(
		const FMatchPlayAuthoritativeSubmitMarkerResult& Left,
		const FMatchPlayAuthoritativeSubmitMarkerResult& Right)
	{
		return AreEnvelopesEqual(Left.RuntimeEnvelope, Right.RuntimeEnvelope)
			&& AreMarkerWriterResultsEqual(
				Left.MarkerResult,
				Right.MarkerResult);
	}

	bool AreAuthoritativeResolveNoLegalMarkerResultsEqual(
		const FMatchPlayAuthoritativeResolveNoLegalMarkerResult& Left,
		const FMatchPlayAuthoritativeResolveNoLegalMarkerResult& Right)
	{
		return AreEnvelopesEqual(Left.RuntimeEnvelope, Right.RuntimeEnvelope)
			&& AreReflectedValuesEqual(
				Left.ResolutionResult,
				Right.ResolutionResult)
			&& AreMarkerAvailabilityResultsEqual(
				Left.ResolutionResult.MarkerAvailabilityResult,
				Right.ResolutionResult.MarkerAvailabilityResult)
			&& AreCompletionResultsEqual(
				Left.ResolutionResult.CompletionResult,
				Right.ResolutionResult.CompletionResult);
	}

	bool AreAuthoritativeDeclineMarkerResultsEqual(
		const FMatchPlayAuthoritativeDeclineMarkerResult& Left,
		const FMatchPlayAuthoritativeDeclineMarkerResult& Right)
	{
		return AreEnvelopesEqual(Left.RuntimeEnvelope, Right.RuntimeEnvelope)
			&& AreReflectedValuesEqual(Left.DeclineResult, Right.DeclineResult)
			&& AreMarkerAvailabilityResultsEqual(
				Left.DeclineResult.MarkerAvailabilityResult,
				Right.DeclineResult.MarkerAvailabilityResult)
			&& AreCompletionResultsEqual(
				Left.DeclineResult.CompletionResult,
				Right.DeclineResult.CompletionResult);
	}

	bool AreAuthoritativeSubmitSkillResultsEqual(
		const FMatchPlayAuthoritativeSubmitSkillResult& Left,
		const FMatchPlayAuthoritativeSubmitSkillResult& Right)
	{
		return AreEnvelopesEqual(Left.RuntimeEnvelope, Right.RuntimeEnvelope)
			&& AreSkillWriterResultsEqual(
				Left.SkillResult,
				Right.SkillResult);
	}

	bool AreAuthoritativeResolveNoLegalSkillResultsEqual(
		const FMatchPlayAuthoritativeResolveNoLegalSkillResult& Left,
		const FMatchPlayAuthoritativeResolveNoLegalSkillResult& Right)
	{
		return AreEnvelopesEqual(Left.RuntimeEnvelope, Right.RuntimeEnvelope)
			&& AreReflectedValuesEqual(
				Left.ResolutionResult,
				Right.ResolutionResult)
			&& AreSkillAvailabilityResultsEqual(
				Left.ResolutionResult.SkillAvailabilityResult,
				Right.ResolutionResult.SkillAvailabilityResult)
			&& AreCompletionResultsEqual(
				Left.ResolutionResult.CompletionResult,
				Right.ResolutionResult.CompletionResult);
	}

	bool AreAuthoritativeDeclineSkillResultsEqual(
		const FMatchPlayAuthoritativeDeclineSkillResult& Left,
		const FMatchPlayAuthoritativeDeclineSkillResult& Right)
	{
		return AreEnvelopesEqual(Left.RuntimeEnvelope, Right.RuntimeEnvelope)
			&& AreReflectedValuesEqual(Left.DeclineResult, Right.DeclineResult)
			&& AreSkillAvailabilityResultsEqual(
				Left.DeclineResult.SkillAvailabilityResult,
				Right.DeclineResult.SkillAvailabilityResult)
			&& AreCompletionResultsEqual(
				Left.DeclineResult.CompletionResult,
				Right.DeclineResult.CompletionResult);
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
		FMatchPlayState AfterInitialize;
		FMatchPlayAuthoritativeBeginOrdinaryAttackResult Begin;
		FMatchPlayState AfterBegin;
		FDeploymentChoice FirstChoice;
		FMatchPlayAuthoritativeDeployOrdinaryResult FirstDeploy;
		FMatchPlayState AfterFirstDeploy;
		FDeploymentChoice SecondChoice;
		FMatchPlayAuthoritativeDeployOrdinaryResult SecondDeploy;
		FMatchPlayState AfterSecondDeploy;
		FMatchPlayAuthoritativeFinishDeploymentResult FirstFinish;
		FMatchPlayState AfterFirstFinish;
		FMatchPlayAuthoritativeFinishDeploymentResult SecondFinish;
		FMatchPlayState AfterSecondFinish;
		FMatchPlayAuthoritativeSubmitCarrierResult Carrier;
		FMatchPlayState FinalState;
		int64 AttackSequence = 0;
		EInitialTurnOrderPlayer AttackingSide =
			EInitialTurnOrderPlayer::None;
		EInitialTurnOrderPlayer DefendingSide =
			EInitialTurnOrderPlayer::None;
		FName CarrierCardId = NAME_None;
	};

	bool BuildToAwaitingCarrier(
		FMatchPlayAuthoritativeSession& Session,
		const FString& Prefix,
		FReachabilityTrace& OutTrace)
	{
		OutTrace.Initialize = Session.InitializeMatch(MakeValidInput(Prefix));
		OutTrace.AfterInitialize = Session.GetStateSnapshot();
		OutTrace.Begin = Session.BeginOrdinaryAttack(6);
		OutTrace.AfterBegin = Session.GetStateSnapshot();
		if (!OutTrace.Initialize.OpeningResult.bSuccess
			|| !OutTrace.Begin.BeginResult.bSuccess)
		{
			return false;
		}

		FMatchPlayState State = OutTrace.AfterBegin;
		OutTrace.AttackSequence = State.CurrentAttack.AttackSequence;
		OutTrace.AttackingSide = State.RuntimeState.CurrentAttackingPlayer;
		OutTrace.DefendingSide = OtherPlayer(OutTrace.AttackingSide);
		if (!FindLegalDeployment(
			State,
			EMatchPlayRelativeDeploymentZone::Forward,
			OutTrace.FirstChoice))
		{
			return false;
		}
		OutTrace.FirstDeploy = Session.DeployOrdinary(
			MakeDeployRequest(OutTrace.FirstChoice));
		OutTrace.AfterFirstDeploy = Session.GetStateSnapshot();
		if (!OutTrace.FirstDeploy.DeploymentResult.bSuccess)
		{
			return false;
		}

		State = OutTrace.AfterFirstDeploy;
		if (!FindLegalDeployment(
			State,
			EMatchPlayRelativeDeploymentZone::Midfield,
			OutTrace.SecondChoice))
		{
			return false;
		}
		OutTrace.SecondDeploy = Session.DeployOrdinary(
			MakeDeployRequest(OutTrace.SecondChoice));
		OutTrace.AfterSecondDeploy = Session.GetStateSnapshot();
		if (!OutTrace.SecondDeploy.DeploymentResult.bSuccess)
		{
			return false;
		}

		State = OutTrace.AfterSecondDeploy;
		OutTrace.FirstFinish = Session.FinishDeployment(
			State.CurrentAttack.AttackSequence,
			State.CurrentAttack.CurrentLegalDeploymentSide);
		OutTrace.AfterFirstFinish = Session.GetStateSnapshot();
		if (!OutTrace.FirstFinish.FinishResult.bSuccess)
		{
			return false;
		}

		State = OutTrace.AfterFirstFinish;
		OutTrace.SecondFinish = Session.FinishDeployment(
			State.CurrentAttack.AttackSequence,
			State.CurrentAttack.CurrentLegalDeploymentSide);
		OutTrace.AfterSecondFinish = Session.GetStateSnapshot();
		if (!OutTrace.SecondFinish.FinishResult.bSuccess)
		{
			return false;
		}

		State = OutTrace.AfterSecondFinish;
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

	bool BuildAwaitingMarkerReachabilityTrace(
		FMatchPlayAuthoritativeSession& Session,
		const FString& Prefix,
		FReachabilityTrace& OutTrace)
	{
		if (!BuildToAwaitingCarrier(Session, Prefix, OutTrace))
		{
			return false;
		}

		OutTrace.Carrier = Session.SubmitCarrier(
			MakeCarrierRequest(OutTrace));
		OutTrace.FinalState = Session.GetStateSnapshot();
		return OutTrace.Carrier.CarrierResult.bSuccess
			&& OutTrace.FinalState.bHasCurrentAttack
			&& OutTrace.FinalState.CurrentAttack.SelectionStage
				== EMatchPlayCurrentAttackSelectionStage::AwaitingMarker;
	}

	bool BuildFoundationBToAwaitingMarker(
		FMatchPlayAuthoritativeSession& Session,
		const FString& Prefix,
		const bool bCreateLegalMarker,
		const TArray<FName>& CarrierSkillIds,
		FReachabilityTrace& OutTrace)
	{
		OutTrace.Initialize = Session.InitializeMatch(
			MakeFoundationBInput(Prefix, CarrierSkillIds));
		OutTrace.AfterInitialize = Session.GetStateSnapshot();
		OutTrace.Begin = Session.BeginOrdinaryAttack(6);
		OutTrace.AfterBegin = Session.GetStateSnapshot();
		if (!OutTrace.Initialize.OpeningResult.bSuccess
			|| !OutTrace.Begin.BeginResult.bSuccess)
		{
			return false;
		}

		FMatchPlayState State = OutTrace.AfterBegin;
		OutTrace.AttackSequence = State.CurrentAttack.AttackSequence;
		OutTrace.AttackingSide = State.RuntimeState.CurrentAttackingPlayer;
		OutTrace.DefendingSide = OtherPlayer(OutTrace.AttackingSide);
		if (!FindLegalDeployment(
			State,
			EMatchPlayRelativeDeploymentZone::Forward,
			OutTrace.FirstChoice))
		{
			return false;
		}
		OutTrace.FirstDeploy = Session.DeployOrdinary(
			MakeDeployRequest(OutTrace.FirstChoice));
		OutTrace.AfterFirstDeploy = Session.GetStateSnapshot();
		if (!OutTrace.FirstDeploy.DeploymentResult.bSuccess)
		{
			return false;
		}

		State = OutTrace.AfterFirstDeploy;
		const EMatchPlayRelativeDeploymentZone DefenderZone =
			bCreateLegalMarker
				? EMatchPlayRelativeDeploymentZone::Backfield
				: EMatchPlayRelativeDeploymentZone::Midfield;
		if (!FindLegalDeployment(State, DefenderZone, OutTrace.SecondChoice))
		{
			return false;
		}
		OutTrace.SecondDeploy = Session.DeployOrdinary(
			MakeDeployRequest(OutTrace.SecondChoice));
		OutTrace.AfterSecondDeploy = Session.GetStateSnapshot();
		if (!OutTrace.SecondDeploy.DeploymentResult.bSuccess)
		{
			return false;
		}

		State = OutTrace.AfterSecondDeploy;
		OutTrace.FirstFinish = Session.FinishDeployment(
			State.CurrentAttack.AttackSequence,
			State.CurrentAttack.CurrentLegalDeploymentSide);
		OutTrace.AfterFirstFinish = Session.GetStateSnapshot();
		if (!OutTrace.FirstFinish.FinishResult.bSuccess)
		{
			return false;
		}
		State = OutTrace.AfterFirstFinish;
		OutTrace.SecondFinish = Session.FinishDeployment(
			State.CurrentAttack.AttackSequence,
			State.CurrentAttack.CurrentLegalDeploymentSide);
		OutTrace.AfterSecondFinish = Session.GetStateSnapshot();
		if (!OutTrace.SecondFinish.FinishResult.bSuccess)
		{
			return false;
		}

		State = OutTrace.AfterSecondFinish;
		const FMatchPlayCurrentAttackCarrierSelectionAvailabilityResult
			CarrierAvailability =
				FMatchPlayCurrentAttackCarrierSelectionAvailability::Query(
					State,
					State.CurrentAttack.AttackSequence,
					OutTrace.AttackingSide);
		for (const auto& Candidate : CarrierAvailability.Candidates)
		{
			if (Candidate.LegalityResult.bIsLegal)
			{
				OutTrace.CarrierCardId = Candidate.CarrierCardId;
				break;
			}
		}
		if (OutTrace.CarrierCardId.IsNone())
		{
			return false;
		}
		OutTrace.Carrier = Session.SubmitCarrier(MakeCarrierRequest(OutTrace));
		OutTrace.FinalState = Session.GetStateSnapshot();
		return OutTrace.Carrier.CarrierResult.bSuccess
			&& OutTrace.FinalState.CurrentAttack.SelectionStage
				== EMatchPlayCurrentAttackSelectionStage::AwaitingMarker;
	}

	bool FindLegalMarker(
		const FMatchPlayState& State,
		const EInitialTurnOrderPlayer DefendingSide,
		FName& OutMarkerCardId)
	{
		const auto Availability =
			FMatchPlayCurrentAttackMarkerSelectionAvailability::Query(
				State,
				State.CurrentAttack.AttackSequence,
				DefendingSide);
		for (const auto& Candidate : Availability.Candidates)
		{
			if (Candidate.LegalityResult.bIsLegal)
			{
				OutMarkerCardId = Candidate.MarkerCardId;
				return true;
			}
		}
		return false;
	}

	FMatchPlayAuthoritativeSubmitMarkerRequest MakeMarkerRequest(
		const FReachabilityTrace& Trace,
		const FName MarkerCardId)
	{
		FMatchPlayAuthoritativeSubmitMarkerRequest Request;
		Request.RequestingSide = Trace.DefendingSide;
		Request.MarkerCardId = MarkerCardId;
		return Request;
	}

	bool BuildFoundationBToAwaitingSkill(
		FMatchPlayAuthoritativeSession& Session,
		const FString& Prefix,
		const TArray<FName>& CarrierSkillIds,
		FReachabilityTrace& OutTrace,
		FMatchPlayAuthoritativeSubmitMarkerResult& OutMarker)
	{
		if (!BuildFoundationBToAwaitingMarker(
			Session,
			Prefix,
			true,
			CarrierSkillIds,
			OutTrace))
		{
			return false;
		}
		FName MarkerCardId;
		if (!FindLegalMarker(
			OutTrace.FinalState,
			OutTrace.DefendingSide,
			MarkerCardId))
		{
			return false;
		}
		OutMarker = Session.SubmitMarker(
			MakeMarkerRequest(OutTrace, MarkerCardId));
		return OutMarker.MarkerResult.bSuccess
			&& Session.GetStateSnapshot().CurrentAttack.SelectionStage
				== EMatchPlayCurrentAttackSelectionStage::AwaitingSkill;
	}

	void TestAwaitingMarkerEndpoint(
		FAutomationTestBase& Test,
		const FString& Prefix,
		const FReachabilityTrace& Trace)
	{
		const FMatchPlayState& State = Trace.FinalState;
		Test.TestTrue(
			*FString::Printf(TEXT("%s has current attack"), *Prefix),
			State.bHasCurrentAttack);
		Test.TestEqual(
			*FString::Printf(TEXT("%s phase is Resolution"), *Prefix),
			State.CurrentAttack.Phase,
			EMatchPlayCurrentAttackPhase::Resolution);
		Test.TestEqual(
			*FString::Printf(TEXT("%s stage is AwaitingMarker"), *Prefix),
			State.CurrentAttack.SelectionStage,
			EMatchPlayCurrentAttackSelectionStage::AwaitingMarker);
		Test.TestEqual(
			*FString::Printf(TEXT("%s Carrier is selected"), *Prefix),
			State.CurrentAttack.ActionPreparation.CarrierCardId,
			Trace.CarrierCardId);
		Test.TestTrue(
			*FString::Printf(TEXT("%s Marker is absent"), *Prefix),
			State.CurrentAttack.ActionPreparation.MarkerCardId.IsNone());
		Test.TestTrue(
			*FString::Printf(TEXT("%s Skill is absent"), *Prefix),
			State.CurrentAttack.ActionPreparation.SkillId.IsNone());
		Test.TestEqual(
			*FString::Printf(TEXT("%s Skill type is absent"), *Prefix),
			State.CurrentAttack.ActionPreparation.ActionType,
			ESkillRuleType::None);
		Test.TestTrue(
			*FString::Printf(TEXT("%s Runner is absent"), *Prefix),
			State.CurrentAttack.ActionPreparation.RunnerCardId.IsNone());
		Test.TestFalse(
			*FString::Printf(TEXT("%s Helper is absent"), *Prefix),
			State.CurrentAttack.ActionPreparation.bHasHelper);
		Test.TestTrue(
			*FString::Printf(TEXT("%s Helper identity is absent"), *Prefix),
			State.CurrentAttack.ActionPreparation.HelperCardId.IsNone());
		Test.TestFalse(
			*FString::Printf(TEXT("%s selected action is absent"), *Prefix),
			State.CurrentAttack.bHasSelectedAction);
		Test.TestEqual(
			*FString::Printf(TEXT("%s Branch Intent is absent"), *Prefix),
			State.CurrentAttack.SelectedAction.ElectiveBranchIntent,
			EMatchPlayElectiveBranchIntent::None);
		Test.TestFalse(
			*FString::Printf(TEXT("%s Resolution Session is absent"), *Prefix),
			State.CurrentAttack.bHasResolutionSession);
		Test.TestFalse(
			*FString::Printf(TEXT("%s Actual Branch is absent"), *Prefix),
			State.CurrentAttack.ResolutionSession.bHasActualBranch);
		Test.TestTrue(
			*FString::Printf(TEXT("%s Initial Route rolls are empty"), *Prefix),
			State.CurrentAttack.ResolutionSession.InitialRouteRollRecords.IsEmpty());
	}

	void TestAwaitingSkillEndpoint(
		FAutomationTestBase& Test,
		const FString& Context,
		const FMatchPlayState& BeforeState,
		const FMatchPlayState& AfterState,
		const FName MarkerCardId)
	{
		Test.TestTrue(*FString::Printf(TEXT("%s has current attack"), *Context),
			AfterState.bHasCurrentAttack);
		Test.TestEqual(*FString::Printf(TEXT("%s phase"), *Context),
			AfterState.CurrentAttack.Phase,
			EMatchPlayCurrentAttackPhase::Resolution);
		Test.TestEqual(*FString::Printf(TEXT("%s stage"), *Context),
			AfterState.CurrentAttack.SelectionStage,
			EMatchPlayCurrentAttackSelectionStage::AwaitingSkill);
		Test.TestEqual(*FString::Printf(TEXT("%s carrier preserved"), *Context),
			AfterState.CurrentAttack.ActionPreparation.CarrierCardId,
			BeforeState.CurrentAttack.ActionPreparation.CarrierCardId);
		Test.TestEqual(*FString::Printf(TEXT("%s marker selected"), *Context),
			AfterState.CurrentAttack.ActionPreparation.MarkerCardId,
			MarkerCardId);
		Test.TestTrue(*FString::Printf(TEXT("%s skill absent"), *Context),
			AfterState.CurrentAttack.ActionPreparation.SkillId.IsNone());
		Test.TestEqual(*FString::Printf(TEXT("%s action type absent"), *Context),
			AfterState.CurrentAttack.ActionPreparation.ActionType,
			ESkillRuleType::None);
		Test.TestTrue(*FString::Printf(TEXT("%s runner absent"), *Context),
			AfterState.CurrentAttack.ActionPreparation.RunnerCardId.IsNone());
		Test.TestFalse(*FString::Printf(TEXT("%s helper absent"), *Context),
			AfterState.CurrentAttack.ActionPreparation.bHasHelper);
		Test.TestTrue(*FString::Printf(TEXT("%s helper identity absent"), *Context),
			AfterState.CurrentAttack.ActionPreparation.HelperCardId.IsNone());
		Test.TestFalse(*FString::Printf(TEXT("%s selected action absent"), *Context),
			AfterState.CurrentAttack.bHasSelectedAction);
		Test.TestEqual(*FString::Printf(TEXT("%s branch intent absent"), *Context),
			AfterState.CurrentAttack.SelectedAction.ElectiveBranchIntent,
			EMatchPlayElectiveBranchIntent::None);
		Test.TestFalse(*FString::Printf(TEXT("%s resolution session absent"), *Context),
			AfterState.CurrentAttack.bHasResolutionSession);
		Test.TestFalse(*FString::Printf(TEXT("%s actual branch absent"), *Context),
			AfterState.CurrentAttack.ResolutionSession.bHasActualBranch);
		Test.TestTrue(*FString::Printf(TEXT("%s route rolls empty"), *Context),
			AfterState.CurrentAttack.ResolutionSession.InitialRouteRollRecords.IsEmpty());
	}

	void TestSubmittedSkillEndpoint(
		FAutomationTestBase& Test,
		const FString& Context,
		const FMatchPlayState& BeforeState,
		const FMatchPlayState& AfterState,
		const FName SkillId,
		const ESkillRuleType ActionType,
		const EMatchPlayCurrentAttackSelectionStage ExpectedStage)
	{
		Test.TestTrue(*FString::Printf(TEXT("%s has current attack"), *Context),
			AfterState.bHasCurrentAttack);
		Test.TestEqual(*FString::Printf(TEXT("%s stage"), *Context),
			AfterState.CurrentAttack.SelectionStage,
			ExpectedStage);
		Test.TestEqual(*FString::Printf(TEXT("%s carrier preserved"), *Context),
			AfterState.CurrentAttack.ActionPreparation.CarrierCardId,
			BeforeState.CurrentAttack.ActionPreparation.CarrierCardId);
		Test.TestEqual(*FString::Printf(TEXT("%s marker preserved"), *Context),
			AfterState.CurrentAttack.ActionPreparation.MarkerCardId,
			BeforeState.CurrentAttack.ActionPreparation.MarkerCardId);
		Test.TestEqual(*FString::Printf(TEXT("%s skill selected"), *Context),
			AfterState.CurrentAttack.ActionPreparation.SkillId,
			SkillId);
		Test.TestEqual(*FString::Printf(TEXT("%s action type selected"), *Context),
			AfterState.CurrentAttack.ActionPreparation.ActionType,
			ActionType);
		Test.TestTrue(*FString::Printf(TEXT("%s runner absent"), *Context),
			AfterState.CurrentAttack.ActionPreparation.RunnerCardId.IsNone());
		Test.TestFalse(*FString::Printf(TEXT("%s helper absent"), *Context),
			AfterState.CurrentAttack.ActionPreparation.bHasHelper);
		Test.TestTrue(*FString::Printf(TEXT("%s helper identity absent"), *Context),
			AfterState.CurrentAttack.ActionPreparation.HelperCardId.IsNone());
		Test.TestFalse(*FString::Printf(TEXT("%s selected action absent"), *Context),
			AfterState.CurrentAttack.bHasSelectedAction);
		Test.TestEqual(*FString::Printf(TEXT("%s branch intent absent"), *Context),
			AfterState.CurrentAttack.SelectedAction.ElectiveBranchIntent,
			EMatchPlayElectiveBranchIntent::None);
		Test.TestFalse(*FString::Printf(TEXT("%s resolution session absent"), *Context),
			AfterState.CurrentAttack.bHasResolutionSession);
		Test.TestFalse(*FString::Printf(TEXT("%s actual branch absent"), *Context),
			AfterState.CurrentAttack.ResolutionSession.bHasActualBranch);
		Test.TestTrue(*FString::Printf(TEXT("%s route rolls empty"), *Context),
			AfterState.CurrentAttack.ResolutionSession.InitialRouteRollRecords.IsEmpty());
	}

	void TestCompletedAttackEndpoint(
		FAutomationTestBase& Test,
		const FString& Context,
		const FMatchPlayCurrentAttackCompletionResult& Completion,
		const FMatchPlayState& FinalState)
	{
		Test.TestTrue(*FString::Printf(TEXT("%s completion success"), *Context),
			Completion.bSuccess);
		Test.TestFalse(*FString::Printf(TEXT("%s current attack cleared"), *Context),
			FinalState.bHasCurrentAttack);
		Test.TestTrue(*FString::Printf(TEXT("%s nested state clears attack"), *Context),
			!Completion.AfterState.bHasCurrentAttack);
		Test.TestFalse(*FString::Printf(TEXT("%s resolution session absent"), *Context),
			FinalState.CurrentAttack.bHasResolutionSession);
		Test.TestFalse(*FString::Printf(TEXT("%s actual branch absent"), *Context),
			FinalState.CurrentAttack.ResolutionSession.bHasActualBranch);
		Test.TestTrue(*FString::Printf(TEXT("%s route rolls empty"), *Context),
			FinalState.CurrentAttack.ResolutionSession.InitialRouteRollRecords.IsEmpty());
		Test.TestEqual(*FString::Printf(TEXT("%s next attacker adopted"), *Context),
			Completion.NextAttackingPlayer,
			FinalState.RuntimeState.CurrentAttackingPlayer);
	}

	enum class ECompletionScoringExpectation : uint8
	{
		Goal,
		NoGoal
	};

	bool HasExpectedCompletionScoringSemantics(
		const FMatchPlayState& BeforeState,
		const FMatchPlayCurrentAttackCompletionResult& Completion,
		const FMatchPlayState& FinalState,
		const ECompletionScoringExpectation Expectation,
		const EInitialTurnOrderPlayer ExpectedScoringSide)
	{
		const int32 PlayerABefore = BeforeState.RuntimeState.PlayerAState.Score;
		const int32 PlayerBBefore = BeforeState.RuntimeState.PlayerBState.Score;
		const int32 PlayerAIncrement =
			Expectation == ECompletionScoringExpectation::Goal
			&& ExpectedScoringSide == EInitialTurnOrderPlayer::PlayerA ? 1 : 0;
		const int32 PlayerBIncrement =
			Expectation == ECompletionScoringExpectation::Goal
			&& ExpectedScoringSide == EInitialTurnOrderPlayer::PlayerB ? 1 : 0;
		const EInitialTurnOrderPlayer ContractScoringSide =
			Expectation == ECompletionScoringExpectation::Goal
				? ExpectedScoringSide
				: EInitialTurnOrderPlayer::None;
		const bool bGoal = Expectation == ECompletionScoringExpectation::Goal;

		return Completion.bSuccess
			&& AreStatesEqual(Completion.BeforeState, BeforeState)
			&& AreStatesEqual(Completion.AfterState, FinalState)
			&& Completion.ScoringSide == ContractScoringSide
			&& Completion.GoalResolveResult.bSuccess == bGoal
			&& Completion.GoalResolveResult.ScoringSide == ContractScoringSide
			&& Completion.GoalResolveResult.PlayerAScoreBefore == PlayerABefore
			&& Completion.GoalResolveResult.PlayerBScoreBefore == PlayerBBefore
			&& Completion.GoalResolveResult.PlayerAScoreAfter
				== PlayerABefore + PlayerAIncrement
			&& Completion.GoalResolveResult.PlayerBScoreAfter
				== PlayerBBefore + PlayerBIncrement
			&& FinalState.RuntimeState.PlayerAState.Score
				== PlayerABefore + PlayerAIncrement
			&& FinalState.RuntimeState.PlayerBState.Score
				== PlayerBBefore + PlayerBIncrement
			&& Completion.GoalResolveResult.UpdatedRuntimeState.PlayerAState.Score
				== FinalState.RuntimeState.PlayerAState.Score
			&& Completion.GoalResolveResult.UpdatedRuntimeState.PlayerBState.Score
				== FinalState.RuntimeState.PlayerBState.Score
			&& !FinalState.bHasCurrentAttack
			&& !Completion.AfterState.bHasCurrentAttack
			&& Completion.NextAttackingPlayer
				== FinalState.RuntimeState.CurrentAttackingPlayer
			&& !FinalState.CurrentAttack.bHasResolutionSession
			&& !FinalState.CurrentAttack.ResolutionSession.bHasActualBranch
			&& FinalState.CurrentAttack.ResolutionSession
				.InitialRouteRollRecords.IsEmpty();
	}

	void TestCompletionScoringContract(
		FAutomationTestBase& Test,
		const FString& Context,
		const FMatchPlayState& BeforeState,
		const FMatchPlayCurrentAttackCompletionResult& Completion,
		const FMatchPlayState& FinalState,
		const ECompletionScoringExpectation Expectation,
		const EInitialTurnOrderPlayer ExpectedScoringSide,
		const bool bTestNegativeMutations)
	{
		const bool bGoal = Expectation == ECompletionScoringExpectation::Goal;
		const EInitialTurnOrderPlayer ContractScoringSide = bGoal
			? ExpectedScoringSide : EInitialTurnOrderPlayer::None;
		const int32 PlayerABefore = BeforeState.RuntimeState.PlayerAState.Score;
		const int32 PlayerBBefore = BeforeState.RuntimeState.PlayerBState.Score;
		const int32 PlayerAAfter = PlayerABefore
			+ (bGoal && ExpectedScoringSide == EInitialTurnOrderPlayer::PlayerA ? 1 : 0);
		const int32 PlayerBAfter = PlayerBBefore
			+ (bGoal && ExpectedScoringSide == EInitialTurnOrderPlayer::PlayerB ? 1 : 0);
		Test.TestEqual(*FString::Printf(TEXT("%s GoalResolve outcome"), *Context),
			Completion.GoalResolveResult.bSuccess, bGoal);
		Test.TestEqual(*FString::Printf(TEXT("%s completion scoring side"), *Context),
			Completion.ScoringSide, ContractScoringSide);
		Test.TestEqual(*FString::Printf(TEXT("%s GoalResolve scoring side"), *Context),
			Completion.GoalResolveResult.ScoringSide, ContractScoringSide);
		Test.TestEqual(*FString::Printf(TEXT("%s PlayerA score before"), *Context),
			Completion.GoalResolveResult.PlayerAScoreBefore, PlayerABefore);
		Test.TestEqual(*FString::Printf(TEXT("%s PlayerB score before"), *Context),
			Completion.GoalResolveResult.PlayerBScoreBefore, PlayerBBefore);
		Test.TestEqual(*FString::Printf(TEXT("%s PlayerA score after"), *Context),
			FinalState.RuntimeState.PlayerAState.Score, PlayerAAfter);
		Test.TestEqual(*FString::Printf(TEXT("%s PlayerB score after"), *Context),
			FinalState.RuntimeState.PlayerBState.Score, PlayerBAfter);
		Test.TestTrue(
			*FString::Printf(TEXT("%s exact scoring semantics"), *Context),
			HasExpectedCompletionScoringSemantics(
				BeforeState, Completion, FinalState, Expectation,
				ExpectedScoringSide));
		if (!bTestNegativeMutations)
		{
			return;
		}

		FMatchPlayCurrentAttackCompletionResult Mutated = Completion;
		Mutated.GoalResolveResult.bSuccess =
			!Mutated.GoalResolveResult.bSuccess;
		Test.TestFalse(*FString::Printf(TEXT("%s rejects goal flag mutation"), *Context),
			HasExpectedCompletionScoringSemantics(
				BeforeState, Mutated, FinalState, Expectation,
				ExpectedScoringSide));

		Mutated = Completion;
		Mutated.ScoringSide = Expectation == ECompletionScoringExpectation::Goal
			? OtherPlayer(ExpectedScoringSide)
			: ExpectedScoringSide;
		Test.TestFalse(*FString::Printf(TEXT("%s rejects scorer mutation"), *Context),
			HasExpectedCompletionScoringSemantics(
				BeforeState, Mutated, FinalState, Expectation,
				ExpectedScoringSide));

		Mutated = Completion;
		++Mutated.GoalResolveResult.PlayerAScoreAfter;
		Test.TestFalse(*FString::Printf(TEXT("%s rejects score mutation"), *Context),
			HasExpectedCompletionScoringSemantics(
				BeforeState, Mutated, FinalState, Expectation,
				ExpectedScoringSide));
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

	enum class EMutationCoverageCategory : uint8
	{
		ReflectedLeaf,
		NonReflectedField,
		ContainerBehavior,
		Envelope
	};

	struct FMutationCoverageAudit
	{
		int64 ExecutedMutationCases = 0;
		int64 EqualBaselineCases = 0;
		TSet<FString> ExpectedLogicalPaths;
		TSet<FString> CoveredLogicalPaths;
		TSet<FString> ReflectedLeafPaths;
		TSet<FString> NonReflectedFieldPaths;
		TSet<FString> ContainerCasePaths;
		TSet<FString> EmptyContainerElementFieldPaths;
		TSet<FString> EnvelopePaths;
		TSet<FString> SkippedGroupPaths;
		TSet<FString> ReachableMapPaths;
		TMap<FString, int32> ExecutionCountByPath;

		void Expect(const FString& Path)
		{
			ExpectedLogicalPaths.Add(Path);
		}

		void Cover(
			const FString& Path,
			const EMutationCoverageCategory Category,
			const bool bEmptyContainerElement = false)
		{
			CoveredLogicalPaths.Add(Path);
			++ExecutedMutationCases;
			++ExecutionCountByPath.FindOrAdd(Path);
			switch (Category)
			{
			case EMutationCoverageCategory::ReflectedLeaf:
				ReflectedLeafPaths.Add(Path);
				break;
			case EMutationCoverageCategory::NonReflectedField:
				NonReflectedFieldPaths.Add(Path);
				break;
			case EMutationCoverageCategory::ContainerBehavior:
				ContainerCasePaths.Add(Path);
				break;
			case EMutationCoverageCategory::Envelope:
				EnvelopePaths.Add(Path);
				break;
			}
			if (bEmptyContainerElement)
			{
				EmptyContainerElementFieldPaths.Add(Path);
			}
		}

		void RecordEqualBaseline()
		{
			++EqualBaselineCases;
		}

		void Skip(const FString& Path)
		{
			SkippedGroupPaths.Add(Path);
		}
	};

	void TestEnvelopeMutationCoverage(
		FAutomationTestBase& Test,
		const FMatchPlayAuthoritativeRuntimeEnvelope& Baseline,
		FMutationCoverageAudit* CoverageAudit = nullptr,
		const FString& Prefix = TEXT("Envelope"))
	{
		auto RejectEqual = [&Test, &Baseline, CoverageAudit, &Prefix](
			const TCHAR* Field,
			const TCHAR* LogicalField,
			const FMatchPlayAuthoritativeRuntimeEnvelope& Mutated)
		{
			const FString Path = Prefix + TEXT(".") + LogicalField;
			if (CoverageAudit != nullptr)
			{
				CoverageAudit->Expect(Path);
			}
			const bool bRejected = !AreEnvelopesEqual(Baseline, Mutated);
			Test.TestTrue(Field, bRejected);
			if (bRejected && CoverageAudit != nullptr)
			{
				CoverageAudit->Cover(
					Path,
					EMutationCoverageCategory::Envelope);
			}
		};

		FMatchPlayAuthoritativeRuntimeEnvelope Mutated = Baseline;
		Mutated.bAccepted = !Mutated.bAccepted;
		RejectEqual(TEXT("Envelope comparator covers bAccepted"), TEXT("bAccepted"), Mutated);
		Mutated = Baseline;
		Mutated.bDomainSuccess = !Mutated.bDomainSuccess;
		RejectEqual(TEXT("Envelope comparator covers bDomainSuccess"), TEXT("bDomainSuccess"), Mutated);
		Mutated = Baseline;
		Mutated.bStateAdvanced = !Mutated.bStateAdvanced;
		RejectEqual(TEXT("Envelope comparator covers bStateAdvanced"), TEXT("bStateAdvanced"), Mutated);
		Mutated = Baseline;
		Mutated.StateDisposition =
			EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
		RejectEqual(TEXT("Envelope comparator covers StateDisposition"), TEXT("StateDisposition"), Mutated);
		Mutated = Baseline;
		Mutated.bRuntimeFault = !Mutated.bRuntimeFault;
		RejectEqual(TEXT("Envelope comparator covers bRuntimeFault"), TEXT("bRuntimeFault"), Mutated);
		Mutated = Baseline;
		Mutated.BeforeState.RuntimeState.bIsInitialized =
			!Mutated.BeforeState.RuntimeState.bIsInitialized;
		RejectEqual(TEXT("Envelope comparator covers BeforeState"), TEXT("BeforeState"), Mutated);
		Mutated = Baseline;
		Mutated.AfterState.bHasCurrentAttack =
			!Mutated.AfterState.bHasCurrentAttack;
		RejectEqual(TEXT("Envelope comparator covers AfterState"), TEXT("AfterState"), Mutated);
		Mutated = Baseline;
		Mutated.CommandKind = EMatchPlayAuthoritativeCommandKind::None;
		RejectEqual(TEXT("Envelope comparator covers CommandKind"), TEXT("CommandKind"), Mutated);
		Mutated = Baseline;
		++Mutated.AttackSequence;
		RejectEqual(TEXT("Envelope comparator covers AttackSequence"), TEXT("AttackSequence"), Mutated);
		Mutated = Baseline;
		Mutated.FailureDisposition =
			EMatchPlayAuthoritativeFailureDisposition
				::RetryableExecutionFailure;
		RejectEqual(TEXT("Envelope comparator covers FailureDisposition"), TEXT("FailureDisposition"), Mutated);
		Mutated = Baseline;
		Mutated.RuntimeFailureCode =
			EMatchPlayAuthoritativeRuntimeFailureCode::NotInitialized;
		RejectEqual(TEXT("Envelope comparator covers RuntimeFailureCode"), TEXT("RuntimeFailureCode"), Mutated);
		Mutated = Baseline;
		Mutated.ErrorMessage = TEXT("mutated runtime message");
		RejectEqual(TEXT("Envelope comparator covers ErrorMessage"), TEXT("ErrorMessage"), Mutated);
	}

	struct FReflectedMutationAudit
	{
		int32 LeafMutations = 0;
		int32 ContainerCases = 0;
		int32 EmptyContainerElementStructures = 0;
		int32 SkippedProperties = 0;
		TArray<FString> SkippedPaths;
	};

	void VisitReflectedMutationLeaves(
		UStruct* Struct,
		void* MutatedContainer,
		void* BaselineContainer,
		const FString& Prefix,
		FReflectedMutationAudit& Audit,
		const TFunctionRef<bool(const FString&)>& RejectMutation,
		FMutationCoverageAudit* CoverageAudit = nullptr,
		bool bEmptyContainerElement = false);

	void VisitReflectedPropertyMutation(
		FProperty* Property,
		void* MutatedValue,
		void* BaselineValue,
		const FString& Path,
		FReflectedMutationAudit& Audit,
		const TFunctionRef<bool(const FString&)>& RejectMutation,
		FMutationCoverageAudit* CoverageAudit,
		const bool bEmptyContainerElement)
	{
		if (FStructProperty* StructProperty =
			CastField<FStructProperty>(Property))
		{
			if (StructProperty->Struct == FMatchState::StaticStruct())
			{
				FMatchState* MutatedState = static_cast<FMatchState*>(MutatedValue);
				++MutatedState->CurrentActionPoint;
				if (CoverageAudit != nullptr)
				{
					CoverageAudit->Expect(Path);
				}
				const bool bRejected = RejectMutation(Path);
				if (bRejected && CoverageAudit != nullptr)
				{
					CoverageAudit->Cover(
						Path,
						EMutationCoverageCategory::ReflectedLeaf,
						bEmptyContainerElement);
				}
				++Audit.LeafMutations;
				StructProperty->CopyCompleteValue(MutatedValue, BaselineValue);
				return;
			}
			VisitReflectedMutationLeaves(
				StructProperty->Struct,
				MutatedValue,
				BaselineValue,
				Path,
				Audit,
				RejectMutation,
				CoverageAudit,
				bEmptyContainerElement);
			return;
		}

		if (FMapProperty* MapProperty = CastField<FMapProperty>(Property))
		{
			if (CoverageAudit != nullptr)
			{
				CoverageAudit->ReachableMapPaths.Add(Path);
			}
			FScriptMapHelper MutatedMap(MapProperty, MutatedValue);
			FScriptMapHelper BaselineMap(MapProperty, BaselineValue);
			TArray<int32> BaselineIndices;
			for (int32 Index = 0; Index < BaselineMap.GetMaxIndex(); ++Index)
			{
				if (BaselineMap.IsValidIndex(Index))
				{
					BaselineIndices.Add(Index);
				}
			}

			if (BaselineIndices.Num() > 0)
			{
				MutatedMap.RemoveAt(BaselineIndices.Last());
			}
			else
			{
				MutatedMap.AddDefaultValue_Invalid_NeedsRehash();
				MutatedMap.Rehash();
			}
			const FString MembershipPath = Path + TEXT(".Membership");
			if (CoverageAudit != nullptr)
			{
				CoverageAudit->Expect(MembershipPath);
			}
			const bool bMembershipRejected = RejectMutation(MembershipPath);
			if (bMembershipRejected && CoverageAudit != nullptr)
			{
				CoverageAudit->Cover(
					MembershipPath,
					EMutationCoverageCategory::ContainerBehavior);
			}
			++Audit.ContainerCases;
			MapProperty->CopyCompleteValue(MutatedValue, BaselineValue);

			bool bAddedCanonicalElement = false;
			if (BaselineIndices.Num() == 0)
			{
				const int32 BaselineIndex =
					BaselineMap.AddDefaultValue_Invalid_NeedsRehash();
				const int32 MutatedIndex =
					MutatedMap.AddDefaultValue_Invalid_NeedsRehash();
				BaselineMap.Rehash();
				MutatedMap.Rehash();
				BaselineIndices = {BaselineIndex};
				check(BaselineIndex == MutatedIndex);
				bAddedCanonicalElement = true;
				++Audit.EmptyContainerElementStructures;
			}

			TArray<int32> Positions = {0};
			if (BaselineIndices.Num() >= 3)
			{
				Positions.Add(BaselineIndices.Num() / 2);
			}
			if (BaselineIndices.Num() >= 2)
			{
				Positions.Add(BaselineIndices.Num() - 1);
			}
			for (const int32 Position : Positions)
			{
				const int32 Index = BaselineIndices[Position];
				VisitReflectedPropertyMutation(
					MapProperty->KeyProp,
					MutatedMap.GetKeyPtr(Index),
					BaselineMap.GetKeyPtr(Index),
					bAddedCanonicalElement
						? FString::Printf(TEXT("%s.EmptyElement.Key[%d]"), *Path, Position)
						: FString::Printf(TEXT("%s.Key[%d]"), *Path, Position),
					Audit,
					[&](const FString& MutationPath)
					{
						MutatedMap.Rehash();
						return RejectMutation(MutationPath);
					},
					CoverageAudit,
					bAddedCanonicalElement || bEmptyContainerElement);
				MutatedMap.Rehash();
				VisitReflectedPropertyMutation(
					MapProperty->ValueProp,
					MutatedMap.GetValuePtr(Index),
					BaselineMap.GetValuePtr(Index),
					bAddedCanonicalElement
						? FString::Printf(TEXT("%s.EmptyElement.Value[%d]"), *Path, Position)
						: FString::Printf(TEXT("%s.Value[%d]"), *Path, Position),
					Audit,
					RejectMutation,
					CoverageAudit,
					bAddedCanonicalElement || bEmptyContainerElement);
			}

			if (bAddedCanonicalElement)
			{
				MutatedMap.EmptyValues();
				BaselineMap.EmptyValues();
			}
			return;
		}

		if (FArrayProperty* ArrayProperty =
			CastField<FArrayProperty>(Property))
		{
			FScriptArrayHelper MutatedArray(ArrayProperty, MutatedValue);
			FScriptArrayHelper BaselineArray(ArrayProperty, BaselineValue);
			if (MutatedArray.Num() > 0)
			{
				MutatedArray.RemoveValues(MutatedArray.Num() - 1, 1);
			}
			else
			{
				MutatedArray.AddValue();
			}
			const FString NumPath = Path + TEXT(".Num");
			if (CoverageAudit != nullptr)
			{
				CoverageAudit->Expect(NumPath);
			}
			const bool bNumRejected = RejectMutation(NumPath);
			if (bNumRejected && CoverageAudit != nullptr)
			{
				CoverageAudit->Cover(
					NumPath,
					EMutationCoverageCategory::ContainerBehavior);
			}
			++Audit.ContainerCases;
			ArrayProperty->CopyCompleteValue(MutatedValue, BaselineValue);

			bool bAddedCanonicalElement = false;
			if (BaselineArray.Num() == 0)
			{
				BaselineArray.AddValue();
				MutatedArray.AddValue();
				bAddedCanonicalElement = true;
				++Audit.EmptyContainerElementStructures;
			}

			TArray<int32> Indices = {0};
			if (BaselineArray.Num() >= 3)
			{
				Indices.Add(BaselineArray.Num() / 2);
			}
			if (BaselineArray.Num() >= 2)
			{
				Indices.Add(BaselineArray.Num() - 1);
			}
			for (const int32 Index : Indices)
			{
				VisitReflectedPropertyMutation(
					ArrayProperty->Inner,
					MutatedArray.GetRawPtr(Index),
					BaselineArray.GetRawPtr(Index),
					bAddedCanonicalElement
						? FString::Printf(TEXT("%s.EmptyElement[%d]"), *Path, Index)
						: FString::Printf(TEXT("%s[%d]"), *Path, Index),
					Audit,
					RejectMutation,
					CoverageAudit,
					bAddedCanonicalElement || bEmptyContainerElement);
			}

			if (BaselineArray.Num() >= 2
				&& !ArrayProperty->Inner->Identical(
					BaselineArray.GetRawPtr(0),
					BaselineArray.GetRawPtr(BaselineArray.Num() - 1)))
			{
				MutatedArray.SwapValues(0, BaselineArray.Num() - 1);
				const FString OrderPath = Path + TEXT(".Order");
				if (CoverageAudit != nullptr)
				{
					CoverageAudit->Expect(OrderPath);
				}
				const bool bOrderRejected = RejectMutation(OrderPath);
				if (bOrderRejected && CoverageAudit != nullptr)
				{
					CoverageAudit->Cover(
						OrderPath,
						EMutationCoverageCategory::ContainerBehavior);
				}
				++Audit.ContainerCases;
				ArrayProperty->CopyCompleteValue(MutatedValue, BaselineValue);
			}

			if (bAddedCanonicalElement)
			{
				MutatedArray.RemoveValues(0, 1);
				BaselineArray.RemoveValues(0, 1);
			}
			return;
		}

		if (FBoolProperty* BoolProperty = CastField<FBoolProperty>(Property))
		{
			BoolProperty->SetPropertyValue(
				MutatedValue,
				!BoolProperty->GetPropertyValue(MutatedValue));
		}
		else if (FEnumProperty* EnumProperty =
			CastField<FEnumProperty>(Property))
		{
			FNumericProperty* Underlying = EnumProperty->GetUnderlyingProperty();
			const int64 Value = Underlying->GetSignedIntPropertyValue(MutatedValue);
			Underlying->SetIntPropertyValue(
				MutatedValue,
				static_cast<int64>(Value == 0 ? 1 : 0));
		}
		else if (FNumericProperty* NumericProperty =
			CastField<FNumericProperty>(Property))
		{
			if (NumericProperty->IsFloatingPoint())
			{
				NumericProperty->SetFloatingPointPropertyValue(
					MutatedValue,
					NumericProperty->GetFloatingPointPropertyValue(MutatedValue)
						+ 1.0);
			}
			else
			{
				const int64 Value =
					NumericProperty->GetSignedIntPropertyValue(MutatedValue);
				NumericProperty->SetIntPropertyValue(
					MutatedValue,
					static_cast<int64>(Value == 0 ? 1 : 0));
			}
		}
		else if (FNameProperty* NameProperty =
			CastField<FNameProperty>(Property))
		{
			NameProperty->SetPropertyValue(
				MutatedValue,
				TEXT("Mutation.Coverage"));
		}
		else if (FStrProperty* StringProperty =
			CastField<FStrProperty>(Property))
		{
			StringProperty->SetPropertyValue(
				MutatedValue,
				TEXT("mutation coverage"));
		}
		else if (FTextProperty* TextProperty =
			CastField<FTextProperty>(Property))
		{
			TextProperty->SetPropertyValue(
				MutatedValue,
				FText::FromString(TEXT("mutation coverage")));
		}
		else
		{
			++Audit.SkippedProperties;
			Audit.SkippedPaths.Add(FString::Printf(
				TEXT("%s (%s)"),
				*Path,
				*Property->GetClass()->GetName()));
			if (CoverageAudit != nullptr)
			{
				CoverageAudit->Expect(Path);
				CoverageAudit->Skip(Path + TEXT(".UnsupportedPropertyType"));
			}
			return;
		}

		if (Property->Identical(MutatedValue, BaselineValue))
		{
			++Audit.SkippedProperties;
			Audit.SkippedPaths.Add(Path + TEXT(" (mutation unchanged)"));
			if (CoverageAudit != nullptr)
			{
				CoverageAudit->Expect(Path);
				CoverageAudit->Skip(Path + TEXT(".MutationUnchanged"));
			}
			return;
		}
		if (CoverageAudit != nullptr)
		{
			CoverageAudit->Expect(Path);
		}
		const bool bRejected = RejectMutation(Path);
		if (bRejected && CoverageAudit != nullptr)
		{
			CoverageAudit->Cover(
				Path,
				EMutationCoverageCategory::ReflectedLeaf,
				bEmptyContainerElement);
		}
		++Audit.LeafMutations;
		Property->CopyCompleteValue(MutatedValue, BaselineValue);
	}

	void VisitReflectedMutationLeaves(
		UStruct* Struct,
		void* MutatedContainer,
		void* BaselineContainer,
		const FString& Prefix,
		FReflectedMutationAudit& Audit,
		const TFunctionRef<bool(const FString&)>& RejectMutation,
		FMutationCoverageAudit* CoverageAudit,
		const bool bEmptyContainerElement)
	{
		for (TFieldIterator<FProperty> It(Struct); It; ++It)
		{
			FProperty* Property = *It;
			VisitReflectedPropertyMutation(
				Property,
				Property->ContainerPtrToValuePtr<void>(MutatedContainer),
				Property->ContainerPtrToValuePtr<void>(BaselineContainer),
				Prefix + TEXT(".") + Property->GetName(),
				Audit,
				RejectMutation,
				CoverageAudit,
				bEmptyContainerElement);
		}
	}

	template <typename TWrapper, typename TNested, typename TAccessor,
		typename TComparator>
	FReflectedMutationAudit TestNestedReflectedMutationCoverage(
		FAutomationTestBase& Test,
		const TCHAR* Context,
		const TWrapper& Baseline,
		TAccessor&& Accessor,
		TComparator&& Comparator,
		FMutationCoverageAudit* CoverageAudit = nullptr,
		const bool bEmptyContainerElement = false)
	{
		TWrapper ComparatorBaseline = Baseline;
		TWrapper Mutated = Baseline;
		FReflectedMutationAudit Audit;
		TNested& ComparatorBaselineNested = Accessor(ComparatorBaseline);
		TNested& MutatedNested = Accessor(Mutated);
		VisitReflectedMutationLeaves(
			TNested::StaticStruct(),
			&MutatedNested,
			&ComparatorBaselineNested,
			Context,
			Audit,
			[&](const FString& Path)
			{
				const bool bRejected = !Comparator(ComparatorBaseline, Mutated);
				Test.TestTrue(
					*FString::Printf(TEXT("Comparator covers %s"), *Path),
					bRejected);
				return bRejected;
			},
			CoverageAudit,
			bEmptyContainerElement);
		Test.TestEqual(
			*FString::Printf(TEXT("%s reflected mutation skipped groups"), Context),
			Audit.SkippedProperties,
			0);
		if (Audit.SkippedPaths.Num() > 0)
		{
			Test.AddInfo(FString::Printf(
				TEXT("%s skipped paths: %s"),
				Context,
				*FString::Join(Audit.SkippedPaths, TEXT("; "))));
		}
		return Audit;
	}

	struct FNonReflectedMutationAudit
	{
		int32 DeclaredFields = 0;
		int32 ReflectedNestedLeaves = 0;
		int32 ContainerCases = 0;
		int32 EmptyContainerCases = 0;
		int32 EqualBaselines = 0;
		int32 SkippedGroups = 0;
		FMutationCoverageAudit* CoverageAudit = nullptr;
	};

	template <typename T, typename TComparator, typename TMutator>
	void TestPlainMutation(
		FAutomationTestBase& Test,
		const FString& Context,
		const T& Baseline,
		TComparator&& Comparator,
		TMutator&& Mutator,
		FNonReflectedMutationAudit& Audit)
	{
		T Mutated = Baseline;
		Mutator(Mutated);
		const bool bRejected = !Comparator(Baseline, Mutated);
		Test.TestTrue(
			*FString::Printf(TEXT("Non-reflected comparator covers %s"), *Context),
			bRejected);
		if (Audit.CoverageAudit != nullptr)
		{
			const FString Path = TEXT("NonReflected.") + Context;
			Audit.CoverageAudit->Expect(Path);
			if (bRejected)
			{
				Audit.CoverageAudit->Cover(
					Path,
					EMutationCoverageCategory::NonReflectedField);
			}
			else
			{
				Audit.CoverageAudit->Skip(Path + TEXT(".ComparatorAcceptedMutation"));
			}
		}
		++Audit.DeclaredFields;
	}

	template <typename T, typename TReflected, typename TAccessor,
		typename TComparator>
	void TestPlainNestedReflectedMutations(
		FAutomationTestBase& Test,
		const FString& Context,
		const T& InputBaseline,
		TAccessor&& Accessor,
		TComparator&& Comparator,
		FNonReflectedMutationAudit& Audit)
	{
		T Baseline = InputBaseline;
		T Mutated = InputBaseline;
		TReflected& BaselineNested = Accessor(Baseline);
		TReflected& MutatedNested = Accessor(Mutated);
		FReflectedMutationAudit ReflectedAudit;
		VisitReflectedMutationLeaves(
			TReflected::StaticStruct(),
			&MutatedNested,
			&BaselineNested,
			Context,
			ReflectedAudit,
			[&](const FString& Path)
			{
				const bool bRejected = !Comparator(Baseline, Mutated);
				Test.TestTrue(
					*FString::Printf(TEXT("Nested reflected comparator covers %s"), *Path),
					bRejected);
				return bRejected;
			},
			Audit.CoverageAudit,
			false);
		Audit.ReflectedNestedLeaves += ReflectedAudit.LeafMutations;
		Audit.ContainerCases += ReflectedAudit.ContainerCases;
		Audit.EmptyContainerCases +=
			ReflectedAudit.EmptyContainerElementStructures;
		Audit.SkippedGroups += ReflectedAudit.SkippedProperties;
		if (Audit.CoverageAudit != nullptr)
		{
			const FString Path = TEXT("NonReflected.") + Context;
			Audit.CoverageAudit->Expect(Path);
			if (ReflectedAudit.LeafMutations > 0
				&& ReflectedAudit.SkippedProperties == 0)
			{
				Audit.CoverageAudit->Cover(
					Path,
					EMutationCoverageCategory::NonReflectedField);
			}
		}
	}

	template <typename TEnum>
	TEnum OtherEnumValue(const TEnum Value)
	{
		return Value == static_cast<TEnum>(0)
			? static_cast<TEnum>(1)
			: static_cast<TEnum>(0);
	}

	template <typename TWrapper, typename TComparator, typename TMutator>
	bool ExecuteAuditedMutation(
		FAutomationTestBase& Test,
		FMutationCoverageAudit& Audit,
		const FString& Path,
		const TWrapper& Baseline,
		TComparator Comparator,
		TMutator Mutator,
		const EMutationCoverageCategory Category,
		const bool bEmptyContainerElement = false)
	{
		Audit.Expect(Path);
		TWrapper Mutated = Baseline;
		Mutator(Mutated);
		const bool bRejected = !Comparator(Baseline, Mutated);
		Test.TestTrue(
			*FString::Printf(TEXT("Mutation is covered: %s"), *Path),
			bRejected);
		if (bRejected)
		{
			Audit.Cover(Path, Category, bEmptyContainerElement);
		}
		else
		{
			Audit.Skip(Path + TEXT(".MutationUnchangedOrComparatorAccepted"));
		}
		return bRejected;
	}

	template <typename TWrapper, typename TReflected, typename TAccessor,
		typename TComparator>
	void TestReflectedSubobjectDeepMutations(
		FAutomationTestBase& Test,
		FMutationCoverageAudit& Audit,
		const FString& Prefix,
		const TWrapper& Baseline,
		TAccessor Accessor,
		TComparator Comparator,
		const bool bEmptyContainerElement)
	{
		TestNestedReflectedMutationCoverage<TWrapper, TReflected>(
			Test,
			*Prefix,
			Baseline,
			Accessor,
			Comparator,
			&Audit,
			bEmptyContainerElement);
	}

	template <typename TWrapper, typename TAccessor, typename TComparator>
	void TestPlayerCardValidationDeepMutations(
		FAutomationTestBase& Test,
		FMutationCoverageAudit& Audit,
		const FString& Prefix,
		const TWrapper& InputBaseline,
		TAccessor Accessor,
		TComparator Comparator,
		const bool bEmptyContainerElement)
	{
		auto Case = [&](const TCHAR* Field, auto Mutator,
			const EMutationCoverageCategory Category =
				EMutationCoverageCategory::NonReflectedField)
		{
			ExecuteAuditedMutation(
				Test, Audit, Prefix + TEXT(".") + Field, InputBaseline,
				Comparator,
				[&](auto& Value){ Mutator(Accessor(Value)); },
				Category, bEmptyContainerElement);
		};
		Case(TEXT("bSuccess"), [](auto& Value){ Value.bSuccess = !Value.bSuccess; });
		Case(TEXT("bIsValid"), [](auto& Value){ Value.bIsValid = !Value.bIsValid; });
		Case(TEXT("ErrorCode"), [](auto& Value){ Value.ErrorCode = OtherEnumValue(Value.ErrorCode); });
		Case(TEXT("ErrorMessage"), [](auto& Value){ Value.ErrorMessage += TEXT(".Mutated"); });
		Case(TEXT("InvalidCardId"), [](auto& Value){ Value.InvalidCardId = TEXT("Mutation.InvalidCard"); });
		Case(TEXT("DuplicateCardIds.Num"), [](auto& Value)
		{
			Value.DuplicateCardIds.Add(TEXT("Mutation.Duplicate"));
		}, EMutationCoverageCategory::ContainerBehavior);

		TWrapper Singleton = InputBaseline;
		Accessor(Singleton).DuplicateCardIds = {TEXT("Canonical.Duplicate")};
		const bool bSingletonEqual = Comparator(Singleton, Singleton);
		Test.TestTrue(*FString::Printf(TEXT("%s singleton duplicate baseline"), *Prefix),
			bSingletonEqual);
		if (bSingletonEqual)
		{
			Audit.RecordEqualBaseline();
		}
		ExecuteAuditedMutation(
			Test,
			Audit,
			Prefix + TEXT(".DuplicateCardIds.EmptyElement[0]"),
			Singleton,
			Comparator,
			[&](auto& Value)
			{
				Accessor(Value).DuplicateCardIds[0] = TEXT("Mutation.Duplicate.Element");
			},
			EMutationCoverageCategory::NonReflectedField,
			bEmptyContainerElement);
	}

	template <typename TWrapper, typename TAccessor, typename TComparator>
	void TestSkillValidationDeepMutations(
		FAutomationTestBase& Test,
		FMutationCoverageAudit& Audit,
		const FString& Prefix,
		const TWrapper& InputBaseline,
		TAccessor Accessor,
		TComparator Comparator,
		const bool bEmptyContainerElement)
	{
		auto Case = [&](const TCHAR* Field, auto Mutator)
		{
			ExecuteAuditedMutation(
				Test, Audit, Prefix + TEXT(".") + Field, InputBaseline,
				Comparator,
				[&](auto& Value){ Mutator(Accessor(Value)); },
				EMutationCoverageCategory::NonReflectedField,
				bEmptyContainerElement);
		};
		Case(TEXT("bSuccess"), [](auto& Value){ Value.bSuccess = !Value.bSuccess; });
		Case(TEXT("bIsValid"), [](auto& Value){ Value.bIsValid = !Value.bIsValid; });
		Case(TEXT("ErrorCode"), [](auto& Value){ Value.ErrorCode = OtherEnumValue(Value.ErrorCode); });
		Case(TEXT("ErrorMessage"), [](auto& Value){ Value.ErrorMessage += TEXT(".Mutated"); });
		Case(TEXT("InvalidSkillId"), [](auto& Value){ Value.InvalidSkillId = TEXT("Mutation.InvalidSkill"); });
		Case(TEXT("InvalidField"), [](auto& Value){ Value.InvalidField = TEXT("Mutation.InvalidField"); });
	}

	template <typename TWrapper, typename TAccessor, typename TComparator>
	void TestSkillRuleDeepMutations(
		FAutomationTestBase& Test,
		FMutationCoverageAudit& Audit,
		const FString& Prefix,
		const TWrapper& InputBaseline,
		TAccessor Accessor,
		TComparator Comparator,
		const bool bEmptyContainerElement)
	{
		auto Case = [&](const TCHAR* Field, auto Mutator)
		{
			ExecuteAuditedMutation(
				Test, Audit, Prefix + TEXT(".") + Field, InputBaseline,
				Comparator,
				[&](auto& Value){ Mutator(Accessor(Value)); },
				EMutationCoverageCategory::NonReflectedField,
				bEmptyContainerElement);
		};
		Case(TEXT("SkillId"), [](auto& Value){ Value.SkillId = TEXT("Mutation.Skill"); });
		Case(TEXT("SkillType"), [](auto& Value){ Value.SkillType = OtherEnumValue(Value.SkillType); });
		Case(TEXT("MinTriggerActionPoint"), [](auto& Value){ ++Value.MinTriggerActionPoint; });
		Case(TEXT("MaxTriggerActionPoint"), [](auto& Value){ ++Value.MaxTriggerActionPoint; });
	}

	template <typename TWrapper, typename TAccessor, typename TComparator>
	void TestPlayerCardQueryDeepMutations(
		FAutomationTestBase& Test, FMutationCoverageAudit& Audit,
		const FString& Prefix, const TWrapper& InputBaseline,
		TAccessor Accessor, TComparator Comparator,
		const bool bEmptyContainerElement)
	{
		auto Case = [&](const TCHAR* Field, auto Mutator)
		{
			ExecuteAuditedMutation(Test, Audit, Prefix + TEXT(".") + Field,
				InputBaseline, Comparator,
				[&](auto& Value){ Mutator(Accessor(Value)); },
				EMutationCoverageCategory::NonReflectedField,
				bEmptyContainerElement);
		};
		Case(TEXT("bSuccess"), [](auto& Value){ Value.bSuccess = !Value.bSuccess; });
		Case(TEXT("bFound"), [](auto& Value){ Value.bFound = !Value.bFound; });
		Case(TEXT("ErrorCode"), [](auto& Value){ Value.ErrorCode = OtherEnumValue(Value.ErrorCode); });
		Case(TEXT("ErrorMessage"), [](auto& Value){ Value.ErrorMessage += TEXT(".Mutated"); });
		Case(TEXT("CardId"), [](auto& Value){ Value.CardId = TEXT("Mutation.CardQuery"); });
		TestReflectedSubobjectDeepMutations<TWrapper, FPlayerCardRuleSnapshot>(
			Test, Audit, Prefix + TEXT(".Snapshot"), InputBaseline,
			[Accessor](auto& Value) -> decltype(auto){ return (Accessor(Value).Snapshot); },
			Comparator, bEmptyContainerElement);
		TestPlayerCardValidationDeepMutations(
			Test, Audit, Prefix + TEXT(".ValidationResult"), InputBaseline,
			[Accessor](auto& Value) -> decltype(auto)
			{
				return (Accessor(Value).ValidationResult);
			},
			Comparator, bEmptyContainerElement);
	}

	template <typename TWrapper, typename TAccessor, typename TComparator>
	void TestCardSnapshotQueryDeepMutations(
		FAutomationTestBase& Test, FMutationCoverageAudit& Audit,
		const FString& Prefix, const TWrapper& InputBaseline,
		TAccessor Accessor, TComparator Comparator,
		const bool bEmptyContainerElement)
	{
		auto Case = [&](const TCHAR* Field, auto Mutator)
		{
			ExecuteAuditedMutation(Test, Audit, Prefix + TEXT(".") + Field,
				InputBaseline, Comparator,
				[&](auto& Value){ Mutator(Accessor(Value)); },
				EMutationCoverageCategory::NonReflectedField,
				bEmptyContainerElement);
		};
		Case(TEXT("bSuccess"), [](auto& Value){ Value.bSuccess = !Value.bSuccess; });
		Case(TEXT("ErrorCode"), [](auto& Value){ Value.ErrorCode = OtherEnumValue(Value.ErrorCode); });
		Case(TEXT("PlayerSide"), [](auto& Value){ Value.PlayerSide = OtherEnumValue(Value.PlayerSide); });
		Case(TEXT("CardId"), [](auto& Value){ Value.CardId = TEXT("Mutation.AuthorityCard"); });
		Case(TEXT("ErrorMessage"), [](auto& Value){ Value.ErrorMessage += TEXT(".Mutated"); });
		TestReflectedSubobjectDeepMutations<TWrapper, FPlayerCardRuleSnapshot>(
			Test, Audit, Prefix + TEXT(".Snapshot"), InputBaseline,
			[Accessor](auto& Value) -> decltype(auto){ return (Accessor(Value).Snapshot); },
			Comparator, bEmptyContainerElement);
		TestPlayerCardQueryDeepMutations(
			Test, Audit, Prefix + TEXT(".UnderlyingQueryResult"), InputBaseline,
			[Accessor](auto& Value) -> decltype(auto)
			{
				return (Accessor(Value).UnderlyingQueryResult);
			},
			Comparator, bEmptyContainerElement);
	}

	template <typename TWrapper, typename TAccessor, typename TComparator>
	void TestSkillRuleQueryDeepMutations(
		FAutomationTestBase& Test, FMutationCoverageAudit& Audit,
		const FString& Prefix, const TWrapper& InputBaseline,
		TAccessor Accessor, TComparator Comparator,
		const bool bEmptyContainerElement)
	{
		auto Case = [&](const TCHAR* Field, auto Mutator)
		{
			ExecuteAuditedMutation(Test, Audit, Prefix + TEXT(".") + Field,
				InputBaseline, Comparator,
				[&](auto& Value){ Mutator(Accessor(Value)); },
				EMutationCoverageCategory::NonReflectedField,
				bEmptyContainerElement);
		};
		Case(TEXT("bSuccess"), [](auto& Value){ Value.bSuccess = !Value.bSuccess; });
		Case(TEXT("bFound"), [](auto& Value){ Value.bFound = !Value.bFound; });
		Case(TEXT("ErrorCode"), [](auto& Value){ Value.ErrorCode = OtherEnumValue(Value.ErrorCode); });
		Case(TEXT("ErrorMessage"), [](auto& Value){ Value.ErrorMessage += TEXT(".Mutated"); });
		Case(TEXT("InvalidSkillId"), [](auto& Value){ Value.InvalidSkillId = TEXT("Mutation.InvalidSkill"); });
		Case(TEXT("InvalidField"), [](auto& Value){ Value.InvalidField = TEXT("Mutation.InvalidField"); });
		TestSkillRuleDeepMutations(
			Test, Audit, Prefix + TEXT(".Snapshot"), InputBaseline,
			[Accessor](auto& Value) -> decltype(auto){ return (Accessor(Value).Snapshot); },
			Comparator, bEmptyContainerElement);
		TestSkillValidationDeepMutations(
			Test, Audit, Prefix + TEXT(".ValidationResult"), InputBaseline,
			[Accessor](auto& Value) -> decltype(auto)
			{
				return (Accessor(Value).ValidationResult);
			},
			Comparator, bEmptyContainerElement);
	}

	template <typename TWrapper, typename TAccessor, typename TComparator>
	void TestSkillGlobalContextDeepMutations(
		FAutomationTestBase& Test, FMutationCoverageAudit& Audit,
		const FString& Prefix, const TWrapper& InputBaseline,
		TAccessor Accessor, TComparator Comparator,
		const bool bEmptyContainerElement)
	{
		auto Case = [&](const TCHAR* Field, auto Mutator)
		{
			ExecuteAuditedMutation(Test, Audit, Prefix + TEXT(".") + Field,
				InputBaseline, Comparator,
				[&](auto& Value){ Mutator(Accessor(Value)); },
				EMutationCoverageCategory::NonReflectedField,
				bEmptyContainerElement);
		};
		Case(TEXT("bSuccess"), [](auto& Value){ Value.bSuccess = !Value.bSuccess; });
		Case(TEXT("RequestedAttackSequence"), [](auto& Value){ ++Value.RequestedAttackSequence; });
		Case(TEXT("RequestingSide"), [](auto& Value){ Value.RequestingSide = OtherEnumValue(Value.RequestingSide); });
		Case(TEXT("ErrorCode"), [](auto& Value){ Value.ErrorCode = OtherEnumValue(Value.ErrorCode); });
		Case(TEXT("AuthoritativeAttackSequence"), [](auto& Value){ ++Value.AuthoritativeAttackSequence; });
		Case(TEXT("CurrentAttackingPlayer"), [](auto& Value){ Value.CurrentAttackingPlayer = OtherEnumValue(Value.CurrentAttackingPlayer); });
		Case(TEXT("CurrentDefendingPlayer"), [](auto& Value){ Value.CurrentDefendingPlayer = OtherEnumValue(Value.CurrentDefendingPlayer); });
		Case(TEXT("FrozenCarrierCardId"), [](auto& Value){ Value.FrozenCarrierCardId = TEXT("Mutation.Carrier"); });
		Case(TEXT("FrozenMarkerCardId"), [](auto& Value){ Value.FrozenMarkerCardId = TEXT("Mutation.Marker"); });
		Case(TEXT("MatchingFrozenCarrierPlacementCount"), [](auto& Value){ ++Value.MatchingFrozenCarrierPlacementCount; });
		Case(TEXT("MatchingFrozenMarkerPlacementCount"), [](auto& Value){ ++Value.MatchingFrozenMarkerPlacementCount; });
		Case(TEXT("ValidatedActionPoint"), [](auto& Value){ ++Value.ValidatedActionPoint; });
		Case(TEXT("ErrorMessage"), [](auto& Value){ Value.ErrorMessage += TEXT(".Mutated"); });
		TestReflectedSubobjectDeepMutations<TWrapper, FMatchPlayCurrentAttackSelectionStateValidationResult>(
			Test, Audit, Prefix + TEXT(".SelectionStateValidationResult"), InputBaseline,
			[Accessor](auto& Value) -> decltype(auto){ return (Accessor(Value).SelectionStateValidationResult); },
			Comparator, bEmptyContainerElement);
		TestReflectedSubobjectDeepMutations<TWrapper, FMatchPlayDeploymentPlacement>(
			Test, Audit, Prefix + TEXT(".FrozenCarrierPlacement"), InputBaseline,
			[Accessor](auto& Value) -> decltype(auto){ return (Accessor(Value).FrozenCarrierPlacement); },
			Comparator, bEmptyContainerElement);
		TestReflectedSubobjectDeepMutations<TWrapper, FMatchPlayDeploymentPlacement>(
			Test, Audit, Prefix + TEXT(".FrozenMarkerPlacement"), InputBaseline,
			[Accessor](auto& Value) -> decltype(auto){ return (Accessor(Value).FrozenMarkerPlacement); },
			Comparator, bEmptyContainerElement);
		TestCardSnapshotQueryDeepMutations(
			Test, Audit, Prefix + TEXT(".CarrierSnapshotQueryResult"), InputBaseline,
			[Accessor](auto& Value) -> decltype(auto){ return (Accessor(Value).CarrierSnapshotQueryResult); },
			Comparator, bEmptyContainerElement);
		TestReflectedSubobjectDeepMutations<TWrapper, FPlayerCardRuleSnapshot>(
			Test, Audit, Prefix + TEXT(".ResolvedCarrierSnapshot"), InputBaseline,
			[Accessor](auto& Value) -> decltype(auto){ return (Accessor(Value).ResolvedCarrierSnapshot); },
			Comparator, bEmptyContainerElement);
		TestSkillValidationDeepMutations(
			Test, Audit, Prefix + TEXT(".SkillRuleSetValidationResult"), InputBaseline,
			[Accessor](auto& Value) -> decltype(auto){ return (Accessor(Value).SkillRuleSetValidationResult); },
			Comparator, bEmptyContainerElement);
	}

	FNonReflectedMutationAudit TestNonReflectedInventoryMutations(
		FAutomationTestBase& Test,
		FMutationCoverageAudit& CoverageAudit)
	{
		FNonReflectedMutationAudit Audit;
		Audit.CoverageAudit = &CoverageAudit;

		FPlayerCardRuleSnapshotValidationResult CardValidation;
		CardValidation.DuplicateCardIds = {
			TEXT("Duplicate.First"),
			TEXT("Duplicate.Middle"),
			TEXT("Duplicate.Last")
		};
		Test.TestTrue(TEXT("PlayerCardValidation equal baseline"),
			ArePlayerCardValidationResultsEqual(CardValidation, CardValidation));
		++Audit.EqualBaselines;
		auto CardValidationComparator = [](const auto& Left, const auto& Right)
		{
			return ArePlayerCardValidationResultsEqual(Left, Right);
		};
		TestPlainMutation(Test, TEXT("PlayerCardValidation.bSuccess"), CardValidation,
			CardValidationComparator, [](auto& Value){ Value.bSuccess = !Value.bSuccess; }, Audit);
		TestPlainMutation(Test, TEXT("PlayerCardValidation.bIsValid"), CardValidation,
			CardValidationComparator, [](auto& Value){ Value.bIsValid = !Value.bIsValid; }, Audit);
		TestPlainMutation(Test, TEXT("PlayerCardValidation.ErrorCode"), CardValidation,
			CardValidationComparator, [](auto& Value){ Value.ErrorCode = OtherEnumValue(Value.ErrorCode); }, Audit);
		TestPlainMutation(Test, TEXT("PlayerCardValidation.ErrorMessage"), CardValidation,
			CardValidationComparator, [](auto& Value){ Value.ErrorMessage = TEXT("mutated"); }, Audit);
		TestPlainMutation(Test, TEXT("PlayerCardValidation.InvalidCardId"), CardValidation,
			CardValidationComparator, [](auto& Value){ Value.InvalidCardId = TEXT("Mutated.Card"); }, Audit);
		TestPlainMutation(Test, TEXT("PlayerCardValidation.DuplicateCardIds"), CardValidation,
			CardValidationComparator, [](auto& Value){ Value.DuplicateCardIds.RemoveAt(2); }, Audit);
		const FString DuplicatePrefix = TEXT("Manual.PlayerCardValidation.DuplicateCardIds");
		FPlayerCardRuleSnapshotValidationResult EmptyDuplicates;
		Test.TestTrue(TEXT("DuplicateCardIds empty equals empty"),
			CardValidationComparator(EmptyDuplicates, EmptyDuplicates));
		CoverageAudit.RecordEqualBaseline();
		ExecuteAuditedMutation(Test, CoverageAudit,
			DuplicatePrefix + TEXT(".Empty.Length"), EmptyDuplicates,
			CardValidationComparator,
			[](auto& Value){ Value.DuplicateCardIds.Add(TEXT("Canonical.Duplicate")); },
			EMutationCoverageCategory::ContainerBehavior);
		++Audit.ContainerCases;
		FPlayerCardRuleSnapshotValidationResult SingletonDuplicates;
		SingletonDuplicates.DuplicateCardIds = {TEXT("Canonical.Duplicate")};
		Test.TestTrue(TEXT("DuplicateCardIds singleton canonical equality"),
			CardValidationComparator(SingletonDuplicates, SingletonDuplicates));
		CoverageAudit.RecordEqualBaseline();
		ExecuteAuditedMutation(Test, CoverageAudit,
			DuplicatePrefix + TEXT(".EmptyElement[0].FName"), SingletonDuplicates,
			CardValidationComparator,
			[](auto& Value){ Value.DuplicateCardIds[0] = TEXT("Mutation.Duplicate"); },
			EMutationCoverageCategory::NonReflectedField, true);
		ExecuteAuditedMutation(Test, CoverageAudit,
			DuplicatePrefix + TEXT(".Singleton.Removal"), SingletonDuplicates,
			CardValidationComparator,
			[](auto& Value){ Value.DuplicateCardIds.Empty(); },
			EMutationCoverageCategory::ContainerBehavior);
		Audit.ContainerCases += 2;
		Test.TestTrue(TEXT("DuplicateCardIds three-element canonical equality"),
			CardValidationComparator(CardValidation, CardValidation));
		CoverageAudit.RecordEqualBaseline();
		ExecuteAuditedMutation(Test, CoverageAudit,
			DuplicatePrefix + TEXT(".Three.Removal"), CardValidation,
			CardValidationComparator,
			[](auto& Value){ Value.DuplicateCardIds.RemoveAt(2); },
			EMutationCoverageCategory::ContainerBehavior);
		ExecuteAuditedMutation(Test, CoverageAudit,
			DuplicatePrefix + TEXT(".Three.Addition"), CardValidation,
			CardValidationComparator,
			[](auto& Value){ Value.DuplicateCardIds.Add(TEXT("Duplicate.Fourth")); },
			EMutationCoverageCategory::ContainerBehavior);
		for (const int32 Index : {0, 1, 2})
		{
			ExecuteAuditedMutation(Test, CoverageAudit,
				FString::Printf(TEXT("%s.Three[%d].FName"), *DuplicatePrefix, Index),
				CardValidation, CardValidationComparator,
				[Index](auto& Value)
				{
					Value.DuplicateCardIds[Index] = FName(*FString::Printf(
						TEXT("Mutation.Duplicate.%d"), Index));
				}, EMutationCoverageCategory::NonReflectedField);
		}
		ExecuteAuditedMutation(Test, CoverageAudit,
			DuplicatePrefix + TEXT(".Three.Order"), CardValidation,
			CardValidationComparator,
			[](auto& Value){ Swap(Value.DuplicateCardIds[0], Value.DuplicateCardIds[2]); },
			EMutationCoverageCategory::ContainerBehavior);
		Audit.ContainerCases += 6;

		FPlayerCardRuleSnapshotQueryResult CardQuery;
		Test.TestTrue(TEXT("PlayerCardQuery equal baseline"),
			ArePlayerCardQueryResultsEqual(CardQuery, CardQuery));
		++Audit.EqualBaselines;
		auto CardQueryComparator = [](const auto& Left, const auto& Right)
		{
			return ArePlayerCardQueryResultsEqual(Left, Right);
		};
		TestPlainMutation(Test, TEXT("PlayerCardQuery.bSuccess"), CardQuery,
			CardQueryComparator, [](auto& Value){ Value.bSuccess = !Value.bSuccess; }, Audit);
		TestPlainMutation(Test, TEXT("PlayerCardQuery.bFound"), CardQuery,
			CardQueryComparator, [](auto& Value){ Value.bFound = !Value.bFound; }, Audit);
		TestPlainMutation(Test, TEXT("PlayerCardQuery.ErrorCode"), CardQuery,
			CardQueryComparator, [](auto& Value){ Value.ErrorCode = OtherEnumValue(Value.ErrorCode); }, Audit);
		TestPlainMutation(Test, TEXT("PlayerCardQuery.ErrorMessage"), CardQuery,
			CardQueryComparator, [](auto& Value){ Value.ErrorMessage = TEXT("mutated"); }, Audit);
		TestPlainMutation(Test, TEXT("PlayerCardQuery.CardId"), CardQuery,
			CardQueryComparator, [](auto& Value){ Value.CardId = TEXT("Mutated.Card"); }, Audit);
		TestPlainNestedReflectedMutations<
			FPlayerCardRuleSnapshotQueryResult,
			FPlayerCardRuleSnapshot>(Test, TEXT("PlayerCardQuery.Snapshot"), CardQuery,
			[](auto& Value) -> decltype(auto){ return (Value.Snapshot); },
			CardQueryComparator, Audit);
		++Audit.DeclaredFields;
		TestPlainMutation(Test, TEXT("PlayerCardQuery.ValidationResult"), CardQuery,
			CardQueryComparator,
			[](auto& Value){ Value.ValidationResult.bSuccess = !Value.ValidationResult.bSuccess; }, Audit);

		FMatchPlayCardSnapshotAuthorityQueryResult AuthorityQuery;
		Test.TestTrue(TEXT("CardSnapshotQuery equal baseline"),
			AreCardSnapshotQueryResultsEqual(AuthorityQuery, AuthorityQuery));
		++Audit.EqualBaselines;
		auto AuthorityComparator = [](const auto& Left, const auto& Right)
		{
			return AreCardSnapshotQueryResultsEqual(Left, Right);
		};
		TestPlainMutation(Test, TEXT("CardSnapshotQuery.bSuccess"), AuthorityQuery,
			AuthorityComparator, [](auto& Value){ Value.bSuccess = !Value.bSuccess; }, Audit);
		TestPlainMutation(Test, TEXT("CardSnapshotQuery.ErrorCode"), AuthorityQuery,
			AuthorityComparator, [](auto& Value){ Value.ErrorCode = OtherEnumValue(Value.ErrorCode); }, Audit);
		TestPlainMutation(Test, TEXT("CardSnapshotQuery.PlayerSide"), AuthorityQuery,
			AuthorityComparator, [](auto& Value){ Value.PlayerSide = EInitialTurnOrderPlayer::PlayerA; }, Audit);
		TestPlainMutation(Test, TEXT("CardSnapshotQuery.CardId"), AuthorityQuery,
			AuthorityComparator, [](auto& Value){ Value.CardId = TEXT("Mutated.Card"); }, Audit);
		TestPlainNestedReflectedMutations<
			FMatchPlayCardSnapshotAuthorityQueryResult,
			FPlayerCardRuleSnapshot>(Test, TEXT("CardSnapshotQuery.Snapshot"), AuthorityQuery,
			[](auto& Value) -> decltype(auto){ return (Value.Snapshot); },
			AuthorityComparator, Audit);
		++Audit.DeclaredFields;
		TestPlainMutation(Test, TEXT("CardSnapshotQuery.UnderlyingQueryResult"), AuthorityQuery,
			AuthorityComparator,
			[](auto& Value){ Value.UnderlyingQueryResult.bSuccess = !Value.UnderlyingQueryResult.bSuccess; }, Audit);
		TestPlainMutation(Test, TEXT("CardSnapshotQuery.ErrorMessage"), AuthorityQuery,
			AuthorityComparator, [](auto& Value){ Value.ErrorMessage = TEXT("mutated"); }, Audit);

		FSkillRuleSnapshotValidationResult SkillValidation;
		Test.TestTrue(TEXT("SkillRuleValidation equal baseline"),
			AreSkillRuleValidationResultsEqual(SkillValidation, SkillValidation));
		++Audit.EqualBaselines;
		auto SkillValidationComparator = [](const auto& Left, const auto& Right)
		{
			return AreSkillRuleValidationResultsEqual(Left, Right);
		};
		TestPlainMutation(Test, TEXT("SkillRuleValidation.bSuccess"), SkillValidation,
			SkillValidationComparator, [](auto& Value){ Value.bSuccess = !Value.bSuccess; }, Audit);
		TestPlainMutation(Test, TEXT("SkillRuleValidation.bIsValid"), SkillValidation,
			SkillValidationComparator, [](auto& Value){ Value.bIsValid = !Value.bIsValid; }, Audit);
		TestPlainMutation(Test, TEXT("SkillRuleValidation.ErrorCode"), SkillValidation,
			SkillValidationComparator, [](auto& Value){ Value.ErrorCode = OtherEnumValue(Value.ErrorCode); }, Audit);
		TestPlainMutation(Test, TEXT("SkillRuleValidation.ErrorMessage"), SkillValidation,
			SkillValidationComparator, [](auto& Value){ Value.ErrorMessage = TEXT("mutated"); }, Audit);
		TestPlainMutation(Test, TEXT("SkillRuleValidation.InvalidSkillId"), SkillValidation,
			SkillValidationComparator, [](auto& Value){ Value.InvalidSkillId = TEXT("Mutated.Skill"); }, Audit);
		TestPlainMutation(Test, TEXT("SkillRuleValidation.InvalidField"), SkillValidation,
			SkillValidationComparator, [](auto& Value){ Value.InvalidField = TEXT("Mutated.Field"); }, Audit);

		FSkillRuleSnapshot SkillRule;
		Test.TestTrue(TEXT("SkillRule equal baseline"), AreSkillRulesEqual(SkillRule, SkillRule));
		++Audit.EqualBaselines;
		auto SkillRuleComparator = [](const auto& Left, const auto& Right)
		{
			return AreSkillRulesEqual(Left, Right);
		};
		TestPlainMutation(Test, TEXT("SkillRule.SkillId"), SkillRule,
			SkillRuleComparator, [](auto& Value){ Value.SkillId = TEXT("Mutated.Skill"); }, Audit);
		TestPlainMutation(Test, TEXT("SkillRule.SkillType"), SkillRule,
			SkillRuleComparator, [](auto& Value){ Value.SkillType = ESkillRuleType::LongShot; }, Audit);
		TestPlainMutation(Test, TEXT("SkillRule.MinTriggerActionPoint"), SkillRule,
			SkillRuleComparator, [](auto& Value){ ++Value.MinTriggerActionPoint; }, Audit);
		TestPlainMutation(Test, TEXT("SkillRule.MaxTriggerActionPoint"), SkillRule,
			SkillRuleComparator, [](auto& Value){ ++Value.MaxTriggerActionPoint; }, Audit);

		FSkillRuleSnapshotQueryResult SkillQuery;
		Test.TestTrue(TEXT("SkillRuleQuery equal baseline"),
			AreSkillRuleQueryResultsEqual(SkillQuery, SkillQuery));
		++Audit.EqualBaselines;
		auto SkillQueryComparator = [](const auto& Left, const auto& Right)
		{
			return AreSkillRuleQueryResultsEqual(Left, Right);
		};
		TestPlainMutation(Test, TEXT("SkillRuleQuery.bSuccess"), SkillQuery,
			SkillQueryComparator, [](auto& Value){ Value.bSuccess = !Value.bSuccess; }, Audit);
		TestPlainMutation(Test, TEXT("SkillRuleQuery.bFound"), SkillQuery,
			SkillQueryComparator, [](auto& Value){ Value.bFound = !Value.bFound; }, Audit);
		TestPlainMutation(Test, TEXT("SkillRuleQuery.ErrorCode"), SkillQuery,
			SkillQueryComparator, [](auto& Value){ Value.ErrorCode = OtherEnumValue(Value.ErrorCode); }, Audit);
		TestPlainMutation(Test, TEXT("SkillRuleQuery.ErrorMessage"), SkillQuery,
			SkillQueryComparator, [](auto& Value){ Value.ErrorMessage = TEXT("mutated"); }, Audit);
		TestPlainMutation(Test, TEXT("SkillRuleQuery.InvalidSkillId"), SkillQuery,
			SkillQueryComparator, [](auto& Value){ Value.InvalidSkillId = TEXT("Mutated.Skill"); }, Audit);
		TestPlainMutation(Test, TEXT("SkillRuleQuery.InvalidField"), SkillQuery,
			SkillQueryComparator, [](auto& Value){ Value.InvalidField = TEXT("Mutated.Field"); }, Audit);
		TestPlainMutation(Test, TEXT("SkillRuleQuery.Snapshot"), SkillQuery,
			SkillQueryComparator, [](auto& Value){ Value.Snapshot.SkillId = TEXT("Mutated.Skill"); }, Audit);
		TestPlainMutation(Test, TEXT("SkillRuleQuery.ValidationResult"), SkillQuery,
			SkillQueryComparator,
			[](auto& Value){ Value.ValidationResult.bSuccess = !Value.ValidationResult.bSuccess; }, Audit);

		FMatchPlayCurrentAttackSkillSelectionGlobalContextResult Global;
		Test.TestTrue(TEXT("SkillGlobalContext equal baseline"),
			AreSkillGlobalContextResultsEqual(Global, Global));
		++Audit.EqualBaselines;
		auto GlobalComparator = [](const auto& Left, const auto& Right)
		{
			return AreSkillGlobalContextResultsEqual(Left, Right);
		};
		TestPlainMutation(Test, TEXT("SkillGlobal.bSuccess"), Global,
			GlobalComparator, [](auto& Value){ Value.bSuccess = !Value.bSuccess; }, Audit);
		TestPlainMutation(Test, TEXT("SkillGlobal.RequestedAttackSequence"), Global,
			GlobalComparator, [](auto& Value){ ++Value.RequestedAttackSequence; }, Audit);
		TestPlainMutation(Test, TEXT("SkillGlobal.RequestingSide"), Global,
			GlobalComparator, [](auto& Value){ Value.RequestingSide = EInitialTurnOrderPlayer::PlayerA; }, Audit);
		TestPlainMutation(Test, TEXT("SkillGlobal.ErrorCode"), Global,
			GlobalComparator, [](auto& Value){ Value.ErrorCode = OtherEnumValue(Value.ErrorCode); }, Audit);
		TestPlainMutation(Test, TEXT("SkillGlobal.AuthoritativeAttackSequence"), Global,
			GlobalComparator, [](auto& Value){ ++Value.AuthoritativeAttackSequence; }, Audit);
		TestPlainMutation(Test, TEXT("SkillGlobal.CurrentAttackingPlayer"), Global,
			GlobalComparator, [](auto& Value){ Value.CurrentAttackingPlayer = EInitialTurnOrderPlayer::PlayerA; }, Audit);
		TestPlainMutation(Test, TEXT("SkillGlobal.CurrentDefendingPlayer"), Global,
			GlobalComparator, [](auto& Value){ Value.CurrentDefendingPlayer = EInitialTurnOrderPlayer::PlayerB; }, Audit);
		TestPlainNestedReflectedMutations<
			FMatchPlayCurrentAttackSkillSelectionGlobalContextResult,
			FMatchPlayCurrentAttackSelectionStateValidationResult>(
				Test, TEXT("SkillGlobal.SelectionValidation"), Global,
				[](auto& Value) -> decltype(auto){ return (Value.SelectionStateValidationResult); },
				GlobalComparator, Audit);
		++Audit.DeclaredFields;
		TestPlainMutation(Test, TEXT("SkillGlobal.FrozenCarrierCardId"), Global,
			GlobalComparator, [](auto& Value){ Value.FrozenCarrierCardId = TEXT("Mutated.Carrier"); }, Audit);
		TestPlainMutation(Test, TEXT("SkillGlobal.FrozenMarkerCardId"), Global,
			GlobalComparator, [](auto& Value){ Value.FrozenMarkerCardId = TEXT("Mutated.Marker"); }, Audit);
		TestPlainMutation(Test, TEXT("SkillGlobal.MatchingFrozenCarrierPlacementCount"), Global,
			GlobalComparator, [](auto& Value){ ++Value.MatchingFrozenCarrierPlacementCount; }, Audit);
		TestPlainMutation(Test, TEXT("SkillGlobal.MatchingFrozenMarkerPlacementCount"), Global,
			GlobalComparator, [](auto& Value){ ++Value.MatchingFrozenMarkerPlacementCount; }, Audit);
		TestPlainNestedReflectedMutations<
			FMatchPlayCurrentAttackSkillSelectionGlobalContextResult,
			FMatchPlayDeploymentPlacement>(Test, TEXT("SkillGlobal.FrozenCarrierPlacement"), Global,
			[](auto& Value) -> decltype(auto){ return (Value.FrozenCarrierPlacement); },
			GlobalComparator, Audit);
		++Audit.DeclaredFields;
		TestPlainNestedReflectedMutations<
			FMatchPlayCurrentAttackSkillSelectionGlobalContextResult,
			FMatchPlayDeploymentPlacement>(Test, TEXT("SkillGlobal.FrozenMarkerPlacement"), Global,
			[](auto& Value) -> decltype(auto){ return (Value.FrozenMarkerPlacement); },
			GlobalComparator, Audit);
		++Audit.DeclaredFields;
		TestPlainMutation(Test, TEXT("SkillGlobal.CarrierSnapshotQueryResult"), Global,
			GlobalComparator,
			[](auto& Value){ Value.CarrierSnapshotQueryResult.bSuccess = !Value.CarrierSnapshotQueryResult.bSuccess; }, Audit);
		TestPlainNestedReflectedMutations<
			FMatchPlayCurrentAttackSkillSelectionGlobalContextResult,
			FPlayerCardRuleSnapshot>(Test, TEXT("SkillGlobal.ResolvedCarrierSnapshot"), Global,
			[](auto& Value) -> decltype(auto){ return (Value.ResolvedCarrierSnapshot); },
			GlobalComparator, Audit);
		++Audit.DeclaredFields;
		TestPlainMutation(Test, TEXT("SkillGlobal.SkillRuleSetValidationResult"), Global,
			GlobalComparator,
			[](auto& Value){ Value.SkillRuleSetValidationResult.bSuccess = !Value.SkillRuleSetValidationResult.bSuccess; }, Audit);
		TestPlainMutation(Test, TEXT("SkillGlobal.ValidatedActionPoint"), Global,
			GlobalComparator, [](auto& Value){ ++Value.ValidatedActionPoint; }, Audit);
		TestPlainMutation(Test, TEXT("SkillGlobal.ErrorMessage"), Global,
			GlobalComparator, [](auto& Value){ Value.ErrorMessage = TEXT("mutated"); }, Audit);

		Test.TestEqual(TEXT("Non-reflected declared field inventory"),
			Audit.DeclaredFields,
			57);
		Test.TestEqual(TEXT("Non-reflected skipped groups"),
			Audit.SkippedGroups,
			0);
		CoverageAudit.EqualBaselineCases += Audit.EqualBaselines;
		return Audit;
	}

	template <typename TWrapper, typename TAccessor, typename TCanonicalize,
		typename TDeepMutations, typename TComparator>
	int32 TestCompletionSnapshotContainerMutations(
		FAutomationTestBase& Test,
		FMutationCoverageAudit& Audit,
		const FString& Context,
		const TWrapper& InputBaseline,
		TAccessor Accessor,
		TCanonicalize Canonicalize,
		TDeepMutations DeepMutations,
		TComparator Comparator)
	{
		int32 Cases = 0;
		TWrapper EmptyBaseline = InputBaseline;
		Accessor(EmptyBaseline).DeploymentSnapshotQueryResults.Empty();
		Test.TestTrue(*FString::Printf(TEXT("%s empty baseline equality"), *Context),
			Comparator(EmptyBaseline, EmptyBaseline));
		Audit.RecordEqualBaseline();
		ExecuteAuditedMutation(Test, Audit, Context + TEXT(".Empty.Length"),
			EmptyBaseline, Comparator,
			[&](auto& Value)
			{
				auto& Items = Accessor(Value).DeploymentSnapshotQueryResults;
				Items.AddDefaulted();
				Canonicalize(Items.Last(), 0);
			}, EMutationCoverageCategory::ContainerBehavior);
		++Cases;

		TWrapper Singleton = EmptyBaseline;
		Accessor(Singleton).DeploymentSnapshotQueryResults.AddDefaulted();
		Canonicalize(Accessor(Singleton).DeploymentSnapshotQueryResults[0], 0);
		Test.TestTrue(*FString::Printf(TEXT("%s singleton canonical equality"), *Context),
			Comparator(Singleton, Singleton));
		Audit.RecordEqualBaseline();
		DeepMutations(Singleton, Context + TEXT(".EmptyElement[0]"), 0, true);
		ExecuteAuditedMutation(Test, Audit, Context + TEXT(".Singleton.Removal"),
			Singleton, Comparator,
			[&](auto& Value){ Accessor(Value).DeploymentSnapshotQueryResults.Empty(); },
			EMutationCoverageCategory::ContainerBehavior);
		++Cases;

		TWrapper Baseline = InputBaseline;
		auto& BaselineItems = Accessor(Baseline).DeploymentSnapshotQueryResults;
		BaselineItems.Empty();
		for (int32 Index = 0; Index < 3; ++Index)
		{
			const int32 Added = BaselineItems.AddDefaulted();
			Canonicalize(BaselineItems[Added], Index);
		}
		Test.TestTrue(*FString::Printf(TEXT("%s canonical equal baseline"), *Context),
			Comparator(Baseline, Baseline));
		Audit.RecordEqualBaseline();
		ExecuteAuditedMutation(Test, Audit, Context + TEXT(".Three.Removal"),
			Baseline, Comparator,
			[&](auto& Value){ Accessor(Value).DeploymentSnapshotQueryResults.RemoveAt(2); },
			EMutationCoverageCategory::ContainerBehavior);
		++Cases;
		ExecuteAuditedMutation(Test, Audit, Context + TEXT(".Three.Addition"),
			Baseline, Comparator,
			[&](auto& Value)
			{
				auto& Items = Accessor(Value).DeploymentSnapshotQueryResults;
				Items.AddDefaulted();
				Canonicalize(Items.Last(), 3);
			}, EMutationCoverageCategory::ContainerBehavior);
		++Cases;
		for (const int32 Index : {0, 1, 2})
		{
			DeepMutations(Baseline,
				FString::Printf(TEXT("%s.Three[%d]"), *Context, Index),
				Index, false);
		}
		ExecuteAuditedMutation(Test, Audit, Context + TEXT(".Three.Order"),
			Baseline, Comparator,
			[&](auto& Value)
			{
				Swap(Accessor(Value).DeploymentSnapshotQueryResults[0],
					Accessor(Value).DeploymentSnapshotQueryResults[2]);
			}, EMutationCoverageCategory::ContainerBehavior);
		++Cases;
		return Cases;
	}

	template <typename TWrapper, typename TAccessor, typename TCanonicalize,
		typename TDeepMutations, typename TComparator>
	int32 TestAvailabilityCandidateContainerMutations(
		FAutomationTestBase& Test,
		FMutationCoverageAudit& Audit,
		const FString& Context,
		const TWrapper& InputBaseline,
		TAccessor Accessor,
		TCanonicalize Canonicalize,
		TDeepMutations DeepMutations,
		TComparator Comparator)
	{
		int32 Cases = 0;
		TWrapper EmptyBaseline = InputBaseline;
		Accessor(EmptyBaseline).Candidates.Empty();
		Test.TestTrue(*FString::Printf(TEXT("%s empty baseline equality"), *Context),
			Comparator(EmptyBaseline, EmptyBaseline));
		Audit.RecordEqualBaseline();
		ExecuteAuditedMutation(Test, Audit, Context + TEXT(".Empty.Length"),
			EmptyBaseline, Comparator,
			[&](auto& Value)
			{
				auto& Candidates = Accessor(Value).Candidates;
				Candidates.AddDefaulted();
				Canonicalize(Candidates.Last(), 0);
			}, EMutationCoverageCategory::ContainerBehavior);
		++Cases;

		TWrapper Singleton = EmptyBaseline;
		Accessor(Singleton).Candidates.AddDefaulted();
		Canonicalize(Accessor(Singleton).Candidates[0], 0);
		Test.TestTrue(*FString::Printf(TEXT("%s singleton canonical equality"), *Context),
			Comparator(Singleton, Singleton));
		Audit.RecordEqualBaseline();
		DeepMutations(Singleton, Context + TEXT(".EmptyElement[0]"), 0, true);
		ExecuteAuditedMutation(Test, Audit, Context + TEXT(".Singleton.Removal"),
			Singleton, Comparator,
			[&](auto& Value){ Accessor(Value).Candidates.Empty(); },
			EMutationCoverageCategory::ContainerBehavior);
		++Cases;

		TWrapper Baseline = InputBaseline;
		auto& BaselineCandidates = Accessor(Baseline).Candidates;
		BaselineCandidates.Empty();
		for (int32 Index = 0; Index < 3; ++Index)
		{
			const int32 Added = BaselineCandidates.AddDefaulted();
			Canonicalize(BaselineCandidates[Added], Index);
		}
		Test.TestTrue(*FString::Printf(TEXT("%s canonical equal baseline"), *Context),
			Comparator(Baseline, Baseline));
		Audit.RecordEqualBaseline();
		ExecuteAuditedMutation(Test, Audit, Context + TEXT(".Three.Removal"),
			Baseline, Comparator,
			[&](auto& Value){ Accessor(Value).Candidates.RemoveAt(2); },
			EMutationCoverageCategory::ContainerBehavior);
		++Cases;
		ExecuteAuditedMutation(Test, Audit, Context + TEXT(".Three.Addition"),
			Baseline, Comparator,
			[&](auto& Value)
			{
				auto& Candidates = Accessor(Value).Candidates;
				Candidates.AddDefaulted();
				Canonicalize(Candidates.Last(), 3);
			}, EMutationCoverageCategory::ContainerBehavior);
		++Cases;
		for (const int32 Index : {0, 1, 2})
		{
			DeepMutations(Baseline,
				FString::Printf(TEXT("%s.Three[%d]"), *Context, Index),
				Index, false);
		}
		ExecuteAuditedMutation(Test, Audit, Context + TEXT(".Three.Order"),
			Baseline, Comparator,
			[&](auto& Value)
			{
				Swap(Accessor(Value).Candidates[0], Accessor(Value).Candidates[2]);
			}, EMutationCoverageCategory::ContainerBehavior);
		++Cases;
		return Cases;
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
	TestEqual(TEXT("All eleven mutations use the gate"),
		MatchPlayAuthoritativeSessionTests::CountOccurrences(
			Implementation,
			TEXT("ExecuteSerialized<")),
		11);
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
	TestEqual(TEXT("Deploy replay exact first legality error"),
		Replay.DeploymentResult.LegalityResult.ErrorCode,
		EMatchPlayOrdinaryDeploymentErrorCode
			::RequestingSideNotCurrentLegalDeploymentSide);
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
	TestTrue(TEXT("Public Session chain reaches AwaitingMarker"),
		BuildAwaitingMarkerReachabilityTrace(
			Session, TEXT("EndToEnd"), Trace));
	TestEqual(TEXT("First legal side is current attacker"),
		Trace.FirstChoice.Side, Trace.AttackingSide);
	TestEqual(TEXT("Second legal side is defender"),
		Trace.SecondChoice.Side, Trace.DefendingSide);
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

	TestTrue(TEXT("Real Carrier writer succeeds"),
		Trace.Carrier.CarrierResult.bSuccess);
	TestEqual(TEXT("Carrier command kind"),
		Trace.Carrier.RuntimeEnvelope.CommandKind,
		EMatchPlayAuthoritativeCommandKind::SubmitCarrier);
	TestAdoptedEnvelope(
		*this,
		TEXT("Carrier"),
		Trace.Carrier.RuntimeEnvelope,
		Trace.AfterSecondFinish,
		Trace.Carrier.CarrierResult.AfterState);
	TestTrue(TEXT("Finish2 to Carrier continuity"),
		AreStatesEqual(
			Trace.Carrier.RuntimeEnvelope.BeforeState,
			Trace.SecondFinish.RuntimeEnvelope.AfterState));
	TestTrue(TEXT("Final snapshot matches Carrier nested AfterState"),
		AreStatesEqual(
			Trace.FinalState,
			Trace.Carrier.CarrierResult.AfterState));
	TestAwaitingMarkerEndpoint(*this, TEXT("EndToEnd"), Trace);
	return true;
}

MATCH_PLAY_AUTHORITATIVE_SESSION_TEST(
	FMatchPlayAuthoritativeSessionCarrierDeterminismTest,
	"17.SubmitCarrierReplayIsolationAndDeterminism")

bool FMatchPlayAuthoritativeSessionCarrierDeterminismTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayAuthoritativeSessionTests;
	FMatchPlayAuthoritativeSession FullChainSessions[3];
	FReachabilityTrace FullChainTraces[3];
	for (int32 Index = 0; Index < 3; ++Index)
	{
		const FString Context = FString::Printf(
			TEXT("FullChainTrace%d"), Index);
		TestTrue(
			*FString::Printf(TEXT("%s reaches AwaitingMarker"), *Context),
			BuildAwaitingMarkerReachabilityTrace(
				FullChainSessions[Index],
				TEXT("CarrierFullChain"),
				FullChainTraces[Index]));
		const FReachabilityTrace& Trace = FullChainTraces[Index];

		TestTrue(
			*FString::Printf(TEXT("%s Initialize snapshot is saved"), *Context),
			AreStatesEqual(
				Trace.AfterInitialize,
				Trace.Initialize.RuntimeEnvelope.AfterState));
		TestTrue(
			*FString::Printf(TEXT("%s Begin snapshot is saved"), *Context),
			AreStatesEqual(
				Trace.AfterBegin,
				Trace.Begin.RuntimeEnvelope.AfterState));
		TestTrue(
			*FString::Printf(TEXT("%s attacker Deploy snapshot is saved"), *Context),
			AreStatesEqual(
				Trace.AfterFirstDeploy,
				Trace.FirstDeploy.RuntimeEnvelope.AfterState));
		TestTrue(
			*FString::Printf(TEXT("%s defender Deploy snapshot is saved"), *Context),
			AreStatesEqual(
				Trace.AfterSecondDeploy,
				Trace.SecondDeploy.RuntimeEnvelope.AfterState));
		TestTrue(
			*FString::Printf(TEXT("%s attacker Finish snapshot is saved"), *Context),
			AreStatesEqual(
				Trace.AfterFirstFinish,
				Trace.FirstFinish.RuntimeEnvelope.AfterState));
		TestTrue(
			*FString::Printf(TEXT("%s defender Finish snapshot is saved"), *Context),
			AreStatesEqual(
				Trace.AfterSecondFinish,
				Trace.SecondFinish.RuntimeEnvelope.AfterState));
		TestTrue(
			*FString::Printf(TEXT("%s Carrier snapshot is saved"), *Context),
			AreStatesEqual(
				Trace.FinalState,
				Trace.Carrier.CarrierResult.AfterState));

		TestTrue(
			*FString::Printf(TEXT("%s Begin to attacker Deploy continuity"), *Context),
			AreStatesEqual(
				Trace.FirstDeploy.RuntimeEnvelope.BeforeState,
				Trace.Begin.RuntimeEnvelope.AfterState));
		TestTrue(
			*FString::Printf(TEXT("%s attacker to defender Deploy continuity"), *Context),
			AreStatesEqual(
				Trace.SecondDeploy.RuntimeEnvelope.BeforeState,
				Trace.FirstDeploy.RuntimeEnvelope.AfterState));
		TestTrue(
			*FString::Printf(TEXT("%s defender Deploy to attacker Finish continuity"), *Context),
			AreStatesEqual(
				Trace.FirstFinish.RuntimeEnvelope.BeforeState,
				Trace.SecondDeploy.RuntimeEnvelope.AfterState));
		TestTrue(
			*FString::Printf(TEXT("%s attacker to defender Finish continuity"), *Context),
			AreStatesEqual(
				Trace.SecondFinish.RuntimeEnvelope.BeforeState,
				Trace.FirstFinish.RuntimeEnvelope.AfterState));
		TestTrue(
			*FString::Printf(TEXT("%s defender Finish to Carrier continuity"), *Context),
			AreStatesEqual(
				Trace.Carrier.RuntimeEnvelope.BeforeState,
				Trace.SecondFinish.RuntimeEnvelope.AfterState));
		TestTrue(
			*FString::Printf(TEXT("%s final State matches Carrier nested AfterState"), *Context),
			AreStatesEqual(
				Trace.FinalState,
				Trace.Carrier.CarrierResult.AfterState));
		TestAwaitingMarkerEndpoint(*this, Context, Trace);
	}

	for (int32 Index = 1; Index < 3; ++Index)
	{
		const FString Context = FString::Printf(
			TEXT("Full chain A to %c"), TCHAR('A' + Index));
		TestTrue(
			*FString::Printf(TEXT("%s Initialize typed result"), *Context),
			AreInitializeResultsEqual(
				FullChainTraces[0].Initialize,
				FullChainTraces[Index].Initialize));
		TestTrue(
			*FString::Printf(TEXT("%s Begin typed result"), *Context),
			AreAuthoritativeBeginResultsEqual(
				FullChainTraces[0].Begin,
				FullChainTraces[Index].Begin));
		TestTrue(
			*FString::Printf(TEXT("%s attacker Deploy typed result"), *Context),
			AreAuthoritativeDeployOrdinaryResultsEqual(
				FullChainTraces[0].FirstDeploy,
				FullChainTraces[Index].FirstDeploy));
		TestTrue(
			*FString::Printf(TEXT("%s defender Deploy typed result"), *Context),
			AreAuthoritativeDeployOrdinaryResultsEqual(
				FullChainTraces[0].SecondDeploy,
				FullChainTraces[Index].SecondDeploy));
		TestTrue(
			*FString::Printf(TEXT("%s attacker Finish typed result"), *Context),
			AreAuthoritativeFinishResultsEqual(
				FullChainTraces[0].FirstFinish,
				FullChainTraces[Index].FirstFinish));
		TestTrue(
			*FString::Printf(TEXT("%s defender Finish typed result"), *Context),
			AreAuthoritativeFinishResultsEqual(
				FullChainTraces[0].SecondFinish,
				FullChainTraces[Index].SecondFinish));
		TestTrue(
			*FString::Printf(TEXT("%s Carrier typed result"), *Context),
			AreAuthoritativeSubmitCarrierResultsEqual(
				FullChainTraces[0].Carrier,
				FullChainTraces[Index].Carrier));
		TestTrue(
			*FString::Printf(TEXT("%s final snapshot"), *Context),
			AreStatesEqual(
				FullChainTraces[0].FinalState,
				FullChainTraces[Index].FinalState));
		TestEqual(
			*FString::Printf(TEXT("%s trusted AttackSequence"), *Context),
			FullChainTraces[0].AttackSequence,
			FullChainTraces[Index].AttackSequence);
		TestEqual(
			*FString::Printf(TEXT("%s trusted attacking side"), *Context),
			FullChainTraces[0].AttackingSide,
			FullChainTraces[Index].AttackingSide);
		TestEqual(
			*FString::Printf(TEXT("%s trusted defending side"), *Context),
			FullChainTraces[0].DefendingSide,
			FullChainTraces[Index].DefendingSide);
		TestEqual(
			*FString::Printf(TEXT("%s trusted attacker CardId"), *Context),
			FullChainTraces[0].FirstChoice.CardId,
			FullChainTraces[Index].FirstChoice.CardId);
		TestEqual(
			*FString::Printf(TEXT("%s trusted attacker SlotId"), *Context),
			FullChainTraces[0].FirstChoice.SlotId,
			FullChainTraces[Index].FirstChoice.SlotId);
		TestEqual(
			*FString::Printf(TEXT("%s trusted defender CardId"), *Context),
			FullChainTraces[0].SecondChoice.CardId,
			FullChainTraces[Index].SecondChoice.CardId);
		TestEqual(
			*FString::Printf(TEXT("%s trusted defender SlotId"), *Context),
			FullChainTraces[0].SecondChoice.SlotId,
			FullChainTraces[Index].SecondChoice.SlotId);
		TestEqual(
			*FString::Printf(TEXT("%s trusted CarrierCardId"), *Context),
			FullChainTraces[0].CarrierCardId,
			FullChainTraces[Index].CarrierCardId);
	}

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

MATCH_PLAY_AUTHORITATIVE_SESSION_TEST(
	FMatchPlayAuthoritativeSessionFoundationBTypesAndSurfaceTest,
	"20.ResolutionFoundationBTypesAndSurface")

bool FMatchPlayAuthoritativeSessionFoundationBTypesAndSurfaceTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayAuthoritativeSessionTests;
	static_assert(std::is_same_v<
		decltype(&FMatchPlayAuthoritativeSession::SubmitMarker),
		FMatchPlayAuthoritativeSubmitMarkerResult
		(FMatchPlayAuthoritativeSession::*)(
			const FMatchPlayAuthoritativeSubmitMarkerRequest&)>);
	static_assert(std::is_same_v<
		decltype(&FMatchPlayAuthoritativeSession::ResolveNoLegalMarker),
		FMatchPlayAuthoritativeResolveNoLegalMarkerResult
		(FMatchPlayAuthoritativeSession::*)()>);
	static_assert(std::is_same_v<
		decltype(&FMatchPlayAuthoritativeSession::DeclineMarker),
		FMatchPlayAuthoritativeDeclineMarkerResult
		(FMatchPlayAuthoritativeSession::*)(
			const FMatchPlayAuthoritativeDeclineMarkerRequest&)>);
	static_assert(std::is_same_v<
		decltype(&FMatchPlayAuthoritativeSession::SubmitSkill),
		FMatchPlayAuthoritativeSubmitSkillResult
		(FMatchPlayAuthoritativeSession::*)(
			const FSkillRuleSnapshotSet&,
			const FMatchPlayAuthoritativeSubmitSkillRequest&)>);
	static_assert(std::is_same_v<
		decltype(&FMatchPlayAuthoritativeSession::ResolveNoLegalSkill),
		FMatchPlayAuthoritativeResolveNoLegalSkillResult
		(FMatchPlayAuthoritativeSession::*)(
			const FSkillRuleSnapshotSet&)>);
	static_assert(std::is_same_v<
		decltype(&FMatchPlayAuthoritativeSession::DeclineSkill),
		FMatchPlayAuthoritativeDeclineSkillResult
		(FMatchPlayAuthoritativeSession::*)(
			const FSkillRuleSnapshotSet&,
			const FMatchPlayAuthoritativeDeclineSkillRequest&)>);
	static_assert(std::is_same_v<
		decltype(FMatchPlayAuthoritativeSubmitMarkerResult::MarkerResult),
		FMatchPlayCurrentAttackMarkerSelectionWriterResult>);
	static_assert(std::is_same_v<
		decltype(FMatchPlayAuthoritativeResolveNoLegalMarkerResult
			::ResolutionResult),
		FMatchPlayResolveNoLegalMarkerResult>);
	static_assert(std::is_same_v<
		decltype(FMatchPlayAuthoritativeDeclineMarkerResult::DeclineResult),
		FMatchPlayMarkerDeclineResult>);
	static_assert(std::is_same_v<
		decltype(FMatchPlayAuthoritativeSubmitSkillResult::SkillResult),
		FMatchPlayCurrentAttackSkillSelectionWriterResult>);
	static_assert(std::is_same_v<
		decltype(FMatchPlayAuthoritativeResolveNoLegalSkillResult
			::ResolutionResult),
		FMatchPlayResolveNoLegalSkillResult>);
	static_assert(std::is_same_v<
		decltype(FMatchPlayAuthoritativeDeclineSkillResult::DeclineResult),
		FMatchPlaySkillDeclineResult>);

	TestEqual(TEXT("SubmitMarker command appended"),
		static_cast<uint8>(EMatchPlayAuthoritativeCommandKind::SubmitMarker),
		static_cast<uint8>(EMatchPlayAuthoritativeCommandKind::SubmitCarrier) + 1);
	TestEqual(TEXT("DeclineSkill is final command"),
		static_cast<uint8>(EMatchPlayAuthoritativeCommandKind::DeclineSkill),
		static_cast<uint8>(EMatchPlayAuthoritativeCommandKind::SubmitCarrier) + 6);
	FString Types;
	TestTrue(TEXT("Foundation B types source loads"), LoadProductionSource(
		TEXT("Source/FMCodex/MatchPlayRuntime/MatchPlayAuthoritativeSessionTypes.h"),
		Types));
	auto RequestContainsAttackSequence = [&Types](const TCHAR* StructName)
	{
		const int32 Begin = Types.Find(StructName);
		if (Begin == INDEX_NONE)
		{
			return true;
		}
		const int32 End = Types.Find(TEXT("};"), ESearchCase::CaseSensitive,
			ESearchDir::FromStart, Begin);
		return End == INDEX_NONE
			|| Types.Mid(Begin, End - Begin).Contains(TEXT("AttackSequence"));
	};
	for (const TCHAR* RequestType : {
		TEXT("FMatchPlayAuthoritativeSubmitMarkerRequest"),
		TEXT("FMatchPlayAuthoritativeDeclineMarkerRequest"),
		TEXT("FMatchPlayAuthoritativeSubmitSkillRequest"),
		TEXT("FMatchPlayAuthoritativeDeclineSkillRequest") })
	{
		TestFalse(*FString::Printf(TEXT("%s has no public AttackSequence"), RequestType),
			RequestContainsAttackSequence(RequestType));
	}
	return true;
}

MATCH_PLAY_AUTHORITATIVE_SESSION_TEST(
	FMatchPlayAuthoritativeSessionSubmitMarkerTest,
	"21.SubmitMarkerSuccessAndFailures")

bool FMatchPlayAuthoritativeSessionSubmitMarkerTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayAuthoritativeSessionTests;
	FMatchPlayAuthoritativeSubmitMarkerRequest EmptyRequest;
	const FMatchPlayAuthoritativeSubmitMarkerResult Uninitialized =
		FMatchPlayAuthoritativeSession().SubmitMarker(EmptyRequest);
	TestFalse(TEXT("Uninitialized marker is not accepted"),
		Uninitialized.RuntimeEnvelope.bAccepted);
	TestEqual(TEXT("Uninitialized marker runtime error"),
		Uninitialized.RuntimeEnvelope.RuntimeFailureCode,
		EMatchPlayAuthoritativeRuntimeFailureCode::NotInitialized);
	TestFalse(TEXT("Uninitialized marker domain not called"),
		Uninitialized.MarkerResult.bSuccess);

	FMatchPlayAuthoritativeSession EarlySession;
	EarlySession.InitializeMatch(MakeFoundationBInput(TEXT("MarkerEarly")));
	EarlySession.BeginOrdinaryAttack(6);
	const FMatchPlayState EarlyState = EarlySession.GetStateSnapshot();
	const FMatchPlayAuthoritativeSubmitMarkerResult Early =
		EarlySession.SubmitMarker(EmptyRequest);
	TestTrue(TEXT("Early marker reaches domain"),
		Early.RuntimeEnvelope.bAccepted);
	TestFalse(TEXT("Early marker fails domain"), Early.MarkerResult.bSuccess);
	TestEqual(TEXT("Early marker exact first error"),
		Early.MarkerResult.LegalityResult.ErrorCode,
		EMatchPlayCurrentAttackMarkerSelectionErrorCode
			::CurrentAttackNotInResolution);
	TestAcceptedDomainFailureNoAdopt(
		*this,
		TEXT("Early marker"),
		Early.RuntimeEnvelope,
		EarlyState,
		EarlySession.GetStateSnapshot());

	FMatchPlayAuthoritativeSession Session;
	FReachabilityTrace Trace;
	TestTrue(TEXT("Legal marker fixture reaches stage"),
		BuildFoundationBToAwaitingMarker(
			Session,
			TEXT("MarkerLegal"),
			true,
			{},
			Trace));
	FName MarkerCardId;
	TestTrue(TEXT("Legal marker is available"),
		FindLegalMarker(
			Session.GetStateSnapshot(),
			Trace.DefendingSide,
			MarkerCardId));

	FMatchPlayAuthoritativeSubmitMarkerRequest WrongSide =
		MakeMarkerRequest(Trace, MarkerCardId);
	WrongSide.RequestingSide = Trace.AttackingSide;
	const FMatchPlayState BeforeFailures = Session.GetStateSnapshot();
	const auto Wrong = Session.SubmitMarker(WrongSide);
	TestEqual(TEXT("Wrong-side marker exact error"),
		Wrong.MarkerResult.LegalityResult.ErrorCode,
		EMatchPlayCurrentAttackMarkerSelectionErrorCode
			::RequestingSideIsNotCurrentDefender);
	TestAcceptedDomainFailureNoAdopt(
		*this,
		TEXT("Wrong-side marker"),
		Wrong.RuntimeEnvelope,
		BeforeFailures,
		Session.GetStateSnapshot());

	FMatchPlayAuthoritativeSubmitMarkerRequest Invalid =
		MakeMarkerRequest(Trace, TEXT("Card.NotDeployed"));
	const auto InvalidResult = Session.SubmitMarker(Invalid);
	TestEqual(TEXT("Undeployed marker exact error"),
		InvalidResult.MarkerResult.LegalityResult.ErrorCode,
		EMatchPlayCurrentAttackMarkerSelectionErrorCode::MarkerNotDeployed);
	TestAcceptedDomainFailureNoAdopt(
		*this,
		TEXT("Undeployed marker"),
		InvalidResult.RuntimeEnvelope,
		BeforeFailures,
		Session.GetStateSnapshot());

	const FMatchPlayAuthoritativeSubmitMarkerResult Success =
		Session.SubmitMarker(MakeMarkerRequest(Trace, MarkerCardId));
	TestTrue(TEXT("Legal marker succeeds"), Success.MarkerResult.bSuccess);
	TestTrue(TEXT("Legal marker envelope succeeds"),
		Success.RuntimeEnvelope.bAccepted
			&& Success.RuntimeEnvelope.bDomainSuccess
			&& Success.RuntimeEnvelope.bStateAdvanced);
	TestEqual(TEXT("Legal marker command kind"),
		Success.RuntimeEnvelope.CommandKind,
		EMatchPlayAuthoritativeCommandKind::SubmitMarker);
	TestTrue(TEXT("Session adopts exact marker AfterState"),
		AreStatesEqual(
			Success.MarkerResult.AfterState,
			Session.GetStateSnapshot()));
	TestEqual(TEXT("Marker transitions to AwaitingSkill"),
		Session.GetStateSnapshot().CurrentAttack.SelectionStage,
		EMatchPlayCurrentAttackSelectionStage::AwaitingSkill);
	TestAwaitingSkillEndpoint(
		*this,
		TEXT("Valid marker post-state"),
		BeforeFailures,
		Session.GetStateSnapshot(),
		MarkerCardId);

	const FMatchPlayState AfterSuccess = Session.GetStateSnapshot();
	const auto Replay = Session.SubmitMarker(
		MakeMarkerRequest(Trace, MarkerCardId));
	TestFalse(TEXT("Marker replay fails"), Replay.MarkerResult.bSuccess);
	TestEqual(TEXT("Marker replay exact stage error"),
		Replay.MarkerResult.LegalityResult.ErrorCode,
		EMatchPlayCurrentAttackMarkerSelectionErrorCode::WrongSelectionStage);
	TestAcceptedDomainFailureNoAdopt(
		*this,
		TEXT("Marker replay"),
		Replay.RuntimeEnvelope,
		AfterSuccess,
		Session.GetStateSnapshot());

	FMatchPlayAuthoritativeSession MismatchSession;
	FReachabilityTrace MismatchTrace;
	TestTrue(TEXT("Mismatch marker fixture reaches stage"),
		BuildFoundationBToAwaitingMarker(
			MismatchSession,
			TEXT("MarkerMismatch"),
			false,
			{},
			MismatchTrace));
	const auto Mismatch = MismatchSession.SubmitMarker(
		MakeMarkerRequest(MismatchTrace, MismatchTrace.SecondChoice.CardId));
	TestEqual(TEXT("Physical mismatch exact error"),
		Mismatch.MarkerResult.LegalityResult.ErrorCode,
		EMatchPlayCurrentAttackMarkerSelectionErrorCode
			::MarkerNotInCarrierPhysicalArea);
	TestAcceptedDomainFailureNoAdopt(
		*this,
		TEXT("Physical mismatch marker"),
		Mismatch.RuntimeEnvelope,
		MismatchTrace.FinalState,
		MismatchSession.GetStateSnapshot());
	return true;
}

MATCH_PLAY_AUTHORITATIVE_SESSION_TEST(
	FMatchPlayAuthoritativeSessionSubmitMarkerDeterminismTest,
	"22.SubmitMarkerReplayIsolationDeterminism")

bool FMatchPlayAuthoritativeSessionSubmitMarkerDeterminismTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayAuthoritativeSessionTests;
	TArray<FMatchPlayAuthoritativeSubmitMarkerResult> Successes;
	TArray<FMatchPlayAuthoritativeSubmitMarkerResult> WrongSides;
	TArray<FMatchPlayAuthoritativeSubmitMarkerResult> Replays;
	TArray<FMatchPlayState> WrongSideFinals;
	TArray<FMatchPlayState> SuccessFinals;
	TArray<FMatchPlayState> ReplayFinals;
	for (int32 Index = 0; Index < 3; ++Index)
	{
		FMatchPlayAuthoritativeSession Session;
		FReachabilityTrace Trace;
		TestTrue(TEXT("Deterministic marker fixture"),
			BuildFoundationBToAwaitingMarker(
				Session,
				TEXT("MarkerDet"),
				true,
				{},
				Trace));
		FName MarkerCardId;
		FindLegalMarker(Session.GetStateSnapshot(), Trace.DefendingSide, MarkerCardId);
		auto Request = MakeMarkerRequest(Trace, MarkerCardId);
		Request.RequestingSide = Trace.AttackingSide;
		WrongSides.Add(Session.SubmitMarker(Request));
		WrongSideFinals.Add(Session.GetStateSnapshot());
		Request.RequestingSide = Trace.DefendingSide;
		Successes.Add(Session.SubmitMarker(Request));
		SuccessFinals.Add(Session.GetStateSnapshot());
		Replays.Add(Session.SubmitMarker(Request));
		ReplayFinals.Add(Session.GetStateSnapshot());
	}
	for (int32 Index = 1; Index < 3; ++Index)
	{
		TestTrue(TEXT("Marker success deterministic"),
			AreAuthoritativeSubmitMarkerResultsEqual(
				Successes[0], Successes[Index]));
		TestTrue(TEXT("Marker success envelope deterministic"),
			AreEnvelopesEqual(
				Successes[0].RuntimeEnvelope,
				Successes[Index].RuntimeEnvelope));
		TestTrue(TEXT("Marker success nested deterministic"),
			AreMarkerWriterResultsEqual(
				Successes[0].MarkerResult,
				Successes[Index].MarkerResult));
		TestTrue(TEXT("Wrong-side marker deterministic"),
			AreAuthoritativeSubmitMarkerResultsEqual(
				WrongSides[0], WrongSides[Index]));
		TestTrue(TEXT("Wrong-side marker envelope deterministic"),
			AreEnvelopesEqual(
				WrongSides[0].RuntimeEnvelope,
				WrongSides[Index].RuntimeEnvelope));
		TestTrue(TEXT("Wrong-side marker nested deterministic"),
			AreMarkerWriterResultsEqual(
				WrongSides[0].MarkerResult,
				WrongSides[Index].MarkerResult));
		TestTrue(TEXT("Marker replay deterministic"),
			AreAuthoritativeSubmitMarkerResultsEqual(
				Replays[0], Replays[Index]));
		TestTrue(TEXT("Marker replay envelope deterministic"),
			AreEnvelopesEqual(
				Replays[0].RuntimeEnvelope,
				Replays[Index].RuntimeEnvelope));
		TestTrue(TEXT("Marker replay nested deterministic"),
			AreMarkerWriterResultsEqual(
				Replays[0].MarkerResult,
				Replays[Index].MarkerResult));
		TestTrue(TEXT("Wrong-side marker final state deterministic"),
			AreStatesEqual(WrongSideFinals[0], WrongSideFinals[Index]));
		TestTrue(TEXT("Marker success final state deterministic"),
			AreStatesEqual(SuccessFinals[0], SuccessFinals[Index]));
		TestTrue(TEXT("Marker replay final state deterministic"),
			AreStatesEqual(ReplayFinals[0], ReplayFinals[Index]));
	}

	FMatchPlayAuthoritativeSession SessionA;
	FMatchPlayAuthoritativeSession SessionB;
	FReachabilityTrace TraceA;
	FReachabilityTrace TraceB;
	BuildFoundationBToAwaitingMarker(
		SessionA, TEXT("MarkerIsoA"), true, {}, TraceA);
	BuildFoundationBToAwaitingMarker(
		SessionB, TEXT("MarkerIsoB"), true, {}, TraceB);
	const FMatchPlayState BBefore = SessionB.GetStateSnapshot();
	FName MarkerA;
	FindLegalMarker(SessionA.GetStateSnapshot(), TraceA.DefendingSide, MarkerA);
	SessionA.SubmitMarker(MakeMarkerRequest(TraceA, MarkerA));
	TestTrue(TEXT("Marker session A cannot mutate session B"),
		AreStatesEqual(BBefore, SessionB.GetStateSnapshot()));
	const FMatchPlayState AAfter = SessionA.GetStateSnapshot();
	FName MarkerB;
	FindLegalMarker(SessionB.GetStateSnapshot(), TraceB.DefendingSide, MarkerB);
	SessionB.SubmitMarker(MakeMarkerRequest(TraceB, MarkerB));
	TestTrue(TEXT("Marker session B cannot mutate session A"),
		AreStatesEqual(AAfter, SessionA.GetStateSnapshot()));
	return true;
}

MATCH_PLAY_AUTHORITATIVE_SESSION_TEST(
	FMatchPlayAuthoritativeSessionResolveNoLegalMarkerTest,
	"23.ResolveNoLegalMarkerCompletion")

bool FMatchPlayAuthoritativeSessionResolveNoLegalMarkerTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayAuthoritativeSessionTests;
	const auto Uninitialized =
		FMatchPlayAuthoritativeSession().ResolveNoLegalMarker();
	TestFalse(TEXT("Uninitialized no-legal marker rejected"),
		Uninitialized.RuntimeEnvelope.bAccepted);

	FMatchPlayAuthoritativeSession NoLegalSession;
	FReachabilityTrace NoLegalTrace;
	TestTrue(TEXT("No-legal marker fixture reaches stage"),
		BuildFoundationBToAwaitingMarker(
			NoLegalSession,
			TEXT("MarkerNoLegal"),
			false,
			{},
			NoLegalTrace));
	const FMatchPlayState Before = NoLegalSession.GetStateSnapshot();
	const auto Success = NoLegalSession.ResolveNoLegalMarker();
	TestTrue(TEXT("No-legal marker resolves"),
		Success.ResolutionResult.bSuccess);
	TestFalse(TEXT("No marker was selectable"),
		Success.ResolutionResult.MarkerAvailabilityResult
			.bCanSelectAnyMarker);
	TestEqual(TEXT("No-legal marker exact source"),
		Success.ResolutionResult.Source,
		EMatchPlayMarkerNoSelectionGoalSource::ResolveNoLegalMarker);
	TestEqual(TEXT("No-legal marker exact reason"),
		Success.ResolutionResult.Reason,
		EMatchPlayMarkerNoSelectionGoalReason::NoLegalMarker);
	TestTrue(TEXT("No-legal marker completion succeeds"),
		Success.ResolutionResult.CompletionResult.bSuccess);
	TestTrue(TEXT("No-legal marker adopts exact completion AfterState"),
		AreStatesEqual(
			Success.ResolutionResult.AfterState,
			NoLegalSession.GetStateSnapshot()));
	TestFalse(TEXT("No-legal marker completes current attack"),
		NoLegalSession.GetStateSnapshot().bHasCurrentAttack);
	TestTrue(TEXT("No-legal marker advances from prior state"),
		!AreStatesEqual(Before, NoLegalSession.GetStateSnapshot()));
	TestAdoptedSuccessEnvelope(
		*this,
		TEXT("No-legal marker completion"),
		Success.RuntimeEnvelope,
		Before,
		Success.ResolutionResult.AfterState,
		NoLegalSession.GetStateSnapshot());
	TestCompletedAttackEndpoint(
		*this,
		TEXT("No-legal marker completion"),
		Success.ResolutionResult.CompletionResult,
		NoLegalSession.GetStateSnapshot());
	TestCompletionScoringContract(
		*this,
		TEXT("No-legal marker completion"),
		Before,
		Success.ResolutionResult.CompletionResult,
		NoLegalSession.GetStateSnapshot(),
		ECompletionScoringExpectation::Goal,
		NoLegalTrace.AttackingSide,
		true);
	const FMatchPlayState CompletedState = NoLegalSession.GetStateSnapshot();
	const auto Replay = NoLegalSession.ResolveNoLegalMarker();
	TestEqual(TEXT("No-legal marker replay exact first error"),
		Replay.ResolutionResult.ErrorCode,
		EMatchPlayResolveNoLegalMarkerErrorCode::NoCurrentAttack);
	TestAcceptedDomainFailureNoAdopt(
		*this,
		TEXT("No-legal marker replay"),
		Replay.RuntimeEnvelope,
		CompletedState,
		NoLegalSession.GetStateSnapshot());

	FMatchPlayAuthoritativeSession LegalSession;
	FReachabilityTrace LegalTrace;
	BuildFoundationBToAwaitingMarker(
		LegalSession, TEXT("MarkerHasLegal"), true, {}, LegalTrace);
	const FMatchPlayState LegalBefore = LegalSession.GetStateSnapshot();
	const auto Rejected = LegalSession.ResolveNoLegalMarker();
	TestEqual(TEXT("Legal marker blocks system path exactly"),
		Rejected.ResolutionResult.ErrorCode,
		EMatchPlayResolveNoLegalMarkerErrorCode::LegalMarkerExists);
	TestAcceptedDomainFailureNoAdopt(
		*this,
		TEXT("Legal marker blocks no-legal command"),
		Rejected.RuntimeEnvelope,
		LegalBefore,
		LegalSession.GetStateSnapshot());
	return true;
}

MATCH_PLAY_AUTHORITATIVE_SESSION_TEST(
	FMatchPlayAuthoritativeSessionDeclineMarkerTest,
	"24.DeclineMarkerCompletion")

bool FMatchPlayAuthoritativeSessionDeclineMarkerTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayAuthoritativeSessionTests;
	FMatchPlayAuthoritativeSession Session;
	FReachabilityTrace Trace;
	TestTrue(TEXT("Decline marker fixture reaches stage"),
		BuildFoundationBToAwaitingMarker(
			Session, TEXT("MarkerDecline"), true, {}, Trace));
	FMatchPlayAuthoritativeDeclineMarkerRequest WrongRequest;
	WrongRequest.RequestingSide = Trace.AttackingSide;
	const FMatchPlayState Before = Session.GetStateSnapshot();
	const auto Wrong = Session.DeclineMarker(WrongRequest);
	TestEqual(TEXT("Wrong-side decline marker exact error"),
		Wrong.DeclineResult.ErrorCode,
		EMatchPlayMarkerDeclineErrorCode
			::RequestingSideIsNotCurrentDefender);
	TestAcceptedDomainFailureNoAdopt(
		*this,
		TEXT("Wrong-side decline marker"),
		Wrong.RuntimeEnvelope,
		Before,
		Session.GetStateSnapshot());

	FMatchPlayAuthoritativeDeclineMarkerRequest Request;
	Request.RequestingSide = Trace.DefendingSide;
	const auto Success = Session.DeclineMarker(Request);
	TestTrue(TEXT("Legal marker decline succeeds"),
		Success.DeclineResult.bSuccess);
	TestTrue(TEXT("Marker decline saw legal marker"),
		Success.DeclineResult.MarkerAvailabilityResult.bCanSelectAnyMarker);
	TestEqual(TEXT("Marker decline exact source"),
		Success.DeclineResult.Source,
		EMatchPlayMarkerNoSelectionGoalSource::DeclineMarker);
	TestEqual(TEXT("Marker decline exact reason"),
		Success.DeclineResult.Reason,
		EMatchPlayMarkerNoSelectionGoalReason::MarkerDeclined);
	TestTrue(TEXT("Marker decline completes attack"),
		Success.DeclineResult.CompletionResult.bSuccess
			&& !Session.GetStateSnapshot().bHasCurrentAttack);
	TestAdoptedSuccessEnvelope(
		*this,
		TEXT("Marker decline completion"),
		Success.RuntimeEnvelope,
		Before,
		Success.DeclineResult.AfterState,
		Session.GetStateSnapshot());
	TestCompletedAttackEndpoint(
		*this,
		TEXT("Marker decline completion"),
		Success.DeclineResult.CompletionResult,
		Session.GetStateSnapshot());
	TestCompletionScoringContract(
		*this,
		TEXT("Marker decline completion"),
		Before,
		Success.DeclineResult.CompletionResult,
		Session.GetStateSnapshot(),
		ECompletionScoringExpectation::Goal,
		Trace.AttackingSide,
		true);

	FMatchPlayAuthoritativeSession NoLegalSession;
	FReachabilityTrace NoLegalTrace;
	BuildFoundationBToAwaitingMarker(
		NoLegalSession, TEXT("MarkerDeclineNone"), false, {}, NoLegalTrace);
	FMatchPlayAuthoritativeDeclineMarkerRequest NoLegalRequest;
	NoLegalRequest.RequestingSide = NoLegalTrace.DefendingSide;
	const auto NoLegal = NoLegalSession.DeclineMarker(NoLegalRequest);
	TestEqual(TEXT("No-legal marker cannot decline exactly"),
		NoLegal.DeclineResult.ErrorCode,
		EMatchPlayMarkerDeclineErrorCode::NoLegalMarkerToDecline);
	TestAcceptedDomainFailureNoAdopt(
		*this,
		TEXT("No-legal marker cannot decline"),
		NoLegal.RuntimeEnvelope,
		NoLegalTrace.FinalState,
		NoLegalSession.GetStateSnapshot());

	FMatchPlayAuthoritativeSession WrongStageSession;
	FReachabilityTrace WrongStageTrace;
	FMatchPlayAuthoritativeSubmitMarkerResult WrongStageMarker;
	BuildFoundationBToAwaitingSkill(
		WrongStageSession,
		TEXT("MarkerDeclineWrongStage"),
		{},
		WrongStageTrace,
		WrongStageMarker);
	FMatchPlayAuthoritativeDeclineMarkerRequest WrongStageRequest;
	WrongStageRequest.RequestingSide = WrongStageTrace.DefendingSide;
	const FMatchPlayState WrongStageBefore =
		WrongStageSession.GetStateSnapshot();
	const auto WrongStage = WrongStageSession.DeclineMarker(WrongStageRequest);
	TestEqual(TEXT("Marker decline wrong-stage exact error"),
		WrongStage.DeclineResult.ErrorCode,
		EMatchPlayMarkerDeclineErrorCode::WrongSelectionStage);
	TestAcceptedDomainFailureNoAdopt(
		*this,
		TEXT("Marker decline wrong-stage"),
		WrongStage.RuntimeEnvelope,
		WrongStageBefore,
		WrongStageSession.GetStateSnapshot());
	return true;
}

MATCH_PLAY_AUTHORITATIVE_SESSION_TEST(
	FMatchPlayAuthoritativeSessionMarkerCompletionDeterminismTest,
	"25.MarkerCompletionIsolationDeterminism")

bool FMatchPlayAuthoritativeSessionMarkerCompletionDeterminismTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayAuthoritativeSessionTests;
	TArray<FMatchPlayAuthoritativeResolveNoLegalMarkerResult> Resolves;
	TArray<FMatchPlayAuthoritativeDeclineMarkerResult> Declines;
	TArray<FMatchPlayState> ResolveFinals;
	TArray<FMatchPlayState> DeclineFinals;
	for (int32 Index = 0; Index < 3; ++Index)
	{
		FMatchPlayAuthoritativeSession ResolveSession;
		FReachabilityTrace ResolveTrace;
		BuildFoundationBToAwaitingMarker(
			ResolveSession, TEXT("MarkerCompleteResolve"), false, {}, ResolveTrace);
		const FMatchPlayState ResolveBefore = ResolveSession.GetStateSnapshot();
		Resolves.Add(ResolveSession.ResolveNoLegalMarker());
		ResolveFinals.Add(ResolveSession.GetStateSnapshot());
		TestCompletionScoringContract(
			*this,
			FString::Printf(TEXT("No-legal marker deterministic run %d"), Index),
			ResolveBefore,
			Resolves.Last().ResolutionResult.CompletionResult,
			ResolveFinals.Last(),
			ECompletionScoringExpectation::Goal,
			ResolveTrace.AttackingSide,
			false);

		FMatchPlayAuthoritativeSession DeclineSession;
		FReachabilityTrace DeclineTrace;
		BuildFoundationBToAwaitingMarker(
			DeclineSession, TEXT("MarkerCompleteDecline"), true, {}, DeclineTrace);
		FMatchPlayAuthoritativeDeclineMarkerRequest Request;
		Request.RequestingSide = DeclineTrace.DefendingSide;
		const FMatchPlayState DeclineBefore = DeclineSession.GetStateSnapshot();
		Declines.Add(DeclineSession.DeclineMarker(Request));
		DeclineFinals.Add(DeclineSession.GetStateSnapshot());
		TestCompletionScoringContract(
			*this,
			FString::Printf(TEXT("Decline marker deterministic run %d"), Index),
			DeclineBefore,
			Declines.Last().DeclineResult.CompletionResult,
			DeclineFinals.Last(),
			ECompletionScoringExpectation::Goal,
			DeclineTrace.AttackingSide,
			false);
	}
	for (int32 Index = 1; Index < 3; ++Index)
	{
		TestTrue(TEXT("No-legal marker completion deterministic"),
			AreAuthoritativeResolveNoLegalMarkerResultsEqual(
				Resolves[0], Resolves[Index]));
		TestTrue(TEXT("No-legal marker envelope deterministic"),
			AreEnvelopesEqual(
				Resolves[0].RuntimeEnvelope,
				Resolves[Index].RuntimeEnvelope));
		TestTrue(TEXT("No-legal marker nested deterministic"),
			AreReflectedValuesEqual(
				Resolves[0].ResolutionResult,
				Resolves[Index].ResolutionResult)
			&& AreMarkerAvailabilityResultsEqual(
				Resolves[0].ResolutionResult.MarkerAvailabilityResult,
				Resolves[Index].ResolutionResult.MarkerAvailabilityResult)
			&& AreCompletionResultsEqual(
				Resolves[0].ResolutionResult.CompletionResult,
				Resolves[Index].ResolutionResult.CompletionResult));
		TestTrue(TEXT("Decline marker completion deterministic"),
			AreAuthoritativeDeclineMarkerResultsEqual(
				Declines[0], Declines[Index]));
		TestTrue(TEXT("Decline marker envelope deterministic"),
			AreEnvelopesEqual(
				Declines[0].RuntimeEnvelope,
				Declines[Index].RuntimeEnvelope));
		TestTrue(TEXT("Decline marker nested deterministic"),
			AreReflectedValuesEqual(
				Declines[0].DeclineResult,
				Declines[Index].DeclineResult)
			&& AreMarkerAvailabilityResultsEqual(
				Declines[0].DeclineResult.MarkerAvailabilityResult,
				Declines[Index].DeclineResult.MarkerAvailabilityResult)
			&& AreCompletionResultsEqual(
				Declines[0].DeclineResult.CompletionResult,
				Declines[Index].DeclineResult.CompletionResult));
		TestTrue(TEXT("No-legal marker final state deterministic"),
			AreStatesEqual(ResolveFinals[0], ResolveFinals[Index]));
		TestTrue(TEXT("Decline marker final state deterministic"),
			AreStatesEqual(DeclineFinals[0], DeclineFinals[Index]));
	}
	TestFalse(TEXT("No-legal and decline provenance stay distinct"),
		Resolves[0].ResolutionResult.Source
			== static_cast<EMatchPlayMarkerNoSelectionGoalSource>(
				Declines[0].DeclineResult.Source));

	FMatchPlayAuthoritativeSession ResolveA;
	FMatchPlayAuthoritativeSession ResolveB;
	FReachabilityTrace ResolveTraceA;
	FReachabilityTrace ResolveTraceB;
	BuildFoundationBToAwaitingMarker(
		ResolveA, TEXT("MarkerResolveIsoA"), false, {}, ResolveTraceA);
	BuildFoundationBToAwaitingMarker(
		ResolveB, TEXT("MarkerResolveIsoB"), false, {}, ResolveTraceB);
	const FMatchPlayState ResolveBBefore = ResolveB.GetStateSnapshot();
	ResolveA.ResolveNoLegalMarker();
	TestTrue(TEXT("Marker NoLegal A cannot mutate B"),
		AreStatesEqual(ResolveBBefore, ResolveB.GetStateSnapshot()));
	const FMatchPlayState ResolveAAfter = ResolveA.GetStateSnapshot();
	ResolveB.ResolveNoLegalMarker();
	TestTrue(TEXT("Marker NoLegal B cannot mutate A"),
		AreStatesEqual(ResolveAAfter, ResolveA.GetStateSnapshot()));

	FMatchPlayAuthoritativeSession DeclineA;
	FMatchPlayAuthoritativeSession DeclineB;
	FReachabilityTrace DeclineTraceA;
	FReachabilityTrace DeclineTraceB;
	BuildFoundationBToAwaitingMarker(
		DeclineA, TEXT("MarkerDeclineIsoA"), true, {}, DeclineTraceA);
	BuildFoundationBToAwaitingMarker(
		DeclineB, TEXT("MarkerDeclineIsoB"), true, {}, DeclineTraceB);
	FMatchPlayAuthoritativeDeclineMarkerRequest DeclineRequestA;
	DeclineRequestA.RequestingSide = DeclineTraceA.DefendingSide;
	FMatchPlayAuthoritativeDeclineMarkerRequest DeclineRequestB;
	DeclineRequestB.RequestingSide = DeclineTraceB.DefendingSide;
	const FMatchPlayState DeclineBBefore = DeclineB.GetStateSnapshot();
	DeclineA.DeclineMarker(DeclineRequestA);
	TestTrue(TEXT("Marker decline A cannot mutate B"),
		AreStatesEqual(DeclineBBefore, DeclineB.GetStateSnapshot()));
	const FMatchPlayState DeclineAAfter = DeclineA.GetStateSnapshot();
	DeclineB.DeclineMarker(DeclineRequestB);
	TestTrue(TEXT("Marker decline B cannot mutate A"),
		AreStatesEqual(DeclineAAfter, DeclineA.GetStateSnapshot()));
	return true;
}

MATCH_PLAY_AUTHORITATIVE_SESSION_TEST(
	FMatchPlayAuthoritativeSessionSubmitSkillFiveActionTest,
	"26.SubmitSkillFiveActionMatrix")

bool FMatchPlayAuthoritativeSessionSubmitSkillFiveActionTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayAuthoritativeSessionTests;
	struct FCase
	{
		const TCHAR* Suffix;
		FName SkillId;
		ESkillRuleType SkillType;
		EMatchPlayCurrentAttackSelectionStage ExpectedStage;
	};
	const TArray<FCase> Cases = {
		{TEXT("LongShot"), TEXT("Skill.FoundationB.LongShot"),
			ESkillRuleType::LongShot,
			EMatchPlayCurrentAttackSelectionStage::AwaitingBranchIntent},
		{TEXT("CutInside"), TEXT("Skill.FoundationB.CutInside"),
			ESkillRuleType::CutInsideShot,
			EMatchPlayCurrentAttackSelectionStage::AwaitingBranchIntent},
		{TEXT("PassControl"), TEXT("Skill.FoundationB.PassControl"),
			ESkillRuleType::PassControl,
			EMatchPlayCurrentAttackSelectionStage::AwaitingRunner},
		{TEXT("Cross"), TEXT("Skill.FoundationB.Cross"),
			ESkillRuleType::Cross,
			EMatchPlayCurrentAttackSelectionStage::AwaitingRunner},
		{TEXT("ThroughBall"), TEXT("Skill.FoundationB.ThroughBall"),
			ESkillRuleType::ThroughBall,
			EMatchPlayCurrentAttackSelectionStage::AwaitingRunner}
	};

	for (const FCase& Case : Cases)
	{
		TArray<FMatchPlayAuthoritativeSubmitSkillResult> Results;
		TArray<FMatchPlayState> BeforeStates;
		TArray<FMatchPlayState> FinalStates;
		for (int32 Index = 0; Index < 3; ++Index)
		{
			FMatchPlayAuthoritativeSession Session;
			FReachabilityTrace Trace;
			FMatchPlayAuthoritativeSubmitMarkerResult Marker;
			TestTrue(*FString::Printf(TEXT("%s reaches AwaitingSkill"), Case.Suffix),
				BuildFoundationBToAwaitingSkill(
					Session,
					FString::Printf(TEXT("SkillMatrix%s"), Case.Suffix),
					{Case.SkillId},
					Trace,
					Marker));
			FMatchPlayAuthoritativeSubmitSkillRequest Request;
			Request.RequestingSide = Trace.AttackingSide;
			Request.SkillId = Case.SkillId;
			const FSkillRuleSnapshotSet Rules =
				MakeSkillRuleSet(Case.SkillId, Case.SkillType);
			BeforeStates.Add(Session.GetStateSnapshot());
			Results.Add(Session.SubmitSkill(Rules, Request));
			FinalStates.Add(Session.GetStateSnapshot());
		}
		const auto& Result = Results[0];
		TestTrue(*FString::Printf(TEXT("%s submit succeeds"), Case.Suffix),
			Result.SkillResult.bSuccess);
		TestEqual(*FString::Printf(TEXT("%s exact selected ID"), Case.Suffix),
			Result.SkillResult.SelectedSkillId,
			Case.SkillId);
		TestEqual(*FString::Printf(TEXT("%s exact action type"), Case.Suffix),
			Result.SkillResult.SelectedActionType,
			Case.SkillType);
		TestEqual(*FString::Printf(TEXT("%s exact next stage"), Case.Suffix),
			FinalStates[0].CurrentAttack.SelectionStage,
			Case.ExpectedStage);
		TestTrue(*FString::Printf(TEXT("%s exact AfterState adopted"), Case.Suffix),
			AreStatesEqual(
				Result.SkillResult.AfterState,
				FinalStates[0]));
		TestSubmittedSkillEndpoint(
			*this,
			FString::Printf(TEXT("%s full post-state"), Case.Suffix),
			BeforeStates[0],
			FinalStates[0],
			Case.SkillId,
			Case.SkillType,
			Case.ExpectedStage);
		for (int32 Index = 1; Index < 3; ++Index)
		{
			TestTrue(*FString::Printf(TEXT("%s full result deterministic"), Case.Suffix),
				AreAuthoritativeSubmitSkillResultsEqual(Results[0], Results[Index]));
			TestTrue(*FString::Printf(TEXT("%s envelope deterministic"), Case.Suffix),
				AreEnvelopesEqual(
					Results[0].RuntimeEnvelope,
					Results[Index].RuntimeEnvelope));
			TestTrue(*FString::Printf(TEXT("%s nested deterministic"), Case.Suffix),
				AreSkillWriterResultsEqual(
					Results[0].SkillResult,
					Results[Index].SkillResult));
			TestTrue(*FString::Printf(TEXT("%s final state deterministic"), Case.Suffix),
				AreStatesEqual(FinalStates[0], FinalStates[Index]));
		}
	}

	const FName BranchSkillId(TEXT("Skill.FoundationB.Isolation.LongShot"));
	const FName RunnerSkillId(TEXT("Skill.FoundationB.Isolation.Cross"));
	FMatchPlayAuthoritativeSession SessionA;
	FMatchPlayAuthoritativeSession SessionB;
	FReachabilityTrace TraceA;
	FReachabilityTrace TraceB;
	FMatchPlayAuthoritativeSubmitMarkerResult MarkerA;
	FMatchPlayAuthoritativeSubmitMarkerResult MarkerB;
	BuildFoundationBToAwaitingSkill(
		SessionA, TEXT("SkillIsolationA"), {BranchSkillId}, TraceA, MarkerA);
	BuildFoundationBToAwaitingSkill(
		SessionB, TEXT("SkillIsolationB"), {RunnerSkillId}, TraceB, MarkerB);
	FMatchPlayAuthoritativeSubmitSkillRequest RequestA;
	RequestA.RequestingSide = TraceA.AttackingSide;
	RequestA.SkillId = BranchSkillId;
	FMatchPlayAuthoritativeSubmitSkillRequest RequestB;
	RequestB.RequestingSide = TraceB.AttackingSide;
	RequestB.SkillId = RunnerSkillId;
	const FMatchPlayState BBefore = SessionB.GetStateSnapshot();
	SessionA.SubmitSkill(
		MakeSkillRuleSet(BranchSkillId, ESkillRuleType::LongShot),
		RequestA);
	TestTrue(TEXT("Branch-intent skill A cannot mutate runner skill B"),
		AreStatesEqual(BBefore, SessionB.GetStateSnapshot()));
	const FMatchPlayState AAfter = SessionA.GetStateSnapshot();
	SessionB.SubmitSkill(
		MakeSkillRuleSet(RunnerSkillId, ESkillRuleType::Cross),
		RequestB);
	TestTrue(TEXT("Runner skill B cannot mutate branch-intent skill A"),
		AreStatesEqual(AAfter, SessionA.GetStateSnapshot()));
	return true;
}

MATCH_PLAY_AUTHORITATIVE_SESSION_TEST(
	FMatchPlayAuthoritativeSessionSubmitSkillFailuresTest,
	"27.SubmitSkillFailuresReplay")

bool FMatchPlayAuthoritativeSessionSubmitSkillFailuresTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayAuthoritativeSessionTests;
	const FName SkillId(TEXT("Skill.FoundationB.Failure"));
	const FSkillRuleSnapshotSet Rules =
		MakeSkillRuleSet(SkillId, ESkillRuleType::LongShot);
	FMatchPlayAuthoritativeSubmitSkillRequest Request;
	Request.RequestingSide = EInitialTurnOrderPlayer::PlayerA;
	Request.SkillId = SkillId;
	const auto Uninitialized =
		FMatchPlayAuthoritativeSession().SubmitSkill(Rules, Request);
	TestFalse(TEXT("Uninitialized skill rejected by runtime"),
		Uninitialized.RuntimeEnvelope.bAccepted);
	TestFalse(TEXT("Uninitialized skill domain not called"),
		Uninitialized.SkillResult.bSuccess);

	FMatchPlayAuthoritativeSession EarlySession;
	FReachabilityTrace EarlyTrace;
	BuildFoundationBToAwaitingMarker(
		EarlySession, TEXT("SkillEarly"), true, {SkillId}, EarlyTrace);
	Request.RequestingSide = EarlyTrace.AttackingSide;
	const FMatchPlayState EarlyBefore = EarlySession.GetStateSnapshot();
	const auto Early = EarlySession.SubmitSkill(Rules, Request);
	TestEqual(TEXT("Pre-AwaitingSkill exact error"),
		Early.SkillResult.LegalityResult.ErrorCode,
		EMatchPlayCurrentAttackSkillSelectionErrorCode::WrongSelectionStage);
	TestAcceptedDomainFailureNoAdopt(
		*this,
		TEXT("Pre-AwaitingSkill"),
		Early.RuntimeEnvelope,
		EarlyBefore,
		EarlySession.GetStateSnapshot());

	FMatchPlayAuthoritativeSession Session;
	FReachabilityTrace Trace;
	FMatchPlayAuthoritativeSubmitMarkerResult Marker;
	TestTrue(TEXT("Skill failure fixture reaches stage"),
		BuildFoundationBToAwaitingSkill(
			Session,
			TEXT("SkillFailures"),
			{SkillId},
			Trace,
			Marker));
	Request.RequestingSide = Trace.DefendingSide;
	const FMatchPlayState Before = Session.GetStateSnapshot();
	const auto WrongSide = Session.SubmitSkill(Rules, Request);
	TestEqual(TEXT("Wrong-side skill exact error"),
		WrongSide.SkillResult.LegalityResult.ErrorCode,
		EMatchPlayCurrentAttackSkillSelectionErrorCode
			::RequestingSideIsNotCurrentAttacker);
	TestAcceptedDomainFailureNoAdopt(
		*this,
		TEXT("Wrong-side skill"),
		WrongSide.RuntimeEnvelope,
		Before,
		Session.GetStateSnapshot());

	Request.RequestingSide = Trace.AttackingSide;
	FSkillRuleSnapshotSet EmptyRules;
	const auto MissingRule = Session.SubmitSkill(EmptyRules, Request);
	TestEqual(TEXT("Unavailable rule exact error"),
		MissingRule.SkillResult.LegalityResult.ErrorCode,
		EMatchPlayCurrentAttackSkillSelectionErrorCode::SkillRuleNotFound);
	TestAcceptedDomainFailureNoAdopt(
		*this,
		TEXT("Missing skill rule"),
		MissingRule.RuntimeEnvelope,
		Before,
		Session.GetStateSnapshot());

	FSkillRuleSnapshotSet InvalidRules = Rules;
	InvalidRules.SkillRules[0].MinTriggerActionPoint = 8;
	InvalidRules.SkillRules[0].MaxTriggerActionPoint = 2;
	const auto InvalidRule = Session.SubmitSkill(InvalidRules, Request);
	TestEqual(TEXT("Invalid rule set exact error"),
		InvalidRule.SkillResult.LegalityResult.ErrorCode,
		EMatchPlayCurrentAttackSkillSelectionErrorCode::InvalidSkillRuleSet);
	TestAcceptedDomainFailureNoAdopt(
		*this,
		TEXT("Invalid skill rule set"),
		InvalidRule.RuntimeEnvelope,
		Before,
		Session.GetStateSnapshot());

	const FName UnownedSkillId(TEXT("Skill.FoundationB.Unowned"));
	FMatchPlayAuthoritativeSubmitSkillRequest UnownedRequest = Request;
	UnownedRequest.SkillId = UnownedSkillId;
	const auto Unowned = Session.SubmitSkill(
		MakeSkillRuleSet(UnownedSkillId, ESkillRuleType::LongShot),
		UnownedRequest);
	TestEqual(TEXT("Unowned skill exact error"),
		Unowned.SkillResult.LegalityResult.ErrorCode,
		EMatchPlayCurrentAttackSkillSelectionErrorCode::CarrierDoesNotOwnSkill);
	TestAcceptedDomainFailureNoAdopt(
		*this,
		TEXT("Carrier does not own skill"),
		Unowned.RuntimeEnvelope,
		Before,
		Session.GetStateSnapshot());

	FSkillRuleSnapshotSet OutsideRangeRules = Rules;
	OutsideRangeRules.SkillRules[0].MinTriggerActionPoint = 7;
	OutsideRangeRules.SkillRules[0].MaxTriggerActionPoint = 8;
	const auto OutsideRange = Session.SubmitSkill(OutsideRangeRules, Request);
	TestEqual(TEXT("Unavailable trigger range exact error"),
		OutsideRange.SkillResult.LegalityResult.ErrorCode,
		EMatchPlayCurrentAttackSkillSelectionErrorCode::ActionPointOutsideSkillRange);
	TestAcceptedDomainFailureNoAdopt(
		*this,
		TEXT("Unavailable skill trigger range"),
		OutsideRange.RuntimeEnvelope,
		Before,
		Session.GetStateSnapshot());

	for (const ESkillRuleType SupportedType : {
		ESkillRuleType::LongShot,
		ESkillRuleType::CutInsideShot,
		ESkillRuleType::Cross,
		ESkillRuleType::PassControl,
		ESkillRuleType::ThroughBall})
	{
		TestTrue(TEXT("Every snapshot-valid skill has participant requirements"),
			FMatchPlaySkillParticipantRequirementQuery::Query(SupportedType).bSuccess);
	}
	const auto UnsupportedParticipant =
		FMatchPlaySkillParticipantRequirementQuery::Query(ESkillRuleType::None);
	TestFalse(TEXT("Unsupported participant type is rejected"),
		UnsupportedParticipant.bSuccess);
	TestEqual(TEXT("Unsupported participant exact error"),
		UnsupportedParticipant.ErrorCode,
		EMatchPlaySkillParticipantRequirementErrorCode::UnsupportedSkillRuleType);

	const auto Success = Session.SubmitSkill(Rules, Request);
	TestTrue(TEXT("Valid skill succeeds after failures"),
		Success.SkillResult.bSuccess);
	const FMatchPlayState After = Session.GetStateSnapshot();
	const auto Replay = Session.SubmitSkill(Rules, Request);
	TestEqual(TEXT("Skill replay exact stage error"),
		Replay.SkillResult.LegalityResult.ErrorCode,
		EMatchPlayCurrentAttackSkillSelectionErrorCode::WrongSelectionStage);
	TestAcceptedDomainFailureNoAdopt(
		*this,
		TEXT("Skill replay"),
		Replay.RuntimeEnvelope,
		After,
		Session.GetStateSnapshot());

	TArray<FMatchPlayAuthoritativeSubmitSkillResult> WrongSideResults;
	TArray<FMatchPlayAuthoritativeSubmitSkillResult> ReplayResults;
	TArray<FMatchPlayState> WrongSideFinals;
	TArray<FMatchPlayState> ReplayFinals;
	for (int32 Index = 0; Index < 3; ++Index)
	{
		FMatchPlayAuthoritativeSession DeterministicSession;
		FReachabilityTrace DeterministicTrace;
		FMatchPlayAuthoritativeSubmitMarkerResult DeterministicMarker;
		BuildFoundationBToAwaitingSkill(
			DeterministicSession,
			TEXT("SkillFailureDeterminism"),
			{SkillId},
			DeterministicTrace,
			DeterministicMarker);
		FMatchPlayAuthoritativeSubmitSkillRequest DeterministicRequest;
		DeterministicRequest.RequestingSide = DeterministicTrace.DefendingSide;
		DeterministicRequest.SkillId = SkillId;
		WrongSideResults.Add(
			DeterministicSession.SubmitSkill(Rules, DeterministicRequest));
		WrongSideFinals.Add(DeterministicSession.GetStateSnapshot());
		DeterministicRequest.RequestingSide = DeterministicTrace.AttackingSide;
		DeterministicSession.SubmitSkill(Rules, DeterministicRequest);
		ReplayResults.Add(
			DeterministicSession.SubmitSkill(Rules, DeterministicRequest));
		ReplayFinals.Add(DeterministicSession.GetStateSnapshot());
	}
	for (int32 Index = 1; Index < 3; ++Index)
	{
		TestTrue(TEXT("Wrong-side skill full result deterministic"),
			AreAuthoritativeSubmitSkillResultsEqual(
				WrongSideResults[0], WrongSideResults[Index]));
		TestTrue(TEXT("Wrong-side skill envelope deterministic"),
			AreEnvelopesEqual(
				WrongSideResults[0].RuntimeEnvelope,
				WrongSideResults[Index].RuntimeEnvelope));
		TestTrue(TEXT("Wrong-side skill nested deterministic"),
			AreSkillWriterResultsEqual(
				WrongSideResults[0].SkillResult,
				WrongSideResults[Index].SkillResult));
		TestTrue(TEXT("Wrong-side skill final state deterministic"),
			AreStatesEqual(WrongSideFinals[0], WrongSideFinals[Index]));
		TestTrue(TEXT("Skill replay full result deterministic"),
			AreAuthoritativeSubmitSkillResultsEqual(
				ReplayResults[0], ReplayResults[Index]));
		TestTrue(TEXT("Skill replay envelope deterministic"),
			AreEnvelopesEqual(
				ReplayResults[0].RuntimeEnvelope,
				ReplayResults[Index].RuntimeEnvelope));
		TestTrue(TEXT("Skill replay nested deterministic"),
			AreSkillWriterResultsEqual(
				ReplayResults[0].SkillResult,
				ReplayResults[Index].SkillResult));
		TestTrue(TEXT("Skill replay final state deterministic"),
			AreStatesEqual(ReplayFinals[0], ReplayFinals[Index]));
	}
	return true;
}

MATCH_PLAY_AUTHORITATIVE_SESSION_TEST(
	FMatchPlayAuthoritativeSessionResolveNoLegalSkillTest,
	"28.ResolveNoLegalSkillCompletion")

bool FMatchPlayAuthoritativeSessionResolveNoLegalSkillTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayAuthoritativeSessionTests;
	FSkillRuleSnapshotSet EmptyRules;
	const auto Uninitialized =
		FMatchPlayAuthoritativeSession().ResolveNoLegalSkill(EmptyRules);
	TestFalse(TEXT("Uninitialized no-legal skill rejected"),
		Uninitialized.RuntimeEnvelope.bAccepted);

	FMatchPlayAuthoritativeSession Session;
	FReachabilityTrace Trace;
	FMatchPlayAuthoritativeSubmitMarkerResult Marker;
	TestTrue(TEXT("No-legal skill fixture reaches stage"),
		BuildFoundationBToAwaitingSkill(
			Session,
			TEXT("SkillNoLegal"),
			{},
			Trace,
			Marker));
	const FMatchPlayState Before = Session.GetStateSnapshot();
	const auto Success = Session.ResolveNoLegalSkill(EmptyRules);
	TestTrue(TEXT("No-legal skill resolves"),
		Success.ResolutionResult.bSuccess);
	TestFalse(TEXT("No skill was selectable"),
		Success.ResolutionResult.SkillAvailabilityResult.bCanSelectAnySkill);
	TestEqual(TEXT("No-legal skill exact source"),
		Success.ResolutionResult.Source,
		EMatchPlaySkillNoSelectionNoGoalSource::ResolveNoLegalSkill);
	TestEqual(TEXT("No-legal skill exact reason"),
		Success.ResolutionResult.Reason,
		EMatchPlaySkillNoSelectionNoGoalReason::NoLegalSkill);
	TestTrue(TEXT("No-legal skill exact AfterState adopted"),
		AreStatesEqual(
			Success.ResolutionResult.AfterState,
			Session.GetStateSnapshot()));
	TestFalse(TEXT("No-legal skill completes current attack"),
		Session.GetStateSnapshot().bHasCurrentAttack);
	TestAdoptedSuccessEnvelope(
		*this,
		TEXT("No-legal skill completion"),
		Success.RuntimeEnvelope,
		Before,
		Success.ResolutionResult.AfterState,
		Session.GetStateSnapshot());
	TestCompletedAttackEndpoint(
		*this,
		TEXT("No-legal skill completion"),
		Success.ResolutionResult.CompletionResult,
		Session.GetStateSnapshot());
	TestCompletionScoringContract(
		*this,
		TEXT("No-legal skill completion"),
		Before,
		Success.ResolutionResult.CompletionResult,
		Session.GetStateSnapshot(),
		ECompletionScoringExpectation::NoGoal,
		Trace.AttackingSide,
		true);
	const FMatchPlayState CompletedState = Session.GetStateSnapshot();
	const auto Replay = Session.ResolveNoLegalSkill(EmptyRules);
	TestEqual(TEXT("No-legal skill replay exact first error"),
		Replay.ResolutionResult.ErrorCode,
		EMatchPlaySkillNoSelectionNoGoalErrorCode::NoCurrentAttack);
	TestAcceptedDomainFailureNoAdopt(
		*this,
		TEXT("No-legal skill replay"),
		Replay.RuntimeEnvelope,
		CompletedState,
		Session.GetStateSnapshot());

	const FName LegalId(TEXT("Skill.FoundationB.NoLegalGuard"));
	const auto LegalRules = MakeSkillRuleSet(LegalId, ESkillRuleType::LongShot);
	FMatchPlayAuthoritativeSession LegalSession;
	FReachabilityTrace LegalTrace;
	FMatchPlayAuthoritativeSubmitMarkerResult LegalMarker;
	BuildFoundationBToAwaitingSkill(
		LegalSession,
		TEXT("SkillHasLegal"),
		{LegalId},
		LegalTrace,
		LegalMarker);
	const FMatchPlayState LegalBefore = LegalSession.GetStateSnapshot();
	const auto Rejected = LegalSession.ResolveNoLegalSkill(LegalRules);
	TestEqual(TEXT("Legal skill blocks system path exactly"),
		Rejected.ResolutionResult.ErrorCode,
		EMatchPlaySkillNoSelectionNoGoalErrorCode::LegalSkillExists);
	TestAcceptedDomainFailureNoAdopt(
		*this,
		TEXT("Legal skill blocks no-legal command"),
		Rejected.RuntimeEnvelope,
		LegalBefore,
		LegalSession.GetStateSnapshot());
	return true;
}

MATCH_PLAY_AUTHORITATIVE_SESSION_TEST(
	FMatchPlayAuthoritativeSessionDeclineSkillTest,
	"29.DeclineSkillCompletion")

bool FMatchPlayAuthoritativeSessionDeclineSkillTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayAuthoritativeSessionTests;
	const FName SkillId(TEXT("Skill.FoundationB.Decline"));
	const auto Rules = MakeSkillRuleSet(SkillId, ESkillRuleType::LongShot);
	FMatchPlayAuthoritativeSession Session;
	FReachabilityTrace Trace;
	FMatchPlayAuthoritativeSubmitMarkerResult Marker;
	TestTrue(TEXT("Skill decline fixture reaches stage"),
		BuildFoundationBToAwaitingSkill(
			Session, TEXT("SkillDecline"), {SkillId}, Trace, Marker));
	FMatchPlayAuthoritativeDeclineSkillRequest Request;
	Request.RequestingSide = Trace.DefendingSide;
	const FMatchPlayState Before = Session.GetStateSnapshot();
	const auto Wrong = Session.DeclineSkill(Rules, Request);
	TestEqual(TEXT("Wrong-side skill decline exact error"),
		Wrong.DeclineResult.ErrorCode,
		EMatchPlaySkillNoSelectionNoGoalErrorCode
			::RequestingSideIsNotCurrentAttacker);
	TestAcceptedDomainFailureNoAdopt(
		*this,
		TEXT("Wrong-side skill decline"),
		Wrong.RuntimeEnvelope,
		Before,
		Session.GetStateSnapshot());

	Request.RequestingSide = Trace.AttackingSide;
	const auto Success = Session.DeclineSkill(Rules, Request);
	TestTrue(TEXT("Legal skill decline succeeds"),
		Success.DeclineResult.bSuccess);
	TestTrue(TEXT("Skill decline saw legal skill"),
		Success.DeclineResult.SkillAvailabilityResult.bCanSelectAnySkill);
	TestEqual(TEXT("Skill decline exact source"),
		Success.DeclineResult.Source,
		EMatchPlaySkillNoSelectionNoGoalSource::DeclineSkill);
	TestEqual(TEXT("Skill decline exact reason"),
		Success.DeclineResult.Reason,
		EMatchPlaySkillNoSelectionNoGoalReason::SkillDeclined);
	TestFalse(TEXT("Skill decline completes current attack"),
		Session.GetStateSnapshot().bHasCurrentAttack);
	TestAdoptedSuccessEnvelope(
		*this,
		TEXT("Skill decline completion"),
		Success.RuntimeEnvelope,
		Before,
		Success.DeclineResult.AfterState,
		Session.GetStateSnapshot());
	TestCompletedAttackEndpoint(
		*this,
		TEXT("Skill decline completion"),
		Success.DeclineResult.CompletionResult,
		Session.GetStateSnapshot());
	TestCompletionScoringContract(
		*this,
		TEXT("Skill decline completion"),
		Before,
		Success.DeclineResult.CompletionResult,
		Session.GetStateSnapshot(),
		ECompletionScoringExpectation::NoGoal,
		Trace.AttackingSide,
		true);

	FSkillRuleSnapshotSet EmptyRules;
	FMatchPlayAuthoritativeSession NoLegalSession;
	FReachabilityTrace NoLegalTrace;
	FMatchPlayAuthoritativeSubmitMarkerResult NoLegalMarker;
	BuildFoundationBToAwaitingSkill(
		NoLegalSession,
		TEXT("SkillDeclineNone"),
		{},
		NoLegalTrace,
		NoLegalMarker);
	FMatchPlayAuthoritativeDeclineSkillRequest NoLegalRequest;
	NoLegalRequest.RequestingSide = NoLegalTrace.AttackingSide;
	const FMatchPlayState NoLegalBefore = NoLegalSession.GetStateSnapshot();
	const auto NoLegal = NoLegalSession.DeclineSkill(
		EmptyRules,
		NoLegalRequest);
	TestEqual(TEXT("No-legal skill cannot decline exactly"),
		NoLegal.DeclineResult.ErrorCode,
		EMatchPlaySkillNoSelectionNoGoalErrorCode::NoLegalSkillToDecline);
	TestAcceptedDomainFailureNoAdopt(
		*this,
		TEXT("No-legal skill cannot decline"),
		NoLegal.RuntimeEnvelope,
		NoLegalBefore,
		NoLegalSession.GetStateSnapshot());

	FMatchPlayAuthoritativeSession WrongStageSession;
	FReachabilityTrace WrongStageTrace;
	BuildFoundationBToAwaitingMarker(
		WrongStageSession,
		TEXT("SkillDeclineWrongStage"),
		true,
		{SkillId},
		WrongStageTrace);
	FMatchPlayAuthoritativeDeclineSkillRequest WrongStageRequest;
	WrongStageRequest.RequestingSide = WrongStageTrace.AttackingSide;
	const FMatchPlayState WrongStageBefore =
		WrongStageSession.GetStateSnapshot();
	const auto WrongStage = WrongStageSession.DeclineSkill(
		Rules,
		WrongStageRequest);
	TestEqual(TEXT("Skill decline wrong-stage exact error"),
		WrongStage.DeclineResult.ErrorCode,
		EMatchPlaySkillNoSelectionNoGoalErrorCode::WrongSelectionStage);
	TestAcceptedDomainFailureNoAdopt(
		*this,
		TEXT("Skill decline wrong-stage"),
		WrongStage.RuntimeEnvelope,
		WrongStageBefore,
		WrongStageSession.GetStateSnapshot());
	return true;
}

MATCH_PLAY_AUTHORITATIVE_SESSION_TEST(
	FMatchPlayAuthoritativeSessionSkillCompletionDeterminismTest,
	"30.SkillCompletionIsolationDeterminism")

bool FMatchPlayAuthoritativeSessionSkillCompletionDeterminismTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayAuthoritativeSessionTests;
	const FName SkillId(TEXT("Skill.FoundationB.CompletionDet"));
	const auto Rules = MakeSkillRuleSet(SkillId, ESkillRuleType::LongShot);
	FSkillRuleSnapshotSet EmptyRules;
	TArray<FMatchPlayAuthoritativeResolveNoLegalSkillResult> Resolves;
	TArray<FMatchPlayAuthoritativeDeclineSkillResult> Declines;
	TArray<FMatchPlayState> ResolveFinals;
	TArray<FMatchPlayState> DeclineFinals;
	for (int32 Index = 0; Index < 3; ++Index)
	{
		FMatchPlayAuthoritativeSession ResolveSession;
		FReachabilityTrace ResolveTrace;
		FMatchPlayAuthoritativeSubmitMarkerResult ResolveMarker;
		BuildFoundationBToAwaitingSkill(
			ResolveSession,
			TEXT("SkillCompleteResolve"),
			{},
			ResolveTrace,
			ResolveMarker);
		const FMatchPlayState ResolveBefore = ResolveSession.GetStateSnapshot();
		Resolves.Add(ResolveSession.ResolveNoLegalSkill(EmptyRules));
		ResolveFinals.Add(ResolveSession.GetStateSnapshot());
		TestCompletionScoringContract(
			*this,
			FString::Printf(TEXT("No-legal skill deterministic run %d"), Index),
			ResolveBefore,
			Resolves.Last().ResolutionResult.CompletionResult,
			ResolveFinals.Last(),
			ECompletionScoringExpectation::NoGoal,
			ResolveTrace.AttackingSide,
			false);

		FMatchPlayAuthoritativeSession DeclineSession;
		FReachabilityTrace DeclineTrace;
		FMatchPlayAuthoritativeSubmitMarkerResult DeclineMarkerResult;
		BuildFoundationBToAwaitingSkill(
			DeclineSession,
			TEXT("SkillCompleteDecline"),
			{SkillId},
			DeclineTrace,
			DeclineMarkerResult);
		FMatchPlayAuthoritativeDeclineSkillRequest Request;
		Request.RequestingSide = DeclineTrace.AttackingSide;
		const FMatchPlayState DeclineBefore = DeclineSession.GetStateSnapshot();
		Declines.Add(DeclineSession.DeclineSkill(Rules, Request));
		DeclineFinals.Add(DeclineSession.GetStateSnapshot());
		TestCompletionScoringContract(
			*this,
			FString::Printf(TEXT("Decline skill deterministic run %d"), Index),
			DeclineBefore,
			Declines.Last().DeclineResult.CompletionResult,
			DeclineFinals.Last(),
			ECompletionScoringExpectation::NoGoal,
			DeclineTrace.AttackingSide,
			false);
	}
	for (int32 Index = 1; Index < 3; ++Index)
	{
		TestTrue(TEXT("No-legal skill completion deterministic"),
			AreAuthoritativeResolveNoLegalSkillResultsEqual(
				Resolves[0], Resolves[Index]));
		TestTrue(TEXT("No-legal skill envelope deterministic"),
			AreEnvelopesEqual(
				Resolves[0].RuntimeEnvelope,
				Resolves[Index].RuntimeEnvelope));
		TestTrue(TEXT("No-legal skill nested deterministic"),
			AreReflectedValuesEqual(
				Resolves[0].ResolutionResult,
				Resolves[Index].ResolutionResult)
			&& AreSkillAvailabilityResultsEqual(
				Resolves[0].ResolutionResult.SkillAvailabilityResult,
				Resolves[Index].ResolutionResult.SkillAvailabilityResult)
			&& AreCompletionResultsEqual(
				Resolves[0].ResolutionResult.CompletionResult,
				Resolves[Index].ResolutionResult.CompletionResult));
		TestTrue(TEXT("Decline skill completion deterministic"),
			AreAuthoritativeDeclineSkillResultsEqual(
				Declines[0], Declines[Index]));
		TestTrue(TEXT("Decline skill envelope deterministic"),
			AreEnvelopesEqual(
				Declines[0].RuntimeEnvelope,
				Declines[Index].RuntimeEnvelope));
		TestTrue(TEXT("Decline skill nested deterministic"),
			AreReflectedValuesEqual(
				Declines[0].DeclineResult,
				Declines[Index].DeclineResult)
			&& AreSkillAvailabilityResultsEqual(
				Declines[0].DeclineResult.SkillAvailabilityResult,
				Declines[Index].DeclineResult.SkillAvailabilityResult)
			&& AreCompletionResultsEqual(
				Declines[0].DeclineResult.CompletionResult,
				Declines[Index].DeclineResult.CompletionResult));
		TestTrue(TEXT("No-legal skill final state deterministic"),
			AreStatesEqual(ResolveFinals[0], ResolveFinals[Index]));
		TestTrue(TEXT("Decline skill final state deterministic"),
			AreStatesEqual(DeclineFinals[0], DeclineFinals[Index]));
	}

	FMatchPlayAuthoritativeSession SessionA;
	FMatchPlayAuthoritativeSession SessionB;
	FReachabilityTrace TraceA;
	FReachabilityTrace TraceB;
	FMatchPlayAuthoritativeSubmitMarkerResult MarkerA;
	FMatchPlayAuthoritativeSubmitMarkerResult MarkerB;
	BuildFoundationBToAwaitingSkill(
		SessionA, TEXT("SkillIsoA"), {}, TraceA, MarkerA);
	BuildFoundationBToAwaitingSkill(
		SessionB, TEXT("SkillIsoB"), {}, TraceB, MarkerB);
	const FMatchPlayState BBefore = SessionB.GetStateSnapshot();
	SessionA.ResolveNoLegalSkill(EmptyRules);
	TestTrue(TEXT("Skill completion session isolation"),
		AreStatesEqual(BBefore, SessionB.GetStateSnapshot()));
	const FMatchPlayState AAfter = SessionA.GetStateSnapshot();
	SessionB.ResolveNoLegalSkill(EmptyRules);
	TestTrue(TEXT("Skill NoLegal B cannot mutate A"),
		AreStatesEqual(AAfter, SessionA.GetStateSnapshot()));

	FMatchPlayAuthoritativeSession DeclineA;
	FMatchPlayAuthoritativeSession DeclineB;
	FReachabilityTrace DeclineTraceA;
	FReachabilityTrace DeclineTraceB;
	FMatchPlayAuthoritativeSubmitMarkerResult DeclineMarkerA;
	FMatchPlayAuthoritativeSubmitMarkerResult DeclineMarkerB;
	BuildFoundationBToAwaitingSkill(
		DeclineA,
		TEXT("SkillDeclineIsoA"),
		{SkillId},
		DeclineTraceA,
		DeclineMarkerA);
	BuildFoundationBToAwaitingSkill(
		DeclineB,
		TEXT("SkillDeclineIsoB"),
		{SkillId},
		DeclineTraceB,
		DeclineMarkerB);
	FMatchPlayAuthoritativeDeclineSkillRequest DeclineRequestA;
	DeclineRequestA.RequestingSide = DeclineTraceA.AttackingSide;
	FMatchPlayAuthoritativeDeclineSkillRequest DeclineRequestB;
	DeclineRequestB.RequestingSide = DeclineTraceB.AttackingSide;
	const FMatchPlayState DeclineBBefore = DeclineB.GetStateSnapshot();
	DeclineA.DeclineSkill(Rules, DeclineRequestA);
	TestTrue(TEXT("Skill decline A cannot mutate B"),
		AreStatesEqual(DeclineBBefore, DeclineB.GetStateSnapshot()));
	const FMatchPlayState DeclineAAfter = DeclineA.GetStateSnapshot();
	DeclineB.DeclineSkill(Rules, DeclineRequestB);
	TestTrue(TEXT("Skill decline B cannot mutate A"),
		AreStatesEqual(DeclineAAfter, DeclineA.GetStateSnapshot()));
	return true;
}

MATCH_PLAY_AUTHORITATIVE_SESSION_TEST(
	FMatchPlayAuthoritativeSessionFoundationBComparatorCoverageTest,
	"31.FoundationBComparatorCoverage")

bool FMatchPlayAuthoritativeSessionFoundationBComparatorCoverageTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayAuthoritativeSessionTests;
	FMutationCoverageAudit CoverageAudit;

	FMatchPlayAuthoritativeSession MarkerSession;
	FReachabilityTrace MarkerTrace;
	BuildFoundationBToAwaitingMarker(
		MarkerSession, TEXT("ComparatorMarker"), true, {}, MarkerTrace);
	FName MarkerCardId;
	FindLegalMarker(
		MarkerSession.GetStateSnapshot(),
		MarkerTrace.DefendingSide,
		MarkerCardId);
	const auto Marker = MarkerSession.SubmitMarker(
		MakeMarkerRequest(MarkerTrace, MarkerCardId));
	TestTrue(TEXT("Marker comparator accepts equal baseline"),
		AreAuthoritativeSubmitMarkerResultsEqual(Marker, Marker));
	CoverageAudit.RecordEqualBaseline();
	TestEnvelopeMutationCoverage(
		*this, Marker.RuntimeEnvelope, &CoverageAudit, TEXT("SubmitMarker.Envelope"));
	const FReflectedMutationAudit MarkerMutationAudit =
		TestNestedReflectedMutationCoverage<
			FMatchPlayAuthoritativeSubmitMarkerResult,
			FMatchPlayCurrentAttackMarkerSelectionWriterResult>(
				*this,
				TEXT("SubmitMarker"),
				Marker,
				[](auto& Value) -> decltype(auto)
				{
					return (Value.MarkerResult);
				},
				[](const auto& Left, const auto& Right)
				{
					return AreAuthoritativeSubmitMarkerResultsEqual(
						Left, Right);
				},
				&CoverageAudit);

	FMatchPlayAuthoritativeSession MarkerResolveSession;
	FReachabilityTrace MarkerResolveTrace;
	BuildFoundationBToAwaitingMarker(
		MarkerResolveSession,
		TEXT("ComparatorMarkerResolve"),
		false,
		{},
		MarkerResolveTrace);
	const auto MarkerResolve =
		MarkerResolveSession.ResolveNoLegalMarker();
	TestTrue(TEXT("Resolve marker comparator accepts equal baseline"),
		AreAuthoritativeResolveNoLegalMarkerResultsEqual(
			MarkerResolve,
			MarkerResolve));
	CoverageAudit.RecordEqualBaseline();
	TestEnvelopeMutationCoverage(*this, MarkerResolve.RuntimeEnvelope,
		&CoverageAudit, TEXT("ResolveNoLegalMarker.Envelope"));
	const FReflectedMutationAudit ResolveMarkerMutationAudit =
		TestNestedReflectedMutationCoverage<
			FMatchPlayAuthoritativeResolveNoLegalMarkerResult,
			FMatchPlayResolveNoLegalMarkerResult>(
				*this,
				TEXT("ResolveNoLegalMarker"),
				MarkerResolve,
				[](auto& Value) -> decltype(auto)
				{
					return (Value.ResolutionResult);
				},
				[](const auto& Left, const auto& Right)
				{
					return AreAuthoritativeResolveNoLegalMarkerResultsEqual(
						Left, Right);
				},
				&CoverageAudit);

	FMatchPlayAuthoritativeSession MarkerDeclineSession;
	FReachabilityTrace MarkerDeclineTrace;
	BuildFoundationBToAwaitingMarker(
		MarkerDeclineSession,
		TEXT("ComparatorMarkerDecline"),
		true,
		{},
		MarkerDeclineTrace);
	FMatchPlayAuthoritativeDeclineMarkerRequest MarkerDeclineRequest;
	MarkerDeclineRequest.RequestingSide = MarkerDeclineTrace.DefendingSide;
	const auto MarkerDecline =
		MarkerDeclineSession.DeclineMarker(MarkerDeclineRequest);
	TestTrue(TEXT("Decline marker comparator accepts equal baseline"),
		AreAuthoritativeDeclineMarkerResultsEqual(
			MarkerDecline,
			MarkerDecline));
	CoverageAudit.RecordEqualBaseline();
	TestEnvelopeMutationCoverage(*this, MarkerDecline.RuntimeEnvelope,
		&CoverageAudit, TEXT("DeclineMarker.Envelope"));
	const FReflectedMutationAudit DeclineMarkerMutationAudit =
		TestNestedReflectedMutationCoverage<
			FMatchPlayAuthoritativeDeclineMarkerResult,
			FMatchPlayMarkerDeclineResult>(
				*this,
				TEXT("DeclineMarker"),
				MarkerDecline,
				[](auto& Value) -> decltype(auto)
				{
					return (Value.DeclineResult);
				},
				[](const auto& Left, const auto& Right)
				{
					return AreAuthoritativeDeclineMarkerResultsEqual(
						Left, Right);
				},
				&CoverageAudit);

	const FName SkillId(TEXT("Skill.FoundationB.Comparator"));
	const auto Rules = MakeSkillRuleSet(SkillId, ESkillRuleType::LongShot);
	FMatchPlayAuthoritativeSession SkillSession;
	FReachabilityTrace SkillTrace;
	FMatchPlayAuthoritativeSubmitMarkerResult SkillMarker;
	BuildFoundationBToAwaitingSkill(
		SkillSession,
		TEXT("ComparatorSkill"),
		{SkillId},
		SkillTrace,
		SkillMarker);
	FMatchPlayAuthoritativeSubmitSkillRequest SkillRequest;
	SkillRequest.RequestingSide = SkillTrace.AttackingSide;
	SkillRequest.SkillId = SkillId;
	const auto Skill = SkillSession.SubmitSkill(Rules, SkillRequest);
	TestTrue(TEXT("Skill comparator accepts equal baseline"),
		AreAuthoritativeSubmitSkillResultsEqual(Skill, Skill));
	CoverageAudit.RecordEqualBaseline();
	TestEnvelopeMutationCoverage(
		*this, Skill.RuntimeEnvelope, &CoverageAudit, TEXT("SubmitSkill.Envelope"));
	const FReflectedMutationAudit SkillMutationAudit =
		TestNestedReflectedMutationCoverage<
			FMatchPlayAuthoritativeSubmitSkillResult,
			FMatchPlayCurrentAttackSkillSelectionWriterResult>(
				*this,
				TEXT("SubmitSkill"),
				Skill,
				[](auto& Value) -> decltype(auto)
				{
					return (Value.SkillResult);
				},
				[](const auto& Left, const auto& Right)
				{
					return AreAuthoritativeSubmitSkillResultsEqual(
						Left, Right);
				},
				&CoverageAudit);

	FSkillRuleSnapshotSet EmptyRules;
	FMatchPlayAuthoritativeSession SkillResolveSession;
	FReachabilityTrace SkillResolveTrace;
	FMatchPlayAuthoritativeSubmitMarkerResult SkillResolveMarker;
	BuildFoundationBToAwaitingSkill(
		SkillResolveSession,
		TEXT("ComparatorSkillResolve"),
		{},
		SkillResolveTrace,
		SkillResolveMarker);
	const auto SkillResolve =
		SkillResolveSession.ResolveNoLegalSkill(EmptyRules);
	TestTrue(TEXT("Resolve skill comparator accepts equal baseline"),
		AreAuthoritativeResolveNoLegalSkillResultsEqual(
			SkillResolve,
			SkillResolve));
	CoverageAudit.RecordEqualBaseline();
	TestEnvelopeMutationCoverage(*this, SkillResolve.RuntimeEnvelope,
		&CoverageAudit, TEXT("ResolveNoLegalSkill.Envelope"));
	const FReflectedMutationAudit ResolveSkillMutationAudit =
		TestNestedReflectedMutationCoverage<
			FMatchPlayAuthoritativeResolveNoLegalSkillResult,
			FMatchPlayResolveNoLegalSkillResult>(
				*this,
				TEXT("ResolveNoLegalSkill"),
				SkillResolve,
				[](auto& Value) -> decltype(auto)
				{
					return (Value.ResolutionResult);
				},
				[](const auto& Left, const auto& Right)
				{
					return AreAuthoritativeResolveNoLegalSkillResultsEqual(
						Left, Right);
				},
				&CoverageAudit);

	FMatchPlayAuthoritativeSession SkillDeclineSession;
	FReachabilityTrace SkillDeclineTrace;
	FMatchPlayAuthoritativeSubmitMarkerResult SkillDeclineMarker;
	BuildFoundationBToAwaitingSkill(
		SkillDeclineSession,
		TEXT("ComparatorSkillDecline"),
		{SkillId},
		SkillDeclineTrace,
		SkillDeclineMarker);
	FMatchPlayAuthoritativeDeclineSkillRequest SkillDeclineRequest;
	SkillDeclineRequest.RequestingSide = SkillDeclineTrace.AttackingSide;
	const auto SkillDecline =
		SkillDeclineSession.DeclineSkill(Rules, SkillDeclineRequest);
	TestTrue(TEXT("Decline skill comparator accepts equal baseline"),
		AreAuthoritativeDeclineSkillResultsEqual(
			SkillDecline,
			SkillDecline));
	CoverageAudit.RecordEqualBaseline();
	TestEnvelopeMutationCoverage(*this, SkillDecline.RuntimeEnvelope,
		&CoverageAudit, TEXT("DeclineSkill.Envelope"));
	const FReflectedMutationAudit DeclineSkillMutationAudit =
		TestNestedReflectedMutationCoverage<
			FMatchPlayAuthoritativeDeclineSkillResult,
			FMatchPlaySkillDeclineResult>(
				*this,
				TEXT("DeclineSkill"),
				SkillDecline,
				[](auto& Value) -> decltype(auto)
				{
					return (Value.DeclineResult);
				},
				[](const auto& Left, const auto& Right)
				{
					return AreAuthoritativeDeclineSkillResultsEqual(
						Left, Right);
				},
				&CoverageAudit);

	int32 DeepContainerCases = 0;
	DeepContainerCases += TestAvailabilityCandidateContainerMutations(
		*this,
		CoverageAudit,
		TEXT("ResolveNoLegalMarker.ResolutionResult.MarkerAvailabilityResult.Candidates"),
		MarkerResolve,
		[](auto& Value) -> decltype(auto)
		{
			return (Value.ResolutionResult.MarkerAvailabilityResult);
		},
		[](auto& Candidate, const int32 Index)
		{
			Candidate.MarkerCardId = FName(*FString::Printf(
				TEXT("Canonical.Marker.%d"), Index));
		},
		[&](const auto& Baseline, const FString& Prefix,
			const int32 Index, const bool bEmpty)
		{
			auto Comparator = [](const auto& Left, const auto& Right)
			{
				return AreAuthoritativeResolveNoLegalMarkerResultsEqual(Left, Right);
			};
			TestNestedReflectedMutationCoverage<
				FMatchPlayAuthoritativeResolveNoLegalMarkerResult,
				FMatchPlayCurrentAttackMarkerSelectionCandidateAvailability>(
					*this, *Prefix, Baseline,
					[Index](auto& Value) -> decltype(auto)
					{
						return (Value.ResolutionResult.MarkerAvailabilityResult.Candidates[Index]);
					}, Comparator, &CoverageAudit, bEmpty);
			TestCardSnapshotQueryDeepMutations(
				*this, CoverageAudit, Prefix + TEXT(".LegalityResult.MarkerSnapshotQueryResult"),
				Baseline,
				[Index](auto& Value) -> decltype(auto)
				{
					return (Value.ResolutionResult.MarkerAvailabilityResult.Candidates[Index]
						.LegalityResult.MarkerSnapshotQueryResult);
				}, Comparator, bEmpty);
		},
		[](const auto& Left, const auto& Right)
		{
			return AreAuthoritativeResolveNoLegalMarkerResultsEqual(Left, Right);
		});
	DeepContainerCases += TestAvailabilityCandidateContainerMutations(
		*this,
		CoverageAudit,
		TEXT("DeclineMarker.DeclineResult.MarkerAvailabilityResult.Candidates"),
		MarkerDecline,
		[](auto& Value) -> decltype(auto)
		{
			return (Value.DeclineResult.MarkerAvailabilityResult);
		},
		[](auto& Candidate, const int32 Index)
		{
			Candidate.MarkerCardId = FName(*FString::Printf(
				TEXT("Canonical.DeclineMarker.%d"), Index));
		},
		[&](const auto& Baseline, const FString& Prefix,
			const int32 Index, const bool bEmpty)
		{
			auto Comparator = [](const auto& Left, const auto& Right)
			{
				return AreAuthoritativeDeclineMarkerResultsEqual(Left, Right);
			};
			TestNestedReflectedMutationCoverage<
				FMatchPlayAuthoritativeDeclineMarkerResult,
				FMatchPlayCurrentAttackMarkerSelectionCandidateAvailability>(
					*this, *Prefix, Baseline,
					[Index](auto& Value) -> decltype(auto)
					{
						return (Value.DeclineResult.MarkerAvailabilityResult.Candidates[Index]);
					}, Comparator, &CoverageAudit, bEmpty);
			TestCardSnapshotQueryDeepMutations(
				*this, CoverageAudit, Prefix + TEXT(".LegalityResult.MarkerSnapshotQueryResult"),
				Baseline,
				[Index](auto& Value) -> decltype(auto)
				{
					return (Value.DeclineResult.MarkerAvailabilityResult.Candidates[Index]
						.LegalityResult.MarkerSnapshotQueryResult);
				}, Comparator, bEmpty);
		},
		[](const auto& Left, const auto& Right)
		{
			return AreAuthoritativeDeclineMarkerResultsEqual(Left, Right);
		});
	DeepContainerCases += TestAvailabilityCandidateContainerMutations(
		*this,
		CoverageAudit,
		TEXT("ResolveNoLegalSkill.ResolutionResult.SkillAvailabilityResult.Candidates"),
		SkillResolve,
		[](auto& Value) -> decltype(auto)
		{
			return (Value.ResolutionResult.SkillAvailabilityResult);
		},
		[](auto& Candidate, const int32 Index)
		{
			Candidate.SkillId = FName(*FString::Printf(
				TEXT("Canonical.Skill.%d"), Index));
		},
		[&](const auto& Baseline, const FString& Prefix,
			const int32 Index, const bool bEmpty)
		{
			auto Comparator = [](const auto& Left, const auto& Right)
			{
				return AreAuthoritativeResolveNoLegalSkillResultsEqual(Left, Right);
			};
			TestNestedReflectedMutationCoverage<
				FMatchPlayAuthoritativeResolveNoLegalSkillResult,
				FMatchPlayCurrentAttackSkillSelectionCandidateAvailability>(
					*this, *Prefix, Baseline,
					[Index](auto& Value) -> decltype(auto)
					{
						return (Value.ResolutionResult.SkillAvailabilityResult.Candidates[Index]);
					}, Comparator, &CoverageAudit, bEmpty);
			TestSkillGlobalContextDeepMutations(
				*this, CoverageAudit, Prefix + TEXT(".LegalityResult.GlobalContextResult"),
				Baseline,
				[Index](auto& Value) -> decltype(auto)
				{
					return (Value.ResolutionResult.SkillAvailabilityResult.Candidates[Index]
						.LegalityResult.GlobalContextResult);
				}, Comparator, bEmpty);
			TestCardSnapshotQueryDeepMutations(
				*this, CoverageAudit, Prefix + TEXT(".LegalityResult.CarrierSnapshotQueryResult"),
				Baseline,
				[Index](auto& Value) -> decltype(auto)
				{
					return (Value.ResolutionResult.SkillAvailabilityResult.Candidates[Index]
						.LegalityResult.CarrierSnapshotQueryResult);
				}, Comparator, bEmpty);
			TestSkillRuleQueryDeepMutations(
				*this, CoverageAudit, Prefix + TEXT(".LegalityResult.SkillRuleQueryResult"),
				Baseline,
				[Index](auto& Value) -> decltype(auto)
				{
					return (Value.ResolutionResult.SkillAvailabilityResult.Candidates[Index]
						.LegalityResult.SkillRuleQueryResult);
				}, Comparator, bEmpty);
			TestSkillRuleDeepMutations(
				*this, CoverageAudit, Prefix + TEXT(".LegalityResult.ResolvedSkillRule"),
				Baseline,
				[Index](auto& Value) -> decltype(auto)
				{
					return (Value.ResolutionResult.SkillAvailabilityResult.Candidates[Index]
						.LegalityResult.ResolvedSkillRule);
				}, Comparator, bEmpty);
		},
		[](const auto& Left, const auto& Right)
		{
			return AreAuthoritativeResolveNoLegalSkillResultsEqual(Left, Right);
		});
	DeepContainerCases += TestAvailabilityCandidateContainerMutations(
		*this,
		CoverageAudit,
		TEXT("DeclineSkill.DeclineResult.SkillAvailabilityResult.Candidates"),
		SkillDecline,
		[](auto& Value) -> decltype(auto)
		{
			return (Value.DeclineResult.SkillAvailabilityResult);
		},
		[](auto& Candidate, const int32 Index)
		{
			Candidate.SkillId = FName(*FString::Printf(
				TEXT("Canonical.DeclineSkill.%d"), Index));
		},
		[&](const auto& Baseline, const FString& Prefix,
			const int32 Index, const bool bEmpty)
		{
			auto Comparator = [](const auto& Left, const auto& Right)
			{
				return AreAuthoritativeDeclineSkillResultsEqual(Left, Right);
			};
			TestNestedReflectedMutationCoverage<
				FMatchPlayAuthoritativeDeclineSkillResult,
				FMatchPlayCurrentAttackSkillSelectionCandidateAvailability>(
					*this, *Prefix, Baseline,
					[Index](auto& Value) -> decltype(auto)
					{
						return (Value.DeclineResult.SkillAvailabilityResult.Candidates[Index]);
					}, Comparator, &CoverageAudit, bEmpty);
			TestSkillGlobalContextDeepMutations(
				*this, CoverageAudit, Prefix + TEXT(".LegalityResult.GlobalContextResult"),
				Baseline,
				[Index](auto& Value) -> decltype(auto)
				{
					return (Value.DeclineResult.SkillAvailabilityResult.Candidates[Index]
						.LegalityResult.GlobalContextResult);
				}, Comparator, bEmpty);
			TestCardSnapshotQueryDeepMutations(
				*this, CoverageAudit, Prefix + TEXT(".LegalityResult.CarrierSnapshotQueryResult"),
				Baseline,
				[Index](auto& Value) -> decltype(auto)
				{
					return (Value.DeclineResult.SkillAvailabilityResult.Candidates[Index]
						.LegalityResult.CarrierSnapshotQueryResult);
				}, Comparator, bEmpty);
			TestSkillRuleQueryDeepMutations(
				*this, CoverageAudit, Prefix + TEXT(".LegalityResult.SkillRuleQueryResult"),
				Baseline,
				[Index](auto& Value) -> decltype(auto)
				{
					return (Value.DeclineResult.SkillAvailabilityResult.Candidates[Index]
						.LegalityResult.SkillRuleQueryResult);
				}, Comparator, bEmpty);
			TestSkillRuleDeepMutations(
				*this, CoverageAudit, Prefix + TEXT(".LegalityResult.ResolvedSkillRule"),
				Baseline,
				[Index](auto& Value) -> decltype(auto)
				{
					return (Value.DeclineResult.SkillAvailabilityResult.Candidates[Index]
						.LegalityResult.ResolvedSkillRule);
				}, Comparator, bEmpty);
		},
		[](const auto& Left, const auto& Right)
		{
			return AreAuthoritativeDeclineSkillResultsEqual(Left, Right);
		});

	DeepContainerCases += TestCompletionSnapshotContainerMutations(
		*this,
		CoverageAudit,
		TEXT("ResolveNoLegalMarker.ResolutionResult.CompletionResult.DeploymentSnapshotQueryResults"),
		MarkerResolve,
		[](auto& Value) -> decltype(auto)
		{
			return (Value.ResolutionResult.CompletionResult);
		},
		[](auto& Item, const int32 Index)
		{
			Item.CardId = FName(*FString::Printf(TEXT("Canonical.ResolveMarker.Snapshot.%d"), Index));
			Item.PlayerSide = Index % 2 == 0
				? EInitialTurnOrderPlayer::PlayerA : EInitialTurnOrderPlayer::PlayerB;
		},
		[&](const auto& Baseline, const FString& Prefix,
			const int32 Index, const bool bEmpty)
		{
			auto Comparator = [](const auto& Left, const auto& Right)
			{
				return AreAuthoritativeResolveNoLegalMarkerResultsEqual(Left, Right);
			};
			TestCardSnapshotQueryDeepMutations(
				*this, CoverageAudit, Prefix, Baseline,
				[Index](auto& Value) -> decltype(auto)
				{
					return (Value.ResolutionResult.CompletionResult
						.DeploymentSnapshotQueryResults[Index]);
				}, Comparator, bEmpty);
		},
		[](const auto& Left, const auto& Right)
		{
			return AreAuthoritativeResolveNoLegalMarkerResultsEqual(Left, Right);
		});
	DeepContainerCases += TestCompletionSnapshotContainerMutations(
		*this,
		CoverageAudit,
		TEXT("DeclineMarker.DeclineResult.CompletionResult.DeploymentSnapshotQueryResults"),
		MarkerDecline,
		[](auto& Value) -> decltype(auto)
		{
			return (Value.DeclineResult.CompletionResult);
		},
		[](auto& Item, const int32 Index)
		{
			Item.CardId = FName(*FString::Printf(TEXT("Canonical.DeclineMarker.Snapshot.%d"), Index));
			Item.PlayerSide = Index % 2 == 0
				? EInitialTurnOrderPlayer::PlayerA : EInitialTurnOrderPlayer::PlayerB;
		},
		[&](const auto& Baseline, const FString& Prefix,
			const int32 Index, const bool bEmpty)
		{
			auto Comparator = [](const auto& Left, const auto& Right)
			{
				return AreAuthoritativeDeclineMarkerResultsEqual(Left, Right);
			};
			TestCardSnapshotQueryDeepMutations(
				*this, CoverageAudit, Prefix, Baseline,
				[Index](auto& Value) -> decltype(auto)
				{
					return (Value.DeclineResult.CompletionResult
						.DeploymentSnapshotQueryResults[Index]);
				}, Comparator, bEmpty);
		},
		[](const auto& Left, const auto& Right)
		{
			return AreAuthoritativeDeclineMarkerResultsEqual(Left, Right);
		});
	DeepContainerCases += TestCompletionSnapshotContainerMutations(
		*this,
		CoverageAudit,
		TEXT("ResolveNoLegalSkill.ResolutionResult.CompletionResult.DeploymentSnapshotQueryResults"),
		SkillResolve,
		[](auto& Value) -> decltype(auto)
		{
			return (Value.ResolutionResult.CompletionResult);
		},
		[](auto& Item, const int32 Index)
		{
			Item.CardId = FName(*FString::Printf(TEXT("Canonical.ResolveSkill.Snapshot.%d"), Index));
			Item.PlayerSide = Index % 2 == 0
				? EInitialTurnOrderPlayer::PlayerA : EInitialTurnOrderPlayer::PlayerB;
		},
		[&](const auto& Baseline, const FString& Prefix,
			const int32 Index, const bool bEmpty)
		{
			auto Comparator = [](const auto& Left, const auto& Right)
			{
				return AreAuthoritativeResolveNoLegalSkillResultsEqual(Left, Right);
			};
			TestCardSnapshotQueryDeepMutations(
				*this, CoverageAudit, Prefix, Baseline,
				[Index](auto& Value) -> decltype(auto)
				{
					return (Value.ResolutionResult.CompletionResult
						.DeploymentSnapshotQueryResults[Index]);
				}, Comparator, bEmpty);
		},
		[](const auto& Left, const auto& Right)
		{
			return AreAuthoritativeResolveNoLegalSkillResultsEqual(Left, Right);
		});
	DeepContainerCases += TestCompletionSnapshotContainerMutations(
		*this,
		CoverageAudit,
		TEXT("DeclineSkill.DeclineResult.CompletionResult.DeploymentSnapshotQueryResults"),
		SkillDecline,
		[](auto& Value) -> decltype(auto)
		{
			return (Value.DeclineResult.CompletionResult);
		},
		[](auto& Item, const int32 Index)
		{
			Item.CardId = FName(*FString::Printf(TEXT("Canonical.DeclineSkill.Snapshot.%d"), Index));
			Item.PlayerSide = Index % 2 == 0
				? EInitialTurnOrderPlayer::PlayerA : EInitialTurnOrderPlayer::PlayerB;
		},
		[&](const auto& Baseline, const FString& Prefix,
			const int32 Index, const bool bEmpty)
		{
			auto Comparator = [](const auto& Left, const auto& Right)
			{
				return AreAuthoritativeDeclineSkillResultsEqual(Left, Right);
			};
			TestCardSnapshotQueryDeepMutations(
				*this, CoverageAudit, Prefix, Baseline,
				[Index](auto& Value) -> decltype(auto)
				{
					return (Value.DeclineResult.CompletionResult
						.DeploymentSnapshotQueryResults[Index]);
				}, Comparator, bEmpty);
		},
		[](const auto& Left, const auto& Right)
		{
			return AreAuthoritativeDeclineSkillResultsEqual(Left, Right);
		});
	const FNonReflectedMutationAudit NonReflectedAudit =
		TestNonReflectedInventoryMutations(*this, CoverageAudit);
	TSet<FString> MissingPaths;
	for (const FString& Path : CoverageAudit.ExpectedLogicalPaths)
	{
		if (!CoverageAudit.CoveredLogicalPaths.Contains(Path))
		{
			MissingPaths.Add(Path);
		}
	}
	TSet<FString> UnexpectedPaths;
	for (const FString& Path : CoverageAudit.CoveredLogicalPaths)
	{
		if (!CoverageAudit.ExpectedLogicalPaths.Contains(Path))
		{
			UnexpectedPaths.Add(Path);
		}
	}
	int64 DuplicateExecutions = 0;
	for (const TPair<FString, int32>& Entry : CoverageAudit.ExecutionCountByPath)
	{
		DuplicateExecutions += FMath::Max(Entry.Value - 1, 0);
	}
	int32 NonReflectedInventoryCovered = 0;
	for (const FString& Path : CoverageAudit.CoveredLogicalPaths)
	{
		if (Path.StartsWith(TEXT("NonReflected.")))
		{
			++NonReflectedInventoryCovered;
		}
	}
	auto JoinPaths = [](const TSet<FString>& Paths)
	{
		TArray<FString> SortedPaths = Paths.Array();
		SortedPaths.Sort();
		return SortedPaths.IsEmpty()
			? FString(TEXT("<none>"))
			: FString::Join(SortedPaths, TEXT("; "));
	};
	TestEqual(TEXT("Non-reflected inventory runtime coverage"),
		NonReflectedInventoryCovered, 57);
	TestEqual(TEXT("Non-reflected declared inventory remains 57"),
		NonReflectedAudit.DeclaredFields, 57);
	TestTrue(TEXT("All six reflected comparator graphs executed leaves"),
		MarkerMutationAudit.LeafMutations > 0
		&& ResolveMarkerMutationAudit.LeafMutations > 0
		&& DeclineMarkerMutationAudit.LeafMutations > 0
		&& SkillMutationAudit.LeafMutations > 0
		&& ResolveSkillMutationAudit.LeafMutations > 0
		&& DeclineSkillMutationAudit.LeafMutations > 0);
	TestEqual(TEXT("Mutation missing paths"), MissingPaths.Num(), 0);
	TestEqual(TEXT("Mutation unexpected paths"), UnexpectedPaths.Num(), 0);
	TestEqual(TEXT("Mutation skipped groups"),
		CoverageAudit.SkippedGroupPaths.Num(), 0);
	TestEqual(TEXT("Reachable TMap paths in six comparator graphs"),
		CoverageAudit.ReachableMapPaths.Num(), 0);
	TestTrue(TEXT("Nine manual container graphs executed"),
		DeepContainerCases > 0 && CoverageAudit.EmptyContainerElementFieldPaths.Num() > 0);
	AddInfo(FString::Printf(TEXT("Mutation missing paths (%d): %s"),
		MissingPaths.Num(), *JoinPaths(MissingPaths)));
	AddInfo(FString::Printf(TEXT("Mutation unexpected paths (%d): %s"),
		UnexpectedPaths.Num(), *JoinPaths(UnexpectedPaths)));
	AddInfo(FString::Printf(TEXT("Mutation skipped groups (%d): %s"),
		CoverageAudit.SkippedGroupPaths.Num(),
		*JoinPaths(CoverageAudit.SkippedGroupPaths)));
	AddInfo(FString::Printf(
		TEXT("Foundation B runtime mutation audit: expected=%d; covered=%d; unique=%d; executions=%lld; duplicates=%lld; reflected leaf unique=%d; non-reflected unique=%d; container behavior unique=%d; empty-element field unique=%d; envelope unique=%d; equal baselines=%lld; missing=%d; unexpected=%d; skipped=%d; reachable TMap=%d (NOT APPLICABLE)."),
		CoverageAudit.ExpectedLogicalPaths.Num(),
		CoverageAudit.CoveredLogicalPaths.Num(),
		CoverageAudit.CoveredLogicalPaths.Num(),
		CoverageAudit.ExecutedMutationCases,
		DuplicateExecutions,
		CoverageAudit.ReflectedLeafPaths.Num(),
		CoverageAudit.NonReflectedFieldPaths.Num(),
		CoverageAudit.ContainerCasePaths.Num(),
		CoverageAudit.EmptyContainerElementFieldPaths.Num(),
		CoverageAudit.EnvelopePaths.Num(),
		CoverageAudit.EqualBaselineCases,
		MissingPaths.Num(),
		UnexpectedPaths.Num(),
		CoverageAudit.SkippedGroupPaths.Num(),
		CoverageAudit.ReachableMapPaths.Num()));
	return true;
}

MATCH_PLAY_AUTHORITATIVE_SESSION_TEST(
	FMatchPlayAuthoritativeSessionFoundationBProductionBoundaryTest,
	"32.FoundationBProductionBoundary")

bool FMatchPlayAuthoritativeSessionFoundationBProductionBoundaryTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayAuthoritativeSessionTests;
	FString Header;
	FString Implementation;
	FString Types;
	TestTrue(TEXT("Foundation B header loads"), LoadProductionSource(
		TEXT("Source/FMCodex/MatchPlayRuntime/MatchPlayAuthoritativeSession.h"),
		Header));
	TestTrue(TEXT("Foundation B implementation loads"), LoadProductionSource(
		TEXT("Source/FMCodex/MatchPlayRuntime/MatchPlayAuthoritativeSession.cpp"),
		Implementation));
	TestTrue(TEXT("Foundation B types load"), LoadProductionSource(
		TEXT("Source/FMCodex/MatchPlayRuntime/MatchPlayAuthoritativeSessionTypes.h"),
		Types));

	for (const TPair<const TCHAR*, const TCHAR*>& Operation : {
		TPair<const TCHAR*, const TCHAR*>(TEXT("Marker writer"),
			TEXT("FMatchPlayCurrentAttackMarkerSelectionWriter::Select(")),
		TPair<const TCHAR*, const TCHAR*>(TEXT("No-legal marker"),
			TEXT("FMatchPlayResolveNoLegalMarker::Resolve(")),
		TPair<const TCHAR*, const TCHAR*>(TEXT("Decline marker"),
			TEXT("FMatchPlayMarkerDecline::Decline(")),
		TPair<const TCHAR*, const TCHAR*>(TEXT("Skill writer"),
			TEXT("FMatchPlayCurrentAttackSkillSelectionWriter::Select(")),
		TPair<const TCHAR*, const TCHAR*>(TEXT("No-legal skill"),
			TEXT("FMatchPlayResolveNoLegalSkill::Resolve(")),
		TPair<const TCHAR*, const TCHAR*>(TEXT("Decline skill"),
			TEXT("FMatchPlaySkillDecline::Decline(")) })
	{
		TestEqual(*FString::Printf(TEXT("%s has one production call"), Operation.Key),
			CountOccurrences(Implementation, Operation.Value),
			1);
	}
	TestEqual(TEXT("All eleven mutations share serialized gate"),
		CountOccurrences(Implementation, TEXT("ExecuteSerialized<")),
		11);
	TestEqual(TEXT("Session retains one state replacement"),
		CountOccurrences(
			Implementation,
			TEXT("AuthoritativeState = Adoption.AdoptedAfterState;")),
		1);
	TestFalse(TEXT("No public system marker request type"),
		Types.Contains(TEXT("FMatchPlayAuthoritativeResolveNoLegalMarkerRequest")));
	TestFalse(TEXT("No public system skill request type"),
		Types.Contains(TEXT("FMatchPlayAuthoritativeResolveNoLegalSkillRequest")));
	TestFalse(TEXT("Skill rules are not stored on Session"),
		Header.Contains(TEXT("FSkillRuleSnapshotSet SkillRuleSet;")));
	for (const TCHAR* ForbiddenWrite : {
		TEXT("AuthoritativeState.CurrentAttack.ActionPreparation.MarkerCardId ="),
		TEXT("AuthoritativeState.CurrentAttack.ActionPreparation.SkillId ="),
		TEXT("AuthoritativeState.CurrentAttack.SelectionStage ="),
		TEXT("AuthoritativeState.bHasCurrentAttack ="),
		TEXT("AuthoritativeState.CurrentAttack.DeploymentPlacements.Add") })
	{
		TestFalse(*FString::Printf(TEXT("Direct gameplay write absent: %s"), ForbiddenWrite),
			Implementation.Contains(ForbiddenWrite));
	}
	const FString Production = Header + Implementation + Types;
	for (const TCHAR* Forbidden : {
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
		TestFalse(*FString::Printf(TEXT("Out-of-scope production surface absent: %s"), Forbidden),
			Production.Contains(Forbidden));
	}
	return true;
}

#undef MATCH_PLAY_AUTHORITATIVE_SESSION_TEST

#endif
