#if WITH_DEV_AUTOMATION_TESTS

#include "PassControlDribbleAdvancePlanQuery.h"

#include "HAL/FileManager.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace PassControlDribbleAdvancePlanQueryTests
{
	const FName SkillId(TEXT("Skill.PassControl.DribbleAdvance"));
	const FName CarrierCardId(TEXT("DribbleAdvance.Carrier"));
	const FName RunnerCardId(TEXT("DribbleAdvance.Runner"));
	const FName MarkerCardId(TEXT("DribbleAdvance.Marker"));
	const FName HelperCardId(TEXT("DribbleAdvance.Helper"));
	const FName CarrierPlayerId(TEXT("Player.DribbleAdvance.Carrier"));
	const FName RunnerPlayerId(TEXT("Player.DribbleAdvance.Runner"));
	const FName MarkerPlayerId(TEXT("Player.DribbleAdvance.Marker"));
	const FName HelperPlayerId(TEXT("Player.DribbleAdvance.Helper"));
	const FGuid LogId(6541, 6542, 6543, 6544);

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

	FPlayerCardRuleSnapshotSet MakeSnapshots(
		const int32 CarrierDribbling = 4,
		const int32 RunnerPassing = 6,
		const int32 MarkerTackling = 5,
		const int32 HelperMarking = 3,
		const bool bCarrierOwnsSkill = true,
		const bool bCarrierGoalkeeper = false,
		const bool bRunnerMidfield = true,
		const bool bMarkerGoalkeeper = false,
		const bool bHelperGoalkeeper = false)
	{
		FPlayerCardRuleSnapshot Carrier = MakeCard(
			CarrierCardId,
			bCarrierGoalkeeper
				? EPlayerPositionType::Goalkeeper
				: EPlayerPositionType::Midfield,
			bCarrierOwnsSkill ? TArray<FName>{ SkillId } : TArray<FName>{});
		Carrier.Attributes.Dribbling = CarrierDribbling;
		Carrier.bIsGoalkeeper = bCarrierGoalkeeper;
		Carrier.bHasGoalkeeperAttributes = bCarrierGoalkeeper;

		FPlayerCardRuleSnapshot Runner = MakeCard(
			RunnerCardId,
			bRunnerMidfield ? EPlayerPositionType::Midfield : EPlayerPositionType::Attack);
		Runner.Attributes.Passing = RunnerPassing;

		FPlayerCardRuleSnapshot Marker = MakeCard(
			MarkerCardId,
			bMarkerGoalkeeper
				? EPlayerPositionType::Goalkeeper
				: EPlayerPositionType::Defense);
		Marker.Attributes.Tackling = MarkerTackling;
		Marker.bIsGoalkeeper = bMarkerGoalkeeper;
		Marker.bHasGoalkeeperAttributes = bMarkerGoalkeeper;

		FPlayerCardRuleSnapshot Helper = MakeCard(
			HelperCardId,
			bHelperGoalkeeper
				? EPlayerPositionType::Goalkeeper
				: EPlayerPositionType::Defense);
		Helper.Attributes.Marking = HelperMarking;
		Helper.bIsGoalkeeper = bHelperGoalkeeper;
		Helper.bHasGoalkeeperAttributes = bHelperGoalkeeper;

		FPlayerCardRuleSnapshotSet Set;
		Set.Cards = { Carrier, Runner, Marker, Helper };
		return Set;
	}

	FSkillRuleSnapshotSet MakeSkillRules(
		const ESkillRuleType SkillType = ESkillRuleType::PassControl)
	{
		FSkillRuleSnapshot Rule;
		Rule.SkillId = SkillId;
		Rule.SkillType = SkillType;
		Rule.MinTriggerActionPoint = 3;
		Rule.MaxTriggerActionPoint = 6;
		FSkillRuleSnapshotSet Set;
		Set.SkillRules = { Rule };
		return Set;
	}

	FPassControlDribbleAdvancePlanQueryInput MakeValidInput()
	{
		FPassControlDribbleAdvancePlanQueryInput Input;
		Input.SkillId = SkillId;
		Input.AdvanceType = EPassControlAdvanceType::DribbleAdvance;
		Input.CarrierCardId = CarrierCardId;
		Input.RunnerCardId = RunnerCardId;
		Input.MarkerCardId = MarkerCardId;
		Input.bHasHelper = true;
		Input.HelperCardId = HelperCardId;
		Input.CurrentActionPoint = 4;
		Input.bHasExternalAttackD6 = true;
		Input.ExternalAttackD6 = 4;
		Input.bHasExternalDefenseD6 = true;
		Input.ExternalDefenseD6 = 3;
		Input.LogId = LogId;
		Input.TurnIndex = 21;
		Input.CarrierPlayerId = CarrierPlayerId;
		Input.RunnerPlayerId = RunnerPlayerId;
		Input.MarkerPlayerId = MarkerPlayerId;
		Input.HelperPlayerId = HelperPlayerId;
		return Input;
	}

	FPassControlDribbleAdvancePlanQueryInput MakeInputWithoutHelper()
	{
		FPassControlDribbleAdvancePlanQueryInput Input = MakeValidInput();
		Input.bHasHelper = false;
		Input.HelperCardId = NAME_None;
		Input.HelperPlayerId = NAME_None;
		return Input;
	}

	FPassControlDribbleAdvancePlanQueryResult Build(
		const FPassControlDribbleAdvancePlanQueryInput& Input = MakeValidInput(),
		const FPlayerCardRuleSnapshotSet& Snapshots = MakeSnapshots(),
		const FSkillRuleSnapshotSet& Rules = MakeSkillRules())
	{
		return FPassControlDribbleAdvancePlanQuery::BuildPlan(Snapshots, Rules, Input);
	}

	bool ExpectFailure(
		FAutomationTestBase& Test,
		const FPassControlDribbleAdvancePlanQueryResult& Result,
		const EPassControlDribbleAdvancePlanQueryErrorCode ErrorCode,
		const FName Field)
	{
		Test.TestFalse(TEXT("Query fails"), Result.bSuccess);
		Test.TestFalse(TEXT("Failure has no plan"), Result.bHasFormulaPlan);
		Test.TestEqual(TEXT("Expected error"), Result.ErrorCode, ErrorCode);
		Test.TestEqual(TEXT("Expected invalid field"), Result.InvalidField, Field);
		Test.TestFalse(TEXT("Failure retains diagnostic"), Result.ErrorMessage.IsEmpty());
		return true;
	}

	bool InputsEqual(
		const FPassControlDribbleAdvancePlanQueryInput& Left,
		const FPassControlDribbleAdvancePlanQueryInput& Right)
	{
		return Left.SkillId == Right.SkillId
			&& Left.AdvanceType == Right.AdvanceType
			&& Left.CarrierCardId == Right.CarrierCardId
			&& Left.RunnerCardId == Right.RunnerCardId
			&& Left.MarkerCardId == Right.MarkerCardId
			&& Left.bHasHelper == Right.bHasHelper
			&& Left.HelperCardId == Right.HelperCardId
			&& Left.CurrentActionPoint == Right.CurrentActionPoint
			&& Left.bHasExternalAttackD6 == Right.bHasExternalAttackD6
			&& Left.ExternalAttackD6 == Right.ExternalAttackD6
			&& Left.bHasExternalDefenseD6 == Right.bHasExternalDefenseD6
			&& Left.ExternalDefenseD6 == Right.ExternalDefenseD6
			&& Left.LogId == Right.LogId
			&& Left.TurnIndex == Right.TurnIndex
			&& Left.CarrierPlayerId == Right.CarrierPlayerId
			&& Left.RunnerPlayerId == Right.RunnerPlayerId
			&& Left.MarkerPlayerId == Right.MarkerPlayerId
			&& Left.HelperPlayerId == Right.HelperPlayerId;
	}

	bool LoadQuerySource(FString& OutSource)
	{
		const FString Directory = FPaths::Combine(
			FPaths::ProjectDir(), TEXT("Source/FMCodex/CoreRules"));
		FString Header;
		FString Implementation;
		const bool bHeaderLoaded = FFileHelper::LoadFileToString(
			Header, *FPaths::Combine(Directory, TEXT("PassControlDribbleAdvancePlanQuery.h")));
		const bool bImplementationLoaded = FFileHelper::LoadFileToString(
			Implementation, *FPaths::Combine(Directory, TEXT("PassControlDribbleAdvancePlanQuery.cpp")));
		OutSource = Header + Implementation;
		return bHeaderLoaded && bImplementationLoaded;
	}

	enum class ECase : uint8
	{
		SuccessWithHelper, SuccessWithoutHelper, DribblingMapping, HalfMapping,
		DefenseWithHelper, DefenseWithoutHelper, PreservesD6AndRunner, GoalkeeperCarrier,
		RejectsNone, RejectsPass, RejectsRun, RejectsUnknown,
		UnexpectedHelperCard, UnexpectedHelperPlayer, UnexpectedHelperBoth,
		MissingHelperCard, MissingHelperPlayer, MissingHelperBoth, MissingHelperSnapshot,
		MissingCarrierSnapshot, MissingRunnerSnapshot, MissingMarkerSnapshot,
		SnapshotDiagnosticStates, SkillTypeMismatch, MissingSkillRule, CarrierMissingSkill,
		RunnerNotMidfield, ValidActionPoint, InvalidGlobalActionPoint, LowSkillActionPoint,
		HighSkillActionPoint, AttackD6One, AttackD6Six, AttackD6Low, AttackD6High,
		DefenseD6One, DefenseD6Six, DefenseD6Low, DefenseD6High,
		MissingAttackD6, MissingDefenseD6, MissingRequiredIdentity,
		MissingSkillId, InvalidLogId, InvalidTurnIndex, InputUnchanged, ResultHelperState,
		NoFormulaExecutionDependencies, NoRandomOrAdvanceD6, NoForbiddenFrameworkDependencies
	};

	bool RunCase(FAutomationTestBase& Test, const ECase Case)
	{
		auto ExpectAdvanceTypeFailure = [&Test](const EPassControlAdvanceType Type)
		{
			FPassControlDribbleAdvancePlanQueryInput Input = MakeValidInput();
			Input.AdvanceType = Type;
			return ExpectFailure(Test, Build(Input),
				EPassControlDribbleAdvancePlanQueryErrorCode::InvalidAdvanceType,
				TEXT("AdvanceType"));
		};

		switch (Case)
		{
		case ECase::SuccessWithHelper:
		{
			const auto Result = Build();
			Test.TestTrue(TEXT("Query succeeds"), Result.bSuccess);
			Test.TestTrue(TEXT("Formula plan exists"), Result.bHasFormulaPlan);
			Test.TestTrue(TEXT("Helper is recorded"), Result.bHasHelper);
			Test.TestEqual(TEXT("Attacker FormulaType"), Result.FormulaPlan.AttackerQueryInput.FormulaType, EFormulaType::Finishing);
			Test.TestEqual(TEXT("Defender FormulaType"), Result.FormulaPlan.DefenderQueryInput.FormulaType, EFormulaType::Finishing);
			return true;
		}
		case ECase::SuccessWithoutHelper:
		{
			const auto Input = MakeInputWithoutHelper();
			const auto Result = Build(Input);
			Test.TestTrue(TEXT("No Helper is legal"), Result.bSuccess);
			Test.TestFalse(TEXT("No Helper is recorded"), Result.bHasHelper);
			Test.TestFalse(TEXT("No Helper query succeeds"), Result.HelperSnapshotQueryResult.bSuccess);
			Test.TestTrue(TEXT("No Helper query result is empty"), Result.HelperSnapshotQueryResult.CardId.IsNone());
			return true;
		}
		case ECase::DribblingMapping:
		{
			const auto Result = Build(MakeValidInput(), MakeSnapshots(4, 6));
			Test.TestEqual(TEXT("Attacker uses Dribbling"), Result.FormulaPlan.AttackerQueryInput.Attribute, ESingleCardFormulaAttribute::Dribbling);
			Test.TestEqual(TEXT("Dribbling modifier"), Result.FormulaPlan.AttackerQueryInput.ExternalModifier, 1.0f);
			return true;
		}
		case ECase::HalfMapping:
		{
			const auto Result = Build(MakeValidInput(), MakeSnapshots(3, 6));
			Test.TestEqual(TEXT("Half-point modifier is preserved"), Result.FormulaPlan.AttackerQueryInput.ExternalModifier, 1.5f);
			return true;
		}
		case ECase::DefenseWithHelper:
		{
			const auto Result = Build(MakeValidInput(), MakeSnapshots(4, 6, 4, 6));
			Test.TestEqual(TEXT("Defender uses Tackling"), Result.FormulaPlan.DefenderQueryInput.Attribute, ESingleCardFormulaAttribute::Tackling);
			Test.TestEqual(TEXT("Helper defense mapping includes plus two"), Result.FormulaPlan.DefenderQueryInput.ExternalModifier, 3.0f);
			return true;
		}
		case ECase::DefenseWithoutHelper:
		{
			const auto Result = Build(MakeInputWithoutHelper(), MakeSnapshots(4, 6, 6, 6));
			Test.TestEqual(TEXT("No Helper uses zero Marking"), Result.FormulaPlan.DefenderQueryInput.ExternalModifier, -1.0f);
			return true;
		}
		case ECase::PreservesD6AndRunner:
		{
			auto Input = MakeValidInput(); Input.ExternalAttackD6 = 6; Input.ExternalDefenseD6 = 1;
			const auto Result = Build(Input);
			Test.TestEqual(TEXT("Attack D6 is preserved"), Result.FormulaPlan.AttackerQueryInput.ExternalD6ComparePoint, 6);
			Test.TestEqual(TEXT("Defense D6 is preserved"), Result.FormulaPlan.DefenderQueryInput.ExternalD6ComparePoint, 1);
			Test.TestEqual(TEXT("Runner CardId is preserved"), Result.FormulaPlan.RunnerCardId, RunnerCardId);
			Test.TestEqual(TEXT("Runner PlayerId is preserved"), Result.FormulaPlan.RunnerPlayerId, RunnerPlayerId);
			return true;
		}
		case ECase::GoalkeeperCarrier:
		{
			const auto CarrierGoalkeeper = Build(
				MakeValidInput(), MakeSnapshots(4, 6, 5, 3, true, true));
			ExpectFailure(
				Test,
				CarrierGoalkeeper,
				EPassControlDribbleAdvancePlanQueryErrorCode::UnsupportedGoalkeeperParticipant,
				TEXT("CarrierCardId"));

			const auto LegalMarkerAndHelperGoalkeepers = Build(
				MakeValidInput(), MakeSnapshots(4, 6, 5, 3, true, false, true, true, true));
			Test.TestTrue(
				TEXT("Marker and Helper GK flags do not cause a dedicated rejection"),
				LegalMarkerAndHelperGoalkeepers.bSuccess);
			return true;
		}
		case ECase::RejectsNone: return ExpectAdvanceTypeFailure(EPassControlAdvanceType::None);
		case ECase::RejectsPass: return ExpectAdvanceTypeFailure(EPassControlAdvanceType::PassAdvance);
		case ECase::RejectsRun: return ExpectAdvanceTypeFailure(EPassControlAdvanceType::RunAdvance);
		case ECase::RejectsUnknown: return ExpectAdvanceTypeFailure(static_cast<EPassControlAdvanceType>(255));
		case ECase::UnexpectedHelperCard:
		{
			auto Input = MakeInputWithoutHelper(); Input.HelperCardId = HelperCardId;
			return ExpectFailure(Test, Build(Input), EPassControlDribbleAdvancePlanQueryErrorCode::UnexpectedHelperIdentity, TEXT("HelperCardId"));
		}
		case ECase::UnexpectedHelperPlayer:
		{
			auto Input = MakeInputWithoutHelper(); Input.HelperPlayerId = HelperPlayerId;
			return ExpectFailure(Test, Build(Input), EPassControlDribbleAdvancePlanQueryErrorCode::UnexpectedHelperIdentity, TEXT("HelperPlayerId"));
		}
		case ECase::UnexpectedHelperBoth:
		{
			auto Input = MakeInputWithoutHelper(); Input.HelperCardId = HelperCardId; Input.HelperPlayerId = HelperPlayerId;
			return ExpectFailure(Test, Build(Input), EPassControlDribbleAdvancePlanQueryErrorCode::UnexpectedHelperIdentity, TEXT("HelperCardId"));
		}
		case ECase::MissingHelperCard:
		{
			auto Input = MakeValidInput(); Input.HelperCardId = NAME_None;
			return ExpectFailure(Test, Build(Input), EPassControlDribbleAdvancePlanQueryErrorCode::MissingHelperIdentity, TEXT("HelperCardId"));
		}
		case ECase::MissingHelperPlayer:
		{
			auto Input = MakeValidInput(); Input.HelperPlayerId = NAME_None;
			return ExpectFailure(Test, Build(Input), EPassControlDribbleAdvancePlanQueryErrorCode::MissingHelperIdentity, TEXT("HelperPlayerId"));
		}
		case ECase::MissingHelperBoth:
		{
			auto Input = MakeValidInput(); Input.HelperCardId = NAME_None; Input.HelperPlayerId = NAME_None;
			return ExpectFailure(Test, Build(Input), EPassControlDribbleAdvancePlanQueryErrorCode::MissingHelperIdentity, TEXT("HelperCardId"));
		}
		case ECase::MissingHelperSnapshot:
		{
			auto Input = MakeValidInput(); Input.HelperCardId = TEXT("Missing.Helper");
			const auto Result = Build(Input);
			Test.TestFalse(TEXT("Helper query fails"), Result.HelperSnapshotQueryResult.bSuccess);
			return ExpectFailure(Test, Result, EPassControlDribbleAdvancePlanQueryErrorCode::HelperSnapshotQueryFailed, TEXT("HelperCardId"));
		}
		case ECase::MissingCarrierSnapshot:
		{
			auto Input = MakeValidInput(); Input.CarrierCardId = TEXT("Missing.Carrier");
			return ExpectFailure(Test, Build(Input), EPassControlDribbleAdvancePlanQueryErrorCode::CarrierSnapshotQueryFailed, TEXT("CarrierCardId"));
		}
		case ECase::MissingRunnerSnapshot:
		{
			auto Input = MakeValidInput(); Input.RunnerCardId = TEXT("Missing.Runner");
			return ExpectFailure(Test, Build(Input), EPassControlDribbleAdvancePlanQueryErrorCode::RunnerSnapshotQueryFailed, TEXT("RunnerCardId"));
		}
		case ECase::MissingMarkerSnapshot:
		{
			auto Input = MakeValidInput(); Input.MarkerCardId = TEXT("Missing.Marker");
			return ExpectFailure(Test, Build(Input), EPassControlDribbleAdvancePlanQueryErrorCode::MarkerSnapshotQueryFailed, TEXT("MarkerCardId"));
		}
		case ECase::SnapshotDiagnosticStates:
		{
			auto FailedInput = MakeValidInput(); FailedInput.HelperCardId = TEXT("Missing.Helper");
			const auto Failed = Build(FailedInput); const auto Absent = Build(MakeInputWithoutHelper());
			Test.TestEqual(TEXT("Failed query keeps CardId"), Failed.HelperSnapshotQueryResult.CardId, FailedInput.HelperCardId);
			Test.TestTrue(TEXT("Absent query keeps empty CardId"), Absent.HelperSnapshotQueryResult.CardId.IsNone());
			return true;
		}
		case ECase::SkillTypeMismatch: return ExpectFailure(Test, Build(MakeValidInput(), MakeSnapshots(), MakeSkillRules(ESkillRuleType::LongShot)), EPassControlDribbleAdvancePlanQueryErrorCode::SkillRuleTypeMismatch, TEXT("SkillType"));
		case ECase::MissingSkillRule: return ExpectFailure(Test, Build(MakeValidInput(), MakeSnapshots(), FSkillRuleSnapshotSet()), EPassControlDribbleAdvancePlanQueryErrorCode::SkillRuleQueryFailed, TEXT("SkillId"));
		case ECase::CarrierMissingSkill: return ExpectFailure(Test, Build(MakeValidInput(), MakeSnapshots(4, 6, 5, 3, false)), EPassControlDribbleAdvancePlanQueryErrorCode::CarrierMissingSkill, TEXT("SkillId"));
		case ECase::RunnerNotMidfield: return ExpectFailure(Test, Build(MakeValidInput(), MakeSnapshots(4, 6, 5, 3, true, false, false)), EPassControlDribbleAdvancePlanQueryErrorCode::RunnerNotMidfield, TEXT("RunnerCardId"));
		case ECase::ValidActionPoint: Test.TestTrue(TEXT("Current action point is accepted"), Build().bSuccess); return true;
		case ECase::InvalidGlobalActionPoint:
		{
			auto Input = MakeValidInput(); Input.CurrentActionPoint = 1;
			return ExpectFailure(Test, Build(Input), EPassControlDribbleAdvancePlanQueryErrorCode::InvalidActionPoint, TEXT("CurrentActionPoint"));
		}
		case ECase::LowSkillActionPoint:
		{
			auto Input = MakeValidInput(); Input.CurrentActionPoint = 2;
			return ExpectFailure(Test, Build(Input), EPassControlDribbleAdvancePlanQueryErrorCode::ActionPointOutOfRange, TEXT("CurrentActionPoint"));
		}
		case ECase::HighSkillActionPoint:
		{
			auto Input = MakeValidInput(); Input.CurrentActionPoint = 7;
			return ExpectFailure(Test, Build(Input), EPassControlDribbleAdvancePlanQueryErrorCode::ActionPointOutOfRange, TEXT("CurrentActionPoint"));
		}
		case ECase::AttackD6One: { auto Input = MakeValidInput(); Input.ExternalAttackD6 = 1; Test.TestTrue(TEXT("Attack D6 lower bound succeeds"), Build(Input).bSuccess); return true; }
		case ECase::AttackD6Six: { auto Input = MakeValidInput(); Input.ExternalAttackD6 = 6; Test.TestTrue(TEXT("Attack D6 upper bound succeeds"), Build(Input).bSuccess); return true; }
		case ECase::AttackD6Low: { auto Input = MakeValidInput(); Input.ExternalAttackD6 = 0; return ExpectFailure(Test, Build(Input), EPassControlDribbleAdvancePlanQueryErrorCode::InvalidAttackD6, TEXT("ExternalAttackD6")); }
		case ECase::AttackD6High: { auto Input = MakeValidInput(); Input.ExternalAttackD6 = 7; return ExpectFailure(Test, Build(Input), EPassControlDribbleAdvancePlanQueryErrorCode::InvalidAttackD6, TEXT("ExternalAttackD6")); }
		case ECase::DefenseD6One: { auto Input = MakeValidInput(); Input.ExternalDefenseD6 = 1; Test.TestTrue(TEXT("Defense D6 lower bound succeeds"), Build(Input).bSuccess); return true; }
		case ECase::DefenseD6Six: { auto Input = MakeValidInput(); Input.ExternalDefenseD6 = 6; Test.TestTrue(TEXT("Defense D6 upper bound succeeds"), Build(Input).bSuccess); return true; }
		case ECase::DefenseD6Low: { auto Input = MakeValidInput(); Input.ExternalDefenseD6 = 0; return ExpectFailure(Test, Build(Input), EPassControlDribbleAdvancePlanQueryErrorCode::InvalidDefenseD6, TEXT("ExternalDefenseD6")); }
		case ECase::DefenseD6High: { auto Input = MakeValidInput(); Input.ExternalDefenseD6 = 7; return ExpectFailure(Test, Build(Input), EPassControlDribbleAdvancePlanQueryErrorCode::InvalidDefenseD6, TEXT("ExternalDefenseD6")); }
		case ECase::MissingAttackD6:
		{
			auto Input = MakeValidInput(); Input.bHasExternalAttackD6 = false;
			return ExpectFailure(Test, Build(Input), EPassControlDribbleAdvancePlanQueryErrorCode::MissingExternalAttackD6, TEXT("ExternalAttackD6"));
		}
		case ECase::MissingDefenseD6:
		{
			auto Input = MakeValidInput(); Input.bHasExternalDefenseD6 = false;
			return ExpectFailure(Test, Build(Input), EPassControlDribbleAdvancePlanQueryErrorCode::MissingExternalDefenseD6, TEXT("ExternalDefenseD6"));
		}
		case ECase::MissingRequiredIdentity:
		{
			auto Input = MakeValidInput(); Input.RunnerPlayerId = NAME_None;
			return ExpectFailure(Test, Build(Input), EPassControlDribbleAdvancePlanQueryErrorCode::MissingRequiredIdentity, TEXT("RunnerPlayerId"));
		}
		case ECase::MissingSkillId:
		{
			auto Input = MakeValidInput(); Input.SkillId = NAME_None;
			return ExpectFailure(Test, Build(Input), EPassControlDribbleAdvancePlanQueryErrorCode::MissingSkillId, TEXT("SkillId"));
		}
		case ECase::InvalidLogId:
		{
			auto Input = MakeValidInput(); Input.LogId.Invalidate();
			return ExpectFailure(Test, Build(Input), EPassControlDribbleAdvancePlanQueryErrorCode::InvalidLogContext, TEXT("LogId"));
		}
		case ECase::InvalidTurnIndex:
		{
			auto Input = MakeValidInput(); Input.TurnIndex = -1;
			return ExpectFailure(Test, Build(Input), EPassControlDribbleAdvancePlanQueryErrorCode::InvalidLogContext, TEXT("TurnIndex"));
		}
		case ECase::InputUnchanged:
		{
			const auto Input = MakeValidInput(); const auto Before = Input; const auto Result = Build(Input);
			Test.TestTrue(TEXT("Input remains unchanged"), InputsEqual(Input, Before));
			Test.TestTrue(TEXT("Result keeps input copy"), InputsEqual(Result.Input, Before));
			return true;
		}
		case ECase::ResultHelperState:
		{
			const auto WithHelper = Build(); const auto WithoutHelper = Build(MakeInputWithoutHelper());
			Test.TestTrue(TEXT("Result agrees for selected Helper"), WithHelper.bHasHelper == WithHelper.Input.bHasHelper);
			Test.TestTrue(TEXT("Result agrees for no Helper"), WithoutHelper.bHasHelper == WithoutHelper.Input.bHasHelper);
			return true;
		}
		case ECase::NoFormulaExecutionDependencies:
		{
			FString Source; Test.TestTrue(TEXT("Query source loads"), LoadQuerySource(Source));
			Test.TestFalse(TEXT("No FormulaResolver"), Source.Contains(TEXT("FormulaResolver")));
			Test.TestFalse(TEXT("No InputAssemblyQuery call"), Source.Contains(TEXT("FSingleCardFormulaInputAssemblyQuery::Assemble")));
			Test.TestFalse(TEXT("No ResolutionExecutor"), Source.Contains(TEXT("ResolutionExecutor")));
			Test.TestFalse(TEXT("No ResolverInputAssembler"), Source.Contains(TEXT("ResolverInputAssembler")));
			return true;
		}
		case ECase::NoRandomOrAdvanceD6:
		{
			FString Source; Test.TestTrue(TEXT("Query source loads"), LoadQuerySource(Source));
			Test.TestFalse(TEXT("No random generation"), Source.Contains(TEXT("Rand")));
			Test.TestFalse(TEXT("No Advance Selection query"), Source.Contains(TEXT("FPassControlAdvanceSelectionQuery")));
			return true;
		}
		case ECase::NoForbiddenFrameworkDependencies:
		{
			FString Source; Test.TestTrue(TEXT("Query source loads"), LoadQuerySource(Source));
			Test.TestFalse(TEXT("No MatchPlay"), Source.Contains(TEXT("MatchPlay")));
			Test.TestFalse(TEXT("No SkillPipeline"), Source.Contains(TEXT("SkillPipeline")));
			Test.TestFalse(TEXT("No SkillEffect"), Source.Contains(TEXT("SkillEffect")));
			Test.TestFalse(TEXT("No DataTable"), Source.Contains(TEXT("DataTable")));
			return true;
		}
		}
		return false;
	}
}

