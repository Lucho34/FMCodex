#include "MatchPlayCurrentAttackResolveThroughBallOneOnOneDirectShotPostRoutePlanOrchestrator.h"

#include "MatchPlayCardSnapshotAuthority.h"
#include "MatchPlayCurrentAttackResolveThroughBallAntiOffsideDecisionOrchestrator.h"
#include "MatchPlayCurrentAttackResolveThroughBallBehindDefenseP1FormulaOrchestrator.h"

namespace MatchPlayCurrentAttackResolveThroughBallOneOnOneDirectShotPostRoutePlan
{
	using FResult = FMatchPlayCurrentAttackResolveThroughBallOneOnOneDirectShotPostRoutePlanResult;
	using EError = EMatchPlayCurrentAttackResolveThroughBallOneOnOneDirectShotPostRoutePlanErrorCode;
	using ESource = EMatchPlayThroughBallOneOnOneSource;
	using EPhase = EMatchPlayCurrentAttackPostRouteRollPhase;
	using EPurpose = EMatchPlayCurrentAttackPostRouteRollPurpose;

	void Fail(FResult& Result, EError Error, const FString& Message, FName Field = NAME_None)
	{
		Result.ErrorCode = Error;
		Result.ErrorMessage = Message;
		Result.InvalidField = Field;
	}

	EError ProviderError(EMatchPlayPostRouteRollProviderResultValidationErrorCode Error)
	{
		return Error == EMatchPlayPostRouteRollProviderResultValidationErrorCode::ProviderFailure
			? EError::PostRouteRollProviderFailed
			: EError::MalformedPostRouteRollProviderResult;
	}

	bool BuildSourceState(FResult& Result)
	{
		Result.SourceProvenanceState = Result.BeforeState;
		auto& Session = Result.SourceProvenanceState.CurrentAttack.ResolutionSession;
		auto& Progress = Session.PostRouteRollProgress;
		if (Result.Source == ESource::AntiOffside)
		{
			if (Progress.Phase == EPhase::OneOnOneDirectShot)
			{
				Progress.Phase = EPhase::PrimaryBranch;
				Progress.RollRecords.SetNum(1);
			}
			else if (Progress.Phase != EPhase::PrimaryBranch)
			{
				Fail(Result, EError::UnsupportedSourcePhase, TEXT("AntiOffside DirectShot requires PrimaryBranch provenance."));
				return false;
			}
		}
		else
		{
			if (Progress.Phase == EPhase::OneOnOneDirectShot)
			{
				Progress.Phase = EPhase::PrimaryBranch;
				Progress.RollRecords.SetNum(2);
			}
			else if (Progress.Phase != EPhase::PrimaryBranch)
			{
				Fail(Result, EError::UnsupportedSourcePhase, TEXT("BehindDefense DirectShot requires P1 provenance."));
				return false;
			}
		}
		const auto Validation = FMatchPlayCurrentAttackResolutionSessionStateValidator::Validate(Result.SourceProvenanceState);
		if (!Validation.bIsCanonical)
		{
			Fail(Result, EError::InvalidSourceProvenanceState, Validation.ErrorMessage);
			return false;
		}
		Result.SourceProvenanceProgressResult = FMatchPlayCurrentAttackPostRouteRollProgressQuery::Evaluate(Session);
		if (!Result.SourceProvenanceProgressResult.bIsCanonical)
		{
			Fail(Result, EError::InvalidSourceProvenanceState, Result.SourceProvenanceProgressResult.ErrorMessage);
			return false;
		}
		if (!Result.SourceProvenanceProgressResult.bContractComplete)
		{
			Fail(Result, EError::IncompleteSourceProvenance, TEXT("DirectShot source provenance is incomplete."));
			return false;
		}
		return true;
	}

