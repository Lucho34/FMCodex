#if WITH_DEV_AUTOMATION_TESTS

#include "LongShotBranchSelectionQuery.h"

#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace LongShotBranchSelectionQueryTests
{
	const FName SkillId(TEXT("Skill.LongShot.BranchSelection"));
	const FName AttackerCardId(TEXT("BranchSelection.Attacker"));
	const FName DefenderCardId(TEXT("BranchSelection.Defender"));
	const FName AttackerPlayerId(TEXT("Player.Attacker"));
	const FName DefenderPlayerId(TEXT("Player.Defender"));
	const FGuid DirectShotLogId(810, 820, 830, 840);
	const FGuid DeadCornerLogId(850, 860, 870, 880);

	FPlayerCardRuleSnapshot MakeCard(
		const FName CardId,
		const EPlayerPositionType Position,
		const TArray<FName>& SkillIds = {})
	{
		FPlayerCardRuleSnapshot Card;
		Card.CardId = CardId;
		Card.PositionTypes = { Position };
		Card.SkillIds = SkillIds;
		return Card;
	}

	FPlayerCardRuleSnapshotSet MakePlayerCardSnapshots()
	{
		FPlayerCardRuleSnapshotSet SnapshotSet;
		SnapshotSet.Cards = {
			MakeCard(
				AttackerCardId,
				EPlayerPositionType::Attack,
				{ SkillId }),
			MakeCard(
				DefenderCardId,
				EPlayerPositionType::Defense)
		};
		return SnapshotSet;
	}

	FSkillRuleSnapshotSet MakeSkillRules()
	{
		FSkillRuleSnapshot Rule;
		Rule.SkillId = SkillId;
		Rule.SkillType = ESkillRuleType::LongShot;
		Rule.MinTriggerActionPoint = 3;
		Rule.MaxTriggerActionPoint = 6;

		FSkillRuleSnapshotSet SnapshotSet;
		SnapshotSet.SkillRules = { Rule };
		return SnapshotSet;
	}

	FLongShotDirectShotPlanQueryInput MakeDirectShotInput(
		const int32 AttackD6 = 4,
		const int32 DefenseD6 = 3)
	{
		FLongShotDirectShotPlanQueryInput Input;
		Input.SkillId = SkillId;
		Input.AttackerCardId = AttackerCardId;
		Input.DefenderCardId = DefenderCardId;
		Input.CurrentActionPoint = 4;
		Input.bHasExternalAttackD6 = true;
		Input.ExternalAttackD6 = AttackD6;
		Input.bHasExternalDefenseD6 = true;
		Input.ExternalDefenseD6 = DefenseD6;
		Input.LogId = DirectShotLogId;
		Input.TurnIndex = 14;
		Input.AttackerPlayerId = AttackerPlayerId;
		Input.DefenderPlayerId = DefenderPlayerId;
		return Input;
	}

	FLongShotDeadCornerDecisionQueryInput MakeDeadCornerInput(
		const int32 D6A = 5,
		const int32 D6B = 6)
	{
		FLongShotDeadCornerDecisionQueryInput Input;
		Input.SkillId = SkillId;
		Input.AttackerCardId = AttackerCardId;
		Input.CurrentActionPoint = 4;
		Input.bHasExternalAttackD6A = true;
		Input.ExternalAttackD6A = D6A;
		Input.bHasExternalAttackD6B = true;
		Input.ExternalAttackD6B = D6B;
		Input.LogId = DeadCornerLogId;
		Input.TurnIndex = 15;
		Input.AttackerPlayerId = AttackerPlayerId;
		return Input;
	}

	FLongShotBranchSelectionQueryInput MakeInput(
		const ELongShotShotBranch Branch)
	{
		FLongShotBranchSelectionQueryInput Input;
		Input.Branch = Branch;
		Input.DirectShotInput = MakeDirectShotInput();
		Input.DeadCornerInput = MakeDeadCornerInput();
		return Input;
	}

	bool AreDirectShotInputsEqual(
		const FLongShotDirectShotPlanQueryInput& Left,
		const FLongShotDirectShotPlanQueryInput& Right)
	{
		return Left.SkillId == Right.SkillId
			&& Left.AttackerCardId == Right.AttackerCardId
			&& Left.DefenderCardId == Right.DefenderCardId
			&& Left.CurrentActionPoint == Right.CurrentActionPoint
			&& Left.bHasExternalAttackD6
				== Right.bHasExternalAttackD6
			&& Left.ExternalAttackD6 == Right.ExternalAttackD6
			&& Left.bHasExternalDefenseD6
				== Right.bHasExternalDefenseD6
			&& Left.ExternalDefenseD6 == Right.ExternalDefenseD6
			&& Left.LogId == Right.LogId
			&& Left.TurnIndex == Right.TurnIndex
			&& Left.AttackerPlayerId == Right.AttackerPlayerId
			&& Left.DefenderPlayerId == Right.DefenderPlayerId;
	}

	bool AreDeadCornerInputsEqual(
		const FLongShotDeadCornerDecisionQueryInput& Left,
		const FLongShotDeadCornerDecisionQueryInput& Right)
	{
		return Left.SkillId == Right.SkillId
			&& Left.AttackerCardId == Right.AttackerCardId
			&& Left.CurrentActionPoint == Right.CurrentActionPoint
			&& Left.bHasExternalAttackD6A
				== Right.bHasExternalAttackD6A
			&& Left.ExternalAttackD6A == Right.ExternalAttackD6A
			&& Left.bHasExternalAttackD6B
				== Right.bHasExternalAttackD6B
			&& Left.ExternalAttackD6B == Right.ExternalAttackD6B
			&& Left.LogId == Right.LogId
			&& Left.TurnIndex == Right.TurnIndex
			&& Left.AttackerPlayerId == Right.AttackerPlayerId;
	}

	bool AreInputsEqual(
		const FLongShotBranchSelectionQueryInput& Left,
		const FLongShotBranchSelectionQueryInput& Right)
	{
		return Left.Branch == Right.Branch
			&& AreDirectShotInputsEqual(
				Left.DirectShotInput,
				Right.DirectShotInput)
			&& AreDeadCornerInputsEqual(
				Left.DeadCornerInput,
				Right.DeadCornerInput);
	}

	bool ArePlayerAttributesEqual(
		const FPlayerAttributes& Left,
		const FPlayerAttributes& Right)
	{
		return Left.Shooting == Right.Shooting
			&& Left.Dribbling == Right.Dribbling
			&& Left.Passing == Right.Passing
			&& Left.OffBall == Right.OffBall
			&& Left.Marking == Right.Marking
			&& Left.Tackling == Right.Tackling
			&& Left.Speed == Right.Speed
			&& Left.Strength == Right.Strength
			&& Left.Stamina == Right.Stamina
			&& Left.LongShot == Right.LongShot;
	}

	bool AreGoalkeeperAttributesEqual(
		const FGoalkeeperAttributes& Left,
		const FGoalkeeperAttributes& Right)
	{
		return Left.Handling == Right.Handling
			&& Left.Positioning == Right.Positioning
			&& Left.Reflex == Right.Reflex
			&& Left.Aerial == Right.Aerial
			&& Left.Anticipation == Right.Anticipation
			&& Left.OneOnOne == Right.OneOnOne;
	}

	bool ArePlayerSnapshotSetsEqual(
		const FPlayerCardRuleSnapshotSet& Left,
		const FPlayerCardRuleSnapshotSet& Right)
	{
		if (Left.Cards.Num() != Right.Cards.Num())
		{
			return false;
		}
		for (int32 Index = 0; Index < Left.Cards.Num(); ++Index)
		{
			const FPlayerCardRuleSnapshot& LeftCard =
				Left.Cards[Index];
			const FPlayerCardRuleSnapshot& RightCard =
				Right.Cards[Index];
			if (LeftCard.CardId != RightCard.CardId
				|| LeftCard.PositionTypes != RightCard.PositionTypes
				|| !ArePlayerAttributesEqual(
					LeftCard.Attributes,
					RightCard.Attributes)
				|| LeftCard.bIsGoalkeeper
					!= RightCard.bIsGoalkeeper
				|| LeftCard.bHasGoalkeeperAttributes
					!= RightCard.bHasGoalkeeperAttributes
				|| !AreGoalkeeperAttributesEqual(
					LeftCard.GoalkeeperAttributes,
					RightCard.GoalkeeperAttributes)
				|| LeftCard.Rarity != RightCard.Rarity
				|| LeftCard.SkillIds != RightCard.SkillIds)
			{
				return false;
			}
		}
		return true;
	}

	bool AreSkillRuleSnapshotSetsEqual(
		const FSkillRuleSnapshotSet& Left,
		const FSkillRuleSnapshotSet& Right)
	{
		if (Left.SkillRules.Num() != Right.SkillRules.Num())
		{
			return false;
		}
		for (int32 Index = 0; Index < Left.SkillRules.Num(); ++Index)
		{
			const FSkillRuleSnapshot& LeftRule =
				Left.SkillRules[Index];
			const FSkillRuleSnapshot& RightRule =
				Right.SkillRules[Index];
			if (LeftRule.SkillId != RightRule.SkillId
				|| LeftRule.SkillType != RightRule.SkillType
				|| LeftRule.MinTriggerActionPoint
					!= RightRule.MinTriggerActionPoint
				|| LeftRule.MaxTriggerActionPoint
					!= RightRule.MaxTriggerActionPoint)
			{
				return false;
			}
		}
		return true;
	}

	FLongShotBranchSelectionQueryResult Select(
		const FLongShotBranchSelectionQueryInput& Input)
	{
		return FLongShotBranchSelectionQuery::Select(
			MakePlayerCardSnapshots(),
			MakeSkillRules(),
			Input);
	}

	bool LoadQuerySource(FString& OutSource)
	{
		const FString Directory = FPaths::Combine(
			FPaths::ProjectDir(),
			TEXT("Source/FMCodex/CoreRules"));
		FString Header;
		FString Implementation;
		const bool bHeaderLoaded = FFileHelper::LoadFileToString(
			Header,
			*FPaths::Combine(
				Directory,
				TEXT("LongShotBranchSelectionQuery.h")));
		const bool bImplementationLoaded =
			FFileHelper::LoadFileToString(
				Implementation,
				*FPaths::Combine(
					Directory,
					TEXT("LongShotBranchSelectionQuery.cpp")));
		OutSource = Header + Implementation;
		return bHeaderLoaded && bImplementationLoaded;
	}
}

