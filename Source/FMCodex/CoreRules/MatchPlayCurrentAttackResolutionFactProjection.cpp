#include "MatchPlayCurrentAttackResolutionFactProjection.h"

#include "MatchPlayCardSnapshotAuthority.h"
#include "MatchPlayCurrentAttackResolutionSessionStateValidator.h"
#include "MatchPlayCurrentAttackResolveSingleCardFinishingFormulaOrchestrator.h"
#include "MatchPlayCurrentAttackResolveThroughBallBehindDefenseP1FormulaOrchestrator.h"
#include "MatchPlayCurrentAttackResolveThroughBallFeetFormulaOrchestrator.h"
#include "MatchPlayCurrentAttackResolveThroughBallOneOnOneDirectShotFormulaOrchestrator.h"

namespace MatchPlayCurrentAttackResolutionFactProjection
{
	using EAttribute = EMatchPlayResolutionFormulaAttribute;
	using EApplication = EMatchPlayResolutionFormulaApplication;
	using EOutcome = EMatchPlayResolutionDecisionOutcome;
	using EParticipantRole = EMatchPlayResolutionParticipantRole;
	using EPostPurpose = EMatchPlayCurrentAttackPostRouteRollPurpose;
	using ERollSemantics = EMatchPlayResolutionRollSemantics;
	using ETermKind = EMatchPlayResolutionFormulaTermKind;
	using FContest = FMatchPlayResolutionFormulaContestFact;
	using FParticipant = FMatchPlayResolutionParticipantFact;
	using FProjection = FMatchPlayCurrentAttackResolutionFactProjection;
	using FRow = FMatchPlayResolutionFormulaRowFact;
	using FSession = FMatchPlayCurrentAttackResolutionSession;
	using FSessionParticipant =
		FMatchPlayCurrentAttackResolutionSessionParticipant;

	struct FGoalkeeperFact
	{
		bool bAvailable = false;
		EInitialTurnOrderPlayer Side = EInitialTurnOrderPlayer::None;
		FName CardId = NAME_None;
		FPlayerCardRuleSnapshot Snapshot;
	};

	void Fail(FProjection& Projection, const FString& Message)
	{
		Projection.bSuccess = false;
		Projection.ErrorMessage = Message;
	}

	FName PostPurposeId(const EPostPurpose Purpose)
	{
		switch (Purpose)
		{
		case EPostPurpose::PrimaryAttack: return TEXT("PrimaryAttackD6");
		case EPostPurpose::PrimaryDefense: return TEXT("PrimaryDefenseD6");
		case EPostPurpose::PairedAttackA: return TEXT("PairedAttackD6A");
		case EPostPurpose::PairedAttackB: return TEXT("PairedAttackD6B");
		case EPostPurpose::BehindDefenseP2Defense:
			return TEXT("BehindDefenseP2DefenseD6");
		case EPostPurpose::OneOnOneChipShotAttack:
			return TEXT("OneOnOneChipShotAttackD6");
		case EPostPurpose::OneOnOneDirectShotAttack:
			return TEXT("OneOnOneDirectShotAttackD6");
		case EPostPurpose::OneOnOneDirectShotDefense:
			return TEXT("OneOnOneDirectShotDefenseD6");
		default: return NAME_None;
		}
	}

	bool IsAttackingPurpose(const EPostPurpose Purpose)
	{
		return Purpose == EPostPurpose::PrimaryAttack
			|| Purpose == EPostPurpose::PairedAttackA
			|| Purpose == EPostPurpose::PairedAttackB
			|| Purpose == EPostPurpose::OneOnOneChipShotAttack
			|| Purpose == EPostPurpose::OneOnOneDirectShotAttack;
	}

	const FMatchPlayCurrentAttackPostRouteRollRecord* FindPostRecord(
		const FSession& Session,
		const EPostPurpose Purpose)
	{
		return Session.PostRouteRollProgress.RollRecords.FindByPredicate(
			[Purpose](const FMatchPlayCurrentAttackPostRouteRollRecord& Record)
			{
				return Record.Purpose == Purpose;
			});
	}

	int32 FindRollIndex(
		const FProjection& Projection,
		const EPostPurpose Purpose)
	{
		for (const FMatchPlayResolutionRollFact& Roll : Projection.Rolls)
		{
			if (!Roll.bInitialRoute && Roll.PostRoutePurpose == Purpose)
			{
				return Roll.SequenceIndex;
			}
		}
		return INDEX_NONE;
	}

	const FMatchPlayResolutionRollFact* FindRoll(
		const FProjection& Projection,
		const EPostPurpose Purpose)
	{
		return Projection.Rolls.FindByPredicate(
			[Purpose](const FMatchPlayResolutionRollFact& Roll)
			{
				return !Roll.bInitialRoute
					&& Roll.PostRoutePurpose == Purpose;
			});
	}

	void AddParticipant(
		FProjection& Projection,
		const EParticipantRole Role,
		const FSessionParticipant& Source)
	{
		if (Source.bIsPresent)
		{
			Projection.Participants.Add({ Role, Source.Side, Source.CardId });
		}
	}

	void AddGoalkeeperParticipant(
		FProjection& Projection,
		const FGoalkeeperFact& Goalkeeper)
	{
		if (!Goalkeeper.bAvailable)
		{
			return;
		}
		const bool bAlreadyPresent = Projection.Participants.ContainsByPredicate(
			[&Goalkeeper](const FParticipant& Participant)
			{
				return Participant.Role == EParticipantRole::Goalkeeper
					&& Participant.Side == Goalkeeper.Side
					&& Participant.CardId == Goalkeeper.CardId;
			});
		if (!bAlreadyPresent)
		{
			Projection.Participants.Add({
				EParticipantRole::Goalkeeper,
				Goalkeeper.Side,
				Goalkeeper.CardId });
		}
	}

	bool QueryGoalkeeper(
		const FMatchPlayState& State,
		const EInitialTurnOrderPlayer DefendingSide,
		FGoalkeeperFact& OutGoalkeeper,
		FString& OutError)
	{
		OutGoalkeeper.Side = DefendingSide;
		const FString& GoalkeeperId = DefendingSide
			== EInitialTurnOrderPlayer::PlayerA
				? State.RuntimeState.PlayerAState.GoalkeeperCardId
				: State.RuntimeState.PlayerBState.GoalkeeperCardId;
		OutGoalkeeper.CardId = FName(*GoalkeeperId);
		if (OutGoalkeeper.CardId.IsNone())
		{
			OutError = TEXT("Resolution facts require the authoritative defending goalkeeper CardId.");
			return false;
		}
		const FMatchPlayCardSnapshotAuthorityQueryResult Query =
			FMatchPlayCardSnapshotAuthorityQuery::FindByPlayerSideAndCardId(
				State.CardSnapshotAuthority,
				DefendingSide,
				OutGoalkeeper.CardId);
		if (!Query.bSuccess)
		{
			OutError = Query.ErrorMessage;
			return false;
		}
		if (!Query.Snapshot.bIsGoalkeeper
			|| !Query.Snapshot.bHasGoalkeeperAttributes)
		{
			OutError = TEXT("Resolution facts require a canonical goalkeeper snapshot.");
			return false;
		}
		OutGoalkeeper.Snapshot = Query.Snapshot;
		OutGoalkeeper.bAvailable = true;
		return true;
	}

