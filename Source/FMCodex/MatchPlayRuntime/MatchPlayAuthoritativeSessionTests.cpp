#include "MatchPlayAuthoritativeSession.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "../CoreRules/MatchPlayCurrentAttackCarrierSelectionAvailability.h"
#include "../CoreRules/MatchPlayCurrentAttackHelperSelectionAvailability.h"
#include "../CoreRules/MatchPlayCurrentAttackMarkerSelectionAvailability.h"
#include "../CoreRules/MatchPlayCurrentAttackResolveInitialRouteOrchestrationTestFixtures.h"
#include "../CoreRules/MatchPlayCurrentAttackRunnerSelectionAvailability.h"
#include "../CoreRules/MatchPlayCurrentAttackSkillSelectionAvailability.h"
#include "../CoreRules/MatchPlayOrdinaryDeploymentAvailability.h"

#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "UObject/UnrealType.h"

#include <type_traits>

namespace MatchPlayAuthoritativeSessionTests
{
	namespace InitialRouteFixtures =
		FMCodex::Tests::MatchPlayCurrentAttackResolveInitialRoute;
	namespace InitialRouteOrchestrationFixtures =
		FMCodex::Tests::MatchPlayCurrentAttackResolveInitialRouteOrchestration;

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

	bool AreRunnerGlobalContextResultsEqual(
		const FMatchPlayCurrentAttackRunnerSelectionGlobalContextResult& Left,
		const FMatchPlayCurrentAttackRunnerSelectionGlobalContextResult& Right)
	{
		if (Left.AttackingPlayerPlacements.Num()
			!= Right.AttackingPlayerPlacements.Num())
		{
			return false;
		}
		for (int32 Index = 0;
			Index < Left.AttackingPlayerPlacements.Num();
			++Index)
		{
			if (!AreReflectedValuesEqual(
				Left.AttackingPlayerPlacements[Index],
				Right.AttackingPlayerPlacements[Index]))
			{
				return false;
			}
		}
		return Left.bSuccess == Right.bSuccess
			&& Left.RequestedAttackSequence == Right.RequestedAttackSequence
			&& Left.AuthoritativeAttackSequence == Right.AuthoritativeAttackSequence
			&& Left.RequestingSide == Right.RequestingSide
			&& Left.CurrentAttackingPlayer == Right.CurrentAttackingPlayer
			&& Left.CurrentDefendingPlayer == Right.CurrentDefendingPlayer
			&& AreSelectionValidationResultsEqual(
				Left.SelectionStateValidationResult,
				Right.SelectionStateValidationResult)
			&& Left.FrozenCarrierCardId == Right.FrozenCarrierCardId
			&& Left.FrozenMarkerCardId == Right.FrozenMarkerCardId
			&& Left.FrozenSkillId == Right.FrozenSkillId
			&& Left.FrozenActionType == Right.FrozenActionType
			&& AreReflectedValuesEqual(
				Left.ParticipantRequirementResult,
				Right.ParticipantRequirementResult)
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
			&& ArePlayerCardValidationResultsEqual(
				Left.AttackingSnapshotSetValidationResult,
				Right.AttackingSnapshotSetValidationResult)
			&& AreReflectedValuesEqual(
				Left.SlotCatalogValidationResult,
				Right.SlotCatalogValidationResult)
			&& Left.ErrorCode == Right.ErrorCode
			&& Left.ErrorMessage == Right.ErrorMessage;
	}

	bool AreRunnerLegalityResultsEqual(
		const FMatchPlayCurrentAttackRunnerSelectionLegalityResult& Left,
		const FMatchPlayCurrentAttackRunnerSelectionLegalityResult& Right)
	{
		return AreReflectedValuesEqual(Left, Right)
			&& AreRunnerGlobalContextResultsEqual(
				Left.GlobalContextResult,
				Right.GlobalContextResult)
			&& AreCardSnapshotQueryResultsEqual(
				Left.RunnerSnapshotQueryResult,
				Right.RunnerSnapshotQueryResult);
	}

	bool AreRunnerAvailabilityResultsEqual(
		const FMatchPlayCurrentAttackRunnerSelectionAvailabilityResult& Left,
		const FMatchPlayCurrentAttackRunnerSelectionAvailabilityResult& Right)
	{
		if (!AreReflectedValuesEqual(Left, Right)
			|| !AreRunnerGlobalContextResultsEqual(
				Left.GlobalContextResult,
				Right.GlobalContextResult)
			|| Left.Candidates.Num() != Right.Candidates.Num())
		{
			return false;
		}
		for (int32 Index = 0; Index < Left.Candidates.Num(); ++Index)
		{
			if (!AreRunnerLegalityResultsEqual(
				Left.Candidates[Index].LegalityResult,
				Right.Candidates[Index].LegalityResult))
			{
				return false;
			}
		}
		return true;
	}

	bool AreAuthoritativeSubmitRunnerResultsEqual(
		const FMatchPlayAuthoritativeSubmitRunnerResult& Left,
		const FMatchPlayAuthoritativeSubmitRunnerResult& Right)
	{
		return AreEnvelopesEqual(Left.RuntimeEnvelope, Right.RuntimeEnvelope)
			&& AreReflectedValuesEqual(Left.RunnerResult, Right.RunnerResult)
			&& AreRunnerLegalityResultsEqual(
				Left.RunnerResult.LegalityResult,
				Right.RunnerResult.LegalityResult);
	}

	bool AreAuthoritativeResolveNoLegalRunnerResultsEqual(
		const FMatchPlayAuthoritativeResolveNoLegalRunnerResult& Left,
		const FMatchPlayAuthoritativeResolveNoLegalRunnerResult& Right)
	{
		return AreEnvelopesEqual(Left.RuntimeEnvelope, Right.RuntimeEnvelope)
			&& AreReflectedValuesEqual(
				Left.ResolutionResult,
				Right.ResolutionResult)
			&& AreRunnerAvailabilityResultsEqual(
				Left.ResolutionResult.RunnerAvailabilityResult,
				Right.ResolutionResult.RunnerAvailabilityResult)
			&& AreCompletionResultsEqual(
				Left.ResolutionResult.CompletionResult,
				Right.ResolutionResult.CompletionResult);
	}

	bool AreAuthoritativeDeclineRunnerResultsEqual(
		const FMatchPlayAuthoritativeDeclineRunnerResult& Left,
		const FMatchPlayAuthoritativeDeclineRunnerResult& Right)
	{
		return AreEnvelopesEqual(Left.RuntimeEnvelope, Right.RuntimeEnvelope)
			&& AreReflectedValuesEqual(Left.DeclineResult, Right.DeclineResult)
			&& AreRunnerAvailabilityResultsEqual(
				Left.DeclineResult.RunnerAvailabilityResult,
				Right.DeclineResult.RunnerAvailabilityResult)
			&& AreCompletionResultsEqual(
				Left.DeclineResult.CompletionResult,
				Right.DeclineResult.CompletionResult);
	}

	bool AreHelperGlobalContextResultsEqual(
		const FMatchPlayCurrentAttackHelperSelectionGlobalContextResult& Left,
		const FMatchPlayCurrentAttackHelperSelectionGlobalContextResult& Right)
	{
		if (Left.DefendingPlayerPlacements.Num()
			!= Right.DefendingPlayerPlacements.Num())
		{
			return false;
		}
		for (int32 Index = 0;
			Index < Left.DefendingPlayerPlacements.Num();
			++Index)
		{
			if (!AreReflectedValuesEqual(
				Left.DefendingPlayerPlacements[Index],
				Right.DefendingPlayerPlacements[Index]))
			{
				return false;
			}
		}
		return Left.bSuccess == Right.bSuccess
			&& Left.RequestedAttackSequence == Right.RequestedAttackSequence
			&& Left.AuthoritativeAttackSequence == Right.AuthoritativeAttackSequence
			&& Left.RequestingSide == Right.RequestingSide
			&& Left.CurrentAttackingPlayer == Right.CurrentAttackingPlayer
			&& Left.CurrentDefendingPlayer == Right.CurrentDefendingPlayer
			&& AreSelectionValidationResultsEqual(
				Left.SelectionStateValidationResult,
				Right.SelectionStateValidationResult)
			&& Left.FrozenCarrierCardId == Right.FrozenCarrierCardId
			&& Left.FrozenMarkerCardId == Right.FrozenMarkerCardId
			&& Left.FrozenSkillId == Right.FrozenSkillId
			&& Left.FrozenActionType == Right.FrozenActionType
			&& Left.FrozenRunnerCardId == Right.FrozenRunnerCardId
			&& AreReflectedValuesEqual(
				Left.ParticipantRequirementResult,
				Right.ParticipantRequirementResult)
			&& Left.MatchingFrozenCarrierPlacementCount
				== Right.MatchingFrozenCarrierPlacementCount
			&& Left.MatchingFrozenMarkerPlacementCount
				== Right.MatchingFrozenMarkerPlacementCount
			&& Left.MatchingFrozenRunnerPlacementCount
				== Right.MatchingFrozenRunnerPlacementCount
			&& AreReflectedValuesEqual(
				Left.FrozenCarrierPlacement,
				Right.FrozenCarrierPlacement)
			&& AreReflectedValuesEqual(
				Left.FrozenMarkerPlacement,
				Right.FrozenMarkerPlacement)
			&& AreReflectedValuesEqual(
				Left.FrozenRunnerPlacement,
				Right.FrozenRunnerPlacement)
			&& ArePlayerCardValidationResultsEqual(
				Left.AttackingSnapshotSetValidationResult,
				Right.AttackingSnapshotSetValidationResult)
			&& ArePlayerCardValidationResultsEqual(
				Left.DefendingSnapshotSetValidationResult,
				Right.DefendingSnapshotSetValidationResult)
			&& Left.ErrorCode == Right.ErrorCode
			&& Left.ErrorMessage == Right.ErrorMessage;
	}

	bool AreHelperLegalityResultsEqual(
		const FMatchPlayCurrentAttackHelperSelectionLegalityResult& Left,
		const FMatchPlayCurrentAttackHelperSelectionLegalityResult& Right)
	{
		return AreReflectedValuesEqual(Left, Right)
			&& AreHelperGlobalContextResultsEqual(
				Left.GlobalContextResult,
				Right.GlobalContextResult)
			&& AreCardSnapshotQueryResultsEqual(
				Left.HelperSnapshotQueryResult,
				Right.HelperSnapshotQueryResult);
	}

	bool AreHelperAvailabilityResultsEqual(
		const FMatchPlayCurrentAttackHelperSelectionAvailabilityResult& Left,
		const FMatchPlayCurrentAttackHelperSelectionAvailabilityResult& Right)
	{
		if (!AreReflectedValuesEqual(Left, Right)
			|| !AreHelperGlobalContextResultsEqual(
				Left.GlobalContextResult,
				Right.GlobalContextResult)
			|| Left.Candidates.Num() != Right.Candidates.Num())
		{
			return false;
		}
		for (int32 Index = 0; Index < Left.Candidates.Num(); ++Index)
		{
			if (!AreHelperLegalityResultsEqual(
				Left.Candidates[Index].LegalityResult,
				Right.Candidates[Index].LegalityResult))
			{
				return false;
			}
		}
		return true;
	}

	bool AreHelperAuthorityResultsEqual(
		const FMatchPlayCurrentAttackHelperParticipantAuthorityResult& Left,
		const FMatchPlayCurrentAttackHelperParticipantAuthorityResult& Right)
	{
		return Left.bSuccess == Right.bSuccess
			&& Left.MatchingPlacementCount == Right.MatchingPlacementCount
			&& AreReflectedValuesEqual(Left.Placement, Right.Placement)
			&& AreCardSnapshotQueryResultsEqual(
				Left.SnapshotQueryResult,
				Right.SnapshotQueryResult)
			&& AreReflectedValuesEqual(Left.Snapshot, Right.Snapshot)
			&& Left.ErrorCode == Right.ErrorCode
			&& Left.ErrorMessage == Right.ErrorMessage;
	}

	bool AreReadyValidationResultsEqual(
		const FMatchPlayCurrentAttackReadyValidationResult& Left,
		const FMatchPlayCurrentAttackReadyValidationResult& Right)
	{
		return AreReflectedValuesEqual(Left, Right)
			&& AreCardSnapshotQueryResultsEqual(
				Left.CarrierSnapshotQueryResult,
				Right.CarrierSnapshotQueryResult)
			&& AreCardSnapshotQueryResultsEqual(
				Left.MarkerSnapshotQueryResult,
				Right.MarkerSnapshotQueryResult)
			&& AreCardSnapshotQueryResultsEqual(
				Left.RunnerSnapshotQueryResult,
				Right.RunnerSnapshotQueryResult)
			&& AreHelperAuthorityResultsEqual(
				Left.HelperAuthorityResult,
				Right.HelperAuthorityResult)
			&& AreReflectedValuesEqual(
				Left.RunnerRelativeZoneResolveResult,
				Right.RunnerRelativeZoneResolveResult);
	}

	bool AreAuthoritativeSubmitHelperResultsEqual(
		const FMatchPlayAuthoritativeSubmitHelperResult& Left,
		const FMatchPlayAuthoritativeSubmitHelperResult& Right)
	{
		return AreEnvelopesEqual(Left.RuntimeEnvelope, Right.RuntimeEnvelope)
			&& AreReflectedValuesEqual(Left.HelperResult, Right.HelperResult)
			&& AreHelperLegalityResultsEqual(
				Left.HelperResult.LegalityResult,
				Right.HelperResult.LegalityResult)
			&& AreReadyValidationResultsEqual(
				Left.HelperResult.ReadyValidationResult,
				Right.HelperResult.ReadyValidationResult);
	}

	bool AreAuthoritativeResolveNoLegalHelperResultsEqual(
		const FMatchPlayAuthoritativeResolveNoLegalHelperResult& Left,
		const FMatchPlayAuthoritativeResolveNoLegalHelperResult& Right)
	{
		return AreEnvelopesEqual(Left.RuntimeEnvelope, Right.RuntimeEnvelope)
			&& AreReflectedValuesEqual(
				Left.ResolutionResult,
				Right.ResolutionResult)
			&& AreHelperAvailabilityResultsEqual(
				Left.ResolutionResult.HelperAvailabilityResult,
				Right.ResolutionResult.HelperAvailabilityResult)
			&& AreReadyValidationResultsEqual(
				Left.ResolutionResult.FinalizationResult.ReadyValidationResult,
				Right.ResolutionResult.FinalizationResult.ReadyValidationResult);
	}

	bool AreAuthoritativeDeclineHelperResultsEqual(
		const FMatchPlayAuthoritativeDeclineHelperResult& Left,
		const FMatchPlayAuthoritativeDeclineHelperResult& Right)
	{
		return AreEnvelopesEqual(Left.RuntimeEnvelope, Right.RuntimeEnvelope)
			&& AreReflectedValuesEqual(Left.DeclineResult, Right.DeclineResult)
			&& AreHelperAvailabilityResultsEqual(
				Left.DeclineResult.HelperAvailabilityResult,
				Right.DeclineResult.HelperAvailabilityResult)
			&& AreReadyValidationResultsEqual(
				Left.DeclineResult.FinalizationResult.ReadyValidationResult,
				Right.DeclineResult.FinalizationResult.ReadyValidationResult);
	}

	bool AreAuthoritativeBeginResolutionSessionResultsEqual(
		const FMatchPlayAuthoritativeBeginResolutionSessionResult& Left,
		const FMatchPlayAuthoritativeBeginResolutionSessionResult& Right)
	{
		return AreEnvelopesEqual(Left.RuntimeEnvelope, Right.RuntimeEnvelope)
			&& AreReflectedValuesEqual(Left.BeginResult, Right.BeginResult);
	}

	bool AreBranchIntentGlobalContextResultsEqual(
		const FMatchPlayCurrentAttackBranchIntentSelectionGlobalContextResult&
			Left,
		const FMatchPlayCurrentAttackBranchIntentSelectionGlobalContextResult&
			Right)
	{
		return Left.bSuccess == Right.bSuccess
			&& Left.RequestedAttackSequence
				== Right.RequestedAttackSequence
			&& Left.AuthoritativeAttackSequence
				== Right.AuthoritativeAttackSequence
			&& Left.RequestingSide == Right.RequestingSide
			&& Left.CurrentAttackingPlayer == Right.CurrentAttackingPlayer
			&& Left.CurrentDefendingPlayer == Right.CurrentDefendingPlayer
			&& AreSelectionValidationResultsEqual(
				Left.SelectionStateValidationResult,
				Right.SelectionStateValidationResult)
			&& AreReflectedValuesEqual(Left.Preparation, Right.Preparation)
			&& Left.FrozenActionType == Right.FrozenActionType
			&& Left.MatchingCarrierPlacementCount
				== Right.MatchingCarrierPlacementCount
			&& Left.MatchingMarkerPlacementCount
				== Right.MatchingMarkerPlacementCount
			&& Left.MatchingRunnerPlacementCount
				== Right.MatchingRunnerPlacementCount
			&& AreCardSnapshotQueryResultsEqual(
				Left.CarrierSnapshotQueryResult,
				Right.CarrierSnapshotQueryResult)
			&& AreCardSnapshotQueryResultsEqual(
				Left.MarkerSnapshotQueryResult,
				Right.MarkerSnapshotQueryResult)
			&& AreCardSnapshotQueryResultsEqual(
				Left.RunnerSnapshotQueryResult,
				Right.RunnerSnapshotQueryResult)
			&& AreHelperAuthorityResultsEqual(
				Left.HelperAuthorityResult,
				Right.HelperAuthorityResult)
			&& Left.ErrorCode == Right.ErrorCode
			&& Left.ErrorMessage == Right.ErrorMessage;
	}

	bool AreBranchIntentLegalityResultsEqual(
		const FMatchPlayCurrentAttackBranchIntentSelectionLegalityResult& Left,
		const FMatchPlayCurrentAttackBranchIntentSelectionLegalityResult& Right)
	{
		return AreReflectedValuesEqual(Left, Right)
			&& AreBranchIntentGlobalContextResultsEqual(
				Left.GlobalContextResult,
				Right.GlobalContextResult);
	}

