#include "FMCodexLocalMatchInteractionView.h"
#include "../CoreRules/MatchPlayCornerResolution.h"

#include "../CoreRules/MatchEndResolver.h"
#include "../CoreRules/MatchPlayCurrentAttackCarrierSelectionAvailability.h"
#include "../CoreRules/MatchPlayCurrentAttackHelperSelectionAvailability.h"
#include "../CoreRules/MatchPlayCurrentAttackMarkerSelectionAvailability.h"
#include "../CoreRules/MatchPlayCurrentAttackPostRouteRollProgressQuery.h"
#include "../CoreRules/MatchPlayCurrentAttackResolveThroughBallAntiOffsideDecisionOrchestrator.h"
#include "../CoreRules/MatchPlayCurrentAttackResolveThroughBallBehindDefenseP1FormulaOrchestrator.h"
#include "../CoreRules/MatchPlayCurrentAttackRunnerSelectionAvailability.h"
#include "../CoreRules/MatchPlayCurrentAttackSkillSelectionAvailability.h"
#include "../CoreRules/MatchPlayDefendingGoalkeeperQuery.h"
#include "../CoreRules/MatchPlayGoalkeeperDeploymentAvailability.h"
#include "../CoreRules/MatchPlayFinishDeployment.h"
#include "../CoreRules/MatchPlayOrdinaryDeploymentAvailability.h"
#include "../CoreRules/MatchPlaySetPieceCarrierAvailability.h"
#include "../CoreRules/MatchPlaySetPieceParticipantEligibility.h"
#include "../CoreRules/MatchPlayTacticalPlayerAdvantageQuery.h"
#include "../CoreRules/SkillRuleSnapshotQuery.h"
#include "FMCodexPlayerOverall.h"
#include "FMCodexPlayerUIPresentationText.h"
#include "FMCodexPrototypeTeamContent.h"

namespace FMCodexLocalMatchInteractionView
{
	EInitialTurnOrderPlayer OtherSide(const EInitialTurnOrderPlayer Side)
	{
		return Side == EInitialTurnOrderPlayer::PlayerA
			? EInitialTurnOrderPlayer::PlayerB
			: Side == EInitialTurnOrderPlayer::PlayerB
				? EInitialTurnOrderPlayer::PlayerA
				: EInitialTurnOrderPlayer::None;
	}

	const TArray<FName>& AvailableCards(
		const FMatchPlayState& State,
		const EInitialTurnOrderPlayer Side)
	{
		return Side == EInitialTurnOrderPlayer::PlayerA
			? State.CardUsageState.PlayerACardUsageState.AvailableCardIds
			: State.CardUsageState.PlayerBCardUsageState.AvailableCardIds;
	}

	FName GoalkeeperCardId(
		const FMatchPlayState& State,
		const EInitialTurnOrderPlayer Side)
	{
		return FName(*(Side == EInitialTurnOrderPlayer::PlayerA
			? State.RuntimeState.PlayerAState.GoalkeeperCardId
			: State.RuntimeState.PlayerBState.GoalkeeperCardId));
	}

	const FCardUsageState& CardUsage(
		const FMatchPlayState& State,
		const EInitialTurnOrderPlayer Side)
	{
		return Side == EInitialTurnOrderPlayer::PlayerA
			? State.CardUsageState.PlayerACardUsageState
			: State.CardUsageState.PlayerBCardUsageState;
	}

	FString JoinLabels(const TArray<FString>& Labels)
	{
		return FString::Join(Labels, TEXT(" / "));
	}

	FString CompactRoleLabel(const TArray<EPlayerPositionType>& Positions)
	{
		TArray<FString> Labels;
		for (const EPlayerPositionType Position : Positions)
		{
			switch (Position)
			{
			case EPlayerPositionType::Attack: Labels.Add(TEXT("FW")); break;
			case EPlayerPositionType::Midfield: Labels.Add(TEXT("MF")); break;
			case EPlayerPositionType::Defense: Labels.Add(TEXT("DF")); break;
			case EPlayerPositionType::Goalkeeper: Labels.Add(TEXT("GK")); break;
			default: break;
			}
		}
		return Labels.IsEmpty() ? TEXT("ROLE N/A") : JoinLabels(Labels);
	}

	bool HasCompletedCrossTerminalContest(
		const FMatchPlayCurrentAttackResolutionFactProjection& Facts)
	{
		if (!Facts.bSuccess || !Facts.bHasFacts
			|| !Facts.bHasActualBranch
			|| Facts.ActualBranch.ActionType != ESkillRuleType::Cross)
		{
			return false;
		}
		const FName ContestId = Facts.ActualBranch.Cross
			== EMatchPlayCrossActualBranch::High
				? FName(TEXT("Cross.High")) : FName(TEXT("Cross.Low"));
		const FMatchPlayResolutionFormulaContestFact* Contest =
			Facts.FormulaContests.FindByPredicate(
				[ContestId](
					const FMatchPlayResolutionFormulaContestFact& Candidate)
				{
					return Candidate.ContestId == ContestId;
				});
		return Contest != nullptr
			&& Contest->bHasResolvedFormula
			&& Contest->AttackRow.bFinalValueResolved
			&& Contest->DefenseRow.bFinalValueResolved
			&& Contest->ResolvedResult.bAttackEnded
			&& !Contest->ResolvedResult.bContinueResolution;
	}

	bool HasCompletedThroughBallFeetTerminalContest(
		const FMatchPlayCurrentAttackResolutionFactProjection& Facts)
	{
		if (!Facts.bSuccess || !Facts.bHasFacts
			|| !Facts.bHasActualBranch
			|| Facts.ActualBranch.ActionType != ESkillRuleType::ThroughBall
			|| Facts.ActualBranch.ThroughBall
				!= EMatchPlayThroughBallActualBranch::Feet)
		{
			return false;
		}
		const FMatchPlayResolutionFormulaContestFact* Contest =
			Facts.FormulaContests.FindByPredicate(
				[](const FMatchPlayResolutionFormulaContestFact& Candidate)
				{
					return Candidate.ContestId == TEXT("ThroughBall.Feet");
				});
		return Contest != nullptr
			&& Contest->bHasResolvedFormula
			&& Contest->AttackRow.bFinalValueResolved
			&& Contest->DefenseRow.bFinalValueResolved
			&& Contest->ResolvedResult.bAttackEnded
			&& !Contest->ResolvedResult.bContinueResolution;
	}

	FString RarityLabel(const ECardRarity Rarity)
	{
		switch (Rarity)
		{
		case ECardRarity::Common: return TEXT("Common");
		case ECardRarity::Regional: return TEXT("Regional");
		case ECardRarity::National: return TEXT("National");
		case ECardRarity::Continental: return TEXT("Continental");
		case ECardRarity::WorldClass: return TEXT("World Class");
		default: return TEXT("Unknown Rarity");
		}
	}

	int32 RackSortGroup(const FPlayerCardRuleSnapshot& Card)
	{
		if (Card.bIsGoalkeeper
			|| Card.PositionTypes.Contains(EPlayerPositionType::Goalkeeper))
		{
			return 0;
		}
		const bool bAttack = Card.PositionTypes.Contains(
			EPlayerPositionType::Attack);
		const bool bMidfield = Card.PositionTypes.Contains(
			EPlayerPositionType::Midfield);
		const bool bDefense = Card.PositionTypes.Contains(
			EPlayerPositionType::Defense);
		// A/M and A/M/D belong to the M group; M/D belongs to D.
		if (bAttack && bMidfield)
		{
			return 2;
		}
		if (bDefense)
		{
			return 1;
		}
		return bMidfield ? 2 : 3;
	}

	void FinalizeCardPresentation(FFMCodexLocalMatchCardView& View)
	{
		View.SkillSummaryLabel = View.SkillLabels.IsEmpty()
			? TEXT("NO SKILL")
			: FString::Join(View.SkillLabels, TEXT("  |  "));
		if (View.bAvailable)
		{
			View.StatusLabels.Add(TEXT("AVAILABLE"));
		}
		if (View.bUsed)
		{
			View.StatusLabels.Add(TEXT("USED"));
		}
		if (View.bDeployed)
		{
			View.StatusLabels.Add(TEXT("DEPLOYED"));
		}
		if (View.bGoalkeeperUsedThisMatch)
		{
			View.StatusLabels.Add(TEXT("GK USED"));
		}
		if (View.bGoalkeeperActivatedThisAttack)
		{
			View.StatusLabels.Add(TEXT("GK ACTIVE"));
		}
		if (View.StatusLabels.IsEmpty())
		{
			View.StatusLabels.Add(TEXT("UNAVAILABLE"));
		}
		View.StatusSummaryLabel = FString::Join(
			View.StatusLabels, TEXT("  |  "));
	}

	FFMCodexLocalMatchCardView::FSkill SkillPresentation(
		const FSkillRuleSnapshotSet& SkillRuleSet,
		const FName SkillId)
	{
		FFMCodexLocalMatchCardView::FSkill Result;
		FSkillRuleSnapshotQueryInput Input;
		Input.SkillId = SkillId;
		const auto Query = FSkillRuleSnapshotQuery::FindBySkillId(
			SkillRuleSet, Input);
		if (Query.bSuccess)
		{
			Result.SkillId = SkillId;
			Result.CanonicalLabel =
				FFMCodexLocalMatchInteractionViewBuilder::ToString(
					Query.Snapshot.SkillType);
			Result.MinTriggerActionPoint = Query.Snapshot.MinTriggerActionPoint;
			Result.MaxTriggerActionPoint = Query.Snapshot.MaxTriggerActionPoint;
		}
		return Result;
	}

	FFMCodexLocalMatchSlotView MakeSlotView(
		const FMatchPlayState& State,
		const FName SlotId,
		const EInitialTurnOrderPlayer EvaluatedSide)
	{
		FFMCodexLocalMatchSlotView View;
		View.SlotId = SlotId;
		const auto Slot = FMatchPlayDeploymentSlotCatalogQuery::FindSlot(
			State.DeploymentSlotCatalog, SlotId);
		if (Slot.bSuccess)
		{
			View.NeutralSide = Slot.SlotDefinition.NeutralSide;
		}
		const auto Zone = FMatchPlayRelativeDeploymentZoneResolver::Resolve(
			State.DeploymentSlotCatalog,
			SlotId,
			State.RuntimeState.CurrentAttackingPlayer,
			EvaluatedSide);
		if (Zone.bSuccess)
		{
			View.RelativeZone = Zone.RelativeZone;
		}
		View.Label = FString::Printf(
			TEXT("%s - Slot %s (%s)"),
			*FFMCodexLocalMatchInteractionViewBuilder::ToString(View.RelativeZone),
			*SlotId.ToString(),
			*FFMCodexLocalMatchInteractionViewBuilder::ToString(View.NeutralSide));
		return View;
	}

	FFMCodexLocalMatchCardView MakeCardView(
		const FMatchPlayState& State,
		const FSkillRuleSnapshotSet& SkillRuleSet,
		const EInitialTurnOrderPlayer Side,
		const FName CardId)
	{
		FFMCodexLocalMatchCardView View;
		View.Side = Side;
		View.CardId = CardId;
		View.DisplayLabel = CardId.IsNone()
			? TEXT("UNKNOWN CARD")
			: FString::Printf(TEXT("Card %s"), *CardId.ToString());
		const auto Card =
			FMatchPlayCardSnapshotAuthorityQuery::FindByPlayerSideAndCardId(
				State.CardSnapshotAuthority, Side, CardId);
		if (!Card.bSuccess)
		{
			View.CompactRoleLabel = TEXT("ROLE N/A");
			View.DeveloperReferenceLabel =
				TEXT("Card reference unavailable");
			FinalizeCardPresentation(View);
			return View;
		}

		View.bGoalkeeper = Card.Snapshot.bIsGoalkeeper;
		View.RackSortGroup = RackSortGroup(Card.Snapshot);
		if (const FFMCodexPrototypePlayerDefinition* Prototype =
			FFMCodexPrototypeTeamContent::Find(CardId))
		{
			View.DisplayLabel = Prototype->PreferredDisplayName.ToString();
			View.EnglishDisplayLabel =
				Prototype->EnglishDisplayName.ToString();
			View.NationalityLabel =
				Prototype->NationalityDisplayName.ToString();
			View.ClubLabel = Prototype->TeamDisplayName.ToString();
			View.BirthDate = Prototype->Card.BirthDate;
			View.HeightCm = Prototype->Card.HeightCm;
			View.WeightKg = Prototype->Card.WeightKg;
			View.PlayerFacingSerialLabel = Prototype->PlayerFacingSerial;
			const FFMCodexPlayerOverallResult Overall =
				Card.Snapshot.bHasGoalkeeperAttributes
					? FFMCodexPlayerOverall::CalculateGoalkeeper(
						Card.Snapshot.GoalkeeperAttributes,
						Card.Snapshot.Rarity)
					: FFMCodexPlayerOverall::CalculateOutfield(
						Card.Snapshot.Attributes,
						Card.Snapshot.Rarity);
			View.OverallRating = Overall.Value;
			View.bHasOverallRating = Overall.bSuccess;
		}
		TArray<FString> Positions;
		for (const EPlayerPositionType Position : Card.Snapshot.PositionTypes)
		{
			Positions.Add(
				FFMCodexLocalMatchInteractionViewBuilder::ToString(Position));
		}
		View.PositionLabel = JoinLabels(Positions);
		View.CompactRoleLabel = CompactRoleLabel(Card.Snapshot.PositionTypes);
		const FPlayerAttributes& A = Card.Snapshot.Attributes;
		View.AttributeValues = {
			{ TEXT("SHO"), A.Shooting },
			{ TEXT("DRI"), A.Dribbling },
			{ TEXT("PAS"), A.Passing },
			{ TEXT("OFF"), A.OffBall },
			{ TEXT("MRK"), A.Marking },
			{ TEXT("TKL"), A.Tackling },
			{ TEXT("SPD"), A.Speed },
			{ TEXT("STR"), A.Strength },
			{ TEXT("STA"), A.Stamina },
			{ TEXT("LS"), A.LongShot }
		};
		View.AttributeSummary = FString::Printf(
			TEXT("SHO %d | DRI %d | PAS %d | OFF %d | MRK %d | TKL %d | SPD %d | STR %d | STA %d | LS %d"),
			A.Shooting, A.Dribbling, A.Passing, A.OffBall,
			A.Marking, A.Tackling, A.Speed, A.Strength,
			A.Stamina, A.LongShot);
		View.CompactAttributeSummary = FString::Printf(
			TEXT("SHO %d | PAS %d | DRI %d | SPD %d"),
			A.Shooting, A.Passing, A.Dribbling, A.Speed);
		if (Card.Snapshot.bHasGoalkeeperAttributes)
		{
			const FGoalkeeperAttributes& G = Card.Snapshot.GoalkeeperAttributes;
			View.AttributeValues = {
				{ TEXT("HAN"), G.Handling },
				{ TEXT("POS"), G.Positioning },
				{ TEXT("REF"), G.Reflex },
				{ TEXT("AER"), G.Aerial },
				{ TEXT("ANT"), G.Anticipation },
				{ TEXT("1V1"), G.OneOnOne }
			};
			View.GoalkeeperAttributeSummary = FString::Printf(
				TEXT("HAN %d | POS %d | REF %d | AER %d | ANT %d | 1v1 %d"),
				G.Handling, G.Positioning, G.Reflex,
				G.Aerial, G.Anticipation, G.OneOnOne);
			View.CompactAttributeSummary = FString::Printf(
				TEXT("HAN %d | REF %d | AER %d | 1v1 %d"),
				G.Handling, G.Reflex, G.Aerial, G.OneOnOne);
		}
		for (const FName SkillId : Card.Snapshot.SkillIds)
		{
			const FFMCodexLocalMatchCardView::FSkill Skill =
				SkillPresentation(SkillRuleSet, SkillId);
			if (!Skill.CanonicalLabel.IsEmpty())
			{
				View.Skills.Add(Skill);
				View.SkillLabels.Add(Skill.CanonicalLabel);
			}
		}
		if (State.bHasCurrentAttack
			&& State.CurrentAttack.RouteKind
				== EMatchPlayCurrentAttackRouteKind::Ordinary)
		{
			View.EligibleTacticalSkills =
				FFMCodexLocalMatchInteractionViewBuilder::
					ProjectEligibleTacticalSkills(
						View.Skills,
						State.CurrentAttack.ActionPoint);
		}
		View.DeveloperReferenceLabel = FString::Printf(
			TEXT("Card reference: %s  |  Rarity: %s"),
			*View.CardId.ToString(), *RarityLabel(Card.Snapshot.Rarity));
		View.RarityLabel = RarityLabel(Card.Snapshot.Rarity);

		const FCardUsageState& Usage = CardUsage(State, Side);
		View.bAvailable = Usage.AvailableCardIds.Contains(CardId);
		View.bUsed = Usage.UsedCardIds.Contains(CardId);
		View.bEjected = Usage.EjectedCardIds.Contains(CardId);
		View.bGoalkeeperUsedThisMatch = View.bGoalkeeper
			&& (Side == EInitialTurnOrderPlayer::PlayerA
				? State.GoalkeeperUsageState.bPlayerAGoalkeeperCardUsed
				: State.GoalkeeperUsageState.bPlayerBGoalkeeperCardUsed);

		if (State.bHasCurrentAttack)
		{
			for (const FMatchPlayDeploymentPlacement& Placement
				: State.CurrentAttack.DeploymentPlacements)
			{
				if (Placement.PlayerSide == Side
					&& Placement.CardId == CardId)
				{
					View.bDeployed = true;
					View.SlotId = Placement.SlotId;
					const auto Slot = MakeSlotView(State, Placement.SlotId, Side);
					View.NeutralSide = Slot.NeutralSide;
					View.RelativeZone = Slot.RelativeZone;
					break;
				}
			}
			View.bGoalkeeperActivatedThisAttack = View.bGoalkeeper
				&& View.bDeployed
				&& State.CurrentAttack.bCurrentDefenseGoalkeeperActivated
				&& Side != State.RuntimeState.CurrentAttackingPlayer;
		}
		if (State.CurrentAttack.RouteKind
				== EMatchPlayCurrentAttackRouteKind::Ordinary
			&& View.bDeployed
			&& Side == State.RuntimeState.CurrentAttackingPlayer)
		{
			View.PitchMiniVisibleTacticalSkills =
				View.EligibleTacticalSkills;
		}
		if (State.bHasCurrentAttack
			&& State.CurrentAttack.RouteKind
				== EMatchPlayCurrentAttackRouteKind::Ordinary
			&& View.bAvailable
			&& !View.bDeployed
			&& Side == State.RuntimeState.CurrentAttackingPlayer)
		{
			View.HandMicroVisibleTacticalSkills =
				View.EligibleTacticalSkills;
		}
		View.HandMicroTacticalMatchCount =
			View.HandMicroVisibleTacticalSkills.Num();
		ensureAlwaysMsgf(View.HandMicroTacticalMatchCount <= 2,
			TEXT("Hand Micro tactical-match count invariant exceeded for %s: %d"),
			*View.CardId.ToString(), View.HandMicroTacticalMatchCount);
		View.bHasHandMicroTacticalMatch =
			View.HandMicroTacticalMatchCount > 0;
		View.PitchMiniTacticalMatchCount =
			View.PitchMiniVisibleTacticalSkills.Num();
		ensureAlwaysMsgf(View.PitchMiniTacticalMatchCount <= 2,
			TEXT("Pitch Mini tactical-match count invariant exceeded for %s: %d"),
			*View.CardId.ToString(), View.PitchMiniTacticalMatchCount);
		View.bHasPitchMiniTacticalMatch =
			View.PitchMiniTacticalMatchCount > 0;
		FinalizeCardPresentation(View);
		return View;
	}