#define LONG_SHOT_BRANCH_TEST(ClassName, TestName) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
		ClassName, \
		"FMCodex.CoreRules.LongShotBranchSelectionQuery." TestName, \
		EAutomationTestFlags::EditorContext \
			| EAutomationTestFlags::EngineFilter)

LONG_SHOT_BRANCH_TEST(
	FLongShotBranchRejectsInvalidBranchTest,
	"RejectsInvalidBranch")

bool FLongShotBranchRejectsInvalidBranchTest::RunTest(
	const FString& Parameters)
{
	using namespace LongShotBranchSelectionQueryTests;
	const FLongShotBranchSelectionQueryResult NoneResult = Select(
		MakeInput(ELongShotShotBranch::None));
	TestFalse(TEXT("None branch fails"), NoneResult.bSuccess);
	TestEqual(
		TEXT("None branch error"),
		NoneResult.ErrorCode,
		ELongShotBranchSelectionQueryErrorCode::InvalidBranch);

	const FLongShotBranchSelectionQueryInput Input =
		MakeInput(static_cast<ELongShotShotBranch>(255));
	const FLongShotBranchSelectionQueryResult Result = Select(Input);
	TestFalse(TEXT("Invalid branch fails"), Result.bSuccess);
	TestEqual(
		TEXT("Invalid branch error"),
		Result.ErrorCode,
		ELongShotBranchSelectionQueryErrorCode::InvalidBranch);
	TestEqual(
		TEXT("No outcome"),
		Result.Outcome,
		ELongShotBranchSelectionOutcome::None);
	TestEqual(
		TEXT("Invalid field"),
		Result.InvalidField,
		FName(TEXT("Branch")));
	TestFalse(
		TEXT("Direct Shot is not evaluated"),
		Result.DirectShotResult.bSuccess);
	TestFalse(
		TEXT("Dead Corner is not evaluated"),
		Result.DeadCornerResult.bSuccess);
	TestEqual(
		TEXT("Direct Shot error remains default"),
		Result.DirectShotResult.ErrorCode,
		ELongShotDirectShotPlanQueryErrorCode::None);
	TestEqual(
		TEXT("Dead Corner error remains default"),
		Result.DeadCornerResult.ErrorCode,
		ELongShotDeadCornerDecisionQueryErrorCode::None);
	return true;
}