	bool AreAuthoritativeSubmitBranchIntentResultsEqual(
		const FMatchPlayAuthoritativeSubmitBranchIntentResult& Left,
		const FMatchPlayAuthoritativeSubmitBranchIntentResult& Right)
	{
		return AreEnvelopesEqual(Left.RuntimeEnvelope, Right.RuntimeEnvelope)
			&& AreReflectedValuesEqual(Left.IntentResult, Right.IntentResult)
			&& AreBranchIntentLegalityResultsEqual(
				Left.IntentResult.LegalityResult,
				Right.IntentResult.LegalityResult)
			&& AreReadyValidationResultsEqual(
				Left.IntentResult.ReadyValidationResult,
				Right.IntentResult.ReadyValidationResult);
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

	bool BuildStage7162ToAwaitingRunner(
		FMatchPlayAuthoritativeSession& Session,
		const FString& Prefix,
		const bool bCreateLegalRunner,
		FReachabilityTrace& OutTrace,
		FName& OutRunnerCardId,
		const ESkillRuleType SkillType = ESkillRuleType::Cross,
		const int32 DeploymentCount = 4)
	{
		const FName SkillId(*FString::Printf(
			TEXT("Skill.%s.%d"),
			*Prefix,
			static_cast<int32>(SkillType)));
		FMatchPlayAuthoritativeSubmitMarkerResult Marker;
		if (!bCreateLegalRunner)
		{
			if (!BuildFoundationBToAwaitingSkill(
				Session,
				Prefix,
				{SkillId},
				OutTrace,
				Marker))
			{
				return false;
			}
		}
		else
		{
			FMatchPlayOpeningInitializeInput Input =
				MakeFoundationBInput(Prefix, {SkillId});
			for (int32 Index = 3; Index <= 4; ++Index)
			{
				FMatchPlayDeploymentSlotDefinition NearA;
				NearA.SlotId = FName(*FString::Printf(
					TEXT("%s_NearA%d"), *Prefix, Index));
				NearA.NeutralSide =
					EMatchPlayNeutralSlotSide::NearPlayerA;
				Input.DeploymentSlotCatalog.Slots.Add(NearA);
				FMatchPlayDeploymentSlotDefinition NearB;
				NearB.SlotId = FName(*FString::Printf(
					TEXT("%s_NearB%d"), *Prefix, Index));
				NearB.NeutralSide =
					EMatchPlayNeutralSlotSide::NearPlayerB;
				Input.DeploymentSlotCatalog.Slots.Add(NearB);
			}
			OutTrace.Initialize = Session.InitializeMatch(Input);
			OutTrace.Begin = Session.BeginOrdinaryAttack(6);
			if (!OutTrace.Initialize.OpeningResult.bSuccess
				|| !OutTrace.Begin.BeginResult.bSuccess)
			{
				return false;
			}

			FMatchPlayState State = Session.GetStateSnapshot();
			OutTrace.AttackSequence =
				State.CurrentAttack.AttackSequence;
			OutTrace.AttackingSide =
				State.RuntimeState.CurrentAttackingPlayer;
			OutTrace.DefendingSide = OtherPlayer(OutTrace.AttackingSide);
			for (int32 Index = 0; Index < DeploymentCount; ++Index)
			{
				State = Session.GetStateSnapshot();
				const bool bAttackerDeploying =
					State.CurrentAttack.CurrentLegalDeploymentSide
						== OutTrace.AttackingSide;
				FDeploymentChoice Choice;
				if (!FindLegalDeployment(
					State,
					bAttackerDeploying
						? EMatchPlayRelativeDeploymentZone::Forward
						: EMatchPlayRelativeDeploymentZone::Backfield,
					Choice)
					|| !Session.DeployOrdinary(
						MakeDeployRequest(Choice))
						.DeploymentResult.bSuccess)
				{
					return false;
				}
			}

			for (int32 Index = 0; Index < 2; ++Index)
			{
				State = Session.GetStateSnapshot();
				if (!Session.FinishDeployment(
					State.CurrentAttack.AttackSequence,
					State.CurrentAttack.CurrentLegalDeploymentSide)
					.FinishResult.bSuccess)
				{
					return false;
				}
			}

			State = Session.GetStateSnapshot();
			const auto CarrierAvailability =
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
			if (OutTrace.CarrierCardId.IsNone()
				|| !Session.SubmitCarrier(MakeCarrierRequest(OutTrace))
					.CarrierResult.bSuccess)
			{
				return false;
			}

			State = Session.GetStateSnapshot();
			FName MarkerCardId;
			if (!FindLegalMarker(
				State,
				OutTrace.DefendingSide,
				MarkerCardId))
			{
				return false;
			}
			Marker = Session.SubmitMarker(
				MakeMarkerRequest(OutTrace, MarkerCardId));
			if (!Marker.MarkerResult.bSuccess)
			{
				return false;
			}
		}

		FMatchPlayAuthoritativeSubmitSkillRequest SkillRequest;
		SkillRequest.RequestingSide = OutTrace.AttackingSide;
		SkillRequest.SkillId = SkillId;
		const auto SkillResult = Session.SubmitSkill(
			MakeSkillRuleSet(SkillId, SkillType),
			SkillRequest);
		if (!SkillResult.SkillResult.bSuccess
			|| Session.GetStateSnapshot().CurrentAttack.SelectionStage
				!= EMatchPlayCurrentAttackSelectionStage::AwaitingRunner)
		{
			return false;
		}

		const FMatchPlayState State = Session.GetStateSnapshot();
		const auto Availability =
			FMatchPlayCurrentAttackRunnerSelectionAvailability::Query(
				State,
				State.CurrentAttack.AttackSequence,
				OutTrace.AttackingSide);
		for (const auto& Candidate : Availability.Candidates)
		{
			if (Candidate.LegalityResult.bIsLegal)
			{
				OutRunnerCardId = Candidate.RunnerCardId;
				break;
			}
		}
		return Availability.bQuerySucceeded
			&& Availability.bCanSelectAnyRunner == bCreateLegalRunner
			&& (bCreateLegalRunner != OutRunnerCardId.IsNone());
	}

	bool BuildStage7163ToAwaitingHelper(
		FMatchPlayAuthoritativeSession& Session,
		const FString& Prefix,
		const bool bCreateLegalHelper,
		FReachabilityTrace& OutTrace,
		FName& OutHelperCardId,
		const ESkillRuleType SkillType = ESkillRuleType::PassControl)
	{
		FName RunnerCardId;
		if (!BuildStage7162ToAwaitingRunner(
			Session,
			Prefix,
			true,
			OutTrace,
			RunnerCardId,
			SkillType,
			bCreateLegalHelper ? 4 : 3))
		{
			return false;
		}

		FMatchPlayAuthoritativeSubmitRunnerRequest RunnerRequest;
		RunnerRequest.RequestingSide = OutTrace.AttackingSide;
		RunnerRequest.RunnerCardId = RunnerCardId;
		const auto RunnerResult = Session.SubmitRunner(RunnerRequest);
		if (!RunnerResult.RunnerResult.bSuccess
			|| Session.GetStateSnapshot().CurrentAttack.SelectionStage
				!= EMatchPlayCurrentAttackSelectionStage::AwaitingHelper)
		{
			return false;
		}

		const FMatchPlayState State = Session.GetStateSnapshot();
		const auto Availability =
			FMatchPlayCurrentAttackHelperSelectionAvailability::Query(
				State,
				State.CurrentAttack.AttackSequence,
				OutTrace.DefendingSide);
		for (const auto& Candidate : Availability.Candidates)
		{
			if (Candidate.LegalityResult.bSuccess)
			{
				OutHelperCardId = Candidate.HelperCardId;
				break;
			}
		}
		return Availability.bQuerySucceeded
			&& Availability.bCanSelectAnyHelper == bCreateLegalHelper
			&& (bCreateLegalHelper != OutHelperCardId.IsNone());
	}

	bool BuildStage7164ToReadyForResolution(
		FMatchPlayAuthoritativeSession& Session,
		const FString& Prefix,
		FReachabilityTrace& OutTrace,
		const ESkillRuleType SkillType = ESkillRuleType::PassControl)
	{
		FName HelperCardId;
		if (!BuildStage7163ToAwaitingHelper(
			Session,
			Prefix,
			true,
			OutTrace,
			HelperCardId,
			SkillType))
		{
			return false;
		}

		FMatchPlayAuthoritativeSubmitHelperRequest Request;
		Request.RequestingSide = OutTrace.DefendingSide;
		Request.HelperCardId = HelperCardId;
		const auto Result = Session.SubmitHelper(Request);
		const FMatchPlayState State = Session.GetStateSnapshot();
		return Result.HelperResult.bSuccess
			&& State.bHasCurrentAttack
			&& State.CurrentAttack.SelectionStage
				== EMatchPlayCurrentAttackSelectionStage::ReadyForResolution
			&& !State.CurrentAttack.bHasResolutionSession;
	}

	bool BuildStage7165ToAwaitingBranchIntent(
		FMatchPlayAuthoritativeSession& Session,
		const FString& Prefix,
		const ESkillRuleType SkillType,
		FReachabilityTrace& OutTrace)
	{
		if (SkillType == ESkillRuleType::Cross)
		{
			FName HelperCardId;
			if (!BuildStage7163ToAwaitingHelper(
				Session,
				Prefix,
				true,
				OutTrace,
				HelperCardId,
				SkillType))
			{
				return false;
			}

			FMatchPlayAuthoritativeSubmitHelperRequest HelperRequest;
			HelperRequest.RequestingSide = OutTrace.DefendingSide;
			HelperRequest.HelperCardId = HelperCardId;
			if (!Session.SubmitHelper(HelperRequest).HelperResult.bSuccess)
			{
				return false;
			}
		}
		else
		{
			if (SkillType != ESkillRuleType::LongShot
				&& SkillType != ESkillRuleType::CutInsideShot)
			{
				return false;
			}
			const FName SkillId(*FString::Printf(
				TEXT("Skill.%s.%d"),
				*Prefix,
				static_cast<int32>(SkillType)));
			FMatchPlayAuthoritativeSubmitMarkerResult Marker;
			if (!BuildFoundationBToAwaitingSkill(
				Session,
				Prefix,
				{SkillId},
				OutTrace,
				Marker))
			{
				return false;
			}

			FMatchPlayAuthoritativeSubmitSkillRequest SkillRequest;
			SkillRequest.RequestingSide = OutTrace.AttackingSide;
			SkillRequest.SkillId = SkillId;
			if (!Session.SubmitSkill(
				MakeSkillRuleSet(SkillId, SkillType),
				SkillRequest).SkillResult.bSuccess)
			{
				return false;
			}
		}

		const FMatchPlayState State = Session.GetStateSnapshot();
		return State.bHasCurrentAttack
			&& State.CurrentAttack.SelectionStage
				== EMatchPlayCurrentAttackSelectionStage
					::AwaitingBranchIntent
			&& State.CurrentAttack.ActionPreparation.ActionType
				== SkillType
			&& !State.CurrentAttack.bHasSelectedAction
			&& !State.CurrentAttack.bHasResolutionSession;
	}

	bool BuildStage7166ToAwaitingRoute(
		FMatchPlayAuthoritativeSession& Session,
		const FString& Prefix,
		const ESkillRuleType SkillType,
		const EMatchPlayElectiveBranchIntent Intent,
		FReachabilityTrace& OutTrace)
	{
		if (SkillType == ESkillRuleType::LongShot
			|| SkillType == ESkillRuleType::CutInsideShot
			|| SkillType == ESkillRuleType::Cross)
		{
			if (!BuildStage7165ToAwaitingBranchIntent(
				Session,
				Prefix,
				SkillType,
				OutTrace))
			{
				return false;
			}

			FMatchPlayAuthoritativeSubmitBranchIntentRequest IntentRequest;
			IntentRequest.RequestingSide = OutTrace.AttackingSide;
			IntentRequest.Intent = Intent;
			if (!Session.SubmitBranchIntent(IntentRequest).IntentResult.bSuccess)
			{
				return false;
			}
		}
		else if (SkillType == ESkillRuleType::PassControl
			|| SkillType == ESkillRuleType::ThroughBall)
		{
			if (!BuildStage7164ToReadyForResolution(
				Session,
				Prefix,
				OutTrace,
				SkillType))
			{
				return false;
			}
		}
		else
		{
			return false;
		}

		const auto BeginResult = Session.BeginResolutionSession();
		const FMatchPlayState State = Session.GetStateSnapshot();
		return BeginResult.BeginResult.bSuccess
			&& State.bHasCurrentAttack
			&& State.CurrentAttack.bHasResolutionSession
			&& State.CurrentAttack.ResolutionSession.Stage
				== EMatchPlayCurrentAttackResolutionStage::AwaitingRoute;
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

	struct FCompletionSemanticFixture
	{
		FMatchPlayState BeforeState;
		FMatchPlayCurrentAttackCompletionResult Completion;
		FMatchPlayState FinalState;
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

		auto SetSemanticScores = [](FCompletionSemanticFixture& Fixture,
			const int32 PlayerAAfterValue,
			const int32 PlayerBAfterValue)
		{
			Fixture.FinalState.RuntimeState.PlayerAState.Score = PlayerAAfterValue;
			Fixture.FinalState.RuntimeState.PlayerBState.Score = PlayerBAfterValue;
			Fixture.Completion.AfterState = Fixture.FinalState;
			Fixture.Completion.GoalResolveResult.PlayerAScoreAfter = PlayerAAfterValue;
			Fixture.Completion.GoalResolveResult.PlayerBScoreAfter = PlayerBAfterValue;
			Fixture.Completion.GoalResolveResult.UpdatedRuntimeState =
				Fixture.FinalState.RuntimeState;
		};
		auto GetBeforeScoreForSide = [](
			const FCompletionSemanticFixture& Fixture,
			const EInitialTurnOrderPlayer Side)
		{
			return Side == EInitialTurnOrderPlayer::PlayerA
				? Fixture.BeforeState.RuntimeState.PlayerAState.Score
				: Fixture.BeforeState.RuntimeState.PlayerBState.Score;
		};
		auto SetSemanticScoresForAttacker = [ExpectedScoringSide](
			FCompletionSemanticFixture& Fixture,
			const int32 AttackerAfterScore,
			const int32 DefenderAfterScore,
			auto SetScores)
		{
			if (ExpectedScoringSide == EInitialTurnOrderPlayer::PlayerA)
			{
				SetScores(Fixture, AttackerAfterScore, DefenderAfterScore);
			}
			else
			{
				SetScores(Fixture, DefenderAfterScore, AttackerAfterScore);
			}
		};

		FCompletionSemanticFixture Canonical;
		Canonical.BeforeState = BeforeState;
		Canonical.Completion = Completion;
		Canonical.FinalState = FinalState;
		Canonical.BeforeState.RuntimeState.PlayerAState.Score = 4;
		Canonical.BeforeState.RuntimeState.PlayerBState.Score = 2;
		Canonical.Completion.BeforeState = Canonical.BeforeState;
		Canonical.Completion.bSuccess = true;
		Canonical.Completion.ScoringSide = ContractScoringSide;
		Canonical.Completion.GoalResolveResult.bSuccess = bGoal;
		Canonical.Completion.GoalResolveResult.ScoringSide = ContractScoringSide;
		Canonical.Completion.GoalResolveResult.PlayerAScoreBefore = 4;
		Canonical.Completion.GoalResolveResult.PlayerBScoreBefore = 2;
		SetSemanticScores(
			Canonical,
			4 + (bGoal && ExpectedScoringSide == EInitialTurnOrderPlayer::PlayerA ? 1 : 0),
			2 + (bGoal && ExpectedScoringSide == EInitialTurnOrderPlayer::PlayerB ? 1 : 0));
		Canonical.Completion.NextAttackingPlayer =
			Canonical.FinalState.RuntimeState.CurrentAttackingPlayer;

		auto IsCanonicalSemantic = [&](const FCompletionSemanticFixture& Fixture)
		{
			return HasExpectedCompletionScoringSemantics(
				Fixture.BeforeState,
				Fixture.Completion,
				Fixture.FinalState,
				Expectation,
				ExpectedScoringSide);
		};
		auto TestSemanticRow = [&](const TCHAR* Row, auto Mutator)
		{
			FCompletionSemanticFixture Baseline = Canonical;
			FCompletionSemanticFixture MutatedFixture = Canonical;
			Test.TestTrue(
				*FString::Printf(TEXT("%s %s baseline PASS"), *Context, Row),
				IsCanonicalSemantic(Baseline));
			Mutator(MutatedFixture, SetSemanticScores);
			Test.TestFalse(
				*FString::Printf(TEXT("%s %s mutation FAIL"), *Context, Row),
				IsCanonicalSemantic(MutatedFixture));
			FCompletionSemanticFixture Restored = Canonical;
			Test.TestTrue(
				*FString::Printf(TEXT("%s %s restored PASS"), *Context, Row),
				IsCanonicalSemantic(Restored));
		};

		if (bGoal)
		{
			TestSemanticRow(TEXT("Goal -> NoGoal"),
				[](auto& Fixture, auto SetScores)
				{
					Fixture.Completion.GoalResolveResult.bSuccess = false;
					Fixture.Completion.ScoringSide = EInitialTurnOrderPlayer::None;
					Fixture.Completion.GoalResolveResult.ScoringSide =
						EInitialTurnOrderPlayer::None;
					SetScores(Fixture, 4, 2);
				});
			TestSemanticRow(TEXT("Goal scorer -> wrong side"),
				[ExpectedScoringSide](auto& Fixture, auto SetScores)
				{
					const EInitialTurnOrderPlayer WrongSide =
						OtherPlayer(ExpectedScoringSide);
					Fixture.Completion.ScoringSide = WrongSide;
					Fixture.Completion.GoalResolveResult.ScoringSide = WrongSide;
					SetScores(
						Fixture,
						4 + (WrongSide == EInitialTurnOrderPlayer::PlayerA ? 1 : 0),
						2 + (WrongSide == EInitialTurnOrderPlayer::PlayerB ? 1 : 0));
				});
			TestSemanticRow(TEXT("Goal score +1 -> 0"),
				[ExpectedScoringSide, GetBeforeScoreForSide,
					SetSemanticScoresForAttacker](auto& Fixture, auto SetScores)
				{
					const int32 AttackerBeforeScore = GetBeforeScoreForSide(
						Fixture,
						ExpectedScoringSide);
					const int32 DefenderBeforeScore = GetBeforeScoreForSide(
						Fixture,
						OtherPlayer(ExpectedScoringSide));
					SetSemanticScoresForAttacker(
						Fixture,
						AttackerBeforeScore,
						DefenderBeforeScore,
						SetScores);
				});
		}
		else
		{
			TestSemanticRow(TEXT("NoGoal -> Goal"),
				[ExpectedScoringSide](auto& Fixture, auto SetScores)
				{
					Fixture.Completion.GoalResolveResult.bSuccess = true;
					Fixture.Completion.ScoringSide = ExpectedScoringSide;
					Fixture.Completion.GoalResolveResult.ScoringSide =
						ExpectedScoringSide;
					SetScores(
						Fixture,
						4 + (ExpectedScoringSide == EInitialTurnOrderPlayer::PlayerA ? 1 : 0),
						2 + (ExpectedScoringSide == EInitialTurnOrderPlayer::PlayerB ? 1 : 0));
				});
			TestSemanticRow(TEXT("NoGoal None -> attacker"),
				[ExpectedScoringSide](auto& Fixture, auto SetScores)
				{
					Fixture.Completion.ScoringSide = ExpectedScoringSide;
					Fixture.Completion.GoalResolveResult.ScoringSide =
						ExpectedScoringSide;
					SetScores(Fixture, 4, 2);
				});
			TestSemanticRow(TEXT("NoGoal score 0 -> +1"),
				[ExpectedScoringSide, GetBeforeScoreForSide,
					SetSemanticScoresForAttacker](auto& Fixture, auto SetScores)
				{
					const int32 AttackerBeforeScore = GetBeforeScoreForSide(
						Fixture,
						ExpectedScoringSide);
					const int32 DefenderBeforeScore = GetBeforeScoreForSide(
						Fixture,
						OtherPlayer(ExpectedScoringSide));
					SetSemanticScoresForAttacker(
						Fixture,
						AttackerBeforeScore + 1,
						DefenderBeforeScore,
						SetScores);
				});
		}
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
		int64 IndependentBaselineCandidatePairs = 0;
		int64 AddressInequalityChecks = 0;
		int64 PreMutationComparatorTrueChecks = 0;
		int64 MutationComparatorFalseChecks = 0;
		int64 RestorationComparatorTrueChecks = 0;
		TSet<FString> ExpectedLogicalPaths;
		TSet<FString> CoveredLogicalPaths;
		TSet<FString> ReflectedLeafPaths;
		TSet<FString> NonReflectedFieldPaths;
		TSet<FString> ContainerCasePaths;
		TSet<FString> EmptyContainerElementFieldPaths;
		TSet<FString> EnvelopePaths;
		TSet<FString> SkippedGroupPaths;
		TSet<FString> ExpectedNonReflectedSchemaFields;
		TSet<FString> CoveredNonReflectedSchemaFields;
		TSet<FString> ExpectedReachableMapPaths;
		TSet<FString> CoveredReachableMapPaths;
		TMap<FString, int32> ExpectedRegistrationCountByPath;
		TMap<FString, int32> ExecutionCountByPath;

		void Expect(const FString& Path)
		{
			ExpectedLogicalPaths.Add(Path);
			++ExpectedRegistrationCountByPath.FindOrAdd(Path);
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

		void RecordIndependentBaselineGate(
			const bool bAddressInequal,
			const bool bPreMutationEqual,
			const bool bMutationRejected,
			const bool bRestoredEqual)
		{
			++IndependentBaselineCandidatePairs;
			AddressInequalityChecks += bAddressInequal ? 1 : 0;
			PreMutationComparatorTrueChecks += bPreMutationEqual ? 1 : 0;
			MutationComparatorFalseChecks += bMutationRejected ? 1 : 0;
			RestorationComparatorTrueChecks += bRestoredEqual ? 1 : 0;
		}

		void Skip(const FString& Path)
		{
			SkippedGroupPaths.Add(Path);
		}
	};

	template <typename TValue, typename TComparator>
	bool TestIndependentMutationGate(
		FAutomationTestBase& Test,
		FMutationCoverageAudit& Audit,
		const FString& Path,
		const TValue& Baseline,
		const TValue& Mutated,
		TComparator Comparator)
	{
		TValue IndependentCandidate = Baseline;
		const bool bAddressInequal =
			&Baseline != &IndependentCandidate
			&& &Baseline != &Mutated
			&& &IndependentCandidate != &Mutated;
		const bool bPreMutationEqual =
			Comparator(Baseline, IndependentCandidate);
		const bool bMutationRejected = !Comparator(Baseline, Mutated);
		IndependentCandidate = Baseline;
		const bool bRestoredEqual =
			Comparator(Baseline, IndependentCandidate);

		Test.TestTrue(
			*FString::Printf(TEXT("Independent addresses: %s"), *Path),
			bAddressInequal);
		Test.TestTrue(
			*FString::Printf(TEXT("Canonical equality before mutation: %s"), *Path),
			bPreMutationEqual);
		Test.TestTrue(
			*FString::Printf(TEXT("Mutation is rejected: %s"), *Path),
			bMutationRejected);
		Test.TestTrue(
			*FString::Printf(TEXT("Canonical equality after restoration: %s"), *Path),
			bRestoredEqual);
		Audit.RecordIndependentBaselineGate(
			bAddressInequal,
			bPreMutationEqual,
			bMutationRejected,
			bRestoredEqual);
		return bAddressInequal
			&& bPreMutationEqual
			&& bMutationRejected
			&& bRestoredEqual;
	}

	template <typename TValue, typename TComparator>
	bool TestIndependentCanonicalEquality(
		FAutomationTestBase& Test,
		const FString& Context,
		const TValue& Baseline,
		TComparator Comparator,
		FMutationCoverageAudit* Audit = nullptr)
	{
		TValue Candidate = Baseline;
		const bool bAddressInequal = &Baseline != &Candidate;
		const bool bEqual = Comparator(Baseline, Candidate);
		Test.TestTrue(
			*FString::Printf(TEXT("%s uses independent objects"), *Context),
			bAddressInequal);
		Test.TestTrue(
			*FString::Printf(TEXT("%s canonical values compare equal"), *Context),
			bEqual);
		if (Audit != nullptr && bAddressInequal && bEqual)
		{
			Audit->RecordEqualBaseline();
		}
		return bAddressInequal && bEqual;
	}

	void RegisterExpectedReflectedPaths(
		UStruct* Struct,
		void* Container,
		const FString& Prefix,
		FMutationCoverageAudit& Audit,
		bool bEmptyContainerElement = false);

	void RegisterExpectedReflectedPropertyPath(
		FProperty* Property,
		void* Value,
		const FString& Path,
		FMutationCoverageAudit& Audit,
		const bool bEmptyContainerElement)
	{
		if (FStructProperty* StructProperty = CastField<FStructProperty>(Property))
		{
			RegisterExpectedReflectedPaths(
				StructProperty->Struct,
				Value,
				Path,
				Audit,
				bEmptyContainerElement);
			return;
		}

		if (FMapProperty* MapProperty = CastField<FMapProperty>(Property))
		{
			Audit.ExpectedReachableMapPaths.Add(Path);
			Audit.Expect(Path + TEXT(".Membership"));
			FScriptMapHelper Map(MapProperty, Value);
			TArray<int32> Indices;
			for (int32 Index = 0; Index < Map.GetMaxIndex(); ++Index)
			{
				if (Map.IsValidIndex(Index))
				{
					Indices.Add(Index);
				}
			}
			bool bAddedCanonicalElement = false;
			if (Indices.IsEmpty())
			{
				const int32 Added = Map.AddDefaultValue_Invalid_NeedsRehash();
				Map.Rehash();
				Indices.Add(Added);
				bAddedCanonicalElement = true;
			}
			TArray<int32> Positions = {0};
			if (Indices.Num() >= 3)
			{
				Positions.Add(Indices.Num() / 2);
			}
			if (Indices.Num() >= 2)
			{
				Positions.Add(Indices.Num() - 1);
			}
			for (const int32 Position : Positions)
			{
				const int32 Index = Indices[Position];
				RegisterExpectedReflectedPropertyPath(
					MapProperty->KeyProp,
					Map.GetKeyPtr(Index),
					bAddedCanonicalElement
						? FString::Printf(TEXT("%s.EmptyElement.Key[%d]"), *Path, Position)
						: FString::Printf(TEXT("%s.Key[%d]"), *Path, Position),
					Audit,
					bAddedCanonicalElement || bEmptyContainerElement);
				RegisterExpectedReflectedPropertyPath(
					MapProperty->ValueProp,
					Map.GetValuePtr(Index),
					bAddedCanonicalElement
						? FString::Printf(TEXT("%s.EmptyElement.Value[%d]"), *Path, Position)
						: FString::Printf(TEXT("%s.Value[%d]"), *Path, Position),
					Audit,
					bAddedCanonicalElement || bEmptyContainerElement);
			}
			if (bAddedCanonicalElement)
			{
				Map.EmptyValues();
			}
			return;
		}

		if (FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Property))
		{
			Audit.Expect(Path + TEXT(".Num"));
			FScriptArrayHelper Array(ArrayProperty, Value);
			bool bAddedCanonicalElement = false;
			if (Array.Num() == 0)
			{
				Array.AddValue();
				bAddedCanonicalElement = true;
			}
			TArray<int32> Indices = {0};
			if (Array.Num() >= 3)
			{
				Indices.Add(Array.Num() / 2);
			}
			if (Array.Num() >= 2)
			{
				Indices.Add(Array.Num() - 1);
			}
			for (const int32 Index : Indices)
			{
				RegisterExpectedReflectedPropertyPath(
					ArrayProperty->Inner,
					Array.GetRawPtr(Index),
					bAddedCanonicalElement
						? FString::Printf(TEXT("%s.EmptyElement[%d]"), *Path, Index)
						: FString::Printf(TEXT("%s[%d]"), *Path, Index),
					Audit,
					bAddedCanonicalElement || bEmptyContainerElement);
			}
			if (Array.Num() >= 2
				&& !ArrayProperty->Inner->Identical(
					Array.GetRawPtr(0), Array.GetRawPtr(Array.Num() - 1)))
			{
				Audit.Expect(Path + TEXT(".Order"));
			}
			if (bAddedCanonicalElement)
			{
				Array.RemoveValues(0, 1);
			}
			return;
		}

		Audit.Expect(Path);
	}

	void RegisterExpectedReflectedPaths(
		UStruct* Struct,
		void* Container,
		const FString& Prefix,
		FMutationCoverageAudit& Audit,
		const bool bEmptyContainerElement)
	{
		for (TFieldIterator<FProperty> It(Struct); It; ++It)
		{
			FProperty* Property = *It;
			RegisterExpectedReflectedPropertyPath(
				Property,
				Property->ContainerPtrToValuePtr<void>(Container),
				Prefix + TEXT(".") + Property->GetName(),
				Audit,
				bEmptyContainerElement);
		}
	}

	template <typename TWrapper, typename TNested, typename TAccessor>
	void RegisterExpectedNestedReflectedPaths(
		FMutationCoverageAudit& Audit,
		const FString& Prefix,
		const TWrapper& InputBaseline,
		TAccessor Accessor,
		const bool bEmptyContainerElement = false)
	{
		TWrapper SchemaFixture = InputBaseline;
		TNested& Nested = Accessor(SchemaFixture);
		RegisterExpectedReflectedPaths(
			TNested::StaticStruct(),
			&Nested,
			Prefix,
			Audit,
			bEmptyContainerElement);
	}

	void RegisterExpectedEnvelopePaths(
		FMutationCoverageAudit& Audit,
		const FString& Prefix)
	{
		for (const TCHAR* Field : {
			TEXT("bAccepted"),
			TEXT("bDomainSuccess"),
			TEXT("bStateAdvanced"),
			TEXT("StateDisposition"),
			TEXT("bRuntimeFault"),
			TEXT("BeforeState"),
			TEXT("AfterState"),
			TEXT("CommandKind"),
			TEXT("AttackSequence"),
			TEXT("FailureDisposition"),
			TEXT("RuntimeFailureCode"),
			TEXT("ErrorMessage")})
		{
			Audit.Expect(Prefix + TEXT(".") + Field);
		}
	}

	struct FCanonicalBuilderAudit
	{
		TSet<FString> ExpectedFields;
		TSet<FString> ExplicitlyInitializedFields;
		TSet<FString> UnsupportedFields;

		void Expect(const FString& Path)
		{
			ExpectedFields.Add(Path);
		}

		void Initialized(const FString& Path)
		{
			ExplicitlyInitializedFields.Add(Path);
		}
	};

	void RegisterCanonicalReflectedSchema(
		UStruct* Struct,
		const FString& Prefix,
		FCanonicalBuilderAudit& Audit)
	{
		for (TFieldIterator<FProperty> It(Struct); It; ++It)
		{
			FProperty* Property = *It;
			const FString Path = Prefix + TEXT(".") + Property->GetName();
			if (FStructProperty* StructProperty = CastField<FStructProperty>(Property))
			{
				RegisterCanonicalReflectedSchema(
					StructProperty->Struct, Path, Audit);
			}
			else
			{
				Audit.Expect(Path);
			}
		}
	}

	void InitializeCanonicalReflectedProperty(
		FProperty* Property,
		void* Value,
		const FString& Path,
		const int32 Seed,
		FCanonicalBuilderAudit& Audit);

	void InitializeCanonicalReflectedStruct(
		UStruct* Struct,
		void* Container,
		const FString& Prefix,
		const int32 Seed,
		FCanonicalBuilderAudit& Audit)
	{
		int32 Offset = 0;
		for (TFieldIterator<FProperty> It(Struct); It; ++It, ++Offset)
		{
			FProperty* Property = *It;
			InitializeCanonicalReflectedProperty(
				Property,
				Property->ContainerPtrToValuePtr<void>(Container),
				Prefix + TEXT(".") + Property->GetName(),
				Seed * 37 + Offset + 1,
				Audit);
		}
	}

	void InitializeCanonicalReflectedProperty(
		FProperty* Property,
		void* Value,
		const FString& Path,
		const int32 Seed,
		FCanonicalBuilderAudit& Audit)
	{
		if (FStructProperty* StructProperty = CastField<FStructProperty>(Property))
		{
			InitializeCanonicalReflectedStruct(
				StructProperty->Struct, Value, Path, Seed, Audit);
			return;
		}
		if (FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Property))
		{
			FScriptArrayHelper Array(ArrayProperty, Value);
			Array.EmptyValues();
			for (int32 Index = 0; Index < 3; ++Index)
			{
				const int32 Added = Array.AddValue();
				InitializeCanonicalReflectedProperty(
					ArrayProperty->Inner,
					Array.GetRawPtr(Added),
					Path,
					Seed * 11 + Index + 1,
					Audit);
			}
			Audit.Initialized(Path);
			return;
		}
		if (FMapProperty* MapProperty = CastField<FMapProperty>(Property))
		{
			FScriptMapHelper Map(MapProperty, Value);
			Map.EmptyValues();
			for (int32 Index = 0; Index < 3; ++Index)
			{
				const int32 Added = Map.AddDefaultValue_Invalid_NeedsRehash();
				InitializeCanonicalReflectedProperty(
					MapProperty->KeyProp,
					Map.GetKeyPtr(Added),
					Path,
					Seed * 13 + Index + 1,
					Audit);
				InitializeCanonicalReflectedProperty(
					MapProperty->ValueProp,
					Map.GetValuePtr(Added),
					Path,
					Seed * 17 + Index + 1,
					Audit);
			}
			Map.Rehash();
			Audit.Initialized(Path);
			return;
		}
		if (FBoolProperty* BoolProperty = CastField<FBoolProperty>(Property))
		{
			BoolProperty->SetPropertyValue(Value, Seed % 2 == 0);
		}
		else if (FEnumProperty* EnumProperty = CastField<FEnumProperty>(Property))
		{
			const UEnum* Enum = EnumProperty->GetEnum();
			const int32 ValueIndex = Enum->NumEnums() > 1 ? 1 : 0;
			EnumProperty->GetUnderlyingProperty()->SetIntPropertyValue(
				Value, Enum->GetValueByIndex(ValueIndex));
		}
		else if (FNumericProperty* NumericProperty = CastField<FNumericProperty>(Property))
		{
			if (NumericProperty->IsFloatingPoint())
			{
				NumericProperty->SetFloatingPointPropertyValue(
					Value, static_cast<double>(FMath::Abs(Seed) + 1));
			}
			else
			{
				NumericProperty->SetIntPropertyValue(
					Value, static_cast<int64>(FMath::Abs(Seed) + 1));
			}
		}
		else if (FNameProperty* NameProperty = CastField<FNameProperty>(Property))
		{
			NameProperty->SetPropertyValue(
				Value,
				FName(*FString::Printf(TEXT("Canonical.%d"), Seed)));
		}
		else if (FStrProperty* StringProperty = CastField<FStrProperty>(Property))
		{
			StringProperty->SetPropertyValue(
				Value,
				FString::Printf(TEXT("Canonical message %d"), Seed));
		}
		else if (FTextProperty* TextProperty = CastField<FTextProperty>(Property))
		{
			TextProperty->SetPropertyValue(
				Value,
				FText::FromString(FString::Printf(TEXT("Canonical text %d"), Seed)));
		}
		else
		{
			Audit.UnsupportedFields.Add(Path);
			return;
		}
		Audit.Initialized(Path);
	}

	void ExpectCanonicalManualFields(
		FCanonicalBuilderAudit& Audit,
		const FString& Prefix,
		std::initializer_list<const TCHAR*> Fields)
	{
		for (const TCHAR* Field : Fields)
		{
			Audit.Expect(Prefix + TEXT(".") + Field);
		}
	}

	void MarkCanonicalManualField(
		FCanonicalBuilderAudit& Audit,
		const FString& Prefix,
		const TCHAR* Field)
	{
		Audit.Initialized(Prefix + TEXT(".") + Field);
	}

	void ExpectCanonicalPlayerCardValidation(
		FCanonicalBuilderAudit& Audit,
		const FString& Prefix)
	{
		ExpectCanonicalManualFields(Audit, Prefix, {
			TEXT("bSuccess"), TEXT("bIsValid"), TEXT("ErrorCode"),
			TEXT("ErrorMessage"), TEXT("InvalidCardId"),
			TEXT("DuplicateCardIds")});
	}

	void InitializeCanonicalPlayerCardValidation(
		FPlayerCardRuleSnapshotValidationResult& Value,
		const FString& Prefix,
		const int32 Seed,
		FCanonicalBuilderAudit& Audit)
	{
		Value.bSuccess = true;
		MarkCanonicalManualField(Audit, Prefix, TEXT("bSuccess"));
		Value.bIsValid = false;
		MarkCanonicalManualField(Audit, Prefix, TEXT("bIsValid"));
		Value.ErrorCode = EPlayerCardRuleSnapshotValidationErrorCode::InvalidCardId;
		MarkCanonicalManualField(Audit, Prefix, TEXT("ErrorCode"));
		Value.ErrorMessage = FString::Printf(TEXT("Canonical validation %d"), Seed);
		MarkCanonicalManualField(Audit, Prefix, TEXT("ErrorMessage"));
		Value.InvalidCardId = FName(*FString::Printf(TEXT("Canonical.Invalid.%d"), Seed));
		MarkCanonicalManualField(Audit, Prefix, TEXT("InvalidCardId"));
		Value.DuplicateCardIds = {
			FName(*FString::Printf(TEXT("Canonical.Duplicate.%d.0"), Seed)),
			FName(*FString::Printf(TEXT("Canonical.Duplicate.%d.1"), Seed)),
			FName(*FString::Printf(TEXT("Canonical.Duplicate.%d.2"), Seed))};
		MarkCanonicalManualField(Audit, Prefix, TEXT("DuplicateCardIds"));
	}

	void ExpectCanonicalPlayerCardQuery(
		FCanonicalBuilderAudit& Audit,
		const FString& Prefix)
	{
		ExpectCanonicalManualFields(Audit, Prefix, {
			TEXT("bSuccess"), TEXT("bFound"), TEXT("ErrorCode"),
			TEXT("ErrorMessage"), TEXT("CardId"), TEXT("Snapshot"),
			TEXT("ValidationResult")});
		RegisterCanonicalReflectedSchema(
			FPlayerCardRuleSnapshot::StaticStruct(), Prefix + TEXT(".Snapshot"), Audit);
		ExpectCanonicalPlayerCardValidation(
			Audit, Prefix + TEXT(".ValidationResult"));
	}

	void InitializeCanonicalPlayerCardQuery(
		FPlayerCardRuleSnapshotQueryResult& Value,
		const FString& Prefix,
		const int32 Seed,
		FCanonicalBuilderAudit& Audit)
	{
		Value.bSuccess = true;
		MarkCanonicalManualField(Audit, Prefix, TEXT("bSuccess"));
		Value.bFound = false;
		MarkCanonicalManualField(Audit, Prefix, TEXT("bFound"));
		Value.ErrorCode = EPlayerCardRuleSnapshotQueryErrorCode::CardNotFound;
		MarkCanonicalManualField(Audit, Prefix, TEXT("ErrorCode"));
		Value.ErrorMessage = FString::Printf(TEXT("Canonical query %d"), Seed);
		MarkCanonicalManualField(Audit, Prefix, TEXT("ErrorMessage"));
		Value.CardId = FName(*FString::Printf(TEXT("Canonical.Query.%d"), Seed));
		MarkCanonicalManualField(Audit, Prefix, TEXT("CardId"));
		InitializeCanonicalReflectedStruct(
			FPlayerCardRuleSnapshot::StaticStruct(),
			&Value.Snapshot,
			Prefix + TEXT(".Snapshot"),
			Seed + 1,
			Audit);
		MarkCanonicalManualField(Audit, Prefix, TEXT("Snapshot"));
		InitializeCanonicalPlayerCardValidation(
			Value.ValidationResult,
			Prefix + TEXT(".ValidationResult"),
			Seed + 2,
			Audit);
		MarkCanonicalManualField(Audit, Prefix, TEXT("ValidationResult"));
	}

	void ExpectCanonicalCardSnapshotQuery(
		FCanonicalBuilderAudit& Audit,
		const FString& Prefix)
	{
		ExpectCanonicalManualFields(Audit, Prefix, {
			TEXT("bSuccess"), TEXT("ErrorCode"), TEXT("PlayerSide"),
			TEXT("CardId"), TEXT("Snapshot"), TEXT("UnderlyingQueryResult"),
			TEXT("ErrorMessage")});
		RegisterCanonicalReflectedSchema(
			FPlayerCardRuleSnapshot::StaticStruct(), Prefix + TEXT(".Snapshot"), Audit);
		ExpectCanonicalPlayerCardQuery(
			Audit, Prefix + TEXT(".UnderlyingQueryResult"));
	}

	void InitializeCanonicalCardSnapshotQuery(
		FMatchPlayCardSnapshotAuthorityQueryResult& Value,
		const FString& Prefix,
		const int32 Seed,
		FCanonicalBuilderAudit& Audit)
	{
		Value.bSuccess = true;
		MarkCanonicalManualField(Audit, Prefix, TEXT("bSuccess"));
		Value.ErrorCode = EMatchPlayCardSnapshotAuthorityQueryErrorCode::SnapshotNotFound;
		MarkCanonicalManualField(Audit, Prefix, TEXT("ErrorCode"));
		Value.PlayerSide = Seed % 2 == 0
			? EInitialTurnOrderPlayer::PlayerA
			: EInitialTurnOrderPlayer::PlayerB;
		MarkCanonicalManualField(Audit, Prefix, TEXT("PlayerSide"));
		Value.CardId = FName(*FString::Printf(TEXT("Canonical.Authority.%d"), Seed));
		MarkCanonicalManualField(Audit, Prefix, TEXT("CardId"));
		InitializeCanonicalReflectedStruct(
			FPlayerCardRuleSnapshot::StaticStruct(),
			&Value.Snapshot,
			Prefix + TEXT(".Snapshot"),
			Seed + 1,
			Audit);
		MarkCanonicalManualField(Audit, Prefix, TEXT("Snapshot"));
		InitializeCanonicalPlayerCardQuery(
			Value.UnderlyingQueryResult,
			Prefix + TEXT(".UnderlyingQueryResult"),
			Seed + 2,
			Audit);
		MarkCanonicalManualField(Audit, Prefix, TEXT("UnderlyingQueryResult"));
		Value.ErrorMessage = FString::Printf(TEXT("Canonical authority %d"), Seed);
		MarkCanonicalManualField(Audit, Prefix, TEXT("ErrorMessage"));
	}

	void ExpectCanonicalSkillValidation(
		FCanonicalBuilderAudit& Audit,
		const FString& Prefix)
	{
		ExpectCanonicalManualFields(Audit, Prefix, {
			TEXT("bSuccess"), TEXT("bIsValid"), TEXT("ErrorCode"),
			TEXT("ErrorMessage"), TEXT("InvalidSkillId"), TEXT("InvalidField")});
	}

	void InitializeCanonicalSkillValidation(
		FSkillRuleSnapshotValidationResult& Value,
		const FString& Prefix,
		const int32 Seed,
		FCanonicalBuilderAudit& Audit)
	{
		Value.bSuccess = true;
		MarkCanonicalManualField(Audit, Prefix, TEXT("bSuccess"));
		Value.bIsValid = false;
		MarkCanonicalManualField(Audit, Prefix, TEXT("bIsValid"));
		Value.ErrorCode = ESkillRuleSnapshotValidationErrorCode::InvalidSkillId;
		MarkCanonicalManualField(Audit, Prefix, TEXT("ErrorCode"));
		Value.ErrorMessage = FString::Printf(TEXT("Canonical skill validation %d"), Seed);
		MarkCanonicalManualField(Audit, Prefix, TEXT("ErrorMessage"));
		Value.InvalidSkillId = FName(*FString::Printf(TEXT("Canonical.Skill.Invalid.%d"), Seed));
		MarkCanonicalManualField(Audit, Prefix, TEXT("InvalidSkillId"));
		Value.InvalidField = FName(*FString::Printf(TEXT("Canonical.Field.%d"), Seed));
		MarkCanonicalManualField(Audit, Prefix, TEXT("InvalidField"));
	}

	void ExpectCanonicalSkillRule(
		FCanonicalBuilderAudit& Audit,
		const FString& Prefix)
	{
		ExpectCanonicalManualFields(Audit, Prefix, {
			TEXT("SkillId"), TEXT("SkillType"),
			TEXT("MinTriggerActionPoint"), TEXT("MaxTriggerActionPoint")});
	}

	void InitializeCanonicalSkillRule(
		FSkillRuleSnapshot& Value,
		const FString& Prefix,
		const int32 Seed,
		FCanonicalBuilderAudit& Audit)
	{
		Value.SkillId = FName(*FString::Printf(TEXT("Canonical.Skill.%d"), Seed));
		MarkCanonicalManualField(Audit, Prefix, TEXT("SkillId"));
		Value.SkillType = ESkillRuleType::LongShot;
		MarkCanonicalManualField(Audit, Prefix, TEXT("SkillType"));
		Value.MinTriggerActionPoint = 2 + Seed % 2;
		MarkCanonicalManualField(Audit, Prefix, TEXT("MinTriggerActionPoint"));
		Value.MaxTriggerActionPoint = 7 + Seed % 2;
		MarkCanonicalManualField(Audit, Prefix, TEXT("MaxTriggerActionPoint"));
	}

	void ExpectCanonicalSkillQuery(
		FCanonicalBuilderAudit& Audit,
		const FString& Prefix)
	{
		ExpectCanonicalManualFields(Audit, Prefix, {
			TEXT("bSuccess"), TEXT("bFound"), TEXT("ErrorCode"),
			TEXT("ErrorMessage"), TEXT("InvalidSkillId"), TEXT("InvalidField"),
			TEXT("Snapshot"), TEXT("ValidationResult")});
		ExpectCanonicalSkillRule(Audit, Prefix + TEXT(".Snapshot"));
		ExpectCanonicalSkillValidation(Audit, Prefix + TEXT(".ValidationResult"));
	}

	void InitializeCanonicalSkillQuery(
		FSkillRuleSnapshotQueryResult& Value,
		const FString& Prefix,
		const int32 Seed,
		FCanonicalBuilderAudit& Audit)
	{
		Value.bSuccess = true;
		MarkCanonicalManualField(Audit, Prefix, TEXT("bSuccess"));
		Value.bFound = false;
		MarkCanonicalManualField(Audit, Prefix, TEXT("bFound"));
		Value.ErrorCode = ESkillRuleSnapshotQueryErrorCode::SkillRuleNotFound;
		MarkCanonicalManualField(Audit, Prefix, TEXT("ErrorCode"));
		Value.ErrorMessage = FString::Printf(TEXT("Canonical skill query %d"), Seed);
		MarkCanonicalManualField(Audit, Prefix, TEXT("ErrorMessage"));
		Value.InvalidSkillId = FName(*FString::Printf(TEXT("Canonical.InvalidSkill.%d"), Seed));
		MarkCanonicalManualField(Audit, Prefix, TEXT("InvalidSkillId"));
		Value.InvalidField = FName(*FString::Printf(TEXT("Canonical.InvalidField.%d"), Seed));
		MarkCanonicalManualField(Audit, Prefix, TEXT("InvalidField"));
		InitializeCanonicalSkillRule(
			Value.Snapshot, Prefix + TEXT(".Snapshot"), Seed + 1, Audit);
		MarkCanonicalManualField(Audit, Prefix, TEXT("Snapshot"));
		InitializeCanonicalSkillValidation(
			Value.ValidationResult,
			Prefix + TEXT(".ValidationResult"),
			Seed + 2,
			Audit);
		MarkCanonicalManualField(Audit, Prefix, TEXT("ValidationResult"));
	}

	void ExpectCanonicalSkillGlobal(
		FCanonicalBuilderAudit& Audit,
		const FString& Prefix)
	{
		ExpectCanonicalManualFields(Audit, Prefix, {
			TEXT("bSuccess"), TEXT("RequestedAttackSequence"),
			TEXT("RequestingSide"), TEXT("ErrorCode"),
			TEXT("AuthoritativeAttackSequence"), TEXT("CurrentAttackingPlayer"),
			TEXT("CurrentDefendingPlayer"), TEXT("SelectionStateValidationResult"),
			TEXT("FrozenCarrierCardId"), TEXT("FrozenMarkerCardId"),
			TEXT("MatchingFrozenCarrierPlacementCount"),
			TEXT("MatchingFrozenMarkerPlacementCount"),
			TEXT("FrozenCarrierPlacement"), TEXT("FrozenMarkerPlacement"),
			TEXT("CarrierSnapshotQueryResult"), TEXT("ResolvedCarrierSnapshot"),
			TEXT("SkillRuleSetValidationResult"), TEXT("ValidatedActionPoint"),
			TEXT("ErrorMessage")});
		RegisterCanonicalReflectedSchema(
			FMatchPlayCurrentAttackSelectionStateValidationResult::StaticStruct(),
			Prefix + TEXT(".SelectionStateValidationResult"), Audit);
		RegisterCanonicalReflectedSchema(
			FMatchPlayDeploymentPlacement::StaticStruct(),
			Prefix + TEXT(".FrozenCarrierPlacement"), Audit);
		RegisterCanonicalReflectedSchema(
			FMatchPlayDeploymentPlacement::StaticStruct(),
			Prefix + TEXT(".FrozenMarkerPlacement"), Audit);
		ExpectCanonicalCardSnapshotQuery(
			Audit, Prefix + TEXT(".CarrierSnapshotQueryResult"));
		RegisterCanonicalReflectedSchema(
			FPlayerCardRuleSnapshot::StaticStruct(),
			Prefix + TEXT(".ResolvedCarrierSnapshot"), Audit);
		ExpectCanonicalSkillValidation(
			Audit, Prefix + TEXT(".SkillRuleSetValidationResult"));
	}

	void InitializeCanonicalSkillGlobal(
		FMatchPlayCurrentAttackSkillSelectionGlobalContextResult& Value,
		const FString& Prefix,
		const int32 Seed,
		FCanonicalBuilderAudit& Audit)
	{
		Value.bSuccess = true;
		MarkCanonicalManualField(Audit, Prefix, TEXT("bSuccess"));
		Value.RequestedAttackSequence = 100 + Seed;
		MarkCanonicalManualField(Audit, Prefix, TEXT("RequestedAttackSequence"));
		Value.RequestingSide = EInitialTurnOrderPlayer::PlayerA;
		MarkCanonicalManualField(Audit, Prefix, TEXT("RequestingSide"));
		Value.ErrorCode = EMatchPlayCurrentAttackSkillSelectionErrorCode::InvalidSkillId;
		MarkCanonicalManualField(Audit, Prefix, TEXT("ErrorCode"));
		Value.AuthoritativeAttackSequence = 200 + Seed;
		MarkCanonicalManualField(Audit, Prefix, TEXT("AuthoritativeAttackSequence"));
		Value.CurrentAttackingPlayer = EInitialTurnOrderPlayer::PlayerA;
		MarkCanonicalManualField(Audit, Prefix, TEXT("CurrentAttackingPlayer"));
		Value.CurrentDefendingPlayer = EInitialTurnOrderPlayer::PlayerB;
		MarkCanonicalManualField(Audit, Prefix, TEXT("CurrentDefendingPlayer"));
		InitializeCanonicalReflectedStruct(
			FMatchPlayCurrentAttackSelectionStateValidationResult::StaticStruct(),
			&Value.SelectionStateValidationResult,
			Prefix + TEXT(".SelectionStateValidationResult"), Seed + 1, Audit);
		MarkCanonicalManualField(Audit, Prefix, TEXT("SelectionStateValidationResult"));
		Value.FrozenCarrierCardId = FName(*FString::Printf(TEXT("Canonical.Carrier.%d"), Seed));
		MarkCanonicalManualField(Audit, Prefix, TEXT("FrozenCarrierCardId"));
		Value.FrozenMarkerCardId = FName(*FString::Printf(TEXT("Canonical.Marker.%d"), Seed));
		MarkCanonicalManualField(Audit, Prefix, TEXT("FrozenMarkerCardId"));
		Value.MatchingFrozenCarrierPlacementCount = 2 + Seed;
		MarkCanonicalManualField(Audit, Prefix, TEXT("MatchingFrozenCarrierPlacementCount"));
		Value.MatchingFrozenMarkerPlacementCount = 3 + Seed;
		MarkCanonicalManualField(Audit, Prefix, TEXT("MatchingFrozenMarkerPlacementCount"));
		InitializeCanonicalReflectedStruct(
			FMatchPlayDeploymentPlacement::StaticStruct(),
			&Value.FrozenCarrierPlacement,
			Prefix + TEXT(".FrozenCarrierPlacement"), Seed + 2, Audit);
		MarkCanonicalManualField(Audit, Prefix, TEXT("FrozenCarrierPlacement"));
		InitializeCanonicalReflectedStruct(
			FMatchPlayDeploymentPlacement::StaticStruct(),
			&Value.FrozenMarkerPlacement,
			Prefix + TEXT(".FrozenMarkerPlacement"), Seed + 3, Audit);
		MarkCanonicalManualField(Audit, Prefix, TEXT("FrozenMarkerPlacement"));
		InitializeCanonicalCardSnapshotQuery(
			Value.CarrierSnapshotQueryResult,
			Prefix + TEXT(".CarrierSnapshotQueryResult"), Seed + 4, Audit);
		MarkCanonicalManualField(Audit, Prefix, TEXT("CarrierSnapshotQueryResult"));
		InitializeCanonicalReflectedStruct(
			FPlayerCardRuleSnapshot::StaticStruct(),
			&Value.ResolvedCarrierSnapshot,
			Prefix + TEXT(".ResolvedCarrierSnapshot"), Seed + 5, Audit);
		MarkCanonicalManualField(Audit, Prefix, TEXT("ResolvedCarrierSnapshot"));
		InitializeCanonicalSkillValidation(
			Value.SkillRuleSetValidationResult,
			Prefix + TEXT(".SkillRuleSetValidationResult"), Seed + 6, Audit);
		MarkCanonicalManualField(Audit, Prefix, TEXT("SkillRuleSetValidationResult"));
		Value.ValidatedActionPoint = 4 + Seed;
		MarkCanonicalManualField(Audit, Prefix, TEXT("ValidatedActionPoint"));
		Value.ErrorMessage = FString::Printf(TEXT("Canonical global %d"), Seed);
		MarkCanonicalManualField(Audit, Prefix, TEXT("ErrorMessage"));
	}

	void RegisterCanonicalBuilderSchemas(FCanonicalBuilderAudit& Audit)
	{
		RegisterCanonicalReflectedSchema(
			FMatchPlayCurrentAttackMarkerSelectionWriterResult::StaticStruct(),
			TEXT("Canonical.SubmitMarkerGraph"), Audit);
		RegisterCanonicalReflectedSchema(
			FMatchPlayResolveNoLegalMarkerResult::StaticStruct(),
			TEXT("Canonical.ResolveMarkerGraph"), Audit);
		RegisterCanonicalReflectedSchema(
			FMatchPlayMarkerDeclineResult::StaticStruct(),
			TEXT("Canonical.DeclineMarkerGraph"), Audit);
		RegisterCanonicalReflectedSchema(
			FMatchPlayCurrentAttackSkillSelectionWriterResult::StaticStruct(),
			TEXT("Canonical.SubmitSkillGraph"), Audit);
		RegisterCanonicalReflectedSchema(
			FMatchPlayResolveNoLegalSkillResult::StaticStruct(),
			TEXT("Canonical.ResolveSkillGraph"), Audit);
		RegisterCanonicalReflectedSchema(
			FMatchPlaySkillDeclineResult::StaticStruct(),
			TEXT("Canonical.DeclineSkillGraph"), Audit);
		RegisterCanonicalReflectedSchema(
			FMatchPlayCurrentAttackMarkerSelectionCandidateAvailability::StaticStruct(),
			TEXT("Canonical.MarkerCandidate"), Audit);
		ExpectCanonicalCardSnapshotQuery(
			Audit,
			TEXT("Canonical.MarkerCandidate.LegalityResult.MarkerSnapshotQueryResult"));
		RegisterCanonicalReflectedSchema(
			FMatchPlayCurrentAttackSkillSelectionCandidateAvailability::StaticStruct(),
			TEXT("Canonical.SkillCandidate"), Audit);
		ExpectCanonicalSkillGlobal(
			Audit,
			TEXT("Canonical.SkillCandidate.LegalityResult.GlobalContextResult"));
		ExpectCanonicalCardSnapshotQuery(
			Audit,
			TEXT("Canonical.SkillCandidate.LegalityResult.CarrierSnapshotQueryResult"));
		ExpectCanonicalSkillQuery(
			Audit,
			TEXT("Canonical.SkillCandidate.LegalityResult.SkillRuleQueryResult"));
		ExpectCanonicalSkillRule(
			Audit,
			TEXT("Canonical.SkillCandidate.LegalityResult.ResolvedSkillRule"));
		ExpectCanonicalCardSnapshotQuery(
			Audit, TEXT("Canonical.CompletionSnapshot"));
	}

	void InitializeCanonicalMarkerCandidate(
		FMatchPlayCurrentAttackMarkerSelectionCandidateAvailability& Value,
		const int32 Seed,
		FCanonicalBuilderAudit& Audit)
	{
		InitializeCanonicalReflectedStruct(
			FMatchPlayCurrentAttackMarkerSelectionCandidateAvailability::StaticStruct(),
			&Value, TEXT("Canonical.MarkerCandidate"), Seed, Audit);
		InitializeCanonicalCardSnapshotQuery(
			Value.LegalityResult.MarkerSnapshotQueryResult,
			TEXT("Canonical.MarkerCandidate.LegalityResult.MarkerSnapshotQueryResult"),
			Seed + 1,
			Audit);
	}

	void InitializeCanonicalSkillCandidate(
		FMatchPlayCurrentAttackSkillSelectionCandidateAvailability& Value,
		const int32 Seed,
		FCanonicalBuilderAudit& Audit)
	{
		InitializeCanonicalReflectedStruct(
			FMatchPlayCurrentAttackSkillSelectionCandidateAvailability::StaticStruct(),
			&Value, TEXT("Canonical.SkillCandidate"), Seed, Audit);
		InitializeCanonicalSkillGlobal(
			Value.LegalityResult.GlobalContextResult,
			TEXT("Canonical.SkillCandidate.LegalityResult.GlobalContextResult"),
			Seed + 1,
			Audit);
		InitializeCanonicalCardSnapshotQuery(
			Value.LegalityResult.CarrierSnapshotQueryResult,
			TEXT("Canonical.SkillCandidate.LegalityResult.CarrierSnapshotQueryResult"),
			Seed + 2,
			Audit);
		InitializeCanonicalSkillQuery(
			Value.LegalityResult.SkillRuleQueryResult,
			TEXT("Canonical.SkillCandidate.LegalityResult.SkillRuleQueryResult"),
			Seed + 3,
			Audit);
		InitializeCanonicalSkillRule(
			Value.LegalityResult.ResolvedSkillRule,
			TEXT("Canonical.SkillCandidate.LegalityResult.ResolvedSkillRule"),
			Seed + 4,
			Audit);
	}

	void InitializeCanonicalCompletionSnapshot(
		FMatchPlayCardSnapshotAuthorityQueryResult& Value,
		const int32 Seed,
		FCanonicalBuilderAudit& Audit)
	{
		InitializeCanonicalCardSnapshotQuery(
			Value, TEXT("Canonical.CompletionSnapshot"), Seed, Audit);
	}

	void InitializeReachableMatchStates(
		UStruct* Struct,
		void* Container,
		const int32 Seed,
		FCanonicalBuilderAudit& Audit)
	{
		int32 Offset = 0;
		for (TFieldIterator<FProperty> It(Struct); It; ++It, ++Offset)
		{
			FProperty* Property = *It;
			void* Value = Property->ContainerPtrToValuePtr<void>(Container);
			if (FStructProperty* StructProperty = CastField<FStructProperty>(Property))
			{
				if (StructProperty->Struct == FMatchState::StaticStruct())
				{
					InitializeCanonicalReflectedStruct(
						FMatchState::StaticStruct(),
						Value,
						TEXT("Canonical.ReachableFMatchState"),
						Seed * 31 + Offset + 1,
						Audit);
				}
				else
				{
					InitializeReachableMatchStates(
						StructProperty->Struct,
						Value,
						Seed * 31 + Offset + 1,
						Audit);
				}
			}
			else if (FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Property))
			{
				if (FStructProperty* InnerStruct =
					CastField<FStructProperty>(ArrayProperty->Inner))
				{
					FScriptArrayHelper Array(ArrayProperty, Value);
					for (int32 Index = 0; Index < Array.Num(); ++Index)
					{
						InitializeReachableMatchStates(
							InnerStruct->Struct,
							Array.GetRawPtr(Index),
							Seed * 31 + Offset + Index + 1,
							Audit);
					}
				}
			}
		}
	}

