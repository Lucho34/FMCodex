#include "FMCodexLocalMatchUMGPresentation.h"

#include "FMCodexPlayerUIPresentationText.h"
#include "FMCodexTacticalDetailPresentation.h"
#include "FMCodexTacticalResolutionNarrativePresentation.h"

#include "FMCodexLocalMatchInteractionView.h"
#include "FMCodexLocalMatchResolutionFeedback.h"

namespace FMCodexLocalMatchUMGPresentation
{
	FFMCodexUMGSkillViewModel MakeSkill(
		const FFMCodexLocalMatchCardView::FSkill& Skill)
	{
		FFMCodexUMGSkillViewModel Result;
		Result.SkillId = Skill.SkillId;
		Result.CanonicalLabel = Skill.CanonicalLabel;
		Result.MinTriggerActionPoint = Skill.MinTriggerActionPoint;
		Result.MaxTriggerActionPoint = Skill.MaxTriggerActionPoint;
		return Result;
	}

	FFMCodexUMGCardViewModel MakeCard(
		const FFMCodexLocalMatchCardView& Card)
	{
		FFMCodexUMGCardViewModel Result;
		Result.CardId = Card.CardId;
		// Keep the shared DTO contract stable for Pitch Mini and legacy callers.
		// Full Card performs its player-facing identity cleanup in its own
		// presentation path so this production pass cannot change other modes.
		Result.IdentityLabel = Card.DisplayLabel.IsEmpty()
			? TEXT("UNKNOWN CARD") : Card.DisplayLabel;
		Result.EnglishIdentityLabel = Card.EnglishDisplayLabel;
		Result.NationalityLabel = Card.NationalityLabel;
		Result.ClubLabel = Card.ClubLabel;
		Result.OwnerLabel =
			FFMCodexLocalMatchInteractionViewBuilder::ToString(Card.Side);
		Result.RoleLabel = Card.CompactRoleLabel.IsEmpty()
			? TEXT("ROLE N/A") : Card.CompactRoleLabel;
		Result.OverallRating = Card.OverallRating;
		Result.bHasOverallRating = Card.bHasOverallRating;
		Result.BirthDate = Card.BirthDate;
		Result.HeightCm = Card.HeightCm;
		Result.WeightKg = Card.WeightKg;
		for (const FFMCodexLocalMatchCardView::FAttribute& Attribute
			: Card.AttributeValues)
		{
			FFMCodexUMGAttributeViewModel& AttributeView =
				Result.AttributeValues.AddDefaulted_GetRef();
			AttributeView.CanonicalLabel = Attribute.CanonicalLabel;
			AttributeView.Value = Attribute.Value;
		}
		for (const FFMCodexLocalMatchCardView::FSkill& Skill : Card.Skills)
		{
			Result.Skills.Add(MakeSkill(Skill));
		}
		for (const FFMCodexLocalMatchCardView::FSkill& Skill
			: Card.EligibleTacticalSkills)
		{
			Result.EligibleTacticalSkills.Add(MakeSkill(Skill));
		}
		for (const FFMCodexLocalMatchCardView::FSkill& Skill
			: Card.HandMicroVisibleTacticalSkills)
		{
			Result.HandMicroVisibleTacticalSkills.Add(MakeSkill(Skill));
		}
		Result.HandMicroTacticalMatchCount =
			Card.HandMicroTacticalMatchCount;
		ensureAlwaysMsgf(Result.HandMicroTacticalMatchCount >= 0
				&& Result.HandMicroTacticalMatchCount <= 2,
			TEXT("Invalid Hand Micro tactical-match count for %s: %d"),
			*Card.CardId.ToString(), Result.HandMicroTacticalMatchCount);
		Result.bHasHandMicroTacticalMatch =
			Card.bHasHandMicroTacticalMatch;
		for (const FFMCodexLocalMatchCardView::FSkill& Skill
			: Card.PitchMiniVisibleTacticalSkills)
		{
			Result.PitchMiniVisibleTacticalSkills.Add(MakeSkill(Skill));
		}
		Result.PitchMiniTacticalMatchCount =
			Card.PitchMiniTacticalMatchCount;
		ensureAlwaysMsgf(Result.PitchMiniTacticalMatchCount >= 0
				&& Result.PitchMiniTacticalMatchCount <= 2,
			TEXT("Invalid Pitch Mini tactical-match count for %s: %d"),
			*Card.CardId.ToString(), Result.PitchMiniTacticalMatchCount);
		Result.bHasPitchMiniTacticalMatch =
			Card.bHasPitchMiniTacticalMatch;
		Result.PlayerFacingSerialLabel = Card.PlayerFacingSerialLabel;
		Result.SkillLabels = Card.SkillLabels;
		Result.SkillSummaryLabel = Card.SkillSummaryLabel.IsEmpty()
			? TEXT("NO SKILL") : Card.SkillSummaryLabel;
		Result.CompactAttributeSummary = Card.CompactAttributeSummary.IsEmpty()
			? TEXT("Attributes unavailable") : Card.CompactAttributeSummary;
		Result.FullAttributeSummary = Card.bGoalkeeper
			? Card.GoalkeeperAttributeSummary : Card.AttributeSummary;
		if (Result.FullAttributeSummary.IsEmpty())
		{
			Result.FullAttributeSummary = TEXT("Attributes unavailable");
		}
		Result.StatusLabels = Card.StatusLabels;
		Result.StatusSummaryLabel = Card.StatusSummaryLabel.IsEmpty()
			? TEXT("UNAVAILABLE") : Card.StatusSummaryLabel;
		Result.RarityLabel = Card.RarityLabel.IsEmpty()
			? TEXT("RARITY N/A") : Card.RarityLabel;
		Result.DeveloperReferenceLabel = Card.DeveloperReferenceLabel;
		Result.bGoalkeeper = Card.bGoalkeeper;
		Result.bAvailable = Card.bAvailable;
		Result.bUsed = Card.bUsed;
		Result.bDeployed = Card.bDeployed;
		return Result;
	}

	void ResolvePitchMiniOwnershipAccent(
		FFMCodexUMGCardViewModel& Card,
		const EInitialTurnOrderPlayer CardSide,
		const EInitialTurnOrderPlayer LocalViewerSide,
		const FFMCodexUMGSidePrimaryColors& SidePrimaryColors)
	{
		if (CardSide != EInitialTurnOrderPlayer::PlayerA
			&& CardSide != EInitialTurnOrderPlayer::PlayerB)
		{
			return;
		}

		Card.bHasPitchMiniOwnershipAccent = true;
		Card.PitchMiniOwnershipAccentColor =
			CardSide == EInitialTurnOrderPlayer::PlayerA
				? SidePrimaryColors.PlayerAPrimaryColor
				: SidePrimaryColors.PlayerBPrimaryColor;
		Card.PitchMiniOwnershipAccentEdge = CardSide == LocalViewerSide
			? EFMCodexUMGPitchMiniOwnershipEdge::Left
			: EFMCodexUMGPitchMiniOwnershipEdge::Right;
	}

	void ResolveSelectedRole(
		FFMCodexUMGCardViewModel& Card,
		const FFMCodexLocalMatchInteractionView& InteractionView)
	{
		struct FRoleMatch
		{
			FName CardId;
			EFMCodexUMGSelectedRole Role;
			const TCHAR* CanonicalLabel;
		};
		const FRoleMatch RoleMatches[] = {
			{ InteractionView.SelectedCarrierCardId,
				EFMCodexUMGSelectedRole::Carrier, TEXT("Carrier") },
			{ InteractionView.SelectedRunnerCardId,
				EFMCodexUMGSelectedRole::Runner, TEXT("Runner") },
			{ InteractionView.SelectedMarkerCardId,
				EFMCodexUMGSelectedRole::Marker, TEXT("Marker") },
			{ InteractionView.SelectedHelperCardId,
				EFMCodexUMGSelectedRole::Helper, TEXT("Helper") }
		};
		int32 MatchingRoleCount = 0;
		for (const FRoleMatch& Match : RoleMatches)
		{
			if (!Match.CardId.IsNone() && Match.CardId == Card.CardId)
			{
				++MatchingRoleCount;
				if (Card.SelectedRole == EFMCodexUMGSelectedRole::None)
				{
					Card.SelectedRole = Match.Role;
					Card.SelectedRoleLabel =
						FFMCodexPlayerUIPresentationText::SelectedRoleTag(
							Match.CanonicalLabel).ToString();
				}
			}
		}
		ensureAlwaysMsgf(MatchingRoleCount <= 1,
			TEXT("Pitch Mini %s matched %d selected attack roles; one is allowed"),
			*Card.CardId.ToString(), MatchingRoleCount);
	}

	EFMCodexUMGSelectionFeedbackReason SelectionFeedbackReason(
		const EFMCodexLocalMatchSelectionFeedbackReason Reason)
	{
		switch (Reason)
		{
		case EFMCodexLocalMatchSelectionFeedbackReason::MarkerWrongPhysicalArea:
			return EFMCodexUMGSelectionFeedbackReason::MarkerWrongPhysicalArea;
		case EFMCodexLocalMatchSelectionFeedbackReason::RunnerIsGoalkeeper:
			return EFMCodexUMGSelectionFeedbackReason::RunnerIsGoalkeeper;
		case EFMCodexLocalMatchSelectionFeedbackReason::RunnerMatchesCarrier:
			return EFMCodexUMGSelectionFeedbackReason::RunnerMatchesCarrier;
		case EFMCodexLocalMatchSelectionFeedbackReason::
			RunnerMissingRequiredPositionType:
			return EFMCodexUMGSelectionFeedbackReason::
				RunnerMissingRequiredPositionType;
		case EFMCodexLocalMatchSelectionFeedbackReason::
			RunnerNotInAttackingForwardArea:
			return EFMCodexUMGSelectionFeedbackReason::
				RunnerNotInAttackingForwardArea;
		case EFMCodexLocalMatchSelectionFeedbackReason::HelperIsGoalkeeper:
			return EFMCodexUMGSelectionFeedbackReason::HelperIsGoalkeeper;
		case EFMCodexLocalMatchSelectionFeedbackReason::HelperMatchesMarker:
			return EFMCodexUMGSelectionFeedbackReason::HelperMatchesMarker;
		case EFMCodexLocalMatchSelectionFeedbackReason::HelperWrongPhysicalArea:
			return EFMCodexUMGSelectionFeedbackReason::HelperWrongPhysicalArea;
		default:
			return EFMCodexUMGSelectionFeedbackReason::None;
		}
	}

	FString SelectionFeedbackReasonKey(
		const EFMCodexUMGSelectionFeedbackReason Reason)
	{
		switch (Reason)
		{
		case EFMCodexUMGSelectionFeedbackReason::MarkerWrongPhysicalArea:
			return TEXT("MarkerWrongPhysicalArea");
		case EFMCodexUMGSelectionFeedbackReason::RunnerIsGoalkeeper:
			return TEXT("RunnerIsGoalkeeper");
		case EFMCodexUMGSelectionFeedbackReason::RunnerMatchesCarrier:
			return TEXT("RunnerMatchesCarrier");
		case EFMCodexUMGSelectionFeedbackReason::RunnerMissingRequiredPositionType:
			return TEXT("RunnerMissingRequiredPositionType");
		case EFMCodexUMGSelectionFeedbackReason::RunnerNotInAttackingForwardArea:
			return TEXT("RunnerNotInAttackingForwardArea");
		case EFMCodexUMGSelectionFeedbackReason::HelperIsGoalkeeper:
			return TEXT("HelperIsGoalkeeper");
		case EFMCodexUMGSelectionFeedbackReason::HelperMatchesMarker:
			return TEXT("HelperMatchesMarker");
		case EFMCodexUMGSelectionFeedbackReason::HelperWrongPhysicalArea:
			return TEXT("HelperWrongPhysicalArea");
		default:
			return FString();
		}
	}

	EInitialTurnOrderPlayer OtherSide(const EInitialTurnOrderPlayer Side)
	{
		return Side == EInitialTurnOrderPlayer::PlayerB
			? EInitialTurnOrderPlayer::PlayerA
			: EInitialTurnOrderPlayer::PlayerB;
	}

	FLinearColor PrimaryColorForSide(
		const EInitialTurnOrderPlayer Side,
		const FFMCodexUMGSidePrimaryColors& Colors)
	{
		return Side == EInitialTurnOrderPlayer::PlayerA
			? Colors.PlayerAPrimaryColor : Colors.PlayerBPrimaryColor;
	}

	FFMCodexUMGAttackTurnTrackerViewModel MakeAttackTurnTracker(
		const int32 MaxAttackTurns,
		const int32 UsedAttackTurns,
		const int32 CurrentAttackIndex,
		const bool bCurrentAttackingSide,
		const FLinearColor& PrimarySideColor)
	{
		FFMCodexUMGAttackTurnTrackerViewModel Result;
		Result.MaxAttackTurns = MaxAttackTurns;
		Result.UsedAttackTurns = UsedAttackTurns;
		Result.CurrentAttackIndex = CurrentAttackIndex;
		Result.bCurrentAttackingSide = bCurrentAttackingSide;
		Result.PrimarySideColor = PrimarySideColor;
		for (int32 AttackIndex = 1;
			AttackIndex <= MaxAttackTurns; ++AttackIndex)
		{
			FFMCodexUMGAttackTurnStepViewModel& Step =
				Result.Steps.AddDefaulted_GetRef();
			Step.AttackIndex = AttackIndex;
			Step.State = AttackIndex <= UsedAttackTurns
				? EFMCodexUMGAttackTurnStepState::Used
				: bCurrentAttackingSide
					&& AttackIndex == CurrentAttackIndex
						? EFMCodexUMGAttackTurnStepState::Current
						: EFMCodexUMGAttackTurnStepState::Remaining;
		}
		return Result;
	}

	EMatchPlayNeutralSlotSide PhysicalSide(
		const EInitialTurnOrderPlayer Side)
	{
		return Side == EInitialTurnOrderPlayer::PlayerB
			? EMatchPlayNeutralSlotSide::NearPlayerB
			: EMatchPlayNeutralSlotSide::NearPlayerA;
	}

	void BuildRack(
		const TArray<FFMCodexLocalMatchCardView>& Roster,
		const TArray<FFMCodexLocalMatchDeploymentGroup>& DeploymentGroups,
		const FFMCodexLocalMatchInteractionView& InteractionView,
		const EInitialTurnOrderPlayer RackSide,
		const bool bLocalRack,
		const FString& SideLabel,
		FFMCodexUMGCardRackViewModel& Rack)
	{
		Rack.bLocalRack = bLocalRack;
		Rack.SideLabel = SideLabel;
		TArray<const FFMCodexLocalMatchCardView*> Sorted;
		Sorted.Reserve(Roster.Num());
		for (const FFMCodexLocalMatchCardView& Card : Roster)
		{
			Sorted.Add(&Card);
		}
		Sorted.StableSort(
			[](const FFMCodexLocalMatchCardView& Left,
				const FFMCodexLocalMatchCardView& Right)
			{
				return Left.RackSortGroup < Right.RackSortGroup;
			});
		for (int32 Index = 0; Index < Sorted.Num(); ++Index)
		{
			const FFMCodexLocalMatchCardView& Card = *Sorted[Index];
			FFMCodexUMGCardRackCellViewModel Cell;
			Cell.StableIndex = Index;
			Cell.bPlayed = Card.bUsed || Card.bDeployed || Card.bEjected;
			Cell.bGoalkeeper = Card.bGoalkeeper;
			Cell.bSetPieceSelectable =
				RackSide == InteractionView.ExpectedActingPlayer
				&& InteractionView.LegalSetPieceCardIds.Contains(Card.CardId);
			Cell.bSetPieceSelected =
				InteractionView.DraftSetPieceCarrierCardId == Card.CardId
				|| InteractionView.DraftCornerNomineeCardIds.Contains(Card.CardId);
			const int32 DraftIndex =
				InteractionView.DraftCornerNomineeCardIds.IndexOfByKey(Card.CardId);
			Cell.SetPieceSelectionOrder = DraftIndex == INDEX_NONE
				? 0 : DraftIndex + 1;
			Cell.bDeploymentDraggable = bLocalRack && !Cell.bPlayed
				&& !Cell.bSetPieceSelectable
				&& DeploymentGroups.ContainsByPredicate(
					[&Card](const FFMCodexLocalMatchDeploymentGroup& Group)
					{
						return Group.CardId == Card.CardId;
					});
			Cell.Card = MakeCard(Card);
			Rack.Cells.Add(MoveTemp(Cell));
		}
	}

	void AddUniqueCard(
		TArray<FFMCodexUMGCardViewModel>& Cards,
		const FFMCodexLocalMatchCardView& Card)
	{
		if (!Cards.ContainsByPredicate(
			[&Card](const FFMCodexUMGCardViewModel& Existing)
			{
				return Existing.CardId == Card.CardId;
			}))
		{
			Cards.Add(MakeCard(Card));
		}
	}

	FString BranchIntentLabel(const EMatchPlayElectiveBranchIntent Intent)
	{
		FString CanonicalLabel;
		switch (Intent)
		{
		case EMatchPlayElectiveBranchIntent::DirectShot:
			CanonicalLabel = TEXT("Direct Shot");
			break;
		case EMatchPlayElectiveBranchIntent::DeadCorner:
			CanonicalLabel = TEXT("Dead Corner");
			break;
		case EMatchPlayElectiveBranchIntent::CrossHigh:
			CanonicalLabel = TEXT("Cross High");
			break;
		case EMatchPlayElectiveBranchIntent::CrossLow:
			CanonicalLabel = TEXT("Cross Low");
			break;
		default:
			return FString();
		}
		return FFMCodexPlayerUIPresentationText::MatchScreenLabel(
			CanonicalLabel).ToString();
	}

