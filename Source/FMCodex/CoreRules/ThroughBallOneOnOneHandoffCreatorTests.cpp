#if WITH_DEV_AUTOMATION_TESTS

#include "ThroughBallOneOnOneHandoffCreator.h"

#include "ThroughBallAntiOffsideOutcomeQuery.h"
#include "ThroughBallBehindDefenseP2OutcomeQuery.h"

#include "Misc/AutomationTest.h"

#include <type_traits>

namespace ThroughBallOneOnOneHandoffCreatorTests
{
	const FName AttackingOwnerId(TEXT("Player.Handoff.Attacking"));
	const FName DefendingOwnerId(TEXT("Player.Handoff.Defending"));
	const FName ShooterCardId(TEXT("Card.Handoff.Shooter"));

	using EError = EThroughBallOneOnOneHandoffCreationErrorCode;

	FThroughBallBehindDefenseP2OutcomeQueryResult MakeP2Result()
	{
		FThroughBallBehindDefenseP2OutcomeQueryResult Result;
		Result.bSuccess = true;
		Result.Decision =
			EThroughBallBehindDefenseP2OutcomeDecision::OneOnOneRequired;
		Result.bContinueResolution = true;
		Result.bRequiresOneOnOne = true;
		Result.RunnerId = ShooterCardId;

		FThroughBallBehindDefenseP1FormulaResolutionExecutionResult& P1Result =
			Result.Input.P1ExecutionResult;
		P1Result.bSuccess = true;
		P1Result.Decision =
			EThroughBallBehindDefenseP1FormulaResolutionExecutionDecision
				::P2Required;
		P1Result.bContinueResolution = true;
		P1Result.bRequiresP2 = true;
		P1Result.RunnerId = ShooterCardId;

		FThroughBallBehindDefenseP1PlanQueryResult& PlanResult =
			P1Result.Input.ResolverInputAssemblyResult.Input.PlanQueryResult;
		PlanResult.bSuccess = true;
		PlanResult.Decision =
			EThroughBallBehindDefenseP1PlanQueryDecision
				::FormulaResolutionRequired;
		PlanResult.bHasFormulaPlan = true;
		PlanResult.FormulaPlan.AttackingOwnerId = AttackingOwnerId;
		PlanResult.FormulaPlan.DefendingOwnerId = DefendingOwnerId;
		PlanResult.FormulaPlan.RunnerId = ShooterCardId;
		return Result;
	}

	FThroughBallAntiOffsideOutcomeQueryResult MakeAntiOffsideResult()
	{
		FThroughBallAntiOffsideOutcomeQueryResult Result;
		Result.bSuccess = true;
		Result.Decision =
			EThroughBallAntiOffsideOutcomeDecision::OneOnOneRequired;
		Result.bContinueResolution = true;
		Result.bRequiresOneOnOne = true;
		Result.RunnerId = ShooterCardId;

		FThroughBallBranchSelectionQueryResult& BranchResult =
			Result.Input.BranchSelectionResult;
		BranchResult.bSuccess = true;
		BranchResult.bHasSelectedThroughBallBranch = true;
		BranchResult.SelectedThroughBallBranch =
			EThroughBallSelectedBranch::AntiOffside;

		FThroughBallParticipantEligibilityQueryResult& EligibilityResult =
			Result.Input.ParticipantEligibilityResult;
		EligibilityResult.bSuccess = true;
		EligibilityResult.Input.AttackingOwnerId = AttackingOwnerId;
		EligibilityResult.Input.DefendingOwnerId = DefendingOwnerId;
		EligibilityResult.Input.RunnerSnapshot.CardId = ShooterCardId;
		return Result;
	}

	FThroughBallBehindDefenseP2OutcomeQueryResult MakeP2OffsideResult()
	{
		FThroughBallBehindDefenseP2OutcomeQueryResult Result = MakeP2Result();
		Result.Decision = EThroughBallBehindDefenseP2OutcomeDecision::Offside;
		Result.bAttackEnded = true;
		Result.bContinueResolution = false;
		Result.bRequiresOneOnOne = false;
		Result.RunnerId = NAME_None;
		return Result;
	}