	template <typename TReflected>
	void InitializeReachableMatchStatesIn(
		TReflected& Value,
		const int32 Seed,
		FCanonicalBuilderAudit& Audit)
	{
		InitializeReachableMatchStates(
			TReflected::StaticStruct(), &Value, Seed, Audit);
	}

	void ExpectPathFields(
		FMutationCoverageAudit& Audit,
		const FString& Prefix,
		std::initializer_list<const TCHAR*> Fields)
	{
		for (const TCHAR* Field : Fields)
		{
			Audit.Expect(Prefix + TEXT(".") + Field);
		}
	}

	void RegisterExpectedPlayerCardValidationDeepPaths(
		FMutationCoverageAudit& Audit,
		const FString& Prefix)
	{
		ExpectPathFields(Audit, Prefix, {
			TEXT("bSuccess"), TEXT("bIsValid"), TEXT("ErrorCode"),
			TEXT("ErrorMessage"), TEXT("InvalidCardId"),
			TEXT("DuplicateCardIds.Num"),
			TEXT("DuplicateCardIds.EmptyElement[0]")});
	}

	void RegisterExpectedSkillValidationDeepPaths(
		FMutationCoverageAudit& Audit,
		const FString& Prefix)
	{
		ExpectPathFields(Audit, Prefix, {
			TEXT("bSuccess"), TEXT("bIsValid"), TEXT("ErrorCode"),
			TEXT("ErrorMessage"), TEXT("InvalidSkillId"), TEXT("InvalidField")});
	}

	void RegisterExpectedSkillRuleDeepPaths(
		FMutationCoverageAudit& Audit,
		const FString& Prefix)
	{
		ExpectPathFields(Audit, Prefix, {
			TEXT("SkillId"), TEXT("SkillType"),
			TEXT("MinTriggerActionPoint"), TEXT("MaxTriggerActionPoint")});
	}

	void RegisterExpectedPlayerCardQueryDeepPaths(
		FMutationCoverageAudit& Audit,
		const FString& Prefix,
		FPlayerCardRuleSnapshotQueryResult& Value)
	{
		ExpectPathFields(Audit, Prefix, {
			TEXT("bSuccess"), TEXT("bFound"), TEXT("ErrorCode"),
			TEXT("ErrorMessage"), TEXT("CardId")});
		RegisterExpectedReflectedPaths(
			FPlayerCardRuleSnapshot::StaticStruct(),
			&Value.Snapshot,
			Prefix + TEXT(".Snapshot"), Audit);
		RegisterExpectedPlayerCardValidationDeepPaths(
			Audit, Prefix + TEXT(".ValidationResult"));
	}

	void RegisterExpectedCardSnapshotQueryDeepPaths(
		FMutationCoverageAudit& Audit,
		const FString& Prefix,
		FMatchPlayCardSnapshotAuthorityQueryResult& Value)
	{
		ExpectPathFields(Audit, Prefix, {
			TEXT("bSuccess"), TEXT("ErrorCode"), TEXT("PlayerSide"),
			TEXT("CardId"), TEXT("ErrorMessage")});
		RegisterExpectedReflectedPaths(
			FPlayerCardRuleSnapshot::StaticStruct(),
			&Value.Snapshot,
			Prefix + TEXT(".Snapshot"), Audit);
		RegisterExpectedPlayerCardQueryDeepPaths(
			Audit,
			Prefix + TEXT(".UnderlyingQueryResult"),
			Value.UnderlyingQueryResult);
	}

	void RegisterExpectedSkillRuleQueryDeepPaths(
		FMutationCoverageAudit& Audit,
		const FString& Prefix)
	{
		ExpectPathFields(Audit, Prefix, {
			TEXT("bSuccess"), TEXT("bFound"), TEXT("ErrorCode"),
			TEXT("ErrorMessage"), TEXT("InvalidSkillId"), TEXT("InvalidField")});
		RegisterExpectedSkillRuleDeepPaths(Audit, Prefix + TEXT(".Snapshot"));
		RegisterExpectedSkillValidationDeepPaths(
			Audit, Prefix + TEXT(".ValidationResult"));
	}

	void RegisterExpectedSkillGlobalDeepPaths(
		FMutationCoverageAudit& Audit,
		const FString& Prefix,
		FMatchPlayCurrentAttackSkillSelectionGlobalContextResult& Value)
	{
		ExpectPathFields(Audit, Prefix, {
			TEXT("bSuccess"), TEXT("RequestedAttackSequence"),
			TEXT("RequestingSide"), TEXT("ErrorCode"),
			TEXT("AuthoritativeAttackSequence"), TEXT("CurrentAttackingPlayer"),
			TEXT("CurrentDefendingPlayer"), TEXT("FrozenCarrierCardId"),
			TEXT("FrozenMarkerCardId"),
			TEXT("MatchingFrozenCarrierPlacementCount"),
			TEXT("MatchingFrozenMarkerPlacementCount"),
			TEXT("ValidatedActionPoint"), TEXT("ErrorMessage")});
		RegisterExpectedReflectedPaths(
			FMatchPlayCurrentAttackSelectionStateValidationResult::StaticStruct(),
			&Value.SelectionStateValidationResult,
			Prefix + TEXT(".SelectionStateValidationResult"), Audit);
		RegisterExpectedReflectedPaths(
			FMatchPlayDeploymentPlacement::StaticStruct(),
			&Value.FrozenCarrierPlacement,
			Prefix + TEXT(".FrozenCarrierPlacement"), Audit);
		RegisterExpectedReflectedPaths(
			FMatchPlayDeploymentPlacement::StaticStruct(),
			&Value.FrozenMarkerPlacement,
			Prefix + TEXT(".FrozenMarkerPlacement"), Audit);
		RegisterExpectedCardSnapshotQueryDeepPaths(
			Audit,
			Prefix + TEXT(".CarrierSnapshotQueryResult"),
			Value.CarrierSnapshotQueryResult);
		RegisterExpectedReflectedPaths(
			FPlayerCardRuleSnapshot::StaticStruct(),
			&Value.ResolvedCarrierSnapshot,
			Prefix + TEXT(".ResolvedCarrierSnapshot"), Audit);
		RegisterExpectedSkillValidationDeepPaths(
			Audit, Prefix + TEXT(".SkillRuleSetValidationResult"));
	}

	void RegisterExpectedContainerBehaviorPaths(
		FMutationCoverageAudit& Audit,
		const FString& Prefix)
	{
		ExpectPathFields(Audit, Prefix, {
			TEXT("Empty.Length"), TEXT("Singleton.Removal"),
			TEXT("Three.Removal"), TEXT("Three.Addition"),
			TEXT("Three.Order")});
	}

	void RegisterExpectedMarkerCandidateContainerPaths(
		FMutationCoverageAudit& Audit,
		FCanonicalBuilderAudit& CanonicalAudit,
		const FString& Prefix,
		const int32 Seed)
	{
		RegisterExpectedContainerBehaviorPaths(Audit, Prefix);
		for (const int32 Index : {0, 1, 2, 3})
		{
			const bool bEmpty = Index == 3;
			const int32 LogicalIndex = bEmpty ? 0 : Index;
			const FString ElementPrefix = bEmpty
				? Prefix + TEXT(".EmptyElement[0]")
				: FString::Printf(TEXT("%s.Three[%d]"), *Prefix, LogicalIndex);
			FMatchPlayCurrentAttackMarkerSelectionCandidateAvailability Candidate;
			InitializeCanonicalMarkerCandidate(
				Candidate, Seed + LogicalIndex, CanonicalAudit);
			RegisterExpectedReflectedPaths(
				FMatchPlayCurrentAttackMarkerSelectionCandidateAvailability::StaticStruct(),
				&Candidate, ElementPrefix, Audit, bEmpty);
			RegisterExpectedCardSnapshotQueryDeepPaths(
				Audit,
				ElementPrefix + TEXT(".LegalityResult.MarkerSnapshotQueryResult"),
				Candidate.LegalityResult.MarkerSnapshotQueryResult);
		}
	}

	void RegisterExpectedSkillCandidateContainerPaths(
		FMutationCoverageAudit& Audit,
		FCanonicalBuilderAudit& CanonicalAudit,
		const FString& Prefix,
		const int32 Seed)
	{
		RegisterExpectedContainerBehaviorPaths(Audit, Prefix);
		for (const int32 Index : {0, 1, 2, 3})
		{
			const bool bEmpty = Index == 3;
			const int32 LogicalIndex = bEmpty ? 0 : Index;
			const FString ElementPrefix = bEmpty
				? Prefix + TEXT(".EmptyElement[0]")
				: FString::Printf(TEXT("%s.Three[%d]"), *Prefix, LogicalIndex);
			FMatchPlayCurrentAttackSkillSelectionCandidateAvailability Candidate;
			InitializeCanonicalSkillCandidate(
				Candidate, Seed + LogicalIndex, CanonicalAudit);
			RegisterExpectedReflectedPaths(
				FMatchPlayCurrentAttackSkillSelectionCandidateAvailability::StaticStruct(),
				&Candidate, ElementPrefix, Audit, bEmpty);
			RegisterExpectedSkillGlobalDeepPaths(
				Audit,
				ElementPrefix + TEXT(".LegalityResult.GlobalContextResult"),
				Candidate.LegalityResult.GlobalContextResult);
			RegisterExpectedCardSnapshotQueryDeepPaths(
				Audit,
				ElementPrefix + TEXT(".LegalityResult.CarrierSnapshotQueryResult"),
				Candidate.LegalityResult.CarrierSnapshotQueryResult);
			RegisterExpectedSkillRuleQueryDeepPaths(
				Audit,
				ElementPrefix + TEXT(".LegalityResult.SkillRuleQueryResult"));
			RegisterExpectedSkillRuleDeepPaths(
				Audit,
				ElementPrefix + TEXT(".LegalityResult.ResolvedSkillRule"));
		}
	}

	void RegisterExpectedCompletionSnapshotContainerPaths(
		FMutationCoverageAudit& Audit,
		FCanonicalBuilderAudit& CanonicalAudit,
		const FString& Prefix,
		const int32 Seed)
	{
		RegisterExpectedContainerBehaviorPaths(Audit, Prefix);
		for (const int32 Index : {0, 1, 2, 3})
		{
			const bool bEmpty = Index == 3;
			const int32 LogicalIndex = bEmpty ? 0 : Index;
			const FString ElementPrefix = bEmpty
				? Prefix + TEXT(".EmptyElement[0]")
				: FString::Printf(TEXT("%s.Three[%d]"), *Prefix, LogicalIndex);
			FMatchPlayCardSnapshotAuthorityQueryResult Snapshot;
			InitializeCanonicalCompletionSnapshot(
				Snapshot, Seed + LogicalIndex, CanonicalAudit);
			RegisterExpectedCardSnapshotQueryDeepPaths(
				Audit, ElementPrefix, Snapshot);
		}
	}