LONG_SHOT_BRANCH_TEST(
	FLongShotBranchDelegatesDirectImmediateMissTest,
	"DelegatesDirectShotImmediateMiss")

bool FLongShotBranchDelegatesDirectImmediateMissTest::RunTest(
	const FString& Parameters)
{
	using namespace LongShotBranchSelectionQueryTests;
	FLongShotBranchSelectionQueryInput Input =
		MakeInput(ELongShotShotBranch::DirectShot);
	Input.DirectShotInput.ExternalAttackD6 = 2;
	Input.DirectShotInput.bHasExternalDefenseD6 = false;
	const FLongShotBranchSelectionQueryResult Result = Select(Input);
	TestTrue(TEXT("Selection succeeds"), Result.bSuccess);
	TestEqual(
		TEXT("Immediate miss outcome"),
		Result.Outcome,
		ELongShotBranchSelectionOutcome::DirectShotImmediateMiss);
	TestEqual(
		TEXT("Nested decision preserved"),
		Result.DirectShotResult.Decision,
		ELongShotDirectShotDecision::ImmediateMiss);
	TestTrue(TEXT("Attack ends"), Result.bAttackEnded);
	TestFalse(TEXT("No goal"), Result.bIsGoal);
	TestFalse(
		TEXT("No Formula Plan"),
		Result.DirectShotResult.bHasFormulaPlan);
	return true;
}