	EFMCodexUMGInteractionCategory InteractionCategory(
		const EFMCodexLocalMatchInteractionCategory Category)
	{
		switch (Category)
		{
		case EFMCodexLocalMatchInteractionCategory::StartMatch:
			return EFMCodexUMGInteractionCategory::StartMatch;
		case EFMCodexLocalMatchInteractionCategory::TacticalPointRoll:
			return EFMCodexUMGInteractionCategory::TacticalPointRoll;
		case EFMCodexLocalMatchInteractionCategory::Deploy:
			return EFMCodexUMGInteractionCategory::Deploy;
		case EFMCodexLocalMatchInteractionCategory::SelectCarrier:
			return EFMCodexUMGInteractionCategory::SelectCarrier;
		case EFMCodexLocalMatchInteractionCategory::SelectMarker:
			return EFMCodexUMGInteractionCategory::SelectMarker;
		case EFMCodexLocalMatchInteractionCategory::SelectSkill:
			return EFMCodexUMGInteractionCategory::SelectSkill;
		case EFMCodexLocalMatchInteractionCategory::SelectRunner:
			return EFMCodexUMGInteractionCategory::SelectRunner;
		case EFMCodexLocalMatchInteractionCategory::SelectHelper:
			return EFMCodexUMGInteractionCategory::SelectHelper;
		case EFMCodexLocalMatchInteractionCategory::SelectBranchIntent:
			return EFMCodexUMGInteractionCategory::SelectBranchIntent;
		case EFMCodexLocalMatchInteractionCategory::SelectOneOnOneShot:
			return EFMCodexUMGInteractionCategory::SelectOneOnOneShot;
		case EFMCodexLocalMatchInteractionCategory::RollCrossAttack:
			return EFMCodexUMGInteractionCategory::RollCrossAttack;
		case EFMCodexLocalMatchInteractionCategory::RollCrossDefense:
			return EFMCodexUMGInteractionCategory::RollCrossDefense;
		case EFMCodexLocalMatchInteractionCategory::RollThroughBallInitialRoute:
			return EFMCodexUMGInteractionCategory::RollThroughBallInitialRoute;
		case EFMCodexLocalMatchInteractionCategory::SelectLongShotBranch:
			return EFMCodexUMGInteractionCategory::SelectLongShotBranch;
		case EFMCodexLocalMatchInteractionCategory::RollLongShotDirectAttack:
			return EFMCodexUMGInteractionCategory::RollLongShotDirectAttack;
		case EFMCodexLocalMatchInteractionCategory::RollLongShotDirectDefense:
			return EFMCodexUMGInteractionCategory::RollLongShotDirectDefense;
		case EFMCodexLocalMatchInteractionCategory::RollLongShotDeadCorner:
			return EFMCodexUMGInteractionCategory::RollLongShotDeadCorner;
		case EFMCodexLocalMatchInteractionCategory::RollCutInsideShotDirectAttack:
			return EFMCodexUMGInteractionCategory::RollCutInsideShotDirectAttack;
		case EFMCodexLocalMatchInteractionCategory::RollCutInsideShotDirectDefense:
			return EFMCodexUMGInteractionCategory::RollCutInsideShotDirectDefense;
		case EFMCodexLocalMatchInteractionCategory::RollCutInsideShotDeadCorner:
			return EFMCodexUMGInteractionCategory::RollCutInsideShotDeadCorner;
		case EFMCodexLocalMatchInteractionCategory::RollPassControlRoute:
			return EFMCodexUMGInteractionCategory::RollPassControlRoute;
		case EFMCodexLocalMatchInteractionCategory::RollPassControlAttack:
			return EFMCodexUMGInteractionCategory::RollPassControlAttack;
		case EFMCodexLocalMatchInteractionCategory::RollPassControlDefense:
			return EFMCodexUMGInteractionCategory::RollPassControlDefense;
		case EFMCodexLocalMatchInteractionCategory::RollCrossRoute:
			return EFMCodexUMGInteractionCategory::RollCrossRoute;
		case EFMCodexLocalMatchInteractionCategory::ResolveSendingOff:
			return EFMCodexUMGInteractionCategory::ResolveSendingOff;
		case EFMCodexLocalMatchInteractionCategory::RollSetPieceType:
			return EFMCodexUMGInteractionCategory::RollSetPieceType;
		case EFMCodexLocalMatchInteractionCategory::SelectSetPieceCarrier:
			return EFMCodexUMGInteractionCategory::SelectSetPieceCarrier;
		case EFMCodexLocalMatchInteractionCategory::ConfirmSetPieceCarrier:
			return EFMCodexUMGInteractionCategory::ConfirmSetPieceCarrier;
		case EFMCodexLocalMatchInteractionCategory::SelectSetPieceMethod:
			return EFMCodexUMGInteractionCategory::SelectSetPieceMethod;
		case EFMCodexLocalMatchInteractionCategory::RollShortFreeKickDirectAttack:
			return EFMCodexUMGInteractionCategory::RollShortFreeKickDirectAttack;
		case EFMCodexLocalMatchInteractionCategory::RollShortFreeKickDirectDefense:
			return EFMCodexUMGInteractionCategory::RollShortFreeKickDirectDefense;
		case EFMCodexLocalMatchInteractionCategory::RollShortFreeKickAngled:
			return EFMCodexUMGInteractionCategory::RollShortFreeKickAngled;
		case EFMCodexLocalMatchInteractionCategory::RollLongFreeKickDirectAttack:
			return EFMCodexUMGInteractionCategory::RollLongFreeKickDirectAttack;
		case EFMCodexLocalMatchInteractionCategory::RollLongFreeKickDirectDefense:
			return EFMCodexUMGInteractionCategory::RollLongFreeKickDirectDefense;
		case EFMCodexLocalMatchInteractionCategory::RollLongFreeKickPower:
			return EFMCodexUMGInteractionCategory::RollLongFreeKickPower;
		case EFMCodexLocalMatchInteractionCategory::RollPenaltyDirectAttack:
			return EFMCodexUMGInteractionCategory::RollPenaltyDirectAttack;
		case EFMCodexLocalMatchInteractionCategory::RollPenaltyDirectDefense:
			return EFMCodexUMGInteractionCategory::RollPenaltyDirectDefense;
		case EFMCodexLocalMatchInteractionCategory::RollPenaltyPanenka:
			return EFMCodexUMGInteractionCategory::RollPenaltyPanenka;
		case EFMCodexLocalMatchInteractionCategory::DraftCornerAttacker:
			return EFMCodexUMGInteractionCategory::DraftCornerAttacker;
		case EFMCodexLocalMatchInteractionCategory::DraftCornerDefender:
			return EFMCodexUMGInteractionCategory::DraftCornerDefender;
		case EFMCodexLocalMatchInteractionCategory::RollCornerParticipantSelection:
			return EFMCodexUMGInteractionCategory::RollCornerParticipantSelection;
		case EFMCodexLocalMatchInteractionCategory::SelectCornerIntent:
			return EFMCodexUMGInteractionCategory::SelectCornerIntent;
		case EFMCodexLocalMatchInteractionCategory::RollCornerRoute:
			return EFMCodexUMGInteractionCategory::RollCornerRoute;
		case EFMCodexLocalMatchInteractionCategory::RollCornerAttack:
			return EFMCodexUMGInteractionCategory::RollCornerAttack;
		case EFMCodexLocalMatchInteractionCategory::RollCornerDefense:
			return EFMCodexUMGInteractionCategory::RollCornerDefense;
		case EFMCodexLocalMatchInteractionCategory::CompleteCrossAndAdvance:
			return EFMCodexUMGInteractionCategory::CompleteCrossAndAdvance;
		case EFMCodexLocalMatchInteractionCategory::RollThroughBallFeetAttack:
			return EFMCodexUMGInteractionCategory::RollThroughBallFeetAttack;
		case EFMCodexLocalMatchInteractionCategory::RollThroughBallFeetDefense:
			return EFMCodexUMGInteractionCategory::RollThroughBallFeetDefense;
		case EFMCodexLocalMatchInteractionCategory
			::RollThroughBallAntiOffsideAttack:
			return EFMCodexUMGInteractionCategory
				::RollThroughBallAntiOffsideAttack;
		case EFMCodexLocalMatchInteractionCategory
			::RollThroughBallOneOnOneChipShotAttack:
			return EFMCodexUMGInteractionCategory
				::RollThroughBallOneOnOneChipShotAttack;
		case EFMCodexLocalMatchInteractionCategory
			::RollThroughBallOneOnOneDirectShotAttack:
			return EFMCodexUMGInteractionCategory
				::RollThroughBallOneOnOneDirectShotAttack;
		case EFMCodexLocalMatchInteractionCategory
			::RollThroughBallOneOnOneDirectShotDefense:
			return EFMCodexUMGInteractionCategory
				::RollThroughBallOneOnOneDirectShotDefense;
		case EFMCodexLocalMatchInteractionCategory
			::RollThroughBallBehindDefenseAttack:
			return EFMCodexUMGInteractionCategory
				::RollThroughBallBehindDefenseAttack;
		case EFMCodexLocalMatchInteractionCategory
			::RollThroughBallBehindDefenseDefense:
			return EFMCodexUMGInteractionCategory
				::RollThroughBallBehindDefenseDefense;
		case EFMCodexLocalMatchInteractionCategory
			::CompleteThroughBallFeetAndAdvance:
			return EFMCodexUMGInteractionCategory
				::CompleteThroughBallFeetAndAdvance;
		case EFMCodexLocalMatchInteractionCategory
			::ApplyCrossTerminalResolution:
			return EFMCodexUMGInteractionCategory
				::ApplyCrossTerminalResolution;
		case EFMCodexLocalMatchInteractionCategory
			::ApplyThroughBallFeetTerminalResolution:
			return EFMCodexUMGInteractionCategory
				::ApplyThroughBallFeetTerminalResolution;
		case EFMCodexLocalMatchInteractionCategory::AdvanceAfterTerminal:
			return EFMCodexUMGInteractionCategory::AdvanceAfterTerminal;
		case EFMCodexLocalMatchInteractionCategory::ContinueResolution:
			return EFMCodexUMGInteractionCategory::ContinueResolution;
		case EFMCodexLocalMatchInteractionCategory::AttackComplete:
			return EFMCodexUMGInteractionCategory::AttackComplete;
		case EFMCodexLocalMatchInteractionCategory::MatchEnded:
			return EFMCodexUMGInteractionCategory::MatchEnded;
		default:
			return EFMCodexUMGInteractionCategory::None;
		}
	}

	EFMCodexUMGBranchIntent BranchIntent(
		const EMatchPlayElectiveBranchIntent Intent)
	{
		switch (Intent)
		{
		case EMatchPlayElectiveBranchIntent::DirectShot:
			return EFMCodexUMGBranchIntent::DirectShot;
		case EMatchPlayElectiveBranchIntent::DeadCorner:
			return EFMCodexUMGBranchIntent::DeadCorner;
		case EMatchPlayElectiveBranchIntent::CrossHigh:
			return EFMCodexUMGBranchIntent::CrossHigh;
		case EMatchPlayElectiveBranchIntent::CrossLow:
			return EFMCodexUMGBranchIntent::CrossLow;
		default:
			return EFMCodexUMGBranchIntent::None;
		}
	}

	EFMCodexUMGOneOnOneChoice OneOnOneChoice(
		const EMatchPlayThroughBallOneOnOneShotChoice Choice)
	{
		return Choice == EMatchPlayThroughBallOneOnOneShotChoice::ChipShot
			? EFMCodexUMGOneOnOneChoice::ChipShot
			: Choice == EMatchPlayThroughBallOneOnOneShotChoice::DirectShot
				? EFMCodexUMGOneOnOneChoice::DirectShot
				: EFMCodexUMGOneOnOneChoice::None;
	}

	FString DeclineLabel(const EFMCodexLocalMatchInteractionCategory Category)
	{
		switch (Category)
		{
		case EFMCodexLocalMatchInteractionCategory::SelectMarker:
			return TEXT("DECLINE MARKER");
		case EFMCodexLocalMatchInteractionCategory::SelectSkill:
			return TEXT("DECLINE SKILL");
		case EFMCodexLocalMatchInteractionCategory::SelectRunner:
			return TEXT("DECLINE RUNNER");
		case EFMCodexLocalMatchInteractionCategory::SelectHelper:
			return TEXT("DECLINE HELPER");
		default:
			return FString();
		}
	}

	FString NoLegalLabel(const EFMCodexLocalMatchInteractionCategory Category)
	{
		switch (Category)
		{
		case EFMCodexLocalMatchInteractionCategory::SelectCarrier:
			return TEXT("RESOLVE NO LEGAL CARRIER");
		case EFMCodexLocalMatchInteractionCategory::SelectMarker:
			return TEXT("RESOLVE NO LEGAL MARKER");
		case EFMCodexLocalMatchInteractionCategory::SelectSkill:
			// Player-facing semantics are identical to voluntary tactical
			// decline; only the typed authoritative entry point differs.
			return TEXT("DECLINE SKILL");
		case EFMCodexLocalMatchInteractionCategory::SelectRunner:
			return TEXT("RESOLVE NO LEGAL RUNNER");
		case EFMCodexLocalMatchInteractionCategory::SelectHelper:
			return TEXT("RESOLVE NO LEGAL HELPER");
		default:
			return FString();
		}
	}

	FString DiceContextLabel(const EFMCodexLocalMatchRollGroup Group)
	{
		switch (Group)
		{
		case EFMCodexLocalMatchRollGroup::InitialRoute:
			return TEXT("ROUTE");
		case EFMCodexLocalMatchRollGroup::PostRoute:
			return TEXT("POST-ROUTE");
		case EFMCodexLocalMatchRollGroup::OneOnOne:
			return TEXT("ONE-ON-ONE");
		default:
			return TEXT("ACCEPTED DICE");
		}
	}

	FString CompactNumber(const float Value)
	{
		FString Result = FString::Printf(TEXT("%.3f"), Value);
		while (Result.EndsWith(TEXT("0")))
		{
			Result.LeftChopInline(1);
		}
		if (Result.EndsWith(TEXT(".")))
		{
			Result.LeftChopInline(1);
		}
		return Result == TEXT("-0") ? TEXT("0") : Result;
	}

	const FFMCodexLocalMatchCardView* FindCardView(
		const FFMCodexLocalMatchInteractionView& InteractionView,
		const EInitialTurnOrderPlayer Side,
		const FName CardId)
	{
		const TArray<FFMCodexLocalMatchCardView>* Roster =
			Side == EInitialTurnOrderPlayer::PlayerA
				? &InteractionView.PlayerACardRoster
				: Side == EInitialTurnOrderPlayer::PlayerB
					? &InteractionView.PlayerBCardRoster : nullptr;
		return Roster == nullptr ? nullptr : Roster->FindByPredicate(
			[CardId](const FFMCodexLocalMatchCardView& Card)
			{
				return Card.CardId == CardId;
			});
	}

	FString PlayerFacingName(
		const FFMCodexLocalMatchInteractionView& InteractionView,
		const EInitialTurnOrderPlayer Side,
		const FName CardId)
	{
		const FFMCodexLocalMatchCardView* Card =
			FindCardView(InteractionView, Side, CardId);
		return FFMCodexPlayerUIPresentationText::InMatchShortPlayerName(
			CardId, Card == nullptr ? FString() : Card->DisplayLabel).ToString();
	}

	const FMatchPlayResolutionParticipantFact* FindParticipant(
		const FMatchPlayCurrentAttackResolutionFactProjection& Facts,
		const EMatchPlayResolutionParticipantRole Role)
	{
		return Facts.Participants.FindByPredicate(
			[Role](const FMatchPlayResolutionParticipantFact& Participant)
			{
				return Participant.Role == Role;
			});
	}

	FFMCodexTacticalNarrativeActor NarrativeActor(
		const FMatchPlayCurrentAttackResolutionFactProjection& Facts,
		const FFMCodexLocalMatchInteractionView& InteractionView,
		const EMatchPlayResolutionParticipantRole Role)
	{
		FFMCodexTacticalNarrativeActor Result;
		const FMatchPlayResolutionParticipantFact* Participant =
			FindParticipant(Facts, Role);
		if (Participant != nullptr)
		{
			Result.CardId = Participant->CardId;
			Result.DisplayName = FText::FromString(PlayerFacingName(
				InteractionView, Participant->Side, Participant->CardId));
		}
		return Result;
	}

	void AddFormulaParticipant(
		FFMCodexUMGInlineFormulaRowViewModel& OutRow,
		const FFMCodexLocalMatchInteractionView& InteractionView,
		const FMatchPlayResolutionFormulaTermFact& Term)
	{
		if (Term.CardId.IsNone()
			|| Term.ParticipantRole
				== EMatchPlayResolutionParticipantRole::None
			|| OutRow.Participants.ContainsByPredicate(
				[&Term, &InteractionView](
					const FFMCodexUMGInlineFormulaParticipantViewModel& Existing)
				{
					return Existing.RoleLabel
						== FFMCodexPlayerUIPresentationText
							::ResolutionParticipantRole(Term.ParticipantRole)
								.ToString()
						&& Existing.PlayerName
							== PlayerFacingName(
								InteractionView, Term.Side, Term.CardId);
				}))
		{
			return;
		}
		const FString Name = PlayerFacingName(
			InteractionView, Term.Side, Term.CardId);
		const FString Role = FFMCodexPlayerUIPresentationText
			::ResolutionParticipantRole(Term.ParticipantRole).ToString();
		if (!Name.IsEmpty() && !Role.IsEmpty())
		{
			OutRow.Participants.Add({ Role, Name });
		}
	}

	FFMCodexUMGInlineFormulaRowViewModel BuildFormulaRow(
		const FMatchPlayResolutionFormulaRowFact& Row,
		const FString& SideLabel,
		const FMatchPlayCurrentAttackResolutionFactProjection& Facts,
		const FFMCodexLocalMatchInteractionView& InteractionView)
	{
		using EFactTerm = EMatchPlayResolutionFormulaTermKind;
		FFMCodexUMGInlineFormulaRowViewModel Result;
		Result.Side = Row.Side;
		Result.SideLabel = SideLabel;
		Result.bKnownNonRollSubtotalResolved =
			Row.bKnownNonRollSubtotalResolved;
		Result.KnownNonRollSubtotal = Row.KnownNonRollSubtotal;
		Result.KnownNonRollSubtotalLabel =
			Row.bKnownNonRollSubtotalResolved
				? FString::Printf(
					TEXT("基础值 %s"),
					*CompactNumber(Row.KnownNonRollSubtotal))
				: TEXT("基础值 ?");
		Result.bFinalValueResolved = Row.bFinalValueResolved;
		Result.FinalValue = Row.FinalValue;
		Result.FinalValueLabel = Row.bFinalValueResolved
			? CompactNumber(Row.FinalValue) : TEXT("?");
		Result.bDisplayedResultResolved = Row.bFinalValueResolved
			|| Row.bKnownNonRollSubtotalResolved;
		Result.bDisplayedResultIsFinalValue = Row.bFinalValueResolved;
		Result.DisplayedResult = Row.bFinalValueResolved
			? Row.FinalValue : Row.KnownNonRollSubtotal;
		Result.DisplayedResultLabel = Row.bFinalValueResolved
			? Result.FinalValueLabel
			: Row.bKnownNonRollSubtotalResolved
				? CompactNumber(Row.KnownNonRollSubtotal)
				: TEXT("?");

		for (const FMatchPlayResolutionFormulaTermFact& Term : Row.Terms)
		{
			FFMCodexUMGInlineFormulaTermViewModel View;
			View.SourceValue = Term.SourceValue;
			View.Multiplier = Term.Multiplier;
			View.Contribution = Term.Contribution;
			View.RollSequenceIndex = Term.RollSequenceIndex;
			if (Term.Kind == EFactTerm::RawRoll)
			{
				View.Kind = EFMCodexUMGInlineFormulaTermKind::RawRoll;
				const FMatchPlayResolutionRollFact* Roll =
					Facts.Rolls.FindByPredicate(
						[&Term](const FMatchPlayResolutionRollFact& Candidate)
						{
							return Candidate.SequenceIndex
								== Term.RollSequenceIndex
								&& Candidate.Semantics
									== EMatchPlayResolutionRollSemantics
										::ArithmeticContest;
						});
				View.bResolved = Roll != nullptr && Roll->bResolved;
				View.RawD6 = View.bResolved ? Roll->RawD6 : 0;
				View.bNextPendingRoll = !View.bResolved
					&& Facts.bHasPendingRoll
					&& Facts.NextPendingRollSequenceIndex
						== Term.RollSequenceIndex;
				View.DisplayLabel = View.bResolved
					? FString::Printf(TEXT("掷点 %d"), View.RawD6)
					: TEXT("掷点 ?");
			}
			else if (Term.Kind == EFactTerm::FixedModifier)
			{
				View.Kind = EFMCodexUMGInlineFormulaTermKind::FixedModifier;
				View.bResolved = Term.bResolved;
				View.DisplayLabel = FString::Printf(
					TEXT("%s%s"), Term.Contribution >= 0.0f ? TEXT("+") : TEXT(""),
					*CompactNumber(Term.Contribution));
			}
			else if (Term.Kind == EFactTerm::TacticalPlayerAdvantage)
			{
				View.Kind = EFMCodexUMGInlineFormulaTermKind::FixedModifier;
				View.bResolved = Term.bResolved;
				View.DisplayLabel = FString::Printf(
					TEXT("战术球员 %s%s"),
					Term.Contribution >= 0.0f ? TEXT("+") : TEXT(""),
					*CompactNumber(Term.Contribution));
			}
			else if (Term.Kind == EFactTerm::Attribute
				|| Term.Kind == EFactTerm::GoalkeeperContribution)
			{
				View.Kind = EFMCodexUMGInlineFormulaTermKind::Attribute;
				View.bResolved = Term.bResolved;
				View.AttributeLabel = FFMCodexPlayerUIPresentationText
					::ResolutionAttribute(Term.Attribute).ToString();
				if (View.AttributeLabel.IsEmpty())
				{
					continue;
				}
				View.DisplayLabel = FMath::IsNearlyEqual(Term.Multiplier, 1.0f)
					? FString::Printf(TEXT("%s %s"), *View.AttributeLabel,
						*CompactNumber(Term.SourceValue))
					: FString::Printf(TEXT("%s %s \u00D7%s"), *View.AttributeLabel,
						*CompactNumber(Term.SourceValue),
						*CompactNumber(Term.Multiplier));
				if (!Term.CardId.IsNone())
				{
					View.ContributorDisplayName = PlayerFacingName(
						InteractionView, Term.Side, Term.CardId);
				}
				AddFormulaParticipant(Result, InteractionView, Term);
			}
			else
			{
				continue;
			}
			Result.Terms.Add(MoveTemp(View));
		}
		return Result;
	}

	bool CanFormulaSurfaceClaim(
		const EFMCodexUMGInteractionCategory Category)
	{
		switch (Category)
		{
		case EFMCodexUMGInteractionCategory::RollCrossAttack:
		case EFMCodexUMGInteractionCategory::RollCrossDefense:
		case EFMCodexUMGInteractionCategory::RollThroughBallFeetAttack:
		case EFMCodexUMGInteractionCategory::RollThroughBallFeetDefense:
		case EFMCodexUMGInteractionCategory::RollThroughBallOneOnOneDirectShotAttack:
		case EFMCodexUMGInteractionCategory::RollThroughBallOneOnOneDirectShotDefense:
		case EFMCodexUMGInteractionCategory
			::RollThroughBallBehindDefenseAttack:
		case EFMCodexUMGInteractionCategory
			::RollThroughBallBehindDefenseDefense:
		case EFMCodexUMGInteractionCategory::RollLongShotDirectAttack:
		case EFMCodexUMGInteractionCategory::RollLongShotDirectDefense:
		case EFMCodexUMGInteractionCategory::RollCutInsideShotDirectAttack:
		case EFMCodexUMGInteractionCategory::RollCutInsideShotDirectDefense:
		case EFMCodexUMGInteractionCategory::RollPassControlAttack:
		case EFMCodexUMGInteractionCategory::RollPassControlDefense:
		case EFMCodexUMGInteractionCategory::ApplyCrossTerminalResolution:
		case EFMCodexUMGInteractionCategory
			::ApplyThroughBallFeetTerminalResolution:
		case EFMCodexUMGInteractionCategory::AdvanceAfterTerminal:
			return true;
		default:
			return false;
		}
	}

	void ClaimPrimaryAction(
		FFMCodexUMGResolutionPrimaryActionSlotViewModel& Slot,
		const FFMCodexUMGPrimaryActionViewModel& Action)
	{
		if (!Action.bAvailable)
		{
			return;
		}
		Slot.bClaimsAction = true;
		Slot.bVisible = true;
		Slot.Action = Action;
	}