	bool RequiresInitialRouteRoll(const ESkillRuleType ActionType)
	{
		return ActionType == ESkillRuleType::Cross
			|| ActionType == ESkillRuleType::PassControl
			|| ActionType == ESkillRuleType::ThroughBall;
	}

	bool InferIntentDeterminedBranch(
		const FSession& Session,
		FMatchPlayCurrentAttackActualBranch& OutBranch)
	{
		const ESkillRuleType ActionType = Session.Bundle.Binding.ActionType;
		const EMatchPlayElectiveBranchIntent Intent =
			Session.Bundle.Binding.ElectiveBranchIntent;
		OutBranch.ActionType = ActionType;
		if (ActionType == ESkillRuleType::LongShot)
		{
			if (Intent == EMatchPlayElectiveBranchIntent::DirectShot)
			{
				OutBranch.LongShot = EMatchPlayLongShotActualBranch::DirectShot;
				return true;
			}
			if (Intent == EMatchPlayElectiveBranchIntent::DeadCorner)
			{
				OutBranch.LongShot = EMatchPlayLongShotActualBranch::DeadCorner;
				return true;
			}
		}
		if (ActionType == ESkillRuleType::CutInsideShot)
		{
			if (Intent == EMatchPlayElectiveBranchIntent::DirectShot)
			{
				OutBranch.CutInsideShot =
					EMatchPlayCutInsideShotActualBranch::DirectShot;
				return true;
			}
			if (Intent == EMatchPlayElectiveBranchIntent::DeadCorner)
			{
				OutBranch.CutInsideShot =
					EMatchPlayCutInsideShotActualBranch::DeadCorner;
				return true;
			}
		}
		return false;
	}

	void AddInitialRouteRoll(
		FProjection& Projection,
		const FSession& Session)
	{
		FMatchPlayResolutionRollFact Roll;
		Roll.SequenceIndex = Projection.Rolls.Num();
		Roll.OperandId = TEXT("InitialRouteD6");
		Roll.bInitialRoute = true;
		Roll.InitialPurpose =
			EMatchPlayCurrentAttackResolutionRollPurpose::InitialRoute;
		Roll.Semantics = ERollSemantics::BranchSelection;
		Roll.OwningSide = Session.Bundle.CurrentAttackingPlayer;
		if (!Session.InitialRouteRollRecords.IsEmpty())
		{
			Roll.bResolved = true;
			Roll.RawD6 = Session.InitialRouteRollRecords[0].RawD6;
		}
		Projection.Rolls.Add(Roll);
	}

	void AddPostRouteRoll(
		FProjection& Projection,
		const FSession& Session,
		const EPostPurpose Purpose,
		const ERollSemantics Semantics,
		const bool bConditionallyRequired = false)
	{
		if (FindRollIndex(Projection, Purpose) != INDEX_NONE)
		{
			return;
		}
		FMatchPlayResolutionRollFact Roll;
		Roll.SequenceIndex = Projection.Rolls.Num();
		Roll.OperandId = PostPurposeId(Purpose);
		Roll.PostRoutePurpose = Purpose;
		Roll.Semantics = Semantics;
		Roll.OwningSide = IsAttackingPurpose(Purpose)
			? Session.Bundle.CurrentAttackingPlayer
			: Session.Bundle.CurrentDefendingPlayer;
		Roll.bConditionallyRequired = bConditionallyRequired;
		if (const FMatchPlayCurrentAttackPostRouteRollRecord* Record =
			FindPostRecord(Session, Purpose))
		{
			Roll.bResolved = true;
			Roll.RawD6 = Record->RawD6;
		}
		Projection.Rolls.Add(Roll);
	}

	void AddAttributeTerm(
		FRow& Row,
		const FName TermId,
		const EParticipantRole Role,
		const EInitialTurnOrderPlayer Side,
		const FName CardId,
		const EAttribute Attribute,
		const int32 Value,
		const float Multiplier = 1.0f,
		const ETermKind Kind = ETermKind::Attribute)
	{
		FMatchPlayResolutionFormulaTermFact Term;
		Term.TermId = TermId;
		Term.Kind = Kind;
		Term.ParticipantRole = Role;
		Term.Side = Side;
		Term.CardId = CardId;
		Term.Attribute = Attribute;
		Term.SourceValue = static_cast<float>(Value);
		Term.Multiplier = Multiplier;
		Term.Contribution = UFormulaResolver::RoundToOneDecimal(
			Term.SourceValue * Multiplier);
		Row.Terms.Add(Term);
	}

	void AddFixedTerm(FRow& Row, const FName TermId, const float Value)
	{
		FMatchPlayResolutionFormulaTermFact Term;
		Term.TermId = TermId;
		Term.Kind = ETermKind::FixedModifier;
		Term.SourceValue = Value;
		Term.Contribution = Value;
		Row.Terms.Add(Term);
	}

	void AddRollTerm(
		FRow& Row,
		const FProjection& Projection,
		const EPostPurpose Purpose)
	{
		FMatchPlayResolutionFormulaTermFact Term;
		Term.TermId = PostPurposeId(Purpose);
		Term.Kind = ETermKind::RawRoll;
		Term.RollSequenceIndex = FindRollIndex(Projection, Purpose);
		if (const FMatchPlayResolutionRollFact* Roll =
			FindRoll(Projection, Purpose))
		{
			Term.Side = Roll->OwningSide;
			Term.bResolved = Roll->bResolved;
			Term.SourceValue = Roll->bResolved
				? static_cast<float>(Roll->RawD6) : 0.0f;
			Term.Contribution = Term.SourceValue;
		}
		else
		{
			Term.bResolved = false;
		}
		Row.Terms.Add(Term);
	}

	bool AreAllTermsResolved(const FRow& Row)
	{
		return !Row.Terms.ContainsByPredicate(
			[](const FMatchPlayResolutionFormulaTermFact& Term)
			{
				return !Term.bResolved;
			});
	}

	float SumTerms(const FRow& Row)
	{
		float Sum = 0.0f;
		for (const FMatchPlayResolutionFormulaTermFact& Term : Row.Terms)
		{
			Sum += Term.Contribution;
		}
		return UFormulaResolver::RoundToOneDecimal(Sum);
	}

	void ProjectKnownRowValues(FRow& Row)
	{
		float KnownSubtotal = 0.0f;
		bool bAllNonRollTermsResolved = true;
		bool bAllRollTermsResolved = true;
		for (const FMatchPlayResolutionFormulaTermFact& Term : Row.Terms)
		{
			if (Term.Kind == ETermKind::RawRoll)
			{
				bAllRollTermsResolved &= Term.bResolved;
				continue;
			}
			bAllNonRollTermsResolved &= Term.bResolved;
			if (Term.bResolved)
			{
				KnownSubtotal += Term.Contribution;
			}
		}
		Row.bKnownNonRollSubtotalResolved = bAllNonRollTermsResolved;
		Row.KnownNonRollSubtotal = bAllNonRollTermsResolved
			? UFormulaResolver::RoundToOneDecimal(KnownSubtotal)
			: 0.0f;
		if (bAllNonRollTermsResolved && bAllRollTermsResolved)
		{
			Row.bFinalValueResolved = true;
			Row.FinalValue = SumTerms(Row);
		}
	}