#define PASS_CONTROL_DRIBBLE_ADVANCE_PLAN_TEST(ClassName, CaseName) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
		ClassName, \
		"FMCodex.CoreRules.PassControlDribbleAdvancePlanQuery." #CaseName, \
		EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter) \
	bool ClassName::RunTest(const FString& Parameters) \
	{ return PassControlDribbleAdvancePlanQueryTests::RunCase(*this, PassControlDribbleAdvancePlanQueryTests::ECase::CaseName); }

PASS_CONTROL_DRIBBLE_ADVANCE_PLAN_TEST(FPassControlDribbleAdvanceSuccessWithHelperTest, SuccessWithHelper)
PASS_CONTROL_DRIBBLE_ADVANCE_PLAN_TEST(FPassControlDribbleAdvanceSuccessWithoutHelperTest, SuccessWithoutHelper)
PASS_CONTROL_DRIBBLE_ADVANCE_PLAN_TEST(FPassControlDribbleAdvanceDribblingMappingTest, DribblingMapping)
PASS_CONTROL_DRIBBLE_ADVANCE_PLAN_TEST(FPassControlDribbleAdvanceHalfMappingTest, HalfMapping)
PASS_CONTROL_DRIBBLE_ADVANCE_PLAN_TEST(FPassControlDribbleAdvanceDefenseWithHelperTest, DefenseWithHelper)
PASS_CONTROL_DRIBBLE_ADVANCE_PLAN_TEST(FPassControlDribbleAdvanceDefenseWithoutHelperTest, DefenseWithoutHelper)
PASS_CONTROL_DRIBBLE_ADVANCE_PLAN_TEST(FPassControlDribbleAdvancePreservesD6AndRunnerTest, PreservesD6AndRunner)
PASS_CONTROL_DRIBBLE_ADVANCE_PLAN_TEST(FPassControlDribbleAdvanceGoalkeeperCarrierTest, GoalkeeperCarrier)
PASS_CONTROL_DRIBBLE_ADVANCE_PLAN_TEST(FPassControlDribbleAdvanceRejectsNoneTest, RejectsNone)
PASS_CONTROL_DRIBBLE_ADVANCE_PLAN_TEST(FPassControlDribbleAdvanceRejectsPassTest, RejectsPass)
PASS_CONTROL_DRIBBLE_ADVANCE_PLAN_TEST(FPassControlDribbleAdvanceRejectsRunTest, RejectsRun)
PASS_CONTROL_DRIBBLE_ADVANCE_PLAN_TEST(FPassControlDribbleAdvanceRejectsUnknownTest, RejectsUnknown)
PASS_CONTROL_DRIBBLE_ADVANCE_PLAN_TEST(FPassControlDribbleAdvanceUnexpectedHelperCardTest, UnexpectedHelperCard)
PASS_CONTROL_DRIBBLE_ADVANCE_PLAN_TEST(FPassControlDribbleAdvanceUnexpectedHelperPlayerTest, UnexpectedHelperPlayer)
PASS_CONTROL_DRIBBLE_ADVANCE_PLAN_TEST(FPassControlDribbleAdvanceUnexpectedHelperBothTest, UnexpectedHelperBoth)
PASS_CONTROL_DRIBBLE_ADVANCE_PLAN_TEST(FPassControlDribbleAdvanceMissingHelperCardTest, MissingHelperCard)
PASS_CONTROL_DRIBBLE_ADVANCE_PLAN_TEST(FPassControlDribbleAdvanceMissingHelperPlayerTest, MissingHelperPlayer)
PASS_CONTROL_DRIBBLE_ADVANCE_PLAN_TEST(FPassControlDribbleAdvanceMissingHelperBothTest, MissingHelperBoth)
PASS_CONTROL_DRIBBLE_ADVANCE_PLAN_TEST(FPassControlDribbleAdvanceMissingHelperSnapshotTest, MissingHelperSnapshot)
PASS_CONTROL_DRIBBLE_ADVANCE_PLAN_TEST(FPassControlDribbleAdvanceMissingCarrierSnapshotTest, MissingCarrierSnapshot)
PASS_CONTROL_DRIBBLE_ADVANCE_PLAN_TEST(FPassControlDribbleAdvanceMissingRunnerSnapshotTest, MissingRunnerSnapshot)
PASS_CONTROL_DRIBBLE_ADVANCE_PLAN_TEST(FPassControlDribbleAdvanceMissingMarkerSnapshotTest, MissingMarkerSnapshot)
PASS_CONTROL_DRIBBLE_ADVANCE_PLAN_TEST(FPassControlDribbleAdvanceSnapshotDiagnosticStatesTest, SnapshotDiagnosticStates)
PASS_CONTROL_DRIBBLE_ADVANCE_PLAN_TEST(FPassControlDribbleAdvanceSkillTypeMismatchTest, SkillTypeMismatch)
PASS_CONTROL_DRIBBLE_ADVANCE_PLAN_TEST(FPassControlDribbleAdvanceMissingSkillRuleTest, MissingSkillRule)
PASS_CONTROL_DRIBBLE_ADVANCE_PLAN_TEST(FPassControlDribbleAdvanceCarrierMissingSkillTest, CarrierMissingSkill)
PASS_CONTROL_DRIBBLE_ADVANCE_PLAN_TEST(FPassControlDribbleAdvanceRunnerNotMidfieldTest, RunnerNotMidfield)
PASS_CONTROL_DRIBBLE_ADVANCE_PLAN_TEST(FPassControlDribbleAdvanceValidActionPointTest, ValidActionPoint)
PASS_CONTROL_DRIBBLE_ADVANCE_PLAN_TEST(FPassControlDribbleAdvanceInvalidGlobalActionPointTest, InvalidGlobalActionPoint)
PASS_CONTROL_DRIBBLE_ADVANCE_PLAN_TEST(FPassControlDribbleAdvanceLowSkillActionPointTest, LowSkillActionPoint)
PASS_CONTROL_DRIBBLE_ADVANCE_PLAN_TEST(FPassControlDribbleAdvanceHighSkillActionPointTest, HighSkillActionPoint)
PASS_CONTROL_DRIBBLE_ADVANCE_PLAN_TEST(FPassControlDribbleAdvanceAttackD6OneTest, AttackD6One)
PASS_CONTROL_DRIBBLE_ADVANCE_PLAN_TEST(FPassControlDribbleAdvanceAttackD6SixTest, AttackD6Six)
PASS_CONTROL_DRIBBLE_ADVANCE_PLAN_TEST(FPassControlDribbleAdvanceAttackD6LowTest, AttackD6Low)
PASS_CONTROL_DRIBBLE_ADVANCE_PLAN_TEST(FPassControlDribbleAdvanceAttackD6HighTest, AttackD6High)
PASS_CONTROL_DRIBBLE_ADVANCE_PLAN_TEST(FPassControlDribbleAdvanceDefenseD6OneTest, DefenseD6One)
PASS_CONTROL_DRIBBLE_ADVANCE_PLAN_TEST(FPassControlDribbleAdvanceDefenseD6SixTest, DefenseD6Six)
PASS_CONTROL_DRIBBLE_ADVANCE_PLAN_TEST(FPassControlDribbleAdvanceDefenseD6LowTest, DefenseD6Low)
PASS_CONTROL_DRIBBLE_ADVANCE_PLAN_TEST(FPassControlDribbleAdvanceDefenseD6HighTest, DefenseD6High)
PASS_CONTROL_DRIBBLE_ADVANCE_PLAN_TEST(FPassControlDribbleAdvanceMissingAttackD6Test, MissingAttackD6)
PASS_CONTROL_DRIBBLE_ADVANCE_PLAN_TEST(FPassControlDribbleAdvanceMissingDefenseD6Test, MissingDefenseD6)
PASS_CONTROL_DRIBBLE_ADVANCE_PLAN_TEST(FPassControlDribbleAdvanceMissingRequiredIdentityTest, MissingRequiredIdentity)
PASS_CONTROL_DRIBBLE_ADVANCE_PLAN_TEST(FPassControlDribbleAdvanceMissingSkillIdTest, MissingSkillId)
PASS_CONTROL_DRIBBLE_ADVANCE_PLAN_TEST(FPassControlDribbleAdvanceInvalidLogIdTest, InvalidLogId)
PASS_CONTROL_DRIBBLE_ADVANCE_PLAN_TEST(FPassControlDribbleAdvanceInvalidTurnIndexTest, InvalidTurnIndex)
PASS_CONTROL_DRIBBLE_ADVANCE_PLAN_TEST(FPassControlDribbleAdvanceInputUnchangedTest, InputUnchanged)
PASS_CONTROL_DRIBBLE_ADVANCE_PLAN_TEST(FPassControlDribbleAdvanceResultHelperStateTest, ResultHelperState)
PASS_CONTROL_DRIBBLE_ADVANCE_PLAN_TEST(FPassControlDribbleAdvanceNoFormulaExecutionDependenciesTest, NoFormulaExecutionDependencies)
PASS_CONTROL_DRIBBLE_ADVANCE_PLAN_TEST(FPassControlDribbleAdvanceNoRandomOrAdvanceD6Test, NoRandomOrAdvanceD6)
PASS_CONTROL_DRIBBLE_ADVANCE_PLAN_TEST(FPassControlDribbleAdvanceNoForbiddenFrameworkDependenciesTest, NoForbiddenFrameworkDependencies)

#undef PASS_CONTROL_DRIBBLE_ADVANCE_PLAN_TEST

#endif