	bool IsSetPieceOrdinaryResolutionCategory(
		const EFMCodexLocalMatchInteractionCategory Category)
	{
		switch (Category)
		{
		case EFMCodexLocalMatchInteractionCategory::RollShortFreeKickDirectAttack:
		case EFMCodexLocalMatchInteractionCategory::RollShortFreeKickDirectDefense:
		case EFMCodexLocalMatchInteractionCategory::RollShortFreeKickAngled:
		case EFMCodexLocalMatchInteractionCategory::RollLongFreeKickDirectAttack:
		case EFMCodexLocalMatchInteractionCategory::RollLongFreeKickDirectDefense:
		case EFMCodexLocalMatchInteractionCategory::RollLongFreeKickPower:
		case EFMCodexLocalMatchInteractionCategory::RollPenaltyDirectAttack:
		case EFMCodexLocalMatchInteractionCategory::RollPenaltyDirectDefense:
		case EFMCodexLocalMatchInteractionCategory::RollPenaltyPanenka:
		case EFMCodexLocalMatchInteractionCategory::RollCornerAttack:
		case EFMCodexLocalMatchInteractionCategory::RollCornerDefense:
			return true;
		default:
			return false;
		}
	}

	FString SetPieceContestLabel(
		const FFMCodexLocalMatchInteractionView& InteractionView,
		const bool bCompactMethod)
	{
		switch (InteractionView.SetPieceType)
		{
		case ESetPieceSelectedType::ShortFreeKick:
			return bCompactMethod
				? TEXT("近距离任意球 · 战术配合")
				: TEXT("近距离任意球 · 直接射门");
		case ESetPieceSelectedType::LongFreeKick:
			return FText::Format(NSLOCTEXT("FMCodexSetPiece", "LongMethod", "{0} · {1}"),
				FFMCodexPlayerUIPresentationText::SetPieceName(InteractionView.SetPieceType),
				bCompactMethod ? FFMCodexPlayerUIPresentationText::LongFreeKickPowerStage()
					: FFMCodexPlayerUIPresentationText::LongShotDirectStage()).ToString();
		case ESetPieceSelectedType::Penalty:
			return bCompactMethod
				? TEXT("点球 · 勺子点球")
				: TEXT("点球 · 常规点球");
		case ESetPieceSelectedType::Corner:
			return InteractionView.CornerActualRoute
				== EMatchPlayCornerRouteIntent::High
					? TEXT("角球 · 高球")
					: InteractionView.CornerActualRoute
						== EMatchPlayCornerRouteIntent::Low
							? TEXT("角球 · 低平球") : TEXT("角球");
		default:
			return TEXT("定位球");
		}
	}

	FString SetPieceTypeLabel(const ESetPieceSelectedType Type)
	{
		return FFMCodexPlayerUIPresentationText::SetPieceName(Type).ToString();
	}

	FString SetPieceOutcomeLabel(
		const FFMCodexLocalMatchInteractionView& InteractionView)
	{
		if (InteractionView.bSetPieceNoLegalCarrier)
		{
			return TEXT("没有合法主罚球员，本次进攻未形成进球");
		}
		if (InteractionView.bSetPieceSystemGoal)
		{
			return TEXT("系统进球");
		}
		if (!InteractionView.bHasSetPieceOutcome)
		{
			return FString();
		}
		if (!InteractionView.bSetPieceGoal)
		{
			return TEXT("未进球");
		}
		if (InteractionView.SetPieceGoalScorerCardId.IsNone())
		{
			return TEXT("进球");
		}
		return FString::Printf(TEXT("进球 · %s"), *PlayerFacingName(
			InteractionView, InteractionView.CurrentAttackingPlayer,
			InteractionView.SetPieceGoalScorerCardId));
	}

	FFMCodexUMGInlineFormulaTermViewModel SetPieceRollTerm(
		const bool bResolved,
		const int32 RawD6,
		const int32 SequenceIndex,
		const bool bNextPending)
	{
		FFMCodexUMGInlineFormulaTermViewModel Result;
		Result.Kind = EFMCodexUMGInlineFormulaTermKind::RawRoll;
		Result.RollSequenceIndex = SequenceIndex;
		Result.RawD6 = bResolved ? RawD6 : 0;
		Result.bResolved = bResolved;
		Result.bNextPendingRoll = bNextPending;
		Result.DisplayLabel = bResolved
			? FString::Printf(TEXT("掷点 %d"), RawD6)
			: FString(TEXT("掷点 ?"));
		return Result;
	}

	void AddSetPieceParticipant(
		FFMCodexUMGInlineFormulaRowViewModel& Row,
		const FFMCodexLocalMatchInteractionView& InteractionView,
		const FMatchPlaySetPieceParticipantBinding& Binding,
		const TCHAR* RoleLabel)
	{
		if (!Binding.bIsBound || Binding.CardId.IsNone())
		{
			return;
		}
		Row.Participants.Add({ RoleLabel, PlayerFacingName(
			InteractionView, Binding.OwnerSide, Binding.CardId) });
	}

	const FFMCodexLocalMatchCardView* FindSetPieceGoalkeeper(
		const FFMCodexLocalMatchInteractionView& InteractionView,
		const EInitialTurnOrderPlayer Defender)
	{
		const TArray<FFMCodexLocalMatchCardView>* Roster =
			Defender == EInitialTurnOrderPlayer::PlayerA
				? &InteractionView.PlayerACardRoster
				: Defender == EInitialTurnOrderPlayer::PlayerB
					? &InteractionView.PlayerBCardRoster : nullptr;
		return Roster == nullptr ? nullptr : Roster->FindByPredicate(
			[](const FFMCodexLocalMatchCardView& Card)
			{
				return Card.bGoalkeeper;
			});
	}

	int32 SetPieceCardAttribute(
		const FFMCodexLocalMatchCardView* Card,
		const TCHAR* CanonicalLabel)
	{
		if (Card == nullptr)
		{
			return 0;
		}
		const FFMCodexLocalMatchCardView::FAttribute* Attribute =
			Card->AttributeValues.FindByPredicate(
				[CanonicalLabel](
					const FFMCodexLocalMatchCardView::FAttribute& Candidate)
				{
					return Candidate.CanonicalLabel == CanonicalLabel;
				});
		return Attribute == nullptr ? 0 : Attribute->Value;
	}

	FFMCodexUMGInlineFormulaTermViewModel SetPieceAttributeTerm(
		const FString& DisplayLabel)
	{
		FFMCodexUMGInlineFormulaTermViewModel Result;
		Result.Kind = EFMCodexUMGInlineFormulaTermKind::Attribute;
		Result.DisplayLabel = DisplayLabel;
		Result.bResolved = true;
		return Result;
	}

	FFMCodexUMGInlineFormulaTermViewModel SetPieceModifierTerm(
		const int32 Modifier,
		const TCHAR* PlayerFacingLabel = nullptr)
	{
		FFMCodexUMGInlineFormulaTermViewModel Result;
		Result.Kind = EFMCodexUMGInlineFormulaTermKind::FixedModifier;
		Result.DisplayLabel = PlayerFacingLabel != nullptr
			? FString(PlayerFacingLabel)
			: FString::Printf(TEXT("%s%d"),
				Modifier >= 0 ? TEXT("+") : TEXT(""), Modifier);
		Result.bResolved = true;
		return Result;
	}

	void AddSetPieceGoalkeeperParticipant(
		FFMCodexUMGInlineFormulaRowViewModel& Row,
		const FFMCodexLocalMatchInteractionView& InteractionView,
		const EInitialTurnOrderPlayer Defender)
	{
		const FFMCodexLocalMatchCardView* Goalkeeper =
			FindSetPieceGoalkeeper(InteractionView, Defender);
		if (Goalkeeper != nullptr)
		{
			Row.Participants.Add({ TEXT("门将"), PlayerFacingName(
				InteractionView, Defender, Goalkeeper->CardId) });
		}
	}

	FString SetPieceTerminalSupportLine(
		const FFMCodexLocalMatchInteractionView& InteractionView,
		const bool bCompactMethod,
		const FString& MethodLabel,
		const FString& ResultTitle)
	{
		if (InteractionView.SetPieceType == ESetPieceSelectedType::Corner)
		{
			if (InteractionView.CornerAttackerNominees.IsEmpty() && !InteractionView.bSetPieceGoal)
				return NSLOCTEXT("FMCodexCorner", "NoAttackingCandidate", "进攻方无人抢到点").ToString();
			const FText AttackerName = InteractionView.CornerRunner.bIsBound
				? FText::FromString(PlayerFacingName(InteractionView, InteractionView.CornerRunner.OwnerSide,
					InteractionView.CornerRunner.CardId)) : FText::GetEmpty();
			if (InteractionView.bSetPieceGoal)
				return FText::Format(NSLOCTEXT("FMCodexCorner", "GoalEvent", "{0}角球破门！"), AttackerName).ToString();
			// Aggregate NoGoal proves neither a save nor a shot wide. Name the
			// actual attacker and known route without inventing a finishing event.
			const FText Route = InteractionView.CornerActualRoute == EMatchPlayCornerRouteIntent::High
				? NSLOCTEXT("FMCodexCorner", "HighAttack", "高球攻门")
				: InteractionView.CornerActualRoute == EMatchPlayCornerRouteIntent::Low
					? NSLOCTEXT("FMCodexCorner", "LowAttack", "低平球攻门")
					: NSLOCTEXT("FMCodexCorner", "CornerAttack", "角球攻门");
			return AttackerName.IsEmpty()
				? FText::Format(NSLOCTEXT("FMCodexCorner", "UnnamedNoGoalEvent", "{0}未能得分。"), Route).ToString()
				: FText::Format(NSLOCTEXT("FMCodexCorner", "NoGoalEvent", "{0}的{1}未能得分。"), AttackerName, Route).ToString();
		}
		if (!InteractionView.SetPieceCarrier.bIsBound)
		{
			return FString::Printf(TEXT("%s · %s"),
				*MethodLabel, *ResultTitle);
		}

		const FString Taker = PlayerFacingName(
			InteractionView,
			InteractionView.SetPieceCarrier.OwnerSide,
			InteractionView.SetPieceCarrier.CardId);
		const EInitialTurnOrderPlayer Defender = OtherSide(
			InteractionView.CurrentAttackingPlayer);
		const FFMCodexLocalMatchCardView* Goalkeeper =
			FindSetPieceGoalkeeper(InteractionView, Defender);
		const FString GoalkeeperName = Goalkeeper == nullptr
			? FString()
			: PlayerFacingName(InteractionView, Defender, Goalkeeper->CardId);

		switch (InteractionView.SetPieceType)
		{
		case ESetPieceSelectedType::ShortFreeKick:
			if (InteractionView.bSetPieceGoal)
			{
				return bCompactMethod
					? FString::Printf(
						TEXT("%s近距离任意球战术配合破门！"), *Taker)
					: FString::Printf(
						TEXT("%s近距离任意球直接破门！"), *Taker);
			}
			if (bCompactMethod)
			{
				return FString::Printf(
					TEXT("%s近距离任意球战术配合未能形成进球。"), *Taker);
			}
			return !GoalkeeperName.IsEmpty()
				? FString::Printf(TEXT("%s近距离任意球被%s扑出！"),
					*Taker, *GoalkeeperName)
				: FString::Printf(TEXT("%s近距离任意球未能破门。"), *Taker);

		case ESetPieceSelectedType::LongFreeKick:
		{
			const FText Name = FFMCodexPlayerUIPresentationText::SetPieceName(
				InteractionView.SetPieceType);
			const FText Player = FText::FromString(Taker);
			if (bCompactMethod)
			{
				return FText::Format(InteractionView.bSetPieceGoal
					? NSLOCTEXT("FMCodexSetPiece", "LongPowerGoal", "{0}{1}{2}得手！")
					: NSLOCTEXT("FMCodexSetPiece", "LongPowerMiss", "{0}{1}{2}未能得分。"),
					Player, Name, FFMCodexPlayerUIPresentationText::LongFreeKickPowerStage()).ToString();
			}
			if (InteractionView.bSetPieceGoal)
			{
				return FText::Format(NSLOCTEXT("FMCodexSetPiece", "LongDirectGoal", "{0}{1}直接破门！"),
					Player, Name).ToString();
			}
			if (!InteractionView.bHasSetPieceFormula)
			{
				return FText::Format(NSLOCTEXT("FMCodexSetPiece", "LongDirectWide",
					"{0}{1}直接射偏。"), Player, Name).ToString();
			}
			return !GoalkeeperName.IsEmpty()
				? FText::Format(NSLOCTEXT("FMCodexSetPiece", "LongDirectStopped",
					"{0}{1}被{2}化解。"), Player, Name, FText::FromString(GoalkeeperName)).ToString()
				: FText::Format(NSLOCTEXT("FMCodexSetPiece", "LongDirectMiss",
					"{0}{1}未能破门。"), Player, Name).ToString();
		}

		case ESetPieceSelectedType::Penalty:
			if (InteractionView.bSetPieceGoal)
			{
				return bCompactMethod
					? FString::Printf(TEXT("%s勺子点球命中！"), *Taker)
					: FString::Printf(TEXT("%s主罚点球命中！"), *Taker);
			}
			if (bCompactMethod)
			{
				return FString::Printf(TEXT("%s勺子点球未能命中。"), *Taker);
			}
			return !GoalkeeperName.IsEmpty()
				? FString::Printf(TEXT("%s点球被%s扑出！"),
					*Taker, *GoalkeeperName)
				: FString::Printf(TEXT("%s点球未能命中。"), *Taker);

		default:
			return FString::Printf(TEXT("%s · %s"),
				*MethodLabel, *ResultTitle);
		}
	}