	void InitializeContest(
		FContest& Contest,
		const FName ContestId,
		const EFormulaType FormulaType,
		const FSession& Session)
	{
		Contest.ContestId = ContestId;
		Contest.FormulaType = FormulaType;
		Contest.Application = EApplication::Pending;
		Contest.AttackRow.RowId = FName(*FString::Printf(
			TEXT("%s.Attack"), *ContestId.ToString()));
		Contest.AttackRow.Side = Session.Bundle.CurrentAttackingPlayer;
		Contest.DefenseRow.RowId = FName(*FString::Printf(
			TEXT("%s.Defense"), *ContestId.ToString()));
		Contest.DefenseRow.Side = Session.Bundle.CurrentDefendingPlayer;
	}

	bool IsDirectShotBranch(const FMatchPlayCurrentAttackActualBranch& Branch)
	{
		return (Branch.ActionType == ESkillRuleType::LongShot
				&& Branch.LongShot
					== EMatchPlayLongShotActualBranch::DirectShot)
			|| (Branch.ActionType == ESkillRuleType::CutInsideShot
				&& Branch.CutInsideShot
					== EMatchPlayCutInsideShotActualBranch::DirectShot);
	}

	bool IsDeadCornerBranch(const FMatchPlayCurrentAttackActualBranch& Branch)
	{
		return (Branch.ActionType == ESkillRuleType::LongShot
				&& Branch.LongShot
					== EMatchPlayLongShotActualBranch::DeadCorner)
			|| (Branch.ActionType == ESkillRuleType::CutInsideShot
				&& Branch.CutInsideShot
					== EMatchPlayCutInsideShotActualBranch::DeadCorner);
	}

	FName PrimaryContestId(const FMatchPlayCurrentAttackActualBranch& Branch)
	{
		if (Branch.ActionType == ESkillRuleType::LongShot)
		{
			return TEXT("LongShot.DirectShot");
		}
		if (Branch.ActionType == ESkillRuleType::CutInsideShot)
		{
			return TEXT("CutInsideShot.DirectShot");
		}
		if (Branch.ActionType == ESkillRuleType::Cross)
		{
			return Branch.Cross == EMatchPlayCrossActualBranch::High
				? FName(TEXT("Cross.High")) : FName(TEXT("Cross.Low"));
		}
		if (Branch.ActionType == ESkillRuleType::PassControl)
		{
			switch (Branch.PassControl)
			{
			case EMatchPlayPassControlActualBranch::PassAdvance:
				return TEXT("PassControl.PassAdvance");
			case EMatchPlayPassControlActualBranch::DribbleAdvance:
				return TEXT("PassControl.DribbleAdvance");
			case EMatchPlayPassControlActualBranch::RunAdvance:
				return TEXT("PassControl.RunAdvance");
			default: return NAME_None;
			}
		}
		if (Branch.ActionType == ESkillRuleType::ThroughBall)
		{
			return Branch.ThroughBall == EMatchPlayThroughBallActualBranch::Feet
				? FName(TEXT("ThroughBall.Feet"))
				: Branch.ThroughBall
					== EMatchPlayThroughBallActualBranch::BehindDefense
						? FName(TEXT("ThroughBall.BehindDefense.P1"))
						: NAME_None;
		}
		return NAME_None;
	}

	void AddDirectShotTerms(
		FContest& Contest,
		FProjection& Projection,
		const FMatchPlayState& State,
		const FMatchPlayCurrentAttackActualBranch& Branch,
		FGoalkeeperFact& Goalkeeper,
		FString& OutError)
	{
		const FSession& Session = State.CurrentAttack.ResolutionSession;
		const auto& Bundle = Session.Bundle;
		const bool bLongShot = Branch.ActionType == ESkillRuleType::LongShot;
		if (bLongShot)
		{
			AddAttributeTerm(Contest.AttackRow, TEXT("Carrier.LongShot"),
				EParticipantRole::Carrier, Bundle.Carrier.Side,
				Bundle.Carrier.CardId, EAttribute::LongShot,
				Bundle.Carrier.Values.LongShot);
		}
		else
		{
			AddAttributeTerm(Contest.AttackRow, TEXT("Carrier.ShootingHalf"),
				EParticipantRole::Carrier, Bundle.Carrier.Side,
				Bundle.Carrier.CardId, EAttribute::Shooting,
				Bundle.Carrier.Values.Shooting, 0.5f);
			AddAttributeTerm(Contest.AttackRow, TEXT("Carrier.DribblingHalf"),
				EParticipantRole::Carrier, Bundle.Carrier.Side,
				Bundle.Carrier.CardId, EAttribute::Dribbling,
				Bundle.Carrier.Values.Dribbling, 0.5f);
		}
		AddRollTerm(Contest.AttackRow, Projection, EPostPurpose::PrimaryAttack);
		AddAttributeTerm(Contest.DefenseRow, TEXT("Marker.Tackling"),
			EParticipantRole::Marker, Bundle.Marker.Side,
			Bundle.Marker.CardId, EAttribute::Tackling,
			Bundle.Marker.Values.Tackling);
		AddRollTerm(Contest.DefenseRow, Projection, EPostPurpose::PrimaryDefense);
		AddFixedTerm(Contest.DefenseRow, TEXT("Defense.FixedBonus"), 2.0f);
		Contest.AttackRow.ParticipatingStamina = {
			Bundle.Carrier.Values.Stamina };
		Contest.DefenseRow.ParticipatingStamina = {
			Bundle.Marker.Values.Stamina };

		if (State.CurrentAttack.bCurrentDefenseGoalkeeperActivated)
		{
			if (!QueryGoalkeeper(State, Bundle.CurrentDefendingPlayer,
				Goalkeeper, OutError))
			{
				return;
			}
			AddGoalkeeperParticipant(Projection, Goalkeeper);
			const int32 AttributeValue = bLongShot
				? Goalkeeper.Snapshot.GoalkeeperAttributes.Positioning
				: Goalkeeper.Snapshot.GoalkeeperAttributes.Handling;
			AddAttributeTerm(Contest.DefenseRow,
				bLongShot ? FName(TEXT("Goalkeeper.PositioningHalf"))
					: FName(TEXT("Goalkeeper.HandlingHalf")),
				EParticipantRole::Goalkeeper,
				Goalkeeper.Side,
				Goalkeeper.CardId,
				bLongShot ? EAttribute::GoalkeeperPositioning
					: EAttribute::GoalkeeperHandling,
				AttributeValue,
				0.5f,
				ETermKind::GoalkeeperContribution);
			Contest.bGoalkeeperParticipated = true;
			Contest.TieRule =
				EMatchPlayResolutionTieRule::GoalkeeperDefenderWins;
		}
	}

