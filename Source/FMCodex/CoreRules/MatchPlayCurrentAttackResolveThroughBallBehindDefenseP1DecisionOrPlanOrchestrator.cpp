#include "MatchPlayCurrentAttackResolveThroughBallBehindDefenseP1DecisionOrPlanOrchestrator.h"
#include "MatchPlayCardSnapshotAuthority.h"

namespace MatchPlayCurrentAttackResolveThroughBallBehindDefenseP1DecisionOrPlan
{
	using EError = EMatchPlayCurrentAttackResolveThroughBallBehindDefenseP1DecisionOrPlanErrorCode;
	using EPurpose = EMatchPlayCurrentAttackPostRouteRollPurpose;
	using EPhase = EMatchPlayCurrentAttackPostRouteRollPhase;
	using FResult = FMatchPlayCurrentAttackResolveThroughBallBehindDefenseP1DecisionOrPlanResult;
	void Fail(FResult& R, EError C, const FString& M) { R.ErrorCode = C; R.ErrorMessage = M; }
	FName Owner(EInitialTurnOrderPlayer S)
	{
		if (S == EInitialTurnOrderPlayer::PlayerA) return TEXT("PlayerA");
		if (S == EInitialTurnOrderPlayer::PlayerB) return TEXT("PlayerB");
		return NAME_None;
	}
	FPlayerAttributes Attributes(const FMatchPlayBoundActionNormalizedParticipantValues& V)
	{
		FPlayerAttributes A; A.Shooting=V.Shooting; A.Dribbling=V.Dribbling; A.Passing=V.Passing; A.OffBall=V.OffBall; A.Marking=V.Marking; A.Tackling=V.Tackling; A.Speed=V.Speed; A.Strength=V.Strength; A.Stamina=V.Stamina; A.LongShot=V.LongShot; return A;
	}
	bool Snapshot(const FMatchPlayState& S, const FMatchPlayCurrentAttackResolutionSessionParticipant& P, FPlayerCardRuleSnapshot& O, FString& M)
	{
		const auto Q=FMatchPlayCardSnapshotAuthorityQuery::FindByPlayerSideAndCardId(S.CardSnapshotAuthority,P.Side,P.CardId);
		if(!Q.bSuccess){M=Q.ErrorMessage;return false;} O=Q.Snapshot; O.Attributes=Attributes(P.Values); return true;
	}
	bool Input(const FMatchPlayState& S,const FSkillRuleSnapshotSet& Rules,FResult& R)
	{
		const auto& A=S.CurrentAttack; const auto& Session=A.ResolutionSession; const auto& B=Session.Bundle; const auto& Rolls=Session.PostRouteRollProgress.RollRecords;
		if(Session.PostRouteRollProgress.Phase!=EPhase::PrimaryBranch || (Rolls.Num()!=1 && Rolls.Num()!=2)){Fail(R,EError::InputAdaptationFailed,TEXT("BehindDefense P1 requires a completed conditional primary-roll contract."));return false;}
		FThroughBallParticipantEligibilityQueryInput I; I.SelectedSkillId=B.Binding.SkillId; I.CurrentActionPoint=A.ActionPoint; I.AttackingOwnerId=Owner(B.CurrentAttackingPlayer); I.DefendingOwnerId=Owner(B.CurrentDefendingPlayer); I.bHasHelper=B.bHasHelper; I.bIsRunnerInAttackingForwardArea=true;
		FString M;
		if(!Snapshot(S,B.Carrier,I.CarrierSnapshot,M)||!Snapshot(S,B.Runner,I.RunnerSnapshot,M)||!Snapshot(S,B.Marker,I.MarkerSnapshot,M)||(B.bHasHelper&&!Snapshot(S,B.Helper,I.HelperSnapshot,M))){Fail(R,EError::ParticipantSnapshotUnavailable,M);return false;}
		R.ParticipantEligibilityResult=FThroughBallParticipantEligibilityQuery::Evaluate(Rules,I);
		if(!R.ParticipantEligibilityResult.bSuccess){Fail(R,EError::ParticipantEligibilityFailed,R.ParticipantEligibilityResult.ErrorMessage);return false;}
		R.QueryInput.ParticipantEligibilityResult=R.ParticipantEligibilityResult; R.QueryInput.SelectedBranch=EThroughBallSelectedBranch::BehindDefense; R.QueryInput.bHasAttackD6=true; R.QueryInput.AttackD6=Rolls[0].RawD6; R.QueryInput.bHasDefenseD6=Rolls.Num()==2; if(R.QueryInput.bHasDefenseD6)R.QueryInput.DefenseD6=Rolls[1].RawD6;
		const uint64 Seq=static_cast<uint64>(Session.AttackSequence); R.QueryInput.LogId=FGuid(0x42445031,static_cast<uint32>(Seq>>32),static_cast<uint32>(Seq),0x504C414E); R.QueryInput.TurnIndex=static_cast<int32>(Session.AttackSequence-1); return true;
	}
	EError ProviderError(EMatchPlayPostRouteRollProviderResultValidationErrorCode C){return C==EMatchPlayPostRouteRollProviderResultValidationErrorCode::ProviderFailure?EError::PostRouteRollProviderFailed:EError::MalformedPostRouteRollProviderResult;}
}