	bool RegenerateSource(FResult& Result, const FSkillRuleSnapshotSet* Rules)
	{
		++Result.SourceDecisionRegenerationCount;
		if (Result.Source == ESource::AntiOffside)
		{
			FMatchPlayCurrentAttackResolveThroughBallAntiOffsideDecisionRequest Request;
			Request.AttackSequence = Result.SourceProvenanceState.CurrentAttack.AttackSequence;
			Request.Mode =
				FMatchPlayCurrentAttackResolveThroughBallAntiOffsideDecisionRequest
					::EMode::RegenerateCompletedDecision;
			Result.AntiOffsideRegenerationResult =
				FMatchPlayCurrentAttackResolveThroughBallAntiOffsideDecisionOrchestrator::Resolve(
					Result.SourceProvenanceState, Request, Rules, nullptr);
			if (!Result.AntiOffsideRegenerationResult.bSuccess)
			{
				Fail(Result, EError::SourceDecisionRegenerationFailed, Result.AntiOffsideRegenerationResult.ErrorMessage);
				return false;
			}
			if (Result.AntiOffsideRegenerationResult.OutcomeResult.Decision != EThroughBallAntiOffsideOutcomeDecision::OneOnOneRequired)
			{
				Fail(Result, EError::SourceDecisionDoesNotRequireOneOnOne, TEXT("AntiOffside does not require OneOnOne."));
				return false;
			}
			++Result.HandoffCreationCount;
			Result.HandoffCreationResult = FThroughBallOneOnOneHandoffCreator::CreateFromAntiOffside(
				Result.AntiOffsideRegenerationResult.OutcomeResult);
		}
		else
		{
			Result.BehindDefenseP1RegenerationResult =
				FMatchPlayCurrentAttackResolveThroughBallBehindDefenseP1FormulaOrchestrator::Resolve(
					Result.SourceProvenanceState, Rules);
			if (!Result.BehindDefenseP1RegenerationResult.bSuccess)
			{
				Fail(Result, EError::SourceDecisionRegenerationFailed, Result.BehindDefenseP1RegenerationResult.ErrorMessage,
					Result.BehindDefenseP1RegenerationResult.InvalidField);
				return false;
			}
			if (Result.BehindDefenseP1RegenerationResult.FormulaExecutionResult.Decision
				!= EThroughBallBehindDefenseP1FormulaResolutionExecutionDecision::OneOnOneRequired)
			{
				Fail(Result, EError::SourceDecisionDoesNotRequireOneOnOne, TEXT("BehindDefense P1 does not require OneOnOne."));
				return false;
			}
			++Result.HandoffCreationCount;
			Result.HandoffCreationResult = FThroughBallOneOnOneHandoffCreator::CreateFromBehindDefenseP1(
				Result.BehindDefenseP1RegenerationResult.FormulaExecutionResult);
		}
		if (!Result.HandoffCreationResult.bSuccess || !Result.HandoffCreationResult.bHasHandoff)
		{
			Fail(Result, EError::HandoffCreationFailed, Result.HandoffCreationResult.ErrorMessage,
				Result.HandoffCreationResult.InvalidField);
			return false;
		}
		return true;
	}

	FName GoalkeeperId(const FMatchPlayState& State, EInitialTurnOrderPlayer Side)
	{
		return Side == EInitialTurnOrderPlayer::PlayerA
			? FName(*State.RuntimeState.PlayerAState.GoalkeeperCardId)
			: Side == EInitialTurnOrderPlayer::PlayerB
				? FName(*State.RuntimeState.PlayerBState.GoalkeeperCardId)
				: NAME_None;
	}