	void AddCompositePrimaryTerms(
		FContest& Contest,
		FProjection& Projection,
		const FMatchPlayState& State,
		const FMatchPlayCurrentAttackActualBranch& Branch,
		FGoalkeeperFact& Goalkeeper,
		FString& OutError)
	{
		const FSession& Session = State.CurrentAttack.ResolutionSession;
		const auto& Bundle = Session.Bundle;
		EAttribute CarrierAttackAttribute = EAttribute::None;
		EAttribute RunnerAttackAttribute = EAttribute::None;
		EAttribute MarkerDefenseAttribute = EAttribute::None;
		EAttribute HelperDefenseAttribute = EAttribute::None;
		int32 CarrierAttackValue = 0;
		int32 RunnerAttackValue = 0;
		int32 MarkerDefenseValue = 0;
		int32 HelperDefenseValue = 0;
		float DefenseFixed = 2.0f;
		bool bCompositeStamina = false;
		EAttribute GoalkeeperAttribute = EAttribute::None;
		int32 GoalkeeperValue = 0;

		if (Branch.ActionType == ESkillRuleType::Cross)
		{
			CarrierAttackAttribute = EAttribute::Passing;
			CarrierAttackValue = Bundle.Carrier.Values.Passing;
			MarkerDefenseAttribute = EAttribute::Tackling;
			MarkerDefenseValue = Bundle.Marker.Values.Tackling;
			if (Branch.Cross == EMatchPlayCrossActualBranch::High)
			{
				RunnerAttackAttribute = EAttribute::Strength;
				RunnerAttackValue = Bundle.Runner.Values.Strength;
				HelperDefenseAttribute = EAttribute::Strength;
				HelperDefenseValue = Bundle.Helper.Values.Strength;
				GoalkeeperAttribute = EAttribute::GoalkeeperAerial;
			}
			else
			{
				RunnerAttackAttribute = EAttribute::Shooting;
				RunnerAttackValue = Bundle.Runner.Values.Shooting;
				HelperDefenseAttribute = EAttribute::Marking;
				HelperDefenseValue = Bundle.Helper.Values.Marking;
				GoalkeeperAttribute = EAttribute::GoalkeeperReflex;
			}
		}
		else if (Branch.ActionType == ESkillRuleType::PassControl)
		{
			RunnerAttackAttribute = Branch.PassControl
				== EMatchPlayPassControlActualBranch::RunAdvance
					? EAttribute::Dribbling : EAttribute::Passing;
			RunnerAttackValue = Branch.PassControl
				== EMatchPlayPassControlActualBranch::RunAdvance
					? Bundle.Runner.Values.Dribbling
					: Bundle.Runner.Values.Passing;
			CarrierAttackAttribute = Branch.PassControl
				== EMatchPlayPassControlActualBranch::PassAdvance
					? EAttribute::Passing
					: Branch.PassControl
						== EMatchPlayPassControlActualBranch::DribbleAdvance
							? EAttribute::Dribbling : EAttribute::OffBall;
			CarrierAttackValue = Branch.PassControl
				== EMatchPlayPassControlActualBranch::PassAdvance
					? Bundle.Carrier.Values.Passing
					: Branch.PassControl
						== EMatchPlayPassControlActualBranch::DribbleAdvance
							? Bundle.Carrier.Values.Dribbling
							: Bundle.Carrier.Values.OffBall;
			MarkerDefenseAttribute = Branch.PassControl
				== EMatchPlayPassControlActualBranch::RunAdvance
					? EAttribute::Marking : EAttribute::Tackling;
			MarkerDefenseValue = Branch.PassControl
				== EMatchPlayPassControlActualBranch::RunAdvance
					? Bundle.Marker.Values.Marking
					: Bundle.Marker.Values.Tackling;
			HelperDefenseAttribute = EAttribute::Marking;
			HelperDefenseValue = Bundle.Helper.Values.Marking;
			GoalkeeperAttribute = EAttribute::GoalkeeperHandling;
		}
		else if (Branch.ActionType == ESkillRuleType::ThroughBall
			&& Branch.ThroughBall == EMatchPlayThroughBallActualBranch::Feet)
		{
			CarrierAttackAttribute = EAttribute::Passing;
			CarrierAttackValue = Bundle.Carrier.Values.Passing;
			RunnerAttackAttribute = EAttribute::OffBall;
			RunnerAttackValue = Bundle.Runner.Values.OffBall;
			MarkerDefenseAttribute = EAttribute::Tackling;
			MarkerDefenseValue = Bundle.Marker.Values.Tackling;
			HelperDefenseAttribute = EAttribute::Marking;
			HelperDefenseValue = Bundle.Helper.Values.Marking;
			GoalkeeperAttribute = EAttribute::GoalkeeperOneOnOne;
			bCompositeStamina = true;
		}
		else
		{
			CarrierAttackAttribute = EAttribute::Passing;
			CarrierAttackValue = Bundle.Carrier.Values.Passing;
			RunnerAttackAttribute = EAttribute::Speed;
			RunnerAttackValue = Bundle.Runner.Values.Speed;
			MarkerDefenseAttribute = EAttribute::Marking;
			MarkerDefenseValue = Bundle.Marker.Values.Marking;
			HelperDefenseAttribute = EAttribute::Speed;
			HelperDefenseValue = Bundle.Helper.Values.Speed;
			DefenseFixed = 1.0f;
			bCompositeStamina = true;
		}

		AddAttributeTerm(Contest.AttackRow, TEXT("Carrier.PrimaryHalf"),
			EParticipantRole::Carrier, Bundle.Carrier.Side,
			Bundle.Carrier.CardId, CarrierAttackAttribute,
			CarrierAttackValue, 0.5f);
		AddAttributeTerm(Contest.AttackRow, TEXT("Runner.PrimaryHalf"),
			EParticipantRole::Runner, Bundle.Runner.Side,
			Bundle.Runner.CardId, RunnerAttackAttribute,
			RunnerAttackValue, 0.5f);
		AddRollTerm(Contest.AttackRow, Projection, EPostPurpose::PrimaryAttack);
		AddAttributeTerm(Contest.DefenseRow, TEXT("Marker.PrimaryHalf"),
			EParticipantRole::Marker, Bundle.Marker.Side,
			Bundle.Marker.CardId, MarkerDefenseAttribute,
			MarkerDefenseValue, 0.5f);
		if (Bundle.bHasHelper)
		{
			AddAttributeTerm(Contest.DefenseRow, TEXT("Helper.PrimaryHalf"),
				EParticipantRole::Helper, Bundle.Helper.Side,
				Bundle.Helper.CardId, HelperDefenseAttribute,
				HelperDefenseValue, 0.5f);
		}
		AddRollTerm(Contest.DefenseRow, Projection, EPostPurpose::PrimaryDefense);
		AddFixedTerm(Contest.DefenseRow, TEXT("Defense.FixedBonus"), DefenseFixed);

		if (bCompositeStamina)
		{
			Contest.AttackRow.ParticipatingStamina = {
				Bundle.Carrier.Values.Stamina,
				Bundle.Runner.Values.Stamina };
			Contest.DefenseRow.ParticipatingStamina = {
				Bundle.Marker.Values.Stamina };
			if (Bundle.bHasHelper)
			{
				Contest.DefenseRow.ParticipatingStamina.Add(
					Bundle.Helper.Values.Stamina);
			}
		}
		else
		{
			// Existing SingleCard assembly intentionally uses only its primary
			// Carrier/Marker snapshots for the no-GK stamina tie-break.
			Contest.AttackRow.ParticipatingStamina = {
				Bundle.Carrier.Values.Stamina };
			Contest.DefenseRow.ParticipatingStamina = {
				Bundle.Marker.Values.Stamina };
		}

		const bool bUsesActiveGoalkeeper =
			State.CurrentAttack.bCurrentDefenseGoalkeeperActivated
			&& GoalkeeperAttribute != EAttribute::None;
		if (bUsesActiveGoalkeeper)
		{
			if (!QueryGoalkeeper(State, Bundle.CurrentDefendingPlayer,
				Goalkeeper, OutError))
			{
				return;
			}
			AddGoalkeeperParticipant(Projection, Goalkeeper);
			switch (GoalkeeperAttribute)
			{
			case EAttribute::GoalkeeperAerial:
				GoalkeeperValue =
					Goalkeeper.Snapshot.GoalkeeperAttributes.Aerial;
				break;
			case EAttribute::GoalkeeperReflex:
				GoalkeeperValue =
					Goalkeeper.Snapshot.GoalkeeperAttributes.Reflex;
				break;
			case EAttribute::GoalkeeperHandling:
				GoalkeeperValue =
					Goalkeeper.Snapshot.GoalkeeperAttributes.Handling;
				break;
			case EAttribute::GoalkeeperOneOnOne:
				GoalkeeperValue =
					Goalkeeper.Snapshot.GoalkeeperAttributes.OneOnOne;
				break;
			default: break;
			}
			AddAttributeTerm(Contest.DefenseRow, TEXT("Goalkeeper.ActiveHalf"),
				EParticipantRole::Goalkeeper,
				Goalkeeper.Side,
				Goalkeeper.CardId,
				GoalkeeperAttribute,
				GoalkeeperValue,
				0.5f,
				ETermKind::GoalkeeperContribution);
			Contest.bGoalkeeperParticipated = true;
			Contest.TieRule =
				EMatchPlayResolutionTieRule::GoalkeeperDefenderWins;
			if (bCompositeStamina
				&& Branch.ActionType == ESkillRuleType::ThroughBall
				&& Branch.ThroughBall
					== EMatchPlayThroughBallActualBranch::Feet)
			{
				Contest.DefenseRow.ParticipatingStamina.Add(
					Goalkeeper.Snapshot.Attributes.Stamina);
			}
		}
	}