	FThroughBallAntiOffsideOutcomeQueryResult MakeAntiOffsideOffsideResult()
	{
		FThroughBallAntiOffsideOutcomeQueryResult Result =
			MakeAntiOffsideResult();
		Result.Decision = EThroughBallAntiOffsideOutcomeDecision::Offside;
		Result.bAttackEnded = true;
		Result.bContinueResolution = false;
		Result.bRequiresOneOnOne = false;
		Result.RunnerId = NAME_None;
		return Result;
	}

	void ExpectFailure(
		FAutomationTestBase& Test,
		const FThroughBallOneOnOneHandoffCreationResult& Result,
		const EError ErrorCode,
		const FName InvalidField)
	{
		Test.TestFalse(TEXT("Creation fails"), Result.bSuccess);
		Test.TestEqual(TEXT("Failure error"), Result.ErrorCode, ErrorCode);
		Test.TestFalse(
			TEXT("Failure message is populated"),
			Result.ErrorMessage.IsEmpty());
		Test.TestEqual(
			TEXT("Failure invalid field"), Result.InvalidField, InvalidField);
		Test.TestFalse(TEXT("Failure has no Handoff"), Result.bHasHandoff);
		Test.TestTrue(
			TEXT("Failure attacking owner is default"),
			Result.Handoff.AttackingOwnerId.IsNone());
		Test.TestTrue(
			TEXT("Failure defending owner is default"),
			Result.Handoff.DefendingOwnerId.IsNone());
		Test.TestTrue(
			TEXT("Failure shooter is default"),
			Result.Handoff.ShooterCardId.IsNone());
	}

	void ExpectSuccess(
		FAutomationTestBase& Test,
		const FThroughBallOneOnOneHandoffCreationResult& Result,
		const FName ExpectedAttackingOwnerId = AttackingOwnerId,
		const FName ExpectedDefendingOwnerId = DefendingOwnerId,
		const FName ExpectedShooterCardId = ShooterCardId)
	{
		Test.TestTrue(TEXT("Creation succeeds"), Result.bSuccess);
		Test.TestEqual(TEXT("Success error is None"), Result.ErrorCode, EError::None);
		Test.TestTrue(
			TEXT("Success message is empty"), Result.ErrorMessage.IsEmpty());
		Test.TestTrue(
			TEXT("Success invalid field is None"), Result.InvalidField.IsNone());
		Test.TestTrue(TEXT("Success has Handoff"), Result.bHasHandoff);
		Test.TestEqual(
			TEXT("Attacking owner is preserved"),
			Result.Handoff.AttackingOwnerId,
			ExpectedAttackingOwnerId);
		Test.TestEqual(
			TEXT("Defending owner is preserved"),
			Result.Handoff.DefendingOwnerId,
			ExpectedDefendingOwnerId);
		Test.TestEqual(
			TEXT("Shooter is preserved"),
			Result.Handoff.ShooterCardId,
			ExpectedShooterCardId);
	}