	FFMCodexUMGInlineFormulaSurfaceViewModel BuildSetPieceFormulaSurface(
		const FFMCodexLocalMatchInteractionView& InteractionView,
		const FFMCodexUMGPrimaryActionViewModel& PrimaryAction)
	{
		FFMCodexUMGInlineFormulaSurfaceViewModel Result;
		if (InteractionView.RouteKind
			!= EMatchPlayCurrentAttackRouteKind::SetPiece)
		{
			return Result;
		}

		const bool bTypeRollPending =
			InteractionView.InteractionCategory == EFMCodexLocalMatchInteractionCategory::RollSetPieceType;
		if (InteractionView.InteractionCategory == EFMCodexLocalMatchInteractionCategory::RollCornerRoute)
		{
			Result.bVisible = true;
			Result.bSuppressLegacyResolution = true;
			Result.ContestId = TEXT("Corner.Route");
			Result.ContestLabel = TEXT("角球路线判定");
			Result.StatusLabel = InteractionView.CornerIntendedRoute == EMatchPlayCornerRouteIntent::High
				? TEXT("选择高球 · 等待路线掷点") : TEXT("选择低平球 · 等待路线掷点");
			Result.RollHelperLabel = FFMCodexTacticalDetailPresentationBuilder::BuildCornerRouteHint(
				InteractionView.CornerIntendedRoute).ToString();
			Result.bShowFormulaRows = false;
			Result.bShowAttackRow = false;
			Result.bShowDefenseRow = false;
			ClaimPrimaryAction(Result.PrimaryAction, PrimaryAction);
			Result.bCanContinue = Result.PrimaryAction.bVisible;
			Result.ContinueActionLabel = Result.PrimaryAction.Action.Label;
			return Result;
		}
		if (bTypeRollPending || InteractionView.bHasSetPieceTypeRoll)
		{
			Result.bVisible = bTypeRollPending;
			Result.bSuppressLegacyResolution = true;
			Result.ContestId = TEXT("SetPiece.Type");
			Result.ContestLabel = TEXT("定位球类型");
			Result.StatusLabel = bTypeRollPending
				? TEXT("掷一次 D6 决定定位球类型")
				: TEXT("定位球类型已确认");
			Result.TacticalPlayerSummaryLabel =
				FFMCodexPlayerUIPresentationText::SetPieceTypeRollHint().ToString();
			Result.bShowFormulaRows = false;
			Result.bShowAttackRow = false;
			Result.bShowDefenseRow = false;
			if (InteractionView.bHasSetPieceTypeRoll)
			{
				Result.RouteResultLabel = FString::Printf(
					TEXT("掷点 %d → %s"), InteractionView.RawSetPieceTypeD6,
					*SetPieceTypeLabel(InteractionView.SetPieceType));
			}
			ClaimPrimaryAction(Result.PrimaryAction, PrimaryAction);
			Result.bCanContinue = Result.PrimaryAction.bVisible;
			Result.ContinueActionLabel = Result.PrimaryAction.Action.Label;
			if (bTypeRollPending
				|| !IsSetPieceOrdinaryResolutionCategory(
					InteractionView.InteractionCategory)
					&& !InteractionView.bTerminalPendingAdvance)
			{
				return Result;
			}
		}

		if (!IsSetPieceOrdinaryResolutionCategory(
				InteractionView.InteractionCategory)
			&& !InteractionView.bTerminalPendingAdvance)
		{
			return Result;
		}
		// Type mapping is reveal-only context. Once the route owns an ordinary
		// resolution screen, its title already communicates the selected route.
		Result.RouteResultLabel.Reset();
		Result.TacticalPlayerSummaryLabel.Reset();

		const bool bAttackPending =
			InteractionView.InteractionCategory
				== EFMCodexLocalMatchInteractionCategory
					::RollShortFreeKickDirectAttack
			|| InteractionView.InteractionCategory
				== EFMCodexLocalMatchInteractionCategory
					::RollLongFreeKickDirectAttack
			|| InteractionView.InteractionCategory
				== EFMCodexLocalMatchInteractionCategory
					::RollPenaltyDirectAttack
			|| InteractionView.InteractionCategory
				== EFMCodexLocalMatchInteractionCategory::RollCornerAttack;
		const bool bDefensePending =
			InteractionView.InteractionCategory
				== EFMCodexLocalMatchInteractionCategory
					::RollShortFreeKickDirectDefense
			|| InteractionView.InteractionCategory
				== EFMCodexLocalMatchInteractionCategory
					::RollLongFreeKickDirectDefense
			|| InteractionView.InteractionCategory
				== EFMCodexLocalMatchInteractionCategory
					::RollPenaltyDirectDefense
			|| InteractionView.InteractionCategory
				== EFMCodexLocalMatchInteractionCategory::RollCornerDefense;
		const bool bCompactPending =
			InteractionView.InteractionCategory
				== EFMCodexLocalMatchInteractionCategory
					::RollShortFreeKickAngled
			|| InteractionView.InteractionCategory
				== EFMCodexLocalMatchInteractionCategory
					::RollLongFreeKickPower
			|| InteractionView.InteractionCategory
				== EFMCodexLocalMatchInteractionCategory::RollPenaltyPanenka;
		const bool bCompactMethod = bCompactPending
			|| InteractionView.bHasSetPiecePairedD6;
		const bool bOpposed = bAttackPending || bDefensePending
			|| InteractionView.bHasSetPieceFormula
			|| (InteractionView.bHasSetPieceAttackD6 && !bCompactMethod);

		Result.bVisible = true;
		Result.bSuppressLegacyResolution = true;
		Result.ContestId = bCompactMethod
			? FName(TEXT("SetPiece.Compact"))
			: FName(TEXT("SetPiece.Opposed"));
		const FString MethodLabel = SetPieceContestLabel(
			InteractionView, bCompactMethod);
		Result.ContestLabel = MethodLabel;
		Result.bShowFormulaRows = bOpposed;
		Result.bShowAttackRow = bOpposed;
		Result.bShowDefenseRow = bOpposed
			&& (!InteractionView.bTerminalPendingAdvance
				|| InteractionView.bHasSetPieceDefenseD6
				|| InteractionView.bHasSetPieceFormula);
		Result.bAttackRowActive = bAttackPending;
		Result.bDefenseRowActive = bDefensePending;

		const FString OutcomeLabel = SetPieceOutcomeLabel(InteractionView);
		if (bAttackPending)
		{
			Result.StatusLabel = TEXT("等待进攻方掷点");
			if (InteractionView.SetPieceType == ESetPieceSelectedType::LongFreeKick)
			{
				Result.RollHelperLabel = FFMCodexPlayerUIPresentationText
					::LongFreeKickDirectOutcomeHint().ToString();
			}
		}
		else if (bDefensePending)
		{
			Result.StatusLabel = InteractionView.bHasSetPieceAttackD6
				? FString::Printf(TEXT("进攻方掷点 %d 已确认 · 等待防守方掷点"),
					InteractionView.SetPieceAttackD6)
				: FString(TEXT("等待防守方掷点"));
		}
		else if (bCompactPending)
		{
			Result.StatusLabel = InteractionView.SetPieceType
				== ESetPieceSelectedType::Penalty
					? TEXT("等待掷点") : TEXT("等待两枚骰子结果");
			Result.RollHelperLabel = FFMCodexPlayerUIPresentationText
				::SetPieceCompactOutcomeHint(InteractionView.SetPieceType).ToString();
		}
		else
		{
			Result.StatusLabel = OutcomeLabel;
		}
		if (!OutcomeLabel.IsEmpty() && InteractionView.bTerminalPendingAdvance)
		{
			// Match the mature ordinary tactical hierarchy: the football event is
			// the large heading; method/outcome and Formula audit remain secondary.
			Result.bNarrativeAvailable = true;
			Result.bNarrativeAttackSuccess = InteractionView.bSetPieceGoal;
			Result.ResultTitle = InteractionView.bSetPieceGoal ? TEXT("进球") : TEXT("未进球");
			Result.NarrativeHeadline = SetPieceTerminalSupportLine(
				InteractionView, bCompactMethod, MethodLabel,
				Result.ResultTitle);
			Result.ResultSubtitle = MethodLabel;
			Result.ContestLabel = Result.NarrativeHeadline;
			Result.StatusLabel = FString::Printf(TEXT("%s · %s"), *MethodLabel, *Result.ResultTitle);
		}

		if (InteractionView.bHasSetPieceFormula)
		{
			const FString Winner = InteractionView.SetPieceFormula.Winner
				== EFormulaWinner::Attacker
					? TEXT("进攻方胜")
					: InteractionView.SetPieceFormula.Winner
						== EFormulaWinner::Defender
							? TEXT("防守方胜") : TEXT("结果已确认");
			Result.RouteResultLabel = FString::Printf(
				TEXT("进攻 %s vs 防守 %s · %s"),
				*CompactNumber(
					InteractionView.SetPieceFormula.AttackerFinalValue),
				*CompactNumber(
					InteractionView.SetPieceFormula.DefenderFinalValue),
				*Winner);
		}
		else if (InteractionView.bHasSetPiecePairedD6)
		{
			Result.RouteResultLabel = InteractionView.SetPieceType
				== ESetPieceSelectedType::Penalty
					? FString::Printf(TEXT("掷点结果：%d"),
						InteractionView.SetPiecePairedD6A)
					: FFMCodexPlayerUIPresentationText::PairedRollResult(
						InteractionView.SetPiecePairedD6A,
						InteractionView.SetPiecePairedD6B).ToString();
		}
		else if (InteractionView.bHasSetPieceAttackD6
			&& InteractionView.bTerminalPendingAdvance)
		{
			Result.RouteResultLabel = FString::Printf(
				TEXT("进攻方掷点：%d"), InteractionView.SetPieceAttackD6);
		}

		if (bOpposed)
		{
			const EInitialTurnOrderPlayer Attacker =
				InteractionView.CurrentAttackingPlayer;
			const EInitialTurnOrderPlayer Defender = OtherSide(Attacker);
			Result.AttackRow.Side = Attacker;
			Result.AttackRow.SideLabel = TEXT("进攻方");
			Result.DefenseRow.Side = Defender;
			Result.DefenseRow.SideLabel = TEXT("防守方");
			if (InteractionView.SetPieceType == ESetPieceSelectedType::Corner)
			{
				AddSetPieceParticipant(Result.AttackRow, InteractionView,
					InteractionView.CornerRunner, TEXT("进攻球员"));
				AddSetPieceParticipant(Result.DefenseRow, InteractionView,
					InteractionView.CornerHelper, TEXT("防守球员"));
			}
			else
			{
				AddSetPieceParticipant(Result.AttackRow, InteractionView,
					InteractionView.SetPieceCarrier, TEXT("主罚球员"));
				AddSetPieceGoalkeeperParticipant(
					Result.DefenseRow, InteractionView, Defender);
			}

			const FPlayerCardRuleSnapshot& CarrierSnapshot =
				InteractionView.SetPieceCarrier.Snapshot;
			const FFMCodexLocalMatchCardView* Goalkeeper =
				FindSetPieceGoalkeeper(InteractionView, Defender);
			switch (InteractionView.SetPieceType)
			{
			case ESetPieceSelectedType::ShortFreeKick:
				Result.AttackRow.Terms.Add(SetPieceAttributeTerm(FString::Printf(
					TEXT("射门 %d / 传球 %d（取较高）"),
					CarrierSnapshot.Attributes.Shooting,
					CarrierSnapshot.Attributes.Passing)));
				Result.DefenseRow.Terms.Add(SetPieceAttributeTerm(FString::Printf(
					TEXT("%s %d"),
					*FFMCodexPlayerUIPresentationText::AttributeLabel(TEXT("HAN")).ToString(),
					SetPieceCardAttribute(Goalkeeper, TEXT("HAN")))));
				Result.DefenseRow.Terms.Add(SetPieceModifierTerm(
					1, TEXT("防守加成 1")));
				Result.AttackRow.KnownNonRollSubtotalLabel =
					TEXT("射门与传球取较高值");
				Result.DefenseRow.KnownNonRollSubtotalLabel =
					FString::Printf(TEXT("门将%s与防守加成"),
						*FFMCodexPlayerUIPresentationText::AttributeLabel(TEXT("HAN")).ToString());
				break;
			case ESetPieceSelectedType::LongFreeKick:
				Result.AttackRow.Terms.Add(SetPieceAttributeTerm(FString::Printf(
					TEXT("远射 %d"), CarrierSnapshot.Attributes.LongShot)));
				Result.DefenseRow.Terms.Add(SetPieceAttributeTerm(FString::Printf(
					TEXT("站位 %d"),
					SetPieceCardAttribute(Goalkeeper, TEXT("POS")))));
				Result.DefenseRow.Terms.Add(SetPieceModifierTerm(
					2, TEXT("防守加成 2")));
				Result.AttackRow.KnownNonRollSubtotalLabel = TEXT("主罚球员远射");
				Result.DefenseRow.KnownNonRollSubtotalLabel =
					TEXT("门将站位与防守加成");
				break;
			case ESetPieceSelectedType::Penalty:
				Result.AttackRow.Terms.Add(SetPieceAttributeTerm(FString::Printf(
					TEXT("射门 %d / 传球 %d（取较高）"),
					CarrierSnapshot.Attributes.Shooting,
					CarrierSnapshot.Attributes.Passing)));
				Result.DefenseRow.Terms.Add(SetPieceAttributeTerm(FString::Printf(
					TEXT("预判 %d"),
					SetPieceCardAttribute(Goalkeeper, TEXT("ANT")))));
				Result.DefenseRow.Terms.Add(SetPieceModifierTerm(
					-3, TEXT("点球防守调整 -3")));
				Result.AttackRow.KnownNonRollSubtotalLabel =
					TEXT("射门与传球取较高值");
				Result.DefenseRow.KnownNonRollSubtotalLabel =
					TEXT("门将预判与点球调整");
				break;
			case ESetPieceSelectedType::Corner:
			{
				const bool bHigh = InteractionView.CornerActualRoute
					== EMatchPlayCornerRouteIntent::High;
				const FString AttackBasis = bHigh
					? FString::Printf(TEXT("力量 %d"),
						InteractionView.CornerRunner.Snapshot.Attributes.Strength)
					: FString::Printf(TEXT("射门 %d"),
						InteractionView.CornerRunner.Snapshot.Attributes.Shooting);
				const FString DefenseBasis = bHigh
					? FString::Printf(TEXT("防守力量 %d / 门将制空 %d（取平均）"),
						InteractionView.CornerHelper.Snapshot.Attributes.Strength,
						SetPieceCardAttribute(Goalkeeper, TEXT("AER")))
					: FString::Printf(TEXT("盯防 %d / 门将反应 %d（取平均）"),
						InteractionView.CornerHelper.Snapshot.Attributes.Marking,
						SetPieceCardAttribute(Goalkeeper, TEXT("REF")));
				Result.AttackRow.Terms.Add(SetPieceAttributeTerm(AttackBasis));
				Result.DefenseRow.Terms.Add(SetPieceAttributeTerm(DefenseBasis));
				Result.DefenseRow.Terms.Add(SetPieceModifierTerm(2, TEXT("防守加成 2")));
				if (InteractionView.CornerCandidateBonus > 0)
				{
					(InteractionView.CornerCandidateBonusSide == Attacker
						? Result.AttackRow : Result.DefenseRow).Terms.Add(
							SetPieceModifierTerm(
								InteractionView.CornerCandidateBonus,
								*FFMCodexPlayerUIPresentationText::CornerCandidateBonus(InteractionView.CornerCandidateBonus).ToString()));
				}
				Result.AttackRow.KnownNonRollSubtotalLabel = bHigh
					? TEXT("进攻球员力量") : TEXT("进攻球员射门");
				Result.DefenseRow.KnownNonRollSubtotalLabel = bHigh
					? TEXT("防守球员力量与门将制空取平均，再加防守加成 2")
					: TEXT("防守球员盯防与门将反应取平均，再加防守加成 2");
				break;
			}
			default:
				break;
			}

			Result.AttackRow.Terms.Add(SetPieceRollTerm(
				InteractionView.bHasSetPieceAttackD6,
				InteractionView.SetPieceAttackD6, 0, bAttackPending));
			Result.DefenseRow.Terms.Add(SetPieceRollTerm(
				InteractionView.bHasSetPieceDefenseD6,
				InteractionView.SetPieceDefenseD6, 1, bDefensePending));
			if (InteractionView.bHasSetPieceAttackKnownSubtotal)
			{
				Result.AttackRow.bKnownNonRollSubtotalResolved = true;
				Result.AttackRow.KnownNonRollSubtotal =
					InteractionView.SetPieceAttackKnownSubtotal;
			}
			if (InteractionView.bHasSetPieceDefenseKnownSubtotal)
			{
				Result.DefenseRow.bKnownNonRollSubtotalResolved = true;
				Result.DefenseRow.KnownNonRollSubtotal =
					InteractionView.SetPieceDefenseKnownSubtotal;
			}
			if (InteractionView.bHasSetPieceAttackCurrentTotal)
			{
				Result.AttackRow.bDisplayedResultResolved = true;
				Result.AttackRow.DisplayedResult =
					InteractionView.SetPieceAttackCurrentTotal;
				Result.AttackRow.DisplayedResultLabel = CompactNumber(
					Result.AttackRow.DisplayedResult);
			}
			if (InteractionView.bHasSetPieceDefenseCurrentTotal)
			{
				Result.DefenseRow.bDisplayedResultResolved = true;
				Result.DefenseRow.DisplayedResult =
					InteractionView.SetPieceDefenseCurrentTotal;
				Result.DefenseRow.DisplayedResultLabel = CompactNumber(
					Result.DefenseRow.DisplayedResult);
			}
			if (InteractionView.bHasSetPieceFormula)
			{
				Result.AttackRow.bFinalValueResolved = true;
				Result.AttackRow.FinalValue =
					InteractionView.SetPieceFormula.AttackerFinalValue;
				Result.AttackRow.FinalValueLabel = CompactNumber(
					Result.AttackRow.FinalValue);
				Result.AttackRow.bDisplayedResultResolved = true;
				Result.AttackRow.bDisplayedResultIsFinalValue = true;
				Result.AttackRow.DisplayedResult = Result.AttackRow.FinalValue;
				Result.AttackRow.DisplayedResultLabel =
					Result.AttackRow.FinalValueLabel;

				Result.DefenseRow.bFinalValueResolved = true;
				Result.DefenseRow.FinalValue =
					InteractionView.SetPieceFormula.DefenderFinalValue;
				Result.DefenseRow.FinalValueLabel = CompactNumber(
					Result.DefenseRow.FinalValue);
				Result.DefenseRow.bDisplayedResultResolved = true;
				Result.DefenseRow.bDisplayedResultIsFinalValue = true;
				Result.DefenseRow.DisplayedResult = Result.DefenseRow.FinalValue;
				Result.DefenseRow.DisplayedResultLabel =
					Result.DefenseRow.FinalValueLabel;
			}
		}

		ClaimPrimaryAction(Result.PrimaryAction, PrimaryAction);
		Result.bCanContinue = Result.PrimaryAction.bVisible;
		Result.ContinueActionLabel = Result.PrimaryAction.Action.Label;
		return Result;
	}

