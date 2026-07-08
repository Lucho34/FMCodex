#if WITH_DEV_AUTOMATION_TESTS

#include "CutInsideShotBranchSelectionQuery.h"

#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace CutInsideShotBranchSelectionQueryTests
{
	const FName SkillId(TEXT("Skill.CutInsideShot.BranchSelection"));
	const FName AttackerCardId(TEXT("CutInsideBranch.Attacker"));
	const FName DefenderCardId(TEXT("CutInsideBranch.Defender"));
	const FName AttackerPlayerId(TEXT("Player.CutInsideBranch.Attacker"));
	const FName DefenderPlayerId(TEXT("Player.CutInsideBranch.Defender"));
	const FGuid DirectShotLogId(910, 920, 930, 940);
	const FGuid DeadCornerLogId(950, 960, 970, 980);

	FPlayerCardRuleSnapshot MakeCard(
		const FName CardId,
		const EPlayerPositionType Position,
		const TArray<FName>& SkillIds = {})
	{
		FPlayerCardRuleSnapshot Card;
		Card.CardId = CardId;
		Card.PositionTypes = { Position };
		Card.SkillIds = SkillIds;
		Card.Attributes.Shooting = 4;
		Card.Attributes.Dribbling = 6;
		Card.Attributes.Tackling = 5;
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
		Rule.SkillType = ESkillRuleType::CutInsideShot;
		Rule.MinTriggerActionPoint = 3;
		Rule.MaxTriggerActionPoint = 6;

		FSkillRuleSnapshotSet SnapshotSet;
		SnapshotSet.SkillRules = { Rule };
		return SnapshotSet;
	}

	FCutInsideShotDirectShotPlanQueryInput MakeDirectShotInput(
		const int32 AttackD6 = 4,
		const int32 DefenseD6 = 3)
	{
		FCutInsideShotDirectShotPlanQueryInput Input;
		Input.SkillId = SkillId;
		Input.AttackerCardId = AttackerCardId;
		Input.DefenderCardId = DefenderCardId;
		Input.CurrentActionPoint = 4;
		Input.bHasExternalAttackD6 = true;
		Input.ExternalAttackD6 = AttackD6;
		Input.bHasExternalDefenseD6 = true;
		Input.ExternalDefenseD6 = DefenseD6;
		Input.LogId = DirectShotLogId;
		Input.TurnIndex = 24;
		Input.AttackerPlayerId = AttackerPlayerId;
		Input.DefenderPlayerId = DefenderPlayerId;
		return Input;
	}

	FCutInsideShotDeadCornerDecisionQueryInput MakeDeadCornerInput(
		const int32 D6A = 5,
		const int32 D6B = 6)
	{
		FCutInsideShotDeadCornerDecisionQueryInput Input;
		Input.SkillId = SkillId;
		Input.AttackerCardId = AttackerCardId;
		Input.CurrentActionPoint = 4;
		Input.bHasExternalAttackD6A = true;
		Input.ExternalAttackD6A = D6A;
		Input.bHasExternalAttackD6B = true;
		Input.ExternalAttackD6B = D6B;
		Input.LogId = DeadCornerLogId;
		Input.TurnIndex = 25;
		Input.AttackerPlayerId = AttackerPlayerId;
		return Input;
	}

	FCutInsideShotBranchSelectionQueryInput MakeInput(
		const ECutInsideShotBranch Branch)
	{
		FCutInsideShotBranchSelectionQueryInput Input;
		Input.Branch = Branch;
		Input.DirectShotInput = MakeDirectShotInput();
		Input.DeadCornerInput = MakeDeadCornerInput();
		return Input;
	}

	bool AreDirectShotInputsEqual(
		const FCutInsideShotDirectShotPlanQueryInput& Left,
		const FCutInsideShotDirectShotPlanQueryInput& Right)
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
		const FCutInsideShotDeadCornerDecisionQueryInput& Left,
		const FCutInsideShotDeadCornerDecisionQueryInput& Right)
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
		const FCutInsideShotBranchSelectionQueryInput& Left,
		const FCutInsideShotBranchSelectionQueryInput& Right)
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

	FCutInsideShotBranchSelectionQueryResult Select(
		const FCutInsideShotBranchSelectionQueryInput& Input)
	{
		return FCutInsideShotBranchSelectionQuery::Select(
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
				TEXT("CutInsideShotBranchSelectionQuery.h")));
		const bool bImplementationLoaded =
			FFileHelper::LoadFileToString(
				Implementation,
				*FPaths::Combine(
					Directory,
					TEXT("CutInsideShotBranchSelectionQuery.cpp")));
		OutSource = Header + Implementation;
		return bHeaderLoaded && bImplementationLoaded;
	}
}