	void BuildCardRosters(
		const FMatchPlayState& State,
		const FSkillRuleSnapshotSet& SkillRuleSet,
		FFMCodexLocalMatchInteractionView& View)
	{
		for (const EInitialTurnOrderPlayer Side : {
			EInitialTurnOrderPlayer::PlayerA,
			EInitialTurnOrderPlayer::PlayerB })
		{
			const FPlayerCardRuleSnapshotSet& SnapshotSet =
				Side == EInitialTurnOrderPlayer::PlayerA
					? State.CardSnapshotAuthority.PlayerACardSnapshots
					: State.CardSnapshotAuthority.PlayerBCardSnapshots;
			TArray<FFMCodexLocalMatchCardView>& Roster =
				Side == EInitialTurnOrderPlayer::PlayerA
					? View.PlayerACardRoster : View.PlayerBCardRoster;
			Roster.Reserve(SnapshotSet.Cards.Num());
			for (const FPlayerCardRuleSnapshot& Card : SnapshotSet.Cards)
			{
				Roster.Add(MakeCardView(
					State, SkillRuleSet, Side, Card.CardId));
			}
		}
	}

	void BuildRecoveryPresentation(
		const FMatchPlayState& State,
		FFMCodexLocalMatchInteractionView& View)
	{
		const FMatchPlayLastRecoveryFact& Fact = State.LastRecoveryFact;
		if (!Fact.bHasRecoveryFact)
		{
			return;
		}
		View.bHasRecoveryFact = true;
		View.RecoverySourceAttackSequence = Fact.SourceAttackSequence;
		if (Fact.SourceAttackSequence <= 0 || Fact.ReturnedCards.Num() > 2)
		{
			View.Diagnostic =
				TEXT("Latest Recovery fact is malformed and was not presented.");
			return;
		}

		for (const FMatchPlayRecoveredCardFactEntry& Entry :
			Fact.ReturnedCards)
		{
			const TArray<FFMCodexLocalMatchCardView>* Roster =
				Entry.OwnerSide == EInitialTurnOrderPlayer::PlayerA
					? &View.PlayerACardRoster
					: Entry.OwnerSide == EInitialTurnOrderPlayer::PlayerB
						? &View.PlayerBCardRoster : nullptr;
			const FFMCodexLocalMatchCardView* Card = Roster == nullptr
				? nullptr
				: Roster->FindByPredicate(
					[&Entry](const FFMCodexLocalMatchCardView& Candidate)
					{
						return Candidate.CardId == Entry.CardId;
					});

			FFMCodexLocalMatchRecoveryPresentationEntry Presented;
			Presented.OwnerSide = Entry.OwnerSide;
			Presented.CardId = Entry.CardId;
			Presented.OwnerDisplayName =
				FFMCodexPlayerUIPresentationText::RecoveryOwner(Entry.OwnerSide);
			Presented.PlayerDisplayName = Card != nullptr
				&& !Card->DisplayLabel.IsEmpty()
					? Card->DisplayLabel : TEXT("球员");
			Presented.PresentationLine =
				FFMCodexPlayerUIPresentationText::RecoveryEntry(
					Entry.OwnerSide, Presented.PlayerDisplayName).ToString();
			View.RecoveryPresentationEntries.Add(MoveTemp(Presented));
		}
	}

	void BuildPitchRegions(
		const FMatchPlayState& State,
		const FSkillRuleSnapshotSet& SkillRuleSet,
		FFMCodexLocalMatchInteractionView& View)
	{
		// Relative attack zones have no meaning after the final advance. In
		// particular, never send None through the debug "Unknown Zone" mapping.
		if (State.RuntimeState.CurrentAttackingPlayer == EInitialTurnOrderPlayer::None)
		{
			return;
		}
		for (const EMatchPlayNeutralSlotSide Side : {
			EMatchPlayNeutralSlotSide::NearPlayerB,
			EMatchPlayNeutralSlotSide::NearPlayerA })
		{
			FFMCodexLocalMatchPitchRegionView Region;
			Region.NeutralSide = Side;
			Region.Label =
				FFMCodexLocalMatchInteractionViewBuilder::ToString(Side);
			Region.bCurrentAttackingSide =
				(State.RuntimeState.CurrentAttackingPlayer
						== EInitialTurnOrderPlayer::PlayerA
					&& Side == EMatchPlayNeutralSlotSide::NearPlayerA)
				|| (State.RuntimeState.CurrentAttackingPlayer
						== EInitialTurnOrderPlayer::PlayerB
					&& Side == EMatchPlayNeutralSlotSide::NearPlayerB);
			for (const FMatchPlayDeploymentSlotDefinition& Slot
				: State.DeploymentSlotCatalog.Slots)
			{
				if (Slot.NeutralSide != Side)
				{
					continue;
				}

				FFMCodexLocalMatchPitchSlotView SlotView;
				SlotView.SlotId = Slot.SlotId;
				SlotView.Label = FString::Printf(
					TEXT("Slot %s"), *Slot.SlotId.ToString());
				SlotView.NeutralSide = Slot.NeutralSide;
				const auto PlayerAZone =
					FMatchPlayRelativeDeploymentZoneResolver::Resolve(
						State.DeploymentSlotCatalog,
						Slot.SlotId,
						State.RuntimeState.CurrentAttackingPlayer,
						EInitialTurnOrderPlayer::PlayerA);
				const auto PlayerBZone =
					FMatchPlayRelativeDeploymentZoneResolver::Resolve(
						State.DeploymentSlotCatalog,
						Slot.SlotId,
						State.RuntimeState.CurrentAttackingPlayer,
						EInitialTurnOrderPlayer::PlayerB);
				if (PlayerAZone.bSuccess)
				{
					SlotView.PlayerARelativeZone = PlayerAZone.RelativeZone;
					Region.PlayerARelativeZone = PlayerAZone.RelativeZone;
				}
				if (PlayerBZone.bSuccess)
				{
					SlotView.PlayerBRelativeZone = PlayerBZone.RelativeZone;
					Region.PlayerBRelativeZone = PlayerBZone.RelativeZone;
				}
				if (State.bHasCurrentAttack)
				{
					const FMatchPlayDeploymentPlacement* Placement =
						State.CurrentAttack.DeploymentPlacements.FindByPredicate(
							[&Slot](const FMatchPlayDeploymentPlacement& Candidate)
							{
								return Candidate.SlotId == Slot.SlotId;
							});
					if (Placement != nullptr)
					{
						SlotView.bOccupied = true;
						SlotView.Card = MakeCardView(
							State,
							SkillRuleSet,
							Placement->PlayerSide,
							Placement->CardId);
					}
				}
				Region.Slots.Add(MoveTemp(SlotView));
			}
			Region.ZoneContextLabel = FString::Printf(
				TEXT("Player A: %s  |  Player B: %s"),
				*FFMCodexLocalMatchInteractionViewBuilder::ToString(
					Region.PlayerARelativeZone),
				*FFMCodexLocalMatchInteractionViewBuilder::ToString(
					Region.PlayerBRelativeZone));
			const EMatchPlayRelativeDeploymentZone AttackerZone =
				State.RuntimeState.CurrentAttackingPlayer
					== EInitialTurnOrderPlayer::PlayerA
						? Region.PlayerARelativeZone
						: Region.PlayerBRelativeZone;
			if (AttackerZone == EMatchPlayRelativeDeploymentZone::Forward)
			{
				View.AttackDirectionLabel = FString::Printf(
					TEXT("%s attacks toward %s"),
					*FFMCodexLocalMatchInteractionViewBuilder::ToString(
						State.RuntimeState.CurrentAttackingPlayer),
					*Region.Label);
			}
			View.PitchRegions.Add(MoveTemp(Region));
		}
	}

	void AddSelectionOption(
		TArray<FFMCodexLocalMatchSelectionOption>& Options,
		const EInitialTurnOrderPlayer Side,
		const FName Id,
		const FName RelatedCardId,
		const FString& DisplayLabel = FString(),
		const ESkillRuleType SkillType = ESkillRuleType::None)
	{
		FFMCodexLocalMatchSelectionOption Option;
		Option.Side = Side;
		Option.Id = Id;
		Option.RelatedCardId = RelatedCardId;
		Option.SkillType = SkillType;
		Option.Label = DisplayLabel.IsEmpty()
			? Id.ToString()
			: DisplayLabel;
		Options.Add(MoveTemp(Option));
	}

	FString RollPurpose(
		const EMatchPlayCurrentAttackPostRouteRollPurpose Purpose)
	{
		switch (Purpose)
		{
		case EMatchPlayCurrentAttackPostRouteRollPurpose::PrimaryAttack:
			return TEXT("Primary Attack");
		case EMatchPlayCurrentAttackPostRouteRollPurpose::PrimaryDefense:
			return TEXT("Primary Defense");
		case EMatchPlayCurrentAttackPostRouteRollPurpose::PairedAttackA:
			return TEXT("Paired Attack A");
		case EMatchPlayCurrentAttackPostRouteRollPurpose::PairedAttackB:
			return TEXT("Paired Attack B");
		case EMatchPlayCurrentAttackPostRouteRollPurpose::BehindDefenseP2Defense:
			return TEXT("Legacy Behind Defense P2 (unreachable)");
		case EMatchPlayCurrentAttackPostRouteRollPurpose::OneOnOneChipShotAttack:
			return TEXT("One-on-One Chip Shot Attack");
		case EMatchPlayCurrentAttackPostRouteRollPurpose::OneOnOneDirectShotAttack:
			return TEXT("One-on-One Direct Shot Attack");
		case EMatchPlayCurrentAttackPostRouteRollPurpose::OneOnOneDirectShotDefense:
			return TEXT("One-on-One Direct Shot Defense");
		default:
			return TEXT("Unknown Post-Route Roll");
		}
	}

	EFMCodexLocalMatchRollGroup RollGroup(
		const EMatchPlayCurrentAttackPostRouteRollPurpose Purpose)
	{
		switch (Purpose)
		{
		case EMatchPlayCurrentAttackPostRouteRollPurpose::OneOnOneChipShotAttack:
		case EMatchPlayCurrentAttackPostRouteRollPurpose::OneOnOneDirectShotAttack:
		case EMatchPlayCurrentAttackPostRouteRollPurpose::OneOnOneDirectShotDefense:
			return EFMCodexLocalMatchRollGroup::OneOnOne;
		default:
			return EFMCodexLocalMatchRollGroup::PostRoute;
		}
	}

	bool RequiresOneOnOneChoice(
		const FMatchPlayState& State,
		const FSkillRuleSnapshotSet& SkillRuleSet)
	{
		const FMatchPlayCurrentAttackResolutionSession& Session =
			State.CurrentAttack.ResolutionSession;
		if (Session.ThroughBallOneOnOneShotChoice
			!= EMatchPlayThroughBallOneOnOneShotChoice::None
			|| !Session.bHasActualBranch
			|| Session.ActualBranch.ActionType != ESkillRuleType::ThroughBall)
		{
			return false;
		}

		if (Session.ActualBranch.ThroughBall
			== EMatchPlayThroughBallActualBranch::AntiOffside)
		{
			FMatchPlayCurrentAttackResolveThroughBallAntiOffsideDecisionRequest
				Request;
			Request.AttackSequence = State.CurrentAttack.AttackSequence;
			const auto Replay =
				FMatchPlayCurrentAttackResolveThroughBallAntiOffsideDecisionOrchestrator
					::Resolve(State, Request, &SkillRuleSet, nullptr);
			return Replay.bSuccess
				&& Replay.ProviderCallCount == 0
				&& Replay.OutcomeResult.Decision
					== EThroughBallAntiOffsideOutcomeDecision::OneOnOneRequired;
		}

		if (Session.ActualBranch.ThroughBall
			== EMatchPlayThroughBallActualBranch::BehindDefense)
		{
			const auto Replay =
				FMatchPlayCurrentAttackResolveThroughBallBehindDefenseP1FormulaOrchestrator
					::Resolve(State, &SkillRuleSet);
			return Replay.bSuccess
				&& Replay.PlanRegenerationProviderCallCount == 0
				&& Replay.FormulaExecutionResult.Decision
					== EThroughBallBehindDefenseP1FormulaResolutionExecutionDecision
						::OneOnOneRequired;
		}
		return false;
	}

	void BuildDeploymentOptions(
		const FMatchPlayState& State,
		const FSkillRuleSnapshotSet& SkillRuleSet,
		FFMCodexLocalMatchInteractionView& View)
	{
		const FMatchPlayCurrentAttackState& Attack = State.CurrentAttack;
		const EInitialTurnOrderPlayer Side = Attack.CurrentLegalDeploymentSide;
		for (const FName CardId : AvailableCards(State, Side))
		{
			const auto Availability =
				FMatchPlayOrdinaryDeploymentAvailability::Query(
					State, Attack.AttackSequence, Side, CardId);
			if (!Availability.bQuerySucceeded)
			{
				View.Diagnostic = Availability.ErrorMessage;
				continue;
			}
			if (Availability.LegalSlotIds.Num() > 0)
			{
				FFMCodexLocalMatchDeploymentGroup Group;
				Group.Side = Side;
				Group.CardId = CardId;
				Group.LegalSlotIds = Availability.LegalSlotIds;
				Group.Card = MakeCardView(
					State, SkillRuleSet, Side, CardId);
				for (const FName SlotId : Availability.LegalSlotIds)
				{
					Group.LegalSlots.Add(MakeSlotView(State, SlotId, Side));
				}
				View.DeploymentGroups.Add(MoveTemp(Group));
			}
			for (const FName SlotId : Availability.LegalSlotIds)
			{
				View.DeploymentOptions.Add({ Side, CardId, SlotId, false });
			}
		}

		const FName GoalkeeperId = GoalkeeperCardId(State, Side);
		const auto GoalkeeperAvailability =
			FMatchPlayGoalkeeperDeploymentAvailability::Query(
				State, Attack.AttackSequence, Side, GoalkeeperId);
		if (GoalkeeperAvailability.bQuerySucceeded)
		{
			if (GoalkeeperAvailability.LegalSlotIds.Num() > 0)
			{
				FFMCodexLocalMatchDeploymentGroup Group;
				Group.Side = Side;
				Group.CardId = GoalkeeperId;
				Group.bGoalkeeper = true;
				Group.LegalSlotIds = GoalkeeperAvailability.LegalSlotIds;
				Group.Card = MakeCardView(
					State, SkillRuleSet, Side, GoalkeeperId);
				for (const FName SlotId : GoalkeeperAvailability.LegalSlotIds)
				{
					Group.LegalSlots.Add(MakeSlotView(State, SlotId, Side));
				}
				View.DeploymentGroups.Add(MoveTemp(Group));
			}
			for (const FName SlotId : GoalkeeperAvailability.LegalSlotIds)
			{
				View.DeploymentOptions.Add({
					Side, GoalkeeperId, SlotId, true });
			}
		}
	}