	void RegisterExpectedStandaloneNonReflectedPaths(
		FMutationCoverageAudit& Audit,
		FCanonicalBuilderAudit& CanonicalAudit)
	{
		for (const TCHAR* Field : {
			TEXT("PlayerCardValidation.bSuccess"),
			TEXT("PlayerCardValidation.bIsValid"),
			TEXT("PlayerCardValidation.ErrorCode"),
			TEXT("PlayerCardValidation.ErrorMessage"),
			TEXT("PlayerCardValidation.InvalidCardId"),
			TEXT("PlayerCardValidation.DuplicateCardIds"),
			TEXT("PlayerCardQuery.bSuccess"),
			TEXT("PlayerCardQuery.bFound"),
			TEXT("PlayerCardQuery.ErrorCode"),
			TEXT("PlayerCardQuery.ErrorMessage"),
			TEXT("PlayerCardQuery.CardId"),
			TEXT("PlayerCardQuery.Snapshot"),
			TEXT("PlayerCardQuery.ValidationResult"),
			TEXT("CardSnapshotQuery.bSuccess"),
			TEXT("CardSnapshotQuery.ErrorCode"),
			TEXT("CardSnapshotQuery.PlayerSide"),
			TEXT("CardSnapshotQuery.CardId"),
			TEXT("CardSnapshotQuery.Snapshot"),
			TEXT("CardSnapshotQuery.UnderlyingQueryResult"),
			TEXT("CardSnapshotQuery.ErrorMessage"),
			TEXT("SkillRuleValidation.bSuccess"),
			TEXT("SkillRuleValidation.bIsValid"),
			TEXT("SkillRuleValidation.ErrorCode"),
			TEXT("SkillRuleValidation.ErrorMessage"),
			TEXT("SkillRuleValidation.InvalidSkillId"),
			TEXT("SkillRuleValidation.InvalidField"),
			TEXT("SkillRule.SkillId"),
			TEXT("SkillRule.SkillType"),
			TEXT("SkillRule.MinTriggerActionPoint"),
			TEXT("SkillRule.MaxTriggerActionPoint"),
			TEXT("SkillRuleQuery.bSuccess"),
			TEXT("SkillRuleQuery.bFound"),
			TEXT("SkillRuleQuery.ErrorCode"),
			TEXT("SkillRuleQuery.ErrorMessage"),
			TEXT("SkillRuleQuery.InvalidSkillId"),
			TEXT("SkillRuleQuery.InvalidField"),
			TEXT("SkillRuleQuery.Snapshot"),
			TEXT("SkillRuleQuery.ValidationResult"),
			TEXT("SkillGlobal.bSuccess"),
			TEXT("SkillGlobal.RequestedAttackSequence"),
			TEXT("SkillGlobal.RequestingSide"),
			TEXT("SkillGlobal.ErrorCode"),
			TEXT("SkillGlobal.AuthoritativeAttackSequence"),
			TEXT("SkillGlobal.CurrentAttackingPlayer"),
			TEXT("SkillGlobal.CurrentDefendingPlayer"),
			TEXT("SkillGlobal.SelectionValidation"),
			TEXT("SkillGlobal.FrozenCarrierCardId"),
			TEXT("SkillGlobal.FrozenMarkerCardId"),
			TEXT("SkillGlobal.MatchingFrozenCarrierPlacementCount"),
			TEXT("SkillGlobal.MatchingFrozenMarkerPlacementCount"),
			TEXT("SkillGlobal.FrozenCarrierPlacement"),
			TEXT("SkillGlobal.FrozenMarkerPlacement"),
			TEXT("SkillGlobal.CarrierSnapshotQueryResult"),
			TEXT("SkillGlobal.ResolvedCarrierSnapshot"),
			TEXT("SkillGlobal.SkillRuleSetValidationResult"),
			TEXT("SkillGlobal.ValidatedActionPoint"),
			TEXT("SkillGlobal.ErrorMessage")})
		{
			Audit.ExpectedNonReflectedSchemaFields.Add(Field);
		}
		ExpectPathFields(Audit, TEXT("NonReflected.PlayerCardValidation"), {
			TEXT("bSuccess"), TEXT("bIsValid"), TEXT("ErrorCode"),
			TEXT("ErrorMessage"), TEXT("InvalidCardId"), TEXT("DuplicateCardIds")});
		ExpectPathFields(Audit, TEXT("Manual.PlayerCardValidation.DuplicateCardIds"), {
			TEXT("Empty.Length"), TEXT("EmptyElement[0].FName"),
			TEXT("Singleton.Removal"), TEXT("Three.Removal"),
			TEXT("Three.Addition"), TEXT("Three[0].FName"),
			TEXT("Three[1].FName"), TEXT("Three[2].FName"),
			TEXT("Three.Order")});

		FPlayerCardRuleSnapshotQueryResult CardQuery;
		InitializeCanonicalPlayerCardQuery(
			CardQuery, TEXT("Canonical.Standalone.PlayerCardQuery"), 900,
			CanonicalAudit);
		ExpectPathFields(Audit, TEXT("NonReflected.PlayerCardQuery"), {
			TEXT("bSuccess"), TEXT("bFound"), TEXT("ErrorCode"),
			TEXT("ErrorMessage"), TEXT("CardId"), TEXT("ValidationResult")});
		RegisterExpectedReflectedPaths(
			FPlayerCardRuleSnapshot::StaticStruct(), &CardQuery.Snapshot,
			TEXT("PlayerCardQuery.Snapshot"), Audit);

		FMatchPlayCardSnapshotAuthorityQueryResult CardSnapshot;
		InitializeCanonicalCardSnapshotQuery(
			CardSnapshot, TEXT("Canonical.Standalone.CardSnapshotQuery"), 910,
			CanonicalAudit);
		ExpectPathFields(Audit, TEXT("NonReflected.CardSnapshotQuery"), {
			TEXT("bSuccess"), TEXT("ErrorCode"), TEXT("PlayerSide"),
			TEXT("CardId"), TEXT("UnderlyingQueryResult"), TEXT("ErrorMessage")});
		RegisterExpectedReflectedPaths(
			FPlayerCardRuleSnapshot::StaticStruct(), &CardSnapshot.Snapshot,
			TEXT("CardSnapshotQuery.Snapshot"), Audit);

		ExpectPathFields(Audit, TEXT("NonReflected.SkillRuleValidation"), {
			TEXT("bSuccess"), TEXT("bIsValid"), TEXT("ErrorCode"),
			TEXT("ErrorMessage"), TEXT("InvalidSkillId"), TEXT("InvalidField")});
		ExpectPathFields(Audit, TEXT("NonReflected.SkillRule"), {
			TEXT("SkillId"), TEXT("SkillType"),
			TEXT("MinTriggerActionPoint"), TEXT("MaxTriggerActionPoint")});
		ExpectPathFields(Audit, TEXT("NonReflected.SkillRuleQuery"), {
			TEXT("bSuccess"), TEXT("bFound"), TEXT("ErrorCode"),
			TEXT("ErrorMessage"), TEXT("InvalidSkillId"), TEXT("InvalidField"),
			TEXT("Snapshot"), TEXT("ValidationResult")});

		FMatchPlayCurrentAttackSkillSelectionGlobalContextResult Global;
		InitializeCanonicalSkillGlobal(
			Global, TEXT("Canonical.Standalone.SkillGlobal"), 920,
			CanonicalAudit);
		ExpectPathFields(Audit, TEXT("NonReflected.SkillGlobal"), {
			TEXT("bSuccess"), TEXT("RequestedAttackSequence"),
			TEXT("RequestingSide"), TEXT("ErrorCode"),
			TEXT("AuthoritativeAttackSequence"), TEXT("CurrentAttackingPlayer"),
			TEXT("CurrentDefendingPlayer"), TEXT("FrozenCarrierCardId"),
			TEXT("FrozenMarkerCardId"),
			TEXT("MatchingFrozenCarrierPlacementCount"),
			TEXT("MatchingFrozenMarkerPlacementCount"),
			TEXT("CarrierSnapshotQueryResult"),
			TEXT("SkillRuleSetValidationResult"),
			TEXT("ValidatedActionPoint"), TEXT("ErrorMessage")});
		RegisterExpectedReflectedPaths(
			FMatchPlayCurrentAttackSelectionStateValidationResult::StaticStruct(),
			&Global.SelectionStateValidationResult,
			TEXT("SkillGlobal.SelectionValidation"), Audit);
		RegisterExpectedReflectedPaths(
			FMatchPlayDeploymentPlacement::StaticStruct(),
			&Global.FrozenCarrierPlacement,
			TEXT("SkillGlobal.FrozenCarrierPlacement"), Audit);
		RegisterExpectedReflectedPaths(
			FMatchPlayDeploymentPlacement::StaticStruct(),
			&Global.FrozenMarkerPlacement,
			TEXT("SkillGlobal.FrozenMarkerPlacement"), Audit);
		RegisterExpectedReflectedPaths(
			FPlayerCardRuleSnapshot::StaticStruct(),
			&Global.ResolvedCarrierSnapshot,
			TEXT("SkillGlobal.ResolvedCarrierSnapshot"), Audit);
	}