#define CUT_INSIDE_BRANCH_TEST(ClassName, TestName) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
		ClassName, \
		"FMCodex.CoreRules.CutInsideShotBranchSelectionQuery." TestName, \
		EAutomationTestFlags::EditorContext \
			| EAutomationTestFlags::EngineFilter)

CUT_INSIDE_BRANCH_TEST(
	FCutInsideBranchRejectsNoneBranchTest,
	"RejectsNoneBranch")

bool FCutInsideBranchRejectsNoneBranchTest::RunTest(
	const FString& Parameters)
{
	using namespace CutInsideShotBranchSelectionQueryTests;
	const FCutInsideShotBranchSelectionQueryResult NoneResult = Select(
		MakeInput(ECutInsideShotBranch::None));
	TestFalse(TEXT("None branch fails"), NoneResult.bSuccess);
	TestEqual(
		TEXT("None branch error"),
		NoneResult.ErrorCode,
		ECutInsideShotBranchSelectionQueryErrorCode::InvalidBranch);
	TestEqual(
		TEXT("Invalid field"),
		NoneResult.InvalidField,
		FName(TEXT("Branch")));
	TestFalse(
		TEXT("Direct Shot is not evaluated"),
		NoneResult.DirectShotResult.bSuccess);
	TestFalse(
		TEXT("Dead Corner is not evaluated"),
		NoneResult.DeadCornerResult.bSuccess);
	return true;
}

CUT_INSIDE_BRANCH_TEST(
	FCutInsideBranchRejectsUnknownBranchTest,
	"RejectsUnknownBranch")

bool FCutInsideBranchRejectsUnknownBranchTest::RunTest(
	const FString& Parameters)
{
	using namespace CutInsideShotBranchSelectionQueryTests;
	const FCutInsideShotBranchSelectionQueryInput Input =
		MakeInput(static_cast<ECutInsideShotBranch>(255));
	const FCutInsideShotBranchSelectionQueryResult Result = Select(Input);
	TestFalse(TEXT("Invalid branch fails"), Result.bSuccess);
	TestEqual(
		TEXT("Invalid branch error"),
		Result.ErrorCode,
		ECutInsideShotBranchSelectionQueryErrorCode::InvalidBranch);
	TestEqual(
		TEXT("No outcome"),
		Result.Outcome,
		ECutInsideShotBranchSelectionOutcome::None);
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
	return true;
}

CUT_INSIDE_BRANCH_TEST(
	FCutInsideBranchDelegatesDirectImmediateMissTest,
	"DelegatesDirectShotImmediateMiss")

bool FCutInsideBranchDelegatesDirectImmediateMissTest::RunTest(
	const FString& Parameters)
{
	using namespace CutInsideShotBranchSelectionQueryTests;
	FCutInsideShotBranchSelectionQueryInput Input =
		MakeInput(ECutInsideShotBranch::DirectShot);
	Input.DirectShotInput.ExternalAttackD6 = 2;
	Input.DirectShotInput.bHasExternalDefenseD6 = false;
	const FCutInsideShotBranchSelectionQueryResult Result =
		Select(Input);
	TestTrue(TEXT("Selection succeeds"), Result.bSuccess);
	TestEqual(
		TEXT("Immediate miss outcome"),
		Result.Outcome,
		ECutInsideShotBranchSelectionOutcome
			::DirectShotImmediateMiss);
	TestEqual(
		TEXT("Nested decision preserved"),
		Result.DirectShotResult.Decision,
		ECutInsideShotDirectShotDecision::ImmediateMiss);
	TestTrue(TEXT("Attack ends"), Result.bAttackEnded);
	TestFalse(TEXT("No goal"), Result.bIsGoal);
	TestFalse(
		TEXT("No Formula Plan"),
		Result.DirectShotResult.bHasFormulaPlan);
	TestFalse(
		TEXT("Dead Corner remains unevaluated"),
		Result.DeadCornerResult.bSuccess);
	return true;
}

CUT_INSIDE_BRANCH_TEST(
	FCutInsideBranchDelegatesDirectPlanTest,
	"DelegatesDirectShotFormulaPlanRequired")