LONG_SHOT_BRANCH_TEST(
	FLongShotBranchDelegatesDirectPlanTest,
	"DelegatesDirectShotFormulaPlanRequired")

bool FLongShotBranchDelegatesDirectPlanTest::RunTest(
	const FString& Parameters)
{
	using namespace LongShotBranchSelectionQueryTests;
	const FLongShotBranchSelectionQueryResult Result = Select(
		MakeInput(ELongShotShotBranch::DirectShot));
	TestTrue(TEXT("Selection succeeds"), Result.bSuccess);
	TestEqual(
		TEXT("Formula Plan outcome"),
		Result.Outcome,
		ELongShotBranchSelectionOutcome
			::DirectShotFormulaPlanRequired);
	TestEqual(
		TEXT("Nested decision preserved"),
		Result.DirectShotResult.Decision,
		ELongShotDirectShotDecision::FormulaResolutionRequired);
	TestTrue(
		TEXT("Formula Plan remains in nested result"),
		Result.DirectShotResult.bHasFormulaPlan);
	TestFalse(TEXT("Attack not ended by selector"), Result.bAttackEnded);
	TestFalse(TEXT("No goal"), Result.bIsGoal);
	return true;
}

LONG_SHOT_BRANCH_TEST(
	FLongShotBranchDelegatesDeadGoalTest,
	"DelegatesDeadCornerGoal")

bool FLongShotBranchDelegatesDeadGoalTest::RunTest(
	const FString& Parameters)
{
	using namespace LongShotBranchSelectionQueryTests;
	const FLongShotBranchSelectionQueryResult Result = Select(
		MakeInput(ELongShotShotBranch::DeadCorner));
	TestTrue(TEXT("Selection succeeds"), Result.bSuccess);
	TestEqual(
		TEXT("Dead Corner goal outcome"),
		Result.Outcome,
		ELongShotBranchSelectionOutcome::DeadCornerGoal);
	TestEqual(
		TEXT("Nested decision preserved"),
		Result.DeadCornerResult.Decision,
		ELongShotDeadCornerDecision::Goal);
	TestTrue(TEXT("Attack ends"), Result.bAttackEnded);
	TestTrue(TEXT("Goal"), Result.bIsGoal);
	return true;
}

