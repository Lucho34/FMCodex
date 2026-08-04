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

	void VisitReflectedMutationLeaves(
		UStruct* Struct,
		void* MutatedContainer,
		const void* BaselineContainer,
		const FString& Prefix,
		const TFunctionRef<void(const FString&)>& RejectMutation);

	void VisitReflectedPropertyMutation(
		FProperty* Property,
		void* MutatedValue,
		const void* BaselineValue,
		const FString& Path,
		const TFunctionRef<void(const FString&)>& RejectMutation)
	{
		if (FStructProperty* StructProperty =
			CastField<FStructProperty>(Property))
		{
			VisitReflectedMutationLeaves(
				StructProperty->Struct,
				MutatedValue,
				BaselineValue,
				Path,
				RejectMutation);
			return;
		}

		if (FArrayProperty* ArrayProperty =
			CastField<FArrayProperty>(Property))
		{
			FScriptArrayHelper MutatedArray(ArrayProperty, MutatedValue);
			FScriptArrayHelper BaselineArray(
				ArrayProperty,
				const_cast<void*>(BaselineValue));
			if (MutatedArray.Num() > 0)
			{
				MutatedArray.RemoveValues(MutatedArray.Num() - 1, 1);
			}
			else
			{
				MutatedArray.AddValue();
			}
			RejectMutation(Path + TEXT(".Num"));
			ArrayProperty->CopyCompleteValue(MutatedValue, BaselineValue);

			if (BaselineArray.Num() > 0)
			{
				VisitReflectedPropertyMutation(
					ArrayProperty->Inner,
					MutatedArray.GetRawPtr(0),
					BaselineArray.GetRawPtr(0),
					Path + TEXT("[0]"),
					RejectMutation);
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
			return;
		}

		RejectMutation(Path);
		Property->CopyCompleteValue(MutatedValue, BaselineValue);
	}

	void VisitReflectedMutationLeaves(
		UStruct* Struct,
		void* MutatedContainer,
		const void* BaselineContainer,
		const FString& Prefix,
		const TFunctionRef<void(const FString&)>& RejectMutation)
	{
		for (TFieldIterator<FProperty> It(Struct); It; ++It)
		{
			FProperty* Property = *It;
			VisitReflectedPropertyMutation(
				Property,
				Property->ContainerPtrToValuePtr<void>(MutatedContainer),
				Property->ContainerPtrToValuePtr<void>(BaselineContainer),
				Prefix + TEXT(".") + Property->GetName(),
				RejectMutation);
		}
	}

	template <typename TWrapper, typename TNested, typename TAccessor,
		typename TComparator>
	int32 TestNestedReflectedMutationCoverage(
		FAutomationTestBase& Test,
		const TCHAR* Context,
		const TWrapper& Baseline,
		TAccessor&& Accessor,
		TComparator&& Comparator)
	{
		TWrapper Mutated = Baseline;
		int32 MutationCount = 0;
		TNested& MutatedNested = Accessor(Mutated);
		const TNested& BaselineNested = Accessor(Baseline);
		VisitReflectedMutationLeaves(
			TNested::StaticStruct(),
			&MutatedNested,
			&BaselineNested,
			Context,
			[&](const FString& Path)
			{
				++MutationCount;
				Test.TestFalse(
					*FString::Printf(TEXT("Comparator covers %s"), *Path),
					Comparator(Baseline, Mutated));
			});
		return MutationCount;
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

	FMatchPlayAuthoritativeSession EarlySession;
	EarlySession.InitializeMatch(MakeFoundationBInput(TEXT("MarkerEarly")));
	EarlySession.BeginOrdinaryAttack(6);
	const FMatchPlayState EarlyState = EarlySession.GetStateSnapshot();
	const FMatchPlayAuthoritativeSubmitMarkerResult Early =
		EarlySession.SubmitMarker(EmptyRequest);
	TestTrue(TEXT("Early marker reaches domain"),
		Early.RuntimeEnvelope.bAccepted);
	TestFalse(TEXT("Early marker fails domain"), Early.MarkerResult.bSuccess);
	TestTrue(TEXT("Early marker leaves state unchanged"),
		AreStatesEqual(EarlyState, EarlySession.GetStateSnapshot()));

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
	TestTrue(TEXT("Wrong-side marker unchanged"),
		AreStatesEqual(BeforeFailures, Session.GetStateSnapshot()));

	FMatchPlayAuthoritativeSubmitMarkerRequest Invalid =
		MakeMarkerRequest(Trace, TEXT("Card.NotDeployed"));
	const auto InvalidResult = Session.SubmitMarker(Invalid);
	TestEqual(TEXT("Undeployed marker exact error"),
		InvalidResult.MarkerResult.LegalityResult.ErrorCode,
		EMatchPlayCurrentAttackMarkerSelectionErrorCode::MarkerNotDeployed);
	TestTrue(TEXT("Undeployed marker unchanged"),
		AreStatesEqual(BeforeFailures, Session.GetStateSnapshot()));

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

	const FMatchPlayState AfterSuccess = Session.GetStateSnapshot();
	const auto Replay = Session.SubmitMarker(
		MakeMarkerRequest(Trace, MarkerCardId));
	TestFalse(TEXT("Marker replay fails"), Replay.MarkerResult.bSuccess);
	TestEqual(TEXT("Marker replay exact stage error"),
		Replay.MarkerResult.LegalityResult.ErrorCode,
		EMatchPlayCurrentAttackMarkerSelectionErrorCode::WrongSelectionStage);
	TestTrue(TEXT("Marker replay unchanged"),
		AreStatesEqual(AfterSuccess, Session.GetStateSnapshot()));

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
	TArray<FMatchPlayAuthoritativeSubmitMarkerResult> Replays;
	TArray<FMatchPlayState> Finals;
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
		const auto Request = MakeMarkerRequest(Trace, MarkerCardId);
		Successes.Add(Session.SubmitMarker(Request));
		Replays.Add(Session.SubmitMarker(Request));
		Finals.Add(Session.GetStateSnapshot());
	}
	for (int32 Index = 1; Index < 3; ++Index)
	{
		TestTrue(TEXT("Marker success deterministic"),
			AreAuthoritativeSubmitMarkerResultsEqual(
				Successes[0], Successes[Index]));
		TestTrue(TEXT("Marker replay deterministic"),
			AreAuthoritativeSubmitMarkerResultsEqual(
				Replays[0], Replays[Index]));
		TestTrue(TEXT("Marker final state deterministic"),
			AreStatesEqual(Finals[0], Finals[Index]));
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

	FMatchPlayAuthoritativeSession LegalSession;
	FReachabilityTrace LegalTrace;
	BuildFoundationBToAwaitingMarker(
		LegalSession, TEXT("MarkerHasLegal"), true, {}, LegalTrace);
	const FMatchPlayState LegalBefore = LegalSession.GetStateSnapshot();
	const auto Rejected = LegalSession.ResolveNoLegalMarker();
	TestEqual(TEXT("Legal marker blocks system path exactly"),
		Rejected.ResolutionResult.ErrorCode,
		EMatchPlayResolveNoLegalMarkerErrorCode::LegalMarkerExists);
	TestTrue(TEXT("Rejected system marker unchanged"),
		AreStatesEqual(LegalBefore, LegalSession.GetStateSnapshot()));
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
	TestTrue(TEXT("Wrong-side decline marker unchanged"),
		AreStatesEqual(Before, Session.GetStateSnapshot()));

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
	for (int32 Index = 0; Index < 3; ++Index)
	{
		FMatchPlayAuthoritativeSession ResolveSession;
		FReachabilityTrace ResolveTrace;
		BuildFoundationBToAwaitingMarker(
			ResolveSession, TEXT("MarkerCompleteResolve"), false, {}, ResolveTrace);
		Resolves.Add(ResolveSession.ResolveNoLegalMarker());

		FMatchPlayAuthoritativeSession DeclineSession;
		FReachabilityTrace DeclineTrace;
		BuildFoundationBToAwaitingMarker(
			DeclineSession, TEXT("MarkerCompleteDecline"), true, {}, DeclineTrace);
		FMatchPlayAuthoritativeDeclineMarkerRequest Request;
		Request.RequestingSide = DeclineTrace.DefendingSide;
		Declines.Add(DeclineSession.DeclineMarker(Request));
	}
	for (int32 Index = 1; Index < 3; ++Index)
	{
		TestTrue(TEXT("No-legal marker completion deterministic"),
			AreAuthoritativeResolveNoLegalMarkerResultsEqual(
				Resolves[0], Resolves[Index]));
		TestTrue(TEXT("Decline marker completion deterministic"),
			AreAuthoritativeDeclineMarkerResultsEqual(
				Declines[0], Declines[Index]));
	}
	TestFalse(TEXT("No-legal and decline provenance stay distinct"),
		Resolves[0].ResolutionResult.Source
			== static_cast<EMatchPlayMarkerNoSelectionGoalSource>(
				Declines[0].DeclineResult.Source));
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
		const auto Result = Session.SubmitSkill(Rules, Request);
		TestTrue(*FString::Printf(TEXT("%s submit succeeds"), Case.Suffix),
			Result.SkillResult.bSuccess);
		TestEqual(*FString::Printf(TEXT("%s exact selected ID"), Case.Suffix),
			Result.SkillResult.SelectedSkillId,
			Case.SkillId);
		TestEqual(*FString::Printf(TEXT("%s exact action type"), Case.Suffix),
			Result.SkillResult.SelectedActionType,
			Case.SkillType);
		TestEqual(*FString::Printf(TEXT("%s exact next stage"), Case.Suffix),
			Session.GetStateSnapshot().CurrentAttack.SelectionStage,
			Case.ExpectedStage);
		TestTrue(*FString::Printf(TEXT("%s exact AfterState adopted"), Case.Suffix),
			AreStatesEqual(
				Result.SkillResult.AfterState,
				Session.GetStateSnapshot()));
	}
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
	TestTrue(TEXT("Pre-AwaitingSkill state unchanged"),
		AreStatesEqual(EarlyBefore, EarlySession.GetStateSnapshot()));

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
	TestTrue(TEXT("Wrong-side skill state unchanged"),
		AreStatesEqual(Before, Session.GetStateSnapshot()));

	Request.RequestingSide = Trace.AttackingSide;
	FSkillRuleSnapshotSet EmptyRules;
	const auto MissingRule = Session.SubmitSkill(EmptyRules, Request);
	TestEqual(TEXT("Unavailable rule exact error"),
		MissingRule.SkillResult.LegalityResult.ErrorCode,
		EMatchPlayCurrentAttackSkillSelectionErrorCode::SkillRuleNotFound);
	TestTrue(TEXT("Unavailable rule state unchanged"),
		AreStatesEqual(Before, Session.GetStateSnapshot()));

	const auto Success = Session.SubmitSkill(Rules, Request);
	TestTrue(TEXT("Valid skill succeeds after failures"),
		Success.SkillResult.bSuccess);
	const FMatchPlayState After = Session.GetStateSnapshot();
	const auto Replay = Session.SubmitSkill(Rules, Request);
	TestEqual(TEXT("Skill replay exact stage error"),
		Replay.SkillResult.LegalityResult.ErrorCode,
		EMatchPlayCurrentAttackSkillSelectionErrorCode::WrongSelectionStage);
	TestTrue(TEXT("Skill replay unchanged"),
		AreStatesEqual(After, Session.GetStateSnapshot()));
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
	TestTrue(TEXT("Rejected no-legal skill unchanged"),
		AreStatesEqual(LegalBefore, LegalSession.GetStateSnapshot()));
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
	TestTrue(TEXT("Wrong-side skill decline unchanged"),
		AreStatesEqual(Before, Session.GetStateSnapshot()));

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
	const auto NoLegal = NoLegalSession.DeclineSkill(
		EmptyRules,
		NoLegalRequest);
	TestEqual(TEXT("No-legal skill cannot decline exactly"),
		NoLegal.DeclineResult.ErrorCode,
		EMatchPlaySkillNoSelectionNoGoalErrorCode::NoLegalSkillToDecline);
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
		Resolves.Add(ResolveSession.ResolveNoLegalSkill(EmptyRules));

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
		Declines.Add(DeclineSession.DeclineSkill(Rules, Request));
	}
	for (int32 Index = 1; Index < 3; ++Index)
	{
		TestTrue(TEXT("No-legal skill completion deterministic"),
			AreAuthoritativeResolveNoLegalSkillResultsEqual(
				Resolves[0], Resolves[Index]));
		TestTrue(TEXT("Decline skill completion deterministic"),
			AreAuthoritativeDeclineSkillResultsEqual(
				Declines[0], Declines[Index]));
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
	return true;
}