	bool BuildPlan(FResult& Result, const FMatchPlayState& Candidate)
	{
		const auto& Session = Candidate.CurrentAttack.ResolutionSession;
		const auto& Bundle = Session.Bundle;
		const auto& Handoff = Result.HandoffCreationResult.Handoff;
		if (!Bundle.bHasRunner || !Bundle.Runner.bIsPresent || Handoff.ShooterCardId != Bundle.Runner.CardId)
		{
			Fail(Result, EError::ShooterIdentityMismatch, TEXT("DirectShot handoff Shooter must match the frozen Runner."), TEXT("ShooterCardId"));
			return false;
		}
		const FName GkId = GoalkeeperId(Candidate, Bundle.CurrentDefendingPlayer);
		const auto SnapshotResult = FMatchPlayCardSnapshotAuthorityQuery::FindByPlayerSideAndCardId(
			Candidate.CardSnapshotAuthority, Bundle.CurrentDefendingPlayer, GkId);
		if (!SnapshotResult.bSuccess)
		{
			Fail(Result, EError::GoalkeeperSnapshotUnavailable, SnapshotResult.ErrorMessage, TEXT("GoalkeeperCardId"));
			return false;
		}
		Result.GoalkeeperSnapshot = SnapshotResult.Snapshot;
		if (!Result.GoalkeeperSnapshot.bIsGoalkeeper || !Result.GoalkeeperSnapshot.bHasGoalkeeperAttributes
			|| Result.GoalkeeperSnapshot.CardId != GkId)
		{
			Fail(Result, EError::InvalidGoalkeeperSnapshot, TEXT("DirectShot canonical goalkeeper snapshot is invalid."), TEXT("GoalkeeperSnapshot"));
			return false;
		}
		Result.bGoalkeeperActivated = Candidate.CurrentAttack.bCurrentDefenseGoalkeeperActivated;
		if (Result.bGoalkeeperActivated)
		{
			int32 MatchingPlacements = 0;
			for (const FMatchPlayDeploymentPlacement& Placement : Candidate.CurrentAttack.DeploymentPlacements)
			{
				if (Placement.PlayerSide == Bundle.CurrentDefendingPlayer && Placement.CardId == GkId)
				{
					++MatchingPlacements;
				}
			}
			if (MatchingPlacements != 1)
			{
				Fail(Result, EError::ActiveGoalkeeperPlacementMismatch, TEXT("Active goalkeeper must have exactly one matching defending placement."), TEXT("DeploymentPlacements"));
				return false;
			}
		}
		const auto& Records = Session.PostRouteRollProgress.RollRecords;
		if (Records.Num() < 2
			|| Records[Records.Num() - 2].Purpose != EPurpose::OneOnOneDirectShotAttack
			|| Records.Last().Purpose != EPurpose::OneOnOneDirectShotDefense)
		{
			Fail(Result, EError::FormulaPlanCreationFailed, TEXT("DirectShot plan requires both ordered DirectShot rolls."), TEXT("RollRecords"));
			return false;
		}
		if (Session.AttackSequence <= 0 || Session.AttackSequence > MAX_int32)
		{
			Fail(Result, EError::UnrepresentableTurnIndex, TEXT("DirectShot TurnIndex cannot represent AttackSequence."));
			return false;
		}
		const uint64 Sequence = static_cast<uint64>(Session.AttackSequence);
		auto& Plan = Result.FormulaPlan;
		Plan.Handoff = Handoff;
		Plan.GoalkeeperCardId = GkId;
		Plan.ShooterShooting = Bundle.Runner.Values.Shooting;
		Plan.GoalkeeperOneOnOne = Result.GoalkeeperSnapshot.GoalkeeperAttributes.OneOnOne;
		Plan.bGoalkeeperActivated = Result.bGoalkeeperActivated;
		Plan.AttackD6 = Records[Records.Num() - 2].RawD6;
		Plan.DefenseD6 = Records.Last().RawD6;
		Plan.LogId = FGuid(0x54423131, static_cast<uint32>(Sequence >> 32), static_cast<uint32>(Sequence), 0x44495245);
		Plan.TurnIndex = static_cast<int32>(Session.AttackSequence - 1);
		Plan.InvolvedCardIds = {Handoff.ShooterCardId, GkId};
		Result.bHasFormulaPlan = true;
		return true;
	}
}