	FFMCodexUMGInlineFormulaSurfaceViewModel BuildInlineFormulaSurface(
		const FMatchPlayCurrentAttackResolutionFactProjection& Facts,
		const FFMCodexLocalMatchInteractionView& InteractionView,
		const FFMCodexUMGPrimaryActionViewModel& PrimaryAction,
		const bool bAcceptedResolutionState)
	{
		FFMCodexUMGInlineFormulaSurfaceViewModel Result;
		const bool bResolvedCross = bAcceptedResolutionState
			&& Facts.bSuccess && Facts.bHasFacts
			&& Facts.ActionType == ESkillRuleType::Cross
			&& Facts.bHasActualBranch
			&& Facts.ActualBranch.ActionType == ESkillRuleType::Cross;
		const bool bResolvedThroughBallFeet = bAcceptedResolutionState
			&& Facts.bSuccess && Facts.bHasFacts
			&& Facts.ActionType == ESkillRuleType::ThroughBall
			&& Facts.bHasActualBranch
			&& Facts.ActualBranch.ActionType == ESkillRuleType::ThroughBall
			&& Facts.ActualBranch.ThroughBall
				== EMatchPlayThroughBallActualBranch::Feet;
		const bool bResolvedThroughBallDirect = bAcceptedResolutionState
			&& Facts.bSuccess && Facts.bHasFacts
			&& Facts.ActionType == ESkillRuleType::ThroughBall
			&& Facts.FormulaContests.ContainsByPredicate(
				[](const FMatchPlayResolutionFormulaContestFact& Contest)
				{
					return Contest.ContestId
						== FName(TEXT("ThroughBall.OneOnOne.DirectShot"));
				});
		const bool bResolvedThroughBallBehind = bAcceptedResolutionState
			&& Facts.bSuccess && Facts.bHasFacts
			&& Facts.ActionType == ESkillRuleType::ThroughBall
			&& Facts.bHasActualBranch
			&& Facts.ActualBranch.ActionType == ESkillRuleType::ThroughBall
			&& Facts.ActualBranch.ThroughBall
				== EMatchPlayThroughBallActualBranch::BehindDefense
			&& !bResolvedThroughBallDirect;
		const bool bResolvedLongShotDirect = bAcceptedResolutionState
			&& Facts.bSuccess && Facts.bHasFacts
			&& Facts.ActionType == ESkillRuleType::LongShot
			&& Facts.bHasActualBranch
			&& Facts.ActualBranch.ActionType == ESkillRuleType::LongShot
			&& Facts.ActualBranch.LongShot
				== EMatchPlayLongShotActualBranch::DirectShot;
		const bool bResolvedCutInsideDirect = bAcceptedResolutionState
			&& Facts.bSuccess && Facts.bHasFacts
			&& Facts.ActionType == ESkillRuleType::CutInsideShot
			&& Facts.bHasActualBranch
			&& Facts.ActualBranch.ActionType == ESkillRuleType::CutInsideShot
			&& Facts.ActualBranch.CutInsideShot
				== EMatchPlayCutInsideShotActualBranch::DirectShot;
		const bool bResolvedPassControl = bAcceptedResolutionState
			&& Facts.bSuccess && Facts.bHasFacts
			&& Facts.ActionType == ESkillRuleType::PassControl
			&& Facts.bHasActualBranch
			&& Facts.ActualBranch.ActionType == ESkillRuleType::PassControl
			&& Facts.ActualBranch.PassControl
				!= EMatchPlayPassControlActualBranch::None;
		const bool bResolvedElectiveDirect = bResolvedLongShotDirect
			|| bResolvedCutInsideDirect;
		if (!bResolvedCross && !bResolvedThroughBallFeet
			&& !bResolvedThroughBallBehind && !bResolvedThroughBallDirect
			&& !bResolvedElectiveDirect && !bResolvedPassControl)
		{
			return Result;
		}

		const FMatchPlayResolutionRollFact* RouteRoll =
			Facts.Rolls.FindByPredicate(
				[](const FMatchPlayResolutionRollFact& Roll)
				{
					return Roll.bInitialRoute && Roll.bResolved
						&& Roll.Semantics
							== EMatchPlayResolutionRollSemantics::BranchSelection;
				});
		if (RouteRoll == nullptr && !bResolvedElectiveDirect)
		{
			return Result;
		}
		const bool bCrossHigh = bResolvedCross
			&& Facts.ActualBranch.Cross == EMatchPlayCrossActualBranch::High;
		if (RouteRoll != nullptr)
		{
			const FString RouteName = bResolvedCross
				? (bCrossHigh ? TEXT("高球传中") : TEXT("低球传中"))
				: bResolvedPassControl
					? Facts.ActualBranch.PassControl
						== EMatchPlayPassControlActualBranch::PassAdvance
							? TEXT("传球推进")
							: Facts.ActualBranch.PassControl
								== EMatchPlayPassControlActualBranch::DribbleAdvance
									? TEXT("盘带推进") : TEXT("跑动推进")
					: bResolvedThroughBallFeet ? TEXT("脚下球")
						: Facts.ActualBranch.ThroughBall
							== EMatchPlayThroughBallActualBranch::AntiOffside
								? TEXT("反越位") : TEXT("身后球");
			Result.RouteResultLabel = FString::Printf(
				TEXT("路线掷点 %d \u2192 判定为%s"),
				RouteRoll->RawD6, *RouteName);
		}

		const FName ContestId = bResolvedCross
			? (bCrossHigh ? FName(TEXT("Cross.High"))
				: FName(TEXT("Cross.Low")))
			: bResolvedThroughBallDirect
				? FName(TEXT("ThroughBall.OneOnOne.DirectShot"))
			: bResolvedLongShotDirect
				? FName(TEXT("LongShot.DirectShot"))
			: bResolvedCutInsideDirect
				? FName(TEXT("CutInsideShot.DirectShot"))
			: bResolvedPassControl
				? Facts.ActualBranch.PassControl
					== EMatchPlayPassControlActualBranch::PassAdvance
						? FName(TEXT("PassControl.PassAdvance"))
						: Facts.ActualBranch.PassControl
							== EMatchPlayPassControlActualBranch::DribbleAdvance
								? FName(TEXT("PassControl.DribbleAdvance"))
								: FName(TEXT("PassControl.RunAdvance"))
			: bResolvedThroughBallFeet
				? FName(TEXT("ThroughBall.Feet"))
				: FName(TEXT("ThroughBall.BehindDefense.P1"));
		const FMatchPlayResolutionFormulaContestFact* Contest =
			Facts.FormulaContests.FindByPredicate(
				[ContestId](
					const FMatchPlayResolutionFormulaContestFact& Candidate)
				{
					return Candidate.ContestId == ContestId;
				});
		if (Contest == nullptr)
		{
			return Result;
		}
		auto RowHasArithmeticRoll = [&Facts](
			const FMatchPlayResolutionFormulaRowFact& Row)
		{
			return Row.Terms.ContainsByPredicate(
				[&Facts](const FMatchPlayResolutionFormulaTermFact& Term)
				{
					return Term.Kind
						== EMatchPlayResolutionFormulaTermKind::RawRoll
						&& Facts.Rolls.ContainsByPredicate(
							[&Term](
								const FMatchPlayResolutionRollFact& Roll)
							{
								return Roll.SequenceIndex
									== Term.RollSequenceIndex
									&& Roll.Semantics
										== EMatchPlayResolutionRollSemantics
											::ArithmeticContest;
							});
				});
		};
		if (!RowHasArithmeticRoll(Contest->AttackRow)
			|| !RowHasArithmeticRoll(Contest->DefenseRow))
		{
			return Result;
		}

		Result.bVisible = true;
		Result.bSuppressLegacyResolution = true;
		Result.ContestId = Contest->ContestId;
		Result.ContestLabel = FFMCodexPlayerUIPresentationText
			::ResolutionContest(Contest->ContestId).ToString();
		Result.AttackRow = BuildFormulaRow(
			Contest->AttackRow,
			FFMCodexPlayerUIPresentationText::ResolutionAttackRow().ToString(),
			Facts,
			InteractionView);
		Result.DefenseRow = BuildFormulaRow(
			Contest->DefenseRow,
			FFMCodexPlayerUIPresentationText::ResolutionDefenseRow().ToString(),
			Facts,
			InteractionView);
		const bool bAttackResolved =
			Result.AttackRow.bFinalValueResolved;
		const bool bDefenseResolved =
			Result.DefenseRow.bFinalValueResolved;
		const FMatchPlayResolutionDecisionFact* BehindDecision =
			bResolvedThroughBallBehind
				? Facts.Decisions.FindByPredicate(
					[](const FMatchPlayResolutionDecisionFact& Decision)
					{
						return Decision.DecisionId
							== FName(TEXT(
								"ThroughBall.BehindDefense.P1.Outcome"));
					})
				: nullptr;
		const bool bBehindOutcomeResolved = BehindDecision != nullptr
			&& BehindDecision->bResolved
			&& BehindDecision->Outcome
				!= EMatchPlayResolutionDecisionOutcome::None;
		const bool bBehindOutOfPlay = bBehindOutcomeResolved
			&& BehindDecision->Outcome
				== EMatchPlayResolutionDecisionOutcome::OutOfPlay;
		const FMatchPlayResolutionDecisionFact* DirectDecision =
			bResolvedThroughBallDirect
				? Facts.Decisions.FindByPredicate(
					[](const FMatchPlayResolutionDecisionFact& Decision)
					{
						return Decision.DecisionId == FName(TEXT(
							"ThroughBall.OneOnOne.DirectShot.Outcome"));
					})
				: nullptr;
		const bool bDirectOutcomeResolved = DirectDecision != nullptr
			&& DirectDecision->bResolved
			&& DirectDecision->Outcome
				!= EMatchPlayResolutionDecisionOutcome::None;
		const FMatchPlayResolutionDecisionFact* ElectiveDirectDecision =
			bResolvedElectiveDirect
				? Facts.Decisions.FindByPredicate(
					[ContestId](const FMatchPlayResolutionDecisionFact& Decision)
					{
						return Decision.DecisionId == FName(*FString::Printf(
							TEXT("%s.Outcome"), *ContestId.ToString()));
					})
				: nullptr;
		const bool bElectiveDirectOutcomeResolved =
			ElectiveDirectDecision != nullptr
			&& ElectiveDirectDecision->bResolved
			&& ElectiveDirectDecision->Outcome
				!= EMatchPlayResolutionDecisionOutcome::None;
		const bool bElectiveDirectImmediateMiss =
			bElectiveDirectOutcomeResolved
			&& ElectiveDirectDecision->Outcome
				== EMatchPlayResolutionDecisionOutcome::ImmediateMiss;
		Result.bShowFormulaRows = !bBehindOutOfPlay
			&& !bElectiveDirectImmediateMiss;
		Result.StatusLabel = !bAttackResolved
			? TEXT("等待进攻方掷点")
			: !bDefenseResolved
				? TEXT("等待防守方掷点")
				: TEXT("双方掷点已完成");
		const bool bTerminalPresentationReady = bResolvedElectiveDirect
			? bElectiveDirectOutcomeResolved
				&& InteractionView.bTerminalPendingAdvance
				&& PrimaryAction.bAvailable
			: bResolvedThroughBallDirect
			? bDirectOutcomeResolved
				&& InteractionView.bTerminalPendingAdvance
				&& PrimaryAction.bAvailable
			: bResolvedPassControl
			? Contest->bHasResolvedFormula
				&& InteractionView.bTerminalPendingAdvance
				&& PrimaryAction.bAvailable
			: bResolvedCross
			? InteractionView.bCrossFormulaComplete
				&& (InteractionView.bCrossTerminalActionAvailable
					|| InteractionView.bTerminalPendingAdvance)
			: InteractionView.bThroughBallFeetFormulaComplete
				&& (InteractionView.bThroughBallFeetTerminalActionAvailable
					|| InteractionView.bTerminalPendingAdvance);
		const bool bBehindNarrativeReady = bResolvedThroughBallBehind
			&& bAttackResolved && bBehindOutcomeResolved
			&& (bBehindOutOfPlay
				? InteractionView.bTerminalPendingAdvance
					&& PrimaryAction.bAvailable
				: bDefenseResolved && Contest->bHasResolvedFormula
					&& (BehindDecision->Outcome
							== EMatchPlayResolutionDecisionOutcome
								::OneOnOneRequired
						|| (BehindDecision->Outcome
								== EMatchPlayResolutionDecisionOutcome
									::DefenderStoppedAttack
							&& InteractionView.bTerminalPendingAdvance
							&& PrimaryAction.bAvailable)));
		const bool bNarrativeReady = bResolvedElectiveDirect
			&& bElectiveDirectImmediateMiss
			? bAttackResolved && bTerminalPresentationReady
				&& PrimaryAction.bAvailable
			: bResolvedThroughBallBehind
			? bBehindNarrativeReady
			: bAttackResolved && bDefenseResolved
				&& Contest->bHasResolvedFormula
				&& (bResolvedElectiveDirect
					? bElectiveDirectOutcomeResolved
					: bResolvedThroughBallDirect
					? bDirectOutcomeResolved
					: Contest->ResolvedResult.Winner != EFormulaWinner::None)
				&& Contest->ResolvedResult.bAttackEnded
				&& !Contest->ResolvedResult.bContinueResolution
				&& bTerminalPresentationReady
				&& PrimaryAction.bAvailable;
		if (bNarrativeReady)
		{
			Result.bNarrativeAttackSuccess = bResolvedElectiveDirect
				? ElectiveDirectDecision->Outcome
					== EMatchPlayResolutionDecisionOutcome::Goal
				: bResolvedThroughBallDirect
				? DirectDecision->Outcome
					== EMatchPlayResolutionDecisionOutcome::Goal
				: bResolvedThroughBallBehind
				? BehindDecision->Outcome
					== EMatchPlayResolutionDecisionOutcome::OneOnOneRequired
				: Contest->ResolvedResult.Winner == EFormulaWinner::Attacker;
			FFMCodexTacticalNarrativePresentationInput NarrativeInput;
			NarrativeInput.Branch = bResolvedLongShotDirect
				? EFMCodexTacticalNarrativeBranch::LongShotDirect
				: bResolvedCutInsideDirect
				? EFMCodexTacticalNarrativeBranch::CutInsideDirect
				: bResolvedThroughBallDirect
				? EFMCodexTacticalNarrativeBranch::ThroughBallOneOnOneDirect
				: bResolvedThroughBallBehind
				? EFMCodexTacticalNarrativeBranch::ThroughBallBehindDefense
				: bResolvedPassControl
				? Facts.ActualBranch.PassControl
					== EMatchPlayPassControlActualBranch::PassAdvance
						? EFMCodexTacticalNarrativeBranch::PassControlPassAdvance
						: Facts.ActualBranch.PassControl
							== EMatchPlayPassControlActualBranch::DribbleAdvance
								? EFMCodexTacticalNarrativeBranch
									::PassControlDribbleAdvance
								: EFMCodexTacticalNarrativeBranch
									::PassControlRunAdvance
				: !bResolvedCross
					? EFMCodexTacticalNarrativeBranch::ThroughBallFeet
				: bCrossHigh
					? EFMCodexTacticalNarrativeBranch::CrossHigh
					: EFMCodexTacticalNarrativeBranch::CrossLow;
			NarrativeInput.AuthorityOutcome = bResolvedElectiveDirect
				? ElectiveDirectDecision->Outcome
				: bResolvedThroughBallDirect
				? DirectDecision->Outcome
				: bResolvedThroughBallBehind
				? BehindDecision->Outcome
				: Result.bNarrativeAttackSuccess
					? EMatchPlayResolutionDecisionOutcome::Goal
					: EMatchPlayResolutionDecisionOutcome::Miss;
			NarrativeInput.AttackSequence = Facts.AttackSequence;
			NarrativeInput.StableEventId = Contest->ContestId;
			NarrativeInput.Carrier = NarrativeActor(Facts, InteractionView,
				EMatchPlayResolutionParticipantRole::Carrier);
			NarrativeInput.Runner = NarrativeActor(Facts, InteractionView,
				EMatchPlayResolutionParticipantRole::Runner);
			NarrativeInput.Marker = NarrativeActor(Facts, InteractionView,
				EMatchPlayResolutionParticipantRole::Marker);
			NarrativeInput.Helper = NarrativeActor(Facts, InteractionView,
				EMatchPlayResolutionParticipantRole::Helper);
			NarrativeInput.Goalkeeper = NarrativeActor(Facts, InteractionView,
				EMatchPlayResolutionParticipantRole::Goalkeeper);

			const FFMCodexTacticalNarrativePresentation Narrative =
				FFMCodexTacticalResolutionNarrativePresentationBuilder::Build(
					NarrativeInput);
			if (Narrative.bSuccess)
			{
				Result.bNarrativeAvailable = Narrative.bNarrativeAvailable;
				Result.ResultTitle = Narrative.ResultTitle.ToString();
				Result.NarrativeHeadline = Narrative.NarrativeText.ToString();
				const FString RouteLabel = bResolvedElectiveDirect
					? TEXT("直接射门")
					: bResolvedThroughBallDirect
					? TEXT("单刀")
					: bResolvedThroughBallBehind
					? TEXT("身后球")
					: bResolvedPassControl
					? Facts.ActualBranch.PassControl
						== EMatchPlayPassControlActualBranch::PassAdvance
							? TEXT("传球推进")
							: Facts.ActualBranch.PassControl
								== EMatchPlayPassControlActualBranch::DribbleAdvance
									? TEXT("盘带推进") : TEXT("跑动推进")
					: !bResolvedCross ? TEXT("脚下球")
					: bCrossHigh ? TEXT("高球传中") : TEXT("低球传中");
				Result.ResultSubtitle = FString::Printf(
					TEXT("%s · %s"), *RouteLabel, *Result.ResultTitle);
				if (Narrative.DefensivePerformerRole
					== EMatchPlayResolutionParticipantRole::Marker)
				{
					Result.DefensiveNarrativePerformer =
						EFMCodexUMGCrossDefensiveNarrativePerformer::Marker;
				}
				else if (Narrative.DefensivePerformerRole
					== EMatchPlayResolutionParticipantRole::Helper)
				{
					Result.DefensiveNarrativePerformer =
						EFMCodexUMGCrossDefensiveNarrativePerformer::Helper;
				}
				Result.ContestLabel = Result.NarrativeHeadline;
				Result.StatusLabel = Result.ResultSubtitle;
			}
		}
		Result.bAttackRowActive = !bAttackResolved;
		if (bResolvedThroughBallBehind && InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::RollThroughBallBehindDefenseAttack)
		{
			Result.RollHelperLabel = FFMCodexPlayerUIPresentationText
				::ThroughBallBehindDefenseOutcomeHint().ToString();
		}
		Result.bDefenseRowActive = !bBehindOutOfPlay
			&& bAttackResolved && !bDefenseResolved;
		if (CanFormulaSurfaceClaim(PrimaryAction.Category))
		{
			ClaimPrimaryAction(Result.PrimaryAction, PrimaryAction);
		}
		Result.bCanContinue = Result.PrimaryAction.bVisible;
		Result.ContinueActionLabel = Result.PrimaryAction.Action.Label;
		return Result;
	}

	EFMCodexUMGThroughBallRoute ThroughBallRoute(
		const EMatchPlayThroughBallActualBranch Route)
	{
		switch (Route)
		{
		case EMatchPlayThroughBallActualBranch::Feet:
			return EFMCodexUMGThroughBallRoute::Feet;
		case EMatchPlayThroughBallActualBranch::BehindDefense:
			return EFMCodexUMGThroughBallRoute::BehindDefense;
		case EMatchPlayThroughBallActualBranch::AntiOffside:
			return EFMCodexUMGThroughBallRoute::AntiOffside;
		default:
			return EFMCodexUMGThroughBallRoute::None;
		}
	}

	FFMCodexUMGThroughBallResolutionViewModel BuildThroughBallSurface(
		const FFMCodexLocalMatchInteractionView& InteractionView,
		const FFMCodexUMGInteractionViewModel& Interaction,
		const FFMCodexUMGInlineFormulaSurfaceViewModel& Formula,
		const bool bRejected)
	{
		FFMCodexUMGThroughBallResolutionViewModel Result;
		const bool bInitialRoutePending =
			InteractionView.bCurrentAttackActive
			&& InteractionView.PresentedActionType == ESkillRuleType::ThroughBall
			&& Interaction.Category
				== EFMCodexUMGInteractionCategory::RollThroughBallInitialRoute;
		if (InteractionView.PresentedActionType != ESkillRuleType::ThroughBall
			|| (InteractionView.MajorPhase
					!= EFMCodexLocalMatchMajorPhase::Resolution
				&& !bInitialRoutePending)
			|| !InteractionView.bCurrentAttackActive)
		{
			return Result;
		}

		Result.bVisible = true;
		Result.bSuppressLegacyResolution = !bRejected;
		Result.TitleLabel = FFMCodexPlayerUIPresentationText
			::ThroughBallTitle().ToString();
		Result.InteractionCategory = Interaction.Category;
		Result.OneOnOneChoices = Interaction.OneOnOneChoices;
		Result.ActionPromptLabel = Interaction.PrimaryAction.Label;

		const FMatchPlayCurrentAttackResolutionFactProjection& Facts =
			InteractionView.ResolutionFacts;
		const bool bHasThroughBallRoute = Facts.bSuccess && Facts.bHasFacts
			&& Facts.bHasActualBranch
			&& Facts.ActualBranch.ActionType == ESkillRuleType::ThroughBall;
		if (!bHasThroughBallRoute)
		{
			Result.Stage = EFMCodexUMGThroughBallStage::InitialRoute;
			Result.StageLabel = FFMCodexPlayerUIPresentationText
				::ThroughBallInitialRouteStage().ToString();
			// The stage is the single semantic instruction. The separate primary
			// label is an action, not a repeated copy of that instruction.
			Result.StatusLabel.Empty();
			Result.ActionPromptLabel.Empty();
			Result.bInitialRouteRollAwaitingInput =
				Interaction.Category
					== EFMCodexUMGInteractionCategory
						::RollThroughBallInitialRoute;
			if (!bRejected && Result.bInitialRouteRollAwaitingInput)
			{
				ClaimPrimaryAction(Result.PrimaryAction, Interaction.PrimaryAction);
			}
			Result.bCanContinue = Result.PrimaryAction.bVisible;
			Result.ContinueActionLabel = Result.PrimaryAction.Action.Label;
			return Result;
		}

		const EMatchPlayThroughBallActualBranch CanonicalRoute =
			Facts.ActualBranch.ThroughBall;
		Result.Route = ThroughBallRoute(CanonicalRoute);
		Result.RouteLabel = FFMCodexPlayerUIPresentationText
			::ThroughBallRoute(CanonicalRoute).ToString();
		Result.bRouteRevealComplete = true;
		const FMatchPlayResolutionRollFact* InitialRouteRoll =
			Facts.Rolls.FindByPredicate(
				[](const FMatchPlayResolutionRollFact& Roll)
				{
					return Roll.bInitialRoute && Roll.bResolved
						&& Roll.Semantics
							== EMatchPlayResolutionRollSemantics::BranchSelection;
				});
		if (InitialRouteRoll != nullptr)
		{
			Result.bHasAuthoritativeInitialRouteRoll = true;
			Result.AuthoritativeInitialRouteD6 = InitialRouteRoll->RawD6;
			Result.RouteResultLabel = FFMCodexPlayerUIPresentationText
				::ThroughBallRouteResult(
					InitialRouteRoll->RawD6, CanonicalRoute).ToString();
		}

		auto FindDecision = [&Facts](const FName DecisionId)
		{
			return Facts.Decisions.FindByPredicate(
				[DecisionId](const FMatchPlayResolutionDecisionFact& Decision)
				{
					return Decision.DecisionId == DecisionId;
				});
		};
		auto ProjectNarrative = [&Result, &Facts, &InteractionView](
			const EFMCodexTacticalNarrativeBranch Branch,
			const FMatchPlayResolutionDecisionFact* Decision)
		{
			if (Decision == nullptr || !Decision->bResolved
				|| Decision->Outcome
					== EMatchPlayResolutionDecisionOutcome::None)
			{
				return;
			}
			FFMCodexTacticalNarrativePresentationInput Input;
			Input.Branch = Branch;
			Input.AuthorityOutcome = Decision->Outcome;
			Input.AttackSequence = Facts.AttackSequence;
			Input.StableEventId = Decision->DecisionId;
			Input.Carrier = NarrativeActor(Facts, InteractionView,
				EMatchPlayResolutionParticipantRole::Carrier);
			Input.Runner = NarrativeActor(Facts, InteractionView,
				EMatchPlayResolutionParticipantRole::Runner);
			Input.Goalkeeper = NarrativeActor(Facts, InteractionView,
				EMatchPlayResolutionParticipantRole::Goalkeeper);
			const FFMCodexTacticalNarrativePresentation Narrative =
				FFMCodexTacticalResolutionNarrativePresentationBuilder::Build(Input);
			if (Narrative.bSuccess && Narrative.bNarrativeAvailable)
			{
				Result.bNarrativeAvailable = true;
				Result.ResultTitle = Narrative.ResultTitle.ToString();
				Result.NarrativeHeadline = Narrative.NarrativeText.ToString();
			}
		};
		const FMatchPlayResolutionDecisionFact* AntiDecision = FindDecision(
			TEXT("ThroughBall.AntiOffside.Outcome"));
		const FMatchPlayResolutionDecisionFact* ChipDecision = FindDecision(
			TEXT("ThroughBall.OneOnOne.ChipShot.Outcome"));
		const bool bDirectShot = Formula.bVisible
			&& Formula.ContestId
				== FName(TEXT("ThroughBall.OneOnOne.DirectShot"));

		if (Interaction.Category
			== EFMCodexUMGInteractionCategory::SelectOneOnOneShot)
		{
			Result.Stage = EFMCodexUMGThroughBallStage::OneOnOneChoice;
			Result.StageLabel = TEXT("选择单刀方式");
			Result.StatusLabel.Empty();
			Result.ActionPromptLabel.Empty();
			Result.Formula = Formula;
			// The ThroughBall parent is the sole owner of source-route context on
			// the shared OneOnOne choice page, regardless of whether the source was
			// BehindDefense or AntiOffside.
			Result.Formula.bParentOwnsRouteContext = true;
			if (AntiDecision != nullptr && AntiDecision->bResolved
				&& AntiDecision->Outcome
					== EMatchPlayResolutionDecisionOutcome::OneOnOneRequired)
			{
				ProjectNarrative(
					EFMCodexTacticalNarrativeBranch::ThroughBallAntiOffside,
					AntiDecision);
			}
			return Result;
		}
		if (bDirectShot)
		{
			Result.Stage = EFMCodexUMGThroughBallStage::OneOnOneResolution;
			Result.RouteLabel = TEXT("单刀");
			Result.StageLabel = TEXT("直接射门");
			Result.StatusLabel.Empty();
			Result.ActionPromptLabel.Empty();
			Result.Formula = Formula;
			Result.Formula.bParentOwnsContestHeading = true;
			Result.Formula.bParentOwnsRouteContext = true;
			return Result;
		}
		if (ChipDecision != nullptr)
		{
			Result.Stage = EFMCodexUMGThroughBallStage::OneOnOneResolution;
			Result.RouteLabel = TEXT("单刀");
			Result.StageLabel = TEXT("挑射");
			Result.StatusLabel.Empty();
			Result.ActionPromptLabel.Empty();
			if (Interaction.Category
				== EFMCodexUMGInteractionCategory
					::RollThroughBallOneOnOneChipShotAttack)
			{
				Result.OutcomeRollHint =
					FFMCodexTacticalDetailPresentationBuilder::
						BuildOutcomeRollHint(
							ESkillRuleType::ThroughBall,
							TEXT("ThroughBall.OneOnOneChip"));
			}
			if (ChipDecision->bResolved
				&& InteractionView.bTerminalPendingAdvance)
			{
				ProjectNarrative(
					EFMCodexTacticalNarrativeBranch::ThroughBallOneOnOneChip,
					ChipDecision);
			}
			if (!bRejected && (Interaction.Category
					== EFMCodexUMGInteractionCategory
						::RollThroughBallOneOnOneChipShotAttack
				|| Interaction.Category
					== EFMCodexUMGInteractionCategory::AdvanceAfterTerminal))
			{
				ClaimPrimaryAction(Result.PrimaryAction,
					Interaction.PrimaryAction);
			}
			Result.bCanContinue = Result.PrimaryAction.bVisible;
			Result.ContinueActionLabel = Result.PrimaryAction.Action.Label;
			return Result;
		}

		switch (CanonicalRoute)
		{
		case EMatchPlayThroughBallActualBranch::Feet:
			Result.Stage = EFMCodexUMGThroughBallStage::FeetContest;
			Result.StageLabel = FFMCodexPlayerUIPresentationText
				::ThroughBallFeetStage().ToString();
			Result.Formula = Formula;
			Result.StatusLabel.Empty();
			Result.ActionPromptLabel.Empty();
			break;
		case EMatchPlayThroughBallActualBranch::BehindDefense:
			Result.Stage =
				EFMCodexUMGThroughBallStage::BehindDefenseFirstStage;
			Result.StageLabel = FFMCodexPlayerUIPresentationText
				::ThroughBallBehindDefenseStage().ToString();
			Result.Formula = Formula;
			Result.StatusLabel.Empty();
			Result.ActionPromptLabel.Empty();
			break;
		case EMatchPlayThroughBallActualBranch::AntiOffside:
			Result.Stage = EFMCodexUMGThroughBallStage::AntiOffsideCheck;
			Result.StageLabel = FFMCodexPlayerUIPresentationText
				::ThroughBallAntiOffsideStage().ToString();
			Result.ActionPromptLabel.Empty();
			if (Interaction.Category
				== EFMCodexUMGInteractionCategory
					::RollThroughBallAntiOffsideAttack)
			{
				Result.OutcomeRollHint =
					FFMCodexTacticalDetailPresentationBuilder::
						BuildOutcomeRollHint(
							ESkillRuleType::ThroughBall,
							TEXT("ThroughBall.AntiOffside"));
			}
			if (AntiDecision != nullptr && AntiDecision->bResolved
				&& (InteractionView.bTerminalPendingAdvance
					|| AntiDecision->Outcome
						== EMatchPlayResolutionDecisionOutcome::OneOnOneRequired))
			{
				ProjectNarrative(
					EFMCodexTacticalNarrativeBranch::ThroughBallAntiOffside,
					AntiDecision);
			}
			if (!bRejected && (Interaction.Category
					== EFMCodexUMGInteractionCategory
						::RollThroughBallAntiOffsideAttack
				|| Interaction.Category
					== EFMCodexUMGInteractionCategory::AdvanceAfterTerminal))
			{
				ClaimPrimaryAction(Result.PrimaryAction,
					Interaction.PrimaryAction);
			}
			Result.bCanContinue = Result.PrimaryAction.bVisible;
			Result.ContinueActionLabel = Result.PrimaryAction.Action.Label;
			break;
		default:
			break;
		}
		if (CanonicalRoute != EMatchPlayThroughBallActualBranch::Feet
			&& CanonicalRoute
				!= EMatchPlayThroughBallActualBranch::BehindDefense
			&& CanonicalRoute
				!= EMatchPlayThroughBallActualBranch::AntiOffside)
		{
			Result.StatusLabel = Result.ActionPromptLabel;
		}
		return Result;
	}