	bool MakePrimarySourceState(
		const FMatchPlayState& State,
		FMatchPlayState& OutState)
	{
		OutState = State;
		FSession& Session = OutState.CurrentAttack.ResolutionSession;
		TArray<FMatchPlayCurrentAttackPostRouteRollRecord> PrimaryRecords;
		for (const FMatchPlayCurrentAttackPostRouteRollRecord& Record
			: Session.PostRouteRollProgress.RollRecords)
		{
			if (Record.Purpose == EPostPurpose::PrimaryAttack
				|| Record.Purpose == EPostPurpose::PrimaryDefense)
			{
				PrimaryRecords.Add(Record);
			}
		}
		Session.PostRouteRollProgress.Phase =
			EMatchPlayCurrentAttackPostRouteRollPhase::PrimaryBranch;
		Session.PostRouteRollProgress.RollRecords = MoveTemp(PrimaryRecords);
		Session.ThroughBallOneOnOneShotChoice =
			EMatchPlayThroughBallOneOnOneShotChoice::None;
		return FMatchPlayCurrentAttackResolutionSessionStateValidator::Validate(
			OutState).bIsCanonical;
	}

	bool ResolvePrimaryFormula(
		const FMatchPlayState& State,
		const FSkillRuleSnapshotSet* SkillRuleSet,
		const FMatchPlayCurrentAttackActualBranch& Branch,
		FFormulaResolverInput& OutInput,
		FFormulaResolutionResult& OutResult,
		FString& OutError)
	{
		FMatchPlayState SourceState;
		if (!MakePrimarySourceState(State, SourceState))
		{
			OutError = TEXT("Resolution fact primary source snapshot is not canonical.");
			return false;
		}
		if (Branch.ActionType == ESkillRuleType::ThroughBall
			&& Branch.ThroughBall
				== EMatchPlayThroughBallActualBranch::Feet)
		{
			const auto Formula =
				FMatchPlayCurrentAttackResolveThroughBallFeetFormulaOrchestrator
					::Resolve(SourceState, SkillRuleSet);
			if (!Formula.bSuccess || !Formula.bHasFormulaResolution
				|| !Formula.ResolverInputAssemblyResult.bHasResolverInput)
			{
				OutError = Formula.ErrorMessage;
				return false;
			}
			OutInput = Formula.ResolverInputAssemblyResult.ResolverInput;
			OutResult = Formula.FormulaResolutionResult;
			return true;
		}
		if (Branch.ActionType == ESkillRuleType::ThroughBall
			&& Branch.ThroughBall
				== EMatchPlayThroughBallActualBranch::BehindDefense)
		{
			const auto Formula =
				FMatchPlayCurrentAttackResolveThroughBallBehindDefenseP1FormulaOrchestrator
					::Resolve(SourceState, SkillRuleSet);
			if (!Formula.bSuccess || !Formula.bHasFormulaResolution
				|| !Formula.ResolverInputAssemblyResult.bHasResolverInput)
			{
				OutError = Formula.ErrorMessage;
				return false;
			}
			OutInput = Formula.ResolverInputAssemblyResult.ResolverInput;
			OutResult = Formula.FormulaResolutionResult;
			return true;
		}
		const auto Formula =
			FMatchPlayCurrentAttackResolveSingleCardFinishingFormulaOrchestrator
				::Resolve(SourceState, SkillRuleSet);
		if (!Formula.bSuccess || !Formula.bHasFormulaResolution
			|| !Formula.ResolverInputAssemblyResult.bSuccess)
		{
			OutError = Formula.ErrorMessage;
			return false;
		}
		OutInput = Formula.ResolverInputAssemblyResult.ResolverInput;
		OutResult = Formula.FormulaResolutionResult;
		return true;
	}

	bool FinalizeResolvedContest(
		FContest& Contest,
		const FFormulaResolverInput& Input,
		const FFormulaResolutionResult& Result,
		FString& OutError)
	{
		const float ProjectedAttack = SumTerms(Contest.AttackRow);
		const float ProjectedDefense = SumTerms(Contest.DefenseRow);
		if (!FMath::IsNearlyEqual(ProjectedAttack, Result.AttackerFinalValue)
			|| !FMath::IsNearlyEqual(ProjectedDefense, Result.DefenderFinalValue))
		{
			OutError = FString::Printf(
				TEXT("Resolution Formula facts diverged from authoritative Final Value (%.1f/%.1f vs %.1f/%.1f)."),
				ProjectedAttack,
				ProjectedDefense,
				Result.AttackerFinalValue,
				Result.DefenderFinalValue);
			return false;
		}
		Contest.Application = EApplication::Applied;
		Contest.bGoalkeeperParticipated = Input.bGoalkeeperParticipated;
		Contest.TieRule = Input.bGoalkeeperParticipated
			? EMatchPlayResolutionTieRule::GoalkeeperDefenderWins
			: EMatchPlayResolutionTieRule::StaminaThenDefender;
		Contest.AttackRow.bFinalValueResolved = true;
		Contest.AttackRow.FinalValue = Result.AttackerFinalValue;
		Contest.DefenseRow.bFinalValueResolved = true;
		Contest.DefenseRow.FinalValue = Result.DefenderFinalValue;
		Contest.bHasResolvedFormula = true;
		Contest.ResolvedInput = Input;
		Contest.ResolvedResult = Result;
		return true;
	}