FMatchPlayCurrentAttackResolveThroughBallOneOnOneDirectShotPostRoutePlanResult
FMatchPlayCurrentAttackResolveThroughBallOneOnOneDirectShotPostRoutePlanOrchestrator::Resolve(
	const FMatchPlayState& BeforeState,
	const FMatchPlayCurrentAttackResolveThroughBallOneOnOneDirectShotPostRoutePlanRequest& Request,
	const FSkillRuleSnapshotSet* SkillRuleSet,
	IMatchPlayPostRouteRollProvider* RollProvider)
{
	using namespace MatchPlayCurrentAttackResolveThroughBallOneOnOneDirectShotPostRoutePlan;
	FResult Result;
	Result.Request = Request;
	Result.BeforeState = BeforeState;
	Result.AfterState = BeforeState;
	if (!BeforeState.RuntimeState.bIsInitialized) { Fail(Result, EError::MatchPlayStateNotInitialized, TEXT("DirectShot requires initialized MatchPlay State.")); return Result; }
	if (!BeforeState.bHasCurrentAttack) { Fail(Result, EError::NoCurrentAttack, TEXT("DirectShot requires a CurrentAttack.")); return Result; }
	if (BeforeState.CurrentAttack.AttackSequence <= 0) { Fail(Result, EError::InvalidCurrentAttackSequence, TEXT("CurrentAttack AttackSequence must be positive.")); return Result; }
	if (Request.AttackSequence <= 0) { Fail(Result, EError::InvalidRequestedAttackSequence, TEXT("Requested AttackSequence must be positive.")); return Result; }
	if (Request.AttackSequence != BeforeState.CurrentAttack.AttackSequence) { Fail(Result, EError::AttackSequenceMismatch, TEXT("Requested AttackSequence does not match CurrentAttack.")); return Result; }
	if (!BeforeState.CurrentAttack.bHasResolutionSession) { Fail(Result, EError::MissingResolutionSession, TEXT("DirectShot requires a Resolution Session.")); return Result; }
	Result.SessionStateValidationResult = FMatchPlayCurrentAttackResolutionSessionStateValidator::Validate(BeforeState);
	if (!Result.SessionStateValidationResult.bIsCanonical) { Fail(Result, EError::InvalidResolutionSessionState, Result.SessionStateValidationResult.ErrorMessage); return Result; }
	const auto& BeforeSession = BeforeState.CurrentAttack.ResolutionSession;
	if (BeforeSession.Stage != EMatchPlayCurrentAttackResolutionStage::RouteResolved) { Fail(Result, EError::RouteNotResolved, TEXT("DirectShot requires RouteResolved.")); return Result; }
	if (!BeforeSession.bHasActualBranch || BeforeSession.ActualBranch.ActionType != ESkillRuleType::ThroughBall) { Fail(Result, EError::UnsupportedThroughBallBranch, TEXT("DirectShot supports only ThroughBall.")); return Result; }
	if (BeforeSession.ActualBranch.ThroughBall == EMatchPlayThroughBallActualBranch::AntiOffside) Result.Source = ESource::AntiOffside;
	else if (BeforeSession.ActualBranch.ThroughBall == EMatchPlayThroughBallActualBranch::BehindDefense) Result.Source = ESource::BehindDefense;
	else { Fail(Result, EError::UnsupportedThroughBallBranch, TEXT("This ThroughBall branch has no OneOnOne DirectShot provenance.")); return Result; }
	if (BeforeSession.ThroughBallOneOnOneShotChoice == EMatchPlayThroughBallOneOnOneShotChoice::None) { Fail(Result, EError::OneOnOneShotChoiceNotSelected, TEXT("DirectShot choice has not been selected.")); return Result; }
	if (BeforeSession.ThroughBallOneOnOneShotChoice != EMatchPlayThroughBallOneOnOneShotChoice::DirectShot) { Fail(Result, EError::OneOnOneShotChoiceDoesNotPermitDirectShot, TEXT("Accepted choice does not permit DirectShot.")); return Result; }
	Result.BeforeProgressResult = FMatchPlayCurrentAttackPostRouteRollProgressQuery::Evaluate(BeforeSession);
	if (!Result.BeforeProgressResult.bIsCanonical) { Fail(Result, EError::InvalidPostRouteProgress, Result.BeforeProgressResult.ErrorMessage); return Result; }
	if (!BuildSourceState(Result) || !RegenerateSource(Result, SkillRuleSet)) return Result;

	FMatchPlayState Candidate = BeforeState;
	auto& CandidateSession = Candidate.CurrentAttack.ResolutionSession;
	auto& Progress = CandidateSession.PostRouteRollProgress;
	const EPhase SourcePhase = EPhase::PrimaryBranch;
	if (Progress.Phase == SourcePhase) Progress.Phase = EPhase::OneOnOneDirectShot;
	else if (Progress.Phase != EPhase::OneOnOneDirectShot) { Fail(Result, EError::UnsupportedSourcePhase, TEXT("DirectShot source phase is unsupported.")); return Result; }
	Result.AfterProgressResult = FMatchPlayCurrentAttackPostRouteRollProgressQuery::Evaluate(CandidateSession);
	if (!Result.AfterProgressResult.bIsCanonical) { Fail(Result, EError::InvalidPostRouteProgress, Result.AfterProgressResult.ErrorMessage); return Result; }
	using EMode = FMatchPlayCurrentAttackResolveThroughBallOneOnOneDirectShotPostRoutePlanRequest::EMode;
	const bool bExplicitRollStep = Request.Mode == EMode::ResolveAttackRoll
		|| Request.Mode == EMode::ResolveDefenseRoll;
	if (Request.Mode == EMode::RegenerateCompletedPlan
		&& !Result.AfterProgressResult.bContractComplete)
	{
		Fail(Result, EError::CompletedPlanRequired, TEXT("OneOnOne DirectShot regeneration requires an already-complete roll contract."));
		return Result;
	}
	if (bExplicitRollStep)
	{
		if (Request.RequestingSide != EInitialTurnOrderPlayer::PlayerA
			&& Request.RequestingSide != EInitialTurnOrderPlayer::PlayerB)
		{
			Fail(Result, EError::InvalidRequestingSide, TEXT("OneOnOne DirectShot roll commands require PlayerA or PlayerB as RequestingSide."));
			return Result;
		}
		const EPurpose RequestedPurpose = Request.Mode == EMode::ResolveAttackRoll
			? EPurpose::OneOnOneDirectShotAttack
			: EPurpose::OneOnOneDirectShotDefense;
		if (Result.AfterProgressResult.bContractComplete
			|| Result.AfterProgressResult.NextPurpose != RequestedPurpose)
		{
			Fail(Result, EError::WrongDirectShotRollStep, TEXT("The requested OneOnOne DirectShot roll is not the current authoritative step."));
			return Result;
		}
		const EInitialTurnOrderPlayer ExpectedSide = RequestedPurpose
			== EPurpose::OneOnOneDirectShotAttack
				? BeforeSession.Bundle.CurrentAttackingPlayer
				: BeforeSession.Bundle.CurrentDefendingPlayer;
		if (Request.RequestingSide != ExpectedSide)
		{
			Fail(Result, EError::WrongRequestingSide, TEXT("The requesting side does not own the current OneOnOne DirectShot roll."));
			return Result;
		}
	}
	const int32 MaximumRollsThisCommand = bExplicitRollStep ? 1 : MAX_int32;
	while (!Result.AfterProgressResult.bContractComplete
		&& Result.ProviderCallCount < MaximumRollsThisCommand)
	{
		const EPurpose Purpose = Result.AfterProgressResult.NextPurpose;
		if (Purpose != EPurpose::OneOnOneDirectShotAttack && Purpose != EPurpose::OneOnOneDirectShotDefense) { Fail(Result, EError::UnexpectedDirectShotRollPurpose, TEXT("DirectShot progress requested an unexpected purpose.")); return Result; }
		if (RollProvider == nullptr) { Fail(Result, EError::PostRouteRollProviderUnavailable, TEXT("DirectShot roll provider is unavailable.")); return Result; }
		const auto ProviderResult = RollProvider->RollD6(Purpose);
		++Result.ProviderCallCount;
		Result.ProviderResults.Add(ProviderResult);
		const auto Validation = FMatchPlayPostRouteRollProviderResultValidator::Validate(Purpose, ProviderResult);
		Result.ProviderValidationResults.Add(Validation);
		if (!Validation.bIsCanonical) { Fail(Result, ProviderError(Validation.ErrorCode), Validation.ErrorMessage); return Result; }
		FMatchPlayCurrentAttackPostRouteRollRecord Record;
		Record.Purpose = Purpose;
		Record.RawD6 = ProviderResult.RawD6;
		Progress.RollRecords.Add(Record);
		Result.AfterProgressResult = FMatchPlayCurrentAttackPostRouteRollProgressQuery::Evaluate(CandidateSession);
		if (!Result.AfterProgressResult.bIsCanonical) { Fail(Result, EError::InvalidPostRouteProgress, Result.AfterProgressResult.ErrorMessage); return Result; }
	}
	if (bExplicitRollStep && !Result.AfterProgressResult.bContractComplete)
	{
		Result.SessionStateValidationResult = FMatchPlayCurrentAttackResolutionSessionStateValidator::Validate(Candidate);
		if (!Result.SessionStateValidationResult.bIsCanonical) { Fail(Result, EError::InvalidCandidateState, Result.SessionStateValidationResult.ErrorMessage); return Result; }
		Result.AfterState = MoveTemp(Candidate);
		Result.bResolvedNewRolls = Result.ProviderCallCount > 0;
		Result.bSuccess = true;
		return Result;
	}
	Result.SessionStateValidationResult = FMatchPlayCurrentAttackResolutionSessionStateValidator::Validate(Candidate);
	if (!Result.SessionStateValidationResult.bIsCanonical) { Fail(Result, EError::InvalidCandidateState, Result.SessionStateValidationResult.ErrorMessage); return Result; }
	if (!BuildPlan(Result, Candidate)) return Result;
	Result.AfterState = MoveTemp(Candidate);
	Result.bResolvedNewRolls = Result.ProviderCallCount > 0;
	Result.bReplayedCompleteRolls = Result.ProviderCallCount == 0;
	Result.bSuccess = true;
	return Result;
}

FMatchPlayCurrentAttackResolveThroughBallOneOnOneDirectShotPostRoutePlanResult
FMatchPlayCurrentAttackResolveThroughBallOneOnOneDirectShotPostRoutePlanOrchestrator::Resolve(
	const FMatchPlayState& BeforeState,
	const FSkillRuleSnapshotSet* SkillRuleSet,
	IMatchPlayPostRouteRollProvider* RollProvider)
{
	FMatchPlayCurrentAttackResolveThroughBallOneOnOneDirectShotPostRoutePlanRequest Request;
	Request.AttackSequence = BeforeState.bHasCurrentAttack
		? BeforeState.CurrentAttack.AttackSequence
		: 0;
	Request.Mode =
		FMatchPlayCurrentAttackResolveThroughBallOneOnOneDirectShotPostRoutePlanRequest
			::EMode::CompletePlan;
	return Resolve(BeforeState, Request, SkillRuleSet, RollProvider);
}