	FFMCodexUMGLongShotResolutionViewModel BuildLongShotSurface(
		const FFMCodexLocalMatchInteractionView& InteractionView,
		const FFMCodexUMGInteractionViewModel& Interaction,
		const FFMCodexUMGInlineFormulaSurfaceViewModel& Formula,
		const bool bRejected)
	{
		FFMCodexUMGLongShotResolutionViewModel Result;
		const bool bLongShot = InteractionView.PresentedActionType
			== ESkillRuleType::LongShot;
		const bool bCutInside = InteractionView.PresentedActionType
			== ESkillRuleType::CutInsideShot;
		const bool bCross = InteractionView.PresentedActionType
			== ESkillRuleType::Cross;
		const bool bPassControl = InteractionView.PresentedActionType
			== ESkillRuleType::PassControl;
		if (!InteractionView.bCurrentAttackActive
			|| (!bLongShot && !bCutInside && !bCross && !bPassControl))
		{
			return Result;
		}

		const bool bChoosingBranch = Interaction.Category
				== EFMCodexUMGInteractionCategory::SelectLongShotBranch
			|| (Interaction.Category
					== EFMCodexUMGInteractionCategory::SelectBranchIntent
				&& (bCutInside || bCross));
		const FMatchPlayCurrentAttackResolutionFactProjection& Facts =
			InteractionView.ResolutionFacts;
		const bool bHasResolvedBranch = !bCross
			&& Facts.bSuccess && Facts.bHasFacts
			&& Facts.bHasActualBranch
			&& Facts.ActualBranch.ActionType
				== InteractionView.PresentedActionType;
		const bool bPassControlRoutePending = bPassControl
			&& Interaction.Category
				== EFMCodexUMGInteractionCategory::RollPassControlRoute
			&& !Facts.bHasActualBranch;
		if (!bChoosingBranch && !bHasResolvedBranch
			&& !bPassControlRoutePending)
		{
			return Result;
		}

		Result.bVisible = true;
		Result.bSuppressLegacyResolution = !bRejected;
		Result.SkillType = InteractionView.PresentedActionType;
		Result.TitleLabel = bCross
			? FFMCodexPlayerUIPresentationText::CrossTitle().ToString()
			: bPassControl
				? FFMCodexPlayerUIPresentationText::PassControlTitle().ToString()
			: bCutInside
				? FFMCodexPlayerUIPresentationText::CutInsideTitle().ToString()
				: FFMCodexPlayerUIPresentationText::LongShotTitle().ToString();
		Result.InteractionCategory = Interaction.Category;
		if (bChoosingBranch)
		{
			Result.Stage = EFMCodexUMGLongShotStage::BranchChoice;
			Result.StageLabel = bCross
				? FFMCodexPlayerUIPresentationText
					::CrossBranchChoiceStage().ToString()
				: bCutInside
					? FFMCodexPlayerUIPresentationText
						::CutInsideBranchChoiceStage().ToString()
					: FFMCodexPlayerUIPresentationText
						::LongShotBranchChoiceStage().ToString();
			Result.BranchChoices = Interaction.BranchChoices;
			if (bCutInside || bCross)
			{
				for (FFMCodexUMGBranchChoiceViewModel& Choice
					: Result.BranchChoices)
				{
					const FName BranchId = bCross
						? Choice.Intent == EFMCodexUMGBranchIntent::CrossHigh
							? FName(TEXT("Cross.High"))
							: Choice.Intent == EFMCodexUMGBranchIntent::CrossLow
								? FName(TEXT("Cross.Low"))
								: NAME_None
						: Choice.Intent == EFMCodexUMGBranchIntent::DirectShot
							? FName(TEXT("CutInside.Direct"))
							: Choice.Intent
								== EFMCodexUMGBranchIntent::DeadCorner
									? FName(TEXT("CutInside.DeadCorner"))
									: NAME_None;
					if (bCutInside
						&& Choice.Intent == EFMCodexUMGBranchIntent::DeadCorner)
					{
						Choice.Label = FFMCodexPlayerUIPresentationText
							::CutInsideDeadCornerStage().ToString();
					}
					Choice.SecondaryLabel =
						FFMCodexTacticalDetailPresentationBuilder
							::BuildBranchChoiceHint(
								bCross ? ESkillRuleType::Cross
									: ESkillRuleType::CutInsideShot,
								BranchId).ToString();
				}
			}
			return Result;
		}
		if (bPassControl)
		{
			Result.Stage = EFMCodexUMGLongShotStage::PassControl;
			if (bPassControlRoutePending)
			{
				Result.StageLabel = TEXT("判定推进方式");
				Result.StatusLabel = TEXT("掷点决定推进方式");
				if (!bRejected)
				{
					ClaimPrimaryAction(
						Result.PrimaryAction, Interaction.PrimaryAction);
				}
				Result.bCanContinue = Result.PrimaryAction.bVisible;
				Result.ContinueActionLabel = Result.PrimaryAction.Action.Label;
				return Result;
			}

			Result.BranchLabel = Formula.RouteResultLabel;
			// Formula.ContestLabel becomes the full terminal Narrative after the
			// disclosure gate. Keep the outer Stage slot semantic so that the
			// nested Formula surface remains the single complete-prose owner.
			Result.StageLabel = FFMCodexPlayerUIPresentationText
				::ResolutionContest(Formula.ContestId).ToString();
			Result.Formula = Formula;
			Result.Formula.bParentOwnsContestHeading = true;
			Result.Formula.bParentOwnsRouteContext = true;
			Result.StatusLabel = Formula.bVisible
				? FString() : FString(TEXT("等待进攻方掷点"));
			Result.bCanContinue = Result.Formula.PrimaryAction.bVisible;
			Result.ContinueActionLabel =
				Result.Formula.PrimaryAction.Action.Label;
			return Result;
		}

		const bool bDirect = bCutInside
			? Facts.ActualBranch.CutInsideShot
				== EMatchPlayCutInsideShotActualBranch::DirectShot
			: Facts.ActualBranch.LongShot
				== EMatchPlayLongShotActualBranch::DirectShot;
		Result.Stage = bDirect ? EFMCodexUMGLongShotStage::DirectShot
			: EFMCodexUMGLongShotStage::DeadCorner;
		Result.BranchLabel = bCutInside
			? (bDirect
				? FFMCodexPlayerUIPresentationText::CutInsideDirectStage().ToString()
				: FFMCodexPlayerUIPresentationText
					::CutInsideDeadCornerStage().ToString())
			: (bDirect
				? FFMCodexPlayerUIPresentationText::LongShotDirectStage().ToString()
				: FFMCodexPlayerUIPresentationText
					::LongShotDeadCornerStage().ToString());
		Result.StageLabel = Result.BranchLabel;
		Result.OutcomeHintLabel = bDirect
			? ((Interaction.Category == EFMCodexUMGInteractionCategory::RollLongShotDirectAttack
				|| Interaction.Category == EFMCodexUMGInteractionCategory::RollCutInsideShotDirectAttack)
				? (bCutInside
					? FFMCodexPlayerUIPresentationText
						::CutInsideDirectOutcomeHint().ToString()
					: FFMCodexPlayerUIPresentationText
						::LongShotDirectOutcomeHint().ToString()) : FString())
			: !InteractionView.bTerminalPendingAdvance
				? FFMCodexPlayerUIPresentationText::LongShotDeadCornerOutcomeHint().ToString()
				: FString();

		if (bDirect)
		{
			Result.Formula = Formula;
			Result.Formula.bParentOwnsContestHeading = true;
			Result.Formula.bParentOwnsRouteContext = true;
			Result.StatusLabel = Formula.bVisible ? FString() : TEXT("等待进攻方掷点");
		}
		else
		{
			auto FindRoll = [&Facts](
				const EMatchPlayCurrentAttackPostRouteRollPurpose Purpose)
			{
				return Facts.Rolls.FindByPredicate(
					[Purpose](const FMatchPlayResolutionRollFact& Roll)
					{
						return Roll.PostRoutePurpose == Purpose;
					});
			};
			const FMatchPlayResolutionRollFact* A = FindRoll(
				EMatchPlayCurrentAttackPostRouteRollPurpose::PairedAttackA);
			const FMatchPlayResolutionRollFact* B = FindRoll(
				EMatchPlayCurrentAttackPostRouteRollPurpose::PairedAttackB);
			Result.bDeadCornerAVisible = A != nullptr && A->bResolved;
			Result.DeadCornerA = Result.bDeadCornerAVisible ? A->RawD6 : 0;
			Result.bDeadCornerBVisible = B != nullptr && B->bResolved;
			Result.DeadCornerB = Result.bDeadCornerBVisible ? B->RawD6 : 0;
			if (Result.bDeadCornerAVisible)
			{
				Result.PairedRollResultLabel = Result.bDeadCornerBVisible
					? FFMCodexPlayerUIPresentationText::PairedRollResult(
						Result.DeadCornerA, Result.DeadCornerB).ToString()
					: FFMCodexPlayerUIPresentationText::FirstPairedRollResult(Result.DeadCornerA).ToString();
			}
			const FMatchPlayResolutionDecisionFact* Decision =
				Facts.Decisions.FindByPredicate(
					[](const FMatchPlayResolutionDecisionFact& Candidate)
					{
						return Candidate.DecisionId == TEXT("DeadCorner.Outcome");
					});
			if (Decision != nullptr && Decision->bResolved
				&& InteractionView.bTerminalPendingAdvance)
			{
				FFMCodexTacticalNarrativePresentationInput Input;
				Input.Branch = bCutInside
					? EFMCodexTacticalNarrativeBranch::CutInsideDeadCorner
					: EFMCodexTacticalNarrativeBranch::LongShotDeadCorner;
				Input.AuthorityOutcome = Decision->Outcome;
				Input.AttackSequence = Facts.AttackSequence;
				Input.StableEventId = Decision->DecisionId;
				Input.Carrier = NarrativeActor(Facts, InteractionView,
					EMatchPlayResolutionParticipantRole::Carrier);
				const FFMCodexTacticalNarrativePresentation Narrative =
					FFMCodexTacticalResolutionNarrativePresentationBuilder::Build(Input);
				if (Narrative.bSuccess)
				{
					Result.bNarrativeAvailable = Narrative.bNarrativeAvailable;
					Result.ResultTitle = Narrative.ResultTitle.ToString();
					Result.NarrativeHeadline = Narrative.NarrativeText.ToString();
				}
			}
		}

		if (!bRejected && ((!bDirect && (Interaction.Category
				== EFMCodexUMGInteractionCategory::RollLongShotDeadCorner
			|| Interaction.Category
				== EFMCodexUMGInteractionCategory::RollCutInsideShotDeadCorner
			|| Interaction.Category
				== EFMCodexUMGInteractionCategory::AdvanceAfterTerminal))
			|| (bDirect && !Formula.bVisible
				&& Interaction.Category
					== EFMCodexUMGInteractionCategory::AdvanceAfterTerminal)))
		{
			ClaimPrimaryAction(Result.PrimaryAction, Interaction.PrimaryAction);
		}
		Result.bCanContinue = Result.PrimaryAction.bVisible;
		Result.ContinueActionLabel = Result.PrimaryAction.Action.Label;
		return Result;
	}
}

void FFMCodexUMGDeploymentTargetProjector::ProjectSlot(
	FFMCodexUMGPitchSlotViewModel& Slot,
	const FName DraggedCardId,
	const TArray<FFMCodexUMGDeploymentChoiceViewModel>& Choices)
{
	Slot.DeploymentTargetCardId = DraggedCardId;
	if (Slot.bOccupied)
	{
		Slot.DeploymentTargetState =
			EFMCodexUMGDeploymentTargetState::Occupied;
		return;
	}
	if (DraggedCardId.IsNone())
	{
		Slot.DeploymentTargetState =
			EFMCodexUMGDeploymentTargetState::Neutral;
		return;
	}

	const FFMCodexUMGDeploymentChoiceViewModel* Choice = Choices.FindByPredicate(
		[DraggedCardId](const FFMCodexUMGDeploymentChoiceViewModel& Candidate)
		{
			return Candidate.CardId == DraggedCardId;
		});
	if (Choice == nullptr)
	{
		Slot.DeploymentTargetState =
			EFMCodexUMGDeploymentTargetState::Unavailable;
		return;
	}

	const bool bIsDestination = Choice->Destinations.ContainsByPredicate(
		[&Slot](const FFMCodexUMGDeploymentDestinationViewModel& Destination)
		{
			return Destination.SlotId == Slot.SlotId;
		});
	Slot.DeploymentTargetState = bIsDestination
		? EFMCodexUMGDeploymentTargetState::Valid
		: EFMCodexUMGDeploymentTargetState::Invalid;
}

FFMCodexUMGCardViewModel FFMCodexLocalMatchUMGPresentationBuilder::BuildCard(
	const FFMCodexLocalMatchCardView& CardView)
{
	return FMCodexLocalMatchUMGPresentation::MakeCard(CardView);
}

FFMCodexUMGMatchScreenViewModel
FFMCodexLocalMatchUMGPresentationBuilder::Build(
	const FFMCodexLocalMatchInteractionView& InteractionView,
	const FFMCodexLocalMatchResolutionFeedback& ResolutionFeedback,
	const FString& DiagnosticMessage,
	const EInitialTurnOrderPlayer LocalViewerSide)
{
	return Build(InteractionView, ResolutionFeedback, DiagnosticMessage,
		LocalViewerSide, FFMCodexUMGSidePrimaryColors());
}