FMatchPlayCurrentAttackResolveThroughBallBehindDefenseP1DecisionOrPlanResult FMatchPlayCurrentAttackResolveThroughBallBehindDefenseP1DecisionOrPlanOrchestrator::Resolve(const FMatchPlayState& BeforeState,const FMatchPlayCurrentAttackResolveThroughBallBehindDefenseP1DecisionOrPlanRequest& Request,const FSkillRuleSnapshotSet* SkillRuleSet,IMatchPlayPostRouteRollProvider* RollProvider)
{
	using namespace MatchPlayCurrentAttackResolveThroughBallBehindDefenseP1DecisionOrPlan; FResult R; R.Request=Request;R.BeforeState=BeforeState;R.AfterState=BeforeState;
	if(!BeforeState.RuntimeState.bIsInitialized){Fail(R,EError::MatchPlayStateNotInitialized,TEXT("BehindDefense P1 requires initialized MatchPlay State."));return R;} if(!BeforeState.bHasCurrentAttack){Fail(R,EError::NoCurrentAttack,TEXT("BehindDefense P1 requires an active CurrentAttack."));return R;} if(BeforeState.CurrentAttack.AttackSequence<=0){Fail(R,EError::InvalidCurrentAttackSequence,TEXT("CurrentAttack AttackSequence must be positive."));return R;} if(Request.AttackSequence<=0){Fail(R,EError::InvalidRequestedAttackSequence,TEXT("Requested AttackSequence must be positive."));return R;} if(Request.AttackSequence!=BeforeState.CurrentAttack.AttackSequence){Fail(R,EError::AttackSequenceMismatch,TEXT("Requested AttackSequence does not match CurrentAttack."));return R;} if(!BeforeState.CurrentAttack.bHasResolutionSession){Fail(R,EError::MissingResolutionSession,TEXT("BehindDefense P1 requires a Resolution Session."));return R;}
	R.SessionStateValidationResult=FMatchPlayCurrentAttackResolutionSessionStateValidator::Validate(BeforeState);if(!R.SessionStateValidationResult.bIsCanonical){Fail(R,EError::InvalidResolutionSessionState,R.SessionStateValidationResult.ErrorMessage);return R;} const auto& BeforeSession=BeforeState.CurrentAttack.ResolutionSession; if(BeforeSession.Stage!=EMatchPlayCurrentAttackResolutionStage::RouteResolved){Fail(R,EError::RouteNotResolved,TEXT("BehindDefense P1 requires RouteResolved."));return R;} if(!BeforeSession.bHasActualBranch||BeforeSession.ActualBranch.ActionType!=ESkillRuleType::ThroughBall||BeforeSession.ActualBranch.ThroughBall!=EMatchPlayThroughBallActualBranch::BehindDefense){Fail(R,EError::NotThroughBallBehindDefenseBranch,TEXT("This operation supports only ThroughBall BehindDefense."));return R;}
	FMatchPlayState Candidate=BeforeState;auto& Session=Candidate.CurrentAttack.ResolutionSession;if(Session.PostRouteRollProgress.Phase==EPhase::None)Session.PostRouteRollProgress.Phase=EPhase::PrimaryBranch;else if(Session.PostRouteRollProgress.Phase!=EPhase::PrimaryBranch){Fail(R,EError::InvalidPostRouteProgress,TEXT("BehindDefense P1 requires primary-branch roll progress."));return R;} R.ProgressResult=FMatchPlayCurrentAttackPostRouteRollProgressQuery::Evaluate(Session);if(!R.ProgressResult.bIsCanonical){Fail(R,EError::InvalidPostRouteProgress,R.ProgressResult.ErrorMessage);return R;}if(!R.ProgressResult.bContractComplete&&RollProvider==nullptr){Fail(R,EError::PostRouteRollProviderUnavailable,TEXT("BehindDefense P1 post-route roll provider is unavailable."));return R;}if(SkillRuleSet==nullptr){Fail(R,EError::SkillRuleSetUnavailable,TEXT("BehindDefense P1 requires authoritative SkillRuleSet."));return R;}
	while(!R.ProgressResult.bContractComplete){const EPurpose P=R.ProgressResult.NextPurpose;const auto V=RollProvider->RollD6(P);++R.ProviderCallCount;R.ProviderResults.Add(V);const auto Check=FMatchPlayPostRouteRollProviderResultValidator::Validate(P,V);R.ProviderValidationResults.Add(Check);if(!Check.bIsCanonical){Fail(R,ProviderError(Check.ErrorCode),Check.ErrorMessage);return R;}FMatchPlayCurrentAttackPostRouteRollRecord Record;Record.Purpose=P;Record.RawD6=V.RawD6;Session.PostRouteRollProgress.RollRecords.Add(Record);R.ProgressResult=FMatchPlayCurrentAttackPostRouteRollProgressQuery::Evaluate(Session);if(!R.ProgressResult.bIsCanonical){Fail(R,EError::InvalidPostRouteProgress,R.ProgressResult.ErrorMessage);return R;}}
	R.SessionStateValidationResult=FMatchPlayCurrentAttackResolutionSessionStateValidator::Validate(Candidate);if(!R.SessionStateValidationResult.bIsCanonical){Fail(R,EError::InvalidPostRouteProgress,R.SessionStateValidationResult.ErrorMessage);return R;}if(!Input(Candidate,*SkillRuleSet,R))return R;R.P1PlanResult=FThroughBallBehindDefenseP1PlanQuery::Evaluate(R.QueryInput);if(!R.P1PlanResult.bSuccess){Fail(R,EError::P1PlanQueryFailed,R.P1PlanResult.ErrorMessage);return R;}R.AfterState=MoveTemp(Candidate);R.bResolvedNewRolls=R.ProviderCallCount>0;R.bReplayedCompleteRolls=R.ProviderCallCount==0;R.bSuccess=true;return R;
}