LONG_SHOT_BRANCH_TEST(
	FLongShotBranchDelegatesDeadMissTest,
	"DelegatesDeadCornerMiss")

bool FLongShotBranchDelegatesDeadMissTest::RunTest(
	const FString& Parameters)
{
	using namespace LongShotBranchSelectionQueryTests;
	FLongShotBranchSelectionQueryInput Input =
		MakeInput(ELongShotShotBranch::DeadCorner);
	Input.DeadCornerInput.ExternalAttackD6A = 4;
	Input.DeadCornerInput.ExternalAttackD6B = 5;
	const FLongShotBranchSelectionQueryResult Result = Select(Input);
	TestTrue(TEXT("Selection succeeds"), Result.bSuccess);
	TestEqual(
		TEXT("Dead Corner miss outcome"),
		Result.Outcome,
		ELongShotBranchSelectionOutcome::DeadCornerMiss);
	TestEqual(
		TEXT("Nested decision preserved"),
		Result.DeadCornerResult.Decision,
		ELongShotDeadCornerDecision::Miss);
	TestTrue(TEXT("Attack ends"), Result.bAttackEnded);
	TestFalse(TEXT("No goal"), Result.bIsGoal);
	return true;
}

LONG_SHOT_BRANCH_TEST(
	FLongShotBranchPreservesDirectFailureTest,
	"PreservesDirectShotQueryFailure")

bool FLongShotBranchPreservesDirectFailureTest::RunTest(
	const FString& Parameters)
{
	using namespace LongShotBranchSelectionQueryTests;
	FLongShotBranchSelectionQueryInput Input =
		MakeInput(ELongShotShotBranch::DirectShot);
	Input.DirectShotInput.SkillId = NAME_None;
	const FLongShotBranchSelectionQueryResult Result = Select(Input);
	TestFalse(TEXT("Selection fails"), Result.bSuccess);
	TestEqual(
		TEXT("Top-level error"),
		Result.ErrorCode,
		ELongShotBranchSelectionQueryErrorCode::DirectShotQueryFailed);
	TestEqual(
		TEXT("Nested error preserved"),
		Result.DirectShotResult.ErrorCode,
		ELongShotDirectShotPlanQueryErrorCode::MissingSkillId);
	TestEqual(
		TEXT("Error message projected"),
		Result.ErrorMessage,
		Result.DirectShotResult.ErrorMessage);
	TestEqual(
		TEXT("Invalid field projected"),
		Result.InvalidField,
		Result.DirectShotResult.InvalidField);
	TestTrue(
		TEXT("Nested input preserved"),
		AreDirectShotInputsEqual(
			Result.DirectShotResult.Input,
			Input.DirectShotInput));
	return true;
}

LONG_SHOT_BRANCH_TEST(
	FLongShotBranchPreservesDeadFailureTest,
	"PreservesDeadCornerQueryFailure")

bool FLongShotBranchPreservesDeadFailureTest::RunTest(
	const FString& Parameters)
{
	using namespace LongShotBranchSelectionQueryTests;
	FLongShotBranchSelectionQueryInput Input =
		MakeInput(ELongShotShotBranch::DeadCorner);
	Input.DeadCornerInput.bHasExternalAttackD6B = false;
	const FLongShotBranchSelectionQueryResult Result = Select(Input);
	TestFalse(TEXT("Selection fails"), Result.bSuccess);
	TestEqual(
		TEXT("Top-level error"),
		Result.ErrorCode,
		ELongShotBranchSelectionQueryErrorCode::DeadCornerQueryFailed);
	TestEqual(
		TEXT("Nested error preserved"),
		Result.DeadCornerResult.ErrorCode,
		ELongShotDeadCornerDecisionQueryErrorCode
			::MissingExternalAttackD6B);
	TestEqual(
		TEXT("Error message projected"),
		Result.ErrorMessage,
		Result.DeadCornerResult.ErrorMessage);
	TestEqual(
		TEXT("Invalid field projected"),
		Result.InvalidField,
		Result.DeadCornerResult.InvalidField);
	TestTrue(
		TEXT("Nested input preserved"),
		AreDeadCornerInputsEqual(
			Result.DeadCornerResult.Input,
			Input.DeadCornerInput));
	return true;
}