	void BuildSelectionOptions(
		const FMatchPlayState& State,
		const FSkillRuleSnapshotSet& SkillRuleSet,
		FFMCodexLocalMatchInteractionView& View)
	{
		const FMatchPlayCurrentAttackState& Attack = State.CurrentAttack;
		const EInitialTurnOrderPlayer Attacker =
			State.RuntimeState.CurrentAttackingPlayer;
		const EInitialTurnOrderPlayer Defender = OtherSide(Attacker);
		const int64 Sequence = Attack.AttackSequence;

		switch (Attack.SelectionStage)
		{
		case EMatchPlayCurrentAttackSelectionStage::AwaitingCarrier:
		{
			View.InteractionCategory =
				EFMCodexLocalMatchInteractionCategory::SelectCarrier;
			View.ExpectedActingPlayer = Attacker;
			const auto Availability =
				FMatchPlayCurrentAttackCarrierSelectionAvailability::Query(
					State, Sequence, Attacker);
			for (const auto& Candidate : Availability.Candidates)
			{
				if (Candidate.LegalityResult.bIsLegal)
				{
					AddSelectionOption(
						View.SelectionOptions,
						Attacker,
						Candidate.CarrierCardId,
						Candidate.CarrierCardId);
				}
			}
			View.bCanResolveNoLegalChoice = Availability.bQuerySucceeded
				&& !Availability.bCanSelectAnyCarrier;
			break;
		}
		case EMatchPlayCurrentAttackSelectionStage::AwaitingMarker:
		{
			View.InteractionCategory =
				EFMCodexLocalMatchInteractionCategory::SelectMarker;
			View.ExpectedActingPlayer = Defender;
			View.bCanDecline = true;
			const auto Availability =
				FMatchPlayCurrentAttackMarkerSelectionAvailability::Query(
					State, Sequence, Defender);
			for (const auto& Candidate : Availability.Candidates)
			{
				if (Candidate.LegalityResult.bIsLegal)
				{
					AddSelectionOption(
						View.SelectionOptions,
						Defender,
						Candidate.MarkerCardId,
						Candidate.MarkerCardId);
				}
				else if (Candidate.LegalityResult.ErrorCode ==
					EMatchPlayCurrentAttackMarkerSelectionErrorCode::
						MarkerNotInCarrierPhysicalArea)
				{
					View.SelectionFeedbackCandidates.Add({
						Candidate.MarkerCardId,
						EFMCodexLocalMatchSelectionFeedbackReason::
							MarkerWrongPhysicalArea });
				}
			}
			View.bCanResolveNoLegalChoice = Availability.bQuerySucceeded
				&& !Availability.bCanSelectAnyMarker;
			break;
		}
		case EMatchPlayCurrentAttackSelectionStage::AwaitingSkill:
		{
			View.InteractionCategory =
				EFMCodexLocalMatchInteractionCategory::SelectSkill;
			View.ExpectedActingPlayer = Attacker;
			const auto Availability =
				FMatchPlayCurrentAttackSkillSelectionAvailability::Query(
					State, Sequence, Attacker, SkillRuleSet);
			View.bCanDecline = Availability.bQuerySucceeded
				&& Availability.bCanSelectAnySkill;
			for (const auto& Candidate : Availability.Candidates)
			{
				if (!Candidate.LegalityResult.bIsLegal)
				{
					if (Candidate.LegalityResult.ErrorCode
							== EMatchPlayCurrentAttackSkillSelectionErrorCode::
								PreparedRunnerIncompatibleWithSkill
						&& Candidate.LegalityResult.ResolvedActionType
							== ESkillRuleType::ThroughBall)
					{
						View.SelectionNotice =
							TEXT("RunnerNotInAttackingForwardArea");
					}
					continue;
				}
				FSkillRuleSnapshotQueryInput QueryInput;
				QueryInput.SkillId = Candidate.SkillId;
				const FSkillRuleSnapshotQueryResult Rule =
					FSkillRuleSnapshotQuery::FindBySkillId(
						SkillRuleSet, QueryInput);
				AddSelectionOption(
					View.SelectionOptions,
					Attacker,
					Candidate.SkillId,
					Attack.ActionPreparation.CarrierCardId,
					Rule.bSuccess
						? FFMCodexLocalMatchInteractionViewBuilder::ToString(
							Rule.Snapshot.SkillType)
						: TEXT("Unknown Skill"),
					Rule.bSuccess
						? Rule.Snapshot.SkillType : ESkillRuleType::None);
			}
			View.bCanResolveNoLegalChoice = Availability.bQuerySucceeded
				&& !Availability.bCanSelectAnySkill;
			break;
		}
		case EMatchPlayCurrentAttackSelectionStage::AwaitingRunner:
		{
			View.InteractionCategory =
				EFMCodexLocalMatchInteractionCategory::SelectRunner;
			View.ExpectedActingPlayer = Attacker;
			const auto Availability =
				FMatchPlayCurrentAttackRunnerSelectionAvailability::Query(
					State, Sequence, Attacker);
			View.bCanDecline = Availability.bQuerySucceeded
				&& Availability.bCanSelectAnyRunner;
			for (const auto& Candidate : Availability.Candidates)
			{
				if (Candidate.LegalityResult.bIsLegal)
				{
					AddSelectionOption(
						View.SelectionOptions,
						Attacker,
						Candidate.RunnerCardId,
						Candidate.RunnerCardId);
				}
				else
				{
					EFMCodexLocalMatchSelectionFeedbackReason FeedbackReason =
						EFMCodexLocalMatchSelectionFeedbackReason::None;
					switch (Candidate.LegalityResult.ErrorCode)
					{
					case EMatchPlayCurrentAttackRunnerSelectionErrorCode::RunnerIsGoalkeeper:
						FeedbackReason = EFMCodexLocalMatchSelectionFeedbackReason::
							RunnerIsGoalkeeper;
						break;
					case EMatchPlayCurrentAttackRunnerSelectionErrorCode::RunnerMatchesCarrier:
						FeedbackReason = EFMCodexLocalMatchSelectionFeedbackReason::
							RunnerMatchesCarrier;
						break;
					case EMatchPlayCurrentAttackRunnerSelectionErrorCode::
						RunnerMissingRequiredPositionType:
						FeedbackReason = EFMCodexLocalMatchSelectionFeedbackReason::
							RunnerMissingRequiredPositionType;
						break;
					case EMatchPlayCurrentAttackRunnerSelectionErrorCode::
						RunnerNotInAttackingForwardArea:
						FeedbackReason = EFMCodexLocalMatchSelectionFeedbackReason::
							RunnerNotInAttackingForwardArea;
						break;
					default:
						break;
					}
					if (FeedbackReason !=
						EFMCodexLocalMatchSelectionFeedbackReason::None)
					{
						View.SelectionFeedbackCandidates.Add({
							Candidate.RunnerCardId, FeedbackReason });
					}
				}
			}
			View.bCanResolveNoLegalChoice = Availability.bQuerySucceeded
				&& !Availability.bCanSelectAnyRunner;
			break;
		}
		case EMatchPlayCurrentAttackSelectionStage::AwaitingHelper:
		{
			View.InteractionCategory =
				EFMCodexLocalMatchInteractionCategory::SelectHelper;
			View.ExpectedActingPlayer = Defender;
			const auto Availability =
				FMatchPlayCurrentAttackHelperSelectionAvailability::Query(
					State, Sequence, Defender);
			// Decline is a voluntary choice and therefore exists only when the
			// defender actually has a legal Helper. Formal absence uses the
			// distinct ResolveNoLegalHelper authority command.
			View.bCanDecline = Availability.bQuerySucceeded
				&& Availability.bCanSelectAnyHelper;
			for (const auto& Candidate : Availability.Candidates)
			{
				if (Candidate.LegalityResult.bSuccess)
				{
					AddSelectionOption(
						View.SelectionOptions,
						Defender,
						Candidate.HelperCardId,
						Candidate.HelperCardId);
				}
				else if (Candidate.LegalityResult.ErrorCode ==
					EMatchPlayCurrentAttackHelperSelectionErrorCode::
						HelperIsGoalkeeper)
				{
					View.SelectionFeedbackCandidates.Add({
						Candidate.HelperCardId,
						EFMCodexLocalMatchSelectionFeedbackReason::
							HelperIsGoalkeeper });
				}
				else if (Candidate.LegalityResult.ErrorCode ==
					EMatchPlayCurrentAttackHelperSelectionErrorCode::
						HelperMatchesMarker)
				{
					View.SelectionFeedbackCandidates.Add({
						Candidate.HelperCardId,
						EFMCodexLocalMatchSelectionFeedbackReason::
							HelperMatchesMarker });
				}
				else if (Candidate.LegalityResult.ErrorCode ==
					EMatchPlayCurrentAttackHelperSelectionErrorCode::
						HelperNotInRunnerPhysicalArea)
				{
					View.SelectionFeedbackCandidates.Add({
						Candidate.HelperCardId,
						EFMCodexLocalMatchSelectionFeedbackReason::
							HelperWrongPhysicalArea });
				}
			}
			View.bCanResolveNoLegalChoice = Availability.bQuerySucceeded
				&& !Availability.bCanSelectAnyHelper;
			break;
		}
		case EMatchPlayCurrentAttackSelectionStage::AwaitingBranchIntent:
			View.InteractionCategory =
				Attack.ActionPreparation.ActionType == ESkillRuleType::LongShot
					? EFMCodexLocalMatchInteractionCategory
						::SelectLongShotBranch
					: EFMCodexLocalMatchInteractionCategory
						::SelectBranchIntent;
			View.ExpectedActingPlayer = Attacker;
			if (Attack.ActionPreparation.ActionType == ESkillRuleType::Cross)
			{
				View.BranchIntentOptions = {
					EMatchPlayElectiveBranchIntent::CrossHigh,
					EMatchPlayElectiveBranchIntent::CrossLow
				};
			}
			else if (Attack.ActionPreparation.ActionType
					== ESkillRuleType::LongShot
				|| Attack.ActionPreparation.ActionType
					== ESkillRuleType::CutInsideShot)
			{
				View.BranchIntentOptions = {
					EMatchPlayElectiveBranchIntent::DirectShot,
					EMatchPlayElectiveBranchIntent::DeadCorner
				};
			}
			break;
		default:
			const ESkillRuleType PreparedActionType =
				Attack.bHasSelectedAction
					? Attack.SelectedAction.ActionType
					: Attack.ActionPreparation.ActionType;
			if (Attack.SelectionStage
					== EMatchPlayCurrentAttackSelectionStage::ReadyForResolution
				&& (PreparedActionType == ESkillRuleType::Cross
					|| PreparedActionType == ESkillRuleType::ThroughBall
					|| PreparedActionType == ESkillRuleType::PassControl))
			{
				View.InteractionCategory = PreparedActionType == ESkillRuleType::Cross
					? EFMCodexLocalMatchInteractionCategory::RollCrossRoute
					: PreparedActionType == ESkillRuleType::ThroughBall
						? EFMCodexLocalMatchInteractionCategory
							::RollThroughBallInitialRoute
						: EFMCodexLocalMatchInteractionCategory
							::RollPassControlRoute;
				View.ExpectedActingPlayer = Attacker;
				View.bHumanInteraction = true;
				View.ContinueActionLabel = PreparedActionType == ESkillRuleType::Cross
					? TEXT("判定传中路线")
					: PreparedActionType == ESkillRuleType::ThroughBall
						? TEXT("判定直塞路线")
						: TEXT("判定控球推进路线");
			}
			else
			{
				View.InteractionCategory =
					EFMCodexLocalMatchInteractionCategory::ContinueResolution;
			}
			break;
		}
	}

	void AttachSelectionCardViews(
		const FMatchPlayState& State,
		const FSkillRuleSnapshotSet& SkillRuleSet,
		FFMCodexLocalMatchInteractionView& View)
	{
		for (FFMCodexLocalMatchSelectionOption& Option
			: View.SelectionOptions)
		{
			if (Option.RelatedCardId.IsNone())
			{
				continue;
			}
			Option.Card = MakeCardView(
				State,
				SkillRuleSet,
				Option.Side,
				Option.RelatedCardId);
			Option.bHasCard = !Option.Card.CardId.IsNone();
		}
	}

	void BuildSetPieceLegalCards(
		const FMatchPlayState& State,
		const EInitialTurnOrderPlayer Side,
		const EMatchPlaySetPieceParticipantRole Role,
		FFMCodexLocalMatchInteractionView& View)
	{
		for (const FName CardId : AvailableCards(State, Side))
		{
			FMatchPlaySetPieceParticipantEligibilityRequest Request;
			Request.ExpectedOwnerSide = Side;
			Request.CardId = CardId;
			Request.Role = Role;
			if (FMatchPlaySetPieceParticipantEligibility::Evaluate(State, Request)
				.bIsEligible)
			{
				View.LegalSetPieceCardIds.Add(CardId);
			}
		}
	}

	void CopyCarrierRoute(
		const FMatchPlaySetPieceParticipantBinding& Carrier,
		const EMatchPlaySetPieceCarrierRouteStage Stage,
		const bool bHasAttackD6,
		const int32 AttackD6,
		const bool bHasDefenseD6,
		const int32 DefenseD6,
		const bool bHasPair,
		const int32 PairA,
		const int32 PairB,
		const bool bHasFormula,
		const FFormulaResolutionResult& Formula,
		const FName GoalScorer,
		FFMCodexLocalMatchInteractionView& View)
	{
		View.SetPieceCarrier = Carrier;
		View.SetPieceCarrierStage = Stage;
		View.bHasSetPieceAttackD6 = bHasAttackD6;
		View.SetPieceAttackD6 = AttackD6;
		View.bHasSetPieceDefenseD6 = bHasDefenseD6;
		View.SetPieceDefenseD6 = DefenseD6;
		View.bHasSetPiecePairedD6 = bHasPair;
		View.SetPiecePairedD6A = PairA;
		View.SetPiecePairedD6B = PairB;
		View.SetPiecePairedD6Total = bHasPair ? PairA + PairB : 0;
		View.bHasSetPieceFormula = bHasFormula;
		View.SetPieceFormula = Formula;
		View.SetPieceGoalScorerCardId = GoalScorer;
	}