	void AddFormulaDecision(
		FProjection& Projection,
		const FContest& Contest)
	{
		FMatchPlayResolutionDecisionFact Decision;
		Decision.DecisionId = FName(*FString::Printf(
			TEXT("%s.Outcome"), *Contest.ContestId.ToString()));
		Decision.Semantics = ERollSemantics::ArithmeticContest;
		for (const FMatchPlayResolutionFormulaTermFact& Term
			: Contest.AttackRow.Terms)
		{
			if (Term.Kind == ETermKind::RawRoll)
			{
				Decision.RollSequenceIndices.Add(Term.RollSequenceIndex);
			}
		}
		for (const FMatchPlayResolutionFormulaTermFact& Term
			: Contest.DefenseRow.Terms)
		{
			if (Term.Kind == ETermKind::RawRoll)
			{
				Decision.RollSequenceIndices.Add(Term.RollSequenceIndex);
			}
		}
		if (Contest.Application == EApplication::SkippedByAuthoritativeGate)
		{
			Decision.bResolved = true;
			Decision.Outcome = Contest.ContestId
				== FName(TEXT("ThroughBall.BehindDefense.P1"))
					? EOutcome::OutOfPlay : EOutcome::ImmediateMiss;
		}
		else if (Contest.bHasResolvedFormula)
		{
			Decision.bResolved = true;
			if (Contest.FormulaType == EFormulaType::Transition)
			{
				Decision.Outcome = Contest.ResolvedResult.Winner
					== EFormulaWinner::Attacker
						? EOutcome::P2Required
						: EOutcome::DefenderStoppedAttack;
			}
			else
			{
				Decision.Outcome = Contest.ResolvedResult.bIsGoal
					? EOutcome::Goal : EOutcome::Miss;
			}
		}
		Projection.Decisions.Add(Decision);
	}

	bool AddPrimaryContest(
		FProjection& Projection,
		const FMatchPlayState& State,
		const FSkillRuleSnapshotSet* SkillRuleSet,
		const FMatchPlayCurrentAttackActualBranch& Branch,
		FString& OutError)
	{
		const FName ContestId = PrimaryContestId(Branch);
		if (ContestId.IsNone())
		{
			return true;
		}
		FContest Contest;
		InitializeContest(
			Contest,
			ContestId,
			Branch.ActionType == ESkillRuleType::ThroughBall
				&& Branch.ThroughBall
					== EMatchPlayThroughBallActualBranch::BehindDefense
						? EFormulaType::Transition : EFormulaType::Finishing,
			State.CurrentAttack.ResolutionSession);
		FGoalkeeperFact Goalkeeper;
		if (IsDirectShotBranch(Branch))
		{
			AddDirectShotTerms(
				Contest, Projection, State, Branch, Goalkeeper, OutError);
		}
		else
		{
			AddCompositePrimaryTerms(
				Contest, Projection, State, Branch, Goalkeeper, OutError);
		}
		if (!OutError.IsEmpty())
		{
			return false;
		}
		ProjectKnownRowValues(Contest.AttackRow);
		ProjectKnownRowValues(Contest.DefenseRow);

		const FMatchPlayResolutionRollFact* AttackRoll =
			FindRoll(Projection, EPostPurpose::PrimaryAttack);
		const bool bConditionalGate = IsDirectShotBranch(Branch)
			|| (Branch.ActionType == ESkillRuleType::ThroughBall
				&& Branch.ThroughBall
					== EMatchPlayThroughBallActualBranch::BehindDefense);
		if (bConditionalGate && AttackRoll != nullptr
			&& AttackRoll->bResolved && AttackRoll->RawD6 <= 2)
		{
			Contest.Application = EApplication::SkippedByAuthoritativeGate;
		}
		else if (AreAllTermsResolved(Contest.AttackRow)
			&& AreAllTermsResolved(Contest.DefenseRow))
		{
			FFormulaResolverInput ResolvedInput;
			FFormulaResolutionResult ResolvedResult;
			if (!ResolvePrimaryFormula(
				State,
				SkillRuleSet,
				Branch,
				ResolvedInput,
				ResolvedResult,
				OutError)
				|| !FinalizeResolvedContest(
					Contest, ResolvedInput, ResolvedResult, OutError))
			{
				return false;
			}
		}
		Projection.FormulaContests.Add(Contest);
		AddFormulaDecision(Projection, Contest);
		return true;
	}

	bool AddOneOnOneDirectShotContest(
		FProjection& Projection,
		const FMatchPlayState& State,
		const FSkillRuleSnapshotSet* SkillRuleSet,
		FString& OutError)
	{
		const FSession& Session = State.CurrentAttack.ResolutionSession;
		if (Session.ThroughBallOneOnOneShotChoice
			!= EMatchPlayThroughBallOneOnOneShotChoice::DirectShot)
		{
			return true;
		}
		FGoalkeeperFact Goalkeeper;
		if (!QueryGoalkeeper(State, Session.Bundle.CurrentDefendingPlayer,
			Goalkeeper, OutError))
		{
			return false;
		}
		AddGoalkeeperParticipant(Projection, Goalkeeper);
		FContest Contest;
		InitializeContest(
			Contest,
			TEXT("ThroughBall.OneOnOne.DirectShot"),
			EFormulaType::Finishing,
			Session);
		Contest.bGoalkeeperParticipated = true;
		Contest.TieRule =
			EMatchPlayResolutionTieRule::GoalkeeperDefenderWins;
		AddAttributeTerm(Contest.AttackRow, TEXT("Runner.Shooting"),
			EParticipantRole::Runner, Session.Bundle.Runner.Side,
			Session.Bundle.Runner.CardId, EAttribute::Shooting,
			Session.Bundle.Runner.Values.Shooting);
		AddRollTerm(Contest.AttackRow, Projection,
			EPostPurpose::OneOnOneDirectShotAttack);
		AddFixedTerm(Contest.AttackRow, TEXT("Attack.FixedBonus"), 1.0f);
		AddAttributeTerm(Contest.DefenseRow,
			TEXT("Goalkeeper.OneOnOneBase"),
			EParticipantRole::Goalkeeper,
			Goalkeeper.Side,
			Goalkeeper.CardId,
			EAttribute::GoalkeeperOneOnOne,
			Goalkeeper.Snapshot.GoalkeeperAttributes.OneOnOne);
		if (State.CurrentAttack.bCurrentDefenseGoalkeeperActivated)
		{
			AddAttributeTerm(Contest.DefenseRow,
				TEXT("Goalkeeper.OneOnOneActiveHalf"),
				EParticipantRole::Goalkeeper,
				Goalkeeper.Side,
				Goalkeeper.CardId,
				EAttribute::GoalkeeperOneOnOne,
				Goalkeeper.Snapshot.GoalkeeperAttributes.OneOnOne,
				0.5f,
				ETermKind::GoalkeeperContribution);
		}
		AddRollTerm(Contest.DefenseRow, Projection,
			EPostPurpose::OneOnOneDirectShotDefense);
		ProjectKnownRowValues(Contest.AttackRow);
		ProjectKnownRowValues(Contest.DefenseRow);

		if (AreAllTermsResolved(Contest.AttackRow)
			&& AreAllTermsResolved(Contest.DefenseRow))
		{
			const auto Formula =
				FMatchPlayCurrentAttackResolveThroughBallOneOnOneDirectShotFormulaOrchestrator
					::Resolve(State, SkillRuleSet);
			if (!Formula.bSuccess
				|| !Formula.DirectShotFormulaResult.bHasResolverInput
				|| !Formula.DirectShotFormulaResult.bHasFormulaResolution)
			{
				OutError = Formula.ErrorMessage;
				return false;
			}
			if (!FinalizeResolvedContest(
				Contest,
				Formula.DirectShotFormulaResult.ResolverInput,
				Formula.DirectShotFormulaResult.FormulaResolutionResult,
				OutError))
			{
				return false;
			}
		}
		Projection.FormulaContests.Add(Contest);
		AddFormulaDecision(Projection, Contest);
		return true;
	}