MATCH_PLAY_AUTHORITATIVE_SESSION_TEST(
	FMatchPlayAuthoritativeSessionFoundationBComparatorCoverageTest,
	"31.FoundationBComparatorCoverage")

bool FMatchPlayAuthoritativeSessionFoundationBComparatorCoverageTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayAuthoritativeSessionTests;

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
	TestEnvelopeMutationCoverage(*this, Marker.RuntimeEnvelope);
	const int32 MarkerLeafMutations =
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
				});
	auto MutatedMarker = Marker;
	MutatedMarker.MarkerResult.LegalityResult.MarkerSnapshotQueryResult
		.ErrorMessage = TEXT("mutated hidden marker snapshot");
	TestFalse(TEXT("Marker comparator covers hidden snapshot result"),
		AreAuthoritativeSubmitMarkerResultsEqual(Marker, MutatedMarker));
	MutatedMarker = Marker;
	MutatedMarker.RuntimeEnvelope.CommandKind =
		EMatchPlayAuthoritativeCommandKind::DeclineMarker;
	TestFalse(TEXT("Marker comparator covers runtime envelope"),
		AreAuthoritativeSubmitMarkerResultsEqual(Marker, MutatedMarker));

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
	TestEnvelopeMutationCoverage(*this, MarkerResolve.RuntimeEnvelope);
	const int32 ResolveMarkerLeafMutations =
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
				});
	auto MutatedMarkerResolve = MarkerResolve;
	TestTrue(TEXT("Resolve marker completion has deployment evidence"),
		MutatedMarkerResolve.ResolutionResult.CompletionResult
			.DeploymentSnapshotQueryResults.Num() > 0);
	if (MutatedMarkerResolve.ResolutionResult.CompletionResult
		.DeploymentSnapshotQueryResults.Num() > 0)
	{
		MutatedMarkerResolve.ResolutionResult.CompletionResult
			.DeploymentSnapshotQueryResults[0].ErrorMessage =
				TEXT("mutated hidden completion snapshot");
		TestFalse(TEXT("Resolve marker comparator covers hidden completion"),
			AreAuthoritativeResolveNoLegalMarkerResultsEqual(
				MarkerResolve,
				MutatedMarkerResolve));
	}

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
	TestEnvelopeMutationCoverage(*this, MarkerDecline.RuntimeEnvelope);
	const int32 DeclineMarkerLeafMutations =
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
				});
	auto MutatedMarkerDecline = MarkerDecline;
	MutatedMarkerDecline.DeclineResult.MarkerAvailabilityResult
		.GlobalBlockingLegalityResult.MarkerSnapshotQueryResult.ErrorMessage =
			TEXT("mutated hidden marker availability");
	TestFalse(TEXT("Decline marker comparator covers hidden availability"),
		AreAuthoritativeDeclineMarkerResultsEqual(
			MarkerDecline,
			MutatedMarkerDecline));

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
	TestEnvelopeMutationCoverage(*this, Skill.RuntimeEnvelope);
	const int32 SkillLeafMutations =
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
				});
	auto MutatedSkill = Skill;
	++MutatedSkill.SkillResult.LegalityResult.GlobalContextResult
		.ValidatedActionPoint;
	TestFalse(TEXT("Skill comparator covers hidden global context"),
		AreAuthoritativeSubmitSkillResultsEqual(Skill, MutatedSkill));
	MutatedSkill = Skill;
	MutatedSkill.SkillResult.LegalityResult.ResolvedSkillRule
		.MaxTriggerActionPoint++;
	TestFalse(TEXT("Skill comparator covers plain resolved rule"),
		AreAuthoritativeSubmitSkillResultsEqual(Skill, MutatedSkill));

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
	TestEnvelopeMutationCoverage(*this, SkillResolve.RuntimeEnvelope);
	const int32 ResolveSkillLeafMutations =
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
				});
	auto MutatedSkillResolve = SkillResolve;
	MutatedSkillResolve.ResolutionResult.SkillAvailabilityResult
		.GlobalContextResult.ErrorMessage =
			TEXT("mutated hidden skill global context");
	TestFalse(TEXT("Resolve skill comparator covers hidden availability"),
		AreAuthoritativeResolveNoLegalSkillResultsEqual(
			SkillResolve,
			MutatedSkillResolve));

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
	TestEnvelopeMutationCoverage(*this, SkillDecline.RuntimeEnvelope);
	const int32 DeclineSkillLeafMutations =
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
				});
	auto MutatedSkillDecline = SkillDecline;
	MutatedSkillDecline.DeclineResult.SkillAvailabilityResult
		.SkillRuleSetValidationResult.InvalidField = TEXT("mutated field");
	TestFalse(TEXT("Decline skill comparator covers rule validation"),
		AreAuthoritativeDeclineSkillResultsEqual(
			SkillDecline,
			MutatedSkillDecline));
	AddInfo(FString::Printf(
		TEXT("Foundation B reflected leaf mutations: marker=%d, resolve-marker=%d, decline-marker=%d, skill=%d, resolve-skill=%d, decline-skill=%d; envelope mutations=72; explicit non-reflected mutations=7."),
		MarkerLeafMutations,
		ResolveMarkerLeafMutations,
		DeclineMarkerLeafMutations,
		SkillLeafMutations,
		ResolveSkillLeafMutations,
		DeclineSkillLeafMutations));
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