	bool RunCase(FAutomationTestBase& Test, const int32 Case)
	{
		switch (Case)
		{
		case 1:
		{
			const FThroughBallOneOnOneHandoffCreationResult Result;
			Test.TestFalse(TEXT("Default Result fails"), Result.bSuccess);
			Test.TestEqual(TEXT("Default error is None"), Result.ErrorCode, EError::None);
			Test.TestTrue(TEXT("Default message is empty"), Result.ErrorMessage.IsEmpty());
			Test.TestTrue(TEXT("Default invalid field is None"), Result.InvalidField.IsNone());
			Test.TestFalse(TEXT("Default Result has no Handoff"), Result.bHasHandoff);
			Test.TestTrue(TEXT("Default attacking owner"), Result.Handoff.AttackingOwnerId.IsNone());
			Test.TestTrue(TEXT("Default defending owner"), Result.Handoff.DefendingOwnerId.IsNone());
			Test.TestTrue(TEXT("Default shooter"), Result.Handoff.ShooterCardId.IsNone());
			break;
		}
		case 2:
			ExpectSuccess(
				Test,
				FThroughBallOneOnOneHandoffCreator::CreateFromBehindDefenseP2(
					MakeP2Result()));
			break;
		case 3:
			ExpectSuccess(
				Test,
				FThroughBallOneOnOneHandoffCreator::CreateFromAntiOffside(
					MakeAntiOffsideResult()));
			break;
		case 4:
		{
			const FName ExactAttack(TEXT("Player.Exact.P2.Attack"));
			const FName ExactDefense(TEXT("Player.Exact.P2.Defense"));
			const FName ExactShooter(TEXT("Card.Exact.P2.Shooter"));
			auto Source = MakeP2Result();
			auto& P1 = Source.Input.P1ExecutionResult;
			auto& Plan =
				P1.Input.ResolverInputAssemblyResult.Input.PlanQueryResult.FormulaPlan;
			Source.RunnerId = ExactShooter;
			P1.RunnerId = ExactShooter;
			Plan.RunnerId = ExactShooter;
			Plan.AttackingOwnerId = ExactAttack;
			Plan.DefendingOwnerId = ExactDefense;
			ExpectSuccess(
				Test,
				FThroughBallOneOnOneHandoffCreator::CreateFromBehindDefenseP2(Source),
				ExactAttack,
				ExactDefense,
				ExactShooter);
			break;
		}
		case 5:
		{
			const FName ExactAttack(TEXT("Player.Exact.Anti.Attack"));
			const FName ExactDefense(TEXT("Player.Exact.Anti.Defense"));
			const FName ExactShooter(TEXT("Card.Exact.Anti.Shooter"));
			auto Source = MakeAntiOffsideResult();
			auto& Eligibility = Source.Input.ParticipantEligibilityResult.Input;
			Source.RunnerId = ExactShooter;
			Eligibility.RunnerSnapshot.CardId = ExactShooter;
			Eligibility.AttackingOwnerId = ExactAttack;
			Eligibility.DefendingOwnerId = ExactDefense;
			ExpectSuccess(
				Test,
				FThroughBallOneOnOneHandoffCreator::CreateFromAntiOffside(Source),
				ExactAttack,
				ExactDefense,
				ExactShooter);
			break;
		}
		case 6:
		{
			auto P2Source = MakeP2Result();
			auto& PlanResult = P2Source.Input.P1ExecutionResult.Input
				.ResolverInputAssemblyResult.Input.PlanQueryResult;
			PlanResult.Input.ParticipantEligibilityResult.Input
				.MarkerSnapshot.CardId = ShooterCardId;
			ExpectSuccess(
				Test,
				FThroughBallOneOnOneHandoffCreator::CreateFromBehindDefenseP2(
					P2Source));

			auto AntiSource = MakeAntiOffsideResult();
			AntiSource.Input.ParticipantEligibilityResult.Input
				.MarkerSnapshot.CardId = ShooterCardId;
			ExpectSuccess(
				Test,
				FThroughBallOneOnOneHandoffCreator::CreateFromAntiOffside(
					AntiSource));
			break;
		}
		case 7:
			ExpectFailure(
				Test,
				FThroughBallOneOnOneHandoffCreator::CreateFromBehindDefenseP2(
					MakeP2OffsideResult()),
				EError::UnsupportedSourceOutcomeDecision,
				TEXT("Decision"));
			break;
		case 8:
			ExpectFailure(
				Test,
				FThroughBallOneOnOneHandoffCreator::CreateFromAntiOffside(
					MakeAntiOffsideOffsideResult()),
				EError::UnsupportedSourceOutcomeDecision,
				TEXT("Decision"));
			break;
		case 9:
		{
			auto Source = MakeP2Result();
			Source.Decision = EThroughBallBehindDefenseP2OutcomeDecision::None;
			ExpectFailure(
				Test,
				FThroughBallOneOnOneHandoffCreator::CreateFromBehindDefenseP2(Source),
				EError::InvalidSourceOutcomeResult,
				TEXT("Decision"));

			Source = MakeP2Result();
			Source.bAttackEnded = true;
			ExpectFailure(
				Test,
				FThroughBallOneOnOneHandoffCreator::CreateFromBehindDefenseP2(Source),
				EError::InvalidSourceOutcomeResult,
				TEXT("bAttackEnded"));

			Source = MakeP2OffsideResult();
			Source.RunnerId = ShooterCardId;
			ExpectFailure(
				Test,
				FThroughBallOneOnOneHandoffCreator::CreateFromBehindDefenseP2(Source),
				EError::InvalidSourceOutcomeResult,
				TEXT("RunnerId"));
			break;
		}
		case 10:
		{
			auto Source = MakeAntiOffsideResult();
			Source.Decision = static_cast<EThroughBallAntiOffsideOutcomeDecision>(255);
			ExpectFailure(
				Test,
				FThroughBallOneOnOneHandoffCreator::CreateFromAntiOffside(Source),
				EError::InvalidSourceOutcomeResult,
				TEXT("Decision"));

			Source = MakeAntiOffsideResult();
			Source.bContinueResolution = false;
			ExpectFailure(
				Test,
				FThroughBallOneOnOneHandoffCreator::CreateFromAntiOffside(Source),
				EError::InvalidSourceOutcomeResult,
				TEXT("bContinueResolution"));

			Source = MakeAntiOffsideOffsideResult();
			Source.RunnerId = ShooterCardId;
			ExpectFailure(
				Test,
				FThroughBallOneOnOneHandoffCreator::CreateFromAntiOffside(Source),
				EError::InvalidSourceOutcomeResult,
				TEXT("RunnerId"));
			break;
		}
		case 11:
		{
			auto Source = MakeP2Result();
			Source.bSuccess = false;
			ExpectFailure(
				Test,
				FThroughBallOneOnOneHandoffCreator::CreateFromBehindDefenseP2(Source),
				EError::SourceOutcomeFailed,
				TEXT("SourceOutcomeResult"));
			break;
		}
		case 12:
		{
			auto Source = MakeAntiOffsideResult();
			Source.bSuccess = false;
			ExpectFailure(
				Test,
				FThroughBallOneOnOneHandoffCreator::CreateFromAntiOffside(Source),
				EError::SourceOutcomeFailed,
				TEXT("SourceOutcomeResult"));
			break;
		}
		case 13:
		{
			auto Source = MakeP2Result();
			Source.ErrorCode =
				EThroughBallBehindDefenseP2OutcomeQueryErrorCode::InvalidP2DefenseD6;
			Source.ErrorMessage = TEXT("Later dirty diagnostic");
			Source.InvalidField = TEXT("LaterDirtyField");
			ExpectFailure(
				Test,
				FThroughBallOneOnOneHandoffCreator::CreateFromBehindDefenseP2(Source),
				EError::InvalidSourceOutcomeResult,
				TEXT("ErrorCode"));

			Source = MakeP2Result();
			Source.ErrorMessage = TEXT("Dirty diagnostic");
			ExpectFailure(
				Test,
				FThroughBallOneOnOneHandoffCreator::CreateFromBehindDefenseP2(Source),
				EError::InvalidSourceOutcomeResult,
				TEXT("ErrorMessage"));

			Source = MakeP2Result();
			Source.InvalidField = TEXT("DirtyField");
			ExpectFailure(
				Test,
				FThroughBallOneOnOneHandoffCreator::CreateFromBehindDefenseP2(Source),
				EError::InvalidSourceOutcomeResult,
				TEXT("InvalidField"));
			break;
		}
		case 14:
		{
			auto Source = MakeAntiOffsideResult();
			Source.ErrorCode =
				EThroughBallAntiOffsideOutcomeQueryErrorCode
					::InvalidAntiOffsideAttackD6;
			Source.ErrorMessage = TEXT("Later dirty diagnostic");
			Source.InvalidField = TEXT("LaterDirtyField");
			ExpectFailure(
				Test,
				FThroughBallOneOnOneHandoffCreator::CreateFromAntiOffside(Source),
				EError::InvalidSourceOutcomeResult,
				TEXT("ErrorCode"));

			Source = MakeAntiOffsideResult();
			Source.ErrorMessage = TEXT("Dirty diagnostic");
			ExpectFailure(
				Test,
				FThroughBallOneOnOneHandoffCreator::CreateFromAntiOffside(Source),
				EError::InvalidSourceOutcomeResult,
				TEXT("ErrorMessage"));

			Source = MakeAntiOffsideResult();
			Source.InvalidField = TEXT("DirtyField");
			ExpectFailure(
				Test,
				FThroughBallOneOnOneHandoffCreator::CreateFromAntiOffside(Source),
				EError::InvalidSourceOutcomeResult,
				TEXT("InvalidField"));
			break;
		}
		case 15:
		{
			auto Source = MakeP2Result();
			Source.Input.P1ExecutionResult.bSuccess = false;
			ExpectFailure(
				Test,
				FThroughBallOneOnOneHandoffCreator::CreateFromBehindDefenseP2(Source),
				EError::InvalidSourceOutcomeResult,
				TEXT("P1ExecutionResult"));

			Source = MakeP2Result();
			Source.Input.P1ExecutionResult.Decision =
				EThroughBallBehindDefenseP1FormulaResolutionExecutionDecision::None;
			ExpectFailure(
				Test,
				FThroughBallOneOnOneHandoffCreator::CreateFromBehindDefenseP2(Source),
				EError::InvalidSourceOutcomeResult,
				TEXT("P1ExecutionResult.Decision"));

			Source = MakeP2Result();
			auto& Plan = Source.Input.P1ExecutionResult.Input
				.ResolverInputAssemblyResult.Input.PlanQueryResult;
			Plan.bSuccess = false;
			ExpectFailure(
				Test,
				FThroughBallOneOnOneHandoffCreator::CreateFromBehindDefenseP2(Source),
				EError::InvalidSourceOutcomeResult,
				TEXT("P1ExecutionResult.PlanQueryResult"));

			Source = MakeP2Result();
			Source.Input.P1ExecutionResult.Input.ResolverInputAssemblyResult.Input
				.PlanQueryResult.bHasFormulaPlan = false;
			ExpectFailure(
				Test,
				FThroughBallOneOnOneHandoffCreator::CreateFromBehindDefenseP2(Source),
				EError::InvalidSourceOutcomeResult,
				TEXT("P1ExecutionResult.PlanQueryResult.bHasFormulaPlan"));
			break;
		}
		case 16:
		{
			auto Source = MakeAntiOffsideResult();
			Source.Input.BranchSelectionResult.bSuccess = false;
			ExpectFailure(
				Test,
				FThroughBallOneOnOneHandoffCreator::CreateFromAntiOffside(Source),
				EError::InvalidSourceOutcomeResult,
				TEXT("BranchSelectionResult"));

			Source = MakeAntiOffsideResult();
			Source.Input.BranchSelectionResult.SelectedThroughBallBranch =
				EThroughBallSelectedBranch::Feet;
			ExpectFailure(
				Test,
				FThroughBallOneOnOneHandoffCreator::CreateFromAntiOffside(Source),
				EError::InvalidSourceOutcomeResult,
				TEXT("BranchSelectionResult.SelectedThroughBallBranch"));

			Source = MakeAntiOffsideResult();
			Source.Input.ParticipantEligibilityResult.bSuccess = false;
			ExpectFailure(
				Test,
				FThroughBallOneOnOneHandoffCreator::CreateFromAntiOffside(Source),
				EError::InvalidSourceOutcomeResult,
				TEXT("ParticipantEligibilityResult"));
			break;
		}
		case 17:
		{
			auto Source = MakeP2Result();
			auto& Plan = Source.Input.P1ExecutionResult.Input
				.ResolverInputAssemblyResult.Input.PlanQueryResult.FormulaPlan;
			Plan.AttackingOwnerId = NAME_None;
			ExpectFailure(
				Test,
				FThroughBallOneOnOneHandoffCreator::CreateFromBehindDefenseP2(Source),
				EError::InvalidAttackingOwnerIdentity,
				TEXT("AttackingOwnerId"));

			Source = MakeP2Result();
			auto& MissingDefensePlan = Source.Input.P1ExecutionResult.Input
				.ResolverInputAssemblyResult.Input.PlanQueryResult.FormulaPlan;
			MissingDefensePlan.DefendingOwnerId = NAME_None;
			ExpectFailure(
				Test,
				FThroughBallOneOnOneHandoffCreator::CreateFromBehindDefenseP2(Source),
				EError::InvalidDefendingOwnerIdentity,
				TEXT("DefendingOwnerId"));

			Source = MakeP2Result();
			auto& DuplicatePlan = Source.Input.P1ExecutionResult.Input
				.ResolverInputAssemblyResult.Input.PlanQueryResult.FormulaPlan;
			DuplicatePlan.DefendingOwnerId = DuplicatePlan.AttackingOwnerId;
			ExpectFailure(
				Test,
				FThroughBallOneOnOneHandoffCreator::CreateFromBehindDefenseP2(Source),
				EError::DuplicateOwnerIdentity,
				TEXT("DefendingOwnerId"));
			break;
		}
		case 18:
		{
			auto Source = MakeAntiOffsideResult();
			Source.Input.ParticipantEligibilityResult.Input.AttackingOwnerId =
				NAME_None;
			ExpectFailure(
				Test,
				FThroughBallOneOnOneHandoffCreator::CreateFromAntiOffside(Source),
				EError::InvalidAttackingOwnerIdentity,
				TEXT("AttackingOwnerId"));

			Source = MakeAntiOffsideResult();
			Source.Input.ParticipantEligibilityResult.Input.DefendingOwnerId =
				NAME_None;
			ExpectFailure(
				Test,
				FThroughBallOneOnOneHandoffCreator::CreateFromAntiOffside(Source),
				EError::InvalidDefendingOwnerIdentity,
				TEXT("DefendingOwnerId"));

			Source = MakeAntiOffsideResult();
			auto& Eligibility =
				Source.Input.ParticipantEligibilityResult.Input;
			Eligibility.DefendingOwnerId = Eligibility.AttackingOwnerId;
			ExpectFailure(
				Test,
				FThroughBallOneOnOneHandoffCreator::CreateFromAntiOffside(Source),
				EError::DuplicateOwnerIdentity,
				TEXT("DefendingOwnerId"));
			break;
		}
		case 19:
		{
			auto Source = MakeP2Result();
			Source.RunnerId = NAME_None;
			ExpectFailure(
				Test,
				FThroughBallOneOnOneHandoffCreator::CreateFromBehindDefenseP2(Source),
				EError::InvalidShooterIdentity,
				TEXT("ShooterCardId"));

			Source = MakeP2Result();
			Source.Input.P1ExecutionResult.RunnerId = NAME_None;
			ExpectFailure(
				Test,
				FThroughBallOneOnOneHandoffCreator::CreateFromBehindDefenseP2(Source),
				EError::InvalidShooterIdentity,
				TEXT("ShooterCardId"));

			Source = MakeP2Result();
			Source.Input.P1ExecutionResult.Input.ResolverInputAssemblyResult.Input
				.PlanQueryResult.FormulaPlan.RunnerId = NAME_None;
			ExpectFailure(
				Test,
				FThroughBallOneOnOneHandoffCreator::CreateFromBehindDefenseP2(Source),
				EError::InvalidShooterIdentity,
				TEXT("ShooterCardId"));

			Source = MakeP2Result();
			Source.Input.P1ExecutionResult.RunnerId = TEXT("Card.Other.Shooter");
			ExpectFailure(
				Test,
				FThroughBallOneOnOneHandoffCreator::CreateFromBehindDefenseP2(Source),
				EError::InconsistentShooterIdentity,
				TEXT("ShooterCardId"));
			break;
		}
		case 20:
		{
			auto Source = MakeAntiOffsideResult();
			Source.RunnerId = NAME_None;
			ExpectFailure(
				Test,
				FThroughBallOneOnOneHandoffCreator::CreateFromAntiOffside(Source),
				EError::InvalidShooterIdentity,
				TEXT("ShooterCardId"));

			Source = MakeAntiOffsideResult();
			Source.Input.ParticipantEligibilityResult.Input.RunnerSnapshot.CardId =
				NAME_None;
			ExpectFailure(
				Test,
				FThroughBallOneOnOneHandoffCreator::CreateFromAntiOffside(Source),
				EError::InvalidShooterIdentity,
				TEXT("ShooterCardId"));

			Source = MakeAntiOffsideResult();
			Source.Input.ParticipantEligibilityResult.Input.RunnerSnapshot.CardId =
				TEXT("Card.Other.Shooter");
			ExpectFailure(
				Test,
				FThroughBallOneOnOneHandoffCreator::CreateFromAntiOffside(Source),
				EError::InconsistentShooterIdentity,
				TEXT("ShooterCardId"));
			break;
		}
		case 21:
		{
			const auto P2Source = MakeP2Result();
			const auto AntiSource = MakeAntiOffsideResult();
			const FName P2RunnerBefore = P2Source.RunnerId;
			const FName P1RunnerBefore = P2Source.Input.P1ExecutionResult.RunnerId;
			const FName AntiRunnerBefore = AntiSource.RunnerId;
			const FName EligibilityRunnerBefore = AntiSource.Input
				.ParticipantEligibilityResult.Input.RunnerSnapshot.CardId;

			const auto P2First =
				FThroughBallOneOnOneHandoffCreator::CreateFromBehindDefenseP2(
					P2Source);
			const auto P2Second =
				FThroughBallOneOnOneHandoffCreator::CreateFromBehindDefenseP2(
					P2Source);
			const auto AntiFirst =
				FThroughBallOneOnOneHandoffCreator::CreateFromAntiOffside(
					AntiSource);
			const auto AntiSecond =
				FThroughBallOneOnOneHandoffCreator::CreateFromAntiOffside(
					AntiSource);

			Test.TestEqual(TEXT("P2 success deterministic"), P2First.bSuccess, P2Second.bSuccess);
			Test.TestEqual(TEXT("P2 error deterministic"), P2First.ErrorCode, P2Second.ErrorCode);
			Test.TestEqual(TEXT("P2 message deterministic"), P2First.ErrorMessage, P2Second.ErrorMessage);
			Test.TestEqual(TEXT("P2 Handoff deterministic"), P2First.Handoff.ShooterCardId, P2Second.Handoff.ShooterCardId);
			Test.TestEqual(TEXT("Anti success deterministic"), AntiFirst.bSuccess, AntiSecond.bSuccess);
			Test.TestEqual(TEXT("Anti error deterministic"), AntiFirst.ErrorCode, AntiSecond.ErrorCode);
			Test.TestEqual(TEXT("Anti message deterministic"), AntiFirst.ErrorMessage, AntiSecond.ErrorMessage);
			Test.TestEqual(TEXT("Anti Handoff deterministic"), AntiFirst.Handoff.ShooterCardId, AntiSecond.Handoff.ShooterCardId);
			Test.TestEqual(TEXT("P2 top Runner is unchanged"), P2Source.RunnerId, P2RunnerBefore);
			Test.TestEqual(TEXT("P2 nested Runner is unchanged"), P2Source.Input.P1ExecutionResult.RunnerId, P1RunnerBefore);
			Test.TestEqual(TEXT("Anti top Runner is unchanged"), AntiSource.RunnerId, AntiRunnerBefore);
			Test.TestEqual(TEXT("Eligibility Runner is unchanged"), AntiSource.Input.ParticipantEligibilityResult.Input.RunnerSnapshot.CardId, EligibilityRunnerBefore);
			break;
		}
		case 22:
		{
			using P2Signature = FThroughBallOneOnOneHandoffCreationResult (*)(
				const FThroughBallBehindDefenseP2OutcomeQueryResult&);
			using AntiSignature = FThroughBallOneOnOneHandoffCreationResult (*)(
				const FThroughBallAntiOffsideOutcomeQueryResult&);
			static_assert(std::is_same_v<
				decltype(&FThroughBallOneOnOneHandoffCreator
					::CreateFromBehindDefenseP2),
				P2Signature>);
			static_assert(std::is_same_v<
				decltype(&FThroughBallOneOnOneHandoffCreator::CreateFromAntiOffside),
				AntiSignature>);
			static_assert(std::is_standard_layout_v<FThroughBallOneOnOneHandoff>);

			const auto P2Result =
				FThroughBallOneOnOneHandoffCreator::CreateFromBehindDefenseP2(
					MakeP2Result());
			const auto AntiResult =
				FThroughBallOneOnOneHandoffCreator::CreateFromAntiOffside(
					MakeAntiOffsideResult());
			Test.TestTrue(TEXT("Formal P2 surface creates"), P2Result.bSuccess);
			Test.TestTrue(TEXT("Formal Anti surface creates"), AntiResult.bSuccess);
			Test.TestTrue(TEXT("P2 Handoff has attacking owner"), !P2Result.Handoff.AttackingOwnerId.IsNone());
			Test.TestTrue(TEXT("P2 Handoff has defending owner"), !P2Result.Handoff.DefendingOwnerId.IsNone());
			Test.TestTrue(TEXT("P2 Handoff has shooter"), !P2Result.Handoff.ShooterCardId.IsNone());
			break;
		}
		default:
			Test.AddError(TEXT("Unknown Handoff Creator test case."));
			return false;
		}

		return true;
	}
}