	void BuildSetPieceView(
		const FMatchPlayState& Snapshot,
		FFMCodexLocalMatchInteractionView& Result)
	{
		const FMatchPlayCurrentAttackState& Attack = Snapshot.CurrentAttack;
		const FMatchPlaySetPieceRouteState& Route = Attack.SetPieceRoute;
		const EInitialTurnOrderPlayer Attacker =
			Snapshot.RuntimeState.CurrentAttackingPlayer;
		const EInitialTurnOrderPlayer Defender = OtherSide(Attacker);
		Result.MajorPhase = EFMCodexLocalMatchMajorPhase::Resolution;
		Result.SetPieceStage = Route.Stage;
		Result.SetPieceType = Route.SelectedType;
		Result.bHasSetPieceTypeRoll = Route.bHasTypeRoll;
		Result.RawSetPieceTypeD6 = Route.RawTypeD6;
		Result.ActionLabel = TEXT("定位球");

		auto ProjectDirectCurrentTotals = [&Snapshot, Defender, &Result](
			const float AttackBase,
			const TFunction<float(const FPlayerCardRuleSnapshot&)>& DefenseBase,
			const float DefenseModifier)
		{
			if (!Result.SetPieceCarrier.bIsBound)
			{
				return;
			}
			const FMatchPlayDefendingGoalkeeperQueryResult Goalkeeper =
				FMatchPlayDefendingGoalkeeperQuery::Query(Snapshot, Defender);
			if (!Goalkeeper.bSuccess)
			{
				return;
			}
			Result.bHasSetPieceAttackKnownSubtotal = true;
			Result.SetPieceAttackKnownSubtotal = AttackBase;
			Result.bHasSetPieceAttackCurrentTotal = true;
			Result.SetPieceAttackCurrentTotal = AttackBase
				+ (Result.bHasSetPieceAttackD6
					? Result.SetPieceAttackD6 : 0);
			Result.bHasSetPieceDefenseKnownSubtotal = true;
			Result.SetPieceDefenseKnownSubtotal =
				DefenseBase(Goalkeeper.Snapshot) + DefenseModifier;
			Result.bHasSetPieceDefenseCurrentTotal = true;
			Result.SetPieceDefenseCurrentTotal =
				Result.SetPieceDefenseKnownSubtotal
				+ (Result.bHasSetPieceDefenseD6
					? Result.SetPieceDefenseD6 : 0);
		};

		if (Route.Stage == EMatchPlaySetPieceRouteStage::AwaitingTypeRoll)
		{
			Result.InteractionCategory =
				EFMCodexLocalMatchInteractionCategory::RollSetPieceType;
			Result.ExpectedActingPlayer = Attacker;
			Result.bHumanInteraction = true;
			Result.ContinueActionLabel = TEXT("掷定位球类型");
			return;
		}

		auto ConfigureCarrierStage = [&](const EMatchPlaySetPieceCarrierRouteStage Stage)
		{
			switch (Stage)
			{
			case EMatchPlaySetPieceCarrierRouteStage::AwaitingCarrier:
				Result.InteractionCategory =
					EFMCodexLocalMatchInteractionCategory::SelectSetPieceCarrier;
				Result.ExpectedActingPlayer = Attacker;
				Result.bHumanInteraction = true;
				BuildSetPieceLegalCards(Snapshot, Attacker,
					EMatchPlaySetPieceParticipantRole::Carrier, Result);
				Result.ContinueActionLabel = TEXT("确认主罚球员");
				break;
			case EMatchPlaySetPieceCarrierRouteStage::AwaitingMethod:
				Result.InteractionCategory =
					EFMCodexLocalMatchInteractionCategory::SelectSetPieceMethod;
				Result.ExpectedActingPlayer = Attacker;
				Result.bHumanInteraction = true;
				break;
			case EMatchPlaySetPieceCarrierRouteStage::DirectAwaitingAttackRoll:
				Result.ExpectedActingPlayer = Attacker;
				Result.bHumanInteraction = true;
				Result.ContinueActionLabel = TEXT("进攻方掷点");
				break;
			case EMatchPlaySetPieceCarrierRouteStage::DirectAwaitingDefenseRoll:
				Result.ExpectedActingPlayer = Defender;
				Result.bHumanInteraction = true;
				Result.ContinueActionLabel = TEXT("防守方掷点");
				break;
			case EMatchPlaySetPieceCarrierRouteStage::AngledAwaitingRoll:
			case EMatchPlaySetPieceCarrierRouteStage::PowerAwaitingRoll:
			case EMatchPlaySetPieceCarrierRouteStage::PanenkaAwaitingRoll:
				Result.ExpectedActingPlayer = Attacker;
				Result.bHumanInteraction = true;
				break;
			default:
				break;
			}
		};

		switch (Route.SelectedType)
		{
		case ESetPieceSelectedType::ShortFreeKick:
		{
			const auto& Short = Route.ShortFreeKick;
			Result.ActionLabel = TEXT("近距离任意球");
			CopyCarrierRoute(Short.Carrier, Short.Stage,
				Short.bHasAttackD6, Short.AttackD6,
				Short.bHasDefenseD6, Short.DefenseD6,
				Short.bHasAngledD6Pair, Short.AngledD6A, Short.AngledD6B,
				Short.bHasFormulaResolution, Short.FormulaResolution,
				Short.GoalScorerCardId, Result);
			Result.bShortAngledEligible = Short.Carrier.bIsBound
				&& Short.Carrier.Snapshot.Attributes.Shooting
					+ Short.Carrier.Snapshot.Attributes.Passing >= 8;
			Result.bHasSetPieceOutcome = Short.GameplayOutcome
				!= EMatchPlayShortFreeKickGameplayOutcome::None;
			Result.bSetPieceGoal = Short.GameplayOutcome
				== EMatchPlayShortFreeKickGameplayOutcome::Goal;
			Result.bSetPieceNoLegalCarrier = Short.bNoLegalCarrier;
			if (Short.Method == EMatchPlayShortFreeKickMethod::Direct)
			{
				ProjectDirectCurrentTotals(
					FMath::Max(Short.Carrier.Snapshot.Attributes.Shooting,
						Short.Carrier.Snapshot.Attributes.Passing),
					[](const FPlayerCardRuleSnapshot& Goalkeeper)
					{
						return Goalkeeper.GoalkeeperAttributes.Handling;
					},
					1.0f);
			}
			ConfigureCarrierStage(Short.Stage);
			if (Short.Stage == EMatchPlaySetPieceCarrierRouteStage::DirectAwaitingAttackRoll)
				Result.InteractionCategory = EFMCodexLocalMatchInteractionCategory::RollShortFreeKickDirectAttack;
			else if (Short.Stage == EMatchPlaySetPieceCarrierRouteStage::DirectAwaitingDefenseRoll)
				Result.InteractionCategory = EFMCodexLocalMatchInteractionCategory::RollShortFreeKickDirectDefense;
			else if (Short.Stage == EMatchPlaySetPieceCarrierRouteStage::AngledAwaitingRoll)
			{
				Result.InteractionCategory = EFMCodexLocalMatchInteractionCategory::RollShortFreeKickAngled;
				Result.ContinueActionLabel = TEXT("掷战术配合双骰");
			}
			break;
		}
		case ESetPieceSelectedType::LongFreeKick:
		{
			const auto& Long = Route.LongFreeKick;
			Result.ActionLabel = FFMCodexPlayerUIPresentationText
				::SetPieceName(ESetPieceSelectedType::LongFreeKick).ToString();
			CopyCarrierRoute(Long.Carrier, Long.Stage,
				Long.bHasAttackD6, Long.AttackD6,
				Long.bHasDefenseD6, Long.DefenseD6,
				Long.bHasPowerD6Pair, Long.PowerD6A, Long.PowerD6B,
				Long.bHasFormulaResolution, Long.FormulaResolution,
				Long.GoalScorerCardId, Result);
			Result.bHasSetPieceOutcome = Long.GameplayOutcome
				!= EMatchPlayLongFreeKickGameplayOutcome::None;
			Result.bSetPieceGoal = Long.GameplayOutcome
				== EMatchPlayLongFreeKickGameplayOutcome::Goal;
			Result.bSetPieceNoLegalCarrier = Long.bNoLegalCarrier;
			if (Long.Method == EMatchPlayLongFreeKickMethod::Direct)
			{
				ProjectDirectCurrentTotals(
					Long.Carrier.Snapshot.Attributes.LongShot,
					[](const FPlayerCardRuleSnapshot& Goalkeeper)
					{
						return Goalkeeper.GoalkeeperAttributes.Positioning;
					},
					2.0f);
			}
			ConfigureCarrierStage(Long.Stage);
			if (Long.Stage == EMatchPlaySetPieceCarrierRouteStage::DirectAwaitingAttackRoll)
				Result.InteractionCategory = EFMCodexLocalMatchInteractionCategory::RollLongFreeKickDirectAttack;
			else if (Long.Stage == EMatchPlaySetPieceCarrierRouteStage::DirectAwaitingDefenseRoll)
				Result.InteractionCategory = EFMCodexLocalMatchInteractionCategory::RollLongFreeKickDirectDefense;
			else if (Long.Stage == EMatchPlaySetPieceCarrierRouteStage::PowerAwaitingRoll)
			{
				Result.InteractionCategory = EFMCodexLocalMatchInteractionCategory::RollLongFreeKickPower;
				Result.ContinueActionLabel = FText::Format(NSLOCTEXT("FMCodexSetPiece", "LongPowerRoll", "掷{0}双骰"),
					FFMCodexPlayerUIPresentationText::LongFreeKickPowerStage()).ToString();
			}
			break;
		}
		case ESetPieceSelectedType::Penalty:
		{
			const auto& Penalty = Route.Penalty;
			Result.ActionLabel = TEXT("点球");
			CopyCarrierRoute(Penalty.Carrier, Penalty.Stage,
				Penalty.bHasAttackD6, Penalty.AttackD6,
				Penalty.bHasDefenseD6, Penalty.DefenseD6,
				Penalty.bHasPanenkaD6, Penalty.PanenkaD6, 0,
				Penalty.bHasFormulaResolution, Penalty.FormulaResolution,
				Penalty.GoalScorerCardId, Result);
			Result.bHasSetPieceOutcome = Penalty.GameplayOutcome
				!= EMatchPlayPenaltyGameplayOutcome::None;
			Result.bSetPieceGoal = Penalty.GameplayOutcome
				== EMatchPlayPenaltyGameplayOutcome::Goal;
			Result.bSetPieceNoLegalCarrier = Penalty.bNoLegalCarrier;
			if (Penalty.Method == EMatchPlayPenaltyMethod::Direct)
			{
				ProjectDirectCurrentTotals(
					FMath::Max(Penalty.Carrier.Snapshot.Attributes.Shooting,
						Penalty.Carrier.Snapshot.Attributes.Passing),
					[](const FPlayerCardRuleSnapshot& Goalkeeper)
					{
						return Goalkeeper.GoalkeeperAttributes.Anticipation;
					},
					-3.0f);
			}
			ConfigureCarrierStage(Penalty.Stage);
			if (Penalty.Stage == EMatchPlaySetPieceCarrierRouteStage::DirectAwaitingAttackRoll)
				Result.InteractionCategory = EFMCodexLocalMatchInteractionCategory::RollPenaltyDirectAttack;
			else if (Penalty.Stage == EMatchPlaySetPieceCarrierRouteStage::DirectAwaitingDefenseRoll)
				Result.InteractionCategory = EFMCodexLocalMatchInteractionCategory::RollPenaltyDirectDefense;
			else if (Penalty.Stage == EMatchPlaySetPieceCarrierRouteStage::PanenkaAwaitingRoll)
			{
				Result.InteractionCategory = EFMCodexLocalMatchInteractionCategory::RollPenaltyPanenka;
				Result.ContinueActionLabel = TEXT("掷勺子点球");
			}
			break;
		}
		case ESetPieceSelectedType::Corner:
		{
			const FMatchPlayCornerRouteState& Corner = Route.Corner;
			Result.ActionLabel = TEXT("角球");
			Result.CornerStage = Corner.Stage;
			Result.bCornerAttackerNominationsLocked = Corner.bAttackerNominationsLocked;
			Result.bCornerDefenderNominationsLocked = Corner.bDefenderNominationsLocked;
			Result.CornerAttackerNominees = Corner.AttackerNominees;
			Result.CornerDefenderNominees = Corner.DefenderNominees;
			Result.bHideCornerAttackerNomineeDetails = false;
			Result.bHasCornerSharedParticipantD6 = Corner.bHasSharedParticipantD6;
			Result.CornerSharedParticipantD6 = Corner.SharedParticipantD6;
			Result.CornerRunner = Corner.Runner;
			Result.CornerHelper = Corner.Helper;
			Result.CornerCandidateBonusSide = Corner.CandidateBonusSide;
			Result.CornerCandidateBonus = Corner.CandidateBonus;
			Result.CornerIntendedRoute = Corner.IntendedRoute;
			Result.bHasCornerRouteD6 = Corner.bHasRouteD6;
			Result.CornerRouteD6 = Corner.RawRouteD6;
			Result.CornerActualRoute = Corner.ActualRoute;
			Result.bHasSetPieceAttackD6 = Corner.bHasAttackD6;
			Result.SetPieceAttackD6 = Corner.AttackD6;
			Result.bHasSetPieceDefenseD6 = Corner.bHasDefenseD6;
			Result.SetPieceDefenseD6 = Corner.DefenseD6;
			Result.bHasSetPieceFormula = Corner.bHasFormulaResolution;
			Result.SetPieceFormula = Corner.FormulaResolution;
			Result.bHasSetPieceOutcome = Corner.GameplayOutcome
				!= EMatchPlayCornerGameplayOutcome::None;
			Result.bSetPieceGoal = Corner.GameplayOutcome
				== EMatchPlayCornerGameplayOutcome::Goal;
			Result.bSetPieceSystemGoal = Corner.GameplayOutcome
				== EMatchPlayCornerGameplayOutcome::SystemGoal;
			Result.SetPieceGoalScorerCardId = Corner.GoalScorerCardId;
			const auto Preview = FMatchPlayCornerResolution::QueryFormulaPreview(Snapshot);
			if (Preview.bAvailable)
			{
				Result.bHasSetPieceAttackKnownSubtotal = true;
				Result.bHasSetPieceDefenseKnownSubtotal = true;
				Result.bHasSetPieceAttackCurrentTotal = true;
				Result.bHasSetPieceDefenseCurrentTotal = true;
				Result.SetPieceAttackKnownSubtotal = Preview.AttackKnownSubtotal;
				Result.SetPieceDefenseKnownSubtotal = Preview.DefenseKnownSubtotal;
				Result.SetPieceAttackCurrentTotal = Preview.AttackCurrentTotal;
				Result.SetPieceDefenseCurrentTotal = Preview.DefenseCurrentTotal;
			}
			auto RollLabels = [](const int32 Count)
			{
				TArray<FString> Labels;
				for (int32 Index = 0; Index < Count; ++Index)
				{
					int32 First = 0, Last = 0;
					for (int32 D6 = 1; D6 <= 6; ++D6)
						if (FMatchPlayCornerResolution::MapParticipantIndex(Count, D6) == Index)
						{
							if (First == 0) First = D6;
							Last = D6;
						}
					Labels.Add(FString::Printf(TEXT("%d–%d"), First, Last));
				}
				return Labels;
			};
			Result.CornerAttackerNomineeRollLabels = RollLabels(Result.CornerAttackerNominees.Num());
			Result.CornerDefenderNomineeRollLabels = RollLabels(Result.CornerDefenderNominees.Num());
			switch (Corner.Stage)
			{
			case EMatchPlaySetPieceCornerRouteStage::AwaitingAttackerNominations:
				Result.InteractionCategory = EFMCodexLocalMatchInteractionCategory::DraftCornerAttacker;
				Result.ExpectedActingPlayer = Attacker;
				BuildSetPieceLegalCards(Snapshot, Attacker, EMatchPlaySetPieceParticipantRole::CornerRunner, Result);
				Result.ContinueActionLabel = FFMCodexPlayerUIPresentationText::CornerLockCandidates(true).ToString(); break;
			case EMatchPlaySetPieceCornerRouteStage::AwaitingDefenderNominations:
				Result.InteractionCategory = EFMCodexLocalMatchInteractionCategory::DraftCornerDefender;
				Result.ExpectedActingPlayer = Defender;
				BuildSetPieceLegalCards(Snapshot, Defender, EMatchPlaySetPieceParticipantRole::CornerHelper, Result);
				Result.ContinueActionLabel = FFMCodexPlayerUIPresentationText::CornerLockCandidates(false).ToString(); break;
			case EMatchPlaySetPieceCornerRouteStage::AwaitingParticipantSelectionRoll:
				Result.InteractionCategory = EFMCodexLocalMatchInteractionCategory::RollCornerParticipantSelection;
				Result.ExpectedActingPlayer = Attacker;
				Result.ContinueActionLabel = TEXT("掷对位球员"); break;
			case EMatchPlaySetPieceCornerRouteStage::AwaitingIntent:
				Result.InteractionCategory = EFMCodexLocalMatchInteractionCategory::SelectCornerIntent;
				Result.ExpectedActingPlayer = Attacker; break;
			case EMatchPlaySetPieceCornerRouteStage::AwaitingRouteRoll:
				Result.InteractionCategory = EFMCodexLocalMatchInteractionCategory::RollCornerRoute;
				Result.ExpectedActingPlayer = Attacker;
				Result.ContinueActionLabel = TEXT("掷角球路线"); break;
			case EMatchPlaySetPieceCornerRouteStage::AwaitingAttackRoll:
				Result.InteractionCategory = EFMCodexLocalMatchInteractionCategory::RollCornerAttack;
				Result.ExpectedActingPlayer = Attacker;
				Result.ContinueActionLabel = TEXT("进攻方掷点"); break;
			case EMatchPlaySetPieceCornerRouteStage::AwaitingDefenseRoll:
				Result.InteractionCategory = EFMCodexLocalMatchInteractionCategory::RollCornerDefense;
				Result.ExpectedActingPlayer = Defender;
				Result.ContinueActionLabel = TEXT("防守方掷点"); break;
			default: break;
			}
			Result.bHumanInteraction = Result.ExpectedActingPlayer
				!= EInitialTurnOrderPlayer::None;
			break;
		}
		default:
			break;
		}
		if (Attack.LifecycleState
			== EMatchPlayCurrentAttackLifecycleState::TerminalPendingAdvance)
		{
			Result.InteractionCategory =
				EFMCodexLocalMatchInteractionCategory::AdvanceAfterTerminal;
			Result.ExpectedActingPlayer = Attacker;
			Result.bHumanInteraction = true;
			Result.bTerminalPendingAdvance = true;
			Result.ContinueActionLabel = TEXT("下一回合");
		}
	}

	bool IsPlayerSide(const EInitialTurnOrderPlayer Side)
	{
		return Side == EInitialTurnOrderPlayer::PlayerA
			|| Side == EInitialTurnOrderPlayer::PlayerB;
	}

	void ClearSetPieceTypeDependentProjection(
		FFMCodexLocalMatchInteractionView& View)
	{
		View.SetPieceStage = EMatchPlaySetPieceRouteStage::None;
		View.SetPieceType = ESetPieceSelectedType::None;
		View.ActionLabel = TEXT("定位球");
		View.bShortAngledEligible = false;
		View.SetPieceCarrierStage = EMatchPlaySetPieceCarrierRouteStage::None;
		View.CornerStage = EMatchPlaySetPieceCornerRouteStage::None;
		View.LegalSetPieceCardIds.Reset();
		View.DraftSetPieceCarrierCardId = NAME_None;
		View.DraftCornerNomineeCardIds.Reset();
		View.bCornerLockConfirmationPending = false;
		View.SetPieceCarrier = {};
		View.CornerAttackerNominees.Reset();
		View.CornerDefenderNominees.Reset();
		View.bCornerAttackerNominationsLocked = false;
		View.bCornerDefenderNominationsLocked = false;
		View.bHideCornerAttackerNomineeDetails = false;
		View.CornerAttackerNomineeRollLabels.Reset();
		View.CornerDefenderNomineeRollLabels.Reset();
		View.CornerRunner = {};
		View.CornerHelper = {};
		View.CornerCandidateBonusSide = EInitialTurnOrderPlayer::None;
		View.CornerCandidateBonus = 0;
		View.CornerIntendedRoute = EMatchPlayCornerRouteIntent::None;
		View.CornerActualRoute = EMatchPlayCornerRouteIntent::None;
		View.bHasCornerSharedParticipantD6 = false;
		View.CornerSharedParticipantD6 = 0;
		View.bHasCornerRouteD6 = false;
		View.CornerRouteD6 = 0;
		View.bHasSetPieceAttackD6 = false;
		View.SetPieceAttackD6 = 0;
		View.bHasSetPieceDefenseD6 = false;
		View.SetPieceDefenseD6 = 0;
		View.bHasSetPiecePairedD6 = false;
		View.SetPiecePairedD6A = 0;
		View.SetPiecePairedD6B = 0;
		View.SetPiecePairedD6Total = 0;
		View.bHasSetPieceFormula = false;
		View.SetPieceFormula = {};
		View.bHasSetPieceOutcome = false;
		View.bSetPieceGoal = false;
		View.bSetPieceSystemGoal = false;
		View.bSetPieceNoLegalCarrier = false;
		View.SetPieceGoalScorerCardId = NAME_None;
		View.InteractionCategory = EFMCodexLocalMatchInteractionCategory::None;
		View.ExpectedActingPlayer = EInitialTurnOrderPlayer::None;
		View.bHumanInteraction = false;
	}