LONG_SHOT_BRANCH_TEST(
	FLongShotBranchPlanDoesNotExecuteTest,
	"DirectShotFormulaPlanDoesNotExecuteFormulaChain")

bool FLongShotBranchPlanDoesNotExecuteTest::RunTest(
	const FString& Parameters)
{
	using namespace LongShotBranchSelectionQueryTests;
	const FLongShotBranchSelectionQueryResult Result = Select(
		MakeInput(ELongShotShotBranch::DirectShot));
	TestTrue(
		TEXT("Plan is generated by nested Query"),
		Result.DirectShotResult.bHasFormulaPlan);
	FString Source;
	TestTrue(TEXT("Query source loads"), LoadQuerySource(Source));
	const TArray<FString> ForbiddenCalls = {
		TEXT("FSingleCardFormulaInputAssemblyQuery::Assemble"),
		TEXT("FSingleCardFormulaResolverInputAssembler::Assemble"),
		TEXT("FSingleCardFormulaResolutionExecutor::Execute"),
		TEXT("UFormulaResolver::ResolveFormula")
	};
	for (const FString& Call : ForbiddenCalls)
	{
		TestFalse(
			TEXT("Formula-chain call is absent"),
			Source.Contains(Call));
	}
	return true;
}

LONG_SHOT_BRANCH_TEST(
	FLongShotBranchDeadHasNoPlanTest,
	"DeadCornerDoesNotGenerateFormulaPlan")

bool FLongShotBranchDeadHasNoPlanTest::RunTest(
	const FString& Parameters)
{
	using namespace LongShotBranchSelectionQueryTests;
	const FLongShotBranchSelectionQueryResult Result = Select(
		MakeInput(ELongShotShotBranch::DeadCorner));
	TestTrue(TEXT("Dead Corner succeeds"), Result.bSuccess);
	TestFalse(
		TEXT("Direct Shot result remains unevaluated"),
		Result.DirectShotResult.bSuccess);
	TestFalse(
		TEXT("No Formula Plan exists in Direct Shot result"),
		Result.DirectShotResult.bHasFormulaPlan);
	TestEqual(
		TEXT("Dead Corner returns decision"),
		Result.DeadCornerResult.Decision,
		ELongShotDeadCornerDecision::Goal);
	return true;
}

LONG_SHOT_BRANCH_TEST(
	FLongShotBranchDirectRequiresDefenseD6Test,
	"DirectShotFormulaResolutionRequiresDefenseD6")

bool FLongShotBranchDirectRequiresDefenseD6Test::RunTest(
	const FString& Parameters)
{
	using namespace LongShotBranchSelectionQueryTests;
	FLongShotBranchSelectionQueryInput Input =
		MakeInput(ELongShotShotBranch::DirectShot);
	Input.DirectShotInput.bHasExternalDefenseD6 = false;
	const FLongShotBranchSelectionQueryResult Result = Select(Input);
	TestFalse(TEXT("Selection fails"), Result.bSuccess);
	TestEqual(
		TEXT("Nested missing Defense D6"),
		Result.DirectShotResult.ErrorCode,
		ELongShotDirectShotPlanQueryErrorCode
			::MissingExternalDefenseD6);
	return true;
}

LONG_SHOT_BRANCH_TEST(
	FLongShotBranchImmediateMissNoDefenseD6Test,
	"DirectShotImmediateMissDoesNotRequireDefenseD6")

bool FLongShotBranchImmediateMissNoDefenseD6Test::RunTest(
	const FString& Parameters)
{
	using namespace LongShotBranchSelectionQueryTests;
	FLongShotBranchSelectionQueryInput Input =
		MakeInput(ELongShotShotBranch::DirectShot);
	Input.DirectShotInput.ExternalAttackD6 = 1;
	Input.DirectShotInput.bHasExternalDefenseD6 = false;
	Input.DirectShotInput.ExternalDefenseD6 = 0;
	const FLongShotBranchSelectionQueryResult Result = Select(Input);
	TestTrue(TEXT("Immediate miss succeeds"), Result.bSuccess);
	TestEqual(
		TEXT("Immediate miss outcome"),
		Result.Outcome,
		ELongShotBranchSelectionOutcome::DirectShotImmediateMiss);
	return true;
}

