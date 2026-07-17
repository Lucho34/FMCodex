#if WITH_DEV_AUTOMATION_TESTS

#include "ThroughBallAntiOffsideOutcomeQuery.h"

#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace ThroughBallAntiOffsideOutcomeQueryTests
{
	const FName SkillId(TEXT("Skill.ThroughBall.AntiOffside"));
	const FName CarrierId(TEXT("ThroughBall.AntiOffside.Carrier"));
	const FName RunnerId(TEXT("ThroughBall.AntiOffside.Runner"));
	const FName MarkerId(TEXT("ThroughBall.AntiOffside.Marker"));
	const FName HelperId(TEXT("ThroughBall.AntiOffside.Helper"));
	const FName AttackingOwnerId(TEXT("Player.AntiOffside.Attacking"));
	const FName DefendingOwnerId(TEXT("Player.AntiOffside.Defending"));

	using EDecision = EThroughBallAntiOffsideOutcomeDecision;
	using EError = EThroughBallAntiOffsideOutcomeQueryErrorCode;

	FPlayerCardRuleSnapshot MakeOutfield(
		const FName CardId,
		const EPlayerPositionType Position)
	{
		FPlayerCardRuleSnapshot Snapshot;
		Snapshot.CardId = CardId;
		Snapshot.PositionTypes = {Position};
		return Snapshot;
	}

	FSkillRuleSnapshotSet MakeSkillRules()
	{
		FSkillRuleSnapshot Rule;
		Rule.SkillId = SkillId;
		Rule.SkillType = ESkillRuleType::ThroughBall;
		Rule.MinTriggerActionPoint = 2;
		Rule.MaxTriggerActionPoint = 8;

		FSkillRuleSnapshotSet Rules;
		Rules.SkillRules = {Rule};
		return Rules;
	}

	FThroughBallBranchSelectionQueryResult MakeBranchResult(
		const int32 SelectionD6 = 5)
	{
		FThroughBallBranchSelectionQueryInput Input;
		Input.bHasExternalSelectionD6 = true;
		Input.ExternalSelectionD6 = SelectionD6;
		return FThroughBallBranchSelectionQuery::Select(Input);
	}

	FThroughBallParticipantEligibilityQueryResult MakeEligibilityResult(
		const bool bHasHelper = false,
		const bool bUseCrossSideDuplicateCardId = false)
	{
		FThroughBallParticipantEligibilityQueryInput Input;
		Input.SelectedSkillId = SkillId;
		Input.CurrentActionPoint = 4;
		Input.AttackingOwnerId = AttackingOwnerId;
		Input.DefendingOwnerId = DefendingOwnerId;
		Input.CarrierSnapshot = MakeOutfield(
			CarrierId,
			EPlayerPositionType::Midfield);
		Input.CarrierSnapshot.SkillIds = {SkillId};
		Input.RunnerSnapshot = MakeOutfield(
			RunnerId,
			EPlayerPositionType::Attack);
		Input.MarkerSnapshot = MakeOutfield(
			bUseCrossSideDuplicateCardId ? CarrierId : MarkerId,
			EPlayerPositionType::Defense);
		Input.bHasHelper = bHasHelper;
		if (bHasHelper)
		{
			Input.HelperSnapshot = MakeOutfield(
				HelperId,
				EPlayerPositionType::Midfield);
		}
		Input.bIsRunnerInAttackingForwardArea = true;
		return FThroughBallParticipantEligibilityQuery::Evaluate(
			MakeSkillRules(),
			Input);
	}

	FThroughBallAntiOffsideOutcomeQueryInput MakeInput(
		const int32 AntiOffsideAttackD6 = 1,
		const int32 BranchSelectionD6 = 5,
		const bool bHasHelper = false)
	{
		FThroughBallAntiOffsideOutcomeQueryInput Input;
		Input.BranchSelectionResult = MakeBranchResult(BranchSelectionD6);
		Input.ParticipantEligibilityResult =
			MakeEligibilityResult(bHasHelper);
		Input.bHasAntiOffsideAttackD6 = true;
		Input.AntiOffsideAttackD6 = AntiOffsideAttackD6;
		return Input;
	}

	void ExpectFailureDefaults(
		FAutomationTestBase& Test,
		const FThroughBallAntiOffsideOutcomeQueryResult& Result,
		const EError ErrorCode,
		const FName InvalidField)
	{
		Test.TestFalse(TEXT("Failure is not successful"), Result.bSuccess);
		Test.TestEqual(TEXT("Failure error"), Result.ErrorCode, ErrorCode);
		Test.TestFalse(
			TEXT("Failure message is populated"),
			Result.ErrorMessage.IsEmpty());
		Test.TestEqual(
			TEXT("Failure invalid field"),
			Result.InvalidField,
			InvalidField);
		Test.TestEqual(
			TEXT("Failure decision is None"),
			Result.Decision,
			EDecision::None);
		Test.TestFalse(
			TEXT("Failure does not end attack"),
			Result.bAttackEnded);
		Test.TestFalse(
			TEXT("Failure does not continue"),
			Result.bContinueResolution);
		Test.TestFalse(
			TEXT("Failure does not require One-on-One"),
			Result.bRequiresOneOnOne);
		Test.TestTrue(
			TEXT("Failure RunnerId is default"),
			Result.RunnerId.IsNone());
	}

	FThroughBallAntiOffsideOutcomeQueryResult ExpectFailure(
		FAutomationTestBase& Test,
		const FThroughBallAntiOffsideOutcomeQueryInput& Input,
		const EError ErrorCode,
		const FName InvalidField)
	{
		const FThroughBallAntiOffsideOutcomeQueryResult Result =
			FThroughBallAntiOffsideOutcomeQuery::Evaluate(Input);
		ExpectFailureDefaults(Test, Result, ErrorCode, InvalidField);
		return Result;
	}

	FThroughBallAntiOffsideOutcomeQueryResult ExpectSuccess(
		FAutomationTestBase& Test,
		const FThroughBallAntiOffsideOutcomeQueryInput& Input)
	{
		const FThroughBallAntiOffsideOutcomeQueryResult Result =
			FThroughBallAntiOffsideOutcomeQuery::Evaluate(Input);
		Test.TestTrue(TEXT("Outcome succeeds"), Result.bSuccess);
		Test.TestEqual(TEXT("Success error is None"), Result.ErrorCode, EError::None);
		Test.TestTrue(
			TEXT("Success message is empty"),
			Result.ErrorMessage.IsEmpty());
		Test.TestTrue(
			TEXT("Success InvalidField is None"),
			Result.InvalidField.IsNone());
		return Result;
	}

	void ExpectOffside(
		FAutomationTestBase& Test,
		const FThroughBallAntiOffsideOutcomeQueryResult& Result)
	{
		Test.TestEqual(TEXT("Offside decision"), Result.Decision, EDecision::Offside);
		Test.TestTrue(TEXT("Offside ends attack"), Result.bAttackEnded);
		Test.TestFalse(
			TEXT("Offside does not continue"),
			Result.bContinueResolution);
		Test.TestFalse(
			TEXT("Offside does not require One-on-One"),
			Result.bRequiresOneOnOne);
		Test.TestTrue(
			TEXT("Offside RunnerId remains default"),
			Result.RunnerId.IsNone());
	}

	void ExpectOneOnOne(
		FAutomationTestBase& Test,
		const FThroughBallAntiOffsideOutcomeQueryResult& Result)
	{
		Test.TestEqual(
			TEXT("One-on-One decision"),
			Result.Decision,
			EDecision::OneOnOneRequired);
		Test.TestFalse(
			TEXT("One-on-One does not end attack"),
			Result.bAttackEnded);
		Test.TestTrue(
			TEXT("One-on-One continues resolution"),
			Result.bContinueResolution);
		Test.TestTrue(
			TEXT("One-on-One is required"),
			Result.bRequiresOneOnOne);
		Test.TestEqual(
			TEXT("One-on-One preserves RunnerId"),
			Result.RunnerId,
			RunnerId);
	}

	bool RunCase(FAutomationTestBase& Test, const int32 Case)
	{
		switch (Case)
		{
		case 1:
		{
			const FThroughBallAntiOffsideOutcomeQueryInput Input;
			const auto Result = ExpectFailure(
				Test,
				Input,
				EError::BranchSelectionFailed,
				TEXT("BranchSelectionResult"));
			Test.TestFalse(
				TEXT("Default D6 presence is preserved"),
				Result.Input.bHasAntiOffsideAttackD6);
			Test.TestEqual(
				TEXT("Default D6 is preserved"),
				Result.Input.AntiOffsideAttackD6,
				0);
			break;
		}
		case 2:
		{
			const FThroughBallAntiOffsideOutcomeQueryResult Result;
			Test.TestFalse(TEXT("Default Result fails"), Result.bSuccess);
			Test.TestEqual(TEXT("Default error"), Result.ErrorCode, EError::None);
			Test.TestEqual(TEXT("Default decision"), Result.Decision, EDecision::None);
			Test.TestFalse(TEXT("Default attack-ended"), Result.bAttackEnded);
			Test.TestFalse(TEXT("Default continue"), Result.bContinueResolution);
			Test.TestFalse(TEXT("Default One-on-One"), Result.bRequiresOneOnOne);
			Test.TestTrue(TEXT("Default RunnerId"), Result.RunnerId.IsNone());
			break;
		}
		case 3:
			ExpectOffside(Test, ExpectSuccess(Test, MakeInput(1)));
			break;
		case 4:
			ExpectOffside(Test, ExpectSuccess(Test, MakeInput(5)));
			break;
		case 5:
			ExpectOneOnOne(Test, ExpectSuccess(Test, MakeInput(6)));
			break;
		case 6:
		{
			const auto Input = MakeInput(6, 6, true);
			const auto Result = ExpectSuccess(Test, Input);
			Test.TestTrue(
				TEXT("Selected Branch success preserved"),
				Result.Input.BranchSelectionResult.bSuccess);
			Test.TestEqual(
				TEXT("Selected Branch preserved"),
				Result.Input.BranchSelectionResult.SelectedThroughBallBranch,
				EThroughBallSelectedBranch::AntiOffside);
			Test.TestEqual(
				TEXT("Selected Branch D6 preserved"),
				Result.Input.BranchSelectionResult.Input.ExternalSelectionD6,
				6);
			Test.TestTrue(
				TEXT("Selected Eligibility success preserved"),
				Result.Input.ParticipantEligibilityResult.bSuccess);
			Test.TestEqual(
				TEXT("Selected attacking owner preserved"),
				Result.Input.ParticipantEligibilityResult.Input.AttackingOwnerId,
				AttackingOwnerId);
			Test.TestEqual(
				TEXT("Selected defending owner preserved"),
				Result.Input.ParticipantEligibilityResult.Input.DefendingOwnerId,
				DefendingOwnerId);
			Test.TestEqual(
				TEXT("Selected Runner preserved"),
				Result.Input.ParticipantEligibilityResult.Input.RunnerSnapshot.CardId,
				RunnerId);
			Test.TestTrue(
				TEXT("Selected Anti-Offside D6 presence preserved"),
				Result.Input.bHasAntiOffsideAttackD6);
			Test.TestEqual(
				TEXT("Selected Anti-Offside D6 preserved"),
				Result.Input.AntiOffsideAttackD6,
				6);
			break;
		}
		case 7:
		{
			auto Input = MakeInput();
			Input.BranchSelectionResult.bSuccess = false;
			Input.BranchSelectionResult.ErrorCode =
				EThroughBallBranchSelectionQueryErrorCode::InvalidSelectionD6;
			Input.BranchSelectionResult.ErrorMessage = TEXT("Upstream branch error");
			Input.BranchSelectionResult.InvalidField = TEXT("ExternalSelectionD6");
			const auto Result = ExpectFailure(
				Test,
				Input,
				EError::BranchSelectionFailed,
				TEXT("BranchSelectionResult"));
			Test.TestNotEqual(
				TEXT("Upstream branch error message is not copied"),
				Result.ErrorMessage,
				Input.BranchSelectionResult.ErrorMessage);
			break;
		}
		case 8:
		{
			auto Input = MakeInput();
			Input.BranchSelectionResult.ErrorCode =
				EThroughBallBranchSelectionQueryErrorCode::InvalidSelectionD6;
			ExpectFailure(
				Test, Input, EError::InvalidBranchSelectionResult,
				TEXT("BranchSelectionResult.ErrorCode"));
			break;
		}
		case 9:
		{
			auto Input = MakeInput();
			Input.BranchSelectionResult.ErrorMessage = TEXT("Forged");
			ExpectFailure(
				Test, Input, EError::InvalidBranchSelectionResult,
				TEXT("BranchSelectionResult.ErrorMessage"));
			break;
		}
		case 10:
		{
			auto Input = MakeInput();
			Input.BranchSelectionResult.InvalidField = TEXT("ForgedField");
			ExpectFailure(
				Test, Input, EError::InvalidBranchSelectionResult,
				TEXT("BranchSelectionResult.InvalidField"));
			break;
		}
		case 11:
		{
			auto Input = MakeInput();
			Input.BranchSelectionResult.bHasSelectedThroughBallBranch = false;
			ExpectFailure(
				Test, Input, EError::InvalidBranchSelectionResult,
				TEXT("BranchSelectionResult.bHasSelectedThroughBallBranch"));
			break;
		}
		case 12:
		{
			auto Input = MakeInput();
			Input.BranchSelectionResult.Input.bHasExternalSelectionD6 = false;
			ExpectFailure(
				Test, Input, EError::InvalidBranchSelectionResult,
				TEXT("BranchSelectionResult.Input.bHasExternalSelectionD6"));
			break;
		}
		case 13:
		{
			auto Input = MakeInput();
			Input.BranchSelectionResult.Input.ExternalSelectionD6 = 0;
			ExpectFailure(
				Test, Input, EError::InvalidBranchSelectionResult,
				TEXT("BranchSelectionResult.Input.ExternalSelectionD6"));
			break;
		}
		case 14:
		{
			auto Input = MakeInput();
			Input.BranchSelectionResult.SelectedThroughBallBranch =
				EThroughBallSelectedBranch::BehindDefense;
			ExpectFailure(
				Test, Input, EError::InvalidBranchSelectionResult,
				TEXT("BranchSelectionResult.SelectedThroughBallBranch"));
			break;
		}
		case 15:
		{
			const auto Input = MakeInput(1, 1);
			ExpectFailure(
				Test, Input, EError::UnsupportedBranch,
				TEXT("BranchSelectionResult.SelectedThroughBallBranch"));
			break;
		}
		case 16:
		{
			const auto Input = MakeInput(1, 3);
			ExpectFailure(
				Test, Input, EError::UnsupportedBranch,
				TEXT("BranchSelectionResult.SelectedThroughBallBranch"));
			break;
		}
		case 17:
		{
			auto Input = MakeInput();
			Input.ParticipantEligibilityResult.bSuccess = false;
			Input.ParticipantEligibilityResult.ErrorCode =
				EThroughBallParticipantEligibilityQueryErrorCode
					::InvalidRunnerIdentity;
			Input.ParticipantEligibilityResult.ErrorMessage =
				TEXT("Upstream eligibility error");
			Input.ParticipantEligibilityResult.InvalidField = TEXT("RunnerSnapshot");
			const auto Result = ExpectFailure(
				Test,
				Input,
				EError::ParticipantEligibilityFailed,
				TEXT("ParticipantEligibilityResult"));
			Test.TestNotEqual(
				TEXT("Upstream eligibility error message is not copied"),
				Result.ErrorMessage,
				Input.ParticipantEligibilityResult.ErrorMessage);
			break;
		}
		case 18:
		{
			auto ErrorCodeInput = MakeInput();
			ErrorCodeInput.ParticipantEligibilityResult.ErrorCode =
				EThroughBallParticipantEligibilityQueryErrorCode
					::InvalidRunnerIdentity;
			ExpectFailure(
				Test, ErrorCodeInput, EError::InvalidParticipantEligibilityResult,
				TEXT("ParticipantEligibilityResult.ErrorCode"));

			auto MessageInput = MakeInput();
			MessageInput.ParticipantEligibilityResult.ErrorMessage = TEXT("Forged");
			ExpectFailure(
				Test, MessageInput, EError::InvalidParticipantEligibilityResult,
				TEXT("ParticipantEligibilityResult.ErrorMessage"));

			auto FieldInput = MakeInput();
			FieldInput.ParticipantEligibilityResult.InvalidField = TEXT("ForgedField");
			ExpectFailure(
				Test, FieldInput, EError::InvalidParticipantEligibilityResult,
				TEXT("ParticipantEligibilityResult.InvalidField"));
			break;
		}
		case 19:
		{
			auto Input = MakeInput();
			Input.ParticipantEligibilityResult.bHasHelper = true;
			ExpectFailure(
				Test, Input, EError::InvalidParticipantEligibilityResult,
				TEXT("ParticipantEligibilityResult.bHasHelper"));
			break;
		}
		case 20:
		{
			auto SkillInput = MakeInput();
			SkillInput.ParticipantEligibilityResult.SkillRuleQueryResult.bSuccess = false;
			ExpectFailure(
				Test, SkillInput, EError::InvalidParticipantEligibilityResult,
				TEXT("ParticipantEligibilityResult.SkillRuleQueryResult"));

			auto CarrierInput = MakeInput();
			CarrierInput.ParticipantEligibilityResult
				.CarrierSnapshotValidationResult.bSuccess = false;
			ExpectFailure(
				Test, CarrierInput, EError::InvalidParticipantEligibilityResult,
				TEXT("ParticipantEligibilityResult.CarrierSnapshotValidationResult"));

			auto RunnerInput = MakeInput();
			RunnerInput.ParticipantEligibilityResult
				.RunnerSnapshotValidationResult.bSuccess = false;
			ExpectFailure(
				Test, RunnerInput, EError::InvalidParticipantEligibilityResult,
				TEXT("ParticipantEligibilityResult.RunnerSnapshotValidationResult"));

			auto MarkerInput = MakeInput();
			MarkerInput.ParticipantEligibilityResult
				.MarkerSnapshotValidationResult.bSuccess = false;
			ExpectFailure(
				Test, MarkerInput, EError::InvalidParticipantEligibilityResult,
				TEXT("ParticipantEligibilityResult.MarkerSnapshotValidationResult"));

			auto HelperInput = MakeInput(1, 5, true);
			HelperInput.ParticipantEligibilityResult
				.HelperSnapshotValidationResult.bSuccess = false;
			ExpectFailure(
				Test, HelperInput, EError::InvalidParticipantEligibilityResult,
				TEXT("ParticipantEligibilityResult.HelperSnapshotValidationResult"));
			break;
		}
		case 21:
		{
			auto Input = MakeInput();
			Input.ParticipantEligibilityResult.Input.AttackingOwnerId = NAME_None;
			ExpectFailure(
				Test, Input, EError::InvalidParticipantEligibilityResult,
				TEXT("ParticipantEligibilityResult.Input.AttackingOwnerId"));
			break;
		}
		case 22:
		{
			auto Input = MakeInput();
			Input.ParticipantEligibilityResult.Input.DefendingOwnerId = NAME_None;
			ExpectFailure(
				Test, Input, EError::InvalidParticipantEligibilityResult,
				TEXT("ParticipantEligibilityResult.Input.DefendingOwnerId"));
			break;
		}
		case 23:
		{
			auto Input = MakeInput();
			Input.ParticipantEligibilityResult.Input.DefendingOwnerId =
				AttackingOwnerId;
			ExpectFailure(
				Test, Input, EError::InvalidParticipantEligibilityResult,
				TEXT("ParticipantEligibilityResult.Input.DefendingOwnerId"));
			break;
		}
		case 24:
		{
			auto Input = MakeInput();
			Input.ParticipantEligibilityResult.Input.RunnerSnapshot.CardId = NAME_None;
			ExpectFailure(
				Test, Input, EError::InvalidParticipantEligibilityResult,
				TEXT("ParticipantEligibilityResult.Input.RunnerSnapshot.CardId"));
			break;
		}
		case 25:
		{
			auto InvalidInput = MakeInput();
			InvalidInput.ParticipantEligibilityResult
				.RunnerSnapshotValidationResult.bIsValid = false;
			ExpectFailure(
				Test, InvalidInput, EError::InvalidParticipantEligibilityResult,
				TEXT("ParticipantEligibilityResult.RunnerSnapshotValidationResult"));

			auto DiagnosticInput = MakeInput();
			DiagnosticInput.ParticipantEligibilityResult
				.RunnerSnapshotValidationResult.ErrorCode =
				EPlayerCardRuleSnapshotValidationErrorCode::InvalidCardId;
			ExpectFailure(
				Test, DiagnosticInput, EError::InvalidParticipantEligibilityResult,
				TEXT("ParticipantEligibilityResult.RunnerSnapshotValidationResult"));
			break;
		}
		case 26:
		{
			auto Input = MakeInput();
			Input.ParticipantEligibilityResult.Input.RunnerSnapshot.bIsGoalkeeper = true;
			ExpectFailure(
				Test, Input, EError::InvalidParticipantEligibilityResult,
				TEXT("ParticipantEligibilityResult.Input.RunnerSnapshot"));
			break;
		}
		case 27:
		{
			auto Input = MakeInput();
			Input.ParticipantEligibilityResult.Input
				.bIsRunnerInAttackingForwardArea = false;
			ExpectFailure(
				Test, Input, EError::InvalidParticipantEligibilityResult,
				TEXT("ParticipantEligibilityResult.Input.bIsRunnerInAttackingForwardArea"));
			break;
		}
		case 28:
		{
			auto DuplicateAttackerInput = MakeInput();
			DuplicateAttackerInput.ParticipantEligibilityResult.Input
				.RunnerSnapshot.CardId = CarrierId;
			ExpectFailure(
				Test, DuplicateAttackerInput,
				EError::InvalidParticipantEligibilityResult,
				TEXT("ParticipantEligibilityResult.Input.RunnerSnapshot.CardId"));

			auto CrossSideInput = MakeInput();
			CrossSideInput.ParticipantEligibilityResult =
				MakeEligibilityResult(false, true);
			Test.TestTrue(
				TEXT("Formal cross-side duplicate Eligibility succeeds"),
				CrossSideInput.ParticipantEligibilityResult.bSuccess);
			ExpectOffside(Test, ExpectSuccess(Test, CrossSideInput));
			break;
		}
		case 29:
		{
			auto Input = MakeInput();
			Input.bHasAntiOffsideAttackD6 = false;
			ExpectFailure(
				Test, Input, EError::MissingAntiOffsideAttackD6,
				TEXT("AntiOffsideAttackD6"));
			break;
		}
		case 30:
			ExpectFailure(
				Test, MakeInput(0), EError::InvalidAntiOffsideAttackD6,
				TEXT("AntiOffsideAttackD6"));
			break;
		case 31:
			ExpectFailure(
				Test, MakeInput(7), EError::InvalidAntiOffsideAttackD6,
				TEXT("AntiOffsideAttackD6"));
			break;
		case 32:
		{
			const auto BranchFive = ExpectSuccess(Test, MakeInput(5, 5));
			const auto BranchSix = ExpectSuccess(Test, MakeInput(5, 6));
			Test.TestNotEqual(
				TEXT("Branch Selection D6 values differ"),
				BranchFive.Input.BranchSelectionResult.Input.ExternalSelectionD6,
				BranchSix.Input.BranchSelectionResult.Input.ExternalSelectionD6);
			Test.TestEqual(
				TEXT("Same Anti-Offside D6 produces same decision"),
				BranchFive.Decision,
				BranchSix.Decision);
			ExpectOffside(Test, BranchFive);
			ExpectOffside(Test, BranchSix);
			break;
		}
		case 33:
		{
			auto Input = MakeInput(5);
			const auto Offside = ExpectSuccess(Test, Input);
			Input.AntiOffsideAttackD6 = 6;
			const auto OneOnOne = ExpectSuccess(Test, Input);
			ExpectOffside(Test, Offside);
			ExpectOneOnOne(Test, OneOnOne);
			Test.TestNotEqual(
				TEXT("Anti-Offside D6 alone switches the decision"),
				Offside.Decision,
				OneOnOne.Decision);
			break;
		}
		case 34:
		{
			const auto Result = ExpectSuccess(Test, MakeInput(6));
			Test.TestEqual(
				TEXT("One-on-One copies Eligibility RunnerId"),
				Result.RunnerId,
				Result.Input.ParticipantEligibilityResult.Input.RunnerSnapshot.CardId);
			break;
		}
		case 35:
			ExpectOffside(Test, ExpectSuccess(Test, MakeInput(4)));
			break;
		case 36:
		{
			const auto Result = ExpectSuccess(Test, MakeInput(6));
			Test.TestEqual(
				TEXT("Attacking-side owner provenance remains in Result Input"),
				Result.Input.ParticipantEligibilityResult.Input.AttackingOwnerId,
				AttackingOwnerId);
			Test.TestEqual(
				TEXT("Defending-side owner provenance remains in Result Input"),
				Result.Input.ParticipantEligibilityResult.Input.DefendingOwnerId,
				DefendingOwnerId);
			Test.TestNotEqual(
				TEXT("Owner sides remain distinct"),
				Result.Input.ParticipantEligibilityResult.Input.AttackingOwnerId,
				Result.Input.ParticipantEligibilityResult.Input.DefendingOwnerId);
			break;
		}
		case 37:
		{
			const auto Input = MakeInput(6, 6, true);
			const auto First = FThroughBallAntiOffsideOutcomeQuery::Evaluate(Input);
			const auto Second = FThroughBallAntiOffsideOutcomeQuery::Evaluate(Input);
			Test.TestEqual(TEXT("Success deterministic"), First.bSuccess, Second.bSuccess);
			Test.TestEqual(TEXT("Error deterministic"), First.ErrorCode, Second.ErrorCode);
			Test.TestEqual(TEXT("Message deterministic"), First.ErrorMessage, Second.ErrorMessage);
			Test.TestEqual(TEXT("InvalidField deterministic"), First.InvalidField, Second.InvalidField);
			Test.TestEqual(TEXT("Decision deterministic"), First.Decision, Second.Decision);
			Test.TestEqual(TEXT("Attack-ended deterministic"), First.bAttackEnded, Second.bAttackEnded);
			Test.TestEqual(TEXT("Continue deterministic"), First.bContinueResolution, Second.bContinueResolution);
			Test.TestEqual(TEXT("One-on-One deterministic"), First.bRequiresOneOnOne, Second.bRequiresOneOnOne);
			Test.TestEqual(TEXT("Runner deterministic"), First.RunnerId, Second.RunnerId);
			Test.TestEqual(TEXT("D6 presence deterministic"), First.Input.bHasAntiOffsideAttackD6, Second.Input.bHasAntiOffsideAttackD6);
			Test.TestEqual(TEXT("D6 deterministic"), First.Input.AntiOffsideAttackD6, Second.Input.AntiOffsideAttackD6);
			Test.TestEqual(TEXT("Branch provenance deterministic"), First.Input.BranchSelectionResult.SelectedThroughBallBranch, Second.Input.BranchSelectionResult.SelectedThroughBallBranch);
			Test.TestEqual(TEXT("Eligibility provenance deterministic"), First.Input.ParticipantEligibilityResult.Input.RunnerSnapshot.CardId, Second.Input.ParticipantEligibilityResult.Input.RunnerSnapshot.CardId);
			break;
		}
		case 38:
		{
			FString Implementation;
			const FString Path = FPaths::Combine(
				FPaths::ProjectDir(),
				TEXT("Source/FMCodex/CoreRules/ThroughBallAntiOffsideOutcomeQuery.cpp"));
			Test.TestTrue(
				TEXT("Production source is readable for auxiliary dependency audit"),
				FFileHelper::LoadFileToString(Implementation, *Path));
			Test.TestFalse(TEXT("Does not call Branch Selection Query"), Implementation.Contains(TEXT("FThroughBallBranchSelectionQuery::Select")));
			Test.TestFalse(TEXT("Does not call Participant Eligibility Query"), Implementation.Contains(TEXT("FThroughBallParticipantEligibilityQuery::Evaluate")));
			Test.TestFalse(TEXT("Does not call SkillRule Query"), Implementation.Contains(TEXT("FSkillRuleSnapshotQuery::FindBySkillId")));
			Test.TestFalse(TEXT("Does not call Snapshot Validator"), Implementation.Contains(TEXT("FPlayerCardRuleSnapshotValidator::Validate")));
			Test.TestFalse(TEXT("Does not call P1 Plan"), Implementation.Contains(TEXT("FThroughBallBehindDefenseP1PlanQuery::Evaluate")));
			Test.TestFalse(TEXT("Does not call P2 Outcome"), Implementation.Contains(TEXT("FThroughBallBehindDefenseP2OutcomeQuery::Evaluate")));
			Test.TestFalse(TEXT("Does not call FormulaResolver"), Implementation.Contains(TEXT("UFormulaResolver::ResolveFormula")));
			Test.TestFalse(TEXT("Does not call FormulaAttackFlow"), Implementation.Contains(TEXT("FormulaAttackFlow")));
			Test.TestFalse(TEXT("Does not call MatchPlay"), Implementation.Contains(TEXT("MatchPlay")));
			Test.TestFalse(TEXT("Does not use RNG"), Implementation.Contains(TEXT("FMath::Rand")) || Implementation.Contains(TEXT("FRandomStream")));
			Test.TestFalse(TEXT("Does not use Active Goalkeeper"), Implementation.Contains(TEXT("ActiveGoalkeeper")) || Implementation.Contains(TEXT("GoalkeeperId")));
			Test.TestFalse(TEXT("Does not access Match State"), Implementation.Contains(TEXT("MatchState")));
			Test.TestFalse(TEXT("Does not create Handoff"), Implementation.Contains(TEXT("Handoff")));
			break;
		}
		default:
			Test.AddError(TEXT("Unknown Anti-Offside Outcome Query test case."));
			return false;
		}

		return true;
	}
}