	void ClearUndisclosedAttackRouteProjection(
		FFMCodexLocalMatchInteractionView& View)
	{
		ClearSetPieceTypeDependentProjection(View);
		View.bCanFinishDeployment = false;
		View.bPlayerADeploymentFinished = false;
		View.bPlayerBDeploymentFinished = false;
		View.bDeploymentComplete = false;
		View.RouteKind = EMatchPlayCurrentAttackRouteKind::None;
		View.MajorPhase = EFMCodexLocalMatchMajorPhase::Selection;
		View.SelectionStage = EMatchPlayCurrentAttackSelectionStage::None;
		View.CurrentLegalDeploymentSide = EInitialTurnOrderPlayer::None;
		View.InteractionCategory = EFMCodexLocalMatchInteractionCategory::None;
		View.ExpectedActingPlayer = EInitialTurnOrderPlayer::None;
		View.bHumanInteraction = false;
		View.bCanDecline = false;
		View.bCanResolveNoLegalChoice = false;
		View.bTacticalPointRollReady = false;
		View.bHasTacticalPlayerCounts = false;
		View.PlayerATacticalPlayerCount = 0;
		View.PlayerBTacticalPlayerCount = 0;
		View.PresentedActionType = ESkillRuleType::None;
		View.ActionLabel.Reset();
		View.ElectiveBranchIntent = EMatchPlayElectiveBranchIntent::None;
		View.ActualBranchLabel.Reset();
		View.OneOnOneChoiceLabel.Reset();
		View.DeploymentPlacements.Reset();
		View.DeploymentOptions.Reset();
		View.DeploymentGroups.Reset();
		View.SelectionOptions.Reset();
		View.SelectionFeedbackCandidates.Reset();
		View.SelectedCarrierCardId = NAME_None;
		View.SelectedRunnerCardId = NAME_None;
		View.SelectedMarkerCardId = NAME_None;
		View.SelectedHelperCardId = NAME_None;
		View.SelectedSkillId = NAME_None;
		View.PitchRegions.Reset();
		View.BranchIntentOptions.Reset();
		View.OneOnOneOptions.Reset();
		View.AcceptedRolls.Reset();
		View.ResolutionFacts = {};
		View.bCrossAttackRollPending = false;
		View.bCrossDefenseRollPending = false;
		View.bCrossFormulaComplete = false;
		View.bCrossTerminalActionAvailable = false;
		View.bLongShotDirectAttackRollPending = false;
		View.bLongShotDirectDefenseRollPending = false;
		View.bLongShotDeadCornerRollPending = false;
		View.bCutInsideShotDirectAttackRollPending = false;
		View.bCutInsideShotDirectDefenseRollPending = false;
		View.bCutInsideShotDeadCornerRollPending = false;
		View.bThroughBallFeetAttackRollPending = false;
		View.bThroughBallFeetDefenseRollPending = false;
		View.bThroughBallAntiOffsideAttackRollPending = false;
		View.bThroughBallOneOnOneChipShotAttackRollPending = false;
		View.bThroughBallOneOnOneDirectShotAttackRollPending = false;
		View.bThroughBallOneOnOneDirectShotDefenseRollPending = false;
		View.bThroughBallBehindDefenseAttackRollPending = false;
		View.bThroughBallBehindDefenseDefenseRollPending = false;
		View.bThroughBallFeetFormulaComplete = false;
		View.bThroughBallFeetTerminalActionAvailable = false;
		View.bTerminalPendingAdvance = false;
	}

	void RedactResolutionRolls(
		const FFMCodexLocalMatchViewerDisclosure& Disclosure,
		FFMCodexLocalMatchInteractionView& View)
	{
		TArray<FFMCodexLocalMatchRollView> DisclosedRolls;
		int32 SeenContestRolls = 0;
		for (const FFMCodexLocalMatchRollView& Roll : View.AcceptedRolls)
		{
			const bool bDisclosed = Roll.Group
				== EFMCodexLocalMatchRollGroup::InitialRoute
					? Disclosure.bRevealRouteRoll
					: SeenContestRolls++
						< FMath::Max(0, Disclosure.RevealedContestD6Count);
			if (bDisclosed)
			{
				DisclosedRolls.Add(Roll);
			}
		}
		View.AcceptedRolls = MoveTemp(DisclosedRolls);

		TArray<FMatchPlayResolutionRollFact> DisclosedFacts;
		int32 SeenContestFacts = 0;
		bool bRemovedAnyFact = false;
		for (const FMatchPlayResolutionRollFact& Roll : View.ResolutionFacts.Rolls)
		{
			const bool bDisclosed = Roll.bInitialRoute
				? Disclosure.bRevealRouteRoll
				: SeenContestFacts++
					< FMath::Max(0, Disclosure.RevealedContestD6Count);
			if (bDisclosed)
			{
				DisclosedFacts.Add(Roll);
			}
			else
			{
				bRemovedAnyFact = true;
			}
		}
		View.ResolutionFacts.Rolls = MoveTemp(DisclosedFacts);
		if (bRemovedAnyFact)
		{
			// Derived formula/decision payloads can encode a hidden raw roll.
			View.ResolutionFacts.FormulaContests.Reset();
			View.ResolutionFacts.Decisions.Reset();
		}
	}

	void RedactTerminalOutcome(
		const FMatchPlayState& Snapshot,
		FFMCodexLocalMatchInteractionView& View)
	{
		if (Snapshot.bHasCurrentAttack)
		{
			const int64 CurrentSequence = Snapshot.CurrentAttack.AttackSequence;
			for (int32 Index = View.GoalHistory.Num() - 1; Index >= 0; --Index)
			{
				const FMatchPlayGoalFact& Goal = View.GoalHistory[Index];
				if (Goal.AttackSequence != CurrentSequence)
				{
					continue;
				}
				if (Goal.ScoringSide == EInitialTurnOrderPlayer::PlayerA)
				{
					View.PlayerAScore = FMath::Max(0, View.PlayerAScore - 1);
				}
				else if (Goal.ScoringSide == EInitialTurnOrderPlayer::PlayerB)
				{
					View.PlayerBScore = FMath::Max(0, View.PlayerBScore - 1);
				}
				View.GoalHistory.RemoveAt(Index);
			}
		}

		View.bHasSetPieceOutcome = false;
		View.bSetPieceGoal = false;
		View.bSetPieceSystemGoal = false;
		View.SetPieceGoalScorerCardId = NAME_None;
		for (FMatchPlayResolutionDecisionFact& Decision
			: View.ResolutionFacts.Decisions)
		{
			switch (Decision.Outcome)
			{
			case EMatchPlayResolutionDecisionOutcome::Goal:
			case EMatchPlayResolutionDecisionOutcome::Miss:
			case EMatchPlayResolutionDecisionOutcome::ImmediateMiss:
			case EMatchPlayResolutionDecisionOutcome::OutOfPlay:
			case EMatchPlayResolutionDecisionOutcome::DefenderStoppedAttack:
			case EMatchPlayResolutionDecisionOutcome::Offside:
				Decision.bResolved = false;
				Decision.Outcome = EMatchPlayResolutionDecisionOutcome::None;
				break;
			default:
				break;
			}
		}
	}

	void ApplyViewerSafety(
		const FMatchPlayState& Snapshot,
		const EInitialTurnOrderPlayer ViewerSide,
		const FFMCodexLocalMatchViewerDisclosure& Disclosure,
		FFMCodexLocalMatchInteractionView& View)
	{
		const bool bValidViewer = IsPlayerSide(ViewerSide);
		if (!Disclosure.bRevealInitialActionPointRoll)
		{
			View.ActionPoint = 0;
			View.RawInitialD12 = 0;
			View.bCanFinishDeployment = false;
		View.bPlayerADeploymentFinished = false;
		View.bPlayerBDeploymentFinished = false;
		View.bDeploymentComplete = false;
		View.RouteKind = EMatchPlayCurrentAttackRouteKind::None;
			View.SendingOffEjectedCardId = NAME_None;
			View.SendingOffEjectedOwnerSide = EInitialTurnOrderPlayer::None;
			if (Snapshot.bHasCurrentAttack)
			{
				ClearUndisclosedAttackRouteProjection(View);
			}
		}

		if (!Disclosure.bRevealSetPieceTypeRoll)
		{
			const bool bWasResolved = View.bHasSetPieceTypeRoll;
			View.bHasSetPieceTypeRoll = false;
			View.RawSetPieceTypeD6 = 0;
			if (bWasResolved)
			{
				ClearSetPieceTypeDependentProjection(View);
			}
		}

		if (!Disclosure.bRevealParticipantSelectionRoll)
		{
			View.bHasCornerSharedParticipantD6 = false;
			View.CornerSharedParticipantD6 = 0;
			View.CornerRunner = {};
			View.CornerHelper = {};
			View.CornerCandidateBonusSide = EInitialTurnOrderPlayer::None;
			View.CornerCandidateBonus = 0;
		}
		if (!Disclosure.bRevealRouteRoll)
		{
			View.bHasCornerRouteD6 = false;
			View.CornerRouteD6 = 0;
			View.CornerActualRoute = EMatchPlayCornerRouteIntent::None;
			View.ActualBranchLabel.Reset();
			View.ResolutionFacts.bHasActualBranch = false;
			View.ResolutionFacts.ActualBranch = {};
		}

		const int32 ContestCount =
			FMath::Max(0, Disclosure.RevealedContestD6Count);
		if (ContestCount < 1)
		{
			View.bHasSetPieceAttackD6 = false;
			View.SetPieceAttackD6 = 0;
			View.bHasSetPieceAttackCurrentTotal = false;
			View.SetPieceAttackCurrentTotal = 0.0f;
		}
		if (ContestCount < 2)
		{
			View.bHasSetPieceDefenseD6 = false;
			View.SetPieceDefenseD6 = 0;
			View.bHasSetPieceDefenseCurrentTotal = false;
			View.SetPieceDefenseCurrentTotal = 0.0f;
		}
		const int32 RequiredPairedCount =
			View.SetPieceType == ESetPieceSelectedType::Penalty ? 1 : 2;
		if (ContestCount < RequiredPairedCount)
		{
			View.bHasSetPiecePairedD6 = false;
			View.SetPiecePairedD6A = 0;
			View.SetPiecePairedD6B = 0;
			View.SetPiecePairedD6Total = 0;
		}
		if ((!View.bHasSetPieceAttackD6 && !View.bHasSetPiecePairedD6)
			|| (View.SetPieceCarrierStage
					== EMatchPlaySetPieceCarrierRouteStage
						::DirectAwaitingDefenseRoll
				&& !View.bHasSetPieceDefenseD6))
		{
			View.bHasSetPieceFormula = false;
			View.SetPieceFormula = {};
		}

		RedactResolutionRolls(Disclosure, View);

		const bool bBothCornerSidesLocked =
			View.bCornerAttackerNominationsLocked
			&& View.bCornerDefenderNominationsLocked;
		const EInitialTurnOrderPlayer Attacker = View.CurrentAttackingPlayer;
		const EInitialTurnOrderPlayer Defender = OtherSide(Attacker);
		if (!bBothCornerSidesLocked)
		{
			if (!bValidViewer || ViewerSide != Attacker)
			{
				View.CornerAttackerNominees.Reset();
				View.CornerAttackerNomineeRollLabels.Reset();
				View.bHideCornerAttackerNomineeDetails = true;
			}
			else
			{
				View.bHideCornerAttackerNomineeDetails = false;
			}
			if (!bValidViewer || ViewerSide != Defender)
			{
				View.CornerDefenderNominees.Reset();
				View.CornerDefenderNomineeRollLabels.Reset();
			}
		}

		if (!bValidViewer || View.ExpectedActingPlayer != ViewerSide)
		{
			View.bCanFinishDeployment = false;
			View.DeploymentOptions.Reset();
			View.DeploymentGroups.Reset();
			View.SelectionOptions.Reset();
			View.LegalSetPieceCardIds.Reset();
			View.BranchIntentOptions.Reset();
			View.OneOnOneOptions.Reset();
			View.bHumanInteraction = false;
		}

		if (!Disclosure.bRevealTerminalOutcome)
		{
			RedactTerminalOutcome(Snapshot, View);
		}
	}
}

FFMCodexLocalMatchViewerDisclosure
FFMCodexLocalMatchViewerDisclosure::FullyDisclosed()
{
	FFMCodexLocalMatchViewerDisclosure Result;
	Result.bRevealInitialActionPointRoll = true;
	Result.bRevealSetPieceTypeRoll = true;
	Result.bRevealParticipantSelectionRoll = true;
	Result.bRevealRouteRoll = true;
	Result.RevealedContestD6Count = MAX_int32;
	Result.bRevealTerminalOutcome = true;
	return Result;
}

FFMCodexLocalMatchInteractionView
FFMCodexLocalMatchInteractionViewBuilder::BuildNoActiveMatch()
{
	FFMCodexLocalMatchInteractionView Result;
	Result.InteractionCategory =
		EFMCodexLocalMatchInteractionCategory::StartMatch;
	return Result;
}