bool FCutInsideBranchDelegatesDirectPlanTest::RunTest(
	const FString& Parameters)
{
	using namespace CutInsideShotBranchSelectionQueryTests;
	const FCutInsideShotBranchSelectionQueryResult Result = Select(
		MakeInput(ECutInsideShotBranch::DirectShot));
	TestTrue(TEXT("Selection succeeds"), Result.bSuccess);
	TestEqual(
		TEXT("Formula Plan outcome"),
		Result.Outcome,
		ECutInsideShotBranchSelectionOutcome
			::DirectShotFormulaPlanRequired);
	TestEqual(
		TEXT("Nested decision preserved"),
		Result.DirectShotResult.Decision,
		ECutInsideShotDirectShotDecision
			::FormulaResolutionRequired);
	TestTrue(
		TEXT("Formula Plan remains in nested result"),
		Result.DirectShotResult.bHasFormulaPlan);
	TestFalse(TEXT("Attack not ended by selector"), Result.bAttackEnded);
	TestFalse(TEXT("No goal"), Result.bIsGoal);
	TestFalse(
		TEXT("Dead Corner remains unevaluated"),
		Result.DeadCornerResult.bSuccess);
	return true;
}

CUT_INSIDE_BRANCH_TEST(
	FCutInsideBranchDelegatesDeadGoalTest,
	"DelegatesDeadCornerGoal")

bool FCutInsideBranchDelegatesDeadGoalTest::RunTest(
	const FString& Parameters)
{
	using namespace CutInsideShotBranchSelectionQueryTests;
	const FCutInsideShotBranchSelectionQueryResult Result = Select(
		MakeInput(ECutInsideShotBranch::DeadCorner));
	TestTrue(TEXT("Selection succeeds"), Result.bSuccess);
	TestEqual(
		TEXT("Dead Corner goal outcome"),
		Result.Outcome,
		ECutInsideShotBranchSelectionOutcome::DeadCornerGoal);
	TestEqual(
		TEXT("Nested decision preserved"),
		Result.DeadCornerResult.Decision,
		ECutInsideShotDeadCornerDecision::Goal);
	TestTrue(TEXT("Attack ends"), Result.bAttackEnded);
	TestTrue(TEXT("Goal"), Result.bIsGoal);
	TestFalse(
		TEXT("Direct Shot remains unevaluated"),
		Result.DirectShotResult.bSuccess);
	return true;
}

CUT_INSIDE_BRANCH_TEST(
	FCutInsideBranchDelegatesDeadMissTest,
	"DelegatesDeadCornerMiss")

bool FCutInsideBranchDelegatesDeadMissTest::RunTest(
	const FString& Parameters)
{
	using namespace CutInsideShotBranchSelectionQueryTests;
	FCutInsideShotBranchSelectionQueryInput Input =
		MakeInput(ECutInsideShotBranch::DeadCorner);
	Input.DeadCornerInput.ExternalAttackD6A = 4;
	Input.DeadCornerInput.ExternalAttackD6B = 5;
	const FCutInsideShotBranchSelectionQueryResult Result =
		Select(Input);
	TestTrue(TEXT("Selection succeeds"), Result.bSuccess);
	TestEqual(
		TEXT("Dead Corner miss outcome"),
		Result.Outcome,
		ECutInsideShotBranchSelectionOutcome::DeadCornerMiss);
	TestEqual(
		TEXT("Nested decision preserved"),
		Result.DeadCornerResult.Decision,
		ECutInsideShotDeadCornerDecision::Miss);
	TestTrue(TEXT("Attack ends"), Result.bAttackEnded);
	TestFalse(TEXT("No goal"), Result.bIsGoal);
	TestFalse(
		TEXT("Direct Shot remains unevaluated"),
		Result.DirectShotResult.bSuccess);
	return true;
}

CUT_INSIDE_BRANCH_TEST(
	FCutInsideBranchPreservesDirectFailureTest,
	"PreservesDirectShotQueryFailure")