LONG_SHOT_BRANCH_TEST(
	FLongShotBranchDirectRequiresDefenderTest,
	"DirectShotRequiresDefenderIdentity")

bool FLongShotBranchDirectRequiresDefenderTest::RunTest(
	const FString& Parameters)
{
	using namespace LongShotBranchSelectionQueryTests;
	FLongShotBranchSelectionQueryInput Input =
		MakeInput(ELongShotShotBranch::DirectShot);
	Input.DirectShotInput.DefenderPlayerId = NAME_None;
	const FLongShotBranchSelectionQueryResult Result = Select(Input);
	TestFalse(TEXT("Selection fails"), Result.bSuccess);
	TestEqual(
		TEXT("Nested log-context error"),
		Result.DirectShotResult.ErrorCode,
		ELongShotDirectShotPlanQueryErrorCode::InvalidLogContext);
	TestEqual(
		TEXT("DefenderPlayerId is identified"),
		Result.InvalidField,
		FName(TEXT("DefenderPlayerId")));
	return true;
}

LONG_SHOT_BRANCH_TEST(
	FLongShotBranchDeadIgnoresDefenderTest,
	"DeadCornerIgnoresDefenderAndDefenseD6")

bool FLongShotBranchDeadIgnoresDefenderTest::RunTest(
	const FString& Parameters)
{
	using namespace LongShotBranchSelectionQueryTests;
	FLongShotBranchSelectionQueryInput Input =
		MakeInput(ELongShotShotBranch::DeadCorner);
	Input.DirectShotInput.DefenderCardId = NAME_None;
	Input.DirectShotInput.DefenderPlayerId = NAME_None;
	Input.DirectShotInput.bHasExternalDefenseD6 = false;
	Input.DirectShotInput.ExternalDefenseD6 = 0;
	const FLongShotBranchSelectionQueryResult Result = Select(Input);
	TestTrue(TEXT("Dead Corner succeeds"), Result.bSuccess);
	TestEqual(
		TEXT("Dead Corner goal"),
		Result.Outcome,
		ELongShotBranchSelectionOutcome::DeadCornerGoal);
	return true;
}

LONG_SHOT_BRANCH_TEST(
	FLongShotBranchDoesNotEvaluateUnselectedTest,
	"DoesNotEvaluateUnselectedBranch")

bool FLongShotBranchDoesNotEvaluateUnselectedTest::RunTest(
	const FString& Parameters)
{
	using namespace LongShotBranchSelectionQueryTests;
	FLongShotBranchSelectionQueryInput DirectInput =
		MakeInput(ELongShotShotBranch::DirectShot);
	DirectInput.DeadCornerInput.SkillId = NAME_None;
	DirectInput.DeadCornerInput.bHasExternalAttackD6A = false;
	const FLongShotBranchSelectionQueryResult DirectResult =
		Select(DirectInput);
	TestTrue(
		TEXT("Direct Shot ignores invalid Dead Corner input"),
		DirectResult.bSuccess);
	TestFalse(
		TEXT("Dead Corner remains unevaluated"),
		DirectResult.DeadCornerResult.bSuccess);
	TestEqual(
		TEXT("Dead Corner error remains default"),
		DirectResult.DeadCornerResult.ErrorCode,
		ELongShotDeadCornerDecisionQueryErrorCode::None);

	FLongShotBranchSelectionQueryInput DeadInput =
		MakeInput(ELongShotShotBranch::DeadCorner);
	DeadInput.DirectShotInput.SkillId = NAME_None;
	DeadInput.DirectShotInput.AttackerCardId = NAME_None;
	const FLongShotBranchSelectionQueryResult DeadResult =
		Select(DeadInput);
	TestTrue(
		TEXT("Dead Corner ignores invalid Direct Shot input"),
		DeadResult.bSuccess);
	TestFalse(
		TEXT("Direct Shot remains unevaluated"),
		DeadResult.DirectShotResult.bSuccess);
	TestEqual(
		TEXT("Direct Shot error remains default"),
		DeadResult.DirectShotResult.ErrorCode,
		ELongShotDirectShotPlanQueryErrorCode::None);
	return true;
}