FFMCodexUMGMatchScreenViewModel
FFMCodexLocalMatchUMGPresentationBuilder::Build(
	const FFMCodexLocalMatchInteractionView& InteractionView,
	const FFMCodexLocalMatchResolutionFeedback& ResolutionFeedback,
	const FString& DiagnosticMessage,
	const EInitialTurnOrderPlayer LocalViewerSide,
	const FFMCodexUMGSidePrimaryColors& SidePrimaryColors)
{
	using namespace FMCodexLocalMatchUMGPresentation;
	FFMCodexUMGMatchScreenViewModel Result;
	const bool bProjectOnPitchPlayerSelection =
		InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::SelectCarrier
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::SelectMarker
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::SelectRunner
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::SelectHelper;
	const EFMCodexUMGOnPitchSelectionIntent OnPitchSelectionIntent =
		InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::SelectCarrier
				? EFMCodexUMGOnPitchSelectionIntent::SubmitCarrier
					: InteractionView.InteractionCategory
						== EFMCodexLocalMatchInteractionCategory::SelectMarker
							? EFMCodexUMGOnPitchSelectionIntent::SubmitMarker
							: InteractionView.InteractionCategory
								== EFMCodexLocalMatchInteractionCategory::SelectRunner
									? EFMCodexUMGOnPitchSelectionIntent::SubmitRunner
									: InteractionView.InteractionCategory
										== EFMCodexLocalMatchInteractionCategory::SelectHelper
											? EFMCodexUMGOnPitchSelectionIntent::SubmitHelper
											: EFMCodexUMGOnPitchSelectionIntent::None;
	const FFMCodexLocalMatchScreenPresentation Screen =
		FFMCodexLocalMatchInteractionViewBuilder::BuildScreenPresentation(
			InteractionView);

	Result.Header.PlayerAScoreLabel = FString::FromInt(
		InteractionView.PlayerAScore);
	Result.Header.PlayerBScoreLabel = FString::FromInt(
		InteractionView.PlayerBScore);
	const EInitialTurnOrderPlayer OpponentSide = OtherSide(LocalViewerSide);
	Result.LocalPlayerLabel =
		FFMCodexLocalMatchInteractionViewBuilder::ToString(LocalViewerSide);
	Result.Header.LeftPlayerLabel = Result.LocalPlayerLabel;
	Result.Header.LeftPlayerSide = LocalViewerSide;
	Result.Header.RightPlayerLabel =
		FFMCodexLocalMatchInteractionViewBuilder::ToString(OpponentSide);
	Result.Header.LeftScoreLabel = FString::FromInt(
		LocalViewerSide == EInitialTurnOrderPlayer::PlayerA
			? InteractionView.PlayerAScore : InteractionView.PlayerBScore);
	Result.Header.RightScoreLabel = FString::FromInt(
		OpponentSide == EInitialTurnOrderPlayer::PlayerA
			? InteractionView.PlayerAScore : InteractionView.PlayerBScore);
	Result.Header.ScoreLabel = FString::Printf(
		TEXT("%s - %s"),
		*Result.Header.LeftScoreLabel,
		*Result.Header.RightScoreLabel);
	auto TrackerForSide = [&](const EInitialTurnOrderPlayer Side)
	{
		return MakeAttackTurnTracker(
			Side == EInitialTurnOrderPlayer::PlayerA
				? InteractionView.PlayerAMaxAttackTurns
				: InteractionView.PlayerBMaxAttackTurns,
			Side == EInitialTurnOrderPlayer::PlayerA
				? InteractionView.PlayerAUsedAttackTurns
				: InteractionView.PlayerBUsedAttackTurns,
			Side == EInitialTurnOrderPlayer::PlayerA
				? InteractionView.PlayerACurrentAttackIndex
				: InteractionView.PlayerBCurrentAttackIndex,
			Side == EInitialTurnOrderPlayer::PlayerA
				? InteractionView.bPlayerACurrentAttackTurn
				: InteractionView.bPlayerBCurrentAttackTurn,
			PrimaryColorForSide(Side, SidePrimaryColors));
	};
	Result.Header.LeftAttackTurnTracker = TrackerForSide(LocalViewerSide);
	Result.Header.RightAttackTurnTracker = TrackerForSide(OpponentSide);
	Result.Header.TurnLabel = InteractionView.AttackSequence > 0
		? FString::Printf(TEXT("TURN %lld"), InteractionView.AttackSequence)
		: TEXT("PRE-MATCH");
	const bool bProjectTacticalPointChip =
		InteractionView.bCurrentAttackActive
		&& InteractionView.RouteKind
			== EMatchPlayCurrentAttackRouteKind::Ordinary;
	Result.Header.CurrentAttackerTacticalPointsLabel =
		bProjectTacticalPointChip
			? FString::Printf(TEXT("TACTICAL POINTS  %d"),
				InteractionView.ActionPoint)
			: FString();
	Result.Header.CurrentPhaseLabel =
		FFMCodexLocalMatchInteractionViewBuilder::ToString(
			InteractionView.MajorPhase);
	Result.Header.AttackSequence = InteractionView.AttackSequence;
	Result.Header.CurrentAttackerTacticalPoints = InteractionView.ActionPoint;
	Result.Header.RawInitialD12 = InteractionView.RawInitialD12;
	Result.Header.RouteKind = InteractionView.RouteKind;
	Result.Header.bHasCurrentAttacker =
		InteractionView.CurrentAttackingPlayer != EInitialTurnOrderPlayer::None;
	Result.Header.bCurrentAttackerOnLeft =
		InteractionView.CurrentAttackingPlayer == LocalViewerSide;
	Result.Header.bShowLeftTacticalPointChip =
		bProjectTacticalPointChip
		&& Result.Header.bCurrentAttackerOnLeft;
	Result.Header.bShowRightTacticalPointChip =
		bProjectTacticalPointChip
		&& !Result.Header.bCurrentAttackerOnLeft;
	Result.Header.LeftTacticalPoints =
		Result.Header.bShowLeftTacticalPointChip
			? InteractionView.ActionPoint : 0;
	Result.Header.RightTacticalPoints =
		Result.Header.bShowRightTacticalPointChip
			? InteractionView.ActionPoint : 0;
	Result.Header.CurrentAttackerAttackIndex =
		InteractionView.CurrentAttackingPlayer
			== EInitialTurnOrderPlayer::PlayerA
				? InteractionView.PlayerACurrentAttackIndex
				: InteractionView.PlayerBCurrentAttackIndex;
	Result.Header.CurrentAttackerMaxAttackTurns =
		InteractionView.CurrentAttackingPlayer
			== EInitialTurnOrderPlayer::PlayerA
				? InteractionView.PlayerAMaxAttackTurns
				: InteractionView.PlayerBMaxAttackTurns;
	Result.Header.bTacticalPointRollReady =
		InteractionView.bTacticalPointRollReady;
	Result.Header.CurrentAttackerLabel =
		FFMCodexLocalMatchInteractionViewBuilder::ToString(
			InteractionView.CurrentAttackingPlayer);
	Result.Header.ExpectedActorLabel = Screen.bSystemResolution
		? TEXT("System Resolution")
		: FString::Printf(TEXT("Expected actor: %s"),
			*FFMCodexLocalMatchInteractionViewBuilder::ToString(
				InteractionView.ExpectedActingPlayer));
	Result.Header.MatchStatusLabel = Screen.MatchStatusLabel;
	Result.Header.AttackerStatusLabel = FString();
	if (InteractionView.bMatchActive && !InteractionView.bMatchEnded
		&& InteractionView.CurrentAttackingPlayer
			!= EInitialTurnOrderPlayer::None)
	{
		const FString AttackerLabel =
			FFMCodexLocalMatchInteractionViewBuilder::ToString(
				InteractionView.CurrentAttackingPlayer).ToUpper();
		Result.Header.AttackerStatusLabel =
			InteractionView.bCurrentAttackActive
			? FString::Printf(TEXT("%s ATTACKING"), *AttackerLabel)
			: FString::Printf(TEXT("NEXT ATTACKER: %s"), *AttackerLabel);
	}
	Result.Header.ActorStatusLabel = Screen.ActingStatusLabel;
	Result.Header.MatchResultLabel = InteractionView.bMatchEnded
		? FFMCodexLocalMatchInteractionViewBuilder::ToString(
			InteractionView.MatchResult)
		: FString();
	Result.Header.bMatchEnded = InteractionView.bMatchEnded;
	Result.Header.bMatchActive = InteractionView.bMatchActive;
	Result.Header.bAttackActive = InteractionView.bCurrentAttackActive;
	Result.Header.bHumanAction = InteractionView.bMatchActive
		&& !InteractionView.bMatchEnded
		&& InteractionView.ExpectedActingPlayer
			!= EInitialTurnOrderPlayer::None;
	Result.Header.bSystemResolution = Screen.bSystemResolution;

	BuildRack(
		LocalViewerSide == EInitialTurnOrderPlayer::PlayerA
			? InteractionView.PlayerACardRoster
			: InteractionView.PlayerBCardRoster,
		InteractionView.DeploymentGroups,
		InteractionView,
		LocalViewerSide,
		true,
		Result.Header.LeftPlayerLabel,
		Result.LocalRack);
	BuildRack(
		OpponentSide == EInitialTurnOrderPlayer::PlayerA
			? InteractionView.PlayerACardRoster
			: InteractionView.PlayerBCardRoster,
		InteractionView.DeploymentGroups,
		InteractionView,
		OpponentSide,
		false,
		Result.Header.RightPlayerLabel,
		Result.OpponentRack);
	Result.LocalRack.bHasTacticalPlayerCount =
		InteractionView.bHasTacticalPlayerCounts;
	Result.LocalRack.TacticalPlayerCount =
		LocalViewerSide == EInitialTurnOrderPlayer::PlayerA
			? InteractionView.PlayerATacticalPlayerCount
			: InteractionView.PlayerBTacticalPlayerCount;
	Result.LocalRack.TacticalPlayerCountLabel = FString::Printf(
		TEXT("战术球员 ×%d"), Result.LocalRack.TacticalPlayerCount);
	Result.OpponentRack.bHasTacticalPlayerCount =
		InteractionView.bHasTacticalPlayerCounts;
	Result.OpponentRack.TacticalPlayerCount =
		OpponentSide == EInitialTurnOrderPlayer::PlayerA
			? InteractionView.PlayerATacticalPlayerCount
			: InteractionView.PlayerBTacticalPlayerCount;
	Result.OpponentRack.TacticalPlayerCountLabel = FString::Printf(
		TEXT("战术球员 ×%d"), Result.OpponentRack.TacticalPlayerCount);

	for (const EMatchPlayNeutralSlotSide VisualSide : {
		PhysicalSide(LocalViewerSide), PhysicalSide(OpponentSide) })
	{
		const FFMCodexLocalMatchPitchRegionView* RegionPtr =
			InteractionView.PitchRegions.FindByPredicate(
				[VisualSide](const FFMCodexLocalMatchPitchRegionView& Candidate)
				{
					return Candidate.NeutralSide == VisualSide;
				});
		if (RegionPtr == nullptr)
		{
			continue;
		}
		const FFMCodexLocalMatchPitchRegionView& Region = *RegionPtr;
		FFMCodexUMGPitchRegionViewModel RegionResult;
		RegionResult.RegionLabel = Region.Label;
		RegionResult.ZoneContextLabel = Region.ZoneContextLabel;
		RegionResult.PhysicalHalfLabel =
			FFMCodexLocalMatchInteractionViewBuilder::ToString(
				Region.NeutralSide);
		RegionResult.bLocalFacingLane = Region.NeutralSide
			== PhysicalSide(LocalViewerSide);
		RegionResult.VisualLaneIndex = Result.PitchRegions.Num();
		RegionResult.TacticalRegionLabel =
			FFMCodexLocalMatchInteractionViewBuilder::ToString(
				LocalViewerSide == EInitialTurnOrderPlayer::PlayerA
					? Region.PlayerARelativeZone
					: Region.PlayerBRelativeZone);
		if (RegionResult.TacticalRegionLabel == TEXT("Forward"))
		{
			RegionResult.VisualRole = EFMCodexUMGPitchVisualRole::Forward;
		}
		else if (RegionResult.TacticalRegionLabel == TEXT("Backfield"))
		{
			RegionResult.VisualRole = EFMCodexUMGPitchVisualRole::Backfield;
		}
		else
		{
			RegionResult.VisualRole = EFMCodexUMGPitchVisualRole::Midfield;
		}
		RegionResult.VisualRoleLabel =
			FFMCodexPlayerUIPresentationText::TacticalRegion(
				RegionResult.TacticalRegionLabel);
		RegionResult.bCurrentAttackingSide = Region.bCurrentAttackingSide;
		for (const FFMCodexLocalMatchPitchSlotView& Slot : Region.Slots)
		{
			FFMCodexUMGPitchSlotViewModel SlotResult;
			SlotResult.SlotId = Slot.SlotId;
			SlotResult.SlotLabel = Slot.Label;
			SlotResult.PhysicalHalfLabel =
				FFMCodexLocalMatchInteractionViewBuilder::ToString(
					Slot.NeutralSide);
			SlotResult.PlayerARelativeZoneLabel =
				FFMCodexLocalMatchInteractionViewBuilder::ToString(
					Slot.PlayerARelativeZone);
			SlotResult.PlayerBRelativeZoneLabel =
				FFMCodexLocalMatchInteractionViewBuilder::ToString(
					Slot.PlayerBRelativeZone);
			SlotResult.bOccupied = Slot.bOccupied;
			SlotResult.DeploymentTargetState = Slot.bOccupied
				? EFMCodexUMGDeploymentTargetState::Occupied
				: EFMCodexUMGDeploymentTargetState::Neutral;
			if (Slot.bOccupied)
			{
				SlotResult.Card = MakeCard(Slot.Card);
				ResolvePitchMiniOwnershipAccent(SlotResult.Card,
					Slot.Card.Side, LocalViewerSide, SidePrimaryColors);
				ResolveSelectedRole(SlotResult.Card, InteractionView);
				if (bProjectOnPitchPlayerSelection)
				{
					const FFMCodexLocalMatchSelectionOption* ProjectedOption =
						InteractionView.SelectionOptions.FindByPredicate(
							[&Slot](const FFMCodexLocalMatchSelectionOption& Option)
							{
								return !Option.Id.IsNone()
									&& Option.RelatedCardId == Slot.Card.CardId;
							});
					if (ProjectedOption != nullptr)
					{
						SlotResult.bSelectableForCurrentPrompt = true;
						SlotResult.OnPitchSelectionOptionId = ProjectedOption->Id;
						SlotResult.OnPitchSelectionIntent =
							OnPitchSelectionIntent;
					}
				}
				const FFMCodexLocalMatchSelectionFeedbackCandidate*
					FeedbackCandidate =
					InteractionView.SelectionFeedbackCandidates.FindByPredicate(
						[&Slot](
							const FFMCodexLocalMatchSelectionFeedbackCandidate& Candidate)
						{
							return Candidate.CardId == Slot.Card.CardId;
						});
				if (FeedbackCandidate != nullptr)
				{
					SlotResult.SelectionFeedbackReason = SelectionFeedbackReason(
						FeedbackCandidate->Reason);
					if (SlotResult.SelectionFeedbackReason !=
						EFMCodexUMGSelectionFeedbackReason::None)
					{
						SlotResult.SelectionFeedbackLabel =
							FFMCodexPlayerUIPresentationText::SelectionFeedback(
								SelectionFeedbackReasonKey(
									SlotResult.SelectionFeedbackReason)).ToString();
					}
				}
			}
			RegionResult.Slots.Add(MoveTemp(SlotResult));
		}
		Result.PitchRegions.Add(MoveTemp(RegionResult));
	}

	Result.Interaction.KickerLabel = Screen.InteractionKicker;
	Result.Interaction.TitleLabel = Screen.InteractionTitle;
	Result.Interaction.Category = InteractionCategory(
		InteractionView.InteractionCategory);
	Result.Interaction.bUseOnPitchPlayerSelection =
		bProjectOnPitchPlayerSelection;
	Result.Interaction.OnPitchSelectionHintLabel =
		bProjectOnPitchPlayerSelection
			? TEXT("Click a player on the pitch")
			: !InteractionView.SelectionNotice.IsEmpty()
				? FFMCodexPlayerUIPresentationText::SelectionFeedback(
					InteractionView.SelectionNotice).ToString()
				: FString();
	Result.Interaction.ClassificationLabel = Screen.bSystemResolution
		? TEXT("SYSTEM") : InteractionView.bHumanInteraction
			? TEXT("HUMAN INPUT") : TEXT("INFORMATION");
	Result.Interaction.CategoryLabel =
		(InteractionView.InteractionCategory
				== EFMCodexLocalMatchInteractionCategory::SelectSkill
			|| InteractionView.InteractionCategory
				== EFMCodexLocalMatchInteractionCategory::SelectBranchIntent
			|| InteractionView.InteractionCategory
				== EFMCodexLocalMatchInteractionCategory::SelectLongShotBranch
			|| InteractionView.InteractionCategory
				== EFMCodexLocalMatchInteractionCategory::SelectOneOnOneShot)
				? FString()
				: FFMCodexLocalMatchInteractionViewBuilder::ToString(
					InteractionView.InteractionCategory);
	Result.Interaction.ExpectedActorLabel = Screen.ActingStatusLabel;
	// Persistent Tactical Points are a Header fact. The Dock presentation does
	// not duplicate them; it remains operation-context focused.
	Result.Interaction.ActionPointLabel = FString();
	Result.Interaction.bSystemResolution = Screen.bSystemResolution;
	Result.Interaction.bMatchEnded = InteractionView.bMatchEnded;
	const bool bTacticalPointPending =
		InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::TacticalPointRoll;
	const bool bCrossRoutePending =
		InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::RollCrossRoute
		&& (InteractionView.ElectiveBranchIntent
				== EMatchPlayElectiveBranchIntent::CrossHigh
			|| InteractionView.ElectiveBranchIntent
				== EMatchPlayElectiveBranchIntent::CrossLow)
		&& !InteractionView.ResolutionFacts.bHasActualBranch;
	const bool bThroughBallRoutePending =
		InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory
				::RollThroughBallInitialRoute
		&& InteractionView.PresentedActionType == ESkillRuleType::ThroughBall
		&& !InteractionView.ResolutionFacts.bHasActualBranch;
	const bool bPassControlRoutePending =
		InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::RollPassControlRoute
		&& InteractionView.PresentedActionType == ESkillRuleType::PassControl
		&& !InteractionView.ResolutionFacts.bHasActualBranch;
	if (bTacticalPointPending)
	{
		Result.Interaction.CrossRollRevealKind =
			EFMCodexUMGCrossRollRevealKind::TacticalPoint;
		Result.Interaction.CrossRollContestId = TEXT("Match.TacticalPoint");
		Result.Interaction.CrossRollSequenceIndex = 0;
		Result.Interaction.CrossRollOwnerSide =
			InteractionView.CurrentAttackingPlayer;
	}
	else if (InteractionView.InteractionCategory
		== EFMCodexLocalMatchInteractionCategory::RollSetPieceType)
	{
		Result.Interaction.CrossRollRevealKind =
			EFMCodexUMGCrossRollRevealKind::SetPieceType;
		Result.Interaction.CrossRollContestId = TEXT("SetPiece.Type");
		Result.Interaction.CrossRollSequenceIndex = 0;
		Result.Interaction.CrossRollOwnerSide =
			InteractionView.ExpectedActingPlayer;
	}
	else if (InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::RollShortFreeKickAngled
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::RollLongFreeKickPower)
	{
		Result.Interaction.CrossRollRevealKind =
			EFMCodexUMGCrossRollRevealKind::SetPiecePairedA;
		Result.Interaction.CrossRollContestId =
			InteractionView.SetPieceType == ESetPieceSelectedType::ShortFreeKick
				? FName(TEXT("SetPiece.Short.Angled"))
				: FName(TEXT("SetPiece.Long.Power"));
		Result.Interaction.CrossRollSequenceIndex = 0;
		Result.Interaction.CrossRollOwnerSide =
			InteractionView.ExpectedActingPlayer;
	}
	else if (InteractionView.InteractionCategory
		== EFMCodexLocalMatchInteractionCategory::RollCornerParticipantSelection)
	{
		Result.Interaction.CrossRollRevealKind =
			EFMCodexUMGCrossRollRevealKind::CornerParticipantSelection;
		Result.Interaction.CrossRollContestId = TEXT("SetPiece.Corner.Participant");
		Result.Interaction.CrossRollSequenceIndex = 0;
		Result.Interaction.CrossRollOwnerSide = InteractionView.ExpectedActingPlayer;
	}
	else if (InteractionView.InteractionCategory
		== EFMCodexLocalMatchInteractionCategory::RollCornerRoute)
	{
		Result.Interaction.CrossRollRevealKind =
			EFMCodexUMGCrossRollRevealKind::CornerRoute;
		Result.Interaction.CrossRollContestId = TEXT("SetPiece.Corner.Route");
		Result.Interaction.CrossRollSequenceIndex = 0;
		Result.Interaction.CrossRollOwnerSide = InteractionView.ExpectedActingPlayer;
	}
	else if (InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::RollShortFreeKickDirectAttack
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::RollLongFreeKickDirectAttack
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::RollPenaltyDirectAttack
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::RollPenaltyPanenka
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::RollCornerAttack)
	{
		Result.Interaction.CrossRollRevealKind =
			EFMCodexUMGCrossRollRevealKind::SetPieceAttack;
		Result.Interaction.CrossRollContestId = TEXT("SetPiece.Attack");
		Result.Interaction.CrossRollSequenceIndex = 0;
		Result.Interaction.CrossRollOwnerSide = InteractionView.ExpectedActingPlayer;
	}
	else if (InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::RollShortFreeKickDirectDefense
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::RollLongFreeKickDirectDefense
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::RollPenaltyDirectDefense
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::RollCornerDefense)
	{
		Result.Interaction.CrossRollRevealKind =
			EFMCodexUMGCrossRollRevealKind::SetPieceDefense;
		Result.Interaction.CrossRollContestId = TEXT("SetPiece.Defense");
		Result.Interaction.CrossRollSequenceIndex = 0;
		Result.Interaction.CrossRollOwnerSide = InteractionView.ExpectedActingPlayer;
	}
	else if (bCrossRoutePending)
	{
		Result.Interaction.CrossRollRevealKind =
			EFMCodexUMGCrossRollRevealKind::InitialRoute;
		Result.Interaction.CrossRollContestId = TEXT("Cross.Route");
		Result.Interaction.CrossRollSequenceIndex = 0;
		Result.Interaction.CrossRollOwnerSide =
			InteractionView.CurrentAttackingPlayer;
	}
	else if (bThroughBallRoutePending)
	{
		Result.Interaction.CrossRollRevealKind =
			EFMCodexUMGCrossRollRevealKind::ThroughBallInitialRoute;
		Result.Interaction.CrossRollContestId = TEXT("ThroughBall.Route");
		Result.Interaction.CrossRollSequenceIndex = 0;
		Result.Interaction.CrossRollOwnerSide =
			InteractionView.CurrentAttackingPlayer;
	}
	else if (bPassControlRoutePending)
	{
		Result.Interaction.CrossRollRevealKind =
			EFMCodexUMGCrossRollRevealKind::PassControlInitialRoute;
		Result.Interaction.CrossRollContestId = TEXT("PassControl.Route");
		Result.Interaction.CrossRollSequenceIndex = 0;
		Result.Interaction.CrossRollOwnerSide =
			InteractionView.CurrentAttackingPlayer;
	}
	else if (InteractionView.InteractionCategory
		== EFMCodexLocalMatchInteractionCategory::RollCrossAttack
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::RollCrossDefense
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory
				::RollThroughBallInitialRoute
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::RollThroughBallFeetAttack
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::RollThroughBallFeetDefense
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory
				::RollThroughBallAntiOffsideAttack
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory
				::RollThroughBallOneOnOneChipShotAttack
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory
				::RollThroughBallOneOnOneDirectShotAttack
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory
				::RollThroughBallOneOnOneDirectShotDefense
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory
				::RollThroughBallBehindDefenseAttack
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory
				::RollThroughBallBehindDefenseDefense
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::RollLongShotDirectAttack
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::RollLongShotDirectDefense
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::RollLongShotDeadCorner
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory
				::RollCutInsideShotDirectAttack
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory
				::RollCutInsideShotDirectDefense
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory
				::RollCutInsideShotDeadCorner
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::RollPassControlAttack
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::RollPassControlDefense)
	{
		const bool bAttackRoll = InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::RollCrossAttack
			|| InteractionView.InteractionCategory
				== EFMCodexLocalMatchInteractionCategory::RollThroughBallFeetAttack
			|| InteractionView.InteractionCategory
				== EFMCodexLocalMatchInteractionCategory
					::RollThroughBallBehindDefenseAttack
			|| InteractionView.InteractionCategory
				== EFMCodexLocalMatchInteractionCategory
					::RollThroughBallAntiOffsideAttack
			|| InteractionView.InteractionCategory
				== EFMCodexLocalMatchInteractionCategory
					::RollThroughBallOneOnOneChipShotAttack
			|| InteractionView.InteractionCategory
				== EFMCodexLocalMatchInteractionCategory
					::RollThroughBallOneOnOneDirectShotAttack
			|| InteractionView.InteractionCategory
				== EFMCodexLocalMatchInteractionCategory::RollLongShotDirectAttack
			|| InteractionView.InteractionCategory
				== EFMCodexLocalMatchInteractionCategory
					::RollCutInsideShotDirectAttack
			|| InteractionView.InteractionCategory
				== EFMCodexLocalMatchInteractionCategory::RollPassControlAttack;
		const bool bThroughBallFeet = InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::RollThroughBallFeetAttack
			|| InteractionView.InteractionCategory
				== EFMCodexLocalMatchInteractionCategory
					::RollThroughBallFeetDefense;
		const bool bThroughBallBehind = InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory
				::RollThroughBallBehindDefenseAttack
			|| InteractionView.InteractionCategory
				== EFMCodexLocalMatchInteractionCategory
					::RollThroughBallBehindDefenseDefense;
		const bool bAntiOffside = InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory
				::RollThroughBallAntiOffsideAttack;
		const bool bChipShot = InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory
				::RollThroughBallOneOnOneChipShotAttack;
		const bool bDirectShot = InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory
				::RollThroughBallOneOnOneDirectShotAttack
			|| InteractionView.InteractionCategory
				== EFMCodexLocalMatchInteractionCategory
					::RollThroughBallOneOnOneDirectShotDefense;
		const bool bLongShotDirect = InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::RollLongShotDirectAttack
			|| InteractionView.InteractionCategory
				== EFMCodexLocalMatchInteractionCategory::RollLongShotDirectDefense;
		const bool bLongShotDead = InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::RollLongShotDeadCorner;
		const bool bCutInsideDirect = InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory
				::RollCutInsideShotDirectAttack
			|| InteractionView.InteractionCategory
				== EFMCodexLocalMatchInteractionCategory
					::RollCutInsideShotDirectDefense;
		const bool bCutInsideDead = InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory
				::RollCutInsideShotDeadCorner;
		const bool bPassControl = InteractionView.InteractionCategory
				== EFMCodexLocalMatchInteractionCategory::RollPassControlAttack
			|| InteractionView.InteractionCategory
				== EFMCodexLocalMatchInteractionCategory::RollPassControlDefense;
		Result.Interaction.CrossRollRevealKind = bLongShotDead
			? EFMCodexUMGCrossRollRevealKind::LongShotDeadCornerA
			: bCutInsideDead
			? EFMCodexUMGCrossRollRevealKind::CutInsideShotDeadCornerA
			: bAttackRoll
			? EFMCodexUMGCrossRollRevealKind::Attack
			: EFMCodexUMGCrossRollRevealKind::Defense;
		Result.Interaction.CrossRollContestId = bAntiOffside
			? FName(TEXT("ThroughBall.AntiOffside"))
			: bChipShot ? FName(TEXT("ThroughBall.OneOnOne.ChipShot"))
			: bDirectShot ? FName(TEXT("ThroughBall.OneOnOne.DirectShot"))
			: bLongShotDirect ? FName(TEXT("LongShot.DirectShot"))
			: bLongShotDead ? FName(TEXT("LongShot.DeadCorner"))
			: bCutInsideDirect ? FName(TEXT("CutInsideShot.DirectShot"))
			: bCutInsideDead ? FName(TEXT("CutInsideShot.DeadCorner"))
			: bPassControl
			? InteractionView.ResolutionFacts.ActualBranch.PassControl
				== EMatchPlayPassControlActualBranch::PassAdvance
					? FName(TEXT("PassControl.PassAdvance"))
					: InteractionView.ResolutionFacts.ActualBranch.PassControl
						== EMatchPlayPassControlActualBranch::DribbleAdvance
							? FName(TEXT("PassControl.DribbleAdvance"))
							: FName(TEXT("PassControl.RunAdvance"))
			: bThroughBallBehind
			? FName(TEXT("ThroughBall.BehindDefense.P1"))
			: bThroughBallFeet ? FName(TEXT("ThroughBall.Feet"))
			: InteractionView.ResolutionFacts.ActualBranch.Cross
				== EMatchPlayCrossActualBranch::High
					? FName(TEXT("Cross.High"))
					: FName(TEXT("Cross.Low"));
		Result.Interaction.CrossRollSequenceIndex =
			InteractionView.ResolutionFacts.NextPendingRollSequenceIndex;
		Result.Interaction.CrossRollOwnerSide =
			InteractionView.ExpectedActingPlayer;
	}
	Result.Interaction.bCanStartNewMatch =
		InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::StartMatch;
	Result.Interaction.bCanRollTacticalPoints =
		InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::TacticalPointRoll;
	Result.Interaction.bHasActingSidePrimaryColor =
		InteractionView.ExpectedActingPlayer
			== EInitialTurnOrderPlayer::PlayerA
		|| InteractionView.ExpectedActingPlayer
			== EInitialTurnOrderPlayer::PlayerB;
	if (Result.Interaction.bHasActingSidePrimaryColor)
	{
		Result.Interaction.ActingSidePrimaryColor = PrimaryColorForSide(
			InteractionView.ExpectedActingPlayer, SidePrimaryColors);
	}
	Result.Interaction.bCanFinishDeployment =
		InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::Deploy;
	Result.Interaction.bCanDecline = InteractionView.bCanDecline;
	Result.Interaction.bCanResolveNoLegal =
		InteractionView.bCanResolveNoLegalChoice;
	const bool bCanContinue =
		InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::ContinueResolution
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::RollCrossRoute
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory
				::RollThroughBallInitialRoute
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::RollCrossAttack
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::RollCrossDefense
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::RollThroughBallFeetAttack
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::RollThroughBallFeetDefense
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory
				::RollThroughBallAntiOffsideAttack
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory
				::RollThroughBallOneOnOneChipShotAttack
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory
				::RollThroughBallOneOnOneDirectShotAttack
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory
				::RollThroughBallOneOnOneDirectShotDefense
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory
				::RollThroughBallBehindDefenseAttack
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory
				::RollThroughBallBehindDefenseDefense
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::RollLongShotDirectAttack
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::RollLongShotDirectDefense
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::RollLongShotDeadCorner
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory
				::RollCutInsideShotDirectAttack
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory
				::RollCutInsideShotDirectDefense
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory
				::RollCutInsideShotDeadCorner
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::RollPassControlRoute
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::RollPassControlAttack
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::RollPassControlDefense
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory
				::ApplyCrossTerminalResolution
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory
				::ApplyThroughBallFeetTerminalResolution
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::AdvanceAfterTerminal;
	const bool bSetPiecePrimaryAction =
		InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::RollSetPieceType
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::ConfirmSetPieceCarrier
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::DraftCornerAttacker
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::DraftCornerDefender
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::RollShortFreeKickDirectAttack
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::RollShortFreeKickDirectDefense
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::RollShortFreeKickAngled
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::RollLongFreeKickDirectAttack
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::RollLongFreeKickDirectDefense
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::RollLongFreeKickPower
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::RollPenaltyDirectAttack
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::RollPenaltyDirectDefense
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::RollPenaltyPanenka
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::RollCornerParticipantSelection
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::RollCornerRoute
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::RollCornerAttack
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::RollCornerDefense;
	const bool bCanContinueWithSetPiece = bCanContinue || bSetPiecePrimaryAction;
	Result.Interaction.PrimaryAction.bAvailable =
		Result.Interaction.bCanStartNewMatch
		|| Result.Interaction.bCanRollTacticalPoints
		|| Result.Interaction.bCanFinishDeployment
		|| bCanContinueWithSetPiece;
	Result.Interaction.PrimaryAction.Category = Result.Interaction.Category;
	Result.Interaction.PrimaryAction.Label =
		Result.Interaction.bCanStartNewMatch ? TEXT("START LOCAL MATCH")
		: Result.Interaction.bCanRollTacticalPoints
			? TEXT("ROLL TACTICAL POINTS")
		: Result.Interaction.bCanFinishDeployment ? TEXT("FINISH DEPLOYMENT")
		: bCanContinueWithSetPiece
			? (InteractionView.InteractionCategory
				== EFMCodexLocalMatchInteractionCategory
					::RollCutInsideShotDirectAttack
					? FFMCodexPlayerUIPresentationText
						::ShotAttackRollAction().ToString()
			: InteractionView.InteractionCategory
				== EFMCodexLocalMatchInteractionCategory
					::RollCutInsideShotDirectDefense
					? FFMCodexPlayerUIPresentationText
						::ShotDefenseRollAction().ToString()
			: InteractionView.InteractionCategory
				== EFMCodexLocalMatchInteractionCategory
					::RollCutInsideShotDeadCorner
					? FFMCodexPlayerUIPresentationText
						::ShotPairedRollAction().ToString()
			: bThroughBallRoutePending
				? FFMCodexPlayerUIPresentationText
					::ThroughBallInitialRouteAction().ToString()
				: bPassControlRoutePending
					? FString(TEXT("判定推进方式"))
				: InteractionView.InteractionCategory
					== EFMCodexLocalMatchInteractionCategory::RollPassControlAttack
						? FString(TEXT("进攻方掷点"))
				: InteractionView.InteractionCategory
					== EFMCodexLocalMatchInteractionCategory::RollPassControlDefense
						? FString(TEXT("防守方掷点"))
				: bCrossRoutePending
					? (InteractionView.ContinueActionLabel.IsEmpty()
						? FString(TEXT("判定传中路线"))
						: InteractionView.ContinueActionLabel)
					: InteractionView.InteractionCategory
						== EFMCodexLocalMatchInteractionCategory
							::RollThroughBallFeetAttack
							? FString(TEXT("进攻方掷点"))
					: InteractionView.InteractionCategory
						== EFMCodexLocalMatchInteractionCategory
							::RollThroughBallFeetDefense
							? FString(TEXT("防守方掷点"))
					: InteractionView.InteractionCategory
						== EFMCodexLocalMatchInteractionCategory
							::RollThroughBallBehindDefenseAttack
								? FString(TEXT("进攻方掷点"))
						: InteractionView.InteractionCategory
							== EFMCodexLocalMatchInteractionCategory
								::RollThroughBallBehindDefenseDefense
								? FString(TEXT("防守方掷点"))
								: InteractionView.InteractionCategory
									== EFMCodexLocalMatchInteractionCategory
										::RollThroughBallAntiOffsideAttack
										? FString(TEXT("掷点判定越位"))
								: InteractionView.InteractionCategory
									== EFMCodexLocalMatchInteractionCategory
										::RollThroughBallOneOnOneChipShotAttack
										? FString(TEXT("挑射掷点"))
								: InteractionView.InteractionCategory
									== EFMCodexLocalMatchInteractionCategory
										::RollThroughBallOneOnOneDirectShotAttack
										? FString(TEXT("进攻方掷点"))
								: InteractionView.InteractionCategory
									== EFMCodexLocalMatchInteractionCategory
										::RollThroughBallOneOnOneDirectShotDefense
										? FString(TEXT("防守方掷点"))
										: (InteractionView.ContinueActionLabel.IsEmpty()
									? FString(TEXT("继续结算"))
					: InteractionView.ContinueActionLabel))
			: FString();
	Result.Interaction.bCanContinue = bCanContinueWithSetPiece;
	Result.Interaction.PrimaryActionLabel =
		Result.Interaction.PrimaryAction.Label;
	Result.Interaction.DeclineActionLabel = InteractionView.bCanDecline
		? DeclineLabel(InteractionView.InteractionCategory) : FString();
	Result.Interaction.NoLegalActionLabel =
		InteractionView.bCanResolveNoLegalChoice
			? NoLegalLabel(InteractionView.InteractionCategory) : FString();
	Result.Interaction.BranchSectionLabel =
		InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::SelectBranchIntent
			|| InteractionView.InteractionCategory
				== EFMCodexLocalMatchInteractionCategory::SelectLongShotBranch
				? (InteractionView.ActionLabel == TEXT("Cross")
					? FString(TEXT("CROSS TYPE")) : FString(TEXT("SHOT TYPE")))
				: InteractionView.InteractionCategory
					== EFMCodexLocalMatchInteractionCategory::SelectOneOnOneShot
						? FString(TEXT("ONE-ON-ONE | CHOOSE SHOT"))
						: FString();
	Result.Interaction.EmptyStateLabel =
		InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::MatchEnded
				? TEXT("MATCH COMPLETE | No gameplay progression is available.")
				: InteractionView.InteractionCategory
					== EFMCodexLocalMatchInteractionCategory::AttackComplete
						? TEXT("ATTACK COMPLETE | Waiting for the next match state.")
						: InteractionView.InteractionCategory
							== EFMCodexLocalMatchInteractionCategory::None
								? TEXT("Interaction unavailable.") : FString();
	if (!InteractionView.ActionLabel.IsEmpty())
	{
		Result.Interaction.LegalActionLabels.Add(InteractionView.ActionLabel);
	}
	for (const FFMCodexLocalMatchDeploymentGroup& Group
		: InteractionView.DeploymentGroups)
	{
		AddUniqueCard(Result.Interaction.CandidateCards, Group.Card);
		FFMCodexUMGDeploymentChoiceViewModel Choice;
		Choice.CardId = Group.CardId;
		Choice.bGoalkeeper = Group.bGoalkeeper;
		Choice.Card = MakeCard(Group.Card);
		for (const FFMCodexLocalMatchSlotView& Slot : Group.LegalSlots)
		{
			Choice.Destinations.Add({ Slot.SlotId, Slot.Label });
		}
		Result.Interaction.DeploymentChoices.Add(MoveTemp(Choice));
	}
	for (const FFMCodexLocalMatchSelectionOption& Option
		: InteractionView.SelectionOptions)
	{
		const FString PlayerFacingOptionLabel =
			InteractionView.InteractionCategory
				== EFMCodexLocalMatchInteractionCategory::SelectSkill
					? FFMCodexPlayerUIPresentationText::Skill(
						Option.Label).ToString()
					: Option.Label;
		Result.Interaction.LegalActionLabels.Add(PlayerFacingOptionLabel);
		if (Option.bHasCard)
		{
			AddUniqueCard(Result.Interaction.CandidateCards, Option.Card);
		}
		FFMCodexUMGSelectionChoiceViewModel Choice;
		Choice.OptionId = Option.Id;
		Choice.Label = PlayerFacingOptionLabel;
		Choice.SkillType = Option.SkillType;
		if (InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::SelectSkill)
		{
			Choice.SecondaryLabel =
				FFMCodexTacticalDetailPresentationBuilder::Build(
					Option.SkillType).CardHint;
		}
		Choice.bHasCard = Option.bHasCard;
		if (Option.bHasCard)
		{
			Choice.Card = MakeCard(Option.Card);
		}
		Result.Interaction.SelectionChoices.Add(MoveTemp(Choice));
	}
	for (const EMatchPlayElectiveBranchIntent Intent
		: InteractionView.BranchIntentOptions)
	{
		const FString Label = BranchIntentLabel(Intent);
		FString SecondaryLabel;
		if (InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::SelectLongShotBranch)
		{
			SecondaryLabel = Intent
					== EMatchPlayElectiveBranchIntent::DirectShot
				? FFMCodexPlayerUIPresentationText::LongShotDirectChoiceHint()
					.ToString()
				: Intent == EMatchPlayElectiveBranchIntent::DeadCorner
					? FFMCodexPlayerUIPresentationText
						::LongShotDeadCornerChoiceHint().ToString()
					: FString();
		}
		Result.Interaction.LegalActionLabels.Add(Label);
		Result.Interaction.BranchChoices.Add({
			BranchIntent(Intent), Label, SecondaryLabel });
	}
	for (const EMatchPlayThroughBallOneOnOneShotChoice Choice
		: InteractionView.OneOnOneOptions)
	{
		const EFMCodexUMGOneOnOneChoice PlayerChoice = OneOnOneChoice(Choice);
		const FString PlayerFacingChoiceLabel =
			FFMCodexPlayerUIPresentationText::MatchScreenLabel(
				FFMCodexLocalMatchInteractionViewBuilder::ToString(Choice))
				.ToString();
		const FString SecondaryLabel = PlayerChoice
				== EFMCodexUMGOneOnOneChoice::DirectShot
			? FFMCodexPlayerUIPresentationText::ThroughBallDirectChoiceHint()
				.ToString()
			: PlayerChoice == EFMCodexUMGOneOnOneChoice::ChipShot
				? FFMCodexPlayerUIPresentationText::ThroughBallChipChoiceHint()
					.ToString()
				: FString();
		Result.Interaction.LegalActionLabels.Add(PlayerFacingChoiceLabel);
		Result.Interaction.OneOnOneChoices.Add({
			PlayerChoice, PlayerFacingChoiceLabel, SecondaryLabel });
	}

	// Terminal feedback remains available for diagnostics, but attack completion
	// has already exposed the next authoritative between-attacks interaction.
	// Do not let the old full-screen result mask the new attacker's roll intent.
	Result.Resolution.bVisible = ResolutionFeedback.bVisible
		&& !(ResolutionFeedback.bTerminal
			&& !InteractionView.bCurrentAttackActive);
	Result.Resolution.bRejected = ResolutionFeedback.bRejected;
	Result.Resolution.bTerminal = ResolutionFeedback.bTerminal;
	Result.Resolution.bNonBlockingNotification =
		ResolutionFeedback.bNonBlockingNotification;
	Result.Resolution.bCanContinue = bCanContinue;
	Result.Resolution.ContinueActionLabel =
		bCanContinue ? Result.Interaction.PrimaryAction.Label : FString();
	Result.Resolution.StepLabel = ResolutionFeedback.StepTitle.IsEmpty()
		? ResolutionFeedback.StepSummary : ResolutionFeedback.StepTitle;
	Result.Resolution.StepSummaryLabel = ResolutionFeedback.StepSummary;
	Result.Resolution.RouteLabel = ResolutionFeedback.RouteSummary;
	Result.Resolution.FormulaFacts = ResolutionFeedback.ResolutionFacts.bHasFacts
		? ResolutionFeedback.ResolutionFacts
		: InteractionView.ResolutionFacts;
	if (!ResolutionFeedback.bRejected)
	{
		for (const FFMCodexLocalMatchRollView& Roll
			: ResolutionFeedback.DiceEntries)
		{
			Result.Resolution.DiceResults.Add({
				DiceContextLabel(Roll.Group), Roll.Purpose, Roll.RawD6 });
			Result.Resolution.DiceLabels.Add(FString::Printf(
				TEXT("D6 %d | %s"), Roll.RawD6, *Roll.Purpose));
		}
		for (int32 Index = 0;
			Index < ResolutionFeedback.ComparisonEntries.Num(); ++Index)
		{
			const FString Heading = Index == 0 ? TEXT("ATTACK")
				: Index == 1 ? TEXT("DEFENSE") : TEXT("EVIDENCE");
			Result.Resolution.ComparisonEvidence.Add({
				Heading, ResolutionFeedback.ComparisonEntries[Index] });
		}
	}
	Result.Resolution.DecisionLabel = ResolutionFeedback.DecisionSummary;
	Result.Resolution.ContinuationLabel =
		ResolutionFeedback.ContinuationSummary;
	Result.Resolution.TerminalLabel = ResolutionFeedback.TerminalSummary;
	Result.Resolution.ErrorLabel = ResolutionFeedback.ErrorMessage;
	const FFMCodexUMGInlineFormulaSurfaceViewModel ProjectedFormula =
		BuildInlineFormulaSurface(
		Result.Resolution.FormulaFacts,
		InteractionView,
		Result.Interaction.PrimaryAction,
		!Result.Resolution.bRejected
			&& InteractionView.bCurrentAttackActive);
	if (InteractionView.PresentedActionType != ESkillRuleType::ThroughBall
		&& InteractionView.PresentedActionType != ESkillRuleType::LongShot
		&& InteractionView.PresentedActionType != ESkillRuleType::CutInsideShot
		&& InteractionView.PresentedActionType != ESkillRuleType::PassControl)
	{
		Result.InlineFormula = ProjectedFormula;
	}
	const FFMCodexUMGInlineFormulaSurfaceViewModel SetPieceFormula =
		BuildSetPieceFormulaSurface(
			InteractionView, Result.Interaction.PrimaryAction);
	if (SetPieceFormula.bVisible || !SetPieceFormula.ContestId.IsNone())
	{
		// Set Piece method rolls return to the same central Formula/Reel/CTA
		// component as mature ordinary tactics. Only genuinely unique setup
		// phases remain on the Set Piece-specific surface.
		Result.InlineFormula = SetPieceFormula;
	}
	// Once Cross High has been selected, its route/formula progression stays on
	// the board. The legacy English Resolution overlay remains available to
	// routes outside this narrow rollout.
	if ((InteractionView.ElectiveBranchIntent
			== EMatchPlayElectiveBranchIntent::CrossHigh
			|| InteractionView.ElectiveBranchIntent
				== EMatchPlayElectiveBranchIntent::CrossLow)
		&& !Result.Resolution.bRejected
		&& !Result.Resolution.FormulaFacts.bHasActualBranch)
	{
		Result.InlineFormula.bSuppressLegacyResolution = true;
		if (bCrossRoutePending)
		{
			Result.InlineFormula.bVisible = true;
			Result.InlineFormula.ContestId = TEXT("Cross.Route");
			Result.InlineFormula.ContestLabel = TEXT("传中");
			Result.InlineFormula.StatusLabel = TEXT("等待路线掷点");
			Result.InlineFormula.RollHelperLabel = FFMCodexTacticalDetailPresentationBuilder::BuildCrossRouteHint(
				InteractionView.ElectiveBranchIntent).ToString();
			Result.InlineFormula.bShowFormulaRows = false;
			ClaimPrimaryAction(
				Result.InlineFormula.PrimaryAction,
				Result.Interaction.PrimaryAction);
			Result.InlineFormula.bCanContinue =
				Result.InlineFormula.PrimaryAction.bVisible;
			Result.InlineFormula.ContinueActionLabel =
				Result.InlineFormula.PrimaryAction.Action.Label;
		}
	}
	Result.ThroughBallResolution = BuildThroughBallSurface(
		InteractionView,
		Result.Interaction,
		ProjectedFormula,
		Result.Resolution.bRejected);
	if (Result.ThroughBallResolution.bVisible)
	{
		// The shared CTA dock remains authoritative, but ThroughBall production
		// presentation does not expose engineering classification/state labels.
		Result.Interaction.KickerLabel = TEXT("直塞");
		Result.Interaction.ClassificationLabel.Empty();
		Result.Interaction.CategoryLabel.Empty();
	}
	Result.LongShotResolution = BuildLongShotSurface(
		InteractionView,
		Result.Interaction,
		ProjectedFormula,
		Result.Resolution.bRejected);
	if (Result.LongShotResolution.bVisible)
	{
		Result.Interaction.KickerLabel = Result.LongShotResolution.TitleLabel;
		Result.Interaction.ClassificationLabel.Empty();
		Result.Interaction.CategoryLabel.Empty();
	}

	Result.DiagnosticLabel = DiagnosticMessage;
	Result.FullTime = InteractionView.FullTime;
	// Reuse header identity, mapped by authoritative side rather than viewer position.
	// A future runtime participant name can supply this DTO without touching match rules.
	Result.FullTime.PlayerA.PlayerDisplayName = FFMCodexPlayerUIPresentationText::MatchScreenLabel(
		LocalViewerSide == EInitialTurnOrderPlayer::PlayerA ? Result.Header.LeftPlayerLabel : Result.Header.RightPlayerLabel);
	Result.FullTime.PlayerB.PlayerDisplayName = FFMCodexPlayerUIPresentationText::MatchScreenLabel(
		LocalViewerSide == EInitialTurnOrderPlayer::PlayerB ? Result.Header.LeftPlayerLabel : Result.Header.RightPlayerLabel);
	if (Result.FullTime.bVisible)
	{
		Result.PitchRegions.Reset();
		Result.Resolution = FFMCodexUMGResolutionViewModel();
		Result.ThroughBallResolution = FFMCodexUMGThroughBallResolutionViewModel();
		Result.LongShotResolution = FFMCodexUMGLongShotResolutionViewModel();
		Result.InlineFormula = FFMCodexUMGInlineFormulaSurfaceViewModel();
	}
	return Result;
}