#define THROUGH_BALL_ANTI_OFFSIDE_OUTCOME_TEST(Number, Name) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
		FThroughBallAntiOffsideOutcomeTest##Number, \
		"FMCodex.CoreRules.ThroughBallAntiOffsideOutcomeQuery." Name, \
		EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter) \
	bool FThroughBallAntiOffsideOutcomeTest##Number::RunTest( \
		const FString& Parameters) \
	{ \
		return ThroughBallAntiOffsideOutcomeQueryTests::RunCase(*this, Number); \
	}

THROUGH_BALL_ANTI_OFFSIDE_OUTCOME_TEST(1, "01DefaultInputFailsSafely")
THROUGH_BALL_ANTI_OFFSIDE_OUTCOME_TEST(2, "02DefaultResultIsFailureSafe")
THROUGH_BALL_ANTI_OFFSIDE_OUTCOME_TEST(3, "03AttackD6OneIsOffside")
THROUGH_BALL_ANTI_OFFSIDE_OUTCOME_TEST(4, "04AttackD6FiveIsOffside")
THROUGH_BALL_ANTI_OFFSIDE_OUTCOME_TEST(5, "05AttackD6SixRequiresOneOnOne")
THROUGH_BALL_ANTI_OFFSIDE_OUTCOME_TEST(6, "06SelectedHighValueInputFieldsPreserved")
THROUGH_BALL_ANTI_OFFSIDE_OUTCOME_TEST(7, "07FormalBranchFailureMapsWithoutCopyingError")
THROUGH_BALL_ANTI_OFFSIDE_OUTCOME_TEST(8, "08RejectsSuccessfulBranchWithErrorCode")
THROUGH_BALL_ANTI_OFFSIDE_OUTCOME_TEST(9, "09RejectsSuccessfulBranchWithErrorMessage")
THROUGH_BALL_ANTI_OFFSIDE_OUTCOME_TEST(10, "10RejectsSuccessfulBranchWithInvalidField")
THROUGH_BALL_ANTI_OFFSIDE_OUTCOME_TEST(11, "11RejectsMissingSelectedBranch")
THROUGH_BALL_ANTI_OFFSIDE_OUTCOME_TEST(12, "12RejectsMissingBranchSelectionD6")
THROUGH_BALL_ANTI_OFFSIDE_OUTCOME_TEST(13, "13RejectsBranchSelectionD6OutOfRange")
THROUGH_BALL_ANTI_OFFSIDE_OUTCOME_TEST(14, "14RejectsBranchD6DecisionMismatch")
THROUGH_BALL_ANTI_OFFSIDE_OUTCOME_TEST(15, "15RejectsFeetBranch")
THROUGH_BALL_ANTI_OFFSIDE_OUTCOME_TEST(16, "16RejectsBehindDefenseBranch")
THROUGH_BALL_ANTI_OFFSIDE_OUTCOME_TEST(17, "17FormalEligibilityFailureMapsWithoutCopyingError")
THROUGH_BALL_ANTI_OFFSIDE_OUTCOME_TEST(18, "18RejectsSuccessfulEligibilityWithDiagnostics")
THROUGH_BALL_ANTI_OFFSIDE_OUTCOME_TEST(19, "19RejectsHelperPresenceMirrorMismatch")
THROUGH_BALL_ANTI_OFFSIDE_OUTCOME_TEST(20, "20RejectsInvalidNestedEligibilitySuccessEnvelope")
THROUGH_BALL_ANTI_OFFSIDE_OUTCOME_TEST(21, "21RejectsMissingAttackingOwner")
THROUGH_BALL_ANTI_OFFSIDE_OUTCOME_TEST(22, "22RejectsMissingDefendingOwner")
THROUGH_BALL_ANTI_OFFSIDE_OUTCOME_TEST(23, "23RejectsSameOwnerOnBothSides")
THROUGH_BALL_ANTI_OFFSIDE_OUTCOME_TEST(24, "24RejectsMissingRunnerId")
THROUGH_BALL_ANTI_OFFSIDE_OUTCOME_TEST(25, "25RejectsInvalidRunnerValidationEnvelope")
THROUGH_BALL_ANTI_OFFSIDE_OUTCOME_TEST(26, "26RejectsGoalkeeperRunner")
THROUGH_BALL_ANTI_OFFSIDE_OUTCOME_TEST(27, "27RejectsRunnerOutsideAttackingForwardArea")
THROUGH_BALL_ANTI_OFFSIDE_OUTCOME_TEST(28, "28RejectsCarrierRunnerIdentityReuse")
THROUGH_BALL_ANTI_OFFSIDE_OUTCOME_TEST(29, "29RejectsMissingAntiOffsideAttackD6")
THROUGH_BALL_ANTI_OFFSIDE_OUTCOME_TEST(30, "30RejectsAntiOffsideAttackD6BelowRange")
THROUGH_BALL_ANTI_OFFSIDE_OUTCOME_TEST(31, "31RejectsAntiOffsideAttackD6AboveRange")
THROUGH_BALL_ANTI_OFFSIDE_OUTCOME_TEST(32, "32BranchSelectionD6DoesNotSelectOutcome")
THROUGH_BALL_ANTI_OFFSIDE_OUTCOME_TEST(33, "33AntiOffsideAttackD6AloneSwitchesOutcome")
THROUGH_BALL_ANTI_OFFSIDE_OUTCOME_TEST(34, "34OneOnOneCopiesEligibilityRunnerId")
THROUGH_BALL_ANTI_OFFSIDE_OUTCOME_TEST(35, "35OffsideKeepsTerminalMetadataAndNoRunner")
THROUGH_BALL_ANTI_OFFSIDE_OUTCOME_TEST(36, "36SelectedOwnerAndSideProvenancePreserved")
THROUGH_BALL_ANTI_OFFSIDE_OUTCOME_TEST(37, "37SelectedObservedFieldsRepeatDeterministically")
THROUGH_BALL_ANTI_OFFSIDE_OUTCOME_TEST(38, "38DependencyAndStateBoundary")

#undef THROUGH_BALL_ANTI_OFFSIDE_OUTCOME_TEST

#endif