	void AddDecision(
		FProjection& Projection,
		const FName DecisionId,
		const ERollSemantics Semantics,
		const TArray<EPostPurpose>& Purposes,
		const bool bResolved,
		const EOutcome Outcome)
	{
		FMatchPlayResolutionDecisionFact Decision;
		Decision.DecisionId = DecisionId;
		Decision.Semantics = Semantics;
		for (const EPostPurpose Purpose : Purposes)
		{
			Decision.RollSequenceIndices.Add(FindRollIndex(Projection, Purpose));
		}
		Decision.bResolved = bResolved;
		Decision.Outcome = bResolved ? Outcome : EOutcome::None;
		Projection.Decisions.Add(Decision);
	}

	void AddNonFormulaDecisions(
		FProjection& Projection,
		const FSession& Session,
		const FMatchPlayCurrentAttackActualBranch& Branch)
	{
		if (IsDeadCornerBranch(Branch))
		{
			const auto* A = FindRoll(Projection, EPostPurpose::PairedAttackA);
			const auto* B = FindRoll(Projection, EPostPurpose::PairedAttackB);
			const bool bResolved = A != nullptr && A->bResolved
				&& B != nullptr && B->bResolved;
			AddDecision(
				Projection,
				TEXT("DeadCorner.Outcome"),
				ERollSemantics::OutcomeDecision,
				{ EPostPurpose::PairedAttackA, EPostPurpose::PairedAttackB },
				bResolved,
				bResolved && A->RawD6 + B->RawD6 >= 11
					? EOutcome::Goal : EOutcome::Miss);
		}
		if (Branch.ActionType != ESkillRuleType::ThroughBall)
		{
			return;
		}
		if (Branch.ThroughBall == EMatchPlayThroughBallActualBranch::AntiOffside)
		{
			const auto* Roll = FindRoll(Projection, EPostPurpose::PrimaryAttack);
			const bool bResolved = Roll != nullptr && Roll->bResolved;
			AddDecision(
				Projection,
				TEXT("ThroughBall.AntiOffside.Outcome"),
				ERollSemantics::OutcomeDecision,
				{ EPostPurpose::PrimaryAttack },
				bResolved,
				bResolved && Roll->RawD6 == 6
					? EOutcome::OneOnOneRequired : EOutcome::Offside);
		}
		const bool bHasP2 = Session.PostRouteRollProgress.Phase
			== EMatchPlayCurrentAttackPostRouteRollPhase::BehindDefenseP2
			|| Session.PostRouteRollProgress.Phase
				== EMatchPlayCurrentAttackPostRouteRollPhase::OneOnOneChipShot
			|| Session.PostRouteRollProgress.Phase
				== EMatchPlayCurrentAttackPostRouteRollPhase::OneOnOneDirectShot;
		if (Branch.ThroughBall == EMatchPlayThroughBallActualBranch::BehindDefense
			&& bHasP2)
		{
			const auto* Roll = FindRoll(
				Projection, EPostPurpose::BehindDefenseP2Defense);
			const bool bResolved = Roll != nullptr && Roll->bResolved;
			AddDecision(
				Projection,
				TEXT("ThroughBall.BehindDefense.P2.Outcome"),
				ERollSemantics::OutcomeDecision,
				{ EPostPurpose::BehindDefenseP2Defense },
				bResolved,
				bResolved && Roll->RawD6 <= 3
					? EOutcome::OneOnOneRequired : EOutcome::Offside);
		}
		if (Session.ThroughBallOneOnOneShotChoice
			== EMatchPlayThroughBallOneOnOneShotChoice::ChipShot)
		{
			const auto* Roll = FindRoll(
				Projection, EPostPurpose::OneOnOneChipShotAttack);
			const bool bResolved = Roll != nullptr && Roll->bResolved;
			AddDecision(
				Projection,
				TEXT("ThroughBall.OneOnOne.ChipShot.Outcome"),
				ERollSemantics::OutcomeDecision,
				{ EPostPurpose::OneOnOneChipShotAttack },
				bResolved,
				bResolved && Roll->RawD6 >= 4
					? EOutcome::Goal : EOutcome::Miss);
		}
	}

	void AddPrimaryRollContract(
		FProjection& Projection,
		const FSession& Session,
		const FMatchPlayCurrentAttackActualBranch& Branch)
	{
		if (IsDirectShotBranch(Branch))
		{
			AddPostRouteRoll(Projection, Session,
				EPostPurpose::PrimaryAttack,
				ERollSemantics::ArithmeticContest);
			AddPostRouteRoll(Projection, Session,
				EPostPurpose::PrimaryDefense,
				ERollSemantics::ArithmeticContest,
				true);
			return;
		}
		if (IsDeadCornerBranch(Branch))
		{
			AddPostRouteRoll(Projection, Session,
				EPostPurpose::PairedAttackA,
				ERollSemantics::OutcomeDecision);
			AddPostRouteRoll(Projection, Session,
				EPostPurpose::PairedAttackB,
				ERollSemantics::OutcomeDecision);
			return;
		}
		if (Branch.ActionType == ESkillRuleType::Cross
			|| Branch.ActionType == ESkillRuleType::PassControl
			|| (Branch.ActionType == ESkillRuleType::ThroughBall
				&& Branch.ThroughBall
					== EMatchPlayThroughBallActualBranch::Feet))
		{
			AddPostRouteRoll(Projection, Session,
				EPostPurpose::PrimaryAttack,
				ERollSemantics::ArithmeticContest);
			AddPostRouteRoll(Projection, Session,
				EPostPurpose::PrimaryDefense,
				ERollSemantics::ArithmeticContest);
			return;
		}
		if (Branch.ActionType == ESkillRuleType::ThroughBall
			&& Branch.ThroughBall
				== EMatchPlayThroughBallActualBranch::BehindDefense)
		{
			AddPostRouteRoll(Projection, Session,
				EPostPurpose::PrimaryAttack,
				ERollSemantics::ArithmeticContest);
			AddPostRouteRoll(Projection, Session,
				EPostPurpose::PrimaryDefense,
				ERollSemantics::ArithmeticContest,
				true);
			return;
		}
		if (Branch.ActionType == ESkillRuleType::ThroughBall
			&& Branch.ThroughBall
				== EMatchPlayThroughBallActualBranch::AntiOffside)
		{
			AddPostRouteRoll(Projection, Session,
				EPostPurpose::PrimaryAttack,
				ERollSemantics::OutcomeDecision);
		}
	}