	void RegisterExpectedFoundationBMutationSchema(
		FMutationCoverageAudit& Audit,
		FCanonicalBuilderAudit& CanonicalAudit)
	{
		FMatchPlayCurrentAttackMarkerSelectionWriterResult SubmitMarker;
		InitializeCanonicalReflectedStruct(
			FMatchPlayCurrentAttackMarkerSelectionWriterResult::StaticStruct(),
			&SubmitMarker, TEXT("Canonical.SubmitMarkerGraph"), 10,
			CanonicalAudit);
		RegisterExpectedEnvelopePaths(Audit, TEXT("SubmitMarker.Envelope"));
		RegisterExpectedReflectedPaths(
			FMatchPlayCurrentAttackMarkerSelectionWriterResult::StaticStruct(),
			&SubmitMarker, TEXT("SubmitMarker"), Audit);

		FMatchPlayResolveNoLegalMarkerResult ResolveMarker;
		InitializeCanonicalReflectedStruct(
			FMatchPlayResolveNoLegalMarkerResult::StaticStruct(),
			&ResolveMarker, TEXT("Canonical.ResolveMarkerGraph"), 20,
			CanonicalAudit);
		RegisterExpectedEnvelopePaths(Audit, TEXT("ResolveNoLegalMarker.Envelope"));
		RegisterExpectedReflectedPaths(
			FMatchPlayResolveNoLegalMarkerResult::StaticStruct(),
			&ResolveMarker, TEXT("ResolveNoLegalMarker"), Audit);

		FMatchPlayMarkerDeclineResult DeclineMarker;
		InitializeCanonicalReflectedStruct(
			FMatchPlayMarkerDeclineResult::StaticStruct(),
			&DeclineMarker, TEXT("Canonical.DeclineMarkerGraph"), 30,
			CanonicalAudit);
		RegisterExpectedEnvelopePaths(Audit, TEXT("DeclineMarker.Envelope"));
		RegisterExpectedReflectedPaths(
			FMatchPlayMarkerDeclineResult::StaticStruct(),
			&DeclineMarker, TEXT("DeclineMarker"), Audit);

		FMatchPlayCurrentAttackSkillSelectionWriterResult SubmitSkill;
		InitializeCanonicalReflectedStruct(
			FMatchPlayCurrentAttackSkillSelectionWriterResult::StaticStruct(),
			&SubmitSkill, TEXT("Canonical.SubmitSkillGraph"), 40,
			CanonicalAudit);
		RegisterExpectedEnvelopePaths(Audit, TEXT("SubmitSkill.Envelope"));
		RegisterExpectedReflectedPaths(
			FMatchPlayCurrentAttackSkillSelectionWriterResult::StaticStruct(),
			&SubmitSkill, TEXT("SubmitSkill"), Audit);

		FMatchPlayResolveNoLegalSkillResult ResolveSkill;
		InitializeCanonicalReflectedStruct(
			FMatchPlayResolveNoLegalSkillResult::StaticStruct(),
			&ResolveSkill, TEXT("Canonical.ResolveSkillGraph"), 50,
			CanonicalAudit);
		RegisterExpectedEnvelopePaths(Audit, TEXT("ResolveNoLegalSkill.Envelope"));
		RegisterExpectedReflectedPaths(
			FMatchPlayResolveNoLegalSkillResult::StaticStruct(),
			&ResolveSkill, TEXT("ResolveNoLegalSkill"), Audit);

		FMatchPlaySkillDeclineResult DeclineSkill;
		InitializeCanonicalReflectedStruct(
			FMatchPlaySkillDeclineResult::StaticStruct(),
			&DeclineSkill, TEXT("Canonical.DeclineSkillGraph"), 60,
			CanonicalAudit);
		RegisterExpectedEnvelopePaths(Audit, TEXT("DeclineSkill.Envelope"));
		RegisterExpectedReflectedPaths(
			FMatchPlaySkillDeclineResult::StaticStruct(),
			&DeclineSkill, TEXT("DeclineSkill"), Audit);

		RegisterExpectedMarkerCandidateContainerPaths(
			Audit, CanonicalAudit,
			TEXT("ResolveNoLegalMarker.ResolutionResult.MarkerAvailabilityResult.Candidates"),
			100);
		RegisterExpectedMarkerCandidateContainerPaths(
			Audit, CanonicalAudit,
			TEXT("DeclineMarker.DeclineResult.MarkerAvailabilityResult.Candidates"),
			200);
		RegisterExpectedSkillCandidateContainerPaths(
			Audit, CanonicalAudit,
			TEXT("ResolveNoLegalSkill.ResolutionResult.SkillAvailabilityResult.Candidates"),
			300);
		RegisterExpectedSkillCandidateContainerPaths(
			Audit, CanonicalAudit,
			TEXT("DeclineSkill.DeclineResult.SkillAvailabilityResult.Candidates"),
			400);
		RegisterExpectedCompletionSnapshotContainerPaths(
			Audit, CanonicalAudit,
			TEXT("ResolveNoLegalMarker.ResolutionResult.CompletionResult.DeploymentSnapshotQueryResults"),
			500);
		RegisterExpectedCompletionSnapshotContainerPaths(
			Audit, CanonicalAudit,
			TEXT("DeclineMarker.DeclineResult.CompletionResult.DeploymentSnapshotQueryResults"),
			600);
		RegisterExpectedCompletionSnapshotContainerPaths(
			Audit, CanonicalAudit,
			TEXT("ResolveNoLegalSkill.ResolutionResult.CompletionResult.DeploymentSnapshotQueryResults"),
			700);
		RegisterExpectedCompletionSnapshotContainerPaths(
			Audit, CanonicalAudit,
			TEXT("DeclineSkill.DeclineResult.CompletionResult.DeploymentSnapshotQueryResults"),
			800);
		RegisterExpectedStandaloneNonReflectedPaths(Audit, CanonicalAudit);
	}

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
			const bool bRejected = CoverageAudit != nullptr
				? TestIndependentMutationGate(
					Test,
					*CoverageAudit,
					Path,
					Baseline,
					Mutated,
					[](const auto& Left, const auto& Right)
					{
						return AreEnvelopesEqual(Left, Right);
					})
				: !AreEnvelopesEqual(Baseline, Mutated);
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
				CoverageAudit->CoveredReachableMapPaths.Add(Path);
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
				CoverageAudit->Skip(Path + TEXT(".MutationUnchanged"));
			}
			return;
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
				return CoverageAudit != nullptr
					? TestIndependentMutationGate(
						Test,
						*CoverageAudit,
						Path,
						ComparatorBaseline,
						Mutated,
						Comparator)
					: !Comparator(ComparatorBaseline, Mutated);
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
		const FString Path = TEXT("NonReflected.") + Context;
		const bool bRejected = Audit.CoverageAudit != nullptr
			? TestIndependentMutationGate(
				Test,
				*Audit.CoverageAudit,
				Path,
				Baseline,
				Mutated,
				Comparator)
			: !Comparator(Baseline, Mutated);
		Test.TestTrue(
			*FString::Printf(TEXT("Non-reflected comparator covers %s"), *Context),
			bRejected);
		if (Audit.CoverageAudit != nullptr)
		{
			if (bRejected)
			{
				Audit.CoverageAudit->CoveredNonReflectedSchemaFields.Add(Context);
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
				return Audit.CoverageAudit != nullptr
					? TestIndependentMutationGate(
						Test,
						*Audit.CoverageAudit,
						Path,
						Baseline,
						Mutated,
						Comparator)
					: !Comparator(Baseline, Mutated);
			},
			Audit.CoverageAudit,
			false);
		Audit.ReflectedNestedLeaves += ReflectedAudit.LeafMutations;
		Audit.ContainerCases += ReflectedAudit.ContainerCases;
		Audit.EmptyContainerCases +=
			ReflectedAudit.EmptyContainerElementStructures;
		Audit.SkippedGroups += ReflectedAudit.SkippedProperties;
		if (Audit.CoverageAudit != nullptr
			&& ReflectedAudit.LeafMutations > 0
			&& ReflectedAudit.SkippedProperties == 0)
		{
			Audit.CoverageAudit->CoveredNonReflectedSchemaFields.Add(Context);
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
		TWrapper Mutated = Baseline;
		Mutator(Mutated);
		const bool bRejected = TestIndependentMutationGate(
			Test,
			Audit,
			Path,
			Baseline,
			Mutated,
			Comparator);
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
		TestIndependentCanonicalEquality(
			Test,
			Prefix + TEXT(" singleton duplicate baseline"),
			Singleton,
			Comparator,
			&Audit);
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
		FMutationCoverageAudit& CoverageAudit,
		FCanonicalBuilderAudit& CanonicalAudit)
	{
		FNonReflectedMutationAudit Audit;
		Audit.CoverageAudit = &CoverageAudit;

		FPlayerCardRuleSnapshotValidationResult CardValidation;
		InitializeCanonicalPlayerCardValidation(
			CardValidation,
			TEXT("Canonical.Standalone.PlayerCardValidation"),
			890,
			CanonicalAudit);
		auto CardValidationComparator = [](const auto& Left, const auto& Right)
		{
			return ArePlayerCardValidationResultsEqual(Left, Right);
		};
		TestIndependentCanonicalEquality(
			Test, TEXT("PlayerCardValidation equal baseline"),
			CardValidation, CardValidationComparator);
		++Audit.EqualBaselines;
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
		InitializeCanonicalPlayerCardValidation(
			EmptyDuplicates,
			TEXT("Canonical.Standalone.PlayerCardValidation"),
			890,
			CanonicalAudit);
		EmptyDuplicates.DuplicateCardIds.Empty();
		TestIndependentCanonicalEquality(
			Test, TEXT("DuplicateCardIds empty equals empty"),
			EmptyDuplicates, CardValidationComparator, &CoverageAudit);
		ExecuteAuditedMutation(Test, CoverageAudit,
			DuplicatePrefix + TEXT(".Empty.Length"), EmptyDuplicates,
			CardValidationComparator,
			[](auto& Value){ Value.DuplicateCardIds.Add(TEXT("Canonical.Duplicate")); },
			EMutationCoverageCategory::ContainerBehavior);
		++Audit.ContainerCases;
		FPlayerCardRuleSnapshotValidationResult SingletonDuplicates;
		InitializeCanonicalPlayerCardValidation(
			SingletonDuplicates,
			TEXT("Canonical.Standalone.PlayerCardValidation"),
			890,
			CanonicalAudit);
		SingletonDuplicates.DuplicateCardIds = {TEXT("Canonical.Duplicate")};
		TestIndependentCanonicalEquality(
			Test, TEXT("DuplicateCardIds singleton canonical equality"),
			SingletonDuplicates, CardValidationComparator, &CoverageAudit);
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
		TestIndependentCanonicalEquality(
			Test, TEXT("DuplicateCardIds three-element canonical equality"),
			CardValidation, CardValidationComparator, &CoverageAudit);
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
		InitializeCanonicalPlayerCardQuery(
			CardQuery, TEXT("Canonical.Standalone.PlayerCardQuery"), 900,
			CanonicalAudit);
		auto CardQueryComparator = [](const auto& Left, const auto& Right)
		{
			return ArePlayerCardQueryResultsEqual(Left, Right);
		};
		TestIndependentCanonicalEquality(
			Test, TEXT("PlayerCardQuery equal baseline"),
			CardQuery, CardQueryComparator);
		++Audit.EqualBaselines;
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
		InitializeCanonicalCardSnapshotQuery(
			AuthorityQuery, TEXT("Canonical.Standalone.CardSnapshotQuery"), 910,
			CanonicalAudit);
		auto AuthorityComparator = [](const auto& Left, const auto& Right)
		{
			return AreCardSnapshotQueryResultsEqual(Left, Right);
		};
		TestIndependentCanonicalEquality(
			Test, TEXT("CardSnapshotQuery equal baseline"),
			AuthorityQuery, AuthorityComparator);
		++Audit.EqualBaselines;
		TestPlainMutation(Test, TEXT("CardSnapshotQuery.bSuccess"), AuthorityQuery,
			AuthorityComparator, [](auto& Value){ Value.bSuccess = !Value.bSuccess; }, Audit);
		TestPlainMutation(Test, TEXT("CardSnapshotQuery.ErrorCode"), AuthorityQuery,
			AuthorityComparator, [](auto& Value){ Value.ErrorCode = OtherEnumValue(Value.ErrorCode); }, Audit);
		TestPlainMutation(Test, TEXT("CardSnapshotQuery.PlayerSide"), AuthorityQuery,
			AuthorityComparator, [](auto& Value){ Value.PlayerSide = OtherEnumValue(Value.PlayerSide); }, Audit);
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
		InitializeCanonicalSkillValidation(
			SkillValidation, TEXT("Canonical.Standalone.SkillValidation"), 911,
			CanonicalAudit);
		auto SkillValidationComparator = [](const auto& Left, const auto& Right)
		{
			return AreSkillRuleValidationResultsEqual(Left, Right);
		};
		TestIndependentCanonicalEquality(
			Test, TEXT("SkillRuleValidation equal baseline"),
			SkillValidation, SkillValidationComparator);
		++Audit.EqualBaselines;
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
		InitializeCanonicalSkillRule(
			SkillRule, TEXT("Canonical.Standalone.SkillRule"), 912,
			CanonicalAudit);
		auto SkillRuleComparator = [](const auto& Left, const auto& Right)
		{
			return AreSkillRulesEqual(Left, Right);
		};
		TestIndependentCanonicalEquality(
			Test, TEXT("SkillRule equal baseline"),
			SkillRule, SkillRuleComparator);
		++Audit.EqualBaselines;
		TestPlainMutation(Test, TEXT("SkillRule.SkillId"), SkillRule,
			SkillRuleComparator, [](auto& Value){ Value.SkillId = TEXT("Mutated.Skill"); }, Audit);
		TestPlainMutation(Test, TEXT("SkillRule.SkillType"), SkillRule,
			SkillRuleComparator, [](auto& Value){ Value.SkillType = OtherEnumValue(Value.SkillType); }, Audit);
		TestPlainMutation(Test, TEXT("SkillRule.MinTriggerActionPoint"), SkillRule,
			SkillRuleComparator, [](auto& Value){ ++Value.MinTriggerActionPoint; }, Audit);
		TestPlainMutation(Test, TEXT("SkillRule.MaxTriggerActionPoint"), SkillRule,
			SkillRuleComparator, [](auto& Value){ ++Value.MaxTriggerActionPoint; }, Audit);

		FSkillRuleSnapshotQueryResult SkillQuery;
		InitializeCanonicalSkillQuery(
			SkillQuery, TEXT("Canonical.Standalone.SkillQuery"), 913,
			CanonicalAudit);
		auto SkillQueryComparator = [](const auto& Left, const auto& Right)
		{
			return AreSkillRuleQueryResultsEqual(Left, Right);
		};
		TestIndependentCanonicalEquality(
			Test, TEXT("SkillRuleQuery equal baseline"),
			SkillQuery, SkillQueryComparator);
		++Audit.EqualBaselines;
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
		InitializeCanonicalSkillGlobal(
			Global, TEXT("Canonical.Standalone.SkillGlobal"), 920,
			CanonicalAudit);
		auto GlobalComparator = [](const auto& Left, const auto& Right)
		{
			return AreSkillGlobalContextResultsEqual(Left, Right);
		};
		TestIndependentCanonicalEquality(
			Test, TEXT("SkillGlobalContext equal baseline"),
			Global, GlobalComparator);
		++Audit.EqualBaselines;
		TestPlainMutation(Test, TEXT("SkillGlobal.bSuccess"), Global,
			GlobalComparator, [](auto& Value){ Value.bSuccess = !Value.bSuccess; }, Audit);
		TestPlainMutation(Test, TEXT("SkillGlobal.RequestedAttackSequence"), Global,
			GlobalComparator, [](auto& Value){ ++Value.RequestedAttackSequence; }, Audit);
		TestPlainMutation(Test, TEXT("SkillGlobal.RequestingSide"), Global,
			GlobalComparator, [](auto& Value){ Value.RequestingSide = OtherEnumValue(Value.RequestingSide); }, Audit);
		TestPlainMutation(Test, TEXT("SkillGlobal.ErrorCode"), Global,
			GlobalComparator, [](auto& Value){ Value.ErrorCode = OtherEnumValue(Value.ErrorCode); }, Audit);
		TestPlainMutation(Test, TEXT("SkillGlobal.AuthoritativeAttackSequence"), Global,
			GlobalComparator, [](auto& Value){ ++Value.AuthoritativeAttackSequence; }, Audit);
		TestPlainMutation(Test, TEXT("SkillGlobal.CurrentAttackingPlayer"), Global,
			GlobalComparator, [](auto& Value){ Value.CurrentAttackingPlayer = OtherEnumValue(Value.CurrentAttackingPlayer); }, Audit);
		TestPlainMutation(Test, TEXT("SkillGlobal.CurrentDefendingPlayer"), Global,
			GlobalComparator, [](auto& Value){ Value.CurrentDefendingPlayer = OtherEnumValue(Value.CurrentDefendingPlayer); }, Audit);
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
		TestIndependentCanonicalEquality(
			Test, Context + TEXT(" empty baseline equality"),
			EmptyBaseline, Comparator, &Audit);
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
		TestIndependentCanonicalEquality(
			Test, Context + TEXT(" singleton canonical equality"),
			Singleton, Comparator, &Audit);
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
		TestIndependentCanonicalEquality(
			Test, Context + TEXT(" canonical equal baseline"),
			Baseline, Comparator, &Audit);
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
		TestIndependentCanonicalEquality(
			Test, Context + TEXT(" empty baseline equality"),
			EmptyBaseline, Comparator, &Audit);
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
		TestIndependentCanonicalEquality(
			Test, Context + TEXT(" singleton canonical equality"),
			Singleton, Comparator, &Audit);
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
		TestIndependentCanonicalEquality(
			Test, Context + TEXT(" canonical equal baseline"),
			Baseline, Comparator, &Audit);
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
	TestEqual(TEXT("All twenty-one mutations use the gate"),
		MatchPlayAuthoritativeSessionTests::CountOccurrences(
			Implementation,
			TEXT("ExecuteSerialized<")),
		21);
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
	TestFalse(TEXT("Direct random generation absent"),
		Production.Contains(TEXT("FMath::Rand")));
	TestFalse(TEXT("Session does not call RollD6 directly"),
		Production.Contains(TEXT("RollD6(")));
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

	MatchPlayAuthoritativeSessionTests::TestIndependentCanonicalEquality(
		*this, TEXT("Initialize comparator accepts equal value"), Initialize,
		[](const auto& Left, const auto& Right)
		{
			return MatchPlayAuthoritativeSessionTests::AreInitializeResultsEqual(
				Left, Right);
		});
	MatchPlayAuthoritativeSessionTests::TestIndependentCanonicalEquality(
		*this, TEXT("Begin comparator accepts equal value"), Begin,
		[](const auto& Left, const auto& Right)
		{
			return MatchPlayAuthoritativeSessionTests
				::AreAuthoritativeBeginResultsEqual(Left, Right);
		});
	MatchPlayAuthoritativeSessionTests::TestIndependentCanonicalEquality(
		*this, TEXT("Finish comparator accepts equal value"), Finish,
		[](const auto& Left, const auto& Right)
		{
			return MatchPlayAuthoritativeSessionTests
				::AreAuthoritativeFinishResultsEqual(Left, Right);
		});

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
	TestIndependentCanonicalEquality(
		*this, TEXT("Deploy comparator accepts equal values"), Deploy,
		[](const auto& Left, const auto& Right)
		{
			return AreAuthoritativeDeployOrdinaryResultsEqual(Left, Right);
		});
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
	TestIndependentCanonicalEquality(
		*this, TEXT("Carrier comparator accepts equal values"), Carrier,
		[](const auto& Left, const auto& Right)
		{
			return AreAuthoritativeSubmitCarrierResultsEqual(Left, Right);
		});
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
	FCanonicalBuilderAudit CanonicalAudit;
	RegisterCanonicalBuilderSchemas(CanonicalAudit);
	RegisterExpectedFoundationBMutationSchema(CoverageAudit, CanonicalAudit);

	FMatchPlayAuthoritativeSession MarkerSession;
	FReachabilityTrace MarkerTrace;
	BuildFoundationBToAwaitingMarker(
		MarkerSession, TEXT("ComparatorMarker"), true, {}, MarkerTrace);
	FName MarkerCardId;
	FindLegalMarker(
		MarkerSession.GetStateSnapshot(),
		MarkerTrace.DefendingSide,
		MarkerCardId);
	auto Marker = MarkerSession.SubmitMarker(
		MakeMarkerRequest(MarkerTrace, MarkerCardId));
	InitializeCanonicalReflectedStruct(
		FMatchPlayCurrentAttackMarkerSelectionWriterResult::StaticStruct(),
		&Marker.MarkerResult, TEXT("Canonical.SubmitMarkerGraph"), 10,
		CanonicalAudit);
	TestIndependentCanonicalEquality(
		*this, TEXT("Marker comparator accepts equal baseline"), Marker,
		[](const auto& Left, const auto& Right)
		{
			return AreAuthoritativeSubmitMarkerResultsEqual(Left, Right);
		}, &CoverageAudit);
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
	auto MarkerResolve =
		MarkerResolveSession.ResolveNoLegalMarker();
	InitializeCanonicalReflectedStruct(
		FMatchPlayResolveNoLegalMarkerResult::StaticStruct(),
		&MarkerResolve.ResolutionResult,
		TEXT("Canonical.ResolveMarkerGraph"), 20, CanonicalAudit);
	TestIndependentCanonicalEquality(
		*this, TEXT("Resolve marker comparator accepts equal baseline"),
		MarkerResolve,
		[](const auto& Left, const auto& Right)
		{
			return AreAuthoritativeResolveNoLegalMarkerResultsEqual(Left, Right);
		}, &CoverageAudit);
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
	auto MarkerDecline =
		MarkerDeclineSession.DeclineMarker(MarkerDeclineRequest);
	InitializeCanonicalReflectedStruct(
		FMatchPlayMarkerDeclineResult::StaticStruct(),
		&MarkerDecline.DeclineResult,
		TEXT("Canonical.DeclineMarkerGraph"), 30, CanonicalAudit);
	TestIndependentCanonicalEquality(
		*this, TEXT("Decline marker comparator accepts equal baseline"),
		MarkerDecline,
		[](const auto& Left, const auto& Right)
		{
			return AreAuthoritativeDeclineMarkerResultsEqual(Left, Right);
		}, &CoverageAudit);
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
	auto Skill = SkillSession.SubmitSkill(Rules, SkillRequest);
	InitializeCanonicalReflectedStruct(
		FMatchPlayCurrentAttackSkillSelectionWriterResult::StaticStruct(),
		&Skill.SkillResult, TEXT("Canonical.SubmitSkillGraph"), 40,
		CanonicalAudit);
	TestIndependentCanonicalEquality(
		*this, TEXT("Skill comparator accepts equal baseline"), Skill,
		[](const auto& Left, const auto& Right)
		{
			return AreAuthoritativeSubmitSkillResultsEqual(Left, Right);
		}, &CoverageAudit);
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
	auto SkillResolve =
		SkillResolveSession.ResolveNoLegalSkill(EmptyRules);
	InitializeCanonicalReflectedStruct(
		FMatchPlayResolveNoLegalSkillResult::StaticStruct(),
		&SkillResolve.ResolutionResult,
		TEXT("Canonical.ResolveSkillGraph"), 50, CanonicalAudit);
	TestIndependentCanonicalEquality(
		*this, TEXT("Resolve skill comparator accepts equal baseline"),
		SkillResolve,
		[](const auto& Left, const auto& Right)
		{
			return AreAuthoritativeResolveNoLegalSkillResultsEqual(Left, Right);
		}, &CoverageAudit);
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
	auto SkillDecline =
		SkillDeclineSession.DeclineSkill(Rules, SkillDeclineRequest);
	InitializeCanonicalReflectedStruct(
		FMatchPlaySkillDeclineResult::StaticStruct(),
		&SkillDecline.DeclineResult,
		TEXT("Canonical.DeclineSkillGraph"), 60, CanonicalAudit);
	TestIndependentCanonicalEquality(
		*this, TEXT("Decline skill comparator accepts equal baseline"),
		SkillDecline,
		[](const auto& Left, const auto& Right)
		{
			return AreAuthoritativeDeclineSkillResultsEqual(Left, Right);
		}, &CoverageAudit);
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
		[&CanonicalAudit](auto& Candidate, const int32 Index)
		{
			InitializeCanonicalMarkerCandidate(
				Candidate, 100 + Index, CanonicalAudit);
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
		[&CanonicalAudit](auto& Candidate, const int32 Index)
		{
			InitializeCanonicalMarkerCandidate(
				Candidate, 200 + Index, CanonicalAudit);
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
		[&CanonicalAudit](auto& Candidate, const int32 Index)
		{
			InitializeCanonicalSkillCandidate(
				Candidate, 300 + Index, CanonicalAudit);
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
		[&CanonicalAudit](auto& Candidate, const int32 Index)
		{
			InitializeCanonicalSkillCandidate(
				Candidate, 400 + Index, CanonicalAudit);
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
		[&CanonicalAudit](auto& Item, const int32 Index)
		{
			InitializeCanonicalCompletionSnapshot(
				Item, 500 + Index, CanonicalAudit);
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
		[&CanonicalAudit](auto& Item, const int32 Index)
		{
			InitializeCanonicalCompletionSnapshot(
				Item, 600 + Index, CanonicalAudit);
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
		[&CanonicalAudit](auto& Item, const int32 Index)
		{
			InitializeCanonicalCompletionSnapshot(
				Item, 700 + Index, CanonicalAudit);
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
		[&CanonicalAudit](auto& Item, const int32 Index)
		{
			InitializeCanonicalCompletionSnapshot(
				Item, 800 + Index, CanonicalAudit);
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
		TestNonReflectedInventoryMutations(
			*this, CoverageAudit, CanonicalAudit);
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
	int64 DuplicateExpected = 0;
	for (const TPair<FString, int32>& Entry :
		CoverageAudit.ExpectedRegistrationCountByPath)
	{
		DuplicateExpected += FMath::Max(Entry.Value - 1, 0);
	}
	TSet<FString> MissingMapPaths;
	for (const FString& Path : CoverageAudit.ExpectedReachableMapPaths)
	{
		if (!CoverageAudit.CoveredReachableMapPaths.Contains(Path))
		{
			MissingMapPaths.Add(Path);
		}
	}
	TSet<FString> UnexpectedMapPaths;
	for (const FString& Path : CoverageAudit.CoveredReachableMapPaths)
	{
		if (!CoverageAudit.ExpectedReachableMapPaths.Contains(Path))
		{
			UnexpectedMapPaths.Add(Path);
		}
	}
	TSet<FString> MissingNonReflectedSchemaFields;
	for (const FString& Path : CoverageAudit.ExpectedNonReflectedSchemaFields)
	{
		if (!CoverageAudit.CoveredNonReflectedSchemaFields.Contains(Path))
		{
			MissingNonReflectedSchemaFields.Add(Path);
		}
	}
	TSet<FString> CanonicalUninitializedFields;
	for (const FString& Path : CanonicalAudit.ExpectedFields)
	{
		if (!CanonicalAudit.ExplicitlyInitializedFields.Contains(Path))
		{
			CanonicalUninitializedFields.Add(Path);
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
		CoverageAudit.CoveredNonReflectedSchemaFields.Num(), 57);
	TestEqual(TEXT("Non-reflected expected inventory remains 57"),
		CoverageAudit.ExpectedNonReflectedSchemaFields.Num(), 57);
	TestEqual(TEXT("Non-reflected missing schema fields"),
		MissingNonReflectedSchemaFields.Num(), 0);
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
	TestEqual(TEXT("Duplicate Expected path registrations"),
		DuplicateExpected, static_cast<int64>(0));
	TestEqual(TEXT("Duplicate Covered path executions"),
		DuplicateExecutions, static_cast<int64>(0));
	TestEqual(TEXT("Mutation skipped groups"),
		CoverageAudit.SkippedGroupPaths.Num(), 0);
	TestTrue(TEXT("Reachable TMap paths are present"),
		CoverageAudit.ExpectedReachableMapPaths.Num() > 0);
	TestEqual(TEXT("Covered reachable TMap path count"),
		CoverageAudit.CoveredReachableMapPaths.Num(),
		CoverageAudit.ExpectedReachableMapPaths.Num());
	TestEqual(TEXT("Missing reachable TMap paths"), MissingMapPaths.Num(), 0);
	TestEqual(TEXT("Unexpected reachable TMap paths"), UnexpectedMapPaths.Num(), 0);
	TestEqual(TEXT("Canonical uninitialized relevant fields"),
		CanonicalUninitializedFields.Num(), 0);
	TestEqual(TEXT("Canonical unsupported reflected fields"),
		CanonicalAudit.UnsupportedFields.Num(), 0);
	TestEqual(TEXT("Every Covered path has an independent pair"),
		CoverageAudit.IndependentBaselineCandidatePairs,
		CoverageAudit.ExecutedMutationCases);
	TestEqual(TEXT("Every Covered path has address inequality"),
		CoverageAudit.AddressInequalityChecks,
		CoverageAudit.ExecutedMutationCases);
	TestEqual(TEXT("Every Covered path has pre-mutation equality"),
		CoverageAudit.PreMutationComparatorTrueChecks,
		CoverageAudit.ExecutedMutationCases);
	TestEqual(TEXT("Every Covered path rejects its mutation"),
		CoverageAudit.MutationComparatorFalseChecks,
		CoverageAudit.ExecutedMutationCases);
	TestEqual(TEXT("Every Covered path restores equality"),
		CoverageAudit.RestorationComparatorTrueChecks,
		CoverageAudit.ExecutedMutationCases);
	TestTrue(TEXT("Nine manual container graphs executed"),
		DeepContainerCases > 0 && CoverageAudit.EmptyContainerElementFieldPaths.Num() > 0);
	AddInfo(FString::Printf(TEXT("Mutation missing paths (%d): %s"),
		MissingPaths.Num(), *JoinPaths(MissingPaths)));
	AddInfo(FString::Printf(TEXT("Mutation unexpected paths (%d): %s"),
		UnexpectedPaths.Num(), *JoinPaths(UnexpectedPaths)));
	AddInfo(FString::Printf(TEXT("Mutation skipped groups (%d): %s"),
		CoverageAudit.SkippedGroupPaths.Num(),
		*JoinPaths(CoverageAudit.SkippedGroupPaths)));
	AddInfo(FString::Printf(TEXT("Reachable TMap paths (%d): %s"),
		CoverageAudit.ExpectedReachableMapPaths.Num(),
		*JoinPaths(CoverageAudit.ExpectedReachableMapPaths)));
	AddInfo(FString::Printf(TEXT("Canonical uninitialized fields (%d): %s"),
		CanonicalUninitializedFields.Num(),
		*JoinPaths(CanonicalUninitializedFields)));
	AddInfo(FString::Printf(
		TEXT("Foundation B runtime mutation audit: expected=%d; covered=%d; unique=%d; executions=%lld; duplicate expected=%lld; duplicate covered=%lld; reflected leaf unique=%d; non-reflected unique=%d; container behavior unique=%d; empty-element field unique=%d; envelope unique=%d; equal baselines=%lld; independent pairs=%lld; address checks=%lld; pre-equal=%lld; mutation-rejected=%lld; restored-equal=%lld; missing=%d; unexpected=%d; skipped=%d; reachable TMap expected=%d; reachable TMap covered=%d; canonical uninitialized=%d."),
		CoverageAudit.ExpectedLogicalPaths.Num(),
		CoverageAudit.CoveredLogicalPaths.Num(),
		CoverageAudit.CoveredLogicalPaths.Num(),
		CoverageAudit.ExecutedMutationCases,
		DuplicateExpected,
		DuplicateExecutions,
		CoverageAudit.ReflectedLeafPaths.Num(),
		CoverageAudit.NonReflectedFieldPaths.Num(),
		CoverageAudit.ContainerCasePaths.Num(),
		CoverageAudit.EmptyContainerElementFieldPaths.Num(),
		CoverageAudit.EnvelopePaths.Num(),
		CoverageAudit.EqualBaselineCases,
		CoverageAudit.IndependentBaselineCandidatePairs,
		CoverageAudit.AddressInequalityChecks,
		CoverageAudit.PreMutationComparatorTrueChecks,
		CoverageAudit.MutationComparatorFalseChecks,
		CoverageAudit.RestorationComparatorTrueChecks,
		MissingPaths.Num(),
		UnexpectedPaths.Num(),
		CoverageAudit.SkippedGroupPaths.Num(),
		CoverageAudit.ExpectedReachableMapPaths.Num(),
		CoverageAudit.CoveredReachableMapPaths.Num(),
		CanonicalUninitializedFields.Num()));
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
			TEXT("FMatchPlaySkillDecline::Decline(")),
		TPair<const TCHAR*, const TCHAR*>(TEXT("Runner writer"),
			TEXT("FMatchPlayCurrentAttackRunnerSelectionWriter::Select(")),
		TPair<const TCHAR*, const TCHAR*>(TEXT("No-legal runner"),
			TEXT("FMatchPlayResolveNoLegalRunner::Resolve(")),
		TPair<const TCHAR*, const TCHAR*>(TEXT("Decline runner"),
			TEXT("FMatchPlayRunnerDecline::Decline(")),
		TPair<const TCHAR*, const TCHAR*>(TEXT("Helper writer"),
			TEXT("FMatchPlayCurrentAttackHelperSelectionWriter::Select(")),
		TPair<const TCHAR*, const TCHAR*>(TEXT("No-legal helper"),
			TEXT("FMatchPlayResolveNoLegalHelper::Resolve(")),
		TPair<const TCHAR*, const TCHAR*>(TEXT("Decline helper"),
			TEXT("FMatchPlayHelperDecline::Decline(")),
		TPair<const TCHAR*, const TCHAR*>(TEXT("Resolution Session begin"),
			TEXT("FMatchPlayCurrentAttackBeginResolutionSessionWriter::Begin(")),
		TPair<const TCHAR*, const TCHAR*>(TEXT("Branch Intent writer"),
			TEXT("FMatchPlayCurrentAttackBranchIntentSelectionWriter::Select(")),
		TPair<const TCHAR*, const TCHAR*>(TEXT("Intent-determined route writer"),
			TEXT("FMatchPlayCurrentAttackResolveInitialRouteWriter::Resolve(")),
		TPair<const TCHAR*, const TCHAR*>(TEXT("Initial Route orchestrator"),
			TEXT("FMatchPlayCurrentAttackResolveInitialRouteOrchestrator::Resolve(")) })
	{
		TestEqual(*FString::Printf(TEXT("%s has one production call"), Operation.Key),
			CountOccurrences(Implementation, Operation.Value),
			1);
	}
	TestEqual(TEXT("All twenty-one mutations share serialized gate"),
		CountOccurrences(Implementation, TEXT("ExecuteSerialized<")),
		21);
	TestEqual(TEXT("Session retains one state replacement"),
		CountOccurrences(
			Implementation,
			TEXT("AuthoritativeState = Adoption.AdoptedAfterState;")),
		1);
	TestFalse(TEXT("No public system marker request type"),
		Types.Contains(TEXT("FMatchPlayAuthoritativeResolveNoLegalMarkerRequest")));
	TestFalse(TEXT("No public system skill request type"),
		Types.Contains(TEXT("FMatchPlayAuthoritativeResolveNoLegalSkillRequest")));
	TestFalse(TEXT("No public system runner request type"),
		Types.Contains(TEXT("FMatchPlayAuthoritativeResolveNoLegalRunnerRequest")));
	TestFalse(TEXT("No public system helper request type"),
		Types.Contains(TEXT("FMatchPlayAuthoritativeResolveNoLegalHelperRequest")));
	TestFalse(TEXT("Skill rules are not stored on Session"),
		Header.Contains(TEXT("FSkillRuleSnapshotSet SkillRuleSet;")));
	for (const TCHAR* ForbiddenWrite : {
		TEXT("AuthoritativeState.CurrentAttack.ActionPreparation.MarkerCardId ="),
		TEXT("AuthoritativeState.CurrentAttack.ActionPreparation.SkillId ="),
		TEXT("AuthoritativeState.CurrentAttack.ActionPreparation.RunnerCardId ="),
		TEXT("AuthoritativeState.CurrentAttack.ActionPreparation.HelperCardId ="),
		TEXT("AuthoritativeState.CurrentAttack.ActionPreparation.bHasHelper ="),
		TEXT("AuthoritativeState.CurrentAttack.SelectedAction.ElectiveBranchIntent ="),
		TEXT("AuthoritativeState.CurrentAttack.bHasResolutionSession ="),
		TEXT("AuthoritativeState.CurrentAttack.ResolutionSession ="),
		TEXT("AuthoritativeState.CurrentAttack.SelectionStage ="),
		TEXT("AuthoritativeState.bHasCurrentAttack ="),
		TEXT("AuthoritativeState.CurrentAttack.DeploymentPlacements.Add") })
	{
		TestFalse(*FString::Printf(TEXT("Direct gameplay write absent: %s"), ForbiddenWrite),
			Implementation.Contains(ForbiddenWrite));
	}
	const FString Production = Header + Implementation + Types;
	for (const TCHAR* Forbidden : {
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

MATCH_PLAY_AUTHORITATIVE_SESSION_TEST(
	FMatchPlayAuthoritativeSessionRunnerSurfaceAndSubmitTest,
	"33.RunnerSurfaceSubmitAndFailures")

bool FMatchPlayAuthoritativeSessionRunnerSurfaceAndSubmitTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayAuthoritativeSessionTests;
	static_assert(std::is_same_v<
		decltype(&FMatchPlayAuthoritativeSession::SubmitRunner),
		FMatchPlayAuthoritativeSubmitRunnerResult
		(FMatchPlayAuthoritativeSession::*)(
			const FMatchPlayAuthoritativeSubmitRunnerRequest&)>);
	static_assert(std::is_same_v<
		decltype(&FMatchPlayAuthoritativeSession::ResolveNoLegalRunner),
		FMatchPlayAuthoritativeResolveNoLegalRunnerResult
		(FMatchPlayAuthoritativeSession::*)()>);
	static_assert(std::is_same_v<
		decltype(&FMatchPlayAuthoritativeSession::DeclineRunner),
		FMatchPlayAuthoritativeDeclineRunnerResult
		(FMatchPlayAuthoritativeSession::*)(
			const FMatchPlayAuthoritativeDeclineRunnerRequest&)>);
	static_assert(std::is_same_v<
		decltype(FMatchPlayAuthoritativeSubmitRunnerResult::RunnerResult),
		FMatchPlayCurrentAttackRunnerSelectionWriterResult>);
	static_assert(std::is_same_v<
		decltype(FMatchPlayAuthoritativeResolveNoLegalRunnerResult
			::ResolutionResult),
		FMatchPlayResolveNoLegalRunnerResult>);
	static_assert(std::is_same_v<
		decltype(FMatchPlayAuthoritativeDeclineRunnerResult::DeclineResult),
		FMatchPlayRunnerDeclineResult>);

	TestEqual(TEXT("SubmitRunner follows DeclineSkill"),
		static_cast<uint8>(EMatchPlayAuthoritativeCommandKind::SubmitRunner),
		static_cast<uint8>(EMatchPlayAuthoritativeCommandKind::DeclineSkill) + 1);
	TestEqual(TEXT("DeclineRunner is final command"),
		static_cast<uint8>(EMatchPlayAuthoritativeCommandKind::DeclineRunner),
		static_cast<uint8>(EMatchPlayAuthoritativeCommandKind::SubmitRunner) + 2);

	FString Types;
	TestTrue(TEXT("Runner types source loads"), LoadProductionSource(
		TEXT("Source/FMCodex/MatchPlayRuntime/MatchPlayAuthoritativeSessionTypes.h"),
		Types));
	auto RequestContainsAttackSequence = [&Types](const TCHAR* StructName)
	{
		const int32 Begin = Types.Find(StructName);
		const int32 End = Begin == INDEX_NONE
			? INDEX_NONE
			: Types.Find(TEXT("};"), ESearchCase::CaseSensitive,
				ESearchDir::FromStart, Begin);
		return Begin == INDEX_NONE || End == INDEX_NONE
			|| Types.Mid(Begin, End - Begin).Contains(TEXT("AttackSequence"));
	};
	for (const TCHAR* RequestType : {
		TEXT("FMatchPlayAuthoritativeSubmitRunnerRequest"),
		TEXT("FMatchPlayAuthoritativeDeclineRunnerRequest") })
	{
		TestFalse(*FString::Printf(TEXT("%s has no public AttackSequence"), RequestType),
			RequestContainsAttackSequence(RequestType));
	}

	FMatchPlayAuthoritativeSubmitRunnerRequest EmptyRequest;
	const auto Uninitialized =
		FMatchPlayAuthoritativeSession().SubmitRunner(EmptyRequest);
	TestFalse(TEXT("Uninitialized runner submit rejected"),
		Uninitialized.RuntimeEnvelope.bAccepted);
	TestEqual(TEXT("Uninitialized runner submit exact runtime code"),
		Uninitialized.RuntimeEnvelope.RuntimeFailureCode,
		EMatchPlayAuthoritativeRuntimeFailureCode::NotInitialized);

	FMatchPlayAuthoritativeSession Session;
	FReachabilityTrace Trace;
	FName RunnerCardId;
	TestTrue(TEXT("Public commands reach AwaitingRunner with legal runner"),
		BuildStage7162ToAwaitingRunner(
			Session,
			TEXT("RunnerSubmit"),
			true,
			Trace,
			RunnerCardId));
	const FMatchPlayState Before = Session.GetStateSnapshot();
	TestEqual(TEXT("Reachability exact stage"),
		Before.CurrentAttack.SelectionStage,
		EMatchPlayCurrentAttackSelectionStage::AwaitingRunner);

	FMatchPlayAuthoritativeSubmitRunnerRequest Request;
	Request.RequestingSide = Trace.DefendingSide;
	Request.RunnerCardId = RunnerCardId;
	const auto WrongSide = Session.SubmitRunner(Request);
	TestEqual(TEXT("Wrong-side runner exact error"),
		WrongSide.RunnerResult.LegalityResult.ErrorCode,
		EMatchPlayCurrentAttackRunnerSelectionErrorCode
			::RequestingSideIsNotCurrentAttacker);
	TestAcceptedDomainFailureNoAdopt(
		*this,
		TEXT("Wrong-side runner"),
		WrongSide.RuntimeEnvelope,
		Before,
		Session.GetStateSnapshot());
	TestEqual(TEXT("Wrong-side sequence derived internally"),
		WrongSide.RunnerResult.Request.AttackSequence,
		Before.CurrentAttack.AttackSequence);

	Request.RequestingSide = Trace.AttackingSide;
	Request.RunnerCardId = TEXT("Runner.Stage7162.Missing");
	const auto Invalid = Session.SubmitRunner(Request);
	TestEqual(TEXT("Missing runner exact error"),
		Invalid.RunnerResult.LegalityResult.ErrorCode,
		EMatchPlayCurrentAttackRunnerSelectionErrorCode::RunnerNotDeployed);
	TestAcceptedDomainFailureNoAdopt(
		*this,
		TEXT("Missing runner"),
		Invalid.RuntimeEnvelope,
		Before,
		Session.GetStateSnapshot());

	Request.RunnerCardId = RunnerCardId;
	const auto Success = Session.SubmitRunner(Request);
	const FMatchPlayState After = Session.GetStateSnapshot();
	TestTrue(TEXT("Legal runner succeeds"), Success.RunnerResult.bSuccess);
	TestEqual(TEXT("SubmitRunner exact command"),
		Success.RuntimeEnvelope.CommandKind,
		EMatchPlayAuthoritativeCommandKind::SubmitRunner);
	TestEqual(TEXT("SubmitRunner derives authoritative sequence"),
		Success.RunnerResult.Request.AttackSequence,
		Before.CurrentAttack.AttackSequence);
	TestEqual(TEXT("Envelope reports derived sequence"),
		Success.RuntimeEnvelope.AttackSequence,
		Before.CurrentAttack.AttackSequence);
	TestEqual(TEXT("Exact runner selected"),
		After.CurrentAttack.ActionPreparation.RunnerCardId,
		RunnerCardId);
	TestEqual(TEXT("Runner success reaches AwaitingHelper"),
		After.CurrentAttack.SelectionStage,
		EMatchPlayCurrentAttackSelectionStage::AwaitingHelper);
	TestAdoptedSuccessEnvelope(
		*this,
		TEXT("SubmitRunner"),
		Success.RuntimeEnvelope,
		Before,
		Success.RunnerResult.AfterState,
		After);

	const auto Replay = Session.SubmitRunner(Request);
	TestEqual(TEXT("Runner replay exact stage error"),
		Replay.RunnerResult.LegalityResult.ErrorCode,
		EMatchPlayCurrentAttackRunnerSelectionErrorCode::WrongSelectionStage);
	TestAcceptedDomainFailureNoAdopt(
		*this,
		TEXT("Runner replay"),
		Replay.RuntimeEnvelope,
		After,
		Session.GetStateSnapshot());
	return true;
}

MATCH_PLAY_AUTHORITATIVE_SESSION_TEST(
	FMatchPlayAuthoritativeSessionRunnerCompletionPathsTest,
	"34.RunnerNoLegalAndDeclinePaths")

bool FMatchPlayAuthoritativeSessionRunnerCompletionPathsTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayAuthoritativeSessionTests;
	FMatchPlayAuthoritativeSession NoLegalSession;
	FReachabilityTrace NoLegalTrace;
	FName NoLegalRunner;
	TestTrue(TEXT("Public commands reach AwaitingRunner without legal runner"),
		BuildStage7162ToAwaitingRunner(
			NoLegalSession,
			TEXT("RunnerNoLegal"),
			false,
			NoLegalTrace,
			NoLegalRunner));
	const FMatchPlayState NoLegalBefore = NoLegalSession.GetStateSnapshot();
	const auto Resolved = NoLegalSession.ResolveNoLegalRunner();
	TestTrue(TEXT("No-legal runner resolves"),
		Resolved.ResolutionResult.bSuccess);
	TestFalse(TEXT("No legal runner existed"),
		Resolved.ResolutionResult.RunnerAvailabilityResult.bCanSelectAnyRunner);
	TestEqual(TEXT("No-legal runner source stays exact"),
		Resolved.ResolutionResult.Source,
		EMatchPlayRunnerNoSelectionNoGoalSource::ResolveNoLegalRunner);
	TestEqual(TEXT("No-legal runner reason stays exact"),
		Resolved.ResolutionResult.Reason,
		EMatchPlayRunnerNoSelectionNoGoalReason::NoLegalRunner);
	TestEqual(TEXT("No-legal sequence derived internally"),
		Resolved.ResolutionResult.Request.AttackSequence,
		NoLegalBefore.CurrentAttack.AttackSequence);
	TestAdoptedSuccessEnvelope(
		*this,
		TEXT("ResolveNoLegalRunner"),
		Resolved.RuntimeEnvelope,
		NoLegalBefore,
		Resolved.ResolutionResult.AfterState,
		NoLegalSession.GetStateSnapshot());
	TestCompletedAttackEndpoint(
		*this,
		TEXT("ResolveNoLegalRunner"),
		Resolved.ResolutionResult.CompletionResult,
		NoLegalSession.GetStateSnapshot());
	TestCompletionScoringContract(
		*this,
		TEXT("ResolveNoLegalRunner"),
		NoLegalBefore,
		Resolved.ResolutionResult.CompletionResult,
		NoLegalSession.GetStateSnapshot(),
		ECompletionScoringExpectation::NoGoal,
		NoLegalTrace.AttackingSide,
		true);

	FMatchPlayAuthoritativeSession NoLegalDeclineSession;
	FReachabilityTrace NoLegalDeclineTrace;
	FName NoLegalDeclineRunner;
	BuildStage7162ToAwaitingRunner(
		NoLegalDeclineSession,
		TEXT("RunnerNoLegalDecline"),
		false,
		NoLegalDeclineTrace,
		NoLegalDeclineRunner);
	FMatchPlayAuthoritativeDeclineRunnerRequest DeclineRequest;
	DeclineRequest.RequestingSide = NoLegalDeclineTrace.AttackingSide;
	const FMatchPlayState NoLegalDeclineBefore =
		NoLegalDeclineSession.GetStateSnapshot();
	const auto NoLegalDecline =
		NoLegalDeclineSession.DeclineRunner(DeclineRequest);
	TestEqual(TEXT("No-legal runner cannot use decline path"),
		NoLegalDecline.DeclineResult.ErrorCode,
		EMatchPlayRunnerNoSelectionNoGoalErrorCode::NoLegalRunnerToDecline);
	TestAcceptedDomainFailureNoAdopt(
		*this,
		TEXT("No-legal runner decline"),
		NoLegalDecline.RuntimeEnvelope,
		NoLegalDeclineBefore,
		NoLegalDeclineSession.GetStateSnapshot());

	FMatchPlayAuthoritativeSession LegalSession;
	FReachabilityTrace LegalTrace;
	FName LegalRunner;
	TestTrue(TEXT("Decline fixture reaches legal runner"),
		BuildStage7162ToAwaitingRunner(
			LegalSession,
			TEXT("RunnerDecline"),
			true,
			LegalTrace,
			LegalRunner));
	const FMatchPlayState LegalBefore = LegalSession.GetStateSnapshot();
	const auto LegalResolve = LegalSession.ResolveNoLegalRunner();
	TestEqual(TEXT("Legal runner blocks no-legal path"),
		LegalResolve.ResolutionResult.ErrorCode,
		EMatchPlayRunnerNoSelectionNoGoalErrorCode::LegalRunnerExists);
	TestAcceptedDomainFailureNoAdopt(
		*this,
		TEXT("Legal runner no-legal path"),
		LegalResolve.RuntimeEnvelope,
		LegalBefore,
		LegalSession.GetStateSnapshot());

	DeclineRequest.RequestingSide = LegalTrace.DefendingSide;
	const auto WrongSide = LegalSession.DeclineRunner(DeclineRequest);
	TestEqual(TEXT("Wrong-side runner decline exact error"),
		WrongSide.DeclineResult.ErrorCode,
		EMatchPlayRunnerNoSelectionNoGoalErrorCode
			::RequestingSideIsNotCurrentAttacker);
	TestAcceptedDomainFailureNoAdopt(
		*this,
		TEXT("Wrong-side runner decline"),
		WrongSide.RuntimeEnvelope,
		LegalBefore,
		LegalSession.GetStateSnapshot());

	DeclineRequest.RequestingSide = LegalTrace.AttackingSide;
	const auto Declined = LegalSession.DeclineRunner(DeclineRequest);
	TestTrue(TEXT("Legal runner decline succeeds"),
		Declined.DeclineResult.bSuccess);
	TestTrue(TEXT("Decline observed legal runner"),
		Declined.DeclineResult.RunnerAvailabilityResult.bCanSelectAnyRunner);
	TestEqual(TEXT("Runner decline source stays exact"),
		Declined.DeclineResult.Source,
		EMatchPlayRunnerNoSelectionNoGoalSource::RunnerDecline);
	TestEqual(TEXT("Runner decline reason stays exact"),
		Declined.DeclineResult.Reason,
		EMatchPlayRunnerNoSelectionNoGoalReason::RunnerDeclined);
	TestEqual(TEXT("Decline sequence derived internally"),
		Declined.DeclineResult.Request.AttackSequence,
		LegalBefore.CurrentAttack.AttackSequence);
	TestAdoptedSuccessEnvelope(
		*this,
		TEXT("DeclineRunner"),
		Declined.RuntimeEnvelope,
		LegalBefore,
		Declined.DeclineResult.AfterState,
		LegalSession.GetStateSnapshot());
	TestFalse(TEXT("NoLegal and Decline sources remain distinct"),
		Resolved.ResolutionResult.Source == Declined.DeclineResult.Source);
	TestFalse(TEXT("NoLegal and Decline reasons remain distinct"),
		Resolved.ResolutionResult.Reason == Declined.DeclineResult.Reason);
	return true;
}

MATCH_PLAY_AUTHORITATIVE_SESSION_TEST(
	FMatchPlayAuthoritativeSessionRunnerDeterminismIsolationTest,
	"35.RunnerIsolationAndDeterminism")

bool FMatchPlayAuthoritativeSessionRunnerDeterminismIsolationTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayAuthoritativeSessionTests;
	TArray<FMatchPlayAuthoritativeSubmitRunnerResult> Submits;
	TArray<FMatchPlayAuthoritativeResolveNoLegalRunnerResult> Resolves;
	TArray<FMatchPlayAuthoritativeDeclineRunnerResult> Declines;
	TArray<FMatchPlayState> SubmitFinals;
	TArray<FMatchPlayState> ResolveFinals;
	TArray<FMatchPlayState> DeclineFinals;
	for (int32 Index = 0; Index < 3; ++Index)
	{
		FMatchPlayAuthoritativeSession SubmitSession;
		FReachabilityTrace SubmitTrace;
		FName RunnerCardId;
		TestTrue(TEXT("Submit determinism fixture reaches Runner"),
			BuildStage7162ToAwaitingRunner(
			SubmitSession,
			TEXT("RunnerSubmitDeterminism"),
			true,
			SubmitTrace,
			RunnerCardId));
		FMatchPlayAuthoritativeSubmitRunnerRequest SubmitRequest;
		SubmitRequest.RequestingSide = SubmitTrace.AttackingSide;
		SubmitRequest.RunnerCardId = RunnerCardId;
		Submits.Add(SubmitSession.SubmitRunner(SubmitRequest));
		SubmitFinals.Add(SubmitSession.GetStateSnapshot());

		FMatchPlayAuthoritativeSession ResolveSession;
		FReachabilityTrace ResolveTrace;
		FName UnusedRunner;
		TestTrue(TEXT("Resolve determinism fixture reaches Runner"),
			BuildStage7162ToAwaitingRunner(
			ResolveSession,
			TEXT("RunnerResolveDeterminism"),
			false,
			ResolveTrace,
			UnusedRunner));
		Resolves.Add(ResolveSession.ResolveNoLegalRunner());
		ResolveFinals.Add(ResolveSession.GetStateSnapshot());

		FMatchPlayAuthoritativeSession DeclineSession;
		FReachabilityTrace DeclineTrace;
		FName DeclineRunnerId;
		TestTrue(TEXT("Decline determinism fixture reaches Runner"),
			BuildStage7162ToAwaitingRunner(
			DeclineSession,
			TEXT("RunnerDeclineDeterminism"),
			true,
			DeclineTrace,
			DeclineRunnerId));
		FMatchPlayAuthoritativeDeclineRunnerRequest DeclineRequest;
		DeclineRequest.RequestingSide = DeclineTrace.AttackingSide;
		Declines.Add(DeclineSession.DeclineRunner(DeclineRequest));
		DeclineFinals.Add(DeclineSession.GetStateSnapshot());
	}
	for (int32 Index = 1; Index < 3; ++Index)
	{
		TestTrue(TEXT("SubmitRunner full result deterministic"),
			AreAuthoritativeSubmitRunnerResultsEqual(Submits[0], Submits[Index]));
		TestTrue(TEXT("ResolveNoLegalRunner full result deterministic"),
			AreAuthoritativeResolveNoLegalRunnerResultsEqual(
				Resolves[0], Resolves[Index]));
		TestTrue(TEXT("DeclineRunner full result deterministic"),
			AreAuthoritativeDeclineRunnerResultsEqual(
				Declines[0], Declines[Index]));
		TestTrue(TEXT("SubmitRunner final state deterministic"),
			AreStatesEqual(SubmitFinals[0], SubmitFinals[Index]));
		TestTrue(TEXT("ResolveNoLegalRunner final state deterministic"),
			AreStatesEqual(ResolveFinals[0], ResolveFinals[Index]));
		TestTrue(TEXT("DeclineRunner final state deterministic"),
			AreStatesEqual(DeclineFinals[0], DeclineFinals[Index]));
	}

	FMatchPlayAuthoritativeSession SessionA;
	FMatchPlayAuthoritativeSession SessionB;
	FReachabilityTrace TraceA;
	FReachabilityTrace TraceB;
	FName RunnerA;
	FName RunnerB;
	TestTrue(TEXT("Isolation fixture A reaches Runner"),
		BuildStage7162ToAwaitingRunner(
			SessionA, TEXT("RunnerIsolationA"), true, TraceA, RunnerA));
	TestTrue(TEXT("Isolation fixture B reaches Runner"),
		BuildStage7162ToAwaitingRunner(
			SessionB, TEXT("RunnerIsolationB"), true, TraceB, RunnerB));
	FMatchPlayAuthoritativeSubmitRunnerRequest RequestA;
	RequestA.RequestingSide = TraceA.AttackingSide;
	RequestA.RunnerCardId = RunnerA;
	const FMatchPlayState BBefore = SessionB.GetStateSnapshot();
	SessionA.SubmitRunner(RequestA);
	TestTrue(TEXT("Runner command on A cannot mutate B"),
		AreStatesEqual(BBefore, SessionB.GetStateSnapshot()));
	const FMatchPlayState AAfter = SessionA.GetStateSnapshot();
	FMatchPlayAuthoritativeDeclineRunnerRequest RequestB;
	RequestB.RequestingSide = TraceB.AttackingSide;
	SessionB.DeclineRunner(RequestB);
	TestTrue(TEXT("Runner command on B cannot mutate A"),
		AreStatesEqual(AAfter, SessionA.GetStateSnapshot()));
	return true;
}

MATCH_PLAY_AUTHORITATIVE_SESSION_TEST(
	FMatchPlayAuthoritativeSessionSubmitHelperTest,
	"36.SubmitHelperSuccessFailuresAndDeterminism")

bool FMatchPlayAuthoritativeSessionSubmitHelperTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayAuthoritativeSessionTests;
	static_assert(std::is_same_v<
		decltype(&FMatchPlayAuthoritativeSession::SubmitHelper),
		FMatchPlayAuthoritativeSubmitHelperResult
		(FMatchPlayAuthoritativeSession::*)(
			const FMatchPlayAuthoritativeSubmitHelperRequest&)>);
	static_assert(std::is_same_v<
		decltype(&FMatchPlayAuthoritativeSession::ResolveNoLegalHelper),
		FMatchPlayAuthoritativeResolveNoLegalHelperResult
		(FMatchPlayAuthoritativeSession::*)()>);
	static_assert(std::is_same_v<
		decltype(&FMatchPlayAuthoritativeSession::DeclineHelper),
		FMatchPlayAuthoritativeDeclineHelperResult
		(FMatchPlayAuthoritativeSession::*)(
			const FMatchPlayAuthoritativeDeclineHelperRequest&)>);
	static_assert(std::is_same_v<
		decltype(FMatchPlayAuthoritativeSubmitHelperResult::HelperResult),
		FMatchPlayCurrentAttackHelperSelectionWriterResult>);
	static_assert(std::is_same_v<
		decltype(FMatchPlayAuthoritativeResolveNoLegalHelperResult
			::ResolutionResult),
		FMatchPlayResolveNoLegalHelperResult>);
	static_assert(std::is_same_v<
		decltype(FMatchPlayAuthoritativeDeclineHelperResult::DeclineResult),
		FMatchPlayHelperDeclineResult>);

	TestEqual(TEXT("SubmitHelper follows DeclineRunner"),
		static_cast<uint8>(EMatchPlayAuthoritativeCommandKind::SubmitHelper),
		static_cast<uint8>(EMatchPlayAuthoritativeCommandKind::DeclineRunner) + 1);
	TestEqual(TEXT("BeginResolutionSession follows DeclineHelper"),
		static_cast<uint8>(
			EMatchPlayAuthoritativeCommandKind::BeginResolutionSession),
		static_cast<uint8>(EMatchPlayAuthoritativeCommandKind::DeclineHelper) + 1);

	FString Types;
	TestTrue(TEXT("Helper types source loads"), LoadProductionSource(
		TEXT("Source/FMCodex/MatchPlayRuntime/MatchPlayAuthoritativeSessionTypes.h"),
		Types));
	auto RequestContainsAttackSequence = [&Types](const TCHAR* StructName)
	{
		const int32 Begin = Types.Find(StructName);
		const int32 End = Begin == INDEX_NONE
			? INDEX_NONE
			: Types.Find(TEXT("};"), ESearchCase::CaseSensitive,
				ESearchDir::FromStart, Begin);
		return Begin == INDEX_NONE || End == INDEX_NONE
			|| Types.Mid(Begin, End - Begin).Contains(TEXT("AttackSequence"));
	};
	for (const TCHAR* RequestType : {
		TEXT("FMatchPlayAuthoritativeSubmitHelperRequest"),
		TEXT("FMatchPlayAuthoritativeDeclineHelperRequest") })
	{
		TestFalse(*FString::Printf(TEXT("%s has no public AttackSequence"), RequestType),
			RequestContainsAttackSequence(RequestType));
	}

	FMatchPlayAuthoritativeSubmitHelperRequest EmptyRequest;
	const auto Uninitialized =
		FMatchPlayAuthoritativeSession().SubmitHelper(EmptyRequest);
	TestFalse(TEXT("Uninitialized Helper submit rejected"),
		Uninitialized.RuntimeEnvelope.bAccepted);
	TestEqual(TEXT("Uninitialized Helper exact runtime code"),
		Uninitialized.RuntimeEnvelope.RuntimeFailureCode,
		EMatchPlayAuthoritativeRuntimeFailureCode::NotInitialized);

	FMatchPlayAuthoritativeSession Session;
	FReachabilityTrace Trace;
	FName HelperCardId;
	TestTrue(TEXT("Public commands reach AwaitingHelper with legal Helper"),
		BuildStage7163ToAwaitingHelper(
			Session,
			TEXT("HelperSubmit"),
			true,
			Trace,
			HelperCardId));
	const FMatchPlayState Before = Session.GetStateSnapshot();
	TestEqual(TEXT("Reachability exact Helper stage"),
		Before.CurrentAttack.SelectionStage,
		EMatchPlayCurrentAttackSelectionStage::AwaitingHelper);

	FMatchPlayAuthoritativeSubmitHelperRequest Request;
	Request.RequestingSide = Trace.AttackingSide;
	Request.HelperCardId = HelperCardId;
	const auto WrongSide = Session.SubmitHelper(Request);
	TestEqual(TEXT("Wrong-side Helper exact error"),
		WrongSide.HelperResult.LegalityResult.ErrorCode,
		EMatchPlayCurrentAttackHelperSelectionErrorCode
			::GlobalContextFailed);
	TestEqual(TEXT("Wrong-side Helper nested exact error"),
		WrongSide.HelperResult.LegalityResult.GlobalContextResult.ErrorCode,
		EMatchPlayCurrentAttackHelperSelectionErrorCode
			::RequestingSideIsNotCurrentDefender);
	TestAcceptedDomainFailureNoAdopt(
		*this,
		TEXT("Wrong-side Helper"),
		WrongSide.RuntimeEnvelope,
		Before,
		Session.GetStateSnapshot());
	TestEqual(TEXT("Wrong-side Helper sequence derived internally"),
		WrongSide.HelperResult.Request.AttackSequence,
		Before.CurrentAttack.AttackSequence);

	Request.RequestingSide = Trace.DefendingSide;
	Request.HelperCardId = TEXT("Helper.Stage7163.Missing");
	const auto Invalid = Session.SubmitHelper(Request);
	TestEqual(TEXT("Missing Helper exact error"),
		Invalid.HelperResult.LegalityResult.ErrorCode,
		EMatchPlayCurrentAttackHelperSelectionErrorCode::HelperNotDeployed);
	TestAcceptedDomainFailureNoAdopt(
		*this,
		TEXT("Missing Helper"),
		Invalid.RuntimeEnvelope,
		Before,
		Session.GetStateSnapshot());

	Request.HelperCardId = HelperCardId;
	const auto Success = Session.SubmitHelper(Request);
	const FMatchPlayState After = Session.GetStateSnapshot();
	TestTrue(TEXT("Legal Helper succeeds"), Success.HelperResult.bSuccess);
	TestTrue(TEXT("Helper Writer validates Ready state"),
		Success.HelperResult.ReadyValidationResult.bSuccess);
	TestEqual(TEXT("SubmitHelper exact command"),
		Success.RuntimeEnvelope.CommandKind,
		EMatchPlayAuthoritativeCommandKind::SubmitHelper);
	TestEqual(TEXT("SubmitHelper derives authoritative sequence"),
		Success.HelperResult.Request.AttackSequence,
		Before.CurrentAttack.AttackSequence);
	TestEqual(TEXT("SubmitHelper reaches ReadyForResolution"),
		After.CurrentAttack.SelectionStage,
		EMatchPlayCurrentAttackSelectionStage::ReadyForResolution);
	TestTrue(TEXT("Selected action is finalized"),
		After.CurrentAttack.bHasSelectedAction);
	TestTrue(TEXT("Selected action records Helper presence"),
		After.CurrentAttack.SelectedAction.bHasHelper);
	TestEqual(TEXT("Selected action records exact Helper"),
		After.CurrentAttack.SelectedAction.HelperCardId,
		HelperCardId);
	TestFalse(TEXT("Preparation is reset after finalization"),
		After.CurrentAttack.ActionPreparation.bHasHelper);
	TestTrue(TEXT("Preparation Helper identity is reset"),
		After.CurrentAttack.ActionPreparation.HelperCardId.IsNone());
	TestTrue(TEXT("Helper selection does not complete attack"),
		After.bHasCurrentAttack);
	TestAdoptedSuccessEnvelope(
		*this,
		TEXT("SubmitHelper"),
		Success.RuntimeEnvelope,
		Before,
		Success.HelperResult.AfterState,
		After);

	const auto Replay = Session.SubmitHelper(Request);
	TestEqual(TEXT("Helper replay exact stage error"),
		Replay.HelperResult.LegalityResult.ErrorCode,
		EMatchPlayCurrentAttackHelperSelectionErrorCode::GlobalContextFailed);
	TestEqual(TEXT("Helper replay nested exact stage error"),
		Replay.HelperResult.LegalityResult.GlobalContextResult.ErrorCode,
		EMatchPlayCurrentAttackHelperSelectionErrorCode::WrongSelectionStage);
	TestAcceptedDomainFailureNoAdopt(
		*this,
		TEXT("Helper replay"),
		Replay.RuntimeEnvelope,
		After,
		Session.GetStateSnapshot());

	TArray<FMatchPlayAuthoritativeSubmitHelperResult> Results;
	TArray<FMatchPlayState> FinalStates;
	for (int32 Index = 0; Index < 3; ++Index)
	{
		FMatchPlayAuthoritativeSession DeterministicSession;
		FReachabilityTrace DeterministicTrace;
		FName DeterministicHelper;
		TestTrue(TEXT("Submit Helper determinism fixture reaches stage"),
			BuildStage7163ToAwaitingHelper(
				DeterministicSession,
				TEXT("HelperSubmitDeterminism"),
				true,
				DeterministicTrace,
				DeterministicHelper));
		FMatchPlayAuthoritativeSubmitHelperRequest DeterministicRequest;
		DeterministicRequest.RequestingSide =
			DeterministicTrace.DefendingSide;
		DeterministicRequest.HelperCardId = DeterministicHelper;
		Results.Add(DeterministicSession.SubmitHelper(DeterministicRequest));
		FinalStates.Add(DeterministicSession.GetStateSnapshot());
	}
	for (int32 Index = 1; Index < 3; ++Index)
	{
		TestTrue(TEXT("SubmitHelper full result deterministic"),
			AreAuthoritativeSubmitHelperResultsEqual(Results[0], Results[Index]));
		TestTrue(TEXT("SubmitHelper final state deterministic"),
			AreStatesEqual(FinalStates[0], FinalStates[Index]));
	}

	FMatchPlayAuthoritativeSession SessionA;
	FMatchPlayAuthoritativeSession SessionB;
	FReachabilityTrace TraceA;
	FReachabilityTrace TraceB;
	FName HelperA;
	FName HelperB;
	TestTrue(TEXT("Submit Helper isolation fixture A reaches stage"),
		BuildStage7163ToAwaitingHelper(
			SessionA, TEXT("HelperSubmitIsolationA"), true, TraceA, HelperA));
	TestTrue(TEXT("Submit Helper isolation fixture B reaches stage"),
		BuildStage7163ToAwaitingHelper(
			SessionB, TEXT("HelperSubmitIsolationB"), true, TraceB, HelperB));
	FMatchPlayAuthoritativeSubmitHelperRequest RequestA;
	RequestA.RequestingSide = TraceA.DefendingSide;
	RequestA.HelperCardId = HelperA;
	const FMatchPlayState BBefore = SessionB.GetStateSnapshot();
	SessionA.SubmitHelper(RequestA);
	TestTrue(TEXT("SubmitHelper on A cannot mutate B"),
		AreStatesEqual(BBefore, SessionB.GetStateSnapshot()));
	return true;
}

MATCH_PLAY_AUTHORITATIVE_SESSION_TEST(
	FMatchPlayAuthoritativeSessionResolveNoLegalHelperTest,
	"37.ResolveNoLegalHelperFormalAbsence")

bool FMatchPlayAuthoritativeSessionResolveNoLegalHelperTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayAuthoritativeSessionTests;
	const auto Uninitialized =
		FMatchPlayAuthoritativeSession().ResolveNoLegalHelper();
	TestFalse(TEXT("Uninitialized no-legal Helper rejected"),
		Uninitialized.RuntimeEnvelope.bAccepted);

	FMatchPlayAuthoritativeSession Session;
	FReachabilityTrace Trace;
	FName NoHelper;
	TestTrue(TEXT("Public commands reach AwaitingHelper without legal Helper"),
		BuildStage7163ToAwaitingHelper(
			Session,
			TEXT("HelperNoLegal"),
			false,
			Trace,
			NoHelper));
	const FMatchPlayState Before = Session.GetStateSnapshot();
	const auto Success = Session.ResolveNoLegalHelper();
	const FMatchPlayState After = Session.GetStateSnapshot();
	TestTrue(TEXT("No-legal Helper resolves"),
		Success.ResolutionResult.bSuccess);
	TestFalse(TEXT("No Helper was selectable"),
		Success.ResolutionResult.HelperAvailabilityResult.bCanSelectAnyHelper);
	TestEqual(TEXT("No-legal Helper exact source"),
		Success.ResolutionResult.Source,
		EMatchPlayHelperAbsenceSource::ResolveNoLegalHelper);
	TestEqual(TEXT("No-legal Helper exact reason"),
		Success.ResolutionResult.Reason,
		EMatchPlayHelperAbsenceReason::NoLegalHelper);
	TestEqual(TEXT("No-legal Helper sequence derived internally"),
		Success.ResolutionResult.Request.AttackSequence,
		Before.CurrentAttack.AttackSequence);
	TestEqual(TEXT("No-legal Helper reaches ReadyForResolution"),
		After.CurrentAttack.SelectionStage,
		EMatchPlayCurrentAttackSelectionStage::ReadyForResolution);
	TestTrue(TEXT("No-legal Helper finalizes selected action"),
		After.CurrentAttack.bHasSelectedAction);
	TestFalse(TEXT("No-legal Helper records formal absence"),
		After.CurrentAttack.SelectedAction.bHasHelper);
	TestTrue(TEXT("No-legal Helper identity is empty"),
		After.CurrentAttack.SelectedAction.HelperCardId.IsNone());
	TestTrue(TEXT("No-legal Helper does not complete attack"),
		After.bHasCurrentAttack);
	TestTrue(TEXT("Formal absence Ready validation succeeds"),
		Success.ResolutionResult.FinalizationResult
			.ReadyValidationResult.bSuccess);
	TestFalse(TEXT("Formal absence performs no Helper authority lookup"),
		Success.ResolutionResult.FinalizationResult.ReadyValidationResult
			.HelperAuthorityResult.bSuccess);
	TestEqual(TEXT("Formal absence has zero Helper placement matches"),
		Success.ResolutionResult.FinalizationResult.ReadyValidationResult
			.HelperAuthorityResult.MatchingPlacementCount,
		0);
	TestTrue(TEXT("Formal absence has no Helper snapshot CardId"),
		Success.ResolutionResult.FinalizationResult.ReadyValidationResult
			.HelperAuthorityResult.SnapshotQueryResult.CardId.IsNone());
	TestAdoptedSuccessEnvelope(
		*this,
		TEXT("ResolveNoLegalHelper"),
		Success.RuntimeEnvelope,
		Before,
		Success.ResolutionResult.AfterState,
		After);

	FMatchPlayAuthoritativeSession LegalSession;
	FReachabilityTrace LegalTrace;
	FName LegalHelper;
	TestTrue(TEXT("No-legal guard fixture reaches legal Helper"),
		BuildStage7163ToAwaitingHelper(
			LegalSession,
			TEXT("HelperNoLegalGuard"),
			true,
			LegalTrace,
			LegalHelper));
	const FMatchPlayState LegalBefore = LegalSession.GetStateSnapshot();
	const auto Rejected = LegalSession.ResolveNoLegalHelper();
	TestEqual(TEXT("Legal Helper blocks NoLegal exactly"),
		Rejected.ResolutionResult.ErrorCode,
		EMatchPlayHelperAbsenceErrorCode::LegalHelperExists);
	TestAcceptedDomainFailureNoAdopt(
		*this,
		TEXT("Legal Helper blocks NoLegal"),
		Rejected.RuntimeEnvelope,
		LegalBefore,
		LegalSession.GetStateSnapshot());

	TArray<FMatchPlayAuthoritativeResolveNoLegalHelperResult> Results;
	TArray<FMatchPlayState> FinalStates;
	for (int32 Index = 0; Index < 3; ++Index)
	{
		FMatchPlayAuthoritativeSession DeterministicSession;
		FReachabilityTrace DeterministicTrace;
		FName DeterministicHelper;
		TestTrue(TEXT("NoLegal Helper determinism fixture reaches stage"),
			BuildStage7163ToAwaitingHelper(
				DeterministicSession,
				TEXT("HelperNoLegalDeterminism"),
				false,
				DeterministicTrace,
				DeterministicHelper));
		Results.Add(DeterministicSession.ResolveNoLegalHelper());
		FinalStates.Add(DeterministicSession.GetStateSnapshot());
	}
	for (int32 Index = 1; Index < 3; ++Index)
	{
		TestTrue(TEXT("ResolveNoLegalHelper full result deterministic"),
			AreAuthoritativeResolveNoLegalHelperResultsEqual(
				Results[0], Results[Index]));
		TestTrue(TEXT("ResolveNoLegalHelper final state deterministic"),
			AreStatesEqual(FinalStates[0], FinalStates[Index]));
	}

	FMatchPlayAuthoritativeSession SessionA;
	FMatchPlayAuthoritativeSession SessionB;
	FReachabilityTrace TraceA;
	FReachabilityTrace TraceB;
	FName HelperA;
	FName HelperB;
	TestTrue(TEXT("NoLegal isolation fixture A reaches stage"),
		BuildStage7163ToAwaitingHelper(
			SessionA, TEXT("HelperNoLegalIsolationA"), false, TraceA, HelperA));
	TestTrue(TEXT("NoLegal isolation fixture B reaches stage"),
		BuildStage7163ToAwaitingHelper(
			SessionB, TEXT("HelperNoLegalIsolationB"), false, TraceB, HelperB));
	const FMatchPlayState BBefore = SessionB.GetStateSnapshot();
	SessionA.ResolveNoLegalHelper();
	TestTrue(TEXT("ResolveNoLegalHelper on A cannot mutate B"),
		AreStatesEqual(BBefore, SessionB.GetStateSnapshot()));
	return true;
}

MATCH_PLAY_AUTHORITATIVE_SESSION_TEST(
	FMatchPlayAuthoritativeSessionDeclineHelperTest,
	"38.DeclineHelperFormalAbsence")

bool FMatchPlayAuthoritativeSessionDeclineHelperTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayAuthoritativeSessionTests;
	FMatchPlayAuthoritativeSession NoLegalSession;
	FReachabilityTrace NoLegalTrace;
	FName NoLegalHelper;
	TestTrue(TEXT("Decline no-legal fixture reaches stage"),
		BuildStage7163ToAwaitingHelper(
			NoLegalSession,
			TEXT("HelperDeclineNoLegal"),
			false,
			NoLegalTrace,
			NoLegalHelper));
	FMatchPlayAuthoritativeDeclineHelperRequest Request;
	Request.RequestingSide = NoLegalTrace.DefendingSide;
	const FMatchPlayState NoLegalBefore = NoLegalSession.GetStateSnapshot();
	const auto NoLegalDecline = NoLegalSession.DeclineHelper(Request);
	TestEqual(TEXT("No legal Helper cannot be declined"),
		NoLegalDecline.DeclineResult.ErrorCode,
		EMatchPlayHelperAbsenceErrorCode::NoLegalHelperToDecline);
	TestAcceptedDomainFailureNoAdopt(
		*this,
		TEXT("No-legal Helper decline"),
		NoLegalDecline.RuntimeEnvelope,
		NoLegalBefore,
		NoLegalSession.GetStateSnapshot());

	const auto NoLegalSuccess = NoLegalSession.ResolveNoLegalHelper();
	TestTrue(TEXT("Distinct NoLegal path remains callable"),
		NoLegalSuccess.ResolutionResult.bSuccess);

	FMatchPlayAuthoritativeSession Session;
	FReachabilityTrace Trace;
	FName HelperCardId;
	TestTrue(TEXT("Decline fixture reaches legal Helper"),
		BuildStage7163ToAwaitingHelper(
			Session,
			TEXT("HelperDecline"),
			true,
			Trace,
			HelperCardId));
	const FMatchPlayState Before = Session.GetStateSnapshot();
	Request.RequestingSide = Trace.AttackingSide;
	const auto WrongSide = Session.DeclineHelper(Request);
	TestEqual(TEXT("Wrong-side decline outer error"),
		WrongSide.DeclineResult.ErrorCode,
		EMatchPlayHelperAbsenceErrorCode::AvailabilityQueryFailed);
	TestEqual(TEXT("Wrong-side decline nested exact error"),
		WrongSide.DeclineResult.HelperAvailabilityResult.ErrorCode,
		EMatchPlayCurrentAttackHelperSelectionErrorCode
			::RequestingSideIsNotCurrentDefender);
	TestAcceptedDomainFailureNoAdopt(
		*this,
		TEXT("Wrong-side Helper decline"),
		WrongSide.RuntimeEnvelope,
		Before,
		Session.GetStateSnapshot());

	Request.RequestingSide = Trace.DefendingSide;
	const auto Success = Session.DeclineHelper(Request);
	const FMatchPlayState After = Session.GetStateSnapshot();
	TestTrue(TEXT("Legal Helper decline succeeds"),
		Success.DeclineResult.bSuccess);
	TestTrue(TEXT("Decline observed legal Helper"),
		Success.DeclineResult.HelperAvailabilityResult.bCanSelectAnyHelper);
	TestEqual(TEXT("Helper decline exact source"),
		Success.DeclineResult.Source,
		EMatchPlayHelperAbsenceSource::HelperDecline);
	TestEqual(TEXT("Helper decline exact reason"),
		Success.DeclineResult.Reason,
		EMatchPlayHelperAbsenceReason::HelperDeclined);
	TestEqual(TEXT("Helper decline sequence derived internally"),
		Success.DeclineResult.Request.AttackSequence,
		Before.CurrentAttack.AttackSequence);
	TestEqual(TEXT("Helper decline reaches ReadyForResolution"),
		After.CurrentAttack.SelectionStage,
		EMatchPlayCurrentAttackSelectionStage::ReadyForResolution);
	TestFalse(TEXT("Helper decline records formal absence"),
		After.CurrentAttack.SelectedAction.bHasHelper);
	TestTrue(TEXT("Helper decline identity is empty"),
		After.CurrentAttack.SelectedAction.HelperCardId.IsNone());
	TestTrue(TEXT("Helper decline does not complete attack"),
		After.bHasCurrentAttack);
	TestAdoptedSuccessEnvelope(
		*this,
		TEXT("DeclineHelper"),
		Success.RuntimeEnvelope,
		Before,
		Success.DeclineResult.AfterState,
		After);
	TestFalse(TEXT("NoLegal and Decline Helper sources remain distinct"),
		NoLegalSuccess.ResolutionResult.Source
			== Success.DeclineResult.Source);
	TestFalse(TEXT("NoLegal and Decline Helper reasons remain distinct"),
		NoLegalSuccess.ResolutionResult.Reason
			== Success.DeclineResult.Reason);

	TArray<FMatchPlayAuthoritativeDeclineHelperResult> Results;
	TArray<FMatchPlayState> FinalStates;
	for (int32 Index = 0; Index < 3; ++Index)
	{
		FMatchPlayAuthoritativeSession DeterministicSession;
		FReachabilityTrace DeterministicTrace;
		FName DeterministicHelper;
		TestTrue(TEXT("Decline Helper determinism fixture reaches stage"),
			BuildStage7163ToAwaitingHelper(
				DeterministicSession,
				TEXT("HelperDeclineDeterminism"),
				true,
				DeterministicTrace,
				DeterministicHelper));
		FMatchPlayAuthoritativeDeclineHelperRequest DeterministicRequest;
		DeterministicRequest.RequestingSide =
			DeterministicTrace.DefendingSide;
		Results.Add(DeterministicSession.DeclineHelper(DeterministicRequest));
		FinalStates.Add(DeterministicSession.GetStateSnapshot());
	}
	for (int32 Index = 1; Index < 3; ++Index)
	{
		TestTrue(TEXT("DeclineHelper full result deterministic"),
			AreAuthoritativeDeclineHelperResultsEqual(Results[0], Results[Index]));
		TestTrue(TEXT("DeclineHelper final state deterministic"),
			AreStatesEqual(FinalStates[0], FinalStates[Index]));
	}
	return true;
}

MATCH_PLAY_AUTHORITATIVE_SESSION_TEST(
	FMatchPlayAuthoritativeSessionBeginResolutionSessionTest,
	"39.BeginResolutionSessionAuthority")

bool FMatchPlayAuthoritativeSessionBeginResolutionSessionTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayAuthoritativeSessionTests;
	static_assert(std::is_same_v<
		decltype(&FMatchPlayAuthoritativeSession::BeginResolutionSession),
		FMatchPlayAuthoritativeBeginResolutionSessionResult
		(FMatchPlayAuthoritativeSession::*)()>);
	static_assert(std::is_same_v<
		decltype(FMatchPlayAuthoritativeBeginResolutionSessionResult
			::BeginResult),
		FMatchPlayCurrentAttackBeginResolutionSessionWriterResult>);

	TestEqual(TEXT("SubmitBranchIntent follows BeginResolutionSession"),
		static_cast<uint8>(
			EMatchPlayAuthoritativeCommandKind::SubmitBranchIntent),
		static_cast<uint8>(
			EMatchPlayAuthoritativeCommandKind::BeginResolutionSession) + 1);

	FString Types;
	TestTrue(TEXT("Resolution Session types source loads"),
		LoadProductionSource(
			TEXT("Source/FMCodex/MatchPlayRuntime/MatchPlayAuthoritativeSessionTypes.h"),
			Types));
	TestFalse(TEXT("No public Resolution Session begin request wrapper"),
		Types.Contains(
			TEXT("FMatchPlayAuthoritativeBeginResolutionSessionRequest")));

	const auto Uninitialized =
		FMatchPlayAuthoritativeSession().BeginResolutionSession();
	TestFalse(TEXT("Uninitialized Resolution Session begin rejected"),
		Uninitialized.RuntimeEnvelope.bAccepted);
	TestEqual(TEXT("Uninitialized Resolution Session begin runtime error"),
		Uninitialized.RuntimeEnvelope.RuntimeFailureCode,
		EMatchPlayAuthoritativeRuntimeFailureCode::NotInitialized);

	FMatchPlayAuthoritativeSession NoAttackSession;
	TestTrue(TEXT("No-attack fixture initializes"),
		NoAttackSession.InitializeMatch(
			MakeFoundationBInput(TEXT("ResolutionBeginNoAttack")))
			.OpeningResult.bSuccess);
	const FMatchPlayState NoAttackBefore =
		NoAttackSession.GetStateSnapshot();
	const auto NoAttack = NoAttackSession.BeginResolutionSession();
	TestEqual(TEXT("No current attack exact error"),
		NoAttack.BeginResult.ErrorCode,
		EMatchPlayCurrentAttackBeginResolutionSessionErrorCode
			::NoCurrentAttack);
	TestAcceptedDomainFailureNoAdopt(
		*this,
		TEXT("No current attack Resolution Session begin"),
		NoAttack.RuntimeEnvelope,
		NoAttackBefore,
		NoAttackSession.GetStateSnapshot());

	FMatchPlayAuthoritativeSession WrongStageSession;
	FReachabilityTrace WrongStageTrace;
	FName WrongStageHelper;
	TestTrue(TEXT("Wrong-stage fixture reaches AwaitingHelper"),
		BuildStage7163ToAwaitingHelper(
			WrongStageSession,
			TEXT("ResolutionBeginWrongStage"),
			true,
			WrongStageTrace,
			WrongStageHelper));
	const FMatchPlayState WrongStageBefore =
		WrongStageSession.GetStateSnapshot();
	const auto WrongStage =
		WrongStageSession.BeginResolutionSession();
	TestEqual(TEXT("Not-Ready exact error"),
		WrongStage.BeginResult.ErrorCode,
		EMatchPlayCurrentAttackBeginResolutionSessionErrorCode
			::WrongSelectionStage);
	TestEqual(TEXT("Not-Ready sequence derived internally"),
		WrongStage.BeginResult.Request.AttackSequence,
		WrongStageBefore.CurrentAttack.AttackSequence);
	TestAcceptedDomainFailureNoAdopt(
		*this,
		TEXT("Not-Ready Resolution Session begin"),
		WrongStage.RuntimeEnvelope,
		WrongStageBefore,
		WrongStageSession.GetStateSnapshot());

	FMatchPlayAuthoritativeSession Session;
	FReachabilityTrace Trace;
	TestTrue(TEXT("Public commands reach ReadyForResolution"),
		BuildStage7164ToReadyForResolution(
			Session,
			TEXT("ResolutionBeginSuccess"),
			Trace));
	const FMatchPlayState Before = Session.GetStateSnapshot();
	const auto Success = Session.BeginResolutionSession();
	const FMatchPlayState After = Session.GetStateSnapshot();
	TestTrue(TEXT("Resolution Session begin succeeds"),
		Success.BeginResult.bSuccess);
	TestTrue(TEXT("Resolution Session is newly created"),
		Success.BeginResult.bCreatedNewSession);
	TestFalse(TEXT("New begin did not find an existing session"),
		Success.BeginResult.LegalityResult.bSessionAlreadyExists);
	TestEqual(TEXT("Resolution Session begin exact command"),
		Success.RuntimeEnvelope.CommandKind,
		EMatchPlayAuthoritativeCommandKind::BeginResolutionSession);
	TestEqual(TEXT("Resolution Session begin derives sequence"),
		Success.BeginResult.Request.AttackSequence,
		Before.CurrentAttack.AttackSequence);
	TestTrue(TEXT("Writer receives exact authoritative BeforeState"),
		AreStatesEqual(Before, Success.BeginResult.BeforeState));
	TestTrue(TEXT("Current attack remains active"),
		After.bHasCurrentAttack);
	TestEqual(TEXT("Selection stage remains ReadyForResolution"),
		After.CurrentAttack.SelectionStage,
		EMatchPlayCurrentAttackSelectionStage::ReadyForResolution);
	TestTrue(TEXT("Resolution Session now exists"),
		After.CurrentAttack.bHasResolutionSession);
	const FMatchPlayCurrentAttackResolutionSession& ResolutionSession =
		After.CurrentAttack.ResolutionSession;
	TestEqual(TEXT("Resolution Session sequence is canonical"),
		ResolutionSession.AttackSequence,
		Before.CurrentAttack.AttackSequence);
	TestEqual(TEXT("Resolution Session awaits route"),
		ResolutionSession.Stage,
		EMatchPlayCurrentAttackResolutionStage::AwaitingRoute);
	TestEqual(TEXT("Bundle binding preserves action type"),
		ResolutionSession.Bundle.Binding.ActionType,
		Before.CurrentAttack.SelectedAction.ActionType);
	TestEqual(TEXT("Bundle binding preserves elective intent"),
		ResolutionSession.Bundle.Binding.ElectiveBranchIntent,
		Before.CurrentAttack.SelectedAction.ElectiveBranchIntent);
	TestEqual(TEXT("Bundle attacker is canonical"),
		ResolutionSession.Bundle.CurrentAttackingPlayer,
		Trace.AttackingSide);
	TestEqual(TEXT("Bundle defender is canonical"),
		ResolutionSession.Bundle.CurrentDefendingPlayer,
		Trace.DefendingSide);
	TestTrue(TEXT("Bundle carrier is present"),
		ResolutionSession.Bundle.Carrier.bIsPresent);
	TestTrue(TEXT("Bundle marker is present"),
		ResolutionSession.Bundle.Marker.bIsPresent);
	TestTrue(TEXT("Bundle runner is present"),
		ResolutionSession.Bundle.bHasRunner
			&& ResolutionSession.Bundle.Runner.bIsPresent);
	TestTrue(TEXT("Bundle Helper is present"),
		ResolutionSession.Bundle.bHasHelper
			&& ResolutionSession.Bundle.Helper.bIsPresent);
	TestTrue(TEXT("Initial Route is unresolved"),
		ResolutionSession.InitialRouteRollRecords.IsEmpty());
	TestFalse(TEXT("Actual Branch is unresolved"),
		ResolutionSession.bHasActualBranch);
	const FMatchPlayCurrentAttackActualBranch DefaultActualBranch;
	TestTrue(TEXT("Actual Branch payload remains default"),
		AreReflectedValuesEqual(
			ResolutionSession.ActualBranch,
			DefaultActualBranch));
	TestAdoptedSuccessEnvelope(
		*this,
		TEXT("BeginResolutionSession"),
		Success.RuntimeEnvelope,
		Before,
		Success.BeginResult.AfterState,
		After);

	const auto Replay = Session.BeginResolutionSession();
	TestTrue(TEXT("Canonical replay is idempotent success"),
		Replay.BeginResult.bSuccess);
	TestTrue(TEXT("Replay detects existing Resolution Session"),
		Replay.BeginResult.LegalityResult.bSessionAlreadyExists);
	TestFalse(TEXT("Replay does not create another Resolution Session"),
		Replay.BeginResult.bCreatedNewSession);
	TestTrue(TEXT("Replay writer AfterState remains exact"),
		AreStatesEqual(After, Replay.BeginResult.AfterState));
	TestAdoptedSuccessEnvelope(
		*this,
		TEXT("Idempotent BeginResolutionSession replay"),
		Replay.RuntimeEnvelope,
		After,
		Replay.BeginResult.AfterState,
		Session.GetStateSnapshot());

	TArray<FMatchPlayAuthoritativeBeginResolutionSessionResult> Results;
	TArray<FMatchPlayState> FinalStates;
	for (int32 Index = 0; Index < 3; ++Index)
	{
		FMatchPlayAuthoritativeSession DeterministicSession;
		FReachabilityTrace DeterministicTrace;
		TestTrue(TEXT("Resolution begin determinism fixture reaches Ready"),
			BuildStage7164ToReadyForResolution(
				DeterministicSession,
				TEXT("ResolutionBeginDeterminism"),
				DeterministicTrace));
		Results.Add(DeterministicSession.BeginResolutionSession());
		FinalStates.Add(DeterministicSession.GetStateSnapshot());
	}
	for (int32 Index = 1; Index < 3; ++Index)
	{
		TestTrue(TEXT("Resolution Session begin result deterministic"),
			AreAuthoritativeBeginResolutionSessionResultsEqual(
				Results[0], Results[Index]));
		TestTrue(TEXT("Resolution Session final state deterministic"),
			AreStatesEqual(FinalStates[0], FinalStates[Index]));
	}

	FMatchPlayAuthoritativeSession SessionA;
	FMatchPlayAuthoritativeSession SessionB;
	FReachabilityTrace TraceA;
	FReachabilityTrace TraceB;
	TestTrue(TEXT("Resolution begin isolation fixture A reaches Ready"),
		BuildStage7164ToReadyForResolution(
			SessionA,
			TEXT("ResolutionBeginIsolationA"),
			TraceA));
	TestTrue(TEXT("Resolution begin isolation fixture B reaches Ready"),
		BuildStage7164ToReadyForResolution(
			SessionB,
			TEXT("ResolutionBeginIsolationB"),
			TraceB));
	const FMatchPlayState SessionBBefore = SessionB.GetStateSnapshot();
	SessionA.BeginResolutionSession();
	TestTrue(TEXT("BeginResolutionSession on A cannot mutate B"),
		AreStatesEqual(SessionBBefore, SessionB.GetStateSnapshot()));
	return true;
}

MATCH_PLAY_AUTHORITATIVE_SESSION_TEST(
	FMatchPlayAuthoritativeSessionSubmitBranchIntentTest,
	"40.SubmitBranchIntentAuthority")

bool FMatchPlayAuthoritativeSessionSubmitBranchIntentTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayAuthoritativeSessionTests;
	static_assert(std::is_same_v<
		decltype(&FMatchPlayAuthoritativeSession::SubmitBranchIntent),
		FMatchPlayAuthoritativeSubmitBranchIntentResult
		(FMatchPlayAuthoritativeSession::*)(
			const FMatchPlayAuthoritativeSubmitBranchIntentRequest&)>);
	static_assert(std::is_same_v<
		decltype(FMatchPlayAuthoritativeSubmitBranchIntentResult
			::IntentResult),
		FMatchPlayCurrentAttackBranchIntentSelectionWriterResult>);

	TestEqual(TEXT("SubmitBranchIntent is final command"),
		static_cast<uint8>(
			EMatchPlayAuthoritativeCommandKind::SubmitBranchIntent),
		static_cast<uint8>(
			EMatchPlayAuthoritativeCommandKind::BeginResolutionSession) + 1);

	FString Types;
	TestTrue(TEXT("Branch Intent types source loads"),
		LoadProductionSource(
			TEXT("Source/FMCodex/MatchPlayRuntime/MatchPlayAuthoritativeSessionTypes.h"),
			Types));
	const FString RequestTypeName =
		TEXT("struct FMCODEX_API FMatchPlayAuthoritativeSubmitBranchIntentRequest");
	const int32 RequestBegin = Types.Find(RequestTypeName);
	const int32 RequestEnd = RequestBegin == INDEX_NONE
		? INDEX_NONE
		: Types.Find(
			TEXT("};"),
			ESearchCase::CaseSensitive,
			ESearchDir::FromStart,
			RequestBegin);
	const FString RequestSurface = RequestBegin == INDEX_NONE
		|| RequestEnd == INDEX_NONE
		? FString()
		: Types.Mid(RequestBegin, RequestEnd - RequestBegin);
	TestTrue(TEXT("Public Branch Intent request exists"),
		RequestBegin != INDEX_NONE && RequestEnd != INDEX_NONE);
	TestFalse(TEXT("Public Branch Intent request has no State"),
		RequestSurface.Contains(TEXT("FMatchPlayState")));
	TestFalse(TEXT("Public Branch Intent request has no AttackSequence"),
		RequestSurface.Contains(TEXT("AttackSequence")));
	TestFalse(TEXT("No Branch Intent Availability abstraction added"),
		Types.Contains(TEXT("BranchIntentAvailability")));

	FMatchPlayAuthoritativeSubmitBranchIntentRequest EmptyRequest;
	const auto Uninitialized =
		FMatchPlayAuthoritativeSession().SubmitBranchIntent(EmptyRequest);
	TestFalse(TEXT("Uninitialized Branch Intent rejected"),
		Uninitialized.RuntimeEnvelope.bAccepted);
	TestEqual(TEXT("Uninitialized Branch Intent runtime error"),
		Uninitialized.RuntimeEnvelope.RuntimeFailureCode,
		EMatchPlayAuthoritativeRuntimeFailureCode::NotInitialized);

	struct FSuccessCase
	{
		const TCHAR* Label;
		ESkillRuleType ActionType;
		EMatchPlayElectiveBranchIntent Intent;
	};
	const FSuccessCase SuccessCases[] = {
		{TEXT("LongShotDirect"), ESkillRuleType::LongShot,
			EMatchPlayElectiveBranchIntent::DirectShot},
		{TEXT("LongShotDeadCorner"), ESkillRuleType::LongShot,
			EMatchPlayElectiveBranchIntent::DeadCorner},
		{TEXT("CutInsideDirect"), ESkillRuleType::CutInsideShot,
			EMatchPlayElectiveBranchIntent::DirectShot},
		{TEXT("CutInsideDeadCorner"), ESkillRuleType::CutInsideShot,
			EMatchPlayElectiveBranchIntent::DeadCorner},
		{TEXT("CrossHigh"), ESkillRuleType::Cross,
			EMatchPlayElectiveBranchIntent::CrossHigh},
		{TEXT("CrossLow"), ESkillRuleType::Cross,
			EMatchPlayElectiveBranchIntent::CrossLow}
	};
	for (const FSuccessCase& Case : SuccessCases)
	{
		FMatchPlayAuthoritativeSession Session;
		FReachabilityTrace Trace;
		TestTrue(
			*FString::Printf(TEXT("%s reaches AwaitingBranchIntent"), Case.Label),
			BuildStage7165ToAwaitingBranchIntent(
				Session,
				FString::Printf(TEXT("BranchIntent%s"), Case.Label),
				Case.ActionType,
				Trace));
		const FMatchPlayState Before = Session.GetStateSnapshot();
		FMatchPlayAuthoritativeSubmitBranchIntentRequest Request;
		Request.RequestingSide = Trace.AttackingSide;
		Request.Intent = Case.Intent;
		const auto Success = Session.SubmitBranchIntent(Request);
		const FMatchPlayState After = Session.GetStateSnapshot();
		TestTrue(*FString::Printf(TEXT("%s intent succeeds"), Case.Label),
			Success.IntentResult.bSuccess);
		TestEqual(*FString::Printf(TEXT("%s action delegated"), Case.Label),
			Success.IntentResult.LegalityResult.ResolvedActionType,
			Case.ActionType);
		TestEqual(*FString::Printf(TEXT("%s intent delegated"), Case.Label),
			Success.IntentResult.LegalityResult.ResolvedIntent,
			Case.Intent);
		TestEqual(*FString::Printf(TEXT("%s sequence derived"), Case.Label),
			Success.IntentResult.Request.AttackSequence,
			Before.CurrentAttack.AttackSequence);
		TestEqual(*FString::Printf(TEXT("%s reaches Ready"), Case.Label),
			After.CurrentAttack.SelectionStage,
			EMatchPlayCurrentAttackSelectionStage::ReadyForResolution);
		TestTrue(*FString::Printf(TEXT("%s attack remains active"), Case.Label),
			After.bHasCurrentAttack);
		TestTrue(*FString::Printf(TEXT("%s selected action exists"), Case.Label),
			After.CurrentAttack.bHasSelectedAction);
		TestEqual(*FString::Printf(TEXT("%s intent persisted"), Case.Label),
			After.CurrentAttack.SelectedAction.ElectiveBranchIntent,
			Case.Intent);
		TestFalse(*FString::Printf(TEXT("%s session not auto-begun"), Case.Label),
			After.CurrentAttack.bHasResolutionSession);
		TestAdoptedSuccessEnvelope(
			*this,
			FString::Printf(TEXT("%s SubmitBranchIntent"), Case.Label),
			Success.RuntimeEnvelope,
			Before,
			Success.IntentResult.AfterState,
			After);

		const auto Begin = Session.BeginResolutionSession();
		const FMatchPlayState BegunState = Session.GetStateSnapshot();
		TestTrue(*FString::Printf(TEXT("%s explicit Begin succeeds"), Case.Label),
			Begin.BeginResult.bSuccess);
		TestTrue(*FString::Printf(TEXT("%s session exists"), Case.Label),
			BegunState.CurrentAttack.bHasResolutionSession);
		TestEqual(*FString::Printf(TEXT("%s session awaits route"), Case.Label),
			BegunState.CurrentAttack.ResolutionSession.Stage,
			EMatchPlayCurrentAttackResolutionStage::AwaitingRoute);
		TestEqual(*FString::Printf(TEXT("%s bundle preserves intent"), Case.Label),
			BegunState.CurrentAttack.ResolutionSession.Bundle.Binding
				.ElectiveBranchIntent,
			Case.Intent);
		TestTrue(*FString::Printf(TEXT("%s no route roll"), Case.Label),
			BegunState.CurrentAttack.ResolutionSession
				.InitialRouteRollRecords.IsEmpty());
		TestFalse(*FString::Printf(TEXT("%s no Actual Branch"), Case.Label),
			BegunState.CurrentAttack.ResolutionSession.bHasActualBranch);
	}

	FMatchPlayAuthoritativeSession FailureSession;
	FReachabilityTrace FailureTrace;
	TestTrue(TEXT("Failure fixture reaches LongShot Branch Intent"),
		BuildStage7165ToAwaitingBranchIntent(
			FailureSession,
			TEXT("BranchIntentFailures"),
			ESkillRuleType::LongShot,
			FailureTrace));
	const FMatchPlayState FailureBefore = FailureSession.GetStateSnapshot();
	FMatchPlayAuthoritativeSubmitBranchIntentRequest FailureRequest;
	FailureRequest.RequestingSide = FailureTrace.DefendingSide;
	FailureRequest.Intent = EMatchPlayElectiveBranchIntent::DirectShot;
	const auto WrongSide = FailureSession.SubmitBranchIntent(FailureRequest);
	TestEqual(TEXT("Wrong-side outer error"),
		WrongSide.IntentResult.LegalityResult.ErrorCode,
		EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
			::GlobalContextFailed);
	TestEqual(TEXT("Wrong-side nested exact error"),
		WrongSide.IntentResult.LegalityResult.GlobalContextResult.ErrorCode,
		EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
			::RequestingSideIsNotCurrentAttacker);
	TestAcceptedDomainFailureNoAdopt(
		*this,
		TEXT("Wrong-side Branch Intent"),
		WrongSide.RuntimeEnvelope,
		FailureBefore,
		FailureSession.GetStateSnapshot());

	FailureRequest.RequestingSide = FailureTrace.AttackingSide;
	FailureRequest.Intent = EMatchPlayElectiveBranchIntent::CrossHigh;
	const auto Mismatch = FailureSession.SubmitBranchIntent(FailureRequest);
	TestEqual(TEXT("Action/intent mismatch exact error"),
		Mismatch.IntentResult.LegalityResult.ErrorCode,
		EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
			::IntentActionTypeMismatch);
	TestAcceptedDomainFailureNoAdopt(
		*this,
		TEXT("Mismatched Branch Intent"),
		Mismatch.RuntimeEnvelope,
		FailureBefore,
		FailureSession.GetStateSnapshot());

	FailureRequest.Intent = EMatchPlayElectiveBranchIntent::None;
	const auto NoneIntent = FailureSession.SubmitBranchIntent(FailureRequest);
	TestEqual(TEXT("None intent exact error"),
		NoneIntent.IntentResult.LegalityResult.ErrorCode,
		EMatchPlayCurrentAttackBranchIntentSelectionErrorCode::InvalidIntent);
	TestAcceptedDomainFailureNoAdopt(
		*this,
		TEXT("None Branch Intent"),
		NoneIntent.RuntimeEnvelope,
		FailureBefore,
		FailureSession.GetStateSnapshot());

	FailureRequest.Intent = EMatchPlayElectiveBranchIntent::DirectShot;
	const auto FirstSuccess =
		FailureSession.SubmitBranchIntent(FailureRequest);
	TestTrue(TEXT("Failure fixture valid intent succeeds"),
		FirstSuccess.IntentResult.bSuccess);
	const FMatchPlayState AfterFirstSuccess =
		FailureSession.GetStateSnapshot();
	const auto Replay = FailureSession.SubmitBranchIntent(FailureRequest);
	TestEqual(TEXT("Replay outer error"),
		Replay.IntentResult.LegalityResult.ErrorCode,
		EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
			::GlobalContextFailed);
	TestEqual(TEXT("Replay nested exact stage error"),
		Replay.IntentResult.LegalityResult.GlobalContextResult.ErrorCode,
		EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
			::WrongSelectionStage);
	TestAcceptedDomainFailureNoAdopt(
		*this,
		TEXT("Branch Intent replay"),
		Replay.RuntimeEnvelope,
		AfterFirstSuccess,
		FailureSession.GetStateSnapshot());

	FMatchPlayAuthoritativeSession PassControlSession;
	FReachabilityTrace PassControlTrace;
	TestTrue(TEXT("PassControl fixture reaches Ready without intent"),
		BuildStage7164ToReadyForResolution(
			PassControlSession,
			TEXT("BranchIntentPassControl"),
			PassControlTrace));
	const FMatchPlayState PassControlBefore =
		PassControlSession.GetStateSnapshot();
	FMatchPlayAuthoritativeSubmitBranchIntentRequest PassControlRequest;
	PassControlRequest.RequestingSide = PassControlTrace.AttackingSide;
	PassControlRequest.Intent =
		EMatchPlayElectiveBranchIntent::DirectShot;
	const auto PassControlRejected =
		PassControlSession.SubmitBranchIntent(PassControlRequest);
	TestEqual(TEXT("PassControl intent attempt exact stage error"),
		PassControlRejected.IntentResult.LegalityResult
			.GlobalContextResult.ErrorCode,
		EMatchPlayCurrentAttackBranchIntentSelectionErrorCode
			::WrongSelectionStage);
	TestAcceptedDomainFailureNoAdopt(
		*this,
		TEXT("PassControl Branch Intent attempt"),
		PassControlRejected.RuntimeEnvelope,
		PassControlBefore,
		PassControlSession.GetStateSnapshot());

	TArray<FMatchPlayAuthoritativeSubmitBranchIntentResult> Results;
	TArray<FMatchPlayState> FinalStates;
	for (int32 Index = 0; Index < 3; ++Index)
	{
		FMatchPlayAuthoritativeSession DeterministicSession;
		FReachabilityTrace DeterministicTrace;
		TestTrue(TEXT("Branch Intent determinism fixture reaches stage"),
			BuildStage7165ToAwaitingBranchIntent(
				DeterministicSession,
				TEXT("BranchIntentDeterminism"),
				ESkillRuleType::LongShot,
				DeterministicTrace));
		FMatchPlayAuthoritativeSubmitBranchIntentRequest Request;
		Request.RequestingSide = DeterministicTrace.AttackingSide;
		Request.Intent = EMatchPlayElectiveBranchIntent::DeadCorner;
		Results.Add(DeterministicSession.SubmitBranchIntent(Request));
		FinalStates.Add(DeterministicSession.GetStateSnapshot());
	}
	for (int32 Index = 1; Index < 3; ++Index)
	{
		TestTrue(TEXT("Branch Intent full result deterministic"),
			AreAuthoritativeSubmitBranchIntentResultsEqual(
				Results[0], Results[Index]));
		TestTrue(TEXT("Branch Intent final state deterministic"),
			AreStatesEqual(FinalStates[0], FinalStates[Index]));
	}

	FMatchPlayAuthoritativeSession SessionA;
	FMatchPlayAuthoritativeSession SessionB;
	FReachabilityTrace TraceA;
	FReachabilityTrace TraceB;
	TestTrue(TEXT("Branch Intent isolation fixture A reaches stage"),
		BuildStage7165ToAwaitingBranchIntent(
			SessionA,
			TEXT("BranchIntentIsolationA"),
			ESkillRuleType::CutInsideShot,
			TraceA));
	TestTrue(TEXT("Branch Intent isolation fixture B reaches stage"),
		BuildStage7165ToAwaitingBranchIntent(
			SessionB,
			TEXT("BranchIntentIsolationB"),
			ESkillRuleType::CutInsideShot,
			TraceB));
	FMatchPlayAuthoritativeSubmitBranchIntentRequest IsolationRequest;
	IsolationRequest.RequestingSide = TraceA.AttackingSide;
	IsolationRequest.Intent = EMatchPlayElectiveBranchIntent::DirectShot;
	const FMatchPlayState SessionBBefore = SessionB.GetStateSnapshot();
	SessionA.SubmitBranchIntent(IsolationRequest);
	TestTrue(TEXT("SubmitBranchIntent on A cannot mutate B"),
		AreStatesEqual(SessionBBefore, SessionB.GetStateSnapshot()));
	return true;
}

MATCH_PLAY_AUTHORITATIVE_SESSION_TEST(
	FMatchPlayAuthoritativeSessionResolveIntentDeterminedRouteTest,
	"41.ResolveIntentDeterminedRouteAuthority")

bool FMatchPlayAuthoritativeSessionResolveIntentDeterminedRouteTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayAuthoritativeSessionTests;
	static_assert(std::is_same_v<
		decltype(&FMatchPlayAuthoritativeSession::ResolveIntentDeterminedRoute),
		FMatchPlayAuthoritativeResolveIntentDeterminedRouteResult
		(FMatchPlayAuthoritativeSession::*)()>);
	static_assert(std::is_same_v<
		decltype(FMatchPlayAuthoritativeResolveIntentDeterminedRouteResult
			::RouteResult),
		FMatchPlayCurrentAttackResolveInitialRouteWriterResult>);

	TestEqual(TEXT("Intent-determined route command appended"),
		static_cast<uint8>(
			EMatchPlayAuthoritativeCommandKind::ResolveIntentDeterminedRoute),
		static_cast<uint8>(
			EMatchPlayAuthoritativeCommandKind::SubmitBranchIntent) + 1);

	FString Header;
	FString Types;
	FString Implementation;
	TestTrue(TEXT("Intent route Session header loads"),
		LoadProductionSource(
			TEXT("Source/FMCodex/MatchPlayRuntime/MatchPlayAuthoritativeSession.h"),
			Header));
	TestTrue(TEXT("Intent route Session types load"),
		LoadProductionSource(
			TEXT("Source/FMCodex/MatchPlayRuntime/MatchPlayAuthoritativeSessionTypes.h"),
			Types));
	TestTrue(TEXT("Intent route Session implementation loads"),
		LoadProductionSource(
			TEXT("Source/FMCodex/MatchPlayRuntime/MatchPlayAuthoritativeSession.cpp"),
			Implementation));
	TestTrue(TEXT("No public intent route request is introduced"),
		!Types.Contains(
			TEXT("FMatchPlayAuthoritativeResolveIntentDeterminedRouteRequest")));
	TestFalse(TEXT("No route mapping is duplicated in Session"),
		Implementation.Contains(TEXT("EMatchPlayLongShotActualBranch"))
			|| Implementation.Contains(TEXT("EMatchPlayCutInsideShotActualBranch")));
	TestEqual(TEXT("One canonical no-provider writer call"),
		CountOccurrences(
			Implementation,
			TEXT("FMatchPlayCurrentAttackResolveInitialRouteWriter::Resolve(")),
		1);

	const auto Uninitialized =
		FMatchPlayAuthoritativeSession().ResolveIntentDeterminedRoute();
	TestFalse(TEXT("Uninitialized intent route rejected"),
		Uninitialized.RuntimeEnvelope.bAccepted);
	TestEqual(TEXT("Uninitialized intent route runtime error"),
		Uninitialized.RuntimeEnvelope.RuntimeFailureCode,
		EMatchPlayAuthoritativeRuntimeFailureCode::NotInitialized);

	struct FSuccessCase
	{
		const TCHAR* Label;
		ESkillRuleType ActionType;
		EMatchPlayElectiveBranchIntent Intent;
	};
	const FSuccessCase SuccessCases[] = {
		{ TEXT("LongShotDirect"), ESkillRuleType::LongShot,
			EMatchPlayElectiveBranchIntent::DirectShot },
		{ TEXT("LongShotDeadCorner"), ESkillRuleType::LongShot,
			EMatchPlayElectiveBranchIntent::DeadCorner },
		{ TEXT("CutInsideDirect"), ESkillRuleType::CutInsideShot,
			EMatchPlayElectiveBranchIntent::DirectShot },
		{ TEXT("CutInsideDeadCorner"), ESkillRuleType::CutInsideShot,
			EMatchPlayElectiveBranchIntent::DeadCorner }
	};
	for (const FSuccessCase& Case : SuccessCases)
	{
		FMatchPlayAuthoritativeSession Session;
		FReachabilityTrace Trace;
		TestTrue(*FString::Printf(TEXT("%s public path reaches AwaitingRoute"), Case.Label),
			BuildStage7166ToAwaitingRoute(
				Session,
				FString::Printf(TEXT("IntentRoute%s"), Case.Label),
				Case.ActionType,
				Case.Intent,
				Trace));
		const FMatchPlayState Before = Session.GetStateSnapshot();
		FMatchPlayCurrentAttackResolveInitialRouteRequest DomainRequest;
		DomainRequest.AttackSequence = Before.CurrentAttack.AttackSequence;
		const auto Canonical =
			FMatchPlayCurrentAttackResolveInitialRouteWriter::Resolve(
				Before,
				DomainRequest,
				nullptr);
		const auto Success = Session.ResolveIntentDeterminedRoute();
		const FMatchPlayState After = Session.GetStateSnapshot();
		TestTrue(*FString::Printf(TEXT("%s canonical writer succeeds without provider"), Case.Label),
			Canonical.bSuccess);
		TestFalse(*FString::Printf(TEXT("%s canonical writer does not call provider"), Case.Label),
			Canonical.bProviderCalled);
		TestTrue(*FString::Printf(TEXT("%s route command succeeds"), Case.Label),
			Success.RouteResult.bSuccess);
		TestTrue(*FString::Printf(TEXT("%s resolves a new route"), Case.Label),
			Success.RouteResult.bResolvedNewRoute);
		TestEqual(*FString::Printf(TEXT("%s command kind"), Case.Label),
			Success.RuntimeEnvelope.CommandKind,
			EMatchPlayAuthoritativeCommandKind::ResolveIntentDeterminedRoute);
		TestEqual(*FString::Printf(TEXT("%s sequence derived"), Case.Label),
			Success.RouteResult.Request.AttackSequence,
			Before.CurrentAttack.AttackSequence);
		TestFalse(*FString::Printf(TEXT("%s has no provider call"), Case.Label),
			Success.RouteResult.bProviderCalled);
		TestFalse(*FString::Printf(TEXT("%s requires no D6"), Case.Label),
			Success.RouteResult.GlobalContextResult.bRequiresInitialRouteD6);
		TestTrue(*FString::Printf(TEXT("%s creates no D6 record"), Case.Label),
			Success.RouteResult.InitialRouteRollRecords.IsEmpty());
		TestTrue(*FString::Printf(TEXT("%s writer receives exact BeforeState"), Case.Label),
			AreStatesEqual(Canonical.BeforeState, Success.RouteResult.BeforeState));
		TestTrue(*FString::Printf(TEXT("%s writer produces exact AfterState"), Case.Label),
			AreStatesEqual(Canonical.AfterState, Success.RouteResult.AfterState));
		TestTrue(*FString::Printf(TEXT("%s delegates canonical Actual Branch"), Case.Label),
			AreReflectedValuesEqual(
				Canonical.ActualBranch,
				Success.RouteResult.ActualBranch));
		TestEqual(*FString::Printf(TEXT("%s delegates canonical mapping action"), Case.Label),
			Success.RouteResult.MappingResult.ActualBranch.ActionType,
			Canonical.MappingResult.ActualBranch.ActionType);
		TestTrue(*FString::Printf(TEXT("%s CurrentAttack remains active"), Case.Label),
			After.bHasCurrentAttack);
		TestEqual(*FString::Printf(TEXT("%s selection remains ready"), Case.Label),
			After.CurrentAttack.SelectionStage,
			EMatchPlayCurrentAttackSelectionStage::ReadyForResolution);
		TestEqual(*FString::Printf(TEXT("%s route stage is canonical"), Case.Label),
			After.CurrentAttack.ResolutionSession.Stage,
			EMatchPlayCurrentAttackResolutionStage::RouteResolved);
		TestTrue(*FString::Printf(TEXT("%s has Actual Branch"), Case.Label),
			After.CurrentAttack.ResolutionSession.bHasActualBranch);
		TestTrue(*FString::Printf(TEXT("%s state has no Initial Route D6 record"), Case.Label),
			After.CurrentAttack.ResolutionSession.InitialRouteRollRecords.IsEmpty());
		TestAdoptedSuccessEnvelope(
			*this,
			FString::Printf(TEXT("%s exact route adoption"), Case.Label),
			Success.RuntimeEnvelope,
			Before,
			Canonical.AfterState,
			After);

		const auto Replay = Session.ResolveIntentDeterminedRoute();
		TestTrue(*FString::Printf(TEXT("%s replay preserves canonical idempotence"), Case.Label),
			Replay.RouteResult.bSuccess);
		TestFalse(*FString::Printf(TEXT("%s replay does not resolve anew"), Case.Label),
			Replay.RouteResult.bResolvedNewRoute);
		TestFalse(*FString::Printf(TEXT("%s replay does not call provider"), Case.Label),
			Replay.RouteResult.bProviderCalled);
		TestAdoptedSuccessEnvelope(
			*this,
			FString::Printf(TEXT("%s replay exact adoption"), Case.Label),
			Replay.RuntimeEnvelope,
			After,
			Replay.RouteResult.AfterState,
			Session.GetStateSnapshot());
	}

	FMatchPlayAuthoritativeSession NoCurrentAttackSession;
	NoCurrentAttackSession.InitializeMatch(MakeValidInput(TEXT("IntentRouteNoAttack")));
	const FMatchPlayState NoCurrentAttackBefore = NoCurrentAttackSession.GetStateSnapshot();
	const auto NoCurrentAttack =
		NoCurrentAttackSession.ResolveIntentDeterminedRoute();
	TestEqual(TEXT("No CurrentAttack has canonical global error"),
		NoCurrentAttack.RouteResult.GlobalContextResult.ErrorCode,
		EMatchPlayCurrentAttackResolveInitialRouteGlobalContextErrorCode
			::NoCurrentAttack);
	TestAcceptedDomainFailureNoAdopt(
		*this,
		TEXT("No CurrentAttack intent route"),
		NoCurrentAttack.RuntimeEnvelope,
		NoCurrentAttackBefore,
		NoCurrentAttackSession.GetStateSnapshot());

	FMatchPlayAuthoritativeSession NoResolutionSession;
	FReachabilityTrace NoResolutionTrace;
	TestTrue(TEXT("No Resolution Session fixture reaches ReadyForResolution"),
		BuildStage7164ToReadyForResolution(
			NoResolutionSession,
			TEXT("IntentRouteNoResolutionSession"),
			NoResolutionTrace));
	const FMatchPlayState NoResolutionBefore =
		NoResolutionSession.GetStateSnapshot();
	const auto NoResolution =
		NoResolutionSession.ResolveIntentDeterminedRoute();
	TestEqual(TEXT("No Resolution Session has canonical global error"),
		NoResolution.RouteResult.GlobalContextResult.ErrorCode,
		EMatchPlayCurrentAttackResolveInitialRouteGlobalContextErrorCode
			::NoResolutionSession);
	TestAcceptedDomainFailureNoAdopt(
		*this,
		TEXT("No Resolution Session intent route"),
		NoResolution.RuntimeEnvelope,
		NoResolutionBefore,
		NoResolutionSession.GetStateSnapshot());

	struct FRngActionCase
	{
		const TCHAR* Label;
		ESkillRuleType ActionType;
		EMatchPlayElectiveBranchIntent Intent;
	};
	const FRngActionCase RngActionCases[] = {
		{ TEXT("Cross"), ESkillRuleType::Cross,
			EMatchPlayElectiveBranchIntent::CrossHigh },
		{ TEXT("PassControl"), ESkillRuleType::PassControl,
			EMatchPlayElectiveBranchIntent::None },
		{ TEXT("ThroughBall"), ESkillRuleType::ThroughBall,
			EMatchPlayElectiveBranchIntent::None }
	};
	for (const FRngActionCase& Case : RngActionCases)
	{
		FMatchPlayAuthoritativeSession Session;
		FReachabilityTrace Trace;
		TestTrue(*FString::Printf(TEXT("%s reaches AwaitingRoute"), Case.Label),
			BuildStage7166ToAwaitingRoute(
				Session,
				FString::Printf(TEXT("IntentRouteReject%s"), Case.Label),
				Case.ActionType,
				Case.Intent,
				Trace));
		const FMatchPlayState Before = Session.GetStateSnapshot();
		const auto Rejected = Session.ResolveIntentDeterminedRoute();
		TestEqual(*FString::Printf(TEXT("%s requires canonical D6 provider"), Case.Label),
			Rejected.RouteResult.ErrorCode,
			EMatchPlayCurrentAttackResolveInitialRouteWriterErrorCode
				::RngProviderUnavailable);
		TestEqual(*FString::Printf(TEXT("%s failure is retryable"), Case.Label),
			Rejected.RouteResult.FailureDisposition,
			EMatchPlayCurrentAttackResolveInitialRouteFailureDisposition
				::RetryableExecutionFailure);
		TestFalse(*FString::Printf(TEXT("%s no provider was called"), Case.Label),
			Rejected.RouteResult.bProviderCalled);
		TestAcceptedDomainFailureNoAdopt(
			*this,
			FString::Printf(TEXT("%s no-provider route rejection"), Case.Label),
			Rejected.RuntimeEnvelope,
			Before,
			Session.GetStateSnapshot());
	}

	TArray<FMatchPlayAuthoritativeResolveIntentDeterminedRouteResult> Results;
	TArray<FMatchPlayState> FinalStates;
	for (int32 Index = 0; Index < 3; ++Index)
	{
		FMatchPlayAuthoritativeSession Session;
		FReachabilityTrace Trace;
		TestTrue(TEXT("Intent route determinism fixture reaches AwaitingRoute"),
			BuildStage7166ToAwaitingRoute(
				Session,
				TEXT("IntentRouteDeterminism"),
				ESkillRuleType::LongShot,
				EMatchPlayElectiveBranchIntent::DeadCorner,
				Trace));
		Results.Add(Session.ResolveIntentDeterminedRoute());
		FinalStates.Add(Session.GetStateSnapshot());
	}
	for (int32 Index = 1; Index < Results.Num(); ++Index)
	{
		TestTrue(TEXT("Intent route envelope deterministic"),
			AreEnvelopesEqual(
				Results[0].RuntimeEnvelope,
				Results[Index].RuntimeEnvelope));
		TestTrue(TEXT("Intent route writer BeforeState deterministic"),
			AreStatesEqual(
				Results[0].RouteResult.BeforeState,
				Results[Index].RouteResult.BeforeState));
		TestTrue(TEXT("Intent route writer AfterState deterministic"),
			AreStatesEqual(
				Results[0].RouteResult.AfterState,
				Results[Index].RouteResult.AfterState));
		TestTrue(TEXT("Intent route Actual Branch deterministic"),
			AreReflectedValuesEqual(
				Results[0].RouteResult.ActualBranch,
				Results[Index].RouteResult.ActualBranch));
		TestTrue(TEXT("Intent route final state deterministic"),
			AreStatesEqual(FinalStates[0], FinalStates[Index]));
	}

	FMatchPlayAuthoritativeSession SessionA;
	FMatchPlayAuthoritativeSession SessionB;
	FReachabilityTrace TraceA;
	FReachabilityTrace TraceB;
	TestTrue(TEXT("Intent route isolation A reaches AwaitingRoute"),
		BuildStage7166ToAwaitingRoute(
			SessionA,
			TEXT("IntentRouteIsolationA"),
			ESkillRuleType::CutInsideShot,
			EMatchPlayElectiveBranchIntent::DirectShot,
			TraceA));
	TestTrue(TEXT("Intent route isolation B reaches AwaitingRoute"),
		BuildStage7166ToAwaitingRoute(
			SessionB,
			TEXT("IntentRouteIsolationB"),
			ESkillRuleType::CutInsideShot,
			EMatchPlayElectiveBranchIntent::DirectShot,
			TraceB));
	const FMatchPlayState SessionBBefore = SessionB.GetStateSnapshot();
	SessionA.ResolveIntentDeterminedRoute();
	TestTrue(TEXT("Intent route resolution on A cannot mutate B"),
		AreStatesEqual(SessionBBefore, SessionB.GetStateSnapshot()));
	return true;
}

