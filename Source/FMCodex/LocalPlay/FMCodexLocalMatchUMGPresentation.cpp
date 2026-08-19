#include "FMCodexLocalMatchUMGPresentation.h"

#include "FMCodexPlayerUIPresentationText.h"

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

	EInitialTurnOrderPlayer OtherSide(const EInitialTurnOrderPlayer Side)
	{
		return Side == EInitialTurnOrderPlayer::PlayerB
			? EInitialTurnOrderPlayer::PlayerA
			: EInitialTurnOrderPlayer::PlayerB;
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
		switch (Intent)
		{
		case EMatchPlayElectiveBranchIntent::DirectShot:
			return TEXT("Direct Shot");
		case EMatchPlayElectiveBranchIntent::DeadCorner:
			return TEXT("Dead Corner");
		case EMatchPlayElectiveBranchIntent::CrossHigh:
			return TEXT("Cross High");
		case EMatchPlayElectiveBranchIntent::CrossLow:
			return TEXT("Cross Low");
		default:
			return TEXT("Unknown Branch");
		}
	}

	EFMCodexUMGInteractionCategory InteractionCategory(
		const EFMCodexLocalMatchInteractionCategory Category)
	{
		switch (Category)
		{
		case EFMCodexLocalMatchInteractionCategory::StartMatch:
			return EFMCodexUMGInteractionCategory::StartMatch;
		case EFMCodexLocalMatchInteractionCategory::BeginAttack:
			return EFMCodexUMGInteractionCategory::BeginAttack;
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
			return TEXT("RESOLVE NO LEGAL SKILL");
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
	const bool bAwaitingHandoff,
	const FString& PendingPlayerLabel,
	const EInitialTurnOrderPlayer LocalViewerSide)
{
	using namespace FMCodexLocalMatchUMGPresentation;
	FFMCodexUMGMatchScreenViewModel Result;
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
	Result.Header.TurnLabel = InteractionView.AttackSequence > 0
		? FString::Printf(TEXT("TURN %lld"), InteractionView.AttackSequence)
		: TEXT("PRE-MATCH");
	Result.Header.CurrentAttackerTacticalPointsLabel =
		InteractionView.CurrentAttackingPlayer != EInitialTurnOrderPlayer::None
			? FString::Printf(TEXT("TACTICAL POINTS  %d"),
				InteractionView.ActionPoint)
			: FString();
	Result.Header.AttackSequence = InteractionView.AttackSequence;
	Result.Header.CurrentAttackerTacticalPoints = InteractionView.ActionPoint;
	Result.Header.bHasCurrentAttacker =
		InteractionView.CurrentAttackingPlayer != EInitialTurnOrderPlayer::None;
	Result.Header.bCurrentAttackerOnLeft =
		InteractionView.CurrentAttackingPlayer == LocalViewerSide;
	Result.Header.CurrentAttackerLabel = FString::Printf(
		TEXT("Current attacker: %s"),
		*FFMCodexLocalMatchInteractionViewBuilder::ToString(
			InteractionView.CurrentAttackingPlayer));
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
			}
			RegionResult.Slots.Add(MoveTemp(SlotResult));
		}
		Result.PitchRegions.Add(MoveTemp(RegionResult));
	}

	Result.Interaction.KickerLabel = Screen.InteractionKicker;
	Result.Interaction.TitleLabel = Screen.InteractionTitle;
	Result.Interaction.Category = InteractionCategory(
		InteractionView.InteractionCategory);
	Result.Interaction.ClassificationLabel = Screen.bSystemResolution
		? TEXT("SYSTEM") : InteractionView.bHumanInteraction
			? TEXT("HUMAN INPUT") : TEXT("INFORMATION");
	Result.Interaction.CategoryLabel =
		FFMCodexLocalMatchInteractionViewBuilder::ToString(
			InteractionView.InteractionCategory);
	Result.Interaction.ExpectedActorLabel = Screen.ActingStatusLabel;
	// Persistent Tactical Points are a Header fact. The Dock presentation does
	// not duplicate them; it remains operation-context focused.
	Result.Interaction.ActionPointLabel = FString();
	Result.Interaction.bSystemResolution = Screen.bSystemResolution;
	Result.Interaction.bMatchEnded = InteractionView.bMatchEnded;
	Result.Interaction.bCanStartNewMatch =
		InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::StartMatch;
	Result.Interaction.bCanBeginOrdinaryAttack =
		InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::BeginAttack;
	Result.Interaction.bCanFinishDeployment =
		InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::Deploy;
	Result.Interaction.bCanDecline = InteractionView.bCanDecline;
	Result.Interaction.bCanResolveNoLegal =
		InteractionView.bCanResolveNoLegalChoice;
	Result.Interaction.bCanContinue =
		InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::ContinueResolution;
	Result.Interaction.PrimaryActionLabel =
		Result.Interaction.bCanStartNewMatch ? TEXT("START LOCAL MATCH")
		: Result.Interaction.bCanBeginOrdinaryAttack ? TEXT("BEGIN ATTACK")
		: Result.Interaction.bCanFinishDeployment ? TEXT("FINISH DEPLOYMENT")
		: Result.Interaction.bCanContinue
			? (InteractionView.ContinueActionLabel.IsEmpty()
				? FString(TEXT("CONTINUE")) : InteractionView.ContinueActionLabel)
			: FString();
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
		Result.Interaction.LegalActionLabels.Add(Option.Label);
		if (Option.bHasCard)
		{
			AddUniqueCard(Result.Interaction.CandidateCards, Option.Card);
		}
		FFMCodexUMGSelectionChoiceViewModel Choice;
		Choice.OptionId = Option.Id;
		Choice.Label = Option.Label.IsEmpty()
			? Option.Id.ToString() : Option.Label;
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
		Result.Interaction.LegalActionLabels.Add(
			FFMCodexLocalMatchInteractionViewBuilder::ToString(Choice));
		Result.Interaction.OneOnOneChoices.Add({
			OneOnOneChoice(Choice),
			FFMCodexLocalMatchInteractionViewBuilder::ToString(Choice) });
	}

	Result.Resolution.bVisible = ResolutionFeedback.bVisible;
	Result.Resolution.bRejected = ResolutionFeedback.bRejected;
	Result.Resolution.bTerminal = ResolutionFeedback.bTerminal;
	Result.Resolution.StepLabel = ResolutionFeedback.StepTitle.IsEmpty()
		? ResolutionFeedback.StepSummary : ResolutionFeedback.StepTitle;
	Result.Resolution.StepSummaryLabel = ResolutionFeedback.StepSummary;
	Result.Resolution.RouteLabel = ResolutionFeedback.RouteSummary;
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

	Result.Handoff.bVisible = bAwaitingHandoff;
	Result.Handoff.NextPlayerLabel = FString::Printf(
		TEXT("Next Player: %s"), *PendingPlayerLabel);
	Result.DiagnosticLabel = DiagnosticMessage;
	return Result;
}