FFMCodexLocalMatchInteractionView
FFMCodexLocalMatchInteractionViewBuilder::BuildAuthorityInternal(
	const FMatchPlayState& Snapshot,
	const FSkillRuleSnapshotSet& SkillRuleSet)
{
	using namespace FMCodexLocalMatchInteractionView;

	if (!Snapshot.RuntimeState.bIsInitialized)
	{
		return BuildNoActiveMatch();
	}

	FFMCodexLocalMatchInteractionView Result;
	Result.bMatchActive = true;
	Result.GoalHistory = Snapshot.GoalHistory;
	Result.PlayerAScore = Snapshot.RuntimeState.PlayerAState.Score;
	Result.PlayerBScore = Snapshot.RuntimeState.PlayerBState.Score;
	Result.CurrentAttackingPlayer =
		Snapshot.RuntimeState.CurrentAttackingPlayer;
	Result.PlayerAMaxAttackTurns =
		Snapshot.RuntimeState.PlayerAState.TotalAttackCount;
	Result.PlayerAUsedAttackTurns =
		Snapshot.RuntimeState.PlayerAState.UsedAttackCount;
	Result.PlayerBMaxAttackTurns =
		Snapshot.RuntimeState.PlayerBState.TotalAttackCount;
	Result.PlayerBUsedAttackTurns =
		Snapshot.RuntimeState.PlayerBState.UsedAttackCount;
	Result.bPlayerACurrentAttackTurn =
		Result.CurrentAttackingPlayer == EInitialTurnOrderPlayer::PlayerA
		&& Result.PlayerAUsedAttackTurns < Result.PlayerAMaxAttackTurns;
	Result.bPlayerBCurrentAttackTurn =
		Result.CurrentAttackingPlayer == EInitialTurnOrderPlayer::PlayerB
		&& Result.PlayerBUsedAttackTurns < Result.PlayerBMaxAttackTurns;
	Result.PlayerACurrentAttackIndex = Result.bPlayerACurrentAttackTurn
		? Result.PlayerAUsedAttackTurns + 1 : 0;
	Result.PlayerBCurrentAttackIndex = Result.bPlayerBCurrentAttackTurn
		? Result.PlayerBUsedAttackTurns + 1 : 0;
	Result.AttackSequence = static_cast<int64>(
		Result.PlayerAUsedAttackTurns + Result.PlayerBUsedAttackTurns + 1);
	const FMatchPlayTacticalPlayerAdvantageResult TacticalBoardStatus =
		FMatchPlayTacticalPlayerAdvantageQuery::EvaluateBoardStatus(Snapshot);
	if (TacticalBoardStatus.bSuccess)
	{
		Result.bHasTacticalPlayerCounts = true;
		Result.PlayerATacticalPlayerCount =
			TacticalBoardStatus.AttackingPlayer
				== EInitialTurnOrderPlayer::PlayerA
					? TacticalBoardStatus.AttackerTacticalPlayerCount
					: TacticalBoardStatus.DefenderTacticalPlayerCount;
		Result.PlayerBTacticalPlayerCount =
			TacticalBoardStatus.AttackingPlayer
				== EInitialTurnOrderPlayer::PlayerB
					? TacticalBoardStatus.AttackerTacticalPlayerCount
					: TacticalBoardStatus.DefenderTacticalPlayerCount;
	}
	BuildCardRosters(Snapshot, SkillRuleSet, Result);
	BuildRecoveryPresentation(Snapshot, Result);
	BuildPitchRegions(Snapshot, SkillRuleSet, Result);

	const FMatchEndResolveResult MatchEnd =
		FMatchEndResolver::ResolveMatchEnd(Snapshot.RuntimeState);
	Result.bMatchEnded = MatchEnd.bSuccess && MatchEnd.bIsMatchEnded;
	if (Result.bMatchEnded)
	{
		Result.FullTime.bVisible = true;
		auto BuildTeam = [&](const EInitialTurnOrderPlayer Side,
			const TArray<FFMCodexLocalMatchCardView>& Roster,
			const int32 Score, FFMCodexFullTimeTeamPresentation& Team)
		{
			const bool bA = Side == EInitialTurnOrderPlayer::PlayerA;
			Team.Name = NSLOCTEXT("FMCodexFullTime", "UnknownTeam", "球队");
			Team.BadgeMark = FText::FromString(bA ? TEXT("A") : TEXT("B"));
			Team.Score = FText::AsNumber(Score);
			FName TeamId;
			bool bSingleTeam = !Roster.IsEmpty();
			for (const auto& Card : Roster)
			{
				const auto* Definition = FFMCodexPrototypeTeamContent::Find(Card.CardId);
				if (!Definition || (!TeamId.IsNone() && TeamId != Definition->TeamId))
				{
					bSingleTeam = false;
					break;
				}
				TeamId = Definition->TeamId;
			}
			if (bSingleTeam && !TeamId.IsNone())
			{
				Team.Name = FFMCodexPrototypeTeamContent::TeamDisplayName(Roster[0].CardId);
				if (TeamId == FFMCodexPrototypeTeamContent::ArsenalTeamId())
				{
					Team.BadgeMark = FText::FromString(TEXT("A"));
					Team.Color = FLinearColor(0.50f, 0.035f, 0.055f);
				}
				else if (TeamId == FFMCodexPrototypeTeamContent::ManchesterCityTeamId())
				{
					Team.BadgeMark = FText::FromString(TEXT("MC"));
					Team.Color = FLinearColor(0.19f, 0.43f, 0.61f);
				}
			}
			for (const auto& Goal : Result.GoalHistory)
			{
				if (Goal.ScoringSide != Side) continue;
				const auto* Definition = FFMCodexPrototypeTeamContent::Find(Goal.ScorerCardId);
				Team.Goals.Add(Goal.bSystemAward
					? NSLOCTEXT("FMCodexFullTime", "AwardedGoal", "规则判定进球")
					: Definition && !Definition->PreferredDisplayName.IsEmpty()
						? Definition->PreferredDisplayName
						: NSLOCTEXT("FMCodexFullTime", "UnnamedScorer", "进球球员"));
			}
			if (Team.Goals.Num() < Score)
			{
				// Old snapshots may predate history; do not fabricate scorer entries.
				Team.Goals.Add(NSLOCTEXT("FMCodexFullTime", "HistoryUnavailable", "部分进球记录不可用"));
			}
		};
		BuildTeam(EInitialTurnOrderPlayer::PlayerA, Result.PlayerACardRoster,
			Result.PlayerAScore, Result.FullTime.PlayerA);
		BuildTeam(EInitialTurnOrderPlayer::PlayerB, Result.PlayerBCardRoster,
			Result.PlayerBScore, Result.FullTime.PlayerB);
		const FMatchResultResolveResult MatchResult =
			FMatchResultResolver::ResolveMatchResult(Snapshot.RuntimeState);
		Result.MatchResult = MatchResult.ResultType;
		Result.MajorPhase = EFMCodexLocalMatchMajorPhase::Complete;
		Result.InteractionCategory =
			EFMCodexLocalMatchInteractionCategory::MatchEnded;
		return Result;
	}

	Result.bCurrentAttackActive = Snapshot.bHasCurrentAttack;
	if (!Snapshot.bHasCurrentAttack)
	{
		Result.MajorPhase = EFMCodexLocalMatchMajorPhase::BetweenAttacks;
		Result.InteractionCategory =
			EFMCodexLocalMatchInteractionCategory::TacticalPointRoll;
		Result.ExpectedActingPlayer = Result.CurrentAttackingPlayer;
		Result.bTacticalPointRollReady = true;
		Result.bHumanInteraction = true;
		return Result;
	}

	const FMatchPlayCurrentAttackState& Attack = Snapshot.CurrentAttack;
	Result.AttackSequence = Attack.AttackSequence;
	Result.ActionPoint = Attack.ActionPoint;
	Result.RawInitialD12 = Attack.RawInitialD12;
	Result.RouteKind = Attack.RouteKind;
	Result.SelectionStage = Attack.SelectionStage;
	Result.CurrentLegalDeploymentSide = Attack.CurrentLegalDeploymentSide;
	Result.DeploymentPlacements = Attack.DeploymentPlacements;
	if (Attack.RouteKind == EMatchPlayCurrentAttackRouteKind::Ordinary)
	{
		const bool bAIsAttacker = Result.CurrentAttackingPlayer == EInitialTurnOrderPlayer::PlayerA;
		Result.bPlayerADeploymentFinished = bAIsAttacker
			? Attack.bAttackerDeploymentFinished : Attack.bDefenderDeploymentFinished;
		Result.bPlayerBDeploymentFinished = bAIsAttacker
			? Attack.bDefenderDeploymentFinished : Attack.bAttackerDeploymentFinished;
		Result.bDeploymentComplete = Attack.bAttackerDeploymentFinished && Attack.bDefenderDeploymentFinished;
	}
	if (Attack.RouteKind == EMatchPlayCurrentAttackRouteKind::SendingOff)
	{
		Result.MajorPhase = EFMCodexLocalMatchMajorPhase::Resolution;
		Result.ActionLabel = TEXT("罚下一人");
		Result.SendingOffEjectedCardId = Attack.SendingOffRoute.EjectedCardId;
		Result.SendingOffEjectedOwnerSide =
			Attack.SendingOffRoute.EjectedCardId.IsNone()
				? EInitialTurnOrderPlayer::None
				: Result.CurrentAttackingPlayer;
		if (Attack.LifecycleState
			== EMatchPlayCurrentAttackLifecycleState::TerminalPendingAdvance)
		{
			Result.InteractionCategory =
				EFMCodexLocalMatchInteractionCategory::AdvanceAfterTerminal;
			Result.ExpectedActingPlayer = Result.CurrentAttackingPlayer;
			Result.bHumanInteraction = true;
			Result.bTerminalPendingAdvance = true;
			Result.ContinueActionLabel = TEXT("下一回合");
		}
		else
		{
			Result.InteractionCategory =
				EFMCodexLocalMatchInteractionCategory::ResolveSendingOff;
			Result.bHumanInteraction = false;
		}
		return Result;
	}
	if (Attack.RouteKind == EMatchPlayCurrentAttackRouteKind::SetPiece)
	{
		BuildSetPieceView(Snapshot, Result);
		return Result;
	}
	if (Attack.bHasSelectedAction)
	{
		Result.SelectedSkillId = Attack.SelectedAction.SkillId;
		Result.SelectedCarrierCardId = Attack.SelectedAction.CarrierCardId;
		Result.SelectedRunnerCardId = Attack.SelectedAction.RunnerCardId;
		Result.SelectedMarkerCardId = Attack.SelectedAction.MarkerCardId;
		Result.SelectedHelperCardId = Attack.SelectedAction.bHasHelper
			? Attack.SelectedAction.HelperCardId : NAME_None;
	}
	else
	{
		Result.SelectedSkillId = Attack.ActionPreparation.SkillId;
		Result.SelectedCarrierCardId = Attack.ActionPreparation.CarrierCardId;
		Result.SelectedRunnerCardId = Attack.ActionPreparation.RunnerCardId;
		Result.SelectedMarkerCardId = Attack.ActionPreparation.MarkerCardId;
		Result.SelectedHelperCardId = Attack.ActionPreparation.bHasHelper
			? Attack.ActionPreparation.HelperCardId : NAME_None;
	}
	const ESkillRuleType PresentedActionType = Attack.bHasResolutionSession
		? Attack.ResolutionSession.Bundle.Binding.ActionType
		: Attack.bHasSelectedAction
			? Attack.SelectedAction.ActionType
			: Attack.ActionPreparation.ActionType;
	Result.PresentedActionType = PresentedActionType;
	if (PresentedActionType != ESkillRuleType::None)
	{
		Result.ActionLabel = ToString(PresentedActionType);
	}
	Result.ElectiveBranchIntent = Attack.bHasResolutionSession
		? Attack.ResolutionSession.Bundle.Binding.ElectiveBranchIntent
		: Attack.bHasSelectedAction
			? Attack.SelectedAction.ElectiveBranchIntent
			: EMatchPlayElectiveBranchIntent::None;
	if (Attack.bHasResolutionSession)
	{
		const FMatchPlayCurrentAttackResolutionSession& Session =
			Attack.ResolutionSession;
		if (Session.bHasActualBranch)
		{
			Result.ActualBranchLabel = ToString(Session.ActualBranch);
		}
		if (Session.ThroughBallOneOnOneShotChoice
			!= EMatchPlayThroughBallOneOnOneShotChoice::None)
		{
			Result.OneOnOneChoiceLabel = ToString(
				Session.ThroughBallOneOnOneShotChoice);
		}
	}
	if (Attack.Phase == EMatchPlayCurrentAttackPhase::Deployment)
	{
		Result.MajorPhase = EFMCodexLocalMatchMajorPhase::Deployment;
		Result.InteractionCategory =
			EFMCodexLocalMatchInteractionCategory::Deploy;
		Result.ExpectedActingPlayer = Attack.CurrentLegalDeploymentSide;
		Result.bHumanInteraction = true;
		BuildDeploymentOptions(Snapshot, SkillRuleSet, Result);
		// The pure canonical function returns a candidate copy; this read does not commit State.
		Result.bCanFinishDeployment = FMatchPlayFinishDeployment::Finish(
			Snapshot, Attack.AttackSequence, Attack.CurrentLegalDeploymentSide).bSuccess;
		return Result;
	}

	if (Attack.bHasResolutionSession)
	{
		const FMatchPlayCurrentAttackResolutionSession& Session =
			Attack.ResolutionSession;
		Result.ResolutionFacts =
			FMatchPlayCurrentAttackResolutionFactProjectionQuery::Project(
				Snapshot,
				&SkillRuleSet);
		for (const FMatchPlayCurrentAttackResolutionRollRecord& Roll
			: Session.InitialRouteRollRecords)
		{
			Result.AcceptedRolls.Add({
				EFMCodexLocalMatchRollGroup::InitialRoute,
				TEXT("Initial Route"),
				Roll.RawD6 });
		}
		for (const FMatchPlayCurrentAttackPostRouteRollRecord& Roll
			: Session.PostRouteRollProgress.RollRecords)
		{
			Result.AcceptedRolls.Add({
				RollGroup(Roll.Purpose),
				RollPurpose(Roll.Purpose),
				Roll.RawD6 });
		}

		Result.MajorPhase = EFMCodexLocalMatchMajorPhase::Resolution;
		if (Attack.LifecycleState
			== EMatchPlayCurrentAttackLifecycleState::TerminalPendingAdvance)
		{
			Result.InteractionCategory =
				EFMCodexLocalMatchInteractionCategory::AdvanceAfterTerminal;
			Result.ExpectedActingPlayer =
				Snapshot.RuntimeState.CurrentAttackingPlayer;
			Result.bHumanInteraction = true;
			Result.bTerminalPendingAdvance = true;
			if (Session.bHasActualBranch
				&& Session.ActualBranch.ActionType == ESkillRuleType::Cross)
			{
				Result.bCrossFormulaComplete =
					HasCompletedCrossTerminalContest(Result.ResolutionFacts);
			}
			if (Session.bHasActualBranch
				&& Session.ActualBranch.ActionType == ESkillRuleType::ThroughBall
				&& Session.ActualBranch.ThroughBall
					== EMatchPlayThroughBallActualBranch::Feet)
			{
				Result.bThroughBallFeetFormulaComplete =
					HasCompletedThroughBallFeetTerminalContest(
						Result.ResolutionFacts);
			}
			Result.ContinueActionLabel = TEXT("下一回合");
			return Result;
		}
		if (Session.Stage
				== EMatchPlayCurrentAttackResolutionStage::RouteResolved
			&& RequiresOneOnOneChoice(Snapshot, SkillRuleSet))
		{
			Result.InteractionCategory =
				EFMCodexLocalMatchInteractionCategory::SelectOneOnOneShot;
			Result.ExpectedActingPlayer = Result.CurrentAttackingPlayer;
			Result.bHumanInteraction = true;
			Result.OneOnOneOptions = {
				EMatchPlayThroughBallOneOnOneShotChoice::ChipShot,
				EMatchPlayThroughBallOneOnOneShotChoice::DirectShot
			};
			return Result;
		}
		if (Session.Stage
				== EMatchPlayCurrentAttackResolutionStage::RouteResolved
			&& Session.bHasActualBranch
			&& Session.ActualBranch.ActionType == ESkillRuleType::PassControl)
		{
			const auto Progress =
				FMatchPlayCurrentAttackPostRouteRollProgressQuery::Evaluate(
					Session);
			if (Progress.bIsCanonical && !Progress.bContractComplete)
			{
				const bool bAttackRoll =
					Session.PostRouteRollProgress.Phase
						== EMatchPlayCurrentAttackPostRouteRollPhase::None
					|| Progress.NextPurpose
						== EMatchPlayCurrentAttackPostRouteRollPurpose
							::PrimaryAttack;
				Result.InteractionCategory = bAttackRoll
					? EFMCodexLocalMatchInteractionCategory
						::RollPassControlAttack
					: EFMCodexLocalMatchInteractionCategory
						::RollPassControlDefense;
				Result.ExpectedActingPlayer = bAttackRoll
					? Session.Bundle.CurrentAttackingPlayer
					: Session.Bundle.CurrentDefendingPlayer;
				Result.bHumanInteraction = true;
				Result.ContinueActionLabel = bAttackRoll
					? TEXT("进攻方掷点")
					: TEXT("防守方掷点");
				return Result;
			}
			if (Progress.bIsCanonical && Progress.bContractComplete)
			{
				// Reconstruction-only recovery for legacy completed-roll snapshots.
				// Normal typed Defense completion advances atomically to terminal.
				Result.InteractionCategory =
					EFMCodexLocalMatchInteractionCategory::ContinueResolution;
				Result.ExpectedActingPlayer =
					Session.Bundle.CurrentAttackingPlayer;
				Result.bHumanInteraction = true;
				Result.ContinueActionLabel = TEXT("确认结算结果");
				return Result;
			}
		}
		if (Session.Stage
				== EMatchPlayCurrentAttackResolutionStage::RouteResolved
			&& Session.bHasActualBranch
			&& Session.ActualBranch.ActionType == ESkillRuleType::LongShot)
		{
			const auto Progress =
				FMatchPlayCurrentAttackPostRouteRollProgressQuery::Evaluate(
					Session);
			if (Progress.bIsCanonical && !Progress.bContractComplete
				&& Session.ActualBranch.LongShot
					== EMatchPlayLongShotActualBranch::DirectShot)
			{
				const bool bAttackRoll =
					Session.PostRouteRollProgress.Phase
						== EMatchPlayCurrentAttackPostRouteRollPhase::None
					|| Progress.NextPurpose
						== EMatchPlayCurrentAttackPostRouteRollPurpose
							::PrimaryAttack;
				Result.InteractionCategory = bAttackRoll
					? EFMCodexLocalMatchInteractionCategory
						::RollLongShotDirectAttack
					: EFMCodexLocalMatchInteractionCategory
						::RollLongShotDirectDefense;
				Result.bLongShotDirectAttackRollPending = bAttackRoll;
				Result.bLongShotDirectDefenseRollPending = !bAttackRoll;
				Result.ExpectedActingPlayer = bAttackRoll
					? Session.Bundle.CurrentAttackingPlayer
					: Session.Bundle.CurrentDefendingPlayer;
				Result.bHumanInteraction = true;
				Result.ContinueActionLabel = bAttackRoll
					? TEXT("进攻方掷远射点数")
					: TEXT("防守方掷防守点数");
				return Result;
			}
			if (Progress.bIsCanonical && !Progress.bContractComplete
				&& Session.ActualBranch.LongShot
					== EMatchPlayLongShotActualBranch::DeadCorner
				&& (Session.PostRouteRollProgress.Phase
						== EMatchPlayCurrentAttackPostRouteRollPhase::None
					|| Progress.NextPurpose
						== EMatchPlayCurrentAttackPostRouteRollPurpose
							::PairedAttackA))
			{
				Result.InteractionCategory =
					EFMCodexLocalMatchInteractionCategory
						::RollLongShotDeadCorner;
				Result.bLongShotDeadCornerRollPending = true;
				Result.ExpectedActingPlayer =
					Session.Bundle.CurrentAttackingPlayer;
				Result.bHumanInteraction = true;
				Result.ContinueActionLabel = TEXT("进攻方掷远射双骰");
				return Result;
			}
			if (Progress.bIsCanonical && Progress.bContractComplete)
			{
				Result.InteractionCategory =
					EFMCodexLocalMatchInteractionCategory::ContinueResolution;
				Result.ExpectedActingPlayer =
					Session.Bundle.CurrentAttackingPlayer;
				Result.bHumanInteraction = true;
				Result.ContinueActionLabel = TEXT("确认结算结果");
				return Result;
			}
		}
		if (Session.Stage
				== EMatchPlayCurrentAttackResolutionStage::RouteResolved
			&& Session.bHasActualBranch
			&& Session.ActualBranch.ActionType
				== ESkillRuleType::CutInsideShot)
		{
			const auto Progress =
				FMatchPlayCurrentAttackPostRouteRollProgressQuery::Evaluate(
					Session);
			if (Progress.bIsCanonical && !Progress.bContractComplete
				&& Session.ActualBranch.CutInsideShot
					== EMatchPlayCutInsideShotActualBranch::DirectShot)
			{
				const bool bAttackRoll =
					Session.PostRouteRollProgress.Phase
						== EMatchPlayCurrentAttackPostRouteRollPhase::None
					|| Progress.NextPurpose
						== EMatchPlayCurrentAttackPostRouteRollPurpose
							::PrimaryAttack;
				Result.InteractionCategory = bAttackRoll
					? EFMCodexLocalMatchInteractionCategory
						::RollCutInsideShotDirectAttack
					: EFMCodexLocalMatchInteractionCategory
						::RollCutInsideShotDirectDefense;
				Result.bCutInsideShotDirectAttackRollPending = bAttackRoll;
				Result.bCutInsideShotDirectDefenseRollPending = !bAttackRoll;
				Result.ExpectedActingPlayer = bAttackRoll
					? Session.Bundle.CurrentAttackingPlayer
					: Session.Bundle.CurrentDefendingPlayer;
				Result.bHumanInteraction = true;
				Result.ContinueActionLabel = bAttackRoll
					? TEXT("进攻方掷内切射门点数")
					: TEXT("防守方掷防守点数");
				return Result;
			}
			if (Progress.bIsCanonical && !Progress.bContractComplete
				&& Session.ActualBranch.CutInsideShot
					== EMatchPlayCutInsideShotActualBranch::DeadCorner
				&& (Session.PostRouteRollProgress.Phase
						== EMatchPlayCurrentAttackPostRouteRollPhase::None
					|| Progress.NextPurpose
						== EMatchPlayCurrentAttackPostRouteRollPurpose
							::PairedAttackA))
			{
				Result.InteractionCategory =
					EFMCodexLocalMatchInteractionCategory
						::RollCutInsideShotDeadCorner;
				Result.bCutInsideShotDeadCornerRollPending = true;
				Result.ExpectedActingPlayer =
					Session.Bundle.CurrentAttackingPlayer;
				Result.bHumanInteraction = true;
				Result.ContinueActionLabel = TEXT("进攻方掷内切死角双骰");
				return Result;
			}
			if (Progress.bIsCanonical && Progress.bContractComplete)
			{
				Result.InteractionCategory =
					EFMCodexLocalMatchInteractionCategory::ContinueResolution;
				Result.ExpectedActingPlayer =
					Session.Bundle.CurrentAttackingPlayer;
				Result.bHumanInteraction = true;
				Result.ContinueActionLabel = TEXT("确认结算结果");
				return Result;
			}
		}
		if (Session.Stage
				== EMatchPlayCurrentAttackResolutionStage::RouteResolved
			&& Session.bHasActualBranch
			&& Session.ActualBranch.ActionType == ESkillRuleType::Cross
			&& (Session.ActualBranch.Cross
					== EMatchPlayCrossActualBranch::High
				|| Session.ActualBranch.Cross
					== EMatchPlayCrossActualBranch::Low))
		{
			const auto Progress =
				FMatchPlayCurrentAttackPostRouteRollProgressQuery::Evaluate(
					Session);
			if (Progress.bIsCanonical && !Progress.bContractComplete)
			{
				const bool bAttackRoll = Session.PostRouteRollProgress.Phase
					== EMatchPlayCurrentAttackPostRouteRollPhase::None
					|| Progress.NextPurpose
						== EMatchPlayCurrentAttackPostRouteRollPurpose
							::PrimaryAttack;
				Result.InteractionCategory = bAttackRoll
					? EFMCodexLocalMatchInteractionCategory
						::RollCrossAttack
					: EFMCodexLocalMatchInteractionCategory
						::RollCrossDefense;
				Result.bCrossAttackRollPending = bAttackRoll;
				Result.bCrossDefenseRollPending = !bAttackRoll;
				Result.ExpectedActingPlayer = bAttackRoll
					? Session.Bundle.CurrentAttackingPlayer
					: Session.Bundle.CurrentDefendingPlayer;
				Result.bHumanInteraction = true;
				Result.ContinueActionLabel = bAttackRoll
					? TEXT("进攻方掷点")
					: TEXT("防守方掷点");
				return Result;
			}
			if (Progress.bIsCanonical && Progress.bContractComplete
				&& HasCompletedCrossTerminalContest(Result.ResolutionFacts))
			{
				Result.InteractionCategory =
					EFMCodexLocalMatchInteractionCategory
						::ApplyCrossTerminalResolution;
				Result.ExpectedActingPlayer =
					Session.Bundle.CurrentAttackingPlayer;
				Result.bHumanInteraction = true;
				Result.bCrossFormulaComplete = true;
				Result.bCrossTerminalActionAvailable = true;
				Result.ContinueActionLabel = TEXT("确认结算结果");
				return Result;
			}
			if (Progress.bIsCanonical && Progress.bContractComplete)
			{
				Result.InteractionCategory =
					EFMCodexLocalMatchInteractionCategory::None;
				Result.Diagnostic = TEXT(
					"Completed Cross rolls do not project one authoritative terminal contest.");
				return Result;
			}
		}
		if (Session.Stage
				== EMatchPlayCurrentAttackResolutionStage::RouteResolved
			&& Session.bHasActualBranch
			&& Session.ActualBranch.ActionType == ESkillRuleType::ThroughBall
			&& Session.ActualBranch.ThroughBall
				== EMatchPlayThroughBallActualBranch::Feet)
		{
			const auto Progress =
				FMatchPlayCurrentAttackPostRouteRollProgressQuery::Evaluate(
					Session);
			if (Progress.bIsCanonical && !Progress.bContractComplete)
			{
				const bool bAttackRoll = Session.PostRouteRollProgress.Phase
					== EMatchPlayCurrentAttackPostRouteRollPhase::None
					|| Progress.NextPurpose
						== EMatchPlayCurrentAttackPostRouteRollPurpose
							::PrimaryAttack;
				Result.InteractionCategory = bAttackRoll
					? EFMCodexLocalMatchInteractionCategory
						::RollThroughBallFeetAttack
					: EFMCodexLocalMatchInteractionCategory
						::RollThroughBallFeetDefense;
				Result.bThroughBallFeetAttackRollPending = bAttackRoll;
				Result.bThroughBallFeetDefenseRollPending = !bAttackRoll;
				Result.ExpectedActingPlayer = bAttackRoll
					? Session.Bundle.CurrentAttackingPlayer
					: Session.Bundle.CurrentDefendingPlayer;
				Result.bHumanInteraction = true;
				Result.ContinueActionLabel = bAttackRoll
					? TEXT("掷进攻方点数")
					: TEXT("掷防守方点数");
				return Result;
			}
			if (Progress.bIsCanonical && Progress.bContractComplete
				&& HasCompletedThroughBallFeetTerminalContest(
					Result.ResolutionFacts))
			{
				Result.InteractionCategory =
					EFMCodexLocalMatchInteractionCategory
						::ApplyThroughBallFeetTerminalResolution;
				Result.ExpectedActingPlayer =
					Session.Bundle.CurrentAttackingPlayer;
				Result.bHumanInteraction = true;
				Result.bThroughBallFeetFormulaComplete = true;
				Result.bThroughBallFeetTerminalActionAvailable = true;
				Result.ContinueActionLabel = TEXT("确认结算结果");
				return Result;
			}
			if (Progress.bIsCanonical && Progress.bContractComplete)
			{
				Result.InteractionCategory =
					EFMCodexLocalMatchInteractionCategory::None;
				Result.Diagnostic = TEXT(
					"Completed ThroughBall Feet rolls do not project one authoritative terminal contest.");
				return Result;
			}
		}
		if (Session.Stage
				== EMatchPlayCurrentAttackResolutionStage::RouteResolved
			&& Session.bHasActualBranch
			&& Session.ActualBranch.ActionType == ESkillRuleType::ThroughBall
			&& Session.ActualBranch.ThroughBall
				== EMatchPlayThroughBallActualBranch::AntiOffside
			&& Session.ThroughBallOneOnOneShotChoice
				== EMatchPlayThroughBallOneOnOneShotChoice::None)
		{
			const auto Progress =
				FMatchPlayCurrentAttackPostRouteRollProgressQuery::Evaluate(
					Session);
			const bool bAttackRollPending = Progress.bIsCanonical
				&& !Progress.bContractComplete
				&& (Session.PostRouteRollProgress.Phase
						== EMatchPlayCurrentAttackPostRouteRollPhase::None
					|| Progress.NextPurpose
						== EMatchPlayCurrentAttackPostRouteRollPurpose
							::PrimaryAttack);
			if (bAttackRollPending)
			{
				Result.InteractionCategory =
					EFMCodexLocalMatchInteractionCategory
						::RollThroughBallAntiOffsideAttack;
				Result.bThroughBallAntiOffsideAttackRollPending = true;
				Result.ExpectedActingPlayer =
					Session.Bundle.CurrentAttackingPlayer;
				Result.bHumanInteraction = true;
				Result.ContinueActionLabel = TEXT("进攻方掷点");
				return Result;
			}
		}
		if (Session.Stage
				== EMatchPlayCurrentAttackResolutionStage::RouteResolved
			&& Session.bHasActualBranch
			&& Session.ActualBranch.ActionType == ESkillRuleType::ThroughBall
			&& Session.ThroughBallOneOnOneShotChoice
				!= EMatchPlayThroughBallOneOnOneShotChoice::None)
		{
			const auto Progress =
				FMatchPlayCurrentAttackPostRouteRollProgressQuery::Evaluate(
					Session);
			const auto Phase = Session.PostRouteRollProgress.Phase;
			if (Session.ThroughBallOneOnOneShotChoice
					== EMatchPlayThroughBallOneOnOneShotChoice::ChipShot
				&& Progress.bIsCanonical
				&& (Phase == EMatchPlayCurrentAttackPostRouteRollPhase::PrimaryBranch
					|| (Phase
							== EMatchPlayCurrentAttackPostRouteRollPhase
								::OneOnOneChipShot
						&& !Progress.bContractComplete)))
			{
				Result.InteractionCategory =
					EFMCodexLocalMatchInteractionCategory
						::RollThroughBallOneOnOneChipShotAttack;
				Result.bThroughBallOneOnOneChipShotAttackRollPending = true;
				Result.ExpectedActingPlayer =
					Session.Bundle.CurrentAttackingPlayer;
				Result.bHumanInteraction = true;
				Result.ContinueActionLabel = TEXT("进攻方掷挑射点数");
				return Result;
			}
			if (Session.ThroughBallOneOnOneShotChoice
					== EMatchPlayThroughBallOneOnOneShotChoice::DirectShot
				&& Progress.bIsCanonical
				&& (Phase == EMatchPlayCurrentAttackPostRouteRollPhase::PrimaryBranch
					|| (Phase
							== EMatchPlayCurrentAttackPostRouteRollPhase
								::OneOnOneDirectShot
						&& !Progress.bContractComplete)))
			{
				const bool bAttackRoll = Phase
					== EMatchPlayCurrentAttackPostRouteRollPhase::PrimaryBranch
					|| Progress.NextPurpose
						== EMatchPlayCurrentAttackPostRouteRollPurpose
							::OneOnOneDirectShotAttack;
				Result.InteractionCategory = bAttackRoll
					? EFMCodexLocalMatchInteractionCategory
						::RollThroughBallOneOnOneDirectShotAttack
					: EFMCodexLocalMatchInteractionCategory
						::RollThroughBallOneOnOneDirectShotDefense;
				Result.bThroughBallOneOnOneDirectShotAttackRollPending =
					bAttackRoll;
				Result.bThroughBallOneOnOneDirectShotDefenseRollPending =
					!bAttackRoll;
				Result.ExpectedActingPlayer = bAttackRoll
					? Session.Bundle.CurrentAttackingPlayer
					: Session.Bundle.CurrentDefendingPlayer;
				Result.bHumanInteraction = true;
				Result.ContinueActionLabel = bAttackRoll
					? TEXT("进攻方掷单刀射门点数")
					: TEXT("防守方掷单刀防守点数");
				return Result;
			}
		}
		if (Session.Stage
				== EMatchPlayCurrentAttackResolutionStage::RouteResolved
			&& Session.bHasActualBranch
			&& Session.ActualBranch.ActionType == ESkillRuleType::ThroughBall
			&& Session.ActualBranch.ThroughBall
				== EMatchPlayThroughBallActualBranch::BehindDefense)
		{
			const auto Progress =
				FMatchPlayCurrentAttackPostRouteRollProgressQuery::Evaluate(
					Session);
			if (Progress.bIsCanonical && !Progress.bContractComplete)
			{
				const bool bAttackRoll =
					Session.PostRouteRollProgress.Phase
						== EMatchPlayCurrentAttackPostRouteRollPhase::None
					|| Progress.NextPurpose
						== EMatchPlayCurrentAttackPostRouteRollPurpose
							::PrimaryAttack;
				Result.InteractionCategory = bAttackRoll
					? EFMCodexLocalMatchInteractionCategory
						::RollThroughBallBehindDefenseAttack
					: EFMCodexLocalMatchInteractionCategory
						::RollThroughBallBehindDefenseDefense;
				Result.bThroughBallBehindDefenseAttackRollPending = bAttackRoll;
				Result.bThroughBallBehindDefenseDefenseRollPending = !bAttackRoll;
				Result.ExpectedActingPlayer = bAttackRoll
					? Session.Bundle.CurrentAttackingPlayer
					: Session.Bundle.CurrentDefendingPlayer;
				Result.bHumanInteraction = true;
				Result.ContinueActionLabel = bAttackRoll
					? TEXT("进攻方掷点")
					: TEXT("防守方掷点");
				return Result;
			}
			if (Progress.bIsCanonical && Progress.bContractComplete)
			{
				Result.InteractionCategory =
					EFMCodexLocalMatchInteractionCategory::ContinueResolution;
				Result.ExpectedActingPlayer =
					Session.Bundle.CurrentAttackingPlayer;
				Result.bHumanInteraction = true;
				Result.ContinueActionLabel = TEXT("确认结算结果");
				return Result;
			}
		}
		if (Session.Stage
			== EMatchPlayCurrentAttackResolutionStage::AwaitingRoute)
		{
			if (Session.Bundle.Binding.ActionType == ESkillRuleType::Cross)
			{
				Result.InteractionCategory =
					EFMCodexLocalMatchInteractionCategory::RollCrossRoute;
				Result.ExpectedActingPlayer =
					Session.Bundle.CurrentAttackingPlayer;
				Result.bHumanInteraction = true;
				Result.ContinueActionLabel = TEXT("判定传中路线");
				return Result;
			}
			if (Session.Bundle.Binding.ActionType
				== ESkillRuleType::ThroughBall)
			{
				Result.InteractionCategory = EFMCodexLocalMatchInteractionCategory
					::RollThroughBallInitialRoute;
				Result.ExpectedActingPlayer =
					Session.Bundle.CurrentAttackingPlayer;
				Result.bHumanInteraction = true;
				Result.ContinueActionLabel = TEXT("判定直塞路线");
				return Result;
			}
			if (Session.Bundle.Binding.ActionType
				== ESkillRuleType::PassControl)
			{
				Result.InteractionCategory =
					EFMCodexLocalMatchInteractionCategory::RollPassControlRoute;
				Result.ExpectedActingPlayer =
					Session.Bundle.CurrentAttackingPlayer;
				Result.bHumanInteraction = true;
				Result.ContinueActionLabel = TEXT("判定控球推进路线");
				return Result;
			}
			Result.InteractionCategory =
				EFMCodexLocalMatchInteractionCategory::ContinueResolution;
			Result.ContinueActionLabel =
				Session.Bundle.Binding.ActionType == ESkillRuleType::Cross
					? TEXT("判定传中路线")
					: TEXT("Continue - Resolve Route");
		}
		else
		{
			Result.InteractionCategory =
				EFMCodexLocalMatchInteractionCategory::ContinueResolution;
			const auto Progress =
				FMatchPlayCurrentAttackPostRouteRollProgressQuery::Evaluate(Session);
			Result.ContinueActionLabel = Session.Bundle.Binding.ActionType
				== ESkillRuleType::ThroughBall
					? TEXT("继续直塞结算")
					: Progress.bIsCanonical
				&& Progress.bContractComplete
					? TEXT("Continue - Apply Formula / Result")
					: TEXT("Continue - Resolve Post-route Step");
		}
		return Result;
	}

	if (Attack.LifecycleState
		== EMatchPlayCurrentAttackLifecycleState::TerminalPendingAdvance)
	{
		Result.MajorPhase = EFMCodexLocalMatchMajorPhase::Resolution;
		Result.InteractionCategory =
			EFMCodexLocalMatchInteractionCategory::AdvanceAfterTerminal;
		Result.ExpectedActingPlayer =
			Snapshot.RuntimeState.CurrentAttackingPlayer;
		Result.bHumanInteraction = true;
		Result.bTerminalPendingAdvance = true;
		Result.ContinueActionLabel = TEXT("下一回合");
		return Result;
	}

	Result.MajorPhase = EFMCodexLocalMatchMajorPhase::Selection;
	Result.bHumanInteraction = Attack.SelectionStage
		!= EMatchPlayCurrentAttackSelectionStage::ReadyForResolution;
	BuildSelectionOptions(Snapshot, SkillRuleSet, Result);
	AttachSelectionCardViews(Snapshot, SkillRuleSet, Result);
	if (Result.InteractionCategory
		== EFMCodexLocalMatchInteractionCategory::ContinueResolution)
	{
		Result.ContinueActionLabel = PresentedActionType == ESkillRuleType::Cross
			? TEXT("判定传中路线")
			: PresentedActionType == ESkillRuleType::ThroughBall
				? TEXT("开始直塞判定")
			: TEXT("Continue - Begin Resolution");
	}
	return Result;
}