bool FCutInsideBranchPreservesDirectFailureTest::RunTest(
	const FString& Parameters)
{
	using namespace CutInsideShotBranchSelectionQueryTests;
	FCutInsideShotBranchSelectionQueryInput Input =
		MakeInput(ECutInsideShotBranch::DirectShot);
	Input.DirectShotInput.SkillId = NAME_None;
	const FCutInsideShotBranchSelectionQueryResult Result =
		Select(Input);
	TestFalse(TEXT("Selection fails"), Result.bSuccess);
	TestEqual(
		TEXT("Top-level error"),
		Result.ErrorCode,
		ECutInsideShotBranchSelectionQueryErrorCode
			::DirectShotQueryFailed);
	TestEqual(
		TEXT("Nested error preserved"),
		Result.DirectShotResult.ErrorCode,
		ECutInsideShotDirectShotPlanQueryErrorCode::MissingSkillId);
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

CUT_INSIDE_BRANCH_TEST(
	FCutInsideBranchPreservesDeadFailureTest,
	"PreservesDeadCornerQueryFailure")

bool FCutInsideBranchPreservesDeadFailureTest::RunTest(
	const FString& Parameters)
{
	using namespace CutInsideShotBranchSelectionQueryTests;
	FCutInsideShotBranchSelectionQueryInput Input =
		MakeInput(ECutInsideShotBranch::DeadCorner);
	Input.DeadCornerInput.bHasExternalAttackD6B = false;
	const FCutInsideShotBranchSelectionQueryResult Result =
		Select(Input);
	TestFalse(TEXT("Selection fails"), Result.bSuccess);
	TestEqual(
		TEXT("Top-level error"),
		Result.ErrorCode,
		ECutInsideShotBranchSelectionQueryErrorCode
			::DeadCornerQueryFailed);
	TestEqual(
		TEXT("Nested error preserved"),
		Result.DeadCornerResult.ErrorCode,
		ECutInsideShotDeadCornerDecisionQueryErrorCode
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

CUT_INSIDE_BRANCH_TEST(
	FCutInsideBranchPlanDoesNotExecuteTest,
	"DirectShotFormulaPlanDoesNotExecuteFormulaChain")

bool FCutInsideBranchPlanDoesNotExecuteTest::RunTest(
	const FString& Parameters)
{
	using namespace CutInsideShotBranchSelectionQueryTests;
	const FCutInsideShotBranchSelectionQueryResult Result = Select(
		MakeInput(ECutInsideShotBranch::DirectShot));
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

CUT_INSIDE_BRANCH_TEST(
	FCutInsideBranchDeadHasNoPlanTest,
	"DeadCornerDoesNotGenerateFormulaPlan")

bool FCutInsideBranchDeadHasNoPlanTest::RunTest(
	const FString& Parameters)
{
	using namespace CutInsideShotBranchSelectionQueryTests;
	const FCutInsideShotBranchSelectionQueryResult Result = Select(
		MakeInput(ECutInsideShotBranch::DeadCorner));
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
		ECutInsideShotDeadCornerDecision::Goal);
	return true;
}

CUT_INSIDE_BRANCH_TEST(
	FCutInsideBranchNoTopLevelFormulaPlanTest,
	"DoesNotCopyFormulaPlanAtTopLevel")

bool FCutInsideBranchNoTopLevelFormulaPlanTest::RunTest(
	const FString& Parameters)
{
	using namespace CutInsideShotBranchSelectionQueryTests;
	const FCutInsideShotBranchSelectionQueryResult Result = Select(
		MakeInput(ECutInsideShotBranch::DirectShot));
	TestTrue(
		TEXT("Nested Direct Shot has Formula Plan"),
		Result.DirectShotResult.bHasFormulaPlan);
	FString Source;
	TestTrue(TEXT("Query source loads"), LoadQuerySource(Source));
	TestFalse(
		TEXT("Selector defines no Formula Plan type"),
		Source.Contains(TEXT("FCutInsideShotDirectShotFormulaPlan")));
	TestFalse(
		TEXT("Selector defines no Formula Plan flag"),
		Source.Contains(TEXT("bHasFormulaPlan")));
	TestFalse(
		TEXT("Selector defines no Formula Plan member"),
		Source.Contains(TEXT(" FormulaPlan")));
	return true;
}

CUT_INSIDE_BRANCH_TEST(
	FCutInsideBranchDirectRequiresDefenseD6Test,
	"DirectShotFormulaResolutionRequiresDefenseD6")

bool FCutInsideBranchDirectRequiresDefenseD6Test::RunTest(
	const FString& Parameters)
{
	using namespace CutInsideShotBranchSelectionQueryTests;
	FCutInsideShotBranchSelectionQueryInput Input =
		MakeInput(ECutInsideShotBranch::DirectShot);
	Input.DirectShotInput.bHasExternalDefenseD6 = false;
	const FCutInsideShotBranchSelectionQueryResult Result =
		Select(Input);
	TestFalse(TEXT("Selection fails"), Result.bSuccess);
	TestEqual(
		TEXT("Nested missing Defense D6"),
		Result.DirectShotResult.ErrorCode,
		ECutInsideShotDirectShotPlanQueryErrorCode
			::MissingExternalDefenseD6);
	return true;
}

CUT_INSIDE_BRANCH_TEST(
	FCutInsideBranchImmediateMissNoDefenseD6Test,
	"DirectShotImmediateMissDoesNotRequireDefenseD6")

bool FCutInsideBranchImmediateMissNoDefenseD6Test::RunTest(
	const FString& Parameters)
{
	using namespace CutInsideShotBranchSelectionQueryTests;
	FCutInsideShotBranchSelectionQueryInput Input =
		MakeInput(ECutInsideShotBranch::DirectShot);
	Input.DirectShotInput.ExternalAttackD6 = 1;
	Input.DirectShotInput.bHasExternalDefenseD6 = false;
	Input.DirectShotInput.ExternalDefenseD6 = 0;
	const FCutInsideShotBranchSelectionQueryResult Result =
		Select(Input);
	TestTrue(TEXT("Immediate miss succeeds"), Result.bSuccess);
	TestEqual(
		TEXT("Immediate miss outcome"),
		Result.Outcome,
		ECutInsideShotBranchSelectionOutcome
			::DirectShotImmediateMiss);
	return true;
}

CUT_INSIDE_BRANCH_TEST(
	FCutInsideBranchDirectRequiresDefenderTest,
	"DirectShotRequiresDefenderIdentity")

bool FCutInsideBranchDirectRequiresDefenderTest::RunTest(
	const FString& Parameters)
{
	using namespace CutInsideShotBranchSelectionQueryTests;
	FCutInsideShotBranchSelectionQueryInput Input =
		MakeInput(ECutInsideShotBranch::DirectShot);
	Input.DirectShotInput.DefenderPlayerId = NAME_None;
	const FCutInsideShotBranchSelectionQueryResult Result =
		Select(Input);
	TestFalse(TEXT("Selection fails"), Result.bSuccess);
	TestEqual(
		TEXT("Nested log-context error"),
		Result.DirectShotResult.ErrorCode,
		ECutInsideShotDirectShotPlanQueryErrorCode
			::InvalidLogContext);
	TestEqual(
		TEXT("DefenderPlayerId is identified"),
		Result.InvalidField,
		FName(TEXT("DefenderPlayerId")));
	return true;
}

CUT_INSIDE_BRANCH_TEST(
	FCutInsideBranchDeadIgnoresDefenderTest,
	"DeadCornerIgnoresDefenderAndDefenseD6")

bool FCutInsideBranchDeadIgnoresDefenderTest::RunTest(
	const FString& Parameters)
{
	using namespace CutInsideShotBranchSelectionQueryTests;
	FCutInsideShotBranchSelectionQueryInput Input =
		MakeInput(ECutInsideShotBranch::DeadCorner);
	Input.DirectShotInput.DefenderCardId = NAME_None;
	Input.DirectShotInput.DefenderPlayerId = NAME_None;
	Input.DirectShotInput.bHasExternalDefenseD6 = false;
	Input.DirectShotInput.ExternalDefenseD6 = 0;
	const FCutInsideShotBranchSelectionQueryResult Result =
		Select(Input);
	TestTrue(TEXT("Dead Corner succeeds"), Result.bSuccess);
	TestEqual(
		TEXT("Dead Corner goal"),
		Result.Outcome,
		ECutInsideShotBranchSelectionOutcome::DeadCornerGoal);
	return true;
}

CUT_INSIDE_BRANCH_TEST(
	FCutInsideBranchDoesNotEvaluateUnselectedTest,
	"DoesNotEvaluateUnselectedBranch")

bool FCutInsideBranchDoesNotEvaluateUnselectedTest::RunTest(
	const FString& Parameters)
{
	using namespace CutInsideShotBranchSelectionQueryTests;
	FCutInsideShotBranchSelectionQueryInput DirectInput =
		MakeInput(ECutInsideShotBranch::DirectShot);
	DirectInput.DeadCornerInput.SkillId = NAME_None;
	DirectInput.DeadCornerInput.bHasExternalAttackD6A = false;
	const FCutInsideShotBranchSelectionQueryResult DirectResult =
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
		ECutInsideShotDeadCornerDecisionQueryErrorCode::None);

	FCutInsideShotBranchSelectionQueryInput DeadInput =
		MakeInput(ECutInsideShotBranch::DeadCorner);
	DeadInput.DirectShotInput.SkillId = NAME_None;
	DeadInput.DirectShotInput.AttackerCardId = NAME_None;
	const FCutInsideShotBranchSelectionQueryResult DeadResult =
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
		ECutInsideShotDirectShotPlanQueryErrorCode::None);
	return true;
}

CUT_INSIDE_BRANCH_TEST(
	FCutInsideBranchInputImmutabilityTest,
	"DoesNotMutateInput")

bool FCutInsideBranchInputImmutabilityTest::RunTest(
	const FString& Parameters)
{
	using namespace CutInsideShotBranchSelectionQueryTests;
	const FCutInsideShotBranchSelectionQueryInput Input =
		MakeInput(ECutInsideShotBranch::DirectShot);
	const FCutInsideShotBranchSelectionQueryInput Before = Input;
	const FCutInsideShotBranchSelectionQueryResult Result =
		Select(Input);
	TestTrue(
		TEXT("Input remains unchanged"),
		AreInputsEqual(Input, Before));
	TestTrue(
		TEXT("Result preserves input copy"),
		AreInputsEqual(Result.Input, Before));
	return true;
}

CUT_INSIDE_BRANCH_TEST(
	FCutInsideBranchPlayerSnapshotsImmutabilityTest,
	"DoesNotMutatePlayerCardSnapshotSet")

bool FCutInsideBranchPlayerSnapshotsImmutabilityTest::RunTest(
	const FString& Parameters)
{
	using namespace CutInsideShotBranchSelectionQueryTests;
	const FPlayerCardRuleSnapshotSet SnapshotSet =
		MakePlayerCardSnapshots();
	const FPlayerCardRuleSnapshotSet Before = SnapshotSet;
	FCutInsideShotBranchSelectionQuery::Select(
		SnapshotSet,
		MakeSkillRules(),
		MakeInput(ECutInsideShotBranch::DirectShot));
	TestTrue(
		TEXT("Player Snapshot Set remains unchanged"),
		ArePlayerSnapshotSetsEqual(SnapshotSet, Before));
	return true;
}

CUT_INSIDE_BRANCH_TEST(
	FCutInsideBranchSkillRulesImmutabilityTest,
	"DoesNotMutateSkillRuleSnapshotSet")

bool FCutInsideBranchSkillRulesImmutabilityTest::RunTest(
	const FString& Parameters)
{
	using namespace CutInsideShotBranchSelectionQueryTests;
	const FSkillRuleSnapshotSet SnapshotSet = MakeSkillRules();
	const FSkillRuleSnapshotSet Before = SnapshotSet;
	FCutInsideShotBranchSelectionQuery::Select(
		MakePlayerCardSnapshots(),
		SnapshotSet,
		MakeInput(ECutInsideShotBranch::DeadCorner));
	TestTrue(
		TEXT("Skill Rule Snapshot Set remains unchanged"),
		AreSkillRuleSnapshotSetsEqual(SnapshotSet, Before));
	return true;
}

CUT_INSIDE_BRANCH_TEST(
	FCutInsideBranchNoRandomGenerationTest,
	"DoesNotGenerateRandomValues")

bool FCutInsideBranchNoRandomGenerationTest::RunTest(
	const FString& Parameters)
{
	using namespace CutInsideShotBranchSelectionQueryTests;
	FString Source;
	TestTrue(TEXT("Query source loads"), LoadQuerySource(Source));
	TestFalse(
		TEXT("No FMath random"),
		Source.Contains(TEXT("FMath::Rand")));
	TestFalse(
		TEXT("No random stream"),
		Source.Contains(TEXT("FRandomStream")));
	return true;
}

CUT_INSIDE_BRANCH_TEST(
	FCutInsideBranchForbiddenDependenciesTest,
	"HasNoForbiddenDependencies")

bool FCutInsideBranchForbiddenDependenciesTest::RunTest(
	const FString& Parameters)
{
	using namespace CutInsideShotBranchSelectionQueryTests;
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
		TEXT("GenericBranchSelection"),
		TEXT("BranchSelectionQueryBase"),
		TEXT("CommonBranchSelection"),
		TEXT("LongShotBranchSelection"),
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

#undef CUT_INSIDE_BRANCH_TEST

#endif