LONG_SHOT_BRANCH_TEST(
	FLongShotBranchInputImmutabilityTest,
	"DoesNotMutateInput")

bool FLongShotBranchInputImmutabilityTest::RunTest(
	const FString& Parameters)
{
	using namespace LongShotBranchSelectionQueryTests;
	const FLongShotBranchSelectionQueryInput Input =
		MakeInput(ELongShotShotBranch::DirectShot);
	const FLongShotBranchSelectionQueryInput Before = Input;
	const FLongShotBranchSelectionQueryResult Result = Select(Input);
	TestTrue(
		TEXT("Input remains unchanged"),
		AreInputsEqual(Input, Before));
	TestTrue(
		TEXT("Result preserves input copy"),
		AreInputsEqual(Result.Input, Before));
	return true;
}

LONG_SHOT_BRANCH_TEST(
	FLongShotBranchPlayerSnapshotsImmutabilityTest,
	"DoesNotMutatePlayerCardSnapshotSet")

bool FLongShotBranchPlayerSnapshotsImmutabilityTest::RunTest(
	const FString& Parameters)
{
	using namespace LongShotBranchSelectionQueryTests;
	const FPlayerCardRuleSnapshotSet SnapshotSet =
		MakePlayerCardSnapshots();
	const FPlayerCardRuleSnapshotSet Before = SnapshotSet;
	FLongShotBranchSelectionQuery::Select(
		SnapshotSet,
		MakeSkillRules(),
		MakeInput(ELongShotShotBranch::DirectShot));
	TestTrue(
		TEXT("Player Snapshot Set remains unchanged"),
		ArePlayerSnapshotSetsEqual(SnapshotSet, Before));
	return true;
}

LONG_SHOT_BRANCH_TEST(
	FLongShotBranchSkillRulesImmutabilityTest,
	"DoesNotMutateSkillRuleSnapshotSet")

bool FLongShotBranchSkillRulesImmutabilityTest::RunTest(
	const FString& Parameters)
{
	using namespace LongShotBranchSelectionQueryTests;
	const FSkillRuleSnapshotSet SnapshotSet = MakeSkillRules();
	const FSkillRuleSnapshotSet Before = SnapshotSet;
	FLongShotBranchSelectionQuery::Select(
		MakePlayerCardSnapshots(),
		SnapshotSet,
		MakeInput(ELongShotShotBranch::DeadCorner));
	TestTrue(
		TEXT("Skill Rule Snapshot Set remains unchanged"),
		AreSkillRuleSnapshotSetsEqual(SnapshotSet, Before));
	return true;
}

LONG_SHOT_BRANCH_TEST(
	FLongShotBranchForbiddenDependenciesTest,
	"HasNoForbiddenDependencies")

bool FLongShotBranchForbiddenDependenciesTest::RunTest(
	const FString& Parameters)
{
	using namespace LongShotBranchSelectionQueryTests;
	FString Source;
	TestTrue(TEXT("Query source loads"), LoadQuerySource(Source));
	const TArray<FString> ForbiddenTerms = {
		TEXT("MatchPlay"),
		TEXT("ExternalApi"),
		TEXT("FormulaAttackFlow"),
		TEXT("DataTable"),
		TEXT("Provider"),
		TEXT("CardDatabase"),
		TEXT("SkillEffect"),
		TEXT("SkillPipeline"),
		TEXT("FMath::Rand"),
		TEXT("FRandomStream"),
		TEXT("UFormulaResolver::ResolveFormula"),
		TEXT("FSingleCardFormulaInputAssemblyQuery::Assemble"),
		TEXT("FSingleCardFormulaResolverInputAssembler::Assemble"),
		TEXT("FSingleCardFormulaResolutionExecutor::Execute")
	};
	for (const FString& Term : ForbiddenTerms)
	{
		TestFalse(
			TEXT("Forbidden dependency is absent"),
			Source.Contains(Term));
	}
	return true;
}

#undef LONG_SHOT_BRANCH_TEST

#endif