FFMCodexLocalMatchInteractionView
FFMCodexLocalMatchInteractionViewBuilder::BuildForViewer(
	const FMatchPlayState& Snapshot,
	const FSkillRuleSnapshotSet& SkillRuleSet,
	const EInitialTurnOrderPlayer ViewerSide,
	const FFMCodexLocalMatchViewerDisclosure& Disclosure)
{
	FFMCodexLocalMatchInteractionView Result =
		BuildAuthorityInternal(Snapshot, SkillRuleSet);
	if (!Result.bMatchActive)
	{
		return Result;
	}

	const FFMCodexLocalMatchViewerDisclosure EffectiveDisclosure =
		Result.bMatchEnded
			? FFMCodexLocalMatchViewerDisclosure::FullyDisclosed()
			: Disclosure;
	FMCodexLocalMatchInteractionView::ApplyViewerSafety(
		Snapshot,
		ViewerSide,
		EffectiveDisclosure,
		Result);
	return Result;
}

#if WITH_DEV_AUTOMATION_TESTS
FFMCodexLocalMatchInteractionView
FFMCodexLocalMatchInteractionViewBuilder::Build(
	const FMatchPlayState& Snapshot,
	const FSkillRuleSnapshotSet& SkillRuleSet)
{
	return BuildAuthorityInternal(Snapshot, SkillRuleSet);
}
#endif

TArray<FFMCodexLocalMatchCardView::FSkill>
FFMCodexLocalMatchInteractionViewBuilder::ProjectEligibleTacticalSkills(
	const TArray<FFMCodexLocalMatchCardView::FSkill>& StaticSkills,
	const int32 CurrentTacticalPoint)
{
	TArray<FFMCodexLocalMatchCardView::FSkill> Result;
	for (const FFMCodexLocalMatchCardView::FSkill& Skill : StaticSkills)
	{
		if (CurrentTacticalPoint >= Skill.MinTriggerActionPoint
			&& CurrentTacticalPoint <= Skill.MaxTriggerActionPoint)
		{
			Result.Add(Skill);
		}
	}

	ensureAlwaysMsgf(Result.Num() <= 2,
		TEXT("Canonical player content projected %d eligible Tactical Skills at TP %d; the supported invariant is at most 2."),
		Result.Num(), CurrentTacticalPoint);
	return Result;
}

FString FFMCodexLocalMatchInteractionViewBuilder::ToString(
	const EFMCodexLocalMatchMajorPhase Phase)
{
	switch (Phase)
	{
	case EFMCodexLocalMatchMajorPhase::NoActiveMatch: return TEXT("No Active Match");
	case EFMCodexLocalMatchMajorPhase::BetweenAttacks: return TEXT("Between Attacks");
	case EFMCodexLocalMatchMajorPhase::Deployment: return TEXT("Deployment");
	case EFMCodexLocalMatchMajorPhase::Selection: return TEXT("Selection");
	case EFMCodexLocalMatchMajorPhase::Resolution: return TEXT("Resolution");
	case EFMCodexLocalMatchMajorPhase::Complete: return TEXT("Complete");
	default: return TEXT("Unknown");
	}
}

