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
			Cell.bPlayed = Card.bUsed || Card.bDeployed;
			Cell.bGoalkeeper = Card.bGoalkeeper;
			Cell.bDeploymentDraggable = bLocalRack && !Cell.bPlayed
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
		if (!bResolvedCross && !bResolvedThroughBallFeet
			&& !bResolvedThroughBallBehind && !bResolvedThroughBallDirect)
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
		if (RouteRoll == nullptr)
		{
			return Result;
		}
		const bool bCrossHigh = bResolvedCross
			&& Facts.ActualBranch.Cross == EMatchPlayCrossActualBranch::High;
		Result.RouteResultLabel = FString::Printf(
			TEXT("路线掷点 %d \u2192 判定为%s"),
			RouteRoll->RawD6,
			bResolvedCross
				? (bCrossHigh ? TEXT("高球传中") : TEXT("低球传中"))
				: bResolvedThroughBallFeet ? TEXT("脚下球")
					: Facts.ActualBranch.ThroughBall
						== EMatchPlayThroughBallActualBranch::AntiOffside
							? TEXT("反越位") : TEXT("身后球"));

		const FName ContestId = bResolvedCross
			? (bCrossHigh ? FName(TEXT("Cross.High"))
				: FName(TEXT("Cross.Low")))
			: bResolvedThroughBallDirect
				? FName(TEXT("ThroughBall.OneOnOne.DirectShot"))
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
		Result.bShowFormulaRows = !bBehindOutOfPlay;
		Result.StatusLabel = !bAttackResolved
			? TEXT("等待进攻方掷点")
			: !bDefenseResolved
				? TEXT("等待防守方掷点")
				: TEXT("双方掷点已完成");
		const bool bTerminalPresentationReady = bResolvedThroughBallDirect
			? bDirectOutcomeResolved
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
		const bool bNarrativeReady = bResolvedThroughBallBehind
			? bBehindNarrativeReady
			: bAttackResolved && bDefenseResolved
				&& Contest->bHasResolvedFormula
				&& (bResolvedThroughBallDirect
					? bDirectOutcomeResolved
					: Contest->ResolvedResult.Winner != EFormulaWinner::None)
				&& Contest->ResolvedResult.bAttackEnded
				&& !Contest->ResolvedResult.bContinueResolution
				&& bTerminalPresentationReady
				&& PrimaryAction.bAvailable;
		if (bNarrativeReady)
		{
			Result.bNarrativeAttackSuccess = bResolvedThroughBallDirect
				? DirectDecision->Outcome
					== EMatchPlayResolutionDecisionOutcome::Goal
				: bResolvedThroughBallBehind
				? BehindDecision->Outcome
					== EMatchPlayResolutionDecisionOutcome::OneOnOneRequired
				: Contest->ResolvedResult.Winner == EFormulaWinner::Attacker;
			FFMCodexTacticalNarrativePresentationInput NarrativeInput;
			NarrativeInput.Branch = bResolvedThroughBallDirect
				? EFMCodexTacticalNarrativeBranch::ThroughBallOneOnOneDirect
				: bResolvedThroughBallBehind
				? EFMCodexTacticalNarrativeBranch::ThroughBallBehindDefense
				: !bResolvedCross
					? EFMCodexTacticalNarrativeBranch::ThroughBallFeet
				: bCrossHigh
					? EFMCodexTacticalNarrativeBranch::CrossHigh
					: EFMCodexTacticalNarrativeBranch::CrossLow;
			NarrativeInput.AuthorityOutcome = bResolvedThroughBallDirect
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
				const FString RouteLabel = bResolvedThroughBallDirect
					? TEXT("单刀")
					: bResolvedThroughBallBehind
					? TEXT("身后球")
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
		if (InteractionView.PresentedActionType != ESkillRuleType::ThroughBall
			|| InteractionView.MajorPhase
				!= EFMCodexLocalMatchMajorPhase::Resolution
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
					== EFMCodexUMGInteractionCategory::ContinueResolution;
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

	Result.Header.ScoreLabel = FString::Printf(
		TEXT("%d - %d"), InteractionView.PlayerAScore,
		InteractionView.PlayerBScore);
	Result.Header.PlayerAScoreLabel = FString::FromInt(
		InteractionView.PlayerAScore);
	Result.Header.PlayerBScoreLabel = FString::FromInt(
		InteractionView.PlayerBScore);
	const EInitialTurnOrderPlayer OpponentSide = OtherSide(LocalViewerSide);
	Result.LocalPlayerLabel =
		FFMCodexLocalMatchInteractionViewBuilder::ToString(LocalViewerSide);
	Result.Header.LeftPlayerLabel = Result.LocalPlayerLabel;
	Result.Header.RightPlayerLabel =
		FFMCodexLocalMatchInteractionViewBuilder::ToString(OpponentSide);
	Result.Header.LeftScoreLabel = FString::FromInt(
		LocalViewerSide == EInitialTurnOrderPlayer::PlayerA
			? InteractionView.PlayerAScore : InteractionView.PlayerBScore);
	Result.Header.RightScoreLabel = FString::FromInt(
		OpponentSide == EInitialTurnOrderPlayer::PlayerA
			? InteractionView.PlayerAScore : InteractionView.PlayerBScore);
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
		InteractionView.bCurrentAttackActive;
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
		true,
		Result.Header.LeftPlayerLabel,
		Result.LocalRack);
	BuildRack(
		OpponentSide == EInitialTurnOrderPlayer::PlayerA
			? InteractionView.PlayerACardRoster
			: InteractionView.PlayerBCardRoster,
		InteractionView.DeploymentGroups,
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
			== EFMCodexLocalMatchInteractionCategory::ContinueResolution
		&& (InteractionView.ElectiveBranchIntent
				== EMatchPlayElectiveBranchIntent::CrossHigh
			|| InteractionView.ElectiveBranchIntent
				== EMatchPlayElectiveBranchIntent::CrossLow)
		&& !InteractionView.ResolutionFacts.bHasActualBranch;
	const bool bThroughBallRoutePending =
		InteractionView.PresentedActionType == ESkillRuleType::ThroughBall
		&& InteractionView.MajorPhase
			== EFMCodexLocalMatchMajorPhase::Resolution
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
	else if (InteractionView.InteractionCategory
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
				::RollThroughBallBehindDefenseDefense)
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
					::RollThroughBallOneOnOneDirectShotAttack;
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
		Result.Interaction.CrossRollRevealKind = bAttackRoll
			? EFMCodexUMGCrossRollRevealKind::Attack
			: EFMCodexUMGCrossRollRevealKind::Defense;
		Result.Interaction.CrossRollContestId = bAntiOffside
			? FName(TEXT("ThroughBall.AntiOffside"))
			: bChipShot ? FName(TEXT("ThroughBall.OneOnOne.ChipShot"))
			: bDirectShot ? FName(TEXT("ThroughBall.OneOnOne.DirectShot"))
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
			== EFMCodexLocalMatchInteractionCategory
				::ApplyCrossTerminalResolution
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory
				::ApplyThroughBallFeetTerminalResolution
		|| InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::AdvanceAfterTerminal;
	Result.Interaction.PrimaryAction.bAvailable =
		Result.Interaction.bCanStartNewMatch
		|| Result.Interaction.bCanRollTacticalPoints
		|| Result.Interaction.bCanFinishDeployment
		|| bCanContinue;
	Result.Interaction.PrimaryAction.Category = Result.Interaction.Category;
	Result.Interaction.PrimaryAction.Label =
		Result.Interaction.bCanStartNewMatch ? TEXT("START LOCAL MATCH")
		: Result.Interaction.bCanRollTacticalPoints
			? TEXT("ROLL TACTICAL POINTS")
		: Result.Interaction.bCanFinishDeployment ? TEXT("FINISH DEPLOYMENT")
		: bCanContinue
			? bThroughBallRoutePending
				? FFMCodexPlayerUIPresentationText
					::ThroughBallInitialRouteAction().ToString()
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
									? FString(TEXT("CONTINUE"))
									: InteractionView.ContinueActionLabel)
			: FString();
	Result.Interaction.bCanContinue = bCanContinue;
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
		Result.Interaction.LegalActionLabels.Add(BranchIntentLabel(Intent));
		Result.Interaction.BranchChoices.Add({
			BranchIntent(Intent), BranchIntentLabel(Intent) });
	}
	for (const EMatchPlayThroughBallOneOnOneShotChoice Choice
		: InteractionView.OneOnOneOptions)
	{
		const FString PlayerFacingChoiceLabel =
			FFMCodexPlayerUIPresentationText::MatchScreenLabel(
				FFMCodexLocalMatchInteractionViewBuilder::ToString(Choice))
				.ToString();
		Result.Interaction.LegalActionLabels.Add(PlayerFacingChoiceLabel);
		Result.Interaction.OneOnOneChoices.Add({
			OneOnOneChoice(Choice), PlayerFacingChoiceLabel });
	}

	// Terminal feedback remains available for diagnostics, but attack completion
	// has already exposed the next authoritative between-attacks interaction.
	// Do not let the old full-screen result mask the new attacker's roll intent.
	Result.Resolution.bVisible = ResolutionFeedback.bVisible
		&& !(ResolutionFeedback.bTerminal
			&& !InteractionView.bCurrentAttackActive);
	Result.Resolution.bRejected = ResolutionFeedback.bRejected;
	Result.Resolution.bTerminal = ResolutionFeedback.bTerminal;
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
	if (InteractionView.PresentedActionType != ESkillRuleType::ThroughBall)
	{
		Result.InlineFormula = ProjectedFormula;
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

	Result.DiagnosticLabel = DiagnosticMessage;
	return Result;
}