	void AddLaterRollContracts(
		FProjection& Projection,
		const FSession& Session,
		const FMatchPlayCurrentAttackActualBranch& Branch)
	{
		if (Branch.ActionType != ESkillRuleType::ThroughBall)
		{
			return;
		}
		const auto Phase = Session.PostRouteRollProgress.Phase;
		if (Branch.ThroughBall == EMatchPlayThroughBallActualBranch::BehindDefense
			&& (Phase == EMatchPlayCurrentAttackPostRouteRollPhase::BehindDefenseP2
				|| Phase == EMatchPlayCurrentAttackPostRouteRollPhase::OneOnOneChipShot
				|| Phase == EMatchPlayCurrentAttackPostRouteRollPhase::OneOnOneDirectShot))
		{
			AddPostRouteRoll(Projection, Session,
				EPostPurpose::BehindDefenseP2Defense,
				ERollSemantics::OutcomeDecision);
		}
		if (Session.ThroughBallOneOnOneShotChoice
			== EMatchPlayThroughBallOneOnOneShotChoice::ChipShot)
		{
			AddPostRouteRoll(Projection, Session,
				EPostPurpose::OneOnOneChipShotAttack,
				ERollSemantics::OutcomeDecision);
		}
		else if (Session.ThroughBallOneOnOneShotChoice
			== EMatchPlayThroughBallOneOnOneShotChoice::DirectShot)
		{
			AddPostRouteRoll(Projection, Session,
				EPostPurpose::OneOnOneDirectShotAttack,
				ERollSemantics::ArithmeticContest);
			AddPostRouteRoll(Projection, Session,
				EPostPurpose::OneOnOneDirectShotDefense,
				ERollSemantics::ArithmeticContest);
		}
	}

	void AddRouteDecision(
		FProjection& Projection,
		const FSession& Session)
	{
		FMatchPlayResolutionDecisionFact Decision;
		Decision.DecisionId = TEXT("InitialRoute.Decision");
		Decision.Semantics = ERollSemantics::BranchSelection;
		if (RequiresInitialRouteRoll(Session.Bundle.Binding.ActionType))
		{
			Decision.RollSequenceIndices.Add(0);
		}
		Decision.bResolved = Session.bHasActualBranch;
		Decision.Outcome = Decision.bResolved
			? EOutcome::BranchSelected : EOutcome::None;
		Projection.Decisions.Add(Decision);
	}
}

FMatchPlayCurrentAttackResolutionFactProjection
FMatchPlayCurrentAttackResolutionFactProjectionQuery::Project(
	const FMatchPlayState& State,
	const FSkillRuleSnapshotSet* SkillRuleSet)
{
	using namespace MatchPlayCurrentAttackResolutionFactProjection;

	FProjection Projection;
	Projection.bSuccess = true;
	if (!State.RuntimeState.bIsInitialized
		|| !State.bHasCurrentAttack
		|| !State.CurrentAttack.bHasResolutionSession)
	{
		return Projection;
	}
	const FMatchPlayCurrentAttackResolutionSessionStateValidationResult Validation =
		FMatchPlayCurrentAttackResolutionSessionStateValidator::Validate(State);
	if (!Validation.bIsCanonical)
	{
		Fail(Projection, Validation.ErrorMessage);
		return Projection;
	}

	const FSession& Session = State.CurrentAttack.ResolutionSession;
	Projection.bHasFacts = true;
	Projection.AttackSequence = Session.AttackSequence;
	Projection.ActionType = Session.Bundle.Binding.ActionType;
	Projection.bHasActualBranch = Session.bHasActualBranch;
	if (Session.bHasActualBranch)
	{
		Projection.ActualBranch = Session.ActualBranch;
	}
	AddParticipant(Projection, EParticipantRole::Carrier, Session.Bundle.Carrier);
	AddParticipant(Projection, EParticipantRole::Runner, Session.Bundle.Runner);
	AddParticipant(Projection, EParticipantRole::Marker, Session.Bundle.Marker);
	AddParticipant(Projection, EParticipantRole::Helper, Session.Bundle.Helper);

	if (RequiresInitialRouteRoll(Projection.ActionType))
	{
		AddInitialRouteRoll(Projection, Session);
	}
	AddRouteDecision(Projection, Session);

	FMatchPlayCurrentAttackActualBranch EffectiveBranch;
	bool bHasEffectiveBranch = Session.bHasActualBranch;
	if (bHasEffectiveBranch)
	{
		EffectiveBranch = Session.ActualBranch;
	}
	else
	{
		bHasEffectiveBranch = InferIntentDeterminedBranch(
			Session, EffectiveBranch);
	}
	if (bHasEffectiveBranch)
	{
		AddPrimaryRollContract(Projection, Session, EffectiveBranch);
		AddLaterRollContracts(Projection, Session, EffectiveBranch);
		FString ProjectionError;
		if (!AddPrimaryContest(
			Projection,
			State,
			SkillRuleSet,
			EffectiveBranch,
			ProjectionError)
			|| !AddOneOnOneDirectShotContest(
				Projection,
				State,
				SkillRuleSet,
				ProjectionError))
		{
			Fail(Projection, ProjectionError);
			return Projection;
		}
		AddNonFormulaDecisions(Projection, Session, EffectiveBranch);
	}

	for (const FMatchPlayResolutionRollFact& Roll : Projection.Rolls)
	{
		bool bCurrentlyRequired = true;
		if (Roll.bConditionallyRequired)
		{
			const FMatchPlayResolutionRollFact* PrimaryAttackRoll =
				Projection.Rolls.FindByPredicate(
					[](const FMatchPlayResolutionRollFact& Candidate)
					{
						return Candidate.PostRoutePurpose
							== EPostPurpose::PrimaryAttack;
					});
			bCurrentlyRequired = PrimaryAttackRoll == nullptr
				|| !PrimaryAttackRoll->bResolved
				|| PrimaryAttackRoll->RawD6 >= 3;
		}
		if (!Roll.bResolved
			&& bCurrentlyRequired
			&& !Projection.bHasPendingRoll)
		{
			Projection.bHasPendingRoll = true;
			Projection.NextPendingRollSequenceIndex = Roll.SequenceIndex;
		}
	}
	return Projection;
}