FFMCodexLocalMatchScreenPresentation
FFMCodexLocalMatchInteractionViewBuilder::BuildScreenPresentation(
	const FFMCodexLocalMatchInteractionView& View)
{
	FFMCodexLocalMatchScreenPresentation Result;
	Result.MatchStatusLabel = View.bMatchEnded
		? TEXT("MATCH ENDED")
		: View.bMatchActive ? TEXT("MATCH IN PROGRESS") : TEXT("READY TO PLAY");
	Result.bSystemResolution = View.bMatchActive
		&& !View.bMatchEnded
		&& View.ExpectedActingPlayer == EInitialTurnOrderPlayer::None;
	Result.ActingStatusLabel = View.bMatchEnded
		? TEXT("FINAL RESULT")
		: View.ExpectedActingPlayer == EInitialTurnOrderPlayer::PlayerA
			? TEXT("PLAYER A TO ACT")
			: View.ExpectedActingPlayer == EInitialTurnOrderPlayer::PlayerB
				? TEXT("PLAYER B TO ACT")
				: Result.bSystemResolution
					? TEXT("SYSTEM RESOLUTION") : TEXT("WAITING TO START");
	Result.InteractionKicker = View.bMatchEnded
		? TEXT("MATCH COMPLETE")
		: View.bHumanInteraction ? TEXT("PLAYER ACTION")
			: Result.bSystemResolution ? TEXT("SYSTEM RESOLUTION")
				: TEXT("LOCAL MATCH");

	switch (View.InteractionCategory)
	{
	case EFMCodexLocalMatchInteractionCategory::StartMatch:
		Result.InteractionTitle = TEXT("Start a Local Match");
		break;
	case EFMCodexLocalMatchInteractionCategory::TacticalPointRoll:
		Result.InteractionTitle = TEXT("Roll Tactical Points");
		break;
	case EFMCodexLocalMatchInteractionCategory::Deploy:
		Result.InteractionTitle = TEXT("Deploy Your Cards");
		break;
	case EFMCodexLocalMatchInteractionCategory::SelectCarrier:
		Result.InteractionTitle = TEXT("Select Carrier");
		break;
	case EFMCodexLocalMatchInteractionCategory::SelectMarker:
		Result.InteractionTitle = TEXT("Select Marker");
		break;
	case EFMCodexLocalMatchInteractionCategory::SelectSkill:
		Result.InteractionTitle = TEXT("Choose Skill");
		break;
	case EFMCodexLocalMatchInteractionCategory::SelectRunner:
		Result.InteractionTitle = TEXT("Select Runner");
		break;
	case EFMCodexLocalMatchInteractionCategory::SelectHelper:
		Result.InteractionTitle = TEXT("Select Helper");
		break;
	case EFMCodexLocalMatchInteractionCategory::SelectBranchIntent:
		Result.InteractionTitle = View.ActionLabel == TEXT("Cross")
			? TEXT("Choose Cross Type") : TEXT("Choose Shot Type");
		break;
	case EFMCodexLocalMatchInteractionCategory::SelectLongShotBranch:
		Result.InteractionTitle = TEXT("选择远射方式");
		break;
	case EFMCodexLocalMatchInteractionCategory::SelectOneOnOneShot:
		Result.InteractionTitle = TEXT("Choose One-on-One Shot");
		break;
	case EFMCodexLocalMatchInteractionCategory::RollCrossAttack:
		Result.InteractionTitle = TEXT("进攻方掷点");
		break;
	case EFMCodexLocalMatchInteractionCategory::RollCrossDefense:
		Result.InteractionTitle = TEXT("防守方掷点");
		break;
	case EFMCodexLocalMatchInteractionCategory::RollLongShotDirectAttack:
		Result.InteractionTitle = TEXT("进攻方掷远射点数");
		break;
	case EFMCodexLocalMatchInteractionCategory::RollLongShotDirectDefense:
		Result.InteractionTitle = TEXT("防守方掷防守点数");
		break;
	case EFMCodexLocalMatchInteractionCategory::RollLongShotDeadCorner:
		Result.InteractionTitle = TEXT("进攻方掷远射双骰");
		break;
	case EFMCodexLocalMatchInteractionCategory::RollCutInsideShotDirectAttack:
		Result.InteractionTitle = TEXT("进攻方掷内切射门点数");
		break;
	case EFMCodexLocalMatchInteractionCategory::RollCutInsideShotDirectDefense:
		Result.InteractionTitle = TEXT("防守方掷防守点数");
		break;
	case EFMCodexLocalMatchInteractionCategory::RollCutInsideShotDeadCorner:
		Result.InteractionTitle = TEXT("进攻方掷内切死角双骰");
		break;
	case EFMCodexLocalMatchInteractionCategory::RollThroughBallInitialRoute:
		Result.InteractionTitle = TEXT("判定直塞路线");
		break;
	case EFMCodexLocalMatchInteractionCategory::RollPassControlRoute:
		Result.InteractionTitle = TEXT("判定控球推进路线");
		break;
	case EFMCodexLocalMatchInteractionCategory::RollCrossRoute:
		Result.InteractionTitle = TEXT("判定传中路线");
		break;
	case EFMCodexLocalMatchInteractionCategory::RollPassControlAttack:
		Result.InteractionTitle = TEXT("进攻方掷点");
		break;
	case EFMCodexLocalMatchInteractionCategory::RollPassControlDefense:
		Result.InteractionTitle = TEXT("防守方掷点");
		break;
	case EFMCodexLocalMatchInteractionCategory::CompleteCrossAndAdvance:
		Result.InteractionTitle = TEXT("下一回合");
		break;
	case EFMCodexLocalMatchInteractionCategory::RollThroughBallFeetAttack:
		Result.InteractionTitle = TEXT("掷进攻方点数");
		break;
	case EFMCodexLocalMatchInteractionCategory::RollThroughBallFeetDefense:
		Result.InteractionTitle = TEXT("掷防守方点数");
		break;
	case EFMCodexLocalMatchInteractionCategory
		::RollThroughBallAntiOffsideAttack:
		Result.InteractionTitle = TEXT("进攻方掷反越位点数");
		break;
	case EFMCodexLocalMatchInteractionCategory
		::RollThroughBallOneOnOneChipShotAttack:
		Result.InteractionTitle = TEXT("进攻方掷挑射点数");
		break;
	case EFMCodexLocalMatchInteractionCategory
		::RollThroughBallOneOnOneDirectShotAttack:
		Result.InteractionTitle = TEXT("进攻方掷单刀射门点数");
		break;
	case EFMCodexLocalMatchInteractionCategory
		::RollThroughBallOneOnOneDirectShotDefense:
		Result.InteractionTitle = TEXT("防守方掷单刀防守点数");
		break;
	case EFMCodexLocalMatchInteractionCategory
		::RollThroughBallBehindDefenseAttack:
		Result.InteractionTitle = TEXT("进攻方掷点");
		break;
	case EFMCodexLocalMatchInteractionCategory
		::RollThroughBallBehindDefenseDefense:
		Result.InteractionTitle = TEXT("防守方掷点");
		break;
	case EFMCodexLocalMatchInteractionCategory
		::CompleteThroughBallFeetAndAdvance:
		Result.InteractionTitle = TEXT("下一回合");
		break;
	case EFMCodexLocalMatchInteractionCategory::ApplyCrossTerminalResolution:
	case EFMCodexLocalMatchInteractionCategory
		::ApplyThroughBallFeetTerminalResolution:
		Result.InteractionTitle = TEXT("确认结算结果");
		break;
	case EFMCodexLocalMatchInteractionCategory::AdvanceAfterTerminal:
		Result.InteractionTitle = TEXT("下一回合");
		break;
	case EFMCodexLocalMatchInteractionCategory::ContinueResolution:
		Result.InteractionTitle = View.ContinueActionLabel.IsEmpty()
			? TEXT("继续结算") : View.ContinueActionLabel;
		break;
	case EFMCodexLocalMatchInteractionCategory::AttackComplete:
		Result.InteractionTitle = TEXT("Attack Complete");
		break;
	case EFMCodexLocalMatchInteractionCategory::MatchEnded:
		Result.InteractionTitle = TEXT("Match Complete");
		break;
	default:
		Result.InteractionTitle = TEXT("Unable to Load Match");
		break;
	}
	return Result;
}

FString FFMCodexLocalMatchInteractionViewBuilder::ToString(
	const EFMCodexLocalMatchInteractionCategory Category)
{
	switch (Category)
	{
	case EFMCodexLocalMatchInteractionCategory::None: return TEXT("None");
	case EFMCodexLocalMatchInteractionCategory::StartMatch: return TEXT("Start Match");
	case EFMCodexLocalMatchInteractionCategory::TacticalPointRoll: return TEXT("Tactical Point Roll");
	case EFMCodexLocalMatchInteractionCategory::Deploy: return TEXT("Deploy / Finish Deployment");
	case EFMCodexLocalMatchInteractionCategory::SelectCarrier: return TEXT("Select Carrier");
	case EFMCodexLocalMatchInteractionCategory::SelectMarker: return TEXT("Select Marker");
	case EFMCodexLocalMatchInteractionCategory::SelectSkill: return TEXT("Select Skill");
	case EFMCodexLocalMatchInteractionCategory::SelectRunner: return TEXT("Select Runner");
	case EFMCodexLocalMatchInteractionCategory::SelectHelper: return TEXT("Select Helper");
	case EFMCodexLocalMatchInteractionCategory::SelectBranchIntent: return TEXT("Select Branch Intent");
	case EFMCodexLocalMatchInteractionCategory::SelectLongShotBranch: return TEXT("选择远射方式");
	case EFMCodexLocalMatchInteractionCategory::SelectOneOnOneShot: return TEXT("Select One-on-One Shot");
	case EFMCodexLocalMatchInteractionCategory::RollCrossAttack: return TEXT("进攻方掷点");
	case EFMCodexLocalMatchInteractionCategory::RollCrossDefense: return TEXT("防守方掷点");
	case EFMCodexLocalMatchInteractionCategory::RollLongShotDirectAttack: return TEXT("进攻方掷远射点数");
	case EFMCodexLocalMatchInteractionCategory::RollLongShotDirectDefense: return TEXT("防守方掷防守点数");
	case EFMCodexLocalMatchInteractionCategory::RollLongShotDeadCorner: return TEXT("进攻方掷远射双骰");
	case EFMCodexLocalMatchInteractionCategory::RollCutInsideShotDirectAttack: return TEXT("进攻方掷内切射门点数");
	case EFMCodexLocalMatchInteractionCategory::RollCutInsideShotDirectDefense: return TEXT("防守方掷防守点数");
	case EFMCodexLocalMatchInteractionCategory::RollCutInsideShotDeadCorner: return TEXT("进攻方掷内切死角双骰");
	case EFMCodexLocalMatchInteractionCategory::RollThroughBallInitialRoute: return TEXT("判定直塞路线");
	case EFMCodexLocalMatchInteractionCategory::RollPassControlRoute: return TEXT("判定控球推进路线");
	case EFMCodexLocalMatchInteractionCategory::RollCrossRoute: return TEXT("判定传中路线");
	case EFMCodexLocalMatchInteractionCategory::RollPassControlAttack: return TEXT("进攻方掷点");
	case EFMCodexLocalMatchInteractionCategory::RollPassControlDefense: return TEXT("防守方掷点");
	case EFMCodexLocalMatchInteractionCategory::CompleteCrossAndAdvance: return TEXT("下一回合");
	case EFMCodexLocalMatchInteractionCategory::RollThroughBallFeetAttack: return TEXT("掷进攻方点数");
	case EFMCodexLocalMatchInteractionCategory::RollThroughBallFeetDefense: return TEXT("掷防守方点数");
	case EFMCodexLocalMatchInteractionCategory::RollThroughBallAntiOffsideAttack: return TEXT("进攻方掷反越位点数");
	case EFMCodexLocalMatchInteractionCategory::RollThroughBallOneOnOneChipShotAttack: return TEXT("进攻方掷挑射点数");
	case EFMCodexLocalMatchInteractionCategory::RollThroughBallOneOnOneDirectShotAttack: return TEXT("进攻方掷单刀射门点数");
	case EFMCodexLocalMatchInteractionCategory::RollThroughBallOneOnOneDirectShotDefense: return TEXT("防守方掷单刀防守点数");
	case EFMCodexLocalMatchInteractionCategory::RollThroughBallBehindDefenseAttack: return TEXT("进攻方掷点");
	case EFMCodexLocalMatchInteractionCategory::RollThroughBallBehindDefenseDefense: return TEXT("防守方掷点");
	case EFMCodexLocalMatchInteractionCategory::CompleteThroughBallFeetAndAdvance: return TEXT("下一回合");
	case EFMCodexLocalMatchInteractionCategory::ApplyCrossTerminalResolution: return TEXT("确认传中结算");
	case EFMCodexLocalMatchInteractionCategory::ApplyThroughBallFeetTerminalResolution: return TEXT("确认直塞结算");
	case EFMCodexLocalMatchInteractionCategory::AdvanceAfterTerminal: return TEXT("下一回合");
	case EFMCodexLocalMatchInteractionCategory::ContinueResolution: return TEXT("继续结算");
	case EFMCodexLocalMatchInteractionCategory::AttackComplete: return TEXT("Attack Complete");
	case EFMCodexLocalMatchInteractionCategory::MatchEnded: return TEXT("Match Ended");
	default: return TEXT("Unknown");
	}
}

FString FFMCodexLocalMatchInteractionViewBuilder::ToString(
	const EInitialTurnOrderPlayer Player)
{
	switch (Player)
	{
	case EInitialTurnOrderPlayer::PlayerA: return TEXT("Player A");
	case EInitialTurnOrderPlayer::PlayerB: return TEXT("Player B");
	default: return TEXT("None");
	}
}

FString FFMCodexLocalMatchInteractionViewBuilder::ToString(
	const EMatchResultType Result)
{
	switch (Result)
	{
	case EMatchResultType::HomeWin: return TEXT("Player A Win");
	case EMatchResultType::AwayWin: return TEXT("Player B Win");
	case EMatchResultType::Draw: return TEXT("Draw");
	default: return TEXT("Not Ended");
	}
}

FString FFMCodexLocalMatchInteractionViewBuilder::ToString(
	const EPlayerPositionType Position)
{
	switch (Position)
	{
	case EPlayerPositionType::Attack: return TEXT("Forward");
	case EPlayerPositionType::Midfield: return TEXT("Midfielder");
	case EPlayerPositionType::Defense: return TEXT("Defender");
	case EPlayerPositionType::Goalkeeper: return TEXT("Goalkeeper");
	default: return TEXT("Unknown Role");
	}
}

FString FFMCodexLocalMatchInteractionViewBuilder::ToString(
	const EMatchPlayNeutralSlotSide Side)
{
	switch (Side)
	{
	case EMatchPlayNeutralSlotSide::NearPlayerA:
		return TEXT("Half Near Player A");
	case EMatchPlayNeutralSlotSide::NearPlayerB:
		return TEXT("Half Near Player B");
	default:
		return TEXT("Unknown Field Region");
	}
}

FString FFMCodexLocalMatchInteractionViewBuilder::ToString(
	const EMatchPlayRelativeDeploymentZone Zone)
{
	switch (Zone)
	{
	case EMatchPlayRelativeDeploymentZone::Forward: return TEXT("Forward");
	case EMatchPlayRelativeDeploymentZone::Midfield: return TEXT("Midfield");
	case EMatchPlayRelativeDeploymentZone::Backfield: return TEXT("Backfield");
	default: return TEXT("Unknown Zone");
	}
}

FString FFMCodexLocalMatchInteractionViewBuilder::ToString(
	const ESkillRuleType SkillType)
{
	switch (SkillType)
	{
	case ESkillRuleType::LongShot: return TEXT("Long Shot");
	case ESkillRuleType::CutInsideShot: return TEXT("Cut Inside");
	case ESkillRuleType::PassControl: return TEXT("Pass Control");
	case ESkillRuleType::Cross: return TEXT("Cross");
	case ESkillRuleType::ThroughBall: return TEXT("Through Ball");
	default: return TEXT("Unknown Skill");
	}
}

FString FFMCodexLocalMatchInteractionViewBuilder::ToString(
	const FMatchPlayCurrentAttackActualBranch& ActualBranch)
{
	switch (ActualBranch.ActionType)
	{
	case ESkillRuleType::LongShot:
		return ActualBranch.LongShot == EMatchPlayLongShotActualBranch::DirectShot
			? TEXT("Direct Shot")
			: ActualBranch.LongShot == EMatchPlayLongShotActualBranch::DeadCorner
				? TEXT("Dead Corner") : TEXT("Unknown Route");
	case ESkillRuleType::CutInsideShot:
		return ActualBranch.CutInsideShot
			== EMatchPlayCutInsideShotActualBranch::DirectShot
				? TEXT("Direct Shot")
				: ActualBranch.CutInsideShot
					== EMatchPlayCutInsideShotActualBranch::DeadCorner
						? TEXT("Dead Corner") : TEXT("Unknown Route");
	case ESkillRuleType::Cross:
		return ActualBranch.Cross == EMatchPlayCrossActualBranch::High
			? TEXT("High Cross")
			: ActualBranch.Cross == EMatchPlayCrossActualBranch::Low
				? TEXT("Low Cross") : TEXT("Unknown Route");
	case ESkillRuleType::PassControl:
		switch (ActualBranch.PassControl)
		{
		case EMatchPlayPassControlActualBranch::PassAdvance:
			return TEXT("Pass Advance");
		case EMatchPlayPassControlActualBranch::DribbleAdvance:
			return TEXT("Dribble Advance");
		case EMatchPlayPassControlActualBranch::RunAdvance:
			return TEXT("Run Advance");
		default:
			return TEXT("Unknown Route");
		}
	case ESkillRuleType::ThroughBall:
		switch (ActualBranch.ThroughBall)
		{
		case EMatchPlayThroughBallActualBranch::Feet:
			return TEXT("To Feet");
		case EMatchPlayThroughBallActualBranch::BehindDefense:
			return TEXT("Behind Defense");
		case EMatchPlayThroughBallActualBranch::AntiOffside:
			return TEXT("Anti-Offside");
		default:
			return TEXT("Unknown Route");
		}
	default:
		return TEXT("Unknown Route");
	}
}

FString FFMCodexLocalMatchInteractionViewBuilder::ToString(
	const EMatchPlayThroughBallOneOnOneShotChoice Choice)
{
	switch (Choice)
	{
	case EMatchPlayThroughBallOneOnOneShotChoice::ChipShot:
		return TEXT("Chip Shot");
	case EMatchPlayThroughBallOneOnOneShotChoice::DirectShot:
		return TEXT("Direct Shot");
	default:
		return TEXT("No One-on-One Choice");
	}
}