MATCH_PLAY_AUTHORITATIVE_SESSION_TEST(
	FMatchPlayAuthoritativeSessionResolveInitialRouteTest,
	"42.ResolveInitialRouteRngAuthority")

bool FMatchPlayAuthoritativeSessionResolveInitialRouteTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayAuthoritativeSessionTests;
	static_assert(std::is_constructible_v<
		FMatchPlayAuthoritativeSession,
		IMatchPlayInitialRouteRollProvider&>);
	static_assert(std::is_same_v<
		decltype(&FMatchPlayAuthoritativeSession::ResolveInitialRoute),
		FMatchPlayAuthoritativeResolveInitialRouteResult
		(FMatchPlayAuthoritativeSession::*)()>);
	static_assert(std::is_same_v<
		decltype(FMatchPlayAuthoritativeResolveInitialRouteResult
			::OrchestrationResult),
		FMatchPlayCurrentAttackResolveInitialRouteOrchestrationResult>);

	TestEqual(TEXT("RNG Initial Route command appended"),
		static_cast<uint8>(EMatchPlayAuthoritativeCommandKind::ResolveInitialRoute),
		static_cast<uint8>(
			EMatchPlayAuthoritativeCommandKind::ResolveIntentDeterminedRoute) + 1);

	FString Header;
	FString Types;
	FString Implementation;
	TestTrue(TEXT("RNG route Session header loads"), LoadProductionSource(
		TEXT("Source/FMCodex/MatchPlayRuntime/MatchPlayAuthoritativeSession.h"),
		Header));
	TestTrue(TEXT("RNG route Session types load"), LoadProductionSource(
		TEXT("Source/FMCodex/MatchPlayRuntime/MatchPlayAuthoritativeSessionTypes.h"),
		Types));
	TestTrue(TEXT("RNG route Session implementation loads"), LoadProductionSource(
		TEXT("Source/FMCodex/MatchPlayRuntime/MatchPlayAuthoritativeSession.cpp"),
		Implementation));
	TestFalse(TEXT("No public RNG route request exists"),
		Types.Contains(TEXT("FMatchPlayAuthoritativeResolveInitialRouteRequest")));
	TestFalse(TEXT("No public D6 gameplay input"),
		Header.Contains(TEXT("ResolveInitialRoute(int32")));
	TestFalse(TEXT("Session contains no D6 mapping table"),
		Implementation.Contains(TEXT("InitialRouteD6 <="))
			|| Implementation.Contains(TEXT("RawD6 <=")));
	TestFalse(TEXT("Session never calls provider directly"),
		Implementation.Contains(TEXT("RollD6(")));
	TestFalse(TEXT("Session contains no direct randomness"),
		Implementation.Contains(TEXT("FMath::Rand")));
	TestEqual(TEXT("One canonical Initial Route orchestration call"),
		CountOccurrences(
			Implementation,
			TEXT("FMatchPlayCurrentAttackResolveInitialRouteOrchestrator::Resolve(")),
		1);

	InitialRouteFixtures::FQueueRollProvider UninitializedProvider;
	UninitializedProvider.Enqueue(InitialRouteFixtures::MakeSuccess(4));
	FMatchPlayAuthoritativeSession UninitializedSession(UninitializedProvider);
	const auto Uninitialized = UninitializedSession.ResolveInitialRoute();
	TestFalse(TEXT("Uninitialized RNG route rejected by runtime gate"),
		Uninitialized.RuntimeEnvelope.bAccepted);
	TestEqual(TEXT("Uninitialized route consumes no provider call"),
		UninitializedProvider.GetCallCount(),
		0);

	struct FSuccessCase
	{
		const TCHAR* Label;
		ESkillRuleType ActionType;
		EMatchPlayElectiveBranchIntent Intent;
		int32 RawD6;
	};
	const FSuccessCase SuccessCases[] = {
		{ TEXT("CrossSelected"), ESkillRuleType::Cross,
			EMatchPlayElectiveBranchIntent::CrossHigh, 4 },
		{ TEXT("CrossOpposite"), ESkillRuleType::Cross,
			EMatchPlayElectiveBranchIntent::CrossHigh, 5 },
		{ TEXT("Pass"), ESkillRuleType::PassControl,
			EMatchPlayElectiveBranchIntent::None, 2 },
		{ TEXT("Dribble"), ESkillRuleType::PassControl,
			EMatchPlayElectiveBranchIntent::None, 3 },
		{ TEXT("Run"), ESkillRuleType::PassControl,
			EMatchPlayElectiveBranchIntent::None, 5 },
		{ TEXT("Feet"), ESkillRuleType::ThroughBall,
			EMatchPlayElectiveBranchIntent::None, 2 },
		{ TEXT("BehindDefense"), ESkillRuleType::ThroughBall,
			EMatchPlayElectiveBranchIntent::None, 3 },
		{ TEXT("AntiOffside"), ESkillRuleType::ThroughBall,
			EMatchPlayElectiveBranchIntent::None, 5 }
	};
	for (const FSuccessCase& Case : SuccessCases)
	{
		InitialRouteFixtures::FQueueRollProvider Provider;
		Provider.Enqueue(InitialRouteFixtures::MakeSuccess(Case.RawD6));
		FMatchPlayAuthoritativeSession Session(Provider);
		FReachabilityTrace Trace;
		TestTrue(*FString::Printf(TEXT("%s public path reaches AwaitingRoute"), Case.Label),
			BuildStage7166ToAwaitingRoute(
				Session,
				FString::Printf(TEXT("RngRoute%s"), Case.Label),
				Case.ActionType,
				Case.Intent,
				Trace));
		const FMatchPlayState Before = Session.GetStateSnapshot();
		const int32 CallsBefore = Provider.GetCallCount();
		const int32 RecordsBefore = Before.CurrentAttack.ResolutionSession
			.InitialRouteRollRecords.Num();

		InitialRouteFixtures::FQueueRollProvider CanonicalProvider;
		CanonicalProvider.Enqueue(InitialRouteFixtures::MakeSuccess(Case.RawD6));
		FMatchPlayCurrentAttackResolveInitialRouteOrchestrationRequest
			CanonicalRequest;
		CanonicalRequest.AttackSequence = Before.CurrentAttack.AttackSequence;
		const auto Canonical =
			FMatchPlayCurrentAttackResolveInitialRouteOrchestrator::Resolve(
				Before,
				CanonicalRequest,
				&CanonicalProvider);

		const auto Success = Session.ResolveInitialRoute();
		const FMatchPlayState After = Session.GetStateSnapshot();
		TestTrue(*FString::Printf(TEXT("%s canonical orchestration succeeds"), Case.Label),
			Canonical.bSuccess);
		TestTrue(*FString::Printf(TEXT("%s Session route succeeds"), Case.Label),
			Success.OrchestrationResult.bSuccess);
		TestEqual(*FString::Printf(TEXT("%s command kind"), Case.Label),
			Success.RuntimeEnvelope.CommandKind,
			EMatchPlayAuthoritativeCommandKind::ResolveInitialRoute);
		TestEqual(*FString::Printf(TEXT("%s derives AttackSequence"), Case.Label),
			Success.OrchestrationResult.Request.AttackSequence,
			Before.CurrentAttack.AttackSequence);
		TestEqual(*FString::Printf(TEXT("%s consumes exactly one provider call"), Case.Label),
			Provider.GetCallCount() - CallsBefore,
			1);
		TestTrue(*FString::Printf(TEXT("%s provider called by canonical writer"), Case.Label),
			Success.OrchestrationResult.bProviderCalled
				&& Success.OrchestrationResult.RouteResult.bProviderCalled);
		TestTrue(*FString::Printf(TEXT("%s exact orchestration result delegated"), Case.Label),
			InitialRouteOrchestrationFixtures::AreResultsEqual(
				Canonical,
				Success.OrchestrationResult));
		TestAdoptedSuccessEnvelope(
			*this,
			FString::Printf(TEXT("%s exact orchestration adoption"), Case.Label),
			Success.RuntimeEnvelope,
			Before,
			Canonical.AfterState,
			After);

		const auto& ResolutionSession = After.CurrentAttack.ResolutionSession;
		TestEqual(*FString::Printf(TEXT("%s adds exactly one roll record"), Case.Label),
			ResolutionSession.InitialRouteRollRecords.Num() - RecordsBefore,
			1);
		TestEqual(*FString::Printf(TEXT("%s records provider D6"), Case.Label),
			ResolutionSession.InitialRouteRollRecords[0].RawD6,
			Case.RawD6);
		TestEqual(*FString::Printf(TEXT("%s records InitialRoute purpose"), Case.Label),
			ResolutionSession.InitialRouteRollRecords[0].Purpose,
			EMatchPlayCurrentAttackResolutionRollPurpose::InitialRoute);
		TestEqual(*FString::Printf(TEXT("%s reaches RouteResolved"), Case.Label),
			ResolutionSession.Stage,
			EMatchPlayCurrentAttackResolutionStage::RouteResolved);
		TestTrue(*FString::Printf(TEXT("%s establishes Actual Branch"), Case.Label),
			ResolutionSession.bHasActualBranch);
		const FMatchPlayCurrentAttackActualBranch ExpectedBranch =
			InitialRouteOrchestrationFixtures::MakeExpectedBranch(
				Case.ActionType,
				Case.Intent,
				Case.RawD6);
		TestTrue(*FString::Printf(TEXT("%s canonical branch mapping preserved"), Case.Label),
			InitialRouteFixtures::AreBranchesEqual(
				ResolutionSession.ActualBranch,
				ExpectedBranch));
		TestTrue(*FString::Printf(TEXT("%s CurrentAttack remains active"), Case.Label),
			After.bHasCurrentAttack);

		const int32 ReplayCallsBefore = Provider.GetCallCount();
		const int32 ReplayRecordsBefore = ResolutionSession
			.InitialRouteRollRecords.Num();
		const auto Replay = Session.ResolveInitialRoute();
		const FMatchPlayState ReplayAfter = Session.GetStateSnapshot();
		TestTrue(*FString::Printf(TEXT("%s replay is canonical success"), Case.Label),
			Replay.OrchestrationResult.bSuccess);
		TestFalse(*FString::Printf(TEXT("%s replay resolves no new route"), Case.Label),
			Replay.OrchestrationResult.bResolvedNewRoute);
		TestEqual(*FString::Printf(TEXT("%s replay consumes zero provider calls"), Case.Label),
			Provider.GetCallCount() - ReplayCallsBefore,
			0);
		TestEqual(*FString::Printf(TEXT("%s replay adds zero roll records"), Case.Label),
			ReplayAfter.CurrentAttack.ResolutionSession.InitialRouteRollRecords.Num()
				- ReplayRecordsBefore,
			0);
		TestAdoptedSuccessEnvelope(
			*this,
			FString::Printf(TEXT("%s replay exact adoption"), Case.Label),
			Replay.RuntimeEnvelope,
			After,
			Replay.OrchestrationResult.AfterState,
			ReplayAfter);
	}

	InitialRouteFixtures::FQueueRollProvider NoAttackProvider;
	NoAttackProvider.Enqueue(InitialRouteFixtures::MakeSuccess(3));
	FMatchPlayAuthoritativeSession NoAttackSession(NoAttackProvider);
	NoAttackSession.InitializeMatch(MakeValidInput(TEXT("RngRouteNoAttack")));
	const FMatchPlayState NoAttackBefore = NoAttackSession.GetStateSnapshot();
	const auto NoAttack = NoAttackSession.ResolveInitialRoute();
	TestEqual(TEXT("No CurrentAttack consumes zero provider calls"),
		NoAttackProvider.GetCallCount(),
		0);
	TestAcceptedDomainFailureNoAdopt(
		*this,
		TEXT("No CurrentAttack RNG route"),
		NoAttack.RuntimeEnvelope,
		NoAttackBefore,
		NoAttackSession.GetStateSnapshot());

	FMatchPlayAuthoritativeSession UnavailableSession;
	FReachabilityTrace UnavailableTrace;
	TestTrue(TEXT("Unavailable provider fixture reaches AwaitingRoute"),
		BuildStage7166ToAwaitingRoute(
			UnavailableSession,
			TEXT("RngRouteUnavailable"),
			ESkillRuleType::Cross,
			EMatchPlayElectiveBranchIntent::CrossHigh,
			UnavailableTrace));
	const FMatchPlayState UnavailableBefore =
		UnavailableSession.GetStateSnapshot();
	const auto Unavailable = UnavailableSession.ResolveInitialRoute();
	TestEqual(TEXT("Unavailable provider canonical error"),
		Unavailable.OrchestrationResult.RouteResult.ErrorCode,
		EMatchPlayCurrentAttackResolveInitialRouteWriterErrorCode
			::RngProviderUnavailable);
	TestFalse(TEXT("Unavailable provider was not called"),
		Unavailable.OrchestrationResult.bProviderCalled);
	TestAcceptedDomainFailureNoAdopt(
		*this,
		TEXT("Unavailable provider RNG route"),
		Unavailable.RuntimeEnvelope,
		UnavailableBefore,
		UnavailableSession.GetStateSnapshot());

	InitialRouteFixtures::FQueueRollProvider FailureProvider;
	FailureProvider.Enqueue(InitialRouteFixtures::MakeFailure());
	FMatchPlayAuthoritativeSession FailureSession(FailureProvider);
	FReachabilityTrace FailureTrace;
	TestTrue(TEXT("Provider failure fixture reaches AwaitingRoute"),
		BuildStage7166ToAwaitingRoute(
			FailureSession,
			TEXT("RngRouteProviderFailure"),
			ESkillRuleType::PassControl,
			EMatchPlayElectiveBranchIntent::None,
			FailureTrace));
	const FMatchPlayState FailureBefore = FailureSession.GetStateSnapshot();
	const auto ProviderFailure = FailureSession.ResolveInitialRoute();
	TestEqual(TEXT("Provider failure consumes exactly one call"),
		FailureProvider.GetCallCount(),
		1);
	TestEqual(TEXT("Provider failure canonical error"),
		ProviderFailure.OrchestrationResult.RouteResult.ErrorCode,
		EMatchPlayCurrentAttackResolveInitialRouteWriterErrorCode
			::RngProviderFailed);
	TestTrue(TEXT("Provider failure reports provider called"),
		ProviderFailure.OrchestrationResult.bProviderCalled);
	TestTrue(TEXT("Provider failure creates no roll record"),
		ProviderFailure.OrchestrationResult.RouteResult
			.InitialRouteRollRecords.IsEmpty());
	TestAcceptedDomainFailureNoAdopt(
		*this,
		TEXT("Provider failure RNG route"),
		ProviderFailure.RuntimeEnvelope,
		FailureBefore,
		FailureSession.GetStateSnapshot());

	InitialRouteFixtures::FQueueRollProvider ShotProvider;
	ShotProvider.Enqueue(InitialRouteFixtures::MakeSuccess(6));
	FMatchPlayAuthoritativeSession ShotSession(ShotProvider);
	FReachabilityTrace ShotTrace;
	TestTrue(TEXT("LongShot fixture reaches AwaitingRoute"),
		BuildStage7166ToAwaitingRoute(
			ShotSession,
			TEXT("RngRouteLongShot"),
			ESkillRuleType::LongShot,
			EMatchPlayElectiveBranchIntent::DirectShot,
			ShotTrace));
	const auto ShotResult = ShotSession.ResolveInitialRoute();
	TestTrue(TEXT("LongShot remains canonically resolvable"),
		ShotResult.OrchestrationResult.bSuccess);
	TestEqual(TEXT("LongShot consumes zero provider calls"),
		ShotProvider.GetCallCount(),
		0);
	TestTrue(TEXT("LongShot creates no Initial Route roll"),
		ShotSession.GetStateSnapshot().CurrentAttack.ResolutionSession
			.InitialRouteRollRecords.IsEmpty());

	InitialRouteFixtures::FQueueRollProvider CutInsideProvider;
	CutInsideProvider.Enqueue(InitialRouteFixtures::MakeSuccess(1));
	FMatchPlayAuthoritativeSession CutInsideSession(CutInsideProvider);
	FReachabilityTrace CutInsideTrace;
	TestTrue(TEXT("CutInsideShot fixture reaches AwaitingRoute"),
		BuildStage7166ToAwaitingRoute(
			CutInsideSession,
			TEXT("RngRouteCutInsideShot"),
			ESkillRuleType::CutInsideShot,
			EMatchPlayElectiveBranchIntent::DeadCorner,
			CutInsideTrace));
	const auto CutInsideResult = CutInsideSession.ResolveInitialRoute();
	TestTrue(TEXT("CutInsideShot remains canonically resolvable"),
		CutInsideResult.OrchestrationResult.bSuccess);
	TestEqual(TEXT("CutInsideShot consumes zero provider calls"),
		CutInsideProvider.GetCallCount(),
		0);
	TestTrue(TEXT("CutInsideShot creates no Initial Route roll"),
		CutInsideSession.GetStateSnapshot().CurrentAttack.ResolutionSession
			.InitialRouteRollRecords.IsEmpty());

	InitialRouteFixtures::FQueueRollProvider IsolationProviderA;
	InitialRouteFixtures::FQueueRollProvider IsolationProviderB;
	IsolationProviderA.Enqueue(InitialRouteFixtures::MakeSuccess(4));
	IsolationProviderB.Enqueue(InitialRouteFixtures::MakeSuccess(5));
	FMatchPlayAuthoritativeSession IsolationA(IsolationProviderA);
	FMatchPlayAuthoritativeSession IsolationB(IsolationProviderB);
	FReachabilityTrace IsolationTraceA;
	FReachabilityTrace IsolationTraceB;
	TestTrue(TEXT("RNG isolation A reaches AwaitingRoute"),
		BuildStage7166ToAwaitingRoute(
			IsolationA, TEXT("RngIsolationA"), ESkillRuleType::Cross,
			EMatchPlayElectiveBranchIntent::CrossHigh, IsolationTraceA));
	TestTrue(TEXT("RNG isolation B reaches AwaitingRoute"),
		BuildStage7166ToAwaitingRoute(
			IsolationB, TEXT("RngIsolationB"), ESkillRuleType::Cross,
			EMatchPlayElectiveBranchIntent::CrossHigh, IsolationTraceB));
	const FMatchPlayState IsolationBBefore = IsolationB.GetStateSnapshot();
	IsolationA.ResolveInitialRoute();
	TestEqual(TEXT("Session A consumes only provider A"),
		IsolationProviderA.GetCallCount(),
		1);
	TestEqual(TEXT("Session A does not consume provider B"),
		IsolationProviderB.GetCallCount(),
		0);
	TestTrue(TEXT("Session A does not mutate Session B"),
		AreStatesEqual(IsolationBBefore, IsolationB.GetStateSnapshot()));

	InitialRouteFixtures::FQueueRollProvider DeterministicProviderA;
	InitialRouteFixtures::FQueueRollProvider DeterministicProviderB;
	DeterministicProviderA.Enqueue(InitialRouteFixtures::MakeSuccess(3));
	DeterministicProviderB.Enqueue(InitialRouteFixtures::MakeSuccess(3));
	FMatchPlayAuthoritativeSession DeterministicA(DeterministicProviderA);
	FMatchPlayAuthoritativeSession DeterministicB(DeterministicProviderB);
	FReachabilityTrace DeterministicTraceA;
	FReachabilityTrace DeterministicTraceB;
	TestTrue(TEXT("RNG determinism A reaches AwaitingRoute"),
		BuildStage7166ToAwaitingRoute(
			DeterministicA, TEXT("RngDeterminism"),
			ESkillRuleType::ThroughBall,
			EMatchPlayElectiveBranchIntent::None,
			DeterministicTraceA));
	TestTrue(TEXT("RNG determinism B reaches AwaitingRoute"),
		BuildStage7166ToAwaitingRoute(
			DeterministicB, TEXT("RngDeterminism"),
			ESkillRuleType::ThroughBall,
			EMatchPlayElectiveBranchIntent::None,
			DeterministicTraceB));
	const auto DeterministicResultA = DeterministicA.ResolveInitialRoute();
	const auto DeterministicResultB = DeterministicB.ResolveInitialRoute();
	TestTrue(TEXT("Identical provider rolls produce equal orchestration"),
		InitialRouteOrchestrationFixtures::AreResultsEqual(
			DeterministicResultA.OrchestrationResult,
			DeterministicResultB.OrchestrationResult));
	TestTrue(TEXT("Identical provider rolls produce equal final State"),
		AreStatesEqual(
			DeterministicA.GetStateSnapshot(),
			DeterministicB.GetStateSnapshot()));
	TestEqual(TEXT("Deterministic providers each called once"),
		DeterministicProviderA.GetCallCount()
			+ DeterministicProviderB.GetCallCount(),
		2);
	return true;
}

#undef MATCH_PLAY_AUTHORITATIVE_SESSION_TEST

#endif
