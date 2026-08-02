#pragma once

#include "CoreMinimal.h"

#include "MatchPlayAuthoritativeSessionTypes.h"

class FMCODEX_API FMatchPlayAuthoritativeSession final
{
public:
	FMatchPlayAuthoritativeSession();
	~FMatchPlayAuthoritativeSession();

	FMatchPlayAuthoritativeInitializeMatchResult InitializeMatch(
		const FMatchPlayOpeningInitializeInput& Input);

	FMatchPlayAuthoritativeBeginOrdinaryAttackResult BeginOrdinaryAttack(
		int32 ActionPoint);

	FMatchPlayAuthoritativeFinishDeploymentResult FinishDeployment(
		int64 AttackSequence,
		EInitialTurnOrderPlayer RequestingSide);

	FMatchPlayAuthoritativeDeployOrdinaryResult DeployOrdinary(
		const FMatchPlayAuthoritativeDeployOrdinaryRequest& Request);

	FMatchPlayAuthoritativeSubmitCarrierResult SubmitCarrier(
		const FMatchPlayAuthoritativeSubmitCarrierRequest& Request);

	FMatchPlayState GetStateSnapshot() const;

private:
	FMatchPlayAuthoritativeSession(
		const FMatchPlayAuthoritativeSession&) = delete;
	FMatchPlayAuthoritativeSession& operator=(
		const FMatchPlayAuthoritativeSession&) = delete;
	FMatchPlayAuthoritativeSession(
		FMatchPlayAuthoritativeSession&&) = delete;
	FMatchPlayAuthoritativeSession& operator=(
		FMatchPlayAuthoritativeSession&&) = delete;

	struct FDomainExecution
	{
		bool bSuccess = false;
		FMatchPlayState CandidateAfterState;
		EMatchPlayAuthoritativeStateDisposition StateDisposition =
			EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
		int64 AttackSequence = 0;
	};

	template <typename TTypedResult, typename TExecuteDomain>
	TTypedResult ExecuteSerialized(
		EMatchPlayAuthoritativeCommandKind CommandKind,
		bool bRequiresInitializedState,
		int64 CommandAttackSequence,
		TExecuteDomain&& ExecuteDomain);

	FMatchPlayState AuthoritativeState;
	bool bExecutingCommand = false;
};