#define THROUGH_BALL_ONE_ON_ONE_HANDOFF_CREATOR_TEST(Number, Name) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
		FThroughBallOneOnOneHandoffCreatorTest##Number, \
		"FMCodex.CoreRules.ThroughBallOneOnOneHandoffCreator." Name, \
		EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter) \
	bool FThroughBallOneOnOneHandoffCreatorTest##Number::RunTest( \
		const FString& Parameters) \
	{ \
		return ThroughBallOneOnOneHandoffCreatorTests::RunCase(*this, Number); \
	}

THROUGH_BALL_ONE_ON_ONE_HANDOFF_CREATOR_TEST(1, "01DefaultCreationResultIsFailureSafe")
THROUGH_BALL_ONE_ON_ONE_HANDOFF_CREATOR_TEST(2, "02ValidBehindDefenseP2OneOnOneCreatesHandoff")
THROUGH_BALL_ONE_ON_ONE_HANDOFF_CREATOR_TEST(3, "03ValidAntiOffsideOneOnOneCreatesHandoff")
THROUGH_BALL_ONE_ON_ONE_HANDOFF_CREATOR_TEST(4, "04BehindDefenseP2PreservesExactIdentities")
THROUGH_BALL_ONE_ON_ONE_HANDOFF_CREATOR_TEST(5, "05AntiOffsidePreservesExactIdentities")
THROUGH_BALL_ONE_ON_ONE_HANDOFF_CREATOR_TEST(6, "06CrossSideDuplicateRawCardIdRemainsUnambiguous")
THROUGH_BALL_ONE_ON_ONE_HANDOFF_CREATOR_TEST(7, "07BehindDefenseP2OffsideIsRejected")
THROUGH_BALL_ONE_ON_ONE_HANDOFF_CREATOR_TEST(8, "08AntiOffsideOffsideIsRejected")
THROUGH_BALL_ONE_ON_ONE_HANDOFF_CREATOR_TEST(9, "09BehindDefenseP2InvalidDecisionMetadataIsRejected")
THROUGH_BALL_ONE_ON_ONE_HANDOFF_CREATOR_TEST(10, "10AntiOffsideInvalidDecisionMetadataIsRejected")
THROUGH_BALL_ONE_ON_ONE_HANDOFF_CREATOR_TEST(11, "11FailedBehindDefenseP2ResultWithResidualRunnerIsRejected")
THROUGH_BALL_ONE_ON_ONE_HANDOFF_CREATOR_TEST(12, "12FailedAntiOffsideResultWithResidualRunnerIsRejected")
THROUGH_BALL_ONE_ON_ONE_HANDOFF_CREATOR_TEST(13, "13SuccessfulBehindDefenseP2ResultWithDirtyDiagnosticsIsRejected")
THROUGH_BALL_ONE_ON_ONE_HANDOFF_CREATOR_TEST(14, "14SuccessfulAntiOffsideResultWithDirtyDiagnosticsIsRejected")
THROUGH_BALL_ONE_ON_ONE_HANDOFF_CREATOR_TEST(15, "15InvalidBehindDefenseP2NestedProvenanceIsRejected")
THROUGH_BALL_ONE_ON_ONE_HANDOFF_CREATOR_TEST(16, "16InvalidAntiOffsideNestedProvenanceIsRejected")
THROUGH_BALL_ONE_ON_ONE_HANDOFF_CREATOR_TEST(17, "17BehindDefenseP2OwnerIdentityErrorsArePrecise")
THROUGH_BALL_ONE_ON_ONE_HANDOFF_CREATOR_TEST(18, "18AntiOffsideOwnerIdentityErrorsArePrecise")
THROUGH_BALL_ONE_ON_ONE_HANDOFF_CREATOR_TEST(19, "19BehindDefenseP2ShooterIdentityErrorsArePrecise")
THROUGH_BALL_ONE_ON_ONE_HANDOFF_CREATOR_TEST(20, "20AntiOffsideShooterIdentityErrorsArePrecise")
THROUGH_BALL_ONE_ON_ONE_HANDOFF_CREATOR_TEST(21, "21CreationIsDeterministicAndDoesNotMutateSources")
THROUGH_BALL_ONE_ON_ONE_HANDOFF_CREATOR_TEST(22, "22PublicCreatorSurfaceSupportsOnlyFormalSourceResults")

#undef THROUGH_BALL_ONE_ON_ONE_HANDOFF_CREATOR_TEST

#endif
